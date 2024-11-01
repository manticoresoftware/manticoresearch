# 重新启动集群

在多主复制集群中，必须建立一个参考点，以便其他节点可以加入并形成集群。这称为集群引导，涉及将单个节点作为 `primary component` 启动。重新启动单个节点或在关闭后重新连接可以正常进行。

在全集群关闭的情况下，应首先启动最后停止的服务器，使用 `--new-cluster` 命令行选项或通过 systemd 运行 `manticore_new_cluster`。为了确保服务器能够成为参考点，位于集群 [路径](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Replication-cluster) 的 `grastate.dat` 文件应更新为 `safe_to_bootstrap` 选项的值为 1。必须满足这两个条件，即 `--new-cluster` 和 `safe_to_bootstrap=1`。如果启动其他节点而未设置这些选项，将会发生错误。可以使用 `--new-cluster-force` 命令行选项强制从其他服务器启动集群。或者，可以通过 systemd 运行 `manticore_new_cluster --force`。

如果集群中的所有服务器发生严重崩溃或不干净关闭，必须识别出在集群 [路径](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Replication-cluster) 的 `grastate.dat` 文件中具有最大 `seqno` 的最先进节点，并使用 `--new-cluster-force` 命令行选项启动它。

<!-- proofread -->