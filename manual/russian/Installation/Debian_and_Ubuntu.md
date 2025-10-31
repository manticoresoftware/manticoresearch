# Установка Manticore в Debian или Ubuntu

### Поддерживаемые релизы:

* Debian
  * 10.0 (Buster)
  * 11.0 (Bullseye)
  * 12.0 (Bookworm)

* Ubuntu
  * 18.04 (Bionic)
  * 20.04 (Focal)
  * 21.04 (Hirsute Hippo)
  * 22.04 (Jammy)
  * 24.04 (Noble)

* Mint
  * 19
  * 20
  * 21

### Тест

Это тестовый раздел

<!-- example installation expanded -->

<!-- request RHEL, Centos, Alma, Amazon, Oracle -->

``` bash
sudo yum install https://repo.manticoresearch.com/manticore-repo.noarch.rpm
sudo yum install manticore manticore-extra
```

Если вы обновляетесь с более старой версии, рекомендуется сначала удалить старые пакеты, чтобы избежать конфликтов, вызванных обновленной структурой пакетов:
```bash
sudo yum --setopt=tsflags=noscripts remove manticore*
```
Это не удалит ваши данные. Если вы вносили изменения в файл конфигурации, они будут сохранены в `/etc/manticoresearch/manticore.conf.rpmsave`.

