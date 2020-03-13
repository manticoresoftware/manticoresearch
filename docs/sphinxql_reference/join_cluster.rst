.. _join_cluster_syntax:

JOIN CLUSTER syntax
--------------------

.. code-block:: none

    JOIN CLUSTER name AT 'ip'
	
    JOIN CLUSTER name [cluster_option,...] 
	
    cluster_option:
       'value' as oath


:ref:`Joins<replication_join_at>` a cluster.

The statement has 2 syntax flavors:


.. code-block:: mysql

    mysql> JOIN CLUSTER posts AT '123.456.789.012:9313';
    Query OK, 0 rows affected (0.00 sec)


For simple cases, the name of the cluster and the IP of one of the nodes must be provided.

.. code-block:: mysql

	mysql> JOIN  CLUSTER click_query '/var/data/click_query/' as path, 'clicks_mirror1:9312,clicks_mirror2:9312,clicks_mirror3:9312' as nodes;
	Query OK, 0 rows affected (0.00 sec)
	
If the node connects to more than one cluster, the storage of replication metadata needs to be in different folders. In this case it's required to explicit set the 'path' of the metadata folder.

A list of `nodes` may be required to be defined in case the cluster nodes are located at different network segments or datacenters. 
In this case :ref:`ALTER CLUSTER name UPDATE nodes <alter_cluster_syntax>` needs to be run to trigger update the nodes list on all cluster nodes.