#ifdef _MSC_VER
#include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <sql.h>
#include <sqlext.h>

#ifdef _MSC_VER
#define snprintf _snprintf
#endif

void
prerr(SQLSMALLINT tpe, SQLHANDLE hnd, const char *func, const char *pref)
{
	SQLCHAR state[6];
	SQLINTEGER errnr;
	SQLCHAR msg[256];
	SQLSMALLINT msglen;

	switch (SQLGetDiagRec(tpe, hnd, 1, state, &errnr, msg, sizeof(msg), &msglen)) {
	case SQL_SUCCESS_WITH_INFO:
		if (msglen >= (signed int)sizeof(msg))
			fprintf(stderr, "(message truncated)\n");
	case SQL_SUCCESS:
		fprintf(stderr,
			"%s: %s: SQLstate %s, Errnr %d, Message %s\n",
			func, pref, state, errnr, msg);
		break;
	case SQL_INVALID_HANDLE:
		fprintf(stderr,
			"%s: %s, invalid handle passed to error function\n",
			func, pref);
		break;
	case SQL_ERROR:
		fprintf(stderr,
			"%s: %s, unexpected error from SQLGetDiagRec\n",
			func, pref);
		break;
	case SQL_NO_DATA:
		fprintf(stderr, "%s: %s, no error message from driver\n",
			func, pref);
		break;
	default:
		fprintf(stderr,
			"%s: %s, weird return value from SQLGetDiagRec\n",
			func, pref);
		break;
	}
}

void
check(SQLRETURN ret, SQLSMALLINT tpe, SQLHANDLE hnd, const char *func)
{
	switch (ret) {
	case SQL_SUCCESS:
		break;
	case SQL_SUCCESS_WITH_INFO:
		prerr(tpe, hnd, func, "Info");
		break;
	case SQL_ERROR:
		prerr(tpe, hnd, func, "Error");
		exit(1);
	case SQL_INVALID_HANDLE:
		fprintf(stderr, "%s: Error: invalid handle\n", func);
		exit(1);
	default:
		fprintf(stderr, "%s: Unexpected return value\n", func);
		break;
	}
}

int
main(int argc, char **argv)
{
	SQLHANDLE env;
	SQLHANDLE dbc;
	SQLHANDLE stmt, stmt2;
	char *dsn = "MonetDB";
	char *user = "monetdb";
	char *pass = "monetdb";
	SQLRETURN ret;
	int i;
	SQLSMALLINT f1;
	char f2[30];
	SQLDOUBLE f3;
	SQL_DATE_STRUCT f4;
	SQL_TIME_STRUCT f5;

	if (argc > 1)
		dsn = argv[1];
	if (argc > 2)
		user = argv[2];
	if (argc > 3)
		pass = argv[3];
	if (argc > 4 || *dsn == '-') {
		fprintf(stderr, "Usage: %s [datasource [user [password]]]\n",
			argv[0]);
		exit(1);
	}

	if (SQLAllocHandle(SQL_HANDLE_ENV, NULL, &env) != SQL_SUCCESS) {
		fprintf(stderr, "Cannot allocate ODBC environment handle\n");
		exit(1);
	}

	ret = SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION,
			    (SQLPOINTER) (size_t) SQL_OV_ODBC3, 0);
	check(ret, SQL_HANDLE_ENV, env, "SQLSetEnvAttr");

	ret = SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc);
	check(ret, SQL_HANDLE_ENV, env, "SQLAllocHandle 1");

	ret = SQLConnect(dbc, (SQLCHAR *) dsn, SQL_NTS,
			 (SQLCHAR *) user, SQL_NTS, (SQLCHAR *) pass, SQL_NTS);
	check(ret, SQL_HANDLE_DBC, dbc, "SQLConnect");

#ifdef WITHOUT_AUTO_COMMIT
	ret = SQLSetConnectAttr(dbc, SQL_ATTR_AUTOCOMMIT,
				(SQLPOINTER) (size_t) SQL_AUTOCOMMIT_OFF, 0);
	check(ret, SQL_HANDLE_DBC, dbc, "SQLSetConnectAttr");
