# SHOW QUERIES

<!-- example SHOW QUERIES -->
```sql
SHOW QUERIES
```

> ПРИМЕЧАНИЕ: Для работы `SHOW QUERIES` требуется [Manticore Buddy](../Installation/Manticore_Buddy.md). Если команда не работает, убедитесь, что Buddy установлен.

`SHOW QUERIES` возвращает информацию обо всех выполняющихся в данный момент запросах. Результат представляет собой таблицу со следующей структурой:

- `id`: ID запроса, который можно использовать в команде [KILL](../Node_info_and_management/KILL.md) для его завершения
- `query`: Текст запроса или его часть
- `time`: Время, затраченное на выполнение команды, или время, прошедшее с момента выполнения запроса (в этом случае значение будет содержать `ago`)
- `protocol`: [Протокол соединения](../Server_settings/Searchd.md#listen), возможные значения: `sphinx`, `mysql`, `http`, `ssl`, `compressed`, `replication` или их комбинация (например, `http,ssl` или `compressed,mysql`)
- `host`: `ip:port` клиента


<!-- request SQL -->
```sql
mysql> SHOW QUERIES;
```

<!-- response SQL -->
```
+------+--------------+---------+----------+-----------------+
| id   | query        | time    | protocol | host            |
+------+--------------+---------+----------+-----------------+
|  111 | select       | 5ms ago | http     | 127.0.0.1:58986 |
|   96 | SHOW QUERIES | 255us   | mysql    | 127.0.0.1:33616 |
+------+--------------+---------+----------+-----------------+
2 rows in set (0.61 sec)
```

<!-- end -->

Обратитесь к [SHOW THREADS](../Node_info_and_management/SHOW_THREADS.md), если вы хотите получить информацию с точки зрения самих потоков.

<!-- proofread -->

