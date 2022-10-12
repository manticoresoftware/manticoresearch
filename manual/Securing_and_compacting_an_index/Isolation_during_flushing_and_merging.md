# Isolation during flushing and merging

When flushing and compacting a real-time table Manticore provides isolation, so that a changed state doesn't affect the queries that were running when this or that operation started.

For instance, while compacting a table we have a pair of disk chunks that are being merged and also a new chunk produced by merging those two. Then, at one moment we create a new version of the index, where instead of the original pair of chunks the new one is placed. That is done seamlessly, so that if there's a long-running query using the original chunks, it will continue seeing the old version of the index while a new query will see the new version with the resulting merged chunk.

Same is true for flushing a RAM chunk: we merge all suitable RAM segments into a new disk chunk, and finally put a new disk chunk into the set of disk chunks and abandon the participated RAM chunk segments. During this operation, Manticore also provides isolation for those queries that started before the operation began.

Moreover, these operations are also transparent for replaces and updates. If you update an attribute in a document which belongs to a disk chunk which is being merged with another one, the update will be applied both to that chunk and to the resulting chunk after the merge. If you delete a document during a merge - it will be deleted in the original chunk and also the resulting merged chunk will either have the document marked deleted, or it will have no such document at all (if the deletion happened on early stage of the merging).
