
#include "optimize.h"
#include "statement.h"
#include <stdio.h>

#define create_stmt_list() list_create((fdestroy)&stmt_destroy)

static int join_cmp_eq( stmt *jn ){
	if (jn->flag == cmp_equal){
		return 0;
	}
	return -1;
}

static int join_cmp_neq( stmt *jn ){
	if (jn->flag != cmp_equal){
		return 0;
	}
	return -1;
}

static int stmt_cmp_nrcols( stmt *s, int *nr ){
	if (s->type != st_filter && s->nrcols == *nr){
		return 0;
	}
	return -1;
}

static int stmt_cmp_filter( stmt *s ){
	if (s->type == st_filter){
		return 0;
	}
	return -1;
}

/* key is a join output */
static int cmp_sel_head( stmt *sel, stmt *key )
{
	if (key->h && key->h == sel->h){
		return 0;
	}
	return -1;
}

static int cmp_sel_tail( stmt *sel, stmt *key )
{
	if (key->t && key->t == sel->h){
		return 0;
	}
	return -1;
}

static int stmt_cmp_head_tail( stmt *h, stmt *key )
{
	if (h->nrcols != key->nrcols ) {
		return -1;
	}
	if (h->nrcols == 1 && h->h == key->h){
		return 0;
	}
	if (h->nrcols == 2 && (
		(h->h == key->h && h->t == key->t) ||
		(h->t == key->h && h->h == key->t) )){
		return 0;
	}
	return -1;
}

static int select_count( stmt *s )
{
	switch(s->type){
		case st_intersect: return select_count( s->op1.stval );
		case st_reljoin: {
			int sc = 0;
			node *n = NULL;
			for (n = s->op1.lval->h; n; n = n->next) {
				sc += select_count(n->data);
			}
			for (n = s->op2.lval->h; n; n = n->next) {
				sc += select_count(n->data);
			}
			return sc;
		}
		case st_join: return select_count(s->op1.stval) +
					select_count(s->op2.stval);
		case st_reverse: return select_count(s->op1.stval);
				 /*
		case st_semijoin: return 1;
		*/
		case st_semijoin: return select_count(s->op1.stval) +
					select_count(s->op2.stval);
		case st_like:
		case st_select: return 1;
		default: return 0;
	}
}

static stmt *stmt_smallest( stmt *h, stmt *key )
{
	int h1 = select_count(h);
	int h2 = select_count(key);

	if (h1 > h2){
		stmt_destroy(key);
		return h;
	} else {
		stmt_destroy(h);
		return key;
	}
}

static int pivot_cmp_tail( stmt *piv, stmt *key )
{
	if (key->t && key->t == piv->t){
		return 0;
	}
	return -1;
}


static int data_cmp( void *d, void *k )
{
	if (d == k) 
		return 0;
	return -1;
}


static stmt *stmt_push_down_tail(stmt * join, stmt * select);
static stmt *stmt_push_down_head(stmt * join, stmt * select){
	stmt *res = NULL;
	if (join->type == st_join){
		res = stmt_join(
			stmt_push_down_head( stmt_dup(join->op1.stval), select),
			stmt_dup(join->op2.stval), 
			join->flag);
		stmt_destroy(join);
		return res;
	} else if (join->type == st_intersect){
		res = stmt_intersect(
			stmt_push_down_head( stmt_dup(join->op1.stval), select),
			stmt_push_down_head( stmt_dup(join->op2.stval), stmt_dup(select))
				);
		stmt_destroy(join);
		return res;
	} else if (join->type == st_reverse){
		res = stmt_reverse(
		      stmt_push_down_tail( stmt_dup(join->op1.stval), select));
		stmt_destroy(join);
		return res;
	} else if (join->type == st_diff){
		stmt *op1 = stmt_semijoin( stmt_dup(join->op1.stval) ,select);
		res = stmt_diff(op1, stmt_dup(join->op2.stval) );
		stmt_destroy(join);
		return res;
	} else if (join->type == st_semijoin){
		select = stmt_dup(select);
		res = stmt_semijoin(
			stmt_push_down_head( stmt_dup(join->op1.stval), select),
			stmt_push_down_head( stmt_dup(join->op2.stval), select)
			);
		stmt_destroy(join);
		return res;
	} else if (join->type == st_select){
		/* nr cols == 2 ie push through op1 and op2 */
		if (join->op2.stval->nrcols >= 1){
		    select = stmt_dup(select);
		    res = stmt_select(
			stmt_push_down_head( stmt_dup(join->op1.stval), select),
			stmt_push_down_head( stmt_dup(join->op2.stval), select),
			join->flag);
		    stmt_destroy(join);
		    return res;
		}
	} else if (join->type == st_bat || join->type == st_column_alias){
		return stmt_semijoin(join, select);
	} else {
		printf("= TODO: common/optimize.c: push down head %s\n", st_type2string(join->type));
	}
	return stmt_semijoin(join, select);
}

