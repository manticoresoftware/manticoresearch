# Установка Manticore на MacOS

## Через пакетный менеджер Homebrew

```bash
brew install manticoresoftware/tap/manticoresearch manticoresoftware/tap/manticore-extra
```

Запустите Manticore как сервис brew:

```bash
brew services start manticoresearch
```

Файл конфигурации Manticore по умолчанию находится по пути `/usr/local/etc/manticoresearch/manticore.conf` или `/opt/homebrew/etc/manticoresearch/manticore.conf`.

Если вы планируете использовать [indexer](../Creating_a_table/Local_tables/Plain_table.md) для получения данных из источников, таких как MySQL, PostgreSQL или другой базы данных через ODBC, возможно, потребуются дополнительные библиотеки, такие как `mysql@5.7`, `libpq` и `unixodbc` соответственно.

#### Пакеты для разработки
Если вы предпочитаете версии "Nightly" (разработческие), выполните:
```bash
brew tap manticoresoftware/tap-dev
brew install manticoresoftware/tap-dev/manticoresearch-dev manticoresoftware/tap-dev/manticore-extra-dev manticoresoftware/tap-dev/manticore-language-packs
brew services start manticoresearch-dev
```

<!--
## Из tarball с бинарными файлами

Скачайте его [с сайта](https://manticoresearch.com/install/) и распакуйте в папку:

```bash
mkdir manticore

cd manticore

wget https://repo.manticoresearch.com/repository/manticoresearch_macos/release/manticore-5.0.2-220530-348514c86-main.tar.gz

tar -xf manticore-5.0.2-220530-348514c86-main.tar.gz

wget https://repo.manticoresearch.com/repository/manticoresearch_macos/release/manticore-columnar-lib-1.15.4-220522-2fef34e-osx10.14.4-x86_64.tar.gz

tar -xf manticore-columnar-lib-1.15.4-220522-2fef34e-osx10.14.4-x86_64.tar.gz

## Start Manticore
FULL_SHARE_DIR=./share/manticore ./bin/searchd -c ./etc/manticoresearch/manticore.conf

## Run indexer
FULL_SHARE_DIR=./share/manticore ./bin/indexer -c ./etc/manticoresearch/manticore.conf
```

Файл конфигурации Manticore после распаковки архива находится по пути `./etc/manticoresearch/manticore.conf`.

-->

<!-- proofread -->

