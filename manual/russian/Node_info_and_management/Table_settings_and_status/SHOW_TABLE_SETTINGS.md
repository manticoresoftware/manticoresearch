# SHOW TABLE SETTINGS

<!-- example SHOW TABLE SETTINGS -->

`SHOW TABLE SETTINGS` — это SQL-запрос, который отображает настройки по каждой таблице в формате, совместимом с конфигурационным файлом.

Синтаксис:

```sql
SHOW TABLE table_name[.N | CHUNK N] SETTINGS
```

Вывод напоминает опцию [--dumpconfig](../../Miscellaneous_tools.md#indextool) утилиты [indextool](../../Miscellaneous_tools.md#indextool). Отчет предоставляет разбивку всех настроек таблицы, включая параметры токенизатора и словаря.

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
POST /sql?mode=raw -d "SHOW TABLE forum SETTINGS;"
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

Вы также можете указать конкретный номер чанка, чтобы просмотреть настройки определенного чанка в RT-таблице. Нумерация начинается с 0.

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

```sql
POST /sql?mode=raw -d "SHOW TABLE forum CHUNK 0 SETTINGS;"
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

