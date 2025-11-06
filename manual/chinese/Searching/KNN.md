# K近邻向量搜索

Manticore Search 支持向每个文档添加由机器学习模型生成的嵌入向量，然后对其进行最近邻搜索。这样可以构建类似搜索、推荐、语义搜索和基于NLP算法的相关性排序等功能，还包括图像、视频和声音搜索。

## 什么是嵌入？

嵌入是一种将数据（如文本、图像或声音）表示为高维空间中的向量的方法。这些向量经过精心设计，使得它们之间的距离反映了所代表数据的相似性。此过程通常采用算法，如文本的词嵌入（例如，Word2Vec、BERT）或图像的神经网络。向量空间的高维特性，每个向量有多个分量，允许表示项目之间复杂且细致的关系。它们的相似性通过向量之间的距离来衡量，通常使用欧氏距离或余弦相似度等方法。

Manticore Search 利用 HNSW 库实现了 k 近邻（KNN）向量搜索。此功能是 [Manticore Columnar Library](https://github.com/manticoresoftware/columnar) 的一部分。

<!-- example KNN -->

### 配置KNN搜索表

要执行 KNN 搜索，必须先配置你的表。浮点向量和 KNN 搜索仅支持实时表（不支持普通表）。表需包含至少一个作为数据向量的 [float_vector](../Creating_a_table/Data_types.md#Float-vector) 属性。你需要指定以下属性：
* `knn_type`：必填参数；目前仅支持 `hnsw`。
* `knn_dims`：必填参数，指定被索引向量的维度。
* `hnsw_similarity`：必填参数，指定 HNSW 索引使用的距离函数。可接受的值有：
  - `L2` - 平方L2距离
  - `IP` - 内积
  - `COSINE` - 余弦相似度
  
  **注意：**使用 `COSINE` 相似度时，插入时向量会自动归一化。这意味着存储的向量值可能与原始输入值不同，因为它们会被转换为单位向量（数学长度/模为1.0的向量），以便高效计算余弦相似度。此归一化保留向量的方向，同时标准化其长度。
* `hnsw_m`：可选参数，定义图中最大外连边数，默认值为16。
* `hnsw_ef_construction`：可选参数，定义构建时的准确度/时间折中。

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

<!-- request JSON -->
```json
POST /sql?mode=raw -d "create table test ( title text, image_vector float_vector knn_type='hnsw' knn_dims='4' hnsw_similarity='l2' );"
```

<!-- response JSON -->

```json
[
  {
    "total": 0,
    "error": "",
    "warning": ""
  }
]
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

使用向量数据最简单的方法是使用**自动嵌入**。此功能下，你创建一个带有 `MODEL_NAME` 和 `FROM` 参数的表，直接插入文本数据，Manticore 会自动为你生成嵌入向量。

##### 创建自动嵌入表

创建自动嵌入表时，指定：
- `MODEL_NAME`：使用的嵌入模型
- `FROM`：用于生成嵌入的字段（为空则代表所有文本/字符串字段）

**支持的嵌入模型：**
- **Sentence Transformers**：任何[合适的基于BERT的Hugging Face模型](https://huggingface.co/sentence-transformers/models)（例如，`sentence-transformers/all-MiniLM-L6-v2`）— 不需要API密钥。Manticore 会在创建表时下载模型。
- **OpenAI**：OpenAI的嵌入模型，如`openai/text-embedding-ada-002` - 需提供 `API_KEY='<OPENAI_API_KEY>'` 参数
- **Voyage**：Voyage AI 嵌入模型 - 需提供 `API_KEY='<VOYAGE_API_KEY>'` 参数
- **Jina**：Jina AI 嵌入模型 - 需提供 `API_KEY='<JINA_API_KEY>'` 参数

关于 `float_vector` 属性的更多信息请参见[这里](../Creating_a_table/Data_types.md#Float-vector)。

<!-- intro -->
##### SQL：

<!-- request SQL -->

使用 sentence-transformers（无需API密钥）
```sql
CREATE TABLE products (
    title TEXT, 
    description TEXT,
    embedding_vector FLOAT_VECTOR KNN_TYPE='hnsw' HNSW_SIMILARITY='l2' 
    MODEL_NAME='sentence-transformers/all-MiniLM-L6-v2' FROM='title'
);
```

使用 OpenAI（需API_KEY参数）
```sql
CREATE TABLE products_openai (
    title TEXT,
    description TEXT, 
    embedding_vector FLOAT_VECTOR KNN_TYPE='hnsw' HNSW_SIMILARITY='l2'
    MODEL_NAME='openai/text-embedding-ada-002' FROM='title,description' API_KEY='...'
);
```

使用所有文本字段生成嵌入（FROM为空）
```sql
CREATE TABLE products_all (
    title TEXT,
    description TEXT,
    embedding_vector FLOAT_VECTOR KNN_TYPE='hnsw' HNSW_SIMILARITY='l2'
    MODEL_NAME='sentence-transformers/all-MiniLM-L6-v2' FROM=''
);
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

使用 sentence-transformers （无需 API 密钥）
```json
POST /sql?mode=raw -d "CREATE TABLE products ( title TEXT, description TEXT, embedding_vector FLOAT_VECTOR KNN_TYPE='hnsw' HNSW_SIMILARITY='l2' MODEL_NAME='sentence-transformers/all-MiniLM-L6-v2' FROM='title');"
```

使用 OpenAI （需要 API_KEY 参数）
```json
POST /sql?mode=raw -d "CREATE TABLE products_openai ( title TEXT, description TEXT, embedding_vector FLOAT_VECTOR KNN_TYPE='hnsw' HNSW_SIMILARITY='l2' MODEL_NAME='openai/text-embedding-ada-002' FROM='title,description' API_KEY='...');"
```

使用所有文本字段进行嵌入（FROM 为空）
```json
POST /sql?mode=raw -d "CREATE TABLE products_all ( title TEXT, description TEXT, embedding_vector FLOAT_VECTOR KNN_TYPE='hnsw' HNSW_SIMILARITY='l2' MODEL_NAME='sentence-transformers/all-MiniLM-L6-v2' FROM='');"
```

<!-- end -->

##### 使用自动嵌入插入数据

<!-- example inserting_embeddings -->

使用自动嵌入时，**不要在INSERT语句中指定向量字段**。嵌入向量会根据 `FROM` 参数指定的文本字段自动生成。

<!-- intro -->
##### SQL：

<!-- request SQL -->

仅插入文本数据 - 嵌入自动生成
```sql
INSERT INTO products (title) VALUES 
('machine learning artificial intelligence'),
('banana fruit sweet yellow');
```

插入多字段 - 若 FROM='title,description' 则两个字段均用于嵌入生成
```sql
INSERT INTO products_openai (title, description) VALUES
('smartphone', 'latest mobile device with advanced features'),
('laptop', 'portable computer for work and gaming');
```

插入空向量（文档不会参与向量搜索）
```sql
INSERT INTO products (title, embedding_vector) VALUES 
('no embedding item', ());
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

仅插入文本数据 - 嵌入向量自动生成
```JSON
POST /sql?mode=raw -d "INSERT INTO products (title) VALUES ('machine learning artificial intelligence'),('banana fruit sweet yellow');"
```

插入多个字段 - 如果 FROM='title,description'，两个字段都用于生成嵌入向量  
```JSON
POST /sql?mode=raw -d "INSERT INTO products_openai (title, description) VALUES ('smartphone', 'latest mobile device with advanced features'), ('laptop', 'portable computer for work and gaming');"
```

插入空向量（文档将被排除在向量搜索之外）
```JSON
POST /sql?mode=raw -d "INSERT INTO products (title, embedding_vector) VALUES ('no embedding item', ());"
```

<!-- end -->

##### 使用自动嵌入搜索

<!-- example embeddings_search -->
搜索方法相同 - 提供查询文本，Manticore 会生成嵌入并查找相似文档：

<!-- intro -->
##### SQL：

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
##### JSON：

<!-- request JSON -->

使用文本查询自动嵌入
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

直接使用向量查询
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

#### 手动插入向量

<!-- example manual_vector -->
另外，你也可以手动插入预先计算好的向量数据，确保其维度与你创建表时指定的相符。你也可以插入空向量，这表示该文档将被排除在向量搜索结果之外。

**重要提示：** 当使用 `hnsw_similarity='cosine'` 时，向量在插入时会自动归一化为单位向量（数学长度/幅度为 1.0 的向量）。这种归一化保留向量的方向，同时标准化其长度，这是进行高效余弦相似度计算所必需的。这意味着存储的值将与您原始输入的值不同。

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

现在，您可以使用 SQL 或 JSON 格式中的 `knn` 子句执行 KNN 搜索。两种接口支持相同的基本参数，确保无论选择哪种格式，都能获得一致的体验：

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
* `field`：包含向量数据的浮点向量属性名称。
* `k`：表示返回文档的数量，是分层可导航小世界（HNSW）索引的关键参数。它指定单个 HNSW 索引应返回的文档数量。但最终结果中包含的文档数量可能会有所不同。例如，如果系统处理被分成磁盘块的实时表，每个块可返回 `k` 个文档，导致总数超过指定的 `k`（累计结果为 `num_chunks * k`）。另一方面，如果请求了 `k` 个文档后，有些文档基于特定属性被过滤，最终返回的文档数可能少于 `k`。请注意，参数 `k` 不适用于 ramchunks。在 ramchunks 上，检索过程不同，因此 `k` 参数对返回文档数量的影响不适用。
* `query`：（推荐参数）搜索查询，可以是：
  - 文本字符串：如果字段配置了自动嵌入，则自动转换为嵌入。如果字段没有自动嵌入，将返回错误。
  - 向量数组：与 `query_vector` 的作用相同。
* `query_vector`：（遗留参数）作为数字数组的搜索向量。为向后兼容仍然支持。
  **注意：** 请求中只使用 `query` 或 `query_vector` 其一，不要同时使用。
* `ef`：搜索时使用的动态列表大小，可选参数。更高的 `ef` 导致更准确但更慢的搜索。
* `rescore`：启用 KNN 重新评分（默认禁用）。在 SQL 中设置为 `1`，在 JSON 中设置为 `true` 可启用重新评分。KNN 搜索完成后，使用量化向量（可能已进行过过采样），用原始（全精度）向量重新计算距离，并对子结果重新排序，以提高排序准确性。
* `oversampling`：设置一个因子（浮点数），用于将执行 KNN 搜索时的 `k` 乘以该因子，从而使用量化向量检索比需求更多的候选项。默认不应用过采样。如果启用了重新评分，可以对这些候选项进行重新评估。过采样也支持非量化向量。由于它增加了 `k` 的值，影响 HNSW 索引工作方式，可能导致结果准确度略有变化。

文档始终按照与搜索向量的距离排序。您指定的任何附加排序条件会在主排序条件之后应用。若要获取距离，有内置函数 [knn_dist()](../Functions/Other_functions.md#KNN_DIST%28%29)。

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

HNSW 索引需要完全加载到内存中才能执行 KNN 搜索，这可能导致大量内存消耗。为减少内存使用，可以应用标量量化——一种通过用有限个离散值表示每个分量（维度）来压缩高维向量的技术。Manticore 支持 8 位和 1 位量化，意味着每个向量分量从 32 位浮点压缩为 8 位甚至 1 位，分别减少 4 倍或 32 倍内存使用。压缩的表示还允许更快的距离计算，因为单个 SIMD 指令能处理更多向量分量。尽管标量量化引入了一些近似误差，但通常是搜索精度与资源效率之间值得的权衡。为了获得更好的精度，可以将量化与重新评分和过采样结合：检索的候选项超过请求数量，然后用原始 32 位浮点向量重新计算这些候选项的距离。

支持的量化类型包括：
* `8bit`：每个向量分量量化为 8 位。
* `1bit`：每个向量分量量化为 1 位。采用非对称量化，查询向量量化为 4 位，存储向量量化为 1 位。这种方法比简单方法精度更高，但性能有所权衡。
* `1bitsimple`：每个向量分量量化为 1 位。这种方法比 `1bit` 更快，但通常精度较低。

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
<!-- intro -->
##### JSON:

<!-- request JSON -->
```json
POST /sql?mode=raw -d "create table test ( title text, image_vector float_vector knn_type='hnsw' knn_dims='4' hnsw_similarity='l2' quantization='1bit');"
```

<!-- response JSON -->

```json
[
  {
    "total": 0,
    "error": "",
    "warning": ""
  }
]
```

<!-- end -->

<!-- Example knn_similar_docs -->

### 通过 id 查找相似文档

> 注意：通过 id 查找相似文档需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果功能不可用，请确认 Buddy 已安装。

根据特定文档的唯一ID查找相似文档是一项常见任务。例如，当用户查看某个特定条目时，Manticore Search 可以高效地识别并显示与其在向量空间中最相似的条目列表。以下是实现方法：

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
* `field`: 这是包含向量数据的浮点向量属性的名称。
* `k`: 表示返回的文档数量，是层次可导航小世界（HNSW）索引的关键参数。它指定单个 HNSW 索引应返回的文档数量。但最终结果中包含的文档数可能会有所不同。例如，如果系统处理的是划分为磁盘块的实时表， 每个块可能返回 `k` 个文档，总计可能超过指定的 `k`（因为累积计数是 `num_chunks * k`）。另一方面，如果请求了 `k` 个文档后，基于特定属性过滤掉了一些，最终文档数量可能小于 `k`。需要注意的是，参数 `k` 不适用于内存块（ramchunks）。在内存块的上下文中，检索过程有所不同，因此 `k` 参数对返回文档数量的影响不适用。
* `document id`: 用于 KNN 相似度搜索的文档ID。


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

### 过滤KNN向量搜索结果

Manticore 还支持对 KNN 搜索返回的文档进行额外过滤，可以通过全文匹配、属性过滤，或者两者结合。

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

