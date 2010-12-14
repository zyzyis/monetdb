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
 * Portions created by CWI are Copyright (C) 1997-July 2008 CWI.
 * Copyright August 2008-2010 MonetDB B.V.
 * All Rights Reserved.
 */


#include "sql_config.h"
#include "rel_updates.h"
#include "rel_semantic.h"
#include "rel_select.h"
#include "rel_exp.h"
#include "rel_subquery.h"
#include "sql_privileges.h"

static sql_exp *
insert_value(mvc *sql, sql_column *c, sql_rel **r, symbol *s)
{
	if (s->token == SQL_NULL) {
		return exp_atom(sql->sa, atom_general(sql->sa, &c->type, NULL));
	} else {
		int is_last = 0;
		exp_kind ek = {type_value, card_value, FALSE};
		sql_exp *e = rel_value_exp2(sql, r, s, sql_sel, ek, &is_last);

		if (!e)
			return(NULL);
		return rel_check_type(sql, &c->type, e, type_equal); 
	}
}

sql_exp ** 
insert_exp_array(sql_table *t, int *Len)
{
	sql_exp **inserts;
	int i, len = list_length(t->columns.set);
	node *m;

	*Len = len;
	inserts = NEW_ARRAY(sql_exp *, len);
	for (m = t->columns.set->h, i = 0; m; m = m->next, i++) {
		sql_column *c = m->data;

		c->colnr = i;
		inserts[i] = NULL;
	}
	return inserts;
}

sql_rel *
rel_insert(sql_allocator *sa, sql_rel *t, sql_rel *inserts)
{
	sql_rel * r = rel_create(sa);

//	if (t->clustered) /* needs a way to propagate ! */
//		t->clustered = 0;
	r->op = op_insert;
	r->l = t;
	r->r = inserts;
	return r;
}

static sql_rel *
rel_insert_cluster(mvc *sql, sql_table *t, sql_rel *inserts)
{
	sql_rel * r = rel_create(sql->sa);

	if (t->cluster) {
		/* order inserts on cluster columns */
		list *obe = new_exp_list(sql->sa);
		node *n, *m;

		for(n=t->cluster->columns->h; n; n = n->next) {
			sql_exp *oe, *e;
			sql_kc *c = n->data;
			int nr;

			nr = c->c->colnr;
			for(m=inserts->exps->h; m && nr; m = m->next) 
				nr--;
			if (nr || !m) {
				/* todo error */
				return NULL;
			}
			e = m->data;
			oe = exp_column(sql->sa, e->rname, e->name, exp_subtype(e), inserts->card, has_nil(e), is_intern(e));
			set_direction(oe, 1 /* ASC */);
			list_append(obe, oe);
		}
		inserts = rel_orderby(sql, inserts, obe);
	}
	r->op = op_insert;
	r->l = rel_basetable(sql->sa, t, t->base.name);
	r->r = inserts;
	return r;
}