/* push select (idx, val) through join(idy,idx) */
static stmt *stmt_push_down_tail(stmt * join, stmt * select){
	stmt *res;
	if (join->type == st_join){
		res = stmt_join(
			stmt_dup(join->op1.stval), 
			stmt_push_down_tail( stmt_dup(join->op2.stval), select),
			join->flag);
		stmt_destroy(join);
		return res;
	} else if (join->type == st_intersect){
		select = stmt_dup(select);
		res = stmt_intersect(
			stmt_push_down_tail( stmt_dup(join->op1.stval), select),
			stmt_push_down_tail( stmt_dup(join->op2.stval), select)
				);
		stmt_destroy(join);
		return res;
	} else if (join->type == st_reverse){
		res = stmt_reverse(
			stmt_push_down_head( stmt_dup(join->op1.stval), 
				select));
		stmt_destroy(join);
		return res;
	} else if (join->type == st_diff){
		stmt *tail = stmt_reverse( stmt_dup(join->op2.stval));
		stmt *op2 = stmt_reverse( stmt_semijoin(tail, select));
		res = stmt_diff( stmt_dup(join->op1.stval), op2 );
		stmt_destroy(join);
		return res;
	} else if (join->type == st_semijoin){
		res = stmt_semijoin(
			stmt_push_down_tail( stmt_dup(join->op1.stval), select),
			stmt_dup(join->op2.stval)
			);
		stmt_destroy(join);
		return res;
	} else if (join->type == st_bat){
		return stmt_reverse(stmt_semijoin(stmt_reverse(join), select));
	/* TODO find pivot in ptable, and push select through the ptable  */
	} else if (join->type == st_pivot){
		/* make sure the ptable is optimized*/
		stmt *nj = optimize(/* BUG no context avail*/NULL, join); 
		return stmt_push_down_tail(nj, select);
	} else if (join->type == st_mark){
		/* for now pushin through a mark is wrong, it should
		 * push through both sides, to keep ordered oids.
		res = stmt_mark(stmt_push_down_tail(
				stmt_dup(join->op1.stval), select), join->flag);
		stmt_destroy(join);
		return res;
		*/
		return stmt_reverse(stmt_semijoin(stmt_reverse(join), select));
	} else {
		printf("= TODO: common/optimize.c: push down tail %s\n", st_type2string(join->type));
	}
	stmt_destroy(select);
	return join;
}

