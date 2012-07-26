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

/*
 * @a M. L. Kersten, P. Boncz, S. Manegold, N. Nes
 * @* Common BAT Operations
 * This module contains the following BAT algebra operations:
 * @itemize
 * @item bulk updates
 * multi-insert, multi-delete, multi-replace
 * @item common aggregates
 * min, max and histogram
 * @item oid column manipulations
 * mark, number and split.
 * @item bat selections
 * select, slice, sample, fragment and restrict.
 * Note: non hash-/index-supported scanselects have been "outsourced"
 * to gdk_scanselect.mx as the fully expanded code grows too large to
 * be (conveniently) compiled in a single file.
 * @item bat partitioning
 * hash partition, range partitioning
 * @end itemize
 * We factor out all possible overhead by inlining code.  This
 * includes the macros BUNhead and BUNtail, which do a test to see
 * whether the atom resides in the buns or in a variable storage
 * heap. The updateloop(dstbat, srcbat, operation) macro invokes
 * operation(dstbat, BUNhead(srcbat), BUNtail(srcbat)) on all buns of
 * the srcbat, but testing only once where they reside.
 */
#include "monetdb_config.h"
#include "gdk.h"
#include "gdk_private.h"

#define updateloop(bn, b, func)						\
	do {								\
		BATiter bi = bat_iterator(b);				\
		BUN p1, p2;						\
									\
		BATloop(b, p1, p2) {					\
			func(bn, BUNhead(bi, p1), BUNtail(bi, p1));	\
		}							\
	} while (0)

/*
 * @+ BAT insert/delete/replace
 * The content of a BAT can be appended to (removed from) another
 * using BATins (BATdel).
 */
static BAT *
insert_string_bat(BAT *b, BAT *n, int append)
{
	BATiter ni;		/* iterator */
	int tt;			/* tail type */
	size_t toff = ~(size_t) 0;	/* tail offset */
	BUN p, q;		/* loop variables */
	oid o = 0;		/* in case we're appending */
	ptr hp, tp;		/* head and tail value pointers */
	unsigned char tbv;	/* tail value-as-bte */
	unsigned short tsv;	/* tail value-as-sht */
#if SIZEOF_VAR_T == 8
	unsigned int tiv;	/* tail value-as-int */
#endif
	var_t v;		/* value */
	int ntw, btw;		/* shortcuts for {b,n}->t->width */

	assert(b->H->type == TYPE_void || b->H->type == TYPE_oid);
	if (n->batCount == 0)
		return b;
	ni = bat_iterator(n);
	btw = b->T->width;
	ntw = n->T->width;
	hp = NULL;
	tp = NULL;
	if (b->H->type != TYPE_void) {
		hp = &o;
		o = MAXoid(b);
	}
	tt = b->ttype;
	if (tt == TYPE_str &&
	    (!GDK_ELIMDOUBLES(b->T->vheap) || b->batCount == 0) &&
	    !GDK_ELIMDOUBLES(n->T->vheap) &&
	    b->T->vheap->hashash == n->T->vheap->hashash &&
	    /* if needs to be kept unique, take slow path */
	    (b->tkey & BOUND2BTRUE) == 0 &&
	    /* if view, only copy if significant part of parent is used */
	    (VIEWtparent(n) == 0 ||
	     BATcapacity(BBP_cache(VIEWtparent(n))) < 2 * BATcount(n))) {
		/* append string heaps */
		toff = b->batCount == 0 ? 0 : b->T->vheap->free;
		/* make sure we get alignment right */
		toff = (toff + GDK_VARALIGN - 1) & ~(GDK_VARALIGN - 1);
		assert(((toff >> GDK_VARSHIFT) << GDK_VARSHIFT) == toff);
		if (HEAPextend(b->T->vheap, toff + n->T->vheap->size) < 0) {
			toff = ~ (size_t) 0;
			goto bunins_failed;
		}
		memcpy(b->T->vheap->base + toff, n->T->vheap->base, n->T->vheap->size);
		b->T->vheap->free = toff + n->T->vheap->free;
		/* flush double-elimination hash table */
		memset(b->T->vheap->base, 0, GDK_STRHASHSIZE);
		if (b->T->width < SIZEOF_VAR_T &&
		    ((size_t) 1 << 8 * b->T->width) < (b->T->width <= 2 ? (b->T->vheap->size >> GDK_VARSHIFT) - GDK_VAROFFSET : (b->T->vheap->size >> GDK_VARSHIFT))) {
			/* offsets aren't going to fit */
			if (GDKupgradevarheap(b->T, (var_t) (b->T->vheap->size >> GDK_VARSHIFT), 0) == GDK_FAIL) {
				toff = ~ (size_t) 0;
				goto bunins_failed;
			}
			btw = b->T->width;
		}
		switch (btw) {
		case 1:
			tt = TYPE_bte;
			break;
		case 2:
			tt = TYPE_sht;
			break;
#if SIZEOF_VAR_T == 8
		case 4:
			tt = TYPE_int;
			break;
#endif
		default:
			tt = TYPE_var;
			break;
		}
		b->T->varsized = 0;
		n->T->varsized = 0;
		b->T->type = tt;
	}

	BATloop(n, p, q) {
		if (!append)
			hp = b->H->type ? BUNhloc(ni, p) : NULL;

		tp = b->T->type ? BUNtail(ni, p) : NULL;
		if (toff != ~ (size_t) 0) {
			assert(tp != NULL);
			switch (ntw) {
			case 1:
				v = (var_t) * (unsigned char *) tp + GDK_VAROFFSET;
				break;
			case 2:
				v = (var_t) * (unsigned short *) tp + GDK_VAROFFSET;
				break;
#if SIZEOF_VAR_T == 8
			case 4:
				v = (var_t) * (unsigned int *) tp;
				break;
#endif
			default:
				v = * (var_t *) tp;
				break;
			}
			v = (var_t) ((((size_t) v << GDK_VARSHIFT) + toff) >> GDK_VARSHIFT);
			assert(v >= GDK_VAROFFSET);
			assert(((size_t) v << GDK_VARSHIFT) < b->T->vheap->free);
			switch (btw) {
			case 1:
				assert(v - GDK_VAROFFSET < ((var_t) 1 << 8));
				tbv = (unsigned char) (v - GDK_VAROFFSET);
				tp = (ptr) &tbv;
				break;
			case 2:
				assert(v - GDK_VAROFFSET < ((var_t) 1 << 16));
				tsv = (unsigned short) (v - GDK_VAROFFSET);
				tp = (ptr) &tsv;
				break;
#if SIZEOF_VAR_T == 8
			case 4:
				assert(v < ((var_t) 1 << 32));
				tiv = (unsigned int) v;
				tp = (ptr) &tiv;
				break;
#endif
			default:
				tp = (ptr) &v;
				break;
			}
		}
		bunfastins(b, hp, tp);
		if (append)
			o++;
	}
	if (toff != ~(size_t) 0) {
		b->T->varsized = 1;
		n->T->varsized = 1;
		b->T->type = TYPE_str;
	}
	return b;
      bunins_failed:
	if (toff != ~(size_t) 0) {
		b->T->varsized = 1;
		n->T->varsized = 1;
		b->T->type = TYPE_str;
	}
	return NULL;
}

