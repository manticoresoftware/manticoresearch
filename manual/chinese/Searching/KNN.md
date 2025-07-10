# K-近邻向量搜索

Manticore Search 支持向每个文档添加由您的机器学习模型生成的嵌入向量，然后对其进行最近邻搜索。这使您能够构建诸如相似性搜索、推荐、语义搜索和基于自然语言处理算法的相关性排序等功能，包括图像、视频和声音搜索。

## 什么是嵌入向量？

嵌入向量是一种将数据（如文本、图像或声音）表示为高维空间向量的方法。这些向量经过设计，使得它们之间的距离反映了所代表数据的相似性。该过程通常采用词嵌入算法（例如 Word2Vec、BERT）用于文本，或神经网络用于图像。向量空间的高维特性，每个向量包含许多分量，可以表示项之间复杂且细微的关系。它们的相似性通过这些向量之间的距离来衡量，通常使用欧氏距离或余弦相似度等方法。

Manticore Search 使用 HNSW 库支持 k-近邻 (KNN) 向量搜索。此功能属于 [Manticore Columnar Library](https://github.com/manticoresoftware/columnar)。

<!-- example KNN -->

### 配置用于 KNN 搜索的表

要运行 KNN 搜索，您必须首先配置表。浮点向量和 KNN 搜索仅支持实时表（不支持普通表）。表中需要至少包含一个 float_vector 属性，作为数据向量。您需要指定以下属性：
* `knn_type`：必填设置；当前仅支持 `hnsw`。
* `knn_dims`：必填设置，指定正在索引的向量维度。
* `hnsw_similarity`：必填设置，指定 HNSW 索引使用的距离函数。可接受的值为：
  - `L2` - 平方 L2 距离
  - `IP` - 内积
  - `COSINE` - 余弦相似度
* `hnsw_m`：可选设置，定义图中最大的出边数量。默认值为 16。
* `hnsw_ef_construction`：可选设置，定义构建时的时间/准确度权衡。

<!-- intro -->
##### SQL

<!-- request SQL -->
```sql
create table test ( title text, image_vector float_vector knn_type='hnsw' knn_dims='4' hnsw_similarity='l2' );
```

<!-- response SQL -->

```sql
Query OK, 0 rows affected (0.01 sec)
```

<!-- intro -->
##### 普通模式（使用配置文件）：

<!-- request Config -->
```ini
table test_vec {
    type = rt
	...
    rt_attr_float_vector = image_vector
    knn = {"attrs":[{"name":"image_vector","type":"hnsw","dims":4,"hnsw_similarity":"L2","hnsw_m":16,"hnsw_ef_construction":200}]}
}
```

<!-- end -->

<!-- example knn_insert -->

### 插入向量数据

创建表后，您需要插入向量数据，确保其维度与创建表时指定的相匹配。您也可以插入空向量；这意味着该文档将从向量搜索结果中排除。

<!-- intro -->
##### SQL：

<!-- request SQL -->

```sql
insert into test values ( 1, 'yellow bag', (0.653448,0.192478,0.017971,0.339821) ), ( 2, 'white bag', (-0.148894,0.748278,0.091892,-0.095406) );
```
<!-- response SQL -->

```sql
Query OK, 2 rows affected (0.00 sec)
```

<!-- intro -->
##### JSON：

<!-- request JSON -->

```json
POST /insert
{
	"table":"test_vec",
	"id":1,
	"doc": 	{ "title" : "yellow bag", "image_vector" : [0.653448,0.192478,0.017971,0.339821] }
}

POST /insert
{
	"table":"test_vec",
	"id":2,
	"doc": 	{ "title" : "white bag", "image_vector" : [-0.148894,0.748278,0.091892,-0.095406] }
}
```

<!-- response JSON -->

```json
{
	"table":"test",
	"_id":1,
	"created":true,
	"result":"created",
	"status":201
}

{
	"table":"test",
	"_id":2,
	"created":true,
	"result":"created",
	"status":201
}
```

<!-- end -->

<!-- example knn_search -->

### KNN 向量搜索

现在，您可以使用 SQL 或 JSON 格式中的 `knn` 子句执行 KNN 搜索。两个接口支持相同的核心参数，确保无论您选择哪种格式，都能获得一致的体验：

- SQL: `select ... from <table name> where knn ( <field>, <k>, <query vector> [,<options>] )`
- JSON:
  ```
  POST /search
  {
      "table": "<table name>",
      "knn":
      {
          "field": "<field>",
          "query_vector": [<query vector>],
          "k": <k>,
          "ef": <ef>,
  "rescore": <rescore>,
  "oversampling": <oversampling>
      }
  }
  ```

参数说明：
* `field`：这是包含向量数据的 float_vector 属性名称。
* `k`：表示返回的文档数量，是分层导航小世界 (HNSW) 索引的关键参数。它指定单个 HNSW 索引应返回的文档数量。然而，最终结果中包含的文档实际数量可能不同。例如，在处理分为磁盘块的实时表时，每个块可能返回 `k` 个文档，这样总数就超过指定的 `k`（累积计数为 `num_chunks * k`）。另一方面，如果在请求 `k` 个文档后，部分文档因特定属性被过滤掉，那么最终的文档数量可能少于 `k`。需要注意的是，`k` 参数不适用于内存块（ramchunks）。对内存块来说，检索过程不同，因此 `k` 参数对返回文档数量的影响不适用。
* `query_vector`：这是搜索向量。
* `ef`：搜索时使用的动态列表大小，可选。`ef` 越高，搜索越准确，但越慢。
* `rescore`：启用 KNN 重新评分（默认禁用）。设置为 SQL 中的 `1` 或 JSON 中的 `true` 可启用重新评分。在使用量化向量（可能带有过采样）完成 KNN 搜索后，距离会使用原始（全精度）向量重新计算，结果重新排序以提高排名准确性。
* `oversampling`：设置一个因子（浮点值），此因子乘以 `k` 执行 KNN 搜索时会检索更多候选项，使用量化向量。默认不使用过采样。如果启用重新评分，这些候选项可以稍后被重新评估。

文档总是按它们与搜索向量的距离排序。您指定的任何附加排序条件将在此主排序条件之后应用。若要获取距离，可以使用内置函数 [knn_dist()](../Functions/Other_functions.md#KNN_DIST%28%29)。

<!-- intro -->
##### SQL：

<!-- request SQL -->

```sql
select id, knn_dist() from test where knn ( image_vector, 5, (0.286569,-0.031816,0.066684,0.032926), { ef=2000, oversampling=3.0, rescore=1 } );
```
<!-- response SQL -->

```sql
+------+------------+
| id   | knn_dist() |
+------+------------+
|    1 | 0.28146550 |
|    2 | 0.81527930 |
+------+------------+
2 rows in set (0.00 sec)
```

<!-- intro -->
##### JSON：

<!-- request JSON -->

```json
POST /search
{
	"table": "test",
	"knn":
	{
		"field": "image_vector",
		"query_vector": [0.286569,-0.031816,0.066684,0.032926],
		"k": 5,
		"ef": 2000, 
		"rescore": true,
		"oversampling": 3.0
	}
}
```

<!-- response JSON -->

```json
{
	"took":0,
	"timed_out":false,
	"hits":
	{
		"total":2,
		"total_relation":"eq",
		"hits":
		[
			{
				"_id": 1,
				"_score":1,
				"_knn_dist":0.28146550,
				"_source":
				{
					"title":"yellow bag",
					"image_vector":[0.653448,0.192478,0.017971,0.339821]
				}
			},
			{
				"_id": 2,
				"_score":1,
				"_knn_dist":0.81527930,
				"_source":
				{
					"title":"white bag",
					"image_vector":[-0.148894,0.748278,0.091892,-0.095406]
				}
			}
		]
	}
}
```

<!-- end -->

<!-- example knn_quantization -->

### 向量量化

HNSW 索引需要完全加载到内存中才能执行 KNN 搜索，这可能导致显著的内存消耗。为了减少内存使用，可以应用标量量化——一种通过用有限数量的离散值表示每个分量（维度）来压缩高维向量的技术。Manticore 支持 8 位和 1 位量化，这意味着每个向量分量从 32 位浮点数压缩为 8 位甚至 1 位，分别将内存使用减少 4 倍或 32 倍。这些压缩表示还允许更快的距离计算，因为更多的向量分量可以在单个 SIMD 指令中处理。虽然标量量化会引入一定的近似误差，但它通常是在搜索准确性和资源效率之间的一个值得权衡。为了获得更好的准确性，量化可以与重评分和过采样结合使用：检索的候选项比请求的更多，并使用原始的 32 位浮点向量重新计算这些候选项的距离。

支持的量化类型包括：
* `8bit`：每个向量分量量化为 8 位。
* `1bit`：每个向量分量量化为 1 位。使用非对称量化，查询向量量化为 4 位，存储向量量化为 1 位。这种方法比更简单的方法提供更高的精度，尽管有一定的性能折衷。
* `1bitsimple`：每个向量分量量化为 1 位。此方法比 `1bit` 更快，但通常不那么准确。

<!-- intro -->
##### SQL:

<!-- request SQL -->
```sql
create table test ( title text, image_vector float_vector knn_type='hnsw' knn_dims='4' hnsw_similarity='l2' quantization='1bit');
```

<!-- response SQL -->

```sql
Query OK, 0 rows affected (0.01 sec)
```
<!-- end -->

<!-- Example knn_similar_docs -->

### 通过 id 查找相似文档

> 注意：通过 id 查找相似文档需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果无法工作，请确保已安装 Buddy。

基于唯一 ID 查找与某一特定文档相似的文档是一个常见任务。例如，当用户查看某个特定项目时，Manticore Search 可以高效地识别并显示在向量空间中与之最相似的项目列表。操作方法如下：

- SQL：`select ... from <table name> where knn ( <field>, <k>, <document id> )`
- JSON：
  ```
  POST /search
  {
      "table": "<table name>",
      "knn":
      {
          "field": "<field>",
          "doc_id": <document id>,
          "k": <k>
      }
  }
  ```

参数说明：
* `field`：这是包含向量数据的浮点向量属性名称。
* `k`：表示返回的文档数量，是层次导航小世界（HNSW）索引的关键参数。它指定单个 HNSW 索引应返回的文档数量。但最终结果中实际包含的文档数量可能会有所不同。例如，如果系统处理的是分成磁盘块的实时表，则每个块可能返回 `k` 个文档，导致总数超过指定的 `k`（累计数量为 `num_chunks * k`）。另一方面，如果请求了 `k` 个文档后，有部分文档因特定属性被过滤掉，则最终文档数量可能少于 `k`。需要注意的是，参数 `k` 不适用于内存块（ramchunks）。在内存块的情况下，检索过程不同，因此参数 `k` 对返回文档数的影响不适用。
* `document id`：用于 KNN 相似度搜索的文档 ID。


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
select id, knn_dist() from test where knn ( image_vector, 5, 1 );
```
<!-- response SQL -->

```sql
+------+------------+
| id   | knn_dist() |
+------+------------+
|    2 | 0.81527930 |
+------+------------+
1 row in set (0.00 sec)
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```json
POST /search
{
  "table": "test",
  "knn":
  {
    "field": "image_vector",
    "doc_id": 1,
    "k": 5
  }
}
```

<!-- response JSON -->

```json
{
	"took":0,
	"timed_out":false,
	"hits":
	{
		"total":1,
		"total_relation":"eq",
		"hits":
		[
			{
				"_id": 2,
				"_score":1643,
				"_knn_dist":0.81527930,
				"_source":
				{
					"title":"white bag",
					"image_vector":[-0.148894,0.748278,0.091892,-0.095406]
				}
			}
		]
	}
}
```

<!-- end -->

<!-- Example knn_filtering -->

### 过滤 KNN 向量搜索结果

Manticore 还支持通过全文匹配、属性过滤器或两者结合，对 KNN 搜索返回的文档进行额外过滤。

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
select id, knn_dist() from test where knn ( image_vector, 5, (0.286569,-0.031816,0.066684,0.032926) ) and match('white') and id < 10;
```
<!-- response SQL -->

```sql
+------+------------+
| id   | knn_dist() |
+------+------------+
|    2 | 0.81527930 |
+------+------------+
1 row in set (0.00 sec)
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```json
POST /search
{
	"table": "test",
	"knn":
	{
		"field": "image_vector",
		"query_vector": [0.286569,-0.031816,0.066684,0.032926],
		"k": 5,
		"filter":
		{
			"bool":
			{
				"must":
				[
					{ "match": {"_all":"white"} },
			        { "range": { "id": { "lt": 10 } } }
				]
			}
		}
	}
}
```

<!-- response JSON -->

```json
{
	"took":0,
	"timed_out":false,
	"hits":
	{
		"total":1,
		"total_relation":"eq",
		"hits":
		[
			{
				"_id": 2,
				"_score":1643,
				"_knn_dist":0.81527930,
				"_source":
				{
					"title":"white bag",
					"image_vector":[-0.148894,0.748278,0.091892,-0.095406]
				}
			}
		]
	}
}
```

<!-- end -->

<!-- proofread -->

