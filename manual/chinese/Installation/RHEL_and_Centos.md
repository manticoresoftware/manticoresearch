# 在 RedHat 和 CentOS 上安装 Manticore 软件包

### 支持的版本：

* CentOS 8，RHEL 8，Oracle Linux 8，CentOS Stream 8
* Amazon Linux 2
* CentOS 9，RHEL 9，AlmaLinux 9
* AlmaLinux 10，其他基于 RHEL 10 的发行版

### YUM 仓库

在 RedHat/CentOS 上安装 Manticore 最简单的方法是使用我们的 YUM 仓库：

安装仓库：
```bash
sudo yum install https://repo.manticoresearch.com/manticore-repo.noarch.rpm
```

然后安装 Manticore Search：
```bash
sudo yum install manticore manticore-extra
```

如果您是从旧版本升级到 Manticore 6，建议先删除旧的软件包，以避免因软件包结构更新而导致的冲突：

```bash
sudo yum remove manticore*
```

这不会删除您的数据和配置文件。

###### 开发包
如果您更喜欢“Nightly”（开发）版本，请执行：

```bash
sudo yum -y install https://repo.manticoresearch.com/manticore-repo.noarch.rpm && \
sudo yum -y --enablerepo manticore-dev install manticore manticore-extra manticore-common manticore-server manticore-server-core manticore-tools manticore-executor manticore-buddy manticore-backup manticore-columnar-lib manticore-server-core-debuginfo manticore-tools-debuginfo manticore-columnar-lib-debuginfo  manticore-icudata manticore-galera manticore-galera-debuginfo manticore-language-packs manticore-load
```

### 独立 RPM 软件包
要从 Manticore 仓库下载独立的 RPM 文件，请按照 https://manticoresearch.com/install/ 上的说明操作。

### 您可能需要的更多软件包
#### 用于 indexer
如果您计划使用 [indexer](../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) 从外部源创建表，您需要确保已安装相应的客户端库，以便能够使用您想要的索引源。下面的命令将一次性安装所有这些库；您可以按需使用，也可以只安装您需要的库（仅针对 mysql 源 - 只需 `mysql-libs` 即可，unixODBC 并非必需）。

```bash
sudo yum install mysql-libs postgresql-libs expat unixODBC
```

在 CentOS Stream 8 中，如果尝试从 MySQL 构建普通表时出现错误 `sql_connect: MySQL source wasn't initialized. Wrong name in dlopen?`，您可能需要运行：

```
dnf install mariadb-connector-c
```

#### 乌克兰语词形还原器

词形还原器需要 Python 3.9 及以上版本。**请确保已安装并且配置时启用了 `--enable-shared`。**
以下是在 Centos 8 中安装 Python 3.9 和乌克兰语词形还原器的方法：

# install Manticore Search and UK lemmatizer from YUM repository

```bash
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
pip3.9 install pymorphy2-dicts-uk
```
<!-- proofread -->

