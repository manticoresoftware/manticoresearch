# Понимание статуса кластера и состояний узлов

## Краткая справка

Статус кластера показывает два ключевых значения, которые помогают понять состояние кластера:

- `cluster_%_status` - может ли кластер принимать записи
- `cluster_%_node_state` - что делает этот конкретный узел

## Значения статусов

<!-- example cluster status values -->

<!-- intro -->
##### SQL:

<!-- request SQL -->
```sql
SHOW STATUS LIKE 'cluster_%_status';
SHOW STATUS LIKE 'cluster_%_node_state';
```

<!-- response SQL-->
```sql
+----------------------------+-------+
| Counter                    | Value |
+----------------------------+-------+
| cluster_posts_status       | primary |
| cluster_posts_node_state   | synced |
+----------------------------+-------+
```

<!-- end -->

### cluster_%_status

- **`primary`** - нормальная работа, можно читать и записывать
- **`non-primary`** - потерян кворум, **запись заблокирована**
- **`disconnected`** - узел изолирован, пытается переподключиться

### cluster_%_node_state

- **`synced`** - нормальная работа
- **`joining`** - узел присоединяется к кластеру (временно)
- **`donor`** - помогает другому узлу присоединиться (временно)
- **`closed`** - узел остановлен
- **`destroyed`** - узел упал, требуется перезапуск

## Типичные сценарии

| Что произошло | cluster_%_status | cluster_%_node_state | Что делать |
|---------------|------------------|---------------------|------------|
| Нормальная работа | `primary` | `synced` | Ничего |
| Большинство узлов недоступно | `non-primary` | `synced` | Инициализировать заново или ждать |
| Узел запускается | `disconnected` | `joining` → `synced` | Ждать |
| Узел упал | varies | `destroyed` | Перезапустить узел |
| Сетевая изоляция (большинство) | `primary` | `synced` | Продолжать работу как обычно |
| Сетевая изоляция (меньшинство) | `non-primary` | `synced` | Ждать или инициализировать заново |

## Восстановление после потери кворума

<!-- example quorum recovery -->
Когда большинство узлов недоступно и статус показывает `non-primary`:

<!-- intro -->
##### SQL:

<!-- request SQL -->
```sql
-- Check which node has highest sequence number
SHOW STATUS LIKE 'cluster_%_last_committed';
-- Bootstrap the cluster (run on best node only)
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
SET CLUSTER posts GLOBAL 'pc.bootstrap' = 1
"
```

<!-- end -->

<!-- proofread -->
