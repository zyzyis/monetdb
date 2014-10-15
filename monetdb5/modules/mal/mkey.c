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
 * (c) Peter Boncz, Stefan Manegold, Niels Nes
 *
 * new functionality for the low-resource-consumption. It will
 * first one by one create a hash value out of the multiple attributes.
 * This hash value is computed by xoring and rotating individual hash
 * values together. We create a hash and rotate command to do this.
 */
#include "monetdb_config.h"
#include "mkey.h"

#define MKEYHASH_bte(valp)	((wrd) *(bte*)(valp))
#define MKEYHASH_sht(valp)	((wrd) *(sht*)(valp))
#define MKEYHASH_int(valp)	((wrd) *(int*)(valp))
#if SIZEOF_WRD == SIZEOF_LNG
#define MKEYHASH_lng(valp)	((wrd) *(lng*)(valp))
#else
#define MKEYHASH_lng(valp)	(((wrd*)(valp))[0] ^ ((wrd*)(valp))[1])
#endif

/* TODO: nil handling. however; we do not want to lose time in bulk_rotate_xor_hash with that */
str
MKEYrotate(wrd *res, const wrd *val, const int *n)
{
	*res = GDK_ROTATE(*val, *n, (sizeof(wrd)*8) - *n, (((wrd)1) << *n) - 1);
	return MAL_SUCCEED;
}

str
MKEYhash(Client cntxt, MalBlkPtr mb, MalStkPtr stk, InstrPtr p)
{
	wrd *res;
	ptr val;
	int tpe = getArgType(mb,p,1);

	(void) cntxt;
	res= getArgReference_wrd(stk,p,0);
	val= getArgReference(stk,p,1);
	switch (ATOMstorage(tpe)) {
	case TYPE_bte:
		*res = MKEYHASH_bte(val);
		break;
	case TYPE_sht:
		*res = MKEYHASH_sht(val);
		break;
	case TYPE_int:
	case TYPE_flt:
		*res = MKEYHASH_int(val);
		break;
	case TYPE_lng:
	case TYPE_dbl:
		*res = MKEYHASH_lng(val);
		break;
	default:
		if (ATOMextern(tpe))
			*res = (*BATatoms[tpe].atomHash)(*(ptr*)val);
		else
			*res = (*BATatoms[tpe].atomHash)(val);
		break;
	}
	return MAL_SUCCEED;
}

str
MKEYbathash(bat *res, const bat *bid)
{
	BAT *b, *dst;
	wrd *r;
	BUN n;

	if ((b = BATdescriptor(*bid)) == NULL)
		throw(SQL, "mkey.bathash", RUNTIME_OBJECT_MISSING);

	assert(BAThvoid(b) || BAThrestricted(b));

	n = BATcount(b);
	dst = BATnew(TYPE_void, TYPE_wrd, n, TRANSIENT);
	if (dst == NULL) {
		BBPreleaseref(b->batCacheid);
		throw(SQL, "mkey.bathash", MAL_MALLOC_FAIL);
	}
	BATseqbase(dst, b->hseqbase);
	BATsetcount(dst, n);

	r = (wrd *) Tloc(dst, BUNfirst(dst));

	switch (ATOMstorage(b->ttype)) {
	case TYPE_void: {
		oid o = b->tseqbase;
		if (o == oid_nil)
			while (n-- > 0)
				*r++ = wrd_nil;
		else
			while (n-- > 0)
				*r++ = (wrd) o++;
		break;
	}
	case TYPE_bte: {
		bte *v = (bte *) Tloc(b, BUNfirst(b));
		while (n-- > 0) {
			*r++ = MKEYHASH_bte(v);
			v++;
		}
		break;
	}
	case TYPE_sht: {
		sht *v = (sht *) Tloc(b, BUNfirst(b));
		while (n-- > 0) {
			*r++ = MKEYHASH_sht(v);
			v++;
		}
		break;
	}
	case TYPE_int:
	case TYPE_flt: {
		int *v = (int *) Tloc(b, BUNfirst(b));
		while (n-- > 0) {
			*r++ = MKEYHASH_int(v);
			v++;
		}
		break;
	}
	case TYPE_lng:
	case TYPE_dbl: {
		lng *v = (lng *) Tloc(b, BUNfirst(b));
		while (n-- > 0) {
			*r++ = MKEYHASH_lng(v);
			v++;
		}
		break;
	}
	default: {
		BATiter bi = bat_iterator(b);
		BUN (*hash)(const void *) = BATatoms[b->ttype].atomHash;
		int (*cmp)(const void *, const void *) = BATatoms[b->ttype].atomCmp;
		void *nil = BATatoms[b->ttype].atomNull;
		BUN i;
		const void *v;

		BATloop(b, i, n) {
			v = BUNtail(bi, i);
			if ((*cmp)(v, nil) == 0)
				*r++ = wrd_nil;
			else
				*r++ = (wrd) (*hash)(v);
		}
		break;
	}
	}

	if (dst->batCount <= 1) {
		BATkey(BATmirror(dst), 1);
		dst->tsorted = dst->trevsorted = 1;
	} else {
		BATkey(BATmirror(dst), 0);
		dst->tsorted = dst->trevsorted = 0;
	}
	dst->T->nonil = b->T->nonil;
	dst->T->nil = b->T->nil;

	if (dst->htype != b->htype) {
		BAT *x = VIEWcreate(b, dst);
		BBPreleaseref(dst->batCacheid);
		dst = x;
	}
	BBPkeepref(*res = dst->batCacheid);
	BBPreleaseref(b->batCacheid);
	return MAL_SUCCEED;
}

