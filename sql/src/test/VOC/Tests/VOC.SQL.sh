#!/bin/bash

cat << EOF > .monetdb
user=monetdb
password=monetdb
EOF

LC_ALL="`locale -a | grep -i en_us | grep -i utf | head -n 1`"
export LC_ALL

Mlog -x "$MTIMEOUT mjclient -h $HOST -p $MAPIPORT -d ${TSTDB} -e -f \"$RELSRCDIR/../VOCcreate_user.sql\" 2>&1"

cat << EOF > .monetdb
user=voc
password=voc
EOF

Mlog -x "$MTIMEOUT mjclient -h $HOST -p $MAPIPORT -d ${TSTDB} -e -f \"$RELSRCDIR/../VOCschema.sql\" 2>&1"
Mlog   "$MTIMEOUT mjclient -h $HOST -p $MAPIPORT -d ${TSTDB} -e -f \"$RELSRCDIR/../VOCinsert.sql\" 2>&1"
Mlog -x "$MTIMEOUT mjclient -h $HOST -p $MAPIPORT -d ${TSTDB} -e -f \"$RELSRCDIR/../VOCquery.sql\" 2>&1"
Mlog -x "$MTIMEOUT mjclient -h $HOST -p $MAPIPORT -d ${TSTDB} -e -f \"$RELSRCDIR/../VOCmanual_examples.sql\" 2>&1"
Mlog -x "$MTIMEOUT mjclient -h $HOST -p $MAPIPORT -d ${TSTDB} -e -f \"$RELSRCDIR/../VOCdrop.sql\" 2>&1"

cat << EOF > .monetdb
user=monetdb
password=monetdb
EOF

Mlog -x "$MTIMEOUT mjclient -h $HOST -p $MAPIPORT -d ${TSTDB} -e -f \"$RELSRCDIR/../VOCdrop_user.sql\" 2>&1"

rm -f .monetdb
