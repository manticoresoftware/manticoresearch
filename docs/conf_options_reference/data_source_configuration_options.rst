Data surce configuration options
------------------------------------

.. _csvpipe_delimiter:

csvpipe_delimiter
~~~~~~~~~~~~~~~~~~

csvpipe source fields delimiter. Optional, default value is ‘,’.

Example:

.. code-block:: ini


    csvpipe_delimiter = ;

.. _mssql_winauth:

mssql_winauth
~~~~~~~~~~~~~~

MS SQL Windows authentication flag. Boolean, optional, default value is
0 (false). Applies to ``mssql`` source type only.

Whether to use currently logged in Windows account credentials for
authentication when connecting to MS SQL Server. Note that when running
``searchd`` as a service, account user can differ from the account you
used to install the service.

Example:

.. code-block:: ini


    mssql_winauth = 1

.. _mysql_connect_flags:

mysql_connect_flags
~~~~~~~~~~~~~~~~~~~~~

MySQL client connection flags. Optional, default value is 0 (do not set
any flags). Applies to ``mysql`` source type only.

This option must contain an integer value with the sum of the flags. The
value will be passed to
`mysql_real_connect() <http://dev.mysql.com/doc/refman/5.0/en/mysql-real-connect.html>`__
verbatim. The flags are enumerated in mysql_com.h include file. Flags
that are especially interesting in regard to indexing, with their
respective values, are as follows:

-  CLIENT_COMPRESS = 32; can use compression protocol

-  CLIENT_SSL = 2048; switch to SSL after handshake

-  CLIENT_SECURE_CONNECTION = 32768; new 4.1 authentication

For instance, you can specify 2080 (2048+32) to use both compression and
SSL, or 32768 to use new authentication only. Initially, this option was
introduced to be able to use compression when the ``indexer`` and
``mysqld`` are on different hosts. Compression on 1 Gbps links is most
likely to hurt indexing time though it reduces network traffic, both in
theory and in practice. However, enabling compression on 100 Mbps links
may improve indexing time significantly (upto 20-30% of the total
indexing time improvement was reported). Your mileage may vary.

Example:

.. code-block:: ini


    mysql_connect_flags = 32 # enable compression

.. _mysql_ssl_cert, mysql_ssl_key, mysql_ssl_ca:

mysql_ssl_cert, mysql_ssl_key, mysql_ssl_ca
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

SSL certificate settings to use for connecting to MySQL server.
Optional, default values are empty strings (do not use SSL). Applies to
``mysql`` source type only.

These directives let you set up secure SSL connection between
``indexer`` and MySQL. The details on creating the certificates and
setting up MySQL server can be found in MySQL documentation.

Example:

.. code-block:: ini


    mysql_ssl_cert = /etc/ssl/client-cert.pem
    mysql_ssl_key = /etc/ssl/client-key.pem
    mysql_ssl_ca = /etc/ssl/cacert.pem

.. _odbc_dsn:

odbc_dsn
~~~~~~~~~

ODBC DSN to connect to. Mandatory, no default value. Applies to ``odbc``
source type only.

ODBC DSN (Data Source Name) specifies the credentials (host, user,
password, etc) to use when connecting to ODBC data source. The format
depends on specific ODBC driver used.

Example:

.. code-block:: ini


    odbc_dsn = Driver={Oracle ODBC Driver};Dbq=myDBName;Uid=myUsername;Pwd=myPassword

.. _sql_attr_bigint:

sql_attr_bigint
~~~~~~~~~~~~~~~~~

64-bit signed integer :ref:`attribute <attributes>` declaration.
Multi-value (there might be multiple attributes declared), optional.
Applies to SQL source types (``mysql``, ``pgsql``, ``mssql``) only. Note
that unlike
:ref:`sql_attr_uint <sql_attr_uint>`,
these values are **signed**.

Example:

.. code-block:: ini


    sql_attr_bigint = my_bigint_id

.. _sql_attr_bool:

sql_attr_bool
~~~~~~~~~~~~~~~

Boolean :ref:`attribute <attributes>` declaration. Multi-value
(there might be multiple attributes declared), optional. Applies to SQL
source types (``mysql``, ``pgsql``, ``mssql``) only. Equivalent to
:ref:`sql_attr_uint <sql_attr_uint>`
declaration with a bit count of 1.

Example:

.. code-block:: ini


    sql_attr_bool = is_deleted # will be packed to 1 bit

.. _sql_attr_float:

sql_attr_float
~~~~~~~~~~~~~~~~

Floating point :ref:`attribute <attributes>` declaration.
Multi-value (there might be multiple attributes declared), optional.
Applies to SQL source types (``mysql``, ``pgsql``, ``mssql``) only.

