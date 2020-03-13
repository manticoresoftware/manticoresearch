.. _delete_cluster_syntax:

DELETE CLUSTER syntax
---------------------

.. code-block:: none

	DELETE CLUSTER name
	

:ref:`Deletes<replication_delete>` a cluster. The command will be propagated to all nodes in the cluster. 

Note that after cluster deletion, the indexes remain intact on the nodes and become local non-replicated indexes.