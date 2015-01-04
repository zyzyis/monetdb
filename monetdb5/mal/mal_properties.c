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
 * (author) M. Kersten
 */
#include "monetdb_config.h"
#include "mal_properties.h"
#include "mal_exception.h"
#include "mal_type.h"		/* for idcmp() */

static str *properties = NULL;
static int nr_properties = 0;
static int max_properties = 0;

sht
PropertyIndex(str name)
{
	int i=0;
	for (i=0; i<nr_properties; i++) {
		if (strcmp(properties[i], name) == 0)
			return i;
	}
	MT_lock_set(&mal_contextLock, "propertyIndex");
	/* small change it's already added */
	for (i=0; i<nr_properties; i++) {
		if (strcmp(properties[i], name) == 0) {
			MT_lock_unset(&mal_contextLock, "propertyIndex");
			return i;
		}
	}
	if (i >= max_properties) {
		max_properties += 256;
		properties = GDKrealloc(properties, max_properties * sizeof(str));
		if( properties == NULL){
			GDKerror("PropertyIndex" MAL_MALLOC_FAIL);
			MT_lock_unset(&mal_contextLock, "propertyIndex");
			return nr_properties;
		}
	}
	properties[nr_properties] = GDKstrdup(name);
	MT_lock_unset(&mal_contextLock, "propertyIndex");
	return nr_properties++;
}

str
PropertyName(sht idx)
{
	if (idx < nr_properties)
		return properties[idx];
	return "None";
}

prop_op_t
PropertyOperator( str s )
{
	if (!s || !*s)
		return op_eq;
	if (*s == '<') {
		if (*(s+1) == '=')
			return op_lte;
		return op_lt;
	} else if (*s == '>') {
		if (*(s+1) == '=')
			return op_gte;
		return op_gt;
	} else if (*s == '=')
		return op_eq;
	else if (*s == '!' && *(s+1) == '=')
		return op_ne;
	return op_eq;
}

str
PropertyOperatorString( prop_op_t op )
{
	switch(op) {
	case op_lt: return "<";
	case op_lte: return "<=";
	default:
	case op_eq: return "=";
	case op_gte: return ">=";
	case op_gt: return ">";
	case op_ne: return "!=";
	}
}

