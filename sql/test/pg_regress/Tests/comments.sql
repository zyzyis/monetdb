--
-- COMMENTS
--

SELECT 'trailing' AS first; -- trailing single line
SELECT /* embedded single line */ 'embedded' AS "second";
SELECT /* both embedded and trailing single line */ 'both' AS third; -- trailing single line

SELECT 'before multi-line' AS fourth;
/* This is an example of SQL which should not execute:
 * select 'multi-line';
 */
SELECT 'after multi-line' AS fifth;

--
-- Nested comments
--

/*
SELECT 'trailing' as x1; -- inside block comment
*/

/* This block comment surrounds a query which itself has a block comment...
SELECT /* embedded single line */ 'embedded' AS x2;
*/

SELECT -- continued after the following block comments...
/* Deeply nested comment.
   This includes a single apostrophe to make sure we aren't decoding this part as a string.
SELECT 'deep nest' AS n1;
/* Second level of nesting...
SELECT 'deeper nest' as n2;
/* Third level of nesting...
SELECT 'deepest nest' as n3;
*/
Hoo boy. Still two deep...
*/
Now just one deep...
*/
'deeply nested example' AS sixth;

/* and this is the end of the file */
