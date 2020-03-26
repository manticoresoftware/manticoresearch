Getting started using Docker container
--------------------------------------

Installing and running
~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: bash

    $ docker run --name manticore -p 9306:9306 -d manticoresearch/manticore

or using docker-compose:

.. code-block:: bash

    version: '2.2'
    
    services:
    
      manticore:
        image: manticoresearch/manticore:nodemode
        restart: always
        ulimits:
            nproc: 65535
            nofile:
                soft: 65535
                hard: 65535
            memlock:
                soft: -1
                hard: -1
			
Please find more info on how to get started with Manticore Search docker here https://github.com/manticoresoftware/docker .


Running queries
~~~~~~~~~~~~~~~

The simple way to connect and do some tests is to use the `SphinxQL <http://docs.manticoresearch.com/latest/html/sphinxql_reference.html>`__ protocol. For this, you need a mysql command line client.

While it implements the MySQL protocol, SphinxQL is not 100% compatible with MySQL syntax. There are specific extensions, like MATCH clause [the most powerful thing in Manticore] or WITHIN GROUP BY and many functions available in MySQL are not implemented (or they are dummy just to allow compatibility with MySQL connector e.g.) or JOINs between indexes which are not supported yet.

First, let's connect to Manticore Search and take a look at the available indexes:

.. code-block:: bash 
   
   $ mysql -P9306 -h0

If you don't have the mysql client installed on host, you can use the built-in mysql client shipped in the Manticore image, which connects directly to Manticore:

.. code-block:: bash 
   
   docker exec -it manticore mysql 

.. code-block:: mysql

   mysql> CREATE TABLE testrt (title TEXT, content TEXT, gid INT);
   mysql> SHOW TABLES;
   +-------+-------------+
   | Index | Type        |
   +-------+-------------+
   | testrt| rt          |
   +-------+-------------+
   1 row in set (0.00 sec)

Now let's look at our RT index:

.. code-block:: mysql

   mysql> DESCRIBE testrt;
   +----------+--------+---------------------+
   | Field    | Type   | Properties          |
   +----------+--------+---------------------+
   | id       | bigint |                     |
   | title    | text   | stored, indexed     |
   | content  | text   | stored, indexed     |
   | gid      | uint   |                     |
   +----------+--------+---------------------+
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

   mysql> SELECT * FROM testrt WHERE MATCH('list of laptops');
   +------+------+-------------------------------------+---------------------------+
   | id   | gid  | title                               | content                   |
   +------+------+-------------------------------------+---------------------------+
   |    1 |   10 | List of HP business laptops         | Elitebook Probook         |
   |    2 |   10 | List of Dell business laptops       | Latitude Precision Vostro |
   |    3 |   20 | List of Dell gaming laptops         | Inspirion Alienware       |
   |    5 |   30 | List of ASUS ultrabooks and laptops | Zenbook Vivobook          |
   +------+------+-------------------------------------+---------------------------+
   4 rows in set (0.00 sec)


As you see in the result set we can only get back the doc id and the attributes.
The fulltext fields values are not returned since the text is only indexed, not stored also, and it's impossible to rebuild the original text.

Now let's add some filtering and more ordering:

.. code-block:: mysql
  
   mysql> SELECT *,WEIGHT() FROM testrt WHERE MATCH('list of laptops') AND gid>10  ORDER BY WEIGHT() DESC,gid DESC;
   +------+------+-------------------------------------+---------------------+----------+
   | id   | gid  | title                               | content             | weight() |
   +------+------+-------------------------------------+---------------------+----------+
   |    5 |   30 | List of ASUS ultrabooks and laptops | Zenbook Vivobook    |     2334 |
   |    3 |   20 | List of Dell gaming laptops         | Inspirion Alienware |     2334 |
   +------+------+-------------------------------------+---------------------+----------+
   2 rows in set (0.00 sec)


The WEIGHT() function returns the calculated matching score. If no ordering specified, the result is sorted descending by the score provided by WEIGHT().
In this example we order first by weight and then by an integer attribute.

The search above does a simple matching, where all words need to be present. But we can do more (and this is just a simple example):

.. code-block:: mysql

   mysql> SELECT *,WEIGHT() FROM testrt WHERE MATCH('"list of business laptops"/3');
   +------+------+-------------------------------------+---------------------------+----------+
   | id   | gid  | title                               | content                   | weight() |
   +------+------+-------------------------------------+---------------------------+----------+
   |    1 |   10 | List of HP business laptops         | Elitebook Probook         |     2397 |
   |    2 |   10 | List of Dell business laptops       | Latitude Precision Vostro |     2397 |
   |    3 |   20 | List of Dell gaming laptops         | Inspirion Alienware       |     2375 |
   |    5 |   30 | List of ASUS ultrabooks and laptops | Zenbook Vivobook          |     2375 |
   +------+------+-------------------------------------+---------------------------+----------+
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

More information about how to use Manticore in Docker and parameters that can be passed to containers are found on the `Docker Hub <https://hub.docker.com/r/manticoresearch/manticore/>`_ page.
