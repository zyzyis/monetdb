
#include <unistd.h>
#include "symbol.h"
#include "sqlscan.h"
#include "sql.h"
#include <mem.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stream.h>
#include <statement.h>

void sql_statement_add(context * lc, char *token)
{
	int len = strlen(token);
	if ((lc->sqllen + len + 1) > lc->sqlsize) {
		while( (lc->sqllen + len + 1) > lc->sqlsize)
			lc->sqlsize *= 2;
		lc->sql = RENEW_ARRAY(char, lc->sql, lc->sqlsize);
	}
	strncpy(lc->sql + lc->sqllen - 1, token, len);
	lc->sqllen += len + 1;
	lc->sql[lc->sqllen - 2] = ' ';
	lc->sql[lc->sqllen - 1] = '\0';
}

#define HASH_SIZE 128
#define HASH_MASK (HASH_SIZE-1)

typedef struct keyword {
	char *keyword;
	int len;
	int token;
	struct keyword *next;
} keyword;

static int keywords_init_done = 0;
static keyword *keywords[HASH_SIZE];

static int keyword_key(char *k, int *l)
{
	char *s = k;
	int h = 0;
	while (*k) {
		h += tolower(*k) - 'a';
		k++;
	}
	*l = k - s;
	return (h < 0) ? -h : h;
}

void keywords_insert(char *k, int token)
{
	keyword *kw = NEW(keyword);
	int len = 0;
	int bucket = keyword_key(k, &len) & HASH_MASK;

	kw->keyword = toLower(k);
	kw->len = len;
	kw->token = token;
	kw->next = keywords[bucket];
	keywords[bucket] = kw;
}

