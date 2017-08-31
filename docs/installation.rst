Installation
=======================


Installing Manticore packages on Debian and Ubuntu
--------------------------------------------------
Supported releases:

*  Debian

	* 7.0 (wheezy)
	* 8.0 (jessie)
	
*  Ubuntu

	* 14.04 LTS (trusty)
	* 16.05 LTS (xenial)

Supported platforms:

* x86
* x86_64	
	
Manticore requires a few libraries to be installed on Debian/Ubuntu. Use    apt-get to download and install these dependencies:

::

	$ sudo apt-get install mysql-client unixodbc libpq5
		
Now you can install Manticore:

::

	$ wget https://github.com/manticoresoftware/manticore/releases/download/2.3.3-170706/manticore_2.3.3-170706-9fbdd5f-1.jessie_amd64.deb
	$ sudo dpkg -i manticore_2.3.3-170706-9fbdd5f-1.jessie_amd64.deb

After preparing configuration file (see :ref:`Quick tour <quick_usage_tour>`), you can start searchd daemon:

::
  
	$ systemctl manticore start
		

Installing Manticore packages on RedHat and CentOS
--------------------------------------------------

Supported releases:

* CentOS 6 and RHEL 6
* CentOS 7 and RHEL 7

Supported platforms:

* x86
* x86_64	
	
Before installation make sure you have these packages installed:

::
   
	$ yum install mysql-client postgresql-libs unixODBC

Download RedHat RPM from Manticore website and install it:

::
		
	$ wget https://github.com/manticoresoftware/manticore/releases/download/2.3.3-170706/manticore-2.3.3.170706.9fbdd5f-1rhel7.x86_64.rpm
	$ rpm -Uhv manticore-2.3.3.170706.9fbdd5f-1rhel7.x86_64.rpm

After preparing configuration file (see :ref:`Quick    tour <quick_usage_tour>`), you can start searchd   daemon:
   
::
	
	$ systemctl searchd start

Installing Manticore on Windows
-------------------------------

To install on Windows, you need to download the zip package and unpack it first.

::
	
	cd C:\Manticore
	unzip manticore-2.3.3-170706-9fbdd5f-dev-pgsql-stemmer-x64-bin.zip


Edit the contents of sphinx.conf.in - specifically entries relating
   to @CONFDIR@ - to paths suitable for your system.

Install the ``searchd`` system as a Windows service:

::
   
	C:\Manticore\bin> C:\Manticore\bin\searchd --install --config C:\Manticore\sphinx.conf.in --servicename Manticore
   

4. The ``searchd`` service will now be listed in the Services panel
   within the Management Console, available from Administrative Tools.
   It will not have been started, as you will need to configure it and
   build your indexes with ``indexer`` before starting the service. A
   guide to do this can be found under :ref:`Quick
   tour <quick_usage_tour>`.


Compiling Manticore from source
-------------------------------

.. _Required tools:

Required tools
~~~~~~~~~~~~~~

* a working compiler

	* on Linux - GNU gcc (4.7.2 and above) and clang can be used
	* on Windows - Microsoft Visual Studio  (community version is enough)
	* on Mac OS - XCode

* cmake - used on all plaftorms 
	
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

::

   $ git clone https://github.com/manticoresoftware/manticore.git

::

   $ wget https://github.com/manticoresoftware/manticore/releases/download/2.3.3-170706/manticore-2.3.3.170706.9fbdd5f.tar.gz
   $ tar zcvf manticore-2.3.3.170706.9fbdd5f.tar.gz
	   

Manticore uses cmake for building sources. We recommend using a folder outside the sources for the building workspace to keep clean the source folders.

::

   $ mkdir build
   $ cd build

Next step is to configure the building with cmake. Available list of configuration options:


* ``CMAKE_BUILD_TYPE`` -  can be Debug , Release , MinSizeRel and RelWithDebInfo (default).
* ``SPLIT_SYMBOLS`` (bool) - specify whenever to create separate files with debugging symbols. In the default build type,RelWithDebInfo, the binaries include the debug symbols. With this option specified, the binaries will be stripped of the debug symbols , which will be put in separate files
* ``USE_BISON,USE_FLEX`` (bool)  - enabled by default, specifies whenever to enable bison and flex tools
* ``LIBS_BUNDLE`` - filepath to a folder with different libraries. This is mostly relevant for Windows building
* ``WITH_STEMMER`` (bool) - specifies if the build should include the libstemmer library. The library is searched in several places, starting with 

	* libstemmer_c folder in the source directory
	* common system path. Please note that in this case, the linking is dynamic and libstemmer should be available system-wide on the installed systems
	* libstemmer_c.tgz in  ``LIBS_BUNDLE`` folder.
	* download from snowball project website. This is done by cmake and no additional tool is required
	
* ``WITH_RE2`` (bool) - specifies if the build should include the RE2 library. The library can be taken from the following locations:

	* in the folder specified by ``WITH_RE2_ROOT`` parameters
	* in libre2 folder of the Manticore sources
	* system wide search, while first looking for headers specified by ``WITH_RE2_INCLUDES`` folder and the lib files in ``WITH_RE2_LIBS`` folder
	* check presence of master.zip in the ``LIBS_BUNDLE`` folder 
	* Download from https://github.com/manticoresoftware/re2/archive/master.zip
	
