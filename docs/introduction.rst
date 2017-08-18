Introduction
============

About
-----

Manticore Search is a full-text search engine, publicly distributed under GPL
version 2, forked from 2.3 branch of open-source search engine Sphinx search.

Technically, Manticore is a standalone software package provides fast and
relevant full-text search functionality to client applications. It was
specially designed to integrate well with SQL databases storing the
data, and to be easily accessed by scripting languages. However, Manticore
does not depend on nor require any specific database to function.

Applications can access Manticore search daemon (searchd) using any of the
following access methods: 
- Manticore own implementation of
MySQL network protocol (using a small SQL subset called SphinxQL, this
is recommended way)
- native search API (SphinxAPI) 
- HTTP protocol
- via MySQL server with a pluggable storage engine (SphinxSE).

Official native SphinxAPI implementations for PHP, Perl, Python, Ruby
and Java are included within the distribution package. API is very
lightweight so porting it to a new language is known to take a few hours
or days. Third party API ports and plugins exist for Perl, C#, Haskell,
Ruby-on-Rails, and possibly other languages and frameworks.

Manticore supports two different indexing backends: “disk” index backend,
and “realtime” (RT) index backend. Disk indexes support online full-text
index rebuilds, but online updates can only be done on non-text
(attribute) data. RT indexes additionally allow for online full-text
index updates.

Data can be loaded into disk indexes using a so-called data source.
Built-in sources can fetch data directly from MySQL, PostgreSQL, MSSQL,
ODBC compliant database (Oracle, etc) or from a pipe in TSV or XML
format. Adding new data sources drivers (eg. to natively support other
DBMSes) is designed to be as easy as possible. RT indexes can only be
populated using SphinxQL.



Manticore features
------------------

Key Manticore features are:

-  high indexing and searching performance;

-  advanced indexing and querying tools (flexible and feature-rich text
   tokenizer, querying language, several different ranking modes, etc);

-  advanced result set post-processing (SELECT with expressions, WHERE,
   ORDER BY, GROUP BY, HAVING etc over text search results);

-  proven scalability up to billions of documents, terabytes of data,
   and thousands of queries per second;

-  easy integration with SQL and XML data sources, and SphinxQL,
   SphinxAPI, or SphinxSE search interfaces;

-  easy scaling with distributed searches.

To expand a bit, Manticore:

-  has high indexing speed (upto 10-15 MB/sec per core on an internal
   benchmark);

-  has high search speed (upto 150-250 queries/sec per core against
   1,000,000 documents, 1.2 GB of data on an internal benchmark);

-  has high scalability (biggest known cluster indexes over
   3,000,000,000 documents, and busiest one peaks over 50,000,000
   queries/day);

-  provides good relevance ranking through combination of phrase
   proximity ranking and statistical (BM25) ranking;

-  provides distributed searching capabilities;

-  provides document excerpts (snippets) generation;

-  provides searching from within application with SphinxQL or SphinxAPI
   interfaces, and from within MySQL with pluggable SphinxSE storage
   engine;

-  supports boolean, phrase, word proximity and other types of queries;

-  supports multiple full-text fields per document (upto 32 by default);

-  supports multiple additional attributes per document (ie. groups,
   timestamps, etc);

-  supports stopwords;

-  supports morphological word forms dictionaries;

-  supports tokenizing exceptions;

-  supports UTF-8 encoding;

-  supports stemming (stemmers for English, Russian, Czech and Arabic
   are built-in; and stemmers for French, Spanish, Portuguese, Italian,
   Romanian, German, Dutch, Swedish, Norwegian, Danish, Finnish,
   Hungarian, are available by building third party `libstemmer
   library <http://snowball.tartarus.org/>`__);

-  supports MySQL natively (all types of tables, including MyISAM,
   InnoDB, NDB, Archive, etc are supported);

-  supports PostgreSQL natively;

-  supports ODBC compliant databases (MS SQL, Oracle, etc) natively;

-  …has 50+ other features not listed here, refer configuration manual!


Where to get Manticore
----------------------

Manticore is available through its official Web site at
http://manticoresearch.com/.

Currently, Manticore distribution tarball includes the following software:

-  ``indexer``: an utility which creates fulltext indexes;

-  ``searchd``: a daemon which enables external software (eg. Web
   applications) to search through fulltext indexes;

-  ``sphinxapi``: a set of searchd client API libraries for popular Web
   scripting languages (PHP, Python, Perl, Ruby).

-  ``spelldump``: a simple command-line tool to extract the items from
   an ``ispell`` or ``MySpell`` (as bundled with OpenOffice) format
   dictionary to help customize your index, for use with
   :ref:`wordforms <wordforms>`.

-  ``indextool``: an utility to dump miscellaneous debug information
   about the index

-  ``wordbreaker``: an utility to break down compound words into
   separate words

   
License
-------

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2 of the License, or (at your
option) any later version. See COPYING file for details.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
59 Temple Place, Suite 330, Boston, MA 02111-1307 USA


Credits
-------

Manticore is derived from Sphinx search engine created  by Andrew Aksyonoff. 
More details about people involved in Sphinx development can be found on this page:   http://sphinxsearch.com/docs/devel.html#credits.

Manticore is developed and maintained by Manticore Software Ltd. 
Current team members:

-  Adrian Nuta
-  Alexey Vinogradov
-  Gloria Vinogradova
-  Ilya Kuznetsov
-  Mindaugas Zukas
-  Sergey Nikolaev
-  Stanislav Klinov
