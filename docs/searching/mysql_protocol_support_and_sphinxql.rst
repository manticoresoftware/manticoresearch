.. _mysql_protocol_support_and_sphinxql:

MySQL protocol support and SphinxQL
-----------------------------------

Manticore searchd daemon supports MySQL binary network protocol and can be
accessed with regular MySQL API. For instance, ‘mysql’ CLI client
program works well. Here's an example of querying Manticore using MySQL
client:

.. code-block:: mysql


    $ mysql -P 9306
    Welcome to the MySQL monitor.  Commands end with ; or \g.
    Your MySQL connection id is 1
    Server version: 0.9.9-dev (r1734)

    Type 'help;' or '\h' for help. Type '\c' to clear the buffer.

    mysql> SELECT * FROM test1 WHERE MATCH('test')
        -> ORDER BY group_id ASC OPTION ranker=bm25;
    +------+--------+----------+------------+
    | id   | weight | group_id | date_added |
    +------+--------+----------+------------+
    |    4 |   1442 |        2 | 1231721236 |
    |    2 |   2421 |      123 | 1231721236 |
    |    1 |   2421 |      456 | 1231721236 |
    +------+--------+----------+------------+
    3 rows in set (0.00 sec)

Note that mysqld was not even running on the test machine. Everything
was handled by searchd itself.

The new access method is supported *in addition* to native APIs which
all still work perfectly well. In fact, both access methods can be used
at the same time. Also, native API is still the default access method.
MySQL protocol support needs to be additionally configured. This is a
matter of 1-line config change, adding a new
:ref:`listener <listen>` with
mysql41 specified as a protocol:

.. code-block:: mysql


    listen = localhost:9306:mysql41

Just supporting the protocol and not the SQL syntax would be useless so
Manticore now also supports a subset of SQL that we dubbed SphinxQL. It
supports the standard querying all the index types with SELECT,
modifying RT indexes with INSERT, REPLACE, and DELETE, and much more.
Full SphinxQL reference is available in :ref:`sphinxql_reference`.