void init_keywords()
{
	int i;

	if (keywords_init_done)
		return;
	keywords_init_done = 1;

	for (i = 0; i < HASH_SIZE; i++)
		keywords[i] = NULL;

	keywords_insert("false", BOOL_FALSE);
	keywords_insert("true", BOOL_TRUE);

	keywords_insert("ALL", ALL);
	keywords_insert("ALTER", ALTER);
	keywords_insert("ADD", ADD);
	keywords_insert("AND", AND);
	keywords_insert("AVG", AGGR);
	keywords_insert("MIN", AGGR);
	keywords_insert("MAX", AGGR);
	keywords_insert("SUM", AGGR);
	keywords_insert("COUNT", AGGR);
	keywords_insert("ANY", ANY);
	keywords_insert("AS", AS);
	keywords_insert("ASC", ASC);
	keywords_insert("AUTHORIZATION", AUTHORIZATION);
	keywords_insert("BETWEEN", BETWEEN);
	keywords_insert("SYMMETRIC", SYMMETRIC);
	keywords_insert("ASYMMETRIC", ASYMMETRIC);
	keywords_insert("BY", BY);
	keywords_insert("CAST", CAST);
	keywords_insert("CHARACTER", CHARACTER);
	keywords_insert("CHECK", CHECK);
	/*
	   keywords_insert("CLOSE", CLOSE );
	 */
	keywords_insert("CONSTRAINT", CONSTRAINT);
	/*
	   keywords_insert("CONTINUE", CONTINUE );
	 */
	keywords_insert("CREATE", CREATE);
	keywords_insert("CROSS", CROSS);
	/*
	   keywords_insert("CURRENT", CURRENT );
	   keywords_insert("CURSOR", CURSOR );
	 */
	keywords_insert("COPY", COPY );
	keywords_insert("RECORDS", RECORDS );
	keywords_insert("DELIMITERS", DELIMITERS );
	keywords_insert("STDIN", STDIN );

	keywords_insert("DEC", sqlDECIMAL);
	keywords_insert("DECIMAL", sqlDECIMAL);
	/*
	   keywords_insert("DECLARE", DECLARE );
	 */
	keywords_insert("DEFAULT", DEFAULT);
	keywords_insert("DESC", DESC);
	keywords_insert("DISTINCT", DISTINCT);
	keywords_insert("DOUBLE", sqlDOUBLE);
	keywords_insert("DROP", DROP);
	keywords_insert("ESCAPE", ESCAPE);
	keywords_insert("EXISTS", EXISTS);
	keywords_insert("EXTRACT", EXTRACT);
	/*
	   keywords_insert("FETCH", FETCH );
	 */
	keywords_insert("FLOAT", sqlFLOAT);
	keywords_insert("FOR", FOR);
	keywords_insert("FOREIGN", FOREIGN);
	/*
	   keywords_insert("FOUND", FOUND );
	 */
	keywords_insert("FROM", FROM);
	keywords_insert("REFERENCES", REFERENCES);

	keywords_insert("MATCH", MATCH);
	keywords_insert("FULL", FULL);
	keywords_insert("PARTIAL", PARTIAL);
	keywords_insert("SIMPLE", SIMPLE);

	keywords_insert("INSERT", INSERT);
	keywords_insert("UPDATE", UPDATE);
	keywords_insert("DELETE", sqlDELETE);

	keywords_insert("ACTION", ACTION);
	keywords_insert("CASCADE", CASCADE);
	keywords_insert("RESTRICT", RESTRICT);
	/*
	   keywords_insert("GO", GO );
	 */
	keywords_insert("GLOBAL", GLOBAL);
	keywords_insert("GROUP", GROUP);
	keywords_insert("HAVING", HAVING);
	keywords_insert("IN", sqlIN);
	keywords_insert("INNER", INNER);
	keywords_insert("INTO", INTO);
	keywords_insert("IS", IS);
	keywords_insert("JOIN", JOIN);
	keywords_insert("KEY", KEY);
	/*
	   keywords_insert("LANGUAGE", LANGUAGE );
	 */
	keywords_insert("LEFT", LEFT);
	keywords_insert("LIKE", LIKE);
	keywords_insert("LIMIT", LIMIT);
	keywords_insert("LOCAL", LOCAL);
	keywords_insert("NATURAL", NATURAL);
	keywords_insert("NOT", NOT);
	keywords_insert("NULL", NULLX);
	keywords_insert("NUMERIC", NUMERIC);
	/*
	   keywords_insert("OF", OF );
	 */
	keywords_insert("ON", ON);
	/*
	   keywords_insert("OPEN", OPEN );
	 */
	keywords_insert("OPTIONS", OPTIONS);
	keywords_insert("OPTION", OPTION);
	keywords_insert("OR", OR);
	keywords_insert("ORDER", ORDER);
	keywords_insert("OUTER", OUTER);
	keywords_insert("PATH", PATH);
	keywords_insert("PRECISION", PRECISION);
	keywords_insert("PRIMARY", PRIMARY);

	keywords_insert("GRANT", GRANT);
	keywords_insert("REVOKE", REVOKE);
	keywords_insert("ROLE", ROLE);
	keywords_insert("ADMIN", ADMIN);
	keywords_insert("PRIVILEGES", PRIVILEGES);
	keywords_insert("HIERARCHY", HIERARCHY);
	keywords_insert("PUBLIC", PUBLIC);
	keywords_insert("CURRENT_USER", CURRENT_USER);
	keywords_insert("CURRENT_ROLE", CURRENT_ROLE);
	/*
	   keywords_insert("PROCEDURE", PROCEDURE );
	 */
	keywords_insert("REAL", REAL);
	keywords_insert("RIGHT", RIGHT);
	keywords_insert("SCHEMA", SCHEMA);
	keywords_insert("SELECT", SELECT);
	keywords_insert("SET", SET);
	keywords_insert("SOME", SOME);
	/*
	   keywords_insert("SQLCODE", SQLCODE );
	 */
	keywords_insert("COLUMN", COLUMN);
	keywords_insert("TABLE", TABLE);
	keywords_insert("TEMPORARY", TEMPORARY);
	keywords_insert("TEMP", TEMPORARY);
	keywords_insert("TO", TO);
	/*
	   keywords_insert("UNDER", UNDER );
	 */
	keywords_insert("UNION", UNION);
	keywords_insert("UNIQUE", UNIQUE);
	keywords_insert("USER", USER);
	keywords_insert("USING", USING);
	keywords_insert("VALUES", VALUES);
	keywords_insert("VIEW", VIEW);
	/*
	   keywords_insert("WHENEVER", WHENEVER );
	 */
	keywords_insert("WHERE", WHERE);
	keywords_insert("WITH", WITH);

	keywords_insert("DATE", sqlDATE);
	keywords_insert("TIME", TIME);
	keywords_insert("TIMESTAMP", TIMESTAMP);
	keywords_insert("INTERVAL", INTERVAL);
	keywords_insert("CURRENT_DATE", CURRENT_DATE);
	keywords_insert("CURRENT_TIME", CURRENT_TIME);
	keywords_insert("CURRENT_TIMESTAMP", CURRENT_TIMESTAMP);
	keywords_insert("NOW", CURRENT_TIMESTAMP);

	keywords_insert("YEAR", YEAR);
	keywords_insert("MONTH", MONTH);
	keywords_insert("DAY", DAY);
	keywords_insert("HOUR", HOUR);
	keywords_insert("MINUTE", MINUTE);
	keywords_insert("SECOND", SECOND);

	keywords_insert("SUBSTRING", SUBSTRING);

	keywords_insert("CASE", CASE);
	keywords_insert("WHEN", WHEN);
	keywords_insert("THEN", THEN);
	keywords_insert("ELSE", ELSE);
	keywords_insert("END", END);
	keywords_insert("NULLIF", NULLIF);
	keywords_insert("COALESCE", COALESCE);

	keywords_insert("COMMIT", COMMIT);
	keywords_insert("ROLLBACK", ROLLBACK);
	keywords_insert("SAVEPOINT", SAVEPOINT);
	keywords_insert("RELEASE", RELEASE);
	keywords_insert("WORK", WORK);
	keywords_insert("CHAIN", CHAIN);
	keywords_insert("PRESERVE", PRESERVE);
	keywords_insert("ROWS", ROWS);
	keywords_insert("NO", NO);

}

