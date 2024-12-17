# 集群恢复

在 Manticore 搜索守护进程停止且集群中没有剩余节点处理请求时，需要进行恢复。由于使用 Galera 库进行复制的多主模式，Manticore 复制集群是一个单一的逻辑实体，维护其节点和数据的一致性，以及整个集群的状态。这允许在多个节点上同时安全写入，并确保集群的完整性。

然而，这也带来了挑战。让我们通过一个包含节点 A、B 和 C 的集群来检查几种情况，以了解在某些或所有节点变得不可用时需要采取的措施。

### Case 1

当节点 A 停止时，其他节点会收到“正常关闭”消息。集群大小减少，并进行法定人数重新计算。

当节点 A 启动时，它将加入集群，并且在与集群完全同步之前不会处理任何写事务。如果捐赠节点 B 或 C 上的写缓存（可以通过 Galera 集群的 [gcache.size](https://galeracluster.com/library/documentation/galera-parameters.html#gcache-size) 控制）仍包含节点 A 错过的所有事务，节点 A 将接收快速增量状态转移（[IST](https://galeracluster.com/library/documentation/state-transfer.html#state-transfer-ist)），即仅传输错过的事务。如果没有，则会发生快照状态转移（[SST](https://galeracluster.com/library/documentation/state-transfer.html#state-transfer-sst)），这涉及到表文件的传输。

### Case 2

在节点 A 和 B 停止的情况下，集群大小减少到 1，节点 C 形成主组件以处理写事务。

节点 A 和 B 然后可以像往常一样启动，并将在启动后加入集群。节点 C 作为捐赠者，向节点 A 和 B 提供状态转移。

### Case 3

所有节点都像往常一样停止，集群处于关闭状态。

现在的问题是如何初始化集群。在 searchd 的干净关闭过程中，节点将最近执行的事务数量写入集群目录中的 [grastate.dat](../../Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md) 文件，并带有标志 `safe_to_bootstrap`。最后停止的节点将有选项 `safe_to_bootstrap: 1` 和最先进的 `seqno` 号。

确保这个节点先启动以形成集群。要启动集群，服务器应在该节点上以标志 [--new-cluster](../../Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md) 启动。在 Linux 上，您也可以运行 `manticore_new_cluster`，它将通过 systemd 以 `--new-cluster` 模式启动 Manticore。

如果另一个节点先启动并引导集群，那么最先进的节点将加入该集群，进行完整的 SST，并接收与其之前获得的表文件相比缺少一些事务的表文件。因此，首先启动最后关闭的节点是重要的，它应在 [grastate.dat](../../Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md) 中具有标志 `safe_to_bootstrap: 1`。

### Case 4

如果由于崩溃或网络故障导致节点 A 从集群中消失，节点 B 和 C 将尝试重新连接节点 A。如果失败，它们将从集群中移除节点 A。在剩余的三个节点中，两个仍在运行，集群保持法定人数，并继续正常运行。

当节点 A 重新启动时，它将自动加入集群，如 [Case 1](../../Creating_a_cluster/Setting_up_replication/Cluster_recovery.md#Case-1) 中所述。

### Case 5

节点 A 和 B 已下线。节点 C 无法单独形成法定人数，因为 1 个节点少于总节点数的一半（3）。因此，节点 C 上的集群被转移到非主状态，并拒绝任何写事务，返回错误消息。

与此同时，节点 C 等待其他节点连接并尝试连接它们。如果发生这种情况，网络恢复，节点 A 和 B 重新上线，集群将自动重组。如果节点 A 和 B 仅暂时与节点 C 断开连接，但仍能相互通信，它们将正常运行，因为它们仍然形成法定人数。

<!-- example case 5 -->
然而，如果节点 A 和 B 因为电力故障崩溃或重启，则必须使用以下命令在节点 C 上激活主组件：

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

在执行此命令之前，务必确认其他节点确实无法访问。否则，可能会发生“分脑”场景，导致形成多个独立的集群。

### Case 6

所有节点都崩溃。在这种情况下，集群目录中的 [grastate.dat](../../Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md) 文件未更新，不包含有效的 `seqno` 序列号。

如果发生这种情况，则需要找到最近数据的节点，并在其上使用 [--new-cluster-force](../../Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md) 命令行选项启动服务器。所有其他节点将按照 [Case 3](../../Creating_a_cluster/Setting_up_replication/Cluster_recovery.md#Case-3) 中所述正常启动。在 Linux 上，您还可以使用 `manticore_new_cluster --force` 命令，通过 systemd 启动 Manticore 以 `--new-cluster-force` 模式。

### Case 7

“分脑”可能导致集群转变为非主状态。例如，考虑一个包含偶数节点（四个）的集群，例如位于不同数据中心的两个节点对。如果网络故障中断了数据中心之间的连接，则发生“分脑”，因为每组节点恰好持有法定人数的一半。因此，两组都停止处理写事务，因为 Galera 复制模型优先考虑数据一致性，而集群在没有法定人数的情况下无法接受写事务。然而，两个组中的节点都尝试与其他组中的节点重新连接，以努力恢复集群。

<!-- example case 7 -->
如果有人希望在网络恢复之前恢复集群，则应采取与 [Case 5](../../Creating_a_cluster/Setting_up_replication/Cluster_recovery.md#Case-5) 中所述相同的步骤，但仅在一组节点上执行。

在执行语句后，执行该语句的节点组将能够再次处理写事务。

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

但是，重要的是要注意，如果在两个组上都执行该语句，将导致形成两个独立的集群，随后网络恢复将不会导致两个组重新连接。
<!-- proofread -->