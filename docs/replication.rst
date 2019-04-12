
.. _replication:

Replication
===========

.. warning::
  Please note that this feature is in preview stage. Some functionality may be not yet complete and may suffer changes.
  Read carefully changelogs of future updates to avoid possible breakages.

Manticore search daemon can replicate a write transaction in a percolate index to other nodes in the cluster.
We took advantage of Percona's fork of Galera library which gives the following benefits:

- true multi-master - read and write to any node at any time

- synchronous replication - no slave lag, no data is lost at node crash

- hot standby - no downtime during failover (since there is no failover)

- tightly coupled - all nodes hold the same state. No diverged data between nodes allowed

- automatic node provisioning -  no need to manually back up the database and restore it on a new node

- easy to use and deploy

- detection and automatic eviction of unreliable nodes

- certification based replication

To use replication in the daemon:

- the daemon should be built with replication support (enabled in the `builds Manticore provides <https://manticoresearch.com/downloads/>`_)

- :ref:`data_dir <data_dir>` option should be set in :ref:`searchd <searchd_program_configuration_options>` section of config

- there should be a :ref:`listen <listen>` for replication protocol directive containing an external IP address and it should not be 0.0.0.0 along with ports range defined and these address port range pair should not be same for all daemons on same box

- there should be at least one value of :ref:`listen <listen>` for SphinxAPI protocol directive containing an external IP address and it should not be 0.0.0.0


.. _replication_cluster:

Replication cluster
-------------------

Replication cluster is a set of nodes among which a write transaction gets replicated.
Replication is configured on a per-index basis. One index can be assigned to only
one cluster. There is no restriction to how many indexes a cluster may have. All
transactions such as ``INSERT``, ``REPLACE``, ``DELETE``, ``TRUNCATE`` in any
percolate index belonging to a cluster are replicated to all other nodes in the
cluster. Replication is multi-master, so writes to any particular node or to
multiple nodes simultaneously work equally well.

Replication cluster configuration options are:

.. _cluster_name:

name
~~~~

Specifies cluster name. Should be unique.

.. _cluster_path:

path
~~~~

Data directory for replication write-set cache and incoming indexes from other nodes.
Should be unique among other clusters in the node. Default is :ref:`data_dir <data_dir>`.

.. _cluster_nodes:

nodes
~~~~~

List of pairs of address:port of all nodes in the cluster (comma separated).
API interface of node should be used for nodes option.
It can contain the current node's address too.
This is used to join cluster initially and rejoin the cluster after restart.

.. _cluster_options:

options
~~~~~~~

Options passed directly to Galera replication plugin as described
here `Galera Documentation Parameters <http://galeracluster.com/documentation-webpages/galeraparameters.html>`_


.. _replication_create:

Create cluster
--------------

To create a cluster at least its :ref:`name <cluster_name>` should
be set. In case of a single cluster or if you are creating the first one
:ref:`path <cluster_path>` may be omitted, in this case :ref:`data_dir <data_dir>`
will be used as the cluster path. For all subsequent clusters you need to specify
:ref:`path <cluster_path>` and it should be available. :ref:`nodes <cluster_nodes>`
may be also set to enumerate all nodes in the cluster.

.. code-block:: sql

    CREATE CLUSTER posts
    CREATE CLUSTER click_query '/var/data/click_query/' as path
    CREATE CLUSTER click_query '/var/data/click_query/' as path, 'clicks_mirror1:9351,clicks_mirror2:9351,clicks_mirror3:9351' as nodes

In case cluster created without nodes list first joined node will be saved as nodes list option.

.. _replication_join:

Join cluster
------------

To join an existing cluster :ref:`name <cluster_name>` and :ref:`nodes <cluster_nodes>` should be set.
In case of a single cluster :ref:`path <cluster_path>` might be omitted, :ref:`data_dir <data_dir>`
will be used as the cluster path then. For all subsequent clusters :ref:`path <cluster_path>` need to be set and should be available.

.. code-block:: sql

    JOIN CLUSTER posts '10.12.1.35:9321' as nodes
    JOIN CLUSTER click_query  'clicks_mirror1:9351;clicks_mirror2:9351;clicks_mirror3:9351' as nodes, '/var/data/click_query/' as path



.. _replication_delete:

Delete cluster
--------------

Delete statement removes cluster by :ref:`name <cluster_name>`. The specified cluster
gets removed from all the nodes, but its indexes are left intact and become just
active local non-replicated indexes.

.. code-block:: sql

     DELETE CLUSTER click_query


.. _replication_alter:

Indexes management
----------------------------------------

ALTER statement adds an existing local PQ index to a cluster or forgets about the
index meaning it doesn't remove the index files on the nodes, the index just becomes
an active non-replicated index.

.. code-block:: sql

     ALTER CLUSTER click_query ADD clicks_daily_index
     ALTER CLUSTER posts DROP weekly_index


The node which receives ALTER query sends the index to other nodes in the cluster.
All local indexes with the same name on other cluster's nodes get replaced.

.. _replication_alter_update:

Nodes management
----------------------------------------

ALTER UPDATE nodes statement set list nodes for cluster on each node to value every node
actually sees now

.. code-block:: sql

     ALTER CLUSTER posts UPDATE nodes
	 
