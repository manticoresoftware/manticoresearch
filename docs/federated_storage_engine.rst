Interface for MySQL FEDERATED storage engine
--------------------------------------------

MySQL FEDERATED overview
~~~~~~~~~~~~~~~~~~~~~~~~

The FEDERATED storage engine lets you access data from a remote MySQL
database without using replication or cluster technology. Querying a
local FEDERATED table automatically pulls the data from the remote
Manticore index. No data is stored on the local tables.

FEDERATED is actually allows MySQL server to talk to ``searchd``,
run search queries, and obtain search results. All indexing and searching
happen outside MySQL.

FEDERATED is available for MySQL since 5.x series. Manticore daemon since 2.6.4
has interface compatible with queries from FEDERATED storage engine.


Using FEDERATED
^^^^^^^^^^^^^^^

To search via FEDERATED, you would need to create special table
“search table”, and then ``SELECT`` from it with full SphinxQL query put into
WHERE clause for ``query`` column. 

Let's begin with an example create statement and search query:

.. code-block:: mysql


    CREATE TABLE t1
    (
        id          INTEGER UNSIGNED NOT NULL,
        channel_id  INTEGER NOT NULL,
        group_id    INTEGER,
        query       VARCHAR(3072) NOT NULL,
        INDEX(query)
    ) ENGINE=FEDERATED
    DEFAULT CHARSET=utf8
    CONNECTION='mysql://FEDERATED@127.0.0.1:9306/DB/test_index';

    SELECT * FROM t1 WHERE query='SELECT * FROM test_index WHERE MATCH (\'pile box\') AND channel_id<1000 GROUP BY group_id';

The only fixed mapping is ``query`` column. It is mandatory and
should have the only index in the table.
	
FEDERATED table should have columns with same names as remote
Manticore index attributes as will be bound to attributes
provided in Manticore result set by name, however might map
not all attributes but only some of them. Arbitrary expression
from query select list wich name “hides” index attribute will
be used at result set.

Manticore daemon identifies query from FEDERATED client by user
name “FEDERATED”.
``CONNECTION`` string parameter should be used to specify default searchd
host, port and indexes for queries issued using this table. Connection string
syntax is as follows:

.. code-block:: none


    CONNECTION="mysql://FEDERATED@HOST:PORT/INDEXNAME"

	
As seen in example, full ``SELECT`` SphinxQL query should be put
into WHERE clause on search query column :ref:`select_syntax`.

Only ``SELECT`` statement is supported but not ``INSERT``, ``REPLACE``,
``UPDATE``, ``DELETE``.

One **very important** note that it is **much** more efficient to allow
Manticore to perform sorting, filtering and slicing the result set than to
raise max matches count and use WHERE, ORDER BY and LIMIT clauses on
MySQL side. This is for two reasons. First, Manticore does a number of
optimizations and performs better than MySQL on these tasks. Second,
less data would need to be packed by searchd, transferred and unpacked
by FEDERATED.
