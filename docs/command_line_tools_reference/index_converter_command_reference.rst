.. _index_converter_command_reference:

``index_converter`` command reference
-------------------------------------

``index_converter`` is a tool for converting indexes created with Sphinx/Manticore Search 2.x to Manticore Search 3.x index format.

The tool can be used in several different ways:

Convert one index at a time:

.. code-block:: bash

    $ index_converter --config /home/myuser/sphinx.conf --index indexname

Convert all indexes:

.. code-block:: bash

    $ index_converter --config /home/myuser/sphinx.conf --all

Convert indexes found in a folder:

.. code-block:: bash

    $ index_converter  --path /var/lib/manticoresearch/data --all

New version of the index is written by default in the same folder. Previous version files are saved with `.old` extension in their name.
An exception is `.spp` (hitlists) file which is the only index component that didn't have any change in the new format.

You can save the new index version to a different folder using `--output-dir` option

.. code-block:: bash

    $ index_converter --config /home/myuser/sphinx.conf --all --output-dir /new/path

A special case is for indexes containing kill-lists. As the behaviour of how kill-lists works has changed (see :ref:`killlist_target`), the delta index should know which are the target indexes for applying the kill-lists
There are 3 ways to have a converted index ready for setting targeted indexes for applying kill-lists:

- Use `--killlist-target` when converting an index

.. code-block:: bash

    $ index_converter --config /home/myuser/sphinx.conf --index deltaindex --killlist-target mainindex:kl

- Add killlist_target in the configuration before doing the conversion

- use :ref:`ALTER ... KILLLIST_TARGET<alter_syntax>` command after conversion

A complete list of ``index_converter`` options:

-  ``--config <file>`` (``-c <file>`` for short) tells
   ``index_converter`` to use the given file as its configuration. Normally, it
   will look for ``sphinx.conf`` in the installation directory (e.g.
   ``/usr/local/sphinx/etc/sphinx.conf`` if installed into
   ``/usr/local/sphinx``), followed by the current directory you are in
   when calling ``index_converter`` from the shell.

- ``--index`` specifies which index should be converted

- ``--path`` - instead of using a config file, a path containing index(es) can be used

- ``--strip-path`` - strips path from filenames referenced by index: stopwords, exceptions and wordforms

- ``--large-docid`` - allows to convert documents with ids larger than 2^63 and display a warning, otherwise it will just exit on the large id with an error. This option was added as in Manticore 3.x doc ids are signed bigint, while previously they were unsigned

- ``--output-dir <dir>`` - writes the new files in a chosen folder rather than the same location as with the existing index files. When this option set, existing index files will remain untouched at their location.

- ``--all``  - converts all indexes from the config

- ``--killlist-target <targets>`` -  sets the target indexes for which kill-lists will be applied. This option should be used only in conjunction with ``--index`` option
