# 搜索结果分页

<!-- example general -->

Manticore Search 默认返回结果集中排名靠前的 20 个匹配文档。

#### SQL
在 SQL 中，您可以使用 `LIMIT` 子句遍历结果集。

`LIMIT` 可以接受一个数字作为返回集的大小，偏移量默认为 0；也可以接受一对偏移量和大小的值。

#### HTTP JSON
在使用 HTTP JSON 时，`offset` 和 `limit` 节点控制结果集的偏移量和返回集的大小。或者，您也可以使用 `size`和 `from` 这对节点来代替。

<!-- intro -->

<!-- request SQL -->

```sql
SELECT  ... FROM ...  [LIMIT [offset,] row_count]
SELECT  ... FROM ...  [LIMIT row_count][ OFFSET offset]
```


<!-- request JSON -->

```json
{
  "index": "<index_name>",
  "query": ...
  ...  
  "limit": 20,
  "offset": 0
}
{
  "index": "<index_name>",
  "query": ...
  ...  
  "size": 20,
  "from": 0
}
```

<!-- end -->

<!-- example maxMatches -->
### 结果集窗口

默认情况下，Manticore Search 使用包含 1000 个最佳排名文档的结果集窗口。若分页超出此值，查询将会报错。

此限制可以通过查询选项 [max_matches](../Searching/Options.md#max_matches) 进行调整。

仅在确有必要时才建议将 `max_matches` 设置为很高的值。高 `max_matches` 值会消耗更多内存，并可能增加查询响应时间。处理深度结果集的一种方法是将 `max_matches` 设置为偏移量和返回集大小之和。

将 `max_matches` 设定为低于 1000 可以减少查询所使用的内存，也可能缩短查询时间，但在大多数情况下，这样的收益可能不会很明显。

<!-- intro -->


<!-- request SQL -->

```sql
SELECT  ... FROM ...   OPTION max_matches=<value>
```


<!-- request JSON -->


```json
{
  "index": "<index_name>",
  "query": ...
  ...
  "max_matches":<value>
  }
}

```

<!-- end -->
<!-- proofread -->