# REPLACE 与 UPDATE

您可以通过更新或替换来修改 RT 或 PQ 表中的现有数据。

[UPDATE](../../Data_creation_and_modification/Updating_documents/UPDATE.md) 会用新值替换现有文档的[按行](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)属性值。全文字段和列式属性不能被更新。如果您需要更改全文字段或列式属性的内容，请使用 [REPLACE](../../Data_creation_and_modification/Updating_documents/REPLACE.md)。

[REPLACE](../../Data_creation_and_modification/Updating_documents/REPLACE.md) 的工作方式类似于 [INSERT](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md)，但如果旧文档与新文档具有相同的 ID，则在插入新文档之前，旧文档会被标记为已删除。请注意，旧文档不会从表中物理删除。删除只能在表的块合并时发生，例如，作为 [OPTIMIZE](../../Securing_and_compacting_a_table/Compacting_a_table.md) 的结果。

### UPDATE 与部分 REPLACE

`UPDATE` 和[部分 REPLACE](../../Data_creation_and_modification/Updating_documents/REPLACE.md?client=REPLACE+SET) 都可以更改字段的值，但它们的操作方式不同：
- `UPDATE` 只能更改既非列式也非全文的字段。此过程是在原地完成的，通常比 `REPLACE` 更快。
- 部分 `REPLACE` 可以更改文档中的任何字段，但要求表中的所有字段都设置为“存储”（尽管这是默认设置）。`UPDATE` 则不需要此要求。

<!-- proofread -->

