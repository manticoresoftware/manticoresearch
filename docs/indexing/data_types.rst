.. _data_types:

Data Types
==========

The following data types are supported in Manticore Search:

Text
^^^^

It is the full-text field part of the index. The content of these fields is indexed and not stored in the original form.
The text is passed through an analyzer pipeline that converts the text to words, applies morphology transformations etc.
Full-text fields can only be used in MATCH() clause, they are not returned in the result set and cannot be used for sorting or aggregation.
Words are stored in an inverted index along with references to the fields they belong and positions in the field.
This allows to search a word inside each field and to use advanced operators like proximity.

String
^^^^^^^

Unlike full-text fields, the string attributes are stored as they are received and cannot be used in full-text searches.
Instead they are returned in results, they can be used in WHERE clause for comparison filtering or REGEX and they can be used for sorting and aggregation.
In general it's recommended to not store large texts in string attributes, but use string attributes for metadata like  names, titles, tags, keys.

Integer
^^^^^^^

Integer type allows 32 bit **unsigned** integer values. Integers can be stored in shorter sizes than 32 bit by specifying a bit count.
For example if we want to store a numeric value which we know is not going to be bigger than 8, the integer can be defined as

.. code-block:: none

   sql_attr_uint = myattr:3

Bitcount integers perform slowly than the full size ones, but they require less RAM.
The bitcount integers are saved in 32-bit chunks, to save space they should be grouped at the end of attributes definitions (otherwise a bitcount integer between 2 full-size integers will occupy 32 bits as well).


Big Integer
^^^^^^^^^^^

Big integers are 64-bit wide **signed** integers.

Timestamps
^^^^^^^^^^

Timestamp allows to represent a unix timestamp which is stored as a 32-bit integer. For timestamps a family of date and time functions are :ref:`available<date_and_time_functions>`.

Float
^^^^^

Real numbers can be stored as 32-bit IEEE 754 single precision floats. Unlike integer types, equal comparison of floats is forbidden due to rounding errors.
A near equal can be used instead, by checking the absolute error margin:

.. code-block:: none

   SELECT  ABS(a-b) <= 0.00001


where 0.00001 is a chosen accepted tolerance.

Another alternative, which can also be used to perform ``IN(attr,val1,val2,val3)`` is to compare floats as integers by choosing a multiplier factor and convert the floats to integers in operations.
For example:

.. code-block:: none

   SELECT IN(attr,2.0,2.5,3.5) ...


can be made as

.. code-block:: none

   SELECT IN(CEIL(attr*100),200,250,350) ....


JSON
^^^^

Allows storing JSON objects for schema-less data. JSON properties can be used in most operations and special functions like :ref:`ALL() <expr-func-all>`, :ref:`ANY() <expr-func-any>`,:ref:`GREATEST() <expr-func-greatest>`,:ref:`LEAST() <expr-func-least>` and :ref:`INDEXOF() <expr-func-indexof>` allow traversal of property arrays.
Text properties are treated same as strings (so it's not possible to use them in full-text matches).

Multi-value integer
^^^^^^^^^^^^^^^^^^^

It's a special type that allows storing variable-length lists of 32-bit unsigned integers. It can be used to store one-to-many numeric values like tags, product categories, properties.
It supports filtering and aggregation, but not sorting.  Filtering can made of condition that requires at least one element to pass (using ANY()) or all (using :ref:`ALL() <expr-func-all>`).
Information like least or greatest element and length of the list can be extracted.


Multi-value big integer
^^^^^^^^^^^^^^^^^^^^^^^

It's a special type that allows storing variable-length lists of 64-bit signed integers. It has the same functionality as multi-value integer.
