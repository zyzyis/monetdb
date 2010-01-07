/* -*- c-basic-offset:4; c-indentation-style:"k&r"; indent-tabs-mode:nil -*- */

/**
 * @file
 *
 * Importing XML Schema types into the Pathfinder type environment.
 *
 * See Section `Importing Schemas' in the W3C XQuery 1.0 and XPath 2.0
 * Formal Semantics and Jan Rittinger's BSc thesis.
 *
 *
 * Copyright Notice:
 * -----------------
 *
 * The contents of this file are subject to the Pathfinder Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License.  You may obtain a copy of the License at
 * http://monetdb.cwi.nl/Legal/PathfinderLicense-1.1.html
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is the Pathfinder system.
 *
 * The Original Code has initially been developed by the Database &
 * Information Systems Group at the University of Konstanz, Germany and
 * the Database Group at the Technische Universitaet Muenchen, Germany.
 * It is now maintained by the Database Systems Group at the Eberhard
 * Karls Universitaet Tuebingen, Germany.  Portions created by the
 * University of Konstanz, the Technische Universitaet Muenchen, and the
 * Universitaet Tuebingen are Copyright (C) 2000-2005 University of
 * Konstanz, (C) 2005-2008 Technische Universitaet Muenchen, and (C)
 * 2008-2010 Eberhard Karls Universitaet Tuebingen, respectively.  All
 * Rights Reserved.
 *
 * $Id$
 */

#ifndef IMPORT_H
#define IMPORT_H

/* PFpnode_t */
#include "abssyn.h"

/** Walk the query prolog and process `import schema' instructions */
void PFschema_import (PFpnode_t *);

#endif /* IMPORT_H */

/* vim:set shiftwidth=4 expandtab: */
