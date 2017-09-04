.. _collations:

Collations
----------

Collations essentially affect the string attribute comparisons. They
specify both the character set encoding and the strategy that Manticore
uses to compare strings when doing ORDER BY or GROUP BY with a string
attribute involved.

String attributes are stored as is when indexing, and no character set
or language information is attached to them. That's okay as long as
Manticore only needs to store and return the strings to the calling
application verbatim. But when you ask Manticore to sort by a string value,
that request immediately becomes quite ambiguous.

First, single-byte (ASCII, or ISO-8859-1, or Windows-1251) strings need
to be processed differently that the UTF-8 ones that may encode every
character with a variable number of bytes. So we need to know what is
the character set type to interpret the raw bytes as meaningful
characters properly.

Second, we additionally need to know the language-specific string
sorting rules. For instance, when sorting according to US rules in
en_US locale, the accented character ‘ï’ (small letter i with
diaeresis) should be placed somewhere after ‘z’. However, when sorting
with French rules and fr_FR locale in mind, it should be placed between
‘i’ and ‘j’. And some other set of rules might choose to ignore accents
at all, allowing ‘ï’ and ‘i’ to be mixed arbitrarily.

Third, but not least, we might need case-sensitive sorting in some
scenarios and case-insensitive sorting in some others.

Collations combine all of the above: the character set, the language
rules, and the case sensitivity. Manticore currently provides the following
four collations.

1. ``libc_ci``

2. ``libc_cs``

3. ``utf8_general_ci``

4. ``binary``

The first two collations rely on several standard C library (libc) calls
and can thus support any locale that is installed on your system. They
provide case-insensitive (_ci) and case-sensitive (_cs) comparisons
respectively. By default they will use C locale, effectively resorting
to bytewise comparisons. To change that, you need to specify a different
available locale using
:ref:`collation_libc_locale <collation_libc_locale>`
directive. The list of locales available on your system can usually be
obtained with the ``locale`` command:

.. code-block:: mysql


    $ locale -a
    C
    en_AG
    en_AU.utf8
    en_BW.utf8
    en_CA.utf8
    en_DK.utf8
    en_GB.utf8
    en_HK.utf8
    en_IE.utf8
    en_IN
    en_NG
    en_NZ.utf8
    en_PH.utf8
    en_SG.utf8
    en_US.utf8
    en_ZA.utf8
    en_ZW.utf8
    es_ES
    fr_FR
    POSIX
    ru_RU.utf8
    ru_UA.utf8

The specific list of the system locales may vary. Consult your OS
documentation to install additional needed locales.

``utf8_general_ci`` and ``binary`` locales are built-in into Manticore. The
first one is a generic collation for UTF-8 data (without any so-called
language tailoring); it should behave similar to ``utf8_general_ci``
collation in MySQL. The second one is a simple bytewise comparison.

Collation can be overridden via SphinxQL on a per-session basis using
``SET collation_connection`` statement. All subsequent SphinxQL queries
will use this collation. SphinxAPI and SphinxSE queries will use the
server default collation, as specified in
:ref:`collation_server <collation_server>`
configuration directive. Manticore currently defaults to ``libc_ci``
collation.

Collations should affect all string attribute comparisons, including
those within ORDER BY and GROUP BY, so differently ordered or grouped
results can be returned depending on the collation chosen. Note that
collations don't affect full-text searching, for that use
:ref:`charset_table <charset_table>`.
