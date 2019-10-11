.. _full-text_fields:

Full-text fields
================

Full-text fields (or just *fields* for brevity) are the textual document
contents that get indexed by Manticore, and can be (quickly) searched for
keywords.

Fields are named, and you can limit your searches to a single field (eg.
search through “title” only) or a subset of fields (eg. to “title” and
“abstract” only). Manticore index format generally supports up to 256
fields.


The text that you send to Manticore gets processed, and a
full-text index (a special data structure that enables quick searches
for a keyword) gets built from that text.
Prior Manticore Search 3.2 the original content of fields is discarded and it's not possible to
*fully* reconstruct it. In newer versions, original content can be optionally stored in index.