void exit_keywords()
{
	int i;

	if (keywords_init_done == 0)
		return;
	keywords_init_done = 0;

	for (i = 0; i < HASH_SIZE; i++) {
		keyword *k = keywords[i];
		while (k) {
			keyword *l = k;
			k = k->next;
			_DELETE(l->keyword);
			_DELETE(l);
		}
	}
}

static keyword *find_keyword(char *yytext)
{
	int len = 0;
	int bucket = keyword_key(mkLower(yytext), &len) & HASH_MASK;
	keyword *k = keywords[bucket];
	while (k) {
		if (len == k->len) {
			if (len < 4) {
				if (*(short *) k->keyword ==
				    *(short *) yytext
				    && strcmp(k->keyword, yytext) == 0)
					return k;
			} else {
				if (*(int *) k->keyword == *(int *) yytext
				    && strcmp(k->keyword, yytext) == 0)
					return k;
			}
		}
		k = k->next;
	}
	return NULL;
}

int keyword_exists(char *yytext)
{
	if (find_keyword(yytext)){
		return 1;
	}
	return 0;
}

static int utf8_getchar(unsigned char *ch)
{
	int res = 0;
        if (*ch >= 0xF0) {
                (res)  = (*(ch)++ - 0xF0) << 18;
                (res) |= (*(ch)++ & 0x3F) << 12;
                (res) |= (*(ch)++ & 0x3F) << 6;
                (res) |= (*(ch)++ & 0x3F);
        } else if (*(ch) >= 0xE0) {
                (res)  = (*(ch)++ - 0xE0) << 12;
                (res) |= (*(ch)++ & 0x3F) << 6;
                (res) |= (*(ch)++ & 0x3F);
        } else if (*(ch) >= 0xC0) {
                (res)  = (*(ch)++ - 0xC0) << 6;
                (res) |= (*(ch)++ & 0x3F);
        } else {
                (res) = *(ch)++;
        }
	return res;
}

