# 从 TSV/CSV 文件获取数据

TSV/CSV 是向 Manticore 索引器传递数据的最简单方式。该方法是由于 xmlpipe2 的限制而创建的。在 xmlpipe2 中，索引器必须将 XML 文件中的每个属性和字段标签映射到对应的模式元素。这种映射需要时间，随着模式中字段和属性的增多，所需时间也会增加。而 TSV/CSV 没有此类问题，因为每个字段和属性都对应 TSV/CSV 文件中的一个特定列。在某些情况下，TSV/CSV 的处理速度可能比 xmlpipe2 略快

## 文件格式
TSV/CSV 文件中的第一列必须是文档 ID。其余列必须与模式定义中的字段和属性声明相匹配。注意，不需要在模式中声明文档 ID，因为它始终被认为存在，必须位于第 1 列，并且需要是唯一的正 64 位有符号整数（非零）。

tsvpipe 和 csvpipe 的区别在于分隔符和引用规则。tsvpipe 使用制表符作为硬编码的分隔符，并且没有引用规则。csvpipe 则有一个 `csvpipe_delimiter` 选项用于指定分隔符，默认值为逗号，同时具有以下引用规则：

- 任何字段都可以加引号
- 包含换行符、双引号或逗号的字段必须加引号
- 字段中的双引号字符必须用两个双引号表示

## 声明 TSV 流

`tsvpipe_command` 指令是必需的，包含了用于生成要索引的 TSV 流的 shell 命令。该命令可以读取 TSV 文件，也可以是一个动态生成制表符分隔内容的程序。

## TSV 索引列

以下指令可用于声明索引列的类型：

- `tsvpipe_field` - 声明一个 `text` 字段。
- `tsvpipe_field_string` - 声明一个文本字段/字符串属性。该列将既作为文本字段索引，也会存储为字符串属性。
- `tsvpipe_attr_uint` - 声明一个整数属性。
- `tsvpipe_attr_timestamp` - 声明一个时间戳属性。
- `tsvpipe_attr_bool` - 声明一个布尔属性。
- `tsvpipe_attr_float` - 声明一个浮点数属性。
- `tsvpipe_attr_bigint` - 声明一个大整数属性。
- `tsvpipe_attr_multi` - 声明一个多值整数属性。
- `tsvpipe_attr_multi_64` - 声明一个 64 位多值整数属性。
- `tsvpipe_attr_string` - 声明一个字符串属性。
- `tsvpipe_attr_json` - 声明一个 JSON 属性。

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

## 声明 CSV 流

`csvpipe_command` 指令是必需的，包含了用于生成要索引的 CSV 流的 shell 命令。该命令可以读取 CSV 文件，也可以是一个动态生成逗号分隔内容的程序。

## CSV 索引列

以下指令可用于声明索引列的类型：

- `csvpipe_field` - 声明一个 `text` 字段。
- `csvpipe_field_string` - 声明一个文本字段/字符串属性。该列将既作为文本字段索引，也会存储为字符串属性。
- `csvpipe_attr_uint` - 声明一个整数属性。
- `csvpipe_attr_timestamp` - 声明一个时间戳属性。
- `csvpipe_attr_bool` - 声明一个布尔属性。
- `csvpipe_attr_float` - 声明一个浮点数属性。
- `csvpipe_attr_bigint` - 声明一个大整数属性。
- `csvpipe_attr_multi` - 声明一个多值整数属性。
- `csvpipe_attr_multi_64` - 声明一个 64 位多值整数属性。
- `csvpipe_attr_string` - 声明一个字符串属性。
- `csvpipe_attr_json` - 声明一个 JSON 属性。

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