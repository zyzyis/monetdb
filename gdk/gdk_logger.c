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
 * Copyright August 2008-2014 MonetDB B.V.
 * All Rights Reserved.
 */

/*
 * (author) N. J. Nes
 *
 * In the philosophy of MonetDB, transaction management overhead
 * should only be paid when necessary. Transaction management is for
 * this purpose implemented as a separate module and applications are
 * required to obey the transaction policy, e.g. obtaining/releasing
 * locks.
 *
 * This module is designed to support efficient logging of the SQL
 * database.  Once loaded, the SQL compiler will insert the proper
 * calls at transaction commit to include the changes in the log file.
 *
 * The logger uses a directory to store its log files. One master log
 * file stores information about the version of the logger and the
 * transaction log files. This file is a simple ascii file with the
 * following format:
 *  {6DIGIT-VERSION\n[log file number \n]*]*}
 * The transaction log files have a binary format, which stores fixed
 * size logformat headers (flag,nr,bid), where the flag is the type of
 * update logged.  The nr field indicates how many changes there were
 * (in case of inserts/deletes).  The bid stores the bid identifier.
 *
 * The key decision to be made by the user is the location of the log
 * file.  Ideally, it should be stored in fail-safe environment, or at
 * least the log and databases should be on separate disk columns.
 *
 * This file system may reside on the same hardware as the database
 * server and therefore the writes are done to the same disk, but
 * could also reside on another system and then the changes are
 * flushed through the network.  The logger works under the assumption
 * that it is called to safeguard updates on the database when it has
 * an exclusive lock on the latest version. This lock should be
 * guaranteed by the calling transaction manager first.
 *
 * Finding the updates applied to a BAT is relatively easy, because
 * each BAT contains a delta structure. On commit these changes are
 * written to the log file and the delta management is reset. Since
 * each commit is written to the same log file, the beginning and end
 * are marked by a log identifier.
 *
 * A server restart should only (re)process blocks which are
 * completely written to disk. A log replay therefore ends in a commit
 * or abort on the changed bats. Once all logs have been read, the
 * changes to the bats are made persistent, i.e. a bbp sub-commit is
 * done.
 */
#include "monetdb_config.h"
#include "gdk.h"
#include "gdk_private.h"
#include "gdk_logger.h"
#include <string.h>

/*
 * The log record encoding is geared at reduced storage space, but at
 * the expense of readability. A user can not easily inspect the log a
 * posteriori to check what has happened.
 *
 */
#define LOG_START	1
#define LOG_END		2
#define LOG_INSERT	3
#define LOG_DELETE	4
#define LOG_UPDATE	5
#define LOG_CREATE	6
#define LOG_DESTROY	7
#define LOG_USE		8
#define LOG_CLEAR	9
#define LOG_SEQ		10

static char *log_commands[] = {
	NULL,
	"LOG_START",
	"LOG_END",
	"LOG_INSERT",
	"LOG_DELETE",
	"LOG_UPDATE",
	"LOG_CREATE",
	"LOG_DESTROY",
	"LOG_USE",
	"LOG_CLEAR",
	"LOG_SEQ",
};

typedef struct logformat_t {
	char flag;
	int tid;
	lng nr;
} logformat;

static int bm_commit(logger *lg);
static int tr_grow(trans *tr);

static void
logbat_destroy(BAT *b)
{
	if (b)
		BBPunfix(b->batCacheid);
}

static BAT *
logbat_new(int tt, BUN size, int role)
{
	BAT *nb = BATnew(TYPE_void, tt, size, role);

	if (nb) {
		BATseqbase(nb, 0);
		nb->batDirty |= 2;
		if (role == PERSISTENT)
			BATmode(nb, PERSISTENT);
	} else {
		fprintf(stderr, "!ERROR: logbat_new: creating new BAT[void:%s]#" BUNFMT " failed\n", ATOMname(tt), size);
	}
	return nb;
}

static int
log_read_format_old(logger *l, logformat *data)
{
	int nr = 0;
	int ok =  mnstr_read(l->log, &data->flag, 1, 1) == 1 &&
		mnstr_readInt(l->log, &nr) == 1 &&
		mnstr_readInt(l->log, &data->tid) == 1;
	data->nr = nr;
	return ok;
}

static int
log_read_format(logger *l, logformat *data)
{
	if (l->postfuncp) /* we need to convert from the old logformat, 
       		             needs to be removed once we released Aug2014 */
		return log_read_format_old(l, data);
	return mnstr_read(l->log, &data->flag, 1, 1) == 1 &&
		mnstr_readLng(l->log, &data->nr) == 1 &&
		mnstr_readInt(l->log, &data->tid) == 1;
}

static int
log_write_format(logger *l, logformat *data)
{
	if (mnstr_write(l->log, &data->flag, 1, 1) == 1 &&
	    mnstr_writeLng(l->log, data->nr) &&
	    mnstr_writeInt(l->log, data->tid))
		return LOG_OK;
	fprintf(stderr, "!ERROR: log_write_format: write failed\n");
	return LOG_ERR;
}

static char *
log_read_string(logger *l)
{
	int len;
	ssize_t nr;
	char *buf;

	if (mnstr_readInt(l->log, &len) != 1) {
		fprintf(stderr, "!ERROR: log_read_string: read failed\n");
		return NULL;
	}
	if (len == 0)
		return NULL;
	buf = GDKmalloc(len);
	if (buf == NULL) {
		fprintf(stderr, "!ERROR: log_read_string: malloc failed\n");
		return NULL;
	}

	if ((nr = mnstr_read(l->log, buf, 1, len)) != (ssize_t) len) {
		buf[len - 1] = 0;
		fprintf(stderr, "!ERROR: log_read_string: couldn't read name (%s) " SSZFMT "\n", buf, nr);
		GDKfree(buf);
		return NULL;
	}
	buf[len - 1] = 0;
	return buf;
}

static int
log_write_string(logger *l, const char *n)
{
	size_t len = strlen(n) + 1;	/* log including EOS */

	assert(len > 1);
	assert(len <= INT_MAX);
	if (!mnstr_writeInt(l->log, (int) len) ||
	    mnstr_write(l->log, n, 1, len) != (ssize_t) len) {
		fprintf(stderr, "!ERROR: log_write_string: write failed\n");
		return LOG_ERR;
	}
	return LOG_OK;
}

static void
log_read_clear(logger *lg, trans *tr, char *name)
{
	if (lg->debug & 1)
		fprintf(stderr, "#logger found log_read_clear %s\n", name);

	if (tr_grow(tr)) {
		tr->changes[tr->nr].type = LOG_CLEAR;
		tr->changes[tr->nr].name = GDKstrdup(name);
		tr->nr++;
	}
}

static void
la_bat_clear(logger *lg, logaction *la)
{
	log_bid bid = logger_find_bat(lg, la->name);
	BAT *b;

	if (lg->debug & 1)
		fprintf(stderr, "#la_bat_clear %s\n", la->name);
	/* do we need to skip these old updates */
	if (BATcount(lg->snapshots_bid)) {
		BUN p = BUNfnd(lg->snapshots_bid, &bid);

		if (p != BUN_NONE) {
			int tid = *(int *) Tloc(lg->snapshots_tid, p);

			if (lg->tid <= tid)
				return;
		}
	}

	b = BATdescriptor(bid);
	if (b) {
		int access = b->batRestricted;
		b->batRestricted = BAT_WRITE;
		BATclear(b, TRUE);
		b->batRestricted = access;
		logbat_destroy(b);
	}
}

static int
log_read_seq(logger *lg, logformat *l)
{
	int seq = (int) l->nr;
	lng val;
	BUN p;

	assert(l->nr <= (lng) INT_MAX);
	if (mnstr_readLng(lg->log, &val) != 1) {
		fprintf(stderr, "!ERROR: log_read_seq: read failed\n");
		return LOG_ERR;
	}

	if ((p = BUNfnd(lg->seqs_id, &seq)) != BUN_NONE) {
		BUNdelete(lg->seqs_id, p, FALSE);
		BUNdelete(lg->seqs_val, p, FALSE);
	}
	BUNappend(lg->seqs_id, &seq, FALSE);
	BUNappend(lg->seqs_val, &val, FALSE);
	return LOG_OK;
}

static int
log_read_updates(logger *lg, trans *tr, logformat *l, char *name)
{
	log_bid bid = logger_find_bat(lg, name);
	BAT *b = BATdescriptor(bid);
	int res = LOG_OK;
	int ht = -1, tt = -1, hseq = 0, tseq = 0;

	if (lg->debug & 1)
		fprintf(stderr, "#logger found log_read_updates %s %s " LLFMT "\n", name, l->flag == LOG_INSERT ? "insert" : l->flag == LOG_DELETE ? "delete" : "update", l->nr);

	if (b) {
		ht = b->htype;
		if (ht == TYPE_void && b->hseqbase != oid_nil)
			hseq = 1;
		tt = b->ttype;
		if (tt == TYPE_void && b->tseqbase != oid_nil)
			tseq = 1;
	} else {		/* search trans action for create statement */
		int i;

		for (i = 0; i < tr->nr; i++) {
			if (tr->changes[i].type == LOG_CREATE && strcmp(tr->changes[i].name, name) == 0) {
				ht = tr->changes[i].ht;
				if (ht < 0) {
					hseq = 1;
					ht = TYPE_void;
				}
				tt = tr->changes[i].tt;
				if (tt < 0) {
					tseq = 1;
					tt = TYPE_void;
				}
				break;
			}
		}
	}
	if (ht >= 0 && tt >= 0) {
		BAT *r;
		void *(*rt) (ptr, stream *, size_t) = BATatoms[tt].atomRead;
		void *tv = ATOMnil(tt);

#if SIZEOF_OID == 8
		if (tt == TYPE_oid && lg->read32bitoid)
			rt = BATatoms[TYPE_int].atomRead;
#endif
		assert(l->nr <= (lng) BUN_MAX);
		r = BATnew(ht, tt, (BUN) l->nr, PERSISTENT);

		if (hseq)
			BATseqbase(r, 0);
		if (tseq)
			BATseqbase(BATmirror(r), 0);

		if (ht == TYPE_void && l->flag == LOG_INSERT) {
			for (; l->nr > 0; l->nr--) {
				void *t = rt(tv, lg->log, 1);

				if (!t) {
					res = LOG_ERR;
					break;
				}
				if (l->flag == LOG_INSERT) {
#if SIZEOF_OID == 8
					if (tt == TYPE_oid && lg->read32bitoid) {
						int vi = * (int *) t;
						if (vi == int_nil)
							* (oid *) t = oid_nil;
						else
							* (oid *) t = vi;
					}
#endif
					BUNappend(r, t, TRUE);
				}
				if (t != tv)
					GDKfree(t);
			}
		} else {
			void *(*rh) (ptr, stream *, size_t) = ht == TYPE_void ? BATatoms[TYPE_oid].atomRead : BATatoms[ht].atomRead;
			void *hv = ATOMnil(ht);

#if SIZEOF_OID == 8
			if ((ht == TYPE_oid || ht == TYPE_void) &&
			    lg->read32bitoid)
				rh = BATatoms[TYPE_int].atomRead;
#endif
			for (; l->nr > 0; l->nr--) {
				void *h = rh(hv, lg->log, 1);
				void *t = rt(tv, lg->log, 1);

				if (!h || !t) {
					res = LOG_ERR;
					break;
				}
#if SIZEOF_OID == 8
				if (lg->read32bitoid) {
					if (ht == TYPE_void || ht == TYPE_oid) {
						int vi = * (int *) h;
						if (vi == int_nil)
							* (oid *) h = oid_nil;
						else
							* (oid *) h = vi;
					}
					if (tt == TYPE_oid) {
						int vi = * (int *) t;
						if (vi == int_nil)
							* (oid *) t = oid_nil;
						else
							* (oid *) t = vi;
					}
				}
#endif
				BUNins(r, h, t, TRUE);
				if (h != hv)
					GDKfree(h);
				if (t != tv)
					GDKfree(t);
			}
			GDKfree(hv);
		}
		GDKfree(tv);
		logbat_destroy(b);

		if (tr_grow(tr)) {
			tr->changes[tr->nr].type = l->flag;
			tr->changes[tr->nr].nr = l->nr;
			tr->changes[tr->nr].ht = ht;
			tr->changes[tr->nr].tt = tt;
			tr->changes[tr->nr].name = GDKstrdup(name);
			tr->changes[tr->nr].b = r;
			tr->nr++;
		}
	} else {
		/* bat missing ERROR or ignore ? currently error. */
		res = LOG_ERR;
	}
	return res;
}

