# 处理获取的数据

默认情况下，`sql_query` 结果集的第一列被索引为文档 ID。

文档 ID *必须* 是第一个字段，并且 **必须是** 唯一的有符号（非零）整数，范围从 -9223372036854775808 到 9223372036854775807。

您可以指定最多 256 个全文字段和任意数量的属性。所有既不是文档 ID（第一列）也不是属性的列将被索引为全文字段。


## 属性声明：

### sql_attr_bigint

声明一个 64 位*有符号*整数。

### sql_attr_bool

声明一个布尔属性。它等同于位数为 1 的整数属性。

### sql_attr_float

声明一个浮点属性。

值将以单精度、32 位 IEEE 754 格式存储。表示范围大约从 1e-38 到 1e+38。可以精确存储的小数位数大约为 7 位。

浮点属性的一个重要用途是存储纬度和经度值（以弧度为单位），以便在查询时进行地球距离计算。

### sql_attr_json

声明一个 JSON 属性。

在索引 JSON 属性时，Manticore 期望文本字段包含 JSON 格式的数据。JSON 属性支持任意 JSON 数据，没有嵌套层级或类型的限制。

### sql_attr_multi

声明一个多值属性。

普通属性每个文档只允许附加一个值。然而，有些情况（如标签或类别）希望附加同一属性的多个值，并能够对值列表进行过滤或分组。

MVA 可以从列中获取值（像其他数据类型一样）——在这种情况下，结果集中的列必须提供一个由逗号分隔的多个整数值的字符串——或者通过运行单独的查询来获取值。

执行查询时，引擎运行查询，按 ID 分组结果，并将值分配给表中对应的文档。未在表中找到的 ID 的值将被丢弃。执行查询前，将运行任何定义的 `sql_query_pre_all`。

sql_attr_multi 的声明格式如下：

```ini
sql_attr_multi = ATTR-TYPE ATTR-NAME 'from' SOURCE-TYPE \
    [;QUERY] \
    [;RANGED-QUERY]
```

其中

* ATTR-TYPE 是 `uint`、`bigint` 或 `timestamp`。
* SOURCE-TYPE 是 `field`、`query`、`ranged-query` 或 `ranged-main-query`。
* QUERY 是一个可选的 SQL 查询，用于获取所有 (docid, attrvalue) 对。
* RANGED-QUERY 是一个可选的 SQL 查询，用于获取最小和最大 ID 值，类似于 `sql_query_range`。
* 反斜杠仅为清晰起见；所有内容也可以声明在一行中。

它用于 ranged-query SOURCE-TYPE。如果使用 `ranged-main-query` SOURCE-TYPE，则省略 RANGED-QUERY，它将自动使用 `sql_query_range` 中的相同查询（在复杂继承设置中非常有用，避免手动多次复制相同查询）。

```ini
sql_attr_multi = uint tag from field
sql_attr_multi = uint tag from query; SELECT id, tag FROM tags
sql_attr_multi = bigint tag from ranged-query; \
    SELECT id, tag FROM tags WHERE id>=$start AND id<=$end; \
    SELECT MIN(id), MAX(id) FROM tags
```

### sql_attr_string

声明一个字符串属性。每个值的最大大小固定为 4GB。

### sql_attr_timestamp

声明一个 UNIX 时间戳。

时间戳可以存储从 1970 年 1 月 1 日到 2038 年 1 月 19 日的日期和时间，精度为一秒。期望的列值应为 UNIX 格式的时间戳，即自 1970 年 1 月 1 日午夜 GMT 起经过的秒数的 32 位无符号整数。时间戳在内部作为整数存储和处理。除了作为整数使用时间戳外，还可以使用不同的基于日期的函数，如时间段排序模式或用于 GROUP BY 的日/周/月/年提取。

注意，MySQL 中的 DATE 或 DATETIME 列类型不能直接用作 Manticore 中的时间戳属性；需要显式使用 UNIX_TIMESTAMP 函数转换此类列（如果数据在范围内）。

注意时间戳不能表示 1970 年 1 月 1 日之前的日期，MySQL 中的 UNIX_TIMESTAMP() 不会返回预期结果。如果只需要处理日期而非时间，建议使用 MySQL 中的 `TO_DAYS()` 函数。


### sql_attr_uint

声明一个无符号整数属性。

您可以通过在属性名后附加 ':BITCOUNT' 来指定整数属性的位数（见下面示例）。位数小于默认 32 位或位字段的属性性能较慢。

```ini
sql_attr_uint = group_id
sql_attr_uint = forum_id:9 # 9 bits for forum_id
```

### sql_field_string

