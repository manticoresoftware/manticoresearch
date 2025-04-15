# Установка пакетов Manticore на RedHat и CentOS

### Поддерживаемые версии:

* CentOS 7, RHEL 7, Oracle Linux 7
* CentOS 8, RHEL 8, Oracle Linux 8, CentOS Stream 8
* Amazon Linux 2
* CentOS 9, RHEL 9, AlmaLinux 9

### YUM репозиторий

Самый простой способ установить Manticore на RedHat/CentOS — воспользоваться нашим YUM репозиторием:

Установите репозиторий:
```bash
sudo yum install https://repo.manticoresearch.com/manticore-repo.noarch.rpm
```

Затем установите Manticore Search:
```bash
sudo yum install manticore manticore-extra
```

Если вы обновляетесь до Manticore 6 с более ранней версии, рекомендуется сначала удалить старые пакеты, чтобы избежать конфликтов, вызванных обновленной структурой пакетов:

```bash
sudo yum remove manticore*
```

Это не удалит ваши данные и файл конфигурации.

###### Пакеты разработки
Если вы предпочитаете "Nightly" (разработческие) версии, выполните:

```bash
sudo yum -y install https://repo.manticoresearch.com/manticore-repo.noarch.rpm && \
sudo yum -y --enablerepo manticore-dev install manticore manticore-extra manticore-common manticore-server manticore-server-core manticore-tools manticore-executor manticore-buddy manticore-backup manticore-columnar-lib manticore-server-core-debuginfo manticore-tools-debuginfo manticore-columnar-lib-debuginfo  manticore-icudata manticore-galera manticore-galera-debuginfo manticore-language-packs manticore-load
```

### Ст standalone RPM пакеты
Чтобы скачать standalone RPM файлы из репозитория Manticore, следуйте инструкциям, доступным на https://manticoresearch.com/install/.

### Дополнительные пакеты, которые могут понадобиться
#### Для индексатора
Если вы планируете использовать [индексатор](../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) для создания таблиц из внешних источников, вам нужно убедиться, что вы установили соответствующие клиентские библиотеки, чтобы сделать доступными источники индексирования, которые вам нужны. Строка ниже установит все из них сразу; не стесняйтесь использовать ее, как есть, или сократите, чтобы установить только необходимые библиотеки (для только mysql источников — достаточно `mysql-libs`, а unixODBC не требуется).

```bash
sudo yum install mysql-libs postgresql-libs expat unixODBC
```

В CentOS Stream 8 вам может понадобиться выполнить:

```
dnf install mariadb-connector-c
```

если вы получите ошибку `sql_connect: MySQL source wasn't initialized. Wrong name in dlopen?`, пытаясь создать простую таблицу из MySQL.

#### Украинский лемматизатор
Лемматизатор требует Python 3.9+. **Убедитесь, что он установлен и настроен с `--enable-shared`.**

Вот как установить Python 3.9 и украинский лемматизатор в CentOS 7/8:

```bash
# установить Manticore Search и УК лемматизатор из YUM репозитория
yum -y install https://repo.manticoresearch.com/manticore-repo.noarch.rpm
yum -y install manticore manticore-lemmatizer-uk

# установить пакеты, необходимые для сборки Python
yum groupinstall "Development Tools" -y
yum install openssl-devel libffi-devel bzip2-devel wget -y

# скачать, собрать и установить Python 3.9
cd ~
wget https://www.python.org/ftp/python/3.9.2/Python-3.9.2.tgz
tar xvf Python-3.9.2.tgz
cd Python-3.9*/
./configure --enable-optimizations --enable-shared
make -j8 altinstall

# обновить кэш компоновщика
ldconfig

# установить pymorphy2 и УК словарь
pip3.9 install pymorphy2[fast]
pip3.9 install pymorphy2-dicts-uk
```
<!-- proofread -->
