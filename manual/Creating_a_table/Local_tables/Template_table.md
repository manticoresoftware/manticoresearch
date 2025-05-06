# Template table

<!-- example template -->
 A Template Table is a special type of table in Manticore that doesn't store any data and doesn't create any files on your disk. Despite this, it can have the same [NLP settings](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Natural-language-processing-specific-settings) as a plain or real-time table. Template tables can be used for the following purposes:

* As a template to inherit settings in the  [Plain mode](../../Creating_a_table/Local_tables.md#Defining-table-schema-in-config-%28Plain mode%29), simplifying your Manticore configuration file.
* Keyword generation with the help of the [CALL KEYWORDS](../../Searching/Autocomplete.md#CALL-KEYWORDS) command.
* Highlighting an arbitrary string using the [CALL SNIPPETS](../../Searching/Highlighting.md#CALL-SNIPPETS) command.


<!-- intro -->
##### Creating a template table via a configuration file:

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