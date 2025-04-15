# Перечень плагинов

## ПОКАЗАТЬ ПЛАГИНЫ
<!-- example Example -->

```sql
SHOW PLUGINS
```

Отображает все загруженные плагины (за исключением плагинов Buddy, см. ниже) и UDF. Столбец "Type" должен быть одним из `udf`, `ranker`, `index_token_filter` или `query_token_filter`. Столбец "Users" является числом потоков, которые в данный момент используют этот плагин в запросе. Столбец "Extra" предназначен для различной дополнительной информации, специфичной для типа плагина; в настоящее время он показывает тип возвращаемого значения для UDF и пуст для всех других типов плагинов.


<!-- intro -->
##### Пример:

<!-- request Example -->

```sql
SHOW PLUGINS;
```

<!-- response -->

```sql
+------+----------+----------------+-------+-------+
| Type | Name     | Library        | Users | Extra |
+------+----------+----------------+-------+-------+
| udf  | sequence | udfexample.dll | 0     | INT   |
+------+----------+----------------+-------+-------+
1 row in set (0.00 sec)
```

<!-- end -->

## ПОКАЗАТЬ ПЛАГИНЫ BUDDY

<!-- example Example_buddy -->

```sql
SHOW BUDDY PLUGINS
```

> ЗАМЕТКА: `SHOW BUDDY PLUGINS` требует [Manticore Buddy](../Installation/Manticore_Buddy.md). Если это не работает, убедитесь, что Buddy установлен.

Это отобразит все доступные плагины, включая основные и локальные.
Чтобы удалить плагин, убедитесь, что используете имя, указанное в столбце Package.

<!-- request Example -->

```sql
SHOW BUDDY PLUGINS;
```

<!-- response -->

```
+------------------------------------------------+-----------------+---------+------+----------------------------------------------------------------------------------------------------------------------------------------------------------+
| Package                                        | Plugin          | Version | Type | Info                                                                                                                                                     |
+------------------------------------------------+-----------------+---------+------+----------------------------------------------------------------------------------------------------------------------------------------------------------+
| manticoresoftware/buddy-plugin-empty-string    | empty-string    | 2.1.5   | core | Обрабатывает пустые запросы, которые могут возникнуть при обрезке комментариев или работе с определенными инструкциями SQL-протокола в комментариях, которые не поддерживаются      |
| manticoresoftware/buddy-plugin-backup          | backup          | 2.1.5   | core | SQL оператор BACKUP                                                                                                                                     |
| manticoresoftware/buddy-plugin-emulate-elastic | emulate-elastic | 2.1.5   | core | Эмулирует некоторые запросы Elastic и генерирует ответы так, как если бы они были выполнены ES                                                                         |
| manticoresoftware/buddy-plugin-insert          | insert          | 2.1.5   | core | Поддержка авто-схемы. Когда выполняется операция вставки, и таблица не существует, она создаёт её с автоопределением типов данных                    |
| manticoresoftware/buddy-plugin-alias           | alias           | 2.1.5   | core |                                                                                                                                                          |
| manticoresoftware/buddy-plugin-select          | select          | 2.1.5   | core | Различные обработчики SELECT, необходимые для mysqldump и поддержки другого программного обеспечения, в основном нацелены на работу аналогично MySQL                                       |
| manticoresoftware/buddy-plugin-show            | show            | 2.1.5   | core | Различные обработчики запросов "show", например, `show queries`, `show fields`, `show full tables` и т.д.                                                     |
| manticoresoftware/buddy-plugin-cli-table       | cli-table       | 2.1.5   | core | /cli конечная точка, основанная на /cli_json - выводит результат запроса в виде таблицы                                                                                       |
| manticoresoftware/buddy-plugin-plugin          | plugin          | 2.1.5   | core | Основная логика для поддержки плагинов и вспомогательных функций. Также обрабатывает `create buddy plugin`, `delete buddy plugin` и `show buddy plugins`                           |
| manticoresoftware/buddy-plugin-test            | test            | 2.1.5   | core | Тестовый плагин, используемый исключительно для тестов                                                                                                                  |
| manticoresoftware/buddy-plugin-insert-mva      | insert-mva      | 2.1.5   | core | Управляет восстановлением полей MVA с помощью mysqldump                                                                                                     |
| manticoresoftware/buddy-plugin-modify-table    | modify-table    | 2.1.5   | core | Помогает стандартизировать параметры в операторах создания и изменения таблиц, чтобы показывать option=1 для целых чисел. Также управляет логикой для создания шардинг-таблиц. |
| manticoresoftware/buddy-plugin-knn             | knn             | 2.1.5   | core | Включает KNN по идентификатору документа                                                                                                                               |
| manticoresoftware/buddy-plugin-replace         | replace         | 2.1.5   | core | Включает частичную замену                                                                                                                                 |
+------------------------------------------------+-----------------+---------+------+----------------------------------------------------------------------------------------------------------------------------------------------------------+-----+
```

<!-- end -->
<!-- proofread -->
