/** atomic natural number construction */
#define lit_nat(n)      PFalg_lit_nat (n)

/** atomic integer construction */
#define lit_int(i)      PFalg_lit_int (i)

/** atomic string construction */
#define lit_str(s)      PFalg_lit_str (s)

/** atomic float construction */
#define lit_flt(f)      PFalg_lit_flt (f)

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
#define doc_tbl()       PFalg_doc_tbl()

/** disjoint union */
#define disjunion(a,b)  PFalg_disjunion ((a),(b))

/** projection operator */
#define project(...)    PFalg_project (__VA_ARGS__)

/** item in the projection list */
#define proj(a,b)       PFalg_proj ((a),(b))

/** rownum operator */
#define rownum(a,b,c,d) PFalg_rownum ((a),(b),(c),(d))

/* selection operator */
#define select(a,b)     PFalg_select ((a),(b))

/* negation operator */
#define negate(a,b,c)   PFalg_negate ((a),(b),(c))

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

#define cast_item(o)         PFalg_cast_item (o)

/** serialization */
#define serialize(a)    PFalg_serialize(a)