#define bunins(b,h,t) if (BUNins(b,h,t,force) == NULL) return NULL;
BAT *
BATins(BAT *b, BAT *n, bit force)
{
	BAT *tmp = NULL, *res = NULL;
	int fastpath = 0;
	int countonly;

	if (b == NULL || n == NULL || BATcount(n) == 0) {
		return b;
	}
	if (b->htype != TYPE_void && b->htype != TYPE_oid) {
		GDKerror("BATins: input must be (V)OID headed\n");
		return NULL;
	}
	ALIGNins(b, "BATins", force);
	BATcompatible(b, n);

	countonly = (b->htype == TYPE_void && b->ttype == TYPE_void);

	if (BUNlast(b) + BATcount(n) > BUN_MAX) {
		GDKerror("BATins: combined BATs too large\n");
		return NULL;
	}

	if (b->htype != TYPE_void &&
	    (b->ttype == TYPE_void ||
	     (!b->H->hash && b->T->hash &&
	      ATOMstorage(b->ttype) == TYPE_int))) {	/* OIDDEPEND */
		return BATmirror(BATins(BATmirror(b), BATmirror(n), force));
	}

	if (BUNlast(b) + BATcount(n) > BATcapacity(b)) {
		/* if needed space exceeds a normal growth extend just
		 * with what's needed */
		BUN ncap = BUNlast(b) + BATcount(n);
		BUN grows = BATgrows(b);

		if (ncap > grows)
			grows = ncap;
		if (BATextend(b, grows) == NULL)
			goto bunins_failed;
	}

	if (b->htype == TYPE_void && b->hseqbase != oid_nil) {
		BATiter ni = bat_iterator(n);
		BUN t = (BUN) b->hseqbase + BATcount(b) - 1;
		oid h = *(oid *) BUNhead(ni, BUNfirst(n));

		if (BATcount(b) == 0 && (BATcount(n) == 1 || BAThdense(n))) {
			b->hseqbase = h;
		} else if (t + 1 != (BUN) h || !BAThdense(n)) {
			b = BATmaterializeh(b);
			countonly = 0;
			if (b == NULL)
				return NULL;
		}
	}
	if (b->ttype == TYPE_void && b->tseqbase != oid_nil) {
		BATiter ni = bat_iterator(n);
		BUN t = (BUN) b->tseqbase + BATcount(b) - 1;
		oid h = *(oid *) BUNtail(ni, BUNfirst(n));

		if (BATcount(b) == 0 && (BATcount(n) == 1 || BATtdense(n))) {
			b->tseqbase = h;
		} else if (t + 1 != (BUN) h || !BATtdense(n)) {
			b = BATmaterializet(b);
			countonly = 0;
			if (b == NULL)
				return NULL;
		}
	}
	if (b->T->hash == NULL && b->batSet == 0 &&
	    (b->tkey & BOUND2BTRUE) == 0 &&
	    ((b->hkey & BOUND2BTRUE) == 0 || n->hkey) &&
	    (b->H->hash == NULL || ATOMstorage(b->htype) == ATOMstorage(TYPE_oid))) {
		if (b->hkey & BOUND2BTRUE && b->batCount > 0) {
			tmp = n = BATkdiff(n, b);
			if (n == NULL)
				return NULL;
		}
		fastpath = 1;
	}

	if (b->H->hash == NULL && b->hkey & BOUND2BTRUE &&
	    (b->batCount > 0 || !n->hkey) &&
	    (BATcount(b) ||
	     (BATcount(n) && n->htype != TYPE_void && !n->hdense))) {
		BAThash(b, BATcount(b) + BATcount(n));
	}
	if (b->T->hash == NULL && b->tkey & BOUND2BTRUE &&
	    (b->batCount > 0 || !n->tkey) &&
	    (BATcount(b) ||
	     (BATcount(n) && n->ttype != TYPE_void && !n->tdense))) {
		BAThash(BATmirror(b), BATcount(b) + BATcount(n));
	}
	BATaccessBegin(b, USE_HHASH | USE_THASH, MMAP_WILLNEED);
	BATaccessBegin(n, USE_HEAD | USE_TAIL, MMAP_SEQUENTIAL);
	b->batDirty = 1;
	if (fastpath) {
		BUN p, q, r = BUNlast(b);

		if (BATcount(b) == 0) {
			ALIGNset(b, n);
		} else if (BATcount(n)) {
			BUN last = BUNlast(b) - 1;
			BATiter ni = bat_iterator(n);
			BATiter bi = bat_iterator(b);
			int xx = ATOMcmp(b->htype, BUNhead(ni, BUNfirst(n)), BUNhead(bi, last));
			if (BAThordered(b) && (!BAThordered(n) || xx < 0)) {
				b->hsorted = FALSE;
				b->H->nosorted = r;
				if (b->hdense) {
					b->hdense = FALSE;
					b->H->nodense = r;
				}
			}
			if (BAThrevordered(b) &&
			    (!BAThrevordered(n) || xx > 0)) {
				b->hrevsorted = FALSE;
				b->H->norevsorted = r;
			}
			if (b->hkey && (b->hkey & BOUND2BTRUE) == 0 &&
			    (!(BAThordered(b) || BAThrevordered(b)) ||
			     n->hkey == 0 || xx == 0)) {
				BATkey(b, FALSE);
			}
			if (b->htype != TYPE_void && b->hsorted && b->hdense &&
			    (BAThdense(n) == 0 ||
			     *(oid *) BUNhloc(bi, last) != 1 + *(oid *) BUNhead(ni, BUNfirst(n)))) {
				b->hdense = FALSE;
				b->H->nodense = r;
			}

			xx = ATOMcmp(b->ttype, BUNtail(ni, BUNfirst(n)), BUNtail(bi, last));
			if (BATtordered(b) && (!BATtordered(n) || xx < 0)) {
				b->tsorted = FALSE;
				b->T->nosorted = r;
				if (b->tdense) {
					b->tdense = FALSE;
					b->T->nodense = r;
				}
			}
			if (BATtrevordered(b) &&
			    (!BATtrevordered(n) || xx > 0)) {
				b->trevsorted = FALSE;
				b->T->norevsorted = r;
			}
			if (b->tkey && (b->tkey & BOUND2BTRUE) == 0 &&
			    (!(BATtordered(b) || BATtrevordered(b)) ||
			     n->tkey == 0 || xx == 0)) {
				BATkey(BATmirror(b), FALSE);
			}
			if (b->ttype != TYPE_void && b->tsorted && b->tdense &&
			    (BATtdense(n) == 0 ||
			     *(oid *) BUNtloc(bi, last) != 1 + *(oid *) BUNtail(ni, BUNfirst(n)))) {
				b->tdense = FALSE;
				b->T->nodense = r;
			}
		}
		if (countonly) {
			BATsetcount(b, b->batCount + n->batCount);
		} else if ((b->htype == TYPE_str &&
			    (b->batCount == 0 || !GDK_ELIMDOUBLES(b->H->vheap)) &&
			    !GDK_ELIMDOUBLES(n->H->vheap) &&
			    b->H->vheap->hashash == n->H->vheap->hashash &&
			    VIEWhparent(n) == 0) ||
			   (b->ttype == TYPE_str &&
			    (b->batCount == 0 || !GDK_ELIMDOUBLES(b->T->vheap)) &&
			    !GDK_ELIMDOUBLES(n->T->vheap) &&
			    b->T->vheap->hashash == n->T->vheap->hashash &&
			    VIEWtparent(n) == 0)) {
			b = insert_string_bat(b, n, 0);
		} else if (b->htype == TYPE_void) {
			BATiter ni = bat_iterator(n);
			BATloop(n, p, q) {
				bunfastins_nocheck(b, r, NULL, BUNtail(ni, p), 0, Tsize(b));
				r++;
			}
		} else if (b->H->hash) {
			BUN i = BUNlast(b);
			BATiter ni = bat_iterator(n);

			BATloop(n, p, q) {
				const void *v = BUNhead(ni, p);

				bunfastins_nocheck(b, r, v, BUNtail(ni, p), Hsize(b), Tsize(b));
				HASHins_oid(b->H->hash, i, v);
				r++;
				i++;
			}
		} else {
			BATiter ni = bat_iterator(n);

			BATloop(n, p, q) {
				bunfastins_nocheck(b, r, BUNhead(ni, p), BUNtail(ni, p), Hsize(b), Tsize(b));
				r++;
			}
		}
		b->H->nonil &= n->H->nonil;
		b->T->nonil &= n->T->nonil;
	} else {
		updateloop(b, n, bunins);
	}
	res = b;
      bunins_failed:
	BATaccessEnd(b, USE_HHASH | USE_THASH, MMAP_WILLNEED);
	BATaccessEnd(n, USE_HEAD | USE_TAIL, MMAP_SEQUENTIAL);
	if (tmp)
		BBPreclaim(tmp);
	return res;
}

