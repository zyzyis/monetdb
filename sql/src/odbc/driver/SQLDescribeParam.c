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
 * SQLDescribeParam()
 * CLI Compliance: ODBC (Microsoft)
 *
 * Note: this function is not supported (yet), it returns an error.
 *
 * Author: Martin van Dinther
 * Date  : 30 aug 2002
 *
 **********************************************************************/

#include "ODBCGlobal.h"
#include "ODBCStmt.h"

SQLRETURN
SQLDescribeParam(SQLHSTMT hStmt, SQLUSMALLINT nParmNumber,
		 SQLSMALLINT *pnDataType, SQLUINTEGER *pnSize,
		 SQLSMALLINT *pnDecDigits, SQLSMALLINT *pnNullable)
{
	ODBCStmt *stmt = (ODBCStmt *) hStmt;
	ODBCDescRec *rec;

#ifdef ODBCDEBUG
	ODBCLOG("SQLDescribeParam\n");
#endif

	if (!isValidStmt(stmt))
		 return SQL_INVALID_HANDLE;

	clearStmtErrors(stmt);

	/* check statement cursor state, query should be prepared or executed */
	if (stmt->State == INITED) {
		/* HY010 = Function sequence error */
		addStmtError(stmt, "HY010", NULL, 0);
		return SQL_ERROR;
	}

	if (nParmNumber < 1 ||
	    nParmNumber > stmt->ImplRowDescr->sql_desc_count) {
		addStmtError(stmt, "07009", NULL, 0);
		return SQL_ERROR;
	}

	rec = &stmt->ImplRowDescr->descRec[nParmNumber];

	if (pnDataType)
		*pnDataType = rec->sql_desc_concise_type;

	if (pnNullable)
		*pnNullable = rec->sql_desc_nullable;

	/* also see SQLDescribeCol */
	if (pnSize)
		*pnSize = ODBCDisplaySize(rec);

	/* also see SQLDescribeCol */
	if (pnDecDigits) {
		switch (rec->sql_desc_concise_type) {
		case SQL_DECIMAL:
		case SQL_NUMERIC:
			*pnDecDigits = rec->sql_desc_scale;
			break;
		case SQL_BIT:
		case SQL_TINYINT:
		case SQL_SMALLINT:
		case SQL_INTEGER:
		case SQL_BIGINT:
			*pnDecDigits = 0;
			break;
		case SQL_TYPE_TIME:
		case SQL_TYPE_TIMESTAMP:
		case SQL_INTERVAL_SECOND:
		case SQL_INTERVAL_DAY_TO_SECOND:
		case SQL_INTERVAL_HOUR_TO_SECOND:
		case SQL_INTERVAL_MINUTE_TO_SECOND:
			*pnDecDigits = rec->sql_desc_precision;
			break;
		}
	}

	return SQL_SUCCESS;
}
