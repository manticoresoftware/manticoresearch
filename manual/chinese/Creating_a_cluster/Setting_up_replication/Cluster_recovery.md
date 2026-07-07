# 集群恢复

如果复制节点或整个集群变得不可用，正确的恢复程序取决于仍有多少节点可以访问以及关闭是干净的还是突然的。

复制集群应被视为一个逻辑系统，而不是一组无关的服务器。这为您提供了多主写入和一致的数据，但也意味着您必须谨慎恢复法定人数。特别是，在确定缺失的节点确实已消失之前，不要运行恢复存活侧写入的手动恢复命令。该命令在本文后面显示为 `SET CLUSTER <name> GLOBAL 'pc.bootstrap' = 1`。如果过早运行它，可能会导致脑裂，并最终形成两个独立的集群。

在下面的示例中，除非另有说明，否则假设一个包含节点 A、B 和 C 的集群。

## 在恢复任何内容之前

首先确定您处于哪种情况：

- 至少有一个节点仍在在线吗？
- 节点是干净停止，还是发生了崩溃？干净停止意味着 `searchd` 正常关闭并在退出前有时间保存其复制状态。崩溃、断电或 `kill -9` 不是干净停止。
- 集群的存活部分是否仍具有法定人数？法定人数意味着足够多的节点仍能相互看到，以安全地保持可写集群。
- 如果所有节点都已关闭，应首先启动哪个节点以恢复集群？

有用的检查：

- `SHOW STATUS LIKE 'cluster_<name>_status'`
- `SHOW STATUS LIKE 'cluster_<name>_size'`
- `SHOW STATUS LIKE 'cluster_<name>_node_state'`
- 如果所有节点都已关闭，请检查 `grastate.dat`，这是存储在集群数据目录中的小型复制状态文件。特别注意 `seqno` 和 `safe_to_bootstrap`：在干净关闭后，通常首选启动的节点是具有最先进 `seqno` 和 `safe_to_bootstrap: 1` 的节点。有关完整的引导程序，请参阅 [重新启动集群](../../Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md)。

干净关闭后 `grastate.dat` 可能的样子示例：

```text
# saved replication state
version: 2.1
uuid:    <cluster-uuid>
seqno:   12345
safe_to_bootstrap: 1
```

在此示例中：

- `seqno: 12345` 表示此节点知道到序列号 12345 的事务
- `safe_to_bootstrap: 1` 表示此节点被标记为可以首先启动

在干净关闭的所有节点都关闭的恢复中，这通常是您使用 `--new-cluster` 首先启动的节点类型，以恢复集群。

恢复后，等待重新启动的节点报告 `cluster_<name>_status=primary` 和 `cluster_<name>_node_state=synced` 后，再将其视为完全可写。您可以使用 `SHOW STATUS LIKE 'cluster_<name>_status'` 和 `SHOW STATUS LIKE 'cluster_<name>_node_state'` 检查这一点。在本地测试中，重新启动的节点有时会在达到 `synced`/`primary` 之前短暂显示 `cluster_<name>_node_state=joining` 和 `cluster_<name>_status=disconnected`。

### 一个节点被干净关闭

如果节点 A 正常关闭，节点 B 和 C 会继续处理写入。您可以使用 `SHOW STATUS LIKE 'cluster_<name>_status'` 和 `SHOW STATUS LIKE 'cluster_<name>_size'` 确认这些节点上的集群仍然健康。

当节点 A 重新启动时，它会自动重新加入集群。在同步完成之前，不要向该节点发送写入。检查 `SHOW STATUS LIKE 'cluster_<name>_status'` 和 `SHOW STATUS LIKE 'cluster_<name>_node_state'`，并等待 `primary` / `synced`。

如果供体节点 B 或 C 仍然在它们的复制缓存中拥有节点 A 错过的所有事务，节点 A 可以使用增量状态传输 (IST) 进行追赶。IST 代表增量状态传输。这意味着节点仅接收它错过的事务，因此恢复通常更快且更轻量。否则，它将需要快照状态传输 (SST)。SST 代表快照状态传输。这意味着从另一个节点复制表文件，而不是仅仅重放缺失的事务。SST 更重：通常更慢，移动更多数据，并且可能在大型集群上使恢复更具破坏性。

### 两个节点被干净关闭，一个节点保持在线

如果节点 A 和 B 被干净关闭，节点 C 保持在线，节点 C 可以继续接受写入。如果您想确认它现在是唯一活动的节点，请在节点 C 上检查 `SHOW STATUS LIKE 'cluster_<name>_status'` 和 `SHOW STATUS LIKE 'cluster_<name>_size'`。

当节点 A 和 B 重新启动时，它们会自动重新加入并从节点 C 同步。在它们重新加入时，检查 `SHOW STATUS LIKE 'cluster_<name>_status'`、`SHOW STATUS LIKE 'cluster_<name>_node_state'` 和 `SHOW STATUS LIKE 'cluster_<name>_size'`。等待所有节点显示 `primary` / `synced` 并且集群大小符合预期后，再将恢复视为完成。

如果节点 A 和 B 是有意从集群中移除的，例如使用 `EXIT CLUSTER`，而节点 C 现在是集群中唯一仍然保留的节点，那么节点 C 在执行自身的干净重启后，也可以通过正常的守护进程启动恢复。在这种仅剩自身的情况下，不需要 `--new-cluster`。

