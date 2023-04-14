# Cost-based optimizer

When Manticore executes a fullscan query, it can either use a plain scan to check every document against the filters or employ additional data and/or algorithms to speed up query execution. Manticore uses a cost-based optimizer (CBO), also known as a "query optimizer" to determine which approach to take.

The CBO can also enhance the performance of full-text queries. See below for more details.

The CBO may decide to replace one or more query filters with one of the following entities if it determines that doing so will improve performance:

1. A **docid index** utilizes a special docid-only secondary index stored in files with the `.spt` extension. Besides improving filters on document IDs, the docid index is also used to accelerate document ID to row ID lookups and to speed up the application of large killlists during daemon startup.
2. A **columnar scan** relies on columnar storage and can only be used on a columnar attribute. It scans every value and tests it against the filter, but it is heavily optimized and is typically faster than the default approach.
3. **Secondary indexes** are generated for all attributes by default. They use the [PGM index](https://pgm.di.unipi.it/) along with Manticore's built-in inverted index to retrieve the list of row IDs corresponding to a value or range of values. Secondary indexes are stored in files with the `.spidx` extension.

The optimizer estimates the cost of each execution path using various attribute statistics, including:

1. Information on the data distribution within an attribute (histograms, stored in `.sphi` files). Histograms are generated automatically when data is indexed and serve as the primary source of information for the CBO.
2. Information from PGM (secondary indexes), which helps estimate the number of document lists to read. This assists in gauging doclist merge performance and in selecting the appropriate merge algorithm (priority queue merge or bitmap merge).
3. Columnar encoding statistics, employed to estimate columnar data decompression performance.
4. A columnar min-max tree. While the CBO uses histograms to estimate the number of documents left after applying the filter, it also needs to determine how many documents the filter had to process. For columnar attributes, partial evaluation of the min-max tree serves this purpose.
5. Full-text dictionary. The CBO utilizes term stats to estimate the cost of evaluating the full-text tree.

The optimizer computes the execution cost for every filter used in a query. Since certain filters can be replaced with several different entities (e.g., for a document id, Manticore can use a plain scan, a docid index lookup, a columnar scan (if the document id is columnar), and a secondary index), the optimizer evaluates all available combinations. However, there is a maximum limit of 1024 combinations.

To estimate query execution costs, the optimizer calculates the estimated costs of the most significant operations performed when executing the query. It uses preset constants to represent the cost of each operation.

The optimizer compares the costs of each execution path and chooses the path with the lowest cost to execute the query.

When working with full-text queries that have filters by attributes, the query optimizer decides between two possible execution paths. One is to execute the full-text query, retrieve the matches, and use filters. The other is to replace filters with one or more entities described above, fetch rowids from them, and inject them into the full-text matching tree. This way, full-text search results will intersect with full-scan results. The query optimizer estimates the cost of full-text tree evaluation and the best possible path for computing filter results. Using this information, the optimizer chooses the execution path.

Another factor to consider is multithreaded query execution (when `pseudo_sharding` is enabled). The CBO is aware that some queries can be executed in multiple threads and takes this into account. The CBO prioritizes shorter query execution times (i.e., latency) over throughput. For instance, if a query using a columnar scan can be executed in multiple threads (and occupy multiple CPU cores) and is faster than a query executed in a single thread using secondary indexes, multithreaded execution will be preferred.

Queries using secondary indexes and docid indexes always run in a single thread, as benchmarks indicate that there is little to no benefit in making them multithreaded.

At present, the optimizer only uses CPU costs and does not take memory or disk usage into account.

<!-- proofread -->