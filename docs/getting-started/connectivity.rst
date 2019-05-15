.. _guide_connectivity:

A guide on connectivity
-----------------------

Manticore Search offers 3 protocols that allows clients to connect in multiple ways. 

SphinxAPI protocol
~~~~~~~~~~~~~~~~~~

The SphinxAPI protocol is used for both connecting with clients as well as connecting multiple daemon instances for cluster setups.
The default port (and assigned by IANA) for SphinxAPI protocol is 9312.

Clients can connect to Manticore over SphinxAPI protocol using API client libraries.
Officially, we support API client libraries for:

* C
* PHP
* Java
* Ruby
* Go

The Go client has it's own repository on `github <https://github.com/manticoresoftware/go-sdk>`_ , while the rest of the clients can be found in `/api <https://github.com/manticoresoftware/manticoresearch/tree/master/api>`_ directory.

For other languages, 3rd party APi clients exists. 
Older API client libraries, including versions for Sphinx Search, should work with newer daemons, however they will not be able to access new features.

SphinxAPI protocol is also used by SphinxSE MySQL plugin, which acts as a proxy engine that allows running Manticore queries from within a MySQL server.
For more information about SphinxSE see :ref:`sphinxse_storage_engine`.

SphinxQL protocol
~~~~~~~~~~~~~~~~~

SphinxQL is an implementation of  MySQL 4.1 protocol. 
Manticore doesn't implement the full range of SQL commands or functions available in MySQL.
There are custom clauses and functions, like the MATCH(), implemented in MySQL that are not found in MYSQL.

Manticore Search doesn't support server-side prepared statements. 
Client-side prepared statements can be used with Manticore. It must be noted that Manticore implements the multi value (MVA) data type for which there is no equivalent in MySQL
or libraries implementing prepared statements. In these cases, the MVA values will need to be crafted in the raw query.


Manticore Search does not have the concept of database like MySQL and no user access control is yet implemented.
However some MySQL clients/connectors demand values set for user/password or database name. These can be set arbitrarily as Manticore will simply ignore the values.

The default port (and assigned by IANA) for SphinxQL protocol is 9306.

MySQL Connectors
^^^^^^^^^^^^^^^^
The official MySQL connectors can be used to connect to Manticore Search, however they might require certain settings passed in the DSN string as the connector 
can try running certain SQL commands not implemented yet in Manticore. 

JDBC Connector 6.x and above require Manticore Search 2.8.2 or greater and the DSN string should contain the following options:

.. code-block:: none

		jdbc:mysql://IP:PORT/DB/?characterEncoding=utf8&maxAllowedPacket=512000&serverTimezone=XXX
		

In case mysql_version_string is used to suppress the Manticore version, the override version should be smaller than **5.1.1**.

.NET MySQL connector uses connection pools by default. To correctly get the statistics of SHOW META, queries along with SHOW META command should be send as one multistatement  (``SELECT ...;SHOW META``). 
With pooling enabled option ``Allow Batch=True`` is required to be added to connection string to allow multistatements. 

.. code-block:: none

		Server=127.0.0.1;Port=9306;Database=somevalue;Uid=somevalue;Pwd=;Allow Batch=True;
		

ODBC
^^^^

Manticore can be accessed using ODBC. It's recommended to set `charset=UTF8` in the ODBC string. 
Some ODBC drivers will not like the reported version by the Manticore daemon. This can be overridden with :ref:`mysql_version_string` option.

The SphinxQL protocol can also be used by a MYSQL server to connect to Manticore using FEDERATED engine.  
For more details about how to connect to Manticore using FEDERATED consult :ref:`federated_storage_engine`.

HTTP API protocol
~~~~~~~~~~~~~~~~~

Recommended port for the HTTP protocol is 9308. Please note this port is not assigned by IANA, while there is no known application to use it, it should be checked if it's available.

There are no special requirements and any HTTP client can be used to connect to Manticore. Some endpoints may require specific headers set, for more information consult :ref:`httpapi_reference`.