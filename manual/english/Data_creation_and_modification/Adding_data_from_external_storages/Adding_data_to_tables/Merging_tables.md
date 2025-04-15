# Merging tables

Merging two existing **plain** tables can be more efficient than indexing the data from scratch and might be desired in some cases (such as merging 'main' and 'delta' tables instead of simply rebuilding 'main' in the 'main+delta' partitioning scheme). Thus,`indexer` provides an option to do that. Merging tables is typically faster than rebuilding, but still **not** instant for huge tables. Essentially, it needs to read the contents of both tables once and write the result once. Merging a 100 GB and 1 GB table, for example, will result in 202 GB of I/O (but that's still likely less than indexing from scratch requires).

The basic command syntax is as follows:

```bash
sudo -u manticore indexer --merge DSTINDEX SRCINDEX [--rotate] [--drop-src]
```

Unless `--drop-src` is specified, only the DSTINDEX table will be affected: the contents of SRCINDEX will be merged into it.

The `--rotate` switch is required if DSTINDEX is already being served by `searchd`.

The typical usage pattern is to merge a smaller update from SRCINDEX into DSTINDEX. Thus, when merging attributes, the values from SRCINDEX will take precedence if duplicate document IDs are encountered. However, note that the "old" keywords will **not** be automatically removed in such cases. For example, if there's a keyword "old" associated with document 123 in DSTINDEX, and a keyword "new" associated with it in SRCINDEX, document 123 will be found by both keywords after the merge. You can supply an explicit condition to remove documents from DSTINDEX to mitigate this; the relevant switch is `--merge-dst-range`:

```bash
sudo -u manticore indexer --merge main delta --merge-dst-range deleted 0 0
```

This switch allows you to apply filters to the destination table along with merging. There can be several filters; all of their conditions must be met in order to include the document in the resulting merged table. In the example above, the filter passes only those records where 'deleted' is 0, eliminating all records that were flagged as deleted.

`--drop-src` enables dropping SRCINDEX after the merge and before rotating the tables, which is important if you specify DSTINDEX in `killlist_target` of DSTINDEX. Otherwise, when rotating the tables, the documents that have been merged into DSTINDEX may be suppressed by SRCINDEX.
<!-- proofread -->