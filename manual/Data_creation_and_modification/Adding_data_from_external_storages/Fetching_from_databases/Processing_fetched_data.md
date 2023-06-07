# Processing fetched data

By default, the first column from the result set of `sql_query` is indexed as the document id.

Document ID *MUST* be the very first field, and it **MUST BE** UNIQUE SIGNED (NON-ZERO) INTEGER NUMBER from -9223372036854775808 to 9223372036854775807.

You can specify up to 256 full-text fields and an arbitrary amount of attributes. All the columns that are neither document ID (the first one) nor attributes will be indexed as full-text fields.


## Declaration of attributes:

### sql_attr_bigint

Declares a 64-bit *signed* integer.

### sql_attr_bool

Declares a boolean attribute. It's equivalent to an integer attribute with bit count of 1.

### sql_attr_float

Declares a floating point attribute.

The values will be stored in single precision, 32-bit IEEE 754 format. Represented range is approximately from 1e-38 to 1e+38. The amount of decimal digits that can be stored precisely is approximately 7.

One important usage of float attributes is storing latitude and longitude values (in radians), for further usage in query-time geosphere distance calculations.

### sql_attr_json

Declares a JSON attribute.

When indexing JSON attributes, Manticore expects a text field with JSON formatted data. JSON attributes support arbitrary JSON data with no limitation in nested levels or types.

### sql_attr_multi

Declares a multi-value attribute.

Plain attributes only allow attaching 1 value per each document. However, there are cases (such as tags or categories) when it is desired to attach multiple values of the same attribute and be able to apply filtering or grouping to value lists.

The MVA can take the values from a column (like the rest of the data types) - in this case, the column in the result set must provide a string with multiple integer values separated by commas - or by running a separate query to get the values.

In case of query, the engine runs the query, groups the result by IDs, and attaches the values to their corresponding documents in the table. Values with an ID not found in the table are discarded. Before query, set of `sql_query_pre_all`, if any, will be executed.

The declaration format for sql_attr_multi is as follows:

```ini
sql_attr_multi = ATTR-TYPE ATTR-NAME 'from' SOURCE-TYPE \
    [;QUERY] \
    [;RANGED-QUERY]
```

where

* ATTR-TYPE is `uint`, `bigint` or `timestamp`.
* SOURCE-TYPE is `field`, `query`, `ranged-query`, or `ranged-main-query`.
* QUERY is an optional SQL query used to fetch all (docid, attrvalue) pairs.
* RANGED-QUERY is an optional SQL query used to fetch min and max ID values, similar to `sql_query_range`.
* The backslashes are included for clarity only; everything can be declared in a single line as well.

It's used with ranged-query SOURCE-TYPE. If using `ranged-main-query` SOURCE-TYPE, then omit the RANGED-QUERY, and it will automatically use the same query from `sql_query_range`(useful option in complex inheritance setups to save having to manually duplicate the same query many times).

```ini
sql_attr_multi = uint tag from field
sql_attr_multi = uint tag from query; SELECT id, tag FROM tags
sql_attr_multi = bigint tag from ranged-query; \
    SELECT id, tag FROM tags WHERE id>=$start AND id<=$end; \
    SELECT MIN(id), MAX(id) FROM tags
```

### sql_attr_string

Declares a string attribute. The maximum size of each value is fixed at 4GB.

### sql_attr_timestamp

Declares a UNIX timestamp.

Timestamps can store dates and times in the range of January 01, 1970, to January 19, 2038, with a precision of one second. The expected column value should be a timestamp in UNIX format, which is a 32-bit unsigned integer number of seconds elapsed since midnight on January 01, 1970, GMT. Timestamps are internally stored and handled as integers everywhere. In addition to working with timestamps as integers, you can also use them with different date-based functions, such as time segments sorting mode or day/week/month/year extraction for GROUP BY.

Note that DATE or DATETIME column types in MySQL cannot be directly used as timestamp attributes in Manticore; you need to explicitly convert such columns using UNIX_TIMESTAMP function (if the data is in range).

