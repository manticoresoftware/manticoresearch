.. _attach_index_syntax:

ATTACH INDEX syntax
-------------------

.. code-block:: mysql


    ATTACH INDEX diskindex TO RTINDEX rtindex

ATTACH INDEX statement lets you move data from a regular disk index to a
RT index.

After a successful ATTACH, the data originally stored in the source disk
index becomes a part of the target RT index, and the source disk index
becomes unavailable (until the next rebuild). ATTACH does not result in
any index data changes. Basically, it just renames the files (making the
source index a new disk chunk of the target RT index), and updates the
metadata. So it is a generally quick operation which might (frequently)
complete as fast as under a second.

Note that when an index is attached to an empty RT index, the fields,
attributes, and text processing settings (tokenizer, wordforms, etc)
from the *source* index are copied over and take effect. The respective
parts of the RT index definition from the configuration file will be
ignored.

ATTACH INDEX comes with a number of restrictions. Most notably, the
target RT index is currently required to be empty, making ATTACH INDEX a
one-time conversion operation only. Those restrictions may be lifted in
future releases, as we add the needed functionality to the RT indexes.
The complete list is as follows.

-  Target RT index needs to be empty. (See :ref:`truncate_rtindex_syntax`)

-  Source disk index needs to have index_sp=0, boundary_step=0,
   stopword_step=1.

-  Source disk index needs to have an empty index_zones setting.

.. code-block:: mysql


    mysql> DESC rt;
    +-----------+---------+
    | Field     | Type    |
    +-----------+---------+
    | id        | integer |
    | testfield | field   |
    | testattr  | uint    |
    +-----------+---------+
    3 rows in set (0.00 sec)

    mysql> SELECT * FROM rt;
    Empty set (0.00 sec)

    mysql> SELECT * FROM disk WHERE MATCH('test');
    +------+--------+----------+------------+
    | id   | weight | group_id | date_added |
    +------+--------+----------+------------+
    |    1 |   1304 |        1 | 1313643256 |
    |    2 |   1304 |        1 | 1313643256 |
    |    3 |   1304 |        1 | 1313643256 |
    |    4 |   1304 |        1 | 1313643256 |
    +------+--------+----------+------------+
    4 rows in set (0.00 sec)

    mysql> ATTACH INDEX disk TO RTINDEX rt;
    Query OK, 0 rows affected (0.00 sec)

    mysql> DESC rt;
    +------------+-----------+
    | Field      | Type      |
    +------------+-----------+
    | id         | integer   |
    | title      | field     |
    | content    | field     |
    | group_id   | uint      |
    | date_added | timestamp |
    +------------+-----------+
    5 rows in set (0.00 sec)

    mysql> SELECT * FROM rt WHERE MATCH('test');
    +------+--------+----------+------------+
    | id   | weight | group_id | date_added |
    +------+--------+----------+------------+
    |    1 |   1304 |        1 | 1313643256 |
    |    2 |   1304 |        1 | 1313643256 |
    |    3 |   1304 |        1 | 1313643256 |
    |    4 |   1304 |        1 | 1313643256 |
    +------+--------+----------+------------+
    4 rows in set (0.00 sec)

    mysql> SELECT * FROM disk WHERE MATCH('test');
    ERROR 1064 (42000): no enabled local indexes to search