str
MKEYrotate_xor_hash(Client cntxt, MalBlkPtr mb, MalStkPtr stk, InstrPtr p)
{
	wrd *dst = getArgReference_wrd(stk, p, 0);
	wrd h = *getArgReference_wrd(stk, p, 1);
	int lbit = *getArgReference_int(stk, p, 2);
	int rbit = (int) sizeof(wrd) * 8 - lbit;
	int tpe = getArgType(mb, p, 3);
	ptr *pval = getArgReference(stk, p, 3);
	wrd val;
	wrd mask = ((wrd) 1 << lbit) - 1;

	(void) cntxt;
	switch (ATOMstorage(tpe)) {
	case TYPE_bte:
		val = MKEYHASH_bte(pval);
		break;
	case TYPE_sht:
		val = MKEYHASH_sht(pval);
		break;
	case TYPE_int:
	case TYPE_flt:
		val = MKEYHASH_int(pval);
		break;
	case TYPE_lng:
	case TYPE_dbl:
		val = MKEYHASH_lng(pval);
		break;
	default:
		if (ATOMextern(tpe))
			val = (*BATatoms[tpe].atomHash)(*(ptr*)pval);
		else
			val = (*BATatoms[tpe].atomHash)(pval);
		break;
	}
	*dst = GDK_ROTATE(h, lbit, rbit, mask) ^ val;
	return MAL_SUCCEED;
}

