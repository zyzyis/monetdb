# The contents of this file are subject to the MonetDB Public License
# Version 1.1 (the "License"); you may not use this file except in
# compliance with the License. You may obtain a copy of the License at
# http://www.monetdb.org/Legal/MonetDBLicense
#
# Software distributed under the License is distributed on an "AS IS"
# basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
# License for the specific language governing rights and limitations
# under the License.
#
# The Original Code is the MonetDB Database System.
#
# The Initial Developer of the Original Code is CWI.
# Portions created by CWI are Copyright (C) 1997-July 2008 CWI.
# Copyright August 2008-2013 MonetDB B.V.
# All Rights Reserved.

cat <<EOF
# The contents of this file are subject to the MonetDB Public License
# Version 1.1 (the "License"); you may not use this file except in
# compliance with the License. You may obtain a copy of the License at
# http://www.monetdb.org/Legal/MonetDBLicense
#
# Software distributed under the License is distributed on an "AS IS"
# basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
# License for the specific language governing rights and limitations
# under the License.
#
# The Original Code is the MonetDB Database System.
#
# The Initial Developer of the Original Code is CWI.
# Portions created by CWI are Copyright (C) 1997-July 2008 CWI.
# Copyright August 2008-2013 MonetDB B.V.
# All Rights Reserved.

# This file was generated by using the script ${0##*/}.

module aggr;

EOF

integer="bte sht int wrd lng"	# all integer types
numeric="$integer flt dbl"	# all numeric types
fixtypes="bit $numeric oid"
alltypes="$fixtypes str"

