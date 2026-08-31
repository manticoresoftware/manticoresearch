# Удалённые таблицы

Удалённая таблица в Manticore Search представлена префиксом [agent](../../Creating_a_table/Creating_a_distributed_table/Creating_a_distributed_table.md) в определении распределённой таблицы. Распределённая таблица может включать сочетание локальных и удалённых таблиц. Если локальные таблицы не указаны, распределённая таблица будет полностью удалённой и будет выступать только как прокси. Например, у вас может быть экземпляр Manticore, который слушает несколько портов и обслуживает разные протоколы, а затем перенаправляет запросы на серверы backend, которые принимают соединения только через внутренний бинарный протокол Manticore, используя постоянные соединения для уменьшения накладных расходов при установлении соединений.  
Хотя распределённая таблица, состоящая только из удалённых, сама не содержит локальных таблиц, она всё равно потребляет ресурсы машины, поскольку должна выполнять окончательные вычисления, такие как слияние результатов и вычисление итоговых агрегированных значений.

## agent

```ini
agent = address1 [ | address2 [...] ][:table-list]
agent = address1[:table-list [ | address2[:table-list [...] ] ] ]
```

Директива `agent` объявляет удаленные агенты, которые опрашиваются каждый раз при поиске по охватывающей распределенной таблице. Эти агенты, по сути, являются указателями на сетевые таблицы. Указанное значение включает адрес и также может включать несколько альтернатив (зеркала агента) либо только для адреса, либо для адреса и списка таблиц. См. [Зеркалирование](../../Creating_a_cluster/Remote_nodes/Mirroring.md) для синтаксиса зеркал и [Балансировка нагрузки](../../Creating_a_cluster/Remote_nodes/Load_balancing.md) для информации о том, как выбираются зеркальные агенты.

Спецификация адреса должна быть одной из следующих:

```ini
address = hostname[:port] # eg. server2:9312
address = /absolute/unix/socket/path # eg. /var/run/manticore2.sock
```

`hostname` — это имя удаленного хоста, `port` — номер удаленного TCP-порта, `table-list` — это список имен таблиц, разделенных запятыми, а квадратные скобки [] обозначают необязательную часть. Для TCP-соединений этот порт является портом удаленного агента/API (обычно `9312`), а не портом MySQL (`9306`).

Если имя таблицы опущено, предполагается, что это та же таблица, где определена эта строка. Другими словами, при определении агентов для распределённой таблицы 'mycoolindex' можно просто указать адрес, и будет предполагаться, что запрос идёт к таблице mycoolindex на конечных точках агента.

Если номер порта опущен, считается, что он равен **9312**. Если он задан, но недопустим (например, 70000), агент будет пропущен.

Вы можете направлять каждого агента к одной или нескольким удалённым таблицам, расположенным на одном или нескольких сетевых серверах без ограничений. Это позволяет несколько различных режимов использования:
* Шардинг по нескольким серверам агентов и создание произвольной топологии кластера
* Шардирование по нескольким серверам-агентам с зеркалированием для обеспечения высокой доступности и балансировки нагрузки (см. [Зеркалирование](../../Creating_a_cluster/Remote_nodes/Mirroring.md) и [Балансировка нагрузки](../../Creating_a_cluster/Remote_nodes/Load_balancing.md))
* Шардинг внутри localhost для использования нескольких ядер (хотя проще использовать несколько локальных таблиц)

Чтобы избежать путаницы:
* Одна запись `agent='host1|host2:table'` означает один удаленный шард с зеркальными бэкендами.
* Несколько записей `agent='...'` означают несколько удаленных шардов.

