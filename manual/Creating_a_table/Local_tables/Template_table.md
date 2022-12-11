# Template table

<!-- example template -->
 Template table is a pseudo-table since it does not store any data and does not create any files on your disk. At the same time it can have the same [NLP settings](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Natural-language-processing-specific-settings) as a plain or a real-time table. Template tables can be used for a few purposes:

* as templates to inherit plain/real-time tables in the [Plain mode](../../Creating_a_table/Local_tables.md#Defining-table-schema-in-config-%28Plain mode%29) just to minify Manticore configuration file
* keywords generation with help of [CALL KEYWORDS](../../Searching/Autocomplete.md#CALL-KEYWORDS)
* highlighting of an arbitrary string using [CALL SNIPPETS](../../Searching/Highlighting.md#CALL-SNIPPETS)


<!-- intro -->
##### CONFIG:

<!-- request CONFIG -->
##### Creating a template table via a configuration file:

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
