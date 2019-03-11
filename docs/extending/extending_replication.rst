
.. _repliation:

Replication
----------------------------------------

Manticoresearch daemon could replicate a write transaction to percolate index among the nodes in cluster.
Daemon uses Galera library from Percona fork with following benefits:

- true multi-master - cluster read and write to any node at any time

- synchronized replication - no data is lost at node crash

- hot standby and no failover and no downtime during failover

- tightly coupled - all nodes at cluster hold the same state and no diverged data between nodes allowed

- automatic node provisioning - no need to manually back up the indexes and move that data into the new node

- easy to use and deploy

- detection and automatic eviction of unreliable nodes

- certification based replication method

.. warning::
   Please note that this feature is in preview stage. Some functionalities are not yet complete and may suffer changes.  
   Read careful changelog of future updates to avoid possible breakages.


To use replication daemon with replication plugin should be installed from release packages or build from source
as described at :ref:`Compiling Manticore from source <compiling_from_source>` . :ref:`data_dir <data_dir>` option
should be set in :ref:`searchd <searchd>` section of config. At least one :ref:`listen <listen>` at daemon
should have external IP address and not `0.0.0.0`.


.. _repliation_cluster:

Cluster
----------------------------------------

Cluster is a nodes list where write transaction got replicated and an indexes list transactions to which are replicated.
All transaction such as ``INSERT``, ``REPLACE``, ``DELETE``, ``TRUNCATE`` to any percolate index in cluster replicated to
all other nodes in cluster to correspond index. As replication is multi-master write to any node or to multiple
nodes simultaneous works well.

Cluster configured with:

.. _cluster_path:

path
~~~~

Data directory there replication plugin stores write set cache and incoming indexes from other nodes.
Should be unique among the other clusters at node. Default is :ref:`data_dir <data_dir>`

.. _cluster_listen:

listen
~~~~~~

This setting lets you specify IP address and port that replication plugin will listen on. Should be external IP
address and not `0.0.0.0`. Should be unique among the other clusters at node. Replication plugin also binds to
next port for incremental state transfer communications.

.. _cluster_uri:

uri
~~~

This is address and port of another node in cluster or list of nodes in cluster with semicolon delimiter.
Might include own address as well as it get blacklisted to prevent loop to self.
Node uses this setting to join cluster initially or after daemon restarts to rejoin cluster.

.. _cluster_options:

options
~~~~~~~

Options passed directly to replication plugin and described at `Galera Documentation Parameters <http://galeracluster.com/documentation-webpages/galeraparameters.html>`__


.. _repliation_create:

Create cluster
----------------------------------------

To create cluster cluster name and :ref:`listen <cluster_listen>` should be set. First cluster or only one cluster at daemon 
:ref:`path <cluster_path>` might be omitted, in this case :ref:`data_dir <data_dir>` will be used for cluster path.
For all subsequent clusters :ref:`path <cluster_path>` should be set and should be available.
:ref:`uri <cluster_uri>` might be also set to enumerate all nodes in cluster.

.. code-block:: sql


    CREATE CLUSTER posts '10.12.1.35:9321' as listen
	CREATE CLUSTER click_query 'clicks_mirror1:9351' as listen, '/var/data/click_query/' as path


.. _repliation_join:

Join cluster
----------------------------------------

To join existed cluster name, :ref:`listen <cluster_listen>`, :ref:`uri <cluster_uri>` should be set.
First cluster or only one cluster at daemon :ref:`path <cluster_path>` might be omitted, in this case :ref:`data_dir <data_dir>`
will be used for cluster path. For all subsequent clusters :ref:`path <cluster_path>` should be set and should be available.

.. code-block:: sql


    JOIN CLUSTER posts '10.12.1.36:9321' as listen, '10.12.1.35:9321' as uri
    JOIN CLUSTER click_query 'clicks_mirror2:9351' as listen, 'clicks_mirror1:9351;clicks_mirror2:9351;clicks_mirror3:9351' as uri, '/var/data/click_query/' as path



.. _repliation_delete:

Delete cluster
----------------------------------------

Delete statement removes cluster by name. It deletes cluster at each node
but keep cluster indexes active locals at daemon.

.. code-block:: sql


     DELETE CLUSTER click_query


.. _repliation_alter:

Cluster indexes management
----------------------------------------

ALTER statement adds existed local index to cluster or remove index from cluster but keep it active at daemon.


.. code-block:: sql


     ALTER CLUSTER click_query ADD clicks_daily_index
     ALTER CLUSTER posts DROP weekly_index


