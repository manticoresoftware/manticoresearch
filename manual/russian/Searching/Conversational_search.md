# Диалоговый поиск

Диалоговый поиск позволяет Manticore Buddy отвечать на вопросы по уже векторизованной таблице. Buddy извлекает самые релевантные строки через KNN-поиск, превращает эти строки в контекст и отправляет контекст вместе с историей беседы в LLM.

Им управляют из SQL с помощью:

* `CREATE CHAT MODEL`
* `SHOW CHAT MODELS`
* `DESCRIBE CHAT MODEL`
* `DROP CHAT MODEL`
* `CALL CHAT`

## Перед началом

Вам нужна векторизованная таблица и провайдер LLM. Требования к таблице описаны ниже. Учетные данные провайдера можно задать в `CREATE CHAT MODEL` через `api_key` либо передать через соответствующую переменную окружения, например `OPENAI_API_KEY`.

## Как это работает

Когда запускается `CALL CHAT`, Buddy строит ответ с retrieval-augmented generation в таком порядке:

1. Buddy загружает чат-модель.
2. Buddy загружает историю беседы для переданного UUID беседы. Если UUID не передан, он создается.
3. Buddy анализирует целевую таблицу и выбирает поле `FLOAT_VECTOR`.
4. LLM решает, как обработать сообщение: выполнить новый поиск, ответить на основе предыдущего поискового контекста или ответить без извлечения.
5. Когда требуется извлечение, Buddy выполняет KNN-поиск по выбранному векторному полю.
6. Buddy строит контекст для LLM из исходных полей `from='...'` векторного поля.
7. Настроенная LLM генерирует ответ.
8. Buddy сохраняет сообщение пользователя и ответ ассистента в истории беседы.

Пятый аргумент `CALL CHAT` внутри называется `fields`, но для диалогового поиска это означает векторное поле, используемое `knn(...)`. Это не список полей для возврата. Buddy выбирает строки через `SELECT *`, а затем убирает векторные столбцы из полезной нагрузки `sources`, чтобы ответ не включал большие значения embedding.

## Требования к таблице

