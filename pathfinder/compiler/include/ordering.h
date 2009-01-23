/**
 * @file
 *
 * Deal with table orderings in physical algebra plans.
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
 * 2008-2009 Eberhard Karls Universitaet Tuebingen, respectively.  All
 * Rights Reserved.
 *
 * $Id$
 */

#ifndef ORDERING_H
#define ORDERING_H

/* Orderings use arrays of column names. */
#include "array.h"

/* We speak about algebra column names. */
#include "algebra.h"

/**
 * We denote the lexicographic ordering @f$ O @f$ on columns
 * @f$ A_1 @f$ (major) and @f$ A_2 @f$ (minor) of a relation @f$ R @f$
 * as @f$ O = [A_1, A_2] @f$.  An ordering may be @em refined:
 * @f[
 *       [A_1, A_2] + A_3  = [A_1, A_2, A_3] .
 * @f]
 */
typedef PFarray_t * PFord_ordering_t;

/**
 * We often deal with sets of orderings.
 *
 * Technically, they are implemented as #PFarray_t arrays.  Some
 * helper functions let you easily deal with these arrays.
 */
typedef PFarray_t * PFord_set_t;

/**
 * Create a new #PFord_ordering_t object.
 */
PFord_ordering_t PFordering (void);

/**
 * Construct an order list (based on the column list) with ascending order.
 */
#define PFord_order_intro(...)                                     \
    PFord_order_intro_ ((sizeof ((PFalg_col_t[]) { __VA_ARGS__ }) \
                         / sizeof (PFalg_col_t)),                  \
                         (PFalg_col_t[]) { __VA_ARGS__ })
PFord_ordering_t PFord_order_intro_ (unsigned int count, PFalg_col_t *cols);

/**
 * Create an empty set of orderings.
 */
PFord_set_t PFord_set (void);

/**
 * Refine an existing ordering by one more column.
 */
PFord_ordering_t PFord_refine (const PFord_ordering_t, 
                               const PFalg_col_t,
                               const bool);

#define PFord_concat(a,b) PFarray_concat(PFarray_copy ((a)), (b))

unsigned int PFord_count (const PFord_ordering_t ordering);

PFalg_col_t PFord_order_col_at (const PFord_ordering_t ordering,
                                unsigned int index);

bool PFord_order_dir_at (const PFord_ordering_t ordering,
                         unsigned int index);

void PFord_set_order_col_at (PFord_ordering_t ordering, unsigned int index,
                             PFalg_col_t name);

void PFord_set_order_dir_at (PFord_ordering_t ordering, unsigned int index,
                             bool dir);

/**
 * Return true if the ordering @a a implies the ordering @a b.
 * I.e., ordering @a a is a refinement of ordering @a b.
 */
bool PFord_implies (const PFord_ordering_t a, const PFord_ordering_t b);

/**
 * Return true if orderings @a a and @a b share a common prefix.
 * 
 * (Ordering @a a can then be derived from @a b by @em refinement
 * and vice versa. It is generally cheaper to refine some existing
 * sort order instead of sorting entirely from scratch.)
 */
bool
PFord_common_prefix (const PFord_ordering_t a, const PFord_ordering_t b);

/**
 * Generate string representation of a given ordering.
 */
char *PFord_str (const PFord_ordering_t);

/**
 * Number of elements in a set of orderings.
 */
unsigned int PFord_set_count (const PFord_set_t);

/**
 * Append an element to a set of orderings.
 * Modifies @a set and gives it back as its return value.
 */
PFord_set_t PFord_set_add (PFord_set_t set, const PFord_ordering_t ord);

/**
 * Return item (ordering) at position @a i in a set of orderings @a set.
 */
PFord_ordering_t PFord_set_at (const PFord_set_t set, unsigned int i);

/**
 * Compute all orderings in the intersection of orderings in @a
 * and orderings in @a b.
 *
 * @note
 *   This function may return duplicates, or prefixes that are
 *   implied by other prefixes in the result.
 */
PFord_set_t PFord_intersect (const PFord_set_t a, const PFord_set_t b);

/**
 * Compute duplicate-free equivalent of the list of orderings @a a.
 * This also eliminates orderings that are implied by some more
 * specific ordering.
 */
PFord_set_t PFord_unique (const PFord_set_t a);

/**
 * Given a list of orderings @a a, compute all prefixes of the
 * orderings in @a a.
 *
 * This ``expands'' the current list of orderings by all those
 * orderings that the original list implies.  Does not produce
 * 1:1 duplicates in the result.
 */
PFord_set_t PFord_prefixes (PFord_set_t a);

/**
 * Compute all permutations of an ordering @a ordering and return
 * them as a set of orderings (see #PFord_set_t).
 */
PFord_set_t PFord_permutations (const PFord_ordering_t ordering);

#endif  /* ORDERING_H */

/* vim:set shiftwidth=4 expandtab: */
