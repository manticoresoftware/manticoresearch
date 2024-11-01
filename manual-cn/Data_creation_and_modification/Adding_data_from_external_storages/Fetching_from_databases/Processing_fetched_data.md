# 处理获取的数据

默认情况下，`sql_query` 的结果集中的第一列会被索引为文档ID。

文档ID**必须**是结果集中的第一个字段，并且**必须是**唯一的已签名（非零）整数，取值范围在 -9223372036854775808 到 9223372036854775807 之间。

您最多可以指定256个全文字段和任意数量的属性。所有既不是文档ID（第一列）也不是属性的列都将被索引为全文字段。


## 属性声明：

### sql_attr_bigint

声明一个64位*有符号*整数属性。

### sql_attr_bool

声明一个布尔属性。相当于位数为1的整数属性。

### sql_attr_float

声明一个浮点属性。

数值将以32位IEEE 754格式的单精度浮点数进行存储。表示范围大约在1e-38到1e+38之间。可以精确存储的小数位数约为7位。

浮点属性的一个重要用途是存储经度和纬度（以弧度表示），用于后续在查询时进行地球球面距离计算。

### sql_attr_json

声明一个JSON属性。

在索引JSON属性时，Manticore期望一个包含JSON格式数据的文本字段。JSON属性支持任意JSON数据，没有嵌套层级或类型的限制。

### sql_attr_multi

声明一个多值属性（MVA）。

普通属性仅允许为每个文档附加1个值。但在某些情况下（如标签或类别），希望附加多个同一属性的值，并能够对值列表进行过滤或分组。

MVA可以从列中获取值（如其他数据类型一样）——在这种情况下，结果集中的列应提供一个以逗号分隔的多个整数值的字符串——或者通过运行单独的查询获取值。

当执行查询时，引擎运行该查询，按ID对结果进行分组，并将值分配给表中相应的文档。未在表中找到ID的值将被丢弃。在执行查询之前，任何定义的 `sql_query_pre_all` 都会被执行。

`sql_attr_multi` 的声明格式如下：

```ini
sql_attr_multi = ATTR-TYPE ATTR-NAME 'from' SOURCE-TYPE \
    [;QUERY] \
    [;RANGED-QUERY]
```

其中：

- ATTR-TYPE 是 `uint`, `bigint` 或 `timestamp`。
- SOURCE-TYPE 是 `field`, `query`, `ranged-query`, 或 `ranged-main-query`。
- QUERY 是一个可选的SQL查询，用于获取所有(docid, attrvalue)对。
- RANGED-QUERY 是一个可选的SQL查询，用于获取最小和最大ID值，类似于 `sql_query_range`。
- 反斜杠仅用于提高可读性；所有内容也可以在单行中声明。

当使用 `ranged-query` 作为SOURCE-TYPE时，使用 RANGED-QUERY 来定义范围查询。如果使用 `ranged-main-query` 作为SOURCE-TYPE，则省略 RANGED-QUERY，它会自动使用 `sql_query_range` 中的相同查询（在复杂的继承设置中，这是一种有用的选项，可以避免手动重复多次相同的查询）。

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

时间戳可以存储从1970年1月1日到2038年1月19日的日期和时间，精确到秒。预期的列值应该是UNIX格式的时间戳，即自1970年1月1日午夜（GMT）以来经过的秒数，这是一个32位无符号整数。时间戳在内部被作为整数存储和处理。除了以整数形式使用时间戳之外，您还可以结合不同的基于日期的函数使用它们，例如时间段排序模式或日/周/月/年提取进行分组操作。

请注意，在MySQL中，DATE或DATETIME列类型不能直接用作Manticore中的时间戳属性；您需要显式地使用 UNIX_TIMESTAMP 函数进行转换（如果数据在范围内）。

请注意，时间戳不能表示1970年1月1日之前的日期，而MySQL中的 UNIX_TIMESTAMP() 函数将不会返回预期值。如果只需要处理日期而不涉及时间，请考虑使用MySQL中的 `TO_DAYS()` 函数。


### sql_attr_uint

声明一个无符号整数属性。

您可以通过在属性名后附加 `:BITCOUNT` 来指定整数属性的位数（参见下面的示例）。位数少于默认32位的属性或位字段性能较慢。

```ini
sql_attr_uint = group_id
sql_attr_uint = forum_id:9 # 9 bits for forum_id
```

### sql_field_string

