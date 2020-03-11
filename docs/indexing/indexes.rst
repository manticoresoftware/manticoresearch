.. _indexes:

Indexes
=======

To be able to answer full-text search queries fast, Manticore needs to
build a special data structure optimized for such queries from your text
data. This structure is called *index*; and the process of building
index from text is called *indexing*.

An index identifier must be a single word, that can contain letters, numbers and underscores. It must start with a letter.

Different index types are well suited for different tasks. For example,
a disk-based tree-based index would be easy to update (ie. insert new
documents to existing index), but rather slow to search. Manticore
architecture allows internally for different *index types*, or
*backends*, to be implemented comparatively easily.

Manticore provides 2 different backends: a **disk index** backend, and a
**RT (realtime) index** backend.

.. _plain_indexes:

Plain indexes
~~~~~~~~~~~~~~~~~~~~~

**Disk indexes** are designed to provide maximum indexing and
searching speed, while keeping the RAM footprint as low as possible.
That comes at a cost of text index updates. You can not update an
existing document or incrementally add a new document to a disk index.
You only can batch rebuild the entire disk index from scratch. (Note
that you still can update document's **attributes** on the fly, even
with the disk indexes.)

This “rebuild only” limitation might look as a big constraint at a first
glance. But in reality, it can very frequently be worked around rather
easily by setting up multiple disk indexes, searching through them all,
and only rebuilding the one with a fraction of the most recently changed
data. See :ref:`live_index_updates` for details.

Plain indexes are available only in :ref:`plain_mode` of searchd.

Real-Time indexes
~~~~~~~~~~~~~~~~~

**RT indexes** enable you to implement dynamic updates and
incremental additions to the full text index. RT stands for Real Time
and they are indeed “soft realtime” in terms of writes, meaning that
most index changes become available for searching as quick as 1
millisecond or less, but could occasionally stall for seconds. (Searches
will still work even during that occasional writing stall.) Refer to
:ref:`real-time_indexes`
for details.

Distributed indexes
~~~~~~~~~~~~~~~~~~~

Manticore supports so-called **distributed
indexes**. Compared to disk and RT indexes, those are not a real
physical backend, but rather just lists of either local or remote
indexes that can be searched transparently to the application, with
Manticore doing all the chores of sending search requests to remote
machines in the cluster, aggregating the result sets, retrying the
failed requests, and even doing some load balancing. See :ref:`distributed_searching` for a
discussion of distributed indexes.

Templates indexes
~~~~~~~~~~~~~~~~~

Template indexes are indexes with no storage backend. They can be used operations that involve only data from input, like keywords and snippets generation.
Available only in :ref:`plain_mode`.

Percolate indexes
~~~~~~~~~~~~~~~~~

Percolate indexes are special Real-Time indexes that store queries instead of documents. They are used for prospective searches ( or "search in reverse").
Refer to :ref:`percolate_query` for more details.


There can be as many indexes per configuration file as necessary.
``indexer`` utility can reindex either all of them (if ``--all`` option
is specified), or a certain explicitly specified subset. ``searchd``
utility will serve all the specified indexes, and the clients can
specify what indexes to search in run time.

.. _index_files:

Index files
~~~~~~~~~~~

Each index consists of a number of files.

Small index components are fully loaded into memory.
Big index components  are read from disk as needed. Currently these use seek+read or mmap() to retrieve data from disk.
Attribute components are opened and mapped with mmap(). They can be loaded fully in memory or left on disk and read when needed.

Plain indexes and RealTime indexes chunks:

+-----------+------------------------------+--------------------------------------------+
| Extension |  Stores                      | Memory management                          |
+===========+==============================+============================================+
| spa       | scalar attrs                 | mmap(), also see :ref:`access_plain_attrs` |
+-----------+------------------------------+--------------------------------------------+
| spd       | document lists               | read from disk and may be cached by OS     |
|           |                              | or mmaped, also see :ref:`access_doclists` |
+-----------+------------------------------+--------------------------------------------+
| spi       | dictionary                   | always loaded in memory                    |
+-----------+------------------------------+--------------------------------------------+
| sph       | index/chunk header           | always loaded in memory                    |
+-----------+------------------------------+--------------------------------------------+
| spk       | kill list                    | loaded and discarded :sup:`[1]`            |
+-----------+------------------------------+--------------------------------------------+
| spl       | index lock file              | on disk only                               |
+-----------+------------------------------+--------------------------------------------+
| spm       | row map                      | mmap()                                     |
+-----------+------------------------------+--------------------------------------------+
| sphi      | secondary index histograms   | always loaded in memory                    |
+-----------+------------------------------+--------------------------------------------+
| spt       | docid lookups                | mmap()                                     |
+-----------+------------------------------+--------------------------------------------+
| spp       | keyword positions            | read from disk and may be cached by OS     |
|           |                              | or mmaped, also see :ref:`access_hitlists` |
+-----------+------------------------------+--------------------------------------------+
| spb       | var-length attrs             | mmap(), also see :ref:`access_blob_attrs`  |
+-----------+------------------------------+--------------------------------------------+
| spds      | documents                    | read from disk, also see                   |
|           |                              | :ref:`docstore_cache_size`                 |
+-----------+------------------------------+--------------------------------------------+


:sup:`[1]` Kill lists -  loaded in memory at startup and discarded after they are applied to targets

RealTime indexes also have:

+-----------+---------------------------+-----------------------------------------+
| Extension |  Stores                   | Memory management                       |
+===========+===========================+=========================================+
+ kill      | RT kill :sup:`[1]`        | on disk only                            |
+-----------+---------------------------+-----------------------------------------+
| meta      | RT header                 | always loaded in memory                 |
+-----------+---------------------------+-----------------------------------------+
| lock      | RT lock file              | on disk only                            |
+-----------+---------------------------+-----------------------------------------+
| ram       | RAM chunk copy :sup:`[2]` | on disk only                            |
+-----------+---------------------------+-----------------------------------------+


:sup:`[1]` RT kill -  documents that are REPLACEd get cleared when the RAM chunk is dumped as a disk chunk.

:sup:`[2]` RAM chunk copy - created when the RAM chunk is flushed to disk. Cleared when the RAM chunk is dumped as a disk chunk.


.. _index_files_access:

Accessing index files
~~~~~~~~~~~~~~~~~~~~~

The daemon uses two access modes to read index data - seek+read and mmap.

In seek+read mode the daemon performs system call pread(2) to read document
lists and keyword positions, i.e. ``spd`` and ``spp`` files. Internal read buffers are
used to optimize reading. The size of these buffers can be tuned with options :ref:`read_buffer_docs`
and :ref:`read_buffer_hits`. There is also :ref:`preopen` option that allows to control
the amount of files opened by daemon at start.

In mmap access mode the search daemon just maps index's file into memory with
mmap(2) system call and OS caches file content by itself. Options
:ref:`read_buffer_docs` and :ref:`read_buffer_hits` have no effect for corresponding
files in this mode. This reader could be used for scalar (int, float, boolean, timestamp)
attributes, var-length (string, mva, json) attributes, document lists and keyword
positions, i.e. ``spa``, ``spb``, ``spd`` and ``spp`` files.

The ``mmap`` reader can also lock index's data in memory via mlock(2) privileged call which prevents swapping out
of the cached data to disk by OS.

To control what access mode will be used :ref:`access_plain_attrs`, :ref:`access_blob_attrs`,
:ref:`access_doclists` and :ref:`access_hitlists` options are available with the following values:

* ``file`` daemon reads index file from disk with seek+read using internal buffers on file access
* ``mmap`` daemon maps index file into memory and OS caches up its contents on file access
* ``mmap_preread`` daemon maps index file into memory and a background thread reads it once to warm up the cache
* ``mlock`` daemon maps index file into memory and then issues mlock system call to cache up the file contents and lock it into memory to prevent it being swapped out

Here is a table which can help you select your desired mode:

+-------------------------+-----------------------------------+-----------------------------------------+----------------------------------------------+----------------------------+
| index part              | keep it on disk                   | keep it in memory                       | cached in memory on daemon start             | lock it in memory          |
+-------------------------+-----------------------------------+-----------------------------------------+----------------------------------------------+----------------------------+
| .spa (plain attributes) | access_plain_attrs=mmap - the file will be mapped to RAM, but your OS will  | access_plain_attrs = mmap_preread (default)  | access_plain_attrs = mlock |
| .spe (skip lists)       | decide whether to really load it to RAM or not and can easily swap it       |                                              |                            |
| .spi (word lists)       | out (default)                                                               |                                              |                            |
| .spt (lookups)          |                                                                             |                                              |                            |
| .spm (killed docs)      |                                                                             |                                              |                            |
+-------------------------+-----------------------------------+-----------------------------------------+----------------------------------------------+----------------------------+
| .spb (blob attributes)  | access_blob_attrs=mmap - the file will be mapped to RAM, but your OS will   | access_blob_attrs = mmap_preread (default)   | access_blob_attrs = mlock  |
| (string, mva and json   | decide whether to really load it to RAM or not and can easily swap it       |                                              |                            |
| attributes)             | out (default)                                                               |                                              |                            |
+-------------------------+-----------------------------------+-----------------------------------------+----------------------------------------------+----------------------------+
| .spd (doc lists)        | access_doclists = file (default)  | access_doclists = mmap, may be still    | no                                           | access_doclists = mlock    |
|                         |                                   | swapped out by OS                       |                                              |                            |
+-------------------------+-----------------------------------+-----------------------------------------+----------------------------------------------+----------------------------+
| .spp (hit lists)        | access_hitlists = file (default)  | access_hitlists = mmap, may be still    | no                                           | access_hitlists = mlock    |
|                         |                                   | swapped out by OS                       |                                              |                            |
+-------------------------+-----------------------------------+-----------------------------------------+----------------------------------------------+----------------------------+

, , , 

There's also a searchd command line option ``--force-preread`` that instructs the
daemon to wait until the attribute files are read prior to starting accepting incoming connections.
Starting daemon with this option allows to make sure that all mapped index files will be 'warmed'
by touching every page of the maps. OS, in turn, usually tries to keep all 'warm' data in the disk cache,
providing maximum performance.

The recommendations are:

* If search performance is very important and you have enough memory - use mlock for attributes and mmap for doclists/hitlists. Be aware mlock is a privileged system call and the user running searchd should have enough privileges. Read :ref:`here<mlock>` for details
* If you can't afford lower performance on start and ready to wait longer on start until it's warmed up - use --force-preread
* If you want searchd to be able to restart faster - stay with mmap_preread
* If you want to save RAM - do not use mlock, then your OS will decide what should be in memory at any given moment of time depending on what is read from disk more frequently
* If search performance doesn't matter at all and you want to save maximum RAM - use access_doclists/access_hitlists=file and access_plain_attrs/access_blob_attrs=mmap

The default mode is to mmap and pre-read attributes and access doclists/hitlists directly
from disk which provides decent search performance, optimal memory usage and faster
searchd restart in most cases.

Operations on indexes
~~~~~~~~~~~~~~~~~~~~~

In :ref:`rt_mode` supported indexes (RT,PQ and distributed) can be created/dropped with :ref:`create_table_syntax` / :ref:`drop_table_syntax`.

In :ref:`plain_mode` Real-Time, percolate and template indexes can be declared in the configuration and they will be created (with empty data) at daemon start.

Plain indexes can only be created by **indexer** tool.
If a plain index is only declared in configuration,but not created, the daemon will print a warning about that.
It must be also noted that the daemon requires at least one index of type RT, percolate or plain in order to start.


Loading or discarding indexes
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

This section is for :ref:`plain_mode`.

At startup, daemon will try to load and make available all indexes found in the configuration file.

HUP signal can be used to make the daemon reload the configuration. This way new indexes can be loaded or existing indexes can be discarded while the daemon is running.
Changing the type of an index, for example from template to Real-Time, can also be performed during a configuration reload.

Alternative to signaling HUP to searchd daemon, the :ref:`RELOAD INDEXES<reload_indexes_syntax>` SphinxQL command can be used.

Refreshing a plain index already loaded by daemon requires running *indexer* with *--rotate* parameter.
In this case, a new version of the plain index is created and when ready, a HUP is send to daemon, which will load the new version of the index in the memory and discard the old one.

Index changes
^^^^^^^^^^^^^
Index schema can be changed on-the-fly in case of attribute. Full-text fields however require re-creating the index.

Change of tokenization settings requires a remaking in case of plain indexes. For Real-Time indexes, these can be made on-the-fly using
:ref:`ALTER RECONFIGURE<alter_syntax>` but they will affect only new content added to index, as it's not possible yet to re-tokenize already indexed texts.

Some settings like :ref:`access_plain_attrs`, which don't alter in any way the index, don't require an index rebuild, just a reload.
