.. _update_syntax:

UPDATE syntax
-------------

.. code-block:: mysql


    UPDATE index SET col1 = newval1 [, ...] WHERE where_condition [OPTION opt_name = opt_value [, ...]]

Multiple attributes and values can be specified in a single statement.
Both RT and disk indexes are supported.

All attributes types (int, bigint, float, strings, MVA, JSON) can be dynamically updated.

.. warning::
  Note that document ``id`` attribute cannot be updated.
  
``where_condition`` has the same syntax as in the SELECT statement (see
:ref:`select_syntax` for
details).

When assigning the out-of-range values to 32-bit attributes, they will
be trimmed to their lower 32 bits without a prompt. For example, if you
try to update the 32-bit unsigned int with a value of 4294967297, the
value of 1 will actually be stored, because the lower 32 bits of
4294967297 (0x100000001 in hex) amount to 1 (0x00000001 in hex).

MVA values sets for updating (and also for INSERT or REPLACE, refer to
:ref:`insert_and_replace_syntax`) must be specified as
comma-separated lists in parentheses. To erase the MVA value, just
assign () to it.

UPDATE can be used to perform partial JSON updates on numeric data types or arrays of numeric data types. For updating other data types or changing type of a property a full update of the JSON attribute is required.

.. code-block:: mysql


    mysql> UPDATE myindex SET enabled=0 WHERE id=123;
    Query OK, 1 rows affected (0.00 sec)

    mysql> UPDATE myindex
      SET bigattr=-100000000000,
        fattr=3465.23,
        mvattr1=(3,6,4),
        mvattr2=()
      WHERE MATCH('something') AND enabled=1;
    Query OK, 148 rows affected (0.01 sec)

OPTION clause. This is a Manticore specific extension that lets you control
a number of per-update options. The syntax is:

.. code-block:: mysql


    OPTION <optionname>=<value> [ , ... ]

The list of allowed options are the same as for
:ref:`SELECT <select_syntax>` statement. Specifically for UPDATE
statement you can use these options:

-  ‘ignore_nonexistent_columns’ - If set to **1** points that the update will silently
   ignore any warnings about trying to update a column which is not
   exists in current index schema. Default value is **0**.

-  ‘strict’ - this option is used while partial updating JSON attributes. 
   By default (strict=1), UPDATE will end in an error if the UPDATE query tries to perform an update on non-numeric properties.
   With strict=0 if multiple properties are updated and some are not allowed, the UPDATE will not end in error and will perform the changes only on allowed properties
   (with the rest being ignored). If none of the SET changes of the UPDATE are not permitted, the command will end in an error even with strict=0.
