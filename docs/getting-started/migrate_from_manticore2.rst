Migrating from Manticore or Sphinx Search 2.x
---------------------------------------------

Upgrading from 2.x to 3.x is not straightforward, because the index storage engine received a massive upgrade and the new searchd can't load older indexes and upgrade them to new format on-the-fly.

The upgrade procedure may differ depending on your setup (number of servers in the cluster,
whether you have HA or not etc.), but in general it's about creating new 3.x index versions
and replacing your existing ones with them along with replacing older 2.x binaries with the new ones.

There have been no changes made on how clients should connect to the engine or any change
in querying mode or queries behavior.

Index storage engine
~~~~~~~~~~~~~~~~~~~~

Manticore Search 3 got a redesigned index storage. Indexes created with Manticore/Sphinx 2.x cannot be loaded by Manticore Search 3.

Existing indexes (RealTime or plain) can be converted with :ref:`index_converter tool<index_converter_command_reference>` or rebuilt from scratch.
In case of RealTime indexes before converting one you need to flush it's memory chunk to disk using :ref:`FLUSH RAMCHUNK<flush_ramchunk_syntax>`.

Because of the 4GB limitation, a RealTime index in 2.x could still have several disk chunks after an optimize operation. After upgrading to 3.x, these indexes can now be optimized to 1-disk chunk with the usual OPTIMIZE command.

Index files also changed. The only component that didn't get any structural changes is the ``spp`` file (hitlists). ``sps`` (strings/json) and ``spm`` (MVA) are now held by ``spb`` (var-length attributes).
The new format has an ``spm`` file present, but it's used for row map (previously it was dedicated for MVA attributes). The new extensions added are ``spt`` (docid lookup), ``sphi`` ( secondary index histograms).
In case you are using scripts that manipulate index files, they should be adapted for the new file extensions.

Kill-list
~~~~~~~~~

Kill-lists have been redesigned in Manticore Search 3. In previous versions kill-lists were applied on the result set provided by each previous searched index on query time.

Thus In 2.x the index order at query time mattered. For example if a delta index had a kill-list in order to apply it against the main index the order had to be `main, delta` (either in a distributed index or in the FROM clause).

In Manticore 3 kill-lists are applied to an index when it's loaded during searchd startup
or gets rotated. New directive :ref:`killlist_target<killlist_target>` in index configuration
specifies target indexes and defines which doc ids from the source index should be used
for suppression. These can be ids from the defined kill-list, actual doc ids of the index or the both.

Documents from the kill-lists are deleted from the target indexes, they are not returned in results even if the search doesn't include the index that provided the kill-lists.
Because of that the order of indexes for searching does not matter any more. Now `delta,main` and `main,delta` will provide the same results.

In previous versions indexes were rotated following the order from the configuration file. In Manticore 3 index rotation order is much smarter and works in accordance with killlist targets.
Before starting to rotate indexes the daemon looks for chains of indexes by killlist_target definitions. It will then first rotate indexes not referenced anywhere as kill-lists targets.
Next it will rotate indexes targeted by already rotated indexes and so on.
For example if we do ``indexer --all`` and we have 3 indexes : main, delta_big (which targets at the main) and delta_small (with target at delta_big), first the delta_small is rotated, then delta_big and finally the main.
This is to ensure that when a dependent index is rotated it gets the most actual kill-list from other indexes.

Removed configuration keys
~~~~~~~~~~~~~~~~~~~~~~~~~~

- docinfo - everything is now extern
- inplace_docinfo_gap -  not needed anymore
- mva_updates_pool -  MVAs don't have anymore a dedicated pool for updates, as now they can be updated directly in the blob (see below).

Updating var-length attributes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

String, JSON and MVA attributes can now be updated using UPDATE statement.

In 2.x string attributes required REPLACE, for JSON it was only possible to update scalar properties (as they were fixed-width) and MVAs could be updated using the MVA pool.
Now updates are performed directly on the blob component. One setting that may require tuning is :ref:`attr_update_reserve<attr_update_reserve>` which allows changing the allocated extra space at the end of the blob used to avoid frequent resizes in case the new values are bigger than the existing values in the blob.

Document IDs
~~~~~~~~~~~~

Doc ids used to be UNSIGNED 64-bit integers. Now they are SIGNED 64-bit integers.