Note timestamps can not represent dates before January 01, 1970, and UNIX_TIMESTAMP() in MySQL will not return anything expected. If you only need to work with dates, not times, consider `TO_DAYS()` function in MySQL instead.


### sql_attr_uint

Declares an unsigned integer attribute.

You can specify the bit count for integer attributes by appending ':BITCOUNT' to attribute name (see example below). Attributes with less than default 32-bit size, or bitfields, perform slower.

```ini
sql_attr_uint = group_id
sql_attr_uint = forum_id:9 # 9 bits for forum_id
```

### sql_field_string

Declares a combo string attribute/text field. The values will be indexed as a full-text field, but also stored in a string attribute with the same name. Note, it should only be used when you are sure you want the field to be searchable both in a full-text manner and as an attribute (with the ability to sort and group by it). If you just want to be able to fetch the original value of the field, you don't need to do anything for it unless you implicitly removed the field from the stored fields list via [stored_fields](../../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#stored_fields).

```ini
sql_field_string = name
```

### sql_file_field

Declares a file based field.

This directive makes indexer interpret field contents as a file name, and load and process the referred file. Files larger than max_file_field_buffer in size are skipped. Any errors during the file loading (IO errors, missed limits, etc.) will be reported as indexing warnings and will not early terminate the indexing. No content will be indexed for such files.

```ini
sql_file_field = field_name
```

### sql_joined_field

Joined/payload field fetch query. Multi-value, optional, the default is an empty list of queries.

`sql_joined_field` lets you use two different features: joined fields and payloads (payload fields). Its syntax is as follows:

```ini
sql_joined_field = FIELD-NAME 'from'  ( 'query' | 'payload-query' | 'ranged-query' | 'ranged-main-query' ); \
		QUERY [ ; RANGE-QUERY ]
```

where

* FIELD-NAME is a joined/payload field name
* QUERY is an SQL query that must fetch values for further processing
* RANGE-QUERY is an optional SQL query that fetches a range of values to process

**Joined fields** let you avoid JOIN and/or GROUP_CONCAT statements in the main document fetch query (sql_query). This can be useful when the SQL-side JOIN is slow, or needs to be offloaded on the Manticore side, or simply to emulate MySQL-specific `GROUP_CONCAT` functionality in case your database server does not support it.

The query must return exactly 2 columns: document ID, and text to append to a joined field. Document IDs can be duplicate, but they must be in ascending order. All the text rows fetched for a given ID will be concatenated together, and the concatenation result will be indexed as the entire contents of a joined field. Rows will be concatenated in the order returned from the query, and separating whitespace will be inserted between them. For instance, if the joined field query returns the following rows:
```ini
( 1, 'red' )
( 1, 'right' )
( 1, 'hand' )
( 2, 'mysql' )
( 2, 'manticore' )
```

then the indexing results would be equivalent to adding a new text field with a value of 'red right hand' to document 1 and 'mysql sphinx' to document 2, including the keyword positions inside the field in the order they come from the query. If the rows need to be in a specific order, that needs to be explicitly defined in the query.

Joined fields are only indexed differently. There are no other differences between joined fields and regular text fields.
Before joined fields query, set of `sql_query_pre_all`, if any, will be executed. That will allow you to set desired encoding, etc. for joined fields context.

When a single query is not efficient enough or does not work because of the database driver limitations, ranged queries can be used. It works similarly to the ranged queries in the main indexing loop. The range will be queried for and fetched upfront once, then multiple queries with different `$start` and `$end` substitutions will be run to fetch the actual data.

When using `ranged-main-query` query, omit the `ranged-query`, and it will automatically use the same query from `sql_query_range` (a useful option in complex inheritance setups to save having to manually duplicate the same query many times).

<!-- example payload -->
**Payloads** let you create a special field in which, instead of keyword positions, so-called user payloads are stored. Payloads are custom integer values attached to every keyword. They can then be used at search time to affect the ranking.

