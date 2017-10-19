Installation
=======================


Installing Manticore packages on Debian and Ubuntu
--------------------------------------------------
Supported releases:

*  Debian

	* 7.0 (wheezy)
	* 8.0 (jessie)
	* 9.0 (stretch)
	
*  Ubuntu

	* 14.04 LTS (trusty)
	* 16.05 LTS (xenial)

Supported platforms:

* x86
* x86_64

Manticore requires no extra libraries to be installed on Debian/Ubuntu.
However if you plan to use 'indexer' tool to create indexes from different sources,
you'll need to install appropriate client libraries. Use apt-get to download and install these dependencies:

.. code-block:: bash

	$ sudo apt-get install libmysqlclient20 libodbc1 libpq5 libexpat1

Note, that you need only libs for types of sources you're going to use. So if you plan to make indexes only
from mysql source, then installing 'libmysqlclient20' will be enough.
If you don't going to use 'indexer' tool at all, you don't need to install these packages.
Now you can install Manticore:

.. code-block:: bash

	$ wget https://github.com/manticoresoftware/manticore/releases/download/2.4.1/manticore_2.4.1-171017-3b31a97-release-stemmer.jessie_amd64-bin.deb
	$ sudo dpkg -i manticore_2.4.1-171017-3b31a97-release-stemmer.jessie_amd64-bin.deb

After preparing configuration file (see :ref:`Quick tour <quick_usage_tour>`), you can start searchd daemon:

.. code-block:: bash

	$ systemctl manticore start


Installing Manticore packages on RedHat and CentOS
--------------------------------------------------

Supported releases:

* CentOS 6 and RHEL 6
* CentOS 7 and RHEL 7

Supported platforms:

* x86
* x86_64

Manticore requires no extra libraries to be installed on RedHat/CentOS.
However if you plan to use 'indexer' tool to create indexes from different sources,
you'll need to install appropriate client libraries. Use yum to download and install these dependencies:

.. code-block:: bash

	$ yum install mysql-libs postgresql-libs expat unixODBC

Note, that you need only libs for types of sources you're going to use. So if you plan to make indexes only
from mysql source, then installing 'mysql-libs' will be enough.
If you don't going to use 'indexer' tool at all, you don't need to install these packages.
Download RedHat RPM from Manticore website and install it:

.. code-block:: bash

	$ wget https://github.com/manticoresoftware/manticore/releases/download/2.4.1/manticore-2.4.1-171017-3b31a97-release-stemmer-rhel7-bin.rpm
	$ rpm -Uhv manticore-2.4.1-171017-3b31a97-release-stemmer-rhel7-bin.rpm

After preparing configuration file (see :ref:`Quick tour <quick_usage_tour>`), you can start searchd daemon:

.. code-block:: bash

	$ systemctl searchd start

Installing Manticore on Windows
-------------------------------

To install on Windows, you need to download the zip package and unpack it first.

.. code-block:: bash
	
	cd C:\Manticore
	unzip manticore-2.4.1-171017-3b31a97-release-pgsql-stemmer-x64-bin.zip


Edit the contents of sphinx.conf.in - specifically entries relating to @CONFDIR@ - to paths suitable for your system.

Install the ``searchd`` system as a Windows service:

.. code-block:: bat

	C:\Manticore\bin> C:\Manticore\bin\searchd --install --config C:\Manticore\sphinx.conf.in --servicename Manticore


4. The ``searchd`` service will now be listed in the Services panel
   within the Management Console, available from Administrative Tools.
   It will not have been started, as you will need to configure it and
   build your indexes with ``indexer`` before starting the service. A
   guide to do this can be found under :ref:`Quick tour <quick_usage_tour>`.


Compiling Manticore from source
-------------------------------

.. _Required tools:

Required tools
~~~~~~~~~~~~~~

* a working compiler

	* on Linux - GNU gcc (4.7.2 and above) or clang can be used
	* on Windows - Microsoft Visual Studio 2015 (community edition is enough)
	* on Mac OS - XCode

* cmake - used on all plaftorms (version 2.8 or above)

Optional dependencies
~~~~~~~~~~~~~~~~~~~~~
* git, flex, bison -  needed if the sources are from cloned repository and not the source tarball
* development version of MySQL client for  MySQL source driver
* development version of unixODBC for the unixODBC source driver
* development version of libPQ for the PostgreSQL source driver
* development version of libexpat for the XMLpipe source driver
* RE2 (bundled in the source tarball) for :ref:`regexp_filter` feature
* lib stemmer (bundled in the source tarball ) for additional language stemmers 

General building options
~~~~~~~~~~~~~~~~~~~~~~~~

