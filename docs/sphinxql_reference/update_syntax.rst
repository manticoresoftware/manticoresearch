.. _update_syntax:

UPDATE syntax
-------------

::


    UPDATE index SET col1 = newval1 [, ...] WHERE where_condition [OPTION opt_name = opt_value [, ...]]

Multiple attributes and values can be specified in a single statement.
Both RT and disk indexes are supported.

All attributes types (int, bigint, float, MVA), except for strings and
JSON attributes, can be dynamically updated.

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

UPDATE can be used to update integer and float values in JSON array. No
strings, arrays and other types yet.

::


    mysql> UPDATE myindex SET enabled=0 WHERE id=123;
    Query OK, 1 rows affected (0.00 sec)

    mysql> UPDATE myindex
      SET bigattr=-100000000000,
        fattr=3465.23,
        mvattr1=(3,6,4),
        mvattr2=()
      WHERE MATCH('hehe') AND enabled=1;
    Query OK, 148 rows affected (0.01 sec)

OPTION clause. This is a Manticore specific extension that lets you control
a number of per-update options. The syntax is:

::


    OPTION <optionname>=<value> [ , ... ]

The list of allowed options are the same as for
:ref:`SELECT <select_syntax>` statement. Specifically for UPDATE
statement you can use these options:

-  ‘ignore_nonexistent_columns’ - points that the update will silently
   ignore any warnings about trying to update a column which is not
   exists in current index schema.

   ‘strict’ - this option is used while updating JSON attributes. It's
   possible to update just some types in JSON. And if you try to update,
   for example, array type you'll get error with ‘strict’ option on and
   warning otherwise.
