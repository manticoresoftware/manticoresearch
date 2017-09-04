.. _reload_index_syntax:

RELOAD INDEX syntax
-------------------

.. code-block:: mysql


    RELOAD INDEX idx [ FROM '/path/to/index_files' ]

RELOAD INDEX allows you to rotate indexes using SphinxQL.

It has two modes of operation. First one (without specifying a path)
makes Manticore daemon check for new index files in directory specified in
:ref:`path`.
New index files must have a idx.new.sp? names.

And if you additionally specify a path, daemon will look for index files
in specified directory, move them to index :ref:`path`, rename from
index_files.sp? to idx.new.sp? and rotate them.

.. code-block:: mysql


    mysql> RELOAD INDEX plain_index;
    mysql> RELOAD INDEX plain_index FROM '/home/mighty/new_index_files';