The values will be stored in single precision, 32-bit IEEE 754 format.
Represented range is approximately from 1e-38 to 1e+38. The amount of
decimal digits that can be stored precisely is approximately 7. One
important usage of the float attributes is storing latitude and
longitude values (in radians), for further usage in query-time geosphere
distance calculations.

Example:

.. code-block:: ini


    sql_attr_float = lat_radians
    sql_attr_float = long_radians

.. _sql_attr_json:

sql_attr_json
~~~~~~~~~~~~~~~

JSON attribute declaration. Multi-value (ie. there may be more than one
such attribute declared), optional. Applies to SQL source types
(``mysql``, ``pgsql``, ``mssql``) only.

When indexing JSON attributes, Manticore expects a text field with JSON
formatted data. JSON attributes supports arbitrary JSON data with no
limitation in nested levels or types.

.. code-block:: ini


    {
        "id": 1,
        "gid": 2,
        "title": "some title",
        "tags": [
            "tag1",
            "tag2",
            "tag3"
            {
                "one": "two",
                "three": [4, 5]
            }
        ]
    }

These attributes allow Manticore to work with documents without a fixed set
of attribute columns. When you filter on a key of a JSON attribute,
documents that don't include the key will simply be ignored.


Example:

.. code-block:: ini


    sql_attr_json = properties

.. _sql_attr_multi:

sql_attr_multi
~~~~~~~~~~~~~~~~

:ref:`Multi-valued attribute <mva_multi-valued_attributes>` (MVA)
declaration. Multi-value (ie. there may be more than one such attribute
declared), optional. Applies to SQL source types (``mysql``, ``pgsql``,
``mssql``) only.

Plain attributes only allow to attach 1 value per each document.
However, there are cases (such as tags or categories) when it is desired
to attach multiple values of the same attribute and be able to apply
filtering or grouping to value lists.

The declaration format is as follows (backslashes are for clarity only;
everything can be declared in a single line as well):

.. code-block:: ini


    sql_attr_multi = ATTR-TYPE ATTR-NAME 'from' SOURCE-TYPE \
        [;QUERY] \
        [;RANGE-QUERY]

where

-  ATTR-TYPE is ‘uint’, ‘bigint’ or ‘timestamp’

-  SOURCE-TYPE is ‘field’, ‘query’, or ‘ranged-query’

-  QUERY is SQL query used to fetch all ( docid, attrvalue ) pairs

-  RANGE-QUERY is SQL query used to fetch min and max ID values, similar
   to ‘sql_query_range’

Example:

.. code-block:: ini

    sql_attr_multi = uint tag from query; SELECT id, tag FROM tags
    sql_attr_multi = bigint tag from ranged-query; \
        SELECT id, tag FROM tags WHERE id>=$start AND id<=$end; \
        SELECT MIN(id), MAX(id) FROM tags

.. _sql_attr_string:

sql_attr_string
~~~~~~~~~~~~~~~~~

String attribute declaration. Multi-value (ie. there may be more than
one such attribute declared), optional. Applies to SQL source types
(``mysql``, ``pgsql``, ``mssql``) only.

String attributes can store arbitrary strings attached to every
document. There's a fixed size limit of 4 MB per value. Also,
``searchd`` will currently cache all the values in RAM, which is an
additional implicit limit.

String attributes can be used for sorting and grouping(ORDER BY, GROUP
BY, WITHIN GROUP ORDER BY). Note that attributes declared using
``sql_attr_string`` will **not** be full-text indexed; you can use
:ref:`sql_field_string <sql_field_string>`
directive for that.

Example:

.. code-block:: ini

    sql_attr_string = title # will be stored but will not be indexed

.. _sql_attr_timestamp:

sql_attr_timestamp
~~~~~~~~~~~~~~~~~~~~

UNIX timestamp :ref:`attribute <attributes>` declaration.
Multi-value (there might be multiple attributes declared), optional.
Applies to SQL source types (``mysql``, ``pgsql``, ``mssql``) only.

Timestamps can store date and time in the range of Jan 01, 1970 to Jan
19, 2038 with a precision of one second. The expected column value
should be a timestamp in UNIX format, ie. 32-bit unsigned integer number
of seconds elapsed since midnight, January 01, 1970, GMT. Timestamps are
internally stored and handled as integers everywhere. But in addition to
working with timestamps as integers, it's also legal to use them along
with different date-based functions, such as time segments sorting mode,
or day/week/month/year extraction for GROUP BY.

Note that DATE or DATETIME column types in MySQL can **not** be
directly used as timestamp attributes in Manticore; you need to explicitly
convert such columns using UNIX_TIMESTAMP function (if data is in
range).

Note timestamps can not represent dates before January 01, 1970, and
UNIX_TIMESTAMP() in MySQL will not return anything expected. If you
only needs to work with dates, not times, consider TO_DAYS() function
in MySQL instead.

