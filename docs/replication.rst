
.. _replication:

Replication
===========

Manticore search daemon can replicate a write transaction (``INSERT``, ``REPLACE``, ``DELETE``, ``TRUNCATE``, ``UPDATE``, etc)
in an index to other nodes in the cluster. 
Currently percolate and Real-Time indexes are supported.
Only Linux packages and builds support replication, Windows and MacOS packages do not support replication.

We took advantage of Percona's fork of Galera library which gives the following benefits:

- true multi-master - read and write to any node at any time

- synchronous replication - no slave lag, no data is lost after a node crash

- hot standby - no downtime during failover (since there is no failover)

- tightly coupled - all the nodes hold the same state. No diverged data between nodes allowed

- automatic node provisioning -  no need to manually back up the database and restore it on a new node

- easy to use and deploy

- detection and automatic eviction of unreliable nodes

- certification based replication

To use replication in Manticore search:

- the daemon should be built with replication support (enabled in the `builds Manticore provides <https://manticoresearch.com/downloads/>`_)

- :ref:`data_dir <data_dir>` option should be set in :ref:`searchd <searchd_program_configuration_options>` section of config

- there should be a :ref:`listen <listen>` for the replication protocol directive containing an external IP address that should not be equal to 0.0.0.0 along with a ports range defined and these "address - port range" pairs should be different for all the daemons on the same box. As a rule of thumb, port range should specify no less than two ports per cluster.

- there should be at least one value of :ref:`listen <listen>` for the SphinxAPI protocol directive
- set unique values on :ref:`server_id <server_id>` for each node. If no value set, the node will try to use the MAC address (or a random number if this fails) to generate a server_id.

Example of searchd configuration block:

.. code-block::  none

  searchd {
    listen    = 9312
    listen    = 192.168.1.101:9360-9370:replication
    data_dir  = /var/lib/manticore/replication/
    server_id = 123
    ...
   }

.. _replication_cluster:

Replication cluster
-------------------

Replication cluster is a set of nodes among which a write transaction gets replicated.
Replication is configured on the per-index basis. One index can be assigned to only
one cluster. There is no restriction on how many indexes a cluster may have. All
transactions such as ``INSERT``, ``REPLACE``, ``DELETE``, ``TRUNCATE`` in any 
index belonging to a cluster are replicated to all the other nodes in the
cluster. Replication is multi-master, so writes to any particular node or to
multiple nodes simultaneously work equally well.

Replication cluster configuration options are:

.. _cluster_name:

name
~~~~

Specifies a name for the cluster. Should be unique.

.. _cluster_path:

path
~~~~

Data directory for a `write-set cache replication <https://galeracluster.com/documentation-webpages/documentation/state-transfer.html>`_
and incoming indexes from other nodes. Should be unique among the other clusters in the node. Default is :ref:`data_dir <data_dir>`.

.. _cluster_nodes:

nodes
~~~~~

A list of address:port pairs for all the nodes in the cluster (comma separated).
A node's API interface should be used for this option.
It can contain the current node's address too.
This list is used to join a node to the cluster and rejoin it after restart.

.. _cluster_options:

options
~~~~~~~

Options passed directly to Galera replication plugin as described
here `Galera Documentation Parameters <https://galeracluster.com/documentation-webpages/documentation/galera-parameters.html>`_


.. _replication_create:

Creating a cluster
------------------

To create a cluster you should set at least its :ref:`name <cluster_name>`. In case of a single cluster or if the cluster you are creating is the first one
:ref:`path <cluster_path>` option may be omitted, in this case :ref:`data_dir <data_dir>` option will be used as the cluster path.
For all subsequent clusters you need to specify :ref:`path <cluster_path>` and this path should be available. :ref:`nodes <cluster_nodes>` option
may be also set to enumerate all the nodes in the cluster.

.. code-block:: sql

    CREATE CLUSTER posts
    CREATE CLUSTER click_query '/var/data/click_query/' as path
    CREATE CLUSTER click_query '/var/data/click_query/' as path, 'clicks_mirror1:9312,clicks_mirror2:9312,clicks_mirror3:9312' as nodes

If a cluster is created without the :ref:`nodes <cluster_nodes>` option, the first node that gets joined to the cluster will be saved as `nodes`.


.. _replication_join_at:

Joining a cluster
-----------------

