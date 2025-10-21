# Понимание статуса кластера и состояний узлов

В этом разделе приводятся подробные объяснения значений `cluster_%_status` и `cluster_%_node_state`, которые отображаются в выводе статуса кластера, включая описание того, что означает каждая комбинация в различных эксплуатационных сценариях.

## Значения cluster_%_status

Переменная `cluster_%_status` указывает статус компонента кластера с точки зрения данного узла. Понимание этих статусов важно для правильного управления кластером и устранения неисправностей.

### Статус PRIMARY

<!-- example primary status -->
Когда статус кластера показывает `PRIMARY`, кластер имеет кворум и может принимать операции как на чтение, так и на запись. Это нормальное рабочее состояние.

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SHOW STATUS LIKE 'cluster_%_status'
```

<!-- response SQL-->

```sql
+----------------------------+-------+
| Counter                    | Value |
+----------------------------+-------+
| cluster_posts_status       | primary |
+----------------------------+-------+
```

<!-- end -->

Статус `PRIMARY` означает:
* Большинство узлов в кластере подключены и обмениваются данными
* Операции записи разрешены и будут реплицироваться
* Кластер функционирует нормально
* Кворум поддерживается

### Статус NON_PRIMARY

<!-- example non_primary status -->
Когда статус кластера показывает `NON_PRIMARY`, кластер потерял кворум. Это критическое состояние, при котором операции записи блокируются, чтобы предотвратить сценарии разделения мозга.

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SHOW STATUS LIKE 'cluster_%_status'
```

<!-- response SQL-->

```sql
+----------------------------+-------+
| Counter                    | Value |
+----------------------------+-------+
| cluster_posts_status       | non-primary |
+----------------------------+-------+
```

<!-- end -->

Статус `NON_PRIMARY` возникает когда:
* Более половины узлов в кластере недоступны
* Произошло разделение сети, разбившее кластер
* Текущая группа узлов не может сформировать большинство

**Критично**: операции записи в этом состоянии отклоняются. Может потребоваться ручное вмешательство с помощью команды bootstrap:

<!-- example non_primary recovery -->

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SET CLUSTER posts GLOBAL 'pc.bootstrap' = 1
```

<!-- request JSON -->

```json
POST /cli -d "
SET CLUSTER posts GLOBAL 'pc.bootstrap' = 1
"
```

<!-- end -->

### Статус DISCONNECTED

<!-- example disconnected status -->
Когда статус кластера показывает `DISCONNECTED`, узел не подключён ни к одному компоненту кластера и пытается переподключиться.

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SHOW STATUS LIKE 'cluster_%_status'
```

<!-- response SQL-->

```sql
+----------------------------+-------+
| Counter                    | Value |
+----------------------------+-------+
| cluster_posts_status       | disconnected |
+----------------------------+-------+
```

<!-- end -->

Статус `DISCONNECTED` обычно встречается при:
* Запуске узла до присоединения к кластеру
* Проблемах с сетевым подключением
* Ошибках формирования кластера
* Временной изоляции от других участников кластера

## Значения cluster_%_node_state

Переменная `cluster_%_node_state` показывает текущее рабочее состояние данного конкретного узла в кластере.

### Состояние synced

<!-- example synced state -->
Когда состояние узла показывает `synced`, узел полностью синхронизирован и работает. Это нормальное рабочее состояние.

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SHOW STATUS LIKE 'cluster_%_node_state'
```

<!-- response SQL-->

```sql
+----------------------------+-------+
| Counter                    | Value |
+----------------------------+-------+
| cluster_posts_node_state   | synced |
+----------------------------+-------+
```

<!-- end -->

Состояние `synced` означает:
* Узел может обслуживать как запросы на чтение, так и на запись
* Все транзакции реплицируются нормально
* Узел поддерживает кворум кластера
* Данные согласованы с другими участниками кластера

### Состояние joining

<!-- example joining state -->
Когда состояние узла показывает `joining`, узел находится в процессе присоединения к кластеру и получения синхронизации состояния.

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SHOW STATUS LIKE 'cluster_%_node_state'
```

<!-- response SQL-->

```sql
+----------------------------+-------+
| Counter                    | Value |
+----------------------------+-------+
| cluster_posts_node_state   | joining |
+----------------------------+-------+
```

<!-- end -->

Во время состояния `joining`:
* Узел получает состояние кластера (IST или SST)
* Узел пока не может обслуживать запросы на чтение или запись
* Идёт передача состояния
* Узел перейдет в состояние `synced` после завершения

### Состояние donor

<!-- example donor state -->
Когда состояние узла показывает `donor`, узел предоставляет передачу состояния для присоединяющегося узла.

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SHOW STATUS LIKE 'cluster_%_node_state'
```

<!-- response SQL-->

```sql
+----------------------------+-------+
| Counter                    | Value |
+----------------------------+-------+
| cluster_posts_node_state   | donor |
+----------------------------+-------+
```

<!-- end -->

Во время состояния `donor`:
* Узел продолжает обслуживать запросы, но может испытывать задержки
* Производится передача состояния (SST) другому узлу
* Узел возвращается в состояние `synced` после завершения передачи
* Возможны временные ухудшения производительности

### Состояние closed

<!-- example closed state -->
Когда состояние узла показывает `closed`, узел выключен или ещё не запущен.

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SHOW STATUS LIKE 'cluster_%_node_state'
```

