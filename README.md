<p align="center">
  <a href="https://manticoresearch.com" target="_blank" rel="noopener">
    <img src="https://manticoresearch.com/logo.png" width="50%" alt="Manticore Search Logo">
  </a>
</p>

<h3 align="center">
  <a href="https://manticoresearch.com">Website</a> •
  <a href="https://manticoresearch.com/install/">Downloads</a> •
  <a href="https://manual.manticoresearch.com">Docs</a> •
  <a href="https://manticoresearch.com/blog/">Blog</a> •
  <a href="https://play.manticoresearch.com">Courses</a> •
  <a href="https://forum.manticoresearch.com">Forum</a> •
  <a href="https://slack.manticoresearch.com">Slack</a> •
  <a href="https://t.me/manticoresearch_en">Telegram (En)</a> •
  <a href="https://t.me/manticore_chat">Telegram (Ru)</a> •
  <a href="https://twitter.com/manticoresearch">Twitter</a>
</h3>

<p>&nbsp;</p>

# Introduction

[![License: GPLv2](https://img.shields.io/badge/license-GPL%20V2-green?style=plastic)](COPYING)
[![Twitter Follow](https://img.shields.io/twitter/follow/manticoresearch?color=green&logo=Twitter&style=plastic)](https://twitter.com/manticoresearch)
[![Slack](https://img.shields.io/badge/slack-manticoresearch-green.svg?logo=slack&style=plastic)](http://slack.manticoresearch.com/)
[![Docker pulls](https://img.shields.io/docker/pulls/manticoresearch/manticore?color=green&style=plastic)](https://github.com/manticoresoftware/docker)
[![Newsletter](https://img.shields.io/badge/newsletter-green?style=plastic)](https://eepurl.com/dkUTHv)
[![Activity](https://img.shields.io/github/commit-activity/m/manticoresoftware/manticoresearch?color=green&style=plastic)](https://github.com/manticoresoftware/manticoresearch/graphs/commit-activity)
[![GitHub closed issues](https://img.shields.io/github/issues-closed/manticoresoftware/manticoresearch?color=green&style=plastic)](https://github.com/manticoresoftware/manticoresearch/issues?q=is%3Aissue+is%3Aclosed)

**❗Read recent [blog post about Manticore vs Elasticsearch](https://manticoresearch.com/blog/manticore-alternative-to-elasticsearch/)❗**

Manticore Search is an easy to use open source fast database for search. Good alternative for Elasticsearch. What distinguishes it from other solutions is:
* It's very fast and therefore more cost-efficient than alternatives, for example Manticore is:
  - **182x faster** than MySQL for [small data](https://db-benchmarks.com/test-hn-small/#mysql-vs-manticore-search) ([reproducible](https://github.com/db-benchmarks/db-benchmarks#get-started)❗)
  - **29x faster** than Elasticsearch for [log analytics](https://db-benchmarks.com/test-logs10m/#elasticsearch-with-no-tuning-vs-manticore-search-default-row-wise-storage) ([reproducible](https://github.com/db-benchmarks/db-benchmarks#get-started)❗)
  - **15x faster** than Elasticsearch for [small dataset](https://db-benchmarks.com/test-hn-small/#manticore-search-vs-elasticsearch) ([reproducible](https://github.com/db-benchmarks/db-benchmarks#get-started)❗)
  - **5x faster** than Elasticsearch for [medium-size data](https://db-benchmarks.com/test-hn/#manticore-search-columnar-storage-vs-elasticsearch) ([reproducible](https://github.com/db-benchmarks/db-benchmarks#get-started)❗)
  - **4x faster** than Elasticsearch for [big data](https://db-benchmarks.com/test-taxi/#manticore-search-vs-elasticsearch) ([reproducible](https://github.com/db-benchmarks/db-benchmarks#get-started)❗)
  - **up to 2x faster** max throughput than Elasticsearch's for data ingestion on a single server ([reproducible](https://manticoresearch.com/blog/manticore-alternative-to-elasticsearch/#data-ingestion-performance)❗)
* With its modern multithreading architecture and efficient query parallelization capabilities, Manticore is able to fully utilize all your CPU cores to achieve the quickest response times possible.
* The powerful and speedy full-text search works seamlessly with both small and large datasets.
* Row-wise storage for small, medium and big size datasets.
* For even larger datasets, Manticore offers columnar storage support through the [Manticore Columnar Library](https://github.com/manticoresoftware/columnar/), capable of handling datasets too big to fit in RAM.
* Performant secondary indexes are automatically created, saving you time and effort.
* The cost-based query optimizer optimizes search queries for optimal performance.
* Manticore is SQL-first, utilizing SQL as its native syntax, and offers compatibility with the MySQL protocol, allowing you to use your preferred MySQL client.
* With clients available in [PHP](https://github.com/manticoresoftware/manticoresearch-php), [Python](https://github.com/manticoresoftware/manticoresearch-python), [JavaScript](https://github.com/manticoresoftware/manticoresearch-javascript), [Java](https://github.com/manticoresoftware/manticoresearch-java), [Elixir](https://github.com/manticoresoftware/manticoresearch-elixir), and [Go](https://github.com/manticoresoftware/go-sdk), integration with Manticore Search becomes easy.
* Manticore also provides a programmatic HTTP JSON protocol for more versatile data and schema management.
* Built in C++, Manticore Search starts quickly and uses minimal RAM, with low-level optimizations contributing to its impressive performance.
* With real-time inserts, newly added documents are immediately accessible.
* Interactive courses are available through [Interactive courses](https://play.manticoresearch.com/) to make learning a breeze.
* Manticore also boasts built-in replication and load balancing for added reliability.
* Data can be synced from sources such as MySQL, PostgreSQL, ODBC, xml, and csv with ease.
* While not fully ACID-compliant, Manticore still supports transactions and binlog to ensure safe writes.
* Effortless data backup and recovery with built-in tools and SQL commands

[Craigslist](https://www.craigslist.org/), [Socialgist](https://socialgist.com/), [PubChem](https://pubchem.ncbi.nlm.nih.gov/), [Rozetka](https://rozetka.com.ua/) and many others use Manticore for efficient searching and stream filtering.

Manticore Search was forked from [Sphinx 2.3.2](https://github.com/sphinxsearch/sphinx) in 2017.

# More features
* Full-text search and relevance:
  - Over 20 [full-text operators](https://play.manticoresearch.com/fulltextintro/) and over 20 ranking factors
  - Custom ranking
* Other search capabilities:
  - [Rich filtering functionality](https://manual.manticoresearch.com/Searching/Full_text_matching/Operators)
  - [Faceted search](https://play.manticoresearch.com/faceting/)
  - [Geo-spatial search](https://play.manticoresearch.com/geosearch/)
  - [Spelling correction](https://play.manticoresearch.com/didyoumean/)
  - [Autocomplete](https://play.manticoresearch.com/simpleautocomplete/)
  - A wide range of functions for filtering and data manipulation
* Natural language processing (NLP):
  - [Stemming](https://manual.manticoresearch.com/Creating_a_table/NLP_and_tokenization/Morphology)
  - [Lemmatization](https://manual.manticoresearch.com/Creating_a_table/NLP_and_tokenization/Morphology)
  - [Stopwords](https://manual.manticoresearch.com/Creating_a_table/NLP_and_tokenization/Ignoring_stop-words#stopwords)
  - [Synonyms](https://manual.manticoresearch.com/Creating_a_table/NLP_and_tokenization/Exceptions)
  - [Wordforms](https://manual.manticoresearch.com/Creating_a_table/NLP_and_tokenization/Wordforms#wordforms)
  - [Advanced tokenization at character and word level](https://manual.manticoresearch.com/Creating_a_table/NLP_and_tokenization/Low-level_tokenization#charset_table)
  - [Proper Chinese segmentation](https://play.manticoresearch.com/icu-chinese/)
  - [Text highlighting](https://play.manticoresearch.com/highlighting/)
* Stream filtering [using a "percolate" table](https://play.manticoresearch.com/pq/)
* High-availability:
  - Data can be distributed across servers and data-centers
  - [Synchronous replication](https://play.manticoresearch.com/replication/)
  - Built-in load balancing
* Security:
  - [https support](https://play.manticoresearch.com/https/)
  - [read-only mode](https://manual.manticoresearch.com/Security/Read_only)
* Data safety:
  - [manticore-backup tool and SQL command BACKUP](https://manual.manticoresearch.com/Securing_and_compacting_a_table/Backup_and_restore) to back up and restore your data
* Data storages:
  - row-wise - requires more RAM, provides faster performance
  - columnar - requires less RAM, still provides decent performance, but lower than the row-wise storage for some kinds of queries
  - docstore - doesn't require RAM at all, but allows only fetching original value, not sorting/grouping/filtering
* Performance optimizations:
  - [Secondary indexes](https://manual.manticoresearch.com/Server_settings/Searchd#secondary_indexes)
  - Cost-based optimizer determines the most efficient execution plan of a search query
* Data types:
  - full-text field - inverted index
  - int, bigint and float numeric fields in row-wise and columnar fashion
  - multi-value attributes (array)
  - string and JSON
  - on-disk "[stored](https://play.manticoresearch.com/docstore/)" for key-value purpose
* Integrations:
  - [Sync from MySQL and PostgreSQL](https://manual.manticoresearch.com/Creating_a_table/Local_tables/Plain_table#Plain-table)
  - [Sync from XML](https://manual.manticoresearch.com/Adding_data_from_external_storages/Fetching_from_XML_streams#XML-file-format)
  - [Sync from CSV](https://manual.manticoresearch.com/Adding_data_from_external_storages/Fetching_from_CSV,TSV#Fetching-from-TSV,CSV)
  - [With MySQL as a storage engine](https://manual.manticoresearch.com/Extensions/SphinxSE#Using-SphinxSE)
  - [With MySQL via FEDERATED engine](https://manual.manticoresearch.com/Extensions/FEDERATED)
  - [ProxySQL](https://manticoresearch.com/2018/06/18/using-proxysql-to-route-inserts-in-a-distributed-realtime-index/)
  - [Manticore Columnar Library](https://github.com/manticoresoftware/columnar)

# Installation

### Docker
Docker image is available on [Docker Hub](https://dockr.ly/33biV0U).

To experiment with Manticore Search in Docker just run:

```
docker run -e EXTRA=1 --name manticore --rm -d manticoresearch/manticore && until docker logs manticore 2>&1 | grep -q "accepting connections"; do sleep 1; done && docker exec -it manticore mysql && docker stop manticore
```

You can then: create a table, add data and run searches. For example:

```
create table movies(title text, year int) morphology='stem_en' html_strip='1' stopwords='en';

insert into movies(title, year) values ('The Seven Samurai', 1954), ('Bonnie and Clyde', 1954), ('Reservoir Dogs', 1992), ('Airplane!', 1980), ('Raging Bull', 1980), ('Groundhog Day', 1993), ('<a href="http://google.com/">Jurassic Park</a>', 1993), ('Ferris Bueller\'s Day Off', 1986);

select highlight(), year from movies where match('the dog');

select highlight(), year from movies where match('days') facet year;

select * from movies where match('google');
```

Note that upon exiting the MySQL client, the Manticore container will be stopped and removed, resulting in no saved data, so **use this way only for testing / sandboxing purposes**.

Read [the full instruction for the docker image](https://github.com/manticoresoftware/docker) for more details including our recommendations on running it in production.

### Packages

## [Ubuntu, Debian, Centos, Windows and MacOS packages are here](https://manticoresearch.com/install/).

### YUM repo for RHEL/Centos/Amazon Linux
```
sudo yum install https://repo.manticoresearch.com/manticore-repo.noarch.rpm
sudo yum install manticore manticore-columnar-lib
```

### APT repo for Ubuntu/Debian/Mint
```
wget https://repo.manticoresearch.com/manticore-repo.noarch.deb
sudo dpkg -i manticore-repo.noarch.deb
sudo apt update
sudo apt install manticore manticore-columnar-lib
```

### Homebrew on MacOS
```
brew install manticoresearch
```

### Windows
See [instruction here](https://manual.manticoresearch.com/Installation/Windows).

### MacOS tarball
See [instruction here](https://manual.manticoresearch.com/Installation/MacOS#From-tarball-with-binaries).


# Documentation and community sites

  * [Documentation](https://manual.manticoresearch.com)
  * [Interactive courses](https://play.manticoresearch.com)
  * [Manticore Community Forum](https://forum.manticoresearch.com/)
  * [Public Slack chat](http://slack.manticoresearch.com/)
  * [Public Telegram chat (En)](https://t.me/manticoresearch_en)
  * [Public Telegram chat (Ru)](https://t.me/manticore_chat)
  * [Bug tracker](https://github.com/manticoresoftware/manticore/issues)

# Third-party integrations
  * https://dev.tiki.org/Manticore-Search - TikiWiki CMS Groupware integration
  * https://github.com/EvilFreelancer/laravel-manticoresearch - Laravel ManticoreSearch plugin
  * https://www.mediawiki.org/wiki/Extension:SphinxSearch - extension for MediaWiki
  * https://github.com/gordonbanderson/silverstripe-manticore-search - integration for SilverStripe
  * https://drwho.virtadpt.net/archive/2022-05-13/combining-manticore-and-searx/ - integration for SearX
  * https://github.com/dimv36/sphinxlink - Extension for PostgreSQL for running search queries and returning data as PostgreSQL tables

# How we can support you
Should your company require any help - we provide full-cycle services in the areas of Sphinx and Manticore Search:
  * Audit
  * Support
  * Consulting
  * Development
  * Training

[More details here](https://manticoresearch.com/services/)

# ❤️ How you can support Manticore Search

Manticore Search is a GPLv2-licensed Open Source project with development made possible by support from our core team, contributors, and sponsors. Building premium Open Source software is not easy. If you would like to make sure Manticore Search stays free, here is how you can help the project:

* [Donation through PayPal](https://www.paypal.me/manticoresearch)
* [Become our client and let us help you](https://manticoresearch.com/services)