static stmt *stmt_push_join_tail(stmt * s, stmt * join);
static stmt *stmt_push_join_head(stmt * s, stmt * join){

	if (s->type == st_reljoin){
		list *l1 = create_stmt_list();
		list *l2 = list_dup(s->op2.lval, (fdup)&stmt_dup);
		node *n;
		for (n = s->op1.lval->h; n; n = n->next) {
			l1 = list_append(l1, stmt_join(join, stmt_dup(n->data), cmp_equal));
		}
		stmt_destroy(s);
		return stmt_reljoin2(l1,l2);
	} else if (s->type == st_join){
		stmt *op1 = stmt_join( join, stmt_dup(s->op1.stval), cmp_equal);
		stmt *res = stmt_join( op1, stmt_dup(s->op2.stval), s->flag );
		stmt_destroy(s);
		return res;
	} else if (s->type == st_reverse){
		stmt *res = stmt_reverse(
			stmt_push_join_tail( stmt_dup(s->op1.stval), stmt_reverse(join)));
		stmt_destroy(s);
		return res;
	} else if (s->type == st_intersect){
		stmt *res = stmt_intersect(
			stmt_push_join_head( stmt_dup(s->op1.stval), join),
			stmt_push_join_head( stmt_dup(s->op2.stval), stmt_dup(join))
				);
		stmt_destroy(s);
		return res;
	} else if (s->type == st_diff){
		stmt *op1 = stmt_join( join, stmt_dup(s->op1.stval), cmp_equal);
		stmt *res = stmt_diff( op1, stmt_dup(s->op2.stval) );
		stmt_destroy(s);
		return res;
	} else if (s->type == st_semijoin){
		stmt *jn = stmt_dup(join);
		stmt *op1 = stmt_push_join_head( stmt_dup(s->op1.stval), jn);
		stmt *op2 = stmt_push_join_head( stmt_dup(s->op2.stval), jn);
		stmt *res = stmt_semijoin( op1, op2 );
		stmt_destroy(s);
		return res;
	} else if (s->type == st_select){
		if (s->op2.stval->nrcols >= 1){
			stmt *jn = stmt_dup(join);
			stmt *op1 = stmt_push_join_head( stmt_dup(s->op1.stval), jn);
			stmt *op2 = stmt_push_join_head( stmt_dup(s->op2.stval), jn);
			stmt *res = stmt_select( op1, op2, s->flag );
			stmt_destroy(s);
			return res;
		}
	} else {
		printf("= TODO: common/optimize.c: push join head %s\n", st_type2string(s->type));
	}
	stmt_destroy(join);
	return s;
}

static stmt *stmt_push_join_tail(stmt * s, stmt * join){

	if (s->type == st_reljoin){
		list *l1 = list_dup(s->op1.lval, (fdup)&stmt_dup);
		list *l2 = create_stmt_list();
		node *n;
		for (n = s->op2.lval->h; n; n = n->next) {
			l2 = list_append(l2, stmt_join(stmt_reverse(join), stmt_dup(n->data), cmp_equal));
		}
		stmt_destroy(s);
		return stmt_reljoin2(l1,l2);
	} else if (s->type == st_join){
		stmt *op2 = stmt_join( stmt_dup(s->op2.stval), join, cmp_equal);
		stmt *res = stmt_join( stmt_dup(s->op1.stval), op2, s->flag );
		stmt_destroy(s);
		return res;
	} else if (s->type == st_reverse){
		stmt *res = stmt_reverse(
			stmt_push_join_head(stmt_dup(s->op1.stval), stmt_reverse(join)));
		stmt_destroy(s);
		return res;
	} else if (s->type == st_intersect){
		stmt *res = stmt_intersect(
			stmt_push_join_tail(stmt_dup(s->op1.stval), join),
			stmt_push_join_tail(stmt_dup(s->op2.stval), stmt_dup(join))
				);
		stmt_destroy(s);
		return res;
	} else if (s->type == st_diff){
		stmt *op2 = stmt_join( stmt_dup(s->op2.stval), join, cmp_equal);
		stmt *res = stmt_diff( stmt_dup(s->op1.stval), op2 );
		stmt_destroy(s);
		return res;
	} else {
		printf("= TODO: common/optimize.c: push join tail %s\n", st_type2string(s->type));
	}
	stmt_destroy(join);
	return s;
}

static stmt *stmt_join2select(stmt * j){

	if (j->type == st_join){
		stmt *res = stmt_select(stmt_dup(j->op1.stval), 
				stmt_reverse(stmt_dup(j->op2.stval)), j->flag);
		stmt_destroy(j);
		return res;
	} else if (j->type == st_reverse){
		stmt *res = stmt_join2select(stmt_dup(j->op1.stval));
		stmt_destroy(j);
		return res;
	} else if (j->type == st_intersect){
		stmt * res = stmt_semijoin(
			stmt_join2select(stmt_dup(j->op1.stval)),
			stmt_join2select(stmt_dup(j->op2.stval)));
		stmt_destroy(j);
		return res;
	} else {
		printf("= TODO: common/optimize.c: join2select %s\n", st_type2string(j->type));
	}
	return j;
}



