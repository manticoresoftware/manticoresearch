# SHOW TABLE STATUS

<!-- example SHOW TABLE STATUS -->

`SHOW TABLE STATUS` 是一个SQL语句，用于显示每个表的各种统计信息。

语法如下：

```sql
SHOW TABLE index_name STATUS
```

根据索引类型，显示的统计信息包含不同的行集：

- **模板表**: `index_type`
- **分布式表**: `index_type`，`query_time_1min`，`query_time_5min`，`query_time_15min`，`query_time_total`，`exact_query_time_1min`，`exact_query_time_5min`，`exact_query_time_15min`，`exact_query_time_total`，`found_rows_1min`，`found_rows_5min`，`found_rows_15min`，`found_rows_total`
- **过滤表**: `index_type`，`stored_queries`，`ram_bytes`，`disk_bytes`，`max_stack_need`，`average_stack_base`，`desired_thread_stack`，`tid`，`tid_saved`，`query_time_1min`，`query_time_5min`，`query_time_15min`，`query_time_total`，`exact_query_time_1min`，`exact_query_time_5min`，`exact_query_time_15min`，`exact_query_time_total`，`found_rows_1min`，`found_rows_5min`，`found_rows_15min`，`found_rows_total`
- **普通表**: `index_type`，`indexed_documents`，`indexed_bytes`，可能包含`field_tokens_*` 和 `total_tokens` 的集合，`ram_bytes`，`disk_bytes`，`disk_mapped`，`disk_mapped_cached`，`disk_mapped_doclists`，`disk_mapped_cached_doclists`，`disk_mapped_hitlists`，`disk_mapped_cached_hitlists`，`killed_documents`，`killed_rate`，`query_time_1min`，`query_time_5min`，`query_time_15min`，`query_time_total`，`exact_query_time_1min`，`exact_query_time_5min`，`exact_query_time_15min`，`exact_query_time_total`，`found_rows_1min`，`found_rows_5min`，`found_rows_15min`，`found_rows_total`
- **实时表**: `index_type`，`indexed_documents`，`indexed_bytes`，可能包含`field_tokens_*` 和 `total_tokens` 的集合，`ram_bytes`，`disk_bytes`，`disk_mapped`，`disk_mapped_cached`，`disk_mapped_doclists`，`disk_mapped_cached_doclists`，`disk_mapped_hitlists`，`disk_mapped_cached_hitlists`，`killed_documents`，`killed_rate`，`ram_chunk`，`ram_chunk_segments_count`，`disk_chunks`，`mem_limit`，`mem_limit_rate`，`ram_bytes_retired`，`optimizing`，`locked`，`tid`，`tid_saved`，`query_time_1min`，`query_time_5min`，`query_time_15min`，`query_time_total`，`exact_query_time_1min`，`exact_query_time_5min`，`exact_query_time_15min`，`exact_query_time_total`，`found_rows_1min`，`found_rows_5min`，`found_rows_15min`，`found_rows_total`

这些值的含义如下：

