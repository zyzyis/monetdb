/*
 * The contents of this file are subject to the MonetDB Public
 * License Version 1.0 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at
 * http://monetdb.cwi.nl/Legal/MonetDBPL-1.0.html
 *
 * Software distributed under the License is distributed on an
 * "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code is the Monet Database System.
 *
 * The Initial Developer of the Original Code is CWI.
 * Portions created by CWI are Copyright (C) 1997-2002 CWI.
 * All Rights Reserved.
 *
 * Contributor(s):
 * 		Martin Kersten  <Martin.Kersten@cwi.nl>
 * 		Peter Boncz  <Peter.Boncz@cwi.nl>
 * 		Niels Nes  <Niels.Nes@cwi.nl>
 * 		Stefan Manegold  <Stefan.Manegold@cwi.nl>
 */

/**********************************************************************
 * SQLConnect()
 * CLI Compliance: ISO 92
 *
 * Author: Martin van Dinther
 * Date  : 30 aug 2002
 *
 **********************************************************************/

#include "ODBCGlobal.h"
#include "ODBCDbc.h"
#include "ODBCUtil.h"

SQLRETURN SQLConnect(
	SQLHDBC		hDbc,
	SQLCHAR *	szDataSource,
	SQLSMALLINT	nDataSourceLength,
	SQLCHAR *	szUID,
	SQLSMALLINT	nUIDLength,
	SQLCHAR *	szPWD,
	SQLSMALLINT	nPWDLength )
{
	ODBCDbc * dbc = (ODBCDbc *) hDbc;
	SQLRETURN rc = SQL_SUCCESS;
	char * dsn = NULL;
	char * uid = NULL;
	char * pwd = NULL;
	char * database = NULL;
	char * host = NULL;
	int port = 0;
	int debug = 0;
	char buf[BUFSIZ + 1];
	char ODBC_INI[] = "ODBC.INI";
	int socket_fd = 0;

	if (! isValidDbc(dbc))
		return SQL_INVALID_HANDLE;

	clearDbcErrors(dbc);

	/* check connection state, should not be connected */
	if (dbc->Connected == 1) {
		/* 08002 = Connection already in use */
		addDbcError(dbc, "08002", NULL, 0);
		return SQL_ERROR;
	}
	assert(dbc->Connected == 0);


	/* convert input string parameters to normal null terminated C strings */
	dsn = copyODBCstr2Cstr(szDataSource, nDataSourceLength);
	if (dsn == NULL) {
		/* IM002 = Datasource not found */
		addDbcError(dbc, "IM002", NULL, 0);
		return SQL_ERROR;
	}

	uid = copyODBCstr2Cstr(szUID, nUIDLength);
	if (uid == NULL) {
		SQLGetPrivateProfileString(dsn, "UID", "", buf, BUFSIZ, ODBC_INI);
		uid = GDKstrdup(buf);
	}
	pwd = copyODBCstr2Cstr(szPWD, nPWDLength);
	if (uid == NULL) {
		SQLGetPrivateProfileString(dsn, "PWD", "", buf, BUFSIZ, ODBC_INI);
		pwd = GDKstrdup(buf);
	}

	/* get the other information from the ODBC.INI file */
	SQLGetPrivateProfileString(dsn, "DATABASE", "", buf, BUFSIZ, ODBC_INI);
	database = GDKstrdup(buf);
	SQLGetPrivateProfileString(dsn, "HOST", "localhost", buf, BUFSIZ, ODBC_INI);
	host = GDKstrdup(buf);
	SQLGetPrivateProfileString(dsn, "PORT", "0", buf, BUFSIZ, ODBC_INI);
	port = atoi(buf);
	SQLGetPrivateProfileString(dsn, "FLAG", "0", buf, BUFSIZ, ODBC_INI);
	debug = atoi(buf);


	/* Retrieved and checked the arguments.
	   Now try to open a connection with the server */
	/* connect to a server on host via port */
	socket_fd = client(host, port);
	if (socket_fd > 0) {
		stream * rs = NULL;
		stream * ws = NULL;
		context * lc = NULL;
		int i;
		char *schema,*login;

		rs = block_stream(socket_rstream(socket_fd, "sql client read"));
		ws = block_stream(socket_wstream(socket_fd, "sql client write"));

		printf("%d\n", debug);
		i = snprintf(buf, BUFSIZ, "api(milsql,%d);\n", debug );
		ws->write( ws, buf, i, 1 );
		ws->flush( ws );
		/* read login */
		login = readblock( rs );
	
		printf("login(%s,%s)\n", uid, pwd);
		i = snprintf(buf, BUFSIZ, "login(%s,%s);\n", uid, pwd );
		ws->write( ws, buf, i, 1 );
		ws->flush( ws );
		/* read schema */
		schema = readblock( rs );
		if (schema){
			char *s = strrchr(schema, '\n');
			if (s) *s = '\0';
		}
		if (strlen(schema) > 0){
			lc = &dbc->Mlc;
			memset(lc, 0, sizeof(context));
			sql_init_context(lc, ws, debug, default_catalog_create());
			catalog_create_stream(rs, lc);
			lc->cat->cc_getschemas(lc->cat, schema, uid);
			lc->cur = ' ';
			if (dbc->Mrs->errnr || lc->out->errnr){
				/* 08001 = Client unable to establish connection */
				addDbcError(dbc, "08001", "sockets not opened correctly", 0);
				rc = SQL_ERROR;
			} else {
				/* all went ok, store the connection info */
				dbc->socket = socket_fd;
				dbc->Mrs = rs;
				dbc->Connected = 1;
			}
		} else {
			/* 08001 = Client unable to establish connection */
			addDbcError(dbc, "08001", "sockets not opened correctly", 0);
			rc = SQL_ERROR;
		}
	} else {
		/* 08001 = Client unable to establish connection */
		addDbcError(dbc, "08001", NULL, 0);
		rc = SQL_ERROR;
	}

	/* store internal information and clean up buffers */
	if (dbc->Connected == 1) {
		if (dbc->DSN != NULL) {
			GDKfree(dbc->DSN);
			dbc->DSN = NULL;
		}
		dbc->DSN = dsn;
		if (dbc->UID != NULL) {
			GDKfree(dbc->UID);
			dbc->UID = NULL;
		}
		dbc->UID = uid;
		if (dbc->PWD != NULL) {
			GDKfree(dbc->PWD);
			dbc->PWD = NULL;
		}
		dbc->PWD = pwd;
		if (dbc->DBNAME != NULL) {
			GDKfree(dbc->DBNAME);
			dbc->DBNAME = NULL;
		}
		dbc->DBNAME = database;

	} else {
		if (uid != NULL) {
			GDKfree(uid);
		}
		if (pwd != NULL) {
			GDKfree(pwd);
		}
		if (database != NULL) {
			GDKfree(database);
		}
	}
	/* free allocated but not stored strings */
	if (host != NULL) {
		GDKfree(host);
	}

	return rc;
}
