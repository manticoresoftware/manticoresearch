# K近邻向量搜索

Manticore Search支持为每个文档添加由您的机器学习模型生成的嵌入，然后对其进行近邻搜索。这样您可以构建如相似度搜索、推荐系统、语义搜索以及基于NLP算法的相关性排名等功能，此外还包括图像、视频和声音搜索。

## 什么是嵌入？

嵌入是一种将数据（如文本、图像或声音）表示为高维空间中的向量的方法。这些向量的设计确保它们之间的距离反映所代表数据的相似度。这个过程通常采用如词嵌入（例如Word2Vec、BERT）用于文本，或神经网络用于图像。向量空间的高维度特性（每个向量有多个组成部分）使得能够表达项目之间复杂且细致的关系。它们的相似度通过这些向量之间的距离来衡量，常用方法有欧几里得距离或余弦相似度。

Manticore Search使用HNSW库实现了k近邻（KNN）向量搜索。该功能是[Manticore Columnar Library](https://github.com/manticoresoftware/columnar)的一部分。

<!-- example KNN -->

### 配置表以进行KNN搜索

要运行KNN搜索，您必须先配置您的表。该表必须至少包含一个float_vector属性，作为数据向量。您需要指定以下属性：
* `knn_type`：必填设置；目前仅支持`hnsw`。
* `knn_dims`：必填设置，指定被索引向量的维度。
* `hnsw_similarity`：必填设置，指定HNSW索引使用的距离函数。可接受的值有：
  - `L2` - 平方L2距离
  - `IP` - 内积
  - `COSINE` - 余弦相似度
* `hnsw_m`：可选设置，定义图中最大出边连接数。默认值为16。
* `hnsw_ef_construction`：可选设置，定义构造时间与准确度的权衡。

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

创建表后，需要插入您的向量数据，确保其维度与创建表时指定的维度一致。

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

### KNN向量搜索

现在，您可以使用SQL或JSON格式的`knn`子句执行KNN搜索。两种接口支持相同的基本参数，确保您无论选择哪种格式都能获得一致的体验：

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

参数说明：
* `field`：包含向量数据的float vector属性名称。
* `k`：表示返回的文档数量，是分层可导航小世界（HNSW）索引的关键参数。它指定单个HNSW索引应返回的文档数。然而，最终结果中包含的文档数可能不同。例如，如果系统处理的是划分为磁盘块的实时表，每个块可能返回`k`个文档，导致总数超过指定的`k`（因为累计数量是`num_chunks * k`）。另一方面，如果在请求`k`个文档后，部分文档由于特定属性被过滤，最终文档数可能少于`k`。需要注意的是，参数`k`不适用于ramchunks。在ramchunks中，检索过程不同，因此`k`参数对返回文档数量的影响不适用。
* `query_vector`：搜索使用的向量。
* `ef`：可选参数，搜索时使用的动态列表大小。`ef`越高，搜索越准确但速度越慢。

文档总是按其与搜索向量的距离排序。您指定的额外排序条件将在这个主要排序条件之后应用。若需获取距离，可使用内置函数[knn_dist()](../Functions/Other_functions.md#KNN_DIST%28%29)。

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


### 通过id查找相似文档

> 注意：通过id查找相似文档需要[Manticore Buddy](../Installation/Manticore_Buddy.md)。若功能不可用，请确保已安装Buddy。

根据文档唯一ID查找相似文档是一项常见任务。例如，当用户查看某个具体条目时，Manticore Search可以高效地识别并展示向量空间中与该条目最相似的项目列表。操作方式如下：

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

参数说明：
* `field`：包含向量数据的float vector属性名称。
* `k`：表示返回的文档数量，是层次可导航小世界（HNSW）索引的关键参数。它指定单个HNSW索引应返回的文档数量。但最终结果中包含的文档数量可能会有所不同。例如，如果系统处理的是分割成磁盘块的实时表，每个块可能返回 `k` 个文档，导致总数超过指定的 `k`（因为累计计数将是 `num_chunks * k`）。另一方面，如果在请求了 `k` 个文档后，根据特定属性过滤掉一些文档，最终的文档数量可能少于 `k`。需要注意的是，参数 `k` 不适用于 ramchunks。在 ramchunks 的上下文中，检索过程的操作方式不同，因此 `k` 参数对返回文档数量的影响不适用。
* `document id`：用于KNN相似度搜索的文档ID。


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

Manticore 还支持对 KNN 搜索返回的文档进行额外过滤，既可以通过全文匹配，也可以通过属性过滤，或者两者结合。

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

