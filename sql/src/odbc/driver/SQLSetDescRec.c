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
 * SQLSetDescRec()
 * CLI Compliance: IOS 92
 **********************************************************************/

#include "ODBCGlobal.h"
#include "ODBCStmt.h"

SQLRETURN SQL_API
SQLSetDescRec(SQLHDESC hDescriptorHandle, SQLSMALLINT nRecordNumber, SQLSMALLINT nType, SQLSMALLINT nSubType, SQLINTEGER nLength, SQLSMALLINT nPrecision, SQLSMALLINT nScale, SQLPOINTER pData, SQLINTEGER *pnStringLength, SQLINTEGER *pnIndicator)
{
	ODBCDesc *desc = (ODBCDesc *) hDescriptorHandle;

#ifdef ODBCDEBUG
	ODBCLOG("SQLSetDescRec " PTRFMT " %d %d %d %d %d %d\n", PTRFMTCAST hDescriptorHandle, nRecordNumber, nType, nSubType, nLength, nPrecision, nScale);
#endif

	if (!isValidDesc(desc))
		return SQL_INVALID_HANDLE;

	if (SQLSetDescField_(desc, nRecordNumber, SQL_DESC_TYPE, (SQLPOINTER) (ssize_t) nType, 0) == SQL_ERROR)
		return SQL_ERROR;
	if ((nType == SQL_DATETIME || nType == SQL_INTERVAL) && SQLSetDescField_(desc, nRecordNumber, SQL_DESC_DATETIME_INTERVAL_CODE, (SQLPOINTER) (ssize_t) nSubType, 0) == SQL_ERROR)
		return SQL_ERROR;
	if (SQLSetDescField_(desc, nRecordNumber, SQL_DESC_OCTET_LENGTH, (SQLPOINTER) (ssize_t) nLength, 0) == SQL_ERROR)
		return SQL_ERROR;
	if (SQLSetDescField_(desc, nRecordNumber, SQL_DESC_PRECISION, (SQLPOINTER) (ssize_t) nPrecision, 0) == SQL_ERROR)
		return SQL_ERROR;
	if (SQLSetDescField_(desc, nRecordNumber, SQL_DESC_SCALE, (SQLPOINTER) (ssize_t) nScale, 0) == SQL_ERROR)
		return SQL_ERROR;
	if (SQLSetDescField_(desc, nRecordNumber, SQL_DESC_OCTET_LENGTH_PTR, (SQLPOINTER) pnStringLength, 0) == SQL_ERROR)
		return SQL_ERROR;
	if (SQLSetDescField_(desc, nRecordNumber, SQL_DESC_INDICATOR_PTR, (SQLPOINTER) pnIndicator, 0) == SQL_ERROR)
		return SQL_ERROR;
	if (SQLSetDescField_(desc, nRecordNumber, SQL_DESC_DATA_PTR, (SQLPOINTER) pData, 0) == SQL_ERROR)
		return SQL_ERROR;
	return desc->Error ? SQL_SUCCESS_WITH_INFO : SQL_SUCCESS;
}
