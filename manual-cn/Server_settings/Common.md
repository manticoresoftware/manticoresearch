# 配置中的“Common”部分

lemmatizer_base
----------------

`lemmatizer_base` 是一个可选的配置指令，指定词形还原字典的基本路径。默认路径为 `/usr/share/manticore`。

Manticore Search 中的词形还原实现（见 [词性](../Creating_a_table/NLP_and_tokenization/Morphology.md) 了解词形还原器）是基于字典的，需不同语言的特定字典文件。这些文件可以从 Manticore 网站下载（https://manticoresearch.com/install/#other-downloads）。

示例：

```ini
lemmatizer_base = /usr/share/manticore/
```

progressive_merge
------------------

`progressive_merge` 是一个配置指令，当启用时，将实时表的磁盘块从小合并为大。此方法加快了合并过程并减少了读/写放大。默认情况下，此设置是启用的。如果禁用，块将按创建顺序合并。

json_autoconv_keynames
------------------------

`json_autoconv_keynames` 是一个可选的配置指令，决定是否以及如何自动转换 JSON 属性中的键名。已知值为 'lowercase'。默认情况下，此设置未指定（意味着不进行转换）。

当设置为 lowercase 时，JSON 属性中的键名将在索引过程中自动转换为小写。这种转换适用于来自所有数据源的 JSON 属性，包括 SQL 和 XMLpipe2。

示例：

```ini
json_autoconv_keynames = lowercase
```

json_autoconv_numbers
-----------------------

`json_autoconv_numbers` 是一个可选的配置指令，决定是否自动检测并将表示数字的 JSON 字符串转换为数值属性。默认值为 0（不将字符串转换为数字）。

当此选项设置为 1 时，像 "1234" 这样的值将作为数字而非字符串进行索引。如果选项设置为 0，则此类值将作为字符串进行索引。这种转换适用于来自所有数据源的 JSON 属性，包括 SQL 和 XMLpipe2。

示例：

```ini
json_autoconv_numbers = 1
```

on_json_attr_error
---------------------

`on_json_attr_error` 是一个可选的配置指令，指定在发现 JSON 格式错误时采取的措施。默认值为 `ignore_attr`（忽略错误）。此设置仅适用于 `sql_attr_json` 属性。

默认情况下，JSON 格式错误被忽略（`ignore_attr`），索引工具会显示警告。将此选项设置为 `fail_index` 将导致在遇到第一个 JSON 格式错误时索引失败。

示例：

```ini
on_json_attr_error = ignore_attr
```

plugin_dir
-----------

`plugin_dir` 是一个可选的配置指令，指定动态库（UDFs）的可信位置。默认路径为 `/usr/local/lib/manticore/`。

此指令设置可以加载 [UDF 库](../Extensions/UDFs_and_Plugins/UDF.md) 的可信目录。

示例：

```ini
plugin_dir = /usr/local/lib/manticore/
```
<!-- proofread -->