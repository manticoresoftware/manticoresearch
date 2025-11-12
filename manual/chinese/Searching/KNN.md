# K近邻向量搜索

Manticore Search 支持向每个文档添加由机器学习模型生成的嵌入向量，然后对它们进行最近邻搜索。这使您可以构建类似度搜索、推荐、语义搜索和基于NLP算法的相关性排名等功能，还包括图像、视频和声音搜索。

## 什么是嵌入？

嵌入是一种表示数据的方法——例如文本、图像或声音——将其表示为高维空间中的向量。这些向量设计成使它们之间的距离反映出所代表数据的相似性。该过程通常采用诸如词嵌入（例如 Word2Vec、BERT）算法来处理文本，或使用神经网络处理图像。向量空间的高维特性，每个向量具有多个分量，能够表现项目之间复杂而微妙的关系。它们的相似度通过向量之间的距离来衡量，通常使用欧氏距离或余弦相似度等方法。

Manticore Search 使用 HNSW 库支持 k近邻（KNN）向量搜索。此功能是 [Manticore Columnar Library](https://github.com/manticoresoftware/columnar) 的一部分。

<!-- example KNN -->

### 配置用于KNN搜索的表

要运行KNN搜索，必须首先配置表。浮点向量和KNN搜索仅支持实时表（不支持普通表）。表需要至少包含一个 [float_vector](../Creating_a_table/Data_types.md#Float-vector) 属性，该属性作为数据向量。您需指定以下属性：
* `knn_type`：必填设置；当前仅支持 `hnsw`。
* `knn_dims`：必填设置，指定索引向量的维数。
* `hnsw_similarity`：必填设置，指定HNSW索引使用的距离函数。可接受的值：
  - `L2` - 平方L2距离
  - `IP` - 内积
  - `COSINE` - 余弦相似度
  
  **注意：** 当使用 `COSINE` 相似度时，插入时向量会自动归一化。这意味着存储的向量值可能与原始输入不同，因为它们会被转换为单位向量（数学长度/幅度为1.0的向量），以实现高效的余弦相似度计算。归一化保持向量方向不变，同时标准化其长度。
* `hnsw_m`：可选设置，定义图中最多的出边连接数，默认值为16。
* `hnsw_ef_construction`：可选设置，定义构建阶段的时间与精度折衷。

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

处理向量数据最简单的方式是使用**自动嵌入**。通过此功能，您创建一个包含 `MODEL_NAME` 和 `FROM` 参数的表，然后只需插入文本数据—Manticore 会自动为您生成嵌入。

##### 创建包含自动嵌入的表

创建自动嵌入表时，请指定：
- `MODEL_NAME`：要使用的嵌入模型
- `FROM`：用于生成嵌入的字段（空表示所有文本/字符串字段）

**支持的嵌入模型：**
- **Sentence Transformers**：任何[合适的基于BERT的Hugging Face模型](https://huggingface.co/sentence-transformers/models)（例如 `sentence-transformers/all-MiniLM-L6-v2`）—无需API密钥。创建表时，Manticore会下载该模型。
- **OpenAI**：OpenAI嵌入模型，如 `openai/text-embedding-ada-002` - 需要 `API_KEY='<OPENAI_API_KEY>'` 参数
- **Voyage**：Voyage AI 嵌入模型 - 需要 `API_KEY='<VOYAGE_API_KEY>'` 参数
- **Jina**：Jina AI 嵌入模型 - 需要 `API_KEY='<JINA_API_KEY>'` 参数

更多关于设置 `float_vector` 属性的信息可以见[这里](../Creating_a_table/Data_types.md#Float-vector)。

<!-- intro -->
##### SQL：

<!-- request SQL -->

使用 sentence-transformers（无须API密钥）
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

使用所有文本字段进行嵌入（FROM为空）
```sql
CREATE TABLE products_all (
    title TEXT,
    description TEXT,
    embedding_vector FLOAT_VECTOR KNN_TYPE='hnsw' HNSW_SIMILARITY='l2'
    MODEL_NAME='sentence-transformers/all-MiniLM-L6-v2' FROM=''
);
```

<!-- intro -->
##### JSON：

<!-- request JSON -->

使用 sentence-transformers（无须API密钥）
```json
POST /sql?mode=raw -d "CREATE TABLE products ( title TEXT, description TEXT, embedding_vector FLOAT_VECTOR KNN_TYPE='hnsw' HNSW_SIMILARITY='l2' MODEL_NAME='sentence-transformers/all-MiniLM-L6-v2' FROM='title');"
```

使用 OpenAI（需要 API_KEY 参数）
```json
POST /sql?mode=raw -d "CREATE TABLE products_openai ( title TEXT, description TEXT, embedding_vector FLOAT_VECTOR KNN_TYPE='hnsw' HNSW_SIMILARITY='l2' MODEL_NAME='openai/text-embedding-ada-002' FROM='title,description' API_KEY='...');"
```

使用所有文本字段进行嵌入（FROM为空）
```json
POST /sql?mode=raw -d "CREATE TABLE products_all ( title TEXT, description TEXT, embedding_vector FLOAT_VECTOR KNN_TYPE='hnsw' HNSW_SIMILARITY='l2' MODEL_NAME='sentence-transformers/all-MiniLM-L6-v2' FROM='');"
```

<!-- end -->

##### 使用自动嵌入插入数据

<!-- example inserting_embeddings -->

使用自动嵌入时，**不要在INSERT语句中指定向量字段**。嵌入会自动从`FROM`参数指定的文本字段生成。

<!-- intro -->
##### SQL：

<!-- request SQL -->

仅插入文本数据——嵌入自动生成
```sql
INSERT INTO products (title) VALUES 
('machine learning artificial intelligence'),
('banana fruit sweet yellow');
```

插入多个字段 — 如果FROM='title,description'，则两者都会用于嵌入生成
```sql
INSERT INTO products_openai (title, description) VALUES
('smartphone', 'latest mobile device with advanced features'),
('laptop', 'portable computer for work and gaming');
```

插入空向量（文档被排除在向量搜索之外）
```sql
INSERT INTO products (title, embedding_vector) VALUES 
('no embedding item', ());
```

<!-- intro -->
##### JSON：

<!-- request JSON -->

仅插入文本数据——嵌入自动生成
```JSON
POST /sql?mode=raw -d "INSERT INTO products (title) VALUES ('machine learning artificial intelligence'),('banana fruit sweet yellow');"
```

插入多个字段 — 如果FROM='title,description'，则两者都会用于嵌入生成
```JSON
POST /sql?mode=raw -d "INSERT INTO products_openai (title, description) VALUES ('smartphone', 'latest mobile device with advanced features'), ('laptop', 'portable computer for work and gaming');"
```

插入空向量（文档被排除在向量搜索之外）
```JSON
POST /sql?mode=raw -d "INSERT INTO products (title, embedding_vector) VALUES ('no embedding item', ());"
```

<!-- end -->

##### 使用自动嵌入进行搜索

<!-- example embeddings_search -->
搜索方式相同—提供查询文本，Manticore 会生成嵌入并找到类似文档：

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

使用文本查询与自动嵌入
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

#### Manual Vector Insertion

<!-- example manual_vector -->
Alternatively, you can manually insert pre-computed vector data, ensuring it matches the dimensions you specified when creating the table. You can also insert an empty vector; this means that the document will be excluded from vector search results.

**Important:** When using `hnsw_similarity='cosine'`, vectors are automatically normalized upon insertion to unit vectors (vectors with a mathematical length/magnitude of 1.0). This normalization preserves the direction of the vector while standardizing its length, which is required for efficient cosine similarity calculations. This means the stored values will differ from your original input values.

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

### KNN vector search

Now, you can perform a KNN search using the `knn` clause in either SQL or JSON format. Both interfaces support the same essential parameters, ensuring a consistent experience regardless of the format you choose:

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

The parameters are:
* `field`: This is the name of the float vector attribute containing vector data.
* `k`: This represents the number of documents to return and is a key parameter for Hierarchical Navigable Small World (HNSW) indexes. It specifies the quantity of documents that a single HNSW index should return. However, the actual number of documents included in the final results may vary. For instance, if the system is dealing with real-time tables divided into disk chunks, each chunk could return `k` documents, leading to a total that exceeds the specified `k` (as the cumulative count would be `num_chunks * k`). On the other hand, the final document count might be less than `k` if, after requesting `k` documents, some are filtered out based on specific attributes. It's important to note that the parameter `k` does not apply to ramchunks. In the context of ramchunks, the retrieval process operates differently, and thus, the `k` parameter's effect on the number of documents returned is not applicable.
* `query`: (Recommended parameter) The search query, which can be either:
  - Text string: Automatically converted to embeddings if the field has auto-embeddings configured. Will return an error if the field doesn't have auto-embeddings.
  - Vector array: Works the same as `query_vector`.
* `query_vector`: (Legacy parameter) The search vector as an array of numbers. Still supported for backward compatibility.
  **Note:** Use either `query` or `query_vector`, not both in the same request.
* `ef`: optional size of the dynamic list used during the search. A higher `ef` leads to more accurate but slower search.
* `rescore`: Enables KNN rescoring (disabled by default). Set to `1` in SQL or `true` in JSON to enable rescoring. After the KNN search is completed using quantized vectors (with possible oversampling), distances are recalculated with the original (full-precision) vectors and results are re-sorted to improve ranking accuracy.
* `oversampling`: Sets a factor (float value) by which `k` is multiplied when executing the KNN search, causing more candidates to be retrieved than needed using quantized vectors. No oversampling is applied by default. These candidates can be re-evaluated later if rescoring is enabled. Oversampling also works with non-quantized vectors. Since it increases `k`, which affects how the HNSW index works, it may cause a small change in result accuracy.

Documents are always sorted by their distance to the search vector. Any additional sorting criteria you specify will be applied after this primary sort condition. For retrieving the distance, there is a built-in function called [knn_dist()](../Functions/Other_functions.md#KNN_DIST%28%29).

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

### Vector quantization

HNSW indexes need to be fully loaded into memory to perform KNN search, which can lead to significant memory consumption. To reduce memory usage, scalar quantization can be applied - a technique that compresses high-dimensional vectors by representing each component (dimension) with a limited number of discrete values. Manticore supports 8-bit and 1-bit quantization, meaning each vector component is compressed from a 32-bit float to 8 bits or even 1 bit, reducing memory usage by 4x or 32x, respectively. These compressed representations also allow for faster distance calculations, as more vector components can be processed in a single SIMD instruction. Although scalar quantization introduces some approximation error, it is often a worthwhile trade-off between search accuracy and resource efficiency. For even better accuracy, quantization can be combined with rescoring and oversampling: more candidates are retrieved than requested, and distances for these candidates are recalculated using the original 32-bit float vectors.

Supported quantization types include:
* `8bit`: Each vector component is quantized to 8 bits.
* `1bit`: Each vector component is quantized to 1 bit. Asymmetric quantization is used, with query vectors quantized to 4 bits and stored vectors to 1 bit. This approach offers greater precision than simpler methods, though with some performance trade-off.
* `1bitsimple`: Each vector component is quantized to 1 bit. This method is faster than `1bit`, but typically less accurate.

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

> 注意：通过 id 查找相似文档需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果无法使用，请确保已安装 Buddy。

根据唯一 ID 查找类似文档是一项常见任务。例如，当用户查看某个特定条目时，Manticore Search 可以高效地识别并显示与该条目在向量空间中最相似的项目列表。操作方式如下：

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
* `field`：这是包含向量数据的浮点向量属性名称。
* `k`：代表返回的文档数量，是层次导航小世界（HNSW）索引的一个关键参数。它指定单个 HNSW 索引应返回的文档数量。但最终结果中包含的文档数量可能会有所不同。例如，当系统处理分为磁盘块的实时表时，每个块都可能返回 `k` 个文档，导致总数超过指定的 `k`（因为总计为 `num_chunks * k`）。另一方面，如果请求了 `k` 个文档后，部分文档被基于特定属性过滤掉，则最终文档数可能少于 `k`。需要注意的是，参数 `k` 不适用于 ramchunks。在 ramchunks 的上下文中，检索过程不同，因此 `k` 参数对返回文档数量的影响不适用。
* `document id`：用于 KNN 相似搜索的文档 ID。


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

Manticore 还支持对 KNN 搜索返回的文档进行额外过滤，过滤方式可以是全文匹配、属性过滤，或两者结合。

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

