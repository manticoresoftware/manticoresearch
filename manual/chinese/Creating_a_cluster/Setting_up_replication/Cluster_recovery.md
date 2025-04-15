# 集群恢复

如果 Manticore 搜索守护进程停止且集群中没有剩余节点可以处理请求，则需要进行恢复。由于用于复制的 Galera 库的多主特性，Manticore 复制集群是一个单一逻辑实体，维护其节点和数据的连贯性，以及整个集群的状态。这允许在多个节点上安全地同时写入，并确保集群的完整性。

然而，这也带来了挑战。让我们通过使用节点 A、B 和 C 的集群来检查几种情况，以了解在某些或所有节点不可用时需要做些什么。

### 情况 1

 当节点 A 停止时，其他节点收到“正常关闭”的消息。集群大小减少，并进行法定人数重新计算。
 
启动节点 A 后，它将加入集群，并在完全与集群同步之前不会处理任何写事务。如果捐赠节点 B 或 C 的写集缓存（可以通过 Galera 集群的 [gcache.size](https://galeracluster.com/library/documentation/galera-parameters.html#gcache-size) 控制）仍包含节点 A 错过的所有事务，节点 A 将接收到快速增量状态传输（[IST](https://galeracluster.com/library/documentation/state-transfer.html#state-transfer-ist)），即只传输错过的事务。如果没有，则会发生快照状态传输（[SST](https://galeracluster.com/library/documentation/state-transfer.html#state-transfer-sst)），涉及表文件的传输。

### 情况 2

在节点 A 和 B 停止的情况下，集群大小减少到 1，节点 C 形成主要组件以处理写事务。

节点 A 和 B 可以像往常一样启动，并将在启动后加入集群。节点 C 作为捐赠者，为节点 A 和 B 提供状态传输。

### 情况 3

所有节点按常规停止，集群关闭。

现在的问题是如何初始化集群。重要的是，在 searchd 的干净关闭下，节点将最后执行事务的数量写入集群目录 [grastate.dat](../../Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md) 文件，并带有标志 `safe_to_bootstrap`。最后停止的节点将具有选项 `safe_to_bootstrap: 1` 和最先进的 `seqno` 数字。

重要的是此节点首先启动以形成集群。要引导集群，服务器应在此节点上以标志 [--new-cluster](../../Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md) 启动。在 Linux 上，您还可以运行 `manticore_new_cluster`，它将通过 systemd 以 `--new-cluster` 模式启动 Manticore。

如果其他节点先启动并引导集群，那么最先进的节点将加入该集群，进行完整的 SST，并接收与之前获得的表文件相比缺失某些事务的表文件。这就是为什么重要的是首先启动最后关闭的节点，因为它在 [grastate.dat](../../Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md) 中应该具有标志 `safe_to_bootstrap: 1`。

### 情况 4

如果发生崩溃或网络故障导致节点 A 从集群中消失，节点 B 和 C 将尝试与节点 A 重新连接。失败后，它们将从集群中移除节点 A。在三节点中的两个节点仍在运行时，集群保持其法定人数并继续正常运行。

当节点 A 重启时，它将自动加入集群，如 [情况 1](../../Creating_a_cluster/Setting_up_replication/Cluster_recovery.md#Case-1) 中所述。

### 情况 5

节点 A 和 B 已离线。节点 C 无法单独形成法定人数，因为 1 个节点少于总节点的二分之一（3）。因此，节点 C 上的集群转移到非主状态，并以错误消息拒绝任何写事务。

与此同时，节点 C 等待其他节点连接并尝试与它们连接。如果发生这种情况，网络恢复且节点 A 和 B 恢复在线，集群将自动重组。如果节点 A 和 B 只是暂时与节点 C 断开连接，但仍可以相互通信，则它们将继续正常运行，因为它们仍形成法定人数。

<!-- example case 5 -->
然而，如果节点 A 和 B 都因电源故障崩溃或重启，则必须使用以下命令在节点 C 上激活主组件：

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

重要的是，在执行此命令之前，您必须确认其他节点确实无法访问。否则，可能会发生脑裂场景，并可能形成独立的集群。

### 情况 6

所有节点均已崩溃。在这种情况下，集群目录中的 [grastate.dat](../../Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md) 文件未更新，并且不包含有效的 `seqno` 序列号。

如果发生这种情况，某人需要找到最近数据的节点，并使用 [--new-cluster-force](../../Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md) 命令行键在其上启动服务器。所有其他节点将像 [情况 3](../../Creating_a_cluster/Setting_up_replication/Cluster_recovery.md#Case-3)) 中描述的那样正常启动。
在 Linux 上，您还可以使用 `manticore_new_cluster --force` 命令，它将通过 systemd 以 `--new-cluster-force` 模式启动 Manticore。

### 情况 7

分裂脑可能导致集群转换为非主状态。例如，考虑一个由偶数个节点（四个）组成的集群，这些节点位于不同的数据中心。如果网络故障中断了数据中心之间的连接，分裂脑就会发生，因为每个节点组恰好持有一半的法定人数。因此，两个组都停止处理写入事务，因为Galera复制模型优先考虑数据一致性，并且没有法定人数的集群无法接受写入事务。然而，两个组中的节点都会尝试重新连接另一组的节点，以努力恢复集群。

<!-- example case 7 -->
如果有人想在网络恢复之前恢复集群，应采取与[Case 5](../../Creating_a_cluster/Setting_up_replication/Cluster_recovery.md#Case-5)中概述的相同步骤，但仅在一个节点组上执行。

执行语句后，运行该语句的节点所在的组将再次能够处理写入事务。


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

然而，需要注意的是，如果在两个组上都发出该语句，将导致形成两个独立的集群，后续的网络恢复不会导致这些组重新加入。
<!-- proofread -->
