# FLUSH ATTRIBUTES

```sql
FLUSH ATTRIBUTES
```

Команда FLUSH ATTRIBUTES сбрасывает все обновления атрибутов, находящиеся в памяти, во всех активных дисковых таблицах на диск. Она возвращает тег, который идентифицирует результат состояния на диске (что по сути является числом фактически выполненных сохранений атрибутов на диск с момента запуска сервера).

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