Example:

.. code-block:: ini


    # sql_query = ... UNIX_TIMESTAMP(added_datetime) AS added_ts ...
    sql_attr_timestamp = added_ts

.. _sql_attr_uint:

sql_attr_uint
~~~~~~~~~~~~~~~

Unsigned integer :ref:`attribute <attributes>` declaration.
Multi-value (there might be multiple attributes declared), optional.
Applies to SQL source types (``mysql``, ``pgsql``, ``mssql``) only.

The column value should fit into 32-bit unsigned integer range. Values
outside this range will be accepted but wrapped around. For instance, -1
will be wrapped around to 2^32-1 or 4,294,967,295.

You can specify bit count for integer attributes by appending
‘:BITCOUNT’ to attribute name (see example below). Attributes with less
than default 32-bit size, or bitfields, perform slower. But they require
less RAM when using :ref:`extern
storage <docinfo>`: such
bitfields are packed together in 32-bit chunks in ``.spa`` attribute
data file. Bit size settings are ignored if using :ref:`inline
storage <docinfo>`.

Example:

.. code-block:: ini


    sql_attr_uint = group_id
    sql_attr_uint = forum_id:9 # 9 bits for forum_id

.. _sql_column_buffers:

sql_column_buffers
~~~~~~~~~~~~~~~~~~~~

Per-column buffer sizes. Optional, default is empty (deduce the sizes
automatically). Applies to ``odbc``, ``mssql`` source types only.

ODBC and MS SQL drivers sometimes can not return the maximum actual
column size to be expected. For instance, NVARCHAR(MAX) columns always
report their length as 2147483647 bytes to ``indexer`` even though the
actually used length is likely considerably less. However, the receiving
buffers still need to be allocated upfront, and their sizes have to be
determined. When the driver does not report the column length at all,
Manticore allocates default 1 KB buffers for each non-char column, and 1 MB
buffers for each char column. Driver-reported column length also gets
clamped by an upper limit of 8 MB, so in case the driver reports
(almost) a 2 GB column length, it will be clamped and a 8 MB buffer will
be allocated instead for that column. These hard-coded limits can be
overridden using the ``sql_column_buffers`` directive, either in order
to save memory on actually shorter columns, or overcome the 8 MB limit
on actually longer columns. The directive values must be a
comma-separated lists of selected column names and sizes:

.. code-block:: ini


    sql_column_buffers = <colname>=<size>[K|M] [, ...]

Example:

.. code-block:: ini


    sql_query = SELECT id, mytitle, mycontent FROM documents
    sql_column_buffers = mytitle=64K, mycontent=10M

.. _sql_db:

sql_db
~~~~~~~

SQL database (in MySQL terms) to use after the connection and perform
further queries within. Mandatory, no default value. Applies to SQL
source types (``mysql``, ``pgsql``, ``mssql``) only.

Example:

.. code-block:: ini


    sql_db = test

.. _sql_field_string:

sql_field_string
~~~~~~~~~~~~~~~~~~

Combined string attribute and full-text field declaration. Multi-value
(ie. there may be more than one such attribute declared), optional.
Applies to SQL source types (``mysql``, ``pgsql``, ``mssql``) only.

:ref:`sql_attr_string <sql_attr_string>`
only stores the column value but does not full-text index it. In some
cases it might be desired to both full-text index the column and store
it as attribute. ``sql_field_string`` lets you do exactly that. Both the
field and the attribute will be named the same.

Example:

.. code-block:: ini


    sql_field_string = title # will be both indexed and stored

.. _sql_file_field:

sql_file_field
~~~~~~~~~~~~~~~~

File based field declaration. Applies to SQL source types (``mysql``,
``pgsql``, ``mssql``) only. Introduced in version 1.10-beta.

This directive makes ``indexer`` interpret field contents as a file
name, and load and index the referred file. Files larger than
:ref:`max_file_field_buffer <max_file_field_buffer>`
in size are skipped. Any errors during the file loading (IO errors,
missed limits, etc) will be reported as indexing warnings and will
**not** early terminate the indexing. No content will be indexed for
such files.

Example:

.. code-block:: ini


    sql_file_field = my_file_path # load and index files referred to by my_file_path

.. _sql_host:

sql_host
~~~~~~~~~

SQL server host to connect to. Mandatory, no default value. Applies to
SQL source types (``mysql``, ``pgsql``, ``mssql``) only.

In the simplest case when Manticore resides on the same host with your
MySQL or PostgreSQL installation, you would simply specify “localhost”.
Note that MySQL client library chooses whether to connect over TCP/IP or
over UNIX socket based on the host name. Specifically “localhost” will
force it to use UNIX socket (this is the default and generally
recommended mode) and “127.0.0.1” will force TCP/IP usage. Refer to
`MySQL
manual <http://dev.mysql.com/doc/refman/5.0/en/mysql-real-connect.html>`__
for more details.

