# Manticore Search

## Introduction

Manticore Search is an open source search server designed to be fast, scalable and with powerful and accurate full-text search capabilities. It is a fork of popular search engine Sphinx.

## Features
* Over 20 full-text operators and over 20 ranking factors, custom rankers
* Advanced tokenization at character and word level
* Morphology preprocessors
* Real-time and offline indexes
* JSON attributes
* Distributed indexes
* Built-in load balancer
* Text highlighting and word correction
* Geo search
* Source data connectors for MySQL, PostgreSQL, MSSQL, ODBC, XML and CSV files
* Connectivity: MySQL-based protocol, HTTP, native libraries


## Download and installation
Packages of latest GA release can be downloaded from http://www.manticoresearch.com/downloads

    $ wget https://github.com/manticoresoftware/manticore/releases/download/x.y.z/manticore_z.y.z.deb
    $ sudo dpkg -i manticore_x.y.z.deb
    $ systemctl manticore start

For more details see [installation](http://docs.manticoresearch.com/latest/html/installation.html#installation).

### Compiling from sources
If you like to use the latest code, you can build Manticore easy from sources:

     git clone https://github.com/manticoresoftware/manticore.git
     cd manticore
     mkdir build && cd build
     cmake .. &&  make install

For more details check [compiling] (http://docs.manticoresearch.com/latest/html/installation.html#compiling-manticore-from-source). 
## Getting started
 The quick way to see Manticore in action is to use the sample RT index preconfigured in sphinx.conf.
 Inserting data in a RT index is done using SphinxQL and in the same way as for a MySQL table
 
      $ mysql -h 127.0.0.1 -P 9306
      mysql> INSERT INTO rt VALUES ( 1, 'first record', 'test one', 123 );
      Query OK, 1 row affected (0.05 sec)
      mysql> INSERT INTO rt VALUES ( 2, 'second record', 'test two', 234 );
      Query OK, 1 row affected (0.00 sec)
 To search we use a SELECT statement with a MATCH clause:
 
     mysql> SELECT * FROM rt WHERE MATCH('test');
     +------+--------+------+
     | id   | weight | gid  |
     +------+--------+------+
     |    1 |   1643 |  123 |
     |    2 |   1643 |  234 |
     +------+--------+------+
     2 rows in set (0.01 sec)
    
## Documentation and support

[Documentation](http://docs.manticoresearch.com)

[Manticore Community Forum](http://forum.manticoresearch.com/)

[Bug tracker](https://github.com/manticoresoftware/manticore/issues)

[Professional Support](http://www.manticoresearch.com/support)

## License
Manticore Search is release under GPLv2, for more information check [COPYING](https://github.com/manticoresoftware/manticore/blob/master/COPYING)

