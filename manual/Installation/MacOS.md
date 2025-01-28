# Installing Manticore on MacOS

## Via Homebrew package manager

```bash
brew install manticoresoftware/tap/manticoresearch manticoresoftware/tap/manticore-extra
```

Start Manticore as a brew service:

```bash
brew services start manticoresearch
```

The default configuration file for Manticore is located at either `/usr/local/etc/manticoresearch/manticore.conf` or `/opt/homebrew/etc/manticoresearch/manticore.conf`.

If you plan to use [indexer](../Creating_a_table/Local_tables/Plain_table.md) to fetch data from sources such as MySQL, PostgreSQL, or another database using ODBC, you may need additional libraries, such as `mysql@5.7`, `libpq`, and `unixodbc`, respectively.

#### Development packages
If you prefer "Nightly" (development) versions do:
```bash
brew tap manticoresoftware/tap-dev
brew install manticoresoftware/tap-dev/manticoresearch-dev manticoresoftware/tap-dev/manticore-extra-dev manticoresoftware/tap-dev/manticore-language-packs
brew services start manticoresearch-dev
```

<!--
## From tarball with binaries

Download it [from the website](https://manticoresearch.com/install/) and unpack to a folder:

```bash
mkdir manticore

cd manticore

wget https://repo.manticoresearch.com/repository/manticoresearch_macos/release/manticore-5.0.2-220530-348514c86-main.tar.gz

tar -xf manticore-5.0.2-220530-348514c86-main.tar.gz

wget https://repo.manticoresearch.com/repository/manticoresearch_macos/release/manticore-columnar-lib-1.15.4-220522-2fef34e-osx10.14.4-x86_64.tar.gz

tar -xf manticore-columnar-lib-1.15.4-220522-2fef34e-osx10.14.4-x86_64.tar.gz

# Start Manticore
FULL_SHARE_DIR=./share/manticore ./bin/searchd -c ./etc/manticoresearch/manticore.conf

# Run indexer
FULL_SHARE_DIR=./share/manticore ./bin/indexer -c ./etc/manticoresearch/manticore.conf
```

Manticore configuration file is `./etc/manticoresearch/manticore.conf` after you unpack the archive.

-->

<!-- proofread -->
