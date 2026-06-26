# Зеркалирование

[Агенты](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) реплики могут использоваться взаимозаменяемо при обработке поискового запроса. Экземпляр Manticore, на котором размещена распределённая таблица с определёнными реплицированными агентами, отслеживает статус реплик (работают или нет) и время отклика, а также выполняет автоматическое переключение при сбое и балансировку нагрузки на основе этой информации.

## Зеркала агента

```ini
agent = node1|node2|node3:9312:shard2
```

В приведённом выше примере объявляется, что `node1:9312`, `node2:9312` и `node3:9312` все имеют таблицу с именем `shard2` и могут использоваться как взаимозаменяемые реплики. Если любой из этих серверов выйдет из строя, запросы будут распределены между оставшимися. Когда сервер вернётся в онлайн, мастер обнаружит это и снова начнёт направлять запросы ко всем репликам.

Зеркало также может включать индивидуальный список таблиц, как показано ниже:

```ini
agent = node1:9312:node1shard2|node2:9312:node2shard2
```

Это работает аналогично предыдущему примеру, но при запросе к разным серверам будут использоваться разные имена таблиц. Например, `node1shard2` будет использоваться при запросе к `node1:9312`, а `node2shard2` — при запросе к `node2:9312`.

По умолчанию выбор реплики использует глобальную или табличную стратегию [ha_strategy](../../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_strategy). Если вы не зададите её явно, стратегия по умолчанию — `random`. Мастер хранит метрики, такие как общее количество запросов, количество ошибок и время отклика для каждого агента, и группирует их во временные интервалы, контролируемые параметром [ha_period_karma](../../Server_settings/Searchd.md#ha_period_karma). Затем эти статистические данные используются стратегиями балансировки, описанными в разделе [Балансировка нагрузки](../../Creating_a_cluster/Remote_nodes/Load_balancing.md).

[Период кармы](../../Server_settings/Searchd.md#ha_period_karma) указывается в секундах и по умолчанию равен 60 секундам. Мастер хранит до 15 интервалов кармы со статистикой по агентам для целей инструментирования (см. [SHOW AGENT STATUS](../../Node_info_and_management/Node_status.md#SHOW-AGENT-STATUS)). Однако для логики HA/LB используются только последние два интервала.

Когда запросов нет, мастер отправляет регулярную команду ping каждые [ha_ping_interval](../../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_ping_interval) для сбора статистики и проверки, жив ли ещё удалённый хост. [ha_ping_interval](../../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_ping_interval) по умолчанию равен 1000 мс. Установка значения 0 отключает пинги, и статистика будет накапливаться только на основе фактических запросов. Вместе с [ha_period_karma](../../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_period_karma) это управляет тем, насколько быстро изменения в состоянии и задержке реплик влияют на балансировку нагрузки.

## Реплики против шардов

Это различие важно:

- Одна запись `agent='host1|host2:table'` означает один удалённый шард с реплицированными бэкендами.
- Несколько записей `agent='...'` означают несколько удалённых шардов.

Например:

```ini
# one shard, two mirrors
agent = node1|node2:9312:products

# two shards
agent = node1:9312:products_a
agent = node2:9312:products_b
```

## Определение реплик

<!-- example mirroring-definition -->
Та же концепция репликации может быть настроена либо в конфигурационном файле, либо с помощью `CREATE TABLE`. Для TCP-соединений `agent=` должен использовать удалённый порт агента/API (обычно `9312`), а не порт MySQL (`9306`). Если вы хотите конкретную политику балансировки только для одной распределённой таблицы, установите `ha_strategy` для этой таблицы, а не полагайтесь только на глобальные настройки по умолчанию.

<!-- intro -->
##### Конфигурация:

<!-- request Config -->
```ini
table products_dist {
  type = distributed
  agent = 127.0.0.1:9312|127.0.0.1:9313:products
  ha_strategy = roundrobin
}
```

<!-- intro -->
##### SQL:

<!-- request SQL -->
```sql
CREATE TABLE products_dist type='distributed'
  agent='127.0.0.1:9312:products|127.0.0.1:9313:products'
  ha_strategy='roundrobin';
```
<!-- end -->

<!-- example mirroring-setup -->
Предположим, что реплицированные таблицы уже существуют, например:

<!-- intro -->
##### SQL на удалённом узле 1:

<!-- request SQL -->
```sql
CREATE TABLE products(id bigint, title text, node string);
INSERT INTO products(id,title,node) VALUES(1,'same title','node1');
```

<!-- intro -->
##### SQL на удалённом узле 2:

<!-- request SQL -->
```sql
CREATE TABLE products(id bigint, title text, node string);
INSERT INTO products(id,title,node) VALUES(1,'same title','node2');
```

<!-- intro -->
##### SQL на мастере:

<!-- request SQL -->
```sql
CREATE TABLE products_dist type='distributed'
  agent='127.0.0.1:9312:products|127.0.0.1:9313:products'
  ha_strategy='roundrobin';
```
<!-- end -->

<!-- example mirroring-sql-usage-and-verification -->
Используйте распределённую таблицу как обычно с мастера, затем проверьте, как она была сохранена и как ведут себя реплики.

<!-- intro -->
##### SQL запрос:

<!-- request SQL -->
```sql
SELECT id, title, node FROM products_dist;
SHOW CREATE TABLE products_dist;
SHOW AGENT STATUS;
```

<!-- intro -->
##### SQL ответ:

<!-- response SQL -->
```sql
+------+------------+-------+
| id   | title      | node  |
+------+------------+-------+
|    1 | same title | node1 |
+------+------------+-------+

+---------------+----------------------------------------------------------------------------------+
| Table         | Create Table                                                                     |
+---------------+----------------------------------------------------------------------------------+
| products_dist | CREATE TABLE products_dist type='distributed' agent='127.0.0.1:9312:products|... |
+---------------+----------------------------------------------------------------------------------+
```
<!-- end -->

<!-- example mirroring-sharded-cluster -->
Пример шардирования таблицы на 4 сервера в общей сложности, в 2 шарда с 2 репликами для каждого шарда.

<!-- intro -->
##### Конфигурация:

<!-- request Config -->
```ini
# node1, node2 carry shard1 as local
# node3, node4 carry shard2 as local

# config on node1, node2
agent = node3:9312|node4:9312:shard2

# config on node3, node4
agent = node1:9312|node2:9312:shard1
```
<!-- end -->
<!-- proofread -->