static void
la_bat_updates(logger *lg, logaction *la)
{
	log_bid bid = logger_find_bat(lg, la->name);
	BAT *b;

	if (bid == 0)
		return;		/* ignore bats no longer in the catalog */

	/* do we need to skip these old updates */
	if (BATcount(lg->snapshots_bid)) {
		BUN p = BUNfnd(lg->snapshots_bid, &bid);

		if (p != BUN_NONE) {
			int tid = *(int *) Tloc(lg->snapshots_tid, p);

			if (lg->tid <= tid)
				return;
		}
	}

	b = BATdescriptor(bid);
	assert(b);
	if (b) {
		if (b->htype == TYPE_void && la->type == LOG_INSERT) {
			BATappend(b, la->b, TRUE);
		} else {
			if (la->type == LOG_INSERT)
				BATins(b, la->b, TRUE);
			else if (la->type == LOG_DELETE)
				BATdel(b, la->b, TRUE);
			else if (la->type == LOG_UPDATE) {
				BATiter bi = bat_iterator(la->b);
				BUN p, q;

				BATloop(la->b, p, q) {
					const void *h = BUNhead(bi, p);
					const void *t = BUNtail(bi, p);

					if (BUNfnd(BATmirror(b), h) == BUN_NONE) {
						/* if value doesn't
						 * exist, insert it if
						 * b void headed,
						 * maintain that by
						 * inserting nils */
						if (b->htype == TYPE_void) {
							if (b->batCount == 0 && *(const oid *) h != oid_nil)
								b->hseqbase = *(const oid *) h;
							if (b->hseqbase != oid_nil && *(const oid *) h != oid_nil) {
								const void *tv = ATOMnilptr(b->ttype);

								while (b->hseqbase + b->batCount < *(const oid *) h)
									BUNappend(b, tv, TRUE);
							}
							BUNappend(b, t, TRUE);
						} else {
							BUNins(b, h, t, TRUE);
						}
					} else {
						BUNreplace(b, h, t, TRUE);
					}
				}
			}
		}
		logbat_destroy(b);
	}
}

static void
log_read_destroy(logger *lg, trans *tr, char *name)
{
	(void) lg;
	if (tr_grow(tr)) {
		tr->changes[tr->nr].type = LOG_DESTROY;
		tr->changes[tr->nr].name = GDKstrdup(name);
		tr->nr++;
	}
}

static void
la_bat_destroy(logger *lg, logaction *la)
{
	log_bid bid = logger_find_bat(lg, la->name);

	if (bid) {
		BUN p;

		logger_del_bat(lg, bid);
		if ((p = BUNfnd(lg->snapshots_bid, &bid)) != BUN_NONE) {
#ifndef NDEBUG
			assert(BBP_desc(bid)->S.role == PERSISTENT);
			assert(0 <= BBP_desc(bid)->H.heap.farmid && BBP_desc(bid)->H.heap.farmid < MAXFARMS);
			assert(BBPfarms[BBP_desc(bid)->H.heap.farmid].roles & (1 << PERSISTENT));
			if (BBP_desc(bid)->H.vheap) {
				assert(0 <= BBP_desc(bid)->H.vheap->farmid && BBP_desc(bid)->H.vheap->farmid < MAXFARMS);
				assert(BBPfarms[BBP_desc(bid)->H.vheap->farmid].roles & (1 << PERSISTENT));
			}
			assert(0 <= BBP_desc(bid)->T.heap.farmid && BBP_desc(bid)->T.heap.farmid < MAXFARMS);
			assert(BBPfarms[BBP_desc(bid)->T.heap.farmid].roles & (1 << PERSISTENT));
			if (BBP_desc(bid)->T.vheap) {
				assert(0 <= BBP_desc(bid)->T.vheap->farmid && BBP_desc(bid)->T.vheap->farmid < MAXFARMS);
				assert(BBPfarms[BBP_desc(bid)->T.vheap->farmid].roles & (1 << PERSISTENT));
			}
#endif
			BUNdelete(lg->snapshots_bid, p, FALSE);
			BUNdelete(lg->snapshots_tid, p, FALSE);
			BUNappend(lg->snapshots_bid, &bid, FALSE);
			BUNappend(lg->snapshots_tid, &lg->tid, FALSE);
		}
	}
}

static int
log_read_create(logger *lg, trans *tr, char *name)
{
	char *buf = log_read_string(lg);

	if (lg->debug & 1)
		fprintf(stderr, "#log_read_create %s\n", name);

	if (!buf) {
		return LOG_ERR;
	} else {
		int ht, tt;
		char *ha = buf, *ta = strchr(buf, ',');

		if (!ta) {
			fprintf(stderr, "!ERROR: log_read_create: inconsistent data read\n");
			return LOG_ERR;
		}
		*ta = 0;
		ta++;		/* skip over , */
		if (strcmp(ha, "vid") == 0) {
			ht = -1;
		} else {
			ht = ATOMindex(ha);
		}
		if (strcmp(ta, "vid") == 0) {
			tt = -1;
		} else {
			tt = ATOMindex(ta);
		}
		if (tr_grow(tr)) {
			tr->changes[tr->nr].type = LOG_CREATE;
			tr->changes[tr->nr].ht = ht;
			tr->changes[tr->nr].tt = tt;
			tr->changes[tr->nr].name = GDKstrdup(name);
			tr->changes[tr->nr].b = NULL;
			tr->nr++;
		}
	}
	if (buf)
		GDKfree(buf);
	return LOG_OK;
}

static void
la_bat_create(logger *lg, logaction *la)
{
	int ht = (la->ht < 0) ? TYPE_void : la->ht;
	int tt = (la->tt < 0) ? TYPE_void : la->tt;
	BAT *b = BATnew(ht, tt, BATSIZE, PERSISTENT);

	if (b != NULL) {
		if (la->ht < 0)
			BATseqbase(b, 0);
		if (la->tt < 0)
			BATseqbase(BATmirror(b), 0);

		BATsetaccess(b, BAT_READ);
		logger_add_bat(lg, b, la->name);
		logbat_destroy(b);
	}
}

static void
log_read_use(logger *lg, trans *tr, logformat *l, char *name)
{
	(void) lg;
	if (tr_grow(tr)) {
		tr->changes[tr->nr].type = LOG_USE;
		tr->changes[tr->nr].nr = l->nr;
		tr->changes[tr->nr].name = GDKstrdup(name);
		tr->changes[tr->nr].b = NULL;
		tr->nr++;
	}
}

static void
la_bat_use(logger *lg, logaction *la)
{
	log_bid bid = (log_bid) la->nr;
	BAT *b = BATdescriptor(bid);
	BUN p;

	assert(la->nr <= (lng) INT_MAX);
	if (!b) {
		GDKerror("logger: could not use bat (%d) for %s\n", (int) bid, la->name);
		return;
	}
	logger_add_bat(lg, b, la->name);
	if ((p = BUNfnd(lg->snapshots_bid, &b->batCacheid)) != BUN_NONE) {
		BUNdelete(lg->snapshots_bid, p, FALSE);
		BUNdelete(lg->snapshots_tid, p, FALSE);
	}
#ifndef NDEBUG
	assert(b->batRole == PERSISTENT);
	assert(0 <= b->H->heap.farmid && b->H->heap.farmid < MAXFARMS);
	assert(BBPfarms[b->H->heap.farmid].roles & (1 << PERSISTENT));
	if (b->H->vheap) {
		assert(0 <= b->H->vheap->farmid && b->H->vheap->farmid < MAXFARMS);
		assert(BBPfarms[b->H->vheap->farmid].roles & (1 << PERSISTENT));
	}
	assert(0 <= b->T->heap.farmid && b->T->heap.farmid < MAXFARMS);
	assert(BBPfarms[b->T->heap.farmid].roles & (1 << PERSISTENT));
	if (b->T->vheap) {
		assert(0 <= b->T->vheap->farmid && b->T->vheap->farmid < MAXFARMS);
		assert(BBPfarms[b->T->vheap->farmid].roles & (1 << PERSISTENT));
	}
#endif
	BUNappend(lg->snapshots_bid, &b->batCacheid, FALSE);
	BUNappend(lg->snapshots_tid, &lg->tid, FALSE);
	logbat_destroy(b);
}


#define TR_SIZE		1024

static trans *
tr_create(trans *tr, int tid)
{
	trans *ntr = GDKmalloc(sizeof(trans));

	if (ntr == NULL)
		return NULL;
	ntr->tid = tid;
	ntr->sz = TR_SIZE;
	ntr->nr = 0;
	ntr->changes = GDKmalloc(sizeof(logaction) * TR_SIZE);
	if (ntr->changes == NULL) {
		GDKfree(ntr);
		return NULL;
	}
	ntr->tr = tr;
	return ntr;
}

static trans *
tr_find(trans *tr, int tid)
/* finds the tid and reorders the chain list, puts trans with tid first */
{
	trans *t = tr, *p = NULL;

	while (t && t->tid != tid) {
		p = t;
		t = t->tr;
	}
	if (!t)
		return NULL;	/* BAD missing transaction */
	if (t == tr)
		return tr;
	if (t->tr)		/* get this tid out of the list */
		p->tr = t->tr;
	t->tr = tr;		/* and move it to the front */
	return t;
}

