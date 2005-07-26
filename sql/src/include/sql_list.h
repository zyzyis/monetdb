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
 * Portions created by CWI are Copyright (C) 1997-2005 CWI.
 * All Rights Reserved.
 */

#ifndef LIST_H
#define LIST_H

#include "sql_mem.h"

typedef struct node {
	struct node *next;
	void *data;
} node;

typedef void (*fdestroy) (void *);

typedef struct list {
	fdestroy destroy;
	node *h;
	node *t;
	int cnt;
} list;

typedef int (*traverse_func) (char *clientdata, int seqnr, void *data);

extern list *list_create(fdestroy destroy);

extern void list_destroy(list *l);
extern int list_length(list *l);

extern list *list_append(list *l, void *data);
extern list *list_prepend(list *l, void *data);

extern node *list_remove_node(list *l, node *n);
extern void list_remove_data(list *l, void *data);
extern void list_move_data(list *l, list *d, void *data);


extern int list_traverse(list *l, traverse_func f, char *clientdata);

/* the compare function gets one element from the list and a key from the
 * as input from the find function 
 * Returns 0 if data and key are equal 
 * */
typedef int (*fcmp) (void *data, void *key);
typedef void *(*fdup) (void *data);
typedef void *(*freduce) (void *v1, void *v2);
typedef void *(*fmap) (void *data, void *clientdata);

extern node *list_find(list *l, void *key, fcmp cmp);
extern list *list_select(list *l, void *key, fcmp cmp, fdup dup);
extern list *list_distinct(list *l, fcmp cmp, fdup dup);
extern void *list_reduce(list *l, freduce red, fdup dup);
extern list *list_map(list *l, void *data, fmap f);
extern int list_cmp(list *l1, list *l2, fcmp cmp);

extern list *list_dup(list *l, fdup dup);
extern list *list_merge(list *l, list *data, fdup dup);

#endif /* LIST_H */
