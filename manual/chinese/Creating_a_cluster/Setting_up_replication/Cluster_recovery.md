# 集群恢复

如果某个复制节点或整个集群不可用，正确的恢复流程取决于仍可访问的节点数量，以及停机是正常还是突然发生的。

应将复制集群视为一个逻辑系统，而不是一组互不相关的服务器。这样可以支持多主写入和一致数据，但也意味着必须谨慎恢复仲裁。尤其是在确认缺失节点确实已经离线之前，不要运行用于恢复幸存节点写入能力的手动恢复命令。本页后面会展示该命令：`SET CLUSTER <name> GLOBAL 'pc.bootstrap' = 1`。如果过早运行，可能造成脑裂，最终形成两个相互独立的集群。

以下示例均假设集群包含节点 A、B 和 C，除非另有说明。

## 开始恢复前

首先确定你遇到的是哪种情况：

- 是否至少有一个节点仍处于在线状态？
- 节点是正常停止，还是发生了崩溃？正常停止意味着 `searchd` 已正常关闭，并在退出前有时间保存复制状态。崩溃、断电或执行 `kill -9` 都不属于正常停止。
- 集群的幸存部分是否仍拥有仲裁？仲裁意味着仍有足够的节点可以相互发现，从而安全地保持为可写集群。
- 如果所有节点都已停止，应先启动哪个节点来恢复集群？

有用的检查：

- `SHOW STATUS LIKE 'cluster_<name>_status'`
- `SHOW STATUS LIKE 'cluster_<name>_size'`
- `SHOW STATUS LIKE 'cluster_<name>_node_state'`
- 如果所有节点都已停止，请检查 `grastate.dat`。这是存储在集群数据目录中的小型复制状态文件。重点查看 `seqno` 和 `safe_to_bootstrap`：正常停止时，通常应首先启动具有最大 `seqno` 且 `safe_to_bootstrap: 1` 的节点。完整的引导流程请参阅[重启集群](../../Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md)。

正常停止后，`grastate.dat` 可能如下所示：

```text
# saved replication state
version: 2.1
uuid:    <cluster-uuid>
seqno:   12345
safe_to_bootstrap: 1
```

在此示例中：

- `seqno: 12345` 表示该节点已知的事务最多达到序列号 12345
- `safe_to_bootstrap: 1` 表示该节点已标记为可首先启动

在所有节点正常停止后的恢复过程中，通常应先使用 `--new-cluster` 启动这类节点，以恢复集群。

恢复后，等待重启节点报告 `cluster_<name>_status=primary` 和 `cluster_<name>_node_state=synced`，再将其视为完全可写。可以使用 `SHOW STATUS LIKE 'cluster_<name>_status'` 和 `SHOW STATUS LIKE 'cluster_<name>_node_state'` 进行检查。在本地测试中，重启节点有时会短暂处于 `cluster_<name>_node_state=joining` 和 `cluster_<name>_status=disconnected` 状态，之后才达到 `synced`/`primary`。

### 一个节点正常关闭

如果节点 A 正常停止，节点 B 和 C 会继续提供写入服务。可以在这两个节点上使用 `SHOW STATUS LIKE 'cluster_<name>_status'` 和 `SHOW STATUS LIKE 'cluster_<name>_size'`，确认集群仍处于健康状态。

节点 A 再次启动时，会自动重新加入集群。在同步完成之前，不要向该节点发送写入请求。检查 `SHOW STATUS LIKE 'cluster_<name>_status'` 和 `SHOW STATUS LIKE 'cluster_<name>_node_state'`，并等待其变为 `primary` / `synced`。

如果供体节点 B 或 C 的复制缓存中仍保留节点 A 遗漏的全部事务，节点 A 可以通过增量状态传输（IST）追赶进度。IST 是增量状态传输的缩写，表示节点只接收遗漏的事务，因此恢复通常更快、开销更低。否则就需要执行快照状态传输（SST）。SST 是快照状态传输的缩写，表示从另一个节点复制表文件，而不是仅重放缺失的事务。SST 的开销更大：通常速度更慢、传输的数据更多，并且在大型集群中可能使恢复过程造成更大的干扰。

### 两个节点正常关闭，一个节点保持在线

如果节点 A 和 B 正常停止，而节点 C 仍在线，节点 C 可以继续接受写入。可以在节点 C 上检查 `SHOW STATUS LIKE 'cluster_<name>_status'` 和 `SHOW STATUS LIKE 'cluster_<name>_size'`，确认它现在是唯一的活动节点。

