# Main+delta schema

<!-- example maindelta -->
In many situations, the total dataset is too large to be frequently rebuilt from scratch, while the number of new records remains relatively small. For example, a forum may have 1,000,000 archived posts but only receive 1,000 new posts per day.

In such cases, implementing "live" (nearly real-time) table updates can be achieved using a "main+delta" scheme.

The concept involves setting up two sources and two tables, with one "main" table for data that rarely changes (if ever), and one "delta" table for new documents. In the example, the 1,000,000 archived posts would be stored in the main table, while the 1,000 new daily posts would be placed in the delta table. The delta table can then be rebuilt frequently, making the documents available for searching within seconds or minutes. Determining which documents belong to which table and rebuilding the main table can be fully automated. One approach is to create a counter table that tracks the ID used to split the documents and update it whenever the main table is rebuilt.

Using a timestamp column as the split variable is more effective than using the ID since timestamps can track not only new documents but also modified ones.

For datasets that may contain modified or deleted documents, the delta table should provide a list of affected documents, ensuring they are suppressed and excluded from search queries. This is accomplished using a feature called Kill Lists. The document IDs to be killed can be specified in an auxiliary query defined by [sql_query_killlist](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md#Table-kill-list). The delta table must indicate the target tables for which the kill lists will be applied using the [killlist_target](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md#killlist_target) directive. The impact of kill lists is permanent on the target table, meaning that even if a search is performed without the delta table, the suppressed documents will not appear in the search results.

Notice how we're overriding `sql_query_pre` in the delta source. We must explicitly include this override. If we don't, the `REPLACE` query would be executed during the delta source's build as well, effectively rendering it useless.

<!-- request Example -->
```ini
# in MySQL
CREATE TABLE deltabreaker (
  index_name VARCHAR(50) NOT NULL,
  created_at TIMESTAMP NOT NULL  DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (index_name)
);

# in manticore.conf
source main {
  ...
  sql_query_pre = REPLACE INTO deltabreaker SET index_name = 'main', created_at = NOW()
  sql_query =  SELECT id, title, UNIX_TIMESTAMP(updated_at) AS updated FROM documents WHERE deleted=0 AND  updated_at  >=FROM_UNIXTIME($start) AND updated_at  <=FROM_UNIXTIME($end)
  sql_query_range  = SELECT ( SELECT UNIX_TIMESTAMP(MIN(updated_at)) FROM documents) min, ( SELECT UNIX_TIMESTAMP(created_at)-1 FROM deltabreaker WHERE index_name='main') max
  sql_query_post_index = REPLACE INTO deltabreaker set index_name = 'delta', created_at = (SELECT created_at FROM deltabreaker t WHERE index_name='main')
  ...
  sql_attr_timestamp = updated
}

source delta : main {
  sql_query_pre =
  sql_query_range = SELECT ( SELECT UNIX_TIMESTAMP(created_at) FROM deltabreaker WHERE index_name='delta') min, UNIX_TIMESTAMP() max
  sql_query_killlist = SELECT id FROM documents WHERE updated_at >=  (SELECT created_at FROM deltabreaker WHERE index_name='delta')
}

table main {
  path = /var/lib/manticore/main
  source = main
}

table delta {
  path = /var/lib/manticore/delta
  source = delta
  killlist_target = main:kl
}
```

<!-- end -->


<!-- proofread -->
