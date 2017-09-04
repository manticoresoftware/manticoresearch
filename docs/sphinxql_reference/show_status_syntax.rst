.. _show_status_syntax:

SHOW STATUS syntax
------------------

.. code-block:: mysql


    SHOW STATUS [ LIKE pattern ]

**SHOW STATUS** displays a number of useful performance counters. IO
and CPU counters will only be available if searchd was started with
–iostats and –cpustats switches respectively.

.. code-block:: mysql


    mysql> SHOW STATUS;
    +--------------------+-------+
    | Counter            | Value |
    +--------------------+-------+
    | uptime             | 216   |
    | connections        | 3     |
    | maxed_out          | 0     |
    | command_search     | 0     |
    | command_excerpt    | 0     |
    | command_update     | 0     |
    | command_keywords   | 0     |
    | command_persist    | 0     |
    | command_status     | 0     |
    | agent_connect      | 0     |
    | agent_retry        | 0     |
    | queries            | 10    |
    | dist_queries       | 0     |
    | query_wall         | 0.075 |
    | query_cpu          | OFF   |
    | dist_wall          | 0.000 |
    | dist_local         | 0.000 |
    | dist_wait          | 0.000 |
    | query_reads        | OFF   |
    | query_readkb       | OFF   |
    | query_readtime     | OFF   |
    | avg_query_wall     | 0.007 |
    | avg_query_cpu      | OFF   |
    | avg_dist_wall      | 0.000 |
    | avg_dist_local     | 0.000 |
    | avg_dist_wait      | 0.000 |
    | avg_query_reads    | OFF   |
    | avg_query_readkb   | OFF   |
    | avg_query_readtime | OFF   |
    +--------------------+-------+
    29 rows in set (0.00 sec)

An optional LIKE clause is supported. Refer to :ref:`show_meta_syntax` for its syntax details.
