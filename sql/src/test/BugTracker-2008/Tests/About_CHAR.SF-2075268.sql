CREATE TABLE TMP (T1 CHAR (10));
INSERT INTO TMP (T1) VALUES ( 'xxxx');
INSERT INTO TMP (T1) VALUES ( 'yyyy ');
INSERT INTO TMP (T1) VALUES ( ' zzzz');
SELECT * FROM TMP WHERE T1 = 'xxxx ';
SELECT * FROM TMP WHERE T1 = 'xxxx';
SELECT * FROM TMP WHERE T1 = 'yyyy ';
SELECT * FROM TMP WHERE T1 = 'yyyy';
SELECT * FROM TMP WHERE T1 = 'zzzz';
SELECT * FROM TMP WHERE T1 = ' zzzz';
DROP TABLE TMP;
