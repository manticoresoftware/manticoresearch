# K近邻向量搜索

Manticore Search 支持将机器学习模型生成的嵌入向量添加到每个文档中，然后基于这些向量进行最近邻搜索。这使您能够构建诸如相似性搜索、推荐系统、语义搜索和基于自然语言处理算法的相关性排序功能，此外还包括图像、视频和声音搜索等。

## 什么是嵌入向量？

嵌入向量是一种表示数据（如文本、图像或声音）的方法，将其表示为高维空间中的向量。这些向量经过设计，使得它们之间的距离能够反映所代表数据的相似性。此过程通常采用诸如词嵌入（例如 Word2Vec、BERT）等算法用于文本，或者神经网络用于图像。向量空间的高维特性——每个向量包含多个分量——使得能够表示项目之间复杂且细致的关系。其相似性通过向量之间的距离来衡量，通常使用欧几里得距离或余弦相似度等方法。

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

#### 自动嵌入（推荐）

处理向量数据最简单的方法是使用**自动嵌入**。通过此功能，您只需带有 `MODEL_NAME` 和 `FROM` 参数创建表，然后插入文本数据——Manticore 会自动为您生成嵌入向量。

##### 创建带有自动嵌入的表

创建支持自动嵌入的表时，需要指定：
- `MODEL_NAME`：使用的嵌入模型
- `FROM`：指定用于生成嵌入的字段（为空表示使用所有文本/字符串字段）

**支持的嵌入模型：**
- **Sentence Transformers**：任意[适用的基于 BERT 的 Hugging Face 模型](https://huggingface.co/sentence-transformers/models)（例如 `sentence-transformers/all-MiniLM-L6-v2`）——无需 API 密钥。创建表时，Manticore 会自动下载模型。
- **OpenAI**：OpenAI 嵌入模型，如 `openai/text-embedding-ada-002`——需要指定 `API_KEY='<OPENAI_API_KEY>'` 参数
- **Voyage**：Voyage AI 嵌入模型——需要指定 `API_KEY='<VOYAGE_API_KEY>'` 参数
- **Jina**：Jina AI 嵌入模型——需要指定 `API_KEY='<JINA_API_KEY>'` 参数

<!-- intro -->
##### SQL:

<!-- request SQL -->

使用 sentence-transformers（无需 API 密钥）
```sql
CREATE TABLE products (
    title TEXT, 
    description TEXT,
    embedding_vector FLOAT_VECTOR KNN_TYPE='hnsw' HNSW_SIMILARITY='l2' 
    MODEL_NAME='sentence-transformers/all-MiniLM-L6-v2' FROM='title'
);
```

使用 OpenAI（需要 API_KEY 参数）
```sql
CREATE TABLE products_openai (
    title TEXT,
    description TEXT, 
    embedding_vector FLOAT_VECTOR KNN_TYPE='hnsw' HNSW_SIMILARITY='l2'
    MODEL_NAME='openai/text-embedding-ada-002' FROM='title,description' API_KEY='...'
);
```

使用所有文本字段生成嵌入（FROM 为空）
```sql
CREATE TABLE products_all (
    title TEXT,
    description TEXT,
    embedding_vector FLOAT_VECTOR KNN_TYPE='hnsw' HNSW_SIMILARITY='l2'
    MODEL_NAME='sentence-transformers/all-MiniLM-L6-v2' FROM=''
);
```

<!-- end -->

##### 使用自动嵌入插入数据

<!-- example inserting_embeddings -->

使用自动嵌入时，**不要在 INSERT 语句中指定向量字段**。嵌入向量会自动根据 `FROM` 参数中指定的文本字段生成。

<!-- intro -->
##### SQL:

<!-- request SQL -->

仅插入文本数据——嵌入自动生成
```sql
INSERT INTO products (title) VALUES 
('machine learning artificial intelligence'),
('banana fruit sweet yellow');
```

插入多个字段——若 FROM='title,description'，两者都用于生成嵌入  
```sql
INSERT INTO products_openai (title, description) VALUES
('smartphone', 'latest mobile device with advanced features'),
('laptop', 'portable computer for work and gaming');
```

插入空向量（该文档将被排除在向量搜索之外）
```sql
INSERT INTO products (title, embedding_vector) VALUES 
('no embedding item', ());
```

<!-- end -->

##### 使用自动嵌入搜索

<!-- example embeddings_search -->
搜索方法相同——提供查询文本，Manticore 会自动生成嵌入并查找相似文档：

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SELECT id, knn_dist() FROM products WHERE knn(embedding_vector, 3, 'machine learning');
```

<!-- response SQL -->

```sql
+------+------------+
| id   | knn_dist() |
+------+------------+
|    1 | 0.12345678 |
|    2 | 0.87654321 |
+------+------------+
2 rows in set (0.00 sec)
```

<!-- end -->

#### 手动向量插入

<!-- example manual_vector -->
或者，您也可以手动插入预先计算好的向量数据，确保其维度与创建表时指定的一致。您还可以插入空向量，这意味着该文档将被排除在向量搜索结果之外。

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















































































































































































































