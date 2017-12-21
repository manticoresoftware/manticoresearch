.. _show_meta_syntax:

SHOW META syntax
----------------

.. code-block:: mysql


    SHOW META [ LIKE pattern ]

**SHOW META** shows additional meta-information about the latest
query such as query time and keyword statistics. IO and CPU counters
will only be available if searchd was started with –iostats and
–cpustats switches respectively. Additional predicted_time,
dist_predicted_time, [{local\|dist}]*fetched*\ [{docs\|hits\|skips}]
counters will only be available if searchd was configured with
:ref:`predicted time
costs <predicted_time_costs>`
and query had predicted_time in OPTION clause.

.. code-block:: mysql


    mysql> SELECT * FROM test1 WHERE MATCH('test|one|two');
    +------+--------+----------+------------+
    | id   | weight | group_id | date_added |
    +------+--------+----------+------------+
    |    1 |   3563 |      456 | 1231721236 |
    |    2 |   2563 |      123 | 1231721236 |
    |    4 |   1480 |        2 | 1231721236 |
    +------+--------+----------+------------+
    3 rows in set (0.01 sec)

    mysql> SHOW META;
    +-----------------------+-------+
    | Variable_name         | Value |
    +-----------------------+-------+
    | total                 | 3     |
    | total_found           | 3     |
    | time                  | 0.005 |
    | keyword[0]            | test  |
    | docs[0]               | 3     |
    | hits[0]               | 5     |
    | keyword[1]            | one   |
    | docs[1]               | 1     |
    | hits[1]               | 2     |
    | keyword[2]            | two   |
    | docs[2]               | 1     |
    | hits[2]               | 2     |
    | cpu_time              | 0.350 |
    | io_read_time          | 0.004 |
    | io_read_ops           | 2     |
    | io_read_kbytes        | 0.4   |
    | io_write_time         | 0.000 |
    | io_write_ops          | 0     |
    | io_write_kbytes       | 0.0   |
    | agents_cpu_time       | 0.000 |
    | agent_io_read_time    | 0.000 |
    | agent_io_read_ops     | 0     |
    | agent_io_read_kbytes  | 0.0   |
    | agent_io_write_time   | 0.000 |
    | agent_io_write_ops    | 0     |
    | agent_io_write_kbytes | 0.0   |
    +-----------------------+-------+
    12 rows in set (0.00 sec)

You can also use the optional LIKE clause. It lets you pick just the
variables that match a pattern. The pattern syntax is that of regular
SQL wildcards, that is, ‘%’ means any number of any characters, and '_'
means a single character:

.. code-block:: mysql


    mysql> SHOW META LIKE 'total%';
    +-----------------------+-------+
    | Variable_name         | Value |
    +-----------------------+-------+
    | total                 | 3     |
    | total_found           | 3     |
    +-----------------------+-------+
    2 rows in set (0.00 sec)

	
SHOW META can be used after executing a :ref:`CALL PQ <call_pq_syntax>`  statement.
In this case, it provides a different :ref:`output <percolate_query_show_meta>`.
	