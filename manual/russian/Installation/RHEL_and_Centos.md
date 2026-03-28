# Установка пакетов Manticore на RedHat и CentOS

### Поддерживаемые версии:

* CentOS 8, RHEL 8, Oracle Linux 8, CentOS Stream 8
* Amazon Linux 2
* CentOS 9, RHEL 9, AlmaLinux 9
* AlmaLinux 10, другие дистрибутивы на базе RHEL 10

### Репозиторий YUM

Самый простой способ установить Manticore на RedHat/CentOS — использовать наш репозиторий YUM:

Установите репозиторий:
```bash
sudo yum install https://repo.manticoresearch.com/manticore-repo.noarch.rpm
```

Затем установите Manticore Search:
```bash
sudo yum install manticore manticore-extra
```

Если вы обновляетесь до Manticore 6 с более старой версии, рекомендуется сначала удалить старые пакеты, чтобы избежать конфликтов, вызванных обновленной структурой пакетов:

```bash
sudo yum remove manticore*
```

Это не удалит ваши данные и файл конфигурации.

###### Пакеты для разработки
Если вы предпочитаете версии "Nightly" (разработческие), выполните:

```bash
sudo yum -y install https://repo.manticoresearch.com/manticore-repo.noarch.rpm && \
sudo yum -y --disablerepo=manticore --enablerepo manticore-dev install manticore
```

### Отдельные RPM-пакеты
Чтобы скачать отдельные RPM-файлы из репозитория Manticore, следуйте инструкциям на https://manticoresearch.com/install/.

### Дополнительные пакеты, которые могут понадобиться
#### Для индексатора
Если вы планируете использовать [indexer](../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) для создания таблиц из внешних источников, необходимо убедиться, что установлены соответствующие клиентские библиотеки для обеспечения поддержки нужных вам источников индексации. В строке ниже они устанавливаются все сразу; вы можете использовать её как есть или сократить установку, оставив только необходимые библиотеки (для MySQL-источников достаточно только `mysql-libs`, unixODBC не обязателен).

```bash
sudo yum install mysql-libs postgresql-libs expat unixODBC
```

В CentOS Stream 8 вам может понадобиться выполнить:

```
dnf install mariadb-connector-c
```

если при сборке plain-таблицы из MySQL вы получаете ошибку `sql_connect: MySQL source wasn't initialized. Wrong name in dlopen?`.
<!-- proofread -->
