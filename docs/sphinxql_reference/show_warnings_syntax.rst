.. _show_warnings_syntax:

SHOW WARNINGS syntax
--------------------

.. code-block:: mysql


    SHOW WARNINGS

**SHOW WARNINGS** statement can be used to retrieve the warning
produced by the latest query. The error message will be returned along
with the query itself:

.. code-block:: mysql


    mysql> SELECT * FROM test1 WHERE MATCH('@@title hello') \G
    ERROR 1064 (42000): index test1: syntax error, unexpected TOK_FIELDLIMIT
    near '@title hello'

    mysql> SELECT * FROM test1 WHERE MATCH('@title -hello') \G
    ERROR 1064 (42000): index test1: query is non-computable (single NOT operator)

    mysql> SELECT * FROM test1 WHERE MATCH('"test doc"/3') \G
    *************************** 1\. row ***************************
            id: 4
        weight: 2500
      group_id: 2
    date_added: 1231721236
    1 row in set, 1 warning (0.00 sec)

    mysql> SHOW WARNINGS \G
    *************************** 1\. row ***************************
      Level: warning
       Code: 1000
    Message: quorum threshold too high (words=2, thresh=3); replacing quorum operator
             with AND operator
    1 row in set (0.00 sec)

