# Restarting a cluster

In a multi-master replication cluster, a reference point must be established before other nodes can join and form the cluster. This is called cluster bootstrapping and involves starting a single node as the `primary component`. Restarting a single node or reconnecting after a shutdown can be done normally.

In case of a full cluster shutdown, the server that was stopped last should be started first with the `--new-cluster` command line option or by running `manticore_new_cluster`   through systemd. To ensure that the server is capable of being the reference point, the `grastate.dat` file located at the cluster [path](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Replication-cluster) should be updated with a value of 1 for the `safe_to_bootstrap` option. Both conditions, `--new-cluster` and `safe_to_bootstrap=1`, must be met. If any other node is started without these options set, an error will occur. The `--new-cluster-force` command line option can be used to override this protection and start the cluster from another server forcibly. Alternatively, you can run `manticore_new_cluster --force` to use systemd.

In the event of a hard crash or an unclean shutdown of all servers in the cluster, the most advanced node with the largest `seqno` in the `grastate.dat` file located at the cluster [path](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Replication-cluster) must be identified and started with the `--new-cluster-force` command line key.

<!-- proofread -->