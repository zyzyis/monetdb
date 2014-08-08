start transaction;
CREATE TABLE BOOLTBL1 (f1 bool);
INSERT INTO BOOLTBL1 (f1) VALUES (cast('true' AS boolean));
INSERT INTO BOOLTBL1 (f1) VALUES ('true');
INSERT INTO BOOLTBL1 (f1) VALUES ('false');

SELECT f1 FROM BOOLTBL1 WHERE f1 = NOT FALSE;
SELECT f1 FROM BOOLTBL1 WHERE f1 = NOT TRUE;

SELECT f1 FROM BOOLTBL1 WHERE f1 = (NOT FALSE);
SELECT f1 FROM BOOLTBL1 WHERE NOT FALSE = f1;
SELECT f1 FROM BOOLTBL1 WHERE NOT f1 = FALSE;

SELECT f1 FROM BOOLTBL1 WHERE f1 = (NOT TRUE);
SELECT f1 FROM BOOLTBL1 WHERE NOT TRUE = f1;
SELECT f1 FROM BOOLTBL1 WHERE NOT f1 = TRUE;
