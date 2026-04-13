# REPLACE против UPDATE

Вы можете изменить существующие данные в таблице RT или PQ, либо обновляя, либо заменяя их.

[UPDATE](../../Data_creation_and_modification/Updating_documents/UPDATE.md) заменяет [поколоночные](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) значения атрибутов существующих документов новыми значениями. Полнотекстовые поля и поколоночные атрибуты не могут быть обновлены. Если необходимо изменить содержимое полнотекстового поля или поколоночных атрибутов, используйте [REPLACE](../../Data_creation_and_modification/Updating_documents/REPLACE.md).

[REPLACE](../../Data_creation_and_modification/Updating_documents/REPLACE.md) работает аналогично [INSERT](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md), за исключением того, что если старый документ имеет тот же ID, что и новый документ, старый документ помечается как удалённый перед вставкой нового. Обратите внимание, что старый документ физически не удаляется из таблицы. Удаление происходит только при слиянии чанков в таблице, например, в результате [OPTIMIZE](../../Securing_and_compacting_a_table/Compacting_a_table.md).

### UPDATE против частичного REPLACE

Оба `UPDATE` и [частичный REPLACE](../../Data_creation_and_modification/Updating_documents/REPLACE.md?client=REPLACE+SET) могут изменить значение поля, но работают они по-разному:
- `UPDATE` может изменить только поля, которые не являются поколоночными или полнотекстовыми. Этот процесс выполняется на месте и обычно быстрее, чем `REPLACE`.
- Частичный `REPLACE` может изменить любое поле в документе, но требует, чтобы все поля в таблице были установлены как "хранимые" (хотя это и является настройкой по умолчанию). Это не требуется при использовании `UPDATE`.

<!-- proofread -->