В таблице должно быть как минимум одно поле `FLOAT_VECTOR`, настроенное для [автополучения embeddings](../Searching/KNN.md#Auto-Embeddings-%28Recommended%29). Векторное поле должно содержать `from='...'`, потому что Buddy использует эти исходные поля как контекст для LLM.

В примерах ниже используется `onnx-models/all-MiniLM-L12-v2-onnx`, который работает через рекомендуемый путь ONNX и не требует ключа API для embeddings.

<!-- example conversational_search_create_table -->

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE TABLE docs (
    id BIGINT,
    title TEXT,
    content TEXT,
    embedding FLOAT_VECTOR
        knn_type='hnsw'
        hnsw_similarity='cosine'
        model_name='onnx-models/all-MiniLM-L12-v2-onnx'
        from='title,content'
) TYPE='rt';

INSERT INTO docs(id, title, content) VALUES
    (1, 'Vector search', 'Vector search compares embeddings to find semantically similar documents.'),
    (2, 'Full-text search', 'Full-text search matches terms and phrases in indexed text.');
```

<!-- end -->

Если `CALL CHAT` не указывает векторное поле, Buddy использует первое поле `FLOAT_VECTOR`, найденное в определении таблицы.

## Создание чат-модели

Используйте `CREATE CHAT MODEL`, чтобы сохранить провайдера LLM, идентификатор модели и настройки извлечения.

<!-- example conversational_search_create_model -->

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE CHAT MODEL assistant (
    model='openai:gpt-4o-mini'
);
```

<!-- end -->

Также можно задать параметры провайдера и лимиты извлечения:

<!-- example conversational_search_create_model_extended -->

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE CHAT MODEL support_assistant (
    model='openai:gpt-4o-mini',
    api_key='your-provider-api-key',
    base_url='http://host.docker.internal:8787/v1',
    timeout=60,
    retrieval_limit=5,
    max_document_length=3000
);
```
<!-- end -->

Обычные параметры:

| Параметр | Обязателен | Описание |
|---|---:|---|
| `model` | Да | Идентификатор модели LLM в формате `provider:model`. |
| `description` | Нет | Сохраненное описание. |
| `api_key` | Нет | Ключ API провайдера, передаваемый расширению `llm`. |
| `base_url` | Нет | Базовый URL провайдера или прокси. |
| `timeout` | Нет | Тайм-аут запроса к LLM, `1..65536`. |
| `retrieval_limit` | Нет | Число документов, запрашиваемых через KNN, `1..50`; по умолчанию `5`. |
| `max_document_length` | Нет | Лимит контекста на документ. `0` отключает усечение; `100..65536` усекaет; по умолчанию `2000`. |

Имена чат-моделей могут содержать только буквы, цифры и символы подчеркивания.

Параметр `model` должен использовать формат `provider:model`:

```sql
model='openai:gpt-4o-mini'
```

Параметр `api_key` провайдера необязателен, если ключ провайдера уже доступен в окружении Buddy. Например, сервис Docker Compose может передавать ключи провайдера так:

```yaml
environment:
  - OPENAI_API_KEY=${OPENAI_API_KEY}
  - OPENROUTER_API_KEY=${OPENROUTER_API_KEY}
```

Если `api_key` не задан в `CREATE CHAT MODEL`, расширение `llm` может использовать соответствующую переменную окружения провайдера. Задавайте `api_key` в чат-модели только если этой модели нужен другой ключ.

## Синтаксис `CALL CHAT`

```sql
CALL CHAT(
    'query',
    'table',
    'model_name',
    'conversation_uuid',
    'vector_field'
);
```

Аргументы только позиционные:

| Позиция | Аргумент | Обязателен | Описание |
|---:|---|---:|---|
| 1 | `query` | Да | Вопрос пользователя. |
| 2 | `table` | Да | Таблица для поиска. |
| 3 | `model_name` | Да | Имя чат-модели. |
| 4 | `conversation_uuid` | Нет | UUID существующей беседы или пустая строка. |
| 5 | `fields` / vector field | Нет | Поле `FLOAT_VECTOR`, используемое в `knn(...)`. |

Аргумент таблицы должен быть обычным идентификатором таблицы, при необходимости с квалификацией `database.table`. Аргумент векторного поля должен быть обычным идентификатором поля.

## Задание вопросов

Используйте `CALL CHAT` с запросом, таблицей и чат-моделью.

<!-- example conversational_search_call_chat -->

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CALL CHAT(
    'What is vector search?',
    'docs',
    'assistant'
);
```

<!-- end -->

Чтобы продолжить беседу, передайте тот же UUID беседы:

<!-- example conversational_search_continue_chat -->

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CALL CHAT(
    'Can you explain it with an example?',
    'docs',
    'assistant',
    'docs-chat-001'
);
```

<!-- end -->

Чтобы искать по конкретному векторному полю, передайте его в качестве пятого аргумента:

<!-- example conversational_search_vector_field -->

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CALL CHAT(
    'Find documents where the title is about vector search',
    'docs',
    'assistant',
    '',
    'title_embedding'
);
```

<!-- end -->

Если пятый аргумент присутствует, Buddy проверяет, что поле существует и имеет тип `FLOAT_VECTOR`. Если аргумент опущен, Buddy определяет первое поле `FLOAT_VECTOR` из `SHOW CREATE TABLE`.


## Подробности поиска и контекста

Когда Buddy требуется извлечение, он выполняет KNN-поиск по выбранному векторному полю и возвращает до `retrieval_limit` строк. Значение порога расстояния по умолчанию равно `0.8`.

Buddy использует извлеченные строки как контекст для LLM. Те же строки возвращаются в `sources`, при этом `knn_dist` включается, а столбцы `FLOAT_VECTOR` удаляются.

`max_document_length` ограничивает объем текста из каждой исходной строки, который можно передать в LLM. Используйте `0`, чтобы отключить усечение; иначе задайте значение от `100` до `65536`.

## Ответ

`CALL CHAT` возвращает одну строку:

| Столбец | Описание |
|---|---|
| `conversation_uuid` | Существующий или сгенерированный UUID беседы. |
| `user_query` | Исходный запрос пользователя. |
| `search_query` | Самостоятельный поисковый запрос, использованный для извлечения. |
| `response` | Ответ LLM. |
| `sources` | Строка JSON с извлеченными исходными строками. |

Пример формы ответа:

```json
{
  "conversation_uuid": "docs-chat-001",
  "user_query": "What is vector search?",
  "search_query": "vector search, embeddings, similarity search",
  "response": "Vector search finds similar items by comparing embeddings...",
  "sources": "[{\"id\":1,\"title\":\"Vector Search\",\"content\":\"...\",\"knn_dist\":0.12}]"
}
```

Векторные поля не включаются в `sources`.

## Управление чат-моделями

Список моделей:

<!-- example conversational_search_show_models -->

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SHOW CHAT MODELS;
```

<!-- end -->

Описание модели:

<!-- example conversational_search_describe_model -->

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
DESCRIBE CHAT MODEL assistant;
```

<!-- end -->

Удаление модели:

<!-- example conversational_search_drop_model -->

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
DROP CHAT MODEL assistant;
```

<!-- end -->

Безопасное удаление:

<!-- example conversational_search_drop_model_if_exists -->

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
DROP CHAT MODEL IF EXISTS assistant;
```

<!-- end -->

`SHOW CHAT MODELS` возвращает `name`, `model` и `created_at`. `DESCRIBE CHAT MODEL` возвращает `property` и `value`; сохраненные API-ключи показываются как `HIDDEN`.

Удаление чат-модели также удаляет таблицу истории бесед этой модели. История бесед хранится отдельно для каждой модели и записывается с TTL 30 дней.
