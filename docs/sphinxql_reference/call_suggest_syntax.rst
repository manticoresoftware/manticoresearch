.. _call_suggest_syntax:

CALL SUGGEST syntax
-------------------

.. code-block:: mysql


    CALL SUGGEST(word, index [,options])

CALL SUGGEST statement works the same as CALL QUSUGGEST, except that if
a bag of words is present, the statement will return suggestions only
for the first word, ignoring the rest. If the first paramenter is a
word, the functionality of CALL SUGGEST and CALL QSUGGEST is the same.