* ``WITH_EXPAT`` (bool)	 enabled compiling with libexpat, used XMLpipe source driver
* ``WITH_MYSQL`` (bool)	 enabled compiling with MySQL client library, used by MySQL source driver. Additional parameters ``WITH_MYSQL_ROOT``, ``WITH_MYSQL_LIBS`` and ``WITH_MYSQL_INCLUDES`` can be used for custom MySQL files
* ``WITH_ODBC`` (bool)	 enabled compiling with ODBC client library, used by ODBC source driver
* ``WITH_PGSQL`` (bool)	 enabled compiling with PostgreSQL client library, used by PostgreSQL source driver
* ``DISTR_BUILD``  -  in case the target is packaging, it specifies the target operating system. Supported values are: centos6, centos7, wheezy, jessie, trusty, xenial, macos

Compiling on UNIX systems
~~~~~~~~~~~~~~~~~~~~~~~~~


To install all dependencies on Debian/Ubuntu:

.. code-block:: bash

   $ apt-get install build-essential cmake unixodbc-dev libpq-dev libexpat-dev libmysqlclient-dev git flex bison

To install all dependencies on CentOS/RHEL:

.. code-block:: bash
   
   $ yum install gcc gcc-c++ make cmake mysql-devel expat-devel postgresql-devel unixODBC-devel rpm-build systemd-units git flex bison
   
RHEL/CentOS 6  ship with a old version of the gcc compiler, which doesn't support `std-c++` standard, for compiling use `devtools` repository: 

.. code-block:: bash

  $ wget http://people.centos.org/tru/devtools-2/devtools-2.repo -O /etc/yum.repos.d/devtools-2.repo
  $ yum upgrade -y
  $ yum install -y devtoolset-2-gcc devtoolset-2-binutils devtoolset-2-gcc-c++
  $ export PATH=/opt/rh/devtoolset-2/root/usr/bin:$PATH
  
We recommend using a separate folder for building instead of compiling in the source folder:

::

   $ mkdir build
   $ cd build
   $ cmake -D WITH_MYSQL=TRUE -DWITH_RE2=1 ../manticore
	   
or if we use sources from tarball:

::
   
   $ cmake -D WITH_MYSQL=TRUE -DWITH_RE2=1 ../manticore-2.3.3.170706.9fbdd5f

To simply compile:

::
   
   $ make -j4
	   
	   
This will create the binary files, however we want to either install Manticore or more convenient to create a package.
To install just do 

::
   
   $ make -j4 install

For packaging use ``package``

::
   
   $ make -j4 package
		

By default, if no operating system was targeted, ``package`` will create only a zip with the binaries.
If, for example, we want to create a deb package for Debian Jessie, we need to specify to cmake the ``DISTR_BUILD`` parameter:

::
   
   $ cmake -D WITH_MYSQL=TRUE -DWITH_RE2=1  -DDISTR_BUILD=jessie../manticore
   $ make -j4 package	   

This will create 2 deb packages, a manticore-x.x.x-bin.deb and a manticore-x.x.x-dbg.deb which contains the version with debug symbols.
Another possible target is ``tarball`` , which create a tar.gz file from the sources.


Compiling on Windows
~~~~~~~~~~~~~~~~~~~~
For building on Windows you need:

* Visual Studio
* Cmake for Windows
* Expat, MySQL and PostgreSQL in bundle directory.

For a simple building on x64:

::
	   
   C:\build>"%PROGRAMW6432%\CMake\bin\cmake.exe" -G "Visual Studio 14 Win64" -DLIBS_BUNDLE="C:\bundle" -DDISTR=none -DCPACK_GENERATOR=ZIP "C:\manticore"
   C:\build>"%PROGRAMW6432%\CMake\bin\cmake.exe" -DWITH_PGSQL=1 -DWITH_RE2=1 -DWITH_STEMMER=1 .
   C:\build>"%PROGRAMW6432%\CMake\bin\cmake.exe" --build . --target package --config RelWithDebInfo
	   

.. _quick_usage_tour:
   
Quick Manticore usage tour
--------------------------
We are going to use SphinxQL protocol as it's the current recommended way and it's also easy to play with. First we connect to Manticore with the normal MySQL client:

::

    $ mysql -h0 -P9306

The default configuration comes with a sample Real-Time. A first step to see it in action is to add several documents to it, then you can start perform searches:
	
::

    mysql> INSERT INTO rt VALUES (1, 'this is', 'a sample text', 11);
	Query OK, 1 row affected (0.00 sec)

    mysql> INSERT INTO rt VALUES (2, 'some more', 'text here', 22);
	Query OK, 1 row affected (0.00 sec)

	mysql> INSERT INTO rt VALUES (3, 'more about this text', 'can be found in this text', 22);
	Query OK, 1 row affected (0.00 sec)


::

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

::
	
	mysql> create database test;
	$ mysql -u test <  /usr/share/doc/manticore/example-conf/example.sql

The sample config uses a ``test`` with no password for connecting to MySQL. Adjust the credentials, then index:

::

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

::

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


::

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


::

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


::

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


::

    mysql>  SELECT COUNT(*) c, id%3 idd FROM test1 GROUP BY idd HAVING COUNT(*)>1;
	+------+------+
	| c    | idd  |
	+------+------+
	|    2 |    1 |
	+------+------+
	1 row in set (0.00 sec)

::

    mysql>  SELECT COUNT(*) FROM test1;
	+----------+
	| count(*) |
	+----------+
	|        4 |
	+----------+
	1 row in set (0.00 sec)

::

	mysql>   CALL KEYWORDS ('one two three', 'test1', 1);
	+------+-----------+------------+------+------+
	| qpos | tokenized | normalized | docs | hits |
	+------+-----------+------------+------+------+
	| 1    | one       | one        | 1    | 2    |
	| 2    | two       | two        | 1    | 2    |
	| 3    | three     | three      | 0    | 0    |
	+------+-----------+------------+------+------+
	3 rows in set (0.00 sec)

