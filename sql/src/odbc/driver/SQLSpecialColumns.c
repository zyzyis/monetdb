/*
 * This code was created by Peter Harvey (mostly during Christmas 98/99).
 * This code is LGPL. Please ensure that this message remains in future
 * distributions and uses of this code (thats about all I get out of it).
 * - Peter Harvey pharvey@codebydesign.com
 * 
 * This file has been modified for the MonetDB project.  See the file
 * Copyright in this directory for more information.
 */

/**********************************************************************
 * SQLSpecialColumns()
 * CLI Compliance: X/Open
 *
 * Note: catalogs are not supported, we ignore any value set for
 * szCatalogName.
 *
 * Author: Martin van Dinther
 * Date  : 30 aug 2002
 *
 **********************************************************************/

#include "ODBCGlobal.h"
#include "ODBCStmt.h"
#include "ODBCUtil.h"


static SQLRETURN
SQLSpecialColumns_(ODBCStmt *stmt, SQLUSMALLINT nIdentifierType,
		   SQLCHAR *szCatalogName, SQLSMALLINT nCatalogNameLength,
		   SQLCHAR *szSchemaName, SQLSMALLINT nSchemaNameLength,
		   SQLCHAR *szTableName, SQLSMALLINT nTableNameLength,
		   SQLUSMALLINT nScope, SQLUSMALLINT nNullable)
{
	RETCODE rc;

	/* buffer for the constructed query to do meta data retrieval */
	char *query = NULL;
	char *query_end = NULL;

	fixODBCstring(szCatalogName, nCatalogNameLength, addStmtError, stmt);
	fixODBCstring(szSchemaName, nSchemaNameLength, addStmtError, stmt);
	fixODBCstring(szTableName, nTableNameLength, addStmtError, stmt);

#ifdef ODBCDEBUG
	ODBCLOG("\"%.*s\" \"%.*s\" \"%.*s\" %d %d\n",
		nCatalogNameLength, szCatalogName,
		nSchemaNameLength, szSchemaName, nTableNameLength, szTableName,
		nScope, nNullable);
#endif

	/* check statement cursor state, no query should be prepared
	   or executed */
	if (stmt->State == EXECUTED) {
		/* 24000 = Invalid cursor state */
		addStmtError(stmt, "24000", NULL, 0);
		return SQL_ERROR;
	}

	/* check for valid IdentifierType argument */
	switch (nIdentifierType) {
	case SQL_BEST_ROWID:
	case SQL_ROWVER:
		break;
	default:
		/* HY097 = Invalid identifier type */
		addStmtError(stmt, "HY097", NULL, 0);
		return SQL_ERROR;
	}

	/* check for valid Scope argument */
	switch (nScope) {
	case SQL_SCOPE_CURROW:
	case SQL_SCOPE_TRANSACTION:
	case SQL_SCOPE_SESSION:
		break;
	default:
		/* HY098 = Invalid scope type */
		addStmtError(stmt, "HY098", NULL, 0);
		return SQL_ERROR;
	}

	/* check for valid Nullable argument */
	switch (nNullable) {
	case SQL_NO_NULLS:
	case SQL_NULLABLE:
		break;
	default:
		/* HY099 = Invalid nullable type */
		addStmtError(stmt, "HY099", NULL, 0);
		return SQL_ERROR;
	}

	/* check if a valid (non null, not empty) table name is supplied */
	if (szTableName == NULL) {
		/* HY009 = Invalid use of null pointer */
		addStmtError(stmt, "HY009", NULL, 0);
		return SQL_ERROR;
	}
	if (nTableNameLength == 0) {
		/* HY090 = Invalid string */
		addStmtError(stmt, "HY090", NULL, 0);
		return SQL_ERROR;
	}

	/* first create a string buffer (1000 extra bytes is plenty */
	query = (char *) malloc(1000 + nTableNameLength + nSchemaNameLength);
	assert(query);
	query_end = query;

	/* SQLSpecialColumns returns a table with the following columns:
	   SMALLINT	scope
	   VARCHAR	column_name NOT NULL
	   SMALLINT	data_type NOT NULL
	   VARCHAR	type_name NOT NULL
	   INTEGER	column_size
	   INTEGER	buffer_length
	   SMALLINT	decimal_digits
	   SMALLINT	pseudo_column
	 */
	if (nIdentifierType == SQL_BEST_ROWID) {
		/* Select from the key table the (smallest) primary/unique key */
		/* Note: SCOPE is SQL_SCOPE_TRANSACTION is 1 */
		/* Note: PSEUDO_COLUMN is SQL_PC_NOT_PSEUDO is 1 */
		strcpy(query_end,
		       "select "
		       "cast(1 as smallint) as scope, "
		       "cast(c.name as varchar) as column_name, "
		       "cast(c.type as smallint) as data_type, "
		       "cast(c.type_name as varchar) as type_name, "
		       "cast(c.column_size as integer) as column_size, "
		       "cast(c.buffer_length as integer) as buffer_length, "
		       "cast(c.decimal_digits as smallint) as decimal_digits, "
		       "cast(1 as smallint) as pseudo_column "
		       "from sys.schemas s, sys.tables t, "
		       " columns c, keys k, keycolumns kc "
		       "where s.id = t.schema_id and t.id = c.table_id and "
		       " t.id = k.table_id and c.id = kc.'column' and "
		       " kc.id = k.id" /*" and k.is_primary = 1"*/);
		query_end += strlen(query_end);
		/* TODO: improve the SQL to get the correct result:
		   - only one set of columns should be returned, also when
		   multiple primary keys are available for this table.
		   - when the table has NO primary key it should return the
		   columns of a unique key (only from ONE unique key which
		   is also the best/smallest key)
		   TODO: optimize SQL:
		   - when no szSchemaName is set (see above) also no filtering on
		   SCHEMA NAME and join with table SCHEMAS is needed!
		 */

		/* add the selection condition */
		/* search pattern is not allowed for table name so use = and not LIKE */
		sprintf(query_end, " and t.name = '%.*s'",
			nTableNameLength, szTableName);
		query_end += strlen(query_end);

		if (szSchemaName != NULL && nSchemaNameLength > 0) {
			/* filtering requested on schema name */
			/* search pattern is not allowed so use = and not LIKE */
			sprintf(query_end, " and s.name = '%.*s'",
				nSchemaNameLength, szSchemaName);
			query_end += strlen(query_end);
		}

		/* add an extra selection when SQL_NO_NULLS is requested */
		if (nNullable == SQL_NO_NULLS) {
			strcpy(query_end, " and c.is_nullable = 0");
			query_end += strlen(query_end);
		}

		/* no ordering needed */
	} else {
		assert(nIdentifierType == SQL_ROWVER);
		/* The backend does not have such info available */
		/* create just a query which results in zero rows */
		/* Note: pseudo_column is sql_pc_unknown is 0 */
		strcpy(query_end,
		       "select "
		       "cast(null as smallint) as scope, "
		       "'' as column_name, "
		       "cast(1 as smallint) as data_type, "
		       "'char' as type_name, "
		       "1 as column_size, "
		       "1 as buffer_length, "
		       "cast(0 as smallint) as decimal_digits, "
		       "cast(0 as smallint) as pseudo_column "
		       "from sys.schemas s where 0 = 1");
		query_end += strlen(query_end);
	}

	/* query the MonetDB data dictionary tables */
	rc = SQLExecDirect_(stmt, (SQLCHAR *) query,
			    (SQLINTEGER) (query_end - query));

	free(query);

	return rc;
}

