Getting Started using official packages
---------------------------------------

Installing and running
~~~~~~~~~~~~~~~~~~~~~~

Packages of latest GA release can be downloaded from http://www.manticoresearch.com/downloads

.. code-block:: bash
   
   $ wget https://github.com/manticoresoftware/manticore/releases/download/x.y.z/manticore_z.y.z.deb
   $ sudo dpkg -i manticore_x.y.z.deb

Start the service:

.. code-block:: bash

    $ systemctl start manticore
	
or

.. code-block:: bash

    $ service manticore start

depending on the distribution used.

At this point you can start using Manticore Search. The configuration file is located at ``/etc/sphinx/sphinx.conf`` (RHEL/CentOS) or ``/etc/sphinxsearch/sphinx.conf`` (Debian/Ubuntu).  The default configuration comes with an empty RT index ready to be used and a sample plain index and interfaces SphinxQL on port 9306 and native API on port 9312.

You can also compile Manticore Search from `sources <https://github.com/manticoresoftware/manticore>`__. `Compilation <http://docs.manticoresearch.com/latest/html/installation.html#compiling-manticore-from-source>`__ is easy and uses cmake and you can also create packages for your operating system. 

Running queries
~~~~~~~~~~~~~~~

The simple way to connect and do some tests is to use the `SphinxQL <http://docs.manticoresearch.com/latest/html/sphinxql_reference.html>`__ protocol. For this, you need a mysql command line client.

While it implements the MySQL protocol, SphinxQL is not 100% compatible with MySQL syntax. There are specific extensions, like MATCH clause [the most powerful thing in Manticore] or WITHIN GROUP BY and many functions available in MySQL are not implemented (or they are dummy just to allow compatibility with MySQL connector e.g.) or JOINs between indexes which are not supported yet.

First, let's connect to Manticore Search and take a look at the available indexes:

.. code-block:: bash 
   
   $ mysql -P9306 -h0

.. code-block:: mysql


   mysql> SHOW TABLES;
   +-------+-------------+
   | Index | Type        |
   +-------+-------------+
   | dist1 | distributed |
   | testrt| rt          |
   +-------+-------------+
   2 rows in set (0.00 sec)

Now let's look at our RT index:

.. code-block:: mysql

   mysql> DESCRIBE testrt;
   +---------+--------+
   | Field   | Type   |
   +---------+--------+
   | id      | bigint |
   | title   | field  |
   | content | field  |
   | gid     | uint   |
   +---------+--------+
   4 rows in set (0.00 sec)

As the RT indexes start empty, let's add some data into it first   


.. code-block:: mysql

  mysql> INSERT INTO testrt VALUES(1,'List of HP business laptops','Elitebook Probook',10);
  Query OK, 1 row affected (0.00 sec)

  mysql> INSERT INTO testrt VALUES(2,'List of Dell business laptops','Latitude Precision Vostro',10);
  Query OK, 1 row affected (0.00 sec)

  mysql> INSERT INTO testrt VALUES(3,'List of Dell gaming laptops','Inspirion Alienware',20);
  Query OK, 1 row affected (0.00 sec)
  
  mysql> INSERT INTO testrt VALUES(4,'Lenovo laptops list','Yoga IdeaPad',30);
  Query OK, 1 row affected (0.01 sec)

  mysql> INSERT INTO testrt VALUES(5,'List of ASUS ultrabooks and laptops','Zenbook Vivobook',30);
  Query OK, 1 row affected (0.01 sec)

Now we have some data, we can do some queries.

Fulltext searches are done with the special clause MATCH, which is the main workhorse.


.. code-block:: mysql

   mysql>  SELECT * FROM testrt WHERE MATCH('list of laptops');
   +------+------+
   | id   | gid  |
   +------+------+
   |    1 |   10 |
   |    2 |   10 |
   |    3 |   20 |
   |    5 |   30 |
   +------+------+
   4 rows in set (0.00 sec)


As you see in the result set we can only get back the doc id and the attributes.
The fulltext fields values are not returned since the text is only indexed, not stored also, and it's impossible to rebuild the original text.

Now let's add some filtering and more ordering:

.. code-block:: mysql
  
   mysql>  SELECT *,WEIGHT() FROM testrt WHERE MATCH('list of laptops') AND gid>10  ORDER BY WEIGHT() DESC,gid DESC;
   +------+------+----------+
   | id   | gid  | weight() |
   +------+------+----------+
   |    5 |   30 |     2334 |
   |    3 |   20 |     2334 |
   +------+------+----------+
   2 rows in set (0.00 sec)


The WEIGHT() function returns the calculated matching score. If no ordering specified, the result is sorted descending by the score provided by WEIGHT().
In this example we order first by weight and then by an integer attribute.

The search above does a simple matching, where all words need to be present. But we can do more (and this is just a simple example):

