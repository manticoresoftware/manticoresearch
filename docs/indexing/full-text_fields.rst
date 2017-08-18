.. _full-text_fields:

Full-text fields
=======================

Full-text fields (or just *fields* for brevity) are the textual document
contents that get indexed by Manticore, and can be (quickly) searched for
keywords.

Fields are named, and you can limit your searches to a single field (eg.
search through “title” only) or a subset of fields (eg. to “title” and
“abstract” only). Manticore index format generally supports up to 256
fields.

Note that the original contents of the fields are **not** stored in
the Manticore index. The text that you send to Manticore gets processed, and a
full-text index (a special data structure that enables quick searches
for a keyword) gets built from that text. But the original text contents
are then simply discarded. Manticore assumes that you store those contents
elsewhere anyway.

Moreover, it is impossible to *fully* reconstruct the original text,
because the specific whitespace, capitalization, punctuation, etc will
all be lost during indexing. It is theoretically possible to partially
reconstruct a given document from the Manticore full-text index, but that
would be a slow process (especially if the :ref:`CRC
dictionary <dict>` is used, which
does not even store the original keywords and works with their hashes
instead).
