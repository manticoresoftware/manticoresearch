# 重启集群

在多主复制集群中，必须建立一个参考点，其他节点才能加入并形成集群。这称为集群引导，涉及将单个节点作为`primary component`启动。单个节点的重启或关机后的重新连接可以正常进行。

在完全关闭集群的情况下，应首先启动最后停止的服务器，并使用`--new-cluster`命令行选项或通过systemd运行`manticore_new_cluster`。为了确保服务器能够作为参考点，位于集群[路径](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Replication-cluster)的`grastate.dat`文件中的`safe_to_bootstrap`选项应更新为1。必须满足两个条件，`--new-cluster`和`safe_to_bootstrap=1`。如果启动其他节点时未设置这些选项，将会发生错误。可以使用`--new-cluster-force`命令行选项来覆盖此保护，强制从另一台服务器启动集群。或者，可以运行`manticore_new_cluster --force`来使用systemd。

在集群中所有服务器发生硬崩溃或非正常关闭的情况下，必须识别并启动位于集群[路径](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Replication-cluster)的`grastate.dat`文件中具有最大`seqno`的最先进节点，并使用`--new-cluster-force`命令行参数启动。 

<!-- proofread -->

