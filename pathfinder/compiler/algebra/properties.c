/**
 * @file
 *
 * Manage properties of logical algebra expressions.
 * (The inference of the properties is done separately for
 *  each property -- see prop/prop_*.c)
 *
 * We consider some properties that can be derived on the logical
 * level of our algebra, like key properties, or the information
 * that a column contains only constant values.  These properties
 * may still be helpful for physical optimization; we will thus
 * propagate any logical property to the physical tree as well.
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
 * The Initial Developer of the Original Code is the Database &
 * Information Systems Group at the University of Konstanz, Germany.
 * Portions created by the University of Konstanz are Copyright (C)
 * 2000-2006 University of Konstanz.  All Rights Reserved.
 *
 * $Id$
 */

/* always include pathfinder.h first! */
#include "pathfinder.h"
#include <assert.h>

#include "properties.h"
#include "alg_dag.h"
#include "mem.h"

/**
 * Create new property container.
 */
PFprop_t *
PFprop (void)
{
    PFprop_t *ret = PFmalloc (sizeof (PFprop_t));

    /* allocate/initialize different slots for constant property*/
    ret->constants   = PFarray (sizeof (const_t));
    ret->l_constants = PFarray (sizeof (const_t));
    ret->r_constants = PFarray (sizeof (const_t));

    /* initialize icols attribute list */
    ret->icols   = 0;
    ret->l_icols = 0;
    ret->r_icols = 0;

    /* initialize key attribute list */
    ret->keys   = 0;
    ret->l_keys = 0;
    ret->r_keys = 0;

    /* initialize cardinality */
    ret->card = 0;

    /* initialize key attribute list */
    ret->reqvals.name = 0;
    ret->reqvals.val  = 0;

    /* initialize domain information */
    ret->domains   = PFarray (sizeof (dom_pair_t));
    ret->dom_rel   = NULL;
    ret->l_domains = PFarray (sizeof (dom_pair_t));
    ret->r_domains = PFarray (sizeof (dom_pair_t));

    return ret;
}

/**
 * Infer all properties of the current tree
 * rooted in root whose flag is set.
 */
void
PFprop_infer (bool card, bool const_, bool dom,
              bool icols, bool key, bool ocols, bool reqval,
              PFla_op_t *root)
{
    PFprop_create_prop (root);

    /* for each property required infer
       the properties of the complete DAG */
    if (card)
        PFprop_infer_card (root);
    if (const_)
        PFprop_infer_const (root);
    if (dom)
        PFprop_infer_dom (root);
    if (icols)
        PFprop_infer_icol (root);
    if (key)
        PFprop_infer_key (root);
    if (ocols)
        PFprop_infer_ocol (root);
    if (reqval)
        PFprop_infer_reqval (root);
}

/* worker for PFprop_create_prop () */
static void
create_prop (PFla_op_t *n)
{
    assert (n);
    if (n->bit_dag)
        return;

    for (unsigned int i = 0; i < PFLA_OP_MAXCHILD && n->child[i]; i++)
        create_prop (n->child[i]);

    n->bit_dag = true;
    n->prop = PFprop ();
}

/**
 * Create new property fields for a DAG rooted in @a root
 */
void
PFprop_create_prop (PFla_op_t *root)
{
    create_prop (root);
    /* reset dirty dag bit */
    PFla_dag_reset (root);
}

/* vim:set shiftwidth=4 expandtab: */
