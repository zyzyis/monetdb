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
 * Copyright August 2008-2012 MonetDB B.V.
 * All Rights Reserved.
 */

#include "monetdb_config.h"
#include "gdk.h"
#include "gdk_private.h"

/* how much to extend the extent and histo bats when we run out of space */
#define GROUPBATINCR	8192

/* BATgroup returns three bats that indicate the grouping of the input
 * bat.  All input and output bats (must) have dense head columns.
 * Grouping means that all equal values are in the same group, and
 * differing values are in different groups.  If specified, the input
 * bat g gives a pre-existing grouping.  This bat must be aligned with
 * b.
 *
 * The outputs are as follows.
 * The groups bat has a dense head which is aligned with the input bat
 * b, and the tail has group id's (type oid).
 * The extents and histo bats have the group id in the head (a dense
 * sequence starting at 0).  The tail of extents is the head oid from
 * b of a representative of the group.  The tail of histo is of type
 * wrd and contains the number of elements from b that are member of
 * the group.
 *
 * The extents and histo bats are optionally created.  The groups bat
 * is always created.  In other words, the groups argument may not be
 * NULL, but the extents and histo arguments may be NULL.
 *
 * There are five different implementations of the grouping code.
 *
 * If it can be trivially determined that all groups are singletons,
 * we can produce the outputs trivially.
 *
 * If the input bats b and g are sorted, or if the subsorted flag is
 * set (only used by BATsubsort), we only need to compare consecutive
 * values.
 *
 * If the input bat b is sorted, but g is not, we can compare
 * consecutive values in b and need to scan sections of g for equal
 * groups.
 *
 * If a hash table already exists on b, we can make use of it.
 *
 * Otherwise we build a partial hash table on the fly.
 */
