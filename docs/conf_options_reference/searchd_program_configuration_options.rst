.. _searchd program configuration options:

``searchd`` program configuration options
-----------------------------------------

.. _agent_connect_timeout_searchd:

agent_connect_timeout
~~~~~~~~~~~~~~~~~~~~~

Instance-wide defaults for
:ref:`agent_connect_timeout` parameter. The last
defined in distributed (network) indexes.

.. _agent_query_timeout_searchd:

agent_query_timeout
~~~~~~~~~~~~~~~~~~~

Instance-wide defaults for
:ref:`agent_query_timeout` parameter. The last
defined in distributed (network) indexes, or also may be overrided
per-query using OPTION clause.

.. _agent_retry_count:

agent_retry_count
~~~~~~~~~~~~~~~~~

Integer, specifies how many times sphinx will try to connect and query
remote agents in distributed index before reporting fatal query error.
Default is 0 (i.e. no retries). This value may be also specified on
per-query basis using ‘OPTION retry_count=XXX’ clause. If per-query
option exists, it will override the one specified in config.

Note, that if you use `agent
mirrors <agent>` in definition of
your distributed index, then before every attempt of connect sphinx will
select different mirror, according to specified
:ref:`ha_strategy <ha_strategy>`\ specified.

For example, if you have 10 mirrors, and surely know, that at least one
of them alive, then you can definitely take the answer to a correct
query, specifying options ``ha_strategy = roundrobin`` and
``agent_retry_count = 9`` in your config.

.. _agent_retry_delay:

agent_retry_delay
~~~~~~~~~~~~~~~~~

Integer, in milliseconds. Specifies the delay sphinx rest before
retrying to query a remote agent in case it fails. The value has sense
only if non-zero
:ref:`agent_retry_count <agent_retry_count>`
or non-zero per-query OPTION retry_count specified. Default is 500.
This value may be also specified on per-query basis using ‘OPTION
retry_delay=XXX’ clause. If per-query option exists, it will override
the one specified in config.

.. _attr_flush_period:

attr_flush_period
~~~~~~~~~~~~~~~~~

When calling ``UpdateAttributes()`` to update document attributes in
real-time, changes are first written to the in-memory copy of attributes
(``docinfo`` must be set to ``extern``). Then, once ``searchd`` shuts
down normally (via ``SIGTERM`` being sent), the changes are written to
disk.

It is possible to tell ``searchd`` to periodically write these changes
back to disk, to avoid them being lost. The time between those intervals
is set with ``attr_flush_period``, in seconds.

It defaults to 0, which disables the periodic flushing, but flushing
will still occur at normal shut-down.

Example:

.. code-block:: ini


    attr_flush_period = 900 # persist updates to disk every 15 minutes


.. _binlog_flush:

binlog_flush
~~~~~~~~~~~~

Binary log transaction flush/sync mode. Optional, default is 2 (flush
every transaction, sync every second).

This directive controls how frequently will binary log be flushed to OS
and synced to disk. Three modes are supported:

-  0, flush and sync every second. Best performance, but up to 1 second
   worth of committed transactions can be lost both on daemon crash, or
   OS/hardware crash.

-  1, flush and sync every transaction. Worst performance, but every
   committed transaction data is guaranteed to be saved.

-  2, flush every transaction, sync every second. Good performance, and
   every committed transaction is guaranteed to be saved in case of
   daemon crash. However, in case of OS/hardware crash up to 1 second
   worth of committed transactions can be lost.

For those familiar with MySQL and InnoDB, this directive is entirely
similar to ``innodb_flush_log_at_trx_commit``. In most cases, the
default hybrid mode 2 provides a nice balance of speed and safety, with
full RT index data protection against daemon crashes, and some
protection against hardware ones.

Example:

.. code-block:: ini


    binlog_flush = 1 # ultimate safety, low speed

.. _binlog_max_log_size:

binlog_max_log_size
~~~~~~~~~~~~~~~~~~~

Maximum binary log file size. Optional, default is 0 (do not reopen
binlog file based on size).

A new binlog file will be forcibly opened once the current binlog file
reaches this limit. This achieves a finer granularity of logs and can
yield more efficient binlog disk usage under certain borderline
workloads.

Example:


.. code-block:: ini


    binlog_max_log_size = 16M

.. _binlog_path:

binlog_path
~~~~~~~~~~~

Binary log (aka transaction log) files path. Optional, default is
build-time configured data directory.

Binary logs are used for crash recovery of RT index data, and also of
attributes updates of plain disk indices that would otherwise only be
stored in RAM until flush. When logging is enabled, every transaction
COMMIT-ted into RT index gets written into a log file. Logs are then
automatically replayed on startup after an unclean shutdown, recovering
the logged changes.

``binlog_path`` directive specifies the binary log files location. It
should contain just the path; ``searchd`` will create and unlink
multiple binlog.\* files in that path as necessary (binlog data,
metadata, and lock files, etc).

