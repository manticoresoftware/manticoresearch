.. _comment_syntax:

Comment syntax
--------------

SphinxQL supports C-style comment syntax. Everything from an opening
``/*`` sequence to a closing ``*/`` sequence is ignored. Comments can
span multiple lines, can not nest, and should not get logged. MySQL
specific ``/*! ... */`` comments are also currently ignored. (As the
comments support was rather added for better compatibility with
``mysqldump`` produced dumps, rather than improving general query
interoperability between Manticore and MySQL.)

::


    SELECT /*! SQL_CALC_FOUND_ROWS */ col1 FROM table1 WHERE ...

