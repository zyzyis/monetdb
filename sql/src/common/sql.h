#ifndef _SQL_H_
#define _SQL_H_

#include <stdio.h>
#include <stdarg.h>
#include "list.h"
#include "catalog.h"
#include "context.h"
#include "symbol.h"

#define _(String) (String)
#define N_(String) (String)

extern char *toLower(char *v);
extern char *removeQuotes(char *v, char Q);
extern char *addQuotes(char *v);

typedef enum tokens {
	SQL_CREATE_SCHEMA,
	SQL_CREATE_TABLE,
	SQL_CREATE_VIEW,
	SQL_DROP_SCHEMA,
	SQL_DROP_TABLE,
	SQL_DROP_VIEW,
	SQL_ALTER_TABLE,
	SQL_NAME,
	SQL_USER,
	SQL_PATH,
	SQL_CHARSET,
	SQL_TABLE,
	SQL_COLUMN,
	SQL_COLUMN_OPTIONS,
	SQL_CONSTRAINT,
	SQL_CHECK,
	SQL_DEFAULT,
	SQL_NOT_NULL,
	SQL_NULL,
	SQL_UNIQUE,
	SQL_PRIMARY_KEY,
	SQL_FOREIGN_KEY,
	SQL_COMMIT,
	SQL_ROLLBACK,
	SQL_INSERT,
	SQL_DELETE,
	SQL_CROSS,
	SQL_JOIN,
	SQL_SELECT,
	SQL_WHERE,
	SQL_FROM,
	SQL_UNION,
	SQL_UNION_ALL,
	SQL_UPDATE_SET,
	SQL_INSERT_INTO,
	SQL_VALUES,
	SQL_ASSIGN,
	SQL_ORDERBY,
	SQL_GROUPBY,
	SQL_HAVING,
	SQL_DESC,
	SQL_AND,
	SQL_OR,
	SQL_NOT,
	SQL_EXISTS,
	SQL_UNOP,
	SQL_BINOP,
	SQL_BETWEEN,
	SQL_NOT_BETWEEN,
	SQL_LIKE,
	SQL_NOT_LIKE,
	SQL_IN,
	SQL_NOT_IN,
	SQL_GRANT,
	SQL_PARAMETER,
	SQL_AMMSC,
	SQL_COMPARE,
	SQL_TEMP_LOCAL,
	SQL_TEMP_GLOBAL,
	SQL_INT_VALUE,
	SQL_ATOM,
	SQL_USING,
	SQL_ESCAPE,
} tokens;

extern char *token2string( int token );

extern list *semantic( context *sql, dlist *sqllist );

#endif /*_SQL_H_*/
