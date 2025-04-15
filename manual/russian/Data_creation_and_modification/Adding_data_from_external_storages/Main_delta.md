# Основная+дельта схема

<!-- пример main+delta -->
Во многих случаях общий объем данных слишком велик, чтобы его часто пересоздавать с нуля, тогда как количество новых записей остается относительно небольшим. Например, форум может иметь 1 000 000 архивных сообщений, но получать только 1 000 новых сообщений в день.

В таких случаях осуществить "живые" (почти в реальном времени) обновления таблиц можно с помощью схемы "основная+дельта".

Концепция заключается в настройке двух источников и двух таблиц: одна "основная" таблица для данных, которые редко меняются (если когда-либо), и одна "дельта" таблица для новых документов. В приведенном примере 1 000 000 архивных сообщений будет храниться в основной таблице, в то время как 1 000 новых дневных сообщений будут помещаться в дельта-таблицу. Дельта-таблицу можно будет перестраивать часто, что позволит сделать документы доступными для поиска в течение секунд или минут. Определение того, какие документы принадлежат какой таблице, и перестройка основной таблицы может быть полностью автоматизирована. Один из подходов заключается в создании таблицы счетчиков, которая отслеживает ID, используемые для разделения документов, и обновляется каждый раз, когда основная таблица перестраивается.

Использование столбца с меткой времени в качестве переменной разделения более эффективно, чем использование ID, поскольку метки времени могут отслеживать не только новые документы, но и измененные.

Для наборов данных, которые могут содержать измененные или удаленные документы, дельта-таблица должна предоставлять список затронутых документов, обеспечивая их подавление и исключение из поисковых запросов. Это достигается с помощью функции, называемой Kill Lists. ID документов, которые нужно "убить", можно указать в вспомогательном запросе, определенном в [sql_query_killlist](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md#Table-kill-list). Дельта-таблица должна указывать целевые таблицы, для которых будут применены списки уничтожения, с помощью директивы [killlist_target](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md#killlist_target). Влияние списков уничтожения на целевую таблицу является постоянным, что означает, что даже если поиск выполняется без дельта-таблицы, подавленные документы не появятся в результатах поиска.

Обратите внимание, как мы переопределяем `sql_query_pre` в источнике дельта. Мы должны явно включить это переопределение. Если мы этого не сделаем, запрос `REPLACE` будет выполнен во время построения источника дельта, что фактически сделает его бесполезным.

<!-- запрос Пример -->
```ini
# в MySQL
CREATE TABLE deltabreaker (
  index_name VARCHAR(50) NOT NULL,
  created_at TIMESTAMP NOT NULL  DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (index_name)
);

# в manticore.conf
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

<!-- конец -->


<!-- корректура -->
