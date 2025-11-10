# 配置中的“Common”部分

lemmatizer_base
----------------

`lemmatizer_base` 是一个可选的配置指令，用于指定词形还原器字典的基础路径。默认路径是 `/usr/share/manticore`

Manticore Search 中的词形还原器实现（参见 [Morphology](../Creating_a_table/NLP_and_tokenization/Morphology.md) 了解词形还原器的相关内容）是基于字典的，需要针对不同语言的特定字典文件。这些文件可以从 Manticore 网站下载（<https://manticoresearch.com/install/#other-downloads>）。

示例：

```ini
lemmatizer_base = /usr/share/manticore/
```

progressive_merge
------------------

`progressive_merge` 是一个配置指令，启用时会将实时表的磁盘块从小到大合并。这种方式加快了合并过程并减少了读写放大。默认情况下，该设置是启用的。如果禁用，块将按创建顺序合并。

json_autoconv_keynames
------------------------

`json_autoconv_keynames` 是一个可选的配置指令，用于决定是否以及如何自动转换 JSON 属性中的键名。已知的值是 'lowercase'。默认情况下，该设置未指定（意味着不进行转换）。

当设置为 lowercase 时，JSON 属性中的键名将在索引时自动转换为小写。此转换适用于所有数据源中的 JSON 属性，包括 SQL 和 XMLpipe2。

示例：

```ini
json_autoconv_keynames = lowercase
```

json_autoconv_numbers
-----------------------

`json_autoconv_numbers` 是一个可选的配置指令，用于决定是否自动检测并将表示数字的 JSON 字符串转换为数值属性。默认值为 0（不将字符串转换为数字）。

当此选项设置为 1 时，诸如 "1234" 的值将作为数字索引，而不是字符串。如果选项设置为 0，则此类值将作为字符串索引。此转换适用于所有数据源中的 JSON 属性，包括 SQL 和 XMLpipe2。

示例：

```ini
json_autoconv_numbers = 1
```

on_json_attr_error
---------------------

`on_json_attr_error` 是一个可选的配置指令，用于指定发现 JSON 格式错误时的处理动作。默认值是 `ignore_attr`（忽略错误）。此设置仅适用于 `sql_attr_json` 属性。

默认情况下，JSON 格式错误会被忽略（`ignore_attr`），索引工具会显示警告。将此选项设置为 `fail_index` 会导致在遇到第一个 JSON 格式错误时索引失败。

示例：

```ini
on_json_attr_error = ignore_attr
```

plugin_dir
-----------

`plugin_dir` 是一个可选的配置指令，用于指定动态库（UDF）的可信位置。默认路径是 `/usr/local/lib/manticore/`。

该指令设置了加载 [UDF 库](../Extensions/UDFs_and_Plugins/UDF.md) 的可信目录。

示例：

```ini
plugin_dir = /usr/local/lib/manticore/
```
<!-- proofread -->