static sql_rel *
insert_into(mvc *sql, dlist *qname, dlist *columns, symbol *val_or_q)
{
	int i, len = 0;
	char *sname = qname_schema(qname);
	char *tname = qname_table(qname);
	sql_schema *s = NULL;
	sql_table *t = NULL;
	list *collist = NULL, *exps;
	sql_rel *r = NULL;
	sql_exp **inserts;
	node *n, *m;

	if (sname && !(s=mvc_bind_schema(sql, sname))) {
		(void) sql_error(sql, 02, "INSERT INTO: no such schema '%s'", sname);
		return NULL;
	}
	if (!s)
		s = cur_schema(sql);
	t = mvc_bind_table(sql, s, tname);
	if (!t && !sname) {
		s = tmp_schema(sql);
		t = mvc_bind_table(sql, s, tname);
		if (!t) 
			t = mvc_bind_table(sql, NULL, tname);
	}
	if (!t) {
		return sql_error(sql, 02, "INSERT INTO: no such table '%s'", tname);
	} else if (isView(t)) {
		return sql_error(sql, 02, "INSERT INTO: cannot insert into view '%s'", tname);
	} else if (t->readonly) {
		return sql_error(sql, 02, "INSERT INTO: cannot insert into read only table '%s'", tname);
	}
	if (t && !isTempTable(t) && STORE_READONLY(active_store_type))
		return sql_error(sql, 02, "INSERT INTO: insert into table '%s' not allowed in readonly mode", tname);

	if (!table_privs(sql, t, PRIV_INSERT)) {
		return sql_error(sql, 02, "INSERT INTO: insufficient privileges for user '%s' to insert into table '%s'", stack_get_string(sql, "current_user"), tname);
	}
	if (columns) {
		dnode *n;

		collist = list_create((fdestroy) NULL);
		for (n = columns->h; n; n = n->next) {
			sql_column *c = mvc_bind_column(sql, t, n->data.sval);

			if (c) {
				list_append(collist, c);
			} else {
				return sql_error(sql, 02, "INSERT INTO: no such column '%s.%s'", tname, n->data.sval);
			}
		}
	} else {
		collist = t->columns.set;
	}

	if (val_or_q->token == SQL_VALUES) {
		dlist *rowlist = val_or_q->data.lval;
		dlist *values;
		dnode *o;

		if (!rowlist->h) {
			r = rel_project(sql->sa, NULL, NULL);
			if (!columns)
				collist = NULL;
		}

		for (o = rowlist->h; o; o = o->next) {
			values = o->data.lval;

			if (dlist_length(values) != list_length(collist)) {
				return sql_error(sql, 02, "INSERT INTO: number of values doesn't match number of columns of table '%s'", tname);
			} else {
				sql_rel *inner = NULL;
				sql_rel *i = NULL;
				list *exps = new_exp_list(sql->sa);
				dnode *n;

				for (n = values->h, m = collist->h; n && m; n = n->next, m = m->next) {
					sql_column *c = m->data;
					sql_rel *r = NULL;
					sql_exp *ins = insert_value(sql, c, &r, n->data.sym);
					if (!ins)
						return NULL;
					if (r && inner)
						inner = rel_crossproduct(sql->sa, inner,r, op_join);
					else if (r) 
						inner = r;
					if (!ins->name)
						exp_label(sql->sa, ins, ++sql->label);
					list_append(exps, ins);
				}
				i = rel_project(sql->sa, inner, exps);
				if (r) {
					r = rel_setop(sql->sa, r, i, op_union);
				} else {
					r = i;
				}
			}
		}
	} else {
		exp_kind ek = {type_value, card_relation, TRUE};

		r = rel_subquery(sql, NULL, val_or_q, ek);
	}
	if (!r) 
		return NULL;

	/* In case of missing project, order by or distinct, we need to add	
	   and projection */
	if (r->op != op_project || r->r || need_distinct(r))
		r = rel_project(sql->sa, r, rel_projections(sql, r, NULL, 0, 0));
	if ((r->exps && list_length(r->exps) != list_length(collist)) ||
	   (!r->exps && collist)) 
		return sql_error(sql, 02, "INSERT INTO: query result doesn't match number of columns in table '%s'", tname);

	inserts = insert_exp_array(t, &len);

	if (r->exps) {
		for (n = r->exps->h, m = collist->h; n && m; n = n->next, m = m->next) {
			sql_column *c = m->data;
			sql_exp *e = n->data;
	
			inserts[c->colnr] = rel_check_type(sql, &c->type, e, type_equal);
		}
	}

	for (i = 0; i < len; i++) {
		if (!inserts[i]) {
			for (m = t->columns.set->h; m; m = m->next) {
				sql_column *c = m->data;

				if (c->colnr == i) {
					sql_exp *e = NULL;

					if (c->def) {
						char *q = sql_message( "select %s;", c->def);
						e = rel_parse_val(sql, q, sql->emode);
						_DELETE(q);
						if (!e || (e = rel_check_type(sql, &c->type, e, type_equal)) == NULL)
							return NULL;
					} else {
						atom *a = atom_general(sql->sa, &c->type, NULL);
						e = exp_atom(sql->sa, a);
					}
					if (!e) 
						return sql_error(sql, 02, "INSERT INTO: column '%s' has no valid default value", c->base.name);
					inserts[i] = e;
				}
			}
			assert(inserts[i]);
		}
	}
	/* now rewrite project exps in proper table order */
	exps = new_exp_list(sql->sa);
	for (i = 0; i<len; i++) 
		list_append(exps, inserts[i]);
	_DELETE(inserts);
	list_destroy(r->exps);
	r->exps = exps;
	return rel_insert(sql->sa, rel_basetable(sql->sa, t, tname), r);
}

