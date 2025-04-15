# 重启集群

在多主复制集群中，必须建立一个参考点，其他节点才能加入并形成集群。这被称为集群引导，并涉及启动一个节点作为 `primary component`。正常情况下可以重启单个节点或在关闭后重新连接。

如果是完全的集群关闭，最后停止的服务器应首先使用 `--new-cluster` 命令行选项或通过运行 `manticore_new_cluster` 通过 systemd 启动。为了确保服务器能够作为参考点，位于集群 [路径](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Replication-cluster) 的 `grastate.dat` 文件应更新 `safe_to_bootstrap` 选项的值为 1。必须满足这两个条件，`--new-cluster` 和 `safe_to_bootstrap=1`。如果没有设置这些选项启动其他节点，将会发生错误。可以使用 `--new-cluster-force` 命令行选项来覆盖此保护，并强制从其他服务器启动集群。或者，您可以运行 `manticore_new_cluster --force` 使用 systemd。

在集群中所有服务器发生硬崩溃或不干净关闭的情况下，必须识别 `grastate.dat` 文件中具有最大 `seqno` 的最高节点，并使用 `--new-cluster-force` 命令行选项启动。

<!-- proofread -->
