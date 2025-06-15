# 重启集群

在多主复制集群中，必须先建立一个参考点，其他节点才能加入并形成集群。这称为集群引导，涉及启动单个节点作为`primary component`。单个节点的重启或关机后的重新连接可以正常进行。

在完全关闭集群的情况下，最后停止的服务器应首先启动，并带有`--new-cluster`命令行选项，或者通过 systemd 运行`manticore_new_cluster`。为了确保该服务器能够作为参考点，位于集群[路径](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Replication-cluster)的`grastate.dat`文件中的`safe_to_bootstrap`选项应更新为1。必须满足两个条件，即`--new-cluster`和`safe_to_bootstrap=1`。如果其他节点启动时未设置这些选项，将会出现错误。可以使用`--new-cluster-force`命令行选项来强制覆盖此保护，从另一台服务器强制启动集群。或者，也可以运行`manticore_new_cluster --force`来使用 systemd。

如果集群中所有服务器发生硬崩溃或不干净关机，则必须识别`grastate.dat`文件中`seqno`最大的、最先进的节点（位于集群[路径](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Replication-cluster)），并使用`--new-cluster-force`命令行键启动该节点。

<!-- proofread -->

