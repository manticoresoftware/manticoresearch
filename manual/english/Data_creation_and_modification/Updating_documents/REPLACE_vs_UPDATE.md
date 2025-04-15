# REPLACE vs UPDATE

You can modify existing data in an RT or PQ table by either updating or replacing it.

[UPDATE](../../Data_creation_and_modification/Updating_documents/UPDATE.md) replaces [row-wise](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) attribute values of existing documents with new values. Full-text fields and columnar attributes cannot be updated. If you need to change the content of a full-text field or columnar attributes, use [REPLACE](../../Data_creation_and_modification/Updating_documents/REPLACE.md).

[REPLACE](../../Data_creation_and_modification/Updating_documents/REPLACE.md) works similarly to [INSERT](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md) except that if an old document has the same ID as the new document, the old document is marked as deleted before the new document is inserted. Note that the old document does not get physically deleted from the table. The deletion can only happen when chunks are merged in a table, e.g., as a result of an [OPTIMIZE](../../Securing_and_compacting_a_table/Compacting_a_table.md).

### UPDATE vs partial REPLACE

Both `UPDATE` and a [partial REPLACE](../../Data_creation_and_modification/Updating_documents/REPLACE.md?client=REPLACE+SET) can change the value of a field, but they operate differently:
- `UPDATE` can only change fields that are neither columnar nor full-text. This process is done in-place, which is typically faster than `REPLACE`.
- A partial `REPLACE` can change any field in a document, but it requires that all fields in the table be set to "stored" (though this is the default setting). This is not necessary with `UPDATE`.

<!-- proofread -->
