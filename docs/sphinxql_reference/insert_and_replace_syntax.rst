.. _insert_and_replace_syntax:

INSERT and REPLACE syntax
-------------------------

::


    {INSERT | REPLACE} INTO index [(column, ...)]
        VALUES (value, ...)
        [, (...)]

INSERT statement is only supported for RT indexes. It inserts new rows
(documents) into an existing index, with the provided column values.

ID column must be present in all cases. Rows with duplicate IDs will
**not** be overwritten by INSERT; use REPLACE to do that. REPLACE
works exactly like INSERT, except that if an old row has the same ID as
a new row, the old row is deleted before the new row is inserted.

``index`` is the name of RT index into which the new row(s) should be
inserted. The optional column names list lets you only explicitly
specify values for some of the columns present in the index. All the
other columns will be filled with their default values (0 for scalar
types, empty string for text types).

Expressions are not currently supported in INSERT and values should be
explicitly specified.

Multiple rows can be inserted using a single INSERT statement by
providing several comma-separated, parentheses-enclosed lists of rows
values.
