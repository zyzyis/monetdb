/*
 * The contents of this file are subject to the MonetDB Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.monetdb.org/Legal/MonetDBLicense
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is the MonetDB Database System.
 *
 * The Initial Developer of the Original Code is CWI.
 * Portions created by CWI are Copyright (C) 1997-July 2008 CWI.
 * Copyright August 2008-2015 MonetDB B.V.
 * All Rights Reserved.
 */

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
 * SQLGetStmtOption()
 * CLI Compliance: deprecated in ODBC 3.0 (replaced by SQLGetStmtAttr())
 * Provided here for old (pre ODBC 3.0) applications and driver managers.
 *
 * Author: Martin van Dinther, Sjoerd Mullender
 * Date  : 30 aug 2002
 *
 **********************************************************************/

#include "ODBCGlobal.h"
#include "ODBCStmt.h"

SQLRETURN SQL_API
SQLGetStmtOption(SQLHSTMT StatementHandle,
		 SQLUSMALLINT Option,
		 SQLPOINTER ValuePtr)
{
	ODBCStmt *stmt = (ODBCStmt *) StatementHandle;
	SQLULEN v;
	SQLRETURN r;

#ifdef ODBCDEBUG
	ODBCLOG("SQLGetStmtOption " PTRFMT " %s\n",
		PTRFMTCAST StatementHandle, translateStmtOption(Option));
#endif

	if (!isValidStmt(stmt))
		 return SQL_INVALID_HANDLE;

	clearStmtErrors(stmt);

	/* only the ODBC 1.0 and ODBC 2.0 options */
	switch (Option) {
	case SQL_ASYNC_ENABLE:
	case SQL_CONCURRENCY:
	case SQL_CURSOR_TYPE:
	case SQL_NOSCAN:
	case SQL_QUERY_TIMEOUT:
	case SQL_RETRIEVE_DATA:
	case SQL_SIMULATE_CURSOR:
	case SQL_USE_BOOKMARKS:
	case SQL_ROW_NUMBER:
		/* SQLGetStmtAttr returns 64 bit value, but we need to
		 * return 32 bit value */
		r = SQLGetStmtAttr_(stmt, Option, &v, 0, NULL);
		if (SQL_SUCCEEDED(r))
			*(SQLUINTEGER *) ValuePtr = (SQLUINTEGER) v;
		return r;
	case SQL_BIND_TYPE:
	case SQL_KEYSET_SIZE:
	case SQL_MAX_LENGTH:
	case SQL_MAX_ROWS:
	case SQL_ROWSET_SIZE:
/*		case SQL_GET_BOOKMARKS:	is deprecated in ODBC 3.0+ */
		/* use mapping as described in ODBC 3.0 SDK Help */
		return SQLGetStmtAttr_(stmt, Option, ValuePtr, 0, NULL);
	default:
		/* Invalid attribute/option identifier */
		addStmtError(stmt, "HY092", NULL, 0);
		break;
	}

	return SQL_ERROR;
}
