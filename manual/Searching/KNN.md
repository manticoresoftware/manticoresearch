# K-nearest neighbor search

<!-- example KNN -->

Manticore can perform k-nearest neighbor (KNN) searches using the HNSW library.

The KNN search functionality is provided by the [Manticore Columnar Library](https://github.com/manticoresoftware/columnar)

To run KNN searches, you must first configure your table. It needs to have at least one float_vector attribute, which serves as a data vector. You need to specify the following properties:
* `knn_type`: A mandatory setting; currently, only `hnsw` is supported.
* `knn_dims`: A mandatory setting that specifies the dimensions of the vectors being indexed.
* `hnsw_similarity`: A mandatory setting that specifies the distance function used by the HNSW index. Acceptable values are `L2`, `IP`, and `COSINE`.
* `hnsw_m`: An optional setting that defines the maximum number of outgoing connections in the graph. The default is 16.
* `hnsw_ef_construction`: An optional setting that defines a construction time/accuracy trade-off.

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
create table test ( title text, image_vector float_vector knn_type='hnsw' knn_dims='4' hnsw_similarity='l2' );
```
<!-- response -->

```sql
Query OK, 0 rows affected (0.01 sec)
```
<!-- end -->

After creating the table, you need to insert your vector data, ensuring it matches the dimensions you specified when creating the table.

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
insert into test values ( 1, 'yellow bag', (0.653448,0.192478,0.017971,0.339821) ), ( 2, 'white bag', (-0.148894,0.748278,0.091892,-0.095406) );
```
<!-- response -->

```sql
Query OK, 2 rows affected (0.00 sec)
```
<!-- end -->

Now, you can initiate the KNN search using the `knn()` clause.

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
select id, knn_dist() from test where knn ( image_vector, 5, (0.286569,-0.031816,0.066684,0.032926) );
```
<!-- response -->

```sql
+------+------------+
| id   | knn_dist() |
+------+------------+
|    1 | 0.28146550 |
|    2 | 0.81527930 |
+------+------------+
2 rows in set (0.00 sec)
```
<!-- end -->

The `knn()` clause has the following syntax: `knn ( <attribute_name>, <k>, <query_vector> )`
* `attribute_name` is the name of the float vector attribute that contains vector data.
* `k` is the number of documents to return. Note that this represents how many documents a single HNSW index will return. The result may have more documents than `k` (e.g., each disk chunk in a real-time table returns `k` docs, resulting in `num_chunks*k` docs), or it may have fewer than `k` docs (e.g., you request `k` docs and filter them by some attribute).
* `<query_vector>` is the search vector.

Documents are always sorted by the distance to the search vector. Any additional sorting that you may specify will be performed after that primary sort condition. There is a built-in function `knn_dist()` that can be used to retrieve the distance.

Manticore also supports additional filtering of documents returned by the KNN search, either by full-text matching, attribute filters, or both.

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
select id, knn_dist() from test where knn ( image_vector, 5, (0.286569,-0.031816,0.066684,0.032926) ) and match('white') and id<10;
```
<!-- response -->

```sql
+------+------------+
| id   | knn_dist() |
+------+------------+
|    2 | 0.81527930 |
+------+------------+
1 row in set (0.00 sec)
```

<!-- end -->

<!-- proofread -->