static void utf8_putchar(int ch, unsigned char *buf)
{
        if ((ch) < 0x80) {
                *(buf)++ = (ch);
        } else if ((ch) < 0x800) {
                *(buf)++ = 0xC0 | ((ch) >> 6);
                *(buf)++ = 0x80 | ((ch) & 0x3F);
        } else if ((ch) < 0x10000) {
                *(buf)++ = 0xE0 | ((ch) >> 12);
                *(buf)++ = 0x80 | (((ch) >> 6) & 0x3F);
                *(buf)++ = 0x80 | ((ch) & 0x3F);
        } else {
                *(buf)++ = 0xF0 | ((ch) >> 18);
                *(buf)++ = 0x80 | (((ch) >> 12) & 0x3F);
                *(buf)++ = 0x80 | (((ch) >> 6) & 0x3F);
                *(buf)++ = 0x80 | ((ch) & 0x3F);
        }
	buf = 0;
}

/* given the first char return how many char's fill a single utf8 char */
static int utf8_charlen(unsigned char ch)
{
	int res = 1;
        if (ch >= 0xF0) {
		res += 3;
        } else if (ch >= 0xE0) {
		res += 2;
        } else if (ch >= 0xC0) {
		res += 1;
	}
	return res;
}

static int utf8_intlen(int ch)
{
        if ((ch) < 0x80) {
		return 1;
        } else if ((ch) < 0x800) {
		return 2;
        } else if ((ch) < 0x10000) {
		return 3;
        } else {
		return 4;
        }
}

/* TODO change to block reads !*/
static int lex_getc(context * lc)
{
	int len = 0;
	unsigned char ch[5];
	int c = EOF;
	ch[4] = 0;
	if (lc->in) {
		*(int*)ch = 0;
		if (lc->in->read(lc->in, ch, 1, 1) == 1) {
			len = utf8_charlen(ch[0]) - 1;
			if (len && lc->in->read(lc->in, ch+1, len, 1) != 1) {
				fprintf(stderr, "couldn't read len %d\n", len);
			}
			c = utf8_getchar(ch);
		}
	} else if (lc->buf && *lc->buf) {
		len = utf8_charlen(lc->buf[0]);
		c = utf8_getchar(lc->buf);
		lc->buf += len;
	}
	if (c == '\n')
		lc->lineno++;
	if (c == EOT)
		c = EOF;
	return c;
}

void lex_append(context *lc, int ch)
{
	int len = utf8_intlen(ch);

	if (lc->yylen+len >= lc->yysize) { /* keep space for EOS */
		lc->yytext = realloc(lc->yytext, lc->yysize << 1);
		lc->yysize = lc->yysize << 1;
	}
	utf8_putchar(ch, lc->yytext+lc->yylen);
	lc->yylen += len;
	lc->yytext[lc->yylen] = 0;
}

int keyword_or_ident(context * lc)
{
	keyword *k = NULL;
	int cur = 0;
	
	assert(lc->yylen == 0);

	lex_append(lc, lc->cur);
	while ((cur = lex_getc(lc)) != EOF) {
		if ((isascii(cur) && !isalnum(cur)) && cur != '_') {
			k = find_keyword(lc->yytext);
			if (k) {
				lc->yyval = k->token;
			} else {
				lc->yyval = IDENT;
			}
			lc->cur = cur;
			return lc->yyval;
		}
		lex_append(lc, cur);
	}
	k = find_keyword(lc->yytext);
	if (k) {
		lc->yyval = k->token;
	} else {
		lc->yyval = IDENT;
	}
	lc->cur = cur;
	return lc->yyval;
}

void skip_white_space(context * lc)
{
	int cur;
	while ((cur = lex_getc(lc)) != EOF && isspace(cur));
	lc->cur = cur;
}


int handle_error(context * lc)
{
	switch (lc->cur) {
	case EOF:
		(void)sql_error( lc, 1, "Unexpected end of file");
		break;
	}
	return -1;
}

int skip_c_comment(context * lc)
{
	int cur;
	int prev = 0;
	while ((cur = lex_getc(lc)) != EOF && !(cur == '/' && prev == '*')) {
		prev = cur;
	}
	if (cur == '/')
		lc->cur = lex_getc(lc);
	return lc->cur;
}

