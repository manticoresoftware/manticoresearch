.. _flush_ramchunk_syntax:

FLUSH RAMCHUNK syntax
---------------------

.. code-block:: mysql


    FLUSH RAMCHUNK rtindex

FLUSH RAMCHUNK forcibly creates a new disk chunk in a RT index.

Normally, RT index would flush and convert the contents of the RAM chunk
into a new disk chunk automatically, once the RAM chunk reaches the
maximum allowed
:ref:`rt_mem_limit <rt_mem_limit>` size.
However, for debugging and testing it might be useful to forcibly create
a new disk chunk, and FLUSH RAMCHUNK statement does exactly that.

Note that using FLUSH RAMCHUNK increases RT index fragmentation. Most
likely, you want to use FLUSH RTINDEX instead. We suggest that you
abstain from using just this statement unless you're absolutely sure
what you're doing. As the right way is to issue FLUSH RAMCHUNK with
following :ref:`OPTIMIZE <optimize_index_syntax>` command. Such combo
allows to keep RT index fragmentation on minimum.

.. code-block:: mysql


    mysql> FLUSH RAMCHUNK rt;
    Query OK, 0 rows affected (0.05 sec)

