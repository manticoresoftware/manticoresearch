# SHOW VERSION

<!-- example SHOW VERSION -->
```sql
SHOW VERSION
```

> ПРИМЕЧАНИЕ: `SHOW VERSION` требует [Manticore Buddy](../Installation/Manticore_Buddy.md). Если команда не работает, убедитесь, что Buddy установлен.

`SHOW VERSION` предоставляет подробную информацию о версиях различных компонентов экземпляра Manticore Search. Эта команда особенно полезна для администраторов и разработчиков, которым необходимо проверить версию Manticore Search, которую они используют, а также версии связанных компонентов.

Таблица вывода содержит две колонки:
- `Component`: В этой колонке указано название конкретного компонента Manticore Search.
- `Version`: В этой колонке отображается информация о версии соответствующего компонента.

<!-- request SQL -->
```sql
mysql> SHOW VERSION;
```

<!-- response SQL -->
```
+------------+--------------------------------+
| Component  | Version                        |
+------------+--------------------------------+
| Daemon     | 6.2.13 61cfe38d2@24011520 dev  |
| Columnar   | columnar 2.2.5 214ce90@240115  |
| Secondary  | secondary 2.2.5 214ce90@240115 |
| Knn        | knn 2.2.5 214ce90@240115       |
| Embeddings | embeddings 1.0.0               |
| Buddy      | buddy v2.0.11                  |
+------------+--------------------------------+
```

<!-- end -->

<!-- proofread -->

