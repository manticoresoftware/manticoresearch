-- Lets see what indexes do we have.
SHOW TABLES;

-- Try to perfoms simples possible fullscan search.
SELECT * FROM test1;

-- Now fulltext search. Note, documets was weighted by ranker.
-- '"document one"/1 will search for documents containing 'document' or 'one'
-- keywords. We can rewrite it as 'document | one'.
SELECT *, WEIGHT() FROM test1 WHERE MATCH('"document one"/1');
-- This query will show us some statistic over the last query.
-- Very useful to see show morphology and star queries actualy treat
-- keywords query.
SHOW META;

-- For SHOW PROFILE and SHOW PLAN to work.
SET profiling=1;
SELECT * FROM test1 WHERE id IN (1,2,4);
SHOW PROFILE;
SELECT id, id%3 idd FROM test1 WHERE MATCH('this is | nothing') GROUP BY idd;
SHOW PROFILE;

SELECT id FROM test1 WHERE MATCH('is this a good plan?');
-- This allow to understand better how exactly Sphinx is going
-- to implement your query.
SHOW PLAN;

-- HAVING works too.
SELECT COUNT(*) c, id%3 idd FROM test1 GROUP BY idd HAVING COUNT(*)>1;

-- How many records in that index?
SELECT COUNT(*) FROM test1;

-- Lets show how morphology works on words in this query.
CALL KEYWORDS ('one two three', 'test1');
-- Now inclode some statistics in response.
CALL KEYWORDS ('one two three', 'test1', 1);
