.. _indexer_command_reference:

``indexer`` command reference
-----------------------------

``indexer`` is the first of the two principal tools as part of Manticore.
Invoked from either the command line directly, or as part of a larger
script, ``indexer`` is solely responsible for gathering the data that
will be searchable.

The calling syntax for ``indexer`` is as follows:

.. code-block:: bash


    indexer [OPTIONS] [indexname1 [indexname2 [...]]]

Essentially you would list the different possible indexes (that you
would later make available to search) in ``sphinx.conf``, so when
calling ``indexer``, as a minimum you need to be telling it what index
(or indexes) you want to index.

If ``sphinx.conf`` contained details on 2 indexes, ``mybigindex`` and
``mysmallindex``, you could do the following:

.. code-block:: bash


    $ indexer mybigindex
    $ indexer mysmallindex mybigindex

As part of the configuration file, ``sphinx.conf``, you specify one or
more indexes for your data. You might call ``indexer`` to reindex one of
them, ad-hoc, or you can tell it to process all indexes - you are not
limited to calling just one, or all at once, you can always pick some
combination of the available indexes.

The exit codes are as follows:

-  0, everything went ok
-  1, there was a problem while indexing (and if –rotate was specified,
   it was skipped)
-  2, indexing went ok, but –rotate attempt failed

The majority of the options for ``indexer`` are given in the
configuration file, however there are some options you might need to
specify on the command line as well, as they can affect how the indexing
operation is performed. These options are:

-  ``--config <file>`` (``-c <file>`` for short) tells
   ``indexer`` to use the given file as its configuration. Normally, it
   will look for ``sphinx.conf`` in the installation directory (e.g.
   ``/usr/local/sphinx/etc/sphinx.conf`` if installed into
   ``/usr/local/sphinx``), followed by the current directory you are in
   when calling ``indexer`` from the shell. This is most of use in
   shared environments where the binary files are installed somewhere
   like ``/usr/local/sphinx/`` but you want to provide users with the
   ability to make their own custom Manticore set-ups, or if you want to
   run multiple instances on a single server. In cases like those you
   could allow them to create their own ``sphinx.conf`` files and pass
   them to ``indexer`` with this option. For example:

   .. code-block:: bash


       $ indexer --config /home/myuser/sphinx.conf myindex

-  ``--all`` tells ``indexer`` to update every index listed in
   ``sphinx.conf``, instead of listing individual indexes. This would be
   useful in small configurations, or ``cron``-type or maintenance jobs
   where the entire index set will get rebuilt each day, or week, or
   whatever period is best. Example usage:

   .. code-block:: bash


       $ indexer --config /home/myuser/sphinx.conf --all

-  ``--rotate`` is used for rotating indexes. Unless you have the
   situation where you can take the search function offline without
   troubling users, you will almost certainly need to keep search
   running whilst indexing new documents. ``--rotate`` creates a second
   index, parallel to the first (in the same place, simply including
   ``.new`` in the filenames). Once complete, ``indexer`` notifies
   ``searchd`` via sending the ``SIGHUP`` signal, and ``searchd`` will
   attempt to rename the indexes (renaming the existing ones to include
   ``.old`` and renaming the ``.new`` to replace them), and then start
   serving from the newer files. Depending on the setting of
   :ref:`seamless_rotate <seamless_rotate>`,
   there may be a slight delay in being able to search the newer
   indexes. Example usage:

   .. code-block:: bash


       $ indexer --rotate --all

-  ``--quiet`` tells ``indexer`` not to output anything, unless there is
   an error. Again, most used for ``cron``-type, or other script jobs
   where the output is irrelevant or unnecessary, except in the event of
   some kind of error. Example usage:

   .. code-block:: bash


       $ indexer --rotate --all --quiet

