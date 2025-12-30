# K近邻向量搜索

Manticore Search 支持将机器学习模型生成的嵌入向量添加到每个文档中，然后对它们进行最近邻搜索。这使您可以基于NLP算法构建相似性搜索、推荐、语义搜索和相关性排序等功能，包括图像、视频和声音搜索等。

## 什么是嵌入？

嵌入是一种表示数据的方法 - 例如文本、图像或声音 - 作为高维空间中的向量。这些向量的构造是为了确保它们之间的距离反映了它们所代表的数据的相似性。此过程通常使用诸如词嵌入（例如Word2Vec、BERT）用于文本或神经网络用于图像的算法。向量空间的高维特性，每个向量有多个分量，允许表示项目之间复杂而细致的关系。它们的相似性通过这些向量之间的距离来衡量，通常使用欧几里得距离或余弦相似度等方法。

Manticore Search 使用 HNSW 库启用 k-最近邻（KNN）向量搜索。此功能是 [Manticore 列式库](https://github.com/manticoresoftware/columnar) 的一部分。

<!-- example KNN -->

### 为 KNN 搜索配置表

要运行 KNN 搜索，您必须首先配置您的表。浮点向量和 KNN 搜索仅在实时表（而不是普通表）中受支持。表需要至少一个 [float_vector](../Creating_a_table/Data_types.md#Float-vector) 属性，作为数据向量。您需要指定以下属性：
* `knn_type`：必填设置；目前仅支持 `hnsw`。
* `knn_dims`：必填设置，指定要索引的向量的维度。
* `hnsw_similarity`：必填设置，指定 HNSW 索引使用的距离函数。可接受的值为：
  - `L2` - 平方 L2
  - `IP` - 点积
  - `COSINE` - 余弦相似度
  
  **注意：** 使用 `COSINE` 相似度时，插入时向量会自动归一化。这意味着存储的向量值可能与原始输入值不同，因为它们将被转换为单位向量（具有数学长度/幅度为 1.0 的向量），以实现高效的余弦相似度计算。这种归一化保留了向量的方向，同时标准化了其长度。
* `hnsw_m`：可选设置，定义图中出连接的最大数量。默认值为 16。
* `hnsw_ef_construction`：可选设置，定义构建时间/准确性权衡。默认值为 200。

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

使用向量数据最简单的方法是使用 **自动嵌入**。使用此功能时，您创建一个带有 `MODEL_NAME` 和 `FROM` 参数的表，然后只需插入您的文本数据 - Manticore 会为您自动生成嵌入。

##### 创建带有自动嵌入的表

创建自动嵌入表时，请指定：
- `MODEL_NAME`：使用的嵌入模型
- `FROM`：用于生成嵌入的字段（留空表示所有文本/字符串字段）

**支持的嵌入模型：**
- **Sentence Transformers**：任何 [合适的基于 BERT 的 Hugging Face 模型](https://huggingface.co/sentence-transformers/models)（例如，`sentence-transformers/all-MiniLM-L6-v2`）——不需要 API 密钥。Manticore 在您创建表时会下载该模型。
- **OpenAI**：OpenAI 嵌入模型，如 `openai/text-embedding-ada-002` - 需要 `API_KEY='<OPENAI_API_KEY>'` 参数
- **Voyage**：Voyage AI 嵌入模型 - 需要 `API_KEY='<VOYAGE_API_KEY>'` 参数
- **Jina**：Jina AI 嵌入模型 - 需要 `API_KEY='<JINA_API_KEY>'` 参数

有关设置 `float_vector` 属性的更多信息，请参见 [此处](../Creating_a_table/Data_types.md#Float-vector)。

<!-- intro -->
##### SQL：

<!-- request SQL -->

使用 sentence-transformers（不需要 API 密钥）
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

使用所有文本字段进行嵌入（FROM 为空）
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

使用自动嵌入时，**在 INSERT 语句中不要指定向量字段**。嵌入将根据 `FROM` 参数中指定的文本字段自动生成。

<!-- intro -->
##### SQL：

<!-- request SQL -->

仅插入文本数据 - 自动生成嵌入
```sql
INSERT INTO products (title) VALUES 
('machine learning artificial intelligence'),
('banana fruit sweet yellow');
```

插入多个字段 - 如果 FROM='title,description'，两者都用于嵌入  
```sql
INSERT INTO products_openai (title, description) VALUES
('smartphone', 'latest mobile device with advanced features'),
('laptop', 'portable computer for work and gaming');
```

插入空向量（文档从向量搜索中排除）
```sql
INSERT INTO products (title, embedding_vector) VALUES 
('no embedding item', ());
```

<!-- end -->

##### 使用自动嵌入进行搜索

<!-- example embeddings_search -->
搜索方式相同 - 提供查询文本，Manticore 会生成嵌入并查找相似文档：

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

使用文本查询和自动嵌入
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

#### 手动向量插入

<!-- example manual_vector -->
或者，您可以手动插入预计算的向量数据，确保其与创建表时指定的维度匹配。您也可以插入空向量；这意味着文档将从向量搜索结果中排除。

**重要：** 当使用 `hnsw_similarity='cosine'` 时，向量在插入时会自动归一化为单位向量（数学长度/大小为 1.0 的向量）。这种归一化保留了向量的方向，同时标准化了其长度，这是进行高效余弦相似度计算所必需的。这意味着存储的值将与您原始输入的值不同。

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

现在，您可以使用 SQL 或 JSON 格式中的 `knn` 子句执行 KNN 搜索。两个接口支持相同的基本参数，确保无论您选择哪种格式，都能获得一致的体验：

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

参数如下：
* `field`：这是包含向量数据的浮点向量属性的名称。
* `k`：这表示要返回的文档数量，是层次可导航小世界（HNSW）索引的关键参数。它指定单个 HNSW 索引应返回的文档数量。然而，最终结果中包含的实际文档数量可能会有所不同。例如，如果系统处理的是分为磁盘块的实时表，每个块可能返回 `k` 个文档，从而导致总数超过指定的 `k`（因为累积计数将是 `num_chunks * k`）。另一方面，如果在请求 `k` 个文档后，基于特定属性过滤掉了一些文档，则最终文档计数可能少于 `k`。需要注意的是，参数 `k` 不适用于 ramchunks。在 ramchunks 的上下文中，检索过程的操作方式不同，因此，`k` 参数对返回文档数量的影响不适用。
* `query`：（推荐参数）搜索查询，可以是：
  - 文本字符串：如果字段配置了自动嵌入，将自动转换为嵌入。如果字段没有自动嵌入，将返回错误。
  - 向量数组：与 `query_vector` 的工作方式相同。
* `query_vector`：（遗留参数）作为数字数组的搜索向量。为了向后兼容，仍然支持。
  **注意：** 使用 `query` 或 `query_vector`，而不是在同一请求中同时使用两者。
* `ef`：搜索过程中使用的动态列表的可选大小。更高的 `ef` 会导致更准确但更慢的搜索。默认值为 10。
* `rescore`：启用 KNN 重新评分（默认禁用）。在 SQL 中设置为 `1` 或在 JSON 中设置为 `true` 以启用重新评分。在使用量化向量（可能存在过采样）完成 KNN 搜索后，使用原始（全精度）向量重新计算距离，并重新排序结果以提高排名准确性。
* `oversampling`：设置一个因子（浮点值），在执行 KNN 搜索时将 `k` 乘以该因子，导致检索到的候选者数量超过所需的量，使用量化向量。默认情况下不应用过采样。如果启用重新评分，这些候选者可以稍后重新评估。过采样也适用于非量化向量。由于它增加了 `k`，这会影响 HNSW 索引的工作方式，因此可能会导致结果准确性的小幅变化。

文档始终按与搜索向量的距离进行排序。您指定的任何其他排序标准将在此主要排序条件之后应用。要检索距离，有一个内置函数 [knn_dist()](../Functions/Other_functions.md#KNN_DIST%28%29)。

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

HNSW 索引需要完全加载到内存中才能执行 KNN 搜索，这可能导致显著的内存消耗。为了减少内存使用，可以应用标量量化——一种通过用有限数量的离散值表示每个组件（维度）来压缩高维向量的技术。Manticore 支持 8 位和 1 位量化，这意味着每个向量组件从 32 位浮点压缩到 8 位甚至 1 位，分别减少内存使用 4 倍或 32 倍。这些压缩表示还允许更快的距离计算，因为可以在单个 SIMD 指令中处理更多的向量组件。尽管标量量化引入了一些近似误差，但在搜索准确性和资源效率之间通常是值得的权衡。为了获得更好的准确性，量化可以与重新评分和过采样结合使用：检索到的候选者数量超过请求的数量，并且这些候选者的距离使用原始 32 位浮点向量重新计算。

支持的量化类型包括：
* `8bit`：每个向量组件量化为 8 位。
* `1bit`：每个向量组件量化为 1 位。使用不对称量化，查询向量量化为 4 位，存储向量量化为 1 位。这种方法提供了比更简单的方法更高的精度，但在性能上有一些权衡。
* `1bitsimple`：每个向量组件量化为 1 位。此方法比 `1bit` 更快，但通常准确性较低。

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

### 通过 ID 查找相似文档

> 注意：通过 ID 查找相似文档需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果它不起作用，请确保 Buddy 已安装。

根据特定文档的唯一 ID 查找相似文档是一项常见任务。例如，当用户查看特定项目时，Manticore Search 可以有效地识别并显示与其在向量空间中最相似的项目列表。以下是您可以执行此操作的方法：

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
* `field`：这是包含向量数据的浮点向量属性的名称。
* `k`：这表示要返回的文档数量，是分层可导航小世界（HNSW）索引的关键参数。它指定了单个 HNSW 索引应返回的文档数量。但是，最终结果中包含的文档实际数量可能会有所不同。例如，如果系统处理的是划分为磁盘块的实时表，每个块可能返回 `k` 个文档，导致总数超过指定的 `k`（因为累计数量将是 `num_chunks * k`）。另一方面，如果在请求 `k` 个文档后，根据特定属性过滤掉一些文档，最终文档数量可能少于 `k`。需要注意的是，参数 `k` 不适用于 ramchunks。在 ramchunks 的上下文中，检索过程运作方式不同，因此 `k` 参数对返回文档数量的影响不适用。
* `document id`：KNN 相似性搜索的文档 ID。


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

Manticore 还支持通过全文匹配、属性过滤或两者结合来进一步过滤 KNN 搜索返回的文档。

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

