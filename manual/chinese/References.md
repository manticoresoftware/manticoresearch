# 参考资料

### SQL 命令
##### 模式管理
* [CREATE TABLE](Creating_a_table/Local_tables/Real-time_table.md#CREATE-TABLE-command:) - 创建新表
* [CREATE TABLE LIKE](Creating_a_table/Local_tables/Real-time_table.md#CREATE-TABLE-LIKE:) - 使用另一张表作为模板创建表
* [CREATE TABLE LIKE ... WITH DATA](Creating_a_table/Local_tables/Real-time_table.md#CREATE-TABLE-LIKE:) - 复制一张表
* [CREATE SOURCE](Integration/Kafka.md#Source) - 创建 Kafka 消费源
* [CREATE MATERIALIZED VIEW](Integration/Kafka.md#Materialized-view) - 从 Kafka 消息中进行数据转换
* [CREATE MV](Integration/Kafka.md#Materialized-view) - 与前一个相同
* [DESCRIBE](Listing_tables.md#DESCRIBE) - 打印表的字段列表及其类型
* [ALTER TABLE](Updating_table_schema_and_settings.md) - 更改表结构/设置
* [ALTER TABLE REBUILD SECONDARY](Updating_table_schema_and_settings.md#Rebuilding-a-secondary-index) - 更新/恢复二级索引
* [ALTER TABLE type='distributed'](Updating_table_schema_and_settings.md#Changing-a-distributed-table) - 更新/恢复二级索引
* [ALTER TABLE RENAME](Updating_table_schema_and_settings.md#Renaming-a-real-time-table)
* [ALTER MATERIALIZED VIEW {name} suspended=1](Integration/Kafka.md#Altering-materialized-views) - 暂停或恢复从 Kafka 源的消费
* [DROP TABLE IF EXISTS](Deleting_a_table.md#Deleting-a-table) - 删除一张表（如果存在）
* [SHOW TABLES](Listing_tables.md#DESCRIBE) - 显示表列表
* [SHOW SOURCES](Integration/Kafka.md#Listing) - 显示 Kafka 源列表
* [SHOW MATERIALIZED VIEWS](Integration/Kafka.md#Listing) - 显示物化视图列表
* [SHOW MVS](Integration/Kafka.md#Listing) - 上一个命令的别名
* [SHOW CREATE TABLE](Listing_tables.md#DESCRIBE) - 显示创建该表的 SQL 命令
* [SHOW TABLE INDEXES](Node_info_and_management/Table_settings_and_status/SHOW_TABLE_INDEXES.md) - 显示该表可用二级索引的信息
* [SHOW TABLE STATUS](Node_info_and_management/Table_settings_and_status/SHOW_TABLE_STATUS.md) - 显示当前表状态信息
* [SHOW TABLE SETTINGS](Node_info_and_management/Table_settings_and_status/SHOW_TABLE_SETTINGS.md) - 显示表设置
* [SHOW LOCKS](Securing_and_compacting_a_table/Freezing_and_locking_a_table.md#SHOW-LOCKS) - 显示冻结表的信息

##### 数据管理
* [INSERT](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md) - 添加新文档
* [Bulk import](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Bulk-import) - 向现有本地实时表加载大批量数据
* [REPLACE](Data_creation_and_modification/Updating_documents/REPLACE.md) - 用新文档替换现有文档
* [REPLACE .. SET](Data_creation_and_modification/Updating_documents/REPLACE.md?client=REPLACE+SET) - 替换表中的一个或多个字段
* [UPDATE](Data_creation_and_modification/Updating_documents/UPDATE.md) - 在文档中执行原地更新
* [DELETE](Data_creation_and_modification/Deleting_documents.md) - 删除文档
* [TRUNCATE TABLE](Emptying_a_table.md) - 删除表中的所有文档

##### 备份
* [BACKUP](Securing_and_compacting_a_table/Backup_and_restore.md#BACKUP-SQL-command-reference) - 备份你的表

##### SELECT
* [SELECT](Searching/Full_text_matching/Basic_usage.md#SQL) - 搜索
  * [WHERE](Searching/Filters.md#Filters) - 过滤
  * [GROUP BY](Searching/Grouping.md) - 分组搜索结果
  * [GROUP BY ORDER](Searching/Grouping.md) - 对分组排序
  * [GROUP BY HAVING](Searching/Grouping.md) - 过滤分组
  * [OPTION](Searching/Options.md#OPTION) - 查询选项
  * [FACET](Searching/Faceted_search.md) - 分面搜索
  * [SUB-SELECTS](Searching/Sub-selects.md) - 关于使用 SELECT 子查询
  * [JOIN](Searching/Joining.md) - 在 SELECT 中连接表
* [EXPLAIN QUERY](Searching/Full_text_matching/Profiling.md#Profiling-without-running-a-query) - 不执行查询本身而显示查询执行计划
* [SHOW META](Node_info_and_management/SHOW_META.md) - 显示已执行查询的扩展信息
* [SHOW PROFILE](Node_info_and_management/Profiling/Query_profile.md) - 显示已执行查询的分析信息
* [SHOW PLAN](Searching/Full_text_matching/Profiling.md#Profiling-the-query-tree-in-SQL) - 显示查询执行后的执行计划
* [SHOW WARNINGS](Node_info_and_management/SHOW_WARNINGS.md) - 显示最新查询的警告

##### 刷新其他事项
* [FLUSH ATTRIBUTES](Securing_and_compacting_a_table/Flushing_attributes.md) - 强制将已更新的属性刷新到磁盘
* [FLUSH HOSTNAMES](Securing_and_compacting_a_table/Flushing_hostnames.md) - 重新刷新与代理主机名关联的 IP
* [FLUSH LOGS](Logging/Rotating_query_and_server_logs.md) - 触发重新打开 searchd 日志和查询日志文件（类似于 USR1）

##### 身份验证与授权
* [CREATE USER](Security/Authentication_and_authorization.md#Users-and-tokens) - 创建身份验证用户
* [DROP USER](Security/Authentication_and_authorization.md#Users-and-tokens) - 删除身份验证用户
* [SET PASSWORD](Security/Authentication_and_authorization.md#Users-and-tokens) - 更改当前或指定用户的密码
* [TOKEN](Security/Authentication_and_authorization.md#Users-and-tokens) - 创建或轮换 bearer token
* [GRANT](Security/Authentication_and_authorization.md#Permissions) - 授予身份验证权限
* [REVOKE](Security/Authentication_and_authorization.md#Permissions) - 撤销身份验证权限
* [SHOW USERS](Security/Authentication_and_authorization.md#Inspecting-authentication-data) - 列出身份验证用户
* [SHOW PERMISSIONS](Security/Authentication_and_authorization.md#Inspecting-authentication-data) - 列出身份验证权限
* [SHOW USAGE](Security/Authentication_and_authorization.md#Inspecting-authentication-data) - 显示身份验证使用计数
* [SHOW TOKEN](Security/Authentication_and_authorization.md#Users-and-tokens) - 显示已存储的 token 哈希值
* [RELOAD AUTH](Security/Authentication_and_authorization.md#Inspecting-authentication-data) - 重新加载身份验证数据

##### 表优化
* [FLUSH RAMCHUNK](Securing_and_compacting_a_table/Flushing_RAM_chunk_to_a_new_disk_chunk.md#FLUSH-RAMCHUNK) - 强制创建新的磁盘块
* [FLUSH TABLE](Securing_and_compacting_a_table/Flushing_RAM_chunk_to_disk.md#FLUSH-TABLE) - 将实时表的 RAM 块刷新到磁盘
* [OPTIMIZE TABLE](Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE) - 压缩 RT 表或分布式表/分片表的物理 RT 目标

##### 导入到实时表
* [ATTACH TABLE](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Attaching_one_table_to_another.md) - 将数据从普通表移动到实时表
* [IMPORT TABLE](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Importing_table.md) - 将之前创建的 RT 或 PQ 表导入到以 RT 模式运行的服务器中

##### 复制
* [JOIN CLUSTER](Creating_a_cluster/Setting_up_replication/Joining_a_replication_cluster.md) - 加入复制集群
* [ALTER CLUSTER](Creating_a_cluster/Setting_up_replication/Managing_replication_nodes.md) - 向复制集群添加/删除表
* [EXIT CLUSTER](Creating_a_cluster/Setting_up_replication/Managing_replication_nodes.md) - 将当前节点从复制集群中分离
* [SET CLUSTER](Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Cluster-parameters) - 更改复制集群设置
* [DELETE CLUSTER](Creating_a_cluster/Setting_up_replication/Deleting_a_replication_cluster.md) - 删除复制集群

##### 普通表旋转
* [RELOAD TABLE](Data_creation_and_modification/Adding_data_from_external_storages/Rotating_a_table.md#RELOAD-TABLE) - 旋转普通表
* [RELOAD TABLES](Data_creation_and_modification/Adding_data_from_external_storages/Rotating_a_table.md#RELOAD-TABLE) - 旋转所有普通表

##### 事务
* [BEGIN](Data_creation_and_modification/Transactions.md#BEGIN,-COMMIT,-and-ROLLBACK) - 开始事务
* [COMMIT](Data_creation_and_modification/Transactions.md#BEGIN,-COMMIT,-and-ROLLBACK) - 结束事务
* [ROLLBACK](Data_creation_and_modification/Transactions.md#BEGIN,-COMMIT,-and-ROLLBACK) - 回滚事务

##### CALL
* [CALL SUGGEST, CALL QSUGGEST](Searching/Spell_correction.md#CALL-QSUGGEST,-CALL-SUGGEST) - 提示拼写纠正后的词语
* [CALL SNIPPETS](Searching/Highlighting.md) - 根据提供的数据和查询构建带高亮的结果片段
* [CALL PQ](Searching/Percolate_query.md) - 执行 percolate 查询
* [CALL KEYWORDS](Searching/Autocomplete.md#CALL-KEYWORDS) - 用于检查关键词如何被分词，也可检索所提供关键词的分词形式
* [CALL AUTOCOMPLETE](Searching/Autocomplete.md#CALL-AUTOCOMPLETE) - 自动补全你的搜索查询

##### 插件
* [CREATE FUNCTION](Extensions/UDFs_and_Plugins/UDF/Creating_a_function.md) - 安装用户自定义函数（UDF）
* [DROP FUNCTION](Extensions/UDFs_and_Plugins/UDF/Deleting_a_function.md) - 删除用户自定义函数（UDF）
* [CREATE PLUGIN](Extensions/UDFs_and_Plugins/Plugins/Creating_a_plugin.md) - 安装插件
* [CREATE BUDDY PLUGIN](Extensions/UDFs_and_Plugins/Plugins/Creating_a_plugin.md#CREATE-BUDDY-PLUGIN) - 安装 Buddy 插件
* [DROP PLUGIN](Extensions/UDFs_and_Plugins/Plugins/Deleting_a_plugin.md#DELETE-PLUGIN) - 删除插件
* [DROP BUDDY PLUGIN](Extensions/UDFs_and_Plugins/Plugins/Deleting_a_plugin.md#DELETE-BUDDY-PLUGIN) - 删除 Buddy 插件
* [RELOAD PLUGINS](Extensions/UDFs_and_Plugins/Plugins/Reloading_plugins.md) - 从给定库中重新加载所有插件
* [ENABLE BUDDY PLUGIN](Extensions/UDFs_and_Plugins/Plugins/Enabling_and_disabling_buddy_plugins.md#ENABLE-BUDDY-PLUGIN) - 重新启用之前被禁用的 Buddy 插件
* [DISABLE BUDDY PLUGIN](Extensions/UDFs_and_Plugins/Plugins/Enabling_and_disabling_buddy_plugins.md#DISABLE-BUDDY-PLUGIN) - 停用一个正在运行的 Buddy 插件

##### 节点管理
* [PURGE](Node_info_and_management/PURGE.md) - 移除与某个表相关的选定数据
* [PURGE BULK_IMPORT](Node_info_and_management/PURGE.md#PURGE-BULK_IMPORT) - 删除 bulk import 遗留的暂存文件

##### 服务器状态
* [SHOW STATUS](Node_info_and_management/Node_status.md#SHOW-STATUS) - 显示一组有用的性能计数器
* [SHOW THREADS](Node_info_and_management/SHOW_THREADS.md) - 列出当前所有活跃的客户端线程
* [SHOW VARIABLES](Node_info_and_management/SHOW_VARIABLES.md) - 列出服务器级变量及其值
* [SHOW VERSION](Node_info_and_management/SHOW_VERSION.md#SHOW-VERSION) - 提供实例各组件的详细版本信息。

### HTTP 端点
* [/sql](Connecting_to_the_server/HTTP.md#SQL-over-HTTP) - 通过 HTTP JSON 执行 SQL 语句
* [/cli](Connecting_to_the_server/HTTP.md#/cli) - 提供 HTTP 命令行接口
* [/insert](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md) - 向实时表插入文档
* [/pq/tbl_name/doc](Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md#Adding-rules-to-a-percolate-table) - 向 percolate 表添加一条 PQ 规则
* [/update](Data_creation_and_modification/Updating_documents/UPDATE.md#Updates-via-HTTP-JSON) - 更新实时表中的文档
* [/replace](Data_creation_and_modification/Updating_documents/REPLACE.md) - 替换实时表中的现有文档，如果不存在则插入
* [/pq/tbl_name/doc/N?refresh=1](Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md#Adding-rules-to-a-percolate-table) - 替换 percolate 表中的一条 PQ 规则
* [/delete](Data_creation_and_modification/Deleting_documents.md) - 从表中移除文档
* [/bulk](Data_creation_and_modification/Updating_documents/UPDATE.md#Bulk-updates) - 在一次调用中执行多个插入、更新或删除操作。更多批量插入信息请见[这里](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md)。
* [/search](Searching/Full_text_matching/Basic_usage.md#HTTP-JSON) - 执行搜索
* [/search -> knn](Searching/KNN.md) - 执行 KNN 向量搜索
* [/pq/tbl_name/search](Searching/Percolate_query.md) - 在 percolate 表中执行反向搜索
* [/tbl_name/_mapping](Creating_a_table/Local_tables/Real-time_table.md#_mapping-API:) - 以 Elasticsearch 风格创建表结构

### 常见内容
* [table, field, and attribute name syntax](Creating_a_table/Data_types.md#Table-and-field-name-syntax)
* [data types](Creating_a_table/Data_types.md)
* [engine](Creating_a_table/Data_types.md)
* [plain mode](Read_this_first.md#Real-time-mode-vs-plain-mode)
* [real-time mode](Read_this_first.md#Real-time-mode-vs-plain-mode)

##### 常见表设置
* [access_plain_attrs](Server_settings/Searchd.md#access_plain_attrs)
* [access_blob_attrs](Server_settings/Searchd.md#access_blob_attrs)
* [access_doclists](Server_settings/Searchd.md#access_doclists)
* [access_hitlists](Server_settings/Searchd.md#access_hitlists)
* [access_dict](Server_settings/Searchd.md#access_dict)
* [attr_update_reserve](Data_creation_and_modification/Updating_documents/UPDATE.md#attr_update_reserve)
* [bigram_delimiter](Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#bigram_delimiter)
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
* [exceptions_list](Creating_a_table/NLP_and_tokenization/Exceptions.md#exceptions_list)
* [expand_keywords](Searching/Options.md#expand_keywords)
* [global_idf](Searching/Options.md#global_idf)
* [hitless_words](Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#hitless_words)
* [hitless_words_list](Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#hitless_words_list)
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
* [stopwords_list](Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopwords_list)
* [stopword_step](Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopword_step)
* [stopwords_unstemmed](Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopwords_unstemmed)
* [type](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [wordforms](Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms)
* [wordforms_list](Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms_list)

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

##### RT 表设置
* [rt_attr_bigint](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [rt_attr_bool](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [rt_attr_float](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [rt_attr_float_vector](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [rt_attr_float_vector_array](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
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
* [@*](Searching/Full_text_matching/Operators.md#Field-search-operator) - 全字段搜索运算符
* ["word1 word2 ... "](Searching/Full_text_matching/Operators.md#Phrase-search-operator) - 短语搜索运算符
* ["word1 word2 ... "~N](Searching/Full_text_matching/Operators.md#Proximity-search-operator) - 邻近搜索运算符
* ["word1 word2 ... "/N](Searching/Full_text_matching/Operators.md#Quorum-matching-operator) - quorum 匹配运算符
* [word1 << word2 << word3](Searching/Full_text_matching/Operators.md#Strict-order-operator) - 严格顺序运算符
* [=word1](Searching/Full_text_matching/Operators.md#Exact-form-modifier) - 精确形式修饰符
* [^word1](Searching/Full_text_matching/Operators.md#Field-start-and-field-end-modifier) - 字段开头修饰符
* [word2$](Searching/Full_text_matching/Operators.md#Field-start-and-field-end-modifier) - 字段结尾修饰符
* [word^N](Searching/Full_text_matching/Operators.md#IDF-boost-modifier) - 关键词 IDF 提升修饰符
* [word1 NEAR/N word2](Searching/Full_text_matching/Operators.md#NEAR-operator) - NEAR，广义邻近运算符
* [word1 NOTNEAR/N word2](Searching/Full_text_matching/Operators.md#NOTNEAR-operator) - NOTNEAR，否定断言运算符
* [word1 PARAGRAPH word2 PARAGRAPH "word3 word4"](Searching/Full_text_matching/Operators.md#SENTENCE-and-PARAGRAPH-operators) - PARAGRAPH 运算符
* [word1 SENTENCE word2 SENTENCE "word3 word4"](Searching/Full_text_matching/Operators.md#SENTENCE-and-PARAGRAPH-operators) - SENTENCE 运算符
* [ZONE:(h3,h4)](Searching/Full_text_matching/Operators.md#ZONE-limit-operator) - ZONE 限制运算符
* [ZONESPAN:(h2)](Searching/Full_text_matching/Operators.md#ZONESPAN-limit-operator) - ZONESPAN 限制运算符
* [@@relaxed](Searching/Full_text_matching/Operators.md#Field-search-operator) - 抑制缺失字段错误
* [t?st](Searching/Full_text_matching/Operators.md#Wildcard-operators) - 通配符运算符
* [REGEX(/pattern/)](Searching/Full_text_matching/Operators.md#REGEX-operator) - REGEX 运算符

## 函数
##### 数学
* [ABS()](Functions/Mathematical_functions.md#ABS%28%29) - 返回绝对值
* [ATAN2()](Functions/Mathematical_functions.md#ATAN2%28%29) - 返回两个参数的反正切函数
* [BITDOT()](Functions/Mathematical_functions.md#BITDOT%28%29) - 返回掩码各位与其权重相乘后的乘积之和
* [CEIL()](Functions/Mathematical_functions.md#CEIL%28%29) - 返回大于或等于该参数的最小整数值
* [COS()](Functions/Mathematical_functions.md#COS%28%29) - 返回该参数的余弦值
* [CRC32()](Functions/Mathematical_functions.md#CRC32%28%29) - 返回该参数的 CRC32 值
* [EXP()](Functions/Mathematical_functions.md#EXP%28%29) - 返回该参数的指数
* [FIBONACCI()](Functions/Mathematical_functions.md#FIBONACCI%28%29) - 返回第 N 个斐波那契数，其中 N 是整数参数
* [FLOOR()](Functions/Mathematical_functions.md#FLOOR%28%29) - 返回小于或等于该参数的最大整数值
* [GREATEST()](Functions/Mathematical_functions.md#GREATEST%28%29) - 以 JSON/MVA 数组为参数，并返回该数组中的最大值
* [IDIV()](Functions/Mathematical_functions.md#IDIV%28%29) - 返回第一个参数除以第二个参数后的整数除法结果
* [LEAST()](Functions/Mathematical_functions.md#LEAST%28%29) - 以 JSON/MVA 数组为参数，并返回该数组中的最小值
* [LN()](Functions/Mathematical_functions.md#LN%28%29) - 返回该参数的自然对数
* [LOG10()](Functions/Mathematical_functions.md#LOG10%28%29) - 返回该参数的常用对数
* [LOG2()](Functions/Mathematical_functions.md#LOG2%28%29) - 返回该参数的二进制对数
* [MAX()](Functions/Mathematical_functions.md#MAX%28%29) - 返回两个参数中较大的一个
* [MIN()](Functions/Mathematical_functions.md#MIN%28%29) - 返回两个参数中较小的一个
* [POW()](Functions/Mathematical_functions.md#POW%28%29) - 返回第一个参数的第二个参数次幂
* [RAND()](Functions/Mathematical_functions.md#RAND%28%29) - 返回 0 到 1 之间的随机浮点数
* [SIN()](Functions/Mathematical_functions.md#SIN%28%29) - 返回该参数的正弦值
* [SQRT()](Functions/Mathematical_functions.md#SQRT%28%29) - 返回该参数的平方根


##### 搜索与排序
* [BM25F()](Functions/Searching_and_ranking_functions.md#BM25F%28%29) - 返回精确的 BM25F 公式值
* [EXIST()](Functions/Searching_and_ranking_functions.md#EXIST%28%29) - 用默认值替换不存在的列
* [GROUP_CONCAT()](Searching/Grouping.md#GROUP_CONCAT%28field%29) - 生成组内所有文档属性值的逗号分隔列表
* [HIGHLIGHT()](Searching/Highlighting.md) - 高亮搜索结果
* [MIN_TOP_SORTVAL()](Functions/Searching_and_ranking_functions.md#MIN_TOP_SORTVAL%28%29) - 返回当前 top-N 匹配中找到的最差元素的排序键值
* [MIN_TOP_WEIGHT()](Functions/Searching_and_ranking_functions.md#MIN_TOP_WEIGHT%28%29) - 返回当前 top-N 匹配中找到的最差元素的权重
* [PACKEDFACTORS()](Functions/Searching_and_ranking_functions.md#PACKEDFACTORS%28%29) - 输出加权因子
* [REMOVE_REPEATS()](Functions/Searching_and_ranking_functions.md#REMOVE_REPEATS%28%29) - 移除具有相同 `column` 值的重复调整行
* [WEIGHT()](Functions/Searching_and_ranking_functions.md#WEIGHT%28%29) - 返回全文匹配分数
* [ZONESPANLIST()](Functions/Searching_and_ranking_functions.md#ZONESPANLIST%28%29) - 返回匹配到的 zone span 对
* [QUERY()](Functions/Searching_and_ranking_functions.md#QUERY%28%29) - 返回当前全文查询

##### 类型转换
* [BIGINT()](Functions/Type_casting_functions.md#BIGINT%28%29) - 强制将整数参数提升为 64 位类型
* [DOUBLE()](Functions/Type_casting_functions.md#DOUBLE%28%29) - 强制将给定参数提升为浮点类型
* [INTEGER()](Functions/Type_casting_functions.md#INTEGER%28%29) - 强制将给定参数提升为 64 位有符号类型
* [TO_STRING()](Functions/Type_casting_functions.md#TO_STRING%28%29) - 强制将参数提升为字符串类型
* [UINT()](Functions/Type_casting_functions.md#UINT%28%29) - 将给定参数转换为 32 位无符号整数类型
* [UINT64()](Functions/Type_casting_functions.md#UINT64%28%29) - 将给定参数转换为 64 位无符号整数类型
* [SINT()](Functions/Type_casting_functions.md#SINT%28%29) - 将 32 位无符号整数解释为有符号 64 位整数

##### 数组与条件
* [ALL()](Functions/Arrays_and_conditions_functions.md#ALL%28%29) - 如果数组中所有元素都满足条件，则返回 1
* [ANY()](Functions/Arrays_and_conditions_functions.md#ANY%28%29) - 如果数组中任一元素满足条件，则返回 1
* [CONTAINS()](Functions/Arrays_and_conditions_functions.md#CONTAINS%28%29) - 检查 (x,y) 点是否在给定多边形内
* [IF()](Functions/Arrays_and_conditions_functions.md#IF%28%29) - 检查第一个参数是否等于 0.0；如果不为零则返回第二个参数，否则返回第三个参数
* [IN()](Functions/Arrays_and_conditions_functions.md#IN%28%29) - 如果第一个参数等于其他任一参数则返回 1，否则返回 0
* [INDEXOF()](Functions/Arrays_and_conditions_functions.md#INDEXOF%28%29) - 遍历数组中的所有元素，并返回第一个匹配元素的索引
* [INTERVAL()](Functions/Arrays_and_conditions_functions.md#INTERVAL%28%29) - 返回小于第一个参数的参数索引
* [LENGTH()](Functions/Arrays_and_conditions_functions.md#LENGTH%28%29) - 返回 MVA 中的元素数量
* [REMAP()](Functions/Arrays_and_conditions_functions.md#REMAP%28%29) - 允许根据条件值对表达式值做某些例外映射

##### 日期和时间
* [NOW()](Functions/Date_and_time_functions.md#NOW%28%29) - 以 INTEGER 形式返回当前时间戳
* [CURTIME()](Functions/Date_and_time_functions.md#CURTIME%28%29) - 返回本地时区的当前时间
* [CURDATE()](Functions/Date_and_time_functions.md#CURDATE%28%29) - 返回本地时区的当前日期
* [UTC_TIME()](Functions/Date_and_time_functions.md#UTC_TIME%28%29) - 返回 UTC 时区的当前时间
* [UTC_TIMESTAMP()](Functions/Date_and_time_functions.md#UTC_TIMESTAMP%28%29) - 返回 UTC 时区的当前日期/时间
* [SECOND()](Functions/Date_and_time_functions.md#SECOND%28%29) - 返回时间戳参数中的整数秒
* [MINUTE()](Functions/Date_and_time_functions.md#MINUTE%28%29) - 返回时间戳参数中的整数分钟
* [HOUR()](Functions/Date_and_time_functions.md#HOUR%28%29) - 返回时间戳参数中的整数小时
* [DAY()](Functions/Date_and_time_functions.md#DAY%28%29) - 返回时间戳参数中的整数日期
* [MONTH()](Functions/Date_and_time_functions.md#MONTH%28%29) - 返回时间戳参数中的整数月份
* [QUARTER()](Functions/Date_and_time_functions.md#QUARTER%28%29) - 返回时间戳参数中的一年中的整数季度
* [YEAR()](Functions/Date_and_time_functions.md#YEAR%28%29) - 返回时间戳参数中的整数年份
* [DAYNAME()](Functions/Date_and_time_functions.md#DAYNAME%28%29) - 返回给定时间戳参数对应的星期名称
* [MONTHNAME()](Functions/Date_and_time_functions.md#MONTHNAME%28%29) - 返回给定时间戳参数对应的月份名称
* [DAYOFWEEK()](Functions/Date_and_time_functions.md#DAYOFWEEK%28%29) - 返回给定时间戳参数对应的整数星期索引
* [DAYOFYEAR()](Functions/Date_and_time_functions.md#DAYOFYEAR%28%29) - 返回给定时间戳参数对应的年内第几天
* [YEARWEEK()](Functions/Date_and_time_functions.md#YEARWEEK%28%29) - 返回给定时间戳参数对应的整数年份以及当前周第一天的日代码
* [YEARMONTH()](Functions/Date_and_time_functions.md#YEARMONTH%28%29) - 返回时间戳参数中的整数年和月代码
* [YEARMONTHDAY()](Functions/Date_and_time_functions.md#YEARMONTHDAY%28%29) - 返回时间戳参数中的整数年、月和日代码
* [TIMEDIFF()](Functions/Date_and_time_functions.md#TIMEDIFF%28%29) - 返回两个时间戳之间的差值
* [DATEDIFF()](Functions/Date_and_time_functions.md#DATEDIFF%28%29) - 返回两个给定时间戳之间的天数
* [DATE()](Functions/Date_and_time_functions.md#DATE%28%29) - 从时间戳参数中格式化日期部分
* [TIME()](Functions/Date_and_time_functions.md#TIME%28%29) - 从时间戳参数中格式化时间部分
* [DATE_FORMAT()](Functions/Date_and_time_functions.md#DATE_FORMAT%28%29) - 根据提供的日期和格式参数返回格式化字符串


##### 地理空间
* [GEODIST()](Functions/Geo_spatial_functions.md#GEODIST%28%29) - 计算两个给定点之间的大地距离
* [GEOPOLY2D()](Functions/Geo_spatial_functions.md#GEOPOLY2D%28%29) - 创建一个将地球曲率考虑在内的多边形
* [POLY2D()](Functions/Geo_spatial_functions.md#POLY2D%28%29) - 在平面空间中创建一个简单多边形

##### 字符串
* [CONCAT()](Functions/String_functions.md#CONCAT%28%29) - 连接两个或多个字符串
* [REGEX()](Functions/String_functions.md#REGEX%28%29) - 如果属性字符串匹配正则表达式则返回 1，否则返回 0
* [SNIPPET()](Functions/String_functions.md#SNIPPET%28%29) - 高亮搜索结果
* [SUBSTRING_INDEX()](Functions/String_functions.md#SUBSTRING_INDEX%28%29) - 返回字符串中指定分隔符出现次数之前的子串

##### 其他
* [CONNECTION_ID()](Functions/Other_functions.md#CONNECTION_ID%28%29) - 返回当前连接 ID
* [KNN_DIST()](Functions/Other_functions.md#KNN_DIST%28%29) - 返回 KNN 向量搜索距离
* [LAST_INSERT_ID()](Functions/Other_functions.md#LAST_INSERT_ID%28%29) - 返回当前会话中最后一条语句插入或替换的文档 ID
* [UUID_SHORT()](Functions/Other_functions.md#UUID_SHORT%28%29) - 返回一个“短”通用标识符，算法与自动 ID 生成相同。

## 配置文件中的常见设置
需要放在配置文件的 `common {}` 段中：
* [lemmatizer_base](Server_settings/Common.md#lemmatizer_base) - 词元还原词典基础路径
* [progressive_merge](Server_settings/Common.md#progressive_merge) - 定义实时表中磁盘块的合并顺序
* [json_autoconv_keynames](Server_settings/Common.md#json_autoconv_keynames) - 是否以及如何自动转换 JSON 属性中的键名
* [json_autoconv_numbers](Server_settings/Common.md#json_autoconv_numbers) - 自动检测并将表示数字的可能 JSON 字符串转换为数值属性
* [on_json_attr_error](Server_settings/Common.md#on_json_attr_error) - 发现 JSON 格式错误时如何处理
* [plugin_dir](Server_settings/Common.md#plugin_dir) - 动态库和 UDF 的位置

## [Indexer](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments)
`indexer` 是用于创建[普通表](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments)的工具

##### Indexer 设置在配置文件中
需要放在配置文件的 `indexer {}` 段中：
* [lemmatizer_cache](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 词元还原缓存大小
* [max_file_field_buffer](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 文件字段自适应缓冲区最大大小
* [max_iops](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 最大索引 I/O 操作每秒数
* [max_iosize](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 允许的最大 I/O 操作大小
* [max_xmlpipe2_field](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - XMLpipe2 源类型允许的最大字段大小
* [mem_limit](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 索引过程内存使用上限
* [on_file_field_error](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 如何处理文件字段中的 I/O 错误
* [write_buffer](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 写缓冲区大小
* [ignore_non_plain](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 忽略关于非普通表的警告

##### Indexer 启动参数
```bash
indexer [OPTIONS] [indexname1 [indexname2 [...]]]
```
* [--all](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 重新构建配置中的所有表
* [--buildstops](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 将表源按索引数据的方式进行分析，生成已索引词项列表
* [--buildfreqs](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 为 --buildstops 添加词频统计
* [--config, -c](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 指定配置文件路径
* [--dump-rows](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 将从 SQL 源检索到的行转储到指定文件中
* [--help](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 显示所有可用参数
* [--keep-attrs](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 重新索引时允许复用现有属性
* [--keep-attrs-names](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 指定要从现有表中复用哪些属性
* [--merge-dst-range](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 在合并期间应用给定的过滤范围
* [--merge-killlists](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 更改合并表时的 kill list 处理方式
* [--merge](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 将两个普通表合并为一个
* [--nohup](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 启用后阻止 indexer 发送 SIGHUP
* [--noprogress](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 隐藏进度详情
* [--remove_dups](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 逻辑上移除重复的数字文档 ID，保留第一行
* [--print-queries](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 输出 indexer 向数据库发送的 SQL 查询
* [--print-rt](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 将从 SQL 源获取的数据显示为插入到实时表的 INSERT
* [--quiet](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 抑制所有输出
* [--rotate](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 在构建完所有表后触发表轮转
* [--sighup-each](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 在每个表构建完成后触发其轮转
* [-v](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - 显示 indexer 版本

## Manticore v2 / Sphinx v2 的表转换器
`index_converter` 是一个用于将使用 Sphinx/Manticore Search 2.x 创建的表转换为 Manticore Search 3.x 表格式的工具。
```bash
index_converter {--config /path/to/config|--path}
```
##### 表转换器启动参数
* [--config, -c](Installation/Migration_from_Sphinx.md#index_converter) - 表配置文件路径
* [--index](Installation/Migration_from_Sphinx.md#index_converter) - 指定要转换的表
* [--path](Installation/Migration_from_Sphinx.md#index_converter) - 设置包含表的路径，而不是配置文件
* [--strip-path](Installation/Migration_from_Sphinx.md#index_converter) - 删除表所引用文件名中的路径
* [--large-docid](Installation/Migration_from_Sphinx.md#index_converter) - 允许转换 ID 大于 2^63 的文档
* [--output-dir](Installation/Migration_from_Sphinx.md#index_converter) - 将新文件写入指定文件夹
* [--all](Installation/Migration_from_Sphinx.md#index_converter) - 转换配置文件/路径中的所有表
* [--killlist-target](Installation/Migration_from_Sphinx.md#index_converter) - 设置应用 kill-list 的目标表

## [Searchd](Starting_the_server/Manually.md)
`searchd` 是 Manticore 服务器。

##### Searchd 设置在配置文件中
需要放在配置文件的 `searchd {}` 段中：
  * [access_blob_attrs](Server_settings/Searchd.md#access_blob_attrs) - 定义如何访问表的 blob 属性文件
  * [access_doclists](Server_settings/Searchd.md#access_doclists) - 定义如何访问表的 doclists 文件
  * [access_hitlists](Server_settings/Searchd.md#access_hitlists) - 定义如何访问表的 hitlists 文件
  * [access_plain_attrs](Server_settings/Searchd.md#access_plain_attrs) - 定义搜索服务器如何访问表的普通属性
  * [access_dict](Server_settings/Searchd.md#access_dict) - 定义如何访问表的字典文件
  * [agent_connect_timeout](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_connect_timeout) - 远程代理连接超时
  * [agent_query_timeout](Searching/Options.md#agent_query_timeout) - 远程代理查询超时
  * [agent_retry_count](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_connect_timeout) - 指定 Manticore 尝试连接和查询远程代理的次数
  * [agent_retry_delay](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) - 发生失败时重试查询远程代理前的延迟
  * [attr_flush_period](Data_creation_and_modification/Updating_documents/UPDATE.md#attr_flush_period) - 设置将更新属性刷新到磁盘之间的时间间隔
  * [auth](Server_settings/Searchd.md#auth) - 启用身份验证和授权
  * [auth_log_level](Server_settings/Searchd.md#auth_log_level) - 控制身份验证日志详细程度
  * [auth_password_policy](Server_settings/Searchd.md#auth_password_policy) - 为身份验证用户设置密码策略
  * [auth_password_min_length](Server_settings/Searchd.md#auth_password_min_length) - 为身份验证用户设置最小密码长度
  * [binlog_flush](Server_settings/Searchd.md#binlog_flush) - 二进制日志事务刷新/同步模式
  * [binlog_max_log_size](Server_settings/Searchd.md#binlog_max_log_size) - 二进制日志文件最大大小
  * [binlog_common](Logging/Binary_logging.md#Binary-logging-strategies) - 所有表共用的二进制日志文件
  * [binlog_filename_digits](Logging/Binary_logging.md#Log-files) - binlog 文件名中的位数
  * [binlog_flush](Logging/Binary_logging.md#Binary-flushing-strategies) - binlog 刷新策略
  * [binlog_path](Server_settings/Searchd.md#binlog_path) - 二进制日志文件路径
  * [client_timeout](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) - 使用持久连接时两次请求之间等待的最长时间
  * [collation_libc_locale](Server_settings/Searchd.md#collation_libc_locale) - 服务器 libc 区域设置
  * [collation_server](Server_settings/Searchd.md#collation_server) - 默认服务器排序规则
  * [data_dir](Server_settings/Searchd.md#data_dir) - Manticore 存放所有内容的数据目录路径（[RT 模式](Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29)）
  * [diskchunk_flush_write_timeout](Server_settings/Searchd.md#diskchunk_flush_write_timeout) - 若没有写入，则自动将 RAM 块刷新到磁盘的超时时间
  * [diskchunk_flush_search_timeout](Server_settings/Searchd.md#diskchunk_flush_search_timeout) - 若表中没有搜索，则阻止自动刷新 RAM 块的超时时间
  * [docstore_cache_size](Server_settings/Searchd.md#docstore_cache_size) - 文档存储中保留在内存里的文档块最大大小
  * [embeddings_threads](Server_settings/Searchd.md#embeddings_threads) - 在为自动嵌入插入、`ALTER TABLE` KNN 重建以及 KNN 文本查询嵌入生成向量时，embeddings 库最多可使用的线程数
  * [expansion_limit](Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#expansion_limit) - 单个通配符可展开的关键词最大数量
  * [grouping_in_utc](Server_settings/Searchd.md#grouping_in_utc) - 启用在分组时间字段时使用 UTC 时区
  * [ha_period_karma](Server_settings/Searchd.md#ha_period_karma) - 代理镜像统计窗口大小
  * [ha_ping_interval](Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_ping_interval) - 代理镜像 ping 之间的间隔
  * [hostname_lookup](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) - 主机名刷新策略
  * [jobs_queue_size](Server_settings/Searchd.md#jobs_queue_size) - 定义队列中同时允许的“job”最大数量
  * [join_batch_size](Searching/Joining.md#Join-batching) - 定义表连接的批大小，以平衡性能和内存使用
  * [join_cache_size](Searching/Joining.md#Join-caching) - 定义用于复用 JOIN 查询结果的缓存大小
  * [kibana_version_string](Server_settings/Searchd.md#kibana_version_string) – 发送给 Kibana 请求的服务器版本字符串
  * [knn_parallel_build](Server_settings/Searchd.md#knn_parallel_build) - 在 RT 块保存、`OPTIMIZE` / 自动优化块合并以及 `ALTER TABLE` KNN 重建期间，用于构建 HNSW 图的工作线程数
  * [listen](Server_settings/Searchd.md#listen) - 指定 searchd 监听的 IP 地址和端口或 Unix 域套接字路径
  * [listen_backlog](Server_settings/Searchd.md#listen_backlog) - TCP 监听队列长度
  * [listen_tfo](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) - 为所有监听器启用 TCP_FASTOPEN 标志
  * [log](Server_settings/Searchd.md#log) - Manticore 服务器日志文件路径
  * [max_batch_queries](Server_settings/Searchd.md#max_batch_queries) - 限制每个批次的查询数量
  * [max_connections](Server_settings/Searchd.md#max_connections) - 活跃连接最大数量
  * [merge_chunks_per_job](Server_settings/Searchd.md#merge_chunks_per_job) - 每个 OPTIMIZE 作业合并多少个 RT 磁盘块
  * [max_filters](Server_settings/Searchd.md#max_filters) - 每个查询允许的最大过滤器数量
  * [max_filter_values](Server_settings/Searchd.md#max_filter_values) - 每个过滤器允许的最大值数量
  * [max_open_files](Server_settings/Searchd.md#max_open_files) - 服务器允许打开的最大文件数
  * [max_packet_size](Server_settings/Searchd.md#max_packet_size) - 允许的最大网络包大小
  * [mysql_version_string](Server_settings/Searchd.md#mysql_version_string) - 通过 MySQL 协议返回的服务器版本字符串
  * [net_throttle_accept](Server_settings/Searchd.md#net_throttle_accept) - 定义网络循环每次迭代接受多少客户端
  * [net_throttle_action](Server_settings/Searchd.md#net_throttle_action)  - 定义网络循环每次迭代处理多少请求
  * [net_wait_tm](Server_settings/Searchd.md#net_wait_tm) - 控制网络线程的忙循环间隔
  * [net_workers](Server_settings/Searchd.md#net_workers) - 网络线程数
  * [network_timeout](Server_settings/Searchd.md#network_timeout) - 客户端请求的网络超时
  * [node_address](Server_settings/Searchd.md#node_address) - 指定节点的网络地址
  * [persistent_connections_limit](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) - 允许与远程持久代理建立的同时持久连接最大数量
  * [parallel_chunk_merges](Server_settings/Searchd.md#parallel_chunk_merges) - 在 OPTIMIZE 期间可并行运行的 RT 磁盘块合并数量
  * [pid_file](Server_settings/Searchd.md#pid_file) - Manticore 服务器 pid 文件路径
  * [preopen_tables](Server_settings/Searchd.md#preopen_tables) - 决定启动时是否强制预先打开所有表
  * [pseudo_sharding](Server_settings/Searchd.md#pseudo_sharding) - 为普通表和实时表的搜索查询启用伪分片
  * [qcache_max_bytes](Server_settings/Searchd.md#qcache_max_bytes) - 为缓存结果集分配的最大 RAM
  * [qcache_thresh_msec](Server_settings/Searchd.md#qcache_thresh_msec) - 查询结果可被缓存的最小墙钟时间阈值
  * [qcache_ttl_sec](Server_settings/Searchd.md#qcache_ttl_sec) - 缓存结果集的过期时间
  * [query_log](Server_settings/Searchd.md#query_log) - 查询日志文件路径
  * [query_log_format](Server_settings/Searchd.md#query_log_format) - 查询日志格式
  * [query_log_min_msec](Server_settings/Searchd.md#query_log_min_msec) - 防止记录过快的查询
  * [query_log_mode](Server_settings/Searchd.md#query_log_mode) - 查询日志文件权限模式
  * [read_buffer_docs](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#read_buffer_docs) - 每个关键词的文档列表读取缓冲区大小
  * [read_buffer_hits](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#read_buffer_docs) - 每个关键词的命中列表读取缓冲区大小
  * [read_unhinted](Server_settings/Searchd.md#read_unhinted) - 未提示读取大小
  * [rt_flush_period](Server_settings/Searchd.md#rt_flush_period) - Manticore 将实时表的 RAM 块刷新到磁盘的频率
  * [rt_merge_iops](Server_settings/Searchd.md#rt_merge_iops) - 实时块合并线程允许执行的最大 I/O 操作数（每秒）
  * [rt_merge_maxiosize](Server_settings/Searchd.md#rt_merge_maxiosize) - 实时块合并线程允许执行的最大 I/O 操作大小
  * [seamless_rotate](Server_settings/Searchd.md#seamless_rotate) - 在旋转需要预缓存大量数据的表时，防止 searchd 卡顿
  * [secondary_indexes](Server_settings/Searchd.md#secondary_indexes) - 启用在搜索查询中使用二级索引
  * [server_id](Server_settings/Searchd.md#server_id) - 用作生成唯一文档 ID 种子的服务器标识符
  * [shutdown_timeout](Server_settings/Searchd.md#shutdown_timeout) - Searchd `--stopwait` 超时时间
  * [shutdown_token](Server_settings/Searchd.md#shutdown_token) - 从 VIP SQL 连接中调用 `shutdown` 命令所需密码的 SHA1 哈希值
  * [skiplist_cache_size](Server_settings/Searchd.md#skiplist_cache_size) - 解压后的 skiplist 的内存缓存最大大小
  * [snippets_file_prefix](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) - 在 `load_files` 模式生成摘要时，追加到本地文件名之前的前缀
  * [sphinxql_state](Server_settings/Searchd.md#sphinxql_state) - 当前 SQL 状态将被序列化到的文件路径
  * [sphinxql_timeout](Server_settings/Searchd.md#sphinxql_timeout) - MySQL 客户端两次请求之间的最长等待时间
  * [ssl_ca](Server_settings/Searchd.md#ssl_ca) - SSL 证书颁发机构证书文件路径
  * [ssl_cert](Server_settings/Searchd.md#ssl_cert) - 服务器 SSL 证书路径
  * [ssl_key](Server_settings/Searchd.md#ssl_key) - 服务器 SSL 证书密钥路径
  * [subtree_docs_cache](Server_settings/Searchd.md#subtree_docs_cache) - 最大公共子树文档缓存大小
  * [subtree_hits_cache](Server_settings/Searchd.md#subtree_hits_cache) - 最大公共子树命中缓存大小，按查询计算
  * [timezone](Server_settings/Searchd.md#timezone) - 日期/时间相关函数使用的时区
  * [thread_stack](Server_settings/Searchd.md#thread_stack) - 一个 job 的最大栈大小
  * [unlink_old](Server_settings/Searchd.md#unlink_old) - 旋转成功后是否取消链接 .old 表副本
  * [watchdog](Server_settings/Searchd.md#watchdog) - 是否启用或禁用 Manticore 服务器 watchdog

##### Searchd 启动参数
```bash
searchd [OPTIONS]
```
* [--check](Starting_the_server/Manually.md#searchd-command-line-options) - 检查配置文件并退出
* [--config, -c](Starting_the_server/Manually.md#searchd-command-line-options) - 指定配置文件路径
* [--console](Starting_the_server/Manually.md#searchd-command-line-options) - 强制服务器以控制台模式运行
* [--coredump](Starting_the_server/Manually.md#searchd-command-line-options) - 启用崩溃时保存 core dump
* [--cpustats](Starting_the_server/Manually.md#searchd-command-line-options) - 启用 CPU 时间报告
* [--auth](Starting_the_server/Manually.md#searchd-command-line-options) - 以交互式身份验证引导模式运行
* [--auth-non-interactive](Starting_the_server/Manually.md#searchd-command-line-options) - 通过 stdin 运行身份验证引导模式
* [--delete](Starting_the_server/Manually.md#searchd-command-line-options) - 从 Microsoft Management Console 以及其他已注册服务的位置移除 Manticore 服务
* [--force-preread](Starting_the_server/Manually.md#searchd-command-line-options) - 在表文件被预读完成前，阻止服务器处理传入连接
* [--help, -h](Starting_the_server/Manually.md#searchd-command-line-options) - 显示所有可用参数
* [--quiet, -q](Starting_the_server/Manually.md#searchd-command-line-options) - 启动时只打印错误
* [--table (--index)](Starting_the_server/Manually.md#searchd-command-line-options) - 仅限制服务器提供指定表
* [--install](Starting_the_server/Manually.md#searchd-command-line-options) - 将 searchd 作为服务安装到 Microsoft Management Console
* [--iostats](Starting_the_server/Manually.md#searchd-command-line-options) - 启用输入/输出报告
* [--listen, -l](Starting_the_server/Manually.md#searchd-command-line-options) - 覆盖配置文件中的 [listen](Server_settings/Searchd.md#listen)
* [--logdebug, --logdebugv, --logdebugvv](Starting_the_server/Manually.md#searchd-command-line-options) - 启用服务器日志中的额外调试输出
* [--logreplication](Starting_the_server/Manually.md#searchd-command-line-options) - 启用服务器日志中的额外复制调试输出
* [--new-cluster](Starting_the_server/Manually.md#searchd-command-line-options) - 初始化复制集群，并将服务器设为参考节点，同时启用 [cluster restart](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md) 保护
* [--new-cluster-force](Starting_the_server/Manually.md#searchd-command-line-options) - 初始化复制集群并将服务器设为参考节点，绕过 [cluster restart](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md) 保护
* [--nodetach](Starting_the_server/Manually.md#searchd-command-line-options) - 让 searchd 保持在前台运行
* [--ntservice](Starting_the_server/Manually.md#searchd-command-line-options) - 供 Microsoft Management Console 在 Windows 平台上将 searchd 作为服务启动
* [--pidfile](Starting_the_server/Manually.md#searchd-command-line-options) - 覆盖配置文件中的 [pid_file](Server_settings/Searchd.md#pid_file)
* [--port, p](Starting_the_server/Manually.md#searchd-command-line-options) - 指定 searchd 应监听的端口，忽略配置文件中指定的端口
* [--replay-flags](Starting_the_server/Manually.md#searchd-command-line-options) - 设置额外的二进制日志回放选项
* [--servicename](Starting_the_server/Manually.md#searchd-command-line-options) - 安装或删除服务时为 searchd 指定给定名称，并在 Microsoft Management Console 中显示
* [--status](Starting_the_server/Manually.md#searchd-command-line-options) - 查询正在运行的搜索服务以返回其状态
* [--stop](Starting_the_server/Manually.md#searchd-command-line-options) - 停止 Manticore 服务器
* [--stopwait](Starting_the_server/Manually.md#searchd-command-line-options) - 优雅地停止 Manticore 服务器
* [--strip-path](Starting_the_server/Manually.md#searchd-command-line-options) - 移除表中引用的所有文件名里的路径名
* [-v](Starting_the_server/Manually.md#searchd-command-line-options) - 显示版本信息

##### Searchd 环境变量
* [MANTICORE_TRACK_DAEMON_SHUTDOWN](Starting_the_server/Manually.md#Environment-variables) - 在 searchd 关闭期间启用详细日志记录

## [Indextool](Miscellaneous_tools.md#indextool)
一组有助于排障的表维护功能。
```bash
indextool <command> [options]
```
##### Indextool 启动参数
用于转储与物理表相关的各种调试信息。
```bash
indextool <command> [options]
```
* [--config, -c](Miscellaneous_tools.md#indextool) - 指定配置文件路径
* [--quiet, -q](Miscellaneous_tools.md#indextool) - 让 indextool 安静运行；不输出横幅等内容
* [--help, -h](Miscellaneous_tools.md#indextool) - 列出所有可用参数
* [-v](Miscellaneous_tools.md#indextool) - 显示版本信息
* [Indextool](Miscellaneous_tools.md#indextool) - 验证配置文件
* [--buildidf](Miscellaneous_tools.md#indextool) - 从一个或多个字典转储构建 IDF 文件
* [--build-infixes](Miscellaneous_tools.md#indextool) - 为现有 `dict=keywords` 表构建中缀
* [--dumpheader](Miscellaneous_tools.md#indextool) - 快速转储提供的表头文件
* [--dumpconfig](Miscellaneous_tools.md#indextool) - 以接近兼容的 manticore.conf 格式从给定表头文件转储表定义
* [--dumpheader](Miscellaneous_tools.md#indextool) - 在配置文件中查找表头路径时按表名转储表头
* [--dumpdict](Miscellaneous_tools.md#indextool) - 转储表字典
* [--dumpdocids](Miscellaneous_tools.md#indextool) - 按表名转储文档 ID
* [--dumphitlist](Miscellaneous_tools.md#indextool) - 转储指定表中给定关键词/ID 的所有出现位置
* [--docextract](Miscellaneous_tools.md#indextool) - 对整个字典/文档/命中执行表检查，并收集属于所请求文档的所有词和命中
* [--fold](Miscellaneous_tools.md#indextool) - 根据表设置测试分词
* [--htmlstrip](Miscellaneous_tools.md#indextool) - 使用指定表的 HTML 去除器设置过滤 STDIN
* [--mergeidf](Miscellaneous_tools.md#indextool) - 将多个 .idf 文件合并为一个文件
* [--morph](Miscellaneous_tools.md#indextool) - 对提供的 STDIN 应用词形处理并将结果输出到 stdout
* [--check](Miscellaneous_tools.md#indextool) - 检查表数据文件的一致性
* [--check-id-dups](Miscellaneous_tools.md#indextool) - 检查重复 ID
* [--check-disk-chunk](Miscellaneous_tools.md#indextool) - 检查 RT 表的单个磁盘块
* [--strip-path](Miscellaneous_tools.md#indextool) - 移除表中引用的所有文件名里的路径名
* [--rotate](Miscellaneous_tools.md#indextool) - 在使用 `--check` 时确定是否检查等待轮转的表
* [--apply-killlists](Miscellaneous_tools.md#indextool) - 对配置文件中列出的所有表应用 kill-list

## [Wordbreaker](Miscellaneous_tools.md#wordbreaker)
将复合词拆分为其组成部分。
```bash
wordbreaker [-dict path/to/dictionary_file] {split|test|bench}
```

##### Wordbreaker 启动参数
* [STDIN](Miscellaneous_tools.md#wordbreaker) - 接受一个要拆分成多个部分的字符串
* [-dict](Miscellaneous_tools.md#wordbreaker) - 指定要使用的词典文件
* [split|test|bench](Miscellaneous_tools.md#wordbreaker) - 指定命令

## [Spelldump](Miscellaneous_tools.md#spelldump)
使用 ispell 或 MySpell 格式提取词典文件内容

```bash
spelldump [options] <dictionary> <affix> [result] [locale-name]
```
* [dictionary](Miscellaneous_tools.md#spelldump) - 主词典文件
* [affix](Miscellaneous_tools.md#spelldump) - 该词典的词缀文件
* [result](Miscellaneous_tools.md#spelldump) - 指定词典数据的输出目标
* [locale-name](Miscellaneous_tools.md#spelldump) - 指定要使用的区域设置详情

## 保留关键字列表

Manticore SQL 语法中当前保留关键字的完整按字母排序列表（因此不能用作标识符）。

```
AND, AS, BY, COLUMNARSCAN, DISTINCT, DIV, DOCIDINDEX, EXPLAIN, FACET, FALSE, FORCE, FROM, HYBRID_MATCH, IGNORE, IN, INDEXES, INNER, IS, JOIN, KNN, LEFT, LIMIT, MOD, NOT, NO_COLUMNARSCAN, NO_DOCIDINDEX, NO_SECONDARYINDEX, NULL, OFFSET, ON, OR, ORDER, RELOAD, SECONDARYINDEX, SELECT, SYSFILTERS, TOKEN, TRUE
```

## 旧版 Manticore 文档

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
* [6.3.8](https://manual.manticoresearch.com/manticore-6-3-8/). [安装页面](https://manticoresearch.com/install-6.3-8/)
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
* [17.5.1](https://manual.manticoresearch.com/manticore-17-5-1/). [安装页面](https://manticoresearch.com/install-17.5.1/)
* [25.0.0](https://manual.manticoresearch.com/manticore-25-0-0/). [安装页面](https://manticoresearch.com/install-25.0.0/)
* [27.1.5](https://manual.manticoresearch.com/manticore-27-1-5/). [安装页面](https://manticoresearch.com/install-27.1.5/)
* [28.6.6](https://manual.manticoresearch.com/manticore-28-6-6/). [安装页面](https://manticoresearch.com/install-28.6.6/)
* [29.0.2](https://manual.manticoresearch.com/manticore-29-0-2/). [安装页面](https://manticoresearch.com/install-29.0.2/)
<!-- proofread -->
