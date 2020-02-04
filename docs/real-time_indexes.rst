.. _real-time_indexes:

Real-time indexes
=================

Real-time indexes (or RT indexes for brevity) are a backend that lets
you insert, update, or delete documents (rows) on the fly. While
querying of RT indexes is possible using any of the SphinxAPI, SphinxQL,
or SphinxSE, updating them is only possible via SphinxQL at the moment.
Full SphinxQL reference is available in  :doc:`sphinxql_reference`.


RT indexes overview
-------------------

RT indexes should be declared in ``manticore.conf``, just as every other
index type. Notable differences from the regular, disk-based indexes are
that a) data sources are not required and ignored, and b) you should
explicitly enumerate all the text fields, not just attributes. Here's an
example:


.. code-block:: bash


    index rt
    {
        type = testrt
        path = /var/lib/manticore/data/testrt
        rt_field = title
        rt_field = content
        rt_attr_uint = gid
    }

RT index can be accessed using MySQL protocol. INSERT, REPLACE, DELETE,
and SELECT statements against RT index are supported. For instance, this
is an example session with the sample index above:

.. code-block:: bash


    $ mysql -h 127.0.0.1 -P 9306
    Welcome to the MySQL monitor.  Commands end with ; or \g.
    Your MySQL connection id is 1
    Server version: 1.10-dev (r2153)

    Type 'help;' or '\h' for help. Type '\c' to clear the buffer.

    mysql> INSERT INTO testrt VALUES ( 1, 'first record', 'test one', 123 );
    Query OK, 1 row affected (0.05 sec)

    mysql> INSERT INTO testrt VALUES ( 2, 'second record', 'test two', 234 );
    Query OK, 1 row affected (0.00 sec)

    mysql> SELECT * FROM testrt;
    +------+------+---------------+----------+
    | id   | gid  | title         | content  |
    +------+------+---------------+----------+
    |    1 |  123 | first record  | test one |
    |    2 |  234 | second record | test two |
    +------+------+---------------+----------+
    2 rows in set (0.00 sec)


    mysql> SELECT * FROM testrt WHERE MATCH('first one');
    +------+------+--------------+----------+
    | id   | gid  | title        | content  |
    +------+------+--------------+----------+
    |    1 |  123 | first record | test one |
    +------+------+--------------+----------+
    1 row in set (0.00 sec)

Both partial and batch INSERT syntaxes are supported, ie. you can
specify a subset of columns, and insert several rows at a time.
Deletions are also possible using DELETE statement; the only currently
supported syntax is DELETE FROM <index> WHERE id=<id>. REPLACE is also
supported, enabling you to implement updates.
Autoincrement values on the ID primary attribute is supported.

.. code-block:: mysql


    mysql> INSERT INTO testrt ( id, title ) VALUES ( 3, 'third row' ), ( 4, 'fourth entry' );
    Query OK, 2 rows affected (0.01 sec)

    mysql> SELECT * FROM testrt;
    +------+--------+------+
    | id   | weight | gid  |
    +------+--------+------+
    |    1 |      1 |  123 |
    |    2 |      1 |  234 |
    |    3 |      1 |    0 |
    |    4 |      1 |    0 |
    +------+--------+------+
    4 rows in set (0.00 sec)

    mysql> DELETE FROM testrt WHERE id=2;
    Query OK, 0 rows affected (0.00 sec)

    mysql> SELECT * FROM testrt WHERE MATCH('test');
    +------+--------+------+
    | id   | weight | gid  |
    +------+--------+------+
    |    1 |   1500 |  123 |
    +------+--------+------+
    1 row in set (0.00 sec)

    mysql> INSERT INTO testrt (title,content,gid) VALUES ('a new record','test three',100);
    ERROR 1064 (42000): duplicate id '1'

    mysql> REPLACE INTO testrt VALUES ( 1, 'first record changed', 'test one', 123 );
    Query OK, 1 row affected (0.01 sec)

    mysql> SELECT * FROM testrt WHERE MATCH('steroids');
    +------+--------+------+
    | id   | weight | gid  |
    +------+--------+------+
    |    1 |   1500 |  123 |
    +------+--------+------+
    1 row in set (0.01 sec)

Data stored in RT index should survive clean shutdown. When binary
logging is enabled, it should also survive crash and/or dirty shutdown,
and recover on subsequent startup.


Known caveats with RT indexes
-----------------------------

RT indexes are currently quality feature, but there are still a few
known usage quirks. Those quirks are listed in this section.

-  Default conservative RAM chunk limit (``rt_mem_limit``) of 32M can
   lead to poor performance on bigger indexes, you should raise it to
   256..1024M if you're planning to index gigabytes.

-  High DELETE/REPLACE rate can lead to kill-list fragmentation and
   impact searching performance. Records are not immediately purged, but only marked as delete. 
   Reclaiming the space used by deleted/old version documents can be done with  :ref:`OPTIMIZE <optimize_index_syntax>` command 
   which during merging the RT chunks also purge marked records.

-  No transaction size limits are currently imposed; too many concurrent
   INSERT/REPLACE transactions might therefore consume a lot of RAM.

-  In case of a damaged binlog, recovery will stop on the first damaged
   transaction, even though it's technically possible to keep looking
   further for subsequent undamaged transactions, and recover those.
   This mid-file damage case (due to flaky HDD/CDD/tape?) is supposed to
   be extremely rare, though.

-  Multiple INSERTs grouped in a single transaction perform better than
   equivalent single-row transactions and are recommended for batch
   loading of data.
   
- Autoincrement ID values don't start from zero, values are generated using an algorithm that makes
  sure in case of Real-Time indexes replicated in a cluster don't generate same IDs

   
RT index internals
------------------

