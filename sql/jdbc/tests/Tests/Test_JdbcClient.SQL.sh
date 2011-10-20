#!/bin/sh

cat << EOF > .monetdb
user=monetdb
password=monetdb
EOF

LC_CTYPE="en_US.UTF-8"
export LC_CTYPE

Mlog -x "java nl.cwi.monetdb.client.JdbcClient -h $HOST -p $MAPIPORT -d $TSTDB -f $TSTSRCBASE/$TSTDIR/JdbcClient_create_tables.sql"

Mlog -x "java nl.cwi.monetdb.client.JdbcClient -h $HOST -p $MAPIPORT -d $TSTDB -f $TSTSRCBASE/$TSTDIR/JdbcClient_inserts_selects.sql"

Mlog -x "java nl.cwi.monetdb.client.JdbcClient -h $HOST -p $MAPIPORT -d $TSTDB -D"

rm -f .monetdb
