# Section "Common" in configuration

lemmatizer_base
----------------

Lemmatizer dictionaries base path. Optional, default is /usr/share/manticore.

Our lemmatizer implementation (see [Morphology](../Creating_a_table/NLP_and_tokenization/Morphology.md) for a discussion of what lemmatizers are) is dictionary driven. lemmatizer\_base directive configures the base dictionary path. File names are hardcoded and specific to a given lemmatizer; the Russian lemmatizer uses ru.pak dictionary file. The dictionaries can be obtained from the Manticore website (<https://manticoresearch.com/install/#other-downloads>).

Example:

```ini
lemmatizer_base = /usr/share/manticore/
```

progressive_merge
------------------

When merging real-time table disk chunks, do it from smaller to bigger ones. It makes merging faster with lower read/write amplification. Enabled by default. If disabled, chunks are merged from first to last created.

json_autoconv_keynames
------------------------

Whether and how to auto-convert key names within JSON attributes. Known value is 'lowercase'. Optional, default value is unspecified (do not convert anything).

When this directive is set to 'lowercase', key names within JSON attributes will be automatically brought to lower case when indexing. This conversion applies to any data source, that is, JSON attributes originating from either SQL or XMLpipe2 sources will all be affected.

Example:

```ini
json_autoconv_keynames = lowercase
```

json_autoconv_numbers
-----------------------

Automatically detect and convert possible JSON strings that represent numbers, into numeric attributes. Optional, default value is 0 (do not convert strings into numbers).

When this option is 1, values such as "1234" will be indexed as numbers instead of strings; if the option is 0, such values will be indexed as strings. This conversion applies to any data source, that is, JSON attributes originating from either SQL or XMLpipe2 sources will all be affected.

Example:

```ini
json_autoconv_numbers = 1
```

on_json_attr_error
---------------------

What to do if JSON format errors are found. Optional, default value is `ignore_attr` (ignore errors). Applies only to `sql_attr_json` attributes.

By default, JSON format errors are ignored (`ignore_attr`) and the indexer tool will just show a warning. Setting this option to `fail_index` will rather make indexing fail at the first JSON format error.

Example:

```ini
on_json_attr_error = ignore_attr
```

plugin_dir
-----------

Trusted location for the dynamic libraries (UDFs). Optional, default is `/usr/local/lib/manticore/`.

Specifies the trusted directory from which the [UDF libraries](../Extensions/UDFs_and_Plugins/UDF.md) can be loaded.

Example:

```ini
plugin_dir = /usr/local/lib/manticore/
```