BAT *
BATappend(BAT *b, BAT *n, bit force)
{
	BUN sz;
	int fastpath = 1;

	if (b == NULL || n == NULL || (sz = BATcount(n)) == 0) {
		return b;
	}
	if (b->htype != TYPE_void && b->htype != TYPE_oid) {
		GDKerror("BATappend: input must be (V)OID headed\n");
		return NULL;
	}
	ALIGNapp(b, "BATappend", force);
	BATcompatible(b, n);

	if (BUNlast(b) + BATcount(n) > BUN_MAX) {
		GDKerror("BATappend: combined BATs too large\n");
		return NULL;
	}

	b->batDirty = 1;
	BATaccessBegin(b, USE_HHASH | USE_THASH, MMAP_WILLNEED);
	BATaccessBegin(n, USE_HEAD | USE_TAIL, MMAP_SEQUENTIAL);

	if (sz > BATcapacity(b) - BUNlast(b)) {
		/* if needed space exceeds a normal growth extend just
		 * with what's needed */
		BUN ncap = BUNlast(b) + sz;
		BUN grows = BATgrows(b);

		if (ncap > grows)
			grows = ncap;
		if (BATextend(b, grows) == NULL)
			goto bunins_failed;
	}

	/* append two void,void bats */
	if (b->ttype == TYPE_void && BATtdense(b)) {
		oid f = n->tseqbase;

		if (n->ttype != TYPE_void)
			f = *(oid *) BUNtloc(bat_iterator(n), BUNfirst(n));

		if (BATcount(b) == 0 && f != oid_nil)
			BATseqbase(BATmirror(b), f);
		if (BATtdense(n) && BATcount(b) + b->tseqbase == f) {
			if (b->htype != TYPE_void) {
				BUN r = BUNlast(b);
				oid m;

				f = MAXoid(b);
				f++;
				if (f + sz >= GDK_oid_max) {
					GDKerror("BATappend: overflow of head value\n");
					return 0;
				}
				m = (oid) (f + sz);
				for (; f < m; f++, r++) {
					bunfastins_nocheck(b, r, (ptr) &f, NULL, Hsize(b), 0);
				}
			} else {
				sz += BATcount(b);
				BATsetcount(b, sz);
			}
			return b;
		}
		/* we need to materialize the tail */
		b = BATmaterializet(b);
		if (b == NULL)
			return NULL;
	}

	/* a hash is useless for void bats */
	if (b->H->hash)
		HASHremove(b);

	if (b->T->hash && (2 * b->T->hash->mask) < (BATcount(b) + sz)) {
		HASHremove(BATmirror(b));
	}
	if (b->T->hash != NULL ||
	    b->batSet ||
	    (b->tkey & BOUND2BTRUE) != 0 ||
	    (b->H->hash != NULL && ATOMstorage(b->htype) != ATOMstorage(TYPE_oid)))
		fastpath = 0;

	if (fastpath) {
		BUN p, q, r = BUNlast(b);

		if (BATcount(b) == 0) {
			BATiter ni = bat_iterator(n);

			ALIGNsetH(BATmirror(b), BATmirror(n));
			b->tseqbase = n->tseqbase;

			if (n->tdense && n->ttype == TYPE_oid) {
				b->tseqbase = *(oid *) BUNtail(ni, BUNfirst(n));
			}
			b->tdense = n->tdense;
			b->T->nodense = n->T->nodense;
			b->tkey |= (n->tkey & TRUE);
			b->T->nokey[0] = n->T->nokey[0];
			b->T->nokey[1] = n->T->nokey[1];
			b->T->nonil = n->T->nonil;
		} else {
			BUN last = BUNlast(b) - 1;
			BATiter ni = bat_iterator(n);
			BATiter bi = bat_iterator(b);
			int xx = ATOMcmp(b->ttype, BUNtail(ni, BUNfirst(n)), BUNtail(bi, last));
			if (BATtordered(b) && (!BATtordered(n) || xx < 0)) {
				b->tsorted = FALSE;
				b->T->nosorted = r;
				if (b->tdense) {
					b->tdense = FALSE;
					b->T->nodense = r;
				}
			}
			if (BATtrevordered(b) &&
			    (!BATtrevordered(n) || xx > 0)) {
				b->trevsorted = FALSE;
				b->T->norevsorted = r;
			}
			if (b->tkey &&
			    (!(BATtordered(b) || BATtrevordered(b)) ||
			     n->tkey == 0 || xx == 0)) {
				BATkey(BATmirror(b), FALSE);
			}
			if (b->ttype != TYPE_void && b->tsorted && b->tdense &&
			    (BATtdense(n) == 0 ||
			     *(oid *) BUNtloc(bi, last) != 1 + *(oid *) BUNtail(ni, BUNfirst(n)))) {
				b->tdense = FALSE;
				b->T->nodense = r;
			}
		}
		if (b->ttype == TYPE_str &&
		    (b->batCount == 0 || !GDK_ELIMDOUBLES(b->T->vheap)) &&
		    !GDK_ELIMDOUBLES(n->T->vheap) &&
		    b->T->vheap->hashash == n->T->vheap->hashash &&
		    VIEWtparent(n) == 0) {
			b = insert_string_bat(b, n, 1);
			if (b == NULL)
				return NULL;
		} else if (b->htype == TYPE_void) {
			BATiter ni = bat_iterator(n);

			BATloop(n, p, q) {
				bunfastins_nocheck(b, r, NULL, BUNtail(ni, p), 0, Tsize(b));
				r++;
			}
			if (b->hseqbase != oid_nil)
				b->hrevsorted = 0;
		} else {
			oid o = MAXoid(b);
			BATiter ni = bat_iterator(n);

			o++;
			BATloop(n, p, q) {
				bunfastins_nocheck(b, r, &o, BUNtail(ni, p), Hsize(b), Tsize(b));
				o++;
				r++;
			}
			b->hrevsorted = 0;
		}
	} else {
		BUN p, q;
		BUN i = BUNlast(b);
		BAT *bm = BBP_cache(-b->batCacheid);
		BATiter ni = bat_iterator(n);

		if (b->tkey & BOUND2BTRUE) {
			b->tdense = b->tsorted = b->trevsorted = 0;
			if (b->hseqbase != oid_nil) {
				oid h = b->hseqbase + i;

				BATloop(n, p, q) {
					const void *t = BUNtail(ni, p);
					if (BUNfnd(bm, t) == BUN_NONE) {
						bunfastins(b, &h, t);
						if (b->T->hash) {
							HASHins(bm, i, t);
						}
						h++;
						i++;
					}
				}
				b->hrevsorted = 0;
			} else {
				oid on = oid_nil;

				BATloop(n, p, q) {
					const void *t = BUNtail(ni, p);
					if (BUNfnd(bm, t) == BUN_NONE) {
						bunfastins(b, &on, t);
						if (b->T->hash) {
							HASHins(bm, i, t);
						}
						i++;
					}
				}
				BATkey(b, FALSE);
				b->hdense = b->hsorted = b->hrevsorted = 0;
			}
		} else if (b->hseqbase != oid_nil) {
			oid h;

			if (b->hseqbase + BATcount(b) + BATcount(n) >= GDK_oid_max) {
				GDKerror("BATappend: overflow of head value\n");
				return NULL;
			}
			h = (oid) (b->hseqbase + BATcount(b));

			BATloop(n, p, q) {
				const void *t = BUNtail(ni, p);

				bunfastins(b, &h, t);
				if (b->T->hash) {
					HASHins(bm, i, t);
				}
				i++;
				h++;
			}
			BATkey(BATmirror(b), FALSE);
			b->tdense = b->tsorted = b->trevsorted = 0;
			b->hrevsorted = 0;
		} else {
			oid on = oid_nil;

			BATloop(n, p, q) {
				const void *t = BUNtail(ni, p);

				bunfastins(b, &on, t);
				if (b->T->hash) {
					HASHins(bm, i, t);
				}
				i++;
			}
			BATkey(b, FALSE);
			BATkey(BATmirror(b), FALSE);
			b->hdense = b->hsorted = b->hrevsorted = 0;
			b->tdense = b->tsorted = b->trevsorted = 0;
		}
	}
	b->H->nonil &= n->H->nonil;
	b->T->nonil &= n->T->nonil;
	BATaccessEnd(b, USE_HHASH | USE_THASH, MMAP_WILLNEED);
	BATaccessEnd(n, USE_HEAD | USE_TAIL, MMAP_SEQUENTIAL);
	return b;
      bunins_failed:
	BATaccessEnd(b, USE_HHASH | USE_THASH, MMAP_WILLNEED);
	BATaccessEnd(n, USE_HEAD | USE_TAIL, MMAP_SEQUENTIAL);
	return NULL;
}


#define bundel(b,h,t) do { if (BUNdel(b,h,t,force) == NULL) { GDKerror("BATdel: BUN does not occur.\n"); return NULL; } } while (0)
BAT *
BATdel(BAT *b, BAT *n, bit force)
{
	ERRORcheck(b == NULL, "set:BAT required\n");
	ERRORcheck(n == NULL, "set:BAT required\n");
	if (BATcount(n) == 0) {
		return b;
	}
	ALIGNdel(b, "BATdel", force);
	TYPEcheck(b->htype, n->htype);
	TYPEcheck(b->ttype, n->ttype);
	updateloop(b, n, bundel);
	return b;
}

#define bundelhead(b,h,t) do { if (BUNdelHead(b,h,force) == NULL) return NULL; } while (0)
BAT *
BATdelHead(BAT *b, BAT *n, bit force)
{
	ERRORcheck(b == NULL, "set:BAT required\n");
	ERRORcheck(n == NULL, "set:BAT required\n");
	if (BATcount(n) == 0) {
		return b;
	}
	ALIGNdel(b, "BATdelHead", force);
	TYPEcheck(b->htype, n->htype);
	updateloop(b, n, bundelhead);
	return b;
}

/*
 * The last in this series is a BATreplace, which replaces all the
 * buns mentioned.
 */
#define BUNreplace_force(a,b,c) BUNreplace(a,b,c,force)
BAT *
BATreplace(BAT *b, BAT *n, bit force)
{
	if (b == NULL || n == NULL || BATcount(n) == 0) {
		return b;
	}
	BATcompatible(b, n);
	updateloop(b, n, BUNreplace_force);

	return b;
}


