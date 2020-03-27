<p align="center">
  <a href="https://manticoresearch.com" target="_blank" rel="noopener">
    <img src="https://manticoresearch.com/wp-content/uploads/2019/12/manticore-logo-central-M-1.png" width="140" alt="Manicore Search Logo">
  </a>
</p>

<h1 align="center">
  Manticore Search 3.4.0
</h1>

<h3 align="center">
  <a href="https://manticoresearch.com">Website</a> • 
  <a href="http://bit.ly/2Q9uGj4">Downloads</a> • 
  <a href="https://docs.manticoresearch.com">Docs</a> • 
  <a href="https://play.manticoresearch.com">Courses</a> • 
  <a href="https://forum.manticoresearch.com">Forum</a> • 
  <a href="https://slack.manticoresearch.com">Chat</a> • 
  <a href="https://twitter.com/manticoresearch">Twitter</a>
</h3>

<p>&nbsp;</p>

# Introduction
Manticore Search is a database designed specifically for search, including full-text search. What differs it from other solutions is:
* Powerful and fast full-text searching which works fine for small and big datasets
* SQL-first: the native Manticore's syntax is SQL. It speaks SQL over HTTP and MySQL protocol (you can use your preferred mysql client)
* JSON over HTTP: to provide more programmatic way to manage your data and schemas Manticore provides HTTP JSON protocol. Very similar to the one from Elasticsearch
* Written fully in C++: starts fast, doesn't take much RAM, low-level optimizations give good performance
* Real-time inserts: after INSERT is made the document can be read immediately
* [Interactive courses](https://play.manticoresearch.com/) for easier learning
* Built-in replication and load balancing
* Can sync from MySQL/PostgreSQL/ODBC/xml/csv out of the box
* Not fully ACID-compliant, but supports transactions and binlog for safe writes

[Craigslist](https://www.craigslist.org/), [Socialgist](https://socialgist.com/), [PubChem](https://pubchem.ncbi.nlm.nih.gov/) and many others use Manticore for efficient searching and stream filtering.

Manticore Search was forked from [Sphinx 2.3.2](https://github.com/sphinxsearch/sphinx) in 2017.

# More features
* Full-text search and relevance:
  - Over 20 [full-text operators](https://play.manticoresearch.com/fulltextintro/) and over 20 ranking factors
  - Custom ranking
* Other search capabilities:
  - [Rich filtering functionality](https://docs.manticoresearch.com/latest/html/searching/expressions,_functions,_and_operators.html)
  - [Faceted search](https://play.manticoresearch.com/faceting/)
  - [Geo-spatial search](https://play.manticoresearch.com/geosearch/)
  - [Spell correction](https://play.manticoresearch.com/didyoumean/)
  - [Autocomplete](https://play.manticoresearch.com/simpleautocomplete/)
  - Wide range of functions for filtering and data manipulation
* NLP:
  - [Stemming](https://docs.manticoresearch.com/latest/html/conf_options_reference/index_configuration_options.html#morphology)
  - [Lemmatization](https://docs.manticoresearch.com/latest/html/conf_options_reference/index_configuration_options.html#morphology)
  - [Stopwords](https://docs.manticoresearch.com/latest/html/conf_options_reference/index_configuration_options.html#stopwords)
  - [Synonyms](https://docs.manticoresearch.com/latest/html/conf_options_reference/index_configuration_options.html#exceptions)
  - [Wordforms](https://docs.manticoresearch.com/latest/html/conf_options_reference/index_configuration_options.html#wordforms)
  - [Advanced tokenization at character and word level](https://docs.manticoresearch.com/latest/html/conf_options_reference/index_configuration_options.html#charset-table)
  - [Proper Chinese segmentation](https://play.manticoresearch.com/icu-chinese/)
  - [Text highlighting](https://play.manticoresearch.com/highlighting/)
* Stream filtering [using a "percolate" index](https://play.manticoresearch.com/pq/)
* High-availability:
  - Data can be distributed across servers and data-centers
  - [Synchronous replication](https://play.manticoresearch.com/replication/)
  - Built-in load balancing
* Security:
  - [https support](https://play.manticoresearch.com/https/)
* Data types:
  - full-text field - inverted index
  - integer and float in-memory numeric fields
  - in-memory "string" for fast filtering
  - on-disk "[stored](https://play.manticoresearch.com/docstore/)" for key-value purpose
  - JSON
  - multi-value attributes
* Integrations:
  - [Sync from MySQL and PostgreSQL](https://docs.manticoresearch.com/latest/html/indexing/sql_data_sources_mysql,_postgresql.html)
  - [Sync from XML](https://docs.manticoresearch.com/latest/html/indexing/xmlpipe2_data_source.html)
  - [Sync from CSV](https://docs.manticoresearch.com/latest/html/indexing/tsvcsv_data_source.html)
  - [With MySQL as a storage engine](https://docs.manticoresearch.com/latest/html/mysql_storage_engine_sphinxse.html)
  - [With MySQL via FEDERATED engine](https://docs.manticoresearch.com/latest/html/federated_storage_engine.html)
  - [ProxySQL](https://manticoresearch.com/2018/06/18/using-proxysql-to-route-inserts-in-a-distributed-realtime-index/)
  
# Installation

### Docker
Docker image is available on [Docker Hub](https://dockr.ly/33biV0U).

To play with Manticore Search in Docker just run:

```
docker run --name manticore --rm -d manticoresearch/manticore && docker exec -it manticore mysql -w && docker stop manticore
```

You can then: create an index, add data and run searches. For example:

```
create table movies(title text, year int) morphology='stem_en' html_strip='1' stopwords='en';

insert into movies(title, year) values ('The Seven Samurai', 1954), ('Bonnie and Clyde', 1954), ('Reservoir Dogs', 1992), ('Airplane!', 1980), ('Raging Bull', 1980), ('Groundhog Day', 1993), ('<a href="http://google.com/">Jurassic Park</a>', 1993), ('Ferris Bueller\'s Day Off', 1986);

select highlight(), year from movies where match('the dog');

select highlight(), year from movies where match('days') facet year;

select * from movies where match('google');
```

When you exit from the mysql client it stops and removes the container, so use this way only for testing / sandboxing purposes. 

Read [the full instruction for the docker image](https://dockr.ly/33biV0U) for more details including our recommendations on running it in production.

### Packages

## [Ubuntu, Debian, Centos, Windows and MacOS packages are here](https://www.manticoresearch.com/downloads).

### YUM repo for RHEL/Centos
```
yum install https://repo.manticoresearch.com/manticore-repo.noarch.rpm
yum install manticore
```

### Homebrew on MacOS
```
brew install manticoresearch
```

### Windows
See [instruction here](https://docs.manticoresearch.com/latest/html/installation.html#installing-manticore-on-windows).

### MacOS .dmg
See [instruction here](https://docs.manticoresearch.com/latest/html/installation.html#installing-manticore-on-macos).


# Documentation and community channels

  * [Interactive courses](https://play.manticoresearch.com)
  * [Documentation](https://docs.manticoresearch.com)
  * [Manticore Community Forum](https://forum.manticoresearch.com/)
  * [Public Slack chat](http://slack.manticoresearch.com/)
  * [Bug tracker](https://github.com/manticoresoftware/manticore/issues)

# How we can support you
Should your company require any help - we provide full-cycle services in the areas of Sphinx and Manticore Search:
  * Audit
  * Support
  * Consulting
  * Development
  * Training

[More details here](https://manticoresearch.com/services/)

# ❤️ How you can support Manticore Search

Manticore Search is a GPLv2-licensed open source project with development made possible by support from our core team, contributors, and sponsors. Building premium open-source software is not easy. If you would like to make sure Manticore Search stays free here is how you can help the project:

* [Donation through PayPal](https://www.paypal.me/manticoresearch)
* [Become our client and let us help you](https://manticoresearch.com/services)

[![Analytics](https://ga-beacon.appspot.com/UA-114439919-1/manticoresoftware/manticore/README.md?pixel&useReferer)](https://github.com/manticoresoftware/manticore)
