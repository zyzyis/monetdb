

#include "Msql.h"
#include "mem.h"
#include <monet_options.h>
#include <comm.h>
#include <sys/stat.h>
#include <query.h>
#include <simple_prompt.h>

#ifdef HAVE_LIBREADLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif

#include <string.h>

#ifdef HAVE_LANGINFO_H
#include <langinfo.h>
#endif
#ifdef HAVE_ICONV_H
#include <iconv.h>
#endif
#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif

/*
 * Debug levels
 * 	0 	no debugging
 * 	1 	continue in case of errors 
 * 	2	output on server additional time statements
 * 	4	output on server bat.info for statements involving bats
 * 	8	output mil code to stderr
 * 	16	output parsed SQL
 * 	32	execute but there is no output send to the client
 * 	64 	output mil code only, no excution on the server.
 * 	1024	trace execution on the server-side by echoing each MIL-statement
 * 		before executing it and printing the respective result immediately
 * 		(for BATs, on the the cardinality and head- & tail-type are printed).
 * 	2048	like 1024, but also print BAT: 
 * 		with <40 BUNs, the whole BAT is printed;
 * 		for larges BATs, we print the first 10 BUNs, the last 10 BUNs, 
 * 		and 10 BUNs sampled from the rest.
 */ 

#define SQL_DUMP 1
#define MIL_DUMP 2

#if !defined(HAVE_SETLOCALE) || \
    !defined(HAVE_ICONV) || \
    !defined(HAVE_NL_LANGINFO)
#	define	NO_LOCALE
#endif

/* if on exit when there is a SQL error */
int exit_on_error = 0;

stream *ws = NULL, *rs = NULL;
int is_chrsp = 0;

#ifdef HAVE_ICONV
iconv_t to_utf = NULL;
iconv_t from_utf = NULL;

char *conv( char *org, iconv_t cd)
{
	if (cd){
		size_t len = strlen(org);
		size_t size = len * 4;
		char *buf = malloc(size);
		char *to = buf;
		char *from = org;

		if (iconv(cd, &from, &len, &to, &size ) == (size_t)-1){
			perror("conv ");
			fprintf(stderr, "could not convert string %s\n", from );
			free(buf);
			exit(1);
		}
		*to = 0;
		return buf;
	} else {
		return _strdup(org);
	}
}
#else
#define conv( org, dummy) _strdup(org)
#endif


void usage( char *prog ){
	fprintf(stderr, "Msql\n");
	fprintf(stderr, "\toptions:\n");
	fprintf(stderr, "\t\t -d          | --debug=[level]\n"); 
	fprintf(stderr, "\t\t -e          | --error          /* exit on a SQL error */\n");
	fprintf(stderr, "\t\t -h hostname | --host=hostname  /* host to connect to */\n");
	fprintf(stderr, "\t\t -p portnr   | --port=portnr    /* port to connect to */\n");
	fprintf(stderr, "\t\t -u user     | --user=user      /* user id */\n" );
	fprintf(stderr, "\t\t -P passwd   | --passwd=passwd  /* password */\n");
	fprintf(stderr, "\t\t               --dump=[mil]     /* dump sql or mil */\n");
	exit(-1);
}

static char *sql_readline( char *prompt ){
	char *line = NULL;
#ifdef HAVE_LIBREADLINE
	if (is_chrsp){
        	if ((line = (char *) readline(prompt)) != NULL) {
			add_history(line);
		}
	} else 
#endif
	{
		int len;
	   	char *buf =(char *)malloc(BUFSIZ);
        	if ((line =(char *)fgets(buf,BUFSIZ,stdin))==NULL) {
		   	free(buf);
			return line;
		}
		/* seems some fgets implementations don't replace the newline
		 * with a EOS marker
		 */
		len = strlen(line);
		if (len && line[len-1] == '\n') 
			line[len-1] = '\0';
	}		
	return line;
}

static int receive( stream *rs, stream *out, int trace );

