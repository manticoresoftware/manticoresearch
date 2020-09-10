# Installing Manticore in Debian or Ubuntu

Supported releases:

* Debian
  * 8.0 (Jessie)
  * 9.0 (Stretch)
  * 10.0 (Buster)

* Ubuntu
  * 14.04 LTS (Trusty)
  * 16.04 LTS (Xenial)
  * 18.04 LTS (Bionic)
  * 20.04 LTS (Focal)

Supported platforms:
* x86
* x86_64

You can install Manticore by adding Manticore's APT repository to your repositories:

```bash
wget https://repo.manticoresearch.com/manticore-repo.noarch.deb
dpkg -i manticore-repo.noarch.deb
apt update
```

and then install Manticore Search with just `apt install`:

```bash
apt install manticore
```

In case you want to install "Nightly" (development) version:
```bash
wget https://repo.manticoresearch.com/manticore-dev-repo.noarch.deb
dpkg -i manticore-dev-repo.noarch.deb
apt update
apt install manticore
```

You can also download individual .deb files from [our site](https://manticoresearch.com/downloads/).

Manticore package depends on zlib and ssl libraries, nothing else is strictly required. However if you plan to use [indexer](Adding_data_from_external_storages/Plain_indexes_creation.md#Indexer-tool) to create indexes from external storages you'll need to install appropriate client libraries. To know what exactly libraries `indexer` requires run it and look at the top of it's output:

```bash
$ sudo -u manticore indexer
Manticore 3.5.0 6903305@200722 release
Copyright (c) 2001-2016, Andrew Aksyonoff
Copyright (c) 2008-2016, Sphinx Technologies Inc (http://sphinxsearch.com)
Copyright (c) 2017-2020, Manticore Software LTD (http://manticoresearch.com)

Built by gcc/clang v 5.4.0,

Built on Linux runner-0277ea0f-project-3858465-concurrent-0 4.19.78-coreos #1 SMP Mon Oct 14 22:56:39 -00 2019 x86_64 x86_64 x86_64 GNU/Linux

Configured by CMake with these definitions: -DCMAKE_BUILD_TYPE=RelWithDebInfo -DDISTR_BUILD=xenial -DUSE_SSL=ON -DDL_UNIXODBC=1 -DUNIXODBC_LIB=libodbc.so.2 -DDL_EXPAT=1 -DEXPAT_LIB=libexpat.so.1 -DUSE_LIBICONV=1 -DDL_MYSQL=1 -DMYSQL_LIB=libmysqlclient.so.20 -DDL_PGSQL=1 -DPGSQL_LIB=libpq.so.5 -DLOCALDATADIR=/var/data -DFULL_SHARE_DIR=/usr/share/manticore -DUSE_ICU=1 -DUSE_BISON=ON -DUSE_FLEX=ON -DUSE_SYSLOG=1 -DWITH_EXPAT=1 -DWITH_ICONV=ON -DWITH_MYSQL=1 -DWITH_ODBC=ON -DWITH_PGSQL=1 -DWITH_RE2=1 -DWITH_STEMMER=1 -DWITH_ZLIB=ON -DGALERA_SOVERSION=31 -DSYSCONFDIR=/etc/manticoresearch
```

Here you can see mentions of **libodbc.so.2**, **libexpat.so.1**, **libmysqlclient.so.20**, and **libpq.so.5**.

Below is a reference table with list of all the client libraries for different Debian/Ubuntu versions:

| Distr | MySQL | PostgreSQL | XMLpipe | UnixODBC |
| - | - | - | - | - |
|  Ubuntu Trusy  |  libmysqlclient.so.18   |  libpq.so.5  | libexpat.so.1 |  libodbc.so.1 |
|  Ubuntu Xenial  |  libmysqlclient.so.20   |  libpq.so.5  | libexpat.so.1 |  libodbc.so.2 |
|  Ubuntu Bionic  |  libmysqlclient.so.20   |  libpq.so.5  | libexpat.so.1 |  libodbc.so.2 |
|  Ubuntu Focal  |  libmysqlclient.so.21   |  libpq.so.5  | libexpat.so.1 |  libodbc.so.2 |
|  Debian Jessie  | libmysqlclient.so.18    |  libpq.so.5  | libexpat.so.1 |  libodbc.so.2 |
|  Debian Stretch  | libmariadbclient.so.18 |  libpq.so.5  | libexpat.so.1 |  libodbc.so.2 |
|  Debian Buster  |  libmariadb.so.3        |  libpq.so.5  | libexpat.so.1 |  libodbc.so.2 |

To find packages that provide the libraries you can use, for example `apt-file`:

```bash
apt-file find libmysqlclient.so.20
libmysqlclient20: /usr/lib/x86_64-linux-gnu/libmysqlclient.so.20
libmysqlclient20: /usr/lib/x86_64-linux-gnu/libmysqlclient.so.20.2.0
libmysqlclient20: /usr/lib/x86_64-linux-gnu/libmysqlclient.so.20.3.6
```

Note that you only need libraries for the types of storages you're going to use. So if you plan to index only from mysql then you might need to install only the MySQL library (in the above case `libmysqlclient20`).

Finally install the needed packages:

```bash
sudo apt-get install libmysqlclient20 libodbc1 libpq5 libexpat1
```

If you aren't going to use `indexer` tool at all, you don't need to find and install any libraries.

To enable CJK tokenization support the official packages contain binaries with embedded ICU library and include ICU data file. They are independent from any ICU runtime library which might be available on your system, and can't be upgraded.

After preparing a configuration file (see [Quick start guide](Quick_start_guide.md)), you can start searchd server:

```bash
systemctl start manticore
```

To enable Manticore at boot:

```bash
systemctl enable manticore
```
