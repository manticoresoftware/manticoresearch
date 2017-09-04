``wordbreaker`` command reference
---------------------------------

``wordbreaker`` is one of the helper tools within the Manticore package. It
is used to split compound words, as usual in URLs, into its component
words. For example, this tool can split “lordoftherings” into its four
component words, or “http://manofsteel.warnerbros.com” into “man of
steel warner bros”. This helps searching, without requiring prefixes or
infixes: searching for “sphinx” wouldn't match “sphinxsearch” but if you
break the compound word and index the separate components, you'll get a
match without the costs of prefix and infix larger index files.

Examples of its usage are:

.. code-block:: bash


    echo manofsteel | bin/wordbreaker -dict dict.txt split
	man of steel

The input stream will be separated in words using the ``-dict``
dictionary file. In no dictionary specified, wordbreaker looks in the working folder for a wordbreaker-dict.txt file. (The dictionary should match the language of the
compound word.) The ``split`` command breaks words from the standard
input, and outputs the result in the standard output. There are also
``test`` and ``bench`` commands that let you test the splitting quality
and benchmark the splitting functionality.

Wordbreaker Wordbreaker needs a dictionary to recognize individual
substrings within a string. To differentiate between different guesses,
it uses the relative frequency of each word in the dictionary: higher
frequency means higher split probability. You can generate such a file
using the ``indexer`` tool, as in

.. code-block:: bash


    indexer --buildstops dict.txt 100000 --buildfreqs myindex -c /path/to/sphinx.conf

which will write the 100,000 most frequent words, along with their
counts, from myindex into dict.txt. The output file is a text file, so
you can edit it by hand, if need be, to add or remove words.