For compiling latest version of Manticore, recommended is checkout the latest code from the github repositiory.
Alternative, for compiling a certain version, you can either checked that version from github or use it's respective source tarball.
In last case avoid to use automatic tarballs from github (named there as 'Source code'), but use provided files as `manticore-2.4.1-171017-3b31a97-release.tar.gz`.
When building from clone you need packages `git`, `flex`, `bison`. When building from tarball they are not necessary. This requirement
may be essential to build on Windows.

.. code-block:: bash

   $ git clone https://github.com/manticoresoftware/manticore.git

.. code-block:: bash

   $ wget https://github.com/manticoresoftware/manticore/releases/download/2.4.1/manticore-2.4.1-171017-3b31a97-release.tar.gz
   $ tar zcvf manticore-2.4.1-171017-3b31a97-release.tar.gz

Next step is to configure the building with cmake. Available list of configuration options:


* ``CMAKE_BUILD_TYPE`` -  can be Debug , Release , MinSizeRel and RelWithDebInfo (default).
* ``SPLIT_SYMBOLS`` (bool) - specify whenever to create separate files with debugging symbols. In the default build type,RelWithDebInfo, the binaries include the debug symbols. With this option specified, the binaries will be stripped of the debug symbols , which will be put in separate files
* ``USE_BISON``,``USE_FLEX`` (bool)  - enabled by default, specifies whenever to enable bison and flex tools
* ``LIBS_BUNDLE`` - filepath to a folder with different libraries. This is mostly relevant for Windows building
* ``WITH_STEMMER`` (bool) - specifies if the build should include the libstemmer library. The library is searched in several places, starting with 

	* libstemmer_c folder in the source directory
	* common system path. Please note that in this case, the linking is dynamic and libstemmer should be available system-wide on the installed systems
	* libstemmer_c.tgz in  ``LIBS_BUNDLE`` folder.
	* download from snowball project website. This is done by cmake and no additional tool is required
	* NOTE: if you have libstemmer in the system, but still want to use static version, say, to build a binary for a system without such lib, provide ``WITH_STEMMER_FORCE_STATIC=1`` in advance.
	
* ``WITH_RE2`` (bool) - specifies if the build should include the RE2 library. The library can be taken from the following locations:

	* in the folder specified by ``WITH_RE2_ROOT`` parameters
	* in libre2 folder of the Manticore sources
	* system wide search, while first looking for headers specified by ``WITH_RE2_INCLUDES`` folder and the lib files in ``WITH_RE2_LIBS`` folder
	* check presence of master.zip in the ``LIBS_BUNDLE`` folder 
	* Download from https://github.com/manticoresoftware/re2/archive/master.zip
	* NOTE: if you have RE2 in the system, but still want to use static version, say, to build a binary for a system without such lib, provide ``WITH_RE2_FORCE_STATIC=1`` in advance.
	
* ``WITH_EXPAT`` (bool)	 enabled compiling with libexpat, used XMLpipe source driver
* ``WITH_MYSQL`` (bool)	 enabled compiling with MySQL client library, used by MySQL source driver. Additional parameters ``WITH_MYSQL_ROOT``, ``WITH_MYSQL_LIBS`` and ``WITH_MYSQL_INCLUDES`` can be used for custom MySQL files
* ``WITH_ODBC`` (bool)	 enabled compiling with ODBC client library, used by ODBC source driver
* ``WITH_PGSQL`` (bool)	 enabled compiling with PostgreSQL client library, used by PostgreSQL source driver
* ``DISTR_BUILD``  -  in case the target is packaging, it specifies the target operating system. Supported values are: `centos6`, `centos7`, `wheezy`, `jessie`, `stretch`, `trusty`, `xenial`, `macos`, `default`.

Compiling on UNIX systems
~~~~~~~~~~~~~~~~~~~~~~~~~


To install all dependencies on Debian/Ubuntu:

.. code-block:: bash

   $ apt-get install build-essential cmake unixodbc-dev libpq-dev libexpat-dev libmysqlclient-dev git flex bison

To install all dependencies on CentOS/RHEL:

.. code-block:: bash

   $ yum install gcc gcc-c++ make cmake mysql-devel expat-devel postgresql-devel unixODBC-devel rpm-build systemd-units git flex bison

