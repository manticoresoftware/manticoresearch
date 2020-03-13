.. _create_cluster_syntax:

CREATE CLUSTER syntax
---------------------

.. code-block:: none


    CREATE CLUSTER name [cluster_option,...] 
	
    cluster_option:
       'value' as oath


:ref:`Create<replication_create>` a cluster.

By default, the cluster metadata is saved in the :ref:`data_dir` and can contain the metadata for a single cluster.
Multiple clusters can be created on same instance, but the cluster metadata files can't be shared in the same folder.
Therefor, it's required to define own folder for cluster data using 'path` option.

An optional list of nodes can be declared with 'nodes' option. Explicit definition of the nodes list may be required in cases where nodes are located 
at different network segments or datacenters.


.. code-block:: mysql

    mysql> CREATE CLUSTER posts;
    Query OK, 0 rows affected (0.00 sec)
    
    mysql> CREATE CLUSTER click_query '/var/data/click_query/' as path, 'clicks_mirror1:9312,clicks_mirror2:9312,clicks_mirror3:9312' as nodes;
    Query OK, 0 rows affected (0.00 sec)