for tp1 in 1:bte 2:sht 4:int 8:wrd 8:lng; do
    for tp2 in 8:dbl 1:bte 2:sht 4:int 4:wrd 8:lng; do
	if [ ${tp1%:*} -le ${tp2%:*} -o ${tp1#*:} = ${tp2#*:} ]; then
	    cat <<EOF
command sum(b:bat[:oid,:${tp1#*:}], e:bat[:oid,:any_1]) :bat[:oid,:${tp2#*:}]
address AGGRsum2_${tp2#*:}
comment "Sum over grouped tail sum on ${tp1#*:}";

command sum(b:bat[:oid,:${tp1#*:}],g:bat[:oid,:oid],e:bat[:oid,:any_1])
		:bat[:oid,:${tp2#*:}]
address AGGRsum3_${tp2#*:}
comment "Grouped tail sum on ${tp1#*:}";

EOF
	    if [ ${tp2#*:} = dbl ]; then
		continue
	    fi
	    cat <<EOF
command subsum(b:bat[:oid,:${tp1#*:}],g:bat[:oid,:oid],e:bat[:oid,:any_1],skip_nils:bit,abort_on_error:bit) :bat[:oid,:${tp2#*:}]
address AGGRsubsum_${tp2#*:}
comment "Grouped sum aggregate";

command subsum(b:bat[:oid,:${tp1#*:}],g:bat[:oid,:oid],e:bat[:oid,:any_1],s:bat[:oid,:oid],skip_nils:bit,abort_on_error:bit) :bat[:oid,:${tp2#*:}]
address AGGRsubsumcand_${tp2#*:}
comment "Grouped sum aggregate with candidates list";

command prod(b:bat[:oid,:${tp1#*:}], e:bat[:oid,:any_1]) :bat[:oid,:${tp2#*:}]
address AGGRprod2_${tp2#*:}
comment "Product over grouped tail product on ${tp1#*:}";

command prod(b:bat[:oid,:${tp1#*:}],g:bat[:oid,:oid],e:bat[:oid,:any_1])
		:bat[:oid,:${tp2#*:}]
address AGGRprod3_${tp2#*:}
comment "Grouped tail product on ${tp1#*:}";

command subprod(b:bat[:oid,:${tp1#*:}],g:bat[:oid,:oid],e:bat[:oid,:any_1],skip_nils:bit,abort_on_error:bit) :bat[:oid,:${tp2#*:}]
address AGGRsubprod_${tp2#*:}
comment "Grouped product aggregate";

command subprod(b:bat[:oid,:${tp1#*:}],g:bat[:oid,:oid],e:bat[:oid,:any_1],s:bat[:oid,:oid],skip_nils:bit,abort_on_error:bit) :bat[:oid,:${tp2#*:}]
address AGGRsubprodcand_${tp2#*:}
comment "Grouped product aggregate with candidates list";

EOF
	fi
    done
done

for tp1 in 4:flt 8:dbl; do
    for tp2 in 4:flt 8:dbl; do
	if [ ${tp1%:*} -le ${tp2%:*} ]; then
	    cat <<EOF
command sum(b:bat[:oid,:${tp1#*:}], e:bat[:oid,:any_1]) :bat[:oid,:${tp2#*:}]
address AGGRsum2_${tp2#*:}
comment "Sum over grouped tail sum on ${tp1#*:}";

command sum(b:bat[:oid,:${tp1#*:}],g:bat[:oid,:oid],e:bat[:oid,:any_1])
		:bat[:oid,:${tp2#*:}]
address AGGRsum3_${tp2#*:}
comment "Grouped tail sum on ${tp1#*:}";

command subsum(b:bat[:oid,:${tp1#*:}],g:bat[:oid,:oid],e:bat[:oid,:any_1],skip_nils:bit,abort_on_error:bit) :bat[:oid,:${tp2#*:}]
address AGGRsubsum_${tp2#*:}
comment "Grouped sum aggregate";

command subsum(b:bat[:oid,:${tp1#*:}],g:bat[:oid,:oid],e:bat[:oid,:any_1],s:bat[:oid,:oid],skip_nils:bit,abort_on_error:bit) :bat[:oid,:${tp2#*:}]
address AGGRsubsumcand_${tp2#*:}
comment "Grouped sum aggregate with candidates list";

command prod(b:bat[:oid,:${tp1#*:}], e:bat[:oid,:any_1]) :bat[:oid,:${tp2#*:}]
address AGGRprod2_${tp2#*:}
comment "Product over grouped tail product on ${tp1#*:}";

command prod(b:bat[:oid,:${tp1#*:}],g:bat[:oid,:oid],e:bat[:oid,:any_1])
		:bat[:oid,:${tp2#*:}]
address AGGRprod3_${tp2#*:}
comment "Grouped tail product on ${tp1#*:}";

command subprod(b:bat[:oid,:${tp1#*:}],g:bat[:oid,:oid],e:bat[:oid,:any_1],skip_nils:bit,abort_on_error:bit) :bat[:oid,:${tp2#*:}]
address AGGRsubprod_${tp2#*:}
comment "Grouped product aggregate";

command subprod(b:bat[:oid,:${tp1#*:}],g:bat[:oid,:oid],e:bat[:oid,:any_1],s:bat[:oid,:oid],skip_nils:bit,abort_on_error:bit) :bat[:oid,:${tp2#*:}]
address AGGRsubprodcand_${tp2#*:}
comment "Grouped product aggregate with candidates list";

EOF
	fi
    done
done

# We may have to extend the signatures to all possible {void,oid} combos
for tp in bte sht int wrd lng flt dbl; do
    cat <<EOF
command avg(b:bat[:oid,:${tp}], e:bat[:oid,:any_1]) :bat[:oid,:dbl]
address AGGRavg2_dbl
comment "Grouped tail average on ${tp}";

command avg(b:bat[:oid,:${tp}], g:bat[:oid,:oid], e:bat[:oid,:any_1]):bat[:oid,:dbl]
address AGGRavg3_dbl
comment "Grouped tail average on ${tp}";

command subavg(b:bat[:oid,:${tp}],g:bat[:oid,:oid],e:bat[:oid,:any_1],skip_nils:bit,abort_on_error:bit) :bat[:oid,:dbl]
address AGGRsubavg_dbl
comment "Grouped avg aggregate";

command subavg(b:bat[:oid,:${tp}],g:bat[:oid,:oid],e:bat[:oid,:any_1],s:bat[:oid,:oid],skip_nils:bit,abort_on_error:bit) :bat[:oid,:dbl]
address AGGRsubavgcand_dbl
comment "Grouped avg aggregate with candidates list";

EOF
    for func in stdev variance; do
	cat <<EOF
command ${func}(b:bat[:oid,:${tp}], e:bat[:oid,:any_1]) :bat[:oid,:dbl]
address AGGR${func}2_dbl
comment "Grouped tail average on ${tp}";

command ${func}(b:bat[:oid,:${tp}], g:bat[:oid,:oid], e:bat[:oid,:any_1]):bat[:oid,:dbl]
address AGGR${func}3_dbl
comment "Grouped tail average on ${tp}";

command sub${func}(b:bat[:oid,:${tp}],g:bat[:oid,:oid],e:bat[:oid,:any_1],skip_nils:bit,abort_on_error:bit) :bat[:oid,:dbl]
address AGGRsub${func}_dbl
comment "Grouped ${func} aggregate";

command sub${func}(b:bat[:oid,:${tp}],g:bat[:oid,:oid],e:bat[:oid,:any_1],s:bat[:oid,:oid],skip_nils:bit,abort_on_error:bit) :bat[:oid,:dbl]
address AGGRsub${func}cand_dbl
comment "Grouped ${func} aggregate with candidates list";

command ${func}p(b:bat[:oid,:${tp}], e:bat[:oid,:any_1]) :bat[:oid,:dbl]
address AGGR${func}p2_dbl
comment "Grouped tail average on ${tp}";

command ${func}p(b:bat[:oid,:${tp}], g:bat[:oid,:oid], e:bat[:oid,:any_1]):bat[:oid,:dbl]
address AGGR${func}p3_dbl
comment "Grouped tail average on ${tp}";

command sub${func}p(b:bat[:oid,:${tp}],g:bat[:oid,:oid],e:bat[:oid,:any_1],skip_nils:bit,abort_on_error:bit) :bat[:oid,:dbl]
address AGGRsub${func}p_dbl
comment "Grouped ${func}p aggregate";

command sub${func}p(b:bat[:oid,:${tp}],g:bat[:oid,:oid],e:bat[:oid,:any_1],s:bat[:oid,:oid],skip_nils:bit,abort_on_error:bit) :bat[:oid,:dbl]
address AGGRsub${func}pcand_dbl
comment "Grouped ${func}p aggregate with candidates list";

EOF
    done
done

cat <<EOF
command min(b:bat[:oid,:any_1], e:bat[:oid,:any_2]) :bat[:oid,:any_1]
address AGGRmin2;

command max(b:bat[:oid,:any_1], e:bat[:oid,:any_2]) :bat[:oid,:any_1]
address AGGRmax2;

command min(b:bat[:oid,:any_1],g:bat[:oid,:oid],e:bat[:oid,:any_2]):bat[:oid,:any_1]
address AGGRmin3;

command max(b:bat[:oid,:any_1], g:bat[:oid,:oid], e:bat[:oid,:any_2])
		:bat[:oid,:any_1]
address AGGRmax3;

command submin(b:bat[:oid,:any_1],g:bat[:oid,:oid],e:bat[:oid,:any_2],skip_nils:bit) :bat[:oid,:oid]
address AGGRsubmin
comment "Grouped minimum aggregate";

command submin(b:bat[:oid,:any_1],g:bat[:oid,:oid],e:bat[:oid,:any_2],s:bat[:oid,:oid],skip_nils:bit) :bat[:oid,:oid]
address AGGRsubmincand
comment "Grouped minimum aggregate with candidates list";

command submax(b:bat[:oid,:any_1],g:bat[:oid,:oid],e:bat[:oid,:any_2],skip_nils:bit) :bat[:oid,:oid]
address AGGRsubmax
comment "Grouped maximum aggregate";

command submax(b:bat[:oid,:any_1],g:bat[:oid,:oid],e:bat[:oid,:any_2],s:bat[:oid,:oid],skip_nils:bit) :bat[:oid,:oid]
address AGGRsubmaxcand
comment "Grouped maximum aggregate with candidates list";

command submin(b:bat[:oid,:any_1],g:bat[:oid,:oid],e:bat[:oid,:any_2],skip_nils:bit) :bat[:oid,:any_1]
address AGGRsubmin_val
comment "Grouped minimum aggregate";

command submin(b:bat[:oid,:any_1],g:bat[:oid,:oid],e:bat[:oid,:any_2],s:bat[:oid,:oid],skip_nils:bit) :bat[:oid,:any_1]
address AGGRsubmincand_val
comment "Grouped minimum aggregate with candidates list";

command submax(b:bat[:oid,:any_1],g:bat[:oid,:oid],e:bat[:oid,:any_2],skip_nils:bit) :bat[:oid,:any_1]
address AGGRsubmax_val
comment "Grouped maximum aggregate";

command submax(b:bat[:oid,:any_1],g:bat[:oid,:oid],e:bat[:oid,:any_2],s:bat[:oid,:oid],skip_nils:bit) :bat[:oid,:any_1]
address AGGRsubmaxcand_val
comment "Grouped maximum aggregate with candidates list";

command count(b:bat[:oid,:any_1], e:bat[:oid,:any_2], ignorenils:bit)
	:bat[:oid,:wrd]
address AGGRcount2
comment "Grouped count";

command count(b:bat[:oid,:any_1], g:bat[:oid,:oid], e:bat[:oid,:any_2],
		ignorenils:bit) :bat[:void,:wrd]
address AGGRcount3;

command size(b:bat[:void,:bit], e:bat[:void,:any_1]) :bat[:void,:wrd]
address AGGRsize2nils
comment "Grouped count of true values";

command count(b:bat[:void,:any_1], e:bat[:oid,:any_2]) :bat[:void,:wrd]
address AGGRcount2nils
comment "Grouped count";

command count(b:bat[:void,:any_1], e:bat[:void,:any_2]) :bat[:void,:wrd]
address AGGRcount2nils;

command count_no_nil(b:bat[:oid,:any_1],e:bat[:oid,:any_1]):bat[:oid,:wrd]
address AGGRcount2nonils;

command count(b:bat[:oid,:any_1], g:bat[:oid,:oid], e:bat[:oid,:any_2])
	:bat[:oid,:wrd]
address AGGRcount3nils
comment "Grouped count";

command count_no_nil(b:bat[:oid,:any_1],g:bat[:oid,:oid],e:bat[:oid,:any_2])
	:bat[:oid,:wrd]
address AGGRcount3nonils;

command subcount(b:bat[:oid,:any_1],g:bat[:oid,:oid],e:bat[:oid,:any_2],skip_nils:bit) :bat[:oid,:wrd]
address AGGRsubcount
comment "Grouped count aggregate";

command subcount(b:bat[:oid,:any_1],g:bat[:oid,:oid],e:bat[:oid,:any_2],s:bat[:oid,:oid],skip_nils:bit) :bat[:oid,:wrd]
address AGGRsubcountcand
comment "Grouped count aggregate with candidates list";

command median(b:bat[:oid,:any_1],g:bat[:oid,:oid],e:bat[:oid,:any_2]) :bat[:oid,:any_1]
address AGGRmedian3
comment "Grouped median aggregate";

function median(b:bat[:oid,:any_1]) :any_1;
	bn := submedian(b, false);
	return algebra.fetch(bn, 0);
end aggr.median;

command submedian(b:bat[:oid,:any_1],skip_nils:bit) :bat[:oid,:any_1]
address AGGRmedian
comment "Median aggregate";

command submedian(b:bat[:oid,:any_1],g:bat[:oid,:oid],e:bat[:oid,:any_2],skip_nils:bit) :bat[:oid,:any_1]
address AGGRsubmedian
comment "Grouped median aggregate";

command submedian(b:bat[:oid,:any_1],g:bat[:oid,:oid],e:bat[:oid,:any_2],s:bat[:oid,:oid],skip_nils:bit) :bat[:oid,:any_1]
address AGGRsubmediancand
comment "Grouped median aggregate with candidate list";
EOF