节点 A 和 B 再次启动时，会自动重新加入集群，并从节点 C 同步。它们重新加入期间，请检查 `SHOW STATUS LIKE 'cluster_<name>_status'`、`SHOW STATUS LIKE 'cluster_<name>_node_state'` 和 `SHOW STATUS LIKE 'cluster_<name>_size'`。等待所有节点显示 `primary` / `synced`，且集群大小符合预期后，再认为恢复完成。

如果节点 A 和 B 已被有意从集群中移除，例如使用了 `EXIT CLUSTER`，并且节点 C 现在是集群中唯一持久化保存的节点，那么节点 C 在自身正常重启后也可以通过正常启动守护进程来恢复。在这种仅剩自身的情况下，不需要使用 `--new-cluster`。

### 所有节点正常关闭

如果多节点集群中的所有节点都正常停止，集群将完全离线，必须以特殊方式重新启动，使其中一个节点成为集群中第一个运行的节点。

正常停止时，每个节点都会将最后一个事务编号写入 `grastate.dat`。最后停止的节点最适合首先启动：

- 它具有最先进的 `seqno`
- 它具有 `safe_to_bootstrap: 1`

使用 [`--new-cluster`](../../Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md) 启动该节点。这会让 Manticore 从该节点启动集群的新副本。如果在 Linux 上通过 systemd 运行 Manticore，请使用 `manticore_new_cluster`。它会自动以 `--new-cluster` 模式启动 Manticore。

之后正常启动其余节点，让它们重新加入。使用 `SHOW STATUS LIKE 'cluster_<name>_status'`、`SHOW STATUS LIKE 'cluster_<name>_node_state'` 和 `SHOW STATUS LIKE 'cluster_<name>_size'` 验证恢复情况。

如果先引导一个状态较旧的节点，状态较新的节点稍后可能加入该节点，并从较旧状态接收完整的 SST，从而丢弃仅存在于状态较新节点上的事务。因此，应优先选择具有 `safe_to_bootstrap: 1` 的节点。

### 使用现有本地表重新创建集群

当没有节点能够通过上述正常恢复方法恢复集群，但集群表仍存在于本地数据目录中时，可以将此流程作为最后手段。这可能发生在保存的集群元数据不完整或与正在运行的版本不兼容时，也可能发生在其他启动失败导致旧集群成员关系无法使用时。

