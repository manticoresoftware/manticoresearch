# FLUSH ATTRIBUTES

```sql
FLUSH ATTRIBUTES
```

Команда FLUSH ATTRIBUTES сбрасывает все обновления атрибутов, находящиеся в памяти, для всех активных дисковых таблиц на диск. Она возвращает тег, который идентифицирует итоговое состояние на диске (фактически это количество выполненных сохранений атрибутов на диск с момента запуска сервера).

```sql
mysql> UPDATE testindex SET channel_id=1107025 WHERE id=1;
Query OK, 1 row affected (0.04 sec)

mysql> FLUSH ATTRIBUTES;
+------+
| tag  |
+------+
|    1 |
+------+
1 row in set (0.19 sec)
```

<!-- proofread -->

