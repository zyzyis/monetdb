/* -*- c-basic-offset:4; c-indentation-style:"k&r"; indent-tabs-mode:nil -*- */

/**
 * @file abssynprint.h 
 * 
 * Debugging: dump XQuery abstract syntax tree in
 * AY&T dot format or human readable
 *
 * Copyright Notice:
 * -----------------
 *
 *  The contents of this file are subject to the MonetDB Public
 *  License Version 1.0 (the "License"); you may not use this file
 *  except in compliance with the License. You may obtain a copy of
 *  the License at http://monetdb.cwi.nl/Legal/MonetDBLicense-1.0.html
 *
 *  Software distributed under the License is distributed on an "AS
 *  IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 *  implied. See the License for the specific language governing
 *  rights and limitations under the License.
 *
 *  The Original Code is the ``Pathfinder'' system. The Initial
 *  Developer of the Original Code is the Database & Information
 *  Systems Group at the University of Konstanz, Germany. Portions
 *  created by U Konstanz are Copyright (C) 2000-2003 University
 *  of Konstanz. All Rights Reserved.
 *
 *  Contributors:
 *          Torsten Grust <torsten.grust@uni-konstanz.de>
 *          Maurice van Keulen <M.van.Keulen@bigfoot.com>
 *          Jens Teubner <jens.teubner@uni-konstanz.de>
 *
 * $Id$
 */

#ifndef ABSSYNPRINT_H
#define ABSSYNPRINT_H

/* FILE, ... */
#include <stdio.h>

/* node, ... */
#include "abssyn.h"

/** Node names to print out for all the abstract syntax tree nodes. */
extern char *p_id[];

PFrc_t PFabssyn_dot (FILE *f, PFpnode_t *root);

PFrc_t PFabssyn_pretty (FILE *f, PFpnode_t *root);

#endif

/* vim:set shiftwidth=4 expandtab: */