SQLRETURN SQL_API
SQLSpecialColumns(SQLHSTMT hStmt, SQLUSMALLINT nIdentifierType,
		  SQLCHAR *szCatalogName, SQLSMALLINT nCatalogNameLength,
		  SQLCHAR *szSchemaName, SQLSMALLINT nSchemaNameLength,
		  SQLCHAR *szTableName, SQLSMALLINT nTableNameLength,
		  SQLUSMALLINT nScope, SQLUSMALLINT nNullable)
{
	ODBCStmt *stmt = (ODBCStmt *) hStmt;

#ifdef ODBCDEBUG
	ODBCLOG("SQLSpecialColumns " PTRFMT " %d ", PTRFMTCAST hStmt,
		nIdentifierType);
#endif

	if (!isValidStmt(stmt))
		 return SQL_INVALID_HANDLE;

	clearStmtErrors(stmt);

	return SQLSpecialColumns_(stmt, nIdentifierType,
				  szCatalogName, nCatalogNameLength,
				  szSchemaName, nSchemaNameLength,
				  szTableName, nTableNameLength,
				  nScope, nNullable);
}

#ifdef WITH_WCHAR
SQLRETURN SQL_API
SQLSpecialColumnsW(SQLHSTMT hStmt, SQLUSMALLINT nIdentifierType,
		   SQLWCHAR *szCatalogName, SQLSMALLINT nCatalogNameLength,
		   SQLWCHAR *szSchemaName, SQLSMALLINT nSchemaNameLength,
		   SQLWCHAR *szTableName, SQLSMALLINT nTableNameLength,
		   SQLUSMALLINT nScope, SQLUSMALLINT nNullable)
{
	ODBCStmt *stmt = (ODBCStmt *) hStmt;
	SQLRETURN rc = SQL_ERROR;
	SQLCHAR *catalog = NULL, *schema = NULL, *table = NULL;

#ifdef ODBCDEBUG
	ODBCLOG("SQLSpecialColumnsW " PTRFMT " %d ", PTRFMTCAST hStmt,
		nIdentifierType);
#endif

	if (!isValidStmt(stmt))
		 return SQL_INVALID_HANDLE;

	clearStmtErrors(stmt);

	fixWcharIn(szCatalogName, nCatalogNameLength, catalog, addStmtError, stmt, goto exit);
	fixWcharIn(szSchemaName, nSchemaNameLength, schema, addStmtError, stmt, goto exit);
	fixWcharIn(szTableName, nTableNameLength, table, addStmtError, stmt, goto exit);

	rc = SQLSpecialColumns_(stmt, nIdentifierType, catalog, SQL_NTS,
				schema, SQL_NTS, table, SQL_NTS,
				nScope, nNullable);

  exit:
	if (catalog)
		free(catalog);
	if (schema)
		free(schema);
	if (table)
		free(table);

	return rc;
}
#endif	/* WITH_WCHAR */