The payload query must return exactly 3 columns:
- document ID
- keyword
- and integer payload value.

Document IDs can be duplicate, but they must be in ascending order. Payloads **must** be unsigned integers within the 24-bit range, i.e., from 0 to 16777215.

The only ranker that accounts for payloads is `proximity_bm25` (the default [ranker](../../Searching/Sorting_and_ranking.md#Available-built-in-rankers)). On tables with payload fields, it will automatically switch to a variant that matches keywords in those fields, computes a sum of matched payloads multiplied by field weights, and adds that sum to the final rank.

Please note that the payload field is ignored for full-text queries containing complex operators. It only works for simple bag-of-words queries.

<!-- intro -->
Configuration example:
<!-- request Configuration file -->
```ini
source min {
    type = mysql
    sql_host = localhost
    sql_user = test
    sql_pass =
    sql_db = test
    sql_query = select 1, 'Nike bag' f \
    UNION select 2, 'Adidas bag' f \
    UNION select 3, 'Reebok bag' f \
    UNION select 4, 'Nike belt' f

    sql_joined_field = tag from payload-query; select 1 id, 'nike' tag, 10 weight \
    UNION select 4 id, 'nike' tag, 10 weight;
}

index idx {
    path = idx
    source = min
}
```

<!-- request Just SELECT -->
```sql
mysql> select * from idx;
+------+------------+------+
| id   | f          | tag  |
+------+------------+------+
|    1 | Nike bag   | nike |
|    2 | Adidas bag |      |
|    3 | Reebok bag |      |
|    4 | Nike belt  | nike |
+------+------------+------+
4 rows in set (0.00 sec)
```

<!-- request Full-text search -->

Note that when you search for `nike | adidas`, the results containing "nike" receive a higher weight due to the "nike" tag and its weight originating from the payload query.
```sql
mysql> select *, weight() from idx where match('nike|adidas');
+------+------------+------+----------+
| id   | f          | tag  | weight() |
+------+------------+------+----------+
|    1 | Nike bag   | nike |    11539 |
|    4 | Nike belt  | nike |    11539 |
|    2 | Adidas bag |      |     1597 |
+------+------------+------+----------+
3 rows in set (0.01 sec)
```

<!-- request Complex full-text search -->

Note that the special payload field is ignored for full-text queries containing complex operators. It only works for simple bag-of-words queries.

```sql
mysql> select *, weight() from idx where match('"nike bag"|"adidas bag"');
+------+------------+------+----------+
| id   | f          | tag  | weight() |
+------+------------+------+----------+
|    2 | Adidas bag |      |     2565 |
|    1 | Nike bag   | nike |     2507 |
+------+------------+------+----------+
2 rows in set (0.00 sec)
```

<!-- end -->

### sql_column_buffers

```ini
sql_column_buffers = <colname>=<size>[K|M] [, ...]
```

Per-column buffer sizes. Optional, default is empty (deduce the sizes automatically). Applies to `odbc`, `mssql` source types only.

ODBC and MS SQL drivers sometimes cannot return the maximum actual column size to be expected. For instance,`NVARCHAR(MAX)` columns always report their length as 2147483647 bytes to `indexer` even though the actually used length is likely considerably less. However, the receiving buffers still need to be allocated upfront, and their sizes have to be determined. When the driver does not report the column length at all, Manticore allocates default 1 KB buffers for each non-char column, and 1 MB buffers for each char column. Driver-reported column length also gets clamped by an upper limit of 8 MB, so in case the driver reports (almost) a 2 GB column length, it will be clamped and an 8 MB buffer will be allocated instead for that column. These hard-coded limits can be overridden using the `sql_column_buffers` directive, either in order to save memory on actually shorter columns or to overcome the 8 MB limit on actually longer columns. The directive values must be a comma-separated list of selected column names and sizes:

Example:

```ini
sql_query = SELECT id, mytitle, mycontent FROM documents
sql_column_buffers = mytitle=64K, mycontent=10M
```
<!-- proofread -->
