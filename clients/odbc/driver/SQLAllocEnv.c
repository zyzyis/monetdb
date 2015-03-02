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
 * SQLAllocEnv()
 * CLI Compliance: deprecated in ODBC 3.0 (replaced by SQLAllocHandle())
 * Provided here for old (pre ODBC 3.0) applications and driver managers.
 *
 * Author: Martin van Dinther, Sjoerd Mullender
 * Date  : 30 Aug 2002
 *
 **********************************************************************/

#include "ODBCGlobal.h"
#include "ODBCEnv.h"

SQLRETURN SQL_API
SQLAllocEnv(SQLHENV *OutputHandlePtr)
{
	SQLRETURN r;

#ifdef ODBCDEBUG
	ODBCLOG("SQLAllocEnv\n");
#endif

	/* use mapping as described in ODBC 3 SDK Help file */
	r = MNDBAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE,
			    (SQLHANDLE *) OutputHandlePtr);
	if (SQL_SUCCEEDED(r)) {
		/* ODBC 3.x applications never call this interface, so
		 * this is an ODBC 2.x application */
		((ODBCEnv *) *OutputHandlePtr)->sql_attr_odbc_version = SQL_OV_ODBC2;
	}
	return r;
}
