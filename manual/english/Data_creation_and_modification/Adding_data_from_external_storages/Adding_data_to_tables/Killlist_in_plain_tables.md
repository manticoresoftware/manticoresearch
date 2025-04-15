# Killlist in plain tables

When using plain tables, there's a challenge arising from the need to have the data in the table as fresh as possible.

In this case, one or more secondary (also known as delta) tables are used to capture the modified data between the time the main table was created and the current time. The modified data can include new, updated, or deleted documents. The search becomes a search over the **main table and the delta table**. This works seamlessly when you just add **new** documents to the delta table, but when it comes to updated or deleted documents, there remains the following issue.

If a document is present in both the main and delta tables, it can cause issues during searching, as the engine will see two versions of a document and won't know how to pick the right one. So, the delta needs to somehow inform the search that there are deleted documents in the main table that should be disregarded. **This is where kill lists come in**.

## Table kill-list
A table can maintain a list of document IDs that can be used to suppress records in other tables. This feature is available for plain tables using database sources or plain tables using XML sources. In the case of database sources, the source needs to provide an additional query defined by `sql_query_killlist`. It will store in the table a list of documents that can be used by the server to remove documents from other plain tables.

This query is expected to return a number of 1-column rows, each containing just the document ID.

In many cases, the query is a union between a query that retrieves a list of updated documents and a list of deleted documents, e.g.:

```ini
sql_query_killlist = \
    SELECT id FROM documents WHERE updated_ts>=@last_reindex UNION \
    SELECT id FROM documents_deleted WHERE deleted_ts>=@last_reindex
```

## Removing documents in a plain table
A plain table can contain a directive called [killlist_target](../../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#killlist_target) that will tell the server it can provide a list of document IDs that should be removed from certain existing tables. The table can use either its document IDs as the source for this list or provide a separate [list](../../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md#Table-kill-list).

### killlist_target

<!-- example killlist_target 1 -->
Sets the table(s) that the kill-list will be applied to. Optional, default value is empty.

When you use [plain tables](../../../Creating_a_table/Local_tables/Plain_table.md) you often need to maintain not just a single table, but a set of them to be able to add/update/delete new documents sooner (read about [delta table updates](../../../Data_creation_and_modification/Adding_data_from_external_storages/Main_delta.md)). n order to suppress matches in the previous (**main**) table that were updated or deleted in the next (**delta**) table, you need to:

1.  Create a kill-list in the **delta** table using [sql_query_killlist](../../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md#Table-kill-list)
2.  Specify **main** table as `killlist_target` in **delta** table settings:


<!-- intro -->
##### CONFIG:

<!-- request CONFIG -->

```ini
table products {
  killlist_target = main:kl

  path = products
  source = src_base
}
```
<!-- end -->

When `killlist_target`  is specified, the kill-list is applied to all the tables listed in it on `searchd` startup. If any of the tables from `killlist_target` are rotated, the kill-list is reapplied to these tables. When the kill-list is applied, tables that were affected save these changes to disk.

`killlist_target` has 3 modes of operation:

1.  `killlist_target = main:kl`. Document IDs from the kill-list of the **delta** table are suppressed in the **main** table (see `sql_query_killlist`).
2.  `killlist_target = main:id`. All document IDs from the **delta** table are suppressed in the **main** table. The kill-list is ignored.
3.  `killlist_target = main`. Both document IDs from the **delta** table and its kill-list are suppressed in the **main** table.

Multiple targets can be specified, separated by commas like:

```ini
killlist_target = table_one:kl,table_two:kl
```

<!-- example killlist_target 2 -->
You can change the `killlist_target`  settings for a table without rebuilding it by using `ALTER`.

However, since the 'old' main table has already written the changes to disk, the documents that were deleted in it will **remain** deleted even if it is no longer in the `killlist_target` of the **delta** table.


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
ALTER TABLE delta KILLLIST_TARGET='new_main_table:kl'
```

<!-- request HTTP -->

```http
POST /cli -d "
ALTER TABLE delta KILLLIST_TARGET='new_main_table:kl'"
```
<!-- end -->
<!-- proofread -->