int skip_sql_comment(context * lc)
{
	int cur;
	while ((cur = lex_getc(lc)) != EOF && (cur != '\n'));
	if (cur == '\n')
		lc->cur = lex_getc(lc);
	return lc->cur;
}

int context_yychar(context * lc, int yychar)
{
	lc->yytext[0] = (char) yychar;
	lc->yytext[1] = 0;
	lc->yyval = yychar;
	lc->yylen = 1;
	return lc->yyval;
}

int context_yy2char(context * lc, int yychar, int yyval)
{
	lc->yytext[0] = (char) yychar;
	lc->yytext[1] = (char) yychar;
	lc->yytext[2] = 0;
	lc->yyval = yyval;
	lc->yylen = 2;
	return lc->yyval;
}

int context_comparison(context * lc, char *yychar)
{
	int len = strlen(yychar);
	strncpy(lc->yytext, yychar, len);
	lc->yytext[len] = '\0';
	lc->yyval = COMPARISON;
	lc->yylen = len;
	return lc->yyval;
}

int lex_string(context * lc, int quote)
{
	int cur = 0;
	int escape = 0;

	assert(lc->yylen == 0);

	while ((cur = lex_getc(lc)) != EOF && (cur != quote || escape)) {
		if (!escape && cur == '\\')
			escape = 1;
		else
			escape = 0;
		lex_append(lc, cur);
	}
	lc->yyval = STRING;
	lc->cur = lex_getc(lc);
	return lc->yyval;
}

static int tokenize(context * lc);

int number(context * lc, int len)
{
	char *yytext = lc->yytext;
	int token = sqlINT;
	int cur = 0;
	int yylen = len;
	int yysz = lc->yysize;
	/* todo start with . */

	yytext[len - 1] = lc->cur;

	while ((cur = lex_getc(lc)) != EOF && isdigit(cur)) {
		if (yylen == yysz) {
			yytext = realloc(yytext, yysz << 1);
			yysz = yysz << 1;
		}
		yytext[yylen++] = (char) cur;
	}
	if (cur == '.') {
		token = INTNUM;
		if (yylen == yysz) {
			yytext = realloc(yytext, yysz << 1);
			yysz = yysz << 1;
		}
		yytext[yylen++] = (char) cur;
		while ((cur = lex_getc(lc)) != EOF && isdigit(cur)) {
			if (yylen == yysz) {
				yytext = realloc(yytext, yysz << 1);
				yysz = yysz << 1;
			}
			yytext[yylen++] = (char) cur;
		}
		if (cur == 'e' || cur == 'E') {
			token = APPROXNUM;
			if (yylen == yysz) {
				yytext = realloc(yytext, yysz << 1);
				yysz = yysz << 1;
			}
			yytext[yylen++] = (char) cur;
			cur = lex_getc(lc);
			if (lc->cur == '-' || lc->cur == '+') {
				token = 0;
				if (yylen == yysz) {
					yytext =
					    realloc(yytext, yysz << 1);
					yysz = yysz << 1;
				}
				yytext[yylen++] = (char) lex_getc(lc);
			}
			while ((cur = lex_getc(lc)) != EOF && isdigit(cur)) {
				token = APPROXNUM;
				if (yylen == yysz) {
					yytext =
					    realloc(yytext, yysz << 1);
					yysz = yysz << 1;
				}
				yytext[yylen++] = (char) cur;
			}
		}
	}
	yytext[yylen] = 0;
	if (token) {
		lc->yyval = token;
	} else if (yylen == 1) {
		if (yytext[0] == '-') {
			if (cur == '-') {
				if (skip_sql_comment(lc) == EOF)
					return lc->cur;
				return tokenize(lc);
			} else {
				lc->cur = cur;
				return context_yychar(lc, yytext[0]);
			}
		} else if (yytext[0] == '+') {
			lc->cur = cur;
			return context_yychar(lc, yytext[0]);
		}
	} else {
		(void)sql_error( lc, 2, "Unexpected symbol %c", lc->cur);
		return -1;
	}
	lc->yytext = yytext;
	lc->yylen = yylen;
	lc->yysize = yysz;

	lc->cur = cur;
	return lc->yyval;
}