sql_rel *
rel_update(sql_allocator *sa, sql_rel *t, sql_rel *updates, list *exps)
{
	sql_rel *r = rel_create(sa);

	r->op = op_update;
	r->l = t;
	r->r = updates;
	r->exps = exps;
	return r;
}

static sql_rel *
update_table(mvc *sql, dlist *qname, dlist *assignmentlist, symbol *opt_where)
{
	char *sname = qname_schema(qname);
	char *tname = qname_table(qname);
	sql_schema *s = NULL;
	sql_table *t = NULL;

	if (sname && !(s=mvc_bind_schema(sql,sname))) {
		(void) sql_error(sql, 02, "UPDATE: no such schema '%s'", sname);
		return NULL;
	}
	if (!s)
		s = cur_schema(sql);
	t = mvc_bind_table(sql, s, tname);
	if (!t && !sname) 
		s = tmp_schema(sql);

	t = mvc_bind_table(sql, s, tname);
	if (!t && !s) {
		sql_subtype *tpe;

		if ((tpe = stack_find_type(sql, tname)) != NULL) 
			t = tpe->comp_type;
	}
	if (!t) {
		return sql_error(sql, 02, "UPDATE: no such table '%s'", tname);
	} else if (isView(t)) {
		return sql_error(sql, 02, "UPDATE: cannot update view '%s'", tname);
	} else if (t->readonly) {
		return sql_error(sql, 02, "UPDATE: cannot update read only table '%s'", tname);
	} else {
		sql_exp *e = NULL;
		sql_rel *r = NULL;
		list *exps = new_exp_list(sql->sa);
		dnode *n;

		if (t && !isTempTable(t) && STORE_READONLY(active_store_type))
			return sql_error(sql, 02, "UPDATE: update table '%s' not allowed in readonly mode", tname);

		if (opt_where) {
			int status = sql->session->status;
	
			r = rel_logical_exp(sql, NULL, opt_where, sql_where);
			if (r) { /* simple predicate which is not using the to 
				    be updated table. We add a select all */

				sql_rel *l = rel_basetable(sql->sa, t, t->base.name );
				r = rel_crossproduct(sql->sa, l, r, op_semi);
			} else {
				sql->errstr[0] = 0;
				sql->session->status = status;
				r = rel_basetable(sql->sa, t, t->base.name );
				r = rel_logical_exp(sql, r, opt_where, sql_where);
				if (r && is_join(r->op))
					r->op = op_semi;
			}
			if (!r) 
				return NULL;
		} else {	/* update all */
			r = rel_basetable(sql->sa, t, t->base.name );
		}
	
		/* We simply create a relation %TID%, updates */

		/* first create the project */
		e = exp_column(sql->sa, rel_name(r), "%TID%", sql_bind_localtype("oid"), CARD_MULTI, 0, 1);
		r = rel_project(sql->sa, r, append(new_exp_list(sql->sa),e));
		for (n = assignmentlist->h; n; n = n->next) {
			symbol *a = NULL;
			sql_exp *v = NULL;
			dlist *assignment = n->data.sym->data.lval;
			char *cname = assignment->h->next->data.sval;
			sql_column *c = mvc_bind_column(sql, t, cname);

			if (!c) {
				rel_destroy(r);
				return sql_error(sql, 02, "UPDATE: no such column '%s.%s'", t->base.name, cname);
			}
			a = assignment->h->data.sym;
			if (a) {
				int status = sql->session->status;
				sql_rel *rel_val = NULL;
				exp_kind ek = {type_value, card_column, FALSE};

				v = rel_value_exp(sql, &rel_val, a, sql_sel, ek);

				if (!v) {
					symbol *s = n->data.sym;
					sql->errstr[0] = 0;
					sql->session->status = status;
					//v = rel_value_exp(sql, &r, a, sql_sel, ek);
					s->token = SQL_COLUMN;
					v = rel_column_exp(sql, &r, s, sql_sel);
				}
				if (!v || (v = rel_check_type(sql, &c->type, v, type_equal)) == NULL) {
					rel_destroy(r);
					return NULL;
				}
				if (rel_val) {
					sql_rel *nr;
					list *exps;

					if (!exp_name(v))
						exp_label(sql->sa, v, ++sql->label);
					rel_val = rel_project(sql->sa, rel_val, rel_projections(sql, rel_val, NULL, 0, 1));
					rel_project_add_exp(sql, rel_val, v);
					exps = rel_projections(sql, r, NULL, 0, 1);
					nr = rel_project(sql->sa, rel_crossproduct(sql->sa, rel_dup(r->l), rel_val, op_join), exps);
					rel_destroy(r);
					r = nr;
					v = exp_column(sql->sa, NULL, exp_name(v), exp_subtype(v), v->card, has_nil(v), is_intern(v));
				}		
			} else {
				v = exp_atom(sql->sa, atom_general(sql->sa, &c->type, NULL));
			}

			if (!v) {
				rel_destroy(r);
				return NULL;
			}
			list_append(exps, exp_column(sql->sa, t->base.name, cname, &c->type, CARD_MULTI, 0, 0));
			rel_project_add_exp(sql, r, v);
		}
		r = rel_update(sql->sa, rel_basetable(sql->sa, t, tname), r, exps);
		return r;
	}
}