str
MKEYbulk_rotate_xor_hash(bat *res, const bat *hid, const int *nbits, const bat *bid)
{
	BAT *hb, *b, *bn;
	int lbit = *nbits;
	int rbit = (int) sizeof(wrd) * 8 - lbit;
	wrd mask = ((wrd) 1 << lbit) - 1;
	wrd *r;
	const wrd *h;
	BUN n;

	if ((hb = BATdescriptor(*hid)) == NULL)
        throw(MAL, "mkey.rotate_xor_hash", RUNTIME_OBJECT_MISSING);

	if ((b = BATdescriptor(*bid)) == NULL) {
		BBPreleaseref(hb->batCacheid);
        throw(MAL, "mkey.rotate_xor_hash",  RUNTIME_OBJECT_MISSING);
    }

	if (!ALIGNsynced(hb, b) && (BATcount(b) || BATcount(hb))) {
		BBPreleaseref(hb->batCacheid);
		BBPreleaseref(b->batCacheid);
		throw(MAL, "mkey.rotate_xor_hash",
			  OPERATION_FAILED ": input bats are not aligned");
	}

	n = BATcount(b);

	bn = BATnew(TYPE_void, TYPE_wrd, n, TRANSIENT);
	if (bn == NULL) {
		BBPreleaseref(hb->batCacheid);
		BBPreleaseref(b->batCacheid);
		throw(MAL, "mkey.rotate_xor_hash", MAL_MALLOC_FAIL);
	}
	BATseqbase(bn, b->hseqbase);
	BATsetcount(bn, n);

	r = (wrd *) Tloc(bn, BUNfirst(bn));
	h = (const wrd *) Tloc(hb, BUNfirst(hb));

	switch (ATOMstorage(b->ttype)) {
	case TYPE_bte: {
		bte *v = (bte *) Tloc(b, BUNfirst(b));
		while (n-- > 0) {
			*r++ = GDK_ROTATE(*h, lbit, rbit, mask) ^ MKEYHASH_bte(v);
			v++;
			h++;
		}
		break;
	}
	case TYPE_sht: {
		sht *v = (sht *) Tloc(b, BUNfirst(b));
		while (n-- > 0) {
			*r++ = GDK_ROTATE(*h, lbit, rbit, mask) ^ MKEYHASH_sht(v);
			v++;
			h++;
		}
		break;
	}
	case TYPE_int:
	case TYPE_flt: {
		int *v = (int *) Tloc(b, BUNfirst(b));
		while (n-- > 0) {
			*r++ = GDK_ROTATE(*h, lbit, rbit, mask) ^ MKEYHASH_int(v);
			v++;
			h++;
		}
		break;
	}
	case TYPE_lng:
	case TYPE_dbl: {
		lng *v = (lng *) Tloc(b, BUNfirst(b));
		while (n-- > 0) {
			*r++ = GDK_ROTATE(*h, lbit, rbit, mask) ^ MKEYHASH_lng(v);
			v++;
			h++;
		}
		break;
	}
	case TYPE_str:
		if (b->T->vheap->hashash) {
			BATiter bi = bat_iterator(b);
			BUN i;
			BATloop(b, i, n) {
				str s = (str) BUNtvar(bi, i);
				*r++ = GDK_ROTATE(*h, lbit, rbit, mask) ^ (wrd) ((BUN *) s)[-1];
				h++;
			}
			break;
		}
		/* fall through */
	default: {
		BATiter bi = bat_iterator(b);
		BUN (*hash)(const void *) = BATatoms[b->ttype].atomHash;
		BUN i;

		BATloop(b, i, n) {
			*r++ = GDK_ROTATE(*h, lbit, rbit, mask) ^ (wrd) (*hash)(BUNtail(bi, i));
			h++;
		}
		break;
	}
	}
	if (bn->batCount <= 1) {
		BATkey(BATmirror(bn), 1);
		bn->tsorted = bn->trevsorted = 1;
	} else {
		BATkey(BATmirror(bn), 0);
		bn->tsorted = bn->trevsorted = 0;
	}
	bn->T->nonil = 1;
	bn->T->nil = 0;

	if (bn->htype != b->htype) {
		BAT *x = VIEWcreate(b, bn);
		BBPreleaseref(bn->batCacheid);
		bn = x;
	}
	BBPkeepref(*res = bn->batCacheid);
	BBPreleaseref(b->batCacheid);
	BBPreleaseref(hb->batCacheid);
	return MAL_SUCCEED;
}

str
MKEYbulkconst_rotate_xor_hash(Client cntxt, MalBlkPtr mb, MalStkPtr stk, InstrPtr p)
{
	bat *res = getArgReference_bat(stk, p, 0);
	bat *hid = getArgReference_bat(stk, p, 1);
	int lbit = *getArgReference_int(stk, p, 2);
	int tpe = getArgType(mb, p, 3);
	ptr *pval = getArgReference(stk, p, 3);
	BAT *hb, *bn;
	int rbit = (int) sizeof(wrd) * 8 - lbit;
	wrd mask = ((wrd) 1 << lbit) - 1;
	wrd *r;
	const wrd *h;
	wrd val;
	BUN n;

	(void) cntxt;

	if ((hb = BATdescriptor(*hid)) == NULL)
        throw(MAL, "mkey.rotate_xor_hash", RUNTIME_OBJECT_MISSING);

	n = BATcount(hb);

	bn = BATnew(TYPE_void, TYPE_wrd, n, TRANSIENT);
	if (bn == NULL) {
		BBPreleaseref(hb->batCacheid);
		throw(MAL, "mkey.rotate_xor_hash", MAL_MALLOC_FAIL);
	}
	BATseqbase(bn, hb->hseqbase);
	BATsetcount(bn, n);

	switch (ATOMstorage(tpe)) {
	case TYPE_bte:
		val = MKEYHASH_bte(pval);
		break;
	case TYPE_sht:
		val = MKEYHASH_sht(pval);
		break;
	case TYPE_int:
	case TYPE_flt:
		val = MKEYHASH_int(pval);
		break;
	case TYPE_lng:
	case TYPE_dbl:
		val = MKEYHASH_lng(pval);
		break;
	default:
		if (ATOMextern(tpe))
			val = (*BATatoms[tpe].atomHash)(*(ptr*)pval);
		else
			val = (*BATatoms[tpe].atomHash)(pval);
		break;
	}

	r = (wrd *) Tloc(bn, BUNfirst(bn));
	h = (const wrd *) Tloc(hb, BUNfirst(hb));

	while (n-- > 0) {
			*r++ = GDK_ROTATE(*h, lbit, rbit, mask) ^ val;
			h++;
	}

	if (bn->batCount <= 1) {
		BATkey(BATmirror(bn), 1);
		bn->tsorted = bn->trevsorted = 1;
	} else {
		BATkey(BATmirror(bn), 0);
		bn->tsorted = bn->trevsorted = 0;
	}
	bn->T->nonil = 1;
	bn->T->nil = 0;

	if (bn->htype != hb->htype) {
		BAT *x = VIEWcreate(hb, bn);
		BBPreleaseref(bn->batCacheid);
		bn = x;
	}
	BBPkeepref(*res = bn->batCacheid);
	BBPreleaseref(hb->batCacheid);
	return MAL_SUCCEED;
}

