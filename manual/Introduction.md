# Introduction

Manticore Search is a database designed specifically for search, including full-text search.

Manticore was born in 2017 as a continuation of [Sphinx Search](https://github.com/sphinxsearch/sphinx). We took the best from Sphinx (C++ core and focus on low level data structures and fine-tuned algorithms), added a lot of new functionality, fixed hundreds of bugs, made it easier to use, kept it open source and made Manticore Search even more lightweight & extremely fast database for search.


## Our key features are:
#### Powerful and fast full-text searching
  * Over 20 [full-text operators](https://play.manticoresearch.com/fulltextintro/)<!--{target="_blank"}--> and over 20 ranking factors
  * Custom ranking
  * [Stemming](Creating_an_index/NLP_and_tokenization/Morphology.md)
  * [Lemmatization](Creating_an_index/NLP_and_tokenization/Morphology.md)
  * [Stopwords](Creating_an_index/NLP_and_tokenization/Ignoring_stop-words.md)
  * [Synonyms](Creating_an_index/NLP_and_tokenization/Exceptions.md)
  * [Wordforms](Creating_an_index/NLP_and_tokenization/Wordforms.md)
  * [Advanced tokenization at character and word level](Creating_an_index/NLP_and_tokenization/Low-level_tokenization.md)
  * [Proper Chinese segmentation](Creating_an_index/NLP_and_tokenization/CJK.md)
  * [Text highlighting](Searching/Highlighting.md)

#### SQL-first
The native Manticore's syntax is SQL. It speaks SQL over HTTP and MySQL protocol. You can use your preferred mysql client to connect to Manticore Search server via SQL protocol in any programming language.

#### JSON over HTTP
To provide more programmatic way to manage your data and schemas Manticore provides HTTP JSON protocol. It is very similar to the one from Elasticsearch.

#### Declarative and imperative schema management
You can create / update / delete indexes online as well as providing schemas in a configuration file.

#### Power of C++
Being written fully in C++ Manticore Search starts fast and doesn't take much RAM. Low-level optimizations give good performance.

#### Real-time inserts
After a new document is added or updated it can be read immediately.

#### Interactive courses
We provide [interactive courses](https://play.manticoresearch.com/) for easier learning.

#### ACID compliance
Manticore is not fully ACID-compliant, but it supports isolated transactions for atomic changes and binary logging for safe writes.

#### Built-in replication and load balancing
Data can be distributed across servers and data-centers. Any Manticore Search node can be both a load balancer and a data node. Manticore implements synchronous multi-master [replication](https://play.manticoresearch.com/replication/) using [Galera library](https://galeracluster.com/) which guarantees consistency between all data nodes and no data loss.

#### Can sync from MySQL/PostgreSQL/ODBC/xml/csv out of the box
Manticore `indexer` tool and rich configuration syntax helps to sync existing data from MySQL, PostgreSQL, any datatbase which speaks ODBC and any other technology which can generate a simple XML or CSV.

#### Integrations
You can integrate Manticore Search with MySQL/MariaDB server via a [FEDERATED engine](Extensions/FEDERATED.md) or use Manticore through [ProxySQL](https://manticoresearch.com/2018/06/18/using-proxysql-to-route-inserts-in-a-distributed-realtime-index/)

#### Stream filtering
Manticore has a special index type called "[percolate](Creating_an_index/Local_indexes/Percolate_index.md)" which implements search in reverse when you index your queries rather than data. It's an extremely powerful tool for full-text data stream filtering: just put all your queries in the index, process your data stream by sending each batch of documents to Manticore Search and you'll get only those back that match some of your stored queries.

# Possible applications:
Manticore's possible applications are not limited by, but include:

  * [Full-text search](https://play.manticoresearch.com/fulltextintro/)
    * when used with small data volume you can benefit from powerful full-text search syntax and low RAM consumption (as little as 7-8 megabytes)
    * when used with big data you can benefit from Manticore's high availability capabilities and ability to serve very large indexes, each taking hundreds of gigabytes of RAM
  * [Faceted search](https://play.manticoresearch.com/faceting/)
  * [Geo-spatial search](https://play.manticoresearch.com/geosearch/)
  * [Spell correction](https://play.manticoresearch.com/didyoumean/)
  * [Autocomplete](https://play.manticoresearch.com/simpleautocomplete/)
  * [Data stream filtering](https://play.manticoresearch.com/pq/)