Node where statement got issued sends local indexes among other nodes in cluster. All local indexes with same name
at other cluster's nodes get replaced. There is no restriction of how many index cluster has.


.. _repliation_write:

Write statements to cluster indexes
----------------------------------------

All write statements such as ``INSERT``, ``REPLACE``, ``DELETE``, ``TRUNCATE`` these change content of cluster's
index should use ``cluster_name:index_name`` format of identificator to make sure the change is for an index at cluster
and will be replicated to all nodes in the cluster and not for index itself otherwise statement will fail with error message.
Other statements such as call or list could use regular index name.

.. code-block:: sql


     INSERT INTO posts:weekly_index VALUES ( 'iphone case' )
     TRUNCATE RTINDEX click_query:weekly_index


Insert of percolate queries at multiple nodes at same time with document ``id`` auto generated might fail as for now
``id`` auto generation looks only for local index and does not take into account cluster distribution.
Reinserting query should work well. However replacing of percolate queries at multiple nodes at same time with document
``id`` auto generated might cause to replace the only query by last finished request.

This ``id`` auto generation behavior will be changed by switch to UUID or cluster auto_increment option.


.. _repliation_status:

Cluster status
----------------------------------------

:ref:`SHOW STATUS <show_status_syntax>` now outputs also cluster status variables. Output format is
``cluster_name_variable_name`` ``variable_value`` . Most of them described at 
`Galera Documentation Status <http://galeracluster.com/documentation-webpages/galerastatusvariables.html>`__ we additionaly
displays 

- cluster_name - the name of the cluster

- node_state - current state of node such as ``undefined``, ``joiner``, ``donor``, ``joined``, ``synced``, ``error``

- indexes_count - how many indexes managed by cluster

- indexes - list of index names managed by cluster


.. code-block:: sql


    mysql> SHOW STATUS;
    +-----------------------------------------+---------------------------------------------------+
    | Counter                                 | Value                                             |
    +-----------------------------------------+---------------------------------------------------+
    | cluster_name                            | post                                              |
    | cluster_post_state_uuid                 | fba97c45-36df-11e9-a84e-eb09d14b8ea7              |
    | cluster_post_conf_id                    | 1                                                 |
    | cluster_post_status                     | primary                                           |
    | cluster_post_size                       | 5                                                 |
    | cluster_post_local_index                | 0                                                 |
    | cluster_post_node_state                 | synced                                            |
    | cluster_post_indexes_count              | 2                                                 |
    | cluster_post_indexes                    | pq1,pq_posts                                      |



.. _repliation_set:

Cluster parameters
----------------------------------------

Replication plugin option might be changed using :ref:`SET <set_syntax>` statement

.. code-block:: sql

     SET CLUSTER click_query GLOBAL 'pc.bootstrap=1'
	 

.. _repliation_restart:

Cluster restart
----------------------------------------

Cluster requires to start one node in a cluster as a reference point, before the all other nodes
join it and form the cluster. This is a cluster bootstrap and it introduces a primary component, before others see it as
a reference point to sync up data. Restart of single node or reconnecting node after shutdown should be done as usual.

In case of cluster restart or starting whole cluster after shutdown daemon stopped last should be started first
with command line key ``--new-cluster`` . To make sure that daemon is able to start as a reference point at
file ``grastate.dat`` into cluster :ref:`path <cluster_path>` should be set ``1`` for option ``safe_to_bootstrap`` .
Attempt to start any other daemon or daemon without that option will cause error message.
To override this protection and start cluster from another daemon command line key ``--new-cluster-force`` could be used.

In case of hard crush or unclean shutdown of all daemons in cluster someone must identify most advanced node with largest
``seqno`` at ``grastate.dat`` file at cluster :ref:`path <cluster_path>` and start that daemon with command line
key ``--new-cluster-force`` .


.. _repliation_diverge:

Cluster with diverged nodes
----------------------------------------

Sometime nodes can be diverged from each other. The state of all nodes might turn into ``non-primary`` due to network split
between nodes, cluster crash, or if replication plugin hit an exception when determining the ``primary component``. Then
it is need to select a node and promote it to be a ``primary component``.

To determine which node needs to be reference, compare the ``last_committed`` cluster status variable value on all nodes.
In case all daemon are already running, no need to start the cluster again. It just needs to promote most advanced node
to be a ``primary component`` with :ref:`SET <set_syntax>` statement

.. code-block:: sql

     SET CLUSTER posts GLOBAL 'pc.bootstrap=1'
	 
	 
All other nodes will reconnect to the node and resync their data based on this node.
