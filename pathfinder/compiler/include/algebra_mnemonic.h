/** atomic natural number construction */
#define lit_nat(n)      PFalg_lit_nat (n)

/** atomic integer construction */
#define lit_int(i)      PFalg_lit_int (i)

/** atomic string construction */
#define lit_str(s)      PFalg_lit_str (s)

/** atomic float construction */
#define lit_dec(f)      PFalg_lit_dec (f)

/** atomic double construction */
#define lit_dbl(d)      PFalg_lit_dbl (d)

/** atomic boolean construction */
#define lit_bln(b)      PFalg_lit_bln (b)

/** tuple construction */
#define tuple(...)      PFalg_tuple (__VA_ARGS__)

/** (schema) attribute construction */
#define att(n,t)        PFalg_att ((n),(t))

/** attribute list construction */
#define attlist(...)    PFalg_attlist (__VA_ARGS__)

/** a sort specification list is just another attribute list */
#define sortby(...)     PFalg_attlist (__VA_ARGS__)

/** literal table construction */
#define lit_tbl(...)    PFalg_lit_tbl (__VA_ARGS__)

/** cartesian product */
#define cross(a,b)      PFalg_cross ((a),(b))

/** equi-join */
#define eqjoin(a,b,c,d) PFalg_eqjoin ((a),(b),(c),(d))

/** staircase join */
#define scjoin(a,b,c)   PFalg_scjoin ((a),(b),(c))

/** document table */
#define doc_tbl(a)      PFalg_doc_tbl((a))

/** disjoint union */
#define disjunion(a,b)  PFalg_disjunion ((a),(b))

/** difference */
#define difference(a,b) PFalg_difference ((a),(b))

/** projection operator */
#define project(...)    PFalg_project (__VA_ARGS__)

/** item in the projection list */
#define proj(a,b)       PFalg_proj ((a),(b))

/** rownum operator */
#define rownum(a,b,c,d) PFalg_rownum ((a),(b),(c),(d))

/* selection operator */
#define select_(a,b)    PFalg_select ((a),(b))

/** type test operator */
#define type(a,b,c,d)   PFalg_type ((a),(b),(c),(d))

/* type cast operator */
#define cast(a,b,c)     PFalg_cast ((a),(b),(c))

/* addition operator */
#define add(a,b,c,d)      PFalg_add ((a),(b),(c),(d))

/* subtraction operator */
#define subtract(a,b,c,d) PFalg_subtract ((a),(b),(c),(d))

/* multiplication operator */
#define multiply(a,b,c,d) PFalg_multiply ((a),(b),(c),(d))

/* division operator */
#define divide(a,b,c,d)   PFalg_divide ((a),(b),(c),(d))

/* numeric equal operator */
#define eq(a,b,c,d)    PFalg_eq ((a),(b),(c),(d))

/* numeric greater-than operator */
#define gt(a,b,c,d) PFalg_gt ((a),(b),(c),(d))

/* numeric negation operator */
#define neg(a,b,c)        PFalg_neg ((a),(b),(c))

/* boolean AND operator */
#define and(a,b,c,d)      PFalg_and ((a),(b),(c),(d))

/* boolean OR operator */
#define or(a,b,c,d)       PFalg_or ((a),(b),(c),(d))

/* boolean NOT operator */
#define not(a,b,c)        PFalg_not ((a),(b),(c))

/* operator forming (partitioned) sum of a column */
#define sum(a,b,c,d)      PFalg_sum ((a),(b),(c),(d))

/* (partitioned) row counting operator */
#define count(a,b,c)      PFalg_count ((a),(b),(c))

/* duplicate elimination operator */
#define distinct(a)       PFalg_distinct ((a))

/* element-constructing operator */
#define element(a,b,c)    PFalg_element ((a),(b),(c))

/* text node-constructing operator */
#define textnode(a,b)     PFalg_textnode ((a),(b))

#define cast_item(o)      PFalg_cast_item (o)

/** serialization */
#define serialize(a)      PFalg_serialize(a)