/*
 * @+ BAT Selections
 * The BAT selectors are among the most heavily used operators.
 * Their efficient implementation is therefore mandatory.
 *
 * The interface supports seven operations: BATslice, BATselect,
 * BATfragment, BATproject, BATrestrict.
 *
 * @- BAT slice
 * This function returns a horizontal slice from a BAT. It optimizes
 * execution by avoiding to copy when the BAT is memory mapped (in
 * this case, an independent submap is created) or else when it is
 * read-only, then a VIEW bat is created as a result.
 *
 * If a new copy has to be created, this function takes care to
 * preserve void-columns (in this case, the seqbase has to be
 * recomputed in the result).
 *
 * Note that the BATslice() is used indirectly as well as a special
 * case for BATselect (range selection on sorted column) and
 * BATsemijoin (when two dense columns are semijoined).
 *
 * NOTE new semantics, the selected range is excluding the high value.
 */
BAT *
BATslice(BAT *b, BUN l, BUN h)
{
	BUN low = l;
	BAT *bn;
	BATiter bni, bi = bat_iterator(b);

	BATcheck(b, "BATslice");
	if (h > BATcount(b))
		h = BATcount(b);
	if (h < l)
		h = l;
	l += BUNfirst(b);
	h += BUNfirst(b);

	if (l > BUN_MAX || h > BUN_MAX) {
		GDKerror("BATslice: boundary out of range\n");
		return NULL;
	}

	/*
	 * If the source BAT is readonly, then we can obtain a VIEW
	 * that just reuses the memory of the source.
	 */
	if (BAThrestricted(b) == BAT_READ && BATtrestricted(b) == BAT_READ) {
		BUN cnt = h - l;
		bn = VIEWcreate_(b, b, TRUE);
		bn->batFirst = bn->batDeleted = bn->batInserted = 0;
		bn->H->heap.base = (bn->htype) ? BUNhloc(bi, l) : NULL;
		bn->T->heap.base = (bn->ttype) ? BUNtloc(bi, l) : NULL;
		bn->H->heap.maxsize = bn->H->heap.size = headsize(bn, cnt);
		bn->T->heap.maxsize = bn->T->heap.size = tailsize(bn, cnt);
		BATsetcount(bn, cnt);
		BATsetcapacity(bn, cnt);
	/*
	 * We have to do it: create a new BAT and put everything into it.
	 */
	} else {
		BUN p = (BUN) l;
		BUN q = (BUN) h;

		bn = BATnew(b->htype, b->ttype, h - l);
		if (bn == NULL) {
			return bn;
		}
		if (b->htype != b->ttype || b->htype != TYPE_void) {
			for (; p < q; p++) {
				bunfastins(bn, BUNhead(bi, p), BUNtail(bi, p));
			}
		} else {
			BATsetcount(bn, h - l);
		}
	}
	bni = bat_iterator(bn);
	if (BAThdense(b)) {
		bn->hdense = TRUE;
		BATseqbase(bn, (oid) (b->hseqbase + low));
	} else if (bn->hkey && bn->htype == TYPE_oid) {
		if (BATcount(bn) == 0) {
			bn->hdense = TRUE;
			BATseqbase(bn, 0);
		} else if (bn->hsorted && *(oid *) BUNhloc(bni, BUNfirst(bn)) + BATcount(bn) - 1 == *(oid *) BUNhloc(bni, BUNlast(bn) - 1)) {
			bn->hdense = TRUE;
			BATseqbase(bn, *(oid *) BUNhloc(bni, BUNfirst(bn)));
		}
	}
	if (BATtdense(b)) {
		bn->tdense = TRUE;
		BATseqbase(BATmirror(bn), (oid) (b->tseqbase + low));
	} else if (bn->tkey && bn->ttype == TYPE_oid) {
		if (BATcount(bn) == 0) {
			bn->tdense = TRUE;
			BATseqbase(BATmirror(bn), 0);
		} else if (bn->tsorted && *(oid *) BUNtloc(bni, BUNfirst(bn)) + BATcount(bn) - 1 == *(oid *) BUNtloc(bni, BUNlast(bn) - 1)) {
			bn->tdense = TRUE;
			BATseqbase(BATmirror(bn), *(oid *) BUNtloc(bni, BUNfirst(bn)));
		}
	}
	bn->hsorted = BAThordered(b);
	bn->tsorted = BATtordered(b);
	bn->hrevsorted = BAThrevordered(b);
	bn->trevsorted = BATtrevordered(b);
	BATkey(bn, BAThkey(b));
	BATkey(BATmirror(bn), BATtkey(b));
	bn->H->nonil = b->H->nonil || bn->batCount == 0;
	bn->T->nonil = b->T->nonil || bn->batCount == 0;
	bn->H->nil = bn->T->nil = 0; /* we just don't know */
	return bn;
      bunins_failed:
	BBPreclaim(bn);
	return NULL;
}

/*
 * @- Top-N selection
 *
 * The top-N elements can be easily obtained by trimming the
 * space. The auxiliary index structures are removed.  For
 * non-variable size BATs it merely requires adjustment of the free
 * space labels. Other BATs require a loop through the tuples to be
 * deleted. [todo]
 */
int
BATtopN(BAT *b, BUN topN)
{
	BATcheck(b, "BATtopN");
	if (topN > BATcount(b)) {
		GDKerror("BATtopN: not enough tuples in target\n");
	} else if (b->H->varsized || b->T->varsized) {
		HASHremove(b);
		while (BATcount(b) > topN)
			BUNdelete(b, BUNlast(b), FALSE);
	} else {
		HASHremove(b);
		BATsetcount(b, topN);
	}
	/* we no longer know if there are NILs */
	b->H->nil = b->htype == TYPE_void && b->hseqbase == oid_nil && topN >= 1;
	b->T->nil = b->ttype == TYPE_void && b->tseqbase == oid_nil && topN >= 1;
	return 0;
}

/*
 * The baseline algorithm for fragment location is a two-phase
 * process.  First we search on the 1st dimension and collect the
 * qualifying BUNs in a marking on the stack. In the second phase, the
 * tail is analyzed for all items already marked and qualifying
 * associations are copied into the result.  An index is exploited
 * when possible.
 */