<!-- response SQL-->

```sql
+----------------------------+-------+
| Counter                    | Value |
+----------------------------+-------+
| cluster_posts_node_state   | closed |
+----------------------------+-------+
```

<!-- end -->

Состояние `closed` означает:
* Узел был корректно выключен
* Узел ещё не запущен
* Это начальное состояние перед присоединением к кластеру
* Нормальное состояние во время планового обслуживания

### Состояние destroyed

<!-- example destroyed state -->
Когда состояние узла показывает `destroyed`, узел столкнулся с ошибкой и был аварийно завершен.

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SHOW STATUS LIKE 'cluster_%_node_state'
```

<!-- response SQL-->

```sql
+----------------------------+-------+
| Counter                    | Value |
+----------------------------+-------+
| cluster_posts_node_state   | destroyed |
+----------------------------+-------+
```

<!-- end -->

Состояние `destroyed` означает:
* Произошла серьёзная проблема, требующая расследования
* Возможно, потребуется ручное вмешательство для перезапуска узла
* Возможна порча данных или сбой системы
* Перед перезапуском следует проверить журналы на предмет ошибок

## Комбинации статусов в разных сценариях

### Нормальная работа

| Сценарий | cluster_%_status | cluster_%_node_state | Описание |
|----------|------------------|---------------------|-------------|
| Здоровый кластер | `primary` | `synced` | Нормальная работа, возможны чтение и запись |
| Запуск узла | `disconnected` → `primary` | `closed` → `joining` → `synced` | Обычная последовательность запуска |
| Присоединение нового узла | `primary` | `donor` (хелпер), `joining` (новый) | Идёт SST |

### Сетевые проблемы

| Сценарий | cluster_%_status | cluster_%_node_state | Требуемое действие |
|----------|------------------|---------------------|-----------------|
| Миноритарный разлад (2 из 5 узлов) | `non-primary` | `synced` | Ожидать восстановления сети или выполнять bootstrap |
| Мажоритарный разлад (3 из 5 узлов) | `primary` | `synced` | Продолжать работу в нормальном режиме |
| Полная изоляция | `disconnected` | `synced` | Проверить сетевое подключение |

### Отказы узлов

| Сценарий | cluster_%_status | cluster_%_node_state | Влияние |
|----------|------------------|---------------------|--------|
| Отказ одного узла (кластер из 5 узлов) | `primary` | `synced` | Кластер продолжает работу в нормальном режиме |
| Кворум потерян (3+ узлов не работают) | `non-primary` | `synced` | Запись запрещена |
| Узел аварийно завершил работу | зависит | `destroyed` | Требуется перезапуск/восстановление узла |

### Критические сценарии сбоев

#### Большинство узлов не работают (кворум потерян)

<!-- example quorum lost scenario -->
Когда большинство узлов в вашем кластере становятся недоступными, оставшиеся узлы показывают статус `NON_PRIMARY` и отвергают операции записи.

В кластере из 5 узлов, где 3+ узлов вышли из строя:

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SHOW STATUS LIKE 'cluster_%_status';
SHOW STATUS LIKE 'cluster_%_node_state';
SHOW STATUS LIKE 'cluster_%_size';
```

<!-- response SQL-->

```sql
+----------------------------+-------+
| Counter                    | Value |
+----------------------------+-------+
| cluster_posts_status       | non-primary |
| cluster_posts_node_state   | synced |
| cluster_posts_size         | 2 |
+----------------------------+-------+
```

<!-- end -->

Чтобы выйти из этой ситуации, определите узел с наибольшим значением `last_committed` и загрузите его:

<!-- example quorum recovery -->

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SHOW STATUS LIKE 'cluster_%_last_committed';
SET CLUSTER posts GLOBAL 'pc.bootstrap' = 1;
```

<!-- response SQL-->

```sql
+----------------------------+-------+
| Counter                    | Value |
+----------------------------+-------+
| cluster_posts_last_committed | 1547 |
+----------------------------+-------+
```

<!-- request JSON -->

```json
POST /cli -d "
SHOW STATUS LIKE 'cluster_%_last_committed';
SET CLUSTER posts GLOBAL 'pc.bootstrap' = 1;
"
```

<!-- end -->

#### Сетевой разрыв (предотвращение split-brain)

<!-- example network partition -->
В случае сетевого разрыва только сторона с большинством сохраняет статус `PRIMARY`:

**Сторона большинства (3 узла в кластере из 5 узлов):**

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SHOW STATUS LIKE 'cluster_%_status';
SHOW STATUS LIKE 'cluster_%_size';
```

<!-- response SQL-->