static void
la_apply(logger *lg, logaction *c)
{
	switch (c->type) {
	case LOG_INSERT:
	case LOG_DELETE:
	case LOG_UPDATE:
		la_bat_updates(lg, c);
		break;
	case LOG_CREATE:
		la_bat_create(lg, c);
		break;
	case LOG_USE:
		la_bat_use(lg, c);
		break;
	case LOG_DESTROY:
		la_bat_destroy(lg, c);
		break;
	case LOG_CLEAR:
		la_bat_clear(lg, c);
		break;
	}
}

static void
la_destroy(logaction *c)
{
	if (c->name)
		GDKfree(c->name);
	if (c->b)
		logbat_destroy(c->b);
}

static int
tr_grow(trans *tr)
{
	if (tr->nr == tr->sz) {
		tr->sz <<= 1;
		tr->changes = (logaction *) GDKrealloc(tr->changes, tr->sz * sizeof(logaction));
		if (tr->changes == NULL)
			return 0;
	}
	/* cleanup the next */
	tr->changes[tr->nr].name = NULL;
	tr->changes[tr->nr].b = NULL;
	return 1;
}

static trans *
tr_destroy(trans *tr)
{
	trans *r = tr->tr;

	GDKfree(tr->changes);
	GDKfree(tr);
	return r;
}

static trans *
tr_commit(logger *lg, trans *tr)
{
	int i;

	if (lg->debug & 1)
		fprintf(stderr, "#tr_commit\n");

	for (i = 0; i < tr->nr; i++) {
		la_apply(lg, &tr->changes[i]);
		la_destroy(&tr->changes[i]);
	}
	return tr_destroy(tr);
}

static trans *
tr_abort(logger *lg, trans *tr)
{
	int i;

	if (lg->debug & 1)
		fprintf(stderr, "#tr_abort\n");

	for (i = 0; i < tr->nr; i++)
		la_destroy(&tr->changes[i]);
	return tr_destroy(tr);
}

/* Update the last transaction id written in the catalog file.
 * Mostly used by the shared logger. */
