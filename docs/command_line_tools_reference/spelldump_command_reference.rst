``spelldump`` command reference
-------------------------------

``spelldump`` is one of the helper tools within the Manticore package.

It is used to extract the contents of a dictionary file that uses
``ispell`` or ``MySpell`` format, which can help build word lists for
*wordforms* - all of the possible forms are pre-built for you.

Its general usage is:

.. code-block:: bash


    spelldump [options] <dictionary> <affix> [result] [locale-name]

The two main parameters are the dictionary's main file and its affix
file; usually these are named as ``[language-prefix].dict`` and
``[language-prefix].aff`` and will be available with most common Linux
distributions, as well as various places online.

``[result]`` specifies where the dictionary data should be output to,
and ``[locale-name]`` additionally specifies the locale details you wish
to use.

There is an additional option, ``-c [file]``, which specifies a file for
case conversion details.

Examples of its usage are:

.. code-block:: bash


    spelldump en.dict en.aff
    spelldump ru.dict ru.aff ru.txt ru_RU.CP1251
    spelldump ru.dict ru.aff ru.txt .1251

The results file will contain a list of all the words in the dictionary
in alphabetical order, output in the format of a wordforms file, which
you can use to customize for your specific circumstances. An example of
the result file:

.. code-block:: bash


    zone > zone
    zoned > zoned
    zoning > zoning

