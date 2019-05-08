How to recover a cluster
========================

There are cases of multiple possible scenarios where searchd service got stopped with no node in cluster being able to serve requests.
In these cases someone need to recover cluster or part of it. Due to multi-master nature of Galera library used for replication,
cluster is like one logical entity and takes care about each node and its data consistency and keeps cluster status as a whole.
This allows safe writes on multiple nodes at the same time and maintain cluster integrity compare to traditional asynchronous replication.

There is considered a cluster of nodes A, B and C and scenarios where some or all nodes are out of service and what to do to bring them back.


.. _replication_diverge_case1:

Case 1
----------

Node A is stopped as usual. The other nodes receive "normal shutdown" message from node A. The cluster size is reduced and issue quorum re-calculation.

After node A got started as usual, it joins the cluster nodes. The node A during join will not serve any write transaction
until it fully synchronized with cluster. If writeset cache on donor node B or C, that is Galera option of cluster `gcache.size`,
still has all transactions missed at node A, node A will receive fast incremental state transfer (`IST`), that is transfer of only missed transactions.
Otherwise snapshot state transfer (`SST`) will start, that is transfer of index files.


.. _replication_diverge_case2:

Case 2
----------

Nodes A and B is stopped as usual. That is same as previous case but cluster size is reduced to 1 and node C itself forms primary component allows it to handle write transactions.

Nodes A and B might be started as usual and will join the cluster. The node C becomes "donor" and provides state transfer to nodes A and B.


.. _replication_diverge_case3:

Case 3
----------

All nodes are stopped as usual and cluster is over.

The problem is how to initialize cluster and it is important that on clean shutdown of searchd node writes last executed transaction number
into the cluster directory `grastate.dat` file along with `safe_to_bootstrap` key. The node stopped last will have `safe_to_bootstrap: 1` and most advanced `seqno` number.

It is important that this node should start first and form cluster. To bootstrap cluster daemon should be started on this node with `--new-cluster` command line key.

If another node starts first and bootstraps cluster then most advanced node joins that cluster, performs full SST and receives index file
with some transactions missed compare to index files it got. That is why it is important to start node that shutdown last
or look at cluster directory `grastate.dat` to find node with `safe_to_bootstrap: 1`


.. _replication_diverge_case4:

Case 4
----------

Node A disappears from the cluster due to crash or network failure.

The nodes B and C try to reconnect missed node A and after failure remove node A from the cluster. Cluster quorum is valid as 2 out of 3 nodes are running and cluster works as usual.

After node A restarts it will join cluster automatically the same way as in :ref:`case 1 <replication_diverge_case1>`.


.. _replication_diverge_case5:

Case 5
----------

Nodes A and B disappear. The node C is not able to form the quorum alone as 1 node is less than half of 3. Then cluster on node C is switch to non-primary state
and node C rejects any write transactions with error.

That single node C is waiting for other nodes to connect and try to connect them as well. If this happens, like network outage passed and nodes A and B are running all the time,
the cluster will be formed again automatically. If the nodes A and B are just cut from node C, but they can still reach each other, they keep work as usual as they still form the quorum.

However if both nodes A and B crashed or restarted due to power outage, someone should turn on primary component on the C node with following statement:

.. code-block:: sql

     SET CLUSTER posts GLOBAL 'pc.bootstrap' = 1


However someone make sure the all other nodes are really unreachable before doing that, otherwise split-brain happens and separate clusters got formed.


.. _replication_diverge_case6:

Case 6
----------

All nodes crashed. In that case the `grastate.dat` file at cluster directory is not updated and does not contain valid sequence number `seqno`.

In this case someone should find most advanced node and start deemon there with `--new-cluster-force` command line key.
All other nodes should start as usual as in :ref:`case 3 <replication_diverge_case3>`.


.. _replication_diverge_case7:

Case 7
----------

Split brain cause cluster to get into non-primary state. For example cluster made from even number of nodes (four) and each two nodes in separate datacenter
and network failure happens between these datacenters.

Split-brain happens as each nodes group has exactly half of quorum. Both groups stop to handle write transactions as Galera replication model cares about data consistency
and cluster without quorum can not accept write transactions. But nodes at both groups try to re-connect nodes from other group to restore cluster.

If someone want to restore cluster without network got restored the same steps as in :ref:`case 5 <replication_diverge_case5>` could be used but only at one group of nodes

.. code-block:: sql

     SET CLUSTER posts GLOBAL 'pc.bootstrap' = 1

After this statement this one group could handle write transactions well.

However want to notice if this statement got issued at both groups it will end up with two separate clusters and restore of network will not make groups to rejoin.