Все агенты опрашиваются параллельно. Список индексов передаётся удалённому агенту без изменений. Точный способ поиска по этому списку в агенте (последовательно или параллельно) зависит только от конфигурации агента (см. настройку [threads](../../Server_settings/Searchd.md#threads)). Мастер не управляет этим удалённо.

Важно отметить, что опция `LIMIT` игнорируется в запросах к агентам. Это связано с тем, что каждый агент может содержать разные таблицы, и ответственность за применение ограничения к итоговому набору результатов лежит на клиенте. Поэтому запрос к физической таблице отличается от запроса к распределённой таблице, если смотреть в логах запросов. Запрос не может быть простой копией исходного запроса, так как это приведёт к некорректным результатам.

Например, если клиент делает запрос SELECT ... LIMIT 10, 10, а существует два агента, при этом второй агент содержит только 10 документов, трансляция исходного запроса `LIMIT 10, 10` приведёт к тому, что от второго агента будут получены 0 документов. Однако запрос `LIMIT 10,10` должен вернуть документы с 10 по 20 из итогового набора. Чтобы решить эту проблему, запрос отправляют агентам с более широким ограничением, например, используя значение max_matches по умолчанию 1000.

Например, если есть распределённая таблица dist, которая ссылается на удалённую таблицу user, запрос клиента `SELECT * FROM dist LIMIT 10,10` будет преобразован в запрос `SELECT * FROM user LIMIT 0,1000` и отправлен удалённой таблице user. После получения результата распределённая таблица применит LIMIT 10,10 и вернёт запрашиваемые 10 документов.

```sql
SELECT * FROM dist LIMIT 10,10;
```

запрос будет преобразован в:

```sql
SELECT * FROM user LIMIT 0,1000
```

Кроме того, значение может содержать опции для каждого отдельного агента, такие как:
* [ha_strategy](../../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_strategy) - `random`, `roundrobin`, `nodeads`, `noerrors` (переопределяет глобальную настройку `ha_strategy` для конкретного агента; см. также [Зеркалирование](../../Creating_a_cluster/Remote_nodes/Mirroring.md))
* `conn` - `pconn`, persistent (эквивалентно установке `agent_persistent` на уровне таблицы)
* `blackhole` `0`,`1` (идентично настройке [agent_blackhole](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_blackhole) для агента)
* `retry_count` целочисленное значение (соответствует [agent_retry_count](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_retry_count), но заданное значение не умножается на число зеркал)

```ini
agent = address1:table-list[[ha_strategy=value, conn=value, blackhole=value]]
```

Пример:

```ini
# config on box1
# sharding a table over 3 servers
agent = box2:9312:shard1
agent = box3:9312:shard2

# config on box2
# sharding a table over 3 servers
agent = box1:9312:shard2
agent = box3:9312:shard3

# config on box3
# sharding a table over 3 servers
agent = box1:9312:shard1
agent = box2:9312:shard3

# per agent options
agent = box1:9312:shard1[ha_strategy=nodeads]
agent = box2:9312:shard2[conn=pconn]
agent = box2:9312:shard2[conn=pconn,ha_strategy=nodeads]
agent = test:9312:any[blackhole=1]
agent = test:9312|box2:9312|box3:9312:any2[retry_count=2]
agent = test:9312|box2:9312:any2[retry_count=2,conn=pconn,ha_strategy=noerrors]
```

## Определение удаленных таблиц

<!-- example remote-table-definition -->
Удаленные таблицы могут быть определены либо в конфигурационном файле, либо с помощью SQL на распределенной таблице.

<!-- intro -->
##### Конфигурационный файл:

<!-- request Config -->
```ini
table products_dist {
  type  = distributed
  agent = 127.0.0.1:9312|127.0.0.1:9313:products
}
```

<!-- intro -->
##### SQL:

<!-- request SQL -->
```sql
CREATE TABLE products_dist type='distributed'
  agent='127.0.0.1:9312:products|127.0.0.1:9313:products';
```
<!-- end -->

<!-- example remote-table-setup -->
Предположим, что удаленные таблицы уже существуют, например:

<!-- intro -->
##### SQL на удаленном узле 1:

<!-- request SQL -->
```sql
CREATE TABLE products(id bigint, title text, node string);
INSERT INTO products(id,title,node) VALUES(1,'same title','node1');
```

<!-- intro -->
##### SQL на удаленном узле 2:

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
  ha_strategy='roundrobin'
  agent_connect_timeout='200ms'
  agent_query_timeout='500ms'
  mirror_retry_count='2';
```
<!-- end -->

<!-- example remote-table-sql-usage-and-verification -->
Используйте распределенную таблицу на мастере точно так же, как любую другую таблицу, а затем проверьте, как она была сохранена.

<!-- intro -->
##### SQL запрос:

<!-- request SQL -->
```sql
SELECT id, title, node FROM products_dist;
SHOW CREATE TABLE products_dist;
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

Для оптимальной производительности рекомендуется помещать удалённые таблицы, расположенные на одном сервере, в одну запись. Например, вместо:
```ini
agent = remote:9312:idx1
agent = remote:9312:idx2
```
следует предпочесть:
```ini
agent = remote:9312:idx1,idx2
```

## agent_persistent

<!-- example agent-persistent -->
<!-- intro -->
##### Конфигурация:

<!-- request Config -->
```ini
agent_persistent = remotebox:9312:index2
```
<!-- end -->

Опция `agent_persistent` позволяет устанавливать постоянное соединение с агентом, то есть соединение не закрывается после выполнения запроса. Синтаксис этой директивы такой же, как у директивы `agent`. Однако вместо открытия нового соединения с агентом для каждого запроса и последующего его закрытия мастер будет поддерживать открытое соединение и повторно использовать его для последующих запросов. Максимальное количество постоянных соединений на хост агента задаётся опцией [persistent_connections_limit](../../Server_settings/Searchd.md#persistent_connections_limit) в секции searchd.

Важно отметить, что значение [persistent_connections_limit](../../Server_settings/Searchd.md#persistent_connections_limit) должно быть больше 0 для использования постоянных соединений с агентом. Если оно не определено, по умолчанию равно 0, и директива `agent_persistent` будет работать так же, как `agent`.

Использование постоянных соединений мастер-агент снижает нагрузку на TCP-порты и экономит время на установку соединений, делая процесс более эффективным.

## agent_blackhole

<!-- example agent-blackhole -->
Директива `agent_blackhole` позволяет перенаправлять запросы удаленным агентам без ожидания или обработки их ответов. Это полезно для отладки или тестирования рабочих кластеров, так как вы можете настроить отдельный экземпляр для отладки/тестирования и перенаправлять ему запросы с рабочего мастера (агрегатора), не мешая рабочему процессу. Master searchd попытается подключиться к агенту-черной дыре и отправить запросы как обычно, но не будет ждать или обрабатывать никакие ответы, а все сетевые ошибки на агентах-черных дырах будут игнорироваться. Формат значения идентичен формату обычной директивы `agent`.

<!-- intro -->
##### Конфигурация:

<!-- request Config -->
```ini
agent_blackhole = testbox:9312:testindex1,testindex2
```
<!-- end -->

## Связанные опции и поддерживаемые области видимости

Опции, связанные с удаленными агентами, поддерживаются не во всех областях видимости одновременно. На этой странице основное внимание уделяется семантике удаленных таблиц и отдельных агентов. Для поведения, специфичного для зеркал, см. [Зеркалирование](../../Creating_a_cluster/Remote_nodes/Mirroring.md) и [Балансировка нагрузки](../../Creating_a_cluster/Remote_nodes/Load_balancing.md). Для полного поведения на уровне демона используйте связанные справочные страницы в [Настройках Searchd](../../Server_settings/Searchd.md).

| Опция | Для всего экземпляра | На таблицу | На запрос | На агента | Полные детали |
|---|---|---|---|---|---|
| `ha_strategy` | да | да | нет | да | [Балансировка нагрузки](../../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_strategy), [Удаленные таблицы: agent](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) |
| `agent_connect_timeout` | да | да | нет | нет | [Searchd: agent_connect_timeout](../../Server_settings/Searchd.md#agent_connect_timeout) |
| `agent_query_timeout` | да | да | да | нет | [Searchd: agent_query_timeout](../../Server_settings/Searchd.md#agent_query_timeout) |
| `agent_retry_count` / `mirror_retry_count` / `retry_count` | да (`agent_retry_count`) | да (`agent_retry_count` или `mirror_retry_count`) | да (`OPTION retry_count=...`) | да (`agent=...[retry_count=...]`) | [Searchd: agent_retry_count](../../Server_settings/Searchd.md#agent_retry_count), [Удаленные таблицы: mirror_retry_count](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#mirror_retry_count), [Удаленные таблицы: agent](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) |
| `agent_retry_delay` | да | нет | да | нет | [Searchd: agent_retry_delay](../../Server_settings/Searchd.md#agent_retry_delay) |
| per-agent `conn` | нет | нет | нет | да | [Удаленные таблицы: agent](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) |
| per-agent `blackhole` | нет | нет | нет | да | [Удаленные таблицы: agent_blackhole](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_blackhole) |

### agent_connect_timeout

<!-- example remote-agent-connect-timeout -->
`agent_connect_timeout` определяет, сколько времени Manticore ждет установления соединения с удаленным агентом. Поддерживается как глобальная настройка по умолчанию для экземпляра и для каждой распределенной таблицы. Полные детали на уровне демона находятся в [Searchd: agent_connect_timeout](../../Server_settings/Searchd.md#agent_connect_timeout).

<!-- intro -->
##### Конфигурация:

<!-- request Config -->
```ini
agent_connect_timeout = 300ms
```

<!-- intro -->
##### SQL:

<!-- request SQL -->
```sql
CREATE TABLE products_dist type='distributed'
  agent='127.0.0.1:9312:products|127.0.0.1:9313:products'
  agent_connect_timeout='300ms';
```
<!-- end -->

### agent_query_timeout

<!-- example remote-agent-query-timeout -->
`agent_query_timeout` определяет, сколько времени Manticore ждет, пока подключенный удаленный агент завершит выполнение запроса. Поддерживается как глобальная настройка по умолчанию для экземпляра, для каждой распределенной таблицы и для каждого запроса как `OPTION agent_query_timeout=...`. Полные детали на уровне демона находятся в [Searchd: agent_query_timeout](../../Server_settings/Searchd.md#agent_query_timeout).

Если достигается `agent_query_timeout`, запрос не повторяется автоматически; вместо этого выводится предупреждение. Поведение также зависит от [reset_network_timeout_on_packet](../../Server_settings/Searchd.md#reset_network_timeout_on_packet).

<!-- intro -->
##### Конфигурация:

<!-- request Config -->
```ini
agent_query_timeout = 10000 # our query can be long, allow up to 10 sec
```

<!-- intro -->
##### SQL:

<!-- request SQL -->
```sql
CREATE TABLE products_dist type='distributed'
  agent='127.0.0.1:9312:products|127.0.0.1:9313:products'
  agent_query_timeout='10000';
```

<!-- intro -->
##### Опция SQL запроса:

<!-- request SQL -->
```sql
SELECT * FROM products_dist OPTION agent_query_timeout=750;
```
<!-- end -->

### agent_retry_count

`agent_retry_count` определяет, сколько раз Manticore попытается подключиться к удаленным агентам и выполнить запросы в распределенной таблице, прежде чем сообщить о фатальной ошибке запроса. Имя параметра зависит от области применения: используйте
- `agent_retry_count` как настройку уровня экземпляра,
- `agent_retry_count` или его псевдоним `mirror_retry_count` для распределенной таблицы,
- `OPTION retry_count=...` для каждого запроса,
- и `[retry_count=...]` внутри отдельного объявления `agent=...`.

Полные детали на уровне демона приведены в разделе [Searchd: agent_retry_count](../../Server_settings/Searchd.md#agent_retry_count).

Если вы используете **зеркала агентов**, сервер выбирает другое зеркало перед каждой попыткой подключения в соответствии с [ha_strategy](../../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_strategy), а `agent_retry_count` суммируется по всем зеркалам.

### mirror_retry_count

<!-- example remote-agent-retry-count -->
`mirror_retry_count` служит той же цели, что и `agent_retry_count`, но только как настройка распределенной таблицы. Если указаны оба значения, приоритет имеет `mirror_retry_count`.

<!-- intro -->
##### Конфигурация:

<!-- request Config -->
```ini
agent_retry_count = 2
```

<!-- intro -->
##### SQL:

<!-- request SQL -->
```sql
CREATE TABLE products_dist type='distributed'
  agent='127.0.0.1:9312:products|127.0.0.1:9313:products'
  mirror_retry_count='2';
```

<!-- intro -->
##### Опция SQL-запроса:

<!-- request SQL -->
```sql
SELECT * FROM products_dist OPTION retry_count=1;
```

<!-- intro -->
##### Ограничение на агента в конфигурации:

<!-- request Config -->
```ini
table products_dist {
  type = distributed
  agent = 127.0.0.1:9312|127.0.0.1:9313:products[retry_count=2]
}
```
<!-- end -->

### agent_retry_delay

<!-- example remote-agent-retry-delay -->
`agent_retry_delay` определяет задержку между повторными попытками. Он поддерживается как глобальная настройка экземпляра по умолчанию и как `OPTION retry_delay=...` для каждого запроса, но не для распределенной таблицы. Полные детали на уровне демона приведены в разделе [Searchd: agent_retry_delay](../../Server_settings/Searchd.md#agent_retry_delay).

Эта опция актуальна только тогда, когда повторные попытки включены через `agent_retry_count` или `OPTION retry_count=...`.

<!-- intro -->
##### Конфигурация:

<!-- request Config -->
```ini
agent_retry_delay = 500ms
```

<!-- intro -->
##### Опция SQL-запроса:

<!-- request SQL -->
```sql
SELECT * FROM products_dist OPTION retry_count=2, retry_delay=300;
```
<!-- end -->

### client_timeout

<!-- example client-timeout -->
Опция `client_timeout` устанавливает максимальное время ожидания между запросами при использовании постоянных соединений. Это глобальная настройка `searchd` уровня экземпляра, а не настройка отдельной таблицы. Значение выражается в секундах или с временным суффиксом. Значение по умолчанию составляет 5 минут. Полные детали на уровне демона приведены в разделе [Searchd: client_timeout](../../Server_settings/Searchd.md#client_timeout).

<!-- intro -->
##### Конфигурация:

<!-- request Config -->
```ini
client_timeout = 1h
```
<!-- end -->

### hostname_lookup

Опция `hostname_lookup` определяет стратегию обновления имён хостов. По умолчанию IP-адреса host-имён агентов кешируются при запуске сервера, чтобы избежать чрезмерного обращения к DNS. Однако в некоторых случаях IP может динамически меняться (например, облачный хостинг), и может быть необходимо не кешировать IP. Установка этой опции в `request` отключает кеширование и выполняет запросы к DNS при каждом запросе. IP-адреса также можно обновлять вручную с помощью команды `FLUSH HOSTNAMES`.

### listen_tfo

Опция `listen_tfo` позволяет использовать флаг TCP_FASTOPEN для всех слушателей. По умолчанию она управляется системой, но может быть явно отключена, установив значение '0'.

Для получения дополнительной информации о расширении TCP Fast Open, пожалуйста, обратитесь к [Wikipedia](https://en.wikipedia.org/wiki/TCP_Fast_Open). Кратко, оно позволяет исключить один круг обмена TCP при установлении соединения.

На практике использование TFO может оптимизировать сетевую эффективность клиент-агента, аналогично использованию `agent_persistent`, но без удержания активных соединений и без ограничений на максимальное количество соединений.

Большинство современных операционных систем поддерживают TFO. Linux (как одна из самых прогрессивных) поддерживает его с 2011 года, начиная с ядра 3.7 (для серверной стороны). Windows поддерживает его с некоторых сборок Windows 10. Другие системы, такие как FreeBSD и MacOS, также участвуют.

Для систем Linux сервер проверяет переменную `/proc/sys/net/ipv4/tcp_fastopen` и действует соответственно. Бит 0 управляет клиентской стороной, а бит 1 управляет слушателями. По умолчанию система имеет этот параметр, установленный в 1, то есть клиенты включены, а слушатели отключены.

### persistent_connections_limit

```ini
persistent_connections_limit = 29 # assume that each host of agents has max_connections = 30 (or 29).
```

Опция `persistent_connections_limit` определяет максимальное количество одновременных постоянных соединений с удалёнными постоянными агентами. Это настройка на уровне экземпляра и должна быть определена в секции конфигурации searchd. Каждый раз при установлении соединения с агентом, определённым в `agent_persistent`, происходит попытка повторно использовать существующее соединение (если оно есть) или создать новое и сохранить его для будущего использования. Однако в некоторых случаях может потребоваться ограничить количество постоянных соединений. Эта директива задаёт лимит и влияет на количество соединений с каждым хостом агента для всех распределённых таблиц.

Рекомендуется установить это значение равным или меньшим, чем опция [max_connections](../../Server_settings/Searchd.md#max_connections) в конфигурации агента.

## Создание распределённых сниппетов

Особый случай распределённой таблицы — одна локальная и несколько удалённых, которая используется исключительно для [создания распределённых сниппетов](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#Distributed-snippets-creation), когда сниппеты берутся из файлов. В этом случае локальная таблица может выступать в роли "шаблонной" таблицы, обеспечивая настройки для токенизации при построении сниппетов.

### snippets_file_prefix

```ini
snippets_file_prefix = /mnt/common/server1/
```

Опция `snippets_file_prefix` — это необязательный префикс, который можно добавить к именам локальных файлов при генерации сниппетов. Значение по умолчанию — текущая рабочая папка.

Чтобы узнать больше о создании распределённых сниппетов, смотрите [CALL SNIPPETS](../../Searching/Highlighting.md).

## Распределённые перколяционные таблицы (DPQ таблицы)

Вы можете создать распределённую таблицу из нескольких [перколяционных](../../Creating_a_table/Local_tables/Percolate_table.md) таблиц. Синтаксис построения такого типа таблиц такой же, как и для других распределённых таблиц, и может включать несколько `local` таблиц, а также `agents`.

Для DPQ операции перечисления сохранённых запросов и поиска по ним (с использованием [CALL PQ](../../Searching/Percolate_query.md#Performing-a-percolate-query-with-CALL-PQ)) прозрачны и работают так, как если бы все таблицы были одной локальной таблицей. Однако операции манипуляции данными, такие как `insert`, `replace`, `truncate`, отсутствуют.

Если в список агентов включить не перколяционную таблицу, поведение будет неопределённым. Если неправильный агент имеет ту же схему, что и внешняя схема PQ таблицы (id, query, tags, filters), это не вызовет ошибку при перечислении сохранённых правил PQ и может засорить список фактических правил PQ, хранящихся в PQ таблицах, собственными не-PQ строками. В результате будьте осторожны и понимаете путаницу, которую это может вызвать. Вызов `CALL PQ` к такому неправильному агенту вызовет ошибку.

Для получения дополнительной информации о выполнении запросов к распределённой перколяционной таблице смотрите [выполнение запросов к распределённой перколяционной таблице](../../Searching/Percolate_query.md#Performing-a-percolate-query-with-CALL-PQ).
<!-- proofread -->

