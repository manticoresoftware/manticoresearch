.. _guide_indexes:

A guide on indexes
-------------------

The Manticore Search daemon can serve multiple data collections, called indexes.

Manticore Search supports two storage index types:

* plain (also called offline or disk) index. Data is indexed once at creation, it supports online rebuilding and online updates for non-text attributes
* RealTime index. Similar to a database table, online updates are possible at any given time

In addition, a special index based on RealTime type, called `percolate`, can be used to store :ref:`Percolate Queries <percolate_query>`.

In the current version, indexes use a schema like a normal database table. The schema can have 3 big types of columns:

* the first column is always an unsigned 64 bit non-zero number, called `id`. Unlike in a database, there is no mechanism of auto incrementing, so you need to be sure the documents ids are unique
* full-text fields - they contain indexed content. There can be multiple full-text fields per index. Full-text searches can be made on all fields or selective. Starting with 3.2 it's possible to also store the original content and retrieve it in results.
* attributes - their values are stored and are not used in full-text matching. Instead they can be used for regular filtering, grouping, sorting. They can be also used in expressions of score ranking.

Field and attribute names must start with a letter and can contain letters, digits and underscore.

The following types can be stored in attributes:

* unsigned 32 bit and signed 64 bit integers
* 32 bit single precision floats
* UNIX timestamps
* booleans
* strings (they can be used just for comparison,grouping or sorting by)
* JSON objects
* multi-value attribute list of unsigned 32-bit integers




Manticore Search supports a storeless index type called distributed which allows searching over multiple indexes. The connected indexes can be local or remote. Distributed indexes allow spreading big data over multiple machines or building high availability setups. As searching over an index is single-threaded, local distributed indexes can be used to make use of multiple CPU cores.



Plain indexes
~~~~~~~~~~~~~
Except numeric (that includes MVA) attributes, the rest of the data in a plain index is immutable. If you need to update/add new records you need to perform again a rebuilding. While index is being rebuilt, existing index is still available to serve requests. When new version is ready, a process called `rotation` is performed which puts the new version online and discards the old one.

The indexing performance process depends on several factors:

* how fast the source can be providing the data
* tokenization settings
* hardware resource (CPU power, storage speed)

In the most simple usage scenario, we would use a single plain index which we rebuild it from time to time. 

This implies:

* index is not as fresh as the data from the source
* indexing duration grows with the data

If we want to have the data more fresh, we need to shorten the indexing interval. If indexing takes too much, it can even overlap the time between indexing, which is a major problem.
However, Manticore Search can perform a search on multiple indexes. From this, an idea was born to use a secondary index that captures only the most recent updates. 

This index will be a lot smaller and we will index it more frequently. From time to time, as this delta index will grow, we will want to "reset" it. 

This can be done by either reindexing the main index  or merge the delta into the main. The main+delta index schema is detailed at :ref:`delta_index_updates`.

As the engine can't globally do a uniqueness on the document ids, an important thing that needs to be considered is if the delta index could contain updates on existing indexed records in the main index.

For this, there is an option that allows defining a list of document ids which are suppressed by the delta index. For more details, check :ref:`sql_query_killlist`.

An example of a plain index configuration using a MySQL source:

.. code-block::  none

  source mysource {
    type             = mysql
	path             = /path/to/realtime
    sql_host         = localhost
	sql_user         = myuser
	sql_pass         = mypass
	sql_db           = mydb
	sql_query        =  SELECT id, title, description, category_id  from mytable
	sql_attr_uint    = category_id
	sql_field_string = title
   }
   
  index myindex {
    type   = plain
	source = mysource
	path   = /path/to/myindex
    ...
   }
   

Real-Time indexes
~~~~~~~~~~~~~~~~~

Real-Time indexes allow online updates, but updating full-text data and non-numeric attributes require a full row replace.

The Real-Time index  starts empty and you can add, replace, update or delete data in the same fashion as for a database table. The updates are first held into a memory zone, defined by :ref:`rt_mem_limit`. 
When this gets filled, it is dumped as disk chunk -  which as structure is similar with a plain index. As the number of disk chunks increase, the search performance decreases, as the searching is done sequentially on the chunks.
To avoid that, there is a command that can merge the disk chunks into a single one - :ref:`optimize_index_syntax`. 

Populating a Real-Time index can be done in two ways: firing INSERTs or converting a plain index to become RealTime.
In case of INSERTs, using a single worker (a script or code) that inserts one record at a time can be slow. You can speed this by batching many rows into one and by using multiple workers that perform inserting. 
Parallel inserts will be faster but also come at using more CPU. The size of the data buffer memory (which we call RAM chunk) also influence the speed of inserting.

An example of Real-Time index configuration:


.. code-block::  none

  index realtime {
    type           = rt
	path           = /path/to/realtime
	rt_field       = title
	rt_field       = description
	rt_attr_uint   = category_id
	rt_attr_string = title
	rt_attr_json   = metadata
	stored_fields  = description
    ...
   }
   

Local distributed indexes
~~~~~~~~~~~~~~~~~~~~~~~~~
A distributed index in Manticore Search doesn't hold any data. Instead it acts as a 'master node' to fire the demanded query on other indexes and provide  merged results from the responses it receives from the 'node' indexes.
A distributed index can connect to local indexes or indexes located on other servers. 
In our case, a distributed index would look like:

.. code-block::  none

  index index_dist {
    type  = distributed
    local = index1
    local = index2
    ...
   }
   
The last step to enable multi-core searches is to define dist_threads  in searchd section. Dist_threads tells the engine the maximum number of threads it can use for a distributed index.



Remote distributed indexes and high availability
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
.. code-block:: none

   index mydist {
             type  = distributed
             agent = box1:9312:shard1
             agent = box2:9312:shard2
             agent = box3:9312:shard3
             agent = box4:9312:shard4
   }
   
Here we have split the data over 4 servers, each serving one of the shards. If one of the servers fails, our distributed index will still work, but we would miss the results from the failed shard.

.. code-block:: none

   index mydist {
             type  = distributed
             agent = box1:9312|box5:9312:shard1
             agent = box2:9312:|box6:9312:shard2
             agent = box3:9312:|box7:9312:shard3
             agent = box4:9312:|box8:9312:shard4
   }
   
Now we added mirrors, each shard is found on 2 servers. By default, the master (the searchd instance with the distributed index) will pick randomly one of the mirrors.

The mode used for picking mirrors can be set with ha_strategy. In addition to random, another simple method is to do a round-robin selection ( ha_strategy= roundrobin).

The more interesting strategies are the latency-weighted probabilities based ones. noerrors and nodeads not only that take out mirrors with issues, but also monitor the response times and do a balancing. If a mirror responds slower (for example due to some operations running on it), it will receive less requests. When the mirror recovers and provides better times, it will get more requests.
