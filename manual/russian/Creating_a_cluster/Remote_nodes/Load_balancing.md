# Балансировка нагрузки

Балансировка нагрузки включена по умолчанию для любой [распределенной таблицы](../../Creating_a_table/Creating_a_distributed_table/Creating_a_distributed_table.md), использующей [зеркалирование](../../Creating_a_cluster/Remote_nodes/Mirroring.md). По умолчанию запросы распределяются случайным образом между зеркалами. Это поведение можно изменить с помощью [ha_strategy](../../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_strategy).

## ha_strategy

```ini
ha_strategy = {random|nodeads|noerrors|roundrobin}
```

Стратегия выбора зеркала для балансировки нагрузки является опциональной и по умолчанию установлена в значение `random`.

`ha_strategy` может быть установлена глобально в конфигурации `searchd` или для каждой распределенной таблицы отдельно. Эта директива управляет стратегией выбора зеркала, или, другими словами, выбором конкретного [зеркала агента](../../Creating_a_cluster/Remote_nodes/Mirroring.md#Agent-mirrors) в распределенной таблице. На практике эта директива контролирует, как мастер балансирует запросы между зеркальными узлами агентов. Реализованы следующие стратегии:

### Простая случайная балансировка

<!-- example conf balancing 1 -->
Режим балансировки по умолчанию — простая линейная случайная выборка среди зеркал. Каждому зеркалу назначаются равные вероятности выбора. Это похоже на циклический перебор (round-robin, RR), но не накладывает строгого порядка выбора.

<!-- intro -->
##### Конфигурация:

<!-- request Config -->
```ini
ha_strategy = random
```

<!-- intro -->
##### SQL:

<!-- request SQL -->
```sql
CREATE TABLE products_dist type='distributed'
  agent='127.0.0.1:9312:products|127.0.0.1:9313:products'
  ha_strategy='random';
```
<!-- end -->

### Адаптивная рандомизированная балансировка

Простая случайная стратегия по умолчанию не учитывает статус зеркал, частоту ошибок и, что наиболее важно, фактическую задержку ответов. Для работы с неоднородными кластерами и временными всплесками нагрузки на узлах агентов существует группа стратегий балансировки, которые динамически корректируют вероятности на основе задержек запросов, наблюдаемых мастером.

Адаптивные стратегии, основанные на **вероятностях, взвешенных по задержке**, работают следующим образом. Размер окна статистики контролируется параметром [`ha_period_karma`](../../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_period_karma), в то время как проверка работоспособности неактивных зеркал и отслеживание времени кругового оборота управляются параметром [`ha_ping_interval`](../../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_ping_interval). Оба параметра описаны далее на этой странице и в разделе [Настройки Searchd](../../Server_settings/Searchd.md).

1. Статистика задержек накапливается блоками по `ha_period_karma` секунд.
2. Вероятности, взвешенные по задержкам, пересчитываются один раз за период кармы.
3. Флаг "живой или мёртвый" корректируется на каждый запрос, включая ping-запросы.

Изначально вероятности равны. На каждом шаге они масштабируются обратными значениями задержек, наблюдаемых в течение последнего периода кармы, а затем нормализуются заново. Например, если в первые 60 секунд после запуска мастера 4 зеркала показали задержки 10 мс, 5 мс, 30 мс и 3 мс соответственно, то первый шаг корректировки будет следующим:

1. Исходные проценты: 0.25, 0.25, 0.25, 0.25.
2. Наблюдаемые задержки: 10 мс, 5 мс, 30 мс, 3 мс.
3. Обратные значения задержек: 0.1, 0.2, 0.0333, 0.333.
4. Масштабированные проценты: 0.025, 0.05, 0.008333, 0.0833.
5. Нормализованные проценты: 0.15, 0.30, 0.05, 0.50.

Это означает, что первое зеркало будет иметь 15% шанс быть выбранным в следующий период кармы, второе — 30%, третье (самое медленное, с задержкой 30 мс) — только 5%, а четвёртое и самое быстрое (3 мс) — 50%. После этого периода второй шаг корректировки снова обновит эти шансы и так далее.

Идея в том, что как только **наблюдаемые задержки** стабилизируются, **вероятности, взвешенные по задержке**, также стабилизируются. Все эти итерации корректировки предназначены для сходимости к точке, где средние задержки примерно равны на всех зеркалах.

#### nodeads

<!-- example conf balancing 2 -->
Вероятности, взвешенные по задержке, но зеркала со слишком большим количеством последовательных неудачных попыток исключаются из выбора. Счетчик последовательных ошибок сбрасывается только полностью успешным запросом. Предупреждения не сбрасывают его. Зеркало начинает считаться мертвым после более чем 3 последовательных неуспешных исходов, поэтому первые 3 неудачи подряд допускаются, но 4-я делает это зеркало непригодным для выбора по стратегии `nodeads`, пока оно снова не ответит успешно.

<!-- intro -->
##### Конфигурация:

<!-- request Config -->
```ini
ha_strategy = nodeads
```

<!-- intro -->
##### SQL:

<!-- request SQL -->
```sql
CREATE TABLE products_dist type='distributed'
  agent='127.0.0.1:9312:products|127.0.0.1:9313:products'
  ha_strategy='nodeads';
```
<!-- end -->

#### noerrors

<!-- example conf balancing 3 -->
Вероятности, взвешенные по задержке, но зеркала с худшим недавним соотношением ошибок/успехов понижаются в приоритете и могут быть исключены из выбора.

`noerrors` анализирует недавние счетчики для каждого зеркала, собранные за последние окна `ha_period_karma`. Сначала он сравнивает соотношение критических сбоев транспорта (`connect_timeouts`, `connect_failures`, `network_errors`, `wrong_replies`, `unexpected_closings`) к общей недавней активности. Соотношения на уровне 3% или ниже считаются практически нулевыми. Если несколько зеркал имеют одинаковый результат, затем сравнивается более широкое соотношение ошибок, которое также включает ответы с предупреждениями. Зеркала без успешных запросов вообще в последнем окне полностью пропускаются.

Таким образом, `noerrors` наиболее полезен для периодических сбоев на транспортном уровне и деградировавших зеркал, а не как гарантия того, что каждая логическая проблема удаленной таблицы исчезнет немедленно. Например, если одно зеркало указывает на отсутствующую таблицу, вы все равно можете увидеть сбои запросов, прежде чем это зеркало накопит достаточно плохой истории, чтобы перестать быть предпочтительным. Используйте [`SHOW AGENT STATUS`](../../Node_info_and_management/Node_status.md#SHOW-AGENT-STATUS) для проверки недавних успехов, предупреждений и сетевых ошибок для каждого зеркала.

<!-- intro -->
##### Конфигурация:

<!-- request Config -->
```ini
ha_strategy = noerrors
```

<!-- intro -->
##### SQL:

<!-- request SQL -->
```sql
CREATE TABLE products_dist type='distributed'
  agent='127.0.0.1:9312:products|127.0.0.1:9313:products'
  ha_strategy='noerrors';
```
<!-- end -->

### Круговая балансировка (Round-robin)

<!-- example conf balancing 4 -->
Простой циклический выбор, то есть выбор первого зеркала в списке, затем второго, затем третьего и так далее, с повторением процесса после достижения последнего зеркала в списке. В отличие от рандомизированных стратегий, RR устанавливает строгий порядок запросов (1, 2, 3, ..., N-1, N, 1, 2, 3, ... и так далее) и гарантирует, что два последовательных запроса не будут отправлены на одно и то же зеркало, пока все зеркала работоспособны.

<!-- intro -->
##### Конфигурация:

<!-- request Config -->
```ini
ha_strategy = roundrobin
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

Предположим, что зеркальные распределенные таблицы уже существуют, например:

```sql
CREATE TABLE products_random type='distributed'
  agent='127.0.0.1:9312:products|127.0.0.1:9313:products';

CREATE TABLE products_rr type='distributed'
  agent='127.0.0.1:9312:products|127.0.0.1:9313:products'
  ha_strategy='roundrobin';
```

<!-- example load-balancing-sql-behavior -->
Разница между `random` и `roundrobin` становится заметной при повторяющихся запросах к зеркальным распределенным таблицам.

<!-- intro -->
##### SQL запрос:

<!-- request SQL -->
```sql
SELECT node FROM products_random;
SELECT node FROM products_random;
SELECT node FROM products_rr;
SELECT node FROM products_rr;
```

<!-- intro -->
##### SQL ответ:

<!-- response SQL -->
```sql
+-------+
| node  |
+-------+
| node2 |
+-------+

+-------+
| node  |
+-------+
| node1 |
+-------+

+-------+
| node  |
+-------+
| node1 |
+-------+

+-------+
| node  |
+-------+
| node2 |
+-------+
```
<!-- end -->

## SHOW AGENT STATUS

<!-- example load-balancing-sql-verification -->
`SHOW AGENT STATUS` — это основной инструмент отладки для зеркальных распределенных таблиц. Полный синтаксис оператора и все форматы вывода смотрите в [SHOW AGENT STATUS](../../Node_info_and_management/Node_status.md#SHOW-AGENT-STATUS). Для балансировки нагрузки наиболее полезными полями являются:

- `ag_N_pingtripmsec` — текущее время кругового пути пинга для зеркала `N`
- `ag_N_errorsarow` — последовательные неуспешные результаты; это то, за чем фактически следит `nodeads`
- `ag_N_1periods_connect_timeouts`, `connect_failures`, `network_errors`, `wrong_replies`, `unexpected_closings` — недавние транспортные сбои
- `ag_N_1periods_warnings` — ответы, которые завершились успешно, но вернули предупреждения
- `ag_N_1periods_succeeded_queries` — недавние успешные запросы
- `ag_N_1periods_msecsperquery` — недавнее среднее время запроса

Эти счетчики за период отчитываются за скользящие окна 1, 5 и 15, где каждое окно имеет длину `ha_period_karma` секунд.

Используйте эти команды на мастере, чтобы проверить, что было сохранено и как ведут себя зеркала.

<!-- intro -->
##### SQL запрос:

<!-- request SQL -->
```sql
SHOW CREATE TABLE products_rr;
SHOW AGENT STATUS;
SHOW AGENT STATUS LIKE '%errorsarow%';
SHOW AGENT STATUS LIKE '%1periods_%';
```

<!-- intro -->
##### SQL ответ:

<!-- response SQL -->
```sql
+---------------------------------+-------+
| Key                             | Value |
+---------------------------------+-------+
| ag_0_pingtripmsec               | 0.233 |
| ag_0_errorsarow                 | 0     |
| ag_0_1periods_network_errors    | 0     |
| ag_0_1periods_warnings          | 0     |
| ag_0_1periods_succeeded_queries | 11    |
| ag_0_1periods_msecsperquery     | n/a   |
+---------------------------------+-------+
```
<!-- end -->

Практическая интерпретация:

- Если `errorsarow` превышает 3, `nodeads` начинает считать это зеркало неработоспособным.
- Если у зеркала нет недавних `succeeded_queries`, `noerrors` полностью пропустит его.
- Если у нескольких зеркал низкий коэффициент ошибок, `noerrors` все равно предпочтет те, у которых меньше задержка, потому что обычная перебалансировка весов остается в силе.
- `pingtripmsec` помогает отличить "доступные, но медленные" зеркала от "сбойных".

## Связанные опции и поддерживаемые области видимости

Опции, связанные с зеркалированием, таймаутами и повторными попытками, поддерживаются не во всех областях видимости одновременно. Эта страница фокусируется на том, как они влияют на балансировку нагрузки. Полный синтаксис и поведение на уровне демона смотрите на связанных справочных страницах.

| Опция | Для всего экземпляра | На таблицу | На запрос | На агента | Полные детали |
|---|---|---|---|---|---|
| `ha_strategy` | да | да | нет | да | [Балансировка нагрузки](../../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_strategy), [Удаленные таблицы](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) |
| `ha_period_karma` | да | нет | нет | нет | [Searchd: ha_period_karma](../../Server_settings/Searchd.md#ha_period_karma) |
| `ha_ping_interval` | да | нет | нет | нет | [Searchd: ha_ping_interval](../../Server_settings/Searchd.md#ha_ping_interval) |
| `agent_connect_timeout` | да | да | нет | нет | [Searchd: agent_connect_timeout](../../Server_settings/Searchd.md#agent_connect_timeout), [Удаленные таблицы: agent_connect_timeout](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_connect_timeout) |
| `agent_query_timeout` | да | да | да | нет | [Searchd: agent_query_timeout](../../Server_settings/Searchd.md#agent_query_timeout), [Удаленные таблицы: agent_query_timeout](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_query_timeout) |
| `agent_retry_count` / `mirror_retry_count` / `retry_count` | да (`agent_retry_count`) | да (`agent_retry_count` или `mirror_retry_count`) | да (`OPTION retry_count=...`) | да (`agent=...[retry_count=...]`) | [Searchd: agent_retry_count](../../Server_settings/Searchd.md#agent_retry_count), [Удаленные таблицы: agent_retry_count](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_retry_count), [Удаленные таблицы: mirror_retry_count](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#mirror_retry_count), [Удаленные таблицы: agent](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) |
| `agent_retry_delay` | да | нет | да | нет | [Searchd: agent_retry_delay](../../Server_settings/Searchd.md#agent_retry_delay) |

### ha_strategy

<!-- example ha-strategy-scopes -->
`ha_strategy` управляет выбором зеркальных агентов. Её можно установить глобально в `searchd`, для каждой распределенной таблицы или для каждого агента внутри `agent = ... [ha_strategy=...]`. Более узкая область видимости переопределяет более широкую.

Примеры стратегий выше уже показывают как глобальную, так и табличную формы. Синтаксис переопределения для конкретного агента смотрите в [Удаленных таблицах](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent).

Если вы хотите, чтобы одна распределенная таблица использовала стратегию балансировки, отличную от глобальной по умолчанию, установите её для этой таблицы.

<!-- intro -->
##### Конфигурация:

<!-- request Config -->
```ini
ha_strategy = random

table products_rr {
  type = distributed
  agent = 127.0.0.1:9312|127.0.0.1:9313:products
  ha_strategy = roundrobin
}
```

<!-- intro -->
##### SQL:

<!-- request SQL -->
```sql
CREATE TABLE products_rr type='distributed'
  agent='127.0.0.1:9312:products|127.0.0.1:9313:products'
  ha_strategy='roundrobin';
```
<!-- end -->

### ha_period_karma

<!-- example ha-period-karma-scope -->
`ha_period_karma` определяет окно статистики зеркал агентов, используемое адаптивными стратегиями балансировки выше, и поддерживается только как настройка `searchd` для всего экземпляра. Полные детали на уровне демона находятся в [Searchd: ha_period_karma](../../Server_settings/Searchd.md#ha_period_karma).

<!-- intro -->
##### Конфигурация:

<!-- request Config -->
```ini
ha_period_karma = 2m
```
<!-- end -->

### ha_ping_interval

<!-- example ha-ping-interval-scope -->
`ha_ping_interval` определяет, как часто проверяются неактивные зеркала, и поддерживается только как настройка `searchd` на уровне всего экземпляра. Полные детали на уровне демона находятся в [Searchd: ha_ping_interval](../../Server_settings/Searchd.md#ha_ping_interval).

<!-- intro -->
##### Конфигурация:

<!-- request Config -->
```ini
ha_ping_interval = 3s
```
<!-- end -->

### agent_connect_timeout

<!-- example agent-connect-timeout-scopes -->
`agent_connect_timeout` определяет, сколько времени Manticore ожидает для установления соединения с удалённым агентом. Он поддерживается как настройка по умолчанию для всего экземпляра и для каждой распределённой таблицы. Полные детали находятся в [Searchd: agent_connect_timeout](../../Server_settings/Searchd.md#agent_connect_timeout) и [Remote tables: agent_connect_timeout](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_connect_timeout).

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

<!-- example agent-query-timeout-scopes -->
`agent_query_timeout` определяет, сколько времени Manticore ожидает, чтобы подключенный удалённый агент завершил запрос. Он поддерживается как настройка по умолчанию для всего экземпляра, для каждой распределённой таблицы и для каждого запроса как `OPTION agent_query_timeout=...`. Полные детали находятся в [Searchd: agent_query_timeout](../../Server_settings/Searchd.md#agent_query_timeout) и [Remote tables: agent_query_timeout](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_query_timeout).

Если достигается `agent_query_timeout`, запрос не повторяется автоматически; вместо этого выводится предупреждение.

<!-- intro -->
##### Конфигурация:

<!-- request Config -->
```ini
agent_query_timeout = 500ms
```

<!-- intro -->
##### SQL:

<!-- request SQL -->
```sql
CREATE TABLE products_dist type='distributed'
  agent='127.0.0.1:9312:products|127.0.0.1:9313:products'
  agent_query_timeout='500ms';
```

<!-- intro -->
##### Опция SQL запроса:

<!-- request SQL -->
```sql
SELECT * FROM products_dist OPTION agent_query_timeout=750;
```
<!-- end -->

### agent_retry_count и mirror_retry_count

<!-- example agent-retry-count-scopes -->
`agent_retry_count` определяет, сколько раз Manticore повторяет работу удалённого агента перед сообщением о фатальной ошибке запроса. Имя варьируется по области применения: используйте `agent_retry_count` как настройку для всего экземпляра, `agent_retry_count` или его алиас `mirror_retry_count` на распределённой таблице, `OPTION retry_count=...` для каждого запроса и `[retry_count=...]` внутри отдельного объявления `agent=...`. Полные детали находятся в [Searchd: agent_retry_count](../../Server_settings/Searchd.md#agent_retry_count), [Remote tables: agent_retry_count](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_retry_count) и [Remote tables: mirror_retry_count](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#mirror_retry_count).

Если вы используете зеркала агентов, количество повторений суммируется по всем зеркалам. Опция `[retry_count=...]` для каждого агента действует как абсолютное ограничение для этого конкретного объявления агента.

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
##### Опция SQL запроса:

<!-- request SQL -->
```sql
SELECT * FROM products_dist OPTION retry_count=1;
```

<!-- intro -->
##### Конфигурация ограничения для каждого агента:

<!-- request Config -->
```ini
table products_dist {
  type = distributed
  agent = 127.0.0.1:9312|127.0.0.1:9313:products[retry_count=2]
}
```
<!-- end -->

### agent_retry_delay

<!-- example agent-retry-delay-scopes -->
`agent_retry_delay` определяет задержку между попытками повторения. Он поддерживается как настройка по умолчанию для всего экземпляра и для каждого запроса как `OPTION retry_delay=...`, но не для каждой распределённой таблицы. Полные детали находятся в [Searchd: agent_retry_delay](../../Server_settings/Searchd.md#agent_retry_delay).

Эта опция имеет значение только когда повторения включены через `agent_retry_count` или `OPTION retry_count=...`.

<!-- intro -->
##### Конфигурация:

<!-- request Config -->
```ini
agent_retry_delay = 500ms
```

<!-- intro -->
##### Опция SQL запроса:

<!-- request SQL -->
```sql
SELECT * FROM products_dist OPTION retry_count=2, retry_delay=300;
```
<!-- end -->

<!-- proofread -->