Example:

.. code-block:: ini


    sql_host = localhost

.. _sql_joined_field:

sql_joined_field
~~~~~~~~~~~~~~~~~~

Joined/payload field fetch query. Multi-value, optional, default is
empty list of queries. Applies to SQL source types (``mysql``,
``pgsql``, ``mssql``) only.

``sql_joined_field`` lets you use two different features: joined fields,
and payloads (payload fields). It's syntax is as follows:

.. code-block:: ini


    sql_joined_field = FIELD-NAME 'from'  ( 'query' | 'payload-query' \
        | 'ranged-query' ); QUERY [ ; RANGE-QUERY ]

where

-  FIELD-NAME is a joined/payload field name;

-  QUERY is an SQL query that must fetch values to index.

-  RANGE-QUERY is an optional SQL query that fetches a range of values
   to index.

**Joined fields** let you avoid JOIN and/or GROUP_CONCAT statements
in the main document fetch query (sql_query). This can be useful when
SQL-side JOIN is slow, or needs to be offloaded on Manticore side, or
simply to emulate MySQL-specific GROUP_CONCAT functionality in case
your database server does not support it.

The query must return exactly 2 columns: document ID, and text to append
to a joined field. Document IDs can be duplicate, but they **must**
be in ascending order. All the text rows fetched for a given ID will be
concatenated together, and the concatenation result will be indexed as
the entire contents of a joined field. Rows will be concatenated in the
order returned from the query, and separating whitespace will be
inserted between them. For instance, if joined field query returns the
following rows:

.. code-block:: ini


    ( 1, 'red' )
    ( 1, 'right' )
    ( 1, 'hand' )
    ( 2, 'mysql' )
    ( 2, 'sphinx' )

then the indexing results would be equivalent to that of adding a new
text field with a value of ‘red right hand’ to document 1 and ‘mysql
sphinx’ to document 2.

Joined fields are only indexed differently. There are no other
differences between joined fields and regular text fields.

When a single query is not efficient enough or does not work because of
the database driver limitations, **ranged queries** can be used. It
works similar to the ranged queries in the main indexing loop, see :ref:`ranged_queries`.
The range will be queried for and fetched upfront once, then multiple
queries with different ``$start`` and ``$end`` substitutions will be run
to fetch the actual data.

**Payloads** let you create a special field in which, instead of
keyword positions, so-called user payloads are stored. Payloads are
custom integer values attached to every keyword. They can then be used
in search time to affect the ranking.

The payload query must return exactly 3 columns: document ID; keyword;
and integer payload value. Document IDs can be duplicate, but they
**must** be in ascending order. Payloads must be unsigned integers
within 24-bit range, ie. from 0 to 16777215. For reference, payloads are
currently internally stored as in-field keyword positions, but that is
not guaranteed and might change in the future.

Currently, the only method to account for payloads is to use
SPH_RANK_PROXIMITY_BM25 ranker. On indexes with payload fields, it
will automatically switch to a variant that matches keywords in those
fields, computes a sum of matched payloads multiplied by field weights,
and adds that sum to the final rank.

Example:

.. code-block:: ini


    sql_joined_field = \
        tagstext from query; \
        SELECT docid, CONCAT('tag',tagid) FROM tags ORDER BY docid ASC

    sql_joined_field = bigint tag from ranged-query; \
        SELECT id, tag FROM tags WHERE id>=$start AND id<=$end ORDER BY id ASC; \
        SELECT MIN(id), MAX(id) FROM tags

.. _sql_pass:

sql_pass
~~~~~~~~~

SQL user password to use when connecting to
:ref:`sql_host <sql_host>`.
Mandatory, no default value. Applies to SQL source types (``mysql``,
``pgsql``, ``mssql``) only.

Example:

.. code-block:: ini


    sql_pass = mysecretpassword

.. _sql_port:

sql_port
~~~~~~~~~

SQL server IP port to connect to. Optional, default is 3306 for
``mysql`` source type and 5432 for ``pgsql`` type. Applies to SQL source
types (``mysql``, ``pgsql``, ``mssql``) only. Note that it depends on
:ref:`sql_host <sql_host>`
setting whether this value will actually be used.

Example:

.. code-block:: ini


    sql_port = 3306

.. _sql_query_killlist:

sql_query_killlist
~~~~~~~~~~~~~~~~~~~~

Kill-list query. Optional, default is empty (no query). Applies to SQL
source types (``mysql``, ``pgsql``, ``mssql``) only.