static int forward_data(stream *out, int trace)
	/* read from stdin */
{
	int done = 0;

	if (trace){
		printf("forward data\n");
		fflush(stdout);
	}
	while(!done){
		char *s = sql_readline("");
		if (!s) {
			done = 1;
			break;
		} else {
			int len = strlen(s);

		       	if (len <= 1){
				out->write(out, "\n", 1, 1);
				if (trace)
					fwrite("\n", 1, 1, stdout);
				done = 1;
				break;
			}
			out->write(out, s, 1, len);
			out->write(out, "\n", 1, 1 );
			if (trace){
				fwrite(s, 1, len, stdout);
				fwrite("\n", 1, 1, stdout);
				fflush(stdout);
			}
		}
		if (s) free(s);
	}
	out->flush(out);
	if (trace) fflush(stdout);

	return receive(rs, out, trace);
}

static int column_info( char *buf, int len, Msql_col *cols, int cur ){
	char *end = buf + len;

	while(buf<end){
		char *s = buf;
		while(buf<end && *buf != ','){
			buf++;
		}
		if (buf>=end) 
			return cur;
		*buf = 0;
		cols[cur].name = conv(s, from_utf);
		s = buf++;
		while(buf<end && *buf != '\n'){
			buf++;
		}
		if (buf>=end) 
			return cur;
		*buf = 0;
		cols[cur].type = _strdup(s);
		buf++;

		cur++;
	}
	return cur;
}

static void header_data( stream *rs, stream *out, int nCols, int trace ){
	Msql_col *cols = (Msql_col*)malloc(sizeof(Msql_col) * nCols);
	int cur = 0;
	int i, len = 0;

	memset(cols, 0, nCols*sizeof(Msql_col));

	if (nCols > 0){ 
		char buf[BLOCK+1];
		int last = 0;
		int nr = bs_read_next(rs,buf,&last);

		cur = column_info(buf, nr, cols, cur);
		while(!last){
			int nr = bs_read_next(rs,buf,&last);
			cur = column_info(buf,nr, cols, cur);
		}
	}
	for (i=0; i<nCols; i++){
		if (cols[i].name){
			int l =	strlen(cols[i].name); 
	        	len += l;
		}
		len += 3;
	} len ++;

	printf("#"); for (i=0; i<len; i++){ printf("-"); } printf("\n");
	printf("#");
	for (i=0; i<nCols; i++){ 
		printf("| %s ", cols[i].name?cols[i].name:""); 
	} 
	printf("|\n");
	printf("#"); for (i=0; i<len; i++){ printf("-"); } printf("\n");

	for (i=0; i<nCols; i++){
		if (cols[i].name) free(cols[i].name);
		if (cols[i].type) free(cols[i].type);
	}
	free(cols);
}

int receive( stream *rs, stream *out, int trace ){
	int status = 0, type = 0, res = 0;
	if ((res = stream_readInt(rs, &type)) && type != Q_END){
		char buf[BLOCK+1];
		int last = 0;
		int nRows;

		stream_readInt(rs, &status);
		if (type < 0 || status < 0){ /* output error */
			int nr = bs_read_next(rs,buf,&last);
			char *s;

			fprintf( stdout, "SQL ERROR %d %d: ", type, status );

			buf[nr] = 0;
			s = conv(buf, from_utf);
			fwrite( s, strlen(s), 1, stdout );
			free(s);
			while(!last){
				int nr = bs_read_next(rs,buf,&last);
				buf[nr] = 0;
				s = conv(buf, from_utf);
				fwrite( s, strlen(s), 1, stdout );
				free(s);
			}
			fprintf( stdout, "\n");
			return status;
		}
		nRows = status;
		if ((type == Q_TABLE || type == Q_DEBUG) && nRows > 0){
			int nr = bs_read_next(rs,buf,&last);
			char *s;
	
			buf[nr] = 0;
			s = conv(buf, from_utf);
			fwrite( s, strlen(s), 1, stdout );
			free(s);
			while(!last){
				int nr = bs_read_next(rs,buf,&last);
				buf[nr] = 0;
				s = conv(buf, from_utf);
				fwrite( s, strlen(s), 1, stdout );
				free(s);
			}
		}
		if (type == Q_RESULT) {
			int i, id;
			stream_readInt(rs, &id);
			header_data(rs, out, nRows, trace);
			i = snprintf(buf, BLOCK, "mvc_export_table( myc, Output, %d, 0, -1, \"\\t\", \"\\n\");\n", id);
			out->write(out, buf, i, 1);
			out->flush(out);
			return receive(rs, out, trace);
		} else if (type == Q_DATA) {
			status = forward_data(out, trace);
		} else if (type == Q_TABLE || type == Q_UPDATE){
			if (nRows > 1)
				printf("SQL  %d Rows affected\n", nRows );
			else if (nRows == 1)
				printf("SQL  1 Row affected\n" );
			else 
				printf("SQL  no Rows affected\n" );
		}
	} else if (type != Q_END){
		printf("type %d, %d , %d\n", type, res, rs->errnr);
	} else {
		printf("type Q_END, %d , %d\n", res, rs->errnr);
	}
	fflush(stdout);
	return status;
}