```sql
+----------------------------+-------+
| Counter                    | Value |
+----------------------------+-------+
| cluster_posts_status       | primary |
| cluster_posts_size         | 3 |
+----------------------------+-------+
```

**Сторона меньшинства (2 узла в кластере из 5 узлов):**

<!-- request SQL -->

```sql
SHOW STATUS LIKE 'cluster_%_status';
SHOW STATUS LIKE 'cluster_%_size';
```

<!-- response SQL-->

```sql
+----------------------------+-------+
| Counter                    | Value |
+----------------------------+-------+
| cluster_posts_status       | non-primary |
| cluster_posts_size         | 2 |
+----------------------------+-------+
```

<!-- end -->

### Переходы состояний при распространённых событиях

#### Присоединение нового узла (успешное)
```
New Node: disconnected/closed → disconnected/joining → primary/synced
Donor Node: primary/synced → primary/donor → primary/synced
Other Nodes: primary/synced (unchanged)
```

#### Восстановление после сетевого разрыва
```
Minority Partition: non-primary/synced → primary/synced (when reconnected)
Majority Partition: primary/synced (unchanged throughout)
```

#### Ручное восстановление после полной потери кластера
```
First node: disconnected/closed → primary/synced (after pc.bootstrap=1)
Other nodes: disconnected/closed → primary/joining → primary/synced
```

### Мониторинг состояния кластера

<!-- example monitoring cluster health -->
Для мониторинга состояния кластера и выявления проблем проверьте эти ключевые статусные переменные:

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SHOW STATUS LIKE 'cluster_%_status';
SHOW STATUS LIKE 'cluster_%_node_state';
SHOW STATUS LIKE 'cluster_%_size';
SHOW STATUS LIKE 'cluster_%_last_committed';
```

<!-- response SQL-->

```sql
+----------------------------+-------+
| Counter                    | Value |
+----------------------------+-------+
| cluster_posts_status       | primary |
| cluster_posts_node_state   | synced |
| cluster_posts_size         | 3 |
| cluster_posts_last_committed | 1547 |
+----------------------------+-------+
```

<!-- end -->

<!-- example monitoring nodes view -->
Чтобы увидеть разницу между сконфигурированными и видимыми узлами:

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SHOW STATUS LIKE 'cluster_%_nodes_set';
SHOW STATUS LIKE 'cluster_%_nodes_view';
```

<!-- response SQL-->

```sql
+----------------------------+--------------------------------------------------------+
| Counter                    | Value                                                  |
+----------------------------+--------------------------------------------------------+
| cluster_posts_nodes_set    | 10.10.0.1:9312,10.10.1.1:9312,10.10.2.1:9312        |
| cluster_posts_nodes_view   | 10.10.0.1:9312:replication,10.10.1.1:9312:replication |
+----------------------------+--------------------------------------------------------+
```

<!-- end -->

Это показывает, что один узел (10.10.2.1:9312) сконфигурирован, но в данный момент не виден в кластере, что может указывать на проблему с подключением.

## Устранение распространённых проблем

### Операции записи отклонены

<!-- example write rejected -->
Когда статус кластера `NON_PRIMARY`, операции записи будут отклонены:

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
INSERT INTO posts:articles VALUES (1, 'test article');
```

<!-- response SQL-->

```sql
ERROR 1064 (42000): cluster posts is not ready, not primary state (replication error)
```

<!-- end -->

**Решение**: Проверьте статус кластера и либо дождитесь восстановления кворума, либо выполните ручную загрузку, если узлы недоступны постоянно.

### Узел застрял в состоянии joining

<!-- example stuck joining -->
Если узел находится в состоянии `joining` длительное время:

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SHOW STATUS LIKE 'cluster_%_node_state';
SHOW STATUS LIKE 'cluster_%_local_recv_queue';
```

<!-- response SQL-->

```sql
+----------------------------+-------+
| Counter                    | Value |
+----------------------------+-------+
| cluster_posts_node_state   | joining |
| cluster_posts_local_recv_queue | 150 |
+----------------------------+-------+
```

<!-- end -->

Это может означать:
* Проблемы с пропускной способностью сети во время SST
* Идёт передача большого объёма данных
* Проблемы с производительностью узла-донора

Проверьте статус узла-донора и сетевое соединение между узлами.

### Несовпадение размера кластера

<!-- example size mismatch -->
Если `cluster_%_size` не соответствует ожидаемому количеству узлов:

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SHOW STATUS LIKE 'cluster_%_size';
SHOW STATUS LIKE 'cluster_%_nodes_view';
```

<!-- response SQL-->

```sql
+----------------------------+-------+
| Counter                    | Value |
+----------------------------+-------+
| cluster_posts_size         | 2 |
| cluster_posts_nodes_view   | 10.10.0.1:9312:replication,10.10.1.1:9312:replication |
+----------------------------+-------+
```

<!-- end -->

Это указывает на то, что некоторые сконфигурированные узлы сейчас недоступны. Проверьте сетевое соединение и состояние этих узлов.

<!-- proofread -->
