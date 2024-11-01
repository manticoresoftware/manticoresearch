# REPLACE vs UPDATE

你可以通过更新或替换的方式来修改实时表（RT）或渗透表（PQ）中的现有数据。

[UPDATE](../../Data_creation_and_modification/Updating_documents/UPDATE.md) 用新值替换现有文档的 [行式存储](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) 属性值。全文字段和列式属性不能被更新。如果需要更改全文字段或列式属性的内容，请使用 [REPLACE](../../Data_creation_and_modification/Updating_documents/REPLACE.md)。

[REPLACE](../../Data_creation_and_modification/Updating_documents/REPLACE.md) 的工作方式与 [INSERT](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md) 类似，不同之处在于，如果旧文档与新文档具有相同的 ID，则旧文档会在插入新文档之前被标记为已删除。注意，旧文档不会从表中被物理删除。删除操作只有在表中的块合并时发生，例如通过 [OPTIMIZE](../../Securing_and_compacting_a_table/Compacting_a_table.md) 进行合并。

### UPDATE vs 部分 REPLACE

`UPDATE` 和 [部分 REPLACE](../../Data_creation_and_modification/Updating_documents/REPLACE.md?client=REPLACE+SET) 都可以更改字段的值，但它们的操作方式不同：

- `UPDATE` 只能更改非列式和非全文字段。此过程是就地完成的，通常比 `REPLACE` 更快。
- 部分 `REPLACE` 可以更改文档中的任何字段，但要求表中的所有字段都设置为“已存储”（这是默认设置）。而 `UPDATE` 则不需要这种设置。

<!-- proofread -->
