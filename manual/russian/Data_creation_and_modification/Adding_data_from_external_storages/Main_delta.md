# Main+delta схема

<!-- example maindelta -->
Во многих ситуациях общий набор данных слишком велик, чтобы его часто перестраивать с нуля, в то время как количество новых записей остается относительно небольшим. Например, на форуме может быть 1 000 000 архивных сообщений, но при этом ежедневно поступает всего 1 000 новых сообщений.

В таких случаях реализация «живых» (почти в реальном времени) обновлений таблиц может быть достигнута с помощью схемы «main+delta».

Концепция заключается в настройке двух источников и двух таблиц: одной «главной» таблицы для данных, которые редко меняются (если вообще меняются), и одной «дельта» таблицы для новых документов. В примере 1 000 000 архивных сообщений хранятся в главной таблице, а 1 000 новых ежедневных сообщений помещаются в дельта-таблицу. Дельта-таблица может затем часто перестраиваться, обеспечивая доступность документов для поиска в течение нескольких секунд или минут. Определение, какие документы относятся к какой таблице, и перестройка главной таблицы могут быть полностью автоматизированы. Один из подходов — создать вспомогательную таблицу-счетчик, которая отслеживает ID, используемый для разделения документов, и обновляет его всякий раз при перестройке главной таблицы.

Использование столбца с временной меткой в качестве переменной разделения более эффективно, чем использование ID, поскольку временные метки позволяют отслеживать не только новые документы, но и изменённые.

Для наборов данных, которые могут содержать изменённые или удалённые документы, дельта-таблица должна предоставлять список затронутых документов, гарантируя их подавление и исключение из поисковых запросов. Это достигается с помощью функции, называемой Kill Lists. ID документов, которые необходимо «убить», могут быть указаны во вспомогательном запросе, определенном директивой [sql_query_killlist](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md#Table-kill-list). Дельта таблица должна указывать целевые таблицы, для которых будут применяться Kill Lists, с помощью директивы [killlist_target](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md#killlist_target). Влияние Kill Lists является постоянным для целевой таблицы, что означает, что даже если поиск выполняется без дельта таблицы, подавленные документы не появятся в результатах поиска.

Обратите внимание, как мы переопределяем `sql_query_pre` в источнике дельта. Мы должны явно включить это переопределение. Если не сделать этого, запрос `REPLACE` будет выполнен и во время построения источника дельта, что фактически сделает его бесполезным.

<!-- request Example -->
```ini
# in MySQL
CREATE TABLE deltabreaker (
  index_name VARCHAR(50) NOT NULL,
  created_at TIMESTAMP NOT NULL  DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (index_name)
);

# in manticore.conf
source main {
  ...
  sql_query_pre = REPLACE INTO deltabreaker SET index_name = 'main', created_at = NOW()
  sql_query =  SELECT id, title, UNIX_TIMESTAMP(updated_at) AS updated FROM documents WHERE deleted=0 AND  updated_at  >=FROM_UNIXTIME($start) AND updated_at  <=FROM_UNIXTIME($end)
  sql_query_range  = SELECT ( SELECT UNIX_TIMESTAMP(MIN(updated_at)) FROM documents) min, ( SELECT UNIX_TIMESTAMP(created_at)-1 FROM deltabreaker WHERE index_name='main') max
  sql_query_post_index = REPLACE INTO deltabreaker set index_name = 'delta', created_at = (SELECT created_at FROM deltabreaker t WHERE index_name='main')
  ...
  sql_attr_timestamp = updated
}

source delta : main {
  sql_query_pre =
  sql_query_range = SELECT ( SELECT UNIX_TIMESTAMP(created_at) FROM deltabreaker WHERE index_name='delta') min, UNIX_TIMESTAMP() max
  sql_query_killlist = SELECT id FROM documents WHERE updated_at >=  (SELECT created_at FROM deltabreaker WHERE index_name='delta')
}

table main {
  path = /var/lib/manticore/main
  source = main
}

table delta {
  path = /var/lib/manticore/delta
  source = delta
  killlist_target = main:kl
}
```

<!-- end -->


<!-- proofread -->

