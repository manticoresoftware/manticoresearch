.. _indexer program configuration options:

``indexer`` program configuration options
-----------------------------------------

.. _lemmatizer_cache:

lemmatizer_cache
~~~~~~~~~~~~~~~~~

Lemmatizer cache size. Optional, default is 256K.

Our lemmatizer implementation (see :ref:`morphology` for a
discussion of what lemmatizers are) uses a compressed dictionary format
that enables a space/speed tradeoff. It can either perform lemmatization
off the compressed data, using more CPU but less RAM, or it can
decompress and precache the dictionary either partially or fully, thus
using less CPU but more RAM. And the lemmatizer_cache directive lets
you control how much RAM exactly can be spent for that uncompressed
dictionary cache.

Currently, the only available dictionaries are ru.pak, en.pak, and
de.pak. These are the russian, english and german dictionaries. The
compressed dictionary is approximately 2 to 10 MB in size. Note that the
dictionary stays in memory at all times, too. The default cache size is
256 KB. The accepted cache sizes are 0 to 2047 MB. It's safe to raise
the cache size too high; the lemmatizer will only use the needed memory.
For instance, the entire Russian dictionary decompresses to
approximately 110 MB; and thus setting lemmatizer_cache anywhere higher
than that will not affect the memory use: even when 1024 MB is allowed
for the cache, if only 110 MB is needed, it will only use those 110 MB.

On our benchmarks, the total indexing time with different cache sizes
was as follows:

-  9.07 sec, morphology = lemmatize_ru, lemmatizer_cache = 0
-  8.60 sec, morphology = lemmatize_ru, lemmatizer_cache = 256K
-  8.33 sec, morphology = lemmatize_ru, lemmatizer_cache = 8M
-  7.95 sec, morphology = lemmatize_ru, lemmatizer_cache = 128M
-  6.85 sec, morphology = stem_ru (baseline)

Your mileage may vary, but a simple rule of thumb would be to either go
with the small default 256 KB cache when pressed for memory, or spend
128 MB extra RAM and cache the entire dictionary for maximum indexing
performance.

Example:


.. code-block:: ini


    lemmatizer_cache = 256M # cache it all

.. _max_file_field_buffer:

max_file_field_buffer
~~~~~~~~~~~~~~~~~~~~~~~~

Maximum file field adaptive buffer size, bytes. Optional, default is 8
MB, minimum is 1 MB.

File field buffer is used to load files referred to from
:ref:`sql_file_field <sql_file_field>`
columns. This buffer is adaptive, starting at 1 MB at first allocation,
and growing in 2x steps until either file contents can be loaded, or
maximum buffer size, specified by ``max_file_field_buffer`` directive,
is reached.

Thus, if there are no file fields are specified, no buffer is allocated
at all. If all files loaded during indexing are under (for example) 2 MB
in size, but ``max_file_field_buffer`` value is 128 MB, peak buffer
usage would still be only 2 MB. However, files over 128 MB would be
entirely skipped.

Example:


.. code-block:: ini

    max_file_field_buffer = 128M

.. _max_iops:

max_iops
~~~~~~~~~

Maximum I/O operations per second, for I/O throttling. Optional, default
is 0 (unlimited).

I/O throttling related option. It limits maximum count of I/O operations
(reads or writes) per any given second. A value of 0 means that no limit
is imposed.

``indexer`` can cause bursts of intensive disk I/O during indexing, and
it might desired to limit its disk activity (and keep something for
other programs running on the same machine, such as ``searchd``). I/O
throttling helps to do that. It works by enforcing a minimum guaranteed
delay between subsequent disk I/O operations performed by ``indexer``.
Modern SATA HDDs are able to perform up to 70-100+ I/O operations per
second (that's mostly limited by disk heads seek time). Limiting
indexing I/O to a fraction of that can help reduce search performance
degradation caused by indexing.

Example:


.. code-block:: ini


    max_iops = 40

.. _max_iosize:

max_iosize
~~~~~~~~~~~

Maximum allowed I/O operation size, in bytes, for I/O throttling.
Optional, default is 0 (unlimited).

I/O throttling related option. It limits maximum file I/O operation
(read or write) size for all operations performed by ``indexer``. A
value of 0 means that no limit is imposed. Reads or writes that are
bigger than the limit will be split in several smaller operations, and
counted as several operation by
:ref:`max_iops <max_iops>`
setting. At the time of this writing, all I/O calls should be under 256
KB (default internal buffer size) anyway, so ``max_iosize`` values
higher than 256 KB must not affect anything.

Example:


.. code-block:: ini


    max_iosize = 1048576

.. _max_xmlpipe2_field:

max_xmlpipe2_field
~~~~~~~~~~~~~~~~~~~~

Maximum allowed field size for XMLpipe2 source type, bytes. Optional,
default is 2 MB.

Example:


.. code-block:: ini


    max_xmlpipe2_field = 8M

.. _mem_limit:

mem_limit
~~~~~~~~~~

Indexing RAM usage limit. Optional, default is 128M.

Enforced memory usage limit that the ``indexer`` will not go above. Can
be specified in bytes, or kilobytes (using K postfix), or megabytes
(using M postfix); see the example. This limit will be automatically
raised if set to extremely low value causing I/O buffers to be less than
8 KB; the exact lower bound for that depends on the indexed data size.
If the buffers are less than 256 KB, a warning will be produced.

Maximum possible limit is 2047M. Too low values can hurt indexing speed,
but 256M to 1024M should be enough for most if not all datasets. Setting
this value too high can cause SQL server timeouts. During the document
collection phase, there will be periods when the memory buffer is
partially sorted and no communication with the database is performed;
and the database server can timeout. You can resolve that either by
raising timeouts on SQL server side or by lowering ``mem_limit``.

Example:


.. code-block:: ini


    mem_limit = 256M
    # mem_limit = 262144K # same, but in KB
    # mem_limit = 268435456 # same, but in bytes

.. _on_file_field_error:

on_file_field_error
~~~~~~~~~~~~~~~~~~~~~~

How to handle IO errors in file fields. Optional, default is
``ignore_field``.

When there is a problem indexing a file referenced by a file field (:ref:`sql_file_field`),
``indexer`` can either index the document, assuming empty content in
this particular field, or skip the document, or fail indexing entirely.
``on_file_field_error`` directive controls that behavior. The values it
takes are:

-  ``ignore_field``, index the current document without field;

-  ``skip_document``, skip the current document but continue indexing;

-  ``fail_index``, fail indexing with an error message.

The problems that can arise are: open error, size error (file too big),
and data read error. Warning messages on any problem will be given at
all times, irregardless of the phase and the ``on_file_field_error``
setting.

Note that with ``on_file_field_error = skip_document`` documents will
only be ignored if problems are detected during an early check phase,
and **not** during the actual file parsing phase. ``indexer`` will
open every referenced file and check its size before doing any work, and
then open it again when doing actual parsing work. So in case a file
goes away between these two open attempts, the document will still be
indexed.

Example:


.. code-block:: ini


    on_file_field_error = skip_document

.. _write_buffer:

write_buffer
~~~~~~~~~~~~~

Write buffer size, bytes. Optional, default is 1 MB.

Write buffers are used to write both temporary and final index files
when indexing. Larger buffers reduce the number of required disk writes.
Memory for the buffers is allocated in addition to
:ref:`mem_limit <mem_limit>`.
Note that several (currently up to 4) buffers for different files will
be allocated, proportionally increasing the RAM usage.

Example:


.. code-block:: ini


    write_buffer = 4M