sql_rel *
rel_delete(sql_allocator *sa, sql_rel *t, sql_rel *deletes)
{
	sql_rel *r = rel_create(sa);

	r->op = op_delete;
	r->l = t;
	r->r = deletes;
	return r;
}

static sql_rel *
delete_table(mvc *sql, dlist *qname, symbol *opt_where)
{
	char *sname = qname_schema(qname);
	char *tname = qname_table(qname);
	sql_schema *schema = NULL;
	sql_table *t = NULL;

	if (sname && !(schema=mvc_bind_schema(sql, sname))) {
		(void) sql_error(sql, 02, "DELETE FROM: no such schema '%s'", sname);
		return NULL;
	}
	if (!schema)
		schema = cur_schema(sql);
	t = mvc_bind_table(sql, schema, tname);
	if (!t && !sname) {
		schema = tmp_schema(sql);
		t = mvc_bind_table(sql, schema, tname);
		if (!t) {
			sql_subtype *tpe = stack_find_type(sql, tname);
			if (tpe)
				t = tpe->comp_type;
		}
	}
	if (!t) {
		return sql_error(sql, 02, "DELETE FROM: no such table '%s'", tname);
	} else if (isView(t)) {
		return sql_error(sql, 02, "DELETE FROM: cannot delete from view '%s'", tname);
	} else if (t->readonly) {
		return sql_error(sql, 02, "DELETE FROM: cannot delete from read only table '%s'", tname);
	}
	if (t && !isTempTable(t) && STORE_READONLY(active_store_type))
		return sql_error(sql, 02, "DELETE FROM: delete from table '%s' not allowed in readonly mode", tname);
	if (!table_privs(sql, t, PRIV_DELETE)) {
		return sql_error(sql, 02, "DELETE FROM: insufficient privileges for user '%s' to delete from table '%s'", stack_get_string(sql, "current_user"), tname);
	} else {
		sql_rel *r = NULL;

		if (opt_where) {
			int status = sql->session->status;

			r = rel_logical_exp(sql, NULL, opt_where, sql_where);
			if (r) { /* simple predicate which is not using the to 
		    		    be updated table. We add a select all */
				sql_rel *l = rel_basetable(sql->sa, t, t->base.name );
				r = rel_crossproduct(sql->sa, l, r, op_join);
			} else {
				sql->errstr[0] = 0;
				sql->session->status = status;
				r = rel_basetable(sql->sa, t, t->base.name );
				r = rel_logical_exp(sql, r, opt_where, sql_where);
			}
			if (!r) {
				return NULL;
			} else {
				sql_exp *e = exp_column(sql->sa, rel_name(r), "%TID%", sql_bind_localtype("oid"), CARD_MULTI, 0, 1);

				r = rel_project(sql->sa, r, append(new_exp_list(sql->sa), e));

			}
			r = rel_delete(sql->sa, rel_basetable(sql->sa, t, tname), r);
		} else {	/* delete all */
			r = rel_delete(sql->sa, rel_basetable(sql->sa, t, tname), NULL);
		}
		return r;
	}
}

