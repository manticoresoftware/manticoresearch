.. _data_types:

Data Types
==========

The following data types are supported in Manticore Search:

Document ID
^^^^^^^^^^^

The identificator of a document in the index. Document IDs must be **unique positive signed** 64-bit integers. Document IDs are implicit and have no declaration. Update operation is forbidden on document ids.


Text
^^^^

One or more text columns form the fulltext part of the index.
The text is passed through an analyzer pipeline that converts the text to words, applies morphology transformations etc.
Full-text fields can only be used in MATCH() clause and cannot be used for sorting or aggregation.
Words are stored in an inverted index along with references to the fields they belong and positions in the field.
This allows to search a word inside each field and to use advanced operators like proximity.
By default the original text of the fields is  indexed and stored. Prior 3.3.0, texts could only be indexed.
It's possible to choose to not store the texts for lowering index size, result set size and overall improved query speed.

Starting with version 3.3.2 it's also possible to only store texts and not index them.
A stored-only column text can't be used in fulltext matches, but is returned in results.
Compared to string attributes, a stored-only text is retrieved only for the documents that are send back in the result set.
This make them more suitable than string attributes for big texts or strings that required only to be present in result set, but not used in any operation,
since they don't consume memory.

Text columns currently can only be defined at index creation. To add/remove a text column, the index needs to be reconstructed.

String
^^^^^^^

Unlike full-text fields, the string attributes are stored as they are received and cannot be used in full-text searches.
Instead they are returned in results, they can be used in WHERE clause for literal comparison or REGEX filtering  and they can be used for sorting and aggregation.
In general it's recommended to not store large texts in string attributes, but use string attributes for metadata like  names, titles, tags, keys.
String attributes are stored in blob component, along with MVAs and JSON attributes, for which it can be set how to be loaded in memory (see :ref:`index_files`).

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

Allows storing JSON objects for schema-less data. JSON properties can be used in most operations and special functions like :ref:`ALL() <expr-func-all>`, :ref:`ANY() <expr-func-any>`, :ref:`GREATEST() <expr-func-greatest>`, :ref:`LEAST() <expr-func-least>` and :ref:`INDEXOF() <expr-func-indexof>` allow traversal of property arrays.

Text properties are treated same as strings so it's not possible to use them in full-text matches expressions, but string functions like :ref:`REGEX <expr-func-regex>` can be used.

In case of JSON properties, enforcing data type is required to be casted in some situations for proper functionality. For example in case of float values :ref:`DOUBLE() <expr-func-double>`  
must be used for proper sorting:

.. code-block:: none

   SELECT * FROM myindex ORDER BY DOUBLE (myjson.myfloat) DESC
   
JSON objects as well as their properties can be tested against NULL with IS (NOT) NULL operator.

JSON attributes are stored in blob component along with string and MVA attributes (see :ref:`index_files`).

Multi-value integer
^^^^^^^^^^^^^^^^^^^

It's a special type that allows storing variable-length lists of 32-bit unsigned integers. It can be used to store one-to-many numeric values like tags, product categories, properties.
It supports filtering and aggregation, but not sorting.  Filtering can made of condition that requires at least one element to pass (using ANY()) or all (using :ref:`ALL() <expr-func-all>`).
Information like least or greatest element and length of the list can be extracted.

MVA attributes are stored in blob component along with string and JSON attributes (see :ref:`index_files`).

Multi-value big integer
^^^^^^^^^^^^^^^^^^^^^^^

It's a special type that allows storing variable-length lists of 64-bit signed integers. It has the same functionality as multi-value integer.
