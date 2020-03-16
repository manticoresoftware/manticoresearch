.. _show_create_table_syntax:

SHOW CREATE TABLE syntax
------------------------

.. code-block:: none


    SHOW CREATE TABLE name
	
	
Prints the CREATE TABLE statement that creates the named table. 

.. code-block:: mysql


    mysql> SHOW CREATE TABLE idx\G
    *************************** 1. row ***************************
           Table: idx
    Create Table: CREATE TABLE idx (
    f text indexed stored
    ) charset_table='non_cjk,cjk' morphology='icu_chinese'
    1 row in set (0.00 sec)



