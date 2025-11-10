# 从 TSV,CSV 获取数据

TSV/CSV 是传递数据给 Manticore 索引器的最简单方式。该方法是由于 xmlpipe2 的限制而创建的。在 xmlpipe2 中，索引器必须将 XML 文件中的每个属性和字段标签映射到相应的模式元素。此映射需要时间，并且随着模式中字段和属性数量的增加而增加。TSV/CSV 没有此类问题，因为每个字段和属性对应于 TSV/CSV 文件中的特定列。在某些情况下，TSV/CSV 的工作速度可能略快于 xmlpipe2。

## 文件格式
TSV/CSV 文件的第一列必须是文档 ID。其余列必须与模式定义中字段和属性的声明相对应。请注意，您不需要在模式中声明文档 ID，因为它始终被视为存在，应该位于第一列，并且需要是唯一的带符号正非零 64 位整数。

tsvpipe 和 csvpipe 之间的区别在于分隔符和引用规则。tsvpipe 的分隔符是硬编码的制表符，并且没有引用规则。csvpipe 有 `csvpipe_delimiter` 选项用于分隔符，默认值为逗号，并且还有引用规则，例如：

* 任何字段都可以被引用
* 包含换行符、双引号或逗号的字段应被引用
* 字段中的双引号字符必须用两个双引号字符表示

## TSV 流声明
`tsvpipe_command` 指令是必需的，包含调用的 shell 命令以生成被索引的 TSV 流。该命令可以读取 TSV 文件，也可以是一个动态生成制表符分隔内容的程序。

## TSV 索引列

以下指令可用于声明索引列的类型：

* `tsvpipe_field` - 声明一个 `text` 字段。
* `tsvpipe_field_string` - 声明一个文本字段/字符串属性。该列既作为文本字段索引，也作为字符串属性存储。
* `tsvpipe_attr_uint` - 声明一个整数属性。
* `tsvpipe_attr_timestamp` - 声明一个时间戳属性。
* `tsvpipe_attr_bool` - 声明一个布尔属性。
* `tsvpipe_attr_float` - 声明一个浮点属性。
* `tsvpipe_attr_bigint` - 声明一个大整数属性。
* `tsvpipe_attr_multi` - 声明一个多值整数属性。
* `tsvpipe_attr_multi_64` - 声明一个多值 64 位整数属性。
* `tsvpipe_attr_string` - 声明一个字符串属性。
* `tsvpipe_attr_json` - 声明一个 JSON 属性。


使用 TSV 文件的源示例：

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

## CSV 流声明
`csvpipe_command` 指令是必需的，包含调用的 shell 命令以生成被索引的 CSV 流。该命令可以读取 CSV 文件，也可以是一个动态生成逗号分隔内容的程序。

## CSV 索引列

以下指令可用于声明索引列的类型：

* `csvpipe_field` - 声明一个 `text` 字段。
* `csvpipe_field_string` - 声明一个文本字段/字符串属性。该列既作为文本字段索引，也作为字符串属性存储。
* `csvpipe_attr_uint` - 声明一个整数属性。
* `csvpipe_attr_timestamp` - 声明一个时间戳属性。
* `csvpipe_attr_bool` - 声明一个布尔属性。
* `csvpipe_attr_float` - 声明一个浮点属性。
* `csvpipe_attr_bigint` - 声明一个大整数属性。
* `csvpipe_attr_multi` - 声明一个多值整数属性。
* `csvpipe_attr_multi_64` - 声明一个多值 64 位整数属性。
* `csvpipe_attr_string` - 声明一个字符串属性。
* `csvpipe_attr_json` - 声明一个 JSON 属性。

使用 CSV 文件的源示例：

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

