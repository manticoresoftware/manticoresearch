# Template index

<!-- example template -->
 Template index is a pseudo-index since it does not store any data and does not create any files on your disk. At the same time it can have the same [NLP settings](../../Creating_an_index/Local_indexes/Plain_and_real-time_index_settings.md#Natural-language-processing-specific-settings) as a plain or a real-time index. Template indexes can be used for few purposes:
 
* as templates to inherit plain/real-time indexes in [Plain mode](../../Creating_an_index/Local_indexes.md#Defining-index-schema-in-config-%28Plain mode%29) just to minify Manticore configuration file
* keywords generation with help of [CALL KEYWORDS](../../Searching/Autocomplete.md#CALL-KEYWORDS)
* highlighting of an arbitrary string using [CALL SNIPPETS](../../Searching/Highlighting.md#CALL-SNIPPETS)


<!-- intro -->
##### CONFIG:

<!-- request CONFIG -->
##### Creating a template index via a configuration file:

```ini
index template {
  type = template
  morphology = stem_en
  wordforms = wordforms.txt
  exceptions = exceptions.txt
  stopwords = stopwords.txt
}
```
<!-- end -->
