# Ссылки

### SQL-команды
##### Управление схемой
* [CREATE TABLE](Creating_a_table/Local_tables/Real-time_table.md#CREATE-TABLE-command:) - Создает новую таблицу
* [CREATE TABLE LIKE](Creating_a_table/Local_tables/Real-time_table.md#CREATE-TABLE-LIKE:) - Создает таблицу, используя другую в качестве шаблона
* [CREATE TABLE LIKE ... WITH DATA](Creating_a_table/Local_tables/Real-time_table.md#CREATE-TABLE-LIKE:) - Копирует таблицу
* [CREATE SOURCE](Integration/Kafka.md#Source) - Создает источник потребителя Kafka
* [CREATE MATERIALIZED VIEW](Integration/Kafka.md#Materialized-view) - Трансформация данных из сообщений Kafka
* [CREATE MV](Integration/Kafka.md#Materialized-view) - То же, что и предыдущая команда
* [DESCRIBE](Listing_tables.md#DESCRIBE) - Выводит список полей таблицы и их типы
* [ALTER TABLE](Updating_table_schema_and_settings.md) - Изменяет схему/настройки таблицы
* [ALTER TABLE REBUILD SECONDARY](Updating_table_schema_and_settings.md#Rebuilding-a-secondary-index) - Обновляет/восстанавливает вторичные индексы
* [ALTER TABLE type='distributed'](Updating_table_schema_and_settings.md#Changing-a-distributed-table) - Обновляет/восстанавливает вторичные индексы
* [ALTER TABLE RENAME](Updating_table_schema_and_settings.md#Renaming-a-real-time-table)
* [ALTER MATERIALIZED VIEW {name} suspended=1](Integration/Kafka.md#Altering-materialized-views) - Приостанавливает или возобновляет потребление из источника Kafka
* [DROP TABLE IF EXISTS](Deleting_a_table.md#Deleting-a-table) - Удаляет таблицу (если она существует)
* [SHOW TABLES](Listing_tables.md#DESCRIBE) - Показывает список таблиц
* [SHOW SOURCES](Integration/Kafka.md#Listing) - Показывает список источников Kafka
* [SHOW MATERIALIZED VIEWS](Integration/Kafka.md#Listing) - Показывает список материализованных представлений
* [SHOW MVS](Integration/Kafka.md#Listing) - Псевдоним предыдущей команды
* [SHOW CREATE TABLE](Listing_tables.md#DESCRIBE) - Показывает SQL-команду для создания таблицы
* [SHOW TABLE INDEXES](Node_info_and_management/Table_settings_and_status/SHOW_TABLE_INDEXES.md) - Отображает информацию о доступных вторичных индексах таблицы
* [SHOW TABLE STATUS](Node_info_and_management/Table_settings_and_status/SHOW_TABLE_STATUS.md) - Показывает информацию о текущем состоянии таблицы
* [SHOW TABLE SETTINGS](Node_info_and_management/Table_settings_and_status/SHOW_TABLE_SETTINGS.md) - Показывает настройки таблицы
* [SHOW LOCKS](Securing_and_compacting_a_table/Freezing_a_table.md#SHOW-LOCKS) - Показывает информацию о замороженных таблицах

##### Управление данными
* [INSERT](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md) - Добавляет новые документы
* [REPLACE](Data_creation_and_modification/Updating_documents/REPLACE.md) - Заменяет существующие документы новыми
* [REPLACE .. SET](Data_creation_and_modification/Updating_documents/REPLACE.md?client=REPLACE+SET) - Заменяет одно или несколько полей в таблице
* [UPDATE](Data_creation_and_modification/Updating_documents/UPDATE.md) - Обновляет документы на месте
* [DELETE](Data_creation_and_modification/Deleting_documents.md) - Удаляет документы
* [TRUNCATE TABLE](Emptying_a_table.md) - Удаляет все документы из таблицы

##### Резервное копирование
* [BACKUP](Securing_and_compacting_a_table/Backup_and_restore.md#BACKUP-SQL-command-reference) - Создает резервные копии ваших таблиц

##### SELECT
* [SELECT](Searching/Full_text_matching/Basic_usage.md#SQL) - Поиск
  * [WHERE](Searching/Filters.md#Filters) - Фильтры
  * [GROUP BY](Searching/Grouping.md) - Группировка результатов поиска
  * [GROUP BY ORDER](Searching/Grouping.md) - Сортировка групп
  * [GROUP BY HAVING](Searching/Grouping.md) - Фильтрация групп
  * [OPTION](Searching/Options.md#OPTION) - Опции запроса
  * [FACET](Searching/Faceted_search.md) - Фасетный поиск
  * [SUB-SELECTS](Searching/Sub-selects.md) - Об использовании подзапросов SELECT
  * [JOIN](Searching/Joining.md) - Соединение таблиц в SELECT
* [EXPLAIN QUERY](Searching/Full_text_matching/Profiling.md#Profiling-without-running-a-query) - Показывает план выполнения запроса без его запуска
* [SHOW META](Node_info_and_management/SHOW_META.md) - Показывает расширенную информацию о выполненном запросе
* [SHOW PROFILE](Node_info_and_management/Profiling/Query_profile.md) - Показывает информацию о профилировании выполненного запроса
* [SHOW PLAN](Searching/Full_text_matching/Profiling.md#Profiling-the-query-tree-in-SQL) - Показывает план выполнения запроса после его выполнения
* [SHOW WARNINGS](Node_info_and_management/SHOW_WARNINGS.md) - Показывает предупреждения от последнего запроса

##### Сброс различных данных
* [FLUSH ATTRIBUTES](Securing_and_compacting_a_table/Flushing_attributes.md) - Принудительно сбрасывает обновленные атрибуты на диск
* [FLUSH HOSTNAMES](Securing_and_compacting_a_table/Flushing_hostnames.md) - Обновляет ассоциации IP с именами хостов агентов
* [FLUSH LOGS](Logging/Rotating_query_and_server_logs.md) - Инициирует переоткрытие файлов логов searchd и запроса (аналогично USR1)

##### Оптимизация таблиц реального времени
* [FLUSH RAMCHUNK](Securing_and_compacting_a_table/Flushing_RAM_chunk_to_a_new_disk_chunk.md#FLUSH-RAMCHUNK) - Форсирует создание нового дискового чанка
* [FLUSH TABLE](Securing_and_compacting_a_table/Flushing_RAM_chunk_to_disk.md#FLUSH-TABLE) - Сбрасывает RAM-чанк таблицы реального времени на диск
* [OPTIMIZE TABLE](Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE) - Ставит таблицу реального времени в очередь на оптимизацию

##### Импорт в таблицу реального времени
* [ATTACH TABLE](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Attaching_one_table_to_another.md) - Переносит данные из простой таблицы в таблицу реального времени
* [IMPORT TABLE](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Importing_table.md) - Импортирует ранее созданную таблицу RT или PQ в сервер, работающий в режиме RT

##### Репликация
* [JOIN CLUSTER](Creating_a_cluster/Setting_up_replication/Joining_a_replication_cluster.md) - Присоединяется к кластеру репликации
* [ALTER CLUSTER](Creating_a_cluster/Setting_up_replication/Managing_replication_nodes.md) - Добавляет/удаляет таблицу из кластера репликации
* [SET CLUSTER](Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Cluster-parameters) - Изменяет настройки кластера репликации
* [DELETE CLUSTER](Creating_a_cluster/Setting_up_replication/Deleting_a_replication_cluster.md) - Удаляет кластер репликации

##### Поворот простой таблицы
* [RELOAD TABLE](Data_creation_and_modification/Adding_data_from_external_storages/Rotating_a_table.md#RELOAD-TABLE) - Поворачивает простую таблицу
* [RELOAD TABLES](Data_creation_and_modification/Adding_data_from_external_storages/Rotating_a_table.md#RELOAD-TABLE) - Поворачивает все простые таблицы

##### Транзакции
* [BEGIN](Data_creation_and_modification/Transactions.md#BEGIN,-COMMIT,-and-ROLLBACK) - Начинает транзакцию
* [COMMIT](Data_creation_and_modification/Transactions.md#BEGIN,-COMMIT,-and-ROLLBACK) - Завершает транзакцию
* [ROLLBACK](Data_creation_and_modification/Transactions.md#BEGIN,-COMMIT,-and-ROLLBACK) - Отменяет транзакцию

##### CALL
* [CALL SUGGEST, CALL QSUGGEST](Searching/Spell_correction.md#CALL-QSUGGEST,-CALL-SUGGEST) - Предлагает слова с исправлением орфографии
* [CALL SNIPPETS](Searching/Highlighting.md) - Формирует выделенный фрагмент результата из предоставленных данных и запроса
* [CALL PQ](Searching/Percolate_query.md) - Выполняет перколяционный запрос
* [CALL KEYWORDS](Searching/Autocomplete.md#CALL-KEYWORDS) - Используется для проверки того, как токенизируются ключевые слова. Также позволяет получить токенизированные формы предоставленных ключевых слов
* [CALL AUTOCOMPLETE](Searching/Autocomplete.md#CALL-AUTOCOMPLETE) - Автоматически дополняет ваш поисковый запрос

##### Плагины
* [CREATE FUNCTION](Extensions/UDFs_and_Plugins/UDF/Creating_a_function.md) - Устанавливает функцию, определённую пользователем (UDF)
* [DROP FUNCTION](Extensions/UDFs_and_Plugins/UDF/Deleting_a_function.md) - Удаляет функцию, определённую пользователем (UDF)
* [CREATE PLUGIN](Extensions/UDFs_and_Plugins/Plugins/Creating_a_plugin.md) - Устанавливает плагин
* [CREATE BUDDY PLUGIN](Extensions/UDFs_and_Plugins/Plugins/Creating_a_plugin.md#CREATE-BUDDY-PLUGIN) - Устанавливает плагин Buddy
* [DROP PLUGIN](Extensions/UDFs_and_Plugins/Plugins/Deleting_a_plugin.md#DELETE-PLUGIN) - Удаляет плагин
* [DROP BUDDY PLUGIN](Extensions/UDFs_and_Plugins/Plugins/Deleting_a_plugin.md#DELETE-BUDDY-PLUGIN) - Удаляет плагин Buddy
* [RELOAD PLUGINS](Extensions/UDFs_and_Plugins/Plugins/Reloading_plugins.md) - Перезагружает все плагины из заданной библиотеки
* [ENABLE BUDDY PLUGIN](Extensions/UDFs_and_Plugins/Plugins/Enabling_and_disabling_buddy_plugins.md#ENABLE-BUDDY-PLUGIN) - Восстанавливает ранее отключённый плагин Buddy
* [DISABLE BUDDY PLUGIN](Extensions/UDFs_and_Plugins/Plugins/Enabling_and_disabling_buddy_plugins.md#DISABLE-BUDDY-PLUGIN) - Отключает активный плагин Buddy

##### Статус сервера
* [SHOW STATUS](Node_info_and_management/Node_status.md#SHOW-STATUS) - Отображает ряд полезных счётчиков производительности
* [SHOW THREADS](Node_info_and_management/SHOW_THREADS.md) - Перечисляет все в данный момент активные клиентские потоки
* [SHOW VARIABLES](Node_info_and_management/SHOW_VARIABLES.md) - Перечисляет переменные сервера и их значения
* [SHOW VERSION](Node_info_and_management/SHOW_VERSION.md#SHOW-VERSION) - Предоставляет подробную информацию о версиях различных компонентов экземпляра.

### HTTP endpoints
* [/sql](Connecting_to_the_server/HTTP.md#SQL-over-HTTP) - Выполнение SQL-запроса через HTTP JSON
* [/cli](Connecting_to_the_server/HTTP.md#/cli) - Предоставляет HTTP-интерфейс командной строки
* [/insert](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md) - Вставляет документ в таблицу реального времени
* [/pq/tbl_name/doc](Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md#Adding-rules-to-a-percolate-table) - Добавляет правило PQ в перколяционную таблицу
* [/update](Data_creation_and_modification/Updating_documents/UPDATE.md#Updates-via-HTTP-JSON) - Обновляет документ в таблице реального времени
* [/replace](Data_creation_and_modification/Updating_documents/REPLACE.md) - Заменяет существующий документ в таблице реального времени или вставляет его, если он отсутствует
* [/pq/tbl_name/doc/N?refresh=1](Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md#Adding-rules-to-a-percolate-table) - Заменяет правило PQ в перколяционной таблице
* [/delete](Data_creation_and_modification/Deleting_documents.md) - Удаляет документ из таблицы
* [/bulk](Data_creation_and_modification/Updating_documents/UPDATE.md#Bulk-updates) - Выполняет несколько операций вставки, обновления или удаления за один вызов. Подробнее о пакетной вставке [здесь](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md).
* [/search](Searching/Full_text_matching/Basic_usage.md#HTTP-JSON) - Выполняет поиск
* [/search -> knn](Searching/KNN.md) - Выполняет поиск по вектору KNN
* [/pq/tbl_name/search](Searching/Percolate_query.md) - Выполняет обратный поиск в перколяционной таблице
* [/tbl_name/_mapping](Creating_a_table/Local_tables/Real-time_table.md#_mapping-API:) - Создаёт схему таблицы в стиле Elasticsearch

### Общие сведения
* [синтаксис имени поля](Creating_a_table/Data_types.md#Field-name-syntax)
* [типы данных](Creating_a_table/Data_types.md)
* [движок](Creating_a_table/Data_types.md)
* [простой режим](Read_this_first.md#Real-time-mode-vs-plain-mode)
* [режим реального времени](Read_this_first.md#Real-time-mode-vs-plain-mode)

##### Общие настройки таблицы
* [access_plain_attrs](Server_settings/Searchd.md#access_plain_attrs)
* [access_blob_attrs](Server_settings/Searchd.md#access_blob_attrs)
* [access_doclists](Server_settings/Searchd.md#access_doclists)
* [access_hitlists](Server_settings/Searchd.md#access_hitlists)
* [access_dict](Server_settings/Searchd.md#access_dict)
* [attr_update_reserve](Data_creation_and_modification/Updating_documents/UPDATE.md#attr_update_reserve)
* [bigram_freq_words](Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#bigram_freq_words)
* [bigram_index](Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#bigram_index)
* [blend_chars](Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#blend_chars)
* [blend_mode](Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#blend_mode)
* [charset_table](Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table)
* [dict](Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)
* [docstore_block_size](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [docstore_compression](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [docstore_compression_level](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [embedded_limit](Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#embedded_limit)
* [exceptions](Creating_a_table/NLP_and_tokenization/Exceptions.md#exceptions)
* [expand_keywords](Searching/Options.md#expand_keywords)
* [global_idf](Searching/Options.md#global_idf)
* [hitless_words](Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#hitless_words)
* [html_index_attrs](Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#html_index_attrs)
* [html_remove_elements](Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#html_remove_elements)
* [html_strip](Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#html_strip)
* [ignore_chars](Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ignore_chars)
* [index_exact_words](Creating_a_table/NLP_and_tokenization/Morphology.md#index_exact_words)
* [index_field_lengths](Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#index_field_lengths)
* [index_sp](Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#index_sp)
* [index_token_filter](Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#index_token_filter)
* [index_zones](Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#index_zones)
* [infix_fields](Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#infix_fields)
* [inplace_enable](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [inplace_hit_gap](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [inplace_reloc_factor](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [inplace_write_factor](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [jieba_hmm](Creating_a_table/NLP_and_tokenization/Morphology.md#jieba_hmm)
* [jieba_mode](Creating_a_table/NLP_and_tokenization/Morphology.md#jieba_mode)
* [jieba_user_dict_path](Creating_a_table/NLP_and_tokenization/Morphology.md#jieba_user_dict_path)
* [killlist_target](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [max_substring_len](Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#max_substring_len)
* [min_infix_len](Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len)
* [min_prefix_len](Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_prefix_len)
* [min_stemming_len](Creating_a_table/NLP_and_tokenization/Morphology.md#min_stemming_len)
* [min_word_len](Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#min_word_len)
* [morphology](Searching/Options.md#morphology)
* [morphology_skip_fields](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology_skip_fields)
* [ngram_chars](Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_chars)
* [ngram_len](Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_len)
* [overshort_step](Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#overshort_step)
* [path](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [phrase_boundary](Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#phrase_boundary)
* [phrase_boundary_step](Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#phrase_boundary_step)
* [prefix_fields](Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#prefix_fields)
* [preopen](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [read_buffer_docs](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [read_buffer_hits](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [regexp_filter](Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#regexp_filter)
* [stopwords](Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopwords)
* [stopword_step](Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopword_step)
* [stopwords_unstemmed](Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopwords_unstemmed)
* [type](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [wordforms](Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms)

##### Plain table settings
* [json_secondary_indexes](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#json_secondary_indexes)
* [source](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [stored_fields](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [stored_only_fields](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [columnar_attrs](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)

##### Distributed table settings
* [local](Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md)
* [agent](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent)
* [agent_connect_timeout](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent)
* [agent_blackhole](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent)
* [agent_persistent](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent)
* [agent_query_timeout](Searching/Options.md#agent_query_timeout)
* [agent_retry_count](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent)
* [ha_strategy](Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_strategy)
* [mirror_retry_count](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent)

##### Настройки RT таблиц
* [rt_attr_bigint](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [rt_attr_bool](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [rt_attr_float](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [rt_attr_float_vector](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [rt_attr_json](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [rt_attr_multi_64](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [rt_attr_multi](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [rt_attr_string](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [rt_attr_timestamp](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [rt_attr_uint](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [rt_field](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [rt_mem_limit](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [diskchunk_flush_write_timeout](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [diskchunk_flush_search_timeout](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)


## Операторы полнотекстового поиска
* [OR](Searching/Full_text_matching/Operators.md#OR-operator)
* [MAYBE](Searching/Full_text_matching/Operators.md#MAYBE-operator)
* [NOT](Searching/Full_text_matching/Operators.md#Negation-operator) - оператор NOT
* [@field](Searching/Full_text_matching/Operators.md#Field-search-operator) - оператор поиска по полю
* [@field%5BN%5D](Searching/Full_text_matching/Operators.md#Field-search-operator) - модификатор ограничения позиции поля
* [@(field1,field2,...)](Searching/Full_text_matching/Operators.md#Field-search-operator) - оператор поиска по нескольким полям
* [@!field](Searching/Full_text_matching/Operators.md#Field-search-operator) - оператор игнорирования поля
* [@!(field1,field2,...)](Searching/Full_text_matching/Operators.md#Field-search-operator) - оператор игнорирования нескольких полей
* [@*](Searching/Full_text_matching/Operators.md#Field-search-operator) - оператор поиска по всем полям
* ["word1 word2 ... "](Searching/Full_text_matching/Operators.md#Phrase-search-operator) - оператор поиска фразы
* ["word1 word2 ... "~N](Searching/Full_text_matching/Operators.md#Proximity-search-operator) - оператор поиска по близости
* ["word1 word2 ... "/N](Searching/Full_text_matching/Operators.md#Quorum-matching-operator) - оператор поиска по кворуму
* [word1 << word2 << word3](Searching/Full_text_matching/Operators.md#Strict-order-operator) - оператор строгого порядка
* [=word1](Searching/Full_text_matching/Operators.md#Exact-form-modifier) - модификатор точной формы
* [^word1](Searching/Full_text_matching/Operators.md#Field-start-and-field-end-modifier) - модификатор начала поля
* [word2$](Searching/Full_text_matching/Operators.md#Field-start-and-field-end-modifier) - модификатор конца поля
* [word^N](Searching/Full_text_matching/Operators.md#IDF-boost-modifier) - модификатор повышения веса ключевого слова IDF
* [word1 NEAR/N word2](Searching/Full_text_matching/Operators.md#NEAR-operator) - NEAR, обобщённый оператор близости
* [word1 NOTNEAR/N word2](Searching/Full_text_matching/Operators.md#NOTNEAR-operator) - NOTNEAR, оператор отрицательного утверждения
* [word1 PARAGRAPH word2 PARAGRAPH "word3 word4"](Searching/Full_text_matching/Operators.md#SENTENCE-and-PARAGRAPH-operators) - оператор PARAGRAPH
* [word1 SENTENCE word2 SENTENCE "word3 word4"](Searching/Full_text_matching/Operators.md#SENTENCE-and-PARAGRAPH-operators) - оператор SENTENCE
* [ZONE:(h3,h4)](Searching/Full_text_matching/Operators.md#ZONE-limit-operator) - оператор ограничения ZONE
* [ZONESPAN:(h2)](Searching/Full_text_matching/Operators.md#ZONESPAN-limit-operator) - оператор ограничения ZONESPAN
* [@@relaxed](Searching/Full_text_matching/Operators.md#Field-search-operator) - подавляет ошибки об отсутствующих полях
* [t?st](Searching/Full_text_matching/Operators.md#Wildcard-operators) - операторы подстановки (wildcard)
* [REGEX(/pattern/)](Searching/Full_text_matching/Operators.md#REGEX-operator) - оператор REGEX

## Функции
##### Математические
* [ABS()](Functions/Mathematical_functions.md#ABS%28%29) - Возвращает абсолютное значение
* [ATAN2()](Functions/Mathematical_functions.md#ATAN2%28%29) - Возвращает арктангенс двух аргументов
* [BITDOT()](Functions/Mathematical_functions.md#BITDOT%28%29) - Возвращает сумму произведений каждого бита маски, умноженного на его вес
* [CEIL()](Functions/Mathematical_functions.md#CEIL%28%29) - Возвращает наименьшее целое число, большее или равное аргументу
* [COS()](Functions/Mathematical_functions.md#COS%28%29) - Возвращает косинус аргумента
* [CRC32()](Functions/Mathematical_functions.md#CRC32%28%29) - Возвращает значение CRC32 аргумента
* [EXP()](Functions/Mathematical_functions.md#EXP%28%29) - Возвращает экспоненту аргумента
* [FIBONACCI()](Functions/Mathematical_functions.md#FIBONACCI%28%29) - Возвращает N-е число Фибоначчи, где N — целочисленный аргумент
* [FLOOR()](Functions/Mathematical_functions.md#FLOOR%28%29) - Возвращает наибольшее целое число, меньшее или равное аргументу
* [GREATEST()](Functions/Mathematical_functions.md#GREATEST%28%29) - Принимает JSON/MVA массив в качестве аргумента и возвращает наибольшее значение в этом массиве
* [IDIV()](Functions/Mathematical_functions.md#IDIV%28%29) - Возвращает результат целочисленного деления первого аргумента на второй аргумент
* [LEAST()](Functions/Mathematical_functions.md#LEAST%28%29) - Принимает JSON/MVA массив в качестве аргумента и возвращает наименьшее значение в этом массиве
* [LN()](Functions/Mathematical_functions.md#LN%28%29) - Возвращает натуральный логарифм аргумента
* [LOG10()](Functions/Mathematical_functions.md#LOG10%28%29) - Возвращает десятичный логарифм аргумента
* [LOG2()](Functions/Mathematical_functions.md#LOG2%28%29) - Возвращает двоичный логарифм аргумента
* [MAX()](Functions/Mathematical_functions.md#MAX%28%29) - Возвращает большее из двух аргументов
* [MIN()](Functions/Mathematical_functions.md#MIN%28%29) - Возвращает меньшее из двух аргументов
* [POW()](Functions/Mathematical_functions.md#POW%28%29) - Возвращает первый аргумент, возведённый в степень второго аргумента
* [RAND()](Functions/Mathematical_functions.md#RAND%28%29) - Возвращает случайное число с плавающей точкой от 0 до 1
* [SIN()](Functions/Mathematical_functions.md#SIN%28%29) - Возвращает синус аргумента
* [SQRT()](Functions/Mathematical_functions.md#SQRT%28%29) - Возвращает квадратный корень аргумента


##### Поиск и ранжирование
* [BM25F()](Functions/Searching_and_ranking_functions.md#BM25F%28%29) - Возвращает точное значение формулы BM25F
* [EXIST()](Functions/Searching_and_ranking_functions.md#EXIST%28%29) - Заменяет несуществующие столбцы значениями по умолчанию
* [GROUP_CONCAT()](Searching/Grouping.md#GROUP_CONCAT%28field%29) - Формирует через запятую список значений атрибутов всех документов в группе
* [HIGHLIGHT()](Searching/Highlighting.md) - Выделяет результаты поиска
* [MIN_TOP_SORTVAL()](Functions/Searching_and_ranking_functions.md#MIN_TOP_SORTVAL%28%29) - Возвращает значение ключа сортировки худшего найденного элемента в текущих топ-N совпадениях
* [MIN_TOP_WEIGHT()](Functions/Searching_and_ranking_functions.md#MIN_TOP_WEIGHT%28%29) - Возвращает вес худшего найденного элемента в текущих топ-N совпадениях
* [PACKEDFACTORS()](Functions/Searching_and_ranking_functions.md#PACKEDFACTORS%28%29) - Выводит весовые коэффициенты
* [REMOVE_REPEATS()](Functions/Searching_and_ranking_functions.md#REMOVE_REPEATS%28%29) - Удаляет повторяющиеся скорректированные строки с одинаковым значением 'column'
* [WEIGHT()](Functions/Searching_and_ranking_functions.md#WEIGHT%28%29) - Возвращает оценку полнотекстового совпадения
* [ZONESPANLIST()](Functions/Searching_and_ranking_functions.md#ZONESPANLIST%28%29) - Возвращает пары совпадающих зоновых интервалов
* [QUERY()](Functions/Searching_and_ranking_functions.md#QUERY%28%29) - Возвращает текущий полнотекстовый запрос

##### Приведение типов
* [BIGINT()](Functions/Type_casting_functions.md#BIGINT%28%29) - Принудительно преобразует целочисленный аргумент в тип 64-битного целого
* [DOUBLE()](Functions/Type_casting_functions.md#DOUBLE%28%29) - Принудительно преобразует заданный аргумент к типу с плавающей точкой
* [INTEGER()](Functions/Type_casting_functions.md#INTEGER%28%29) - Принудительно преобразует заданный аргумент к знаковому 64-битному типу
* [TO_STRING()](Functions/Type_casting_functions.md#TO_STRING%28%29) - Принудительно преобразует аргумент к строковому типу
* [UINT()](Functions/Type_casting_functions.md#UINT%28%29) - Преобразует заданный аргумент в 32-битное беззнаковое целое число
* [UINT64()](Functions/Type_casting_functions.md#UINT64%28%29) - Преобразует заданный аргумент в 64-битное беззнаковое целое число
* [SINT()](Functions/Type_casting_functions.md#SINT%28%29) - Интерпретирует 32-битное беззнаковое целое как знаковое 64-битное целое

##### Массивы и условия
* [ALL()](Functions/Arrays_and_conditions_functions.md#ALL%28%29) - Возвращает 1, если условие истинно для всех элементов массива
* [ANY()](Functions/Arrays_and_conditions_functions.md#ANY%28%29) - Возвращает 1, если условие истинно для любого элемента массива
* [CONTAINS()](Functions/Arrays_and_conditions_functions.md#CONTAINS%28%29) - Проверяет, находится ли точка (x,y) внутри заданного полигона
* [IF()](Functions/Arrays_and_conditions_functions.md#IF%28%29) - Проверяет, равен ли первый аргумент 0.0, возвращает второй аргумент, если он не равен нулю, или третий, когда равен
* [IN()](Functions/Arrays_and_conditions_functions.md#IN%28%29) - Возвращает 1, если первый аргумент равен любому из остальных аргументов, иначе 0
* [INDEXOF()](Functions/Arrays_and_conditions_functions.md#INDEXOF%28%29) - Проходит по всем элементам массива и возвращает индекс первого совпадающего элемента
* [INTERVAL()](Functions/Arrays_and_conditions_functions.md#INTERVAL%28%29) - Возвращает индекс аргумента, который меньше первого аргумента
* [LENGTH()](Functions/Arrays_and_conditions_functions.md#LENGTH%28%29) - Возвращает число элементов в MVA
* [REMAP()](Functions/Arrays_and_conditions_functions.md#REMAP%28%29) - Позволяет сделать исключения из значений выражений в зависимости от значений условия

##### Дата и время
* [NOW()](Functions/Date_and_time_functions.md#NOW%28%29) - Возвращает текущую временную метку в виде INTEGER
* [CURTIME()](Functions/Date_and_time_functions.md#CURTIME%28%29) - Возвращает текущее время в локальном часовом поясе
* [CURDATE()](Functions/Date_and_time_functions.md#CURDATE%28%29) - Возвращает текущую дату в локальном часовом поясе
* [UTC_TIME()](Functions/Date_and_time_functions.md#UTC_TIME%28%29) - Возвращает текущее время в часовом поясе UTC
* [UTC_TIMESTAMP()](Functions/Date_and_time_functions.md#UTC_TIMESTAMP%28%29) - Возвращает текущие дату и время в часовом поясе UTC
* [SECOND()](Functions/Date_and_time_functions.md#SECOND%28%29) - Возвращает целочисленные секунды из аргумента временной метки
* [MINUTE()](Functions/Date_and_time_functions.md#MINUTE%28%29) - Возвращает целочисленные минуты из аргумента временной метки
* [HOUR()](Functions/Date_and_time_functions.md#HOUR%28%29) - Возвращает целочисленный час из аргумента временной метки
* [DAY()](Functions/Date_and_time_functions.md#DAY%28%29) - Возвращает целочисленный день из аргумента временной метки
* [MONTH()](Functions/Date_and_time_functions.md#MONTH%28%29) - Возвращает целочисленный месяц из аргумента временной метки
* [QUARTER()](Functions/Date_and_time_functions.md#QUARTER%28%29) - Возвращает целочисленный квартал года из аргумента временной метки
* [YEAR()](Functions/Date_and_time_functions.md#YEAR%28%29) - Возвращает целочисленный год из аргумента временной метки
* [DAYNAME()](Functions/Date_and_time_functions.md#DAYNAME%28%29) - Возвращает название дня недели для заданного аргумента временной метки
* [MONTHNAME()](Functions/Date_and_time_functions.md#MONTHNAME%28%29) - Возвращает название месяца для заданного аргумента временной метки
* [DAYOFWEEK()](Functions/Date_and_time_functions.md#DAYOFWEEK%28%29) - Возвращает целочисленный индекс дня недели для заданного аргумента временной метки
* [DAYOFYEAR()](Functions/Date_and_time_functions.md#DAYOFYEAR%28%29) - Возвращает целочисленный день года для заданного аргумента временной метки
* [YEARWEEK()](Functions/Date_and_time_functions.md#YEARWEEK%28%29) - Возвращает целочисленный год и код дня первого дня текущей недели для заданного аргумента временной метки
* [YEARMONTH()](Functions/Date_and_time_functions.md#YEARMONTH%28%29) - Возвращает целочисленные коды года и месяца из аргумента временной метки
* [YEARMONTHDAY()](Functions/Date_and_time_functions.md#YEARMONTHDAY%28%29) - Возвращает целочисленные коды года, месяца и дня из аргумента временной метки
* [TIMEDIFF()](Functions/Date_and_time_functions.md#TIMEDIFF%28%29) - Возвращает разницу между временными метками
* [DATEDIFF()](Functions/Date_and_time_functions.md#DATEDIFF%28%29) - Возвращает количество дней между двумя заданными временными метками
* [DATE()](Functions/Date_and_time_functions.md#DATE%28%29) - Форматирует часть даты из аргумента временной метки
* [TIME()](Functions/Date_and_time_functions.md#TIME%28%29) - Форматирует часть времени из аргумента временной метки
* [DATE_FORMAT()](Functions/Date_and_time_functions.md#DATE_FORMAT%28%29) - Возвращает отформатированную строку на основе предоставленных аргументов даты и формата


##### Геопространственные
* [GEODIST()](Functions/Geo_spatial_functions.md#GEODIST%28%29) - Вычисляет геосферное расстояние между двумя заданными точками
* [GEOPOLY2D()](Functions/Geo_spatial_functions.md#GEOPOLY2D%28%29) - Создаёт полигон с учётом кривизны Земли
* [POLY2D()](Functions/Geo_spatial_functions.md#POLY2D%28%29) - Создаёт простой полигон в плоском пространстве

##### Строковые
* [CONCAT()](Functions/String_functions.md#CONCAT%28%29) - Конкатенирует две или более строк
* [REGEX()](Functions/String_functions.md#REGEX%28%29) - Возвращает 1, если регулярное выражение совпало со строкой атрибута, иначе 0
* [SNIPPET()](Functions/String_functions.md#SNIPPET%28%29) - Подсвечивает результаты поиска
* [SUBSTRING_INDEX()](Functions/String_functions.md#SUBSTRING_INDEX%28%29) - Возвращает подстроку строки до указанного количества разделителей

##### Другие
* [CONNECTION_ID()](Functions/Other_functions.md#CONNECTION_ID%28%29) - Возвращает идентификатор текущего соединения
* [KNN_DIST()](Functions/Other_functions.md#KNN_DIST%28%29) - Возвращает расстояние для KNN векторного поиска
* [LAST_INSERT_ID()](Functions/Other_functions.md#LAST_INSERT_ID%28%29) - Возвращает ID документов, вставленных или заменённых последним запросом в текущей сессии
* [UUID_SHORT()](Functions/Other_functions.md#UUID_SHORT%28%29) - Возвращает "короткий" универсальный идентификатор, созданный по тому же алгоритму, что и генерация auto-id.

## Общие настройки в конфигурационном файле
Должны быть помещены в секцию `common {}` в конфигурационном файле:
* [lemmatizer_base](Server_settings/Common.md#lemmatizer_base) - Базовый путь к словарям лемматизатора
* [progressive_merge](Server_settings/Common.md#progressive_merge) - Определяет порядок слияния дисковых чанков в таблице реального времени
* [json_autoconv_keynames](Server_settings/Common.md#json_autoconv_keynames) - Включить и как автопреобразовывать имена ключей в JSON-атрибутах
* [json_autoconv_numbers](Server_settings/Common.md#json_autoconv_numbers) - Автоматически определять и преобразовывать возможные JSON-строки, представляющие числа, в числовые атрибуты
* [on_json_attr_error](Server_settings/Common.md#on_json_attr_error) - Действия при обнаружении ошибок формата JSON
* [plugin_dir](Server_settings/Common.md#plugin_dir) - Расположение динамических библиотек и UDF

## [Indexer](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments)
`indexer` — это инструмент для создания [plain tables](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments)

##### Настройки Indexer в конфигурационном файле
Должны быть помещены в секцию `indexer {}` в конфигурационном файле:
* [lemmatizer_cache](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - Размер кэша лемматизатора
* [max_file_field_buffer](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - Максимальный размер адаптивного буфера для файловых полей
* [max_iops](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - Максимальное количество операций ввода-вывода во время индексации в секунду
* [max_iosize](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - Максимально допустимый размер операции ввода-вывода
* [max_xmlpipe2_field](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - Максимально допустимый размер поля для источника типа XMLpipe2
* [mem_limit](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - Лимит использования оперативной памяти при индексации
* [on_file_field_error](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - Как обрабатывать ошибки ввода-вывода в полях файлов
* [write_buffer](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - Размер буфера записи
* [ignore_non_plain](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - Игнорировать предупреждения о не-простых таблицах

##### Параметры запуска индексатора
```bash
indexer [OPTIONS] [indexname1 [indexname2 [...]]]
```
* [--all](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - Перестраивает все таблицы из конфигурации
* [--buildstops](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - Анализирует источник таблицы как при индексировании данных, создавая список индексируемых терминов
* [--buildfreqs](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - Добавляет подсчёт частоты в таблицу для --buildstops
* [--config, -c](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - Указывает путь к файлу конфигурации
* [--dump-rows](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - Сохраняет строки, полученные из SQL-источника(ов), в указанный файл
* [--help](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - Показывает все доступные параметры
* [--keep-attrs](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - Позволяет повторно использовать существующие атрибуты при переиндексации
* [--keep-attrs-names](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - Указывает, какие атрибуты использовать повторно из существующей таблицы
* [--merge-dst-range](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - Применяет заданный диапазон фильтра при слиянии
* [--merge-killlists](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - Изменяет обработку списков удаления при слиянии таблиц
* [--merge](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - Объединяет две простые таблицы в одну
* [--nohup](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - Не позволяет индексатору отправлять сигнал SIGHUP при включении этой опции
* [--noprogress](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - Скрывает детали прогресса
* [--print-queries](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - Показывает SQL-запросы, отправляемые индексатором в базу данных
* [--print-rt](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - Отображает данные, полученные из SQL-источника(ов), в виде INSERT-запросов для таблицы в реальном времени
* [--quiet](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - Подавляет любой вывод
* [--rotate](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - Запускает ротацию таблиц после построения всех таблиц
* [--sighup-each](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - Запускает ротацию каждой таблицы после её построения
* [-v](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - Показывает версию индексатора

## Конвертер таблиц для Manticore v2 / Sphinx v2
`index_converter` — это инструмент для конвертации таблиц, созданных с помощью Sphinx/Manticore Search 2.x, в формат таблиц Manticore Search 3.x.
```bash
index_converter {--config /path/to/config|--path}
```
##### Параметры запуска конвертера таблиц
* [--config, -c](Installation/Migration_from_Sphinx.md#index_converter) - Путь к файлу конфигурации таблицы
* [--index](Installation/Migration_from_Sphinx.md#index_converter) - Указывает, какую таблицу конвертировать
* [--path](Installation/Migration_from_Sphinx.md#index_converter) - Устанавливает путь, содержащий таблицу(ы), вместо файла конфигурации
* [--strip-path](Installation/Migration_from_Sphinx.md#index_converter) - Удаляет путь из имён файлов, на которые ссылается таблица
* [--large-docid](Installation/Migration_from_Sphinx.md#index_converter) - Позволяет конвертировать документы с id больше чем 2^63
* [--output-dir](Installation/Migration_from_Sphinx.md#index_converter) - Записывает новые файлы в указанную папку
* [--all](Installation/Migration_from_Sphinx.md#index_converter) - Конвертирует все таблицы из файла конфигурации / пути
* [--killlist-target](Installation/Migration_from_Sphinx.md#index_converter) - Устанавливает целевые таблицы для применения списков удаления

## [Searchd](Starting_the_server/Manually.md)
`searchd` — сервер Manticore.

##### Настройки searchd в конфигурационном файле
Размещается в секции `searchd {}` конфигурационного файла:
  * [access_blob_attrs](Server_settings/Searchd.md#access_blob_attrs) - Определяет, как файл blob-атрибутов таблицы доступен
  * [access_doclists](Server_settings/Searchd.md#access_doclists) - Определяет, как файл списков документов таблицы доступен
  * [access_hitlists](Server_settings/Searchd.md#access_hitlists) - Определяет, как файл списков попаданий таблицы доступен
  * [access_plain_attrs](Server_settings/Searchd.md#access_plain_attrs) - Определяет, как поисковый сервер получает доступ к простым атрибутам таблицы
  * [access_dict](Server_settings/Searchd.md#access_dict) - Определяет способ доступа к файлу словаря таблицы
  * [agent_connect_timeout](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_connect_timeout) - Тайм-аут подключения к удалённому агенту
  * [agent_query_timeout](Searching/Options.md#agent_query_timeout) - Тайм-аут запроса к удалённому агенту
  * [agent_retry_count](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_connect_timeout) - Задает количество попыток подключения и запроса к удалённым агентам Manticore
  * [agent_retry_delay](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) - Задает задержку перед повторной попыткой запроса к удалённому агенту в случае неудачи
  * [attr_flush_period](Data_creation_and_modification/Updating_documents/UPDATE.md#attr_flush_period) - Определяет период между сбросами обновлённых атрибутов на диск
  * [binlog_flush](Server_settings/Searchd.md#binlog_flush) - Режим сброса/синхронизации транзакций бинарного лога
  * [binlog_max_log_size](Server_settings/Searchd.md#binlog_max_log_size) - Максимальный размер файла бинарного лога
  * [binlog_common](Logging/Binary_logging.md#Binary-logging-strategies) - Общий файл бинарного лога для всех таблиц
  * [binlog_filename_digits](Logging/Binary_logging.md#Log-files) - Количество цифр в имени файла бинарного лога
  * [binlog_flush](Logging/Binary_logging.md#Binary-flushing-strategies) - Стратегия сброса бинарного лога
  * [binlog_path](Server_settings/Searchd.md#binlog_path) - Путь к файлам бинарного лога
  * [client_timeout](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) - Максимальное время ожидания между запросами при использовании постоянных соединений
  * [collation_libc_locale](Server_settings/Searchd.md#collation_libc_locale) - Локаль libc сервера
  * [collation_server](Server_settings/Searchd.md#collation_server) - Коляция сервера по умолчанию
  * [data_dir](Server_settings/Searchd.md#data_dir) - Путь к каталогу данных, в котором Manticore хранит всё ([RT режим](Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29))
  * [diskchunk_flush_write_timeout](Server_settings/Searchd.md#diskchunk_flush_write_timeout) - Тайм-аут авто-сброса RAM чанка, если в него нет записей
  * [diskchunk_flush_search_timeout](Server_settings/Searchd.md#diskchunk_flush_search_timeout) - Тайм-аут предотвращения авто-сброса RAM чанка, если в таблице нет поисковых запросов
  * [docstore_cache_size](Server_settings/Searchd.md#docstore_cache_size) - Максимальный размер блоков документов из хранилища документов, удерживаемых в памяти
  * [expansion_limit](Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#expansion_limit) - Максимальное количество расширенных ключевых слов для одного шаблона с подстановкой
  * [grouping_in_utc](Server_settings/Searchd.md#grouping_in_utc) - Включает использование часового пояса UTC для группировки временных полей
  * [ha_period_karma](Server_settings/Searchd.md#ha_period_karma) - Размер окна статистики зеркал агентов
  * [ha_ping_interval](Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_ping_interval) - Интервал между пингами зеркал агентов
  * [hostname_lookup](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) - Стратегия обновления имён хостов
  * [jobs_queue_size](Server_settings/Searchd.md#jobs_queue_size) - Определяет максимальное количество одновременных «задач» в очереди
  * [join_batch_size](Searching/Joining.md#Join-batching) - Определяет размер пакета для объединений таблиц для балансировки производительности и использования памяти
  * [join_cache_size](Searching/Joining.md#Join-caching) - Определяет размер кеша для повторного использования результатов JOIN-запросов
  * [kibana_version_string](Server_settings/Searchd.md#kibana_version_string) – Строка версии сервера, отправляемая в ответ на запросы Kibana
  * [listen](Server_settings/Searchd.md#listen) - Определяет IP-адрес и порт или путь к Unix-доменному сокету, на которых searchd слушает подключения
  * [listen_backlog](Server_settings/Searchd.md#listen_backlog) - Очередь слушающих TCP соединений
  * [listen_tfo](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) - Включает флаг TCP_FASTOPEN для всех слушателей
  * [log](Server_settings/Searchd.md#log) - Путь к файлу логов сервера Manticore
  * [max_batch_queries](Server_settings/Searchd.md#max_batch_queries) - Ограничивает количество запросов в каждом пакете
  * [max_connections](Server_settings/Searchd.md#max_connections) - Максимальное количество активных соединений
  * [max_filters](Server_settings/Searchd.md#max_filters) - Максимальное количество фильтров в одном запросе
  * [max_filter_values](Server_settings/Searchd.md#max_filter_values) - Максимальное количество значений в одном фильтре
  * [max_open_files](Server_settings/Searchd.md#max_open_files) - Максимальное количество файлов, которое сервер может открыть одновременно
  * [max_packet_size](Server_settings/Searchd.md#max_packet_size) - Максимальный размер сетевого пакета
  * [mysql_version_string](Server_settings/Searchd.md#mysql_version_string) - Строка версии сервера, возвращаемая через протокол MySQL
  * [net_throttle_accept](Server_settings/Searchd.md#net_throttle_accept) - Определяет количество клиентов, принимаемых за каждую итерацию сетевого цикла
  * [net_throttle_action](Server_settings/Searchd.md#net_throttle_action)  - Определяет количество обрабатываемых запросов за каждую итерацию сетевого цикла
  * [net_wait_tm](Server_settings/Searchd.md#net_wait_tm) - Контролирует интервалы циклов ожидания сетевого потока
  * [net_workers](Server_settings/Searchd.md#net_workers) - Количество сетевых потоков
  * [network_timeout](Server_settings/Searchd.md#network_timeout) - Тайм-аут сети для клиентских запросов
  * [node_address](Server_settings/Searchd.md#node_address) - Указывает сетевой адрес узла
  * [persistent_connections_limit](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) - Максимальное количество одновременных постоянных соединений с удалёнными постоянными агентами
  * [pid_file](Server_settings/Searchd.md#pid_file) - Путь к PID-файлу сервера Manticore
  * [predicted_time_costs](Server_settings/Searchd.md#predicted_time_costs) - Стоимости для модели прогнозирования времени запроса
  * [preopen_tables](Server_settings/Searchd.md#preopen_tables) - Определяет, будут ли все таблицы принудительно предварительно открыты при запуске
  * [pseudo_sharding](Server_settings/Searchd.md#pseudo_sharding) - Включает псевдо-шардинг для поисковых запросов к обычным и real-time таблицам
  * [qcache_max_bytes](Server_settings/Searchd.md#qcache_max_bytes) - Максимальный объем ОЗУ, выделенный для кэшированных наборов результатов
  * [qcache_thresh_msec](Server_settings/Searchd.md#qcache_thresh_msec) - Минимальный порог времени в миллисекундах, при достижении которого результат запроса сохраняется в кэше
  * [qcache_ttl_sec](Server_settings/Searchd.md#qcache_ttl_sec) - Период действия кэшированного набора результатов
  * [query_log](Server_settings/Searchd.md#query_log) - Путь к файлу журнала запросов
  * [query_log_format](Server_settings/Searchd.md#query_log_format) - Формат журнала запросов
  * [query_log_min_msec](Server_settings/Searchd.md#query_log_min_msec) - Предотвращает логирование слишком быстрых запросов
  * [query_log_mode](Server_settings/Searchd.md#query_log_mode) - Режим прав доступа к файлу журнала запросов
  * [read_buffer_docs](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#read_buffer_docs) - Размер буфера чтения на ключевое слово для списков документов
  * [read_buffer_hits](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#read_buffer_docs) - Размер буфера чтения на ключевое слово для списков попаданий
  * [read_unhinted](Server_settings/Searchd.md#read_unhinted) - Размер ненаправленного чтения
  * [rt_flush_period](Server_settings/Searchd.md#rt_flush_period) - Как часто Manticore сбрасывает в диск чанки оперативной памяти real-time таблиц
  * [rt_merge_iops](Server_settings/Searchd.md#rt_merge_iops) - Максимальное количество операций ввода-вывода (в секунду), разрешенных потоку слияния чанкoв real-time таблиц
  * [rt_merge_maxiosize](Server_settings/Searchd.md#rt_merge_maxiosize) - Максимальный размер операции ввода-вывода, разрешенной потоку слияния чанкoв real-time таблиц
  * [seamless_rotate](Server_settings/Searchd.md#seamless_rotate) - Предотвращает зависания searchd при ротации таблиц с огромным количеством данных для предварительного кэширования
  * [secondary_indexes](Server_settings/Searchd.md#secondary_indexes) - Включает использование вторичных индексов для поисковых запросов
  * [server_id](Server_settings/Searchd.md#server_id) - Идентификатор сервера, используемый как зерно для генерации уникального ID документа
  * [shutdown_timeout](Server_settings/Searchd.md#shutdown_timeout) - Таймаут для `--stopwait` searchd
  * [shutdown_token](Server_settings/Searchd.md#shutdown_token) - SHA1-хэш пароля, необходимого для выполнения команды `shutdown` через VIP SQL соединение
  * [snippets_file_prefix](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) - Префикс, добавляемый к локальным именам файлов при генерации сниппетов в режиме `load_files`
  * [sphinxql_state](Server_settings/Searchd.md#sphinxql_state) - Путь к файлу, в котором будет сериализовано текущее состояние SQL
  * [sphinxql_timeout](Server_settings/Searchd.md#sphinxql_timeout) - Максимальное время ожидания между запросами от MySQL клиента
  * [ssl_ca](Server_settings/Searchd.md#ssl_ca) - Путь к файлу сертификата Центра Сертификации SSL
  * [ssl_cert](Server_settings/Searchd.md#ssl_cert) - Путь к SSL-сертификату сервера
  * [ssl_key](Server_settings/Searchd.md#ssl_key) - Путь к ключу SSL-сертификата сервера
  * [subtree_docs_cache](Server_settings/Searchd.md#subtree_docs_cache) - Максимальный размер кэша документов общего поддерева
  * [subtree_hits_cache](Server_settings/Searchd.md#subtree_hits_cache) - Максимальный размер кэша попаданий общего поддерева для каждого запроса
  * [timezone](Server_settings/Searchd.md#timezone) - Часовой пояс, используемый функциями даты и времени
  * [thread_stack](Server_settings/Searchd.md#thread_stack) - Максимальный размер стека для задачи
  * [unlink_old](Server_settings/Searchd.md#unlink_old) - Нужно ли разлинковывать .old копии таблиц после успешной ротации
  * [watchdog](Server_settings/Searchd.md#watchdog) - Включить или отключить сторожевой таймер сервера Manticore

##### Параметры запуска Searchd
```bash
searchd [OPTIONS]
```
* [--config, -c](Starting_the_server/Manually.md#searchd-command-line-options) - Указывает путь к конфигурационному файлу
* [--console](Starting_the_server/Manually.md#searchd-command-line-options) - Принудительно запускает сервер в консольном режиме
* [--coredump](Starting_the_server/Manually.md#searchd-command-line-options) - Включает сохранение дампа памяти при сбое
* [--cpustats](Starting_the_server/Manually.md#searchd-command-line-options) - Включает отчет о времени процессора
* [--delete](Starting_the_server/Manually.md#searchd-command-line-options) - Удаляет службу Manticore из Microsoft Management Console и других мест регистрации служб
* [--force-preread](Starting_the_server/Manually.md#searchd-command-line-options) - Запрещает серверу обслуживать входящие подключения, пока файлы таблиц не будут предварительно прочитаны
* [--help, -h](Starting_the_server/Manually.md#searchd-command-line-options) - Показывает все доступные параметры
* [--table (--index)](Starting_the_server/Manually.md#searchd-command-line-options) - Ограничивает сервер обслуживанием только указанной таблицы
* [--install](Starting_the_server/Manually.md#searchd-command-line-options) - Устанавливает searchd в качестве службы в Microsoft Management Console
* [--iostats](Starting_the_server/Manually.md#searchd-command-line-options) - Включает отчет об операциях ввода-вывода
* [--listen, -l](Starting_the_server/Manually.md#searchd-command-line-options) - Переопределяет [listen](Server_settings/Searchd.md#listen) из конфигурационного файла
* [--logdebug, --logdebugv, --logdebugvv](Starting_the_server/Manually.md#searchd-command-line-options) - Включает дополнительный вывод отладочной информации в лог сервера
* [--logreplication](Starting_the_server/Manually.md#searchd-command-line-options) - Включает дополнительный отладочный вывод репликации в лог сервера
* [--new-cluster](Starting_the_server/Manually.md#searchd-command-line-options) - Инициализирует кластер репликации и устанавливает сервер в качестве узла-эталона с защитой при [перезапуске кластера](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md)
* [--new-cluster-force](Starting_the_server/Manually.md#searchd-command-line-options) - Инициализирует кластер репликации и устанавливает сервер в качестве опорного узла, обходя защиту [перезапуска кластера](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md)
* [--nodetach](Starting_the_server/Manually.md#searchd-command-line-options) - Запускает searchd в форграунде
* [--ntservice](Starting_the_server/Manually.md#searchd-command-line-options) - Используется Microsoft Management Console для запуска searchd как службы на платформах Windows
* [--pidfile](Starting_the_server/Manually.md#searchd-command-line-options) - Переопределяет [pid_file](Server_settings/Searchd.md#pid_file) в файле конфигурации
* [--port, p](Starting_the_server/Manually.md#searchd-command-line-options) - Определяет порт, на котором searchd должен слушать, игнорируя порт, указанный в файле конфигурации
* [--replay-flags](Starting_the_server/Manually.md#searchd-command-line-options) - Устанавливает дополнительные опции воспроизведения бинарного лога
* [--servicename](Starting_the_server/Manually.md#searchd-command-line-options) - Назначает заданное имя searchd при установке или удалении службы, отображаемое в Microsoft Management Console
* [--status](Starting_the_server/Manually.md#searchd-command-line-options) - Запрашивает статус работающей службы поиска
* [--stop](Starting_the_server/Manually.md#searchd-command-line-options) - Останавливает сервер Manticore
* [--stopwait](Starting_the_server/Manually.md#searchd-command-line-options) - Корректно останавливает сервер Manticore
* [--strip-path](Starting_the_server/Manually.md#searchd-command-line-options) - Удаляет пути из всех указанных в таблице имён файлов
* [-v](Starting_the_server/Manually.md#searchd-command-line-options) - Показывает информацию о версии

##### Переменные окружения searchd
* [MANTICORE_TRACK_DAEMON_SHUTDOWN](Starting_the_server/Manually.md#Environment-variables) - Включает подробное логирование при завершении работы searchd

## [Indextool](Miscellaneous_tools.md#indextool)
Различные функции обслуживания таблиц, полезные для отладки.
```bash
indextool <command> [options]
```
##### Параметры запуска Indextool
Используется для дампа различной отладочной информации, связанной с физической таблицей.
```bash
indextool <command> [options]
```
* [--config, -c](Miscellaneous_tools.md#indextool) - Указывает путь к файлу конфигурации
* [--quiet, -q](Miscellaneous_tools.md#indextool) - Заставляет indextool работать тихо; без вывода баннера и прочего
* [--help, -h](Miscellaneous_tools.md#indextool) - Показывает все доступные параметры
* [-v](Miscellaneous_tools.md#indextool) - Показывает информацию о версии
* [Indextool](Miscellaneous_tools.md#indextool) - Проверяет файл конфигурации
* [--buildidf](Miscellaneous_tools.md#indextool) - Создаёт IDF файл из одного или нескольких дампов словаря
* [--build-infixes](Miscellaneous_tools.md#indextool) - Создаёт инфиксы для существующей таблицы dict=keywords
* [--dumpheader](Miscellaneous_tools.md#indextool) - Быстро выводит заголовочный файл указанной таблицы
* [--dumpconfig](Miscellaneous_tools.md#indextool) - Выводит определение таблицы из указанного заголовочного файла в почти совместимом формате manticore.conf
* [--dumpheader](Miscellaneous_tools.md#indextool) - Выводит заголовок таблицы по имени, производя поиск пути к заголовку в файле конфигурации
* [--dumpdict](Miscellaneous_tools.md#indextool) - Выводит словарь таблицы
* [--dumpdocids](Miscellaneous_tools.md#indextool) - Выводит ID документов по имени таблицы
* [--dumphitlist](Miscellaneous_tools.md#indextool) - Выводит все появления указанного ключевого слова/id в заданной таблице
* [--docextract](Miscellaneous_tools.md#indextool) - Запускает проверку таблицы на всём словаре/документах/хитах и собирает все слова и появления, относящиеся к заданному документу
* [--fold](Miscellaneous_tools.md#indextool) - Тестирует токенизацию на основе настроек таблицы
* [--htmlstrip](Miscellaneous_tools.md#indextool) - Фильтрует STDIN с использованием настроек HTML-стриппера для указанной таблицы
* [--mergeidf](Miscellaneous_tools.md#indextool) - Объединяет несколько .idf файлов в один
* [--morph](Miscellaneous_tools.md#indextool) - Применяет морфологию к входным данным STDIN и выводит результат на stdout
* [--check](Miscellaneous_tools.md#indextool) - Проверяет файлы данных таблицы на целостность
* [--check-id-dups](Miscellaneous_tools.md#indextool) - Проверяет наличие дублирующихся ID
* [--check-disk-chunk](Miscellaneous_tools.md#indextool) - Проверяет один диск-чанк таблицы RT
* [--strip-path](Miscellaneous_tools.md#indextool) - Удаляет пути из всех имён файлов, указанных в таблице
* [--rotate](Miscellaneous_tools.md#indextool) - Определяет, проверять ли таблицу, ожидающую ротации при использовании `--check`
* [--apply-killlists](Miscellaneous_tools.md#indextool) - Применяет списки исключений для всех таблиц, перечисленных в конфигурационном файле

## [Wordbreaker](Miscellaneous_tools.md#wordbreaker)
Разбивает сложные слова на составляющие.
```bash
wordbreaker [-dict path/to/dictionary_file] {split|test|bench}
```

##### Параметры запуска Wordbreaker
* [STDIN](Miscellaneous_tools.md#wordbreaker) - Принимает строку для разбиения на части
* [-dict](Miscellaneous_tools.md#wordbreaker) - Указывает файл словаря для использования
* [split|test|bench](Miscellaneous_tools.md#wordbreaker) - Указывает команду

## [Spelldump](Miscellaneous_tools.md#spelldump)
Извлекает содержимое файла словаря в формате ispell или MySpell

```bash
spelldump [options] <dictionary> <affix> [result] [locale-name]
```
* [dictionary](Miscellaneous_tools.md#spelldump) - Основной файл словаря
* [affix](Miscellaneous_tools.md#spelldump) - Аффиксный файл для словаря
* [result](Miscellaneous_tools.md#spelldump) - Указывает место вывода данных словаря
* [locale-name](Miscellaneous_tools.md#spelldump) - Указывает детали локали для использования

## Список зарезервированных ключевых слов

Полный алфавитный список ключевых слов, которые в настоящее время зарезервированы в синтаксисе Manticore SQL (следовательно, не могут использоваться как идентификаторы).

```
AND, AS, BY, COLUMNARSCAN, DISTINCT, DIV, DOCIDINDEX, EXPLAIN, FACET, FALSE, FORCE, FROM, IGNORE, IN, INDEXES, INNER, IS, JOIN, KNN, LEFT, LIMIT, MOD, NOT, NO_COLUMNARSCAN, NO_DOCIDINDEX, NO_SECONDARYINDEX, NULL, OFFSET, ON, OR, ORDER, RELOAD, SECONDARYINDEX, SELECT, SYSFILTERS, TRUE
```

## Документация по старым версиям Manticore

* [2.4.1](https://repo.manticoresearch.com/repository/old_docs/manticoresearch-2.4.1.pdf)
* [2.5.1](https://repo.manticoresearch.com/repository/old_docs/manticoresearch-2.5.1.pdf)
* [2.6.0](https://repo.manticoresearch.com/repository/old_docs/manticoresearch-2.6.0.pdf)
* [2.6.1](https://repo.manticoresearch.com/repository/old_docs/manticoresearch-2.6.1.pdf)
* [2.6.2](https://repo.manticoresearch.com/repository/old_docs/manticoresearch-2.6.2.pdf)
* [2.6.3](https://repo.manticoresearch.com/repository/old_docs/manticoresearch-2.6.3.pdf)
* [2.6.4](https://repo.manticoresearch.com/repository/old_docs/manticoresearch-2.6.4.pdf)
* [2.7.0](https://repo.manticoresearch.com/repository/old_docs/manticoresearch-2.7.0.pdf)
* [2.7.1](https://repo.manticoresearch.com/repository/old_docs/manticoresearch-2.7.1.pdf)
* [2.7.2](https://repo.manticoresearch.com/repository/old_docs/manticoresearch-2.7.2.pdf)
* [2.7.3](https://repo.manticoresearch.com/repository/old_docs/manticoresearch-2.7.3.pdf)
* [2.7.4](https://repo.manticoresearch.com/repository/old_docs/manticoresearch-2.7.4.pdf)
* [2.7.5](https://repo.manticoresearch.com/repository/old_docs/manticoresearch-2.7.5.pdf)
* [2.8.0](https://repo.manticoresearch.com/repository/old_docs/manticoresearch-2.8.0.pdf)
* [2.8.1](https://repo.manticoresearch.com/repository/old_docs/manticoresearch-2.8.1.pdf)
* [2.8.2](https://repo.manticoresearch.com/repository/old_docs/manticoresearch-2.8.2.pdf)
* [3.0.0](https://repo.manticoresearch.com/repository/old_docs/manticoresearch-3.0.0.pdf)
* [3.0.2](https://repo.manticoresearch.com/repository/old_docs/manticoresearch-3.0.2.pdf)
* [3.1.0](https://repo.manticoresearch.com/repository/old_docs/manticoresearch-3.1.0.pdf)
* [3.1.2](https://repo.manticoresearch.com/repository/old_docs/manticoresearch-3.1.2.pdf)
* [3.2.0](https://repo.manticoresearch.com/repository/old_docs/manticoresearch-3.2.0.pdf)
* [3.2.2](https://repo.manticoresearch.com/repository/old_docs/manticoresearch-3.2.2.pdf)
* [3.3.0](https://repo.manticoresearch.com/repository/old_docs/manticoresearch-3.3.0.pdf)
* [3.4.0](https://repo.manticoresearch.com/repository/old_docs/manticoresearch-3.4.0.pdf)
* [3.4.2](https://repo.manticoresearch.com/repository/old_docs/manticoresearch-3.4.2.pdf)
* [3.5.0](https://manual.manticoresearch.com/manticore-3-5-0/)
* [3.5.2](https://manual.manticoresearch.com/manticore-3-5-2/)
* [3.5.4](https://manual.manticoresearch.com/manticore-3-5-4/)
* [4.0.2](https://manual.manticoresearch.com/manticore-4-0-2/)
* [4.2.0](https://manual.manticoresearch.com/manticore-4-2-0/)
* [5.0.2](https://manual.manticoresearch.com/manticore-5-0-2/). [Installation page](https://manticoresearch.com/install-5.0.0/)
* [6.0.0](https://manual.manticoresearch.com/manticore-6-0-0/). [Installation page](https://manticoresearch.com/install-6.0.0/)
* [6.0.2](https://manual.manticoresearch.com/manticore-6-0-2/). [Installation page](https://manticoresearch.com/install-6.0.2/)
* [6.0.4](https://manual.manticoresearch.com/manticore-6-0-4/). [Installation page](https://manticoresearch.com/install-6.0.4/)
* [6.2.0](https://manual.manticoresearch.com/manticore-6-2-0/). [Installation page](https://manticoresearch.com/install-6.2.0/)
* [6.2.12](https://manual.manticoresearch.com/manticore-6-2-12/). [Installation page](https://manticoresearch.com/install-6.2.12/)
* [6.3.0](https://manual.manticoresearch.com/manticore-6-3-0/). [Installation page](https://manticoresearch.com/install-6.3.0/)
* [6.3.2](https://manual.manticoresearch.com/manticore-6-3-2/). [Installation page](https://manticoresearch.com/install-6.3.2/)
* [6.3.4](https://manual.manticoresearch.com/manticore-6-3-4/). [Installation page](https://manticoresearch.com/install-6.3.4/)
* [6.3.6](https://manual.manticoresearch.com/manticore-6-3-6/). [Installation page](https://manticoresearch.com/install-6.3.6/)
* [6.3.8](https://manual.manticoresearch.com/manticore-6-3-8/). [Installation page](https://manticoresearch.com/install-6.3.8/)
* [7.0.0](https://manual.manticoresearch.com/manticore-7-0-0/). [Installation page](https://manticoresearch.com/install-7.0.0/)
* [7.4.6](https://manual.manticoresearch.com/manticore-7-4-6/). [Installation page](https://manticoresearch.com/install-7.4.6/)
* [9.2.14](https://manual.manticoresearch.com/manticore-9-2-14/). [Installation page](https://manticoresearch.com/install-9.2.14/)
* [9.3.2](https://manual.manticoresearch.com/manticore-9-3-2/). [Installation page](https://manticoresearch.com/install-9.3.2/)
* [10.1.0](https://manual.manticoresearch.com/manticore-10-1-0/). [Installation page](https://manticoresearch.com/install-10.1.0/)
<!-- proofread -->