static stmt *stmt_push_down_head_wrap( stmt *j, stmt *sel ){
	return stmt_push_down_head(stmt_dup(j), stmt_dup(sel));
}
static stmt *stmt_push_down_tail_wrap( stmt *j, stmt *sel ){
	return stmt_push_down_tail(stmt_dup(j), stmt_dup(sel));
}

/*
 * The push_selects_down function converts a conjunction set into a
 * reduced conjuntion tree where the selects are grouped together and
 * pushed under the join. Also joins are grouped together.
 * */
static list *push_selects_down(list * con){
	int one = 1;
	int two = 2;

	list *rsel = create_stmt_list();
	list *res = create_stmt_list();
	list *sels, *dsels, *joins, *djoins;
	node *n = NULL;
       
	sels = list_select(con, (void*)&one, (fcmp)&stmt_cmp_nrcols, (fdup)&stmt_dup );
	dsels = list_distinct( sels, (fcmp)&stmt_cmp_head_tail, (fdup)&stmt_dup );

	joins = list_select(con, (void*)&two, (fcmp)&stmt_cmp_nrcols, (fdup)&stmt_dup );
	djoins = list_distinct( joins, (fcmp)&stmt_cmp_head_tail, (fdup)&stmt_dup );
	for( n = dsels->h; n; n = n->next){
		list *esels = list_select(sels, n->data, (fcmp)&stmt_cmp_head_tail, (fdup)&stmt_dup );
		stmt *sel = (stmt*)list_reduce(esels, (freduce)&stmt_semijoin, (fdup)&stmt_dup );
		list_destroy(esels);
		/* todo check for foreign key selects */
		list_append(rsel, sel);
	}
	list_destroy(sels);
	list_destroy(dsels);

	/* todo join order rewrites */
	for( n = djoins->h; n; n = n->next){
		list *ejoins = list_select(joins, n->data, (fcmp)&stmt_cmp_head_tail, (fdup)&stmt_dup );
		stmt *join = ejoins->h->data;
		list *eqjoins = NULL, *nqjoins = NULL;
		stmt *reljoin = NULL;

		/* todo check for foreign key joins */
		node *hsel = list_find(rsel, (void*)join, (fcmp)&cmp_sel_head);
		node *tsel = list_find(rsel, (void*)join, (fcmp)&cmp_sel_tail);

		/* push select through all joins */
		if (hsel){
			stmt *sel = hsel->data;
			list *nejoins = list_map(ejoins, sel, 
				(fmap) &stmt_push_down_head_wrap);
			list_destroy(ejoins);
			ejoins = nejoins;
		}
		if (tsel){
			stmt *sel = tsel->data;
			list *nejoins = list_map(ejoins, sel, 
				(fmap) &stmt_push_down_tail_wrap);
			list_destroy(ejoins);
			ejoins = nejoins;
		}

		/* look for multi-att equi-join candidates */
		eqjoins = list_select(ejoins, (void*)1, (fcmp)&join_cmp_eq, (fdup)&stmt_dup );
		if (list_length(eqjoins) > 1) {
			/* rewrite conjuction of equi-join into multi-att equi-join */
			reljoin = stmt_reljoin1(eqjoins);
			/* collect the remaining non-equi joins */
			nqjoins = list_select(ejoins, (void*)1, (fcmp)&join_cmp_neq, (fdup)&stmt_dup );
			list_destroy(ejoins);
		} else {
			/* no multi-att equi-join found; theat all joins equally */
			reljoin = NULL;
			nqjoins = ejoins;
		}
		list_destroy(eqjoins);

		if (list_length(nqjoins) > 0) {
			/* changes to [].().select(TRUE) */
			join = (stmt*)list_reduce(nqjoins, (freduce)&stmt_intersect, (fdup)&stmt_dup );
		} else {
			join = NULL;
		}
		list_destroy(nqjoins);
		
		if (reljoin && join) {
			join = stmt_intersect(reljoin, join);
		} else if (reljoin) {
			join = reljoin;
		}

		list_append(res, join);
	}
	/* find least expensive join (with head/tail selects) */
	/* continue finding usages of this until done */

	list_destroy(joins);
	list_destroy(djoins);
	if (!list_length(res)){
		if (list_length(rsel) == 1){
			list_destroy(res);
			return rsel;
		} else {
			printf("error: more then one selected but no joins\n");
		}
	}
	list_destroy(rsel);
	return res;
}

