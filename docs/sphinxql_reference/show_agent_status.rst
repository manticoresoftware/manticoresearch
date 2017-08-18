.. _show_agent_status:

SHOW AGENT STATUS
-----------------

::


    SHOW AGENT ['agent'|'index'] STATUS [ LIKE pattern ]

Displays the statistic of :ref:`remote
agents <agent>` or distributed
index. It includes the values like the age of the last request, last
answer, the number of different kind of errors and successes, etc. The
statistic is shown for every agent for last 1, 5 and 15 intervals, each
of them of
:ref:`ha_period_karma <ha_period_karma>`
seconds. The command exists only in sphinxql.

::


    mysql> SHOW AGENT STATUS;
    +------------------------------------+----------------------------+
    | Variable_name                      | Value                      |
    +------------------------------------+----------------------------+
    | status_period_seconds              | 60                         |
    | status_stored_periods              | 15                         |
    | ag_0_hostname                      | 192.168.0.202:6713         |
    | ag_0_references                    | 2                          |
    | ag_0_lastquery                     | 0.41                       |
    | ag_0_lastanswer                    | 0.19                       |
    | ag_0_lastperiodmsec                | 222                        |
    | ag_0_errorsarow                    | 0                          |
    | ag_0_1periods_query_timeouts       | 0                          |
    | ag_0_1periods_connect_timeouts     | 0                          |
    | ag_0_1periods_connect_failures     | 0                          |
    | ag_0_1periods_network_errors       | 0                          |
    | ag_0_1periods_wrong_replies        | 0                          |
    | ag_0_1periods_unexpected_closings  | 0                          |
    | ag_0_1periods_warnings             | 0                          |
    | ag_0_1periods_succeeded_queries    | 27                         |
    | ag_0_1periods_msecsperquery        | 232.31                     |
    | ag_0_5periods_query_timeouts       | 0                          |
    | ag_0_5periods_connect_timeouts     | 0                          |
    | ag_0_5periods_connect_failures     | 0                          |
    | ag_0_5periods_network_errors       | 0                          |
    | ag_0_5periods_wrong_replies        | 0                          |
    | ag_0_5periods_unexpected_closings  | 0                          |
    | ag_0_5periods_warnings             | 0                          |
    | ag_0_5periods_succeeded_queries    | 146                        |
    | ag_0_5periods_msecsperquery        | 231.83                     |
    | ag_1_hostname                      | 192.168.0.202:6714         |
    | ag_1_references                    | 2                          |
    | ag_1_lastquery                     | 0.41                       |
    | ag_1_lastanswer                    | 0.19                       |
    | ag_1_lastperiodmsec                | 220                        |
    | ag_1_errorsarow                    | 0                          |
    | ag_1_1periods_query_timeouts       | 0                          |
    | ag_1_1periods_connect_timeouts     | 0                          |
    | ag_1_1periods_connect_failures     | 0                          |
    | ag_1_1periods_network_errors       | 0                          |
    | ag_1_1periods_wrong_replies        | 0                          |
    | ag_1_1periods_unexpected_closings  | 0                          |
    | ag_1_1periods_warnings             | 0                          |
    | ag_1_1periods_succeeded_queries    | 27                         |
    | ag_1_1periods_msecsperquery        | 231.24                     |
    | ag_1_5periods_query_timeouts       | 0                          |
    | ag_1_5periods_connect_timeouts     | 0                          |
    | ag_1_5periods_connect_failures     | 0                          |
    | ag_1_5periods_network_errors       | 0                          |
    | ag_1_5periods_wrong_replies        | 0                          |
    | ag_1_5periods_unexpected_closings  | 0                          |
    | ag_1_5periods_warnings             | 0                          |
    | ag_1_5periods_succeeded_queries    | 146                        |
    | ag_1_5periods_msecsperquery        | 230.85                     |
    +------------------------------------+----------------------------+
    50 rows in set (0.01 sec)

An optional LIKE clause is supported. Refer to :ref:`show_meta_syntax` for its syntax details.

::


    mysql> SHOW AGENT STATUS LIKE '%5period%msec%';
    +-----------------------------+--------+
    | Key                         | Value  |
    +-----------------------------+--------+
    | ag_0_5periods_msecsperquery | 234.72 |
    | ag_1_5periods_msecsperquery | 233.73 |
    | ag_2_5periods_msecsperquery | 343.81 |
    +-----------------------------+--------+
    3 rows in set (0.00 sec)

You can specify a particular agent by its address. In this case only
that agent's data will be displayed. Also, ``agent_`` prefix will be used
instead of ``ag_N_``:

::


    mysql> SHOW AGENT '192.168.0.202:6714' STATUS LIKE '%15periods%';
    +-------------------------------------+--------+
    | Variable_name                       | Value  |
    +-------------------------------------+--------+
    | agent_15periods_query_timeouts      | 0      |
    | agent_15periods_connect_timeouts    | 0      |
    | agent_15periods_connect_failures    | 0      |
    | agent_15periods_network_errors      | 0      |
    | agent_15periods_wrong_replies       | 0      |
    | agent_15periods_unexpected_closings | 0      |
    | agent_15periods_warnings            | 0      |
    | agent_15periods_succeeded_queries   | 439    |
    | agent_15periods_msecsperquery       | 231.73 |
    +-------------------------------------+--------+
    9 rows in set (0.00 sec)

Finally, you can check the status of the agents in a specific
distributed index. It can be done with a ``SHOW AGENT ‘index’ STATUS``
statement. That statement shows the index HA status (ie. whether or not
it uses agent mirrors at all), and then the mirror information
(specifically: address, blackhole and persistent flags, and the mirror
selection probability used when one of the :ref:`weighted-probability
strategies <ha_strategy>` is in
effect).

::


    mysql> SHOW AGENT dist_index STATUS;
    +--------------------------------------+--------------------------------+
    | Variable_name                        | Value                          |
    +--------------------------------------+--------------------------------+
    | dstindex_1_is_ha                     | 1                              |
    | dstindex_1mirror1_id                 | 192.168.0.202:6713:loc         |
    | dstindex_1mirror1_probability_weight | 0.372864                       |
    | dstindex_1mirror1_is_blackhole       | 0                              |
    | dstindex_1mirror1_is_persistent      | 0                              |
    | dstindex_1mirror2_id                 | 192.168.0.202:6714:loc         |
    | dstindex_1mirror2_probability_weight | 0.374635                       |
    | dstindex_1mirror2_is_blackhole       | 0                              |
    | dstindex_1mirror2_is_persistent      | 0                              |
    | dstindex_1mirror3_id                 | dev1.sphinxsearch.com:6714:loc |
    | dstindex_1mirror3_probability_weight | 0.252501                       |
    | dstindex_1mirror3_is_blackhole       | 0                              |
    | dstindex_1mirror3_is_persistent      | 0                              |
    +--------------------------------------+--------------------------------+
    13 rows in set (0.00 sec)