- `index_type`: 当前索引类型之一，如 `disk`，`rt`，`percolate`，`template` 和 `distributed`
- `indexed_documents`: 已索引的文档数量
- `indexed_bytes`: 已索引文本的总大小
- `stored_queries`: 表中存储的过滤查询数量
- `field_tokens_XXX`: 可选，每个字段的总词元数量，用于`BM25A` 和 `BM25F` 排名函数
- `total_tokens`: 可选，所有`field_tokens_XXX`的总和
- `ram_bytes`: 表占用的总内存
- `disk_bytes`: 表占用的总磁盘空间
- `disk_mapped`: 文件映射的总大小
- `disk_mapped_cached`: 实际缓存到内存中的文件映射总大小
- `disk_mapped_doclists` 和 `disk_mapped_cached_doclists`: 属于文档列表的总映射和缓存映射部分
- `disk_mapped_hitlists` 和 `disk_mapped_cached_hitlists`: 属于命中列表的总映射和缓存映射部分
- `killed_documents` 和 `killed_rate`: 被删除的文档数量及其删除率
- `ram_chunk`: 实时或过滤表的内存块大小
- `ram_chunk_segments_count`: 内存块内部由多个段组成，通常不超过32段
- `disk_chunks`: 实时表中的磁盘块数量
- `mem_limit`: 实时表的`rt_mem_limit`值
- `mem_limit_rate`: 内存块被刷新为磁盘块的比率
- `ram_bytes_retired`: 内存块中的垃圾大小
- `optimizing`: 大于0表示表当前正在进行优化
- `locked`: 大于0表示表当前被锁定
- `max_stack_need`: 计算最复杂的过滤查询所需的栈空间
- `average_stack_base`: 计算过滤查询时通常占用的栈空间
- `desired_thread_stack`: 所需的线程栈大小
- `tid` 和 `tid_saved`: 表保存状态的标识
- `query_time_*` 和 `exact_query_time_*`: 查询执行时间统计数据
- `found_rows_*`: 查询找到的行数统计数据

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
mysql> SHOW TABLE statistic STATUS;
```

<!-- response SQL -->

```sql
+-----------------------------+--------------------------------------------------------------------------+
| Variable_name               | Value                                                                    |
+-----------------------------+--------------------------------------------------------------------------+
| index_type                  | rt                                                                       |
| indexed_documents           | 146000                                                                   |
| indexed_bytes               | 149504000                                                                |
| ram_bytes                   | 87674788                                                                 |
| disk_bytes                  | 1762811                                                                  |
| disk_mapped                 | 794147                                                                   |
| disk_mapped_cached          | 802816                                                                   |
| disk_mapped_doclists        | 0                                                                        |
| disk_mapped_cached_doclists | 0                                                                        |
| disk_mapped_hitlists        | 0                                                                        |
| disk_mapped_cached_hitlists | 0                                                                        |
| killed_documents            | 0                                                                        |
| killed_rate                 | 0.00%                                                                    |
| ram_chunk                   | 86865484                                                                 |
| ram_chunk_segments_count    | 24                                                                       |
| disk_chunks                 | 1                                                                        |
| mem_limit                   | 134217728                                                                |
| mem_limit_rate              | 95.00%                                                                   |
| ram_bytes_retired           | 0                                                                        |
| optimizing                  | 1                                                                        |
| locked                      | 0                                                                        |
| tid                         | 0                                                                        |
| tid_saved                   | 0                                                                        |
| query_time_1min             | {"queries":0, "avg":"-", "min":"-", "max":"-", "pct95":"-", "pct99":"-"} |
| query_time_5min             | {"queries":0, "avg":"-", "min":"-", "max":"-", "pct95":"-", "pct99":"-"} |
| query_time_15min            | {"queries":0, "avg":"-", "min":"-", "max":"-", "pct95":"-", "pct99":"-"} |
| query_time_total            | {"queries":0, "avg":"-", "min":"-", "max":"-", "pct95":"-", "pct99":"-"} |
| found_rows_1min             | {"queries":0, "avg":"-", "min":"-", "max":"-", "pct95":"-", "pct99":"-"} |
| found_rows_5min             | {"queries":0, "avg":"-", "min":"-", "max":"-", "pct95":"-", "pct99":"-"} |
| found_rows_15min            | {"queries":0, "avg":"-", "min":"-", "max":"-", "pct95":"-", "pct99":"-"} |
| found_rows_total            | {"queries":0, "avg":"-", "min":"-", "max":"-", "pct95":"-", "pct99":"-"} |
+-----------------------------+--------------------------------------------------------------------------+
29 rows in set (0.00 sec)
```

<!-- intro -->
##### PHP:
<!-- request PHP -->

``` php
$index->status();
```

<!-- response PHP -->

```php
Array(
    [index_type] => rt
    [indexed_documents] => 3
    [indexed_bytes] => 0
    [ram_bytes] => 6678
    [disk_bytes] => 611
    [ram_chunk] => 990
    [ram_chunk_segments_count] => 2
    [mem_limit] => 134217728
    [ram_bytes_retired] => 0
    [optimizing] => 0
    [locked] => 0
    [tid] => 15
    [query_time_1min] => {"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}
    [query_time_5min] => {"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}
    [query_time_15min] => {"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}
    [query_time_total] => {"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}
    [found_rows_1min] => {"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}
    [found_rows_5min] => {"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}
    [found_rows_15min] => {"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}
    [found_rows_total] => {"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}

)
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('SHOW TABLE statistic STATUS')
```
<!-- response Python -->

```python
{u'columns': [{u'Key': {u'type': u'string'}},
              {u'Value': {u'type': u'string'}}],
 u'data': [
   {u'Key': u'index_type', u'Value': u'rt'}
    {u'Key': u'indexed_documents', u'Value': u'3'}
    {u'Key': u'indexed_bytes', u'Value': u'0'}
    {u'Key': u'ram_bytes', u'Value': u'6678'}
    {u'Key': u'disk_bytes', u'Value': u'611'}
    {u'Key': u'ram_chunk', u'Value': u'990'}
    {u'Key': u'ram_chunk_segments_count', u'Value': u'2'}
    {u'Key': u'mem_limit', u'Value': u'134217728'}
    {u'Key': u'ram_bytes_retired', u'Value': u'0'}
    {u'Key': u'optimizing', u'Value': u'0'}
    {u'Key': u'locked', u'Value': u'0'}
    {u'Key': u'tid', u'Value': u'15'}
    {u'Key': u'query_time_1min', u'Value': u'{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}'}
    {u'Key': u'query_time_5min', u'Value': u'{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}'}
    {u'Key': u'query_time_15min', u'Value': u'{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}'}
    {u'Key': u'query_time_total', u'Value': u'{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}'}
    {u'Key': u'found_rows_1min', u'Value': u'{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}'}
    {u'Key': u'found_rows_5min', u'Value': u'{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}'}
    {u'Key': u'found_rows_15min', u'Value': u'{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}'}
    {u'Key': u'found_rows_total', u'Value': u'{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}'}],
 u'error': u'',
 u'total': 0,
 u'warning': u''}
```
<!-- intro -->
##### Javascript:

<!-- request Javascript -->

```javascript
res = await utilsApi.sql('SHOW TABLE statistic STATUS');
```
<!-- response Javascript -->

```javascript
{"columns": [{"Key": {"type": "string"}},
              {"Value": {"type": "string"}}],
 "data": [
   {"Key": "index_type", "Value": "rt"}
    {"Key": "indexed_documents", "Value": "3"}
    {"Key": "indexed_bytes", "Value": "0"}
    {"Key": "ram_bytes", "Value": "6678"}
    {"Key": "disk_bytes", "Value": "611"}
    {"Key": "ram_chunk", "Value": "990"}
    {"Key": "ram_chunk_segments_count", "Value": "2"}
    {"Key": "mem_limit", "Value": "134217728"}
    {"Key": "ram_bytes_retired", "Value": "0"}
    {"Key": "optimizing", "Value": "0"}
    {"Key": "locked", "Value": "0"}
    {"Key": "tid", "Value": "15"}
    {"Key": "query_time_1min", "Value": "{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}"}
    {"Key": "query_time_5min", "Value": "{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}"}
    {"Key": "query_time_15min", "Value": "{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}"}
    {"Key": "query_time_total", "Value": "{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}"}
    {"Key": "found_rows_1min", "Value": "{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}"}
    {"Key": "found_rows_5min", "Value": "{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}"}
    {"Key": "found_rows_15min", "Value": "{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}"}
    {"Key": "found_rows_total", "Value": "{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}"}],
 "error": "",
 "total": 0,
 "warning": ""}
```
<!-- intro -->
##### Java:

<!-- request Java -->

```java
utilsApi.sql("SHOW TABLE statistic STATUS");

```
<!-- response Java -->
```java
{columns=[{ Key : { type=string }},
              { Value : { type=string }}],
  data : [
   { Key=index_type, Value=rt}
    { Key=indexed_documents, Value=3}
    { Key=indexed_bytes, Value=0}
    { Key=ram_bytes, Value=6678}
    { Key=disk_bytes, Value=611}
    { Key=ram_chunk, Value=990}
    { Key=ram_chunk_segments_count, Value=2}
    { Key=mem_limit, Value=134217728}
    { Key=ram_bytes_retired, Value=0}
    { Key=optimizing, Value=0}
    { Key=locked, Value=0}
    { Key=tid, Value=15}
    { Key=query_time_1min, Value={queries:1, avg_sec:0.001, min_sec:0.001, max_sec:0.001, pct95_sec:0.001, pct99_sec:0.001}}
    { Key=query_time_5min, Value={queries:1, avg_sec:0.001, min_sec:0.001, max_sec:0.001, pct95_sec:0.001, pct99_sec:0.001}}
    { Key=query_time_15min, Value={queries:1, avg_sec:0.001, min_sec:0.001, max_sec:0.001, pct95_sec:0.001, pct99_sec:0.001}}
    { Key=query_time_total, Value={queries:1, avg_sec:0.001, min_sec:0.001, max_sec:0.001, pct95_sec:0.001, pct99_sec:0.001}}
    { Key=found_rows_1min, Value={queries:1, avg:3, min:3, max:3, pct95:3, pct99:3}}
    { Key=found_rows_5min, Value={queries:1, avg:3, min:3, max:3, pct95:3, pct99:3}}
    { Key=found_rows_15min, Value={queries:1, avg:3, min:3, max:3, pct95:3, pct99:3}}
    { Key=found_rows_total, Value={queries:1, avg:3, min:3, max:3, pct95:3, pct99:3}}],
  error= ,
  total=0,
  warning= }
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("SHOW TABLE statistic STATUS");

```
<!-- response C# -->
```clike
{columns=[{ Key : { type=string }},
              { Value : { type=string }}],
  data : [
   { Key=index_type, Value=rt}
    { Key=indexed_documents, Value=3}
    { Key=indexed_bytes, Value=0}
    { Key=ram_bytes, Value=6678}
    { Key=disk_bytes, Value=611}
    { Key=ram_chunk, Value=990}
    { Key=ram_chunk_segments_count, Value=2}
    { Key=mem_limit, Value=134217728}
    { Key=ram_bytes_retired, Value=0}
    { Key=optimizing, Value=0}
    { Key=locked, Value=0}
    { Key=tid, Value=15}
    { Key=query_time_1min, Value={queries:1, avg_sec:0.001, min_sec:0.001, max_sec:0.001, pct95_sec:0.001, pct99_sec:0.001}}
    { Key=query_time_5min, Value={queries:1, avg_sec:0.001, min_sec:0.001, max_sec:0.001, pct95_sec:0.001, pct99_sec:0.001}}
    { Key=query_time_15min, Value={queries:1, avg_sec:0.001, min_sec:0.001, max_sec:0.001, pct95_sec:0.001, pct99_sec:0.001}}
    { Key=query_time_total, Value={queries:1, avg_sec:0.001, min_sec:0.001, max_sec:0.001, pct95_sec:0.001, pct99_sec:0.001}}
    { Key=found_rows_1min, Value={queries:1, avg:3, min:3, max:3, pct95:3, pct99:3}}
    { Key=found_rows_5min, Value={queries:1, avg:3, min:3, max:3, pct95:3, pct99:3}}
    { Key=found_rows_15min, Value={queries:1, avg:3, min:3, max:3, pct95:3, pct99:3}}
    { Key=found_rows_total, Value={queries:1, avg:3, min:3, max:3, pct95:3, pct99:3}}],
  error="" ,
  total=0,
  warning="" }
```

<!-- intro -->
##### TypeScript:

<!-- request TypeScript -->

```typescript
res = await utilsApi.sql('SHOW TABLE statistic STATUS');
```
<!-- response TypeScript -->

```typescript
{
	"columns": 
	[{
		"Key": {"type": "string"}
	},
    {
    	"Value": {"type": "string"}
    }],
	"data": 
	[
		{"Key": "index_type", "Value": "rt"}
	    {"Key": "indexed_documents", "Value": "3"}
	    {"Key": "indexed_bytes", "Value": "0"}
	    {"Key": "ram_bytes", "Value": "6678"}
	    {"Key": "disk_bytes", "Value": "611"}
	    {"Key": "ram_chunk", "Value": "990"}
	    {"Key": "ram_chunk_segments_count", "Value": "2"}
	    {"Key": "mem_limit", "Value": "134217728"}
	    {"Key": "ram_bytes_retired", "Value": "0"}
	    {"Key": "optimizing", "Value": "0"}
	    {"Key": "locked", "Value": "0"}
	    {"Key": "tid", "Value": "15"}
	    {"Key": "query_time_1min", "Value": "{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}"}
	    {"Key": "query_time_5min", "Value": "{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}"}
	    {"Key": "query_time_15min", "Value": "{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}"}
	    {"Key": "query_time_total", "Value": "{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}"}
	    {"Key": "found_rows_1min", "Value": "{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}"}
	    {"Key": "found_rows_5min", "Value": "{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}"}
	    {"Key": "found_rows_15min", "Value": "{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}"}
	    {"Key": "found_rows_total", "Value": "{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}"}
	],
	"error": "",
	"total": 0,
	"warning": ""
}
```

<!-- intro -->
##### Go:

<!-- request Go -->

```go
apiClient.UtilsAPI.Sql(context.Background()).Body("SHOW TABLE statistic STATUS").Execute()
```
<!-- response Go -->

```go
{
	"columns": 
	[{
		"Key": {"type": "string"}
	},
    {
    	"Value": {"type": "string"}
    }],
	"data": 
	[
		{"Key": "index_type", "Value": "rt"}
	    {"Key": "indexed_documents", "Value": "3"}
	    {"Key": "indexed_bytes", "Value": "0"}
	    {"Key": "ram_bytes", "Value": "6678"}
	    {"Key": "disk_bytes", "Value": "611"}
	    {"Key": "ram_chunk", "Value": "990"}
	    {"Key": "ram_chunk_segments_count", "Value": "2"}
	    {"Key": "mem_limit", "Value": "134217728"}
	    {"Key": "ram_bytes_retired", "Value": "0"}
	    {"Key": "optimizing", "Value": "0"}
	    {"Key": "locked", "Value": "0"}
	    {"Key": "tid", "Value": "15"}
	    {"Key": "query_time_1min", "Value": "{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}"}
	    {"Key": "query_time_5min", "Value": "{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}"}
	    {"Key": "query_time_15min", "Value": "{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}"}
	    {"Key": "query_time_total", "Value": "{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}"}
	    {"Key": "found_rows_1min", "Value": "{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}"}
	    {"Key": "found_rows_5min", "Value": "{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}"}
	    {"Key": "found_rows_15min", "Value": "{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}"}
	    {"Key": "found_rows_total", "Value": "{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}"}
	],
	"error": "",
	"total": 0,
	"warning": ""
}
```

<!-- end -->