int lex_symbol(context * lc)
{
	int cur = lc->cur;
	int next = 0;

	if (cur == '/') {
		next = lex_getc(lc);
		if (next == '*') {
			skip_c_comment(lc);
			return tokenize(lc);
		} else {
			lc->cur = next;
			return context_yychar(lc, cur);
		}
	} else if (isdigit(cur)) {
		lc->cur = cur;
		return number(lc, 1);
	} else if (cur == '#') {
		if (skip_sql_comment(lc) == EOF)
			return lc->cur;
		return tokenize(lc);
	} else if (cur == '\'' || cur == '"') {
		return lex_string(lc, cur);
	}
	switch (cur) {
	case '-':
		next = lex_getc(lc);
		if (next == '-') {
			if (skip_sql_comment(lc) == EOF)
				return lc->cur;
			return tokenize(lc);
		}
		lc->cur = next;
		return context_yychar(lc, cur);
	case '+':
	case '*':
	case '/':
	case '(':
	case ')':
	case ',':
	case '.':
	case ';':
	case '=':
		lc->cur = lex_getc(lc);
		return context_yychar(lc, cur);
	case '<':
		lc->cur = lex_getc(lc);
		if (lc->cur == '=') {
			lc->cur = lex_getc(lc);
			return context_comparison(lc, "<=");
		} else if (lc->cur == '>') {
			lc->cur = lex_getc(lc);
			return context_comparison(lc, "<>");
		} else {
			return context_comparison(lc, "<");
		}
	case '>':
		lc->cur = lex_getc(lc);
		if (lc->cur != '=') {
			return context_comparison(lc, ">");
		} else {
			lc->cur = lex_getc(lc);
			return context_comparison(lc, ">=");
		}
	case '|':
		lc->cur = lex_getc(lc);
		if (lc->cur == '|') {
			return context_yy2char(lc, cur, CONCATSTRING);
		}
	}
	(void)sql_error( lc, 3, "Unexpected symbol %c", lc->cur);
	return -1;
}

int tokenize(context * lc)
{
	while (1) {
		if (isspace(lc->cur)) {
			skip_white_space(lc);
			if (lc->cur == EOF)
				return lc->cur;
		} else if (isdigit(lc->cur)) {
			return number(lc, 1);
		} else if (isalpha(lc->cur) || lc->cur == '_') {
			return keyword_or_ident(lc);
		} else if (ispunct(lc->cur)) {
			return lex_symbol(lc);
		}
		if (lc->cur == EOF)
			return handle_error(lc);
	}
}

int sqllex(YYSTYPE * yylval, void *parm)
{
	context *lc = (context *) parm;
	int token = lc->prev;

	lc->prev = 0;

	if (!token) {
		lc->yylen = 0;
		lc->yytext[0] = 0;

		token = tokenize(lc);
		yylval->sval = lc->yytext;

		if (token == UNION) {
			token = tokenize(lc);
			yylval->sval = lc->yytext;
			if (token == JOIN) {
				sql_statement_add(lc, "UNION JOIN");
				return UNIONJOIN;
			} else {
				lc->prev = token;
				yylval->sval = "UNION";
				sql_statement_add(lc, "UNION");
				sql_statement_add(lc, lc->yytext);
				return UNION;
			}
		} else {
			if (token == IDENT || token == COMPARISON ||
			    token == STRING || token == AGGR || 
			    token == TYPE)
				yylval->sval = _strdup(lc->yytext);

			if (token == STRING)
				sql_statement_add(lc, "'");

			sql_statement_add(lc, lc->yytext);

			if (token == STRING)
				sql_statement_add(lc, "'");
		}
	}
	return token;
}

int parse_error(context * lc, char *err)
{
	(void)sql_error( lc, 4, 
		 "%s(%d) %s at token (%d): %s\n in statement: %s",
		 lc->filename, lc->lineno, err, lc->yyval, lc->yytext, lc->sql);
	return 1;
}
