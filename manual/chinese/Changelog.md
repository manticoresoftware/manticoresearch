# 更新日志

# 版本 9.2.14
发布日期：2025年3月28日

### 小改动
* [Commit ](https://github.com/manticoresoftware/manticoresearch/commit/fe9473dc3f4fcfa0eaae748b538fe853f4ce078b) 实现了 `--mockstack` 标志以计算递归操作堆栈需求。新的 `--mockstack` 模式会分析并报告递归表达式求值、pttern 匹配操作、过滤器处理所需的堆栈大小。计算出的堆栈需求会输出到控制台以便调试和优化。

* [Issue #3058](https://github.com/manticoresoftware/manticoresearch/pull/3058) 默认启用了 [boolean_simplify](../Searching/Options.md#boolean_simplify)。

* [Issue #3172](https://github.com/manticoresoftware/manticoresearch/issues/3172) 添加了一个新的配置选项：`searchd.kibana_version_string`，在使用期望特定 Elasticsearch 版本的 Kibana 或 OpenSearch Dashboards 与 Manticore 一起使用时，此选项可能非常有用。

* [Issue #3211](https://github.com/manticoresoftware/manticoresearch/issues/3211) 修复了 [CALL SUGGEST](../Searching/Spell_correction.md#CALL-QSUGGEST,-CALL-SUGGEST) 在处理2个字符的单词时的问题。

* [Issue #490](https://github.com/manticoresoftware/manticoresearch-buddy/issues/490) 改进了 [fuzzy search](../Searching/Spell_correction.md#Fuzzy-Search)：之前在搜索 "def ghi" 时，如果存在其他匹配文档，有时无法找到 "defghi"。

* ⚠️ BREAKING [Issue #3165](https://github.com/manticoresoftware/manticoresearch/issues/3165) 为了一致性，将某些 HTTP JSON 响应中的 `_id` 更改为 `id`。请确保相应地更新你的应用程序。

* ⚠️ BREAKING [Issue #3186](https://github.com/manticoresoftware/manticoresearch/issues/3186) 在集群加入过程中增加了对 `server_id` 的检查，以确保每个节点都有唯一的 ID。当加入的节点与集群中现有节点具有相同的 `server_id` 时，`JOIN CLUSTER` 操作现在可能会失败，并返回表示重复 [server_id](../Server_settings/Searchd.md#server_id) 的错误信息。为解决此问题，请确保复制集群中的每个节点都有唯一的 [server_id](../Server_settings/Searchd.md#server_id)。你可以在尝试加入集群前，在配置文件的 "searchd" 部分中将默认 [server_id](../Server_settings/Searchd.md#server_id) 更改为唯一值。此更改更新了复制协议。如果你正在运行复制集群，你需要：

  - 首先，正常停止所有节点

  - 然后，使用 Linux 中的工具 `manticore_new_cluster` 以 `--new-cluster` 参数启动最后停止的节点.

  - 阅读有关 [restarting a cluster](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster) 的更多详细信息.

### Bugfixes
* [Commit 6fda](https://github.com/manticoresoftware/manticoresearch/commit/6fdad3923dd6953f4b781943eed5ec5c28b7f808) 修复了因等待后丢失调度器而导致的崩溃问题；现在，像 `serializer` 这样的特定调度器能够被正确恢复.

* [Commit c333](https://github.com/manticoresoftware/manticoresearch/commit/c333b277d4a504de5fe298e9ab570a9f9ea31e0a) 修复了一个 bug，即右侧联接表的权重无法在 `ORDER BY` 子句中使用的问题.

* [Issue #2644](https://github.com/manticoresoftware/manticoresearch/issues/2644) gcc 14.2.0：修复了使用 `const knn::DocDist_t*&` 调用 `lower_bound` 时的错误. ❤️ 感谢 [@Azq2](https://github.com/Azq2) 的 PR.

* [Issue #3018](https://github.com/manticoresoftware/manticoresearch/issues/3018) 修复了自动模式插入过程中处理大写表名的问题.

* [Issue #3119](https://github.com/manticoresoftware/manticoresearch/issues/3119) 修复了解码无效 base64 输入时导致崩溃的问题.

* [Issue #3121](https://github.com/manticoresoftware/manticoresearch/issues/3121) 修复了 `ALTER` 操作中的两个 KNN 索引相关问题：浮点向量现在保留其原始维度，并且 KNN 索引现在能正确生成.

* [Issue #3123](https://github.com/manticoresoftware/manticoresearch/issues/3123) 修复了在空 JSON 列上构建辅助索引时导致崩溃的问题.

* [Issue #3138](https://github.com/manticoresoftware/manticoresearch/issues/3138) 修复了因重复条目导致的崩溃问题.

* [Issue #3151](https://github.com/manticoresoftware/manticoresearch/issues/3151) 修复了无法将 `fuzzy=1` 选项与 `ranker` 或 `field_weights` 一起使用的问题.

* [Issue #3163](https://github.com/manticoresoftware/manticoresearch/issues/3163) 修复了 `SET GLOBAL timezone` 无效的 bug.

* [Issue #3181](https://github.com/manticoresoftware/manticoresearch/issues/3181) 修复了使用大于2^63的 ID 时文本字段值可能丢失的问题.

* [Issue #3189](https://github.com/manticoresoftware/manticoresearch/issues/3189) 修复了：`UPDATE` 语句现在能够正确遵循 `query_log_min_msec` 设置.

* [Issue #3247](https://github.com/manticoresoftware/manticoresearch/issues/3247) 修复了在保存实时磁盘块时可能发生的竞态条件，该条件可能导致 `JOIN CLUSTER` 失败.


# 版本 7.4.6
发布日期：2025年2月28日

### 主要变更
* [Issue #832](https://github.com/manticoresoftware/manticoresearch/issues/832) 与 [Kibana](../Integration/Kibana.md) 集成，以实现更简单、更高效的数据可视化.

### 小改动
* [Issue #1727](https://github.com/manticoresoftware/manticoresearch/issues/1727) 修复了 arm64 与 x86_64 之间的浮点精度差异问题.

* [Issue #2995](https://github.com/manticoresoftware/manticoresearch/issues/2995) 实现了联接批处理的性能优化.

* [Issue #3039](https://github.com/manticoresoftware/manticoresearch/issues/3039) 对直方图中的 EstimateValues 实现了性能优化.

* [Issue #3099](https://github.com/manticoresoftware/manticoresearch/issues/3099) 增加了对 Boost 1.87.0 的支持. ❤️ 感谢 [@cho-m](https://github.com/cho-m) 的 PR.
* [Issue #77](https://github.com/manticoresoftware/columnar/issues/77) 在创建多个值的过滤器时优化了块数据重用；向属性元数据添加了最小/最大值；根据最小/最大值实现了过滤器值的预过滤。

### Bugfixes
* [Commit 73ac](https://github.com/manticoresoftware/manticoresearch/commit/73ac22f358a7a0e734b332c3943c86e6294c06c3) 修复了在使用来自左表和右表的属性时，连接查询中表达式的处理；修复了右表的 index_weights 选项。
* [Issue #2915](https://github.com/manticoresoftware/manticoresearch/issues/2915) 在 `SELECT ... JOIN` 查询中使用 `avg()` 可能导致不正确的结果；现在已修复。
* [Issue #2996](https://github.com/manticoresoftware/manticoresearch/issues/2996) 修复了因连接批处理启用时的隐式截止导致的不正确结果集。
* [Issue #3031](https://github.com/manticoresoftware/manticoresearch/issues/3031) 修复了在活跃的块合并进行时，守护进程在关闭时崩溃的问题。
* [Issue #3037](http://github.com/manticoresoftware/manticoresearch/issues/3037) 修复了 `IN(...)` 可能产生不正确结果的问题。
* [Issue #3038](https://github.com/manticoresoftware/manticoresearch/issues/3038) 在版本 7.0.0 中设置 `max_iops` / `max_iosize` 可能会降低索引性能；现在已修复。
* [Issue #3042](https://github.com/manticoresoftware/manticoresearch/issues/3042) 修复了连接查询缓存中的内存泄漏。
* [Issue #3052](https://github.com/manticoresoftware/manticoresearch/issues/3052) 修复了连接 JSON 查询中的查询选项处理。
* [Issue #3054](https://github.com/manticoresoftware/manticoresearch/issues/3054) 修复了 ATTACH TABLE 命令的问题。
* [Issue #3079](https://github.com/manticoresoftware/manticoresearch/issues/3079) 修复了错误消息中的不一致性。
* [Issue #3087](https://github.com/manticoresoftware/manticoresearch/issues/3087) 在每个表中设置 `diskchunk_flush_write_timeout=-1` 并没有禁用索引刷新；现在已修复。
* [Issue #3088](https://github.com/manticoresoftware/manticoresearch/issues/3088) 解决了大 ID 批量替换后的重复条目问题。
* [Issue #3126](https://github.com/manticoresoftware/manticoresearch/issues/3126) 修复了因全文本查询中使用单个 `NOT` 操作符及表达式排名器导致的守护进程崩溃。
* [Issue #3128](https://github.com/manticoresoftware/manticoresearch/pull/3128) 修复了 CJSON 库中的潜在漏洞。❤️ 感谢 [@tabudz](https://github.com/tabudz) 的 PR。

# Version 7.0.0
Released: January 30th 2025

### Major changes
* [Issue #1497](https://github.com/manticoresoftware/manticoresearch/issues/1497) 添加了新的 [模糊搜索](../Searching/Spell_correction.md#Fuzzy-Search) 和 [自动完成](../Searching/Autocomplete.md#CALL-AUTOCOMPLETE) 功能，以便于搜索。
* [Issue #1500](https://github.com/manticoresoftware/manticoresearch/issues/1500) [与 Kafka 集成](../Integration/Kafka.md#Syncing-from-Kafka)。
* [Issue #1928](https://github.com/manticoresoftware/manticoresearch/issues/1928) 引入了 [JSON 的二级索引](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#json_secondary_indexes)。
* [Issue #2361](https://github.com/manticoresoftware/manticoresearch/issues/2361) 更新和搜索在更新期间不再被块合并阻塞。
* [Issue #2787](https://github.com/manticoresoftware/manticoresearch/issues/2787) 为 RT 表自动 [磁盘块刷新](../Server_settings/Searchd.md#diskchunk_flush_write_timeout) 以提高性能；现在，我们自动将 RAM 块刷新到磁盘块，防止因 RAM 块缺乏优化而导致的性能问题，这有时会根据块大小导致不稳定。
* [Issue #2811](https://github.com/manticoresoftware/manticoresearch/issues/2811) 提供 [滚动](../Searching/Pagination.md#Scroll-Search-Option) 选项以便于分页。
* [Issue #931](https://github.com/manticoresoftware/manticoresearch/issues/931) 与 [Jieba](https://github.com/fxsjy/jieba) 集成，以便于 [中文分词](../Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md)。

### Minor changes
* ⚠️ BREAKING [Issue #1111](https://github.com/manticoresoftware/manticoresearch/issues/1111) 修复了 RT 表中 `global_idf` 的支持。需要重新创建表。
* ⚠️ BREAKING [Issue #2103](https://github.com/manticoresoftware/manticoresearch/issues/2103) 从内部 `cjk` 字符集删除了泰文字符。请相应地更新您的字符集定义：如果您的字符集为 `cjk,non_cjk` 且泰文字符对您很重要，请将其更改为 `cjk,thai,non_cjk`，或者 `cont,non_cjk`，其中 `cont` 是对所有连写脚本语言的新的称谓（即 `cjk` + `thai`）。使用 [ALTER TABLE](../Updating_table_schema_and_settings.md#Updating-table-FT-settings-in-RT-mode) 修改现有表。
* ⚠️ BREAKING [Issue #2468](https://github.com/manticoresoftware/manticoresearch/issues/2468) [CALL SUGGEST / QSUGGEST](../Searching/Spell_correction.md#CALL-QSUGGEST,-CALL-SUGGEST) 现在与分布式表兼容。这增加了主/代理协议版本。如果您在具有多个实例的分布式环境中运行 Manticore Search，请确保首先升级代理，然后再升级主节点。
* ⚠️ BREAKING [Issue #2889](https://github.com/manticoresoftware/manticoresearch/issues/2889) 将列名从 `Name` 更改为 PQ [SHOW META](../Node_info_and_management/SHOW_META.md#SHOW-META-for-PQ-tables) 的 `Variable name`。
* ⚠️ BREAKING [Issue #879](https://github.com/manticoresoftware/manticoresearch/issues/879) 引入了 [每表 binlog](../Logging/Binary_logging.md#Per-table-binary-logging-configuration) 及新选项: [binlog_common](../Logging/Binary_logging.md#Binary-logging-strategies), [binlog](../Logging/Binary_logging.md#Per-table-binary-logging-configuration) 用于 `create table` / `alter table`。在升级到新版本之前，您需要对 Manticore 实例进行干净的关闭。
* ⚠️ BREAKING [Issue #1789](https://github.com/manticoresoftware/manticoresearch/issues/1789) 修复了节点以错误的复制协议版本加入集群时的错误信息。此更改更新了复制协议。如果您正在运行一个复制集群，您需要：
  - 首先，干净地停止所有节点
  - 然后，以 `--new-cluster` 启动最后停止的节点，使用工具 `manticore_new_cluster` 在 Linux 上进行操作。
  - 有关更多详细信息，请阅读 [重启集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster)。
* ⚠️ BREAKING [Issue #2308](https://github.com/manticoresoftware/manticoresearch/issues/2308) 为 [`ALTER CLUSTER ADD` 和 `DROP`](../Creating_a_cluster/Setting_up_replication/Adding_and_removing_a_table_from_a_replication_cluster.md#Adding-and-removing-a-table-from-a-replication-cluster) 添加了对多个表的支持。该更改也影响复制协议。有关处理此更新的指导，请参考前面的部分。
* [Issue #2997](https://github.com/manticoresoftware/manticoresearch/issues/2997) 修复了 Macos 上的 dlopen 问题。
* [Commit 4954](https://github.com/manticoresoftware/manticoresearch/commit/4954b5de7341a29902a9b8fbb9a040f7942c77c4) 更改了具有 KNN 索引的表上 OPTIMIZE TABLE 的默认截止值，以提高搜索性能。
* [Commit cfc8](https://github.com/manticoresoftware/manticoresearch/commit/cfc87ecb6e33a8163c2235243b6b40e699dbf526) 在 `FACET` 和 `GROUP BY` 中为 `ORDER BY` 添加了 `COUNT(DISTINCT)` 的支持。
* [Issue #1103](https://github.com/manticoresoftware/manticoresearch/issues/1103) 提高了 [日志记录](../Logging/Server_logging.md#Server-logging) 块合并的清晰度。
* [Issue #1130](https://github.com/manticoresoftware/manticoresearch/issues/1130) 添加了对 [DBeaver](../Integration/DBeaver.md) 的支持。
* [Issue #1546](https://github.com/manticoresoftware/manticoresearch/issues/1546) 为 [POLY2D()](../Functions/Geo_spatial_functions.md#POLY2D%28%29)/[GEOPOLY2D()](../Functions/Geo_spatial_functions.md#GEOPOLY2D%28%29) 函数实现了二级索引。
* [Issue #1630](https://github.com/manticoresoftware/manticoresearch/issues/1630) HTTP 请求现在支持 `Content-Encoding: gzip`。
* [Issue #1831](https://github.com/manticoresoftware/manticoresearch/issues/1831) 添加了 `SHOW LOCKS` 命令。
* [Issue #2187](https://github.com/manticoresoftware/manticoresearch/issues/2197) 允许 Buddy 请求守护进程绕过 [searchd.max_connections](../Server_settings/Searchd.md#max_connections) 限制。
* [Issue #2208](https://github.com/manticoresoftware/manticoresearch/issues/2208) 添加了通过 JSON HTTP 接口连接表的支持。
* [Issue #2235](https://github.com/manticoresoftware/manticoresearch/issues/2235) 通过 Buddy 记录成功处理的查询及其原始形式。
* [Issue #2249](https://github.com/manticoresoftware/manticoresearch/issues/2249) 添加了为复制表运行 `mysqldump` 的特殊模式。
* [Issue #2268](https://github.com/manticoresoftware/manticoresearch/issues/2268) 改进了在复制和 `ALTER TABLE` 语句中重命名外部文件的功能。
* [Issue #2402](https://github.com/manticoresoftware/manticoresearch/issues/2402) 将 [searchd.max_packet_size](../Server_settings/Searchd.md#max_packet_size) 的默认值更新为 128MB。
* [Issue #2419](https://github.com/manticoresoftware/manticoresearch/issues/2419) 在 JSON ["match"](../Searching/Full_text_matching/Basic_usage.md#match) 中添加了对 [IDF 增益修饰符](../Searching/Full_text_matching/Operators.md#IDF-boost-modifier) 的支持。
* [Issue #2430](https://github.com/manticoresoftware/manticoresearch/issues/2430) 增强了 [binlog](../Logging/Binary_logging.md#Binary-logging) 写入同步以防止错误。
* [Issue #2458](https://github.com/manticoresoftware/manticoresearch/issues/2458) 在 Windows 套件中整合了 zlib 支持。
* [Issue #2479](https://github.com/manticoresoftware/manticoresearch/issues/2479) 添加了对 SHOW TABLE INDEXES 命令的支持。
* [Issue #2485](https://github.com/manticoresoftware/manticoresearch/issues/2485) 为 Buddy 回复设置会话元数据。
* [Issue #2490](https://github.com/manticoresoftware/manticoresearch/issues/2490) 在兼容性端点上实现了毫秒精度的聚合。
* [Issue #2500](https://github.com/manticoresoftware/manticoresearch/issues/2500) 更改了复制操作失败时的错误消息。
* [Issue #2584](https://github.com/manticoresoftware/manticoresearch/pull/2584) 在 SHOW STATUS 中新增 [性能指标](../Node_info_and_management/Node_status.md#Query-Time-Statistics): 最小/最大/平均/95 倍和 99 倍分位数针对过去 1、5 和 15 分钟的每种查询类型。
* [Issue #2639](https://github.com/manticoresoftware/manticoresearch/pull/2639) 将请求和响应中的所有 `index` 替换为 `table`。
* [Issue #2643](https://github.com/manticoresoftware/manticoresearch/issues/2643) 在 HTTP `/sql` 端点的聚合结果中添加了 `distinct` 列。
* [Issue #268](https://github.com/manticoresoftware/manticoresearch-buddy/issues/268) 实现了从 Elasticsearch 导入的数据类型的自动检测。
* [Issue #2744](https://github.com/manticoresoftware/manticoresearch/issues/2744) 添加了对字符串 JSON 字段比较表达式的排序支持。
* [Issue #2752](https://github.com/manticoresoftware/manticoresearch/issues/2752) 在选择列表中添加了对 `uuid_short` 表达式的支持。
* [Issue #2783](https://github.com/manticoresoftware/manticoresearch/issues/2783) Manticore Search 现在直接运行 Buddy，不再使用 `manticore-buddy` 包装器。
* [Issue #2785](https://github.com/manticoresoftware/manticoresearch/issues/2785) 为缺少表和不支持插入操作的表提供了差异化的错误消息。
* [Issue #2789](https://github.com/manticoresoftware/manticoresearch/issues/2789) OpenSSL 3 现在静态构建到 `searchd` 中。
* [Issue #2790](https://github.com/manticoresoftware/manticoresearch/issues/2790) 增加了 `CALL uuid_short` 语句，以生成多个 `uuid_short` 值的序列。
* [Issue #2803](https://github.com/manticoresoftware/manticoresearch/issues/2803) 为 JOIN 操作中的右表增加了单独选项。
* [Issue #2810](https://github.com/manticoresoftware/manticoresearch/issues/2810) 改进了 HTTP JSON 聚合性能，以匹配 SphinxQL 中的 `GROUP BY`。
* [Issue #2854](https://github.com/manticoresoftware/manticoresearch/issues/2854) 为 Kibana 日期相关请求增加了对 `fixed_interval` 的支持。
* [Issue #2909](https://github.com/manticoresoftware/manticoresearch/issues/2909) 实现了 JOIN 查询的批处理，这提高了某些 JOIN 查询的性能，达到几百倍甚至上千倍。
* [Issue #2937](https://github.com/manticoresoftware/manticoresearch/issues/2937) 启用了在 fullscan 查询中使用连接表权重。
* [Issue #2953](https://github.com/manticoresoftware/manticoresearch/issues/2953) 修复了连接查询的日志记录。
* [Issue #337](https://github.com/manticoresoftware/manticoresearch-buddy/issues/337) 在非调试模式下隐藏了来自 `searchd` 日志的 Buddy 异常。
* [Issue #2931](https://github.com/manticoresoftware/manticoresearch/issues/2931) 如果用户设置了错误的端口用于复制监听器，则守护进程关闭并显示错误消息。

### Bug fixes
* [Commit 0c6b](https://github.com/manticoresoftware/manticoresearch/commit/0c6bdaf41ea3684e05952e1a8273893535b923f5) 修复：在包含超过 32 列的 JOIN 查询中返回错误结果。
* [Issue #2335](https://github.com/manticoresoftware/manticoresearch/issues/2335) 解决了在条件中使用两个 json 属性时表连接失败的问题。
* [Issue #2338](https://github.com/manticoresoftware/manticoresearch/issues/2338) 修复了在包含 [cutoff](../Searching/Options.md#cutoff) 的多查询中错误的 total_relation。
* [Issue #2366](https://github.com/manticoresoftware/manticoresearch/issues/2366) 修正了对右表中的 `json.string` 的过滤。
* [Issue #2406](https://github.com/manticoresoftware/manticoresearch/issues/2406) 启用在任何 POST HTTP JSON 端点（插入/替换/批量）中对所有值使用 `null`。在这种情况下，将使用默认值。
* [Issue #2418](https://github.com/manticoresoftware/manticoresearch/issues/2418) 通过调整 [max_packet_size](../Server_settings/Searchd.md#max_packet_size) 网络缓冲区的分配，优化内存消耗以进行初始套接字探测。
* [Issue #2420](https://github.com/manticoresoftware/manticoresearch/issues/2420) 修复了通过 JSON 接口将无符号整型插入到 bigint 属性中的问题。
* [Issue #2422](https://github.com/manticoresoftware/manticoresearch/issues/2422) 修复第二级索引以正确与启用排除过滤器和伪分片一起工作。
* [Issue #2423](https://github.com/manticoresoftware/manticoresearch/issues/2423) 解决了 [manticore_new_cluster](../Starting_the_server/Manually.md#searchd-command-line-options) 中的一个错误。
* [Issue #2448](https://github.com/manticoresoftware/manticoresearch/issues/2448) 解决了由于格式错误的 `_update` 请求导致的守护进程崩溃。
* [Issue #2452](https://github.com/manticoresoftware/manticoresearch/issues/2452) 修复了直方图无法处理带有排除的值过滤器的问题。
* [Issue #55](https://github.com/manticoresoftware/columnar/issues/55) 修复了在分布式表上的 [knn](../Searching/KNN.md#KNN-vector-search) 查询。
* [Issue #68](https://github.com/manticoresoftware/columnar/issues/68) 增强了对列访问器中表编码的排除过滤器的处理。
* [Commit 0eb1](https://github.com/manticoresoftware/manticoresearch/commit/0eb1579462013cb134bcaf74e06f6eeb0c2ecc4d) 修复了表达式解析器未遵循重新定义的 `thread_stack`。
* [Commit c304](https://github.com/manticoresoftware/manticoresearch/commit/c3041fd1d3e14773f3c5aa635343915a71f5cb95) 修复了克隆列式 IN 表达式时的崩溃。
* [Commit edad](https://github.com/manticoresoftware/columnar/commit/edadc694c68d6022bdd13134263667430a42cc1d) 修复了位图迭代器中的反转问题，该问题导致崩溃。
* [Commit fc30](https://github.com/manticoresoftware/executor/commit/fc302ff1117b0b835a6f5a3c07057baf5fba14d9) 修复了某些 Manticore 软件包被 `unattended-upgrades` 自动删除的问题。
* [Issue #1019](https://github.com/manticoresoftware/manticoresearch/issues/1019) 改进了来自 DbForge MySQL 工具的查询处理。
* [Issue #1107](https://github.com/manticoresoftware/manticoresearch/issues/1107) 修复了在 `CREATE TABLE` 和 `ALTER TABLE` 中特殊字符的转义问题。❤️ 感谢 [@subnix](https://github.com/subnix) 提交的 PR。
* [Issue #116](https://github.com/manticoresoftware/manticoresearch-backup/issues/116#issuecomment-2216597206) 修复了在冻结索引中更新 Blob 属性时发生的死锁。死锁是由于在尝试解冻索引时锁冲突引起的。这也可能导致 manticore-backup 失败。
* [Issue #1818](https://github.com/manticoresoftware/manticoresearch/issues/1818) `OPTIMIZE` 现在在表被冻结时抛出错误。
* [Issue #2001](https://github.com/manticoresoftware/manticoresearch/issues/2001) 允许使用函数名称作为列名。
* [Issue #2153](https://github.com/manticoresoftware/manticoresearch/issues/2153) 修复了在查询未知磁盘块的表设置时守护进程崩溃的问题。
* [Issue #2184](https://github.com/manticoresoftware/manticoresearch/issues/2184) 修复了在 `FREEZE` 和 `FLUSH RAMCHUNK` 后 `searchd` 停止时出现的挂起问题。
* [Issue #2228](https://github.com/manticoresoftware/manticoresearch/issues/2228) 从保留字中移除了与日期/时间相关的标记（和正则表达式）。
* [Issue #2255](https://github.com/manticoresoftware/manticoresearch/issues/2255) 修复了在 `FACET` 中有超过 5 个排序字段时的崩溃问题。
* [Issue #2265](https://github.com/manticoresoftware/manticoresearch/issues/2265) 修复了在启用 `index_field_lengths` 时无法恢复 `mysqldump` 的问题。
* [Issue #2291](https://github.com/manticoresoftware/manticoresearch/issues/2291) 修复了执行 `ALTER TABLE` 命令时的崩溃问题。
* [Issue #2333](https://github.com/manticoresoftware/manticoresearch/issues/2333) 修复了 Windows 包中的 MySQL DLL，以便索引器能够正常工作。
* [Issue #2393](https://github.com/manticoresoftware/manticoresearch/issues/2393) 修复了 GCC 编译错误。❤️ 感谢 [@animetosho](https://github.com/animetosho) 提交的 PR。
* [Issue #2447](https://github.com/manticoresoftware/manticoresearch/issues/2447) 修复了在 [_update](../Data_creation_and_modification/Updating_documents/REPLACE.md?client=Elasticsearch-like+partial) 中的转义问题。
* [Issue #2460](https://github.com/manticoresoftware/manticoresearch/issues/2460) 修复了在声明多个具有相同名称的属性或字段时索引器崩溃的问题。
* [Issue #2467](https://github.com/manticoresoftware/manticoresearch/issues/2467) 解决了由于错误变换嵌套布尔查询导致守护进程崩溃的问题，涉及“compat”搜索相关的端点。
* [Issue #2493](https://github.com/manticoresoftware/manticoresearch/issues/2493) 修复了在带修饰符的短语中的扩展问题。
* [Issue #2535](https://github.com/manticoresoftware/manticoresearch/issues/2535) 解决了在使用 [ZONE](../Searching/Full_text_matching/Operators.md#ZONE-limit-operator) 或 [ZONESPAN](../Searching/Full_text_matching/Operators.md#ZONESPAN-limit-operator) 全文操作符时的守护进程崩溃问题。
* [Issue #2552](https://github.com/manticoresoftware/manticoresearch/issues/2552) 修复了使用关键字字典的普通和 RT 表的中缀生成。
* [Issue #2571](https://github.com/manticoresoftware/manticoresearch/issues/2571) 修复了在 `FACET` 查询中的错误回复；将 `FACET` 中 `COUNT(*)` 的默认排序顺序设置为 `DESC`。
* [Issue #2580](https://github.com/manticoresoftware/manticoresearch/issues/2580) 修复了 Windows 启动时的守护进程崩溃。
* [Issue #2603](https://github.com/manticoresoftware/manticoresearch/issues/2603) 修复了 HTTP 端点 `/sql` 和 `/sql?mode=raw` 的查询截断问题；使来自这些端点的请求在不需要 `query=` 头的情况下保持一致。
* [Issue #2623](https://github.com/manticoresoftware/manticoresearch/issues/2623) 修复了自动模式创建表但同时失败的问题。
* [Issue #2627](https://github.com/manticoresoftware/manticoresearch/issues/2627) 修复了 HNSW 库以支持加载多个 KNN 索引。
* [Issue #2630](https://github.com/manticoresoftware/manticoresearch/issues/2630) 修复了当多个条件同时发生时的冻结问题。
* [Issue #2645](https://github.com/manticoresoftware/manticoresearch/issues/2645) 修复了使用 `OR` 进行 KNN 搜索时崩溃的问题。
* [Issue #2647](https://github.com/manticoresoftware/manticoresearch/issues/2647) 修复了 `indextool --mergeidf *.idf --out global.idf` 在创建后删除输出文件的问题。
* [Issue #2658](https://github.com/manticoresoftware/manticoresearch/issues/2658) 修复了在外部选择中使用 `ORDER BY` 字符串进行子选择时的守护进程崩溃。
* [Issue #2686](https://github.com/manticoresoftware/manticoresearch/issues/2686) 修复了同时更新浮点属性和字符串属性时的崩溃。
* [Issue #2704](https://github.com/manticoresoftware/manticoresearch/issues/2704) 修复了来自 `lemmatize_xxx_all` 令牌器的多个停用词增加后续令牌的 `hitpos` 问题。
* [Issue #2708](https://github.com/manticoresoftware/manticoresearch/issues/2708) 修复了在 `ALTER ... ADD COLUMN ... TEXT` 时的崩溃问题。
* [Issue #2737](https://github.com/manticoresoftware/manticoresearch/issues/2737) 修复了在冻结表中更新 blob 属性时，即使有一个 RAM 块，也会导致后续 `SELECT` 查询等待直到表解冻的问题。
* [Issue #2742](https://github.com/manticoresoftware/manticoresearch/issues/2742) 修复了对于具有打包因素的查询跳过查询缓存的问题。
* [Issue #2775](https://github.com/manticoresoftware/manticoresearch/issues/2775) 现在 Manticore 在未知操作上报告错误，而不是在 `_bulk` 请求中崩溃。
* [Issue #2791](https://github.com/manticoresoftware/manticoresearch/issues/2791) 修复了 HTTP `_bulk` 端点插入文档 ID 返回的问题。
* [Issue #2797](https://github.com/manticoresoftware/manticoresearch/issues/2797) 修复了在处理多个表时的分组器崩溃问题，其中一个表为空，而另一个表有不同数量的匹配条目。
* [Issue #2835](https://github.com/manticoresoftware/manticoresearch/issues/2835) 修复了复杂 `SELECT` 查询中的崩溃问题。
* [Issue #2872](https://github.com/manticoresoftware/manticoresearch/issues/2872) 如果 `IN` 表达式中的 `ALL` 或 `ANY` 参数不是 JSON 属性，则添加错误消息。
* [Issue #2882](https://github.com/manticoresoftware/manticoresearch/issues/2882) 修复了在大型表中更新 MVA 时的守护进程崩溃。
* [Issue #2888](https://github.com/manticoresoftware/manticoresearch/issues/2888) 修复了在 `libstemmer` 中令牌化失败时的崩溃问题。 ❤️ 感谢 [@subnix](https://github.com/subnix) 提交的 PR。
* [Issue #2919](https://github.com/manticoresoftware/manticoresearch/issues/2919) 修复了右表的连接权重在表达式中无法正常工作的情况。
* [Issue #2919](https://github.com/manticoresoftware/manticoresearch/issues/2919) 修复了右连接表权重在表达式中不起作用的问题。
* [Issue #325](https://github.com/manticoresoftware/manticoresearch-buddy/issues/325) 修复了当表已存在时，`CREATE TABLE IF NOT EXISTS ... WITH DATA` 失败的问题。
* [Issue #351](https://github.com/manticoresoftware/manticoresearch-buddy/issues/351) 修复了在使用文档 ID 进行 KNN 计数时未定义数组键 "id" 的错误。
* [Issue #359](https://github.com/manticoresoftware/manticoresearch-buddy/issues/359) 修复了 `REPLACE INTO cluster_name:table_name` 功能。
* [Issue #67](https://github.com/manticoresoftware/docker/issues/67) 修复了在使用 `--network=host` 运行 Manticore Docker 容器时的致命错误。

# 版本 6.3.8
发布日期：2024年11月22日

版本 6.3.8 继续 6.3 系列，并仅包含错误修复。

### 错误修复

* [PR #2777](https://github.com/manticoresoftware/manticoresearch/pull/2777) 修复了当查询并发受到 `threads` 或 `max_threads_per_query` 设置限制时，可用线程的计算问题。

# manticore-extra v1.1.20

发布日期：2024年10月7日

### 错误修复

* [Issue #64](https://github.com/manticoresoftware/executor/issues/64) 解决了一个问题，即 `unattended-upgrades` 实用工具在基于 Debian 的系统上自动安装软件包更新时，会错误地标记多个 Manticore 软件包，包括 `manticore-galera`、`manticore-executor` 和 `manticore-columnar-lib`，以供删除。此问题是由于 `dpkg` 错误地将虚拟软件包 `manticore-extra` 视为冗余而引起的。已进行更改以确保 `unattended-upgrades` 不再尝试删除必要的 Manticore 组件。

# 版本 6.3.6
发布日期：2024年8月2日

版本 6.3.6 继续 6.3 系列，并仅包括错误修复。

### 错误修复

* [Issue #2477](https://github.com/manticoresoftware/manticoresearch/issues/2477) 修复了版本 6.3.4 中引入的崩溃问题，该崩溃可能在处理表达式和分布式或多个表时发生。
* [Issue #2352](https://github.com/manticoresoftware/manticoresearch/issues/2352) 修复了由于 `max_query_time` 导致在查询多个索引时提前退出引起的守护进程崩溃或内部错误。

# 版本 6.3.4
发布日期：2024年7月31日

版本 6.3.4 继续 6.3 系列，仅包含一些小改进和错误修复。

### 小更改
* [Issue #1130](https://github.com/manticoresoftware/manticoresearch/issues/1130) 增加对 [DBeaver](../Integration/DBeaver.md#Integration-with-DBeaver) 的支持。
* [Issue #2146](https://github.com/manticoresoftware/manticoresearch/issues/2146) 改进了单词形式和例外中的定界符转义。
* [Issue #2315](https://github.com/manticoresoftware/manticoresearch/issues/2315) 向 SELECT 列表表达式添加了字符串比较运算符。
* [Issue #2363](https://github.com/manticoresoftware/manticoresearch/issues/2363) 增加对 Elastic 风格批量请求中的空值的支持。
* [Issue #2374](https://github.com/manticoresoftware/manticoresearch/issues/2374) 增加对 mysqldump 版本 9 的支持。
* [Issue #2375](https://github.com/manticoresoftware/manticoresearch/issues/2375) 改进了带有 JSON 路径的 HTTP JSON 查询中的错误处理，以指向错误发生的节点。

### 错误修复
* [Issue #2280](https://github.com/manticoresoftware/manticoresearch/issues/2280) 修复了在 disk_chunks > 1 的情况下通配符查询中许多匹配项导致的性能下降。
* [Issue #2332](https://github.com/manticoresoftware/manticoresearch/issues/2332) 修复了在空 MVA 数组的 MVA MIN 或 MAX SELECT 列表表达式中的崩溃。
* [Issue #2339](https://github.com/manticoresoftware/manticoresearch/issues/2339) 修复了对 Kibana 的无限范围请求的错误处理。
* [Issue #2342](https://github.com/manticoresoftware/manticoresearch/issues/2342) 修复了在 SELECT 列表中未包含属性时，从右表对列属性的连接过滤。
* [Issue #2343](https://github.com/manticoresoftware/manticoresearch/issues/2343) 修复了 Manticore 6.3.2 中重复的 'static' 修饰符。
* [Issue #2344](https://github.com/manticoresoftware/manticoresearch/issues/2344) 修复了在使用右表上的 MATCH() 时，LEFT JOIN 返回不匹配条目的问题。
* [Issue #2350](https://github.com/manticoresoftware/manticoresearch/issues/2350) 修复了在 RT 索引中使用 `hitless_words` 时保存磁盘块的问题。
* [Issue #2364](https://github.com/manticoresoftware/manticoresearch/issues/2364) 'aggs_node_sort' 属性现在可以在其他属性之间以任何顺序添加。
* [Issue #2368](https://github.com/manticoresoftware/manticoresearch/issues/2368) 修复了 JSON 查询中全文检索与过滤顺序的问题。
* [Issue #2376](https://github.com/manticoresoftware/manticoresearch/issues/2376) 修复了与长 UTF-8 请求的 JSON 响应不正确相关的错误。
* [Issue #2684](https://github.com/manticoresoftware/dev/issues/2684) 修复了依赖于连接属性的预排序/预过滤表达式的计算。
* [Issue #301](https://github.com/manticoresoftware/manticoresearch-buddy/issues/301) 更改了计算度量数据大小的方法，从读取 `manticore.json` 文件而不是检查数据目录的完整大小。
* [Issue #302](https://github.com/manticoresoftware/manticoresearch-buddy/issues/302) 增加了对 Vector.dev 的验证请求的处理。

# 版本 6.3.2
发布日期：2024年6月26日

版本 6.3.2 继续 6.3 系列，并包括若干错误修复，其中一些在 6.3.0 发布后被发现。

### 重大更改
* ⚠️[Issue #2305](https://github.com/manticoresoftware/manticoresearch/issues/2305) 更新了 aggs.range 值为数字。

### 错误修复
* [Commit c51c](https://github.com/manticoresoftware/manticoresearch/commit/c51c7fa4bd0ec4ad8d53188b888461bff29f1417) 修复了按存储检查与 rset 合并的分组。
* [Commit 0e85](https://github.com/manticoresoftware/manticoresearch/commit/0e85b9d5682f51681750d798aef6a11cfb93d69e) 修复了在使用 CRC 字典并启用 `local_df` 时，在 RT 索引中使用通配符字符查询时导致的崩溃。
* [Issue #2200](https://github.com/manticoresoftware/manticoresearch/issues/2200) 修复了在没有 GROUP BY 的情况下，使用 `count(*)` 时的 JOIN 崩溃。
* [Issue #2201](https://github.com/manticoresoftware/manticoresearch/issues/2201) 修复了在尝试按全文字段进行分组时未返回警告的 JOIN。
* [Issue #2230](https://github.com/manticoresoftware/manticoresearch/issues/2230) 解决了通过 `ALTER TABLE` 添加属性时未考虑 KNN 选项的问题。
* [Issue #2231](https://github.com/manticoresoftware/manticoresearch/issues/2231) 修复了在版本 6.3.0 中删除 `manticore-tools` Redhat 包失败的问题。
* [Issue #2242](https://github.com/manticoresoftware/manticoresearch/issues/2242) 修正了 JOIN 和多个 FACET 语句返回不正确结果的问题。
* [Issue #2250](https://github.com/manticoresoftware/manticoresearch/issues/2250) 修复了在集群中的表上执行 ALTER TABLE 时产生错误。
* [Issue #2252](https://github.com/manticoresoftware/manticoresearch/issues/2252) 修复了原始查询通过 SphinxQL 接口传递给 buddy 的问题。
* [Issue #2267](https://github.com/manticoresoftware/manticoresearch/issues/2267) 改进了 `CALL KEYWORDS` 中 RT 索引与磁盘块的通配符扩展。
* [Issue #271](https://github.com/manticoresoftware/manticoresearch-buddy/issues/271) 修复了不正确的 `/cli` 请求挂起问题。
* [Issue #274](https://github.com/manticoresoftware/manticoresearch-buddy/issues/274) 解决了对 Manticore 的并发请求可能会卡住的问题。
* [Issue #275](https://github.com/manticoresoftware/manticoresearch-buddy/issues/275) 修复了通过 `/cli` 执行 `drop table if exists t; create table t` 时的挂起问题。

### 与复制相关
* [Issue #2270](https://github.com/manticoresoftware/manticoresearch/issues/2270) 在 `/_bulk` HTTP 端点中新增对 `cluster:name` 格式的支持。

# 版本 6.3.0
发布日期：2024年5月23日

### 主要变更
* [Issue #839](https://github.com/manticoresoftware/manticoresearch/issues/839) 实现了 [float_vector](Creating_a_table/Data_types.md#Float-vector) 数据类型；实现了 [vector search](Searching/KNN.md#KNN-vector-search)。
* [Issue #1673](https://github.com/manticoresoftware/manticoresearch/issues/1673) [INNER/LEFT JOIN](Searching/Joining.md) (**测试阶段**)。
* [Issue #1744](https://github.com/manticoresoftware/manticoresearch/issues/1744) 实现了日期格式的自动检测，用于 [timestamp](../Creating_a_table/Data_types.md#Timestamps) 字段。
* [Issue #1720](https://github.com/manticoresoftware/manticoresearch/pull/1720) 将 Manticore Search 的许可证从 GPLv2-or-later 更改为 GPLv3-or-later。
* [Commit 7a55](https://github.com/manticoresoftware/manticoresearch/commit/7a5508424) 在 Windows 上运行 Manticore 现在需要 Docker 来运行 Buddy。
* [Issue #1541](https://github.com/manticoresoftware/manticoresearch/issues/1541) 新增了 [REGEX](Searching/Full_text_matching/Operators.md#REGEX-operator) 全文操作符。
* [Issue #2091](https://github.com/manticoresoftware/manticoresearch/issues/2091) 支持 Ubuntu Noble 24.04。
* [Commit 514d](https://github.com/manticoresoftware/manticoresearch/commit/514d35b497f4bdb20b6473ff963752a978a4bb8d) 针对更好的性能和新的日期/时间函数进行了时间操作的改进：
  - [CURDATE()](Functions/Date_and_time_functions.md#CURDATE%28%29) - 返回本地时区的当前日期
  - [QUARTER()](Functions/Date_and_time_functions.md#QUARTER%28%29) - 返回时间戳参数的整数季度
  - [DAYNAME()](Functions/Date_and_time_functions.md#DAYNAME%28%29) - 返回给定时间戳参数的星期几名称
  - [MONTHNAME()](Functions/Date_and_time_functions.md#MONTHNAME%28%29) - 返回给定时间戳参数的月份名称
  - [DAYOFWEEK()](Functions/Date_and_time_functions.md#DAYOFWEEK%28%29) - 返回给定时间戳参数的整数星期几索引
  - [DAYOFYEAR()](Functions/Date_and_time_functions.md#DAYOFYEAR%28%29) - 返回给定时间戳参数的整数年份中的天数
  - [YEARWEEK()](Functions/Date_and_time_functions.md#YEARWEEK%28%29) - 返回给定时间戳参数的当前周第一天的年份和天数代码
  - [DATEDIFF()](Functions/Date_and_time_functions.md#DATEDIFF%28%29) - 返回两个给定时间戳之间的天数
  - [DATE()](Functions/Date_and_time_functions.md#DATE%28%29) - 格式化给定时间戳参数的日期部分
  - [TIME()](Functions/Date_and_time_functions.md#TIME%28%29) - 格式化给定时间戳参数的时间部分
  - [timezone](Server_settings/Searchd.md#timezone) - 日期/时间相关函数使用的时区。
* [Commit 30e7](https://github.com/manticoresoftware/manticoresearch/commit/30e789d9657f9b3093c280c6e5b0e46a021f6dc7) 向 HTTP 接口和 SQL 中新增了 [range](Searching/Faceted_search.md#Facet-over-set-of-ranges)、[histogram](Searching/Faceted_search.md#Facet-over-histogram-values)、[date_range](Searching/Faceted_search.md#Facet-over-set-of-date-ranges) 和 [date_histogram](Searching/Faceted_search.md#Facet-over-histogram-date-values) 聚合。

### 次要变更
* [Issue #1285](https://github.com/manticoresoftware/manticoresearch/issues/1285) 支持 Filebeat 版本 8.10 - 8.11。
* [Issue #1771](https://github.com/manticoresoftware/manticoresearch/issues/1771) [ALTER TABLE ... type='distributed'](Updating_table_schema_and_settings.md#Changing-a-distributed-table)。
* [Issue #1788](https://github.com/manticoresoftware/manticoresearch/issues/1788) 添加了使用 [CREATE TABLE ... LIKE ... WITH DATA](Creating_a_table/Local_tables/Real-time_table.md#CREATE-TABLE-LIKE:) SQL 语句复制表的功能。
* [Issue #2072](https://github.com/manticoresoftware/manticoresearch/issues/2072) 优化了 [表压缩算法](Securing_and_compacting_a_table/Compacting_a_table.md#Compacting-a-Table)：之前，手动 [OPTIMIZE](Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE) 和自动 [auto_optimize](Server_settings/Searchd.md#auto_optimize) 过程首先会合并块以确保数量不超过限制，然后从所有包含已删除文档的其他块中清除已删除的文档。这种方法有时资源消耗过大，现已被禁用。现在，块合并仅根据 [progressive_merge](Server_settings/Common.md#progressive_merge) 设置进行。然而，包含大量已删除文档的块更可能被合并。
* [Commit ce6c](https://github.com/manticoresoftware/columnar/commit/ce6c5d3381edcdc411a00a5923d13d56dd9b3010) 添加了对加载新版本的二级索引的保护。
* [Issue #1417](https://github.com/manticoresoftware/manticoresearch/issues/1417) 通过 [REPLACE INTO ... SET](Data_creation_and_modification/Updating_documents/REPLACE.md#SQL-REPLACE) 实现部分替换。
* [Commit 7c16](https://github.com/manticoresoftware/manticoresearch/commit/7c167b934ce416d3dfbd658744a231e638b14d9a) 更新了默认合并缓冲区大小：`.spa`（标量属性）：256KB -> 8MB；`.spb`（大块属性）：256KB -> 8MB；`.spc`（列属性）：1MB，未改变；`.spds`（文档存储）：256KB -> 8MB；`.spidx`（二级索引）：256KB 缓冲区 -> 128MB 内存限制；`.spi`（字典）：256KB -> 16MB；`.spd`（文档列表）：8MB，未改变；`.spp`（命中列表）：8MB，未改变；`.spe`（跳表）：256KB -> 8MB。
* [Issue #1859](https://github.com/manticoresoftware/manticoresearch/issues/1859) 通过 JSON 添加了 [复合聚合](Searching/Grouping.md#GROUP-BY-multiple-fields-at-once)。
* [Commit 216b](https://github.com/manticoresoftware/manticoresearch-buddy/commit/216b824) 禁用了 PCRE.JIT，因为某些正则表达式模式存在问题，且没有显著的时间收益。
* [Commit 55cd](https://github.com/manticoresoftware/manticoresearch/commit/55cdce32) 添加了对 vanilla Galera v.3 (api v25) 的支持（`libgalera_smm.so` 来自 MySQL 5.x）。
* [Commit 86f9](https://github.com/manticoresoftware/manticoresearch-buddy/commit/86f98ab) 将指标后缀从 `_rate` 更改为 `_rps`。
* [Commit c0c1](https://github.com/manticoresoftware/manticoresearch-helm/commit/c0c1cff780031412ef57cefab987dfd16613368e) 改进了关于负载均衡器 HA 支持的文档。
* [Commit d1d2](https://github.com/manticoresoftware/manticoresearch/commit/d1d2ca18857c68106e89743756260e73285be739) 将错误消息中的 `index` 更改为 `table`；修复了 bison 解析器错误消息的修正。
* [Commit fd26](https://github.com/manticoresoftware/manticoresearch/commit/fd26671b) 支持将 `manticore.tbl` 作为表名。
* [Issue #1105](https://github.com/manticoresoftware/manticoresearch/issues/1105) 支持通过 systemd 运行索引器（[文档](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Schedule-indexer-via-systemd)）。❤️ 感谢 [@subnix](https://github.com/subnix) 的 PR。
* [Issue #1294](https://github.com/manticoresoftware/manticoresearch/issues/1294) GEODIST() 中的二级索引支持。
* [Issue #1394](https://github.com/manticoresoftware/manticoresearch/issues/1394) 简化了 [SHOW THREADS](Node_info_and_management/SHOW_THREADS.md#SHOW-THREADS)。
* [Issue #1424](https://github.com/manticoresoftware/manticoresearch/issues/1424) 为 `create distributed table` 语句添加了默认值（`agent_connect_timeout` 和 `agent_query_timeout`）的支持。
* [Issue #1442](https://github.com/manticoresoftware/manticoresearch/issues/1442) 添加了 [expansion_limit](Searching/Options.md#expansion_limit) 搜索查询选项，该选项覆盖 `searchd.expansion_limit`。
* [Issue #1448](https://github.com/manticoresoftware/manticoresearch/issues/1448) 实现了 [ALTER TABLE](Updating_table_schema_and_settings.md#Updating-table-schema-in-RT-mode) 用于 int->bigint 转换。
* [Issue #146](https://github.com/manticoresoftware/manticoresearch/issues/1456) MySQL 响应中的元信息。
* [Issue #1494](https://github.com/manticoresoftware/manticoresearch/issues/1494) [SHOW VERSION](Node_info_and_management/SHOW_VERSION.md#SHOW-VERSION)。
* [Issue #1582](https://github.com/manticoresoftware/manticoresearch/issues/1582) 支持通过 JSON [根据 id 数组删除文档](Data_creation_and_modification/Deleting_documents.md?client=JSON#Deleting-documents)。
* [Issue #1589](https://github.com/manticoresoftware/manticoresearch/issues/1589) 改善错误 "不支持的值类型"。
* [Issue #1634](https://github.com/manticoresoftware/manticoresearch/issues/1634) 在 `SHOW STATUS` 中添加了 Buddy 版本。
* [Issue #1641](https://github.com/manticoresoftware/manticoresearch/issues/1641) 在关键字没有文档的情况下优化匹配请求。
* [Issue #1712](https://github.com/manticoresoftware/manticoresearch/issues/1712) 添加了将字符串值 "true" 和 "false" 转换为布尔属性的功能。
* [Issue #1713](https://github.com/manticoresoftware/manticoresearch/issues/1713) 添加了 [access_dict](Server_settings/Searchd.md#access_dict) 表和 searchd 选项。
* [Issue #1767](https://github.com/manticoresoftware/manticoresearch/issues/1767) 添加了新选项：[expansion_merge_threshold_docs](Server_settings/Searchd.md#expansion_merge_threshold_docs) 和 [expansion_merge_threshold_hits](Server_settings/Searchd.md#expansion_merge_threshold_hits)，用于配置文件的 searchd 部分；使得合并扩展术语的小术语的阈值可配置。
* [Issue #1768](https://github.com/manticoresoftware/manticoresearch/issues/1768) 添加了 `@@system.sessions` 中最后命令时间的显示。
* [Issue #1806](https://github.com/manticoresoftware/manticoresearch/issues/1806) 将 Buddy 协议升级到版本 2。
* [Issue #1810](https://github.com/manticoresoftware/manticoresearch/issues/1810) 实现了额外的请求格式到 `/sql` 端点，以便于与库的集成。
* [Issue #1825](https://github.com/manticoresoftware/manticoresearch/pull/1825) 在 SHOW BUDDY PLUGINS 中添加了信息部分。
* [Issue #1837](https://github.com/manticoresoftware/manticoresearch/issues/1837) 改进了 `CALL PQ` 大数据包的内存消耗。
* [Issue #1853](https://github.com/manticoresoftware/manticoresearch/pull/1853) 将编译器从 Clang 15 切换到 Clang 16。
* [Issue #1857](https://github.com/manticoresoftware/manticoresearch/issues/1857) 添加了字符串比较。❤️ 感谢 [@etcd](https://github.com/etcd) 的 PR。
* [Issue #1915](https://github.com/manticoresoftware/manticoresearch/issues/1915) 添加了对已连接存储字段的支持。
* [Issue #1937](https://github.com/manticoresoftware/manticoresearch/issues/1937) 记录客户端的主机：端口在查询日志中。
* [Issue #1981](https://github.com/manticoresoftware/manticoresearch/issues/1981) 修复了错误。
* [Issue #1983](https://github.com/manticoresoftware/manticoresearch/issues/1983) 引入对 [通过 JSON 的详细级别支持](Node_info_and_management/Profiling/Query_plan.md?client=JSON#Query-plan)。
* [Issue #2010](https://github.com/manticoresoftware/manticoresearch/issues/2010) 禁用来自 Buddy 的查询日志，除非设置了 `log_level=debug`。
* [Issue #2035](https://github.com/manticoresoftware/manticoresearch/issues/2035) Linux Mint 21.3 支持。
* [Issue #2056](https://github.com/manticoresoftware/manticoresearch/issues/2056) Manticore 无法与 Mysql 8.3+ 一起构建。
* [Issue #2112](https://github.com/manticoresoftware/manticoresearch/issues/2112) `DEBUG DEDUP` 命令用于实时表块，这些块在附加包含重复项的普通表后可能会出现重复条目。
* [Issue #212](https://github.com/manticoresoftware/manticoresearch-buddy/issues/212) 在 [SHOW QUERIES](Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES) 中添加了时间。
* [Issue #218](https://github.com/manticoresoftware/manticoresearch-buddy/issues/218) 将 `@timestamp` 列处理为时间戳。
* [Issue #252](https://github.com/manticoresoftware/manticoresearch-buddy/issues/250) 实现了启用/禁用 Buddy 插件的逻辑。
* [Issue #254](https://github.com/manticoresoftware/manticoresearch-buddy/issues/254) 将 composer 更新到修复 recent CVE 的更新版本。
* [Issue #340](https://github.com/manticoresoftware/manticoresearch/issues/340) 在与 `RuntimeDirectory` 相关的 Manticore systemd 单元中的小优化。
* [Issue #51](https://github.com/manticoresoftware/executor/issues/51) 添加了 rdkafka 支持并更新到 PHP 8.3.3。
* [Issue #527](https://github.com/manticoresoftware/manticoresearch/issues/527) 支持 [附加](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Attaching_one_table_to_another.md#Attaching-one-table-to-another) 一个 RT 表。新命令 [ALTER TABLE ... RENAME](Updating_table_schema_and_settings.md#Renaming-a-real-time-table)。

### 破坏性变化和弃用
* ⚠️[Issue #1436](https://github.com/manticoresoftware/manticoresearch/issues/1436) 修复了 IDF 计算问题。`local_df` 现在是默认选项。改进了主代理搜索协议（版本已更新）。如果您在多实例的分布式环境中运行 Manticore Search，请确保首先升级代理，然后升级主节点。
* ⚠️[Issue #1572](https://github.com/manticoresoftware/manticoresearch/issues/1572) 添加了分布式表的复制并更新了复制协议。如果您正在运行复制集群，则需要：
  - 首先，干净地停止所有节点
  - 然后，使用 `--new-cluster` 启动最后停止的节点，使用 `manticore_new_cluster` 工具在 Linux 上。
  - 请阅读 [重新启动集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster) 以获取更多详细信息。
* ⚠️[Issue #1763](https://github.com/manticoresoftware/manticoresearch/pull/1763) HTTP API 端点别名 `/json/*` 已被弃用。
* ⚠️[Issue #1982](https://github.com/manticoresoftware/manticoresearch/issues/1982) 将 JSON 中的 [profile](Node_info_and_management/Profiling/Query_profile.md#Query-profile) 改为 [plan](Node_info_and_management/Profiling/Query_plan.md#Query-plan)，并为 JSON 添加了查询分析。
* ⚠️[Commit e235](https://github.com/manticoresoftware/manticoresearch-backup/commit/e23585a) manticore-backup 不再备份 `plugin_dir`。
* ⚠️[Issue #171](https://github.com/manticoresoftware/manticoresearch-buddy/issues/155) 将 Buddy 迁移到 Swoole，以提高性能和稳定性。在切换到新版本时，请确保所有 Manticore 包已更新。
* ⚠️[Issue #196](https://github.com/manticoresoftware/manticoresearch-buddy/pull/205) 将所有核心插件合并到 Buddy 中并更改核心逻辑。
* ⚠️[Issue #2107](https://github.com/manticoresoftware/manticoresearch/issues/2107) 在 `/search` 响应中将文档 ID 视为数字。
* ⚠️[Issue #38](https://github.com/manticoresoftware/executor/issues/38) 添加了 Swoole，禁用 ZTS，并移除了并行扩展。
* ⚠️[Issue #1929](https://github.com/manticoresoftware/manticoresearch/issues/1929) 在某些情况下，`charset_table` 的覆盖不起作用。

### 复制相关的更改
* [Commit 3376](https://github.com/manticoresoftware/manticoresearch/commit/3376351f73db3ff0cb6cb708a304a7bd83e69cfb) 修复了大文件 SST 上的复制错误。
* [Commit 6d36](https://github.com/manticoresoftware/manticoresearch/commit/6d36c68fb7ce3cb274b467aac435788905084952) 添加了一个重试机制到复制命令；修复了在存在丢包的繁忙网络上复制连接失败的问题。
* [Commit 842e](https://github.com/manticoresoftware/manticoresearch/commit/842e27e71790833ab4125d49103d2933d5df60c3) 将复制中的致命消息更改为警告消息。
* [Commit 8c32](https://github.com/manticoresoftware/manticoresearch/commit/8c32bce7db39d8ed3b1a06aaa1d2c3ec372084f0) 修复了没有表或空表的复制集群中 `gcache.page_size` 的计算；同时修复了 Galera 选项的保存和加载。
* [Commit a2af](https://github.com/manticoresoftware/manticoresearch/commit/a2af06ca36abcbf893e43c8803853130b397c54b) 添加了跳过更新节点复制命令的功能，适用于加入集群的节点。
* [Commit c054](https://github.com/manticoresoftware/manticoresearch/commit/c0541e6892f5116b1f957b40060ba8ecd01a5afb) 修复了在更新 blob 属性与替换文档时的死锁问题。
* [Commit e80d](https://github.com/manticoresoftware/manticoresearch/commit/e80d505b963999016033613c40c1cf3151d648e7) 添加了 [replication_connect_timeout](Server_settings/Searchd.md#replication_connect_timeout)、[replication_query_timeout](Server_settings/Searchd.md#replication_query_timeout)、[replication_retry_delay](Server_settings/Searchd.md#replication_retry_delay)、[replication_retry_count](Server_settings/Searchd.md#replication_retry_count) searchd 配置选项，以控制复制过程中的网络，类似于 `searchd.agent_*` 但默认值不同。
* [Issue #1356](https://github.com/manticoresoftware/manticoresearch/issues/1356) 修复了在某些节点缺失且这些节点的名称解析失败后的复制节点重试问题。
* [Issue #1445](https://github.com/manticoresoftware/manticoresearch/issues/1445) 修复了在 `show variables` 中的复制日志详细级别。
* [Issue #1482](https://github.com/manticoresoftware/manticoresearch/issues/1482) 修复了连接到在 Kubernetes 中重启的 pod 的集群的加入节点的复制问题。
* [Issue #1962](https://github.com/manticoresoftware/manticoresearch/issues/1962) 修复了在空集群中等待复制修改的长时间延迟，因无效的节点名称导致。

### Bug fixes
* [Commit 8a48](https://github.com/manticoresoftware/manticoresearch/commit/8a486c7ba1a293879cdecc41fb52eb7ab0cbf832) 修复了在 `count distinct` 中未使用的匹配清理，可能导致崩溃。
* [Issue #1569](https://github.com/manticoresoftware/manticoresearch/issues/1569) 二进制日志现在按事务粒度写入。
* [Issue #2089](https://github.com/manticoresoftware/manticoresearch/issues/2089) 修复了与 64 位 ID 相关的错误，可能导致通过 MySQL 插入时出现“包格式错误”错误，从而导致 [表损坏和重复 ID](https://github.com/manticoresoftware/manticoresearch/issues/2090)。
* [Issue #2160](https://github.com/manticoresoftware/manticoresearch/issues/2160) 修复了日期被插入为 UTC 而非当地时区的问题。
* [Issue #2177](https://github.com/manticoresoftware/manticoresearch/issues/2177) 修复了在实时表中使用非空 `index_token_filter` 进行搜索时发生的崩溃。
* [Issue #2209](https://github.com/manticoresoftware/manticoresearch/issues/2209) 更改了实时列存储中的重复过滤，以修复崩溃和错误的查询结果。
* [Commit 001d](https://github.com/manticoresoftware/manticoresearch/commit/001d82904e4b887c49387308291aeefb07c0fbc4) 修复了处理连接字段后导致内存损坏的 html 剥离器。
* [Commit 00eb](https://github.com/manticoresoftware/manticoresearch/commit/00eb2ebf) 避免在刷新后倒带流以防止与 mysqldump 的通信问题。
* [Commit 0553](https://github.com/manticoresoftware/manticoresearch/commit/05534114acf08c70ffcc41a23d558dec09a34373) 如果预读尚未开始，则不等待预读完成。
* [Commit 055a](https://github.com/manticoresoftware/manticoresearch/commit/055aac09f877570cd29b6594cb283182eba7f73b) 修复了大 Buddy 输出字符串在 searchd 日志中断行的问题。
* [Commit 0a88](https://github.com/manticoresoftware/manticoresearch/commit/0a88056307f82c50fcdc458ad6dbc307a00e3f92) 移动了关于失败的头部 `debugv` 详细级别的 MySQL 接口警告。
* [Commit 150a](https://github.com/manticoresoftware/manticoresearch/commit/150a20286b7d98d24b844ae577d7fda4a444798d) 修复了多个集群管理操作中的竞争条件；禁止以相同名称或路径创建多个集群。
* [Commit 2e40](https://github.com/manticoresoftware/manticoresearch/commit/2e405c1c9ebac6f46206e3df17ac1c19129c9aa3) 修复了全文查询中的隐式截止问题；将 MatchExtended 拆分为模板 partD。
* [Commit 75f5](https://github.com/manticoresoftware/manticoresearch/commit/75f51421542a2a0658f9e2f9f31048c8c5224479) 修复了索引和加载表到守护进程之间的 `index_exact_words` 的不一致性。
* [Commit 7643](https://github.com/manticoresoftware/manticoresearch/commit/7643976bea142e08535d701000eff0d09a99fda2) 修复了无效集群删除的错误消息未被捕获。
* [Commit 7a03](https://github.com/manticoresoftware/manticoresearch/commit/7a034ac990d5ec9344320c05a6a2315583b9b858) 修复了 CBO 与队列联合；修复了 CBO 与 RT 伪分片的问题。
* [Commit 7b4e](https://github.com/manticoresoftware/manticoresearch/commit/7b4ea95e) 当没有次级索引（SI）库和配置中的参数时，发出了误导性的警告信息“警告: 次级索引设置但未能初始化次级库”。
* [Commit 8496](https://github.com/manticoresoftware/manticoresearch/commit/8496317f099fade8d2db463fb88483e7fcdfe85b) 修复了在法定人数下的命中排序。
* [Commit 8973](https://github.com/manticoresoftware/manticoresearch-buddy/commit/8973ad3) 修复了 ModifyTable 插件中大写选项的问题。
* [Commit 9935](https://github.com/manticoresoftware/manticoresearch-buddy/commit/99350d4c184379b19f3efa9751d9b5be5e6edfb5) 修复了从包含空 JSON 值（表示为 NULL）的转储恢复的问题。
* [Commit a28f](https://github.com/manticoresoftware/manticoresearch/commit/a28f6e087b3d957e48816070be17d0823904936a) 修复了在使用 pcon 接收 SST 时，连接节点的 SST 超时问题。
* [Commit b5a5](https://github.com/manticoresoftware/manticoresearch/commit/b5a55cc26debcddcda5d9caef8366a707e0dd763) 修复了选择别名字符串属性时崩溃的问题。
* [Commit c556](https://github.com/manticoresoftware/manticoresearch/commit/c556e7ed8b6da8eac8ef094650df5f0a8977916b) 在 `morphology_skip_fields` 字段中添加了将术语转换为 `=term` 的全文本查询的查询转换。
* [Commit cdc3](https://github.com/manticoresoftware/manticoresearch/commit/cdc38085145cd1025b96fcac2a46188a0425d9e1) 添加了缺失的配置键（skiplist_cache_size）。
* [Commit cf6e](https://github.com/manticoresoftware/manticoresearch/commit/cf6ec3bfdc3f0990fb31ff53822b2331a6667727) 修复了大型复杂查询中表达式排名器崩溃的问题。
* [Commit e513](https://github.com/manticoresoftware/manticoresearch/commit/e5139ba75eef7223df375ae3cc3f138fa7d925f1) 修复了全文本 CBO 与无效索引提示的问题。
* [Commit eb05](https://github.com/manticoresoftware/manticoresearch/commit/eb0514b3a6f57f278252b0724eebd112ec116e0f) 在关闭时中断预读以加快关闭速度。
* [Commit f945](https://github.com/manticoresoftware/manticoresearch/commit/f94555a293cfa51004ff81135b8bd8500b955115) 更改全文本查询的堆栈计算，以避免在复杂查询时崩溃。
* [Issue #1262](https://github.com/manticoresoftware/manticoresearch/issues/1262) 修复了在索引具有相同名称多个列的 SQL 源时，索引器崩溃的问题。
* [Issue #1273](https://github.com/manticoresoftware/manticoresearch/issues/1273) 返回 0 而不是 <empty> 用于不存在的 sysvars。
* [Issue #1289](https://github.com/manticoresoftware/manticoresearch/issues/1289) 修复了检查 RT 表外部文件时的 indextool 错误。
* [Issue #1335](https://github.com/manticoresoftware/manticoresearch/issues/1335) 修复了由于短语中多词形造成的查询解析错误。
* [Issue #1364](https://github.com/manticoresoftware/manticoresearch/issues/1364) 添加了对旧 binlog 版本的空 binlog 文件的重放。
* [Issue #1365](https://github.com/manticoresoftware/manticoresearch/issues/1365) 修复了最后一个空 binlog 文件的删除问题。
* [Issue #1372](https://github.com/manticoresoftware/manticoresearch/issues/1372) 修复了在 `data_dir` 更改影响守护进程启动时当前工作目录后，转换为相对路径（从守护进程的启动目录转换为绝对路径）的问题。
* [Issue #1393](https://github.com/manticoresoftware/manticoresearch/issues/1393) 在 hn_small 中最慢的时间退化：在守护进程启动时获取/缓存 CPU 信息。
* [Issue #1395](https://github.com/manticoresoftware/manticoresearch/issues/1395) 修复了索引加载期间关于缺少外部文件的警告。
* [Issue #1402](https://github.com/manticoresoftware/manticoresearch/issues/1402) 修复了在释放数据指针属性时全局分组者崩溃的问题。
* [Issue #1403](https://github.com/manticoresoftware/manticoresearch/issues/1403) _ADDITIONAL_SEARCHD_PARAMS 无法正常工作。
* [Issue #1427](https://github.com/manticoresoftware/manticoresearch/issues/1427) 修复了按表的 `agent_query_timeout` 被默认查询选项 `agent_query_timeout` 替换的问题。
* [Issue #1444](https://github.com/manticoresoftware/manticoresearch/issues/1444) 修复了在使用 `packedfactors()` 时，分组器和排名器崩溃的问题，尤其是每个匹配多个值。
* [Issue #1458](https://github.com/manticoresoftware/manticoresearch/issues/1458) Manticore 在频繁索引更新时崩溃。
* [Issue #1481](https://github.com/manticoresoftware/manticoresearch/issues/1481) 修复了解析错误后解析查询清理时崩溃的问题。
* [Issue #1484](https://github.com/manticoresoftware/manticoresearch/issues/1484) 修复了 HTTP JSON 请求未路由到 buddy 的问题。
* [Issue #1499](https://github.com/manticoresoftware/manticoresearch/issues/1499) JSON 属性根值不能是数组。修复了。
* [Issue #1507](https://github.com/manticoresoftware/manticoresearch/issues/1507) 修复了在事务中重新创建表时的崩溃问题。
* [Issue #1515](https://github.com/manticoresoftware/manticoresearch/issues/1515) 修复了 RU 词干的缩写形式的扩展。
* [Issue #1579](https://github.com/manticoresoftware/manticoresearch/issues/1579) 修复了在 [date_format](Functions/Date_and_time_functions.md#DATE_FORMAT()) 表达式中使用 JSON 和 STRING 属性的问题。
* [Issue #1580](https://github.com/manticoresoftware/manticoresearch/issues/1580) 修复了多个别名 JSON 字段的分组器。
* [Issue #1594](https://github.com/manticoresoftware/manticoresearch/issues/1594) dev 中的 total_related 错误：修复了隐式截止与限制；在 JSON 查询中添加了更好的全扫描检测。
* [Issue #1603](https://github.com/manticoresoftware/manticoresearch/issues/1603) 修复了在所有日期表达式中 JSON 和 STRING 属性的使用问题。
* [Issue #1609](https://github.com/manticoresoftware/manticoresearch/issues/1609) 使用 LEVENSHTEIN() 时崩溃。
* [Issue #1612](https://github.com/manticoresoftware/manticoresearch/issues/1612) 修复了在搜索查询解析错误后内存损坏的问题。
* [Issue #1614](https://github.com/manticoresoftware/manticoresearch/issues/1614) 禁用短于 `min_prefix_len` / `min_infix_len` 的术语的通配符扩展。
* [Issue #1617](https://github.com/manticoresoftware/manticoresearch/issues/1617) 修改行为，如果 Buddy 成功处理请求则不记录错误。
* [Issue #1635](https://github.com/manticoresoftware/manticoresearch/issues/1635) 修复了设置了限制的搜索查询元数据中的总计。
* [Issue #1640](https://github.com/manticoresoftware/manticoresearch/issues/1640) 不可能在普通模式下通过 JSON 使用大写表。
* [Issue #1643](https://github.com/manticoresoftware/manticoresearch/issues/1643) 提供了默认的 `SPH_EXTNODE_STACK_SIZE` 值。
* [Issue #1646](https://github.com/manticoresoftware/manticoresearch/issues/1646) 修复了 MVA 属性上与 ALL/ANY 的负过滤器的 SphinxQL 日志。
* [Issue #1660](https://github.com/manticoresoftware/manticoresearch/issues/1660) 修复了来自其他索引的 docid 杀死列表的应用。❤️ 感谢 [@raxoft](https://github.com/raxoft) 的 PR。
* [Issue #1668](https://github.com/manticoresoftware/manticoresearch/issues/1668) 修复了由于原始索引全扫描（没有任何索引迭代器）过早退出导致的错过匹配；从普通行迭代器中移除了截止。
* [Issue #1671](https://github.com/manticoresoftware/manticoresearch/issues/1671) 修复了在查询具有代理和本地表的分布式表时的 `FACET` 错误。
* [Issue #1690](https://github.com/manticoresoftware/manticoresearch/issues/1690) 修复了对大值的直方图估计崩溃。
* [Issue #1692](https://github.com/manticoresoftware/manticoresearch/issues/1692) 在修改表 tbl 添加列 col uint 时崩溃。
* [Issue #1710](https://github.com/manticoresoftware/manticoresearch/issues/1710) 对条件 `WHERE json.array IN (<value>)` 的结果为空。
* [Issue #172](https://github.com/manticoresoftware/manticoresearch-buddy/issues/172) 修复了在向 `/cli` 发送请求时与 TableFormatter 相关的问题。
* [Issue #1742](https://github.com/manticoresoftware/manticoresearch/issues/1742) 在缺少 wordforms 文件的情况下，`CREATE TABLE` 不会失败。
* [Issue #1762](https://github.com/manticoresoftware/manticoresearch/issues/1762) RT 表中的属性顺序现在遵循配置顺序。
* [Issue #1765](https://github.com/manticoresoftware/manticoresearch/issues/1765) 使用 'should' 条件的 HTTP 布尔查询返回不正确的结果。
* [Issue #1769](https://github.com/manticoresoftware/manticoresearch/issues/1769) 按字符串属性排序在 `SPH_SORT_ATTR_DESC` 和 `SPH_SORT_ATTR_ASC` 下不起作用。
* [Issue #177](https://github.com/manticoresoftware/manticoresearch-buddy/issues/177) 禁用了对 Buddy 的 curl 请求中的 `Expect: 100-continue` HTTP 头。
* [Issue #1791](https://github.com/manticoresoftware/manticoresearch/issues/1791) 由于 GROUP BY 别名导致崩溃。
* [Issue #1792](https://github.com/manticoresoftware/manticoresearch/issues/1792) SQL 元摘要在 Windows 上显示错误时间。
* [Issue #1794](https://github.com/manticoresoftware/manticoresearch/issues/1794) 修复了 JSON 查询中的单个术语性能下降。
* [Issue #1798](https://github.com/manticoresoftware/manticoresearch/issues/1798) 不兼容的过滤器在 `/search` 上没有引发错误。
* [Issue #1802](https://github.com/manticoresoftware/manticoresearch/issues/1802) 修复了 `ALTER CLUSTER ADD` 和 `JOIN CLUSTER` 操作以等待彼此，防止竞态条件，其中 `ALTER` 在捐赠者发送表到加入者节点时向集群添加表。
* [Issue #1811](https://github.com/manticoresoftware/manticoresearch/issues/1811) 对 `/pq/{table}/*` 请求的处理不正确。
* [Issue #1816](https://github.com/manticoresoftware/manticoresearch/issues/1816) 在某些情况下 `UNFREEZE` 不起作用。
* [Issue #183](https://github.com/manticoresoftware/manticoresearch-buddy/pull/183) 修复了某些情况下 MVA 恢复的问题。
* [Issue #1849](https://github.com/manticoresoftware/manticoresearch/issues/1849) 修复了在与 MCL 一起使用时关闭时的 indextool 崩溃。
* [Issue #1866](https://github.com/manticoresoftware/manticoresearch/issues/1866) 修复了对 `/cli_json` 请求的不必要的 URL 解码。
* [Issue #1872](https://github.com/manticoresoftware/manticoresearch/issues/1872) 更改守护进程启动时的 plugin_dir 设置逻辑。
* [Issue #1874](https://github.com/manticoresoftware/manticoresearch/issues/1874) 修改表 ... 异常失败。
* [Issue #1891](https://github.com/manticoresoftware/manticoresearch/issues/1891#issuecomment-2051941200) 在插入数据时 Manticore 崩溃，出现 `signal 11`。
* [Issue #1920](https://github.com/manticoresoftware/manticoresearch/issues/1920) 减少 [low_priority](Searching/Options.md#low_priority) 的节流。
* [Issue #1924](https://github.com/manticoresoftware/manticoresearch/issues/1924) Mysqldump + mysql 恢复错误。
* [Issue #1951](https://github.com/manticoresoftware/manticoresearch/issues/1951) 修复了在缺少本地表或不正确的代理描述的情况下分布式表的错误创建；现在返回错误信息。
* [Issue #1972](https://github.com/manticoresoftware/manticoresearch/issues/1972) 实现了一个 `FREEZE` 计数器以避免冻结/解冻问题。
* [Issue #1980](https://github.com/manticoresoftware/manticoresearch/issues/1980) 在 OR 节点中遵守查询超时。之前 `max_query_time` 在某些情况下可能不起作用。
* [Issue #1986](https://github.com/manticoresoftware/manticoresearch/issues/1986) 无法将新文件重命名为当前的 [manticore.json]。
* [Issue #1988](https://github.com/manticoresoftware/manticoresearch/issues/1988) 完整文本查询可能会忽略 `SecondaryIndex` CBO 提示。
* [Issue #1990](https://github.com/manticoresoftware/manticoresearch/issues/1990) 修复了 `expansion_limit` 以切片来自多个磁盘块或 RAM 块的调用关键字的最终结果集。
* [Issue #1994](https://github.com/manticoresoftware/manticoresearch/issues/1994) 错误的外部文件。
* [Issue #2021](https://github.com/manticoresoftware/manticoresearch/issues/2021) 停止 Manticore 后，可能会有一些 manticore-executor 进程继续运行。
* [Issue #2029](https://github.com/manticoresoftware/manticoresearch/issues/2029) 使用勒温斯坦距离时崩溃。
* [Issue #2037](https://github.com/manticoresoftware/manticoresearch/issues/2037) 在空索引上运行多个最大操作符后出现错误。
* [Issue #2052](https://github.com/manticoresoftware/manticoresearch/issues/2052) 在与 JSON.field 的多组操作中崩溃。
* [Issue #2067](https://github.com/manticoresoftware/manticoresearch/issues/2067) 在对 _update 的不正确请求时 Manticore 崩溃。
* [Issue #2069](https://github.com/manticoresoftware/manticoresearch/issues/2069) 修复了 JSON 接口中用于封闭范围的字符串过滤器比较器的问题。
* [Issue #2082](https://github.com/manticoresoftware/manticoresearch/issues/2082) 当 data_dir 路径位于符号链接上时，`alter` 失败。
* [Issue #2102](https://github.com/manticoresoftware/manticoresearch/pull/2102) 改进了 mysqldump 中 SELECT 查询的特殊处理，以确保生成的 INSERT 语句与 Manticore 兼容。
* [Issue #2103](https://github.com/manticoresoftware/manticoresearch/issues/2103) 泰国字符在错误的字符集中的问题。
* [Issue #2124](https://github.com/manticoresoftware/manticoresearch/issues/2124) 使用保留字的 SQL 时崩溃。
* [Issue #2154](https://github.com/manticoresoftware/manticoresearch/issues/2154) 具有词形的表无法导入。
* [Issue #2176](https://github.com/manticoresoftware/manticoresearch/issues/2176) 修复了当引擎参数设置为 'columnar' 且通过 JSON 添加重复 ID 时发生的崩溃。
* [Issue #223](https://github.com/manticoresoftware/manticoresearch-buddy/issues/223) 尝试插入没有模式且没有列名的文档时给出适当的错误。
* [Issue #239](https://github.com/manticoresoftware/manticoresearch-buddy/issues/239) 自动模式多行插入可能失败。
* [Issue #399](https://github.com/manticoresoftware/manticoresearch/issues/399) 在索引时添加错误消息，如果在数据源中声明了 id 属性。
* [Issue #59](https://github.com/manticoresoftware/manticoresearch-helm/issues/59) Manticore 集群故障。
* [Issue #68](https://github.com/manticoresoftware/manticoresearch-helm/issues/68) 如果存在渗透表，optimize.php 会崩溃。
* [Issue #77](https://github.com/manticoresoftware/manticoresearch-helm/issues/77) 在 Kubernetes 上部署时修复错误。
* [Issue #274](https://github.com/manticoresoftware/manticoresearch-buddy/issues/274) 修复对 Buddy 的并发请求处理不当的问题。

### 与 manticore-backup 相关
* [Issue #97](https://github.com/manticoresoftware/manticoresearch-backup/pull/97) 设置 VIP HTTP 端口为默认值（如果可用）。
各种改进：改进版本检查和流式 ZSTD 解压缩；在恢复期间添加用户提示以应对版本不匹配；修复恢复时不同版本的错误提示行为；增强解压缩逻辑以直接从流中读取而不是存入工作内存；添加 `--force` 标志。
* [Commit 3b35](https://github.com/manticoresoftware/manticoresearch-backup/commit/3b357ea) 在 Manticore 搜索启动后添加备份版本显示，以识别此阶段的问题。
* [Commit ad2e](https://github.com/manticoresoftware/manticoresearch-backup/commit/ad2e5da) 更新连接到守护进程失败的错误消息。
* [Commit ce5e](https://github.com/manticoresoftware/manticoresearch-backup/commit/ce5ea85) 修复了将绝对根备份路径转换为相对路径的问题，并移除了恢复时的可写检查，以便从不同路径恢复。
* [Commit db7e](https://github.com/manticoresoftware/manticoresearch-backup/commit/db7e2b9) 为文件迭代器添加排序，以确保在各种情况下的一致性。
* [Issue #106](https://github.com/manticoresoftware/manticoresearch-backup/issues/105) 多个配置的备份和恢复。
* [Issue #91](https://github.com/manticoresoftware/manticoresearch-backup/pull/91) 添加 defattr，以防止 RHEL 上安装后文件中出现不寻常的用户权限。
* [Issue #91](https://github.com/manticoresoftware/manticoresearch-backup/pull/91) 添加额外的 chown，以确保在 Ubuntu 中文件默认归根用户所有。

### 与 MCL（列式、二级、knn 库）相关
* [Commit f104](https://github.com/manticoresoftware/columnar/commit/f104a4709bbb144140d09a09bc0fc1db1895e331) 向量搜索支持。
* [Commit 2169](https://github.com/manticoresoftware/columnar/commit/216953beef6676f7e34c9f6f1a30c5980d746710) 修复了在中断的二级索引构建设置过程中临时文件的清理问题。这解决了守护进程在创建 `tmp.spidx` 文件时超过打开文件限制的问题。
* [Commit 709b](https://github.com/manticoresoftware/columnar/commit/709b9acaaac97d9a1ca8796892f9ad432021c785) 为列式和 SI 使用单独的 streamvbyte 库。
* [Commit 1c26](https://github.com/manticoresoftware/manticoresearch/commit/1c2623763ced14bb94ad624dde9614a938408d89) 添加了一个警告，指出列式存储不支持 json 属性。
* [Commit 3acd](https://github.com/manticoresoftware/columnar/commit/3acd6714ffce103447d7931dfc75bb1a7449f340) 修复在 SI 中的数据解包。
* [Commit 574c](https://github.com/manticoresoftware/manticoresearch/commit/574c023152564fa72092c18ebd1f594abfd4b6d7) 修复了在混合行式和列式存储情况下保存磁盘块时发生的崩溃。
* [Commit e87f](https://github.com/manticoresoftware/columnar/commit/e87f088b458d4c5b1b6159fc72ed0d57b786891a) 修复了 SI 迭代器提示已处理区块的问题。
* [Issue #1474](https://github.com/manticoresoftware/manticoresearch/issues/1474) 对于使用列式引擎的行式 MVA 列，更新已损坏。
* [Issue #1510](https://github.com/manticoresoftware/manticoresearch/issues/1510) 修复聚合用于 `HAVING` 的列式属性时发生的崩溃。
* [Issue #1519](https://github.com/manticoresoftware/manticoresearch/issues/1519) 修复使用列属性时在 `expr` 排名器中的崩溃。

### 与 Docker 相关
* ❗[Issue #42](https://github.com/manticoresoftware/docker/issues/42) 通过环境变量支持 [普通索引](https://github.com/manticoresoftware/docker#building-plain-tables)。
* ❗[Issue #47](https://github.com/manticoresoftware/docker/issues/47) 通过环境变量改善配置的灵活性。
* [Issue #54](https://github.com/manticoresoftware/docker/pull/54) 改进 Docker 的 [备份和恢复](https://github.com/manticoresoftware/docker#backup-and-restore) 过程。
* [Issue #77](https://github.com/manticoresoftware/docker/issues/76) 改进入口点以仅在第一次启动时处理备份恢复。
* [Commit a27c](https://github.com/manticoresoftware/docker/commit/a27c048dd9e73d8d6bea8d9a4830cafb486b82c5) 修复查询记录到 stdout。
* [Issue #38](https://github.com/manticoresoftware/docker/issues/38) 如果没有设置 EXTRA，则静默 BUDDY 警告。
* [Issue #71](https://github.com/manticoresoftware/docker/pull/71) 修复 `manticore.conf.sh` 中的主机名。

# 版本 6.2.12
发布日期：2023 年 8 月 23 日

版本 6.2.12 继续 6.2 系列，并解决了 6.2.0 发布后发现的问题。

### 错误修复
* ❗[Issue #1351](https://github.com/manticoresoftware/manticoresearch/issues/1351) "Manticore 6.2.0 在 Centos 7 上无法通过 systemctl 启动"：将 `TimeoutStartSec` 从 `infinity` 修改为 `0` 以提高与 Centos 7 的兼容性。
* ❗[Issue #1364](https://github.com/manticoresoftware/manticoresearch/issues/1364) "从 6.0.4 升级到 6.2.0 后崩溃"：为旧 binlog 版本的空 binlog 文件添加重放功能。
* [PR #1334](https://github.com/manticoresoftware/manticoresearch/pull/1334) "修复 searchdreplication.cpp 中的拼写错误"：更正 `searchdreplication.cpp` 中的拼写错误：beggining -> beginning。
* [Issue #1337](https://github.com/manticoresoftware/manticoresearch/issues/1337) "Manticore 6.2.0 警告：conn (local)(12)，sock=8088：由于 MySQL 头失败而退出，AsyncNetInputBuffer_c::AppendData：错误 11（资源暂时不可用）返回 -1"：降低 MySQL 接口关于头的警告的详细程度至 logdebugv。
* [Issue #1355](https://github.com/manticoresoftware/manticoresearch/issues/1355) "当 node_address 无法解析时，加入集群会挂起"：改善在某些节点无法访问且名称解析失败时的复制重试。这应该解决与 Kubernetes 和 Docker 节点相关的复制问题。增强了复制启动失败的错误消息，并对测试模型 376 进行了更新。此外，为名称解析失败提供了清晰的错误消息。
* [Issue #1361](https://github.com/manticoresoftware/manticoresearch/issues/1361) "字符集 non_cjk 中没有 'Ø' 的小写映射"：调整了 'Ø' 字符的映射。
* [Issue #1365](https://github.com/manticoresoftware/manticoresearch/issues/1365) "searchd 在干净停止后留下 binlog.meta 和 binlog.001"：确保最后一个空的 binlog 文件被正确移除。
* [Commit 0871](https://github.com/manticoresoftware/manticoresearch/commit/0871070845e93ad702ae0dfb89b35e5f24cd0851): 修复与原子复制限制相关的 Windows 上的 `Thd_t` 构建问题。
* [Commit 1cc0](https://github.com/manticoresoftware/manticoresearch/commit/1cc04bedc174737118fd5a3663e35f4e95bd4f8c): 解决 FT CBO 与 `ColumnarScan` 的问题。
* [Commit c6bf](https://github.com/manticoresoftware/manticoresearch/commit/c6bfb30d53e3f4b67f17769667733677a661c95b): 对测试 376 进行了更正，并在测试中添加了对 `AF_INET` 错误的替代。
* [Commit cbc3](https://github.com/manticoresoftware/manticoresearch/commit/cbc38705e0b4dcc9665e3e8cf64826290ebd11cf): 解决在更新 blob 属性与替换文档时的复制过程中的死锁问题。还在提交期间移除了索引的 rlock，因为它在更基本的级别上已经被锁定。

### 次要更改
* [Commit 4f91](https://github.com/manticoresoftware/manticoresearch/commit/4f913a36e84e01f950b2797051222b0b1094d6fe) 更新手册中有关 `/bulk` 端点的信息。

### MCL
* 支持 [Manticore Columnar Library](https://github.com/manticoresoftware/columnar) v2.2.4

# 版本 6.2.0
发布日期：2023 年 8 月 4 日

### 主要更改
* 查询优化器已增强以支持全文查询，大幅提高搜索效率和性能。
* 集成：
  - [mysqldump](https://dev.mysql.com/doc/refman/8.0/en/mysqldump.html) - 使用 `mysqldump` [进行逻辑备份](../Securing_and_compacting_a_table/Backup_and_restore.md#Backup-and-restore-with-mysqldump)
  - [Apache Superset](https://manticoresearch.com/blog/manticoresearch-apache-superset-integration/) 和 [Grafana](https://manticoresearch.com/blog/manticoresearch-grafana-integration/) 可视化存储在 Manticore 中的数据
  - [HeidiSQL](https://www.heidisql.com/) 和 [DBForge](https://www.devart.com/dbforge/) 使与 Manticore 的开发变得更加容易
* 我们开始使用 [GitHub workflows](https://github.com/manticoresoftware/manticoresearch/actions)，使贡献者能够更简单地利用核心团队在准备软件包时应用的相同持续集成（CI）流程。所有作业都可以在 GitHub 托管的运行器上运行，这便于在您自己的 Manticore Search 分支中无缝测试更改。
* 我们开始使用 [CLT](https://github.com/manticoresoftware/clt) 来测试复杂场景。例如，我们现在能够确保在提交后构建的包可以在所有支持的 Linux 操作系统上正确安装。命令行测试器（CLT）提供了一种用户友好的方式以交互模式记录测试并轻松重放它们。
* 通过采用哈希表和HyperLogLog的组合，在计数唯一操作中显著提高了性能。
* 使包含辅助索引的查询能够多线程执行，线程数限制为物理CPU核心数。这应该会显著提高查询执行速度。
* `pseudo_sharding` 已调整为限制为可用线程数。此更新显著提升了吞吐量性能。
* 用户现在可以通过配置设置指定 [默认属性存储引擎](../Server_settings/Searchd.md#engine)，提供更好的自定义以匹配特定工作负载要求。
* 支持 [Manticore Columnar Library 2.2.0](https://github.com/manticoresoftware/columnar/)，并在 [辅助索引](../Server_settings/Searchd.md#secondary_indexes) 中进行多项错误修复和改进。

### 小改动
* [Buddy #153](https://github.com/manticoresoftware/manticoresearch-buddy/issues/153)：[/pq](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md?client=JSON#Adding-rules-to-a-percolate-table) HTTP 端点现在作为 `/json/pq` HTTP 端点的别名。
* [Commit 0bf1](https://github.com/manticoresoftware/manticoresearch/commit/0bf17d9e)：我们确保了`upper()`和`lower()`的多字节兼容性。
* [Commit 2bb9](https://github.com/manticoresoftware/manticoresearch/commit/2bb92765)：现在返回预计算值，而不是扫描索引以进行 `count(*)` 查询。
* [Commit 3c84](https://github.com/manticoresoftware/manticoresearch/commit/3c84091f)：现在可以使用 `SELECT` 进行任意计算并显示 `@@sysvars`。与之前不同的是，您不再仅限于一次计算。因此，像 `select user(), database(), @@version_comment, version(), 1+1 as a limit 10` 的查询将返回所有列。请注意，可选的 'limit' 将始终被忽略。
* [Commit 6aca](https://github.com/manticoresoftware/manticoresearch/commit/6aca32f0)：实现了 `CREATE DATABASE` 存根查询。
* [Commit 9dc1](https://github.com/manticoresoftware/manticoresearch/commit/9dc12334)：执行 `ALTER TABLE table REBUILD SECONDARY` 时，即使属性未更新，辅助索引现在也始终会被重建。
* [Commit 46ed](https://github.com/manticoresoftware/manticoresearch/commit/46edb089)：在使用CBO之前，现在可以识别利用预计算数据的排序器，以避免不必要的CBO计算。
* [Commit 102a](https://github.com/manticoresoftware/manticoresearch/commit/102ac604)：实现了模拟和利用全文表达式堆栈，以防止守护进程崩溃。
* [Commit 979f](https://github.com/manticoresoftware/manticoresearch/commit/979fa27c)：为不使用字符串/mvas/json 属性的匹配添加了一个快速代码路径的匹配克隆代码。
* [Commit a073](https://github.com/manticoresoftware/manticoresearch/commit/a0735ffe)：添加了对 `SELECT DATABASE()` 命令的支持。但它将始终返回 `Manticore`。此添加对与各种MySQL工具的集成至关重要。
* [Commit bc04](https://github.com/manticoresoftware/manticoresearch/commit/bc04908)：修改了 [/cli](../Connecting_to_the_server/HTTP.md#/cli) 端点的响应格式，并添加了 `/cli_json` 端点，以作为之前的 `/cli`。
* [Commit d70b](https://github.com/manticoresoftware/manticoresearch/commit/d70b0d58)：现在可以在运行时使用 `SET` 语句修改 `thread_stack`。提供会话本地和守护进程广泛的变体。当前值可以在 `show variables` 输出中访问。
* [Commit d96e](https://github.com/manticoresoftware/manticoresearch/commit/d96ec6b9)：代码已集成到CBO中，以更准确地估算在字符串属性上执行过滤器的复杂性。
* [Commit e77d](https://github.com/manticoresoftware/manticoresearch/commit/e77dd72f5a04531c352fad0d7afcd2a1cbae2510)：DocidIndex成本计算已改进，提高了整体性能。
* [Commit f3ae](https://github.com/manticoresoftware/manticoresearch/commit/f3ae8bea)：负载指标（类似于Linux上的 'uptime'）现在在 `SHOW STATUS` 命令中可见。
* [Commit f3cc](https://github.com/manticoresoftware/manticoresearch/commit/f3cc0971)：`DESC` 和 `SHOW CREATE TABLE` 的字段和属性顺序现在与 `SELECT * FROM` 相匹配。
* [Commit f3d2](https://github.com/manticoresoftware/manticoresearch/commit/f3d248a6)：不同的内部解析器现在在各种错误中提供它们的内部助记码（例如，`P01`）。此增强帮助识别哪个解析器导致了错误，并且还隐藏了不必要的内部细节。
* [Issue #271](https://github.com/manticoresoftware/manticoresearch/issues/271) "有时 CALL SUGGEST 不会建议单个字母拼写错误的更正"：改善了短单词的 [SUGGEST/QSUGGEST](../Searching/Spell_correction.md#CALL-QSUGGEST,-CALL-SUGGEST) 行为：添加了 `sentence` 选项以显示完整句子。
* [Issue #696](https://github.com/manticoresoftware/manticoresearch/issues/696) "当启用词干时，渗透索引未正确按确切短语查询搜索"：已修改渗透查询以处理确切术语修饰符，从而改善搜索功能。
* [Issue #829](https://github.com/manticoresoftware/manticoresearch/issues/829) "日期格式化方法"：添加了 [date_format()](../Functions/Date_and_time_functions.md#DATE_FORMAT()) 选择列表表达式，该表达式暴露了 `strftime()` 函数。
* [Issue #961](https://github.com/manticoresoftware/manticoresearch/issues/961) "通过 HTTP JSON API 排序桶"：为 HTTP 接口中的每个聚合桶引入了可选的 [sort property](../Searching/Faceted_search.md#HTTP-JSON)。
* [Issue #1062](https://github.com/manticoresoftware/manticoresearch/issues/1062) "改善 JSON 插入 API 失败的错误日志 - "不支持的值类型"": `/bulk` 端点在发生错误的情况下报告处理和未处理字符串（文档）的数量信息。
* [Issue #1070](https://github.com/manticoresoftware/manticoresearch/issues/1070) "CBO 提示不支持多个属性": 启用索引提示以处理多个属性。
* [Issue #1106](https://github.com/manticoresoftware/manticoresearch/issues/1106) "在 http 搜索查询中添加标签": 标签已添加到 [HTTP PQ 响应](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md?client=JSON#Adding-rules-to-a-percolate-table)。
* [Issue #1301](https://github.com/manticoresoftware/manticoresearch/issues/1301) "buddy 不应并行创建表": 解决了导致并行 CREATE TABLE 操作失败的问题。现在，只有一个 `CREATE TABLE` 操作可以同时运行。
* [Issue #1303](https://github.com/manticoresoftware/manticoresearch/issues/1303) "添加对 @ 的支持以用于列名"。
* [Issue #1316](https://github.com/manticoresoftware/manticoresearch/issues/1316) "出租车数据集的查询在 ps=1 时很慢": CBO 逻辑已被精炼，默认直方图分辨率已设置为 8k，以提高随机分布值属性的准确性。
* [Issue #1317](https://github.com/manticoresoftware/manticoresearch/issues/1317) "修复 h 数据集上的 CBO 与全文搜索": 实施了增强逻辑以确定何时使用位图迭代器交集以及何时使用优先级队列。
* [Issue #1318](https://github.com/manticoresoftware/manticoresearch/issues/1318) "columnar: 将迭代器接口更改为单次调用": 列式迭代器现在使用单个 `Get` 调用，替换了之前的两步 `AdvanceTo` + `Get` 调用以检索值。
* [Issue #1319](https://github.com/manticoresoftware/manticoresearch/issues/1319) "聚合计算加速（移除 CheckReplaceEntry?）": 从组排序器中移除了 `CheckReplaceEntry` 调用，以加快聚合函数的计算。
* [Issue #1320](https://github.com/manticoresoftware/manticoresearch/issues/1320) "创建表 read_buffer_docs/hits 不理解 k/m/g 语法": `CREATE TABLE` 选项 `read_buffer_docs` 和 `read_buffer_hits` 现在支持 k/m/g 语法。
* [语言包](../Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) 现在可以通过执行命令 `apt/yum install manticore-language-packs` 在 Linux 上轻松安装。在 macOS 上，使用命令 `brew install manticoresoftware/tap/manticore-language-packs`。
* 字段和属性的顺序现在在 `SHOW CREATE TABLE` 和 `DESC` 操作之间一致。
* 如果在执行 `INSERT` 查询时磁盘空间不足，则新的 `INSERT` 查询将在足够的磁盘空间可用之前失败。
* [UINT64()](../Functions/Type_casting_functions.md#UINT64%28%29) 类型转换函数已添加。
* `/bulk` 端点现在将空行视为 [提交](../Data_creation_and_modification/Transactions.md#BEGIN,-COMMIT,-and-ROLLBACK) 命令。更多信息见 [这里](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md?client=JSON#Bulk-adding-documents)。
* 已实施关于 [无效索引提示](../Searching/Options.md#Query-optimizer-hints) 的警告，提供更多透明度并允许进行错误缓解。
* 当单个过滤器与 `count(*)` 一起使用时，查询现在在可用时利用来自二级索引的预计算数据，显著加快查询时间。

### ⚠️ 重大变更
* ⚠️ 在版本 6.2.0 中创建或修改的表无法被旧版本读取。
* ⚠️ 文档 ID 现在在索引和 INSERT 操作中被视为无符号的 64 位整数。
* ⚠️ 查询优化器提示的语法已更新。新格式为 `/*+ SecondaryIndex(uid) */`。请注意，旧语法不再受支持。
* ⚠️ [Issue #1160](https://github.com/manticoresoftware/manticoresearch/issues/1160): 表名中不允许使用 `@` 以防止语法冲突。
* ⚠️ 标记为 `indexed` 和 `attribute` 的字符串字段/属性现在在 `INSERT`、`DESC` 和 `ALTER` 操作期间被视为单个字段。
* ⚠️ [Issue #1057](https://github.com/manticoresoftware/manticoresearch/issues/1057): 在不支持 SSE 4.2 的系统上，MCL 库将不再加载。
* ⚠️ [Issue #1143](https://github.com/manticoresoftware/manticoresearch/issues/1143): [agent_query_timeout](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_query_timeout) 已被破坏。已修复并现在生效。

### 错误修复
* [Commit 2a6e](https://github.com/manticoresoftware/manticoresearch/commit/2a6ea8f7) "DROP TABLE 时崩溃": 解决了导致在执行 DROP TABLE 语句时延迟完成写入操作（优化、磁盘块保存）的问题。添加了一条警告以在执行 DROP TABLE 命令后通知表目录不为空。
* [Commit 2ebd](https://github.com/manticoresoftware/manticoresearch/commit/2ebd424d): 添加了对缺少的列式属性的支持，该属性在用于按多个属性分组的代码中缺失。
* [Commit 3be4](https://github.com/manticoresoftware/manticoresearch/commit/3be4503f752216b37b8cde8e47fd2d77bbce63ae) 解决了一个可能因磁盘空间耗尽而导致的崩溃问题，通过正确处理 binlog 中的写入错误。
* [Commit 6adb](https://github.com/manticoresoftware/manticoresearch/commit/6adb0934): 修复了在查询中使用多个列式扫描迭代器（或二级索引迭代器）时偶尔发生的崩溃。
* [Commit 6bd9](https://github.com/manticoresoftware/manticoresearch/commit/6bd9f709): 使用基于预计算数据的排序器时，过滤器未被移除。此问题已得到解决。
* [Commit 6d03](https://github.com/manticoresoftware/manticoresearch/commit/6d03566a): CBO 代码已更新，以提供更好的估计，对于在多个线程中执行的使用过滤器的基于行属性的查询。
* [Commit 6dd3](https://github.com/manticoresoftware/manticoresearch/commit/6dd3964), [Helm #56](https://github.com/manticoresoftware/manticoresearch-helm/issues/56) "Kubernetes 集群中的致命崩溃转储": 修复了 JSON 根对象的缺陷布隆过滤器；修复了由于按 JSON 字段过滤而导致的守护进程崩溃。
* [Commit 6e1b](https://github.com/manticoresoftware/manticoresearch/commit/6e1b849) 修正了由于无效的 `manticore.json` 配置导致的守护进程崩溃。
* [Commit 6fbc](https://github.com/manticoresoftware/manticoresearch/commit/6fbc4189) 修复了 json 范围过滤器以支持 int64 值。
* [Commit 9c67](https://github.com/manticoresoftware/manticoresearch/commit/9c677426) `.sph` 文件可能在 `ALTER` 过程中损坏。已修复。
* [Commit 77cc](https://github.com/manticoresoftware/manticoresearch/commit/77ccd8f): 为替换语句的复制添加了共享密钥，以解决在从多个主节点复制时出现的 `pre_commit` 错误。
* [Commit 2884](https://github.com/manticoresoftware/manticoresearch/commit/2884519e) 解决了对函数如 'date_format()' 的 bigint 检查存在的问题。
* [Commit 9513](https://github.com/manticoresoftware/manticoresearch/commit/9513aca5): 当排序器使用预计算数据时，[SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META) 中将不再显示迭代器。
* [Commit a2a7](https://github.com/manticoresoftware/manticoresearch/commit/a2a70555): 全文节点堆栈大小已更新，以防止在复杂全文查询时崩溃。
* [Commit a062](https://github.com/manticoresoftware/manticoresearch/commit/a0626d7e): 修复了在使用 JSON 和字符串属性进行更新复制过程中导致崩溃的错误。
* [Commit b3e6](https://github.com/manticoresoftware/manticoresearch/commit/b3e63e5d): 字符串构建器已更新为使用 64 位整数，以避免处理大型数据集时崩溃。
* [Commit c472](https://github.com/manticoresoftware/manticoresearch/commit/c472e5b): 解决了在多个索引中发生的计数distinct时的崩溃。
* [Commit d073](https://github.com/manticoresoftware/manticoresearch/commit/d0730272): 修复了一个问题，即即使禁用 `pseudo_sharding`，对 RT 索引的磁盘块的查询也可以在多个线程中执行。
* [Commit d205](https://github.com/manticoresoftware/manticoresearch/commit/d205508e) `show index status` 命令返回的值集已被修改，现根据使用的索引类型而变化。
* [Commit e9bc](https://github.com/manticoresoftware/manticoresearch/commit/e9bc2f9) 修复了处理批量请求时的 HTTP 错误，以及在网络循环中未将错误返回给客户端的问题。
* [Commit f77c](https://github.com/manticoresoftware/manticoresearch/commit/f77ce0e6) 为 PQ 使用扩展堆栈。
* [Commit fac2](https://github.com/manticoresoftware/manticoresearch/commit/fac2d9687a20582765aaa89be9597804c4f845af) 更新了导出排名输出，以与 [packedfactors()](../Functions/Searching_and_ranking_functions.md#PACKEDFACTORS%28%29) 对齐。
* [Commit ff87](https://github.com/manticoresoftware/manticoresearch/commit/ff87cc4): 修复了 SphinxQL 查询日志中过滤器中的字符串列表问题。
* [Issue #589](https://github.com/manticoresoftware/manticoresearch/issues/589) "字符集定义似乎依赖于代码顺序": 修复了重复项的字符集映射错误。
* [Issue #811](https://github.com/manticoresoftware/manticoresearch/issues/811) "映射多个词的词形干扰CJK标点符号之间的短语搜索": 修复了词形短语查询中的 ngram 令牌位置。
* [Issue #834](https://github.com/manticoresoftware/manticoresearch/issues/834) "搜索查询中的等号破坏请求": 确保确切符号可以被转义，并修复 `expand_keywords` 选项引起的双重精确扩展。
* [Issue #864](https://github.com/manticoresoftware/manticoresearch/issues/864) "exceptions/stopwords 冲突"
* [Issue #910](https://github.com/manticoresoftware/manticoresearch/issues/910) "调用 snippets() 时 Manticore 崩溃，使用 libstemmer_fr 和 index_exact_words": 解决了调用 `SNIPPETS()` 时导致崩溃的内部冲突。
* [Issue #946](https://github.com/manticoresoftware/manticoresearch/issues/946) "SELECT 中的重复记录": 修复了在具有 `not_terms_only_allowed` 选项的查询中，针对被杀死的文档的结果集中出现重复文档的问题。
* [Issue #967](https://github.com/manticoresoftware/manticoresearch/issues/967) "在 UDF 函数中使用 JSON 参数导致崩溃": 修复了在启用伪分片和使用 JSON 参数的 UDF 时处理搜索导致的守护进程崩溃。
* [Issue #1050](https://github.com/manticoresoftware/manticoresearch/issues/1050) "FEDERATED 中的 count(*)": 修复了通过具有聚合的 `FEDERATED` 引擎查询时发生的守护进程崩溃。
* [Issue #1052](https://github.com/manticoresoftware/manticoresearch/issues/1052) 修复了 `rt_attr_json` 列与列存储不兼容的问题。
* [Issue #1072](https://github.com/manticoresoftware/manticoresearch/issues/1072) "* 被 ignore_chars 从搜索查询中移除": 修复了这个问题，使查询中的通配符不受 `ignore_chars` 的影响。
* [Issue #1075](https://github.com/manticoresoftware/manticoresearch/issues/1075) "indextool --check fails if there's a distributed table": indextool 现在与 json 配置中具有 'distributed' 和 'template' 索引的实例兼容。
* [Issue #1081](https://github.com/manticoresoftware/manticoresearch/issues/1081) "particular select on particular RT dataset leads to crash of searchd": 解决了对带有 packedfactors 和大内存缓冲区的查询导致的守护进程崩溃问题。
* [Issue #1095](https://github.com/manticoresoftware/manticoresearch/issues/1095) "With not_terms_only_allowed deleted documents are ignored"
* [Issue #1099](https://github.com/manticoresoftware/manticoresearch/issues/1099) "indextool --dumpdocids is not working": 恢复了 `--dumpdocids` 命令的功能。
* [Issue #1100](https://github.com/manticoresoftware/manticoresearch/issues/1100) "indextool --buildidf is not working": indextool 现在在完成 globalidf 后关闭文件。
* [Issue #1104](https://github.com/manticoresoftware/manticoresearch/issues/1104) "Count(*) is trying to be treated as schema set in remote tables": 解决了在代理返回空结果集时，守护进程向分布式索引查询发送错误消息的问题。
* [Issue #1109](https://github.com/manticoresoftware/manticoresearch/issues/1109) "FLUSH ATTRIBUTES hangs with threads=1"。
* [Issue #1126](https://github.com/manticoresoftware/manticoresearch/issues/1126) "Lost connection to MySQL server during query - manticore 6.0.5": 解决了在列属性上使用多个过滤器时发生的崩溃问题。
* [Issue #1135](https://github.com/manticoresoftware/manticoresearch/issues/1135) "JSON string filtering case sensitivity": 修正了用于 HTTP 搜索请求的过滤器的排序规则，以确保其正确工作。
* [Issue #1140](https://github.com/manticoresoftware/manticoresearch/issues/1140) "Match in a wrong field": 修复了与 `morphology_skip_fields` 相关的损害。
* [Issue #1155](https://github.com/manticoresoftware/manticoresearch/issues/1155) "system remote commands via API should pass g_iMaxPacketSize": 更新了以绕过节点之间复制命令的 `max_packet_size` 检查。此外，最新的集群错误已添加到状态显示中。
* [Issue #1302](https://github.com/manticoresoftware/manticoresearch/issues/1302) "tmp files left on failed optimize": 修正了在合并或优化过程中发生错误后，临时文件被遗留的问题。
* [Issue #1304](https://github.com/manticoresoftware/manticoresearch/issues/1304) "add env var for buddy start timeout": 添加了环境变量 `MANTICORE_BUDDY_TIMEOUT`（默认 3 秒），以控制守护进程在启动时等待好友消息的持续时间。
* [Issue #1305](https://github.com/manticoresoftware/manticoresearch/issues/1305) "Int overflow when saving PQ meta": 减轻了守护进程在保存大型 PQ 索引时的过度内存消耗。
* [Issue #1306](https://github.com/manticoresoftware/manticoresearch/issues/1306) "Can't recreate RT table after altering its external file": 修正了外部文件为空字符串的更改错误；修复了在更改外部文件后留下的 RT 索引外部文件。
* [Issue #1307](https://github.com/manticoresoftware/manticoresearch/issues/1307) "SELECT statement sum(value) as value doesn't work properly": 修复了带别名的选择列表表达式可能隐藏索引属性的问题；还修复了将 sum 更改为 int64 的整数统计。
* [Issue #1308](https://github.com/manticoresoftware/manticoresearch/issues/1308) "Avoid binding to localhost in replication": 确保复制不在具有多个 IP 的主机名上绑定到 localhost。
* [Issue #1309](https://github.com/manticoresoftware/manticoresearch/issues/1309) "reply to mysql client failed for data larger 16Mb": 修复了向客户端返回大于 16Mb 的 SphinxQL 数据包的问题。
* [Issue #1310](https://github.com/manticoresoftware/manticoresearch/issues/1310) "wrong reference in "paths to external files should be absolute": 修正了在 `SHOW CREATE TABLE` 中显示外部文件完整路径的问题。
* [Issue #1311](https://github.com/manticoresoftware/manticoresearch/issues/1311) "debug build crashes on long strings in snippets": 现在，允许在 `SNIPPET()` 函数目标文本中使用超过 255 个字符的长字符串。
* [Issue #1312](https://github.com/manticoresoftware/manticoresearch/issues/1312) "spurious crash on use-after-delete in kqueue polling (master-agent)": 修复了在 kqueue 驱动的系统（FreeBSD、MacOS 等）上，主服务器无法连接到代理时发生的崩溃问题。
* [Issue #1313](https://github.com/manticoresoftware/manticoresearch/issues/1313) "too long connect to itself": 从主服务器连接到 MacOS/BSD 上的代理时，现在使用统一的连接+查询超时，而不仅仅是连接超时。
* [Issue #1314](https://github.com/manticoresoftware/manticoresearch/issues/1314) "pq (json meta) with unreached embedded synonyms fails to load": 修复了 pq 中的嵌入同义词标志。
* [Issue #1315](https://github.com/manticoresoftware/manticoresearch/issues/1315) "Allow some functions (sint, fibonacci, second, minute, hour, day, month, year, yearmonth, yearmonthday) to use implicitly promoted argument values"。
* [Issue #1321](https://github.com/manticoresoftware/manticoresearch/issues/1321) "Enable multithreaded SI in fullscan, but limit threads": 代码已被实现到 CBO，以更好地预测在全文查询中使用二级索引时的多线程性能。
* [Issue #1322](https://github.com/manticoresoftware/manticoresearch/issues/1322) "count(*) queries still slow after using precalc sorters": 在使用使用预计算数据的排序器时，不再启动迭代器，以避免对性能的有害影响。
* [Issue #1411](https://github.com/manticoresoftware/manticoresearch/issues/1141) "query log in sphinxql does not preserve original queries for MVA's": 现在，`all()/any()`被记录。

# 版本 6.0.4
发布日期：2023年3月15日

### 新特性
* 改进与 Logstash、Beats 等的集成，包括：
  - 支持 Logstash 版本 7.6 - 7.15，Filebeat 版本 7.7 - 7.12  
  - 自动模式支持。
  - 增加对 Elasticsearch 类格式的批量请求处理。
* [Buddy commit ce90](https://github.com/manticoresoftware/manticoresearch-buddy/commit/ce907ea) 在 Manticore 启动时记录 Buddy 版本。

### Bug修复
* [Issue #588](https://github.com/manticoresoftware/manticoresearch/issues/588)， [Issue #942](https://github.com/manticoresoftware/manticoresearch/issues/942) 修复了大字典索引的搜索元数据和调用关键字中的坏字符。
* [Issue #1027](https://github.com/manticoresoftware/manticoresearch/issues/1027) 小写 HTTP 头被拒绝。
* ❗[Issue #1039](https://github.com/manticoresoftware/manticoresearch/issues/1039) 修复了在读取 Buddy 控制台输出时守护进程的内存泄漏。
* [Issue #1056](https://github.com/manticoresoftware/manticoresearch/issues/1056) 修复了问号的意外行为。
* [Issue #1064](https://github.com/manticoresoftware/manticoresearch/issues/1064) - 修复了分词器小写表中的竞争条件导致崩溃。
* [Commit 59bb](https://github.com/manticoresoftware/manticoresearch/commit/59bb54c) 修复了 JSON 接口中对于 id 显式设为 null 的文档的批量写入处理。
* [Commit 7b6b](https://github.com/manticoresoftware/manticoresearch/commit/7b6b25f) 修复了 CALL KEYWORDS 中对于多个相同术语的术语统计。
* [Commit f381](https://github.com/manticoresoftware/manticoresearch/commit/f381ad2) 默认配置现在由 Windows 安装程序创建；路径不再在运行时替换。
* [Commit 6940](https://github.com/manticoresoftware/manticoresearch/commit/6940e95)， [Commit cc5a](https://github.com/manticoresoftware/manticoresearch/commit/cc5a480) 修复了具有多个网络节点的集群的复制问题。
* [Commit 4972](https://github.com/manticoresoftware/manticoresearch/commit/49722ab) 修复了 `/pq` HTTP 端点为 `/json/pq` HTTP 端点的别名。
* [Commit 3b53](https://github.com/manticoresoftware/manticoresearch/commit/3b5385a) 修复了 Buddy 重启时守护进程崩溃。
* [Buddy commit fba9](https://github.com/manticoresoftware/manticoresearch-buddy/commit/fba9c8c) 在收到无效请求时显示原始错误。
* [Buddy commit db95](https://github.com/manticoresoftware/manticoresearch-buddy/commit/db9532c) 允许备份路径中有空格，并对正则表达式添加一些魔法以支持单引号。

# 版本 6.0.2
发布日期：2023年2月10日

### Bug修复
* [Issue #1024 crash 2](https://github.com/manticoresoftware/manticoresearch/issues/1024) 在具有较多结果的 Facet 搜索中崩溃 / 段错误
* ❗[Issue #1029](https://github.com/manticoresoftware/manticoresearch/issues/1029) - 警告：编译值 KNOWN_CREATE_SIZE (16) 小于测量值 (208)。请考虑修复该值！
* ❗[Issue #1032](https://github.com/manticoresoftware/manticoresearch/issues/1032) - Manticore 6.0.0 普通索引崩溃
* ❗[Issue #1033](https://github.com/manticoresoftware/manticoresearch/issues/1033) - 守护进程重启时丢失多个分布式项
* ❗[Issue #1064](https://github.com/manticoresoftware/manticoresearch/issues/1064) - 分词器小写表中的竞争条件

# 版本 6.0.0
发布日期：2023年2月7日

从此版本开始，Manticore Search 提供 Manticore Buddy，一个用 PHP 编写的侧车守护进程，处理不需要超低延迟或高吞吐量的高阶功能。Manticore Buddy 在后台运行，您甚至可能没有意识到它正在运行。尽管它对于最终用户是不可见的，但使 Manticore Buddy 易于安装并与主 C++ 基础的守护进程兼容是一个重大挑战。这个重大变化将使团队能够开发广泛的新高阶功能，如分片编排、访问控制和认证，以及 mysqldump、DBeaver、Grafana MySQL 连接器等各种集成。现在它已经能够处理 [SHOW QUERIES](../Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES)， [BACKUP](../Securing_and_compacting_a_table/Backup_and_restore.md#BACKUP-SQL-command-reference) 和 [自动模式](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema)。

此版本还包括 130 多个 bug 修复和众多新特性，其中许多可以视为重大更新。

### 重大变化
* 🔬 实验性：您现在可以执行与 Elasticsearch 兼容的 [插入](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real_time_table.md?example=Elasticsearch#Adding-documents-to-a-real-time-table) 和 [替换](Data_creation_and_modification/Updating_documents/REPLACE.md?example=Elasticsearch#REPLACE) JSON 查询，这使得使用 Manticore 与 Logstash（版本< 7.13）、Filebeat 及其他 Beats 家族的工具结合使用成为可能。默认启用。您可以使用 `SET GLOBAL ES_COMPAT=off` 禁用它。
* 支持 [Manticore 列库 2.0.0](https://github.com/manticoresoftware/columnar/)，在 [二级索引](../Server_settings/Searchd.md#secondary_indexes) 中进行了众多修复和改进。**⚠️ 重大变更**：截至此版本，二级索引默认开启。如果您升级自 Manticore 5，请确保执行 [ALTER TABLE table_name REBUILD SECONDARY](../Updating_table_schema_and_settings.md#Rebuilding-a-secondary-index)。如下所示是更多细节。
* [Commit c436](https://github.com/manticoresoftware/manticoresearch/commit/c436f9023536f767610451911955ae36d90aa638) 自动模式：您现在可以跳过创建表，只需插入第一个文档，Manticore 将根据其字段自动创建表。详细信息请参见 [这里](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema)。您可以使用 [searchd.auto_schema](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema) 来打开/关闭此功能。
* 对 [基于成本的优化器](../Searching/Cost_based_optimizer.md) 进行了大幅改进，可在许多情况下降低查询响应时间。
  - [Issue #1008](https://github.com/manticoresoftware/manticoresearch/issues/1008) CBO 中的并行化性能估计。
  - [Issue #1014](https://github.com/manticoresoftware/manticoresearch/issues/1014) CBO 现在了解 [二级索引](../Server_settings/Searchd.md#secondary_indexes)，并可以更智能地工作。
  - [Commit cef9](https://github.com/manticoresoftware/columnar/commit/cef9b54dc3a4ee9138ebc4df3a671715b996d6b3) 列式表/字段的编码统计现在存储在元数据中，以帮助 CBO 做出更明智的决策。
  - [Commit 2b95](https://github.com/manticoresoftware/manticoresearch/commit/2b95fb8c2ba422c9c5a66e2b61c1f7922f31c7aa) 添加了 CBO 提示以微调其行为。
* [遥测](../Telemetry.md#Telemetry)：我们很高兴地宣布此次发布添加了遥测功能。此功能使我们能够收集匿名和去个性化的指标，以帮助我们改善产品的性能和用户体验。请放心，所有收集的数据都是 **完全匿名的，并且不会与任何个人信息关联**。如果需要，此功能可以在设置中 [轻松关闭](../Telemetry.md#Telemetry)。
* [Commit 5aaf](https://github.com/manticoresoftware/manticoresearch/commit/5aafb17d81b601e399e212a344a4738f65576cdd) [ALTER TABLE table_name REBUILD SECONDARY](../Updating_table_schema_and_settings.md#Rebuilding-a-secondary-index) 以便在您想要时重建二级索引，例如：
  - 当您从 Manticore 5 迁移到新版本时，
  - 当您对索引中的属性进行了 [UPDATE](../Data_creation_and_modification/Updating_documents/UPDATE.md#UPDATE) （即 [就地更新，而不是替换](Data_creation_and_modification/Updating_documents/REPLACE_vs_UPDATE.md)）
* [Issue #821](https://github.com/manticoresoftware/manticoresearch/issues/821) 新工具 `manticore-backup` 用于 [备份和恢复 Manticore 实例](../Securing_and_compacting_a_table/Backup_and_restore.md)
* SQL 命令 [BACKUP](../Securing_and_compacting_a_table/Backup_and_restore.md#BACKUP-SQL-command-reference) 用于从 Manticore 内部进行备份。
* SQL 命令 [SHOW QUERIES](../Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES) 作为查看正在运行的查询而不是线程的简单方式。
* [Issue #551](https://github.com/manticoresoftware/manticoresearch/issues/551) SQL 命令 `KILL` 用于终止长时间运行的 `SELECT`。
* 动态 `max_matches` 用于聚合查询，以提高准确性并降低响应时间。

### 小改动
* [Issue #822](https://github.com/manticoresoftware/manticoresearch/issues/822) SQL 命令 [FREEZE/UNFREEZE](../Securing_and_compacting_a_table/Freezing_a_table.md) 以便为备份准备实时/普通表。
* [Commit c470](https://github.com/manticoresoftware/manticoresearch/commit/c47052f18ad4868134de1a97e8d7da570e531dfa) 新设置 `accurate_aggregation` 和 `max_matches_increase_threshold` 用于控制聚合精度。
* [Issue #718](https://github.com/manticoresoftware/manticoresearch/issues/718) 支持带符号的负 64 位 ID。请注意，您仍然不能使用 ID > 2^63，但可以使用从 -2^63 到 0 的范围内的 ID。
* 由于我们最近添加了对二级索引的支持，“索引” 一词可能会让人感到困惑，因为它可以指二级索引、全文索引或普通/实时 `索引`。为减少混淆，我们将后者重命名为“表”。以下 SQL/命令行命令受此更改的影响。它们的旧版本已被弃用，但仍可正常使用：
  - `index <table name>` => `table <table name>`，
  - `searchd -i / --index` => `searchd -t / --table`，
  - `SHOW INDEX STATUS` => `SHOW TABLE STATUS`，
  - `SHOW INDEX SETTINGS` => `SHOW TABLE SETTINGS`，
  - `FLUSH RTINDEX` => `FLUSH TABLE`，
  - `OPTIMIZE INDEX` => `OPTIMIZE TABLE`，
  - `ATTACH TABLE plain TO RTINDEX rt` => `ATTACH TABLE plain TO TABLE rt`，
  - `RELOAD INDEX` => `RELOAD TABLE`，
  - `RELOAD INDEXES` => `RELOAD TABLES`。

  我们不打算使旧形式变得过时，但为了确保与文档的兼容性，我们建议在您的应用程序中更改名称。将来版本中将更改的内容是各种 SQL 和 JSON 命令输出中的“索引”重命名为“表”。
* 使用有状态 UDF 的查询现在被强制在单个线程中执行。
* [Issue #1011](https://github.com/manticoresoftware/manticoresearch/issues/1011) 时间调度相关的重构，作为并行块合并的先决条件。
* **⚠️ BREAKING CHANGE**：列存储格式已更改。您需要重建那些具有列属性的表。
* **⚠️ BREAKING CHANGE**：二级索引文件格式已更改，因此如果您在搜索中使用二级索引并且在配置文件中设置了 `searchd.secondary_indexes = 1`，请注意新版本的 Manticore **将跳过加载具有二级索引的表**。建议您：
  - 在升级之前，将配置文件中的 `searchd.secondary_indexes` 更改为 0。
  - 运行实例，Manticore 将加载带有警告的表。
  - 对每个索引运行 `ALTER TABLE <table name> REBUILD SECONDARY` 以重建二级索引。
  如果您正在运行复制集群，则需要在所有节点上运行 `ALTER TABLE <table name> REBUILD SECONDARY` 或按照 [此说明](../Securing_and_compacting_a_table/Compacting_a_table.md#Optimizing-clustered-tables) 进行，只需更改：运行 `ALTER .. REBUILD SECONDARY` 而不是 `OPTIMIZE`。
* **⚠️ 重大变更**：binlog 版本已更新，因此来自以前版本的任何 binlog 都将无法重放。在升级过程中确保 Manticore Search 以干净的方式停止是很重要的。这意味着在停止以前的实例后， `/var/lib/manticore/binlog/` 中不应有除 `binlog.meta` 之外的 binlog 文件。
* [Issue #849](https://github.com/manticoresoftware/manticoresearch/issues/849) `SHOW SETTINGS`：您现在可以从 Manticore 内部查看配置文件中的设置。
* [Issue #1007](https://github.com/manticoresoftware/manticoresearch/issues/1007) [SET GLOBAL CPUSTATS=1/0](../Server_settings/Setting_variables_online.md#SET) 用于开启/关闭 CPU 时间追踪；当 CPU 时间追踪关闭时， [SHOW THREADS](../Node_info_and_management/SHOW_THREADS.md) 现在不显示 CPU 统计信息。
* [Issue #1009](https://github.com/manticoresoftware/manticoresearch/issues/1009) RT 表 RAM 块段现在可以在 RAM 块被冲刷时合并。
* [Issue #1012](https://github.com/manticoresoftware/manticoresearch/issues/1012) 将次级索引进度添加到 [indexer](../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) 的输出中。
* [Issue #1013](https://github.com/manticoresoftware/manticoresearch/issues/1013) 以前，如果 Manticore 在启动时无法开始提供表记录，则该记录可能会从索引列表中删除。新的行为是将其保留在列表中，以便在下次启动时尝试加载。
* [indextool --docextract](../Miscellaneous_tools.md#indextool) 返回请求文档的所有单词和命中。
* [Commit 2b29](https://github.com/manticoresoftware/manticoresearch/commit/2b296ee20e520b85bcbb4383e87095d31e7165dc) 环境变量 `dump_corrupt_meta` 启用在 searchd 无法加载索引的情况下将损坏的表元数据转储到日志中。
* [Commit c7a3](https://github.com/manticoresoftware/manticoresearch/commit/c7a3136eefe0eb2f69e3d99fae9c10beb5821ea6) `DEBUG META` 可以显示 `max_matches` 和伪分片统计信息。
* [Commit 6bca](https://github.com/manticoresoftware/manticoresearch/commit/6bca4848dc5c8396dbc1e057be4b9b2d579794e8) 提供了一个更好的错误提示，而不是令人困惑的“索引头格式不是 json，将尝试将其视为二进制...” 。
* [Commit bef3](https://github.com/manticoresoftware/lemmatizer-uk/commit/bef3ff0386d3ee87ec57619782100972c1122e47) 乌克兰语词形还原器路径已更改。
* [Commit 4ae7](https://github.com/manticoresoftware/manticoresearch/commit/4ae789595329a2951e194d1191ddb3121459a560) 次级索引统计信息已添加到 [SHOW META](../Node_info_and_management/SHOW_META.md#SHOW-META)。
* [Commit 2e7c](https://github.com/manticoresoftware/manticoresearch/commit/2e7c585e) JSON 接口现在可以使用 Swagger Editor 轻松可视化 https://manual.manticoresearch.com/Openapi#OpenAPI-specification。
* **⚠️ 重大变更**：复制协议已更改。如果您正在运行复制集群，则在升级到 Manticore 5 时需要：
  - 首先干净地停止所有节点
  - 然后使用 `--new-cluster` 启动最后停止的节点（在 Linux 中运行工具 `manticore_new_cluster`）。
  - 阅读 [重启集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster) 以获取更多详细信息。

### 与 Manticore 列式库相关的更改
* 次级索引与列式存储的集成重构。
* [Commit efe2](https://github.com/manticoresoftware/columnar/commit/efe26b2a) Manticore 列式库优化，可以通过部分初步的最小/最大评估来降低响应时间。
* [Commit 2757](https://github.com/manticoresoftware/columnar/commit/2757b99b) 如果磁盘块合并被中断，守护进程现在会清理与 MCL 相关的临时文件。
* [Commit e9c6](https://github.com/manticoresoftware/manticoresearch/commit/e9c672014b63e2b49e7a5a53880583af86c3ab4e) 在崩溃时列式和次级库版本被转储到日志中。
* [Commit f5e8](https://github.com/manticoresoftware/columnar/commit/f5e84eeb8b6cb102f13e3468246702427527fef9) 为次级索引添加了快速文档列表回绕的支持。
* [Commit 06df](https://github.com/manticoresoftware/manticoresearch/commit/06dfdd2e668933495dd6cb52d5ba2e0aa9b26e58) 查询如 `select attr, count(*) from plain_index`（无过滤）在使用 MCL 时现在更快。
* [Commit 0a76](https://github.com/manticoresoftware/manticoresearch/commit/0a76ecb9adbe886868b962dd425de5f475bbf55e) @@autocommit 在 HandleMysqlSelectSysvar 中以便与大于 8.25 的 mysql 的 .net connector 兼容。
* **⚠️ 重大变更**： [MCL 问题 #17](https://github.com/manticoresoftware/columnar/issues/17) MCL：添加 SSE 代码以进行列式扫描。MCL 现在至少需要 SSE4.2。

### 与打包相关的更改
* [Commit 4d19](https://github.com/manticoresoftware/manticoresearch/commit/4d19f5cbe49a31228aa09253f061165bfe80e51b) **⚠️ 重大变更**：已停止对 Debian Stretch 和 Ubuntu Xenial 的支持。
* RHEL 9 支持，包括 Centos 9、Alma Linux 9 和 Oracle Linux 9。
* [Issue #924](https://github.com/manticoresoftware/manticoresearch/issues/924) Debian Bookworm 支持。
* [Issue #636](https://github.com/manticoresoftware/manticoresearch/issues/636) 打包：为 Linux 和 MacOS 提供 arm64 版本。
* [PR #26](https://github.com/manticoresoftware/docker/pull/26) 多架构 (x86_64 / arm64) docker 镜像。
* [为贡献者简化打包构建](../Installation/Compiling_from_sources.md#Building-using-CI-Docker)。
* 现在可以使用 APT 安装特定版本。
* [Commit a6b8](https://github.com/manticoresoftware/manticoresearch/commit/51fddca5c2a3ebb8576fae4c18660656ba22de0f) Windows 安装程序（之前我们只提供了一个归档文件）。
* 切换到使用 CLang 15 进行编译。
* **⚠️ 重大变化**：自定义 Homebrew 公式，包括 Manticore 列库的公式。要安装 Manticore、MCL 和任何其他必要组件，请使用以下命令 `brew install manticoresoftware/manticore/manticoresearch manticoresoftware/manticore/manticore-extra`。

### Bug 修复
* [Issue #479](https://github.com/manticoresoftware/manticoresearch/issues/479) 名称为 `text` 的字段
* [Issue #501](https://github.com/manticoresoftware/manticoresearch/issues/501) id 不能是非 bigint
* [Issue #646](https://github.com/manticoresoftware/manticoresearch/issues/646) ALTER 与名称为 "text" 的字段
* ❗[Issue #652](https://github.com/manticoresoftware/manticoresearch/issues/652) 可能的 BUG：HTTP (JSON) 偏移量和限制影响面结果
* ❗[Issue #827](https://github.com/manticoresoftware/manticoresearch/issues/827) Searchd 在高负载下挂起/崩溃
* ❗[Issue #996](https://github.com/manticoresoftware/manticoresearch/issues/996) PQ 索引内存不足
* ❗[Commit 1041](https://github.com/manticoresoftware/manticoresearch/commit/10416ef7dddf06c0d759e32ccd6ebaa2468f7cbf) `binlog_flush = 1` 自 Sphinx 起一直存在问题。已修复。
* [MCL Issue #14](https://github.com/manticoresoftware/columnar/issues/14) MCL：当 ft 字段过多时在选择时崩溃
* [Issue #470](https://github.com/manticoresoftware/manticoresearch/issues/470) sql_joined_field 不能存储
* [Issue #713](https://github.com/manticoresoftware/manticoresearch/issues/713) 使用 LEVENSHTEIN() 时崩溃
* [Issue #743](https://github.com/manticoresoftware/manticoresearch/issues/743) Manticore 意外崩溃，无法正常重启
* [Issue #788](https://github.com/manticoresoftware/manticoresearch/issues/788) 通过 /sql 调用 KEYWORDS 返回控制字符，导致 json 破坏
* [Issue #789](https://github.com/manticoresoftware/manticoresearch/issues/789) mariadb 无法创建 FEDERATED 表
* [Issue #796](https://github.com/manticoresoftware/manticoresearch/issues/796) 警告：dlopen() 失败：/usr/bin/lib_manticore_columnar.so：无法打开共享对象文件：没有此类文件或目录
* [Issue #797](https://github.com/manticoresoftware/manticoresearch/issues/797) 当通过 api 进行 ZONESPAN 搜索时 Manticore 崩溃
* [Issue #799](https://github.com/manticoresoftware/manticoresearch/issues/799) 使用多个索引和面 Distinct 时权重错误
* [Issue #801](https://github.com/manticoresoftware/manticoresearch/issues/801) SphinxQL 组查询在 SQL 索引重新处理后挂起
* [Issue #802](https://github.com/manticoresoftware/manticoresearch/issues/802) MCL：在 5.0.2 和 manticore-columnar-lib 1.15.4 中索引器崩溃
* [Issue #813](https://github.com/manticoresoftware/manticoresearch/issues/813) Manticore 5.0.2 FEDERATED 返回空集（MySQL 8.0.28）
* [Issue #824](https://github.com/manticoresoftware/manticoresearch/issues/824) 当结果为零时在 2 个索引上进行 COUNT DISTINCT 选择会抛出内部错误
* [Issue #826](https://github.com/manticoresoftware/manticoresearch/issues/826) 删除查询时崩溃
* [Issue #843](https://github.com/manticoresoftware/manticoresearch/issues/843) MCL：长文本字段的 bug
* [Issue #856](https://github.com/manticoresoftware/manticoresearch/issues/856) 5.0.2 rtindex：聚合搜索限制行为不如预期
* [Issue #863](https://github.com/manticoresoftware/manticoresearch/issues/863) 返回的命中是 Nonetype 对象，即使对于应该返回多个结果的搜索也是如此
* [Issue #870](https://github.com/manticoresoftware/manticoresearch/issues/870) 在 SELECT 表达式中使用属性和存储字段时崩溃
* [Issue #872](https://github.com/manticoresoftware/manticoresearch/issues/872) 崩溃后表变得不可见
* [Issue #877](https://github.com/manticoresoftware/manticoresearch/issues/877) 搜索查询中的两个负项会给出错误：查询不可计算
* [Issue #878](https://github.com/manticoresoftware/manticoresearch/issues/878) a -b -c 通过 json query_string 不起作用
* [Issue #886](https://github.com/manticoresoftware/manticoresearch/issues/886) 伪分片与查询匹配
* [Issue #893](https://github.com/manticoresoftware/manticoresearch/issues/893) Manticore 5.0.2 min/max 函数的工作不如预期 ...
* [Issue #896](https://github.com/manticoresoftware/manticoresearch/issues/896) 字段 "weight" 解析不正确
* [Issue #897](https://github.com/manticoresoftware/manticoresearch/issues/897) Manticore 服务启动时崩溃并不断重启
* [Issue #900](https://github.com/manticoresoftware/manticoresearch/issues/900) 按 j.a 分组时，一些内容工作不正常
* [Issue #913](https://github.com/manticoresoftware/manticoresearch/issues/913) 当在排序器中使用 expr 时，Searchd 崩溃，但仅对具有两个接近度的查询
* [Issue #916](https://github.com/manticoresoftware/manticoresearch/issues/916) net_throttle_action 已损坏
* [Issue #919](https://github.com/manticoresoftware/manticoresearch/issues/919) MCL：在查询执行时 Manticore 崩溃，其他在集群恢复期间崩溃。
* [Issue #925](https://github.com/manticoresoftware/manticoresearch/issues/925) SHOW CREATE TABLE 输出没有反引号
* [Issue #930](https://github.com/manticoresoftware/manticoresearch/issues/930) 现在可以通过 JDBC 连接器从 Java 查询 Manticore
* [Issue #933](https://github.com/manticoresoftware/manticoresearch/issues/933) bm25f 排名问题
* [Issue #934](https://github.com/manticoresoftware/manticoresearch/issues/934) 无配置索引在第一加载状态冻结在监视程序中
* [Issue #937](https://github.com/manticoresoftware/manticoresearch/issues/937) 排序面数据时发生段错误
* [Issue #940](https://github.com/manticoresoftware/manticoresearch/issues/940) 在 CONCAT(TO_STRING) 时崩溃
* [Issue #947](https://github.com/manticoresoftware/manticoresearch/issues/947) 在某些情况下，单个简单选择可能导致整个实例停滞，因此您无法登录或运行其他查询，直到正在运行的选择完成。
* [Issue #948](https://github.com/manticoresoftware/manticoresearch/issues/948) 索引器崩溃
* [Issue #950](https://github.com/manticoresoftware/manticoresearch/issues/950) 从 facet distinct 返回错误的计数
* [Issue #953](https://github.com/manticoresoftware/manticoresearch/issues/953) 内置 sph04 排序器中 LCS 计算不正确
* [Issue #955](https://github.com/manticoresoftware/manticoresearch/issues/955) 5.0.3 开发版本崩溃
* [Issue #963](https://github.com/manticoresoftware/manticoresearch/issues/963) 在列式引擎列上使用 JSON 的 FACET 崩溃
* [Issue #982](https://github.com/manticoresoftware/manticoresearch/issues/982) MCL: 5.0.3 从二级索引崩溃
* [PR #984](https://github.com/manticoresoftware/manticoresearch/pull/984) @@autocommit 在 HandleMysqlSelectSysvar 中
* [PR #985](https://github.com/manticoresoftware/manticoresearch/pull/985) 修复 RT 索引中的线程块分布
* [Issue #985](https://github.com/manticoresoftware/manticoresearch/issues/985) 修复 RT 索引中的线程块分布
* [Issue #986](https://github.com/manticoresoftware/manticoresearch/issues/986) 默认 max_query_time 错误
* [Issue #987](https://github.com/manticoresoftware/manticoresearch/issues/987) 在多线程执行中使用正则表达式时崩溃
* [Issue #988](https://github.com/manticoresoftware/manticoresearch/issues/988) 破坏了向后兼容的索引
* [Issue #989](https://github.com/manticoresoftware/manticoresearch/issues/989) indextool 报告检查列属性时出错
* [Issue #990](https://github.com/manticoresoftware/manticoresearch/issues/990) JSON grouper 克隆的内存泄漏
* [Issue #991](https://github.com/manticoresoftware/manticoresearch/issues/991) levenshtein 函数克隆的内存泄漏
* [Issue #992](https://github.com/manticoresoftware/manticoresearch/issues/992) 加载元数据时错误信息丢失
* [Issue #993](https://github.com/manticoresoftware/manticoresearch/issues/993) 从动态索引/子键和系统变量传播错误
* [Issue #994](https://github.com/manticoresoftware/manticoresearch/issues/994) 在列式存储中对列式字符串进行计数的崩溃
* [Issue #995](https://github.com/manticoresoftware/manticoresearch/issues/995) MCL: 从 taxi1 获取 min(pickup_datetime) 崩溃
* [Issue #997](https://github.com/manticoresoftware/manticoresearch/issues/997) 空的排除 JSON 查询从选择列表中移除列
* [Issue #998](https://github.com/manticoresoftware/manticoresearch/issues/998) 当前调度器上运行的二级任务有时会导致异常副作用
* [Issue #999](https://github.com/manticoresoftware/manticoresearch/issues/999) 使用 facet distinct 和不同模式时崩溃
* [Issue #1000](https://github.com/manticoresoftware/manticoresearch/issues/1000) MCL: 在没有列式库的情况下运行后，列式 rt 索引损坏
* [Issue #1001](https://github.com/manticoresoftware/manticoresearch/issues/1001) 在 JSON 中隐式截止不起作用
* [Issue #1002](https://github.com/manticoresoftware/manticoresearch/issues/1002) 列式分组器问题
* [Issue #1003](https://github.com/manticoresoftware/manticoresearch/issues/1003) 无法删除索引中的最后一个字段
* [Issue #1004](https://github.com/manticoresoftware/manticoresearch/issues/1004) --new-cluster 之后的错误行为
* [Issue #1005](https://github.com/manticoresoftware/manticoresearch/issues/1005) “columnar library not loaded”，但它不是必需的
* [Issue #1006](https://github.com/manticoresoftware/manticoresearch/issues/1006) 删除查询没有错误
* [Issue #1010](https://github.com/manticoresoftware/manticoresearch/issues/1010) 修复 Windows 构建中的 ICU 数据文件位置
* [PR #1018](https://github.com/manticoresoftware/manticoresearch/pull/1018) 握手发送问题
* [Issue #1020](https://github.com/manticoresoftware/manticoresearch/issues/1020) 在展示创建表时显示 ID
* [Issue #1024 crash 1](https://github.com/manticoresoftware/manticoresearch/issues/1024) 在结果数量较多的情况下，Facet 搜索崩溃/分段错误。
* [Issue #1026](https://github.com/manticoresoftware/manticoresearch/issues/1026) RT 索引：当正在插入许多文档且 RAMchunk 已满时，searchd “卡住” 永久不变
* [Commit 4739](https://github.com/manticoresoftware/manticoresearch/commit/4739dafd) 线程在关闭时卡住，而节点之间的复制正在忙
* [Commit ab87](https://github.com/manticoresoftware/manticoresearch/commit/ab87836782e7ae43fe5f9dea739eed0d336b42c1) 在 JSON 范围过滤器中混合浮点数和整数可能导致 Manticore 忽略该过滤器
* [Commit d001](https://github.com/manticoresoftware/manticoresearch/commit/d00101c2905f6393ce3fca23c4f6dcb2506f4bd9) JSON 中的浮动过滤器不准确
* [Commit 4092](https://github.com/manticoresoftware/manticoresearch/commit/4092c6a8) 如果索引被更改则丢弃未提交的事务（或者可能崩溃）
* [Commit 9692](https://github.com/manticoresoftware/manticoresearch/commit/9692a840) 使用反斜杠时的查询语法错误
* [Commit 0c19](https://github.com/manticoresoftware/manticoresearch/commit/0c19ad8d) workers_clients 在 SHOW STATUS 中可能出错
* [Commit 1772](https://github.com/manticoresoftware/manticoresearch/commit/17720e1e2f2ec38337df565bee78f7ce9d9c744a) 修复没有文档存储的合并 ram 段时崩溃
* [Commit f45b](https://github.com/manticoresoftware/manticoresearch/commit/f45b337b5c265433bb7d6262ad65d02f65edbaf3) 修复 JSON 过滤器的等于条件的丢失 ALL/ANY 条件
* [Commit 3e83](https://github.com/manticoresoftware/manticoresearch/commit/3e83c1f0c0e6e7a9d4297113f69a112cbabd9e64) 复制可能会失败，错误信息为`got exception while reading ist stream: mkstemp(./gmb_pF6TJi) failed: 13 (Permission denied)`，如果searchd从一个无法写入的目录启动。
* [Commit 92e5](https://github.com/manticoresoftware/manticoresearch/commit/92e5629f7150f62524b385fca670a01ba9590f5b) 自4.0.2版本以来，崩溃日志仅包含偏移量。此提交修复了此问题。

# 版本 5.0.2
发布日期：2022年5月30日

### 修复的错误
* ❗[问题 #791](https://github.com/manticoresoftware/manticoresearch/issues/791) - 错误的栈大小可能导致崩溃。

# 版本 5.0.0
发布日期：2022年5月18日


### 主要新特性
* 🔬 支持[Manticore Columnar Library 1.15.2](https://github.com/manticoresoftware/columnar/)，启用[二级索引](../Server_settings/Searchd.md#secondary_indexes)的beta版本。默认情况下，普通和实时列式及行式索引的二级索引构建是开启的（如果使用[Manticore Columnar Library](https://github.com/manticoresoftware/columnar)），但要在搜索中启用它，您需要在配置文件中或者使用[SET GLOBAL](../Server_settings/Setting_variables_online.md)设置`secondary_indexes = 1`。此新功能在所有操作系统中都受到支持，除了旧版Debian Stretch和Ubuntu Xenial。
* [只读模式](Security/Read_only.md)：现在您可以指定仅处理读取查询而丢弃任何写入的监听器。
* 新的[/cli](../Connecting_to_the_server/HTTP.md#/cli)端点，用于通过HTTP运行SQL查询，更加方便。
* 通过JSON在HTTP上进行更快的批量INSERT/REPLACE/DELETE：之前您可以通过HTTP JSON协议提供多个写入命令，但它们是逐个处理的，现在它们作为单个事务处理。
* [#720](https://github.com/manticoresoftware/manticoresearch/issues/720) JSON协议支持[嵌套过滤器](../Searching/Filters.md#Nested-bool-query)。之前您无法在JSON中编写类似`a=1 and (b=2 or c=3)`的内容：`must`（AND）、`should`（OR）和`must_not`（NOT）仅在最高级别上工作。现在它们可以嵌套。
* 在HTTP协议中支持[分块传输编码](https://en.wikipedia.org/wiki/Chunked_transfer_encoding)。您现在可以在应用程序中使用分块传输来传输大型批次，减少资源消耗（因为不需要计算`Content-Length`）。在服务器端，Manticore现在总是以流式方式处理传入的HTTP数据，无需等待整个批次像以前那样传输，这：
  - 减少峰值RAM使用，降低OOM风险
  - 减少响应时间（我们的测试表明处理100MB批次的时间缩短了11%）
  - 允许您绕过[max_packet_size](../Server_settings/Searchd.md#max_packet_size)，传输比`max_packet_size`（128MB）最大的允许值更大的批量，例如一次1GB。
* [#719](https://github.com/manticoresoftware/manticoresearch/issues/719) 支持HTTP接口的`100 Continue`：现在您可以从`curl`中传输大型批量（包括各种编程语言使用的curl库），其默认执行`Expect: 100-continue`并在实际发送批量之前等待一段时间。之前您必须添加`Expect: `头，现在不需要了。

  <details>

  之前（注意响应时间）：

  ```bash
  $ time curl -v -sX POST http://localhost:9318/bulk -H "Content-Type: application/x-ndjson" --data '{"insert": {"index": "user", "doc":  {"name":"Prof. Matt Heaney IV","email":"ibergnaum@yahoo.com","description":"Tempora ullam eaque consequatur. Vero aut minima ut et ut omnis officiis vel. Molestiae quis voluptatum sint numquam.","age":15,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"Prof. Boyd McKenzie","email":"carlotta11@hotmail.com","description":"Blanditiis maiores odio corporis eaque illum. Aut et rerum iste. Neque et ullam quisquam officia dignissimos quo cumque.","age":84,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"Mr. Johann Smith","email":"stiedemann.tristin@ziemann.com","description":"Temporibus amet magnam consequatur omnis consequatur illo fugit. Debitis natus doloremque est tempore deserunt vero. Harum eos corrupti nemo ut.","age":89,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"Hector Pouros","email":"hickle.mafalda@hotmail.com","description":" as voluptatem inventore sit. Aliquam fugit perferendis est id aut odio et sapiente.","age":64,"active":1}}}'
  *   Trying 127.0.0.1...
  * Connected to localhost (127.0.0.1) port 9318 (#0)
  > POST /bulk HTTP/1.1
  > Host: localhost:9318
  > User-Agent: curl/7.47.0
  > Accept: */*
  > Content-Type: application/x-ndjson
  > Content-Length: 1025
  > Expect: 100-continue
  >
  * Done waiting for 100-continue
  * We are completely uploaded and fine
  < HTTP/1.1 200 OK
  < Server: 4.2.0 15e927b@211223 release (columnar 1.11.4 327b3d4@211223)
  < Content-Type: application/json; charset=UTF-8
  < Content-Length: 434
  <
  * Connection #0 to host localhost left intact
  {"items":[{"insert":{"table":"user","_id":2811798918248005633,"created":true,"result":"created","status":201}},{"insert":{"table":"user","_id":2811798918248005634,"created":true,"result":"created","status":201}},{"insert":{"table":"user","_id":2811798918248005635,"created":true,"result":"created","status":201}},{"insert":{"table":"user","_id":2811798918248005636,"created":true,"result":"created","status":201}}],"errors":false}
  real	0m1.022s
  user	0m0.001s
  sys	0m0.010s
  ```

  现在：
  ```bash
$ time curl -v -sX POST http://localhost:9318/bulk -H "Content-Type: application/x-ndjson" --data '{"insert": {"index": "user", "doc":  {"name":"Prof. Matt Heaney IV","email":"ibergnaum@yahoo.com","description":"Tempora ullam eaque consequatur. Vero aut minima ut et ut omnis officiis vel. Molestiae quis voluptatum sint numquam.","age":15,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"Prof. Boyd McKenzie","email":"carlotta11@hotmail.com","description":"Blanditiis maiores odio corporis eaque illum. Aut et rerum iste. Neque et ullam quisquam officia dignissimos quo cumque.","age":84,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"Mr. Johann Smith","email":"stiedemann.tristin@ziemann.com","description":"Temporibus amet magnam consequatur omnis consequatur illo fugit. Debitis natus doloremque est tempore deserunt vero. Harum eos corrupti nemo ut.","age":89,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"Hector Pouros","email":"hickle.mafalda@hotmail.com","description":" as voluptatem inventore sit. Aliquam fugit perferendis est id aut odio et sapiente.","age":64,"active":1}}}'
  *   Trying 127.0.0.1...
  * Connected to localhost (127.0.0.1) port 9318 (#0)
  > POST /bulk HTTP/1.1
  > Host: localhost:9318
  > User-Agent: curl/7.47.0
  > Accept: */*
  > Content-Type: application/x-ndjson
  > Content-Length: 1025
  > Expect: 100-continue
  >
  < HTTP/1.1 100 Continue
  < Server: 4.2.1 63e5749@220405 dev
  < Content-Type: application/json; charset=UTF-8
  < Content-Length: 0
  * We are completely uploaded and fine
  < HTTP/1.1 200 OK
  < Server: 4.2.1 63e5749@220405 dev
  < Content-Type: application/json; charset=UTF-8
  < Content-Length: 147
  <
  * Connection #0 to host localhost left intact
  {"items":[{"bulk":{"table":"user","_id":2811798919590182916,"created":4,"deleted":0,"updated":0,"result":"created","status":201}}],"errors":false}
  real	0m0.015s
  user	0m0.005s
  sys	0m0.004s
  ```

  </details>

* **⚠️ 重大变更**：[伪分片](../Server_settings/Searchd.md#pseudo_sharding)默认启用。如果你想禁用它，请确保在 Manticore 配置文件的 `searchd` 节添加 `pseudo_sharding = 0`。
* 在实时/普通索引中至少有一个全文字段不再是强制的。你现在可以在没有全文搜索需求的情况下使用 Manticore。
* [快速获取](../Creating_a_table/Data_types.md#fast_fetch) 对于由 [Manticore 列式库](https://github.com/manticoresoftware/columnar) 支持的属性：像 `select * from <columnar table>` 这样的查询现在比以前快得多，尤其是当架构中有许多字段时。
* **⚠️ 重大变更**：隐式 [截断](../Searching/Options.md#cutoff)。Manticore 现在不会花费时间和资源处理结果集中不需要的数据。缺点是它会影响 [SHOW META](../Node_info_and_management/SHOW_META.md#SHOW-META) 中的 `total_found` 和 JSON 输出中的 [hits.total](../Searching/Full_text_matching/Basic_usage.md#HTTP-JSON)。现在只有在看到 `total_relation: eq` 时才是准确的，而 `total_relation: gte` 意味着匹配文档的实际数量大于你获得的 `total_found` 值。要保留之前的行为，你可以使用搜索选项 `cutoff=0`，这会使 `total_relation` 始终为 `eq`。
* **⚠️ 重大变更**：所有全文字段现在默认为 [已存储](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#stored_fields)。你需要使用 `stored_fields = `（空值）使所有字段变为非存储（即恢复到之前的行为）。
* [#715](https://github.com/manticoresoftware/manticoresearch/issues/715) HTTP JSON 支持 [搜索选项](../Searching/Options.md#General-syntax)。

### 次要变更
* **⚠️ 重大变更**：索引元文件格式更改。以前元文件（`.meta`、`.sph`）是二进制格式，现在是 JSON。新版本的 Manticore 将自动转换旧索引，但是：
  - 你可能会看到警告，如 `WARNING: ... syntax error, unexpected TOK_IDENT`
  - 你将无法使用以前的 Manticore 版本运行索引，请确保你有备份
* **⚠️ 重大变更**：使用 [HTTP 保持连接](../Connecting_to_the_server/HTTP.md#Keep-alive) 支持会话状态。当客户端也支持时，这使 HTTP 变为有状态。例如，使用新的 [/cli](../Connecting_to_the_server/HTTP.md#/cli) 端点和 HTTP 保持连接（在所有浏览器中默认开启）后，你可以在 `SELECT` 之后调用 `SHOW META`，其工作方式与通过 mysql 相同。请注意，以前也支持 `Connection: keep-alive` HTTP 标头，但它只是重用同一连接。从这个版本开始，它还使会话变为有状态。
* 现在可以指定 `columnar_attrs = *` 在 [普通模式](Read_this_first.md#Real-time-mode-vs-plain-mode) 下将所有属性定义为列式，在属性列表很长时很有用。
* 复制 SST 更快
* **⚠️ 重大变更**：复制协议已更改。如果你正在运行复制集群，升级到 Manticore 5 时需要：
  - 先完全停止所有节点
  - 然后使用 `--new-cluster` 启动最后停止的节点（在 Linux 中运行 `manticore_new_cluster` 工具）。
  - 阅读关于 [重启集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster) 的更多详细信息。
* 复制改进：
  - SST 更快
  - 抗噪声，有助于复制节点之间网络不稳定的情况
  - 改进的日志记录
* 安全性改进：当配置中未指定 `listen` 时，Manticore 现在监听 `127.0.0.1` 而不是 `0.0.0.0`。尽管在随 Manticore Search 附带的默认配置中指定了 `listen` 设置，并且没有 `listen` 的配置不常见，但仍然是可能的。之前 Manticore 会监听 `0.0.0.0`，这并不安全，现在它监听 `127.0.0.1`，通常不向互联网暴露。
* 针对列属性的聚合速度更快。
* 提高了 `AVG()` 精度：之前 Manticore 在聚合时内部使用 `float`，现在使用 `double` ，大大提高了精度。
* 改进对 JDBC MySQL 驱动程序的支持。
* [jemalloc](https://github.com/jemalloc/jemalloc) 的 `DEBUG malloc_stats` 支持。
* [optimize_cutoff](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#optimize_cutoff) 现在可以作为每个表设置，这在您创建或更改表时可以设置。
* **⚠️ 重大更改**： [query_log_format](../Server_settings/Searchd.md#query_log_format) 现在默认为 **`sphinxql`**。如果您习惯于 `plain` 格式，需要在您的配置文件中添加 `query_log_format = plain`。
* 显著减少内存消耗：在使用存储字段的情况下，Manticore 在长时间和密集插入/替换/优化工作负载中的 RAM 消耗显著降低。
* [shutdown_timeout](../Server_settings/Searchd.md#shutdown_timeout) 默认值从 3 秒增加到 60 秒。
* [Commit ffd0](https://github.com/manticoresoftware/manticoresearch/commit/ffd0499d329d2c383f14c8a44c4cc84338ab56e7) 支持 Java mysql connector >= 6.0.3：在 [Java mysql connection 6.0.3](https://mvnrepository.com/artifact/mysql/mysql-connector-java/6.0.3) 中，他们更改了连接 MySQL 的方式，这破坏了与 Manticore 的兼容性。现在支持新行为。
* [Commit 1da6](https://github.com/manticoresoftware/manticoresearch/commit/1da6dbec) 禁用在加载索引时保存新的磁盘块（例如，在 searchd 启动时）。
* [Issue #746](https://github.com/manticoresoftware/manticoresearch/issues/746) 支持 glibc >= 2.34。
* [Issue #784](https://github.com/manticoresoftware/manticoresearch/issues/784) 将 'VIP' 连接与普通 (非 VIP) 连接分别统计。之前 VIP 连接会计入 `max_connections` 限制，这可能导致非 VIP 连接出现“已到达最大值”错误。现在 VIP 连接不计入限制。当前 VIP 连接的数量也可以在 `SHOW STATUS` 和 `status` 中查看。
* [ID](../Creating_a_table/Data_types.md#Document-ID) 现在可以明确指定。
* [Issue #687](https://github.com/manticoresoftware/manticoresearch/issues/687) 支持 mysql 原型的 zstd 压缩。

### ⚠️ 其他少量重大更改
* ⚠️ BM25F 公式已略有更新以提高搜索相关性。只有在使用函数 [BM25F()](../Functions/Searching_and_ranking_functions.md#BM25F%28%29) 时，此更改才会影响搜索结果，它不会更改默认排名公式的行为。
* ⚠️ 更改了 REST [/sql](../Connecting_to_the_server/HTTP.md#mode=raw) 端点的行为：`/sql?mode=raw` 现在需要转义并返回数组。
* ⚠️ `/bulk` INSERT/REPLACE/DELETE 请求的响应格式变化：
  - 之前每个子查询都是一个单独的事务，导致单独的响应
  - 现在整个批处理被视为一个单独的事务，返回单个响应
* ⚠️ 搜索选项 `low_priority` 和 `boolean_simplify` 现在需要一个值 (`0/1`)：之前您可以执行 `SELECT ... OPTION low_priority, boolean_simplify`，现在您需要执行 `SELECT ... OPTION low_priority=1, boolean_simplify=1`。
* ⚠️ 如果您使用旧的 [php](https://github.com/manticoresoftware/manticoresearch/blob/master/api/sphinxapi.php)、[python](https://github.com/manticoresoftware/manticoresearch/blob/master/api/sphinxapi.py) 或 [java](https://github.com/manticoresoftware/manticoresearch/tree/master/api/java) 客户端，请遵循相应的链接并找到更新版本。**旧版本与 Manticore 5 不完全兼容。**
* ⚠️ HTTP JSON 请求现在在模式 `query_log_format=sphinxql` 下以不同格式记录。之前仅记录全文部分，现在完整记录。

### 新包
* **⚠️ 重大更改**：由于新的结构，当您升级到 Manticore 5 时，建议在安装新包之前删除旧包：
  - 基于 RPM：`yum remove manticore*`
  - Debian 和 Ubuntu：`apt remove manticore*`
* 新的 deb/rpm 包结构。以前的版本提供了：
  - `manticore-server` 及其所需的 `searchd`（主搜索守护进程）
  - `manticore-tools` 包括 `indexer` 和 `indextool`
  - `manticore` 包括所有内容
  - `manticore-all` RPM 作为一个元包，指向 `manticore-server` 和 `manticore-tools`

  新结构为：
  - `manticore` - deb/rpm 元包，作为依赖项安装上述所有内容
  - `manticore-server-core` - `searchd` 和单独运行它所需的一切
  - `manticore-server` - systemd 文件和其他补充脚本
  - `manticore-tools` - `indexer`、`indextool` 和其他工具
  - `manticore-common` - 默认配置文件、默认数据目录、默认停用词
  - `manticore-icudata`、`manticore-dev`、`manticore-converter` 变化不大
  - `.tgz` 打包包含所有包
* 支持 Ubuntu Jammy
* 通过 [YUM repo](../Installation/RHEL_and_Centos.md#YUM-repository) 支持 Amazon Linux 2。

### Bug 修复
* [Issue #815](https://github.com/manticoresoftware/manticoresearch/issues/815) 使用 UDF 功能时随机崩溃。
* [Issue #287](https://github.com/manticoresoftware/manticoresearch/issues/287) 索引 RT 索引时内存不足。
* [Issue #604](https://github.com/manticoresoftware/manticoresearch/issues/604) 重大变更 3.6.0, 4.2.0 sphinxql-parser
* [Issue #667](https://github.com/manticoresoftware/manticoresearch/issues/667) 致命错误：内存不足（无法分配 9007199254740992 字节）
* [Issue #676](https://github.com/manticoresoftware/manticoresearch/issues/676) 字符串未正确传递给 UDF
* ❗[Issue #698](https://github.com/manticoresoftware/manticoresearch/issues/698) 在尝试向 rt 索引添加文本列后，Searchd 崩溃
* [Issue #705](https://github.com/manticoresoftware/manticoresearch/issues/705) 索引器无法找到所有列
* ❗[Issue #709](https://github.com/manticoresoftware/manticoresearch/issues/705) 按 json.boolean 分组出现错误
* [Issue #716](https://github.com/manticoresoftware/manticoresearch/issues/716) 与索引相关的 indextool 命令（例如 --dumpdict）失败
* ❗[Issue #724](https://github.com/manticoresoftware/manticoresearch/issues/724) 字段从选择中消失
* [Issue #727](https://github.com/manticoresoftware/manticoresearch/issues/727) .NET HttpClient Content-Type 不兼容，使用 `application/x-ndjson`
* [Issue #729](https://github.com/manticoresoftware/manticoresearch/issues/729) 字段长度计算
* ❗[Issue #730](https://github.com/manticoresoftware/manticoresearch/issues/730) 创建/插入/删除列式表时存在内存泄漏
* [Issue #731](https://github.com/manticoresoftware/manticoresearch/issues/731) 在某些条件下结果中出现空列
* ❗[Issue #749](https://github.com/manticoresoftware/manticoresearch/issues/749) 守护进程启动时崩溃
* ❗[Issue #750](https://github.com/manticoresoftware/manticoresearch/issues/750) 守护进程启动时挂起
* ❗[Issue #751](https://github.com/manticoresoftware/manticoresearch/issues/751) 在 SST 时崩溃
* [Issue #752](https://github.com/manticoresoftware/manticoresearch/issues/752) Json 属性标记为列式当 engine='columnar'
* [Issue #753](https://github.com/manticoresoftware/manticoresearch/issues/753) 复制监听 0
* [Issue #754](https://github.com/manticoresoftware/manticoresearch/issues/754) columnar_attrs = * 与 csvpipe 不兼容
* ❗[Issue #755](https://github.com/manticoresoftware/manticoresearch/issues/755) 在 rt 的列式中选择 float 时崩溃
* ❗[Issue #756](https://github.com/manticoresoftware/manticoresearch/issues/756) Indextool 在检查过程中更改 rt 索引
* [Issue #757](https://github.com/manticoresoftware/manticoresearch/issues/757) 需要检查监听器端口范围交集
* [Issue #758](https://github.com/manticoresoftware/manticoresearch/issues/758) 记录原始错误，如果 RT 索引未能保存磁盘块
* [Issue #759](https://github.com/manticoresoftware/manticoresearch/issues/759) 仅报告一个 RE2 配置错误
* ❗[Issue #760](https://github.com/manticoresoftware/manticoresearch/issues/760) 提交 5463778558586d2508697fa82e71d657ac36510f 中的 RAM 消耗变化
* [Issue #761](https://github.com/manticoresoftware/manticoresearch/issues/761) 第 3 个节点在脏重启后未形成非主集群
* [Issue #762](https://github.com/manticoresoftware/manticoresearch/issues/762) 更新计数增加 2
* [Issue #763](https://github.com/manticoresoftware/manticoresearch/issues/763) 新版本 4.2.1 会破坏使用形态学创建的 4.2.0 的索引
* [Issue #764](https://github.com/manticoresoftware/manticoresearch/issues/764) json 键中没有转义 /sql?mode=raw
* ❗[Issue #765](https://github.com/manticoresoftware/manticoresearch/issues/765) 使用函数隐藏其他值
* ❗[Issue #766](https://github.com/manticoresoftware/manticoresearch/issues/766) FixupAttrForNetwork 中的一行触发内存泄漏
* ❗[Issue #767](https://github.com/manticoresoftware/manticoresearch/issues/767) 在 4.2.0 和 4.2.1 中与 docstore 缓存相关的内存泄漏
* [Issue #768](https://github.com/manticoresoftware/manticoresearch/issues/768) 网络上的存储字段出现奇怪的乒乓现象
* [Issue #769](https://github.com/manticoresoftware/manticoresearch/issues/769) 如果在“common”部分未提及，则 lemmatizer_base 重置为空
* [Issue #770](https://github.com/manticoresoftware/manticoresearch/issues/770) pseudo_sharding 使通过 id 的 SELECT 更慢
* [Issue #771](https://github.com/manticoresoftware/manticoresearch/issues/771) DEBUG malloc_stats 在使用 jemalloc 时输出零
* [Issue #772](https://github.com/manticoresoftware/manticoresearch/issues/772) 删除/添加列使值不可见
* [Issue #773](https://github.com/manticoresoftware/manticoresearch/issues/773) 无法向列式表添加 column bit(N)
* [Issue #774](https://github.com/manticoresoftware/manticoresearch/issues/774) "cluster" 在 manticore.json 中启动时变为空
* ❗[Commit 1da4](https://github.com/manticoresoftware/manticoresearch/commits/1da4ce891e4ff13727cfd3331d72771c64ee7949) HTTP 操作在 SHOW STATUS 中未被跟踪

* [Commit 3810](https://github.com/manticoresoftware/manticoresearch/commits/381000ab4af4c3c98ec8c730699ad7f39039cec8) 禁用低频单关键字查询的 pseudo_sharding
* [Commit 8003](https://github.com/manticoresoftware/manticoresearch/commits/800325cca283a87801b28929d82420fafc76a0ee) 修复存储属性与索引合并的冲突
* [Commit cddf](https://github.com/manticoresoftware/manticoresearch/commits/cddfeed6296f2041c6aae18414e5cd9cabf08281) 通用的不同值获取器；为列式字符串添加了专用不同获取器
* [Commit fba4](https://github.com/manticoresoftware/manticoresearch/commits/fba4bb4f) 修复从 docstore 获取空整数属性
* [Commit f300](https://github.com/manticoresoftware/manticoresearch/commit/f3009a9242fa16956adf871c4887e6e0303ba364) `ranker` 在查询日志中可以被指定两次

## 版本 4.2.0，2021年12月23日

### 主要新功能
* **实时索引和完整文本查询的伪分片支持**。在之前的版本中，我们添加了有限的伪分片支持。从这个版本开始，您可以通过简单地启用 [searchd.pseudo_sharding](Server_settings/Searchd.md#pseudo_sharding) 来获得伪分片和多核处理器的所有好处。最酷的是，您无需对索引或查询做任何修改，只需启用它，如果有空闲的 CPU，就会用来降低响应时间。它支持用于完整文本、过滤和分析查询的普通和实时索引。例如，这里是启用伪分片如何使大多数查询的 **平均响应时间降低约 10 倍** 在 [Hacker news curated comments dataset](https://zenodo.org/record/45901/) 的基础上乘以 100（116百万文档在普通索引中）。

![伪分片开启与关闭在 4.2.0](4.2.0_ps_on_vs_off.png)

* [**Debian Bullseye**](https://manticoresearch.com/install/) 现在被支持。

<!-- example pq_transactions_4.2.0 -->
* PQ 事务现在是原子性和隔离的。之前 PQ 事务的支持是有限的。它使得 **REPLACE into PQ 更快**，尤其是在您需要一次替换大量规则时。性能细节：

<!-- intro -->
#### 4.0.2

<!-- request 4.0.2 -->

插入 **1M PQ 规则需要 48 秒**，在 10K 批次中 **REPLACE 仅需 406 秒**。

```sql
root@perf3 ~ # mysql -P9306 -h0 -e "drop table if exists pq; create table pq (f text, f2 text, j json, s string) type='percolate';"; date; for m in `seq 1 1000`; do (echo -n "insert into pq (id,query,filters,tags) values "; for n in `seq 1 1000`; do echo -n "(0,'@f (cat | ( angry dog ) | (cute mouse)) @f2 def', 'j.json.language=\"en\"', '{\"tag1\":\"tag1\",\"tag2\":\"tag2\"}')"; [ $n != 1000 ] && echo -n ","; done; echo ";")|mysql -P9306 -h0; done; date; mysql -P9306 -h0 -e "select count(*) from pq"

Wed Dec 22 10:24:30 AM CET 2021
Wed Dec 22 10:25:18 AM CET 2021
+----------+
| count(*) |
+----------+
|  1000000 |
+----------+

root@perf3 ~ # date; (echo "begin;"; for offset in `seq 0 10000 30000`; do n=0; echo "replace into pq (id,query,filters,tags) values "; for id in `mysql -P9306 -h0 -NB -e "select id from pq limit $offset, 10000 option max_matches=1000000"`; do echo "($id,'@f (tiger | ( angry bear ) | (cute panda)) @f2 def', 'j.json.language=\"de\"', '{\"tag1\":\"tag1\",\"tag2\":\"tag2\"}')"; n=$((n+1)); [ $n != 10000 ] && echo -n ","; done; echo ";"; done; echo "commit;") > /tmp/replace.sql; date
Wed Dec 22 10:26:23 AM CET 2021
Wed Dec 22 10:26:27 AM CET 2021
root@perf3 ~ # time mysql -P9306 -h0 < /tmp/replace.sql

real	6m46.195s
user	0m0.035s
sys	0m0.008s
```

<!-- intro -->

#### 4.2.0

<!-- request 4.2.0 -->

插入 **1M PQ 规则需要 34 秒**，在 10K 批次中 **REPLACE 需要 23 秒**。

```sql
root@perf3 ~ # mysql -P9306 -h0 -e "drop table if exists pq; create table pq (f text, f2 text, j json, s string) type='percolate';"; date; for m in `seq 1 1000`; do (echo -n "insert into pq (id,query,filters,tags) values "; for n in `seq 1 1000`; do echo -n "(0,'@f (cat | ( angry dog ) | (cute mouse)) @f2 def', 'j.json.language=\"en\"', '{\"tag1\":\"tag1\",\"tag2\":\"tag2\"}')"; [ $n != 1000 ] && echo -n ","; done; echo ";")|mysql -P9306 -h0; done; date; mysql -P9306 -h0 -e "select count(*) from pq"

Wed Dec 22 10:06:38 AM CET 2021
Wed Dec 22 10:07:12 AM CET 2021
+----------+
| count(*) |
+----------+
|  1000000 |
+----------+

root@perf3 ~ # date; (echo "begin;"; for offset in `seq 0 10000 990000`; do n=0; echo "replace into pq (id,query,filters,tags) values "; for id in `mysql -P9306 -h0 -NB -e "select id from pq limit $offset, 10000 option max_matches=1000000"`; do echo "($id,'@f (tiger | ( angry bear ) | (cute panda)) @f2 def', 'j.json.language=\"de\"', '{\"tag1\":\"tag1\",\"tag2\":\"tag2\"}')"; n=$((n+1)); [ $n != 10000 ] && echo -n ","; done; echo ";"; done; echo "commit;") > /tmp/replace.sql; date
Wed Dec 22 10:12:31 AM CET 2021
Wed Dec 22 10:14:00 AM CET 2021
root@perf3 ~ # time mysql -P9306 -h0 < /tmp/replace.sql

real	0m23.248s
user	0m0.891s
sys	0m0.047s
```

<!-- end -->

### 次要更改
* [optimize_cutoff](Server_settings/Searchd.md#optimize_cutoff) 现在作为 `searchd` 部分中的配置选项可用。它在您想要将所有索引中的实时块数量全球限制为特定数量时非常有用。
* [Commit 0087](https://github.com/manticoresoftware/manticoresearch/commit/00874743823eaf43e2a09a088392fd7e0a617f47) 准确 [count(distinct ...)](Searching/Grouping.md#COUNT%28DISTINCT-field%29) 和 [FACET ... distinct](Searching/Faceted_search.md#Faceting-without-duplicates) 在多个具有相同字段设置/顺序的本地物理索引（实时/普通）上。
* [PR #598](https://github.com/manticoresoftware/manticoresearch/pull/598) bigint 对 `YEAR()` 和其他时间戳函数的支持。
* [Commit 8e85](https://github.com/manticoresoftware/manticoresearch/commit/8e85d4bce596cc88f9d9615ea243a27e4afde0f7) 自适应 [rt_mem_limit](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_mem_limit)。之前，Manticore Search 在保存新的磁盘块到磁盘之前会收集正好达到 `rt_mem_limit` 的数据，而在保存过程中仍会收集多达 10% 的数据（即双缓冲）以最小化可能的插入暂停。如果该限制也被耗尽，则在磁盘块完全保存到磁盘之前，添加新文档会被阻止。新的自适应限制是基于我们现在拥有 [auto-optimize](Server_settings/Searchd.md#auto_optimize) 的事实，因此如果磁盘块不完全遵守 `rt_mem_limit` 并开始提前刷新磁盘块，这并不是大问题。因此，现在我们收集最多 50% 的 `rt_mem_limit` 并将其保存为磁盘块。在保存时，我们查看统计信息（我们保存了多少，保存过程中到达了多少新文档）并重新计算下次将使用的初始速率。例如，如果我们保存了 9000 万个文档，而在保存过程中到达了 1000 万个文档，速率就是 90%，因此我们知道下次可以在开始刷新另一个磁盘块之前收集最多 90% 的 `rt_mem_limit`。速率值自动计算，从 33.3% 到 95%。
* [Issue #628](https://github.com/manticoresoftware/manticoresearch/issues/628) [unpack_zlib](Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Database_connection.md#unpack_zlib) 用于 PostgreSQL 源。感谢 [Dmitry Voronin](https://github.com/dimv36) 的 [贡献](https://github.com/manticoresoftware/manticoresearch/pull/630)。
* [Commit 6d54](https://github.com/manticoresoftware/manticoresearch/commit/6d54cf2b319b131970c29410cc21abcbdf8083b1) `indexer -v` 和 `--version`。之前您仍然可以看到索引器的版本，但不支持 `-v`/`--version`。
* [Issue #662](https://github.com/manticoresoftware/manticoresearch/issues/662) 启动 Manticore 时默认的无限 mlock 限制。
* [Commit 63c8](https://github.com/manticoresoftware/manticoresearch/commit/63c8cd05cf399e705c2c14939411a92cb784735a) 自旋锁 -> coro rwlock 的操作队列。
* [Commit 4113](https://github.com/manticoresoftware/manticoresearch/commit/41130ce30d1734bbf8e6b8b63c4e815caf9464f2) 环境变量 `MANTICORE_TRACK_RT_ERRORS` 对于调试 RT 段损坏非常有用。

### 重大更改
* Binlog 版本已增加，之前版本的 binlog 不会被重放，因此请确保在升级期间干净地停止 Manticore Search：停止之前的实例后，/var/lib/manticore/binlog/ 中不应有 binlog 文件，除了 `binlog.meta`。
* [Commit 3f65](https://github.com/manticoresoftware/manticoresearch/commit/3f659f36e87d99ee262205a8eae4663a255be630) 在 `show threads option format=all` 中新增了 "chain" 列。它显示了一些任务信息票据的堆栈，对于性能分析非常有用，因此如果您正在解析 `show threads` 输出，请注意新列的加入。
* `searchd.workers` 自 3.5.0 起已被废弃，现已不推荐使用，如果您在配置文件中仍然存在，该条目将在启动时触发警告。Manticore Search 会启动，但会有警告。
* 如果您使用 PHP 和 PDO 访问 Manticore，则需要执行 `PDO::ATTR_EMULATE_PREPARES`。

### 错误修复
* ❗[Issue #650](https://github.com/manticoresoftware/manticoresearch/issues/650) Manticore 4.0.2 比 Manticore 3.6.3 慢。4.0.2 在批量插入方面比以前的版本快，但在单个文档插入方面明显更慢。此问题已在 4.2.0 中修复。
* ❗[Commit 22f4](https://github.com/manticoresoftware/manticoresearch/commit/22f4141bed1411c83684ae6e0bae832071cce3df) 在高强度 REPLACE 负载下，RT 索引可能会损坏，或者可能会崩溃。
* [Commit 03be](https://github.com/manticoresoftware/manticoresearch/commit/03be91e4f720a3f20bee0278dd8f0d44abd3ef0e) 修复了在合并分组器和组 N 排序器时的平均值；修复了聚合的合并问题。
* [Commit 2ea5](https://github.com/manticoresoftware/manticoresearch/commit/2ea575d34efe38661f14d858f2b552456f29fe42) `indextool --check` 可能会崩溃。
* [Commit 7ec7](https://github.com/manticoresoftware/manticoresearch/commit/7ec76d4abf01b5fca6f10a0000fe64d5d8e5fa52) 由于 UPDATE 导致的 RAM 耗尽问题。
* [Commit 658a](https://github.com/manticoresoftware/manticoresearch/commit/658a727e41f3c3fbcde9754149528f34ed76ba19) 守护进程可能在 INSERT 时挂起。
* [Commit 46e4](https://github.com/manticoresoftware/manticoresearch/commit/46e42b9b386e31e53247ed634e158b533e28063e) 守护进程在关闭时可能挂起。
* [Commit f8d7](https://github.com/manticoresoftware/manticoresearch/commit/f8d7d5177cc83d250b493b4cfc619732b29cf072) 守护进程在关闭时可能会崩溃。
* [Commit 733a](https://github.com/manticoresoftware/manticoresearch/commit/733accf165142a1d1574689525ccca8907971158) 守护进程可能在崩溃时挂起。
* [Commit f7f8](https://github.com/manticoresoftware/manticoresearch/commit/f7f8bd8c0dbb23a59a88502dcba16b0918fb2330) 守护进程在启动时尝试用无效的节点列表重新加入集群时可能会崩溃。
* [Commit 1401](https://github.com/manticoresoftware/manticoresearch/commit/140155613821ecaddcd1a3dfc4f7369b0df6aef7) 如果在启动时无法解析其代理之一，则分布式索引可能在 RT 模式下被完全遗忘。
* [Issue #683](https://github.com/manticoresoftware/manticoresearch/issues/683) attr bit(N) engine='columnar' 失败。
* [Issue #682](https://github.com/manticoresoftware/manticoresearch/issues/682) 创建表失败，但留下目录。
* [Issue #663](https://github.com/manticoresoftware/manticoresearch/issues/663) 配置失败，错误信息：未知键名 'attr_update_reserve'。
* [Issue #632](https://github.com/manticoresoftware/manticoresearch/issues/632) Manticore 在批量查询时崩溃。
* [Issue #679](https://github.com/manticoresoftware/manticoresearch/issues/679) 批量查询再次导致崩溃，版本为 v4.0.3。
* [Commit f7f8](https://github.com/manticoresoftware/manticoresearch/commit/f7f8bd8c0dbb23a59a88502dcba16b0918fb2330) 修复了由于尝试使用无效节点列表重新加入集群而导致的守护进程启动崩溃
* [Issue #643](https://github.com/manticoresoftware/manticoresearch/issues/643) Manticore 4.0.2 在批量插入后不接受连接
* [Issue #635](https://github.com/manticoresoftware/manticoresearch/issues/635) 带有 ORDER BY JSON.field 或字符串属性的 FACET 查询可能会崩溃
* [Issue #634](https://github.com/manticoresoftware/manticoresearch/issues/634) 使用 packedfactors 查询时崩溃 SIGSEGV
* [Commit 4165](https://github.com/manticoresoftware/manticoresearch/commits/41657f15f8fcf1e93cfad7f835bcf705d6e8349e) morphology_skip_fields 在创建表时不受支持

## 版本 4.0.2，2021年9月21日

### 主要新功能
- **全面支持 [Manticore Columnar Library](https://github.com/manticoresoftware/columnar)**。之前 Manticore Columnar Library 仅支持普通索引。现在它被支持：
  - 在实时索引中用于 `INSERT`、`REPLACE`、`DELETE`、`OPTIMIZE`
  - 在复制过程中
  - 在 `ALTER` 中
  - 在 `indextool --check` 中
- **自动索引压缩** ([Issue #478](https://github.com/manticoresoftware/manticoresearch/issues/478))。终于不必手动或通过定时任务或其他自动化方式调用 OPTIMIZE。Manticore 现在自动为您处理，且默认启用。您可以通过 [optimize_cutoff](../Server_settings/Setting_variables_online.md) 全局变量设置默认压缩阈值。
- **块快照和锁定系统的改进**。这些更改从外部看起来可能是不可见的，但它们显著改善了实时索引中许多事务的行为。简而言之，以前大多数 Manticore 数据操作严重依赖锁，现在我们使用磁盘块快照代替。
- **实时索引的批量 INSERT 性能显著提高**。例如，在 [Hetzner 的 AX101 服务器](https://www.hetzner.com/dedicated-rootserver/ax101) 上，配备 SSD、128 GB RAM 和 AMD 的 Ryzen™ 9 5950X (16*2 核心) **在 3.6.0 中，您可以每秒插入 236K 文档**到模式为 `name text, email string, description text, age int, active bit(1)` 的表中（默认 `rt_mem_limit`，批量大小 25000，16 个并发插入工作线程，共计插入 1600 万个文档）。在 4.0.2 中，同样的并发/批次/计数可实现 **每秒 357K 文档**。

  <details>

  - 读取操作（例如 SELECTs、复制）通过快照执行
  - 仅改变内部索引结构而不修改模式/文档的操作（例如，合并 RAM 段、保存磁盘块、合并磁盘块）通过只读快照执行，并在最后替换现有块
  - UPDATE 和 DELETE 针对现有块执行，但在可能发生合并的情况下，写入会被收集，然后应用于新的块
  - UPDATE 为每个块依次获取独占锁。合并在进入收集块属性阶段时获取共享锁。因此，同一时间只有一个（合并或更新）操作可以访问该块的属性。
  - 当合并进入需要属性的阶段时，设置一个特殊标志。当 UPDATE 完成时，检查该标志，如果被设置，则整个更新存储在特殊集合中。最后，当合并完成时，将更新应用于新生的磁盘块。
  - ALTER 通过独占锁运行
  - 复制作为常规读取操作，但额外保存属性在 SST 之前并禁止在 SST 期间更新

  </details>

- **[ALTER](Updating_table_schema_and_settings.md) 可以添加/删除全文字段**（在 RT 模式下）。以前只能添加/删除属性。
- 🔬 **实验性：用于全扫描查询的伪分片** - 允许并行化任何非全文搜索查询。您现在只需启用新选项 [searchd.pseudo_sharding](Server_settings/Searchd.md#pseudo_sharding)，便可期待非全文搜索查询的响应时间最多降低 `CPU cores`。请注意，它可能会占用所有现有的 CPU 核心，因此如果您不仅关心延迟，还关心吞吐量 - 请谨慎使用。

### 小改动
<!-- example -->
- Linux Mint 和 Ubuntu Hirsute Hippo 通过 [APT repository](Installation/Debian_and_Ubuntu.md#APT-repository) 支持
- 在某些情况下，通过 HTTP 在大索引中更快地按 ID 更新（取决于 ID 分布）
- [671e65a2](https://github.com/manticoresoftware/lemmatizer-uk/commit/671e65a296800537123a3f8b324eeaa3f58b9632) - 向 lemmatizer-uk 添加了缓存


<!-- intro -->
#### 3.6.0

<!-- request 3.6.0 -->
```
time curl -X POST -d '{"update":{"index":"idx","id":4611686018427387905,"doc":{"mode":0}}}' -H "Content-Type: application/x-ndjson" http://127.0.0.1:6358/json/bulk

real    0m43.783s
user    0m0.008s
sys     0m0.007s
```

<!-- intro -->
#### 4.0.2

<!-- request 4.0.2 -->
```
time curl -X POST -d '{"update":{"index":"idx","id":4611686018427387905,"doc":{"mode":0}}}' -H "Content-Type: application/x-ndjson" http://127.0.0.1:6358/json/bulk

real    0m0.006s
user    0m0.004s
sys     0m0.001s
```
<!-- end -->
- [custom startup flags for systemd](Starting_the_server/Linux.md#Custom-startup-flags-using-systemd)。现在您不需要手动启动 searchd，如果需要使用特定的启动标志运行 Manticore 
- 新函数 [LEVENSHTEIN()](Functions/String_functions.md#LEVENSHTEIN%28%29) 计算 Levenshtein 距离
- 新增 [searchd 启动标志](Starting_the_server/Manually.md#searchd-command-line-options) `--replay-flags=ignore-trx-errors` 和 `--replay-flags=ignore-all-errors`，以便在 binlog 损坏时仍能启动 searchd
- [Issue #621](https://github.com/manticoresoftware/manticoresearch/issues/621) - 暴露来自 RE2 的错误
- 对于由本地简单索引组成的分布式索引，提供更准确的 [COUNT(DISTINCT)](Searching/Grouping.md#COUNT%28DISTINCT-field%29)
- [FACET DISTINCT](Searching/Faceted_search.md#Faceting-without-duplicates) 在进行分面搜索时移除重复项
- [精确形式修饰符](Searching/Full_text_matching/Operators.md#Exact-form-modifier) 现在不需要 [形态学](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology)，并且适用于启用 [插入/前缀](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) 搜索的索引

### 重大变更
- 新版本可以读取旧版本索引，但旧版本无法读取 Manticore 4 的索引
- 移除了按 ID 隐式排序。如果需要，明确进行排序
- `charset_table` 的默认值从 `0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451` 更改为 `non_cjk`
- `OPTIMIZE` 会自动进行。如果不需要，请确保在配置文件的 `searchd` 部分设置 `auto_optimize=0`
- [Issue #616](https://github.com/manticoresoftware/manticoresearch/issues/616) `ondisk_attrs_default` 已被弃用，现在已移除
- 对于贡献者：我们现在在 Linux 构建中使用 Clang 编译器，因为我们的测试表明它可以构建更快的 Manticore Search 和 Manticore Columnar Library
- 如果在搜索查询中未指定 [max_matches](Searching/Options.md#max_matches)，则其会隐式更新为新列存储所需的最低值，以提高性能。这可能会影响 [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META) 中的度量 `total`，但不会影响实际找到文档数量的 `total_found`。

### 从 Manticore 3 迁移
- 确保您干净地停止 Manticore 3：
  - `/var/lib/manticore/binlog/` 中不应存在 binlog 文件（该目录中仅应有 `binlog.meta`）
  - 否则，Manticore 4 无法回复的索引将无法运行
- 新版本可以读取旧版本索引，但旧版本无法读取 Manticore 4 的索引，因此确保备份，如果您希望能够轻松回滚新版本
- 如果您运行一个复制集群，请确保您：
  - 首先干净地停止所有节点
  - 然后使用 `--new-cluster` 启动最后停止的节点（在 Linux 中运行工具 `manticore_new_cluster`）。
  - 阅读关于 [重启集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster) 的详细信息

### Bug 修复
- 许多复制问题已得到修复：
  - [Commit 696f](https://github.com/manticoresoftware/manticoresearch/commit/696f8649535f2b5285b878c9581c0d2b1139ae09) - 修复了在联接处进行 SST 时崩溃；在写入文件块时在联接节点添加了 SHA1 验证以加快索引加载；在索引加载时在联接节点添加了变化索引文件的轮转；当活动索引被来自捐赠节点的新索引替换时，在联接节点添加了移除索引文件；在捐赠节点添加了用于发送文件和块的复制日志点
  - [Commit b296](https://github.com/manticoresoftware/manticoresearch/commit/b296c55af68314a8fc66f8b104e7301d673c3b68) - 在地址不正确的情况下的 JOIN CLUSTER 崩溃
  - [Commit 418b](https://github.com/manticoresoftware/manticoresearch/commit/418bf880a5e05ffbb68f311a1c23006a9df3220e) - 当大索引的初始复制时，联接节点可能会出现 `ERROR 1064 (42000): invalid GTID, (null)` 的错误；在另一个节点加入时，捐赠节点可能变得无响应
  - [Commit 6fd3](https://github.com/manticoresoftware/manticoresearch/commit/6fd350d28e4905211cf8a7298cbf7cdda101c83d) - 对于大索引，哈希计算可能会错误，这可能导致复制失败
  - [Issue #615](https://github.com/manticoresoftware/manticoresearch/issues/615) - 集群重启时复制失败
- [Issue #574](https://github.com/manticoresoftware/manticoresearch/issues/574) - `indextool --help` 不显示参数 `--rotate`
- [Issue #578](https://github.com/manticoresoftware/manticoresearch/issues/578) - searchd 在闲置时的高 CPU 使用率，约一天后
- [Issue #587](https://github.com/manticoresoftware/manticoresearch/issues/587) - 立即刷新 .meta
- [Issue #617](https://github.com/manticoresoftware/manticoresearch/issues/617) - manticore.json 被清空
- [Issue #618](https://github.com/manticoresoftware/manticoresearch/issues/618) - searchd --stopwait 在 root 下失败。这也修复了 systemctl 行为（之前它在 ExecStop 时显示失败，并且没有足够长的时间等待 searchd 正常停止）
- [Issue #619](https://github.com/manticoresoftware/manticoresearch/issues/619) - INSERT/REPLACE/DELETE 与 SHOW STATUS。`command_insert`、`command_replace` 以及其他指标显示错误的指标
- [Issue #620](https://github.com/manticoresoftware/manticoresearch/issues/620) - 简单索引的 `charset_table` 默认值错误
- [Commit 8f75](https://github.com/manticoresoftware/manticoresearch/commit/8f7536887d4b58a5bab5647fc067b2d0482c2038) - 新的磁盘块未被 mlocked
- [Issue #607](https://github.com/manticoresoftware/manticoresearch/issues/607) - 当无法通过名称解析节点时 Manticore 集群节点崩溃
- [Issue #623](https://github.com/manticoresoftware/manticoresearch/issues/623) - 更新索引的复制可能导致不确定状态
- [Commit ca03](https://github.com/manticoresoftware/manticoresearch/commit/ca03d2280c5197e9b311f0f03f551e3b702a8130) - 索引器在为具有 json 属性的简单索引源建立索引时可能会挂起
- [Commit 53c7](https://github.com/manticoresoftware/manticoresearch/commit/53c753054558ff3f82c7af365273d32537c5338b) - 修复了 PQ 索引不等表达式过滤器
- [Commit ccf9](https://github.com/manticoresoftware/manticoresearch/commit/ccf94e022ba6763d6286ab5bac767b6f6a846846) - 修复在匹配超过1000的查询列表时选择窗口。 `SELECT * FROM pq ORDER BY id desc LIMIT 1000 , 100 OPTION max_matches=1100` 之前无法正常工作
- [Commit a048](https://github.com/manticoresoftware/manticoresearch/commit/a0483fe9bef3c45dffe96f5df4f4382aee29c072) - 对Manticore的HTTPS请求可能会导致像“最大包大小(8388608)超出”的警告
- [Issue #648](https://github.com/manticoresoftware/manticoresearch/issues/648) - Manticore 3在对字符串属性进行几次更新后可能会挂起


## 版本 3.6.0，2021年5月3日
**在Manticore 4之前的维护发布**

### 主要新功能
- 对于普通索引，支持[Manticore列库](https://github.com/manticoresoftware/columnar/)。为普通索引添加新的设置[columnar_attrs](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#columnar_attrs)
- 支持[乌克兰语词形还原工具](https://github.com/manticoresoftware/lemmatizer-uk)
- 完全修订的直方图。在构建索引时，Manticore还为每个字段构建直方图，然后它用这些直方图进行更快速的过滤。在3.6.0中，算法进行了全面修订，如果您的数据量很大并且进行了大量过滤，则可以获得更高的性能。

### 小变更
- 工具 `manticore_new_cluster [--force]` 对通过systemd重启复制集群有用
- [--drop-src](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Merging_tables.md#Merging-tables) 用于`indexer --merge`
- [新模式](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) `blend_mode='trim_all'`
- 添加了对使用反引号转义JSON路径的[支持](Searching/Full_text_matching/Escaping.md#Escaping-json-node-names-in-SQL)
- [indextool --check](Miscellaneous_tools.md#indextool) 可以在RT模式下工作
- [FORCE/IGNORE INDEX(id)](Data_creation_and_modification/Updating_documents/UPDATE.md#Updates-via-SQL) 用于SELECT/UPDATE
- 合并的磁盘块的块ID现在是唯一的
- [indextool --check-disk-chunk CHUNK_NAME](Miscellaneous_tools.md#indextool)

### 优化
- [更快的JSON解析](https://github.com/manticoresoftware/manticoresearch/commit/0c25fc1788e3f77f60cb7fb8f1623d8466bba606)，我们的测试表明，查询如`WHERE json.a = 1`的延迟降低了3-4%
- 非文档命令 `DEBUG SPLIT` 是自动分片/重平衡的前提条件

### 错误修复
- [Issue #584](https://github.com/manticoresoftware/manticoresearch/issues/524) - 不准确和不稳定的FACET结果
- [Issue #506](https://github.com/manticoresoftware/manticoresearch/issues/506) - 使用MATCH时的奇怪行为：受到此问题影响的用户需要重建索引，因为问题出现在构建索引的阶段
- [Issue #387](https://github.com/manticoresoftware/manticoresearch/issues/387) - 在运行带有SNIPPET()功能的查询时的间歇性核心转储
- 处理复杂查询时有用的栈优化：
  - [Issue #469](https://github.com/manticoresoftware/manticoresearch/issues/469) - SELECT结果导致崩溃转储
  - [e8420cc7](https://github.com/manticoresoftware/manticoresearch/tree/e8420cc789149c84731be0874be916e33b263bd6) - 用于过滤树的栈大小检测
- [Issue #461](https://github.com/manticoresoftware/manticoresearch/issues/461) - 使用IN条件的更新未能正确生效
- [Issue #464](https://github.com/manticoresoftware/manticoresearch/issues/464) - CALL PQ后立即SHOW STATUS返回 - [Issue #481](https://github.com/manticoresoftware/manticoresearch/issues/481) - 修复静态二进制构建
- [Issue #502](https://github.com/manticoresoftware/manticoresearch/issues/502) - 多查询中的错误
- [Issue #514](https://github.com/manticoresoftware/manticoresearch/issues/514) - 使用“create table”时无法使用不寻常的列名
- [Commit d1db](https://github.com/manticoresoftware/manticoresearch/commit/d1dbe771288328c963ca8512f564c6b7cf4f466c) - 在重放binlog时字符串属性更新的守护进程崩溃；将binlog版本设置为10
- [Commit 775d](https://github.com/manticoresoftware/manticoresearch/commit/775d0555562f121911d3c30085947e743fadac2d)  - 修复表达式栈帧检测运行时（测试207）
- [Commit 4795](https://github.com/manticoresoftware/manticoresearch/commit/4795dc49194c7745761bb5006ecb266e8a9224fa)  - 针对空存储查询，渗透索引过滤器和标签为空（测试369）
- [Commit c3f0](https://github.com/manticoresoftware/manticoresearch/commit/c3f0bf4dca0e4eae6ea899dd566878df57eba085)  - 在网络延迟较长且错误率高的情况下破坏复制SST流（不同数据中心的复制）；将复制命令版本更新为1.03
- [Commit ba2d](https://github.com/manticoresoftware/manticoresearch/commit/ba2d6619b8107f3fb177741bbdff3709bac248cd)  - 加入者在写操作后锁定集群以加入集群（测试385）
- [Commit de4d](https://github.com/manticoresoftware/manticoresearch/commit/de4dcb9ff6c316bc2a9ef6c62eb64769f04ee103)  - 带有精确修饰符的通配符匹配（测试321）
- [Commit 6524](https://github.com/manticoresoftware/manticoresearch/commit/6524fc6af032a0932bd42c37874a619e1fb7f075)  - docid检查点与docstore
- [Commit f4ab](https://github.com/manticoresoftware/manticoresearch/commit/f4ab83c291e65a5873d271c67b5491f629544f1d)  - 解析无效xml时，索引器行为不一致
- [Commit 7b72](https://github.com/manticoresoftware/manticoresearch/commit/7b727e22fe01b1a180d4a8d3a3fea92c18eac707)  - 带有NOTNEAR的存储渗透查询运行时间过长（测试349）
- [Commit 812d](https://github.com/manticoresoftware/manticoresearch/commit/812dab745f0c972ecde5d5b3d4a313cd6e1e143e)  - 对以通配符开头的短语的错误权重
- [提交 1771](https://github.com/manticoresoftware/manticoresearch/commit/1771afc669c75670b3511bf02ab9bfc8982b94d2)  - 具有通配符的渗透查询在匹配时生成没有有效负载的术语，这导致交错的命中并中断匹配（测试 417）
- [提交 aa0d](https://github.com/manticoresoftware/manticoresearch/commit/aa0d8c2bcb41428d41dbfb66e42a872a79a4ee4b)  - 修复并行查询时“总计”的计算
- [提交 18d8](https://github.com/manticoresoftware/manticoresearch/commit/18d81b3cfbb930b7361dae645fe13283d4fbf6ef)  - 在 Windows 上处理多个并发会话时崩溃
- [提交 8443](https://github.com/manticoresoftware/manticoresearch/commit/84432f23d6fbf7a7d0b67c8c9412eb5933b03596)  - 某些索引设置无法被复制
- [提交 9341](https://github.com/manticoresoftware/manticoresearch/commit/93411fe6af4246ca4acce758944d513a1660b92b)  - 在高频率添加新事件时，netloop 有时会因原子“踢”事件被处理多次而冻结，从而丢失实际操作
查询的状态，而不是服务器的状态
- [提交 d805](https://github.com/manticoresoftware/manticoresearch/commit/d805fc125244ef08d14fb3d1826e2c356862d539)  - 新清空的磁盘块可能在提交时丢失
- [提交 63cb](https://github.com/manticoresoftware/manticoresearch/commit/63cbf008d50979cdf7062664fb02a672a3bb1131)  - 分析器中的“不准确”的“net_read”
- [提交 f537](https://github.com/manticoresoftware/manticoresearch/commit/f5379bb27234b25919966c50ccb6f469ead646e2)  - 阿拉伯文（从右到左文本）中的渗透问题
- [提交 49ee](https://github.com/manticoresoftware/manticoresearch/commit/49eeb4202d4ce4875c07c35087d7108c1e177328)  - 在重复的列名上未正确选择 id
- [提交 refa](https://github.com/manticoresoftware/manticoresearch/commit/4b7df262bbfec9b9e95a44e72ad326ac8a786c49) 的网络事件，以修复罕见情况下的崩溃
- [e8420cc7](https://github.com/manticoresoftware/manticoresearch/tree/e8420cc789149c84731be0874be916e33b263bd6) 在 `indextool --dumpheader` 中的修复
- [提交 ff71](https://github.com/manticoresoftware/manticoresearch/commit/ff7163532ed39042c0d3c3c2b902ff41f94d7f0c) - TRUNCATE WITH RECONFIGURE 在存储字段时错误地工作

### 重大变化：
- 新的 binlog 格式：在升级之前，您需要干净地停止 Manticore
- 索引格式稍有变化：新版本可以很好地读取您现有的索引，但如果您决定从 3.6.0 降级到旧版本，则较新的索引将无法读取
- 复制格式更改：不要从旧版本复制到 3.6.0 及其反之亦然，请在所有节点上同时切换到新版本
- `reverse_scan` 已被弃用。确保您自 3.6.0 以来没有在查询中使用该选项，因为它们会失败
- 从此版本开始，我们不再为 RHEL6、Debian Jessie 和 Ubuntu Trusty 提供构建。如果您需要它们得到支持，请 [联系我们](https://manticoresearch.com/contact-us/)

### 弃用
- 不再隐式按 id 排序。如果您依赖于它，请确保相应地更新您的查询
- 搜索选项 `reverse_scan` 已被弃用

## 版本 3.5.4，2020年12月10日

### 新特性
- 新的 Python、Javascript 和 Java 客户端现已普遍可用，并在本手册中有良好文档。
- 自动丢弃 real-time 索引的磁盘块。这项优化允许在 [OPTIMIZing](Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE) 实时索引时自动丢弃磁盘块，当块显然不再需要时（所有文档都被压制）。以前仍然需要合并，现在块可以瞬间丢弃。 [cutoff](Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks) 选项被忽略，即使没有实际合并，也会删除过时的磁盘块。这在您保持索引的保留并删除旧文档时非常有用。现在压缩这样的索引会更快。
- [独立 NOT](Searching/Options.md#not_terms_only_allowed) 作为 SELECT 的一个选项

### 次要变化
- [问题 #453](https://github.com/manticoresoftware/manticoresearch/issues/453) 新选项 [indexer.ignore_non_plain=1](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#ignore_non_plain) 在您运行 `indexer --all` 并且配置文件中不仅有普通索引的情况下非常有用。没有 `ignore_non_plain=1`，您将收到警告和相应的退出代码。
- [SHOW PLAN ... OPTION format=dot](Node_info_and_management/Profiling/Query_plan.md#Dot-format-for-SHOW-PLAN) 和 [EXPLAIN QUERY ... OPTION format=dot](Searching/Full_text_matching/Profiling.md#Profiling-without-running-a-query) 可以可视化全文查询计划执行。对于理解复杂查询很有用。

### 弃用
- `indexer --verbose` 已被弃用，因为它从未对索引器输出添加任何内容
- 用于转储看门狗的回溯信号 `USR2` 现在将替代 `USR1`

### 错误修复
- [问题 #423](https://github.com/manticoresoftware/manticoresearch/issues/423) 斯拉夫字符的句点调用片段保留模式不突出显示
- [问题 #435](https://github.com/manticoresoftware/manticoresearch/issues/435) RTINDEX - GROUP N BY 表达式选择 = 致命崩溃
- [提交 2b3b](https://github.com/manticoresoftware/manticoresearch/commit/2b3b62bd0ce2b54544733d524a169e81f71b5388) searchd 状态在集群中显示段错误
- [提交 9dd2](https://github.com/manticoresoftware/manticoresearch/commit/9dd25c193ed2a53750a0e0e68566fd2b833fcb0c) 'SHOW INDEX index.N SETTINGS' 不处理块 >9
- [问题 #389](https://github.com/manticoresoftware/manticoresearch/issues/389) 崩溃 Manticore 的错误
- [提交 fba1](https://github.com/manticoresoftware/manticoresearch/commit/fba166178ff35c73a0c70d131cf3aca4a635e961) 转换器创建损坏的索引
- [Commit eecd](https://github.com/manticoresoftware/manticoresearch/commit/eecd61d861c742471175be19a5e5455034b5db5f) stopword_step=0 与 CALL SNIPPETS()
- [Commit ea68](https://github.com/manticoresoftware/manticoresearch/commit/ea6850e449601199190f38a387c04b8a7b7e90f6) 在本地索引中，低 max_matches 时 count distinct 返回 0
- [Commit 362f](https://github.com/manticoresoftware/manticoresearch/commit/362f27db3421f09a41c6cce04b5baace00fa4c4d) 使用聚合时，不会在命中结果中返回存储文本


## 版本 3.5.2，2020年10月1日

### 新特性

* OPTIMIZE 将磁盘块减少到若干个块（默认值为 `2*核心数量`），而非单个块。最佳块数量可以通过 [cutoff](Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks) 选项进行控制。
* NOT 运算符现在可以单独使用。默认情况下它被禁用，因为意外的单个 NOT 查询可能会很慢。可以通过将新的 searchd 指令 [not_terms_only_allowed](Server_settings/Searchd.md#not_terms_only_allowed) 设置为 `0` 来启用它。
* 新设置 [max_threads_per_query](Server_settings/Searchd.md#max_threads_per_query) 设置一个查询可以使用多少线程。如果未设置该指令，则查询可以使用的线程数最多为 [threads](Server_settings/Searchd.md#threads) 的值。
每个 `SELECT` 查询的线程数可以通过 [OPTION threads=N](Searching/Options.md#threads) 来限制，从而覆盖全局的 `max_threads_per_query`。
* 现在可以通过 [IMPORT TABLE](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Importing_table.md) 导入过濾索引。
* HTTP API `/search` 收到对 [分面](Searching/Faceted_search.md#HTTP-JSON)/[分组](Searching/Grouping.md) 的基本支持，新的查询节点为 `aggs`。

### 次要更改

* 如果未声明复制监听指令，发动机会尝试使用在定义的 'sphinx' 端口之后的端口，最多到 200。
* `listen=...:sphinx` 需要明确设置用于 SphinxSE 连接或 SphinxAPI 客户端。
* [SHOW INDEX STATUS](Node_info_and_management/Table_settings_and_status/SHOW_TABLE_STATUS.md) 输出新指标：`killed_documents`，`killed_rate`，`disk_mapped_doclists`，`disk_mapped_cached_doclists`，`disk_mapped_hitlists` 和 `disk_mapped_cached_hitlists`。
* SQL 命令 `status` 现在输出 `Queue\Threads` 和 `Tasks\Threads`。

### 弃用：

* `dist_threads` 现在被完全弃用，如果仍使用该指令，searchd 将记录警告。

### Docker

官方的 Docker 镜像现在基于 Ubuntu 20.04 LTS

### 打包

除了一般的 `manticore` 包，您还可以按组件安装 Manticore Search：

- `manticore-server-core` - 提供 `searchd`，手册页，日志目录，API 和 galera 模块。它也会将 `manticore-common` 作为依赖项安装。
- `manticore-server` - 提供核心的自动化脚本（init.d，systemd），以及 `manticore_new_cluster` 包装器。它也会将 `manticore-server-core` 作为依赖项安装。
- `manticore-common` - 提供配置，停止词，通用文档和骨架文件夹（datadir，modules 等）。
- `manticore-tools` - 提供辅助工具（`indexer`，`indextool` 等），它们的手册页和示例。它也会将 `manticore-common` 作为依赖项安装。
- `manticore-icudata` (RPM) 或 `manticore-icudata-65l` (DEB) - 提供 ICU 数据文件以用于 icu 形态学。
- `manticore-devel` (RPM) 或 `manticore-dev` (DEB) - 提供 UDF 的开发头文件。

### 错误修复

1. [Commit 2a47](https://github.com/manticoresoftware/manticoresearch/commit/2a474dc1a26e8b0f8aaaae95669caf2f1d4b7746) 在 RT 索引中，处理不同块时，守护进程崩溃
2. [Commit 57a1](https://github.com/manticoresoftware/manticoresearch/commit/57a19e5ad5663ef0ca7436595218fb1221d28c8e) 空的远程文档的快速路径
3. [Commit 07dd](https://github.com/manticoresoftware/manticoresearch/commit/07dd3f313c63fb82c22092f9907ef24e3475250e) 表达栈帧检测运行时
4. [Commit 08ae](https://github.com/manticoresoftware/manticoresearch/commit/08ae357cf1012bc8e2da54c20b205b592efda3d4) 在过濾索引中匹配超过 32 个字段
5. [Commit 16b9](https://github.com/manticoresoftware/manticoresearch/commit/16b9390fd4cdb07a77ac4497adb935573a1710e5) 复制监听端口范围
6. [Commit 5fa6](https://github.com/manticoresoftware/manticoresearch/commit/5fa671affeacb6441a59c8a88479bfd423df7c81) 在 pq 上显示创建表
7. [Commit 54d1](https://github.com/manticoresoftware/manticoresearch/commit/54d133b6449105a9fb0168db3f1fbb05fb5aa1f6) HTTPS 端口行为
8. [Commit fdbb](https://github.com/manticoresoftware/manticoresearch/commit/fdbbe5245cc296cc5c1ae3ae2fb9cb08fb66a248) 替换时混合文档存储行
9. [Commit afb5](https://github.com/manticoresoftware/manticoresearch/commit/afb53f648ef4e64fa1776c58e66f6e716ac730ad) 将 TFO 不可用消息的级别切换为 'info'
10. [Commit 59d9](https://github.com/manticoresoftware/manticoresearch/commit/59d94cefc6e88af360b8046726a16b7eaa9f0b71) strcmp 无效使用时崩溃
11. [Commit 04af](https://github.com/manticoresoftware/manticoresearch/commit/04af0349c1e784c2230d204592d6272b440bb375) 向集群添加带系统（停止词）文件的索引
12. [Commit 5014](https://github.com/manticoresoftware/manticoresearch/commit/50148b4eae9f3a924082005cf5d2f9c8339af7f5) 合并具有大型字典的索引；RT 优化大型磁盘块
13. [Commit a2ad](https://github.com/manticoresoftware/manticoresearch/commit/a2adf1582843f63d4dcc1dab8f7598e0917094a2) Indextool 可以从当前版本导出元数据
14. [Commit 69f6](https://github.com/manticoresoftware/manticoresearch/commit/69f6d5f77c64b97e3f395aea8c555b11c77fb732) 在 GROUP N 中组顺序的问题
15. [Commit 24d5](https://github.com/manticoresoftware/manticoresearch/commit/24d5d80f508576794b751d70f9f9b524564e3c14) SphinxSE 在握手后显式刷新
16. [Commit 31c4](https://github.com/manticoresoftware/manticoresearch/commit/31c4d78ac03ee85d9a02b86075448a04ac80f9e2) 避免在不必要时复制巨大的描述
17. [Commit 2959](https://github.com/manticoresoftware/manticoresearch/commit/2959e2caa8610f98fcdf2e5a82160f58687f4a27) 显示线程中的负时间
18. [Commit f0b3](https://github.com/manticoresoftware/manticoresearch/commit/f0b357102c70f3ac1d7da260c60b67322940e647) 令牌过滤器插件与零位置增量
19. [Commit a49e](https://github.com/manticoresoftware/manticoresearch/commit/a49e5bc13ea763e6f01dd5993e891858a84d1846) 将多个命中的'FAIL'更改为'WARNING'

## 版本 3.5.0，2020年7月22日

### 主要新特性：
* 此版本耗时很长，因为我们努力将多任务模式从线程更改为 **协程**。这使配置变得更简单，查询并行化也更加直接：Manticore 只使用给定数量的线程（参见新设置 [threads](Server_settings/Searchd.md#threads)），并且新模式确保以最优方式完成。
* [高亮](Searching/Highlighting.md#Highlighting-options)的变化：
  - 任何与多个字段一起工作的高亮（`highlight({},'field1, field2'` 或 `highlight` 在 json 查询中）现在默认按字段应用限制。
  - 任何与普通文本一起工作的高亮（`highlight({}, string_attr)` 或 `snippet()` 现在对整个文档应用限制。
  - [按字段限制](Searching/Highlighting.md#limits_per_field)可以通过 `limits_per_field=0` 选项切换为全局限制（默认值为 `1`）。
  - 通过 HTTP JSON 进行高亮的 [allow_empty](Searching/Highlighting.md#allow_empty) 现在默认值为 `0`。

* 现在可以将同一端口 [用于](Server_settings/Searchd.md#listen) http, https 和二进制 API（以接受来自远程 Manticore 实例的连接）。仍然需要`listen = *:mysql`以通过 mysql 协议进行连接。Manticore 现在自动检测尝试连接到它的客户端类型，除了 MySQL（由于协议的限制）。

* 在实时模式下，字段现在可以同时是 [文本和字符串属性](Creating_a_table/Data_types.md#String) - [GitHub 问题 #331](https://github.com/manticoresoftware/manticoresearch/issues/331)。

  在 [普通模式](Read_this_first.md#Real-time-mode-vs-plain-mode) 中称为 `sql_field_string`。现在它也在 [实时模式](Read_this_first.md#Real-time-mode-vs-plain-mode) 中可用于实时索引。您可以按如下示例使用它：

  <!-- more -->
  ```sql
  create table t(f string attribute indexed);
  insert into t values(0,'abc','abc');
  select * from t where match('abc');
  +---------------------+------+
  | id                  | f    |
  +---------------------+------+
  | 2810845392541843463 | abc  |
  +---------------------+------+
  1 row in set (0.01 sec)

  mysql> select * from t where f='abc';
  +---------------------+------+
  | id                  | f    |
  +---------------------+------+
  | 2810845392541843463 | abc  |
  +---------------------+------+
  1 row in set (0.00 sec)
  ```
  <!-- \more -->

### 次要变化
* 您现在可以 [高亮字符串属性](Searching/Highlighting.md#Highlighting-via-SQL)。
* SQL 接口的 SSL 和压缩支持
* 支持 mysql 客户端 [`status`](Node_info_and_management/Node_status.md#STATUS) 命令。
* [复制](Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Setting-up-replication)现在可以复制外部文件（停用词、例外等）。
* 过滤器运算符 [`in`](Searching/Filters.md#Set-filters) 现在可以通过 HTTP JSON 接口使用。
* HTTP JSON 中的 [`expressions`](Searching/Expressions.md#expressions)。
* [您现在可以动态更改 `rt_mem_limit`](https://github.com/manticoresoftware/manticoresearch/issues/344) 在实时模式中，即可以执行 `ALTER ... rt_mem_limit=<new value>`。
* 现在可以使用 [单独的 CJK 字符集表](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology)： `chinese`, `japanese` 和 `korean`。
* [thread_stack](Server_settings/Searchd.md#thread_stack) 现在限制最大线程栈，而不是初始。
* 改进了 `SHOW THREADS` 的输出。
* 显示长时间 `CALL PQ` 的进度在 `SHOW THREADS` 中。
* cpustat、iostat、coredump 可以在运行时通过 [SET](Server_settings/Setting_variables_online.md#SET) 更改。
* 实现了 `SET [GLOBAL] wait_timeout=NUM`，

### 破坏性变更：
* **索引格式已更改。** 在 3.5.0 中构建的索引不能被 Manticore 版本 < 3.5.0 加载，但 Manticore 3.5.0 理解较旧的格式。
* [`INSERT INTO PQ VALUES()`](Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md)（即不提供列列表）以前恰好期望 `(query, tags)` 作为值。已更改为 `(id,query,tags,filters)`。如果希望它自动生成，则可以将 id 设置为 0。
* [`allow_empty=0`](Searching/Highlighting.md#allow_empty) 是通过 HTTP JSON 接口进行高亮的新默认值。
* 在 `CREATE TABLE` / `ALTER TABLE` 中，仅允许外部文件（停用词、例外等）使用绝对路径。

### 弃用：
* `ram_chunks_count` 在 `SHOW INDEX STATUS` 中重命名为 `ram_chunk_segments_count`。
* `workers` 已过时。现在只有一个工作模式。
* `dist_threads` 已过时。现在所有查询都尽可能并行（受到 `threads` 和 `jobs_queue_size` 的限制）。
* `max_children` 已过时。使用 [threads](Server_settings/Searchd.md#threads) 设置 Manticore 将使用的线程数（默认设置为 CPU 核心的数量）。
* `queue_max_length` 已过时。真正需要时可以使用 [jobs_queue_size](Server_settings/Searchd.md#jobs_queue_size) 来微调内部作业队列大小（默认设置为无限制）。
* 所有 `/json/*` 端点现在可以在没有 `/json/` 的情况下使用，例如 `/search`, `/insert`, `/delete`, `/pq` 等。
* `field` 的含义 "全文字段" 在 `describe` 中被重命名为 "text"。
  <!-- more -->
  **3.4.2:**
  ```sql
  mysql> describe t;
  +-------+--------+----------------+
  | Field | Type   | Properties     |
  +-------+--------+----------------+
  | id    | bigint |                |
  | f     | field  | indexed stored |
  +-------+--------+----------------+
  ```
  **3.5.0**:
  ```sql
  mysql> describe t;
  +-------+--------+----------------+
  | Field | Type   | Properties     |
  +-------+--------+----------------+
  | id    | bigint |                |
  | f     | text   | indexed stored |
  +-------+--------+----------------+
  ```
  <!-- \more -->
* 俄文字母 `и` 在 `non_cjk` 字符集表中不映射到 `i`（这是默认情况），因为它对俄语词干提取器和词形还原器影响太大。
* `read_timeout`。请改用 [network_timeout](Server_settings/Searchd.md#network_timeout)，该选项同时控制读取和写入。


### 软件包

* Ubuntu Focal 20.04 官方软件包
* deb 软件包名称从 `manticore-bin` 更改为 `manticore`

### 修复程序：
1. [问题 #351](https://github.com/manticoresoftware/manticoresearch/issues/351) searchd 内存泄漏
2. [提交 ceab](https://github.com/manticoresoftware/manticoresearch/commit/ceabe44f) 短小的超出边界读取片段
3. [提交 1c3e](https://github.com/manticoresoftware/manticoresearch/commit/1c3e84a3) 危险的写入到本地变量，导致崩溃查询
4. [提交 26e0](https://github.com/manticoresoftware/manticoresearch/commit/26e094ab) 第 226 测试中的微小内存泄漏
5. [提交 d2c7](https://github.com/manticoresoftware/manticoresearch/commit/d2c7f86a) 第 226 测试中的巨大内存泄漏
6. [提交 0dd8](https://github.com/manticoresoftware/manticoresearch/commit/0dd80122) 集群显示节点同步，但 `count(*)` 显示不同的数字
7. [提交 f1c1](https://github.com/manticoresoftware/manticoresearch/commit/f1c1ac3f) 美化：日志中重复且有时丢失的警告信息
8. [提交 f1c1](https://github.com/manticoresoftware/manticoresearch/commit/f1c1ac3f) 美化：日志中的 (null) 索引名称
9. [提交 359d](https://github.com/manticoresoftware/manticoresearch/commit/359dbd30) 无法检索超过 70M 的结果
10. [提交 19f3](https://github.com/manticoresoftware/manticoresearch/commit/19f328ee) 无法使用无列语法插入 PQ 规则
11. [提交 bf68](https://github.com/manticoresoftware/manticoresearch/commit/bf685d5d) 向集群中的索引插入文档时误导性的错误信息
12. [提交 2cf1](https://github.com/manticoresoftware/manticoresearch/commit/2cf18c83) `/json/replace` 和 `json/update` 返回科学计数法的 id
13. [问题 #324](https://github.com/manticoresoftware/manticoresearch/issues/324) 在同一查询中更新 json 标量属性和 mva
14. [提交 d384](https://github.com/manticoresoftware/manticoresearch/commit/d38409eb) `hitless_words` 在实时模式下不工作
15. [提交 5813](https://github.com/manticoresoftware/manticoresearch/commit/5813d639) 在实时模式下应禁止 `ALTER RECONFIGURE`
16. [提交 5813](https://github.com/manticoresoftware/manticoresearch/commit/5813d639) `rt_mem_limit` 在 searchd 重启后重置为 128M
17. highlight() 有时挂起
18. [提交 7cd8](https://github.com/manticoresoftware/manticoresearch/commit/7cd878f4) 在实时模式下无法使用 U+code
19. [提交 2b21](https://github.com/manticoresoftware/manticoresearch/commit/2b213de4) 在实时模式下无法在词形中使用通配符
20. [提交 e9d0](https://github.com/manticoresoftware/manticoresearch/commit/e9d07e68) 修复 `SHOW CREATE TABLE` 与多个词形文件冲突
21. [提交 fc90](https://github.com/manticoresoftware/manticoresearch/commit/fc90a84f) 无 "query" 的 JSON 查询崩溃 searchd
22. Manticore [官方 docker](https://hub.docker.com/r/manticoresearch/manticore) 无法从 mysql 8 索引数据
23. [提交 23e0](https://github.com/manticoresoftware/manticoresearch/commit/23e05d32) HTTP /json/insert 需要 id
24. [提交 bd67](https://github.com/manticoresoftware/manticoresearch/commit/bd679af0) `SHOW CREATE TABLE` 对于 PQ 不工作
25. [提交 bd67](https://github.com/manticoresoftware/manticoresearch/commit/bd679af0) `CREATE TABLE LIKE` 对于 PQ 无法正常工作
26. [提交 5eac](https://github.com/manticoresoftware/manticoresearch/commit/5eacf28f) 显示索引状态的设置的行末
27. [提交 cb15](https://github.com/manticoresoftware/manticoresearch/commit/cb153228) HTTP JSON 响应中 "highlight" 的空标题
28. [问题 #318](https://github.com/manticoresoftware/manticoresearch/issues/318) `CREATE TABLE LIKE` 中缀错误
29. [提交 9040](https://github.com/manticoresoftware/manticoresearch/commit/9040d22c) 在负载下 RT 崩溃
30. [cd512c7d ](https://github.com/manticoresoftware/manticoresearch/commit/cd512c7d ) 在 RT 磁盘块崩溃时丢失崩溃日志
31. [问题 #323](https://github.com/manticoresoftware/manticoresearch/issues/323) 导入表失败并关闭连接
32. [提交 6275](https://github.com/manticoresoftware/manticoresearch/commit/6275316a) `ALTER reconfigure` 损坏 PQ 索引
33. [提交 9c1d](https://github.com/manticoresoftware/manticoresearch/commit/9c1d221e) 更改索引类型后的 searchd 重载问题
34. [提交 71e2](https://github.com/manticoresoftware/manticoresearch/commit/71e2b5bb) 导入表时因缺失文件而导致守护进程崩溃
35. [问题 #322](https://github.com/manticoresoftware/manticoresearch/issues/322) 使用多索引、分组和 ranker = none 时崩溃
36. [提交 c3f5](https://github.com/manticoresoftware/manticoresearch/commit/c3f58490) `HIGHLIGHT()` 在字符串属性中不高亮
37. [问题 #320](https://github.com/manticoresoftware/manticoresearch/issues/320) `FACET` 在字符串属性上无法排序
38. [Commit 4f1a](https://github.com/manticoresoftware/manticoresearch/commit/4f1a1f25) 缺少数据目录时出现错误
39. [Commit 04f4](https://github.com/manticoresoftware/manticoresearch/commit/04f4ddd4) access_* 在实时模式下不支持
40. [Commit 1c06](https://github.com/manticoresoftware/manticoresearch/commit/1c0616a2) 字符串中的坏 JSON 对象：1. `CALL PQ` 在 JSON 大于某个值时返回 "字符串中的坏 JSON 对象：1"。
41. [Commit 32f9](https://github.com/manticoresoftware/manticoresearch/commit/32f943d6) 实时模式不一致。在某些情况下，我无法删除索引，因为它未知，也无法创建，因为目录不为空。
42. [Issue #319](https://github.com/manticoresoftware/manticoresearch/issues/319) 选择时崩溃
43. [Commit 22a2](https://github.com/manticoresoftware/manticoresearch/commit/22a28dd7) `max_xmlpipe2_field` = 2M 在 2M 字段上返回警告
44. [Issue #342](https://github.com/manticoresoftware/manticoresearch/issues/342) 查询条件执行错误
45. [Commit dd8d](https://github.com/manticoresoftware/manticoresearch/commit/dd8dcab2) 简单的两个术语搜索只找到一个术语的文档
46. [Commit 9091](https://github.com/manticoresoftware/manticoresearch/commit/90919e62) 在 PQ 中无法匹配带有大写字母的 JSON 键
47. [Commit 56da](https://github.com/manticoresoftware/manticoresearch/commit/56da086a) 在 csv+docstore 上索引器崩溃
48. [Issue #363](https://github.com/manticoresoftware/manticoresearch/issues/363) 在 centos 7 中使用 `[null]` 的 JSON 属性导致插入数据损坏
49. 主要 [Issue #345](https://github.com/manticoresoftware/manticoresearch/issues/345) 不插入记录，count() 是随机的，"replace into" 返回 OK
50. max_query_time 使 SELECT 变得过慢
51. [Issue #352](https://github.com/manticoresoftware/manticoresearch/issues/352) Mac OS 上主代理通信失败
52. [Issue #328](https://github.com/manticoresoftware/manticoresearch/issues/328) 使用 Connector.Net/Mysql 8.0.19 连接 Manticore 时出错
53. [Commit daa7](https://github.com/manticoresoftware/manticoresearch/commit/daa760d2fe83668038318f0f384ceefc0d075e18) 修复了 \0 的转义并优化了性能
54. [Commit 9bc5](https://github.com/manticoresoftware/manticoresearch/commit/9bc5c01a493db993b90ff6c1ad51aeae028f629c) 修复了计数不同与 JSON 的问题
55. [Commit 4f89](https://github.com/manticoresoftware/manticoresearch/commit/4f89a965a3af0db548ae9d1763731ebaebe21f5a) 修复了在其他节点上删除表失败的问题
56. [Commit 952a](https://github.com/manticoresoftware/manticoresearch/commit/952af5a5931fd72b87edc169588777778b365231) 修复紧急运行的 PQ 调用崩溃


## 版本 3.4.2, 2020年4月10日
### 关键修复
* [Commit 2ffe](https://github.com/manticoresoftware/manticoresearch/commit/2ffe2d261793f9681265bc4abe38d695e72c121e) 修复 RT 索引从旧版本失败以索引数据

## 版本 3.4.0, 2020年3月26日
### 主要变化
* 服务器在两种模式下工作：实时模式和普通模式
   *   实时模式需要 data_dir 并且在配置中不允许索引定义
   *   在普通模式下，索引在配置中定义；不允许 data_dir
* 复制仅在实时模式下可用

### 次要变更
* charset_table 默认为非 CJK 别名
* 在实时模式下，全文字段默认被索引和存储
* 实时模式中的全文字段名称从 'field' 更改为 'text'
* ALTER RTINDEX 更名为 ALTER TABLE
* TRUNCATE RTINDEX 更名为 TRUNCATE TABLE

### 特性
* 仅存储字段
* SHOW CREATE TABLE, IMPORT TABLE

### 改进
* 无锁 PQ 更快
* /sql 可以在 mode=raw 中执行任何类型的 SQL 语句
* mysql41 协议的 mysql 别名
* 默认 state.sql 在 data_dir 中

### 修复
* [Commit a533](https://github.com/manticoresoftware/manticoresearch/commit/a5333644e8e0c69746a9f24da8221fc965478e4a) 修复 highlight() 中字段语法错误引起的崩溃
* [Commit 7fbb](https://github.com/manticoresoftware/manticoresearch/commit/7fbb9f2e618096289cba7f8b834530b46aa94055) 修复在复制 RT 索引及文档存储时服务器崩溃
* [Commit 24a0](https://github.com/manticoresoftware/manticoresearch/commit/24a04687eb43b4f4719c7c4a5fe3c19099240467) 修复在使用内嵌或前缀选项的 highlight 破坏索引和未启用存储字段的索引时崩溃
* [Commit 3465](https://github.com/manticoresoftware/manticoresearch/commit/3465c1ce9c860c09f6ccc5b83a63e3c68c241d8d) 修复有关空文档存储和空索引的 dock-id 查找的错误
* [Commit a707](https://github.com/manticoresoftware/manticoresearch/commit/b8601b4144b9092be43a7caa7cd6b19cb37e14c7) 修复 #314 SQL 插入命令的尾随分号
* [Commit 9562](https://github.com/manticoresoftware/manticoresearch/commit/95628c9b9b8f16fe1bd18599a84e9673575adf1a) 删除查询单词不匹配的警告
* [Commit b860](https://github.com/manticoresoftware/manticoresearch/commit/b8601b4144b9092be43a7caa7cd6b19cb37e14c7) 修复通过 ICU 分段的 snippets 中的查询
* [Commit 5275](https://github.com/manticoresoftware/manticoresearch/commit/5275516ca4631c584e6e0127fa161a18d2658fd0) 修复文档存储区块缓存中的查找/添加竞争条件
* [Commit f06e](https://github.com/manticoresoftware/manticoresearch/commit/f06ef97af5c7390427e82525c150535290e42c3e) 修复文档存储中的内存泄漏
* [Commit a725](https://github.com/manticoresoftware/manticoresearch/commit/a7258ba8340e246c0d8aaeb4a7a74129eba807eb) 修复 #316 LAST_INSERT_ID 在 INSERT 时返回空
* [Commit 1ebd](https://github.com/manticoresoftware/manticoresearch/commit/1ebd5bf8d74113bcfa138ce63ce11966990a4215) 修复 #317 json/update HTTP 端点以支持 MVA 的数组和 JSON 属性的对象
* [Commit e426](https://github.com/manticoresoftware/manticoresearch/commit/e426950a536382a142e2a42ed207c341149ff685) 修复在没有显式 ID 的情况下索引器转储实时索引的崩溃

## 版本 3.3.0, 2020年2月4日
### 特性
* 并行实时索引搜索
* EXPLAIN QUERY 命令
* 没有索引定义的配置文件（alpha 版本）
* CREATE/DROP TABLE 命令（alpha 版本）
* indexer --print-rt - 可以从源读取并打印实时索引的 INSERTs

### 改进
* 更新到 Snowball 2.0 词干分析器
* SHOW INDEX STATUS 的 LIKE 过滤器
* 针对高 max_matches 改进内存使用
* SHOW INDEX STATUS 为 RT 索引 добавляет ram_chunks_count
* 无锁 PQ
* 将 LimitNOFILE 更改为 65536


### 错误修复
* [Commit 9c33](https://github.com/manticoresoftware/manticoresearch/commit/9c33aab8c1fd9059c8644d23d5f58f03010043c6) 添加重复属性的索引模式检查 #293
* [Commit a008](https://github.com/manticoresoftware/manticoresearch/commit/a0085f9477e191da9e0e835b91583fb9cfde698e) 修复无命中的术语崩溃
* [Commit 6895](https://github.com/manticoresoftware/manticoresearch/commit/68953740af8dca1833026d0f448c71416ed3044a) 修复 ATTACH 后的松散 docstore
* [Commit d6f6](https://github.com/manticoresoftware/manticoresearch/commit/d6f696edea53ea2b9a68251e4054e4816082c285) 修复分布式设置中的 docstore 问题
* [Commit bce2](https://github.com/manticoresoftware/manticoresearch/commit/bce2b7ec883d208f31094ee42a5b6f01d18a998f) 在排序器中用 OpenHash 替换 FixedHash
* [Commit e0ba](https://github.com/manticoresoftware/manticoresearch/commit/e0baf7392937969e640fb5d10e8ccb312fe36446) 修复索引定义中重复名称的属性
* [Commit ca81](https://github.com/manticoresoftware/manticoresearch/commit/ca81114bf1ac7ea7adcc8509bdc14d1f30b4f461) 修复 HIGHLIGHT() 中的 html_strip
* [Commit 493a](https://github.com/manticoresoftware/manticoresearch/commit/493a5e916d70fd4a3f14d1ee487234434f4a7ce3) 修复 HIGHLIGHT() 中的 passage 宏
* [Commit a82d](https://github.com/manticoresoftware/manticoresearch/commit/a82d41c775692f837f267d554d7d4a4b499d69f0) 修复当实时索引创建小或大磁盘块时的双缓冲问题
* [Commit a404](https://github.com/manticoresoftware/manticoresearch/commit/a404c85de0d41d9d1c75b7540479d8a3af9e61aa) 修复 kqueue 的事件删除
* [Commit 8bea](https://github.com/manticoresoftware/manticoresearch/commit/8bea0f6fd1666b817995c4dd6e67f6f1c9161638) 修复 RT 索引的大 rt_mem_limit 值磁盘块的保存
* [Commit 8707](https://github.com/manticoresoftware/manticoresearch/commit/8707f0395b7ace52dc0cba9c8ed92a2bc4392932) 修复索引时的浮点溢出
* [Commit a564](https://github.com/manticoresoftware/manticoresearch/commit/a56434ce71bb7714b4d8e75278aaa075b27ece80) 修复向 RT 索引插入负 ID 的错误
* [Commit bbeb](https://github.com/manticoresoftware/manticoresearch/commit/bbebfd7551a68641154761cac962053b54806759) 修复在排名器字段掩码上服务器崩溃
* [Commit 3809](https://github.com/manticoresoftware/manticoresearch/commit/3809cc1b0863e3a5a200dc423be3f7c38be56113) 修复使用查询缓存时崩溃
* [Commit dc2a](https://github.com/manticoresoftware/manticoresearch/commit/dc2a585bc724678679e4297362200bbd49ce32bb) 修复使用 RT 索引 RAM 段进行并行插入时崩溃

## 版本 3.2.2，2019 年 12 月 19 日
### 特性
* RT 索引的自增 ID
* 通过新的 HIGHLIGHT() 函数支持 docstore 的高亮，同时在 HTTP API 中也可用
* SNIPPET() 可以使用特殊功能 QUERY()，返回当前 MATCH 查询
* 高亮函数的新 field_separator 选项。

### 改进和变更
* 对于远程节点的存储字段懒加载（可以显著提高性能）
* 字符串和表达式不再中断多查询和 FACET 优化
* RHEL/CentOS 8 构建现在使用来自 mariadb-connector-c-devel 的 mysql libclient
* ICU 数据文件现在随软件包提供，icu_data_dir 被删除
* systemd 服务文件包括 'Restart=on-failure' 策略
* indextool 现在可以在线检查实时索引
* 默认配置现在是 /etc/manticoresearch/manticore.conf
* RHEL/CentOS 上的服务从 'searchd' 更名为 'manticore'
* 移除了 query_mode 和 exact_phrase 片段选项

### 错误修复
* [Commit 6ae4](https://github.com/manticoresoftware/manticoresearch/commit/6ae474c7894a6bee222d5b18e59a44fdbf57843a) 修复通过 HTTP 接口进行 SELECT 查询时崩溃
* [Commit 5957](https://github.com/manticoresoftware/manticoresearch/commit/59577513a49eac5a4a3c5e2cb38394d3246b5d35) 修复 RT 索引保存磁盘块但不标记某些文档为已删除
* [Commit e861](https://github.com/manticoresoftware/manticoresearch/commit/e861f0fca0e88924450695d4e9d6acff7a36558a) 修复使用 dist_threads 进行多索引或多查询搜索时的崩溃
* [Commit 4409](https://github.com/manticoresoftware/manticoresearch/commit/440991fc977b8479800b45cdbc862eeb1ba5d965) 修复对具有宽 utf8 字符编码点的长元词生成中缀时崩溃
* [Commit 5fd5](https://github.com/manticoresoftware/manticoresearch/commit/5fd599b48bba527c023e6aa0b262dca51ffb8a1c) 修复添加套接字到 IOCP 时的竞争
* [Commit cf10](https://github.com/manticoresoftware/manticoresearch/commit/cf10d7d3589f77adfaefbe52a51777bc9c67cf99) 修复布尔查询与 json 选择列表之间的问题
* [Commit 996d](https://github.com/manticoresoftware/manticoresearch/commit/996de77f4c3ec103d965e28c36098fd07dba50c4) 修复 indextool 检查以报告错误的跳过列表偏移，并检查 doc2row 查找
* [Commit 6e3f](https://github.com/manticoresoftware/manticoresearch/commit/6e3fc9e88941c9427410f9d0adeebb7dd09a900f) 修复索引器在大数据上生成负跳过列表偏移的坏索引
* [Commit faed](https://github.com/manticoresoftware/manticoresearch/commit/faed3220b423a96401982bf47117edf1f62c584e) 修复 JSON 仅将数字转换为字符串，JSON 字符串转换为数字的表达式
* [Commit 5331](https://github.com/manticoresoftware/manticoresearch/commit/533197200249d648ebcbdaca5d605861ee566348) 修复 indextool 在命令行设置多个命令时以错误代码退出
* [Commit 7955](https://github.com/manticoresoftware/manticoresearch/commit/795520ac351d22e4497ebaf6c249bf25081842b6) 修复 #275 binlog 错误时的无效状态，磁盘上没有剩余空间
* [Commit 2284](https://github.com/manticoresoftware/manticoresearch/commit/2284da5e3ff77bf7be4038a4d950c0580e4159c8) 修复 #279 在 IN 过滤器中对 JSON 属性的崩溃
* [Commit ce2e](https://github.com/manticoresoftware/manticoresearch/commit/ce2e4b4765111a7d331ffc256911f12770a8942d) 修复 #281 错误的管道关闭调用
* [Commit 5355](https://github.com/manticoresoftware/manticoresearch/commit/535589ba8a37baf5c6056afb5ff969fd70476feb) 修复服务器在CALL PQ时挂起，递归的 JSON 属性编码为字符串
* [Commit a5fc](https://github.com/manticoresoftware/manticoresearch/commit/a5fc8a36e1644ee051632408ab8b53e79aeb9ff4) 修复在 multiand 节点中超出 doclist 末尾的问题
* [Commit a362](https://github.com/manticoresoftware/manticoresearch/commit/a3628617ef4ee89a811d4ba2c4a04cbbc298c1b5) 修复线程公共信息的检索
* [Commit f8d2](https://github.com/manticoresoftware/manticoresearch/commit/f8d2d7bbbb90af3d2ee97424f97b05927f09d42b) 修复 docstore 缓存锁

## 版本 3.2.0, 2019年10月17日
### 特性
* 文档存储
* 新指令 stored_fields, docstore_cache_size, docstore_block_size, docstore_compression, docstore_compression_level

### 改进与变更
* 改善 SSL 支持
* 更新非CJK内置字符集
* 禁用在查询日志中记录 UPDATE/DELETE 语句的 SELECT
* RHEL/CentOS 8 包

### Bug 修复
* [Commit 301a](https://github.com/manticoresoftware/manticoresearch/commit/301a806b14725e842300ee225521407464374c41) 修复 RT 索引的磁盘块中替换文档时崩溃
* [Commit 46c1](https://github.com/manticoresoftware/manticoresearch/commit/46c1cad8fa11c1fc09cc34b20d978fdfcd4e6d27) 修复 \#269 LIMIT N OFFSET M
* [Commit 92a4](https://github.com/manticoresoftware/manticoresearch/commit/92a46edaa196ef33f3ac2225fbdba2c2b88ee45d) 修复带有显式设置的 id 或提供的 id 列表以跳过搜索的 DELETE 语句
* [Commit 8ca7](https://github.com/manticoresoftware/manticoresearch/commit/8ca78c138d837caab30dc6e2343a3c4d1687bb87) 修复在 netloop 的 windowspoll 投票器中删除事件后的错误索引
* [Commit 6036](https://github.com/manticoresoftware/manticoresearch/commit/603631e2bff5cb0acf25f68b85621742490eccd9) 修复 JSON 通过 HTTP 的浮点数向上取整
* [Commit 62f6](https://github.com/manticoresoftware/manticoresearch/commit/62f64cb9e75ec28fc3c29f8414ce9656f5580254) 修复远程片段检查空路径; 修复 Windows 测试
* [Commit aba2](https://github.com/manticoresoftware/manticoresearch/commit/aba274c2c5abc1029894ba9f95b786bda93e8f22) 修复配置在 Windows 上重新加载与 Linux 相同的工作方式
* [Commit 6b8c](https://github.com/manticoresoftware/manticoresearch/commit/6b8c4242ef77c3fa4d0ccb7d76d81714b6728f0b) 修复 \#194 PQ 与形态和词干分析器一起工作
* [Commit 174d](https://github.com/manticoresoftware/manticoresearch/commit/174d312905285aa5746f4f404c53e6dc669ef42f) 修复 RT 已退休段的管理

## 版本 3.1.2, 2019年8月22日
### 特性与改进
* HTTP API 的实验性 SSL 支持
* CALL KEYWORDS 的字段过滤
* /json/search 的 max_matches
* 默认 Galera gcache.size 的自动大小调整
* 改进 FreeBSD 支持

### Bug 修复
* [Commit 0a1a](https://github.com/manticoresoftware/manticoresearch/commit/0a1a2c81038794983b4e30c8aefc7cc20e89b03f) 修复在同一 RT 索引存在并具有不同路径的节点中复制 RT 索引
* [Commit 4adc](https://github.com/manticoresoftware/manticoresearch/commit/4adc075294ac823289f745e2cc419f18c7dcf2e2) 修复无活动索引的刷新重新调度
* [Commit d6c0](https://github.com/manticoresoftware/manticoresearch/commit/d6c00a6f750c04021747e9c99e0f77707a4cbfc3) 改进 RT/PQ 索引的刷新重新调度
* [Commit d0a7](https://github.com/manticoresoftware/manticoresearch/commit/d0a7c95906b9b5ba33c98267d54501cfe27508d6) 修复 \#250 index_field_lengths 索引选项用于 TSV 和 CSV 管道源
* [Commit 1266](https://github.com/manticoresoftware/manticoresearch/commit/1266d54800242709fbe2c34e72eb69b6595b23bd) 修复 indextool 在空索引上的错误报告
* [Commit 553c](https://github.com/manticoresoftware/manticoresearch/commit/553ca73ca111f28e0888d59cd48b0d6df181ecd6) 修复 Manticore SQL 查询日志中的空选择列表
* [Commit 56c8](https://github.com/manticoresoftware/manticoresearch/commit/56c8584456293665ddd4ce4c94307d74473a78b9) 修复索引器 -h/--help 响应

## 版本 3.1.0, 2019年7月16日
### 特性与改进
* 实时索引的复制
* 针对中文的 ICU 分词器
* 新的形态选项 icu_chinese
* 新指令 icu_data_dir
* 多个语句事务用于复制
* LAST_INSERT_ID() 和 @session.last_insert_id
* LIKE 'pattern' 用于 SHOW VARIABLES
* 多文档插入用于提取索引
* 为配置添加时间解析器
* 内部任务管理器
* mlock 用于文档和命中列表组件
* 监禁片段路径

### 删除
* 放弃 RLP 库支持以支持 ICU; 所有 rlp\* 指令已删除
* 禁用 UPDATE 更新文档 ID

### Bug 修复
* [Commit f047](https://github.com/manticoresoftware/manticoresearch/commit/f047222339fd0e62a07e9ca8b2a829297d5db861) 修复 concat 和 group_concat 中的缺陷
* [Commit b081](https://github.com/manticoresoftware/manticoresearch/commit/b08147ee24712b5eaa51f261626b5d16c6242fdd) 修复提取索引中的查询 uid 为 BIGINT 属性类型
* [Commit 4cd8](https://github.com/manticoresoftware/manticoresearch/commit/4cd85afaf76eef0ce9fa1cd073f3fed160279890) 如果预分配新的磁盘块失败，不要崩溃
* [Commit 1a55](https://github.com/manticoresoftware/manticoresearch/commit/1a551227886474054111088412f2b2396f485652) 为 ALTER 添加缺失的时间戳数据类型
* [Commit f3a8](https://github.com/manticoresoftware/manticoresearch/commit/f3a8e096fd496a6b001d398aadbb781b2dbad929) 修复了错误的 mmap 读取导致的崩溃
* [Commit 4475](https://github.com/manticoresoftware/manticoresearch/commit/447577115192db7ac7bfea70b5ba58ee2c61229f) 修复了复制过程中集群锁哈希的问题
* [Commit ff47](https://github.com/manticoresoftware/manticoresearch/commit/ff476df955f057bd38e8b6ad6056325e9d6fbb9c) 修复了复制过程中提供者泄漏的问题
* [Commit 58dc](https://github.com/manticoresoftware/manticoresearch/commit/58dcbb779c4c30ca1560460c78e35ba114b83716) 修复了索引器中未定义 sigmask 的问题（#246）
* [Commit 3dd8](https://github.com/manticoresoftware/manticoresearch/commit/3dd8278e747d534f69fe9c3d360eef188ab7b6d5) 修复了 netloop 报告中的竞态问题
* [Commit a02a](https://github.com/manticoresoftware/manticoresearch/commit/a02aae05a3fa3d1caf2821f43c7e237b6de76336) 为 HA 策略重平衡器实现零间隙

## 版本 3.0.2，2019年5月31日
### 改进
* 为文档和命中列表新增了 mmap 读取器
* `/sql` HTTP 端点的响应现在与 `/json/search` 的响应相同
* 新增指令 `access_plain_attrs`、`access_blob_attrs`、`access_doclists`、`access_hitlists`
* 用于复制设置的新指令 `server_id`

### 移除项
* 移除了 HTTP `/search` 端点

### 弃用项
* 用 `access_*` 指令替换了 `read_buffer`、`ondisk_attrs`、`ondisk_attrs_default`、`mlock`

### 错误修复
* [Commit 849c](https://github.com/manticoresoftware/manticoresearch/commit/849c16e1040f382795ba0635cb48686f9db4f2d8) 允许选择列表中以数字开头的属性名称
* [Commit 48e6](https://github.com/manticoresoftware/manticoresearch/commit/48e6c302ca37bec0bc49c79619c60bf787ed0708) 修复了 UDF 中的 MVAs 问题，并修复了 MVA 别名问题
* [Commit 0555](https://github.com/manticoresoftware/manticoresearch/commit/055586a9a2ebfa8874f68099c5bc798a145fd4ef) 修复了使用 SENTENCE 查询时导致的崩溃（#187）
* [Commit 93bf](https://github.com/manticoresoftware/manticoresearch/commit/93bf52f23f1c53c8a93a603387ca5463e1fc2ae8) 修复了在 MATCH() 周围使用 () 的支持问题（#143）
* [Commit 599e](https://github.com/manticoresoftware/manticoresearch/commit/599ee79ccd38cce0023a55cbce8f50c3dca62e38) 修复了 ALTER cluster 语句保存集群状态时的问题
* [Commit 230c](https://github.com/manticoresoftware/manticoresearch/commit/230c321e277514b191629538eb3f02d219113d95) 修复了在 ALTER index 时，使用 blob 属性导致服务器崩溃的问题
* [Commit 5802](https://github.com/manticoresoftware/manticoresearch/commit/5802b85aa93a81caea77073a3381ef912fdd677c) 修复了通过 id 过滤时出现的问题（#196）
* [Commit 25d2](https://github.com/manticoresoftware/manticoresearch/commit/25d2dabda4a299a29bd375405862126da9b4bfac) 放弃在模板索引上进行搜索
* [Commit 2a30](https://github.com/manticoresoftware/manticoresearch/commit/2a30d5b49d76faab031d195f533974938769b8e6) 修复了 SQL 响应中 id 列未采用常规 bigint 类型的问题


## 版本 3.0.0，2019年5月6日
### 特性及改进
* 全新的索引存储方式。非标量属性不再受每个索引 4GB 大小的限制
* attr_update_reserve 指令
* 使用 UPDATE 可以更新 String、JSON 和 MVAs
* killlists 在索引加载时应用
* killlist_target 指令
* 加速多重 AND 搜索
* 提升了整体性能和内存使用效率
* 用于升级基于 2.x 制作的索引的转换工具
* CONCAT() 函数
* JOIN CLUSTER cluster AT 'nodeaddress:port'
* ALTER CLUSTER posts UPDATE nodes
* node_address 指令
* 在 SHOW STATUS 中打印的节点列表

### 行为变化
* 对于带有 killlists 的索引，服务器不再按照配置中定义的顺序轮换索引，而是遵循 killlist_target 链
* 搜索中索引的顺序不再决定 killlists 应用的顺序
* 文档 ID 现在为有符号大整数

### 移除的指令
* docinfo（现始终为 extern）、inplace_docinfo_gap、mva_updates_pool

## 版本 2.8.2 GA，2019年4月2日
### 特性及改进
* 针对 percolate 索引的 Galera 复制
* OPTION morphology

### 编译说明
Cmake 最低版本现在为 3.13。编译需要 boost 和 libssl
开发库。

### 错误修复
* [Commit 6967](https://github.com/manticoresoftware/manticoresearch/commit/6967fedb2ef818ec1c825d482563edd05e1c9245) 修复了在对多个分布式索引进行查询时，选择列表中过多星号导致的崩溃
* [Commit 36df](https://github.com/manticoresoftware/manticoresearch/commit/36df1a407dc08263690e3492518613ace82d69ca) 修复了通过 Manticore SQL 接口发送大数据包的问题（#177）
* [Commit 5793](https://github.com/manticoresoftware/manticoresearch/commit/57932aec734583fa93359faaf5034b2e2c9d352b) 修复了在 RT 优化时 MVA 更新导致服务器崩溃的问题（#170）
* [Commit edb2](https://github.com/manticoresoftware/manticoresearch/commit/edb24b870423add86eba471d361e0e5aff098b18) 修复了在 SIGHUP 后配置重新加载、移除 RT 索引时，因移除 binlog 而导致的服务器崩溃问题
* [Commit bd3e](https://github.com/manticoresoftware/manticoresearch/commit/bd3e66e0085bc7e2e351b817dfc972fd8158fbce) 修复了 mysql 握手认证插件负载的问题
* [Commit 6a21](https://github.com/manticoresoftware/manticoresearch/commit/6a217f6ed82fb10f752213b15617b9cedf1e8533) 修复了 RT 索引中 phrase_boundary 设置的问题（#172）
* [Commit 3562](https://github.com/manticoresoftware/manticoresearch/commit/3562f652753e4091fd7b5b0f65b58341c8cbcb31) 修复了在 ATTACH 索引到自身时发生死锁的问题（#168）
* [Commit 250b](https://github.com/manticoresoftware/manticoresearch/commit/250b3f0e74ad18dda34ba080aa13ca87798a9ac9) 修复了服务器崩溃后 binlog 保存空元数据的问题
* [Commit 4aa6](https://github.com/manticoresoftware/manticoresearch/commit/4aa6c69ad3a181cd7c5dafb990fb528121f68e59) 修复了由于从 RT 索引带有磁盘块的排序器的字符串导致的服务器崩溃

## 版本 2.8.1 GA，2019年3月6日
### 新功能和改进
* SUBSTRING_INDEX()
* 对于逐层查询支持 SENTENCE 和 PARAGRAPH
* Debian/Ubuntu 的 systemd 生成器；还添加了 LimitCORE 以允许核心转储

### 缺陷修复
* [Commit 84fe](https://github.com/manticoresoftware/manticoresearch/commit/84fe7405d17b59f4dcde15a6c8e8503923503def) 修复了服务器在匹配模式全部和空全文查询时崩溃
* [Commit daa8](https://github.com/manticoresoftware/manticoresearch/commit/daa88b579fa373d5d2e869e43a2e178363b0fef1) 修复了删除静态字符串时崩溃
* [Commit 2207](https://github.com/manticoresoftware/manticoresearch/commit/22078537dddbd9ce81a182dcc879f235b6e87004) 修复了 indextool 失败时的退出代码为 FATAL
* [Commit 0721](https://github.com/manticoresoftware/manticoresearch/commit/0721696d6780c200c65d596624a8187983fb7fcb) 修复了 [\#109](https://github.com/manticoresoftware/manticoresearch/issues/109) 由于错误的精确形式检查而导致的前缀无匹配
* [Commit 8af8](https://github.com/manticoresoftware/manticoresearch/commit/8af810111b8e2f87bc9f378172eff1ab9725c7e7) 修复了 [\#161](https://github.com/manticoresoftware/manticoresearch/issues/161) 对 RT 索引的配置设置重新加载
* [Commit e2d5](https://github.com/manticoresoftware/manticoresearch/commit/e2d592773f4fd702ace90070a793a842a0fec6ab) 修复了服务器在访问大型 JSON 字符串时崩溃
* [Commit 75cd](https://github.com/manticoresoftware/manticoresearch/commit/75cd1342d05cdc352e3dd145cdbc6f79394a165b) 修复了在 JSON 文档中由于索引剥离器而导致的 PQ 字段错误匹配来自兄弟字段
* [Commit e2f7](https://github.com/manticoresoftware/manticoresearch/commit/e2f775437df922674d772ed4417780492502e65a) 修复了在 RHEL7 构建中解析 JSON 时服务器崩溃
* [Commit 3a25](https://github.com/manticoresoftware/manticoresearch/commit/3a25a5808feb3f8b80866991b436d6c1241618c2) 修复了当斜杠位于边缘时 JSON 解转义崩溃
* [Commit be9f](https://github.com/manticoresoftware/manticoresearch/commit/be9f497872bea62dbdccd64fc28294fd7776c289) 修复了选项 'skip_empty' 以跳过空文档且不警告它们不是有效的 JSON
* [Commit 266e](https://github.com/manticoresoftware/manticoresearch/commit/266e0e7b088549722a805fc837bf101ff681a5e8) 修复了 [\#140](https://github.com/manticoresoftware/manticoresearch/issues/161) 浮点数输出 8 位数字当 6 位不足以精确
* [Commit 3f6d](https://github.com/manticoresoftware/manticoresearch/commit/3f6d23891064ee8e94030c4231497cdd7da33a6a) 修复了空 jsonobj 创建
* [Commit f3c7](https://github.com/manticoresoftware/manticoresearch/commit/f3c7848a59b7eedc67c3403d6d4b37ce6fa94dc8) 修复了 [\#160](https://github.com/manticoresoftware/manticoresearch/issues/161) 空 mva 输出 NULL 而不是空字符串
* [Commit 0afa](https://github.com/manticoresoftware/manticoresearch/commit/0afa2ed058e5759470b1d7354c722faab34f98bb) 修复了在没有 pthread_getname_np 的情况下构建失败
* [Commit 9405](https://github.com/manticoresoftware/manticoresearch/commit/9405fccdeb27f0302a8d9a848981d30080216777) 修复了服务器关闭时线程池工作者崩溃

## 版本 2.8.0 GA，2019年1月28日
### 改进
* 用于逐层索引的分布式索引
* CALL PQ 新选项和变更：
    *   skip_bad_json
    *   mode (sparsed/sharded)
    *   JSON 文档可以作为 JSON 数组传递
    *   shift
    *   列名 'UID'、'Documents'、'Query'、'Tags'、'Filters' 被重命名为 'id'、'documents'、'query'、'tags'、'filters'
* DESCRIBE pq TABLE
* SELECT FROM pq WHERE UID 不再可能，使用 'id' 代替
* 在 pq 索引上进行 SELECT 与常规索引处于同一水平（例如，您可以通过 REGEX() 过滤规则）
* ANY/ALL 可以在 PQ 标签上使用
* 表达式对 JSON 字段具有自动转换，不需要显式转换
* 内置 'non_cjk' charset_table 和 'cjk' ngram_chars
* 支持 50 种语言的内置停用词集合
* 停用词声明中的多个文件也可以用逗号分隔
* CALL PQ 可以接受文档的 JSON 数组

### 缺陷修复
* [Commit a4e1](https://github.com/manticoresoftware/manticoresearch/commit/a4e19afee54dafdc04b0da53372dafd8c0d332d6) 修复了与 csjon 相关的泄漏
* [Commit 28d8](https://github.com/manticoresoftware/manticoresearch/commit/28d862774874891a03e361da1b0347ebe8066ce0) 修复了由于 JSON 中缺少值而导致的崩溃
* [Commit bf4e](https://github.com/manticoresoftware/manticoresearch/commit/bf4e9ea27c349cdc9ae4e54e960a4a17b90e38fa) 修复了 RT 索引的空元数据保存
* [Commit 33b4](https://github.com/manticoresoftware/manticoresearch/commit/33b4573529e7c7c7bce19bf9d54054866f30d99a) 修复了丢失的形式标志（精确）用于词形还原器的序列
* [Commit 6b95](https://github.com/manticoresoftware/manticoresearch/commit/6b95d48240d3b5520afade19c249d79e020a5e88) 修复了字符串属性 > 4M 使用饱和而不是溢出
* [Commit 6214](https://github.com/manticoresoftware/manticoresearch/commit/621418b829e70af36aaa322a2f51ece3f86bc499) 修复了服务器在 SIGHUP 上崩溃时禁用索引
* [Commit 3f7e](https://github.com/manticoresoftware/manticoresearch/commit/3f7e35d1482966cc45d5abbcb2de5de3508d66b0) 修复了同时 API 会话状态命令时服务器崩溃
* [Commit cd9e](https://github.com/manticoresoftware/manticoresearch/commit/cd9e4f1709a48ddafc128c450c2d882bc11ba50e) 修复了对带有字段过滤器的 RT 索引的删除查询时服务器崩溃
* [Commit 9376](https://github.com/manticoresoftware/manticoresearch/commit/9376470d455fcd256c6abff79c431a6919dfa7ac) 修复了对分布式索引的 CALL PQ 时服务器崩溃，文档为空
* [Commit 8868](https://github.com/manticoresoftware/manticoresearch/commit/8868b207644f4f8cc1ab0c270adb35493fed22e1) 修复了 Manticore SQL 错误信息超过 512 个字符的问题
* [Commit de9d](https://github.com/manticoresoftware/manticoresearch/commit/de9deda9c142823d6dbf529423417b5c670fae94) 修复了在没有 binlog 的情况下保存 percolate 索引时的崩溃问题
* [Commit 2b21](https://github.com/manticoresoftware/manticoresearch/commit/2b219e1a32791e7740bb210b7d408f96abc6374f) 修复了在 OSX 中 HTTP 接口无法工作的问题
* [Commit e92c](https://github.com/manticoresoftware/manticoresearch/commit/e92c60240f8f9e1756bfa14fba0bbb402586a389) 修复了 indextool 在检查 MVA 时错误消息错误的问题
* [Commit 238b](https://github.com/manticoresoftware/manticoresearch/commit/238bdea59bad89f097403f1c978658ce45f16c70) 修复了在 FLUSH RTINDEX 时写锁定以避免在保存和从 rt_flush_period 正常刷新时锁定整个索引的问题
* [Commit c26a](https://github.com/manticoresoftware/manticoresearch/commit/c26a236bd42b7082db079a51a84172066d337d64) 修复了 ALTER percolate 索引在等待搜索加载时卡住的问题
* [Commit 9ee5](https://github.com/manticoresoftware/manticoresearch/commit/9ee5703f6c28f57cd5bff5e705966d93fe30d267) 修复了 max_children 使用默认线程池 worker 数量的问题，当其值为 0 时
* [Commit 5138](https://github.com/manticoresoftware/manticoresearch/commit/5138fc048a1bd146b271ce6a72fe954344281f69) 修复了在使用 index_token_filter 插件及 stopwords 和 stopword_step=0 进行数据索引时的错误
* [Commit 2add](https://github.com/manticoresoftware/manticoresearch/commit/2add3d319ac62fe450bf60e89033100853dc2ecf) 修复了在使用 aot 词形还原器的索引定义中缺失 lemmatizer_base 时崩溃的问题

## 版本 2.7.5 GA, 2018年12月4日
### 改进
* REGEX 函数
* JSON API 搜索的 limit/offset
* qcache 的 profiler points

### Bug修复
* [Commit eb3c](https://github.com/manticoresoftware/manticoresearch/commit/eb3c7683b9740cff4cc4cadd2ab9f990b9f7d173) 修复了在 FACET 中使用多个属性宽类型时服务器崩溃的问题
* [Commit d915](https://github.com/manticoresoftware/manticoresearch/commit/d915cf6696514f4be365ffb5981bccaa15983e65) 修复了在 FACET 查询的主要选择列表中的隐式分组问题
* [Commit 5c25](https://github.com/manticoresoftware/manticoresearch/commit/5c25dc271a8c11a1c82ca3c88e518acbc6a8c164) 修复了在使用 GROUP N BY 查询时的崩溃问题
* [Commit 85d3](https://github.com/manticoresoftware/manticoresearch/commit/85d30a232d4ffd89916b673a9b2db946d272a945) 修复了在内存操作处理崩溃时的死锁问题
* [Commit 8516](https://github.com/manticoresoftware/manticoresearch/commit/85166b5ef2be38061d62f2b20f15acaa0311258a) 修复了在检查时 indextool 的内存消耗问题
* [Commit 58fb](https://github.com/manticoresoftware/manticoresearch/commit/58fb03166830d8b23e7ab9ce56309db7ddac22f4) 修复了不再需要 gmock 包含的问题，因为上游自己解决了

## 版本 2.7.4 GA, 2018年11月1日
### 改进
* 在远程分布式索引情况下，SHOW THREADS 打印原始查询而不是 API 调用
* SHOW THREADS 新选项 `format=sphinxql` 以 SQL 格式打印所有查询
* SHOW PROFILE 打印额外的 `clone_attrs` 阶段

### Bug修复
* [Commit 4f15](https://github.com/manticoresoftware/manticoresearch/commit/4f155712a0bccc1bd01cc191647bc8cff814888e) 修复了在没有 malloc_stats 和 malloc_trim 的 libc 下构建失败的问题
* [Commit f974](https://github.com/manticoresoftware/manticoresearch/commit/f974f20bda3214a56877c393a192be1a77150958) 修复了在 CALL KEYWORDS 结果集中单词内的特殊符号的问题
* [Commit 0920](https://github.com/manticoresoftware/manticoresearch/commit/092083282ea8ae0b2e72fb6989c57ccec81e74ac) 修复了无法通过 API 或远程代理调用分布式索引的 CALL KEYWORDS 的问题
* [Commit fd68](https://github.com/manticoresoftware/manticoresearch/commit/fd686bfe88b720ffd7642e36897ba45161cbd7d2) 修复了分布式索引 agent_query_timeout 向代理传播为 max_query_time 的问题
* [Commit 4ffa](https://github.com/manticoresoftware/manticoresearch/commit/4ffa623e9d357a3b0b441615089e211f92f8de32) 修复了 OPTIMIZE 命令影响磁盘块的总文档计数，且破坏了权重计算的问题
* [Commit dcaf](https://github.com/manticoresoftware/manticoresearch/commit/dcaf4e0e3c8e9b0fbec47bbf3307feddec0936a6) 修复了混合的 RT 索引中多个尾部命中的问题
* [Commit eee3](https://github.com/manticoresoftware/manticoresearch/commit/eee381754e902a43a8f499e1c950198a2a3e6ee0) 修复了旋转时的死锁问题

## 版本 2.7.3 GA, 2018年9月26日
### 改进
* CALL KEYWORDS 的 sort_mode 选项
* DEBUG 在 VIP 连接上可以执行 'crash <password>' 以在服务器上执行故意的 SIGEGV 操作
* DEBUG 可以执行 'malloc_stats' 以在 searchd.log 中转储 malloc stats，执行 'malloc_trim' 以进行 malloc_trim()
* 如果系统上存在 gdb 改进的回溯

### Bug修复
* [Commit 0f3c](https://github.com/manticoresoftware/manticoresearch/commit/0f3cc333f6129451ad1ae2c97905b6c04ba5d454) 修复了在 Windows 上重命名时的崩溃或失败问题
* [Commit 1455](https://github.com/manticoresoftware/manticoresearch/commit/1455ba2bf8079f03ce7f861a8d9662a360d705de) 修复了在 32 位系统上服务器崩溃的问题
* [Commit ad37](https://github.com/manticoresoftware/manticoresearch/commit/ad3710d53b5cd6a28b8c60352d0f9e285b03d9fa) 修复了在 SNIPPET 表达式为空时服务器崩溃或挂起的问题
* [Commit b36d](https://github.com/manticoresoftware/manticoresearch/commit/b36d79214364500252665860f000ae85343528b6) 修复了断裂的非渐进优化，并修复渐进优化以不为最旧磁盘块创建杀戮列表的问题
* [Commit 34b0](https://github.com/manticoresoftware/manticoresearch/commit/34b032499afd42ce47a4c7247814b4031094388a) 修复了在线程池 worker 模式下 SQL 和 API 的 queue_max_length 错误回复问题
* [Commit ae4b](https://github.com/manticoresoftware/manticoresearch/commit/ae4b3202cbdb8014cabe2b90e269d5cf74f49871) 修复了在向 PQ 索引添加全扫描查询时设置 regexp 或 rlp 选项导致的崩溃
* [Commit f80f](https://github.com/manticoresoftware/manticoresearch/commit/f80f8d5d7560187078868aed9a9575f4549e98aa) 修复了连续调用一个 PQ 后导致的崩溃
* [Commit 9742](https://github.com/manticoresoftware/manticoresearch/commit/9742f5f0866af73f8cd8483ecd18a507ea80dd65) 重构 AcquireAccum
* [Commit 39e5](https://github.com/manticoresoftware/manticoresearch/commit/39e5bc3751b7295222eb76407c5d72ce1dad545b) 修复了调用 pq 后内存泄漏的问题
* [Commit 21bc](https://github.com/manticoresoftware/manticoresearch/commit/21bcc6d17395f0f57dde79f6716ef303b7ea527d) 代码美化重构 (c++11 风格 c-trs, 默认值, nullptrs)
* [Commit 2d69](https://github.com/manticoresoftware/manticoresearch/commit/2d690398f14c736956cfdd66feb4d3091d6b3a4d) 修复了尝试插入重复项到 PQ 索引时的内存泄漏
* [Commit 5ed9](https://github.com/manticoresoftware/manticoresearch/commit/5ed92c4b66da6423fa28f11b1fd59103ed1e1fb9) 修复了在 JSON 字段 IN 中使用大值时的崩溃
* [Commit 4a52](https://github.com/manticoresoftware/manticoresearch/commit/4a5262e2776aa261e34ba5c914ea60804f59de3f) 修复了在调用 KEYWORDS 语句时设置扩展限制导致的服务器崩溃
* [Commit 5526](https://github.com/manticoresoftware/manticoresearch/commit/552646bb6fefde4e2525298e43e628839b421d3d) 修复了 PQ 匹配查询中的无效过滤器;
* [Commit 204f](https://github.com/manticoresoftware/manticoresearch/commit/204f521408b730198a5bab2c20407a3d7df94873) 引入小对象分配器用于指针属性
* [Commit 2545](https://github.com/manticoresoftware/manticoresearch/commit/25453e5387152c0575d1eda1b2beb89a49c7493a) 将 ISphFieldFilter 重构为引用计数版本
* [Commit 1366](https://github.com/manticoresoftware/manticoresearch/commit/1366ee06e828397074b69bec9265d7147170c60e) 修复了在非终止字符串上使用 strtod 时的未定义行为/段错误
* [Commit 94bc](https://github.com/manticoresoftware/manticoresearch/commit/94bc6fcbdae4d08bcf6da2966e3ec2966091289f) 修复了 JSON 结果集处理中的内存泄漏
* [Commit e78e](https://github.com/manticoresoftware/manticoresearch/commit/e78e9c948963416caff9e4e46296a58080107835) 修复了在应用属性添加时超出内存块末尾的读取
* [Commit fad5](https://github.com/manticoresoftware/manticoresearch/commit/fad572fb543606b33e688eb82a485b86011545fd) 修复了 CSphDict 的引用计数版本重构
* [Commit fd84](https://github.com/manticoresoftware/manticoresearch/commit/fd841a472b472280c93f4af3506851d19f6adc45) 修复了外部的 AOT 内部类型泄漏
* [Commit 5ee7](https://github.com/manticoresoftware/manticoresearch/commit/5ee7f208c7685f2ebb9e5623a8802b721e2a7f0a) 修复了分词器管理中的内存泄漏
* [Commit 116c](https://github.com/manticoresoftware/manticoresearch/commit/116c5f1abebee9a0d99afe93546a1d8e4c6c6958) 修复了分组器中的内存泄漏
* [Commit 56fd](https://github.com/manticoresoftware/manticoresearch/commit/56fdbc9ab38973a9a0ba8ccee45f71cf33c16423) 对动态指针在匹配中的特殊释放/复制（内存泄漏分组器）
* [Commit b1fc](https://github.com/manticoresoftware/manticoresearch/commit/31db660f378541aa0d1e7cf2e7209bb0a1f47fd8) 修复了 RT 的动态字符串内存泄漏
* [Commit 517b](https://github.com/manticoresoftware/manticoresearch/commit/517b9e8749f092e923cd2884b674b5bb84e20755) 重构分组器
* [Commit b1fc](https://github.com/manticoresoftware/manticoresearch/commit/b1fc16140e5dc44290686330b476a254e0092107) 小幅重构 (c++11 c-trs, 一些格式调整)
* [Commit 7034](https://github.com/manticoresoftware/manticoresearch/commit/7034e073f4cf2844762e0a464b7c3de05d3d122b) 将 ISphMatchComparator 重构为引用计数版本
* [Commit b1fc](https://github.com/manticoresoftware/manticoresearch/commit/b1fc16140e5dc44290686330b476a254e0092107) 将克隆器设为私有
* [Commit efbc](https://github.com/manticoresoftware/manticoresearch/commit/efbc0511d6809c4a57453c7283f9bf53f3fb8d97) 简化了 MVA_UPSIZE, DOCINFO2ID_T, DOCINFOSETID 的本机小端处理
* [Commit 6da0](https://github.com/manticoresoftware/manticoresearch/commit/6da0df4ac9946fc59f98d229b90e12c9221dd4c0) 将 valgrind 支持添加到 ubertests
* [Commit 1d17](https://github.com/manticoresoftware/manticoresearch/commit/1d17669cb7ec54ac80b392e101f37a688eb98080) 修复了因为连接上的 'success' 标志竞争导致的崩溃
* [Commit 5a09](https://github.com/manticoresoftware/manticoresearch/commit/5a09c32d7ded56b2c0bd3e2ad7968cb8d6362919) 将 epoll 切换到边缘触发模式
* [Commit 5d52](https://github.com/manticoresoftware/manticoresearch/commit/5d528682737fca03a4352a3093b1319ec27d598c) 修复了在表达式中格式如在过滤器的 IN 语句
* [Commit bd8b](https://github.com/manticoresoftware/manticoresearch/commit/bd8b3c976ff8b4667f55dfdd101b20a920137ac5) 修复了在提交具有大 docid 的文档时在 RT 索引处的崩溃
* [Commit ce65](https://github.com/manticoresoftware/manticoresearch/commit/ce656b83b747ce7c55795b53915770c13fb99cfe) 修复了 indextool 中无参数选项的问题
* [Commit 08c9](https://github.com/manticoresoftware/manticoresearch/commit/08c9507177820f441f534bf06a11dac5e54bebb4) 修复了扩展关键字的内存泄漏
* [Commit 30c7](https://github.com/manticoresoftware/manticoresearch/commit/30c75a2f525ec9bda625d903acdc9d4d2e2413dc) 修复了 JSON 分组器的内存泄漏
* [Commit 6023](https://github.com/manticoresoftware/manticoresearch/commit/6023f269b6f2080e4d380b0957605ef8107c8c9f) 修复了全局用户变量的泄漏
* [Commit 7c13](https://github.com/manticoresoftware/manticoresearch/commit/7c138f15ca23c0c8717fa12041e3db7f988fcf15) 修复了在早期拒绝匹配时动态字符串的泄漏
* [Commit 9154](https://github.com/manticoresoftware/manticoresearch/commit/9154b18eaed5302e8330cb3a95bd968959ccb312) 修复了 length(<expression>) 的泄漏
* [Commit 43fc](https://github.com/manticoresoftware/manticoresearch/commit/43fca3a4e26139765d0fac8de054200dc4875fc6) 修复了因为 parser 中的 strdup() 导致的内存泄漏
* [Commit 71ff](https://github.com/manticoresoftware/manticoresearch/commit/71ff77737e63d1019b7c9804dca7fa2351025dba) 修复了重构表达式解析器以准确跟踪引用计数

## 版本 2.7.2 GA，2018年8月27日
### 改进
* 与 MySQL 8 客户端的兼容性
* [TRUNCATE](Emptying_a_table.md) WITH RECONFIGURE
* 为 RT 索引在 SHOW STATUS 中取消内存计数器
* 多个代理的全局缓存
* 改进 Windows 上的 IOCP
* HTTP 协议的 VIP 连接
* Manticore SQL [DEBUG](Reporting_bugs.md#DEBUG) 命令可以运行各种子命令
* [shutdown_token](Server_settings/Searchd.md#shutdown_token) - 调用 `shutdown` 时需要的密码的 SHA1 哈希值，使用 DEBUG 命令
* 新统计信息来显示代理状态 (_ping, _has_perspool, _need_resolve)
* indexer 的 --verbose 选项现在接受 \[debugvv\] 用于打印调试消息

### 错误修复
* [Commit 3900](https://github.com/manticoresoftware/manticoresearch/commit/390082a7be0a1f9539b30361d11d54de35c62a44) 删除了优化时的 wlock
* [Commit 4c33](https://github.com/manticoresoftware/manticoresearch/commit/4c337648329be22373be33333cbc9c5664d18b80) 修复了重载索引设置时的 wlock
* [Commit b5ea](https://github.com/manticoresoftware/manticoresearch/commit/b5ea8dc0b18a23dbd739e92c66dd93b0094df14b) 修复了使用 JSON 过滤器查询时的内存泄漏
* [Commit 930e](https://github.com/manticoresoftware/manticoresearch/commit/930e831a43fd3d1ce996185be968d1df49a088d0) 修复了 PQ 结果集中空文档
* [Commit 53de](https://github.com/manticoresoftware/manticoresearch/commit/53deecfecb374431f84f8592a1095a77407b8aea) 修复了因移除任务而导致的任务混淆
* [Commit cad9](https://github.com/manticoresoftware/manticoresearch/commit/cad9b9c617719aad97e9f5a2ae93491153d8e504) 修复了错误的远程主机计数
* [Commit 9000](https://github.com/manticoresoftware/manticoresearch/commit/90008cc15c4684d445d37a391bb4a667bbc2e365) 修复了解析的代理描述符的内存泄漏
* [Commit 978d](https://github.com/manticoresoftware/manticoresearch/commit/978d839641ad6040c22c9fc3b703b1a02685f266) 修复了搜索中的泄漏
* [Commit 0193](https://github.com/manticoresoftware/manticoresearch/commit/0193946105ac08913362b7d7acf0c90f9b38dc85) 在显式/内联 c-trs、override/final 使用上的外观变化
* [Commit 943e](https://github.com/manticoresoftware/manticoresearch/commit/943e2997b33f9b2eda23d94a66068910f9476ebd) 修复了本地/远程模式中 JSON 的泄漏
* [Commit 02db](https://github.com/manticoresoftware/manticoresearch/commit/02dbdd6f1d0b72e7e458ebebdd2c67f989577e6b) 修复了本地/远程模式中 JSON 排序列表达式的泄漏
* [Commit c74d](https://github.com/manticoresoftware/manticoresearch/commit/c74d0b4035e4214510376845b22cb676a8da24a3) 修复了常量别名的泄漏
* [Commit 6e5b](https://github.com/manticoresoftware/manticoresearch/commit/6e5b57e1d3952c9695376728a143bf5434208f53) 修复了预读取线程的泄漏
* [Commit 39c7](https://github.com/manticoresoftware/manticoresearch/commit/39c740fef5bf71da0bee60a35f5dd9f471af850f) 修复了因 network 轮询中的等待阻塞而导致的退出卡住
* [Commit adaf](https://github.com/manticoresoftware/manticoresearch/commit/adaf9772f95772ad6d5297acace6c5cb92b56ad5) 修复了在将 HA 代理切换到常规主机时 'ping' 行为的卡住
* [Commit 32c4](https://github.com/manticoresoftware/manticoresearch/commit/32c40eac9cdd40b15d79e58b2f15416f164f9701) 为仪表板存储分离 gc
* [Commit 511a](https://github.com/manticoresoftware/manticoresearch/commit/511a3cf2c1f1858f6e94fe9f8175b7216db3cbd6) 修复了引用计数指针的修复
* [Commit 32c4](https://github.com/manticoresoftware/manticoresearch/commit/af2326cd2927effbad7bec88e8dc238a566cf27e) 修复了不存在索引时的 indextool 崩溃
* [Commit 156e](https://github.com/manticoresoftware/manticoresearch/commit/156edc717dc63d3ea120f42466eba6e022da88be) 修复了 xmlpipe 索引中超过属性/字段的输出名称
* [Commit cdac](https://github.com/manticoresoftware/manticoresearch/commit/cdac6d1f17d384bfc8bae49e91241b2f346c3830) 修复了如果配置中没有索引器部分，则默认索引器的值
* [Commit e61e](https://github.com/manticoresoftware/manticoresearch/commit/e61ec00b6b27d1d5878247e2ee817f3b1e7fde16) 修复了服务器重启后 RT 索引在磁盘块中嵌入的错误停用词
* [Commit 5fba](https://github.com/manticoresoftware/manticoresearch/commit/5fba49d5e28c17de4c0acbd984466127f42de6e8) 修复了跳过幻影（已关闭但未最终从轮询器中删除）连接
* [Commit f22a](https://github.com/manticoresoftware/manticoresearch/commit/f22ae34b623906b3c5a05a06198dad2e548dc541) 修复了混合（孤立）网络任务
* [Commit 4689](https://github.com/manticoresoftware/manticoresearch/commit/46890e70eb8dcd00c1bf1b030538b8f131c601c2) 修复了写操作后读取时崩溃
* [Commit 03f9](https://github.com/manticoresoftware/manticoresearch/commit/03f9df904f611c438d5ebcaaeef911b0dc8d6c39) 修复了在 Windows 上运行测试时 searchd 崩溃
* [Commit e925](https://github.com/manticoresoftware/manticoresearch/commit/e9255ed2704790a3d7f5273d38b85433463cb3ff) 修复了在常规 connect() 上处理 EINPROGRESS 代码
* [Commit 248b](https://github.com/manticoresoftware/manticoresearch/commit/248b72f1544b27e9e82773284050d44eeb1714e1) 修复了在处理 TFO 时的连接超时

## 版本 2.7.1 GA，2018年7月4日
### 改进
* 改进 PQ 中匹配多个文档的通配符性能
* 支持 PQ 中的全扫描查询
* 支持 PQ 中的 MVA 属性
* 支持在过热索引的正则表达式和 RLP 

### 错误修复
* [Commit 6885](https://github.com/manticoresoftware/manticoresearch/commit/68856261b41f6950666f9e5122726839fb3c71d0) 修复了查询字符串的丢失
* [Commit 0f17](https://github.com/manticoresoftware/manticoresearch/commit/0f1770943cba53bfd2e0edfdf0b1495ff0dd0e89) 修复了 SHOW THREADS 语句中的空信息
* [Commit 53fa](https://github.com/manticoresoftware/manticoresearch/commit/53faa36a916bb87868e83aa79c1a1f972fb20ca0) 修复了使用 NOTNEAR 操作符时的崩溃
* [Commit 2602](https://github.com/manticoresoftware/manticoresearch/commit/26029a7cd77b518fb3a27588d7db6c8bffc73b0f) 修复了错误筛选器导致 PQ 删除的错误信息


## 版本 2.7.0 GA, 2018年6月11日
### 改进
* 减少系统调用数量，以避免 Meltdown 和 Spectre 补丁的影响
* 本地索引管理的内部重写
* 远程片段重构
* 完整配置重载
* 所有节点连接现在是独立的
* proto 改进
* Windows 通信从 wsapoll 切换到 IO 完成端口
* 可以使用 TFO 在主节点和子节点之间通信
* [SHOW STATUS](Node_info_and_management/Node_status.md#SHOW-STATUS) 现在输出服务器版本和 mysql_version_string
* 为在 CALL PQ 中调用的文档添加 `docs_id` 选项。
* 过滤的渗透查询现在可以包含表达式
* 分布式索引可以与 FEDERATED 一起使用
* 虚拟 SHOW NAMES COLLATE 和 `SET wait_timeout`（为了更好的 ProxySQL 兼容性）

### 错误修复
* [Commit 5bcf](https://github.com/manticoresoftware/manticoresearch/commit/5bcff05d19e495f1a44f190885b25cbb6f217c4a) 修复添加不等于 PQ 的标签
* [Commit 9ebc](https://github.com/manticoresoftware/manticoresearch/commit/9ebc58916cd515eaa88da66d0895aebf0d1f2b5f) 修复添加文档 ID 字段到 JSON 文档 CALL PQ 语句
* [Commit 8ae0](https://github.com/manticoresoftware/manticoresearch/commit/8ae0e593a286110d8505d88baa3ac9e433cb38c9) 修复 PQ 索引的刷新语句处理程序
* [Commit c24b](https://github.com/manticoresoftware/manticoresearch/commit/c24b152344ecd77661566cddb803487f51d3c1aa) 修复 JSON 和字符串属性上的 PQ 过滤
* [Commit 1b8b](https://github.com/manticoresoftware/manticoresearch/commit/1b8bdde19aaa362785ea4c33bdc019154f8fbac4) 修复解析空 JSON 字符串
* [Commit 1ad8](https://github.com/manticoresoftware/manticoresearch/commit/1ad8a07dbfc1131913a8d92c261fbb48f934e8b7) 修复使用 OR 过滤器的多查询崩溃
* [Commit 69b8](https://github.com/manticoresoftware/manticoresearch/commit/69b89806380a229e36287ad9daf6f0ea2b5453eb) 修复索引工具使用配置公共部分（lemmatizer_base 选项）用于命令（dumpheader）
* [Commit 6dbe](https://github.com/manticoresoftware/manticoresearch/commit/6dbeaf2389d2af6a9dfccb56bfc986685b9f203e) 修复结果集和过滤器中的空字符串
* [Commit 39c4](https://github.com/manticoresoftware/manticoresearch/commit/39c4eb55ecc85f6dd54f6c8c6d6dfcf87fd8748e) 修复负文档 ID 值
* [Commit 266b](https://github.com/manticoresoftware/manticoresearch/commit/266b7054f98e7d30fc2829b5c2467dc9f90def92) 修复非常长的单词索引的单词剪切长度
* [Commit 4782](https://github.com/manticoresoftware/manticoresearch/commit/47823b0bd06cbd016a24345c99bb8693790b43dd) 修复 PQ 中的通配符查询的多个文档匹配


## 版本 2.6.4 GA, 2018年5月3日
### 功能和改进
* MySQL FEDERATED 引擎 [支持](Extensions/FEDERATED.md)
* MySQL 数据包现在返回 SERVER_STATUS_AUTOCOMMIT 标志，增加了与 ProxySQL 的兼容性
* [listen_tfo](Server_settings/Searchd.md#listen_tfo) - 为所有监听器启用 TCP Fast Open 连接
* indexer --dumpheader 现在可以从 .meta 文件中转储 RT 头信息
* Ubuntu Bionic 的 cmake 构建脚本

### 错误修复
* [Commit 355b](https://github.com/manticoresoftware/manticoresearch/commit/355b11629174813abd9cd5bf2233be0783f77745) 修复 RT 索引的无效查询缓存条目；
* [Commit 546e](https://github.com/manticoresoftware/manticoresearch/commit/546e2297a01cc2913bc0d33052d5bcefae8737eb) 修复无缝旋转后索引设置丢失
* [Commit 0c45](https://github.com/manticoresoftware/manticoresearch/commit/0c4509898393993bc87194d2aca2070395ff7f83) 修复固定中缀与前缀长度设置；在不支持的中缀长度上添加警告
* [Commit 8054](https://github.com/manticoresoftware/manticoresearch/commit/80542fa54d727c781635918e8d4d65a20c2fbd1e) 修复 RT 索引的自动刷新顺序
* [Commit 705d](https://github.com/manticoresoftware/manticoresearch/commit/705d8c5f407726ed1c6f9745ecffa375682c8969) 修复具有多个属性和查询多个索引的索引的结果集模式问题
* [Commit b0ba](https://github.com/manticoresoftware/manticoresearch/commit/b0ba932fff4a082f642cb737381c8a70215d4cc9) 修复在文档重复的批量插入中丢失的一些命中
* [Commit 4510](https://github.com/manticoresoftware/manticoresearch/commit/4510fa44ad54b5bbac53c591144501565130fcae) 修复优化失败以合并文档数量大的 RT 索引的磁盘块

## 版本 2.6.3 GA, 2018年3月28日
### 改进
* 编译时使用 jemalloc。如果系统存在 jemalloc，可以使用 cmake 标志 `-DUSE_JEMALLOC=1` 启用

### 错误修复
* [Commit 85a6](https://github.com/manticoresoftware/manticoresearch/commit/85a6d7e3db75f28c491042409bb677ed39eba07b) 修复日志 expand_keywords 选项到 Manticore SQL 查询日志
* [Commit caaa](https://github.com/manticoresoftware/manticoresearch/commit/caaa3842f4dbd8507fd9e20bf3371a0c8ec0c087) 修复 HTTP 接口以正确处理大型查询
* [Commit e386](https://github.com/manticoresoftware/manticoresearch/commit/e386d842b36195719e459b2db1fb20e402f9a51f) 修复在启用 index_field_lengths 的 RT 索引上进行 DELETE 时服务器崩溃
* [Commit cd53](https://github.com/manticoresoftware/manticoresearch/commit/ce528f37eb5d8284661928f6f3b340a4a1985253) 修复 cpustats searchd cli 选项以在不支持的系统上工作
* [Commit 8740](https://github.com/manticoresoftware/manticoresearch/commit/8740fd63ae842c2ab0e51a8fcd6180d787ed57ec) 修复了定义最小长度的utf8子字符串匹配


## 版本 2.6.2 GA，2018年2月23日
### 改进
* 在使用NOT操作符和批处理文档的情况下，改进了[Percolate Queries](Searching/Percolate_query.md)的性能。
* [percolate_query_call](Searching/Percolate_query.md)可以根据[dist_threads](Server_settings/Searchd.md#threads)使用多个线程
* 新的全文匹配操作符NOTNEAR/N
* 对于渗透索引的SELECT增加LIMIT
* [expand_keywords](Searching/Options.md#expand_keywords)可以接受'start'，'exact'（其中'star,exact'与'1'具有相同效果）
* 对于[joined fields](Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Processing_fetched_data.md#sql_joined_field)的范围主查询，使用sql_query_range定义的范围查询

### 修复
* [Commit 72dc](https://github.com/manticoresoftware/manticoresearch/commit/72dcf669744e9b7d636dfc213d24df85ab301f6b) 修复了在搜索ram段时崩溃；在双缓冲时保存磁盘块的死锁；在优化期间保存磁盘块时的死锁
* [Commit 3613](https://github.com/manticoresoftware/manticoresearch/commit/36137149a1c3c0893bdda5a28fc7e8244bf2d4ae) 修复了在xml嵌入式模式中遇到空属性名称时的索引器崩溃
* [Commit 48d7](https://github.com/manticoresoftware/manticoresearch/commit/48d7e8001d2a66466ca64577f27ddc5421a67251) 修复了未拥有的pid-file错误解链接
* [Commit a556](https://github.com/manticoresoftware/manticoresearch/commit/a5563a465ddc59ef71e65f17b68bc33f9700e838) 修复了在临时文件夹中有时留下的孤儿fifo
* [Commit 2376](https://github.com/manticoresoftware/manticoresearch/commit/2376e8fc4508944b96959bd10686c6d51f5145e8) 修复了错误的NULL行导致的空FACET结果集
* [Commit 4842](https://github.com/manticoresoftware/manticoresearch/commit/4842b67acdd5da75672db6c08ac563b48577ddd7) 修复了作为Windows服务运行时的损坏索引锁
* [Commit be35](https://github.com/manticoresoftware/manticoresearch/commit/be35feef54ded59125750916cd211e56108eddae) 修复了mac os上的错误iconv库
* [Commit 8374](https://github.com/manticoresoftware/manticoresearch/commit/83744a977d7837d1d87cd506b88b0e7eb199efe6) 修复了错误的count(*)


## 版本 2.6.1 GA，2018年1月26日
### 改进
* 在有镜像的代理情况下，[agent_retry_count](Server_settings/Searchd.md#agent_retry_count)给出了每个镜像的重试次数，而不是每个代理的重试次数，每个代理的总重试次数为agent_retry_count*mirrors。
* 现可按索引指定[agent_retry_count](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_retry_count)，覆盖全局值。增加了一个别名[mirror_retry_count](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#mirror_retry_count)。
* 可以在代理定义中指定重试次数，该值表示每个代理的重试次数
* 现在Percolate Queries在HTTP JSON API中可用，地址为[/json/pq](Searching/Percolate_query.md#Performing-a-percolate-query-with-CALL-PQ)。
* 为可执行文件添加-h和-v选项（帮助和版本）
* [morphology_skip_fields](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology)对实时索引的支持

### 修复
* [Commit a40b](https://github.com/manticoresoftware/manticore/commit/a40b0793feff65e40d10062568d9847c08d10f57) 修复了与sql_range_step一起在MVA字段上正确工作的范围主查询
* [Commit f2f5](https://github.com/manticoresoftware/manticore/commit/f2f53757db45bcfb1544263ce0817e856656a621) 修复了黑洞系统循环挂起的问题，以及黑洞代理似乎断开的情况
* [Commit 84e1](https://github.com/manticoresoftware/manticore/commit/84e1f54aef25e0fce98870ad2dd784db5116f1d6) 修复了查询ID的一致性，修复了存储查询的重复ID
* [Commit 1948](https://github.com/manticoresoftware/manticore/commit/19484231814fcb82b21763a3a4a9f45adc6b2d40) 修复了在不同状态下关闭时的服务器崩溃
* [Commit 9a70](https://github.com/manticoresoftware/manticore/commit/9a706b499a1d61a90076065a1a703029d49db958)  [Commit 3495](https://github.com/manticoresoftware/manticore/commit/3495fd70cba8846b1a50d55d6679b039414c3d2a) 长查询的超时
* [Commit 3359](https://github.com/manticoresoftware/manticore/commit/3359bcd89b4f79a645fe84b8cf8616ce0addff02) 在基于kqueue的系统（Mac OS X，BSD）上重构了主代理网络轮询


## 版本 2.6.0，2017年12月29日
### 特性和改进
* [HTTP JSON](Connecting_to_the_server/HTTP.md)：JSON查询现在可以对属性进行等值比较，MVA和JSON属性可以在插入和更新中使用，通过JSON API进行的更新和删除可以在分布式索引上执行
* [Percolate Queries](Searching/Percolate_query.md)
* 从代码中移除了对32位docids的支持。同时删除了所有转换/加载32位docids的遗留索引的代码。
* [仅对某些字段进行形态学处理](https://github.com/manticoresoftware/manticore/issues/7)。新的索引指令morphology_skip_fields允许定义形态学不适用的字段列表。
* [expand_keywords现在可以是使用OPTION语句设置的查询运行时指令](https://github.com/manticoresoftware/manticore/issues/8)

### 修复
* [Commit 0cfa](https://github.com/manticoresoftware/manticore/commit/0cfae4c) 修复了在使用rlp构建时服务器的调试版本崩溃（可能在发布版本下未定义行为）
* [Commit 3242](https://github.com/manticoresoftware/manticore/commit/324291e) 修复了在启用渐进选项时RT索引优化，合并错误顺序的kill-lists
* [Commit ac0e](https://github.com/manticoresoftware/manticore/commit/ac0efee)  修复了mac上的小崩溃
* 在全面的静态代码分析后进行的许多小修复
* 其他小的错误修复

### 升级
在此版本中，我们更改了主节点和代理之间通信所使用的内部协议。如果您在分布式环境中运行 Manticoresearch 和多个实例，请确保先升级代理，然后再升级主节点。

## 版本 2.5.1，2017 年 11 月 23 日
### 功能和改进
* 对 [HTTP API 协议](Connecting_to_the_server/HTTP.md) 的 JSON 查询。支持搜索、插入、更新、删除、替换操作。数据操作命令也可以批量处理，目前有一些限制，因为 MVA 和 JSON 属性不能用于插入、替换或更新。
* [RELOAD INDEXES](Data_creation_and_modification/Adding_data_from_external_storages/Rotating_a_table.md#RELOAD-TABLES) 命令
* [FLUSH LOGS](Logging/Rotating_query_and_server_logs.md) 命令
* [SHOW THREADS](Node_info_and_management/SHOW_THREADS.md) 可以显示优化、轮换或刷新过程的进度。
* GROUP N 按照 MVA 属性正确工作
* blackhole 代理在单独的线程上运行，不再影响主节点查询
* 实现了索引的引用计数，以避免因轮换和高负载而导致的停滞
* 实现了 SHA1 哈希，目前尚未对外暴露
* 修复了在 FreeBSD、macOS 和 Alpine 上编译的问题

### 修复
* [提交 9897](https://github.com/manticoresoftware/manticore/commit/989752b) 过滤回归与块索引
* [提交 b1c3](https://github.com/manticoresoftware/manticore/commit/b1c3864) 将 PAGE_SIZE 重命名为 ARENA_PAGE_SIZE，以兼容 musl
* [提交 f213](https://github.com/manticoresoftware/manticore/commit/f2133cc) 禁用 cmake < 3.1.0 的 googletests
* [提交 f30e](https://github.com/manticoresoftware/manticore/commit/0839de7) 服务器重启时无法绑定套接字
* [提交 0807](https://github.com/manticoresoftware/manticore/commit/0807240) 修复关机时服务器崩溃
* [提交 3e3a](https://github.com/manticoresoftware/manticore/commit/3e3acc3) 修复系统 blackhole 线程的显示线程问题
* [提交 262c](https://github.com/manticoresoftware/manticore/commit/262c3fe) 重构 iconv 的配置检查，修复在 FreeBSD 和 Darwin 上的构建问题

## 版本 2.4.1 GA，2017 年 10 月 16 日
### 功能和改进
* WHERE 子句中属性过滤之间的 OR 运算符
* 维护模式（SET MAINTENANCE=1）
* [CALL KEYWORDS](Searching/Autocomplete.md#CALL-KEYWORDS) 可用于分布式索引
* [协调 UTC](Server_settings/Searchd.md#grouping_in_utc)
* [query_log_mode](Server_settings/Searchd.md#query_log_mode) 用于自定义日志文件权限
* 字段权重可以为零或负数
* [max_query_time](Searching/Options.md#max_query_time) 现已影响全扫描
* 添加了 [net_wait_tm](Server_settings/Searchd.md#net_wait_tm)、[net_throttle_accept](Server_settings/Searchd.md#net_throttle_accept) 和 [net_throttle_action](Server_settings/Searchd.md#net_throttle_action) 以微调网络线程（在 workers=thread_pool 的情况下）
* COUNT DISTINCT 可以与切面搜索一起使用
* IN 可以与 JSON 浮点数组一起使用
* 整数/浮点表达式不再破坏多查询优化
* [SHOW META](Node_info_and_management/SHOW_META.md) 在使用多查询优化时显示 `multiplier` 行

### 编译
Manticore Search 使用 cmake 构建，编译所需的最低 gcc 版本为 4.7.2。

### 文件夹和服务
* Manticore Search 在 `manticore` 用户下运行。
* 默认数据文件夹现在是 `/var/lib/manticore/`。
