.. _indextool_command_reference:

``indextool`` command reference
-------------------------------

``indextool`` is one of the helper tools within the Manticore package. It
is used to dump miscellaneous debug information about the physical
index. (Additional functionality such as index verification is planned
in the future, hence the indextool name rather than just indexdump.) Its
general usage is:

.. code-block:: mysql


    indextool <command> [options]

Options apply to all commands:

-  ``--config <file>`` (``-c <file>`` for short) overrides
   the built-in config file names.

-  ``--quiet`` (``-q`` for short) keep indextool quiet - it will not
   output banner, etc.

-  ``--help`` (``-h`` for short) lists all of the parameters that can be
   called in your particular build of ``indextool``.

-  ``-v`` show version information of your particular build of ``indextool``.

The commands are as follows:

-  ``--checkconfig`` just loads and verifies the config file to check if
   it's valid, without syntax errors.

-  ``--buildidf DICTFILE1 [DICTFILE2 ...] --out IDFILE`` build IDF file from one or several dictionary dumps.
   Additional parameter ``-skip-uniq`` will skip unique (df=1) words.

-  ``--build-infixes INDEXNAME`` build infixes for an existing
   dict=keywords index (upgrades .sph, .spi in place). You can use this
   option for legacy index files that already use dict=keywords, but now
   need to support infix searching too; updating the index files with
   indextool may prove easier or faster than regenerating them from
   scratch with indexer.

-  ``--dumpheader FILENAME.sph`` quickly dumps the provided index header
   file without touching any other index files or even the configuration
   file. The report provides a breakdown of all the index settings, in
   particular the entire attribute and field list.

-  ``--dumpconfig FILENAME.sph`` dumps the index definition from the
   given index header file in (almost) compliant ``manticore.conf`` file
   format.

-  ``--dumpheader INDEXNAME`` dumps index header by index name with
   looking up the header path in the configuration file.

-  ``--dumpdict INDEXNAME`` dumps dictionary. 
   Additional ``-stats`` switch will dump to dictionary the total number of documents. It is required for dictionary files that are used  for creation of IDF files.

-  ``--dumpdocids INDEXNAME`` dumps document IDs by index name.

-  ``--dumphitlist INDEXNAME KEYWORD`` dumps all the hits (occurrences)
   of a given keyword in a given index, with keyword specified as text.

-  ``--dumphitlist INDEXNAME --wordid ID`` dumps all the hits
   (occurrences) of a given keyword in a given index, with keyword
   specified as internal numeric ID.

-  ``--fold INDEXNAME OPTFILE`` This options is useful too see how
   actually tokenizer proceeds input. You can feed indextool with text
   from file if specified or from stdin otherwise. The output will
   contain spaces instead of separators (accordingly to your
   charset_table settings) and lowercased letters in words.

-  ``--htmlstrip INDEXNAME`` filters stdin using HTML stripper settings
   for a given index, and prints the filtering results to stdout. Note
   that the settings will be taken from manticore.conf, and not the index
   header.

-  ``--mergeidf NODE1.idf [NODE2.idf ...] --out GLOBAL.idf`` merge several .idf files into a single one.
   Additional parameter ``-skip-uniq`` will skip unique (df=1) words.
   

-  ``--morph INDEXNAME`` applies morphology to the given stdin and
   prints the result to stdout.

-  ``--check INDEXNAME`` checks the index data files for consistency
   errors that might be introduced either by bugs in ``indexer`` and/or
   hardware faults. ``--check`` also works on RT indexes, RAM and disk
   chunks.

-  ``--strip-path`` strips the path names from all the file names
   referenced from the index (stopwords, wordforms, exceptions, etc).
   This is useful for checking indexes built on another machine with
   possibly different path layouts.

-  ``--rotate`` works only with ``--check`` and defines whether to check
   index waiting for rotation, i.e. with .new extension. This is useful
   when you want to check your index before actually using it.

-  ``--apply-killlists`` loads and applies kill-lists for all indexes listed
   in the config file. Changes are saved in .SPM files. Kill-list files (.SPK)
   are deleted. This can be useful if you want to move applying indexes from
   daemon startup to indexing stage.
