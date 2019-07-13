.. _common program configuration options:

Common section configuration options
------------------------------------

.. _lemmatizer_base:

lemmatizer_base
~~~~~~~~~~~~~~~

Lemmatizer dictionaries base path. Optional, default is /usr/local/share
(as in –datadir switch to ./configure script).

Our lemmatizer implementation (see :ref:`morphology` for a
discussion of what lemmatizers are) is dictionary driven.
lemmatizer_base directive configures the base dictionary path. File
names are hardcoded and specific to a given lemmatizer; the Russian
lemmatizer uses ru.pak dictionary file. The dictionaries can be obtained
from the Manticore website (https://manticoresearch.com/downloads/).

Example:

.. code-block:: ini


    lemmatizer_base = /usr/local/share/sphinx/dicts/

.. _progressive_merge:

progressive_merge
~~~~~~~~~~~~~~~~~

Merge Real-Time index chunks during OPTIMIZE operation from smaller to bigger. Progressive merge merger faster and reads/write less data.
Enabled by default. If disabled, chunks are merged from first to last created.

.. _json_autoconv_keynames:

json_autoconv_keynames
~~~~~~~~~~~~~~~~~~~~~~

Whether and how to auto-convert key names within JSON attributes. Known
value is ‘lowercase’. Optional, default value is unspecified (do not
convert anything).

When this directive is set to ‘lowercase’, key names within JSON
attributes will be automatically brought to lower case when indexing.
This conversion applies to any data source, that is, JSON attributes
originating from either SQL or XMLpipe2 sources will all be affected.

Example:

.. code-block:: ini


    json_autoconv_keynames = lowercase


.. _json_autoconv_numbers:

json_autoconv_numbers
~~~~~~~~~~~~~~~~~~~~~

Automatically detect and convert possible JSON strings that represent
numbers, into numeric attributes. Optional, default value is 0 (do not
convert strings into numbers).

When this option is 1, values such as “1234” will be indexed as numbers
instead of strings; if the option is 0, such values will be indexed as
strings. This conversion applies to any data source, that is, JSON
attributes originating from either SQL or XMLpipe2 sources will all be
affected.

Example:

.. code-block:: ini


    json_autoconv_numbers = 1


.. _on_json_attr_error:

on_json_attr_error
~~~~~~~~~~~~~~~~~~

What to do if JSON format errors are found. Optional, default value is
``ignore_attr`` (ignore errors). Applies only to ``sql_attr_json``
attributes.

By default, JSON format errors are ignored (``ignore_attr``) and the
indexer tool will just show a warning. Setting this option to
``fail_index`` will rather make indexing fail at the first JSON format
error.

Example:


.. code-block:: ini


    on_json_attr_error = ignore_attr


.. _plugin_dir:

plugin_dir
~~~~~~~~~~

Trusted location for the dynamic libraries (UDFs). Optional, default is
empty (no location).

Specifies the trusted directory from which the :ref:`UDF
libraries <udfs_user_defined_functions>` can be
loaded.

Example:


.. code-block:: ini


    plugin_dir = /usr/local/sphinx/lib


.. _icu_data_dir:

icu_data_dir
~~~~~~~~~~~~~~~

A folder that contains data used by ICU to segment Chinese text. Should only be specified if you've built
ICU from sources. If ICU is loaded as a dynamic library (supplied in a package provided by us,
e.g. ``libicu_dev``), it doesn't require any external data. This folder must contain a .dat file (e.g. ``icudt64l.dat``).

Example:


.. code-block:: ini


    icu_data_dir = /home/myuser/icu_data