static int ins = 0;
int parse_line( const unsigned char *line )
{
	int len = 0;
	int esc = 0;
	int cnt = 0;

	while (isspace(*line)) line++;

	for(;*line != 0; line++, len++){
		if (esc){
			while (isdigit(*line)) 
				line++;
			esc = 0;
		} else if (ins &&  *line == '\''){
			ins = 0;
		} else if (*line == '\\'){
			if (!ins && line[1] == 'q')
				return -1;
			esc = 1;
		} else if (*line == '\''){
			ins = 1;
		/* skip comments */
		} else if (!ins && len && 
			 ((*line == '-' && line[-1] == '-') || *line =='#') ){
			while(*line && *line != '\n') line++;
		} else if (!ins && len && *line == '*' && line[-1] == '/'){
			while(*line && *line != '/' && line[-1] != '*') line++;
		/* count command's */
		} else if (!ins && *line == ';'){
			cnt++;
		/* next checks are to skip long utf8 charachters */
		} else if (*line >= 0xF0){
			line += 3;
		} else if (*line >= 0xE0){
			line += 2;
		} else if (*line >= 0xC0){
			line += 1;
		}
	}
	return cnt;
}

void clientAccept( stream *ws, stream *rs, char *prompt, int trace ){
	int  i, lineno = 0;
	char *line = NULL;
	char buf[BUFSIZ];

	while(!feof(stdin)){
		int cmdcnt = 0;
		if (line) {
			free(line);
		}
		line = sql_readline(prompt);
		if (!line) break;
		lineno++;
		cmdcnt = parse_line((unsigned char*)line);
		if (cmdcnt < 0)
			break;
		if (trace)
			printf("# %5d: %d %s\n", lineno, cmdcnt, line);
#ifdef HAVE_ICONV
		if (to_utf)
		{
			char *converted = conv(line, to_utf);
			ws->write( ws, converted, strlen(converted), 1 );
			free(converted);
		} 
		else 
#endif
		{
			ws->write( ws, line, strlen(line), 1 );
		}
		ws->write( ws, "\n", 1, 1 );
		if (cmdcnt){
			ins = 0;
			ws->flush( ws );
		}

		for (; cmdcnt > 0; cmdcnt--) {
			int status = receive(rs, ws, trace);
			if (status < 0 && exit_on_error)
				exit(status);
		}
	}
	
	i = snprintf(buf, BUFSIZ, "COMMIT;\n" );
	ws->write( ws, buf, i, 1 );
	ws->flush( ws );
	(void)receive(rs, ws, trace);

	/* client waves goodbye */
	buf[0] = EOT; 
	ws->write( ws, buf, 1, 1 );
	ws->flush( ws );
}

void skip_block(stream *rs){
	int last = 0;
	char buf[BLOCK+1];

	bs_read_next(rs,buf,&last);
	while(!last){
		bs_read_next(rs,buf,&last);
	}
}

