.. _index_merging:

Index merging
=======================

Merging two existing indexes can be more efficient than indexing the
data from scratch, and desired in some cases (such as merging ‘main’ and
‘delta’ indexes instead of simply reindexing ‘main’ in ‘main+delta’
partitioning scheme). So ``indexer`` has an option to do that. Merging
the indexes is normally faster than reindexing but still *not* instant
on huge indexes. Basically, it will need to read the contents of both
indexes once and write the result once. Merging 100 GB and 1 GB index,
for example, will result in 202 GB of IO (but that's still likely less
than the indexing from scratch requires).

The basic command syntax is as follows:

::


    indexer --merge DSTINDEX SRCINDEX [--rotate]

Only the DSTINDEX index will be affected: the contents of SRCINDEX will
be merged into it. ``--rotate`` switch will be required if DSTINDEX is
already being served by ``searchd``. The initially devised usage pattern
is to merge a smaller update from SRCINDEX into DSTINDEX. Thus, when
merging the attributes, values from SRCINDEX will win if duplicate
document IDs are encountered. Note, however, that the “old” keywords
will *not* be automatically removed in such cases. For example, if
there's a keyword “old” associated with document 123 in DSTINDEX, and a
keyword “new” associated with it in SRCINDEX, document 123 will be found
by *both* keywords after the merge. You can supply an explicit condition
to remove documents from DSTINDEX to mitigate that; the relevant switch
is ``--merge-dst-range``:

::


    indexer --merge main delta --merge-dst-range deleted 0 0

This switch lets you apply filters to the destination index along with
merging. There can be several filters; all of their conditions must be
met in order to include the document in the resulting merged index. In
the example above, the filter passes only those records where ‘deleted’
is 0, eliminating all records that were flagged as deleted (for
instance, using
:ref:`UpdateAttributes() <update_attributes>`
call).
