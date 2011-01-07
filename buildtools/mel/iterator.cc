// The contents of this file are subject to the MonetDB Public License
// Version 1.1 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://monetdb.cwi.nl/Legal/MonetDBLicense-1.1.html
//
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License.
//
// The Original Code is the MonetDB Database System.
//
// The Initial Developer of the Original Code is CWI.
// Portions created by CWI are Copyright (C) 1997-July 2008 CWI.
// Copyright August 2008-2011 MonetDB B.V.
// All Rights Reserved.

#include <monetdb_config.h>
#include "iterator.h"
#include "ListIterator.h"
#include "language.h"
#include <string.h>
#include <stdio.h>

Iterator::Iterator(int t, char *n, char *fcn, List *args, char *hlp) 
        : Command(t, n, fcn, NORMAL, NULL, args, hlp)
{
}

const char *
Iterator::Token() const
{
        return "TOK_ITERATOR";
}

ostream &
Iterator::print(language *l, ostream &o) const
{
	return l->gen_command(o, *this);
}
