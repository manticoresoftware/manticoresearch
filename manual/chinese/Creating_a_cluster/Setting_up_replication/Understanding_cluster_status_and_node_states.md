# 理解集群状态和节点状态

本节提供有关在集群状态输出中出现的 `cluster_%_status` 和 `cluster_%_node_state` 值的详细说明，包括在不同操作场景下每种组合的含义。

## cluster_%_status 值

`cluster_%_status` 变量指示从该节点的角度看集群组件的状态。理解这些状态对于正确的集群管理和故障排除至关重要。

### PRIMARY 状态

<!-- example primary status -->
当集群状态显示为 `PRIMARY` 时，集群具有法定人数，可以接受读写操作。这是正常的操作状态。

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

`PRIMARY` 状态意味着：
* 集群中的大多数节点已连接并且正在通信
* 允许进行写操作，并将被复制
* 集群正常运行
* 保持法定人数

### NON_PRIMARY 状态

<!-- example non_primary status -->
当集群状态显示为 `NON_PRIMARY` 时，集群已失去法定人数。这是一个关键状态，写操作被阻止以防止脑裂场景的发生。

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

`NON_PRIMARY` 状态发生在：
* 集群中的超过一半的节点不可达
* 网络分区已将集群拆分
* 当前节点组无法形成多数

**关键**：在此状态下，写操作被拒绝。可能需要手动干预，使用引导命令：

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

### DISCONNECTED 状态

<!-- example disconnected status -->
当集群状态显示为 `DISCONNECTED` 时，该节点未连接到任何集群组件，并正试图重新连接。

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

`DISCONNECTED` 状态通常发生在：
* 节点启动之前，尚未加入集群
* 网络连接问题
* 集群形成问题
* 暂时与其他集群成员隔离

## cluster_%_node_state 值

`cluster_%_node_state` 变量显示该特定节点在集群中的当前操作状态。

### synced 状态

<!-- example synced state -->
当节点状态显示为 `synced` 时，节点已完全同步并处于操作状态。这是正常的工作状态。

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

`synced` 状态意味着：
* 节点可以同时处理读和写请求
* 所有事务都正在正常复制
* 节点正在为集群法定人数做贡献
* 数据与其他集群成员一致

### joining 状态

<!-- example joining state -->
当节点状态显示为 `joining` 时，节点正在加入集群并接收状态同步。

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

在 `joining` 状态期间：
* 节点正在接收集群状态 (IST 或 SST)
* 节点尚无法处理读或写请求
* 状态传输正在进行中
* 完成后，节点将转变为 `synced`

### donor 状态

<!-- example donor state -->
当节点状态显示为 `donor` 时，节点正在向加入的节点提供状态传输。

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

在 `donor` 状态期间：
* 节点继续处理请求，但可能会出现延迟
* 正在向另一节点提供状态传输 (SST)
* 传输完成后节点返回 `synced`
* 性能可能会暂时受到影响

### closed 状态

<!-- example closed state -->
当节点状态显示为 `closed` 时，节点已关闭或尚未启动。

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

`closed` 状态表示：
* 节点已干净地关闭
* 节点尚未启动
* 这是加入集群前的初始状态
* 在计划维护期间的正常状态

### destroyed 状态

<!-- example destroyed state -->
当节点状态显示为 `destroyed` 时，节点遇到错误并异常关闭。

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

`destroyed` 状态表示：
* 发生了严重问题，需要调查
* 节点可能需要手动干预以重新启动
* 可能存在数据损坏或系统故障
* 在重新启动之前查看日志以获取错误详情

## 不同场景下的状态组合

### 正常操作

| 场景 | cluster_%_status | cluster_%_node_state | 描述 |
|----------|------------------|---------------------|-------------|
| 健康的集群 | `primary` | `synced` | 正常操作，可以读/写 |
| 节点启动中 | `disconnected` → `primary` | `closed` → `joining` → `synced` | 正常启动序列 |
| 新节点加入 | `primary` | `donor` (帮助节点), `joining` (新节点) | SST 正在进行中 |

### 网络问题

| 场景 | cluster_%_status | cluster_%_node_state | 需要采取的行动 |
|----------|------------------|---------------------|-----------------|
| 少数分区 (5 个节点中的 2 个) | `non-primary` | `synced` | 等待网络或引导 |
| 多数分区 (5 个节点中的 3 个) | `primary` | `synced` | 继续正常操作 |
| 完全隔离 | `disconnected` | `synced` | 检查网络连接 |

### 节点故障

| 场景 | cluster_%_status | cluster_%_node_state | 影响 |
|----------|------------------|---------------------|--------|
| 单个节点故障 (5 节点集群) | `primary` | `synced` | 集群正常继续 |
| 超过半数节点宕机 (3+ 节点宕机) | `non-primary` | `synced` | 不允许写入 |
| 节点宕机 | 视情况而定 | `destroyed` | 节点需要重启/恢复 |

### 关键故障场景

#### 大多数节点宕机（丧失法定人数）

<!-- example quorum lost scenario -->
当集群中的大多数节点不可用时，剩余节点将显示 `NON_PRIMARY` 状态并拒绝写操作。

在一个5节点集群中，3个及以上节点失败时：

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

要从该场景恢复，识别 `last_committed` 值最高的节点并引导启动它：

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

#### 网络分区（防止脑裂）

<!-- example network partition -->
在网络分区场景中，只有拥有多数的分区保持 `PRIMARY` 状态：

**多数分区（5节点集群中3个节点）：**

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

**少数分区（5节点集群中2个节点）：**

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

### 常见事件中的状态转换

#### 新节点加入（成功）
```
New Node: disconnected/closed → disconnected/joining → primary/synced
Donor Node: primary/synced → primary/donor → primary/synced
Other Nodes: primary/synced (unchanged)
```

#### 网络分区恢复
```
Minority Partition: non-primary/synced → primary/synced (when reconnected)
Majority Partition: primary/synced (unchanged throughout)
```

#### 整个集群完全丧失后的手动恢复
```
First node: disconnected/closed → primary/synced (after pc.bootstrap=1)
Other nodes: disconnected/closed → primary/joining → primary/synced
```

### 监控集群健康状态

<!-- example monitoring cluster health -->
为了监控集群健康状态并识别问题，请检查这些关键状态变量：

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
查看配置节点与可见节点的差异：

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

这显示一个节点（10.10.2.1:9312）已配置但当前对集群不可见，表明可能存在连接问题。

## 常见问题排查

### 写操作被拒绝

<!-- example write rejected -->
当集群状态为 `NON_PRIMARY` 时，写操作将被拒绝：

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

**解决方案**：检查集群状态，等待法定人数恢复或如果节点永久不可用则手动引导启动。

### 节点卡在 joining 状态

<!-- example stuck joining -->
如果节点长时间处于 `joining` 状态：

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

这可能表示：
* SST 期间网络带宽问题
* 大量数据集传输中
* 捐献节点性能问题

检查捐献节点状态及节点间的网络连接。

### 集群规模不匹配

<!-- example size mismatch -->
当 `cluster_%_size` 与预期节点数不符时：

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

这表明一些配置的节点目前无法访问。检查缺失节点的网络连接和节点状态。

<!-- proofread -->
