# 配置中的“Common”部分

lemmatizer_base
----------------

`lemmatizer_base` 是一个可选的配置指令，用于指定词形还原器字典的基础路径。默认路径是 `/usr/share/manticore`

Manticore Search中的词形还原器实现（参见 [Morphology](../Creating_a_table/NLP_and_tokenization/Morphology.md) 了解词形还原器的相关知识）是基于字典的，需要针对不同语言的特定字典文件。这些文件可以从Manticore官网(<https://manticoresearch.com/install/#other-downloads>)下载。

示例：

```ini
lemmatizer_base = /usr/share/manticore/
```

progressive_merge
------------------

progressive_merge 是一个配置指令，启用后，会将实时表的磁盘块从小到大合并。这种方式加快了合并过程，减少了读写放大。默认情况下，此设置是启用的。如果禁用，块将按创建顺序合并。

json_autoconv_keynames
------------------------

json_autoconv_keynames 是一个可选的配置指令，用于决定是否以及如何自动转换JSON属性中的键名。已知的取值为 'lowercase'。默认情况下，此设置未指定（意味着不进行转换）。

当设置为 lowercase 时，JSON属性中的键名将在索引时自动转换为小写。此转换适用于来自所有数据源（包括SQL和XMLpipe2）的JSON属性。

示例：

```ini
json_autoconv_keynames = lowercase
```

json_autoconv_numbers
-----------------------

json_autoconv_numbers 是一个可选的配置指令，用于决定是否自动检测并转换表示数字的JSON字符串为数值属性。默认值为0（不将字符串转换为数字）。

当该选项设置为1时，诸如 "1234" 的值将作为数字索引而非字符串。如果设置为0，则这些值将作为字符串索引。此转换适用于来自所有数据源（包括SQL和XMLpipe2）的JSON属性。

示例：

```ini
json_autoconv_numbers = 1
```

on_json_attr_error
---------------------

on_json_attr_error 是一个可选的配置指令，用于指定发现JSON格式错误时的处理措施。默认值为 `ignore_attr`（忽略错误）。此设置仅适用于 `sql_attr_json` 属性。

默认情况下，JSON格式错误将被忽略（`ignore_attr`），索引工具会显示警告。将此选项设置为 `fail_index` 会导致在遇到第一个JSON格式错误时索引失败。

示例：

```ini
on_json_attr_error = ignore_attr
```

plugin_dir
-----------

plugin_dir 是一个可选的配置指令，用于指定动态库（UDF）信任位置。默认路径为 `/usr/local/lib/manticore/`。

该指令设置了加载 [UDF库](../Extensions/UDFs_and_Plugins/UDF.md)的信任目录。

示例：

```ini
plugin_dir = /usr/local/lib/manticore/
```
<!-- proofread -->