typedef int (*fptr)(stream *ws, stream *rs, char **lines, int cnt, int rlen, int dump, FILE *out );

int handle_result( stream *ws, stream *rs, int cnt, fptr f, int rlen, int dump, FILE *out){
	int i, res = 0, eof = 0, cur = 0;
	char *sc, *ec;
	bstream *bs = bstream_create(rs, BLOCK);
	char **lines = NEW_ARRAY(char*,cnt*rlen);

	eof = (bstream_read(bs, bs->size - (bs->len - bs->pos)) == 0);
	sc = bs->buf + bs->pos;
	ec = bs->buf + bs->len;
	while(sc < ec){
		char *s;
	
		s = sc;
	
		while(sc<ec && *sc != '\n') sc++;
	
		if (sc>=ec && !eof){
			bs->pos = s - bs->buf;
			eof = (bstream_read(bs, bs->size - (bs->len - bs->pos)) == 0);
			sc = bs->buf + bs->pos; 
			ec = bs->buf + bs->len; 
			continue;
		} else if (eof){
			break;
		}
	
		*sc = 0;
		for (i=0; i<rlen-1; i++){
			char *e = s;

			e = strchr(s, '\t');
			if (!e) 
				return -1;

			*e = 0;
			if (*s == '\"'){
				s++;
				*(e-1) = 0;
			}
			lines[cur++] = _strdup(s);
			s = e+1;
		}
		if (*s == '\"'){
			s++;
			*(sc-1) = 0;
		}
		lines[cur++] = _strdup(s);
		sc++;
	}
	bstream_destroy(bs);
	res = f(ws, rs, lines, cnt, rlen, dump, out);
	for(i=0; i<(cnt*rlen); i++)
		_DELETE(lines[i]);
	return res;
}

static int execute( stream *ws, stream *rs, const char *query)
{
	int type, status;

	ws->write(ws, (char*)query, strlen(query), 1);
	ws->flush(ws);

	if (!stream_readInt(rs, &type) || type == Q_END) {
		return -1;
	}

	stream_readInt(rs, &status);	/* read result size (is < 0 on error) */

	if (type != Q_RESULT)
		return -1;

	skip_block(rs);

	if (!stream_readInt(rs, &type) || type == Q_END) {
		return -1;
	}

	stream_readInt(rs, &status);	/* read result size (is < 0 on error) */

	if (type != Q_TABLE)
		return -1;

	return status;
}

static void dump_data( stream *rs, int nRows, char *table, int dump ){
	FILE *out = stdout;
	char buf[BLOCK+1];
	const char *copystring =
	    "COPY %d RECORDS INTO %s FROM stdin USING DELIMITERS '\\t';\n";
	int last = 0, nr = 0;

	if (dump == SQL_DUMP){
		printf( copystring, nRows, table);
	} else {
		snprintf(buf, BLOCK, "%s.data", table );
		out = fopen( buf, "w+");
		if (!out)
			printf( "Could not open %s for writing\n", table);
	}

	nr = bs_read_next(rs, buf, &last);
	fwrite( buf, nr, 1, out );
	while(!last){
		int nr = bs_read_next(rs, buf, &last);
		fwrite( buf, nr, 1, out );
	}
	fprintf(out, "\n"); /* extra empty line */
	if (out != stdout)
		fclose(out);
}

typedef struct sql_type {
	char *sqlname;
	int digits;
	int scale;
	char *name;
	struct sql_type *next;
} sql_type;

static sql_type *create_type(char *sname, char *digits, char *scale, char *name)
{
	sql_type *t = NEW(sql_type);
	t->sqlname = _strdup(sname);
	t->digits = strtol(digits,NULL,10);
	t->scale = strtol(scale,NULL,10);
	t->name = _strdup(name);
	t->next = NULL;
	return t;
}

static void destroy_type(sql_type *t)
{
	sql_type *n = t;
	while ( t ) {
		n = t->next;
		_DELETE(t->sqlname);
		_DELETE(t->name);
		_DELETE(t);
		t = n;
	}
}

static sql_type *types = NULL;

