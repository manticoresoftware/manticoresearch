# Isolation during flushing and merging

Manticore provides isolation during the flushing and merging process of a real-time table to prevent any changes from affecting running queries.

For example, during table compaction, a pair of disk chunks are merged and a new chunk is produced. At one point, a new version of the table is created with the new chunk replacing the original pair. This is done seamlessly so that a long-running query using the original chunks will continue to see the old version of the table, while a new query will see the new version with the resulting merged chunk.

The same applies to flushing a RAM chunk, where suitable RAM segments are merged into a new disk chunk and the participated RAM chunk segments are abandoned. During this operation, Manticore provides isolation for queries that started before the operation began.

Furthermore, these operations are transparent for replaces and updates. If you update an attribute in a document that belongs to a disk chunk being merged with another one, the update will be applied to both that chunk and the resulting merged chunk. If you delete a document during a merge, it will be deleted in the original chunk and also in the resulting merged chunk, which will either have the document marked as deleted or have no such document at all if the deletion happened early in the merging process.
<!-- proofread -->