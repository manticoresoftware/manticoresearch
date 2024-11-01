# K 近邻向量搜索

Manticore Search 支持将由机器学习模型生成的嵌入（embeddings）添加到每个文档中，并对其进行最近邻搜索（KNN）。这使得您能够构建类似相似性搜索、推荐、语义搜索以及基于 NLP 算法的相关性排名等功能，甚至可以包括图像、视频和声音的搜索。

## 什么是嵌入（Embedding）？

嵌入是一种将数据（如文本、图像或声音）表示为高维空间向量的方法。这些向量经过精心设计，以确保它们之间的距离反映所表示数据的相似性。该过程通常使用算法，如文本的词向量（例如 Word2Vec、BERT），或用于图像的神经网络。向量空间的高维性质，每个向量包含多个分量，可以代表复杂且微妙的项目关系。通过计算这些向量之间的距离，通常使用如欧几里得距离或余弦相似度来度量相似性。

Manticore Search 使用 HNSW 库实现了 K-nearest Neighbor（KNN）向量搜索。该功能是 [Manticore Columnar Library](https://github.com/manticoresoftware/columnar) 的一部分。

<!-- example KNN -->

### 配置表以进行 KNN 搜索

要运行 KNN 搜索，首先需要配置表。表中必须至少包含一个浮点向量属性，该属性作为数据向量。您需要指定以下属性：

- `knn_type`：必需设置；当前仅支持 `hnsw`。

- `knn_dims`：必需设置，指定被索引向量的维数。

- ```
  hnsw_similarity
  ```

  ：必需设置，指定 HNSW 索引使用的距离函数。可接受的值为：

  - `L2` - L2 平方距离
  - `IP` - 内积
  - `COSINE` - 余弦相似度

- `hnsw_m`：可选设置，定义图中最大出边连接数，默认值为 16。

- `hnsw_ef_construction`：可选设置，定义构建时间与准确性的权衡。

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

<!-- example knn_insert -->

### 插入向量数据

在创建表后，您需要插入向量数据，确保其维度与创建表时指定的维度一致。

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
	"index":"test_vec",
	"id":1,
	"doc": 	{ "title" : "yellow bag", "image_vector" : [0.653448,0.192478,0.017971,0.339821] }
}

POST /insert
{
	"index":"test_vec",
	"id":2,
	"doc": 	{ "title" : "white bag", "image_vector" : [-0.148894,0.748278,0.091892,-0.095406] }
}
```

<!-- response JSON -->

```json
{
	"_index":"test",
	"_id":1,
	"created":true,
	"result":"created",
	"status":201
}

{
	"_index":"test",
	"_id":2,
	"created":true,
	"result":"created",
	"status":201
}
```

<!-- end -->

<!-- example knn_search -->

### KNN 向量搜索

现在，您可以使用 SQL 或 JSON 格式中的 `knn` 子句执行 KNN 搜索。这两种接口都支持相同的基本参数，以确保无论您选择哪种格式，都能获得一致的体验：

- SQL: `select ... from <table name> where knn ( <field>, <k>, <query vector> [,<ef>] )`
- JSON:
  ```
  POST /search
  {
      "index": "<table name>",
      "knn":
      {
          "field": "<field>",
          "query_vector": [<query vector>],
          "k": <k>,
          "ef": <ef>
      }
  }
  ```

参数包括：
* `field`：这是包含向量数据的浮点向量属性的名称。
* `k`：表示要返回的文档数量，是 Hierarchical Navigable Small World (HNSW) 索引的关键参数。它指定单个 HNSW 索引应返回的文档数量。然而，实际包含在最终结果中的文档数量可能会有所不同。例如，如果系统处理的是分成磁盘块的实时表，每个块可能返回 `k` 个文档，从而导致总数超过指定的 `k`（因为累积数量将是 `num_chunks * k`）。另一方面，如果在请求 `k` 个文档后，某些文档根据特定属性被过滤掉，则最终文档计数可能少于 `k`。需要注意的是，`k` 参数不适用于 ramchunks。在 ramchunks 的上下文中，检索过程的操作方式不同，因此 `k` 参数对返回文档数量的影响不适用。
* `query_vector`：这是搜索向量。
* `ef`：可选的动态列表大小，用于搜索期间。较高的 `ef` 会导致更准确但更慢的搜索。

文档总是根据它们与搜索向量的距离进行排序。您指定的任何附加排序标准将在此主排序条件之后应用。要检索距离，可以使用内置函数 [knn_dist()](../Functions/Other_functions.md#KNN_DIST())。

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
select id, knn_dist() from test where knn ( image_vector, 5, (0.286569,-0.031816,0.066684,0.032926), 2000 );
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
	"index": "test",
	"knn":
	{
		"field": "image_vector",
		"query_vector": [0.286569,-0.031816,0.066684,0.032926],
		"k": 5,
		"ef": 2000
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

<!-- Example knn_similar_docs -->


### 通过 ID 查找相似文档

> NOTE: 通过 ID 查找相似文档需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果它无法正常工作，请确保已安装 Buddy。

根据特定文档的唯一 ID 查找相似文档是一项常见任务。例如，当用户查看特定项目时，Manticore Search 可以高效地识别并显示在向量空间中与其最相似的项目列表。以下是如何实现这一点：

- SQL: `select ... from <table name> where knn ( <field>, <k>, <document id> )`
- JSON:
  ```
  POST /search
  {
      "index": "<table name>",
      "knn":
      {
          "field": "<field>",
          "doc_id": <document id>,
          "k": <k>
      }
  }
  ```

参数包括：
* `field`：这是包含向量数据的浮点向量属性的名称。
* `k`：表示要返回的文档数量，是 Hierarchical Navigable Small World (HNSW) 索引的关键参数。它指定单个 HNSW 索引应返回的文档数量。然而，实际包含在最终结果中的文档数量可能会有所不同。例如，如果系统处理的是分成磁盘块的实时表，每个块可能返回 `k` 个文档，从而导致总数超过指定的 `k`（因为累积数量将是 `num_chunks * k`）。另一方面，如果在请求 `k` 个文档后，某些文档根据特定属性被过滤掉，则最终文档计数可能少于 `k`。需要注意的是，`k` 参数不适用于 ramchunks。在 ramchunks 的上下文中，检索过程的操作方式不同，因此 `k` 参数对返回文档数量的影响不适用。
* `document id`：用于 KNN 相似性搜索的文档 ID。


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
  "index": "test",
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

Manticore 还支持对 KNN 搜索返回的文档进行额外的过滤，可以通过全字匹配、属性过滤或两者结合来实现。

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
	"index": "test",
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
