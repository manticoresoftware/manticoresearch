# Ссылки

### SQL команды
##### Управление схемами
* [CREATE TABLE](Creating_a_table/Local_tables/Real-time_table.md#CREATE-TABLE-command:) - Создает новую таблицу
* [CREATE TABLE LIKE](Creating_a_table/Local_tables/Real-time_table.md#CREATE-TABLE-LIKE:) - Создает таблицу, используя другую в качестве шаблона
* [CREATE TABLE LIKE ... WITH DATA](Creating_a_table/Local_tables/Real-time_table.md#CREATE-TABLE-LIKE:) - Копирует таблицу
* [CREATE SOURCE](Integration/Kafka.md#Source) - Создает источник потребителя Kafka
* [CREATE MATERIALIZED VIEW](Integration/Kafka.md#Materialized-view) - Преобразование данных из сообщений Kafka
* [CREATE MV](Integration/Kafka.md#Materialized-view) - То же, что и предыдущая команда
* [DESCRIBE](Listing_tables.md#DESCRIBE) - Выводит список полей таблицы и их типы
* [ALTER TABLE](Updating_table_schema_and_settings.md) - Изменяет схему / настройки таблицы
* [ALTER TABLE REBUILD SECONDARY](Updating_table_schema_and_settings.md#Rebuilding-a-secondary-index) - Обновляет/восстанавливает вторичные индексы
* [ALTER TABLE type='distributed'](Updating_table_schema_and_settings.md#Changing-a-distributed-table) - Обновляет/восстанавливает вторичные индексы
* [ALTER TABLE RENAME](Updating_table_schema_and_settings.md#Renaming-a-real-time-table)
* [ALTER MATERIALIZED VIEW {name} suspended=1](Integration/Kafka.md#Altering-materialized-views) - Приостанавливает или возобновляет потребление из источника Kafka
* [DROP TABLE IF EXISTS](Deleting_a_table.md#Deleting-a-table) - Удаляет таблицу (если существует)
* [SHOW TABLES](Listing_tables.md#DESCRIBE) - Показывает список таблиц
* [SHOW SOURCES](Integration/Kafka.md#Listing) - Показывает список источников Kafka
* [SHOW MATERIALIZED VIEWS](Integration/Kafka.md#Listing) - Показывает список материализованных представлений
* [SHOW MVS](Integration/Kafka.md#Listing) - Псевдоним предыдущей команды
* [SHOW CREATE TABLE](Listing_tables.md#DESCRIBE) - Показывает SQL команду создания таблицы
* [SHOW TABLE INDEXES](Node_info_and_management/Table_settings_and_status/SHOW_TABLE_INDEXES.md) - Отображает информацию о доступных вторичных индексах таблицы
* [SHOW TABLE STATUS](Node_info_and_management/Table_settings_and_status/SHOW_TABLE_STATUS.md) - Показывает информацию о текущем статусе таблицы
* [SHOW TABLE SETTINGS](Node_info_and_management/Table_settings_and_status/SHOW_TABLE_SETTINGS.md) - Показывает настройки таблицы
* [SHOW LOCKS](Securing_and_compacting_a_table/Freezing_a_table.md#SHOW-LOCKS) - Показывает информацию о замороженных таблицах

##### Управление данными
* [INSERT](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md) - Добавляет новые документы
* [REPLACE](Data_creation_and_modification/Updating_documents/REPLACE.md) - Заменяет существующие документы новыми
* [REPLACE .. SET](Data_creation_and_modification/Updating_documents/REPLACE.md?client=REPLACE+SET) - Заменяет одно или несколько полей в таблице
* [UPDATE](Data_creation_and_modification/Updating_documents/UPDATE.md) - Выполняет обновление документов на месте
* [DELETE](Data_creation_and_modification/Deleting_documents.md) - Удаляет документы
* [TRUNCATE TABLE](Emptying_a_table.md) - Удаляет все документы из таблицы

##### Резервное копирование
* [BACKUP](Securing_and_compacting_a_table/Backup_and_restore.md#BACKUP-SQL-command-reference) - Создает резервные копии таблиц

##### SELECT
* [SELECT](Searching/Full_text_matching/Basic_usage.md#SQL) - Поиск
  * [WHERE](Searching/Filters.md#Filters) - Фильтры
  * [GROUP BY](Searching/Grouping.md) - Группировка результатов поиска
  * [GROUP BY ORDER](Searching/Grouping.md) - Сортировка групп
  * [GROUP BY HAVING](Searching/Grouping.md) - Фильтрация групп
  * [OPTION](Searching/Options.md#OPTION) - Опции запроса
  * [FACET](Searching/Faceted_search.md) - Фасетный поиск
  * [SUB-SELECTS](Searching/Sub-selects.md) - О использовании SELECT подзапросов
  * [JOIN](Searching/Joining.md) - Объединение таблиц в SELECT
* [EXPLAIN QUERY](Searching/Full_text_matching/Profiling.md#Profiling-without-running-a-query) - Показывает план выполнения запроса без его запуска
* [SHOW META](Node_info_and_management/SHOW_META.md) - Показывает расширенную информацию о выполненном запросе
* [SHOW PROFILE](Node_info_and_management/Profiling/Query_profile.md) - Показывает профилирование выполненного запроса
* [SHOW PLAN](Searching/Full_text_matching/Profiling.md#Profiling-the-query-tree-in-SQL) - Показывает план выполнения запроса после его выполнения
* [SHOW WARNINGS](Node_info_and_management/SHOW_WARNINGS.md) - Показывает предупреждения последнего запроса

##### Сброс различных данных
* [FLUSH ATTRIBUTES](Securing_and_compacting_a_table/Flushing_attributes.md) - Принудительно сбрасывает обновленные атрибуты на диск
* [FLUSH HOSTNAMES](Securing_and_compacting_a_table/Flushing_hostnames.md) - Обновляет IP, связанные с именами хостов агентов
* [FLUSH LOGS](Logging/Rotating_query_and_server_logs.md) - Инициирует переоткрытие файлов журналов searchd и запросов (аналогично USR1)

##### Оптимизация таблиц реального времени
* [FLUSH RAMCHUNK](Securing_and_compacting_a_table/Flushing_RAM_chunk_to_a_new_disk_chunk.md#FLUSH-RAMCHUNK) - Форсирует создание нового дискового чанка
* [FLUSH TABLE](Securing_and_compacting_a_table/Flushing_RAM_chunk_to_disk.md#FLUSH-TABLE) - Сбрасывает RAM чанки таблицы реального времени на диск
* [OPTIMIZE TABLE](Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE) - Добавляет таблицу реального времени в очередь на оптимизацию

##### Импорт в таблицу реального времени
* [ATTACH TABLE](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Attaching_one_table_to_another.md) - Перемещает данные из обычной таблицы в таблицу реального времени
* [IMPORT TABLE](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Importing_table.md) - Импортирует ранее созданную RT или PQ таблицу в сервер, работающий в режиме RT

##### Репликация
* [JOIN CLUSTER](Creating_a_cluster/Setting_up_replication/Joining_a_replication_cluster.md) - Присоединяется к кластеру репликации
* [ALTER CLUSTER](Creating_a_cluster/Setting_up_replication/Managing_replication_nodes.md) - Добавляет/удаляет таблицу из кластера репликации
* [SET CLUSTER](Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Cluster-parameters) - Изменяет настройки кластера репликации
* [DELETE CLUSTER](Creating_a_cluster/Setting_up_replication/Deleting_a_replication_cluster.md) - Удаляет кластер репликации

##### Plain table rotate
* [RELOAD TABLE](Data_creation_and_modification/Adding_data_from_external_storages/Rotating_a_table.md#RELOAD-TABLE) - Переходит к следующему состоянию plain-таблицы
* [RELOAD TABLES](Data_creation_and_modification/Adding_data_from_external_storages/Rotating_a_table.md#RELOAD-TABLE) - Переходит к следующему состоянию для всех plain-таблиц

##### Transactions
* [BEGIN](Data_creation_and_modification/Transactions.md#BEGIN,-COMMIT,-and-ROLLBACK) - Начинает транзакцию
* [COMMIT](Data_creation_and_modification/Transactions.md#BEGIN,-COMMIT,-and-ROLLBACK) - Завершает транзакцию
* [ROLLBACK](Data_creation_and_modification/Transactions.md#BEGIN,-COMMIT,-and-ROLLBACK) - Отменяет транзакцию

##### CALL
* [CALL SUGGEST, CALL QSUGGEST](Searching/Spell_correction.md#CALL-QSUGGEST,-CALL-SUGGEST) - Предлагает исправленные варианты написания слов
* [CALL SNIPPETS](Searching/Highlighting.md) - Создает выделенный сниппет результатов из предоставленных данных и запроса
* [CALL PQ](Searching/Percolate_query.md) - Выполняет перколяционный запрос
* [CALL KEYWORDS](Searching/Autocomplete.md#CALL-KEYWORDS) - Используется для проверки, как ключевые слова токенизируются. Также позволяет получить токенизированные формы предоставленных ключевых слов
* [CALL AUTOCOMPLETE](Searching/Autocomplete.md#CALL-AUTOCOMPLETE) - Автозаполняет ваш поисковый запрос

##### Plugins
* [CREATE FUNCTION](Extensions/UDFs_and_Plugins/UDF/Creating_a_function.md) - Устанавливает функцию, определенную пользователем (UDF)
* [DROP FUNCTION](Extensions/UDFs_and_Plugins/UDF/Deleting_a_function.md) - Удаляет функцию, определенную пользователем (UDF)
* [CREATE PLUGIN](Extensions/UDFs_and_Plugins/Plugins/Creating_a_plugin.md) - Устанавливает плагин
* [CREATE BUDDY PLUGIN](Extensions/UDFs_and_Plugins/Plugins/Creating_a_plugin.md#CREATE-BUDDY-PLUGIN) - Устанавливает Buddy-плагин
* [DROP PLUGIN](Extensions/UDFs_and_Plugins/Plugins/Deleting_a_plugin.md#DELETE-PLUGIN) - Удаляет плагин
* [DROP BUDDY PLUGIN](Extensions/UDFs_and_Plugins/Plugins/Deleting_a_plugin.md#DELETE-BUDDY-PLUGIN) - Удаляет Buddy-плагин
* [RELOAD PLUGINS](Extensions/UDFs_and_Plugins/Plugins/Reloading_plugins.md) - Перезагружает все плагины из данной библиотеки
* [ENABLE BUDDY PLUGIN](Extensions/UDFs_and_Plugins/Plugins/Enabling_and_disabling_buddy_plugins.md#ENABLE-BUDDY-PLUGIN) - Вновь активирует ранее отключенный Buddy-плагин
* [DISABLE BUDDY PLUGIN](Extensions/UDFs_and_Plugins/Plugins/Enabling_and_disabling_buddy_plugins.md#DISABLE-BUDDY-PLUGIN) - Деактивирует активный Buddy-плагин

##### Server status
* [SHOW STATUS](Node_info_and_management/Node_status.md#SHOW-STATUS) - Отображает ряд полезных счетчиков производительности
* [SHOW THREADS](Node_info_and_management/SHOW_THREADS.md) - Показывает все текущие активные клиентские потоки
* [SHOW VARIABLES](Node_info_and_management/SHOW_VARIABLES.md) - Показывает серверные переменные и их значения
* [SHOW VERSION](Node_info_and_management/SHOW_VERSION.md#SHOW-VERSION) - Предоставляет подробную информацию о версиях различных компонентов экземпляра.

### HTTP endpoints
* [/sql](Connecting_to_the_server/HTTP.md#SQL-over-HTTP) - Выполнение SQL-запроса через HTTP JSON
* [/cli](Connecting_to_the_server/HTTP.md#/cli) - Предоставляет HTTP CLI (интерфейс командной строки)
* [/insert](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md) - Вставляет документ в таблицу реального времени
* [/pq/tbl_name/doc](Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md#Adding-rules-to-a-percolate-table) - Добавляет правило PQ в перколяционную таблицу
* [/update](Data_creation_and_modification/Updating_documents/UPDATE.md#Updates-via-HTTP-JSON) - Обновляет документ в таблице реального времени
* [/replace](Data_creation_and_modification/Updating_documents/REPLACE.md) - Заменяет существующий документ в таблице реального времени или вставляет его, если он не существует
* [/pq/tbl_name/doc/N?refresh=1](Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md#Adding-rules-to-a-percolate-table) - Заменяет правило PQ в перколяционной таблице
* [/delete](Data_creation_and_modification/Deleting_documents.md) - Удаляет документ из таблицы
* [/bulk](Data_creation_and_modification/Updating_documents/UPDATE.md#Bulk-updates) - Выполняет несколько операций вставки, обновления или удаления за один вызов. Подробнее о пакетной вставке читайте [здесь](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md).
* [/search](Searching/Full_text_matching/Basic_usage.md#HTTP-JSON) - Выполняет поиск
* [/search -> knn](Searching/KNN.md) - Выполняет поиск KNN по векторам
* [/pq/tbl_name/search](Searching/Percolate_query.md) - Выполняет обратный поиск в перколяционной таблице
* [/tbl_name/_mapping](Creating_a_table/Local_tables/Real-time_table.md#_mapping-API:) - Создает схему таблицы в стиле Elasticsearch

### Common things
* [field name syntax](Creating_a_table/Data_types.md#Field-name-syntax)
* [data types](Creating_a_table/Data_types.md)
* [engine](Creating_a_table/Data_types.md)
* [plain mode](Read_this_first.md#Real-time-mode-vs-plain-mode)
* [real-time mode](Read_this_first.md#Real-time-mode-vs-plain-mode)

##### Common table settings
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

##### Настройки Plain table
* [json_secondary_indexes](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#json_secondary_indexes)
* [source](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [stored_fields](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [stored_only_fields](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [columnar_attrs](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)

##### Настройки Distributed table
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
* ["word1 word2 ... "](Searching/Full_text_matching/Operators.md#Phrase-search-operator) - оператор поиска по фразе
* ["word1 word2 ... "~N](Searching/Full_text_matching/Operators.md#Proximity-search-operator) - оператор поиска с близостью
* ["word1 word2 ... "/N](Searching/Full_text_matching/Operators.md#Quorum-matching-operator) - оператор кворума
* [word1 << word2 << word3](Searching/Full_text_matching/Operators.md#Strict-order-operator) - оператор строгого порядка
* [=word1](Searching/Full_text_matching/Operators.md#Exact-form-modifier) - модификатор точной формы
* [^word1](Searching/Full_text_matching/Operators.md#Field-start-and-field-end-modifier) - модификатор начала поля
* [word2$](Searching/Full_text_matching/Operators.md#Field-start-and-field-end-modifier) - модификатор конца поля
* [word^N](Searching/Full_text_matching/Operators.md#IDF-boost-modifier) - модификатор усиления IDF для ключевого слова
* [word1 NEAR/N word2](Searching/Full_text_matching/Operators.md#NEAR-operator) - NEAR, обобщённый оператор близости
* [word1 NOTNEAR/N word2](Searching/Full_text_matching/Operators.md#NOTNEAR-operator) - NOTNEAR, оператор отрицательной близости
* [word1 PARAGRAPH word2 PARAGRAPH "word3 word4"](Searching/Full_text_matching/Operators.md#SENTENCE-and-PARAGRAPH-operators) - оператор PARAGRAPH
* [word1 SENTENCE word2 SENTENCE "word3 word4"](Searching/Full_text_matching/Operators.md#SENTENCE-and-PARAGRAPH-operators) - оператор SENTENCE
* [ZONE:(h3,h4)](Searching/Full_text_matching/Operators.md#ZONE-limit-operator) - оператор ограничения ZONE
* [ZONESPAN:(h2)](Searching/Full_text_matching/Operators.md#ZONESPAN-limit-operator) - оператор ограничения ZONESPAN
* [@@relaxed](Searching/Full_text_matching/Operators.md#Field-search-operator) - подавляет ошибки о пропущенных полях
* [t?st](Searching/Full_text_matching/Operators.md#Wildcard-operators) - операторы с подстановочными знаками
* [REGEX(/pattern/)](Searching/Full_text_matching/Operators.md#REGEX-operator) - оператор REGEX

## Функции
##### Математические
* [ABS()](Functions/Mathematical_functions.md#ABS%28%29) - Возвращает абсолютное значение
* [ATAN2()](Functions/Mathematical_functions.md#ATAN2%28%29) - Возвращает арктангенс двух аргументов
* [BITDOT()](Functions/Mathematical_functions.md#BITDOT%28%29) - Возвращает сумму произведений каждого бита маски на его вес
* [CEIL()](Functions/Mathematical_functions.md#CEIL%28%29) - Возвращает наименьшее целое число, не меньшее аргумента
* [COS()](Functions/Mathematical_functions.md#COS%28%29) - Возвращает косинус аргумента
* [CRC32()](Functions/Mathematical_functions.md#CRC32%28%29) - Возвращает значение CRC32 аргумента
* [EXP()](Functions/Mathematical_functions.md#EXP%28%29) - Возвращает экспоненту аргумента
* [FIBONACCI()](Functions/Mathematical_functions.md#FIBONACCI%28%29) - Возвращает N-е число Фибоначчи, где N — целочисленный аргумент
* [FLOOR()](Functions/Mathematical_functions.md#FLOOR%28%29) - Возвращает наибольшее целое число, меньшее или равное аргументу
* [GREATEST()](Functions/Mathematical_functions.md#GREATEST%28%29) - Принимает JSON/MVA массив в качестве аргумента и возвращает наибольшее значение в этом массиве
* [IDIV()](Functions/Mathematical_functions.md#IDIV%28%29) - Возвращает результат целочисленного деления первого аргумента на второй
* [LEAST()](Functions/Mathematical_functions.md#LEAST%28%29) - Принимает JSON/MVA массив в качестве аргумента и возвращает наименьшее значение в этом массиве
* [LN()](Functions/Mathematical_functions.md#LN%28%29) - Возвращает натуральный логарифм аргумента
* [LOG10()](Functions/Mathematical_functions.md#LOG10%28%29) - Возвращает десятичный логарифм аргумента
* [LOG2()](Functions/Mathematical_functions.md#LOG2%28%29) - Возвращает двоичный логарифм аргумента
* [MAX()](Functions/Mathematical_functions.md#MAX%28%29) - Возвращает большее из двух аргументов
* [MIN()](Functions/Mathematical_functions.md#MIN%28%29) - Возвращает меньшее из двух аргументов
* [POW()](Functions/Mathematical_functions.md#POW%28%29) - Возвращает первый аргумент, возведённый в степень второго аргумента
* [RAND()](Functions/Mathematical_functions.md#RAND%28%29) - Возвращает случайное число с плавающей запятой от 0 до 1
* [SIN()](Functions/Mathematical_functions.md#SIN%28%29) - Возвращает синус аргумента
* [SQRT()](Functions/Mathematical_functions.md#SQRT%28%29) - Возвращает квадратный корень аргумента


##### Поиск и ранжирование
* [BM25F()](Functions/Searching_and_ranking_functions.md#BM25F%28%29) - Возвращает точное значение формулы BM25F
* [EXIST()](Functions/Searching_and_ranking_functions.md#EXIST%28%29) - Заменяет несуществующие столбцы значениями по умолчанию
* [GROUP_CONCAT()](Searching/Grouping.md#GROUP_CONCAT%28field%29) - Формирует список через запятую значений атрибута всех документов в группе
* [HIGHLIGHT()](Searching/Highlighting.md) - Подсвечивает результаты поиска
* [MIN_TOP_SORTVAL()](Functions/Searching_and_ranking_functions.md#MIN_TOP_SORTVAL%28%29) - Возвращает значение ключа сортировки наихудшего найденного элемента в текущем топ-N совпадений
* [MIN_TOP_WEIGHT()](Functions/Searching_and_ranking_functions.md#MIN_TOP_WEIGHT%28%29) - Возвращает вес наихудшего найденного элемента в текущем топ-N совпадений
* [PACKEDFACTORS()](Functions/Searching_and_ranking_functions.md#PACKEDFACTORS%28%29) - Выводит весовые коэффициенты
* [REMOVE_REPEATS()](Functions/Searching_and_ranking_functions.md#REMOVE_REPEATS%28%29) - Удаляет повторяющиеся отрегулированные строки с одинаковым значением 'column'
* [WEIGHT()](Functions/Searching_and_ranking_functions.md#WEIGHT%28%29) - Возвращает оценку совпадения полнотекстового поиска
* [ZONESPANLIST()](Functions/Searching_and_ranking_functions.md#ZONESPANLIST%28%29) - Возвращает пары совпадающих зон
* [QUERY()](Functions/Searching_and_ranking_functions.md#QUERY%28%29) - Возвращает текущий полнотекстовый запрос

##### Приведение типов
* [BIGINT()](Functions/Type_casting_functions.md#BIGINT%28%29) - Принудительно преобразует целочисленный аргумент к 64-битному типу
* [DOUBLE()](Functions/Type_casting_functions.md#DOUBLE%28%29) - Принудительно преобразует данный аргумент к типу с плавающей точкой
* [INTEGER()](Functions/Type_casting_functions.md#INTEGER%28%29) - Принудительно преобразует данный аргумент к 64-битному знаковому типу
* [TO_STRING()](Functions/Type_casting_functions.md#TO_STRING%28%29) - Принудительно преобразует аргумент к строковому типу
* [UINT()](Functions/Type_casting_functions.md#UINT%28%29) - Преобразует данный аргумент к 32-битному беззнаковому целому числу
* [UINT64()](Functions/Type_casting_functions.md#UINT64%28%29) - Преобразует данный аргумент к 64-битному беззнаковому целому числу
* [SINT()](Functions/Type_casting_functions.md#SINT%28%29) - Интерпретирует 32-битное беззнаковое целое число как знаковое 64-битное число

##### Массивы и условия
* [ALL()](Functions/Arrays_and_conditions_functions.md#ALL%28%29) - Возвращает 1, если условие истинно для всех элементов массива
* [ANY()](Functions/Arrays_and_conditions_functions.md#ANY%28%29) - Возвращает 1, если условие истинно для любого элемента массива
* [CONTAINS()](Functions/Arrays_and_conditions_functions.md#CONTAINS%28%29) - Проверяет, находится ли точка (x,y) внутри заданного полигона
* [IF()](Functions/Arrays_and_conditions_functions.md#IF%28%29) - Проверяет, равен ли первый аргумент 0.0; возвращает второй аргумент, если он не равен нулю, иначе третий
* [IN()](Functions/Arrays_and_conditions_functions.md#IN%28%29) - Возвращает 1, если первый аргумент равен любому из других аргументов, иначе 0
* [INDEXOF()](Functions/Arrays_and_conditions_functions.md#INDEXOF%28%29) - Перебирает все элементы массива и возвращает индекс первого подходящего элемента
* [INTERVAL()](Functions/Arrays_and_conditions_functions.md#INTERVAL%28%29) - Возвращает индекс аргумента, который меньше первого аргумента
* [LENGTH()](Functions/Arrays_and_conditions_functions.md#LENGTH%28%29) - Возвращает количество элементов в MVA
* [REMAP()](Functions/Arrays_and_conditions_functions.md#REMAP%28%29) - Позволяет сделать исключения для значений выражений в зависимости от значений условий

##### Дата и время
* [NOW()](Functions/Date_and_time_functions.md#NOW%28%29) - Возвращает текущую метку времени как INTEGER
* [CURTIME()](Functions/Date_and_time_functions.md#CURTIME%28%29) - Возвращает текущее время в локальном часовом поясе
* [CURDATE()](Functions/Date_and_time_functions.md#CURDATE%28%29) - Возвращает текущую дату в локальном часовом поясе
* [UTC_TIME()](Functions/Date_and_time_functions.md#UTC_TIME%28%29) - Возвращает текущее время в часовом поясе UTC
* [UTC_TIMESTAMP()](Functions/Date_and_time_functions.md#UTC_TIMESTAMP%28%29) - Возвращает текущие дату/время в часовом поясе UTC
* [SECOND()](Functions/Date_and_time_functions.md#SECOND%28%29) - Возвращает значение секунд из метки времени
* [MINUTE()](Functions/Date_and_time_functions.md#MINUTE%28%29) - Возвращает значение минут из метки времени
* [HOUR()](Functions/Date_and_time_functions.md#HOUR%28%29) - Возвращает целочисленный час из аргумента временной метки
* [DAY()](Functions/Date_and_time_functions.md#DAY%28%29) - Возвращает целочисленный день из аргумента временной метки
* [MONTH()](Functions/Date_and_time_functions.md#MONTH%28%29) - Возвращает целочисленный месяц из аргумента временной метки
* [QUARTER()](Functions/Date_and_time_functions.md#QUARTER%28%29) - Возвращает целочисленный квартал года из аргумента временной метки
* [YEAR()](Functions/Date_and_time_functions.md#YEAR%28%29) - Возвращает целочисленный год из аргумента временной метки
* [DAYNAME()](Functions/Date_and_time_functions.md#DAYNAME%28%29) - Возвращает название дня недели для данного аргумента временной метки
* [MONTHNAME()](Functions/Date_and_time_functions.md#MONTHNAME%28%29) - Возвращает название месяца для данного аргумента временной метки
* [DAYOFWEEK()](Functions/Date_and_time_functions.md#DAYOFWEEK%28%29) - Возвращает целочисленный индекс дня недели для данного аргумента временной метки
* [DAYOFYEAR()](Functions/Date_and_time_functions.md#DAYOFYEAR%28%29) - Возвращает целочисленный день года для данного аргумента временной метки
* [YEARWEEK()](Functions/Date_and_time_functions.md#YEARWEEK%28%29) - Возвращает целочисленный год и код дня первой даты текущей недели для данного аргумента временной метки
* [YEARMONTH()](Functions/Date_and_time_functions.md#YEARMONTH%28%29) - Возвращает целочисленные код года и месяца из аргумента временной метки
* [YEARMONTHDAY()](Functions/Date_and_time_functions.md#YEARMONTHDAY%28%29) - Возвращает целочисленные код года, месяца и дня из аргумента временной метки
* [TIMEDIFF()](Functions/Date_and_time_functions.md#TIMEDIFF%28%29) - Возвращает разницу между временными метками
* [DATEDIFF()](Functions/Date_and_time_functions.md#DATEDIFF%28%29) - Возвращает количество дней между двумя заданными временными метками
* [DATE()](Functions/Date_and_time_functions.md#DATE%28%29) - Форматирует часть даты из аргумента временной метки
* [TIME()](Functions/Date_and_time_functions.md#TIME%28%29) - Форматирует часть времени из аргумента временной метки
* [DATE_FORMAT()](Functions/Date_and_time_functions.md#DATE_FORMAT%28%29) - Возвращает форматированную строку на основе предоставленных аргументов даты и формата


##### Геопространственные функции
* [GEODIST()](Functions/Geo_spatial_functions.md#GEODIST%28%29) - Вычисляет расстояние на сфере между двумя заданными точками
* [GEOPOLY2D()](Functions/Geo_spatial_functions.md#GEOPOLY2D%28%29) - Создаёт многоугольник с учётом кривизны Земли
* [POLY2D()](Functions/Geo_spatial_functions.md#POLY2D%28%29) - Создаёт простой многоугольник в плоском пространстве

##### Строковые функции
* [CONCAT()](Functions/String_functions.md#CONCAT%28%29) - Конкатенирует две и более строк
* [REGEX()](Functions/String_functions.md#REGEX%28%29) - Возвращает 1, если регулярное выражение совпало со строкой атрибута, и 0 в противном случае
* [SNIPPET()](Functions/String_functions.md#SNIPPET%28%29) - Подсвечивает результаты поиска
* [SUBSTRING_INDEX()](Functions/String_functions.md#SUBSTRING_INDEX%28%29) - Возвращает подстроку строки до указанного количества разделителей

##### Прочие
* [CONNECTION_ID()](Functions/Other_functions.md#CONNECTION_ID%28%29) - Возвращает текущий ID соединения
* [KNN_DIST()](Functions/Other_functions.md#KNN_DIST%28%29) - Возвращает расстояние поиска вектора KNN
* [LAST_INSERT_ID()](Functions/Other_functions.md#LAST_INSERT_ID%28%29) - Возвращает id документов, вставленных или заменённых последним оператором в текущей сессии
* [UUID_SHORT()](Functions/Other_functions.md#UUID_SHORT%28%29) - Возвращает "короткий" универсальный идентификатор по тому же алгоритму, что и генерация авто-id.

## Общие настройки в конфигурационном файле
Помещается в секцию `common {}` в конфигурационном файле:
* [lemmatizer_base](Server_settings/Common.md#lemmatizer_base) - Базовый путь к словарям лемматизатора
* [progressive_merge](Server_settings/Common.md#progressive_merge) - Определяет порядок слияния дисковых чанков в таблице реального времени
* [json_autoconv_keynames](Server_settings/Common.md#json_autoconv_keynames) - Будет ли и как автоконвертировать имена ключей в JSON-атрибутах
* [json_autoconv_numbers](Server_settings/Common.md#json_autoconv_numbers) - Автоматически обнаруживает и конвертирует возможные строки JSON, представляющие числа, в числовые атрибуты
* [on_json_attr_error](Server_settings/Common.md#on_json_attr_error) - Что делать при обнаружении ошибок формата JSON
* [plugin_dir](Server_settings/Common.md#plugin_dir) - Расположение динамических библиотек и UDF

## [Indexer](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments)
`indexer` — инструмент для создания [простых таблиц](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments)

##### Настройки Indexer в конфигурационном файле
Помещается в секцию `indexer {}` в конфигурационном файле:
* [lemmatizer_cache](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - Размер кеша лемматизатора
* [max_file_field_buffer](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - Максимальный адаптивный размер буфера для файлового поля
* [max_iops](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - Максимальное количество I/O операций индексирования в секунду
* [max_iosize](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - Максимальный допустимый размер операции ввода-вывода
* [max_xmlpipe2_field](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - Максимальный допустимый размер поля для источника типа XMLpipe2
* [mem_limit](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - Лимит использования оперативной памяти для индексирования
* [on_file_field_error](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - Как обрабатывать ошибки ввода-вывода в файловых полях
* [write_buffer](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - Размер буфера записи
* [ignore_non_plain](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - Игнорировать предупреждения о неплоских таблицах

##### Параметры запуска индексатора
```bash
indexer [OPTIONS] [indexname1 [indexname2 [...]]]
```
* [--all](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - Перестраивает все таблицы из конфигурации
* [--buildstops](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - Анализирует источник таблицы так, как если бы индексировал данные, формируя список индексированных терминов
* [--buildfreqs](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - Добавляет в таблицу подсчет частоты для --buildstops
* [--config, -c](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - Указывает путь к файлу конфигурации
* [--dump-rows](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - Выгружает строки, полученные из SQL источников, в указанный файл
* [--help](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - Отображает все доступные параметры
* [--keep-attrs](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - Позволяет повторно использовать существующие атрибуты при переиндексации
* [--keep-attrs-names](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - Указывает, какие атрибуты повторно использовать из существующей таблицы
* [--merge-dst-range](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - Применяет указанный диапазон фильтра при слиянии
* [--merge-killlists](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - Изменяет обработку списков исключений при слиянии таблиц
* [--merge](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - Объединяет две плоских таблицы в одну
* [--nohup](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - Предотвращает отправку индексатором сигнала SIGHUP при включении этой опции
* [--noprogress](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - Скрывает детали прогресса
* [--print-queries](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - Выводит SQL-запросы, отправленные индексатором в базу данных
* [--print-rt](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - Показывает данные, полученные из SQL источников, как INSERT-запросы в таблицу реального времени
* [--quiet](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - Подавляет весь вывод
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
* [--strip-path](Installation/Migration_from_Sphinx.md#index_converter) - Удаляет путь из имен файлов, используемых таблицей
* [--large-docid](Installation/Migration_from_Sphinx.md#index_converter) - Позволяет конвертировать документы с id больше 2^63
* [--output-dir](Installation/Migration_from_Sphinx.md#index_converter) - Записывает новые файлы в указанную папку
* [--all](Installation/Migration_from_Sphinx.md#index_converter) - Конвертирует все таблицы из файла конфигурации / пути
* [--killlist-target](Installation/Migration_from_Sphinx.md#index_converter) - Устанавливает целевые таблицы для применения списков исключений

## [Searchd](Starting_the_server/Manually.md)
`searchd` — это сервер Manticore.

##### Настройки Searchd в файле конфигурации
Должны быть помещены в секцию `searchd {}` файла конфигурации:
  * [access_blob_attrs](Server_settings/Searchd.md#access_blob_attrs) - Определяет, как осуществляется доступ к файлу атрибутов blob таблицы
  * [access_doclists](Server_settings/Searchd.md#access_doclists) - Определяет, как осуществляется доступ к файлу doclists таблицы
  * [access_hitlists](Server_settings/Searchd.md#access_hitlists) - Определяет, как осуществляется доступ к файлу hitlists таблицы
  * [access_plain_attrs](Server_settings/Searchd.md#access_plain_attrs) - Определяет, как поисковый сервер обращается к плоским атрибутам таблицы
  * [access_dict](Server_settings/Searchd.md#access_dict) - Определяет, как осуществляется доступ к словарному файлу таблицы
  * [agent_connect_timeout](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_connect_timeout) - Таймаут подключения к удалённому агенту
  * [agent_query_timeout](Searching/Options.md#agent_query_timeout) - Таймаут запроса к удалённому агенту
  * [agent_retry_count](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_connect_timeout) - Определяет количество попыток Manticore подключения и запроса к удалённым агентам
  * [agent_retry_delay](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) - Устанавливает задержку перед повторной попыткой запроса к удалённому агенту в случае сбоя
  * [attr_flush_period](Data_creation_and_modification/Updating_documents/UPDATE.md#attr_flush_period) - Устанавливает период времени между сбросами обновлённых атрибутов на диск
  * [binlog_flush](Server_settings/Searchd.md#binlog_flush) - Режим сброса/синхронизации транзакций бинарного лога
  * [binlog_max_log_size](Server_settings/Searchd.md#binlog_max_log_size) - Максимальный размер файла бинарного лога
  * [binlog_common](Logging/Binary_logging.md#Binary-logging-strategies) - Общий файл бинарного лога для всех таблиц
  * [binlog_filename_digits](Logging/Binary_logging.md#Log-files) - Количество цифр в имени файла бинарного лога
  * [binlog_flush](Logging/Binary_logging.md#Binary-flushing-strategies) - Стратегия сброса бинарного лога
  * [binlog_path](Server_settings/Searchd.md#binlog_path) - Путь к файлам бинарного лога
  * [client_timeout](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) - Максимальное время ожидания между запросами при использовании постоянных соединений
  * [collation_libc_locale](Server_settings/Searchd.md#collation_libc_locale) - Локаль libc сервера
  * [collation_server](Server_settings/Searchd.md#collation_server) - Стандартная сортировка сервера
  * [data_dir](Server_settings/Searchd.md#data_dir) - Путь к каталогу данных, где Manticore хранит всё ([RT mode](Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29))
  * [diskchunk_flush_write_timeout](Server_settings/Searchd.md#diskchunk_flush_write_timeout) - Таймаут для автоматического сброса RAM-чунка при отсутствии записи в него
  * [diskchunk_flush_search_timeout](Server_settings/Searchd.md#diskchunk_flush_search_timeout) - Таймаут для предотвращения автоматического сброса RAM-чунка при отсутствии поисков в таблице
  * [docstore_cache_size](Server_settings/Searchd.md#docstore_cache_size) - Максимальный размер блоков документов из хранилища документов, удерживаемых в памяти
  * [expansion_limit](Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#expansion_limit) - Максимальное количество расширенных ключевых слов для одного шаблона с подстановочными символами
  * [grouping_in_utc](Server_settings/Searchd.md#grouping_in_utc) - Включает использование часового пояса UTC для группировки временных полей
  * [ha_period_karma](Server_settings/Searchd.md#ha_period_karma) - Размер окна статистики зеркал агентов
  * [ha_ping_interval](Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_ping_interval) - Интервал между пингами зеркал агентов
  * [hostname_lookup](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) - Стратегия обновления имён хостов
  * [jobs_queue_size](Server_settings/Searchd.md#jobs_queue_size) - Определяет максимальное количество "заданий" в очереди одновременно
  * [join_batch_size](Searching/Joining.md#Join-batching) - Определяет размер пакета при объединениях таблиц для балансировки производительности и использования памяти
  * [join_cache_size](Searching/Joining.md#Join-caching) - Определяет размер кэша для повторного использования результатов JOIN-запросов
  * [kibana_version_string](Server_settings/Searchd.md#kibana_version_string) – Строка версии сервера, которая отправляется в ответ на запросы Kibana
  * [listen](Server_settings/Searchd.md#listen) - Указывает IP-адрес и порт или путь к Unix-доменному сокету, на котором searchd принимает подключения
  * [listen_backlog](Server_settings/Searchd.md#listen_backlog) - Очередь ожидания TCP-соединений (backlog)
  * [listen_tfo](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) - Включает флаг TCP_FASTOPEN для всех слушателей
  * [log](Server_settings/Searchd.md#log) - Путь к файлу журнала сервера Manticore
  * [max_batch_queries](Server_settings/Searchd.md#max_batch_queries) - Ограничивает количество запросов в одном пакете
  * [max_connections](Server_settings/Searchd.md#max_connections) - Максимальное количество активных соединений
  * [max_filters](Server_settings/Searchd.md#max_filters) - Максимально допустимое количество фильтров на запрос
  * [max_filter_values](Server_settings/Searchd.md#max_filter_values) - Максимально допустимое количество значений на фильтр
  * [max_open_files](Server_settings/Searchd.md#max_open_files) - Максимальное количество файлов, которые сервер может открыть
  * [max_packet_size](Server_settings/Searchd.md#max_packet_size) - Максимально допустимый размер сетевого пакета
  * [mysql_version_string](Server_settings/Searchd.md#mysql_version_string) - Строка версии сервера, возвращаемая через MySQL-протокол
  * [net_throttle_accept](Server_settings/Searchd.md#net_throttle_accept) - Определяет, сколько клиентов принимается за каждую итерацию сетевого цикла
  * [net_throttle_action](Server_settings/Searchd.md#net_throttle_action)  - Определяет, сколько запросов обрабатывается за каждую итерацию сетевого цикла
  * [net_wait_tm](Server_settings/Searchd.md#net_wait_tm) - Контролирует интервал цикла ожидания сетевого потока
  * [net_workers](Server_settings/Searchd.md#net_workers) - Количество сетевых потоков
  * [network_timeout](Server_settings/Searchd.md#network_timeout) - Таймаут сети для клиентских запросов
  * [node_address](Server_settings/Searchd.md#node_address) - Определяет сетевой адрес узла
  * [persistent_connections_limit](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) - Максимальное количество одновременных постоянных соединений к удалённым постоянным агентам
  * [pid_file](Server_settings/Searchd.md#pid_file) - Путь к файлу pid сервера Manticore
  * [predicted_time_costs](Server_settings/Searchd.md#predicted_time_costs) - Стоимости для модели предсказания времени запроса
  * [preopen_tables](Server_settings/Searchd.md#preopen_tables) - Определяет, следует ли принудительно предварительно открывать все таблицы при запуске
  * [pseudo_sharding](Server_settings/Searchd.md#pseudo_sharding) - Включает псевдо-шардинг для поисковых запросов по обычным и реальным таблицам
  * [qcache_max_bytes](Server_settings/Searchd.md#qcache_max_bytes) - Максимальный объем оперативной памяти, выделяемый для кэшированных наборов результатов
  * [qcache_thresh_msec](Server_settings/Searchd.md#qcache_thresh_msec) - Минимальный порог времени выполнения запроса, при котором результат кэшируется
  * [qcache_ttl_sec](Server_settings/Searchd.md#qcache_ttl_sec) - Период устаревания кэшированного набора результатов
  * [query_log](Server_settings/Searchd.md#query_log) - Путь к файлу журнала запросов
  * [query_log_format](Server_settings/Searchd.md#query_log_format) - Формат журнала запросов
  * [query_log_min_msec](Server_settings/Searchd.md#query_log_min_msec) - Предотвращает запись в журнал слишком быстрых запросов
  * [query_log_mode](Server_settings/Searchd.md#query_log_mode) - Режим прав доступа к файлу журнала запросов
  * [read_buffer_docs](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#read_buffer_docs) - Размер буфера чтения на ключевое слово для списков документов
  * [read_buffer_hits](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#read_buffer_docs) - Размер буфера чтения на ключевое слово для списков совпадений
  * [read_unhinted](Server_settings/Searchd.md#read_unhinted) - Размер чтения без подсказок
  * [rt_flush_period](Server_settings/Searchd.md#rt_flush_period) - Как часто Manticore сбрасывает в диск блоки оперативной памяти реальных таблиц
  * [rt_merge_iops](Server_settings/Searchd.md#rt_merge_iops) - Максимальное количество операций ввода-вывода (в секунду), которое разрешено выполнять потоку слияния блоков реальных таблиц
  * [rt_merge_maxiosize](Server_settings/Searchd.md#rt_merge_maxiosize) - Максимальный размер операции ввода-вывода, которую разрешено выполнять потоку слияния блоков реальных таблиц
  * [seamless_rotate](Server_settings/Searchd.md#seamless_rotate) - Предотвращает зависание searchd при вращении таблиц с огромным объемом данных для предварительного кэширования
  * [secondary_indexes](Server_settings/Searchd.md#secondary_indexes) - Включает использование вторичных индексов для поисковых запросов
  * [server_id](Server_settings/Searchd.md#server_id) - Идентификатор сервера, используемый как сэмпл для генерации уникального ID документа
  * [shutdown_timeout](Server_settings/Searchd.md#shutdown_timeout) - Таймаут `--stopwait` для searchd
  * [shutdown_token](Server_settings/Searchd.md#shutdown_token) - SHA1 хеш пароля, необходимого для выполнения команды `shutdown` из VIP SQL подключения
  * [snippets_file_prefix](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) - Префикс, добавляемый к локальным именам файлов при генерации сниппетов в режиме `load_files`
  * [sphinxql_state](Server_settings/Searchd.md#sphinxql_state) - Путь к файлу, в котором сериализуется текущее состояние SQL
  * [sphinxql_timeout](Server_settings/Searchd.md#sphinxql_timeout) - Максимальное время ожидания между запросами от клиента MySQL
  * [ssl_ca](Server_settings/Searchd.md#ssl_ca) - Путь к файлу сертификата удостоверяющего центра SSL
  * [ssl_cert](Server_settings/Searchd.md#ssl_cert) - Путь к SSL-сертификату сервера
  * [ssl_key](Server_settings/Searchd.md#ssl_key) - Путь к ключу SSL-сертификата сервера
  * [subtree_docs_cache](Server_settings/Searchd.md#subtree_docs_cache) - Максимальный размер кэша общих поддеревьев документов
  * [subtree_hits_cache](Server_settings/Searchd.md#subtree_hits_cache) - Максимальный размер кэша общих поддеревьев совпадений, на запрос
  * [timezone](Server_settings/Searchd.md#timezone) - Часовой пояс, используемый функциями, связанными с датой и временем
  * [thread_stack](Server_settings/Searchd.md#thread_stack) - Максимальный размер стека для задания
  * [unlink_old](Server_settings/Searchd.md#unlink_old) - Следует ли удалять .old копии таблиц при успешном вращении
  * [watchdog](Server_settings/Searchd.md#watchdog) - Включать или выключать сторожевой таймер сервера Manticore

##### Параметры запуска Searchd
```bash
searchd [OPTIONS]
```
* [--config, -c](Starting_the_server/Manually.md#searchd-command-line-options) - Указывает путь к конфигурационному файлу
* [--console](Starting_the_server/Manually.md#searchd-command-line-options) - Принудительно запускает сервер в консольном режиме
* [--coredump](Starting_the_server/Manually.md#searchd-command-line-options) - Включает сохранение дампа памяти при сбое
* [--cpustats](Starting_the_server/Manually.md#searchd-command-line-options) - Включает отчет о затратах CPU
* [--delete](Starting_the_server/Manually.md#searchd-command-line-options) - Удаляет службу Manticore из Microsoft Management Console и других мест регистрации служб
* [--force-preread](Starting_the_server/Manually.md#searchd-command-line-options) - Запрещает серверу принимать входящие подключения до завершения предварительного чтения файлов таблиц
* [--help, -h](Starting_the_server/Manually.md#searchd-command-line-options) - Отображает все доступные параметры
* [--table (--index)](Starting_the_server/Manually.md#searchd-command-line-options) - Ограничивает сервер обслуживанием только указанной таблицы
* [--install](Starting_the_server/Manually.md#searchd-command-line-options) - Устанавливает searchd в качестве службы в Microsoft Management Console
* [--iostats](Starting_the_server/Manually.md#searchd-command-line-options) - Включает отчет о вводе-выводе
* [--listen, -l](Starting_the_server/Manually.md#searchd-command-line-options) - Переопределяет параметр [listen](Server_settings/Searchd.md#listen) из конфигурационного файла
* [--logdebug, --logdebugv, --logdebugvv](Starting_the_server/Manually.md#searchd-command-line-options) - Включает дополнительный отладочный вывод в лог сервера
* [--logreplication](Starting_the_server/Manually.md#searchd-command-line-options) - Включает дополнительный отладочный вывод репликации в лог сервера
* [--new-cluster](Starting_the_server/Manually.md#searchd-command-line-options) - Инициализирует кластер репликации и устанавливает сервер в качестве эталонного узла с защитой при [перезапуске кластера](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md)
* [--new-cluster-force](Starting_the_server/Manually.md#searchd-command-line-options) - Инициализирует кластер репликации и устанавливает сервер в качестве опорного узла, обходя защиту [перезапуска кластера](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md)
* [--nodetach](Starting_the_server/Manually.md#searchd-command-line-options) - Держит searchd работающим на переднем плане
* [--ntservice](Starting_the_server/Manually.md#searchd-command-line-options) - Используется Microsoft Management Console для запуска searchd как службы на платформах Windows
* [--pidfile](Starting_the_server/Manually.md#searchd-command-line-options) - Переопределяет [pid_file](Server_settings/Searchd.md#pid_file) в конфигурационном файле
* [--port, p](Starting_the_server/Manually.md#searchd-command-line-options) - Указывает порт, на котором searchd должен слушать, игнорируя порт, указанный в конфигурационном файле
* [--replay-flags](Starting_the_server/Manually.md#searchd-command-line-options) - Устанавливает дополнительные параметры воспроизведения бинарного лога
* [--servicename](Starting_the_server/Manually.md#searchd-command-line-options) - Назначает заданное имя для searchd при установке или удалении службы, как отображается в Microsoft Management Console
* [--status](Starting_the_server/Manually.md#searchd-command-line-options) - Запрашивает статус запущенной поисковой службы
* [--stop](Starting_the_server/Manually.md#searchd-command-line-options) - Останавливает сервер Manticore
* [--stopwait](Starting_the_server/Manually.md#searchd-command-line-options) - Аккуратно останавливает сервер Manticore
* [--strip-path](Starting_the_server/Manually.md#searchd-command-line-options) - Удаляет пути из всех имен файлов, на которые ссылается таблица
* [-v](Starting_the_server/Manually.md#searchd-command-line-options) - Отображает информацию о версии

##### Переменные окружения searchd
* [MANTICORE_TRACK_DAEMON_SHUTDOWN](Starting_the_server/Manually.md#Environment-variables) - Включает подробное логирование при завершении работы searchd

## [Indextool](Miscellaneous_tools.md#indextool)
Разнообразные функции обслуживания таблиц, полезные для устранения неполадок.
```bash
indextool <command> [options]
```
##### Параметры запуска Indextool
Используются для дампа различной отладочной информации, связанной с физической таблицей.
```bash
indextool <command> [options]
```
* [--config, -c](Miscellaneous_tools.md#indextool) - Указывает путь к конфигурационному файлу
* [--quiet, -q](Miscellaneous_tools.md#indextool) - Подавляет вывод indextool; нет вывода баннера и т.д.
* [--help, -h](Miscellaneous_tools.md#indextool) - Список всех доступных параметров
* [-v](Miscellaneous_tools.md#indextool) - Отображает информацию о версии
* [Indextool](Miscellaneous_tools.md#indextool) - Проверяет конфигурационный файл
* [--buildidf](Miscellaneous_tools.md#indextool) - Создает IDF файл из одного или нескольких дампов словарей
* [--build-infixes](Miscellaneous_tools.md#indextool) - Строит инфиксы для существующей таблицы dict=keywords
* [--dumpheader](Miscellaneous_tools.md#indextool) - Быстро выводит дамп заголовочного файла таблицы
* [--dumpconfig](Miscellaneous_tools.md#indextool) - Выводит определение таблицы из данного заголовочного файла в формате, близком к manticore.conf
* [--dumpheader](Miscellaneous_tools.md#indextool) - Выводит заголовок таблицы по имени, ища путь к заголовку в конфигурационном файле
* [--dumpdict](Miscellaneous_tools.md#indextool) - Выводит словарь таблицы
* [--dumpdocids](Miscellaneous_tools.md#indextool) - Выводит идентификаторы документов по имени таблицы
* [--dumphitlist](Miscellaneous_tools.md#indextool) - Выводит все вхождения заданного ключевого слова/ID в указанной таблице
* [--docextract](Miscellaneous_tools.md#indextool) - Выполняет проверку таблицы, проходя по всему словарю/документам/вхождениям и собирает все слова и вхождения, относящиеся к запрошенному документу
* [--fold](Miscellaneous_tools.md#indextool) - Тестирует токенизацию на основе настроек таблицы
* [--htmlstrip](Miscellaneous_tools.md#indextool) - Фильтрует STDIN с использованием настроек HTML stripper для указанной таблицы
* [--mergeidf](Miscellaneous_tools.md#indextool) - Объединяет несколько .idf файлов в один
* [--morph](Miscellaneous_tools.md#indextool) - Применяет морфологию к STDIN и выводит результат в stdout
* [--check](Miscellaneous_tools.md#indextool) - Проверяет файлы данных таблицы на согласованность
* [--check-id-dups](Miscellaneous_tools.md#indextool) - Проверяет дубликаты ID
* [--check-disk-chunk](Miscellaneous_tools.md#indextool) - Проверяет отдельный диск-чанк таблицы RT
* [--strip-path](Miscellaneous_tools.md#indextool) - Удаляет пути из всех имен файлов, на которые ссылается таблица
* [--rotate](Miscellaneous_tools.md#indextool) - Определяет, нужно ли проверять таблицу, ожидающую ротацию при использовании `--check`
* [--apply-killlists](Miscellaneous_tools.md#indextool) - Применяет kill-списки ко всем таблицам, перечисленным в конфигурационном файле

## [Wordbreaker](Miscellaneous_tools.md#wordbreaker)
Разбивает сложные слова на составные части.
```bash
wordbreaker [-dict path/to/dictionary_file] {split|test|bench}
```

##### Параметры запуска Wordbreaker
* [STDIN](Miscellaneous_tools.md#wordbreaker) - Принимает строку для разбиения на части
* [-dict](Miscellaneous_tools.md#wordbreaker) - Указывает словарь для использования
* [split|test|bench](Miscellaneous_tools.md#wordbreaker) - Задает команду

## [Spelldump](Miscellaneous_tools.md#spelldump)
Извлекает содержимое словаря в формате ispell или MySpell

```bash
spelldump [options] <dictionary> <affix> [result] [locale-name]
```
* [dictionary](Miscellaneous_tools.md#spelldump) - Основной файл словаря
* [affix](Miscellaneous_tools.md#spelldump) - Файл аффиксов для словаря
* [result](Miscellaneous_tools.md#spelldump) - Указывает место назначения для вывода данных словаря
* [locale-name](Miscellaneous_tools.md#spelldump) - Указывает локаль для использования

## Список зарезервированных ключевых слов

Полный алфавитный список ключевых слов, которые в настоящее время зарезервированы в синтаксисе Manticore SQL (следовательно, их нельзя использовать в качестве идентификаторов).

```
AND, AS, BY, COLUMNARSCAN, DISTINCT, DIV, DOCIDINDEX, EXPLAIN, FACET, FALSE, FORCE, FROM, IGNORE, IN, INDEXES, INNER, IS, JOIN, KNN, LEFT, LIMIT, MOD, NOT, NO_COLUMNARSCAN, NO_DOCIDINDEX, NO_SECONDARYINDEX, NULL, OFFSET, ON, OR, ORDER, RELOAD, SECONDARYINDEX, SELECT, SYSFILTERS, TRUE
```

## Документация для старых версий Manticore

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
* [13.2.3](https://manual.manticoresearch.com/manticore-13-2-3/). [Installation page](https://manticoresearch.com/install-13.2.3/)
* [13.6.7](https://manual.manticoresearch.com/manticore-13-6-7/). [Installation page](https://manticoresearch.com/install-13.6.7/)
* [13.11.0](https://manual.manticoresearch.com/manticore-13-11-0/). [Installation page](https://manticoresearch.com/install-13.11.0/)
* [13.11.1](https://manual.manticoresearch.com/manticore-13-11-1/). [Installation page](https://manticoresearch.com/install-13.11.1/)
<!-- proofread -->