#endif

	/* create a test table to be filled with values */
	ret = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
	check(ret, SQL_HANDLE_DBC, dbc, "SQLAllocHandle 2");

	ret = SQLExecDirect(stmt, (SQLCHAR*)
			    "CREATE TABLE test (\n"
			    "   i INT(11) DEFAULT '0' NOT NULL,\n"
			    "   s VARCHAR,\n"
			    "   f FLOAT,\n"
			    "   d DATE,\n"
			    "   t TIME\n"
			    ")", SQL_NTS);
	check(ret, SQL_HANDLE_STMT, stmt, "SQLExecDirect 1");

	/* prepare for filling the test table */
	/* we use a single statement with parameters whose values vary */

	/* bind a bunch of parameters before preparing the statement */
	ret = SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_SSHORT,
			       SQL_INTEGER, 0, 0, &f1, sizeof(f1), NULL);
	check(ret, SQL_HANDLE_STMT, stmt, "SQLBindParameter 1");
	ret = SQLBindParameter(stmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR,
			       SQL_VARCHAR, 0, 0, &f2, sizeof(f2), NULL);
	check(ret, SQL_HANDLE_STMT, stmt, "SQLBindParameter 2");
	ret = SQLBindParameter(stmt, 3, SQL_PARAM_INPUT, SQL_C_DOUBLE,
			       SQL_FLOAT, 0, 0, &f3, sizeof(f3), NULL);
	check(ret, SQL_HANDLE_STMT, stmt, "SQLBindParameter 3");

	ret = SQLPrepare(stmt, (SQLCHAR*)
			 "INSERT INTO test VALUES (?, ?, ?, ?, ?)",
			 SQL_NTS);
	check(ret, SQL_HANDLE_STMT, stmt, "SQLPrepare 1");

	/* bind the rest of the parameters after preparing the statement */
	ret = SQLBindParameter(stmt, 4, SQL_PARAM_INPUT, SQL_C_TYPE_DATE,
			       SQL_TYPE_DATE, 0, 0, &f4, sizeof(f4), NULL);
	check(ret, SQL_HANDLE_STMT, stmt, "SQLBindParameter 4");
	ret = SQLBindParameter(stmt, 5, SQL_PARAM_INPUT, SQL_C_TYPE_TIME,
			       SQL_TYPE_TIME, 0, 0, &f5, sizeof(f5), NULL);
	check(ret, SQL_HANDLE_STMT, stmt, "SQLBindParameter 5");

	/* do the actual filling of the test table */
	f4.year = 2003;
	f4.month = 1;
	f4.day = 1;
	f5.hour = 0;
	f5.minute = 0;
	f5.second = 0;
	for (i = 0; i < 2000; i++) {
		f1 = i;
		snprintf(f2, sizeof(f2), "value %d", i);
		f3 = i * 1.5;
		f4.day++;
		if ((f4.day == 29 && f4.month == 2) ||
		    (f4.day == 31 && (f4.month == 4 || f4.month == 6 ||
				      f4.month == 9 || f4.month == 11)) ||
		    f4.day == 32) {
			f4.day = 1;
			f4.month++;
			if (f4.month == 13) {
				f4.month = 1;
				f4.year++;
			}
		}
		f5.second++;
		if (f5.second == 60) {
			f5.second = 0;
			f5.minute++;
			if (f5.minute == 60) {
				f5.minute = 0;
				f5.hour++;
				if (f5.hour == 25)
					f5.hour = 0;
			}
		}
		ret = SQLExecute(stmt);
		check(ret, SQL_HANDLE_STMT, stmt, "SQLExecute 1");
	}

	/* Now we are going to read back the values from the test table.
	   We create two statment handles, one of which will be used
	   to read the even table entries and the other for the odd
	   table entries. */

	/* first the handle for the even entries */

	/* bind the columns before preparing the statement */
	ret = SQLBindCol(stmt, 1, SQL_C_SSHORT, &f1, sizeof(f1), NULL);
	check(ret, SQL_HANDLE_STMT, stmt, "SQLBindCol 1");
	ret = SQLBindCol(stmt, 2, SQL_C_CHAR, &f2, sizeof(f2), NULL);
	check(ret, SQL_HANDLE_STMT, stmt, "SQLBindCol 2");
	ret = SQLBindCol(stmt, 3, SQL_C_DOUBLE, &f3, sizeof(f3), NULL);
	check(ret, SQL_HANDLE_STMT, stmt, "SQLBindCol 3");
	ret = SQLBindCol(stmt, 4, SQL_C_TYPE_DATE, &f4, sizeof(f4), NULL);
	check(ret, SQL_HANDLE_STMT, stmt, "SQLBindCol 4");
	ret = SQLBindCol(stmt, 5, SQL_C_TYPE_TIME, &f5, sizeof(f5), NULL);
	check(ret, SQL_HANDLE_STMT, stmt, "SQLBindCol 5");

	ret = SQLPrepare(stmt, (SQLCHAR*)"SELECT * FROM test WHERE 2*(i/2) = i", SQL_NTS);
	check(ret, SQL_HANDLE_STMT, stmt, "SQLPrepare 2");

	ret = SQLExecute(stmt);
	check(ret, SQL_HANDLE_STMT, stmt, "SQLExecute 2");

	/* now the handle for the odd entries */
	ret = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt2);
	check(ret, SQL_HANDLE_DBC, dbc, "SQLAllocHandle 3");

	ret = SQLPrepare(stmt2, (SQLCHAR*)"SELECT * FROM test WHERE 2*(i/2) <> i", SQL_NTS);
	check(ret, SQL_HANDLE_STMT, stmt2, "SQLPrepare 3");

	/* bind the columns after preparing the statement */
	ret = SQLBindCol(stmt2, 1, SQL_C_SSHORT, &f1, sizeof(f1), NULL);
	check(ret, SQL_HANDLE_STMT, stmt2, "SQLBindCol 6");
	ret = SQLBindCol(stmt2, 2, SQL_C_CHAR, &f2, sizeof(f2), NULL);
	check(ret, SQL_HANDLE_STMT, stmt2, "SQLBindCol 7");
	ret = SQLBindCol(stmt2, 3, SQL_C_DOUBLE, &f3, sizeof(f3), NULL);
	check(ret, SQL_HANDLE_STMT, stmt2, "SQLBindCol 8");
	ret = SQLBindCol(stmt2, 4, SQL_C_TYPE_DATE, &f4, sizeof(f4), NULL);
	check(ret, SQL_HANDLE_STMT, stmt2, "SQLBindCol 9");
	ret = SQLBindCol(stmt2, 5, SQL_C_TYPE_TIME, &f5, sizeof(f5), NULL);
	check(ret, SQL_HANDLE_STMT, stmt2, "SQLBindCol 10");

	ret = SQLExecute(stmt2);
	check(ret, SQL_HANDLE_STMT, stmt2, "SQLExecute 3");

	for (;;) {
		/* Alternate fetching an even and an odd entry.  The
		   end result should be that we get all entries in the
		   correct order. */
		ret = SQLFetch(stmt);
		if (ret == SQL_NO_DATA)
			break;
		check(ret, SQL_HANDLE_STMT, stmt, "SQLFetch 1");

		printf("%d %s %g %04d:%02d:%02d %02d-%02d-%02d\n",
		       f1, f2, f3, f4.year, f4.month, f4.day,
		       f5.hour, f5.minute, f5.second);

		ret = SQLFetch(stmt2);
		if (ret == SQL_NO_DATA)
			break;
		check(ret, SQL_HANDLE_STMT, stmt2, "SQLFetch 2");

		printf("%d %s %g %04d:%02d:%02d %02d-%02d-%02d\n",
		       f1, f2, f3, f4.year, f4.month, f4.day,
		       f5.hour, f5.minute, f5.second);
	}

	ret = SQLCloseCursor(stmt);
	check(ret, SQL_HANDLE_STMT, stmt, "SQLCloseCursor");

	/* cleanup and disconnect */
	ret = SQLFreeHandle(SQL_HANDLE_STMT, stmt2);
	check(ret, SQL_HANDLE_STMT, stmt2, "SQLFreeHandle 1");

	/* drop the test table */
	ret = SQLExecDirect(stmt, (SQLCHAR*) "DROP TABLE test", SQL_NTS);
	check(ret, SQL_HANDLE_STMT, stmt, "SQLExecDirect 2");

	ret = SQLFreeHandle(SQL_HANDLE_STMT, stmt);
	check(ret, SQL_HANDLE_STMT, stmt, "SQLFreeHandle 2");

	ret = SQLDisconnect(dbc);
	check(ret, SQL_HANDLE_DBC, dbc, "SQLDisconnect");

	ret = SQLFreeHandle(SQL_HANDLE_DBC, dbc);
	check(ret, SQL_HANDLE_DBC, dbc, "SQLFreeHandle 3");

	ret = SQLFreeHandle(SQL_HANDLE_ENV, env);
	check(ret, SQL_HANDLE_STMT, stmt, "SQLFreeHandle 4");

	return 0;
}