static char *find_type(char *name, char *Digits, char *Scale)
{
	int digits = strtol(Digits,NULL,10);
	/*int scale = strtol(Scale,NULL,10);*/
	sql_type *m, *n;

	/* assumes the types are ordered on name,digits,scale where is always
	 * 0 > n
	 */
	for ( n = types; n; n = n->next ) {
		if (strcasecmp(n->sqlname, name) == 0){
			if ((digits && n->digits >= digits) || 
					(digits == n->digits)){
				return n->name;
			}
			for (m = n; m; m = m->next ) {
				if (strcasecmp(m->sqlname, name) != 0){
					break;
				}
				n = m;
				if ((digits && m->digits >= digits) || 
					(digits == m->digits)){
					return m->name;
				}
			}
			return n->name;
		}
	}
	assert(0);
	return NULL;
}

static const char *types_format = "select sqlname,digits,scale,systemname from types;\n"; 
#define T_SQLNAME 0
#define T_DIGITS 1
#define T_SCALE 2
#define T_SYSTEMNAME 3

/* TODO nullable/default */
static int dump_type( stream *ws, stream *rs, char **ctypes, int cnt, int rlen, int dump, FILE *out )
{
	sql_type *prev = types;
	int i,j;

	for(i=0,j=0; i<cnt; i++, j+=rlen){
		sql_type *t = create_type( ctypes[j+T_SQLNAME], 
				    ctypes[j+T_DIGITS],
				    ctypes[j+T_SCALE],
				    ctypes[j+T_SYSTEMNAME]);
		if (prev) 
			prev -> next = t;
		if (!types)
			types = t;
		prev = t;
	}
	return 0;
}

static const char *column_format = "select name,type,type_digits,type_scale,null,default from columns c,tables t where c.table_id = t.id and '%s' = t.name order by c.number;\n"; 
#define COLUMN 6
#define C_NAME 0
#define C_TYPE 1
#define C_TYPE_DIGITS 2
#define C_TYPE_SCALE 3

/* TODO nullable/default */
static int dump_column( stream *ws, stream *rs, char **columns, int cnt, int rlen, int dump, FILE *out )
{
	int i,j;
	for(i=0,j=0; i<cnt; i++, j+=rlen){
	    if (dump == SQL_DUMP){
		if (strcmp(columns[j+C_TYPE_DIGITS], "0") == 0){
			printf("\t%s %s", 
					columns[j+C_NAME], columns[j+C_TYPE]);
		} else if (strcmp(columns[j+C_TYPE_SCALE], "0") == 0){
			printf("\t%s %s(%s)", 
					columns[j+C_NAME], columns[j+C_TYPE],
					columns[j+C_TYPE_DIGITS]);
		} else {
			printf("\t%s %s(%s,%s)", 
					columns[j+C_NAME], columns[j+C_TYPE],
					columns[j+C_TYPE_DIGITS],
					columns[j+C_TYPE_SCALE]);
		}
		if (i < cnt-1)
			printf(",");
		printf("\n");
	    } else {
		fprintf(out, "%s,\"\\%c\",%s\n", columns[j+C_NAME], 
					     (i==(cnt-1))?'n':'t',
					     find_type(columns[j+C_TYPE],
					     columns[j+C_TYPE_DIGITS],
					     columns[j+C_TYPE_SCALE])
					     );
	    }
	}
	return 0;
}

static int dump_table( stream *ws, stream *rs, char **tables, int cnt, int rlen, int dump, FILE *out)
{
	int i;
	for(i=0; i<cnt; i++){
		int ccnt, res = 0;
		char query[BUFSIZ], *s = tables[i];

		snprintf(query, BUFSIZ, column_format, s);
		ccnt = execute(ws, rs, query);
		if (ccnt > 0){
			FILE * out = stdout;


			if (dump == SQL_DUMP){
				printf("CREATE TABLE %s (\n", s );
			} else {
				char buf[BUFSIZ];
				snprintf(buf, BLOCK, "%s.fmt", s );
				out = fopen( buf, "w+");
				if (!out){
					printf( "Could not open %s for writing\n", s);
					return -1;
				}
			}
			res = handle_result(ws, rs, ccnt, &dump_column, COLUMN, dump, out);
			if (dump == SQL_DUMP)
				printf(");\n" );
			else
				fclose(out);
		}
		if (res == 0){
			char *select_format = "select * from %s;\n";
			snprintf(query, BUFSIZ, select_format, s);
			res = execute(ws, rs, query);
			if (res > 0)
				dump_data(rs, res, s, dump );
		} else {
			return res;
		}
	}
	return 0;
}

