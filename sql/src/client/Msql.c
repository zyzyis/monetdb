

#include "mem.h"
#include <comm.h>
#include <sys/stat.h>
#include <query.h>

#ifdef HAVE_LIBGETOPT 
#include <getopt.h>
#endif

#ifdef HAVE_LIBREADLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif

stream *ws = NULL, *rs = NULL;

void usage( char *prog ){
	fprintf(stderr, "sql_client\n");
	fprintf(stderr, "\toptions:\n");
	fprintf(stderr, "\t\t -d          | --debug=[level]\n"); 
	fprintf(stderr, "\t\t -h hostname | --host=hostname  /* host to connect to */\n");
	fprintf(stderr, "\t\t -p portnr   | --port=portnr    /* port to connect to */\n");
	fprintf(stderr, "\t\t -u user     | --user=user      /* user id */\n" );
	fprintf(stderr, "\t\t -o level    | --optimize=level /* optimzation level */\n" );
	fprintf(stderr, "\t\t -a api      | --api=api \n"); 
	fprintf(stderr, " 	/* examples: python,sql(schema) */\n");
	exit(-1);
}


void receive( stream *rs ){
	int flag = 0;
	if (rs->readInt(rs, &flag) && flag != COMM_DONE){
		char buf[BLOCK+1], *n = buf;
		int last = 0;
		int type;
		int status;
		int nRows;

		rs->readInt(rs, &type);
		rs->readInt(rs, &status);
		if (status < 0){ /* output error */
			int nr = bs_read_next(rs,buf,&last);
			fprintf( stdout, "SQL ERROR %d: ", status );
			fwrite( n, nr, 1, stdout );
			while(!last){
				int nr = bs_read_next(rs,buf,&last);
				fwrite( buf, nr, 1, stdout );
			}
			fprintf( stdout, "\n");
		}
		nRows = status;
		if (type == QTABLE && nRows > 0){
			int nr = bs_read_next(rs,buf,&last);
	
			fwrite( buf, nr, 1, stdout );
			while(!last){
				int nr = bs_read_next(rs,buf,&last);
				fwrite( buf, nr, 1, stdout );
			}
		}
		if (type == QTABLE || type == QUPDATE){
			if (nRows > 1)
				printf("%d Rows affected\n", nRows );
			else if (nRows == 1)
				printf("1 Row affected\n" );
			else 
				printf("no Rows affected\n" );
		}
	} else if (flag != COMM_DONE){
		printf("flag %d\n", flag);
	}
}

int parse_line( const char *line )
{
	int cnt = 0;
	int ins = 0;
	int esc = 0;
	while (isspace(*line)) line++;
	if (*line && *line == '-' && line[1] == '-')
		return 0;
	for(;*line != 0; line++){
		if (esc){
			while (isdigit(*line)) 
				line++;
			esc = 0;
		} else if (ins &&  *line == '\''){
			ins = 0;
		} else if (*line == '\\'){
			esc = 1;
		} else if (*line == '\''){
			ins = 1;
		} else if (*line == ';'){
			cnt++;
		}
	}
	return cnt;
}

void clientAccept( stream *ws, stream *rs ){
	int  i,	is_chrsp	= 0;
	char *prompt = "> ";
	char *line = NULL;
	struct stat st;
	char buf[BUFSIZ];

	fstat(fileno(stdin),&st);
	if (S_ISCHR(st.st_mode))
	   is_chrsp = 1;

	while(!feof(stdin)){
		int cmdcnt = 0;
		if (line) {
			free(line);
		}
#ifdef HAVE_LIBREADLINE
		if (is_chrsp){
	        	if ((line = (char *) readline(prompt)) == NULL) {
	               		break;
			}
			add_history(line);
		} else 
#endif
		{
		   	char *buf =(char *)malloc(BUFSIZ);
	        	if ((line =(char *)fgets(buf,BUFSIZ,stdin))==NULL) {
			   	free(buf);
	                	break;
			}
		}
		cmdcnt = parse_line(line);
		ws->write( ws, line, strlen(line), 1 );
		if (cmdcnt)
			ws->flush( ws );

		for (; cmdcnt > 0; cmdcnt--)
			receive(rs);
	}
	
	i = snprintf(buf, BUFSIZ, "COMMIT;\n" );
	ws->write( ws, buf, i, 1 );
	ws->flush( ws );
	receive(rs);

	/* client waves goodbye */
	buf[0] = EOT; 
	ws->write( ws, buf, 1, 1 );
	ws->flush( ws );
}

int
main(int ac, char **av)
{
	char buf[BUFSIZ];
	char *user = NULL;
	char *prog = *av, *host = "localhost";
	int debug = 0, fd = 0, port = 45123;
	int opt = 1;
	char *api = NULL;

	static struct option long_options[] =
             {
               {"debug", 2, 0, 'd'},
               {"host", 1, 0, 'h'},
               {"port", 1, 0, 'p'},
               {"user", 1, 0, 'u'},
               {"optimize", 1, 0, 'o'},
               {"api", 1, 0, 'a'},
               {0, 0, 0, 0}
             };

	while(1){
		int option_index = 0;

		int c = getopt_long( ac, av, "a:dh:p:u:o:", 
				long_options, &option_index);

		if (c == -1)
			break;

		switch (c){
		case 0:
			/* all long options are mapped on their short version */
			printf("option %s", long_options[option_index].name);
			if (optarg)
				printf( " with arg %s", optarg );
			printf("\n");
			usage(prog);
			break;
		case 'a':
			api=_strdup(optarg);
			break;
		case 'd':
			debug=2;
			if (optarg) debug=strtol(optarg,NULL,10);
			break;
		case 'h':
			host=_strdup(optarg);
			break;
		case 'p':
			port=strtol(optarg,NULL,10);
			break;
		case 'u':
			user=_strdup(optarg);
			break;
		case 'o':
			opt=strtol(optarg,NULL,10);
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

	fd = client( host, port);
	rs = block_stream(socket_rstream( fd, "client read"));
	ws = block_stream(socket_wstream( fd, "client write"));
	if (rs->errnr || ws->errnr){
		fprintf(stderr, "sockets not opened correctly\n");
		exit(1);
	}
	if (!user) user = _strdup("default-user");
	snprintf(buf, BUFSIZ, "info(\"%s\", %d, %d);\n", user, debug, opt);
	ws->write( ws, buf, strlen(buf), 1 );
	ws->flush( ws );
	if (api){
		snprintf(buf, BUFSIZ, "%s;\n", api ); 
	} else {
		/* default sql */
		snprintf(buf, BUFSIZ, "sql;\n");
	}
	ws->write( ws, buf, strlen(buf), 1 );
	ws->flush( ws );

	clientAccept( ws, rs );

	if (rs){
	       	rs->close(rs);
	       	rs->destroy(rs);
	}
	ws->close(ws);
	ws->destroy(ws);

/*
libstream.so requires GDK{realloc,malloc,free}  and they are provided by
gdk.[co], here. 
Intel's "C++ Compiler for 32-bit applications, Version 5.0.1 Build
010730D0", however, doesn't see the implementations of
GDK{realloc,malloc,free}, and complains about "undefined references in
libstream.so", unless there are references to GDK{realloc,malloc,free}
in main(). Hence, we add such "fake" references, here.
(See also the "alloca-story" in sql_client.)
*/
#ifdef __INTEL_COMPILER
	if(0){
		(void)GDKrealloc(0,0);
		(void)GDKmalloc(0);
		(void)GDKfree(0);
	}
#endif

	return 0;
} /* main */