To join an existing cluster :ref:`name <cluster_name>` and any working node should be set.
In case of a single cluster :ref:`path <cluster_path>` might be omitted, :ref:`data_dir <data_dir>`
will be used as the cluster path. For all subsequent clusters :ref:`path <cluster_path>` needs to be set and it should be available.

.. code-block:: sql

    JOIN CLUSTER posts AT '10.12.1.35:9312'

A node joins a cluster by getting the data from the node provided and, if successful, it updates node lists in all the other cluster nodes
similar to :ref:`ALTER CLUSTER ... UPDATE nodes <replication_alter_update>`. These lists are used to rejoin nodes to
the cluster on restart.

There are two lists of nodes. One is used to rejoin nodes to the cluster on restart, it is updated across all nodes by
:ref:`ALTER CLUSTER ... UPDATE nodes <replication_alter_update>`. ``JOIN CLUSTER ... AT`` does the same update automatically.
:ref:`SHOW STATUS <replication_status>` shows this list as ``cluster_post_nodes_set``.

The second list is a list of all active nodes used for replication. This list doesn't require manual management.
:ref:`ALTER CLUSTER ... UPDATE nodes <replication_alter_update>` actually copies this list of nodes to the list of nodes
used to rejoin on restart. :ref:`SHOW STATUS <replication_status>` shows this list as ``cluster_post_nodes_view``.

When nodes are located at different network segments or in different datacenters :ref:`nodes <cluster_nodes>` option may be set
explicitly. That allows to minimize traffic between nodes and to use gateway nodes for datacenters intercommunication.
The following command joins an existing cluster using the :ref:`nodes <cluster_nodes>` option.

.. code-block:: sql

    JOIN CLUSTER click_query 'clicks_mirror1:9312;clicks_mirror2:9312;clicks_mirror3:9312' as nodes, '/var/data/click_query/' as path

Note that when this syntax is used, ``cluster_post_nodes_set`` list is not updated automatically. Use :ref:`ALTER CLUSTER ... UPDATE nodes <replication_alter_update>`
to update it.

``JOIN CLUSTER`` statement completes when a node receives all the neccesary data to be in sync with all the other nodes in the cluster.


.. _replication_delete:

Deleting a cluster
------------------

Delete statement removes a cluster specified with :ref:`name <cluster_name>`. The cluster
gets removed from all the nodes, but its indexes are left intact and become active local non-replicated indexes.

.. code-block:: sql

     DELETE CLUSTER click_query


.. _replication_alter:

Managing indexes
----------------

``ALTER CLUSTER <cluster_name> ADD <index_name>`` adds an existing local index to the cluster.
The node which receives the ALTER query sends the index to the other nodes in the cluster. All the local
indexes with the same name on the other nodes of the cluster get replaced with the new index.

``ALTER CLUSTER <cluster_name> DROP <index_name>`` forgets about a local index, i.e., it doesn't remove
the index files on the nodes but just makes it an active non-replicated index.

.. code-block:: sql

     ALTER CLUSTER click_query ADD clicks_daily_index
     ALTER CLUSTER posts DROP weekly_index


.. _replication_alter_update:

Managing nodes
--------------

``ALTER CLUSTER <cluster_name> UPDATE nodes`` statement updates node lists on each node of the cluster to include
every active node in the cluster. See :ref:`Joining a cluster<replication_join_at>` for more info on node lists.

.. code-block:: sql

     ALTER CLUSTER posts UPDATE nodes
	 
For example, when the cluster was initially created, the list of nodes used for rejoining the cluster was ``10.10.0.1:9312,10.10.1.1:9312``.
Since then other nodes joined the cluster and now we have the following active nodes: ``10.10.0.1:9312,10.10.1.1:9312,10.15.0.1:9312,10.15.0.3:9312``.
But the list of nodes used for rejoining the cluster is still the same. Running the ``ALTER CLUSTER ... UPDATE nodes`` statement
copies the list of active nodes to the list of nodes used to rejoin on restart. After this, the list of nodes used on restart includes all
the active nodes in the cluster.

Both lists of nodes can be viewed using :ref:`SHOW STATUS <replication_status>` statement (``cluster_post_nodes_set`` and ``cluster_post_nodes_view``).

.. _replication_write:

Write statements
----------------

All write statements such as ``INSERT``, ``REPLACE``, ``DELETE``, ``TRUNCATE``, ``UPDATE`` that
change the content of a cluster's index should use ``cluster_name:index_name`` expression in place of an index name to make
sure the change is propagated to all replicas in the cluster. An error will be triggered otherwise.

