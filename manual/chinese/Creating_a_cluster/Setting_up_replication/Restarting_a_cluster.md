# 重启集群

在多主复制集群中，必须先建立一个参考点，其他节点才能加入并形成集群。这称为集群引导，涉及将单个节点启动为 `primary component`。单个节点的重启或关闭后的重新连接可以正常进行。

如果是整个集群的完全关闭，应先启动最后停止的服务器，并使用 `--new-cluster` 命令行选项或通过 systemd 运行 `manticore_new_cluster`。为了确保服务器能够成为参考点，集群 [路径](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Replication-cluster) 下的 `grastate.dat` 文件中，`safe_to_bootstrap` 选项应更新为值 1。`--new-cluster` 和 `safe_to_bootstrap=1` 两个条件都必须满足。如果在未设置这些选项的情况下启动其他节点，会报错。可以使用 `--new-cluster-force` 命令行选项来强制覆盖此保护，并从另一台服务器启动集群。或者，可以通过 systemd 运行 `manticore_new_cluster --force`。

如果所有服务器发生严重崩溃或不干净关闭，必须识别出 `grastate.dat` 文件中集群 [路径](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Replication-cluster) 下具有最大 `seqno` 的最先进节点，并使用 `--new-cluster-force` 命令行选项启动该节点。

<!-- proofread -->

