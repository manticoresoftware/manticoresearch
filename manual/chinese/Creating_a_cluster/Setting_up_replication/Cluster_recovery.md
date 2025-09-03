# 集群恢复

当 Manticore 搜索守护进程停止且群集中没有剩余节点可以处理请求时，就需要进行恢复。由于用于复制的 Galera 库具备多主模式的特性，Manticore 复制集群是一个单一的逻辑实体，维护其节点和数据的一致性，以及整个集群的状态。这使得可以在多个节点上安全地同时写入，并确保集群的完整性。

然而，这也带来了挑战。让我们通过一个由节点 A、B 和 C 组成的集群来检查几种场景，看看当一些或所有节点不可用时需要做些什么。

### 情况 1

当节点 A 停止时，其他节点会收到“正常关机”消息。集群规模减少，进行仲裁重新计算。

节点 A 启动后，会加入集群，并且在完全与集群同步之前不会提供任何写事务服务。如果捐赠节点 B 或 C 上的 writeset 缓存（可通过 Galera 集群的 [gcache.size](https://galeracluster.com/library/documentation/galera-parameters.html#gcache-size) 控制）仍包含节点 A 错过的所有事务，节点 A 会接收快速增量状态传输（[IST](https://galeracluster.com/library/documentation/state-transfer.html#state-transfer-ist)），即仅传输错过的事务。如果没有，则会进行快照状态传输（[SST](https://galeracluster.com/library/documentation/state-transfer.html#state-transfer-sst)），涉及表文件的传输。

### 情况 2

如果节点 A 和 B 停止，集群规模减少到 1，节点 C 形成主组件以处理写事务。

节点 A 和 B 可以照常启动，并在启动后加入集群。节点 C 作为捐赠者，为节点 A 和 B 提供状态传输。

### 情况 3

所有节点均已正常停止，集群关闭。

现在的问题是如何初始化集群。重要的是，在 searchd 的干净关机时，节点会将最后执行的事务编号写入集群目录中的 [grastate.dat](../../Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md) 文件中，并带有标志 `safe_to_bootstrap`。最后停止的节点会有选项 `safe_to_bootstrap: 1` 和最新的 `seqno` 编号。

必须首先启动该节点以形成集群。要引导集群，应在该节点上使用标志 [--new-cluster](../../Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md) 启动服务器。在 Linux 上，也可以运行 `manticore_new_cluster`，通过 systemd 以 `--new-cluster` 模式启动 Manticore。

如果其他节点先启动并引导集群，那么最新的节点加入该集群，执行完整 SST，接收的表文件中会缺少某些之前拥有的事务。因此，重要的是首先启动最后关闭的节点，其 [grastate.dat](../../Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md) 文件中应带有 `safe_to_bootstrap: 1` 标志。

### 情况 4

当发生崩溃或网络故障导致节点 A 从集群中消失时，节点 B 和 C 会尝试重新连接节点 A。连接失败后，它们将从集群中移除节点 A。在三节点中仍有两个节点运行情况下，集群维持仲裁并继续正常运行。

节点 A 重新启动时会自动加入集群，如 [情况 1](../../Creating_a_cluster/Setting_up_replication/Cluster_recovery.md#Case-1) 所述。

### 情况 5

节点 A 和 B 已离线。节点 C 无法单独形成仲裁，因为 1 个节点少于总节点数（3）的一半。因此，节点 C 上的集群切换至非主状态，拒绝任何写事务并返回错误信息。

同时，节点 C 会等待其他节点连接，并尝试连接它们。如果网络恢复且节点 A 和 B 回在线，集群将自动重新形成。如果节点 A 和 B 只是暂时与节点 C 断开，但它们之间仍可通信，则它们将继续正常运行，因为它们仍形成了仲裁。

<!-- example case 5 -->
不过，如果节点 A 和 B 因断电等原因崩溃或重启，则必须有人使用以下命令激活节点 C 上的主组件：

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

执行此命令前，务必确认其他节点确实无法访问。否则可能发生脑裂（split-brain）情况，形成分裂的集群。

### 情况 6

所有节点均已崩溃。在这种情况下，集群目录中的 [grastate.dat](../../Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md) 文件未被更新，且不包含有效的 `seqno` 序列号。

如发生此情况，需要找到数据最为近期的节点，并使用命令行参数 [--new-cluster-force](../../Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md) 启动该节点上的服务器。其余节点将照常启动，如 [情况 3](../../Creating_a_cluster/Setting_up_replication/Cluster_recovery.md#Case-3) 中描述。
在 Linux 上，也可以使用命令 `manticore_new_cluster --force`，通过 systemd 以 `--new-cluster-force` 模式启动 Manticore。

### 情况 7

Split-brain 可能导致集群转变为非主状态。例如，考虑一个由偶数个节点（四个）组成的集群，比如位于不同数据中心的两对节点。如果网络故障中断了数据中心之间的连接，就会发生 split-brain，因为每个节点组恰好拥有一半的法定人数。结果，由于 Galera 复制模型优先考虑数据一致性，且集群在没有法定人数的情况下无法接受写事务，两个组都会停止处理写事务。然而，这两个组中的节点都会尝试重新连接另一个组的节点，以恢复集群。

<!-- example case 7 -->
如果有人想在网络恢复之前恢复集群，应按照[案例5](../../Creating_a_cluster/Setting_up_replication/Cluster_recovery.md#Case-5)中概述的相同步骤操作，但只针对其中一组节点进行操作。

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

但需要注意的是，如果该语句在两个组中都执行，将导致形成两个独立的集群，之后的网络恢复也不会使这两个组重新加入。
<!-- proofread -->

