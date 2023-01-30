# Installing Manticore packages on RedHat and CentOS

### Supported releases:

* CentOS 7 and RHEL 7
* CentOS 8 and RHEL 8
* CentOS Stream 8
* Amazon Linux 2

### YUM repository

The easiest way to install Manticore in RedHat/Centos is by using our YUM repository:

Install the repository:
```bash
sudo yum install https://repo.manticoresearch.com/manticore-repo.noarch.rpm
```

Then install Manticore Search:
```bash
sudo yum install manticore manticore-extra
```

If you are upgrading to Manticore version 5 or higher it's recommended to remove all the old Manticore packages before you install the new ones since package structure got changed in version 5.0.0:

```bash
sudo yum remove manticore*
```

It won't remove your data and configuration file.

###### Development packages
If you prefer "Nightly" (development) versions do:

```bash
sudo yum install https://repo.manticoresearch.com/manticore-repo.noarch.rpm
sudo yum --enablerepo manticore-dev install manticore manticore-columnar-lib manticore-server-core-debuginfo manticore-tools-debuginfo manticore-columnar-lib-debuginfo
```

### Standalone RPM packages
You can also download standalone rpm files [from our site](https://manticoresearch.com/install/) and install them using tools `rpm` or `yum`.

### More packages you may need
#### For indexer
If you plan to use [indexer](../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) to create tables from external sources, you'll need to make sure you have installed corresponding client libraries in order to make available of indexing sources you want. The line below will install all of them at once; feel free to use it as is, or to reduce it to install only libraries you need (for only mysql sources - just `mysql-libs` should be enough, and unixODBC is not necessary).

```bash
sudo yum install mysql-libs postgresql-libs expat unixODBC
```

In CentOS Stream 8 you may need to run:

```
dnf install mariadb-connector-c
```

if you get error `sql_connect: MySQL source wasn't initialized. Wrong name in dlopen?` trying to build a plain table from MySQL.

#### Ukrainian lemmatizer
The lemmatizer requires Python 3.9+. **Make sure you have it installed and that it's configured with `--enable-shared`.**

Here's how to install Python 3.9 and the Ukrainian lemmatizer in Centos 7/8:

```bash
# install Manticore Search and UK lemmatizer from YUM repository
yum -y install https://repo.manticoresearch.com/manticore-repo.noarch.rpm
yum -y install manticore manticore-lemmatizer-uk

# install packages needed for building Python
yum groupinstall "Development Tools" -y
yum install openssl-devel libffi-devel bzip2-devel wget -y

# download, build and install Python 3.9
cd ~
wget https://www.python.org/ftp/python/3.9.2/Python-3.9.2.tgz
tar xvf Python-3.9.2.tgz
cd Python-3.9*/
./configure --enable-optimizations --enable-shared
make -j8 altinstall

# update linker cache
ldconfig

# install pymorphy2 and UK dictionary
pip3.9 install pymorphy2[fast]
pip3.9 install pymorphy2-dicts-uk
```

After you have all installed make sure you have the following in your Manticore Search configuration file (`/etc/manticoresearch/manticore.conf` by default):

```
common {
    plugin_dir = /usr/local/manticore/lib/
}
```
