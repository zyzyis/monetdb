/* -*- c-basic-offset:4; c-indentation-style:"k&r"; indent-tabs-mode:nil -*- */

/*******************************************
* staircasejoin.c : staircase join operators
*
* TODO: Test whether the copy phase can better be replaced by memcpy.
*/

/* Debug flags */
const int PF_DEBUG_RESULT_BAT = 1;  /* check size of result BAT */

/* Parameters */
const int PF_SOFTWARE_PREFETCH = 128;

#define prefetch_loc(addr) \
  __asm__ __volatile__ ("prefetchnta %0" \
                        : \
                        : \
                        "m" (*(((char*)(((unsigned int)(addr))&~0x7f)))))


/*******************************************************************/

#define PF_INCL_BOUNDARY
	#undef PF_MEMCPY
	@:ilcaspref(_incl)
	@:scjfunc_br_void(_incl)
	#define PF_MEMCPY
	@:ilcaspref(_incl_memcpy)
	@:scjfunc_br_void(_incl_memcpy)
#undef PF_INCL_BOUNDARY
	#undef PF_MEMCPY
	@:ilcaspref(_excl)
	@:scjfunc_br_void(_excl)
	#define PF_MEMCPY
	@:ilcaspref(_excl_memcpy)
	@:scjfunc_br_void(_excl_memcpy)


@= ilcaspref
inline void PFinnerloop_copyandscan_prefetching@1(oid copy_limit,
	oid *doc_curr_x_ptr, oid *doc_yval, oid context_prev_y,
	oid **result_curr_ptr)
{
    /* Copy-and-scan algorithm with prefetching. We employ Duff's Device
       with 32 cases to increase loop unrolling. */

    int doc_curr_x = *doc_curr_x_ptr;
    oid* result_curr = *result_curr_ptr;

    /* copy phase */
    int est = MIN(copy_limit,context_prev_y);

#ifdef PF_MEMCPY
    int n = est - doc_curr_x + 1;
    memcpy(result_curr, &doc_yval[doc_curr_x], n);
    doc_curr_x += n;
    result_curr += n;

#else
    int n = (est-doc_curr_x+31)/32;
    if (n!=0) switch ((est-doc_curr_x)%32)
    {
        case 0: do {
                    prefetch_loc(&doc_yval[doc_curr_x] + PF_SOFTWARE_PREFETCH);
                    *result_curr++=doc_curr_x++;
                    case 31: *result_curr ++= doc_curr_x++;
                    case 30: *result_curr ++= doc_curr_x++;
                    case 29: *result_curr ++= doc_curr_x++;
                    case 28: *result_curr ++= doc_curr_x++;
                    case 27: *result_curr ++= doc_curr_x++;
                    case 26: *result_curr ++= doc_curr_x++;
                    case 25: *result_curr ++= doc_curr_x++;
                    case 24: *result_curr ++= doc_curr_x++;
                    case 23: *result_curr ++= doc_curr_x++;
                    case 22: *result_curr ++= doc_curr_x++;
                    case 21: *result_curr ++= doc_curr_x++;
                    case 20: *result_curr ++= doc_curr_x++;
                    case 19: *result_curr ++= doc_curr_x++;
                    case 18: *result_curr ++= doc_curr_x++;
                    case 17: *result_curr ++= doc_curr_x++;
                    case 16: *result_curr ++= doc_curr_x++;
                    case 15: *result_curr ++= doc_curr_x++;
                    case 14: *result_curr ++= doc_curr_x++;
                    case 13: *result_curr ++= doc_curr_x++;
                    case 12: *result_curr ++= doc_curr_x++;
                    case 11: *result_curr ++= doc_curr_x++;
                    case 10: *result_curr ++= doc_curr_x++;
                    case  9: *result_curr ++= doc_curr_x++;
                    case  8: *result_curr ++= doc_curr_x++;
                    case  7: *result_curr ++= doc_curr_x++;
                    case  6: *result_curr ++= doc_curr_x++;
                    case  5: *result_curr ++= doc_curr_x++;
                    case  4: *result_curr ++= doc_curr_x++;
                    case  3: *result_curr ++= doc_curr_x++;
                    case  2: *result_curr ++= doc_curr_x++;
                    case  1: *result_curr ++= doc_curr_x++;
                } while(--n>0);
    }
#endif

    /* scan phase */
    while(doc_curr_x < copy_limit)
    {
#ifdef PF_INCL_BOUNDARY
        if (doc_yval[doc_curr_x] <= context_prev_y)
#else
            if (doc_yval[doc_curr_x] < context_prev_y)
#endif
                *result_curr++=doc_curr_x++;
            else
                break;
    }

    *doc_curr_x_ptr=doc_curr_x;
    *result_curr_ptr=result_curr;
}


