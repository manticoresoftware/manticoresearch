Introduction
============

Manticore Search is a solution for building high-performance search applications. It is a fork of `Sphinx search <http://www.sphinxsearch.com>`__ engine and, as it’s predecesor, it’s written in C++ and distributed under open source license GPL version 2.

While most modern databases include full-text indexes, they have two issues:

* they are not fast enough. While some can perform well on simple searches, their performance get hit when doing complex text searches or when adding other filtering
* the full-text capabilities are limited, which can be the lack of ability to customize the tokenization process or lack of operators that can perform advanced matching

hence the need for a dedicated full-text search engine, which can be fast but at the same time offer advanced features.

In most cases, it is used to assist a traditional database by replacing the full-text search operations and in some cases it can help on non-full-text operations as well. A special utility is shipped along with the main component which can grab data from various sources (databases, XML or TSV/CSV pipelines) and build the indexes.

Manticore Search is similar in usage to a database: an application connects to it, performs queries, gets results, returns them to end user.

.. figure:: images/manticore_database_flow_1.png
   :scale: 50 %
   :align: center
   :alt: Typical flow
   
   Typical flow with Manticore and a database

Data can be also be fed into indexes like in a traditional database, in which case indexing is near real-time, without any dependency or connection to a specific database. Regardless of how data is indexed, there is no difference at searching.

Indexes are similar to a data table, with rows called and viewed as documents and columns as fields and attributes. JSON attribute/column is available for schemaless data, however full-text matching is not yet available on it. Manticore Search offers several ways of connecting:

* an own implementation of the MySQL network protocol, using a small SQL subset called SphinxQL. Any MySQL client/library can be used. Currently this is the recommended interface.
* an HTTP API , which can accept search queries in SQL, native or JSON format
* a native API, with official and 3rd party clients available for many languages
* a proxy storage engine for MySQL called SphinxSE

In addition to regular text searching, Manticore can also perform inverse search (percolate queries), hit highlighting, word corrections, attribute faceting, geospatial searches. Manticore is designed to scale by supporting distributing a data collection across multiple servers and offer an internal load-balancer for High Availability.
