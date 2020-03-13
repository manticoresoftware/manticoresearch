.. _alter_cluster_syntax:

ALTER CLUSTER syntax
--------------------

.. code-block:: none

	ALTER CLUSTER name UPDATE nodes
	

Currently the ALTER statement supports only :ref:`UPDATE nodes<replication_alter>` operation. This operation is required to be run manual when nodes list are explicit defined. 
It will update the list of nodes used for rejoining in case of restart.

.. code-block:: mysql

	mysql> ALTER CLUSTER posts UPDATE nodes;
	Query OK, 0 rows affected (0.00 sec)
	
