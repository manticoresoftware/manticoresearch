# Execution of fetch queries

With all the SQL drivers, indexing generally works as follows.

* connection to the database is established;
* pre query as `sql_query_pre`  is   executed to perform any necessary initial setup, such as setting per-connection encoding with MySQL;
* main query as `sql_query` is executed and the rows it returns are indexed;
* post-query as `sql_query_post` is executed to perform any necessary cleanup;
* connection to the database is closed;
* indexer does the sorting phase (to be pedantic, index-type specific post-processing);
* connection to the database is established again;
* post-index query  as `sql_query_post_index` is executed to perform any necessary final cleanup;
* connection to the database is closed again.

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

index myindex {
  type   = plain
  source = mysource
  path   = /path/to/myindex
  ...
 }
```

## sql_query

This is the query which is used to retrieve documents from SQL server. There can be only one sql_query declared and it's mandatory to have one.
See also [Indexing fetched data](../../Adding_data_from_external_storages/Fetching_from_databases/Indexing_fetched_data.md#Indexing-fetched-data)

## sql_query_pre

Pre-fetch query, or pre-query. Multi-value, optional, default is empty list of queries. They are executed before the sql_query exactly in order of appearance in the configuration file. Pre-query results are ignored.

Pre-queries are useful in a lot of ways. They are used to setup encoding, mark records that are going to be indexed, update internal counters, set various per-connection SQL server options and variables, and so on.

Perhaps the most frequent pre-query usage is to specify the encoding that the server will use for the rows it returns. Note that Manticore accepts only UTF-8 texts. Two MySQL specific examples of setting the encoding are:

```ini
sql_query_pre = SET CHARACTER_SET_RESULTS=utf8
sql_query_pre = SET NAMES utf8
```

Also specific to MySQL sources, it is useful to disable query cache (for indexer connection only) in pre-query, because indexing queries are not going to be re-run frequently anyway, and there's no sense in caching their results.
That could be achieved with:

```ini
sql_query_pre = SET SESSION query_cache_type=OFF
```

## sql_query_post

Post-fetch query. Optional, default value is empty.

This query is executed immediately after sql_query completes successfully. When post-fetch query produces errors, they are reported as warnings, but indexing is *not* terminated. It's result set is ignored. Note that indexing is not yet completed at the point when this query gets executed, and further indexing still may fail. Therefore, any permanent updates should not be done from here. For instance, updates on helper table that permanently change the last successfully indexed ID should not be run from post-fetch query; they should be run from post-index query instead.

## sql_query_post_index

Post-index query. Optional, default value is empty.

This query is executed when indexing is fully and successfully completed. If this query produces errors, they are reported as warnings, but indexing is not terminated. It's result set is ignored. `$maxid` macro can be used in its text; it will be expanded to maximum document ID which was actually fetched from the database during indexing. If no documents were indexed, `$maxid` will be expanded to 0.

Example:
```ini
sql_query_post_index = REPLACE INTO counters ( id, val ) \
    VALUES ( 'max_indexed_id', $maxid )
```

The difference between post-query and post-index query is in that post-query is run immediately when Manticore received all the documents, but further indexing may still fail for some other reason. On the contrary, by the time the post-index query gets executed, it is guaranteed that the indexing was successful. Database connection is dropped and re-established because sorting phase can be very lengthy and would just timeout otherwise.
