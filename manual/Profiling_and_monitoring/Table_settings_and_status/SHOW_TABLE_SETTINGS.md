# SHOW TABLE SETTINGS

<!-- example SHOW TABLE SETTINGS -->

`SHOW TABLE SETTINGS` is an SQL statement that displays per-table settings in a format that is compatible with the config file.

The syntax is:

```sql
SHOW TABLE index_name[.N | CHUNK N] SETTINGS
```

Output is similar to the [--dumpconfig](../../Miscellaneous_tools.md#indextool) option of the [indextool](../../Miscellaneous_tools.md#indextool) utility. The report provides a breakdown of all the table settings, including tokenizer and dictionary options.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
SHOW TABLE forum SETTINGS;
```

<!-- response SQL -->
```sql
+---------------+-----------------------------------------------------------------------------------------------------------+
| Variable_name | Value                                                                                                     |
+---------------+-----------------------------------------------------------------------------------------------------------+
| settings      | min_prefix_len = 3
charset_table = 0..9, A..Z->a..z, _, -, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F |
+---------------+-----------------------------------------------------------------------------------------------------------+
1 row in set (0.00 sec)
```

<!-- end -->

<!-- example SHOW TABLE SETTINGS N -->

You can also specify a particular chunk number to view the settings of a particular chunk of an RT table. The number is 0-based.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
SHOW TABLE forum CHUNK 0 SETTINGS;
```

<!-- response SQL -->
```sql
+---------------+-----------------------------------------------------------------------------------------------------------+
| Variable_name | Value                                                                                                     |
+---------------+-----------------------------------------------------------------------------------------------------------+
| settings      | min_prefix_len = 3
charset_table = 0..9, A..Z->a..z, _, -, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F |
+---------------+-----------------------------------------------------------------------------------------------------------+
1 row in set (0.00 sec)
```

<!-- end -->
