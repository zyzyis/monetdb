#!/usr/bin/env bash

q=uncompilable_comparison.SF-1871790.xq
echo "\"pf -M $q >/dev/null\""
echo "\"pf -M $q >/dev/null\"" >&2
pf -M $q >/dev/null
echo "\"pf -A $q >/dev/null\""
echo "\"pf -A $q >/dev/null\"" >&2
pf -A $q >/dev/null
