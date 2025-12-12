# K-近邻向量搜索

Manticore Search 支持为每个文档添加由机器学习模型生成的嵌入向量，然后基于这些向量进行近邻搜索。这样可以构建类似相似度搜索、推荐、语义搜索以及基于自然语言处理算法的相关性排序等功能，还包括图像、视频和声音搜索。

## 什么是嵌入向量？

嵌入向量是一种表示数据（例如文本、图像或声音）的方法，将其表示为高维空间中的向量。这些向量被设计为使得它们之间的距离能够反映所代表数据的相似度。此过程通常使用诸如词嵌入（例如 Word2Vec、BERT）用于文本，或用于图像的神经网络等算法。向量空间的高维性质，每个向量包含多个分量，允许表示项目之间复杂且细微的关系。它们的相似度通过向量之间的距离来衡量，常用的度量方法包括欧氏距离或余弦相似度。

Manticore Search 使用 HNSW 库支持 k-近邻（KNN）向量搜索。该功能是 [Manticore 列存库](https://github.com/manticoresoftware/columnar) 的一部分。

<!-- example KNN -->

### 配置用于 KNN 搜索的表

要进行 KNN 搜索，必须先配置表。浮点向量和 KNN 搜索仅支持实时表（不支持普通表）。表需要至少包含一个 [float_vector](../Creating_a_table/Data_types.md#Float-vector) 属性，作为数据向量。需要指定以下属性：
* `knn_type`：必填设置；目前只支持 `hnsw`。
* `knn_dims`：必填设置，指定被索引向量的维度。
* `hnsw_similarity`：必填设置，指定 HNSW 索引所用的距离函数。可接受的值为：
  - `L2` - 平方 L2
  - `IP` - 内积
  - `COSINE` - 余弦相似度
  
  **注意：** 使用 `COSINE` 相似度时，向量在插入时会自动归一化。这意味着存储的向量值可能与原始输入值不同，因为它们会被转换为单位向量（数学长度/幅度为 1.0 的向量），以便高效计算余弦相似度。归一化过程中向量的方向被保留，而长度被标准化。
* `hnsw_m`：可选设置，定义图中最大出边连接数，默认值为 16。
* `hnsw_ef_construction`：可选设置，定义构建时的准确度/时间权衡。

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
##### 纯模式（使用配置文件）：

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

处理向量数据的最简单方式是使用**自动嵌入**功能。借助此功能，您只需创建带有 `MODEL_NAME` 和 `FROM` 参数的表，然后直接插入文本数据，Manticore 会自动为您生成嵌入。

##### 创建带自动嵌入的表

创建自动嵌入用表时，需要指定：
- `MODEL_NAME`：使用的嵌入模型
- `FROM`：用于生成嵌入的字段（为空则表示所有文本/字符串字段）
- `API_KEY`：远程模型（OpenAI、Voyage、Jina）要求必填。通过发起实际 API 请求验证 API 密钥。
- `API_URL`：可选。自定义 API 端点 URL。未指定则使用默认提供商端点（例如 OpenAI 使用 `https://api.openai.com/v1/embeddings`）。
- `API_TIMEOUT`：可选。API 请求的 HTTP 超时时间（秒），默认为 10 秒。设置为 `'0'` 时使用默认超时。该配置同时适用于建表时的验证请求和插入操作期间的嵌入生成。

**支持的嵌入模型：**
- **Sentence Transformers**：任何[适合的基于 BERT 的 Hugging Face 模型](https://huggingface.co/sentence-transformers/models)（例如 `sentence-transformers/all-MiniLM-L6-v2`）——无需 API 密钥。Manticore 在建表时会下载模型。
- **OpenAI、Voyage、Jina**：远程嵌入模型（例如 `openai/text-embedding-ada-002`，`voyage/voyage-3.5-lite`，`jina/jina-embeddings-v2-base-en`）——需要 `API_KEY='<API_KEY>'` 参数。可选指定 `API_URL='<CUSTOM_URL>'` 以使用自定义 API 端点，及 `API_TIMEOUT='<SECONDS>'` 以配置 HTTP 超时（默认 10 秒）。

关于 float_vector 属性的更多设置细节，可参见 [这里](../Creating_a_table/Data_types.md#Float-vector)。

<!-- intro -->
##### SQL:

<!-- request SQL -->

使用 sentence-transformers（无需 API Key）
```sql
CREATE TABLE products (
    title TEXT, 
    description TEXT,
    embedding_vector FLOAT_VECTOR KNN_TYPE='hnsw' HNSW_SIMILARITY='l2' 
    MODEL_NAME='sentence-transformers/all-MiniLM-L6-v2' FROM='title'
);
```

使用 OpenAI（需 API_KEY 参数）
```sql
CREATE TABLE products_openai (
    title TEXT,
    description TEXT, 
    embedding_vector FLOAT_VECTOR KNN_TYPE='hnsw' HNSW_SIMILARITY='l2'
    MODEL_NAME='openai/text-embedding-ada-002' FROM='title,description' API_KEY='...'
);
```

使用带自定义 API URL 和超时的 OpenAI（可选）
```sql
CREATE TABLE products_openai_custom (
    title TEXT,
    description TEXT, 
    embedding_vector FLOAT_VECTOR KNN_TYPE='hnsw' HNSW_SIMILARITY='l2'
    MODEL_NAME='openai/text-embedding-ada-002' FROM='title,description' 
    API_KEY='...' API_URL='https://custom-api.example.com/v1/embeddings' API_TIMEOUT='30'
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

使用自动嵌入时，**插入语句中不需指定向量字段**。嵌入会自动从 `FROM` 参数指定的文本字段生成。

<!-- intro -->
##### SQL:

<!-- request SQL -->

只插入文本数据 — 嵌入会自动生成
```sql
INSERT INTO products (title) VALUES 
('machine learning artificial intelligence'),
('banana fruit sweet yellow');
```

插入多个字段 — 如果 FROM='title,description'，这两个字段都会参与嵌入生成
```sql
INSERT INTO products_openai (title, description) VALUES
('smartphone', 'latest mobile device with advanced features'),
('laptop', 'portable computer for work and gaming');
```

插入空向量（文档将不参与向量搜索）
```sql
INSERT INTO products (title, embedding_vector) VALUES 
('no embedding item', ());
```

<!-- end -->

##### 使用自动嵌入搜索

<!-- example embeddings_search -->
搜索方式相同——输入查询文本，Manticore 会自动生成嵌入并查找相似文档：

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

<!-- intro -->
##### JSON:

<!-- request JSON -->

使用文本查询的自动嵌入
```json
POST /search
{
    "table": "products",
    "knn": {
        "field": "embedding_vector",
        "query": "machine learning",
        "k": 3
    }
}
```

使用直接向量查询
```json
POST /search
{
    "table": "products",
    "knn": {
        "field": "embedding_vector",
        "query": [0.1, 0.2, 0.3, 0.4],
        "k": 3
    }
}
```

<!-- response JSON -->

```json
{
    "took": 0,
    "timed_out": false,
    "hits": {
        "total": 2,
        "total_relation": "eq",
        "hits": [
            {
                "_id": 1,
                "_score": 1,
                "_knn_dist": 0.12345678,
                "_source": {
                    "title": "machine learning artificial intelligence"
                }
            },
            {
                "_id": 2,
                "_score": 1,
                "_knn_dist": 0.87654321,
                "_source": {
                    "title": "banana fruit sweet yellow"
                }
            }
        ]
    }
}
```

<!-- end -->

#### 手动向量插入

<!-- example manual_vector -->
或者，您可以手动插入预先计算好的向量数据，确保其与您创建表时指定的维度相匹配。您也可以插入一个空向量；这意味着该文档将被排除在向量搜索结果之外。

**重要提示：** 当使用 `hnsw_similarity='cosine'` 时，向量在插入时会自动被归一化为单位向量（数学长度/幅度为1.0的向量）。这种归一化保留了向量的方向，同时统一了其长度，这是高效余弦相似度计算所必需的。这意味着存储的值将与您原始输入的值有所不同。

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

现在，您可以使用 SQL 或 JSON 格式中的 `knn` 子句执行 KNN 搜索。两种接口支持相同的关键参数，确保无论选择哪种格式体验一致：

- SQL: `select ... from <table name> where knn ( <field>, <k>, <query vector> [,<options>] )`
- JSON:
  ```
  POST /search
  {
      "table": "<table name>",
      "knn":
      {
          "field": "<field>",
          "query": "<text or vector>",
          "k": <k>,
          "ef": <ef>,
		  "rescore": <rescore>,
		  "oversampling": <oversampling>
      }
  }
  ```

参数说明：
* `field`：这是包含向量数据的浮点向量属性名称。
* `k`：表示要返回的文档数量，是分层可导航小世界（HNSW）索引的关键参数。它指定单个 HNSW 索引应返回的文档数量。但最终结果中包含的文档数可能有所不同。例如，如果系统处理分成多个磁盘块的实时表，每个块都可能返回 `k` 个文档，总数可能超过指定的 `k`（因为总数为 `num_chunks * k`）。另一方面，如果请求 `k` 个文档后基于特定属性过滤掉了一些文档，最终文档数可能少于 `k`。需要注意的是，参数 `k` 不适用于 ramchunks。在 ramchunks 的上下文中，检索过程不同，因此 `k` 参数对返回文档数的影响不适用。
* `query`：（推荐参数）搜索查询，可以是：
  - 文本字符串：如果字段配置了自动嵌入，则会自动转换为嵌入。若字段没有自动嵌入，则会报错。
  - 向量数组：作用与 `query_vector` 相同。
* `query_vector`：（遗留参数）作为数字数组的搜索向量。仍然支持以保持向后兼容。
  **注意：** 请求中只能使用 `query` 或 `query_vector` 中的一个，不能同时使用。
* `ef`：搜索时使用的动态列表大小（可选）。`ef`越大搜索越准确但越慢。
* `rescore`：启用KNN重评分（默认禁用）。SQL中设置为 `1` 或 JSON 中设置为 `true` 以启用。KNN搜索使用量化后的向量完成（可能带有过采样）后，距离会用原始（全精度）向量重新计算，结果重新排序以提高排名准确度。
* `oversampling`：设置乘以 `k` 的因子（浮点值），执行 KNN 搜索时检索比需求更多的候选项，基于量化向量的检索默认不使用过采样。启用重评分时，这些候选将被重新评估。过采样也适用于非量化向量。由于增加了 `k`，这会影响 HNSW 指数的工作，可能会导致结果准确度出现小幅变化。

文档始终按照与搜索向量的距离排序。您指定的其他排序条件将应用于此主要排序条件之后。要检索距离，可以使用内置函数 [knn_dist()](../Functions/Other_functions.md#KNN_DIST%28%29)。

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
		"query": [0.286569,-0.031816,0.066684,0.032926],
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

HNSW 索引需要完全加载到内存中以执行 KNN 搜索，这可能导致大量内存消耗。为了减少内存使用，可以应用标量量化——一种通过将每个分量（维度）表示为有限个离散值来压缩高维向量的技术。Manticore 支持 8 位和 1 位量化，意味着每个向量分量从32位浮点数压缩到8位甚至1位，分别减少4倍或32倍内存使用。这些压缩表示还允许更快的距离计算，因为可以在单条 SIMD 指令中处理更多的向量分量。虽然标量量化引入了一些近似误差，但这通常是在搜索准确度和资源效率之间的可取权衡。为了获得更好的准确度，量化可以与重评分和过采样结合：检索的候选数超过请求数，并使用原始32位浮点向量重新计算这些候选的距离。

支持的量化类型包括：
* `8bit`：每个向量分量量化为8位。
* `1bit`：每个向量分量量化为1位。采用非对称量化，查询向量量化为4位，存储向量量化为1位。这种方法比更简单的方式提供更高精度，但带来一定的性能折衷。
* `1bitsimple`：每个向量分量被量化为1位。该方法比 `1bit` 更快，但通常精度较低。

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

### 通过id查找相似文档

> 注意：通过id查找相似文档需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果不起作用，请确保已安装Buddy。

基于唯一ID查找与特定文档相似的文档是一个常见任务。例如，当用户查看某个特定条目时，Manticore Search可以高效地识别并显示在向量空间中与其最相似的条目列表。操作方法如下：

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
* `k`：表示返回的文档数量，是分层可导航小世界图（HNSW）索引的重要参数。它指定单个HNSW索引应返回的文档数量。不过，最终结果中包含的文档实际数量可能有所不同。例如，若系统处理被划分为多个磁盘块的实时表，每个块可以返回 `k` 个文档，导致总数超过指定的 `k`（累计数为 `num_chunks * k`）。另一方面，若请求 `k` 个文档后基于特定属性过滤掉部分文档，最终文档数可能少于 `k`。需要注意的是，参数 `k` 不适用于ramchunks。在ramchunks的上下文中，检索过程不同，因此 `k` 参数对返回文档数的影响不适用。
* `document id`：用于KNN相似性搜索的文档ID。


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

### 筛选KNN向量搜索结果

Manticore还支持对KNN搜索返回的文档进行额外筛选，可通过全文匹配、属性过滤，或两者结合实现。

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
		"query": [0.286569,-0.031816,0.066684,0.032926],
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

