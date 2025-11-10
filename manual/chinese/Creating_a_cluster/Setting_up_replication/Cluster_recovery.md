# 集群恢复

当 Manticore 搜索守护进程停止且集群中没有剩余节点可提供服务时，需要进行恢复。由于用于复制的 Galera 库具有多主特性，Manticore 复制集群是一个单一的逻辑实体，维护其节点和数据的一致性，以及整个集群的状态。这允许多个节点同时安全写入，并确保集群的完整性。

然而，这也带来了挑战。让我们通过使用节点 A、B 和 C 的集群来检查几种场景，看看当部分或全部节点不可用时需要做什么。

### 情况 1

当节点 A 停止时，其他节点会收到“正常关闭”消息。集群规模减少，并重新计算法定人数。

启动节点 A 后，它会加入集群，并且在完全与集群同步之前不会处理任何写事务。如果捐赠节点 B 或 C 上的写集缓存（可以通过 Galera 集群的 [gcache.size](https://galeracluster.com/library/documentation/galera-parameters.html#gcache-size) 控制）仍包含节点 A 错过的所有事务，节点 A 将接收快速增量状态传输（[IST](https://galeracluster.com/library/documentation/state-transfer.html#state-transfer-ist)），即仅传输错过的事务。如果没有，则会进行快照状态传输（[SST](https://galeracluster.com/library/documentation/state-transfer.html#state-transfer-sst)），涉及表文件的传输。

### 情况 2

在节点 A 和 B 停止的情况下，集群规模减少到一个，节点 C 形成主组件以处理写事务。

然后，节点 A 和 B 可以像往常一样启动，并在启动后加入集群。节点 C 作为捐赠者，向节点 A 和 B 提供状态传输。

### 情况 3

所有节点都像往常一样停止，集群关闭。

现在的问题是如何初始化集群。重要的是，在 searchd 的干净关闭时，节点会将最后执行的事务编号写入集群目录中的 [grastate.dat](../../Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md) 文件，并带有标志 `safe_to_bootstrap`。最后停止的节点将具有选项 `safe_to_bootstrap: 1` 和最先进的 `seqno` 编号。

重要的是该节点应首先启动以形成集群。要引导集群，应在该节点上使用标志 [--new-cluster](../../Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md) 启动服务器。在 Linux 上，你也可以运行 `manticore_new_cluster`，它将通过 systemd 以 `--new-cluster` 模式启动 Manticore。

如果另一个节点先启动并引导集群，则最先进的节点加入该集群，执行完整 SST，并接收一个表文件，其中某些事务与之前获得的表文件相比缺失。这就是为什么重要的是先启动最后关闭的节点，该节点在 [grastate.dat](../../Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md) 中应有标志 `safe_to_bootstrap: 1`。

### 情况 4

在发生崩溃或网络故障导致节点 A 从集群中消失的情况下，节点 B 和 C 会尝试重新连接节点 A。连接失败后，它们会将节点 A 从集群中移除。由于三个节点中仍有两个节点运行，集群保持法定人数并继续正常运行。

当节点 A 重启时，它将自动加入集群，如 [情况 1](../../Creating_a_cluster/Setting_up_replication/Cluster_recovery.md#Case-1) 所述。

### 情况 5

节点 A 和 B 已离线。节点 C 无法单独形成法定人数，因为 1 个节点少于总节点数（3）的一半。因此，节点 C 上的集群被转移到非主状态，并拒绝任何写事务，返回错误消息。

同时，节点 C 等待其他节点连接，并尝试连接它们。如果发生这种情况，网络恢复且节点 A 和 B 重新上线，集群将自动重组。如果节点 A 和 B 只是暂时与节点 C 断开连接，但仍能相互通信，它们将继续正常运行，因为它们仍然形成法定人数。

<!-- example case 5 -->
但是，如果节点 A 和 B 都因断电崩溃或重启，则必须有人使用以下命令在节点 C 上激活主组件：

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

重要的是，在执行此命令之前，必须确认其他节点确实无法访问。否则，可能发生脑裂（split-brain）场景，形成分离的集群。

### 情况 6

所有节点都崩溃。在这种情况下，集群目录中的 [grastate.dat](../../Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md) 文件未更新，且不包含有效的 `seqno` 序列号。

如果发生这种情况，需要有人找到数据最新的节点，并使用命令行参数 [--new-cluster-force](../../Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md) 启动该节点上的服务器。所有其他节点将按正常方式启动，如 [情况 3](../../Creating_a_cluster/Setting_up_replication/Cluster_recovery.md#Case-3) 所述。
在 Linux 上，你也可以使用 `manticore_new_cluster --force` 命令，它将通过 systemd 以 `--new-cluster-force` 模式启动 Manticore。

### 情况 7

Split-brain 可能导致集群转变为非主状态。例如，考虑一个由偶数个节点（四个）组成的集群，比如位于不同数据中心的两对节点。如果网络故障中断了数据中心之间的连接，就会发生 split-brain，因为每组节点恰好持有半数的法定人数。因此，两个组都停止处理写事务，因为 Galera 复制模型优先保证数据一致性，且集群在没有法定人数的情况下无法接受写事务。然而，两个组的节点都会尝试重新连接另一组的节点，以恢复集群。

<!-- example case 7 -->
如果有人想在网络恢复之前恢复集群，应按照[案例 5](../../Creating_a_cluster/Setting_up_replication/Cluster_recovery.md#Case-5)中概述的相同步骤操作，但只针对其中一组节点。

执行该语句后，运行该语句的节点所在的组将能够再次处理写事务。


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

但是，需要注意的是，如果该语句在两个组都执行，将导致形成两个独立的集群，随后网络恢复时这两个组不会重新合并。
<!-- proofread -->

