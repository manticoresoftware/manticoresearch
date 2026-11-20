# Присоединение к кластеру репликации

<!-- example joining a replication cluster 1 -->
Для присоединения к существующему кластеру необходимо указать как минимум:
* [Имя](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Replication-cluster) кластера
* `host:port` другого узла кластера, к которому вы присоединяетесь

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
JOIN CLUSTER posts AT '10.12.1.35:9312'
```

<!-- request JSON -->

```json
POST /cli -d "
JOIN CLUSTER posts AT '10.12.1.35:9312'
"
```

<!-- request PHP -->

```php
$params = [
  'cluster' => 'posts',
  'body' => [
      '10.12.1.35:9312'
  ]
];
$response = $client->cluster->join($params);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('JOIN CLUSTER posts AT \'10.12.1.35:9312\'')
```

<!-- response Python -->
```python
{u'error': u'', u'total': 0, u'warning': u''}
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('JOIN CLUSTER posts AT \'10.12.1.35:9312\'')
```

<!-- response Python-asyncio -->
```python
{u'error': u'', u'total': 0, u'warning': u''}
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('JOIN CLUSTER posts AT \'10.12.1.35:9312\'');
```

<!-- response javascript -->
```javascript
{"total":0,"error":"","warning":""}
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("JOIN CLUSTER posts AT '10.12.1.35:9312'");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("JOIN CLUSTER posts AT '10.12.1.35:9312'");
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("JOIN CLUSTER posts AT '10.12.1.35:9312'", Some(true)).await;
```


<!-- end -->

Если включены [аутентификация и авторизация](../../Security/Authentication_and_authorization.md), пользователь, от имени которого выполняется репликация, должен иметь разрешение `replication`. Этого пользователя можно указать в команде `JOIN CLUSTER`:

```sql
GRANT replication ON 'posts' TO 'repl_user';
JOIN CLUSTER posts AT '10.12.1.35:9312' 'repl_user' AS user;
```

На присоединяемом узле и узлах-донорах должен существовать один и тот же пользователь с совпадающими сохранёнными данными аутентификации. Недостаточно независимо создать на каждом узле пользователя с одинаковыми именем и паролем: сохранённые данные аутентификации могут различаться.

Если пользователь не указан, для присоединения используется пользователь текущего сеанса. После успешного присоединения сохранённый пользователь кластера берётся из метаданных кластера-донора.

> ПРИМЕЧАНИЕ: Если аутентификация включена, успешное выполнение `JOIN CLUSTER` заменяет все локальные данные аутентификации на присоединяемом узле данными аутентификации кластера-донора. Если для журналирования аутентификации задан уровень `info` или выше, перед заменой Manticore сохраняет резервную копию прежних локальных данных аутентификации в формате JSON в `searchd.log.auth`. Эта копия содержит соли и хеши учётных данных, поэтому ограничьте доступ к журналу аутентификации и удаляйте конфиденциальные данные перед его передачей другим лицам. Если `JOIN CLUSTER` не удаётся получить данные пользователя с узла-донора, проверьте пользователя репликации и совпадение данных аутентификации на узлах-донорах, а также проверьте `searchd.log.auth` на этих узлах на наличие ошибок аутентификации API.

<!-- example joining a replication cluster 1_1 -->
В большинстве случаев при работе с одним кластером репликации этого достаточно. Однако при создании нескольких кластеров репликации необходимо также задать [путь](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Replication-cluster) и убедиться, что каталог доступен.

<!-- request SQL -->
```sql
JOIN CLUSTER c2 at '127.0.0.1:10201' 'c2' as path
```

<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "JOIN CLUSTER c2 at '127.0.0.1:10201' 'c2' as path"
```
<!-- end -->

Узел присоединяется к кластеру, получая данные с указанного узла. В случае успеха он обновляет списки узлов на всех остальных узлах кластера так же, как при ручном выполнении [ALTER CLUSTER ... UPDATE nodes](../../Creating_a_cluster/Setting_up_replication/Managing_replication_nodes.md). Этот список используется для повторного присоединения узлов к кластеру после перезапуска.

Существует два списка узлов:
1.`cluster_<name>_nodes_set`: используется для повторного присоединения узлов к кластеру после перезапуска. Обновляется на всех узлах так же, как при выполнении [ALTER CLUSTER ... UPDATE nodes](../../Creating_a_cluster/Setting_up_replication/Managing_replication_nodes.md). Команда `JOIN CLUSTER` выполняет это обновление автоматически. В [состоянии кластера](../../Creating_a_cluster/Setting_up_replication/Replication_cluster_status.md) этот список отображается как `cluster_<name>_nodes_set`.
2. `cluster_<name>_nodes_view`:  содержит все активные узлы, участвующие в репликации, и не требует ручного управления. Команда [ALTER CLUSTER ... UPDATE nodes](../../Creating_a_cluster/Setting_up_replication/Managing_replication_nodes.md) копирует этот список в список узлов, используемый для повторного присоединения после перезапуска. В [состоянии кластера](../../Creating_a_cluster/Setting_up_replication/Replication_cluster_status.md) этот список отображается как `cluster_<name>_nodes_view`.

<!-- example joining a replication cluster  2 -->
Если узлы находятся в разных сегментах сети или центрах обработки данных, параметр [nodes](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Replication-cluster) можно задать явно. Это сокращает трафик между узлами и позволяет использовать узлы-шлюзы для связи между центрами обработки данных. Следующий код присоединяет узел к существующему кластеру с помощью параметра [nodes](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Replication-cluster).