static int dump_view( stream *ws, stream *rs, char **views, int cnt, int rlen, int dump, FILE *out)
{
	int i,j;
	for(i=0, j=0; i<cnt; i++, j+=rlen)
		printf("CREATE VIEW %s %s\n", views[j+0], views[j+1] );
	return 0;
}

static int dump_tables( stream *ws, stream *rs, int dump )
{
	int cnt = 0;
	char *tables = "select name from tables where type=0;\n"; 
	char *views = "select name,query from tables where type=2;\n";

	cnt = execute( ws, rs, types_format);
	if (cnt > 0){
		int res = handle_result(ws, rs, cnt, &dump_type, 4, dump, stdout);
		if (res != 0) 
			return res;
	}

	cnt = execute( ws, rs, tables);
	if (cnt > 0){
		int res = handle_result(ws, rs, cnt, &dump_table, 1, dump, stdout);
		if (res != 0) 
			return res;
	}
	if (dump == SQL_DUMP){
		cnt = execute( ws, rs, views);
		if (cnt > 0){
			int res = handle_result(ws, rs, cnt, &dump_view, 2, dump, stdout);
			if (res != 0) 
				return res;
		}
	}

	return cnt;
}


int
main(int ac, char **av)
{
	char *prompt = NULL;
	char buf[BUFSIZ];
	char *prog = *av, *config = NULL, *passwd = NULL, *user = NULL;
	char *login = NULL, *db = NULL, *schema = NULL;
	int i = 0, debug = 0, fd = 0, port = 0, setlen = 0, dump = 0, trace = 0;
	opt 	*set = NULL;

	static struct option long_options[] =
             {
               {"debug", 2, 0, 'd'},
               {"error", 0, 0, 'e'},
	       {"dump", 2, 0, 'D'},
	       {"config", 1, 0, 'c'},
               {"host", 1, 0, 'h'},
               {"port", 1, 0, 'p'},
               {"passwd", 1, 0, 'P'},
               {"user", 1, 0, 'u'},
               {"trace",0, 0, 0},
               {0, 0, 0, 0}
             };

	if (!(setlen = mo_builtin_settings(&set)) )
                usage(prog);

	while(1){
		int option_index = 0;

		int c = getopt_long( ac, av, "c:d::eDh:p:P:u:", 
				long_options, &option_index);

		if (c == -1)
			break;

		switch (c){
		case 0:
			if (strcmp(long_options[option_index].name,
						"trace") == 0) {
				trace = 1;
				break;
			}
			/* all other long options are mapped on their short version */
			printf("option %s", long_options[option_index].name);
			if (optarg)
				printf( " with arg %s", optarg );
			printf("\n");
			usage(prog);
			break;
		case 'c':
			config = _strdup(optarg);
			break;
		case 'd':
			if (optarg){ 
				setlen = mo_add_option( &set, setlen, 
					opt_cmdline, "sql_debug", optarg );
			} else {
				setlen = mo_add_option( &set, setlen, 
					opt_cmdline, "sql_debug", "2" );
			}
			break;
		case 'e':
			exit_on_error = 1;
			break;
		case 'D':
			if (optarg){ 
				dump = MIL_DUMP;
			} else {
				dump = SQL_DUMP;
			}
			break;
		case 'h':
			setlen = mo_add_option( &set, setlen, 
					opt_cmdline, "host", optarg );
			break;
		case 'p':
			setlen = mo_add_option( &set, setlen, 
					opt_cmdline, "sql_port", optarg );
			break;
		case 'P':
			passwd = _strdup(optarg);
			break;
		case 'u':
			user = _strdup(optarg);
			break;
		case '?':
			usage(prog);
		default:
			printf( "?? getopt returned character code 0%o ??\n",c);
			usage(prog);
		}
	}

	if (optind < ac){
		printf("some arguments are not parsed by getopt\n");
		while(optind < ac)
			printf("%s ", av[optind++]);
		printf("\n");
		usage(prog);
	}

#ifndef NO_LOCALE 
	if (setlocale(LC_CTYPE, "") == NULL){ /* why is this needed ? */
		fprintf(stderr, "WARN: cannot set locale\n");
	} else {
		char *codeset = NULL;
		if ((codeset = nl_langinfo(CODESET)) == NULL){
			fprintf(stderr, "WARN: cannot get codeset\n");
		} else {
			to_utf = iconv_open("UTF-8", codeset);
			from_utf = iconv_open(codeset, "UTF-8" );
		}
	}
#endif

	if (config){
		setlen = mo_config_file(&set, setlen, config );
		free(config);
	} else {
		if (!(setlen = mo_system_config(&set, setlen)) )
			usage(prog);
	}

	stream_init();
	debug = strtol(mo_find_option(set, setlen, "sql_debug"), NULL, 10);
	port = strtol(mo_find_option(set, setlen, "sql_port"), NULL, 10);
	fd = client( mo_find_option(set, setlen, "host"), port);
	if (fd < 0)
		return fd;

	prompt = mo_find_option(set, setlen, "sql_prompt");
	if (!prompt) prompt = _strdup("Msql> ");

	rs = block_stream(socket_rstream( fd, "client read"));
	ws = block_stream(socket_wstream( fd, "client write"));
	if (rs->errnr || ws->errnr){
		fprintf(stderr, "sockets not opened correctly\n");
		exit(1);
	}
	/*
	 * New start client sequence
	 *
	 * 1) socket connect
	 * 2) send 'api(sql,debug)' api(sql,0);
	 * 3) receive request for login 
	 * 4) send user/passwd
	 */
	if (!user)
		user = simple_prompt("User: ", BUFSIZ, 1 );
	if (!passwd)
		passwd = simple_prompt("Password: ", BUFSIZ, 0 );

	snprintf(buf, BUFSIZ, "api(sql,%d);\n", debug ); 
	ws->write( ws, buf, strlen(buf), 1 );
	ws->flush( ws );
	/* read login */
	login = readblock( rs );

	if (login) free(login);

	i = snprintf(buf, BUFSIZ, "login(%s,%s);\n", user, passwd );
	ws->write( ws, buf, i, 1 );
	ws->flush( ws );
	/* read database, schema */
	db = readblock( rs );
	if (db){
		char *s = strrchr(db, ',');
		if (s){ 
			*s = '\0';
			schema = s+1;
			s = strrchr(schema, '\n');
			if (s){ 
				*s = '\0';
			}
		}
	}

	if (strlen(schema) > 0){
		struct stat st;

		fprintf(stdout, "SQL  connected to database %s using schema %s\n", db, schema ); 
		fflush(stdout);
		if (!dump){
			fstat(fileno(stdin),&st);
#ifdef HAVE_LIBREADLINE
			if (S_ISCHR(st.st_mode))
	   			is_chrsp = 1;
#endif
			clientAccept( ws, rs, prompt, trace );
		} else {
			dump_tables( ws, rs, dump );
		}
	}

	if (db) free(db); /* db + schema */
	if (user) free(user);
	if (passwd) free(passwd);
	if (prompt) free(prompt);

	if (rs){
	       	rs->close(rs);
	       	rs->destroy(rs);
	}
	ws->close(ws);
	ws->destroy(ws);

	destroy_type(types);
#ifndef NO_LOCALE 
	if (to_utf) iconv_close(to_utf);
	if (from_utf) iconv_close(from_utf);
#endif

	mo_free_options(set,setlen);
	return 0;
} /* main */
