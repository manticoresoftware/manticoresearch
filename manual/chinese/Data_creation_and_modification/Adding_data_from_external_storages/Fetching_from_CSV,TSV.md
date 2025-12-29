# 从TSV、CSV获取数据

TSV/CSV是将数据传递给Manticore索引器的最简单方式。这种方法是因为xmlpipe2的限制而创建的。在xmlpipe2中，索引器必须将XML文件中的每个属性和字段标签映射到相应的模式元素。这种映射需要时间，并且随着模式中字段和属性数量的增加而增加。TSV/CSV没有这样的问题，因为每个字段和属性都对应于TSV/CSV文件中的一个特定列。在某些情况下，TSV/CSV可能比xmlpipe2稍微快一些。

## 文件格式
TSV/CSV文件的第一列必须是文档ID。其余列必须镜像模式定义中字段和属性的声明。请注意，您不需要在模式中声明文档ID，因为它总是被视为存在，位于第一列，并且需要是一个唯一的带符号正整数64位整数。

tspipe和csvpipe之间的区别在于分隔符和引用规则。tspipe使用制表符作为硬编码分隔符，没有引用规则。csvpipe有一个`csvpipe_delimiter`选项用于分隔符，默认值为逗号，还具有引用规则，例如：

* 任何字段都可以被引用
* 包含换行符、双引号或逗号的字段应被引用
* 字段中的双引号字符必须由两个双引号字符表示

## TSV流的声明
`tsvpipe_command`指令是必需的，包含用于生成要索引的TSV流的shell命令。该命令可以读取一个TSV文件，但它也可以是一个程序，实时生成制表符分隔的内容。

## TSV索引列

以下指令可用于声明索引列的类型：

* `tsvpipe_field` - 声明一个`text`字段。
* `tsvpipe_field_string` - 声明一个文本字段/字符串属性。该列将同时作为文本字段索引并存储为字符串属性。
* `tsvpipe_attr_uint` - 声明一个整数属性。
* `tsvpipe_attr_timestamp` - 声明一个时间戳属性。
* `tsvpipe_attr_bool` - 声明一个布尔属性。
* `tsvpipe_attr_float` - 声明一个浮点属性。
* `tsvpipe_attr_bigint` - 声明一个大整数属性。
* `tsvpipe_attr_multi` - 声明一个带有整数值的多值属性。
* `tsvpipe_attr_multi_64` - 声明一个带有64位整数值的多值属性。
* `tsvpipe_attr_string` - 声明一个字符串属性。
* `tsvpipe_attr_json` - 声明一个JSON属性。


使用TSV文件的示例源代码：

```ini
source tsv_test
{
    type = tsvpipe
    tsvpipe_command = cat /tmp/rock_bands.tsv
    tsvpipe_field = name
    tsvpipe_attr_multi = genre_tags
}
```

```ini
1   Led Zeppelin    35,23,16
2   Deep Purple 35,92
3   Frank Zappa 35,23,16,92,33,24
```

## CSV流的声明
`csvpipe_command`指令是必需的，包含用于生成要索引的CSV流的shell命令。该命令可以仅读取一个CSV文件，但它也可以是一个程序，实时生成逗号分隔的内容。

## CSV索引列

以下指令可用于声明索引列的类型：

* `csvpipe_field` - 声明一个`text`字段。
* `csvpipe_field_string` - 声明一个文本字段/字符串属性。该列将同时作为文本字段索引并存储为字符串属性。
* `csvpipe_attr_uint` - 声明一个整数属性。
* `csvpipe_attr_timestamp` - 声明一个时间戳属性。
* `csvpipe_attr_bool` - 声明一个布尔属性。
* `csvpipe_attr_float` - 声明一个浮点属性。
* `csvpipe_attr_bigint` - 声明一个大整数属性。
* `csvpipe_attr_multi` - 声明一个带有整数值的多值属性。
* `csvpipe_attr_multi_64` - 声明一个带有64位整数值的多值属性。
* `csvpipe_attr_string` - 声明一个字符串属性。
* `csvpipe_attr_json` - 声明一个JSON属性。

使用CSV文件的示例源代码：

```ini
source csv_test
{
    type = csvpipe
    csvpipe_command = cat /tmp/rock_bands.csv
    csvpipe_field = name
    csvpipe_attr_multi = genre_tags
}
```

```ini
1,"Led Zeppelin","35,23,16"
2,"Deep Purple","35,92"
3,"Frank Zappa","35,23,16,92,33,24"
```
<!-- proofread -->

