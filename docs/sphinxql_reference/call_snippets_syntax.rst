.. _call_snippets_syntax:

CALL SNIPPETS syntax
--------------------

.. code-block:: mysql


    CALL SNIPPETS(data, index, query[, opt_value AS opt_name[, ...]])

CALL SNIPPETS statement builds a snippet from provided data and query,
using specified index settings.

``data`` is the source data to extract a snippet from. It could be a
single string, or the list of the strings enclosed in curly brackets.
``index`` is the name of the index from which to take the text
processing settings. ``query`` is the full-text query to build snippets
for. Additional options are documented in :ref:`build_excerpts`. Usage
example:

.. code-block:: mysql


    CALL SNIPPETS('this is my document text', 'test1', 'hello world',
        5 AS around, 200 AS limit);
    CALL SNIPPETS(('this is my document text','this is my another text'), 'test1', 'hello world',
        5 AS around, 200 AS limit);
    CALL SNIPPETS(('data/doc1.txt','data/doc2.txt','/home/sphinx/doc3.txt'), 'test1', 'hello world',
        5 AS around, 200 AS limit, 1 AS load_files);

