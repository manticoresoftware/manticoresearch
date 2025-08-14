# 参考资料

### SQL 命令
##### 模式管理
* [CREATE TABLE](Creating_a_table/Local_tables/Real-time_table.md#CREATE-TABLE-command:) - 创建新表
* [CREATE TABLE LIKE](Creating_a_table/Local_tables/Real-time_table.md#CREATE-TABLE-LIKE:) - 以另一张表为模板创建表
* [CREATE TABLE LIKE ... WITH DATA](Creating_a_table/Local_tables/Real-time_table.md#CREATE-TABLE-LIKE:) - 复制一张表
* [CREATE SOURCE](Integration/Kafka.md#Source) - 创建 Kafka 消费者源
* [CREATE MATERIALIZED VIEW](Integration/Kafka.md#Materialized-view) - 从 Kafka 消息进行数据转化
* [CREATE MV](Integration/Kafka.md#Materialized-view) - 与上一个相同
* [DESCRIBE](Listing_tables.md#DESCRIBE) - 输出表字段列表及其类型
* [ALTER TABLE](Updating_table_schema_and_settings.md) - 更改表模式 / 设置
* [ALTER TABLE REBUILD SECONDARY](Updating_table_schema_and_settings.md#Rebuilding-a-secondary-index) - 更新/恢复二级索引
* [ALTER TABLE type='distributed'](Updating_table_schema_and_settings.md#Changing-a-distributed-table) - 更新/恢复二级索引
* [ALTER TABLE RENAME](Updating_table_schema_and_settings.md#Renaming-a-real-time-table)
* [ALTER MATERIALIZED VIEW {name} suspended=1](Integration/Kafka.md#Altering-materialized-views) - 挂起或恢复从 Kafka 源的消费
* [DROP TABLE IF EXISTS](Deleting_a_table.md#Deleting-a-table) - 删除表（如果存在）
* [SHOW TABLES](Listing_tables.md#DESCRIBE) - 显示表列表
* [SHOW SOURCES](Integration/Kafka.md#Listing) - 显示 Kafka 源列表
* [SHOW MATERIALIZED VIEWS](Integration/Kafka.md#Listing) - 显示物化视图列表
* [SHOW MVS](Integration/Kafka.md#Listing) - 前一命令的别名
* [SHOW CREATE TABLE](Listing_tables.md#DESCRIBE) - 显示创建表的 SQL 命令
* [SHOW TABLE INDEXES](Node_info_and_management/Table_settings_and_status/SHOW_TABLE_INDEXES.md) - 显示表的可用二级索引信息
* [SHOW TABLE STATUS](Node_info_and_management/Table_settings_and_status/SHOW_TABLE_STATUS.md) - 显示当前表状态信息
* [SHOW TABLE SETTINGS](Node_info_and_management/Table_settings_and_status/SHOW_TABLE_SETTINGS.md) - 显示表设置
* [SHOW LOCKS](Securing_and_compacting_a_table/Freezing_a_table.md#SHOW-LOCKS) - 显示冻结表信息

##### 数据管理
* [INSERT](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md) - 添加新文档
* [REPLACE](Data_creation_and_modification/Updating_documents/REPLACE.md) - 用新文档替换现有文档
* [REPLACE .. SET](Data_creation_and_modification/Updating_documents/REPLACE.md?client=REPLACE+SET) - 替换表中一个或多个字段
* [UPDATE](Data_creation_and_modification/Updating_documents/UPDATE.md) - 在文档中进行原地更新
* [DELETE](Data_creation_and_modification/Deleting_documents.md) - 删除文档
* [TRUNCATE TABLE](Emptying_a_table.md) - 删除表中所有文档

##### 备份
* [BACKUP](Securing_and_compacting_a_table/Backup_and_restore.md#BACKUP-SQL-command-reference) - 备份您的表

##### SELECT
* [SELECT](Searching/Full_text_matching/Basic_usage.md#SQL) - 搜索
  * [WHERE](Searching/Filters.md#Filters) - 过滤条件
  * [GROUP BY](Searching/Grouping.md) - 分组搜索结果
  * [GROUP BY ORDER](Searching/Grouping.md) - 排序分组
  * [GROUP BY HAVING](Searching/Grouping.md) - 过滤分组
  * [OPTION](Searching/Options.md#OPTION) - 查询选项
  * [FACET](Searching/Faceted_search.md) - 分面搜索
  * [SUB-SELECTS](Searching/Sub-selects.md) - 关于使用 SELECT 子查询
  * [JOIN](Searching/Joining.md) - SELECT 中的表连接
* [EXPLAIN QUERY](Searching/Full_text_matching/Profiling.md#Profiling-without-running-a-query) - 显示查询执行计划，不执行查询
* [SHOW META](Node_info_and_management/SHOW_META.md) - 显示已执行查询的扩展信息
* [SHOW PROFILE](Node_info_and_management/Profiling/Query_profile.md) - 显示已执行查询的分析信息
* [SHOW PLAN](Searching/Full_text_matching/Profiling.md#Profiling-the-query-tree-in-SQL) - 查询执行后显示执行计划
* [SHOW WARNINGS](Node_info_and_management/SHOW_WARNINGS.md) - 显示最新查询的警告信息

##### 刷新相关操作
* [FLUSH ATTRIBUTES](Securing_and_compacting_a_table/Flushing_attributes.md) - 强制刷新更新的属性到磁盘
* [FLUSH HOSTNAMES](Securing_and_compacting_a_table/Flushing_hostnames.md) - 更新与代理主机名关联的 IP
* [FLUSH LOGS](Logging/Rotating_query_and_server_logs.md) - 重新打开 searchd 日志和查询日志文件（类似 USR1）

##### 实时表优化
* [FLUSH RAMCHUNK](Securing_and_compacting_a_table/Flushing_RAM_chunk_to_a_new_disk_chunk.md#FLUSH-RAMCHUNK) - 强制创建新的磁盘块
* [FLUSH TABLE](Securing_and_compacting_a_table/Flushing_RAM_chunk_to_disk.md#FLUSH-TABLE) - 将实时表内存块刷新到磁盘
* [OPTIMIZE TABLE](Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE) - 将实时表加入优化队列

##### 导入到实时表
* [ATTACH TABLE](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Attaching_one_table_to_another.md) - 将数据从普通表移动到实时表
* [IMPORT TABLE](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Importing_table.md) - 将预先创建的 RT 或 PQ 表导入到运行于 RT 模式的服务器

##### 复制
* [JOIN CLUSTER](Creating_a_cluster/Setting_up_replication/Joining_a_replication_cluster.md) - 加入复制集群
* [ALTER CLUSTER](Creating_a_cluster/Setting_up_replication/Managing_replication_nodes.md) - 向复制集群添加/删除表
* [SET CLUSTER](Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Cluster-parameters) - 更改复制集群设置
* [DELETE CLUSTER](Creating_a_cluster/Setting_up_replication/Deleting_a_replication_cluster.md) - 删除复制集群

##### 普通表旋转
* [RELOAD TABLE](Data_creation_and_modification/Adding_data_from_external_storages/Rotating_a_table.md#RELOAD-TABLE) - 旋转普通表
* [RELOAD TABLES](Data_creation_and_modification/Adding_data_from_external_storages/Rotating_a_table.md#RELOAD-TABLE) - 旋转所有普通表

##### 事务
* [BEGIN](Data_creation_and_modification/Transactions.md#BEGIN,-COMMIT,-and-ROLLBACK) - 开始一个事务
* [COMMIT](Data_creation_and_modification/Transactions.md#BEGIN,-COMMIT,-and-ROLLBACK) - 结束一个事务
* [ROLLBACK](Data_creation_and_modification/Transactions.md#BEGIN,-COMMIT,-and-ROLLBACK) - 回滚一个事务

##### CALL
* [CALL SUGGEST, CALL QSUGGEST](Searching/Spell_correction.md#CALL-QSUGGEST,-CALL-SUGGEST) - 提供拼写纠正建议
* [CALL SNIPPETS](Searching/Highlighting.md) - 根据提供的数据和查询构建高亮结果摘要
* [CALL PQ](Searching/Percolate_query.md) - 执行一个过筛查询
* [CALL KEYWORDS](Searching/Autocomplete.md#CALL-KEYWORDS) - 用于检查关键字的分词情况。也允许检索提供关键字的分词形式
* [CALL AUTOCOMPLETE](Searching/Autocomplete.md#CALL-AUTOCOMPLETE) - 自动补全搜索查询

##### 插件
* [CREATE FUNCTION](Extensions/UDFs_and_Plugins/UDF/Creating_a_function.md) - 安装用户自定义函数 (UDF)
* [DROP FUNCTION](Extensions/UDFs_and_Plugins/UDF/Deleting_a_function.md) - 删除用户自定义函数 (UDF)
* [CREATE PLUGIN](Extensions/UDFs_and_Plugins/Plugins/Creating_a_plugin.md) - 安装插件
* [CREATE BUDDY PLUGIN](Extensions/UDFs_and_Plugins/Plugins/Creating_a_plugin.md#CREATE-BUDDY-PLUGIN) - 安装 Buddy 插件
* [DROP PLUGIN](Extensions/UDFs_and_Plugins/Plugins/Deleting_a_plugin.md#DELETE-PLUGIN) - 删除插件
* [DROP BUDDY PLUGIN](Extensions/UDFs_and_Plugins/Plugins/Deleting_a_plugin.md#DELETE-BUDDY-PLUGIN) - 删除 Buddy 插件
* [RELOAD PLUGINS](Extensions/UDFs_and_Plugins/Plugins/Reloading_plugins.md) - 从指定库重载所有插件
* [ENABLE BUDDY PLUGIN](Extensions/UDFs_and_Plugins/Plugins/Enabling_and_disabling_buddy_plugins.md#ENABLE-BUDDY-PLUGIN) - 重新激活之前禁用的 Buddy 插件
* [DISABLE BUDDY PLUGIN](Extensions/UDFs_and_Plugins/Plugins/Enabling_and_disabling_buddy_plugins.md#DISABLE-BUDDY-PLUGIN) - 禁用活动状态的 Buddy 插件

##### 服务器状态
* [SHOW STATUS](Node_info_and_management/Node_status.md#SHOW-STATUS) - 显示多个有用的性能计数器
* [SHOW THREADS](Node_info_and_management/SHOW_THREADS.md) - 列出所有当前活动的客户端线程
* [SHOW VARIABLES](Node_info_and_management/SHOW_VARIABLES.md) - 列出服务器范围内的变量及其值
* [SHOW VERSION](Node_info_and_management/SHOW_VERSION.md#SHOW-VERSION) - 提供实例各组件的详细版本信息

### HTTP 端点
* [/sql](Connecting_to_the_server/HTTP.md#SQL-over-HTTP) - 通过 HTTP JSON 执行 SQL 语句
* [/cli](Connecting_to_the_server/HTTP.md#/cli) - 提供 HTTP 命令行界面
* [/insert](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md) - 向实时表插入文档
* [/pq/tbl_name/doc](Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md#Adding-rules-to-a-percolate-table) - 向过筛表添加 PQ 规则
* [/update](Data_creation_and_modification/Updating_documents/UPDATE.md#Updates-via-HTTP-JSON) - 更新实时表中的文档
* [/replace](Data_creation_and_modification/Updating_documents/REPLACE.md) - 替换实时表中已有的文档，若不存在则插入
* [/pq/tbl_name/doc/N?refresh=1](Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md#Adding-rules-to-a-percolate-table) - 替换过筛表中的 PQ 规则
* [/delete](Data_creation_and_modification/Deleting_documents.md) - 从表中删除文档
* [/bulk](Data_creation_and_modification/Updating_documents/UPDATE.md#Bulk-updates) - 在单次调用中执行多个插入、更新或删除操作。更多关于批量插入的信息请查阅[这里](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md)。
* [/search](Searching/Full_text_matching/Basic_usage.md#HTTP-JSON) - 执行搜索
* [/search -> knn](Searching/KNN.md) - 执行 KNN 向量搜索
* [/pq/tbl_name/search](Searching/Percolate_query.md) - 在过筛表中执行反向搜索
* [/tbl_name/_mapping](Creating_a_table/Local_tables/Real-time_table.md#_mapping-API:) - 以 Elasticsearch 风格创建表结构

### 常见事项
* [字段名语法](Creating_a_table/Data_types.md#Field-name-syntax)
* [数据类型](Creating_a_table/Data_types.md)
* [引擎](Creating_a_table/Data_types.md)
* [普通模式](Read_this_first.md#Real-time-mode-vs-plain-mode)
* [实时模式](Read_this_first.md#Real-time-mode-vs-plain-mode)

##### 常见表设置
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

##### 普通表设置
* [json_secondary_indexes](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#json_secondary_indexes)
* [source](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [stored_fields](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [stored_only_fields](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [columnar_attrs](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)

##### 分布式表设置
* [local](Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md)
* [agent](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent)
* [agent_connect_timeout](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent)
* [agent_blackhole](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent)
* [agent_persistent](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent)
* [agent_query_timeout](Searching/Options.md#agent_query_timeout)
* [agent_retry_count](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent)
* [ha_strategy](Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_strategy)
* [mirror_retry_count](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent)

##### RT table settings
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


## Full-text search operators
* [OR](Searching/Full_text_matching/Operators.md#OR-operator)
* [MAYBE](Searching/Full_text_matching/Operators.md#MAYBE-operator)
* [NOT](Searching/Full_text_matching/Operators.md#Negation-operator) - NOT 运算符
* [@field](Searching/Full_text_matching/Operators.md#Field-search-operator) - 字段搜索运算符
* [@field%5BN%5D](Searching/Full_text_matching/Operators.md#Field-search-operator) - 字段位置限制修饰符
* [@(field1,field2,...)](Searching/Full_text_matching/Operators.md#Field-search-operator) - 多字段搜索运算符
* [@!field](Searching/Full_text_matching/Operators.md#Field-search-operator) - 忽略字段搜索运算符
* [@!(field1,field2,...)](Searching/Full_text_matching/Operators.md#Field-search-operator) - 忽略多字段搜索运算符
* [@*](Searching/Full_text_matching/Operators.md#Field-search-operator) - 全字段搜索运算符
* ["word1 word2 ... "](Searching/Full_text_matching/Operators.md#Phrase-search-operator) - 短语搜索运算符
* ["word1 word2 ... "~N](Searching/Full_text_matching/Operators.md#Proximity-search-operator) - 临近搜索运算符
* ["word1 word2 ... "/N](Searching/Full_text_matching/Operators.md#Quorum-matching-operator) - 法定人数匹配运算符
* [word1 << word2 << word3](Searching/Full_text_matching/Operators.md#Strict-order-operator) - 严格顺序运算符
* [=word1](Searching/Full_text_matching/Operators.md#Exact-form-modifier) - 精确形式修饰符
* [^word1](Searching/Full_text_matching/Operators.md#Field-start-and-field-end-modifier) - 字段起始修饰符
* [word2$](Searching/Full_text_matching/Operators.md#Field-start-and-field-end-modifier) - 字段结束修饰符
* [word^N](Searching/Full_text_matching/Operators.md#IDF-boost-modifier) - 关键词 IDF 提升修饰符
* [word1 NEAR/N word2](Searching/Full_text_matching/Operators.md#NEAR-operator) - NEAR，广义临近运算符
* [word1 NOTNEAR/N word2](Searching/Full_text_matching/Operators.md#NOTNEAR-operator) - NOTNEAR，否定断言运算符
* [word1 PARAGRAPH word2 PARAGRAPH "word3 word4"](Searching/Full_text_matching/Operators.md#SENTENCE-and-PARAGRAPH-operators) - 段落运算符
* [word1 SENTENCE word2 SENTENCE "word3 word4"](Searching/Full_text_matching/Operators.md#SENTENCE-and-PARAGRAPH-operators) - 句子运算符
* [ZONE:(h3,h4)](Searching/Full_text_matching/Operators.md#ZONE-limit-operator) - 区域限制运算符
* [ZONESPAN:(h2)](Searching/Full_text_matching/Operators.md#ZONESPAN-limit-operator) - 区域跨度限制运算符
* [@@relaxed](Searching/Full_text_matching/Operators.md#Field-search-operator) - 抑制缺失字段错误
* [t?st](Searching/Full_text_matching/Operators.md#Wildcard-operators) - 通配符运算符
* [REGEX(/pattern/)](Searching/Full_text_matching/Operators.md#REGEX-operator) - 正则表达式运算符

## Functions
##### Mathematical
* [ABS()](Functions/Mathematical_functions.md#ABS%28%29) - 返回绝对值
* [ATAN2()](Functions/Mathematical_functions.md#ATAN2%28%29) - 返回两个参数的反正切函数
* [BITDOT()](Functions/Mathematical_functions.md#BITDOT%28%29) - 返回掩码每个位与其权重乘积的和
* [CEIL()](Functions/Mathematical_functions.md#CEIL%28%29) - 返回大于或等于参数的最小整数值
* [COS()](Functions/Mathematical_functions.md#COS%28%29) - 返回参数的余弦值
* [CRC32()](Functions/Mathematical_functions.md#CRC32%28%29) - 返回参数的 CRC32 值
* [EXP()](Functions/Mathematical_functions.md#EXP%28%29) - 返回参数的指数值
* [FIBONACCI()](Functions/Mathematical_functions.md#FIBONACCI%28%29) - 返回第 N 个斐波那契数，其中 N 是整数参数
* [FLOOR()](Functions/Mathematical_functions.md#FLOOR%28%29) - 返回小于或等于参数的最大整数值
* [GREATEST()](Functions/Mathematical_functions.md#GREATEST%28%29) - 以 JSON/MVA 数组作为参数，返回该数组中的最大值
* [IDIV()](Functions/Mathematical_functions.md#IDIV%28%29) - 返回第一个参数除以第二个参数的整数除法结果
* [LEAST()](Functions/Mathematical_functions.md#LEAST%28%29) - 以 JSON/MVA 数组作为参数，返回该数组中的最小值
* [LN()](Functions/Mathematical_functions.md#LN%28%29) - 返回参数的自然对数
* [LOG10()](Functions/Mathematical_functions.md#LOG10%28%29) - 返回参数的常用对数（以10为底）
* [LOG2()](Functions/Mathematical_functions.md#LOG2%28%29) - 返回参数的二进制对数（以2为底）
* [MAX()](Functions/Mathematical_functions.md#MAX%28%29) - 返回两个参数中的较大值
* [MIN()](Functions/Mathematical_functions.md#MIN%28%29) - 返回两个参数中的较小值
* [POW()](Functions/Mathematical_functions.md#POW%28%29) - 返回第一个参数的第二个参数次幂
* [RAND()](Functions/Mathematical_functions.md#RAND%28%29) - 返回 0 到 1 之间的随机浮点数
* [SIN()](Functions/Mathematical_functions.md#SIN%28%29) - 返回参数的正弦值
* [SQRT()](Functions/Mathematical_functions.md#SQRT%28%29) - 返回参数的平方根


##### 搜索和排序
* [BM25F()](Functions/Searching_and_ranking_functions.md#BM25F%28%29) - 返回精确的 BM25F 公式值
* [EXIST()](Functions/Searching_and_ranking_functions.md#EXIST%28%29) - 用默认值替换不存在的列
* [GROUP_CONCAT()](Searching/Grouping.md#GROUP_CONCAT%28field%29) - 生成组内所有文档属性值的逗号分隔列表
* [HIGHLIGHT()](Searching/Highlighting.md) - 高亮显示搜索结果
* [MIN_TOP_SORTVAL()](Functions/Searching_and_ranking_functions.md#MIN_TOP_SORTVAL%28%29) - 返回当前 top-N 匹配中最差元素的排序键值
* [MIN_TOP_WEIGHT()](Functions/Searching_and_ranking_functions.md#MIN_TOP_WEIGHT%28%29) - 返回当前 top-N 匹配中最差元素的权重值
* [PACKEDFACTORS()](Functions/Searching_and_ranking_functions.md#PACKEDFACTORS%28%29) - 输出加权因子
* [REMOVE_REPEATS()](Functions/Searching_and_ranking_functions.md#REMOVE_REPEATS%28%29) - 移除具有相同“列”值的重复调整行
* [WEIGHT()](Functions/Searching_and_ranking_functions.md#WEIGHT%28%29) - 返回全文匹配得分
* [ZONESPANLIST()](Functions/Searching_and_ranking_functions.md#ZONESPANLIST%28%29) - 返回匹配区域跨度的对列表
* [QUERY()](Functions/Searching_and_ranking_functions.md#QUERY%28%29) - 返回当前全文查询

##### 类型转换
* [BIGINT()](Functions/Type_casting_functions.md#BIGINT%28%29) - 强制将整数参数升级为 64 位类型
* [DOUBLE()](Functions/Type_casting_functions.md#DOUBLE%28%29) - 强制将给定参数转换为浮点类型
* [INTEGER()](Functions/Type_casting_functions.md#INTEGER%28%29) - 强制将给定参数升级为 64 位有符号类型
* [TO_STRING()](Functions/Type_casting_functions.md#TO_STRING%28%29) - 强制将参数转换为字符串类型
* [UINT()](Functions/Type_casting_functions.md#UINT%28%29) - 将给定参数转换为 32 位无符号整数类型
* [UINT64()](Functions/Type_casting_functions.md#UINT64%28%29) - 将给定参数转换为 64 位无符号整数类型
* [SINT()](Functions/Type_casting_functions.md#SINT%28%29) - 将 32 位无符号整数解释为有符号 64 位整数

##### 数组和条件
* [ALL()](Functions/Arrays_and_conditions_functions.md#ALL%28%29) - 如果条件对数组中的所有元素均为真，返回 1
* [ANY()](Functions/Arrays_and_conditions_functions.md#ANY%28%29) - 如果条件对数组中的任何元素为真，返回 1
* [CONTAINS()](Functions/Arrays_and_conditions_functions.md#CONTAINS%28%29) - 检查 (x,y) 点是否在给定多边形内
* [IF()](Functions/Arrays_and_conditions_functions.md#IF%28%29) - 检查第一个参数是否等于 0.0，若不为零则返回第二个参数，否则返回第三个参数
* [IN()](Functions/Arrays_and_conditions_functions.md#IN%28%29) - 如果第一个参数等于其他任一参数，返回 1，否则返回 0
* [INDEXOF()](Functions/Arrays_and_conditions_functions.md#INDEXOF%28%29) - 遍历数组所有元素，返回第一个匹配元素的索引
* [INTERVAL()](Functions/Arrays_and_conditions_functions.md#INTERVAL%28%29) - 返回小于第一个参数的参数的索引
* [LENGTH()](Functions/Arrays_and_conditions_functions.md#LENGTH%28%29) - 返回 MVA 中元素的数量
* [REMAP()](Functions/Arrays_and_conditions_functions.md#REMAP%28%29) - 根据条件值对表达式值做一些例外处理

##### 日期和时间
* [NOW()](Functions/Date_and_time_functions.md#NOW%28%29) - 返回当前时间戳，类型为 INTEGER
* [CURTIME()](Functions/Date_and_time_functions.md#CURTIME%28%29) - 返回本地时区的当前时间
* [CURDATE()](Functions/Date_and_time_functions.md#CURDATE%28%29) - 返回本地时区的当前日期
* [UTC_TIME()](Functions/Date_and_time_functions.md#UTC_TIME%28%29) - 返回 UTC 时区的当前时间
* [UTC_TIMESTAMP()](Functions/Date_and_time_functions.md#UTC_TIMESTAMP%28%29) - 返回 UTC 时区的当前日期/时间
* [SECOND()](Functions/Date_and_time_functions.md#SECOND%28%29) - 返回时间戳参数的整数秒
* [MINUTE()](Functions/Date_and_time_functions.md#MINUTE%28%29) - 返回时间戳参数的整数分钟
* [HOUR()](Functions/Date_and_time_functions.md#HOUR%28%29) - 返回时间戳参数中的整数小时
* [DAY()](Functions/Date_and_time_functions.md#DAY%28%29) - 返回时间戳参数中的整数天
* [MONTH()](Functions/Date_and_time_functions.md#MONTH%28%29) - 返回时间戳参数中的整数月
* [QUARTER()](Functions/Date_and_time_functions.md#QUARTER%28%29) - 返回时间戳参数中的整数季度
* [YEAR()](Functions/Date_and_time_functions.md#YEAR%28%29) - 返回时间戳参数中的整数年份
* [DAYNAME()](Functions/Date_and_time_functions.md#DAYNAME%28%29) - 返回指定时间戳参数的星期名
* [MONTHNAME()](Functions/Date_and_time_functions.md#MONTHNAME%28%29) - 返回指定时间戳参数的月份名称
* [DAYOFWEEK()](Functions/Date_and_time_functions.md#DAYOFWEEK%28%29) - 返回指定时间戳参数的星期索引整数
* [DAYOFYEAR()](Functions/Date_and_time_functions.md#DAYOFYEAR%28%29) - 返回指定时间戳参数的当年天数整数
* [YEARWEEK()](Functions/Date_and_time_functions.md#YEARWEEK%28%29) - 返回指定时间戳参数的年份和当前周第一天的日期代码整数
* [YEARMONTH()](Functions/Date_and_time_functions.md#YEARMONTH%28%29) - 返回时间戳参数中的整数年份和月份代码
* [YEARMONTHDAY()](Functions/Date_and_time_functions.md#YEARMONTHDAY%28%29) - 返回时间戳参数中的整数年份、月份和日期代码
* [TIMEDIFF()](Functions/Date_and_time_functions.md#TIMEDIFF%28%29) - 返回时间戳之间的差值
* [DATEDIFF()](Functions/Date_and_time_functions.md#DATEDIFF%28%29) - 返回两个时间戳之间的天数
* [DATE()](Functions/Date_and_time_functions.md#DATE%28%29) - 格式化时间戳参数的日期部分
* [TIME()](Functions/Date_and_time_functions.md#TIME%28%29) - 格式化时间戳参数的时间部分
* [DATE_FORMAT()](Functions/Date_and_time_functions.md#DATE_FORMAT%28%29) - 根据给定的日期和格式参数返回格式化字符串


##### 地理空间
* [GEODIST()](Functions/Geo_spatial_functions.md#GEODIST%28%29) - 计算两点之间的地球表面距离
* [GEOPOLY2D()](Functions/Geo_spatial_functions.md#GEOPOLY2D%28%29) - 创建考虑地球曲率的多边形
* [POLY2D()](Functions/Geo_spatial_functions.md#POLY2D%28%29) - 创建平面空间中的简单多边形

##### 字符串
* [CONCAT()](Functions/String_functions.md#CONCAT%28%29) - 连接两个或多个字符串
* [REGEX()](Functions/String_functions.md#REGEX%28%29) - 如果正则表达式匹配属性字符串则返回1，否则返回0
* [SNIPPET()](Functions/String_functions.md#SNIPPET%28%29) - 高亮搜索结果
* [SUBSTRING_INDEX()](Functions/String_functions.md#SUBSTRING_INDEX%28%29) - 返回指定分隔符出现次数之前的字符串子串

##### 其他
* [CONNECTION_ID()](Functions/Other_functions.md#CONNECTION_ID%28%29) - 返回当前连接ID
* [KNN_DIST()](Functions/Other_functions.md#KNN_DIST%28%29) - 返回KNN向量搜索距离
* [LAST_INSERT_ID()](Functions/Other_functions.md#LAST_INSERT_ID%28%29) - 返回当前会话中最后语句插入或替换的文档ID
* [UUID_SHORT()](Functions/Other_functions.md#UUID_SHORT%28%29) - 返回“短”通用标识符，使用与自动ID生成相同的算法。

## 配置文件中的常见设置
放在配置文件的 `common {}` 部分：
* [lemmatizer_base](Server_settings/Common.md#lemmatizer_base) - 词形还原词典基路径
* [progressive_merge](Server_settings/Common.md#progressive_merge) - 定义实时表中磁盘块合并的顺序
* [json_autoconv_keynames](Server_settings/Common.md#json_autoconv_keynames) - 是否以及如何自动转换JSON属性中的键名
* [json_autoconv_numbers](Server_settings/Common.md#json_autoconv_numbers) - 自动检测并转换可能表示数字的JSON字符串为数字属性
* [on_json_attr_error](Server_settings/Common.md#on_json_attr_error) - 发现JSON格式错误时的处理方式
* [plugin_dir](Server_settings/Common.md#plugin_dir) - 动态库和UDFs的位置

## [Indexer](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments)
`indexer` 是一个用于创建 [plain tables](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) 的工具

##### 配置文件中的Indexer设置
放在配置文件的 `indexer {}` 部分：
* [lemmatizer_cache](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 词形还原缓存大小
* [max_file_field_buffer](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 最大文件字段自适应缓冲区大小
* [max_iops](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 最大索引I/O操作次数每秒
* [max_iosize](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 允许的最大I/O操作大小
* [max_xmlpipe2_field](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - XMLpipe2源类型允许的最大字段大小
* [mem_limit](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 索引RAM使用限制
* [on_file_field_error](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 如何处理文件字段中的IO错误
* [write_buffer](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 写缓冲区大小
* [ignore_non_plain](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 忽略关于非普通表的警告

##### 索引器启动参数
```bash
indexer [OPTIONS] [indexname1 [indexname2 [...]]]
```
* [--all](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 从配置中重建所有表
* [--buildstops](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 分析表源，仿佛索引数据一样，生成已索引词的列表
* [--buildfreqs](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 为 --buildstops 的表添加频率计数
* [--config, -c](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 指定配置文件路径
* [--dump-rows](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 将SQL源检索的行转储到指定文件中
* [--help](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 显示所有可用参数
* [--keep-attrs](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 允许在重新索引时重用现有属性
* [--keep-attrs-names](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 指定要从现有表中重用的属性
* [--merge-dst-range](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 在合并时应用给定的过滤范围
* [--merge-killlists](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 修改合并表时的kill列表处理
* [--merge](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 将两个普通表合并为一个
* [--nohup](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 启用此选项时阻止索引器发送SIGHUP信号
* [--noprogress](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 隐藏进度详情
* [--print-queries](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 输出索引器发送给数据库的SQL查询
* [--print-rt](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 显示从SQL源获取的数据作为实时表的INSERT语句
* [--quiet](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 关闭所有输出
* [--rotate](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 在所有表构建完成后启动表轮换
* [--sighup-each](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 每构建一个表就触发该表的轮换
* [-v](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 显示索引器版本

## Manticore v2 / Sphinx v2 的表转换器
`index_converter` 是一个工具，用于将使用 Sphinx/Manticore Search 2.x 创建的表转换为 Manticore Search 3.x 的表格式。
```bash
index_converter {--config /path/to/config|--path}
```
##### 表转换器启动参数
* [--config, -c](Installation/Migration_from_Sphinx.md#index_converter) - 表配置文件路径
* [--index](Installation/Migration_from_Sphinx.md#index_converter) - 指定要转换的表
* [--path](Installation/Migration_from_Sphinx.md#index_converter) - 设置包含表的路径，替代配置文件
* [--strip-path](Installation/Migration_from_Sphinx.md#index_converter) - 从表引用的文件名中去除路径
* [--large-docid](Installation/Migration_from_Sphinx.md#index_converter) - 允许转换ID大于2^63的文档
* [--output-dir](Installation/Migration_from_Sphinx.md#index_converter) - 在指定文件夹写入新文件
* [--all](Installation/Migration_from_Sphinx.md#index_converter) - 转换配置文件/路径中的所有表
* [--killlist-target](Installation/Migration_from_Sphinx.md#index_converter) - 设置应用kill-list的目标表

## [Searchd](Starting_the_server/Manually.md)
`searchd` 是 Manticore 服务器。

##### 配置文件中的 Searchd 设置
放置于配置文件的 `searchd {}` 区块中：
  * [access_blob_attrs](Server_settings/Searchd.md#access_blob_attrs) - 定义如何访问表的blob属性文件
  * [access_doclists](Server_settings/Searchd.md#access_doclists) - 定义如何访问表的doclists文件
  * [access_hitlists](Server_settings/Searchd.md#access_hitlists) - 定义如何访问表的hitlists文件
  * [access_plain_attrs](Server_settings/Searchd.md#access_plain_attrs) - 定义搜索服务器如何访问表的普通属性
  * [access_dict](Server_settings/Searchd.md#access_dict) - 定义如何访问表的字典文件
  * [agent_connect_timeout](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_connect_timeout) - 远程代理连接超时
  * [agent_query_timeout](Searching/Options.md#agent_query_timeout) - 远程代理查询超时
  * [agent_retry_count](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_connect_timeout) - 指定Manticore尝试连接和查询远程代理的次数
  * [agent_retry_delay](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) - 指定在失败时重试查询远程代理之前的延迟时间
  * [attr_flush_period](Data_creation_and_modification/Updating_documents/UPDATE.md#attr_flush_period) - 设置将更新的属性刷新到磁盘之间的时间间隔
  * [binlog_flush](Server_settings/Searchd.md#binlog_flush) - 二进制日志事务刷新/同步模式
  * [binlog_max_log_size](Server_settings/Searchd.md#binlog_max_log_size) - 最大二进制日志文件大小
  * [binlog_common](Logging/Binary_logging.md#Binary-logging-strategies) - 所有表的公共二进制日志文件
  * [binlog_filename_digits](Logging/Binary_logging.md#Log-files) - 二进制日志文件名中的数字位数
  * [binlog_flush](Logging/Binary_logging.md#Binary-flushing-strategies) - 二进制日志刷新策略
  * [binlog_path](Server_settings/Searchd.md#binlog_path) - 二进制日志文件路径
  * [client_timeout](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) - 使用持久连接时请求之间的最大等待时间
  * [collation_libc_locale](Server_settings/Searchd.md#collation_libc_locale) - 服务器 libc 区域设置
  * [collation_server](Server_settings/Searchd.md#collation_server) - 默认服务器排序规则
  * [data_dir](Server_settings/Searchd.md#data_dir) - Manticore 存储所有内容的数据目录路径 ([RT模式](Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29))
  * [diskchunk_flush_write_timeout](Server_settings/Searchd.md#diskchunk_flush_write_timeout) - 如果没有写入，自动刷新内存块的超时时间
  * [diskchunk_flush_search_timeout](Server_settings/Searchd.md#diskchunk_flush_search_timeout) - 如果表中没有搜索，防止自动刷新内存块的超时时间
  * [docstore_cache_size](Server_settings/Searchd.md#docstore_cache_size) - 保存在内存中的文档存储文档块的最大大小
  * [expansion_limit](Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#expansion_limit) - 单个通配符展开的最大关键字数量
  * [grouping_in_utc](Server_settings/Searchd.md#grouping_in_utc) - 启用对时间字段进行分组时使用UTC时区
  * [ha_period_karma](Server_settings/Searchd.md#ha_period_karma) - 代理镜像统计窗口大小
  * [ha_ping_interval](Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_ping_interval) - 代理镜像ping的间隔
  * [hostname_lookup](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) - 主机名更新策略
  * [jobs_queue_size](Server_settings/Searchd.md#jobs_queue_size) - 定义队列中允许的“作业”最大数量
  * [join_batch_size](Searching/Joining.md#Join-batching) - 定义表连接的批处理大小以平衡性能和内存使用
  * [join_cache_size](Searching/Joining.md#Join-caching) - 定义用于重用JOIN查询结果的缓存大小
  * [kibana_version_string](Server_settings/Searchd.md#kibana_version_string) – 服务器响应Kibana请求时发送的版本字符串
  * [listen](Server_settings/Searchd.md#listen) - 指定searchd监听的IP地址和端口或Unix域套接字路径
  * [listen_backlog](Server_settings/Searchd.md#listen_backlog) - TCP监听队列长度
  * [listen_tfo](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) - 为所有监听器启用TCP_FASTOPEN标志
  * [log](Server_settings/Searchd.md#log) - Manticore服务器日志文件路径
  * [max_batch_queries](Server_settings/Searchd.md#max_batch_queries) - 限制每批查询的数量
  * [max_connections](Server_settings/Searchd.md#max_connections) - 最大活动连接数
  * [max_filters](Server_settings/Searchd.md#max_filters) - 每查询允许的最大过滤器数量
  * [max_filter_values](Server_settings/Searchd.md#max_filter_values) - 每过滤器允许的最大值数量
  * [max_open_files](Server_settings/Searchd.md#max_open_files) - 服务器允许打开的最大文件数
  * [max_packet_size](Server_settings/Searchd.md#max_packet_size) - 允许的最大网络数据包大小
  * [mysql_version_string](Server_settings/Searchd.md#mysql_version_string) - 通过MySQL协议返回的服务器版本字符串
  * [net_throttle_accept](Server_settings/Searchd.md#net_throttle_accept) - 定义网络循环每次迭代接受多少客户端
  * [net_throttle_action](Server_settings/Searchd.md#net_throttle_action)  - 定义网络循环每次迭代处理多少请求
  * [net_wait_tm](Server_settings/Searchd.md#net_wait_tm) - 控制网络线程的忙循环间隔
  * [net_workers](Server_settings/Searchd.md#net_workers) - 网络线程数量
  * [network_timeout](Server_settings/Searchd.md#network_timeout) - 客户端请求网络超时
  * [node_address](Server_settings/Searchd.md#node_address) - 指定节点的网络地址
  * [persistent_connections_limit](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) - 远程持久代理的同时持久连接最大数量
  * [pid_file](Server_settings/Searchd.md#pid_file) - Manticore服务器pid文件路径
  * [predicted_time_costs](Server_settings/Searchd.md#predicted_time_costs) - 查询时间预测模型的成本
  * [preopen_tables](Server_settings/Searchd.md#preopen_tables) - 决定是否在启动时强制预打开所有表
  * [pseudo_sharding](Server_settings/Searchd.md#pseudo_sharding) - 为普通表和实时表的搜索查询启用伪分片
  * [qcache_max_bytes](Server_settings/Searchd.md#qcache_max_bytes) - 为缓存结果集分配的最大内存
  * [qcache_thresh_msec](Server_settings/Searchd.md#qcache_thresh_msec) - 查询结果被缓存的最短实际时间阈值
  * [qcache_ttl_sec](Server_settings/Searchd.md#qcache_ttl_sec) - 缓存结果集的过期时间
  * [query_log](Server_settings/Searchd.md#query_log) - 查询日志文件路径
  * [query_log_format](Server_settings/Searchd.md#query_log_format) - 查询日志格式
  * [query_log_min_msec](Server_settings/Searchd.md#query_log_min_msec) - 防止记录响应过快的查询
  * [query_log_mode](Server_settings/Searchd.md#query_log_mode) - 查询日志文件权限模式
  * [read_buffer_docs](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#read_buffer_docs) - 每关键词文档列表的读取缓冲区大小
  * [read_buffer_hits](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#read_buffer_docs) - 每关键词命中列表的读取缓冲区大小
  * [read_unhinted](Server_settings/Searchd.md#read_unhinted) - 未提示读取大小
  * [rt_flush_period](Server_settings/Searchd.md#rt_flush_period) - Manticore 多久将实时表的内存块刷新到磁盘
  * [rt_merge_iops](Server_settings/Searchd.md#rt_merge_iops) - 实时块合并线程允许执行的最大I/O操作次数（每秒）
  * [rt_merge_maxiosize](Server_settings/Searchd.md#rt_merge_maxiosize) - 实时块合并线程允许执行的最大单次I/O操作大小
  * [seamless_rotate](Server_settings/Searchd.md#seamless_rotate) - 防止在旋转拥有大量数据的表以进行预缓存时 searchd 卡顿
  * [secondary_indexes](Server_settings/Searchd.md#secondary_indexes) - 启用针对搜索查询的二级索引使用
  * [server_id](Server_settings/Searchd.md#server_id) - 用作生成唯一文档ID种子的服务器标识符
  * [shutdown_timeout](Server_settings/Searchd.md#shutdown_timeout) - Searchd `--stopwait` 超时设置
  * [shutdown_token](Server_settings/Searchd.md#shutdown_token) - 调用 VIP SQL 连接的 `shutdown` 命令所需的密码 SHA1 哈希值
  * [snippets_file_prefix](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) - 在 `load_files` 模式生成片段时，添加到本地文件名前缀
  * [sphinxql_state](Server_settings/Searchd.md#sphinxql_state) - 当前 SQL 状态序列化保存文件路径
  * [sphinxql_timeout](Server_settings/Searchd.md#sphinxql_timeout) - 等待 MySQL 客户端请求之间的最长时间
  * [ssl_ca](Server_settings/Searchd.md#ssl_ca) - SSL 证书颁发机构证书文件路径
  * [ssl_cert](Server_settings/Searchd.md#ssl_cert) - 服务器 SSL 证书路径
  * [ssl_key](Server_settings/Searchd.md#ssl_key) - 服务器 SSL 证书密钥路径
  * [subtree_docs_cache](Server_settings/Searchd.md#subtree_docs_cache) - 最大公共子树文档缓存大小
  * [subtree_hits_cache](Server_settings/Searchd.md#subtree_hits_cache) - 每查询最大公共子树命中缓存大小
  * [timezone](Server_settings/Searchd.md#timezone) - 日期/时间相关函数使用的时区
  * [thread_stack](Server_settings/Searchd.md#thread_stack) - 任务的最大堆栈大小
  * [unlink_old](Server_settings/Searchd.md#unlink_old) - 是否在成功旋转时取消链接旧表副本（.old）
  * [watchdog](Server_settings/Searchd.md#watchdog) - 是否启用 Manticore 服务器看门狗

##### Searchd 启动参数
```bash
searchd [OPTIONS]
```
* [--config, -c](Starting_the_server/Manually.md#searchd-command-line-options) - 指定配置文件路径
* [--console](Starting_the_server/Manually.md#searchd-command-line-options) - 强制服务器以控制台模式运行
* [--coredump](Starting_the_server/Manually.md#searchd-command-line-options) - 启用崩溃时的核心转储保存
* [--cpustats](Starting_the_server/Manually.md#searchd-command-line-options) - 启用 CPU 时间报告
* [--delete](Starting_the_server/Manually.md#searchd-command-line-options) - 从 Microsoft 管理控制台及其它服务注册位置移除 Manticore 服务
* [--force-preread](Starting_the_server/Manually.md#searchd-command-line-options) - 阻止服务器在预读表文件之前处理传入连接
* [--help, -h](Starting_the_server/Manually.md#searchd-command-line-options) - 显示所有可用参数
* [--table (--index)](Starting_the_server/Manually.md#searchd-command-line-options) - 限制服务器只服务指定的表
* [--install](Starting_the_server/Manually.md#searchd-command-line-options) - 在 Microsoft 管理控制台安装 searchd 作为服务
* [--iostats](Starting_the_server/Manually.md#searchd-command-line-options) - 启用输入/输出报告
* [--listen, -l](Starting_the_server/Manually.md#searchd-command-line-options) - 覆盖配置文件中的 [listen](Server_settings/Searchd.md#listen) 设置
* [--logdebug, --logdebugv, --logdebugvv](Starting_the_server/Manually.md#searchd-command-line-options) - 在服务器日志中启用额外的调试输出
* [--logreplication](Starting_the_server/Manually.md#searchd-command-line-options) - 在服务器日志中启用额外的复制调试输出
* [--new-cluster](Starting_the_server/Manually.md#searchd-command-line-options) - 初始化复制集群，并将服务器设置为带有 [集群重启](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md) 保护的参考节点
* [--new-cluster-force](Starting_the_server/Manually.md#searchd-command-line-options) - 初始化复制集群并将服务器设置为参考节点，绕过[集群重启](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md)保护
* [--nodetach](Starting_the_server/Manually.md#searchd-command-line-options) - 保持searchd在前台运行
* [--ntservice](Starting_the_server/Manually.md#searchd-command-line-options) - 由Microsoft管理控制台用于在Windows平台上以服务方式启动searchd
* [--pidfile](Starting_the_server/Manually.md#searchd-command-line-options) - 覆盖配置文件中的[pid_file](Server_settings/Searchd.md#pid_file)
* [--port, p](Starting_the_server/Manually.md#searchd-command-line-options) - 指定searchd监听端口，忽略配置文件中指定的端口
* [--replay-flags](Starting_the_server/Manually.md#searchd-command-line-options) - 设置额外的二进制日志重放选项
* [--servicename](Starting_the_server/Manually.md#searchd-command-line-options) - 在安装或删除服务时为searchd指定名称，显示在Microsoft管理控制台中
* [--status](Starting_the_server/Manually.md#searchd-command-line-options) - 查询正在运行的搜索服务以返回其状态
* [--stop](Starting_the_server/Manually.md#searchd-command-line-options) - 停止Manticore服务器
* [--stopwait](Starting_the_server/Manually.md#searchd-command-line-options) - 优雅地停止Manticore服务器
* [--strip-path](Starting_the_server/Manually.md#searchd-command-line-options) - 从表中引用的所有文件名中删除路径名
* [-v](Starting_the_server/Manually.md#searchd-command-line-options) - 显示版本信息

##### Searchd环境变量
* [MANTICORE_TRACK_DAEMON_SHUTDOWN](Starting_the_server/Manually.md#Environment-variables) - 启用searchd关闭期间的详细日志记录

## [Indextool](Miscellaneous_tools.md#indextool)
各种有助于故障排除的表维护功能。
```bash
indextool <command> [options]
```
##### Indextool启动参数
用于转储与物理表相关的各种调试信息。
```bash
indextool <command> [options]
```
* [--config, -c](Miscellaneous_tools.md#indextool) - 指定配置文件路径
* [--quiet, -q](Miscellaneous_tools.md#indextool) - 使indextool静默运行；无横幅输出等
* [--help, -h](Miscellaneous_tools.md#indextool) - 列出所有可用参数
* [-v](Miscellaneous_tools.md#indextool) - 显示版本信息
* [Indextool](Miscellaneous_tools.md#indextool) - 验证配置文件
* [--buildidf](Miscellaneous_tools.md#indextool) - 从一个或多个字典转储文件构建IDF文件
* [--build-infixes](Miscellaneous_tools.md#indextool) - 为现有dict=keywords表构建中缀词
* [--dumpheader](Miscellaneous_tools.md#indextool) - 快速转储提供的表头文件
* [--dumpconfig](Miscellaneous_tools.md#indextool) - 以近似符合manticore.conf格式转储给定表头文件的表定义
* [--dumpheader](Miscellaneous_tools.md#indextool) - 通过表名转储表头，并在配置文件中查找头文件路径
* [--dumpdict](Miscellaneous_tools.md#indextool) - 转储表字典
* [--dumpdocids](Miscellaneous_tools.md#indextool) - 按表名转储文档ID
* [--dumphitlist](Miscellaneous_tools.md#indextool) - 转储指定表中给定关键词/ID的所有出现
* [--docextract](Miscellaneous_tools.md#indextool) - 对整个字典/文档/命中运行表检查，收集属于请求文档的所有词和命中
* [--fold](Miscellaneous_tools.md#indextool) - 根据表设置测试分词
* [--htmlstrip](Miscellaneous_tools.md#indextool) - 使用指定表的HTML剥离设置过滤STDIN
* [--mergeidf](Miscellaneous_tools.md#indextool) - 将多个.idf文件合并为一个文件
* [--morph](Miscellaneous_tools.md#indextool) - 对提供的STDIN应用形态学处理并将结果输出到stdout
* [--check](Miscellaneous_tools.md#indextool) - 检查表数据文件的一致性
* [--check-id-dups](Miscellaneous_tools.md#indextool) - 检查重复ID
* [--check-disk-chunk](Miscellaneous_tools.md#indextool) - 检查RT表的单个磁盘块
* [--strip-path](Miscellaneous_tools.md#indextool) - 从表中引用的所有文件名中删除路径名
* [--rotate](Miscellaneous_tools.md#indextool) - 指定使用`--check`时是否检查等待旋转的表
* [--apply-killlists](Miscellaneous_tools.md#indextool) - 应用配置文件中列出所有表的杀死列表

## [Wordbreaker](Miscellaneous_tools.md#wordbreaker)
将复合词拆分为其组成部分。
```bash
wordbreaker [-dict path/to/dictionary_file] {split|test|bench}
```

##### Wordbreaker启动参数
* [STDIN](Miscellaneous_tools.md#wordbreaker) - 接受一个字符串以拆分成部分
* [-dict](Miscellaneous_tools.md#wordbreaker) - 指定要使用的字典文件
* [split|test|bench](Miscellaneous_tools.md#wordbreaker) - 指定命令

## [Spelldump](Miscellaneous_tools.md#spelldump)
使用ispell或MySpell格式提取字典文件内容

```bash
spelldump [options] <dictionary> <affix> [result] [locale-name]
```
* [dictionary](Miscellaneous_tools.md#spelldump) - 主字典文件
* [affix](Miscellaneous_tools.md#spelldump) - 字典的词缀文件
* [result](Miscellaneous_tools.md#spelldump) - 指定字典数据的输出目标
* [locale-name](Miscellaneous_tools.md#spelldump) - 指定使用的区域设置细节

## 预留关键字列表

Manticore SQL语法中当前保留（因此不能用作标识符）的关键字的完整字母表顺序列表。

```
AND, AS, BY, COLUMNARSCAN, DISTINCT, DIV, DOCIDINDEX, EXPLAIN, FACET, FALSE, FORCE, FROM, IGNORE, IN, INDEXES, INNER, IS, JOIN, KNN, LEFT, LIMIT, MOD, NOT, NO_COLUMNARSCAN, NO_DOCIDINDEX, NO_SECONDARYINDEX, NULL, OFFSET, ON, OR, ORDER, RELOAD, SECONDARYINDEX, SELECT, SYSFILTERS, TRUE
```

## 旧版Manticore文档

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
<!-- proofread -->

