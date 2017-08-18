.. _query_log_formats:

Query log formats
-----------------------------

Two query log formats are supported. Plain text format is still the
default one. However, while it might be more convenient for manual
monitoring and review, but hard to replay for benchmarks, it only logs
*search* queries but not the other types of requests, does not always
contain the complete search query data, etc. The default text format is
also harder (and sometimes impossible) to replay for benchmarking
purposes. The ``sphinxql`` format alleviates that. It aims to be
complete and automatable, even though at the cost of brevity and
readability.


.. _plain_log_format:

Plain log format
~~~~~~~~~~~~~~~~

By default, ``searchd`` logs all successfully executed search queries
into a query log file. Here's an example:

::


    [Fri Jun 29 21:17:58 2007] 0.004 sec 0.004 sec [all/0/rel 35254 (0,20)] [lj] test
    [Fri Jun 29 21:20:34 2007] 0.024 sec 0.024 sec [all/0/rel 19886 (0,20) @channel_id] [lj] test

This log format is as follows:

::


    [query-date] real-time wall-time [match-mode/filters-count/sort-mode
        total-matches (offset,limit) @groupby-attr] [index-name] query

-  real-time is a time measured just from start to finish of the query

-  wall-time like real-time but not including waiting for agents and
   merging result sets time

Match mode can take one of the following values:

-  “all” for SPH_MATCH_ALL mode;

-  “any” for SPH_MATCH_ANY mode;

-  “phr” for SPH_MATCH_PHRASE mode;

-  “bool” for SPH_MATCH_BOOLEAN mode;

-  “ext” for SPH_MATCH_EXTENDED mode;

-  “ext2” for SPH_MATCH_EXTENDED2 mode;

-  “scan” if the full scan mode was used, either by being specified with
   SPH_MATCH_FULLSCAN, or if the query was empty (as documented under
   :ref:`Matching Modes <matching_modes>`)

Sort mode can take one of the following values:

-  “rel” for SPH_SORT_RELEVANCE mode;

-  “attr-” for SPH_SORT_ATTR_DESC mode;

-  “attr+” for SPH_SORT_ATTR_ASC mode;

-  “tsegs” for SPH_SORT_TIME_SEGMENTS mode;

-  “ext” for SPH_SORT_EXTENDED mode.

Additionally, if ``searchd`` was started with ``--iostats``, there will
be a block of data after where the index(es) searched are listed.

A query log entry might take the form of:

::


    [Fri Jun 29 21:17:58 2007] 0.004 sec [all/0/rel 35254 (0,20)] [lj]
       [ios=6 kb=111.1 ms=0.5] test

This additional block is information regarding I/O operations in
performing the search: the number of file I/O operations carried out,
the amount of data in kilobytes read from the index files and time spent
on I/O operations (although there is a background processing component,
the bulk of this time is the I/O operation time).



.. _sphinxQL_log_format:

SphinxQL log format
~~~~~~~~~~~~~~~~~~~

This new log format introduced with the goals begin logging everything
and then some, and in a format easy to automate (for instance,
automatically replay). SphinxQL log format can either be enabled via the
:ref:`query_log_format <query_log_format>`
directive in the configuration file, or switched back and forth on the
fly with the
:ref:`SET GLOBAL query_log_format=... <set_syntax>` statement
via SphinxQL. In the new format, the example from the previous section
would look as follows. (Wrapped below for readability, but with just one
query per line in the actual log.)

::


    /* Fri Jun 29 21:17:58.609 2007 2011 conn 2 real 0.004 wall 0.004 found 35254 */
    SELECT * FROM lj WHERE MATCH('test') OPTION ranker=proximity;

    /* Fri Jun 29 21:20:34 2007.555 conn 3 real 0.024 wall 0.024 found 19886 */
    SELECT * FROM lj WHERE MATCH('test') GROUP BY channel_id
    OPTION ranker=proximity;

Note that **all** requests would be logged in this format, including
those sent via SphinxAPI and SphinxSE, not just those sent via SphinxQL.
Also note, that this kind of logging works only with plain log files and
will not work if you use ‘syslog’ service for logging.

The features of SphinxQL log format compared to the default text one are
as follows.

-  All request types should be logged. (This is still work in progress.)

-  Full statement data will be logged where possible.

-  Errors and warnings are logged.

-  The log should be automatically replayable via SphinxQL.

-  Additional performance counters (currently, per-agent distributed
   query times) are logged.

Use sphinxql:compact_in to shorten your IN() clauses in log if you have
too much values in it.

Every request (including both SphinxAPI and SphinxQL) request must
result in exactly one log line. All request types, including INSERT,
CALL SNIPPETS, etc will eventually get logged, though as of time of this
writing, that is a work in progress). Every log line must be a valid
SphinxQL statement that reconstructs the full request, except if the
logged request is too big and needs shortening for performance reasons.
Additional messages, counters, etc can be logged in the comments section
after the request.
