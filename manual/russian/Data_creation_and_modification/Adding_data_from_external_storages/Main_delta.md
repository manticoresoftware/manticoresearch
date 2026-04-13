# Схема main+delta

<!-- example maindelta -->
Во многих ситуациях полный набор данных слишком велик, чтобы его часто перестраивать с нуля, в то время как количество новых записей остается относительно небольшим. Например, на форуме может быть 1 000 000 архивных сообщений, но ежедневно добавляется только 1 000 новых сообщений.

В таких случаях реализация "живых" (почти в реальном времени) обновлений таблицы может быть достигнута с использованием схемы "main+delta".

Идея заключается в настройке двух источников и двух таблиц: одной "основной" таблицы для данных, которые редко меняются (если вообще меняются), и одной "дельта" таблицы для новых документов. В примере 1 000 000 архивных сообщений будут храниться в основной таблице, а 1 000 новых ежедневных сообщений будут помещены в дельта-таблицу. Затем дельта-таблицу можно часто перестраивать, делая документы доступными для поиска в течение секунд или минут. Определение того, какие документы принадлежат какой таблице, и перестроение основной таблицы могут быть полностью автоматизированы. Один из подходов — создать таблицу-счетчик, которая отслеживает ID, используемый для разделения документов, и обновлять ее при каждом перестроении основной таблицы.

Использование столбца с меткой времени в качестве переменной для разделения более эффективно, чем использование ID, поскольку метки времени могут отслеживать не только новые документы, но и измененные.

Для наборов данных, которые могут содержать измененные или удаленные документы, дельта-таблица должна предоставлять список затронутых документов, гарантируя их подавление и исключение из поисковых запросов. Это достигается с помощью функции, называемой Списки исключений (Kill Lists). ID документов, которые нужно исключить, могут быть указаны во вспомогательном запросе, определенном с помощью [sql_query_killlist](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md#Table-kill-list). Дельта-таблица должна указать целевые таблицы, для которых будут применяться списки исключений, с помощью директивы [killlist_target](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md#killlist_target). Влияние списков исключений на целевую таблицу является постоянным, что означает, что даже если поиск выполняется без дельта-таблицы, подавленные документы не появятся в результатах поиска.

Обратите внимание, как мы переопределяем `sql_query_pre` в источнике дельта. Мы должны явно включить это переопределение. Если мы этого не сделаем, запрос `REPLACE` будет выполнен также во время сборки источника дельта, что фактически сделает его бесполезным.

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

