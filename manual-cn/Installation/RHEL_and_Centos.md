# 在 RedHat 和 CentOS 上安装 Manticore

### 支持的发行版：

* CentOS 7, RHEL 7, Oracle Linux 7
* CentOS 8, RHEL 8, Oracle Linux 8, CentOS Stream 8
* Amazon Linux 2
* CentOS 9, RHEL 9, AlmaLinux 9

### YUM 仓库

在 RedHat/CentOS 上安装 Manticore 的最简单方法是使用我们的 YUM 仓库：

安装仓库：
```bash
sudo yum install https://repo.manticoresearch.com/manticore-repo.noarch.rpm
```

安装 Manticore Search：
```bash
sudo yum install manticore manticore-extra
```

如果你是从旧版本升级到 Manticore 6，建议首先移除旧的软件包，以避免由于包结构更新而导致的冲突：

```bash
sudo yum remove manticore*
```

这不会删除你的数据和配置文件。

###### 开发包
如果你更喜欢“Nightly”（开发版），可以运行：

```bash
sudo yum -y install https://repo.manticoresearch.com/manticore-repo.noarch.rpm && \
sudo yum -y --enablerepo manticore-dev install manticore manticore-extra manticore-common manticore-server manticore-server-core manticore-tools manticore-executor manticore-buddy manticore-backup manticore-columnar-lib manticore-server-core-debuginfo manticore-tools-debuginfo manticore-columnar-lib-debuginfo  manticore-icudata manticore-galera manticore-galera-debuginfo manticore-language-packs
```

### 独立的 RPM 包
要从 Manticore 仓库下载独立的 RPM 文件，请按照 [这里的说明](https://manticoresearch.com/install/)。

### 其他可能需要的包
#### 对于索引器
如果你计划使用[索引器](../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-工具)从外部源创建表，需要确保安装相应的客户端库，以便能够访问所需的索引源。下面的命令将一次性安装所有必要的库；你可以按需减少，只安装所需的库（仅针对 MySQL 源，只需安装 mysql-libs 即可，unixODBC 不是必需的）。


```bash
sudo yum install mysql-libs postgresql-libs expat unixODBC
```

在 CentOS Stream 8 上，你可能需要运行：

```
dnf install mariadb-connector-c
```

如果遇到错误 `sql_connect: MySQL source wasn't initialized. Wrong name in dlopen?`，则可能需要运行此命令。

#### 乌克兰词形还原器
词形还原器需要 Python 3.9 及以上版本。**确保已安装并配置为 `--enable-shared`。**

以下是在 CentOS 7/8 上安装 Python 3.9 和乌克兰词形还原器的步骤：

```bash
# 从 YUM 仓库安装 Manticore Search 和乌克兰词形还原器
yum -y install https://repo.manticoresearch.com/manticore-repo.noarch.rpm
yum -y install manticore manticore-lemmatizer-uk

# 安装构建 Python 所需的软件包
yum groupinstall "Development Tools" -y
yum install openssl-devel libffi-devel bzip2-devel wget -y

# 下载、构建并安装 Python 3.9
cd ~
wget https://www.python.org/ftp/python/3.9.2/Python-3.9.2.tgz
tar xvf Python-3.9.2.tgz
cd Python-3.9*/
./configure --enable-optimizations --enable-shared
make -j8 altinstall

# 更新链接器缓存
ldconfig

# 安装 pymorphy2 和乌克兰词典
pip3.9 install pymorphy2[fast]
pip3.9 install pymorphy2-dicts-uk
```
<!-- proofread -->