此流程会创建一个新集群。它会丢弃旧的集群成员关系和复制历史，然后将选定节点上的表复制到其他节点。旧集群中只要仍有节点拥有健康的 `primary` / `synced` 副本，就不要使用此流程。在这种情况下，应保留健康集群，并通过 [JOIN CLUSTER](../../Creating_a_cluster/Setting_up_replication/Joining_a_replication_cluster.md#Joining-a-replication-cluster) 恢复其他节点。

1. 停止属于失败集群的每个节点上的 Manticore。在此流程期间不要写入本地表。
2. 备份每个节点上的完整 `data_dir`。如果旧集群描述符包含重新创建集群时需要的自定义 `path`、`nodes` 或提供程序 `options`，请保留一份副本。
3. 选择本地表中包含你要保留的数据的节点。该节点将成为新集群的数据源。稍后将其表添加到集群时，会替换其他节点上同名的表。
4. 在每个节点上编辑 `<data_dir>/manticore.json`。从顶层 `clusters` 对象中仅删除失败集群对应的条目。例如，对于名为 `posts` 的集群，删除 `clusters.posts`。

   保留 `indexes` 对象和每个表目录。不要删除或重新创建这些表。

5. 在每个节点上正常启动 Manticore。原集群的表现在应显示为本地表。继续操作前，请检查选定源节点上的表。如果表缺失或不完整，请停止操作并恢复备份。
6. 在选定的源节点上创建集群。如果仍需要，请加入已保存的 `path`、`nodes` 或提供程序选项。如果启用了身份验证，请在此语句和下面的 `JOIN CLUSTER` 语句中添加 `'<replication-user>' AS user`。每个节点上的账户都必须具有匹配的已存储身份验证数据和 `replication` 权限。请参阅[设置复制](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Setting-up-replication)。

   ```sql
   CREATE CLUSTER posts
   ```

7. 在其他每个节点上，通过第一个节点加入新集群：

   ```sql
   JOIN CLUSTER posts AT 'node-a.example:9312'
   ```

8. 在选定的源节点上，将现有本地表附加到新集群：

   ```sql
   ALTER CLUSTER posts ADD table_a, table_b
   ```

9. 等待每个节点报告 `cluster_posts_status=primary` 和 `cluster_posts_node_state=synced`，再恢复写入。

### 一个节点崩溃或无法访问

如果节点 A 因崩溃或网络问题消失，节点 B 和 C 会先尝试重新连接它。如果失败，它们会将该节点从集群中移除，重新计算仲裁，并继续作为主集群运行。

在本地测试中，移除对等节点并不是立即完成的：幸存节点会在几秒内保持旧的集群大小，之后才删除失败节点并切换到规模更小的主集群。

节点 A 再次启动时，会自动重新加入集群，并以与单节点正常关闭后相同的方式追赶进度。同样，请使用 `SHOW STATUS LIKE 'cluster_<name>_status'`、`SHOW STATUS LIKE 'cluster_<name>_node_state'` 和 `SHOW STATUS LIKE 'cluster_<name>_size'`，确认恢复已完成。

### 两个节点丢失，仅一个节点仍在运行

如果节点 A 和 B 丢失，只剩节点 C 仍在运行，那么节点 C 在三节点集群中将不再拥有仲裁。它会切换到 `non-primary` 状态并拒绝写入。

写入错误信息会明确说明这一点：

```sql
ERROR 1064 (42000): cluster '<name>' is not ready, not primary state (synced)
```

如果节点 A 和 B 只是暂时断开连接，但彼此仍能发现对方，那么它们可能继续接受写入，而节点 C 保持隔离。如果需要查看哪一侧仍可写，可以在两侧分别运行 `SHOW STATUS LIKE 'cluster_<name>_status'`。

如果节点 A 和 B 确实已崩溃，而节点 C 是你希望继续使用的唯一幸存副本，请在节点 C 上运行以下命令，使其恢复可写：

<!-- example bootstrap-primary -->
如果已确认其他节点确实处于离线状态，请运行：

<!-- intro -->
##### SQL：

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

- 只有在确认其他节点无法访问后才能运行此命令
- 只能在必须保留的一侧运行此命令
- 引导后，该节点可以重新接受写入，其他节点之后可以从该节点重新加入

### 所有节点均崩溃

如果整个集群意外停止，**不要随意引导某个节点**：这样可能丢失最近的事务。如果仍有节点处于 `primary` / `synced` 状态并继续运行，请改为遵循[单节点恢复](../../Creating_a_cluster/Setting_up_replication/Cluster_recovery.md#One-node-crashed-or-became-unreachable)。

1. 保持所有节点停止，禁用自动重启，并备份每个节点的 `data_dir`。
2. 在每个节点上找到此集群的 `grastate.dat`（位于 `data_dir` 下，也可能位于特定于集群的子目录中）：

   ```bash
   sudo find /var/lib/manticore -name grastate.dat -type f
   ```

   如果你的 `data_dir` 不同，请替换默认路径。
3. 比较来自同一集群历史的各节点之间的 `uuid` 和 `seqno`。崩溃后可能出现 `seqno: -1` 和 `safe_to_bootstrap: 0`；这两个值**无法确定最新数据所在的节点**。如果无法确定哪个节点拥有最新状态，请在强制重启**之前**检查日志和数据，或寻求帮助。
4. 确定最新节点后，仅使用 `manticore_new_cluster --force`（systemd）或 `searchd --new-cluster-force`（直接启动）引导该节点。正常启动其他节点。在恢复写入前，确认状态为 `primary` / `synced`，且集群大小符合预期。

`--new-cluster-force` 会绕过正常的安全检查。对于完整的正常停机，请改为遵循[重启集群](../../Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md)。

### 偶数规模的集群失去仲裁，例如发生脑裂后

偶数规模集群发生脑裂的风险最高。例如，假设四个节点分裂成两个数据中心中的两对互相隔离的节点。每一侧都恰好拥有原始成员的一半，因此两侧都没有仲裁，并且都会停止接受写入。

如果必须在连接恢复之前恢复写入，请只选择脑裂的一侧，并在该侧运行相同的恢复命令，使其重新变为可写。在此之前，请在两侧检查 `SHOW STATUS LIKE 'cluster_<name>_status'`，以确定当前哪一侧处于 `non-primary` 状态。

<!-- example bootstrap-primary-split -->
选择应保留为可写集群的一侧，然后运行：

<!-- intro -->
##### SQL：

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

绝不要在两侧都执行该语句。否则会创建两个独立的主集群，网络恢复后它们不会自动合并。

在本地测试中，四节点集群突然丢失一半节点时，会重现相同的 `non-primary` 行为；执行相同的恢复命令后，其中一个幸存部分会恢复为 `primary`。
<!-- proofread -->