> **Примечание:** При использовании этого синтаксиса список `cluster_<name>_nodes_set` кластера не обновляется автоматически. Для его обновления используйте [ALTER CLUSTER ... UPDATE nodes](../../Creating_a_cluster/Setting_up_replication/Managing_replication_nodes.md).


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
JOIN CLUSTER click_query 'clicks_mirror1:9312;clicks_mirror2:9312;clicks_mirror3:9312' as nodes
```

<!-- request JSON -->

```json
POST /cli -d "
JOIN CLUSTER click_query 'clicks_mirror1:9312;clicks_mirror2:9312;clicks_mirror3:9312' as nodes
"
```

<!-- request PHP -->

```php
$params = [
  'cluster' => 'posts',
  'body' => [
      'nodes' => 'clicks_mirror1:9312;clicks_mirror2:9312;clicks_mirror3:9312'
  ]
];
$response = $client->cluster->join($params);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('JOIN CLUSTER click_query \'clicks_mirror1:9312;clicks_mirror2:9312;clicks_mirror3:9312\' as nodes')
```

<!-- response Python -->
```python
{u'error': u'', u'total': 0, u'warning': u''}
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('JOIN CLUSTER click_query \'clicks_mirror1:9312;clicks_mirror2:9312;clicks_mirror3:9312\' as nodes')
```

<!-- response Python-asyncio -->
```python
{u'error': u'', u'total': 0, u'warning': u''}
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('JOIN CLUSTER click_query \'clicks_mirror1:9312;clicks_mirror2:9312;clicks_mirror3:9312\' as nodes');
```

<!-- response javascript -->
```javascript
{"total":0,"error":"","warning":""}
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("JOIN CLUSTER click_query 'clicks_mirror1:9312;clicks_mirror2:9312;clicks_mirror3:9312' as nodes");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("JOIN CLUSTER click_query 'clicks_mirror1:9312;clicks_mirror2:9312;clicks_mirror3:9312' as nodes");
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("JOIN CLUSTER click_query 'clicks_mirror1:9312;clicks_mirror2:9312;clicks_mirror3:9312' as nodes", Some(true)).await;
```

<!-- end -->

Команда `JOIN CLUSTER` выполняется синхронно и завершается, как только узел получит все данные от других узлов кластера и синхронизируется с ними.

Операция `JOIN CLUSTER` может завершиться ошибкой, указывающей на дублирование [server_id](../../Server_settings/Searchd.md#server_id). Это происходит, если `server_id` присоединяемого узла совпадает с `server_id` уже существующего узла кластера. Чтобы устранить проблему, убедитесь, что у каждого узла кластера репликации уникальный [server_id](../../Server_settings/Searchd.md#server_id). Перед попыткой присоединения к кластеру можно заменить значение [server_id](../../Server_settings/Searchd.md#server_id) по умолчанию на уникальное в разделе "searchd" файла конфигурации.

Перед передачей данных каждый узел кластера проверяет доступность присоединяемого узла по его адресу [node_address](../../Server_settings/Searchd.md#node_address) и порту API: именно к ним подключается узел, отправляющий данные. Если какой-либо узел сообщает, что подключиться не удалось, например из-за невозможности разрешить имя присоединяемого узла, `JOIN CLUSTER` завершается с ошибкой этого узла, не затрагивая кластер. Перед окончательным отказом проверка повторяется с увеличивающимися паузами, зависящими от [replication_retry_delay](../../Server_settings/Searchd.md#replication_retry_delay). Узлы, не ответившие на проверку, например из-за перезапуска, пропускаются. Такая же проверка выполняется, когда узел повторно присоединяется к своему кластеру при запуске. Узел из списка узлов кластера, имя которого больше не разрешается, например удалённый контейнер, также пропускается: при присоединении, перезапуске и других операциях с кластером используются остальные узлы, а присоединение не приводит к созданию нового кластера.

Когда присоединяемому узлу нужна полная копия таблиц кластера (SST), Galera выбирает один из синхронизированных узлов в качестве донора, и его `cluster_<name>_node_state` принимает значение `donor`. Донором не обязательно становится узел, указанный в `AT`. Пока донор передаёт файлы таблиц, операции записи в таблицы кластера на доноре ожидают завершения передачи. Они не отклоняются, поэтому клиенты видят задержки и могут столкнуться с истечением собственных тайм-аутов. Записи с других узлов донор также применяет только после передачи. На остальных узлах запись приостанавливается лишь ненадолго, пока присоединяемый узел входит в кластер, а чтение не блокируется ни на одном узле. Время ожидания на доноре растёт с размером таблиц кластера; отслеживать передачу можно с помощью [cluster_<name>_sst_total](../../Creating_a_cluster/Setting_up_replication/Replication_cluster_status.md#SST-Progress-Metrics).

Выбрать донора нельзя, и им может стать узел, на который ваши клиенты отправляют записи. Чтобы запись продолжалась во время присоединения узла, направляйте её на другие узлы, пока какой-либо узел сообщает о состоянии `donor`. Например, настройте проверку работоспособности в балансировщике нагрузки так, чтобы узел в состоянии `donor` считался недоступным для записи, и добавляйте узлы при низкой нагрузке на запись. Если узел перезапускается, пропустив лишь недавние транзакции, он синхронизируется с помощью инкрементальной передачи состояния (IST): файлы таблиц не копируются, а операции записи на доноре не ожидают завершения передачи. IST возможна, пока кеш наборов записей донора (`gcache.size` в [параметрах](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#options) кластера) содержит все транзакции, пропущенные узлом.
<!-- proofread -->
