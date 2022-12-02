# Cost-based optimizer

When Manticore executes a query it can either use plain scan and check every document against the filters or use additional data and/or algorithms to speed up query execution.
The following options are currently supported:
1. **Lookup by docid** which uses a secondary by document ids stored in files with the extension `.spt`. Manticore uses it to:
   * speed up document id->row id lookups
   * filters over document ids: `WHERE id = `, `WHERE id in (...)`
2. **Columnar analyzer**. Can be used instead of a plain filter on a columnar attribute. It still does a scan of every value and tests it against the filter, but it is heavily optimized and is usually noticeably faster than the default approach.
3. **Secondary indexes**. Secondary indexes are generated for all attributes by default. It uses [PGM index](https://pgm.di.unipi.it/) together with the built-in Manticore's inverted index to retrieve the list of row ids corresponding to a value or a range of values.
The secondary indexes are stored in files `.spidx`.

Manticore uses query cost-based optimizer (CBO) to decide which path to take.

The optimizer estimates the cost of each execution path using attribute statistics. The statistics include information on data distribution within an attribute (histograms, stored in `.sphi`). Histograms are generated automatically when data is indexed.

The optimizer calculates the execution cost for every filter used in a query. Since certain filters can be replaced with several different entities (e.g., for a document id Manticore can use a plain scan, a lookup by the document id, the columnar analyzer (if the document id is columnar) and a secondary index), the optimizer evaluates every available combination. Note that there's a maximum limit of the number of combinations (1024).

To estimate query execution costs, the optimizer calculates estimated costs of the most significant operations that are performed when the query is executed. The optimizer uses preset constants that represent the cost of each operation.

The optimizer compares costs of each execution path and the path with the least cost is used for executing the query.

Currently the optimizer only uses CPU costs. Memory and disk usage is not considered.
