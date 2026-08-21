<p align="center">
  <a href="https://manticoresearch.com" target="_blank" rel="noopener">
    <img src="https://manticoresearch.com/logo.png" width="50%" alt="Manticore Search Logo">
  </a>
</p>

<h3 align="center"><strong>Easy to use open source fast database for search</strong></h3>
<p align="center">
Manticore Search is an easy-to-use, open-source, and fast database designed for search. It is a great alternative to Elasticsearch.
</p>

<div align="center">
<a href="https://trendshift.io/repositories/3537" target="_blank"><img src="https://trendshift.io/api/badge/repositories/3537" alt="manticoresoftware%2Fmanticoresearch | Trendshift" style="width: 250px; height: 55px;" width="250" height="55"/></a>
</div>

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
  <a href="https://twitter.com/manticoresearch">Twitter</a> •
  <a href="https://github.com/manticoresoftware/manticoresearch/discussions/categories/feedback">User feedback</a>
</h3>

<p align="center">
<a href="LICENSE"><img alt="License: GPLv3 or later" src="https://img.shields.io/badge/license-GPL%20V3%2B-green?style=plastic"></a>
<a href="https://github.com/manticoresoftware/manticoresearch/actions/workflows/test.yml?query=branch%3Amain"><img alt="GitHub Actions Workflow Status" src="https://img.shields.io/github/actions/workflow/status/manticoresoftware/manticoresearch/test.yml?branch=main&style=plastic&color=green"></a>
<a href="https://twitter.com/manticoresearch"><img alt="Twitter Follow" src="https://img.shields.io/twitter/follow/manticoresearch?color=green&logo=Twitter&style=plastic"></a>
<a href="http://slack.manticoresearch.com/"><img alt="Slack" src="https://img.shields.io/badge/slack-manticoresearch-green.svg?logo=slack&style=plastic"></a>
<a href="https://github.com/manticoresoftware/docker"><img alt="Docker pulls" src="https://img.shields.io/docker/pulls/manticoresearch/manticore?color=green&style=plastic"></a>
<a href="https://eepurl.com/dkUTHv"><img alt="Newsletter" src="https://img.shields.io/badge/newsletter-green?style=plastic"></a>
<a href="https://github.com/manticoresoftware/manticoresearch/graphs/commit-activity"><img alt="Activity" src="https://img.shields.io/github/commit-activity/m/manticoresoftware/manticoresearch?color=green&style=plastic"></a>
<a href="https://github.com/manticoresoftware/manticoresearch/issues?q=is%3Aissue+is%3Aclosed"><img alt="GitHub closed issues" src="https://img.shields.io/github/issues-closed/manticoresoftware/manticoresearch?color=green&style=plastic"></a>
</p>

# Introduction

