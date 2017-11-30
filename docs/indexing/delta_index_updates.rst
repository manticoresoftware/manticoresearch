.. _delta_index_updates:

Delta index updates
===================

There's a frequent situation when the total dataset is too big to be
reindexed from scratch often, but the amount of new records is rather
small. Example: a forum with a 1,000,000 archived posts, but only 1,000
new posts per day.

In this case, “live” (almost real time) index updates could be
implemented using so called “main+delta” scheme.

The idea is to set up two sources and two indexes, with one “main” index
for the data which only changes rarely (if ever), and one “delta” for
the new documents. In the example above, 1,000,000 archived posts would
go to the main index, and newly inserted 1,000 posts/day would go to the
delta index. Delta index could then be reindexed very frequently, and
the documents can be made available to search in a matter of minutes.

Specifying which documents should go to what index and reindexing main
index could also be made fully automatic. One option would be to make a
counter table which would track the ID which would split the documents,
and update it whenever the main index is reindexed.

Example 3.3. Fully automated live updates
                                         

.. code-block:: ini


    # in MySQL
    CREATE TABLE sph_counter
    (
        counter_id INTEGER PRIMARY KEY NOT NULL,
        max_doc_id INTEGER NOT NULL
    );

    # in sphinx.conf
    source main
    {
        # ...
        sql_query_pre = SET NAMES utf8
        sql_query_pre = REPLACE INTO sph_counter SELECT 1, MAX(id) FROM documents
        sql_query = SELECT id, title, body FROM documents \
            WHERE id<=( SELECT max_doc_id FROM sph_counter WHERE counter_id=1 )
    }

    source delta : main
    {
        sql_query_pre = SET NAMES utf8
        sql_query = SELECT id, title, body FROM documents \
            WHERE id>( SELECT max_doc_id FROM sph_counter WHERE counter_id=1 )
    }

    index main
    {
        source = main
        path = /path/to/main
        # ... all the other settings
    }

    # note how all other settings are copied from main,
    # but source and path are overridden (they MUST be)
    index delta : main
    {
        source = delta
        path = /path/to/delta
    }

Note how we're overriding ``sql_query_pre`` in the delta source. We need
to explicitly have that override. Otherwise ``REPLACE`` query would be
run when indexing delta source too, effectively nullifying it. However,
when we issue the directive in the inherited source for the first time,
it removes *all* inherited values, so the encoding setup is also lost.
So ``sql_query_pre`` in the delta can not just be empty; and we need to
issue the encoding setup query explicitly once again.
