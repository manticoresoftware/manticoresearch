# 处理抓取的数据

默认情况下，`sql_query`结果集中的第一列被索引为文档ID。

文档ID *必须* 是第一个字段，并且必须是唯一的有符号（非零）整数，范围从 -9223372036854775808 到 9223372036854775807。

您可以指定最多 256 个全文字段和任意数量的属性。所有既不是文档ID（第一个）也不是属性的列将被索引为全文字段。


## 属性声明：

### sql_attr_bigint

声明一个 64 位 *有符号* 整数。

### sql_attr_bool

声明一个布尔属性。它相当于一个位数为 1 的整数属性。

### sql_attr_float

声明一个浮点属性。

值将以单精度、32 位 IEEE 754 格式存储。表示的范围大约从 1e-38 到 1e+38。可以精确存储的十进制位数大约为 7。

浮点属性的一个重要用途是存储纬度和经度值（以弧度表示），以便在查询时进行地球球面距离计算。

### sql_attr_json

声明一个 JSON 属性。

在索引 JSON 属性时，Manticore 期望一个包含 JSON 格式数据的文本字段。JSON 属性支持任意 JSON 数据，没有嵌套层级或类型的限制。

### sql_attr_multi

声明一个多值属性。

普通属性只允许为每个文档附加 1 个值。但是，在某些情况下（例如标记或类别），希望附加同一属性的多个值，并能够对值列表进行过滤或分组。

MVA 可以从一列中获取值（像其他数据类型一样）- 在这种情况下，结果集中的列必须提供用逗号分隔的多个整数值的字符串- 或通过运行单独的查询来获取值。

在执行查询时，引擎运行查询，按 ID 对结果进行分组，并将值分配给表中相应的文档。未在表中找到ID的值将被丢弃。在执行查询之前，任何定义的 `sql_query_pre_all` 将被运行。

sql_attr_multi 的声明格式如下：

```ini
sql_attr_multi = ATTR-TYPE ATTR-NAME 'from' SOURCE-TYPE \
    [;QUERY] \
    [;RANGED-QUERY]
```

其中

* ATTR-TYPE 是 `uint`，`bigint` 或 `timestamp`。
* SOURCE-TYPE 是 `field`，`query`， `ranged-query` 或 `ranged-main-query`。
* QUERY 是一个可选的 SQL 查询，用于获取所有 (docid, attrvalue) 对。
* RANGED-QUERY 是一个可选的 SQL 查询，用于获取最小和最大 ID 值，类似于 `sql_query_range`。
* 反斜杠仅用于清晰；所有内容也可以在单行中声明。

它与 ranged-query SOURCE-TYPE 一起使用。如果使用 `ranged-main-query` SOURCE-TYPE，则省略 RANGED-QUERY，它将自动使用来自 `sql_query_range` 的相同查询（在复杂继承设置中节省了手动重复同一查询多次的有用选项）。

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

时间戳可以存储的日期和时间范围为 1970 年 1 月 1 日到 2038 年 1 月 19 日，精度为一秒。期望的列值应为 UNIX 格式的时间戳，作为自 1970 年 1 月 1 日午夜以来经过的秒数，这是一个 32 位无符号整数。时间戳在内部以整数形式存储和处理。在将时间戳作为整数使用的同时，您还可以使用不同的基于日期的函数，例如时间段排序模式或用于 GROUP BY 的日期/周/月/年提取。

请注意，MySQL 中的 DATE 或 DATETIME 列类型不能直接用作 Manticore 中的时间戳属性；您需要使用 UNIX_TIMESTAMP 函数显式转换这些列（如果数据在范围内）。

请注意，时间戳不能表示 1970 年 1 月 1 日之前的日期，MySQL 中的 UNIX_TIMESTAMP() 将不会返回任何预期的值。如果您只需处理日期而不是时间，请考虑 MySQL 中的 `TO_DAYS()` 函数。


### sql_attr_uint

声明一个无符号整数属性。

您可以通过在属性名称后附加 ':BITCOUNT' 来指定整数属性的位数（见下面的示例）。小于默认 32 位大小的属性或位字段性能较慢。

```ini
sql_attr_uint = group_id
sql_attr_uint = forum_id:9 # 论坛_id 的 9 位
```

### sql_field_string

