
SELECT * FROM (
SELECT 'SYSTEM TABLE' AS TABLE_TYPE, name FROM tables WHERE type = 0
UNION ALL
SELECT 'NO TABLE' AS TABLE_TYPE, name FROM tables WHERE type = 2
) AS a
WHERE TABLE_TYPE LIKE 'TABLE';

SELECT * FROM (
SELECT 'SYSTEM TABLE' AS TABLE_TYPE, name FROM tables
UNION ALL
SELECT 'NO TABLE' AS TABLE_TYPE, name FROM tables
) AS a
WHERE TABLE_TYPE LIKE 'TABLE';

SELECT * FROM tables
WHERE (name LIKE 'TABLE' OR name LIKE 'VIEW');

SELECT 'bla' AS iets WHERE iets LIKE 'boe';
ROLLBACK;

savepoint sp1;
select * from session;
release savepoint sp1;


