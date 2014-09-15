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
 * Copyright August 2008-2014 MonetDB B.V.
 * All Rights Reserved.
 */

#ifndef _SEEN_PROPERTIES_H
#define _SEEN_PROPERTIES_H 1

#include "utils.h"

#define MEROPROPFILE ".merovingian_properties"

confkeyval *getDefaultProps(void);
int writeProps(confkeyval *ckv, const char *path);
void writePropsBuf(confkeyval *ckv, char **buf);
int readProps(confkeyval *ckv, const char *path);
int readAllProps(confkeyval *ckv, const char *path);
void readPropsBuf(confkeyval *ckv, char *buf);
char *setProp(char *path, char *key, char *val);

#endif

/* vim:set ts=4 sw=4 noexpandtab: */
