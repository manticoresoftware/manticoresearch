# Установка пакетов Manticore на RedHat и CentOS

### Поддерживаемые выпуски:

* CentOS 8, RHEL 8, Oracle Linux 8, CentOS Stream 8
* Amazon Linux 2
* CentOS 9, RHEL 9, AlmaLinux 9
* AlmaLinux 10, другие дистрибутивы на основе RHEL 10

### Репозиторий YUM

Самый простой способ установить Manticore на RedHat/CentOS — использовать наш репозиторий YUM:

Установите репозиторий:
```bash
sudo yum install https://repo.manticoresearch.com/manticore-repo.noarch.rpm
```

Затем установите Manticore Search:
```bash
sudo yum install manticore
```

Если вы обновляетесь до Manticore 6 с более старой версии, рекомендуется сначала удалить старые пакеты, чтобы избежать конфликтов, вызванных обновленной структурой пакетов:

```bash
sudo yum remove manticore*
```

Это не удалит ваши данные и файл конфигурации.

###### Пакеты для разработки
Если вы предпочитаете "Ночные" (разрабатываемые) версии, выполните:

```bash
sudo yum -y install https://repo.manticoresearch.com/manticore-repo.noarch.rpm && \
sudo yum -y --disablerepo=manticore --enablerepo manticore-dev install manticore
```

### Автономные RPM-пакеты
Чтобы скачать автономные RPM-файлы из репозитория Manticore, следуйте инструкциям, доступным по адресу https://manticoresearch.com/install/.

### Дополнительные пакеты, которые могут вам понадобиться
#### Для indexer
Если вы планируете использовать [indexer](../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) для создания таблиц из внешних источников, вам необходимо убедиться, что установлены соответствующие клиентские библиотеки, чтобы были доступны нужные вам источники индексирования. Строка ниже установит их все сразу; вы можете использовать её как есть или сократить, чтобы установить только необходимые библиотеки (только для источников mysql — должно быть достаточно `mysql-libs`, а unixODBC не нужен).

```bash
sudo yum install mysql-libs postgresql-libs expat unixODBC
```

В CentOS Stream 8 может потребоваться выполнить:

```
dnf install mariadb-connector-c
```

если вы получаете ошибку `sql_connect: MySQL source wasn't initialized. Wrong name in dlopen?` при попытке построить обычную таблицу из MySQL.

#### Украинский лемматизатор
Лемматизатор требует Python 3.9+. **Убедитесь, что он установлен и сконфигурирован с опцией `--enable-shared`.**

Вот как установить Python 3.9 и украинский лемматизатор в Centos 8:

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
<!-- proofread -->
