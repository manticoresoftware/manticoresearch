# Установка Manticore на MacOS

## Через менеджер пакетов Homebrew

```bash
brew install manticoresoftware/tap/manticoresearch manticoresoftware/tap/manticore-extra
```

Запустите Manticore как службу brew:

```bash
brew services start manticoresearch
```

Файл конфигурации по умолчанию для Manticore находится по адресу `/usr/local/etc/manticoresearch/manticore.conf` или `/opt/homebrew/etc/manticoresearch/manticore.conf`.

Если вы планируете использовать [индексатор](../Creating_a_table/Local_tables/Plain_table.md) для извлечения данных из источников, таких как MySQL, PostgreSQL или другой базы данных с использованием ODBC, вам могут понадобиться дополнительные библиотеки, такие как `mysql@5.7`, `libpq` и `unixodbc`, соответственно.

#### Пакеты разработки
Если вы предпочитаете "Ночные" (разработческие) версии, выполните:
```bash
brew tap manticoresoftware/tap-dev
brew install manticoresoftware/tap-dev/manticoresearch-dev manticoresoftware/tap-dev/manticore-extra-dev manticoresoftware/tap-dev/manticore-language-packs
brew services start manticoresearch-dev
```

<!--
## Из tarball с бинарниками

Скачайте [с сайта](https://manticoresearch.com/install/) и распакуйте в папку:

```bash
mkdir manticore

cd manticore

wget https://repo.manticoresearch.com/repository/manticoresearch_macos/release/manticore-5.0.2-220530-348514c86-main.tar.gz

tar -xf manticore-5.0.2-220530-348514c86-main.tar.gz

wget https://repo.manticoresearch.com/repository/manticoresearch_macos/release/manticore-columnar-lib-1.15.4-220522-2fef34e-osx10.14.4-x86_64.tar.gz

tar -xf manticore-columnar-lib-1.15.4-220522-2fef34e-osx10.14.4-x86_64.tar.gz

# Запустить Manticore
FULL_SHARE_DIR=./share/manticore ./bin/searchd -c ./etc/manticoresearch/manticore.conf

# Запустить индексатор
FULL_SHARE_DIR=./share/manticore ./bin/indexer -c ./etc/manticoresearch/manticore.conf
```

Файл конфигурации Manticore — это `./etc/manticoresearch/manticore.conf` после того, как вы распакуете архив.

-->

<!-- proofread -->