.. code-block:: sql

     INSERT INTO posts:weekly_index VALUES ( 'iphone case' )
     TRUNCATE RTINDEX click_query:weekly_index
     UPDATE INTO posts:rt_tags SET tags=(101, 302, 304) WHERE MATCH ('use') AND id IN (1,101,201)
     DELETE FROM clicks:rt WHERE MATCH ('dumy') AND gid>206

Read statements such as ``CALL PQ``, ``SELECT`` or ``DESCRIBE`` can use either regular index names not prepended with
a cluster name or ``cluster_name:index_name``. ``cluster_name:index_name`` syntax ignores the cluster name and may be used
on an index that doesn't belong to the cluster.

.. code-block:: sql

     SELECT * FROM weekly_index
     CALL PQ('posts:weekly_index', 'document is here')

ID auto generation uses UUID_SHORT similar to MySQL function. It is valid cluster wide UUID when :ref:`server_id <server_id>`
properly configured.

All write statements for HTTP interface to a cluster's index should set ``cluster`` property along with ``index`` name.
An error will be triggered otherwise.

``UpdateAttributes`` statement from API interface to specific index always set proper cluster at server and there is no way
to know is update to index got propagated into cluster properly or node diverged and statement updated only local index.

.. _replication_status:

Cluster status
--------------

:ref:`SHOW STATUS <show_status_syntax>` outputs, among other information, cluster status variables. The output format is
``cluster_name_variable_name`` ``variable_value``. Most of them are described in
`Galera Documentation Status <http://galeracluster.com/documentation-webpages/galerastatusvariables.html>`__.
Additionally we display:

- cluster_name - :ref:`name <cluster_name>` of the cluster

- node_state - current state of the node: ``closed``, ``destroyed``, ``joining``, ``donor``, ``synced``

- indexes_count - number of indexes managed by the cluster

- indexes - list of index names managed by the cluster

- nodes_set - list of nodes in the cluster defined with cluster CREATE, JOIN or ALTER UPDATE commands

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
------------------

Replication plugin options can be changed using :ref:`SET <set_syntax>` statement:

.. code-block:: sql

     SET CLUSTER click_query GLOBAL 'pc.bootstrap' = 1

See Galera `Documentation Parameters <http://galeracluster.com/documentation-webpages/galeraparameters.html>`_
for a list of available options.

.. _replication_restart:

Restarting a cluster
--------------------

A replication cluster requires its single node to be started as a
reference point before all the other nodes join it and form a cluster. This is
called cluster bootstrapping which introduces a ``primary component`` before others
see that as a reference point to sync up the data from. The restart of a single node
or reconnecting from a node after a shutdown can be done as usual.

After the whole cluster shutdown the daemon that was stopped last should be started first
with ``--new-cluster`` command line option. To make sure that the daemon is able to
start as a reference point the ``grastate.dat`` file located at the cluster :ref:`path <cluster_path>`
should be updated with the value of ``1`` for ``safe_to_bootstrap`` option. I.e.,
both conditions, ``--new-cluster`` and ``safe_to_bootstrap=1``, must be satisfied.
An attempt to start any other node without these options set will trigger an error.
To override this protection and start cluster from another daemon forcibly, ``--new-cluster-force``
command line option may be used.

In case of a hard crash or an unclean shutdown of all the daemons in the cluster you need to
identify the most advanced node that has the largest ``seqno`` in the ``grastate.dat`` file
located at the cluster :ref:`path <cluster_path>` and start that daemon with the command line
key ``--new-cluster-force``.


.. _replication_diverge:

Cluster with diverged nodes
---------------------------

Sometimes replicated nodes can diverge from each other. The state of all the nodes
might turn into ``non-primary`` due to a network split between nodes, a cluster
crash, or if the replication plugin hits an exception when determining the ``primary component``.
Then it's necessary to select a node and promote it to the ``primary component``.

To determine which node needs to be a reference, compare the ``last_committed``
cluster status variable value on all nodes. If all the daemons are already
running there's no need to start the cluster again. You just need to promote the
most advanced node to the ``primary component`` with :ref:`SET <set_syntax>` statement:

.. code-block:: sql

     SET CLUSTER posts GLOBAL 'pc.bootstrap' = 1


All other nodes will reconnect to the node and resync their data based on this node.

.. toctree::
   :titlesonly:
   
   replication/recover-cluster
   
   