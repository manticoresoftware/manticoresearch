# Installing Manticore in Debian or Ubuntu

### Supported releases:

* Debian
  * 9.0 (Stretch)
  * 10.0 (Buster)
  * 11.0 (Bullseye)

* Ubuntu
  * 16.04 LTS (Xenial)
  * 18.04 LTS (Bionic)
  * 20.04 LTS (Focal)
  * 21.04 (Hirsute Hippo)
  * 22.04 (Ubuntu Jammy)

* Mint
  * 19
  * 20

### APT repository
The easiest way to install Manticore in Ubuntu/Debian/Mint is by using our APT repository

Install the repository:
```bash
wget https://repo.manticoresearch.com/manticore-repo.noarch.deb
sudo dpkg -i manticore-repo.noarch.deb
sudo apt update
```
(install `wget` if it's not installed; install `gnupg2` if `apt-key` fails).

Then install Manticore Search:
```
sudo apt install manticore manticore-extra
```

If you are upgrading to Manticore version 5 or higher it's recommended to remove all the old Manticore packages before you install the new ones since package structure got changed in version 5.0.0:

```bash
sudo apt remove manticore*
```

It won't remove your data and configuration file.

###### Development packages
If you prefer "Nightly" (development) versions do:
```bash
wget https://repo.manticoresearch.com/manticore-dev-repo.noarch.deb
sudo dpkg -i manticore-dev-repo.noarch.deb
sudo apt update
sudo apt install manticore manticore-extra manticore-server-core-dbgsym manticore-tools-dbgsym manticore-columnar-lib-dbgsym
```

### Standalone DEB packages
You can also download individual .deb files from [our site](https://manticoresearch.com/install/).

### More packages you may need
#### For indexer
Manticore package depends on zlib and ssl libraries, nothing else is strictly required. However if you plan to use [indexer](../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) to create tables from external storages you'll need to install appropriate client libraries. To know what exactly libraries `indexer` requires run it and look at the top of it's output:

```bash
$ sudo -u manticore indexer
Manticore 3.5.4 13f8d08d@201211 release
Copyright (c) 2001-2016, Andrew Aksyonoff
Copyright (c) 2008-2016, Sphinx Technologies Inc (http://sphinxsearch.com)
Copyright (c) 2017-2023, Manticore Software LTD (https://manticoresearch.com)

Built by gcc/clang v 5.4.0,

Built on Linux runner-0277ea0f-project-3858465-concurrent-0 4.19.78-coreos #1 SMP Mon Oct 14 22:56:39 -00 2019 x86_64 x86_64 x86_64 GNU/Linux

Configured by CMake with these definitions: -DCMAKE_BUILD_TYPE=RelWithDebInfo -DDISTR_BUILD=xenial -DUSE_SSL=ON -DDL_UNIXODBC=1 -DUNIXODBC_LIB=libodbc.so.2 -DDL_EXPAT=1 -DEXPAT_LIB=libexpat.so.1 -DUSE_LIBICONV=1 -DDL_MYSQL=1 -DMYSQL_LIB=libmysqlclient.so.20 -DDL_PGSQL=1 -DPGSQL_LIB=libpq.so.5 -DLOCALDATADIR=/var/data -DFULL_SHARE_DIR=/usr/share/manticore -DUSE_ICU=1 -DUSE_BISON=ON -DUSE_FLEX=ON -DUSE_SYSLOG=1 -DWITH_EXPAT=1 -DWITH_ICONV=ON -DWITH_MYSQL=1 -DWITH_ODBC=ON -DWITH_POSTGRESQL=1 -DWITH_RE2=1 -DWITH_STEMMER=1 -DWITH_ZLIB=ON -DGALERA_SOVERSION=31 -DSYSCONFDIR=/etc/manticoresearch
```

Here you can see mentions of **libodbc.so.2**, **libexpat.so.1**, **libmysqlclient.so.20**, and **libpq.so.5**.

Below is a reference table with list of all the client libraries for different Debian/Ubuntu versions:

| Distr | MySQL | PostgreSQL | XMLpipe | UnixODBC |
| - | - | - | - | - |
|  Ubuntu Trusy  |  libmysqlclient.so.18   |  libpq.so.5  | libexpat.so.1 |  libodbc.so.1 |
|  Ubuntu Xenial  |  libmysqlclient.so.20   |  libpq.so.5  | libexpat.so.1 |  libodbc.so.2 |
|  Ubuntu Bionic  |  libmysqlclient.so.20   |  libpq.so.5  | libexpat.so.1 |  libodbc.so.2 |
|  Ubuntu Focal  |  libmysqlclient.so.21   |  libpq.so.5  | libexpat.so.1 |  libodbc.so.2 |
|  Ubuntu Hirsute  |  libmysqlclient.so.21   |  libpq.so.5  | libexpat.so.1 |  libodbc.so.2 |
|  Ubuntu Jammy  |  libmysqlclient.so.21   |  libpq.so.5  | libexpat.so.1 |  libodbc.so.2 |
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

Note that you only need libraries for the types of storages you're going to use. So if you plan to build tables only from mysql then you might need to install only the MySQL library (in the above case `libmysqlclient20`).

Finally install the needed packages:

```bash
sudo apt-get install libmysqlclient20 libodbc1 libpq5 libexpat1
```

If you aren't going to use `indexer` tool at all, you don't need to find and install any libraries.

To enable CJK tokenization support, the official packages contain binaries with embedded ICU library and include ICU data file. They are independent from any ICU runtime library which might be available on your system, and can't be upgraded.

#### Ukrainian lemmatizer
The lemmatizer requires Python 3.9+. **Make sure you have it installed and that it's configured with `--enable-shared`.**

Here's how to install Python 3.9 and the Ukrainian lemmatizer on Debian and Ubuntu:

```bash
# install Manticore Search and UK lemmatizer from APT repository
cd ~
wget https://repo.manticoresearch.com/manticore-repo.noarch.deb
sudo dpkg -i manticore-repo.noarch.deb
sudo apt -y update
sudo apt -y install manticore manticore-lemmatizer-uk

# install packages needed for building Python
sudo apt -y update
sudo apt -y install wget build-essential libreadline-dev libncursesw5-dev libssl-dev libsqlite3-dev tk-dev libgdbm-dev libc6-dev libbz2-dev libffi-dev zlib1g-dev

# download, build and install Python 3.9
cd ~
wget https://www.python.org/ftp/python/3.9.4/Python-3.9.4.tgz
tar xzf Python-3.9.4.tgz
cd Python-3.9.4
./configure --enable-optimizations --enable-shared
sudo make -j8 altinstall

# update linker cache
sudo ldconfig

# install pymorphy2 and UK dictionary
sudo pip3.9 install pymorphy2[fast]
sudo pip3.9 install pymorphy2-dicts-uk
```