This query is expected to return a number of 1-column rows, each
containing just the document ID. The returned document IDs are stored
within an index. Kill-list for a given index suppresses results from
*other* indexes, depending on index order in the query. The intended use
is to help implement deletions and updates on existing indexes without
rebuilding (actually even touching them), and especially to fight
phantom results problem.

Let us dissect an example. Assume we have two indexes, ‘main’ and
‘delta’. Assume that documents 2, 3, and 5 were deleted since last
reindex of ‘main’, and documents 7 and 11 were updated (ie. their text
contents were changed). Assume that a keyword ‘test’ occurred in all
these mentioned documents when we were indexing ‘main’; still occurs in
document 7 as we index ‘delta’; but does not occur in document 11 any
more. We now reindex delta and then search through both these indexes in
proper (least to most recent) order:

.. code-block:: ini


    $res = $cl->Query ( "test", "main delta" );

First, we need to properly handle deletions. The result set should not
contain documents 2, 3, or 5. Second, we also need to avoid phantom
results. Unless we do something about it, document 11 *will* appear in
search results! It will be found in ‘main’ (but not ‘delta’). And it
will make it to the final result set unless something stops it.

Kill-list, or K-list for short, is that something. Kill-list attached to
‘delta’ will suppress the specified rows from **all** the preceding
indexes, in this case just ‘main’. So to get the expected results, we
should put all the updated *and* deleted document IDs into it.

Note that in the distributed index setup, K-lists are **local to every
node in the cluster**. They are **not** get transmitted over the
network when sending queries. (Because that might be too much of an
impact when the K-list is huge.) You will need to setup a separate
per-server K-lists in that case.

Example:

.. code-block:: ini


    sql_query_killlist = \
        SELECT id FROM documents WHERE updated_ts>=@last_reindex UNION \
        SELECT id FROM documents_deleted WHERE deleted_ts>=@last_reindex

.. _sql_query_post_index:

sql_query_post_index
~~~~~~~~~~~~~~~~~~~~~~~

Post-index query. Optional, default value is empty. Applies to SQL
source types (``mysql``, ``pgsql``, ``mssql``) only.

This query is executed when indexing is fully and successfully
completed. If this query produces errors, they are reported as warnings,
but indexing is **not** terminated. It's result set is ignored.
``$maxid`` macro can be used in its text; it will be expanded to maximum
document ID which was actually fetched from the database during
indexing. If no documents were indexed, $maxid will be expanded to 0.

Example:

.. code-block:: ini


    sql_query_post_index = REPLACE INTO counters ( id, val ) \
        VALUES ( 'max_indexed_id', $maxid )

.. _sql_query_post:

sql_query_post
~~~~~~~~~~~~~~~~

Post-fetch query. Optional, default value is empty. Applies to SQL
source types (``mysql``, ``pgsql``, ``mssql``) only.

This query is executed immediately after
:ref:`sql_query <sql_query>`
completes successfully. When post-fetch query produces errors, they are
reported as warnings, but indexing is **not** terminated. It's result
set is ignored. Note that indexing is **not** yet completed at the
point when this query gets executed, and further indexing still may
fail. Therefore, any permanent updates should not be done from here. For
instance, updates on helper table that permanently change the last
successfully indexed ID should not be run from post-fetch query; they
should be run from `post-index
query <sql_query_post_index>`
instead.

Example:

.. code-block:: ini


    sql_query_post = DROP TABLE my_tmp_table

.. _sql_query_pre:

sql_query_pre
~~~~~~~~~~~~~~~

Pre-fetch query, or pre-query. Multi-value, optional, default is empty
list of queries. Applies to SQL source types (``mysql``, ``pgsql``,
``mssql``) only.

Multi-value means that you can specify several pre-queries. They are
executed before `the main fetch
query <sqlquery>`, and they
will be executed exactly in order of appearance in the configuration
file. Pre-query results are ignored.

Pre-queries are useful in a lot of ways. They are used to setup
encoding, mark records that are going to be indexed, update internal
counters, set various per-connection SQL server options and variables,
and so on.

Perhaps the most frequent pre-query usage is to specify the encoding
that the server will use for the rows it returns. Note that Manticore
accepts only UTF-8 texts. Two MySQL specific examples of setting the
encoding are:

.. code-block:: ini


    sql_query_pre = SET CHARACTER_SET_RESULTS=utf8
    sql_query_pre = SET NAMES utf8

Also specific to MySQL sources, it is useful to disable query cache (for
indexer connection only) in pre-query, because indexing queries are not
going to be re-run frequently anyway, and there's no sense in caching
their results. That could be achieved with:

.. code-block:: ini


    sql_query_pre = SET SESSION query_cache_type=OFF

Example:

.. code-block:: ini


    sql_query_pre = SET NAMES utf8
    sql_query_pre = SET SESSION query_cache_type=OFF

