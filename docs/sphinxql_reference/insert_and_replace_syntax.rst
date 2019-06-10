.. _insert_and_replace_syntax:

INSERT and REPLACE syntax
-------------------------

.. code-block:: none


    {INSERT | REPLACE} INTO index [(column, ...)]
        VALUES (value, ...)
        [, (...)]

INSERT statement is only supported for RT and percolate indexes. It inserts new rows
(documents) into an existing index, with the provided column values.

``index`` is the name of RT or percolate index into which the new row(s) should be
inserted. The optional column names list lets you only explicitly
specify values for some of the columns present in the index. All the
other columns will be filled with their default values (0 for scalar
types, empty string for text types).

Expressions are not currently supported in INSERT and values should be
explicitly specified.

RT index INSERT features
~~~~~~~~~~~~~~~~~~~~~~~~

ID column is mandatory for RT indexes. Rows with duplicate IDs will
**not** be overwritten by INSERT; use REPLACE to do that. REPLACE
works exactly like INSERT, except that if an old row has the same ID as
a new row, the old row is deleted before the new row is inserted.

Multiple rows can be inserted using a single INSERT statement by
providing several comma-separated, parentheses-enclosed lists of rows
values.


Percolate index INSERT features
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

For percolate indexes INSERT is used to store queries (aka PQ rules) and their meta (id, tags), so the schema is predefined and may include only the following
columns:

 * ``id`` - numeric id of stored query (if omited, will be assigned automatically)
 * ``query`` - full-text query to store
 * ``filters`` - filters to store (without ``query`` will define the full query as full-scan)
 * ``tags`` - string with one or many comma-separated tags, which may be used to selectively show/delete saved queries.

All other names for columns are not supported and will trigger an error.

.. code-block:: sql

 INSERT INTO pq (id, query, filters) VALUES ( 1, 'filter test', 'gid >= 10' )
 INSERT INTO index_name (query) VALUES ( 'full text query terms' );
 INSERT INTO index_name (query, tags, filters) VALUES ( 'full text query terms', 'tags', 'filters' );

In case of omitted schema ``INSERT`` expects one or two params, first is full-text ``query``, and second (optional)
is ``tags``. ``id`` in this case will be generated automatically (maximum current id in the index + 1), ``filters`` will be empty.

.. code-block:: sql

 INSERT INTO index_name VALUES ( 'full text query terms', 'tags');
 INSERT INTO index_name VALUES ( 'full text query terms');

Multiple rows can be inserted using a single INSERT statement by
providing several comma-separated, parentheses-enclosed lists of rows
values.

Also, you can insert values only into local percolate index. Distributed percolate (i.e. distributed index built from percolate agents/locals) is not
supported for INSERTs yet.
