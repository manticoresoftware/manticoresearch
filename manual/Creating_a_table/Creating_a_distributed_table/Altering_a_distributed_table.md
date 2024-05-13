# Altering a distributed table

<!-- example local_dist -->

You may wish to adjust settings for distributed tables.
ManticoreSearch allows the use of `ALTER` syntax for such tables.


### General syntax of ALTER TABLE for distributed tables

```sql
ALTER TABLE `distr_table_name` type='distributed' [[local='local_index_name'], [agent='host:port:remote_index'] ... ]
```

<!-- intro -->
<!-- request RT mode -->
```sql
ALTER TABLE local_dist type='distributed' local='index1' local='index2' agent='127.0.0.1:9312:remote_index';
```

<!-- end -->
<!-- proofread -->
