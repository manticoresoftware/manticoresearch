.. _set_syntax:

SET syntax
----------

::


    SET [GLOBAL] server_variable_name = value
    SET [INDEX index_name] GLOBAL @user_variable_name = (int_val1 [, int_val2, ...])
    SET NAMES value
    SET @@dummy_variable = ignored_value

SET statement modifies a variable value. The variable names are
case-insensitive. No variable value changes survive server restart.

SET NAMES statement and SET @@variable_name syntax, both introduced do
nothing. They were implemented to maintain compatibility with 3rd party
MySQL client libraries, connectors, and frameworks that may need to run
this statement when connecting.

There are the following classes of the variables:

1. per-session server variable

2. global server variable

3. global user variable

4. global distributed variable

Global user variables are shared between concurrent sessions. Currently,
the only supported value type is the list of BIGINTs, and these
variables can only be used along with IN() for filtering purpose. The
intended usage scenario is uploading huge lists of values to ``searchd``
(once) and reusing them (many times) later, saving on network overheads.
Global user variables might be either transferred to all agents of
distributed index or set locally in case of local index defined at
distibuted index. Example:

::


    // in session 1
    mysql> SET GLOBAL @myfilter=(2,3,5,7,11,13);
    Query OK, 0 rows affected (0.00 sec)

    // later in session 2
    mysql> SELECT * FROM test1 WHERE group_id IN @myfilter;
    +------+--------+----------+------------+-----------------+------+
    | id   | weight | group_id | date_added | title           | tag  |
    +------+--------+----------+------------+-----------------+------+
    |    3 |      1 |        2 | 1299338153 | another doc     | 15   |
    |    4 |      1 |        2 | 1299338153 | doc number four | 7,40 |
    +------+--------+----------+------------+-----------------+------+
    2 rows in set (0.02 sec)

Per-session and global server variables affect certain server settings
in the respective scope. Known per-session server variables are:

-  ``AUTOCOMMIT = {0 | 1}``
-  Whether any data modification statement should be implicitly wrapped
   by BEGIN and COMMIT.

-  ``COLLATION_CONNECTION = collation_name``
-  Selects the collation to be used for ORDER BY or GROUP BY on string
   values in the subsequent queries. Refer to `the section called
   “Collations” <collations>` for a list of known collation
   names.

-  ``CHARACTER_SET_RESULTS = charset_name``
-  Does nothing; a placeholder to support frameworks, clients, and
   connectors that attempt to automatically enforce a charset when
   connecting to a Manticore server.

-  ``SQL_AUTO_IS_NULL = value``
-  Does nothing; a placeholder to support frameworks, clients, and
   connectors that attempt to automatically enforce a charset when
   connecting to a Manticore server.

-  ``SQL_MODE = value``
-  Does nothing; a placeholder to support frameworks, clients, and
   connectors that attempt to automatically enforce a charset when
   connecting to a Manticore server.

-  ``PROFILING = {0 | 1}``
-  Enables query profiling in the current session. Defaults to 0. See
   also :ref:`show_profile_syntax`.

Known global server variables are:

-  ``QUERY_LOG_FORMAT = {plain | sphinxql}``
-  Changes the current log format.

-  ``LOG_LEVEL = {info | debug | debugv | debugvv}``
-  Changes the current log verboseness level.

-  ``QCACHE_MAX_BYTES = <value>``
-  Changes the :ref:`query cache <query_cache>` RAM use limit to a
   given value.

-  ``QCACHE_THRESH_MSEC = <value>``
-  Changes the :ref:`query cache <query_cache>` minimum wall time
   threshold to a given value.

-  ``QCACHE_TTL_SEC = <value>``
-  Changes the :ref:`query cache <query_cache>` TTL for a cached
   result to a given value.

-  ``MAINTENANCE = {0 | 1}``
-  When set to 1, puts the server in maintenance mode. Only clients with
   vip connections can execute queries in this mode. All new non-vip
   incoming connections are refused.

-  ``GROUPING_IN_UTC = {0 | 1}``
-  When set to 1, cause timed grouping functions (day(), month(),
   year(), yearmonth(), yearmonthday()) to be calculated in utc. Read
   the doc for
   :ref:`grouping_in_utc <grouping_in_utc>`
   config params for more details.

Examples:

::


    mysql> SET autocommit=0;
    Query OK, 0 rows affected (0.00 sec)

    mysql> SET GLOBAL query_log_format=sphinxql;
    Query OK, 0 rows affected (0.00 sec)

