# 集群恢复

当 Manticore 搜索守护进程停止且集群中没有剩余节点可提供服务时，必须进行恢复。由于用于复制的 Galera 库具有多主特性，Manticore 复制集群是一个单一的逻辑实体，它维护其节点和数据的一致性，以及整个集群的状态。这允许在多个节点上同时安全写入，确保集群的完整性。

然而，这也带来了挑战。让我们以由节点 A、B 和 C 组成的集群为例，看看当部分或全部节点不可用时需要做些什么。

### 情况 1

当节点 A 停止时，其他节点会收到“正常关闭”消息。集群规模缩小，并进行仲裁数重新计算。

启动节点 A 时，它会加入集群，但在与集群完全同步之前不会处理任何写事务。如果捐赠者节点 B 或 C 上的写集缓存（可通过 Galera 集群的 [gcache.size](https://galeracluster.com/library/documentation/galera-parameters.html#gcache-size) 控制）仍包含节点 A 错过的所有事务，节点 A 将接收一次快速增量状态转移（[IST](https://galeracluster.com/library/documentation/state-transfer.html#state-transfer-ist)），即仅传输错过的事务。如果没有，将进行快照状态转移（[SST](https://galeracluster.com/library/documentation/state-transfer.html#state-transfer-sst)），涉及转移表文件。

### 情况 2

在节点 A 和 B 都停止的场景下，集群规模减少到 1，由节点 C 形成主要组件处理写事务。

然后，节点 A 和 B 可以照常启动，启动后将加入集群。节点 C 作为捐赠者，为节点 A 和 B 提供状态转移。

### 情况 3

所有节点都已关闭，集群处于关闭状态。

现在的问题是如何初始化集群。重要的是，在 searchd 的干净关闭过程中，节点会将最后执行的事务编号写入集群目录中的 [grastate.dat](../../Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md) 文件，并带有 `safe_to_bootstrap` 标志。最后停止的节点会有选项 `safe_to_bootstrap: 1` 和最先进的 `seqno` 编号。

必须先启动这个节点以形成集群。要启动集群，应该使用 [--new-cluster](../../Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md) 标志在该节点上启动服务器。在 Linux 上，也可以运行 `manticore_new_cluster`，它将通过 systemd 以 `--new-cluster` 模式启动 Manticore。

如果其他节点先启动并自举集群，那么最先进的节点会加入该集群，执行完全 SST 并接收一个缺失部分事务的表文件，这与之前它所拥有的表文件不同。这就是为什么必须先启动最后关闭的节点，它在 [grastate.dat](../../Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md) 中应有 `safe_to_bootstrap: 1` 标志。

### 情况 4

发生崩溃或网络故障导致节点 A 从集群消失时，节点 B 和 C 会尝试重新连接节点 A。失败后，它们会将节点 A 从集群中移除。由于三节点中的两个仍在运行，集群保持仲裁数，继续正常运行。

当节点 A 重启时，它将自动加入集群，如[情况 1](../../Creating_a_cluster/Setting_up_replication/Cluster_recovery.md#Case-1)所述。

### 情况 5

节点 A 和 B 离线。节点 C 无法自行形成仲裁数，因为 1 个节点小于全部 3 个节点的一半。因此，节点 C 上的集群状态变为非主要状态，拒绝任何写事务并显示错误消息。

同时，节点 C 等待其他节点连接并尝试连接它们。如果发生这种情况，网络恢复，节点 A 和 B 重新上线，集群将自动重组。如果节点 A 和 B 只是临时断开与节点 C 的连接，但它们仍能相互通信，则它们将继续正常运行，因为它们仍然形成了仲裁数。

<!-- example case 5 -->
但如果节点 A 和 B 都因断电而崩溃或重启，则必须有人使用以下命令在节点 C 上激活主组件：

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

重要的是，在执行该命令之前，必须确认其他节点确实无法访问。否则，可能会发生脑裂，形成分离的集群。

### 情况 6

所有节点崩溃。在这种情况下，集群目录中的 [grastate.dat](../../Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md) 文件未更新，且不包含有效的 `seqno` 序列号。

如果发生这种情况，需要有人找到数据最新的节点，并使用命令行参数 [--new-cluster-force](../../Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md) 启动该节点上的服务器。所有其他节点将按正常方式启动，如[情况 3](../../Creating_a_cluster/Setting_up_replication/Cluster_recovery.md#Case-3)所述。
在 Linux 上，也可以使用 `manticore_new_cluster --force` 命令，通过 systemd 以 `--new-cluster-force` 模式启动 Manticore。

### 情况 7

脑裂可能导致集群转变为非主状态。例如，考虑一个由偶数个节点（四个）组成的集群，比如位于不同数据中心的两对节点。如果网络故障中断了数据中心之间的连接，脑裂就会发生，因为每组节点恰好持有半数仲裁。结果，两组节点都停止处理写事务，因为Galera复制模型优先考虑数据一致性，没有仲裁的集群无法接受写事务。然而，两组中的节点都会尝试与另一组的节点重新连接，以努力恢复集群。

<!-- example case 7 -->
如果有人在网络恢复之前想要恢复集群，应采取[案例5](../../Creating_a_cluster/Setting_up_replication/Cluster_recovery.md#Case-5)中概述的相同步骤，但仅在其中一组节点上执行。

执行该语句后，运行该语句的节点所在组将能够再次处理写事务。


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

然而，需要注意的是，如果在两组节点上都执行该语句，将导致形成两个独立的集群，随后的网络恢复也不会使两组重新合并。
<!-- proofread -->

