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
 * SQLProcedureColumns()
 * CLI Compliance: ODBC (Microsoft)
 *
 * Note: this function is not implemented (it only sets an error),
 * because MonetDB SQL frontend does not support stored procedures.
 *
 * Author: Martin van Dinther
 * Date  : 30 aug 2002
 *
 **********************************************************************/

#include "ODBCGlobal.h"
#include "ODBCStmt.h"
#include "ODBCUtil.h"


static SQLRETURN
SQLProcedureColumns_(ODBCStmt *stmt,
		     SQLCHAR *szCatalogName, SQLSMALLINT nCatalogNameLength,
		     SQLCHAR *szSchemaName, SQLSMALLINT nSchemaNameLength,
		     SQLCHAR *szProcName, SQLSMALLINT nProcNameLength,
		     SQLCHAR *szColumnName, SQLSMALLINT nColumnNameLength)
{
	fixODBCstring(szCatalogName, nCatalogNameLength, addStmtError, stmt);
	fixODBCstring(szSchemaName, nSchemaNameLength, addStmtError, stmt);
	fixODBCstring(szProcName, nProcNameLength, addStmtError, stmt);
	fixODBCstring(szColumnName, nColumnNameLength, addStmtError, stmt);

#ifdef ODBCDEBUG
	ODBCLOG("\"%.*s\" \"%.*s\" \"%.*s\" \"%.*s\"\n",
		nCatalogNameLength, szCatalogName,
		nSchemaNameLength, szSchemaName,
		nProcNameLength, szProcName,
		nColumnNameLength, szColumnName);
#endif

	/* SQLProcedureColumns returns a table with the following columns:
	   VARCHAR	procedure_cat
	   VARCHAR	procedure_schem
	   VARCHAR	procedure_name NOT NULL
	   VARCHAR	column_name NOT NULL
	   SMALLINT	column_type NOT NULL
	   SMALLINT	data_type NOT NULL
	   VARCHAR	type_name NOT NULL
	   INTEGER	column_size
	   INTEGER	buffer_length
	   SMALLINT	decimal_digits
	   SMALLINT	num_prec_radix
	   SMALLINT	nullable NOT NULL
	   VARCHAR	remarks
	   VARCHAR	column_def
	   SMALLINT	sql_data_type NOT NULL
	   SMALLINT	sql_datetime_sub
	   INTEGER	char_octet_length
	   INTEGER	ordinal_position NOT NULL
	   VARCHAR	is_nullable
	*/

	/* Driver does not support this function */
	addStmtError(stmt, "IM001", NULL, 0);

	return SQL_ERROR;
}

SQLRETURN SQL_API
SQLProcedureColumns(SQLHSTMT hStmt,
		    SQLCHAR *szCatalogName, SQLSMALLINT nCatalogNameLength,
		    SQLCHAR *szSchemaName, SQLSMALLINT nSchemaNameLength,
		    SQLCHAR *szProcName, SQLSMALLINT nProcNameLength,
		    SQLCHAR *szColumnName, SQLSMALLINT nColumnNameLength)
{
	ODBCStmt *stmt = (ODBCStmt *) hStmt;

#ifdef ODBCDEBUG
	ODBCLOG("SQLProcedureColumns " PTRFMT " ", PTRFMTCAST hStmt);
#endif

	if (!isValidStmt(stmt))
		 return SQL_INVALID_HANDLE;

	clearStmtErrors(stmt);

	return SQLProcedureColumns_(stmt,
				    szCatalogName, nCatalogNameLength,
				    szSchemaName, nSchemaNameLength,
				    szProcName, nProcNameLength,
				    szColumnName, nColumnNameLength);
}

#ifdef WITH_WCHAR
SQLRETURN SQL_API
SQLProcedureColumnsW(SQLHSTMT hStmt,
		     SQLWCHAR *szCatalogName, SQLSMALLINT nCatalogNameLength,
		     SQLWCHAR *szSchemaName, SQLSMALLINT nSchemaNameLength,
		     SQLWCHAR *szProcName, SQLSMALLINT nProcNameLength,
		     SQLWCHAR *szColumnName, SQLSMALLINT nColumnNameLength)
{
	ODBCStmt *stmt = (ODBCStmt *) hStmt;
	SQLRETURN rc = SQL_ERROR;
	SQLCHAR *catalog = NULL, *schema = NULL, *proc = NULL, *column = NULL;

#ifdef ODBCDEBUG
	ODBCLOG("SQLProcedureColumnsW " PTRFMT " ", PTRFMTCAST hStmt);
#endif

	if (!isValidStmt(stmt))
		 return SQL_INVALID_HANDLE;

	clearStmtErrors(stmt);

	fixWcharIn(szCatalogName, nCatalogNameLength, catalog, addStmtError, stmt, goto exit);
	fixWcharIn(szSchemaName, nSchemaNameLength, schema, addStmtError, stmt, goto exit);
	fixWcharIn(szProcName, nProcNameLength, proc, addStmtError, stmt, goto exit);
	fixWcharIn(szColumnName, nColumnNameLength, column, addStmtError, stmt, goto exit);

	rc = SQLProcedureColumns_(stmt, catalog, SQL_NTS, schema, SQL_NTS,
				  proc, SQL_NTS, column, SQL_NTS);

  exit:
	if (catalog)
		free(catalog);
	if (schema)
		free(schema);
	if (proc)
		free(proc);
	if (column)
		free(column);

	return rc;
}
#endif	/* WITH_WCHAR */
