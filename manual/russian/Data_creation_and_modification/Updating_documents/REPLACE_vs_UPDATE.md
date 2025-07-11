# REPLACE vs UPDATE

You can modify existing data in an RT or PQ table by either updating or replacing it.

[UPDATE](../../Data_creation_and_modification/Updating_documents/UPDATE.md) заменяет [поколо строк](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) значения атрибутов существующих документов новыми значениями. Полнотекстовые поля и колоночные атрибуты нельзя обновлять. Если нужно изменить содержимое полнотекстового поля или колоночных атрибутов, используйте [REPLACE](../../Data_creation_and_modification/Updating_documents/REPLACE.md).

[REPLACE](../../Data_creation_and_modification/Updating_documents/REPLACE.md) работает аналогично [INSERT](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md), за исключением того, что если у старого документа такой же ID, как у нового, старый документ помечается как удалённый перед вставкой нового документа. Обратите внимание, что старый документ не удаляется физически из таблицы. Удаление может произойти только при слиянии чанков в таблице, например, в результате [OPTIMIZE](../../Securing_and_compacting_a_table/Compacting_a_table.md).

### UPDATE vs partial REPLACE

Оба `UPDATE` и [partial REPLACE](../../Data_creation_and_modification/Updating_documents/REPLACE.md?client=REPLACE+SET) могут изменить значение поля, но работают они по-разному:
- `UPDATE` может изменять только поля, которые не являются колоночными или полнотекстовыми. Этот процесс выполняется на месте, что обычно быстрее, чем `REPLACE`.
- Частичный `REPLACE` может изменить любое поле в документе, но требует, чтобы все поля в таблице были установлены как "stored" (хотя это настройка по умолчанию). Для `UPDATE` это не обязательно.

<!-- proofread -->