list *pivot( list *pivots, stmt *st, stmt *p, int markid ){
	list *npivots;
	stmt *j, *pnl, *pnr;
	node *pn;

	node *tp = list_find(pivots, (void*)st, (fcmp)&pivot_cmp_tail);

	j = stmt_push_join_head(st, p);
	if (tp){
		/* convert to a select [op](l,r).select(TRUE); */
		j = stmt_push_join_tail(j, stmt_reverse(stmt_dup(tp->data)));
		j = stmt_join2select(j);
	}

	/*
	pnl = stmt_mark(j, markid);
	*/
	pnr = stmt_mark(stmt_reverse(j), markid);

	/*
	if (tp)
		pnr = stmt_intersect(pnr,pnl);
		*/

	npivots = create_stmt_list();
	if (!tp) {
		pnl = stmt_mark(stmt_dup(j), markid);
		list_append (npivots, pnl );
	}
	for (pn = pivots->h; pn; pn = pn->next){
		list_append (npivots, stmt_join (stmt_dup(pnr), stmt_dup(pn->data), cmp_equal));
	}
	stmt_destroy(pnr);
	return npivots;
}

/* merge_pivot_sets tries to merge pivot sets using the joins in the join set
 * returns a set of pivot sets.
 * */
/*
static list *merge_pivot_sets(list *pivotsets, list *joins)
{
	stmt *st;
	if (list_length(joins) == 0)
		return NULL;
	if (!pivotsets){
		list *pivots = create_stmt_list();
		node *n = joins->h;
		pivotsets = list_create((fdestroy)&list_destroy);
		st = n->data;

		if (st->nrcols == 1) {
			list_append(pivots, 
					stmt_mark(stmt_reverse(st), markid++));
		}
		if (st->nrcols == 2) {
			list_append(pivots, 
					stmt_mark (stmt_reverse(st), markid));
			list_append(pivots, 
					stmt_mark(st, markid++));
		}
		list_append(pivotsets,pivots);
	}
}

static list *pivot_sets(list *joins, int *Markid)
{
	int markid = *Markid;
	list *psets = list_create((fdestroy)&list_destroy);
	node *n;

	for( n = joins->h; n; n = n->next ){
		list *pivots = create_stmt_list();
		stmt *st = n->data;

		if (st->nrcols == 1) {
			list_append(pivots, stmt_mark(stmt_reverse(stmt_dup(st)), markid++));
		}
		if (st->nrcols == 2) {
			list_append(pivots, stmt_mark (stmt_reverse(stmt_dup(st)), markid));
			list_append(pivots, stmt_mark(stmt_dup(st), markid++));
		}
		list_append(psets,pivots);
	}
	*Markid = markid;
	return psets;
}
*/

static stmt *set2pivot(list * l)
{
	list *pivots = create_stmt_list();
	stmt *join, *st;
	node *n;
	int len = 0;
	int markid = 0;

	l = push_selects_down(l);
	n = l->h;

	join = (stmt*)list_reduce(l, (freduce)&stmt_smallest, (fdup)&stmt_dup);
	n = list_find(l, (void*)join, data_cmp);
	stmt_destroy(join);
	if (!n) {
		list_destroy(l);
		list_destroy(pivots);
		return NULL;
	}
	st = n->data;
	if (st->nrcols == 1) {
		list_append(pivots, stmt_mark(stmt_reverse(stmt_dup(st)), markid++));
	}
	if (st->nrcols == 2) {
		list_append(pivots, stmt_mark (stmt_reverse(stmt_dup(st)), markid));
		list_append(pivots, stmt_mark( stmt_dup(st), markid++));
	}
	n = list_remove_node(l, n);
	len = list_length(l) + 1;
	while (list_length(l) > 0 && len > 0) {
		len--;
		n = l->h;
		while (n) {
			stmt *st = n->data;
			node *p = NULL;
			for ( p = pivots->h; p; p = p->next) {
				list *nps = NULL;
				stmt *pv = p->data;
				if (pv->t == st->h) {
					nps = pivot(pivots, stmt_dup(st), stmt_dup(pv), markid++);
					list_destroy(pivots);
					pivots = nps;
					n = list_remove_node(l, n);
					break;
				} else if (pv->t == st->t) {
					nps = pivot(pivots, stmt_reverse(stmt_dup(st)), stmt_dup(pv), markid++);
					list_destroy(pivots);
					pivots = nps;
					n = list_remove_node(l, n);
					break;
				}
			}
			if (n)
				n = n->next;
		}
	}
	if (!len) {
		assert(0);
		return NULL;
	}
	list_destroy(l);
	
	return stmt_list(pivots);
}

