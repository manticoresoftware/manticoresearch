# 重启集群

当整个多节点复制集群都已停止时，必须先启动其中一个节点，以便其他节点知道应加入集群的哪个副本。

有一种例外情况可以正常启动：如果集群已经干净地变成单节点集群，例如其他节点通过 `EXIT CLUSTER` 离开后，正常启动剩余节点即可。干净关闭后，Manticore 会将这个仅包含自身的集群恢复为 `primary` / `synced`，无需使用 `--new-cluster`。

首次启动时的决策依据是 `grastate.dat`，这是存储在集群数据目录中的小型复制状态文件。最重要的字段包括：

- `seqno` - 该节点已知的最后一个事务编号
- `safe_to_bootstrap` - 该节点是否被标记为可在干净关闭后首先启动

干净关闭后，`grastate.dat` 可能如下所示：

```text
# saved replication state
version: 2.1
uuid:    <cluster-uuid>
seqno:   12345
safe_to_bootstrap: 1
```

在此示例中：

- `seqno: 12345` 表示该节点已知序列号不超过 12345 的事务
- `safe_to_bootstrap: 1` 表示该节点被标记为可首先启动

如果整个集群都已干净关闭，请启动最后停止的节点。实际上，这通常是具有以下特征的节点：

- `seqno` 最大
- `safe_to_bootstrap: 1`

先启动该节点。这会告诉 Manticore 从该节点创建集群的新副本。然后正常启动其余节点，使它们重新加入集群。

<!-- example restart-cluster-clean -->
整个集群干净关闭后，请使用此方法。


<!-- intro -->
##### 命令行：
如果直接启动 Manticore，请使用：

<!-- request Bash -->

```bash
searchd --new-cluster
```

<!-- intro -->
##### systemd：
如果在 Linux 上通过 systemd 运行 Manticore，请使用：

<!-- request Systemd -->

```bash
manticore_new_cluster
```
<!-- end -->

如果其他节点在没有所需干净关闭状态的情况下先启动，Manticore 会拒绝启动，以防止集群从较旧的副本恢复。

### 如果集群意外停止

不要假设最后停止的节点拥有最新数据，也不要在任意节点上强制重启。请按照[集群恢复](../../Creating_a_cluster/Setting_up_replication/Cluster_recovery.md#All-nodes-crashed)中的说明，先确定正确的节点。

<!-- proofread -->
