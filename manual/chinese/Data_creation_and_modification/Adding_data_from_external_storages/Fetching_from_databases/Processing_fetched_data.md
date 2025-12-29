# 处理获取的数据

默认情况下，`sql_query`结果集的第一列被索引为文档ID。

文档ID *MUST* 是非常第一个字段，并且 **MUST BE** 唯一的非零整数，范围从 -9223372036854775808 到 9223372036854775807。

你可以指定最多256个全文字段和任意数量的属性。所有既不是文档ID（第一个字段）也不是属性的列将被索引为全文字段。


## 属性声明：

### sql_attr_bigint

声明一个64位 *signed* 整数。

### sql_attr_bool

声明一个布尔属性。它等同于一个位数为1的整数属性。

### sql_attr_float

声明一个浮点属性。

值将以单精度，32位IEEE 754格式存储。表示范围大约是从1e-38到1e+38。可以精确存储的大约7位小数。

浮点属性的一个重要用途是存储以弧度表示的纬度和经度值，以便在查询时进行地理球体距离计算。

### sql_attr_json

声明一个JSON属性。

在索引JSON属性时，Manticore期望一个包含JSON格式数据的文本字段。JSON属性支持任意JSON数据，没有嵌套级别或类型的限制。

### sql_attr_multi

声明一个多值属性。

普通属性仅允许每个文档附加一个值。然而，在某些情况下（如标签或类别），希望附加多个相同属性的值，并能够对值列表进行过滤或分组。

MVA可以从列中获取值（如其他数据类型一样）——在这种情况下，结果集中的列必须提供用逗号分隔的多个整数值的字符串——或者通过运行单独的查询来获取值。

在执行查询时，引擎会运行查询，按ID分组结果，并将值分配给表中的相应文档。ID未在表中找到的值将被丢弃。在执行查询之前，任何定义的 `sql_query_pre_all` 将被运行。

sql_attr_multi 的声明格式如下：

```ini
sql_attr_multi = ATTR-TYPE ATTR-NAME 'from' SOURCE-TYPE \
    [;QUERY] \
    [;RANGED-QUERY]
```

其中

* ATTR-TYPE 是 `uint`，`bigint` 或 `timestamp`。
* SOURCE-TYPE 是 `field`，`query`，`ranged-query` 或 `ranged-main-query`。
* QUERY 是一个可选的SQL查询，用于获取所有 (docid, attrvalue) 对。
* RANGED-QUERY 是一个可选的SQL查询，用于获取最小和最大ID值，类似于 `sql_query_range`。
* 退格符只是为了清晰而包含的；所有内容也可以在单行中声明。

它用于 ranged-query SOURCE-TYPE。如果使用 `ranged-main-query` SOURCE-TYPE，则省略 RANGED-QUERY，并将自动使用与 `sql_query_range` 相同的查询（在复杂的继承设置中很有用，可以节省手动多次复制相同查询的麻烦）。

```ini
sql_attr_multi = uint tag from field
sql_attr_multi = uint tag from query; SELECT id, tag FROM tags
sql_attr_multi = bigint tag from ranged-query; \
    SELECT id, tag FROM tags WHERE id>=$start AND id<=$end; \
    SELECT MIN(id), MAX(id) FROM tags
```

### sql_attr_string

声明一个字符串属性。每个值的最大大小固定为4GB。

### sql_attr_timestamp

声明一个UNIX时间戳。

时间戳可以存储从1970年1月1日到2038年1月19日之间的日期和时间，精度为一秒。预期的列值应为UNIX格式的时间戳，即自1970年1月1日格林尼治标准时间午夜以来的32位无符号整数秒数。时间戳在内部以整数形式存储和处理。除了以整数形式处理时间戳外，还可以使用不同的日期函数，如时间段排序模式或使用 GROUP BY 进行日/周/月/年提取。

请注意，MySQL中的DATE或DATETIME列不能直接用作Manticore的时间戳属性；需要显式使用UNIX_TIMESTAMP函数（如果数据在范围内）进行转换。

请注意，时间戳不能表示1970年1月1日之前的日期，而MySQL中的UNIX_TIMESTAMP()将不会返回预期的结果。如果你只需要处理日期，而不是时间，可以考虑使用MySQL中的TO_DAYS()函数。


### sql_attr_uint

声明一个无符号整数属性。

可以通过在属性名称后附加 `:BITCOUNT` 来指定整数属性的位数（参见下面的示例）。具有小于默认32位大小或位字段的属性会变慢。

```ini
sql_attr_uint = group_id
sql_attr_uint = forum_id:9 # 9 bits for forum_id
```

### sql_field_string