static int
logger_update_catalog_file(logger *lg, const char *dir, const char *filename, int role)
{
	FILE *fp;
	int bak_exists;
	int farmid = BBPselectfarm(role, 0, offheap);

	bak_exists = 0;
	/* check if an older file exists and move bak it up */
#if defined(_MSC_VER)
	if (_access(filename, 0) != -1) {
#else
	if (access(filename, 0) != -1) {
#endif
		bak_exists = 1;
		if (GDKmove(farmid, dir, filename, NULL, dir, filename, "bak") < 0) {
			fprintf(stderr, "!ERROR: logger_update_catalog_file: rename %s to %s.bak in %s failed\n", filename, filename, dir);
			return LOG_ERR;
		}
	}

	if ((fp = GDKfileopen(farmid, dir, filename, NULL, "w")) != NULL) {
		if (fprintf(fp, "%06d\n\n", lg->version) < 0) {
			fprintf(stderr, "!ERROR: logger_update_catalog_file: write to %s failed\n", filename);
			return LOG_ERR;
		}

		if (fprintf(fp, LLFMT "\n", lg->id) < 0 || fclose(fp) < 0) {
			fprintf(stderr, "!ERROR: logger_update_catalog_file: write/flush to %s failed\n", filename);
			return LOG_ERR;
		}

		/* cleanup the bak file, if it exists*/
		if (bak_exists) {
			GDKunlink(farmid, dir, filename, "bak");
		}
	} else {
		fprintf(stderr, "!ERROR: logger_update_catalog_file: could not create %s\n", filename);
		GDKerror("logger_update_catalog_file: could not open %s\n", filename);
		return LOG_ERR;
	}
	return LOG_OK;
}

static int
logger_open(logger *lg)
{
	char id[BUFSIZ];
	char *filename;

	snprintf(id, BUFSIZ, LLFMT, lg->id);
	filename = GDKfilepath(BBPselectfarm(lg->dbfarm_role, 0, offheap), lg->dir, LOGFILE, id);

	lg->log = open_wstream(filename);
	lg->end = 0;
	if (lg->log == NULL || mnstr_errnr(lg->log)) {
		fprintf(stderr, "!ERROR: logger_open: creating %s failed\n", filename);
		return LOG_ERR;
	}

	return LOG_OK;
}

static void
logger_close(logger *lg)
{
	stream *log = lg->log;

	if (log) {
		mnstr_close(log);
		mnstr_destroy(log);
	}
	lg->log = NULL;
}

static int
logger_readlog(logger *lg, char *filename)
{
	trans *tr = NULL;
	logformat l;
	int err = 0;
	time_t t0, t1;
	struct stat sb;
	lng fpos;
	char* path = GDKfilepath_long(BBPselectfarm(lg->dbfarm_role, 0, offheap), filename, NULL);

	if (lg->debug & 1) {
		fprintf(stderr, "#logger_readlog opening %s\n", filename);
	}

	lg->log = open_rstream(path);

	/* if the file doesn't exist, there is nothing to be read back */
	if (!lg->log || mnstr_errnr(lg->log)) {
		if (lg->log)
			mnstr_destroy(lg->log);
		lg->log = NULL;
		return LOG_ERR;
	}
	if (fstat(fileno(getFile(lg->log)), &sb) < 0) {
		fprintf(stderr, "!ERROR: logger_readlog: fstat on opened file %s failed\n", filename);
		mnstr_destroy(lg->log);
		lg->log = NULL;
		/* If we can't read the files, it might simply be empty.
		 * In that case we can't return LOG_ERR, since it's actually fine */
		return 1;
	}
	t0 = time(NULL);
	while (!err && log_read_format(lg, &l)) {
		char *name = NULL;

		t1 = time(NULL);
		if (t1 - t0 > 10) {
			t0 = t1;
			/* not more than once every 10 seconds */
			if (mnstr_fgetpos(lg->log, &fpos) == 0) {
				printf("# still reading write-ahead log \"%s\" (%d%% done)\n", filename, (int) ((fpos * 100 + 50) / sb.st_size));
				fflush(stdout);
			}
		}
		if (l.flag != LOG_START && l.flag != LOG_END && l.flag != LOG_SEQ) {
			name = log_read_string(lg);

			if (!name) {
				err = -1;
				break;
			}
		}
		if (lg->debug & 1) {
			fprintf(stderr, "#logger_readlog: ");
			if (l.flag > 0 &&
			    l.flag < (char) (sizeof(log_commands) / sizeof(log_commands[0])))
				fprintf(stderr, "%s", log_commands[(int) l.flag]);
			else
				fprintf(stderr, "%d", l.flag);
			fprintf(stderr, " %d " LLFMT, l.tid, l.nr);
			if (name)
				fprintf(stderr, " %s", name);
			fprintf(stderr, "\n");
		}
		/* find proper transaction record */
		if (l.flag != LOG_START)
			tr = tr_find(tr, l.tid);
		switch (l.flag) {
		case LOG_START:
			assert(l.nr <= (lng) INT_MAX);
			if (l.nr > lg->tid)
				lg->tid = (int)l.nr;
			tr = tr_create(tr, (int)l.nr);
			if (lg->debug & 1)
				fprintf(stderr, "#logger tstart %d\n", tr->tid);
			break;
		case LOG_END:
			if (tr == NULL)
				err = 1;
			else if (l.tid != l.nr)	/* abort record */
				tr = tr_abort(lg, tr);
			else
				tr = tr_commit(lg, tr);
			break;
		case LOG_SEQ:
			err = (log_read_seq(lg, &l) != LOG_OK);
			break;
		case LOG_INSERT:
		case LOG_DELETE:
		case LOG_UPDATE:
			if (name == NULL || tr == NULL)
				err = 1;
			else
				err = (log_read_updates(lg, tr, &l, name) != LOG_OK);
			break;
		case LOG_CREATE:
			if (name == NULL || tr == NULL)
				err = 1;
			else
				err = (log_read_create(lg, tr, name) != LOG_OK);
			break;
		case LOG_USE:
			if (name == NULL || tr == NULL)
				err = 1;
			else
				log_read_use(lg, tr, &l, name);
			break;
		case LOG_DESTROY:
			if (name == NULL || tr == NULL)
				err = 1;
			else
				log_read_destroy(lg, tr, name);
			break;
		case LOG_CLEAR:
			if (name == NULL || tr == NULL)
				err = 1;
			else
				log_read_clear(lg, tr, name);
			break;
		default:
			err = -2;
		}
		if (name)
			GDKfree(name);
		lg->changes++;
	}
	logger_close(lg);

	/* remaining transactions are not committed, ie abort */
	while (tr)
		tr = tr_abort(lg, tr);
	return LOG_OK;
}

/*
 * The log files are incrementally numbered, starting from 2. They are processed in the
 * same sequence.
 */
static int
logger_readlogs(logger *lg, FILE *fp, char *filename)
{
	int res = LOG_OK;
	char id[BUFSIZ];

	if (lg->debug & 1) {
		fprintf(stderr, "#logger_readlogs logger id is " LLFMT "\n", lg->id);
	}

	while (fgets(id, BUFSIZ, fp) != NULL) {
		char log_filename[BUFSIZ];
		lng lid = strtoll(id, NULL, 10);

		if (lg->debug & 1) {
			fprintf(stderr, "#logger_readlogs last logger id written in %s is " LLFMT "\n", filename, lid);
		}

		while(lid > lg->id && res != LOG_ERR) {
			snprintf(log_filename, BUFSIZ, "%s." LLFMT, filename, lg->id);
			if ((logger_readlog(lg, log_filename)) == LOG_ERR && lg->shared && lg->id > 1) {
				/* we cannot distinguish errors from
				 * incomplete transactions (even if we
				 * would log aborts in the logs). So
				 * we simply abort and move to the
				 * next log file.
				 * The only special case is if the files is missing altogether
				 * and the logger is a shared one,
				 * then we have missing transactions and we should abort.
				 * Yeah, and we also ignore the 1st files it most likely never exists. */
				res = LOG_ERR;
				fprintf(stderr, "#logger_readlogs missing shared logger file %s. Aborting\n", log_filename);
			}
			/* Increment the id only at the end, since we want to re-read the last file.
			 * That is because last time we read it, it was empty, since the logger create empty files
			 * and fills them in later. */
			lg->id++;
		}
		/* if this is a shared logger, write the id in the shared file */
		if (lg->shared) {
			logger_update_catalog_file(lg, lg->local_dir, LOGFILE_SHARED, lg->local_dbfarm_role);
		}
	}
	return res;
}

static int
logger_commit(logger *lg)
{
	int id = LOG_SID;
	BUN p;

	if (lg->debug & 1)
		fprintf(stderr, "#logger_commit\n");

	p = BUNfnd(lg->seqs_id, &id);
	BUNdelete(lg->seqs_id, p, FALSE);
	BUNdelete(lg->seqs_val, p, FALSE);
	BUNappend(lg->seqs_id, &id, FALSE);
	BUNappend(lg->seqs_val, &lg->id, FALSE);

	/* cleanup old snapshots */
	if (BATcount(lg->snapshots_bid)) {
		BATclear(lg->snapshots_bid, FALSE);
		BATclear(lg->snapshots_tid, FALSE);
		BATcommit(lg->snapshots_bid);
		BATcommit(lg->snapshots_tid);
	}
	return bm_commit(lg);
}

static int
check_version(logger *lg, FILE *fp)
{
	int version = 0;

	if (fscanf(fp, "%6d", &version) != 1) {
		GDKerror("Could not read the version number from the file '%s/log'.\n",
			 lg->dir);

		return -1;
	}
	if (version != lg->version) {
		if (lg->prefuncp == NULL ||
		    (*lg->prefuncp)(version, lg->version) != 0) {
			GDKerror("Incompatible database version %06d, "
				 "this server supports version %06d\n"
				 "Please move away %s.",
				 version, lg->version, lg->dir);

			return -1;
		}
	} else
		lg->postfuncp = NULL;	 /* don't call */
	if (fgetc(fp) != '\n' ||	 /* skip \n */
	    fgetc(fp) != '\n')		 /* skip \n */
		return -1;
	return 0;
}

static int
bm_subcommit(BAT *list_bid, BAT *list_nme, BAT *catalog_bid, BAT *catalog_nme, BAT *extra, int debug)
{
	BUN p, q;
	BUN nn = 3 + (list_bid->batFirst > list_bid->batDeleted ? list_bid->batFirst - list_bid->batDeleted : 0) + BATcount(list_bid) + (extra ? BATcount(extra) : 0);
	bat *n = GDKmalloc(sizeof(bat) * nn);
	int i = 0;
	BATiter iter = (list_nme)?bat_iterator(list_nme):bat_iterator(list_bid);
	int res;

	n[i++] = 0;		/* n[0] is not used */

	/* first loop over deleted then over current and new */
	for (p = list_bid->batDeleted; p < list_bid->batFirst; p++) {
		bat col = *(log_bid *) Tloc(list_bid, p);

		if (debug & 1)
			fprintf(stderr, "#commit deleted %s (%d) %s\n",
				BBPname(col), col,
				(list_bid == catalog_bid) ? BUNtail(iter, p) : "snapshot");
		n[i++] = abs(col);
	}
	BATloop(list_bid, p, q) {
		bat col = *(log_bid *) Tloc(list_bid, p);

		if (debug & 1)
			fprintf(stderr, "#commit new %s (%d) %s\n",
				BBPname(col), col,
				(list_bid == catalog_bid) ? BUNtail(iter, p) : "snapshot");
		n[i++] = abs(col);
	}
	if (extra) {
		iter = bat_iterator(extra);
		BATloop(extra, p, q) {
			str name = (str) BUNtail(iter, p);

			if (debug & 1)
				fprintf(stderr, "#commit extra %s %s\n",
					name,
					(list_bid == catalog_bid) ? BUNtail(iter, p) : "snapshot");
			n[i++] = abs(BBPindex(name));
		}
	}
	/* now commit catalog, so it's also up to date on disk */
	n[i++] = abs(catalog_bid->batCacheid);
	n[i++] = abs(catalog_nme->batCacheid);
	assert((BUN) i <= nn);
	BATcommit(catalog_bid);
	BATcommit(catalog_nme);
	res = TMsubcommit_list(n, i);
	GDKfree(n);
	if (res < 0)
		fprintf(stderr, "!ERROR: bm_subcommit: commit failed\n");
	return res;
}

static void
logger_fatal(const char *format, const char *arg1, const char *arg2, const char *arg3)
{
	char *buf;

	GDKfatal(format, arg1, arg2, arg3);
	GDKlog(format, arg1, arg2, arg3);
	if ((buf = GDKerrbuf) != NULL) {
		fprintf(stderr, "%s", buf);
		fflush(stderr);
	}
	GDKexit(1);
}

static int
logger_create_catalog_file(int debug, logger *lg, const char *fn, FILE *fp, const char *filename, char *bak) {
	log_bid bid = 0;
	/* catalog does not exist, so the log file also
	 * shouldn't exist */
	if (fp != NULL) {
		logger_fatal(
				"logger_create_catalog_file: there is no logger catalog, but there is a log file.\n"
						"Are you sure you are using the correct combination of database\n"
						"(--dbpath) and log directory (--set %s_logdir)?\n", fn, 0, 0);
		return LOG_ERR;
	}

	lg->catalog_bid = logbat_new(TYPE_int, BATSIZE, PERSISTENT);
	lg->catalog_nme = logbat_new(TYPE_str, BATSIZE, PERSISTENT);
	if (lg->catalog_bid == NULL || lg->catalog_nme == NULL)
		logger_fatal("logger_create_catalog_file: cannot create catalog bats", 0, 0, 0);
	if (debug & 1)
		fprintf(stderr, "#create %s catalog\n", fn);

	/* Make persistent */
	bid = lg->catalog_bid->batCacheid;
	BBPincref(bid, TRUE);
	snprintf(bak, BUFSIZ, "%s_catalog_bid", fn);
	if (BBPrename(lg->catalog_bid->batCacheid, bak) < 0)
		logger_fatal("logger_create_catalog_file: BBPrename to %s failed", bak, 0, 0);


	/* Make persistent */
	bid = lg->catalog_nme->batCacheid;
	BBPincref(bid, TRUE);
	snprintf(bak, BUFSIZ, "%s_catalog_nme", fn);
	if (BBPrename(lg->catalog_nme->batCacheid, bak) < 0)
		logger_fatal("logger_create_catalog_file: BBPrename to %s failed", bak, 0, 0);


	if (!GDKcreatedir(filename)) {
		logger_fatal("logger_create_catalog_file: cannot create directory for log file %s\n", filename, 0, 0);
		return LOG_ERR;
	}
	if ((fp = GDKfileopen(BBPselectfarm(lg->dbfarm_role, 0, offheap), filename, NULL, NULL, "w")) == NULL) {
		logger_fatal("logger_create_catalog_file: cannot create log file %s\n", filename, 0, 0);
		return LOG_ERR;
	}
	lg->id++;
	if (fprintf(fp, "%06d\n\n" LLFMT "\n", lg->version, lg->id) < 0) {
		fclose(fp);
		unlink(filename);
		logger_fatal("logger_create_catalog_file: writing log file %s failed",
				filename, 0, 0);
	}
	if (fclose(fp) < 0) {
		unlink(filename);
		logger_fatal("logger_create_catalog_file: closing log file %s failed",
				filename, 0, 0);
	}
	fp = NULL;
	if (bm_subcommit(lg->catalog_bid, lg->catalog_nme, lg->catalog_bid,
			lg->catalog_nme, NULL, lg->debug) != 0) {
		/* cannot commit catalog, so remove log */
		unlink(filename);
		return LOG_ERR;
	}
	return LOG_OK;
}

/* find the persistent catalog. As non persistent bats
 * require a logical reference we also add a logical
 * reference for the persistent bats */
static int
logger_find_persistent_catalog(logger *lg, const char *fn, FILE *fp, char *bak, bat *catalog_bid, bat *catalog_nme) {
	BUN p, q;
	BAT *b = BATdescriptor(*catalog_bid), *n;
	if (b == 0)
		logger_fatal("logger_find_persistent_catalog: inconsistent database, catalog does not exist", 0, 0, 0);

	snprintf(bak, BUFSIZ, "%s_catalog_nme", fn);
	*catalog_nme = BBPindex(bak);
	n = BATdescriptor(*catalog_nme);
	if (n == 0)
		logger_fatal("logger_find_persistent_catalog: inconsistent database, catalog_nme does not exist", 0, 0, 0);

	/* the catalog exists, and so should the log file */
	if (fp == NULL) {
		logger_fatal(
				"logger_find_persistent_catalog: there is a logger catalog, but no log file.\n"
						"Are you sure you are using the correct combination of database\n"
						"(--dbpath) and log directory (--set %s_logdir)?\n"
						"If you have done a recent update of the server, it may be that your\n"
						"logs are in an old location.  You should then either use\n"
						"--set %s_logdir=<path to old log directory> or move the old log\n"
						"directory to the new location (%s).\n", fn, fn, lg->dir);
		return LOG_ERR;
	}
	lg->catalog_bid = b;
	lg->catalog_nme = n;
	BATloop(b, p, q) {
		bat bid = *(log_bid *) Tloc(b, p);
		BBPincref(bid, TRUE);
	}
	return LOG_OK;
}

/* Set the logdir path, add a dbfarm if needed.
 * Returns the role of the dbfarm containing the logdir.
 */
static int
logger_set_logdir_path(char *filename, const char *fn, const char *logdir, int shared) {
	int role = PERSISTENT; /* default role is persistent, i.e. the default dbfarm */

	if (MT_path_absolute(logdir)) {
		char logdir_parent_path[BUFSIZ] = "";
		char logdir_name[BUFSIZ] = "";
		/* split the logdir string into absolute parent dir path and (relative) log dir name */
		if (GDKextractParentAndLastDirFromPath(logdir, logdir_parent_path, logdir_name)) {
			/* set the new relative logdir locaiton including the logger function name subdir */
			snprintf(filename, BUFSIZ, "%s%c%s%c", logdir_name, DIR_SEP, fn, DIR_SEP);

			/* add a new dbfarm for the logger directory using the parent dir path,
			 * assuming it is set, s.t. the logs are stored in a location other than the default dbfarm,
			 * or at least it appears so to (multi)dbfarm aware functions */
			if (!shared) {
				role = LOG_DIR;
			} else {
				role = SHARED_LOG_DIR;
			}
			BBPaddfarm(logdir_parent_path, 1 << role);
		} else {
			logger_fatal("logger_set_logdir_path: logdir path is not correct (%s)."
					"Make sure you specify a valid absolute or relative path.\n", logdir, 0, 0);
		}
	} else {
		/* just concat the logdir and fn with appropriate separators */
		snprintf(filename, BUFSIZ, "%s%c%s%c", logdir, DIR_SEP, fn, DIR_SEP);
	}

	return role;
}

/* Load data from the logger logdir
 * Initialize new directories and catalog files if none are present, unless running in read-only mode
 * Load data and persist it in the BATs
 * Convert 32bit data to 64bit, unless running in read-only mode */
static int
logger_load(int debug, const char* fn, char filename[BUFSIZ], logger* lg)
{
	int id = LOG_SID;
	FILE *fp;
	log_bid seqs_id = 0;
	char bak[BUFSIZ];
	bat catalog_bid, catalog_nme, bid;
	int farmid = BBPselectfarm(lg->dbfarm_role, 0, offheap);

	snprintf(filename, BUFSIZ, "%s%s", lg->dir, LOGFILE);
	snprintf(bak, BUFSIZ, "%s.bak", filename);

	/* try to open logfile backup, or failing that, the file
	 * itself. we need to know whether this file exists when
	 * checking the database consistency later on */
	if ((fp = GDKfileopen(farmid, bak, NULL, NULL, "r")) != NULL) {
		fclose(fp);
		(void) GDKunlink(farmid, lg->dir, LOGFILE, NULL);
		if (GDKmove(farmid, lg->dir, LOGFILE, "bak", lg->dir, LOGFILE, NULL) != 0)
			logger_fatal("logger_load: cannot move log.bak file back.\n", 0, 0, 0);
	}
	fp = GDKfileopen(farmid, filename, NULL, NULL, "r");

	snprintf(bak, BUFSIZ, "%s_catalog", fn);
	bid = BBPindex(bak);

	snprintf(bak, BUFSIZ, "%s_catalog_bid", fn);
	catalog_bid = BBPindex(bak);

	if (bid != 0 && catalog_bid == 0)
		logger_fatal("Logger_new: ancient database, please upgrade "
			     "first to Jan2014 (11.17.X) release", 0, 0, 0);

	/* this is intentional - even if catalog_bid is 0, but the logger is shared,
	 * force it to find the persistent catalog */
	if (catalog_bid == 0 &&	!lg->shared) {
		if (logger_create_catalog_file(debug, lg, fn, fp, filename, bak) == LOG_ERR) {
			goto error;
		}
	} else {
		if (logger_find_persistent_catalog(lg, fn, fp, bak, &catalog_bid, &catalog_nme) == LOG_ERR) {
			goto error;
		}
	}

	seqs_id = logger_find_bat(lg, "seqs_id");
	if (seqs_id == 0) {
		lg->seqs_id = logbat_new(TYPE_int, 1, PERSISTENT);
		snprintf(bak, BUFSIZ, "%s_seqs_id", fn);
		if (BBPrename(lg->seqs_id->batCacheid, bak) < 0)
			logger_fatal("logger_load: BBPrename to %s failed",
				     bak, 0, 0);
		logger_add_bat(lg, lg->seqs_id, "seqs_id");

		lg->seqs_val = logbat_new(TYPE_lng, 1, PERSISTENT);
		snprintf(bak, BUFSIZ, "%s_seqs_val", fn);
		if (BBPrename(lg->seqs_val->batCacheid, bak) < 0)
			logger_fatal("logger_load: BBPrename to %s failed",
				     bak, 0, 0);
		logger_add_bat(lg, lg->seqs_val, "seqs_val");

		if (BUNappend(lg->seqs_id, &id, FALSE) == NULL ||
		    BUNappend(lg->seqs_val, &lg->id, FALSE) == NULL)
			logger_fatal("logger_load: failed to append value to "
				     "sequences bat", 0, 0, 0);

		lg->snapshots_bid = logbat_new(TYPE_int, 1, PERSISTENT);
		snprintf(bak, BUFSIZ, "%s_snapshots_bid", fn);
		if (BBPrename(lg->snapshots_bid->batCacheid, bak) < 0)
			logger_fatal("logger_load: BBPrename to %s failed",
				     bak, 0, 0);
		logger_add_bat(lg, lg->snapshots_bid, "snapshots_bid");

		lg->snapshots_tid = logbat_new(TYPE_int, 1, PERSISTENT);
		snprintf(bak, BUFSIZ, "%s_snapshots_tid", fn);
		if (BBPrename(lg->snapshots_tid->batCacheid, bak) < 0)
			logger_fatal("logger_load: BBPrename to %s failed",
				     bak, 0, 0);
		logger_add_bat(lg, lg->snapshots_tid, "snapshots_tid");

		if (bm_subcommit(lg->catalog_bid, lg->catalog_nme, lg->catalog_bid, lg->catalog_nme, NULL, lg->debug) < 0)
			logger_fatal("logger_load: commit failed", 0, 0, 0);
	} else {
		bat seqs_val = logger_find_bat(lg, "seqs_val");
		bat snapshots_bid = logger_find_bat(lg, "snapshots_bid");
		bat snapshots_tid = logger_find_bat(lg, "snapshots_tid");

		lg->seqs_id = BATdescriptor(seqs_id);
		if (lg->seqs_id == 0)
			logger_fatal("logger_load: inconsistent database, seqs_id does not exist", 0, 0, 0);
		lg->seqs_val = BATdescriptor(seqs_val);
		if (lg->seqs_val == 0)
			logger_fatal("logger_load: inconsistent database, seqs_val does not exist", 0, 0, 0);
		if (BATcount(lg->seqs_id) && !lg->shared) {
			BUN p = BUNfnd(lg->seqs_id, &id);
			lg->id = *(lng *) Tloc(lg->seqs_val, p);
			if (lg->debug & 1) {
				fprintf(stderr, "#logger_load setting new logger id to " LLFMT "\n", lg->id);
			}
		} else {
			if (BUNappend(lg->seqs_id, &id, FALSE) == NULL ||
			    BUNappend(lg->seqs_val, &lg->id, FALSE) == NULL)
				logger_fatal("logger_load: failed to append "
					     "value to sequences bat", 0, 0, 0);
		}
		lg->snapshots_bid = BATdescriptor(snapshots_bid);
		if (lg->snapshots_bid == 0)
			logger_fatal("logger_load: inconsistent database, snapshots_bid does not exist", 0, 0, 0);
		lg->snapshots_tid = BATdescriptor(snapshots_tid);
		if (lg->snapshots_tid == 0)
			logger_fatal("logger_load: inconsistent database, snapshots_tid does not exist", 0, 0, 0);
	}
	lg->freed = BATnew(TYPE_void, TYPE_int, 1, TRANSIENT);
	if (lg->freed == NULL)
		logger_fatal("logger_load: failed to create freed bat", 0, 0, 0);
	BATseqbase(lg->freed, 0);
	snprintf(bak, BUFSIZ, "%s_freed", fn);
	/* do not rename it if this is a shared logger */
	if (!lg->shared && BBPrename(lg->freed->batCacheid, bak) < 0)
		logger_fatal("logger_load: BBPrename to %s failed", bak, 0, 0);

	if (fp != NULL) {
#if SIZEOF_OID == 8
		char cvfile[BUFSIZ];
#endif
		if (check_version(lg, fp)) {
			goto error;
		}

#if SIZEOF_OID == 8
		/* When a file *_32-64-convert exists in the database,
		 * it was left there by the BBP initialization code
		 * when it did a conversion of 32-bit OIDs to 64 bits
		 * (see the comment above fixoidheapcolumn and
		 * fixoidheap in gdk_bbp).  It the file exists, we
		 * first create a file called convert-32-64 in the log
		 * directory and we write the current log ID into that
		 * file.  After this file is created, we delete the
		 * *_32-64-convert file in the database.  We then know
		 * that while reading the logs, we have to read OID
		 * values as 32 bits (this is indicated by setting the
		 * read32bitoid flag).  When we're done reading the
		 * logs, we remove the file (and reset the flag).  If
		 * we get interrupted before we have written this
		 * file, the file in the database will still exist, so
		 * the next time we're started, BBPinit will not
		 * convert OIDs (that was done before we got
		 * interrupted), but we will still know to convert the
		 * OIDs ourselves.  If we get interrupted after we
		 * have deleted the file from the database, we check
		 * whether the file convert-32-64 exists and if it
		 * contains the expected ID.  If it does, we again
		 * know that we have to convert.  If the ID is not
		 * what we expect, the conversion was apparently done
		 * already, and so we can delete the file. */

		/* Do not do conversion logger is shared/read-only */
		if (!lg->shared) {
			snprintf(cvfile, sizeof(cvfile), "%sconvert-32-64", lg->dir);
			snprintf(bak, sizeof(bak), "%s_32-64-convert", fn);
			{
				FILE *fp1;
				long off;
				int curid;

				/* read the current log id without disturbing
				 * the file pointer */
				off = ftell(fp);
				if (fscanf(fp, "%d", &curid) != 1)
					curid = -1; /* shouldn't happen? */
				fseek(fp, off, SEEK_SET);


			if ((fp1 = GDKfileopen(farmid, bak, NULL, NULL, "r")) != NULL) {
				/* file indicating that we need to do
				 * a 32->64 bit OID conversion exists;
				 * record the fact in case we get
				 * interrupted, and set the flag so
				 * that we actually do what's asked */
				fclose(fp1);
				/* first create a versioned file using
				 * the current log id */
				if ((fp1 = GDKfileopen(farmid, cvfile, NULL, NULL, "w")) == NULL ||
				    fprintf(fp1, "%d\n", curid) < 2 ||
				    fflush(fp1) != 0 || /* make sure it's save on disk */
#if defined(_MSC_VER)
				    _commit(_fileno(fp1)) < 0 ||
#elif defined(HAVE_FDATASYNC)
				    fdatasync(fileno(fp1)) < 0 ||
#elif defined(HAVE_FSYNC)
				    fsync(fileno(fp1)) < 0 ||
#endif
				    fclose(fp1) != 0)
					logger_fatal("logger_load: failed to write %s\n", cvfile, 0, 0);
				/* then remove the unversioned file
				 * that gdk_bbp created (in this
				 * order!) */
				unlink(bak);
				/* set the flag that we need to convert */
				lg->read32bitoid = 1;
			} else if ((fp1 = GDKfileopen(farmid, cvfile, NULL, NULL, "r")) != NULL) {
				/* the versioned conversion file
				 * exists: check version */
				int newid;

					if (fscanf(fp1, "%d", &newid) == 1 &&
						newid == curid) {
						/* versions match, we need to
						 * convert */
						lg->read32bitoid = 1;
					}
					fclose(fp1);
					if (!lg->read32bitoid) {
						/* no conversion, so we can
						 * remove the versioned
						 * file */
						unlink(cvfile);
					}
				}
			}
		}
#endif
		lg->changes++;
		if (logger_readlogs(lg, fp, filename) == LOG_ERR) {
			goto error;
		}
		fclose(fp);
		fp = NULL;
#if SIZEOF_OID == 8
		if (lg->read32bitoid && !lg->shared) {
			/* we converted, remove versioned file and
			 * reset conversion flag */
			unlink(cvfile);
			lg->read32bitoid = 0;
		}
#endif
		if (lg->postfuncp)
			(*lg->postfuncp)(lg);
	}

	return LOG_OK;
	error:
	if (fp)
		fclose(fp);
	if (lg)
		GDKfree(lg);
	return LOG_ERR;
}

/* Initialize a new logger
 * It will load any data in the logdir and persist it in the BATs*/
static logger *
logger_new(int debug, const char *fn, const char *logdir, int version, preversionfix_fptr prefuncp, postversionfix_fptr postfuncp, int shared, const char *local_logdir)
{
	logger *lg = (struct logger *) GDKmalloc(sizeof(struct logger));
	char filename[BUFSIZ];
	char shared_log_filename[BUFSIZ];

	if (lg == NULL) {
		fprintf(stderr, "!ERROR: logger_new: allocating logger structure failed\n");
		return NULL;
	}

	lg->debug = debug;
	lg->shared = shared;

	lg->changes = 0;
	lg->version = version;
	lg->id = 1;

	lg->tid = 0;
#if SIZEOF_OID == 8
	lg->read32bitoid = 0;
#endif

	lg->dbfarm_role = logger_set_logdir_path(filename, fn, logdir, shared);
	if ((lg->fn = GDKstrdup(fn)) == NULL ||
	    (lg->dir = GDKstrdup(filename)) == NULL) {
		fprintf(stderr, "!ERROR: logger_new: strdup failed\n");
		GDKfree(lg->fn);
		GDKfree(lg->dir);
		GDKfree(lg);
		return NULL;
	}
	if (lg->debug & 1) {
		fprintf(stderr, "#logger_new dir set to %s\n", lg->dir);
	}

	if (shared) {
		/* set the local logdir as well
		 * here we pass 0 for the shared flag, since we want these file(s) to be stored in the default logdir */
		lg->local_dbfarm_role = logger_set_logdir_path(filename, fn, local_logdir, 0);
		if ((lg->local_dir = GDKstrdup(filename)) == NULL) {
			fprintf(stderr, "!ERROR: logger_new: strdup failed\n");
			GDKfree(lg->fn);
			GDKfree(lg->dir);
			GDKfree(lg->local_dir);
			GDKfree(lg);
			return NULL;
		}
		if (lg->debug & 1) {
			fprintf(stderr, "#logger_new local_dir set to %s\n", lg->local_dir);
		}

		/* get last shared logger id from the local log dir,
		 * but first check if the file exists */
		snprintf(shared_log_filename, BUFSIZ, "%s%s", lg->local_dir, LOGFILE_SHARED);
#if defined(_MSC_VER)
		if (_access(shared_log_filename, 0) != -1) {
#else
		if (access(shared_log_filename, 0) != -1) {
#endif
			lng res = logger_read_last_transaction_id(lg, lg->local_dir, LOGFILE_SHARED, lg->local_dbfarm_role);
			if (res == LOG_ERR) {
				fprintf(stderr, "!ERROR: logger_new: failed to read previous shared logger id form %s\n", LOGFILE_SHARED);
				GDKfree(lg->fn);
				GDKfree(lg->dir);
				GDKfree(lg->local_dir);
				GDKfree(lg);
				return NULL;
			}

			lg->id = res;
			if (lg->debug & 1) {
				fprintf(stderr, "#logger_new last shared transactions is read form %s is " LLFMT "\n", shared_log_filename, lg->id);
			}
		} else {
			if (lg->debug & 1) {
				fprintf(stderr, "#logger_new no previous %s found\n", LOGFILE_SHARED);
			}
		}
	}

	lg->prefuncp = prefuncp;
	lg->postfuncp = postfuncp;
	lg->log = NULL;
	lg->end = 0;
	lg->catalog_bid = NULL;
	lg->catalog_nme = NULL;
	lg->snapshots_bid = NULL;
	lg->snapshots_tid = NULL;
	lg->seqs_id = NULL;
	lg->seqs_val = NULL;

	if (logger_load(debug, fn, filename, lg) == LOG_OK) {
		return lg;
	}
	return NULL;
}

/* Reload (shared) logger
 * It will load any data in the logdir and persist it in the BATs */
int
logger_reload(logger *lg)
{
	char filename[BUFSIZ];

	snprintf(filename, BUFSIZ, "%s", lg->dir);
	if (lg->debug & 1) {
		fprintf(stderr, "#logger_reload %s\n", filename);
	}

	return logger_load(lg->debug, lg->fn, filename, lg);
}

/* Create a new logger */
logger *
logger_create(int debug, const char *fn, const char *logdir, int version, preversionfix_fptr prefuncp, postversionfix_fptr postfuncp)
{
	logger *lg = logger_new(debug, fn, logdir, version, prefuncp, postfuncp, 0, NULL);

	if (!lg)
		return NULL;
	if (logger_open(lg) == LOG_ERR) {
		logger_destroy(lg);

		return NULL;
	}
	if (lg->changes &&
	    (logger_restart(lg) != LOG_OK ||
	     logger_cleanup(lg, 0) != LOG_OK)) {
		logger_destroy(lg);

		return NULL;
	}
	return lg;
}

/* Create a new shared logger, that is for slaves reading the master log directory.
 * Assumed to be read-only */
logger *
logger_create_shared(int debug, const char *fn, const char *logdir, const char *local_logdir, int version, preversionfix_fptr prefuncp, postversionfix_fptr postfuncp)
{
	logger *lg = NULL;

	lg = logger_new(debug, fn, logdir, version, prefuncp, postfuncp, 1, local_logdir);

	return lg;
}

void
logger_destroy(logger *lg)
{
	if (lg->catalog_bid) {
		BUN p, q;
		BAT *b = lg->catalog_bid;

		logger_cleanup(lg, 0);

		/* destroy the deleted */
/* would be an error ....
		for (p = b->batDeleted; p < b->batFirst; p++) {
			bat bid = *(log_bid *) BUNhead(b, p);

			BBPdecref(bid, TRUE);
		}
*/
		/* free resources */
		BATloop(b, p, q) {
			bat bid = *(log_bid *) Tloc(b, p);

			BBPdecref(bid, TRUE);
		}

		BBPdecref(lg->catalog_bid->batCacheid, TRUE);
		BBPdecref(lg->catalog_nme->batCacheid, TRUE);
		logbat_destroy(lg->catalog_bid);
		logbat_destroy(lg->catalog_nme);
		logbat_destroy(lg->freed);
	}
	GDKfree(lg->fn);
	GDKfree(lg->dir);
	logger_close(lg);
	GDKfree(lg);
}

int
logger_exit(logger *lg)
{
	FILE *fp;
	char filename[BUFSIZ];
	int farmid = BBPselectfarm(lg->dbfarm_role, 0, offheap);

	logger_close(lg);
	if (GDKmove(farmid, lg->dir, LOGFILE, NULL, lg->dir, LOGFILE, "bak") < 0) {
		fprintf(stderr, "!ERROR: logger_exit: rename %s to %s.bak in %s failed\n",
			LOGFILE, LOGFILE, lg->dir);
		return LOG_ERR;
	}

	snprintf(filename, BUFSIZ, "%s%s", lg->dir, LOGFILE);
	if ((fp = GDKfileopen(farmid, filename, NULL, NULL, "w")) != NULL) {
		char ext[BUFSIZ];

		if (fprintf(fp, "%06d\n\n", lg->version) < 0) {
			fprintf(stderr, "!ERROR: logger_exit: write to %s failed\n",
				filename);
			return LOG_ERR;
		}
		lg->id ++;

		if (logger_commit(lg) != LOG_OK) {
			fprintf(stderr, "!ERROR: logger_exit: logger_commit failed\n");
			return LOG_ERR;
		}

		if (fprintf(fp, LLFMT "\n", lg->id) < 0 ||
		    fclose(fp) < 0) {
			fprintf(stderr, "!ERROR: logger_exit: write/flush to %s failed\n",
				filename);
			return LOG_ERR;
		}

		/* atomic action, switch to new log, keep old for
		 * later cleanup actions */
		snprintf(ext, BUFSIZ, "bak-" LLFMT, lg->id);

		if (GDKmove(farmid, lg->dir, LOGFILE, "bak", lg->dir, LOGFILE, ext) < 0) {
			fprintf(stderr, "!ERROR: logger_exit: rename %s.bak to %s.%s failed\n",
				LOGFILE, LOGFILE, ext);
			return LOG_ERR;
		}

		lg->changes = 0;
	} else {
		fprintf(stderr, "!ERROR: logger_exit: could not create %s\n",
			filename);
		GDKerror("logger_exit: could not open %s\n", filename);
		return LOG_ERR;
	}
	return LOG_OK;
}

int
logger_restart(logger *lg)
{
	int res = 0;

	if ((res = logger_exit(lg)) == LOG_OK)
		res = logger_open(lg);

	return res;
}

/* Clean-up write-ahead log files already persisted in the BATs.
 * Update the LOGFILE and delete all bak- files as well.
 */
static int
logger_cleanup_old(logger *lg, int keep_persisted_log_files)
{
	char buf[BUFSIZ];
	lng id;
	int farmid = BBPselectfarm(lg->dbfarm_role, 0, offheap);
	int cleanupResult = 0;

	// Calculate offset based on the number of files to keep
	id = lg->id - keep_persisted_log_files;

	// Stop cleaning up once bak- files are no longer found
	while (cleanupResult == LOG_OK) {
		// clean up the WAL file
		if (lg->debug & 1) {
			snprintf(buf, BUFSIZ, "%s%s." LLFMT, lg->dir, LOGFILE, id);
			fprintf(stderr, "#logger_cleanup_old %s\n", buf);
		}
		snprintf(buf, BUFSIZ, LLFMT, id);
		cleanupResult = GDKunlink(farmid, lg->dir, LOGFILE, buf);

		// clean up the bak- WAL files
		if (lg->debug & 1) {
			snprintf(buf, BUFSIZ, "%s%s.bak-" LLFMT, lg->dir, LOGFILE, id);
			fprintf(stderr, "#logger_cleanup_old %s\n", buf);
		}
		snprintf(buf, BUFSIZ, "bak-" LLFMT, id);
		cleanupResult = GDKunlink(farmid, lg->dir, LOGFILE, buf);

		id = id - 1;
	}
	return LOG_OK;
}

int
logger_cleanup(logger *lg, int keep_persisted_log_files)
{
	char buf[BUFSIZ];
	char id[BUFSIZ];
	FILE *fp = NULL;
	int farmid = BBPselectfarm(lg->dbfarm_role, 0, offheap);

	snprintf(buf, BUFSIZ, "%s%s.bak-" LLFMT, lg->dir, LOGFILE, lg->id);

	if (lg->debug & 1) {
		fprintf(stderr, "#logger_cleanup %s\n", buf);
	}

	if (keep_persisted_log_files == 0) {
		// If keep_persisted_log_files is 0, remove the last persisted WAL files as well
		// => less work for logger_cleanup_old()
		if ((fp = GDKfileopen(farmid, buf, NULL, NULL, "r")) == NULL) {
			fprintf(stderr, "!ERROR: logger_cleanup: cannot open file %s\n", buf);
			return LOG_ERR;
		}

		/* skip catalog */
		while (fgets(id, BUFSIZ, fp) != NULL && id[0] != '\n')
			;

		while (fgets(id, BUFSIZ, fp) != NULL) {
			char *e = strchr(id, '\n');

			if (e)
				*e = 0;
			GDKunlink(farmid, lg->dir, LOGFILE, id);
		}
		fclose(fp);
	}

	snprintf(buf, BUFSIZ, "bak-" LLFMT, lg->id);

	GDKunlink(farmid, lg->dir, LOGFILE, buf);

	if (keep_persisted_log_files > 0) {
		// Clean up the old WAL files as well, if any
		// We will ignore the output of logger_cleanup_old
		logger_cleanup_old(lg, keep_persisted_log_files);
	}

	return LOG_OK;
}

/* Clean-up write-ahead log files already persisted in the BATs, leaving only the most recent one.
 * Keeps only the number of files set in lg->keep_persisted_log_files.
 * Only the bak- files are deleted for the preserved WAL files.
 */
lng
logger_changes(logger *lg)
{
	return lg->changes;
}

/* Read the last recorded transactions id from a logfile */
lng
logger_read_last_transaction_id(logger *lg, char *dir, char *logger_file, int role)
{
	char filename[BUFSIZ];
	FILE *fp;
	char id[BUFSIZ];
	lng lid = LOG_ERR;
	int farmid = BBPselectfarm(role, 0, offheap);

	snprintf(filename, BUFSIZ, "%s%s", dir, logger_file);
	if ((fp = GDKfileopen(farmid, filename, NULL, NULL, "r")) == NULL) {
		fprintf(stderr, "!ERROR: logger_read_last_transaction_id: unable to open file %s\n", filename);
		goto error;
	}

	if (check_version(lg, fp)) {
		fprintf(stderr, "!ERROR: logger_read_last_transaction_id: inconsistent log version for file %s\n", filename);
		goto error;
	}

	/* read the last id */
	while (fgets(id, BUFSIZ, fp) != NULL) {
		lid = strtoll(id, NULL, 10);
		if (lg->debug & 1) {
			fprintf(stderr, "#logger_read_last_transaction_id last logger id written in %s is " LLFMT "\n", filename, lid);
		}
	}

	return lid;

	error:
	if (fp)
		fclose(fp);
	if (lg)
		GDKfree(lg);
	return LOG_ERR;
}

int
logger_sequence(logger *lg, int seq, lng *id)
{
	BUN p = BUNfnd(lg->seqs_id, &seq);

	if (p != BUN_NONE) {
		*id = *(lng *) Tloc(lg->seqs_val, p);

		return 1;
	}
	return 0;
}

/*
 * Changes made to the BAT descriptor should be stored in the log
 * files.  Actually, we need to save the descriptor file, perhaps we
 * should simply introduce a versioning scheme.
 */
int
log_bat_persists(logger *lg, BAT *b, const char *name)
{
	char *ha, *ta;
	int len;
	char buf[BUFSIZ];
	logformat l;
	int havevoid = 0;
	int flag = (b->batPersistence == PERSISTENT) ? LOG_USE : LOG_CREATE;
	BUN p;

	l.nr = 0;
	if (flag == LOG_USE) {
#ifndef NDEBUG
		assert(b->batRole == PERSISTENT);
		assert(0 <= b->H->heap.farmid && b->H->heap.farmid < MAXFARMS);
		assert(BBPfarms[b->H->heap.farmid].roles & (1 << PERSISTENT));
		if (b->H->vheap) {
			assert(0 <= b->H->vheap->farmid && b->H->vheap->farmid < MAXFARMS);
			assert(BBPfarms[b->H->vheap->farmid].roles & (1 << PERSISTENT));
		}
		assert(0 <= b->T->heap.farmid && b->T->heap.farmid < MAXFARMS);
		assert(BBPfarms[b->T->heap.farmid].roles & (1 << PERSISTENT));
		if (b->T->vheap) {
			assert(0 <= b->T->vheap->farmid && b->T->vheap->farmid < MAXFARMS);
			assert(BBPfarms[b->T->vheap->farmid].roles & (1 << PERSISTENT));
		}
#endif
		l.nr = b->batCacheid;
	}
	l.flag = flag;
	l.tid = lg->tid;
	lg->changes++;
	if (log_write_format(lg, &l) == LOG_ERR ||
	    log_write_string(lg, name) == LOG_ERR)
		return LOG_ERR;

	if (lg->debug & 1)
		fprintf(stderr, "#persists bat %s (%d) %s\n",
			name, b->batCacheid,
			(flag == LOG_USE) ? "use" : "create");

	if (flag == LOG_USE) {
		assert(b->batRole == PERSISTENT);
		assert(b->H->heap.farmid == 0);
		assert(b->H->vheap == NULL ||
		       BBPfarms[b->H->vheap->farmid].roles & (1 << PERSISTENT));
		assert(b->T->heap.farmid == 0);
		assert(b->T->vheap == NULL ||
		       BBPfarms[b->T->vheap->farmid].roles & (1 << PERSISTENT));
		if ((p = BUNfnd(lg->snapshots_bid, &b->batCacheid)) != BUN_NONE){
			BUNdelete(lg->snapshots_bid, p, FALSE);
			BUNdelete(lg->snapshots_tid, p, FALSE);
		}
		BUNappend(lg->snapshots_bid, &b->batCacheid, FALSE);
		BUNappend(lg->snapshots_tid, &lg->tid, FALSE);
		return LOG_OK;
	}

	ha = ATOMname(b->htype);
	if (b->htype == TYPE_void && BAThdense(b)) {
		ha = "vid";
		havevoid = 1;
	}
	ta = ATOMname(b->ttype);
	if (!havevoid && b->ttype == TYPE_void && BATtdense(b)) {
		ta = "vid";
	}
	len = snprintf(buf, BUFSIZ, "%s,%s", ha, ta);
	len++;			/* include EOS */
	if (!mnstr_writeInt(lg->log, len) ||
	    mnstr_write(lg->log, buf, 1, len) != (ssize_t) len) {
		fprintf(stderr, "!ERROR: log_bat_persists: write failed\n");
		return LOG_ERR;
	}

	if (lg->debug & 1)
		fprintf(stderr, "#Logged new bat [%s,%s] %s " BUNFMT " (%d)\n",
			ha, ta, name, BATcount(b), b->batCacheid);
	return log_bat(lg, b, name);
}

int
log_bat_transient(logger *lg, const char *name)
{
	log_bid bid = logger_find_bat(lg, name);
	logformat l;
	BUN p;

	l.flag = LOG_DESTROY;
	l.tid = lg->tid;
	l.nr = 0;
	lg->changes++;

	/* if this is a snapshot bat, we need to skip all changes */
	if ((p = BUNfnd(lg->snapshots_bid, &bid)) != BUN_NONE) {
#ifndef NDEBUG
		assert(BBP_desc(bid)->S.role == PERSISTENT);
		assert(0 <= BBP_desc(bid)->H.heap.farmid && BBP_desc(bid)->H.heap.farmid < MAXFARMS);
		assert(BBPfarms[BBP_desc(bid)->H.heap.farmid].roles & (1 << PERSISTENT));
		if (BBP_desc(bid)->H.vheap) {
			assert(0 <= BBP_desc(bid)->H.vheap->farmid && BBP_desc(bid)->H.vheap->farmid < MAXFARMS);
			assert(BBPfarms[BBP_desc(bid)->H.vheap->farmid].roles & (1 << PERSISTENT));
		}
		assert(0 <= BBP_desc(bid)->T.heap.farmid && BBP_desc(bid)->T.heap.farmid < MAXFARMS);
		assert(BBPfarms[BBP_desc(bid)->T.heap.farmid].roles & (1 << PERSISTENT));
		if (BBP_desc(bid)->T.vheap) {
			assert(0 <= BBP_desc(bid)->T.vheap->farmid && BBP_desc(bid)->T.vheap->farmid < MAXFARMS);
			assert(BBPfarms[BBP_desc(bid)->T.vheap->farmid].roles & (1 << PERSISTENT));
		}
#endif
		BUNdelete(lg->snapshots_bid, p, FALSE);
		BUNdelete(lg->snapshots_tid, p, FALSE);
		BUNappend(lg->snapshots_bid, &bid, FALSE);
		BUNappend(lg->snapshots_tid, &lg->tid, FALSE);
	}

	if (log_write_format(lg, &l) == LOG_ERR ||
	    log_write_string(lg, name) == LOG_ERR) {
		fprintf(stderr, "!ERROR: log_bat_transient: write failed\n");
		return LOG_ERR;
	}

	if (lg->debug & 1)
		fprintf(stderr, "#Logged destroyed bat %s\n", name);
	return LOG_OK;
}

int
log_delta(logger *lg, BAT *b, const char *name)
{
	int ok = GDK_SUCCEED;
	logformat l;
	BUN p;

	if (lg->debug & 128) {
		/* logging is switched off */
		return LOG_OK;
	}

	l.tid = lg->tid;
	l.nr = (BUNlast(b) - BUNfirst(b));
	lg->changes += l.nr;

	if (l.nr) {
		BATiter bi = bat_iterator(b);
		int (*wh) (const void *, stream *, size_t) = b->htype == TYPE_void ? BATatoms[TYPE_oid].atomWrite : BATatoms[b->htype].atomWrite;
		int (*wt) (const void *, stream *, size_t) = BATatoms[b->ttype].atomWrite;

		l.flag = LOG_UPDATE;
		if (log_write_format(lg, &l) == LOG_ERR ||
		    log_write_string(lg, name) == LOG_ERR)
			return LOG_ERR;

		for (p = BUNfirst(b); p < BUNlast(b) && ok == GDK_SUCCEED; p++) {
			const void *h = BUNhead(bi, p);
			const void *t = BUNtail(bi, p);

			ok = wh(h, lg->log, 1);
			ok = (ok == GDK_FAIL) ? ok : wt(t, lg->log, 1);
		}

		if (lg->debug & 1)
			fprintf(stderr, "#Logged %s " LLFMT " inserts\n", name, l.nr);
	}
	if (ok == GDK_FAIL)
		fprintf(stderr, "!ERROR: log_delta: write failed\n");
	return (ok == GDK_SUCCEED) ? LOG_OK : LOG_ERR;
}

int
log_bat(logger *lg, BAT *b, const char *name)
{
	int ok = GDK_SUCCEED;
	logformat l;
	BUN p;

	if (lg->debug & 128) {
		/* logging is switched off */
		return LOG_OK;
	}

	l.tid = lg->tid;
	l.nr = (BUNlast(b) - b->batInserted);
	lg->changes += l.nr;

	if (l.nr) {
		BATiter bi = bat_iterator(b);
		int (*wh) (const void *, stream *, size_t) = BATatoms[b->htype].atomWrite;
		int (*wt) (const void *, stream *, size_t) = BATatoms[b->ttype].atomWrite;

		l.flag = LOG_INSERT;
		if (log_write_format(lg, &l) == LOG_ERR ||
		    log_write_string(lg, name) == LOG_ERR)
			return LOG_ERR;

		if (b->htype == TYPE_void &&
		    b->ttype > TYPE_void &&
		    b->ttype < TYPE_str &&
		    !isVIEW(b)) {
			const void *t = BUNtail(bi, b->batInserted);

			ok = wt(t, lg->log, (size_t)l.nr);
		} else {
			for (p = b->batInserted; p < BUNlast(b) && ok == GDK_SUCCEED; p++) {
				const void *h = BUNhead(bi, p);
				const void *t = BUNtail(bi, p);

				ok = wh(h, lg->log, 1);
				ok = (ok == GDK_FAIL) ? ok : wt(t, lg->log, 1);
			}
		}

		if (lg->debug & 1)
			fprintf(stderr, "#Logged %s " LLFMT " inserts\n", name, l.nr);
	}
	l.nr = (b->batFirst - b->batDeleted);
	lg->changes += l.nr;

	if (l.nr && ok == GDK_SUCCEED) {
		BATiter bi = bat_iterator(b);
		int (*wh) (const void *, stream *, size_t) = BATatoms[b->htype].atomWrite;
		int (*wt) (const void *, stream *, size_t) = BATatoms[b->ttype].atomWrite;

		l.flag = LOG_DELETE;
		if (log_write_format(lg, &l) == LOG_ERR ||
		    log_write_string(lg, name) == LOG_ERR)
			return LOG_ERR;

		for (p = b->batDeleted; p < b->batFirst && ok == GDK_SUCCEED; p++) {
			const void *h = BUNhead(bi, p);
			const void *t = BUNtail(bi, p);

			ok = wh(h, lg->log, 1);
			ok = (ok == GDK_FAIL) ? ok : wt(t, lg->log, 1);
		}

		if (lg->debug & 1)
			fprintf(stderr, "#Logged %s " LLFMT " deletes\n", name, l.nr);
	}
	if (ok == GDK_FAIL)
		fprintf(stderr, "!ERROR: log_bat: write failed\n");
	return (ok == GDK_SUCCEED) ? LOG_OK : LOG_ERR;
}

int
log_bat_clear(logger *lg, const char *name)
{
	logformat l;

	if (lg->debug & 128) {
		/* logging is switched off */
		return LOG_OK;
	}

	l.nr = 1;
	l.tid = lg->tid;
	lg->changes += l.nr;

	l.flag = LOG_CLEAR;
	if (log_write_format(lg, &l) == LOG_ERR ||
	    log_write_string(lg, name) == LOG_ERR)
		return LOG_ERR;

	if (lg->debug & 1)
		fprintf(stderr, "#Logged clear %s\n", name);

	return LOG_OK;
}

int
log_tstart(logger *lg)
{
	logformat l;

	l.flag = LOG_START;
	l.tid = ++lg->tid;
	l.nr = lg->tid;

	if (lg->debug & 1)
		fprintf(stderr, "#log_tstart %d\n", lg->tid);

	return log_write_format(lg, &l);
}

#define DBLKSZ 8192
#define DBLKMASK 8191
#define SEGSZ 64*DBLKSZ
static char zeros[DBLKSZ] = { 0 };

static int
pre_allocate(logger *lg)
{
	lng p;

	if (mnstr_fgetpos(lg->log, &p) != 0)
		return -1;
	if (p + DBLKSZ > lg->end) {
		lng s = p;

		if (p > lg->end) {
			lg->end = (p & ~DBLKMASK);
			if (p > DBLKSZ)
				p -= DBLKSZ;
		}
		if (p < lg->end) {
			p = (lg->end - p);
			if (mnstr_write(lg->log, zeros, (size_t) p, 1) < 0)
				return -1;
			lg->end += p;
			p = 0;
		}
		for (; p < SEGSZ; p += DBLKSZ, lg->end += DBLKSZ) {
			if (mnstr_write(lg->log, zeros, DBLKSZ, 1) < 0)
				return -1;
		}
		if (mnstr_fsetpos(lg->log, s) < 0)
			return -1;
	}
	return 0;
}

int
log_tend(logger *lg)
{
	logformat l;
	int res = 0;

	if (lg->debug & 1)
		fprintf(stderr, "#log_tend %d\n", lg->tid);

	if (DELTAdirty(lg->snapshots_bid)) {
		/* sub commit all new snapshots */
		BAT *tids, *bids;

#if 0
		/* We can't use this version because we still use
		 * BUNdelete on lg->snapshots_tid, so it is not
		 * necessarily dense-headed */
		tids = BATsubselect(lg->snapshots_tid, NULL, &lg->tid, &lg->tid,
				    TRUE, TRUE, FALSE);
		if (tids == NULL) {
			fprintf(stderr, "!ERROR: log_tend: subselect failed\n");
			return LOG_ERR;
		}
		bids = BATproject(tids, lg->snapshots_bid);
#else
		tids = BATuselect(lg->snapshots_tid, &lg->tid, &lg->tid);
		if (tids == NULL) {
			fprintf(stderr, "!ERROR: log_tend: select failed\n");
			return LOG_ERR;
		}
		bids = BATsemijoin(lg->snapshots_bid, tids);
#endif
		BBPunfix(tids->batCacheid);
		if (bids == NULL) {
			fprintf(stderr, "!ERROR: log_tend: semijoin failed\n");
			return LOG_ERR;
		}
		res = bm_subcommit(bids, NULL, lg->snapshots_bid,
				   lg->snapshots_tid, NULL, lg->debug);
		BBPunfix(bids->batCacheid);
	}
	l.flag = LOG_END;
	l.tid = lg->tid;
	l.nr = lg->tid;
	if (res ||
	    log_write_format(lg, &l) == LOG_ERR ||
	    mnstr_flush(lg->log) ||
	    mnstr_fsync(lg->log) ||
	    pre_allocate(lg) < 0) {
		fprintf(stderr, "!ERROR: log_tend: write failed\n");
		return LOG_ERR;
	}
	return LOG_OK;
}

int
log_abort(logger *lg)
{
	logformat l;

	if (lg->debug & 1)
		fprintf(stderr, "#log_abort %d\n", lg->tid);

	l.flag = LOG_END;
	l.tid = lg->tid;
	l.nr = -1;

	if (log_write_format(lg, &l) == LOG_ERR)
		return LOG_ERR;

	return LOG_OK;
}

/* a transaction in it self */
int
log_sequence(logger *lg, int seq, lng val)
{
	logformat l;
	BUN p;

	l.flag = LOG_SEQ;
	l.tid = lg->tid;
	l.nr = seq;

	if (lg->debug & 1)
		fprintf(stderr, "#log_sequence (%d," LLFMT ")\n", seq, val);

	if ((p = BUNfnd(lg->seqs_id, &seq)) != BUN_NONE) {
		BUNdelete(lg->seqs_id, p, FALSE);
		BUNdelete(lg->seqs_val, p, FALSE);
	}
	BUNappend(lg->seqs_id, &seq, FALSE);
	BUNappend(lg->seqs_val, &val, FALSE);

	if (log_write_format(lg, &l) == LOG_ERR ||
	    !mnstr_writeLng(lg->log, val) ||
	    mnstr_flush(lg->log) ||
	    mnstr_fsync(lg->log) ||
	    pre_allocate(lg) < 0) {
		fprintf(stderr, "!ERROR: log_sequence: write failed\n");
		return LOG_ERR;
	}

	return LOG_OK;
}

static int
bm_commit(logger *lg)
{
	BUN p, q;
	BAT *b = lg->catalog_bid;
	BAT *n = logbat_new(TYPE_str, BATcount(lg->freed), TRANSIENT);
	int res;

	/* remove the destroyed bats */
	for (p = b->batDeleted; p < b->batFirst; p++) {
		bat bid = *(log_bid *) Tloc(b, p);
		BAT *lb = BATdescriptor(bid);

		BATmode(lb, TRANSIENT);
		BBPdecref(bid, TRUE);
		logbat_destroy(lb);

		if (lg->debug & 1)
			fprintf(stderr, "#bm_commit: delete %d (%d)\n",
				bid, BBP_lrefs(bid));
	}

	/* subcommit the freed snapshots */
	BATseqbase(n, 0);
	if (BATcount(lg->freed)) {

		BATloop(lg->freed, p, q) {
			bat bid = *(log_bid *) Tloc(lg->freed, p);
			BAT *lb = BATdescriptor(bid);
			str name = BBPname(bid);

			BATmode(lb, TRANSIENT);
			logbat_destroy(lb);
			if (lg->debug & 1)
				fprintf(stderr,
					"#commit deleted (snapshot) %s (%d)\n",
					name, bid);
			BUNappend(n, name, FALSE);
			BBPdecref(bid, TRUE);
		}
	}

	for (p = b->batInserted; p < BUNlast(b); p++) {
		log_bid bid = *(log_bid *) Tloc(b, p);
		BAT *lb = BATdescriptor(bid);

		assert(lb);
		BATmode(lb, PERSISTENT);
		assert(lb->batRestricted > BAT_WRITE);
		logbat_destroy(lb);

		if (lg->debug & 1)
			fprintf(stderr, "#bm_commit: create %d (%d)\n",
				bid, BBP_lrefs(bid));
	}
	res = bm_subcommit(lg->catalog_bid, lg->catalog_nme, lg->catalog_bid, lg->catalog_nme, n, lg->debug);
	BBPreclaim(n);
	BATclear(lg->freed, FALSE);
	BATcommit(lg->freed);
	return res != 0 ? LOG_ERR : LOG_OK;
}

log_bid
logger_add_bat(logger *lg, BAT *b, const char *name)
{
	log_bid bid = logger_find_bat(lg, name);

	assert(b->batRestricted > 0 ||
	       b == lg->snapshots_bid ||
	       b == lg->snapshots_tid ||
	       b == lg->catalog_bid ||
	       b == lg->catalog_nme ||
	       b == lg->seqs_id ||
	       b == lg->seqs_val);
	assert(b->batRole == PERSISTENT);
	if (bid) {
		if (bid != b->batCacheid) {
			logger_del_bat(lg, bid);
		} else {
			return bid;
		}
	}
	bid = b->batCacheid;
	if (lg->debug & 1)
		fprintf(stderr, "#create %s\n", name);
	lg->changes += BATcount(b) + 1;
	BUNappend(lg->catalog_bid, &bid, FALSE);
	BUNappend(lg->catalog_nme, name, FALSE);
	BBPincref(bid, TRUE);
	return bid;
}

void
logger_del_bat(logger *lg, log_bid bid)
{
	BAT *b = BATdescriptor(bid);
	BUN p = BUNfnd(lg->catalog_bid, &bid), q;

	assert(p != BUN_NONE);

	/* if this is a not logger commited snapshot bat, make it
	 * transient */
	if (p >= lg->catalog_bid->batInserted &&
	    (q = BUNfnd(lg->snapshots_bid, &bid)) != BUN_NONE) {

		BUNdelete(lg->snapshots_bid, q, FALSE);
		BUNdelete(lg->snapshots_tid, q, FALSE);
		if (lg->debug & 1)
			fprintf(stderr,
				"#logger_del_bat release snapshot %d (%d)\n",
				bid, BBP_lrefs(bid));
		BUNappend(lg->freed, &bid, FALSE);
	} else if (p >= lg->catalog_bid->batInserted)
		BBPdecref(bid, TRUE);
	if (b) {
		lg->changes += BATcount(b) + 1;
		BBPunfix(b->batCacheid);
	}
	BUNdelete(lg->catalog_bid, p, FALSE);
	BUNdelete(lg->catalog_nme, p, FALSE);
/*assert(BBP_lrefs(bid) == 0);*/
}

log_bid
logger_find_bat(logger *lg, const char *name)
{
	log_bid res = 0;
	BUN p = BUNfnd(lg->catalog_nme, name);

	if (p != BUN_NONE)
		res = *(log_bid *) Tloc(lg->catalog_bid, p);
	return res;
}
