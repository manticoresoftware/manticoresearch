# Main+delta schema

There's a frequent situation when the total dataset is too big to be rebuilt from scratch often, but the amount of new records is rather small. Example: a forum with a 1,000,000 archived posts, but only 1,000 new posts per day.

In this case, "live" (almost real time) table updates could be implemented using so called "main+delta" scheme.

The idea is to set up two sources and two tables, with one "main" table for the data which only changes rarely (if ever), and one "delta" for the new documents. In the example above, 1,000,000 archived posts would go to the main table, and newly inserted 1,000 posts/day would go to the delta table. Delta table could then be rebuilt very frequently, and the documents can be made available to search in a matter of minutes. Specifying which documents should go to what table and rebuilding the main table could also be made fully automatic. One option would be to make a counter table which would track the ID which would split the documents, and update it whenever the main table is rebuilt.

Example: Fully automated live updates

```ini
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

table main
{
    source = main
    path = /path/to/main
    # ... all the other settings
}

**note how all other settings are copied from main, but source and path are overridden (they MUST be)**
table delta : main
{
    source = delta
    path = /path/to/delta
}
```

A better split variable is to use a timestamp column instead of the ID as timestamps can track not just new documents, but also modified ones.

For the datasets that can have documents modified or deleted, the delta table should also provide a list with documents that suffered changes in order to be suppressed and not be used in search queries. This is achieved with the feature called Kill lists. The document ids to be killed can be provided in an auxiliary query defined by [sql_query_killlist](../Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md#Table-kill-list). The delta must point the tables for which the kill-lists will be applied by directive [killlist_target](../Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md#killlist_target). The effect of kill-lists is permanent on the target table, meaning even if the search is made without the delta table, the suppressed documents will not appear in searches.

Note how we're overriding `sql_query_pre` in the delta source. We need to explicitly have that override. Otherwise `REPLACE` query would be run when building the delta source too, effectively nullifying it. However, when we issue the directive in the inherited source for the first time, it removes *all* inherited values, so the encoding setup is also lost. So `sql_query_pre` in the delta can not just be empty; and we need to issue the encoding setup query explicitly once again.