(git, flex, bison doesn't necessary if you build from tarball)

RHEL/CentOS 6  ship with a old version of the gcc compiler, which doesn't support `-std=c++11` flag, for compiling use `devtools` repository:

.. code-block:: bash

   $ wget http://people.centos.org/tru/devtools-2/devtools-2.repo -O /etc/yum.repos.d/devtools-2.repo
   $ yum upgrade -y
   $ yum install -y devtoolset-2-gcc devtoolset-2-binutils devtoolset-2-gcc-c++
   $ export PATH=/opt/rh/devtoolset-2/root/usr/bin:$PATH

Manticore uses `cmake` for building. We recommend to use a folder outside the sources to keep them clean.

.. code-block:: bash

   $ mkdir build
   $ cd build
   $ cmake -D WITH_MYSQL=1 -DWITH_RE2=1 ../manticore

or if we use sources from tarball:

.. code-block:: bash

   $ cmake -D WITH_MYSQL=1 -DWITH_RE2=1 ../manticore-2.4.1-171017-3b31a97-release

To simply compile:

.. code-block:: bash

   $ make -j4


This will create the binary files, however we want to either install Manticore or more convenient to create a package.
To install just do 

.. code-block:: bash

   $ make -j4 install

For packaging use ``package``

.. code-block:: bash

   $ make -j4 package


By default, if no operating system was targeted, ``package`` will create only a zip with the binaries.
If, for example, we want to create a deb package for Debian Jessie, we need to specify to cmake the ``DISTR_BUILD`` parameter:

.. code-block:: bash

   $ cmake -DDISTR_BUILD=jessie ../manticore
   $ make -j4 package	   

This will create 2 deb packages, a manticore-x.x.x-bin.deb and a manticore-x.x.x-dbg.deb which contains the version with debug symbols.
Another possible target is ``tarball`` , which create a tar.gz file from the sources.


Compiling on Windows
~~~~~~~~~~~~~~~~~~~~
For building on Windows you need:

* Visual Studio
* Cmake for Windows
* Expat, MySQL and PostgreSQL in bundle directory.

If you build from git clone, you also need to provide `git`, `flex`, `bison` tools. They may be fond in `cygwin` framework.
When building from tarball these tools are not necessary.

For a simple building on x64:

.. code-block:: bat

   C:\build>"%PROGRAMW6432%\CMake\bin\cmake.exe" -G "Visual Studio 14 Win64" -DLIBS_BUNDLE="C:\bundle" "C:\manticore"
   C:\build>"%PROGRAMW6432%\CMake\bin\cmake.exe" -DWITH_PGSQL=1 -DWITH_RE2=1 -DWITH_STEMMER=1 .
   C:\build>"%PROGRAMW6432%\CMake\bin\cmake.exe" --build . --target package --config RelWithDebInfo


.. _quick_usage_tour:

Quick Manticore usage tour
--------------------------
We are going to use SphinxQL protocol as it's the current recommended way and it's also easy to play with. First we connect to Manticore with the normal MySQL client:

.. code-block:: bash

    $ mysql -h0 -P9306

The default configuration comes with a sample Real-Time. A first step to see it in action is to add several documents to it, then you can start perform searches:
	
.. code-block:: bash

    mysql> INSERT INTO rt VALUES (1, 'this is', 'a sample text', 11);
	Query OK, 1 row affected (0.00 sec)

    mysql> INSERT INTO rt VALUES (2, 'some more', 'text here', 22);
	Query OK, 1 row affected (0.00 sec)

	mysql> INSERT INTO rt VALUES (3, 'more about this text', 'can be found in this text', 22);
	Query OK, 1 row affected (0.00 sec)


.. code-block:: mysql

    mysql> SELECT *,weight() FROM rt  WHERE MATCH('text') ORDER BY WEIGHT() DESC;
	+------+------+----------+
	| id   | gid  | weight() |
	+------+------+----------+
	|    3 |   22 |     2252 |
	|    1 |   11 |     1319 |
	|    2 |   22 |     1319 |
	+------+------+----------+
	3 rows in set (0.00 sec)


In the sample configuration there is also a plain index with MySQL source, which needs to be indexed first in order to start using it.
First, we populate the sample table in MySQL:

.. code-block:: bash

	mysql> create database test;
	$ mysql -u test <  /usr/share/doc/manticore/example-conf/example.sql

The sample config uses a ``test`` with no password for connecting to MySQL. Adjust the credentials, then index:

.. code-block:: bash

	$ sudo -u manticore indexer -c /etc/sphinxsearch/sphinx.conf test1 --rotate
	Manticore 2.3.3 9b7033e@170806 master...origin/master-id64-dev
	Copyright (c) 2001-2016, Andrew Aksyonoff
	Copyright (c) 2008-2016, Sphinx Technologies Inc (http://sphinxsearch.com)
	Copyright (c) 2017, Manticore Software LTD (http://manticoresearch.com)

	using config file '/etc/sphinxsearch/sphinx.conf'...
	indexing index 'test1'...
	collected 4 docs, 0.0 MB
	sorted 0.0 Mhits, 100.0% done
	total 4 docs, 193 bytes
	total 0.002 sec, 81503 bytes/sec, 1689.18 docs/sec
	total 4 reads, 0.000 sec, 8.1 kb/call avg, 0.0 msec/call avg
	total 12 writes, 0.000 sec, 0.1 kb/call avg, 0.0 msec/call avg
	rotating indices: successfully sent SIGHUP to searchd (pid=2947).

Now let's run several queries:	

.. code-block:: mysql

	mysql> SELECT *, WEIGHT() FROM test1 WHERE MATCH('"document one"/1');SHOW META;
	+------+----------+------------+----------+
	| id   | group_id | date_added | weight() |
	+------+----------+------------+----------+
	|    1 |        1 | 1502280778 |     2663 |
	|    2 |        1 | 1502280778 |     1528 |
	+------+----------+------------+----------+
	2 rows in set (0.00 sec)

	+---------------+----------+
	| Variable_name | Value    |
	+---------------+----------+
	| total         | 2        |
	| total_found   | 2        |
	| time          | 0.000    |
	| keyword[0]    | document |
	| docs[0]       | 2        |
	| hits[0]       | 2        |
	| keyword[1]    | one      |
	| docs[1]       | 1        |
	| hits[1]       | 2        |
	+---------------+----------+
	9 rows in set (0.00 sec)


.. code-block:: mysql

	mysql>  SET profiling=1;SELECT * FROM test1 WHERE id IN (1,2,4);SHOW PROFILE;
	Query OK, 0 rows affected (0.00 sec)
	
	+------+----------+------------+
	| id   | group_id | date_added |
	+------+----------+------------+
	|    1 |        1 | 1502280778 |
	|    2 |        1 | 1502280778 |
	|    4 |        2 | 1502280778 |
	+------+----------+------------+
	3 rows in set (0.00 sec)
	
	+--------------+----------+----------+---------+
	| Status       | Duration | Switches | Percent |
	+--------------+----------+----------+---------+
	| unknown      | 0.000059 | 4        | 44.70   |
	| net_read     | 0.000001 | 1        | 0.76    |
	| local_search | 0.000042 | 1        | 31.82   |
	| sql_parse    | 0.000012 | 1        | 9.09    |
	| fullscan     | 0.000001 | 1        | 0.76    |
	| finalize     | 0.000007 | 1        | 5.30    |
	| aggregate    | 0.000006 | 2        | 4.55    |
	| net_write    | 0.000004 | 1        | 3.03    |
	| eval_post    | 0.000000 | 1        | 0.00    |
	| total        | 0.000132 | 13       | 0       |
	+--------------+----------+----------+---------+
	10 rows in set (0.00 sec)


.. code-block:: mysql

	mysql> SELECT id, id%3 idd FROM test1 WHERE MATCH('this is | nothing') GROUP BY idd;SHOW PROFILE;
	+------+------+
	| id   | idd  |
	+------+------+
	|    1 |    1 |
	|    2 |    2 |
	|    3 |    0 |
	+------+------+
	3 rows in set (0.00 sec)
	
	+--------+----------+----------+---------+
	| Status | Duration | Switches | Percent |
	+--------+----------+----------+---------+
	| total  | 0.000000 | 0        | 0       |
	+--------+----------+----------+---------+
	1 row in set (0.00 sec)


.. code-block:: none

	mysql> SELECT id FROM test1 WHERE MATCH('is this a good plan?');SHOW PLAN\G
	Empty set (0.00 sec)
	
	*************************** 1. row ***************************
	Variable: transformed_tree
		Value: AND(
		AND(KEYWORD(is, querypos=1)),
		AND(KEYWORD(this, querypos=2)),
		AND(KEYWORD(a, querypos=3)),
		AND(KEYWORD(good, querypos=4)),
		AND(KEYWORD(plan, querypos=5)))
	1 row in set (0.00 sec)


.. code-block:: mysql

    mysql>  SELECT COUNT(*) c, id%3 idd FROM test1 GROUP BY idd HAVING COUNT(*)>1;
	+------+------+
	| c    | idd  |
	+------+------+
	|    2 |    1 |
	+------+------+
	1 row in set (0.00 sec)

.. code-block:: mysql

    mysql>  SELECT COUNT(*) FROM test1;
	+----------+
	| count(*) |
	+----------+
	|        4 |
	+----------+
	1 row in set (0.00 sec)

.. code-block:: mysql

	mysql>   CALL KEYWORDS ('one two three', 'test1', 1);
	+------+-----------+------------+------+------+
	| qpos | tokenized | normalized | docs | hits |
	+------+-----------+------------+------+------+
	| 1    | one       | one        | 1    | 2    |
	| 2    | two       | two        | 1    | 2    |
	| 3    | three     | three      | 0    | 0    |
	+------+-----------+------------+------+------+
	3 rows in set (0.00 sec)

