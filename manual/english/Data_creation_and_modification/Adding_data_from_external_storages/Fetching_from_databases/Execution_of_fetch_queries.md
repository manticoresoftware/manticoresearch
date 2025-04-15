# Execution of fetch queries

With all the SQL drivers, building a plain table generally works as follows.

* A connection to the database is established.
* The `sql_query_pre_all` queries are executed to perform any necessary initial setup, such as setting per-connection encoding with MySQL. These queries run before the entire indexing process, and also after a reconnect for indexing MVA attributes and joined fields.
* The `sql_query_pre` pre-query is executed to perform any necessary initial setup, such as setting up temporary tables or maintaining counter tables. These queries run once for the entire indexing process.
* Pre-queries as `sql_query_pre` is executed to perform any necessary initial setup, such as setting up temporary
  tables, or maintaining counter table. These queries run once per whole indexing.
* Main query as `sql_query` is executed and the rows it returns are processed.
* Post-query as `sql_query_post` is executed to perform some necessary cleanup.
* The connection to the database is closed.
* Indexer does the sorting phase (to be pedantic, table-type specific post-processing).
* A connection to the database is established again.
* Post-processing query as `sql_query_post_index` is executed to perform some necessary final cleanup.
* The connection to the database is closed again.

Example of a source fetching data from MYSQL:

```ini
source mysource {
  type             = mysql
  path             = /path/to/realtime
  sql_host         = localhost
  sql_user         = myuser
  sql_pass         = mypass
  sql_db           = mydb
  sql_query_pre    = SET CHARACTER_SET_RESULTS=utf8
  sql_query_pre    = SET NAMES utf8
  sql_query        =  SELECT id, title, description, category_id  FROM mytable
  sql_query_post   = DROP TABLE view_table
  sql_query_post_index = REPLACE INTO counters ( id, val ) \
    VALUES ( 'max_indexed_id', $maxid )
  sql_attr_uint    = category_id
  sql_field_string = title
 }

table mytable {
  type   = plain
  source = mysource
  path   = /path/to/mytable
  ...
 }
```

## sql_query

This is the query used to retrieve documents from a SQL server. There can be only one sql_query declared, and it's mandatory to have one. See also [Processing fetched data](../../../Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Processing_fetched_data.md#Processing-fetched-data)

## sql_query_pre

Pre-fetch query or pre-query. This is a multi-value, optional setting, with the default being an empty list of queries. The pre-queries are executed before the sql_query in the order they appear in the configuration file. The results of the pre-queries are ignored.

Pre-queries are useful in many ways. They can be used to set up encoding, mark records that are going to be indexed, update internal counters, set various per-connection SQL server options and variables, and so on.

Perhaps the most frequent use of pre-query is to specify the encoding that the server will use for the rows it returns. Note that Manticore accepts only UTF-8 text. Two MySQL specific examples of setting the encoding are:

```ini
sql_query_pre = SET CHARACTER_SET_RESULTS=utf8
sql_query_pre = SET NAMES utf8
```

Also, specific to MySQL sources, it is useful to disable query cache (for indexer connection only) in pre-query, because indexing queries are not going to be re-run frequently anyway, and there's no sense in caching their results.
That could be achieved with:

```ini
sql_query_pre = SET SESSION query_cache_type=OFF
```

## sql_query_post

Post-fetch query. This is an optional setting, with the default value being empty.

This query is executed immediately after sql_query completes successfully. When the post-fetch query produces errors, they are reported as warnings, but indexing is not terminated. Its result set is ignored. Note that indexing is not yet completed at the point when this query gets executed, and further indexing may still fail. Therefore, any permanent updates should not be done from here. For instance, updates on a helper table that permanently change the last successfully indexed ID should not be run from the `sql_query_post` query; they should be run from the `sql_query_post_index` query instead.

## sql_query_post_index

Post-processing query. This is an optional setting, with the default value being empty.

This query is executed when indexing is fully and successfully completed. If this query produces errors, they are reported as warnings, but indexing is not terminated. Its result set is ignored. The `$maxid` macro can be used in its text; it will be expanded to the maximum document ID that was actually fetched from the database during indexing. If no documents were indexed, `$maxid` will be expanded to 0.

Example:
```ini
sql_query_post_index = REPLACE INTO counters ( id, val ) \
    VALUES ( 'max_indexed_id', $maxid )
```

The difference between `sql_query_post` and `sql_query_post_index` is that `sql_query_post` is run immediately when Manticore receives all the documents, but further indexing may still fail for some other reason. On the contrary, by the time the `sql_query_post_index` query gets executed, it is guaranteed that the table was created successfully. Database connection is dropped and re-established because sorting phase can be very lengthy and would just time out otherwise.

<!-- proofread -->
