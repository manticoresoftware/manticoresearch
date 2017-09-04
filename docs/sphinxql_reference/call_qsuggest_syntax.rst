.. _call_qsuggest_syntax:

CALL QSUGGEST syntax
--------------------

.. code-block:: mysql


    CALL QSUGGEST(word, index [,options])

CALL QSUGGEST statement enumerates for a giving word all suggestions
from the dictionary. This statement works only on indexes with infixing
enabled and dict=keywords. It returns the suggested keywords,
Levenshtein distance between the suggested and original keyword and the
docs statistic of the suggested keyword. If the first parameter is a bag
of words, the function will return suggestions only for the last word,
ignoring the rest. Several options are supported for customization:

-  ``limit`` - returned N top matches, default is 5

-  ``max_edits`` - keep only dictionary words which Levenshtein distance
   is less or equal, default is 4

-  ``result_stats`` - provide Levenshtein distance and document count of
   the found words, default is 1 (enabled)

-  ``delta_len`` - keep only dictionary words whose length difference is
   less, default is 3

-  ``max_matches`` - number of matches to keep, default is 25

-  ``reject`` - defaults to 4; rejected words are matches that are not
   better than those already in the match queue. They are put in a
   rejected queue that gets reset in case one actually can go in the
   match queue. This parameter defines the size of the rejected queue
   (as reject\*max(max_matched,limit)). If the rejected queue is
   filled, the engine stops looking for potential matches.

-  ``result_line`` - alternate mode to display the data by returning all
   suggests, distances and docs each per one row, default is 0

-  ``non_char`` - do not skip dictionary words with non alphabet
   symbols, default is 0 (skip such words)

.. code-block:: mysql


    mysql> CALL QSUGGEST('automaticlly ','forum', 5 as limit, 4 as max_edits,1 as result_stats,3 as delta_len,0 as result_line,25 as max_matches,4 as reject );
    +---------------+----------+------+
    | suggest       | distance | docs |
    +---------------+----------+------+
    | automatically | 1        | 282  |
    | automaticly   | 1        | 6    |
    | automaticaly  | 1        | 3    |
    | automagically | 2        | 14   |
    | automtically  | 2        | 1    |
    +---------------+----------+------+
    5 rows in set (0.00 sec)