声明一个组合字符串属性/文本字段。值将作为全文字段索引，但也会以相同名称存储在字符串属性中。请注意，只有在确定希望该字段以全文方式和属性方式（具有排序和分组的能力）进行搜索时，才应使用此功能。如果你只是想获取字段的原始值，除非你隐式地通过 [stored_fields](../../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#stored_fields) 从存储字段列表中删除了该字段，否则不需要为此做任何操作。

```ini
sql_field_string = name
```

### sql_file_field

声明一个基于文件的字段。

此指令使索引器将字段内容解释为文件名，并加载和处理所引用的文件。大小超过 max_file_field_buffer 的文件将被跳过。在文件加载过程中（I/O错误、超出限制等）发生的任何错误将作为索引警告报告，并不会提前终止索引。此类文件不会被索引内容。

```ini
sql_file_field = field_name
```

### sql_joined_field

联合/负载字段获取查询。多值，可选，默认为空查询列表。

`sql_joined_field` 允许使用两种不同的功能：联合字段和负载（负载字段）。其语法如下：

```ini
sql_joined_field = FIELD-NAME 'from'  ( 'query' | 'payload-query' | 'ranged-query' | 'ranged-main-query' ); \
		QUERY [ ; RANGE-QUERY ]
```

其中

* FIELD-NAME 是联合/负载字段名称
* QUERY 是一个必须获取进一步处理值的SQL查询
* RANGE-QUERY 是一个可选的SQL查询，用于获取要处理的值范围

**Joined fields** 让您避免在主文档查询（sql_query）中使用 JOIN 和/或 GROUP_CONCAT 语句。当 SQL 端的 JOIN 慢，或需要卸载到 Manticore 端，或者仅仅是为了模拟 MySQL 特定的 `GROUP_CONCAT` 功能（以防您的数据库服务器不支持该功能）时，这会很有用。

查询必须恰好返回 2 列：文档 ID 和要追加到已连接字段的文本。文档 ID 可以重复，但必须是升序。对于给定 ID 获取的所有文本行将被串联在一起，串联结果将作为已连接字段的全部内容进行索引。行将按照查询返回的顺序连接，并在它们之间插入分隔空白字符。例如，如果已连接字段查询返回如下行：
```ini
( 1, 'red' )
( 1, 'right' )
( 1, 'hand' )
( 2, 'mysql' )
( 2, 'manticore' )
```

那么索引结果将等同于为文档 1 添加一个新文本字段，其值为 'red right hand'，为文档 2 添加一个新文本字段，其值为 'mysql sphinx'，其中包括关键字在字段内按照查询顺序出现的位置。如果需要特定顺序，必须在查询中明确指定。

已连接字段只在索引方式上有所不同。已连接字段和常规文本字段之间没有其他区别。

在执行已连接字段查询之前，如果存在任何 `sql_query_pre_all` 集合，将会被执行。这允许您在已连接字段上下文内设置所需的编码等。

当单个查询效率不够高或由于数据库驱动程序限制而无法工作时，可以使用范围查询。它的工作方式类似于主索引循环中的范围查询。区间将被预先查询并获取一次，然后执行多个带有不同 `$start` 和 `$end` 替换的查询以获取实际数据。

使用 `ranged-main-query` 查询时，省略 `ranged-query`，它将自动使用 `sql_query_range` 中的相同查询（在复杂继承设置中非常有用，可以避免手动多次复制相同的查询）。

<!-- example payload -->
**Payloads** 允许您创建一个特殊字段，其中不存储关键字位置，而是存储所谓的用户负载。负载是附加到每个关键字的自定义整数值。然后可以在搜索时用于影响排名。

负载查询必须返回恰好 3 列：
- 文档 ID
- 关键字
- 整数负载值。

文档 ID 可以重复，但必须是升序。负载**必须**是 24 位范围内的无符号整数，即从 0 到 16777215。

唯一考虑负载的排序器是 `proximity_bm25`（默认 [ranker](../../../Searching/Sorting_and_ranking.md#Available-built-in-rankers)）。对于具有负载字段的表，它会自动切换到一个变体，该变体匹配这些字段中的关键字，计算匹配负载乘以字段权重的总和，并将该总和加到最终排名中。

请注意，负载字段会被包含复杂操作符的全文查询忽略。它仅适用于简单的词袋查询。

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

请注意，当您搜索 `nike | adidas` 时，结果中包含 "nike" 的部分因为来自负载查询的 "nike" 标签及其权重而获得更高权重。
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

请注意，特殊的负载字段会被包含复杂操作符的全文查询忽略。它仅适用于简单的词袋查询。

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

每列缓冲大小。可选，默认为空（自动推断大小）。仅适用于 `odbc`，`mssql` 源类型。

ODBC 和 MS SQL 驱动程序有时无法返回预期的最大实际列大小。例如，`NVARCHAR(MAX)` 列总是向 `indexer` 报告其长度为 2147483647 字节，即使实际使用的长度可能远小于此。然而，接收缓冲区仍需预先分配且大小必须确定。当驱动程序根本不报告列长度时，Manticore 会为每个非字符列分配默认的 1 KB 缓冲区，为每个字符列分配 1 MB 缓冲区。驱动程序报告的列长度也会被限制在 8 MB 的上限，因此如果驱动程序报告（几乎）2 GB 的列长度，它会被限制，并为该列分配 8 MB 缓冲区。可以通过 `sql_column_buffers` 指令覆盖这些硬编码限制，以节省实际较短列的内存或突破 8 MB 限制以支持实际更长列。指令值必须为逗号分隔的选定列名和大小列表：

示例：

```ini
sql_query = SELECT id, mytitle, mycontent FROM documents
sql_column_buffers = mytitle=64K, mycontent=10M
```
<!-- proofread -->

