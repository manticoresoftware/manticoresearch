# Introduction

Manticore Search is a multi-storage database designed specifically for search, including full-text search.

Manticore Search is an [open-source database](https://github.com/manticoresoftware/manticoresearch/) that was created in 2017 as a continuation of [Sphinx Search](https://sphinxsearch.com/) engine. We took all the best from it, significantly improved its functionality, [fixed hundreds of bugs](https://manual.manticoresearch.com/Changelog), rewrote the code almost completely and kept it open-source! That all has made Manticore Search a modern, fast, light-weight and full-featured database with outstanding full-text search capabilities.

## Our key features are:
#### Powerful and fast full-text searching which works fine for small and big datasets
  * Over 20 [full-text operators](https://play.manticoresearch.com/fulltextintro/)<!--{target="_blank"}--> and over 20 ranking factors
  * Custom ranking
  * [Stemming](Creating_a_table/NLP_and_tokenization/Morphology.md)
  * [Lemmatization](Creating_a_table/NLP_and_tokenization/Morphology.md)
  * [Stopwords](Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md)
  * [Synonyms](Creating_a_table/NLP_and_tokenization/Exceptions.md)
  * [Wordforms](Creating_a_table/NLP_and_tokenization/Wordforms.md)
  * [Advanced tokenization at character and word level](Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md)
  * [Proper Chinese segmentation](Creating_a_table/NLP_and_tokenization/CJK.md)
  * [Text highlighting](Searching/Highlighting.md)

#### Modern MPP architecture
Modern MPP architecture and smart query parallelization capabilities allow to fully utilize all your CPU cores to lower response time as much as possible, when needed.

#### Row-wise storage
Traditional row-wise storage for small, medium and big size datasets.

#### Column-oriented storage
Columnar storage support via the [Manticore Columnar Library](https://github.com/manticoresoftware/columnar/) for bigger datasets (much bigger than can fit in RAM).

#### SQL-first
The native Manticore's syntax is SQL. It speaks SQL over HTTP and MySQL protocol. You can use your preferred mysql client to connect to Manticore Search server via SQL protocol in any programming language.

#### JSON over HTTP
To provide more programmatic way to manage your data and schemas Manticore provides [HTTP JSON](Searching/Full_text_matching/Basic_usage.md#HTTP-JSON) protocol. It is very similar to the one from Elasticsearch.

#### Declarative and imperative schema management
You can create / update / delete tables online as well as providing schemas in a configuration file.

#### Power of C++
Being written fully in C++, Manticore Search starts fast and doesn't take much RAM. Low-level optimizations give good performance.

#### Real-time inserts
After a new document is added or updated it can be read immediately.

#### Interactive courses
We provide [interactive courses](https://play.manticoresearch.com/) for easier learning.

#### ACID compliance
Manticore is not fully ACID-compliant, but it supports isolated transactions for atomic changes and binary logging for safe writes.

#### Built-in replication and load balancing
Data can be distributed across servers and data-centers. Any Manticore Search node can be both a load balancer and a data node. Manticore implements virtually synchronous multi-master [replication](https://play.manticoresearch.com/replication/) using [Galera library](https://galeracluster.com/) which guarantees consistency between all data nodes, no data loss and gives good replication performance.

#### Can sync from MySQL/PostgreSQL/ODBC/xml/csv out of the box
Manticore `indexer` tool and rich configuration syntax helps to sync existing data from MySQL, PostgreSQL, any database which speaks ODBC and any other technology which can generate a simple XML or CSV.

#### Integrations
You can integrate Manticore Search with MySQL/MariaDB server via a [FEDERATED engine](Extensions/FEDERATED.md) or use Manticore through [ProxySQL](https://manticoresearch.com/2018/06/18/using-proxysql-to-route-inserts-in-a-distributed-realtime-index/)

#### Stream filtering
Manticore has a special table type called "[percolate](Creating_a_table/Local_tables/Percolate_table.md)" which implements search in reverse when you table your queries rather than data. It's an extremely powerful tool for full-text data stream filtering: just put all your queries in the table, process your data stream by sending each batch of documents to Manticore Search and you'll get only those back that match some of your stored queries.

# Possible applications:
Manticore's possible applications are not limited by, but include:

  * [Full-text search](https://play.manticoresearch.com/fulltextintro/)
    * when used with small data volume you can benefit from powerful full-text search syntax and low RAM consumption (as little as 7-8 megabytes)
    * when used with big data you can benefit from Manticore's high availability capabilities and ability to serve very large tables, each taking hundreds of gigabytes of RAM
  * OLAP: using Manicore Search + [Manticore Columnar Library](https://github.com/manticoresoftware/columnar), you can handle terabytes of data on a single or multiple servers for data analysis
  * [Faceted search](https://play.manticoresearch.com/faceting/)
  * [Geo-spatial search](https://play.manticoresearch.com/geosearch/)
  * [Spell correction](https://play.manticoresearch.com/didyoumean/)
  * [Autocomplete](https://play.manticoresearch.com/simpleautocomplete/)
  * [Data stream filtering](https://play.manticoresearch.com/pq/)