For example on cluster creation nodes list was ``10.10.0.1:9312,10.10.1.1:9312`` since that
other nodes also joined the cluster and currently cluster view is
``10.10.0.1:9312,10.10.1.1:9312,10.15.0.1:9312,10.15.0.3:9312``. It might be better to issue this
statement and update nodes list from current cluster view to reach more nodes in cluster on node restart.
Cluster nodes list and current cluster view at node might be inspected at
:ref:`SHOW STATUS <replication_status>` statement.


.. _replication_write:

Write statements
----------------------------------------

All write statements such as ``INSERT``, ``REPLACE``, ``DELETE``, ``TRUNCATE`` that
change content of a cluster's index should follow ``cluster_name:index_name`` naming
format to make sure the change is to be propagated to all replicas in the cluster.
An error will be triggered otherwise.

.. code-block:: sql

     INSERT INTO posts:weekly_index VALUES ( 'iphone case' )
     TRUNCATE RTINDEX click_query:weekly_index

Read statements such as ``CALL PQ`` or ``SELECT``
can use regular index names not prepended with cluster name.

.. code-block:: sql

     SELECT * FROM weekly_index
     CALL PQ('weekly_index', 'document is here')


Insert of a percolate query at multiple nodes of the same cluster at the same time
with auto generated document id may trigger an error as for now id auto generation
takes into account only local index, but the replication guarantees no id conflict.
Retry should work well in most cases, but depends on the insert rate.
However replacing of percolate queries at multiple nodes at same time with document
``id`` auto generated might cause to replace the only query by last finished request.

In future this behavior may be improved by switching to UUID.


.. _replication_status:

Cluster status
----------------------------------------

:ref:`SHOW STATUS <show_status_syntax>` among other information also outputs
cluster status variables. Output format is
``cluster_name_variable_name`` ``variable_value``. Most of them are described in
`Galera Documentation Status <http://galeracluster.com/documentation-webpages/galerastatusvariables.html>`__.
We additionally display:

- cluster_name - :ref:`name <cluster_name>` of the cluster

- node_state - current state of the node: ``closed``, ``destroyed``, ``joining``, ``donor``, ``synced``

- indexes_count - how many indexes are managed by the cluster

- indexes - list of index names managed by the cluster

- nodes_set - list of nodes in cluster defined on cluster create or join

- nodes_view - actual list of nodes in cluster which this node sees



.. code-block:: sql


    mysql> SHOW STATUS;
    +----------------------------+-------------------------------------------------------------------------------------+
    | Counter                    | Value                                                                               |
    +----------------------------+-------------------------------------------------------------------------------------+
    | cluster_name               | post                                                                                |
    | cluster_post_state_uuid    | fba97c45-36df-11e9-a84e-eb09d14b8ea7                                                |
    | cluster_post_conf_id       | 1                                                                                   |
    | cluster_post_status        | primary                                                                             |
    | cluster_post_size          | 5                                                                                   |
    | cluster_post_local_index   | 0                                                                                   |
    | cluster_post_node_state    | synced                                                                              |
    | cluster_post_indexes_count | 2                                                                                   |
    | cluster_post_indexes       | pq1,pq_posts                                                                        |
    | cluster_post_nodes_set     | 10.10.0.1:9312                                                                      |
    | cluster_post_nodes_view    | 10.10.0.1:9312,10.10.0.1:9320:replication,10.10.1.1:9312,10.10.1.1:9320:replication |



.. _replication_set:

Cluster parameters
----------------------------------------

Replication plugin options can be changed using :ref:`SET <set_syntax>` statement:

.. code-block:: sql

     SET CLUSTER click_query GLOBAL 'pc.bootstrap' = 1


.. _replication_restart:

Cluster restart
----------------------------------------

Replication cluster requires a single node of a cluster to be started as a
reference point prior to all the other nodes join it and form a cluster. This is
called cluster bootstrapping and introduces a ``primary component`` before others
see that as a reference point to sync up the data from. Restart of a single node
or reconnecting from a node after a shutdown can be done as usual.

After the whole cluster shutdown the daemon that was stopped last should be started first
with command line key ``--new-cluster``. To make sure that the daemon is able to
start as a reference point file ``grastate.dat`` in cluster :ref:`path <cluster_path>`
should be updated with the value of ``1`` for option ``safe_to_bootstrap``. I.e.
the both conditions should be satisfied: ``--new-cluster`` and ``safe_to_bootstrap=1``.
Attempt to start any other node without these options will trigger an error.
To override this protection and start cluster from another daemon forcibly command line key
``--new-cluster-force`` can be used.

In case of a hard crash or unclean shutdown of all daemons in the cluster you need to
identify the most advanced node with the largest ``seqno`` in ``grastate.dat`` file
in cluster :ref:`path <cluster_path>` and start that daemon with command line
key ``--new-cluster-force``.


.. _replication_diverge:

Cluster with diverged nodes
----------------------------------------

Sometimes replicated nodes can be diverged from each other. The state of all nodes
might turn into ``non-primary`` due to a network split between the nodes, a cluster
crash, or if replication plugin hits an exception when determining the ``primary component``.
Then it's needed to select a node and promote it to be a ``primary component``.

To determine which node needs to be a reference compare the ``last_committed``
cluster status variable value on all the nodes. In case all the daemons are already
running there's no need to start the cluster again. You just need to promote the
most advanced node to be a ``primary component`` with :ref:`SET <set_syntax>` statement:

.. code-block:: sql

     SET CLUSTER posts GLOBAL 'pc.bootstrap=1'


All other nodes will reconnect to the node and resync their data based on this node.
