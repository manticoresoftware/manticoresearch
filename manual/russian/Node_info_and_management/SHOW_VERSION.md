# SHOW VERSION

<!-- example SHOW VERSION -->
```sql
SHOW VERSION
```

> ПРИМЕЧАНИЕ: `SHOW VERSION` требует [Manticore Buddy](../Installation/Manticore_Buddy.md). Если команда не работает, убедитесь, что Buddy установлен.

`SHOW VERSION` предоставляет подробную информацию о версиях различных компонентов экземпляра Manticore Search. Эта команда особенно полезна для администраторов и разработчиков, которым необходимо проверить версию Manticore Search, а также версии связанных с ним компонентов.

Таблица вывода включает два столбца:
- `Component`: В этом столбце указано имя конкретного компонента Manticore Search.
- `Version`: В этом столбце отображается информация о версии соответствующего компонента.

<!-- request SQL -->
```sql
mysql> SHOW VERSION;
```

<!-- response SQL -->
```
+------------+-------------------------------------+
| Component  | Version                             |
+------------+-------------------------------------+
| Daemon     | 13.13.4 0bc5a9641@25101507 dev      |
| Columnar   | columnar 8.1.0 e1522a2@25100213     |
| Secondary  | secondary 8.1.0 e1522a2@25100213    |
| Knn        | knn 8.1.0 e1522a2@25100213          |
| Embeddings | embeddings 1.0.1                    |
| Buddy      | buddy v3.35.1+25090418-41d9811f-dev |
+------------+-------------------------------------+
```

<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "SHOW VERSION"
```

<!-- response JSON -->
```JSON
[
  {
    "total": 6,
    "error": "",
    "warning": "",
    "columns": [
      {
        "Component": {
          "type": "string"
        }
      },
      {
        "Version": {
          "type": "string"
        }
      }
    ],
    "data": [
      {
        "Component": "Daemon",
        "Version": "13.13.4 0bc5a9641@25101507 dev"
      },
      {
        "Component": "Columnar",
        "Version": "columnar 8.1.0 e1522a2@25100213"
      },
      {
        "Component": "Secondary",
        "Version": "secondary 8.1.0 e1522a2@25100213"
      },
      {
        "Component": "Knn",
        "Version": "knn 8.1.0 e1522a2@25100213"
      },
      {
        "Component": "Embeddings",
        "Version": "embeddings 1.0.1"
      },
      {
        "Component": "Buddy",
        "Version": "buddy v3.35.1+25090418-41d9811f-dev"
      }
    ]
  }
]
```

<!-- end -->

<!-- proofread -->