RT index is internally chunked. It keeps a so-called RAM chunk that
stores all the most recent changes. RAM chunk memory usage is rather
strictly limited with per-index
:ref:`rt_mem_limit <rt_mem_limit>`
directive. Once RAM chunk grows over this limit, a new disk chunk is
created from its data, and RAM chunk is reset. Thus, while most changes
on the RT index will be performed in RAM only and complete instantly (in
milliseconds), those changes that overflow the RAM chunk will stall for
the duration of disk chunk creation (a few seconds).

Manticore uses double-buffering to avoid INSERT stalls. When data is being
dumped to disk, the second buffer is used, so further INSERTs won't be
delayed. The second buffer is defined to be 10% the size of the standard
buffer,
:ref:`rt_mem_limit <rt_mem_limit>`, but
future versions of Manticore may allow configuring this further.

Disk chunks are, in fact, just regular disk-based indexes. But they're a
part of an RT index and automatically managed by it, so you need not
configure nor manage them manually. Because a new disk chunk is created
every time RT chunk overflows the limit, and because in-memory chunk
format is close to on-disk format, the disk chunks will be approximately
``rt_mem_limit`` bytes in size each.

Generally, it is better to set the limit bigger, to minimize both the
frequency of flushes, and the index fragmentation (number of disk
chunks). For instance, on a dedicated search server that handles a big
RT index, it can be advised to set ``rt_mem_limit`` to 1-2 GB. A global
limit on all indexes is also planned, but not yet implemented.

Disk chunk full-text index data can not be actually modified, so the
full-text field changes (ie. row deletions and updates) suppress a
previous row version from a disk chunk using a kill-list, but do not
actually physically purge the data. Therefore, on workloads with high
full-text updates ratio index might eventually get polluted by these
previous row versions, and searching performance would degrade. Physical
index purging that would improve the performance may be performed with
:ref:`OPTIMIZE <optimize_index_syntax>` command.

When a search is performed, a search thread looks for matches in the RAM chunk and then on each disk chunk.
The search becomes slower as the disk has more chunks, as a single search thread is used. 
The effect increases with the number of disk chunks and it's more visible on slower storage.
Multi-threaded searching over the disk chunks can be enabled by setting :ref:`dist_threads<dist_threads>` to a value greater than one.
Depending on the value of dist_threads, a number of search threads will be launched in parallel and search over the disk chunks. 
The search on the RAM chunk is not parallelized.

Data in RAM chunk gets saved to disk on clean daemon shutdown, and then
loaded back on startup. However, on daemon or server crash, updates from
RAM chunk might be lost. To prevent that, binary logging of transactions
can be used; see the section called :ref:`binary_logging` for details.

Full-text changes in RT index are transactional. They are stored in a
per-thread accumulator until COMMIT, then applied at once. Bigger
batches per single COMMIT should result in faster indexing.

.. _binary_logging:

Binary logging
--------------

Binary logs are essentially a recovery mechanism. With binary logs
enabled, ``searchd`` writes every given transaction to the binlog file,
and uses that for recovery after an unclean shutdown. On clean shutdown,
RAM chunks are saved to disk, and then all the binlog files are
unlinked.

During normal operation, a new binlog file will be opened every time
when ``binlog_max_log_size`` limit is reached. Older, already closed
binlog files are kept until all of the transactions stored in them (from
all indexes) are flushed as a disk chunk. Setting the limit to 0 pretty
much prevents binlog from being unlinked at all while ``searchd`` is
running; however, it will still be unlinked on clean shutdown.
(``binlog_max_log_size`` defaults to 0.)

There are 3 different binlog flushing strategies, controlled by
:ref:`binlog_flush <binlog_flush>`
directive which takes the values of 0, 1, or 2. 0 means to flush the log
to OS and sync it to disk every second; 1 means flush and sync every
transaction; and 2 (the default mode) means flush every transaction but
sync every second. Sync is relatively slow because it has to perform
physical disk writes, so mode 1 is the safest (every committed
transaction is guaranteed to be written on disk) but the slowest.
Flushing log to OS prevents from data loss on ``searchd`` crashes but
not system crashes. Mode 2 is the default.

On recovery after an unclean shutdown, binlogs are replayed and all
logged transactions since the last good on-disk state are restored.
Transactions are checksummed so in case of binlog file corruption
garbage data will **not** be replayed; such a broken transaction will
be detected and, currently, will stop replay. Transactions also start
with a magic marker and timestamped, so in case of binlog damage in the
middle of the file, it's technically possible to skip broken
transactions and keep replaying from the next good one, and/or it's
possible to replay transactions until a given timestamp (point-in-time
recovery), but none of that is implemented yet.

One unwanted side effect of binlogs is that actively updating a small RT
index that fully fits into a RAM chunk part will lead to an ever-growing
binlog that can never be unlinked until clean shutdown. Binlogs are
essentially append-only deltas against the last known good saved state
on disk, and unless RAM chunk gets saved, they can not be unlinked. An
ever-growing binlog is not very good for disk use and crash recovery
time. To avoid this, you can configure ``searchd`` to perform a periodic
RAM chunk flush to fix that problem using a
:ref:`rt_flush_period <rt_flush_period>`
directive. With periodic flushes enabled, ``searchd`` will keep a
separate thread, checking whether RT indexes RAM chunks need to be
written back to disk. Once that happens, the respective binlogs can be
(and are) safely unlinked.

Note that ``rt_flush_period`` only controls the frequency at which the
*checks* happen. There are no *guarantees* that the particular RAM chunk
will get saved. For instance, it does not make sense to regularly
re-save a huge RAM chunk that only gets a few rows worth of updates. The
search daemon determine whether to actually perform the flush with a few
heuristics.
