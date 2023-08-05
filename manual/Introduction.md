# Introduction

Manticore Search is a multi-storage database specifically designed for search, with robust full-text search capabilities.

As an open-source database (available on [GitHub](https://github.com/manticoresoftware/manticoresearch/)), Manticore Search was created in 2017 as a continuation of [Sphinx Search](https://sphinxsearch.com/) engine. Our development team took all the best features of Sphinx and significantly improved its functionality, fixing hundreds of bugs along the way (as detailed in our [Changelog](https://manual.manticoresearch.com/Changelog)). With nearly complete code rewrites, Manticore Search is now a modern, fast, and light-weight database with full features and exceptional full-text search capabilities.

## Our key features are:
#### Powerful and fast full-text searching that works well for small and large datasets

  * Over 20 [full-text operators](https://play.manticoresearch.com/fulltextintro/)<!--{target="_blank"}-->  and over 20 ranking factors
  * Custom ranking
  * [Stemming](Creating_a_table/NLP_and_tokenization/Morphology.md)
  * [Lemmatization](Creating_a_table/NLP_and_tokenization/Morphology.md)
  * [Stopwords](Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md)
  * [Synonyms](Creating_a_table/NLP_and_tokenization/Exceptions.md)
  * [Wordforms](Creating_a_table/NLP_and_tokenization/Wordforms.md)
  * [Advanced tokenization at character and word level](Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md)
  * [Proper Chinese segmentation](Creating_a_table/NLP_and_tokenization/CJK.md)
  * [Text highlighting](Searching/Highlighting.md)

#### Multithreading
Manticore Search utilizes a smart query parallelization to lower response time and fully utilize all CPU cores when needed.

#### Cost-based query optimizer
The cost-based query optimizer uses statistical data about the indexed data to evaluate the relative costs of different execution plans for a given query. This allows the optimizer to determine the most efficient plan for retrieving the desired results, taking into account factors such as the size of the indexed data, the complexity of the query, and the available resources.

#### Storage options
Manticore offers both [row-wise and column-oriented storage options](Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) to accommodate datasets of various sizes. The traditional and default row-wise storage option is available for datasets of all sizes - small, medium, and large, while the columnar storage option is provided through the Manticore Columnar Library for even larger datasets. The key difference between these storage options is that row-wise storage requires all attributes (excluding full-text fields) to be kept in RAM for optimal performance, while columnar storage does not, thus offering lower RAM consumption, but with a potential for slightly slower performance (as demonstrated by the statistics on https://db-benchmarks.com/).

#### Automatic secondary indexes
[Manticore Columnar Library](https://github.com/manticoresoftware/columnar/) uses [Piecewise Geometric Model index](https://github.com/gvinciguerra/PGM-index), which exploits a learned mapping between the indexed keys and their location in memory. The succinctness of this mapping, coupled with a peculiar recursive construction algorithm, makes the PGM-index a data structure that dominates traditional indexes by orders of magnitude in space while still offering the best query and update time performance. Secondary indexes are ON by default for all numeric fields.

#### SQL-first
Manticore's native syntax is SQL and it supports SQL over HTTP and MySQL protocol, allowing for connection through popular mysql clients in any programming language.

#### JSON over HTTP
For a more programmatic approach to managing data and schemas, Manticore provides [HTTP JSON](Searching/Full_text_matching/Basic_usage.md#HTTP-JSON) protocol, similar to that of Elasticsearch.

#### Elasticsearch-compatible writes
You can execute Elasticsearch-compatible [insert](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Adding-documents-to-a-real-time-table) and [replace](Data_creation_and_modification/Updating_documents/REPLACE.md#REPLACE) JSON queries which enables using Manticore with tools like Logstash (version < 7.13), Filebeat and other tools from the Beats family.

#### Declarative and imperative schema management
Easily create, update, and delete tables online or through a configuration file.

#### The benefits of C++ and the convenience of PHP
The Manticore Search daemon is developed in C++, offering fast start times and efficient memory utilization. The utilization of low-level optimizations further boosts performance. Another crucial component, called [Manticore Buddy](https://github.com/manticoresoftware/manticoresearch-buddy), is written in PHP and is utilized for high-level functionality that does not require lightning-fast response times or extremely high processing power. Although contributing to the C++ code may pose a challenge, adding a new SQL/JSON command using Manticore Buddy should be a straightforward process.

#### Real-Time inserts
Newly added or updated documents can be immediately read.

#### Interactive courses for easy learning
We offer [free interactive courses](https://play.manticoresearch.com/) to make learning effortless.

#### Transactions
While Manticore is not fully ACID-compliant, it supports isolated transactions for atomic changes and binary logging for safe writes.

#### Built-In replication and load balancing
Data can be distributed across servers and data centers with any Manticore Search node acting as both a load balancer and a data node. Manticore implements virtually synchronous multi-master [replication](https://play.manticoresearch.com/replication/) using the [Galera library](https://galeracluster.com/), ensuring data consistency across all nodes, preventing data loss, and providing exceptional replication performance.

#### Built-in backup capabilities
Manticore is equipped with an external tool [manticore-backup](Securing_and_compacting_a_table/Backup_and_restore.md), and the [BACKUP](Securing_and_compacting_a_table/Backup_and_restore.md#BACKUP-SQL-command-reference) SQL command to simplify the process of backing up and restoring your data. Alternatively, you can use [mysqldump](https://dev.mysql.com/doc/refman/8.0/en/mysqldump.html) to [make logical backups](../Securing_and_compacting_a_table/Backup_and_restore.md#Backup-and-restore-with-mysqldump).

#### Out-of-the-box data sync
The `indexer` tool and comprehensive configuration syntax of Manticore make it easy to sync data from sources like MySQL, PostgreSQL, ODBC-compatible databases, XML, and CSV.

#### Integration options
You can integrate Manticore Search with a MySQL/MariaDB server using the [FEDERATED engine](Extensions/FEDERATED.md) or via [ProxySQL](https://manticoresearch.com/2018/06/18/using-proxysql-to-route-inserts-in-a-distributed-realtime-index/).

You can use [Apache Superset](https://manticoresearch.com/blog/manticoresearch-apache-superset-integration/) and [Grafana](https://manticoresearch.com/blog/manticoresearch-grafana-integration/) to visualize data stored in Manticore. Various MySQL tools can be used to develop Manticore queries interactively, such as [HeidiSQL](https://www.heidisql.com/) and [DBForge](https://www.devart.com/dbforge/).

#### Stream filtering made easy
Manticore offers a special table type, the "[percolate](Creating_a_table/Local_tables/Percolate_table.md)" table, which allows you to search queries instead of data, making it an efficient tool for filtering full-text data streams. Simply store your queries in the table, process your data stream by sending each batch of documents to Manticore Search, and receive only the results that match your stored queries.

# Possible applications:
Manticore has a variety of use cases, including:

  * [Full-text search](https://play.manticoresearch.com/fulltextintro/)
    * With small data volumes, enjoy the benefits of powerful full-text search syntax and low memory consumption (as low as 7-8 MB).
    * With large data, benefit from Manticore's high availability and ability to handle massive tables.
  * OLAP: Use Manticore Search and the [Manticore Columnar Library](https://github.com/manticoresoftware/columnar) to analyze terabytes of data on a single or multiple servers.
  * [Faceted search](https://play.manticoresearch.com/faceting/)
  * [Geo-spatial search](https://play.manticoresearch.com/geosearch/)
  * [Spell correction](https://play.manticoresearch.com/didyoumean/)
  * [Autocomplete](https://play.manticoresearch.com/simpleautocomplete/)
  * [Data stream filtering](https://play.manticoresearch.com/pq/)

# Requirements

* Architecture: arm64 or x86_64
* OS: Debian-based (e.g. Debian, Ubuntu, Mint), RHEL-based (e.g. RHEL, CentOS, Alma, Oracle Linux, Amazon Linux), Windows, or MacOS.
* [Manticore Columnar Library](https://github.com/manticoresoftware/columnar), which provides [columnar storage](Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) and [secondary indexes](Introduction.md#Automatic-secondary-indexes), requires a CPU with SSE >= 4.2.
* No specific disk space or RAM requirements are needed. An empty Manticore Search instance only uses around 40MB of RSS RAM.

<!-- proofread -->
