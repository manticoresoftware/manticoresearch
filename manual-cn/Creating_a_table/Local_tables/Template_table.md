# 模板表

<!-- example template -->

模板表是Manticore中的一种特殊类型的表，它不存储任何数据，也不会在您的磁盘上创建任何文件。

尽管如此，它可以具有与普通表或实时表相同的[NLP设置](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#inplace_reloc_factor)。模板表可以用于以下目的：

- 作为模板，在[普通模式](../../Creating_a_table/Local_tables.md#Defining-table-schema-in-config-%28Plain mode%29)中继承设置，从而简化您的Manticore配置文件。
- 借助[CALL KEYWORDS](../../Searching/Autocomplete.md#CALL-KEYWORDS)命令进行关键词生成。
- 使用[CALL SNIPPETS](../../Searching/Highlighting.md#CALL-SNIPPETS)命令高亮显示任意字符串。


<!-- intro -->
##### 通过配置文件创建模板表：

<!-- request CONFIG -->

```ini
table template {
  type = template
  morphology = stem_en
  wordforms = wordforms.txt
  exceptions = exceptions.txt
  stopwords = stopwords.txt
}
```
<!-- end -->
<!-- proofread -->