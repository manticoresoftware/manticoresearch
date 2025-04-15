# Section "Common" in configuration


lemmatizer_base

----------------


`lemmatizer_base` 是一个可选的配置指令，用于指定词形还原词典的基础路径。默认路径为 `/usr/share/manticore`


Manticore Search 中的词形还原实现（请参见 [Morphology](../Creating_a_table/NLP_and_tokenization/Morphology.md) 了解什么是词形还原器）是基于字典的，并且需要不同语言的特定字典文件。这些文件可以从 Manticore 网站下载 (<https://manticoresearch.com/install/#other-downloads>).


示例：


```ini

lemmatizer_base = /usr/share/manticore/

```


progressive_merge

------------------


`progressive_merge` 是一个配置指令，当启用时，它会将实时表的磁盘块从小块合并到大块。这种方法加速了合并过程并减少了读/写放大。默认情况下，此设置是启用的。如果禁用，块将按照创建的顺序进行合并。


json_autoconv_keynames

------------------------


`json_autoconv_keynames` 是一个可选配置指令，决定是否以及如何自动转换 JSON 属性中的键名。已知值为 'lowercase'。默认情况下，此设置未指定（意味着不进行转换）。


当设置为 lowercase 时，JSON 属性中的键名将在索引过程中自动转换为小写。此转换适用于所有数据源中的 JSON 属性，包括 SQL 和 XMLpipe2。


示例：


```ini

json_autoconv_keynames = lowercase

```


json_autoconv_numbers

-----------------------


`json_autoconv_numbers` 是一个可选的配置指令，决定是否自动检测并转换表示数字的 JSON 字符串为数值属性。默认值为 0（不将字符串转换为数字）。


当该选项设置为 1 时，像 "1234" 这样的值将被索引为数字而不是字符串。如果该选项设置为 0，则这些值将被索引为字符串。此转换适用于所有数据源中的 JSON 属性，包括 SQL 和 XMLpipe2。


示例：


```ini

json_autoconv_numbers = 1

```
