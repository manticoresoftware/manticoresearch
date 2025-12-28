# 参考资料

### SQL 命令
##### 模式管理
* [CREATE TABLE](Creating_a_table/Local_tables/Real-time_table.md#CREATE-TABLE-command:) - 创建新表
* [CREATE TABLE LIKE](Creating_a_table/Local_tables/Real-time_table.md#CREATE-TABLE-LIKE:) - 使用另一张表作为模板创建表
* [CREATE TABLE LIKE ... WITH DATA](Creating_a_table/Local_tables/Real-time_table.md#CREATE-TABLE-LIKE:) - 复制表
* [CREATE SOURCE](Integration/Kafka.md#Source) - 创建 Kafka 消费者源
* [CREATE MATERIALIZED VIEW](Integration/Kafka.md#Materialized-view) - 从 Kafka 消息进行数据转换
* [CREATE MV](Integration/Kafka.md#Materialized-view) - 同上
* [DESCRIBE](Listing_tables.md#DESCRIBE) - 输出表的字段列表及其类型
* [ALTER TABLE](Updating_table_schema_and_settings.md) - 更改表模式 / 设置
* [ALTER TABLE REBUILD SECONDARY](Updating_table_schema_and_settings.md#Rebuilding-a-secondary-index) - 更新/恢复二级索引
* [ALTER TABLE type='distributed'](Updating_table_schema_and_settings.md#Changing-a-distributed-table) - 更新/恢复二级索引
* [ALTER TABLE RENAME](Updating_table_schema_and_settings.md#Renaming-a-real-time-table)
* [ALTER MATERIALIZED VIEW {name} suspended=1](Integration/Kafka.md#Altering-materialized-views) - 挂起或恢复从 Kafka 源的消费
* [DROP TABLE IF EXISTS](Deleting_a_table.md#Deleting-a-table) - 删除表（如果存在）
* [SHOW TABLES](Listing_tables.md#DESCRIBE) - 显示表列表
* [SHOW SOURCES](Integration/Kafka.md#Listing) - 显示 Kafka 源列表
* [SHOW MATERIALIZED VIEWS](Integration/Kafka.md#Listing) - 显示物化视图列表
* [SHOW MVS](Integration/Kafka.md#Listing) - 上一命令的别名
* [SHOW CREATE TABLE](Listing_tables.md#DESCRIBE) - 显示创建表的 SQL 命令
* [SHOW TABLE INDEXES](Node_info_and_management/Table_settings_and_status/SHOW_TABLE_INDEXES.md) - 显示表的可用二级索引信息
* [SHOW TABLE STATUS](Node_info_and_management/Table_settings_and_status/SHOW_TABLE_STATUS.md) - 显示当前表状态信息
* [SHOW TABLE SETTINGS](Node_info_and_management/Table_settings_and_status/SHOW_TABLE_SETTINGS.md) - 显示表设置
* [SHOW LOCKS](Securing_and_compacting_a_table/Freezing_and_locking_a_table.md#SHOW-LOCKS) - 显示冻结表的信息

##### 数据管理
* [INSERT](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md) - 添加新文档
* [REPLACE](Data_creation_and_modification/Updating_documents/REPLACE.md) - 用新文档替换已有文档
* [REPLACE .. SET](Data_creation_and_modification/Updating_documents/REPLACE.md?client=REPLACE+SET) - 替换表中一个或多个字段
* [UPDATE](Data_creation_and_modification/Updating_documents/UPDATE.md) - 对文档进行原地更新
* [DELETE](Data_creation_and_modification/Deleting_documents.md) - 删除文档
* [TRUNCATE TABLE](Emptying_a_table.md) - 删除表中的所有文档

##### 备份
* [BACKUP](Securing_and_compacting_a_table/Backup_and_restore.md#BACKUP-SQL-command-reference) - 备份表

##### SELECT
* [SELECT](Searching/Full_text_matching/Basic_usage.md#SQL) - 查询
  * [WHERE](Searching/Filters.md#Filters) - 过滤器
  * [GROUP BY](Searching/Grouping.md) - 分组查询结果
  * [GROUP BY ORDER](Searching/Grouping.md) - 对分组排序
  * [GROUP BY HAVING](Searching/Grouping.md) - 过滤分组
  * [OPTION](Searching/Options.md#OPTION) - 查询选项
  * [FACET](Searching/Faceted_search.md) - 分面搜索
  * [SUB-SELECTS](Searching/Sub-selects.md) - 关于使用 SELECT 子查询
  * [JOIN](Searching/Joining.md) - 在 SELECT 中连接表
* [EXPLAIN QUERY](Searching/Full_text_matching/Profiling.md#Profiling-without-running-a-query) - 显示查询执行计划但不运行查询
* [SHOW META](Node_info_and_management/SHOW_META.md) - 显示执行查询的扩展信息
* [SHOW PROFILE](Node_info_and_management/Profiling/Query_profile.md) - 显示执行查询的分析信息
* [SHOW PLAN](Searching/Full_text_matching/Profiling.md#Profiling-the-query-tree-in-SQL) - 显示查询执行计划（查询执行后）
* [SHOW WARNINGS](Node_info_and_management/SHOW_WARNINGS.md) - 显示最近一次查询的警告

##### 刷新各种内容
* [FLUSH ATTRIBUTES](Securing_and_compacting_a_table/Flushing_attributes.md) - 强制将更新后的属性刷新到磁盘
* [FLUSH HOSTNAMES](Securing_and_compacting_a_table/Flushing_hostnames.md) - 更新与代理主机名关联的 IP
* [FLUSH LOGS](Logging/Rotating_query_and_server_logs.md) - 重新打开 searchd 和查询日志文件（类似 USR1 信号）

##### 实时表优化
* [FLUSH RAMCHUNK](Securing_and_compacting_a_table/Flushing_RAM_chunk_to_a_new_disk_chunk.md#FLUSH-RAMCHUNK) - 强制创建新磁盘块
* [FLUSH TABLE](Securing_and_compacting_a_table/Flushing_RAM_chunk_to_disk.md#FLUSH-TABLE) - 将实时表的 RAM 块刷新到磁盘
* [OPTIMIZE TABLE](Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE) - 将实时表加入优化队列

##### 导入到实时表
* [ATTACH TABLE](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Attaching_one_table_to_another.md) - 将数据从普通表移动到实时表
* [IMPORT TABLE](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Importing_table.md) - 将先前创建的 RT 或 PQ 表导入正在 RT 模式运行的服务器

##### 复制
* [JOIN CLUSTER](Creating_a_cluster/Setting_up_replication/Joining_a_replication_cluster.md) - 加入复制集群
* [ALTER CLUSTER](Creating_a_cluster/Setting_up_replication/Managing_replication_nodes.md) - 向复制集群添加/删除表
* [SET CLUSTER](Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Cluster-parameters) - 更改复制集群设置
* [DELETE CLUSTER](Creating_a_cluster/Setting_up_replication/Deleting_a_replication_cluster.md) - 删除复制集群

##### 普通表轮换
* [RELOAD TABLE](Data_creation_and_modification/Adding_data_from_external_storages/Rotating_a_table.md#RELOAD-TABLE) - 轮换普通表
* [RELOAD TABLES](Data_creation_and_modification/Adding_data_from_external_storages/Rotating_a_table.md#RELOAD-TABLE) - 轮换所有普通表

##### 事务
* [BEGIN](Data_creation_and_modification/Transactions.md#BEGIN,-COMMIT,-and-ROLLBACK) - 开始一个事务
* [COMMIT](Data_creation_and_modification/Transactions.md#BEGIN,-COMMIT,-and-ROLLBACK) - 完成一个事务
* [ROLLBACK](Data_creation_and_modification/Transactions.md#BEGIN,-COMMIT,-and-ROLLBACK) - 回滚一个事务

##### CALL
* [CALL SUGGEST, CALL QSUGGEST](Searching/Spell_correction.md#CALL-QSUGGEST,-CALL-SUGGEST) - 建议拼写纠正单词
* [CALL SNIPPETS](Searching/Highlighting.md) - 根据提供的数据和查询构建高亮结果摘要
* [CALL PQ](Searching/Percolate_query.md) - 运行反向查询
* [CALL KEYWORDS](Searching/Autocomplete.md#CALL-KEYWORDS) - 用于检查关键词的分词方式。并允许获取给定关键词的分词形式
* [CALL AUTOCOMPLETE](Searching/Autocomplete.md#CALL-AUTOCOMPLETE) - 自动完成搜索查询

##### 插件
* [CREATE FUNCTION](Extensions/UDFs_and_Plugins/UDF/Creating_a_function.md) - 安装用户定义函数（UDF）
* [DROP FUNCTION](Extensions/UDFs_and_Plugins/UDF/Deleting_a_function.md) - 删除用户定义函数（UDF）
* [CREATE PLUGIN](Extensions/UDFs_and_Plugins/Plugins/Creating_a_plugin.md) - 安装插件
* [CREATE BUDDY PLUGIN](Extensions/UDFs_and_Plugins/Plugins/Creating_a_plugin.md#CREATE-BUDDY-PLUGIN) - 安装 Buddy 插件
* [DROP PLUGIN](Extensions/UDFs_and_Plugins/Plugins/Deleting_a_plugin.md#DELETE-PLUGIN) - 删除插件
* [DROP BUDDY PLUGIN](Extensions/UDFs_and_Plugins/Plugins/Deleting_a_plugin.md#DELETE-BUDDY-PLUGIN) - 删除 Buddy 插件
* [RELOAD PLUGINS](Extensions/UDFs_and_Plugins/Plugins/Reloading_plugins.md) - 从指定库重新加载所有插件
* [ENABLE BUDDY PLUGIN](Extensions/UDFs_and_Plugins/Plugins/Enabling_and_disabling_buddy_plugins.md#ENABLE-BUDDY-PLUGIN) - 重新激活先前禁用的 Buddy 插件
* [DISABLE BUDDY PLUGIN](Extensions/UDFs_and_Plugins/Plugins/Enabling_and_disabling_buddy_plugins.md#DISABLE-BUDDY-PLUGIN) - 停用已激活的 Buddy 插件

##### 服务器状态
* [SHOW STATUS](Node_info_and_management/Node_status.md#SHOW-STATUS) - 显示多个有用的性能计数器
* [SHOW THREADS](Node_info_and_management/SHOW_THREADS.md) - 列出所有当前活动的客户端线程
* [SHOW VARIABLES](Node_info_and_management/SHOW_VARIABLES.md) - 列出服务器范围内的变量及其值
* [SHOW VERSION](Node_info_and_management/SHOW_VERSION.md#SHOW-VERSION) - 提供实例各种组件的详细版本信息

### HTTP 端点
* [/sql](Connecting_to_the_server/HTTP.md#SQL-over-HTTP) - 通过 HTTP JSON 执行 SQL 语句
* [/cli](Connecting_to_the_server/HTTP.md#/cli) - 提供 HTTP 命令行界面
* [/insert](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md) - 向实时表插入文档
* [/pq/tbl_name/doc](Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md#Adding-rules-to-a-percolate-table) - 向反向查询表添加 PQ 规则
* [/update](Data_creation_and_modification/Updating_documents/UPDATE.md#Updates-via-HTTP-JSON) - 更新实时表中的文档
* [/replace](Data_creation_and_modification/Updating_documents/REPLACE.md) - 替换实时表中已存在的文档，若不存在则插入
* [/pq/tbl_name/doc/N?refresh=1](Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md#Adding-rules-to-a-percolate-table) - 替换反向查询表中的 PQ 规则
* [/delete](Data_creation_and_modification/Deleting_documents.md) - 从表中删除文档
* [/bulk](Data_creation_and_modification/Updating_documents/UPDATE.md#Bulk-updates) - 在单次调用中执行多个插入、更新或删除操作。更多批量插入内容请见[这里](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md)。
* [/search](Searching/Full_text_matching/Basic_usage.md#HTTP-JSON) - 执行搜索
* [/search -> knn](Searching/KNN.md) - 执行 KNN 向量搜索
* [/pq/tbl_name/search](Searching/Percolate_query.md) - 在反向查询表中执行反向搜索
* [/tbl_name/_mapping](Creating_a_table/Local_tables/Real-time_table.md#_mapping-API:) - 以 Elasticsearch 风格创建表模式

### 常用内容
* [字段名称语法](Creating_a_table/Data_types.md#Field-name-syntax)
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

##### 平凡表设置
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

##### 实时表设置
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


## 全文搜索运算符
* [OR](Searching/Full_text_matching/Operators.md#OR-operator)
* [MAYBE](Searching/Full_text_matching/Operators.md#MAYBE-operator)
* [NOT](Searching/Full_text_matching/Operators.md#Negation-operator) - NOT 运算符
* [@field](Searching/Full_text_matching/Operators.md#Field-search-operator) - 字段搜索运算符
* [@field%5BN%5D](Searching/Full_text_matching/Operators.md#Field-search-operator) - 字段位置限制修饰符
* [@(field1,field2,...)](Searching/Full_text_matching/Operators.md#Field-search-operator) - 多字段搜索运算符
* [@!field](Searching/Full_text_matching/Operators.md#Field-search-operator) - 忽略字段搜索运算符
* [@!(field1,field2,...)](Searching/Full_text_matching/Operators.md#Field-search-operator) - 忽略多字段搜索运算符
* [@*](Searching/Full_text_matching/Operators.md#Field-search-operator) - 所有字段搜索运算符
* ["word1 word2 ... "](Searching/Full_text_matching/Operators.md#Phrase-search-operator) - 短语搜索运算符
* ["word1 word2 ... "~N](Searching/Full_text_matching/Operators.md#Proximity-search-operator) - 接近搜索运算符
* ["word1 word2 ... "/N](Searching/Full_text_matching/Operators.md#Quorum-matching-operator) - 协议匹配运算符
* [word1 << word2 << word3](Searching/Full_text_matching/Operators.md#Strict-order-operator) - 严格顺序运算符
* [=word1](Searching/Full_text_matching/Operators.md#Exact-form-modifier) - 精确形式修饰符
* [^word1](Searching/Full_text_matching/Operators.md#Field-start-and-field-end-modifier) - 字段开始修饰符
* [word2$](Searching/Full_text_matching/Operators.md#Field-start-and-field-end-modifier) - 字段结束修饰符
* [word^N](Searching/Full_text_matching/Operators.md#IDF-boost-modifier) - 关键词 IDF 增强修饰符
* [word1 NEAR/N word2](Searching/Full_text_matching/Operators.md#NEAR-operator) - NEAR，泛化接近运算符
* [word1 NOTNEAR/N word2](Searching/Full_text_matching/Operators.md#NOTNEAR-operator) - NOTNEAR，否定断言运算符
* [word1 PARAGRAPH word2 PARAGRAPH "word3 word4"](Searching/Full_text_matching/Operators.md#SENTENCE-and-PARAGRAPH-operators) - PARAGRAPH 运算符
* [word1 SENTENCE word2 SENTENCE "word3 word4"](Searching/Full_text_matching/Operators.md#SENTENCE-and-PARAGRAPH-operators) - SENTENCE 运算符
* [ZONE:(h3,h4)](Searching/Full_text_matching/Operators.md#ZONE-limit-operator) - ZONE 限制运算符
* [ZONESPAN:(h2)](Searching/Full_text_matching/Operators.md#ZONESPAN-limit-operator) - ZONESPAN 限制运算符
* [@@relaxed](Searching/Full_text_matching/Operators.md#Field-search-operator) - 抑制缺少字段的错误
* [t?st](Searching/Full_text_matching/Operators.md#Wildcard-operators) - 通配符运算符
* [REGEX(/pattern/)](Searching/Full_text_matching/Operators.md#REGEX-operator) - REGEX 运算符

## 函数
##### 数学
* [ABS()](Functions/Mathematical_functions.md#ABS%28%29) - 返回绝对值
* [ATAN2()](Functions/Mathematical_functions.md#ATAN2%28%29) - 返回两个参数的反正切函数
* [BITDOT()](Functions/Mathematical_functions.md#BITDOT%28%29) - 返回每个位掩码位与权重相乘后的和
* [CEIL()](Functions/Mathematical_functions.md#CEIL%28%29) - 返回大于或等于参数的最小整数值
* [COS()](Functions/Mathematical_functions.md#COS%28%29) - 返回参数的余弦值
* [CRC32()](Functions/Mathematical_functions.md#CRC32%28%29) - 返回参数的CRC32值
* [EXP()](Functions/Mathematical_functions.md#EXP%28%29) - 返回参数的指数值
* [FIBONACCI()](Functions/Mathematical_functions.md#FIBONACCI%28%29) - 返回第N个斐波那契数，N是整数参数
* [FLOOR()](Functions/Mathematical_functions.md#FLOOR%28%29) - 返回小于或等于参数的最大整数值
* [GREATEST()](Functions/Mathematical_functions.md#GREATEST%28%29) - 以JSON/MVA数组作为参数，返回该数组中的最大值
* [IDIV()](Functions/Mathematical_functions.md#IDIV%28%29) - 返回第一个参数除以第二个参数的整数结果
* [LEAST()](Functions/Mathematical_functions.md#LEAST%28%29) - 以JSON/MVA数组作为参数，返回该数组中的最小值
* [LN()](Functions/Mathematical_functions.md#LN%28%29) - 返回参数的自然对数
* [LOG10()](Functions/Mathematical_functions.md#LOG10%28%29) - 返回参数的常用对数
* [LOG2()](Functions/Mathematical_functions.md#LOG2%28%29) - 返回参数的二进制对数
* [MAX()](Functions/Mathematical_functions.md#MAX%28%29) - 返回两个参数中的较大值
* [MIN()](Functions/Mathematical_functions.md#MIN%28%29) - 返回两个参数中的较小值
* [POW()](Functions/Mathematical_functions.md#POW%28%29) - 返回第一个参数的第二个参数次方
* [RAND()](Functions/Mathematical_functions.md#RAND%28%29) - 返回0到1之间的随机浮点数
* [SIN()](Functions/Mathematical_functions.md#SIN%28%29) - 返回参数的正弦值
* [SQRT()](Functions/Mathematical_functions.md#SQRT%28%29) - 返回参数的平方根


##### 搜索和排名
* [BM25F()](Functions/Searching_and_ranking_functions.md#BM25F%28%29) - 返回精确的BM25F公式值
* [EXIST()](Functions/Searching_and_ranking_functions.md#EXIST%28%29) - 用默认值替换不存在的列
* [GROUP_CONCAT()](Searching/Grouping.md#GROUP_CONCAT%28field%29) - 生成分组中属性值的逗号分隔列表
* [HIGHLIGHT()](Searching/Highlighting.md) - 高亮搜索结果
* [MIN_TOP_SORTVAL()](Functions/Searching_and_ranking_functions.md#MIN_TOP_SORTVAL%28%29) - 返回当前前N匹配中最差元素的排序键值
* [MIN_TOP_WEIGHT()](Functions/Searching_and_ranking_functions.md#MIN_TOP_WEIGHT%28%29) - 返回当前前N匹配中最差元素的权重
* [PACKEDFACTORS()](Functions/Searching_and_ranking_functions.md#PACKEDFACTORS%28%29) - 输出权重因子
* [REMOVE_REPEATS()](Functions/Searching_and_ranking_functions.md#REMOVE_REPEATS%28%29) - 删除具有相同'column'值的重复调整行
* [WEIGHT()](Functions/Searching_and_ranking_functions.md#WEIGHT%28%29) - 返回全文匹配得分
* [ZONESPANLIST()](Functions/Searching_and_ranking_functions.md#ZONESPANLIST%28%29) - 返回匹配区域区间的对
* [QUERY()](Functions/Searching_and_ranking_functions.md#QUERY%28%29) - 返回当前全文查询

##### 类型转换
* [BIGINT()](Functions/Type_casting_functions.md#BIGINT%28%29) - 强制将整数参数转换为64位类型
* [DOUBLE()](Functions/Type_casting_functions.md#DOUBLE%28%29) - 强制将给定参数转换为浮点类型
* [INTEGER()](Functions/Type_casting_functions.md#INTEGER%28%29) - 强制将给定参数转换为64位带符号类型
* [TO_STRING()](Functions/Type_casting_functions.md#TO_STRING%28%29) - 强制将参数转换为字符串类型
* [UINT()](Functions/Type_casting_functions.md#UINT%28%29) - 将给定参数转换为32位无符号整数类型
* [UINT64()](Functions/Type_casting_functions.md#UINT64%28%29) - 将给定参数转换为64位无符号整数类型
* [SINT()](Functions/Type_casting_functions.md#SINT%28%29) - 将32位无符号整数解释为64位带符号整数

##### 数组和条件
* [ALL()](Functions/Arrays_and_conditions_functions.md#ALL%28%29) - 如果数组中所有元素的条件为真，则返回1
* [ANY()](Functions/Arrays_and_conditions_functions.md#ANY%28%29) - 如果数组中任何元素的条件为真，则返回1
* [CONTAINS()](Functions/Arrays_and_conditions_functions.md#CONTAINS%28%29) - 检查给定多边形内的（x，y）点
* [IF()](Functions/Arrays_and_conditions_functions.md#IF%28%29) - 检查第一个参数是否等于0.0，如果非零则返回第二个参数，如果为零则返回第三个参数
* [IN()](Functions/Arrays_and_conditions_functions.md#IN%28%29) - 如果第一个参数等于其他参数中的任何一个，则返回1，否则返回0
* [INDEXOF()](Functions/Arrays_and_conditions_functions.md#INDEXOF%28%29) - 遍历数组中的所有元素并返回第一个匹配元素的索引
* [INTERVAL()](Functions/Arrays_and_conditions_functions.md#INTERVAL%28%29) - 返回小于第一个参数的参数的索引
* [LENGTH()](Functions/Arrays_and_conditions_functions.md#LENGTH%28%29) - 返回MVA中的元素数量
* [REMAP()](Functions/Arrays_and_conditions_functions.md#REMAP%28%29) - 根据条件值对表达式的值进行一些例外处理

##### 日期和时间
* [NOW()](Functions/Date_and_time_functions.md#NOW%28%29) - 返回当前时间戳作为整数
* [CURTIME()](Functions/Date_and_time_functions.md#CURTIME%28%29) - 返回本地时区的当前时间
* [CURDATE()](Functions/Date_and_time_functions.md#CURDATE%28%29) - 返回本地时区的当前日期
* [UTC_TIME()](Functions/Date_and_time_functions.md#UTC_TIME%28%29) - 返回UTC时区的当前时间
* [UTC_TIMESTAMP()](Functions/Date_and_time_functions.md#UTC_TIMESTAMP%28%29) - 返回UTC时区的当前日期/时间
* [SECOND()](Functions/Date_and_time_functions.md#SECOND%28%29) - 从时间戳参数返回整数秒
* [MINUTE()](Functions/Date_and_time_functions.md#MINUTE%28%29) - 从时间戳参数返回整数分钟
* [HOUR()](Functions/Date_and_time_functions.md#HOUR%28%29) - 从时间戳参数返回整数小时
* [DAY()](Functions/Date_and_time_functions.md#DAY%28%29) - 从时间戳参数返回整数天
* [MONTH()](Functions/Date_and_time_functions.md#MONTH%28%29) - 从时间戳参数返回整数月份
* [QUARTER()](Functions/Date_and_time_functions.md#QUARTER%28%29) - 从时间戳参数返回当年的整数季度
* [YEAR()](Functions/Date_and_time_functions.md#YEAR%28%29) - 从时间戳参数返回整数年份
* [DAYNAME()](Functions/Date_and_time_functions.md#DAYNAME%28%29) - 从给定的时间戳参数返回星期几名称
* [MONTHNAME()](Functions/Date_and_time_functions.md#MONTHNAME%28%29) - 从给定的时间戳参数返回月份名称
* [DAYOFWEEK()](Functions/Date_and_time_functions.md#DAYOFWEEK%28%29) - 从给定的时间戳参数返回星期几索引
* [DAYOFYEAR()](Functions/Date_and_time_functions.md#DAYOFYEAR%28%29) - 从给定的时间戳参数返回一年中的整数天数
* [YEARWEEK()](Functions/Date_and_time_functions.md#YEARWEEK%28%29) - 从给定的时间戳参数返回当前周的整数年和天码
* [YEARMONTH()](Functions/Date_and_time_functions.md#YEARMONTH%28%29) - 从时间戳参数返回整数年和月码
* [YEARMONTHDAY()](Functions/Date_and_time_functions.md#YEARMONTHDAY%28%29) - 从时间戳参数返回整数年、月和日码
* [TIMEDIFF()](Functions/Date_and_time_functions.md#TIMEDIFF%28%29) - 返回两个时间戳之间的差值
* [DATEDIFF()](Functions/Date_and_time_functions.md#DATEDIFF%28%29) - 返回两个给定时间戳之间相差的天数
* [DATE()](Functions/Date_and_time_functions.md#DATE%28%29) - 格式化时间戳参数中的日期部分
* [TIME()](Functions/Date_and_time_functions.md#TIME%28%29) - 格式化时间戳参数中的时间部分
* [DATE_FORMAT()](Functions/Date_and_time_functions.md#DATE_FORMAT%28%29) - 根据提供的日期和格式参数返回一个格式化的字符串


##### 地理空间
* [GEODIST()](Functions/Geo_spatial_functions.md#GEODIST%28%29) - 计算两个给定点之间的地表距离
* [GEOPOLY2D()](Functions/Geo_spatial_functions.md#GEOPOLY2D%28%29) - 创建一个考虑地球曲率的多边形
* [POLY2D()](Functions/Geo_spatial_functions.md#POLY2D%28%29) - 在平面空间中创建一个简单的多边形

##### 字符串
* [CONCAT()](Functions/String_functions.md#CONCAT%28%29) - 连接两个或多个字符串
* [REGEX()](Functions/String_functions.md#REGEX%28%29) - 如果正则表达式匹配字符串或属性，则返回1，否则返回0
* [SNIPPET()](Functions/String_functions.md#SNIPPET%28%29) - 突出显示搜索结果
* [SUBSTRING_INDEX()](Functions/String_functions.md#SUBSTRING_INDEX%28%29) - 返回在指定分隔符出现次数之前的时间戳子字符串

##### 其他
* [CONNECTION_ID()](Functions/Other_functions.md#CONNECTION_ID%28%29) - 返回当前连接ID
* [KNN_DIST()](Functions/Other_functions.md#KNN_DIST%28%29) - 返回KNN向量搜索距离
* [LAST_INSERT_ID()](Functions/Other_functions.md#LAST_INSERT_ID%28%29) - 返回由当前会话中的最后语句插入或替换的文档ID
* [UUID_SHORT()](Functions/Other_functions.md#UUID_SHORT%28%29) - 返回遵循与自动ID生成相同的算法的“短”全局唯一标识符。

## 配置文件中的常见设置
应放在配置文件的 `common {}` 部分：
* [lemmatizer_base](Server_settings/Common.md#lemmatizer_base) - 词干化字典基础路径
* [progressive_merge](Server_settings/Common.md#progressive_merge) - 定义实时表中磁盘块合并的顺序
* [json_autoconv_keynames](Server_settings/Common.md#json_autoconv_keynames) - 是否以及如何自动转换JSON属性内的键名
* [json_autoconv_numbers](Server_settings/Common.md#json_autoconv_numbers) - 自动检测并转换可能表示数字的JSON字符串为数值属性
* [on_json_attr_error](Server_settings/Common.md#on_json_attr_error) - 如果发现JSON格式错误应采取什么措施
* [plugin_dir](Server_settings/Common.md#plugin_dir) - 动态库和UDF的位置

## [索引器](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments)
`indexer` 是一个工具用于创建 [普通表](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments)

##### 配置文件中的索引器设置
放置在配置文件的 `indexer {}` 部分：
* [lemmatizer_cache](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 词元化器缓存大小
* [max_file_field_buffer](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 文件字段自适应缓冲区最大大小
* [max_iops](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 最大索引 I/O 操作次数每秒
* [max_iosize](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 最大允许的 I/O 操作大小
* [max_xmlpipe2_field](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - XMLpipe2 源类型允许的字段最大大小
* [mem_limit](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 索引时的内存使用限制
* [on_file_field_error](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 文件字段中如何处理 I/O 错误
* [write_buffer](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 写缓冲区大小
* [ignore_non_plain](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 忽略关于非纯表的警告

##### 索引器启动参数
```bash
indexer [OPTIONS] [indexname1 [indexname2 [...]]]
```
* [--all](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 从配置中重建所有表
* [--buildstops](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 分析表源，如同索引数据一样，生成索引词列表
* [--buildfreqs](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 为 --buildstops 添加词频计数到表中
* [--config, -c](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 指定配置文件路径
* [--dump-rows](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 将从 SQL 源检索的行转储到指定文件
* [--help](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 显示所有可用参数
* [--keep-attrs](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 重新索引时允许重用现有属性
* [--keep-attrs-names](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 指定从现有表中重用的属性
* [--merge-dst-range](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 合并时应用给定的过滤范围
* [--merge-killlists](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 合并表时改变 kill 列表处理方式
* [--merge](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 将两个纯表合并为一个
* [--nohup](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 启用该选项时，防止索引器发送 SIGHUP
* [--noprogress](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 隐藏进度详情
* [--print-queries](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 输出索引器发送到数据库的 SQL 查询
* [--print-rt](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 以插入语句形式显示从 SQL 源获取到的实时表数据
* [--quiet](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 抑制所有输出
* [--rotate](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 所有表构建完成后启动表轮换
* [--sighup-each](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 每个表构建完成后触发轮换
* [-v](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 显示索引器版本

## Manticore v2 / Sphinx v2 的表转换器
`index_converter` 是一个用于将 Sphinx/Manticore Search 2.x 创建的表转换为 Manticore Search 3.x 表格式的工具。
```bash
index_converter {--config /path/to/config|--path}
```
##### 表转换器启动参数
* [--config, -c](Installation/Migration_from_Sphinx.md#index_converter) - 表配置文件的路径
* [--index](Installation/Migration_from_Sphinx.md#index_converter) - 指定要转换的表
* [--path](Installation/Migration_from_Sphinx.md#index_converter) - 设置包含表的路径，代替配置文件
* [--strip-path](Installation/Migration_from_Sphinx.md#index_converter) - 从表引用的文件名中删除路径
* [--large-docid](Installation/Migration_from_Sphinx.md#index_converter) - 允许转换文档 ID 大于 2^63 的文档
* [--output-dir](Installation/Migration_from_Sphinx.md#index_converter) - 在指定文件夹中写入新文件
* [--all](Installation/Migration_from_Sphinx.md#index_converter) - 转换配置文件 / 路径中的所有表
* [--killlist-target](Installation/Migration_from_Sphinx.md#index_converter) - 设置用于应用 kill-lists 的目标表

## [Searchd](Starting_the_server/Manually.md)
`searchd` 是 Manticore 服务器。

##### 搜索d设置在配置文件中
要在配置文件的`searchd {}`部分放置：
  * [access_blob_attrs](Server_settings/Searchd.md#access_blob_attrs) - 定义如何访问表的blob属性文件
  * [access_doclists](Server_settings/Searchd.md#access_doclists) - 定义如何访问表的doclists文件
  * [access_hitlists](Server_settings/Searchd.md#access_hitlists) - 定义如何访问表的hitlists文件
  * [access_plain_attrs](Server_settings/Searchd.md#access_plain_attrs) - 定义搜索服务器如何访问表的普通属性
  * [access_dict](Server_settings/Searchd.md#access_dict) - 定义如何访问表的字典文件
  * [agent_connect_timeout](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_connect_timeout) - 远程代理连接超时
  * [agent_query_timeout](Searching/Options.md#agent_query_timeout) - 远程代理查询超时
  * [agent_retry_count](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_connect_timeout) - 指定Manticore尝试连接和查询远程代理的次数
  * [agent_retry_delay](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) - 在失败后指定在重新查询远程代理之前的时间延迟
  * [attr_flush_period](Data_creation_and_modification/Updating_documents/UPDATE.md#attr_flush_period) - 设置更新属性刷新到磁盘的时间间隔
  * [binlog_flush](Server_settings/Searchd.md#binlog_flush) - 二进制日志事务刷新/同步模式
  * [binlog_max_log_size](Server_settings/Searchd.md#binlog_max_log_size) - 二进制日志文件的最大大小
  * [binlog_common](Logging/Binary_logging.md#Binary-logging-strategies) - 所有表的公共二进制日志文件
  * [binlog_filename_digits](Logging/Binary_logging.md#Log-files) - 二进制日志文件名中的数字位数
  * [binlog_flush](Logging/Binary_logging.md#Binary-flushing-strategies) - 二进制日志刷新策略
  * [binlog_path](Server_settings/Searchd.md#binlog_path) - 二进制日志文件路径
  * [client_timeout](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) - 使用持久连接时等待请求之间的最大时间
  * [collation_libc_locale](Server_settings/Searchd.md#collation_libc_locale) - 服务器libc区域设置
  * [collation_server](Server_settings/Searchd.md#collation_server) - 默认服务器排序规则
  * [data_dir](Server_settings/Searchd.md#data_dir) - 数据目录路径，Manticore在此存储所有内容（[RT模式](Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29））
  * [diskchunk_flush_write_timeout](Server_settings/Searchd.md#diskchunk_flush_write_timeout) - 如果没有写入，则自动刷新RAM块的超时时间
  * [diskchunk_flush_search_timeout](Server_settings/Searchd.md#diskchunk_flush_search_timeout) - 如果表中没有搜索，则防止自动刷新RAM块的超时时间
  * [docstore_cache_size](Server_settings/Searchd.md#docstore_cache_size) - 存储在内存中的文档块的最大大小
  * [expansion_limit](Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#expansion_limit) - 单个通配符的最大扩展关键词数量
  * [grouping_in_utc](Server_settings/Searchd.md#grouping_in_utc) - 启用使用UTC时区进行分组时间字段
  * [ha_period_karma](Server_settings/Searchd.md#ha_period_karma) - 剂量统计窗口大小
  * [ha_ping_interval](Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_ping_interval) - 剂量ping间隔
  * [hostname_lookup](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) - 主机名重置策略
  * [jobs_queue_size](Server_settings/Searchd.md#jobs_queue_size) - 定义同时允许的最大“任务”数量
  * [join_batch_size](Searching/Joining.md#Join-batching) - 定义表连接的批处理大小以平衡性能和内存使用
  * [join_cache_size](Searching/Joining.md#Join-caching) - 定义用于重用JOIN查询结果的缓存大小
  * [kibana_version_string](Server_settings/Searchd.md#kibana_version_string) – 服务器版本字符串，响应Kibana请求时发送
  * [listen](Server_settings/Searchd.md#listen) - 指定searchd监听的IP地址和端口或Unix域套接字路径
  * [listen_backlog](Server_settings/Searchd.md#listen_backlog) - TCP监听回退
  * [listen_tfo](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) - 为所有监听器启用TCP_FASTOPEN标志
  * [log](Server_settings/Searchd.md#log) - Manticore服务器日志文件路径
  * [max_batch_queries](Server_settings/Searchd.md#max_batch_queries) - 每批限制查询数量
  * [max_connections](Server_settings/Searchd.md#max_connections) - 最大活动连接数
  * [max_filters](Server_settings/Searchd.md#max_filters) - 每个查询允许的最大过滤器计数
  * [max_filter_values](Server_settings/Searchd.md#max_filter_values) - 每个过滤器允许的最大值计数
  * [max_open_files](Server_settings/Searchd.md#max_open_files) - 服务器允许打开的最大文件数
  * [max_packet_size](Server_settings/Searchd.md#max_packet_size) - 允许的最大网络数据包大小
  * [mysql_version_string](Server_settings/Searchd.md#mysql_version_string) - 通过MySQL协议返回的服务器版本字符串
  * [net_throttle_accept](Server_settings/Searchd.md#net_throttle_accept) - 每次网络循环接受客户端的数量
  * [net_throttle_action](Server_settings/Searchd.md#net_throttle_action)  - 每次网络循环处理请求的数量
  * [net_wait_tm](Server_settings/Searchd.md#net_wait_tm) - 控制网络线程的忙循环间隔
  * [net_workers](Server_settings/Searchd.md#net_workers) - 网络线程的数量
  * [network_timeout](Server_settings/Searchd.md#network_timeout) - 客户端请求的网络超时
  * [node_address](Server_settings/Searchd.md#node_address) - 指定节点的网络地址
  * [persistent_connections_limit](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) - 同时连接到远程持久代理的最大持久连接数
  * [pid_file](Server_settings/Searchd.md#pid_file) - Manticore服务器pid文件路径
  * [predicted_time_costs](Server_settings/Searchd.md#predicted_time_costs) - 查询时间预测模型的成本
  * [preopen_tables](Server_settings/Searchd.md#preopen_tables) - 确定是否在启动时强制预打开所有表
  * [pseudo_sharding](Server_settings/Searchd.md#pseudo_sharding) - 使搜索查询能够使用普通和实时表的伪分片
  * [qcache_max_bytes](Server_settings/Searchd.md#qcache_max_bytes) - 分配给缓存结果集的最大RAM
  * [qcache_thresh_msec](Server_settings/Searchd.md#qcache_thresh_msec) - 防止缓存查询结果的最小墙钟时间阈值
  * [qcache_ttl_sec](Server_settings/Searchd.md#qcache_ttl_sec) - 缓存结果集的过期周期
  * [query_log](Server_settings/Searchd.md#query_log) - 查询日志文件路径
  * [query_log_format](Server_settings/Searchd.md#query_log_format) - 查询日志格式
  * [query_log_min_msec](Server_settings/Searchd.md#query_log_min_msec) - 防止记录太快的查询
  * [query_log_mode](Server_settings/Searchd.md#query_log_mode) - 查询日志文件权限模式
  * [read_buffer_docs](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#read_buffer_docs) - 文档列表的每个关键词读缓冲区大小
  * [read_buffer_hits](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#read_buffer_docs) - 击中列表的每个关键词读缓冲区大小
  * [read_unhinted](Server_settings/Searchd.md#read_unhinted) - 未提示读取大小
  * [rt_flush_period](Server_settings/Searchd.md#rt_flush_period) - Manticore定期刷新实时表的RAM块到磁盘的频率
  * [rt_merge_iops](Server_settings/Searchd.md#rt_merge_iops) - 实时块合并线程每秒允许执行的最大I/O操作数
  * [rt_merge_maxiosize](Server_settings/Searchd.md#rt_merge_maxiosize) - 实时块合并线程允许执行的最大I/O操作大小
  * [seamless_rotate](Server_settings/Searchd.md#seamless_rotate) - 防止在旋转具有大量数据的表时搜索d停滞
  * [secondary_indexes](Server_settings/Searchd.md#secondary_indexes) - 使搜索查询能够使用二级索引
  * [server_id](Server_settings/Searchd.md#server_id) - 用于生成唯一文档ID的服务器标识符
  * [shutdown_timeout](Server_settings/Searchd.md#shutdown_timeout) - 搜索d `--stopwait` 超时
  * [shutdown_token](Server_settings/Searchd.md#shutdown_token) - 从VIP SQL连接调用`shutdown`命令所需的SHA1哈希密码
  * [snippets_file_prefix](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) - 在`load_files`模式下生成片段时附加到本地文件名的前缀
  * [sphinxql_state](Server_settings/Searchd.md#sphinxql_state) - 当前SQL状态将序列化的文件路径
  * [sphinxql_timeout](Server_settings/Searchd.md#sphinxql_timeout) - MySQL客户端之间等待请求的最大时间
  * [ssl_ca](Server_settings/Searchd.md#ssl_ca) - SSL证书颁发机构证书文件路径
  * [ssl_cert](Server_settings/Searchd.md#ssl_cert) - 服务器SSL证书路径
  * [ssl_key](Server_settings/Searchd.md#ssl_key) - 服务器SSL证书密钥路径
  * [subtree_docs_cache](Server_settings/Searchd.md#subtree_docs_cache) - 共同子树文档缓存的最大大小
  * [subtree_hits_cache](Server_settings/Searchd.md#subtree_hits_cache) - 每个查询的共同子树击中缓存的最大大小
  * [timezone](Server_settings/Searchd.md#timezone) - 日期/时间相关函数使用的时区
  * [thread_stack](Server_settings/Searchd.md#thread_stack) - 作业的最大堆栈大小
  * [unlink_old](Server_settings/Searchd.md#unlink_old) - 成功旋转后是否卸链接旧表副本
  * [watchdog](Server_settings/Searchd.md#watchdog) - 是否启用或禁用Manticore服务器看门狗

##### Searchd 启动参数
```bash
searchd [OPTIONS]
```
* [--config, -c](Starting_the_server/Manually.md#searchd-command-line-options) - 指定配置文件的路径
* [--console](Starting_the_server/Manually.md#searchd-command-line-options) - 强制服务器以控制台模式运行
* [--coredump](Starting_the_server/Manually.md#searchd-command-line-options) - 启用崩溃时保存核心转储
* [--cpustats](Starting_the_server/Manually.md#searchd-command-line-options) - 启用 CPU 时间报告
* [--delete](Starting_the_server/Manually.md#searchd-command-line-options) - 从 Microsoft 管理控制台及其他注册服务的位置移除 Manticore 服务
* [--force-preread](Starting_the_server/Manually.md#searchd-command-line-options) - 防止服务器在表文件预读完成前服务传入连接
* [--help, -h](Starting_the_server/Manually.md#searchd-command-line-options) - 显示所有可用参数
* [--table (--index)](Starting_the_server/Manually.md#searchd-command-line-options) - 限制服务器只服务指定的表
* [--install](Starting_the_server/Manually.md#searchd-command-line-options) - 将 searchd 安装为 Microsoft 管理控制台中的服务
* [--iostats](Starting_the_server/Manually.md#searchd-command-line-options) - 启用输入/输出报告
* [--listen, -l](Starting_the_server/Manually.md#searchd-command-line-options) - 覆盖配置文件中的 [listen](Server_settings/Searchd.md#listen)
* [--logdebug, --logdebugv, --logdebugvv](Starting_the_server/Manually.md#searchd-command-line-options) - 启用在服务器日志中的额外调试输出
* [--logreplication](Starting_the_server/Manually.md#searchd-command-line-options) - 启用在服务器日志中的额外复制调试输出
* [--new-cluster](Starting_the_server/Manually.md#searchd-command-line-options) - 初始化复制集群，并将服务器设置为带有 [集群重启](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md) 保护的参考节点
* [--new-cluster-force](Starting_the_server/Manually.md#searchd-command-line-options) - 初始化复制集群，并将服务器设置为参考节点，跳过 [集群重启](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md) 保护
* [--nodetach](Starting_the_server/Manually.md#searchd-command-line-options) - 使 searchd 保持在前台运行
* [--ntservice](Starting_the_server/Manually.md#searchd-command-line-options) - 由 Microsoft 管理控制台用于在 Windows 平台上以服务方式启动 searchd
* [--pidfile](Starting_the_server/Manually.md#searchd-command-line-options) - 覆盖配置文件中的 [pid_file](Server_settings/Searchd.md#pid_file)
* [--port, p](Starting_the_server/Manually.md#searchd-command-line-options) - 指定 searchd 应监听的端口，忽略配置文件中指定的端口
* [--replay-flags](Starting_the_server/Manually.md#searchd-command-line-options) - 设置附加的二进制日志重放选项
* [--servicename](Starting_the_server/Manually.md#searchd-command-line-options) - 在安装或删除服务时，为 searchd 指定名称，该名称显示在 Microsoft 管理控制台中
* [--status](Starting_the_server/Manually.md#searchd-command-line-options) - 查询运行的搜索服务以返回其状态
* [--stop](Starting_the_server/Manually.md#searchd-command-line-options) - 停止 Manticore 服务器
* [--stopwait](Starting_the_server/Manually.md#searchd-command-line-options) - 优雅地停止 Manticore 服务器
* [--strip-path](Starting_the_server/Manually.md#searchd-command-line-options) - 从表中引用的所有文件名中移除路径名
* [-v](Starting_the_server/Manually.md#searchd-command-line-options) - 显示版本信息

##### Searchd 环境变量
* [MANTICORE_TRACK_DAEMON_SHUTDOWN](Starting_the_server/Manually.md#Environment-variables) - 在 searchd 关闭期间启用详细日志记录

## [Indextool](Miscellaneous_tools.md#indextool)
各种有助于故障排除的表维护功能。
```bash
indextool <command> [options]
```
##### Indextool 启动参数
用于转储与物理表相关的各种调试信息。
```bash
indextool <command> [options]
```
* [--config, -c](Miscellaneous_tools.md#indextool) - 指定配置文件路径
* [--quiet, -q](Miscellaneous_tools.md#indextool) - 保持indextool静默；无横幅输出等
* [--help, -h](Miscellaneous_tools.md#indextool) - 列出所有可用参数
* [-v](Miscellaneous_tools.md#indextool) - 显示版本信息
* [Indextool](Miscellaneous_tools.md#indextool) - 验证配置文件
* [--buildidf](Miscellaneous_tools.md#indextool) - 从一个或多个字典转储构建IDF文件
* [--build-infixes](Miscellaneous_tools.md#indextool) - 为现有的dict=keywords表构建中缀
* [--dumpheader](Miscellaneous_tools.md#indextool) - 快速转储提供的表头文件
* [--dumpconfig](Miscellaneous_tools.md#indextool) - 以几乎兼容的manticore.conf格式转储给定表头文件的表定义
* [--dumpheader](Miscellaneous_tools.md#indextool) - 通过表名转储表头，同时在配置文件中查找表头路径
* [--dumpdict](Miscellaneous_tools.md#indextool) - 转储表字典
* [--dumpdocids](Miscellaneous_tools.md#indextool) - 按表名转储文档ID
* [--dumphitlist](Miscellaneous_tools.md#indextool) - 转储指定表中给定关键字/ID的所有出现
* [--docextract](Miscellaneous_tools.md#indextool) - 运行表检查，遍历整个字典/文档/命中，收集属于请求文档的所有词和命中
* [--fold](Miscellaneous_tools.md#indextool) - 基于表设置测试分词
* [--htmlstrip](Miscellaneous_tools.md#indextool) - 使用指定表的HTML剥离器设置过滤STDIN
* [--mergeidf](Miscellaneous_tools.md#indextool) - 合并多个.idf文件为一个文件
* [--morph](Miscellaneous_tools.md#indextool) - 对提供的STDIN应用形态学处理，并将结果输出到stdout
* [--check](Miscellaneous_tools.md#indextool) - 检查表数据文件的一致性
* [--check-id-dups](Miscellaneous_tools.md#indextool) - 检查重复的ID
* [--check-disk-chunk](Miscellaneous_tools.md#indextool) - 检查RT表的单个磁盘块
* [--strip-path](Miscellaneous_tools.md#indextool) - 从表中引用的所有文件名中移除路径名
* [--rotate](Miscellaneous_tools.md#indextool) - 决定是否在使用`--check`时检查等待旋转的表
* [--apply-killlists](Miscellaneous_tools.md#indextool) - 应用配置文件中列出所有表的kill-list

## [Wordbreaker](Miscellaneous_tools.md#wordbreaker)
将复合词拆分成其组成部分。
```bash
wordbreaker [-dict path/to/dictionary_file] {split|test|bench}
```

##### Wordbreaker 启动参数
* [STDIN](Miscellaneous_tools.md#wordbreaker) - 接收一个字符串进行拆分
* [-dict](Miscellaneous_tools.md#wordbreaker) - 指定要使用的词典文件
* [split|test|bench](Miscellaneous_tools.md#wordbreaker) - 指定命令

## [Spelldump](Miscellaneous_tools.md#spelldump)
使用ispell或MySpell格式提取字典文件内容

```bash
spelldump [options] <dictionary> <affix> [result] [locale-name]
```
* [dictionary](Miscellaneous_tools.md#spelldump) - 主字典文件
* [affix](Miscellaneous_tools.md#spelldump) - 字典的词缀文件
* [result](Miscellaneous_tools.md#spelldump) - 指定字典数据的输出目标
* [locale-name](Miscellaneous_tools.md#spelldump) - 指定要使用的区域设置详情

## 保留关键字列表

当前在Manticore SQL语法中保留的关键字的完整字母顺序列表（因此不能用作标识符）。

```
AND, AS, BY, COLUMNARSCAN, DISTINCT, DIV, DOCIDINDEX, EXPLAIN, FACET, FALSE, FORCE, FROM, IGNORE, IN, INDEXES, INNER, IS, JOIN, KNN, LEFT, LIMIT, MOD, NOT, NO_COLUMNARSCAN, NO_DOCIDINDEX, NO_SECONDARYINDEX, NULL, OFFSET, ON, OR, ORDER, RELOAD, SECONDARYINDEX, SELECT, SYSFILTERS, TRUE
```

## 旧版本Manticore文档

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
* [5.0.2](https://manual.manticoresearch.com/manticore-5-0-2/). [安装页面](https://manticoresearch.com/install-5.0.0/)
* [6.0.0](https://manual.manticoresearch.com/manticore-6-0-0/). [安装页面](https://manticoresearch.com/install-6.0.0/)
* [6.0.2](https://manual.manticoresearch.com/manticore-6-0-2/). [安装页面](https://manticoresearch.com/install-6.0.2/)
* [6.0.4](https://manual.manticoresearch.com/manticore-6-0-4/). [安装页面](https://manticoresearch.com/install-6.0.4/)
* [6.2.0](https://manual.manticoresearch.com/manticore-6-2-0/). [安装页面](https://manticoresearch.com/install-6.2.0/)
* [6.2.12](https://manual.manticoresearch.com/manticore-6-2-12/). [安装页面](https://manticoresearch.com/install-6.2.12/)
* [6.3.0](https://manual.manticoresearch.com/manticore-6-3-0/). [安装页面](https://manticoresearch.com/install-6.3.0/)
* [6.3.2](https://manual.manticoresearch.com/manticore-6-3-2/). [安装页面](https://manticoresearch.com/install-6.3.2/)
* [6.3.4](https://manual.manticoresearch.com/manticore-6-3-4/). [安装页面](https://manticoresearch.com/install-6.3.4/)
* [6.3.6](https://manual.manticoresearch.com/manticore-6-3-6/). [安装页面](https://manticoresearch.com/install-6.3.6/)
* [6.3.8](https://manual.manticoresearch.com/manticore-6-3-8/). [安装页面](https://manticoresearch.com/install-6.3.8/)
* [7.0.0](https://manual.manticoresearch.com/manticore-7-0-0/). [安装页面](https://manticoresearch.com/install-7.0.0/)
* [7.4.6](https://manual.manticoresearch.com/manticore-7-4-6/). [安装页面](https://manticoresearch.com/install-7.4.6/)
* [9.2.14](https://manual.manticoresearch.com/manticore-9-2-14/). [安装页面](https://manticoresearch.com/install-9.2.14/)
* [9.3.2](https://manual.manticoresearch.com/manticore-9-3-2/). [安装页面](https://manticoresearch.com/install-9.3.2/)
* [10.1.0](https://manual.manticoresearch.com/manticore-10-1-0/). [安装页面](https://manticoresearch.com/install-10.1.0/)
* [13.2.3](https://manual.manticoresearch.com/manticore-13-2-3/). [安装页面](https://manticoresearch.com/install-13.2.3/)
* [13.6.7](https://manual.manticoresearch.com/manticore-13-6-7/). [安装页面](https://manticoresearch.com/install-13.6.7/)
* [13.11.0](https://manual.manticoresearch.com/manticore-13-11-0/). [安装页面](https://manticoresearch.com/install-13.11.0/)
* [13.11.1](https://manual.manticoresearch.com/manticore-13-11-1/). [安装页面](https://manticoresearch.com/install-13.11.1/)
* [13.13.0](https://manual.manticoresearch.com/manticore-13-13-0/). [安装页面](https://manticoresearch.com/install-13.13.0/)
* [14.1.0](https://manual.manticoresearch.com/manticore-14-1-0/). [安装页面](https://manticoresearch.com/install-14.1.0/)
* [15.1.0](https://manual.manticoresearch.com/manticore-15-1-0/). [安装页面](https://manticoresearch.com/install-15.1.0/)
<!-- proofread -->

