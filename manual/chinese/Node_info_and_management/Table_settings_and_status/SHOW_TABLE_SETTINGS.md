# SHOW TABLE SETTINGS


<!--
data for the following examples:

DROP TABLE IF EXISTS forum;
CREATE TABLE forum(f text) min_prefix_len='3' charset_table='0..9, A..Z->a..z, _, -, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F';
-->

<!-- example SHOW TABLE SETTINGS -->

`SHOW TABLE SETTINGS` 是一个以与配置文件兼容的格式显示每个表设置的 SQL 语句。

语法是：

```sql
SHOW TABLE table_name[.N | CHUNK N] SETTINGS
```

输出类似于 [indextool](../../Miscellaneous_tools.md#indextool) 工具的 [--dumpconfig](../../Miscellaneous_tools.md#indextool) 选项。报告详细列出了所有表设置，包括分词器和字典选项。

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

<!-- intro -->
##### JSON:
<!-- request JSON -->

```JSON
POST /sql?mode=raw -d "SHOW TABLE forum SETTINGS"
```

<!-- response JSON -->
```JSON
[
  {
    "columns": [
      {
        "Variable_name": {
          "type": "string"
        }
      },
      {
        "Value": {
          "type": "string"
        }
      }
    ],
    "data": [
      {
        "Variable_name": "settings",
        "Value": "min_prefix_len = 3\ncharset_table = 0..9, A..Z->a..z, _, -, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F"
      }
    ],
    "total": 1,
    "error": "",
    "warning": ""
  }
]
```

<!-- end -->

<!-- example SHOW TABLE SETTINGS N -->

你也可以指定特定的块编号来查看 RT 表中某个特定块的设置。编号从 0 开始。

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


<!-- intro -->
##### JSON:
<!-- request JSON -->

```JSON
POST /sql?mode=raw -d "SHOW TABLE forum CHUNK 0 SETTINGS"
```

<!-- response JSON -->
```JSON
[
  {
    "columns": [
      {
        "Variable_name": {
          "type": "string"
        }
      },
      {
        "Value": {
          "type": "string"
        }
      }
    ],
    "data": [
      {
        "Variable_name": "settings",
        "Value": "min_prefix_len = 3\ncharset_table = 0..9, A..Z->a..z, _, -, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F"
      }
    ],
    "total": 1,
    "error": "",
    "warning": ""
  }
]
```

<!-- end -->
<!-- proofread -->

