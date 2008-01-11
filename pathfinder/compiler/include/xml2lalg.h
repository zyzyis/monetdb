/* -*- c-basic-offset:4; c-indentation-style:"k&r"; indent-tabs-mode:nil -*- */

/**
 * @file
 * 
 * Importing of XML-serialized logical Algebra Plans
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
 * is now maintained by the Database Systems Group at the Technische
 * Universitaet Muenchen, Germany.  Portions created by the University of
 * Konstanz and the Technische Universitaet Muenchen are Copyright (C)
 * 2000-2005 University of Konstanz and (C) 2005-2008 Technische
 * Universitaet Muenchen, respectively.  All Rights Reserved.
 *
 * $Id$
 */




#ifndef XML2ALG_H
#define XML2ALG_H


#include "pathfinder.h"


#include "logical.h"

#include "array.h"


typedef struct XML2LALGContext XML2LALGContext;


struct XML2LALGContext {

    PFarray_t* nodeStore;   

    void* docXPathCtx;
   
    PFalg_att_t (*convert2PFLA_attributeName)(const char* s);
};



XML2LALGContext* 
PFxml2la_xml2lalgContext(void);

PFla_op_t* 
PFxml2la_importXMLFromFile(XML2LALGContext* ctx, const char *filename);

PFla_op_t* 
PFxml2la_importXMLFromMemory(XML2LALGContext* ctx, const char* xml, int size);



#endif