-  ``--noprogress`` does not display progress details as they occur;
   instead, the final status details (such as documents indexed, speed
   of indexing and so on are only reported at completion of indexing. In
   instances where the script is not being run on a console (or ‘tty’),
   this will be on by default. Example usage:

   .. code-block:: bash


       $ indexer --rotate --all --noprogress

-  ``--buildstops <outputfile.text> <N>`` reviews the index
   source, as if it were indexing the data, and produces a list of the
   terms that are being indexed. In other words, it produces a list of
   all the searchable terms that are becoming part of the index. Note;
   it does not update the index in question, it simply processes the
   data ‘as if’ it were indexing, including running queries defined with
   ``sql_query_pre`` or ``sql_query_post``. ``outputfile.txt`` will
   contain the list of words, one per line, sorted by frequency with
   most frequent first, and ``N`` specifies the maximum number of words
   that will be listed; if sufficiently large to encompass every word in
   the index, only that many words will be returned. Such a dictionary
   list could be used for client application features around “Did you
   mean…” functionality, usually in conjunction with ``--buildfreqs``,
   below. Example:

   .. code-block:: bash


       $ indexer myindex --buildstops word_freq.txt 1000

   This would produce a document in the current directory,
   ``word_freq.txt`` with the 1,000 most common words in ‘myindex’,
   ordered by most common first. Note that the file will pertain to the
   last index indexed when specified with multiple indexes or ``--all``
   (i.e. the last one listed in the configuration file)

-  ``--buildfreqs`` works with ``--buildstops`` (and is ignored if
   ``--buildstops`` is not specified). As ``--buildstops`` provides the
   list of words used within the index, ``--buildfreqs`` adds the
   quantity present in the index, which would be useful in establishing
   whether certain words should be considered stopwords if they are too
   prevalent. It will also help with developing “Did you mean…” features
   where you can how much more common a given word compared to another,
   similar one. Example:

   .. code-block:: bash


       $ indexer myindex --buildstops word_freq.txt 1000 --buildfreqs

   This would produce the ``word_freq.txt`` as above, however after each
   word would be the number of times it occurred in the index in
   question.

-  ``--merge <dst-index> <src-index>`` is used for
   physically merging indexes together, for example if you have a
   main+delta scheme, where the main index rarely changes, but the delta
   index is rebuilt frequently, and ``--merge`` would be used to combine
   the two. The operation moves from right to left - the contents of
   ``src-index`` get examined and physically combined with the contents
   of ``dst-index`` and the result is left in ``dst-index``. In
   pseudo-code, it might be expressed as: ``dst-index += src-index`` An
   example:

   .. code-block:: bash


       $ indexer --merge main delta --rotate

   In the above example, where the main is the master, rarely modified
   index, and delta is the less frequently modified one, you might use
   the above to call ``indexer`` to combine the contents of the delta
   into the main index and rotate the indexes.

-  ``--merge-dst-range <attr> <min> <max>`` runs the
   filter range given upon merging. Specifically, as the merge is
   applied to the destination index (as part of ``--merge``, and is
   ignored if ``--merge`` is not specified), ``indexer`` will also
   filter the documents ending up in the destination index, and only
   documents will pass through the filter given will end up in the final
   index. This could be used for example, in an index where there is a
   ‘deleted’ attribute, where 0 means ‘not deleted’. Such an index could
   be merged with:

   .. code-block:: bash


       $ indexer --merge main delta --merge-dst-range deleted 0 0

   Any documents marked as deleted (value 1) would be removed from the
   newly-merged destination index. It can be added several times to the
   command line, to add successive filters to the merge, all of which
   must be met in order for a document to become part of the final
   index.

-  ``--merge-killlists`` (and its shorter alias ``--merge-klists``)
   changes the way kill lists are processed when merging indexes. By
   default, both kill lists get discarded after a merge. That supports
   the most typical main+delta merge scenario. With this option enabled,
   however, kill lists from both indexes get concatenated and stored
   into the destination index. Note that a source (delta) index kill
   list will be used to suppress rows from a destination (main) index at
   all times.

-  ``--keep-attrs`` allows to reuse existing attributes on reindexing.
   Whenever the index is rebuilt, each new document id is checked for
   presence in the “old” index, and if it already exists, its attributes
   are transferred to the “new” index; if not found, attributes from the
   new index are used. If the user has updated attributes in the index,
   but not in the actual source used for the index, all updates will be
   lost when reindexing; using –keep-attrs enables saving the updated
   attribute values from the previous index. It is possible to specify a
   path for index files to used instead of reference path from config:


   .. code-block:: bash


       indexer myindex --keep-attrs=/path/to/index/files
	   
-  ``--keep-attrs-names=<attributes list>`` allows to specify attributes
   to reuse from existing index on reindexing. By default all attributes
   from existed index reused at new “index”

   .. code-block:: bash


       indexer myindex --keep-attrs=/path/to/index/files --keep-attrs-names=update,state
   
-  ``--dump-rows <FILE>`` dumps rows fetched by SQL source(s) into
   the specified file, in a MySQL compatible syntax. Resulting dumps are
   the exact representation of data as received by ``indexer`` and help
   to repeat indexing-time issues.

-  ``--verbose`` guarantees that every row that caused problems indexing
   (duplicate, zero, or missing document ID; or file field IO issues;
   etc) will be reported. By default, this option is off, and problem
   summaries may be reported instead.

-  ``--sighup-each`` is useful when you are rebuilding many big indexes,
   and want each one rotated into ``searchd`` as soon as possible. With
   ``--sighup-each``, ``indexer`` will send a SIGHUP signal to searchd
   after successfully completing the work on each index. (The default
   behavior is to send a single SIGHUP after all the indexes were
   built.)

-  ``--nohup`` is useful when you want to check your index with
   indextool before actually rotating it. indexer won't send SIGHUP if
   this option is on.

-  ``--print-queries`` prints out SQL queries that ``indexer`` sends to
   the database, along with SQL connection and disconnection events.
   That is useful to diagnose and fix problems with SQL sources.
