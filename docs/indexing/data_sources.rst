.. _data_sources:

Data sources
=======================

The data to be indexed can generally come from very different sources:
SQL databases, plain text files, HTML files, mailboxes, and so on. From
Manticore point of view, the data it indexes is a set of structured
*documents*, each of which has the same set of *fields* and
*attributes*. This is similar to SQL, where each row would correspond to
a document, and each column to either a field or an attribute.

Depending on what source Manticore should get the data from, different code
is required to fetch the data and prepare it for indexing. This code is
called *data source driver* (or simply *driver* or *data source* for
brevity).

At the time of this writing, there are built-in drivers for MySQL,
PostgreSQL, MS SQL (on Windows), and ODBC. There is also a generic
driver called xmlpipe2, which runs a specified command and reads the
data from its ``stdout``. See :ref:`xmlpipe2_data_source` section for the format
description. tsvpipe (Tab Separated Values) and csvpipe (Comma Separated
Values) data source also available and described in :ref:`tsvcsv_data_source`.

There can be as many sources per index as necessary. They will be
sequentially processed in the very same order which was specified in
index definition. All the documents coming from those sources will be
merged as if they were coming from a single source.
