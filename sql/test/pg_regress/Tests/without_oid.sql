--
-- WITHOUT OID
--

--
-- This test tries to verify that WITHOUT OIDS actually saves space.
-- On machines where MAXALIGN is 8, WITHOUT OIDS may or may not save any
-- space, depending on the size of the tuple header + null bitmap.
-- As of 8.0 we need a 9-bit null bitmap to force the difference to appear.
--
CREATE TABLE wi (oid oid GENERATED ALWAYS AS IDENTITY, i INT,
                 n1 int, n2 int, n3 int, n4 int,
                 n5 int, n6 int, n7 int, n8 int) /* WITH OIDS */;
CREATE TABLE wo (i INT,
                 n1 int, n2 int, n3 int, n4 int,
                 n5 int, n6 int, n7 int, n8 int) /* WITHOUT OIDS */;

INSERT INTO wi (i) VALUES (1);  -- 1
INSERT INTO wo (i) SELECT i FROM wi;  -- 1
INSERT INTO wo (i) SELECT i+1 FROM wi;  -- 1+1=2
INSERT INTO wi (i) SELECT i+1 FROM wo;  -- 1+2=3
INSERT INTO wi (i) SELECT i+3 FROM wi;  -- 3+3=6
INSERT INTO wo (i) SELECT i+2 FROM wi;  -- 2+6=8
INSERT INTO wo (i) SELECT i+8 FROM wo;  -- 8+8=16
INSERT INTO wi (i) SELECT i+6 FROM wo;  -- 6+16=22
INSERT INTO wi (i) SELECT i+22 FROM wi;  -- 22+22=44
INSERT INTO wo (i) SELECT i+16 FROM wi;  -- 16+44=60
INSERT INTO wo (i) SELECT i+60 FROM wo;  -- 60+60=120
INSERT INTO wi (i) SELECT i+44 FROM wo;  -- 44+120=164
INSERT INTO wi (i) SELECT i+164 FROM wi;  -- 164+164=328
INSERT INTO wo (i) SELECT i+120 FROM wi;  -- 120+328=448
INSERT INTO wo (i) SELECT i+448 FROM wo;  -- 448+448=896
INSERT INTO wi (i) SELECT i+328 FROM wo;  -- 328+896=1224
INSERT INTO wi (i) SELECT i+1224 FROM wi;  -- 1224+1224=2448
INSERT INTO wo (i) SELECT i+896 FROM wi;  -- 896+2448=3344
INSERT INTO wo (i) SELECT i+3344 FROM wo;  -- 3344+3344=6688
INSERT INTO wi (i) SELECT i+2448 FROM wo;  -- 2448+6688=9136
INSERT INTO wo (i) SELECT i+6688 FROM wi WHERE i<=2448;  -- 6688+2448=9136

SELECT count(oid) FROM wi;
SELECT count(oid) FROM wo;-- should fail
SELECT count(i) FROM wo;

-- Replaced PostgreSQL "VACUUM ANALYZE my_table;" with MonetDB "call vacuum('sys', 'my_table');" and analyze sys.my_table;"
call vacuum('sys', 'wi'); analyze sys.wi;
call vacuum('sys', 'wo'); analyze sys.wo;

/*
SELECT min(relpages) < max(relpages), min(reltuples) - max(reltuples)
  FROM pg_class
 WHERE relname IN ('wi', 'wo');
*/
SELECT "schema", "table", "column", type, count, typewidth, columnsize, heapsize, hashes, imprints, sorted FROM sys.storage
 WHERE "table" IN ('wi', 'wo');

DROP TABLE wi;
DROP TABLE wo;


--
-- WITH / WITHOUT OIDS in CREATE TABLE AS
--
CREATE TABLE create_table_test (
    a int,
    b int
);

INSERT INTO create_table_test VALUES (5, 10);
INSERT INTO create_table_test VALUES (10, 15);

CREATE TABLE create_table_test2 /* WITH OIDS */ AS
    SELECT row_number() over () AS oid, a + b AS c1, a - b AS c2 FROM create_table_test WITH DATA;

CREATE TABLE create_table_test3 /* WITHOUT OIDS */ AS
    SELECT a + b AS c1, a - b AS c2 FROM create_table_test WITH DATA;

SELECT count(oid) FROM create_table_test2;
SELECT count(oid) FROM create_table_test3; -- should fail
SELECT count(*) FROM create_table_test3;

DROP TABLE create_table_test;
DROP TABLE create_table_test2;
DROP TABLE create_table_test3;
