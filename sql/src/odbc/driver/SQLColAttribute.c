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
 * SQLColAttribute()
 * CLI Compliance: ISO 92
 *
 * Author: Martin van Dinther
 * Date  : 30 aug 2002
 *
 **********************************************************************/

#include "ODBCGlobal.h"
#include "ODBCStmt.h"
#include "ODBCUtil.h"


SQLRETURN
SQLColAttribute_(ODBCStmt *stmt, SQLUSMALLINT nCol,
		 SQLUSMALLINT nFieldIdentifier, SQLPOINTER pszValue,
		 SQLSMALLINT nValueLengthMax, SQLSMALLINT *pnValueLength,
		 SQLPOINTER pnValue)
{
	ODBCDescRec *rec;

	if (!isValidStmt(stmt))
		 return SQL_INVALID_HANDLE;

	clearStmtErrors(stmt);

	/* check statement cursor state, query should be prepared or executed */
	if (stmt->State == INITED) {
		/* caller should have called SQLPrepare or SQLExecDirect first */
		/* HY010 = Function sequence error */
		addStmtError(stmt, "HY010", NULL, 0);
		return SQL_ERROR;
	}

	/* When the query is only prepared (via SQLPrepare) we do not have
	 * the Columns info yet (this is a limitation of the current
	 * MonetDB SQL frontend implementation). */
	/* we only have a correct data when the query is executed and a Result is created */
	if (stmt->State != EXECUTED) {
		/* HY000 = General Error */
		addStmtError(stmt, "HY000",
			     "Cannot return the column info. Query must be executed first",
			     0);
		return SQL_ERROR;
	}
	if (stmt->ImplRowDescr->descRec == NULL) {
		addStmtError(stmt, "HY000",
			     "Cannot return the column info. No result set is available",
			     0);
		return SQL_ERROR;
	}

	/* check input parameter */
	if (nCol < 1 || nCol > stmt->ImplRowDescr->sql_desc_count) {
		/* 07009 = Invalid descriptor index */
		addStmtError(stmt, "07009", NULL, 0);
		return SQL_ERROR;
	}

/* TODO: finish implementation */
	rec = stmt->ImplRowDescr->descRec + nCol;

	switch (nFieldIdentifier) {
	case SQL_COLUMN_LENGTH:
	case SQL_COLUMN_PRECISION:
	case SQL_COLUMN_SCALE:
		/* XXX needs to be implemented */
		break;
	case SQL_DESC_AUTO_UNIQUE_VALUE: /* SQL_COLUMN_AUTO_INCREMENT */
		if (pnValue)
			* (int *) pnValue = rec->sql_desc_auto_unique_value;
		break;
	case SQL_DESC_BASE_COLUMN_NAME:
		copyString(rec->sql_desc_base_column_name,
			   pszValue, nValueLengthMax, pnValueLength,
			   addStmtError, stmt);
		break;
	case SQL_DESC_BASE_TABLE_NAME:
		copyString(rec->sql_desc_base_table_name,
			   pszValue, nValueLengthMax, pnValueLength,
			   addStmtError, stmt);
		break;
	case SQL_DESC_CASE_SENSITIVE: /* SQL_COLUMN_CASE_SENSITIVE */
		if (pnValue)
			* (int *) pnValue = rec->sql_desc_case_sensitive;
		break;
	case SQL_DESC_CATALOG_NAME: /* SQL_COLUMN_QUALIFIER_NAME */
		copyString(rec->sql_desc_catalog_name,
			   pszValue, nValueLengthMax, pnValueLength,
			   addStmtError, stmt);
		break;
	case SQL_DESC_CONCISE_TYPE: /* SQL_COLUMN_TYPE */
		if (pnValue)
			* (int *) pnValue = rec->sql_desc_concise_type;
		break;
	case SQL_DESC_COUNT:
		if (pnValue)
			* (int *) pnValue = stmt->ImplRowDescr->sql_desc_count;
		break;
	case SQL_DESC_DISPLAY_SIZE: /* SQL_COLUMN_DISPLAY_SIZE */
		if (pnValue)
			* (int *) pnValue = rec->sql_desc_display_size;
		break;
	case SQL_DESC_FIXED_PREC_SCALE:	/* SQL_COLUMN_MONEY */
		if (pnValue)
			* (int *) pnValue = rec->sql_desc_fixed_prec_scale;
		break;
	case SQL_DESC_LABEL:	/* SQL_COLUMN_LABEL */
		copyString(rec->sql_desc_label,
			   pszValue, nValueLengthMax, pnValueLength,
			   addStmtError, stmt);
		break;
	case SQL_DESC_LENGTH:
		if (pnValue)
			* (int *) pnValue = rec->sql_desc_length;
		break;
	case SQL_DESC_LITERAL_PREFIX:
		copyString(rec->sql_desc_literal_prefix,
			   pszValue, nValueLengthMax, pnValueLength,
			   addStmtError, stmt);
		break;
	case SQL_DESC_LITERAL_SUFFIX:
		copyString(rec->sql_desc_literal_suffix,
			   pszValue, nValueLengthMax, pnValueLength,
			   addStmtError, stmt);
		break;
	case SQL_DESC_LOCAL_TYPE_NAME:
		copyString(rec->sql_desc_local_type_name,
			   pszValue, nValueLengthMax, pnValueLength,
			   addStmtError, stmt);
		break;
	case SQL_DESC_NAME:
		copyString(rec->sql_desc_name,
			   pszValue, nValueLengthMax, pnValueLength,
			   addStmtError, stmt);
		break;
	case SQL_DESC_NULLABLE:
		if (pnValue)
			* (int *) pnValue = rec->sql_desc_nullable;
		break;
	case SQL_DESC_NUM_PREC_RADIX:
		if (pnValue)
			* (int *) pnValue = rec->sql_desc_num_prec_radix;
		break;
	case SQL_DESC_OCTET_LENGTH:
		if (pnValue)
			* (int *) pnValue = rec->sql_desc_octet_length;
		break;
	case SQL_DESC_PRECISION:
		if (pnValue)
			* (int *) pnValue = rec->sql_desc_precision;
		break;
	case SQL_DESC_SCALE:
		if (pnValue)
			* (int *) pnValue = rec->sql_desc_scale;
		break;
	case SQL_DESC_SCHEMA_NAME: /* SQL_COLUMN_OWNER_NAME */
		copyString(rec->sql_desc_schema_name,
			   pszValue, nValueLengthMax, pnValueLength,
			   addStmtError, stmt);
		break;
	case SQL_DESC_SEARCHABLE: /* SQL_COLUMN_SEARCHABLE */
		if (pnValue)
			* (int *) pnValue = rec->sql_desc_searchable;
		break;
	case SQL_DESC_TABLE_NAME: /* SQL_COLUMN_TABLE_NAME */
		copyString(rec->sql_desc_table_name,
			   pszValue, nValueLengthMax, pnValueLength,
			   addStmtError, stmt);
		break;
	case SQL_DESC_TYPE:
		if (pnValue)
			* (int *) pnValue = rec->sql_desc_type;
		break;
	case SQL_DESC_TYPE_NAME: /* SQL_COLUMN_TYPE_NAME */
		copyString(rec->sql_desc_type_name,
			   pszValue, nValueLengthMax, pnValueLength,
			   addStmtError, stmt);
		break;
	case SQL_DESC_UNNAMED:
		if (pnValue)
			* (int *) pnValue = rec->sql_desc_unnamed;
		break;
	case SQL_DESC_UNSIGNED:	/* SQL_COLUMN_UNSIGNED */
		if (pnValue)
			* (int *) pnValue = rec->sql_desc_unsigned;
		break;
	case SQL_DESC_UPDATABLE: /* SQL_COLUMN_UPDATABLE */
		if (pnValue)
			* (int *) pnValue = rec->sql_desc_updatable;
		break;
	default:
		/* HY091 = Invalid descriptor field identifier */
		addStmtError(stmt, "HY091", NULL, 0);
		return SQL_ERROR;
	}

	return stmt->Error ? SQL_SUCCESS_WITH_INFO : SQL_SUCCESS;
}

SQLRETURN SQL_API
SQLColAttribute(SQLHSTMT hStmt, SQLUSMALLINT nCol,
		SQLUSMALLINT nFieldIdentifier, SQLPOINTER pszValue,
		SQLSMALLINT nValueLengthMax, SQLSMALLINT *pnValueLength,
		SQLPOINTER pnValue)
{
#ifdef ODBCDEBUG
	ODBCLOG("SQLColAttribute %d\n", nFieldIdentifier);
#endif

	return SQLColAttribute_((ODBCStmt *) hStmt, nCol, nFieldIdentifier,
				pszValue, nValueLengthMax, pnValueLength,
				pnValue);
}
