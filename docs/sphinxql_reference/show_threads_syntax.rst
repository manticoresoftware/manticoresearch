.. _show_threads_syntax:

SHOW THREADS syntax
-------------------

.. code-block:: none


    SHOW THREADS [ OPTION columns=width ]

SHOW THREADS lists all currently active client threads, not counting
system threads. It returns a table with columns that describe:

-  **thread id**
-  **connection protocol**, possible values are sphinxapi and
   sphinxql
-  **thread state**, possible values are handshake, net_read,
   net_write, query, net_idle
-  **time** since the current state was changed (in seconds, with
   microsecond precision)
-  **information** about queries

The ‘Info’ column will be cut at the width you've specified in the
‘columns=width’ option (notice the third row in the example table
below). This column will contain raw SphinxQL queries and, if there are
API queries, full text syntax and comments will be displayed. With an
API-snippet, the data size will be displayed along with the query.

.. code-block:: none


    mysql> SHOW THREADS OPTION columns=50;
    +------+----------+-------+----------+----------------------------------------------------+
    | Tid  | Proto    | State | Time     | Info                                               |
    +------+----------+-------+----------+----------------------------------------------------+
    | 5168 | sphinxql | query | 0.000002 | show threads option columns=50                     |
    | 5175 | sphinxql | query | 0.000002 | select * from rt where match ( 'the box' )         |
    | 1168 | sphinxql | query | 0.000002 | select * from rt where match ( 'the box and faximi |
    +------+----------+-------+----------+----------------------------------------------------+
    3 row in set (0.00 sec)