static sql_rel *
rel_import(mvc *sql, sql_table *t, char *tsep, char *rsep, char *ssep, char *ns, char *filename, lng nr, lng offset, int locked)
{
	sql_rel *res;
	list *exps, *args;
	node *n;
	sql_subtype tpe;
	sql_exp *import;
	sql_schema *sys = mvc_bind_schema(sql, "sys");
	int len = 7 + (filename?1:0);
	sql_subfunc *f = sql_find_func(sql->sa, sys, "copyfrom", len); 
	
	f->res.comp_type = t;
 	sql_find_subtype(&tpe, "varchar", 0, 0);
	args = append( append( append( append( append( append( new_exp_list(sql->sa), 
		exp_atom_str(sql->sa, t->s?t->s->base.name:NULL, &tpe)), 
		exp_atom_str(sql->sa, t->base.name, &tpe)), 
		exp_atom_str(sql->sa, tsep, &tpe)), 
		exp_atom_str(sql->sa, rsep, &tpe)), 
		exp_atom_str(sql->sa, ssep, &tpe)), 
		exp_atom_str(sql->sa, ns, &tpe));

	if (filename)
		append( args, exp_atom_str(sql->sa, filename, &tpe)); 
	import = exp_op(sql->sa,  
		append(
			append( 
				append( args, 
					exp_atom_lng(sql->sa, nr)), 
					exp_atom_lng(sql->sa, offset)), 
					exp_atom_int(sql->sa, locked)), f); 
	
	exps = new_exp_list(sql->sa);
	for (n = t->columns.set->h; n; n = n->next) {
		sql_column *c = n->data;
		append(exps, exp_column(sql->sa, t->base.name, c->base.name, &c->type, CARD_MULTI, c->null, 0));
	}
	res = rel_table_func(sql->sa, import, exps);
	return res;
}

static sql_rel *
copyfrom(mvc *sql, dlist *qname, dlist *files, dlist *seps, dlist *nr_offset, str null_string, int locked)
{
	sql_rel *rel = NULL;
	char *sname = qname_schema(qname);
	char *tname = qname_table(qname);
	sql_schema *s = NULL;
	sql_table *t = NULL;
	char *tsep = seps->h->data.sval;
	char *rsep = seps->h->next->data.sval;
	char *ssep = (seps->h->next->next)?seps->h->next->next->data.sval:NULL;
	char *ns = (null_string)?null_string:"null";
	lng nr = (nr_offset)?nr_offset->h->data.l_val:-1;
	lng offset = (nr_offset)?nr_offset->h->next->data.l_val:0;

	assert(!nr_offset || nr_offset->h->type == type_lng);
	assert(!nr_offset || nr_offset->h->next->type == type_lng);
	if (sname && !(s=mvc_bind_schema(sql, sname))) {
		(void) sql_error(sql, 02, "COPY INTO: no such schema '%s'", sname);
		return NULL;
	}
	if (!s)
		s = cur_schema(sql);
	t = mvc_bind_table(sql, s, tname);
	if (!t && !sname) {
		s = tmp_schema(sql);
		t = mvc_bind_table(sql, s, tname);
		if (!t) {
			sql_subtype *tpe = stack_find_type(sql, tname);
			if (tpe)
				t = tpe->comp_type;
		}
	}
	if (!t) 
		return sql_error(sql, 02, "COPY INTO: no such table '%s'", tname);
	if (t->readonly) 
		return sql_error(sql, 02, "COPY INTO: cannot copy into read only table '%s'", tname);
	if (t && !isTempTable(t) && STORE_READONLY(active_store_type))
		return sql_error(sql, 02, "COPY INTO: copy into table '%s' not allowed in readonly mode", tname);
	if (files) {
		dnode *n = files->h;
#ifdef HAVE_REALPATH
		char realdbfarm[PATH_MAX];
		char realfile[PATH_MAX];
#endif

		if (sql->user_id != USER_MONETDB)
			return sql_error(sql, 02, "COPY INTO: insufficient privileges: "
					"COPY INTO from file(s) requires administrator rights, "
					"use 'COPY INTO \"%s\" FROM STDIN' instead", tname);

#ifdef HAVE_REALPATH
		if (realpath(GDKgetenv("gdk_dbfarm"), realdbfarm) == NULL) {
			return sql_error(sql, 02, "COPY INTO: failed to get real path for dbfarm '%s'",
					GDKgetenv("gdk_dbfarm"));
		}
#endif

		for (; n; n = n->next) {
			char *fname = n->data.sval;
			sql_rel *nrel;

			if (fname && !MT_path_absolute(fname))
				return sql_error(sql, 02, "COPY INTO: filename must "
						"have absolute path: %s", fname);
#ifdef HAVE_REALPATH
			if (realpath(fname, realfile) == NULL) {
				return sql_error(sql, 02, "COPY INTO: failed to get real path for '%s'",
						fname);
			}
			if (strncmp(realdbfarm, realfile, strlen(realdbfarm)) == 0)
				return sql_error(sql, 02, "COPY INTO: file may not "
						"reside in the server's dbfarm: %s", realfile);
#endif

			nrel = rel_import(sql, t, tsep, rsep, ssep, ns, fname, nr, offset, locked);

			if (!rel)
				rel = nrel;
			else
				rel = rel_setop(sql->sa, rel, nrel, op_union);
			if (!rel)
				return rel;
		}
	} else {
		rel = rel_import(sql, t, tsep, rsep, ssep, ns, NULL, nr, offset, locked);
	}
	if (!rel)
		return rel;
	return rel_insert_cluster(sql, t, rel);
}

