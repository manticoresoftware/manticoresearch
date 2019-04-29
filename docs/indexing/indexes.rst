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

Percolate indexes
~~~~~~~~~~~~~~~~~

Percolate indexes are special Real-Time indexes that store queries instead of documents. They are used for prospective searches ( or "search in reverse").
Refer to :ref:`percolate_query` for more details.


There can be as many indexes per configuration file as necessary.
``indexer`` utility can reindex either all of them (if ``--all`` option
is specified), or a certain explicitly specified subset. ``searchd``
utility will serve all the specified indexes, and the clients can
specify what indexes to search in run time.


Index files
~~~~~~~~~~~

Each index consists of a number of files.

Small index components are fully loaded into memory.
Big index components  are read from disk as needed.  Currently these use seek+read to retrieve data from disk.
Attribute components are opened and mapped with mmap(). They can be loaded fully in memory or left on disk and read when needed.

Plain indexes and RealTime indexes chunks:

+-----------+------------------------------+-----------------------------------------+
| Extension |  Stores                      | Memory management                       |
+===========+==============================+=========================================+
| spa       | scalar attrs                 | mmap(), also see :ref:`ondisk_attrs`    |
+-----------+------------------------------+-----------------------------------------+
| spd       | document lists               | read, on disk, gets cached by OS        |
+-----------+------------------------------+-----------------------------------------+
| spi       | dictionary                   | always loaded in memory                 |
+-----------+------------------------------+-----------------------------------------+
| sph       | index/chunk header           | always loaded in memory                 |
+-----------+------------------------------+-----------------------------------------+
| spk       | Kill list                    | loaded and discarded :sup:`[1]`         |
+-----------+------------------------------+-----------------------------------------+
| spl       | index lock file              | on disk only                            |
+-----------+------------------------------+-----------------------------------------+
| spm       | row map                      | mmap()                                  |
+-----------+------------------------------+-----------------------------------------+
| sphi      | secondary index histograms   | always loaded in memory                 |
+-----------+------------------------------+-----------------------------------------+
| spt       | docid lookups                | mmap()                                  |
+-----------+------------------------------+-----------------------------------------+
| spp       | keyword positions            | read, on disk, gets cached by OS        |
+-----------+------------------------------+-----------------------------------------+
| spb       | var-length attrs             | mmap(), also see :ref:`ondisk_attrs`    |
+-----------+------------------------------+-----------------------------------------+

:sup:`[1]` Kill lists -  loaded in memory at startup and discarded after they are applied to targets

RealTime indexes also have:

+-----------+---------------------------+-----------------------------------------+
| Extension |  Stores                   | Memory management                       |
+===========+===========================+=========================================+
+ kill      | RT kill :sup:`[1]`        |  on disk only                           |
+-----------+---------------------------+-----------------------------------------+
| meta      | RT header                 | always loaded in memory                 |
+-----------+---------------------------+-----------------------------------------+
| lock      | RT lock file              | on disk only                            |
+-----------+---------------------------+-----------------------------------------+
| ram       | RAM chunk copy :sup:`[2]` | on disk only                            |
+-----------+---------------------------+-----------------------------------------+


:sup:`[1]` RT kill -  documents that gets REPLACEd. Gets cleared when RAM chunk is dumped as disk chunk.

:sup:`[2]` RAM chunk copy - created when RAM chunk is flushed to disk. Cleared when RAM chunk is dumped as disk chunk.

Operations on indexes
~~~~~~~~~~~~~~~~~~~~~

Declaration
^^^^^^^^^^^

Plain indexes can only be created by **indexer** tool.
If a plain index is only declared in configuration,but not created, the daemon will print a warning about that.
It must be also noted that the daemon requires at least one index of type RT, percolate or plain in order to start.

Real-Time, percolate and template indexes can be declared in the configuration and they will be created (with empty data) at daemon start.

Loading or discarding indexes
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

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

Some settings like :ref:`mlock` and :ref:`ondisk_attrs`, which don't alter in any way the index, don't require an index rebuild, just a reload.