.. _sql_query_range:

sql_query_range
~~~~~~~~~~~~~~~~~

Range query setup. Optional, default is empty. Applies to SQL source
types (``mysql``, ``pgsql``, ``mssql``) only.

Setting this option enables ranged document fetch queries (see :ref:`ranged_queries`).
Ranged queries are useful to avoid notorious MyISAM table locks when
indexing lots of data. (They also help with other less notorious issues,
such as reduced performance caused by big result sets, or additional
resources consumed by InnoDB to serialize big read transactions.)

The query specified in this option must fetch min and max document IDs
that will be used as range boundaries. It must return exactly two
integer fields, min ID first and max ID second; the field names are
ignored.

When ranged queries are enabled,
:ref:`sql_query <sql_query>`
will be required to contain ``$start`` and ``$end`` macros (because it
obviously would be a mistake to index the whole table many times over).
Note that the intervals specified by ``$start``..\ ``$end`` will not
overlap, so you should **not** remove document IDs that are exactly
equal to ``$start`` or ``$end`` from your query. The example in :ref:`ranged_queries`)
illustrates that; note how it uses greater-or-equal and less-or-equal
comparisons.

Example:

.. code-block:: ini


    sql_query_range = SELECT MIN(id),MAX(id) FROM documents

.. _sql_query:

sql_query
~~~~~~~~~~

Main document fetch query. Mandatory, no default value. Applies to SQL
source types (``mysql``, ``pgsql``, ``mssql``) only.

There can be only one main query. This is the query which is used to
retrieve documents from SQL server. You can specify up to 32 full-text
fields (formally, upto SPH_MAX_FIELDS from sphinx.h), and an arbitrary
amount of attributes. All of the columns that are neither document ID
(the first one) nor attributes will be full-text indexed.

Document ID **MUST** be the very first field, and it **MUST BE UNIQUE
UNSIGNED POSITIVE (NON-ZERO, NON-NEGATIVE) INTEGER NUMBER**. It can be
either 32-bit or 64-bit, depending on how you built Manticore; by default
it builds with 32-bit IDs support but ``--enable-id64`` option to
``configure`` allows to build with 64-bit document and word IDs support.

Example:

.. code-block:: ini


  sql_query = \
  SELECT id, group_id, UNIX_TIMESTAMP(date_added) AS date_added, \
  title, content \
  FROM documents


.. _sql_ranged_throttle:

sql_ranged_throttle
~~~~~~~~~~~~~~~~~~~~~

Ranged query throttling period, in milliseconds. Optional, default is 0
(no throttling). Applies to SQL source types (``mysql``, ``pgsql``,
``mssql``) only.

Throttling can be useful when indexer imposes too much load on the
database server. It causes the indexer to sleep for given amount of
milliseconds once per each ranged query step. This sleep is
unconditional, and is performed before the fetch query.

Example:

.. code-block:: ini


    sql_ranged_throttle = 1000 # sleep for 1 sec before each query step

.. _sql_range_step:

sql_range_step
~~~~~~~~~~~~~~~~

Range query step. Optional, default is 1024. Applies to SQL source types
(``mysql``, ``pgsql``, ``mssql``) only.

Only used when :ref:`ranged_queries`
are enabled. The full document IDs interval fetched by
:ref:`sql_query_range <sql_query_range>`
will be walked in this big steps. For example, if min and max IDs
fetched are 12 and 3456 respectively, and the step is 1000, indexer will
call
:ref:`sql_query <sql_query>`
several times with the following substitutions:

-  $start=12, $end=1011

-  $start=1012, $end=2011

-  $start=2012, $end=3011

-  $start=3012, $end=3456

Example:

.. code-block:: ini


    sql_range_step = 1000

.. _sql_sock:

sql_sock
~~~~~~~~~

UNIX socket name to connect to for local SQL servers. Optional, default
value is empty (use client library default settings). Applies to SQL
source types (``mysql``, ``pgsql``, ``mssql``) only.

On Linux, it would typically be ``/var/lib/mysql/mysql.sock``. On
FreeBSD, it would typically be ``/tmp/mysql.sock``. Note that it depends
on :ref:`sql_host <sql_host>`
setting whether this value will actually be used.

Example:

.. code-block:: ini


    sql_sock = /tmp/mysql.sock

.. _sql_user:

sql_user
~~~~~~~~~

SQL user to use when connecting to
:ref:`sql_host <sql_host>`.
Mandatory, no default value. Applies to SQL source types (``mysql``,
``pgsql``, ``mssql``) only.

Example:

.. code-block:: ini

    sql_user = test

.. _type_source:

type
~~~~

