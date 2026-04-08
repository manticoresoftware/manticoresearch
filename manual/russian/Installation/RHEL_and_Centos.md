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
<!-- proofread -->
