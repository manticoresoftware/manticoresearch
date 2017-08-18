.. _common program configuration options:

Common section configuration options
------------------------------------

.. _lemmatizer_base:

lemmatizer_base
~~~~~~~~~~~~~~~~

Lemmatizer dictionaries base path. Optional, default is /usr/local/share
(as in –datadir switch to ./configure script).

Our lemmatizer implementation (see :ref:`morphology` for a
discussion of what lemmatizers are) is dictionary driven.
lemmatizer_base directive configures the base dictionary path. File
names are hardcoded and specific to a given lemmatizer; the Russian
lemmatizer uses ru.pak dictionary file. The dictionaries can be obtained
from the Manticore website.

Example:

::


    lemmatizer_base = /usr/local/share/sphinx/dicts/
	

.. _json_autoconv_keynames:

json_autoconv_keynames
~~~~~~~~~~~~~~~~~~~~~~~~

Whether and how to auto-convert key names within JSON attributes. Known
value is ‘lowercase’. Optional, default value is unspecified (do not
convert anything).

When this directive is set to ‘lowercase’, key names within JSON
attributes will be automatically brought to lower case when indexing.
This conversion applies to any data source, that is, JSON attributes
originating from either SQL or XMLpipe2 sources will all be affected.

Example:

::


    json_autoconv_keynames = lowercase

	
.. _json_autoconv_numbers:

json_autoconv_numbers
~~~~~~~~~~~~~~~~~~~~~~~

Automatically detect and convert possible JSON strings that represent
numbers, into numeric attributes. Optional, default value is 0 (do not
convert strings into numbers).

When this option is 1, values such as “1234” will be indexed as numbers
instead of strings; if the option is 0, such values will be indexed as
strings. This conversion applies to any data source, that is, JSON
attributes originating from either SQL or XMLpipe2 sources will all be
affected.

Example:

::


    json_autoconv_numbers = 1


.. _on_json_attr_error:

on_json_attr_error
~~~~~~~~~~~~~~~~~~~~~

What to do if JSON format errors are found. Optional, default value is
``ignore_attr`` (ignore errors). Applies only to ``sql_attr_json``
attributes.

By default, JSON format errors are ignored (``ignore_attr``) and the
indexer tool will just show a warning. Setting this option to
``fail_index`` will rather make indexing fail at the first JSON format
error.

Example:


::


    on_json_attr_error = ignore_attr


.. _plugin_dir:

plugin_dir
~~~~~~~~~~~

Trusted location for the dynamic libraries (UDFs). Optional, default is
empty (no location).

Specifies the trusted directory from which the :ref:`UDF
libraries <udfs_user_defined_functions>` can be
loaded. Requires `workers =
thread <workers>` to
take effect.

Example:


::


    plugin_dir = /usr/local/sphinx/lib


.. _rlp_environment:

rlp_environment
~~~~~~~~~~~~~~~~

RLP environment configuration file. Mandatory if RLP is used.

Example:


::


    rlp_environment = /home/myuser/RLP/rlp-environment.xml



.. _rlp_max_batch_docs:

rlp_max_batch_docs
~~~~~~~~~~~~~~~~~~~~~

Maximum number of documents batched before processing them by the RLP.
Optional, default is 50. This option has effect only if
``morphology = rlp_chinese_batched`` is specified.

Example:

::


    rlp_max_batch_docs = 100



.. _rlp_max_batch_size:

rlp_max_batch_size
~~~~~~~~~~~~~~~~~~~~~

Maximum total size of documents batched before processing them by the
RLP. Optional, default is 51200. Do not set this value to more than 10Mb
because sphinx splits large documents to 10Mb chunks before processing
them by the RLP. This option has effect only if
``morphology = rlp_chinese_batched`` is specified.

Example:


::


    rlp_max_batch_size = 100k

	
.. _rlp_root:

rlp_root
~~~~~~~~~

Path to the RLP root folder. Mandatory if RLP is used.

Example:

::


    rlp_root = /home/myuser/RLP

	
