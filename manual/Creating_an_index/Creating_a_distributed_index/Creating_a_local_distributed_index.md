# Creating a local distributed index

<!-- example local_dist -->

A distributed index in Manticore Search doesn't hold any data. Instead it acts as a 'master node' to proxy the demanded query to other indexes and provide merged results from the responses it receives from the 'node' indexes. A distributed index can connect to local indexes or indexes located on other servers. The simplest example of a distributed index looks so:


<!-- intro -->
##### Configuration file:

<!-- request Configuration file -->
```ini
index index_dist {
  type  = distributed
  local = index1
  local = index2
  ...
 }
```

<!-- request RT mode -->
```sql
CREATE TABLE local_dist type='distributed' local='index1' local='index2';
```

<!-- end -->