static sql_rel *
bincopyfrom(mvc *sql, dlist *qname, dlist *files)
{
	char *sname = qname_schema(qname);
	char *tname = qname_table(qname);
	sql_schema *s = NULL;
	sql_table *t = NULL;

	dnode *dn;
	node *n;
	sql_rel *res;
	list *exps, *args;
	sql_subtype tpe;
	sql_exp *import;
	sql_schema *sys = mvc_bind_schema(sql, "sys");
	sql_subfunc *f = sql_find_func(sql->sa, sys, "copyfrom", 2); 

	if (sql->user_id != USER_MONETDB) {
		(void) sql_error(sql, 02, "COPY INTO: insufficient privileges: "
				"binary COPY INTO requires administrator rights");
		return NULL;
	}

	if (sname && !(s=mvc_bind_schema(sql, sname))) {
		(void) sql_error(sql, 02, "COPY INTO: no such schema '%s'", sname);
		return NULL;
	}
	if (!s)
		s = cur_schema(sql);
	t = mvc_bind_table(sql, s, tname);
	if (!t && !sname) {
		s = tmp_schema(sql);
		t = mvc_bind_table(sql, s, tname);
		if (!t) {
			sql_subtype *tpe = stack_find_type(sql, tname);
			if (tpe)
				t = tpe->comp_type;
		}
	}
	if (!t) 
		return sql_error(sql, 02, "COPY INTO: no such table '%s'", tname);
	if (t->readonly) 
		return sql_error(sql, 02, "COPY INTO: cannot copy into read only table '%s'", tname);
	if (t && !isTempTable(t) && STORE_READONLY(active_store_type))
		return sql_error(sql, 02, "COPY INTO: copy into table '%s' not allowed in readonly mode", tname);
	if (files == NULL)
		return sql_error(sql, 02, "COPY INTO: must specify files");

	f->res.comp_type = t;
 	sql_find_subtype(&tpe, "varchar", 0, 0);
	args = append( append( new_exp_list(sql->sa), 
		exp_atom_str(sql->sa, t->s?t->s->base.name:NULL, &tpe)), 
		exp_atom_str(sql->sa, t->base.name, &tpe));

	for (dn = files->h; dn; dn = dn->next) {
		append(args, exp_atom_str(sql->sa, dn->data.sval, &tpe)); 

		/* extend the bincopyfrom, with extra args and types */
	}
	
	import = exp_op(sql->sa,  args, f); 

	exps = new_exp_list(sql->sa);
	for (n = t->columns.set->h; n; n = n->next) {
		sql_column *c = n->data;
		append(exps, exp_column(sql->sa, t->base.name, c->base.name, &c->type, CARD_MULTI, c->null, 0));
	}
	res = rel_table_func(sql->sa, import, exps);
	return rel_insert_cluster(sql, t, res);
}

static sql_rel *
rel_output(mvc *sql, sql_rel *l, sql_exp *sep, sql_exp *rsep, sql_exp *ssep, sql_exp *null_string, sql_exp *file) 
{
	sql_rel *rel = rel_create(sql->sa);
	list *exps = new_exp_list(sql->sa);

	append(exps, sep);
	append(exps, rsep);
	append(exps, ssep);
	append(exps, null_string);
	if (file)
		append(exps, file);
	rel->l = l;
	rel->r = NULL;
	rel->op = op_ddl;
	rel->flag = DDL_OUTPUT;
	rel->exps = exps;
	rel->card = 0;
	rel->nrcols = 0;
	return rel;
}

