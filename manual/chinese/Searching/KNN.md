# K近邻向量搜索

Manticore Search 支持将您的机器学习模型生成的嵌入向量添加到每个文档中，然后对其进行近邻搜索。这使您能够构建相似度搜索、推荐、语义搜索和基于自然语言处理算法的相关性排序等功能，还包括图像、视频和声音搜索等。

## 什么是嵌入向量？

嵌入向量是一种将数据（如文本、图像或声音）表示为高维空间中向量的方法。这些向量经过设计，使它们之间的距离能够反映所代表数据的相似性。这个过程通常使用诸如词嵌入（例如 Word2Vec、BERT）用于文本，或神经网络用于图像等算法。向量空间的高维特性，即每个向量具有多个分量，允许表示复杂且细腻的项目间关系。相似性通过这些向量之间的距离来衡量，常用的方法包括欧几里得距离或余弦相似度。

Manticore Search 利用 HNSW 库实现了 k 近邻（KNN）向量搜索功能。该功能是 [Manticore Columnar Library](https://github.com/manticoresoftware/columnar) 的一部分。

<!-- example KNN -->

### 配置表进行 KNN 搜索

要执行 KNN 搜索，您必须先配置表。浮点向量和 KNN 搜索仅支持实时表（不支持普通表）。表需要至少包含一个 float_vector 属性，作为数据向量。您需要指定以下属性：
* `knn_type`：必填设置；目前仅支持 `hnsw`。
* `knn_dims`：必填设置，指定被索引向量的维度。
* `hnsw_similarity`：必填设置，指定 HNSW 索引使用的距离函数。可接受的值有：
  - `L2` - 平方 L2 距离
  - `IP` - 内积
  - `COSINE` - 余弦相似度
* `hnsw_m`：可选设置，定义图中最大出边连接数。默认值为16。
* `hnsw_ef_construction`：可选设置，定义构建时的精度与时间的权衡。

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

创建表之后，您需要插入向量数据，确保其维度与建表时指定的维度一致。您也可以插入空向量；这意味着该文档将从向量搜索结果中排除。

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
insert into test values ( 1, 'yellow bag', (0.653448,0.192478,0.017971,0.339821) ), ( 2, 'white bag', (-0.148894,0.748278,0.091892,-0.095406) );
```
<!-- response SQL -->

```sql
Query OK, 2 rows affected (0.00 sec)
```

<!-- intro -->
##### JSON:

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

现在，您可以使用 SQL 或 JSON 格式中的 `knn` 子句执行 KNN 搜索。这两种接口支持相同的关键参数，确保无论选择哪种格式都有一致的体验：

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
* `field`：包含向量数据的浮点向量属性名称。
* `k`：代表返回的文档数量，是层次可导航小世界（HNSW）索引的关键参数。它指定单个 HNSW 索引应返回的文档数量。但最终结果中的文档数量可能有差异。例如，如果系统处理分割成多个磁盘块的实时表，每个块可能返回 `k` 个文档，导致总数超过指定的 `k`（因为累积数量为 `num_chunks * k`）。另一方面，若请求的 `k` 个文档中，有些因特定属性被过滤，最终文档数可能小于 `k`。需要注意的是，参数 `k` 不适用于 ramchunk。在 ramchunk 的上下文中，检索过程不同，因此 `k` 对返回文档数的影响不适用。
* `query_vector`：搜索向量。
* `ef`：可选，搜索时使用的动态列表大小。较高的 `ef` 值意味着更精确但更慢的搜索。
* `rescore`：启用 KNN 重新评分（默认禁用）。在 SQL 中设置为 `1`，在 JSON 中设置为 `true` 以启用。完成基于量化向量（且可能存在过采样）的 KNN 搜索后，会使用原始（全精度）向量重新计算距离，并重新排序结果以提高排名精度。
* `oversampling`：设置搜索时乘以 `k` 的因子（浮点数），使得使用量化向量检索时返回的候选数量多于需求数量。默认不使用过采样。如果启用了重新评分，这些候选可以被重新评估。过采样同样适用于非量化向量。由于它增加了 `k`，影响 HNSW 索引工作方式，可能引起结果精度的轻微变化。

文档总是按照与搜索向量距离进行排序。您指定的任何额外排序条件将在此主要排序条件之后应用。如需获取距离，有一个内置函数称为 [knn_dist()](../Functions/Other_functions.md#KNN_DIST%28%29)。

<!-- intro -->
##### SQL:

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















































































































































































































