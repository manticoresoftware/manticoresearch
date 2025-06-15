# 更新日志

## 版本 10.1.0
**发布日期**：2025年6月9日

该版本代表了包含新功能、一个重大变更以及大量稳定性改进和错误修复的更新。更改重点是增强监控能力、改进搜索功能，并修复影响系统稳定性和性能的各种关键问题。

**从版本10.1.0开始，不再支持CentOS 7。建议用户升级到受支持的操作系统。**

### 重大变更
* ⚠️ [v10.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.0.0) [问题 #540](https://github.com/manticoresoftware/manticoresearch-buddy/issues/540) 重大变更：将 [模糊搜索](Searching/Spell_correction.md#Fuzzy-Search) 的默认 `layouts=''`

### 新功能和改进
* 🆕 [v10.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.0) [问题 #537](https://github.com/manticoresoftware/manticoresearch-buddy/issues/537) 新增内置的 [Prometheus导出器](Node_info_and_management/Node_status.md#Prometheus-Exporter)
* 🆕 [v9.8.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.8.0) [问题 #3409](https://github.com/manticoresoftware/manticoresearch/issues/3409) 新增 [ALTER TABLE tbl REBUILD KNN](Updating_table_schema_and_settings.md#Rebuilding-a-KNN-index)
* 🆕 [v9.7.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.7.0) [问题 #1778](https://github.com/manticoresoftware/manticoresearch/issues/1778) 新增自动嵌入生成（尚未正式发布，因为代码在主分支，需要更多测试）
* 🆕 [v9.6.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.6.0) 提升KNN API版本以支持自动嵌入
* 🆕 [v9.5.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.0) [问题 #1894](https://github.com/manticoresoftware/manticoresearch/issues/1894) 改进集群恢复：定期保存 `seqno` 以加快节点崩溃后重启速度
* 🆕 [v9.4.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.4.0) [问题 #2400](https://github.com/manticoresoftware/manticoresearch/issues/2400) 新增对最新 [Logstash](Integration/Logstash.md#Integration-with-Logstash) 和 Beats 的支持

### 错误修复
* 🪲 [v10.0.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.0.1) 修正词形处理：用户定义的词形现在覆盖自动生成的；添加测试用例至测试22
* 🪲 [v9.8.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.8.2) 修复：再次更新deps.txt，包括与嵌入库相关的MCL打包修复
* 🪲 [v9.8.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.8.1) 修复：更新deps.txt，修复MCL和嵌入库的打包问题
* 🪲 [v9.7.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.7.3) [问题 #3306](https://github.com/manticoresoftware/manticoresearch/issues/3306) 修复索引期间信号11崩溃问题
* 🪲 [v9.7.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.7.2) [问题 #3109](https://github.com/manticoresoftware/manticoresearch/issues/3109) 修复了不存在的 `@@variables` 总是返回0的问题
* 🪲 [v9.7.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.7.1) [问题 #3377](https://github.com/manticoresoftware/manticoresearch/issues/3377) 修复与 [remove_repeats()](Functions/Searching_and_ranking_functions.md#REMOVE_REPEATS%28%29) 相关的崩溃
* 🪲 [v9.6.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.6.3) [PR #3411](https://github.com/manticoresoftware/manticoresearch/pull/3411) 修复：为遥测指标使用动态版本检测
* 🪲 [v9.6.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.6.2) 修复：[SHOW VERSION](Node_info_and_management/SHOW_VERSION.md#SHOW-VERSION) 输出的小错误
* 🪲 [v9.6.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.6.1) 修复：创建带KNN属性但无模型的表时崩溃
* 🪲 [v9.5.16](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.16) [问题 #3342](https://github.com/manticoresoftware/manticoresearch/issues/3342) 修复 `SELECT ... FUZZY=0` 不总是禁用模糊搜索的问题
* 🪲 [v9.5.15](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.15) [PR #3397](https://github.com/manticoresoftware/manticoresearch/pull/3397) 支持 [MCL](https://github.com/manticoresoftware/columnar) 4.2.2；修复旧存储格式错误
* 🪲 [v9.5.14](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.14) [问题 #3392](https://github.com/manticoresoftware/manticoresearch/issues/3392) 修复HTTP JSON回复中字符串处理错误
* 🪲 [v9.5.13](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.13) [问题 #3356](https://github.com/manticoresoftware/manticoresearch/issues/3356) 修复复杂全文查询情况下的崩溃（common-sub-term）
* 🪲 [v9.5.12](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.12) 修正磁盘块自动刷新错误信息中的错字
* 🪲 [v9.5.11](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.11) [问题 #3195](https://github.com/manticoresoftware/manticoresearch/issues/3195) 改进 [自动磁盘块刷新](Securing_and_compacting_a_table/Flushing_RAM_chunk_to_disk.md#Flushing-RAM-chunk-to-disk)：优化运行时跳过保存
* 🪲 [v9.5.10](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.10) [问题 #3313](https://github.com/manticoresoftware/manticoresearch/issues/3313) 修复使用 [indextool](Miscellaneous_tools.md#indextool) 检查RT表所有磁盘块重复ID问题
* 🪲 [v9.5.9](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.9) [问题 #3132](https://github.com/manticoresoftware/manticoresearch/issues/3132) 在JSON API中增加对 `_random` 排序的支持
* 🪲 [v9.5.8](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.8) [ Issue #3382](https://github.com/manticoresoftware/manticoresearch/issues/3382) 修复问题：无法通过 JSON HTTP API 使用 uint64 文档 ID
* 🪲 [v9.5.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.7) [ Issue #3385](https://github.com/manticoresoftware/manticoresearch/issues/3385) 修复按 `id != value` 过滤时结果不正确的问题
* 🪲 [v9.5.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.6) [ PR #538](https://github.com/manticoresoftware/manticoresearch-buddy/pull/538) 修复某些情况下模糊匹配的严重错误
* 🪲 [v9.5.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.5) [ Issue #3199](https://github.com/manticoresoftware/manticoresearch/issues/3199) 修复 Buddy HTTP 查询参数中的空格解码（例如 `%20` 和 `+`）
* 🪲 [v9.5.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.4) [ Issue #3133](https://github.com/manticoresoftware/manticoresearch/issues/3133) 修复 facet 搜索中 `json.field` 排序错误
* 🪲 [v9.5.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.3) [ Issue #3091](https://github.com/manticoresoftware/manticoresearch/issues/3091) 修复 SQL 与 JSON API 中分隔符搜索结果不一致的问题
* 🪲 [v9.5.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.2) [ Issue #2819](https://github.com/manticoresoftware/manticoresearch/issues/2819) 性能提升：将分布式表的 `DELETE FROM` 替换为 `TRUNCATE`
* 🪲 [v9.5.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.1) [ Issue #3080](https://github.com/manticoresoftware/manticoresearch/issues/3080) 修复使用 JSON 属性时过滤别名的 `geodist()` 导致的崩溃

## 版本 9.3.2
发布：2025年5月2日

此版本包含多个错误修复和稳定性改进，更好的表使用跟踪，以及内存与资源管理的增强。

❤️ 特别感谢 [@cho-m](https://github.com/cho-m) 修复与 Boost 1.88.0 编译兼容性问题，以及 [@benwills](https://github.com/benwills) 改进了 `stored_only_fields` 的文档说明。

* 🪲 [v9.3.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.3.2)  修复“显示线程”列显示 CPU 活动为浮点数而非字符串的问题；同时修复 PyMySQL 结果集由于错误数据类型导致的解析错误。
* 🪲 [v9.3.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.3.1) [ Issue #3343](https://github.com/manticoresoftware/manticoresearch/issues/3343) 修复优化过程被中断时残留的 `tmp.spidx` 文件。
* 🆕 [v9.3.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.3.0) [ PR #3337](https://github.com/manticoresoftware/manticoresearch/pull/3337) 添加了每表命令计数器和详细的表使用统计。
* 🪲 [v9.2.39](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.39) [ Issue #3236](https://github.com/manticoresoftware/manticoresearch/issues/3236) 修复：防止复杂块更新导致表损坏。序列工作线程中使用等待函数破坏了序列处理，可能导致表损坏。
重新实现了自动刷新。移除外部轮询队列以避免不必要的表锁。新增“小表”条件：如果文档数量低于“小表限制”（8192）且未使用 Secondary Index (SI)，则跳过刷新。

* 🪲 [v9.2.38](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.38)  修复：跳过为使用 `ALL`/`ANY` 过滤字符串列表的筛选器创建 Secondary Index (SI)，不影响 JSON 属性。
* 🪲 [v9.2.37](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.37) [ Issue #2898](https://github.com/manticoresoftware/manticoresearch/issues/2898) 为系统表添加了反引号支持。
* 🪲 [v9.2.36](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.36)  修复：老代码中对集群操作使用了占位符。解析器现清楚分隔表名和集群名字段。
* 🪲 [v9.2.35](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.35)  修复：取消引用单个 `'` 导致崩溃。
* 🪲 [v9.2.34](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.34) [ Issue #3090](https://github.com/manticoresoftware/manticoresearch/issues/3090) 修复：处理大文档 ID 时可能未找到的问题。
* 🪲 [v9.2.33](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.33)  修复：位向量大小使用无符号整数。
* 🪲 [v9.2.32](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.32)  修复：合并过程减少峰值内存使用。docid 到 rowid 的查找现在每文档使用 12 字节，而非 16 字节。例如：20亿文档需要 24GB RAM，而不是 36GB。
* 🪲 [v9.2.31](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.31) [ Issue #3238](https://github.com/manticoresoftware/manticoresearch/issues/3238) 修复：大型实时表中 `COUNT(*)` 计数不正确。
* 🪲 [v9.2.30](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.30)  修复：清零字符串属性时的未定义行为。
* 🪲 [v9.2.29](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.29)  小修正：改进警告文本。
* 🪲 [v9.2.28](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.28) [ Issue #3290](https://github.com/manticoresoftware/manticoresearch/issues/3290) 改进：增强 `indextool --buildidf`
* 🪲 [v9.2.27](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.27) [ Issue #3032](https://github.com/manticoresoftware/manticoresearch/issues/3032) 使用 Kafka 集成后，现在可以为特定 Kafka 分区创建源。
* 🪲 [v9.2.26](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.26) [ Issue #3301](https://github.com/manticoresoftware/manticoresearch/issues/3301) 修复：`ORDER BY` 和 `WHERE` 作用于 `id` 可能导致内存溢出（OOM）错误。
* 🪲 [v9.2.25](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.25) [ Issue #3171](https://github.com/manticoresoftware/manticoresearch/issues/3171) 修复：在具有多个磁盘块的 RT 表上使用带有多个 JSON 属性的 grouper 时，导致段错误崩溃。
* 🪲 [v9.2.24](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.24) [ Issue #3246](https://github.com/manticoresoftware/manticoresearch/issues/3246) 修复：`WHERE string ANY(...)` 查询在 RAM 块刷新后失败。
* 🪲 [v9.2.23](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.23) [ PR #518](https://github.com/manticoresoftware/manticoresearch-buddy/pull/518) 次要的自动分片语法改进。
* 🪲 [v9.2.22](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.22) [ Issue #2763](https://github.com/manticoresoftware/manticoresearch/issues/2763) 修复：使用 `ALTER TABLE` 时，全局 idf 文件未被加载。
* 🪲 [v9.2.21](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.21) 修复：全局 idf 文件可能很大。我们现在更快释放表以避免占用不必要的资源。
* 🪲 [v9.2.20](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.20) [ PR #3277](https://github.com/manticoresoftware/manticoresearch/pull/3277) 改进：更好的分片选项验证。

* 🪲 [v9.2.19](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.19) [ PR #3275](https://github.com/manticoresoftware/manticoresearch/pull/3275) 修复：与 Boost 1.88.0 的构建兼容性。
* 🪲 [v9.2.18](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.18) [ Issue #3228](https://github.com/manticoresoftware/manticoresearch/issues/3228) 修复：分布式表创建时崩溃（无效指针问题）。

* 🪲 [v9.2.17](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.17) [ PR #3272](https://github.com/manticoresoftware/manticoresearch/pull/3272) 修复：多行模糊 `FACET` 问题。
* 🪲 [v9.2.16](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.16) [ Issue #3063](https://github.com/manticoresoftware/manticoresearch/issues/3063) 修复：使用 `GEODIST` 函数时距离计算的错误。
* 🪲 [v9.2.15](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.15) [ Issue #3027](https://github.com/manticoresoftware/manticoresearch/issues/3027) 小幅改进：支持 Elastic 的 `query_string` 过滤格式。

## 版本 9.2.14
发布：2025年3月28日

### 小幅变动
* [Commit ](https://github.com/manticoresoftware/manticoresearch/commit/fe9473dc3f4fcfa0eaae748b538fe853f4ce078b) 实现了 `--mockstack` 标志以计算递归操作堆栈需求。新的 `--mockstack` 模式分析并报告递归表达式求值、模式匹配操作、过滤处理所需的堆栈大小。计算出的堆栈需求输出至控制台，便于调试和优化。
* [Issue #3058](https://github.com/manticoresoftware/manticoresearch/pull/3058) 默认启用 [boolean_simplify](Searching/Options.md#boolean_simplify)。
* [Issue #3172](https://github.com/manticoresoftware/manticoresearch/issues/3172) 新增配置项：`searchd.kibana_version_string`，对于使用特定版本 Kibana 或 OpenSearch Dashboards 并期望某特定 Elasticsearch 版本时非常有用。
* [Issue #3211](https://github.com/manticoresoftware/manticoresearch/issues/3211) 修复 [CALL SUGGEST](Searching/Spell_correction.md#CALL-QSUGGEST,-CALL-SUGGEST) 以支持双字符词。
* [Issue #490](https://github.com/manticoresoftware/manticoresearch-buddy/issues/490) 改进了 [fuzzy search](Searching/Spell_correction.md#Fuzzy-Search)：此前搜索 “def ghi” 若存在其他匹配文档，有时找不到 “defghi”。
* ⚠️ 破坏性变更 [Issue #3165](https://github.com/manticoresoftware/manticoresearch/issues/3165) 在某些 HTTP JSON 响应中，将 `_id` 改为 `id`，以保持一致性。请确保相应更新您的应用程序。
* ⚠️ 破坏性变更 [Issue #3186](https://github.com/manticoresoftware/manticoresearch/issues/3186) 在集群加入时增加了 `server_id` 检查，确保每个节点 ID 唯一。`JOIN CLUSTER` 操作现在可能因与现有节点的 `server_id` 重复而失败，并显示错误消息。为解决此问题，请确保复制集群中的每个节点具有唯一的 [server_id](Server_settings/Searchd.md#server_id)。您可以在配置文件的 "searchd" 部分将默认 [server_id](Server_settings/Searchd.md#server_id) 改为唯一值后再尝试加入集群。此更改更新了复制协议。如果您运行复制集群，需要：
  - 首先，干净地停止所有节点
  - 然后，使用 `--new-cluster` 参数启动最后停止的节点，可在 Linux 使用 `manticore_new_cluster` 工具。
  - 详见关于 [重新启动集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster) 的说明。

### 修复
* [Commit 6fda](https://github.com/manticoresoftware/manticoresearch/commit/6fdad3923dd6953f4b781943eed5ec5c28b7f808) 修复了等待后调度器丢失导致的崩溃；现在特定调度器如 `serializer` 得以正确恢复。
* [Commit c333](https://github.com/manticoresoftware/manticoresearch/commit/c333b277d4a504de5fe298e9ab570a9f9ea31e0a) 修复了右表连接权重无法用于 `ORDER BY` 子句的错误。
* [问题 #2644](https://github.com/manticoresoftware/manticoresearch/issues/2644) gcc 14.2.0：修复了对 `const knn::DocDist_t*&` 的 `lower_bound` 调用错误。❤️ 感谢 [@Azq2](https://github.com/Azq2) 的 PR。
* [问题 #3018](https://github.com/manticoresoftware/manticoresearch/issues/3018) 修复了在自动模式插入时处理大写表名的问题。
* [问题 #3119](https://github.com/manticoresoftware/manticoresearch/issues/3119) 修复了解码无效 base64 输入时的崩溃。
* [问题 #3121](https://github.com/manticoresoftware/manticoresearch/issues/3121) 修复了两个与 KNN 索引 ALTER 操作相关的问题：浮点向量现在保持其原始维度，KNN 索引现在正确生成。
* [问题 #3123](https://github.com/manticoresoftware/manticoresearch/issues/3123) 修复了在空 JSON 列上构建二级索引时的崩溃。
* [问题 #3138](https://github.com/manticoresoftware/manticoresearch/issues/3138) 修复了因重复条目导致的崩溃。
* [问题 #3151](https://github.com/manticoresoftware/manticoresearch/issues/3151) 修复：`fuzzy=1` 选项不能与 `ranker` 或 `field_weights` 一起使用的问题。
* [问题 #3163](https://github.com/manticoresoftware/manticoresearch/issues/3163) 修复了 `SET GLOBAL timezone` 无效的 bug。
* [问题 #3181](https://github.com/manticoresoftware/manticoresearch/issues/3181) 修复了当使用大于 2^63 的 ID 时文本字段值可能丢失的问题。
* [问题 #3189](https://github.com/manticoresoftware/manticoresearch/issues/3189) 修复：`UPDATE` 语句现在正确遵守 `query_log_min_msec` 设置。
* [问题 #3247](https://github.com/manticoresoftware/manticoresearch/issues/3247) 修复了保存实时磁盘块时的竞态条件，该问题可能导致 `JOIN CLUSTER` 失败。


## 版本 7.4.6
发布时间：2025年2月28日

### 主要变更
* [问题 #832](https://github.com/manticoresoftware/manticoresearch/issues/832) 集成了 [Kibana](Integration/Kibana.md)，以便更容易和更高效地进行数据可视化。

### 次要变更
* [问题 #1727](https://github.com/manticoresoftware/manticoresearch/issues/1727) 修复了 arm64 和 x86_64 之间的浮点精度差异。
* [问题 #2995](https://github.com/manticoresoftware/manticoresearch/issues/2995) 实现了连接批处理的性能优化。
* [问题 #3039](https://github.com/manticoresoftware/manticoresearch/issues/3039) 实现了直方图中 EstimateValues 的性能优化。
* [问题 #3099](https://github.com/manticoresoftware/manticoresearch/issues/3099) 添加了对 Boost 1.87.0 的支持。❤️ 感谢 [@cho-m](https://github.com/cho-m) 的 PR。
* [问题 #77](https://github.com/manticoresoftware/columnar/issues/77) 优化了创建多值过滤器时的块数据重用；向属性元数据添加了最小/最大值；实现了基于最小/最大值的过滤值预过滤。

### Bug修复
* [提交 73ac](https://github.com/manticoresoftware/manticoresearch/commit/73ac22f358a7a0e734b332c3943c86e6294c06c3) 修复了在连接查询中使用来自左表和右表属性的表达式时的处理；修复了右表的 index_weights 选项。
* [问题 #2915](https://github.com/manticoresoftware/manticoresearch/issues/2915) 在 `SELECT ... JOIN` 查询中使用 `avg()` 可能导致错误结果；现已修复。
* [问题 #2996](https://github.com/manticoresoftware/manticoresearch/issues/2996) 修复了启用连接批处理时隐式截断导致的错误结果集。
* [问题 #3031](https://github.com/manticoresoftware/manticoresearch/issues/3031) 修复了在守护进程关闭时活动块合并进行中发生的崩溃。
* [问题 #3037](http://github.com/manticoresoftware/manticoresearch/issues/3037) 修复了 `IN(...)` 可能产生错误结果的问题。
* [问题 #3038](https://github.com/manticoresoftware/manticoresearch/issues/3038) 在版本 7.0.0 中设置 `max_iops` / `max_iosize` 可能降低索引性能；现已修复。
* [问题 #3042](https://github.com/manticoresoftware/manticoresearch/issues/3042) 修复了连接查询缓存中的内存泄漏。
* [问题 #3052](https://github.com/manticoresoftware/manticoresearch/issues/3052) 修复了连接 JSON 查询中查询选项的处理。
* [问题 #3054](https://github.com/manticoresoftware/manticoresearch/issues/3054) 修复了 ATTACH TABLE 命令的问题。
* [问题 #3079](https://github.com/manticoresoftware/manticoresearch/issues/3079) 修复了错误信息的不一致性。
* [问题 #3087](https://github.com/manticoresoftware/manticoresearch/issues/3087) 为每个表设置 `diskchunk_flush_write_timeout=-1` 未禁用索引刷新；现已修复。
* [问题 #3088](https://github.com/manticoresoftware/manticoresearch/issues/3088) 解决了批量替换大 ID 后的重复条目。
* [问题 #3126](https://github.com/manticoresoftware/manticoresearch/issues/3126) 修复了全文查询中使用单个 `NOT` 运算符和表达式排序器时导致的守护进程崩溃。
* [问题 #3128](https://github.com/manticoresoftware/manticoresearch/pull/3128) 修复了 CJSON 库中的潜在漏洞。❤️ 感谢 [@tabudz](https://github.com/tabudz) 的 PR。

## 版本 7.0.0
发布时间：2025年1月30日

### 主要变更
* [问题 #1497](https://github.com/manticoresoftware/manticoresearch/issues/1497) 新增了更易搜索的 [模糊搜索](Searching/Spell_correction.md#Fuzzy-Search) 和 [自动完成](Searching/Autocomplete.md#CALL-AUTOCOMPLETE) 功能。
* [问题 #1500](https://github.com/manticoresoftware/manticoresearch/issues/1500) [集成 Kafka](Integration/Kafka.md#Syncing-from-Kafka)。
* [问题 #1928](https://github.com/manticoresoftware/manticoresearch/issues/1928) 引入了 JSON 的 [二级索引](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#json_secondary_indexes)。
* [Issue #2361](https://github.com/manticoresoftware/manticoresearch/issues/2361) 更新期间的更新和搜索不再被区块合并阻塞。
* [Issue #2787](https://github.com/manticoresoftware/manticoresearch/issues/2787) RT 表的自动[磁盘区块刷新](Server_settings/Searchd.md#diskchunk_flush_write_timeout)以提升性能；现在，我们会自动将内存区块刷新到磁盘区块，防止内存区块缺乏优化可能导致的性能问题，有时这取决于区块大小会引起不稳定。
* [Issue #2811](https://github.com/manticoresoftware/manticoresearch/issues/2811) [Scroll](Searching/Pagination.md#Scroll-Search-Option)选项，方便分页。
* [Issue #931](https://github.com/manticoresoftware/manticoresearch/issues/931) 与[Jieba](https://github.com/fxsjy/jieba)集成，实现更好的[中文分词](Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md)。

### 小改动
* ⚠️ 重大变更 [Issue #1111](https://github.com/manticoresoftware/manticoresearch/issues/1111) 修复了 RT 表中 `global_idf` 的支持。需要重建表。
* ⚠️ 重大变更 [Issue #2103](https://github.com/manticoresoftware/manticoresearch/issues/2103) 从内部 `cjk` 字符集移除了泰文字符。请相应更新字符集定义：如果你有 `cjk,non_cjk` 且泰文字对你很重要，改为 `cjk,thai,non_cjk`，或 `cont,non_cjk`，其中 `cont` 是所有连续脚本语言的新标识（即 `cjk` + `thai`）。使用[ALTER TABLE](Updating_table_schema_and_settings.md#Updating-table-FT-settings-in-RT-mode)修改已有表。
* ⚠️ 重大变更 [Issue #2468](https://github.com/manticoresoftware/manticoresearch/issues/2468) [CALL SUGGEST / QSUGGEST](Searching/Spell_correction.md#CALL-QSUGGEST,-CALL-SUGGEST)现支持分布式表。这提升了主/代理协议版本。如果你在分布式环境中运行多个实例，先升级代理，再升级主节点。
* ⚠️ 重大变更 [Issue #2889](https://github.com/manticoresoftware/manticoresearch/issues/2889) PQ [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META-for-PQ-tables)中的列名从 `Name` 改为 `Variable name`。
* ⚠️ 重大变更 [Issue #879](https://github.com/manticoresoftware/manticoresearch/issues/879) 引入了[每表二进制日志](Logging/Binary_logging.md#Per-table-binary-logging-configuration)和新选项：[binlog_common](Logging/Binary_logging.md#Binary-logging-strategies)、[binlog](Logging/Binary_logging.md#Per-table-binary-logging-configuration)用于 `create table` / `alter table`。升级前需干净关闭 Manticore 实例。
* ⚠️ 重大变更 [Issue #1789](https://github.com/manticoresoftware/manticoresearch/issues/1789) 修正了节点因复制协议版本错误加入集群时的错误信息。本次变更更新了复制协议。若运行复制集群，你需要：
  - 首先，干净关闭所有节点
  - 然后，使用 Linux 工具 `manticore_new_cluster` 以 `--new-cluster` 启动最后关闭的节点
  - 详细信息请参阅[重启集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster)。
* ⚠️ 重大变更 [Issue #2308](https://github.com/manticoresoftware/manticoresearch/issues/2308) 为[`ALTER CLUSTER ADD`和`DROP`](Creating_a_cluster/Setting_up_replication/Adding_and_removing_a_table_from_a_replication_cluster.md#Adding-and-removing-a-table-from-a-replication-cluster)支持多个表。此变更也影响复制协议。请参考前节升级指南。
* [Issue #2997](https://github.com/manticoresoftware/manticoresearch/issues/2997) 修复了 MacOS 上 dlopen 问题。
* [Commit 4954](https://github.com/manticoresoftware/manticoresearch/commit/4954b5de7341a29902a9b8fbb9a040f7942c77c4) 修改了带 KNN 索引表的 OPTIMIZE TABLE 默认截止点，提升搜索性能。
* [Commit cfc8](https://github.com/manticoresoftware/manticoresearch/commit/cfc87ecb6e33a8163c2235243b6b40e699dbf526) 为 `FACET` 和 `GROUP BY` 的 `ORDER BY` 添加了 `COUNT(DISTINCT)` 支持。
* [Issue #1103](https://github.com/manticoresoftware/manticoresearch/issues/1103) 提升了[日志](Logging/Server_logging.md#Server-logging)区块合并的清晰度。
* [Issue #1130](https://github.com/manticoresoftware/manticoresearch/issues/1130) 新增对[DBeaver](Integration/DBeaver.md)的支持。
* [Issue #1546](https://github.com/manticoresoftware/manticoresearch/issues/1546) 实现了[POLY2D()](Functions/Geo_spatial_functions.md#POLY2D%28%29)/[GEOPOLY2D()](Functions/Geo_spatial_functions.md#GEOPOLY2D%28%29)函数的二级索引。
* [Issue #1630](https://github.com/manticoresoftware/manticoresearch/issues/1630) HTTP 请求现支持 `Content-Encoding: gzip`。
* [Issue #1831](https://github.com/manticoresoftware/manticoresearch/issues/1831) 新增 `SHOW LOCKS` 命令。
* [Issue #2187](https://github.com/manticoresoftware/manticoresearch/issues/2197) 允许 Buddy 请求绕过守护进程的 [searchd.max_connections](Server_settings/Searchd.md#max_connections) 限制。
* [Issue #2208](https://github.com/manticoresoftware/manticoresearch/issues/2208) 支持通过 JSON HTTP 接口连接表。
* [Issue #2235](https://github.com/manticoresoftware/manticoresearch/issues/2235) 通过 Buddy 以原始形式记录成功处理的查询。
* [Issue #2249](https://github.com/manticoresoftware/manticoresearch/issues/2249) 为复制表增加特殊模式运行 `mysqldump`。
* [Issue #2268](https://github.com/manticoresoftware/manticoresearch/issues/2268) 改进了 `CREATE TABLE` 和 `ALTER TABLE` 语句中外部文件在复制时的重命名。
* [Issue #2402](https://github.com/manticoresoftware/manticoresearch/issues/2402) 更新了 [searchd.max_packet_size](Server_settings/Searchd.md#max_packet_size) 默认值为128MB。
* [Issue #2419](https://github.com/manticoresoftware/manticoresearch/issues/2419) 为JSON中的["match"](Searching/Full_text_matching/Basic_usage.md#match)添加了对[IDF增强修饰符](Searching/Full_text_matching/Operators.md#IDF-boost-modifier)的支持。
* [Issue #2430](https://github.com/manticoresoftware/manticoresearch/issues/2430) 增强了[binlog](Logging/Binary_logging.md#Binary-logging)写入的同步以防止错误。
* [Issue #2458](https://github.com/manticoresoftware/manticoresearch/issues/2458) 在Windows包中加入了zlib支持。
* [Issue #2479](https://github.com/manticoresoftware/manticoresearch/issues/2479) 增加了对SHOW TABLE INDEXES命令的支持。
* [Issue #2485](https://github.com/manticoresoftware/manticoresearch/issues/2485) 为Buddy回复设置了会话元数据。
* [Issue #2490](https://github.com/manticoresoftware/manticoresearch/issues/2490) 兼容接口中的聚合支持毫秒级分辨率。
* [Issue #2500](https://github.com/manticoresoftware/manticoresearch/issues/2500) 当复制启动失败时，更改了集群操作的错误信息。
* [Issue #2584](https://github.com/manticoresoftware/manticoresearch/pull/2584) 在SHOW STATUS中新增了[性能指标](Node_info_and_management/Node_status.md#Query-Time-Statistics)：最近1、5、15分钟内每种查询类型的最小/最大/平均/95百分位/99百分位统计数据。
* [Issue #2639](https://github.com/manticoresoftware/manticoresearch/pull/2639) 将请求和响应中的所有`index`替换为`table`。
* [Issue #2643](https://github.com/manticoresoftware/manticoresearch/issues/2643) 在HTTP `/sql`端点的聚合结果中添加了`distinct`列。
* [Issue #268](https://github.com/manticoresoftware/manticoresearch-buddy/issues/268) 实现了Elasticsearch导入数据类型的自动检测。
* [Issue #2744](https://github.com/manticoresoftware/manticoresearch/issues/2744) 添加了字符串JSON字段比较表达式的排序规则支持。
* [Issue #2752](https://github.com/manticoresoftware/manticoresearch/issues/2752) 支持在选择列表中使用`uuid_short`表达式。
* [Issue #2783](https://github.com/manticoresoftware/manticoresearch/issues/2783) Manticore Search现在直接运行Buddy，无需`manticore-buddy`包装器。
* [Issue #2785](https://github.com/manticoresoftware/manticoresearch/issues/2785) 针对表缺失和不支持插入操作的表，提供了不同的错误信息。
* [Issue #2789](https://github.com/manticoresoftware/manticoresearch/issues/2789) OpenSSL 3现已静态编译进`searchd`。
* [Issue #2790](https://github.com/manticoresoftware/manticoresearch/issues/2790) 新增`CALL uuid_short`语句，用于生成包含多个`uuid_short`值的序列。
* [Issue #2803](https://github.com/manticoresoftware/manticoresearch/issues/2803) 为JOIN操作中的右表添加了独立选项。
* [Issue #2810](https://github.com/manticoresoftware/manticoresearch/issues/2810) 提升HTTP JSON聚合性能，使其与SphinxQL中的`GROUP BY`性能相匹配。
* [Issue #2854](https://github.com/manticoresoftware/manticoresearch/issues/2854) 支持Kibana日期相关请求中的`fixed_interval`。
* [Issue #2909](https://github.com/manticoresoftware/manticoresearch/issues/2909) 实现了JOIN查询的批处理，大幅提升部分JOIN查询性能，提升幅度达数百甚至数千倍。
* [Issue #2937](https://github.com/manticoresoftware/manticoresearch/issues/2937) 支持在fullscan查询中使用连接表的权重。
* [Issue #2953](https://github.com/manticoresoftware/manticoresearch/issues/2953) 修复了join查询的日志记录问题。
* [Issue #337](https://github.com/manticoresoftware/manticoresearch-buddy/issues/337) 在非调试模式下隐藏Buddy异常，避免记录至`searchd`日志。
* [Issue #2931](https://github.com/manticoresoftware/manticoresearch/issues/2931) 当用户设置错误的复制监听端口时，守护进程会带有错误提示进行关闭。

### Bug 修复
* [Commit 0c6b](https://github.com/manticoresoftware/manticoresearch/commit/0c6bdaf41ea3684e05952e1a8273893535b923f5) 修复：含有超过32列的JOIN查询返回错误结果。
* [Issue #2335](https://github.com/manticoresoftware/manticoresearch/issues/2335) 解决了在条件中使用两个json属性时表连接失败的问题。
* [Issue #2338](https://github.com/manticoresoftware/manticoresearch/issues/2338) 修正了多查询中使用[cutoff](Searching/Options.md#cutoff)时total_relation不正确的问题。
* [Issue #2366](https://github.com/manticoresoftware/manticoresearch/issues/2366) 修正了[表连接](Searching/Joining.md)中右表按`json.string`过滤的问题。
* [Issue #2406](https://github.com/manticoresoftware/manticoresearch/issues/2406) 允许在任何POST HTTP JSON端点（insert/replace/bulk）中使用`null`作为所有值，此时将使用默认值。
* [Issue #2418](https://github.com/manticoresoftware/manticoresearch/issues/2418) 优化了内存消耗，调整了初始socket探测时对[最大包大小](Server_settings/Searchd.md#max_packet_size)网络缓冲区的分配。
* [Issue #2420](https://github.com/manticoresoftware/manticoresearch/issues/2420) 修复了通过JSON接口向bigint属性插入无符号整型的问题。
* [Issue #2422](https://github.com/manticoresoftware/manticoresearch/issues/2422) 修正了在启用exclude filters和pseudo_sharding时二级索引的正确工作。
* [Issue #2423](https://github.com/manticoresoftware/manticoresearch/issues/2423) 处理了[manticore_new_cluster](Starting_the_server/Manually.md#searchd-command-line-options)中的一个错误。
* [Issue #2448](https://github.com/manticoresoftware/manticoresearch/issues/2448) 解决了由于格式错误的`_update`请求导致守护进程崩溃的问题。
* [Issue #2452](https://github.com/manticoresoftware/manticoresearch/issues/2452) 修正了直方图无法处理带有排除值过滤器的问题。
* [Issue #55](https://github.com/manticoresoftware/columnar/issues/55) 修复了针对分布式表的 [knn](Searching/KNN.md#KNN-vector-search) 查询。
* [Issue #68](https://github.com/manticoresoftware/columnar/issues/68) 增强了列访问器中对表编码排除过滤器的处理。
* [Commit 0eb1](https://github.com/manticoresoftware/manticoresearch/commit/0eb1579462013cb134bcaf74e06f6eeb0c2ecc4d) 修正表达式解析器没有遵守重新定义的 `thread_stack`。
* [Commit c304](https://github.com/manticoresoftware/manticoresearch/commit/c3041fd1d3e14773f3c5aa635343915a71f5cb95) 修复克隆列式 IN 表达式时的崩溃。
* [Commit edad](https://github.com/manticoresoftware/columnar/commit/edadc694c68d6022bdd13134263667430a42cc1d) 修正了位图迭代器中的反转问题，该问题导致崩溃。
* [Commit fc30](https://github.com/manticoresoftware/executor/commit/fc302ff1117b0b835a6f5a3c07057baf5fba14d9) 修复某些 Manticore 软件包被 `unattended-upgrades` 自动移除的问题。
* [Issue #1019](https://github.com/manticoresoftware/manticoresearch/issues/1019) 改进了对 DbForge MySQL 工具查询的处理。
* [Issue #1107](https://github.com/manticoresoftware/manticoresearch/issues/1107) 修正了 `CREATE TABLE` 和 `ALTER TABLE` 中对特殊字符的转义。❤️ 感谢 [@subnix](https://github.com/subnix) 提交的 PR。
* [Issue #116](https://github.com/manticoresoftware/manticoresearch-backup/issues/116#issuecomment-2216597206) 修正了在冻结索引中更新 blob 属性时的死锁。死锁发生是因为尝试解冻索引时锁冲突，可能导致 manticore-backup 失败。
* [Issue #1818](https://github.com/manticoresoftware/manticoresearch/issues/1818) 当表被冻结时，`OPTIMIZE` 现在会抛出错误。
* [Issue #2001](https://github.com/manticoresoftware/manticoresearch/issues/2001) 允许使用函数名作为列名。
* [Issue #2153](https://github.com/manticoresoftware/manticoresearch/issues/2153) 修正查询具有未知磁盘块的表设置时守护进程崩溃的问题。
* [Issue #2184](https://github.com/manticoresoftware/manticoresearch/issues/2184) 修复 `searchd` 在执行 `FREEZE` 和 `FLUSH RAMCHUNK` 后停止时挂起的问题。
* [Issue #2228](https://github.com/manticoresoftware/manticoresearch/issues/2228) 从保留字中移除了日期/时间相关的标记（和正则表达式）。
* [Issue #2255](https://github.com/manticoresoftware/manticoresearch/issues/2255) 修正 `FACET` 中排序字段超过 5 个时的崩溃。
* [Issue #2265](https://github.com/manticoresoftware/manticoresearch/issues/2265) 修复启用 `index_field_lengths` 时恢复 `mysqldump` 失败的问题。
* [Issue #2291](https://github.com/manticoresoftware/manticoresearch/issues/2291) 修正执行 `ALTER TABLE` 命令时的崩溃。
* [Issue #2333](https://github.com/manticoresoftware/manticoresearch/issues/2333) 修复 Windows 包中 MySQL DLL 以使索引器正常工作。
* [Issue #2393](https://github.com/manticoresoftware/manticoresearch/issues/2393) 修正 GCC 编译错误。❤️ 感谢 [@animetosho](https://github.com/animetosho) 提交的 PR。
* [Issue #2447](https://github.com/manticoresoftware/manticoresearch/issues/2447) 修正 [_update](Data_creation_and_modification/Updating_documents/REPLACE.md?client=Elasticsearch-like+partial) 中的转义问题。
* [Issue #2460](https://github.com/manticoresoftware/manticoresearch/issues/2460) 修正声明多个相同名称属性或字段时索引器崩溃。
* [Issue #2467](https://github.com/manticoresoftware/manticoresearch/issues/2467) 解决 "compat" 搜索相关端点嵌套 bool 查询错误转换导致守护进程崩溃的问题。
* [Issue #2493](https://github.com/manticoresoftware/manticoresearch/issues/2493) 修正带修饰符短语中的扩展问题。
* [Issue #2535](https://github.com/manticoresoftware/manticoresearch/issues/2535) 解决使用 [ZONE](Searching/Full_text_matching/Operators.md#ZONE-limit-operator) 或 [ZONESPAN](Searching/Full_text_matching/Operators.md#ZONESPAN-limit-operator) 全文运算符时守护进程崩溃。
* [Issue #2552](https://github.com/manticoresoftware/manticoresearch/issues/2552) 修正带关键词字典的普通和 RT 表的中缀生成。
* [Issue #2571](https://github.com/manticoresoftware/manticoresearch/issues/2571) 修正 `FACET` 查询的错误回复；将 `FACET` 使用 `COUNT(*)` 时默认排序顺序设置为 `DESC`。
* [Issue #2580](https://github.com/manticoresoftware/manticoresearch/issues/2580) 修复 Windows 启动时守护进程崩溃。
* [Issue #2603](https://github.com/manticoresoftware/manticoresearch/issues/2603) 修正 HTTP 端点 `/sql` 和 `/sql?mode=raw` 的查询截断；使这些端点的请求一致，无需 `query=` 头。
* [Issue #2623](https://github.com/manticoresoftware/manticoresearch/issues/2623) 修正自动架构创建表但同时失败的问题。
* [Issue #2627](https://github.com/manticoresoftware/manticoresearch/issues/2627) 修正 HNSW 库以支持加载多个 KNN 索引。
* [Issue #2630](https://github.com/manticoresoftware/manticoresearch/issues/2630) 修正多重条件同时发生时的冻结问题。
* [Issue #2645](https://github.com/manticoresoftware/manticoresearch/issues/2645) 修正使用 `OR` 进行 KNN 搜索时的致命错误崩溃。
* [Issue #2647](https://github.com/manticoresoftware/manticoresearch/issues/2647) 修正 `indextool --mergeidf *.idf --out global.idf` 创建输出文件后删除输出文件的问题。
* [Issue #2658](https://github.com/manticoresoftware/manticoresearch/issues/2658) 修复外部查询带有 `ORDER BY` 字符串的子查询导致守护进程崩溃。
* [Issue #2686](https://github.com/manticoresoftware/manticoresearch/issues/2686) 修复了在更新浮点属性与字符串属性时导致的崩溃。
* [Issue #2704](https://github.com/manticoresoftware/manticoresearch/issues/2704) 修复了来自 `lemmatize_xxx_all` 分词器的多个停用词增加后续标记 `hitpos` 的问题。
* [Issue #2708](https://github.com/manticoresoftware/manticoresearch/issues/2708) 修复了 `ALTER ... ADD COLUMN ... TEXT` 操作时的崩溃问题。
* [Issue #2737](https://github.com/manticoresoftware/manticoresearch/issues/2737) 修复了在冻结表中更新 blob 属性（至少有一个 RAM chunk）时导致后续 `SELECT` 查询等待表解冻的问题。
* [Issue #2742](https://github.com/manticoresoftware/manticoresearch/issues/2742) 修复了包含打包因子的查询被跳过查询缓存的问题。
* [Issue #2775](https://github.com/manticoresoftware/manticoresearch/issues/2775) Manticore 现在对未知操作在 `_bulk` 请求时报告错误，而非崩溃。
* [Issue #2791](https://github.com/manticoresoftware/manticoresearch/issues/2791) 修复了 HTTP `_bulk` 端点返回插入文档 ID 的问题。
* [Issue #2797](https://github.com/manticoresoftware/manticoresearch/issues/2797) 修复了在处理多个表时 grouper 崩溃的问题，其中一个表为空，另一个表的匹配条目数量不同。
* [Issue #2835](https://github.com/manticoresoftware/manticoresearch/issues/2835) 修复了复杂的 `SELECT` 查询崩溃的问题。
* [Issue #2872](https://github.com/manticoresoftware/manticoresearch/issues/2872) 添加了错误提示，如果 `IN` 表达式中的 `ALL` 或 `ANY` 参数不是 JSON 属性。
* [Issue #2882](https://github.com/manticoresoftware/manticoresearch/issues/2882) 修复了在大表中更新 MVA 导致守护进程崩溃的问题。
* [Issue #2888](https://github.com/manticoresoftware/manticoresearch/issues/2888) 修复了 `libstemmer` 分词失败时的崩溃问题。❤️ 感谢 [@subnix](https://github.com/subnix) 的 PR。
* [Issue #2919](https://github.com/manticoresoftware/manticoresearch/issues/2919) 修复了表达式中右表连接权重未正确生效的问题。
* [Issue #2919](https://github.com/manticoresoftware/manticoresearch/issues/2919) 修复了表达式中右连接表权重不起作用的问题。
* [Issue #325](https://github.com/manticoresoftware/manticoresearch-buddy/issues/325) 修复了表已存在时，`CREATE TABLE IF NOT EXISTS ... WITH DATA` 失败的问题。
* [Issue #351](https://github.com/manticoresoftware/manticoresearch-buddy/issues/351) 修复了使用文档 ID 按 KNN 计数时出现的未定义数组键 "id" 错误。
* [Issue #359](https://github.com/manticoresoftware/manticoresearch-buddy/issues/359) 修复了 `REPLACE INTO cluster_name:table_name` 功能的问题。
* [Issue #67](https://github.com/manticoresoftware/docker/issues/67) 修复了以 `--network=host` 运行 Manticore Docker 容器时的严重错误。

## Version 6.3.8
发布日期：2024年11月22日

版本 6.3.8 继续 6.3 系列，仅包含错误修复。

### 错误修复

* [PR #2777](https://github.com/manticoresoftware/manticoresearch/pull/2777) 修正了当查询并发受 `threads` 或 `max_threads_per_query` 设置限制时，可用线程数的计算。

## manticore-extra v1.1.20

发布日期：2024年10月7日

### 错误修复

* [Issue #64](https://github.com/manticoresoftware/executor/issues/64) 解决了 Debian 系统上负责自动安装软件包更新的 `unattended-upgrades` 工具错误地标记多个 Manticore 软件包（包括 `manticore-galera`、`manticore-executor` 和 `manticore-columnar-lib`）为移除的情况。此问题是由于 `dpkg` 错误地将虚拟软件包 `manticore-extra` 视为多余。对此进行了修改，确保 `unattended-upgrades` 不再尝试移除关键的 Manticore 组件。

## Version 6.3.6
发布日期：2024年8月2日

版本 6.3.6 继续 6.3 系列，仅包含错误修复。

### 错误修复

* [Issue #2477](https://github.com/manticoresoftware/manticoresearch/issues/2477) 修复了在处理表达式和分布式或多表时，版本 6.3.4 引入的崩溃问题。
* [Issue #2352](https://github.com/manticoresoftware/manticoresearch/issues/2352) 修复了当查询多个索引时，由 `max_query_time` 导致提前退出引发的守护进程崩溃或内部错误。

## Version 6.3.4
发布日期：2024年7月31日

版本 6.3.4 继续 6.3 系列，包含一些小改进和错误修复。

### 小改进
* [Issue #1130](https://github.com/manticoresoftware/manticoresearch/issues/1130) 增加了对 [DBeaver](Integration/DBeaver.md#Integration-with-DBeaver) 的支持。
* [Issue #2146](https://github.com/manticoresoftware/manticoresearch/issues/2146) 改进了词形变化和例外中的分隔符转义。
* [Issue #2315](https://github.com/manticoresoftware/manticoresearch/issues/2315) 在 SELECT 列表达式中添加了字符串比较运算符。
* [Issue #2363](https://github.com/manticoresoftware/manticoresearch/issues/2363) 增加了对类似 Elastic 的批量请求中 null 值的支持。
* [Issue #2374](https://github.com/manticoresoftware/manticoresearch/issues/2374) 增加了对 mysqldump 版本 9 的支持。
* [Issue #2375](https://github.com/manticoresoftware/manticoresearch/issues/2375) 改进了 HTTP JSON 查询中 JSON 路径的错误处理，显示发生错误的节点。

### 错误修复
* [Issue #2280](https://github.com/manticoresoftware/manticoresearch/issues/2280) 修复了当 disk_chunks > 1 时，含有大量匹配项的通配符查询性能下降的问题。
* [Issue #2332](https://github.com/manticoresoftware/manticoresearch/issues/2332) 修复了空的 MVA 数组中 MVA 最小值或最大值 SELECT 列表达式导致的崩溃。
* [Issue #2339](https://github.com/manticoresoftware/manticoresearch/issues/2339) 修复了对 Kibana 无限范围请求的错误处理。
* [问题 #2342](https://github.com/manticoresoftware/manticoresearch/issues/2342) 修复了当属性不在 SELECT 列表中时，右表的列式属性上的连接过滤器。
* [问题 #2343](https://github.com/manticoresoftware/manticoresearch/issues/2343) 修复了 Manticore 6.3.2 中重复的 'static' 说明符。
* [问题 #2344](https://github.com/manticoresoftware/manticoresearch/issues/2344) 修复了使用 MATCH() 在右表上时，LEFT JOIN 返回非匹配条目的问题。
* [问题 #2350](https://github.com/manticoresoftware/manticoresearch/issues/2350) 修复了带有 `hitless_words` 的 RT 索引磁盘块保存问题。
* [问题 #2364](https://github.com/manticoresoftware/manticoresearch/issues/2364) 现在可以在其他属性中任意顺序添加 'aggs_node_sort' 属性。
* [问题 #2368](https://github.com/manticoresoftware/manticoresearch/issues/2368) 修复了 JSON 查询中全文与过滤顺序错误的问题。
* [问题 #2376](https://github.com/manticoresoftware/manticoresearch/issues/2376) 修复了长 UTF-8 请求导致的错误 JSON 响应的 Bug。
* [问题 #2684](https://github.com/manticoresoftware/dev/issues/2684) 修正了依赖于连接属性的预排序/预过滤表达式计算。
* [问题 #301](https://github.com/manticoresoftware/manticoresearch-buddy/issues/301) 更改了指标数据大小计算方法，改为从 `manticore.json` 文件读取，而不是检查数据目录的整体大小。
* [问题 #302](https://github.com/manticoresoftware/manticoresearch-buddy/issues/302) 增加了对 Vector.dev 验证请求的处理。

## 版本 6.3.2
发布日期：2024年6月26日

6.3.2 版本延续了 6.3 系列，并包含多个错误修复，其中一些是在 6.3.0 发布后发现的。

### 重大变更
* ⚠️[问题 #2305](https://github.com/manticoresoftware/manticoresearch/issues/2305) 更新了 aggs.range 值为数字类型。

### 错误修复
* [提交 c51c](https://github.com/manticoresoftware/manticoresearch/commit/c51c7fa4bd0ec4ad8d53188b888461bff29f1417) 修正了基于存储检查与 rset 合并的分组问题。
* [提交 0e85](https://github.com/manticoresoftware/manticoresearch/commit/0e85b9d5682f51681750d798aef6a11cfb93d69e) 修复了当在使用 CRC 字典且启用 `local_df` 的 RT 索引中使用通配符字符查询时，守护进程崩溃的问题。
* [问题 #2200](https://github.com/manticoresoftware/manticoresearch/issues/2200) 修复了 JOIN 中没有 GROUP BY 时，使用 `count(*)` 导致的崩溃。
* [问题 #2201](https://github.com/manticoresoftware/manticoresearch/issues/2201) 修正了在尝试按全文字段分组时，JOIN 未返回警告的问题。
* [问题 #2230](https://github.com/manticoresoftware/manticoresearch/issues/2230) 解决了通过 `ALTER TABLE` 添加属性时，未考虑 KNN 选项的问题。
* [问题 #2231](https://github.com/manticoresoftware/manticoresearch/issues/2231) 修复了 6.3.0 版本中无法删除 `manticore-tools` Redhat 包的问题。
* [问题 #2242](https://github.com/manticoresoftware/manticoresearch/issues/2242) 修正了 JOIN 和多个 FACET 语句返回错误结果的问题。
* [问题 #2250](https://github.com/manticoresoftware/manticoresearch/issues/2250) 修复了当表在集群中时，ALTER TABLE 产生错误的问题。
* [问题 #2252](https://github.com/manticoresoftware/manticoresearch/issues/2252) 修正了原始查询未正确传递给 buddy 的 SphinxQL 接口问题。
* [问题 #2267](https://github.com/manticoresoftware/manticoresearch/issues/2267) 改进了 RT 索引带有磁盘块时 `CALL KEYWORDS` 的通配符扩展。
* [问题 #271](https://github.com/manticoresoftware/manticoresearch-buddy/issues/271) 修复了错误的 `/cli` 请求导致的挂起。
* [问题 #274](https://github.com/manticoresoftware/manticoresearch-buddy/issues/274) 解决了并发请求 Manticore 时可能卡住的问题。
* [问题 #275](https://github.com/manticoresoftware/manticoresearch-buddy/issues/275) 修复了通过 `/cli` 执行 `drop table if exists t; create table t` 导致的挂起。

### 复制相关
* [问题 #2270](https://github.com/manticoresoftware/manticoresearch/issues/2270) 增加了 `/_bulk` HTTP 端点对 `cluster:name` 格式的支持。

## 版本 6.3.0
发布日期：2024年5月23日

### 主要变更
* [问题 #839](https://github.com/manticoresoftware/manticoresearch/issues/839) 实现了 [float_vector](Creating_a_table/Data_types.md#Float-vector) 数据类型；实现了 [向量搜索](Searching/KNN.md#KNN-vector-search)。
* [问题 #1673](https://github.com/manticoresoftware/manticoresearch/issues/1673) 支持 [INNER/LEFT JOIN](Searching/Joining.md)（**测试阶段**）。
* [问题 #1744](https://github.com/manticoresoftware/manticoresearch/issues/1744) 实现了对 [timestamp](Creating_a_table/Data_types.md#Timestamps) 字段日期格式的自动检测。
* [问题 #1720](https://github.com/manticoresoftware/manticoresearch/pull/1720) 将 Manticore Search 许可证从 GPLv2-or-later 改为 GPLv3-or-later。
* [提交 7a55](https://github.com/manticoresoftware/manticoresearch/commit/7a5508424) Windows 环境运行 Manticore 现在需要使用 Docker 来运行 Buddy。
* [问题 #1541](https://github.com/manticoresoftware/manticoresearch/issues/1541) 新增了 [REGEX](Searching/Full_text_matching/Operators.md#REGEX-operator) 全文操作符。
* [问题 #2091](https://github.com/manticoresoftware/manticoresearch/issues/2091) 支持 Ubuntu Noble 24.04。
* [提交 514d](https://github.com/manticoresoftware/manticoresearch/commit/514d35b497f4bdb20b6473ff963752a978a4bb8d) 重构了时间操作以提升性能，并新增日期/时间函数：
  - [CURDATE()](Functions/Date_and_time_functions.md#CURDATE%28%29) - 返回本地时区当前日期
  - [QUARTER()](Functions/Date_and_time_functions.md#QUARTER%28%29) - 返回时间戳参数所在年的季度整数
  - [DAYNAME()](Functions/Date_and_time_functions.md#DAYNAME%28%29) - 返回给定时间戳参数的星期名称
  - [MONTHNAME()](Functions/Date_and_time_functions.md#MONTHNAME%28%29) - 返回给定时间戳参数的月份名称
  - [DAYOFWEEK()](Functions/Date_and_time_functions.md#DAYOFWEEK%28%29) - 返回给定时间戳参数的整数星期索引
  - [DAYOFYEAR()](Functions/Date_and_time_functions.md#DAYOFYEAR%28%29) - 返回给定时间戳参数的整数年份中天数
  - [YEARWEEK()](Functions/Date_and_time_functions.md#YEARWEEK%28%29) - 返回给定时间戳参数的整数年份及当前周第一天的日期代码
  - [DATEDIFF()](Functions/Date_and_time_functions.md#DATEDIFF%28%29) - 返回两个给定时间戳之间的天数
  - [DATE()](Functions/Date_and_time_functions.md#DATE%28%29) - 格式化时间戳参数中的日期部分
  - [TIME()](Functions/Date_and_time_functions.md#TIME%28%29) - 格式化时间戳参数中的时间部分
  - [timezone](Server_settings/Searchd.md#timezone) - 日期/时间相关函数使用的时区。
* [Commit 30e7](https://github.com/manticoresoftware/manticoresearch/commit/30e789d9657f9b3093c280c6e5b0e46a021f6dc7) 向HTTP接口添加了 [range](Searching/Faceted_search.md#Facet-over-set-of-ranges)、[histogram](Searching/Faceted_search.md#Facet-over-histogram-values)、[date_range](Searching/Faceted_search.md#Facet-over-set-of-date-ranges) 和 [date_histogram](Searching/Faceted_search.md#Facet-over-histogram-date-values) 聚合，同时在SQL中增加了类似表达式。

### 小的更改
* [Issue #1285](https://github.com/manticoresoftware/manticoresearch/issues/1285) 支持 Filebeat 版本 8.10 - 8.11。
* [Issue #1771](https://github.com/manticoresoftware/manticoresearch/issues/1771) [ALTER TABLE ... type='distributed'](Updating_table_schema_and_settings.md#Changing-a-distributed-table)。
* [Issue #1788](https://github.com/manticoresoftware/manticoresearch/issues/1788) 增加了通过 [CREATE TABLE ... LIKE ... WITH DATA](Creating_a_table/Local_tables/Real-time_table.md#CREATE-TABLE-LIKE:) SQL语句复制表的能力。
* [Issue #2072](https://github.com/manticoresoftware/manticoresearch/issues/2072) 优化了[表压缩算法](Securing_and_compacting_a_table/Compacting_a_table.md#Compacting-a-Table)：之前手动的 [OPTIMIZE](Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE) 和自动的 [auto_optimize](Server_settings/Searchd.md#auto_optimize) 过程会先合并chunk以确保数量不超限，然后从包含删除文档的所有其他chunk中清除删除的文档。这种做法有时资源消耗过大，现已禁用。现在，chunk合并仅依赖于 [progressive_merge](Server_settings/Common.md#progressive_merge) 设置。然而，删除文档数较多的chunk更容易被合并。
* [Commit ce6c](https://github.com/manticoresoftware/columnar/commit/ce6c5d3381edcdc411a00a5923d13d56dd9b3010) 增加了对加载新版次级索引的保护。
* [Issue #1417](https://github.com/manticoresoftware/manticoresearch/issues/1417) 通过 [REPLACE INTO ... SET](Data_creation_and_modification/Updating_documents/REPLACE.md#SQL-REPLACE) 实现了部分替换。
* [Commit 7c16](https://github.com/manticoresoftware/manticoresearch/commit/7c167b934ce416d3dfbd658744a231e638b14d9a) 更新了默认合并缓冲区大小：`.spa`（标量属性）：256KB -> 8MB；`.spb`（Blob属性）：256KB -> 8MB；`.spc`（列式属性）：1MB，保持不变；`.spds`（文档存储）：256KB -> 8MB；`.spidx`（二级索引）：从256KB缓冲区改为128MB内存限制；`.spi`（字典）：256KB -> 16MB；`.spd`（文档列表）：8MB，保持不变；`.spp`（命中列表）：8MB，保持不变；`.spe`（跳跃列表）：256KB -> 8MB。
* [Issue #1859](https://github.com/manticoresoftware/manticoresearch/issues/1859) 通过JSON添加了[复合聚合](Searching/Grouping.md#GROUP-BY-multiple-fields-at-once)。
* [Commit 216b](https://github.com/manticoresoftware/manticoresearch-buddy/commit/216b824) 因某些正则表达式模式问题及无明显时间优势，禁用PCRE.JIT。
* [Commit 55cd](https://github.com/manticoresoftware/manticoresearch/commit/55cdce32) 添加了对vanilla Galera v.3 (api v25) (`libgalera_smm.so` 来自MySQL 5.x)的支持。
* [Commit 86f9](https://github.com/manticoresoftware/manticoresearch-buddy/commit/86f98ab) 将指标后缀从 `_rate` 改为 `_rps`。
* [Commit c0c1](https://github.com/manticoresoftware/manticoresearch-helm/commit/c0c1cff780031412ef57cefab987dfd16613368e) 改进了关于负载均衡器高可用性支持的文档。
* [Commit d1d2](https://github.com/manticoresoftware/manticoresearch/commit/d1d2ca18857c68106e89743756260e73285be739) 错误消息中将 `index` 改为 `table`；修复了bison解析器错误消息。
* [Commit fd26](https://github.com/manticoresoftware/manticoresearch/commit/fd26671b) 支持将 `manticore.tbl` 作为表名。
* [Issue #1105](https://github.com/manticoresoftware/manticoresearch/issues/1105) 支持通过 systemd 运行索引器（[文档](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Schedule-indexer-via-systemd)）。❤️ 感谢 [@subnix](https://github.com/subnix) 的PR。
* [Issue #1294](https://github.com/manticoresoftware/manticoresearch/issues/1294) GEODIST() 中支持二级索引。
* [Issue #1394](https://github.com/manticoresoftware/manticoresearch/issues/1394) 简化了 [SHOW THREADS](Node_info_and_management/SHOW_THREADS.md#SHOW-THREADS)。
* [Issue #1424](https://github.com/manticoresoftware/manticoresearch/issues/1424) 为 `create distributed table` 语句添加支持默认值（`agent_connect_timeout` 和 `agent_query_timeout`）。
* [Issue #1442](https://github.com/manticoresoftware/manticoresearch/issues/1442) 添加了搜索查询选项 [expansion_limit](Searching/Options.md#expansion_limit)，以覆盖 `searchd.expansion_limit`。
* [Issue #1448](https://github.com/manticoresoftware/manticoresearch/issues/1448) 实现了 [ALTER TABLE](Updating_table_schema_and_settings.md#Updating-table-schema-in-RT-mode) 用于 int->bigint 的转换。
* [Issue #1456](https://github.com/manticoresoftware/manticoresearch/issues/1456) MySQL 响应中的元信息。
* [Issue #1494](https://github.com/manticoresoftware/manticoresearch/issues/1494) [SHOW VERSION](Node_info_and_management/SHOW_VERSION.md#SHOW-VERSION)。
* [Issue #1582](https://github.com/manticoresoftware/manticoresearch/issues/1582) 支持通过 JSON 进行的[按 id 数组删除文档](Data_creation_and_modification/Deleting_documents.md?client=JSON#Deleting-documents)。
* [Issue #1589](https://github.com/manticoresoftware/manticoresearch/issues/1589) 改进错误提示“unsupported value type”。
* [Issue #1634](https://github.com/manticoresoftware/manticoresearch/issues/1634) 在 `SHOW STATUS` 中添加了 Buddy 版本信息。
* [Issue #1641](https://github.com/manticoresoftware/manticoresearch/issues/1641) 针对关键词零文档的匹配请求优化。
* [Issue #1712](https://github.com/manticoresoftware/manticoresearch/issues/1712) 添加了在提交数据时，字符串值 "true" 和 "false" 转换为 bool 属性的功能。
* [Issue #1713](https://github.com/manticoresoftware/manticoresearch/issues/1713) 添加了 [access_dict](Server_settings/Searchd.md#access_dict) 表和 searchd 选项。
* [Issue #1767](https://github.com/manticoresoftware/manticoresearch/issues/1767) 在 searchd 配置部分添加了新选项：[expansion_merge_threshold_docs](Server_settings/Searchd.md#expansion_merge_threshold_docs) 和 [expansion_merge_threshold_hits](Server_settings/Searchd.md#expansion_merge_threshold_hits)；使扩展词汇中微小词项合并的阈值可配置。
* [Issue #1768](https://github.com/manticoresoftware/manticoresearch/issues/1768) 在 `@@system.sessions` 中增加了最后命令时间的显示。
* [Issue #1806](https://github.com/manticoresoftware/manticoresearch/issues/1806) 升级 Buddy 协议至版本 2。
* [Issue #1810](https://github.com/manticoresoftware/manticoresearch/issues/1810) 实现了 `/sql` 端点的额外请求格式，便于与库集成。
* [Issue #1825](https://github.com/manticoresoftware/manticoresearch/pull/1825) 在 SHOW BUDDY PLUGINS 中添加了 Info 部分。
* [Issue #1837](https://github.com/manticoresoftware/manticoresearch/issues/1837) 改进了 `CALL PQ` 处理大包时的内存消耗。
* [Issue #1853](https://github.com/manticoresoftware/manticoresearch/pull/1853) 编译器由 Clang 15 切换至 Clang 16。
* [Issue #1857](https://github.com/manticoresoftware/manticoresearch/issues/1857) 增加字符串比较功能。❤️ 感谢 [@etcd](https://github.com/etcd) 的 PR。
* [Issue #1915](https://github.com/manticoresoftware/manticoresearch/issues/1915) 添加了对联合存储字段的支持。
* [Issue #1937](https://github.com/manticoresoftware/manticoresearch/issues/1937) 在查询日志中记录客户端的 host:port。
* [Issue #1981](https://github.com/manticoresoftware/manticoresearch/issues/1981) 修正错误信息。
* [Issue #1983](https://github.com/manticoresoftware/manticoresearch/issues/1983) 为 [通过 JSON 查询计划的详细级别](Node_info_and_management/Profiling/Query_plan.md?client=JSON#Query-plan)添加了详细等级支持。
* [Issue #2010](https://github.com/manticoresoftware/manticoresearch/issues/2010) 禁止记录 Buddy 查询日志，除非设置了 `log_level=debug`。
* [Issue #2035](https://github.com/manticoresoftware/manticoresearch/issues/2035) 支持 Linux Mint 21.3。
* [Issue #2056](https://github.com/manticoresoftware/manticoresearch/issues/2056) Manticore 无法用 Mysql 8.3+ 构建。
* [Issue #2112](https://github.com/manticoresoftware/manticoresearch/issues/2112) 对可能在附加包含重复条目的普通表后出现重复条目的实时表块，添加了 `DEBUG DEDUP` 命令。
* [Issue #212](https://github.com/manticoresoftware/manticoresearch-buddy/issues/212) 在 [SHOW QUERIES](Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES) 中添加时间显示。
* [Issue #218](https://github.com/manticoresoftware/manticoresearch-buddy/issues/218) 将 `@timestamp` 列处理为时间戳。
* [Issue #252](https://github.com/manticoresoftware/manticoresearch-buddy/issues/250) 实现了启用/禁用 buddy 插件的逻辑。
* [Issue #254](https://github.com/manticoresoftware/manticoresearch-buddy/issues/254) 更新 composer 至更新的版本，修复了近期 CVE。
* [Issue #340](https://github.com/manticoresoftware/manticoresearch/issues/340) 对 Manticore systemd 单元中与 `RuntimeDirectory` 相关的小幅优化。
* [Issue #51](https://github.com/manticoresoftware/executor/issues/51) 添加了 rdkafka 支持并更新至 PHP 8.3.3。
* [Issue #527](https://github.com/manticoresoftware/manticoresearch/issues/527) 支持[附加](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Attaching_one_table_to_another.md#Attaching-one-table-to-another)实时表。新增命令 [ALTER TABLE ... RENAME](Updating_table_schema_and_settings.md#Renaming-a-real-time-table)。

### 重大变更和废弃说明
* ⚠️[Issue #1436](https://github.com/manticoresoftware/manticoresearch/issues/1436) 修复了 IDF 计算问题。`local_df` 现为默认设置。改进了主从搜索协议（版本更新）。如果您在分布式环境中运行多个 Manticore Search 实例，请确保先升级代理，再升级主服务器。
* ⚠️[Issue #1572](https://github.com/manticoresoftware/manticoresearch/issues/1572) 添加了分布式表的复制并更新了复制协议。如果您运行复制集群，需要：
  - 首先，干净地停止所有节点
  - 然后，使用 Linux 下的工具 `manticore_new_cluster` 以 `--new-cluster` 启动最后停止的节点。
  - 阅读有关[重启集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster)的更多详细信息。
* ⚠️[问题 #1763](https://github.com/manticoresoftware/manticoresearch/pull/1763) HTTP API端点别名`/json/*`已被废弃。
* ⚠️[问题 #1982](https://github.com/manticoresoftware/manticoresearch/issues/1982) JSON中的[profile](Node_info_and_management/Profiling/Query_profile.md#Query-profile)已更改为[plan](Node_info_and_management/Profiling/Query_plan.md#Query-plan)，新增了JSON查询分析。
* ⚠️[提交 e235](https://github.com/manticoresoftware/manticoresearch-backup/commit/e23585a) manticore-backup不再备份`plugin_dir`。
* ⚠️[问题 #171](https://github.com/manticoresoftware/manticoresearch-buddy/issues/155) 将Buddy迁移到Swoole以提升性能和稳定性。切换到新版本时，请确保所有Manticore包已更新。
* ⚠️[问题 #196](https://github.com/manticoresoftware/manticoresearch-buddy/pull/205) 将所有核心插件合并至Buddy并更改核心逻辑。
* ⚠️[问题 #2107](https://github.com/manticoresoftware/manticoresearch/issues/2107) `/search`响应中将文档ID视为数字处理。
* ⚠️[问题 #38](https://github.com/manticoresoftware/executor/issues/38) 添加了Swoole，禁用了ZTS，移除了parallel扩展。
* ⚠️[问题 #1929](https://github.com/manticoresoftware/manticoresearch/issues/1929) 在`charset_table`中的覆盖在某些情况下不起作用。

### 复制相关变更
* [提交 3376](https://github.com/manticoresoftware/manticoresearch/commit/3376351f73db3ff0cb6cb708a304a7bd83e69cfb) 修复大型文件SST复制错误。
* [提交 6d36](https://github.com/manticoresoftware/manticoresearch/commit/6d36c68fb7ce3cb274b467aac435788905084952) 为复制命令添加重试机制；修复在网络拥堵且丢包时复制加入失败问题。
* [提交 842e](https://github.com/manticoresoftware/manticoresearch/commit/842e27e71790833ab4125d49103d2933d5df60c3) 将复制中的FATAL消息改为WARNING消息。
* [提交 8c32](https://github.com/manticoresoftware/manticoresearch/commit/8c32bce7db39d8ed3b1a06aaa1d2c3ec372084f0) 修正无表或空表复制集群中`gcache.page_size`的计算；修正Galera选项保存与加载。
* [提交 a2af](https://github.com/manticoresoftware/manticoresearch/commit/a2af06ca36abcbf893e43c8803853130b397c54b) 新增在加入集群节点跳过更新节点复制命令的功能。
* [提交 c054](https://github.com/manticoresoftware/manticoresearch/commit/c0541e6892f5116b1f957b40060ba8ecd01a5afb) 修复更新blob属性与替换文档复制时的死锁。
* [提交 e80d](https://github.com/manticoresoftware/manticoresearch/commit/e80d505b963999016033613c40c1cf3151d648e7) 新增[replication_connect_timeout](Server_settings/Searchd.md#replication_connect_timeout)、[replication_query_timeout](Server_settings/Searchd.md#replication_query_timeout)、[replication_retry_delay](Server_settings/Searchd.md#replication_retry_delay)、[replication_retry_count](Server_settings/Searchd.md#replication_retry_count)配置选项，用于控制复制时的网络，类似于`searchd.agent_*`但默认不同。
* [问题 #1356](https://github.com/manticoresoftware/manticoresearch/issues/1356) 修复复制节点在某些节点丢失且名称解析失败后的重试问题。
* [问题 #1445](https://github.com/manticoresoftware/manticoresearch/issues/1445) 修正了`show variables`中的复制日志详细级别。
* [问题 #1482](https://github.com/manticoresoftware/manticoresearch/issues/1482) 修复Kubernetes中pod重启后连接集群的加入节点复制问题。
* [问题 #1962](https://github.com/manticoresoftware/manticoresearch/issues/1962) 修复空集群上使用无效节点名进行复制变更时的长时间等待问题。

### Bug修复
* [提交 8a48](https://github.com/manticoresoftware/manticoresearch/commit/8a486c7ba1a293879cdecc41fb52eb7ab0cbf832) 修复`count distinct`中未使用匹配项清理可能导致崩溃的问题。
* [问题 #1569](https://github.com/manticoresoftware/manticoresearch/issues/1569) 二进制日志现在以事务粒度写入。
* [问题 #2089](https://github.com/manticoresoftware/manticoresearch/issues/2089) 修复64位ID相关的一个bug，该bug可能导致通过MySQL插入时出现“Malformed packet”错误，从而导致[表损坏和ID重复](https://github.com/manticoresoftware/manticoresearch/issues/2090)。
* [问题 #2160](https://github.com/manticoresoftware/manticoresearch/issues/2160) 修复日期以UTC而非本地时区插入的问题。
* [问题 #2177](https://github.com/manticoresoftware/manticoresearch/issues/2177) 修复在带非空`index_token_filter`的实时表搜索时崩溃的问题。
* [问题 #2209](https://github.com/manticoresoftware/manticoresearch/issues/2209) 更改实时列存储中的重复过滤，修复崩溃和错误查询结果。
* [提交 001d](https://github.com/manticoresoftware/manticoresearch/commit/001d82904e4b887c49387308291aeefb07c0fbc4) 修复html剥离器在处理联合字段后破坏内存的问题。
* [提交 00eb](https://github.com/manticoresoftware/manticoresearch/commit/00eb2ebf) 避免flush后回绕流，以防与mysqldump通信误差。
* [提交 0553](https://github.com/manticoresoftware/manticoresearch/commit/05534114acf08c70ffcc41a23d558dec09a34373) 如果预读取未开始，则不等待其完成。
* [提交 055a](https://github.com/manticoresoftware/manticoresearch/commit/055aac09f877570cd29b6594cb283182eba7f73b) 修复Buddy输出字符串过长，改为在searchd日志中分多行显示。
* [Commit 0a88](https://github.com/manticoresoftware/manticoresearch/commit/0a88056307f82c50fcdc458ad6dbc307a00e3f92) 将 MySQL 接口关于失败头部 `debugv` 详细级别的警告移至更合适位置。
* [Commit 150a](https://github.com/manticoresoftware/manticoresearch/commit/150a20286b7d98d24b844ae577d7fda4a444798d) 修复多个集群管理操作中的竞态条件；禁止创建同名或路径相同的多个集群。
* [Commit 2e40](https://github.com/manticoresoftware/manticoresearch/commit/2e405c1c9ebac6f46206e3df17ac1c19129c9aa3) 修复全文查询中隐式截断问题；将 MatchExtended 拆分为模板 partD。
* [Commit 75f5](https://github.com/manticoresoftware/manticoresearch/commit/75f51421542a2a0658f9e2f9f31048c8c5224479) 修正索引和加载表到守护进程时 `index_exact_words` 的不一致。
* [Commit 7643](https://github.com/manticoresoftware/manticoresearch/commit/7643976bea142e08535d701000eff0d09a99fda2) 修复删除无效集群时缺失的错误信息。
* [Commit 7a03](https://github.com/manticoresoftware/manticoresearch/commit/7a034ac990d5ec9344320c05a6a2315583b9b858) 修复 CBO 与队列联合使用的问题；修复 CBO 与 RT 伪分片的问题。
* [Commit 7b4e](https://github.com/manticoresoftware/manticoresearch/commit/7b4ea95e) 当启动时未加载辅助索引库（SI）和配置中未设置相关参数时，会发出误导性的警告信息“WARNING: secondary_indexes set but failed to initialize secondary library”。
* [Commit 8496](https://github.com/manticoresoftware/manticoresearch/commit/8496317f099fade8d2db463fb88483e7fcdfe85b) 修正 quorum 中的命中排序。
* [Commit 8973](https://github.com/manticoresoftware/manticoresearch-buddy/commit/8973ad3) 修复 ModifyTable 插件中大写选项的问题。
* [Commit 9935](https://github.com/manticoresoftware/manticoresearch-buddy/commit/99350d4c184379b19f3efa9751d9b5be5e6edfb5) 修复从包含空 json 值（表示为 NULL）的转储恢复时的问题。
* [Commit a28f](https://github.com/manticoresoftware/manticoresearch/commit/a28f6e087b3d957e48816070be17d0823904936a) 使用 pcon 修正接收 SST 时连接节点的 SST 超时问题。
* [Commit b5a5](https://github.com/manticoresoftware/manticoresearch/commit/b5a55cc26debcddcda5d9caef8366a707e0dd763) 修复选择别名字符串属性时的崩溃。
* [Commit c556](https://github.com/manticoresoftware/manticoresearch/commit/c556e7ed8b6da8eac8ef094650df5f0a8977916b) 添加对含有 `morphology_skip_fields` 字段的全文查询中，将术语转换为 `=term` 的查询转换。
* [Commit cdc3](https://github.com/manticoresoftware/manticoresearch/commit/cdc38085145cd1025b96fcac2a46188a0425d9e1) 补充缺失的配置键（skiplist_cache_size）。
* [Commit cf6e](https://github.com/manticoresoftware/manticoresearch/commit/cf6ec3bfdc3f0990fb31ff53822b2331a6667727) 修复表达式排序器处理大型复杂查询时的崩溃。
* [Commit e513](https://github.com/manticoresoftware/manticoresearch/commit/e5139ba75eef7223df375ae3cc3f138fa7d925f1) 修复全文 CBO 与无效索引提示符的问题。
* [Commit eb05](https://github.com/manticoresoftware/manticoresearch/commit/eb0514b3a6f57f278252b0724eebd112ec116e0f) 关闭时中断预读以加快关闭速度。
* [Commit f945](https://github.com/manticoresoftware/manticoresearch/commit/f94555a293cfa51004ff81135b8bd8500b955115) 修改全文查询的栈计算，避免复杂查询时崩溃。
* [Issue #1262](https://github.com/manticoresoftware/manticoresearch/issues/1262) 修复索引器在索引具有多个同名列的 SQL 源时崩溃的问题。
* [Issue #1273](https://github.com/manticoresoftware/manticoresearch/issues/1273) 对不存在的系统变量返回 0 而不是 <empty>。
* [Issue #1289](https://github.com/manticoresoftware/manticoresearch/issues/1289) 修复检查 RT 表外部文件时 indextool 出错的问题。
* [Issue #1335](https://github.com/manticoresoftware/manticoresearch/issues/1335) 修复因短语中包含多词形态导致的查询解析错误。
* [Issue #1364](https://github.com/manticoresoftware/manticoresearch/issues/1364) 支持回放带有旧 binlog 版本的空 binlog 文件。
* [Issue #1365](https://github.com/manticoresoftware/manticoresearch/issues/1365) 修复删除最后一个空 binlog 文件的问题。
* [Issue #1372](https://github.com/manticoresoftware/manticoresearch/issues/1372) 修正启动守护进程时 `data_dir` 变更导致的当前工作目录中相对路径被转换为绝对路径的问题。
* [Issue #1393](https://github.com/manticoresoftware/manticoresearch/issues/1393) hn_small 中最慢时间性能下降：守护进程启动时获取/缓存 CPU 信息。
* [Issue #1395](https://github.com/manticoresoftware/manticoresearch/issues/1395) 修复索引加载时缺失外部文件的警告。
* [Issue #1402](https://github.com/manticoresoftware/manticoresearch/issues/1402) 修复全局分组器在释放数据指针属性时崩溃。
* [Issue #1403](https://github.com/manticoresoftware/manticoresearch/issues/1403) _ADDITIONAL_SEARCHD_PARAMS 参数无法生效。
* [Issue #1427](https://github.com/manticoresoftware/manticoresearch/issues/1427) 修复每表 `agent_query_timeout` 被默认查询选项 `agent_query_timeout` 替换的问题。
* [Issue #1444](https://github.com/manticoresoftware/manticoresearch/issues/1444) 修复使用 `packedfactors()` 并且每个匹配有多个值时，分组器和排序器崩溃的问题。
* [Issue #1458](https://github.com/manticoresoftware/manticoresearch/issues/1458) 频繁索引更新时 Manticore 崩溃。
* [Issue #1481](https://github.com/manticoresoftware/manticoresearch/issues/1481) 修复解析错误后清理解析查询时的崩溃。
* [Issue #1484](https://github.com/manticoresoftware/manticoresearch/issues/1484) 修复 HTTP JSON 请求未被路由到 buddy 的问题。
* [Issue #1499](https://github.com/manticoresoftware/manticoresearch/issues/1499) JSON 属性根值无法是数组。修复。
* [Issue #1507](https://github.com/manticoresoftware/manticoresearch/issues/1507) 修复了事务内表重建时的崩溃。
* [Issue #1515](https://github.com/manticoresoftware/manticoresearch/issues/1515) 修复了俄语词干短形式的扩展。
* [Issue #1579](https://github.com/manticoresoftware/manticoresearch/issues/1579) 修复了在 [date_format](Functions/Date_and_time_functions.md#DATE_FORMAT()) 表达式中 JSON 和 STRING 属性的使用。
* [Issue #1580](https://github.com/manticoresoftware/manticoresearch/issues/1580) 修复了多个别名映射到 JSON 字段时的分组器问题。
* [Issue #1594](https://github.com/manticoresoftware/manticoresearch/issues/1594) 修正 dev 中的 total_related 错误：修复了隐式的 cutoff 与 limit；改进了 json 查询中的全扫描检测。
* [Issue #1603](https://github.com/manticoresoftware/manticoresearch/issues/1603) 修复了所有日期表达式中 JSON 和 STRING 属性的使用问题。
* [Issue #1609](https://github.com/manticoresoftware/manticoresearch/issues/1609) 修复了使用 LEVENSHTEIN() 时的崩溃。
* [Issue #1612](https://github.com/manticoresoftware/manticoresearch/issues/1612) 修复了搜索查询解析错误并使用高亮时的内存损坏问题。
* [Issue #1614](https://github.com/manticoresoftware/manticoresearch/issues/1614) 禁用了对短于 `min_prefix_len` / `min_infix_len` 的词条的通配符扩展。
* [Issue #1617](https://github.com/manticoresoftware/manticoresearch/issues/1617) 修改行为：如果 Buddy 成功处理请求则不记录错误。
* [Issue #1635](https://github.com/manticoresoftware/manticoresearch/issues/1635) 修正了带 limit 设置的查询中搜索查询元信息的 total。
* [Issue #1640](https://github.com/manticoresoftware/manticoresearch/issues/1640) 修复了在纯模式下通过 JSON 使用大写表名的问题。
* [Issue #1643](https://github.com/manticoresoftware/manticoresearch/issues/1643) 提供了默认的 `SPH_EXTNODE_STACK_SIZE` 值。
* [Issue #1646](https://github.com/manticoresoftware/manticoresearch/issues/1646) 修复了针对 MVA 属性的 ALL/ANY 负过滤器的 SphinxQL 日志问题。
* [Issue #1660](https://github.com/manticoresoftware/manticoresearch/issues/1660) 修复了应用来自其他索引的 docid killlists。❤️ 感谢 [@raxoft](https://github.com/raxoft) 的贡献。
* [Issue #1668](https://github.com/manticoresoftware/manticoresearch/issues/1668) 修复了原始索引全扫描时（无任何索引迭代器）因提前退出导致的匹配遗漏；移除了普通行迭代器中的 cutoff。
* [Issue #1671](https://github.com/manticoresoftware/manticoresearch/issues/1671) 修复了在带 agent 和本地表的分布式表查询中 FACET 出错的问题。
* [Issue #1690](https://github.com/manticoresoftware/manticoresearch/issues/1690) 修复了大数值直方图估算时的崩溃。
* [Issue #1692](https://github.com/manticoresoftware/manticoresearch/issues/1692) 修复了 alter table tbl add column col uint 时的崩溃。
* [Issue #1710](https://github.com/manticoresoftware/manticoresearch/issues/1710) 修复了条件 `WHERE json.array IN (<value>)` 返回空结果的问题。
* [Issue #172](https://github.com/manticoresoftware/manticoresearch-buddy/issues/172) 修复了向 `/cli` 发送请求时 TableFormatter 的问题。
* [Issue #1742](https://github.com/manticoresoftware/manticoresearch/issues/1742) 修复了缺少 wordforms 文件时 `CREATE TABLE` 不会失败的问题。
* [Issue #1762](https://github.com/manticoresoftware/manticoresearch/issues/1762) 现在 RT 表中属性的顺序遵循配置顺序。
* [Issue #1765](https://github.com/manticoresoftware/manticoresearch/issues/1765) 修复了带 'should' 条件的 HTTP bool 查询返回不正确结果的问题。
* [Issue #1769](https://github.com/manticoresoftware/manticoresearch/issues/1769) 修复了使用 `SPH_SORT_ATTR_DESC` 和 `SPH_SORT_ATTR_ASC` 对字符串属性排序无效的问题。
* [Issue #177](https://github.com/manticoresoftware/manticoresearch-buddy/issues/177) 禁用了对 Buddy 的 curl 请求中的 `Expect: 100-continue` HTTP 头。
* [Issue #1791](https://github.com/manticoresoftware/manticoresearch/issues/1791) 修复了由 GROUP BY 别名导致的崩溃。
* [Issue #1792](https://github.com/manticoresoftware/manticoresearch/issues/1792) 修正了 Windows 上 SQL 元信息摘要显示错误时间的问题。
* [Issue #1794](https://github.com/manticoresoftware/manticoresearch/issues/1794) 修复了 JSON 查询单词性能急剧下降的问题。
* [Issue #1798](https://github.com/manticoresoftware/manticoresearch/issues/1798) 不兼容的过滤器未在 `/search` 报错的问题已修正。
* [Issue #1802](https://github.com/manticoresoftware/manticoresearch/issues/1802) 修复了 `ALTER CLUSTER ADD` 和 `JOIN CLUSTER` 操作的等待逻辑，避免了 `ALTER` 在群集添加表时与供体向加入节点发送表的竞争条件。
* [Issue #1811](https://github.com/manticoresoftware/manticoresearch/issues/1811) 修复了 `/pq/{table}/*` 请求的错误处理。
* [Issue #1816](https://github.com/manticoresoftware/manticoresearch/issues/1816) 修复了 `UNFREEZE` 在某些情况下无效的问题。
* [Issue #183](https://github.com/manticoresoftware/manticoresearch-buddy/pull/183) 修复了某些情况下 MVA 恢复的问题。
* [Issue #1849](https://github.com/manticoresoftware/manticoresearch/issues/1849) 修复了使用 MCL 时 indextool 关闭时崩溃的问题。
* [Issue #1866](https://github.com/manticoresoftware/manticoresearch/issues/1866) 修复了 `/cli_json` 请求不必要的 URL 解码问题。
* [Issue #1872](https://github.com/manticoresoftware/manticoresearch/issues/1872) 修改了守护进程启动时的 plugin_dir 设置逻辑。
* [Issue #1874](https://github.com/manticoresoftware/manticoresearch/issues/1874) 修复了 alter table ... exceptions 失败的问题。
* [Issue #1891](https://github.com/manticoresoftware/manticoresearch/issues/1891#issuecomment-2051941200) 修复了插入数据时 Manticore 的 signal 11 崩溃。
* [Issue #1920](https://github.com/manticoresoftware/manticoresearch/issues/1920) 降低了 [low_priority](Searching/Options.md#low_priority) 的限制。
* [问题 #1924](https://github.com/manticoresoftware/manticoresearch/issues/1924) Mysqldump + mysql 恢复漏洞。
* [问题 #1951](https://github.com/manticoresoftware/manticoresearch/issues/1951) 修正了在缺少本地表或代理描述不正确时分布式表错误创建的问题；现在会返回错误信息。
* [问题 #1972](https://github.com/manticoresoftware/manticoresearch/issues/1972) 实现了一个 `FREEZE` 计数器以避免冻结/解除冻结问题。
* [问题 #1980](https://github.com/manticoresoftware/manticoresearch/issues/1980) 遵守 OR 节点中的查询超时。之前在某些情况下 `max_query_time` 可能不工作。
* [问题 #1986](https://github.com/manticoresoftware/manticoresearch/issues/1986) 不能将新的重命名为当前的 [manticore.json]。
* [问题 #1988](https://github.com/manticoresoftware/manticoresearch/issues/1988) 全文查询可能忽略 `SecondaryIndex` CBO 提示。
* [问题 #1990](https://github.com/manticoresoftware/manticoresearch/issues/1990) 修正了 `expansion_limit`，以便对来自多个磁盘块或内存块的调用关键字切片最终结果集。
* [问题 #1994](https://github.com/manticoresoftware/manticoresearch/issues/1994) 错误的外部文件。
* [问题 #2021](https://github.com/manticoresoftware/manticoresearch/issues/2021) 停止 Manticore 后，可能会留下几个 manticore-executor 进程仍在运行。
* [问题 #2029](https://github.com/manticoresoftware/manticoresearch/issues/2029) 使用 Levenshtein 距离时崩溃。
* [问题 #2037](https://github.com/manticoresoftware/manticoresearch/issues/2037) 多次在空索引上运行 max 操作符后出现错误。
* [问题 #2052](https://github.com/manticoresoftware/manticoresearch/issues/2052) multi-group 使用 JSON.field 时崩溃。
* [问题 #2067](https://github.com/manticoresoftware/manticoresearch/issues/2067) Manticore 在错误请求 _update 时崩溃。
* [问题 #2069](https://github.com/manticoresoftware/manticoresearch/issues/2069) 修正了 JSON 接口中字符串过滤比较器用于闭合范围的问题。
* [问题 #2082](https://github.com/manticoresoftware/manticoresearch/issues/2082) 当 data_dir 路径位于符号链接上时，`alter` 操作失败。
* [问题 #2102](https://github.com/manticoresoftware/manticoresearch/pull/2102) 改进了 mysqldump 中对 SELECT 查询的特殊处理，以保证生成的 INSERT 语句与 Manticore 兼容。
* [问题 #2103](https://github.com/manticoresoftware/manticoresearch/issues/2103) 泰文字符的字符集错误。
* [问题 #2124](https://github.com/manticoresoftware/manticoresearch/issues/2124) 使用带有保留字的 SQL 语句时崩溃。
* [问题 #2154](https://github.com/manticoresoftware/manticoresearch/issues/2154) 带有 wordforms 的表无法导入。
* [问题 #2176](https://github.com/manticoresoftware/manticoresearch/issues/2176) 修正了当引擎参数设置为 'columnar' 并通过 JSON 添加重复 ID 时发生的崩溃。
* [问题 #223](https://github.com/manticoresoftware/manticoresearch-buddy/issues/223) 尝试插入没有模式也没有列名的文档时给出正确的错误信息。
* [问题 #239](https://github.com/manticoresoftware/manticoresearch-buddy/issues/239) 自动模式的多行插入可能失败。
* [问题 #399](https://github.com/manticoresoftware/manticoresearch/issues/399) 在索引时添加了错误信息，如果在数据源声明了 id 属性。
* [问题 #59](https://github.com/manticoresoftware/manticoresearch-helm/issues/59) Manticore 集群宕机。
* [问题 #68](https://github.com/manticoresoftware/manticoresearch-helm/issues/68) presence percolate 表时 optimize.php 会崩溃。
* [问题 #77](https://github.com/manticoresoftware/manticoresearch-helm/issues/77) 修正了在 Kubernetes 部署时出现的错误。
* [问题 #274](https://github.com/manticoresoftware/manticoresearch-buddy/issues/274) 修正了对 Buddy 的并发请求处理错误。

### 与 manticore-backup 相关
* [问题 #97](https://github.com/manticoresoftware/manticoresearch-backup/pull/97) 当可用时设置 VIP HTTP 端口为默认。
多项改进：改进版本检查和流式 ZSTD 解压；在恢复过程中添加版本不匹配的用户提示；修正恢复时对于不同版本的错误提示行为；增强了解压逻辑，直接从流读取而非先读到工作内存；添加了 `--force` 标志。
* [提交 3b35](https://github.com/manticoresoftware/manticoresearch-backup/commit/3b357ea) 在 Manticore 搜索启动后添加备份版本显示，以识别该阶段的问题。
* [提交 ad2e](https://github.com/manticoresoftware/manticoresearch-backup/commit/ad2e5da) 更新了连接守护进程失败的错误信息。
* [提交 ce5e](https://github.com/manticoresoftware/manticoresearch-backup/commit/ce5ea85) 修正将绝对根备份路径转为相对路径的问题，并移除了恢复时的可写性检查，以允许从不同路径恢复。
* [提交 db7e](https://github.com/manticoresoftware/manticoresearch-backup/commit/db7e2b9) 为文件迭代器添加排序，以确保各种情况下的一致性。
* [问题 #106](https://github.com/manticoresoftware/manticoresearch-backup/issues/105) 多配置的备份与恢复。
* [问题 #91](https://github.com/manticoresoftware/manticoresearch-backup/pull/91) 添加 defattr 以防止在 RHEL 安装后文件出现异常用户权限。
* [问题 #91](https://github.com/manticoresoftware/manticoresearch-backup/pull/91) 添加额外的 chown 以确保 Ubuntu 中文件默认为 root 用户。

### 与 MCL（列式、二级索引、KNN 库）相关
* [提交 f104](https://github.com/manticoresoftware/columnar/commit/f104a4709bbb144140d09a09bc0fc1db1895e331) 支持向量搜索。
* [Commit 2169](https://github.com/manticoresoftware/columnar/commit/216953beef6676f7e34c9f6f1a30c5980d746710) 修复了在中断的二级索引构建设置期间临时文件的清理问题。此修复解决了守护进程在创建 `tmp.spidx` 文件时超过打开文件限制的问题。
* [Commit 709b](https://github.com/manticoresoftware/columnar/commit/709b9acaaac97d9a1ca8796892f9ad432021c785) 对 columnar 和 SI 使用了独立的 streamvbyte 库。
* [Commit 1c26](https://github.com/manticoresoftware/manticoresearch/commit/1c2623763ced14bb94ad624dde9614a938408d89) 添加了列存储不支持 JSON 属性的警告。
* [Commit 3acd](https://github.com/manticoresoftware/columnar/commit/3acd6714ffce103447d7931dfc75bb1a7449f340) 修复了 SI 中数据解包问题。
* [Commit 574c](https://github.com/manticoresoftware/manticoresearch/commit/574c023152564fa72092c18ebd1f594abfd4b6d7) 修复了使用混合行存和列存存储保存磁盘块时的崩溃问题。
* [Commit e87f](https://github.com/manticoresoftware/columnar/commit/e87f088b458d4c5b1b6159fc72ed0d57b786891a) 修复了 SI 迭代器被提示已处理过的区块的问题。
* [Issue #1474](https://github.com/manticoresoftware/manticoresearch/issues/1474) 使用列存引擎时，行存多值属性（MVA）列的更新损坏。
* [Issue #1510](https://github.com/manticoresoftware/manticoresearch/issues/1510) 修复了在 `HAVING` 中聚合列存属性时的崩溃。
* [Issue #1519](https://github.com/manticoresoftware/manticoresearch/issues/1519) 修复了在 `expr` 排序器中使用列存属性时的崩溃。

### 相关 Docker
* ❗[Issue #42](https://github.com/manticoresoftware/docker/issues/42) 通过环境变量支持[纯索引](https://github.com/manticoresoftware/docker#building-plain-tables)。
* ❗[Issue #47](https://github.com/manticoresoftware/docker/issues/47) 通过环境变量改进了配置的灵活性。
* [Issue #54](https://github.com/manticoresoftware/docker/pull/54) 改进了 Docker 的[备份和恢复](https://github.com/manticoresoftware/docker#backup-and-restore)流程。
* [Issue #77](https://github.com/manticoresoftware/docker/issues/76) 改进入口脚本，使备份恢复仅在首次启动时处理。
* [Commit a27c](https://github.com/manticoresoftware/docker/commit/a27c048dd9e73d8d6bea8d9a4830cafb486b82c5) 修复了查询日志输出到标准输出的问题。
* [Issue #38](https://github.com/manticoresoftware/docker/issues/38) 在未设置 EXTRA 时静音 BUDDY 警告。
* [Issue #71](https://github.com/manticoresoftware/docker/pull/71) 修复了 `manticore.conf.sh` 中的主机名问题。

## 版本 6.2.12
发布日期：2023年8月23日

版本 6.2.12 继续 6.2 系列，解决了 6.2.0 发布后发现的问题。

### Bug 修复
* ❗[Issue #1351](https://github.com/manticoresoftware/manticoresearch/issues/1351) “Manticore 6.2.0 在 Centos 7 上无法通过 systemctl 启动”：将 `TimeoutStartSec` 从 `infinity` 修改为 `0`，以便更好兼容 Centos 7。
* ❗[Issue #1364](https://github.com/manticoresoftware/manticoresearch/issues/1364) “从 6.0.4 升级到 6.2.0 后崩溃”：为旧版 binlog 中的空 binlog 文件添加了重放功能。
* [PR #1334](https://github.com/manticoresoftware/manticoresearch/pull/1334) “修正 searchdreplication.cpp 中的拼写错误”：修正了 `searchdreplication.cpp` 中的拼写错误：beggining -> beginning。
* [Issue #1337](https://github.com/manticoresoftware/manticoresearch/issues/1337) “Manticore 6.2.0 WARNING: conn (local)(12), sock=8088: bailing on failed MySQL header, AsyncNetInputBuffer_c::AppendData: error 11 (Resource temporarily unavailable) return -1”：降低了 MySQL 接口关于头信息警告的详细日志级别至 logdebugv。
* [Issue #1355](https://github.com/manticoresoftware/manticoresearch/issues/1355) “当 node_address 无法解析时，join cluster 挂起”：改进了当某些节点不可达且名称解析失败时的复制重试。此修复应解决 Kubernetes 和 Docker 节点与复制相关的问题。增强了复制启动失败时的错误信息，并对测试模型 376 进行了更新。同时，为名称解析失败提供了清晰的错误信息。
* [Issue #1361](https://github.com/manticoresoftware/manticoresearch/issues/1361) “字符集 non_cjk 中’Ø’缺少小写映射”：调整了’Ø’字符的映射。
* [Issue #1365](https://github.com/manticoresoftware/manticoresearch/issues/1365) “clean stop 后 searchd 留下 binlog.meta 和 binlog.001”：确保正确删除最后一个空的 binlog 文件。
* [Commit 0871](https://github.com/manticoresoftware/manticoresearch/commit/0871070845e93ad702ae0dfb89b35e5f24cd0851)：修复了 Windows 上关于 `Thd_t` 构建中原子复制限制的问题。
* [Commit 1cc0](https://github.com/manticoresoftware/manticoresearch/commit/1cc04bedc174737118fd5a3663e35f4e95bd4f8c)：处理了 FT CBO 和 `ColumnarScan` 之间的问题。
* [Commit c6bf](https://github.com/manticoresoftware/manticoresearch/commit/c6bfb30d53e3f4b67f17769667733677a661c95b)：修正了测试 376，并为测试中 `AF_INET` 错误添加了替代方案。
* [Commit cbc3](https://github.com/manticoresoftware/manticoresearch/commit/cbc38705e0b4dcc9665e3e8cf64826290ebd11cf)：解决了复制期间更新 blob 属性与替换文档时的死锁问题。并且移除了提交时索引的读锁，因为它已在更底层上锁。

### 次要更改
* [Commit 4f91](https://github.com/manticoresoftware/manticoresearch/commit/4f913a36e84e01f950b2797051222b0b1094d6fe) 更新了手册中关于 `/bulk` 端点的信息。

### MCL
* 支持 [Manticore Columnar Library](https://github.com/manticoresoftware/columnar) v2.2.4

## 版本 6.2.0
发布日期：2023年8月4日

### 重大更改
* 查询优化器增强，支持全文查询，大幅提升搜索效率和性能。
* 集成于：
  - [mysqldump](https://dev.mysql.com/doc/refman/8.0/en/mysqldump.html) - 使用 `mysqldump` 进行[逻辑备份](Securing_and_compacting_a_table/Backup_and_restore.md#Backup-and-restore-with-mysqldump)
  - [Apache Superset](https://manticoresearch.com/blog/manticoresearch-apache-superset-integration/) 和 [Grafana](https://manticoresearch.com/blog/manticoresearch-grafana-integration/) 用于可视化存储在 Manticore 中的数据
  - [HeidiSQL](https://www.heidisql.com/) 和 [DBForge](https://www.devart.com/dbforge/) 方便与 Manticore 的开发
* 我们开始使用 [GitHub workflows](https://github.com/manticoresoftware/manticoresearch/actions)，让贡献者更容易使用核心团队在准备包时应用的相同持续集成（CI）流程。所有作业都可以在 GitHub 托管的运行器上运行，从而便于在你自己的 Manticore Search 分支中无缝测试更改。
* 我们开始使用 [CLT](https://github.com/manticoresoftware/clt) 测试复杂场景。例如，现在我们可以确保一次提交后的包能够在所有支持的 Linux 操作系统上正确安装。命令行测试器（CLT）提供了一种以交互模式记录测试并轻松重播的用户友好方式。
* 通过结合使用哈希表和 HyperLogLog，显著提升了 count distinct 操作的性能。
* 启用了包含二级索引的查询的多线程执行，线程数限制为物理 CPU 核心数。这应大幅提升查询执行速度。
* 调整了 `pseudo_sharding`，限制为空闲线程数。这一更新显著增强了吞吐性能。
* 用户现在可以通过配置设置指定[默认属性存储引擎](Server_settings/Searchd.md#engine)，更好地定制以匹配特定的工作负载需求。
* 支持 [Manticore Columnar Library 2.2.0](https://github.com/manticoresoftware/columnar/)，在[二级索引](Server_settings/Searchd.md#secondary_indexes)中修复了众多错误并进行了改进。

### 小改动
* [Buddy #153](https://github.com/manticoresoftware/manticoresearch-buddy/issues/153)：[/pq](Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md?client=JSON#Adding-rules-to-a-percolate-table) HTTP 端点现在作为 `/json/pq` HTTP 端点的别名。
* [提交 0bf1](https://github.com/manticoresoftware/manticoresearch/commit/0bf17d9e)：确保了 `upper()` 和 `lower()` 的多字节兼容性。
* [提交 2bb9](https://github.com/manticoresoftware/manticoresearch/commit/2bb92765)：对 `count(*)` 查询不再扫描索引，改为返回预计算值。
* [提交 3c84](https://github.com/manticoresoftware/manticoresearch/commit/3c84091f)：现在可以使用 `SELECT` 进行任意计算并显示 `@@sysvars`。与之前不同，您不再只限于一个计算。因此，诸如 `select user(), database(), @@version_comment, version(), 1+1 as a limit 10` 的查询将返回所有列。请注意，可选的 'limit' 总是被忽略。
* [提交 6aca](https://github.com/manticoresoftware/manticoresearch/commit/6aca32f0)：实现了 `CREATE DATABASE` 桩查询。
* [提交 9dc1](https://github.com/manticoresoftware/manticoresearch/commit/9dc12334)：执行 `ALTER TABLE table REBUILD SECONDARY` 时，二级索引现在始终重建，即使属性未更新。
* [提交 46ed](https://github.com/manticoresoftware/manticoresearch/commit/46edb089)：使用预计算数据的排序器现在会在使用 CBO 前被识别，以避免不必要的 CBO 计算。
* [提交 102a](https://github.com/manticoresoftware/manticoresearch/commit/102ac604)：实现了全文表达式栈的模拟和使用，防止守护进程崩溃。
* [提交 979f](https://github.com/manticoresoftware/manticoresearch/commit/979fa27c)：为不使用字符串/mvas/json 属性的匹配克隆代码添加了快速路径。
* [提交 a073](https://github.com/manticoresoftware/manticoresearch/commit/a0735ffe)：添加了对 `SELECT DATABASE()` 命令的支持，但它始终返回 `Manticore`。该添加对于与各种 MySQL 工具的集成非常重要。
* [提交 bc04](https://github.com/manticoresoftware/manticoresearch/commit/bc04908)：修改了 [/cli](Connecting_to_the_server/HTTP.md#/cli) 端点的响应格式，并添加了 `/cli_json` 端点作为之前 `/cli` 的功能。
* [提交 d70b](https://github.com/manticoresoftware/manticoresearch/commit/d70b0d58)：`thread_stack` 现在可以在运行时通过 `SET` 语句修改。提供了会话本地和守护进程范围两种变体。当前值可在 `show variables` 输出中访问。
* [提交 d96e](https://github.com/manticoresoftware/manticoresearch/commit/d96ec6b9)：代码已集成到 CBO 中，更准确地估计字符串属性过滤执行的复杂性。
* [提交 e77d](https://github.com/manticoresoftware/manticoresearch/commit/e77dd72f5a04531c352fad0d7afcd2a1cbae2510)：改进了 DocidIndex 成本计算，提高整体性能。
* [提交 f3ae](https://github.com/manticoresoftware/manticoresearch/commit/f3ae8bea)：类似 Linux 上的 'uptime'，加载指标现在可以在 `SHOW STATUS` 命令中查看。
* [提交 f3cc](https://github.com/manticoresoftware/manticoresearch/commit/f3cc0971)：`DESC` 和 `SHOW CREATE TABLE` 的字段及属性顺序现在与 `SELECT * FROM` 保持一致。
* [提交 f3d2](https://github.com/manticoresoftware/manticoresearch/commit/f3d248a6)：不同的内部解析器现在在各种错误中提供其内部助记码（例如 `P01`），这有助于识别哪个解析器导致错误，同时隐藏非必要的内部细节。
* [Issue #271](https://github.com/manticoresoftware/manticoresearch/issues/271) “有时 CALL SUGGEST 不会纠正单字母拼写错误”：改进了[SUGGEST/QSUGGEST](Searching/Spell_correction.md#CALL-QSUGGEST,-CALL-SUGGEST)对短词的处理行为：新增了选项 `sentence` 用于显示整句。
* [Issue #696](https://github.com/manticoresoftware/manticoresearch/issues/696) “启用词干处理时，渗透索引对精确短语查询的搜索不正确”：修改了渗透查询以处理精确词项修饰符，提升了搜索功能。
* [Issue #829](https://github.com/manticoresoftware/manticoresearch/issues/829) “日期格式化方法”：新增了[date_format()](Functions/Date_and_time_functions.md#DATE_FORMAT())选择列表表达式，暴露了 `strftime()` 函数。
* [Issue #961](https://github.com/manticoresoftware/manticoresearch/issues/961) “通过 HTTP JSON API 对桶排序”：在 HTTP 接口的每个聚合桶中引入了可选的[排序属性](Searching/Faceted_search.md#HTTP-JSON)。
* [Issue #1062](https://github.com/manticoresoftware/manticoresearch/issues/1062) “改进 JSON 插入接口失败时的错误日志 - ‘不支持的值类型’”：`/bulk` 端点在出错情况下报告已处理和未处理字符串（文档）的数量信息。
* [Issue #1070](https://github.com/manticoresoftware/manticoresearch/issues/1070) “CBO 提示不支持多个属性”：使索引提示支持多个属性。
* [Issue #1106](https://github.com/manticoresoftware/manticoresearch/issues/1106) “为 http 搜索查询添加标签”：已向[HTTP PQ 响应](Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md?client=JSON#Adding-rules-to-a-percolate-table)中添加了标签。
* [Issue #1301](https://github.com/manticoresoftware/manticoresearch/issues/1301) “buddy 不应并行创建表”：修复了并行执行 CREATE TABLE 导致的失败问题。现在，`CREATE TABLE` 操作一次只能执行一个。
* [Issue #1303](https://github.com/manticoresoftware/manticoresearch/issues/1303) “为列名添加对 @ 的支持”。
* [Issue #1316](https://github.com/manticoresoftware/manticoresearch/issues/1316) “使用 ps=1 时出租车数据集查询缓慢”：细化了 CBO 逻辑，默认直方图分辨率设置为 8k，以更准确地处理随机分布值的属性。
* [Issue #1317](https://github.com/manticoresoftware/manticoresearch/issues/1317) “修正 hn 数据集中 CBO 与全文本的关系”：改进了决定何时使用位图迭代器交集以及何时使用优先队列的逻辑。
* [Issue #1318](https://github.com/manticoresoftware/manticoresearch/issues/1318) “columnar: 将迭代器接口改为单次调用”：Columnar 迭代器现在使用单次 `Get` 调用，取代了之前的两步 `AdvanceTo` + `Get` 调用方式来获取值。
* [Issue #1319](https://github.com/manticoresoftware/manticoresearch/issues/1319) “聚合计算加速（移除 CheckReplaceEntry？）”：从组排序器中移除了 `CheckReplaceEntry` 调用，加快了聚合函数的计算速度。
* [Issue #1320](https://github.com/manticoresoftware/manticoresearch/issues/1320) “创建表时 read_buffer_docs/hits 不支持 k/m/g 语法”：`CREATE TABLE` 选项 `read_buffer_docs` 和 `read_buffer_hits` 现支持 k/m/g 语法。
* 英语、德语和俄语的[语言包](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology)现在可在 Linux 上通过执行命令 `apt/yum install manticore-language-packs` 轻松安装。在 macOS 上可使用命令 `brew install manticoresoftware/tap/manticore-language-packs`。
* 字段和属性的顺序现在在 `SHOW CREATE TABLE` 和 `DESC` 操作之间保持一致。
* 如果执行 `INSERT` 查询时磁盘空间不足，新的 `INSERT` 查询将失败，直到有足够的磁盘空间可用。
* 新增了[UINT64()](Functions/Type_casting_functions.md#UINT64%28%29)类型转换函数。
* `/bulk` 端点现在将空行视为[提交](Data_creation_and_modification/Transactions.md#BEGIN,-COMMIT,-and-ROLLBACK)命令。详情见[这里](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md?client=JSON#Bulk-adding-documents)。
* 实现了[无效索引提示](Searching/Options.md#Query-optimizer-hints)的警告，提升透明度并允许错误缓解。
* 当 `count(*)` 与单个过滤器一起使用时，查询现在在可用时利用二级索引的预计算数据，大幅提升查询速度。

### ⚠️ 破坏性变更
* ⚠️ 版本 6.2.0 创建或修改的表不能被旧版本读取。
* ⚠️ 文档 ID 现在在索引和 INSERT 操作中作为无符号 64 位整数处理。
* ⚠️ 查询优化器提示的语法已更新，新格式为 `/*+ SecondaryIndex(uid) */`。旧语法不再支持。
* ⚠️ [Issue #1160](https://github.com/manticoresoftware/manticoresearch/issues/1160)：为防止语法冲突，表名中禁止使用 `@`。
* ⚠️ 被标记为 `indexed` 和 `attribute` 的字符串字段/属性现在在 `INSERT`、`DESC` 和 `ALTER` 操作中被视为单个字段。
* ⚠️ [Issue #1057](https://github.com/manticoresoftware/manticoresearch/issues/1057)：不支持 SSE 4.2 的系统将不再加载 MCL 库。
* ⚠️ [Issue #1143](https://github.com/manticoresoftware/manticoresearch/issues/1143)：修复了[agent_query_timeout](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_query_timeout)的问题，现在有效。

### Bugfixes
* [提交 2a6e](https://github.com/manticoresoftware/manticoresearch/commit/2a6ea8f7) “DROP TABLE 崩溃”：解决了在执行 DROP TABLE 语句时导致 RT 表写操作（优化、磁盘块保存）完成等待时间延长的问题。增加了警告，在执行 DROP TABLE 命令后通知表目录不为空。
* [提交 2ebd](https://github.com/manticoresoftware/manticoresearch/commit/2ebd424d)：添加了对缺失于多属性分组代码中的列式属性的支持。
* [提交 3be4](https://github.com/manticoresoftware/manticoresearch/commit/3be4503f752216b37b8cde8e47fd2d77bbce63ae) 解决了潜在因磁盘空间耗尽导致的崩溃问题，通过正确处理 binlog 中的写入错误。
* [提交 6adb](https://github.com/manticoresoftware/manticoresearch/commit/6adb0934)：修复了使用多个列式扫描迭代器（或二级索引迭代器）查询时偶发的崩溃问题。
* [提交 6bd9](https://github.com/manticoresoftware/manticoresearch/commit/6bd9f709)：修复了使用预计算数据的排序器时过滤器未被移除的问题。
* [提交 6d03](https://github.com/manticoresoftware/manticoresearch/commit/6d03566a)：更新了 CBO 代码，以便更好地估算多线程中执行的基于行属性过滤的查询。
* [提交 6dd3](https://github.com/manticoresoftware/manticoresearch/commit/6dd3964), [Helm #56](https://github.com/manticoresoftware/manticoresearch-helm/issues/56) “Kubernetes 集群中致命崩溃转储”：修复了 JSON 根对象的错误 Bloom 过滤器；修复了因基于 JSON 字段过滤导致的守护进程崩溃。
* [提交 6e1b](https://github.com/manticoresoftware/manticoresearch/commit/6e1b849) 修正了因无效的 `manticore.json` 配置引起的守护进程崩溃。
* [提交 6fbc](https://github.com/manticoresoftware/manticoresearch/commit/6fbc4189) 修复了 json 范围过滤器以支持 int64 值。
* [提交 9c67](https://github.com/manticoresoftware/manticoresearch/commit/9c677426) `.sph` 文件可能因 `ALTER` 操作被破坏。已修复。
* [提交 77cc](https://github.com/manticoresoftware/manticoresearch/commit/77ccd8f)：增加了 replace 语句复制的共享密钥，以解决从多个主节点复制 replace 时出现的 `pre_commit` 错误。
* [提交 2884](https://github.com/manticoresoftware/manticoresearch/commit/2884519e) 解决了针对类似 'date_format()' 函数的大整数检查问题。
* [提交 9513](https://github.com/manticoresoftware/manticoresearch/commit/9513aca5)：在使用预计算数据排序器时，迭代器不再显示于 [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META) 中。
* [提交 a2a7](https://github.com/manticoresoftware/manticoresearch/commit/a2a70555)：更新全文节点栈大小，防止复杂全文查询时崩溃。
* [提交 a062](https://github.com/manticoresoftware/manticoresearch/commit/a0626d7e)：修复了带有 JSON 和字符串属性的更新复制时引起的崩溃BUG。
* [提交 b3e6](https://github.com/manticoresoftware/manticoresearch/commit/b3e63e5d)：更新字符串构建器使用 64 位整数，避免处理大数据集时崩溃。
* [提交 c472](https://github.com/manticoresoftware/manticoresearch/commit/c472e5b)：修复了跨多个索引执行 count distinct 时的崩溃问题。
* [提交 d073](https://github.com/manticoresoftware/manticoresearch/commit/d0730272)：修复了即使禁用 `pseudo_sharding`，对 RT 索引磁盘块的查询仍可多线程执行的问题。
* [提交 d205](https://github.com/manticoresoftware/manticoresearch/commit/d205508e) 修改了 `show index status` 命令返回的值集，现在根据使用的索引类型有所不同。
* [提交 e9bc](https://github.com/manticoresoftware/manticoresearch/commit/e9bc2f9) 修复了处理批量请求时的 HTTP 错误及错误未返回给客户端的问题。
* [提交 f77c](https://github.com/manticoresoftware/manticoresearch/commit/f77ce0e6) 为 PQ 使用了扩展堆栈。
* [提交 fac2](https://github.com/manticoresoftware/manticoresearch/commit/fac2d9687a20582765aaa89be9597804c4f845af) 更新导出排序器输出，使之与 [packedfactors()](Functions/Searching_and_ranking_functions.md#PACKEDFACTORS%28%29) 保持一致。
* [提交 ff87](https://github.com/manticoresoftware/manticoresearch/commit/ff87cc4)：修复了 SphinxQL 查询日志过滤器中的字符串列表问题。
* [问题 #589](https://github.com/manticoresoftware/manticoresearch/issues/589) “字符集定义似乎依赖代码排序顺序”：修复了重复字符集映射错误。
* [问题 #811](https://github.com/manticoresoftware/manticoresearch/issues/811) “在词形变化映射多词组时，CJK 标点符号间关键字干扰短语搜索”：修复了包含词形变化的短语查询中 ngram 令牌位置问题。
* [问题 #834](https://github.com/manticoresoftware/manticoresearch/issues/834) “搜索查询中的等号破坏请求”：确保精确符号可被转义，同时修复了 `expand_keywords` 选项导致的双重精确扩展。
* [问题 #864](https://github.com/manticoresoftware/manticoresearch/issues/864) “exceptions/stopwords 冲突”
* [问题 #910](https://github.com/manticoresoftware/manticoresearch/issues/910) “调用 call snippets() 时使用 libstemmer_fr 和 index_exact_words 导致 Manticore 崩溃”：解决了调用 `SNIPPETS()` 时内部冲突导致的崩溃。
* [问题 #946](https://github.com/manticoresoftware/manticoresearch/issues/946) “SELECT 查询中记录重复”：修正了带有 `not_terms_only_allowed` 选项且包含已删除文档的 RT 索引中查询结果文档重复的问题。
* [问题 #967](https://github.com/manticoresoftware/manticoresearch/issues/967) “在 UDF 函数中使用 JSON 参数导致崩溃”：修复了在启用伪分片且 UDF 带有 JSON 参数时处理搜索导致的守护进程崩溃。
* [问题 #1050](https://github.com/manticoresoftware/manticoresearch/issues/1050) “FEDERATED 中的 count(*)”：修复了通过 `FEDERATED` 引擎执行带聚合查询时出现的守护进程崩溃。
* [问题 #1052](https://github.com/manticoresoftware/manticoresearch/issues/1052) 修复了 `rt_attr_json` 列与列存储不兼容的问题。
* [问题 #1072](https://github.com/manticoresoftware/manticoresearch/issues/1072) “* 被 ignore_chars 从搜索查询中移除”：修复了这一问题，使查询中的通配符不受 `ignore_chars` 的影响。
* [问题 #1075](https://github.com/manticoresoftware/manticoresearch/issues/1075) “indextool --check 在存在分布式表时失败”：indextool 现在兼容在 json 配置中含有“distributed”和“template”索引的实例。
* [问题 #1081](https://github.com/manticoresoftware/manticoresearch/issues/1081) “特定 RT 数据集上的特定选择导致 searchd 崩溃”：解决了使用 packedfactors 和大内部缓冲区的查询导致的守护进程崩溃。
* [问题 #1095](https://github.com/manticoresoftware/manticoresearch/issues/1095) “使用 not_terms_only_allowed 时已删除文档被忽略”
* [问题 #1099](https://github.com/manticoresoftware/manticoresearch/issues/1099) “indextool --dumpdocids 无法工作”：恢复了 `--dumpdocids` 命令的功能。
* [问题 #1100](https://github.com/manticoresoftware/manticoresearch/issues/1100) “indextool --buildidf 无法工作”：indextool 现在在完成 globalidf 后关闭文件。
* [问题 #1104](https://github.com/manticoresoftware/manticoresearch/issues/1104) “Count(*) 试图被当作远程表中的 schema set”：解决了当代理返回空结果集时，守护进程对分布式索引的查询发送错误消息的问题。
* [问题 #1109](https://github.com/manticoresoftware/manticoresearch/issues/1109) “使用 threads=1 时 FLUSH ATTRIBUTES 挂起”。
* [问题 #1126](https://github.com/manticoresoftware/manticoresearch/issues/1126) “查询期间丢失与 MySQL 服务器的连接 - manticore 6.0.5”：修复了使用多重筛选条件对列属性查询时发生的崩溃。
* [问题 #1135](https://github.com/manticoresoftware/manticoresearch/issues/1135) “JSON 字符串过滤的大小写敏感性”：修正了用于 HTTP 搜索请求筛选器的校对，确保功能正确。
* [问题 #1140](https://github.com/manticoresoftware/manticoresearch/issues/1140) “错误字段中的匹配”：修复了与 `morphology_skip_fields` 相关的问题。
* [问题 #1155](https://github.com/manticoresoftware/manticoresearch/issues/1155) “通过 API 的系统远程命令应传递 g_iMaxPacketSize”：更新了以绕过节点间复制命令的 `max_packet_size` 检查。此外，最新的集群错误已添加到状态显示中。
* [问题 #1302](https://github.com/manticoresoftware/manticoresearch/issues/1302) “失败优化时遗留的临时文件”：修正了在合并或优化过程中发生错误后留下临时文件的问题。
* [问题 #1304](https://github.com/manticoresoftware/manticoresearch/issues/1304) “为 buddy 启动超时添加环境变量”：增加了环境变量 `MANTICORE_BUDDY_TIMEOUT`（默认3秒），用于控制守护进程在启动时等待 buddy 消息的时间。
* [问题 #1305](https://github.com/manticoresoftware/manticoresearch/issues/1305) “保存 PQ 元数据时的整数溢出”：缓解了保存大型 PQ 索引时守护进程的过度内存消耗。
* [问题 #1306](https://github.com/manticoresoftware/manticoresearch/issues/1306) “无法在更改其外部文件后重新创建 RT 表”：修正了对外部文件使用空字符串的 alter 错误；修复了在修改外部文件后残留的 RT 索引外部文件。
* [问题 #1307](https://github.com/manticoresoftware/manticoresearch/issues/1307) “SELECT 语句 sum(value) as value 不正常工作”：修复了带别名的选择列表表达式可能屏蔽索引属性的问题；还修正了整数的 sum 到 int64 的计数。
* [问题 #1308](https://github.com/manticoresoftware/manticoresearch/issues/1308) “避免在复制中绑定到 localhost”：确保复制不会对具有多个 IP 地址的主机名绑定到 localhost。
* [问题 #1309](https://github.com/manticoresoftware/manticoresearch/issues/1309) “对 MySQL 客户端回复超过16Mb数据失败”：修复了向客户端返回超过16Mb的 SphinxQL 数据包的问题。
* [问题 #1310](https://github.com/manticoresoftware/manticoresearch/issues/1310) “‘外部文件路径应为绝对路径’中的错误引用”：修正了 `SHOW CREATE TABLE` 中外部文件完整路径的显示。
* [问题 #1311](https://github.com/manticoresoftware/manticoresearch/issues/1311) “调试构建在 snippet 中遇到长字符串时崩溃”：现在，允许 `SNIPPET()` 函数的目标文本中出现长字符串（>255 字符）。
* [问题 #1312](https://github.com/manticoresoftware/manticoresearch/issues/1312) “kqueue 轮询中的使用后释放（use-after-delete）错误导致的虚假崩溃（master-agent）”：修复了在 kqueue 驱动系统（FreeBSD、MacOS 等）上 master 无法连接到 agent 时的崩溃。
* [问题 #1313](https://github.com/manticoresoftware/manticoresearch/issues/1313) “连接自身时间过长”：在 MacOS/BSD 上从 master 连接到 agent 时，现在采用统一的 connect+query 超时，而不只是连接超时。
* [问题 #1314](https://github.com/manticoresoftware/manticoresearch/issues/1314) “pq（json 元数据）中未达到的嵌入同义词导致加载失败”：修复了 pq 中的嵌入同义词标志。
* [问题 #1315](https://github.com/manticoresoftware/manticoresearch/issues/1315) “允许某些函数（sint、fibonacci、second、minute、hour、day、month、year、yearmonth、yearmonthday）隐式提升参数值”。
* [问题 #1321](https://github.com/manticoresoftware/manticoresearch/issues/1321) “启用 fullscan 中的多线程 SI，但限制线程数”：代码已集成到 CBO 中，更好地预测在全文查询中使用二级索引时的多线程性能。
* [问题 #1322](https://github.com/manticoresoftware/manticoresearch/issues/1322) “使用预计算排序器后 count(*) 查询依然缓慢”：在使用预计算数据的排序器时，不再初始化迭代器，避免了性能下降。
* [问题 #1411](https://github.com/manticoresoftware/manticoresearch/issues/1141) “sphinxql 的查询日志不保留 MVA 的原始查询”：现在，`all()/any()` 被记录。

## 版本 6.0.4
发布时间：2023年3月15日

### 新特性
* 改进了与 Logstash、Beats 等的集成，包括：
  - 支持 Logstash 版本 7.6 - 7.15，Filebeat 版本 7.7 - 7.12
  - 自动模式支持。
  - 增加了对类似 Elasticsearch 格式的批量请求的处理。
* [Buddy 提交 ce90](https://github.com/manticoresoftware/manticoresearch-buddy/commit/ce907ea) Manticore 启动时记录 Buddy 版本。

### Bug修复
* [问题 #588](https://github.com/manticoresoftware/manticoresearch/issues/588), [问题 #942](https://github.com/manticoresoftware/manticoresearch/issues/942) 修复了 bigram 索引中搜索元数据和调用关键字的非法字符问题。
* [问题 #1027](https://github.com/manticoresoftware/manticoresearch/issues/1027) 拒绝小写 HTTP 头。
* ❗[问题 #1039](https://github.com/manticoresoftware/manticoresearch/issues/1039) 修复了 Buddy 控制台输出读取时守护进程的内存泄漏。
* [问题 #1056](https://github.com/manticoresoftware/manticoresearch/issues/1056) 修复了问号的意外行为。
* [问题 #1064](https://github.com/manticoresoftware/manticoresearch/issues/1064) - 修复了 tokenizer 小写表的竞态条件导致崩溃的问题。
* [提交 59bb](https://github.com/manticoresoftware/manticoresearch/commit/59bb54c) 修复了 JSON 接口中显式设置 id 为 null 的文档的批量写入处理。
* [提交 7b6b](https://github.com/manticoresoftware/manticoresearch/commit/7b6b25f) 修复了 CALL KEYWORDS 中多个相同词项的统计错误。
* [提交 f381](https://github.com/manticoresoftware/manticoresearch/commit/f381ad2) Windows 安装程序现在创建默认配置；运行时不再替换路径。
* [提交 6940](https://github.com/manticoresoftware/manticoresearch/commit/6940e95), [提交 cc5a](https://github.com/manticoresoftware/manticoresearch/commit/cc5a480) 修复了多网络节点集群的复制问题。
* [提交 4972](https://github.com/manticoresoftware/manticoresearch/commit/49722ab) 修复了 `/pq` HTTP 端点作为 `/json/pq` HTTP 端点的别名。
* [提交 3b53](https://github.com/manticoresoftware/manticoresearch/commit/3b5385a) 修复了 Buddy 重启时守护进程崩溃的问题。
* [Buddy 提交 fba9](https://github.com/manticoresoftware/manticoresearch-buddy/commit/fba9c8c) 对接收到的无效请求显示原始错误。
* [Buddy 提交 db95](https://github.com/manticoresoftware/manticoresearch-buddy/commit/db9532c) 允许备份路径中包含空格，并对正则表达式进行了优化以支持单引号。

## 版本 6.0.2
发布时间：2023年2月10日

### Bug修复
* [问题 #1024 crash 2](https://github.com/manticoresoftware/manticoresearch/issues/1024) Facet 搜索在大量结果时崩溃/段错误
* ❗[问题 #1029](https://github.com/manticoresoftware/manticoresearch/issues/1029) - 警告：编译时内置值 KNOWN_CREATE_SIZE（16）小于实际测量值（208）。建议修正该值！
* ❗[问题 #1032](https://github.com/manticoresoftware/manticoresearch/issues/1032) - Manticore 6.0.0 plain 索引崩溃
* ❗[问题 #1033](https://github.com/manticoresoftware/manticoresearch/issues/1033) - 守护进程重启后多个分布式索引丢失
* ❗[问题 #1064](https://github.com/manticoresoftware/manticoresearch/issues/1064) - tokenizer 小写表的竞态条件

## 版本 6.0.0
发布时间：2023年2月7日

从本版本开始，Manticore Search 附带了 Manticore Buddy，这是一个用 PHP 编写的辅助守护进程，处理不需要极低延迟或高吞吐量的高级功能。Manticore Buddy 在后台运行，用户可能甚至不会察觉它的存在。尽管对终端用户不可见，但使 Manticore Buddy 易于安装且与基于 C++ 的主守护进程兼容是一个重大挑战。此重大变更将允许团队开发各种新的高级功能，如分片编排、访问控制和身份验证，以及 mysqldump、DBeaver、Grafana mysql 连接器等多种集成。目前它已经支持 [SHOW QUERIES](Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES)、[BACKUP](Securing_and_compacting_a_table/Backup_and_restore.md#BACKUP-SQL-command-reference) 和 [自动模式](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema)。

本版本还包含 130 多个 bug 修复和众多功能更新，其中很多可视为重大更新。

### 重大变更
* 🔬 实验性功能：现在可以执行兼容 Elasticsearch 的 [insert](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md?example=Elasticsearch#Adding-documents-to-a-real-time-table) 和 [replace](Data_creation_and_modification/Updating_documents/REPLACE.md?example=Elasticsearch#REPLACE) JSON 查询，这使得 Manticore 可以与 Logstash（版本 < 7.13）、Filebeat 及 Beats 系列的其他工具配合使用。默认启用。您可以使用 `SET GLOBAL ES_COMPAT=off` 来禁用该功能。
* 支持 [Manticore Columnar Library 2.0.0](https://github.com/manticoresoftware/columnar/) ，对 [二级索引](Server_settings/Searchd.md#secondary_indexes) 进行了众多修复和改进。**⚠️ 重大变更**：从本版本开始，二级索引默认启用。如果您从 Manticore 5 升级，请务必执行 [ALTER TABLE table_name REBUILD SECONDARY](Updating_table_schema_and_settings.md#Rebuilding-a-secondary-index)。详情见下文。
* [提交 c436](https://github.com/manticoresoftware/manticoresearch/commit/c436f9023536f767610451911955ae36d90aa638) 自动模式：现在您可以跳过建表，直接插入首个文档，Manticore 会根据文档字段自动创建表。详细说明见 [这里](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema)。您可以使用 [searchd.auto_schema](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema) 来开启或关闭此功能。
* 对 [基于成本的优化器](Searching/Cost_based_optimizer.md) 进行了彻底改造，在许多情况下降低了查询响应时间。
  - [Issue #1008](https://github.com/manticoresoftware/manticoresearch/issues/1008) CBO 中的并行性能估计。
  - [Issue #1014](https://github.com/manticoresoftware/manticoresearch/issues/1014) CBO 现在识别[二级索引](Server_settings/Searchd.md#secondary_indexes)，能够更加智能地执行。
  - [提交 cef9](https://github.com/manticoresoftware/columnar/commit/cef9b54dc3a4ee9138ebc4df3a671715b996d6b3) 现在将列式表/字段的编码统计存储于元数据中，以辅助 CBO 做出更智能的决策。
  - [提交 2b95](https://github.com/manticoresoftware/manticoresearch/commit/2b95fb8c2ba422c9c5a66e2b61c1f7922f31c7aa) 增加了用于微调行为的 CBO 提示。
* [遥测](Telemetry.md#Telemetry)：我们很高兴地宣布本版本新增了遥测功能。该功能允许我们收集匿名且去标识化的指标，以帮助提升产品性能和用户体验。请放心，所有收集的数据**完全匿名，不会关联任何个人信息**。如有需要，您可以在设置中[轻松关闭此功能](Telemetry.md#Telemetry)。
* [提交 5aaf](https://github.com/manticoresoftware/manticoresearch/commit/5aafb17d81b601e399e212a344a4738f65576cdd) 新增 [ALTER TABLE table_name REBUILD SECONDARY](Updating_table_schema_and_settings.md#Rebuilding-a-secondary-index) 命令，允许您随时重建二级索引，例如：
  - 从 Manticore 5 迁移到新版本时，
  - 当您对索引中的属性进行了[UPDATE](Data_creation_and_modification/Updating_documents/UPDATE.md#UPDATE)（即[原地更新，不是替换](Data_creation_and_modification/Updating_documents/REPLACE_vs_UPDATE.md)）时
* [Issue #821](https://github.com/manticoresoftware/manticoresearch/issues/821) 新增工具 `manticore-backup` 用于[备份和恢复 Manticore 实例](Securing_and_compacting_a_table/Backup_and_restore.md)
* SQL 命令 [BACKUP](Securing_and_compacting_a_table/Backup_and_restore.md#BACKUP-SQL-command-reference) ，可在 Manticore 内部执行备份。
* SQL 命令 [SHOW QUERIES](Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES) ，提供查看运行中查询（非线程）的一种便捷方式。
* [Issue #551](https://github.com/manticoresoftware/manticoresearch/issues/551) 新增 SQL 命令 `KILL` ，用于终止长时间运行的 `SELECT`。
* 动态调整聚合查询的 `max_matches`，以提升准确度并降低响应时间。

### 小改动
* [Issue #822](https://github.com/manticoresoftware/manticoresearch/issues/822) 新增 SQL 命令 [FREEZE/UNFREEZE](Securing_and_compacting_a_table/Freezing_a_table.md) ，用于准备实时/普通表的备份。
* [提交 c470](https://github.com/manticoresoftware/manticoresearch/commit/c47052f18ad4868134de1a97e8d7da570e531dfa) 新增设置 `accurate_aggregation` 与 `max_matches_increase_threshold`，用于控制聚合准确度。
* [Issue #718](https://github.com/manticoresoftware/manticoresearch/issues/718) 支持带符号的负 64 位 ID。注意，仍不能使用 ID 超过 2^63，但现在可以使用范围从 -2^63 到 0 的 ID。
* 由于我们最近加入了二级索引支持，导致“索引”一词可能指代二级索引、全文索引或普通/实时 `index`，造成混淆。为减少困惑，我们将后者重命名为“表（table）”。以下 SQL/命令行命令受此影响。它们的旧版本已废弃但依然可用：
  - `index <table name>` => `table <table name>`，
  - `searchd -i / --index` => `searchd -t / --table`，
  - `SHOW INDEX STATUS` => `SHOW TABLE STATUS`，
  - `SHOW INDEX SETTINGS` => `SHOW TABLE SETTINGS`，
  - `FLUSH RTINDEX` => `FLUSH TABLE`，
  - `OPTIMIZE INDEX` => `OPTIMIZE TABLE`，
  - `ATTACH TABLE plain TO RTINDEX rt` => `ATTACH TABLE plain TO TABLE rt`，
  - `RELOAD INDEX` => `RELOAD TABLE`，
  - `RELOAD INDEXES` => `RELOAD TABLES`。

  我们不打算废除旧版本命令，但为确保与文档兼容，建议您在应用中改用新名称。未来版本将会在多种 SQL 与 JSON 命令输出中，将“index”重命名为“table”。
* 含状态的 UDF（用户自定义函数）查询现强制单线程执行。
* [Issue #1011](https://github.com/manticoresoftware/manticoresearch/issues/1011) 重构了与时间调度相关的所有内容，为并行分块合并奠定基础。
* **⚠️ 重大变更**：列式存储格式已更改。您需要重建包含列式属性的表。
* **⚠️ 重大变更**：二级索引的文件格式已更改，因此如果您正在使用二级索引进行搜索，并且在配置文件中设置了 `searchd.secondary_indexes = 1`，请注意新版本的 Manticore **将跳过加载具有二级索引的表**。建议：
  - 在升级之前，将配置文件中的 `searchd.secondary_indexes` 修改为 0。
  - 运行实例。Manticore 会加载表并发出警告。
  - 对每个索引运行 `ALTER TABLE <table name> REBUILD SECONDARY` 以重建二级索引。

  如果您运行的是复制集群，则需要在所有节点上运行 `ALTER TABLE <table name> REBUILD SECONDARY`，或者按照[此说明](Securing_and_compacting_a_table/Compacting_a_table.md#Optimizing-clustered-tables)操作，只需将 `OPTIMIZE` 替换为运行 `ALTER .. REBUILD SECONDARY`。
* **⚠️ 重大变更**：binlog 版本已更新，之前版本的任何 binlog 都不会被重放。升级过程中，确保 Manticore Search 被正常停止非常重要。这意味着在停止之前的实例后，`/var/lib/manticore/binlog/` 目录中除 `binlog.meta` 之外不应存在 binlog 文件。
* [Issue #849](https://github.com/manticoresoftware/manticoresearch/issues/849) `SHOW SETTINGS`：现在可以在 Manticore 内部查看配置文件中的设置。
* [Issue #1007](https://github.com/manticoresoftware/manticoresearch/issues/1007) [SET GLOBAL CPUSTATS=1/0](Server_settings/Setting_variables_online.md#SET) 用于开启/关闭 CPU 时间追踪；[SHOW THREADS](Node_info_and_management/SHOW_THREADS.md) 在关闭 CPU 时间追踪时不再显示 CPU 统计信息。
* [Issue #1009](https://github.com/manticoresoftware/manticoresearch/issues/1009) RT 表的 RAM chunk 分段现在可以在正在刷新 RAM chunk 时合并。
* [Issue #1012](https://github.com/manticoresoftware/manticoresearch/issues/1012) 在 [indexer](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) 输出中添加了二级索引进度信息。
* [Issue #1013](https://github.com/manticoresoftware/manticoresearch/issues/1013) 之前如果 Manticore 无法启动提供服务，表记录可能会从索引列表中被移除。新的行为是将其保留在列表中，以便尝试在下次启动时加载。
* [indextool --docextract](Miscellaneous_tools.md#indextool) 返回属于请求文档的所有词汇和命中记录。
* [Commit 2b29](https://github.com/manticoresoftware/manticoresearch/commit/2b296ee20e520b85bcbb4383e87095d31e7165dc) 环境变量 `dump_corrupt_meta` 可在 searchd 无法加载索引时将损坏的表元数据转储到日志中。
* [Commit c7a3](https://github.com/manticoresoftware/manticoresearch/commit/c7a3136eefe0eb2f69e3d99fae9c10beb5821ea6) `DEBUG META` 现在可以显示 `max_matches` 和伪分片统计信息。
* [Commit 6bca](https://github.com/manticoresoftware/manticoresearch/commit/6bca4848dc5c8396dbc1e057be4b9b2d579794e8) 提供了比之前让人困惑的 “Index header format is not json, will try it as binary...” 更好的错误提示。
* [Commit bef3](https://github.com/manticoresoftware/lemmatizer-uk/commit/bef3ff0386d3ee87ec57619782100972c1122e47) 乌克兰语词形还原器路径已更改。
* [Commit 4ae7](https://github.com/manticoresoftware/manticoresearch/commit/4ae789595329a2951e194d1191ddb3121459a560) 在 [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META) 中添加了二级索引统计信息。
* [Commit 2e7c](https://github.com/manticoresoftware/manticoresearch/commit/2e7c585e) JSON 接口现在可以使用 Swagger 编辑器轻松可视化 https://manual.manticoresearch.com/Openapi#OpenAPI-specification。
* **⚠️ 重大变更**：复制协议已更改。如果您运行复制集群，升级到 Manticore 5 时，需要：
  - 首先干净地停止所有节点
  - 然后用 `--new-cluster` 启动最后停止的节点（Linux 下运行工具 `manticore_new_cluster`）。
  - 详细信息请参阅[重新启动集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster)。

### 与 Manticore Columnar 库相关的变更
* 对二级索引与 Columnar 存储的集成进行了重构。
* [Commit efe2](https://github.com/manticoresoftware/columnar/commit/efe26b2a) Manticore Columnar 库优化，通过部分预先 min/max 评估降低响应时间。
* [Commit 2757](https://github.com/manticoresoftware/columnar/commit/2757b99b) 如果磁盘 chunk 合并被中断，守护进程将清理与 MCL 相关的临时文件。
* [Commit e9c6](https://github.com/manticoresoftware/manticoresearch/commit/e9c672014b63e2b49e7a5a53880583af86c3ab4e) 崩溃时将 Columnar 和二级库的版本信息记录到日志。
* [Commit f5e8](https://github.com/manticoresoftware/columnar/commit/f5e84eeb8b6cb102f13e3468246702427527fef9) 为二级索引添加了快速 doclist 回绕支持。
* [Commit 06df](https://github.com/manticoresoftware/manticoresearch/commit/06dfdd2e668933495dd6cb52d5ba2e0aa9b26e58) 对于使用 MCL 的情况，类似 `select attr, count(*) from plain_index`（无过滤）的查询性能更快。
* [Commit 0a76](https://github.com/manticoresoftware/manticoresearch/commit/0a76ecb9adbe886868b962dd425de5f475bbf55e) 为了兼容 MySQL 8.25 以上的 .net 连接器，`HandleMysqlSelectSysvar` 中添加了 @@autocommit 支持。
* **⚠️ 重大变更**： [MCL Issue #17](https://github.com/manticoresoftware/columnar/issues/17) MCL：为列扫描添加 SSE 代码。MCL 现在至少需要 SSE4.2 支持。

### 与打包相关的变更
* [Commit 4d19](https://github.com/manticoresoftware/manticoresearch/commit/4d19f5cbe49a31228aa09253f061165bfe80e51b) **⚠️ 重大变更**：不再支持 Debian Stretch 和 Ubuntu Xenial。
* 支持 RHEL 9，包括 CentOS 9、Alma Linux 9 和 Oracle Linux 9。
* [Issue #924](https://github.com/manticoresoftware/manticoresearch/issues/924) 支持 Debian Bookworm。
* [Issue #636](https://github.com/manticoresoftware/manticoresearch/issues/636) 打包：针对 Linux 和 MacOS 的 arm64 构建。
* [PR #26](https://github.com/manticoresoftware/docker/pull/26) 多架构 (x86_64 / arm64) 的 docker 镜像。
* [简化贡献者的软件包构建](Installation/Compiling_from_sources.md#Building-using-CI-Docker)。
* 现在可以使用 APT 安装特定版本。
* [Commit a6b8](https://github.com/manticoresoftware/manticoresearch/commit/51fddca5c2a3ebb8576fae4c18660656ba22de0f) Windows 安装程序（之前我们只提供了一个归档文件）。
* 切换至使用 CLang 15 编译。
* **⚠️ 重大更改**：自定义的 Homebrew 配方，包括 Manticore Columnar Library 的配方。要安装 Manticore、MCL 及其它必要组件，请使用以下命令 `brew install manticoresoftware/manticore/manticoresearch manticoresoftware/manticore/manticore-extra`。

### 修复的错误
* [Issue #479](https://github.com/manticoresoftware/manticoresearch/issues/479) 名为 `text` 的字段
* [Issue #501](https://github.com/manticoresoftware/manticoresearch/issues/501) id 不能是非 bigint 类型
* [Issue #646](https://github.com/manticoresoftware/manticoresearch/issues/646) ALTER 与名为 "text" 的字段冲突
* ❗[Issue #652](https://github.com/manticoresoftware/manticoresearch/issues/652) 可能的 BUG：HTTP (JSON) 的 offset 和 limit 影响 facet 结果
* ❗[Issue #827](https://github.com/manticoresoftware/manticoresearch/issues/827) searchd 在高负载下挂起/崩溃
* ❗[Issue #996](https://github.com/manticoresoftware/manticoresearch/issues/996) PQ 索引内存溢出
* ❗[Commit 1041](https://github.com/manticoresoftware/manticoresearch/commit/10416ef7dddf06c0d759e32ccd6ebaa2468f7cbf) `binlog_flush = 1` 自 Sphinx 以来一直存在问题。已修复。
* [MCL Issue #14](https://github.com/manticoresoftware/columnar/issues/14) MCL：当 FT 字段过多时选择会崩溃
* [Issue #470](https://github.com/manticoresoftware/manticoresearch/issues/470) sql_joined_field 不能存储
* [Issue #713](https://github.com/manticoresoftware/manticoresearch/issues/713) 使用 LEVENSHTEIN() 时崩溃
* [Issue #743](https://github.com/manticoresoftware/manticoresearch/issues/743) Manticore 意外崩溃且无法正常重启
* [Issue #788](https://github.com/manticoresoftware/manticoresearch/issues/788) 通过 /sql 调用 KEYWORDS 返回控制字符，破坏 JSON
* [Issue #789](https://github.com/manticoresoftware/manticoresearch/issues/789) mariadb 无法创建 FEDERATED 表
* [Issue #796](https://github.com/manticoresoftware/manticoresearch/issues/796) 警告：dlopen() 调用失败：/usr/bin/lib_manticore_columnar.so：无法打开共享对象文件：没有此类文件或目录
* [Issue #797](https://github.com/manticoresoftware/manticoresearch/issues/797) 通过 API 使用 ZONESPAN 搜索时 Manticore 崩溃
* [Issue #799](https://github.com/manticoresoftware/manticoresearch/issues/799) 使用多个索引和 facet distinct 时权重错误
* [Issue #801](https://github.com/manticoresoftware/manticoresearch/issues/801) SphinxQL 分组查询在 SQL 索引重新处理后挂起
* [Issue #802](https://github.com/manticoresoftware/manticoresearch/issues/802) MCL：Indexer 在 5.0.2 和 manticore-columnar-lib 1.15.4 中崩溃
* [Issue #813](https://github.com/manticoresoftware/manticoresearch/issues/813) Manticore 5.0.2 FEDERATED 返回空集 (MySQL 8.0.28)
* [Issue #824](https://github.com/manticoresoftware/manticoresearch/issues/824) 当结果为零时，对两个索引执行 select COUNT DISTINCT 抛出内部错误
* [Issue #826](https://github.com/manticoresoftware/manticoresearch/issues/826) 删除查询时崩溃
* [Issue #843](https://github.com/manticoresoftware/manticoresearch/issues/843) MCL：长文本字段的 Bug
* [Issue #856](https://github.com/manticoresoftware/manticoresearch/issues/856) 5.0.2 rtindex：聚合搜索限制行为不符合预期
* [Issue #863](https://github.com/manticoresoftware/manticoresearch/issues/863) 即使搜索应返回多个结果，返回的 Hits 也是 None 类型对象
* [Issue #870](https://github.com/manticoresoftware/manticoresearch/issues/870) 在 SELECT 表达式中使用 Attribute 和 Stored Field 时崩溃
* [Issue #872](https://github.com/manticoresoftware/manticoresearch/issues/872) 崩溃后表变得不可见
* [Issue #877](https://github.com/manticoresoftware/manticoresearch/issues/877) 搜索查询中两个负面词导致错误：查询不可计算
* [Issue #878](https://github.com/manticoresoftware/manticoresearch/issues/878) a -b -c 在 JSON query_string 中不起作用
* [Issue #886](https://github.com/manticoresoftware/manticoresearch/issues/886) pseudo_sharding 与查询匹配
* [Issue #893](https://github.com/manticoresoftware/manticoresearch/issues/893) Manticore 5.0.2 min/max 函数不能按预期工作...
* [Issue #896](https://github.com/manticoresoftware/manticoresearch/issues/896) 字段 "weight" 解析不正确
* [Issue #897](https://github.com/manticoresoftware/manticoresearch/issues/897) Manticore 服务启动时崩溃并持续重启
* [Issue #900](https://github.com/manticoresoftware/manticoresearch/issues/900) group by j.a, smth 工作异常
* [Issue #913](https://github.com/manticoresoftware/manticoresearch/issues/913) 当 ranker 中使用 expr，但查询包含两个近似时，searchd 崩溃
* [Issue #916](https://github.com/manticoresoftware/manticoresearch/issues/916) net_throttle_action 失效
* [Issue #919](https://github.com/manticoresoftware/manticoresearch/issues/919) MCL：执行查询时 Manticore 崩溃，集群恢复期间也崩溃。
* [Issue #925](https://github.com/manticoresoftware/manticoresearch/issues/925) SHOW CREATE TABLE 输出缺少反引号
* [Issue #930](https://github.com/manticoresoftware/manticoresearch/issues/930) 现在可以通过 JDBC 连接器从 Java 查询 Manticore
* [Issue #933](https://github.com/manticoresoftware/manticoresearch/issues/933) bm25f 排名问题
* [Issue #934](https://github.com/manticoresoftware/manticoresearch/issues/934) 无配置索引在 watchdog 中冻结于首次加载状态
* [Issue #937](https://github.com/manticoresoftware/manticoresearch/issues/937) 排序分面数据时发生段错误
* [Issue #940](https://github.com/manticoresoftware/manticoresearch/issues/940) CONCAT(TO_STRING) 崩溃
* [Issue #947](https://github.com/manticoresoftware/manticoresearch/issues/947) 在某些情况下，单个简单的 select 查询可能导致整个实例停止响应，因此您无法登录或执行其他查询，直到正在运行的 select 完成。
* [Issue #948](https://github.com/manticoresoftware/manticoresearch/issues/948) 索引器崩溃
* [Issue #950](https://github.com/manticoresoftware/manticoresearch/issues/950) facet distinct 计数错误
* [Issue #953](https://github.com/manticoresoftware/manticoresearch/issues/953) 内置 sph04 排名器中的 LCS 计算不正确
* [Issue #955](https://github.com/manticoresoftware/manticoresearch/issues/955) 5.0.3 版本开发中崩溃
* [Issue #963](https://github.com/manticoresoftware/manticoresearch/issues/963) 在引擎列存储中使用 json 的 FACET 崩溃
* [Issue #982](https://github.com/manticoresoftware/manticoresearch/issues/982) MCL：5.0.3 版本中从二级索引崩溃
* [PR #984](https://github.com/manticoresoftware/manticoresearch/pull/984) @@autocommit 在 HandleMysqlSelectSysvar 中
* [PR #985](https://github.com/manticoresoftware/manticoresearch/pull/985) 修复 RT 索引中的线程块分配
* [Issue #985](https://github.com/manticoresoftware/manticoresearch/issues/985) 修复 RT 索引中的线程块分配
* [Issue #986](https://github.com/manticoresoftware/manticoresearch/issues/986) 默认 max_query_time 错误
* [Issue #987](https://github.com/manticoresoftware/manticoresearch/issues/987) 多线程执行中使用正则表达式导致崩溃
* [Issue #988](https://github.com/manticoresoftware/manticoresearch/issues/988) 向后索引兼容性破损
* [Issue #989](https://github.com/manticoresoftware/manticoresearch/issues/989) indextool 报告检查列存储属性错误
* [Issue #990](https://github.com/manticoresoftware/manticoresearch/issues/990) json 聚合克隆的内存泄漏
* [Issue #991](https://github.com/manticoresoftware/manticoresearch/issues/991) levenshtein 函数克隆的内存泄漏
* [Issue #992](https://github.com/manticoresoftware/manticoresearch/issues/992) 加载元数据时错误信息丢失
* [Issue #993](https://github.com/manticoresoftware/manticoresearch/issues/993) 传播来自动态索引/子键和系统变量的错误
* [Issue #994](https://github.com/manticoresoftware/manticoresearch/issues/994) 在列存储中对列存储字符串计算 count distinct 崩溃
* [Issue #995](https://github.com/manticoresoftware/manticoresearch/issues/995) MCL：taxi1 中的 min(pickup_datetime) 崩溃
* [Issue #997](https://github.com/manticoresoftware/manticoresearch/issues/997) 空的 excludes JSON 查询从选择列表中移除列
* [Issue #998](https://github.com/manticoresoftware/manticoresearch/issues/998) 当前调度器上的二级任务有时会导致异常副作用
* [Issue #999](https://github.com/manticoresoftware/manticoresearch/issues/999) 使用 facet distinct 和不同模式时崩溃
* [Issue #1000](https://github.com/manticoresoftware/manticoresearch/issues/1000) MCL：未加载列存储库就运行导致列存储实时索引损坏
* [Issue #1001](https://github.com/manticoresoftware/manticoresearch/issues/1001) json 中隐式 cutoff 无效
* [Issue #1002](https://github.com/manticoresoftware/manticoresearch/issues/1002) 列存储聚合器问题
* [Issue #1003](https://github.com/manticoresoftware/manticoresearch/issues/1003) 无法删除索引中的最后一个字段
* [Issue #1004](https://github.com/manticoresoftware/manticoresearch/issues/1004) --new-cluster 后行为异常
* [Issue #1005](https://github.com/manticoresoftware/manticoresearch/issues/1005) “columnar library not loaded”，但其实不需要加载
* [Issue #1006](https://github.com/manticoresoftware/manticoresearch/issues/1006) 删除查询无错误提示
* [Issue #1010](https://github.com/manticoresoftware/manticoresearch/issues/1010) 修复 Windows 版本中的 ICU 数据文件位置
* [PR #1018](https://github.com/manticoresoftware/manticoresearch/pull/1018) 握手发送问题
* [Issue #1020](https://github.com/manticoresoftware/manticoresearch/issues/1020) 在 show create table 中显示 id
* [Issue #1024 crash 1](https://github.com/manticoresoftware/manticoresearch/issues/1024) Facet 搜索结果数量较大时发生崩溃/段错误
* [Issue #1026](https://github.com/manticoresoftware/manticoresearch/issues/1026) RT 索引：当大量文档插入且 RAMchunk 已满时，searchd “永远卡住”
* [Commit 4739](https://github.com/manticoresoftware/manticoresearch/commit/4739dafd) 复制节点间忙碌时关闭线程卡住
* [Commit ab87](https://github.com/manticoresoftware/manticoresearch/commit/ab87836782e7ae43fe5f9dea739eed0d336b42c1) JSON 范围过滤器中混用浮点数和整数可能导致 Manticore 忽略过滤器
* [Commit d001](https://github.com/manticoresoftware/manticoresearch/commit/d00101c2905f6393ce3fca23c4f6dcb2506f4bd9) JSON 中的浮点过滤器不准确
* [Commit 4092](https://github.com/manticoresoftware/manticoresearch/commit/4092c6a8) 如果索引被修改，则丢弃未提交的事务（否则可能崩溃）
* [Commit 9692](https://github.com/manticoresoftware/manticoresearch/commit/9692a840) 使用反斜杠时查询语法错误
* [Commit 0c19](https://github.com/manticoresoftware/manticoresearch/commit/0c19ad8d) SHOW STATUS 中 workers_clients 可能错误
* [Commit 1772](https://github.com/manticoresoftware/manticoresearch/commit/17720e1e2f2ec38337df565bee78f7ce9d9c744a) 修复了合并无文档存储的内存段时的崩溃问题
* [Commit f45b](https://github.com/manticoresoftware/manticoresearch/commit/f45b337b5c265433bb7d6262ad65d02f65edbaf3) 修复了等于 JSON 过滤器中遗漏的 ALL/ANY 条件
* [Commit 3e83](https://github.com/manticoresoftware/manticoresearch/commit/3e83c1f0c0e6e7a9d4297113f69a112cbabd9e64) 如果 searchd 是从它无法写入的目录启动，复制可能会失败并出现 `got exception while reading ist stream: mkstemp(./gmb_pF6TJi) failed: 13 (Permission denied)` 异常。
* [Commit 92e5](https://github.com/manticoresoftware/manticoresearch/commit/92e5629f7150f62524b385fca670a01ba9590f5b) 自 4.0.2 版本以来崩溃日志仅包含偏移，此提交修复了这一问题。

## 版本 5.0.2
发布：2022 年 5 月 30 日

### Bug 修复
* ❗[Issue #791](https://github.com/manticoresoftware/manticoresearch/issues/791) - 错误的栈大小可能导致崩溃。

## 版本 5.0.0
发布：2022 年 5 月 18 日


### 主要新功能
* 🔬 支持 [Manticore 列库 1.15.2](https://github.com/manticoresoftware/columnar/)，该版本支持 [二级索引](Server_settings/Searchd.md#secondary_indexes) 测试版。默认对普通和实时的列式及行式索引（如果使用了 [Manticore 列库](https://github.com/manticoresoftware/columnar)）开启二级索引构建，但要启用搜索需要在配置文件或使用 [SET GLOBAL](Server_settings/Setting_variables_online.md) 设置 `secondary_indexes = 1`。此新功能支持除 Debian Stretch 旧版和 Ubuntu Xenial 外的所有操作系统。
* [只读模式](Security/Read_only.md)：现在可以指定只处理读取查询的监听器，丢弃所有写操作。
* 新增 [/cli](Connecting_to_the_server/HTTP.md#/cli) 端点，通过 HTTP 运行 SQL 查询更方便。
* 通过 JSON over HTTP 的批量 INSERT/REPLACE/DELETE 速度更快：之前可通过 HTTP JSON 协议提供多个写命令，但逐个处理，现在作为单一事务处理。
* [#720](https://github.com/manticoresoftware/manticoresearch/issues/720) JSON 协议支持 [嵌套过滤器](Searching/Filters.md#Nested-bool-query)。此前 JSON 中无法实现如 `a=1 and (b=2 or c=3)` ，`must` (AND), `should` (OR) 和 `must_not` (NOT) 仅支持最高级别，现在支持嵌套。
* 支持 HTTP 协议中的 [分块传输编码](https://en.wikipedia.org/wiki/Chunked_transfer_encoding)。应用现在可使用分块传输以减少资源消耗（无需计算 `Content-Length`）传输大批量数据。服务端 Manticore 现始终以流式方式处理进来的 HTTP 数据，无需等待整个批次传输完成，这：
  - 降低峰值内存使用，减少 OOM 风险
  - 缩短响应时间（测试显示处理 100MB 批量时减少约 11%）
  - 允许绕过 [max_packet_size](Server_settings/Searchd.md#max_packet_size) 限制，传输远大于允许的最大值（128MB）的批量数据，比如一次传输 1GB。
* [#719](https://github.com/manticoresoftware/manticoresearch/issues/719) HTTP 接口支持 `100 Continue`：现在可以从 `curl`（及包括各种编程语言使用的 curl 库）传输大型批次，这些会默认发出 `Expect: 100-continue` 并等待一段时间后才真正发送批次。以前需要添加 `Expect: ` 头部，现在不再需要。

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
  real
0m1.022s
  user
0m0.001s

  sys
0m0.010s
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
  real

0m0.015s

  user
0m0.005s
  sys
0m0.004s
  ```
  </details>

* **⚠️ 重大更改**：[伪分片](Server_settings/Searchd.md#pseudo_sharding) 默认启用。如果您想禁用它，请确保在 Manticore 配置文件的 `searchd` 部分添加 `pseudo_sharding = 0`。
* 在实时/普通索引中至少有一个全文字段不再是强制性的。现在，即使在与全文搜索无关的情况下，您也可以使用 Manticore。
* 由 [Manticore 列存库](https://github.com/manticoresoftware/columnar) 支持的属性的[快速提取](Creating_a_table/Data_types.md#fast_fetch)：对 `<columnar 表>` 执行 `select *` 这类查询现在比以前快得多，尤其是当模式中有许多字段时。
* **⚠️ 重大更改**：隐式[cutoff](Searching/Options.md#cutoff)。Manticore 现在不会花费时间和资源处理结果集中您不需要的数据。缺点是这影响了 [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META) 中的 `total_found` 和 JSON 输出中的 [hits.total](Searching/Full_text_matching/Basic_usage.md#HTTP-JSON)。现在只有在看到 `total_relation: eq` 时才准确，而 `total_relation: gte` 表示实际匹配文档数大于您得到的 `total_found` 值。要保留之前的行为，可以使用搜索选项 `cutoff=0`，这会使 `total_relation` 始终为 `eq`。
* **⚠️ 重大更改**：所有全文字段现在默认[存储](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#stored_fields)。您需要使用 `stored_fields = `（空值）使所有字段不存储（即恢复之前的行为）。
* [#715](https://github.com/manticoresoftware/manticoresearch/issues/715) HTTP JSON 支持[搜索选项](Searching/Options.md#General-syntax)。
### 小改进
* **⚠️ 重大更改**：索引元数据文件格式变更。以前元数据文件（`.meta`、`.sph`）为二进制格式，现在改为纯 JSON。新版 Manticore 会自动转换旧索引，但：
  - 可能会收到类似 `WARNING: ... syntax error, unexpected TOK_IDENT` 的警告
  - 旧版本 Manticore 将无法运行该索引，请确保备份
* **⚠️ 重大更改**：支持通过[HTTP Keep-alive](Connecting_to_the_server/HTTP.md#Keep-alive)实现会话状态。这样当客户端也支持时，HTTP 变为有状态。例如，使用新的 [/cli](Connecting_to_the_server/HTTP.md#/cli) 端点和 HTTP Keep-alive（所有浏览器默认开启）时，您可以在执行 `SELECT` 后调用 `SHOW META`，其行为与通过 mysql 相同。注意，之前虽然支持 `Connection: keep-alive` HTTP 头，但只是复用了连接。从此版本起，它也使会话变为有状态。
* 现在可以在[普通模式](Read_this_first.md#Real-time-mode-vs-plain-mode) 中指定 `columnar_attrs = *`，将所有属性定义为列存储，在属性列表长时非常有用。
* 复制 SST 更快。
* **⚠️ 重大更改**：复制协议已更改。如果您运行复制集群，升级到 Manticore 5 时需要：
  - 先干净地停止所有节点
  - 然后用 `--new-cluster` 启动最后停止的节点（Linux 下运行工具 `manticore_new_cluster`）
  - 详情请参阅[重启集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster)
* 复制改进：
  - SST 更快
  - 噪声抗扰性提升，有助于复制节点之间网络不稳定时
  - 日志改进
* 安全性改进：当配置中未指定任何 `listen` 时，Manticore 现在监听 `127.0.0.1` 而非 `0.0.0.0`。尽管 Manticore Search 所带的默认配置中有明确的 `listen` 设置，且一般不会出现完全没有 `listen` 的配置，但这种情况仍然可能发生。之前 Manticore 会监听在不安全的 `0.0.0.0` 上，现在改为监听通常不会暴露在互联网的 `127.0.0.1`。
* 更快的列式属性聚合。
* 提升了 `AVG()` 的精度：之前 Manticore 聚合内部使用 `float`，现在改为使用 `double`，显著提高了精度。
* 改进了对 JDBC MySQL 驱动的支持。
* 为 [jemalloc](https://github.com/jemalloc/jemalloc) 增加了 `DEBUG malloc_stats` 支持。
* [optimize_cutoff](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#optimize_cutoff) 现已作为每表设置提供，可在 CREATE 或 ALTER 表时设置。
* **⚠️ 重大变更**：[query_log_format](Server_settings/Searchd.md#query_log_format) 默认值现为 **`sphinxql`**。如果你习惯 `plain` 格式，需要在配置文件中添加 `query_log_format = plain`。
* 显著降低内存占用：在使用存储字段且进行长时间密集的插入/替换/优化负载时，Manticore 现在消耗显著更少的 RAM。
* [shutdown_timeout](Server_settings/Searchd.md#shutdown_timeout) 默认值从 3 秒调整为 60 秒。

* [提交 ffd0](https://github.com/manticoresoftware/manticoresearch/commit/ffd0499d329d2c383f14c8a44c4cc84338ab56e7) 支持 Java mysql connector >= 6.0.3：在 [Java mysql 6.0.3 连接](https://mvnrepository.com/artifact/mysql/mysql-connector-java/6.0.3) 中，连接 MySQL 的方式发生了变化，导致与 Manticore 不兼容。现在支持新行为。
* [提交 1da6](https://github.com/manticoresoftware/manticoresearch/commit/1da6dbec) 禁用了加载索引（如 searchd 启动时）时保存新的磁盘块。
* [问题 #746](https://github.com/manticoresoftware/manticoresearch/issues/746) 支持 glibc >= 2.34。
* [问题 #784](https://github.com/manticoresoftware/manticoresearch/issues/784) 将“VIP”连接单独计数，与普通（非 VIP）连接区分开。之前 VIP 连接计入 `max_connections` 限制，可能导致非 VIP 连接出现“连接数已满”错误。现在 VIP 连接不计入此限制。当前 VIP 连接数同样可以在 `SHOW STATUS` 和 `status` 中查看。
* [ID](Creating_a_table/Data_types.md#Document-ID) 现在可以显式指定。
* [问题 #687](https://github.com/manticoresoftware/manticoresearch/issues/687) 为 MySQL 协议支持 zstd 压缩。
### ⚠️ 其他小的破坏性更改
* ⚠️ BM25F 公式略微更新以提升搜索相关性。仅影响使用函数 [BM25F()](Functions/Searching_and_ranking_functions.md#BM25F%28%29) 的搜索结果，不改变默认排序公式的行为。
* ⚠️ REST [/sql](Connecting_to_the_server/HTTP.md#mode=raw) 端点行为改变：`/sql?mode=raw` 现在需要转义，并返回数组。

* ⚠️ `/bulk` 的 INSERT/REPLACE/DELETE 请求响应格式变化：
  - 之前每个子查询视为单独事务，返回独立响应
  - 现在整个批次视为单一事务，返回单一响应
* ⚠️ 搜索选项 `low_priority` 和 `boolean_simplify` 现在需要赋值（`0/1`）：之前可写为 `SELECT ... OPTION low_priority, boolean_simplify`，现需写为 `SELECT ... OPTION low_priority=1, boolean_simplify=1`。
* ⚠️ 如果你在使用旧版 [php](https://github.com/manticoresoftware/manticoresearch/blob/master/api/sphinxapi.php)、[python](https://github.com/manticoresoftware/manticoresearch/blob/master/api/sphinxapi.py) 或 [java](https://github.com/manticoresoftware/manticoresearch/tree/master/api/java) 客户端，请访问对应链接获取新版。**旧版与 Manticore 5 不完全兼容。**
* ⚠️ HTTP JSON 请求在 `query_log_format=sphinxql` 模式下日志格式已变化。之前仅记录全文部分，现在直接按原样记录。
### 新的软件包
* **⚠️ 重大变更**：鉴于新版结构，升级到 Manticore 5 时建议先卸载旧软件包然后再安装新版本：
  - 基于 RPM：`yum remove manticore*`

  - Debian 和 Ubuntu：`apt remove manticore*`
* 新的 deb/rpm 包结构。之前版本提供：
  - `manticore-server` 含 `searchd`（主搜索守护进程）及其所有依赖
  - `manticore-tools` 含 `indexer` 和 `indextool`
  - `manticore` 包含所有组件
  - `manticore-all` RPM 元包，指向 `manticore-server` 和 `manticore-tools`
  新结构为：
  - `manticore` - deb/rpm 元包，安装所有上述作为依赖
  - `manticore-server-core` - 仅包含 `searchd` 及其运行所需内容
  - `manticore-server` - 包含 systemd 文件及其他辅助脚本

  - `manticore-tools` - `indexer`、`indextool` 及其他工具
  - `manticore-common` - 默认配置文件、默认数据目录、默认停用词
  - `manticore-icudata`、`manticore-dev`、`manticore-converter` 变化不大
  - 提供包含所有软件包的 `.tgz` 压缩包
* 支持 Ubuntu Jammy
* 支持通过 [YUM 仓库](Installation/RHEL_and_Centos.md#YUM-repository) 安装 Amazon Linux 2
### 修复的 Bug
* [问题 #815](https://github.com/manticoresoftware/manticoresearch/issues/815) 使用 UDF 函数时随机崩溃
* [问题 #287](https://github.com/manticoresoftware/manticoresearch/issues/287) RT 索引时内存溢出
* [问题 #604](https://github.com/manticoresoftware/manticoresearch/issues/604) 3.6.0、4.2.0 sphinxql 解析器的破坏性更改
* [问题 #667](https://github.com/manticoresoftware/manticoresearch/issues/667) FATAL: 内存不足（无法分配 9007199254740992 字节）
* [问题 #676](https://github.com/manticoresoftware/manticoresearch/issues/676) 字符串未正确传递给 UDF
* ❗[问题 #698](https://github.com/manticoresoftware/manticoresearch/issues/698) 尝试向 rt 索引添加文本列后 searchd 崩溃
* [问题 #705](https://github.com/manticoresoftware/manticoresearch/issues/705) Indexer 找不到所有列
* ❗[问题 #709](https://github.com/manticoresoftware/manticoresearch/issues/705) 按 json.boolean 分组错误
* [问题 #716](https://github.com/manticoresoftware/manticoresearch/issues/716) 与索引相关的 indextool 命令（例如 --dumpdict）失败
* ❗[问题 #724](https://github.com/manticoresoftware/manticoresearch/issues/724) 选中的字段消失
* [问题 #727](https://github.com/manticoresoftware/manticoresearch/issues/727) .NET HttpClient 在使用 `application/x-ndjson` 时 Content-Type 不兼容
* [问题 #729](https://github.com/manticoresoftware/manticoresearch/issues/729) 字段长度计算
* ❗[问题 #730](https://github.com/manticoresoftware/manticoresearch/issues/730) 创建/插入到/删除列式表存在内存泄漏
* [问题 #731](https://github.com/manticoresoftware/manticoresearch/issues/731) 在某些条件下结果中的空列
* ❗[问题 #749](https://github.com/manticoresoftware/manticoresearch/issues/749) 守护进程启动时崩溃
* ❗[问题 #750](https://github.com/manticoresoftware/manticoresearch/issues/750) 守护进程启动时挂起
* ❗[问题 #751](https://github.com/manticoresoftware/manticoresearch/issues/751) SST 时崩溃
* [问题 #752](https://github.com/manticoresoftware/manticoresearch/issues/752) 当 engine='columnar' 时 json 属性被标记为列式
* [问题 #753](https://github.com/manticoresoftware/manticoresearch/issues/753) 复制监听 0
* [问题 #754](https://github.com/manticoresoftware/manticoresearch/issues/754) columnar_attrs = * 不适用于 csvpipe
* ❗[问题 #755](https://github.com/manticoresoftware/manticoresearch/issues/755) 在 rt 的列式中选择浮点数时崩溃
* ❗[问题 #756](https://github.com/manticoresoftware/manticoresearch/issues/756) Indextool 在检查期间更改 rt 索引
* [问题 #757](https://github.com/manticoresoftware/manticoresearch/issues/757) 需要检查监听器端口范围交叉
* [问题 #758](https://github.com/manticoresoftware/manticoresearch/issues/758) 在 RT 索引保存磁盘块失败时记录原始错误
* [问题 #759](https://github.com/manticoresoftware/manticoresearch/issues/759) RE2 配置只报告一个错误
* ❗[问题 #760](https://github.com/manticoresoftware/manticoresearch/issues/760) 提交 5463778558586d2508697fa82e71d657ac36510f 中的 RAM 消耗变化
* [问题 #761](https://github.com/manticoresoftware/manticoresearch/issues/761) 第三个节点在脏重启后未组成非主集群
* [问题 #762](https://github.com/manticoresoftware/manticoresearch/issues/762) 更新计数增加了 2
* [问题 #763](https://github.com/manticoresoftware/manticoresearch/issues/763) 使用形态学的 4.2.0 创建的索引在新版本 4.2.1 中损坏
* [问题 #764](https://github.com/manticoresoftware/manticoresearch/issues/764) json 键 /sql?mode=raw 中无转义
* ❗[问题 #765](https://github.com/manticoresoftware/manticoresearch/issues/765) 使用函数隐藏其他值
* ❗[问题 #766](https://github.com/manticoresoftware/manticoresearch/issues/766) FixupAttrForNetwork 中的一行触发内存泄漏
* ❗[问题 #767](https://github.com/manticoresoftware/manticoresearch/issues/767) 4.2.0 和 4.2.1 中与 docstore 缓存相关的内存泄漏
* [问题 #768](https://github.com/manticoresoftware/manticoresearch/issues/768) 网络上传送字段出现奇怪的乒乓现象
* [问题 #769](https://github.com/manticoresoftware/manticoresearch/issues/769) 如果在 'common' 部分没有提及，lemmatizer_base 会被重置为空
* [问题 #770](https://github.com/manticoresoftware/manticoresearch/issues/770) pseudo_sharding 使按 id 查询变慢
* [问题 #771](https://github.com/manticoresoftware/manticoresearch/issues/771) 使用 jemalloc 时 DEBUG malloc_stats 输出为零
* [问题 #772](https://github.com/manticoresoftware/manticoresearch/issues/772) 删除/添加列导致值不可见
* [问题 #773](https://github.com/manticoresoftware/manticoresearch/issues/773) 无法向列式表添加 bit(N) 列
* [问题 #774](https://github.com/manticoresoftware/manticoresearch/issues/774) 启动时 manticore.json 中的 "cluster" 变空

* ❗[提交 1da4](https://github.com/manticoresoftware/manticoresearch/commits/1da4ce891e4ff13727cfd3331d72771c64ee7949) HTTP 操作未在 SHOW STATUS 中跟踪

* [提交 3810](https://github.com/manticoresoftware/manticoresearch/commits/381000ab4af4c3c98ec8c730699ad7f39039cec8) 禁用低频单关键词查询的 pseudo_sharding
* [提交 8003](https://github.com/manticoresoftware/manticoresearch/commits/800325cca283a87801b28929d82420fafc76a0ee) 修复存储属性与索引合并的问题

* [提交 cddf](https://github.com/manticoresoftware/manticoresearch/commits/cddfeed6296f2041c6aae18414e5cd9cabf08281) 泛化不同值获取器；添加了针对列式字符串的专用不同值获取器

* [提交 fba4](https://github.com/manticoresoftware/manticoresearch/commits/fba4bb4f) 修复从 docstore 获取空整数属性的问题

* [提交 f300](https://github.com/manticoresoftware/manticoresearch/commit/f3009a9242fa16956adf871c4887e6e0303ba364) 查询日志中 `ranker` 可能被指定两次
## 版本 4.2.0，2021年12月23日

### 主要新特性
* **实时索引和全文查询的伪分片支持**。在之前的版本中，我们添加了有限的伪分片支持。从本版本开始，只需启用[searchd.pseudo_sharding](Server_settings/Searchd.md#pseudo_sharding)，即可获得伪分片和多核处理器的所有好处。最棒的是，您不需要对索引或查询做任何操作，只需启用它，如果有空闲的CPU资源，就会用来降低响应时间。它支持全文、本地和实时索引的全文过滤和分析查询。例如，以下是在[Hacker news curated comments dataset](https://zenodo.org/record/45901/)（乘以100倍，即平面索引中包含1.16亿文档）上启用伪分片后，大多数查询的**平均响应时间降低约10倍**的情况。

![Pseudo sharding on vs off in 4.2.0](4.2.0_ps_on_vs_off.png)

* 现在支持[**Debian Bullseye**](https://manticoresearch.com/install/)。

<!-- example pq_transactions_4.2.0 -->
* PQ事务现在是原子且隔离的。以前PQ事务的支持有限。它使得**REPLACE到PQ的速度更快**，尤其是在需要同时替换大量规则时。性能详情：

<!-- intro -->
#### 4.0.2
<!-- request 4.0.2 -->
插入100万条PQ规则需要**48秒**，而分批替换仅40K条（每批1万条）需要**406秒**。
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
插入100万条PQ规则需要**34秒**，而分批替换它们则需要**23秒**（每批1万条）。
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
### 小改动
* [optimize_cutoff](Server_settings/Searchd.md#optimize_cutoff) 现在作为`searchd`配置节中的选项可用。当您想将所有索引中RT块数量全局限制为特定数字时，此配置非常有用。
* [Commit 0087](https://github.com/manticoresoftware/manticoresearch/commit/00874743823eaf43e2a09a088392fd7e0a617f47) 实现了对多个具有相同字段集/顺序的本地物理索引（实时/平面）进行准确的[count(distinct ...)](Searching/Grouping.md#COUNT%28DISTINCT-field%29)和[FACET ... distinct](Searching/Faceted_search.md#Faceting-without-duplicates)功能。
* [PR #598](https://github.com/manticoresoftware/manticoresearch/pull/598) 为`YEAR()`和其他时间戳函数添加了bigint支持。

* [Commit 8e85](https://github.com/manticoresoftware/manticoresearch/commit/8e85d4bce596cc88f9d9615ea243a27e4afde0f7) 自适应 [rt_mem_limit](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_mem_limit)。以前，Manticore Search会收集最多`rt_mem_limit`的数据后保存新磁盘块，在保存时仍然会收集最多10%（即双缓冲区）以最小化插入中断的可能性。如果该限制也被耗尽，则在磁盘块完全写入磁盘之前，添加新文档将被阻塞。新的自适应限制基于现在有了[自动优化](Server_settings/Searchd.md#auto_optimize)这一事实，因此磁盘块不完全遵守`rt_mem_limit`而较早开始刷新不是大问题。现在我们收集`rt_mem_limit`的最多50%后保存为磁盘块。在保存时，我们查看统计信息（保存了多少文档，保存期间有多少新文档到达）并重新计算下次的初始率。例如，如果我们保存了9000万文档，而保存期间又来了1000万，比例就是90%，所以下次我们知道可以收集`rt_mem_limit`的90%后开始刷新另一个磁盘块。比例值自动计算，范围从33.3%到95%。
* [Issue #628](https://github.com/manticoresoftware/manticoresearch/issues/628) 针对PostgreSQL源支持[unpack_zlib](Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Database_connection.md#unpack_zlib)。感谢[Dmitry Voronin](https://github.com/dimv36)的[贡献](https://github.com/manticoresoftware/manticoresearch/pull/630)。
* [Commit 6d54](https://github.com/manticoresoftware/manticoresearch/commit/6d54cf2b319b131970c29410cc21abcbdf8083b1) 支持`indexer -v`和`--version`。以前你仍然可以看到indexer版本，但`-v`/`--version`命令不支持。
* [Issue #662](https://github.com/manticoresoftware/manticoresearch/issues/662) 当通过systemd启动Manticore时，默认无限mlock限制。
* [Commit 63c8](https://github.com/manticoresoftware/manticoresearch/commit/63c8cd05cf399e705c2c14939411a92cb784735a) 将自旋锁改为协程读写锁的操作队列。

* [Commit 4113](https://github.com/manticoresoftware/manticoresearch/commit/41130ce30d1734bbf8e6b8b63c4e815caf9464f2) 环境变量`MANTICORE_TRACK_RT_ERRORS`，有助于调试RT段损坏。
### 重大变动
* binlog版本升级，之前版本的binlog不会被重放，因此升级时确保干净地停止Manticore Search：停止之前实例后，`/var/lib/manticore/binlog/`目录中不应有binlog文件，除`binlog.meta`外。
* [Commit 3f65](https://github.com/manticoresoftware/manticoresearch/commit/3f659f36e87d99ee262205a8eae4663a255be630) 在`show threads option format=all`中新增列“chain”。它显示某些任务信息票据的堆栈，主要用于分析需求。如果您解析`show threads`输出，请注意新增的列。
* `searchd.workers`自3.5.0起被废弃，现在已弃用，如果配置文件中仍有它，启动时会触发警告。Manticore Search会启动，但伴有警告。
* 如果您使用PHP和PDO访问Manticore，需要设置`PDO::ATTR_EMULATE_PREPARES`。
### Bug修复
* ❗[Issue #650](https://github.com/manticoresoftware/manticoresearch/issues/650) Manticore 4.0.2比Manticore 3.6.3慢。4.0.2版本在批量插入方面比之前版本更快，但单文档插入明显变慢。在4.2.0中已修复。
* ❗[提交 22f4](https://github.com/manticoresoftware/manticoresearch/commit/22f4141bed1411c83684ae6e0bae832071cce3df) RT 索引在高强度 REPLACE 负载下可能损坏，或可能崩溃
* [提交 03be](https://github.com/manticoresoftware/manticoresearch/commit/03be91e4f720a3f20bee0278dd8f0d44abd3ef0e) 修复合并 groupers 和 group N 排序器时的平均值；修复聚合合并
* [提交 2ea5](https://github.com/manticoresoftware/manticoresearch/commit/2ea575d34efe38661f14d858f2b552456f29fe42) `indextool --check` 可能崩溃
* [提交 7ec7](https://github.com/manticoresoftware/manticoresearch/commit/7ec76d4abf01b5fca6f10a0000fe64d5d8e5fa52) 由 UPDATE 导致的内存耗尽问题
* [提交 658a](https://github.com/manticoresoftware/manticoresearch/commit/658a727e41f3c3fbcde9754149528f34ed76ba19) 守护进程可能在 INSERT 时挂起
* [提交 46e4](https://github.com/manticoresoftware/manticoresearch/commit/46e42b9b386e31e53247ed634e158b533e28063e) 守护进程可能在关闭时挂起
* [提交 f8d7](https://github.com/manticoresoftware/manticoresearch/commit/f8d7d5177cc83d250b493b4cfc619732b29cf072) 守护进程关闭时可能崩溃
* [提交 733a](https://github.com/manticoresoftware/manticoresearch/commit/733accf165142a1d1574689525ccca8907971158) 守护进程崩溃时可能挂起
* [提交 f7f8](https://github.com/manticoresoftware/manticoresearch/commit/f7f8bd8c0dbb23a59a88502dcba16b0918fb2330) 守护进程启动时尝试用无效节点列表重新加入集群可能崩溃
* [提交 1401](https://github.com/manticoresoftware/manticoresearch/commit/140155613821ecaddcd1a3dfc4f7369b0df6aef7) 分布式索引在 RT 模式下启动时如果无法解析某个代理，可能被完全遗忘
* [问题 #683](https://github.com/manticoresoftware/manticoresearch/issues/683) attr bit(N) engine='columnar' 失败
* [问题 #682](https://github.com/manticoresoftware/manticoresearch/issues/682) 创建表失败，但留下目录
* [问题 #663](https://github.com/manticoresoftware/manticoresearch/issues/663) 配置失败，提示未知键名 'attr_update_reserve'
* [问题 #632](https://github.com/manticoresoftware/manticoresearch/issues/632) Manticore 批量查询崩溃

* [问题 #679](https://github.com/manticoresoftware/manticoresearch/issues/679) 批量查询在 v4.0.3 中再次导致崩溃

* [提交 f7f8](https://github.com/manticoresoftware/manticoresearch/commit/f7f8bd8c0dbb23a59a88502dcba16b0918fb2330) 修复守护进程启动时用无效节点列表重新加入集群崩溃
* [问题 #643](https://github.com/manticoresoftware/manticoresearch/issues/643) Manticore 4.0.2 在批量插入后不接受连接
* [问题 #635](https://github.com/manticoresoftware/manticoresearch/issues/635) FACET 查询携带 ORDER BY JSON.field 或字符串属性可能崩溃
* [问题 #634](https://github.com/manticoresoftware/manticoresearch/issues/634) 查询带 packedfactors 出现 SIGSEGV 崩溃
* [提交 4165](https://github.com/manticoresoftware/manticoresearch/commits/41657f15f8fcf1e93cfad7f835bcf705d6e8349e) morphology_skip_fields 不支持 create table
## 版本 4.0.2，2021年9月21日
### 主要新功能
- **完全支持 [Manticore Columnar Library](https://github.com/manticoresoftware/columnar)**。此前 Manticore Columnar Library 只支持普通索引。现在支持：
  - 实时索引中的 `INSERT`、`REPLACE`、`DELETE`、`OPTIMIZE`

  - 复制

  - `ALTER`
  - `indextool --check`
- **自动索引压缩** ([问题 #478](https://github.com/manticoresoftware/manticoresearch/issues/478))。终于，无需手动调用 OPTIMIZE，也无需用定时任务或其它自动化方式。Manticore 现在默认自动完成。你可以通过全局变量 [optimize_cutoff](Server_settings/Setting_variables_online.md) 设置默认压缩阈值。
- **数据块快照和锁系统重构**。这些改动乍看可能不明显，但显著改善了实时索引中的许多操作行为。简而言之，之前大多数 Manticore 数据操作重度依赖锁，如今改用磁盘块快照。
- **实时索引的批量 INSERT 性能显著提升**。例如，在 [Hetzner 服务器 AX101](https://www.hetzner.com/dedicated-rootserver/ax101)（SSD、128 GB 内存、AMD Ryzen™ 9 5950X（16*2 核））上，**3.6.0 版本可达到每秒插入 236K 条文档**，表模式为 `name text, email string, description text, age int, active bit(1)`（默认 `rt_mem_limit`，批大小 25000，16 并发插入工作线程，总插入 1600 万文档）。4.0.2 版本在相同并发/批次/数量条件下，**达到每秒 357K 条文档**。
  <details>
  - 读取操作（如 SELECT、复制）基于快照执行

  - 仅更改内部索引结构且不修改 schema/文档的操作（如合并 RAM 段、保存磁盘块、合并磁盘块）使用只读快照执行，最终替换现有数据块

  - UPDATE 和 DELETE 针对现有数据块执行，但合并时会先收集写入操作，然后应用到新数据块上
  - UPDATE 顺序获取每个数据块的独占锁。合并进入收集该数据块属性阶段时获取共享锁。即同一时间只有一个（合并或更新）操作能访问数据块属性

  - 合并进入需要属性阶段时设置特殊标志。UPDATE 完成后检查该标志，如设置则整批更新保存到专用集合。合并完成后，把更新应用到新磁盘块
  - ALTER 通过独占锁运行
  - 复制正常作为读取操作，同时在 SST 前保存属性，且 SST 期间禁止更新
  </details>
- **[ALTER](Updating_table_schema_and_settings.md) 可以添加/删除全文字段**（在 RT 模式下）。之前它只能添加/删除一个属性。


- 🔬 **实验性功能：针对全扫描查询的伪分片** - 允许并行化任何非全文搜索查询。现在不需要手动准备分片，只需启用新选项 [searchd.pseudo_sharding](Server_settings/Searchd.md#pseudo_sharding)，即可期望非全文搜索查询的响应时间减少至最多 `CPU 核心数`。注意，它可能会轻易占用所有现有 CPU 核心，因此如果你不仅关心延迟，还关心吞吐量——请谨慎使用。
### 小更新

<!-- example -->
- 支持 Linux Mint 和 Ubuntu Hirsute Hippo，通过 [APT 仓库](Installation/Debian_and_Ubuntu.md#APT-repository)
- 在某些情况下，通过 HTTP 按 id 更新大索引更快（取决于 id 的分布）

- [671e65a2](https://github.com/manticoresoftware/lemmatizer-uk/commit/671e65a296800537123a3f8b324eeaa3f58b9632) - 为 lemmatizer-uk 增加缓存
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
- [systemd 的自定义启动标志](Starting_the_server/Linux.md#Custom-startup-flags-using-systemd)。现在如果你需要使用某些特定启动标志运行 Manticore，不必手动启动 searchd

- 新函数 [LEVENSHTEIN()](Functions/String_functions.md#LEVENSHTEIN%28%29)，用于计算莱文斯坦距离
- 新增 [searchd 启动标志](Starting_the_server/Manually.md#searchd-command-line-options) `--replay-flags=ignore-trx-errors` 和 `--replay-flags=ignore-all-errors`，以便在 binlog 损坏时仍能启动 searchd
- [问题 #621](https://github.com/manticoresoftware/manticoresearch/issues/621) - 公开 RE2 的错误信息
- 对由本地普通索引组成的分布式索引提供更准确的 [COUNT(DISTINCT)](Searching/Grouping.md#COUNT%28DISTINCT-field%29)
- [FACET DISTINCT](Searching/Faceted_search.md#Faceting-without-duplicates) 用于在分面搜索时去重
- [精确形式修饰符](Searching/Full_text_matching/Operators.md#Exact-form-modifier) 现在不再需要 [形态学](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology)，并适用于启用了 [中缀/前缀](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) 搜索的索引
### 重大变更
- 新版本可以读取旧索引，但旧版本无法读取 Manticore 4 的索引

- 移除基于 id 的隐式排序。如需排序请显式指定
- `charset_table` 的默认值从 `0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451` 改为 `non_cjk`
- `OPTIMIZE` 操作自动执行。如果不需要，请确保在配置文件的 `searchd` 部分设置 `auto_optimize=0`
- [问题 #616](https://github.com/manticoresoftware/manticoresearch/issues/616) `ondisk_attrs_default` 已废弃，现在被移除
- 对贡献者：Linux 编译现在使用 Clang 编译器，根据我们的测试，它能构建更快的 Manticore Search 和 Manticore Columnar 库
- 如果搜索查询中未指定 [max_matches](Searching/Options.md#max_matches)，则它会自动更新为满足新列存储性能需求的最小值。这可能影响 [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META) 中的 `total` 指标，但不影响实际找到的文档数 `total_found`
### Manticore 3 迁移
- 确保干净停止 Manticore 3：
  - `/var/lib/manticore/binlog/` 目录内不应有 binlog 文件（目录中仅保留 `binlog.meta`）

  - 否则 Manticore 4 无法重放某些 binlog，且对应索引无法加载
- 新版本可以读取旧索引，但旧版本无法读取 Manticore 4 的索引，若需方便回滚，请务必做好备份
- 如果运行复制集群，请确保：
  - 首先干净关闭所有节点
  - 然后以 `--new-cluster` 启动最后停止的节点（Linux 下运行 `manticore_new_cluster` 工具）
  - 更多细节请阅读 [重启集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster)
### Bug 修复
- 修复了大量复制相关问题：
  - [Commit 696f](https://github.com/manticoresoftware/manticoresearch/commit/696f8649535f2b5285b878c9581c0d2b1139ae09) - 修复了加入节点在 SST 期间遇到活动索引时崩溃；新增加入节点写入文件块时的 sha1 校验以加速索引加载；新增加入节点索引加载时已更改索引文件的轮换；新增替换索引文件时加入节点索引文件的删除；新增捐赠节点发送文件及块的复制日志点
  - [Commit b296](https://github.com/manticoresoftware/manticoresearch/commit/b296c55af68314a8fc66f8b104e7301d673c3b68) - 修正 JOIN CLUSTER 时地址错误导致的崩溃
  - [Commit 418b](https://github.com/manticoresoftware/manticoresearch/commit/418bf880a5e05ffbb68f311a1c23006a9df3220e) - 在大索引的初始复制时，加入节点可能出现 `ERROR 1064 (42000): invalid GTID, (null)` 错误，且捐赠节点在其它节点加入时可能变得无响应
  - [Commit 6fd3](https://github.com/manticoresoftware/manticoresearch/commit/6fd350d28e4905211cf8a7298cbf7cdda101c83d) - 修正了大索引哈希计算错误，导致复制失败
  - [Issue #615](https://github.com/manticoresoftware/manticoresearch/issues/615) - 集群重启时复制失败
- [Issue #574](https://github.com/manticoresoftware/manticoresearch/issues/574) - `indextool --help` 未显示参数 `--rotate`
- [Issue #578](https://github.com/manticoresoftware/manticoresearch/issues/578) - searchd 空闲约一天后出现高 CPU 使用率
- [Issue #587](https://github.com/manticoresoftware/manticoresearch/issues/587) - 立即刷新 .meta 文件
- [问题 #617](https://github.com/manticoresoftware/manticoresearch/issues/617) - manticore.json 被清空
- [问题 #618](https://github.com/manticoresoftware/manticoresearch/issues/618) - searchd --stopwait 在 root 下失败。它还修复了 systemctl 的行为（之前显示 ExecStop 失败，并且没有足够长的时间让 searchd 正确停止）
- [问题 #619](https://github.com/manticoresoftware/manticoresearch/issues/619) - INSERT/REPLACE/DELETE 与 SHOW STATUS。`command_insert`、`command_replace` 和其他显示错误的指标
- [问题 #620](https://github.com/manticoresoftware/manticoresearch/issues/620) - 纯索引的 `charset_table` 有一个错误的默认值
- [提交 8f75](https://github.com/manticoresoftware/manticoresearch/commit/8f7536887d4b58a5bab5647fc067b2d0482c2038) - 新的磁盘块没有被 mlocked
- [问题 #607](https://github.com/manticoresoftware/manticoresearch/issues/607) - 当无法使用名称解析一个节点时，Manticore 集群节点崩溃


- [问题 #623](https://github.com/manticoresoftware/manticoresearch/issues/623) - 更新索引的复制可能导致未定义状态
- [提交 ca03](https://github.com/manticoresoftware/manticoresearch/commit/ca03d2280c5197e9b311f0f03f551e3b702a8130) - 索引器在对具有 json 属性的纯索引源进行索引时可能会卡住

- [提交 53c7](https://github.com/manticoresoftware/manticoresearch/commit/53c753054558ff3f82c7af365273d32537c5338b) - 修复 PQ 索引中的不等表达式过滤器
- [提交 ccf9](https://github.com/manticoresoftware/manticoresearch/commit/ccf94e022ba6763d6286ab5bac767b6f6a846846) - 修复在 1000 个匹配项以上的列表查询中的选择窗口。`SELECT * FROM pq ORDER BY id desc LIMIT 1000 , 100 OPTION max_matches=1100` 之前无法正常工作
- [提交 a048](https://github.com/manticoresoftware/manticoresearch/commit/a0483fe9bef3c45dffe96f5df4f4382aee29c072) - 对 Manticore 的 HTTPS 请求可能导致警告，例如“最大数据包大小（8388608）超出”
- [问题 #648](https://github.com/manticoresoftware/manticoresearch/issues/648) - Manticore 3 在字符串属性经过几次更新后可能会卡住

## 版本 3.6.0，2021 年 5 月 3 日
**Manticore 4 之前的维护版本**
### 主要新特性
- 支持 [Manticore 列库](https://github.com/manticoresoftware/columnar/) 用于纯索引。新的设置 [columnar_attrs](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#columnar_attrs) 用于纯索引
- 支持 [乌克兰形态学分析器](https://github.com/manticoresoftware/lemmatizer-uk)
- 完全修订的直方图。在构建索引时，Manticore 还为其中的每个字段构建直方图，然后用于更快的筛选。在 3.6.0 中，算法经过全面修订，如果您有大量数据并进行大量筛选，可以获得更高的性能。
### 次要更改
- 工具 `manticore_new_cluster [--force]` 用于通过 systemd 重新启动一个复制集群
- `indexer --merge` 的 [--drop-src](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Merging_tables.md#Merging-tables)

- [新模式](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) `blend_mode='trim_all'`
- 添加了 [支持转义 JSON 路径](Searching/Full_text_matching/Escaping.md#Escaping-json-node-names-in-SQL) 的反引号
- [indextool --check](Miscellaneous_tools.md#indextool) 可以在 RT 模式下工作

- [FORCE/IGNORE INDEX(id)](Data_creation_and_modification/Updating_documents/UPDATE.md#Updates-via-SQL) 用于 SELECT/UPDATE
- 现在合并磁盘块的块 ID 是唯一的
- [indextool --check-disk-chunk CHUNK_NAME](Miscellaneous_tools.md#indextool)
### 优化
- [更快的 JSON 解析](https://github.com/manticoresoftware/manticoresearch/commit/0c25fc1788e3f77f60cb7fb8f1623d8466bba606)，我们的测试显示在查询 `WHERE json.a = 1` 上延迟降低了 3-4%
- 非文档化命令 `DEBUG SPLIT` 作为自动分片/重新平衡的前提条件
### 修复
- [问题 #584](https://github.com/manticoresoftware/manticoresearch/issues/524) - 不准确和不稳定的 FACET 结果
- [问题 #506](https://github.com/manticoresoftware/manticoresearch/issues/506) - 使用 MATCH 时的奇怪行为：受到此问题影响的人需要重建索引，因为问题出现在构建索引的阶段
- [问题 #387](https://github.com/manticoresoftware/manticoresearch/issues/387) - 运行带有 SNIPPET() 函数的查询时间歇性核心转储
- 对处理复杂查询有用的堆栈优化：
  - [问题 #469](https://github.com/manticoresoftware/manticoresearch/issues/469) - SELECT 结果导致 CRASH DUMP
  - [e8420cc7](https://github.com/manticoresoftware/manticoresearch/tree/e8420cc789149c84731be0874be916e33b263bd6) - 过滤器树的堆栈大小检测
- [问题 #461](https://github.com/manticoresoftware/manticoresearch/issues/461) - 使用 IN 条件的更新未能正确生效
- [问题 #464](https://github.com/manticoresoftware/manticoresearch/issues/464) - 在 CALL PQ 后立即 SHOW STATUS 返回 - [问题 #481](https://github.com/manticoresoftware/manticoresearch/issues/481) - 修复静态二进制构建
- [问题 #502](https://github.com/manticoresoftware/manticoresearch/issues/502) - 多查询中的 bug
- [问题 #514](https://github.com/manticoresoftware/manticoresearch/issues/514) - 使用 'create table' 时无法使用不寻常的列名
- [提交 d1db](https://github.com/manticoresoftware/manticoresearch/commit/d1dbe771288328c963ca8512f564c6b7cf4f466c) - 在更新字符串属性的 binlog 重放时守护进程崩溃；将 binlog 版本设置为 10
- [提交 775d](https://github.com/manticoresoftware/manticoresearch/commit/775d0555562f121911d3c30085947e743fadac2d)  - 修复表达式堆栈帧检测运行时（测试 207）
- [提交 4795](https://github.com/manticoresoftware/manticoresearch/commit/4795dc49194c7745761bb5006ecb266e8a9224fa)  - 过滤器和标签在空存储查询时为空（测试 369）
- [Commit c3f0](https://github.com/manticoresoftware/manticoresearch/commit/c3f0bf4dca0e4eae6ea899dd566878df57eba085)  - 网络延迟高且错误率高时复制 SST 流中断（不同数据中心复制）；更新复制命令版本为 1.03
- [Commit ba2d](https://github.com/manticoresoftware/manticoresearch/commit/ba2d6619b8107f3fb177741bbdff3709bac248cd)  - 聚合器在加入集群后对写操作锁定集群（测试 385）
- [Commit de4d](https://github.com/manticoresoftware/manticoresearch/commit/de4dcb9ff6c316bc2a9ef6c62eb64769f04ee103)  - 具有精确修饰符的通配符匹配（测试 321）
- [Commit 6524](https://github.com/manticoresoftware/manticoresearch/commit/6524fc6af032a0932bd42c37874a619e1fb7f075)  - docid 检查点与 docstore
- [Commit f4ab](https://github.com/manticoresoftware/manticoresearch/commit/f4ab83c291e65a5873d271c67b5491f629544f1d)  - 解析无效 xml 时索引器行为不一致
- [Commit 7b72](https://github.com/manticoresoftware/manticoresearch/commit/7b727e22fe01b1a180d4a8d3a3fea92c18eac707)  - 带有 NOTNEAR 的存储垂询查询运行时间过长（测试 349）
- [Commit 812d](https://github.com/manticoresoftware/manticoresearch/commit/812dab745f0c972ecde5d5b3d4a313cd6e1e143e)  - 以通配符开始的短语权重错误
- [Commit 1771](https://github.com/manticoresoftware/manticoresearch/commit/1771afc669c75670b3511bf02ab9bfc8982b94d2)  - 带通配符的垂询查询生成匹配时无有效载荷的词项导致命中交错且匹配失败（测试 417）
- [Commit aa0d](https://github.com/manticoresoftware/manticoresearch/commit/aa0d8c2bcb41428d41dbfb66e42a872a79a4ee4b)  - 修正并行查询情况下 'total' 的计算
- [Commit 18d8](https://github.com/manticoresoftware/manticoresearch/commit/18d81b3cfbb930b7361dae645fe13283d4fbf6ef)  - Windows 下守护进程多并发会话崩溃
- [Commit 8443](https://github.com/manticoresoftware/manticoresearch/commit/84432f23d6fbf7a7d0b67c8c9412eb5933b03596)  - 某些索引设置无法复制
- [Commit 9341](https://github.com/manticoresoftware/manticoresearch/commit/93411fe6af4246ca4acce758944d513a1660b92b)  - 在高新增事件速率下，netloop 有时冻结，原因是原子 'kick' 事件一次处理多事件且丢失实际动作
查询状态，而非服务器状态
- [Commit d805](https://github.com/manticoresoftware/manticoresearch/commit/d805fc125244ef08d14fb3d1826e2c356862d539)  - 新刷写的磁盘块可能在提交时丢失

- [Commit 63cb](https://github.com/manticoresoftware/manticoresearch/commit/63cbf008d50979cdf7062664fb02a672a3bb1131)  - profiler 中 'net_read' 不准确
- [Commit f537](https://github.com/manticoresoftware/manticoresearch/commit/f5379bb27234b25919966c50ccb6f469ead646e2)  - 带阿拉伯语（从右向左文本）的垂询问题
- [Commit 49ee](https://github.com/manticoresoftware/manticoresearch/commit/49eeb4202d4ce4875c07c35087d7108c1e177328)  - 重复列名时 id 未正确选取
- [Commit refa](https://github.com/manticoresoftware/manticoresearch/commit/4b7df262bbfec9b9e95a44e72ad326ac8a786c49) 修复罕见情况下网络事件崩溃
- [e8420cc7](https://github.com/manticoresoftware/manticoresearch/tree/e8420cc789149c84731be0874be916e33b263bd6) 修正 `indextool --dumpheader` 中的错误
- [Commit ff71](https://github.com/manticoresoftware/manticoresearch/commit/ff7163532ed39042c0d3c3c2b902ff41f94d7f0c) - 用带重新配置的 TRUNCATE 处理存储字段错误

### 重大更改：
- 新的 binlog 格式：升级前需干净停止 Manticore
- 索引格式略有变化：新版本可正常读取现有索引，但如果从 3.6.0 降级至旧版本，新索引将无法读取

- 复制格式改变：不要从旧版本复制到 3.6.0 或反向，需所有节点同时切换到新版本

- `reverse_scan` 已弃用。从 3.6.0 起确保查询不使用此选项，否则会失败
- 从此版本起不再提供 RHEL6、Debian Jessie 和 Ubuntu Trusty 的构建版本。如需关键支持，[联系我们](https://manticoresearch.com/contact-us/)
### 弃用内容
- 不再隐式按 id 排序。如依赖此功能，请相应更新查询

- 搜索选项 `reverse_scan` 已弃用
## 版本 3.5.4，2020 年 12 月 10 日
### 新功能

- 全新 Python、Javascript 和 Java 客户端现已正式发布，并在本手册中有详细文档
- 实时索引的磁盘块自动丢弃。此优化允许在优化实时索引时自动丢弃明显不再需要的磁盘块（所有文档均已删除）。之前仍需合并，现在可即时丢弃该块。将忽略 [cutoff](Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks) 选项，即使未实际合并，过时的磁盘块仍会被移除。适用于索引保留策略及删除旧文档，压缩此类索引更快。
- 作为 SELECT 选项支持独立的 NOT [standalone NOT](Searching/Options.md#not_terms_only_allowed)

### 次要更改
- [Issue #453](https://github.com/manticoresoftware/manticoresearch/issues/453) 新选项 [indexer.ignore_non_plain=1](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#ignore_non_plain) 在运行 `indexer --all` 且配置文件包含非纯索引时非常有用。未设置 `ignore_non_plain=1` 会产生警告及对应退出代码。
- [SHOW PLAN ... OPTION format=dot](Node_info_and_management/Profiling/Query_plan.md#Dot-format-for-SHOW-PLAN) 和 [EXPLAIN QUERY ... OPTION format=dot](Searching/Full_text_matching/Profiling.md#Profiling-without-running-a-query) 支持全文本查询计划执行的可视化。对于理解复杂查询非常有用。
### 弃用项
- `indexer --verbose` 已弃用，因为它从未为索引器输出添加任何内容
- 用于转储 watchdog 的回溯信号现在应使用 `USR2`，而不是 `USR1`
### Bug 修复
- [Issue #423](https://github.com/manticoresoftware/manticoresearch/issues/423) 西里尔字母句点调用 snippets retain 模式不高亮
- [Issue #435](https://github.com/manticoresoftware/manticoresearch/issues/435) RTINDEX - GROUP N BY 表达式选择 = 致命崩溃
- [Commit 2b3b](https://github.com/manticoresoftware/manticoresearch/commit/2b3b62bd0ce2b54544733d524a169e81f71b5388) 当在集群中时，searchd 状态显示段错误


- [Commit 9dd2](https://github.com/manticoresoftware/manticoresearch/commit/9dd25c193ed2a53750a0e0e68566fd2b833fcb0c) 'SHOW INDEX index.N SETTINGS' 不支持 chunks >9

- [Issue #389](https://github.com/manticoresoftware/manticoresearch/issues/389) 导致 Manticore 崩溃的 Bug

- [Commit fba1](https://github.com/manticoresoftware/manticoresearch/commit/fba166178ff35c73a0c70d131cf3aca4a635e961) 转换器创建的索引损坏
- [Commit eecd](https://github.com/manticoresoftware/manticoresearch/commit/eecd61d861c742471175be19a5e5455034b5db5f) stopword_step=0 与 CALL SNIPPETS()
- [Commit ea68](https://github.com/manticoresoftware/manticoresearch/commit/ea6850e449601199190f38a387c04b8a7b7e90f6) 在本地索引上最大匹配数低时 count distinct 返回 0
- [Commit 362f](https://github.com/manticoresoftware/manticoresearch/commit/362f27db3421f09a41c6cce04b5baace00fa4c4d) 使用聚合时存储的文本不会在命中中返回
## 版本 3.5.2，2020年10月1日
### 新功能

* OPTIMIZE 将磁盘块减少到若干个块（默认是 `2* 核心数`），而非单个块。最佳的块数可以通过 [cutoff](Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks) 选项控制。

* NOT 运算符现在可单独使用。默认被禁用，因为意外的单 NOT 查询可能很慢。可以通过设置新的 searchd 指令 [not_terms_only_allowed](Server_settings/Searchd.md#not_terms_only_allowed) 为 `0` 启用。
* 新设置 [max_threads_per_query](Server_settings/Searchd.md#max_threads_per_query) 设置查询可使用的最大线程数。如果未设置该指令，查询可使用的线程数将达到 [threads](Server_settings/Searchd.md#threads) 的值。
每个 `SELECT` 查询可以通过 [OPTION threads=N](Searching/Options.md#threads) 覆盖全局 `max_threads_per_query` 来限制线程数。
* Percolate 索引现在可以通过 [IMPORT TABLE](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Importing_table.md) 导入。

* HTTP API `/search` 通过新的查询节点 `aggs` 获得了基本的 [分面搜索](Searching/Faceted_search.md#HTTP-JSON) / [分组](Searching/Grouping.md) 支持。

### 小改动

* 如果未声明复制监听指令，引擎将尝试使用定义的 'sphinx' 端口之后的端口，最多到 200。

* `listen=...:sphinx` 需要明确设置以支持 SphinxSE 连接或 SphinxAPI 客户端。

* [SHOW INDEX STATUS](Node_info_and_management/Table_settings_and_status/SHOW_TABLE_STATUS.md) 输出新增指标：`killed_documents`、`killed_rate`、`disk_mapped_doclists`、`disk_mapped_cached_doclists`、`disk_mapped_hitlists` 以及 `disk_mapped_cached_hitlists`。

* SQL 命令 `status` 现在输出 `Queue\Threads` 和 `Tasks\Threads`。

### 弃用：
* `dist_threads` 现已完全弃用，如果仍使用该指令，searchd 会记录警告。
### Docker
官方 Docker 镜像现在基于 Ubuntu 20.04 LTS
### 打包
除常规的 `manticore` 包外，还可以按组件安装 Manticore Search：

- `manticore-server-core` - 提供 `searchd`，手册页，日志目录，API 和 galera 模块。同时作为依赖安装 `manticore-common`。

- `manticore-server` - 提供核心自动化脚本（init.d，systemd）以及 `manticore_new_cluster` 包装器。同时作为依赖安装 `manticore-server-core`。
- `manticore-common` - 提供配置、停用词、通用文档和骨架文件夹（数据目录、模块等）。
- `manticore-tools` - 提供辅助工具（`indexer`，`indextool` 等），它们的手册页和示例。同时作为依赖安装 `manticore-common`。
- `manticore-icudata`（RPM）或 `manticore-icudata-65l`（DEB） - 提供 ICU 数据文件，用于 icu 形态学功能。
- `manticore-devel`（RPM）或 `manticore-dev`（DEB） - 提供 UDF 的开发头文件。
### Bug 修复
1. [Commit 2a47](https://github.com/manticoresoftware/manticoresearch/commit/2a474dc1a26e8b0f8aaaae95669caf2f1d4b7746) RT 索引不同块的 grouper 守护进程崩溃
2. [Commit 57a1](https://github.com/manticoresoftware/manticoresearch/commit/57a19e5ad5663ef0ca7436595218fb1221d28c8e) 空远程文档的快速路径
3. [Commit 07dd](https://github.com/manticoresoftware/manticoresearch/commit/07dd3f313c63fb82c22092f9907ef24e3475250e) 表达式堆栈帧检测运行时
4. [Commit 08ae](https://github.com/manticoresoftware/manticoresearch/commit/08ae357cf1012bc8e2da54c20b205b592efda3d4) Percolate 索引支持超过 32 字段的匹配
5. [Commit 16b9](https://github.com/manticoresoftware/manticoresearch/commit/16b9390fd4cdb07a77ac4497adb935573a1710e5) 复制监听端口范围
6. [Commit 5fa6](https://github.com/manticoresoftware/manticoresearch/commit/5fa671affeacb6441a59c8a88479bfd423df7c81) PQ 上显示创建表
7. [Commit 54d1](https://github.com/manticoresoftware/manticoresearch/commit/54d133b6449105a9fb0168db3f1fbb05fb5aa1f6) HTTPS 端口行为
8. [提交 fdbb](https://github.com/manticoresoftware/manticoresearch/commit/fdbbe5245cc296cc5c1ae3ae2fb9cb08fb66a248) 替换时混合文档存储行
9. [提交 afb5](https://github.com/manticoresoftware/manticoresearch/commit/afb53f648ef4e64fa1776c58e66f6e716ac730ad) 将 TFO 不可用消息级别切换为 'info'
10. [提交 59d9](https://github.com/manticoresoftware/manticoresearch/commit/59d94cefc6e88af360b8046726a16b7eaa9f0b71) strcmp 非法使用导致崩溃
11. [提交 04af](https://github.com/manticoresoftware/manticoresearch/commit/04af0349c1e784c2230d204592d6272b440bb375) 向带系统（停用词）文件的集群添加索引
12. [提交 5014](https://github.com/manticoresoftware/manticoresearch/commit/50148b4eae9f3a924082005cf5d2f9c8339af7f5) 合并带大型词典的索引；RT 优化大磁盘块
13. [提交 a2ad](https://github.com/manticoresoftware/manticoresearch/commit/a2adf1582843f63d4dcc1dab8f7598e0917094a2) Indextool 可以从当前版本转储元数据

14. [提交 69f6](https://github.com/manticoresoftware/manticoresearch/commit/69f6d5f77c64b97e3f395aea8c555b11c77fb732) GROUP N 中的组顺序问题

15. [提交 24d5](https://github.com/manticoresoftware/manticoresearch/commit/24d5d80f508576794b751d70f9f9b524564e3c14) 握手后 SphinxSE 的显式刷新
16. [提交 31c4](https://github.com/manticoresoftware/manticoresearch/commit/31c4d78ac03ee85d9a02b86075448a04ac80f9e2) 避免不必要时复制巨大描述
17. [提交 2959](https://github.com/manticoresoftware/manticoresearch/commit/2959e2caa8610f98fcdf2e5a82160f58687f4a27) show threads 中的负时间
18. [提交 f0b3](https://github.com/manticoresoftware/manticoresearch/commit/f0b357102c70f3ac1d7da260c60b67322940e647) 令牌过滤插件与零位置增量
19. [提交 a49e](https://github.com/manticoresoftware/manticoresearch/commit/a49e5bc13ea763e6f01dd5993e891858a84d1846) 多次命中时将 'FAIL' 改为 'WARNING'
## 版本 3.5.0，2020年7月22日
### 主要新特性：

* 此版本耗时较长，因为我们一直在努力将多任务模式从线程改为**协程**。这使得配置更简单，查询并行化更加直接：Manticore 只使用给定数量的线程（见新设置[threads](Server_settings/Searchd.md#threads)），新模式确保以最优方式完成。

* [高亮显示](Searching/Highlighting.md#Highlighting-options)的变化：

  - 任何针对多个字段的高亮（`highlight({},'field1, field2'`）或 JSON 查询中的 `highlight`），现在默认按字段应用限制。

  - 任何针对纯文本的高亮（`highlight({}, string_attr)` 或 `snippet()`）现在对整个文档应用限制。
  - [每字段限制](Searching/Highlighting.md#limits_per_field) 可以通过 `limits_per_field=0` 选项切换为全局限制（默认为 `1`）。
  - 通过 HTTP JSON 进行高亮时，[allow_empty](Searching/Highlighting.md#allow_empty) 现默认值为 `0`。
* 同一端口现在[可用于](Server_settings/Searchd.md#listen) http、https 和二进制 API（用于接受远程 Manticore 实例的连接）。通过 mysql 协议连接仍需使用 `listen = *:mysql`。Manticore 现在能自动识别尝试连接的客户端类型，MySQL 除外（因协议限制）。
* 在 RT 模式下，字段现可同时为[文本和字符串属性](Creating_a_table/Data_types.md#String) — [GitHub 问题 #331](https://github.com/manticoresoftware/manticoresearch/issues/331)。
  在[普通模式](Read_this_first.md#Real-time-mode-vs-plain-mode)中称为 `sql_field_string`。现在也适用于实时索引的[RT 模式](Read_this_first.md#Real-time-mode-vs-plain-mode)。示例如下：
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
### 小改动
* 现可[高亮字符串属性](Searching/Highlighting.md#Highlighting-via-SQL)。
* SQL 接口支持 SSL 和压缩。
* 支持 mysql 客户端的 [`status`](Node_info_and_management/Node_status.md#STATUS) 命令。
* [复制](Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Setting-up-replication)现可复制外部文件（停用词、异常词等）。
* 过滤操作符 [`in`](Searching/Filters.md#Set-filters) 现可通过 HTTP JSON 接口使用。
* HTTP JSON 中的 [`expressions`](Searching/Expressions.md#expressions)。
* 现可在 RT 模式下[动态修改 `rt_mem_limit`](https://github.com/manticoresoftware/manticoresearch/issues/344)，即可以执行 `ALTER ... rt_mem_limit=<新值>`。

* 现可使用[独立的 CJK 字符集表](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology)：`chinese`、`japanese` 和 `korean`。
* [thread_stack](Server_settings/Searchd.md#thread_stack) 现在限制最大线程栈，而非初始大小。
* 改进了 `SHOW THREADS` 输出。
* 显示长时间 `CALL PQ` 的进度于 `SHOW THREADS` 中。
* cpustat、iostat、coredump 可通过[SET](Server_settings/Setting_variables_online.md#SET) 运行时修改。

* 实现了 `SET [GLOBAL] wait_timeout=NUM`。
### 破坏性变更：
* **索引格式已更改。** 3.5.0 构建的索引无法被低于 3.5.0 版本的 Manticore 加载，但 Manticore 3.5.0 可识别旧格式。
* [`INSERT INTO PQ VALUES()`](Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md)（即不提供列列表）之前预期的值正好是 `(query, tags)`。现已更改为 `(id,query,tags,filters)`。如果希望自动生成 id，可以将 id 设置为 0。
* [`allow_empty=0`](Searching/Highlighting.md#allow_empty) 是通过 HTTP JSON 接口高亮显示的新默认值。
* 在 `CREATE TABLE`/`ALTER TABLE` 中，外部文件（停用词、例外等）只允许使用绝对路径。
### 弃用:
* `ram_chunks_count` 在 `SHOW INDEX STATUS` 中重命名为 `ram_chunk_segments_count`。
* `workers` 已过时。目前只有一种 workers 模式。
* `dist_threads` 已过时。现在所有查询都尽可能并行（受 `threads` 和 `jobs_queue_size` 限制）。
* `max_children` 已过时。使用 [threads](Server_settings/Searchd.md#threads) 设置 Manticore 将使用的线程数（默认设置为 CPU 核心数）。
* `queue_max_length` 已过时。如果确实需要，请使用 [jobs_queue_size](Server_settings/Searchd.md#jobs_queue_size) 调整内部作业队列大小（默认无限制）。
* 所有 `/json/*` 端点现在可无 `/json/` 前缀访问，例如 `/search`、`/insert`、`/delete`、`/pq` 等。
* 在 `describe` 中，“field”表示“全文字段”的含义重命名为“text”。
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
* 西里尔字母 `и` 在 `non_cjk` charset_table（默认值）中不再映射为 `i`，因为这对俄语词干提取器和词形还原器影响太大。
* `read_timeout`。请改用 [network_timeout](Server_settings/Searchd.md#network_timeout)，它控制读取和写入。
### 软件包
* Ubuntu Focal 20.04 官方包
* deb 包名称由 `manticore-bin` 改为 `manticore`
### 缺陷修复:
1. [Issue #351](https://github.com/manticoresoftware/manticoresearch/issues/351) searchd 内存泄漏
2. [Commit ceab](https://github.com/manticoresoftware/manticoresearch/commit/ceabe44f) 片段中小范围越界读取
3. [Commit 1c3e](https://github.com/manticoresoftware/manticoresearch/commit/1c3e84a3) 危险的本地变量写入导致崩溃的查询
4. [Commit 26e0](https://github.com/manticoresoftware/manticoresearch/commit/26e094ab) 测试 226 中排序器的小内存泄漏
5. [Commit d2c7](https://github.com/manticoresoftware/manticoresearch/commit/d2c7f86a) 测试 226 中的大量内存泄漏
6. [Commit 0dd8](https://github.com/manticoresoftware/manticoresearch/commit/0dd80122) 集群显示节点同步但 `count(*)` 显示不同数字
7. [Commit f1c1](https://github.com/manticoresoftware/manticoresearch/commit/f1c1ac3f) 外观：日志中重复且有时丢失的警告信息
8. [Commit f1c1](https://github.com/manticoresoftware/manticoresearch/commit/f1c1ac3f) 外观：日志中的 (null) 索引名称
9. [Commit 359d](https://github.com/manticoresoftware/manticoresearch/commit/359dbd30) 无法检索超过 7000 万条结果
10. [Commit 19f3](https://github.com/manticoresoftware/manticoresearch/commit/19f328ee) 无法插入无列语法的 PQ 规则
11. [Commit bf68](https://github.com/manticoresoftware/manticoresearch/commit/bf685d5d) 向集群中索引插入文档时误导性错误信息
12. [Commit 2cf1](https://github.com/manticoresoftware/manticoresearch/commit/2cf18c83) `/json/replace` 和 `json/update` 返回的 id 使用指数形式
13. [Issue #324](https://github.com/manticoresoftware/manticoresearch/issues/324) 在同一查询中更新 json 标量属性和 mva
14. [Commit d384](https://github.com/manticoresoftware/manticoresearch/commit/d38409eb) `hitless_words` 在 RT 模式下无效
15. [Commit 5813](https://github.com/manticoresoftware/manticoresearch/commit/5813d639) RT 模式下应禁止 `ALTER RECONFIGURE`
16. [Commit 5813](https://github.com/manticoresoftware/manticoresearch/commit/5813d639) searchd 重启后 `rt_mem_limit` 重置为 128M
17. highlight() 有时会挂起
18. [Commit 7cd8](https://github.com/manticoresoftware/manticoresearch/commit/7cd878f4) RT 模式下无法使用 U+ 代码
19. [Commit 2b21](https://github.com/manticoresoftware/manticoresearch/commit/2b213de4) RT 模式下无法在 wordforms 使用通配符
20. [Commit e9d0](https://github.com/manticoresoftware/manticoresearch/commit/e9d07e68) 修复 `SHOW CREATE TABLE` 处理多个 wordform 文件问题
21. [Commit fc90](https://github.com/manticoresoftware/manticoresearch/commit/fc90a84f) JSON 查询无 "query" 字段会导致 searchd 崩溃
22. Manticore [官方 docker](https://hub.docker.com/r/manticoresearch/manticore) 无法从 mysql 8 进行索引
23. [Commit 23e0](https://github.com/manticoresoftware/manticoresearch/commit/23e05d32) HTTP /json/insert 需要 id
24. [Commit bd67](https://github.com/manticoresoftware/manticoresearch/commit/bd679af0) `SHOW CREATE TABLE` 不适用于 PQ
25. [Commit bd67](https://github.com/manticoresoftware/manticoresearch/commit/bd679af0) `CREATE TABLE LIKE` 在 PQ 方面工作不正常
26. [Commit 5eac](https://github.com/manticoresoftware/manticoresearch/commit/5eacf28f) `show index status` 设置中行尾问题
27. [Commit cb15](https://github.com/manticoresoftware/manticoresearch/commit/cb153228) HTTP JSON 响应中 "highlight" 的标题为空
28. [Issue #318](https://github.com/manticoresoftware/manticoresearch/issues/318) `CREATE TABLE LIKE` 中的中缀错误
29. [提交 9040](https://github.com/manticoresoftware/manticoresearch/commit/9040d22c) RT 在高负载下崩溃
30. [cd512c7d ](https://github.com/manticoresoftware/manticoresearch/commit/cd512c7d ) RT 磁盘块崩溃时丢失崩溃日志
31. [问题 #323](https://github.com/manticoresoftware/manticoresearch/issues/323) 导入表失败并关闭连接
32. [提交 6275](https://github.com/manticoresoftware/manticoresearch/commit/6275316a) `ALTER reconfigure` 破坏 PQ 索引
33. [提交 9c1d](https://github.com/manticoresoftware/manticoresearch/commit/9c1d221e) 更改索引类型后 searchd 重载问题
34. [提交 71e2](https://github.com/manticoresoftware/manticoresearch/commit/71e2b5bb) 守护进程在导入表时因缺失文件崩溃
35. [问题 #322](https://github.com/manticoresoftware/manticoresearch/issues/322) 使用多个索引、group by 和 ranker = none 查询时崩溃
36. [提交 c3f5](https://github.com/manticoresoftware/manticoresearch/commit/c3f58490) `HIGHLIGHT()` 不在字符串属性中高亮
37. [问题 #320](https://github.com/manticoresoftware/manticoresearch/issues/320) `FACET` 无法对字符串属性排序
38. [提交 4f1a](https://github.com/manticoresoftware/manticoresearch/commit/4f1a1f25) 缺失数据目录时出错
39. [提交 04f4](https://github.com/manticoresoftware/manticoresearch/commit/04f4ddd4) access_* 在 RT 模式下不支持
40. [提交 1c06](https://github.com/manticoresoftware/manticoresearch/commit/1c0616a2) 字符串中的坏 JSON 对象：1. `CALL PQ` 当 json 超过某个值时返回 "Bad JSON objects in strings: 1"。
41. [提交 32f9](https://github.com/manticoresoftware/manticoresearch/commit/32f943d6) RT 模式不一致。在某些情况下无法删除索引（未知），也无法创建，因为目录不为空。
42. [问题 #319](https://github.com/manticoresoftware/manticoresearch/issues/319) 查询时崩溃
43. [提交 22a2](https://github.com/manticoresoftware/manticoresearch/commit/22a28dd7) `max_xmlpipe2_field` = 2M 在 2M 字段上返回警告
44. [问题 #342](https://github.com/manticoresoftware/manticoresearch/issues/342) 查询条件执行错误
45. [提交 dd8d](https://github.com/manticoresoftware/manticoresearch/commit/dd8dcab2) 仅包含一个词的文档被简单两词搜索命中
46. [提交 9091](https://github.com/manticoresoftware/manticoresearch/commit/90919e62) PQ 中无法匹配键含大写字母的 json
47. [提交 56da](https://github.com/manticoresoftware/manticoresearch/commit/56da086a) 索引程序在 csv+docstore 时崩溃
48. [问题 #363](https://github.com/manticoresoftware/manticoresearch/issues/363) centos 7 中 json 属性使用 `[null]` 导致插入数据损坏
49. 重大 [问题 #345](https://github.com/manticoresoftware/manticoresearch/issues/345) 记录未被插入，count() 随机，"replace into" 返回 OK
50. max_query_time 过度减慢 SELECT 查询速度


51. [问题 #352](https://github.com/manticoresoftware/manticoresearch/issues/352) Mac OS 上主从代理通信失败
52. [问题 #328](https://github.com/manticoresoftware/manticoresearch/issues/328) 使用 Connector.Net/Mysql 8.0.19 连接 Manticore 时出错
53. [提交 daa7](https://github.com/manticoresoftware/manticoresearch/commit/daa760d2fe83668038318f0f384ceefc0d075e18) 修复 \0 的转义并优化性能

54. [提交 9bc5](https://github.com/manticoresoftware/manticoresearch/commit/9bc5c01a493db993b90ff6c1ad51aeae028f629c) 修复 count distinct 与 json 的问题
55. [提交 4f89](https://github.com/manticoresoftware/manticoresearch/commit/4f89a965a3af0db548ae9d1763731ebaebe21f5a) 修复其他节点删除表失败
56. [提交 952a](https://github.com/manticoresoftware/manticoresearch/commit/952af5a5931fd72b87edc169588777778b365231) 修复紧密调用 pq 时崩溃
## 版本 3.4.2，2020 年 4 月 10 日
### 关键修复
* [提交 2ffe](https://github.com/manticoresoftware/manticoresearch/commit/2ffe2d261793f9681265bc4abe38d695e72c121e) 修复旧版本 RT 索引无法索引数据

## 版本 3.4.0，2020 年 3 月 26 日
### 重大变更
* 服务器支持两种模式：rt-mode 和 plain-mode
   * rt-mode 需要 data_dir，配置中不定义索引
   * plain-mode 索引在配置中定义，不允许 data_dir
* 复制仅在 rt-mode 有效

### 小改动
* charset_table 默认为 non_cjk 别名
* rt-mode 下全文字段默认被索引和存储

* rt-mode 中全文字段名称由 'field' 改为 'text'
* ALTER RTINDEX 改名为 ALTER TABLE
* TRUNCATE RTINDEX 改名为 TRUNCATE TABLE
### 新功能
* 仅存储字段

* SHOW CREATE TABLE，IMPORT TABLE
### 改进
* PQ 无锁机制大大加速
* /sql 在 mode=raw 下可执行任意类型的 SQL 语句
* mysql41 协议的别名 mysql
* data_dir 中默认 state.sql
### Bug 修复
* [提交 a533](https://github.com/manticoresoftware/manticoresearch/commit/a5333644e8e0c69746a9f24da8221fc965478e4a) 修复 highlight() 中错误字段语法导致崩溃
* [提交 7fbb](https://github.com/manticoresoftware/manticoresearch/commit/7fbb9f2e618096289cba7f8b834530b46aa94055) 修复含 docstore 的 RT 索引复制导致服务器崩溃
* [提交 24a0](https://github.com/manticoresoftware/manticoresearch/commit/24a04687eb43b4f4719c7c4a5fe3c19099240467) 修复给启用 infix 或 prefix 选项且无存储字段索引的 highlight 导致崩溃
* [提交 3465](https://github.com/manticoresoftware/manticoresearch/commit/3465c1ce9c860c09f6ccc5b83a63e3c68c241d8d) 修复空索引假报空 docstore 和 doc-id 查询错误
* [提交 a707](https://github.com/manticoresoftware/manticoresearch/commit/b8601b4144b9092be43a7caa7cd6b19cb37e14c7) 修复 #314 SQL 插入命令末尾分号问题
* [提交 9562](https://github.com/manticoresoftware/manticoresearch/commit/95628c9b9b8f16fe1bd18599a84e9673575adf1a) 移除查询词不匹配的警告

* [Commit b860](https://github.com/manticoresoftware/manticoresearch/commit/b8601b4144b9092be43a7caa7cd6b19cb37e14c7) 修复通过 ICU 分词的片段查询
* [Commit 5275](https://github.com/manticoresoftware/manticoresearch/commit/5275516ca4631c584e6e0127fa161a18d2658fd0) 修复 docstore 块缓存中的查找/添加竞态条件
* [Commit f06e](https://github.com/manticoresoftware/manticoresearch/commit/f06ef97af5c7390427e82525c150535290e42c3e) 修复 docstore 中的内存泄漏
* [Commit a725](https://github.com/manticoresoftware/manticoresearch/commit/a7258ba8340e246c0d8aaeb4a7a74129eba807eb) 修复 #316 INSERT 时 LAST_INSERT_ID 返回空值
* [Commit 1ebd](https://github.com/manticoresoftware/manticoresearch/commit/1ebd5bf8d74113bcfa138ce63ce11966990a4215) 修复 #317 json/update HTTP 端点以支持 MVA 数组和 JSON 属性的对象
* [Commit e426](https://github.com/manticoresoftware/manticoresearch/commit/e426950a536382a142e2a42ed207c341149ff685) 修复索引器转储未指定 ID 的 rt 的问题
## 版本 3.3.0，2020 年 2 月 4 日

### 特性
* 并行实时索引搜索
* EXPLAIN QUERY 命令
* 无索引定义的配置文件（测试版）
* CREATE/DROP TABLE 命令（测试版）
* indexer --print-rt - 可以从数据源读取并打印实时索引的 INSERT 语句
### 改进


* 更新到 Snowball 2.0 词干提取器
* SHOW INDEX STATUS 支持 LIKE 过滤
* 针对高 max_matches 的内存使用优化
* SHOW INDEX STATUS 为 RT 索引增加 ram_chunks_count 字段
* 无锁优先队列
* 将 LimitNOFILE 修改为 65536
### 修复
* [Commit 9c33](https://github.com/manticoresoftware/manticoresearch/commit/9c33aab8c1fd9059c8644d23d5f58f03010043c6) 增加索引架构中重复属性的检查 #293
* [Commit a008](https://github.com/manticoresoftware/manticoresearch/commit/a0085f9477e191da9e0e835b91583fb9cfde698e) 修复 hitless terms 引起的崩溃
* [Commit 6895](https://github.com/manticoresoftware/manticoresearch/commit/68953740af8dca1833026d0f448c71416ed3044a) 修复 ATTACH 后松散的 docstore
* [Commit d6f6](https://github.com/manticoresoftware/manticoresearch/commit/d6f696edea53ea2b9a68251e4054e4816082c285) 修复分布式设置中的 docstore 问题
* [Commit bce2](https://github.com/manticoresoftware/manticoresearch/commit/bce2b7ec883d208f31094ee42a5b6f01d18a998f) 将 sorter 中的 FixedHash 替换为 OpenHash
* [Commit e0ba](https://github.com/manticoresoftware/manticoresearch/commit/e0baf7392937969e640fb5d10e8ccb312fe36446) 修复索引定义中重复属性名的问题
* [Commit ca81](https://github.com/manticoresoftware/manticoresearch/commit/ca81114bf1ac7ea7adcc8509bdc14d1f30b4f461) 修复 HIGHLIGHT() 中的 html_strip
* [Commit 493a](https://github.com/manticoresoftware/manticoresearch/commit/493a5e916d70fd4a3f14d1ee487234434f4a7ce3) 修复 HIGHLIGHT() 中的 passage 宏
* [Commit a82d](https://github.com/manticoresoftware/manticoresearch/commit/a82d41c775692f837f267d554d7d4a4b499d69f0) 修复 RT 索引创建小或大磁盘块时的双缓冲问题
* [Commit a404](https://github.com/manticoresoftware/manticoresearch/commit/a404c85de0d41d9d1c75b7540479d8a3af9e61aa) 修复 kqueue 事件删除问题

* [Commit 8bea](https://github.com/manticoresoftware/manticoresearch/commit/8bea0f6fd1666b817995c4dd6e67f6f1c9161638) 修复 RT 索引 rt_mem_limit 大值时磁盘块保存问题
* [Commit 8707](https://github.com/manticoresoftware/manticoresearch/commit/8707f0395b7ace52dc0cba9c8ed92a2bc4392932) 修复索引中的浮点溢出问题
* [Commit a564](https://github.com/manticoresoftware/manticoresearch/commit/a56434ce71bb7714b4d8e75278aaa075b27ece80) 修复插入负 ID 文档到 RT 索引时的错误
* [Commit bbeb](https://github.com/manticoresoftware/manticoresearch/commit/bbebfd7551a68641154761cac962053b54806759) 修复服务器 ranker fieldmask 崩溃
* [Commit 3809](https://github.com/manticoresoftware/manticoresearch/commit/3809cc1b0863e3a5a200dc423be3f7c38be56113) 修复使用查询缓存时的崩溃
* [Commit dc2a](https://github.com/manticoresoftware/manticoresearch/commit/dc2a585bc724678679e4297362200bbd49ce32bb) 修复使用并行插入的 RT 索引 RAM 段崩溃

## 版本 3.2.2，2019 年 12 月 19 日
### 特性
* RT 索引的自增 ID
* 通过新的 HIGHLIGHT() 函数支持 docstore 高亮，也可通过 HTTP API 使用
* SNIPPET() 支持使用特殊函数 QUERY()，返回当前 MATCH 查询
* 为高亮函数新增 field_separator 选项
### 改进和更改
* 远程节点存储字段的惰性获取（显著提升性能）
* 字符串和表达式不再破坏多查询和 FACET 优化
* RHEL/CentOS 8 构建改用 mariadb-connector-c-devel 提供的 mysql libclient

* ICU 数据文件随软件包一同提供，移除 icu_data_dir
* systemd 服务文件包含 'Restart=on-failure' 策略
* indextool 现在可在线检查实时索引
* 默认配置文件更改为 /etc/manticoresearch/manticore.conf
* RHEL/CentOS 中服务名从 'searchd' 改为 'manticore'
* 移除 query_mode 和 exact_phrase 片段选项
### 修复
* [Commit 6ae4](https://github.com/manticoresoftware/manticoresearch/commit/6ae474c7894a6bee222d5b18e59a44fdbf57843a) 修复 HTTP 接口上的 SELECT 查询崩溃
* [Commit 5957](https://github.com/manticoresoftware/manticoresearch/commit/59577513a49eac5a4a3c5e2cb38394d3246b5d35) 修复 RT 索引保存磁盘块但未标记部分文档为删除的问题
* [Commit e861](https://github.com/manticoresoftware/manticoresearch/commit/e861f0fca0e88924450695d4e9d6acff7a36558a) 修复使用 dist_threads 多索引或多查询搜索时的崩溃
* [Commit 4409](https://github.com/manticoresoftware/manticoresearch/commit/440991fc977b8479800b45cdbc862eeb1ba5d965) 修复长术语带宽 UTF-8 代码点时中缀生成崩溃
* [Commit 5fd5](https://github.com/manticoresoftware/manticoresearch/commit/5fd599b48bba527c023e6aa0b262dca51ffb8a1c) 修复添加套接字到 IOCP 时的竞态条件
* [Commit cf10](https://github.com/manticoresoftware/manticoresearch/commit/cf10d7d3589f77adfaefbe52a51777bc9c67cf99) 修复布尔查询与 JSON 选择列表的问题
* [Commit 996d](https://github.com/manticoresoftware/manticoresearch/commit/996de77f4c3ec103d965e28c36098fd07dba50c4) 修复 indextool 检查错误的 skiplist 偏移，检查 doc2row 查找
* [Commit 6e3f](https://github.com/manticoresoftware/manticoresearch/commit/6e3fc9e88941c9427410f9d0adeebb7dd09a900f) 修复索引器在大量数据时产生带负 skiplist 偏移的错误索引
* [Commit faed](https://github.com/manticoresoftware/manticoresearch/commit/faed3220b423a96401982bf47117edf1f62c584e) 修复 JSON 只将数字转换为字符串以及表达式中的 JSON 字符串和数字转换
* [Commit 5331](https://github.com/manticoresoftware/manticoresearch/commit/533197200249d648ebcbdaca5d605861ee566348) 修复 indextool 在命令行设置多个命令时退出并返回错误代码
* [Commit 7955](https://github.com/manticoresoftware/manticoresearch/commit/795520ac351d22e4497ebaf6c249bf25081842b6) 修复 #275 在磁盘空间不足错误时 binlog 处于无效状态

* [Commit 2284](https://github.com/manticoresoftware/manticoresearch/commit/2284da5e3ff77bf7be4038a4d950c0580e4159c8) 修复 #279 在 JSON 属性的 IN 过滤器上崩溃
* [Commit ce2e](https://github.com/manticoresoftware/manticoresearch/commit/ce2e4b4765111a7d331ffc256911f12770a8942d) 修复 #281 错误的管道关闭调用
* [Commit 5355](https://github.com/manticoresoftware/manticoresearch/commit/535589ba8a37baf5c6056afb5ff969fd70476feb) 修复服务器在调用 PQ 并递归处理作为字符串编码的 JSON 属性时挂起
* [Commit a5fc](https://github.com/manticoresoftware/manticoresearch/commit/a5fc8a36e1644ee051632408ab8b53e79aeb9ff4) 修复 multiand 节点中超出文档列表末尾的推进

* [Commit a362](https://github.com/manticoresoftware/manticoresearch/commit/a3628617ef4ee89a811d4ba2c4a04cbbc298c1b5) 修复线程公共信息的检索
* [Commit f8d2](https://github.com/manticoresoftware/manticoresearch/commit/f8d2d7bbbb90af3d2ee97424f97b05927f09d42b) 修复文档存储缓存锁
## Version 3.2.0, 2019年10月17日
### 功能
* 文档存储

* 新指令 stored_fields, docstore_cache_size, docstore_block_size, docstore_compression, docstore_compression_level
### 改进和变更
* 改进 SSL 支持
* 更新非 CJK 内建字符集
* 禁用 UPDATE/DELETE 语句在查询日志中记录 SELECT
* RHEL/CentOS 8 软件包
### 修复的错误
* [Commit 301a](https://github.com/manticoresoftware/manticoresearch/commit/301a806b14725e842300ee225521407464374c41) 修复在 RT 索引的磁盘块中替换文档时崩溃
* [Commit 46c1](https://github.com/manticoresoftware/manticoresearch/commit/46c1cad8fa11c1fc09cc34b20d978fdfcd4e6d27) 修复 #269 LIMIT N OFFSET M
* [Commit 92a4](https://github.com/manticoresoftware/manticoresearch/commit/92a46edaa196ef33f3ac2225fbdba2c2b88ee45d) 修复带有显式设置 id 或提供 id 列表的 DELETE 语句跳过搜索的问题

* [Commit 8ca7](https://github.com/manticoresoftware/manticoresearch/commit/8ca78c138d837caab30dc6e2343a3c4d1687bb87) 修复在 windowspoll 轮询器的 netloop 中事件移除后索引错误
* [Commit 6036](https://github.com/manticoresoftware/manticoresearch/commit/603631e2bff5cb0acf25f68b85621742490eccd9) 修复通过 HTTP 发送的 JSON 中浮点数四舍五入问题
* [Commit 62f6](https://github.com/manticoresoftware/manticoresearch/commit/62f64cb9e75ec28fc3c29f8414ce9656f5580254) 修复远程摘要首先检查空路径；修正 Windows 测试
* [Commit aba2](https://github.com/manticoresoftware/manticoresearch/commit/aba274c2c5abc1029894ba9f95b786bda93e8f22) 修复配置重载以在 Windows 上与 Linux 上相同方式工作
* [Commit 6b8c](https://github.com/manticoresoftware/manticoresearch/commit/6b8c4242ef77c3fa4d0ccb7d76d81714b6728f0b) 修复 #194 PQ 与形态学和词干分析器配合工作
* [Commit 174d](https://github.com/manticoresoftware/manticoresearch/commit/174d312905285aa5746f4f404c53e6dc669ef42f) 修复 RT 已废弃段管理
## Version 3.1.2, 2019年8月22日

### 功能和改进
* HTTP API 的实验性 SSL 支持
* CALL KEYWORDS 的字段过滤器
* /json/search 的 max_matches 参数
* 默认 Galera gcache.size 的自动大小调整
* 改进 FreeBSD 支持
### 修复的错误
* [Commit 0a1a](https://github.com/manticoresoftware/manticoresearch/commit/0a1a2c81038794983b4e30c8aefc7cc20e89b03f) 修复将 RT 索引复制到存在相同 RT 索引但路径不同的节点时的问题

* [Commit 4adc](https://github.com/manticoresoftware/manticoresearch/commit/4adc075294ac823289f745e2cc419f18c7dcf2e2) 修复无活动索引的刷新重新调度
* [Commit d6c0](https://github.com/manticoresoftware/manticoresearch/commit/d6c00a6f750c04021747e9c99e0f77707a4cbfc3) 改进 RT/PQ 索引刷新重新调度
* [Commit d0a7](https://github.com/manticoresoftware/manticoresearch/commit/d0a7c95906b9b5ba33c98267d54501cfe27508d6) 修复 #250 TSV 和 CSV 管道源的 index_field_lengths 索引选项
* [Commit 1266](https://github.com/manticoresoftware/manticoresearch/commit/1266d54800242709fbe2c34e72eb69b6595b23bd) 修复 indextool 对空索引的块索引检查错误报告
* [Commit 553c](https://github.com/manticoresoftware/manticoresearch/commit/553ca73ca111f28e0888d59cd48b0d6df181ecd6) 修复 Manticore SQL 查询日志中的空选择列表
* [Commit 56c8](https://github.com/manticoresoftware/manticoresearch/commit/56c8584456293665ddd4ce4c94307d74473a78b9) 修复索引器 -h/--help 响应
## Version 3.1.0, 2019年7月16日
### 功能和改进
* 实时索引的复制
* 中文的 ICU 分词器
* 新形态学选项 icu_chinese
* 新指令 icu_data_dir
* 复制的多语句事务
* LAST_INSERT_ID() 和 @session.last_insert_id

* SHOW VARIABLES 的 LIKE 'pattern'
* 多文档插入到感知索引
* 配置的时间解析器

* 内部任务管理器
* 文档和命中列表组件的 mlock
* 限制摘要路径
### 移除
* 放弃对 RLP 库的支持，转而支持 ICU；所有 rlp\* 指令已移除
* 禁用使用 UPDATE 更新文档 ID
### Bug修复
* [Commit f047](https://github.com/manticoresoftware/manticoresearch/commit/f047222339fd0e62a07e9ca8b2a829297d5db861) 修复 concat 和 group_concat 中的缺陷
* [Commit b081](https://github.com/manticoresoftware/manticoresearch/commit/b08147ee24712b5eaa51f261626b5d16c6242fdd) 修复 percolate 索引中查询 uid 为 BIGINT 属性类型
* [Commit 4cd8](https://github.com/manticoresoftware/manticoresearch/commit/4cd85afaf76eef0ce9fa1cd073f3fed160279890) 若无法预分配新磁盘块，则不导致崩溃
* [Commit 1a55](https://github.com/manticoresoftware/manticoresearch/commit/1a551227886474054111088412f2b2396f485652) 在 ALTER 中添加缺失的时间戳数据类型

* [Commit f3a8](https://github.com/manticoresoftware/manticoresearch/commit/f3a8e096fd496a6b001d398aadbb781b2dbad929) 修复错误的 mmap 读取导致的崩溃问题
* [Commit 4475](https://github.com/manticoresoftware/manticoresearch/commit/447577115192db7ac7bfea70b5ba58ee2c61229f) 修复复制中集群锁的哈希
* [Commit ff47](https://github.com/manticoresoftware/manticoresearch/commit/ff476df955f057bd38e8b6ad6056325e9d6fbb9c) 修复复制中提供者的泄漏
* [Commit 58dc](https://github.com/manticoresoftware/manticoresearch/commit/58dcbb779c4c30ca1560460c78e35ba114b83716) 修复 \#246 索引器中未定义的 sigmask
* [Commit 3dd8](https://github.com/manticoresoftware/manticoresearch/commit/3dd8278e747d534f69fe9c3d360eef188ab7b6d5) 修复 netloop 报告中的竞争
* [Commit a02a](https://github.com/manticoresoftware/manticoresearch/commit/a02aae05a3fa3d1caf2821f43c7e237b6de76336) 在 HA 策略再平衡器中清除间隙

## 版本 3.0.2, 2019年5月31日
### 改进

* 为文档和命中列表添加 mmap 阅读器
* `/sql` HTTP 端点响应现在与 `/json/search` 响应相同

* 新增指令 `access_plain_attrs`, `access_blob_attrs`, `access_doclists`, `access_hitlists`
* 新增复制设置的指令 `server_id`
### 移除
* 移除了 HTTP `/search` 端点
### 弃用
* `read_buffer`, `ondisk_attrs`, `ondisk_attrs_default`, `mlock` 被 `access_*` 指令替代
### Bug修复
* [Commit 849c](https://github.com/manticoresoftware/manticoresearch/commit/849c16e1040f382795ba0635cb48686f9db4f2d8) 允许 select 列表中的属性名以数字开头
* [Commit 48e6](https://github.com/manticoresoftware/manticoresearch/commit/48e6c302ca37bec0bc49c79619c60bf787ed0708) 修复了 UDF 中的 MVA，以及 MVA 别名
* [Commit 0555](https://github.com/manticoresoftware/manticoresearch/commit/055586a9a2ebfa8874f68099c5bc798a145fd4ef) 修复了使用 SENTENCE 查询时的 \#187 崩溃问题


* [Commit 93bf](https://github.com/manticoresoftware/manticoresearch/commit/93bf52f23f1c53c8a93a603387ca5463e1fc2ae8) 修复了 MATCH() 周围支持括号的 \#143
* [Commit 599e](https://github.com/manticoresoftware/manticoresearch/commit/599ee79ccd38cce0023a55cbce8f50c3dca62e38) 修复了 ALTER cluster 语句中集群状态的保存
* [Commit 230c](https://github.com/manticoresoftware/manticoresearch/commit/230c321e277514b191629538eb3f02d219113d95) 修复了带 blob 属性的 ALTER index 导致的服务器崩溃
* [Commit 5802](https://github.com/manticoresoftware/manticoresearch/commit/5802b85aa93a81caea77073a3381ef912fdd677c) 修复了通过 id 过滤的 \#196
* [Commit 25d2](https://github.com/manticoresoftware/manticoresearch/commit/25d2dabda4a299a29bd375405862126da9b4bfac) 放弃在模板索引上搜索
* [Commit 2a30](https://github.com/manticoresoftware/manticoresearch/commit/2a30d5b49d76faab031d195f533974938769b8e6) 修复了 SQL 回复中 id 列为常规 bigint 类型
## 版本 3.0.0, 2019年5月6日
### 功能和改进
* 新的索引存储。非标量属性不再局限于每个索引 4GB 的大小
* attr_update_reserve 指令
* String、JSON 和 MVA 可以使用 UPDATE 更新
* killlists 在索引加载时应用
* killlist_target 指令
* 加快多 AND 搜索速度
* 更好的平均性能和内存使用

* 用于升级使用 2.x 制作的索引的转换工具
* CONCAT() 函数
* JOIN CLUSTER cluster AT 'nodeaddress:port'
* ALTER CLUSTER 提交更新节点

* node_address 指令
* SHOW STATUS 中打印节点列表

### 行为变化
* 对于带 killist 的索引，服务器不再按 conf 中定义的顺序旋转索引，而是遵循 killlist 目标链
* 搜索中的索引顺序不再定义 killlists 应用的顺序
* 文档 ID 现在是有符号的大整数

### 移除的指令
* docinfo（现在总是外部），inplace_docinfo_gap，mva_updates_pool
## 版本 2.8.2 GA, 2019年4月2日

### 功能和改进
* Galera 复制用于 percolate 索引
* OPTION 形态学
### 编译说明
Cmake 的最低版本现在是 3.13。编译需要 boost 和 libssl
开发库。
### Bug修复
* [Commit 6967](https://github.com/manticoresoftware/manticoresearch/commit/6967fedb2ef818ec1c825d482563edd05e1c9245) 修复了查询多个分布式索引的 select 列表中有多个星号时导致的崩溃
* [Commit 36df](https://github.com/manticoresoftware/manticoresearch/commit/36df1a407dc08263690e3492518613ace82d69ca) 修复了通过 Manticore SQL 接口发送大数据包的 [\#177](https://github.com/manticoresoftware/manticoresearch/issues/177) 
* [Commit 5793](https://github.com/manticoresoftware/manticoresearch/commit/57932aec734583fa93359faaf5034b2e2c9d352b) 修复了使用 MVA 更新后 RT 优化导致的服务器崩溃的 [\#170](https://github.com/manticoresoftware/manticoresearch/issues/170) 

* [Commit edb2](https://github.com/manticoresoftware/manticoresearch/commit/edb24b870423add86eba471d361e0e5aff098b18) 修复了由于 RT 索引移除后配置重载导致的服务器崩溃
* [Commit bd3e](https://github.com/manticoresoftware/manticoresearch/commit/bd3e66e0085bc7e2e351b817dfc972fd8158fbce) 修复了 mysql 握手认证插件的有效载荷问题
* [提交 6a21](https://github.com/manticoresoftware/manticoresearch/commit/6a217f6ed82fb10f752213b15617b9cedf1e8533) 修复了 RT 索引中的 phrase_boundary 设置问题[#172](https://github.com/manticoresoftware/manticoresearch/issues/172)
* [提交 3562](https://github.com/manticoresoftware/manticoresearch/commit/3562f652753e4091fd7b5b0f65b58341c8cbcb31) 修复了 ATTACH 索引自我死锁问题[#168](https://github.com/manticoresoftware/manticoresearch/issues/168)
* [提交 250b](https://github.com/manticoresoftware/manticoresearch/commit/250b3f0e74ad18dda34ba080aa13ca87798a9ac9) 修复 binlog 在服务器崩溃后保存空元数据的问题

* [提交 4aa6](https://github.com/manticoresoftware/manticoresearch/commit/4aa6c69ad3a181cd7c5dafb990fb528121f68e59) 修复 RT 索引带磁盘块时字符串排序器导致的服务器崩溃问题
## 版本 2.8.1 GA，2019 年 3 月 6 日
### 新功能和改进
* SUBSTRING_INDEX()
* percolate 查询支持 SENTENCE 和 PARAGRAPH
* Debian/Ubuntu 的 systemd 生成器；还添加了 LimitCORE 以允许生成 core dump
### Bug 修复
* [提交 84fe](https://github.com/manticoresoftware/manticoresearch/commit/84fe7405d17b59f4dcde15a6c8e8503923503def) 修复 match 模式为 all 且全文查询为空时服务器崩溃的问题
* [提交 daa8](https://github.com/manticoresoftware/manticoresearch/commit/daa88b579fa373d5d2e869e43a2e178363b0fef1) 修复删除静态字符串时崩溃的问题
* [提交 2207](https://github.com/manticoresoftware/manticoresearch/commit/22078537dddbd9ce81a182dcc879f235b6e87004) 修正 indextool 失败时的退出码（FATAL）
* [提交 0721](https://github.com/manticoresoftware/manticoresearch/commit/0721696d6780c200c65d596624a8187983fb7fcb) 修复因错误的精确形式检测导致前缀无匹配问题[#109](https://github.com/manticoresoftware/manticoresearch/issues/109)
* [提交 8af8](https://github.com/manticoresoftware/manticoresearch/commit/8af810111b8e2f87bc9f378172eff1ab9725c7e7) 修复 RT 索引的配置重载问题[#161](https://github.com/manticoresoftware/manticoresearch/issues/161)
* [提交 e2d5](https://github.com/manticoresoftware/manticoresearch/commit/e2d592773f4fd702ace90070a793a842a0fec6ab) 修复访问大型 JSON 字符串时服务器崩溃的问题
* [提交 75cd](https://github.com/manticoresoftware/manticoresearch/commit/75cd1342d05cdc352e3dd145cdbc6f79394a165b) 修复 JSON 文档中 PQ 字段被索引剥离器修改导致兄弟字段匹配错误问题
* [提交 e2f7](https://github.com/manticoresoftware/manticoresearch/commit/e2f775437df922674d772ed4417780492502e65a) 修复 RHEL7 构建解析 JSON 时服务器崩溃的问题
* [提交 3a25](https://github.com/manticoresoftware/manticoresearch/commit/3a25a5808feb3f8b80866991b436d6c1241618c2) 修复反斜杠在边缘时 JSON 转义崩溃问题

* [提交 be9f](https://github.com/manticoresoftware/manticoresearch/commit/be9f497872bea62dbdccd64fc28294fd7776c289) 修正 'skip_empty' 选项以跳过空文档且不警告其不是有效 JSON
* [提交 266e](https://github.com/manticoresoftware/manticoresearch/commit/266e0e7b088549722a805fc837bf101ff681a5e8) 修复浮点数位数不足时输出 8 位数字而非 6 位的问题[#140](https://github.com/manticoresoftware/manticoresearch/issues/161)
* [提交 3f6d](https://github.com/manticoresoftware/manticoresearch/commit/3f6d23891064ee8e94030c4231497cdd7da33a6a) 修复空 jsonobj 创建问题
* [提交 f3c7](https://github.com/manticoresoftware/manticoresearch/commit/f3c7848a59b7eedc67c3403d6d4b37ce6fa94dc8) 修复空 mva 输出 NULL 而非空字符串问题[#160](https://github.com/manticoresoftware/manticoresearch/issues/161)
* [提交 0afa](https://github.com/manticoresoftware/manticoresearch/commit/0afa2ed058e5759470b1d7354c722faab34f98bb) 修复 pthread_getname_np 缺失时构建失败问题
* [提交 9405](https://github.com/manticoresoftware/manticoresearch/commit/9405fccdeb27f0302a8d9a848981d30080216777) 修复线程池工作者在服务器关闭时崩溃问题
## 版本 2.8.0 GA，2019 年 1 月 28 日
### 改进
* percolate 索引的分布式索引支持
* CALL PQ 新选项及变更：
    *   skip_bad_json
    *   mode（稀疏/sparsed 和 分片/sharded）
    *   JSON 文档可以作为 JSON 数组传递
    *   shift
    *   列名 'UID', 'Documents', 'Query', 'Tags', 'Filters' 改名为 'id', 'documents', 'query', 'tags', 'filters'
* DESCRIBE pq TABLE
* 不能再使用 SELECT FROM pq WHERE UID，改用 'id'
* pq 索引上的 SELECT 与普通索引持平（例如可以通过 REGEX() 过滤规则）

* PQ 标签支持 ANY/ALL
* 表达式对 JSON 字段自动转换，无需显式类型转换
* 内置 'non_cjk' charset_table 和 'cjk' ngram_chars
* 内置 50 种语言的停用词集合
* 停用词声明中多个文件也可用逗号分隔
* CALL PQ 可接受 JSON 数组形式的文档
### Bug 修复
* [提交 a4e1](https://github.com/manticoresoftware/manticoresearch/commit/a4e19afee54dafdc04b0da53372dafd8c0d332d6) 修复 csjon 相关内存泄漏
* [提交 28d8](https://github.com/manticoresoftware/manticoresearch/commit/28d862774874891a03e361da1b0347ebe8066ce0) 修复 json 中缺失值导致的崩溃
* [提交 bf4e](https://github.com/manticoresoftware/manticoresearch/commit/bf4e9ea27c349cdc9ae4e54e960a4a17b90e38fa) 修复 RT 索引保存空元数据的问题
* [提交 33b4](https://github.com/manticoresoftware/manticoresearch/commit/33b4573529e7c7c7bce19bf9d54054866f30d99a) 修复词形还原序列丢失精确标志（exact）
* [提交 6b95](https://github.com/manticoresoftware/manticoresearch/commit/6b95d48240d3b5520afade19c249d79e020a5e88) 修复超过 4M 的字符串属性使用饱和处理替代溢出
* [提交 6214](https://github.com/manticoresoftware/manticoresearch/commit/621418b829e70af36aaa322a2f51ece3f86bc499) 修复禁用索引时 SIGHUP 导致服务器崩溃
* [提交 3f7e](https://github.com/manticoresoftware/manticoresearch/commit/3f7e35d1482966cc45d5abbcb2de5de3508d66b0) 修复 API 会话状态命令同时执行时服务器崩溃
* [提交 cd9e](https://github.com/manticoresoftware/manticoresearch/commit/cd9e4f1709a48ddafc128c450c2d882bc11ba50e) 修复了带字段过滤的 RT 索引删除查询导致服务器崩溃的问题
* [提交 9376](https://github.com/manticoresoftware/manticoresearch/commit/9376470d455fcd256c6abff79c431a6919dfa7ac) 修复了对分布式索引调用 PQ 时空文档导致服务器崩溃的问题
* [提交 8868](https://github.com/manticoresoftware/manticoresearch/commit/8868b207644f4f8cc1ab0c270adb35493fed22e1) 修复了 Manticore SQL 错误信息大于 512 字符时被截断的问题
* [提交 de9d](https://github.com/manticoresoftware/manticoresearch/commit/de9deda9c142823d6dbf529423417b5c670fae94) 修复了保存无 binlog 的 percolate 索引时崩溃的问题
* [提交 2b21](https://github.com/manticoresoftware/manticoresearch/commit/2b219e1a32791e7740bb210b7d408f96abc6374f) 修复了 OSX 上 HTTP 接口无法工作的错误

* [提交 e92c](https://github.com/manticoresoftware/manticoresearch/commit/e92c60240f8f9e1756bfa14fba0bbb402586a389) 修复了 indextool 在检查 MVA 时错误提示的问题
* [提交 238b](https://github.com/manticoresoftware/manticoresearch/commit/238bdea59bad89f097403f1c978658ce45f16c70) 修复了 FLUSH RTINDEX 期间写锁定整个索引的问题，确保保存和定期从 rt_flush_period 刷新时不锁全索引
* [提交 c26a](https://github.com/manticoresoftware/manticoresearch/commit/c26a236bd42b7082db079a51a84172066d337d64) 修复了 ALTER percolate 索引时卡在等待搜索加载的问题
* [提交 9ee5](https://github.com/manticoresoftware/manticoresearch/commit/9ee5703f6c28f57cd5bff5e705966d93fe30d267) 修复了当 max_children 设置为 0 时，默认使用线程池工作线程数量的问题
* [提交 5138](https://github.com/manticoresoftware/manticoresearch/commit/5138fc048a1bd146b271ce6a72fe954344281f69) 修复了带有 index_token_filter 插件并且设置了 stopwords 和 stopword_step=0 的索引数据索引时的错误

* [提交 2add](https://github.com/manticoresoftware/manticoresearch/commit/2add3d319ac62fe450bf60e89033100853dc2ecf) 修复了仍使用 aot 词形还原器定义索引时，缺少 lemmatizer_base 导致崩溃的问题
## 版本 2.7.5 GA，2018年12月4日
### 改进
* REGEX 函数
* json API 搜索支持 limit/offset
* qcache 的性能分析点
### Bug 修复

* [提交 eb3c](https://github.com/manticoresoftware/manticoresearch/commit/eb3c7683b9740cff4cc4cadd2ab9f990b9f7d173) 修复了在 FACET 时处理多属性宽类型导致服务器崩溃的问题
* [提交 d915](https://github.com/manticoresoftware/manticoresearch/commit/d915cf6696514f4be365ffb5981bccaa15983e65) 修复了 FACET 查询主选择列表隐式分组的问题
* [提交 5c25](https://github.com/manticoresoftware/manticoresearch/commit/5c25dc271a8c11a1c82ca3c88e518acbc6a8c164) 修复了带 GROUP N BY 查询时崩溃的问题
* [提交 85d3](https://github.com/manticoresoftware/manticoresearch/commit/85d30a232d4ffd89916b673a9b2db946d272a945) 修复了处理内存操作崩溃时的死锁问题
* [提交 8516](https://github.com/manticoresoftware/manticoresearch/commit/85166b5ef2be38061d62f2b20f15acaa0311258a) 修复了 indextool 检查过程中的内存消耗问题

* [提交 58fb](https://github.com/manticoresoftware/manticoresearch/commit/58fb03166830d8b23e7ab9ce56309db7ddac22f4) 修复了不再需要包含 gmock 的问题，因为上游已经自行解决
## 版本 2.7.4 GA，2018年11月1日
### 改进
* SHOW THREADS 对远程分布式索引打印原始查询而不是 API 调用
* SHOW THREADS 新选项 `format=sphinxql` 以 SQL 格式打印所有查询
* SHOW PROFILE 增加了 `clone_attrs` 阶段
### Bug 修复
* [提交 4f15](https://github.com/manticoresoftware/manticoresearch/commit/4f155712a0bccc1bd01cc191647bc8cff814888e) 修复了在没有 malloc_stats 和 malloc_trim 的 libc 上编译失败的问题

* [提交 f974](https://github.com/manticoresoftware/manticoresearch/commit/f974f20bda3214a56877c393a192be1a77150958) 修复了 CALL KEYWORDS 结果集中单词中包含特殊符号的问题
* [提交 0920](https://github.com/manticoresoftware/manticoresearch/commit/092083282ea8ae0b2e72fb6989c57ccec81e74ac) 修复了通过 API 或远程代理调用 CALL KEYWORDS 到分布式索引时损坏的问题
* [提交 fd68](https://github.com/manticoresoftware/manticoresearch/commit/fd686bfe88b720ffd7642e36897ba45161cbd7d2) 修复了分布式索引 agent_query_timeout 未传递给代理时的最大查询时间问题
* [提交 4ffa](https://github.com/manticoresoftware/manticoresearch/commit/4ffa623e9d357a3b0b441615089e211f92f8de32) 修复了磁盘块的总文档计数受到 OPTIMIZE 命令影响，破坏权重计算的问题
* [提交 dcaf](https://github.com/manticoresoftware/manticoresearch/commit/dcaf4e0e3c8e9b0fbec47bbf3307feddec0936a6) 修复了混合查询中 RT 索引多重尾部命中问题
* [提交 eee3](https://github.com/manticoresoftware/manticoresearch/commit/eee381754e902a43a8f499e1c950198a2a3e6ee0) 修复了旋转时的死锁问题

## 版本 2.7.3 GA，2018年9月26日
### 改进
* CALL KEYWORDS 增加 sort_mode 选项
* VIP 连接上的 DEBUG 可以执行 'crash <password>'，以便故意触发服务器的 SIGEGV 操作
* DEBUG 可以执行 'malloc_stats' 以在 searchd.log 中转储 malloc 统计信息，执行 'malloc_trim' 以调用 malloc_trim()
* 如果系统中存在 gdb，改进了堆栈跟踪
### Bug 修复
* [提交 0f3c](https://github.com/manticoresoftware/manticoresearch/commit/0f3cc333f6129451ad1ae2c97905b6c04ba5d454) 修复了 Windows 上重命名时的崩溃或失败问题
* [提交 1455](https://github.com/manticoresoftware/manticoresearch/commit/1455ba2bf8079f03ce7f861a8d9662a360d705de) 修复了 32 位系统上服务器崩溃问题
* [提交 ad37](https://github.com/manticoresoftware/manticoresearch/commit/ad3710d53b5cd6a28b8c60352d0f9e285b03d9fa) 修复了空 SNIPPET 表达式导致服务器崩溃或挂起的问题
* [提交 b36d](https://github.com/manticoresoftware/manticoresearch/commit/b36d79214364500252665860f000ae85343528b6) 修复了非渐进式优化损坏问题，改进渐进式优化不为最旧的磁盘块创建杀死列表
* [Commit 34b0](https://github.com/manticoresoftware/manticoresearch/commit/34b032499afd42ce47a4c7247814b4031094388a) 修复了线程池工作模式下 SQL 和 API 中 queue_max_length 的错误回复
* [Commit ae4b](https://github.com/manticoresoftware/manticoresearch/commit/ae4b3202cbdb8014cabe2b90e269d5cf74f49871) 修复了在 PQ 索引中添加带有 regexp 或 rlp 选项的全扫描查询时的崩溃问题
* [Commit f80f](https://github.com/manticoresoftware/manticoresearch/commit/f80f8d5d7560187078868aed9a9575f4549e98aa) 修复了连续调用多个 PQ 时的崩溃问题
* [Commit 9742](https://github.com/manticoresoftware/manticoresearch/commit/9742f5f0866af73f8cd8483ecd18a507ea80dd65) 重构 AcquireAccum
* [Commit 39e5](https://github.com/manticoresoftware/manticoresearch/commit/39e5bc3751b7295222eb76407c5d72ce1dad545b) 修复调用 pq 后的内存泄漏
* [Commit 21bc](https://github.com/manticoresoftware/manticoresearch/commit/21bcc6d17395f0f57dde79f6716ef303b7ea527d) 外观上的重构（c++11 风格的 c-trs，默认值，nullptr）
* [Commit 2d69](https://github.com/manticoresoftware/manticoresearch/commit/2d690398f14c736956cfdd66feb4d3091d6b3a4d) 修复尝试向 PQ 索引插入重复项时的内存泄漏
* [Commit 5ed9](https://github.com/manticoresoftware/manticoresearch/commit/5ed92c4b66da6423fa28f11b1fd59103ed1e1fb9) 修复 JSON 字段 IN 语句中大值导致的崩溃
* [Commit 4a52](https://github.com/manticoresoftware/manticoresearch/commit/4a5262e2776aa261e34ba5c914ea60804f59de3f) 修复对设置了扩展限制的 RT 索引执行 CALL KEYWORDS 语句时服务器崩溃
* [Commit 5526](https://github.com/manticoresoftware/manticoresearch/commit/552646bb6fefde4e2525298e43e628839b421d3d) 修复 PQ matches 查询中过滤器无效的问题；
* [Commit 204f](https://github.com/manticoresoftware/manticoresearch/commit/204f521408b730198a5bab2c20407a3d7df94873) 为 ptr 属性引入小对象分配器
* [Commit 2545](https://github.com/manticoresoftware/manticoresearch/commit/25453e5387152c0575d1eda1b2beb89a49c7493a) 将 ISphFieldFilter 重构为引用计数版本
* [Commit 1366](https://github.com/manticoresoftware/manticoresearch/commit/1366ee06e828397074b69bec9265d7147170c60e) 修复在非终止字符串上使用 strtod 导致的未定义行为/段错误
* [Commit 94bc](https://github.com/manticoresoftware/manticoresearch/commit/94bc6fcbdae4d08bcf6da2966e3ec2966091289f) 修复 json 结果集处理中的内存泄漏
* [Commit e78e](https://github.com/manticoresoftware/manticoresearch/commit/e78e9c948963416caff9e4e46296a58080107835) 修复应用属性添加时读取超出内存块末尾的问题
* [Commit fad5](https://github.com/manticoresoftware/manticoresearch/commit/fad572fb543606b33e688eb82a485b86011545fd) 重构 CSphDict 为引用计数版本
* [Commit fd84](https://github.com/manticoresoftware/manticoresearch/commit/fd841a472b472280c93f4af3506851d19f6adc45) 修复 AOT 内部类型外泄漏
* [Commit 5ee7](https://github.com/manticoresoftware/manticoresearch/commit/5ee7f208c7685f2ebb9e5623a8802b721e2a7f0a) 修复分词器管理的内存泄漏
* [Commit 116c](https://github.com/manticoresoftware/manticoresearch/commit/116c5f1abebee9a0d99afe93546a1d8e4c6c6958) 修复分组器的内存泄漏
* [Commit 56fd](https://github.com/manticoresoftware/manticoresearch/commit/56fdbc9ab38973a9a0ba8ccee45f71cf33c16423) 对 matches 中的动态指针进行特殊的释放/复制（分组器的内存泄漏）
* [Commit b1fc](https://github.com/manticoresoftware/manticoresearch/commit/31db660f378541aa0d1e7cf2e7209bb0a1f47fd8) 修复 RT 动态字符串的内存泄漏
* [Commit 517b](https://github.com/manticoresoftware/manticoresearch/commit/517b9e8749f092e923cd2884b674b5bb84e20755) 重构分组器
* [Commit b1fc](https://github.com/manticoresoftware/manticoresearch/commit/b1fc16140e5dc44290686330b476a254e0092107) 小幅重构（c++11 c-trs，部分格式调整）
* [Commit 7034](https://github.com/manticoresoftware/manticoresearch/commit/7034e073f4cf2844762e0a464b7c3de05d3d122b) 将 ISphMatchComparator 重构为引用计数版本
* [Commit b1fc](https://github.com/manticoresoftware/manticoresearch/commit/b1fc16140e5dc44290686330b476a254e0092107) 私有化克隆器
* [Commit efbc](https://github.com/manticoresoftware/manticoresearch/commit/efbc0511d6809c4a57453c7283f9bf53f3fb8d97) 简化 MVA_UPSIZE、DOCINFO2ID_T、DOCINFOSETID 的原生小端处理
* [Commit 6da0](https://github.com/manticoresoftware/manticoresearch/commit/6da0df4ac9946fc59f98d229b90e12c9221dd4c0) 为 ubertests 添加 valgrind 支持
* [Commit 1d17](https://github.com/manticoresoftware/manticoresearch/commit/1d17669cb7ec54ac80b392e101f37a688eb98080) 修复连接中 'success' 标志竞争引发的崩溃
* [Commit 5a09](https://github.com/manticoresoftware/manticoresearch/commit/5a09c32d7ded56b2c0bd3e2ad7968cb8d6362919) 将 epoll 切换为边缘触发模式
* [Commit 5d52](https://github.com/manticoresoftware/manticoresearch/commit/5d528682737fca03a4352a3093b1319ec27d598c) 修复表达式中格式类似过滤器的 IN 语句
* [Commit bd8b](https://github.com/manticoresoftware/manticoresearch/commit/bd8b3c976ff8b4667f55dfdd101b20a920137ac5) 修复在提交具有大 docid 的文档到 RT 索引时的崩溃
* [Commit ce65](https://github.com/manticoresoftware/manticoresearch/commit/ce656b83b747ce7c55795b53915770c13fb99cfe) 修复 indextool 中缺少参数的选项
* [Commit 08c9](https://github.com/manticoresoftware/manticoresearch/commit/08c9507177820f441f534bf06a11dac5e54bebb4) 修复扩展关键字的内存泄漏

* [Commit 30c7](https://github.com/manticoresoftware/manticoresearch/commit/30c75a2f525ec9bda625d903acdc9d4d2e2413dc) 修复 json 分组器的内存泄漏
* [Commit 6023](https://github.com/manticoresoftware/manticoresearch/commit/6023f269b6f2080e4d380b0957605ef8107c8c9f) 修复全局用户变量泄漏
* [Commit 7c13](https://github.com/manticoresoftware/manticoresearch/commit/7c138f15ca23c0c8717fa12041e3db7f988fcf15) 修复早期拒绝匹配中动态字符串的泄漏
* [Commit 9154](https://github.com/manticoresoftware/manticoresearch/commit/9154b18eaed5302e8330cb3a95bd968959ccb312) 修复了 length(<expression>) 的泄漏问题
* [Commit 43fc](https://github.com/manticoresoftware/manticoresearch/commit/43fca3a4e26139765d0fac8de054200dc4875fc6) 修复了解析器中因为 strdup() 导致的内存泄漏
* [Commit 71ff](https://github.com/manticoresoftware/manticoresearch/commit/71ff77737e63d1019b7c9804dca7fa2351025dba) 重构表达式解析器以准确跟踪引用计数的修复
## 版本 2.7.2 GA，2018年8月27日
### 改进
* 与 MySQL 8 客户端的兼容性
* 支持带 RECONFIGURE 的 [TRUNCATE](Emptying_a_table.md)
* 在 RT 索引的 SHOW STATUS 中弃用内存计数器
* 多代理的全局缓存

* 改进了 Windows 上的 IOCP
* HTTP 协议的 VIP 连接
* Manticore SQL [DEBUG](Reporting_bugs.md#DEBUG) 命令，可运行各种子命令
* [shutdown_token](Server_settings/Searchd.md#shutdown_token) - 需要 SHA1 哈希密码来通过 DEBUG 命令调用 `shutdown`
* 新增 SHOW AGENT STATUS 统计信息（_ping、_has_perspool、_need_resolve）
* indexer 的 --verbose 选项现支持 \[debugvv\] 以打印调试信息
### Bug修复
* [Commit 3900](https://github.com/manticoresoftware/manticoresearch/commit/390082a7be0a1f9539b30361d11d54de35c62a44) 移除了 optimize 时的写锁
* [Commit 4c33](https://github.com/manticoresoftware/manticoresearch/commit/4c337648329be22373be33333cbc9c5664d18b80) 修复了重新加载索引设置时的写锁问题
* [Commit b5ea](https://github.com/manticoresoftware/manticoresearch/commit/b5ea8dc0b18a23dbd739e92c66dd93b0094df14b) 修复了使用 JSON 过滤器查询时的内存泄漏
* [Commit 930e](https://github.com/manticoresoftware/manticoresearch/commit/930e831a43fd3d1ce996185be968d1df49a088d0) 修复了 PQ 结果集中的空文档问题
* [Commit 53de](https://github.com/manticoresoftware/manticoresearch/commit/53deecfecb374431f84f8592a1095a77407b8aea) 修复了因任务被移除而导致的混淆问题
* [Commit cad9](https://github.com/manticoresoftware/manticoresearch/commit/cad9b9c617719aad97e9f5a2ae93491153d8e504) 修复了错误的远程主机计数
* [Commit 9000](https://github.com/manticoresoftware/manticoresearch/commit/90008cc15c4684d445d37a391bb4a667bbc2e365) 修复了解析的代理描述符的内存泄漏
* [Commit 978d](https://github.com/manticoresoftware/manticoresearch/commit/978d839641ad6040c22c9fc3b703b1a02685f266) 修复了搜索中的内存泄漏
* [Commit 0193](https://github.com/manticoresoftware/manticoresearch/commit/0193946105ac08913362b7d7acf0c90f9b38dc85) 对显式/内联 c-trs、override/final 用法的格式调整
* [Commit 943e](https://github.com/manticoresoftware/manticoresearch/commit/943e2997b33f9b2eda23d94a66068910f9476ebd) 修复了本地/远程 schema 中 json 泄漏问题
* [Commit 02db](https://github.com/manticoresoftware/manticoresearch/commit/02dbdd6f1d0b72e7e458ebebdd2c67f989577e6b) 修复了本地/远程 schema 中 json 排序列表达式的泄漏
* [Commit c74d](https://github.com/manticoresoftware/manticoresearch/commit/c74d0b4035e4214510376845b22cb676a8da24a3) 修复了常量别名的泄漏
* [Commit 6e5b](https://github.com/manticoresoftware/manticoresearch/commit/6e5b57e1d3952c9695376728a143bf5434208f53) 修复了预读线程泄漏
* [Commit 39c7](https://github.com/manticoresoftware/manticoresearch/commit/39c740fef5bf71da0bee60a35f5dd9f471af850f) 修复了因网络循环中等待卡住导致退出时卡死的问题
* [Commit adaf](https://github.com/manticoresoftware/manticoresearch/commit/adaf9772f95772ad6d5297acace6c5cb92b56ad5) 修复了将 HA 代理更改为普通主机时 'ping' 行为卡死问题
* [Commit 32c4](https://github.com/manticoresoftware/manticoresearch/commit/32c40eac9cdd40b15d79e58b2f15416f164f9701) 分离了仪表盘存储的垃圾回收
* [Commit 511a](https://github.com/manticoresoftware/manticoresearch/commit/511a3cf2c1f1858f6e94fe9f8175b7216db3cbd6) 修复了引用计数指针的相关问题
* [Commit 32c4](https://github.com/manticoresoftware/manticoresearch/commit/af2326cd2927effbad7bec88e8dc238a566cf27e) 修复了 indextool 在索引不存在时崩溃问题
* [Commit 156e](https://github.com/manticoresoftware/manticoresearch/commit/156edc717dc63d3ea120f42466eba6e022da88be) 修正了 xmlpipe 索引中过度属性/字段的输出名称
* [Commit cdac](https://github.com/manticoresoftware/manticoresearch/commit/cdac6d1f17d384bfc8bae49e91241b2f346c3830) 修复了配置中无索引器段时默认索引器值的问题
* [Commit e61e](https://github.com/manticoresoftware/manticoresearch/commit/e61ec00b6b27d1d5878247e2ee817f3b1e7fde16) 修复了服务器重启后 RT 索引磁盘块内错误嵌入的停用词

* [Commit 5fba](https://github.com/manticoresoftware/manticoresearch/commit/5fba49d5e28c17de4c0acbd984466127f42de6e8) 修复了跳过幻影（已关闭但未从轮询器最终删除）连接的问题
* [Commit f22a](https://github.com/manticoresoftware/manticoresearch/commit/f22ae34b623906b3c5a05a06198dad2e548dc541) 修复了混合（孤立）网络任务问题
* [Commit 4689](https://github.com/manticoresoftware/manticoresearch/commit/46890e70eb8dcd00c1bf1b030538b8f131c601c2) 修复了写操作后读取动作崩溃的问题
* [Commit 03f9](https://github.com/manticoresoftware/manticoresearch/commit/03f9df904f611c438d5ebcaaeef911b0dc8d6c39) 修复了 Windows 上运行测试时 searchd 崩溃的问题
* [Commit e925](https://github.com/manticoresoftware/manticoresearch/commit/e9255ed2704790a3d7f5273d38b85433463cb3ff) 修正了对常规 connect() 调用中 EINPROGRESS 代码的处理
* [Commit 248b](https://github.com/manticoresoftware/manticoresearch/commit/248b72f1544b27e9e82773284050d44eeb1714e1) 修复了使用 TFO 时的连接超时问题

## 版本 2.7.1 GA，2018年7月4日
### 改进
* 提升了 PQ 中匹配多个文档时通配符的性能
* 支持 PQ 中的 fullscan 查询
* 支持 PQ 中的 MVA 属性


* 支持 percolate 索引的正则表达式和 RLP
### Bug修复
* [Commit 6885](https://github.com/manticoresoftware/manticoresearch/commit/68856261b41f6950666f9e5122726839fb3c71d0) 修复了查询字符串丢失问题
* [Commit 0f17](https://github.com/manticoresoftware/manticoresearch/commit/0f1770943cba53bfd2e0edfdf0b1495ff0dd0e89) 修复了 SHOW THREADS 语句中空的 info 问题
* [Commit 53fa](https://github.com/manticoresoftware/manticoresearch/commit/53faa36a916bb87868e83aa79c1a1f972fb20ca0) 修复了使用 NOTNEAR 操作符匹配时的崩溃问题
* [Commit 2602](https://github.com/manticoresoftware/manticoresearch/commit/26029a7cd77b518fb3a27588d7db6c8bffc73b0f) 修复了 PQ 删除时错误的过滤器错误信息
## 版本 2.7.0 GA，2018年6月11日
### 改进
* 减少系统调用次数，避免 Meltdown 和 Spectre 补丁的影响
* 本地索引管理的内部重写
* 远程摘要重构
* 完全配置重新加载
* 所有节点连接现在都是独立的
* 协议改进
* Windows 通信从 wsapoll 切换到 IO 完成端口

* 主节点和节点间通信可以使用 TFO
* [SHOW STATUS](Node_info_and_management/Node_status.md#SHOW-STATUS) 现在输出服务器版本和 mysql_version_string
* 增加了 CALL PQ 文档调用的 `docs_id` 选项。
* percolate 查询过滤器现在可以包含表达式
* 分布式索引可以与 FEDERATED 一起工作
* 虚拟的 SHOW NAMES COLLATE 和 `SET wait_timeout`（以更好支持 ProxySQL）
### 修复
* [Commit 5bcf](https://github.com/manticoresoftware/manticoresearch/commit/5bcff05d19e495f1a44f190885b25cbb6f217c4a) 修复了 PQ 标签中添加的不等于问题
* [Commit 9ebc](https://github.com/manticoresoftware/manticoresearch/commit/9ebc58916cd515eaa88da66d0895aebf0d1f2b5f) 修复了调用 PQ 语句中添加文档 ID 字段到 JSON 文档的问题
* [Commit 8ae0](https://github.com/manticoresoftware/manticoresearch/commit/8ae0e593a286110d8505d88baa3ac9e433cb38c9) 修复了对 PQ 索引的 flush 语句处理器
* [Commit c24b](https://github.com/manticoresoftware/manticoresearch/commit/c24b152344ecd77661566cddb803487f51d3c1aa) 修复了 PQ 对 JSON 和字符串属性的过滤问题
* [Commit 1b8b](https://github.com/manticoresoftware/manticoresearch/commit/1b8bdde19aaa362785ea4c33bdc019154f8fbac4) 修复了空 JSON 字符串的解析问题


* [Commit 1ad8](https://github.com/manticoresoftware/manticoresearch/commit/1ad8a07dbfc1131913a8d92c261fbb48f934e8b7) 修复了多查询中带 OR 过滤器时的崩溃问题
* [Commit 69b8](https://github.com/manticoresoftware/manticoresearch/commit/69b89806380a229e36287ad9daf6f0ea2b5453eb) 修复了 indextool 使用配置公共部分（lemmatizer_base 选项）进行命令（dumpheader）
* [Commit 6dbe](https://github.com/manticoresoftware/manticoresearch/commit/6dbeaf2389d2af6a9dfccb56bfc986685b9f203e) 修复了结果集和过滤器中的空字符串
* [Commit 39c4](https://github.com/manticoresoftware/manticoresearch/commit/39c4eb55ecc85f6dd54f6c8c6d6dfcf87fd8748e) 修复了负文档 ID 值的问题
* [Commit 266b](https://github.com/manticoresoftware/manticoresearch/commit/266b7054f98e7d30fc2829b5c2467dc9f90def92) 修复了对非常长单词的截断长度
* [Commit 4782](https://github.com/manticoresoftware/manticoresearch/commit/47823b0bd06cbd016a24345c99bb8693790b43dd) 修复了 PQ 通配符查询匹配多个文档问题
## 版本 2.6.4 GA，2018年5月3日

### 新特性和改进
* 支持 MySQL FEDERATED 引擎 [支持](Extensions/FEDERATED.md)
* MySQL 包现在返回 SERVER_STATUS_AUTOCOMMIT 标志，增强与 ProxySQL 的兼容性
* [listen_tfo](Server_settings/Searchd.md#listen_tfo) - 为所有监听器启用 TCP Fast Open 连接
* indexer --dumpheader 现在也可以从 .meta 文件中导出 RT 头部信息
* Ubuntu Bionic 的 cmake 构建脚本
### 修复
* [Commit 355b](https://github.com/manticoresoftware/manticoresearch/commit/355b11629174813abd9cd5bf2233be0783f77745) 修复了 RT 索引的无效查询缓存条目；

* [Commit 546e](https://github.com/manticoresoftware/manticoresearch/commit/546e2297a01cc2913bc0d33052d5bcefae8737eb) 修复了无缝旋转后索引设置丢失的问题
* [Commit 0c45](https://github.com/manticoresoftware/manticoresearch/commit/0c4509898393993bc87194d2aca2070395ff7f83) 修复了夹入与前缀长度设置问题；增加了不支持夹入长度的警告
* [Commit 8054](https://github.com/manticoresoftware/manticoresearch/commit/80542fa54d727c781635918e8d4d65a20c2fbd1e) 修复了 RT 索引自动刷新顺序

* [Commit 705d](https://github.com/manticoresoftware/manticoresearch/commit/705d8c5f407726ed1c6f9745ecffa375682c8969) 修复了多属性索引及多索引查询结果集模式问题
* [Commit b0ba](https://github.com/manticoresoftware/manticoresearch/commit/b0ba932fff4a082f642cb737381c8a70215d4cc9) 修复了批量插入时文档重复导致的部分命中丢失
* [Commit 4510](https://github.com/manticoresoftware/manticoresearch/commit/4510fa44ad54b5bbac53c591144501565130fcae) 修复了优化失败合并包含大量文档的 RT 索引磁盘块问题
## 版本 2.6.3 GA，2018年3月28日
### 改进
* 编译时加入 jemalloc。如果系统中存在 jemalloc，可以通过 cmake 标志 `-DUSE_JEMALLOC=1` 启用


### 修复
* [Commit 85a6](https://github.com/manticoresoftware/manticoresearch/commit/85a6d7e3db75f28c491042409bb677ed39eba07b) 修复了日志展开关键词选项到 Manticore SQL 查询日志的问题
* [Commit caaa](https://github.com/manticoresoftware/manticoresearch/commit/caaa3842f4dbd8507fd9e20bf3371a0c8ec0c087) 修复了 HTTP 接口正确处理大尺寸查询的问题
* [Commit e386](https://github.com/manticoresoftware/manticoresearch/commit/e386d842b36195719e459b2db1fb20e402f9a51f) 修复了启用 index_field_lengths 时对 RT 索引执行 DELETE 导致的服务器崩溃问题
* [Commit cd53](https://github.com/manticoresoftware/manticoresearch/commit/ce528f37eb5d8284661928f6f3b340a4a1985253) 修复了 cpustats searchd CLI 选项在不支持系统上的工作问题
* [Commit 8740](https://github.com/manticoresoftware/manticoresearch/commit/8740fd63ae842c2ab0e51a8fcd6180d787ed57ec) 修复了定义最小长度时 utf8 子串匹配的问题
## 版本 2.6.2 GA，2018年2月23日
### 改进

* 改善了在使用 NOT 运算符和批量文档情况下的[Percolate Queries](Searching/Percolate_query.md)性能。
* [percolate_query_call](Searching/Percolate_query.md) 可以根据 [dist_threads](Server_settings/Searchd.md#threads) 使用多线程。
* 新增全文匹配运算符 NOTNEAR/N。
* 对 percolate 索引的 SELECT 添加 LIMIT 支持。
* [expand_keywords](Searching/Options.md#expand_keywords) 可以接受 'start'、'exact' （其中 'start,exact' 与 '1' 效果相同）。
* 针对使用 sql_query_range 定义范围查询的[联接字段](Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Processing_fetched_data.md#sql_joined_field)的范围主查询。
### Bug 修复
* [Commit 72dc](https://github.com/manticoresoftware/manticoresearch/commit/72dcf669744e9b7d636dfc213d24df85ab301f6b) 修复了搜索 RAM 段时崩溃；保存磁盘块双缓冲时死锁；优化时保存磁盘块死锁。
* [Commit 3613](https://github.com/manticoresoftware/manticoresearch/commit/36137149a1c3c0893bdda5a28fc7e8244bf2d4ae) 修复了索引器在 XML 嵌入式模式下属性名为空时崩溃。


* [Commit 48d7](https://github.com/manticoresoftware/manticoresearch/commit/48d7e8001d2a66466ca64577f27ddc5421a67251) 修复了错误解除未拥有的 pid 文件链接问题。
* [Commit a556](https://github.com/manticoresoftware/manticoresearch/commit/a5563a465ddc59ef71e65f17b68bc33f9700e838) 修复了有时留在临时文件夹中的孤立 FIFO。
* [Commit 2376](https://github.com/manticoresoftware/manticoresearch/commit/2376e8fc4508944b96959bd10686c6d51f5145e8) 修复了空 FACET 结果集包含错误 NULL 行的问题。
* [Commit 4842](https://github.com/manticoresoftware/manticoresearch/commit/4842b67acdd5da75672db6c08ac563b48577ddd7) 修复了在 Windows 服务模式运行服务器时索引锁损坏的问题。
* [Commit be35](https://github.com/manticoresoftware/manticoresearch/commit/be35feef54ded59125750916cd211e56108eddae) 修复了 mac os 上错误的 iconv 库。
* [Commit 8374](https://github.com/manticoresoftware/manticoresearch/commit/83744a977d7837d1d87cd506b88b0e7eb199efe6) 修复了错误的 count(\*) 统计。
## 版本 2.6.1 GA, 2018年1月26日
### 改进

* [agent_retry_count](Server_settings/Searchd.md#agent_retry_count) 在有镜像的代理中，返回每个镜像的重试次数，而非每个代理的总重试次数，总重试次数为 agent_retry_count\*mirrors。
* [agent_retry_count](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_retry_count) 现在可以针对单个索引指定，覆盖全局值。新增别名 [mirror_retry_count](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#mirror_retry_count)。
* 可以在代理定义中指定 retry_count，表示每个代理的重试次数。
* Percolate Queries 现在支持通过 HTTP JSON API 在 [/json/pq](Searching/Percolate_query.md#Performing-a-percolate-query-with-CALL-PQ) 使用。
* 可执行文件增加了 -h 和 -v 参数（帮助与版本）。
* [morphology_skip_fields](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) 支持实时索引。
### Bug 修复


* [Commit a40b](https://github.com/manticoresoftware/manticore/commit/a40b0793feff65e40d10062568d9847c08d10f57) 修复了范围主查询在 MVA 字段上使用 sql_range_step 时的正确性问题。
* [Commit f2f5](https://github.com/manticoresoftware/manticore/commit/f2f53757db45bcfb1544263ce0817e856656a621) 修复了黑洞系统循环挂起和黑洞代理连接丢失的问题。
* [Commit 84e1](https://github.com/manticoresoftware/manticore/commit/84e1f54aef25e0fce98870ad2dd784db5116f1d6) 修复了查询 id 不一致和存储查询重复 id 问题。
* [Commit 1948](https://github.com/manticoresoftware/manticore/commit/19484231814fcb82b21763a3a4a9f45adc6b2d40) 修复了服务器在多种状态下关闭时崩溃。
* [Commit 9a70](https://github.com/manticoresoftware/manticore/commit/9a706b499a1d61a90076065a1a703029d49db958)  [Commit 3495](https://github.com/manticoresoftware/manticore/commit/3495fd70cba8846b1a50d55d6679b039414c3d2a) 修复了长查询超时问题。
* [Commit 3359](https://github.com/manticoresoftware/manticore/commit/3359bcd89b4f79a645fe84b8cf8616ce0addff02) 重构了基于 kqueue 系统（Mac OS X，BSD）的主-代理网络轮询。
## 版本 2.6.0，2017年12月29日

### 功能和改进
* [HTTP JSON](Connecting_to_the_server/HTTP.md)：JSON 查询现在支持属性的相等判断，MVA 和 JSON 属性可用于插入和更新，分布式索引支持通过 JSON API 执行更新和删除。
* [Percolate Queries](Searching/Percolate_query.md)
* 移除了对 32 位 docid 的支持，也移除了转换/加载旧版 32 位 docid 索引的所有代码。
* [仅为部分字段启用形态学](https://github.com/manticoresoftware/manticore/issues/7) 。新增索引指令 morphology_skip_fields，可定义不应用形态学的字段列表。
* [expand_keywords 现在可以通过 OPTION 语句作为查询运行时指令设置](https://github.com/manticoresoftware/manticore/issues/8)

### Bug 修复
* [Commit 0cfa](https://github.com/manticoresoftware/manticore/commit/0cfae4c) 修复了服务器调试版本（发布版可能有未定义行为）使用 rlp 构建时崩溃的问题。

* [Commit 3242](https://github.com/manticoresoftware/manticore/commit/324291e) 修复了开启 progressive 选项的 RT 索引优化中合并 kill list 顺序错误的问题。
* [Commit ac0e](https://github.com/manticoresoftware/manticore/commit/ac0efee) 修复了 mac 上的小崩溃。
* 经过全面静态代码分析后进行的众多小修复。
* 其他小的 bug 修复。
### 升级说明
本次发布更改了主控和代理之间通信使用的内部协议。如果您在分布式环境中运行多个 Manticoresearch 实例，请确保先升级代理，再升级主控。
## 版本 2.5.1，2017年11月23日
### 功能和改进
* JSON 查询在 [HTTP API 协议](Connecting_to_the_server/HTTP.md) 上支持。支持搜索、插入、更新、删除、替换操作。数据操作命令也可以批量执行，但目前存在一些限制，例如多值属性（MVA）和 JSON 属性不能用于插入、替换或更新。
* [RELOAD INDEXES](Data_creation_and_modification/Adding_data_from_external_storages/Rotating_a_table.md#RELOAD-TABLES) 命令
* [FLUSH LOGS](Logging/Rotating_query_and_server_logs.md) 命令

* [SHOW THREADS](Node_info_and_management/SHOW_THREADS.md) 可显示优化、轮换或刷新进度。
* GROUP N BY 能够正确处理多值属性（MVA）
* blackhole 代理运行在独立线程，不再影响主查询
* 实现了索引的引用计数，以避免轮换和高负载导致的停顿
* 实现了 SHA1 哈希，但尚未对外暴露
* 解决了在 FreeBSD、macOS 和 Alpine 上的编译问题
### Bugfixes
* [Commit 9897](https://github.com/manticoresoftware/manticore/commit/989752b) 修复了基于块索引的过滤器回归问题

* [Commit b1c3](https://github.com/manticoresoftware/manticore/commit/b1c3864) 将 PAGE_SIZE 重命名为 ARENA_PAGE_SIZE，以兼容 musl
* [Commit f213](https://github.com/manticoresoftware/manticore/commit/f2133cc) 禁用 cmake 版本低于 3.1.0 的 googletests
* [Commit f30e](https://github.com/manticoresoftware/manticore/commit/0839de7) 修复服务器重启时绑定套接字失败问题
* [Commit 0807](https://github.com/manticoresoftware/manticore/commit/0807240) 修复服务器关闭时崩溃问题
* [Commit 3e3a](https://github.com/manticoresoftware/manticore/commit/3e3acc3) 修正系统 blackhole 线程的 show threads
* [Commit 262c](https://github.com/manticoresoftware/manticore/commit/262c3fe) 重构了 iconv 的配置检测，修复了 FreeBSD 和 Darwin 平台的构建问题
## 版本 2.4.1 GA，2017 年 10 月 16 日
### 新功能和改进
* WHERE 子句中属性过滤支持 OR 操作符
* 维护模式（SET MAINTENANCE=1）
* [CALL KEYWORDS](Searching/Autocomplete.md#CALL-KEYWORDS) 现可用于分布式索引
* [分组时使用 UTC 时间](Server_settings/Searchd.md#grouping_in_utc)
* [query_log_mode](Server_settings/Searchd.md#query_log_mode) 支持自定义日志文件权限
* 字段权重可为零或负值

* [max_query_time](Searching/Options.md#max_query_time) 现在可影响全表扫描
* 新增[net_wait_tm](Server_settings/Searchd.md#net_wait_tm)、[net_throttle_accept](Server_settings/Searchd.md#net_throttle_accept) 和 [net_throttle_action](Server_settings/Searchd.md#net_throttle_action) 以便微调网络线程（当 workers=thread_pool）

* COUNT DISTINCT 支持 facet 搜索
* IN 能用于 JSON 浮点数组
* 多查询优化不再被整数/浮点表达式破坏
* [SHOW META](Node_info_and_management/SHOW_META.md) 在使用多查询优化时显示 `multiplier` 行
### 编译

Manticore Search 使用 cmake 构建，最低 gcc 版本要求为 4.7.2。
### 文件夹和服务
* Manticore Search 以 `manticore` 用户身份运行。
* 默认数据文件夹为 `/var/lib/manticore/`。
* 默认日志文件夹为 `/var/log/manticore/`。
* 默认 pid 文件夹为 `/var/run/manticore/`。
### Bugfixes
* [Commit a58c](https://github.com/manticoresoftware/manticore/commit/a58c619) 修复了导致 Java 连接器出错的 SHOW COLLATION 语句
* [Commit 631c](https://github.com/manticoresoftware/manticore/commit/631cf4e) 修复处理分布式索引时崩溃；给分布式索引哈希加锁；移除 agent 的移动和复制操作符
* [Commit 942b](https://github.com/manticoresoftware/manticore/commit/942bec0) 修复因并行重连导致的分布式索引处理崩溃
* [Commit e5c1](https://github.com/manticoresoftware/manticore/commit/e5c1ed2) 修复崩溃处理器在存储查询到服务器日志时崩溃
* [Commit 4a4b](https://github.com/manticoresoftware/manticore/commit/4a4bda5) 修复多查询中池化属性导致的崩溃
* [Commit 3873](https://github.com/manticoresoftware/manticore/commit/3873bfb) 通过防止索引页包含进核心文件，缩小了核心文件大小
* [Commit 11e6](https://github.com/manticoresoftware/manticore/commit/11e6254) 修复当指定无效代理时 searchd 启动崩溃问题
* [Commit 4ca6](https://github.com/manticoresoftware/manticore/commit/4ca6350) 修复 indexer 在 sql_query_killlist 查询中报错
* [Commit 123a](https://github.com/manticoresoftware/manticore/commit/123a9f0) 修复 fold_lemmas=1 与命中计数不一致问题
* [Commit cb99](https://github.com/manticoresoftware/manticore/commit/cb99164) 修复 html_strip 行为不一致问题
* [Commit e406](https://github.com/manticoresoftware/manticore/commit/e406761) 修复 optimize rt 索引丢失新设置问题；修复 optimize 使用 sync 选项时锁泄漏问题；

* [Commit 86ae](https://github.com/manticoresoftware/manticore/commit/86aeb82) 修复处理错误多查询问题
* [Commit 2645](https://github.com/manticoresoftware/manticore/commit/2645230) 修复结果集依赖于多查询顺序的问题