gdk_return
BATgroup_internal(BAT **groups, BAT **extents, BAT **histo,
		  BAT *b, BAT *g, BAT *e, BAT *h, int subsorted)
{
	BAT *gn = NULL, *en = NULL, *hn = NULL;
	int (*cmp)(const void *, const void *);
	const oid *grps = NULL;
	oid *ngrps, ngrp, prev = 0;
	oid *exts = NULL;
	wrd *cnts = NULL;
	BUN p, q, r;
	const void *v, *pv;
	BATiter bi;
	char *ext = NULL;
	Hash *hs = NULL;
	BUN hb;
	BUN maxgrps;

	if (b == NULL || !BAThdense(b)) {
		GDKerror("BATgroup: b must be dense-headed\n");
		return GDK_FAIL;
	}
	/* g is NULL or [oid(dense),oid] and same size as b */
	assert(g == NULL || BAThdense(g));
	assert(g == NULL || BATttype(g) == TYPE_oid);
	assert(g == NULL || BATcount(b) == BATcount(g));
	assert(g == NULL || BATcount(b) == 0 || b->hseqbase == g->hseqbase);
	/* e is NULL or [oid(dense),oid] */
	assert(e == NULL || BAThdense(e));
	assert(e == NULL || BATttype(e) == TYPE_oid);
	/* h is NULL or [oid(dense),wrd] */
	assert(h == NULL || BAThdense(h));
	assert(h == NULL || h->ttype == TYPE_wrd);
	/* e and h are aligned */
	assert(e == NULL || h == NULL || BATcount(e) == BATcount(h));
	assert(e == NULL || h == NULL || e->hseqbase == h->hseqbase);
	/* we want our output to go somewhere */
	assert(groups != NULL);

	if (b->tkey || BATcount(b) <= 1 || (g && (g->tkey || BATtdense(g)))) {
		/* grouping is trivial: 1 element per group */
		if (BATcount(b) == 1 && b->htype == TYPE_oid)
			ngrp = * (oid *) Hloc(b, BUNfirst(b));
		else
			ngrp = BATcount(b) == 0 ? 0 : b->hseqbase;
		gn = BATnew(TYPE_void, TYPE_void, BATcount(b));
		if (gn == NULL)
			goto error;
		BATsetcount(gn, BATcount(b));
		BATseqbase(gn, ngrp);
		BATseqbase(BATmirror(gn), 0);
		*groups = gn;
		if (extents) {
			en = BATnew(TYPE_void, TYPE_void, BATcount(b));
			if (en == NULL)
				goto error;
			BATsetcount(en, BATcount(b));
			BATseqbase(en, 0);
			BATseqbase(BATmirror(en), ngrp);
			*extents = en;
		}
		if (histo) {
			wrd one = 1;

			hn = BATconstant(TYPE_wrd, &one, BATcount(b));
			if (hn == NULL)
				goto error;
			*histo = hn;
		}
		return GDK_SUCCEED;
	}
	assert(g == NULL || !BATtdense(g)); /* i.e. g->ttype == TYPE_oid */
	bi = bat_iterator(b);
	cmp = BATatoms[b->ttype].atomCmp;
	gn = BATnew(TYPE_void, TYPE_oid, BATcount(b));
	if (gn == NULL)
		goto error;
	ngrps = (oid *) Tloc(gn, BUNfirst(gn));
	maxgrps = BATcount(b) / 10;
	if (e && maxgrps < BATcount(e))
		maxgrps += BATcount(e);
	if (h && maxgrps < BATcount(h))
		maxgrps += BATcount(h);
	if (maxgrps < GROUPBATINCR)
		maxgrps = BATcount(b);
	if (extents) {
		en = BATnew(TYPE_void, TYPE_oid, maxgrps);
		if (en == NULL)
			goto error;
		exts = (oid *) Tloc(en, BUNfirst(en));
	}
	if (histo) {
		hn = BATnew(TYPE_void, TYPE_wrd, maxgrps);
		if (hn == NULL)
			goto error;
		cnts = (wrd *) Tloc(hn, BUNfirst(hn));
	}
	ngrp = 0;
	BATsetcount(gn, BATcount(b));
	BATseqbase(gn, b->hseqbase);
	if (g)
		grps = (const oid *) Tloc(g, BUNfirst(g));
	if (((b->tsorted || b->trevsorted) &&
	     (g == NULL || g->tsorted || g->trevsorted)) ||
	    subsorted) {
		/* we only need to compare each entry with the previous */
		if (grps)
			prev = *grps++;
		pv = BUNtail(bi, BUNfirst(b));
		*ngrps++ = ngrp;
		if (extents)
			*exts++ = b->hseqbase;
		if (histo)
			*cnts = 1;
		for (r = BUNfirst(b), p = r + 1, q = r + BATcount(b);
		     p < q;
		     p++) {
			v = BUNtail(bi, p);
			if ((grps && *grps++ != prev) || cmp(pv, v) != 0) {
				ngrp++;
				if (grps)
					prev = *grps;
				if (ngrp == maxgrps) {
					/* we need to extend extents
					 * and histo bats */
					maxgrps += GROUPBATINCR;
					if (maxgrps > BATcount(b))
						maxgrps = BATcount(b);
					if (extents) {
						BATsetcount(en, ngrp);
						en = BATextend(en, maxgrps);
						exts = (oid *) Tloc(en, BUNfirst(en) + ngrp);
					}
					if (histo) {
						BATsetcount(hn, ngrp);
						hn = BATextend(hn, maxgrps);
						cnts = (wrd *) Tloc(hn, BUNfirst(hn) + ngrp - 1);
					}
				}
				if (extents)
					*exts++ = b->hseqbase + (oid) (p - r);
				if (histo)
					*++cnts = 1;
			} else if (histo) {
				*cnts += 1;
			}
			*ngrps++ = ngrp;
			pv = v;
		}
		/* ngrp is the id of the last group, turn it into the count */
		ngrp++;
		gn->tsorted = 1;
		*groups = gn;
	} else if (b->tsorted || b->trevsorted) {
		BUN i, j;

		/* for each value, we need to scan all previous equal
		 * values (a consecutive, possibly empty, range) to
		 * see if we can find one in the same old group */
		pv = BUNtail(bi, BUNfirst(b));
		ngrps[0] = ngrp;
		if (extents)
			exts[0] = b->hseqbase;
		if (histo)
			cnts[0] = 1;
		ngrp++;		/* the next group to be assigned */
		gn->tsorted = 1; /* be optimistic */
		for (j = r = BUNfirst(b), p = r + 1, q = r + BATcount(b);
		     p < q;
		     p++) {
			v = BUNtail(bi, p);
			if (cmp(v, pv) == 0) {
				/* range [j, p) is all same value,
				 * find one in the same group */
				for (i = j; i < p; i++) {
					if (grps[i - r] == grps[p - r]) {
						oid grp = ngrps[i - r];
						ngrps[p - r] = grp;
						if (histo)
							cnts[grp]++;
						if (grp != ngrp - 1)
							gn->tsorted = 0;
						break;
					}
				}
				if (i < p) {
					/* we found the value/group
					 * combination, go to next
					 * value */
					continue;
				}
			} else {
				/* value differs from previous value */
				j = p;
				pv = v;
			}
			/* start a new group */
			if (ngrp == maxgrps) {
				/* we need to extend extents and histo
				 * bats */
				maxgrps += GROUPBATINCR;
				if (maxgrps > BATcount(b))
					maxgrps = BATcount(b);
				if (extents) {
					BATsetcount(en, ngrp);
					en = BATextend(en, maxgrps);
					exts = (oid *) Tloc(en, BUNfirst(en));
				}
				if (histo) {
					BATsetcount(hn, ngrp);
					hn = BATextend(hn, maxgrps);
					cnts = (wrd *) Tloc(hn, BUNfirst(hn));
				}
			}
			if (extents)
				exts[ngrp] = b->hseqbase + (oid) (p - r);
			if (histo)
				cnts[ngrp] = 1;
			ngrps[p - r] = ngrp;
			ngrp++;
		}
	} else if (b->T->hash) {
		/* we already have a hash table on b */
		hs = b->T->hash;
		for (r = BUNfirst(b), p = r, q = r + BATcount(b); p < q; p++) {
			v = BUNtail(bi, p);
			/* this loop is similar, but not equal, to
			 * HASHloop: the difference is that we only
			 * consider BUNs smaller than the one we're
			 * looking up (p), and that we also consider
			 * the input groups */
			for (hb = hs->hash[HASHprobe(hs, v)];
			     hb != BUN_NONE;
			     hb = hs->link[hb]) {
				if (hb < p &&
				    (grps == NULL ||
				     grps[hb - r] == grps[p - r]) &&
				    cmp(v, BUNtail(bi, hb)) == 0) {
					ngrps[p - r] = ngrps[hb - r];
					if (histo)
						cnts[ngrps[hb - r]]++;
					break;
				}
			}
			if (hb == BUN_NONE) {
				/* no equal found: start new group */
				if (ngrp == maxgrps) {
					/* we need to extend extents
					 * and histo bats */
					maxgrps += GROUPBATINCR;
					if (maxgrps > BATcount(b))
						maxgrps = BATcount(b);
					if (extents) {
						BATsetcount(en, ngrp);
						en = BATextend(en, maxgrps);
						exts = (oid *) Tloc(en, BUNfirst(en));
					}
					if (histo) {
						BATsetcount(hn, ngrp);
						hn = BATextend(hn, maxgrps);
						cnts = (wrd *) Tloc(hn, BUNfirst(hn));
					}
				}
				if (extents)
					exts[ngrp] = b->hseqbase + (oid) (p - r);
				if (histo)
					cnts[ngrp] = 1;
				ngrps[p - r] = ngrp;
				ngrp++;
			}
		}
		gn->tsorted = BATcount(gn) <= 1;
	} else {
		const char *nme;
		size_t nmelen;
		Heap *hp = NULL;
		BUN prb;

		/* not sorted, and no pre-existing hash table: we'll
		 * build an incomplete hash table on the fly--also see
		 * BATassertHeadProps and BATderiveHeadProps for
		 * similar code */
		nme = BBP_physical(b->batCacheid);
		nmelen = strlen(nme);
		if ((hp = GDKzalloc(sizeof(Heap))) == NULL ||
		    (hp->filename = GDKmalloc(nmelen + 30)) == NULL ||
		    snprintf(hp->filename, nmelen + 30,
			     "%s.hash" SZFMT, nme, MT_getpid()) < 0 ||
		    (ext = GDKstrdup(hp->filename + nmelen + 1)) == NULL ||
		    (hs = HASHnew(hp, b->ttype, BUNlast(b),
				  HASHmask(b->batCount))) == NULL) {
			if (hp) {
				if (hp->filename)
					GDKfree(hp->filename);
				GDKfree(hp);
			}
			if (ext)
				GDKfree(ext);
			hp = NULL;
			ext = NULL;
			GDKerror("BATgroup: cannot allocate hash table\n");
			goto error;
		}
		for (r = BUNfirst(b), p = r, q = r + BATcount(b); p < q; p++) {
			v = BUNtail(bi, p);
			prb = HASHprobe(hs, v);
			for (hb = hs->hash[prb];
			     hb != BUN_NONE;
			     hb = hs->link[hb]) {
				if ((grps == NULL ||
				     grps[hb - r] == grps[p - r]) &&
				    cmp(v, BUNtail(bi, hb)) == 0) {
					ngrps[p - r] = ngrps[hb - r];
					if (histo)
						cnts[ngrps[hb - r]]++;
					break;
				}
			}
			if (hb == BUN_NONE) {
				/* no equal found: start new group and
				 * enter into hash table */
				if (ngrp == maxgrps) {
					/* we need to extend extents
					 * and histo bats */
					maxgrps += GROUPBATINCR;
					if (maxgrps > BATcount(b))
						maxgrps = BATcount(b);
					if (extents) {
						BATsetcount(en, ngrp);
						en = BATextend(en, maxgrps);
						exts = (oid *) Tloc(en, BUNfirst(en));
					}
					if (histo) {
						BATsetcount(hn, ngrp);
						hn = BATextend(hn, maxgrps);
						cnts = (wrd *) Tloc(hn, BUNfirst(hn));
					}
				}
				if (extents)
					exts[ngrp] = b->hseqbase + (oid) (p - r);
				if (histo)
					cnts[ngrp] = 1;
				ngrps[p - r] = ngrp;
				ngrp++;
				hs->link[p] = hs->hash[prb];
				hs->hash[prb] = p;
			}
		}
		if (hp->storage == STORE_MEM)
			HEAPfree(hp);
		else
			HEAPdelete(hp, nme, ext);
		GDKfree(hp);
		GDKfree(hs);
		GDKfree(ext);
		gn->tsorted = BATcount(gn) <= 1;
	}
	if (extents) {
		BATsetcount(en, (BUN) ngrp);
		BATseqbase(en, 0);
		en->tkey = 1;
		en->tsorted = 1;
		en->trevsorted = BATcount(en) <= 1;
		en->T->nonil = 1;
		en->T->nil = 0;
		*extents = en;
	}
	if (histo) {
		BATsetcount(hn, (BUN) ngrp);
		BATseqbase(hn, 0);
		hn->tkey = 0;
		hn->tsorted = 0;
		hn->trevsorted = 0;
		hn->T->nonil = 1;
		hn->T->nil = 0;
		*histo = hn;
	}
	gn->tkey = ngrp == BATcount(gn);
	gn->trevsorted = BATcount(gn) <= 1;
	gn->T->nonil = 1;
	gn->T->nil = 0;
	*groups = gn;
	return GDK_SUCCEED;
  error:
	if (gn)
		BBPunfix(gn->batCacheid);
	if (en)
		BBPunfix(en->batCacheid);
	if (hn)
		BBPunfix(hn->batCacheid);
	return GDK_FAIL;
}

gdk_return
BATgroup(BAT **groups, BAT **extents, BAT **histo,
	 BAT *b, BAT *g, BAT *e, BAT *h)
{
	return BATgroup_internal(groups, extents, histo, b, g, e, h, 0);
}
