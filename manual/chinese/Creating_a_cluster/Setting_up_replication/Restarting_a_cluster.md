# 重启集群

当整个复制集群宕机时，必须首先启动一个节点，以便其他节点知道要加入哪个集群副本。

该首次启动决策基于 `grastate.dat`，这是一个存储在集群数据目录中的小型复制状态文件。最重要的字段是：

- `seqno` - 该节点已知的最后一个事务编号
- `safe_to_bootstrap` - 该节点是否被标记为在干净关闭后可以安全地首先启动

干净关闭后 `grastate.dat` 的示例内容：

```text
# saved replication state
version: 2.1
uuid:    <cluster-uuid>
seqno:   12345
safe_to_bootstrap: 1
```

在此示例中：

- `seqno: 12345` 表示该节点知道的事务序列号最多到 12345
- `safe_to_bootstrap: 1` 表示该节点被标记为可以安全地首先启动

如果整个集群已干净关闭，请启动最后停止的节点。实际上，这通常是具有：

- 最先进的 `seqno`
- `safe_to_bootstrap: 1`

的节点。首先启动该节点。这会告诉 Manticore 从该节点启动一个新的集群副本。之后，正常启动其余节点，以便它们可以重新加入。

<!-- example restart-cluster-clean -->
在干净的完整集群关闭后使用此方法。


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

如果其他节点在没有必要干净关闭状态的情况下首先启动，启动将被拒绝，以保护集群不从旧副本恢复。

如果所有节点崩溃或未干净关闭，`grastate.dat` 可能不再适合用于正常引导选择。在这种情况下，找到具有最新数据的节点，通常是具有最大 `seqno` 的节点，并使用 `--new-cluster-force` 启动它。这会覆盖正常保护，强制集群从选定节点启动。

<!-- example restart-cluster-force -->
在崩溃或未干净的完整集群关闭后使用此方法。


<!-- intro -->
##### 命令行：
如果直接启动 Manticore，请使用：

<!-- request Bash -->

```bash
searchd --new-cluster-force
```

<!-- intro -->

##### systemd：
如果在 Linux 上通过 systemd 运行 Manticore，请使用：

<!-- request Systemd -->

```bash
manticore_new_cluster --force
```
<!-- end -->

<!-- proofread -->
