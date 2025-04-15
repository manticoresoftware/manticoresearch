# Section "Common" in configuration

lemmatizer_base
----------------

The `lemmatizer_base` is an optional configuration directive that specifies the base path for lemmatizer dictionaries. The default path is `/usr/share/manticore`

The lemmatizer implementation in Manticore Search (see [Morphology](../Creating_a_table/NLP_and_tokenization/Morphology.md) to learn what lemmatizers are) is dictionary-driven and requires specific dictionary files for different languages. These files can be downloaded from the Manticore website (<https://manticoresearch.com/install/#other-downloads>).

Example:

```ini
lemmatizer_base = /usr/share/manticore/
```

progressive_merge
------------------

The progressive_merge is a configuration directive that, when enabled, merges real-time table disk chunks from smaller to larger ones. This approach speeds up the merging process and reduces read/write amplification. By default, this setting is enabled. If disabled, the chunks are merged in the order they were created.

json_autoconv_keynames
------------------------

The json_autoconv_keynames is an optional configuration directive that determines if and how to auto-convert key names within JSON attributes. The known value is 'lowercase'. By default, this setting is unspecified (meaning no conversion occurs).

When set to lowercase, key names within JSON attributes will be automatically converted to lowercase during indexing. This conversion applies to JSON attributes from all data sources, including SQL and XMLpipe2.

Example:

```ini
json_autoconv_keynames = lowercase
```

json_autoconv_numbers
-----------------------

The json_autoconv_numbers is an optional configuration directive that determines whether to automatically detect and convert JSON strings that represent numbers into numeric attributes. The default value is 0 (do not convert strings into numbers).

When this option is set to 1, values such as "1234" will be indexed as numbers instead of strings. If the option is set to 0, such values will be indexed as strings. This conversion applies to JSON attributes from all data sources, including SQL and XMLpipe2.

Example:

```ini
json_autoconv_numbers = 1
```

on_json_attr_error
---------------------

on_json_attr_error is an optional configuration directive that specifies the action to take if JSON format errors are found. The default value is `ignore_attr`(ignore errors). This setting applies only to `sql_attr_json` attributes.

By default, JSON format errors are ignored (`ignore_attr`), and the indexer tool will show a warning. Setting this option to `fail_index` will cause indexing to fail at the first JSON format error.

Example:

```ini
on_json_attr_error = ignore_attr
```

plugin_dir
-----------

The plugin_dir is an optional configuration directive that specifies the trusted location for dynamic libraries (UDFs). The default path is  `/usr/local/lib/manticore/`.

This directive sets the trusted directory from which the [UDF libraries](../Extensions/UDFs_and_Plugins/UDF.md) can be loaded.

Example:

```ini
plugin_dir = /usr/local/lib/manticore/
```
<!-- proofread -->