What distinguishes Manticore from other solutions is:
* It's very fast and therefore more cost-efficient than alternatives. In the current [reproducible benchmarks](https://github.com/db-benchmarks/db-benchmarks), Manticore Search 27.1.5 is:
  - [**340x faster** than MySQL 9.7.1](https://db-benchmarks.com/?cache=fast_avg&engines=manticoresearch_27.1.5%2Cmysql_9.7.1&tests=hn_small&memory=110000) and [**6.51x faster** than Typesense 27.1](https://db-benchmarks.com/?cache=fast_avg&engines=manticoresearch_27.1.5%2Ctypesense_27.1&tests=hn_small&memory=110000) for 1.1M Hacker News comments
  - **3.85x faster** than tuned Elasticsearch 9.4.3 for [100M+ Hacker News comments](https://db-benchmarks.com/?cache=fast_avg&engines=manticoresearch_rowwise_27.1.5%2Celasticsearch_tuned_9.4.3&tests=hn&memory=110000)
  - [**5.03x faster** than tuned Elasticsearch 9.4.3](https://db-benchmarks.com/?cache=fast_avg&engines=elasticsearch_tuned_9.4.3%2Cmanticoresearch_columnar_27.1.5&tests=logs10m&memory=110000&queries=0%2C1%2C3%2C4%2C10%2C11) for selected typical DevOps queries on 10M Nginx logs; [**1.71x faster** than ClickHouse 26.6.1.1193](https://db-benchmarks.com/?cache=fast_avg&engines=manticoresearch_columnar_27.1.5%2Cclickhouse_26.6.1.1193&tests=logs10m&memory=110000) across the dashboard's default 10M-log query selection
  - [**2.02x faster** than tuned Elasticsearch 9.4.3](https://db-benchmarks.com/?cache=fast_avg&engines=manticoresearch_columnar_27.1.5%2Celasticsearch_tuned_9.4.3&tests=taxi&memory=110000) and [**3.16x faster** than ClickHouse 26.6.1.1193](https://db-benchmarks.com/?cache=fast_avg&engines=manticoresearch_columnar_27.1.5%2Cclickhouse_26.6.1.1193&tests=taxi&memory=110000) for 1.7B NYC taxi rides
  - For the same [10M Nginx-log ingestion](https://db-benchmarks.com/?cache=fast_avg&engines=elasticsearch_tuned_9.4.3%2Cmanticoresearch_columnar_27.1.5&tests=logs10m&memory=110000&queries=0%2C1%2C3%2C4%2C10%2C11), Manticore Search Columnar 27.1.5 completed in **5m 46s** vs **10m 15s** for tuned Elasticsearch 9.4.3, using **1.02 vs 3.80 CPU cores** on average, **3.98 GB vs 36.98 GB RAM** on average, and **0.41 MB read / 8.05 GB written** vs **322.14 MB read / 18.47 GB written**.

  Results are workload-specific; use the linked dashboard to select the queries that match your workload.
* ⚡ **Multi-threaded query execution** and efficient query parallelization use all CPU cores for low response times.
* 🔎 **Full-text search** works seamlessly with both small and large datasets.
* 🧩 **Hybrid search** combines full-text and vector retrieval in a single query for better relevance.
* 💬 **Conversational search** lets existing vectorized tables answer questions with KNN retrieval, conversation history, and LLM-backed responses through SQL `CALL CHAT` or the HTTP JSON `/search` endpoint.
* 🏎️ **Row-wise storage** provides fast performance for small, medium, and large datasets.
* 🗄️ **Columnar storage**, powered by the [Manticore Columnar Library](https://github.com/manticoresoftware/columnar/), handles datasets too large to fit in RAM.
* 📇 **Automatic secondary indexes** use the PGM-index (Piecewise Geometric Model index) for efficient mapping between indexed keys and memory locations.
* 🧠 **Cost-based query optimization** uses indexed-data statistics to select efficient execution plans.
* 🧮 **SQL-first and MySQL-compatible**: use SQL as the native syntax with your preferred MySQL client.
* 🔌 **Official clients** for [PHP](https://github.com/manticoresoftware/manticoresearch-php), [Python](https://github.com/manticoresoftware/manticoresearch-python), [Python asyncio](https://github.com/manticoresoftware/manticoresearch-python-asyncio) [JavaScript](https://github.com/manticoresoftware/manticoresearch-javascript), [Typescript](https://github.com/manticoresoftware/manticoresearch-typescript), [Java](https://github.com/manticoresoftware/manticoresearch-java), [Elixir](https://github.com/manticoresoftware/manticoresearch-elixir), [Go](https://github.com/manticoresoftware/manticoresearch-go), and [Rust](https://github.com/manticoresoftware/manticoresearch-rust) make integration easy.
* 🌐 **HTTP JSON API and Elasticsearch-compatible writes** support versatile data and schema management.
* 🧱 **Sharded tables** transparently distribute reads and writes across physical shards on one node or across a replication cluster.
* 🔐 **Built-in authentication and authorization** secure MySQL, HTTP/HTTPS, distributed remote agents, and replication-related operations with users, bearer tokens, and fine-grained permissions.
* 🪶 **C++ performance and a small footprint**: Manticore Search starts quickly and uses around 40MB RSS for an empty instance.
* ⚡ **Real-time inserts** make newly added or updated documents immediately accessible.
* 🎓 **Interactive courses** at [play.manticoresearch.com](https://play.manticoresearch.com/) make learning easier.
* 🔄 **Virtually synchronous multi-master replication** uses Galera and includes load balancing.
* 🔗 **Data synchronization** supports sources such as MySQL, PostgreSQL, ODBC, XML, and CSV.
* 🛡️ **Safe writes**: while not fully ACID-compliant, Manticore supports isolated transactions and binary logging.
* 💾 **Backup and recovery** include `manticore-backup`, SQL `BACKUP`, and S3-compatible backup/restore support.

[Craigslist](https://www.craigslist.org/), [Socialgist](https://socialgist.com/), [PubChem](https://pubchem.ncbi.nlm.nih.gov/), [Rozetka](https://rozetka.com.ua/) and many others use Manticore for efficient searching and stream filtering.

# More features
* Full-text search and relevance:
  - Over 20 [full-text operators](https://play.manticoresearch.com/fulltextintro/) and over 20 ranking factors
  - Custom ranking
* Other search capabilities:
  - [Rich filtering functionality](https://manual.manticoresearch.com/Searching/Full_text_matching/Operators)
  - [Fuzzy search](https://manual.manticoresearch.com/Searching/Spell_correction#Fuzzy-Search)
  - [Faceted search](https://play.manticoresearch.com/faceting/)
  - [Geo-spatial search](https://play.manticoresearch.com/geosearch/)
  - [Hybrid search](https://manual.manticoresearch.com/Searching/Hybrid_search)
  - [Vector search](https://manual.manticoresearch.com/Searching/KNN)
  - [Conversational search](https://manual.manticoresearch.com/Searching/Conversational_search)
  - [Joining tables](https://manual.manticoresearch.com/Searching/Joining)
  - [Spelling correction](https://play.manticoresearch.com/didyoumean/)
  - [Autocomplete](https://manual.manticoresearch.com/Searching/Autocomplete#Autocomplete)
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
* Stream filtering:
  - [using a "percolate" table](https://play.manticoresearch.com/pq/)
  - or the [Kafka integration](https://manual.manticoresearch.com/Integration/Kafka)
* High-availability:
  - [Sharded tables](https://manual.manticoresearch.com/Creating_a_table/Creating_a_sharded_table/Creating_a_sharded_table)
  - Data can be distributed across servers and data-centers
  - [Synchronous replication](https://play.manticoresearch.com/replication/)
  - Built-in load balancing
* Security:
  - [Authentication and authorization](https://manual.manticoresearch.com/Security/Authentication_and_authorization)
  - [https support](https://play.manticoresearch.com/https/)
  - [read-only mode](https://manual.manticoresearch.com/Security/Read_only)
* Data safety:
  - [manticore-backup tool and SQL command BACKUP](https://manual.manticoresearch.com/Securing_and_compacting_a_table/Backup_and_restore) to back up and restore your data
  - [S3-compatible backup and restore](https://manual.manticoresearch.com/Securing_and_compacting_a_table/Backup_and_restore)
* Data storages:
  - row-wise - requires more RAM, provides faster performance
  - columnar - requires less RAM, still provides decent performance, but lower than the row-wise storage for some kinds of queries
  - docstore - doesn't require RAM at all, but allows only fetching original value, not sorting/grouping/filtering
* Performance optimizations:
  - [Secondary indexes](https://manual.manticoresearch.com/Server_settings/Searchd#secondary_indexes)
  - Cost-based optimizer determines the most efficient execution plan of a search query
* Data types:
  - full-text field - inverted index
  - [UUID document IDs](https://manual.manticoresearch.com/Creating_a_table/Data_types#UUID-document-IDs) for real-time tables
  - int, bigint and float numeric fields in row-wise and columnar fashion
  - multi-value attributes (array)
  - string and JSON
  - on-disk "[stored](https://play.manticoresearch.com/docstore/)" for key-value purpose
* Integrations:
  - [Sync from MySQL and PostgreSQL](https://manual.manticoresearch.com/Creating_a_table/Local_tables/Plain_table#Plain-table)
  - [Sync from XML](https://manual.manticoresearch.com/Adding_data_from_external_storages/Fetching_from_XML_streams#XML-file-format)
  - [Sync from CSV](https://manual.manticoresearch.com/Adding_data_from_external_storages/Fetching_from_CSV,TSV#Fetching-from-TSV,CSV)
  - [Sync from ODBC](https://manual.manticoresearch.com/Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Introduction#Introduction)
  - [Sync from MS SQL](https://manual.manticoresearch.com/Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Introduction#Introduction)
  - [Sync from Kafka](https://manual.manticoresearch.com/Integration/Kafka)
  - [With MySQL as a storage engine](https://manual.manticoresearch.com/Extensions/SphinxSE#Using-SphinxSE)
  - [With MySQL via FEDERATED engine](https://manual.manticoresearch.com/Extensions/FEDERATED)
  - [ProxySQL](https://manticoresearch.com/blog/using-proxysql-to-route-inserts-in-a-distributed-realtime-index/)
  - [Apache Superset](https://manticoresearch.com/blog/manticoresearch-apache-superset-integration/)
  - [Grafana](https://manticoresearch.com/blog/manticoresearch-grafana-integration/)
  - [Fluentbit](https://manticoresearch.com/blog/integration-of-manticore-with-fluentbit/)
  - [Kibana](https://manual.manticoresearch.com/Integration/Kibana#Integration-of-Manticore-with-Kibana) ([Demo](https://github.com/manticoresoftware/kibana-demo))
  - [Logstash/Filebeat](https://manticoresearch.com/blog/integration-of-manticore-with-logstash-filebeat/)
  - [Vector.dev](https://manticoresearch.com/blog/integration-of-manticore-with-vectordev/)
  - [Mysqldump](https://manual.manticoresearch.com/Securing_and_compacting_a_table/Backup_and_restore#Backup-and-restore-with-mysqldump)
  - [Manticore Columnar Library](https://github.com/manticoresoftware/columnar)

# Installation

## Quick install

For Linux and macOS with Homebrew, use the one-line installer:

```sh
curl https://manticoresearch.com | sh
```

It detects Debian/Ubuntu/Mint, RHEL/CentOS/Amazon/Oracle/Fedora-like RPM systems, and macOS with Homebrew. It configures the official Manticore repository when needed, installs `manticore`, starts the service when supported, and prints the result.

Common commands:

```sh
curl https://manticoresearch.com | sh -s help
curl https://manticoresearch.com | sh -s list-versions
curl https://manticoresearch.com | sh -s version 25.0.0
curl https://manticoresearch.com | sh -s dev
curl https://manticoresearch.com | sh -s upgrade
```

## Docker

Docker image is available on [Docker Hub](https://dockr.ly/33biV0U).

To experiment with Manticore Search in Docker just run:

```
docker run --name manticore --rm -d manticoresearch/manticore && until docker logs manticore 2>&1 | grep -q "accepting connections"; do sleep 1; done && docker exec -it manticore mysql && docker stop manticore
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

### Manual package installation

If you prefer explicit package-manager commands, use the manual instructions below or see the [downloads page](https://manticoresearch.com/install/).

### RPM repo for RHEL/CentOS/Amazon/Oracle/Fedora-like Linux

```
sudo yum install https://repo.manticoresearch.com/manticore-repo.noarch.rpm
sudo yum install manticore
```

or use `dnf` when available:

```
sudo dnf install https://repo.manticoresearch.com/manticore-repo.noarch.rpm
sudo dnf install manticore
```

### APT repo for Ubuntu/Debian/Mint

```
wget https://repo.manticoresearch.com/manticore-repo.noarch.deb
sudo apt install ./manticore-repo.noarch.deb
sudo apt update
sudo apt install manticore
```

### Homebrew on macOS

```
brew install manticoresoftware/tap/manticoresearch manticoresoftware/tap/manticore-extra
```

### Windows

For WSL/WSL2, use the Linux installer inside the WSL distribution. For native Windows, see the [Windows installation instructions](https://manual.manticoresearch.com/Installation/Windows).


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
Should your company require any help - we provide full-cycle services:
  * Audit
  * Support
  * Consulting
  * Development
  * Training

[More details here](https://manticoresearch.com/services/)

# ❤️ How you can support Manticore Search
Manticore Search is an Open Source project with development made possible by support from our core team, contributors, and sponsors. Building premium Open Source software is not easy. If you would like to make sure Manticore Search stays free, here is how you can help the project:

* [Donation through PayPal](https://www.paypal.me/manticoresearch)
* [Become our client and let us help you](https://manticoresearch.com/services)

# License
Manticore Search is distributed under [GPLv3 or later](./LICENSE). Manticore Search uses and re-distributes other open-source components. Please check the [component licenses](./component-licenses) directory for details.
