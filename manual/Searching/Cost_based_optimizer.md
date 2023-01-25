# Cost-based optimizer

When Manticore executes a fullscan query, it can either use plain scan to check every document against the filters, or it can use additional data and/or algorithms to speed up the query execution. To decide which approach to take, Manticore uses a query cost-based optimizer ("CBO" also known as "query optimizer").

The CBO may decide to replace one or more query filters with one of the following entities if it determines that it will improve performance:

1. A **docid index**, which uses a special docid-only secondary index stored in files with the `.spt` extension. In addition to improving filters on document ids, the docid index is also used to speed up document id to row id lookups, and to speed up the application of large killlists on daemon startup.
2. A **columnar scan**, which uses columnar storage and can only be used on a columnar attribute. It still scans every value and tests it against the filter, but it is heavily optimized and is usually faster than the default approach.
3. **Secondary indexes**, which are generated for all attributes by default. They use the [PGM index](https://pgm.di.unipi.it/) together with Manticore's built-in inverted index to retrieve the list of row ids corresponding to a value or range of values. The secondary indexes are stored in files with the `.spidx` extension.

The optimizer estimates the cost of each execution path using different attribute statistics, including:

1. Information on the data distribution within an attribute (histograms, stored in `.sphi` files). Histograms are generated automatically when data is indexed and are the main source of information for the CBO.
2. Information from PGM (secondary indexes), which is used to estimate the number of document lists to read. This helps to estimate doclist merge performance and to choose the correct merge algorithm (priority queue merge or bitmap merge).
3. Columnar encoding statistics, which are used to estimate columnar data decompression performance.
4. A columnar min-max tree. The CBO uses histograms to estimate the number of documents left after the filter was applied, but it also needs to estimate how many documents the filter had to process. For columnar attributes, partial evaluation of the min-max tree is used for that purpose.

The optimizer calculates the execution cost for every filter used in a query. Because certain filters can be replaced with several different entities (e.g., for a document id, Manticore can use a plain scan, a docid index lookup, a columnar scan (if the document id is columnar), and a secondary index), the optimizer evaluates every available combination. Note that there is a maximum limit of 1024 combinations.

To estimate query execution costs, the optimizer calculates the estimated costs of the most significant operations that are performed when the query is executed. It uses preset constants to represent the cost of each operation.

The optimizer compares the costs of each execution path and chooses the path with the lowest cost to execute the query.

Another thing to consider is multithreaded query execution (when `pseudo_sharding` is enabled). The CBO knows that some queries can be executed in multiple threads and takes that into account. The CBO favors smaller query execution times (i.e., latency) over throughput. For example, if a query using a columnar scan can be executed in multiple threads (and occupy multiple CPU cores) and is faster than a query executed in a single thread using secondary indexes, multithreaded execution will be preferred.

Queries using secondary indexes and docid indexes always run in a single thread, as benchmarks show that there is little to no benefit in making them multithreaded.

Currently, the optimizer only uses CPU costs and does not consider memory or disk usage.
