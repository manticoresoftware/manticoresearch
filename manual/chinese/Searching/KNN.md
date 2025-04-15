# K-近邻向量搜索

Manticore Search 支持将您机器学习模型生成的嵌入添加到每个文档中，然后在这些文档上执行最近邻搜索。这使您能够构建诸如相似性搜索、推荐、语义搜索和基于 NLP 算法的相关性排序等功能，此外还包括图像、视频和声音搜索。

## 什么是嵌入？

嵌入是一种以向量形式在高维空间中表示数据（例如文本、图像或声音）的方法。这些向量经过精心设计，以确保它们之间的距离反映了所代表数据的相似性。这个过程通常采用类似词嵌入（例如，Word2Vec、BERT）用于文本或神经网络用于图像的算法。高维向量空间的特性，每个向量有很多组件，允许表示项目之间复杂而细微的关系。它们的相似性通过这些向量之间的距离来衡量，通常使用欧几里得距离或余弦相似度等方法来测量。

Manticore Search 使用 HNSW 库启用 K-近邻（KNN）向量搜索。此功能是 [Manticore Columnar Library](https://github.com/manticoresoftware/columnar) 的一部分。

<!-- example KNN -->

### 配置 KNN 搜索的表

要运行 KNN 搜索，您必须首先配置表。表中必须具有至少一个 float_vector 属性，该属性作为数据向量。您需要指定以下属性：
* `knn_type`：必填设置；目前仅支持 `hnsw`。
* `knn_dims`：必填设置，指定索引向量的维度。
* `hnsw_similarity`：必填设置，指定 HNSW 索引使用的距离函数。可接受的值有：
  - `L2` - 平方 L2
  - `IP` - 内积
  - `COSINE` - 余弦相似度
* `hnsw_m`：可选设置，定义图中最大出连接数。默认值为 16。
* `hnsw_ef_construction`：可选设置，定义构造时间/精度权衡。

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

创建表后，您需要插入向量数据，确保其与创建表时指定的维度匹配。

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

现在，您可以使用 SQL 或 JSON 格式中的 `knn` 子句执行 KNN 搜索。两种接口支持相同的基本参数，确保无论您选择哪种格式，体验的一致性：

- SQL: `select ... from <table name> where knn ( <field>, <k>, <query vector> [,<ef>] )`
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
          "ef": <ef>
      }
  }
  ```

参数为：
* `field`：这是包含向量数据的 float 向量属性的名称。
* `k`：这是要返回的文档数量，是 HNSW 索引的一个关键参数。它指定单个 HNSW 索引应返回的文档数量。然而，最终结果中包含的文档数量可能会有所不同。例如，如果系统处理的实时表按磁盘块划分，每个块可能返回 `k` 个文档，从而导致总数超过指定的 `k`（因为累积计数将是 `num_chunks * k`）。另一方面，如果在请求 `k` 个文档后，有些基于特定属性被过滤掉，则最终文档计数可能会少于 `k`。需要注意的是，参数 `k` 不适用于 ramchunks。在 ramchunks 的上下文中，检索过程的工作方式不同，因此，`k` 参数对返回的文档数量的影响不适用。
* `query_vector`：这是搜索向量。
* `ef`：可选的动态列表大小，用于搜索。较高的 `ef` 会导致更准确但更慢的搜索。

文档始终按其与搜索向量的距离进行排序。您指定的任何其他排序标准将在此主排序条件之后应用。要检索距离，有一个内置函数叫做 [knn_dist()](../Functions/Other_functions.md#KNN_DIST%28%29)。

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
2 行在集合中 (0.00 秒)
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
					"title":"黄色袋",
					"image_vector":[0.653448,0.192478,0.017971,0.339821]
				}
			},
			{
				"_id": 2,
				"_score":1,
				"_knn_dist":0.81527930,
				"_source":
				{
					"title":"白色袋",
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

> 注意：通过 ID 查找相似文档需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果无法工作，请确保已安装 Buddy。

根据特定文档的唯一 ID 查找相似文档是一项常见任务。例如，当用户查看特定项目时，Manticore Search 可以有效识别并显示与其在向量空间中最相似的项目列表。以下是如何做到这一点：

- SQL: `select ... from <table name> where knn ( <field>, <k>, <document id> )`
- JSON:
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

参数如下：
* `field`: 这是包含向量数据的浮点向量属性的名称。
* `k`: 这是表示要返回的文档数量的关键参数，适用于层次可导航的小世界 (HNSW) 索引。它指定单个 HNSW 索引应返回的文档数量。然而，最终结果中包含的实际文档数量可能会有所不同。例如，如果系统处理的是分成磁盘块的实时表格，则每个块可能返回 `k` 文档，导致总量超过指定的 `k`（因为累计计数将为 `num_chunks * k`）。另一方面，如果在请求 `k` 文档后，根据特定属性筛选掉一些文档，最终文档计数可能会少于 `k`。重要的是要注意，参数 `k` 不适用于 ramchunks。在 ramchunks 的上下文中，检索过程的操作方式不同，因此，`k` 参数对返回文档数量的影响不适用。
* `document id`: KNN 相似性搜索的文档 ID。


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
1 行在集合中 (0.00 秒)
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
					"title":"白色袋",
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

Manticore 还支持通过 KNN 搜索返回的文档进行额外过滤，既可以通过全文匹配，也可以通过属性过滤，或二者结合。

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
1 行在集合中 (0.00 秒)
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
					{ "match": {"_all":"白色"} },
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
					"title":"白色袋",
					"image_vector":[-0.148894,0.748278,0.091892,-0.095406]
				}
			}
		]
	}
}
```

<!-- end -->

<!-- proofread -->
