# SHOW VERSION

<!-- example SHOW VERSION -->
```sql
SHOW VERSION
```

> NOTE: `SHOW VERSION` requires [Manticore Buddy](../Installation/Manticore_Buddy.md). If it doesn't work, make sure Buddy is installed.

`SHOW VERSION` provides detailed version information of various components of the Manticore Search instance. This command is particularly useful for administrators and developers who need to verify the version of Manticore Search they are running, along with the versions of its associated components.

The output table includes two columns:
- `Component`: This column names the specific component of Manticore Search.
- `Version`: This column displays the version information for the respective component.

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

