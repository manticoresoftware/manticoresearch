# 更新日志

## 版本 13.11.1
**发布于**：2025年9月13日

### Bug 修复
* 🪲 [v13.11.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.11.1) [问题 #3751](https://github.com/manticoresoftware/manticoresearch/issues/3751) 修复了启用嵌入时删除文档导致的崩溃。

## 版本 13.11.0
**发布于**：2025年9月13日

此版本的主要亮点是[自动嵌入](Searching/KNN.md#Auto-Embeddings-%28Recommended%29) — 这是一项新功能，使语义搜索如同使用 SQL 一样简单。
无需外部服务或复杂管道：只需插入文本并用自然语言搜索。

### 自动嵌入提供的功能

- **直接从文本自动生成嵌入**  
- **自然语言查询**，理解含义，而不仅仅是关键词  
- **支持多种模型**（OpenAI、Hugging Face、Voyage、Jina）  
- **无缝集成** SQL 及 JSON API  

### 推荐库
- 推荐使用的 [MCL](https://github.com/manticoresoftware/columnar) 版本：8.0.3
- 推荐使用的 [Buddy](Installation/Manticore_Buddy.md#Manticore-Buddy) 版本：3.35.1

如果您按照[官方安装指南](https://manticoresearch.com/install/)操作，则无需担心这些。

### 新功能和改进
* 🆕 [v13.11.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.11.0) [合并请求 #3746](https://github.com/manticoresoftware/manticoresearch/pull/3746) 在 JSON 查询中新增了对“query”支持，用于生成嵌入。
* 🆕 [v13.10.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.10.0) [问题 #3709](https://github.com/manticoresoftware/manticoresearch/issues/3709) manticore-server RPM 软件包不再拥有 `/run`。
* 🆕 [v13.9.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.9.0) [合并请求 #3716](https://github.com/manticoresoftware/manticoresearch/pull/3716) 在配置中新增对 `boolean_simplify` 的支持。
* 🆕 [v13.8.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.8.0) [问题 #3253](https://github.com/manticoresoftware/manticoresearch/issues/3253) 安装 sysctl 配置以增加 vm.max_map_count，适用于大型数据集。
* 🆕 [v13.7.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.7.0) [合并请求 #3681](https://github.com/manticoresoftware/manticoresearch/pull/3681) 新增对 `alter table <table> modify column <column> api_key=<key>` 的支持。

### Bug 修复
* 🪲 [v13.10.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.10.5) [合并请求 #3737](https://github.com/manticoresoftware/manticoresearch/pull/3737) 大型64位ID的滚动选项现在能正确返回所有文档。
* 🪲 [v13.10.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.10.4) [合并请求 #3736](https://github.com/manticoresoftware/manticoresearch/pull/3736) 修复了使用带过滤树的 KNN 时的崩溃。
* 🪲 [v13.10.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.10.3) [问题 #3520](https://github.com/manticoresoftware/manticoresearch/issues/3520) `/sql` 端点不再允许执行 SHOW VERSION 命令。
* 🪲 [v13.10.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.10.2) [合并请求 #3637](https://github.com/manticoresoftware/manticoresearch/pull/3637) 更新了 Windows 安装脚本。
* 🪲 [v13.10.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.10.1) 修复了 Linux 上的本地时区检测。
* 🪲 [v13.9.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.9.2) [合并请求 #3726](https://github.com/manticoresoftware/manticoresearch/pull/3726) 列式模式下重复的 ID 现在正确返回错误。
* 🪲 [v13.9.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.9.1) [合并请求 #3333](https://github.com/manticoresoftware/manticoresearch/pull/3333) 手册现已自动翻译。
* 🪲 [v13.8.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.8.6) [合并请求 #3715](https://github.com/manticoresoftware/manticoresearch/pull/3715) 修复了当批次中所有文档被跳过时嵌入生成的问题。
* 🪲 [v13.8.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.8.5) [合并请求 #3711](https://github.com/manticoresoftware/manticoresearch/pull/3711) 新增了 Jina 和 Voyage 嵌入模型，以及其他自动嵌入相关的更改。
* 🪲 [v13.8.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.8.4) [合并请求 #3710](https://github.com/manticoresoftware/manticoresearch/pull/3710) 修复了带多个维度的联合查询中的崩溃。
* 🪲 [v13.8.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.8.3) 修复了在 _bulk 端点中多语句事务的删除/更新提交未计为错误的问题。
* 🪲 [v13.8.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.8.2) [合并请求 #3705](https://github.com/manticoresoftware/manticoresearch/pull/3705) 修复了按非列式字符串属性连接时的崩溃，并改进了错误报告。
* 🪲 [v13.8.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.8.1) [合并请求 #3704](https://github.com/manticoresoftware/manticoresearch/pull/3704) 修复了未指定模型时查询嵌入的崩溃；新增了嵌入字符串用于主-代理通信；新增测试。
* 🪲 [v13.7.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.7.2) [合并请求 #Buddy#589](https://github.com/manticoresoftware/manticoresearch-buddy/pull/589) 移除了模糊搜索的默认 IDF 处理。
* 🪲 [v13.7.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.7.1) [问题 #3454](https://github.com/manticoresoftware/manticoresearch/issues/3454) 修复了大型64位ID时滚动解码错误。
* 🪲 [v13.6.9](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.9) [ Issue #3674](https://github.com/manticoresoftware/manticoresearch/issues/3674) 修复了JDBC+MySQL驱动/连接池在transaction_read_only设置下的问题。
* 🪲 [v13.6.8](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.8) [ PR #3676](https://github.com/manticoresoftware/manticoresearch/pull/3676) 修复了嵌入模型返回空结果集时的崩溃问题。

## 版本 13.6.7
**发布于**：2025年8月8日

推荐的 [MCL](https://github.com/manticoresoftware/columnar) 版本：8.0.1  
推荐的 [Buddy](Installation/Manticore_Buddy.md#Manticore-Buddy) 版本：3.34.2  

如果你遵循[官方安装指南](https://manticoresearch.com/install/)，则无需担心此问题。

### 新功能与改进
* 🆕 [v13.6.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.0) [ Issue #2226](https://github.com/manticoresoftware/manticoresearch/issues/2226) 支持在PHRASE、PROXIMITY和QUORUM运算符中显式使用 '|' （或运算）。
* 🆕 [v13.5.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.5.0) [ PR #3591](https://github.com/manticoresoftware/manticoresearch/pull/3591) 查询中实现自动生成嵌入（进行中，尚未准备好用于生产环境）。
* 🆕 [v13.4.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.4.0) [ PR #3585](https://github.com/manticoresoftware/manticoresearch/pull/3585) 修正了逻辑，优先使用buddy_path配置中设置的buddy线程数，而非守护进程值。
* 🆕 [v13.3.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.3.0) [ PR #3577](https://github.com/manticoresoftware/manticoresearch/pull/3577) 支持与本地分布式表的关联。
* 🆕 [#3647](https://github.com/manticoresoftware/manticoresearch/issues/3647) 增加了Debian 13 "Trixie"的支持。

### Bug修复
* 🪲 [v13.6.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.7) [ Issue #3524](https://github.com/manticoresoftware/manticoresearch/issues/3524) 修复了在行存储中保存生成的嵌入的问题。
* 🪲 [v13.6.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.6) [ Issue #3563](https://github.com/manticoresoftware/manticoresearch/issues/3563) 修复了Sequel Ace及其他集成因“unknown sysvar”错误导致失败的问题。
* 🪲 [v13.6.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.5) [ Issue #3467](https://github.com/manticoresoftware/manticoresearch/issues/3467) 修复了DBeaver及其他集成因“unknown sysvar”错误导致失败的问题。
* 🪲 [v13.6.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.4) [ Issue #3524](https://github.com/manticoresoftware/manticoresearch/issues/3524) 修复了多字段嵌入拼接问题；同时修复了查询中嵌入生成的问题。
* 🪲 [v13.6.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.3) [ Issue #3641](https://github.com/manticoresoftware/manticoresearch/issues/3641) 修复了13.6.0版本中短语丢失除第一个之外所有括号关键字的错误。
* 🪲 [v13.6.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.2) 修复了transform_phrase中的内存泄漏问题。
* 🪲 [v13.6.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.1) 修复了13.6.0版本中的内存泄漏。
* 🪲 [v13.5.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.5.2) [ Issue #3651](https://github.com/manticoresoftware/manticoresearch/issues/3651) 修复了与全文搜索模糊测试相关的更多问题。
* 🪲 [v13.5.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.5.1) [ Issue #3560](https://github.com/manticoresoftware/manticoresearch/issues/3560) 修复了使用KNN数据时OPTIMIZE TABLE可能无限挂起的情况。
* 🪲 [v13.4.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.4.2) [ Issue #2544](https://github.com/manticoresoftware/manticoresearch/issues/2544) 修复了添加float_vector列可能导致索引损坏的问题。
* 🪲 [v13.4.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.4.1) [ Issue #3651](https://github.com/manticoresoftware/manticoresearch/issues/3651) 添加了全文解析的模糊测试，并修复了测试中发现的若干问题。
* 🪲 [v13.3.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.3.1) [ Issue #3583](https://github.com/manticoresoftware/manticoresearch/issues/3583) 修复了使用复杂布尔过滤器并启用高亮时的崩溃问题。
* 🪲 [v13.2.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.7) [ Issue #3481](https://github.com/manticoresoftware/manticoresearch/issues/3481) 修复了同时使用HTTP更新、分布式表和错误的复制集群时崩溃的问题。
* 🪲 [v13.2.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.6) [ PR #3567](https://github.com/manticoresoftware/manticoresearch/pull/3567) 更新了manticore-backup依赖至1.9.6版本。
* 🪲 [v13.2.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.5) [ PR #3565](https://github.com/manticoresoftware/manticoresearch/pull/3565) 修正了CI配置以提升Docker镜像兼容性。
* 🪲 [v13.2.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.4) 修正了长令牌的处理。某些特殊令牌（如正则表达式模式）可能创建过长的词，所以现在在使用前会缩短它们。

## 版本 13.2.3
**发布于**：2025年7月8日

### 重大变更
* ⚠️ [v13.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.0.0) 更新KNN库API以支持空的[float_vector](https://manual.manticoresearch.com/Creating_a_table/Data_types#Float-vector)值。该更新不改变数据格式，但会提升Manticore Search / MCL API版本。
* ⚠️ [v12.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/12.0.0) [ PR #3516](https://github.com/manticoresoftware/manticoresearch/pull/3516) 修复了在 KNN 索引训练和构建过程中源和目标行 ID 不正确的错误。此次更新未更改数据格式，但提升了 Manticore Search / MCL API 版本。
* ⚠️ [v11.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.0.0) 新增支持 KNN 向量搜索的新功能，如量化、重评分和过采样。此版本更改了 KNN 数据格式和[KNN_DIST() SQL 语法](https://manual.manticoresearch.com/Searching/KNN?client=SQL#KNN-vector-search)。新版本可以读取旧数据，但旧版本无法读取新格式。

### 新功能和改进
* 🆕 [v13.2.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.0) [ PR #3549](https://github.com/manticoresoftware/manticoresearch/issues/3525) 修复了 `@@collation_database` 的问题，导致与部分 mysqldump 版本不兼容
* 🆕 [v13.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.1.0) [ Issue #3489](https://github.com/manticoresoftware/manticoresearch/issues/3489) 修复模糊搜索中的一个错误，该错误阻止解析某些 SQL 查询
* 🆕 [v12.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/12.1.0) [ Issue #3426](https://github.com/manticoresoftware/manticoresearch/issues/3426) 新增对 RHEL 10 操作系统的支持
* 🆕 [v11.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.1.0) 新增支持 KNN 搜索中的空浮点向量功能[ KNN-search](https://manual.manticoresearch.com/Searching/KNN#KNN-vector-search)
* 🆕 [v10.2.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.0) [ Issue #3252](https://github.com/manticoresoftware/manticoresearch/issues/3252) [log_level](https://manual.manticoresearch.com/Server_settings/Setting_variables_online#SET) 现在也调整 Buddy 的日志详细级别

### 错误修复
* 🪲 [v13.2.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.3) [ PR #3556](https://github.com/manticoresoftware/manticoresearch/pull/3556) 修正 JSON 查询中 "oversampling" 选项解析问题
* 🪲 [v13.2.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.2)  通过移除 Boost stacktrace 的使用，修复了 Linux 和 FreeBSD 上的崩溃日志记录问题
* 🪲 [v13.2.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.1) [ Issue #3298](https://github.com/manticoresoftware/manticoresearch/issues/3298) 修正容器内运行时的崩溃日志记录问题
* 🪲 [v12.0.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/12.0.2)  通过计数微秒提升了每个表统计的准确性
* 🪲 [v12.0.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/12.0.1) [ PR #3522](https://github.com/manticoresoftware/manticoresearch/pull/3522) 修正了连接查询中按 MVA 分面时的崩溃问题
* 🪲 [v11.0.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.0.3) [ Issue #3502](https://github.com/manticoresoftware/manticoresearch/issues/3502) 修复了与向量搜索量化相关的崩溃
* 🪲 [v11.0.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.0.2) [ Issue #3493](https://github.com/manticoresoftware/manticoresearch/issues/3493) 将 `SHOW THREADS` 中的 CPU 利用率显示更改为整数
* 🪲 [v11.0.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.0.1)  修正列存和二级库的路径
* 🪲 [v10.2.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.7)  新增对 MCL 5.0.5 的支持，包括修复 embeddings 库文件名的问题
* 🪲 [v10.2.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.6) [ Issue #3469](https://github.com/manticoresoftware/manticoresearch/issues/3469) 针对 Issue #3469 进行了另一项修复
* 🪲 [v10.2.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.4) [ Issue #3469](https://github.com/manticoresoftware/manticoresearch/issues/3469) 修复了 HTTP 请求中包含布尔查询时返回空结果的问题
* 🪲 [v10.2.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.5)  更改了 embeddings 库的默认文件名，并增加了 KNN 向量中 'from' 字段的检查
* 🪲 [v10.2.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.3) [ PR #3464](https://github.com/manticoresoftware/manticoresearch/pull/3464) 更新 Buddy 至版本 3.30.2，包含[PR #565 关于内存使用和错误日志](https://github.com/manticoresoftware/manticoresearch-buddy/pull/565)
* 🪲 [v10.2.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.2) [ Issue #3470](https://github.com/manticoresoftware/manticoresearch/pull/3470) 修复了联接查询中 JSON 字符串过滤器、null 过滤器和排序问题
* 🪲 [v10.2.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.1)  修复了 `dist/test_kit_docker_build.sh` 中导致 Buddy 提交缺失的错误
* 🪲 [v10.1.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.4)  修正连接查询中按 MVA 分组导致的崩溃
* 🪲 [v10.1.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.3) [ Issue #3434](https://github.com/manticoresoftware/manticoresearch/issues/3434) 修复了过滤空字符串的错误
* 🪲 [v10.1.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.2) [ PR #3452](https://github.com/manticoresoftware/manticoresearch/pull/3452) 更新 Buddy 至版本 3.29.7，解决了 [Buddy #507 - 多查询请求中使用模糊搜索产生的错误](https://github.com/manticoresoftware/manticoresearch-buddy/issues/507) 以及 [Buddy #561 - 监控指标速率修复](https://github.com/manticoresoftware/manticoresearch-buddy/pull/561)，这是 [Helm Release 10.1.0](https://github.com/manticoresoftware/manticoresearch-helm/releases/tag/manticoresearch-10.1.0) 所必须的。
* 🪲 [v10.1.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.1) 更新 Buddy 至版本 3.29.4，解决了 [#3388 - “未定义的数组键 'Field'”](https://github.com/manticoresoftware/manticoresearch/issues/3388) 和 [Buddy #547 - layouts='ru' 可能无效](https://github.com/manticoresoftware/manticoresearch-buddy/issues/547)

## 版本 10.1.0
**发布于**: 2025年6月9日

此版本代表了带有新功能，一个重大变更以及大量稳定性改进和错误修复的更新。更新内容重点在于增强监控能力、改进搜索功能，并修复了影响系统稳定性和性能的多个关键问题。

**从版本 10.1.0 开始，不再支持 CentOS 7。强烈建议用户升级到支持的操作系统。**

### 重大变更
* ⚠️ [v10.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.0.0) [ 问题 #540](https://github.com/manticoresoftware/manticoresearch-buddy/issues/540) 重大变更：将 [模糊搜索](Searching/Spell_correction.md#Fuzzy-Search) 的默认 `layouts=''`。

### 新功能和改进
* 🆕 [v10.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.0) [ 问题 #537](https://github.com/manticoresoftware/manticoresearch-buddy/issues/537) 添加内置的 [Prometheus 导出器](Node_info_and_management/Node_status.md#Prometheus-Exporter)
* 🆕 [v9.8.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.8.0) [ 问题 #3409](https://github.com/manticoresoftware/manticoresearch/issues/3409) 添加 [ALTER TABLE tbl REBUILD KNN](Updating_table_schema_and_settings.md#Rebuilding-a-KNN-index)
* 🆕 [v9.7.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.7.0) [ 问题 #1778](https://github.com/manticoresoftware/manticoresearch/issues/1778) 添加自动生成嵌入功能（目前尚未正式发布，因为代码已经合并到主分支但还需更多测试）
* 🆕 [v9.6.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.6.0) 提升 KNN API 版本以支持自动嵌入
* 🆕 [v9.5.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.0) [ 问题 #1894](https://github.com/manticoresoftware/manticoresearch/issues/1894) 改进集群恢复：周期性保存 `seqno`，以加快崩溃后节点重启速度
* 🆕 [v9.4.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.4.0) [ 问题 #2400](https://github.com/manticoresoftware/manticoresearch/issues/2400) 添加对最新 [Logstash](Integration/Logstash.md#Integration-with-Logstash) 和 Beats 的支持

### Bug 修复
* 🪲 [v10.0.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.0.1) 修复词形变化处理：用户定义的形式现在覆盖自动生成的形式；添加测试用例至测试22
* 🪲 [v9.8.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.8.2) 修复：再次更新 deps.txt，包含 MCL 中与嵌入库相关的打包修复
* 🪲 [v9.8.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.8.1) 修复：更新 deps.txt，修复 MCL 及嵌入库的打包问题
* 🪲 [v9.7.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.7.3) [ 问题 #3306](https://github.com/manticoresoftware/manticoresearch/issues/3306) 修复索引时导致信号11崩溃的问题
* 🪲 [v9.7.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.7.2) [ 问题 #3109](https://github.com/manticoresoftware/manticoresearch/issues/3109) 修复不存在的 `@@variables` 总是返回0的问题
* 🪲 [v9.7.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.7.1) [ 问题 #3377](https://github.com/manticoresoftware/manticoresearch/issues/3377) 修复与 [remove_repeats()](Functions/Searching_and_ranking_functions.md#REMOVE_REPEATS%28%29) 相关的崩溃问题
* 🪲 [v9.6.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.6.3) [ PR #3411](https://github.com/manticoresoftware/manticoresearch/pull/3411) 修复：使用动态版本检测以支持遥测指标
* 🪲 [v9.6.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.6.2) 修复：[SHOW VERSION](Node_info_and_management/SHOW_VERSION.md#SHOW-VERSION) 输出中的小错误
* 🪲 [v9.6.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.6.1) 修复：创建带有 KNN 属性但无模型的表时导致崩溃
* 🪲 [v9.5.16](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.16) [ 问题 #3342](https://github.com/manticoresoftware/manticoresearch/issues/3342) 修复 `SELECT ... FUZZY=0` 不总是禁用模糊搜索的问题
* 🪲 [v9.5.15](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.15) [ PR #3397](https://github.com/manticoresoftware/manticoresearch/pull/3397) 添加对 [MCL](https://github.com/manticoresoftware/columnar) 4.2.2 的支持；修复了旧存储格式中的错误
* 🪲 [v9.5.14](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.14) [ 问题 #3392](https://github.com/manticoresoftware/manticoresearch/issues/3392) 修复 HTTP JSON 回复中字符串处理错误
* 🪲 [v9.5.13](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.13) [ 问题 #3356](https://github.com/manticoresoftware/manticoresearch/issues/3356) 修复复杂全文检索查询（包含通用子词）导致的崩溃问题
* 🪲 [v9.5.12](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.12) 修正了磁盘块自动刷新错误信息中的拼写错误
* 🪲 [v9.5.11](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.11) [ Issue #3195](https://github.com/manticoresoftware/manticoresearch/issues/3195) 改进了[自动磁盘块刷新](Securing_and_compacting_a_table/Flushing_RAM_chunk_to_disk.md#Flushing-RAM-chunk-to-disk)：如果正在运行优化，则跳过保存
* 🪲 [v9.5.10](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.10) [ Issue #3313](https://github.com/manticoresoftware/manticoresearch/issues/3313) 修复了使用[indextool](Miscellaneous_tools.md#indextool)对实时表中所有磁盘块进行重复ID检查的问题
* 🪲 [v9.5.9](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.9) [ Issue #3132](https://github.com/manticoresoftware/manticoresearch/issues/3132) 在JSON API中添加了对`_random`排序的支持
* 🪲 [v9.5.8](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.8) [ Issue #3382](https://github.com/manticoresoftware/manticoresearch/issues/3382) 修复了通过JSON HTTP API无法使用uint64文档ID的问题
* 🪲 [v9.5.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.7) [ Issue #3385](https://github.com/manticoresoftware/manticoresearch/issues/3385) 修正了使用`id != value`过滤时返回结果不准确的问题
* 🪲 [v9.5.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.6) [ PR #538](https://github.com/manticoresoftware/manticoresearch-buddy/pull/538) 修复了某些情况下模糊匹配的严重bug
* 🪲 [v9.5.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.5) [ Issue #3199](https://github.com/manticoresoftware/manticoresearch/issues/3199) 修复了Buddy HTTP查询参数中对空格的解码（例如，`%20`和`+`）
* 🪲 [v9.5.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.4) [ Issue #3133](https://github.com/manticoresoftware/manticoresearch/issues/3133) 修正了facet搜索中`json.field`上的错误排序
* 🪲 [v9.5.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.3) [ Issue #3091](https://github.com/manticoresoftware/manticoresearch/issues/3091) 修正了SQL与JSON API在分隔符搜索结果不一致的问题
* 🪲 [v9.5.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.2) [ Issue #2819](https://github.com/manticoresoftware/manticoresearch/issues/2819) 性能提升：对分布式表替换使用`DELETE FROM`为`TRUNCATE`
* 🪲 [v9.5.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.1) [ Issue #3080](https://github.com/manticoresoftware/manticoresearch/issues/3080) 修复了使用JSON属性过滤别名`geodist()`时导致崩溃的问题

## 版本 9.3.2
发布：2025年5月2日

此版本包含多项错误修复和稳定性改进，更好的表使用跟踪，以及内存和资源管理的增强。

❤️ 特别感谢[@cho-m](https://github.com/cho-m)修复了Boost 1.88.0的构建兼容性问题，感谢[@benwills](https://github.com/benwills)改进了关于`stored_only_fields`的文档。

* 🪲 [v9.3.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.3.2) 修复了“显示线程”列中CPU活动显示为浮点数而非字符串的问题；同时修复了因数据类型错误导致的PyMySQL结果集解析错误。
* 🪲 [v9.3.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.3.1) [ Issue #3343](https://github.com/manticoresoftware/manticoresearch/issues/3343) 修复了优化过程中断时遗留的`tmp.spidx`文件问题。
* 🆕 [v9.3.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.3.0) [ PR #3337](https://github.com/manticoresoftware/manticoresearch/pull/3337) 新增了每张表的命令计数器及详细的表使用统计。
* 🪲 [v9.2.39](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.39) [ Issue #3236](https://github.com/manticoresoftware/manticoresearch/issues/3236) 修复：防止通过移除复杂块更新导致表损坏。由于可等待函数在串行工作线程内部使用，打破了串行处理，可能损坏表。
	重新实现了自动刷新。移除了外部轮询队列以避免不必要的表锁。添加了“小表”条件：如果文档数量低于“小表限制”（8192），且未使用次级索引（SI），则跳过刷新。

* 🪲 [v9.2.38](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.38) 修复：跳过为使用`ALL`/`ANY`字符串列表过滤器创建次级索引（SI），不影响JSON属性。
* 🪲 [v9.2.37](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.37) [ Issue #2898](https://github.com/manticoresoftware/manticoresearch/issues/2898) 为系统表添加了反引号支持。
* 🪲 [v9.2.36](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.36) 修复：在旧代码中使用聚类操作时用占位符。在解析器中，我们现在清晰分隔了表名和聚类名字段。
* 🪲 [v9.2.35](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.35) 修复：在取消引用单个`'`时崩溃的问题。
* 🪲 [v9.2.34](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.34) [ Issue #3090](https://github.com/manticoresoftware/manticoresearch/issues/3090) 修复：大文档ID处理（之前可能无法找到）。
* 🪲 [v9.2.33](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.33) 修复：位向量大小使用无符号整数。
* 🪲 [v9.2.32](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.32) 修复：减少合并时的峰值内存使用。docid到rowid的查找现使用12字节/文档，之前为16字节。示例：2十亿文档使用24GB内存，而不是36GB。
* 🪲 [v9.2.31](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.31) [ Issue #3238](https://github.com/manticoresoftware/manticoresearch/issues/3238) 修复：大型实时表中 `COUNT(*)` 值不正确的问题。
* 🪲 [v9.2.30](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.30)  修复：清零字符串属性时的未定义行为。
* 🪲 [v9.2.29](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.29)  小修正：改进了警告文本。
* 🪲 [v9.2.28](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.28) [ Issue #3290](https://github.com/manticoresoftware/manticoresearch/issues/3290) 改进：增强了 `indextool --buildidf`
* 🪲 [v9.2.27](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.27) [ Issue #3032](https://github.com/manticoresoftware/manticoresearch/issues/3032) 通过 Kafka 集成，现在可以为特定 Kafka 分区创建数据源。
* 🪲 [v9.2.26](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.26) [ Issue #3301](https://github.com/manticoresoftware/manticoresearch/issues/3301) 修复：`ORDER BY` 和 `WHERE` 应用于 `id` 时可能导致内存溢出（OOM）错误。
* 🪲 [v9.2.25](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.25) [ Issue #3171](https://github.com/manticoresoftware/manticoresearch/issues/3171) 修复：当 RT 表有多个磁盘块时，使用带有多个 JSON 属性的 grouper 导致段错误崩溃。
* 🪲 [v9.2.24](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.24) [ Issue #3246](https://github.com/manticoresoftware/manticoresearch/issues/3246) 修复：RAM 块刷新后，`WHERE string ANY(...)` 查询失败。
* 🪲 [v9.2.23](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.23) [ PR #518](https://github.com/manticoresoftware/manticoresearch-buddy/pull/518) 小幅自动分片语法改进。
* 🪲 [v9.2.22](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.22) [ Issue #2763](https://github.com/manticoresoftware/manticoresearch/issues/2763) 修复：使用 `ALTER TABLE` 时未加载全局 idf 文件。
* 🪲 [v9.2.21](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.21)  修复：全局 idf 文件可能很大。我们现在更早释放表，避免持有不必要的资源。
* 🪲 [v9.2.20](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.20) [ PR #3277](https://github.com/manticoresoftware/manticoresearch/pull/3277) 改进：更好的分片选项验证。

* 🪲 [v9.2.19](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.19) [ PR #3275](https://github.com/manticoresoftware/manticoresearch/pull/3275) 修复：兼容 Boost 1.88.0 的构建。
* 🪲 [v9.2.18](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.18) [ Issue #3228](https://github.com/manticoresoftware/manticoresearch/issues/3228) 修复：分布式表创建期间崩溃（无效指针问题）。

* 🪲 [v9.2.17](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.17) [ PR #3272](https://github.com/manticoresoftware/manticoresearch/pull/3272) 修复：多行模糊 `FACET` 问题。
* 🪲 [v9.2.16](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.16) [ Issue #3063](https://github.com/manticoresoftware/manticoresearch/issues/3063) 修复：使用 `GEODIST` 函数时距离计算错误。
* 🪲 [v9.2.15](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.15) [ Issue #3027](https://github.com/manticoresoftware/manticoresearch/issues/3027) 小幅改进：支持 Elastic 的 `query_string` 过滤器格式。

## Version 9.2.14
Released: March 28th 2025

### Minor changes
* [Commit ](https://github.com/manticoresoftware/manticoresearch/commit/fe9473dc3f4fcfa0eaae748b538fe853f4ce078b) 实现了 `--mockstack` 标志来计算递归操作堆栈需求。新的 `--mockstack` 模式分析并报告递归表达式评估、模式匹配操作、过滤处理所需的堆栈大小。计算结果输出到控制台以方便调试和优化。
* [Issue #3058](https://github.com/manticoresoftware/manticoresearch/pull/3058) 默认启用 [boolean_simplify](Searching/Options.md#boolean_simplify)。
* [Issue #3172](https://github.com/manticoresoftware/manticoresearch/issues/3172) 添加新配置选项：`searchd.kibana_version_string`，当将 Manticore 与期望特定 Elasticsearch 版本的 Kibana 或 OpenSearch Dashboards 结合使用时非常有用。
* [Issue #3211](https://github.com/manticoresoftware/manticoresearch/issues/3211) 修复 [CALL SUGGEST](Searching/Spell_correction.md#CALL-QSUGGEST,-CALL-SUGGEST) 以支持 2 字符单词。
* [Issue #490](https://github.com/manticoresoftware/manticoresearch-buddy/issues/490) 改进了 [模糊搜索](Searching/Spell_correction.md#Fuzzy-Search)：之前在搜索“def ghi”时，如果存在另一个匹配文档，有时无法找到“defghi”。
* ⚠️ BREAKING [Issue #3165](https://github.com/manticoresoftware/manticoresearch/issues/3165) 为了一致性，在某些 HTTP JSON 响应中将 `_id` 改为 `id`。请确保相应更新您的应用程序。
* ⚠️ 重大更改 [Issue #3186](https://github.com/manticoresoftware/manticoresearch/issues/3186) 在集群加入时添加了对 `server_id` 的检查，以确保每个节点具有唯一的ID。当加入的节点与集群中现有节点具有相同的 `server_id` 时，`JOIN CLUSTER` 操作现在可能会失败并提示重复的 [server_id](Server_settings/Searchd.md#server_id)。为解决此问题，请确保复制集群中的每个节点具有唯一的 [server_id](Server_settings/Searchd.md#server_id)。您可以在配置文件的 "searchd" 部分将默认的 [server_id](Server_settings/Searchd.md#server_id) 更改为唯一值，然后再尝试加入集群。此更改更新了复制协议。如果您正在运行复制集群，您需要：
  - 首先，干净地停止所有节点
  - 然后，使用 Linux 中的工具 `manticore_new_cluster`，带 `--new-cluster` 参数启动最后停止的节点。
  - 更多详情请阅读关于 [重启集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster) 。

### Bug修复
* [Commit 6fda](https://github.com/manticoresoftware/manticoresearch/commit/6fdad3923dd6953f4b781943eed5ec5c28b7f808) 修复了等待后调度器丢失导致的崩溃；现在能够正确恢复如 `serializer` 之类的特定调度器。
* [Commit c333](https://github.com/manticoresoftware/manticoresearch/commit/c333b277d4a504de5fe298e9ab570a9f9ea31e0a) 修复了一个bug，导致右侧联接表中的权重无法在 `ORDER BY` 子句中使用。
* [Issue #2644](https://github.com/manticoresoftware/manticoresearch/issues/2644) gcc 14.2.0：修正了对 `const knn::DocDist_t*&` 使用 `lower_bound` 调用错误。❤️ 感谢 [@Azq2](https://github.com/Azq2) 的PR。
* [Issue #3018](https://github.com/manticoresoftware/manticoresearch/issues/3018) 修复了自动模式插入时处理大写表名的问题。
* [Issue #3119](https://github.com/manticoresoftware/manticoresearch/issues/3119) 修复了解码无效base64输入时的崩溃。
* [Issue #3121](https://github.com/manticoresoftware/manticoresearch/issues/3121) 修复了两个与 KNN 索引相关的 ALTER 问题：浮点向量现在保持其原始维度，KNN 索引现在正确生成。
* [Issue #3123](https://github.com/manticoresoftware/manticoresearch/issues/3123) 修复了在空JSON列上构建二级索引时崩溃的问题。
* [Issue #3138](https://github.com/manticoresoftware/manticoresearch/issues/3138) 修复了由重复条目导致的崩溃。
* [Issue #3151](https://github.com/manticoresoftware/manticoresearch/issues/3151) 修复了 `fuzzy=1` 选项无法与 `ranker` 或 `field_weights` 一起使用的问题。
* [Issue #3163](https://github.com/manticoresoftware/manticoresearch/issues/3163) 修复了 `SET GLOBAL timezone` 无效的 bug。
* [Issue #3181](https://github.com/manticoresoftware/manticoresearch/issues/3181) 修复了使用超过 2^63 的 ID 时文本字段值可能丢失的问题。
* [Issue #3189](https://github.com/manticoresoftware/manticoresearch/issues/3189) 修复了 `UPDATE` 语句现在正确遵守 `query_log_min_msec` 设置的问题。
* [Issue #3247](https://github.com/manticoresoftware/manticoresearch/issues/3247) 修复了保存实时磁盘块时的竞争条件，该问题可能导致 `JOIN CLUSTER` 失败。


## 版本 7.4.6
发布时间：2025年2月28日

### 重大变更
* [Issue #832](https://github.com/manticoresoftware/manticoresearch/issues/832) 集成了 [Kibana](Integration/Kibana.md)，实现更便捷高效的数据可视化。

### 小改动
* [Issue #1727](https://github.com/manticoresoftware/manticoresearch/issues/1727) 修复了 arm64 和 x86_64 之间的浮点数精度差异。
* [Issue #2995](https://github.com/manticoresoftware/manticoresearch/issues/2995) 实现了联接批处理的性能优化。
* [Issue #3039](https://github.com/manticoresoftware/manticoresearch/issues/3039) 实现了直方图中 EstimateValues 的性能优化。
* [Issue #3099](https://github.com/manticoresoftware/manticoresearch/issues/3099) 添加了对 Boost 1.87.0 的支持。❤️ 感谢 [@cho-m](https://github.com/cho-m) 的PR。
* [Issue #77](https://github.com/manticoresoftware/columnar/issues/77) 优化了多值过滤器创建时的块数据重用；向属性元数据添加了最小/最大值；实现了基于最小/最大值的过滤值预过滤。

### Bug修复
* [Commit 73ac](https://github.com/manticoresoftware/manticoresearch/commit/73ac22f358a7a0e734b332c3943c86e6294c06c3) 修复了在使用左右两侧表属性的联接查询中表达式的处理；修正了右侧表的 index_weights 选项。
* [Issue #2915](https://github.com/manticoresoftware/manticoresearch/issues/2915) 使用 `avg()` 在 `SELECT ... JOIN` 查询中可能导致错误结果，现已修复。
* [Issue #2996](https://github.com/manticoresoftware/manticoresearch/issues/2996) 修复了联接批处理启用时隐式截止导致结果集错误的问题。
* [Issue #3031](https://github.com/manticoresoftware/manticoresearch/issues/3031) 修复了停止后台守护进程时，当有活动块合并过程中发生的崩溃。
* [Issue #3037](http://github.com/manticoresoftware/manticoresearch/issues/3037) 修复了 `IN(...)` 可能产生错误结果的问题。
* [Issue #3038](https://github.com/manticoresoftware/manticoresearch/issues/3038) 在版本7.0.0中设置 `max_iops` / `max_iosize` 可能导致索引性能下降，此问题现已修复。
* [Issue #3042](https://github.com/manticoresoftware/manticoresearch/issues/3042) 修复了联接查询缓存的内存泄漏。
* [Issue #3052](https://github.com/manticoresoftware/manticoresearch/issues/3052) 修复了联接JSON查询中的查询选项的处理。
* [Issue #3054](https://github.com/manticoresoftware/manticoresearch/issues/3054) 修复了 ATTACH TABLE 命令的问题。
* [问题 #3079](https://github.com/manticoresoftware/manticoresearch/issues/3079) 修正了错误信息中的不一致性。
* [问题 #3087](https://github.com/manticoresoftware/manticoresearch/issues/3087) 针对每个表设置 `diskchunk_flush_write_timeout=-1` 未能禁用索引刷新；现已修复。
* [问题 #3088](https://github.com/manticoresoftware/manticoresearch/issues/3088) 解决了大 ID 批量替换后出现的重复条目问题。
* [问题 #3126](https://github.com/manticoresoftware/manticoresearch/issues/3126) 修复了由带单个 `NOT` 运算符且带表达式排名器的全文搜索查询引起的守护进程崩溃。
* [问题 #3128](https://github.com/manticoresoftware/manticoresearch/pull/3128) 修复了 CJSON 库中的潜在漏洞。❤️ 感谢 [@tabudz](https://github.com/tabudz) 的 PR。

## 版本 7.0.0
发布时间：2025 年 1 月 30 日

### 主要变更
* [问题 #1497](https://github.com/manticoresoftware/manticoresearch/issues/1497) 新增了更便利的[模糊搜索](Searching/Spell_correction.md#Fuzzy-Search)和[自动完成](Searching/Autocomplete.md#CALL-AUTOCOMPLETE)功能。
* [问题 #1500](https://github.com/manticoresoftware/manticoresearch/issues/1500) 实现了与 Kafka 的[集成](Integration/Kafka.md#Syncing-from-Kafka)。
* [问题 #1928](https://github.com/manticoresoftware/manticoresearch/issues/1928) 新增了针对 JSON 的[二级索引](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#json_secondary_indexes)。
* [问题 #2361](https://github.com/manticoresoftware/manticoresearch/issues/2361) 更新及更新期间的搜索不再被块合并阻塞。
* [问题 #2787](https://github.com/manticoresoftware/manticoresearch/issues/2787) 针对实时表增加自动[磁盘块刷新](Server_settings/Searchd.md#diskchunk_flush_write_timeout)以提升性能；现会自动将内存块刷新到磁盘块，防止因内存块缺乏优化而导致的性能问题，这种差异有时会引起不稳定，具体依赖于块大小。
* [问题 #2811](https://github.com/manticoresoftware/manticoresearch/issues/2811) 新增更简便的分页[滚动搜索](Searching/Pagination.md#Scroll-Search-Option)选项。
* [问题 #931](https://github.com/manticoresoftware/manticoresearch/issues/931) 集成了 [Jieba](https://github.com/fxsjy/jieba)，提升了[中文分词](Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md)效果。

### 次要变更
* ⚠️ 破坏性变更 [问题 #1111](https://github.com/manticoresoftware/manticoresearch/issues/1111) 修正了实时表对 `global_idf` 的支持。需要重新创建表。
* ⚠️ 破坏性变更 [问题 #2103](https://github.com/manticoresoftware/manticoresearch/issues/2103) 从内部 `cjk` 字符集移除了泰文字符。请相应更新字符集定义：如果你使用了 `cjk,non_cjk` 并且需要支持泰文，请改为 `cjk,thai,non_cjk`，或者 `cont,non_cjk`，其中 `cont` 是对所有连续书写语言（即 `cjk` + `thai`）的新命名。可通过[ALTER TABLE](Updating_table_schema_and_settings.md#Updating-table-FT-settings-in-RT-mode) 修改现有表。
* ⚠️ 破坏性变更 [问题 #2468](https://github.com/manticoresoftware/manticoresearch/issues/2468) [CALL SUGGEST / QSUGGEST](Searching/Spell_correction.md#CALL-QSUGGEST,-CALL-SUGGEST) 现已支持分布式表。这将提升主/代理协议版本。如果你在分布式环境运行 Manticore Search 多实例，务必先升级代理端，再升级主服务器。
* ⚠️ 破坏性变更 [问题 #2889](https://github.com/manticoresoftware/manticoresearch/issues/2889) 将 PQ [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META-for-PQ-tables) 的列名由 `Name` 改为 `Variable name`。
* ⚠️ 破坏性变更 [问题 #879](https://github.com/manticoresoftware/manticoresearch/issues/879) 引入了[每表二进制日志](Logging/Binary_logging.md#Per-table-binary-logging-configuration)功能，添加了新选项：[binlog_common](Logging/Binary_logging.md#Binary-logging-strategies)、[binlog](Logging/Binary_logging.md#Per-table-binary-logging-configuration)，适用于 `create table` / `alter table`。升级到新版本前，需先完整关闭 Manticore 实例。
* ⚠️ 破坏性变更 [问题 #1789](https://github.com/manticoresoftware/manticoresearch/issues/1789) 修正了当节点使用错误的复制协议版本加入集群时出现的错误提示。此变更更新了复制协议。若使用复制集群，请执行：
  - 先干净关闭所有节点
  - 再使用 Linux 下工具 `manticore_new_cluster` 启动最后停止的节点，并使用 `--new-cluster` 参数。
  - 详细步骤请参见[重启集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster)。
* ⚠️ 破坏性变更 [问题 #2308](https://github.com/manticoresoftware/manticoresearch/issues/2308) 增加了 [`ALTER CLUSTER ADD` 和 `DROP`](Creating_a_cluster/Setting_up_replication/Adding_and_removing_a_table_from_a_replication_cluster.md#Adding-and-removing-a-table-from-a-replication-cluster) 对多表的支持。此改动也影响复制协议，处理建议参照上节说明。
* [问题 #2997](https://github.com/manticoresoftware/manticoresearch/issues/2997) 修复了 MacOS 上的 dlopen 问题。
* [提交 4954](https://github.com/manticoresoftware/manticoresearch/commit/4954b5de7341a29902a9b8fbb9a040f7942c77c4) 修改了 KNN 索引表的默认 OPTIMIZE TABLE 截断值以提升搜索性能。
* [提交 cfc8](https://github.com/manticoresoftware/manticoresearch/commit/cfc87ecb6e33a8163c2235243b6b40e699dbf526) 为 `FACET` 和 `GROUP BY` 中的 `ORDER BY` 添加了对 `COUNT(DISTINCT)` 的支持。
* [问题 #1103](https://github.com/manticoresoftware/manticoresearch/issues/1103) 提升了[日志](Logging/Server_logging.md#Server-logging)中块合并的清晰度。
* [Issue #1130](https://github.com/manticoresoftware/manticoresearch/issues/1130) 添加了对 [DBeaver](Integration/DBeaver.md) 的支持。
* [Issue #1546](https://github.com/manticoresoftware/manticoresearch/issues/1546) 实现了 [POLY2D()](Functions/Geo_spatial_functions.md#POLY2D%28%29)/[GEOPOLY2D()](Functions/Geo_spatial_functions.md#GEOPOLY2D%28%29) 函数的二级索引。
* [Issue #1630](https://github.com/manticoresoftware/manticoresearch/issues/1630) HTTP 请求现支持 `Content-Encoding: gzip`。
* [Issue #1831](https://github.com/manticoresoftware/manticoresearch/issues/1831) 添加了 `SHOW LOCKS` 命令。
* [Issue #2187](https://github.com/manticoresoftware/manticoresearch/issues/2197) 允许 Buddy 请求守护进程时绕过 [searchd.max_connections](Server_settings/Searchd.md#max_connections) 限制。
* [Issue #2208](https://github.com/manticoresoftware/manticoresearch/issues/2208) 添加了通过 JSON HTTP 接口连接表的支持。
* [Issue #2235](https://github.com/manticoresoftware/manticoresearch/issues/2235) 通过 Buddy 以原始形式记录成功处理的查询。
* [Issue #2249](https://github.com/manticoresoftware/manticoresearch/issues/2249) 为复制表运行 `mysqldump` 添加了特殊模式。
* [Issue #2268](https://github.com/manticoresoftware/manticoresearch/issues/2268) 改进了 `CREATE TABLE` 和 `ALTER TABLE` 语句中外部文件的重命名复制处理。
* [Issue #2402](https://github.com/manticoresoftware/manticoresearch/issues/2402) 更新了 [searchd.max_packet_size](Server_settings/Searchd.md#max_packet_size) 默认值为 128MB。
* [Issue #2419](https://github.com/manticoresoftware/manticoresearch/issues/2419) 添加了 JSON ["match"](Searching/Full_text_matching/Basic_usage.md#match) 中对 [IDF boost modifier](Searching/Full_text_matching/Operators.md#IDF-boost-modifier) 的支持。
* [Issue #2430](https://github.com/manticoresoftware/manticoresearch/issues/2430) 增强了 [binlog](Logging/Binary_logging.md#Binary-logging) 写入同步以防止错误。
* [Issue #2458](https://github.com/manticoresoftware/manticoresearch/issues/2458) 在 Windows 包中集成了 zlib 支持。
* [Issue #2479](https://github.com/manticoresoftware/manticoresearch/issues/2479) 添加了 SHOW TABLE INDEXES 命令的支持。
* [Issue #2485](https://github.com/manticoresoftware/manticoresearch/issues/2485) 设置了 Buddy 回复的会话元数据。
* [Issue #2490](https://github.com/manticoresoftware/manticoresearch/issues/2490) 兼容端点的聚合时间精度提升至毫秒级。
* [Issue #2500](https://github.com/manticoresoftware/manticoresearch/issues/2500) 修改了集群操作中复制启动失败时的错误提示。
* [Issue #2584](https://github.com/manticoresoftware/manticoresearch/pull/2584) 在 SHOW STATUS 中新增了[性能指标](Node_info_and_management/Node_status.md#Query-Time-Statistics)：最近 1、5 和 15 分钟内每种查询类型的最小/最大/平均/第 95/99 百分位响应时间。
* [Issue #2639](https://github.com/manticoresoftware/manticoresearch/pull/2639) 请求和响应中的所有 `index` 一律替换为 `table`。
* [Issue #2643](https://github.com/manticoresoftware/manticoresearch/issues/2643) 为 HTTP `/sql` 端点的聚合结果增加了 `distinct` 列。
* [Issue #268](https://github.com/manticoresoftware/manticoresearch-buddy/issues/268) 实现了 Elasticsearch 导入数据类型的自动检测。
* [Issue #2744](https://github.com/manticoresoftware/manticoresearch/issues/2744) 为字符串 JSON 字段比较表达式添加了排序规则支持。
* [Issue #2752](https://github.com/manticoresoftware/manticoresearch/issues/2752) 添加了对选择列表中 `uuid_short` 表达式的支持。
* [Issue #2783](https://github.com/manticoresoftware/manticoresearch/issues/2783) Manticore Search 现在直接运行 Buddy，无需 `manticore-buddy` 包装器。
* [Issue #2785](https://github.com/manticoresoftware/manticoresearch/issues/2785) 针对丢失表和不支持插入操作的表区分错误消息。
* [Issue #2789](https://github.com/manticoresoftware/manticoresearch/issues/2789) OpenSSL 3 现在静态集成到 `searchd` 中。
* [Issue #2790](https://github.com/manticoresoftware/manticoresearch/issues/2790) 添加了 `CALL uuid_short` 语句以生成包含多个 `uuid_short` 值的序列。
* [Issue #2803](https://github.com/manticoresoftware/manticoresearch/issues/2803) 为 JOIN 操作中的右表添加了单独的选项。
* [Issue #2810](https://github.com/manticoresoftware/manticoresearch/issues/2810) 改进了 HTTP JSON 聚合性能，使其匹配 SphinxQL 中的 `GROUP BY`。
* [Issue #2854](https://github.com/manticoresoftware/manticoresearch/issues/2854) 添加了对 Kibana 日期相关请求中 `fixed_interval` 的支持。
* [Issue #2909](https://github.com/manticoresoftware/manticoresearch/issues/2909) 实现了 JOIN 查询的批处理，大幅提升某些 JOIN 查询的性能，速度提升数百至数千倍。
* [Issue #2937](https://github.com/manticoresoftware/manticoresearch/issues/2937) 启用了全扫描查询中对连接表权重的使用。
* [Issue #2953](https://github.com/manticoresoftware/manticoresearch/issues/2953) 修复了连接查询的日志记录问题。
* [Issue #337](https://github.com/manticoresoftware/manticoresearch-buddy/issues/337) 在非调试模式下隐藏了 Buddy 异常，不再输出到 `searchd` 日志。
* [Issue #2931](https://github.com/manticoresoftware/manticoresearch/issues/2931) 当用户为复制监听器设置错误端口时，守护进程会带错误信息关闭。

### Bug 修复
* [Commit 0c6b](https://github.com/manticoresoftware/manticoresearch/commit/0c6bdaf41ea3684e05952e1a8273893535b923f5) 修复：包含超过 32 列的 JOIN 查询返回错误结果。
* [Issue #2335](https://github.com/manticoresoftware/manticoresearch/issues/2335) 解决了使用两个 json 属性条件时表连接失败的问题。
* [Issue #2338](https://github.com/manticoresoftware/manticoresearch/issues/2338) 修正了带有 [cutoff](Searching/Options.md#cutoff) 的多查询中的错误 total_relation。
* [Issue #2366](https://github.com/manticoresoftware/manticoresearch/issues/2366) 修正了在 [表连接](Searching/Joining.md) 中对右表的 `json.string` 过滤。
* [Issue #2406](https://github.com/manticoresoftware/manticoresearch/issues/2406) 允许在任何 POST HTTP JSON 端点（insert/replace/bulk）中对所有值使用 `null`。在此情况下使用默认值。
* [Issue #2418](https://github.com/manticoresoftware/manticoresearch/issues/2418) 通过调整初始套接字探测的 [max_packet_size](Server_settings/Searchd.md#max_packet_size) 网络缓冲区分配，优化了内存消耗。
* [Issue #2420](https://github.com/manticoresoftware/manticoresearch/issues/2420) 修复了通过 JSON 接口将无符号整型插入 bigint 属性的问题。
* [Issue #2422](https://github.com/manticoresoftware/manticoresearch/issues/2422) 修正了二级索引在启用 exclude 过滤器和 pseudo_sharding 时的正确工作。
* [Issue #2423](https://github.com/manticoresoftware/manticoresearch/issues/2423) 解决了 [manticore_new_cluster](Starting_the_server/Manually.md#searchd-command-line-options) 中的一个错误。
* [Issue #2448](https://github.com/manticoresoftware/manticoresearch/issues/2448) 修复了错误格式 `_update` 请求导致的守护进程崩溃。
* [Issue #2452](https://github.com/manticoresoftware/manticoresearch/issues/2452) 解决了直方图无法处理包含排除项的值过滤器的问题。
* [Issue #55](https://github.com/manticoresoftware/columnar/issues/55) 修复了针对分布式表的 [knn](Searching/KNN.md#KNN-vector-search) 查询。
* [Issue #68](https://github.com/manticoresoftware/columnar/issues/68) 增强了列访问器中对表编码上的排除过滤器的处理。
* [Commit 0eb1](https://github.com/manticoresoftware/manticoresearch/commit/0eb1579462013cb134bcaf74e06f6eeb0c2ecc4d) 修复了表达式解析器未遵守重新定义 `thread_stack` 的问题。
* [Commit c304](https://github.com/manticoresoftware/manticoresearch/commit/c3041fd1d3e14773f3c5aa635343915a71f5cb95) 修复了克隆列式 IN 表达式时的崩溃。
* [Commit edad](https://github.com/manticoresoftware/columnar/commit/edadc694c68d6022bdd13134263667430a42cc1d) 修复了位图迭代器中的取反问题，该问题导致崩溃。
* [Commit fc30](https://github.com/manticoresoftware/executor/commit/fc302ff1117b0b835a6f5a3c07057baf5fba14d9) 修复了某些 Manticore 软件包被 `unattended-upgrades` 自动移除的问题。
* [Issue #1019](https://github.com/manticoresoftware/manticoresearch/issues/1019) 改进了对 DbForge MySQL 工具生成查询的处理。
* [Issue #1107](https://github.com/manticoresoftware/manticoresearch/issues/1107) 修正了 `CREATE TABLE` 和 `ALTER TABLE` 中对特殊字符的转义。❤️ 感谢 [@subnix](https://github.com/subnix) 的 PR。
* [Issue #116](https://github.com/manticoresoftware/manticoresearch-backup/issues/116#issuecomment-2216597206) 修复了在冻结索引中更新 blob 属性时发生的死锁。该死锁由于尝试解冻索引时锁冲突引起，也可能导致 manticore-backup 失败。
* [Issue #1818](https://github.com/manticoresoftware/manticoresearch/issues/1818) 当表被冻结时，`OPTIMIZE` 现在会抛出错误。
* [Issue #2001](https://github.com/manticoresoftware/manticoresearch/issues/2001) 允许函数名作为列名使用。
* [Issue #2153](https://github.com/manticoresoftware/manticoresearch/issues/2153) 修复查询表设置时因未知磁盘块导致守护进程崩溃。
* [Issue #2184](https://github.com/manticoresoftware/manticoresearch/issues/2184) 修正 `searchd` 在执行 `FREEZE` 和 `FLUSH RAMCHUNK` 后停止时挂起的问题。
* [Issue #2228](https://github.com/manticoresoftware/manticoresearch/issues/2228) 从保留字中移除了日期/时间相关的标记（及正则表达式）。
* [Issue #2255](https://github.com/manticoresoftware/manticoresearch/issues/2255) 修复了在 `FACET` 使用超过 5 个排序字段时的崩溃。
* [Issue #2265](https://github.com/manticoresoftware/manticoresearch/issues/2265) 修复启用 `index_field_lengths` 后恢复 `mysqldump` 失败的问题。
* [Issue #2291](https://github.com/manticoresoftware/manticoresearch/issues/2291) 修复执行 `ALTER TABLE` 命令时的崩溃。
* [Issue #2333](https://github.com/manticoresoftware/manticoresearch/issues/2333) 修正 Windows 包中索引器的 MySQL DLL 以正常工作。
* [Issue #2393](https://github.com/manticoresoftware/manticoresearch/issues/2393) 修复了 GCC 编译错误。❤️ 感谢 [@animetosho](https://github.com/animetosho) 的 PR。
* [Issue #2447](https://github.com/manticoresoftware/manticoresearch/issues/2447) 修正了 [_update](Data_creation_and_modification/Updating_documents/REPLACE.md?client=Elasticsearch-like+partial) 中的转义问题。
* [Issue #2460](https://github.com/manticoresoftware/manticoresearch/issues/2460) 修复在声明多个同名属性或字段时索引器崩溃。
* [Issue #2467](https://github.com/manticoresoftware/manticoresearch/issues/2467) 解决了针对 "compat" 搜索相关端点的嵌套布尔查询错误转换导致的守护进程崩溃。
* [Issue #2493](https://github.com/manticoresoftware/manticoresearch/issues/2493) 修正带修饰符短语中的扩展。
* [Issue #2535](https://github.com/manticoresoftware/manticoresearch/issues/2535) 解决使用 [ZONE](Searching/Full_text_matching/Operators.md#ZONE-limit-operator) 或 [ZONESPAN](Searching/Full_text_matching/Operators.md#ZONESPAN-limit-operator) 全文操作符导致守护进程崩溃。
* [Issue #2552](https://github.com/manticoresoftware/manticoresearch/issues/2552) 修复带关键词字典的普通和 RT 表的中缀生成。
* [Issue #2571](https://github.com/manticoresoftware/manticoresearch/issues/2571) 修复了 `FACET` 查询中的错误回复；为使用 `COUNT(*)` 的 `FACET` 设置默认排序顺序为 `DESC`。
* [Issue #2580](https://github.com/manticoresoftware/manticoresearch/issues/2580) 修复了 Windows 启动时守护进程崩溃的问题。
* [Issue #2603](https://github.com/manticoresoftware/manticoresearch/issues/2603) 修复了 HTTP 端点 `/sql` 和 `/sql?mode=raw` 的查询截断问题；使这些端点的请求一致，无需 `query=` 头。
* [Issue #2623](https://github.com/manticoresoftware/manticoresearch/issues/2623) 修复了自动架构创建表但同时失败的问题。
* [Issue #2627](https://github.com/manticoresoftware/manticoresearch/issues/2627) 修复了 HNSW 库以支持加载多个 KNN 索引。
* [Issue #2630](https://github.com/manticoresoftware/manticoresearch/issues/2630) 修复了多条件同时发生时的冻结问题。
* [Issue #2645](https://github.com/manticoresoftware/manticoresearch/issues/2645) 修复了使用 `OR` 与 KNN 搜索时发生致命错误崩溃的问题。
* [Issue #2647](https://github.com/manticoresoftware/manticoresearch/issues/2647) 修复了 `indextool --mergeidf *.idf --out global.idf` 创建输出文件后删除该文件的问题。
* [Issue #2658](https://github.com/manticoresoftware/manticoresearch/issues/2658) 修复了带有外层 `ORDER BY` 字符串的子查询导致守护进程崩溃的问题。
* [Issue #2686](https://github.com/manticoresoftware/manticoresearch/issues/2686) 修复了更新浮点属性和字符串属性同时进行时的崩溃问题。
* [Issue #2704](https://github.com/manticoresoftware/manticoresearch/issues/2704) 修复了来自 `lemmatize_xxx_all` 分词器的多个停用词增加随后词的 `hitpos` 的问题。
* [Issue #2708](https://github.com/manticoresoftware/manticoresearch/issues/2708) 修复了执行 `ALTER ... ADD COLUMN ... TEXT` 时的崩溃。
* [Issue #2737](https://github.com/manticoresoftware/manticoresearch/issues/2737) 修复了在带有至少一个 RAM 分块的冻结表中更新 blob 属性后，随后的 `SELECT` 查询需等待表解除冻结的问题。
* [Issue #2742](https://github.com/manticoresoftware/manticoresearch/issues/2742) 修复了带有打包因子的查询跳过缓存的问题。
* [Issue #2775](https://github.com/manticoresoftware/manticoresearch/issues/2775) Manticore 现在对未知操作报错，而不是在 `_bulk` 请求时崩溃。
* [Issue #2791](https://github.com/manticoresoftware/manticoresearch/issues/2791) 修复了 HTTP `_bulk` 端点插入文档 ID 返回问题。
* [Issue #2797](https://github.com/manticoresoftware/manticoresearch/issues/2797) 修复了处理多个表时出现崩溃，其中一个表为空，另一个表匹配条目数不同。
* [Issue #2835](https://github.com/manticoresoftware/manticoresearch/issues/2835) 修复了复杂 `SELECT` 查询中的崩溃。
* [Issue #2872](https://github.com/manticoresoftware/manticoresearch/issues/2872) 添加了错误提示，若 `IN` 表达式中的 `ALL` 或 `ANY` 参数不是 JSON 属性。
* [Issue #2882](https://github.com/manticoresoftware/manticoresearch/issues/2882) 修复了大型表中更新 MVA 时守护进程崩溃。
* [Issue #2888](https://github.com/manticoresoftware/manticoresearch/issues/2888) 修复了 `libstemmer` 分词失败时的崩溃。 ❤️ 感谢 [@subnix](https://github.com/subnix) 的 PR。
* [Issue #2919](https://github.com/manticoresoftware/manticoresearch/issues/2919) 修复了右表连接权重在表达式中未正确生效的问题。
* [Issue #2919](https://github.com/manticoresoftware/manticoresearch/issues/2919) 修复了右连接表权重在表达式中无效的问题。
* [Issue #325](https://github.com/manticoresoftware/manticoresearch-buddy/issues/325) 修复了当表已存在时 `CREATE TABLE IF NOT EXISTS ... WITH DATA` 失败的问题。
* [Issue #351](https://github.com/manticoresoftware/manticoresearch-buddy/issues/351) 修复了使用文档 ID 对 KNN 计数时报 undefined array key "id" 错误。
* [Issue #359](https://github.com/manticoresoftware/manticoresearch-buddy/issues/359) 修复了 `REPLACE INTO cluster_name:table_name` 功能。
* [Issue #67](https://github.com/manticoresoftware/docker/issues/67) 修复了使用 `--network=host` 运行 Manticore Docker 容器时的 FATAL 错误。

## Version 6.3.8
发布：2024年11月22日

版本 6.3.8 继续 6.3 系列，仅包含错误修复。

### 错误修复

* [PR #2777](https://github.com/manticoresoftware/manticoresearch/pull/2777) 修复了在通过 `threads` 或 `max_threads_per_query` 设置限制查询并发时，计算可用线程数的问题。

## manticore-extra v1.1.20

发布：2024年10月7日

### 错误修复

* [Issue #64](https://github.com/manticoresoftware/executor/issues/64) 解决了在基于 Debian 的系统中，`unattended-upgrades` 工具自动安装包更新时错误地将多个 Manticore 包（包括 `manticore-galera`、`manticore-executor` 和 `manticore-columnar-lib`）标记为删除的情况。这是由于 `dpkg` 错误地将虚拟包 `manticore-extra` 视为冗余导致的。已做出更改，确保 `unattended-upgrades` 不再尝试删除关键的 Manticore 组件。

## Version 6.3.6
发布：2024年8月2日

版本 6.3.6 继续 6.3 系列，仅包含错误修复。

### 错误修复

* [Issue #2477](https://github.com/manticoresoftware/manticoresearch/issues/2477) 修复了 6.3.4 版本引入的崩溃，该崩溃可能发生在处理表达式及分布式或多表时。
* [Issue #2352](https://github.com/manticoresoftware/manticoresearch/issues/2352) 修复了因 `max_query_time` 导致查询多个索引时提前退出而出现的守护进程崩溃或内部错误。

## Version 6.3.4
发布：2024年7月31日

版本 6.3.4 继续 6.3 系列，仅包含小幅改进和错误修复。

### 小幅改动
* [Issue #1130](https://github.com/manticoresoftware/manticoresearch/issues/1130) 添加了对 [DBeaver](Integration/DBeaver.md#Integration-with-DBeaver) 的支持。
* [Issue #2146](https://github.com/manticoresoftware/manticoresearch/issues/2146) 改进了单词形式和例外中分隔符的转义。
* [Issue #2315](https://github.com/manticoresoftware/manticoresearch/issues/2315) 在 SELECT 列表达式中添加了字符串比较运算符。
* [Issue #2363](https://github.com/manticoresoftware/manticoresearch/issues/2363) 添加了对 Elastic 样式批量请求中 null 值的支持。
* [Issue #2374](https://github.com/manticoresoftware/manticoresearch/issues/2374) 添加了对 mysqldump 版本 9 的支持。
* [Issue #2375](https://github.com/manticoresoftware/manticoresearch/issues/2375) 改进了 HTTP JSON 查询中的错误处理，增加了发生错误节点的 JSON 路径。

### Bug 修复
* [Issue #2280](https://github.com/manticoresoftware/manticoresearch/issues/2280) 修复了当 disk_chunks > 1 时，通配符查询匹配很多结果导致的性能下降问题。
* [Issue #2332](https://github.com/manticoresoftware/manticoresearch/issues/2332) 修复了 MVA MIN 或 MAX SELECT 列表达式中空 MVA 数组导致的崩溃。
* [Issue #2339](https://github.com/manticoresoftware/manticoresearch/issues/2339) 修复了 Kibana 的无限范围请求处理不正确的问题。
* [Issue #2342](https://github.com/manticoresoftware/manticoresearch/issues/2342) 修复了当属性不在 SELECT 列表中时，对右表列式属性的 join 过滤器问题。
* [Issue #2343](https://github.com/manticoresoftware/manticoresearch/issues/2343) 修复了 Manticore 6.3.2 中重复的 'static' 标识符。
* [Issue #2344](https://github.com/manticoresoftware/manticoresearch/issues/2344) 修复了使用右表 MATCH() 时 LEFT JOIN 返回非匹配条目的问题。
* [Issue #2350](https://github.com/manticoresoftware/manticoresearch/issues/2350) 修复了 RT 索引中带 `hitless_words` 的磁盘块保存问题。
* [Issue #2364](https://github.com/manticoresoftware/manticoresearch/issues/2364) 现在 'aggs_node_sort' 属性可以在其他属性中任意顺序添加。
* [Issue #2368](https://github.com/manticoresoftware/manticoresearch/issues/2368) 修复了 JSON 查询中全文检索与过滤器顺序错误的问题。
* [Issue #2376](https://github.com/manticoresoftware/manticoresearch/issues/2376) 修复了长 UTF-8 请求导致 JSON 响应错误的 bug。
* [Issue #2684](https://github.com/manticoresoftware/dev/issues/2684) 修复了依赖于连接属性的 presort/prefilter 表达式计算错误。
* [Issue #301](https://github.com/manticoresoftware/manticoresearch-buddy/issues/301) 修改指标数据大小的计算方法，从检查整个数据目录大小改为从 `manticore.json` 文件读取。
* [Issue #302](https://github.com/manticoresoftware/manticoresearch-buddy/issues/302) 添加了对 Vector.dev 验证请求的处理。

## 版本 6.3.2
发布时间：2024年6月26日

版本 6.3.2 继续 6.3 系列，包含多个 bug 修复，其中一些是在 6.3.0 发布后发现的。

### 重大变更
* ⚠️[Issue #2305](https://github.com/manticoresoftware/manticoresearch/issues/2305) 更新了 aggs.range 的值，改为数字类型。

### Bug 修复
* [Commit c51c](https://github.com/manticoresoftware/manticoresearch/commit/c51c7fa4bd0ec4ad8d53188b888461bff29f1417) 修复了按存储检验与 rset 合并分组问题。
* [Commit 0e85](https://github.com/manticoresoftware/manticoresearch/commit/0e85b9d5682f51681750d798aef6a11cfb93d69e) 修复了在 RT 索引使用 CRC 字典且启用 `local_df` 时，对含通配符字符查询导致的守护进程崩溃。
* [Issue #2200](https://github.com/manticoresoftware/manticoresearch/issues/2200) 修复了在没有 GROUP BY 的 JOIN 中使用 `count(*)` 导致的崩溃。
* [Issue #2201](https://github.com/manticoresoftware/manticoresearch/issues/2201) 修复了 JOIN 试图按全文字段分组时未返回警告的问题。
* [Issue #2230](https://github.com/manticoresoftware/manticoresearch/issues/2230) 修正了通过 `ALTER TABLE` 添加属性时未考虑 KNN 选项的问题。
* [Issue #2231](https://github.com/manticoresoftware/manticoresearch/issues/2231) 修复了版本 6.3.0 中无法卸载 `manticore-tools` Redhat 包的问题。
* [Issue #2242](https://github.com/manticoresoftware/manticoresearch/issues/2242) 修正了 JOIN 和多个 FACET 语句返回错误结果的问题。
* [Issue #2250](https://github.com/manticoresoftware/manticoresearch/issues/2250) 修复了当表处于集群中时 ALTER TABLE 报错的问题。
* [Issue #2252](https://github.com/manticoresoftware/manticoresearch/issues/2252) 修正了从 SphinxQL 接口将原始查询传递给 buddy 的问题。
* [Issue #2267](https://github.com/manticoresoftware/manticoresearch/issues/2267) 改进了带磁盘块的 RT 索引中 `CALL KEYWORDS` 的通配符扩展。
* [Issue #271](https://github.com/manticoresoftware/manticoresearch-buddy/issues/271) 修复了错误的 `/cli` 请求导致的挂起。
* [Issue #274](https://github.com/manticoresoftware/manticoresearch-buddy/issues/274) 解决了对 Manticore 的并发请求可能卡住的问题。
* [Issue #275](https://github.com/manticoresoftware/manticoresearch-buddy/issues/275) 修复了通过 `/cli` 执行 `drop table if exists t; create table t` 导致的挂起。

### 复制相关
* [Issue #2270](https://github.com/manticoresoftware/manticoresearch/issues/2270) 在 `/_bulk` HTTP 端点添加了对 `cluster:name` 格式的支持。

## 版本 6.3.0
发布时间：2024年5月23日

### 主要变更
* [Issue #839](https://github.com/manticoresoftware/manticoresearch/issues/839) 实现了 [float_vector](Creating_a_table/Data_types.md#Float-vector) 数据类型；实现了 [向量搜索](Searching/KNN.md#KNN-vector-search)。
* [Issue #1673](https://github.com/manticoresoftware/manticoresearch/issues/1673) 实现了 [INNER/LEFT JOIN](Searching/Joining.md)（**测试阶段**）。
* [Issue #1744](https://github.com/manticoresoftware/manticoresearch/issues/1744) 实现了对 [timestamp](Creating_a_table/Data_types.md#Timestamps) 字段日期格式的自动检测。
* [Issue #1720](https://github.com/manticoresoftware/manticoresearch/pull/1720) 将 Manticore Search 许可从 GPLv2-or-later 更改为 GPLv3-or-later。
* [Commit 7a55](https://github.com/manticoresoftware/manticoresearch/commit/7a5508424) 现在在 Windows 上运行 Manticore 需要通过 Docker 来运行 Buddy。
* [Issue #1541](https://github.com/manticoresoftware/manticoresearch/issues/1541) 添加了一个 [REGEX](Searching/Full_text_matching/Operators.md#REGEX-operator) 全文操作符。
* [Issue #2091](https://github.com/manticoresoftware/manticoresearch/issues/2091) 支持 Ubuntu Noble 24.04。
* [Commit 514d](https://github.com/manticoresoftware/manticoresearch/commit/514d35b497f4bdb20b6473ff963752a978a4bb8d) 重构了时间操作以提升性能并新增日期/时间函数：
  - [CURDATE()](Functions/Date_and_time_functions.md#CURDATE%28%29) - 返回本地时区当前日期
  - [QUARTER()](Functions/Date_and_time_functions.md#QUARTER%28%29) - 返回时间戳参数所属年份的季度整数
  - [DAYNAME()](Functions/Date_and_time_functions.md#DAYNAME%28%29) - 返回时间戳参数对应的星期名称
  - [MONTHNAME()](Functions/Date_and_time_functions.md#MONTHNAME%28%29) - 返回时间戳参数对应的月份名称
  - [DAYOFWEEK()](Functions/Date_and_time_functions.md#DAYOFWEEK%28%29) - 返回时间戳参数对应的星期索引整数
  - [DAYOFYEAR()](Functions/Date_and_time_functions.md#DAYOFYEAR%28%29) - 返回时间戳参数对应的年份中的第几天整数
  - [YEARWEEK()](Functions/Date_and_time_functions.md#YEARWEEK%28%29) - 返回时间戳参数对应的年份和本周第一天的日期码整数
  - [DATEDIFF()](Functions/Date_and_time_functions.md#DATEDIFF%28%29) - 返回两个时间戳之间相差的天数
  - [DATE()](Functions/Date_and_time_functions.md#DATE%28%29) - 格式化时间戳参数中的日期部分
  - [TIME()](Functions/Date_and_time_functions.md#TIME%28%29) - 格式化时间戳参数中的时间部分
  - [timezone](Server_settings/Searchd.md#timezone) - 日期/时间相关函数使用的时区。
* [Commit 30e7](https://github.com/manticoresoftware/manticoresearch/commit/30e789d9657f9b3093c280c6e5b0e46a021f6dc7) 向 HTTP 接口新增了 [range](Searching/Faceted_search.md#Facet-over-set-of-ranges)、[histogram](Searching/Faceted_search.md#Facet-over-histogram-values)、[date_range](Searching/Faceted_search.md#Facet-over-set-of-date-ranges) 和 [date_histogram](Searching/Faceted_search.md#Facet-over-histogram-date-values) 聚合，以及相似的 SQL 表达式。

### 小幅变化
* [Issue #1285](https://github.com/manticoresoftware/manticoresearch/issues/1285) 支持 Filebeat 版本 8.10 - 8.11。
* [Issue #1771](https://github.com/manticoresoftware/manticoresearch/issues/1771) 支持 [ALTER TABLE ... type='distributed'](Updating_table_schema_and_settings.md#Changing-a-distributed-table)。
* [Issue #1788](https://github.com/manticoresoftware/manticoresearch/issues/1788) 通过 [CREATE TABLE ... LIKE ... WITH DATA](Creating_a_table/Local_tables/Real-time_table.md#CREATE-TABLE-LIKE:) SQL 语句新增了复制表的功能。
* [Issue #2072](https://github.com/manticoresoftware/manticoresearch/issues/2072) 优化了 [表压缩算法](Securing_and_compacting_a_table/Compacting_a_table.md#Compacting-a-Table)：之前，手动执行 [OPTIMIZE](Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE) 及自动执行的 [auto_optimize](Server_settings/Searchd.md#auto_optimize) 过程都会先合并数据块以确保数量不超过限制，然后从所有已删除文档的数据块中删除文档。该方式有时过于耗费资源，现已禁用。现在，数据块合并完全依据 [progressive_merge](Server_settings/Common.md#progressive_merge) 设置进行。然而，包含大量已删除文档的数据块更容易被合并。
* [Commit ce6c](https://github.com/manticoresoftware/columnar/commit/ce6c5d3381edcdc411a00a5923d13d56dd9b3010) 新增了防止加载新版二级索引的保护机制。
* [Issue #1417](https://github.com/manticoresoftware/manticoresearch/issues/1417) 支持通过 [REPLACE INTO ... SET](Data_creation_and_modification/Updating_documents/REPLACE.md#SQL-REPLACE) 进行部分替换。
* [Commit 7c16](https://github.com/manticoresoftware/manticoresearch/commit/7c167b934ce416d3dfbd658744a231e638b14d9a) 更新了默认合并缓冲区大小：`.spa`（标量属性）从 256KB 调整为 8MB；`.spb`（blob 属性）从 256KB 调整为 8MB；`.spc`（列式属性）1MB，未改变；`.spds`（文档存储）从 256KB 调整为 8MB；`.spidx`（二级索引）缓冲区从 256KB 调整为内存限制 128MB；`.spi`（字典）从 256KB 调整为 16MB；`.spd`（文档列表）8MB，未改变；`.spp`（命中列表）8MB，未改变；`.spe`（跳跃列表）从 256KB 调整为 8MB。
* [Issue #1859](https://github.com/manticoresoftware/manticoresearch/issues/1859) 新增通过 JSON 方式的 [复合聚合](Searching/Grouping.md#GROUP-BY-multiple-fields-at-once)。
* [Commit 216b](https://github.com/manticoresoftware/manticoresearch-buddy/commit/216b824) 由于部分正则表达式存在问题且无明显时间优势，禁用了 PCRE.JIT。
* [Commit 55cd](https://github.com/manticoresoftware/manticoresearch/commit/55cdce32) 新增对原生 Galera v.3 (api v25) (`libgalera_smm.so`，来源于 MySQL 5.x) 的支持。
* [Commit 86f9](https://github.com/manticoresoftware/manticoresearch-buddy/commit/86f98ab) 指标后缀由 `_rate` 更改为 `_rps`。
* [Commit c0c1](https://github.com/manticoresoftware/manticoresearch-helm/commit/c0c1cff780031412ef57cefab987dfd16613368e) 改进了关于负载均衡器高可用性支持的文档。
* [Commit d1d2](https://github.com/manticoresoftware/manticoresearch/commit/d1d2ca18857c68106e89743756260e73285be739) 在错误信息中将 `index` 更改为 `table`；修复了 bison 解析器错误消息。
* [Commit fd26](https://github.com/manticoresoftware/manticoresearch/commit/fd26671b) 支持将 `manticore.tbl` 用作表名。
* [Issue #1105](https://github.com/manticoresoftware/manticoresearch/issues/1105) 支持通过 systemd 运行索引器 ([文档](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Schedule-indexer-via-systemd))。❤️ 感谢 [@subnix](https://github.com/subnix) 的 PR。
* [Issue #1294](https://github.com/manticoresoftware/manticoresearch/issues/1294) GEODIST() 中支持二级索引。
* [Issue #1394](https://github.com/manticoresoftware/manticoresearch/issues/1394) 简化了 [SHOW THREADS](Node_info_and_management/SHOW_THREADS.md#SHOW-THREADS)。
* [Issue #1424](https://github.com/manticoresoftware/manticoresearch/issues/1424) 为 `create distributed table` 语句添加了默认值支持 (`agent_connect_timeout` 和 `agent_query_timeout`)。
* [Issue #1442](https://github.com/manticoresoftware/manticoresearch/issues/1442) 添加了覆盖 `searchd.expansion_limit` 的搜索查询选项 [expansion_limit](Searching/Options.md#expansion_limit)。
* [Issue #1448](https://github.com/manticoresoftware/manticoresearch/issues/1448) 实现了 int 转 bigint 的 [ALTER TABLE](Updating_table_schema_and_settings.md#Updating-table-schema-in-RT-mode) 操作。
* [Issue #1456](https://github.com/manticoresoftware/manticoresearch/issues/1456) MySQL 响应中添加了元信息。
* [Issue #1494](https://github.com/manticoresoftware/manticoresearch/issues/1494) [SHOW VERSION](Node_info_and_management/SHOW_VERSION.md#SHOW-VERSION)。
* [Issue #1582](https://github.com/manticoresoftware/manticoresearch/issues/1582) 支持通过 JSON 按 id 数组删除文档 ([删除文档](Data_creation_and_modification/Deleting_documents.md?client=JSON#Deleting-documents))。
* [Issue #1589](https://github.com/manticoresoftware/manticoresearch/issues/1589) 改进错误提示 "unsupported value type"。
* [Issue #1634](https://github.com/manticoresoftware/manticoresearch/issues/1634) 在 `SHOW STATUS` 中添加了 Buddy 版本信息。
* [Issue #1641](https://github.com/manticoresoftware/manticoresearch/issues/1641) 针对关键字零文档情况优化匹配请求。
* [Issue #1712](https://github.com/manticoresoftware/manticoresearch/issues/1712) 在发布数据时添加了从字符串 "true" 和 "false" 转换为布尔属性的支持。
* [Issue #1713](https://github.com/manticoresoftware/manticoresearch/issues/1713) 添加了 [access_dict](Server_settings/Searchd.md#access_dict) 表和 searchd 选项。
* [Issue #1767](https://github.com/manticoresoftware/manticoresearch/issues/1767) 在配置的 searchd 部分添加了新选项：[expansion_merge_threshold_docs](Server_settings/Searchd.md#expansion_merge_threshold_docs) 和 [expansion_merge_threshold_hits](Server_settings/Searchd.md#expansion_merge_threshold_hits)；使扩展词条中微小词条合并阈值可配置。
* [Issue #1768](https://github.com/manticoresoftware/manticoresearch/issues/1768) 在 `@@system.sessions` 中添加了最后命令时间显示。
* [Issue #1806](https://github.com/manticoresoftware/manticoresearch/issues/1806) 将 Buddy 协议升级到版本 2。
* [Issue #1810](https://github.com/manticoresoftware/manticoresearch/issues/1810) 为 `/sql` 端点实现了附加请求格式，便于与库集成。
* [Issue #1825](https://github.com/manticoresoftware/manticoresearch/pull/1825) 为 SHOW BUDDY PLUGINS 添加了信息部分。
* [Issue #1837](https://github.com/manticoresoftware/manticoresearch/issues/1837) 改进了使用大包时 `CALL PQ` 的内存消耗。
* [Issue #1853](https://github.com/manticoresoftware/manticoresearch/pull/1853) 编译器从 Clang 15 切换到 Clang 16。
* [Issue #1857](https://github.com/manticoresoftware/manticoresearch/issues/1857) 添加了字符串比较。❤️ 感谢 [@etcd](https://github.com/etcd) 的 PR。
* [Issue #1915](https://github.com/manticoresoftware/manticoresearch/issues/1915) 添加了对联合存储字段的支持。
* [Issue #1937](https://github.com/manticoresoftware/manticoresearch/issues/1937) 在查询日志中记录客户端主机:端口。
* [Issue #1981](https://github.com/manticoresoftware/manticoresearch/issues/1981) 修复错误提示。
* [Issue #1983](https://github.com/manticoresoftware/manticoresearch/issues/1983) 引入支持 [通过 JSON 查询计划](Node_info_and_management/Profiling/Query_plan.md?client=JSON#Query-plan) 的详细等级。
* [Issue #2010](https://github.com/manticoresoftware/manticoresearch/issues/2010) 除非设置 `log_level=debug`，否则禁用 Buddy 查询的日志记录。
* [Issue #2035](https://github.com/manticoresoftware/manticoresearch/issues/2035) 支持 Linux Mint 21.3。
* [Issue #2056](https://github.com/manticoresoftware/manticoresearch/issues/2056) Manticore 无法使用 Mysql 8.3+ 构建。
* [Issue #2112](https://github.com/manticoresoftware/manticoresearch/issues/2112) 为实时表块添加了 `DEBUG DEDUP` 命令，适用于附加包含重复项的普通表后可能出现的重复条目。
* [Issue #212](https://github.com/manticoresoftware/manticoresearch-buddy/issues/212) 在 [SHOW QUERIES](Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES) 中添加时间。
* [Issue #218](https://github.com/manticoresoftware/manticoresearch-buddy/issues/218) 将 `@timestamp` 列作为时间戳处理。
* [Issue #252](https://github.com/manticoresoftware/manticoresearch-buddy/issues/250) 实现启用/禁用 buddy 插件的逻辑。
* [Issue #254](https://github.com/manticoresoftware/manticoresearch-buddy/issues/254) 更新 composer 到修复近期 CVE 的更高版本。
* [Issue #340](https://github.com/manticoresoftware/manticoresearch/issues/340) Manticore systemd 单元中与 `RuntimeDirectory` 相关的轻微优化。
* [Issue #51](https://github.com/manticoresoftware/executor/issues/51) 添加了 rdkafka 支持，并更新到 PHP 8.3.3。
* [Issue #527](https://github.com/manticoresoftware/manticoresearch/issues/527) 支持 [附加](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Attaching_one_table_to_another.md#Attaching-one-table-to-another) RT 表。新增命令 [ALTER TABLE ... RENAME](Updating_table_schema_and_settings.md#Renaming-a-real-time-table)。

### 重大更改和废弃内容
* ⚠️[Issue #1436](https://github.com/manticoresoftware/manticoresearch/issues/1436) 修复了 IDF 计算问题。`local_df` 现在为默认值。改进了主从搜索协议（版本已更新）。如果你在多实例分布式环境中运行 Manticore Search，请确保先升级代理，再升级主节点。
* ⚠️[Issue #1572](https://github.com/manticoresoftware/manticoresearch/issues/1572) 添加了分布式表的复制功能并更新了复制协议。如果你运行复制集群，需要：
  - 首先，干净停止所有节点
  - 然后，使用 Linux 下的工具 `manticore_new_cluster`，带上 `--new-cluster` 参数，启动最后停止的节点。
  - 更多详情请阅读关于 [重启集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster) 的内容。
* ⚠️[Issue #1763](https://github.com/manticoresoftware/manticoresearch/pull/1763) HTTP API 端点别名 `/json/*` 已废弃。
* ⚠️[Issue #1982](https://github.com/manticoresoftware/manticoresearch/issues/1982) JSON 中将 [profile](Node_info_and_management/Profiling/Query_profile.md#Query-profile) 改为 [plan](Node_info_and_management/Profiling/Query_plan.md#Query-plan)，新增了 JSON 查询分析功能。
* ⚠️[Commit e235](https://github.com/manticoresoftware/manticoresearch-backup/commit/e23585a) manticore-backup 不再备份 `plugin_dir`。
* ⚠️[Issue #171](https://github.com/manticoresoftware/manticoresearch-buddy/issues/155) Buddy 迁移至 Swoole 以提高性能和稳定性。升级到新版本时，确保所有 Manticore 包也已更新。
* ⚠️[Issue #196](https://github.com/manticoresoftware/manticoresearch-buddy/pull/205) 合并了所有核心插件到 Buddy，并变更了核心逻辑。
* ⚠️[Issue #2107](https://github.com/manticoresoftware/manticoresearch/issues/2107) 在 `/search` 响应中将文档 ID 视为数字类型。
* ⚠️[Issue #38](https://github.com/manticoresoftware/executor/issues/38) 添加了 Swoole，禁用了 ZTS，并移除了 parallel 扩展。
* ⚠️[Issue #1929](https://github.com/manticoresoftware/manticoresearch/issues/1929) `charset_table` 中的覆盖在某些情况下未生效。

### 复制相关更改
* [Commit 3376](https://github.com/manticoresoftware/manticoresearch/commit/3376351f73db3ff0cb6cb708a304a7bd83e69cfb) 修复了大文件 SST 复制时的错误。
* [Commit 6d36](https://github.com/manticoresoftware/manticoresearch/commit/6d36c68fb7ce3cb274b467aac435788905084952) 为复制命令添加重试机制；修复了在网络繁忙且丢包时复制连接失败的问题。
* [Commit 842e](https://github.com/manticoresoftware/manticoresearch/commit/842e27e71790833ab4125d49103d2933d5df60c3) 将复制中的 FATAL 消息更改为 WARNING 消息。
* [Commit 8c32](https://github.com/manticoresoftware/manticoresearch/commit/8c32bce7db39d8ed3b1a06aaa1d2c3ec372084f0) 修复了无表或空表的复制集群中 `gcache.page_size` 的计算；同时修复了 Galera 选项的保存和加载问题。
* [Commit a2af](https://github.com/manticoresoftware/manticoresearch/commit/a2af06ca36abcbf893e43c8803853130b397c54b) 新增功能，允许在加入集群的节点上跳过更新节点的复制命令。
* [Commit c054](https://github.com/manticoresoftware/manticoresearch/commit/c0541e6892f5116b1f957b40060ba8ecd01a5afb) 修复了更新二进制属性与替换文档间复制时的死锁问题。
* [Commit e80d](https://github.com/manticoresoftware/manticoresearch/commit/e80d505b963999016033613c40c1cf3151d648e7) 添加了 [replication_connect_timeout](Server_settings/Searchd.md#replication_connect_timeout)、[replication_query_timeout](Server_settings/Searchd.md#replication_query_timeout)、[replication_retry_delay](Server_settings/Searchd.md#replication_retry_delay)、[replication_retry_count](Server_settings/Searchd.md#replication_retry_count) 这些 searchd 配置选项，用于控制复制过程中的网络表现，类似于 `searchd.agent_*`，但默认值不同。
* [Issue #1356](https://github.com/manticoresoftware/manticoresearch/issues/1356) 修复了当部分节点丢失且名称解析失败时，复制节点重试的问题。
* [Issue #1445](https://github.com/manticoresoftware/manticoresearch/issues/1445) 修复了 `show variables` 中复制日志的详细程度问题。
* [Issue #1482](https://github.com/manticoresoftware/manticoresearch/issues/1482) 修复 Kubernetes 中 pod 重启后连接到集群的加入节点的复制问题。
* [Issue #1962](https://github.com/manticoresoftware/manticoresearch/issues/1962) 修复了空集群使用无效节点名称时长时间等待复制变更的问题。

### Bug 修复
* [Commit 8a48](https://github.com/manticoresoftware/manticoresearch/commit/8a486c7ba1a293879cdecc41fb52eb7ab0cbf832) 修复了 `count distinct` 中未使用匹配结果清理导致的崩溃问题。
* [Issue #1569](https://github.com/manticoresoftware/manticoresearch/issues/1569) 二进制日志现以事务粒度方式写入。
* [问题 #2089](https://github.com/manticoresoftware/manticoresearch/issues/2089) 修复了与64位ID相关的一个错误，该错误可能导致通过MySQL插入时出现“Malformed packet”错误，进而导致[表损坏和ID重复](https://github.com/manticoresoftware/manticoresearch/issues/2090)。
* [问题 #2160](https://github.com/manticoresoftware/manticoresearch/issues/2160) 修复了日期插入时被当作UTC时间而非本地时区的问题。
* [问题 #2177](https://github.com/manticoresoftware/manticoresearch/issues/2177) 修复了在带有非空 `index_token_filter` 的实时表中执行搜索时发生的崩溃问题。
* [问题 #2209](https://github.com/manticoresoftware/manticoresearch/issues/2209) 更改了RT列存储中的重复过滤以修复崩溃和错误的查询结果。
* [提交 001d](https://github.com/manticoresoftware/manticoresearch/commit/001d82904e4b887c49387308291aeefb07c0fbc4) 修复了html剥离器在处理联接字段后损坏内存的问题。
* [提交 00eb](https://github.com/manticoresoftware/manticoresearch/commit/00eb2ebf) 避免flush后回拨流以防止与mysqldump的通信异常。
* [提交 0553](https://github.com/manticoresoftware/manticoresearch/commit/05534114acf08c70ffcc41a23d558dec09a34373) 如果预读没有开始，则不等待其完成。
* [提交 055a](https://github.com/manticoresoftware/manticoresearch/commit/055aac09f877570cd29b6594cb283182eba7f73b) 修复搜索守护进程日志中大块Buddy输出字符串未正确分多行显示的问题。
* [提交 0a88](https://github.com/manticoresoftware/manticoresearch/commit/0a88056307f82c50fcdc458ad6dbc307a00e3f92) 移动了MySQL接口关于失败的头部 `debugv` 详细级别的警告。
* [提交 150a](https://github.com/manticoresoftware/manticoresearch/commit/150a20286b7d98d24b844ae577d7fda4a444798d) 修复了多集群管理操作中的竞态条件；禁止创建名称或路径相同的多个集群。
* [提交 2e40](https://github.com/manticoresoftware/manticoresearch/commit/2e405c1c9ebac6f46206e3df17ac1c19129c9aa3) 修复全文查询中的隐式截止；将MatchExtended拆分成模板partD。
* [提交 75f5](https://github.com/manticoresoftware/manticoresearch/commit/75f51421542a2a0658f9e2f9f31048c8c5224479) 修正了 `index_exact_words` 在索引和加载表到守护进程时的不一致问题。
* [提交 7643](https://github.com/manticoresoftware/manticoresearch/commit/7643976bea142e08535d701000eff0d09a99fda2) 修复了无效集群删除时缺少错误消息的问题。
* [提交 7a03](https://github.com/manticoresoftware/manticoresearch/commit/7a034ac990d5ec9344320c05a6a2315583b9b858) 修复了CBO与队列联合的问题；修复了CBO与RT伪分片问题。
* [提交 7b4e](https://github.com/manticoresoftware/manticoresearch/commit/7b4ea95e) 在无二级索引(SI)库和配置参数启动时，避免发出误导性的警告信息“WARNING: secondary_indexes set but failed to initialize secondary library”。
* [提交 8496](https://github.com/manticoresoftware/manticoresearch/commit/8496317f099fade8d2db463fb88483e7fcdfe85b) 修正了少数热排序中的问题。
* [提交 8973](https://github.com/manticoresoftware/manticoresearch-buddy/commit/8973ad3) 修复了ModifyTable插件中大写选项的问题。
* [提交 9935](https://github.com/manticoresoftware/manticoresearch-buddy/commit/99350d4c184379b19f3efa9751d9b5be5e6edfb5) 修复从包含空json值(表示为NULL)的转储恢复时的问题。
* [提交 a28f](https://github.com/manticoresoftware/manticoresearch/commit/a28f6e087b3d957e48816070be17d0823904936a) 通过使用pcon，修复了在接收SST时，连接节点的SST超时问题。
* [提交 b5a5](https://github.com/manticoresoftware/manticoresearch/commit/b5a55cc26debcddcda5d9caef8366a707e0dd763) 修复了选择别名字符串属性时的崩溃。
* [提交 c556](https://github.com/manticoresoftware/manticoresearch/commit/c556e7ed8b6da8eac8ef094650df5f0a8977916b) 添加了对带有 `morphology_skip_fields` 字段的全文查询中，将词条转换为 `=term` 的查询转换。
* [提交 cdc3](https://github.com/manticoresoftware/manticoresearch/commit/cdc38085145cd1025b96fcac2a46188a0425d9e1) 补充了缺失的配置键(skiplist_cache_size)。
* [提交 cf6e](https://github.com/manticoresoftware/manticoresearch/commit/cf6ec3bfdc3f0990fb31ff53822b2331a6667727) 修复了表达式排序器处理复杂大型查询时的崩溃。
* [提交 e513](https://github.com/manticoresoftware/manticoresearch/commit/e5139ba75eef7223df375ae3cc3f138fa7d925f1) 修复全文CBO与无效索引提示的冲突。
* [提交 eb05](https://github.com/manticoresoftware/manticoresearch/commit/eb0514b3a6f57f278252b0724eebd112ec116e0f) 在关闭时中断预读以加速关闭。
* [提交 f945](https://github.com/manticoresoftware/manticoresearch/commit/f94555a293cfa51004ff81135b8bd8500b955115) 更改全文查询的堆栈计算以避免复杂查询时崩溃。
* [问题 #1262](https://github.com/manticoresoftware/manticoresearch/issues/1262) 修复索引器在索引包含多个同名列的SQL源时崩溃。
* [问题 #1273](https://github.com/manticoresoftware/manticoresearch/issues/1273) 对不存在的系统变量返回0而非<empty>。
* [问题 #1289](https://github.com/manticoresoftware/manticoresearch/issues/1289) 修复indextool在检查RT表外部文件时的错误。
* [问题 #1335](https://github.com/manticoresoftware/manticoresearch/issues/1335) 修复了由于短语中含多词形式而导致的查询解析错误。
* [问题 #1364](https://github.com/manticoresoftware/manticoresearch/issues/1364) 新增了对旧版本binlog的空binlog文件重放支持。
* [问题 #1365](https://github.com/manticoresoftware/manticoresearch/issues/1365) 修复了删除最后一个空binlog文件的问题。
* [问题 #1372](https://github.com/manticoresoftware/manticoresearch/issues/1372) 修复了在 `data_dir` 更改后相对路径错误的问题（转换为守护进程启动目录的绝对路径），影响守护进程启动时的当前工作目录。
* [问题 #1393](https://github.com/manticoresoftware/manticoresearch/issues/1393) hn_small 中最慢的时间下降：在守护进程启动时获取/缓存 CPU 信息。
* [问题 #1395](https://github.com/manticoresoftware/manticoresearch/issues/1395) 修复了索引加载期间缺少外部文件的警告。
* [问题 #1402](https://github.com/manticoresoftware/manticoresearch/issues/1402) 修复了释放数据指针属性时全局分组器崩溃的问题。
* [问题 #1403](https://github.com/manticoresoftware/manticoresearch/issues/1403) _ADDITIONAL_SEARCHD_PARAMS 无效。
* [问题 #1427](https://github.com/manticoresoftware/manticoresearch/issues/1427) 修复了每表的 `agent_query_timeout` 被默认查询选项 `agent_query_timeout` 替换的问题。
* [问题 #1444](https://github.com/manticoresoftware/manticoresearch/issues/1444) 修复了使用带有匹配多个值的 `packedfactors()` 时分组器和排名器崩溃的问题。
* [问题 #1458](https://github.com/manticoresoftware/manticoresearch/issues/1458) Manticore 在频繁索引更新时崩溃。
* [问题 #1481](https://github.com/manticoresoftware/manticoresearch/issues/1481) 修复了解析错误后解析查询清理时崩溃的问题。
* [问题 #1484](https://github.com/manticoresoftware/manticoresearch/issues/1484) 修复了 HTTP JSON 请求未路由至 buddy 的问题。
* [问题 #1499](https://github.com/manticoresoftware/manticoresearch/issues/1499) JSON 属性根值不能是数组。已修复。
* [问题 #1507](https://github.com/manticoresoftware/manticoresearch/issues/1507) 修复了事务内表重建时崩溃的问题。
* [问题 #1515](https://github.com/manticoresoftware/manticoresearch/issues/1515) 修复了俄语词干短形式的展开。
* [问题 #1579](https://github.com/manticoresoftware/manticoresearch/issues/1579) 修复了在 [date_format](Functions/Date_and_time_functions.md#DATE_FORMAT()) 表达式中使用 JSON 和 STRING 属性的问题。
* [问题 #1580](https://github.com/manticoresoftware/manticoresearch/issues/1580) 修复了对 JSON 字段多个别名的分组器问题。
* [问题 #1594](https://github.com/manticoresoftware/manticoresearch/issues/1594) 开发中错误的 total_related：修正了隐式截止与限制的问题；改进了 JSON 查询中的全扫描检测。
* [问题 #1603](https://github.com/manticoresoftware/manticoresearch/issues/1603) 修复了在所有日期表达式中使用 JSON 和 STRING 属性的问题。
* [问题 #1609](https://github.com/manticoresoftware/manticoresearch/issues/1609) 使用 LEVENSHTEIN() 时崩溃。
* [问题 #1612](https://github.com/manticoresoftware/manticoresearch/issues/1612) 修复了搜索查询解析错误后高亮部分的内存损坏。
* [问题 #1614](https://github.com/manticoresoftware/manticoresearch/issues/1614) 禁用对短于 `min_prefix_len` / `min_infix_len` 的词的通配符扩展。
* [问题 #1617](https://github.com/manticoresoftware/manticoresearch/issues/1617) 修改行为：如果 Buddy 成功处理请求，则不再记录错误。
* [问题 #1635](https://github.com/manticoresoftware/manticoresearch/issues/1635) 修复了对设置了 limit 的查询，在 meta 中 total 值错误的问题。
* [问题 #1640](https://github.com/manticoresoftware/manticoresearch/issues/1640) 无法通过 JSON 以纯模式使用大写表名的问题。
* [问题 #1643](https://github.com/manticoresoftware/manticoresearch/issues/1643) 提供了默认的 `SPH_EXTNODE_STACK_SIZE` 值。
* [问题 #1646](https://github.com/manticoresoftware/manticoresearch/issues/1646) 修复了 MVA 属性使用 ALL/ANY 时 SphinxQL 记录负筛选器的问题。
* [问题 #1660](https://github.com/manticoresoftware/manticoresearch/issues/1660) 修正了来自其他索引的 docid 杀列表应用问题。❤️ 感谢 [@raxoft](https://github.com/raxoft) 的 PR。
* [问题 #1668](https://github.com/manticoresoftware/manticoresearch/issues/1668) 修复了在无索引迭代器的原始索引全扫描时因提前退出导致遗漏匹配的问题；移除了纯行迭代器的截止。
* [问题 #1671](https://github.com/manticoresoftware/manticoresearch/issues/1671) 修复了使用代理和本地表查询分布式表时 `FACET` 错误。
* [问题 #1690](https://github.com/manticoresoftware/manticoresearch/issues/1690) 修复了大值直方图估计时的崩溃。
* [问题 #1692](https://github.com/manticoresoftware/manticoresearch/issues/1692) alter table tbl add column col uint 时崩溃。
* [问题 #1710](https://github.com/manticoresoftware/manticoresearch/issues/1710) 条件 `WHERE json.array IN (<value>)` 返回空结果。
* [问题 #172](https://github.com/manticoresoftware/manticoresearch-buddy/issues/172) 修复了发送请求到 `/cli` 时 TableFormatter 的问题。
* [问题 #1742](https://github.com/manticoresoftware/manticoresearch/issues/1742) 在缺少 wordforms 文件时 `CREATE TABLE` 未失败。
* [问题 #1762](https://github.com/manticoresoftware/manticoresearch/issues/1762) RT 表中的属性顺序现在遵循配置顺序。
* [问题 #1765](https://github.com/manticoresoftware/manticoresearch/issues/1765) 带 'should' 条件的 HTTP bool 查询返回错误结果。
* [问题 #1769](https://github.com/manticoresoftware/manticoresearch/issues/1769) 使用 `SPH_SORT_ATTR_DESC` 和 `SPH_SORT_ATTR_ASC` 排序字符串属性无效。
* [问题 #177](https://github.com/manticoresoftware/manticoresearch-buddy/issues/177) 禁用对 Buddy 的 curl 请求中的 `Expect: 100-continue` HTTP 头。
* [问题 #1791](https://github.com/manticoresoftware/manticoresearch/issues/1791) 由 GROUP BY 别名引起的崩溃。
* [问题 #1792](https://github.com/manticoresoftware/manticoresearch/issues/1792) Windows 上 SQL meta summary 显示错误时间。
* [Issue #1794](https://github.com/manticoresoftware/manticoresearch/issues/1794) 修复了 JSON 查询中单一词项性能下降的问题。
* [Issue #1798](https://github.com/manticoresoftware/manticoresearch/issues/1798) 不兼容的过滤器在 `/search` 上未引发错误。
* [Issue #1802](https://github.com/manticoresoftware/manticoresearch/issues/1802) 修复了 `ALTER CLUSTER ADD` 和 `JOIN CLUSTER` 操作相互等待，防止 `ALTER` 在捐赠者发送表到加入节点时添加表到集群的竞争条件。
* [Issue #1811](https://github.com/manticoresoftware/manticoresearch/issues/1811) 修正了 `/pq/{table}/*` 请求的错误处理。
* [Issue #1816](https://github.com/manticoresoftware/manticoresearch/issues/1816) `UNFREEZE` 在某些情况下无法工作。
* [Issue #183](https://github.com/manticoresoftware/manticoresearch-buddy/pull/183) 修复了某些情况下 MVA 恢复的问题。
* [Issue #1849](https://github.com/manticoresoftware/manticoresearch/issues/1849) 修复了关闭时使用 MCL 时 indextool 崩溃的问题。
* [Issue #1866](https://github.com/manticoresoftware/manticoresearch/issues/1866) 修正了 `/cli_json` 请求的不必要的 URL 解码。
* [Issue #1872](https://github.com/manticoresoftware/manticoresearch/issues/1872) 变更了守护进程启动时插件目录设置逻辑。
* [Issue #1874](https://github.com/manticoresoftware/manticoresearch/issues/1874) alter table ... 异常失败。
* [Issue #1891](https://github.com/manticoresoftware/manticoresearch/issues/1891#issuecomment-2051941200) 插入数据时 Manticore 出现 `signal 11` 崩溃。
* [Issue #1920](https://github.com/manticoresoftware/manticoresearch/issues/1920) 减少了对 [low_priority](Searching/Options.md#low_priority) 的限流。
* [Issue #1924](https://github.com/manticoresoftware/manticoresearch/issues/1924) Mysqldump + mysql 恢复的错误。
* [Issue #1951](https://github.com/manticoresoftware/manticoresearch/issues/1951) 修正了在本地表缺失或代理描述错误时分布式表创建错误；现在返回错误信息。
* [Issue #1972](https://github.com/manticoresoftware/manticoresearch/issues/1972) 实现了一个 `FREEZE` 计数器以避免 freeze/unfreeze 问题。
* [Issue #1980](https://github.com/manticoresoftware/manticoresearch/issues/1980) 遵守 OR 节点中的查询超时。此前 `max_query_time` 在某些情况下可能无效。
* [Issue #1986](https://github.com/manticoresoftware/manticoresearch/issues/1986) 无法将 new 重命名为 current [manticore.json]。
* [Issue #1988](https://github.com/manticoresoftware/manticoresearch/issues/1988) 全文查询可能忽略 `SecondaryIndex` CBO 提示。
* [Issue #1990](https://github.com/manticoresoftware/manticoresearch/issues/1990) 修正 `expansion_limit` 对来自多个磁盘块或 RAM 块的调用关键字最终结果集的切片限制。
* [Issue #1994](https://github.com/manticoresoftware/manticoresearch/issues/1994) 错误的外部文件。
* [Issue #2021](https://github.com/manticoresoftware/manticoresearch/issues/2021) 停止 Manticore 后，一些 manticore-executor 进程可能继续运行。
* [Issue #2029](https://github.com/manticoresoftware/manticoresearch/issues/2029) 使用 Levenshtein 距离时崩溃。
* [Issue #2037](https://github.com/manticoresoftware/manticoresearch/issues/2037) 在空索引上运行多个 max 操作符后出现错误。
* [Issue #2052](https://github.com/manticoresoftware/manticoresearch/issues/2052) 多组带有 JSON.field 时崩溃。
* [Issue #2067](https://github.com/manticoresoftware/manticoresearch/issues/2067) 对 _update 的错误请求导致 Manticore 崩溃。
* [Issue #2069](https://github.com/manticoresoftware/manticoresearch/issues/2069) 修正 JSON 接口中闭区间字符串过滤比较符的问题。
* [Issue #2082](https://github.com/manticoresoftware/manticoresearch/issues/2082) 当 data_dir 路径位于符号链接上时，`alter` 失败。
* [Issue #2102](https://github.com/manticoresoftware/manticoresearch/pull/2102) 改进 mysqldump 中 SELECT 查询的特殊处理，确保生成的 INSERT 语句兼容 Manticore。
* [Issue #2103](https://github.com/manticoresoftware/manticoresearch/issues/2103) 泰文字符集错误。
* [Issue #2124](https://github.com/manticoresoftware/manticoresearch/issues/2124) 使用保留字的 SQL 导致崩溃。
* [Issue #2154](https://github.com/manticoresoftware/manticoresearch/issues/2154) 带词形变化的表无法导入。
* [Issue #2176](https://github.com/manticoresoftware/manticoresearch/issues/2176) 修复了引擎参数设为 'columnar' 并通过 JSON 添加重复 ID 时发生的崩溃。
* [Issue #223](https://github.com/manticoresoftware/manticoresearch-buddy/issues/223) 尝试插入无模式且无列名文档时的正确错误提示。
* [Issue #239](https://github.com/manticoresoftware/manticoresearch-buddy/issues/239) 自动模式多行插入可能失败。
* [Issue #399](https://github.com/manticoresoftware/manticoresearch/issues/399) 如果数据源声明了 id 属性，索引过程中增加错误提示。
* [Issue #59](https://github.com/manticoresoftware/manticoresearch-helm/issues/59) Manticore 集群崩溃。
* [Issue #68](https://github.com/manticoresoftware/manticoresearch-helm/issues/68) 若存在 percolate 表，optimize.php 会崩溃。
* [Issue #77](https://github.com/manticoresoftware/manticoresearch-helm/issues/77) 修复 Kubernetes 部署错误。
* [Issue #274](https://github.com/manticoresoftware/manticoresearch-buddy/issues/274) 修复对 Buddy 的并发请求错误处理。

### 与 manticore-backup 相关
* [Issue #97](https://github.com/manticoresoftware/manticoresearch-backup/pull/97) 当可用时设置 VIP HTTP 端口为默认。
各种改进：改进了版本检查和ZSTD流解压缩；在还原过程中增加了版本不匹配的用户提示；修正了还原时对不同版本提示行为的错误；增强了解压缩逻辑，直接从流中读取而不是加载到工作内存；添加了`--force`标志
* [提交 3b35](https://github.com/manticoresoftware/manticoresearch-backup/commit/3b357ea) 在Manticore搜索启动后添加备份版本显示，以便在此阶段识别问题。
* [提交 ad2e](https://github.com/manticoresoftware/manticoresearch-backup/commit/ad2e5da) 更新守护进程连接失败的错误信息。
* [提交 ce5e](https://github.com/manticoresoftware/manticoresearch-backup/commit/ce5ea85) 修正将绝对根备份路径转换为相对路径的问题，并移除还原时的可写性检查，以支持从不同路径进行还原。
* [提交 db7e](https://github.com/manticoresoftware/manticoresearch-backup/commit/db7e2b9) 为文件迭代器增加排序，确保在各种情况下的一致性。
* [问题 #106](https://github.com/manticoresoftware/manticoresearch-backup/issues/105) 多配置的备份和还原。
* [问题 #91](https://github.com/manticoresoftware/manticoresearch-backup/pull/91) 添加defattr，以防止在RHEL安装后文件出现异常的用户权限。
* [问题 #91](https://github.com/manticoresoftware/manticoresearch-backup/pull/91) 添加额外的chown，确保文件在Ubuntu中默认归root用户所有。

### 与MCL（列式，二级索引，knn库）相关
* [提交 f104](https://github.com/manticoresoftware/columnar/commit/f104a4709bbb144140d09a09bc0fc1db1895e331) 支持向量搜索。
* [提交 2169](https://github.com/manticoresoftware/columnar/commit/216953beef6676f7e34c9f6f1a30c5980d746710) 修复在中断的二级索引构建设置期间临时文件清理问题。此修复解决了守护进程在创建`tmp.spidx`文件时超过打开文件限制的问题。
* [提交 709b](https://github.com/manticoresoftware/columnar/commit/709b9acaaac97d9a1ca8796892f9ad432021c785) 为列式存储和二级索引使用独立的streamvbyte库。
* [提交 1c26](https://github.com/manticoresoftware/manticoresearch/commit/1c2623763ced14bb94ad624dde9614a938408d89) 添加警告，列式存储不支持json属性。
* [提交 3acd](https://github.com/manticoresoftware/columnar/commit/3acd6714ffce103447d7931dfc75bb1a7449f340) 修正二级索引中的数据解包问题。
* [提交 574c](https://github.com/manticoresoftware/manticoresearch/commit/574c023152564fa72092c18ebd1f594abfd4b6d7) 修复混合行存储和列式存储的磁盘块保存时的崩溃。
* [提交 e87f](https://github.com/manticoresoftware/columnar/commit/e87f088b458d4c5b1b6159fc72ed0d57b786891a) 修复二级索引迭代器提示已处理块的问题。
* [问题 #1474](https://github.com/manticoresoftware/manticoresearch/issues/1474) 行式多值属性（MVA）列在列式引擎中的更新失效。
* [问题 #1510](https://github.com/manticoresoftware/manticoresearch/issues/1510) 修复聚合到在`HAVING`中使用的列式属性时的崩溃。
* [问题 #1519](https://github.com/manticoresoftware/manticoresearch/issues/1519) 修复`expr`排位器使用列式属性时的崩溃。

### 与Docker相关
* ❗[问题 #42](https://github.com/manticoresoftware/docker/issues/42) 通过环境变量支持[纯表索引](https://github.com/manticoresoftware/docker#building-plain-tables)。
* ❗[问题 #47](https://github.com/manticoresoftware/docker/issues/47) 通过环境变量提高配置灵活性。
* [问题 #54](https://github.com/manticoresoftware/docker/pull/54) 改进Docker的[备份和还原](https://github.com/manticoresoftware/docker#backup-and-restore)过程。
* [问题 #77](https://github.com/manticoresoftware/docker/issues/76) 改进入口点，仅在首次启动时处理备份还原。
* [提交 a27c](https://github.com/manticoresoftware/docker/commit/a27c048dd9e73d8d6bea8d9a4830cafb486b82c5) 修复查询日志输出到stdout。
* [问题 #38](https://github.com/manticoresoftware/docker/issues/38) 若未设置EXTRA，静音BUDDY警告。
* [问题 #71](https://github.com/manticoresoftware/docker/pull/71) 修复`manticore.conf.sh`中的主机名。

## 版本 6.2.12
发布日期：2023年8月23日

版本6.2.12延续6.2系列，修复了6.2.0发布后发现的问题。

### Bug修复
* ❗[问题 #1351](https://github.com/manticoresoftware/manticoresearch/issues/1351) “Manticore 6.2.0在Centos 7上无法通过systemctl启动”：将`TimeoutStartSec`从`infinity`改为`0`，以提高与Centos 7的兼容性。
* ❗[问题 #1364](https://github.com/manticoresoftware/manticoresearch/issues/1364) “从6.0.4升级到6.2.0后崩溃”：为旧版binlog的空日志文件添加回放功能。
* [PR #1334](https://github.com/manticoresoftware/manticoresearch/pull/1334) “修正searchdreplication.cpp中的拼写错误”：将`beggining`改为`beginning`。
* [问题 #1337](https://github.com/manticoresoftware/manticoresearch/issues/1337) “Manticore 6.2.0警告：conn (local)(12), sock=8088: bailing on failed MySQL header, AsyncNetInputBuffer_c::AppendData: error 11（资源临时不可用）返回-1”：将MySQL接口关于头信息的警告级别降低到logdebugv。
* [问题 #1355](https://github.com/manticoresoftware/manticoresearch/issues/1355) “当node_address无法解析时，join cluster挂起”：改进在某些节点不可达且名称解析失败时的复制重试。这应修复Kubernetes和Docker节点相关的复制问题。增强复制启动失败的错误信息，更新测试模型376。同时提供了名称解析失败的清晰错误提示。
* [Issue #1361](https://github.com/manticoresoftware/manticoresearch/issues/1361) "字符集 non_cjk 中“Ø”的无小写映射": 调整了字符 'Ø' 的映射。
* [Issue #1365](https://github.com/manticoresoftware/manticoresearch/issues/1365) "searchd 在正常停止后留下 binlog.meta 和 binlog.001": 确保最后一个空的 binlog 文件被正确移除。
* [Commit 0871](https://github.com/manticoresoftware/manticoresearch/commit/0871070845e93ad702ae0dfb89b35e5f24cd0851): 修复了 Windows 上与原子复制限制相关的 `Thd_t` 构建问题。
* [Commit 1cc0](https://github.com/manticoresoftware/manticoresearch/commit/1cc04bedc174737118fd5a3663e35f4e95bd4f8c): 解决了 FT CBO 与 `ColumnarScan` 的问题。
* [Commit c6bf](https://github.com/manticoresoftware/manticoresearch/commit/c6bfb30d53e3f4b67f17769667733677a661c95b): 修正了测试 376 并为测试中的 `AF_INET` 错误添加了替代处理。
* [Commit cbc3](https://github.com/manticoresoftware/manticoresearch/commit/cbc38705e0b4dcc9665e3e8cf64826290ebd11cf): 解决了复制过程中更新 blob 属性与替换文档时的死锁问题。同时移除了提交期间索引的 rlock，因为它已经在更基础的层面被锁定。

### 小改动
* [Commit 4f91](https://github.com/manticoresoftware/manticoresearch/commit/4f913a36e84e01f950b2797051222b0b1094d6fe) 更新了手册中 `/bulk` 端点的信息。

### MCL
* 支持 [Manticore Columnar Library](https://github.com/manticoresoftware/columnar) v2.2.4

## 版本 6.2.0
发布时间：2023年8月4日

### 重大改动
* 查询优化器已增强以支持全文查询，显著提升搜索效率和性能。
* 与以下工具的集成：
  - [mysqldump](https://dev.mysql.com/doc/refman/8.0/en/mysqldump.html) - 使用 `mysqldump` 进行[逻辑备份](Securing_and_compacting_a_table/Backup_and_restore.md#Backup-and-restore-with-mysqldump)
  - [Apache Superset](https://manticoresearch.com/blog/manticoresearch-apache-superset-integration/) 和 [Grafana](https://manticoresearch.com/blog/manticoresearch-grafana-integration/) 用于可视化存储在 Manticore 中的数据
  - [HeidiSQL](https://www.heidisql.com/) 和 [DBForge](https://www.devart.com/dbforge/) 以简化与 Manticore 的开发工作
* 我们开始使用 [GitHub workflows](https://github.com/manticoresoftware/manticoresearch/actions)，使贡献者更易于使用与核心团队相同的持续集成（CI）流程准备软件包。所有任务均可在 GitHub 托管的运行器上执行，方便无缝测试你自己分支上的 Manticore Search 变更。
* 我们开始使用 [CLT](https://github.com/manticoresoftware/clt) 来测试复杂场景。例如，我们现在能够确保提交之后构建的软件包可以在所有支持的 Linux 操作系统上正确安装。命令行测试器（CLT）提供交互式录制测试和轻松回放测试的简便方式。
* 通过结合使用哈希表和 HyperLogLog，显著提升了计数去重操作的性能。
* 启用包含二级索引的查询的多线程执行，线程数限制为物理 CPU 核心数。这将大幅提升查询执行速度。
* 对 `pseudo_sharding` 进行了调整，使其限制在空闲线程数量内。此更新显著增强了吞吐性能。
* 用户现在可以通过配置设置指定[默认属性存储引擎](Server_settings/Searchd.md#engine)，更好地定制以适应特定工作负载需求。
* 支持 [Manticore Columnar Library 2.2.0](https://github.com/manticoresoftware/columnar/)，在[二级索引](Server_settings/Searchd.md#secondary_indexes) 中修复了众多缺陷并进行了改进。

### 小改动
* [Buddy #153](https://github.com/manticoresoftware/manticoresearch-buddy/issues/153): [/pq](Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md?client=JSON#Adding-rules-to-a-percolate-table) HTTP 端点现作为 `/json/pq` HTTP 端点的别名。
* [Commit 0bf1](https://github.com/manticoresoftware/manticoresearch/commit/0bf17d9e): 我们确保了 `upper()` 和 `lower()` 的多字节兼容性。
* [Commit 2bb9](https://github.com/manticoresoftware/manticoresearch/commit/2bb92765): 对于 `count(*)` 查询，不再扫描索引，而是直接返回预计算值。
* [Commit 3c84](https://github.com/manticoresoftware/manticoresearch/commit/3c84091f): 现在可以使用 `SELECT` 进行任意计算和显示 `@@sysvars`。不同于以往，你不再仅限于单一计算。因此，像 `select user(), database(), @@version_comment, version(), 1+1 as a limit 10` 的查询将返回所有列。注意，可选的 'limit' 将始终被忽略。
* [Commit 6aca](https://github.com/manticoresoftware/manticoresearch/commit/6aca32f0): 实现了 `CREATE DATABASE` 的存根查询。
* [Commit 9dc1](https://github.com/manticoresoftware/manticoresearch/commit/9dc12334): 当执行 `ALTER TABLE table REBUILD SECONDARY` 时，即使属性未更新，二级索引也总会被重建。
* [Commit 46ed](https://github.com/manticoresoftware/manticoresearch/commit/46edb089): 现在在使用 CBO 之前先识别使用预计算数据的排序器，避免不必要的 CBO 计算。
* [Commit 102a](https://github.com/manticoresoftware/manticoresearch/commit/102ac604): 实现并使用全文表达式堆栈的模拟，防止守护进程崩溃。
* [Commit 979f](https://github.com/manticoresoftware/manticoresearch/commit/979fa27c): 为匹配不使用字符串/mvas/json 属性的匹配克隆代码新增了快速路径。
* [提交 a073](https://github.com/manticoresoftware/manticoresearch/commit/a0735ffe)：添加了对 `SELECT DATABASE()` 命令的支持。但它始终返回 `Manticore`。此添加对于与各种 MySQL 工具的集成至关重要。
* [提交 bc04](https://github.com/manticoresoftware/manticoresearch/commit/bc04908)：修改了 [/cli](Connecting_to_the_server/HTTP.md#/cli) 端点的响应格式，新增 `/cli_json` 端点以实现之前 `/cli` 的功能。
* [提交 d70b](https://github.com/manticoresoftware/manticoresearch/commit/d70b0d58)：现在可以通过 `SET` 语句在运行时更改 `thread_stack`。提供了会话本地和守护进程全局两种变体。当前值可通过 `show variables` 输出查看。
* [提交 d96e](https://github.com/manticoresoftware/manticoresearch/commit/d96ec6b9)：代码已集成到 CBO，以更准确地估算字符串属性上执行过滤器的复杂性。
* [提交 e77d](https://github.com/manticoresoftware/manticoresearch/commit/e77dd72f5a04531c352fad0d7afcd2a1cbae2510)：改进了 DocidIndex 成本计算，提升了整体性能。
* [提交 f3ae](https://github.com/manticoresoftware/manticoresearch/commit/f3ae8bea)：类似于 Linux 上的“uptime”，负载指标现在可以通过 `SHOW STATUS` 命令查看。
* [提交 f3cc](https://github.com/manticoresoftware/manticoresearch/commit/f3cc0971)：`DESC` 和 `SHOW CREATE TABLE` 的字段及属性顺序现在与 `SELECT * FROM` 保持一致。
* [提交 f3d2](https://github.com/manticoresoftware/manticoresearch/commit/f3d248a6)：不同的内部解析器现在在各种错误中提供其内部助记代码（例如 `P01`）。此增强有助于识别导致错误的解析器，同时隐藏非必要的内部细节。
* [问题 #271](https://github.com/manticoresoftware/manticoresearch/issues/271) “有时 CALL SUGGEST 不会建议单字母拼写错误的更正”：改进了 [SUGGEST/QSUGGEST](Searching/Spell_correction.md#CALL-QSUGGEST,-CALL-SUGGEST) 对短词的表现：新增选项 `sentence` 用以显示整句。
* [问题 #696](https://github.com/manticoresoftware/manticoresearch/issues/696) “启动索引在启用词干处理时无法通过精确短语查询正确搜索”：修改了启动查询以支持精确词修饰符，提升搜索功能。
* [问题 #829](https://github.com/manticoresoftware/manticoresearch/issues/829) “日期格式化方法”：新增了 [date_format()](Functions/Date_and_time_functions.md#DATE_FORMAT()) 选择列表表达式，暴露了 `strftime()` 函数。
* [问题 #961](https://github.com/manticoresoftware/manticoresearch/issues/961) “通过 HTTP JSON API 对桶排序”：在 HTTP 接口中为每个聚合桶引入了可选的 [排序属性](Searching/Faceted_search.md#HTTP-JSON)。
* [问题 #1062](https://github.com/manticoresoftware/manticoresearch/issues/1062) “改进 JSON 插入接口失败时的错误日志 - “不支持的值类型””：当发生错误时，`/bulk` 端点报告处理和未处理字符串（文档）的数量信息。
* [问题 #1070](https://github.com/manticoresoftware/manticoresearch/issues/1070) “CBO 提示不支持多个属性”：启用索引提示以处理多个属性。
* [问题 #1106](https://github.com/manticoresoftware/manticoresearch/issues/1106) “为 HTTP 搜索查询添加标签”：在 [HTTP PQ 响应](Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md?client=JSON#Adding-rules-to-a-percolate-table) 中添加了标签。
* [问题 #1301](https://github.com/manticoresoftware/manticoresearch/issues/1301) “buddy 不应并行创建表”：解决了并行执行 CREATE TABLE 操作时导致失败的问题。现在同一时间只允许一个 `CREATE TABLE` 操作运行。
* [问题 #1303](https://github.com/manticoresoftware/manticoresearch/issues/1303) “为列名添加 @ 支持”。
* [问题 #1316](https://github.com/manticoresoftware/manticoresearch/issues/1316) “使用 ps=1 查询出租车数据集速度慢”：改进了 CBO 逻辑，默认直方图分辨率设为 8k，以便更准确地处理随机分布值的属性。
* [问题 #1317](https://github.com/manticoresoftware/manticoresearch/issues/1317) “修正 CBO 与全文索引在 hn 数据集上的区别”：加强了确定何时使用位图迭代器交集和何时使用优先队列的逻辑。
* [问题 #1318](https://github.com/manticoresoftware/manticoresearch/issues/1318) “columnar：将迭代器接口更改为单次调用”：Columnar 迭代器现改为只用单次 `Get` 调用，替代之前的两步 `AdvanceTo` + `Get` 获取值。
* [问题 #1319](https://github.com/manticoresoftware/manticoresearch/issues/1319) “加速聚合计算（移除 CheckReplaceEntry？）”：从组排序器中移除 `CheckReplaceEntry` 调用，加快聚合函数计算速度。
* [问题 #1320](https://github.com/manticoresoftware/manticoresearch/issues/1320) “创建表 read_buffer_docs/hits 不支持 k/m/g 语法”：`CREATE TABLE` 选项中的 `read_buffer_docs` 和 `read_buffer_hits` 现支持 k/m/g 语法。
* [语言包](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) 英文、德文和俄文现可通过在 Linux 执行 `apt/yum install manticore-language-packs` 命令轻松安装。在 macOS 上使用命令 `brew install manticoresoftware/tap/manticore-language-packs`。
* 字段和属性顺序现在在 `SHOW CREATE TABLE` 和 `DESC` 操作之间保持一致。
* 如果执行 `INSERT` 查询时磁盘空间不足，则新的 `INSERT` 查询会失败，直到有足够磁盘空间可用。
* 新增了 [UINT64()](Functions/Type_casting_functions.md#UINT64%28%29) 类型转换函数。
* `/bulk`端点现在将空行处理为[提交](Data_creation_and_modification/Transactions.md#BEGIN,-COMMIT,-and-ROLLBACK)命令。更多信息见[这里](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md?client=JSON#Bulk-adding-documents)。
* 已为[无效索引提示](Searching/Options.md#Query-optimizer-hints)实现了警告，提供了更多透明度并允许错误缓解。
* 当`count(*)`与单个过滤条件一起使用时，查询现在在可用时利用来自二级索引的预计算数据，大幅加快查询速度。

### ⚠️ 重大变更
* ⚠️ 在6.2.0版本中创建或修改的表无法被旧版本读取。
* ⚠️ 文档ID在索引和INSERT操作中现在作为无符号64位整数处理。
* ⚠️ 查询优化器提示的语法已更新。新格式为`/*+ SecondaryIndex(uid) */`。请注意旧语法已不再支持。
* ⚠️ [问题 #1160](https://github.com/manticoresoftware/manticoresearch/issues/1160): 为防止语法冲突，禁止在表名中使用`@`符号。
* ⚠️ 标记为`indexed`和`attribute`的字符串字段/属性在`INSERT`、`DESC`和`ALTER`操作中现在被视为单个字段。
* ⚠️ [问题 #1057](https://github.com/manticoresoftware/manticoresearch/issues/1057): MCL库不再在不支持SSE4.2的系统上加载。
* ⚠️ [问题 #1143](https://github.com/manticoresoftware/manticoresearch/issues/1143): [agent_query_timeout](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent query_timeout)曾出现故障，现已修复并生效。

### Bug修复
* [提交 2a6e](https://github.com/manticoresoftware/manticoresearch/commit/2a6ea8f7) “DROP TABLE故障”：解决了在执行DROP TABLE语句时导致RT表写操作（优化、磁盘区块保存）等待时间过长的问题。新增警告以通知在执行DROP TABLE命令后表目录非空。
* [提交 2ebd](https://github.com/manticoresoftware/manticoresearch/commit/2ebd424d): 增加了对列属性的支持，该功能之前在用于多属性分组的代码中缺失。
* [提交 3be4](https://github.com/manticoresoftware/manticoresearch/commit/3be4503f752216b37b8cde8e47fd2d77bbce63ae) 解决了因磁盘空间不足导致的崩溃问题，通过正确处理binlog中的写入错误。
* [提交 6adb](https://github.com/manticoresoftware/manticoresearch/commit/6adb0934): 修复了在查询中使用多个列扫描迭代器（或二级索引迭代器）时偶尔发生的崩溃。
* [提交 6bd9](https://github.com/manticoresoftware/manticoresearch/commit/6bd9f709): 解决了使用预计算数据的排序器时过滤器未被移除的问题。
* [提交 6d03](https://github.com/manticoresoftware/manticoresearch/commit/6d03566a): 更新了CBO代码，提供了对多线程执行的基于行属性过滤查询更好的估计。
* [提交 6dd3](https://github.com/manticoresoftware/manticoresearch/commit/6dd3964), [Helm #56](https://github.com/manticoresoftware/manticoresearch-helm/issues/56) “Kubernetes集群中严重崩溃转储”：修复了JSON根对象的错误布隆过滤器；修复了因JSON字段过滤导致的守护进程崩溃。
* [提交 6e1b](https://github.com/manticoresoftware/manticoresearch/commit/6e1b849) 修正了因无效`manticore.json`配置导致的守护进程崩溃。
* [提交 6fbc](https://github.com/manticoresoftware/manticoresearch/commit/6fbc4189) 修复了json范围过滤器以支持int64值。
* [提交 9c67](https://github.com/manticoresoftware/manticoresearch/commit/9c677426) 修复了`.sph`文件在`ALTER`时可能损坏的问题。
* [提交 77cc](https://github.com/manticoresoftware/manticoresearch/commit/77ccd8f): 添加了用于替换语句复制的共享密钥，以解决从多个主节点复制替换时出现的`pre_commit`错误。
* [提交 2884](https://github.com/manticoresoftware/manticoresearch/commit/2884519e) 解决了对函数如'date_format()'进行bigint检查的问题。
* [提交 9513](https://github.com/manticoresoftware/manticoresearch/commit/9513aca5): 当排序器使用预计算数据时，迭代器不再显示在[SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META)中。
* [提交 a2a7](https://github.com/manticoresoftware/manticoresearch/commit/a2a70555): 更新了全文节点堆栈大小，防止复杂全文查询崩溃。
* [提交 a062](https://github.com/manticoresoftware/manticoresearch/commit/a0626d7e): 修复了带有JSON和字符串属性的更新复制过程中导致崩溃的错误。
* [提交 b3e6](https://github.com/manticoresoftware/manticoresearch/commit/b3e63e5d): 更新了字符串构建器，使用64位整数以避免处理大型数据集时崩溃。
* [提交 c472](https://github.com/manticoresoftware/manticoresearch/commit/c472e5b): 解决了跨多个索引执行count distinct时的崩溃。
* [提交 d073](https://github.com/manticoresoftware/manticoresearch/commit/d0730272): 修复了即使禁用`pseudo_sharding`，RT索引磁盘区块的查询仍可能在多线程中执行的问题。
* [提交 d205](https://github.com/manticoresoftware/manticoresearch/commit/d205508e) `show index status`命令返回的值集合已修改，现根据使用的索引类型不同而变化。
* [提交 e9bc](https://github.com/manticoresoftware/manticoresearch/commit/e9bc2f9) 修复了处理批量请求时的HTTP错误及错误未从网络循环返回给客户端的问题。
* [提交 f77c](https://github.com/manticoresoftware/manticoresearch/commit/f77ce0e6) 为PQ使用了扩展堆栈。
* [Commit fac2](https://github.com/manticoresoftware/manticoresearch/commit/fac2d9687a20582765aaa89be9597804c4f845af) 更新了导出排名器输出以与 [packedfactors()](Functions/Searching_and_ranking_functions.md#PACKEDFACTORS%28%29) 对齐。
* [Commit ff87](https://github.com/manticoresoftware/manticoresearch/commit/ff87cc4): 修复了 SphinxQL 查询日志中过滤器中字符串列表的问题。
* [Issue #589](https://github.com/manticoresoftware/manticoresearch/issues/589) “字符集定义似乎依赖于代码排序”：修正了重复项的字符集映射错误。
* [Issue #811](https://github.com/manticoresoftware/manticoresearch/issues/811) “在词形映射多个词时，带有 CJK 标点的短语搜索受干扰”：修复了带有词形映射的短语查询中 ngram 标记位置的问题。
* [Issue #834](https://github.com/manticoresoftware/manticoresearch/issues/834) “搜索查询中的等号导致请求失败”：确保精确符号可以转义，并修正了 `expand_keywords` 选项导致的双重精确扩展。
* [Issue #864](https://github.com/manticoresoftware/manticoresearch/issues/864) “exceptions/stopwords 冲突”
* [Issue #910](https://github.com/manticoresoftware/manticoresearch/issues/910) “调用 call snippets() 使用 libstemmer_fr 和 index_exact_words 时 Manticore 崩溃”：解决了调用 `SNIPPETS()` 时导致崩溃的内部冲突。
* [Issue #946](https://github.com/manticoresoftware/manticoresearch/issues/946) “SELECT 期间出现重复记录”：修正了带有 `not_terms_only_allowed` 选项和已删除文档的 RT 索引查询结果中出现重复文档的问题。
* [Issue #967](https://github.com/manticoresoftware/manticoresearch/issues/967) “在 UDF 函数中使用 JSON 参数导致崩溃”：修复了启用伪分片和带有 JSON 参数的 UDF 搜索处理时守护进程崩溃的问题。
* [Issue #1050](https://github.com/manticoresoftware/manticoresearch/issues/1050) “FEDERATED 中的 count(*)”：修复了通过带聚合的 `FEDERATED` 引擎查询时发生的守护进程崩溃。
* [Issue #1052](https://github.com/manticoresoftware/manticoresearch/issues/1052) 修复了 `rt_attr_json` 列与列存储不兼容的问题。
* [Issue #1072](https://github.com/manticoresoftware/manticoresearch/issues/1072) “被 ignore_chars 移除搜索查询中的 *”：修复该问题，使查询中的通配符不会受到 `ignore_chars` 的影响。
* [Issue #1075](https://github.com/manticoresoftware/manticoresearch/issues/1075) “indextool --check 如果存在分布式表则失败”：indextool 现在兼容配置 JSON 中包含“distributed”和“template”索引的实例。
* [Issue #1081](https://github.com/manticoresoftware/manticoresearch/issues/1081) “针对某 RT 数据集的特定 SELECT 导致 searchd 崩溃”：解决了包含 packedfactors 和大内部缓冲区的查询导致的守护进程崩溃。
* [Issue #1095](https://github.com/manticoresoftware/manticoresearch/issues/1095) “带 not_terms_only_allowed 时删除的文档被忽略”
* [Issue #1099](https://github.com/manticoresoftware/manticoresearch/issues/1099) “indextool --dumpdocids 不工作”：恢复了 `--dumpdocids` 命令的功能。
* [Issue #1100](https://github.com/manticoresoftware/manticoresearch/issues/1100) “indextool --buildidf 不工作”：indextool 现在线程完成 globalidf 后关闭文件。
* [Issue #1104](https://github.com/manticoresoftware/manticoresearch/issues/1104) “Count(*) 在远程表中被误处理为 schema 集”：解决了当代理返回空结果集，针对分布式索引的查询导致守护进程发送错误消息的问题。
* [Issue #1109](https://github.com/manticoresoftware/manticoresearch/issues/1109) “FLUSH ATTRIBUTES 在 threads=1 时挂起”。
* [Issue #1126](https://github.com/manticoresoftware/manticoresearch/issues/1126) “查询期间丢失与 MySQL 服务器的连接 - manticore 6.0.5”：解决了多过滤器应用于列存储属性时发生的崩溃问题。
* [Issue #1135](https://github.com/manticoresoftware/manticoresearch/issues/1135) “JSON 字符串过滤的大小写敏感性”：更正了校对以正确处理 HTTP 搜索请求中使用的过滤器。
* [Issue #1140](https://github.com/manticoresoftware/manticoresearch/issues/1140) “错误字段中的匹配”：修复了与 `morphology_skip_fields` 相关的问题。
* [Issue #1155](https://github.com/manticoresoftware/manticoresearch/issues/1155) “通过 API 的系统远程命令应传递 g_iMaxPacketSize”：进行了更新，以绕过节点间复制命令的 `max_packet_size` 检查。此外，最新集群错误已添加到状态显示。
* [Issue #1302](https://github.com/manticoresoftware/manticoresearch/issues/1302) “优化失败后留下临时文件”：修正了在合并或优化过程中出错后遗留临时文件的问题。
* [Issue #1304](https://github.com/manticoresoftware/manticoresearch/issues/1304) “添加处理伙伴启动超时的环境变量”：添加环境变量 `MANTICORE_BUDDY_TIMEOUT`（默认 3 秒）来控制守护进程启动时等待伙伴消息的时间。
* [Issue #1305](https://github.com/manticoresoftware/manticoresearch/issues/1305) “保存 PQ 元数据时的整型溢出”：减少了守护进程在保存大型 PQ 索引时的过度内存消耗。
* [Issue #1306](https://github.com/manticoresoftware/manticoresearch/issues/1306) “修改外部文件后无法重新创建 RT 表”：修正了外部文件为空字符串时的修改错误；修复了修改外部文件后遗留的 RT 索引外部文件。
* [Issue #1307](https://github.com/manticoresoftware/manticoresearch/issues/1307) “SELECT 语句 sum(value) as value 无法正确工作”：修复了带别名的选择列表表达式可能遮蔽索引属性的问题；还修正了整数类型中 sum 转换为 int64 计数的问题。
* [Issue #1308](https://github.com/manticoresoftware/manticoresearch/issues/1308) “避免在复制中绑定到localhost”：确保复制不会绑定到具有多个IP的主机名的localhost。
* [Issue #1309](https://github.com/manticoresoftware/manticoresearch/issues/1309) “回复MySQL客户端失败，数据大于16Mb”：修复了向客户端返回大于16Mb的SphinxQL包的问题。
* [Issue #1310](https://github.com/manticoresoftware/manticoresearch/issues/1310) ““外部文件路径应为绝对路径”中的错误引用”：修正了`SHOW CREATE TABLE`中外部文件的完整路径显示。
* [Issue #1311](https://github.com/manticoresoftware/manticoresearch/issues/1311) “调试版本在片段中遇长字符串时崩溃”：现在`SNIPPET()`函数目标文本允许长字符串（超过255个字符）。
* [Issue #1312](https://github.com/manticoresoftware/manticoresearch/issues/1312) “在kqueue轮询中使用删除后数据导致的虚假崩溃（master-agent）”：修复了在kqueue驱动系统（FreeBSD、MacOS等）主服务器无法连接到代理时的崩溃问题。
* [Issue #1313](https://github.com/manticoresoftware/manticoresearch/issues/1313) “与自身连接时间过长”：在MacOS/BSD上从主服务器连接到代理时，现在使用统一的连接+查询超时，而不仅仅是连接超时。
* [Issue #1314](https://github.com/manticoresoftware/manticoresearch/issues/1314) “pq（json元数据）含未触及的嵌入式同义词加载失败”：修复了pq中的嵌入式同义词标志。
* [Issue #1315](https://github.com/manticoresoftware/manticoresearch/issues/1315) “允许部分函数（sint，fibonacci，second，minute，hour，day，month，year，yearmonth，yearmonthday）隐式使用提升的参数值”。
* [Issue #1321](https://github.com/manticoresoftware/manticoresearch/issues/1321) “启用fullscan中的多线程二级索引，但限制线程数”：在CBO中实现了代码，用以更好地预测在全文查询中使用二级索引时的多线程性能。
* [Issue #1322](https://github.com/manticoresoftware/manticoresearch/issues/1322) “使用预计算排序器后count(*)查询仍然缓慢”：使用预计算数据的排序器时不再初始化迭代器，避免了性能下降。
* [Issue #1411](https://github.com/manticoresoftware/manticoresearch/issues/1141) “sphinxql的查询日志未保存多值属性(MVA)的原始查询”：现在`all()/any()`被记录。

## 版本 6.0.4
发布时间：2023年3月15日

### 新功能
* 改进了与Logstash、Beats等的集成，包括：
  - 支持Logstash版本7.6 - 7.15，Filebeat版本7.7 - 7.12
  - 自动模式支持。
  - 添加了对类似Elasticsearch格式的批量请求处理。
* [Buddy commit ce90](https://github.com/manticoresoftware/manticoresearch-buddy/commit/ce907ea) 启动时显示Buddy版本。

### Bug修复
* [Issue #588](https://github.com/manticoresoftware/manticoresearch/issues/588), [Issue #942](https://github.com/manticoresoftware/manticoresearch/issues/942) 修复了大ram索引搜索元数据中的坏字符和调用关键词问题。
* [Issue #1027](https://github.com/manticoresoftware/manticoresearch/issues/1027) 小写HTTP头部被拒绝。
* ❗[Issue #1039](https://github.com/manticoresoftware/manticoresearch/issues/1039) 修复了守护进程在读取Buddy控制台输出时的内存泄漏。
* [Issue #1056](https://github.com/manticoresoftware/manticoresearch/issues/1056) 修正了问号的异常行为。
* [Issue #1064](https://github.com/manticoresoftware/manticoresearch/issues/1064) - 修复了分词器小写表的竞态条件引发的崩溃。
* [Commit 59bb](https://github.com/manticoresoftware/manticoresearch/commit/59bb54c) 修正了JSON接口中针对id显式设为null的文档的批量写入处理。
* [Commit 7b6b](https://github.com/manticoresoftware/manticoresearch/commit/7b6b25f) 修正了CALL KEYWORDS中多次出现相同词的术语统计。
* [Commit f381](https://github.com/manticoresoftware/manticoresearch/commit/f381ad2) Windows安装程序现默认创建配置；运行时不再替换路径。
* [Commit 6940](https://github.com/manticoresoftware/manticoresearch/commit/6940e95), [Commit cc5a](https://github.com/manticoresoftware/manticoresearch/commit/cc5a480) 修复了多个网络节点集群的复制问题。
* [Commit 4972](https://github.com/manticoresoftware/manticoresearch/commit/49722ab) 修正了`/pq` HTTP端点，使其成为`/json/pq` HTTP端点的别名。
* [Commit 3b53](https://github.com/manticoresoftware/manticoresearch/commit/3b5385a) 修复了Buddy重启时守护进程崩溃的问题。
* [Buddy commit fba9](https://github.com/manticoresoftware/manticoresearch-buddy/commit/fba9c8c) 显示无效请求时的原始错误。
* [Buddy commit db95](https://github.com/manticoresoftware/manticoresearch-buddy/commit/db9532c) 允许备份路径中包含空格，并对正则表达式做了调整以支持单引号。

## 版本 6.0.2
发布时间：2023年2月10日

### Bug修复
* [Issue #1024 crash 2](https://github.com/manticoresoftware/manticoresearch/issues/1024) Facet搜索大量结果时崩溃/段错误。
* ❗[Issue #1029](https://github.com/manticoresoftware/manticoresearch/issues/1029) - 警告：编译时值KNOWN_CREATE_SIZE（16）小于测量值（208）。建议修正此值！
* ❗[Issue #1032](https://github.com/manticoresoftware/manticoresearch/issues/1032) - Manticore 6.0.0 plain索引崩溃。
* ❗[Issue #1033](https://github.com/manticoresoftware/manticoresearch/issues/1033) - 守护程序重启时多个分布式实例丢失。
* ❗[Issue #1064](https://github.com/manticoresoftware/manticoresearch/issues/1064) - 分词器小写表的竞态条件。

## 版本 6.0.0
发布时间：2023年2月7日

从本次版本开始，Manticore Search 配备了 Manticore Buddy，这是一个用 PHP 编写的旁车守护进程，负责处理不要求超低延迟或高吞吐量的高级功能。Manticore Buddy 在幕后运行，您甚至可能不会意识到它在运行。尽管对最终用户是不可见的，但使 Manticore Buddy 易于安装且与基于 C++ 的主守护进程兼容是一个重大挑战。这一重大变化将使团队能够开发广泛的新高级功能，例如分片协调、访问控制和认证，以及各种集成，如 mysqldump、DBeaver、Grafana mysql 连接器。目前它已经处理 [SHOW QUERIES](Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES)、[BACKUP](Securing_and_compacting_a_table/Backup_and_restore.md#BACKUP-SQL-command-reference) 和 [自动模式](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema)。

本次发布还包括 130 多个错误修复和众多功能，其中许多可以被视为重大改进。

### 主要更改
* 🔬 实验性：您现在可以执行 Elasticsearch 兼容的 [insert](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md?example=Elasticsearch#Adding-documents-to-a-real-time-table) 和 [replace](Data_creation_and_modification/Updating_documents/REPLACE.md?example=Elasticsearch#REPLACE) JSON 查询，使得可以使用 Manticore 与 Logstash（7.13 以下版本）、Filebeat 以及 Beats 系列的其他工具配合使用。默认启用。您可以使用 `SET GLOBAL ES_COMPAT=off` 禁用它。
* 支持 [Manticore Columnar Library 2.0.0](https://github.com/manticoresoftware/columnar/) ，在 [二级索引](Server_settings/Searchd.md#secondary_indexes) 中包含众多修复和改进。**⚠️ 突破性更改**：从本版本起，二级索引默认开启。如果您是从 Manticore 5 升级，请确保执行 [ALTER TABLE table_name REBUILD SECONDARY](Updating_table_schema_and_settings.md#Rebuilding-a-secondary-index)。详情见下文。
* [Commit c436](https://github.com/manticoresoftware/manticoresearch/commit/c436f9023536f767610451911955ae36d90aa638) 自动模式：您现在可以跳过创建表的步骤，只需插入首个文档，Manticore 将根据其字段自动创建表。详细信息见 [这里](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema)。您可以使用 [searchd.auto_schema](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema) 来启用或关闭该功能。
* 对 [基于代价的优化器](Searching/Cost_based_optimizer.md) 进行了全面改进，许多情况下降低了查询响应时间。
  - [Issue #1008](https://github.com/manticoresoftware/manticoresearch/issues/1008) CBO 中的并行化性能估计。
  - [Issue #1014](https://github.com/manticoresoftware/manticoresearch/issues/1014) CBO 现在了解 [二级索引](Server_settings/Searchd.md#secondary_indexes)，表现更加智能。
  - [Commit cef9](https://github.com/manticoresoftware/columnar/commit/cef9b54dc3a4ee9138ebc4df3a671715b996d6b3) 将列式表/字段的编码统计信息存储到元数据中，以帮助 CBO 做出更智能的决策。
  - [Commit 2b95](https://github.com/manticoresoftware/manticoresearch/commit/2b95fb8c2ba422c9c5a66e2b61c1f7922f31c7aa) 添加了用于微调行为的 CBO 提示。
* [遥测](Telemetry.md#Telemetry)：我们高兴地宣布本版本新增了遥测功能。此功能允许我们收集匿名和非个人化的指标，帮助改进产品的性能和用户体验。请放心，所有收集的数据**完全匿名且不会关联任何个人信息**。如果需要，该功能可以在设置中[轻松关闭](Telemetry.md#Telemetry)。
* [Commit 5aaf](https://github.com/manticoresoftware/manticoresearch/commit/5aafb17d81b601e399e212a344a4738f65576cdd) [ALTER TABLE table_name REBUILD SECONDARY](Updating_table_schema_and_settings.md#Rebuilding-a-secondary-index) 用于在任意时间重建二级索引，例如：
  - 从 Manticore 5 迁移到新版本时，
  - 对索引中属性执行了 [UPDATE](Data_creation_and_modification/Updating_documents/UPDATE.md#UPDATE)（即[就地更新，而非替换](Data_creation_and_modification/Updating_documents/REPLACE_vs_UPDATE.md)）
* [Issue #821](https://github.com/manticoresoftware/manticoresearch/issues/821) 新工具 `manticore-backup` 用于 [备份和恢复 Manticore 实例](Securing_and_compacting_a_table/Backup_and_restore.md)
* SQL 命令 [BACKUP](Securing_and_compacting_a_table/Backup_and_restore.md#BACKUP-SQL-command-reference) 支持在 Manticore 内部执行备份。
* SQL 命令 [SHOW QUERIES](Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES) 提供了查看正在运行查询的简单方法，而非线程。
* [Issue #551](https://github.com/manticoresoftware/manticoresearch/issues/551) SQL 命令 `KILL` 用于终止长时间运行的 `SELECT`。
* 动态调整聚合查询的 `max_matches`，提高精确度并降低响应时间。

### 次要更改
* [Issue #822](https://github.com/manticoresoftware/manticoresearch/issues/822) SQL 命令 [FREEZE/UNFREEZE](Securing_and_compacting_a_table/Freezing_a_table.md) 用于为备份准备实时/普通表。
* [Commit c470](https://github.com/manticoresoftware/manticoresearch/commit/c47052f18ad4868134de1a97e8d7da570e531dfa) 新增设置 `accurate_aggregation` 和 `max_matches_increase_threshold`，用于控制聚合精度。
* [Issue #718](https://github.com/manticoresoftware/manticoresearch/issues/718) 支持带符号的负 64 位 ID。注意，您仍然不能使用 ID > 2^63，但现在可以使用范围为 -2^63 到 0 的 ID。
* 随着我们最近添加了对二级索引的支持，出现了混淆，因为“index”可能指二级索引、全文索引或普通/实时 `index`。为减少混淆，我们将后者重命名为“table”。以下SQL/命令行命令受此更改影响。旧版本已弃用，但仍可使用：
  - `index <table name>` => `table <table name>`，
  - `searchd -i / --index` => `searchd -t / --table`，
  - `SHOW INDEX STATUS` => `SHOW TABLE STATUS`，
  - `SHOW INDEX SETTINGS` => `SHOW TABLE SETTINGS`，
  - `FLUSH RTINDEX` => `FLUSH TABLE`，
  - `OPTIMIZE INDEX` => `OPTIMIZE TABLE`，
  - `ATTACH TABLE plain TO RTINDEX rt` => `ATTACH TABLE plain TO TABLE rt`，
  - `RELOAD INDEX` => `RELOAD TABLE`，
  - `RELOAD INDEXES` => `RELOAD TABLES`。

  我们不打算让旧形式过时，但为确保与文档兼容，建议在您的应用中更改名称。未来版本将把各种SQL和JSON命令输出中的“index”重命名为“table”。
* 使用有状态UDF的查询现在强制单线程执行。
* [Issue #1011](https://github.com/manticoresoftware/manticoresearch/issues/1011) 重构所有与时间调度相关的部分，为并行块合并做准备。
* **⚠️ 重大变更**：列存储格式已更改。需要重建含有列属性的那些表。
* **⚠️ 重大变更**：二级索引文件格式已更改，如果您在配置文件中使用了 `searchd.secondary_indexes = 1` 并使用二级索引进行搜索，请注意新版本Manticore **将跳过加载含有二级索引的表**。建议操作为：
  - 升级前将配置文件中的 `searchd.secondary_indexes` 改为0。
  - 启动实例。Manticore 将加载表并发出警告。
  - 对每个索引运行 `ALTER TABLE <table name> REBUILD SECONDARY` 来重建二级索引。

  如果您运行复制集群，需要在所有节点上运行 `ALTER TABLE <table name> REBUILD SECONDARY`，或遵循[此说明](Securing_and_compacting_a_table/Compacting_a_table.md#Optimizing-clustered-tables)但将 `ALTER .. REBUILD SECONDARY` 替代 `OPTIMIZE`。
* **⚠️ 重大变更**：binlog版本已更新，因此之前版本的binlog不会被重放。升级过程中需确保Manticore Search干净停止。即停止之前的实例后，`/var/lib/manticore/binlog/` 下除 `binlog.meta` 外不应有binlog文件。
* [Issue #849](https://github.com/manticoresoftware/manticoresearch/issues/849) `SHOW SETTINGS`：现在可以在Manticore内部查看配置文件中的设置。
* [Issue #1007](https://github.com/manticoresoftware/manticoresearch/issues/1007) [SET GLOBAL CPUSTATS=1/0](Server_settings/Setting_variables_online.md#SET) 开启/关闭CPU时间追踪；[SHOW THREADS](Node_info_and_management/SHOW_THREADS.md) 关闭CPU时间追踪时不显示CPU统计信息。
* [Issue #1009](https://github.com/manticoresoftware/manticoresearch/issues/1009) 实时表RAM块段现在可在RAM块刷新时进行合并。
* [Issue #1012](https://github.com/manticoresoftware/manticoresearch/issues/1012) 在 [indexer](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) 输出中增加了二级索引进度。
* [Issue #1013](https://github.com/manticoresoftware/manticoresearch/issues/1013) 之前如果Manticore启动时无法提供服务，表记录会从索引列表中移除。新行为是在列表中保留该记录，尝试下一次启动加载。
* [indextool --docextract](Miscellaneous_tools.md#indextool) 返回所请求文档的所有单词和命中记录。
* [Commit 2b29](https://github.com/manticoresoftware/manticoresearch/commit/2b296ee20e520b85bcbb4383e87095d31e7165dc) 环境变量 `dump_corrupt_meta` 允许在 searchd 无法加载索引时将损坏的表元数据转储到日志。
* [Commit c7a3](https://github.com/manticoresoftware/manticoresearch/commit/c7a3136eefe0eb2f69e3d99fae9c10beb5821ea6) `DEBUG META` 可以显示 `max_matches` 及伪分片统计。
* [Commit 6bca](https://github.com/manticoresoftware/manticoresearch/commit/6bca4848dc5c8396dbc1e057be4b9b2d579794e8) 更好的错误提示，替代之前令人迷惑的“Index header format is not json, will try it as binary...”。
* [Commit bef3](https://github.com/manticoresoftware/lemmatizer-uk/commit/bef3ff0386d3ee87ec57619782100972c1122e47) 乌克兰语词形还原器路径已更改。
* [Commit 4ae7](https://github.com/manticoresoftware/manticoresearch/commit/4ae789595329a2951e194d1191ddb3121459a560) 在 [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META) 中增加二级索引统计信息。
* [Commit 2e7c](https://github.com/manticoresoftware/manticoresearch/commit/2e7c585e) JSON接口现在可以通过Swagger Editor轻松可视化 https://manual.manticoresearch.com/Openapi#OpenAPI-specification。
* **⚠️ 重大变更**：复制协议已更改。如果您运行复制集群，升级到Manticore 5时需要：
  - 首先干净停止所有节点
  - 然后用 `--new-cluster` 启动最后停止的节点（Linux下运行工具 `manticore_new_cluster`）。
  - 更多细节请参阅 [重启集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster)。

### 与 Manticore 列存库相关的变更
* 重构二级索引与列存储的集成。
* [Commit efe2](https://github.com/manticoresoftware/columnar/commit/efe26b2a) Manticore列存库优化，通过局部预评估最小/最大值降低响应时间。
* [Commit 2757](https://github.com/manticoresoftware/columnar/commit/2757b99b) 如果磁盘块合并被中断，守护进程现在会清理与 MCL 相关的临时文件。
* [Commit e9c6](https://github.com/manticoresoftware/manticoresearch/commit/e9c672014b63e2b49e7a5a53880583af86c3ab4e) 在崩溃时，列式和二级库版本信息会被写入日志。
* [Commit f5e8](https://github.com/manticoresoftware/columnar/commit/f5e84eeb8b6cb102f13e3468246702427527fef9) 增加对快速文档列表倒带到二级索引的支持。
* [Commit 06df](https://github.com/manticoresoftware/manticoresearch/commit/06dfdd2e668933495dd6cb52d5ba2e0aa9b26e58) 在使用 MCL 的情况下，像 `select attr, count(*) from plain_index`（无过滤）的查询现在更快了。
* [Commit 0a76](https://github.com/manticoresoftware/manticoresearch/commit/0a76ecb9adbe886868b962dd425de5f475bbf55e) @@autocommit 在 HandleMysqlSelectSysvar 中，以兼容 MySQL 8.25 以上的 .net 连接器。
* **⚠️ 重大变更**： [MCL Issue #17](https://github.com/manticoresoftware/columnar/issues/17) MCL：向列式扫描添加 SSE 代码。MCL 现在至少需要 SSE4.2 支持。

### 与打包相关的变更
* [Commit 4d19](https://github.com/manticoresoftware/manticoresearch/commit/4d19f5cbe49a31228aa09253f061165bfe80e51b) **⚠️ 重大变更**：不再支持 Debian Stretch 和 Ubuntu Xenial。
* 支持 RHEL 9，包括 Centos 9、Alma Linux 9 和 Oracle Linux 9。
* [Issue #924](https://github.com/manticoresoftware/manticoresearch/issues/924) 支持 Debian Bookworm。
* [Issue #636](https://github.com/manticoresoftware/manticoresearch/issues/636) 打包：支持 Linux 和 MacOS 的 arm64 构建。
* [PR #26](https://github.com/manticoresoftware/docker/pull/26) 多架构（x86_64 / arm64）Docker 镜像。
* [简化了贡献者的包构建](Installation/Compiling_from_sources.md#Building-using-CI-Docker)。
* 现在可以使用 APT 安装特定版本。
* [Commit a6b8](https://github.com/manticoresoftware/manticoresearch/commit/51fddca5c2a3ebb8576fae4c18660656ba22de0f) Windows 安装程序（之前只提供了压缩包）。
* 切换到使用 CLang 15 编译。
* **⚠️ 重大变更**：自定义 Homebrew 配方，包括 Manticore 列式库的配方。安装 Manticore、MCL 及其他必要组件，请使用以下命令 `brew install manticoresoftware/manticore/manticoresearch manticoresoftware/manticore/manticore-extra`。

### Bug 修复
* [Issue #479](https://github.com/manticoresoftware/manticoresearch/issues/479) 字段名为 `text` 的问题。
* [Issue #501](https://github.com/manticoresoftware/manticoresearch/issues/501) id 不能是非 bigint 类型问题。
* [Issue #646](https://github.com/manticoresoftware/manticoresearch/issues/646) ALTER 与字段名为 "text" 的冲突问题。
* ❗[Issue #652](https://github.com/manticoresoftware/manticoresearch/issues/652) 可能的 BUG：HTTP (JSON) 的 offset 和 limit 影响选择面结果。
* ❗[Issue #827](https://github.com/manticoresoftware/manticoresearch/issues/827) 在高负载下 searchd 挂起/崩溃问题。
* ❗[Issue #996](https://github.com/manticoresoftware/manticoresearch/issues/996) PQ 索引内存溢出。
* ❗[Commit 1041](https://github.com/manticoresoftware/manticoresearch/commit/10416ef7dddf06c0d759e32ccd6ebaa2468f7cbf) `binlog_flush = 1` 从 Sphinx 以来一直损坏。已修复。
* [MCL Issue #14](https://github.com/manticoresoftware/columnar/issues/14) MCL：当全文字段过多时选择崩溃。
* [Issue #470](https://github.com/manticoresoftware/manticoresearch/issues/470) sql_joined_field 不能存储。
* [Issue #713](https://github.com/manticoresoftware/manticoresearch/issues/713) 使用 LEVENSHTEIN() 时崩溃。
* [Issue #743](https://github.com/manticoresoftware/manticoresearch/issues/743) Manticore 意外崩溃且无法正常重启。
* [Issue #788](https://github.com/manticoresoftware/manticoresearch/issues/788) 通过 /sql 调用 KEYWORDS 返回控制字符，导致 json 解析失败。
* [Issue #789](https://github.com/manticoresoftware/manticoresearch/issues/789) mariadb 无法创建 FEDERATED 表。
* [Issue #796](https://github.com/manticoresoftware/manticoresearch/issues/796) 警告：dlopen() 失败: /usr/bin/lib_manticore_columnar.so: 无法打开共享对象文件：没有此文件或目录。
* [Issue #797](https://github.com/manticoresoftware/manticoresearch/issues/797) 通过 api 使用 ZONESPAN 搜索时 Manticore 崩溃。
* [Issue #799](https://github.com/manticoresoftware/manticoresearch/issues/799) 使用多个索引和 facet distinct 时权重错误。
* [Issue #801](https://github.com/manticoresoftware/manticoresearch/issues/801) SphinxQL 组查询在 SQL 索引重处理后挂起。
* [Issue #802](https://github.com/manticoresoftware/manticoresearch/issues/802) MCL：索引器在 5.0.2 和 manticore-columnar-lib 1.15.4 中崩溃。
* [Issue #813](https://github.com/manticoresoftware/manticoresearch/issues/813) Manticore 5.0.2 FEDERATED 返回空集合（MySQL 8.0.28）。
* [Issue #824](https://github.com/manticoresoftware/manticoresearch/issues/824) 当 COUNT DISTINCT 在两个索引上查询结果为零时抛出内部错误。
* [Issue #826](https://github.com/manticoresoftware/manticoresearch/issues/826) 删除查询导致崩溃。
* [Issue #843](https://github.com/manticoresoftware/manticoresearch/issues/843) MCL：长文本字段的 BUG。
* [Issue #856](https://github.com/manticoresoftware/manticoresearch/issues/856) 5.0.2 rtindex：聚合搜索限制行为不符合预期。
* [Issue #863](https://github.com/manticoresoftware/manticoresearch/issues/863) 即使是应该返回多个结果的搜索，返回的 Hits 也是 Nonetype 对象。
* [Issue #870](https://github.com/manticoresoftware/manticoresearch/issues/870) 在 SELECT 表达式中使用 Attribute 和存储字段时崩溃。
* [Issue #872](https://github.com/manticoresoftware/manticoresearch/issues/872) 崩溃后表不可见。
* [Issue #877](https://github.com/manticoresoftware/manticoresearch/issues/877) 搜索查询中存在两个否定项会导致错误：查询不可计算
* [Issue #878](https://github.com/manticoresoftware/manticoresearch/issues/878) 通过 json query_string 执行 a -b -c 不起作用
* [Issue #886](https://github.com/manticoresoftware/manticoresearch/issues/886) pseudo_sharding 与 query match
* [Issue #893](https://github.com/manticoresoftware/manticoresearch/issues/893) Manticore 5.0.2 中 min/max 函数不能如预期工作...
* [Issue #896](https://github.com/manticoresoftware/manticoresearch/issues/896) 字段 "weight" 解析不正确
* [Issue #897](https://github.com/manticoresoftware/manticoresearch/issues/897) Manticore 服务启动时崩溃且持续重启
* [Issue #900](https://github.com/manticoresoftware/manticoresearch/issues/900) group by j.a, smth 工作异常
* [Issue #913](https://github.com/manticoresoftware/manticoresearch/issues/913) 当在 ranker 中使用 expr 时 Searchd 崩溃，但仅限于带有两个接近操作的查询
* [Issue #916](https://github.com/manticoresoftware/manticoresearch/issues/916) net_throttle_action 已损坏
* [Issue #919](https://github.com/manticoresoftware/manticoresearch/issues/919) MCL：查询执行时 Manticore 崩溃，集群恢复期间其他崩溃。
* [Issue #925](https://github.com/manticoresoftware/manticoresearch/issues/925) SHOW CREATE TABLE 输出不带反引号
* [Issue #930](https://github.com/manticoresoftware/manticoresearch/issues/930) 现在可以通过 JDBC 连接器从 Java 查询 Manticore
* [Issue #933](https://github.com/manticoresoftware/manticoresearch/issues/933) bm25f 排名问题
* [Issue #934](https://github.com/manticoresoftware/manticoresearch/issues/934) 无配置索引在 watchdog 中首次加载时冻结
* [Issue #937](https://github.com/manticoresoftware/manticoresearch/issues/937) 排序分类数据时发生段错误
* [Issue #940](https://github.com/manticoresoftware/manticoresearch/issues/940) 使用 CONCAT(TO_STRING) 时崩溃
* [Issue #947](https://github.com/manticoresoftware/manticoresearch/issues/947) 在某些情况下，单个简单的 select 查询可能导致整个实例停滞，无法登录或执行其他查询，直到该查询完成。
* [Issue #948](https://github.com/manticoresoftware/manticoresearch/issues/948) Indexer 崩溃
* [Issue #950](https://github.com/manticoresoftware/manticoresearch/issues/950) facet distinct 计数错误
* [Issue #953](https://github.com/manticoresoftware/manticoresearch/issues/953) 内置 sph04 ranker 中 LCS 计算错误
* [Issue #955](https://github.com/manticoresoftware/manticoresearch/issues/955) 5.0.3 开发版崩溃
* [Issue #963](https://github.com/manticoresoftware/manticoresearch/issues/963) 在引擎 columnar 上使用 FACET 和 json 导致崩溃
* [Issue #982](https://github.com/manticoresoftware/manticoresearch/issues/982) MCL：5.0.3 从二级索引崩溃
* [PR #984](https://github.com/manticoresoftware/manticoresearch/pull/984) 在 HandleMysqlSelectSysvar 中加入 @@autocommit
* [PR #985](https://github.com/manticoresoftware/manticoresearch/pull/985) 修复 RT 索引中的线程块分配
* [Issue #985](https://github.com/manticoresoftware/manticoresearch/issues/985) 修复 RT 索引中的线程块分配
* [Issue #986](https://github.com/manticoresoftware/manticoresearch/issues/986) 默认 max_query_time 错误
* [Issue #987](https://github.com/manticoresoftware/manticoresearch/issues/987) 在多线程执行中使用正则表达式查询时崩溃
* [Issue #988](https://github.com/manticoresoftware/manticoresearch/issues/988) 反向索引兼容性损坏
* [Issue #989](https://github.com/manticoresoftware/manticoresearch/issues/989) indextool 报告检查 columnar 属性时错误
* [Issue #990](https://github.com/manticoresoftware/manticoresearch/issues/990) json grouper 克隆发生内存泄漏
* [Issue #991](https://github.com/manticoresoftware/manticoresearch/issues/991) levenshtein 函数克隆内存泄漏
* [Issue #992](https://github.com/manticoresoftware/manticoresearch/issues/992) 加载元数据时错误信息丢失
* [Issue #993](https://github.com/manticoresoftware/manticoresearch/issues/993) 传播来自动态索引/子键及系统变量的错误
* [Issue #994](https://github.com/manticoresoftware/manticoresearch/issues/994) 在列存储上针对列式字符串执行 count distinct 崩溃
* [Issue #995](https://github.com/manticoresoftware/manticoresearch/issues/995) MCL：taxi1 的 min(pickup_datetime) 查询崩溃
* [Issue #997](https://github.com/manticoresoftware/manticoresearch/issues/997) 空的 excludes JSON 查询会从选择列表中移除列
* [Issue #998](https://github.com/manticoresoftware/manticoresearch/issues/998) 二级任务在当前调度器上运行有时导致异常副作用
* [Issue #999](https://github.com/manticoresoftware/manticoresearch/issues/999) facet distinct 与不同架构时崩溃
* [Issue #1000](https://github.com/manticoresoftware/manticoresearch/issues/1000) MCL：未加载 columnar 库情况下运行导致列式 rt 索引损坏
* [Issue #1001](https://github.com/manticoresoftware/manticoresearch/issues/1001) json 中的隐式 cutoff 无效
* [Issue #1002](https://github.com/manticoresoftware/manticoresearch/issues/1002) 列式 grouper 问题
* [Issue #1003](https://github.com/manticoresoftware/manticoresearch/issues/1003) 无法删除索引中的最后一个字段
* [Issue #1004](https://github.com/manticoresoftware/manticoresearch/issues/1004) --new-cluster 之后行为异常
* [Issue #1005](https://github.com/manticoresoftware/manticoresearch/issues/1005) 报告 "columnar library not loaded"，但其实并不需要
* [Issue #1006](https://github.com/manticoresoftware/manticoresearch/issues/1006) 删除查询无错误提示
* [Issue #1010](https://github.com/manticoresoftware/manticoresearch/issues/1010) 修复 Windows 构建中的 ICU 数据文件位置
* [PR #1018](https://github.com/manticoresoftware/manticoresearch/pull/1018) 握手发送问题
* [Issue #1020](https://github.com/manticoresoftware/manticoresearch/issues/1020) 在 show create table 中显示 id
* [Issue #1024 crash 1](https://github.com/manticoresoftware/manticoresearch/issues/1024) 面向大数量结果的 Facet 搜索时崩溃/段错误。
* [Issue #1026](https://github.com/manticoresoftware/manticoresearch/issues/1026) RT 索引：当插入大量文档且 RAMchunk 满时，searchd “卡住” 永远不动
* [Commit 4739](https://github.com/manticoresoftware/manticoresearch/commit/4739dafd) 关闭时线程卡住，同时节点间复制繁忙
* [Commit ab87](https://github.com/manticoresoftware/manticoresearch/commit/ab87836782e7ae43fe5f9dea739eed0d336b42c1) 在 JSON 范围过滤器中混用浮点数和整数可能导致 Manticore 忽略该过滤器
* [Commit d001](https://github.com/manticoresoftware/manticoresearch/commit/d00101c2905f6393ce3fca23c4f6dcb2506f4bd9) JSON 中的浮点数过滤不准确
* [Commit 4092](https://github.com/manticoresoftware/manticoresearch/commit/4092c6a8) 索引被更改时丢弃未提交的事务（否则可能崩溃）
* [Commit 9692](https://github.com/manticoresoftware/manticoresearch/commit/9692a840) 使用反斜杠时查询语法错误
* [Commit 0c19](https://github.com/manticoresoftware/manticoresearch/commit/0c19ad8d) SHOW STATUS 中的 workers_clients 可能错误
* [Commit 1772](https://github.com/manticoresoftware/manticoresearch/commit/17720e1e2f2ec38337df565bee78f7ce9d9c744a) 修复合并无 docstores 的内存段时的崩溃
* [Commit f45b](https://github.com/manticoresoftware/manticoresearch/commit/f45b337b5c265433bb7d6262ad65d02f65edbaf3) 修正 equals JSON 过滤器中遗漏的 ALL/ANY 条件
* [Commit 3e83](https://github.com/manticoresoftware/manticoresearch/commit/3e83c1f0c0e6e7a9d4297113f69a112cbabd9e64) 如果 searchd 从无写权限目录启动，复制时可能失败，出现 `got exception while reading ist stream: mkstemp(./gmb_pF6TJi) failed: 13 (Permission denied)`
* [Commit 92e5](https://github.com/manticoresoftware/manticoresearch/commit/92e5629f7150f62524b385fca670a01ba9590f5b) 4.0.2 之后崩溃日志只包含偏移，此提交修复该问题。

## 版本 5.0.2
发布：2022年5月30日

### Bug 修复
* ❗[Issue #791](https://github.com/manticoresoftware/manticoresearch/issues/791) - 错误的栈大小可能导致崩溃。

## 版本 5.0.0
发布：2022年5月18日


### 重大新特性
* 🔬 支持 [Manticore Columnar Library 1.15.2](https://github.com/manticoresoftware/columnar/)，启用 [二级索引](Server_settings/Searchd.md#secondary_indexes) 测试版。对于平面和实时列存储及行存储索引（如果使用了 [Manticore Columnar Library](https://github.com/manticoresoftware/columnar)），默认构建二级索引，但要启用搜索功能，需要在配置文件中或使用 [SET GLOBAL](Server_settings/Setting_variables_online.md) 设置 `secondary_indexes = 1`。新功能支持所有操作系统，旧版 Debian Stretch 和 Ubuntu Xenial 除外。
* [只读模式](Security/Read_only.md)：现在可以指定仅处理读取查询而丢弃所有写入的监听器。
* 新增 [/cli](Connecting_to_the_server/HTTP.md#/cli) 端点，使通过 HTTP 运行 SQL 查询更便捷。
* 通过 HTTP JSON 进行更快的批量 INSERT/REPLACE/DELETE：之前可以通过 HTTP JSON 协议提供多个写入命令，但逐一处理，现在作为单个事务处理。
* [#720](https://github.com/manticoresoftware/manticoresearch/issues/720) JSON 协议支持 [嵌套过滤器](Searching/Filters.md#Nested-bool-query)。之前不能在 JSON 中编码诸如 `a=1 and (b=2 or c=3)` 这样的复杂条件：`must`（AND）、`should`（OR）和 `must_not`（NOT）仅支持最高层级。现在支持嵌套。
* 支持 HTTP 协议中的 [分块传输编码](https://en.wikipedia.org/wiki/Chunked_transfer_encoding)。应用可使用分块传输发送大批量数据，减少资源消耗（无需计算 `Content-Length`）。服务器端，Manticore 现在始终以流式方式处理传入 HTTP 数据，无需等待整个批量传输完成，与之前不同，这能：
  - 降低峰值内存使用，降低 OOM 风险
  - 缩短响应时间（测试显示处理 100MB 批量时减少约 11%）
  - 绕过 [max_packet_size](Server_settings/Searchd.md#max_packet_size) 限制，支持批量传输远大于 `max_packet_size` 最大值（128MB），例如一次 1GB。
* [#719](https://github.com/manticoresoftware/manticoresearch/issues/719) HTTP 接口支持 `100 Continue`：现在可以通过 `curl`（包括各种编程语言的 curl 库）传输大批数据，这些客户端默认发送 `Expect: 100-continue` 并等待一段时间后才实际发送数据。之前必须添加 `Expect: ` 头部，现在不再需要。

  <details>

  之前（注意响应时间）：

  ```bash
  $ time curl -v -sX POST http://localhost:9318/bulk -H "Content-Type: application/x-ndjson" --data '{"insert": {"index": "user", "doc":  {"name":"Prof. Matt Heaney IV","email":"ibergnaum@yahoo.com","description":"Tempora ullam eaque consequatur. Vero aut minima ut et ut omnis officiis vel. Molestiae quis voluptatum sint numquam.","age":15,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"Prof. Boyd McKenzie","email":"carlotta11@hotmail.com","description":"Blanditiis maiores odio corporis eaque illum. Aut et rerum iste. Neque et ullam quisquam officia dignissimos quo cumque.","age":84,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"约翰·史密斯先生","email":"stiedemann.tristin@ziemann.com","description":"Temporibus amet magnam consequatur omnis consequatur illo fugit. Debitis natus doloremque est tempore deserunt vero. Harum eos corrupti nemo ut.","age":89,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"赫克托·普罗斯","email":"hickle.mafalda@hotmail.com","description":" as voluptatem inventore sit. Aliquam fugit perferendis est id aut odio et sapiente.","age":64,"active":1}}}'
  *   正在尝试 127.0.0.1...
  * 已连接到 localhost (127.0.0.1) 端口 9318 (#0)
  > POST /bulk HTTP/1.1
  > Host: localhost:9318
  > User-Agent: curl/7.47.0
  > Accept: */*
  > Content-Type: application/x-ndjson
  > Content-Length: 1025
  > Expect: 100-continue
  >
  * 等待 100-continue 完成
  * 上传完全完成，状态正常
  < HTTP/1.1 200 OK
  < Server: 4.2.0 15e927b@211223 release (columnar 1.11.4 327b3d4@211223)
  < Content-Type: application/json; charset=UTF-8
  < Content-Length: 434
  <
  * 保持 #0 连接到主机 localhost 不关闭
  {"items":[{"insert":{"table":"user","_id":2811798918248005633,"created":true,"result":"created","status":201}},{"insert":{"table":"user","_id":2811798918248005634,"created":true,"result":"created","status":201}},{"insert":{"table":"user","_id":2811798918248005635,"created":true,"result":"created","status":201}},{"insert":{"table":"user","_id":2811798918248005636,"created":true,"result":"created","status":201}}],"errors":false}
  real	0m1.022s
  user	0m0.001s
  sys	0m0.010s
  ```

  现在：
  ```bash
  $ time curl -v -sX POST http://localhost:9318/bulk -H "Content-Type: application/x-ndjson" --data '{"insert": {"index": "user", "doc":  {"name":"马特·希尼教授 IV","email":"ibergnaum@yahoo.com","description":"Tempora ullam eaque consequatur. Vero aut minima ut et ut omnis officiis vel. Molestiae quis voluptatum sint numquam.","age":15,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"博伊德·麦肯齐教授","email":"carlotta11@hotmail.com","description":"Blanditiis maiores odio corporis eaque illum. Aut et rerum iste. Neque et ullam quisquam officia dignissimos quo cumque.","age":84,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"约翰·史密斯先生","email":"stiedemann.tristin@ziemann.com","description":"Temporibus amet magnam consequatur omnis consequatur illo fugit. Debitis natus doloremque est tempore deserunt vero. Harum eos corrupti nemo ut.","age":89,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"赫克托·普罗斯","email":"hickle.mafalda@hotmail.com","description":" as voluptatem inventore sit. Aliquam fugit perferendis est id aut odio et sapiente.","age":64,"active":1}}}'
  *   正在尝试 127.0.0.1...
  * 已连接到 localhost (127.0.0.1) 端口 9318 (#0)
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
  * 上传完全完成，状态正常
  < HTTP/1.1 200 OK
  < Server: 4.2.1 63e5749@220405 dev
  < Content-Type: application/json; charset=UTF-8
  < Content-Length: 147
  <
  * 保持 #0 连接到主机 localhost 不关闭
  {"items":[{"bulk":{"table":"user","_id":2811798919590182916,"created":4,"deleted":0,"updated":0,"result":"created","status":201}}],"errors":false}
  real	0m0.015s
  user	0m0.005s
  sys	0m0.004s
  ```

  </details>

* **⚠️ 破坏性变更**: [伪分片](Server_settings/Searchd.md#pseudo_sharding) 默认启用。若您想禁用它，请确保在 Manticore 配置文件中的 `searchd` 部分添加 `pseudo_sharding = 0`。
* 现在在实时/普通索引中至少有一个全文字段不再是强制性的。您现在可以在与全文搜索无关的情况下使用 Manticore。
* [快速提取](Creating_a_table/Data_types.md#fast_fetch) 由 [Manticore Columnar Library](https://github.com/manticoresoftware/columnar) 支持的属性：查询如 `select * from <columnar table>` 现在比以前快得多，特别是在模式中字段较多时。
* **⚠️ 破坏性变更**: 隐式 [cutoff](Searching/Options.md#cutoff)。Manticore 现在不会花费时间和资源处理您不需要返回结果集中的数据。缺点是它会影响 [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META) 中的 `total_found` 和 JSON 输出中 [hits.total](Searching/Full_text_matching/Basic_usage.md#HTTP-JSON) 的准确性。只有在看到 `total_relation: eq` 时结果才准确，`total_relation: gte` 表示实际匹配文档数大于您获得的 `total_found` 值。您可以使用搜索选项 `cutoff=0` 来保持以前的行为，这样 `total_relation` 总是 `eq`。
* **⚠️ 破坏性变更**: 所有全文字段现在默认是 [存储的](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#stored_fields)。如果想让所有字段不存储（即恢复以前的行为），需要使用 `stored_fields = `（空值）。
* [#715](https://github.com/manticoresoftware/manticoresearch/issues/715) HTTP JSON 支持 [搜索选项](Searching/Options.md#General-syntax)。

### 小改动
* **⚠️ 破坏性变更**: 索引元文件格式改变。以前的元文件（`.meta`，`.sph`）是二进制格式，现在改为 json 格式。新的 Manticore 版本会自动转换旧索引，但：
  - 可能出现警告，如 `WARNING: ... syntax error, unexpected TOK_IDENT`
  - 无法用旧版本 Manticore 运行该索引，请确保您有备份
* **⚠️ 重大变更**：通过[HTTP keep-alive](Connecting_to_the_server/HTTP.md#Keep-alive)支持会话状态。这使得在客户端也支持的情况下，HTTP具有状态性。例如，使用新的[/cli](Connecting_to_the_server/HTTP.md#/cli)端点和HTTP keep-alive（所有浏览器默认开启），可以在`SELECT`后调用`SHOW META`，效果与通过mysql访问相同。注意，之前支持`Connection: keep-alive` HTTP 头，仅仅复用了同一连接。从本版本开始，它还使会话具有状态性。
* 现在可以指定`columnar_attrs = *`，在[纯模式](Read_this_first.md#Real-time-mode-vs-plain-mode)中将所有属性定义为列存储，这在属性列表较长时非常有用。
* 更快的复制SST。
* **⚠️ 重大变更**：复制协议已更改。如果您运行复制集群，升级到Manticore 5时需要：
  - 先干净地停止所有节点
  - 然后用`--new-cluster`启动最后停止的节点（在Linux中运行工具`manticore_new_cluster`）
  - 更多细节请阅读[重启集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster)。
* 复制改进：
  - 更快的SST
  - 噪声抵抗，有助于复制节点间网络不稳定时保持稳定
  - 日志改进
* 安全改进：如果配置中未指定任何`listen`，Manticore现在监听`127.0.0.1`而非`0.0.0.0`。虽然Manticore Search默认配置中都有指定`listen`，配置中无`listen`的情况较少，但仍可能存在。之前Manticore监听`0.0.0.0`，安全性较差，现在监听`127.0.0.1`，通常不对外网开放。
* 对列存储属性的聚合更快。
* 提高`AVG()`精度：之前Manticore聚合内部使用`float`，现在使用`double`，显著提高精度。
* 改进了对JDBC MySQL驱动的支持。
* 支持`DEBUG malloc_stats`用于[jemalloc](https://github.com/jemalloc/jemalloc)。
* [optimize_cutoff](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#optimize_cutoff)现可作为单表设置，在CREATE或ALTER表时可设定。
* **⚠️ 重大变更**：[query_log_format](Server_settings/Searchd.md#query_log_format)默认改为**`sphinxql`**。如果习惯使用`plain`格式，需要在配置文件中添加`query_log_format = plain`。
* 显著减少内存消耗：在长时间、高强度的insert/replace/optimize负载且使用存储字段时，Manticore消耗的RAM明显减少。
* [shutdown_timeout](Server_settings/Searchd.md#shutdown_timeout)默认值从3秒提升至60秒。
* [Commit ffd0](https://github.com/manticoresoftware/manticoresearch/commit/ffd0499d329d2c383f14c8a44c4cc84338ab56e7) 支持 Java mysql connector >= 6.0.3：在[Java mysql 连接器6.0.3](https://mvnrepository.com/artifact/mysql/mysql-connector-java/6.0.3)中更改了连接MySQL的方式，导致与Manticore兼容性中断。现已支持新行为。
* [Commit 1da6](https://github.com/manticoresoftware/manticoresearch/commit/1da6dbec) 禁止在加载索引（如searchd启动时）时保存新的磁盘块。
* [Issue #746](https://github.com/manticoresoftware/manticoresearch/issues/746) 支持glibc >= 2.34。
* [Issue #784](https://github.com/manticoresoftware/manticoresearch/issues/784) VIP连接与普通（非VIP）连接分开计数。之前VIP连接计入`max_connections`限制，可能导致非VIP连接出现“连接数已满”错误。现在VIP连接不计入限制。当前VIP连接数可通过`SHOW STATUS`和`status`查看。
* [ID](Creating_a_table/Data_types.md#Document-ID)现可显式指定。
* [Issue #687](https://github.com/manticoresoftware/manticoresearch/issues/687) 支持mysql协议的zstd压缩。

### ⚠️ 其他小的重大变更
* ⚠️ BM25F公式略有更新以提升搜索相关性。仅影响使用函数[BM25F()](Functions/Searching_and_ranking_functions.md#BM25F%28%29)的搜索结果，不影响默认排序公式行为。
* ⚠️ 更改REST [/sql](Connecting_to_the_server/HTTP.md#mode=raw)端点行为：`/sql?mode=raw` 现在要求转义且返回数组。
* ⚠️ `/bulk` INSERT/REPLACE/DELETE请求响应格式变更：
  - 之前每个子查询作为独立事务，返回独立响应
  - 现在整个批次作为单一事务，返回单一响应
* ⚠️ 搜索选项`low_priority`和`boolean_simplify`现须赋值(`0/1`)使用：之前可写成`SELECT ... OPTION low_priority, boolean_simplify`，现需写成`SELECT ... OPTION low_priority=1, boolean_simplify=1`。
* ⚠️ 如果使用旧版[php](https://github.com/manticoresoftware/manticoresearch/blob/master/api/sphinxapi.php)、[python](https://github.com/manticoresoftware/manticoresearch/blob/master/api/sphinxapi.py)或[java](https://github.com/manticoresoftware/manticoresearch/tree/master/api/java)客户端，请访问相应链接获取更新版本。**旧版与Manticore 5不完全兼容。**
* ⚠️ HTTP JSON请求在`query_log_format=sphinxql`模式下日志记录格式变更。之前只记录全文部分，现在完整记录。

### 新软件包
* **⚠️ 重大变更**：因结构调整，升级至Manticore 5时建议先卸载旧版软件包，再安装新版：
  - 基于RPM的系统：`yum remove manticore*`
  - Debian和Ubuntu：`apt remove manticore*`
* 新的 deb/rpm 包结构。之前的版本提供：
  - `manticore-server` 包含 `searchd`（主搜索守护进程）和所有运行所需内容
  - `manticore-tools` 包含 `indexer` 和 `indextool`
  - `manticore` 包含所有内容
  - `manticore-all` RPM 作为元包，引用 `manticore-server` 和 `manticore-tools`

  新结构如下：
  - `manticore` - deb/rpm 元包，安装以上所有包作为依赖
  - `manticore-server-core` - `searchd` 及所有单独运行所需内容
  - `manticore-server` - systemd 文件及其他辅助脚本
  - `manticore-tools` - `indexer`、`indextool` 及其他工具
  - `manticore-common` - 默认配置文件、默认数据目录、默认停用词
  - `manticore-icudata`、`manticore-dev`、`manticore-converter` 变化不大
  - 包含所有包的 `.tgz` 捆绑包
* 支持 Ubuntu Jammy
* 通过 [YUM 仓库](Installation/RHEL_and_Centos.md#YUM-repository) 支持 Amazon Linux 2

### Bug 修复
* [问题 #815](https://github.com/manticoresoftware/manticoresearch/issues/815) 使用 UDF 函数时的随机崩溃
* [问题 #287](https://github.com/manticoresoftware/manticoresearch/issues/287) 索引 RT 索引时内存不足
* [问题 #604](https://github.com/manticoresoftware/manticoresearch/issues/604) 3.6.0、4.2.0 版本 sphinxql-parser 的破坏性更改
* [问题 #667](https://github.com/manticoresoftware/manticoresearch/issues/667) FATAL: 内存不足（无法分配 9007199254740992 字节）
* [问题 #676](https://github.com/manticoresoftware/manticoresearch/issues/676) 字符串未正确传递给 UDF
* ❗[问题 #698](https://github.com/manticoresoftware/manticoresearch/issues/698) 在尝试向 RT 索引添加文本列后，searchd 崩溃
* [问题 #705](https://github.com/manticoresoftware/manticoresearch/issues/705) Indexer 无法找到所有列
* ❗[问题 #709](https://github.com/manticoresoftware/manticoresearch/issues/705) 按 json.boolean 分组时出现错误
* [问题 #716](https://github.com/manticoresoftware/manticoresearch/issues/716) 与索引相关的 indextool 命令（如 --dumpdict）失败
* ❗[问题 #724](https://github.com/manticoresoftware/manticoresearch/issues/724) 字段从选取结果中消失
* [问题 #727](https://github.com/manticoresoftware/manticoresearch/issues/727) 使用 `application/x-ndjson` 时 .NET HttpClient 的 Content-Type 不兼容
* [问题 #729](https://github.com/manticoresoftware/manticoresearch/issues/729) 字段长度计算问题
* ❗[问题 #730](https://github.com/manticoresoftware/manticoresearch/issues/730) 创建/插入到/删除列式表时存在内存泄漏
* [问题 #731](https://github.com/manticoresoftware/manticoresearch/issues/731) 在特定条件下结果中出现空列
* ❗[问题 #749](https://github.com/manticoresoftware/manticoresearch/issues/749) 守护进程启动时崩溃
* ❗[问题 #750](https://github.com/manticoresoftware/manticoresearch/issues/750) 守护进程启动时挂起
* ❗[问题 #751](https://github.com/manticoresoftware/manticoresearch/issues/751) SST 阶段崩溃
* [问题 #752](https://github.com/manticoresoftware/manticoresearch/issues/752) 当 engine='columnar' 时，json 属性被标记为列式存储
* [问题 #753](https://github.com/manticoresoftware/manticoresearch/issues/753) 复制侦听地址为 0
* [问题 #754](https://github.com/manticoresoftware/manticoresearch/issues/754) columnar_attrs = * 在 csvpipe 中不工作
* ❗[问题 #755](https://github.com/manticoresoftware/manticoresearch/issues/755) 在 rt 中选择 float 类型列式字段时崩溃
* ❗[问题 #756](https://github.com/manticoresoftware/manticoresearch/issues/756) Indextool 在检查过程中修改 RT 索引
* [问题 #757](https://github.com/manticoresoftware/manticoresearch/issues/757) 需要检查监听端口范围是否有交叉
* [问题 #758](https://github.com/manticoresoftware/manticoresearch/issues/758) 当 RT 索引保存磁盘块失败时记录原始错误
* [问题 #759](https://github.com/manticoresoftware/manticoresearch/issues/759) RE2 配置仅报告一个错误
* ❗[问题 #760](https://github.com/manticoresoftware/manticoresearch/issues/760) commit 5463778558586d2508697fa82e71d657ac36510f 中 RAM 使用变化
* [问题 #761](https://github.com/manticoresoftware/manticoresearch/issues/761) 第三个节点在脏重启后未成为非主集群
* [问题 #762](https://github.com/manticoresoftware/manticoresearch/issues/762) 更新计数器增加了 2
* [问题 #763](https://github.com/manticoresoftware/manticoresearch/issues/763) 4.2.1 新版本损坏了用 4.2.0 结合形态学创建的索引
* [问题 #764](https://github.com/manticoresoftware/manticoresearch/issues/764) json 键中无转义 /sql?mode=raw
* ❗[问题 #765](https://github.com/manticoresoftware/manticoresearch/issues/765) 使用函数时隐藏了其他值
* ❗[问题 #766](https://github.com/manticoresoftware/manticoresearch/issues/766) FixupAttrForNetwork 中一行代码引发内存泄漏
* ❗[问题 #767](https://github.com/manticoresoftware/manticoresearch/issues/767) 4.2.0 和 4.2.1 版本中与文档存储缓存相关的内存泄漏
* [问题 #768](https://github.com/manticoresoftware/manticoresearch/issues/768) 通过网络使用存储字段时出现奇怪的往返现象
* [问题 #769](https://github.com/manticoresoftware/manticoresearch/issues/769) 如果 'common' 部分没有提及，lemmatizer_base 被重置为空
* [问题 #770](https://github.com/manticoresoftware/manticoresearch/issues/770) pseudo_sharding 导致按 id 查询变慢
* [问题 #771](https://github.com/manticoresoftware/manticoresearch/issues/771) 使用 jemalloc 时 DEBUG malloc_stats 输出为零
* [问题 #772](https://github.com/manticoresoftware/manticoresearch/issues/772) 删除/添加列使值不可见
* [问题 #773](https://github.com/manticoresoftware/manticoresearch/issues/773) 无法向列式表添加 bit(N) 列
* [问题 #774](https://github.com/manticoresoftware/manticoresearch/issues/774) manticore.json 启动时 "cluster" 字段为空
* ❗[提交 1da4](https://github.com/manticoresoftware/manticoresearch/commits/1da4ce891e4ff13727cfd3331d72771c64ee7949) HTTP 操作未在 SHOW STATUS 中跟踪
* [提交 3810](https://github.com/manticoresoftware/manticoresearch/commits/381000ab4af4c3c98ec8c730699ad7f39039cec8) 禁用低频单关键字查询的 pseudo_sharding
* [提交 8003](https://github.com/manticoresoftware/manticoresearch/commits/800325cca283a87801b28929d82420fafc76a0ee) 修正存储属性与索引合并的问题
* [提交 cddf](https://github.com/manticoresoftware/manticoresearch/commits/cddfeed6296f2041c6aae18414e5cd9cabf08281) 泛化 distinct 值获取器；增加了针对列式字符串的专用 distinct 获取器
* [提交 fba4](https://github.com/manticoresoftware/manticoresearch/commits/fba4bb4f) 修正从 docstore 获取空整数属性的问题
* [提交 f300](https://github.com/manticoresoftware/manticoresearch/commit/f3009a9242fa16956adf871c4887e6e0303ba364) 查询日志中 `ranker` 可能被指定两次

## 版本 4.2.0，2021 年 12 月 23 日

### 主要新特性
* **实时索引和全文查询的伪分片支持**。在之前的版本我们增加了有限的伪分片支持。从本版本开始，只需启用 [searchd.pseudo_sharding](Server_settings/Searchd.md#pseudo_sharding)，即可充分利用伪分片和您的多核处理器。最棒的是，您无需对索引或查询做任何操作，只要开启它，如果有空闲 CPU 资源，就会被使用以降低响应时间。它支持全文检索、过滤和分析查询的普通与实时索引。例如，启用伪分片后，在 [Hacker news 精选评论数据集](https://zenodo.org/record/45901/)（乘以 100 次，包含 1.16 亿文档的普通索引）上的大多数查询**平均响应时间降低约 10 倍**。

![4.2.0 中伪分片开启 vs 关闭](4.2.0_ps_on_vs_off.png)

* 现已支持 [**Debian Bullseye**](https://manticoresearch.com/install/)。

<!-- example pq_transactions_4.2.0 -->
* PQ 事务现在支持原子性和隔离性。之前 PQ 事务支持有限。现在显著加快了对 PQ 的 REPLACE 操作，尤其是在需要一次替换大量规则时。性能细节：

<!-- intro -->
#### 4.0.2

<!-- request 4.0.2 -->

插入 100 万条 PQ 规则耗时为 **48 秒**，而分批替换仅 4 万条（每批 1 万条）耗时为 **406 秒**。

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

插入 100 万条 PQ 规则耗时为 **34 秒**，分批替换耗时为 **23 秒**（每批 1 万条）。

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

### 次要变更
* [optimize_cutoff](Server_settings/Searchd.md#optimize_cutoff) 现可作为配置选项置于 `searchd` 部分。当您希望全局限制所有索引中的 RT 分块数量到特定数目时，此选项非常有用。
* [提交 0087](https://github.com/manticoresoftware/manticoresearch/commit/00874743823eaf43e2a09a088392fd7e0a617f47) 修正了多个具有相同字段集/顺序的本地物理索引（实时/普通）上的准确 [count(distinct ...)](Searching/Grouping.md#COUNT%28DISTINCT-field%29) 和 [FACET ... distinct](Searching/Faceted_search.md#Faceting-without-duplicates)。
* [PR #598](https://github.com/manticoresoftware/manticoresearch/pull/598) 为 `YEAR()` 及其他时间戳函数添加 bigint 支持。
* [提交 8e85](https://github.com/manticoresoftware/manticoresearch/commit/8e85d4bce596cc88f9d9615ea243a27e4afde0f7) 自适应 [rt_mem_limit](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_mem_limit)。此前 Manticore Search 会精确收集到 `rt_mem_limit` 量数据才保存为新的磁盘块，且保存时还会额外收集最多 10%（双缓冲）以尽量减少插入暂停。如果该限制也被耗尽，新增文档会被阻塞直到该磁盘块完成保存。新的自适应限制基于我们已有的 [auto-optimize](Server_settings/Searchd.md#auto_optimize) 机制，因此磁盘块不完全遵守 `rt_mem_limit`，而是提前开始刷新也无大碍。现在我们收集到 `rt_mem_limit` 的 50% 大小就开始保存磁盘块。保存过程中会查看统计数据（已经保存多少，保存时又来了多少新文档）并重新计算初始比例，以便下次参考。例如，如果保存了 9000 万文档，保存期间又来了 1000 万，比例为 90%，所以下次可以收集到 `rt_mem_limit` 的 90% 后再开始刷新。比例值自动计算范围是 33.3% 到 95%。
* [Issue #628](https://github.com/manticoresoftware/manticoresearch/issues/628) PostgreSQL 数据源支持 [unpack_zlib](Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Database_connection.md#unpack_zlib)。感谢 [Dmitry Voronin](https://github.com/dimv36) 的 [贡献](https://github.com/manticoresoftware/manticoresearch/pull/630)。
* [提交 6d54](https://github.com/manticoresoftware/manticoresearch/commit/6d54cf2b319b131970c29410cc21abcbdf8083b1) 支持 `indexer -v` 和 `--version`。此前虽然能看到 indexer 的版本号，但 `-v`/`--version` 不被支持。
* [Issue #662](https://github.com/manticoresoftware/manticoresearch/issues/662) 通过 systemd 启动 Manticore 时，默认无限制 mlock 限制。
* [提交 63c8](https://github.com/manticoresoftware/manticoresearch/commit/63c8cd05cf399e705c2c14939411a92cb784735a) 将 coro rwlock 的自旋锁改为操作队列。
* [提交 4113](https://github.com/manticoresoftware/manticoresearch/commit/41130ce30d1734bbf8e6b8b63c4e815caf9464f2) 环境变量 `MANTICORE_TRACK_RT_ERRORS`，有助于调试 RT 段损坏。

### 重大变更
* Binlog 版本已升级，之前版本的 binlog 不会被重放，因此请确保在升级期间干净地停止 Manticore Search：停止之前的实例后，`/var/lib/manticore/binlog/` 目录下除了 `binlog.meta` 外不应存在其他 binlog 文件。
* [提交 3f65](https://github.com/manticoresoftware/manticoresearch/commit/3f659f36e87d99ee262205a8eae4663a255be630) 在 `show threads option format=all` 中新增了 "chain" 列。它显示了一些任务信息票证的堆栈，对于性能分析需求非常有用，解析 `show threads` 输出时请注意新增这一列。
* `searchd.workers` 自 3.5.0 版本起已废弃，现在被标记为不推荐使用，如果配置文件中仍包含该项，启动时会触发警告。Manticore Search 会启动，但会带有警告信息。
* 如果你使用 PHP 和 PDO 访问 Manticore，需要设置 `PDO::ATTR_EMULATE_PREPARES`

### 修复内容
* ❗[问题 #650](https://github.com/manticoresoftware/manticoresearch/issues/650) Manticore 4.0.2 比 Manticore 3.6.3 慢。4.0.2 在批量插入速度上快于之前版本，但单文档插入明显更慢。此问题在 4.2.0 中修复。
* ❗[提交 22f4](https://github.com/manticoresoftware/manticoresearch/commit/22f4141bed1411c83684ae6e0bae832071cce3df) RT 索引在高强度 REPLACE 负载下可能损坏，或导致崩溃
* [提交 03be](https://github.com/manticoresoftware/manticoresearch/commit/03be91e4f720a3f20bee0278dd8f0d44abd3ef0e)  修正合并 groupers 和 group N sorter 时的平均值计算；修正聚合合并问题
* [提交 2ea5](https://github.com/manticoresoftware/manticoresearch/commit/2ea575d34efe38661f14d858f2b552456f29fe42) `indextool --check` 可能崩溃
* [提交 7ec7](https://github.com/manticoresoftware/manticoresearch/commit/7ec76d4abf01b5fca6f10a0000fe64d5d8e5fa52) 更新操作导致的内存耗尽问题
* [提交 658a](https://github.com/manticoresoftware/manticoresearch/commit/658a727e41f3c3fbcde9754149528f34ed76ba19) 守护进程在 INSERT 时可能挂起
* [提交 46e4](https://github.com/manticoresoftware/manticoresearch/commit/46e42b9b386e31e53247ed634e158b533e28063e) 守护进程在关闭时可能挂起
* [提交 f8d7](https://github.com/manticoresoftware/manticoresearch/commit/f8d7d5177cc83d250b493b4cfc619732b29cf072) 守护进程关闭时可能崩溃
* [提交 733a](https://github.com/manticoresoftware/manticoresearch/commit/733accf165142a1d1574689525ccca8907971158) 守护进程崩溃时可能挂起
* [提交 f7f8](https://github.com/manticoresoftware/manticoresearch/commit/f7f8bd8c0dbb23a59a88502dcba16b0918fb2330) 守护进程启动时尝试用无效节点列表重加入集群可能崩溃
* [提交 1401](https://github.com/manticoresoftware/manticoresearch/commit/140155613821ecaddcd1a3dfc4f7369b0df6aef7) 分布式索引在 RT 模式下启动时如果无法解析某代理节点，可能会完全丢失
* [问题 #683](https://github.com/manticoresoftware/manticoresearch/issues/683) attr bit(N) engine='columnar' 失败
* [问题 #682](https://github.com/manticoresoftware/manticoresearch/issues/682) 创建表失败，但留下目录
* [问题 #663](https://github.com/manticoresoftware/manticoresearch/issues/663) 配置失败，错误信息：unknown key name 'attr_update_reserve'
* [问题 #632](https://github.com/manticoresoftware/manticoresearch/issues/632) Manticore 在批量查询时崩溃
* [问题 #679](https://github.com/manticoresoftware/manticoresearch/issues/679) 批量查询导致 v4.0.3 再次崩溃
* [提交 f7f8](https://github.com/manticoresoftware/manticoresearch/commit/f7f8bd8c0dbb23a59a88502dcba16b0918fb2330) 修复守护进程启动时尝试用无效节点列表重加入集群导致崩溃
* [问题 #643](https://github.com/manticoresoftware/manticoresearch/issues/643) Manticore 4.0.2 批量插入后无法接受连接
* [问题 #635](https://github.com/manticoresoftware/manticoresearch/issues/635) FACET 查询使用 ORDER BY JSON.field 或字符串属性时可能崩溃
* [问题 #634](https://github.com/manticoresoftware/manticoresearch/issues/634) 查询时使用 packedfactors 导致 SIGSEGV 崩溃
* [提交 4165](https://github.com/manticoresoftware/manticoresearch/commits/41657f15f8fcf1e93cfad7f835bcf705d6e8349e) create table 时不支持 morphology_skip_fields 问题修复

## 版本 4.0.2，2021年9月21日

### 主要新功能
- **全面支持 [Manticore Columnar Library](https://github.com/manticoresoftware/columnar)**。之前 Manticore Columnar Library 仅支持普通索引。现在支持：
  - 实时索引中的 `INSERT`、`REPLACE`、`DELETE`、`OPTIMIZE`
  - 复制功能
  - `ALTER`
  - `indextool --check`
- **自动索引压缩** ([问题 #478](https://github.com/manticoresoftware/manticoresearch/issues/478))。终于不需要手动调用 OPTIMIZE 或通过 crontask 或其他自动化方式进行压缩。Manticore 现在默认自动完成此操作。可通过全局变量 [optimize_cutoff](Server_settings/Setting_variables_online.md) 设置默认压缩阈值。
- **块快照与锁系统重构**。这些变化乍看可能无感，但显著提升了实时索引中多项操作的表现。简而言之，以前大多数 Manticore 数据处理依赖重锁定，现在改用磁盘块快照机制。
- **显著提升实时索引批量插入性能**。例如在 [Hetzner的 AX101 服务器](https://www.hetzner.com/dedicated-rootserver/ax101)（SSD，128GB 内存，AMD Ryzen™ 9 5950X 16*2 核）上，**3.6.0 版本能实现每秒插入 236K 文档**，表结构为 `name text, email string, description text, age int, active bit(1)`（默认 `rt_mem_limit`，批量大小 25000，16 线程并发插入，共插入 1600 万文档）。4.0.2 版本在相同的并发/批量/总量下达到 **357K 文档每秒**。

  <details>

  - 读取操作（如 SELECT 和复制）均通过快照执行
  - 仅更改内部索引结构而不修改架构/文档的操作（例如合并RAM段、保存磁盘块、合并磁盘块）使用只读快照执行，并最终替换现有块
  - UPDATE 和 DELETE 针对现有块执行，但在合并可能发生的情况下，写入操作会被收集，然后应用于新块
  - UPDATE 为每个块依次获取独占锁。合并在进入从块收集属性阶段时获取共享锁。因此同时只有一个（合并或更新）操作可以访问块的属性。
  - 当合并进入需要属性的阶段时，会设置一个特殊标志。UPDATE 完成后，会检查该标志，如果设置，则整个更新存储在特殊集合中。最终，当合并完成时，应用更新集到新生成的磁盘块。
  - ALTER 通过独占锁运行
  - 复制按常规读操作运行，但另外会在 SST 之前保存属性，并禁止 SST 期间的更新

  </details>

- **[ALTER](Updating_table_schema_and_settings.md) 可以添加/删除全文字段**（在 RT 模式下）。此前只能添加/删除属性。
- 🔬 **实验性功能：全扫描查询的伪分片** - 允许并行化任何非全文检索查询。无需手动准备分片，只需启用新选项 [searchd.pseudo_sharding](Server_settings/Searchd.md#pseudo_sharding)，即可获得最高至 `CPU 核心数` 倍的非全文检索查询响应时间降低。注意这可能会占用所有 CPU 核心，因此如果不仅关心延迟，还关注吞吐量，请谨慎使用。

### 小更新
<!-- example -->
- 通过 [APT 仓库](Installation/Debian_and_Ubuntu.md#APT-repository) 支持 Linux Mint 和 Ubuntu Hirsute Hippo
- 在某些情况下，通过 HTTP 以 id 更新大索引更快（依赖于 id 分布）
- [671e65a2](https://github.com/manticoresoftware/lemmatizer-uk/commit/671e65a296800537123a3f8b324eeaa3f58b9632) - 为 lemmatizer-uk 添加缓存


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
- [systemd 的自定义启动参数](Starting_the_server/Linux.md#Custom-startup-flags-using-systemd)。现在如果需要用特定启动参数运行 Manticore，无需手动启动 searchd
- 新函数 [LEVENSHTEIN()](Functions/String_functions.md#LEVENSHTEIN%28%29) 计算 Levenshtein 距离
- 新增搜索守护进程启动参数 [searchd 启动参数](Starting_the_server/Manually.md#searchd-command-line-options)  `--replay-flags=ignore-trx-errors` 和 `--replay-flags=ignore-all-errors`，可在二进制日志损坏时仍启动 searchd
- [Issue #621](https://github.com/manticoresoftware/manticoresearch/issues/621) - 暴露 RE2 的错误
- 对由本地普通索引组成的分布式索引，实现更精确的 [COUNT(DISTINCT)](Searching/Grouping.md#COUNT%28DISTINCT-field%29)
- [FACET DISTINCT](Searching/Faceted_search.md#Faceting-without-duplicates) 用于分面搜索中去重
- [精确形式修饰符](Searching/Full_text_matching/Operators.md#Exact-form-modifier) 现在无需 [形态学](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology)，并支持启用 [中缀/前缀](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) 搜索的索引

### 破坏性变更
- 新版本能读取旧索引，但旧版本无法读取 Manticore 4 的索引
- 移除隐式按 id 排序。需要排序时请显式指定
- `charset_table` 默认值从 `0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451` 改为 `non_cjk`
- `OPTIMIZE` 自动执行。如不需要，请确保在配置文件的 `searchd` 段中设置 `auto_optimize=0`
- [Issue #616](https://github.com/manticoresoftware/manticoresearch/issues/616)  `ondisk_attrs_default` 弃用，现已移除
- 对贡献者：我们现用 Clang 编译器构建 Linux 版本，经测试它能生成更快速的 Manticore Search 和 Manticore Columnar Library
- 若搜索查询未指定 [max_matches](Searching/Options.md#max_matches)，则会根据新列存储的性能需求隐式更新为最低必要值。这可能影响 [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META) 中 `total` 指标，但不影响实际找到文档数 `total_found`。

### 从 Manticore 3 迁移
- 确保干净停止 Manticore 3：
  - `/var/lib/manticore/binlog/` 中不应有 binlog 文件（目录中仅应有 `binlog.meta`）
  - 否则 Manticore 4 无法回复该索引的 binlog，索引无法运行
- 新版本能读取旧索引，但旧版本无法读取 Manticore 4 的索引，若想方便回退新版本，请务必备份
- 若运行复制集群，请确保：
  - 先干净停止所有节点
  - 然后用 `--new-cluster` 启动最后停止的节点（Linux 下运行工具 `manticore_new_cluster`）。
  - 详情请见 [重启集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster)

### Bug 修复
- 修复大量复制问题：
  - [Commit 696f](https://github.com/manticoresoftware/manticoresearch/commit/696f8649535f2b5285b878c9581c0d2b1139ae09) - 修复带有活动索引的加入节点在 SST 期间崩溃；加入节点写入文件块时添加 sha1 校验加速索引加载；加入节点索引加载时添加已更改索引文件轮转；加入节点替换活动索引为新索引时添加删除旧索引文件；为捐赠节点发送文件和块添加复制日志点
  - [提交 b296](https://github.com/manticoresoftware/manticoresearch/commit/b296c55af68314a8fc66f8b104e7301d673c3b68) - 在地址不正确的情况下，JOIN CLUSTER 导致崩溃
  - [提交 418b](https://github.com/manticoresoftware/manticoresearch/commit/418bf880a5e05ffbb68f311a1c23006a9df3220e) - 在大型索引的初始复制过程中，加入节点可能会因 `ERROR 1064 (42000): invalid GTID, (null)` 失败，且在另一个节点加入时，捐赠节点可能会变得无响应
  - [提交 6fd3](https://github.com/manticoresoftware/manticoresearch/commit/6fd350d28e4905211cf8a7298cbf7cdda101c83d) - 对于大型索引，哈希计算可能错误，导致复制失败
  - [问题 #615](https://github.com/manticoresoftware/manticoresearch/issues/615) - 集群重启时复制失败
- [问题 #574](https://github.com/manticoresoftware/manticoresearch/issues/574) - `indextool --help` 未显示参数 `--rotate`
- [问题 #578](https://github.com/manticoresoftware/manticoresearch/issues/578) - searchd 在空闲约一天后 CPU 使用率过高
- [问题 #587](https://github.com/manticoresoftware/manticoresearch/issues/587) - 立即刷新 .meta 文件
- [问题 #617](https://github.com/manticoresoftware/manticoresearch/issues/617) - manticore.json 被清空
- [问题 #618](https://github.com/manticoresoftware/manticoresearch/issues/618) - searchd --stopwait 在 root 下失败。且修复了 systemctl 行为（之前 ExecStop 显示失败，且未等待足够时间使 searchd 正常停止）
- [问题 #619](https://github.com/manticoresoftware/manticoresearch/issues/619) - INSERT/REPLACE/DELETE 与 SHOW STATUS 相关。`command_insert`、`command_replace` 等显示了错误的指标
- [问题 #620](https://github.com/manticoresoftware/manticoresearch/issues/620) - 普通索引的 `charset_table` 默认值错误
- [提交 8f75](https://github.com/manticoresoftware/manticoresearch/commit/8f7536887d4b58a5bab5647fc067b2d0482c2038) - 新的磁盘块未被 mlocked
- [问题 #607](https://github.com/manticoresoftware/manticoresearch/issues/607) - 当无法通过名称解析节点时，Manticore 集群节点崩溃
- [问题 #623](https://github.com/manticoresoftware/manticoresearch/issues/623) - 更新后的索引复制可能导致未定义状态
- [提交 ca03](https://github.com/manticoresoftware/manticoresearch/commit/ca03d2280c5197e9b311f0f03f551e3b702a8130) - 索引器在为普通索引源带有 json 属性索引时可能挂起
- [提交 53c7](https://github.com/manticoresoftware/manticoresearch/commit/53c753054558ff3f82c7af365273d32537c5338b) - 修复 PQ 索引的不等表达式过滤器
- [提交 ccf9](https://github.com/manticoresoftware/manticoresearch/commit/ccf94e022ba6763d6286ab5bac767b6f6a846846) - 修复列表查询中超过 1000 条匹配时的 select 窗口。先前 `SELECT * FROM pq ORDER BY id desc LIMIT 1000 , 100 OPTION max_matches=1100` 无法正常工作
- [提交 a048](https://github.com/manticoresoftware/manticoresearch/commit/a0483fe9bef3c45dffe96f5df4f4382aee29c072) - 向 Manticore 发送 HTTPS 请求可能导致类似“max packet size(8388608) exceeded”的警告
- [问题 #648](https://github.com/manticoresoftware/manticoresearch/issues/648) - Manticore 3 在字符串属性多次更新后可能挂起


## 版本 3.6.0，2021年5月3日
**Manticore 4 之前的维护版本**

### 主要新功能
- 支持用于普通索引的 [Manticore Columnar Library](https://github.com/manticoresoftware/columnar/)。普通索引新增设置 [columnar_attrs](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#columnar_attrs)
- 支持 [乌克兰语词形还原器](https://github.com/manticoresoftware/lemmatizer-uk)
- 完全改进的直方图。构建索引时，Manticore 也会为其中每个字段建立直方图，随后用于更快的过滤。在 3.6.0 中算法全面改进，如果您拥有大量数据并执行大量过滤，可以获得更高性能。

### 次要更改
- 工具 `manticore_new_cluster [--force]`，方便通过 systemd 重启复制集群
- 为 `indexer --merge` 添加 [--drop-src](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Merging_tables.md#Merging-tables)
- 新模式 [new mode](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) `blend_mode='trim_all'`
- 增加通过反引号支持转义 JSON 路径的功能 [support for escaping JSON path](Searching/Full_text_matching/Escaping.md#Escaping-json-node-names-in-SQL)
- [indextool --check](Miscellaneous_tools.md#indextool) 支持 RT 模式
- 对 SELECT/UPDATE 支持 [FORCE/IGNORE INDEX(id)](Data_creation_and_modification/Updating_documents/UPDATE.md#Updates-via-SQL)
- 合并的磁盘块的块 ID 现在唯一
- 新增命令 [indextool --check-disk-chunk CHUNK_NAME](Miscellaneous_tools.md#indextool)

### 优化
- [更快的 JSON 解析](https://github.com/manticoresoftware/manticoresearch/commit/0c25fc1788e3f77f60cb7fb8f1623d8466bba606)，测试显示在类似 `WHERE json.a = 1` 的查询中延迟降低 3-4%
- 非公开命令 `DEBUG SPLIT`，为自动分片/重新平衡做准备

### Bug 修复
- [问题 #584](https://github.com/manticoresoftware/manticoresearch/issues/524) - FACET 结果不准确且不稳定
- [问题 #506](https://github.com/manticoresoftware/manticoresearch/issues/506) - 使用 MATCH 出现异常行为；受此问题影响用户需重新构建索引，因为问题出在构建索引阶段
- [问题 #387](https://github.com/manticoresoftware/manticoresearch/issues/387) - 使用 SNIPPET() 函数执行查询时偶发核心转储
- 针对处理复杂查询的栈优化：
  - [问题 #469](https://github.com/manticoresoftware/manticoresearch/issues/469) - SELECT 导致崩溃转储
  - [e8420cc7](https://github.com/manticoresoftware/manticoresearch/tree/e8420cc789149c84731be0874be916e33b263bd6) - 过滤树的栈大小检测
- [Issue #461](https://github.com/manticoresoftware/manticoresearch/issues/461) - 使用 IN 条件进行更新时未能正确生效
- [Issue #464](https://github.com/manticoresoftware/manticoresearch/issues/464) - CALL PQ 后立即执行 SHOW STATUS 返回 - [Issue #481](https://github.com/manticoresoftware/manticoresearch/issues/481) - 修复静态二进制构建问题
- [Issue #502](https://github.com/manticoresoftware/manticoresearch/issues/502) - 多查询中的错误
- [Issue #514](https://github.com/manticoresoftware/manticoresearch/issues/514) - 使用 'create table' 时无法使用非常规列名
- [Commit d1db](https://github.com/manticoresoftware/manticoresearch/commit/d1dbe771288328c963ca8512f564c6b7cf4f466c) - 在回放包含字符串属性更新的 binlog 时守护进程崩溃；将 binlog 版本设置为 10
- [Commit 775d](https://github.com/manticoresoftware/manticoresearch/commit/775d0555562f121911d3c30085947e743fadac2d)  - 修复表达式堆栈帧检测运行时错误（测试 207）
- [Commit 4795](https://github.com/manticoresoftware/manticoresearch/commit/4795dc49194c7745761bb5006ecb266e8a9224fa)  - 空存储查询的 percolate 索引过滤器和标签为空（测试 369）
- [Commit c3f0](https://github.com/manticoresoftware/manticoresearch/commit/c3f0bf4dca0e4eae6ea899dd566878df57eba085)  - 在高延迟及高错误率网络（不同数据中心复制）中复制 SST 流中断；复制命令版本更新至 1.03
- [Commit ba2d](https://github.com/manticoresoftware/manticoresearch/commit/ba2d6619b8107f3fb177741bbdff3709bac248cd)  - 集群写入操作中连接锁定（测试 385）
- [Commit de4d](https://github.com/manticoresoftware/manticoresearch/commit/de4dcb9ff6c316bc2a9ef6c62eb64769f04ee103)  - 带有 exact 修饰符的通配符匹配（测试 321）
- [Commit 6524](https://github.com/manticoresoftware/manticoresearch/commit/6524fc6af032a0932bd42c37874a619e1fb7f075)  - docid 检查点与文档存储的协调
- [Commit f4ab](https://github.com/manticoresoftware/manticoresearch/commit/f4ab83c291e65a5873d271c67b5491f629544f1d)  - 索引器解析无效 XML 时行为不一致
- [Commit 7b72](https://github.com/manticoresoftware/manticoresearch/commit/7b727e22fe01b1a180d4a8d3a3fea92c18eac707)  - 存储的带 NOTNEAR 的 percolate 查询无限运行（测试 349）
- [Commit 812d](https://github.com/manticoresoftware/manticoresearch/commit/812dab745f0c972ecde5d5b3d4a313cd6e1e143e)  - 以通配符开始的短语权重错误
- [Commit 1771](https://github.com/manticoresoftware/manticoresearch/commit/1771afc669c75670b3511bf02ab9bfc8982b94d2)  - 带通配符的 percolate 查询生成无有效负载的词项导致命中交错并影响匹配（测试 417）
- [Commit aa0d](https://github.com/manticoresoftware/manticoresearch/commit/aa0d8c2bcb41428d41dbfb66e42a872a79a4ee4b)  - 修正并行化查询中 'total' 的计算
- [Commit 18d8](https://github.com/manticoresoftware/manticoresearch/commit/18d81b3cfbb930b7361dae645fe13283d4fbf6ef)  - Windows 守护进程中多并发会话导致崩溃
- [Commit 8443](https://github.com/manticoresoftware/manticoresearch/commit/84432f23d6fbf7a7d0b67c8c9412eb5933b03596)  - 部分索引设置不能被复制
- [Commit 9341](https://github.com/manticoresoftware/manticoresearch/commit/93411fe6af4246ca4acce758944d513a1660b92b)  - 在高添加事件速率时 netloop 有时冻结，因原子 'kick' 事件被多事件重复处理，导致部分实际操作丢失
查询状态，而非服务器状态
- [Commit d805](https://github.com/manticoresoftware/manticoresearch/commit/d805fc125244ef08d14fb3d1826e2c356862d539)  - 新刷新磁盘块可能在提交时丢失
- [Commit 63cb](https://github.com/manticoresoftware/manticoresearch/commit/63cbf008d50979cdf7062664fb02a672a3bb1131)  - profiler 中 'net_read' 不准确
- [Commit f537](https://github.com/manticoresoftware/manticoresearch/commit/f5379bb27234b25919966c50ccb6f469ead646e2)  - Arabic 文本（从右到左）percolate 问题
- [Commit 49ee](https://github.com/manticoresoftware/manticoresearch/commit/49eeb4202d4ce4875c07c35087d7108c1e177328)  - 重复列名时未正确选取 id
- [Commit refa](https://github.com/manticoresoftware/manticoresearch/commit/4b7df262bbfec9b9e95a44e72ad326ac8a786c49)  - 修复罕见情况下网络事件崩溃问题
- [e8420cc7](https://github.com/manticoresoftware/manticoresearch/tree/e8420cc789149c84731be0874be916e33b263bd6) 修复 `indextool --dumpheader`
- [Commit ff71](https://github.com/manticoresoftware/manticoresearch/commit/ff7163532ed39042c0d3c3c2b902ff41f94d7f0c) - 使用 TRUNCATE WITH RECONFIGURE 时存储字段处理错误

### 重大变更：
- 新 binlog 格式：在升级前需彻底停止 Manticore
- 索引格式略有更改：新版本可正常读取现有索引，但若从 3.6.0 降级至更早版本，新建的索引将无法读取
- 复制格式变更：不要从旧版本复制到 3.6.0 或反向复制，应当在所有节点上一次切换到新版本
- `reverse_scan` 已废弃。请确保自 3.6.0 起不使用该查询选项，否则查询将失败
- 自本版本起，不再提供 RHEL6、Debian Jessie 和 Ubuntu Trusty 的构建版本。如需继续支持，请 [联系我们](https://manticoresearch.com/contact-us/)

### 废弃内容
- 不再隐式按 id 排序。如果依赖此功能，请相应更新查询
- 搜索选项 `reverse_scan` 已被废弃

## 版本 3.5.4，2020年12月10日

### 新特性
- 新的 Python、Javascript 及 Java 客户端现已正式发布，且在本手册中有详尽文档说明。
- 实时索引的磁盘分块自动丢弃。此优化使得在优化实时索引时，当分块显然不再需要（所有文档均被抑制）时，可以自动丢弃该磁盘分块。之前仍需要合并，现在该分块可以立即丢弃。[cutoff](Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks) 选项被忽略，即使实际上没有合并，过时的磁盘分块也会被删除。这在你在索引中维护保留并删除较早文档时非常有用。现在压缩这类索引会更快。
- 作为SELECT选项的[独立 NOT](Searching/Options.md#not_terms_only_allowed)

### 小改动
- [问题 #453](https://github.com/manticoresoftware/manticoresearch/issues/453) 新选项 [indexer.ignore_non_plain=1](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#ignore_non_plain) 对于运行 `indexer --all` 并且配置文件中不仅有普通索引的情况非常有用。没有 `ignore_non_plain=1` 会收到警告和相应的退出代码。
- [SHOW PLAN ... OPTION format=dot](Node_info_and_management/Profiling/Query_plan.md#Dot-format-for-SHOW-PLAN) 和 [EXPLAIN QUERY ... OPTION format=dot](Searching/Full_text_matching/Profiling.md#Profiling-without-running-a-query) 支持全文本查询计划执行的可视化。便于理解复杂查询。

### 已废弃
- `indexer --verbose` 已废弃，因为它从未增加索引器输出内容
- 导出 watchdog 的回溯信号现在使用 `USR2` 替代 `USR1`

### 修复
- [问题 #423](https://github.com/manticoresoftware/manticoresearch/issues/423) 西里尔字符期间调用片段保留模式不高亮
- [问题 #435](https://github.com/manticoresoftware/manticoresearch/issues/435) RTINDEX - GROUP N BY 表达式选择导致致命崩溃
- [提交 2b3b](https://github.com/manticoresoftware/manticoresearch/commit/2b3b62bd0ce2b54544733d524a169e81f71b5388) 当处于集群中时 searchd 状态显示段错误
- [提交 9dd2](https://github.com/manticoresoftware/manticoresearch/commit/9dd25c193ed2a53750a0e0e68566fd2b833fcb0c) 'SHOW INDEX index.N SETTINGS' 未正确显示大于9的分块
- [问题 #389](https://github.com/manticoresoftware/manticoresearch/issues/389) 导致 Manticore 崩溃的错误
- [提交 fba1](https://github.com/manticoresoftware/manticoresearch/commit/fba166178ff35c73a0c70d131cf3aca4a635e961) 转换器创建了损坏的索引
- [提交 eecd](https://github.com/manticoresoftware/manticoresearch/commit/eecd61d861c742471175be19a5e5455034b5db5f) stopword_step=0 与 CALL SNIPPETS() 的问题
- [提交 ea68](https://github.com/manticoresoftware/manticoresearch/commit/ea6850e449601199190f38a387c04b8a7b7e90f6) 局部索引中低 max_matches 下的 distinct 计数返回0
- [提交 362f](https://github.com/manticoresoftware/manticoresearch/commit/362f27db3421f09a41c6cce04b5baace00fa4c4d) 使用聚合时文本未返回到命中结果中


## 版本 3.5.2，2020年10月1日

### 新功能

* OPTIMIZE 将磁盘分块数量压缩至多个分块（默认是 `2* 核心数`），而非单个。最优分块数可通过 [cutoff](Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks) 选项控制。
* NOT 运算符现在可以单独使用。默认禁用，因为单独 NOT 查询可能很慢。可通过设置新的 searchd 指令 [not_terms_only_allowed](Server_settings/Searchd.md#not_terms_only_allowed) 为 `0` 启用。
* 新设置 [max_threads_per_query](Server_settings/Searchd.md#max_threads_per_query) 控制一条查询所用线程数。若不设置该指令，查询可使用线程数不超过 [threads](Server_settings/Searchd.md#threads) 的值。
每条 `SELECT` 查询，可通过 [OPTION threads=N](Searching/Options.md#threads) 限制线程数，覆盖全局 `max_threads_per_query`。
* 现在可通过 [IMPORT TABLE](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Importing_table.md) 导入 Percolate 索引。
* HTTP API `/search` 对 [分面搜索](Searching/Faceted_search.md#HTTP-JSON)/[分组](Searching/Grouping.md) 提供基本支持，通过新增查询节点 `aggs` 实现。

### 小改动

* 若未声明复制监听指令，引擎会尝试使用在已定义‘sphinx’端口之后的端口，最多至200端口。
* `listen=...:sphinx` 需显式设置以支持 SphinxSE 连接或 SphinxAPI 客户端。
* [SHOW INDEX STATUS](Node_info_and_management/Table_settings_and_status/SHOW_TABLE_STATUS.md) 现在输出新指标：`killed_documents`、`killed_rate`、`disk_mapped_doclists`、`disk_mapped_cached_doclists`、`disk_mapped_hitlists` 及 `disk_mapped_cached_hitlists`。
* SQL 命令 `status` 现在输出 `Queue\Threads` 和 `Tasks\Threads`。

### 弃用

* `dist_threads` 现已完全废弃，searchd 若仍使用该指令会记录警告。

### Docker

官方 Docker 映像现在基于 Ubuntu 20.04 LTS

### 软件包

除了常规的 `manticore` 包，你还可以按组件安装 Manticore Search：

- `manticore-server-core` - 提供 `searchd`、手册页、日志目录、API 和 galera 模块。此包还会作为依赖安装 `manticore-common`。
- `manticore-server` - 提供核心的自动化脚本（init.d，systemd）及 `manticore_new_cluster` 封装器。此包还会作为依赖安装 `manticore-server-core`。
- `manticore-common` - 提供配置、停用词、通用文档和骨架文件夹（datadir、modules等）
- `manticore-tools` - 提供辅助工具（`indexer`、`indextool`等）、其手册页和示例。此包还会作为依赖安装 `manticore-common`。
- `manticore-icudata`（RPM）或 `manticore-icudata-65l`（DEB）- 提供 ICU 数据文件以支持 ICU 形态学使用。
- `manticore-devel`（RPM）或 `manticore-dev`（DEB） - 提供 UDF 的开发头文件。

### Bug修复

1. [提交 2a47](https://github.com/manticoresoftware/manticoresearch/commit/2a474dc1a26e8b0f8aaaae95669caf2f1d4b7746) RT 索引中使用不同块时，grouper 守护进程崩溃
2. [提交 57a1](https://github.com/manticoresoftware/manticoresearch/commit/57a19e5ad5663ef0ca7436595218fb1221d28c8e) 空远程文档的快速路径
3. [提交 07dd](https://github.com/manticoresoftware/manticoresearch/commit/07dd3f313c63fb82c22092f9907ef24e3475250e) 表达式栈帧检测运行时
4. [提交 08ae](https://github.com/manticoresoftware/manticoresearch/commit/08ae357cf1012bc8e2da54c20b205b592efda3d4) percolate 索引中匹配超过 32 个字段
5. [提交 16b9](https://github.com/manticoresoftware/manticoresearch/commit/16b9390fd4cdb07a77ac4497adb935573a1710e5) 复制监听端口范围
6. [提交 5fa6](https://github.com/manticoresoftware/manticoresearch/commit/5fa671affeacb6441a59c8a88479bfd423df7c81) 在 pq 上显示创建表
7. [提交 54d1](https://github.com/manticoresoftware/manticoresearch/commit/54d133b6449105a9fb0168db3f1fbb05fb5aa1f6) HTTPS 端口行为
8. [提交 fdbb](https://github.com/manticoresoftware/manticoresearch/commit/fdbbe5245cc296cc5c1ae3ae2fb9cb08fb66a248) 替换时混合 docstore 行
9. [提交 afb5](https://github.com/manticoresoftware/manticoresearch/commit/afb53f648ef4e64fa1776c58e66f6e716ac730ad) 将 TFO 不可用消息级别切换为“info”
10. [提交 59d9](https://github.com/manticoresoftware/manticoresearch/commit/59d94cefc6e88af360b8046726a16b7eaa9f0b71) strcmp 非法使用导致崩溃
11. [提交 04af](https://github.com/manticoresoftware/manticoresearch/commit/04af0349c1e784c2230d204592d6272b440bb375) 将索引添加到带有系统（停用词）文件的集群
12. [提交 5014](https://github.com/manticoresoftware/manticoresearch/commit/50148b4eae9f3a924082005cf5d2f9c8339af7f5) 合并带有大型词典的索引；RT 优化大型磁盘块
13. [提交 a2ad](https://github.com/manticoresoftware/manticoresearch/commit/a2adf1582843f63d4dcc1dab8f7598e0917094a2) Indextool 可从当前版本转储元数据
14. [提交 69f6](https://github.com/manticoresoftware/manticoresearch/commit/69f6d5f77c64b97e3f395aea8c555b11c77fb732) GROUP N 中分组顺序问题
15. [提交 24d5](https://github.com/manticoresoftware/manticoresearch/commit/24d5d80f508576794b751d70f9f9b524564e3c14) 握手后对 SphinxSE 进行显式刷新
16. [提交 31c4](https://github.com/manticoresoftware/manticoresearch/commit/31c4d78ac03ee85d9a02b86075448a04ac80f9e2) 避免在不必要时复制大量描述
17. [提交 2959](https://github.com/manticoresoftware/manticoresearch/commit/2959e2caa8610f98fcdf2e5a82160f58687f4a27) show threads 中的负时间
18. [提交 f0b3](https://github.com/manticoresoftware/manticoresearch/commit/f0b357102c70f3ac1d7da260c60b67322940e647) 处理标记过滤插件与零位置差异
19. [提交 a49e](https://github.com/manticoresoftware/manticoresearch/commit/a49e5bc13ea763e6f01dd5993e891858a84d1846) 多重命中时将 “FAIL” 改为 “WARNING”

## 版本 3.5.0，2020年7月22日

### 主要新功能：
* 本次发布耗时较长，因为我们一直在努力将多任务模式从线程改为**协程**。这使配置更简单，查询并行化更直接：Manticore 现在只使用设定数量的线程（参见新设置 [threads](Server_settings/Searchd.md#threads)），而新模式确保以最优方式执行。
* [高亮显示](Searching/Highlighting.md#Highlighting-options)中的变更：
  - 任何支持多个字段的高亮（`highlight({},'field1, field2'`）或 json 查询中的 `highlight`）现在默认按字段应用限制。
  - 任何支持纯文本的高亮（`highlight({}, string_attr)`或 `snippet()`）现在对整个文档应用限制。
  - [按字段限制](Searching/Highlighting.md#limits_per_field)可通过选项 `limits_per_field=0` 切换为全局限制（默认为 `1`）。
  - [allow_empty](Searching/Highlighting.md#allow_empty) 在通过 HTTP JSON 进行高亮时默认值现在是 `0`。

* 同一端口[现在可以用于](Server_settings/Searchd.md#listen) http、https 和二进制 API（接受来自远程 Manticore 实例的连接）。通过 mysql 协议的连接仍需 `listen = *:mysql`。除 MySQL 以外，Manticore 现在能自动检测连接客户端类型（MySQL 有协议限制）。

* 在 RT 模式下，字段现在可以同时是[文本和字符串属性](Creating_a_table/Data_types.md#String)——[GitHub 问题 #331](https://github.com/manticoresoftware/manticoresearch/issues/331)。

  在[纯模式](Read_this_first.md#Real-time-mode-vs-plain-mode)中称为 `sql_field_string`。现在它也可用于实时索引的[RT 模式](Read_this_first.md#Real-time-mode-vs-plain-mode)。示例如下：

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

### 次要更改
* 现在可以[高亮字符串属性](Searching/Highlighting.md#Highlighting-via-SQL)。
* SQL 接口支持 SSL 和压缩
* 支持 mysql 客户端的 [`status`](Node_info_and_management/Node_status.md#STATUS) 命令。
* [复制](Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Setting-up-replication)现在可以复制外部文件（停用词、例外等）。
* 过滤操作符 [`in`](Searching/Filters.md#Set-filters) 现在通过 HTTP JSON 接口可用。
* HTTP JSON 中的 [`expressions`](Searching/Expressions.md#expressions)。
* [现在可以在 RT 模式下动态更改 `rt_mem_limit`](https://github.com/manticoresoftware/manticoresearch/issues/344)，即可以执行 `ALTER ... rt_mem_limit=<new value>`。
* 现在可以使用 [独立的 CJK 字符集表](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology)：`chinese`、`japanese` 和 `korean`。
* [thread_stack](Server_settings/Searchd.md#thread_stack) 现在限制最大线程栈，而非初始大小。
* 改进了 `SHOW THREADS` 输出。
* 在 `SHOW THREADS` 中显示长时间运行的 `CALL PQ` 进度。
* cpustat、iostat、coredump 可以在运行时通过 [SET](Server_settings/Setting_variables_online.md#SET) 修改。
* 实现了 `SET [GLOBAL] wait_timeout=NUM` ，

### 重大更改：
* **索引格式已更改。** 3.5.0 版本构建的索引无法被低于 3.5.0 的 Manticore 版本加载，但 Manticore 3.5.0 可以识别旧格式。
* [`INSERT INTO PQ VALUES()`](Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md) （即未提供列列表时）以前严格要求 `(query, tags)` 作为值。现在更改为 `(id,query,tags,filters)`。id 可以设置为 0，以便自动生成。
* [`allow_empty=0`](Searching/Highlighting.md#allow_empty) 是通过 HTTP JSON 接口高亮显示的新默认值。
* 外部文件（如停用词、例外词等）在 `CREATE TABLE`/`ALTER TABLE` 中只允许使用绝对路径。

### 弃用：
* `ram_chunks_count` 在 `SHOW INDEX STATUS` 中已重命名为 `ram_chunk_segments_count`。
* `workers` 已过时。现在只有一种 workers 模式。
* `dist_threads` 已过时。现在所有查询尽可能并行（受限于 `threads` 和 `jobs_queue_size`）。
* `max_children` 已过时。请使用 [threads](Server_settings/Searchd.md#threads) 设置 Manticore 使用的线程数（默认为 CPU 核心数）。
* `queue_max_length` 已过时。如有需要，请使用 [jobs_queue_size](Server_settings/Searchd.md#jobs_queue_size) 细调内部作业队列大小（默认无限制）。
* 所有 `/json/*` 端点现在可直接通过无 `/json/` 前缀的路径访问，例如 `/search`、`/insert`、`/delete`、`/pq` 等。
* `field` 表示“全文字段”的含义在 `describe` 中已重命名为“text”。
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
* 西里尔字母 `и` 不再映射到 `i`，因为 `non_cjk` 字符集表（默认）对俄语词干分析器和词形还原器影响过大。
* `read_timeout` 被废弃。请使用同时控制读写的 [network_timeout](Server_settings/Searchd.md#network_timeout)。


### 软件包

* 官方 Ubuntu Focal 20.04 软件包
* deb 软件包名由 `manticore-bin` 改为 `manticore`

### 修复的错误：
1. [Issue #351](https://github.com/manticoresoftware/manticoresearch/issues/351) searchd 内存泄漏
2. [Commit ceab](https://github.com/manticoresoftware/manticoresearch/commit/ceabe44f) 片段读取越界细微修正
3. [Commit 1c3e](https://github.com/manticoresoftware/manticoresearch/commit/1c3e84a3) 危险地写入局部变量导致崩溃查询
4. [Commit 26e0](https://github.com/manticoresoftware/manticoresearch/commit/26e094ab) 测试 226 中排序器的轻微内存泄漏
5. [Commit d2c7](https://github.com/manticoresoftware/manticoresearch/commit/d2c7f86a) 测试 226 中巨大内存泄漏
6. [Commit 0dd8](https://github.com/manticoresoftware/manticoresearch/commit/0dd80122) 集群显示节点同步，但 `count(*)` 显示不同数字
7. [Commit f1c1](https://github.com/manticoresoftware/manticoresearch/commit/f1c1ac3f) 外观：日志中重复且有时丢失警告信息
8. [Commit f1c1](https://github.com/manticoresoftware/manticoresearch/commit/f1c1ac3f) 外观：日志中出现 (null) 索引名
9. [Commit 359d](https://github.com/manticoresoftware/manticoresearch/commit/359dbd30) 无法获取超过 7000 万个结果
10. [Commit 19f3](https://github.com/manticoresoftware/manticoresearch/commit/19f328ee) 使用无列语法插入 PQ 规则失败
11. [Commit bf68](https://github.com/manticoresoftware/manticoresearch/commit/bf685d5d) 向集群中的索引插入文档时误导性错误信息
12. [Commit 2cf1](https://github.com/manticoresoftware/manticoresearch/commit/2cf18c83) `/json/replace` 和 `json/update` 返回指数形式的 id
13. [Issue #324](https://github.com/manticoresoftware/manticoresearch/issues/324) 在同一查询中更新 json 标量属性和 mva
14. [Commit d384](https://github.com/manticoresoftware/manticoresearch/commit/d38409eb) `hitless_words` 在 RT 模式无效
15. [Commit 5813](https://github.com/manticoresoftware/manticoresearch/commit/5813d639) 不允许在 rt 模式下执行 `ALTER RECONFIGURE`
16. [Commit 5813](https://github.com/manticoresoftware/manticoresearch/commit/5813d639) searchd 重启后 `rt_mem_limit` 被重置为 128M
17. highlight() 有时会挂起
18. [Commit 7cd8](https://github.com/manticoresoftware/manticoresearch/commit/7cd878f4) RT 模式下无法使用 U+ 代码
19. [Commit 2b21](https://github.com/manticoresoftware/manticoresearch/commit/2b213de4) RT 模式下失败使用词形变形的通配符
20. [Commit e9d0](https://github.com/manticoresoftware/manticoresearch/commit/e9d07e68) 修复 `SHOW CREATE TABLE` 与多个词形变形文件的问题
21. [Commit fc90](https://github.com/manticoresoftware/manticoresearch/commit/fc90a84f) JSON 查询缺少 "query" 导致 searchd 崩溃
22. Manticore [官方 docker](https://hub.docker.com/r/manticoresearch/manticore) 无法从 mysql 8 索引
23. [Commit 23e0](https://github.com/manticoresoftware/manticoresearch/commit/23e05d32) HTTP /json/insert 需要 id
24. [Commit bd67](https://github.com/manticoresoftware/manticoresearch/commit/bd679af0) `SHOW CREATE TABLE` 对 PQ 无效
25. [Commit bd67](https://github.com/manticoresoftware/manticoresearch/commit/bd679af0) `CREATE TABLE LIKE` 对 PQ 不能正常工作
26. [Commit 5eac](https://github.com/manticoresoftware/manticoresearch/commit/5eacf28f) show index status 的设置行末换行问题
27. [Commit cb15](https://github.com/manticoresoftware/manticoresearch/commit/cb153228) HTTP JSON 响应中 "highlight" 的空标题
28. [Issue #318](https://github.com/manticoresoftware/manticoresearch/issues/318) `CREATE TABLE LIKE` 中缀错误
29. [Commit 9040](https://github.com/manticoresoftware/manticoresearch/commit/9040d22c) RT 负载下崩溃
30. [cd512c7d ](https://github.com/manticoresoftware/manticoresearch/commit/cd512c7d ) RT 磁盘块崩溃时丢失崩溃日志
31. [Issue #323](https://github.com/manticoresoftware/manticoresearch/issues/323) 导入表失败并关闭连接
32. [Commit 6275](https://github.com/manticoresoftware/manticoresearch/commit/6275316a) `ALTER reconfigure` 损坏 PQ 索引
33. [Commit 9c1d](https://github.com/manticoresoftware/manticoresearch/commit/9c1d221e) 修改索引类型后 searchd 重载问题
34. [Commit 71e2](https://github.com/manticoresoftware/manticoresearch/commit/71e2b5bb) 缺失文件导入表时守护进程崩溃
35. [Issue #322](https://github.com/manticoresoftware/manticoresearch/issues/322) 使用多个索引、group by 和 ranker = none 时查询崩溃
36. [Commit c3f5](https://github.com/manticoresoftware/manticoresearch/commit/c3f58490) `HIGHLIGHT()` 无法高亮字符串属性
37. [Issue #320](https://github.com/manticoresoftware/manticoresearch/issues/320) `FACET` 无法对字符串属性排序
38. [Commit 4f1a](https://github.com/manticoresoftware/manticoresearch/commit/4f1a1f25) 缺失数据目录时的错误
39. [Commit 04f4](https://github.com/manticoresoftware/manticoresearch/commit/04f4ddd4) RT 模式不支持 access_*
40. [Commit 1c06](https://github.com/manticoresoftware/manticoresearch/commit/1c0616a2) 字符串中 JSON 对象错误：1. `CALL PQ` 当 json 超过某值时返回 "Bad JSON objects in strings: 1"
41. [Commit 32f9](https://github.com/manticoresoftware/manticoresearch/commit/32f943d6) RT 模式不一致。在某些情况下无法丢弃索引因为未知，也无法创建因为目录非空。
42. [Issue #319](https://github.com/manticoresoftware/manticoresearch/issues/319) 查询时崩溃
43. [Commit 22a2](https://github.com/manticoresoftware/manticoresearch/commit/22a28dd7) `max_xmlpipe2_field` = 2M 时对 2M 字段返回警告
44. [Issue #342](https://github.com/manticoresoftware/manticoresearch/issues/342) 查询条件执行错误
45. [Commit dd8d](https://github.com/manticoresoftware/manticoresearch/commit/dd8dcab2) 简单的两词搜索找到仅包含一个词的文档
46. [Commit 9091](https://github.com/manticoresoftware/manticoresearch/commit/90919e62) PQ 中无法匹配键中有大写字母的 json
47. [Commit 56da](https://github.com/manticoresoftware/manticoresearch/commit/56da086a) 索引器在 csv+docstore 上崩溃
48. [Issue #363](https://github.com/manticoresoftware/manticoresearch/issues/363) centos 7 中 json 属性使用 `[null]` 导致插入数据损坏
49. 重大 [Issue #345](https://github.com/manticoresoftware/manticoresearch/issues/345) 记录未插入，count() 非确定，"replace into" 返回 OK
50. max_query_time 过度减慢 SELECT 操作
51. [Issue #352](https://github.com/manticoresoftware/manticoresearch/issues/352) Mac OS 上主从通信失败
52. [Issue #328](https://github.com/manticoresoftware/manticoresearch/issues/328) 使用 Connector.Net/Mysql 8.0.19 连接 Manticore 时错误
53. [Commit daa7](https://github.com/manticoresoftware/manticoresearch/commit/daa760d2fe83668038318f0f384ceefc0d075e18) 修复 \0 转义并优化性能
54. [Commit 9bc5](https://github.com/manticoresoftware/manticoresearch/commit/9bc5c01a493db993b90ff6c1ad51aeae028f629c) 修正 count distinct 与 json 的问题
55. [Commit 4f89](https://github.com/manticoresoftware/manticoresearch/commit/4f89a965a3af0db548ae9d1763731ebaebe21f5a) 修复其他节点删除表失败
56. [Commit 952a](https://github.com/manticoresoftware/manticoresearch/commit/952af5a5931fd72b87edc169588777778b365231) 修复紧密运行 call pq 时崩溃


## 版本 3.4.2，2020年4月10日
### 关键错误修复
* [Commit 2ffe](https://github.com/manticoresoftware/manticoresearch/commit/2ffe2d261793f9681265bc4abe38d695e72c121e) 修复旧版本 RT 索引无法索引数据的问题

## 版本 3.4.0，2020年3月26日
### 主要变更
* 服务器工作两种模式：rt 模式和普通模式
   * rt 模式需要 data_dir，配置中无索引定义
   * 普通模式中索引在配置中定义；不允许有 data_dir
* 复制只在 rt 模式下可用

### 次要变更
* charset_table 默认为 non_cjk 别名
* rt 模式中文本字段默认被索引且存储
* rt 模式中文本字段从 'field' 重命名为 'text'
* ALTER RTINDEX 改名为 ALTER TABLE
* TRUNCATE RTINDEX 改名为 TRUNCATE TABLE

### 新特性
* 仅存储字段
* SHOW CREATE TABLE，IMPORT TABLE

### 改进
* 更快的无锁 PQ
* /sql 可执行任何类型的 SQL 语句，mode=raw
* mysql41 协议的 mysql 别名
* data_dir 中默认的 state.sql

### 错误修复
* [提交 a533](https://github.com/manticoresoftware/manticoresearch/commit/a5333644e8e0c69746a9f24da8221fc965478e4a) 修复 highlight() 中错误字段语法导致的崩溃
* [提交 7fbb](https://github.com/manticoresoftware/manticoresearch/commit/7fbb9f2e618096289cba7f8b834530b46aa94055) 修复带有 docstore 的复制 RT 索引时服务器崩溃
* [提交 24a0](https://github.com/manticoresoftware/manticoresearch/commit/24a04687eb43b4f4719c7c4a5fe3c19099240467) 修复对带有 infix 或 prefix 选项的索引进行 highlight 时崩溃，以及对未启用存储字段的索引的崩溃
* [提交 3465](https://github.com/manticoresoftware/manticoresearch/commit/3465c1ce9c860c09f6ccc5b83a63e3c68c241d8d) 修复空索引时关于空 docstore 和 doc-id 查找的错误提示
* [提交 a707](https://github.com/manticoresoftware/manticoresearch/commit/b8601b4144b9092be43a7caa7cd6b19cb37e14c7) 修复 #314 带分号结尾的 SQL insert 命令
* [提交 9562](https://github.com/manticoresoftware/manticoresearch/commit/95628c9b9b8f16fe1bd18599a84e9673575adf1a) 移除查询词不匹配的警告
* [提交 b860](https://github.com/manticoresoftware/manticoresearch/commit/b8601b4144b9092be43a7caa7cd6b19cb37e14c7) 修复通过 ICU 分词的查询在片段中的问题
* [提交 5275](https://github.com/manticoresoftware/manticoresearch/commit/5275516ca4631c584e6e0127fa161a18d2658fd0) 修复 docstore 块缓存中的查找/添加竞态条件
* [提交 f06e](https://github.com/manticoresoftware/manticoresearch/commit/f06ef97af5c7390427e82525c150535290e42c3e) 修复 docstore 内存泄漏
* [提交 a725](https://github.com/manticoresoftware/manticoresearch/commit/a7258ba8340e246c0d8aaeb4a7a74129eba807eb) 修复 #316 INSERT 时 LAST_INSERT_ID 返回空值
* [提交 1ebd](https://github.com/manticoresoftware/manticoresearch/commit/1ebd5bf8d74113bcfa138ce63ce11966990a4215) 修复 #317 json/update HTTP 端点以支持 MVA 的数组和 JSON 属性的对象
* [提交 e426](https://github.com/manticoresoftware/manticoresearch/commit/e426950a536382a142e2a42ed207c341149ff685) 修复 indexer 在没有显式 ID 时转储 rt 的问题

## 版本 3.3.0，2020 年 2 月 4 日
### 功能
* 并行实时索引搜索
* EXPLAIN QUERY 命令
* 无索引定义的配置文件（alpha 版本）
* CREATE/DROP TABLE 命令（alpha 版本）
* indexer --print-rt - 可以从源读取并打印用于实时索引的 INSERT 语句

### 改进
* 更新为 Snowball 2.0 词干提取器
* SHOW INDEX STATUS 支持 LIKE 过滤器
* 改进高 max_matches 下的内存使用
* SHOW INDEX STATUS 添加了 RT 索引的 ram_chunks_count
* 无锁优先队列（PQ）
* 将 LimitNOFILE 改为 65536


### Bug 修复
* [提交 9c33](https://github.com/manticoresoftware/manticoresearch/commit/9c33aab8c1fd9059c8644d23d5f58f03010043c6) 增加对索引模式中重复属性的检查 #293
* [提交 a008](https://github.com/manticoresoftware/manticoresearch/commit/a0085f9477e191da9e0e835b91583fb9cfde698e) 修复 hitless 词汇崩溃
* [提交 6895](https://github.com/manticoresoftware/manticoresearch/commit/68953740af8dca1833026d0f448c71416ed3044a) 修复 ATTACH 后松散的 docstore
* [提交 d6f6](https://github.com/manticoresoftware/manticoresearch/commit/d6f696edea53ea2b9a68251e4054e4816082c285) 修复分布式设置中的 docstore 问题
* [提交 bce2](https://github.com/manticoresoftware/manticoresearch/commit/bce2b7ec883d208f31094ee42a5b6f01d18a998f) 在排序器中将 FixedHash 替换为 OpenHash
* [提交 e0ba](https://github.com/manticoresoftware/manticoresearch/commit/e0baf7392937969e640fb5d10e8ccb312fe36446) 修复索引定义中属性名重复的问题
* [提交 ca81](https://github.com/manticoresoftware/manticoresearch/commit/ca81114bf1ac7ea7adcc8509bdc14d1f30b4f461) 修复 HIGHLIGHT() 中的 html_strip
* [提交 493a](https://github.com/manticoresoftware/manticoresearch/commit/493a5e916d70fd4a3f14d1ee487234434f4a7ce3) 修复 HIGHLIGHT() 中的 passage 宏
* [提交 a82d](https://github.com/manticoresoftware/manticoresearch/commit/a82d41c775692f837f267d554d7d4a4b499d69f0) 修复 RT 索引创建小或大磁盘块时的双缓冲问题
* [提交 a404](https://github.com/manticoresoftware/manticoresearch/commit/a404c85de0d41d9d1c75b7540479d8a3af9e61aa) 修复 kqueue 的事件删除
* [提交 8bea](https://github.com/manticoresoftware/manticoresearch/commit/8bea0f6fd1666b817995c4dd6e67f6f1c9161638) 修复 RT 索引对于大 rt_mem_limit 值磁盘块保存的问题
* [提交 8707](https://github.com/manticoresoftware/manticoresearch/commit/8707f0395b7ace52dc0cba9c8ed92a2bc4392932) 修复索引时浮点溢出
* [提交 a564](https://github.com/manticoresoftware/manticoresearch/commit/a56434ce71bb7714b4d8e75278aaa075b27ece80) 修复向 RT 索引插入负 ID 文档时出错的问题
* [提交 bbeb](https://github.com/manticoresoftware/manticoresearch/commit/bbebfd7551a68641154761cac962053b54806759) 修复 ranker fieldmask 导致的服务器崩溃
* [提交 3809](https://github.com/manticoresoftware/manticoresearch/commit/3809cc1b0863e3a5a200dc423be3f7c38be56113) 修复使用查询缓存时崩溃
* [提交 dc2a](https://github.com/manticoresoftware/manticoresearch/commit/dc2a585bc724678679e4297362200bbd49ce32bb) 修复使用并行插入的 RT 索引 RAM 段崩溃

## 版本 3.2.2，2019 年 12 月 19 日
### 功能
* RT 索引的自增 ID
* 通过新的 HIGHLIGHT() 函数支持 docstore 高亮，HTTP API 也可用
* SNIPPET() 可使用特殊函数 QUERY() 返回当前 MATCH 查询
* 高亮功能新增 field_separator 选项

### 改进与变更
* 远程节点对存储字段的懒惰抓取（可显著提升性能）
* 字符串和表达式不再破坏多查询和 FACET 优化
* RHEL/CentOS 8 版本现在使用 mariadb-connector-c-devel 提供的 mysql libclient
* ICU 数据文件现在随软件包一起发布，移除 icu_data_dir
* systemd 服务文件包含 'Restart=on-failure' 策略
* indextool 现在可以在线检查实时索引
* 默认配置现在是 /etc/manticoresearch/manticore.conf
* RHEL/CentOS 上的服务已从 'searchd' 重命名为 'manticore'
* 移除了 query_mode 和 exact_phrase snippet 的选项

### Bug 修复
* [提交 6ae4](https://github.com/manticoresoftware/manticoresearch/commit/6ae474c7894a6bee222d5b18e59a44fdbf57843a) 修复 HTTP 接口上执行 SELECT 查询时崩溃的问题
* [提交 5957](https://github.com/manticoresoftware/manticoresearch/commit/59577513a49eac5a4a3c5e2cb38394d3246b5d35) 修复 RT 索引保存磁盘块但未标记某些文档为已删除的问题
* [提交 e861](https://github.com/manticoresoftware/manticoresearch/commit/e861f0fca0e88924450695d4e9d6acff7a36558a) 修复使用 dist_threads 对多索引或多查询搜索时崩溃的问题
* [提交 4409](https://github.com/manticoresoftware/manticoresearch/commit/440991fc977b8479800b45cdbc862eeb1ba5d965) 修复对包含宽 UTF8 代码点的长词进行中缀生成时崩溃的问题
* [提交 5fd5](https://github.com/manticoresoftware/manticoresearch/commit/5fd599b48bba527c023e6aa0b262dca51ffb8a1c) 修复将套接字添加到 IOCP 时的竞态条件
* [提交 cf10](https://github.com/manticoresoftware/manticoresearch/commit/cf10d7d3589f77adfaefbe52a51777bc9c67cf99) 修复布尔查询与 JSON select 列表的问题
* [提交 996d](https://github.com/manticoresoftware/manticoresearch/commit/996de77f4c3ec103d965e28c36098fd07dba50c4) 修复 indextool 检查错误的跳跃列表偏移，修正 doc2row 查找检查
* [提交 6e3f](https://github.com/manticoresoftware/manticoresearch/commit/6e3fc9e88941c9427410f9d0adeebb7dd09a900f) 修复索引器在大数据上产生负跳跃列表偏移导致的错误索引
* [提交 faed](https://github.com/manticoresoftware/manticoresearch/commit/faed3220b423a96401982bf47117edf1f62c584e) 修复 JSON 仅将数字转换为字符串及表达式中 JSON 字符串转数字的问题
* [提交 5331](https://github.com/manticoresoftware/manticoresearch/commit/533197200249d648ebcbdaca5d605861ee566348) 修复 indextool 在命令行设置多个命令时以错误码退出
* [提交 7955](https://github.com/manticoresoftware/manticoresearch/commit/795520ac351d22e4497ebaf6c249bf25081842b6) 修复 #275 错误无剩余磁盘空间时 binlog 状态无效
* [提交 2284](https://github.com/manticoresoftware/manticoresearch/commit/2284da5e3ff77bf7be4038a4d950c0580e4159c8) 修复 #279 对 JSON 属性使用 IN 过滤器时崩溃
* [提交 ce2e](https://github.com/manticoresoftware/manticoresearch/commit/ce2e4b4765111a7d331ffc256911f12770a8942d) 修复 #281 错误的管道关闭调用
* [提交 5355](https://github.com/manticoresoftware/manticoresearch/commit/535589ba8a37baf5c6056afb5ff969fd70476feb) 修复调用 PQ 递归 JSON 属性编码为字符串时服务器挂起
* [提交 a5fc](https://github.com/manticoresoftware/manticoresearch/commit/a5fc8a36e1644ee051632408ab8b53e79aeb9ff4) 修复 multiand 节点中文档列表遍历超出结尾问题
* [提交 a362](https://github.com/manticoresoftware/manticoresearch/commit/a3628617ef4ee89a811d4ba2c4a04cbbc298c1b5) 修复线程公共信息检索
* [提交 f8d2](https://github.com/manticoresoftware/manticoresearch/commit/f8d2d7bbbb90af3d2ee97424f97b05927f09d42b) 修复文档存储缓存锁相关问题

## 版本 3.2.0，2019年10月17日
### 功能
* 文档存储
* 新指令 stored_fields、docstore_cache_size、docstore_block_size、docstore_compression、docstore_compression_level

### 改进和变更
* 改进 SSL 支持
* 更新 non_cjk 内置字符集
* 禁用 UPDATE/DELETE 语句在查询日志中记录为 SELECT
* RHEL/CentOS 8 软件包

### Bug 修复
* [提交 301a](https://github.com/manticoresoftware/manticoresearch/commit/301a806b14725e842300ee225521407464374c41) 修复 RT 索引磁盘块中文档替换崩溃
* [提交 46c1](https://github.com/manticoresoftware/manticoresearch/commit/46c1cad8fa11c1fc09cc34b20d978fdfcd4e6d27) 修复 #269 LIMIT N OFFSET M
* [提交 92a4](https://github.com/manticoresoftware/manticoresearch/commit/92a46edaa196ef33f3ac2225fbdba2c2b88ee45d) 修复带显式设置 id 或 id 列表的 DELETE 语句跳过搜索问题
* [提交 8ca7](https://github.com/manticoresoftware/manticoresearch/commit/8ca78c138d837caab30dc6e2343a3c4d1687bb87) 修复 windowspoll 轮询器中事件移除后索引错误
* [提交 6036](https://github.com/manticoresoftware/manticoresearch/commit/603631e2bff5cb0acf25f68b85621742490eccd9) 修复通过 HTTP 处理 JSON 的浮点数四舍五入问题
* [提交 62f6](https://github.com/manticoresoftware/manticoresearch/commit/62f64cb9e75ec28fc3c29f8414ce9656f5580254) 修复远程片段首先检查空路径的问题；修正 Windows 测试
* [提交 aba2](https://github.com/manticoresoftware/manticoresearch/commit/aba274c2c5abc1029894ba9f95b786bda93e8f22) 修复配置重新加载在 Windows 上与 Linux 行为一致
* [提交 6b8c](https://github.com/manticoresoftware/manticoresearch/commit/6b8c4242ef77c3fa4d0ccb7d76d81714b6728f0b) 修复 #194 PQ 支持形态学和词干分析
* [提交 174d](https://github.com/manticoresoftware/manticoresearch/commit/174d312905285aa5746f4f404c53e6dc669ef42f) 修复 RT 已退休段管理

## 版本 3.1.2，2019年8月22日
### 功能和改进
* HTTP API 的实验性 SSL 支持
* CALL KEYWORDS 的字段过滤器
* /json/search 的 max_matches
*自动调整默认 Galera gcache.size
* 改进 FreeBSD 支持

### Bug 修复
* [提交 0a1a](https://github.com/manticoresoftware/manticoresearch/commit/0a1a2c81038794983b4e30c8aefc7cc20e89b03f) 修复将 RT 索引复制到具有相同 RT 索引但路径不同的节点问题
* [提交 4adc](https://github.com/manticoresoftware/manticoresearch/commit/4adc075294ac823289f745e2cc419f18c7dcf2e2) 修复无活动索引的刷新重新调度
* [提交 d6c0](https://github.com/manticoresoftware/manticoresearch/commit/d6c00a6f750c04021747e9c99e0f77707a4cbfc3) 改进 RT/PQ 索引的刷新重新调度
* [提交 d0a7](https://github.com/manticoresoftware/manticoresearch/commit/d0a7c95906b9b5ba33c98267d54501cfe27508d6) 修复 #250 TSV 和 CSV 管道源的 index_field_lengths 索引选项
* [Commit 1266](https://github.com/manticoresoftware/manticoresearch/commit/1266d54800242709fbe2c34e72eb69b6595b23bd) 修复了 indextool 在空索引上进行块索引检查时的错误报告
* [Commit 553c](https://github.com/manticoresoftware/manticoresearch/commit/553ca73ca111f28e0888d59cd48b0d6df181ecd6) 修复了 Manticore SQL 查询日志中的空 SELECT 列表
* [Commit 56c8](https://github.com/manticoresoftware/manticoresearch/commit/56c8584456293665ddd4ce4c94307d74473a78b9) 修复了 indexer -h/--help 响应

## 版本 3.1.0，2019年7月16日
### 功能和改进
* 实时索引的复制
* 中文的 ICU 分词器
* 新的形态选项 icu_chinese
* 新指令 icu_data_dir
* 支持多语句事务的复制
* LAST_INSERT_ID() 和 @session.last_insert_id
* SHOW VARIABLES 的 LIKE 'pattern'
* 为待处理索引添加多文档插入
* 添加了配置的时间解析器
* 内部任务管理器
* 文档和命中列表组件的内存锁定
* jail snippets 路径

### 移除
* 放弃支持 RLP 库，转而支持 ICU；移除了所有 rlp\* 指令
* 禁用了使用 UPDATE 更新文档 ID 

### 错误修复
* [Commit f047](https://github.com/manticoresoftware/manticoresearch/commit/f047222339fd0e62a07e9ca8b2a829297d5db861) 修复了 concat 和 group_concat 中的缺陷
* [Commit b081](https://github.com/manticoresoftware/manticoresearch/commit/b08147ee24712b5eaa51f261626b5d16c6242fdd) 修复了待处理索引中查询 uid 应为 BIGINT 属性类型
* [Commit 4cd8](https://github.com/manticoresoftware/manticoresearch/commit/4cd85afaf76eef0ce9fa1cd073f3fed160279890) 在无法预分配新的磁盘块时不崩溃
* [Commit 1a55](https://github.com/manticoresoftware/manticoresearch/commit/1a551227886474054111088412f2b2396f485652) 添加了 ALTER 中缺失的时间戳数据类型
* [Commit f3a8](https://github.com/manticoresoftware/manticoresearch/commit/f3a8e096fd496a6b001d398aadbb781b2dbad929) 修复了 mmap 读取错误导致的崩溃
* [Commit 4475](https://github.com/manticoresoftware/manticoresearch/commit/447577115192db7ac7bfea70b5ba58ee2c61229f) 修复了复制中集群锁定的哈希
* [Commit ff47](https://github.com/manticoresoftware/manticoresearch/commit/ff476df955f057bd38e8b6ad6056325e9d6fbb9c) 修复了复制中提供者的泄漏
* [Commit 58dc](https://github.com/manticoresoftware/manticoresearch/commit/58dcbb779c4c30ca1560460c78e35ba114b83716) 修复了 \#246 indexer 中未定义的 sigmask
* [Commit 3dd8](https://github.com/manticoresoftware/manticoresearch/commit/3dd8278e747d534f69fe9c3d360eef188ab7b6d5) 修复了 netloop 报告中的竞争
* [Commit a02a](https://github.com/manticoresoftware/manticoresearch/commit/a02aae05a3fa3d1caf2821f43c7e237b6de76336) 为 HA 策略再平衡器设置了零间隙

## 版本 3.0.2，2019年5月31日
### 改进
* 为文档和命中列表添加了 mmap 读取器
* `/sql` HTTP 端点响应现在与 `/json/search` 响应相同
* 新指令 `access_plain_attrs`, `access_blob_attrs`, `access_doclists`, `access_hitlists`
* 为复制设置添加了新指令 `server_id`

### 移除
* 移除了 HTTP `/search` 端点

### 弃用
* `read_buffer`, `ondisk_attrs`, `ondisk_attrs_default`, `mlock` 被 `access_*` 指令替代

### 错误修复
* [Commit 849c](https://github.com/manticoresoftware/manticoresearch/commit/849c16e1040f382795ba0635cb48686f9db4f2d8) 允许 SELECT 列表中以数字开头的属性名称
* [Commit 48e6](https://github.com/manticoresoftware/manticoresearch/commit/48e6c302ca37bec0bc49c79619c60bf787ed0708) 修复了 UDFs 中的 MVAs，修复了 MVA 的别名
* [Commit 0555](https://github.com/manticoresoftware/manticoresearch/commit/055586a9a2ebfa8874f68099c5bc798a145fd4ef) 修复了使用带 SENTENCE 查询导致的 \#187 崩溃
* [Commit 93bf](https://github.com/manticoresoftware/manticoresearch/commit/93bf52f23f1c53c8a93a603387ca5463e1fc2ae8) 修复了 \#143 支持在 MATCH() 周围使用 ()
* [Commit 599e](https://github.com/manticoresoftware/manticoresearch/commit/599ee79ccd38cce0023a55cbce8f50c3dca62e38) 修复了 ALTER cluster 语句中集群状态的保存
* [Commit 230c](https://github.com/manticoresoftware/manticoresearch/commit/230c321e277514b191629538eb3f02d219113d95) 修复了在带 blob 属性的 ALTER 索引时服务器的崩溃
* [Commit 5802](https://github.com/manticoresoftware/manticoresearch/commit/5802b85aa93a81caea77073a3381ef912fdd677c) 修复了 \#196 按 ID 过滤的问题
* [Commit 25d2](https://github.com/manticoresoftware/manticoresearch/commit/25d2dabda4a299a29bd375405862126da9b4bfac) 丢弃在模板索引上的搜索
* [Commit 2a30](https://github.com/manticoresoftware/manticoresearch/commit/2a30d5b49d76faab031d195f533974938769b8e6) 修复了 SQL 回复中 ID 列为常规 bigint 类型


## 版本 3.0.0，2019年5月6日
### 功能和改进
* 新的索引存储。非标量属性不再限制为每个索引 4GB 大小
* attr_update_reserve 指令
* 可以使用 UPDATE 更新字符串、JSON 和 MVAs
* killlists 在索引加载时应用
* killlist_target 指令
* 提高了多 AND 搜索的速度
* 更好的平均性能和 RAM 使用
* 用于升级 2.x 制作的索引的转换工具
* CONCAT() 函数
* JOIN CLUSTER cluster AT 'nodeaddress:port'
* ALTER CLUSTER posts UPDATE nodes
* node_address 指令
* SHOW STATUS 中列出了节点

### 行为更改
* 在含有 killists 的索引情况下，服务器不按 conf 定义的顺序轮换索引，而是按照 killlist 目标的链条
* 搜索中索引的顺序不再定义 killlists 的应用顺序
* 文档 ID 现在是有符号的 big integers

### 移除的指令
* docinfo (现在总是 extern), inplace_docinfo_gap, mva_updates_pool

## 版本 2.8.2 GA，2019年4月2日
### 功能和改进
* Galera 复制用于待处理索引
* OPTION 形态学

### 编译说明
Cmake 最低版本现在为 3.13。编译需要 boost 和 libssl
开发库。

### 错误修复
* [Commit 6967](https://github.com/manticoresoftware/manticoresearch/commit/6967fedb2ef818ec1c825d482563edd05e1c9245) 修复了在查询多个分布式索引时，选择列表中存在多个星号导致的崩溃问题
* [Commit 36df](https://github.com/manticoresoftware/manticoresearch/commit/36df1a407dc08263690e3492518613ace82d69ca) 修复了 [\#177](https://github.com/manticoresoftware/manticoresearch/issues/177) 通过 Manticore SQL 接口传输大数据包的问题
* [Commit 5793](https://github.com/manticoresoftware/manticoresearch/commit/57932aec734583fa93359faaf5034b2e2c9d352b) 修复了 [\#170](https://github.com/manticoresoftware/manticoresearch/issues/170) 在 RT 优化时更新 MVA 导致的服务器崩溃
* [Commit edb2](https://github.com/manticoresoftware/manticoresearch/commit/edb24b870423add86eba471d361e0e5aff098b18) 修复了在 SIGHUP 配置重载后，由于移除 RT 索引导致删除 binlog 时服务器崩溃的问题
* [Commit bd3e](https://github.com/manticoresoftware/manticoresearch/commit/bd3e66e0085bc7e2e351b817dfc972fd8158fbce) 修复了 MySQL 握手认证插件加载的问题
* [Commit 6a21](https://github.com/manticoresoftware/manticoresearch/commit/6a217f6ed82fb10f752213b15617b9cedf1e8533) 修复了 [\#172](https://github.com/manticoresoftware/manticoresearch/issues/172) RT 索引中的 phrase_boundary 设置
* [Commit 3562](https://github.com/manticoresoftware/manticoresearch/commit/3562f652753e4091fd7b5b0f65b58341c8cbcb31) 修复了 [\#168](https://github.com/manticoresoftware/manticoresearch/issues/168) 在 ATTACH 索引自身时出现的死锁
* [Commit 250b](https://github.com/manticoresoftware/manticoresearch/commit/250b3f0e74ad18dda34ba080aa13ca87798a9ac9) 修复了 binlog 在服务器崩溃后保存空元数据的问题
* [Commit 4aa6](https://github.com/manticoresoftware/manticoresearch/commit/4aa6c69ad3a181cd7c5dafb990fb528121f68e59) 修复了由于带有磁盘块的 RT 索引中字符串传递到 sorter 时导致的服务器崩溃

## 版本 2.8.1 GA, 2019年3月6日
### 功能和改进
* 支持 SUBSTRING_INDEX()
* 为 percolate 查询支持 SENTENCE 和 PARAGRAPH
* 为 Debian/Ubuntu 提供 systemd 生成器；同时新增 LimitCORE 以支持 core dump

### Bug 修复
* [Commit 84fe](https://github.com/manticoresoftware/manticoresearch/commit/84fe7405d17b59f4dcde15a6c8e8503923503def) 修复了在匹配模式为 all 且全文查询为空时服务器崩溃的问题
* [Commit daa8](https://github.com/manticoresoftware/manticoresearch/commit/daa88b579fa373d5d2e869e43a2e178363b0fef1) 修复了删除静态字符串时的崩溃
* [Commit 2207](https://github.com/manticoresoftware/manticoresearch/commit/22078537dddbd9ce81a182dcc879f235b6e87004) 修复了当 indextool 以 FATAL 失败时的退出码
* [Commit 0721](https://github.com/manticoresoftware/manticoresearch/commit/0721696d6780c200c65d596624a8187983fb7fcb) 修复了 [\#109](https://github.com/manticoresoftware/manticoresearch/issues/109) 因错误的精确形式检查导致前缀无匹配
* [Commit 8af8](https://github.com/manticoresoftware/manticoresearch/commit/8af810111b8e2f87bc9f378172eff1ab9725c7e7) 修复了 [\#161](https://github.com/manticoresoftware/manticoresearch/issues/161) RT 索引配置重载问题
* [Commit e2d5](https://github.com/manticoresoftware/manticoresearch/commit/e2d592773f4fd702ace90070a793a842a0fec6ab) 修复了访问大型 JSON 字符串时服务器崩溃
* [Commit 75cd](https://github.com/manticoresoftware/manticoresearch/commit/75cd1342d05cdc352e3dd145cdbc6f79394a165b) 修复了 JSON 文档中经索引剥离器修改的 PQ 字段导致来自兄弟字段的错误匹配
* [Commit e2f7](https://github.com/manticoresoftware/manticoresearch/commit/e2f775437df922674d772ed4417780492502e65a) 修复了 RHEL7 构建中解析 JSON 时的服务器崩溃
* [Commit 3a25](https://github.com/manticoresoftware/manticoresearch/commit/3a25a5808feb3f8b80866991b436d6c1241618c2) 修复了当反斜杠位于边界时的 JSON 反转义崩溃
* [Commit be9f](https://github.com/manticoresoftware/manticoresearch/commit/be9f497872bea62dbdccd64fc28294fd7776c289) 修复了选项 'skip_empty' 能跳过空文档且不警告其非有效 JSON 的问题
* [Commit 266e](https://github.com/manticoresoftware/manticoresearch/commit/266e0e7b088549722a805fc837bf101ff681a5e8) 修复了 [\#140](https://github.com/manticoresoftware/manticoresearch/issues/161) 当 6 位数不足以表示精度时浮点数输出 8 位数字
* [Commit 3f6d](https://github.com/manticoresoftware/manticoresearch/commit/3f6d23891064ee8e94030c4231497cdd7da33a6a) 修复了空 jsonobj 创建问题
* [Commit f3c7](https://github.com/manticoresoftware/manticoresearch/commit/f3c7848a59b7eedc67c3403d6d4b37ce6fa94dc8) 修复了 [\#160](https://github.com/manticoresoftware/manticoresearch/issues/161) 空 mva 输出 NULL 而非空字符串
* [Commit 0afa](https://github.com/manticoresoftware/manticoresearch/commit/0afa2ed058e5759470b1d7354c722faab34f98bb) 修复了缺少 pthread_getname_np 的构建失败问题
* [Commit 9405](https://github.com/manticoresoftware/manticoresearch/commit/9405fccdeb27f0302a8d9a848981d30080216777) 修复了带有 thread_pool 工作线程时服务器关闭时崩溃

## 版本 2.8.0 GA, 2019年1月28日
### 改进
* 支持 percolate 索引的分布式索引
* CALL PQ 新选项和改动：
    *   skip_bad_json
    *   mode（sparsed/sharded）
    *   支持以 JSON 数组形式传递文档
    *   shift
    *   列名称 'UID', 'Documents', 'Query', 'Tags', 'Filters' 重命名为 'id', 'documents', 'query', 'tags', 'filters'
* 支持 DESCRIBE pq TABLE
* 不再支持 SELECT FROM pq WHERE UID，改用 'id'
* pq 索引上的 SELECT 与常规索引等价（例如，可以通过 REGEX() 过滤规则）
* 支持在 PQ 标签上使用 ANY/ALL
* 表达式对 JSON 字段自动转换，无需显式转换
* 内置 'non_cjk' 字符集表和 'cjk' ngram_chars
* 内置 50 种语言的停用词集合
* 停用词声明中的多个文件可以用逗号分隔
* CALL PQ 可接受 JSON 文档数组

### Bug 修复
* [Commit a4e1](https://github.com/manticoresoftware/manticoresearch/commit/a4e19afee54dafdc04b0da53372dafd8c0d332d6) 修复了与 csjon 相关的内存泄漏
* [Commit 28d8](https://github.com/manticoresoftware/manticoresearch/commit/28d862774874891a03e361da1b0347ebe8066ce0) 修复了由于 json 中缺失值导致的崩溃
* [Commit bf4e](https://github.com/manticoresoftware/manticoresearch/commit/bf4e9ea27c349cdc9ae4e54e960a4a17b90e38fa) 修复了 RT 索引空元数据的保存问题
* [Commit 33b4](https://github.com/manticoresoftware/manticoresearch/commit/33b4573529e7c7c7bce19bf9d54054866f30d99a) 修复了词形还原序列的丢失形式标志（exact）
* [Commit 6b95](https://github.com/manticoresoftware/manticoresearch/commit/6b95d48240d3b5520afade19c249d79e020a5e88) 修复了字符串属性大于 4M 时使用饱和而非溢出的处理
* [Commit 6214](https://github.com/manticoresoftware/manticoresearch/commit/621418b829e70af36aaa322a2f51ece3f86bc499) 修复了服务器在 SIGHUP 信号下禁用索引时的崩溃
* [Commit 3f7e](https://github.com/manticoresoftware/manticoresearch/commit/3f7e35d1482966cc45d5abbcb2de5de3508d66b0) 修复了服务器在同时执行 API 会话状态命令时的崩溃
* [Commit cd9e](https://github.com/manticoresoftware/manticoresearch/commit/cd9e4f1709a48ddafc128c450c2d882bc11ba50e) 修复了带字段过滤器删除 RT 索引查询时服务器崩溃的问题
* [Commit 9376](https://github.com/manticoresoftware/manticoresearch/commit/9376470d455fcd256c6abff79c431a6919dfa7ac) 修复了在分布式索引上调用 PQ 时传入空文档导致的服务器崩溃
* [Commit 8868](https://github.com/manticoresoftware/manticoresearch/commit/8868b207644f4f8cc1ab0c270adb35493fed22e1) 修复了 Manticore SQL 错误消息超过 512 字符时的截断问题
* [Commit de9d](https://github.com/manticoresoftware/manticoresearch/commit/de9deda9c142823d6dbf529423417b5c670fae94) 修复了无 binlog 时保存 percolate 索引导致的崩溃
* [Commit 2b21](https://github.com/manticoresoftware/manticoresearch/commit/2b219e1a32791e7740bb210b7d408f96abc6374f) 修复了 OSX 上 http 接口无法工作的错误
* [Commit e92c](https://github.com/manticoresoftware/manticoresearch/commit/e92c60240f8f9e1756bfa14fba0bbb402586a389) 修复了 indextool 检查 MVA 时错误的错误信息
* [Commit 238b](https://github.com/manticoresoftware/manticoresearch/commit/238bdea59bad89f097403f1c978658ce45f16c70) 修复了 FLUSH RTINDEX 时的写锁问题，避免在保存和定期 flush（来自 rt_flush_period）时锁定整个索引
* [Commit c26a](https://github.com/manticoresoftware/manticoresearch/commit/c26a236bd42b7082db079a51a84172066d337d64) 修复了 ALTER percolate 索引时卡在等待搜索加载的问题
* [Commit 9ee5](https://github.com/manticoresoftware/manticoresearch/commit/9ee5703f6c28f57cd5bff5e705966d93fe30d267) 修复了当 max_children 为 0 时使用线程池工作者默认数量的问题
* [Commit 5138](https://github.com/manticoresoftware/manticoresearch/commit/5138fc048a1bd146b271ce6a72fe954344281f69) 修复了在带有 index_token_filter 插件且 stopwords 和 stopword_step=0 时对索引进行数据索引的错误
* [Commit 2add](https://github.com/manticoresoftware/manticoresearch/commit/2add3d319ac62fe450bf60e89033100853dc2ecf) 修复了在索引定义中仍使用 aot 词形还原器时，缺少 lemmatizer_base 导致的崩溃

## 版本 2.7.5 GA，2018年12月4日
### 改进
* REGEX 函数
* json API 搜索的 limit/offset 支持
* qcache 的性能分析点

### Bug 修复
* [Commit eb3c](https://github.com/manticoresoftware/manticoresearch/commit/eb3c7683b9740cff4cc4cadd2ab9f990b9f7d173) 修复了在 FACET 查询中使用多重宽属性类型时服务器崩溃
* [Commit d915](https://github.com/manticoresoftware/manticoresearch/commit/d915cf6696514f4be365ffb5981bccaa15983e65) 修复了 FACET 查询主选择列表中的隐式 group by 问题
* [Commit 5c25](https://github.com/manticoresoftware/manticoresearch/commit/5c25dc271a8c11a1c82ca3c88e518acbc6a8c164) 修复了使用 GROUP N BY 查询时的崩溃
* [Commit 85d3](https://github.com/manticoresoftware/manticoresearch/commit/85d30a232d4ffd89916b673a9b2db946d272a945) 修复了在内存操作处理崩溃时的死锁问题
* [Commit 8516](https://github.com/manticoresoftware/manticoresearch/commit/85166b5ef2be38061d62f2b20f15acaa0311258a) 修复了 indextool 检查时的内存消耗问题
* [Commit 58fb](https://github.com/manticoresoftware/manticoresearch/commit/58fb03166830d8b23e7ab9ce56309db7ddac22f4) 修复了不再需要 gmock 头文件的问题，上游已自行解决

## 版本 2.7.4 GA，2018年11月1日
### 改进
* 在远程分布式索引的情况下，SHOW THREADS 显示原始查询而非 API 调用
* SHOW THREADS 新增选项 `format=sphinxql`，以 SQL 格式打印所有查询
* SHOW PROFILE 添加了额外的 `clone_attrs` 阶段

### Bug 修复
* [Commit 4f15](https://github.com/manticoresoftware/manticoresearch/commit/4f155712a0bccc1bd01cc191647bc8cff814888e) 修复了在 libc 不支持 malloc_stats、malloc_trim 时构建失败的问题
* [Commit f974](https://github.com/manticoresoftware/manticoresearch/commit/f974f20bda3214a56877c393a192be1a77150958) 修复了 CALL KEYWORDS 结果集中词内特殊符号问题
* [Commit 0920](https://github.com/manticoresoftware/manticoresearch/commit/092083282ea8ae0b2e72fb6989c57ccec81e74ac) 修复了通过 API 调用到分布式索引或远程代理时的 CALL KEYWORDS 断裂问题
* [Commit fd68](https://github.com/manticoresoftware/manticoresearch/commit/fd686bfe88b720ffd7642e36897ba45161cbd7d2) 修复了分布式索引 agent_query_timeout 未正确传递给代理的 max_query_time 问题
* [Commit 4ffa](https://github.com/manticoresoftware/manticoresearch/commit/4ffa623e9d357a3b0b441615089e211f92f8de32) 修复了磁盘块的总文档计数受 OPTIMIZE 命令影响导致权重计算错误的问题
* [Commit dcaf](https://github.com/manticoresoftware/manticoresearch/commit/dcaf4e0e3c8e9b0fbec47bbf3307feddec0936a6) 修复了来自 blended 的 RT 索引多次尾部命中问题
* [Commit eee3](https://github.com/manticoresoftware/manticoresearch/commit/eee381754e902a43a8f499e1c950198a2a3e6ee0) 修复了旋转时的死锁

## 版本 2.7.3 GA，2018年9月26日
### 改进
* CALL KEYWORDS 的 sort_mode 选项
* VIP 连接上的 DEBUG 可以执行 'crash <password>'，用于故意触发服务器上的 SIGEGV 操作
* DEBUG 可以执行 'malloc_stats'，将 malloc 统计信息输出到 searchd.log；执行 'malloc_trim' 来调用 malloc_trim()
* 如果系统中存在 gdb，改进了回溯功能

### Bug 修复
* [提交 0f3c](https://github.com/manticoresoftware/manticoresearch/commit/0f3cc333f6129451ad1ae2c97905b6c04ba5d454) 修复了 Windows 上重命名操作的崩溃或失败
* [提交 1455](https://github.com/manticoresoftware/manticoresearch/commit/1455ba2bf8079f03ce7f861a8d9662a360d705de) 修复了 32 位系统上服务器的崩溃问题
* [提交 ad37](https://github.com/manticoresoftware/manticoresearch/commit/ad3710d53b5cd6a28b8c60352d0f9e285b03d9fa) 修复了空 SNIPPET 表达式导致的服务器崩溃或挂起
* [提交 b36d](https://github.com/manticoresoftware/manticoresearch/commit/b36d79214364500252665860f000ae85343528b6) 修复了非渐进式优化损坏的问题，并修正了渐进式优化以避免为最旧的磁盘块创建杀死列表
* [提交 34b0](https://github.com/manticoresoftware/manticoresearch/commit/34b032499afd42ce47a4c7247814b4031094388a) 修复了线程池工作模式下 SQL 和 API 对 queue_max_length 返回错误的问题
* [提交 ae4b](https://github.com/manticoresoftware/manticoresearch/commit/ae4b3202cbdb8014cabe2b90e269d5cf74f49871) 修复了在 PQ 索引中添加带有 regexp 或 rlp 选项的全扫描查询时崩溃的问题
* [提交 f80f](https://github.com/manticoresoftware/manticoresearch/commit/f80f8d5d7560187078868aed9a9575f4549e98aa) 修复了连续调用多个 PQ 时的崩溃问题
* [提交 9742](https://github.com/manticoresoftware/manticoresearch/commit/9742f5f0866af73f8cd8483ecd18a507ea80dd65) 重构了 AcquireAccum
* [提交 39e5](https://github.com/manticoresoftware/manticoresearch/commit/39e5bc3751b7295222eb76407c5d72ce1dad545b) 修复了调用 pq 后的内存泄漏
* [提交 21bc](https://github.com/manticoresoftware/manticoresearch/commit/21bcc6d17395f0f57dde79f6716ef303b7ea527d) 美化重构（使用 c++11 风格的构造函数、默认值、nullptr）
* [提交 2d69](https://github.com/manticoresoftware/manticoresearch/commit/2d690398f14c736956cfdd66feb4d3091d6b3a4d) 修复了尝试向 PQ 索引插入重复项时的内存泄漏
* [提交 5ed9](https://github.com/manticoresoftware/manticoresearch/commit/5ed92c4b66da6423fa28f11b1fd59103ed1e1fb9) 修复了 JSON 字段中 IN 语句带大量值时的崩溃
* [提交 4a52](https://github.com/manticoresoftware/manticoresearch/commit/4a5262e2776aa261e34ba5c914ea60804f59de3f) 修复了 RT 索引中设置了扩展限制的 CALL KEYWORDS 语句导致服务器崩溃的问题
* [提交 5526](https://github.com/manticoresoftware/manticoresearch/commit/552646bb6fefde4e2525298e43e628839b421d3d) 修复了 PQ 匹配查询中的无效过滤器
* [提交 204f](https://github.com/manticoresoftware/manticoresearch/commit/204f521408b730198a5bab2c20407a3d7df94873) 为指针属性引入小对象分配器
* [提交 2545](https://github.com/manticoresoftware/manticoresearch/commit/25453e5387152c0575d1eda1b2beb89a49c7493a) 重构了 ISphFieldFilter 使其采用引用计数风格
* [提交 1366](https://github.com/manticoresoftware/manticoresearch/commit/1366ee06e828397074b69bec9265d7147170c60e) 修复了对非终止字符串使用 strtod 时的未定义行为和 SIGSEGV
* [提交 94bc](https://github.com/manticoresoftware/manticoresearch/commit/94bc6fcbdae4d08bcf6da2966e3ec2966091289f) 修复了 json 结果集处理中的内存泄漏
* [提交 e78e](https://github.com/manticoresoftware/manticoresearch/commit/e78e9c948963416caff9e4e46296a58080107835) 修复了应用属性添加时越界读取内存块的问题
* [提交 fad5](https://github.com/manticoresoftware/manticoresearch/commit/fad572fb543606b33e688eb82a485b86011545fd) 重构了 CSphDict 使其使用引用计数风格
* [提交 fd84](https://github.com/manticoresoftware/manticoresearch/commit/fd841a472b472280c93f4af3506851d19f6adc45) 修复了外部 AOT 内部类型的泄漏
* [提交 5ee7](https://github.com/manticoresoftware/manticoresearch/commit/5ee7f208c7685f2ebb9e5623a8802b721e2a7f0a) 修复了分词器管理的内存泄漏
* [提交 116c](https://github.com/manticoresoftware/manticoresearch/commit/116c5f1abebee9a0d99afe93546a1d8e4c6c6958) 修复了 grouper 中的内存泄漏
* [提交 56fd](https://github.com/manticoresoftware/manticoresearch/commit/56fdbc9ab38973a9a0ba8ccee45f71cf33c16423) 针对 matches 中动态指针的特殊释放/复制（修复 grouper 内存泄漏）
* [提交 b1fc](https://github.com/manticoresoftware/manticoresearch/commit/31db660f378541aa0d1e7cf2e7209bb0a1f47fd8) 修复了 RT 模式中动态字符串的内存泄漏
* [提交 517b](https://github.com/manticoresoftware/manticoresearch/commit/517b9e8749f092e923cd2884b674b5bb84e20755) 重构了 grouper
* [提交 b1fc](https://github.com/manticoresoftware/manticoresearch/commit/b1fc16140e5dc44290686330b476a254e0092107) 小幅重构（使用 c++11 构造函数，部分格式调整）
* [提交 7034](https://github.com/manticoresoftware/manticoresearch/commit/7034e073f4cf2844762e0a464b7c3de05d3d122b) 重构了 ISphMatchComparator 使其采用引用计数风格
* [提交 b1fc](https://github.com/manticoresoftware/manticoresearch/commit/b1fc16140e5dc44290686330b476a254e0092107) 将 cloner 私有化
* [提交 efbc](https://github.com/manticoresoftware/manticoresearch/commit/efbc0511d6809c4a57453c7283f9bf53f3fb8d97) 简化了 MVA_UPSIZE、DOCINFO2ID_T、DOCINFOSETID 的本地小端处理
* [提交 6da0](https://github.com/manticoresoftware/manticoresearch/commit/6da0df4ac9946fc59f98d229b90e12c9221dd4c0) 为 ubertests 增加了 valgrind 支持
* [提交 1d17](https://github.com/manticoresoftware/manticoresearch/commit/1d17669cb7ec54ac80b392e101f37a688eb98080) 修复了连接上 'success' 标志的竞态条件导致的崩溃
* [提交 5a09](https://github.com/manticoresoftware/manticoresearch/commit/5a09c32d7ded56b2c0bd3e2ad7968cb8d6362919) 将 epoll 切换到边沿触发模式
* [提交 5d52](https://github.com/manticoresoftware/manticoresearch/commit/5d528682737fca03a4352a3093b1319ec27d598c) 修复了表达式中带有过滤器格式的 IN 语句问题
* [提交 bd8b](https://github.com/manticoresoftware/manticoresearch/commit/bd8b3c976ff8b4667f55dfdd101b20a920137ac5) 修复 RT 索引在提交大文档 ID 的文档时崩溃的问题
* [提交 ce65](https://github.com/manticoresoftware/manticoresearch/commit/ce656b83b747ce7c55795b53915770c13fb99cfe) 修复 indextool 中无参数选项的问题
* [提交 08c9](https://github.com/manticoresoftware/manticoresearch/commit/08c9507177820f441f534bf06a11dac5e54bebb4) 修复扩展关键字的内存泄漏
* [提交 30c7](https://github.com/manticoresoftware/manticoresearch/commit/30c75a2f525ec9bda625d903acdc9d4d2e2413dc) 修复 json 分组器的内存泄漏
* [提交 6023](https://github.com/manticoresoftware/manticoresearch/commit/6023f269b6f2080e4d380b0957605ef8107c8c9f) 修复全局用户变量泄漏
* [提交 7c13](https://github.com/manticoresoftware/manticoresearch/commit/7c138f15ca23c0c8717fa12041e3db7f988fcf15) 修复早期拒绝匹配时动态字符串泄漏
* [提交 9154](https://github.com/manticoresoftware/manticoresearch/commit/9154b18eaed5302e8330cb3a95bd968959ccb312) 修复 length(<expression>) 的泄漏
* [提交 43fc](https://github.com/manticoresoftware/manticoresearch/commit/43fca3a4e26139765d0fac8de054200dc4875fc6) 修复解析器中因 strdup() 导致的内存泄漏
* [提交 71ff](https://github.com/manticoresoftware/manticoresearch/commit/71ff77737e63d1019b7c9804dca7fa2351025dba) 重构表达式解析器以精确跟踪引用计数

## 版本 2.7.2 GA，2018 年 8 月 27 日
### 改进
* 兼容 MySQL 8 客户端
* 支持带 RECONFIGURE 的 [TRUNCATE](Emptying_a_table.md)
* 取消 SHOW STATUS 中 RT 索引的内存计数器
* 全局多代理缓存
* 改善 Windows 下的 IOCP
* HTTP 协议的 VIP 连接
* Manticore SQL 的 [DEBUG](Reporting_bugs.md#DEBUG) 命令，可运行各种子命令
* [shutdown_token](Server_settings/Searchd.md#shutdown_token) - 需要使用 DEBUG 命令调用 `shutdown` 的密码 SHA1 哈希
* 新统计项用于 SHOW AGENT STATUS (_ping、_has_perspool、_need_resolve)
* indexer 的 --verbose 选项现在支持 \[debugvv\] 用于打印调试信息

### Bug 修复
* [提交 3900](https://github.com/manticoresoftware/manticoresearch/commit/390082a7be0a1f9539b30361d11d54de35c62a44) 移除 optimize 时的写锁
* [提交 4c33](https://github.com/manticoresoftware/manticoresearch/commit/4c337648329be22373be33333cbc9c5664d18b80) 修复重新加载索引设置时的写锁
* [提交 b5ea](https://github.com/manticoresoftware/manticoresearch/commit/b5ea8dc0b18a23dbd739e92c66dd93b0094df14b) 修复带 JSON 过滤器查询的内存泄漏
* [提交 930e](https://github.com/manticoresoftware/manticoresearch/commit/930e831a43fd3d1ce996185be968d1df49a088d0) 修复 PQ 结果集中的空文档
* [提交 53de](https://github.com/manticoresoftware/manticoresearch/commit/53deecfecb374431f84f8592a1095a77407b8aea) 修复任务混淆问题（因任务被移除）
* [提交 cad9](https://github.com/manticoresoftware/manticoresearch/commit/cad9b9c617719aad97e9f5a2ae93491153d8e504) 修正远程主机计数错误
* [提交 9000](https://github.com/manticoresoftware/manticoresearch/commit/90008cc15c4684d445d37a391bb4a667bbc2e365) 修复解析代理描述符的内存泄漏
* [提交 978d](https://github.com/manticoresoftware/manticoresearch/commit/978d839641ad6040c22c9fc3b703b1a02685f266) 修复搜索中的内存泄漏
* [提交 0193](https://github.com/manticoresoftware/manticoresearch/commit/0193946105ac08913362b7d7acf0c90f9b38dc85) 对显式/内联 c-trs、override/final 用法的外观调整
* [提交 943e](https://github.com/manticoresoftware/manticoresearch/commit/943e2997b33f9b2eda23d94a66068910f9476ebd) 修复本地/远程模式下 json 的内存泄漏
* [提交 02db](https://github.com/manticoresoftware/manticoresearch/commit/02dbdd6f1d0b72e7e458ebebdd2c67f989577e6b) 修复本地/远程模式下 json 排序列表达式的内存泄漏
* [提交 c74d](https://github.com/manticoresoftware/manticoresearch/commit/c74d0b4035e4214510376845b22cb676a8da24a3) 修复常量别名泄漏
* [提交 6e5b](https://github.com/manticoresoftware/manticoresearch/commit/6e5b57e1d3952c9695376728a143bf5434208f53) 修复预读线程泄漏
* [提交 39c7](https://github.com/manticoresoftware/manticoresearch/commit/39c740fef5bf71da0bee60a35f5dd9f471af850f) 修复因 netloop 等待卡住导致退出卡死的问题
* [提交 adaf](https://github.com/manticoresoftware/manticoresearch/commit/adaf9772f95772ad6d5297acace6c5cb92b56ad5) 修复 HA 代理切换为普通主机时 'ping' 行为卡死
* [提交 32c4](https://github.com/manticoresoftware/manticoresearch/commit/32c40eac9cdd40b15d79e58b2f15416f164f9701) 为仪表盘存储分离垃圾回收
* [提交 511a](https://github.com/manticoresoftware/manticoresearch/commit/511a3cf2c1f1858f6e94fe9f8175b7216db3cbd6) 修复引用计数指针问题
* [提交 32c4](https://github.com/manticoresoftware/manticoresearch/commit/af2326cd2927effbad7bec88e8dc238a566cf27e) 修复 indextool 在索引不存在时崩溃
* [提交 156e](https://github.com/manticoresoftware/manticoresearch/commit/156edc717dc63d3ea120f42466eba6e022da88be) 修正 xmlpipe 索引中属性/字段超出时的输出名称
* [提交 cdac](https://github.com/manticoresoftware/manticoresearch/commit/cdac6d1f17d384bfc8bae49e91241b2f346c3830) 修正配置中无 indexer 部分时索引器默认值
* [提交 e61e](https://github.com/manticoresoftware/manticoresearch/commit/e61ec00b6b27d1d5878247e2ee817f3b1e7fde16) 修复服务器重启后 RT 索引磁盘块中错误嵌入的停用词
* [提交 5fba](https://github.com/manticoresoftware/manticoresearch/commit/5fba49d5e28c17de4c0acbd984466127f42de6e8) 修复跳过虚幻连接（已关闭但未从轮询器最终删除）
* [提交 f22a](https://github.com/manticoresoftware/manticoresearch/commit/f22ae34b623906b3c5a05a06198dad2e548dc541) 修复混合（孤立）网络任务
* [提交 4689](https://github.com/manticoresoftware/manticoresearch/commit/46890e70eb8dcd00c1bf1b030538b8f131c601c2) 修复写操作后读取动作崩溃问题
* [Commit 03f9](https://github.com/manticoresoftware/manticoresearch/commit/03f9df904f611c438d5ebcaaeef911b0dc8d6c39) 修复了在 Windows 上运行测试时 searchd 崩溃的问题
* [Commit e925](https://github.com/manticoresoftware/manticoresearch/commit/e9255ed2704790a3d7f5273d38b85433463cb3ff) 修复了处理 usual connect() 上的 EINPROGRESS 代码的问题
* [Commit 248b](https://github.com/manticoresoftware/manticoresearch/commit/248b72f1544b27e9e82773284050d44eeb1714e1) 修复了使用 TFO 工作时的连接超时问题

## 版本 2.7.1 GA，2018年7月4日
### 改进
* 改进了在 PQ 上匹配多个文档时的通配符性能
* 支持 PQ 上的全扫描查询
* 支持 PQ 上的 MVA 属性
* 支持 percolate 索引的正则表达式和 RLP

### Bug 修复
* [Commit 6885](https://github.com/manticoresoftware/manticoresearch/commit/68856261b41f6950666f9e5122726839fb3c71d0) 修复了查询字符串丢失的问题
* [Commit 0f17](https://github.com/manticoresoftware/manticoresearch/commit/0f1770943cba53bfd2e0edfdf0b1495ff0dd0e89) 修复了 SHOW THREADS 语句中信息为空的问题
* [Commit 53fa](https://github.com/manticoresoftware/manticoresearch/commit/53faa36a916bb87868e83aa79c1a1f972fb20ca0) 修复了匹配使用 NOTNEAR 操作符时崩溃的问题
* [Commit 2602](https://github.com/manticoresoftware/manticoresearch/commit/26029a7cd77b518fb3a27588d7db6c8bffc73b0f) 修复了 PQ 删除时错误的过滤器提示信息


## 版本 2.7.0 GA，2018年6月11日
### 改进
* 减少系统调用次数以避免 Meltdown 和 Spectre 补丁的影响
* 本地索引管理的内部重写
* 远程 snippet 重构
* 完全配置重新加载
* 所有节点连接现在独立
* 协议改进
* Windows 通信从 wsapoll 切换到 IO 完成端口
* 主节点和节点间通讯可使用 TFO
* [SHOW STATUS](Node_info_and_management/Node_status.md#SHOW-STATUS) 现在输出服务器版本和 mysql_version_string
* 增加了 CALL PQ 中调用文档时的 `docs_id` 选项
* percolate 查询过滤器现在可以包含表达式
* 分布式索引可以与 FEDERATED 一起工作
* 虚拟的 SHOW NAMES COLLATE 和 `SET wait_timeout`（增强 ProxySQL 兼容性）

### Bug 修复
* [Commit 5bcf](https://github.com/manticoresoftware/manticoresearch/commit/5bcff05d19e495f1a44f190885b25cbb6f217c4a) 修复了 PQ 添加“不等于”标签的问题
* [Commit 9ebc](https://github.com/manticoresoftware/manticoresearch/commit/9ebc58916cd515eaa88da66d0895aebf0d1f2b5f) 修复了向 JSON 文档 CALL PQ 语句添加文档 ID 字段的问题
* [Commit 8ae0](https://github.com/manticoresoftware/manticoresearch/commit/8ae0e593a286110d8505d88baa3ac9e433cb38c9) 修复了 PQ 索引的 flush 语句处理器
* [Commit c24b](https://github.com/manticoresoftware/manticoresearch/commit/c24b152344ecd77661566cddb803487f51d3c1aa) 修复了 PQ 对 JSON 和字符串属性的过滤
* [Commit 1b8b](https://github.com/manticoresoftware/manticoresearch/commit/1b8bdde19aaa362785ea4c33bdc019154f8fbac4) 修复了空 JSON 字符串的解析
* [Commit 1ad8](https://github.com/manticoresoftware/manticoresearch/commit/1ad8a07dbfc1131913a8d92c261fbb48f934e8b7) 修复了多查询中带 OR 过滤器时的崩溃
* [Commit 69b8](https://github.com/manticoresoftware/manticoresearch/commit/69b89806380a229e36287ad9daf6f0ea2b5453eb) 修复了 indextool 使用配置公共部分（lemmatizer_base 选项）处理命令（dumpheader）
* [Commit 6dbe](https://github.com/manticoresoftware/manticoresearch/commit/6dbeaf2389d2af6a9dfccb56bfc986685b9f203e) 修复了结果集和过滤器中的空字符串
* [Commit 39c4](https://github.com/manticoresoftware/manticoresearch/commit/39c4eb55ecc85f6dd54f6c8c6d6dfcf87fd8748e) 修复了文档 ID 的负值问题
* [Commit 266b](https://github.com/manticoresoftware/manticoresearch/commit/266b7054f98e7d30fc2829b5c2467dc9f90def92) 修复了对非常长词语建立索引时的词截断长度
* [Commit 4782](https://github.com/manticoresoftware/manticoresearch/commit/47823b0bd06cbd016a24345c99bb8693790b43dd) 修复了 PQ 上通配符查询匹配多个文档的问题


## 版本 2.6.4 GA，2018年5月3日
### 功能和改进
* MySQL FEDERATED 引擎的[支持](Extensions/FEDERATED.md)
* MySQL 数据包现在返回 SERVER_STATUS_AUTOCOMMIT 标志，增强了与 ProxySQL 的兼容性
* [listen_tfo](Server_settings/Searchd.md#listen_tfo) - 为所有监听启用 TCP Fast Open 连接
* indexer --dumpheader 现在也可以从 .meta 文件转储 RT 头信息
* 针对 Ubuntu Bionic 的 cmake 构建脚本

### Bug 修复
* [Commit 355b](https://github.com/manticoresoftware/manticoresearch/commit/355b11629174813abd9cd5bf2233be0783f77745) 修复了 RT 索引的无效查询缓存条目；
* [Commit 546e](https://github.com/manticoresoftware/manticoresearch/commit/546e2297a01cc2913bc0d33052d5bcefae8737eb) 修复了无缝旋转后索引设置丢失的问题
* [Commit 0c45](https://github.com/manticoresoftware/manticoresearch/commit/0c4509898393993bc87194d2aca2070395ff7f83) 修复了中缀与前缀长度设置问题；增加不支持中缀长度时的警告
* [Commit 8054](https://github.com/manticoresoftware/manticoresearch/commit/80542fa54d727c781635918e8d4d65a20c2fbd1e) 修复了 RT 索引自动刷新顺序
* [Commit 705d](https://github.com/manticoresoftware/manticoresearch/commit/705d8c5f407726ed1c6f9745ecffa375682c8969) 修复了多个属性索引及查询多个索引时结果集架构的问题
* [Commit b0ba](https://github.com/manticoresoftware/manticoresearch/commit/b0ba932fff4a082f642cb737381c8a70215d4cc9) 修复了批量插入时部分命中由于文档重复丢失的问题
* [Commit 4510](https://github.com/manticoresoftware/manticoresearch/commit/4510fa44ad54b5bbac53c591144501565130fcae) 修复了优化时未能合并大型文档数的 RT 索引磁盘块问题

## 版本 2.6.3 GA，2018年3月28日
### 改进
* 编译时支持 jemalloc。如果系统有 jemalloc，可通过 cmake 选项 `-DUSE_JEMALLOC=1` 启用

### Bug 修复
* [Commit 85a6](https://github.com/manticoresoftware/manticoresearch/commit/85a6d7e3db75f28c491042409bb677ed39eba07b) 修正了 Manticore SQL 查询日志中的 expand_keywords 选项
* [Commit caaa](https://github.com/manticoresoftware/manticoresearch/commit/caaa3842f4dbd8507fd9e20bf3371a0c8ec0c087) 修正了 HTTP 接口以正确处理大尺寸查询
* [Commit e386](https://github.com/manticoresoftware/manticoresearch/commit/e386d842b36195719e459b2db1fb20e402f9a51f) 修正了启用了 index_field_lengths 的 RT 索引执行 DELETE 时服务器崩溃的问题
* [Commit cd53](https://github.com/manticoresoftware/manticoresearch/commit/ce528f37eb5d8284661928f6f3b340a4a1985253) 修正了 cpustats searchd CLI 选项使其在不支持的系统上工作
* [Commit 8740](https://github.com/manticoresoftware/manticoresearch/commit/8740fd63ae842c2ab0e51a8fcd6180d787ed57ec) 修正了定义了最小长度时 UTF8 子串匹配的问题


## 版本 2.6.2 GA，2018年2月23日
### 改进
* 提升了 [Percolate Queries](Searching/Percolate_query.md) 在使用 NOT 操作符以及批量文档时的性能。
* [percolate_query_call](Searching/Percolate_query.md) 可以根据 [dist_threads](Server_settings/Searchd.md#threads) 使用多线程
* 新增全文匹配操作符 NOTNEAR/N
* 支持 percolate 索引上的 SELECT 的 LIMIT
* [expand_keywords](Searching/Options.md#expand_keywords) 可接受 'start'、'exact'（其中 'start,exact' 与 '1' 效果相同）
* 针对使用 sql_query_range 定义的范围查询，支持针对 [joined fields](Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Processing_fetched_data.md#sql_joined_field) 的有范围主查询

### Bug修复
* [Commit 72dc](https://github.com/manticoresoftware/manticoresearch/commit/72dcf669744e9b7d636dfc213d24df85ab301f6b) 修正了搜索内存段时崩溃；修正了保存磁盘块时双缓冲导致的死锁；修正了优化过程中保存磁盘块导致的死锁
* [Commit 3613](https://github.com/manticoresoftware/manticoresearch/commit/36137149a1c3c0893bdda5a28fc7e8244bf2d4ae) 修正了带有空属性名的 XML 嵌入式 schema 导致的索引器崩溃
* [Commit 48d7](https://github.com/manticoresoftware/manticoresearch/commit/48d7e8001d2a66466ca64577f27ddc5421a67251) 修正了错误地取消链接非拥有的 pid 文件
* [Commit a556](https://github.com/manticoresoftware/manticoresearch/commit/a5563a465ddc59ef71e65f17b68bc33f9700e838) 修正了临时文件夹中有时遗留的孤儿 FIFO 文件
* [Commit 2376](https://github.com/manticoresoftware/manticoresearch/commit/2376e8fc4508944b96959bd10686c6d51f5145e8) 修正了 FACET 结果集中空且错误的 NULL 行
* [Commit 4842](https://github.com/manticoresoftware/manticoresearch/commit/4842b67acdd5da75672db6c08ac563b48577ddd7) 修正了作为 Windows 服务运行时索引锁损坏的问题
* [Commit be35](https://github.com/manticoresoftware/manticoresearch/commit/be35feef54ded59125750916cd211e56108eddae) 修正了 macOS 上错误的 iconv 库
* [Commit 8374](https://github.com/manticoresoftware/manticoresearch/commit/83744a977d7837d1d87cd506b88b0e7eb199efe6) 修正了错误的 count(\*) 计算


## 版本 2.6.1 GA，2018年1月26日
### 改进
* [agent_retry_count](Server_settings/Searchd.md#agent_retry_count) 针对带有镜像(agent mirrors)的代理，重试次数计数改为每个镜像的重试次数，代理总重试次数为 agent_retry_count\*镜像数。
* [agent_retry_count](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_retry_count) 现在可以针对每个索引单独指定，覆盖全局值。新增别名 [mirror_retry_count](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#mirror_retry_count)。
* 可在代理定义中指定 retry_count，该值表示每个代理的重试次数
* Percolate Queries 现已支持 HTTP JSON API，可在 [/json/pq](Searching/Percolate_query.md#Performing-a-percolate-query-with-CALL-PQ) 使用。
* 给可执行文件新增了 -h 和 -v 选项（帮助和版本）
* [morphology_skip_fields](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) 支持实时索引（Real-Time indexes）

### Bug修复
* [Commit a40b](https://github.com/manticoresoftware/manticore/commit/a40b0793feff65e40d10062568d9847c08d10f57) 修正了针对 MVA 字段使用 sql_range_step 时的 ranged-main-query 正确工作
* [Commit f2f5](https://github.com/manticoresoftware/manticore/commit/f2f53757db45bcfb1544263ce0817e856656a621) 修正了黑洞系统循环挂起和黑洞代理似乎断开的问题
* [Commit 84e1](https://github.com/manticoresoftware/manticore/commit/84e1f54aef25e0fce98870ad2dd784db5116f1d6) 修正了查询 ID 保持一致以及存储查询 ID 重复的问题
* [Commit 1948](https://github.com/manticoresoftware/manticore/commit/19484231814fcb82b21763a3a4a9f45adc6b2d40) 修正了多种状态下服务器关机时崩溃的问题
* [Commit 9a70](https://github.com/manticoresoftware/manticore/commit/9a706b499a1d61a90076065a1a703029d49db958)  [Commit 3495](https://github.com/manticoresoftware/manticore/commit/3495fd70cba8846b1a50d55d6679b039414c3d2a) 修正在长查询时超时的问题
* [Commit 3359](https://github.com/manticoresoftware/manticore/commit/3359bcd89b4f79a645fe84b8cf8616ce0addff02) 重构了基于 kqueue 系统（Mac OS X、BSD）的主-代理网络轮询


## 版本 2.6.0，2017年12月29日
### 特性与改进
* [HTTP JSON](Connecting_to_the_server/HTTP.md)：JSON 查询现在支持对属性的等值判断，MVA 和 JSON 属性可用于插入和更新，通过 JSON API 也可以对分布式索引执行更新和删除
* [Percolate Queries](Searching/Percolate_query.md)
* 从代码中移除了对 32 位 docid 的支持，同时移除了所有转换/加载 32 位 docid 旧索引的代码
* [仅针对特定字段的形态学处理](https://github.com/manticoresoftware/manticore/issues/7)。新增索引指令 morphology_skip_fields 允许定义不应用形态学处理的字段列表。
* [expand_keywords 现在可以通过 OPTION 语句设置为查询运行时指令](https://github.com/manticoresoftware/manticore/issues/8)

### Bug修复
* [提交 0cfa](https://github.com/manticoresoftware/manticore/commit/0cfae4c) 修复了使用 rlp 构建时服务器（以及发布版可能未定义行为）调试构建的崩溃问题
* [提交 3242](https://github.com/manticoresoftware/manticore/commit/324291e) 修复了启用 progressive 选项的 RT 索引优化中合并顺序错误的 kill-lists 问题
* [提交 ac0e](https://github.com/manticoresoftware/manticore/commit/ac0efee) Mac 平台的轻微崩溃修复
* 经过彻底静态代码分析后修复的许多小问题
* 其它小的 bug 修复

### 升级
在此版本中，我们更改了 masters 和 agents 之间通信所使用的内部协议。如果您在分布式环境中运行多个 Manticoresearch 实例，请确保先升级 agents，然后再升级 masters。

## 版本 2.5.1，2017年11月23日
### 特性和改进
* 支持基于 [HTTP API 协议](Connecting_to_the_server/HTTP.md) 的 JSON 查询。支持搜索、插入、更新、删除、替换操作。数据操作命令也可以批量执行，目前有一些限制，例如 MVA 和 JSON 属性不能用于插入、替换或更新。
* [RELOAD INDEXES](Data_creation_and_modification/Adding_data_from_external_storages/Rotating_a_table.md#RELOAD-TABLES) 命令
* [FLUSH LOGS](Logging/Rotating_query_and_server_logs.md) 命令
* [SHOW THREADS](Node_info_and_management/SHOW_THREADS.md) 可以显示优化、旋转或刷新进度。
* GROUP N BY 正确支持 MVA 属性
* 黑洞 agents 运行在独立线程中，不再影响主查询
* 对索引实现了引用计数，避免旋转及高负载导致的停顿
* 实现了 SHA1 哈希，尚未对外公开
* 修正了在 FreeBSD、macOS 和 Alpine 上的编译问题

### Bug修复
* [提交 9897](https://github.com/manticoresoftware/manticore/commit/989752b) 修复了基于块索引的过滤回归问题
* [提交 b1c3](https://github.com/manticoresoftware/manticore/commit/b1c3864) 重命名 PAGE_SIZE -> ARENA_PAGE_SIZE，以兼容 musl
* [提交 f213](https://github.com/manticoresoftware/manticore/commit/f2133cc) 禁用 cmake 版本低于 3.1.0 的 googletests
* [提交 f30e](https://github.com/manticoresoftware/manticore/commit/0839de7) 修复服务器重启时无法绑定套接字的问题
* [提交 0807](https://github.com/manticoresoftware/manticore/commit/0807240) 修复服务器关闭时的崩溃
* [提交 3e3a](https://github.com/manticoresoftware/manticore/commit/3e3acc3) 修复显示系统黑洞线程的 show threads
* [提交 262c](https://github.com/manticoresoftware/manticore/commit/262c3fe) 重新设计 iconv 配置检查，修复 FreeBSD 和 Darwin 上的构建问题

## 版本 2.4.1 GA，2017年10月16日
### 特性与改进
* WHERE 子句中属性过滤器之间支持 OR 操作符
* 维护模式 ( SET MAINTENANCE=1)
* [CALL KEYWORDS](Searching/Autocomplete.md#CALL-KEYWORDS) 支持分布式索引
* [UTC 分组](Server_settings/Searchd.md#grouping_in_utc)
* 支持 [query_log_mode](Server_settings/Searchd.md#query_log_mode) 来自定义日志文件权限
* 字段权重可为零或负值
* [max_query_time](Searching/Options.md#max_query_time) 现在可影响全扫描查询
* 新增 [net_wait_tm](Server_settings/Searchd.md#net_wait_tm)、[net_throttle_accept](Server_settings/Searchd.md#net_throttle_accept) 和 [net_throttle_action](Server_settings/Searchd.md#net_throttle_action) 参数，用于网络线程微调（当 workers=thread_pool 时）
* COUNT DISTINCT 支持 facet 搜索
* IN 支持 JSON 浮点数组
* 多查询优化不会被整数/浮点表达式破坏
* 使用多查询优化时，[SHOW META](Node_info_and_management/SHOW_META.md) 显示一个 `multiplier` 行

### 编译
Manticore Search 使用 cmake 构建，最低支持的 gcc 版本为 4.7.2。

### 文件夹与服务
* Manticore Search 以 `manticore` 用户身份运行。
* 默认数据文件夹现在是 `/var/lib/manticore/`。
* 默认日志文件夹现在是 `/var/log/manticore/`。
* 默认 pid 文件夹现在是 `/var/run/manticore/`。

### Bug修复
* [提交 a58c](https://github.com/manticoresoftware/manticore/commit/a58c619) 修复 SHOW COLLATION 语句导致 Java 连接断开的错误
* [提交 631c](https://github.com/manticoresoftware/manticore/commit/631cf4e) 修复处理分布式索引时的崩溃；为分布式索引哈希添加锁；移除 agent 的移动与复制操作符
* [提交 942b](https://github.com/manticoresoftware/manticore/commit/942bec0) 修复分布式索引并行重连导致的崩溃
* [提交 e5c1](https://github.com/manticoresoftware/manticore/commit/e5c1ed2) 修复崩溃处理程序在存储查询至服务器日志时的崩溃
* [提交 4a4b](https://github.com/manticoresoftware/manticore/commit/4a4bda5) 修复多查询中使用的池化属性导致的崩溃
* [提交 3873](https://github.com/manticoresoftware/manticore/commit/3873bfb) 通过防止索引页包含在 core 文件中减小 core 文件大小
* [提交 11e6](https://github.com/manticoresoftware/manticore/commit/11e6254) 修复启动时指定无效 agents 导致的 searchd 崩溃
* [提交 4ca6](https://github.com/manticoresoftware/manticore/commit/4ca6350) 修复 sql_query_killlist 查询中索引器错误报告
* [提交 123a](https://github.com/manticoresoftware/manticore/commit/123a9f0) 修复 fold_lemmas=1 与命中计数的问题
* [提交 cb99](https://github.com/manticoresoftware/manticore/commit/cb99164) 修复 html_strip 行为不一致问题
* [提交 e406](https://github.com/manticoresoftware/manticore/commit/e406761) 修复优化 rt 索引时丢失新设置；修复带同步选项的优化时锁泄漏
* [提交 86ae](https://github.com/manticoresoftware/manticore/commit/86aeb82) 修复错误的多查询处理问题
* [Commit 2645](https://github.com/manticoresoftware/manticore/commit/2645230) 修正了结果集依赖多查询顺序的问题
* [Commit 7239](https://github.com/manticoresoftware/manticore/commit/72395d9) 修正了带有错误查询的多查询导致服务器崩溃的问题
* [Commit f353](https://github.com/manticoresoftware/manticore/commit/f353326) 修正了从共享锁到独占锁的转换问题
* [Commit 3754](https://github.com/manticoresoftware/manticore/commit/3754785) 修正了无索引查询导致服务器崩溃的问题
* [Commit 29f3](https://github.com/manticoresoftware/manticore/commit/29f360e) 修正了服务器死锁问题

## 版本 2.3.3，2017年7月6日
* Manticore 品牌标识

