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

<!-- example macos_dlopen -->
#### Resolving MySQL client library errors on macOS

When running `indexer` on macOS, you may encounter an error related to the MySQL client library: `MySQL source wasn't initialized. Wrong name in dlopen?`. This occurs because the MySQL client library version on your macOS differs from the one `indexer` expects. To resolve this issue, you can set the `MYSQL_LIB` environment variable to the path of the MySQL client library:

- **ARM64 architecture**: `/opt/homebrew/opt/mysql-client/lib/libmysqlclient.dylib`
- **x86_64 architecture**: `/usr/local/opt/mysql-client/lib/libmysqlclient.dylib`

<!-- intro -->
Here's an example of the error you might encounter:
<!-- request Error -->
```bash
~ indexer -c min_mysql.conf --all
Manticore 6.3.9 8a1536204@25012115 dev
Copyright (c) 2001-2016, Andrew Aksyonoff
Copyright (c) 2008-2016, Sphinx Technologies Inc (http://sphinxsearch.com)
Copyright (c) 2017-2024, Manticore Software LTD (https://manticoresearch.com)

using config file '/Users/sn/min_mysql.conf'...
indexing table 'idx'...
ERROR: table 'idx': sql_connect: MySQL source wasn't initialized. Wrong name in dlopen? (DSN=mysql://root:***@127.0.0.1:3306/test).
total 0 docs, 0 bytes
total 0.001 sec, 0 bytes/sec, 0.00 docs/sec
total 0 reads, 0.000 sec, 0.0 kb/call avg, 0.0 msec/call avg
total 0 writes, 0.000 sec, 0.0 kb/call avg, 0.0 msec/call avg
```

<!-- intro -->
To fix this issue, set the `MYSQL_LIB` environment variable to the path of the MySQL client library:
<!-- request Solution -->
```bash
~ MYSQL_LIB=/opt/homebrew/opt/mysql-client/lib/libmysqlclient.dylib indexer -c min_mysql.conf --all
Manticore 6.3.9 8a1536204@25012115 dev
Copyright (c) 2001-2016, Andrew Aksyonoff
Copyright (c) 2008-2016, Sphinx Technologies Inc (http://sphinxsearch.com)
Copyright (c) 2017-2024, Manticore Software LTD (https://manticoresearch.com)

using config file '/Users/sn/min_mysql.conf'...
indexing table 'idx'...
collected 1 docs, 0.0 MB
creating lookup: 0.0 Kdocs, 100.0% done
sorted 0.0 Mhits, 100.0% done
total 1 docs, 3 bytes
total 0.136 sec, 22 bytes/sec, 7.34 docs/sec
total 3 reads, 0.000 sec, 0.0 kb/call avg, 0.0 msec/call avg
total 17 writes, 0.000 sec, 0.0 kb/call avg, 0.0 msec/call avg
```

<!-- end -->

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
