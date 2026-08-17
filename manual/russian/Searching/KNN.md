# Поиск ближайших векторов KNN

Manticore Search поддерживает добавление эмбеддингов, сгенерированных моделями машинного обучения, к каждому документу, а затем поиск ближайших соседей по ним. Это позволяет строить такие функции, как поиск по сходству, рекомендации, семантический поиск и ранжирование релевантности на основе алгоритмов NLP, а также поиск по изображениям, видео и звуку.

Чтобы объединить векторный поиск KNN с полнотекстовым поиском для лучшей релевантности, см. [Гибридный поиск](../Searching/Hybrid_search.md).

## Что такое эмбеддинг?

Эмбеддинг - это способ представления данных, таких как текст, изображения или звук, в виде векторов в многомерном пространстве. Эти векторы строятся так, чтобы расстояние между ними отражало сходство представляемых ими данных. Обычно для этого используют такие алгоритмы, как word embeddings (например, Word2Vec, BERT) для текста или нейросети для изображений. Многомерная природа векторного пространства, где у каждого вектора много компонент, позволяет описывать сложные и тонкие связи между объектами. Их сходство оценивают по расстоянию между этими векторами, которое часто измеряют с помощью евклидова расстояния или косинусного сходства.

Manticore Search поддерживает поиск k ближайших соседей (KNN) с использованием библиотеки HNSW. Эта функциональность входит в состав [Manticore Columnar Library](https://github.com/manticoresoftware/columnar).

<!-- example KNN -->

### Настройка таблицы для поиска KNN

Чтобы выполнять поиск KNN, сначала нужно настроить таблицу. Вещественные векторы и поиск KNN поддерживаются только в таблицах real-time (не в plain-таблицах). В таблице должен быть как минимум один атрибут [float_vector](../Creating_a_table/Data_types.md#Float-vector), который служит вектором данных. Нужно указать следующие свойства:
* `knn_type`: Обязательный параметр; сейчас поддерживается только `hnsw`.
* `knn_dims`: Обязательный параметр, задающий размерность индексируемых векторов.
* `hnsw_similarity`: Обязательный параметр, задающий функцию расстояния, используемую индексом HNSW. Допустимые значения:
  - `L2` - Квадрат евклидова расстояния L2
  - `IP` - Скалярное произведение
  - `COSINE` - Косинусное сходство

  **Примечание:** При использовании сходства `COSINE` векторы автоматически нормализуются при вставке. Это означает, что сохраненные значения векторов могут отличаться от исходных входных значений, поскольку они будут преобразованы в единичные векторы (векторы с математической длиной/модулем 1.0), чтобы можно было эффективно вычислять косинусное сходство. Такая нормализация сохраняет направление вектора, одновременно стандартизируя его длину.
* `hnsw_m`: Необязательный параметр, задающий максимальное число исходящих связей в графе. Значение по умолчанию - 16.
* `hnsw_ef_construction`: Необязательный параметр, задающий компромисс между временем построения и точностью. Значение по умолчанию - 200.

> ПРИМЕЧАНИЕ: Построение графа HNSW при сохранении чанков RT, при слиянии чанков `OPTIMIZE TABLE` / auto-optimize и при перестройке KNN через `ALTER TABLE ... ADD/DROP/REBUILD` выполняется параллельно по умолчанию на многопроцессорных хостах; число worker-потоков управляется настройкой `searchd` [`knn_parallel_build`](../Server_settings/Searchd.md#knn_parallel_build) (установите `1`, чтобы принудительно использовать последовательный путь). Это влияет только на скорость построения. Поскольку параллельное построение HNSW может вставлять векторы в другом порядке, итоговый граф может не быть побитово идентичен последовательной сборке.

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
POST /sql?mode=raw -d "create table test ( title text, image_vector float_vector knn_type='hnsw' knn_dims='4' hnsw_similarity='l2' )"
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
##### Plain-режим (через файл конфигурации) - Векторные данные вручную:

<!-- request Config -->
```ini
table test_vec {
    type = rt
	...
    rt_attr_float_vector = image_vector
    knn = {"attrs":[{"name":"image_vector","type":"hnsw","dims":4,"hnsw_similarity":"L2","hnsw_m":16,"hnsw_ef_construction":200}]}
}
```

**Примечание:** Для автоэмбеддингов в plain-режиме см. пример ниже, где показано, как использовать параметры `model_name` и `from` в конфигурации `knn`.

<!-- end -->

<!-- example knn_insert -->

### Вставка векторных данных

#### Автоэмбеддинги (рекомендуется)

Самый простой способ работать с векторными данными - использовать **автоэмбеддинги**. С этой функцией вы создаете таблицу с параметрами `MODEL_NAME` и `FROM`, а затем просто вставляете текстовые данные - Manticore автоматически генерирует эмбеддинги за вас.

##### Создание таблицы с автоэмбеддингами

При создании таблицы для автоэмбеддингов укажите:
- `MODEL_NAME`: Модель эмбеддингов, которую нужно использовать
- `FROM`: Какие поля использовать для генерации эмбеддингов (пустое значение означает все текстовые/строковые поля)
- `API_KEY`: Обязателен для удаленных моделей (OpenAI, Voyage, Jina). API-ключ проверяется при создании таблицы путем выполнения реального запроса к API.
- `API_URL`: Необязателен. Пользовательский URL конечной точки API. Если не указан, используется конечная точка провайдера по умолчанию (например, `https://api.openai.com/v1/embeddings` для OpenAI).
- `API_TIMEOUT`: Необязателен. Таймаут HTTP-запросов к API в секундах. Значение по умолчанию - 10 секунд. Установите `'0'`, чтобы использовать таймаут по умолчанию. Применяется и к проверочным запросам при создании таблицы, и к генерации эмбеддингов во время операций INSERT.

Для удаленных моделей `MODEL_NAME` можно записывать в двух формах:
- Устаревшая форма с префиксом провайдера: `openai/text-embedding-ada-002`, `voyage/voyage-3.5-lite`, `jina/jina-embeddings-v4`
- Явная форма provider-signal для пользовательских endpoint'ов: `openai:text-embedding-ada-002`, `openai:openai/text-embedding-ada-002`, `voyage:custom-model`, `jina:custom-model`

Когда вы используете форму `provider:model` вместе с `API_URL`, часть перед `:` только выбирает формат запроса. Часть после `:` отправляется на удаленную конечную точку без изменений. Это полезно для шлюзов, совместимых с OpenAI, таких как OpenRouter или LiteLLM.

**Поддерживаемые модели эмбеддингов:**

| Тип модели | Пример | Требуется API-ключ | Примечания |
|------------|---------|-----------------|-------|
| **ONNX (рекомендуется)** | `Xenova/all-MiniLM-L6-v2` | Нет | Локальные модели из любого репозитория Hugging Face, который содержит файл `.onnx`. Работают на быстром бэкенде Manticore ONNX Runtime. Список: [модели feature-extraction ONNX](https://huggingface.co/Xenova/models?pipeline_tag=feature-extraction&search=minilm). |
| **Sentence Transformers** | `sentence-transformers/all-MiniLM-L6-v2` | Нет | Локальные модели на основе BERT, загружаются автоматически. По-прежнему поддерживаются — если доступно, используйте ONNX выше. |
| **Qwen** | `Qwen/Qwen3-Embedding-0.6B` | Нет | Локальные модели семейства Qwen |
| **Llama** | `TinyLlama/TinyLlama-1.1B-Chat-v1.0` | Нет | Локальные модели семейства Llama |
| **Mistral** | `Locutusque/TinyMistral-248M-v2` | Нет | Локальные модели семейства Mistral |
| **Gemma** | `h2oai/embeddinggemma-300m` | Нет | Локальные модели семейства Gemma |
| **OpenAI** | `openai/text-embedding-ada-002` or `openai:text-embedding-ada-002` | Да | `API_KEY='***'` |
| **Voyage** | `voyage/voyage-3.5-lite` or `voyage:voyage-3.5-lite` | Да | `API_KEY='***'` |
| **Jina** | `jina/jina-embeddings-v4` or `jina:jina-embeddings-v4` | Да | `API_KEY='***'` |

**Требования к формату локальной модели:**
- Должна быть сохранена в формате `safetensors` (только один файл)
- Поддерживаемые семейства: Qwen, Llama, Mistral, Gemma
- Проверенные модели: `TinyLlama/TinyLlama-1.1B-Chat-v1.0`, `Locutusque/TinyMistral-248M-v2`, `Qwen/Qwen3-Embedding-0.6B`, `h2oai/embeddinggemma-300m`
- Другие модели `safetensors` тоже могут работать, но это не гарантируется

Дополнительную информацию о настройке атрибута `float_vector` можно найти [здесь](../Creating_a_table/Data_types.md#Float-vector).

<!-- intro -->
##### SQL:

<!-- request SQL -->

Использование локальной [ONNX-модели](https://huggingface.co/Xenova/models?pipeline_tag=feature-extraction&search=minilm) — рекомендуется (ключ API не нужен)
```sql
CREATE TABLE products (
    title TEXT,
    description TEXT,
    embedding_vector FLOAT_VECTOR KNN_TYPE='hnsw' HNSW_SIMILARITY='l2'
    MODEL_NAME='Xenova/all-MiniLM-L6-v2' FROM='title'
);
```

Использование sentence-transformers (ключ API не нужен; работает через путь Candle — если доступно, используйте ONNX выше)
```sql
CREATE TABLE products_st (
    title TEXT,
    description TEXT,
    embedding_vector FLOAT_VECTOR KNN_TYPE='hnsw' HNSW_SIMILARITY='l2'
    MODEL_NAME='sentence-transformers/all-MiniLM-L6-v2' FROM='title'
);
```

Использование локальных эмбеддингов Qwen (API-ключ не нужен)
```sql
CREATE TABLE products_qwen (
    title TEXT,
    description TEXT,
    embedding_vector FLOAT_VECTOR KNN_TYPE='hnsw' HNSW_SIMILARITY='l2'
    MODEL_NAME='Qwen/Qwen3-Embedding-0.6B' FROM='title' CACHE_PATH='/opt/homebrew/var/manticore/.cache/manticore'
);
```

Использование OpenAI (требуется параметр API_KEY)
```sql
CREATE TABLE products_openai (
    title TEXT,
    description TEXT,
    embedding_vector FLOAT_VECTOR KNN_TYPE='hnsw' HNSW_SIMILARITY='l2'
    MODEL_NAME='openai/text-embedding-ada-002' FROM='title,description' API_KEY='...'
);
```
Использование OpenAI с пользовательским API URL и таймаутом (необязательно)
```sql
CREATE TABLE products_openai_custom (
    title TEXT,
    description TEXT,
    embedding_vector FLOAT_VECTOR KNN_TYPE='hnsw' HNSW_SIMILARITY='l2'
    MODEL_NAME='openai:text-embedding-ada-002' FROM='title,description'
    API_KEY='***' API_URL='https://custom-api.example.com/v1/embeddings' API_TIMEOUT='30'
);
```

Использование шлюза, совместимого с OpenAI, который ожидает модельный идентификатор с указанием провайдера
```sql
CREATE TABLE products_openrouter (
    title TEXT,
    description TEXT,
    embedding_vector FLOAT_VECTOR KNN_TYPE='hnsw' HNSW_SIMILARITY='l2'
    MODEL_NAME='openai:openai/text-embedding-ada-002' FROM='title,description'
    API_KEY='***' API_URL='https://openrouter.ai/api/v1/embeddings' API_TIMEOUT='30'
);
```

Использование всех текстовых полей для эмбеддингов (FROM пуст)
```sql
CREATE TABLE products_all (
    title TEXT,
    description TEXT,
    embedding_vector FLOAT_VECTOR KNN_TYPE='hnsw' HNSW_SIMILARITY='l2'
    MODEL_NAME='Xenova/all-MiniLM-L6-v2' FROM=''
);
```

<!-- intro -->
##### Plain-режим (через файл конфигурации):

<!-- request Config -->
```ini
table products {
    type = rt
    path = /path/to/products
    rt_field = title
    rt_field = description
    rt_attr_float_vector = embedding_vector
    knn = {"attrs":[{"name":"embedding_vector","type":"hnsw","hnsw_similarity":"L2","hnsw_m":16,"hnsw_ef_construction":200,"model_name":"Xenova/all-MiniLM-L6-v2","from":"title"}]}
}
```

Использование OpenAI с API-ключом в plain-режиме:
```ini
table products_openai {
    type = rt
    path = /path/to/products_openai
    rt_field = title
    rt_field = description
    rt_attr_float_vector = embedding_vector
    knn = {"attrs":[{"name":"embedding_vector","type":"hnsw","hnsw_similarity":"L2","hnsw_m":16,"hnsw_ef_construction":200,"model_name":"openai/text-embedding-ada-002","from":"title,description","api_key":"your-api-key-here"}]}
}
```

Использование всех текстовых полей (пустой FROM):
```ini
table products_all {
    type = rt
    path = /path/to/products_all
    rt_field = title
    rt_field = description
    rt_attr_float_vector = embedding_vector
    knn = {"attrs":[{"name":"embedding_vector","type":"hnsw","hnsw_similarity":"L2","hnsw_m":16,"hnsw_ef_construction":200,"model_name":"Xenova/all-MiniLM-L6-v2","from":""}]}
}
```

**Важные замечания для plain-режима:**
- При использовании `model_name` вы **не должны** указывать `dims` - модель автоматически определяет размерность вектора. Параметры `dims` и `model_name` взаимоисключающие.
- Если вы **не** используете `model_name` (ручная вставка вектора), вы **должны** указать `dims`, чтобы задать размерность вектора.
- Параметр `from` задает, какие поля использовать для генерации эмбеддингов (список через запятую или пустая строка для всех текстовых/строковых полей). Этот параметр обязателен при использовании `model_name`.
- Для моделей на основе API (OpenAI, Voyage, Jina) включите параметр `api_key` в конфигурацию knn

<!-- end -->

##### Вставка данных с автоэмбеддингами

<!--
данные для следующего примера:

DROP TABLE IF EXISTS products;
CREATE TABLE products(title text, embedding_vector float_vector knn_type='hnsw' hnsw_similarity='l2' model_name='Xenova/all-MiniLM-L6-v2' from='title');
DROP TABLE IF EXISTS products_openai;
CREATE TABLE products_openai(title text, description text, embedding_vector float_vector knn_type='hnsw' hnsw_similarity='l2' model_name='Xenova/all-MiniLM-L6-v2' from='title,description');
-->

<!-- example inserting_embeddings -->

При использовании автоэмбеддингов вы можете:

- Не указывать поле вектора и позволить Manticore сгенерировать эмбеддинги из полей, перечисленных в `FROM`
- Явно передать свой вектор для строки
- Передать `()`, чтобы пропустить генерацию и сохранить нулевой вектор

Если позже вы выполните `ALTER TABLE ... REBUILD EMBEDDINGS`, строки, которые сейчас содержат нулевые векторы из `()`, тоже будут пересозданы.

<!-- intro -->
##### SQL:

<!-- request SQL -->

Вставка только текстовых данных - эмбеддинги генерируются автоматически
```sql
INSERT INTO products (title) VALUES
('machine learning artificial intelligence'),
('banana fruit sweet yellow');
```

Вставка пользовательского вектора
```sql
INSERT INTO products (title, embedding_vector) VALUES
('machine learning artificial intelligence', (0.653448,0.192478,0.017971,0.339821));
```

Вставка нескольких полей - оба используются для эмбеддинга, если `FROM='title,description'`
```sql
INSERT INTO products_openai (title, description) VALUES
('smartphone', 'latest mobile device with advanced features'),
('laptop', 'portable computer for work and gaming');
```

Вставка пустого вектора (без автоматической генерации; сохраняется нулевой вектор)
```sql
INSERT INTO products (title, embedding_vector) VALUES
('no embedding item', ());
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

Вставка только текстовых данных - эмбеддинги генерируются автоматически
```JSON
POST /sql?mode=raw -d "INSERT INTO products (title) VALUES ('machine learning artificial intelligence'),('banana fruit sweet yellow')"
```

Вставьте несколько полей — они оба используются для встраивания, если FROM='title,description'
```JSON
POST /sql?mode=raw -d "INSERT INTO products_openai (title, description) VALUES ('smartphone', 'latest mobile device with advanced features'), ('laptop', 'portable computer for work and gaming')"
```

Вставка пустого вектора (документ исключается из векторного поиска)
```JSON
POST /sql?mode=raw -d "INSERT INTO products (title, embedding_vector) VALUES ('no embedding item', ())"
```

<!-- end -->

##### Поиск с автоэмбеддингами

<!-- example embeddings_search -->
Поиск работает так же - передайте текст запроса, и Manticore сгенерирует эмбеддинги и найдет похожие документы:

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SELECT id, knn_dist() FROM products WHERE knn(embedding_vector, 'machine learning');
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

Использование текстового запроса с автоэмбеддингами
```json
POST /search
{
    "table": "products",
    "knn": {
        "field": "embedding_vector",
        "query": "machine learning"
    }
}
```

Использование векторного запроса напрямую
```json
POST /search
{
    "table": "products",
    "knn": {
        "field": "embedding_vector",
        "query": [0.1, 0.2, 0.3, 0.4]
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

#### Ручная вставка векторов

<!-- example manual_vector -->
В качестве альтернативы вы можете вручную вставлять предварительно вычисленные векторные данные, убедившись, что они соответствуют размерности, указанной при создании таблицы. Вы также можете вставить пустой вектор; это означает, что документ будет исключен из результатов векторного поиска.

**Важно:** При использовании `hnsw_similarity='cosine'` векторы автоматически нормализуются при вставке до единичных векторов (векторов с математической длиной/модулем 1.0). Такая нормализация сохраняет направление вектора, одновременно стандартизируя его длину, что требуется для эффективных вычислений косинусного сходства. Это означает, что сохраненные значения будут отличаться от исходных входных значений.

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
	"id":1,
	"created":true,
	"result":"created",
	"status":201
}

{
	"table":"test",
	"id":2,
	"created":true,
	"result":"created",
	"status":201
}
```

<!-- end -->

<!-- example knn_search -->

### Векторный поиск KNN

Теперь вы можете выполнять поиск KNN с помощью предложения `knn` как в формате SQL, так и в формате JSON. Оба интерфейса поддерживают одни и те же основные параметры, обеспечивая единообразный опыт независимо от выбранного формата:

- SQL: `select ... from <table name> where knn ( <field>, <query vector> [,<options>] )`
- JSON:
  ```
  POST /search
  {
      "table": "<table name>",
      "knn":
      {
          "field": "<field>",
          "query": "<text or vector>",
          "ef": <ef>,
		  "rescore": <rescore>,
		  "oversampling": <oversampling>
      }
  }
  ```

Параметры:
* `field`: Имя атрибута float_vector, содержащего векторные данные.
* `k`: Устаревший параметр. Вместо него используйте `limit` в запросе. Он использовался для задания количества документов, которое должен вернуть один индекс HNSW. Однако фактическое число документов в итоговых результатах может отличаться. Например, если система работает с таблицами real-time, разделенными на disk chunks, каждый chunk может вернуть `k` документов, что приведет к итоговому числу больше указанного `k` (так как суммарное количество будет `num_chunks * k`). С другой стороны, итоговое число документов может оказаться меньше `k`, если после запроса `k` документов часть из них будет отфильтрована по определенным атрибутам. Важно отметить, что параметр `k` не применяется к ramchunks. В контексте ramchunks процесс получения работает иначе, и поэтому влияние параметра `k` на число возвращаемых документов неприменимо.
* `query`: (Рекомендуемый параметр) Поисковый запрос, который может быть:
  - Текстовой строкой: автоматически преобразуется в эмбеддинги, если для поля настроены автоэмбеддинги. Вернет ошибку, если у поля нет автоэмбеддингов.
  - Массивом векторов: работает так же, как `query_vector`.
* `query_vector`: (Устаревший параметр) Поисковый вектор в виде массива чисел. По-прежнему поддерживается для обратной совместимости.
  **Примечание:** Используйте либо `query`, либо `query_vector`, но не оба параметра в одном запросе.
* `ef`: необязательный размер динамического списка, используемого во время поиска. Более высокое значение `ef` дает более точный, но более медленный поиск. Значение по умолчанию - 10.
* `rescore`: Включает повторное оценивание KNN (по умолчанию включено). Установите `0` в SQL или `false` в JSON, чтобы отключить повторное оценивание. После завершения поиска KNN с использованием квантизованных векторов (с возможным oversampling) расстояния пересчитываются по исходным (full-precision) векторам, и результаты пересортировываются для повышения точности ранжирования.
* `oversampling`: Задает коэффициент (значение float), на который умножается `k` при выполнении поиска KNN, из-за чего с использованием квантизованных векторов извлекается больше кандидатов, чем требуется. По умолчанию применяется `oversampling=3.0`. Эти кандидаты можно затем переоценить, если повторное оценивание включено. Oversampling также работает и с неквантизованными векторами. Поскольку он увеличивает `k`, а это влияет на работу индекса HNSW, он может вызвать небольшое изменение точности результатов.
* `early_termination`: Включает или отключает адаптивное раннее завершение при обходе графа HNSW. По умолчанию включено. Установите `0` в SQL или `false` в JSON, чтобы отключить. Подробности см. в разделе [Раннее завершение](../Searching/KNN.md#Early-termination).

Когда задан текстовый запрос (то есть Manticore сначала встраивает строку перед поиском), число потоков, используемых библиотекой embeddings, можно переопределить для каждого запроса в SQL с помощью `OPTION embeddings_threads = N`. Это значение ограничивает вызов embeddings только для этого запроса, переопределяя глобальную настройку [embeddings_threads](../Server_settings/Searchd.md#embeddings_threads); `0` означает отсутствие ограничения. Эта опция не влияет на запрос, если он передан в виде массива векторов.

Документы всегда сортируются по расстоянию до вектора поиска. Любые дополнительные критерии сортировки, которые вы укажете, будут применяться после этого основного условия сортировки. Чтобы получить расстояние, есть встроенная функция [knn_dist()](../Functions/Other_functions.md#KNN_DIST%28%29).

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
select id, knn_dist() from test where knn ( image_vector, (0.286569,-0.031816,0.066684,0.032926), { ef=2000, oversampling=3.0, rescore=1 } );
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

### Квантование векторов

Индексы HNSW должны быть полностью загружены в память, чтобы выполнять поиск KNN, что может приводить к значительному потреблению памяти. Чтобы снизить использование памяти, можно применить скалярное квантование - технику, которая сжимает многомерные векторы, представляя каждую компоненту (размерность) ограниченным числом дискретных значений. Manticore поддерживает 8-битное и 1-битное квантование, то есть каждая компонента вектора сжимается с 32-битного float до 8 бит или даже до 1 бита, уменьшая использование памяти в 4 раза или 32 раза соответственно. Эти сжатые представления также позволяют быстрее вычислять расстояния, поскольку больше компонент вектора можно обработать одной SIMD-инструкцией. Хотя скалярное квантование вносит некоторую погрешность аппроксимации, часто это оправданный компромисс между точностью поиска и эффективностью использования ресурсов. Для еще более высокой точности квантование можно сочетать с повторным оцениванием и oversampling: запрашивается больше кандидатов, чем нужно, а расстояния для этих кандидатов пересчитываются с использованием исходных 32-битных float-векторов.

Поддерживаемые типы квантования:
* `8bit`: Каждая компонента вектора квантуется до 8 бит.
* `1bit`: Каждая компонента вектора квантуется до 1 бита. Используется асимметричное квантование: векторы запроса квантуются до 4 бит, а сохраненные векторы - до 1 бита. Такой подход дает более высокую точность, чем более простые методы, но с некоторым компромиссом по производительности.
* `1bitsimple`: Каждая компонента вектора квантуется до 1 бита. Этот метод быстрее, чем `1bit`, но обычно менее точен.

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
POST /sql?mode=raw -d "create table test ( title text, image_vector float_vector knn_type='hnsw' knn_dims='4' hnsw_similarity='l2' quantization='1bit')"
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

### Поиск похожих документов по id

> ПРИМЕЧАНИЕ: Поиск похожих документов по id требует [Manticore Buddy](../Installation/Manticore_Buddy.md). Если он не работает, убедитесь, что Buddy установлен.

Поиск документов, похожих на конкретный документ по его уникальному ID, - распространенная задача. Например, когда пользователь просматривает определенный товар, Manticore Search может эффективно определить и показать список товаров, которые наиболее похожи на него в векторном пространстве. Вот как это сделать:

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

Параметры:
* `field`: Имя атрибута float_vector, содержащего векторные данные.
* `k`: Это число документов, которые нужно вернуть, и ключевой параметр для индексов Hierarchical Navigable Small World (HNSW). Он задает количество документов, которое должен вернуть один индекс HNSW. Однако фактическое число документов в итоговых результатах может отличаться. Например, если система работает с таблицами real-time, разделенными на disk chunks, каждый chunk может вернуть `k` документов, что приведет к итоговому числу больше указанного `k` (так как суммарное количество будет `num_chunks * k`). С другой стороны, итоговое число документов может оказаться меньше `k`, если после запроса `k` документов часть из них будет отфильтрована по определенным атрибутам. Важно отметить, что параметр `k` не применяется к ramchunks. В контексте ramchunks процесс получения работает иначе, и поэтому влияние параметра `k` на число возвращаемых документов неприменимо.
* `document id`: ID документа для KNN-поиска похожих документов.


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

### Фильтрация результатов векторного поиска KNN

Manticore также поддерживает дополнительную фильтрацию документов, возвращаемых поиском KNN, либо по полнотекстовому совпадению, либо по фильтрам атрибутов, либо по обоим сразу.

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
		"k": 5
	},
	"query":
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

<!-- example knn_filtering_strategies -->

### Стратегии фильтрации: prefilter против postfilter

При объединении векторного поиска KNN с фильтрами атрибутов Manticore поддерживает две стратегии, которые отличаются тем, когда фильтр применяется относительно обхода графа HNSW.

* Предварительная фильтрация (по умолчанию; `prefilter=1` (SQL) или `"prefilter": true` (JSON, default)) передает фильтр непосредственно в обход HNSW. Каждый кандидат проверяется на соответствие фильтру до добавления в результирующую кучу - в итоговые `k` результатов попадают только подходящие документы. Это уменьшает число бесполезных вычислений расстояния и гарантирует, что будет возвращено ровно `k` подходящих документов (если существует `k` подходящих документов).

* Постфильтрация (`prefilter=0` (SQL) или `"prefilter": false` (JSON)) сначала выполняет поиск KNN по всему набору данных, а затем применяет фильтр к результатам. Это безопасно и предсказуемо: граф HNSW обходится без вмешательства, а фильтр влияет только на то, какие результаты возвращаются клиенту. Недостаток в том, что на кандидатов, которые в итоге будут отброшены, граф может потратить ресурсы. При жестком фильтре, который совпадает лишь с небольшой долей документов, возвращаемые `k` результатов могут оказаться значительно меньше запрошенного числа, потому что большинство кандидатов KNN не пройдет фильтр.

Внутри Manticore использует алгоритм на основе ACORN-1 для предварительной фильтрации. Наивная предварительная фильтрация просто пропускала бы несовпадающие узлы, что создает риск потерять "bridge"-узлы, соединяющие разъединенные части графа HNSW, из-за чего полнота резко падает по мере ужесточения фильтра. ACORN-1 избегает этого: когда узел не проходит фильтр, его соседи все равно добавляются в очередь обхода. Это позволяет обходу обходить отфильтрованные узлы и сохранять связность графа. Обход ACORN-1 автоматически включается, когда фильтр проходит менее 60% всех документов.

**Автоматический переход на brute-force:** Когда включена предварительная фильтрация, Manticore оценивает, что дешевле - выполнить полный перебор расстояний по отфильтрованному подмножеству или обходить граф HNSW. Оценка сравнивает ожидаемое число узлов, посещаемых HNSW, с числом документов, прошедших фильтр. Если отфильтрованное множество достаточно мало, чтобы его прямой перебор был быстрее, Manticore автоматически переключается на brute-force, полностью пропуская HNSW. Это обеспечивает корректность и хорошую производительность даже при экстремальной селективности.

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
-- prefilter (default): filter applied during HNSW traversal (ACORN-1 used automatically)
SELECT id, knn_dist() FROM test
WHERE knn ( image_vector, (0.286569,-0.031816,0.066684,0.032926) )
AND price < 100;

-- postfilter: KNN runs over full dataset, filter applied to results
SELECT id, knn_dist() FROM test
WHERE knn ( image_vector, (0.286569,-0.031816,0.066684,0.032926), { prefilter=0 } )
AND price < 100;
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```json
// prefilter (default): filter applied during HNSW traversal
POST /search
{
    "table": "test",
    "knn": {
        "field": "image_vector",
        "query": [0.286569,-0.031816,0.066684,0.032926]
    },
    "query": {
        "range": { "price": { "lt": 100 } }
    }
}

// postfilter: filter applied after KNN search
POST /search
{
    "table": "test",
    "knn": {
        "field": "image_vector",
        "query": [0.286569,-0.031816,0.066684,0.032926],
        "prefilter": false
    },
    "query": {
        "range": { "price": { "lt": 100 } }
    }
}
```

<!-- end -->

### Раннее завершение

По умолчанию Manticore использует адаптивный алгоритм раннего завершения при обходе графа HNSW. Вместо того чтобы всегда исследовать весь набор кандидатов, заданный `ef`, он отслеживает скорость, с которой новые кандидаты улучшают результирующий набор, и завершает поиск раньше, когда эта скорость стабильно опускается ниже порога. Это уменьшает число вычислений расстояний без заметного ухудшения качества результатов.

Раннее завершение включено по умолчанию и автоматически отключается, когда `k` равно 10 или меньше, поскольку для таких маленьких наборов результатов накладные расходы алгоритма не оправданы. Выигрыш в производительности масштабируется с `k` - чем больше набор результатов, тем больше вычислений расстояния можно сэкономить за счет раннего завершения.

Учтите, что oversampling умножает эффективное `k`, используемое во время обхода HNSW, поэтому раннее завершение тоже выигрывает от oversampling: более высокое эффективное `k` означает больше кандидатов, которые потенциально можно пропустить.

<!-- example knn_early_termination -->
Чтобы явно управлять ранним завершением, используйте параметр `early_termination`:

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
-- disable early termination
SELECT id, knn_dist() FROM test WHERE knn ( image_vector, (0.286569,-0.031816,0.066684,0.032926), { ef=200, early_termination=0 } );

-- enable early termination explicitly (default)
SELECT id, knn_dist() FROM test WHERE knn ( image_vector, (0.286569,-0.031816,0.066684,0.032926), { ef=200, early_termination=1 } );
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
        "ef": 200,
        "early_termination": false
    }
}
```

<!-- end -->

Когда стоит отключить раннее завершение:
* Когда критически важна точность набора результатов и вы не можете позволить себе никакой аппроксимации сверх той, которую уже обеспечивает HNSW.
* Когда используются низкие значения `k` (примерно 30 или меньше), при которых раннее завершение дает мало выигрыша в производительности, но может снизить точность.

<!-- proofread -->
