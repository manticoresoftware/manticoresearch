.. _charsets,_case_folding,_translation_tables,_and_re:

Charsets, case folding, translation tables, and replacement rules
=================================================================

When indexing some index, Manticore fetches documents from the specified
sources, splits the text into words, and does case folding so that
“Abc”, “ABC” and “abc” would be treated as the same word (or, to be
pedantic, *term*).

To do that properly, Manticore needs to know

-  what encoding is the source text in (and this encoding should always
   be UTF-8);

-  what characters are letters and what are not;

-  what letters should be folded to what letters.

This should be configured on a per-index basis using
:ref:`charset_table`.
option.
:ref:`charset_table`
specifies the table that maps letter characters to their case folded
versions. The characters that are not in the table are considered to be
non-letters and will be treated as word separators when indexing or
searching through this index.

Default tables currently include English and Russian characters. Please
do submit your tables for other languages!

You can also specify text pattern replacement rules. For example, given
the rules

.. code-block:: mysql


    regexp_filter = \**(\d+)\" => \1 inch
    regexp_filter = (BLUE|RED) => COLOR

the text ‘RED TUBE 5" LONG’ would be indexed as ‘COLOR TUBE 5 INCH
LONG’, and 'PLANK 2" x 4“‘as 'PLANK 2 INCH x 4 INCH’. Rules are applied
in the given order. Text in queries is also replaced; a search for”BLUE
TUBE" would actually become a search for “COLOR TUBE”. Note that Manticore
must be built with the –with-re2 option to use this feature.