声明一个组合字符串属性/文本字段。值将作为全文字段进行索引，但也会以相同名称存储在字符串属性中。请注意，仅当您确定希望该字段在全文方式和作为属性可搜索（能够按其进行排序和分组）时，才应使用它。如果您只想能够获取字段的原始值，则不需要对其进行任何操作，除非您明确地通过 [stored_fields](../../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#stored_fields) 从存储字段列表中删除了该字段。

```ini
sql_field_string = name
```

### sql_file_field

声明一个文件基字段。
此指令使索引器将字段内容解释为文件名，并加载和处理所引用的文件。大于 max_file_field_buffer 大小的文件将被跳过。在文件加载期间的任何错误（IO 错误、超出限制等）将被报告为索引警告，并且不会提前终止索引。对于此类文件，将不会有内容被索引。

```ini
sql_file_field = field_name
```

### sql_joined_field

联接/负载字段提取查询。多值， 可选，默认是空查询列表。

`sql_joined_field` 允许您使用两种不同的功能：联接字段和负载（负载字段）。其语法如下：

```ini
sql_joined_field = FIELD-NAME 'from'  ( 'query' | 'payload-query' | 'ranged-query' | 'ranged-main-query' ); \
		QUERY [ ; RANGE-QUERY ]
```

其中

* FIELD-NAME 是一个联接/负载字段名
* QUERY 是一个 SQL 查询，必须提取用于进一步处理的值
* RANGE-QUERY 是一个可选的 SQL 查询，用于提取要处理的值范围

**联接字段** 使您能够避免在主文档提取查询（sql_query）中使用 JOIN 和/或 GROUP_CONCAT 语句。当 SQL 侧的 JOIN 很慢，或需要在 Manticore 侧离线处理，或仅仅是为了模拟 MySQL 特定的 `GROUP_CONCAT` 功能（如果您的数据库服务器不支持它）时，这非常有用。

查询必须返回恰好 2 列：文档 ID 和要附加到联接字段的文本。文档 ID 可以重复，但必须按升序排列。为给定 ID 提取的所有文本行将会被连接在一起，连接结果将作为联接字段的整个内容进行索引。行将按照查询返回的顺序进行连接，并在它们之间插入空格。例如，如果联接字段查询返回以下行：
```ini
( 1, 'red' )
( 1, 'right' )
( 1, 'hand' )
( 2, 'mysql' )
( 2, 'manticore' )
```

那么索引结果将等同于向文档 1 添加一个值为 'red right hand' 的新文本字段，以及向文档 2 添加一个值为 'mysql sphinx' 的字段，包括字段中关键字位置的顺序。如果行需要按特定顺序排列，必须在查询中明确规定。

联接字段的索引方式不同而已。在联接字段和常规文本字段之间没有其他差异。

在执行联接字段查询之前，会运行任何 set 的 `sql_query_pre_all`，如果存在。这允许您设置期望的编码等。

当单个查询效率不够或由于数据库驱动程序的限制而无法工作时，可以使用范围查询。它的工作方式类似于主索引循环中的范围查询。范围将被提前查询并提取一次，然后将运行多个带有不同 `$start` 和 `$end` 替换的查询以提取实际数据。

当使用 `ranged-main-query` 查询时，请省略 `ranged-query`，它将自动使用来自 `sql_query_range` 的相同查询（在复杂的继承设置中，将相同查询手动复制多次是一个有用的选项）。

<!-- example payload -->
**负载** 让您创建一个特殊字段，其中存储的是所谓的用户负载，而不是关键字位置。负载是附加到每个关键字的自定义整数值。它们可以在搜索时用于影响排名。

负载查询必须返回恰好 3 列：
- 文档 ID
- 关键字
- 和整数负载值。

文档 ID 可以重复，但必须按升序排列。负载 **必须** 是24位范围内的无符号整数，即从 0 到 16777215。

唯一考虑负载的排名器是 `proximity_bm25`（默认 [ranker](../../Searching/Sorting_and_ranking.md#Available-built-in-rankers)）。在具有负载字段的表中，它将自动切换到一个变体，以匹配那些字段中的关键字，计算与字段权重相乘的匹配负载的总和，并将该总和添加到最终排名中。

请注意，负载字段在包含复杂运算符的全文查询中被忽略。它仅适用于简单的词袋查询。

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

请注意，当您搜索 `nike | adidas` 时，包含 "nike" 的结果由于 "nike" 标签及其来源于负载查询的权重而获得更高的权重。
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
请注意，特殊有效载荷字段在包含复杂运算符的全文查询中被忽略。它仅适用于简单的词袋查询。


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

