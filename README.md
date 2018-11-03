# Manticoresearch with CJK Word Segmentation
## Why Word Segmentation for CJK
Manticore (Sphinx) is a great full-text search engine. It supports almost all languages, including CJK (Chinese, Japanese, Korean) supported by its ngram tokenizer.
**Ngram = 1** will splits all CJK to single character, which likes split all English to alphabet. This will makes searching with much lower efficiency.
**Ngram = 2 or greater** will creates much more meaningless “word”, and make the index file much larger, and also makes the speed of indexing, searching lower.
so, a Word Segmentation is necessary for CJK.

Manticore is a folk of Sphinx search and makes some [optimization with performance](https://manticoresearch.com/2018/03/18/sphinx-3-vs-manticore-performance-benchmark/). And Sphinxsearch 3.x is no longer open-sourced at present. So, Manticoresearch is the best to add CJK Word Segmentation.

## Implementation
The algorithm is based on a word dictionary, which implemented with [cedar](http://www.tkl.iis.u-tokyo.ac.jp/~ynaga/cedar/). <code>cedar</code> is C++ implementation of efficiently-updatable double-array trie, which is best for the dictionary. Its License: GNU GPLv2, LGPLv2.1, and BSD;  or e-mail author for other licenses you want.

It use Minium Matching (but not single character) to match the dictionary with string to get words. If it can’t resolve some ambiguity, just make the single character as on word, this mechanism can guarantee search engine find out content without missing.

## Installation
Same as the original Manticore, see the [Docs](https://docs.manticoresearch.com/latest/html/)

## Usage
**1. prepare your words**
Put all your words into an .txt file, one word per line, which likes:

>\# words.txt
>中文
>中国語
>중국어

**2. create your dictionary**
You will get <code>make_segdictionary</code> after building secceed. run the command:
```shell
./make_segdictionary words.txt words.dict
```
Now, you get the dictionary: words.dict

**3. configure**
Just add one line to the <code>index {...} block</code> in your config file:
```config
index {
    ...
    seg_dictionary = path-to-your-segmentation-words-dictionary
    ...
}
```
**remind:** It works for both of plain index and real-time index.

------------------------------------------------------------------------------------------------------------------------------
---

[![manticoresearch](https://manticoresearch.com/wp-content/uploads/2018/03/manticoresearch.png)](https://manticoresearch.com)

-----------------

[![Docs](https://img.shields.io/badge/docs-latest-brightgreen.svg)](https://docs.manticoresearch.com/latest/html/)
[![Release](https://img.shields.io/github/release/manticoresoftware/manticore.svg)](https://github.com/manticoresoftware/manticore/releases)
[![GitHub last commit](https://img.shields.io/github/last-commit/manticoresoftware/manticore.svg)](https://github.com/manticoresoftware/manticore/commits/master)
[![License](https://img.shields.io/github/license/manticoresoftware/manticore.svg?maxAge=2592000)](https://github.com/adriannuta/manticore/blob/master/COPYING)
[![Slack][slack-badge]][slack-url]
[![Twitter](https://img.shields.io/twitter/follow/manticoresearch.svg?style=social&label=Follow)](https://twitter.com/manticoresearch)

### Docker image

[![Docker Automated build](https://img.shields.io/docker/automated/manticoresearch/manticore.svg)](https://hub.docker.com/r/manticoresearch/manticore/)
[![MicroBadger](https://images.microbadger.com/badges/image/manticoresearch/manticore.svg)](https://microbadger.com/images/manticoresearch/manticore)
[![Docker Stars](https://img.shields.io/docker/stars/manticoresearch/manticore.svg)](https://hub.docker.com/r/manticoresearch/manticore/)

## Introduction

Manticore Search is an open source search server designed to be fast, scalable and with powerful and accurate full-text search capabilities. It is a fork of popular search engine Sphinx.

[![IMAGE ALT TEXT](http://img.youtube.com/vi/-5lB6_L28gw/0.jpg)](http://www.youtube.com/watch?v=-5lB6_L28gw "Introduction to Manticore Search")

## Features
* Over 20 full-text operators and over 20 ranking factors, custom rankers
* Advanced tokenization at character and word level
* Morphology preprocessors
* Real-time and offline indexes
* JSON attributes
* Distributed indexes
* Built-in load balancer
* Text highlighting and word correction
* Geo search
* Source data connectors for MySQL, PostgreSQL, MSSQL, ODBC, XML and CSV files
* Connectivity: MySQL-based protocol, HTTP, native libraries


## Installation

### Docker images
Docker images are available on [Docker Hub](https://hub.docker.com/r/manticoresearch/manticore/).

To launch a Manticore Search container run

    docker run --name manticore -p 9306:9306 -d manticoresearch/manticore

### Precompiled binaries
Packages of latest GA release can be downloaded from http://www.manticoresearch.com/downloads

    $ wget https://github.com/manticoresoftware/manticore/releases/download/x.y.z/manticore_z.y.z.deb
    $ sudo dpkg -i manticore_x.y.z.deb
    $ systemctl manticore start

For more details see [installation](https://docs.manticoresearch.com/latest/html/installation.html#installation).

### Compiling from sources
If you like to use the latest code, you can build Manticore easy from sources:

     git clone https://github.com/manticoresoftware/manticore.git
     cd manticore
     mkdir build && cd build
     cmake .. &&  make install

For more details check [compiling](https://docs.manticoresearch.com/latest/html/installation.html#compiling-manticore-from-source). 
## Getting started
 The quick way to see Manticore in action is to use the sample RT index preconfigured in sphinx.conf.
 Inserting data in a RT index is done using SphinxQL and in the same way as for a MySQL table
 
      $ mysql -h 127.0.0.1 -P 9306
      mysql> INSERT INTO rt VALUES ( 1, 'first record', 'test one', 123 );
      Query OK, 1 row affected (0.05 sec)
      mysql> INSERT INTO rt VALUES ( 2, 'second record', 'test two', 234 );
      Query OK, 1 row affected (0.00 sec)
 To search we use a SELECT statement with a MATCH clause:
 
     mysql> SELECT * FROM rt WHERE MATCH('test');
     +------+--------+------+
     | id   | weight | gid  |
     +------+--------+------+
     |    1 |   1643 |  123 |
     |    2 |   1643 |  234 |
     +------+--------+------+
     2 rows in set (0.01 sec)
    
## Documentation and support

[Documentation](https://docs.manticoresearch.com)

[Manticore Community Forum](http://forum.manticoresearch.com/)

[Bug tracker](https://github.com/manticoresoftware/manticore/issues)

[Professional Support](https://manticoresearch.com/professional-support/)

## Contributing
Refer to  [CONTRIBUTING.md](https://github.com/manticoresoftware/manticore/blob/master/CONTRIBUTING.md)

## License
Manticore Search is released under GPLv2, for more information check [COPYING](https://github.com/manticoresoftware/manticore/blob/master/COPYING)

[![Analytics](https://ga-beacon.appspot.com/UA-114439919-1/manticoresoftware/manticore/README.md?pixel&useReferer)](https://github.com/manticoresoftware/manticore)

[slack-url]: https://slack.manticoresearch.com/
[slack-badge]:  https://img.shields.io/badge/Slack-join%20chat-green.svg