声明一个组合字符串属性/全文字段。该值将作为全文字段被索引，同时也会以相同名称存储在字符串属性中。请注意，仅当您确定需要该字段既以全文方式可搜索又能作为属性（用于排序和分组）时才使用它。如果您只希望能够获取字段的原始值，而不进行其他操作，则不需要为其做任何设置，除非您通过 [stored_fields](../../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#stored_fields) 显式移除了该字段。

```ini
sql_field_string = name
```

### sql_file_field

声明基于文件的字段。

此指令会使索引器将字段内容视为文件名，并加载和处理指定的文件。大于 `max_file_field_buffer` 的文件将被跳过。文件加载过程中发生的任何错误（IO错误、超过限制等）将被报告为索引警告，不会导致索引过程提前终止。这些文件的内容不会被索引。

```ini
sql_file_field = field_name
```

### sql_joined_field

连接/载荷字段的获取查询。多值，可选，默认值为空列表。

`sql_joined_field` 允许使用两种不同的功能：连接字段和载荷字段。其语法如下：

```ini
sql_joined_field = FIELD-NAME 'from'  ( 'query' | 'payload-query' | 'ranged-query' | 'ranged-main-query' ); \
		QUERY [ ; RANGE-QUERY ]
```

其中：

- FIELD-NAME 是连接/载荷字段名称。
- QUERY 是一个SQL查询，用于获取要进一步处理的值。
- RANGE-QUERY 是一个可选的SQL查询，用于获取要处理的值的范围。

**连接字段** 允许避免在主文档获取查询（`sql_query`）中使用 JOIN 和/或 GROUP_CONCAT 语句。这在SQL端JOIN速度较慢、需要在Manticore端进行分担，或者数据库服务器不支持 `GROUP_CONCAT` 功能时非常有用。

查询必须返回完全两列：文档ID和要附加到连接字段的文本。文档ID可以重复，但必须按升序排列。对于给定ID获取的所有文本行将被连接在一起，连接结果将作为连接字段的全部内容被索引。返回的行将按照查询中的顺序进行连接，并在它们之间插入空格。例如，如果连接字段查询返回以下行：

```ini
( 1, 'red' )
( 1, 'right' )
( 1, 'hand' )
( 2, 'mysql' )
( 2, 'manticore' )
```

然后，索引结果将等同于向文档1添加一个新文本字段，其值为 'red right hand'，向文档2添加一个新文本字段，其值为 'mysql sphinx'，并保留查询中关键字在字段内的顺序。如果这些行需要按特定顺序排列，则需要在查询中显式定义该顺序。

连接字段仅在索引时有所不同。除此之外，连接字段与普通的文本字段没有其他区别。

在执行连接字段查询之前，如果存在 `sql_query_pre_all`，将首先运行它们。这允许您在连接字段的上下文中设置所需的编码等。

当单个查询效率不够高或由于数据库驱动程序限制而无法使用时，可以使用范围查询。它的工作方式类似于主索引循环中的范围查询。范围查询会预先获取一次，然后运行多个带有不同 `$start` 和 `$end` 替换的查询以获取实际数据。

当使用 `ranged-main-query` 查询时，可以省略 `ranged-query`，它会自动使用 `sql_query_range` 中的相同查询（这是一个有用的选项，在复杂的继承设置中可以避免手动多次重复相同的查询）。

<!-- example payload -->**载荷字段** 允许您创建一个特殊字段，其中不是存储关键字的位置，而是存储所谓的用户载荷。载荷是附加到每个关键字的自定义整数值，之后可以在搜索时使用这些载荷来影响排名。

载荷查询必须返回正好三列：

- 文档ID
- 关键字
- 整数载荷值

文档ID可以重复，但必须按升序排列。载荷**必须**是24位范围内的无符号整数，即从0到16777215。

唯一考虑载荷的排名算法是 `proximity_bm25`（这是默认的[排名算法](../../Searching/Sorting_and_ranking.md#Available-built-in-rankers)）。在包含载荷字段的表上，它将自动切换到一个变体，该变体匹配这些字段中的关键字，计算匹配的载荷之和，并乘以字段权重，将该和加到最终的排名中。

请注意，对于包含复杂运算符的全文查询，载荷字段将被忽略。它仅适用于简单的词袋查询。

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

注意，当您搜索 `nike | adidas` 时，包含 "nike" 的结果由于来自载荷查询的 "nike" 标签及其权重，会得到较高的权重。
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

请注意，对于包含复杂运算符的全文查询，特殊的载荷字段会被忽略。它仅适用于简单的词袋查询。

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

每列的缓冲区大小设置。可选，默认值为空（自动推导大小）。适用于 `odbc` 和 `mssql` 源类型。

ODBC 和 MS SQL 驱动有时无法返回期望的列的最大实际大小。例如，`NVARCHAR(MAX)` 列总是报告其长度为 2147483647 字节，尽管实际使用的长度可能小得多。然而，接收缓冲区仍需预先分配，并且必须确定其大小。当驱动程序完全不报告列的长度时，Manticore 会为每个非字符列分配默认的 1 KB 缓冲区，为每个字符列分配 1 MB 缓冲区。驱动程序报告的列长度也会受到 8 MB 上限的限制，因此如果驱动报告的列长度接近 2 GB，该列的缓冲区会被限制为 8 MB。可以使用 `sql_column_buffers` 指令覆盖这些硬编码的限制，以节省内存或突破 8 MB 限制。指令值必须是选定列名和大小的逗号分隔列表：

示例：

```ini
sql_query = SELECT id, mytitle, mycontent FROM documents
sql_column_buffers = mytitle=64K, mycontent=10M
```
<!-- proofread -->
