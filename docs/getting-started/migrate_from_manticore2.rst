Migrating from Manticore or Sphinx Search 2.x
---------------------------------------------

Upgrading from 2.x to 3.x is not straight-forward because the index storage engine received a massive upgrade and the new searchd can't load existing indexes and upgrade them to new format on-the-fly.

The upgrade procedure depends on the number of servers available, but in general it's about creating the 3.x index versions, perform some tests with 3.x searchd daemon (load indexes, run typical queries)
and if everything is ok to replace 2.x binaries and index files with their 3.x counterparts.

There are no changes on how clients should connect to the engine or any change in querying mode or changing in behaviour of queries. 

Index storage engine
~~~~~~~~~~~~~~~~~~~~

Manticore Search 3 got a redesigned index storage. Indexes created with Manticore/Sphinx 2.x cannot be loaded by Manticore Search 3.

Existing indexes (RealTime or plain) can be converted with :ref:`index_converter tool<index_converter_command_reference>` or remade from scratch.
In case of RealTime indexes before converting them the memory chunk must be flushed as disk chunk using :ref:`FLUSH RAMCHUNK<flush_ramchunk_syntax>`.

Because of the 4GB limitation, a RealTime index in 2.x could still have several disk chunks after an optimize operation. After upgrading to 3.x, these indexes can now be optimized to 1-disk chunk with the usual OPTIMIZE command.

Index files also changed. The only component that is didn't undergo structural changes is the ``spp`` file (hitlists). ``sps`` (strings/json) and ``spm`` (MVA) are now hold by ``spb`` (var-length attributes).
New format has an ``spm`` file present, but it's used for row map (before it was containing the MVA attributes). New extensions added are ``spt`` (docid lookup), ``sphi`` ( secondary index histograms).
In case you are using scripts that manipulate index files, these should be adapted for the new file extensions.


Kill-list
~~~~~~~~~

Kill-lists have been redesigned in Manticore Search 3. In previous versions, the kill-lists were loaded at query time and applied on the result set provided by indexes already searched.

In 2.x the index order at query time mattered.  For example if a delta index had kill-lists in order to apply the kill-lists on main index the order had to be main, delta (either in a distributed index or in FROM clause).

In version 3 the kill-lists are applied at index loading during searchd startup. In the index configuration that has kill-lists, new directive :ref:`killlist_target<killlist_target>` will specify on which indexes the kill-lists should be applied.
The killlist_target directive points the target indexes and defines which doc ids from the source index should be used for suppression. These can be ids from a defined kill-list, the actual doc ids of the index or both.

The documents in the kill-lists are deleted from the target indexes, they are not returned in results, even if the search doesn't include the index that provide the kill-lists.
Because of this the order of indexes for searching does not matter. Now doing delta,main  or main, delta will provide same results.


Removed configuration keys
~~~~~~~~~~~~~~~~~~~~~~~~~~

- docinfo - everything is now extern
- inplace_docinfo_gap -  not needed anymore
- mva_updates_pool -  MVAs don't have anymore a dedicated pool for updates, as now they can be updated directly in the blob (see below).



Updating var-length attributes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

String, JSON and MVA attributes can now be updated using UPDATE statement.

In 2.x strings attributes required REPLACE, for JSON it was only possible to update scalar properties (as they were fixed-width) and MVAs could be updated using the MVA pool.

Now updates are performed directly on the blob component. One setting that may require tunning is :ref:`attr_update_reserve<attr_update_reserve>` -  which allow changing the allocated extra space at the end of the blob used to to avoid frequent resizes in case the new values are bigger than existing values in the blog.


Document IDs
~~~~~~~~~~~~

Doc ids used to be UNSIGNED 64-bit integers. Now they are SIGNED 64-bit integers. 
