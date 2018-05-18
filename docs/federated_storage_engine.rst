.. _federated_storage_engine:

MySQL FEDERATED storage engine support
--------------------------------------

MySQL FEDERATED overview
~~~~~~~~~~~~~~~~~~~~~~~~

The FEDERATED storage engine lets you access data from a remote MySQL
database without using replication or cluster technology. Querying a
local FEDERATED table automatically pulls the data from the remote
Manticore index. No data is stored on the local tables.

With FEDERATED engine a MySQL server can connect to a local or remote Manticore daemon and perform search queries.
Performing queries via FEDERATED is similar to SphinxSE plugin. Unlike SphinxSE, the FEDERATED engine is bundled in all MySQL installs and can be used with Manticore out of the box, without any additional plugin compiling or changes to the MySQL server. 

Using FEDERATED
~~~~~~~~~~~~~~~

An actual Manticore query cannot be used directly with FEDERATED engine and must be "proxied" (send as a string in a column) due to limitations of FEDERATED engine and the fact that Manticore implements custom syntax like the MATCH clause.

To search via FEDERATED, you would need to create first a  FEDERATED engine table.
The Manticore query will be included in a ``query`` column in the ``SELECT`` performed over the FEDERATED table.
 
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

The only fixed mapping is ``query`` column. It is mandatory and must be the only column with an index attached.

The Manticore index that is linked via FEDERATED **must** be an index with storage (plain or RealTime).

FEDERATED table should have columns with same names as remote
Manticore index attributes as will be bound to attributes
provided in Manticore result set by name, however might map
not all attributes but only some of them. Arbitrary expression
from query select list which name “hides” index attribute will
be used at result set.

Manticore daemon identifies query from FEDERATED client by user
name “FEDERATED”.
``CONNECTION`` string parameter should be used to specify searchd
host, SphinxQL port and indexes for queries issued using this table. Connection string
syntax is as follows:

.. code-block:: none


    CONNECTION="mysql://FEDERATED@HOST:PORT/DB/INDEXNAME"

	
Since Manticore doesn't have the concept of database, the ``DB`` string can be random as it will be ignored by Manticore, but MySQL requires a value in the CONNECTION string definition.
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
between Manticore and MySQL.

JOINs can be performed between FEDERATED table and other MySQL tables. This can be used to retrieve information that is not stored in the Manticore index.

.. code-block:: mysql

   mysql> select t1.id,mysqltable.longtext from t1 join mysqltable on t1.id=mysqltable.id where query='SELECT * from test_index where match(\'pile box\')'