Empty value disables binary logging. That improves performance, but puts
RT index data at risk.

WARNING! It is strongly recommended to always explicitly define
‘binlog_path’ option in your config. Otherwise, the default path, which
in most cases is the same as working folder, may point to the folder
with no write access (for example, /usr/local/var/data). In this case,
the searchd will not start at all.

Example:


.. code-block:: ini


    binlog_path = # disable logging
    binlog_path = /var/data # /var/data/binlog.001 etc will be created

.. _client_timeout:

client_timeout
~~~~~~~~~~~~~~

Maximum time to wait between requests (in seconds) when using persistent
connections. Optional, default is five minutes.

Example:


.. code-block:: ini


    client_timeout = 3600

.. _collation_libc_locale:

collation_libc_locale
~~~~~~~~~~~~~~~~~~~~~

Server libc locale. Optional, default is C.

Specifies the libc locale, affecting the libc-based collations. Refer to
:ref:`collations` section for
the details.

Example:


.. code-block:: ini


    collation_libc_locale = fr_FR

.. _collation_server:

collation_server
~~~~~~~~~~~~~~~~

Default server collation. Optional, default is libc_ci.

Specifies the default collation used for incoming requests. The
collation can be overridden on a per-query basis. Refer to :ref:`collations` section for the list of
available collations and other details.

Example:


.. code-block:: ini


    collation_server = utf8_ci

.. _dist_threads:

dist_threads
~~~~~~~~~~~~

Max local worker threads to use for parallelizable requests (searching a
distributed index; building a batch of snippets). Optional, default is
0, which means to disable in-request parallelism.

Distributed index can include several local indexes. ``dist_threads``
lets you easily utilize multiple CPUs/cores for that (previously
existing alternative was to specify the indexes as remote agents,
pointing searchd to itself and paying some network overheads).

When set to a value N greater than 1, this directive will create up to N
threads for every query, and schedule the specific searches within these
threads. For example, if there are 7 local indexes to search and
dist_threads is set to 2, then 2 parallel threads would be created: one
that sequentially searches 4 indexes, and another one that searches the
other 3 indexes.

In case of CPU bound workload, setting ``dist_threads`` to 1x the number
of cores is advised (creating more threads than cores will not improve
query time). In case of mixed CPU/disk bound workload it might sometimes
make sense to use more (so that all cores could be utilizes even when
there are threads that wait for I/O completion).

Building a batch of snippets with ``load_files`` flag enabled can also
be parallelized. Up to ``dist_threads`` threads are be created to
process those files. That speeds up snippet extraction when the total
amount of document data to process is significant (hundreds of
megabytes).

Example:


.. code-block:: ini


    index dist_test
    {
        type = distributed
        local = chunk1
        local = chunk2
        local = chunk3
        local = chunk4
    }

    # ...

    dist_threads = 4

.. _expansion_limit:

expansion_limit
~~~~~~~~~~~~~~~

The maximum number of expanded keywords for a single wildcard. Optional,
default is 0 (no limit).

When doing substring searches against indexes built with
``dict = keywords`` enabled, a single wildcard may potentially result in
thousands and even millions of matched keywords (think of matching 'a\*'
against the entire Oxford dictionary). This directive lets you limit the
impact of such expansions. Setting ``expansion_limit = N`` restricts
expansions to no more than N of the most frequent matching keywords (per
each wildcard in the query).

Example:


.. code-block:: ini


    expansion_limit = 16

.. _grouping_in_utc:

grouping_in_utc
~~~~~~~~~~~~~~~

Specifies whether timed grouping in API and SphinxQL will be calculated
in local timezone, or in UTC. Optional, default is 0 (means 'local tz').

By default all 'group by time' expressions (like group by day, week,
month and year in API, also group by day, month, year, yearmonth,
yearmonthday in SphinxQL) is done using local time. I.e. when you have
docs with attributes timed ``13:00 utc`` and ``15:00 utc`` - in case of
grouping they both will fall into facility group according to your local
tz setting. Say, if you live in ``utc``, it will be one day, but if you
live in ``utc+10``, then these docs will be matched into different ``group
by day`` facility groups (since 13:00 utc in UTC+10 tz 23:00 local time,
but 15:00 is 01:00 of the next day). Sometimes such behavior is
unacceptable, and it is desirable to make time grouping not dependent
from timezone. Of course, you can run the daemon with defined global TZ
env variable, but it will affect not only grouping, but also
timestamping in the logs, which may be also undesirable. Switching ‘on’
this option (either in config, either using :ref:`set global <set_syntax>` statement in
sphinxql) will cause all time grouping expressions to be calculated in
UTC, leaving the rest of time-depentend functions (i.e. logging of the
daemon) in local TZ.

.. _ha_period_karma:

ha_period_karma
~~~~~~~~~~~~~~~

Agent mirror statistics window size, in seconds. Optional, default is
60.

For a distributed index with agent mirrors in it (see more in `remote
agents <agent>`), master tracks
several different per-mirror counters. These counters are then used for
failover and balancing. (Master picks the best mirror to use based on
the counters.) Counters are accumulated in blocks of ``ha_period_karma``
seconds.

After beginning a new block, master may still use the accumulated values
from the previous one, until the new one is half full. Thus, any
previous history stops affecting the mirror choice after 1.5 times
ha_period_karma seconds at most.

Despite that at most 2 blocks are used for mirror selection, upto 15
last blocks are actually stored, for instrumentation purposes. They can
be inspected using :ref:`SHOW AGENT STATUS <show_agent_status>`
statement.

Example:


.. code-block:: ini


    ha_period_karma = 120

.. _ha_ping_interval:

ha_ping_interval
~~~~~~~~~~~~~~~~

Interval between agent mirror pings, in milliseconds. Optional, default
is 1000.

For a distributed index with agent mirrors in it (see more in :ref:`remote
agents <agent>`), master sends
all mirrors a ping command during the idle periods. This is to track the
current agent status (alive or dead, network roundtrip, etc). The
interval between such pings is defined by this directive.

To disable pings, set ha_ping_interval to 0.

Example:


.. code-block:: ini


    ha_ping_interval = 0

.. _hostname_lookup:

hostname_lookup
~~~~~~~~~~~~~~~

Hostnames renew strategy. By default, IP addresses of agent host names
are cached at daemon start to avoid extra flood to DNS. In some cases
the IP can change dynamically (e.g. cloud hosting) and it might be
desired to don't cache the IPs. Setting this option to ‘request’
disabled the caching and queries the DNS at each query. The IP addresses
can also be manually renewed with FLUSH HOSTNAMES command.

.. _listen_backlog:

listen_backlog
~~~~~~~~~~~~~~

TCP listen backlog. Optional, default is 5.

Windows builds currently can only process the requests one by one.
Concurrent requests will be enqueued by the TCP stack on OS level, and
requests that can not be enqueued will immediately fail with “connection
refused” message. listen_backlog directive controls the length of the
connection queue. Non-Windows builds should work fine with the default
value.

Example:


.. code-block:: ini


    listen_backlog = 20

.. _listen:

listen
~~~~~~

This setting lets you specify IP address and port, or Unix-domain socket
path, that ``searchd`` will listen on.

The informal grammar for ``listen`` setting is:

.. code-block:: ini


    listen = ( address ":" port | port | path ) [ ":" protocol ] [ "_vip" ]

I.e. you can specify either an IP address (or hostname) and port number,
or just a port number, or Unix socket path. If you specify port number
but not the address, ``searchd`` will listen on all network interfaces.
Unix path is identified by a leading slash.

You can also specify a protocol handler (listener) to be used for
connections on this socket. Supported protocol values are ‘sphinx’
(Manticore 0.9.x API protocol) and ‘mysql41’ (MySQL protocol used since 4.1
upto at least 5.1). More details on MySQL protocol support can be found
in :ref:`mysql_protocol_support_and_sphinxql` section.

Adding a “_vip" suffix to a protocol (for instance “sphinx_vip” or
“mysql41_vip”) makes all connections to that port bypass the thread
pool and always forcibly create a new dedicated thread. That's useful
for managing in case of a severe overload when the daemon would either
stall or not let you connect via a regular port.

Examples:
^

.. code-block:: ini


    listen = localhost
    listen = localhost:5000
    listen = 192.168.0.1:5000
    listen = /var/run/sphinx.s
    listen = 9312
    listen = localhost:9306:mysql41