str
MKEYconstbulk_rotate_xor_hash(bat *res, const wrd *h, const int *nbits, const bat *bid)
{
	BAT *b, *bn;
	int lbit = *nbits;
	int rbit = (int) sizeof(wrd) * 8 - lbit;
	wrd mask = ((wrd) 1 << lbit) - 1;
	wrd *r;
	BUN n;

	if ((b = BATdescriptor(*bid)) == NULL)
        throw(MAL, "mkey.rotate_xor_hash",  RUNTIME_OBJECT_MISSING);

	n = BATcount(b);

	bn = BATnew(TYPE_void, TYPE_wrd, n, TRANSIENT);
	if (bn == NULL) {
		BBPreleaseref(b->batCacheid);
		throw(MAL, "mkey.rotate_xor_hash", MAL_MALLOC_FAIL);
	}
	BATseqbase(bn, b->hseqbase);
	BATsetcount(bn, n);

	r = (wrd *) Tloc(bn, BUNfirst(bn));

	switch (ATOMstorage(b->ttype)) {
	case TYPE_bte: {
		bte *v = (bte *) Tloc(b, BUNfirst(b));
		while (n-- > 0) {
			*r++ = GDK_ROTATE(*h, lbit, rbit, mask) ^ MKEYHASH_bte(v);
			v++;
		}
		break;
	}
	case TYPE_sht: {
		sht *v = (sht *) Tloc(b, BUNfirst(b));
		while (n-- > 0) {
			*r++ = GDK_ROTATE(*h, lbit, rbit, mask) ^ MKEYHASH_sht(v);
			v++;
		}
		break;
	}
	case TYPE_int:
	case TYPE_flt: {
		int *v = (int *) Tloc(b, BUNfirst(b));
		while (n-- > 0) {
			*r++ = GDK_ROTATE(*h, lbit, rbit, mask) ^ MKEYHASH_int(v);
			v++;
		}
		break;
	}
	case TYPE_lng:
	case TYPE_dbl: {
		lng *v = (lng *) Tloc(b, BUNfirst(b));
		while (n-- > 0) {
			*r++ = GDK_ROTATE(*h, lbit, rbit, mask) ^ MKEYHASH_lng(v);
			v++;
		}
		break;
	}
	case TYPE_str:
		if (b->T->vheap->hashash) {
			BATiter bi = bat_iterator(b);
			BUN i;
			BATloop(b, i, n) {
				str s = (str) BUNtvar(bi, i);
				*r++ = GDK_ROTATE(*h, lbit, rbit, mask) ^ (wrd) ((BUN *) s)[-1];
			}
			break;
		}
		/* fall through */
	default: {
		BATiter bi = bat_iterator(b);
		BUN (*hash)(const void *) = BATatoms[b->ttype].atomHash;
		BUN i;

		BATloop(b, i, n) {
			*r++ = GDK_ROTATE(*h, lbit, rbit, mask) ^ (wrd) (*hash)(BUNtail(bi, i));
		}
		break;
	}
	}
	if (bn->batCount <= 1) {
		BATkey(BATmirror(bn), 1);
		bn->tsorted = bn->trevsorted = 1;
	} else {
		BATkey(BATmirror(bn), 0);
		bn->tsorted = bn->trevsorted = 0;
	}
	bn->T->nonil = 1;
	bn->T->nil = 0;

	if (bn->htype != b->htype) {
		BAT *x = VIEWcreate(b, bn);
		BBPreleaseref(bn->batCacheid);
		bn = x;
	}
	BBPkeepref(*res = bn->batCacheid);
	BBPreleaseref(b->batCacheid);
	return MAL_SUCCEED;
}
