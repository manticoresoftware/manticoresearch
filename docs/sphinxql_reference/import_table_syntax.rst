.. _import_table_syntax:

IMPORT TABLE syntax
-------------------

.. code-block:: none


    IMPORT TABLE table_name FROM 'path'
	

Imports a peviously created RT index into a daemon running in RT mode. All index files are copied to :ref:`data_dir`.
All external index files such as wordforms, exceptions and stopwords are also copied to :ref:`data_dir`.
IMPORT TABLE has the following limitations:

-  paths to external files that were originally specified in the config file must be absolute;

-  only RT indexes are supported;

-  plain indexes need to be converted to RT indexes via ATTACH INDEX; see :ref:`attach_index_syntax`.