#define restrict1(cmptype, TYPE, BUNhead)				\
	do {								\
		if (BAThordered(b)) {					\
			BUN p1, p2;					\
									\
			b = BATmirror(b);				\
			SORTloop(b, p1, p2, hl, hh) {			\
				*m++ = p1;				\
			}						\
			b = BATmirror(b);				\
		} else {						\
			int lval = !cmptype##_EQ(ATOMnilptr(t), hl, TYPE); \
			int hval = !cmptype##_EQ(ATOMnilptr(t), hh, TYPE); \
									\
			if (hval && lval && cmptype##_GT(hl,hh,TYPE)) {	\
				GDKerror("BATrestrict: illegal head range.\n");	\
			} else {					\
				BATiter bi = bat_iterator(b);		\
									\
				BATloop(b, p, l) {			\
					if ((!lval || cmptype##_LE(hl, BUNhead(bi, p), TYPE)) && \
					    (!hval || cmptype##_LE(BUNhead(bi, p), hh, TYPE))) { \
						*m++ = p;		\
					}				\
				}					\
			}						\
		}							\
	} while (0)

#define restrict2(cmptype, TYPE, BUNhead, BUNtail)			\
	do {								\
		tl = cmptype##_EQ(ATOMnilptr(t), tl, TYPE) ? 0 : tl;	\
		th = cmptype##_EQ(ATOMnilptr(t), th, TYPE) ? 0 : th;	\
		if (th && tl && cmptype##_GT(tl, th, TYPE)) {		\
			GDKerror("BATrestrict: illegal tail range.\n");	\
		} else {						\
			BATiter bi = bat_iterator(b);			\
									\
			for (; i < m; i++) {				\
				const void *v = BUNtail(bi, *i);	\
									\
				if ((!tl || cmptype##_LE(tl, v, TYPE)) && \
				    (!th || cmptype##_LE(v, th, TYPE))) { \
					bunfastins(bn, BUNhead(bi, *i), v); \
				}					\
			}						\
		}							\
	} while (0)

BAT *
BATrestrict(BAT *b, const void *hl, const void *hh, const void *tl, const void *th)
{
	BAT *bn;
	BUN p = 0, l;
	BUN *mark, *m, *i;
	BUN s;
	int t;

	BATcheck(hl, "BATrestrict: hl is null");
	BATcheck(hh, "BATrestrict: hh is null");
	BATcheck(tl, "BATrestrict: tl is null");
	BATcheck(th, "BATrestrict: th is null");
	bn = BATnew(BAThtype(b), BATttype(b), BATguess(b));
	ESTIDEBUG THRprintf(GDKout, "#BATrestrict: estimated resultsize: " BUNFMT "\n", BATguess(b));

	if (bn == NULL) {
		return NULL;
	}
	BATkey(bn, BAThkey(b));
	BATkey(BATmirror(bn), BATtkey(b));
	bn->hsorted = BAThordered(b);
	bn->tsorted = BATtordered(b);
	bn->hrevsorted = BAThrevordered(b);
	bn->trevsorted = BATtrevordered(b);
	bn->H->nonil = b->H->nonil;
	bn->T->nonil = b->T->nonil;

	s = BATcount(b);
	if (s == 0) {
		ESTIDEBUG THRprintf(GDKout, "#BATrestrict: actual resultsize: " BUNFMT "\n", BATcount(bn));

		return bn;
	}
	if ((mark = (BUN *) GDKmalloc((unsigned) s * sizeof(BUN))) == NULL)
		goto bunins_failed;
	m = mark;
	i = mark;
	switch (ATOMstorage(t = b->htype)) {
	case TYPE_bte:
		restrict1(simple, bte, BUNhloc);
		break;
	case TYPE_sht:
		restrict1(simple, sht, BUNhloc);
		break;
	case TYPE_int:
		restrict1(simple, int, BUNhloc);
		break;
	case TYPE_flt:
		restrict1(simple, flt, BUNhloc);
		break;
	case TYPE_dbl:
		restrict1(simple, dbl, BUNhloc);
		break;
	case TYPE_lng:
		restrict1(simple, lng, BUNhloc);
		break;
	default:
		if (b->hvarsized) {
			restrict1(atom, t, BUNhvar);
		} else {
			restrict1(atom, t, BUNhloc);
		}
		break;
	}

	/* second phase */
	if (b->hvarsized) {
		switch (ATOMstorage(t = b->ttype)) {
		case TYPE_bte:
			restrict2(simple, bte, BUNhvar, BUNtloc);
			break;
		case TYPE_sht:
			restrict2(simple, sht, BUNhvar, BUNtloc);
			break;
		case TYPE_int:
			restrict2(simple, int, BUNhvar, BUNtloc);
			break;
		case TYPE_flt:
			restrict2(simple, flt, BUNhvar, BUNtloc);
			break;
		case TYPE_dbl:
			restrict2(simple, dbl, BUNhvar, BUNtloc);
			break;
		case TYPE_lng:
			restrict2(simple, lng, BUNhvar, BUNtloc);
			break;
		default:
			if (b->tvarsized) {
				restrict2(atom, t, BUNhvar, BUNtvar);
			} else {
				restrict2(atom, t, BUNhvar, BUNtloc);
			}
			break;
		}
	} else {
		switch (ATOMstorage(t = b->ttype)) {
		case TYPE_bte:
			restrict2(simple, bte, BUNhloc, BUNtloc);
			break;
		case TYPE_sht:
			restrict2(simple, sht, BUNhloc, BUNtloc);
			break;
		case TYPE_int:
			restrict2(simple, int, BUNhloc, BUNtloc);
			break;
		case TYPE_flt:
			restrict2(simple, flt, BUNhloc, BUNtloc);
			break;
		case TYPE_dbl:
		case TYPE_lng:
			restrict2(simple, lng, BUNhloc, BUNtloc);
			break;
		default:
			if (b->tvarsized) {
				restrict2(atom, t, BUNhloc, BUNtvar);
			} else {
				restrict2(atom, t, BUNhloc, BUNtloc);
			}
			break;
		}
	}
	GDKfree(mark);

	/* propagate alignment info */
	if (BATcount(bn) == BATcount(b))
		ALIGNset(bn, b);

	ESTIDEBUG THRprintf(GDKout, "#BATrestrict: actual resultsize: " BUNFMT "\n", BATcount(bn));

	return bn;

      bunins_failed:
	GDKfree(mark);
	BBPreclaim(bn);
	return NULL;
}

/*
 * @+ BAT Sorting
 * BATsort returns a sorted copy. BATorder sorts the BAT itself.
 */
int
BATordered(BAT* b)
{
	if (!b->hsorted)
		BATderiveHeadProps(b, 0);
	return b->hsorted;
}

int
BATordered_rev(BAT* b)
{
	if (!b->hrevsorted)
		BATderiveHeadProps(b, 0);
	return b->hrevsorted;
}

/* Sort b according to stable and reverse, do it in-place if copy is
 * unset, otherwise do it on a copy */
static BAT *
BATorder_internal(BAT *b, int stable, int reverse, int copy, const char *func)
{
	BATcheck(b, func);
	/* set some trivial properties (probably not necessary, but
	 * it's cheap) */
	if (b->htype == TYPE_void) {
		b->hsorted = 1;
		b->hrevsorted = b->hseqbase == oid_nil || b->U->count <= 1;
		b->hkey |= b->hseqbase != oid_nil;
	} else if (b->U->count <= 1) {
		b->hsorted = b->hrevsorted = 1;
	}
	if (reverse ? b->hrevsorted : b->hsorted) {
		/* b is already ordered as desired, hence we return b
		 * as is */
		return copy ? BATcopy(b, b->htype, b->ttype, FALSE) : b;
	}
	if (copy) {
		/* now make a writable copy that we're going to sort
		 * materialize any VOID columns while we're at it */
		b = BATcopy(b, BAThtype(b), BATttype(b), TRUE);
	} else if (b->ttype == TYPE_void && b->tseqbase != oid_nil) {
		/* materialize void-tail in-place */
		/* note, we don't need to materialize the head column:
		 * if it is void, either we didn't get here (already
		 * sorted correctly), or we will fall into BATrevert
		 * below which does the materialization for us */
		b = BATmaterializet(b);
		if (b == NULL)
			return NULL;
	}

	if ((reverse ? b->hsorted : b->hrevsorted) && (!stable || b->hkey)) {
		/* b is ordered in the opposite direction, hence we
		 * revert b (note that if requesting stable sort, the
		 * column needs to be key) */
		return BATrevert(b);
	}
	if (reverse) {
		if (stable) {
			if (GDKssort_rev(Hloc(b, BUNfirst(b)),
					 Tloc(b, BUNfirst(b)),
					 b->H->vheap ? b->H->vheap->base : NULL,
					 BATcount(b), Hsize(b), Tsize(b),
					 b->htype) < 0) {
				if (copy)
					BBPreclaim(b);
				return NULL;
			}
		} else {
			GDKqsort_rev(Hloc(b, BUNfirst(b)), Tloc(b, BUNfirst(b)),
				     b->H->vheap ? b->H->vheap->base : NULL,
				     BATcount(b), Hsize(b), Tsize(b), b->htype);
		}
		b->hrevsorted = 1;
		b->hsorted = b->U->count <= 1;
	} else {
		if (stable) {
			if (GDKssort(Hloc(b, BUNfirst(b)),
				     Tloc(b, BUNfirst(b)),
				     b->H->vheap ? b->H->vheap->base : NULL,
				     BATcount(b), Hsize(b), Tsize(b),
				     b->htype) < 0) {
				if (copy)
					BBPreclaim(b);
				return NULL;
			}
		} else {
			GDKqsort(Hloc(b, BUNfirst(b)), Tloc(b, BUNfirst(b)),
				 b->H->vheap ? b->H->vheap->base : NULL,
				 BATcount(b), Hsize(b), Tsize(b), b->htype);
		}
		b->hsorted = 1;
		b->hrevsorted = b->U->count <= 1;
	}
	b->tsorted = b->trevsorted = 0;
	HASHdestroy(b);
	ALIGNdel(b, func, FALSE);
	b->hdense = 0;
	b->tdense = 0;
	b->batDirtydesc = b->H->heap.dirty = b->T->heap.dirty = TRUE;

	return b;
}

BAT *
BATorder(BAT *b)
{
	return BATorder_internal(b, 0, 0, 0, "BATorder");
}

BAT *
BATorder_rev(BAT *b)
{
	return BATorder_internal(b, 0, 1, 0, "BATorder_rev");
}

BAT *
BATsorder(BAT *b)
{
	return BATorder_internal(b, 1, 0, 0, "BATsorder");
}

BAT *
BATsorder_rev(BAT *b)
{
	return BATorder_internal(b, 1, 1, 0, "BATsorder_rev");
}

BAT *
BATsort(BAT *b)
{
	return BATorder_internal(b, 0, 0, 1, "BATsort");
}

BAT *
BATsort_rev(BAT *b)
{
	return BATorder_internal(b, 0, 1, 1, "BATsort_rev");
}

BAT *
BATssort(BAT *b)
{
	return BATorder_internal(b, 1, 0, 1, "BATssort");
}

BAT *
BATssort_rev(BAT *b)
{
	return BATorder_internal(b, 1, 1, 1, "BATssort_rev");
}

/*
 * @+ Reverse a BAT
 * BATrevert rearranges a BAT in reverse order.
 */
BAT *
BATrevert(BAT *b)
{
	char *h, *t;
	BUN p, q;
	BATiter bi = bat_iterator(b);
	size_t s;
	int x;

	BATcheck(b, "BATrevert");
	if ((b->htype == TYPE_void && b->hseqbase != oid_nil) || (b->ttype == TYPE_void && b->tseqbase != oid_nil)) {
		/* materialize void columns in-place */
		b = BATmaterialize(b);
		if (b == NULL)
			return NULL;
	}
	ALIGNdel(b, "BATrevert", FALSE);
	s = Hsize(b);
	if (s > 0) {
		h = (char *) GDKmalloc(s);
		if (!h) {
			return NULL;
		}
		for (p = BUNlast(b) ? BUNlast(b) - 1 : 0, q = BUNfirst(b); p > q; p--, q++) {
			char *ph = BUNhloc(bi, p);
			char *qh = BUNhloc(bi, q);

			memcpy(h, ph, s);
			memcpy(ph, qh, s);
			memcpy(qh, h, s);
		}
		GDKfree(h);
	}
	s = Tsize(b);
	if (s > 0) {
		t = (char *) GDKmalloc(s);
		if (!t) {
			return NULL;
		}
		for (p = BUNlast(b) ? BUNlast(b) - 1 : 0, q = BUNfirst(b); p > q; p--, q++) {
			char *pt = BUNtloc(bi, p);
			char *qt = BUNtloc(bi, q);

			memcpy(t, pt, s);
			memcpy(pt, qt, s);
			memcpy(qt, t, s);
		}
		GDKfree(t);
	}
	HASHdestroy(b);
	/* interchange sorted and revsorted */
	x = b->hrevsorted;
	b->hrevsorted = b->hsorted;
	b->hsorted = x;
	x = b->trevsorted;
	b->trevsorted = b->tsorted;
	b->tsorted = x;
	b->hdense = FALSE;
	b->tdense = FALSE;
	return b;
}

/*
 * @+ Introducing OID Columns
 * The BATmark operation is normally used to prepare a class of query
 * results. Likewise, BATnumber is heavily used in the SQL front-end.
 */
BAT *
BATmark(BAT *b, oid oid_base)
{
	BAT *bn;

	BATcheck(b, "BATmark");
	bn = VIEWhead(b);
	if (bn) {
		BATseqbase(BATmirror(bn), oid_base);
		if (oid_base == oid_nil)
			bn->T->nonil = 0;
		if (BAThrestricted(b) != BAT_READ) {
			BAT *v = bn;

			bn = BATcopy(v, v->htype, v->ttype, TRUE);
			BBPreclaim(v);
		}
	}
	return bn;
}

static void
BATsetprop_wrd(BAT *b, int idx, wrd val)
{
	BATsetprop(b, idx, TYPE_wrd, &val);
}

#define BUNnumber(bx,hx,tx)	bunfastins_nocheck(bx, r, hx, (ptr)&i, Hsize(bx), Tsize(bx)); r++; i++;
BAT *
BATnumber(BAT *b)
{
/* 64bit: BATnumber should return a [any,wrd] bat instead of [any,int] */
	int i = 0;
	BAT *bn;
	BUN r;

	BATcheck(b, "BATnumber");
	/* assert(BATcount(b) <= MAXINT); */
	bn = BATnew(b->htype, TYPE_int, BATcount(b));
	if (bn == NULL)
		return NULL;
	r = BUNfirst(bn);
	updateloop(bn, b, BUNnumber);
	ALIGNsetH(bn, b);
	BATsetprop_wrd(bn, GDK_AGGR_CARD, i);	/* 64bit: no (wrd) cast to remind us */
	bn->hsorted = BAThordered(b);
	bn->tsorted = 1;
	bn->hrevsorted = BAThrevordered(b);
	bn->trevsorted = BATcount(bn) <= 1;
	bn->H->nonil = b->H->nonil;
	bn->T->nonil = 1;
	return bn;
      bunins_failed:
	BBPreclaim(bn);
	return NULL;
}

BAT *
BATgroup(BAT *b, int start, int incr, int grpsize)
{
/* 64bit: this should probably use wrd instead of int */
	BUN p, q, r;
	int ngroups = 1, i = 0;
	BAT *bn;
	BATiter bi = bat_iterator(b);

	BATcheck(b, "BATgroup");
	bn = BATnew(b->htype, TYPE_int, BATcount(b));
	if (bn == NULL)
		return NULL;
	r = BUNfirst(bn);

	ALIGNsetH(bn, b);

	BATloop(b, p, q) {
		bunfastins_nocheck(bn, r, BUNhead(bi, p), (ptr) &start, Hsize(bn), Tsize(bn));
		r++;
		if (i == grpsize - 1) {
			start += incr;
			i = 0;
			ngroups++;
		} else {
			i++;
		}
	}
	if (i == 0)
		ngroups--;
	BATsetprop_wrd(bn, GDK_AGGR_CARD, ngroups);
	bn->hsorted = BAThordered(b);
	bn->tsorted = 1;
	bn->hrevsorted = BAThrevordered(b);
	bn->trevsorted = BATcount(bn) <= 1;
	bn->H->nonil = b->H->nonil;
	bn->T->nonil = 1;
	return bn;
      bunins_failed:
	BBPreclaim(bn);
	return NULL;
}

#define mark_grp_init(BUNfnd)				\
	do {						\
		BUN w;					\
							\
		BUNfnd(w, gi, &v);			\
		if (w != BUN_NONE) {			\
			n = * (oid *) BUNtloc(gi, w);	\
		} else {				\
			n = oid_nil;			\
		}					\
	} while (0)

#define mark_grp_loop4(BUNhead, BUNtail, init_n)	\
	do {							\
		oid u = oid_nil;				\
		oid n = oid_nil;				\
								\
		bn->T->nil = 0;					\
		BATloop(b, p, q) {				\
			oid v = * (oid *) BUNtail(bi, p);	\
								\
			if (v != u) {				\
				init_n;				\
				u = v;				\
			} else if (n != oid_nil) {		\
				n++;				\
			}					\
			if (n == oid_nil)			\
				bn->T->nil =1;			\
			bunfastins_nocheck_inc(bn, r, BUNhead(bi, p), &n); \
		}						\
		bn->T->nonil = !bn->T->nil;			\
	} while (0)

#define mark_grp_loop3(BUNhead, BUNtail, BUNfnd)			\
	do {								\
		bn->T->nil = 0;						\
		BATloop(b, p, q) {					\
			oid n = oid_nil;				\
			BUN w;						\
			const void *v = BUNtail(bi, p);			\
			oid *m;						\
									\
			BUNfnd(w, gci, v);				\
			if (w != BUN_NONE && *(m = (oid*) BUNtloc(gci, w)) != oid_nil) { \
				n = (*m)++;				\
			} else						\
				bn->T->nil = 1;				\
			bunfastins_nocheck_inc(bn, r, BUNhead(bi, p), &n); \
		}							\
		bn->T->nonil = !bn->T->nil;				\
	} while (0)

#define mark_grp_loop2(BUNhead, BUNtail)				\
	do {								\
		if (gc) {						\
			BATiter gci = bat_iterator(gc);			\
									\
			if (BAThdense(gc)) {				\
				mark_grp_loop3(BUNhead,			\
					       BUNtail, BUNfndVOID);	\
			} else {					\
				mark_grp_loop3(BUNhead,			\
					       BUNtail, BUNfndOID);	\
			}						\
		} else {						\
			if (s) {					\
				mark_grp_loop4(BUNhead,			\
					       BUNtail, n = *s);	\
			} else {					\
				if (BAThdense(g)) {			\
					mark_grp_loop4(BUNhead,		\
						       BUNtail,		\
						       mark_grp_init(BUNfndVOID)); \
				} else {				\
					mark_grp_loop4(BUNhead,		\
						       BUNtail,		\
						       mark_grp_init(BUNfndOID)); \
				}					\
			}						\
		}							\
	} while (0)

BAT *
BATmark_grp(BAT *b, BAT *g, oid *s)
{
	BAT *bn, *gc = NULL;
	bit trivprop = FALSE;

	BATcheck(b, "BATmark_grp");
	BATcheck(g, "BATmark_grp");
	ERRORcheck(b->ttype != TYPE_void && b->ttype != TYPE_oid,
		   "BATmark_grp: tail of BAT b must be oid.\n");
	ERRORcheck(g->htype != TYPE_void && g->htype != TYPE_oid,
		   "BATmark_grp: head of BAT g must be oid.\n");
	ERRORcheck(b->ttype == TYPE_void && b->tseqbase == oid_nil,
		   "BATmark_grp: tail of BAT b must not be nil.\n");
	ERRORcheck(g->htype == TYPE_void && g->hseqbase == oid_nil,
		   "BATmark_grp: head of BAT g must not be nil.\n");
	ERRORcheck(s && *s == oid_nil,
		   "BATmark_grp: base oid s must not be nil.\n");
	ERRORcheck(!s && g->ttype != TYPE_oid,
		   "BATmark_grp: tail of BAT g must be oid.\n");

	if (BATcount(b) == 0 || BATcount(g) == 1) {
		if (s) {
			return BATmark(b, *s);
		} else {
			BATiter gi = bat_iterator(g);
			return BATmark(b, *(oid *) BUNtloc(gi, BUNfirst(g)));
		}
	}

	if (!BATtordered(b)) {
		if (s) {
			BUN p, q, r;

			if (BAThdense(g)) {
				gc = BATnew(TYPE_void, TYPE_oid, BATcount(g));
				if (gc == NULL)
					return NULL;
				r = BUNfirst(gc);
				BATloop(g, p, q) {
					bunfastins_nocheck_inc(gc, r, NULL, s);
				}
			} else {
				BATiter gi = bat_iterator(g);
				gc = BATnew(TYPE_oid, TYPE_oid, BATcount(g));
				if (gc == NULL)
					return NULL;
				r = BUNfirst(gc);
				BATloop(g, p, q) {
					bunfastins_nocheck_inc(gc, r, BUNhloc(gi, p), s);
				}
			}
			BATsetcount(gc, BATcount(g));
			gc->hdense = BAThdense(g);
			if (gc->hdense) {
				BATseqbase(gc, g->hseqbase);
				gc->hsorted = 1;
			} else {
				gc->hsorted = BAThordered(g);
			}
			gc->hrevsorted = BAThrevordered(g);
			BATkey(gc, (gc->hdense || g->hkey != FALSE));
			gc->H->nonil = g->H->nonil;
			gc->T->nonil = 1;
		} else {
			gc = BATcopy(g, g->htype, g->ttype, TRUE);
			if (gc == NULL)
				return NULL;
		}
	}
	bn = BATnew(b->htype, TYPE_oid, BATcount(b));
	if (bn == NULL) {
		if (gc)
			BBPreclaim(gc);
		return NULL;
	}

	{
		BUN p, q, r = BUNfirst(bn);
		BATiter bi = bat_iterator(b);
		BATiter gi = bat_iterator(g);

		if (b->hvarsized) {
			if (b->ttype == TYPE_void) {
				mark_grp_loop2(BUNhvar, BUNtvar);
			} else {
				mark_grp_loop2(BUNhvar, BUNtloc);
			}
		} else {
			if (b->ttype == TYPE_void) {
				mark_grp_loop2(BUNhloc, BUNtvar);
			} else {
				mark_grp_loop2(BUNhloc, BUNtloc);
			}
		}
		BATsetcount(bn, BATcount(b));
	}

	trivprop = (BATcount(bn) < 2);
	ALIGNsetH(bn, b);
	bn->hsorted = trivprop || BAThordered(b);
	bn->hrevsorted = trivprop || BAThrevordered(b);
	bn->hdense = BAThdense(b) || (trivprop && b->htype == TYPE_oid);
	if (bn->hdense) {
		BATseqbase(bn, b->hseqbase);
	}
	BATkey(bn, (bn->hdense || b->hkey != FALSE));
	bn->tsorted = trivprop;
	bn->trevsorted = trivprop;
	bn->tdense = trivprop;
	if (BATtdense(bn)) {
		BATiter bni = bat_iterator(bn);
		BATseqbase(BATmirror(bn), *(oid *) BUNtloc(bni, BUNfirst(bn)));
	}
	BATkey(BATmirror(bn), trivprop);
	bn->H->nonil = b->H->nonil;

	if (gc)
		BBPreclaim(gc);
	return bn;
      bunins_failed:
	if (gc)
		BBPreclaim(gc);
	BBPreclaim(bn);
	return NULL;
}


BAT *
BATconst(BAT *b, int tailtype, const void *v)
{
	BAT *bn;
	void *p;
	BUN i, n;

	BATcheck(b, "BATconst");
	if (v == NULL)
		return NULL;
	n = BATcount(b);
	bn = BATnew(TYPE_void, tailtype, n);
	if (bn == NULL)
		return NULL;
	p = Tloc(bn, bn->U->first);
	switch (ATOMstorage(tailtype)) {
	case TYPE_void:
		v = &oid_nil;
		BATseqbase(BATmirror(bn), oid_nil);
		bn->T->nil = n >= 1;
		break;
	case TYPE_bte:
		for (i = 0; i < n; i++)
			((bte *) p)[i] = * (bte *) v;
		bn->T->nil = n >= 1 && * (bte *) v == bte_nil;
		break;
	case TYPE_sht:
		for (i = 0; i < n; i++)
			((sht *) p)[i] = * (sht *) v;
		bn->T->nil = n >= 1 && * (sht *) v == sht_nil;
		break;
	case TYPE_int:
	case TYPE_flt:
		assert(sizeof(int) == sizeof(flt));
		for (i = 0; i < n; i++)
			((int *) p)[i] = * (int *) v;
		bn->T->nil = n >= 1 && (ATOMstorage(tailtype) == TYPE_int ? * (int *) v == int_nil : * (flt *) v == flt_nil);
		break;
	case TYPE_lng:
	case TYPE_dbl:
		assert(sizeof(lng) == sizeof(dbl));
		for (i = 0; i < n; i++)
			((lng *) p)[i] = * (lng *) v;
		bn->T->nil = n >= 1 && (ATOMstorage(tailtype) == TYPE_lng ? * (lng *) v == lng_nil : * (dbl *) v == dbl_nil);
		break;
	default:
		bn->T->nil = n >= 1 && ATOMcmp(tailtype, v, ATOMnilptr(tailtype)) == 0;
		for (i = BUNfirst(bn), n += i; i < n; i++)
			tfastins_nocheck(bn, i, v, Tsize(bn));
		break;
	}
	BATsetcount(bn, BATcount(b));
	bn->tsorted = 1;
	bn->trevsorted = 1;
	bn->T->nonil = !bn->T->nil;
	bn->T->key = BATcount(bn) <= 1;
	if (b->H->type != bn->H->type) {
		BAT *bnn = VIEWcreate(b, bn);
		BBPunfix(bn->batCacheid);
		bn = bnn;
	} else {
		BATseqbase(bn, b->hseqbase);
	}

	return bn;

  bunins_failed:
	BBPreclaim(bn);
	return NULL;
}

/*
 * @+ BAT Aggregates
 *
 * We retain the size() and card() aggregate results in the column
 * descriptor.  We would like to have such functionality in an
 * extensible way for many aggregates, for DD (1) we do not want to
 * change the binary BAT format on disk and (2) aggr and size are the
 * most relevant aggregates.
 *
 * It is all hacked into the aggr[3] records; three adjacent integers
 * that were left over in the column record. We refer to these as if
 * it where an int aggr[3] array.  The below routines set and retrieve
 * the aggregate values from the tail of the BAT, as many
 * aggregate-manipulating BAT functions work on tail.
 *
 * The rules are as follows: aggr[0] contains the alignment ID of the
 * column (if set i.e. nonzero).  Hence, if this value is nonzero and
 * equal to b->talign, the precomputed aggregate values in
 * aggr[GDK_AGGR_SIZE] and aggr[GDK_AGGR_CARD] hold. However, only one
 * of them may be set at the time. This is encoded by the value
 * int_nil, which cannot occur in these two aggregates.
 *
 * This was now extended to record the property whether we know there
 * is a nil value present by mis-using the highest bits of both
 * GDK_AGGR_SIZE and GDK_AGGR_CARD.
 */
#define GDK_NIL_BIT 0x80000000	/* (1 << 31) */

void
PROPdestroy(PROPrec *p)
{
	PROPrec *n;

	while (p) {
		n = p->next;
		if (p->v.vtype == TYPE_str)
			GDKfree(p->v.val.sval);
		GDKfree(p);
		p = n;
	}
}

PROPrec *
BATgetprop(BAT *b, int idx)
{
	PROPrec *p = b->T->props;

	while (p) {
		if (p->id == idx)
			return p;
		p = p->next;
	}
	return NULL;
}

void
BATsetprop(BAT *b, int idx, int type, void *v)
{
	ValRecord vr;
	PROPrec *p = BATgetprop(b, idx);

	if (!p && (p = (PROPrec *) GDKmalloc(sizeof(PROPrec))) != NULL) {
		p->id = idx;
		p->next = b->T->props;
		p->v.vtype = 0;
		b->T->props = p;
	}
	if (p) {
		VALset(&vr, type, v);
		VALcopy(&p->v, &vr);
		b->batDirtydesc = TRUE;
	}
}

void
BATpropagate(BAT *dst, BAT *src, int idx)
{
	PROPrec *p = BATgetprop(src, idx);

	if (p)
		BATsetprop(dst, idx, p->v.vtype, VALget(&p->v));
}


/*
 * The BAThistogram function calculates the frequency distribution of
 * the tail values in its operand bat. Notice, that updates on the
 * result do not affect the delta administration.
 * Construction of a histogram over a string (or complex object) can
 * be sped up using the reference information in the BUN and bulk
 * copying the heap.
 *
 * There are separate versions for each type, and for each a hash- and
 * a merge-algorithms.
 */
#define histoloop_sorted(BUNtail, cmpfnc, TYPE)			\
	do {							\
		const void *prev = BUNtail(bi, BUNfirst(b));	\
								\
		BATloop(b, p, q) {				\
			v = BUNtail(bi, p);			\
			if (cmpfnc(v, prev, TYPE) == 0) {	\
				yy++;				\
			} else {				\
				bunfastins(bn, prev, &yy);	\
				yy = 1;				\
			}					\
			prev = v;				\
		}						\
		bunfastins(bn, prev, &yy);			\
	} while (0)

#define histoloop_merge(BUNtail, HASHloop)				\
	do {								\
		BATiter bni = bat_iterator(bn);				\
									\
		BATloop(b, p, q) {					\
			v = BUNtail(bi, p);				\
			if (BATprepareHash(bn))				\
				goto bunins_failed;			\
			HASHloop(bni, bn->H->hash, r, v)		\
				break;					\
			if (r == BUN_NONE) {				\
				/* not found */				\
				if (BUNins(bn, v, &yy, FALSE) == NULL)	\
					goto bunins_failed;		\
				r = BUNlast(bn) - 1;			\
			}						\
			(* (int *) BUNtloc(bni, r))++;			\
		}							\
		HASHdestroy(bn);					\
	} while (0)

BAT *
BAThistogram(BAT *b)
{
	BAT *bn;
	BUN r;
	int yy = 0, tt = 0;
	BUN p, q;
	BATiter bi = bat_iterator(b);
	int tricky;
	const void *v;

	BATcheck(b, "BAThistogram");

	if (b->talign == 0) {
		b->talign = OIDnew(1);
	}

	if (b->tkey || BATcount(b) <= 1) {
		yy = 1;
		return BATconst(BATmirror(b), TYPE_int, &yy);
	}

	tricky = (b->ttype == TYPE_str && strElimDoubles(b->T->vheap));
	bn = BATnew(tricky ? (b->T->width == 1 ? TYPE_bte : (b->T->width == 2 ? TYPE_sht : (b->T->width == 4 ? TYPE_int : TYPE_lng))) : b->ttype, TYPE_int, 200);
	if (bn == NULL)
		return bn;

	if (BATtordered(b) || BATtrevordered(b)) {
		/* the important information here is that equal values
		 * are consecutive; we don't care about sortedness as
		 * such */
		switch (ATOMstorage(bn->htype)) {
		case TYPE_bte:
			histoloop_sorted(BUNtloc, simple_CMP, bte);
			break;
		case TYPE_sht:
			histoloop_sorted(BUNtloc, simple_CMP, sht);
			break;
		case TYPE_int:
		case TYPE_flt:
			histoloop_sorted(BUNtloc, simple_CMP, int);
			break;
		case TYPE_lng:
		case TYPE_dbl:
			histoloop_sorted(BUNtloc, simple_CMP, lng);
			break;
		default:
			tt = bn->htype;
			if (bn->hvarsized)
				histoloop_sorted(BUNtvar, atom_CMP, tt);
			else
				histoloop_sorted(BUNtloc, atom_CMP, tt);
			break;
		}
	} else {
		switch (ATOMstorage(bn->htype)) {
		case TYPE_bte:
			histoloop_merge(BUNtloc, HASHloop_bte);
			break;
		case TYPE_sht:
			histoloop_merge(BUNtloc, HASHloop_sht);
			break;
		case TYPE_int:
		case TYPE_flt:
			histoloop_merge(BUNtloc, HASHloop_int);
			break;
		case TYPE_lng:
		case TYPE_dbl:
			histoloop_merge(BUNtloc, HASHloop_lng);
			break;
		default:
			if (bn->hvarsized)
				histoloop_merge(BUNtvar, HASHloopvar);
			else
				histoloop_merge(BUNtloc, HASHlooploc);
			break;
		}
	}

	/*
	 * And now correct the interpretation of the values
	 * encountered by bulk copying the heap as well
	 */
	if (tricky) {
		bn->H->vheap = (Heap *) GDKzalloc(sizeof(Heap));
		if (bn->H->vheap == NULL)
			goto bunins_failed;
		bn->H->vheap->parentid = bn->batCacheid;
		if (b->T->vheap->filename) {
			char *nme = BBP_physical(bn->batCacheid);

			bn->H->vheap->filename = (str) GDKmalloc(strlen(nme) + 12);
			if (bn->H->vheap->filename == NULL)
				goto bunins_failed;
			GDKfilepath(bn->H->vheap->filename, NULL, nme, "hheap");
		}
		if (HEAPcopy(bn->H->vheap, b->T->vheap) < 0)
			goto bunins_failed;
		bn->htype = b->ttype;
		bn->hvarsized = 1;
		bn->H->width = b->T->width;
		bn->H->shift = b->T->shift;
	}

	bn->hsorted = BATcount(bn) <= 1 || BATtordered(b);
	bn->hrevsorted = BATcount(bn) <= 1 || BATtrevordered(b);
	bn->tsorted = BATcount(bn) <= 1;
	bn->trevsorted = BATcount(bn) <= 1;
	bn->halign = NOID_AGGR(b->talign);
	if (BATcount(bn) == BATcount(b))
		ALIGNsetH(bn, BATmirror(b));
	BATkey(bn, TRUE);
	BATkey(BATmirror(bn), BATcount(bn) < 2);
	bn->H->nonil = b->T->nonil;
	if (b->ttype == TYPE_bit) {
		BATiter bni = bat_iterator(bn);
		bit trueval = TRUE;
		BUN p = BUNfnd(bn, &trueval);

		BATsetprop_wrd(b, GDK_AGGR_SIZE, (p != BUN_NONE) ? *(int *) BUNtloc(bni, p) : 0);
	}
	BATsetprop_wrd(b, GDK_AGGR_CARD, (wrd) BATcount(bn));
	return bn;
      bunins_failed:
	BBPreclaim(bn);
	return NULL;
}

/*
 * The BATcount_no_nil function counts all BUN in a BAT that have a
 * non-nil tail value.
 */
BUN
BATcount_no_nil(BAT *b)
{
	BUN cnt = 0;
	BUN i, n;
	const void *p, *nil;
	const char *base;
	int (*cmp)(const void *, const void *);

	BATcheck(b, "BATcnt");
	n = BATcount(b);
	if (b->T->nonil)
		return n;
	p = Tloc(b, b->U->first);
	switch (ATOMstorage(b->ttype)) {
	case TYPE_void:
		cnt = b->tseqbase == oid_nil ? 0 : n;
		break;
	case TYPE_bte:
		for (i = 0; i < n; i++)
			cnt += ((const bte *) p)[i] != bte_nil;
		break;
	case TYPE_sht:
		for (i = 0; i < n; i++)
			cnt += ((const sht *) p)[i] != sht_nil;
		break;
	case TYPE_int:
		for (i = 0; i < n; i++)
			cnt += ((const int *) p)[i] != int_nil;
		break;
	case TYPE_lng:
		for (i = 0; i < n; i++)
			cnt += ((const lng *) p)[i] != lng_nil;
		break;
	case TYPE_flt:
		for (i = 0; i < n; i++)
			cnt += ((const flt *) p)[i] != flt_nil;
		break;
	case TYPE_dbl:
		for (i = 0; i < n; i++)
			cnt += ((const dbl *) p)[i] != dbl_nil;
		break;
	case TYPE_str:
		base = b->T->vheap->base;
		switch (b->T->width) {
		case 1:
			for (i = 0; i < n; i++)
				cnt += base[((var_t) ((const unsigned char *) p)[i] + GDK_VAROFFSET) << GDK_VARSHIFT] != '\200';
			break;
		case 2:
			for (i = 0; i < n; i++)
				cnt += base[((var_t) ((const unsigned short *) p)[i] + GDK_VAROFFSET) << GDK_VARSHIFT] != '\200';
			break;
#if SIZEOF_VAR_T != SIZEOF_INT
		case 4:
			for (i = 0; i < n; i++)
				cnt += base[(var_t) ((const unsigned int *) p)[i] << GDK_VARSHIFT] != '\200';
			break;
#endif
		default:
			for (i = 0; i < n; i++)
				cnt += base[((const var_t *) p)[i] << GDK_VARSHIFT] != '\200';
			break;
		}
		break;
	default:
		nil = ATOMnilptr(b->ttype);
		cmp = BATatoms[b->ttype].atomCmp;
		if (b->tvarsized) {
			base = b->T->vheap->base;
			for (i = 0; i < n; i++)
				cnt += (*cmp)(nil, base + ((const var_t *) p)[i]) != 0;
		} else {
			for (i = BUNfirst(b), n += i; i < n; i++)
				cnt += (*cmp)(Tloc(b, i), nil) != 0;
		}
		break;
	}
	if (cnt == BATcount(b)) {
		/* we learned something */
		b->T->nonil = 1;
		assert(b->T->nil == 0);
		b->T->nil = 0;
	}
	return cnt;
}
