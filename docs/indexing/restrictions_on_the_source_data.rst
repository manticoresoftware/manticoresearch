.. _restrictions_on_the_source_data:

Restrictions on the source data
-------------------------------

There are a few different restrictions imposed on the source data which
is going to be indexed by Manticore, of which the single most important one
is:

**ALL DOCUMENT IDS MUST BE UNIQUE SIGNED POSITIVE NON-ZERO 64-BIT INTEGER NUMBERS**

Indexing documents with same ID will result in strange search results, as matching processes will return the
document id, however it's possible that selection to pick another document with same ID, leading to incorrect search results.
It is also possible that duplicate IDs to trigger crashes at indexing due to internal assertion checks.
In case of distributed indexes, having documents with same ID in different nodes can also lead to incorrect results.
In case of delta indexes providing updates for main indexes, the outdated documents with same ID as newer ones can be suppressed 
by :ref:`kill-lists <killlist_target>`.