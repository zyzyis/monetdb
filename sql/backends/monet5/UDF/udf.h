/*
 * The contents of this file are subject to the MonetDB Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://monetdb.cwi.nl/Legal/MonetDBLicense-1.1.html
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
 * Copyright August 2008-2011 MonetDB B.V.
 * All Rights Reserved.
*/

#ifndef _SQL_UDF_H_
#define _SQL_UDF_H_
#include "monetdb_config.h"
#include "sql.h"
#include <string.h>

#ifdef WIN32
#ifndef LIBUDF
#define udf_export extern __declspec(dllimport)
#else
#define udf_export extern __declspec(dllexport)
#endif
#else
#define udf_export extern
#endif

udf_export str UDFreverse(str *ret, str *src);
udf_export str UDFBATreverse(int *ret, int *bid);

#endif /* _SQL_UDF_H_ */