.. code-block:: mysql

   mysql> SELECT *,WEIGHT() FROM testrt WHERE MATCH('"list of business laptops"/3');
   +------+------+----------+
   | id   | gid  | weight() |
   +------+------+----------+
   |    1 |   10 |     2397 |
   |    2 |   10 |     2397 |
   |    3 |   20 |     2375 |
   |    5 |   30 |     2375 |
   +------+------+----------+
   4 rows in set (0.00 sec)
   
   
   mysql> SHOW META;
   +---------------+----------+
   | Variable_name | Value    |
   +---------------+----------+
   | total         | 4        |
   | total_found   | 4        |
   | time          | 0.000    |
   | keyword[0]    | list     |
   | docs[0]       | 5        |
   | hits[0]       | 5        |
   | keyword[1]    | of       |
   | docs[1]       | 4        |
   | hits[1]       | 4        |
   | keyword[2]    | business |
   | docs[2]       | 2        |
   | hits[2]       | 2        |
   | keyword[3]    | laptops  |
   | docs[3]       | 5        |
   | hits[3]       | 5        |
   +---------------+----------+
   15 rows in set (0.00 sec)

   
Here we search for 4 words, but we can have a match even if only 3 words (of 4) are found. The search will rank higher first the documents that contain all the words.
We also added a `SHOW META  <http://docs.manticoresearch.com/latest/html/sphinxql_reference/show_meta_syntax.html>`__ command. 
SHOW META returns information about previous executed query, that is number of found records (in total_found), execution time (in time) and statistics about the keywords of the search.


To create a new RT index, you need to define it in the sphinx.conf. A simple definition looks like:

.. code-block:: none

   index myindexname {
         type = rt
         path = /path/to/myrtindex
         rt_mem_limit = 256M
         rt_field = title
         rt_attr_uint = attr1
         rt_attr_uint = attr2
   }

To get the index online you need to either restart the daemon or send a HUP signal to it.

Using plain indexes
~~~~~~~~~~~~~~~~~~~

Unlike RT, the plain index requires setting up the source and run the indexing process which gathers the data.
For this we need to edit the sphinx.conf configuration file. The initial configuration comes with a sample plain index along with a source.
For simplicity we use a MySQL source.

First, the database credentials need to be adjusted 

.. code-block:: none
   
   ...
      sql_host                = localhost
      sql_user                = test
      sql_pass                =
      sql_db                  = test
      sql_port                = 3306  # optional, default is 3306
   ...

Then we look after the ``sql_query``, which is the query that grabs the data

.. code-block:: none

        sql_query               = \
                SELECT id, group_id, UNIX_TIMESTAMP(date_added) AS date_added, title, content \
                FROM documents

For a quick test, we're going to use the following sample table in MySQL:

.. code-block:: mysql

   DROP TABLE IF EXISTS test.documents;
   CREATE TABLE test.documents
   (
   	id			INTEGER PRIMARY KEY NOT NULL AUTO_INCREMENT,
   	group_id	INTEGER NOT NULL,
	date_added	DATETIME NOT NULL,
	title		VARCHAR(255) NOT NULL,
	content		TEXT NOT NULL
   );
   
   INSERT INTO test.documents ( id, group_id,  date_added, title, content ) VALUES
	( 1, 1, NOW(), 'test one', 'this is my test document number one. also checking search within phrases.' ),
	( 2, 1, NOW(), 'test two', 'this is my test document number two' ),
	( 3, 2, NOW(), 'another doc', 'this is another group' ),
	( 4, 2, NOW(), 'doc number four', 'this is to test groups' );



If you want to use your table, you need make some changes in the source definition. One is to modify the ``sql_query``. Keep in mind that the first column in the result set must be an unsigned unique integer - for most cases this is your primary key id of a table.


If not specified, the rest of the columns are indexed as fulltext fields. Columns which should be used as attributes need to be declared.
In our example group_id and date_added are attributes:

.. code-block:: none

      sql_attr_uint           = group_id
      sql_attr_timestamp      = date_added

Once we have this setup, we can run the indexing process:

.. code-block:: none

   $ sudo -u manticore  indexer test1  --rotate
   using config file '/etc/sphinxsearch/sphinx.conf'...
   indexing index 'test1'...
   collected 4 docs, 0.0 MB
   sorted 0.0 Mhits, 100.0% done
   total 4 docs, 193 bytes
   total 0.015 sec, 12335 bytes/sec, 255.65 docs/sec
   total 4 reads, 0.000 sec, 8.1 kb/call avg, 0.0 msec/call avg
   total 12 writes, 0.000 sec, 0.1 kb/call avg, 0.0 msec/call avg

Index is created and is ready to be used:

.. code-block:: mysql
   
   mysql> SHOW TABLES;
   +-------+-------------+
   | Index | Type        |
   +-------+-------------+
   | dist1 | distributed |
   | testrt| rt          |
   | test1 | local       |
   +-------+-------------+
   3 rows in set (0.00 sec)
   
   mysql> SELECT * FROM test1;
   +------+----------+------------+
   | id   | group_id | date_added |
   +------+----------+------------+
   |    1 |        1 | 1507904567 |
   |    2 |        1 | 1507904567 |
   |    3 |        2 | 1507904567 |
   |    4 |        2 | 1507904567 |
   +------+----------+------------+
   4 rows in set (0.00 sec)
   
A quick test of a search which should match 2 terms, but not match another one:

.. code-block:: mysql
   
   mysql> SELECT * FROM test1 WHERE MATCH('test document -one');
   +------+----------+------------+-------+
   | id   | group_id | date_added | tag   |
   +------+----------+------------+-------+
   |    2 |        1 | 1519040667 | 2,4,6 |
   +------+----------+------------+-------+
   1 row in set (0.00 sec)

   