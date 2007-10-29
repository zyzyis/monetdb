#!/bin/bash

cat << EOF > .monetdb
user=monetdb
password=monetdb
EOF

LANG="en_US.UTF-8"
export LANG

Mlog -x "$MTIMEOUT mjclient -h $HOST -p $MAPIPORT -d ${TSTDB} -f \"$RELSRCDIR/jdbc_no_debug.SF-1739356-data.sql\""

rm -f .monetdb