Data source type. Mandatory, no default value. Known types are
``mysql``, ``pgsql``, ``mssql``, ``xmlpipe2``, ``tsvpipe``, ``csvpipe``
and ``odbc``.

All other per-source options depend on source type selected by this
option. Names of the options used for SQL sources (ie. MySQL,
PostgreSQL, MS SQL) start with ``sql_``; names of the ones used for
xmlpipe2 or tsvpipe, csvpipe start with ``xmlpipe_`` and ``tsvpipe_``,
``csvpipe_`` correspondingly. All source types are conditional; they
might or might not be supported depending on your build settings,
installed client libraries, etc. ``mssql`` type is currently only
available on Windows. ``odbc`` type is available both on Windows
natively and on Linux through `UnixODBC
library <http://www.unixodbc.org/>`__.

Example:

.. code-block:: ini


    type = mysql

.. _unpack_mysqlcompress_maxsize:

unpack_mysqlcompress_maxsize
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Buffer size for UNCOMPRESS()ed data. Optional, default value is 16M.

When using
:ref:`unpack_mysqlcompress <unpack_mysqlcompress>`,
due to implementation intricacies it is not possible to deduce the
required buffer size from the compressed data. So the buffer must be
preallocated in advance, and unpacked data can not go over the buffer
size. This option lets you control the buffer size, both to limit
``indexer`` memory use, and to enable unpacking of really long data
fields if necessary.

Example:

.. code-block:: ini


    unpack_mysqlcompress_maxsize = 1M

.. _unpack_mysqlcompress:

unpack_mysqlcompress
~~~~~~~~~~~~~~~~~~~~~

Columns to unpack using MySQL UNCOMPRESS() algorithm. Multi-value,
optional, default value is empty list of columns. Applies to SQL source
types (``mysql``, ``pgsql``, ``mssql``) only.

Columns specified using this directive will be unpacked by ``indexer``
using modified zlib algorithm used by MySQL COMPRESS() and UNCOMPRESS()
functions. When indexing on a different box than the database, this lets
you offload the database, and save on network traffic. The feature is
only available if zlib and zlib-devel were both available during build
time.

Example:

.. code-block:: ini


    unpack_mysqlcompress = body_compressed
    unpack_mysqlcompress = description_compressed

.. _unpack_zlib:

unpack_zlib
~~~~~~~~~~~~

Columns to unpack using zlib (aka deflate, aka gunzip). Multi-value,
optional, default value is empty list of columns. Applies to SQL source
types (``mysql``, ``pgsql``, ``mssql``) only.

Columns specified using this directive will be unpacked by ``indexer``
using standard zlib algorithm (called deflate and also implemented by
``gunzip``). When indexing on a different box than the database, this
lets you offload the database, and save on network traffic. The feature
is only available if zlib and zlib-devel were both available during
build time.

Example:

.. code-block:: ini


    unpack_zlib = col1
    unpack_zlib = col2

.. _xmlpipe_attr_bigint:

xmlpipe_attr_bigint
~~~~~~~~~~~~~~~~~~~~~

xmlpipe signed 64-bit integer attribute declaration. Multi-value,
optional. Applies to ``xmlpipe2`` source type only. Syntax fully matches
that of
:ref:`sql_attr_bigint <sql_attr_bigint>`.

Example:

.. code-block:: ini


    xmlpipe_attr_bigint = my_bigint_id

.. _xmlpipe_attr_bool:

xmlpipe_attr_bool
~~~~~~~~~~~~~~~~~~~

xmlpipe boolean attribute declaration. Multi-value, optional. Applies to
``xmlpipe2`` source type only. Syntax fully matches that of
:ref:`sql_attr_bool <sql_attr_bool>`.

Example:

.. code-block:: ini


    xmlpipe_attr_bool = is_deleted # will be packed to 1 bit

.. _xmlpipe_attr_float:

xmlpipe_attr_float
~~~~~~~~~~~~~~~~~~~~

xmlpipe floating point attribute declaration. Multi-value, optional.
Applies to ``xmlpipe2`` source type only. Syntax fully matches that of
:ref:`sql_attr_float <sql_attr_float>`.

Example:

.. code-block:: ini


    xmlpipe_attr_float = lat_radians
    xmlpipe_attr_float = long_radians

.. _xmlpipe_attr_json:

xmlpipe_attr_json
~~~~~~~~~~~~~~~~~~~

JSON attribute declaration. Multi-value (ie. there may be more than one
such attribute declared), optional.

This directive is used to declare that the contents of a given XML tag
are to be treated as a JSON document and stored into a Manticore index for
later use. Refer to :ref:`sql_attr_json`
for more details on the JSON attributes.

Example:

.. code-block:: ini


    xmlpipe_attr_json = properties

.. _xmlpipe_attr_multi_64:

xmlpipe_attr_multi_64
~~~~~~~~~~~~~~~~~~~~~~~~

xmlpipe MVA attribute declaration. Declares the BIGINT (signed 64-bit
integer) MVA attribute. Multi-value, optional. Applies to ``xmlpipe2``
source type only.

This setting declares an MVA attribute tag in xmlpipe2 stream. The
contents of the specified tag will be parsed and a list of integers that
will constitute the MVA will be extracted, similar to how
:ref:`sql_attr_multi <sql_attr_multi>`
parses SQL column contents when ‘field’ MVA source type is specified.

Example:

.. code-block:: ini


    xmlpipe_attr_multi_64 = taglist

.. _xmlpipe_attr_multi:

xmlpipe_attr_multi
~~~~~~~~~~~~~~~~~~~~

xmlpipe MVA attribute declaration. Multi-value, optional. Applies to
``xmlpipe2`` source type only.

This setting declares an MVA attribute tag in xmlpipe2 stream. The
contents of the specified tag will be parsed and a list of integers that
will constitute the MVA will be extracted, similar to how
:ref:`sql_attr_multi <sql_attr_multi>`
parses SQL column contents when ‘field’ MVA source type is specified.

Example:

.. code-block:: ini


    xmlpipe_attr_multi = taglist

.. _xmlpipe_attr_string:

xmlpipe_attr_string
~~~~~~~~~~~~~~~~~~~~~

xmlpipe string declaration. Multi-value, optional. Applies to
``xmlpipe2`` source type only.

This setting declares a string attribute tag in xmlpipe2 stream. The
contents of the specified tag will be parsed and stored as a string
value.

Example:

.. code-block:: ini


    xmlpipe_attr_string = subject

.. _xmlpipe_attr_timestamp:

xmlpipe_attr_timestamp
~~~~~~~~~~~~~~~~~~~~~~~~

xmlpipe UNIX timestamp attribute declaration. Multi-value, optional.
Applies to ``xmlpipe2`` source type only. Syntax fully matches that of
:ref:`sql_attr_timestamp <sql_attr_timestamp>`.

Example:

.. code-block:: ini


    xmlpipe_attr_timestamp = published

.. _xmlpipe_attr_uint:

xmlpipe_attr_uint
~~~~~~~~~~~~~~~~~~~

xmlpipe integer attribute declaration. Multi-value, optional. Applies to
``xmlpipe2`` source type only. Syntax fully matches that of
:ref:`sql_attr_uint <sql_attr_uint>`.

Example:

.. code-block:: ini


    xmlpipe_attr_uint = author_id

.. _xmlpipe_command:

xmlpipe_command
~~~~~~~~~~~~~~~~

Shell command that invokes xmlpipe2 stream producer. Mandatory. Applies
to ``xmlpipe2`` source types only.

Specifies a command that will be executed and which output will be
parsed for documents. Refer to :ref:`xmlpipe2_data_source` for specific format
description.

Example:

.. code-block:: ini


    xmlpipe_command = cat /home/sphinx/test.xml

.. _xmlpipe_field:

xmlpipe_field
~~~~~~~~~~~~~~

xmlpipe field declaration. Multi-value, optional. Applies to
``xmlpipe2`` source type only. Refer to :ref:`xmlpipe2_data_source`.

Example:

.. code-block:: ini


    xmlpipe_field = subject
    xmlpipe_field = content

.. _xmlpipe_field_string:

xmlpipe_field_string
~~~~~~~~~~~~~~~~~~~~~~

xmlpipe field and string attribute declaration. Multi-value, optional.
Applies to ``xmlpipe2`` source type only. Refer to :ref:`xmlpipe2_data_source`.

Makes the specified XML element indexed as both a full-text field and a
string attribute. Equivalent to <sphinx:field name=“field”
attr=“string”/> declaration within the XML file.

Example:

.. code-block:: ini


    xmlpipe_field_string = subject

.. _xmlpipe_fixup_utf8:

xmlpipe_fixup_utf8
~~~~~~~~~~~~~~~~~~~~

Perform Manticore-side UTF-8 validation and filtering to prevent XML parser
from choking on non-UTF-8 documents. Optional, default is 0. Applies to
``xmlpipe2`` source type only.

Under certain occasions it might be hard or even impossible to guarantee
that the incoming XMLpipe2 document bodies are in perfectly valid and
conforming UTF-8 encoding. For instance, documents with national
single-byte encodings could sneak into the stream. libexpat XML parser
is fragile, meaning that it will stop processing in such cases. UTF8
fixup feature lets you avoid that. When fixup is enabled, Manticore will
preprocess the incoming stream before passing it to the XML parser and
replace invalid UTF-8 sequences with spaces.

Example:

.. code-block:: ini


    xmlpipe_fixup_utf8 = 1