### 所有节点都被干净关闭

如果多节点集群中的所有节点都已正常停止，那么集群就处于完全离线状态，必须以特殊方式重新启动，这样其中一个节点才能成为集群中第一个运行的节点。

在干净关闭时，每个节点将其最后一个事务号写入 `grastate.dat`。最后关闭的节点是首先启动的最安全节点：

- 它具有最先进 `seqno`
- 它具有 `safe_to_bootstrap: 1`

使用 [`--new-cluster`](../../Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md) 启动该节点。这会告诉 Manticore 从此节点开始一个新的集群副本。如果您通过 Linux 上的 systemd 运行 Manticore，请使用 `manticore_new_cluster`。它会为您以 `--new-cluster` 模式启动 Manticore。

之后，正常启动其余节点并让它们重新加入。通过 `SHOW STATUS LIKE 'cluster_<name>_status'`、`SHOW STATUS LIKE 'cluster_<name>_node_state'` 和 `SHOW STATUS LIKE 'cluster_<name>_size'` 验证恢复。

如果您首先引导一个较不先进的节点，一个更先进的节点可能会稍后加入它并从较旧状态接收完整的 SST，这可能会丢弃仅存在于更先进节点上的事务。这就是为什么 `safe_to_bootstrap: 1` 的节点应是您的首选。

### 一个节点崩溃或变得不可达

如果节点 A 因崩溃或网络问题消失，节点 B 和 C 会首先尝试重新连接到它。如果失败，它们会将其从集群中移除，重新计算法定人数，并继续作为主集群运行。

在本地测试中，对等节点的移除并非立即发生：存活的节点会在几秒钟内保持旧的集群大小，之后才会丢弃失败的对等节点并切换到较小的主集群。

当节点 A 重新启动时，它会自动重新加入，并以与干净的一节点关闭后相同的方式进行同步。同样，使用 `SHOW STATUS LIKE 'cluster_<name>_status'`、`SHOW STATUS LIKE 'cluster_<name>_node_state'` 和 `SHOW STATUS LIKE 'cluster_<name>_size'` 来确认恢复已完成。

### 有两个节点消失，只剩一个节点仍在运行

如果节点 A 和 B 都丢失，只剩节点 C 在运行，那么在三个节点的集群中，节点 C 将不再拥有法定人数。它会切换到 `non-primary` 并拒绝写入。

写入错误是明确的：

```sql
ERROR 1064 (42000): cluster '<name>' is not ready, not primary state (synced)
```

如果节点 A 和 B 仅临时断开连接但仍能互相看到，它们可能会继续接受写入，而节点 C 保持隔离。如果需要查看哪一侧仍可写入，请在每侧使用 `SHOW STATUS LIKE 'cluster_<name>_status'`。

如果节点 A 和 B 确实崩溃，且节点 C 是唯一希望继续使用的存活副本，请在节点 C 上运行此命令，使其重新变为可写入状态：

<!-- example bootstrap-primary -->
如果您已确认其他节点确实离线，请运行：

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

重要提示：

- 仅在确认其他节点无法到达后运行此命令
- 仅在必须存活的一侧运行
- 引导后，该节点可以再次接受写入，其他节点之后可以从它重新加入

### 所有节点崩溃

如果所有节点都崩溃，`grastate.dat` 通常不再适合用于正常引导选择。在本地测试中，所有节点显示：

- `seqno: -1`
- `safe_to_bootstrap: 0`

在这种情况下，选择数据最新的节点，并使用[`--new-cluster-force`](../../Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md)启动它。这会强制Manticore从该节点启动一个新的集群副本，即使通常的干净关闭元数据不可靠。如果你在Linux上通过systemd运行Manticore，请使用`manticore_new_cluster --force`。它会为你以`--new-cluster-force`模式启动Manticore。

然后正常启动其余节点并让它们重新加入。使用 `SHOW STATUS LIKE 'cluster_<name>_status'`、`SHOW STATUS LIKE 'cluster_<name>_node_state'` 和 `SHOW STATUS LIKE 'cluster_<name>_size'` 验证恢复。

### 偶数大小的集群失去法定人数，例如在分裂后

偶数大小的集群最容易出现脑裂风险。例如，想象四个节点分裂成两个隔离的对，分布在两个数据中心。每边恰好有原始成员的一半，因此两边都没有法定人数，两边都停止接受写入。

如果在连接恢复前必须恢复写入，请仅选择分裂的一侧，并在该侧运行相同的恢复命令，使该侧重新变为可写入。在执行此操作之前，请在两侧检查 `SHOW STATUS LIKE 'cluster_<name>_status'`，以确定哪一侧当前为非主集群。

<!-- example bootstrap-primary-split -->
选择应保持为可写集群的一侧，然后运行：

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

永远不要在两侧都执行该语句。如果这样做，您将创建两个独立的主集群，当网络恢复时它们不会自动合并。

在本地测试中，突然失去四节点集群的一半会重现相同的 `non-primary` 行为，相同的恢复命令使一个存活的一半重新变为 `primary`。
<!-- proofread -->
