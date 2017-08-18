.. _alter_syntax:

ALTER syntax
------------

::


    ALTER TABLE index {ADD|DROP} COLUMN column_name [{INTEGER|INT|BIGINT|FLOAT|BOOL|MULTI|MULTI64|JSON|STRING}]

It supports adding one attribute at a time for both plain and RT
indexes. The int, bigint, float, bool, multi-valued, multi-valued 64bit,
json and string attribute types are supported. You can add json and
string attributes, but you cannot modify their values.

Implementation details. The querying of an index is impossible (because
of a write lock) while adding a column. This may change in the future.
The newly created attribute values are set to 0. ALTER will not work for
distributed indexes and indexes without any attributes. DROP COLUMN will
fail if an index has only one attribute.

::


    ALTER RTINDEX index RECONFIGURE

ALTER can also reconfigure an existing RT index, so that new
tokenization, morphology, and other text processing settings from
sphinx.conf take effect on the newly INSERT-ed rows, while retaining the
existing rows as they were. Internally, it forcibly saves the current
RAM chunk as a new disk chunk, and adjusts the index header, so that the
new rows are tokenized using the new rules. Note that as the queries are
currently parsed separately for every disk chunk, this might result in
warnings regarding the keyword sets mismatch.

::


    mysql> desc plain;
    +------------+-----------+
    | Field      | Type      |
    +------------+-----------+
    | id         | bigint    |
    | text       | field     |
    | group_id   | uint      |
    | date_added | timestamp |
    +------------+-----------+
    4 rows in set (0.01 sec)

    mysql> alter table plain add column test integer;
    Query OK, 0 rows affected (0.04 sec)

    mysql> desc plain;
    +------------+-----------+
    | Field      | Type      |
    +------------+-----------+
    | id         | bigint    |
    | text       | field     |
    | group_id   | uint      |
    | date_added | timestamp |
    | test       | uint      |
    +------------+-----------+
    5 rows in set (0.00 sec)

    mysql> alter table plain drop column group_id;
    Query OK, 0 rows affected (0.01 sec)

    mysql> desc plain;
    +------------+-----------+
    | Field      | Type      |
    +------------+-----------+
    | id         | bigint    |
    | text       | field     |
    | date_added | timestamp |
    | test       | uint      |
    +------------+-----------+
    4 rows in set (0.00 sec)