/* The group code is needed for double elimination.
 * if a value is selected twice once on the left hand of the
 * 'OR' and once on the right hand of the 'OR' it will be in the
 * result twice.
 *
 * current version is broken, unique also remove normal doubles.
 */
static stmt *sets2pivot(list * ll)
{
	node *n = ll->h;
	if (n) {
		stmt *pivots = set2pivot(n->data);
		n = n->next;
		while (n) {
			stmt *npivots = set2pivot(n->data);
			list *l = npivots->op1.lval;
			list *inserts = create_stmt_list();

			node *m, *c;

			/* we use a special bat insert to garantee unique head 
			 * oids 
			 */
			for (m = l->h ; m; m = m->next) {
				for (c = pivots->op1.lval->h; c; c = c->next){
					stmt *cd = c->data;
					stmt *md = m->data;
					if (cd->t == md->t) {
						list_append (inserts, stmt_append (stmt_dup(cd), stmt_dup(md)));
						break;
					}
				}
			}
			stmt_destroy(pivots);
			stmt_destroy(npivots);
			pivots = stmt_list(inserts);
			n = n->next;
		}
		{
		   group *g = NULL;
		   stmt *u;
		   node *m;
		   list *inserts = create_stmt_list();

		   for( m = pivots->op1.lval->h; m; m = m->next){
		   	g = grp_create(stmt_dup(m->data), g);
		   }
		   u = stmt_reverse( stmt_unique( stmt_reverse( stmt_dup(g->ext) ), NULL));
		   grp_destroy(g);

		   for( m = pivots->op1.lval->h; m; m = m->next){
		   	list_append( inserts, 
				stmt_semijoin( stmt_dup(m->data), stmt_dup(u)));
		   }
		   stmt_destroy(u);
		   stmt_destroy(pivots);
		   return stmt_list(inserts);
		}
		/* no double elimination jet 
		return pivots;
		 */
	}
	return NULL;
}

static stmt *op_stmt2pivot(stmt * s)
{
	if (s->type != st_set && s->type != st_sets) {
		s = stmt_set(s);
	}
	if (s->type == st_sets) {
		stmt *ns = sets2pivot(s->op1.lval);
		stmt_destroy(s);
		s = ns;
	} else {
		stmt *ns = set2pivot(s->op1.lval);
		stmt_destroy(s);
		s = ns;
	}
	return s;
}

static stmt *op_stmt2filter(stmt *s)
{
	if (s->type != st_set && s->type != st_sets) {
		s = stmt_set(s);
	}
	if (s->type == st_sets) {
		printf("#TODO st_sets filter not handled\n" );
	} else {
		list *l = s->op1.lval;
		list *filters = 
			list_select(l, (void*)1, (fcmp)&stmt_cmp_filter, (fdup)&stmt_dup);
		s = list_reduce(filters, (freduce)&stmt_semijoin, (fdup)&stmt_dup);
		list_destroy(filters);
		return s;
	}
	return NULL;
}


static stmt *find_pivot(stmt * subset, stmt * t)
{
	assert(t);
	if (t){
		node *n;
		for (n = subset->op1.lval->h; n; n = n->next) {
			stmt *s = n->data;
			if (s->t == t) 
				return stmt_dup(s);
		}
	}
	assert(0);
	return NULL;
}