static sql_rel *
copyto(mvc *sql, symbol *sq, str filename, dlist *seps, str null_string)
{
#ifdef HAVE_REALPATH
	char realdbfarm[PATH_MAX];
	char realfile[PATH_MAX];
#endif
	char *tsep = seps->h->data.sval;
	char *rsep = seps->h->next->data.sval;
	char *ssep = (seps->h->next->next)?seps->h->next->next->data.sval:"\"";
	char *ns = (null_string)?null_string:"null";
	sql_exp *tsep_e, *rsep_e, *ssep_e, *ns_e, *fname_e;
	exp_kind ek = {type_value, card_relation, TRUE};
	sql_rel *r = rel_subquery(sql, NULL, sq, ek);

	if (!r) 
		return NULL;

	tsep_e = exp_atom_clob(sql->sa, tsep);
	rsep_e = exp_atom_clob(sql->sa, rsep);
	ssep_e = exp_atom_clob(sql->sa, ssep);
	ns_e = exp_atom_clob(sql->sa, ns);
	fname_e = filename?exp_atom_clob(sql->sa, filename):NULL;

	if (filename) {
		struct stat fs;
		if (sql->user_id != USER_MONETDB)
			return sql_error(sql, 02, "COPY INTO: insufficient privileges: "
					"COPY INTO file requires administrator rights, "
					"use 'COPY ... INTO STDOUT' instead");
		if (filename && *filename != '/')
			return sql_error(sql, 02, "COPY INTO: filename must "
					"have absolute path: %s", filename);
#ifdef HAVE_REALPATH
		if (realpath(GDKgetenv("gdk_dbfarm"), realdbfarm) == NULL) {
			return sql_error(sql, 02, "COPY INTO: failed to get real path for dbfarm '%s'",
					GDKgetenv("gdk_dbfarm"));
		}
		if (realpath(filename, realfile) == NULL) {
			return sql_error(sql, 02, "COPY INTO: failed to get real path for '%s'",
					filename);
		}
		if (strncmp(realdbfarm, realfile, strlen(realdbfarm)) == 0)
			return sql_error(sql, 02, "COPY INTO: file may not "
					"reside in the server's dbfarm: %s", realfile);
#endif
		if (lstat(filename, &fs) == 0)
			return sql_error(sql, 02, "COPY INTO: file already "
					"exists: %s", filename);
	}

	return rel_output(sql, r, tsep_e, rsep_e, ssep_e, ns_e, fname_e);
}


sql_rel *
rel_updates(mvc *sql, symbol *s)
{
	sql_rel *ret = NULL;

	switch (s->token) {
	case SQL_COPYFROM:
	{
		dlist *l = s->data.lval;

		ret = copyfrom(sql, l->h->data.lval, l->h->next->data.lval, l->h->next->next->data.lval, l->h->next->next->next->data.lval, l->h->next->next->next->next->data.sval, l->h->next->next->next->next->next->data.i_val);
		sql->type = Q_UPDATE;
	}
		break;
	case SQL_BINCOPYFROM:
	{
		dlist *l = s->data.lval;

		ret = bincopyfrom(sql, l->h->data.lval, l->h->next->data.lval);
		sql->type = Q_UPDATE;
	}
		break;
	case SQL_COPYTO:
	{
		dlist *l = s->data.lval;

		ret = copyto(sql, l->h->data.sym, l->h->next->data.sval, l->h->next->next->data.lval, l->h->next->next->next->data.sval);
		sql->type = Q_UPDATE;
	}
		break;
	case SQL_INSERT:
	{
		dlist *l = s->data.lval;

		ret = insert_into(sql, l->h->data.lval, l->h->next->data.lval, l->h->next->next->data.sym);
		sql->type = Q_UPDATE;
	}
		break;
	case SQL_UPDATE:
	{
		dlist *l = s->data.lval;

		ret = update_table(sql, l->h->data.lval, l->h->next->data.lval, l->h->next->next->data.sym);
		sql->type = Q_UPDATE;
	}
		break;
	case SQL_DELETE:
	{
		dlist *l = s->data.lval;

		ret = delete_table(sql, l->h->data.lval, l->h->next->data.sym);
		sql->type = Q_UPDATE;
	}
		break;
	default:
		return sql_error(sql, 01, "Updates statement unknown Symbol(" PTRFMT ")->token = %s", PTRFMTCAST s, token2string(s->token));
	}
	return ret;
}
