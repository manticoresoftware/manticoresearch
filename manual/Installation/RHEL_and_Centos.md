# Installing Manticore packages on RedHat and CentOS

Supported releases:

* CentOS 6 and RHEL 6
* CentOS 7 and RHEL 7
* CentOS 8 and RHEL 8

Supported platforms:
* x86
* x86_64

The easiest way to install Manticore in RedHat/Centos is by using our YUM repository:

Install the repository:
```bash
yum install https://repo.manticoresearch.com/manticore-repo.noarch.rpm
```

Then install Manticore Search:
```bash
yum install manticore
```

If you prefer "Nightly" (development) versions do:

```bash
yum install https://repo.manticoresearch.com/manticore-repo.noarch.rpm
yum --enablerepo manticore-dev install manticore
```

You can also download individual rpm files [from our site](https://manticoresearch.com/downloads/) and install them using tools `rpm` or `yum`.

Apart the package, if you plan to use [indexer](Adding_data_from_external_storages/Plain_indexes_creation.md#Indexer-tool) to create indexes from external sources, you'll need to install corresponding client libraries in order to make available of indexing sources you want. The line below will install all of them at once; feel free to use it as is, or to reduce it to install only libraries you need (for only mysql sources - just `mysql-libs` should be enough, and unixODBC is not necessary).

```bash
yum install mysql-libs postgresql-libs expat unixODBC
```