stmt *optimize( context *c, stmt *s ){
	stmt *res = s;

	switch(s->type){
	/* first just return those statements which we cannot optimize,
	 * such as schema manipulation, transaction managment, 
	 * and user authentication.
	 */
	case st_none:
	case st_release: case st_commit: case st_rollback: 
	case st_schema: case st_table: case st_column: case st_key: 
	case st_create_schema: case st_drop_schema: 
	case st_create_table: case st_drop_table: 
	case st_create_column: case st_null: case st_default: 
	case st_create_key: 
	case st_create_role: case st_drop_role: 
	case st_grant_role: case st_revoke_role: 
	case st_grant: case st_revoke:

	case st_dbat: case st_obat: case st_basetable: case st_kbat:

	case st_atom: 
	case st_find: 
	case st_bulkinsert: case st_senddata: 
	case st_var:

		s->optimized = 1;
		return stmt_dup(s);

	case st_reljoin: {
		if (s->optimized) 
			return stmt_dup(s);

	{	node *n1, *n2;

		for(n1 = s->op1.lval->h, n2 = s->op2.lval->h; n1 && n2; n1 = n1->next, n2 = n2->next ){
			n1->data = optimize(c, n1->data);
			n2->data = optimize(c, n2->data);
		}
		s->optimized = 1;
		return stmt_dup(s);
	}}

	case st_temp:
	case st_filter: 
	case st_select: case st_select2: case st_like: case st_semijoin: 
	case st_diff: case st_intersect: case st_union: case st_outerjoin:
	case st_join: 
	case st_mirror: case st_reverse: case st_const: case st_mark: 
	case st_group: case st_group_ext: case st_derive: case st_unique: 
	case st_limit: case st_order: case st_reorder: case st_ordered: 

	case st_alias: case st_column_alias: 
	case st_ibat: 
	case st_output: 
	case st_append: case st_insert: case st_replace: 
	case st_exception:

	case st_count: case st_aggr: 
	case st_op: case st_unop: case st_binop: case st_Nop:

		if (s->optimized) 
			return stmt_dup(s);

		if (s->op1.stval){
			stmt *os = s->op1.stval;
		       	stmt *ns = optimize(c, os);
			s->op1.stval = ns;
			stmt_destroy(os);
		}
		if (s->op2.stval){
			stmt *os = s->op2.stval;
		       	stmt *ns = optimize(c, os);
			s->op2.stval = ns;
			stmt_destroy(os);
		}
		if (s->op3.stval){
			stmt *os = s->op3.stval;
		       	stmt *ns = optimize(c, os);
			s->op3.stval = ns;
			stmt_destroy(os);
		}
		s->optimized = 1;
		return stmt_dup(s);

	case st_list: {
		stmt *res = NULL;
		node *n;
		list *l = s->op1.lval;
		list *nl = NULL;

		if (s->optimized) 
			return stmt_dup(s);

		nl = create_stmt_list();
		for(n = l->h; n; n = n->next ){
			stmt *ns = optimize(c, n->data);
			list_append(nl, ns);
		}
		res = stmt_list(nl);
		res->optimized = 1;
		return res;
	} 

	case st_bat: case st_ubat: 
		if (s->optimized) 
			return stmt_dup(s);
		if (s->op1.cval->table->type == tt_view){
			return optimize(c, s->op1.cval->s);
		} else {
			s->optimized = 1;
			return stmt_dup(s);
		}

	case st_pivot: {
		stmt *ns = optimize(c, s->op2.stval); /* optimize ptable */
		stmt *filter = op_stmt2filter(s->op2.stval->op3.stval);
		stmt *np = find_pivot(ns, s->t);
		if (filter)
			np = stmt_semijoin(np, optimize(c, filter));
		stmt_destroy(ns);
		return np;
	}
	case st_partial_pivot: {
		stmt *ns = optimize(c, s->op2.stval); /* optimize ptable */
		stmt *np = find_pivot(ns, s->t);
		stmt_destroy(ns);
		return np;
	}
	case st_ptable:
		if (!s->op4.stval){ /* use op4 to store the new ptable */
			stmt *pivots = op_stmt2pivot(stmt_dup(s->op3.stval));
			/* also optimize the pivots */
			s->op4.stval = optimize(c, pivots);
			assert(s->op4.stval);
			stmt_destroy(pivots);
		}
		return stmt_dup(s->op4.stval);
	case st_set: case st_sets: 
		assert(0); 	/* these should have been rewriten by now */
	}
	return res;
}