There can be multiple listen directives, ``searchd`` will listen for
client connections on all specified ports and sockets. If no ``listen``
directives are found then the server will listen on all available
interfaces using the default SphinxAPI port 9312, and also on default
SphinxQL port 9306. Both port numbers are assigned by IANA (see
http://www.iana.org/assignments/port-numbers for details) and should
therefore be available.

Unix-domain sockets are not supported on Windows.

.. _log:

log
~~~

Log file name. Optional, default is ‘searchd.log’. All ``searchd`` run
time events will be logged in this file.

Also you can use the ‘syslog’ as the file name. In this case the events
will be sent to syslog daemon. To use the syslog option the sphinx must
be configured ‘–with-syslog’ on building.

Example:


.. code-block:: ini


    log = /var/log/searchd.log

.. _max_batch_queries:

max_batch_queries
~~~~~~~~~~~~~~~~~

Limits the amount of queries per batch. Optional, default is 32.

Makes searchd perform a sanity check of the amount of the queries
submitted in a single batch when using
:ref:`multi-queries <multi_queries>`. Set it to 0 to skip the
check.

Example:


.. code-block:: ini


    max_batch_queries = 256

.. _max_children:

max_children
~~~~~~~~~~~~

Maximum amount of worker threads (or in other words, concurrent queries
to run in parallel). Optional, default is 0 (unlimited) in
workers=threads, or 1.5 times the CPU cores count in
workers=thread_pool mode.

max_children imposes a hard limit on the number of threads working on
user queries. There might still be additional internal threads doing
maintenance tasks, but when it comes to user queries, it is no more than
max_children concurrent threads (and queries) at all times.

Note that in workers=threads mode a thread is allocated for every
connection rather than an active query. So when there are 100 idle
connections but only 2 active connections with currently running
queries, that still accounts for 102 threads, all of them subject to
max_children limit. So with a max_children set way too low, and pooled
connections not reused well enough on the application side, you can
effectively DOS your own server. When the limit is reached, any
additional incoming connections will be dismissed with a temporary
“maxed out” error immediately as they attempt to connect. Thus, in
threads mode you should choose the max_children limit rather carefully,
with not just the concurrent queries but also with potentially idle
*network connections* in mind.

Now, in workers=thread_pool mode the network connections are separated
from query processing, so in the example above, 100 idle connections
will all be handled by an internal network thread, and only the 2
actually active queries will be subject to max_children limit. When the
limit is reached, any additional incoming *connections* will still be
accepted, and any additional *queries* will :ref:`get
enqueued <queue_max_length>`
until there are free worker threads. The queries will only start failing
with a temporary. Thus, in thread_pool mode it makes little sense to
raise max_children much higher than the amount of CPU cores. Usually
that will only hurt CPU contention and *decrease* the general
throughput.

Example:


.. code-block:: ini


    max_children = 10

.. _max_filters:

max_filters
~~~~~~~~~~~

Maximum allowed per-query filter count. Only used for internal sanity
checks, does not directly affect RAM use or performance. Optional,
default is 256.

Example:


.. code-block:: ini


    max_filters = 1024

.. _max_filter_values:

max_filter_values
~~~~~~~~~~~~~~~~~

Maximum allowed per-filter values count. Only used for internal sanity
checks, does not directly affect RAM use or performance. Optional,
default is 4096.

Example:


.. code-block:: ini


    max_filter_values = 16384

.. _max_packet_size:

max_packet_size
~~~~~~~~~~~~~~~

Maximum allowed network packet size. Limits both query packets from
clients, and response packets from remote agents in distributed
environment. Only used for internal sanity checks, does not directly
affect RAM use or performance. Optional, default is 8M.

Example:


.. code-block:: ini


    max_packet_size = 32M

.. _mva_updates_pool:

mva_updates_pool
~~~~~~~~~~~~~~~~

Shared pool size for in-memory MVA updates storage. Optional, default
size is 1M.

This setting controls the size of the shared storage pool for updated
MVA values. Specifying 0 for the size disable MVA updates at all. Once
the pool size limit is hit, MVA update attempts will result in an error.
However, updates on regular (scalar) attributes will still work. Due to
internal technical difficulties, currently it is **not** possible to
store (flush) **any** updates on indexes where MVA were updated;
though this might be implemented in the future. In the meantime, MVA
updates are intended to be used as a measure to quickly catchup with
latest changes in the database until the next index rebuild; not as a
persistent storage mechanism.

Example:


.. code-block:: ini


    mva_updates_pool = 16M

.. _mysql_version_string:

mysql_version_string
~~~~~~~~~~~~~~~~~~~~

A server version string to return via MySQL protocol. Optional, default
is empty (return Manticore version).

Several picky MySQL client libraries depend on a particular version
number format used by MySQL, and moreover, sometimes choose a different
execution path based on the reported version number (rather than the
indicated capabilities flags). For instance, Python MySQLdb 1.2.2 throws
an exception when the version number is not in X.Y.ZZ format; MySQL .NET
connector 6.3.x fails internally on version numbers 1.x along with a
certain combination of flags, etc. To workaround that, you can use
``mysql_version_string`` directive and have ``searchd`` report a
different version to clients connecting over MySQL protocol. (By
default, it reports its own version.)

Example:


.. code-block:: ini


    mysql_version_string = 5.0.37

.. _net_workers:

net_workers
~~~~~~~~~~~

Number of network threads for workers=thread_pool mode, default is 1.

Useful for extremely high query rates, when just 1 thread is not enough
to manage all the incoming queries.

.. _net_wait_tm:

net_wait_tm
~~~~~~~~~~~

Control busy loop interval of a network thread for workers=thread_pool mode,
default is 1, might be set to -1, 0, positive integer.

In case daemon configured as pure master and routes requests to agents
it is important to handle requests without delays and do not allow net-thread
to sleep or cut out from CPU. Here is busy loop to do that. After incoming
request, network thread use CPU poll for ``10 * net_wait_tm`` milliseconds
in case ``net_wait_tm`` is positive number or polls only with CPU in
case ``net_wait_tm`` is ``0``. Also busy loop might be disabled with ``net_wait_tm = -1``
- this way poller set timeout of ``1ms`` for system poll call.

.. _net_throttle_accept:
.. _net_throttle_action:

net_throttle_accept net_throttle_action
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Control network thread for workers=thread_pool mode,
default is 0.

These options define how many clients got accepted and how many requests
processed on each iteration of network loop, in case of value above zero.
Zero value means do not constrain network loop. These options might help to
fine tune network loop throughput at high load scenario.

.. _ondisk_attrs_default:

ondisk_attrs_default
~~~~~~~~~~~~~~~~~~~~

Instance-wide defaults for
:ref:`ondisk_attrs <ondisk_attrs>`
directive. Optional, default is 0 (all attributes are loaded in memory).
This directive lets you specify the default value of ondisk_attrs for
all indexes served by this copy of searchd. Per-index directives take
precedence, and will overwrite this instance-wide default value,
allowing for fine-grain control.

.. _persistent_connections_limit:

persistent_connections_limit
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The maximum # of simultaneous persistent connections to remote
:ref:`persistent
agents <agent_persistent>`. Each
time connecting agent defined under ‘agent_persistent’ we try to reuse
existing connection (if any), or connect and save the connection for the
future. However we can't hold unlimited # of such persistent
connections, since each one holds a worker on agent size (and finally
we'll receive the ‘maxed out’ error, when all of them are busy). This
very directive limits the number. It affects the num of connections to
each agent's host, across all distributed indexes.

It is reasonable to set the value equal or less than
:ref:`max_children <max_children>`
option of the agents.

Example:


.. code-block:: ini


    persistent_connections_limit = 29 # assume that each host of agents has max_children = 30 (or 29).

.. _pid_file:

pid_file
~~~~~~~~

``searchd`` process ID file name. Mandatory.

PID file will be re-created (and locked) on startup. It will contain
head daemon process ID while the daemon is running, and it will be
unlinked on daemon shutdown. It's mandatory because Manticore uses it
internally for a number of things: to check whether there already is a
running instance of ``searchd``; to stop ``searchd``; to notify it that
it should rotate the indexes. Can also be used for different external
automation scripts.

Example:


.. code-block:: ini


    pid_file = /var/run/searchd.pid

.. _predicted_time_costs:

predicted_time_costs
~~~~~~~~~~~~~~~~~~~~

Costs for the query time prediction model, in nanoseconds. Optional,
default is “doc=64, hit=48, skip=2048, match=64” (without the quotes).

Terminating queries before completion based on their execution time (via
either
:ref:`SetMaxQueryTime() <set_max_query_time>`
API call, or :ref:`SELECT … OPTION
max_query_time <select_option>` SphinxQL statement) is a
nice safety net, but it comes with an inborn drawback: indeterministic
(unstable) results. That is, if you repeat the very same (complex)
search query with a time limit several times, the time limit will get
hit at different stages, and you will get *different* result sets.

There is a new option, :ref:`SELECT … OPTION
max_predicted_time <select_option>`, that lets you limit
the query time *and* get stable, repeatable results. Instead of
regularly checking the actual current time while evaluating the query,
which is indeterministic, it predicts the current running time using a
simple linear model instead:

.. code-block:: ini


    predicted_time =
        doc_cost * processed_documents +
        hit_cost * processed_hits +
        skip_cost * skiplist_jumps +
        match_cost * found_matches

The query is then terminated early when the ``predicted_time`` reaches a
given limit.

Of course, this is not a hard limit on the actual time spent (it is,
however, a hard limit on the amount of *processing* work done), and a
simple linear model is in no way an ideally precise one. So the wall
clock time *may* be either below or over the target limit. However, the
error margins are quite acceptable: for instance, in our experiments
with a 100 msec target limit the majority of the test queries fell into
a 95 to 105 msec range, and *all* of the queries were in a 80 to 120
msec range. Also, as a nice side effect, using the modeled query time
instead of measuring actual run time results in somewhat less
gettimeofday() calls, too.

No two server makes and models are identical, so
``predicted_time_costs`` directive lets you configure the costs for the
model above. For convenience, they are integers, counted in nanoseconds.
(The limit in max_predicted_time is counted in milliseconds, and
having to specify cost values as 0.000128 ms instead of 128 ns is
somewhat more error prone.) It is not necessary to specify all 4 costs
at once, as the missed one will take the default values. However, we
strongly suggest to specify all of them, for readability.

Example:


.. code-block:: ini


    predicted_time_costs = doc=128, hit=96, skip=4096, match=128

.. _preopen_indexes:

preopen_indexes
~~~~~~~~~~~~~~~

Whether to forcibly preopen all indexes on startup. Optional, default is
1 (preopen everything).

When set to 1, this directive overrides and enforces
:ref:`preopen <preopen>` on all
indexes. They will be preopened, no matter what is the per-index
``preopen`` setting. When set to 0, per-index settings can take effect.
(And they default to 0.)

Pre-opened indexes avoid races between search queries and rotations that
can cause queries to fail occasionally. They also make ``searchd`` use
more file handles. In most scenarios it's therefore preferred and
recommended to preopen indexes.

Example:


.. code-block:: ini


    preopen_indexes = 1

.. _qcache_max_bytes:

qcache_max_bytes
~~~~~~~~~~~~~~~~

Integer, in bytes. The maximum RAM allocated for cached result sets.
Default is 0, meaning disabled. Refer to `query
cache <query_cache>` for details.

.. code-block:: ini


    qcache_max_bytes = 16777216

.. _qcache_thresh_msec:

qcache_thresh_msec
~~~~~~~~~~~~~~~~~~

Integer, in milliseconds. The minimum wall time threshold for a query
result to be cached. Defaults to 3000, or 3 seconds. 0 means cache
everything. Refer to :ref:`query cache <query_cache>` for details.

.. _qcache_ttl_sec:

qcache_ttl_sec
~~~~~~~~~~~~~~

Integer, in seconds. The expiration period for a cached result set.
Defaults to 60, or 1 minute. The minimum possible value is 1 second.
Refer to :ref:`query cache <query_cache>` for details.

.. _query_log_format:

query_log_format
~~~~~~~~~~~~~~~~

Query log format. Optional, allowed values are ‘plain’ and ‘sphinxql’,
default is ‘plain’.

The default one logs queries in a custom text format. The ‘sphinxql’
logs valid SphinxQL statements. This directive allows to switch between
the two formats on search daemon startup. The log format can also be
altered on the fly, using ``SET GLOBAL query_log_format=sphinxql``
syntax. Refer to :ref:`query_log_formats` for more
discussion and format details.

Example:


.. code-block:: ini


    query_log_format = sphinxql

.. _query_log_min_msec:

query_log_min_msec
~~~~~~~~~~~~~~~~~~

Limit (in milliseconds) that prevents the query from being written to
the query log. Optional, default is 0 (all queries are written to the
query log). This directive specifies that only queries with execution
times that exceed the specified limit will be logged.

.. _query_log:

query_log
~~~~~~~~~

Query log file name. Optional, default is empty (do not log queries).
All search queries will be logged in this file. The format is described
in :ref:`query_log_formats`.
In case of ‘plain’ format, you can use the ‘syslog’ as the path to the
log file. In this case all search queries will be sent to syslog daemon
with LOG_INFO priority, prefixed with ‘[query]’ instead of timestamp.
To use the syslog option the sphinx must be configured ‘–with-syslog’ on
building.

Example:


.. code-block:: ini


    query_log = /var/log/query.log


.. _query_log_mode:

query_log_mode
~~~~~~~~~~~~~~

By default the searchd and query log files are created with 600 permission, so only the user under which daemon runs and root users can read the log files.
query_log_mode allows settings a different permission. 
This can be handy to allow other users to be able to read the log files (for example monitoring solutions running on non-root users).

Example:

.. code-block:: ini


    query_log_mode  = 666

   
	
.. _queue_max_length:

queue_max_length
~~~~~~~~~~~~~~~~

Maximum pending queries queue length for workers=thread_pool mode,
default is 0 (unlimited).

In case of high CPU load thread pool queries queue may grow all the
time. This directive lets you constrain queue length and start rejecting
incoming queries at some point with a “maxed out” message.

.. _read_buffer:

read_buffer
~~~~~~~~~~~

Per-keyword read buffer size. Optional, default is 256K.

For every keyword occurrence in every search query, there are two
associated read buffers (one for document list and one for hit list).
This setting lets you control their sizes, increasing per-query RAM use,
but possibly decreasing IO time.

Example:


.. code-block:: ini


    read_buffer = 1M

.. _read_timeout:

read_timeout
~~~~~~~~~~~~

Network client request read timeout, in seconds. Optional, default is 5
seconds. ``searchd`` will forcibly close the client connections which
fail to send a query within this timeout.

Example:


.. code-block:: ini


    read_timeout = 1

.. _read_unhinted:

read_unhinted
~~~~~~~~~~~~~

Unhinted read size. Optional, default is 32K.

When querying, some reads know in advance exactly how much data is there
to be read, but some currently do not. Most prominently, hit list size
in not currently known in advance. This setting lest you control how
much data to read in such cases. It will impact hit list IO time,
reducing it for lists larger than unhinted read size, but raising it for
smaller lists. It will **not** affect RAM use because read buffer
will be already allocated. So it should be not greater than
read_buffer.

Example:


.. code-block:: ini


    read_unhinted = 32K

.. _rt_flush_period:

rt_flush_period
~~~~~~~~~~~~~~~

RT indexes RAM chunk flush check period, in seconds. Optional, default
is 10 hours.

Actively updated RT indexes that however fully fit in RAM chunks can
result in ever-growing binlogs, impacting disk use and crash recovery
time. With this directive the search daemon performs periodic flush
checks, and eligible RAM chunks can get saved, enabling consequential
binlog cleanup. See :ref:`binary_logging` for more details.

Example:


.. code-block:: ini


    rt_flush_period = 3600 # 1 hour

.. _rt_merge_iops:

rt_merge_iops
~~~~~~~~~~~~~

A maximum number of I/O operations (per second) that the RT chunks merge
thread is allowed to start. Optional, default is 0 (no limit).

This directive lets you throttle down the I/O impact arising from the
``OPTIMIZE`` statements. It is guaranteed that all the RT optimization
activity will not generate more disk iops (I/Os per second) than the
configured limit. Modern SATA drives can perform up to around 100 I/O
operations per second, and limiting rt_merge_iops can reduce search
performance degradation caused by merging.

Example:


.. code-block:: ini


    rt_merge_iops = 40

.. _rt_merge_maxiosize:

rt_merge_maxiosize
~~~~~~~~~~~~~~~~~~

A maximum size of an I/O operation that the RT chunks merge thread is
allowed to start. Optional, default is 0 (no limit).

This directive lets you throttle down the I/O impact arising from the
``OPTIMIZE`` statements. I/Os bigger than this limit will be broken down
into 2 or more I/Os, which will then be accounted as separate I/Os with
regards to the
:ref:`rt_merge_iops <rt_merge_iops>`
limit. Thus, it is guaranteed that all the optimization activity will
not generate more than (rt_merge_iops \* rt_merge_maxiosize) bytes
of disk I/O per second.

Example:


.. code-block:: ini


    rt_merge_maxiosize = 1M

.. _seamless_rotate:

seamless_rotate
~~~~~~~~~~~~~~~

Prevents ``searchd`` stalls while rotating indexes with huge amounts of
data to precache. Optional, default is 1 (enable seamless rotation). On
Windows systems seamless rotation is disabled by default.

Indexes may contain some data that needs to be precached in RAM. At the
moment, ``.spa``, ``.spi`` and ``.spm`` files are fully precached (they
contain attribute data, MVA data, and keyword index, respectively.)
Without seamless rotate, rotating an index tries to use as little RAM as
possible and works as follows:

1. new queries are temporarily rejected (with “retry” error code);

2. ``searchd`` waits for all currently running queries to finish;

3. old index is deallocated and its files are renamed;

4. new index files are renamed and required RAM is allocated;

5. new index attribute and dictionary data is preloaded to RAM;

6. ``searchd`` resumes serving queries from new index.

However, if there's a lot of attribute or dictionary data, then
preloading step could take noticeable time - up to several minutes in
case of preloading 1-5+ GB files.

With seamless rotate enabled, rotation works as follows:

1. new index RAM storage is allocated;

2. new index attribute and dictionary data is asynchronously preloaded
   to RAM;

3. on success, old index is deallocated and both indexes' files are
   renamed;

4. on failure, new index is deallocated;

5. at any given moment, queries are served either from old or new index
   copy.

Seamless rotate comes at the cost of higher **peak** memory usage during
the rotation (because both old and new copies of ``.spa/.spi/.spm`` data
need to be in RAM while preloading new copy). Average usage stays the
same.

Example:


.. code-block:: ini


    seamless_rotate = 1

.. _shutdown_timeout:

shutdown_timeout
~~~~~~~~~~~~~~~~

searchd –stopwait wait time, in seconds. Optional, default is 3 seconds.

When you run searchd –stopwait your daemon needs to perform some
activities before stopping like finishing queries, flushing RT RAM
chunk, flushing attributes and updating binlog. And it requires some
time. searchd –stopwait will wait up to shutdown_time seconds for
daemon to finish its jobs. Suitable time depends on your index size and
load.

Example:


.. code-block:: ini


    shutdown_timeout = 5 # wait for up to 5 seconds

.. _snippets_file_prefix:

snippets_file_prefix
~~~~~~~~~~~~~~~~~~~~

A prefix to prepend to the local file names when generating snippets.
Optional, default is empty.

This prefix can be used in distributed snippets generation along with
``load_files`` or ``load_files_scattered`` options.

Note how this is a prefix, and **not** a path! Meaning that if a
prefix is set to “server1” and the request refers to “file23”,
``searchd`` will attempt to open “server1file23” (all of that without
quotes). So if you need it to be a path, you have to mention the
trailing slash.

Note also that this is a local option, it does not affect the agents
anyhow. So you can safely set a prefix on a master server. The requests
routed to the agents will not be affected by the master's setting. They
will however be affected by the agent's own settings.

This might be useful, for instance, when the document storage locations
(be those local storage or NAS mountpoints) are inconsistent across the
servers.

Example:


.. code-block:: ini


    snippets_file_prefix = /mnt/common/server1/

.. _sphinxql_state:

sphinxql_state
~~~~~~~~~~~~~~

Path to a file where current SphinxQL state will be serialized.

On daemon startup, this file gets replayed. On eligible state changes
(eg. SET GLOBAL), this file gets rewritten automatically. This can
prevent a hard-to-diagnose problem: If you load UDF functions, but
Manticore crashes, when it gets (automatically) restarted, your UDF and
global variables will no longer be available; using persistent state
helps a graceful recovery with no such surprises.

Example:


.. code-block:: ini


    sphinxql_state = uservars.sql

.. _sphinxql_timeout:

sphinxql_timeout
~~~~~~~~~~~~~~~~

Maximum time to wait between requests (in seconds) when using sphinxql
interface. Optional, default is 15 minutes.

Example:


.. code-block:: ini


    sphinxql_timeout = 900

.. _subtree_docs_cache:

subtree_docs_cache
~~~~~~~~~~~~~~~~~~

Max common subtree document cache size, per-query. Optional, default is
0 (disabled).

Limits RAM usage of a common subtree optimizer (see :ref:`multi_queries`). At most this much RAM will
be spent to cache document entries per each query. Setting the limit to
0 disables the optimizer.

Example:


.. code-block:: ini


    subtree_docs_cache = 8M

.. _subtree_hits_cache:

subtree_hits_cache
~~~~~~~~~~~~~~~~~~

Max common subtree hit cache size, per-query. Optional, default is 0
(disabled).

Limits RAM usage of a common subtree optimizer (see :ref:`multi_queries`). At most this much RAM will
be spent to cache keyword occurrences (hits) per each query. Setting the
limit to 0 disables the optimizer.

Example:


.. code-block:: ini


    subtree_hits_cache = 16M

.. _thread_stack:

thread_stack
~~~~~~~~~~~~

Per-thread stack size. Optional, default is 1M.

In the ``workers = threads`` mode, every request is processed with a
separate thread that needs its own stack space. By default, 1M per
thread are allocated for stack. However, extremely complex search
requests might eventually exhaust the default stack and require more.
For instance, a query that matches a thousands of keywords (either
directly or through term expansion) can eventually run out of stack.
``searchd`` attempts to estimate the expected stack use, and blocks the
potentially dangerous queries. To process such queries, you can either
set the thread stack size by using the ``thread_stack`` directive (or
switch to a different ``workers`` setting if that is possible).

A query with N levels of nesting is estimated to require approximately
30+0.16\*N KB of stack, meaning that the default 64K is enough for
queries with upto 250 levels, 150K for upto 700 levels, etc. If the
stack size limit is not met, ``searchd`` fails the query and reports the
required stack size in the error message.

Example:


.. code-block:: ini


    thread_stack = 256K

.. _unlink_old:

unlink_old
~~~~~~~~~~

Whether to unlink .old index copies on successful rotation. Optional,
default is 1 (do unlink).

Example:


.. code-block:: ini


    unlink_old = 0

.. _watchdog:

watchdog
~~~~~~~~

Threaded server watchdog. Optional, default is 1 (watchdog enabled).

A crashed query in ``threads`` multi-processing mode
(``:ref:`workers` = threads``)
can take down the entire server. With watchdog feature enabled,
``searchd`` additionally keeps a separate lightweight process that
monitors the main server process, and automatically restarts the latter
in case of abnormal termination. Watchdog is enabled by default.

Example:


.. code-block:: ini


    watchdog = 0 # disable watchdog

.. _workers:

workers
~~~~~~~

Multi-processing mode (MPM). Optional; allowed values are thread_pool,
and threads. Default is thread_pool.

Lets you choose how ``searchd`` processes multiple concurrent requests.
The possible values are:

-  threads
-  A new dedicated thread is created on every incoming network
   connection. Subsequent queries on that connection are handled by that
   thread. When a client disconnected, the thread gets killed.

-  thread_pool
-  A worker threads pool is created on daemon startup. An internal
   network thread handles all the incoming network connections.
   Subsequent queries on any connection are then put into a queue, and
   processed in order by the first avaialble worker thread from the
   pool. No threads are normally created or killed, neither for new
   connections, nor for new queries. Network thread uses epoll() and
   poll() on Linux, kqueue() on Mac OS/BSD, and WSAPoll on Windows
   (Vista and later). This is the default mode.

Thread pool is a newer, better, faster implementation of threads mode
which does not suffer from overheads of creating a new thread per every
new connection and managing a lot of parallel threads. We still retain
workers=threads for the transition period, but thread pool is scheduled
to become the only MPM mode.

Example:


.. code-block:: ini


    workers = thread_pool