声明一个组合字符串属性/文本字段。值将被索引为全文字段，同时也存储在同名的字符串属性中。注意，只有当您确定希望该字段既能全文搜索又能作为属性（支持排序和分组）时，才应使用此项。如果您只想获取字段的原始值，除非您通过 [stored_fields](../../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#stored_fields) 显式移除了字段，否则无需做任何操作。

```ini
sql_field_string = name
```

### sql_file_field

声明一个基于文件的字段。

此指令使索引器将字段内容解释为文件名，并加载处理所指文件。大于 max_file_field_buffer 的文件将被跳过。文件加载期间的任何错误（IO 错误、超出限制等）将作为索引警告报告，不会提前终止索引。此类文件不会被索引任何内容。

```ini
sql_file_field = field_name
```

### sql_joined_field

连接/负载字段获取查询。多值，可选，默认是空查询列表。

`sql_joined_field` 允许您使用两种不同功能：连接字段和负载字段（payload fields）。其语法如下：

```ini
sql_joined_field = FIELD-NAME 'from'  ( 'query' | 'payload-query' | 'ranged-query' | 'ranged-main-query' ); \
		QUERY [ ; RANGE-QUERY ]
```

其中

* FIELD-NAME 是连接/负载字段名
* QUERY 是必须获取值以供进一步处理的 SQL 查询
* RANGE-QUERY 是可选的 SQL 查询，用于获取要处理的值范围

**Joined fields** 让您避免在主文档获取查询（sql_query）中使用 JOIN 和/或 GROUP_CONCAT 语句。当 SQL 端的 JOIN 速度较慢，或者需要将其卸载到 Manticore 端，或者仅仅是为了模拟 MySQL 特有的 `GROUP_CONCAT` 功能（以防您的数据库服务器不支持该功能）时，这会非常有用。

查询必须返回恰好 2 列：文档 ID 和要附加到 joined 字段的文本。文档 ID 可以重复，但必须按升序排列。对于给定 ID 获取的所有文本行将被连接在一起，连接结果将被索引为 joined 字段的全部内容。行将按照查询返回的顺序连接，并在它们之间插入分隔空白。例如，如果 joined 字段查询返回以下行：
```ini
( 1, 'red' )
( 1, 'right' )
( 1, 'hand' )
( 2, 'mysql' )
( 2, 'manticore' )
```

那么索引结果将等同于向文档 1 添加一个新文本字段，值为 'red right hand'，向文档 2 添加一个新文本字段，值为 'mysql sphinx'，包括字段内关键字的位置，顺序与查询返回的顺序一致。如果行需要特定顺序，则必须在查询中显式定义。

Joined fields 仅在索引方式上有所不同。joined fields 和普通文本字段之间没有其他区别。

在执行 joined fields 查询之前，任何存在的 `sql_query_pre_all` 都会被执行。这允许您在 joined fields 的上下文中设置所需的编码等。

当单个查询效率不够或由于数据库驱动限制而无法工作时，可以使用范围查询。它的工作方式类似于主索引循环中的范围查询。范围将被预先查询并一次性获取，然后使用不同的 `$start` 和 `$end` 替换运行多个查询以获取实际数据。

使用 `ranged-main-query` 查询时，省略 `ranged-query`，它将自动使用 `sql_query_range` 中的相同查询（在复杂继承设置中非常有用，可以避免手动多次复制相同查询）。

<!-- example payload -->
**Payloads** 让您创建一个特殊字段，其中存储的不是关键字位置，而是所谓的用户负载。负载是附加到每个关键字的自定义整数值。它们可以在搜索时用于影响排名。

负载查询必须返回恰好 3 列：
- 文档 ID
- 关键字
- 以及整数负载值。

文档 ID 可以重复，但必须按升序排列。负载 **必须** 是 24 位范围内的无符号整数，即从 0 到 16777215。

唯一考虑负载的排序器是 `proximity_bm25`（默认的[排序器](../../../Searching/Sorting_and_ranking.md#Available-built-in-rankers)）。在带有负载字段的表上，它会自动切换到一个变体，该变体匹配这些字段中的关键字，计算匹配负载乘以字段权重的总和，并将该总和加到最终排名中。

请注意，负载字段会被忽略用于包含复杂操作符的全文查询。它仅适用于简单的词袋查询。

<!-- intro -->
配置示例：
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

请注意，当您搜索 `nike | adidas` 时，包含 "nike" 的结果由于来自负载查询的 "nike" 标签及其权重而获得更高权重。
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

请注意，特殊负载字段会被忽略用于包含复杂操作符的全文查询。它仅适用于简单的词袋查询。

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

每列缓冲区大小。可选，默认为空（自动推断大小）。仅适用于 `odbc`、`mssql` 源类型。

ODBC 和 MS SQL 驱动有时无法返回预期的最大实际列大小。例如，`NVARCHAR(MAX)` 列总是向 `indexer` 报告其长度为 2147483647 字节，尽管实际使用的长度可能远小于此。然而，接收缓冲区仍需预先分配，并且必须确定其大小。当驱动根本不报告列长度时，Manticore 会为每个非字符列分配默认 1 KB 缓冲区，为每个字符列分配 1 MB 缓冲区。驱动报告的列长度也会被限制在 8 MB 的上限，因此如果驱动报告（几乎）2 GB 的列长度，则会被限制，并为该列分配 8 MB 缓冲区。这些硬编码限制可以通过 `sql_column_buffers` 指令覆盖，既可以节省实际较短列的内存，也可以突破实际较长列的 8 MB 限制。指令值必须是选定列名和大小的逗号分隔列表：

示例：

```ini
sql_query = SELECT id, mytitle, mycontent FROM documents
sql_column_buffers = mytitle=64K, mycontent=10M
```
<!-- proofread -->