Если вы ищете отдельные пакеты, их можно найти [здесь](https://manticoresearch.com/install/#separate-packages).

Для получения дополнительной информации об установке смотрите [ниже](../Installation/RHEL_and_Centos.md).

<!-- request Debian, Ubuntu, Mint -->

``` bash
wget https://repo.manticoresearch.com/manticore-repo.noarch.deb
sudo dpkg -i manticore-repo.noarch.deb
sudo apt update
sudo apt install manticore manticore-extra
```

Если вы обновляетесь до Manticore 6 с более старой версии, рекомендуется сначала удалить старые пакеты, чтобы избежать конфликтов, вызванных обновленной структурой пакетов:
```bash
sudo apt remove manticore*
```
Это не удалит ваши данные или файл конфигурации.

<!-- end -->

### Репозиторий APT
Самый простой способ установить Manticore в Ubuntu/Debian/Mint — использовать наш репозиторий APT.

Установите репозиторий:
```bash
wget https://repo.manticoresearch.com/manticore-repo.noarch.deb
sudo dpkg -i manticore-repo.noarch.deb
sudo apt update
```
(установите `wget`, если он не установлен; установите `gnupg2`, если `apt-key` не работает).

Затем установите Manticore Search:
```
sudo apt install manticore manticore-extra
```

Если вы обновляетесь до Manticore 6 с более старой версии, рекомендуется сначала удалить старые пакеты, чтобы избежать конфликтов из-за обновлённой структуры пакетов:

```bash
sudo apt remove manticore*
```

Это не удалит ваши данные и файл конфигурации.

###### Пакеты для разработки
Если вы предпочитаете версии "Nightly" (разработческие), выполните:
```bash
wget https://repo.manticoresearch.com/manticore-dev-repo.noarch.deb && \
sudo dpkg -i manticore-dev-repo.noarch.deb && \
sudo apt -y update && \
sudo apt -y install manticore manticore-extra manticore-common manticore-server manticore-server-core manticore-tools manticore-executor manticore-buddy manticore-backup manticore-columnar-lib manticore-server-core-dbgsym manticore-tools-dbgsym manticore-columnar-lib-dbgsym manticore-icudata-65l manticore-galera manticore-galera-dbgsym manticore-language-packs manticore-load
```

### Автономные DEB-пакеты
Чтобы скачать автономные DEB-файлы из репозитория Manticore, следуйте инструкциям на https://manticoresearch.com/install/.

### Другие пакеты, которые могут понадобиться
#### Для indexer
Пакет Manticore зависит от библиотек zlib и ssl, других обязательных зависимостей нет. Однако, если вы планируете использовать [indexer](../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) для создания таблиц из внешних хранилищ, вам нужно установить соответствующие клиентские библиотеки. Чтобы узнать, какие конкретно библиотеки нужны `indexer`, запустите его и посмотрите в начале его вывода:

```bash
$ sudo -u manticore indexer
Manticore 3.5.4 13f8d08d@201211 release
Copyright (c) 2001-2016, Andrew Aksyonoff
Copyright (c) 2008-2016, Sphinx Technologies Inc (http://sphinxsearch.com)
Copyright (c) 2017-2020, Manticore Software LTD (https://manticoresearch.com)

Built by gcc/clang v 5.4.0,

Built on Linux runner-0277ea0f-project-3858465-concurrent-0 4.19.78-coreos #1 SMP Mon Oct 14 22:56:39 -00 2019 x86_64 x86_64 x86_64 GNU/Linux

Configured by CMake with these definitions: -DCMAKE_BUILD_TYPE=RelWithDebInfo -DDISTR_BUILD=xenial -DUSE_SSL=ON -DDL_UNIXODBC=1 -DUNIXODBC_LIB=libodbc.so.2 -DDL_EXPAT=1 -DEXPAT_LIB=libexpat.so.1 -DUSE_LIBICONV=1 -DDL_MYSQL=1 -DMYSQL_LIB=libmysqlclient.so.20 -DDL_PGSQL=1 -DPGSQL_LIB=libpq.so.5 -DLOCALDATADIR=/var/data -DFULL_SHARE_DIR=/usr/share/manticore -DUSE_ICU=1 -DUSE_BISON=ON -DUSE_FLEX=ON -DUSE_SYSLOG=1 -DWITH_EXPAT=1 -DWITH_ICONV=ON -DWITH_MYSQL=1 -DWITH_ODBC=ON -DWITH_POSTGRESQL=1 -DWITH_RE2=1 -DWITH_STEMMER=1 -DWITH_ZLIB=ON -DGALERA_SOVERSION=31 -DSYSCONFDIR=/etc/manticoresearch
```

Здесь видно упоминания **libodbc.so.2**, **libexpat.so.1**, **libmysqlclient.so.20** и **libpq.so.5**.

Ниже таблица с перечнем всех клиентских библиотек для разных версий Debian/Ubuntu:

| Distr | MySQL | PostgreSQL | XMLpipe | UnixODBC |
| - | - | - | - | - |
| Ubuntu Trusty | libmysqlclient.so.18 | libpq.so.5 | libexpat.so.1 | libodbc.so.1 |
| Ubuntu Bionic | libmysqlclient.so.20 | libpq.so.5 | libexpat.so.1 | libodbc.so.2 |
| Ubuntu Focal | libmysqlclient.so.21 | libpq.so.5 | libexpat.so.1 | libodbc.so.2 |
| Ubuntu Hirsute | libmysqlclient.so.21 | libpq.so.5 | libexpat.so.1 | libodbc.so.2 |
| Ubuntu Jammy | libmysqlclient.so.21 | libpq.so.5 | libexpat.so.1 | libodbc.so.2 |
| Debian Jessie | libmysqlclient.so.18 | libpq.so.5 | libexpat.so.1 | libodbc.so.2 |
| Debian Buster | libmariadb.so.3 | libpq.so.5 | libexpat.so.1 | libodbc.so.2 |
| Debian Bullseye | libmariadb.so.3 | libpq.so.5 | libexpat.so.1 | libodbc.so.2 |
| Debian Bookworm | libmariadb.so.3 | libpq.so.5 | libexpat.so.1 | libodbc.so.2 |

Чтобы найти пакеты, в которых содержатся эти библиотеки, можно использовать, например, `apt-file`:

```bash
apt-file find libmysqlclient.so.20
libmysqlclient20: /usr/lib/x86_64-linux-gnu/libmysqlclient.so.20
libmysqlclient20: /usr/lib/x86_64-linux-gnu/libmysqlclient.so.20.2.0
libmysqlclient20: /usr/lib/x86_64-linux-gnu/libmysqlclient.so.20.3.6
```

Обратите внимание, что нужны только библиотеки для тех типов хранилищ, которые вы будете использовать. Если вы планируете строить таблицы только из MySQL, то, возможно, достаточно установить только библиотеку MySQL (в примере выше — `libmysqlclient20`).

В конце установите необходимые пакеты:

```bash
sudo apt-get install libmysqlclient20 libodbc1 libpq5 libexpat1
```

Если вы вообще не собираетесь использовать инструмент `indexer`, искать и устанавливать библиотеки не нужно.

Чтобы включить поддержку токенизации CJK, официальные пакеты содержат бинарные файлы с встроенной библиотекой ICU и включают файл данных ICU (или не включают). Они независимы от любой ICU runtime библиотеки, которая может быть доступна на вашей системе, и не могут быть обновлены.

#### Украинский лемматизатор
Лемматизатор требует Python 3.9+. **Убедитесь, что он установлен и с опцией `--enable-shared`.**

Вот как установить Python 3.9 и украинский лемматизатор в Debian и Ubuntu:

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
sudo LD_LIBRARY_PATH=~/Python-3.9.4 pip3.9 install pymorphy2[fast]
sudo LD_LIBRARY_PATH=~/Python-3.9.4 pip3.9 install pymorphy2-dicts-uk
```
<!-- proofread -->

