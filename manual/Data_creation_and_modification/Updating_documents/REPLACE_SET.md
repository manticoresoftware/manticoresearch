# REPLACE+SET

<!-- example replace -->

`REPLACE + SET` works similarly to [INSERT](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md), but it marks the previous document with the same ID as deleted before inserting a new one.

**Warning!** This approach does not work when the table contains a text attribute without a stored property.

For the HTTP JSON protocol, these replacement types only support an Elasticsearch-like approach

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
REPLACE INTO products SET description='HUAWEI Matebook 15', price=10 WHERE id = 55;
```

<!-- response -->

```sql
Query OK, 1 row affected (0.00 sec)
```


<!-- intro -->
##### HTTP (Elasticsearch style)

<!-- request Elasticsearch -->

```json

POST /products/_update/55
{
  "description": "HUAWEI Matebook 15",
  "price": 10
}
```

<!-- response Elasticsearch -->
```json
{
"_id":55,
"_index":"products",
"_primary_term":1,
"_seq_no":0,
"_shards":{
    "failed":0,
    "successful":1,
    "total":1
},
"_type":"_doc",
"_version":1,
"result":"updated"
}
```

`REPLACE + SET` is available ony for RT tables.

When you run a `REPLACE + SET`, the previous document is not removed, but it's marked as deleted, so the table size grows until chunk merging happens, and the marked documents won't be included. To force a chunk merge, use the [OPTIMIZE statement](../../Securing_and_compacting_a_table/Compacting_a_table.md).

The syntax of the `REPLACE + SET` statement similar with [UPDATE statement syntax](../../Data_creation_and_modification/Updating_documents/UPDATE.md).

```sql
REPLACE INTO table SET [column1=value1, column2=value2, ...]
    WHERE id = [int]
```

<!-- proofread -->
