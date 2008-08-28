CREATE TABLE A(q1 bigint, q2 bigint);
INSERT INTO A VALUES('4567890123456789','4567890123456789');

SELECT 2 * q1 AS q FROM A;

drop table A;

CREATE TABLE A2
(COL1 INTEGER,
COL2 INTEGER);
INSERT INTO A2 VALUES (111,1112);

SELECT SUM(2 * COL1 * COL2)
FROM A2;

drop table A2;

CREATE TABLE A (
COL1 INTEGER,
COL2 INTEGER,
COL3 INTEGER
);

INSERT INTO A VALUES(1000,-2000,NULL);

SELECT (-COL2+COL1)
FROM A
WHERE COL3 IS NULL;

drop table a;