@= scjfunc_br_void
int PFstaircasejoin_br_void@1(BAT **res, BAT *doc, BAT *context,
	                      int *height, int *upperbound)
{
    char *name = "PFstaircasejoin_br_void@1";
    BAT *result;
    oid *result_curr, *result_last;
    BUN context_prev, context_curr, context_last;
    oid context_prev_x, context_prev_y;
    oid doc_curr_x;
    oid doc_limit = doc->hseqbase + BATcount(doc) - 1;
    oid *doc_yval = ((oid*) BUNfirst(doc)) - doc->hseqbase;
    oid copy_limit;
    int xx;

    /* doc should be head-dense and context head-ordered */
    if (!BAThdense(doc))
    {
        GDKerror("%s: %s must be dense on head.\n",
                 name, BBP_logical(doc->batCacheid));
        return GDK_FAIL;
    }
    if (!(BAThordered(context)&1)) /* GDK_SORTED==65 ?!? */
    {
        GDKerror("%s: %s must be ordered on head.\n",
                 name, BBP_logical(context->batCacheid));
        return GDK_FAIL;
    }

    /* Special case: context is empty or doc is empty
    */
    if (BATcount(context)==0 || BATcount(doc)==0)
    {
        *res = BATnew(TYPE_oid, TYPE_void, 0);
        return GDK_SUCCEED;
    }

    /* Allocate result BAT according to upperbound. */
    result = *res = BATnew(TYPE_oid, TYPE_void, *upperbound);
    if (result == NULL)
    {
        GDKerror("%s: could not allocate a result BAT of size %d.\n",
                 name, *upperbound);
        return GDK_FAIL;
    }
    result_curr = (oid*) BUNfirst(result);
    result_last = (oid*) (result->batBuns->base + result->batBuns->size);

    /* Determine first node in context */
    context_prev = BUNfirst(context);
    context_prev_x = *(oid*) BUNhead(context, context_prev);
    context_prev_y = doc_yval[context_prev_x];

    /* Start scanning doc at x-coord of first context node */
    doc_curr_x = MAX(MIN(doc_limit+1,context_prev_x),doc->hseqbase);

    /* The "for" below is the same as
     * BATloopFast(context,context_curr,context_last,xx)
     * only that it starts at the second bun.
     */
    for(context_last = BUNlast(context)
            , xx = BUNsize(context)
            , context_curr = BUNfirst(context) + xx
            ; context_curr < context_last
            ; context_curr += xx)
    {
        oid context_curr_x = *(oid*) BUNhead(context, context_curr);
        oid context_curr_y = doc_yval[context_curr_x];

        /* Pruning */
        if (context_curr_y > context_prev_y)
        {

            /* In case of excluding boundary, skip boundary */
#ifndef PF_INCL_BOUNDARY
            if (context_prev_x==doc_curr_x) doc_curr_x++;
#endif

            copy_limit = MIN(MIN(MIN(doc_curr_x + result_last - result_curr,
                                     context_curr_x),
                                 doc_limit+1),
                             context_prev_y + *height);

            PFinnerloop_copyandscan_prefetching@1(copy_limit, &doc_curr_x,
                    doc_yval, context_prev_y, &result_curr);

            if (PF_DEBUG_RESULT_BAT)
            {
                if (result_curr >= result_last && doc_curr_x < context_curr_x)
                {
                    BATfree(result);
                    result = *res = NULL;
                    GDKerror("%s: upperbound %d not large enough "
                             "for result BAT.", name, *upperbound);
                    return GDK_FAIL;
                }
            }

            /* Next context node */
            context_prev_x = context_curr_x;
            context_prev_y = context_curr_y;
            doc_curr_x = MAX(MIN(doc_limit+1,context_prev_x),doc->hseqbase);
        }
    }

    /* Handle remaining document nodes after rightmost context node */

#ifndef PF_INCL_BOUNDARY
    if (context_prev_x==doc_curr_x) doc_curr_x++;
#endif

    copy_limit = MIN(MIN(doc_curr_x + result_last - result_curr,
                doc_limit + 1),
            context_prev_y + *height);
    PFinnerloop_copyandscan_prefetching@1(copy_limit, &doc_curr_x,
            doc_yval, context_prev_y, &result_curr);

    if (PF_DEBUG_RESULT_BAT)
    {
        if (result_curr >= result_last && doc_curr_x < doc_limit + 1)
        {
            BATfree (result);
            result = *res = NULL;
            GDKerror("%s: upperbound %d not large enough for result BAT.",
                     name, *upperbound);
            return GDK_FAIL;
        }
    }

    /* Fix result BAT properties */
    result->batBuns->free = ((BUN) result_curr) - result->batBuns->base;
    result->hsorted = GDK_SORTED;
    BATkey (result, TRUE);
    BATseqbase (BATmirror(result), oid_nil);
    return GDK_SUCCEED;
}

/* vim:set shiftwidth=4 expandtab: */
