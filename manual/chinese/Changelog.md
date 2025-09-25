# 更新日志

## 版本 13.11.1
**发布日期**：2025年9月13日

### Bug 修复
* 🪲 [v13.11.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.11.1) [ Issue #3751](https://github.com/manticoresoftware/manticoresearch/issues/3751) 修复启用嵌入时删除文档导致崩溃的问题。

## 版本 13.11.0
**发布日期**：2025年9月13日

此次发布的主要亮点是[自动嵌入](Searching/KNN.md#Auto-Embeddings-%28Recommended%29)——一项让语义搜索如同使用 SQL 一样简单的新功能。
无需外部服务或复杂管道：只需插入文本，即可通过自然语言进行搜索。

### 自动嵌入提供的功能

- **直接从文本自动生成嵌入**
- **理解语义的自然语言查询，而不仅仅是关键词**
- **支持多种模型**（OpenAI、Hugging Face、Voyage、Jina）
- **与 SQL 和 JSON API 的无缝集成**

### 推荐的库
- 推荐 [MCL](https://github.com/manticoresoftware/columnar) 版本：8.0.3
- 推荐 [Buddy](Installation/Manticore_Buddy.md#Manticore-Buddy) 版本：3.35.1

如果您遵循[官方安装指南](https://manticoresearch.com/install/)，则无需担心这些细节。

### 新功能与改进
* 🆕 [v13.11.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.11.0) [ PR #3746](https://github.com/manticoresoftware/manticoresearch/pull/3746) 在 JSON 查询中添加了用于生成嵌入的 "query" 支持。
* 🆕 [v13.10.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.10.0) [ Issue #3709](https://github.com/manticoresoftware/manticoresearch/issues/3709) manticore-server RPM 包不再拥有 `/run`。
* 🆕 [v13.9.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.9.0) [ PR #3716](https://github.com/manticoresoftware/manticoresearch/pull/3716) 配置中添加了对 `boolean_simplify` 的支持。
* 🆕 [v13.8.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.8.0) [ Issue #3253](https://github.com/manticoresoftware/manticoresearch/issues/3253) 安装了 sysctl 配置以增加大数据集的 vm.max_map_count。
* 🆕 [v13.7.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.7.0) [ PR #3681](https://github.com/manticoresoftware/manticoresearch/pull/3681) 支持 `alter table <table> modify column <column> api_key=<key>`。

### Bug 修复
* 🪲 [v13.10.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.10.5) [ PR #3737](https://github.com/manticoresoftware/manticoresearch/pull/3737) scroll 选项现在正确返回所有具有大型 64 位 ID 的文档。
* 🪲 [v13.10.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.10.4) [ PR #3736](https://github.com/manticoresoftware/manticoresearch/pull/3736) 修复使用带有过滤树的 KNN 时的崩溃问题。
* 🪲 [v13.10.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.10.3) [ Issue #3520](https://github.com/manticoresoftware/manticoresearch/issues/3520) `/sql` 端点不再支持 SHOW VERSION 命令。
* 🪲 [v13.10.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.10.2) [ PR #3637](https://github.com/manticoresoftware/manticoresearch/pull/3637) 更新了 Windows 安装脚本。
* 🪲 [v13.10.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.10.1) 修复了 Linux 上的本地时区检测。
* 🪲 [v13.9.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.9.2) [ PR #3726](https://github.com/manticoresoftware/manticoresearch/pull/3726) 列式模式下重复的 ID 现在正确返回错误。
* 🪲 [v13.9.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.9.1) [ PR #3333](https://github.com/manticoresoftware/manticoresearch/pull/3333) 手册现已自动翻译。
* 🪲 [v13.8.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.8.6) [ PR #3715](https://github.com/manticoresoftware/manticoresearch/pull/3715) 修复批次中全部文档被跳过时嵌入生成的问题。
* 🪲 [v13.8.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.8.5) [ PR #3711](https://github.com/manticoresoftware/manticoresearch/pull/3711) 添加了 Jina 和 Voyage 嵌入模型，以及其他自动嵌入相关改动。
* 🪲 [v13.8.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.8.4) [ PR #3710](https://github.com/manticoresoftware/manticoresearch/pull/3710) 修复了多重属性面板联接查询时的崩溃。
* 🪲 [v13.8.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.8.3) 修复了 _bulk 端点中多语句事务删除/更新提交未被计为错误的问题。
* 🪲 [v13.8.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.8.2) [ PR #3705](https://github.com/manticoresoftware/manticoresearch/pull/3705) 修复非列式字符串属性联接时的崩溃并改进错误报告。
* 🪲 [v13.8.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.8.1) [ PR #3704](https://github.com/manticoresoftware/manticoresearch/pull/3704) 修复查询嵌入未指定模型时的崩溃；新增嵌入字符串到主代理通信；添加了测试。
* 🪲 [v13.7.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.7.2) [ PR #Buddy#589](https://github.com/manticoresoftware/manticoresearch-buddy/pull/589) 移除了模糊搜索的默认 IDF 黑客。
* 🪲 [v13.7.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.7.1) [ Issue #3454](https://github.com/manticoresoftware/manticoresearch/issues/3454) 修复大型 64 位 ID 的 scroll 解码错误。
* 🪲 [v13.6.9](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.9) [ Issue #3674](https://github.com/manticoresoftware/manticoresearch/issues/3674) 修复了 JDBC+MySQL 驱动/连接池在 transaction_read_only 设置下的问题。
* 🪲 [v13.6.8](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.8) [ PR #3676](https://github.com/manticoresoftware/manticoresearch/pull/3676) 修复了嵌入模型返回空结果集时的崩溃。

## 版本 13.6.7
**发布日期**：2025年8月8日

推荐的 [MCL](https://github.com/manticoresoftware/columnar) 版本：8.0.1  
推荐的 [Buddy](Installation/Manticore_Buddy.md#Manticore-Buddy) 版本：3.34.2  

如果您遵循[官方安装指南](https://manticoresearch.com/install/)，则无需担心此问题。

### 新功能和改进
* 🆕 [v13.6.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.0) [ Issue #2226](https://github.com/manticoresoftware/manticoresearch/issues/2226) 支持 PHRASE、PROXIMITY 和 QUORUM 运算符中显式使用 '|'（或）。
* 🆕 [v13.5.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.5.0) [ PR #3591](https://github.com/manticoresoftware/manticoresearch/pull/3591) 查询中的自动嵌入生成（进行中，尚未准备好生产环境使用）。
* 🆕 [v13.4.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.4.0) [ PR #3585](https://github.com/manticoresoftware/manticoresearch/pull/3585) 修正逻辑：如果设置了 buddy_path 配置，则优先使用 buddy 线程数，而非使用守护进程值。
* 🆕 [v13.3.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.3.0) [ PR #3577](https://github.com/manticoresoftware/manticoresearch/pull/3577) 支持与本地分布式表的连接。
* 🆕 [#3647](https://github.com/manticoresoftware/manticoresearch/issues/3647) 新增对 Debian 13 “Trixie”的支持。

### Bug 修复
* 🪲 [v13.6.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.7) [ Issue #3524](https://github.com/manticoresoftware/manticoresearch/issues/3524) 修复了在行存储中保存生成的嵌入时的问题。
* 🪲 [v13.6.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.6) [ Issue #3563](https://github.com/manticoresoftware/manticoresearch/issues/3563) 修复了 Sequel Ace 和其他集成因“未知 sysvar”错误而失败的问题。
* 🪲 [v13.6.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.5) [ Issue #3467](https://github.com/manticoresoftware/manticoresearch/issues/3467) 修复了 DBeaver 和其他集成因“未知 sysvar”错误而失败的问题。
* 🪲 [v13.6.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.4) [ Issue #3524](https://github.com/manticoresoftware/manticoresearch/issues/3524) 修复了多字段嵌入连接的问题；同时修复了从查询生成嵌入的问题。
* 🪲 [v13.6.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.3) [ Issue #3641](https://github.com/manticoresoftware/manticoresearch/issues/3641) 修复了版本 13.6.0 中的一个错误：一个短语除了第一个之外，丢失了所有括号内的关键字。
* 🪲 [v13.6.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.2) 修复了 transform_phrase 中的内存泄漏。
* 🪲 [v13.6.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.1) 修复了版本 13.6.0 中的内存泄漏。
* 🪲 [v13.5.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.5.2) [ Issue #3651](https://github.com/manticoresoftware/manticoresearch/issues/3651) 修复了更多与全文搜索模糊测试相关的问题。
* 🪲 [v13.5.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.5.1) [ Issue #3560](https://github.com/manticoresoftware/manticoresearch/issues/3560) 修复了包含 KNN 数据时，OPTIMIZE TABLE 可能无限挂起的情况。
* 🪲 [v13.4.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.4.2) [ Issue #2544](https://github.com/manticoresoftware/manticoresearch/issues/2544) 修复了添加 float_vector 列可能导致索引损坏的问题。
* 🪲 [v13.4.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.4.1) [ Issue #3651](https://github.com/manticoresoftware/manticoresearch/issues/3651) 为全文解析增加了模糊测试并修复了测试中发现的若干问题。
* 🪲 [v13.3.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.3.1) [ Issue #3583](https://github.com/manticoresoftware/manticoresearch/issues/3583) 修复了使用复杂布尔过滤器与高亮显示时的崩溃。
* 🪲 [v13.2.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.7) [ Issue #3481](https://github.com/manticoresoftware/manticoresearch/issues/3481) 修复了同时使用 HTTP 更新、分布式表和错误的复制集群时的崩溃。
* 🪲 [v13.2.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.6) [ PR #3567](https://github.com/manticoresoftware/manticoresearch/pull/3567) 更新 manticore-backup 依赖至版本 1.9.6。
* 🪲 [v13.2.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.5) [ PR #3565](https://github.com/manticoresoftware/manticoresearch/pull/3565) 修复 CI 配置以提升 Docker 镜像兼容性。
* 🪲 [v13.2.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.4) 修正了长词元的处理。某些特殊的词元（如正则表达式模式）可能会生成过长的词，因此现在使用前会进行缩短。

## 版本 13.2.3
**发布日期**：2025年7月8日

### 不兼容变更
* ⚠️ [v13.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.0.0) 更新了 KNN 库 API 以支持空的 [float_vector](https://manual.manticoresearch.com/Creating_a_table/Data_types#Float-vector) 值。此更新未更改数据格式，但会提升 Manticore Search / MCL API 版本。
* ⚠️ [v12.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/12.0.0) [ PR #3516](https://github.com/manticoresoftware/manticoresearch/pull/3516) 修复了KNN索引训练和构建过程中源和目标行ID不正确的错误。此更新不改变数据格式，但提高了Manticore Search / MCL API版本。
* ⚠️ [v11.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.0.0) 添加了对新的KNN向量搜索功能的支持，如量化、重评分和过采样。此版本更改了KNN数据格式和[KNN_DIST() SQL语法](https://manual.manticoresearch.com/Searching/KNN?client=SQL#KNN-vector-search)。新版本可以读取旧数据，但旧版本不能读取新格式。

### 新功能和改进
* 🆕 [v13.2.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.0) [ PR #3549](https://github.com/manticoresoftware/manticoresearch/issues/3525) 修复了`@@collation_database`导致与某些mysqldump版本不兼容的问题
* 🆕 [v13.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.1.0) [ Issue #3489](https://github.com/manticoresoftware/manticoresearch/issues/3489) 修复了模糊搜索中导致某些SQL查询无法解析的错误
* 🆕 [v12.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/12.1.0) [ Issue #3426](https://github.com/manticoresoftware/manticoresearch/issues/3426) 添加了对RHEL 10操作系统的支持
* 🆕 [v11.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.1.0) 支持在[KNN搜索](https://manual.manticoresearch.com/Searching/KNN#KNN-vector-search)中使用空的浮点向量
* 🆕 [v10.2.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.0) [ Issue #3252](https://github.com/manticoresoftware/manticoresearch/issues/3252) [log_level](https://manual.manticoresearch.com/Server_settings/Setting_variables_online#SET) 现在也调整Buddy日志的详细程度

### 错误修复
* 🪲 [v13.2.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.3) [ PR #3556](https://github.com/manticoresoftware/manticoresearch/pull/3556) 修复了JSON查询中"oversampling"选项解析的问题
* 🪲 [v13.2.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.2) 修复了通过移除Boost堆栈跟踪导致的Linux和FreeBSD上的崩溃日志问题
* 🪲 [v13.2.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.1) [ Issue #3298](https://github.com/manticoresoftware/manticoresearch/issues/3298) 修复了容器内运行时的崩溃日志问题
* 🪲 [v12.0.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/12.0.2) 改进了每表统计的准确性，使用微秒级计数
* 🪲 [v12.0.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/12.0.1) [ PR #3522](https://github.com/manticoresoftware/manticoresearch/pull/3522) 修复了连接查询中按MVA分面时导致的崩溃
* 🪲 [v11.0.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.0.3) [ Issue #3502](https://github.com/manticoresoftware/manticoresearch/issues/3502) 修复了与向量搜索量化相关的崩溃
* 🪲 [v11.0.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.0.2) [ Issue #3493](https://github.com/manticoresoftware/manticoresearch/issues/3493) 将`SHOW THREADS`中的CPU利用率显示为整数
* 🪲 [v11.0.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.0.1) 修正了列向和二级库的路径
* 🪲 [v10.2.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.7) 支持MCL 5.0.5，包括嵌入库文件名的修复
* 🪲 [v10.2.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.6) [ Issue #3469](https://github.com/manticoresoftware/manticoresearch/issues/3469) 对问题#3469应用了另一项修复
* 🪲 [v10.2.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.4) [ Issue #3469](https://github.com/manticoresoftware/manticoresearch/issues/3469) 修复了带有布尔查询的HTTP请求返回空结果的问题
* 🪲 [v10.2.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.5) 更改了嵌入库的默认文件名，并添加了对KNN向量中'from'字段的检查
* 🪲 [v10.2.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.3) [ PR #3464](https://github.com/manticoresoftware/manticoresearch/pull/3464) 更新Buddy至3.30.2版本，其中包括[PR #565关于内存使用和错误日志的改进](https://github.com/manticoresoftware/manticoresearch-buddy/pull/565)
* 🪲 [v10.2.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.2) [ Issue #3470](https://github.com/manticoresoftware/manticoresearch/pull/3470) 修复了JOIN查询中JSON字符串过滤器、null过滤器和排序问题
* 🪲 [v10.2.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.1) 修复了`dist/test_kit_docker_build.sh`中的一个bug，该bug导致缺少Buddy提交信息
* 🪲 [v10.1.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.4) 修复了在连接查询中按MVA分组时的崩溃
* 🪲 [v10.1.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.3) [ Issue #3434](https://github.com/manticoresoftware/manticoresearch/issues/3434) 修复了过滤空字符串的错误
* 🪲 [v10.1.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.2) [ PR #3452](https://github.com/manticoresoftware/manticoresearch/pull/3452) 更新 Buddy 到版本 3.29.7，修复了 [Buddy #507 - 模糊搜索多查询请求错误](https://github.com/manticoresoftware/manticoresearch-buddy/issues/507) 和 [Buddy #561 - 指标速率修复](https://github.com/manticoresoftware/manticoresearch-buddy/pull/561)，此版本为 [Helm Release 10.1.0](https://github.com/manticoresoftware/manticoresearch-helm/releases/tag/manticoresearch-10.1.0) 所需
* 🪲 [v10.1.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.1) 更新 Buddy 到版本 3.29.4，解决了 [#3388 - “未定义的数组键 'Field'”](https://github.com/manticoresoftware/manticoresearch/issues/3388) 和 [Buddy #547 - layouts='ru' 可能不工作](https://github.com/manticoresoftware/manticoresearch-buddy/issues/547)

## 版本 10.1.0
**发布**：2025年6月9日

该版本是一次包含新功能、一个重大变更及众多稳定性改进和错误修复的更新。更改重点是增强监控能力、改进搜索功能以及修复影响系统稳定性和性能的多个关键问题。

**从版本 10.1.0 起，不再支持 CentOS 7。建议用户升级到受支持的操作系统。**

### 重大变更
* ⚠️ [v10.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.0.0) [ Issue #540](https://github.com/manticoresoftware/manticoresearch-buddy/issues/540) 重大变更：将 [模糊搜索](Searching/Spell_correction.md#Fuzzy-Search) 的默认 `layouts=''`

### 新功能和改进
* 🆕 [v10.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.0) [ Issue #537](https://github.com/manticoresoftware/manticoresearch-buddy/issues/537) 添加内置的 [Prometheus 导出器](Node_info_and_management/Node_status.md#Prometheus-Exporter)
* 🆕 [v9.8.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.8.0) [ Issue #3409](https://github.com/manticoresoftware/manticoresearch/issues/3409) 添加 [ALTER TABLE tbl REBUILD KNN](Updating_table_schema_and_settings.md#Rebuilding-a-KNN-index)
* 🆕 [v9.7.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.7.0) [ Issue #1778](https://github.com/manticoresoftware/manticoresearch/issues/1778) 添加自动生成嵌入（尚未正式宣布，因为代码已在主分支但仍需更多测试）
* 🆕 [v9.6.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.6.0) 提升 KNN API 版本以支持自动嵌入
* 🆕 [v9.5.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.0) [ Issue #1894](https://github.com/manticoresoftware/manticoresearch/issues/1894) 改进集群恢复：定期保存 `seqno`，加快节点崩溃后的重启速度
* 🆕 [v9.4.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.4.0) [ Issue #2400](https://github.com/manticoresoftware/manticoresearch/issues/2400) 支持最新的 [Logstash](Integration/Logstash.md#Integration-with-Logstash) 和 Beats

### 错误修复
* 🪲 [v10.0.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.0.1) 修正词形处理：用户定义的词形覆盖自动生成的词形；添加测试用例至测试22
* 🪲 [v9.8.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.8.2) 修复：再次更新 deps.txt，包含 MCL 中与嵌入库相关的打包修复
* 🪲 [v9.8.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.8.1) 修复：更新 deps.txt，包含 MCL 和嵌入库的打包修复
* 🪲 [v9.7.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.7.3) [ Issue #3306](https://github.com/manticoresoftware/manticoresearch/issues/3306) 修复索引时因信号11导致的崩溃
* 🪲 [v9.7.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.7.2) [ Issue #3109](https://github.com/manticoresoftware/manticoresearch/issues/3109) 修复不存在的 `@@variables` 总是返回0的问题
* 🪲 [v9.7.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.7.1) [ Issue #3377](https://github.com/manticoresoftware/manticoresearch/issues/3377) 修复与 [remove_repeats()](Functions/Searching_and_ranking_functions.md#REMOVE_REPEATS%28%29) 相关的崩溃
* 🪲 [v9.6.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.6.3) [ PR #3411](https://github.com/manticoresoftware/manticoresearch/pull/3411) 修复：对遥测指标使用动态版本检测
* 🪲 [v9.6.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.6.2) 修复：[SHOW VERSION](Node_info_and_management/SHOW_VERSION.md#SHOW-VERSION) 输出的小修正
* 🪲 [v9.6.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.6.1) 修复：创建带有 KNN 属性但无模型的表时崩溃
* 🪲 [v9.5.16](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.16) [ Issue #3342](https://github.com/manticoresoftware/manticoresearch/issues/3342) 修复 `SELECT ... FUZZY=0` 并不总是禁用模糊搜索的问题
* 🪲 [v9.5.15](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.15) [ PR #3397](https://github.com/manticoresoftware/manticoresearch/pull/3397) 支持 [MCL](https://github.com/manticoresoftware/columnar) 4.2.2；修复旧存储格式错误
* 🪲 [v9.5.14](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.14) [ Issue #3392](https://github.com/manticoresoftware/manticoresearch/issues/3392) 修复 HTTP JSON 应答中字符串的错误处理
* 🪲 [v9.5.13](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.13) [ Issue #3356](https://github.com/manticoresoftware/manticoresearch/issues/3356) 修复复杂全文检索查询（常见子词项）中的崩溃问题
* 🪲 [v9.5.12](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.12)  修正了磁盘块自动刷新错误信息中的拼写错误
* 🪲 [v9.5.11](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.11) [ Issue #3195](https://github.com/manticoresoftware/manticoresearch/issues/3195) 改进了[自动磁盘块刷新](Securing_and_compacting_a_table/Flushing_RAM_chunk_to_disk.md#Flushing-RAM-chunk-to-disk)：当正在运行优化时跳过保存
* 🪲 [v9.5.10](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.10) [ Issue #3313](https://github.com/manticoresoftware/manticoresearch/issues/3313) 修复了使用[indextool](Miscellaneous_tools.md#indextool)检查RT表中所有磁盘块重复ID的问题
* 🪲 [v9.5.9](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.9) [ Issue #3132](https://github.com/manticoresoftware/manticoresearch/issues/3132) 在JSON API中增加了对`_random`排序的支持
* 🪲 [v9.5.8](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.8) [ Issue #3382](https://github.com/manticoresoftware/manticoresearch/issues/3382) 修复了JSON HTTP API中无法使用uint64文档ID的问题
* 🪲 [v9.5.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.7) [ Issue #3385](https://github.com/manticoresoftware/manticoresearch/issues/3385) 修正了按`id != value`过滤时返回结果不正确的问题
* 🪲 [v9.5.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.6) [ PR #538](https://github.com/manticoresoftware/manticoresearch-buddy/pull/538) 修复了在某些情况下模糊匹配的关键错误
* 🪲 [v9.5.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.5) [ Issue #3199](https://github.com/manticoresoftware/manticoresearch/issues/3199) 修复了Buddy HTTP查询参数中空格解码的问题（例如 `%20` 和 `+`）
* 🪲 [v9.5.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.4) [ Issue #3133](https://github.com/manticoresoftware/manticoresearch/issues/3133) 修复了facet搜索中`json.field`排序错误的问题
* 🪲 [v9.5.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.3) [ Issue #3091](https://github.com/manticoresoftware/manticoresearch/issues/3091) 修复了SQL和JSON API中定界符搜索结果不一致的问题
* 🪲 [v9.5.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.2) [ Issue #2819](https://github.com/manticoresoftware/manticoresearch/issues/2819) 性能改进：将分布式表中的`DELETE FROM`替换为`TRUNCATE`
* 🪲 [v9.5.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.1) [ Issue #3080](https://github.com/manticoresoftware/manticoresearch/issues/3080) 修复了使用JSON属性过滤别名`geodist()`时导致的崩溃

## 版本 9.3.2
发布日期：2025年5月2日

此次发布包含多项错误修复和稳定性改进，更好的表使用跟踪，以及内存和资源管理的增强。

❤️ 特别感谢 [@cho-m](https://github.com/cho-m) 修复了与Boost 1.88.0的构建兼容性问题，以及 [@benwills](https://github.com/benwills) 改进了关于`stored_only_fields`的文档。

* 🪲 [v9.3.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.3.2)  修复了“显示线程”列显示为浮点数而非字符串的CPU活动问题；同时修正了PyMySQL结果集解析错误，原因是数据类型错误。
* 🪲 [v9.3.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.3.1) [ Issue #3343](https://github.com/manticoresoftware/manticoresearch/issues/3343) 修复了优化过程被中断时遗留的`tmp.spidx`文件问题。
* 🆕 [v9.3.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.3.0) [ PR #3337](https://github.com/manticoresoftware/manticoresearch/pull/3337) 新增了每表命令计数器和详细的表使用统计。
* 🪲 [v9.2.39](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.39) [ Issue #3236](https://github.com/manticoresoftware/manticoresearch/issues/3236) 修复：通过移除复杂块更新防止表损坏。串行工作线程中使用等待函数破坏了串行处理，可能导致表损坏。
	重新实现了自动刷新。移除外部轮询队列以避免不必要的表锁。新增“小表”条件：当文档数量低于“小表限制”（8192）且未使用二级索引（SI）时，跳过刷新。

* 🪲 [v9.2.38](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.38)  修复：跳过为使用`ALL`/`ANY`字符串列表过滤器创建二级索引（SI），不影响JSON属性。
* 🪲 [v9.2.37](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.37) [ Issue #2898](https://github.com/manticoresoftware/manticoresearch/issues/2898) 为系统表添加了反引号支持。
* 🪲 [v9.2.36](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.36)  修复：在旧代码中使用占位符进行集群操作。在解析器中，我们现在清楚地分开了表名和集群名字段。
* 🪲 [v9.2.35](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.35)  修复：取消引用单个`'`时报错导致崩溃。
* 🪲 [v9.2.34](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.34) [ Issue #3090](https://github.com/manticoresoftware/manticoresearch/issues/3090) 修复：大文档ID的处理（之前可能会找不到）。
* 🪲 [v9.2.33](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.33)  修复：位向量大小使用无符号整数。
* 🪲 [v9.2.32](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.32)  修复：减少合并时的峰值内存使用。docid到rowid的查找现在使用每文档12字节，代替之前的16字节。举例：20亿文档只需24 GB RAM，而不是36 GB。
* 🪲 [v9.2.31](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.31) [ Issue #3238](https://github.com/manticoresoftware/manticoresearch/issues/3238) 修复：大型实时表中 `COUNT(*)` 值不正确。
* 🪲 [v9.2.30](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.30)  修复：归零字符串属性时的未定义行为。
* 🪲 [v9.2.29](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.29)  小修正：改进了警告文本。
* 🪲 [v9.2.28](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.28) [ Issue #3290](https://github.com/manticoresoftware/manticoresearch/issues/3290) 改进：增强了 `indextool --buildidf`
* 🪲 [v9.2.27](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.27) [ Issue #3032](https://github.com/manticoresoftware/manticoresearch/issues/3032) 通过 Kafka 集成，现在可以为特定 Kafka 分区创建源。
* 🪲 [v9.2.26](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.26) [ Issue #3301](https://github.com/manticoresoftware/manticoresearch/issues/3301) 修复：`ORDER BY` 和 `WHERE` 对 `id` 可能导致 OOM（内存溢出）错误。
* 🪲 [v9.2.25](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.25) [ Issue #3171](https://github.com/manticoresoftware/manticoresearch/issues/3171) 修复：当 RT 表具有多个磁盘块时，使用多个 JSON 属性的 grouper 导致的段错误崩溃。
* 🪲 [v9.2.24](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.24) [ Issue #3246](https://github.com/manticoresoftware/manticoresearch/issues/3246) 修复：RAM 块刷新后 `WHERE string ANY(...)` 查询失败。
* 🪲 [v9.2.23](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.23) [ PR #518](https://github.com/manticoresoftware/manticoresearch-buddy/pull/518) 小的自动分片语法改进。
* 🪲 [v9.2.22](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.22) [ Issue #2763](https://github.com/manticoresoftware/manticoresearch/issues/2763) 修复：使用 `ALTER TABLE` 时未加载全局 idf 文件。
* 🪲 [v9.2.21](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.21)  修复：全局 idf 文件可能很大。现在我们更早释放表以避免占用不必要的资源。
* 🪲 [v9.2.20](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.20) [ PR #3277](https://github.com/manticoresoftware/manticoresearch/pull/3277) 改进：更好的分片选项验证。

* 🪲 [v9.2.19](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.19) [ PR #3275](https://github.com/manticoresoftware/manticoresearch/pull/3275) 修复：与 Boost 1.88.0 的构建兼容性。
* 🪲 [v9.2.18](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.18) [ Issue #3228](https://github.com/manticoresoftware/manticoresearch/issues/3228) 修复：分布式表创建时崩溃（无效指针问题）。

* 🪲 [v9.2.17](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.17) [ PR #3272](https://github.com/manticoresoftware/manticoresearch/pull/3272) 修复：多行模糊 `FACET` 问题。
* 🪲 [v9.2.16](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.16) [ Issue #3063](https://github.com/manticoresoftware/manticoresearch/issues/3063) 修复：使用 `GEODIST` 函数时距离计算的错误。
* 🪲 [v9.2.15](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.15) [ Issue #3027](https://github.com/manticoresoftware/manticoresearch/issues/3027) 小改进：支持 Elastic 的 `query_string` 过滤器格式。

## Version 9.2.14
Released: March 28th 2025

### Minor changes
* [Commit ](https://github.com/manticoresoftware/manticoresearch/commit/fe9473dc3f4fcfa0eaae748b538fe853f4ce078b) 实现了 `--mockstack` 标志，用于计算递归操作的栈需求。新的 `--mockstack` 模式分析并报告递归表达式求值、模式匹配操作、过滤器处理所需的栈大小。计算出的栈需求将在控制台输出，便于调试和优化。
* [Issue #3058](https://github.com/manticoresoftware/manticoresearch/pull/3058) 默认启用 [boolean_simplify](Searching/Options.md#boolean_simplify)。
* [Issue #3172](https://github.com/manticoresoftware/manticoresearch/issues/3172) 新增配置选项：`searchd.kibana_version_string`，当 Manticore 与特定版本的 Kibana 或 OpenSearch Dashboards 配合使用且需要特定 Elasticsearch 版本时，该选项非常有用。
* [Issue #3211](https://github.com/manticoresoftware/manticoresearch/issues/3211) 修正 [CALL SUGGEST](Searching/Spell_correction.md#CALL-QSUGGEST,-CALL-SUGGEST) 对两个字符单词的支持。
* [Issue #490](https://github.com/manticoresoftware/manticoresearch-buddy/issues/490) 改进了[模糊搜索](Searching/Spell_correction.md#Fuzzy-Search)：此前在存在其他匹配文档时，有时无法找到搜索词 “def ghi” 中的 “defghi”。
* ⚠️ BREAKING [Issue #3165](https://github.com/manticoresoftware/manticoresearch/issues/3165) 一些 HTTP JSON 响应中将 `_id` 改为 `id`，以保持一致性。请确保相应更新您的应用程序。
* ⚠️ 重大变更 [Issue #3186](https://github.com/manticoresoftware/manticoresearch/issues/3186) 在集群加入时添加了 `server_id` 检查，以确保每个节点都有唯一的 ID。现在，当加入节点的 `server_id` 与集群中已有节点相同时，`JOIN CLUSTER` 操作可能会失败并显示重复的 [server_id](Server_settings/Searchd.md#server_id) 错误信息。为解决此问题，请确保复制集群中的每个节点具有唯一的 [server_id](Server_settings/Searchd.md#server_id)。您可以在尝试加入集群之前，在配置文件的 "searchd" 部分将默认的 [server_id](Server_settings/Searchd.md#server_id) 更改为唯一值。此更改更新了复制协议。如果您正在运行复制集群，需要：
  - 首先，干净地停止所有节点
  - 然后，使用 Linux 下的 `manticore_new_cluster` 工具，带上 `--new-cluster` 启动最后停止的节点。
  - 有关更多详情，请阅读[重启集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster)。

### Bug修复
* [Commit 6fda](https://github.com/manticoresoftware/manticoresearch/commit/6fdad3923dd6953f4b781943eed5ec5c28b7f808) 修复了等待后调度器丢失导致的崩溃；现在，特定调度器如 `serializer` 能够正确恢复。
* [Commit c333](https://github.com/manticoresoftware/manticoresearch/commit/c333b277d4a504de5fe298e9ab570a9f9ea31e0a) 修复了右表连接中的权重无法在 `ORDER BY` 子句中使用的错误。
* [Issue #2644](https://github.com/manticoresoftware/manticoresearch/issues/2644) gcc 14.2.0：修正了 `lower_bound` 调用错误，针对 `const knn::DocDist_t*&`。❤️ 感谢 [@Azq2](https://github.com/Azq2) 提交的 PR。
* [Issue #3018](https://github.com/manticoresoftware/manticoresearch/issues/3018) 修复了自动创建模式插入时大写表名处理问题。
* [Issue #3119](https://github.com/manticoresoftware/manticoresearch/issues/3119) 修复了解码无效 base64 输入时崩溃的问题。
* [Issue #3121](https://github.com/manticoresoftware/manticoresearch/issues/3121) 修复了两个与 KNN 索引 ALTER 操作相关的问题：浮点向量现在保持其原始维度，KNN 索引生成得以正常进行。
* [Issue #3123](https://github.com/manticoresoftware/manticoresearch/issues/3123) 修复了在空 JSON 列上构建二级索引时崩溃的问题。
* [Issue #3138](https://github.com/manticoresoftware/manticoresearch/issues/3138) 修复了由于重复条目导致的崩溃。
* [Issue #3151](https://github.com/manticoresoftware/manticoresearch/issues/3151) 修复了 `fuzzy=1` 选项不能与 `ranker` 或 `field_weights` 一起使用的问题。
* [Issue #3163](https://github.com/manticoresoftware/manticoresearch/issues/3163) 修复了 `SET GLOBAL timezone` 无效的错误。
* [Issue #3181](https://github.com/manticoresoftware/manticoresearch/issues/3181) 修复了使用超过 2^63 的 ID 时文本字段值可能丢失的问题。
* [Issue #3189](https://github.com/manticoresoftware/manticoresearch/issues/3189) 修复了 `UPDATE` 语句现在正确遵守 `query_log_min_msec` 设置的问题。
* [Issue #3247](https://github.com/manticoresoftware/manticoresearch/issues/3247) 修复了保存实时磁盘块时的竞态条件，避免了导致 `JOIN CLUSTER` 失败的问题。


## 版本 7.4.6
发布：2025年2月28日

### 重大更改
* [Issue #832](https://github.com/manticoresoftware/manticoresearch/issues/832) 集成了 [Kibana](Integration/Kibana.md)，以实现更简便高效的数据可视化。

### 次要更改
* [Issue #1727](https://github.com/manticoresoftware/manticoresearch/issues/1727) 修正了 arm64 和 x86_64 之间的浮点精度差异。
* [Issue #2995](https://github.com/manticoresoftware/manticoresearch/issues/2995) 实现了连接批处理的性能优化。
* [Issue #3039](https://github.com/manticoresoftware/manticoresearch/issues/3039) 实现了直方图中 EstimateValues 的性能优化。
* [Issue #3099](https://github.com/manticoresoftware/manticoresearch/issues/3099) 添加了对 Boost 1.87.0 的支持。❤️ 感谢 [@cho-m](https://github.com/cho-m) 的 PR。
* [Issue #77](https://github.com/manticoresoftware/columnar/issues/77) 优化了创建多值过滤器时的块数据重用；增加了属性元数据中的最小/最大值；实现了基于最小/最大值的过滤值预过滤。

### Bug修复
* [Commit 73ac](https://github.com/manticoresoftware/manticoresearch/commit/73ac22f358a7a0e734b332c3943c86e6294c06c3) 修复了在使用左右表属性的连接查询中表达式的处理问题；修正了右表的 index_weights 选项。
* [Issue #2915](https://github.com/manticoresoftware/manticoresearch/issues/2915) 在 `SELECT ... JOIN` 查询中使用 `avg()` 可能导致结果不正确；现在已修复。
* [Issue #2996](https://github.com/manticoresoftware/manticoresearch/issues/2996) 修复了启用连接批处理时隐式截止导致错误结果集的问题。
* [Issue #3031](https://github.com/manticoresoftware/manticoresearch/issues/3031) 修复了守护进程在活动块合并期间关闭时崩溃的问题。
* [Issue #3037](http://github.com/manticoresoftware/manticoresearch/issues/3037) 修复了 `IN(...)` 可能产生错误结果的问题。
* [Issue #3038](https://github.com/manticoresoftware/manticoresearch/issues/3038) 版本 7.0.0 中设置 `max_iops` / `max_iosize` 会降低索引性能；现已修复。
* [Issue #3042](https://github.com/manticoresoftware/manticoresearch/issues/3042) 修复了连接查询缓存中的内存泄漏。
* [Issue #3052](https://github.com/manticoresoftware/manticoresearch/issues/3052) 修复了连接 JSON 查询中查询选项的处理。
* [Issue #3054](https://github.com/manticoresoftware/manticoresearch/issues/3054) 修复了 ATTACH TABLE 命令的问题。
* [Issue #3079](https://github.com/manticoresoftware/manticoresearch/issues/3079) 修正了错误信息中的不一致问题。
* [Issue #3087](https://github.com/manticoresoftware/manticoresearch/issues/3087) 针对每个表设置 `diskchunk_flush_write_timeout=-1` 未能禁用索引刷新的问题已修复。
* [Issue #3088](https://github.com/manticoresoftware/manticoresearch/issues/3088) 解决了批量替换大ID后出现的重复条目问题。
* [Issue #3126](https://github.com/manticoresoftware/manticoresearch/issues/3126) 修复了因带有单个 `NOT` 运算符和表达式排序器的全文查询导致守护进程崩溃的问题。
* [Issue #3128](https://github.com/manticoresoftware/manticoresearch/pull/3128) 修复了 CJSON 库中的潜在漏洞。❤️ 感谢 [@tabudz](https://github.com/tabudz) 提交PR。

## Version 7.0.0
发布于：2025年1月30日

### 重大更新
* [Issue #1497](https://github.com/manticoresoftware/manticoresearch/issues/1497) 新增了便于搜索的[模糊搜索](Searching/Spell_correction.md#Fuzzy-Search)和[自动补全](Searching/Autocomplete.md#CALL-AUTOCOMPLETE)功能。
* [Issue #1500](https://github.com/manticoresoftware/manticoresearch/issues/1500) 实现了与 Kafka 的[集成](Integration/Kafka.md#Syncing-from-Kafka)。
* [Issue #1928](https://github.com/manticoresoftware/manticoresearch/issues/1928) 引入了[JSON的二级索引](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#json_secondary_indexes)。
* [Issue #2361](https://github.com/manticoresoftware/manticoresearch/issues/2361) 更新及更新过程中搜索不再被块合并阻塞。
* [Issue #2787](https://github.com/manticoresoftware/manticoresearch/issues/2787) 对RT表实现了自动[磁盘chunk刷新](Server_settings/Searchd.md#diskchunk_flush_write_timeout)以提升性能；现在我们会自动将内存中的chunk刷新到磁盘chunk，防止由于内存chunk缺乏优化引起的性能问题，这在某些chunk大小情况下可能导致不稳定。
* [Issue #2811](https://github.com/manticoresoftware/manticoresearch/issues/2811) 新增了[滚动分页](Searching/Pagination.md#Scroll-Search-Option)选项，更方便分页。
* [Issue #931](https://github.com/manticoresoftware/manticoresearch/issues/931) 集成了[Jieba](https://github.com/fxsjy/jieba)，提升了[中文分词](Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md)效果。

### 次要更新
* ⚠️ 重大变更 [Issue #1111](https://github.com/manticoresoftware/manticoresearch/issues/1111) 修复了 RT 表中对 `global_idf` 的支持。需要重新创建表。
* ⚠️ 重大变更 [Issue #2103](https://github.com/manticoresoftware/manticoresearch/issues/2103) 内部 `cjk` 字符集不再包含泰文字符。请相应更新您的字符集定义：如果您使用了 `cjk,non_cjk` 且泰文字符对您很重要，请修改为 `cjk,thai,non_cjk`，或者使用 `cont,non_cjk`，其中 `cont` 是所有连续脚本语言（即 `cjk` + `thai`）的新标识。请使用 [ALTER TABLE](Updating_table_schema_and_settings.md#Updating-table-FT-settings-in-RT-mode) 修改现有表。
* ⚠️ 重大变更 [Issue #2468](https://github.com/manticoresoftware/manticoresearch/issues/2468) [CALL SUGGEST / QSUGGEST](Searching/Spell_correction.md#CALL-QSUGGEST,-CALL-SUGGEST) 现在兼容分布式表。此更新提升了主从协议版本。如果您在分布式环境中运行 Manticore Search，确保先升级 agents，再升级 masters。
* ⚠️ 重大变更 [Issue #2889](https://github.com/manticoresoftware/manticoresearch/issues/2889) PQ [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META-for-PQ-tables) 中列名从 `Name` 改为 `Variable name`。
* ⚠️ 重大变更 [Issue #879](https://github.com/manticoresoftware/manticoresearch/issues/879) 引入了[每表二进制日志](Logging/Binary_logging.md#Per-table-binary-logging-configuration)新选项：[binlog_common](Logging/Binary_logging.md#Binary-logging-strategies)、[binlog](Logging/Binary_logging.md#Per-table-binary-logging-configuration)，支持 `create table` / `alter table`。升级前需要干净关闭 Manticore 实例。
* ⚠️ 重大变更 [Issue #1789](https://github.com/manticoresoftware/manticoresearch/issues/1789) 修正了节点以错误复制协议版本加入集群时的错误提示。此变更涉及复制协议更新。如果您运行复制集群，请：
  - 首先，干净关闭所有节点
  - 然后，使用 Linux 上的工具 `manticore_new_cluster` 启动最后停止的节点，并传入 `--new-cluster` 参数。
  - 详情请参阅[重启集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster)。
* ⚠️ 重大变更 [Issue #2308](https://github.com/manticoresoftware/manticoresearch/issues/2308) 在 [`ALTER CLUSTER ADD` 和 `DROP`](Creating_a_cluster/Setting_up_replication/Adding_and_removing_a_table_from_a_replication_cluster.md#Adding-and-removing-a-table-from-a-replication-cluster) 中增加了对多个表的支持。此变更也影响复制协议。请参阅上一节了解更新操作指南。
* [Issue #2997](https://github.com/manticoresoftware/manticoresearch/issues/2997) 修复了 Macos 上 dlopen 问题。
* [Commit 4954](https://github.com/manticoresoftware/manticoresearch/commit/4954b5de7341a29902a9b8fbb9a040f7942c77c4) 修改了带有 KNN 索引表的 OPTIMIZE TABLE 默认截止阈值以提升搜索性能。
* [Commit cfc8](https://github.com/manticoresoftware/manticoresearch/commit/cfc87ecb6e33a8163c2235243b6b40e699dbf526) 增加了 `FACET` 和 `GROUP BY` 中 `ORDER BY` 对 `COUNT(DISTINCT)` 的支持。
* [Issue #1103](https://github.com/manticoresoftware/manticoresearch/issues/1103) 改善了[日志](Logging/Server_logging.md#Server-logging)中 chunk 合并的清晰度。
* [Issue #1130](https://github.com/manticoresoftware/manticoresearch/issues/1130) 添加了对 [DBeaver](Integration/DBeaver.md) 的支持。
* [Issue #1546](https://github.com/manticoresoftware/manticoresearch/issues/1546) 为 [POLY2D()](Functions/Geo_spatial_functions.md#POLY2D%28%29)/[GEOPOLY2D()](Functions/Geo_spatial_functions.md#GEOPOLY2D%28%29) 函数实现了二级索引。
* [Issue #1630](https://github.com/manticoresoftware/manticoresearch/issues/1630) HTTP 请求现在支持 `Content-Encoding: gzip`。
* [Issue #1831](https://github.com/manticoresoftware/manticoresearch/issues/1831) 添加了 `SHOW LOCKS` 命令。
* [Issue #2187](https://github.com/manticoresoftware/manticoresearch/issues/2197) 允许 Buddy 请求绕过 [searchd.max_connections](Server_settings/Searchd.md#max_connections) 限制。
* [Issue #2208](https://github.com/manticoresoftware/manticoresearch/issues/2208) 添加了通过 JSON HTTP 接口连接表的支持。
* [Issue #2235](https://github.com/manticoresoftware/manticoresearch/issues/2235) 通过 Buddy 成功处理的查询记录以原始形式记录日志。
* [Issue #2249](https://github.com/manticoresoftware/manticoresearch/issues/2249) 添加了针对复制表运行 `mysqldump` 的特殊模式。
* [Issue #2268](https://github.com/manticoresoftware/manticoresearch/issues/2268) 改进了 `CREATE TABLE` 和 `ALTER TABLE` 语句中外部文件的重命名复制操作。
* [Issue #2402](https://github.com/manticoresoftware/manticoresearch/issues/2402) 更新了 [searchd.max_packet_size](Server_settings/Searchd.md#max_packet_size) 的默认值为128MB。
* [Issue #2419](https://github.com/manticoresoftware/manticoresearch/issues/2419) 添加了 JSON ["match"](Searching/Full_text_matching/Basic_usage.md#match) 中的 [IDF boost modifier](Searching/Full_text_matching/Operators.md#IDF-boost-modifier) 支持。
* [Issue #2430](https://github.com/manticoresoftware/manticoresearch/issues/2430) 加强了 [binlog](Logging/Binary_logging.md#Binary-logging) 写入同步，防止错误发生。
* [Issue #2458](https://github.com/manticoresoftware/manticoresearch/issues/2458) Windows 软件包中加入了 zlib 支持。
* [Issue #2479](https://github.com/manticoresoftware/manticoresearch/issues/2479) 添加了对 SHOW TABLE INDEXES 命令的支持。
* [Issue #2485](https://github.com/manticoresoftware/manticoresearch/issues/2485) 设置了 Buddy 响应的会话元数据。
* [Issue #2490](https://github.com/manticoresoftware/manticoresearch/issues/2490) 兼容端点的聚合支持毫秒级分辨率。
* [Issue #2500](https://github.com/manticoresoftware/manticoresearch/issues/2500) 更改了集群操作中复制启动失败时的错误信息。
* [Issue #2584](https://github.com/manticoresoftware/manticoresearch/pull/2584) 在 SHOW STATUS 中新增了 [性能指标](Node_info_and_management/Node_status.md#Query-Time-Statistics)：最近1、5和15分钟内各类型查询的最小值/最大值/平均值/95分位/99分位。
* [Issue #2639](https://github.com/manticoresoftware/manticoresearch/pull/2639) 请求和响应中的所有 `index` 实例均替换为 `table`。
* [Issue #2643](https://github.com/manticoresoftware/manticoresearch/issues/2643) 在 HTTP `/sql` 端点的聚合结果中添加了 `distinct` 列。
* [Issue #268](https://github.com/manticoresoftware/manticoresearch-buddy/issues/268) 实现了从 Elasticsearch 导入数据类型的自动检测。
* [Issue #2744](https://github.com/manticoresoftware/manticoresearch/issues/2744) 为字符串 JSON 字段比较表达式添加了排序规则支持。
* [Issue #2752](https://github.com/manticoresoftware/manticoresearch/issues/2752) 增加了选择列表中对 `uuid_short` 表达式的支持。
* [Issue #2783](https://github.com/manticoresoftware/manticoresearch/issues/2783) Manticore Search 现在直接运行 Buddy，不再使用 `manticore-buddy` 代理。
* [Issue #2785](https://github.com/manticoresoftware/manticoresearch/issues/2785) 针对缺少表与不支持插入操作的表，错误信息进行了区分。
* [Issue #2789](https://github.com/manticoresoftware/manticoresearch/issues/2789) OpenSSL 3 现在静态编译进了 `searchd`。
* [Issue #2790](https://github.com/manticoresoftware/manticoresearch/issues/2790) 添加了 `CALL uuid_short` 语句，可生成包含多个 `uuid_short` 值的序列。
* [Issue #2803](https://github.com/manticoresoftware/manticoresearch/issues/2803) 添加了 JOIN 操作中右侧表的独立选项。
* [Issue #2810](https://github.com/manticoresoftware/manticoresearch/issues/2810) 改进了 HTTP JSON 聚合性能，使其达到 SphinxQL 中 `GROUP BY` 的性能水平。
* [Issue #2854](https://github.com/manticoresoftware/manticoresearch/issues/2854) 在 Kibana 日期相关请求中添加了对 `fixed_interval` 的支持。
* [Issue #2909](https://github.com/manticoresoftware/manticoresearch/issues/2909) 实现了 JOIN 查询的批处理，大幅提升了某些 JOIN 查询的性能，提升幅度达数百甚至数千倍。
* [Issue #2937](https://github.com/manticoresoftware/manticoresearch/issues/2937) 允许在全扫描查询中使用连接表的权重。
* [Issue #2953](https://github.com/manticoresoftware/manticoresearch/issues/2953) 修复了连接查询的日志记录。
* [Issue #337](https://github.com/manticoresoftware/manticoresearch-buddy/issues/337) 在非调试模式下隐藏了 Buddy 异常，不再在 `searchd` 日志中显示。
* [Issue #2931](https://github.com/manticoresoftware/manticoresearch/issues/2931) 若用户为复制监听器设置错误端口，守护进程关闭时会显示错误信息。

### Bug 修复
* [Commit 0c6b](https://github.com/manticoresoftware/manticoresearch/commit/0c6bdaf41ea3684e05952e1a8273893535b923f5) 修复：包含超过32列的 JOIN 查询返回了错误结果的问题。
* [Issue #2335](https://github.com/manticoresoftware/manticoresearch/issues/2335) 解决了在条件中使用两个 JSON 属性时表连接失败的问题。
* [Issue #2338](https://github.com/manticoresoftware/manticoresearch/issues/2338) 修正了多查询中使用[cutoff](Searching/Options.md#cutoff)时total_relation计算错误的问题。
* [Issue #2366](https://github.com/manticoresoftware/manticoresearch/issues/2366) 修正了在[表连接](Searching/Joining.md)中右表对`json.string`的过滤。
* [Issue #2406](https://github.com/manticoresoftware/manticoresearch/issues/2406) 允许在任何POST HTTP JSON端点（插入/替换/批量）中为所有值使用`null`，此时使用默认值。
* [Issue #2418](https://github.com/manticoresoftware/manticoresearch/issues/2418) 通过调整初始socket探测时[最大包大小](Server_settings/Searchd.md#max_packet_size)网络缓冲的分配，优化了内存消耗。
* [Issue #2420](https://github.com/manticoresoftware/manticoresearch/issues/2420) 修复了通过JSON接口向bigint属性插入无符号整型的问题。
* [Issue #2422](https://github.com/manticoresoftware/manticoresearch/issues/2422) 修正了启用pseudo_sharding时二级索引对排除过滤器的正确支持。
* [Issue #2423](https://github.com/manticoresoftware/manticoresearch/issues/2423) 修复了[manticore_new_cluster](Starting_the_server/Manually.md#searchd-command-line-options)中的一个 bug。
* [Issue #2448](https://github.com/manticoresoftware/manticoresearch/issues/2448) 解决了格式错误的`_update`请求导致守护进程崩溃的问题。
* [Issue #2452](https://github.com/manticoresoftware/manticoresearch/issues/2452) 修复了直方图无法处理带排除项的值过滤器的问题。
* [Issue #55](https://github.com/manticoresoftware/columnar/issues/55) 修复了针对分布式表的[knn](Searching/KNN.md#KNN-vector-search)查询。
* [Issue #68](https://github.com/manticoresoftware/columnar/issues/68) 增强了列式访问器中对表编码上的排除过滤的处理。
* [Commit 0eb1](https://github.com/manticoresoftware/manticoresearch/commit/0eb1579462013cb134bcaf74e06f6eeb0c2ecc4d) 修正了表达式解析器未遵守重新定义的`thread_stack`的问题。
* [Commit c304](https://github.com/manticoresoftware/manticoresearch/commit/c3041fd1d3e14773f3c5aa635343915a71f5cb95) 修复了克隆列式IN表达式时的崩溃。
* [Commit edad](https://github.com/manticoresoftware/columnar/commit/edadc694c68d6022bdd13134263667430a42cc1d) 修复了位图迭代器的反转问题导致的崩溃。
* [Commit fc30](https://github.com/manticoresoftware/executor/commit/fc302ff1117b0b835a6f5a3c07057baf5fba14d9) 修正了一些Manticore软件包被`unattended-upgrades`自动移除的问题。
* [Issue #1019](https://github.com/manticoresoftware/manticoresearch/issues/1019) 改进了对DbForge MySQL工具查询的处理。
* [Issue #1107](https://github.com/manticoresoftware/manticoresearch/issues/1107) 修复了`CREATE TABLE`和`ALTER TABLE`中特殊字符的转义。❤️ 感谢[@subnix](https://github.com/subnix)的PR。
* [Issue #116](https://github.com/manticoresoftware/manticoresearch-backup/issues/116#issuecomment-2216597206) 修复了在冻结索引中更新blob属性时出现的死锁。该死锁是因尝试解冻索引时锁冲突导致，也可能导致manticore-backup失败。
* [Issue #1818](https://github.com/manticoresoftware/manticoresearch/issues/1818) 当表冻结时，`OPTIMIZE`命令现在会抛出错误。
* [Issue #2001](https://github.com/manticoresoftware/manticoresearch/issues/2001) 允许使用函数名作为列名。
* [Issue #2153](https://github.com/manticoresoftware/manticoresearch/issues/2153) 修复了查询包含未知磁盘块的表设置时守护进程崩溃的问题。
* [Issue #2184](https://github.com/manticoresoftware/manticoresearch/issues/2184) 修正了`FREEZE`和`FLUSH RAMCHUNK`后`searchd`停止时挂起的问题。
* [Issue #2228](https://github.com/manticoresoftware/manticoresearch/issues/2228) 从保留字中移除了日期/时间相关的标记（和正则表达式）。
* [Issue #2255](https://github.com/manticoresoftware/manticoresearch/issues/2255) 修复了`FACET`排序字段超过5个时的崩溃。
* [Issue #2265](https://github.com/manticoresoftware/manticoresearch/issues/2265) 修正了启用`index_field_lengths`时`mysqldump`恢复失败的问题。
* [Issue #2291](https://github.com/manticoresoftware/manticoresearch/issues/2291) 修复了执行`ALTER TABLE`命令时崩溃的问题。
* [Issue #2333](https://github.com/manticoresoftware/manticoresearch/issues/2333) 修复了Windows包中MySQL DLL，确保索引器正常工作。
* [Issue #2393](https://github.com/manticoresoftware/manticoresearch/issues/2393) 修正了GCC编译错误。❤️ 感谢[@animetosho](https://github.com/animetosho)的PR。
* [Issue #2447](https://github.com/manticoresoftware/manticoresearch/issues/2447) 修复了[_update](Data_creation_and_modification/Updating_documents/REPLACE.md?client=Elasticsearch-like+partial)中的转义问题。
* [Issue #2460](https://github.com/manticoresoftware/manticoresearch/issues/2460) 修正了声明多个同名属性或字段时索引器崩溃的问题。
* [Issue #2467](https://github.com/manticoresoftware/manticoresearch/issues/2467) 解决了针对“compat”搜索相关端点的嵌套布尔查询错误转换导致守护进程崩溃的问题。
* [Issue #2493](https://github.com/manticoresoftware/manticoresearch/issues/2493) 修正了带修饰符短语中的扩展问题。
* [Issue #2535](https://github.com/manticoresoftware/manticoresearch/issues/2535) 解决了使用[ZONE](Searching/Full_text_matching/Operators.md#ZONE-limit-operator)或[ZONESPAN](Searching/Full_text_matching/Operators.md#ZONESPAN-limit-operator)全文操作符时守护进程崩溃的问题。
* [Issue #2552](https://github.com/manticoresoftware/manticoresearch/issues/2552) 修正了带关键词字典的普通表和实时表的中缀生成。
* [问题 #2571](https://github.com/manticoresoftware/manticoresearch/issues/2571) 修复了 `FACET` 查询中的错误回复；将带有 `COUNT(*)` 的 `FACET` 默认排序顺序设置为 `DESC`。
* [问题 #2580](https://github.com/manticoresoftware/manticoresearch/issues/2580) 修复了 Windows 启动期间守护进程崩溃的问题。
* [问题 #2603](https://github.com/manticoresoftware/manticoresearch/issues/2603) 修复了 HTTP 端点 `/sql` 和 `/sql?mode=raw` 的查询截断问题；使这些端点的请求一致，无需 `query=` 头。
* [问题 #2623](https://github.com/manticoresoftware/manticoresearch/issues/2623) 修复了自动模式创建表同时失败的问题。
* [问题 #2627](https://github.com/manticoresoftware/manticoresearch/issues/2627) 修复了 HNSW 库以支持加载多个 KNN 索引。
* [问题 #2630](https://github.com/manticoresoftware/manticoresearch/issues/2630) 修复了多重条件同时发生时的冻结问题。
* [问题 #2645](https://github.com/manticoresoftware/manticoresearch/issues/2645) 修复了使用 `OR` 与 KNN 搜索时的致命错误崩溃。
* [问题 #2647](https://github.com/manticoresoftware/manticoresearch/issues/2647) 修复了 `indextool --mergeidf *.idf --out global.idf` 创建后删除输出文件的问题。
* [问题 #2658](https://github.com/manticoresoftware/manticoresearch/issues/2658) 修复了外层查询中带有 `ORDER BY` 字符串的子查询导致守护进程崩溃的问题。
* [问题 #2686](https://github.com/manticoresoftware/manticoresearch/issues/2686) 修复了在同时更新浮点属性和字符串属性时崩溃的问题。
* [问题 #2704](https://github.com/manticoresoftware/manticoresearch/issues/2704) 修复了 `lemmatize_xxx_all` 分词器中的多个停用词导致后续分词 `hitpos` 增加的问题。
* [问题 #2708](https://github.com/manticoresoftware/manticoresearch/issues/2708) 修复了 `ALTER ... ADD COLUMN ... TEXT` 时的崩溃。
* [问题 #2737](https://github.com/manticoresoftware/manticoresearch/issues/2737) 修复了在冻结表中更新 blob 属性且至少包含一个 RAM chunk 时，随后的 `SELECT` 查询需等待表解冻的问题。
* [问题 #2742](https://github.com/manticoresoftware/manticoresearch/issues/2742) 修复了带有打包因子的查询跳过查询缓存的问题。
* [问题 #2775](https://github.com/manticoresoftware/manticoresearch/issues/2775) Manticore 现对 `_bulk` 请求中的未知操作报错，而非崩溃。
* [问题 #2791](https://github.com/manticoresoftware/manticoresearch/issues/2791) 修复了 HTTP `_bulk` 端点返回插入文档 ID 的问题。
* [问题 #2797](https://github.com/manticoresoftware/manticoresearch/issues/2797) 修复了分组器在处理多个表时，一张为空且另一张的匹配条目数不同导致崩溃的问题。
* [问题 #2835](https://github.com/manticoresoftware/manticoresearch/issues/2835) 修复了复杂 `SELECT` 查询中的崩溃。
* [问题 #2872](https://github.com/manticoresoftware/manticoresearch/issues/2872) 添加了错误信息，当 `IN` 表达式中 `ALL` 或 `ANY` 参数不是 JSON 属性时提示。
* [问题 #2882](https://github.com/manticoresoftware/manticoresearch/issues/2882) 修复了在大表中更新 MVA 时的守护进程崩溃。
* [问题 #2888](https://github.com/manticoresoftware/manticoresearch/issues/2888) 修复了 `libstemmer` 分词失败时的崩溃。❤️ 感谢 [@subnix](https://github.com/subnix) 的 PR。
* [问题 #2919](https://github.com/manticoresoftware/manticoresearch/issues/2919) 修复了右表联结权重在表达式中未正常工作的情况。
* [问题 #2919](https://github.com/manticoresoftware/manticoresearch/issues/2919) 修复了右联结表权重在表达式中不工作的问题。
* [问题 #325](https://github.com/manticoresoftware/manticoresearch-buddy/issues/325) 修复了表已存在时 `CREATE TABLE IF NOT EXISTS ... WITH DATA` 命令失败的问题。
* [问题 #351](https://github.com/manticoresoftware/manticoresearch-buddy/issues/351) 修复了在使用文档 ID 的 KNN 计数时出现的未定义数组键 "id" 错误。
* [问题 #359](https://github.com/manticoresoftware/manticoresearch-buddy/issues/359) 修复了 `REPLACE INTO cluster_name:table_name` 功能。
* [问题 #67](https://github.com/manticoresoftware/docker/issues/67) 修复了使用 `--network=host` 运行 Manticore Docker 容器时的致命错误。

## 版本 6.3.8
发布日期：2024年11月22日

6.3.8 版本延续了 6.3 系列，仅包含错误修复。

### 错误修复

* [PR #2777](https://github.com/manticoresoftware/manticoresearch/pull/2777) 修复在 `threads` 或 `max_threads_per_query` 设置限制查询并发数时，可用线程数计算错误的问题。

## manticore-extra 版本 v1.1.20

发布日期：2024年10月7日

### 错误修复

* [问题 #64](https://github.com/manticoresoftware/executor/issues/64) 解决了 Debian 系统上自动安装包更新的 `unattended-upgrades` 工具错误地标记多个 Manticore 包（包含 `manticore-galera`、`manticore-executor` 和 `manticore-columnar-lib`）为待删除的虚拟包 `manticore-extra` 被误判为冗余的问题。已做出修改，确保 `unattended-upgrades` 不再尝试删除重要的 Manticore 组件。

## 版本 6.3.6
发布日期：2024年8月2日

6.3.6 版本延续了 6.3 系列，仅包含错误修复。

### 错误修复

* [问题 #2477](https://github.com/manticoresoftware/manticoresearch/issues/2477) 修复了 6.3.4 版本中引入的崩溃问题，可能在处理表达式及分布式或多表查询时发生。
* [问题 #2352](https://github.com/manticoresoftware/manticoresearch/issues/2352) 修复了多索引查询时因 `max_query_time` 引发的守护进程崩溃或内部错误。

## 版本 6.3.4
发布日期：2024年7月31日

6.3.4 版本延续了 6.3 系列，仅包含小改进和错误修复。

### 小变更
* [问题 #1130](https://github.com/manticoresoftware/manticoresearch/issues/1130) 增加了对 [DBeaver](Integration/DBeaver.md#Integration-with-DBeaver) 的支持。
* [问题 #2146](https://github.com/manticoresoftware/manticoresearch/issues/2146) 改进了词形和例外中分隔符的转义。
* [问题 #2315](https://github.com/manticoresoftware/manticoresearch/issues/2315) 在 SELECT 列表达式中添加了字符串比较运算符。
* [问题 #2363](https://github.com/manticoresoftware/manticoresearch/issues/2363) 增加了对类似 Elastic 批量请求中 null 值的支持。
* [问题 #2374](https://github.com/manticoresoftware/manticoresearch/issues/2374) 增加了对 mysqldump 版本 9 的支持。
* [问题 #2375](https://github.com/manticoresoftware/manticoresearch/issues/2375) 改进了 HTTP JSON 查询中的错误处理，增加了发生错误节点的 JSON 路径。

### 错误修复
* [问题 #2280](https://github.com/manticoresoftware/manticoresearch/issues/2280) 修复了当 disk_chunks > 1 时，通配符查询在匹配数量较多时的性能下降问题。
* [问题 #2332](https://github.com/manticoresoftware/manticoresearch/issues/2332) 修复了在空 MVA 数组上对 MVA MIN 或 MAX SELECT 列表达式导致的崩溃。
* [问题 #2339](https://github.com/manticoresoftware/manticoresearch/issues/2339) 修复了对 Kibana 无限范围请求的错误处理。
* [问题 #2342](https://github.com/manticoresoftware/manticoresearch/issues/2342) 修复了当属性不在 SELECT 列表中时，使用来自右表的列式属性的连接过滤器的问题。
* [问题 #2343](https://github.com/manticoresoftware/manticoresearch/issues/2343) 修复了 Manticore 6.3.2 中重复的 'static' 说明符。
* [问题 #2344](https://github.com/manticoresoftware/manticoresearch/issues/2344) 修复了使用 MATCH() 对右表进行查询时 LEFT JOIN 返回了不匹配条目的问题。
* [问题 #2350](https://github.com/manticoresoftware/manticoresearch/issues/2350) 修复了带 `hitless_words` 的 RT 索引磁盘块保存问题。
* [问题 #2364](https://github.com/manticoresoftware/manticoresearch/issues/2364) 现在 'aggs_node_sort' 属性可以在其他属性中任意顺序添加。
* [问题 #2368](https://github.com/manticoresoftware/manticoresearch/issues/2368) 修复了 JSON 查询中全文索引与过滤器顺序导致的错误。
* [问题 #2376](https://github.com/manticoresoftware/manticoresearch/issues/2376) 修复了针对长 UTF-8 请求产生错误 JSON 响应的 bug。
* [问题 #2684](https://github.com/manticoresoftware/dev/issues/2684) 修正了依赖连接属性的 presort/prefilter 表达式计算。
* [问题 #301](https://github.com/manticoresoftware/manticoresearch-buddy/issues/301) 更改了计算指标数据大小的方法，改为从 `manticore.json` 文件读取，而非检查数据目录的整体大小。
* [问题 #302](https://github.com/manticoresoftware/manticoresearch-buddy/issues/302) 增加了对 Vector.dev 验证请求的处理。

## 版本 6.3.2
发布：2024年6月26日

版本 6.3.2 继续 6.3 系列，并包含多个错误修复，其中一些是在 6.3.0 版本发布后发现的。

### 破坏性变更
* ⚠️[问题 #2305](https://github.com/manticoresoftware/manticoresearch/issues/2305) 更新了 aggs.range 的值为数字类型。

### 错误修复
* [提交 c51c](https://github.com/manticoresoftware/manticoresearch/commit/c51c7fa4bd0ec4ad8d53188b888461bff29f1417) 修复了存储检查与 rset 合并的分组问题。
* [提交 0e85](https://github.com/manticoresoftware/manticoresearch/commit/0e85b9d5682f51681750d798aef6a11cfb93d69e) 修复了当在 RT 索引中使用 CRC 字典和启用 `local_df` 时，带通配符字符查询导致守护进程崩溃的问题。
* [问题 #2200](https://github.com/manticoresoftware/manticoresearch/issues/2200) 修复了在无 GROUP BY 的 JOIN 上使用 `count(*)` 时的崩溃。
* [问题 #2201](https://github.com/manticoresoftware/manticoresearch/issues/2201) 修复了在尝试按全文字段分组时 JOIN 不返回警告的问题。
* [问题 #2230](https://github.com/manticoresoftware/manticoresearch/issues/2230) 解决了通过 `ALTER TABLE` 添加属性时未考虑 KNN 选项的问题。
* [问题 #2231](https://github.com/manticoresoftware/manticoresearch/issues/2231) 修复了 6.3.0 版本中无法移除 `manticore-tools` Redhat 包的问题。
* [问题 #2242](https://github.com/manticoresoftware/manticoresearch/issues/2242) 纠正了 JOIN 和多个 FACET 语句返回错误结果的问题。
* [问题 #2250](https://github.com/manticoresoftware/manticoresearch/issues/2250) 修复了当表属于集群时 ALTER TABLE 导致错误的问题。
* [问题 #2252](https://github.com/manticoresoftware/manticoresearch/issues/2252) 修复了原始查询未从 SphinxQL 接口传递到 buddy 的问题。
* [问题 #2267](https://github.com/manticoresoftware/manticoresearch/issues/2267) 改进了 RT 索引带磁盘块的 `CALL KEYWORDS` 中的通配符扩展。
* [问题 #271](https://github.com/manticoresoftware/manticoresearch-buddy/issues/271) 修复了错误的 `/cli` 请求导致的挂起问题。
* [问题 #274](https://github.com/manticoresoftware/manticoresearch-buddy/issues/274) 解决了对 Manticore 并发请求可能卡住的问题。
* [问题 #275](https://github.com/manticoresoftware/manticoresearch-buddy/issues/275) 修复了通过 `/cli` 执行 `drop table if exists t; create table t` 时挂起的问题。

### 复制相关
* [问题 #2270](https://github.com/manticoresoftware/manticoresearch/issues/2270) 在 `/_bulk` HTTP 端点中增加了对 `cluster:name` 格式的支持。

## 版本 6.3.0
发布：2024年5月23日

### 重大变更
* [问题 #839](https://github.com/manticoresoftware/manticoresearch/issues/839) 实现了 [float_vector](Creating_a_table/Data_types.md#Float-vector) 数据类型；实现了 [向量搜索](Searching/KNN.md#KNN-vector-search)。
* [问题 #1673](https://github.com/manticoresoftware/manticoresearch/issues/1673) 实现了 [INNER/LEFT JOIN](Searching/Joining.md)（**测试阶段**）。
* [Issue #1744](https://github.com/manticoresoftware/manticoresearch/issues/1744) 为 [timestamp](Creating_a_table/Data_types.md#Timestamps) 字段实现了日期格式的自动检测。
* [Issue #1720](https://github.com/manticoresoftware/manticoresearch/pull/1720) 将 Manticore Search 许可证从 GPLv2 或更高版本更改为 GPLv3 或更高版本。
* [Commit 7a55](https://github.com/manticoresoftware/manticoresearch/commit/7a5508424) Windows 上运行 Manticore 现在需要 Docker 来运行 Buddy。
* [Issue #1541](https://github.com/manticoresoftware/manticoresearch/issues/1541) 添加了一个 [REGEX](Searching/Full_text_matching/Operators.md#REGEX-operator) 全文操作符。
* [Issue #2091](https://github.com/manticoresoftware/manticoresearch/issues/2091) 支持 Ubuntu Noble 24.04。
* [Commit 514d](https://github.com/manticoresoftware/manticoresearch/commit/514d35b497f4bdb20b6473ff963752a978a4bb8d) 重新设计了时间操作以提升性能并新增日期/时间函数：
  - [CURDATE()](Functions/Date_and_time_functions.md#CURDATE%28%29) - 返回本地时区的当前日期
  - [QUARTER()](Functions/Date_and_time_functions.md#QUARTER%28%29) - 返回时间戳参数的年份季度整数
  - [DAYNAME()](Functions/Date_and_time_functions.md#DAYNAME%28%29) - 返回给定时间戳参数的星期名称
  - [MONTHNAME()](Functions/Date_and_time_functions.md#MONTHNAME%28%29) - 返回给定时间戳参数的月份名称
  - [DAYOFWEEK()](Functions/Date_and_time_functions.md#DAYOFWEEK%28%29) - 返回给定时间戳参数的星期整数索引
  - [DAYOFYEAR()](Functions/Date_and_time_functions.md#DAYOFYEAR%28%29) - 返回给定时间戳参数的年份中的第几天整数
  - [YEARWEEK()](Functions/Date_and_time_functions.md#YEARWEEK%28%29) - 返回给定时间戳参数的年份和当前周第一天的日码整数
  - [DATEDIFF()](Functions/Date_and_time_functions.md#DATEDIFF%28%29) - 返回两个给定时间戳之间的天数
  - [DATE()](Functions/Date_and_time_functions.md#DATE%28%29) - 格式化时间戳参数中的日期部分
  - [TIME()](Functions/Date_and_time_functions.md#TIME%28%29) - 格式化时间戳参数中的时间部分
  - [timezone](Server_settings/Searchd.md#timezone) - 日期/时间相关函数使用的时区。
* [Commit 30e7](https://github.com/manticoresoftware/manticoresearch/commit/30e789d9657f9b3093c280c6e5b0e46a021f6dc7) 向 HTTP 接口添加了 [range](Searching/Faceted_search.md#Facet-over-set-of-ranges)、[histogram](Searching/Faceted_search.md#Facet-over-histogram-values)、[date_range](Searching/Faceted_search.md#Facet-over-set-of-date-ranges) 和 [date_histogram](Searching/Faceted_search.md#Facet-over-histogram-date-values) 聚合，并在 SQL 中添加了相似表达式。

### 小改动
* [Issue #1285](https://github.com/manticoresoftware/manticoresearch/issues/1285) 支持 Filebeat 版本 8.10 - 8.11。
* [Issue #1771](https://github.com/manticoresoftware/manticoresearch/issues/1771) [ALTER TABLE ... type='distributed'](Updating_table_schema_and_settings.md#Changing-a-distributed-table)。
* [Issue #1788](https://github.com/manticoresoftware/manticoresearch/issues/1788) 通过 [CREATE TABLE ... LIKE ... WITH DATA](Creating_a_table/Local_tables/Real-time_table.md#CREATE-TABLE-LIKE:) SQL 语句添加了复制表的功能。
* [Issue #2072](https://github.com/manticoresoftware/manticoresearch/issues/2072) 优化了 [表压缩算法](Securing_and_compacting_a_table/Compacting_a_table.md#Compacting-a-Table)：之前，手动的 [OPTIMIZE](Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE) 和自动的 [auto_optimize](Server_settings/Searchd.md#auto_optimize) 过程都会先合并块以确保数量不超过限制，然后从所有包含已删除文档的其他块中删除已删除文档。此方法有时资源消耗过大，现已禁用。现在，块合并仅根据 [progressive_merge](Server_settings/Common.md#progressive_merge) 设置进行。具有大量已删除文档的块更有可能被合并。
* [Commit ce6c](https://github.com/manticoresoftware/columnar/commit/ce6c5d3381edcdc411a00a5923d13d56dd9b3010) 添加了对加载新版本二级索引的保护。
* [Issue #1417](https://github.com/manticoresoftware/manticoresearch/issues/1417) 通过 [REPLACE INTO ... SET](Data_creation_and_modification/Updating_documents/REPLACE.md#SQL-REPLACE) 支持部分替换。
* [Commit 7c16](https://github.com/manticoresoftware/manticoresearch/commit/7c167b934ce416d3dfbd658744a231e638b14d9a) 更新了默认合并缓冲区大小：`.spa`（标量属性）：256KB -> 8MB；`.spb`（blob 属性）：256KB -> 8MB；`.spc`（列属性）：1MB，无变化；`.spds`（文档存储）：256KB -> 8MB；`.spidx`（二级索引）：256KB 缓冲区 -> 128MB 内存限制；`.spi`（字典）：256KB -> 16MB；`.spd`（文档列表）：8MB，无变化；`.spp`（命中列表）：8MB，无变化；`.spe`（跳跃列表）：256KB -> 8MB。
* [Issue #1859](https://github.com/manticoresoftware/manticoresearch/issues/1859) 通过 JSON 添加了[复合聚合](Searching/Grouping.md#GROUP-BY-multiple-fields-at-once)。
* [Commit 216b](https://github.com/manticoresoftware/manticoresearch-buddy/commit/216b824) 因某些正则表达式模式问题并无显著时间优势，禁用了 PCRE.JIT。
* [Commit 55cd](https://github.com/manticoresoftware/manticoresearch/commit/55cdce32) 添加了对原生 Galera v.3 (api v25) (`libgalera_smm.so` 来自 MySQL 5.x) 的支持。
* [Commit 86f9](https://github.com/manticoresoftware/manticoresearch-buddy/commit/86f98ab) 将指标后缀从 `_rate` 改为 `_rps`。
* [Commit c0c1](https://github.com/manticoresoftware/manticoresearch-helm/commit/c0c1cff780031412ef57cefab987dfd16613368e) 改进了关于负载均衡器 HA 支持的文档。
* [Commit d1d2](https://github.com/manticoresoftware/manticoresearch/commit/d1d2ca18857c68106e89743756260e73285be739) 在错误消息中将 `index` 改为 `table`；修正了bison解析器的错误消息修正。
* [Commit fd26](https://github.com/manticoresoftware/manticoresearch/commit/fd26671b) 支持 `manticore.tbl` 作为表名。
* [Issue #1105](https://github.com/manticoresoftware/manticoresearch/issues/1105) 支持通过 systemd 运行索引器（[文档](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Schedule-indexer-via-systemd)）。❤️ 感谢 [@subnix](https://github.com/subnix) 的 PR。
* [Issue #1294](https://github.com/manticoresoftware/manticoresearch/issues/1294) GEODIST() 中支持辅助索引。
* [Issue #1394](https://github.com/manticoresoftware/manticoresearch/issues/1394) 简化了 [SHOW THREADS](Node_info_and_management/SHOW_THREADS.md#SHOW-THREADS)。
* [Issue #1424](https://github.com/manticoresoftware/manticoresearch/issues/1424) 为 `create distributed table` 语句添加默认值支持（`agent_connect_timeout` 和 `agent_query_timeout`）。
* [Issue #1442](https://github.com/manticoresoftware/manticoresearch/issues/1442) 添加了 [expansion_limit](Searching/Options.md#expansion_limit) 搜索查询选项，用以覆盖 `searchd.expansion_limit`。
* [Issue #1448](https://github.com/manticoresoftware/manticoresearch/issues/1448) 实现了 [ALTER TABLE](Updating_table_schema_and_settings.md#Updating-table-schema-in-RT-mode) 的 int->bigint 转换。
* [Issue #1456](https://github.com/manticoresoftware/manticoresearch/issues/1456) MySQL 响应中的元信息。
* [Issue #1494](https://github.com/manticoresoftware/manticoresearch/issues/1494) [SHOW VERSION](Node_info_and_management/SHOW_VERSION.md#SHOW-VERSION)。
* [Issue #1582](https://github.com/manticoresoftware/manticoresearch/issues/1582) 通过 JSON 支持使用 id 数组删除文档（[文档](Data_creation_and_modification/Deleting_documents.md?client=JSON#Deleting-documents)）。
* [Issue #1589](https://github.com/manticoresoftware/manticoresearch/issues/1589) 改进了 “unsupported value type” 错误信息。
* [Issue #1634](https://github.com/manticoresoftware/manticoresearch/issues/1634) 在 `SHOW STATUS` 中添加了 Buddy 版本信息。
* [Issue #1641](https://github.com/manticoresoftware/manticoresearch/issues/1641) 针对关键词无文档的情况，优化匹配请求。
* [Issue #1712](https://github.com/manticoresoftware/manticoresearch/issues/1712) 添加了对布尔属性从字符串 “true” 和 “false” 转换的支持，在发布数据时使用。
* [Issue #1713](https://github.com/manticoresoftware/manticoresearch/issues/1713) 添加了 [access_dict](Server_settings/Searchd.md#access_dict) 表和 searchd 选项。
* [Issue #1767](https://github.com/manticoresoftware/manticoresearch/issues/1767) 在配置的 searchd 部分添加新选项：[expansion_merge_threshold_docs](Server_settings/Searchd.md#expansion_merge_threshold_docs) 和 [expansion_merge_threshold_hits](Server_settings/Searchd.md#expansion_merge_threshold_hits)；使得合并扩展词中的微小词的阈值可配置。
* [Issue #1768](https://github.com/manticoresoftware/manticoresearch/issues/1768) 在 `@@system.sessions` 中添加了最后命令时间的显示。
* [Issue #1806](https://github.com/manticoresoftware/manticoresearch/issues/1806) 升级 Buddy 协议至版本 2。
* [Issue #1810](https://github.com/manticoresoftware/manticoresearch/issues/1810) 为 `/sql` 端点实现了额外的请求格式，以便于与库的集成。
* [Issue #1825](https://github.com/manticoresoftware/manticoresearch/pull/1825) 为 SHOW BUDDY PLUGINS 添加了信息部分。
* [Issue #1837](https://github.com/manticoresoftware/manticoresearch/issues/1837) 改进了在使用大包的 `CALL PQ` 中的内存消耗。
* [Issue #1853](https://github.com/manticoresoftware/manticoresearch/pull/1853) 编译器从 Clang 15 切换到 Clang 16。
* [Issue #1857](https://github.com/manticoresoftware/manticoresearch/issues/1857) 添加字符串比较。❤️ 感谢 [@etcd](https://github.com/etcd) 的 PR。
* [Issue #1915](https://github.com/manticoresoftware/manticoresearch/issues/1915) 添加了对联接存储字段的支持。
* [Issue #1937](https://github.com/manticoresoftware/manticoresearch/issues/1937) 在查询日志中记录客户端的 host:port。
* [Issue #1981](https://github.com/manticoresoftware/manticoresearch/issues/1981) 修正错误。
* [Issue #1983](https://github.com/manticoresoftware/manticoresearch/issues/1983) 引入了针对 [通过 JSON 显示的查询计划](Node_info_and_management/Profiling/Query_plan.md?client=JSON#Query-plan)的详细级别支持。
* [Issue #2010](https://github.com/manticoresoftware/manticoresearch/issues/2010) 除非设置了 `log_level=debug`，否则禁用记录来自 Buddy 的查询日志。
* [Issue #2035](https://github.com/manticoresoftware/manticoresearch/issues/2035) 支持 Linux Mint 21.3。
* [Issue #2056](https://github.com/manticoresoftware/manticoresearch/issues/2056) Manticore 无法用 Mysql 8.3+ 构建。
* [Issue #2112](https://github.com/manticoresoftware/manticoresearch/issues/2112) 针对附加包含重复项的普通表后，实时表块可能出现重复条目的情况，添加了 `DEBUG DEDUP` 命令。
* [Issue #212](https://github.com/manticoresoftware/manticoresearch-buddy/issues/212) 在 [SHOW QUERIES](Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES) 中添加时间字段。
* [Issue #218](https://github.com/manticoresoftware/manticoresearch-buddy/issues/218) 将 `@timestamp` 列作为时间戳处理。
* [Issue #252](https://github.com/manticoresoftware/manticoresearch-buddy/issues/250) 实现启用/禁用 buddy 插件的逻辑。
* [Issue #254](https://github.com/manticoresoftware/manticoresearch-buddy/issues/254) 更新了 composer 至修复了近期 CVE 的较新版本。
* [问题 #340](https://github.com/manticoresoftware/manticoresearch/issues/340) Manticore systemd 单元中与 `RuntimeDirectory` 相关的轻微优化。
* [问题 #51](https://github.com/manticoresoftware/executor/issues/51) 添加了 rdkafka 支持并更新到 PHP 8.3.3。
* [问题 #527](https://github.com/manticoresoftware/manticoresearch/issues/527) 支持[附加](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Attaching_one_table_to_another.md#Attaching-one-table-to-another)一个 RT 表。新增命令 [ALTER TABLE ... RENAME](Updating_table_schema_and_settings.md#Renaming-a-real-time-table)。

### 重大更改和弃用
* ⚠️[问题 #1436](https://github.com/manticoresoftware/manticoresearch/issues/1436) 修复了 IDF 计算问题。现在默认启用 `local_df`。改进了主-代理搜索协议（版本已更新）。如果您在分布式环境中运行 Manticore Search 多个实例，请确保先升级代理，再升级主节点。
* ⚠️[问题 #1572](https://github.com/manticoresoftware/manticoresearch/issues/1572) 添加了分布式表的复制并更新了复制协议。如果您运行复制集群，需要：
  - 首先，干净地停止所有节点
  - 然后，使用 Linux 工具 `manticore_new_cluster` 以 `--new-cluster` 参数启动最后停止的节点。
  - 更多细节请阅读[重启集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster)。
* ⚠️[问题 #1763](https://github.com/manticoresoftware/manticoresearch/pull/1763) HTTP API 端点别名 `/json/*` 已废弃。
* ⚠️[问题 #1982](https://github.com/manticoresoftware/manticoresearch/issues/1982) JSON 中的[profile](Node_info_and_management/Profiling/Query_profile.md#Query-profile) 改为[plan](Node_info_and_management/Profiling/Query_plan.md#Query-plan)，并增加了 JSON 查询分析。
* ⚠️[提交 e235](https://github.com/manticoresoftware/manticoresearch-backup/commit/e23585a) manticore-backup 不再备份 `plugin_dir`。
* ⚠️[问题 #171](https://github.com/manticoresoftware/manticoresearch-buddy/issues/155) 将 Buddy 迁移到 Swoole 以提升性能和稳定性。切换到新版本时，请确保所有 Manticore 包均已更新。
* ⚠️[问题 #196](https://github.com/manticoresoftware/manticoresearch-buddy/pull/205) 将所有核心插件合并进 Buddy 并更改了核心逻辑。
* ⚠️[问题 #2107](https://github.com/manticoresoftware/manticoresearch/issues/2107) 在 `/search` 响应中将文档 ID 作为数字处理。
* ⚠️[问题 #38](https://github.com/manticoresoftware/executor/issues/38) 添加了 Swoole，禁用了 ZTS，并移除了 parallel 扩展。
* ⚠️[问题 #1929](https://github.com/manticoresoftware/manticoresearch/issues/1929) `charset_table` 中的覆盖在某些情况下未生效。

### 复制相关更改
* [提交 3376](https://github.com/manticoresoftware/manticoresearch/commit/3376351f73db3ff0cb6cb708a304a7bd83e69cfb) 修复了大文件 SST 复制错误。
* [提交 6d36](https://github.com/manticoresoftware/manticoresearch/commit/6d36c68fb7ce3cb274b467aac435788905084952) 为复制命令增加了重试机制；修复了网络拥塞及丢包情况下复制加入失败的问题。
* [提交 842e](https://github.com/manticoresoftware/manticoresearch/commit/842e27e71790833ab4125d49103d2933d5df60c3) 将复制中的 FATAL 消息改为 WARNING 消息。
* [提交 8c32](https://github.com/manticoresoftware/manticoresearch/commit/8c32bce7db39d8ed3b1a06aaa1d2c3ec372084f0) 修复了无表或空表的复制集群中 `gcache.page_size` 的计算；同时修复了 Galera 选项保存和加载问题。
* [提交 a2af](https://github.com/manticoresoftware/manticoresearch/commit/a2af06ca36abcbf893e43c8803853130b397c54b) 添加了跳过加入节点复制命令的功能。
* [提交 c054](https://github.com/manticoresoftware/manticoresearch/commit/c0541e6892f5116b1f957b40060ba8ecd01a5afb) 修复了更新 blob 属性与替换文档时复制死锁问题。
* [提交 e80d](https://github.com/manticoresoftware/manticoresearch/commit/e80d505b963999016033613c40c1cf3151d648e7) 新增了 [replication_connect_timeout](Server_settings/Searchd.md#replication_connect_timeout)、[replication_query_timeout](Server_settings/Searchd.md#replication_query_timeout)、[replication_retry_delay](Server_settings/Searchd.md#replication_retry_delay)、[replication_retry_count](Server_settings/Searchd.md#replication_retry_count) 等 searchd 配置选项，用于控制复制过程中的网络行为，类似于 `searchd.agent_*` 但默认值不同。
* [问题 #1356](https://github.com/manticoresoftware/manticoresearch/issues/1356) 修复了复制节点在部分节点丢失且这些节点名称解析失败后的重试问题。
* [问题 #1445](https://github.com/manticoresoftware/manticoresearch/issues/1445) 修复了 `show variables` 中复制日志详细等级的问题。
* [问题 #1482](https://github.com/manticoresoftware/manticoresearch/issues/1482) 修复了 Kubernetes 中 pod 重启时加入节点连接集群的复制问题。
* [问题 #1962](https://github.com/manticoresoftware/manticoresearch/issues/1962) 修复了空集群上带无效节点名时复制 ALTER 操作长时间等待的问题。

### Bug 修复
* [提交 8a48](https://github.com/manticoresoftware/manticoresearch/commit/8a486c7ba1a293879cdecc41fb52eb7ab0cbf832) 修复了 `count distinct` 中未使用匹配项清理可能导致崩溃的问题。
* [问题 #1569](https://github.com/manticoresoftware/manticoresearch/issues/1569) 二进制日志现在以事务粒度写入。
* [Issue #2089](https://github.com/manticoresoftware/manticoresearch/issues/2089) 修复了与64位ID相关的一个错误，该错误可能导致通过MySQL插入时出现“Malformed packet”错误，进而导致[表损坏和ID重复](https://github.com/manticoresoftware/manticoresearch/issues/2090)。
* [Issue #2160](https://github.com/manticoresoftware/manticoresearch/issues/2160) 修正了日期被插入时错误地当作UTC时间而非本地时区的问题。
* [Issue #2177](https://github.com/manticoresoftware/manticoresearch/issues/2177) 修复了在对实时表执行搜索且`index_token_filter`非空时发生的崩溃。
* [Issue #2209](https://github.com/manticoresoftware/manticoresearch/issues/2209) 更改了RT列存储中的重复过滤机制，修复崩溃和错误的查询结果。
* [Commit 001d](https://github.com/manticoresoftware/manticoresearch/commit/001d82904e4b887c49387308291aeefb07c0fbc4) 修复了html剥离器在处理关联字段后导致内存损坏的问题。
* [Commit 00eb](https://github.com/manticoresoftware/manticoresearch/commit/00eb2ebf) 避免了flush之后回退流位置，以防与mysqldump通信错误。
* [Commit 0553](https://github.com/manticoresoftware/manticoresearch/commit/05534114acf08c70ffcc41a23d558dec09a34373) 如果预读取尚未开始，则不等待其完成。
* [Commit 055a](https://github.com/manticoresoftware/manticoresearch/commit/055aac09f877570cd29b6594cb283182eba7f73b) 修复了Buddy输出字符串过大，未分多行显示在searchd日志中的问题。
* [Commit 0a88](https://github.com/manticoresoftware/manticoresearch/commit/0a88056307f82c50fcdc458ad6dbc307a00e3f92) 将MySQL接口关于失败的头部`debugv`详细等级的警告移动。
* [Commit 150a](https://github.com/manticoresoftware/manticoresearch/commit/150a20286b7d98d24b844ae577d7fda4a444798d) 修正了多集群管理操作中的竞态条件；禁止创建同名或同路径的多个集群。
* [Commit 2e40](https://github.com/manticoresoftware/manticoresearch/commit/2e405c1c9ebac6f46206e3df17ac1c19129c9aa3) 修复全文查询中的隐式截断；将MatchExtended拆分为模板部分。
* [Commit 75f5](https://github.com/manticoresoftware/manticoresearch/commit/75f51421542a2a0658f9e2f9f31048c8c5224479) 修正了`index_exact_words`在索引与加载表到守护进程时的不一致。
* [Commit 7643](https://github.com/manticoresoftware/manticoresearch/commit/7643976bea142e08535d701000eff0d09a99fda2) 修复了删除无效集群时未显示错误消息的问题。
* [Commit 7a03](https://github.com/manticoresoftware/manticoresearch/commit/7a034ac990d5ec9344320c05a6a2315583b9b858) 修正了CBO与队列合并的问题；修复了CBO与RT伪分片的问题。
* [Commit 7b4e](https://github.com/manticoresoftware/manticoresearch/commit/7b4ea95e) 在未加载辅助索引（SI）库及配置参数时，避免发布误导性的警告信息“WARNING: secondary_indexes set but failed to initialize secondary library”。
* [Commit 8496](https://github.com/manticoresoftware/manticoresearch/commit/8496317f099fade8d2db463fb88483e7fcdfe85b) 修正了quorum中的命中排序问题。
* [Commit 8973](https://github.com/manticoresoftware/manticoresearch-buddy/commit/8973ad3) 修复了ModifyTable插件中大写选项导致的问题。
* [Commit 9935](https://github.com/manticoresoftware/manticoresearch-buddy/commit/99350d4c184379b19f3efa9751d9b5be5e6edfb5) 修复了从包含空json值（表现为NULL）的dump恢复的问题。
* [Commit a28f](https://github.com/manticoresoftware/manticoresearch/commit/a28f6e087b3d957e48816070be17d0823904936a) 通过使用pcon修复了联接节点在接收SST时的SST超时问题。
* [Commit b5a5](https://github.com/manticoresoftware/manticoresearch/commit/b5a55cc26debcddcda5d9caef8366a707e0dd763) 修复了选择别名字符串属性时的崩溃。
* [Commit c556](https://github.com/manticoresoftware/manticoresearch/commit/c556e7ed8b6da8eac8ef094650df5f0a8977916b) 添加了对`morphology_skip_fields`字段的全文查询中将词项转换为`=term`的查询转换。
* [Commit cdc3](https://github.com/manticoresoftware/manticoresearch/commit/cdc38085145cd1025b96fcac2a46188a0425d9e1) 补充了缺失的配置键（skiplist_cache_size）。
* [Commit cf6e](https://github.com/manticoresoftware/manticoresearch/commit/cf6ec3bfdc3f0990fb31ff53822b2331a6667727) 修复了表达式排序器在处理大型复杂查询时的崩溃。
* [Commit e513](https://github.com/manticoresoftware/manticoresearch/commit/e5139ba75eef7223df375ae3cc3f138fa7d925f1) 修复了全文CBO与无效索引提示的问题。
* [Commit eb05](https://github.com/manticoresoftware/manticoresearch/commit/eb0514b3a6f57f278252b0724eebd112ec116e0f) 在关闭时中断预读取，以加快关闭速度。
* [Commit f945](https://github.com/manticoresoftware/manticoresearch/commit/f94555a293cfa51004ff81135b8bd8500b955115) 更改全文查询的堆栈计算，避免在出现复杂查询时崩溃。
* [Issue #1262](https://github.com/manticoresoftware/manticoresearch/issues/1262) 修复了索引器在索引具有多个同名列的SQL源时崩溃的问题。
* [Issue #1273](https://github.com/manticoresoftware/manticoresearch/issues/1273) 对于不存在的系统变量返回0而非<empty>。
* [Issue #1289](https://github.com/manticoresoftware/manticoresearch/issues/1289) 修复了检查RT表外部文件时indextool的错误。
* [Issue #1335](https://github.com/manticoresoftware/manticoresearch/issues/1335) 修复了由于短语内多词形导致的查询解析错误。
* [Issue #1364](https://github.com/manticoresoftware/manticoresearch/issues/1364) 添加了旧版binlog的空binlog文件重放功能。
* [Issue #1365](https://github.com/manticoresoftware/manticoresearch/issues/1365) 修复了删除最后一个空binlog文件的问题。
* [Issue #1372](https://github.com/manticoresoftware/manticoresearch/issues/1372) 修复了在守护进程启动时，`data_dir` 更改后影响当前工作目录时相对路径错误（转换为守护进程启动目录的绝对路径）。
* [Issue #1393](https://github.com/manticoresoftware/manticoresearch/issues/1393) hn_small 中的最慢时间退化：在守护进程启动时获取/缓存 CPU 信息。
* [Issue #1395](https://github.com/manticoresoftware/manticoresearch/issues/1395) 修复了索引加载期间关于缺失外部文件的警告。
* [Issue #1402](https://github.com/manticoresoftware/manticoresearch/issues/1402) 修复了全局分组器在释放数据指针属性时的崩溃。
* [Issue #1403](https://github.com/manticoresoftware/manticoresearch/issues/1403) `_ADDITIONAL_SEARCHD_PARAMS` 未生效。
* [Issue #1427](https://github.com/manticoresoftware/manticoresearch/issues/1427) 修复了每个表的 `agent_query_timeout` 被默认查询选项 `agent_query_timeout` 覆盖的问题。
* [Issue #1444](https://github.com/manticoresoftware/manticoresearch/issues/1444) 修复了使用带有每匹配多个值的 `packedfactors()` 时，分组器和排序器的崩溃。
* [Issue #1458](https://github.com/manticoresoftware/manticoresearch/issues/1458) Manticore 在频繁更新索引时崩溃。
* [Issue #1481](https://github.com/manticoresoftware/manticoresearch/issues/1481) 修复了解析错误后解析查询清理时的崩溃。
* [Issue #1484](https://github.com/manticoresoftware/manticoresearch/issues/1484) 修复了 HTTP JSON 请求未路由到 buddy 的问题。
* [Issue #1499](https://github.com/manticoresoftware/manticoresearch/issues/1499) JSON 属性的根值不能是数组。已修复。
* [Issue #1507](https://github.com/manticoresoftware/manticoresearch/issues/1507) 修复了事务中表重建时的崩溃。
* [Issue #1515](https://github.com/manticoresoftware/manticoresearch/issues/1515) 修复了俄语词元的简写形式展开。
* [Issue #1579](https://github.com/manticoresoftware/manticoresearch/issues/1579) 修复了在[date_format](Functions/Date_and_time_functions.md#DATE_FORMAT())表达式中使用 JSON 和 STRING 属性的问题。
* [Issue #1580](https://github.com/manticoresoftware/manticoresearch/issues/1580) 修复了多别名指向 JSON 字段的分组器。
* [Issue #1594](https://github.com/manticoresoftware/manticoresearch/issues/1594) dev 中 total_related 错误：修复了隐式 cutoff 与 limit 的问题；在 json 查询中增加更好的全扫描检测。
* [Issue #1603](https://github.com/manticoresoftware/manticoresearch/issues/1603) 修复了所有日期表达式中使用 JSON 和 STRING 属性的问题。
* [Issue #1609](https://github.com/manticoresoftware/manticoresearch/issues/1609) 使用 LEVENSHTEIN() 时崩溃。
* [Issue #1612](https://github.com/manticoresoftware/manticoresearch/issues/1612) 修复了带高亮的搜索查询解析错误后内存破坏的问题。
* [Issue #1614](https://github.com/manticoresoftware/manticoresearch/issues/1614) 禁用对短于 `min_prefix_len` / `min_infix_len` 的词项进行通配符扩展。
* [Issue #1617](https://github.com/manticoresoftware/manticoresearch/issues/1617) 修改行为：如果 Buddy 成功处理请求，则不记录错误。
* [Issue #1635](https://github.com/manticoresoftware/manticoresearch/issues/1635) 修复了查询设置了 limit 时，搜索查询元信息中的 total。
* [Issue #1640](https://github.com/manticoresoftware/manticoresearch/issues/1640) 无法通过 JSON 在纯模式下使用大写表名。
* [Issue #1643](https://github.com/manticoresoftware/manticoresearch/issues/1643) 提供了默认的 `SPH_EXTNODE_STACK_SIZE` 值。
* [Issue #1646](https://github.com/manticoresoftware/manticoresearch/issues/1646) 修复了 MVA 属性使用 ALL/ANY 的负过滤器在 SphinxQL 日志中的问题。
* [Issue #1660](https://github.com/manticoresoftware/manticoresearch/issues/1660) 修复了其他索引的 docid 杀列表应用问题。❤️ 感谢 [@raxoft](https://github.com/raxoft) 的 PR。
* [Issue #1668](https://github.com/manticoresoftware/manticoresearch/issues/1668) 修复了在原始索引全扫描（无任何索引迭代器）时因提前退出导致丢失匹配的问题；移除普通行迭代器中的 cutoff。
* [Issue #1671](https://github.com/manticoresoftware/manticoresearch/issues/1671) 修复了对含代理和本地表的分布式表查询时的 `FACET` 错误。
* [Issue #1690](https://github.com/manticoresoftware/manticoresearch/issues/1690) 修复了大值直方图估计时的崩溃。
* [Issue #1692](https://github.com/manticoresoftware/manticoresearch/issues/1692) alter table tbl add column col uint 时崩溃。
* [Issue #1710](https://github.com/manticoresoftware/manticoresearch/issues/1710) 条件 `WHERE json.array IN (<value>)` 返回空结果。
* [Issue #172](https://github.com/manticoresoftware/manticoresearch-buddy/issues/172) 修复了发送请求到 `/cli` 时 TableFormatter 的问题。
* [Issue #1742](https://github.com/manticoresoftware/manticoresearch/issues/1742) 缺失 wordforms 文件时，`CREATE TABLE` 未失败。
* [Issue #1762](https://github.com/manticoresoftware/manticoresearch/issues/1762) RT 表中属性顺序现在遵循配置顺序。
* [Issue #1765](https://github.com/manticoresoftware/manticoresearch/issues/1765) 具有 'should' 条件的 HTTP bool 查询返回不正确结果。
* [Issue #1769](https://github.com/manticoresoftware/manticoresearch/issues/1769) 按字符串属性排序时，`SPH_SORT_ATTR_DESC` 和 `SPH_SORT_ATTR_ASC` 不起作用。
* [Issue #177](https://github.com/manticoresoftware/manticoresearch-buddy/issues/177) 为 curl 请求到 Buddy 禁用了 `Expect: 100-continue` HTTP 头。
* [Issue #1791](https://github.com/manticoresoftware/manticoresearch/issues/1791) GROUP BY 别名导致崩溃。
* [Issue #1792](https://github.com/manticoresoftware/manticoresearch/issues/1792) Windows 上 SQL 元信息摘要显示错误时间。
* [Issue #1794](https://github.com/manticoresoftware/manticoresearch/issues/1794) 修复了 JSON 查询中单词性能下降的问题。
* [Issue #1798](https://github.com/manticoresoftware/manticoresearch/issues/1798) 不兼容的过滤器在 `/search` 上未引发错误。
* [Issue #1802](https://github.com/manticoresoftware/manticoresearch/issues/1802) 修复了 `ALTER CLUSTER ADD` 和 `JOIN CLUSTER` 操作等待彼此完成的问题，防止在 `ALTER` 向集群添加表时，捐赠节点向加入节点发送表的竞争条件。
* [Issue #1811](https://github.com/manticoresoftware/manticoresearch/issues/1811) 错误处理了 `/pq/{table}/*` 请求。
* [Issue #1816](https://github.com/manticoresoftware/manticoresearch/issues/1816) `UNFREEZE` 在某些情况下不起作用。
* [Issue #183](https://github.com/manticoresoftware/manticoresearch-buddy/pull/183) 修复了在某些情况下 MVA 恢复的问题。
* [Issue #1849](https://github.com/manticoresoftware/manticoresearch/issues/1849) 修复了与 MCL 一起使用时 indextool 关闭时崩溃的问题。
* [Issue #1866](https://github.com/manticoresoftware/manticoresearch/issues/1866) 修复了 `/cli_json` 请求的不必要 URL 解码。
* [Issue #1872](https://github.com/manticoresoftware/manticoresearch/issues/1872) 更改守护进程启动时的 plugin_dir 设置逻辑。
* [Issue #1874](https://github.com/manticoresoftware/manticoresearch/issues/1874) alter table ... 异常失败。
* [Issue #1891](https://github.com/manticoresoftware/manticoresearch/issues/1891#issuecomment-2051941200) 插入数据时 Manticore 崩溃，信号 11。
* [Issue #1920](https://github.com/manticoresoftware/manticoresearch/issues/1920) 降低了 [low_priority](Searching/Options.md#low_priority) 的限流阈值。
* [Issue #1924](https://github.com/manticoresoftware/manticoresearch/issues/1924) Mysqldump + mysql 恢复的错误。
* [Issue #1951](https://github.com/manticoresoftware/manticoresearch/issues/1951) 修复了在缺少本地表或代理描述不正确时分布式表错误创建的问题；现在返回错误信息。
* [Issue #1972](https://github.com/manticoresoftware/manticoresearch/issues/1972) 实现了 `FREEZE` 计数器以避免冻结/解冻问题。
* [Issue #1980](https://github.com/manticoresoftware/manticoresearch/issues/1980) 遵守 OR 节点中的查询超时。此前在某些情况下 `max_query_time` 可能不起作用。
* [Issue #1986](https://github.com/manticoresoftware/manticoresearch/issues/1986) 重命名 new 到 current [manticore.json] 失败。
* [Issue #1988](https://github.com/manticoresoftware/manticoresearch/issues/1988) 全文查询可能忽略 `SecondaryIndex` CBO 提示。
* [Issue #1990](https://github.com/manticoresoftware/manticoresearch/issues/1990) 修正 `expansion_limit`，用于对来自多个磁盘块或内存块的调用关键字的最终结果集切片。
* [Issue #1994](https://github.com/manticoresoftware/manticoresearch/issues/1994) 错误的外部文件。
* [Issue #2021](https://github.com/manticoresoftware/manticoresearch/issues/2021) 停止 Manticore 后可能残留一些 manticore-executor 进程。
* [Issue #2029](https://github.com/manticoresoftware/manticoresearch/issues/2029) 使用 Levenshtein 距离时崩溃。
* [Issue #2037](https://github.com/manticoresoftware/manticoresearch/issues/2037) 多次 max 操作在空索引上运行后出现错误。
* [Issue #2052](https://github.com/manticoresoftware/manticoresearch/issues/2052) 多组与 JSON.field 一起使用时崩溃。
* [Issue #2067](https://github.com/manticoresoftware/manticoresearch/issues/2067) 对 _update 的错误请求导致 Manticore 崩溃。
* [Issue #2069](https://github.com/manticoresoftware/manticoresearch/issues/2069) 修正了 JSON 接口中字符串过滤器的封闭区间比较器问题。
* [Issue #2082](https://github.com/manticoresoftware/manticoresearch/issues/2082) 当 data_dir 路径位于符号链接上时，`alter` 失败。
* [Issue #2102](https://github.com/manticoresoftware/manticoresearch/pull/2102) 改进 mysqldump 中对 SELECT 查询的特殊处理，确保生成的 INSERT 语句与 Manticore 兼容。
* [Issue #2103](https://github.com/manticoresoftware/manticoresearch/issues/2103) 泰语字符归属错误字符集。
* [Issue #2124](https://github.com/manticoresoftware/manticoresearch/issues/2124) 使用 SQL 中保留字时崩溃。
* [Issue #2154](https://github.com/manticoresoftware/manticoresearch/issues/2154) 使用词形变化的表无法导入。
* [Issue #2176](https://github.com/manticoresoftware/manticoresearch/issues/2176) 修复了当引擎参数设置为 'columnar' 并通过 JSON 添加重复 ID 时发生的崩溃。
* [Issue #223](https://github.com/manticoresoftware/manticoresearch-buddy/issues/223) 尝试插入无模式且无列名文档时给出正确错误。
* [Issue #239](https://github.com/manticoresoftware/manticoresearch-buddy/issues/239) 自动模式多行插入可能失败。
* [Issue #399](https://github.com/manticoresoftware/manticoresearch/issues/399) 在数据源声明了 id 属性时，索引时添加错误信息。
* [Issue #59](https://github.com/manticoresoftware/manticoresearch-helm/issues/59) Manticore 集群崩溃。
* [Issue #68](https://github.com/manticoresoftware/manticoresearch-helm/issues/68) 如果存在 percolate 表，optimize.php 会崩溃。
* [Issue #77](https://github.com/manticoresoftware/manticoresearch-helm/issues/77) 修复了 Kubernetes 部署错误。
* [Issue #274](https://github.com/manticoresoftware/manticoresearch-buddy/issues/274) 修正了对 Buddy 并发请求处理错误。

### 与 manticore-backup 相关
* [Issue #97](https://github.com/manticoresoftware/manticoresearch-backup/pull/97) 当可用时，设置 VIP HTTP 端口为默认值。
各种改进：改进了版本检查和流式ZSTD解压缩；在恢复时添加了版本不匹配的用户提示；修正了恢复时不同版本的错误提示行为；增强了解压缩逻辑，直接从流中读取而不是读入工作内存；添加了`--force`标志
* [Commit 3b35](https://github.com/manticoresoftware/manticoresearch-backup/commit/3b357ea) 在Manticore搜索启动后添加备份版本显示，以便在此阶段识别问题。
* [Commit ad2e](https://github.com/manticoresoftware/manticoresearch-backup/commit/ad2e5da) 更新了连接守护进程失败时的错误信息。
* [Commit ce5e](https://github.com/manticoresoftware/manticoresearch-backup/commit/ce5ea85) 修复了将绝对根备份路径转换为相对路径的问题，并移除了恢复时的可写性检查，以支持从不同路径恢复。
* [Commit db7e](https://github.com/manticoresoftware/manticoresearch-backup/commit/db7e2b9) 为文件迭代器添加排序，确保各种情况下的一致性。
* [Issue #106](https://github.com/manticoresoftware/manticoresearch-backup/issues/105) 多配置的备份与恢复。
* [Issue #91](https://github.com/manticoresoftware/manticoresearch-backup/pull/91) 添加defattr以防止RHEL安装后文件出现异常用户权限。
* [Issue #91](https://github.com/manticoresoftware/manticoresearch-backup/pull/91) 添加额外的chown，确保Ubuntu环境下文件默认属于root用户。

### 与MCL相关（列式、二级、knn库）
* [Commit f104](https://github.com/manticoresoftware/columnar/commit/f104a4709bbb144140d09a09bc0fc1db1895e331) 支持向量搜索。
* [Commit 2169](https://github.com/manticoresoftware/columnar/commit/216953beef6676f7e34c9f6f1a30c5980d746710) 修复了二级索引构建被中断时临时文件清理的问题。解决了守护进程创建`tmp.spidx`文件时超过打开文件限制的问题。
* [Commit 709b](https://github.com/manticoresoftware/columnar/commit/709b9acaaac97d9a1ca8796892f9ad432021c785) 为列式和二级索引使用独立的streamvbyte库。
* [Commit 1c26](https://github.com/manticoresoftware/manticoresearch/commit/1c2623763ced14bb94ad624dde9614a938408d89) 添加警告指出列式存储不支持json属性。
* [Commit 3acd](https://github.com/manticoresoftware/columnar/commit/3acd6714ffce103447d7931dfc75bb1a7449f340) 修复二级索引中的数据解包。
* [Commit 574c](https://github.com/manticoresoftware/manticoresearch/commit/574c023152564fa72092c18ebd1f594abfd4b6d7) 修复混合行式和列式存储的磁盘块保存时崩溃。
* [Commit e87f](https://github.com/manticoresoftware/columnar/commit/e87f088b458d4c5b1b6159fc72ed0d57b786891a) 修复了二级索引迭代器被提示已处理的块问题。
* [Issue #1474](https://github.com/manticoresoftware/manticoresearch/issues/1474) 使用列式引擎时，行式MVA列的更新失效。
* [Issue #1510](https://github.com/manticoresoftware/manticoresearch/issues/1510) 修复对用于`HAVING`中的列式属性聚合时崩溃。
* [Issue #1519](https://github.com/manticoresoftware/manticoresearch/issues/1519) 修复了在`expr`排行器中使用列式属性时崩溃。

### 与Docker相关
* ❗[Issue #42](https://github.com/manticoresoftware/docker/issues/42) 通过环境变量支持[plain indexation](https://github.com/manticoresoftware/docker#building-plain-tables)。
* ❗[Issue #47](https://github.com/manticoresoftware/docker/issues/47) 通过环境变量提升配置灵活性。
* [Issue #54](https://github.com/manticoresoftware/docker/pull/54) 改进了Docker的[备份和恢复](https://github.com/manticoresoftware/docker#backup-and-restore)流程。
* [Issue #77](https://github.com/manticoresoftware/docker/issues/76) 改进entrypoint，仅在首次启动时处理备份恢复。
* [Commit a27c](https://github.com/manticoresoftware/docker/commit/a27c048dd9e73d8d6bea8d9a4830cafb486b82c5) 修正查询日志输出到stdout。
* [Issue #38](https://github.com/manticoresoftware/docker/issues/38) 如果未设置EXTRA，则静音BUDDY警告。
* [Issue #71](https://github.com/manticoresoftware/docker/pull/71) 修正`manticore.conf.sh`中的主机名。

## 版本 6.2.12
发布日期：2023年8月23日

6.2.12版本继续6.2系列，解决了6.2.0发布后发现的问题。

### Bug 修复
* ❗[Issue #1351](https://github.com/manticoresoftware/manticoresearch/issues/1351) “Manticore 6.2.0无法通过systemctl在Centos 7上启动”：将`TimeoutStartSec`从`infinity`修改为`0`，以提高与Centos 7的兼容性。
* ❗[Issue #1364](https://github.com/manticoresoftware/manticoresearch/issues/1364) “从6.0.4升级到6.2.0后崩溃”：为旧版本binlog的空日志文件添加了重放功能。
* [PR #1334](https://github.com/manticoresoftware/manticoresearch/pull/1334) “修复searchdreplication.cpp中的拼写错误”：修正了`searchdreplication.cpp`中的拼写错误：beggining -> beginning。
* [Issue #1337](https://github.com/manticoresoftware/manticoresearch/issues/1337) “Manticore 6.2.0 WARNING: conn (local)(12), sock=8088: bailing on failed MySQL header, AsyncNetInputBuffer_c::AppendData: error 11 (Resource temporarily unavailable) return -1”：将MySQL接口关于header失败的警告日志级别降低到logdebugv。
* [Issue #1355](https://github.com/manticoresoftware/manticoresearch/issues/1355) “当node_address无法解析时join集群会挂起”：改进了当某些节点不可达且名称解析失败时的复制重试机制。此修复应解决Kubernetes和Docker节点相关的复制问题。增强了复制启动失败的错误信息，并对测试模型376进行了更新。另外，添加了清晰的名称解析失败错误提示。
* [Issue #1361](https://github.com/manticoresoftware/manticoresearch/issues/1361) “字符集 non_cjk 中“Ø”的小写映射缺失”：调整了‘Ø’字符的映射。
* [Issue #1365](https://github.com/manticoresoftware/manticoresearch/issues/1365) “searchd 在干净停止后留下 binlog.meta 和 binlog.001”：确保正确删除了最后一个空的 binlog 文件。
* [Commit 0871](https://github.com/manticoresoftware/manticoresearch/commit/0871070845e93ad702ae0dfb89b35e5f24cd0851)：修复了 Windows 平台上与原子复制限制相关的 `Thd_t` 构建问题。
* [Commit 1cc0](https://github.com/manticoresoftware/manticoresearch/commit/1cc04bedc174737118fd5a3663e35f4e95bd4f8c)：解决了 FT CBO 与 `ColumnarScan` 相关的问题。
* [Commit c6bf](https://github.com/manticoresoftware/manticoresearch/commit/c6bfb30d53e3f4b67f17769667733677a661c95b)：修正了测试 376，并为测试中 `AF_INET` 错误添加了替代方案。
* [Commit cbc3](https://github.com/manticoresoftware/manticoresearch/commit/cbc38705e0b4dcc9665e3e8cf64826290ebd11cf)：解决了复制过程中更新 blob 属性与替换文档导致的死锁问题。同时，移除了提交期间对索引的 rlock，因为索引已在更底层被锁定。

### 小改动
* [Commit 4f91](https://github.com/manticoresoftware/manticoresearch/commit/4f913a36e84e01f950b2797051222b0b1094d6fe) 更新了手册中关于 `/bulk` 端点的信息。

### MCL
* 支持 [Manticore Columnar Library](https://github.com/manticoresoftware/columnar) v2.2.4

## 版本 6.2.0
发布：2023年8月4日

### 重大改动
* 查询优化器增强了对全文搜索查询的支持，大幅提升了搜索效率和性能。
* 集成了：
  - [mysqldump](https://dev.mysql.com/doc/refman/8.0/en/mysqldump.html) —— 使用 `mysqldump` 进行[逻辑备份](Securing_and_compacting_a_table/Backup_and_restore.md#Backup-and-restore-with-mysqldump)
  - [Apache Superset](https://manticoresearch.com/blog/manticoresearch-apache-superset-integration/) 和 [Grafana](https://manticoresearch.com/blog/manticoresearch-grafana-integration/) 用于可视化存储在 Manticore 中的数据
  - [HeidiSQL](https://www.heidisql.com/) 和 [DBForge](https://www.devart.com/dbforge/) 以简化 Manticore 的开发
* 我们开始使用 [GitHub workflows](https://github.com/manticoresoftware/manticoresearch/actions)，简化贡献者利用核心团队在准备软件包时使用的相同持续集成（CI）流程。所有工作均可在 GitHub 托管的 runner 上运行，方便在您 fork 的 Manticore Search 中无缝测试更改。
* 我们开始使用 [CLT](https://github.com/manticoresoftware/clt) 测试复杂场景。例如，现在能够确保某次提交后构建的软件包能在所有受支持的 Linux 发行版中正确安装。命令行测试器（CLT）提供了以交互模式录制测试并轻松回放的友好方式。
* 通过结合哈希表和 HyperLogLog，显著提升了 count distinct 操作的性能。
* 启用了包含二级索引查询的多线程执行，线程数限制为物理 CPU 核心数。此改动将显著提升查询执行速度。
* `pseudo_sharding` 已调整为限于空闲线程数。这一更新显著提高了吞吐性能。
* 用户现在可以通过配置设置指定[默认属性存储引擎](Server_settings/Searchd.md#engine)，以更好地自定义满足特定工作负载需求。
* 支持 [Manticore Columnar Library 2.2.0](https://github.com/manticoresoftware/columnar/)，对[二级索引](Server_settings/Searchd.md#secondary_indexes)进行了多项漏洞修复和改进。

### 小改动
* [Buddy #153](https://github.com/manticoresoftware/manticoresearch-buddy/issues/153)：[/pq](Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md?client=JSON#Adding-rules-to-a-percolate-table) HTTP 端点现在作为 `/json/pq` HTTP 端点的别名。
* [Commit 0bf1](https://github.com/manticoresoftware/manticoresearch/commit/0bf17d9e)：确保 `upper()` 和 `lower()` 的多字节兼容性。
* [Commit 2bb9](https://github.com/manticoresoftware/manticoresearch/commit/2bb92765)：`count(*)` 查询不再扫描索引，而是返回预先计算值。
* [Commit 3c84](https://github.com/manticoresoftware/manticoresearch/commit/3c84091f)：现在可以使用 `SELECT` 进行任意计算并显示 `@@sysvars`。与以前不同，不再限制只能进行一个计算。因此，查询如 `select user(), database(), @@version_comment, version(), 1+1 as a limit 10` 将返回所有列。注意，可选的 ‘limit’ 总是被忽略。
* [Commit 6aca](https://github.com/manticoresoftware/manticoresearch/commit/6aca32f0)：实现了 `CREATE DATABASE` 占位查询。
* [Commit 9dc1](https://github.com/manticoresoftware/manticoresearch/commit/9dc12334)：执行 `ALTER TABLE table REBUILD SECONDARY` 时，二级索引不论属性是否更新，均会被重建。
* [Commit 46ed](https://github.com/manticoresoftware/manticoresearch/commit/46edb089)：使用预计算数据的排序器现在会在使用 CBO 之前被识别，以避免不必要的 CBO 计算。
* [Commit 102a](https://github.com/manticoresoftware/manticoresearch/commit/102ac604)：实现并使用了全文表达式堆栈的模拟，防止守护进程崩溃。
* [Commit 979f](https://github.com/manticoresoftware/manticoresearch/commit/979fa27c)：为不使用字符串/mvas/json 属性的匹配克隆代码添加了快速路径。
* [提交 a073](https://github.com/manticoresoftware/manticoresearch/commit/a0735ffe)：添加了对 `SELECT DATABASE()` 命令的支持。然而，它总是返回 `Manticore`。此新增功能对于与各种 MySQL 工具的集成至关重要。
* [提交 bc04](https://github.com/manticoresoftware/manticoresearch/commit/bc04908)：修改了 [/cli](Connecting_to_the_server/HTTP.md#/cli) 端点的响应格式，并添加了 `/cli_json` 端点来实现先前的 `/cli` 功能。
* [提交 d70b](https://github.com/manticoresoftware/manticoresearch/commit/d70b0d58)：`thread_stack` 现在可以通过 `SET` 语句在运行时更改。提供了会话本地和守护进程范围的变体。当前值可以在 `show variables` 输出中访问。
* [提交 d96e](https://github.com/manticoresoftware/manticoresearch/commit/d96ec6b9)：代码已集成到 CBO 中，以更准确地估计对字符串属性执行过滤器的复杂度。
* [提交 e77d](https://github.com/manticoresoftware/manticoresearch/commit/e77dd72f5a04531c352fad0d7afcd2a1cbae2510)：改进了 DocidIndex 成本计算，提升了整体性能。
* [提交 f3ae](https://github.com/manticoresoftware/manticoresearch/commit/f3ae8bea)：类似于 Linux 上的“uptime”，负载指标现在可在 `SHOW STATUS` 命令中查看。
* [提交 f3cc](https://github.com/manticoresoftware/manticoresearch/commit/f3cc0971)：`DESC` 和 `SHOW CREATE TABLE` 的字段和属性顺序现与 `SELECT * FROM` 保持一致。
* [提交 f3d2](https://github.com/manticoresoftware/manticoresearch/commit/f3d248a6)：不同的内部解析器现在在各种错误中提供其内部助记码（如 `P01`）。此改进有助于识别哪个解析器导致错误，同时隐藏非关键的内部细节。
* [问题 #271](https://github.com/manticoresoftware/manticoresearch/issues/271) “有时 CALL SUGGEST 不会建议单字母拼写错误的更正”：改进了 [SUGGEST/QSUGGEST](Searching/Spell_correction.md#CALL-QSUGGEST,-CALL-SUGGEST) 对短词的行为：添加了 `sentence` 选项以显示整句。
* [问题 #696](https://github.com/manticoresoftware/manticoresearch/issues/696) “当启用词干提取时，Percolate 索引无法正确通过精确短语查询搜索”：已修改 Percolate 查询以处理精确词项修饰符，改进搜索功能。
* [问题 #829](https://github.com/manticoresoftware/manticoresearch/issues/829) “日期格式化方法”：添加了 [date_format()](Functions/Date_and_time_functions.md#DATE_FORMAT()) 选择列表表达式，暴露了 `strftime()` 函数。
* [问题 #961](https://github.com/manticoresoftware/manticoresearch/issues/961) “通过 HTTP JSON API 对桶进行排序”：在 HTTP 接口的聚合桶中引入了可选的 [sort 属性](Searching/Faceted_search.md#HTTP-JSON)。
* [问题 #1062](https://github.com/manticoresoftware/manticoresearch/issues/1062) “改进 JSON 插入 API 失败的错误日志 - “不支持的值类型””：在发生错误时，`/bulk` 端点会报告已处理和未处理字符串（文档）的数量信息。
* [问题 #1070](https://github.com/manticoresoftware/manticoresearch/issues/1070) “CBO 提示不支持多个属性”：使索引提示支持多个属性。
* [问题 #1106](https://github.com/manticoresoftware/manticoresearch/issues/1106) “向 HTTP 搜索查询添加标签”：已向 [HTTP PQ 响应](Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md?client=JSON#Adding-rules-to-a-percolate-table) 添加标签。
* [问题 #1301](https://github.com/manticoresoftware/manticoresearch/issues/1301) “buddy 不应并行创建表”：解决了导致并行执行 CREATE TABLE 操作失败的问题。现在，同一时间只能执行一个 `CREATE TABLE` 操作。
* [问题 #1303](https://github.com/manticoresoftware/manticoresearch/issues/1303) “支持将 @ 作为列名”。
* [问题 #1316](https://github.com/manticoresoftware/manticoresearch/issues/1316) “ts 数据集上的查询在 ps=1 时较慢”：改进了 CBO 逻辑，并将默认直方图分辨率设置为 8k，以便更准确地处理随机分布值的属性。
* [问题 #1317](https://github.com/manticoresoftware/manticoresearch/issues/1317) “修复 CBO 与 hn 数据集上的全文搜索冲突”：实施了改进的逻辑以确定何时使用位图迭代器交集，何时使用优先队列。
* [问题 #1318](https://github.com/manticoresoftware/manticoresearch/issues/1318) “columnar：将迭代器接口改为单次调用”：列式迭代器现改为使用单次 `Get` 调用，替代之前先 `AdvanceTo` 后 `Get` 的两步调用以获取值。
* [问题 #1319](https://github.com/manticoresoftware/manticoresearch/issues/1319) “聚合计算加速（移除 CheckReplaceEntry？）”：从分组排序器中移除了 `CheckReplaceEntry` 调用，以加速聚合函数计算。
* [问题 #1320](https://github.com/manticoresoftware/manticoresearch/issues/1320) “create table 的 read_buffer_docs/hits 不支持 k/m/g 语法”：`CREATE TABLE` 的选项 `read_buffer_docs` 和 `read_buffer_hits` 现支持 k/m/g 语法。
* 英文、德文和俄文的[语言包](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology)现可通过在 Linux 上执行命令 `apt/yum install manticore-language-packs` 轻松安装。在 macOS 上，使用命令 `brew install manticoresoftware/tap/manticore-language-packs`。
* 字段和属性顺序现在在 `SHOW CREATE TABLE` 和 `DESC` 操作之间保持一致。
* 在执行 `INSERT` 查询时，如果磁盘空间不足，新的 `INSERT` 查询将失败，直到有足够的磁盘空间。
* 新增了 [UINT64()](Functions/Type_casting_functions.md#UINT64%28%29) 类型转换函数。
* `/bulk`端点现在将空行处理为[提交](Data_creation_and_modification/Transactions.md#BEGIN,-COMMIT,-and-ROLLBACK)命令。更多信息见[这里](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md?client=JSON#Bulk-adding-documents)。
* 已实现针对[无效索引提示](Searching/Options.md#Query-optimizer-hints)的警告，提升了透明度并允许错误缓解。
* 当`count(*)`与单一过滤器一起使用时，查询现在在可用时利用二级索引的预计算数据，大幅加快查询速度。

### ⚠️ 破坏性变更
* ⚠️ 版本6.2.0中创建或修改的表无法被旧版本读取。
* ⚠️ 文档ID现在在索引和INSERT操作期间作为无符号64位整数处理。
* ⚠️ 查询优化器提示的语法已更新。新格式为`/*+ SecondaryIndex(uid) */`。请注意旧语法不再支持。
* ⚠️ [问题 #1160](https://github.com/manticoresoftware/manticoresearch/issues/1160)：禁止在表名中使用`@`，以避免语法冲突。
* ⚠️ 标记为`indexed`和`attribute`的字符串字段/属性现在在`INSERT`、`DESC`和`ALTER`操作中被视为单一字段。
* ⚠️ [问题 #1057](https://github.com/manticoresoftware/manticoresearch/issues/1057)：不支持SSE 4.2的系统将不再加载MCL库。
* ⚠️ [问题 #1143](https://github.com/manticoresoftware/manticoresearch/issues/1143)：[agent_query_timeout](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_query_timeout)故障已修复，现在生效。

### Bug修复
* [提交 2a6e](https://github.com/manticoresoftware/manticoresearch/commit/2a6ea8f7) “DROP TABLE时崩溃”：解决了执行DROP TABLE语句时RT表写操作（优化、磁盘块保存）等待时间过长的问题。新增警告，执行DROP TABLE命令后如果表目录非空将通知用户。
* [提交 2ebd](https://github.com/manticoresoftware/manticoresearch/commit/2ebd424d)：在多属性分组代码中补充了对列式属性的支持。
* [提交 3be4](https://github.com/manticoresoftware/manticoresearch/commit/3be4503f752216b37b8cde8e47fd2d77bbce63ae) 解决了因磁盘空间耗尽导致的崩溃问题，通过正确处理binlog写入错误避免崩溃。
* [提交 6adb](https://github.com/manticoresoftware/manticoresearch/commit/6adb0934)：修复了在查询中使用多个列式扫描迭代器（或二级索引迭代器）时偶发的崩溃问题。
* [提交 6bd9](https://github.com/manticoresoftware/manticoresearch/commit/6bd9f709)：使用预计算数据的排序器时过滤器未被移除问题已修复。
* [提交 6d03](https://github.com/manticoresoftware/manticoresearch/commit/6d03566a)：更新了CBO代码，改进了多线程执行基于行的属性过滤器查询的估算。
* [提交 6dd3](https://github.com/manticoresoftware/manticoresearch/commit/6dd3964), [Helm #56](https://github.com/manticoresoftware/manticoresearch-helm/issues/56) “Kubernetes集群中严重崩溃转储”：修复了JSON根对象的缺陷布隆过滤器；修复了因过滤JSON字段导致的守护进程崩溃。
* [提交 6e1b](https://github.com/manticoresoftware/manticoresearch/commit/6e1b849) 修正了因无效的`manticore.json`配置导致的守护进程崩溃。
* [提交 6fbc](https://github.com/manticoresoftware/manticoresearch/commit/6fbc4189) 修复了json范围过滤器对int64值的支持。
* [提交 9c67](https://github.com/manticoresoftware/manticoresearch/commit/9c677426) 解决了`.sph`文件在`ALTER`操作时可能损坏的问题。
* [提交 77cc](https://github.com/manticoresoftware/manticoresearch/commit/77ccd8f)：添加替换语句复制的共享密钥，解决多个主节点复制replace时发生的`pre_commit`错误。
* [提交 2884](https://github.com/manticoresoftware/manticoresearch/commit/2884519e) 解决了对函数如'date_format()'进行bigint检查的问题。
* [提交 9513](https://github.com/manticoresoftware/manticoresearch/commit/9513aca5)：当排序器使用预计算数据时，迭代器将不再显示在[SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META)中。
* [提交 a2a7](https://github.com/manticoresoftware/manticoresearch/commit/a2a70555)：更新全文节点的栈大小，防止复杂全文查询导致的崩溃。
* [提交 a062](https://github.com/manticoresoftware/manticoresearch/commit/a0626d7e)：修复了在包含JSON和字符串属性的更新复制过程中导致崩溃的bug。
* [提交 b3e6](https://github.com/manticoresoftware/manticoresearch/commit/b3e63e5d)：字符串构建器更新为使用64位整数，避免处理大型数据集时崩溃。
* [提交 c472](https://github.com/manticoresoftware/manticoresearch/commit/c472e5b)：修复了跨多个索引的count distinct导致的崩溃。
* [提交 d073](https://github.com/manticoresoftware/manticoresearch/commit/d0730272)：修正了即使禁用`pseudo_sharding`，RT索引磁盘块上的查询仍可被多线程执行的问题。
* [提交 d205](https://github.com/manticoresoftware/manticoresearch/commit/d205508e) `show index status`命令返回的值集合已修改，现根据所使用的索引类型而变化。
* [提交 e9bc](https://github.com/manticoresoftware/manticoresearch/commit/e9bc2f9) 修复处理批量请求时的HTTP错误，以及net loop未将错误返回客户端的问题。
* [提交 f77c](https://github.com/manticoresoftware/manticoresearch/commit/f77ce0e6) 对PQ采用了扩展栈。
* [提交 fac2](https://github.com/manticoresoftware/manticoresearch/commit/fac2d9687a20582765aaa89be9597804c4f845af) 更新导出排序器输出以匹配[packedfactors()](Functions/Searching_and_ranking_functions.md#PACKEDFACTORS%28%29)。
* [提交 ff87](https://github.com/manticoresoftware/manticoresearch/commit/ff87cc4)：修复了 SphinxQL 查询日志中过滤器中字符串列表的问题。
* [问题 #589](https://github.com/manticoresoftware/manticoresearch/issues/589) “字符集定义似乎依赖于代码的排序”：修正了重复项的不正确字符集映射。
* [问题 #811](https://github.com/manticoresoftware/manticoresearch/issues/811) “词形映射多个词语会影响带有关键词间 CJK 标点符号的短语搜索”：修复了带词形的短语查询中 ngram 令牌位置问题。
* [问题 #834](https://github.com/manticoresoftware/manticoresearch/issues/834) “搜索查询中的等号导致请求失败”：确保可以转义该符号并修正了 `expand_keywords` 选项导致的双重精确扩展。
* [问题 #864](https://github.com/manticoresoftware/manticoresearch/issues/864) “exceptions/stopwords 冲突”
* [问题 #910](https://github.com/manticoresoftware/manticoresearch/issues/910) “调用 call snippets() 时使用 libstemmer_fr 和 index_exact_words 导致 Manticore 崩溃”：解决了调用 `SNIPPETS()` 时导致崩溃的内部冲突。
* [问题 #946](https://github.com/manticoresoftware/manticoresearch/issues/946) “SELECT 结果出现重复记录”：修复了对带已删除文档的 RT 索引使用 `not_terms_only_allowed` 选项的查询结果中出现重复文档的问题。
* [问题 #967](https://github.com/manticoresoftware/manticoresearch/issues/967) “在 UDF 函数中使用 JSON 参数导致崩溃”：修正了启用伪分片且带 JSON 参数的 UDF 时导致守护进程崩溃的问题。
* [问题 #1050](https://github.com/manticoresoftware/manticoresearch/issues/1050) “FEDERATED 中的 count(*)”：修正了通过 `FEDERATED` 引擎执行带聚合的查询时导致守护进程崩溃的问题。
* [问题 #1052](https://github.com/manticoresoftware/manticoresearch/issues/1052) 修复了 `rt_attr_json` 列与列式存储不兼容的问题。
* [问题 #1072](https://github.com/manticoresoftware/manticoresearch/issues/1072) “* 被 ignore_chars 从搜索查询中移除”：修复此问题，使查询中的通配符不受 `ignore_chars` 影响。
* [问题 #1075](https://github.com/manticoresoftware/manticoresearch/issues/1075) “indextool --check 在存在分布式表时失败”：indextool 现兼容 JSON 配置中带有“distributed”和“template”索引的实例。
* [问题 #1081](https://github.com/manticoresoftware/manticoresearch/issues/1081) “特定 RT 数据集的特定查询导致 searchd 崩溃”：解决了带 packedfactors 和大量内部缓冲的查询导致守护进程崩溃的问题。
* [问题 #1095](https://github.com/manticoresoftware/manticoresearch/issues/1095) “使用 not_terms_only_allowed 时忽略已删除文档”
* [问题 #1099](https://github.com/manticoresoftware/manticoresearch/issues/1099) “indextool --dumpdocids 无法工作”：恢复了 `--dumpdocids` 命令的功能。
* [问题 #1100](https://github.com/manticoresoftware/manticoresearch/issues/1100) “indextool --buildidf 无法工作”：indextool 现在在完成 globalidf 后关闭文件。
* [问题 #1104](https://github.com/manticoresoftware/manticoresearch/issues/1104) “Count(*) 在远程表中被当作 schema set 处理”：解决了当代理返回空结果集时，守护进程对分布式索引的查询发送错误消息的问题。
* [问题 #1109](https://github.com/manticoresoftware/manticoresearch/issues/1109) “FLUSH ATTRIBUTES 在 threads=1 时挂起”。
* [问题 #1126](https://github.com/manticoresoftware/manticoresearch/issues/1126) “查询过程中与 MySQL 服务器失去连接 - manticore 6.0.5”：已解决使用多个列式属性过滤器时发生的崩溃。
* [问题 #1135](https://github.com/manticoresoftware/manticoresearch/issues/1135) “JSON 字符串过滤的大小写敏感性”：修正排序规则以使其在 HTTP 搜索请求中的过滤功能正常。
* [问题 #1140](https://github.com/manticoresoftware/manticoresearch/issues/1140) “错误字段中的匹配”：修正了与 `morphology_skip_fields` 相关的问题。
* [问题 #1155](https://github.com/manticoresoftware/manticoresearch/issues/1155) “通过 API 的系统远程命令应传递 g_iMaxPacketSize”：更新以绕过节点间复制命令的 `max_packet_size` 检查。同时，已将最新集群错误添加到状态显示中。
* [问题 #1302](https://github.com/manticoresoftware/manticoresearch/issues/1302) “优化失败时留下临时文件”：修复在合并或优化过程中发生错误后遗留临时文件的问题。
* [问题 #1304](https://github.com/manticoresoftware/manticoresearch/issues/1304) “为 buddy 启动超时添加环境变量”：新增环境变量 `MANTICORE_BUDDY_TIMEOUT`（默认值 3 秒）以控制启动时守护进程等待 buddy 消息的时长。
* [问题 #1305](https://github.com/manticoresoftware/manticoresearch/issues/1305) “保存 PQ 元数据时整数溢出”：减轻了守护进程保存大型 PQ 索引时过度的内存消耗。
* [问题 #1306](https://github.com/manticoresoftware/manticoresearch/issues/1306) “修改外部文件后无法重建 RT 表”：修正了修改外部文件时使用空字符串的错误；修复了修改外部文件后 RT 索引外部文件未清除的问题。
* [问题 #1307](https://github.com/manticoresoftware/manticoresearch/issues/1307) “SELECT 语句 sum(value) as value 工作异常”：修复了带别名的选择列表表达式可能隐藏索引属性的问题；同时修正了整数 int64 的 sum 变为 count 的问题。
* [Issue #1308](https://github.com/manticoresoftware/manticoresearch/issues/1308) “避免在复制中绑定到localhost”：确保复制不会绑定到具有多个IP的主机名的localhost。
* [Issue #1309](https://github.com/manticoresoftware/manticoresearch/issues/1309) “回复给mysql客户端失败，数据大于16Mb”：修复了向客户端返回大于16Mb的SphinxQL数据包的问题。
* [Issue #1310](https://github.com/manticoresoftware/manticoresearch/issues/1310) “‘paths to external files should be absolute’中错误的引用”：纠正了`SHOW CREATE TABLE`中外部文件完整路径的显示。
* [Issue #1311](https://github.com/manticoresoftware/manticoresearch/issues/1311) “调试版本在snippet中长字符串崩溃”：现在，`SNIPPET()`函数目标中的长字符串（>255个字符）被允许。
* [Issue #1312](https://github.com/manticoresoftware/manticoresearch/issues/1312) “kqueue轮询中use-after-delete的虚假崩溃（master-agent）”：修复了在kqueue驱动的系统（FreeBSD、MacOS等）上当master无法连接agent时的崩溃。
* [Issue #1313](https://github.com/manticoresoftware/manticoresearch/issues/1313) “连接自身时间过长”：当master连接MacOS/BSD上的agent时，现在使用统一的连接+查询超时，而不仅仅是连接。
* [Issue #1314](https://github.com/manticoresoftware/manticoresearch/issues/1314) “‘pq (json meta) with unreached embedded synonyms fails to load’”：修复了pq中嵌入同义词标志的问题。
* [Issue #1315](https://github.com/manticoresoftware/manticoresearch/issues/1315) “允许某些函数（sint、fibonacci、second、minute、hour、day、month、year、yearmonth、yearmonthday）使用隐式提升的参数值”。
* [Issue #1321](https://github.com/manticoresoftware/manticoresearch/issues/1321) “启用全文扫描中的多线程SI，但限制线程数”：代码已被实现到CBO中，以更好地预测在全文查询中使用二级索引时多线程的性能。
* [Issue #1322](https://github.com/manticoresoftware/manticoresearch/issues/1322) “使用预计算排序器后count(*)查询仍然缓慢”：当使用带有预计算数据的排序器时，不再初始化迭代器，从而避免了性能下降。
* [Issue #1411](https://github.com/manticoresoftware/manticoresearch/issues/1141) “sphinxql中的查询日志不保留MVA的原始查询”：现在，`all()/any()`被记录。

## 版本 6.0.4
发布日期：2023年3月15日

### 新特性
* 改进了与Logstash、Beats等的集成，包括：
  - 支持Logstash版本7.6 - 7.15，Filebeat版本7.7 - 7.12
  - 自动模式支持。
  - 新增对Elasticsearch-like格式批量请求的处理。
* [Buddy提交 ce90](https://github.com/manticoresoftware/manticoresearch-buddy/commit/ce907ea) Manticore启动时记录Buddy版本。

### Bug修复
* [Issue #588](https://github.com/manticoresoftware/manticoresearch/issues/588)、[Issue #942](https://github.com/manticoresoftware/manticoresearch/issues/942) 修复了bigram索引中的搜索元信息和关键词调用中的错误字符。
* [Issue #1027](https://github.com/manticoresoftware/manticoresearch/issues/1027) 拒绝小写HTTP头。
* ❗[Issue #1039](https://github.com/manticoresoftware/manticoresearch/issues/1039) 修复了Buddy控制台读取输出时守护进程的内存泄漏。
* [Issue #1056](https://github.com/manticoresoftware/manticoresearch/issues/1056) 修复了问号的异常行为。
* [Issue #1064](https://github.com/manticoresoftware/manticoresearch/issues/1064) 修复了分词器小写表中的竞态条件导致崩溃。
* [提交 59bb](https://github.com/manticoresoftware/manticoresearch/commit/59bb54c) 修复了JSON接口中显式设置id为null的文档的批量写入处理。
* [提交 7b6b](https://github.com/manticoresoftware/manticoresearch/commit/7b6b25f) 修复了CALL KEYWORDS中多个相同词项的词项统计。
* [提交 f381](https://github.com/manticoresoftware/manticoresearch/commit/f381ad2) 默认配置现在由Windows安装程序创建；路径不再运行时替换。
* [提交 6940](https://github.com/manticoresoftware/manticoresearch/commit/6940e95)、[提交 cc5a](https://github.com/manticoresoftware/manticoresearch/commit/cc5a480) 修复了多网络节点集群的复制问题。
* [提交 4972](https://github.com/manticoresoftware/manticoresearch/commit/49722ab) 修复了`/pq` HTTP端点为`/json/pq` HTTP端点的别名。
* [提交 3b53](https://github.com/manticoresoftware/manticoresearch/commit/3b5385a) 修复了Buddy重启时守护进程崩溃。
* [Buddy提交 fba9](https://github.com/manticoresoftware/manticoresearch-buddy/commit/fba9c8c) 在收到无效请求时显示原始错误。
* [Buddy提交 db95](https://github.com/manticoresoftware/manticoresearch-buddy/commit/db9532c) 允许备份路径中有空格，并为正则表达式添加魔法支持单引号。

## 版本 6.0.2
发布日期：2023年2月10日

### Bug修复
* [Issue #1024 crash 2](https://github.com/manticoresoftware/manticoresearch/issues/1024) Facet搜索结果较多时崩溃/段错误
* ❗[Issue #1029](https://github.com/manticoresoftware/manticoresearch/issues/1029) - 警告：编译时内置值KNOWN_CREATE_SIZE（16）小于测量值（208）。建议修正该值！
* ❗[Issue #1032](https://github.com/manticoresoftware/manticoresearch/issues/1032) - Manticore 6.0.0纯索引崩溃
* ❗[Issue #1033](https://github.com/manticoresoftware/manticoresearch/issues/1033) - 守护进程重启时多个分布式丢失
* ❗[Issue #1064](https://github.com/manticoresoftware/manticoresearch/issues/1064) - 分词器小写表中的竞态条件

## 版本 6.0.0
发布日期：2023年2月7日

从此版本开始，Manticore Search 附带了 Manticore Buddy，这是一个用 PHP 编写的 sidecar 守护进程，负责处理不需要超低延迟或高吞吐量的高级功能。Manticore Buddy 在后台运行，你甚至可能不会意识到它正在运行。虽然对最终用户来说是不可见的，但使 Manticore Buddy 便于安装并与基于 C++ 的主守护进程兼容是一项重大挑战。这个重大变动将使团队能够开发各种新的高级功能，如分片编排、访问控制和认证，以及各种集成，比如 mysqldump、DBeaver、Grafana mysql 连接器。目前它已经处理了 [SHOW QUERIES](Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES)、[BACKUP](Securing_and_compacting_a_table/Backup_and_restore.md#BACKUP-SQL-command-reference) 和 [自动模式](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema)。

本版本还包含了 130 多个错误修复和众多功能，许多功能可视为重大更新。

### 重大变更
* 🔬 实验性功能：现在可以执行与 Elasticsearch 兼容的 [insert](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md?example=Elasticsearch#Adding-documents-to-a-real-time-table) 和 [replace](Data_creation_and_modification/Updating_documents/REPLACE.md?example=Elasticsearch#REPLACE) JSON 查询，这使得 Manticore 能够与 Logstash（版本 < 7.13）、Filebeat 以及 Beats 家族的其他工具一起使用。默认启用。你可以使用 `SET GLOBAL ES_COMPAT=off` 来禁用该功能。
* 支持 [Manticore Columnar Library 2.0.0](https://github.com/manticoresoftware/columnar/) ，在[二级索引](Server_settings/Searchd.md#secondary_indexes)方面进行了诸多修复和改进。**⚠️ 重大变更**：从本版本起二级索引默认开启。如果你是从 Manticore 5 升级，请确保执行 [ALTER TABLE table_name REBUILD SECONDARY](Updating_table_schema_and_settings.md#Rebuilding-a-secondary-index)。详情见下文。
* [Commit c436](https://github.com/manticoresoftware/manticoresearch/commit/c436f9023536f767610451911955ae36d90aa638) 自动模式：现在可以跳过创建表，直接插入首条文档，Manticore 将根据文档字段自动创建表。详细说明请参见[这里](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema)。你可以使用 [searchd.auto_schema](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema) 开启或关闭该功能。
* 对[基于成本的优化器](Searching/Cost_based_optimizer.md)进行了大幅重构，在多种情况下降低了查询响应时间。
  - [Issue #1008](https://github.com/manticoresoftware/manticoresearch/issues/1008) CBO 中并行化性能估计。
  - [Issue #1014](https://github.com/manticoresoftware/manticoresearch/issues/1014) CBO 现支持识别[二级索引](Server_settings/Searchd.md#secondary_indexes)，并表现得更智能。
  - [Commit cef9](https://github.com/manticoresoftware/columnar/commit/cef9b54dc3a4ee9138ebc4df3a671715b996d6b3) 列存表/字段的编码统计信息现在存储在元数据中，帮助 CBO 做出更明智的决策。
  - [Commit 2b95](https://github.com/manticoresoftware/manticoresearch/commit/2b95fb8c2ba422c9c5a66e2b61c1f7922f31c7aa) 添加了 CBO 提示，用于微调其行为。
* [遥测](Telemetry.md#Telemetry)：我们很高兴在本版本中加入了遥测功能。该功能允许我们收集匿名且去标识化的指标，帮助我们提升产品性能和用户体验。请放心，所有收集的数据**完全匿名，不会与任何个人信息关联**。如果需要，可[轻松关闭](Telemetry.md#Telemetry)该功能。
* [Commit 5aaf](https://github.com/manticoresoftware/manticoresearch/commit/5aafb17d81b601e399e212a344a4738f65576cdd) 添加了 [ALTER TABLE table_name REBUILD SECONDARY](Updating_table_schema_and_settings.md#Rebuilding-a-secondary-index) 以便随时重建二级索引，例如：
  - 从 Manticore 5 升级到新版本时，
  - 对索引中的属性进行了 [UPDATE](Data_creation_and_modification/Updating_documents/UPDATE.md#UPDATE)（即 [原位更新，不是替换](Data_creation_and_modification/Updating_documents/REPLACE_vs_UPDATE.md)）
* [Issue #821](https://github.com/manticoresoftware/manticoresearch/issues/821) 新工具 `manticore-backup` 用于[备份和恢复 Manticore 实例](Securing_and_compacting_a_table/Backup_and_restore.md)
* SQL 命令 [BACKUP](Securing_and_compacting_a_table/Backup_and_restore.md#BACKUP-SQL-command-reference) 来执行内部备份。
* SQL 命令 [SHOW QUERIES](Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES) 方便查看运行中的查询，替代查看线程。
* [Issue #551](https://github.com/manticoresoftware/manticoresearch/issues/551) 新增 SQL 命令 `KILL` 用于终止长时间运行的 `SELECT`。
* 针对聚合查询实现动态 `max_matches` ，以提高准确度并降低响应时间。

### 次要变更
* [Issue #822](https://github.com/manticoresoftware/manticoresearch/issues/822) 新增 SQL 命令 [FREEZE/UNFREEZE](Securing_and_compacting_a_table/Freezing_a_table.md)，用于准备实时表或普通表备份。
* [Commit c470](https://github.com/manticoresoftware/manticoresearch/commit/c47052f18ad4868134de1a97e8d7da570e531dfa) 新增设置项 `accurate_aggregation` 和 `max_matches_increase_threshold` ，用于控制聚合准确度。
* [Issue #718](https://github.com/manticoresoftware/manticoresearch/issues/718) 支持有符号负64位 ID。注意，你仍然不能使用 ID > 2^63，但现在可以使用范围为 -2^63 到 0 的 ID。
* 由于我们最近添加了对二级索引的支持，导致了混淆，因为“index”可能指代二级索引、全文索引或普通/实时 `index`。为了减少混淆，我们将后者重命名为“table”。以下 SQL/命令行命令受此更改影响。它们的旧版本已弃用，但仍可使用：
  - `index <table name>` => `table <table name>`，
  - `searchd -i / --index` => `searchd -t / --table`，
  - `SHOW INDEX STATUS` => `SHOW TABLE STATUS`，
  - `SHOW INDEX SETTINGS` => `SHOW TABLE SETTINGS`，
  - `FLUSH RTINDEX` => `FLUSH TABLE`，
  - `OPTIMIZE INDEX` => `OPTIMIZE TABLE`，
  - `ATTACH TABLE plain TO RTINDEX rt` => `ATTACH TABLE plain TO TABLE rt`，
  - `RELOAD INDEX` => `RELOAD TABLE`，
  - `RELOAD INDEXES` => `RELOAD TABLES`。

  我们不打算使旧形式过时，但为了确保与文档的兼容性，建议在您的应用程序中更改名称。将在未来的版本中更改的是各种 SQL 和 JSON 命令输出中的 “index” 到 “table” 的重命名。
* 带有有状态 UDF 的查询现在强制在单线程中执行。
* [Issue #1011](https://github.com/manticoresoftware/manticoresearch/issues/1011) 重构了所有与时间调度相关的内容，作为并行块合并的前置条件。
* **⚠️ 不兼容变更**：列存储格式已更改。您需要重建那些具有列存属性的表。
* **⚠️ 不兼容变更**：二级索引文件格式已更改，因此如果您正在使用二级索引进行搜索，并且配置文件中有 `searchd.secondary_indexes = 1`，请注意新版本的 Manticore **将跳过加载包含二级索引的表**。建议采取以下措施：
  - 在升级前，将配置文件中的 `searchd.secondary_indexes` 改为 0。
  - 启动实例，Manticore 会加载这些表并发出警告。
  - 对每个索引运行 `ALTER TABLE <table name> REBUILD SECONDARY` 以重建二级索引。

  如果您正在运行复制集群，需要在所有节点上运行 `ALTER TABLE <table name> REBUILD SECONDARY`，或者按照[此说明](Securing_and_compacting_a_table/Compacting_a_table.md#Optimizing-clustered-tables)操作，只需将命令替换为运行 `ALTER .. REBUILD SECONDARY` 而不是 `OPTIMIZE`。
* **⚠️ 不兼容变更**：binlog 版本已经更新，因此之前版本生成的任何 binlog 文件将无法重放。升级过程中务必确保 Manticore Search 被干净地停止。这意味着在停止之前的实例后，除了 `binlog.meta` 之外，`/var/lib/manticore/binlog/` 目录中不应存在 binlog 文件。
* [Issue #849](https://github.com/manticoresoftware/manticoresearch/issues/849) `SHOW SETTINGS`：现在您可以在 Manticore 中查看配置文件中的设置。
* [Issue #1007](https://github.com/manticoresoftware/manticoresearch/issues/1007) [SET GLOBAL CPUSTATS=1/0](Server_settings/Setting_variables_online.md#SET) 用于打开/关闭 CPU 时间跟踪；[SHOW THREADS](Node_info_and_management/SHOW_THREADS.md) 在关闭 CPU 时间跟踪时不显示 CPU 统计信息。
* [Issue #1009](https://github.com/manticoresoftware/manticoresearch/issues/1009) RT 表的 RAM 块段现在可以在 RAM 块被刷新时合并。
* [Issue #1012](https://github.com/manticoresoftware/manticoresearch/issues/1012) 在 [indexer](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) 输出中添加了二级索引进度。
* [Issue #1013](https://github.com/manticoresoftware/manticoresearch/issues/1013) 之前，如果 Manticore 启动时无法启用某个表，它会将该表从索引列表中移除。现在新行为是保留该表以便在下次启动时尝试加载它。
* [indextool --docextract](Miscellaneous_tools.md#indextool) 返回属于请求文档的所有词和点击数。
* [Commit 2b29](https://github.com/manticoresoftware/manticoresearch/commit/2b296ee20e520b85bcbb4383e87095d31e7165dc) 环境变量 `dump_corrupt_meta` 在 searchd 无法加载索引时启用将损坏的表元数据转储到日志。
* [Commit c7a3](https://github.com/manticoresoftware/manticoresearch/commit/c7a3136eefe0eb2f69e3d99fae9c10beb5821ea6) `DEBUG META` 可以显示 `max_matches` 和伪分片统计信息。
* [Commit 6bca](https://github.com/manticoresoftware/manticoresearch/commit/6bca4848dc5c8396dbc1e057be4b9b2d579794e8) 提供了更好的错误提示，替代了混淆的 “Index header format is not json, will try it as binary...” 。
* [Commit bef3](https://github.com/manticoresoftware/lemmatizer-uk/commit/bef3ff0386d3ee87ec57619782100972c1122e47) 乌克兰词形还原路径已更改。
* [Commit 4ae7](https://github.com/manticoresoftware/manticoresearch/commit/4ae789595329a2951e194d1191ddb3121459a560) 在 [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META) 中添加了二级索引统计。
* [Commit 2e7c](https://github.com/manticoresoftware/manticoresearch/commit/2e7c585e) 现在可以使用 Swagger Editor https://manual.manticoresearch.com/Openapi#OpenAPI-specification 轻松可视化 JSON 接口。
* **⚠️ 不兼容变更**：复制协议已更改。如果您运行复制集群，升级到 Manticore 5 时需要：
  - 先干净地停止所有节点
  - 然后用 `--new-cluster` 启动最后停止的节点（在 Linux 下运行工具 `manticore_new_cluster`）。
  - 更多细节请阅读关于[重新启动集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster)的文档。

### 与 Manticore 列存库相关的变更
* 重构二级索引与列存储的集成。
* [Commit efe2](https://github.com/manticoresoftware/columnar/commit/efe26b2a) Manticore 列存储库优化，通过部分预评估 min/max 可以降低响应时间。
* [Commit 2757](https://github.com/manticoresoftware/columnar/commit/2757b99b) 如果磁盘块合并被中断，守护进程现在会清理与 MCL 相关的临时文件。
* [Commit e9c6](https://github.com/manticoresoftware/manticoresearch/commit/e9c672014b63e2b49e7a5a53880583af86c3ab4e) 崩溃时，Columnar 和次级库版本信息会被写入日志。
* [Commit f5e8](https://github.com/manticoresoftware/columnar/commit/f5e84eeb8b6cb102f13e3468246702427527fef9) 为次级索引添加了快速文档列表倒带的支持。
* [Commit 06df](https://github.com/manticoresoftware/manticoresearch/commit/06dfdd2e668933495dd6cb52d5ba2e0aa9b26e58) 类似 `select attr, count(*) from plain_index`（无过滤条件）的查询在使用 MCL 时现在更快。
* [Commit 0a76](https://github.com/manticoresoftware/manticoresearch/commit/0a76ecb9adbe886868b962dd425de5f475bbf55e) 为 HandleMysqlSelectSysvar 中的 @@autocommit 添加支持，以兼容 MySQL 8.25 以上的 .net 连接器
* **⚠️ BREAKING CHANGE**: [MCL Issue #17](https://github.com/manticoresoftware/columnar/issues/17) MCL：向列扫描添加 SSE 代码。MCL 现在至少需要 SSE4.2。

### 打包相关更改
* [Commit 4d19](https://github.com/manticoresoftware/manticoresearch/commit/4d19f5cbe49a31228aa09253f061165bfe80e51b) **⚠️ BREAKING CHANGE**：不再支持 Debian Stretch 和 Ubuntu Xenial。
* 支持 RHEL 9，包括 Centos 9、Alma Linux 9 和 Oracle Linux 9。
* [Issue #924](https://github.com/manticoresoftware/manticoresearch/issues/924) 支持 Debian Bookworm。
* [Issue #636](https://github.com/manticoresoftware/manticoresearch/issues/636) 打包：针对 Linux 和 MacOS 的 arm64 构建。
* [PR #26](https://github.com/manticoresoftware/docker/pull/26) 多架构（x86_64 / arm64）Docker 镜像。
* [为贡献者简化的包构建](Installation/Compiling_from_sources.md#Building-using-CI-Docker)。
* 现在可以使用 APT 安装指定版本。
* [Commit a6b8](https://github.com/manticoresoftware/manticoresearch/commit/51fddca5c2a3ebb8576fae4c18660656ba22de0f) 提供了 Windows 安装程序（之前仅提供压缩包）。
* 切换到使用 CLang 15 编译。
* **⚠️ BREAKING CHANGE**：自定义 Homebrew 配方，包括 Manticore Columnar Library 公式。安装 Manticore、MCL 以及其它必要组件时，请使用以下命令 `brew install manticoresoftware/manticore/manticoresearch manticoresoftware/manticore/manticore-extra`。

### Bug 修复
* [Issue #479](https://github.com/manticoresoftware/manticoresearch/issues/479) 名为 `text` 的字段
* [Issue #501](https://github.com/manticoresoftware/manticoresearch/issues/501) id 不能是非 bigint 类型
* [Issue #646](https://github.com/manticoresoftware/manticoresearch/issues/646) ALTER 操作与名为 "text" 的字段的问题
* ❗[Issue #652](https://github.com/manticoresoftware/manticoresearch/issues/652) 可能的 BUG：HTTP (JSON) 的 offset 和 limit 会影响 facet 结果
* ❗[Issue #827](https://github.com/manticoresoftware/manticoresearch/issues/827) searchd 在高负载下挂起/崩溃
* ❗[Issue #996](https://github.com/manticoresoftware/manticoresearch/issues/996) PQ 索引内存溢出
* ❗[Commit 1041](https://github.com/manticoresoftware/manticoresearch/commit/10416ef7dddf06c0d759e32ccd6ebaa2468f7cbf) 自 Sphinx 起一直损坏的 `binlog_flush = 1`，现已修复。
* [MCL Issue #14](https://github.com/manticoresoftware/columnar/issues/14) MCL：当全文字段过多时，select 导致崩溃
* [Issue #470](https://github.com/manticoresoftware/manticoresearch/issues/470) sql_joined_field 不能被存储
* [Issue #713](https://github.com/manticoresoftware/manticoresearch/issues/713) 使用 LEVENSHTEIN() 时崩溃
* [Issue #743](https://github.com/manticoresoftware/manticoresearch/issues/743) Manticore 意外崩溃，无法正常重启
* [Issue #788](https://github.com/manticoresoftware/manticoresearch/issues/788) 通过 /sql 调用 KEYWORDS 返回控制字符，导致 JSON 错误
* [Issue #789](https://github.com/manticoresoftware/manticoresearch/issues/789) mariadb 无法创建 FEDERATED 表
* [Issue #796](https://github.com/manticoresoftware/manticoresearch/issues/796) 警告：dlopen() 失败：/usr/bin/lib_manticore_columnar.so：无法打开共享对象文件：没有那个文件或目录
* [Issue #797](https://github.com/manticoresoftware/manticoresearch/issues/797) 使用 api 通过 ZONESPAN 搜索时 Manticore 崩溃
* [Issue #799](https://github.com/manticoresoftware/manticoresearch/issues/799) 使用多个索引和 facet distinct 时权重错误
* [Issue #801](https://github.com/manticoresoftware/manticoresearch/issues/801) SphinxQL 分组查询在 SQL 索引重新处理后挂起
* [Issue #802](https://github.com/manticoresoftware/manticoresearch/issues/802) MCL：Indexer 在 5.0.2 和 manticore-columnar-lib 1.15.4 中崩溃
* [Issue #813](https://github.com/manticoresoftware/manticoresearch/issues/813) Manticore 5.0.2 FEDERATED 返回空集（MySQL 8.0.28）
* [Issue #824](https://github.com/manticoresoftware/manticoresearch/issues/824) 在两个索引上使用 COUNT DISTINCT 并且结果为零时抛出内部错误
* [Issue #826](https://github.com/manticoresoftware/manticoresearch/issues/826) 删除查询导致崩溃
* [Issue #843](https://github.com/manticoresoftware/manticoresearch/issues/843) MCL：长文本字段的 Bug
* [Issue #856](https://github.com/manticoresoftware/manticoresearch/issues/856) 5.0.2 rtindex：聚合搜索限制行为不符合预期
* [Issue #863](https://github.com/manticoresoftware/manticoresearch/issues/863) 对应应返回多个结果的搜索，返回结果是 Nonetype 对象
* [Issue #870](https://github.com/manticoresoftware/manticoresearch/issues/870) 在 SELECT 表达式中使用 Attribute 和 Stored Field 时崩溃
* [Issue #872](https://github.com/manticoresoftware/manticoresearch/issues/872) 崩溃后表变为不可见
* [Issue #877](https://github.com/manticoresoftware/manticoresearch/issues/877) 搜索查询中的两个负面词语导致错误：查询不可计算
* [Issue #878](https://github.com/manticoresoftware/manticoresearch/issues/878) 通过 json query_string 使用 a -b -c 不起作用
* [Issue #886](https://github.com/manticoresoftware/manticoresearch/issues/886) 带有查询匹配的 pseudo_sharding
* [Issue #893](https://github.com/manticoresoftware/manticoresearch/issues/893) Manticore 5.0.2 中 min/max 函数没有按预期工作...
* [Issue #896](https://github.com/manticoresoftware/manticoresearch/issues/896) 字段 "weight" 解析不正确
* [Issue #897](https://github.com/manticoresoftware/manticoresearch/issues/897) Manticore 服务启动时崩溃并不断重启
* [Issue #900](https://github.com/manticoresoftware/manticoresearch/issues/900) 使用 group by j.a, smth 出现错误
* [Issue #913](https://github.com/manticoresoftware/manticoresearch/issues/913) 当在排序器中使用表达式时，查询含有两个邻近词组时 Searchd 崩溃
* [Issue #916](https://github.com/manticoresoftware/manticoresearch/issues/916) net_throttle_action 出现故障
* [Issue #919](https://github.com/manticoresoftware/manticoresearch/issues/919) MCL：Manticore 在查询执行时崩溃，且在集群恢复期间其他崩溃
* [Issue #925](https://github.com/manticoresoftware/manticoresearch/issues/925) SHOW CREATE TABLE 输出缺少反引号
* [Issue #930](https://github.com/manticoresoftware/manticoresearch/issues/930) 现可通过 JDBC 连接器从 Java 查询 Manticore
* [Issue #933](https://github.com/manticoresoftware/manticoresearch/issues/933) bm25f 排名问题
* [Issue #934](https://github.com/manticoresoftware/manticoresearch/issues/934) 无配置索引在 watchdog 首次加载时被冻结
* [Issue #937](https://github.com/manticoresoftware/manticoresearch/issues/937) 排序 Facet 数据时发生段错误
* [Issue #940](https://github.com/manticoresoftware/manticoresearch/issues/940) CONCAT(TO_STRING) 导致崩溃
* [Issue #947](https://github.com/manticoresoftware/manticoresearch/issues/947) 在某些情况下，一个简单的 select 查询可能导致整个实例停滞，无法登录或执行任何其他查询，直到该查询完成。
* [Issue #948](https://github.com/manticoresoftware/manticoresearch/issues/948) 索引器崩溃
* [Issue #950](https://github.com/manticoresoftware/manticoresearch/issues/950) facet distinct 返回错误计数
* [Issue #953](https://github.com/manticoresoftware/manticoresearch/issues/953) LCS 在内置 sph04 排序器中的计算不正确
* [Issue #955](https://github.com/manticoresoftware/manticoresearch/issues/955) 5.0.3 开发版本崩溃
* [Issue #963](https://github.com/manticoresoftware/manticoresearch/issues/963) 在 engine columnar 上使用 json 的 FACET 崩溃
* [Issue #982](https://github.com/manticoresoftware/manticoresearch/issues/982) MCL：5.0.3 从二级索引崩溃
* [PR #984](https://github.com/manticoresoftware/manticoresearch/pull/984) 在 HandleMysqlSelectSysvar 中添加 @@autocommit
* [PR #985](https://github.com/manticoresoftware/manticoresearch/pull/985) 修复 RT 索引中的线程块分配
* [Issue #985](https://github.com/manticoresoftware/manticoresearch/issues/985) 修复 RT 索引中的线程块分配
* [Issue #986](https://github.com/manticoresoftware/manticoresearch/issues/986) 默认 max_query_time 错误
* [Issue #987](https://github.com/manticoresoftware/manticoresearch/issues/987) 在多线程执行中使用正则表达式导致崩溃
* [Issue #988](https://github.com/manticoresoftware/manticoresearch/issues/988) 反向索引兼容性损坏
* [Issue #989](https://github.com/manticoresoftware/manticoresearch/issues/989) indextool 报告检查 columnar 属性时错误
* [Issue #990](https://github.com/manticoresoftware/manticoresearch/issues/990) json grouper 克隆内存泄漏
* [Issue #991](https://github.com/manticoresoftware/manticoresearch/issues/991) levenshtein 函数克隆内存泄漏
* [Issue #992](https://github.com/manticoresoftware/manticoresearch/issues/992) 加载元数据时错误信息丢失
* [Issue #993](https://github.com/manticoresoftware/manticoresearch/issues/993) 传播来自动态索引/子键和系统变量的错误
* [Issue #994](https://github.com/manticoresoftware/manticoresearch/issues/994) 在 columnar 存储中的 columnar 字符串上执行 count distinct 时崩溃
* [Issue #995](https://github.com/manticoresoftware/manticoresearch/issues/995) MCL：taxi1 的 min(pickup_datetime) 查询崩溃
* [Issue #997](https://github.com/manticoresoftware/manticoresearch/issues/997) 空的 excludes JSON 查询从选择列表中移除列
* [Issue #998](https://github.com/manticoresoftware/manticoresearch/issues/998) 当前调度器上运行的二级任务有时会导致异常副作用
* [Issue #999](https://github.com/manticoresoftware/manticoresearch/issues/999) 使用 facet distinct 并且不同架构时崩溃
* [Issue #1000](https://github.com/manticoresoftware/manticoresearch/issues/1000) MCL：未加载 columnar 库运行后，columnar rt 索引损坏
* [Issue #1001](https://github.com/manticoresoftware/manticoresearch/issues/1001) json 中隐式 cutoff 无效
* [Issue #1002](https://github.com/manticoresoftware/manticoresearch/issues/1002) columnar grouper 问题
* [Issue #1003](https://github.com/manticoresoftware/manticoresearch/issues/1003) 无法删除索引中的最后一个字段
* [Issue #1004](https://github.com/manticoresoftware/manticoresearch/issues/1004) --new-cluster 后行为异常
* [Issue #1005](https://github.com/manticoresoftware/manticoresearch/issues/1005) “columnar library not loaded”，但其实不需要
* [Issue #1006](https://github.com/manticoresoftware/manticoresearch/issues/1006) 删除查询无错误提示
* [Issue #1010](https://github.com/manticoresoftware/manticoresearch/issues/1010) 修正 Windows 构建中 ICU 数据文件的位置
* [PR #1018](https://github.com/manticoresoftware/manticoresearch/pull/1018) 握手发送问题
* [Issue #1020](https://github.com/manticoresoftware/manticoresearch/issues/1020) 在 show create table 中显示 id
* [Issue #1024 crash 1](https://github.com/manticoresoftware/manticoresearch/issues/1024) Facet 搜索大量结果时崩溃 / 段错误。
* [Issue #1026](https://github.com/manticoresoftware/manticoresearch/issues/1026) 实时索引：当插入大量文档且 RAM chunk 满时，searchd “卡死”不动
* [Commit 4739](https://github.com/manticoresoftware/manticoresearch/commit/4739dafd) 当复制在节点间繁忙时，线程在关闭时卡住
* [Commit ab87](https://github.com/manticoresoftware/manticoresearch/commit/ab87836782e7ae43fe5f9dea739eed0d336b42c1) 在 JSON 范围过滤器中混合使用浮点数和整数会导致 Manticore 忽略该过滤器
* [Commit d001](https://github.com/manticoresoftware/manticoresearch/commit/d00101c2905f6393ce3fca23c4f6dcb2506f4bd9) JSON 中的浮点过滤器不准确
* [Commit 4092](https://github.com/manticoresoftware/manticoresearch/commit/4092c6a8) 如果索引被修改，丢弃未提交的事务（否则可能崩溃）
* [Commit 9692](https://github.com/manticoresoftware/manticoresearch/commit/9692a840) 使用反斜杠时的查询语法错误
* [Commit 0c19](https://github.com/manticoresoftware/manticoresearch/commit/0c19ad8d) SHOW STATUS 中的 workers_clients 可能错误
* [Commit 1772](https://github.com/manticoresoftware/manticoresearch/commit/17720e1e2f2ec38337df565bee78f7ce9d9c744a) 修复合并无文档存储的内存段时崩溃
* [Commit f45b](https://github.com/manticoresoftware/manticoresearch/commit/f45b337b5c265433bb7d6262ad65d02f65edbaf3) 修正了 equals JSON 过滤器缺失的 ALL/ANY 条件
* [Commit 3e83](https://github.com/manticoresoftware/manticoresearch/commit/3e83c1f0c0e6e7a9d4297113f69a112cbabd9e64) 如果 searchd 从一个无法写入的目录启动，复制可能失败，出现 `got exception while reading ist stream: mkstemp(./gmb_pF6TJi) failed: 13 (Permission denied)`
* [Commit 92e5](https://github.com/manticoresoftware/manticoresearch/commit/92e5629f7150f62524b385fca670a01ba9590f5b) 自 4.0.2 起崩溃日志仅包含偏移量。本提交对此进行修正。

## 版本 5.0.2
发布于：2022年5月30日

### 修复
* ❗[Issue #791](https://github.com/manticoresoftware/manticoresearch/issues/791) - 错误的栈大小可能导致崩溃。

## 版本 5.0.0
发布于：2022年5月18日


### 主要新特性
* 🔬 支持 [Manticore Columnar Library 1.15.2](https://github.com/manticoresoftware/columnar/)，开启了[二级索引](Server_settings/Searchd.md#secondary_indexes)测试版功能。对于普通和实时列式及行式索引（如果使用了 [Manticore Columnar Library](https://github.com/manticoresoftware/columnar)），默认开启二级索引构建，但要启用搜索功能，需在配置文件中或通过 [SET GLOBAL](Server_settings/Setting_variables_online.md) 设置 `secondary_indexes = 1`。新功能支持所有操作系统，除了旧的 Debian Stretch 和 Ubuntu Xenial。
* [只读模式](Security/Read_only.md)：现在可以指定只处理读查询、忽略任何写操作的监听器。
* 新增 [/cli](Connecting_to_the_server/HTTP.md#/cli) 端点，使通过 HTTP 运行 SQL 查询更加简单。
* 通过 HTTP JSON 实现更快的批量 INSERT/REPLACE/DELETE：之前可以通过 HTTP JSON 协议提供多个写命令，但它们是逐个处理的，现在作为单一事务进行处理。
* [#720](https://github.com/manticoresoftware/manticoresearch/issues/720) JSON 协议支持[嵌套过滤器](Searching/Filters.md#Nested-bool-query)；之前 JSON 中无法写出类似 `a=1 and (b=2 or c=3)` 的表达式：`must`（与）、`should`（或）、`must_not`（非）仅在最高层生效。现在支持嵌套。
* 支持 HTTP 协议中的[分块传输编码](https://en.wikipedia.org/wiki/Chunked_transfer_encoding)。应用程序现在可使用分块传输来发送大批量数据，减少资源消耗（无需计算 `Content-Length`）。服务器端，Manticore 始终以流式方式处理传入的 HTTP 数据，无需等待整个批次传输完毕，从而：
  - 降低峰值内存使用，减少内存不足风险
  - 缩短响应时间（测试显示处理 100MB 批次时时间减少约 11%）
  - 可突破 [max_packet_size](Server_settings/Searchd.md#max_packet_size) 限制，传输超过最大允许的 128MB 批次，例如一次 1GB。
* [#719](https://github.com/manticoresoftware/manticoresearch/issues/719) HTTP 接口支持 `100 Continue`：现在可以直接通过 `curl`（以及各种语言的 curl 库）传输大批次数据，这些工具默认发送 `Expect: 100-continue`，先等待然后才发送数据。之前需添加空 `Expect:` 头，现在不再需要。

  <details>

  之前（注意响应时间）：

  ```bash
  $ time curl -v -sX POST http://localhost:9318/bulk -H "Content-Type: application/x-ndjson" --data '{"insert": {"index": "user", "doc":  {"name":"Prof. Matt Heaney IV","email":"ibergnaum@yahoo.com","description":"Tempora ullam eaque consequatur. Vero aut minima ut et ut omnis officiis vel. Molestiae quis voluptatum sint numquam.","age":15,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"Prof. Boyd McKenzie","email":"carlotta11@hotmail.com","description":"Blanditiis maiores odio corporis eaque illum. Aut et rerum iste. Neque et ullam quisquam officia dignissimos quo cumque.","age":84,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"约翰·史密斯先生","email":"stiedemann.tristin@ziemann.com","description":"临时事件伟大地实现了所有的实现并逃离。债务出生的痛苦是真实的时间消失。这里没有腐败的人。","age":89,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"赫克托·普罗斯","email":"hickle.mafalda@hotmail.com","description":"作为发明权的某些方面。某些逃逸的偏爱是反对恨和智慧的。","age":64,"active":1}}}'
  *   尝试连接127.0.0.1...
  * 已连接到localhost (127.0.0.1) 端口9318 (#0)
  > POST /bulk HTTP/1.1
  > Host: localhost:9318
  > User-Agent: curl/7.47.0
  > Accept: */*
  > Content-Type: application/x-ndjson
  > Content-Length: 1025
  > Expect: 100-continue
  >
  * 等待100-continue完成
  * 上传完全完成，状态良好
  < HTTP/1.1 200 OK
  < Server: 4.2.0 15e927b@211223 release (columnar 1.11.4 327b3d4@211223)
  < Content-Type: application/json; charset=UTF-8
  < Content-Length: 434
  <
  * 与localhost的连接#0保持不变
  {"items":[{"insert":{"table":"user","_id":2811798918248005633,"created":true,"result":"created","status":201}},{"insert":{"table":"user","_id":2811798918248005634,"created":true,"result":"created","status":201}},{"insert":{"table":"user","_id":2811798918248005635,"created":true,"result":"created","status":201}},{"insert":{"table":"user","_id":2811798918248005636,"created":true,"result":"created","status":201}}],"errors":false}
  real	0m1.022s
  user	0m0.001s
  sys	0m0.010s
  ```

  现今：
  ```bash
  $ time curl -v -sX POST http://localhost:9318/bulk -H "Content-Type: application/x-ndjson" --data '{"insert": {"index": "user", "doc":  {"name":"马特·希尼教授 IV","email":"ibergnaum@yahoo.com","description":"暂时整体有效。真实的低限和全部职务。痛苦让欲望永远存在。","age":15,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"博伊德·麦肯齐教授","email":"carlotta11@hotmail.com","description":"更大憎恨的体魄光辉。任意事物。无法和解的尊严。","age":84,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"约翰·史密斯先生","email":"stiedemann.tristin@ziemann.com","description":"临时事件伟大地实现了所有的实现并逃离。债务出生的痛苦是真实的时间消失。这里没有腐败的人。","age":89,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"赫克托·普罗斯","email":"hickle.mafalda@hotmail.com","description":"作为发明权的某些方面。某些逃逸的偏爱是反对恨和智慧的。","age":64,"active":1}}}'
  *   尝试连接127.0.0.1...
  * 已连接到localhost (127.0.0.1) 端口9318 (#0)
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
  * 上传完全完成，状态良好
  < HTTP/1.1 200 OK
  < Server: 4.2.1 63e5749@220405 dev
  < Content-Type: application/json; charset=UTF-8
  < Content-Length: 147
  <
  * 与localhost的连接#0保持不变
  {"items":[{"bulk":{"table":"user","_id":2811798919590182916,"created":4,"deleted":0,"updated":0,"result":"created","status":201}}],"errors":false}
  real	0m0.015s
  user	0m0.005s
  sys	0m0.004s
  ```

  </details>

* **⚠️ 重大变更**：[伪分片](Server_settings/Searchd.md#pseudo_sharding)默认启用。如果你想禁用它，确保在Manticore配置文件的`searchd`部分添加`pseudo_sharding = 0`。
* 真实时间/普通索引中至少有一个全文字段不是必需的。你现在可以在不涉及全文搜索的情况下使用Manticore。
* 通过[Manticore列式库](https://github.com/manticoresoftware/columnar)支持的属性实现了[快速提取](Creating_a_table/Data_types.md#fast_fetch)：查询如`select * from <columnar table>`比以前快得多，尤其是在架构中有很多字段时。
* **⚠️ 重大变更**：隐式[截止](Searching/Options.md#cutoff)。Manticore现在不再花费时间和资源处理你不需要出现在结果集中的数据。缺点是影响了[SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META)中的`total_found`和JSON输出中的[hits.total](Searching/Full_text_matching/Basic_usage.md#HTTP-JSON)。只有当你看到`total_relation: eq`时才准确，而`total_relation: gte`意味着匹配文档的实际数量超过你获得的`total_found`值。要保持以前的行为，你可以使用搜索选项`cutoff=0`，这会使`total_relation`始终为`eq`。
* **⚠️ 重大变更**：所有全文字段现在默认[存储](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#stored_fields)。你需要使用`stored_fields = `（空值）使所有字段非存储（即恢复以前的行为）。
* [#715](https://github.com/manticoresoftware/manticoresearch/issues/715) HTTP JSON支持[搜索选项](Searching/Options.md#General-syntax)。

### 小变更
* **⚠️ 重大变更**：索引元文件格式更改。以前元文件（`.meta`，`.sph`）是二进制格式，现在是json格式。新的Manticore版本会自动转换旧索引，但：
  - 你可能会收到警告如`WARNING: ... syntax error, unexpected TOK_IDENT`
  - 你将无法用以前的Manticore版本运行该索引，请确保你有备份
* **⚠️ 重大变更**：通过[HTTP keep-alive](Connecting_to_the_server/HTTP.md#Keep-alive)支持会话状态。这使得当客户端也支持时，HTTP变为有状态。例如，使用新的[/cli](Connecting_to_the_server/HTTP.md#/cli)端点和HTTP keep-alive（所有浏览器默认开启）时，可以在`SELECT`之后调用`SHOW META`，其行为将与通过mysql访问相同。注意，以前也支持`Connection: keep-alive` HTTP头，但它只是导致复用相同连接。从此版本开始，该功能也使会话成为有状态。
* 现在可以指定`columnar_attrs = *`来在[plain模式](Read_this_first.md#Real-time-mode-vs-plain-mode)下将所有属性定义为列式，这在列表较长时很有用。
* 更快的复制SST。
* **⚠️ 重大变更**：复制协议已更改。如果您运行复制集群，升级到Manticore 5时需要：
  - 首先干净地停止所有节点
  - 然后用`--new-cluster`启动最后停止的节点（Linux下运行工具`manticore_new_cluster`）
  - 更多细节请阅读[重启集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster)。
* 复制改进：
  - 更快的SST
  - 噪声抵抗，有助于处理复制节点间网络不稳定的情况
  - 改进日志记录
* 安全改进：若配置中未指定`listen`，Manticore现在监听`127.0.0.1`而非`0.0.0.0`。虽然Manticore Search默认配置中通常会指定`listen`，且无`listen`配置的情况较少见，但仍有可能存在。以前Manticore监听`0.0.0.0`，安全性较差；现在监听`127.0.0.1`，通常不会暴露到互联网。
* 更快的列式属性聚合。
* `AVG()`精度提升：之前Manticore内部用于聚合的是`float`，现改为使用`double`，显著提升精度。
* 改进对JDBC MySQL驱动的支持。
* 对[jemalloc](https://github.com/jemalloc/jemalloc)支持`DEBUG malloc_stats`。
* [optimize_cutoff](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#optimize_cutoff)现可作为每表设置，在CREATE或ALTER表时设定。
* **⚠️ 重大变更**：[query_log_format](Server_settings/Searchd.md#query_log_format)现默认值为**`sphinxql`**。如果习惯使用`plain`格式，需要在配置文件中添加`query_log_format = plain`。
* 显著降低内存消耗：在使用存储字段的情况下，Manticore在长时间大量插入/替换/优化的工作负载下消耗的RAM明显减少。
* [shutdown_timeout](Server_settings/Searchd.md#shutdown_timeout)的默认值从3秒提高到60秒。
* [Commit ffd0](https://github.com/manticoresoftware/manticoresearch/commit/ffd0499d329d2c383f14c8a44c4cc84338ab56e7) 支持Java mysql connector >= 6.0.3：在[Java mysql connection 6.0.3](https://mvnrepository.com/artifact/mysql/mysql-connector-java/6.0.3)中，它们更改了连接mysql的方式，导致与Manticore不兼容。现在支持新行为。
* [Commit 1da6](https://github.com/manticoresoftware/manticoresearch/commit/1da6dbec) 禁止在加载索引（例如searchd启动时）时保存新磁盘块。
* [Issue #746](https://github.com/manticoresoftware/manticoresearch/issues/746) 支持glibc >= 2.34。
* [Issue #784](https://github.com/manticoresoftware/manticoresearch/issues/784) 将‘VIP’连接与普通连接（非VIP）区分计数。以前VIP连接计入`max_connections`限制，可能导致非VIP连接出现“maxed out”错误。现在VIP连接不计入限制。当前VIP连接数可在`SHOW STATUS`和`status`中查看。
* [ID](Creating_a_table/Data_types.md#Document-ID)现在可显式指定。
* [Issue #687](https://github.com/manticoresoftware/manticoresearch/issues/687) 支持mysql协议的zstd压缩。

### ⚠️ 其他小的重大变更
* ⚠️ BM25F公式略有更新以提升搜索相关性。仅影响使用函数[BM25F()](Functions/Searching_and_ranking_functions.md#BM25F%28%29)的搜索结果，不改变默认排名公式行为。
* ⚠️ 更改REST [/sql](Connecting_to_the_server/HTTP.md#mode=raw)端点行为：`/sql?mode=raw`现在需要转义且返回数组。
* ⚠️ `/bulk`的INSERT/REPLACE/DELETE请求响应格式更改：
  - 之前每个子查询构成独立事务，返回独立响应
  - 现在整个批处理视为单一事务，返回单个响应
* ⚠️ 搜索选项`low_priority`和`boolean_simplify`现在需要赋值（`0/1`）：以前可写`SELECT ... OPTION low_priority, boolean_simplify`，现在需写`SELECT ... OPTION low_priority=1, boolean_simplify=1`。
* ⚠️ 如果使用旧版[php](https://github.com/manticoresoftware/manticoresearch/blob/master/api/sphinxapi.php)、[python](https://github.com/manticoresoftware/manticoresearch/blob/master/api/sphinxapi.py)或[java](https://github.com/manticoresoftware/manticoresearch/tree/master/api/java)客户端，请访问相应链接获取更新版本。**旧版本与Manticore 5不完全兼容。**
* ⚠️ HTTP JSON请求在`query_log_format=sphinxql`模式下以不同格式记录。之前只记录全文部分，现在原样记录。

### 新的软件包
* **⚠️ 重大变更**：因结构调整，升级到Manticore 5时，建议先删除旧软件包，再安装新版本：
  - 基于RPM的系统：`yum remove manticore*`
  - Debian和Ubuntu：`apt remove manticore*`
* 新的 deb/rpm 包结构。之前的版本提供了：
  - `manticore-server`，包含 `searchd`（主搜索守护进程）及其所需所有内容
  - `manticore-tools`，包含 `indexer` 和 `indextool`
  - `manticore`，包含所有内容
  - `manticore-all` RPM 作为元包，引用了 `manticore-server` 和 `manticore-tools`

  新结构为：
  - `manticore` - deb/rpm 元包，安装所有上述内容作为依赖项
  - `manticore-server-core` - 包含 `searchd` 及运行其所需所有内容
  - `manticore-server` - systemd 文件及其他辅助脚本
  - `manticore-tools` - `indexer`、`indextool` 和其他工具
  - `manticore-common` - 默认配置文件、默认数据目录、默认停用词
  - `manticore-icudata`、`manticore-dev`、`manticore-converter` 改动不大
  - 包含所有包的 `.tgz` 包
* 支持 Ubuntu Jammy
* 通过 [YUM 仓库](Installation/RHEL_and_Centos.md#YUM-repository) 支持 Amazon Linux 2

### Bug 修复
* [问题 #815](https://github.com/manticoresoftware/manticoresearch/issues/815) 使用 UDF 函数时随机崩溃
* [问题 #287](https://github.com/manticoresoftware/manticoresearch/issues/287) RT 索引时内存溢出
* [问题 #604](https://github.com/manticoresoftware/manticoresearch/issues/604) 3.6.0，4.2.0 sphinxql-parser 破坏性变更
* [问题 #667](https://github.com/manticoresoftware/manticoresearch/issues/667) FATAL：内存溢出（无法分配 9007199254740992 字节）
* [问题 #676](https://github.com/manticoresoftware/manticoresearch/issues/676) 字符串未正确传递给 UDF
* ❗[问题 #698](https://github.com/manticoresoftware/manticoresearch/issues/698) 尝试向 rt 索引添加文本列后 searchd 崩溃
* [问题 #705](https://github.com/manticoresoftware/manticoresearch/issues/705) Indexer 找不到所有列
* ❗[问题 #709](https://github.com/manticoresoftware/manticoresearch/issues/705) 按 json.boolean 分组工作不正确
* [问题 #716](https://github.com/manticoresoftware/manticoresearch/issues/716) indextool 相关的索引命令（例如 --dumpdict）失败
* ❗[问题 #724](https://github.com/manticoresoftware/manticoresearch/issues/724) 字段从选择中消失
* [问题 #727](https://github.com/manticoresoftware/manticoresearch/issues/727) .NET HttpClient 使用 `application/x-ndjson` 时 Content-Type 不兼容
* [问题 #729](https://github.com/manticoresoftware/manticoresearch/issues/729) 字段长度计算
* ❗[问题 #730](https://github.com/manticoresoftware/manticoresearch/issues/730) 创建/插入到/删除 columnar 表存在内存泄漏
* [问题 #731](https://github.com/manticoresoftware/manticoresearch/issues/731) 在某些条件下结果中出现空列
* ❗[问题 #749](https://github.com/manticoresoftware/manticoresearch/issues/749) 守护进程启动时崩溃
* ❗[问题 #750](https://github.com/manticoresoftware/manticoresearch/issues/750) 守护进程启动时挂起
* ❗[问题 #751](https://github.com/manticoresoftware/manticoresearch/issues/751) SST 期间崩溃
* [问题 #752](https://github.com/manticoresoftware/manticoresearch/issues/752) 当 engine='columnar' 时 json 属性标记为 columnar
* [问题 #753](https://github.com/manticoresoftware/manticoresearch/issues/753) 复制侦听在 0 端口
* [问题 #754](https://github.com/manticoresoftware/manticoresearch/issues/754) columnar_attrs = * 在 csvpipe 中不起作用
* ❗[问题 #755](https://github.com/manticoresoftware/manticoresearch/issues/755) rt 中在 columnar 选择 float 时崩溃
* ❗[问题 #756](https://github.com/manticoresoftware/manticoresearch/issues/756) Indextool 检查期间更改 rt 索引
* [问题 #757](https://github.com/manticoresoftware/manticoresearch/issues/757) 需要检查监听端口范围交叉
* [问题 #758](https://github.com/manticoresoftware/manticoresearch/issues/758) RT 索引保存磁盘块失败时记录原始错误
* [问题 #759](https://github.com/manticoresoftware/manticoresearch/issues/759) RE2 配置只报告一个错误
* ❗[问题 #760](https://github.com/manticoresoftware/manticoresearch/issues/760) 提交 5463778558586d2508697fa82e71d657ac36510f 中内存消耗变化
* [问题 #761](https://github.com/manticoresoftware/manticoresearch/issues/761) 第 3 个节点在脏重启后未构成非主集群
* [问题 #762](https://github.com/manticoresoftware/manticoresearch/issues/762) 更新计数器增加了 2
* [问题 #763](https://github.com/manticoresoftware/manticoresearch/issues/763) 4.2.1 版本用 4.2.0 创建带形态学的索引损坏
* [问题 #764](https://github.com/manticoresoftware/manticoresearch/issues/764) json key 中无转义 /sql?mode=raw
* ❗[问题 #765](https://github.com/manticoresoftware/manticoresearch/issues/765) 使用函数隐藏其他值
* ❗[问题 #766](https://github.com/manticoresoftware/manticoresearch/issues/766) FixupAttrForNetwork 中一行代码触发内存泄漏
* ❗[问题 #767](https://github.com/manticoresoftware/manticoresearch/issues/767) 4.2.0 和 4.2.1 中 docstore 缓存相关内存泄漏
* [问题 #768](https://github.com/manticoresoftware/manticoresearch/issues/768) 存储字段网络传输异常
* [问题 #769](https://github.com/manticoresoftware/manticoresearch/issues/769) lemmatizer_base 如果未在 'common' 部分提及则重置为空
* [问题 #770](https://github.com/manticoresoftware/manticoresearch/issues/770) pseudo_sharding 导致按 id 查询变慢
* [问题 #771](https://github.com/manticoresoftware/manticoresearch/issues/771) 使用 jemalloc 时 DEBUG malloc_stats 输出为零
* [问题 #772](https://github.com/manticoresoftware/manticoresearch/issues/772) 删除/添加列使值不可见
* [问题 #773](https://github.com/manticoresoftware/manticoresearch/issues/773) 无法向 columnar 表添加 bit(N) 列
* [问题 #774](https://github.com/manticoresoftware/manticoresearch/issues/774) manticore.json 中启动时 "cluster" 为空
* ❗[提交 1da4](https://github.com/manticoresoftware/manticoresearch/commits/1da4ce891e4ff13727cfd3331d72771c64ee7949) HTTP 操作在 SHOW STATUS 中未被跟踪
* [提交 3810](https://github.com/manticoresoftware/manticoresearch/commits/381000ab4af4c3c98ec8c730699ad7f39039cec8) 禁用低频单关键词查询的 pseudo_sharding
* [提交 8003](https://github.com/manticoresoftware/manticoresearch/commits/800325cca283a87801b28929d82420fafc76a0ee) 修正了存储属性与索引合并的问题
* [提交 cddf](https://github.com/manticoresoftware/manticoresearch/commits/cddfeed6296f2041c6aae18414e5cd9cabf08281) 通用化了去重值获取器；为列式字符串新增了专用去重获取器
* [提交 fba4](https://github.com/manticoresoftware/manticoresearch/commits/fba4bb4f) 修复了从文档存储中获取空整数属性的问题
* [提交 f300](https://github.com/manticoresoftware/manticoresearch/commit/f3009a9242fa16956adf871c4887e6e0303ba364) 查询日志中 `ranker` 可能被指定两次

## 版本 4.2.0，2021年12月23日

### 主要新特性
* **对实时索引和全文查询的伪分片支持**。在上一个版本中我们添加了有限的伪分片支持。从此版本开始，您只需启用[searchd.pseudo_sharding](Server_settings/Searchd.md#pseudo_sharding)，即可充分利用伪分片和多核处理器带来的所有好处。最棒的是，您无需对索引或查询做任何改动，只需启用它，如果有闲置 CPU，系统会自动利用，以降低响应时间。它支持纯索引和实时索引的全文、过滤及分析查询。例如，启用伪分片后，在[Hacker news 精选评论数据集](https://zenodo.org/record/45901/)（原始数据乘以100，即含1.16亿文档的纯索引）上，大多数查询的**平均响应时间降低大约10倍**。

![伪分片开启 vs 关闭于 4.2.0](4.2.0_ps_on_vs_off.png)

* 现支持[**Debian Bullseye**](https://manticoresearch.com/install/)。

<!-- example pq_transactions_4.2.0 -->
* PQ 事务现为原子且隔离的。之前 PQ 事务支持有限，新的改进使得**REPLACE 插入 PQ**尤其快，尤其是在一次替换大量规则时。性能细节：

<!-- intro -->
#### 4.0.2

<!-- request 4.0.2 -->

插入 100 万条 PQ 规则需要 **48 秒**，而以 1 万条批次替换 4 万条规则需要 **406 秒**。

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

插入 100 万条 PQ 规则需要 **34 秒**，以 1 万条批次替换时仅需 **23 秒**。

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

### 次要改动
* 现可在 `searchd` 配置节中使用 [optimize_cutoff](Server_settings/Searchd.md#optimize_cutoff) 选项。此选项有助于在所有索引中全局限制 RT 分区数量。
* [提交 0087](https://github.com/manticoresoftware/manticoresearch/commit/00874743823eaf43e2a09a088392fd7e0a617f47) 准确支持多个具有相同字段集合/顺序的本地物理索引（实时/纯索引）上的 [count(distinct ...)](Searching/Grouping.md#COUNT%28DISTINCT-field%29) 和 [FACET ... distinct](Searching/Faceted_search.md#Faceting-without-duplicates)。
* [PR #598](https://github.com/manticoresoftware/manticoresearch/pull/598) 为 `YEAR()` 及其他时间戳函数添加 bigint 支持。
* [提交 8e85](https://github.com/manticoresoftware/manticoresearch/commit/8e85d4bce596cc88f9d9615ea243a27e4afde0f7) 自适应 [rt_mem_limit](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_mem_limit)。此前 Manticore Search 收集的数据量精确到 `rt_mem_limit`，然后保存为磁盘块；保存时还会多收集最多 10%（即双缓冲）数据，防止插入暂停。当极限被突破时，新增文档被阻断，直到磁盘块完全保存完毕。新自适应限额基于现有的 [auto-optimize](Server_settings/Searchd.md#auto_optimize)，允许磁盘块提前刷新，稍微不完全遵守 `rt_mem_limit`。现在我们收集最多占 `rt_mem_limit` 50% 数据作为一个磁盘块。保存时，统计已保存文档数和保存期间新抵达文档数，计算一个比率，用于下次收集限额。例如，已保存 9000 万文档，保存时又新到 1000 万文档，比例为 90%，意味着下次收集最多占 `rt_mem_limit` 的 90% 后开始刷新。该比例自动计算范围在 33.3% 至 95% 之间。
* [Issue #628](https://github.com/manticoresoftware/manticoresearch/issues/628) PostgreSQL 源支持 [unpack_zlib](Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Database_connection.md#unpack_zlib)。感谢 [Dmitry Voronin](https://github.com/dimv36) 的[贡献](https://github.com/manticoresoftware/manticoresearch/pull/630)。
* [提交 6d54](https://github.com/manticoresoftware/manticoresearch/commit/6d54cf2b319b131970c29410cc21abcbdf8083b1) 支持 `indexer -v` 和 `--version` 参数。此前虽可查看 indexer 版本，但 `-v`/`--version` 不被支持。
* [Issue #662](https://github.com/manticoresoftware/manticoresearch/issues/662) 当 Manticore 通过 systemd 启动时，默认无限制 mlock 限额。
* [提交 63c8](https://github.com/manticoresoftware/manticoresearch/commit/63c8cd05cf399e705c2c14939411a92cb784735a) 旋转锁改为协程读写锁的操作队列。
* [提交 4113](https://github.com/manticoresoftware/manticoresearch/commit/41130ce30d1734bbf8e6b8b63c4e815caf9464f2) 新增环境变量 `MANTICORE_TRACK_RT_ERRORS`，便于调试实时分段损坏问题。

### 重大变更
* Binlog 版本已升级，之前版本的 binlog 不会被回放，因此升级时请确保干净地停止 Manticore Search：停止之前的实例后，在 `/var/lib/manticore/binlog/` 中不应有除 `binlog.meta` 之外的 binlog 文件。
* [提交 3f65](https://github.com/manticoresoftware/manticoresearch/commit/3f659f36e87d99ee262205a8eae4663a255be630) `show threads option format=all` 中新增列“chain”。它显示一些任务信息票据的堆栈，主要用于分析需求，因此如果你解析 `show threads` 输出，请注意这一新列。
* `searchd.workers` 自 3.5.0 起被废弃，现在处于不推荐使用状态，如果配置文件里还包含它，启动时会触发警告。Manticore Search 会启动，但会显示警告。
* 如果你使用 PHP 和 PDO 访问 Manticore，则需要开启 `PDO::ATTR_EMULATE_PREPARES`

### Bug 修复
* ❗[问题 #650](https://github.com/manticoresoftware/manticoresearch/issues/650) Manticore 4.0.2 比 Manticore 3.6.3 慢。4.0.2 在批量插入方面比之前版本更快，但单文档插入明显更慢。该问题已在 4.2.0 修复。
* ❗[提交 22f4](https://github.com/manticoresoftware/manticoresearch/commit/22f4141bed1411c83684ae6e0bae832071cce3df) RT 索引在高强度 REPLACE 负载下可能损坏，或导致崩溃。
* [提交 03be](https://github.com/manticoresoftware/manticoresearch/commit/03be91e4f720a3f20bee0278dd8f0d44abd3ef0e) 修复 groupers 和 group N sorter 合并时的平均值；修复聚合合并。
* [提交 2ea5](https://github.com/manticoresoftware/manticoresearch/commit/2ea575d34efe38661f14d858f2b552456f29fe42) `indextool --check` 可能崩溃。
* [提交 7ec7](https://github.com/manticoresoftware/manticoresearch/commit/7ec76d4abf01b5fca6f10a0000fe64d5d8e5fa52) UPDATE 导致的内存耗尽问题。
* [提交 658a](https://github.com/manticoresoftware/manticoresearch/commit/658a727e41f3c3fbcde9754149528f34ed76ba19) 守护进程在 INSERT 时可能挂起。
* [提交 46e4](https://github.com/manticoresoftware/manticoresearch/commit/46e42b9b386e31e53247ed634e158b533e28063e) 守护进程在关闭时可能挂起。
* [提交 f8d7](https://github.com/manticoresoftware/manticoresearch/commit/f8d7d5177cc83d250b493b4cfc619732b29cf072) 守护进程关闭时可能崩溃。
* [提交 733a](https://github.com/manticoresoftware/manticoresearch/commit/733accf165142a1d1574689525ccca8907971158) 守护进程崩溃时可能挂起。
* [提交 f7f8](https://github.com/manticoresoftware/manticoresearch/commit/f7f8bd8c0dbb23a59a88502dcba16b0918fb2330) 守护进程启动时尝试使用无效节点列表重新加入集群可能崩溃。
* [提交 1401](https://github.com/manticoresoftware/manticoresearch/commit/140155613821ecaddcd1a3dfc4f7369b0df6aef7) 分布式索引在 RT 模式下如果启动时无法解析某个代理，可能会被完全遗忘。
* [问题 #683](https://github.com/manticoresoftware/manticoresearch/issues/683) 属性 bit(N) engine='columnar' 失败。
* [问题 #682](https://github.com/manticoresoftware/manticoresearch/issues/682) 创建表失败，但留下目录。
* [问题 #663](https://github.com/manticoresoftware/manticoresearch/issues/663) 配置失败，未知键名 'attr_update_reserve'。
* [问题 #632](https://github.com/manticoresoftware/manticoresearch/issues/632) Manticore 在批量查询时崩溃。
* [问题 #679](https://github.com/manticoresoftware/manticoresearch/issues/679) v4.0.3 批量查询再次导致崩溃。
* [提交 f7f8](https://github.com/manticoresoftware/manticoresearch/commit/f7f8bd8c0dbb23a59a88502dcba16b0918fb2330) 修复启动时尝试使用无效节点列表重新加入集群导致守护进程崩溃。
* [问题 #643](https://github.com/manticoresoftware/manticoresearch/issues/643) Manticore 4.0.2 在批量插入后不接受连接。
* [问题 #635](https://github.com/manticoresoftware/manticoresearch/issues/635) FACET 查询中使用 ORDER BY JSON.field 或字符串属性时可能崩溃。
* [问题 #634](https://github.com/manticoresoftware/manticoresearch/issues/634) 使用 packedfactors 查询时崩溃 SIGSEGV。
* [提交 4165](https://github.com/manticoresoftware/manticoresearch/commits/41657f15f8fcf1e93cfad7f835bcf705d6e8349e) create table 不支持 morphology_skip_fields 的问题。

## 版本 4.0.2，2021年9月21日

### 主要新特性
- **全面支持 [Manticore Columnar Library](https://github.com/manticoresoftware/columnar)**。此前 Manticore Columnar Library 仅支持普通索引。现在支持：
  - 实时索引的 `INSERT`、`REPLACE`、`DELETE`、`OPTIMIZE`
  - 复制
  - `ALTER`
  - `indextool --check`
- **自动索引压缩**（[问题 #478](https://github.com/manticoresoftware/manticoresearch/issues/478)）。现在你无需手动或通过定时任务等自动化方式调用 OPTIMIZE，Manticore 会自动且默认执行该操作。你可通过全局变量 [optimize_cutoff](Server_settings/Setting_variables_online.md) 设置默认压缩阈值。
- **块快照和锁系统改造**。这些改变初看可能察觉不明显，但极大改善了实时索引中许多操作的行为。简言之，以往大多数 Manticore 数据操作依赖大量锁，如今改用磁盘块快照。
- **极大提升实时索引的批量 INSERT 性能**。例如在 [Hetzner 的服务器 AX101](https://www.hetzner.com/dedicated-rootserver/ax101) 上，使用 SSD、128 GB 内存和 AMD Ryzen™ 9 5950X（16 核 × 2 线程），**3.6.0 版本可以达到每秒插入 236K 文档**，表结构为 `name text, email string, description text, age int, active bit(1)` （默认 `rt_mem_limit`，批次大小 25000，16 并发插入工人，总计插入 1600 万文档）。在 4.0.2 中，保持相同的并发、批次和总数量，性能提升至 **每秒插入 357K 文档**。

  <details>

  - 读取操作（如 SELECT、复制）使用快照进行
  - 只更改内部索引结构而不修改模式/文档的操作（例如合并RAM段、保存磁盘块、合并磁盘块）使用只读快照执行，最终替换现有块
  - UPDATE 和 DELETE 针对现有块执行，但如果正在进行合并，则写入操作会被收集，然后应用到新块
  - UPDATE 对每个块依次获取独占锁。合并在进入从块收集属性阶段时获取共享锁。因此同时只有一个（合并或更新）操作可以访问块的属性。
  - 当合并进入需要属性的阶段时会设置一个特殊标志。UPDATE 完成时检查此标志，如果设置，则整个更新存储在特殊集合中。最终，当合并完成时，将更新应用到新生成的磁盘块。
  - ALTER 通过独占锁运行
  - 复制作为常规读取操作运行，但另外会在 SST 之前保存属性，并在 SST 期间禁止更新

  </details>

- **[ALTER](Updating_table_schema_and_settings.md) 可以添加/删除全文字段**（在 RT 模式）。之前只能添加/删除一个属性。
- 🔬 **实验性：针对全扫描查询的伪分片** - 允许并行化任何非全文搜索查询。不再需要手动准备分片，只需启用新选项 [searchd.pseudo_sharding](Server_settings/Searchd.md#pseudo_sharding)，即可期望对非全文搜索查询的响应时间减少至最多 CPU 核心数的水平。注意它很容易占满所有 CPU 核心，因此如果您不仅关心延迟，还关心吞吐量，请谨慎使用。

### 小改动
<!-- example -->
- 通过 [APT 仓库](Installation/Debian_and_Ubuntu.md#APT-repository) 支持 Linux Mint 和 Ubuntu Hirsute Hippo
- 在某些情况下，通过 HTTP 以 id 进行大索引中的更快更新（依赖于 id 分布）
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
- [systemd 的自定义启动标志](Starting_the_server/Linux.md#Custom-startup-flags-using-systemd)。现在如果需要用某些特定启动标志运行 Manticore，您无需手动启动 searchd
- 新函数 [LEVENSHTEIN()](Functions/String_functions.md#LEVENSHTEIN%28%29)，计算 Levenshtein 距离
- 添加新的 [searchd 启动标志](Starting_the_server/Manually.md#searchd-command-line-options) `--replay-flags=ignore-trx-errors` 和 `--replay-flags=ignore-all-errors`，以便在 binlog 损坏时仍可启动 searchd
- [Issue #621](https://github.com/manticoresoftware/manticoresearch/issues/621) - 公开 RE2 错误
- 针对由本地纯索引组成的分布式索引，更准确的 [COUNT(DISTINCT)](Searching/Grouping.md#COUNT%28DISTINCT-field%29)
- [FACET DISTINCT](Searching/Faceted_search.md#Faceting-without-duplicates) 用于在做分面搜索时去除重复项
- [exact form 修饰符](Searching/Full_text_matching/Operators.md#Exact-form-modifier) 现在不依赖 [morphology](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology)，并适用于启用 [infix/prefix](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) 搜索的索引

### 重大改动
- 新版本可以读取旧索引，但旧版本无法读取 Manticore 4 的索引
- 移除了隐式的按 id 排序。如需排序请显式指定
- `charset_table` 默认值从 `0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451` 改为 `non_cjk`
- `OPTIMIZE` 自动执行。如果不需要，请确保在配置文件的 `searchd` 部分设置 `auto_optimize=0`
- [Issue #616](https://github.com/manticoresoftware/manticoresearch/issues/616) `ondisk_attrs_default` 已弃用，现已移除
- 对贡献者：我们现在 Linux 构建采用 Clang 编译器，根据测试它能构建出更快的 Manticore Search 和 Manticore Columnar Library
- 如果在搜索查询中未指定 [max_matches](Searching/Options.md#max_matches)，则会隐式更新到性能所需的最低值，以适配新的列存储。这可能会影响 [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META) 中的总数指标 `total`，但不会影响实际找到文档数 `total_found`。

### 从 Manticore 3 迁移
- 确保已干净停止 Manticore 3：
  - `/var/lib/manticore/binlog/` 中不应有 binlog 文件（目录中仅应有 `binlog.meta`）
  - 否则 Manticore 4 可能无法重放这些 binlog，无法启动相应索引
- 新版本可以读取旧索引，但旧版本不能读取 Manticore 4 索引，请确保做好备份，以方便回滚
- 如果运行复制集群，确保您：
  - 先干净停止所有节点
  - 然后用 `--new-cluster` 启动最后停止的节点（Linux 运行工具 `manticore_new_cluster`）
  - 更多细节请参考 [重启集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster)

### Bug 修复
- 修复大量复制相关问题：
  - [Commit 696f](https://github.com/manticoresoftware/manticoresearch/commit/696f8649535f2b5285b878c9581c0d2b1139ae09) - 修复 joiner 上带活动索引的 SST 期间崩溃；添加 joiner 节点写入文件块时 sha1 校验以加快索引加载；添加 joiner 节点索引加载时变更索引文件的轮转；添加 joiner 节点收到 donor 节点新索引替换活动索引时删除索引文件功能；为 donor 节点发送文件和块添加复制日志点
  - [Commit b296](https://github.com/manticoresoftware/manticoresearch/commit/b296c55af68314a8fc66f8b104e7301d673c3b68) - 当地址不正确时，在 JOIN CLUSTER 出现崩溃
  - [Commit 418b](https://github.com/manticoresoftware/manticoresearch/commit/418bf880a5e05ffbb68f311a1c23006a9df3220e) - 在大索引的初始复制过程中，加入节点可能失败并显示 `ERROR 1064 (42000): invalid GTID, (null)`，捐赠者节点在另外一个节点加入时可能变得无响应
  - [Commit 6fd3](https://github.com/manticoresoftware/manticoresearch/commit/6fd350d28e4905211cf8a7298cbf7cdda101c83d) - 大索引的哈希计算可能错误，导致复制失败
  - [Issue #615](https://github.com/manticoresoftware/manticoresearch/issues/615) - 集群重启时复制失败
- [Issue #574](https://github.com/manticoresoftware/manticoresearch/issues/574) - `indextool --help` 没有显示参数 `--rotate`
- [Issue #578](https://github.com/manticoresoftware/manticoresearch/issues/578) - searchd 在闲置约一天后CPU高占用
- [Issue #587](https://github.com/manticoresoftware/manticoresearch/issues/587) - 立即刷新 .meta 文件
- [Issue #617](https://github.com/manticoresoftware/manticoresearch/issues/617) - manticore.json 文件被清空
- [Issue #618](https://github.com/manticoresoftware/manticoresearch/issues/618) - root 用户下 searchd --stopwait 失败。此修复也改进了 systemctl 行为（之前 ExecStop 显示失败且等待时间不足，导致 searchd 无法正常停止）
- [Issue #619](https://github.com/manticoresoftware/manticoresearch/issues/619) - INSERT/REPLACE/DELETE 与 SHOW STATUS。`command_insert`、`command_replace` 等显示了错误的指标
- [Issue #620](https://github.com/manticoresoftware/manticoresearch/issues/620) - 纯索引的 `charset_table` 默认值错误
- [Commit 8f75](https://github.com/manticoresoftware/manticoresearch/commit/8f7536887d4b58a5bab5647fc067b2d0482c2038) - 新磁盘块未被 mlocked
- [Issue #607](https://github.com/manticoresoftware/manticoresearch/issues/607) - 当无法通过名称解析节点时，Manticore 集群节点崩溃
- [Issue #623](https://github.com/manticoresoftware/manticoresearch/issues/623) - 更新索引的复制可能导致未定义状态
- [Commit ca03](https://github.com/manticoresoftware/manticoresearch/commit/ca03d2280c5197e9b311f0f03f551e3b702a8130) - 索引器在索引带有 json 属性的纯索引源时可能挂起
- [Commit 53c7](https://github.com/manticoresoftware/manticoresearch/commit/53c753054558ff3f82c7af365273d32537c5338b) - 修复 PQ 索引的非等值表达式过滤
- [Commit ccf9](https://github.com/manticoresoftware/manticoresearch/commit/ccf94e022ba6763d6286ab5bac767b6f6a846846) - 修正列表查询中超过1000条匹配的分页窗口问题。之前 `SELECT * FROM pq ORDER BY id desc LIMIT 1000 , 100 OPTION max_matches=1100` 无法工作
- [Commit a048](https://github.com/manticoresoftware/manticoresearch/commit/a0483fe9bef3c45dffe96f5df4f4382aee29c072) - 对 Manticore 发起 HTTPS 请求时可能出现类似 "max packet size(8388608) exceeded" 的警告
- [Issue #648](https://github.com/manticoresoftware/manticoresearch/issues/648) - Manticore 3 在对字符串属性进行几次更新后可能挂起


## 版本 3.6.0，2021年5月3日
**Manticore 4 之前的维护版本**

### 主要新功能
- 支持纯索引的 [Manticore Columnar Library](https://github.com/manticoresoftware/columnar/)。新增纯索引设置项 [columnar_attrs](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#columnar_attrs)
- 支持 [乌克兰语词形还原器（Ukrainian Lemmatizer）](https://github.com/manticoresoftware/lemmatizer-uk)
- 完全修订的直方图。在构建索引时，Manticore 也会为其中的每个字段构建直方图，用于更快的过滤。在3.6.0版本中该算法经过完全修订，对于大量数据和大量过滤操作能获得更高的性能。

### 小改动
- 工具 `manticore_new_cluster [--force]`，便于通过 systemd 重启复制集群
- `indexer --merge` 支持 [--drop-src](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Merging_tables.md#Merging-tables)
- 新模式 [blend_mode='trim_all'](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology)
- 增加对使用反引号转义 JSON 路径的支持 [support for escaping JSON path](Searching/Full_text_matching/Escaping.md#Escaping-json-node-names-in-SQL)
- [indextool --check](Miscellaneous_tools.md#indextool) 可在 RT 模式下工作
- SELECT/UPDATE 支持 [FORCE/IGNORE INDEX(id)](Data_creation_and_modification/Updating_documents/UPDATE.md#Updates-via-SQL)
- 合并磁盘块的块 ID 现为唯一
- 新增 [indextool --check-disk-chunk CHUNK_NAME](Miscellaneous_tools.md#indextool)

### 优化
- [更快的 JSON 解析](https://github.com/manticoresoftware/manticoresearch/commit/0c25fc1788e3f77f60cb7fb8f1623d8466bba606)，我们的测试显示在类似 `WHERE json.a = 1` 的查询中，延迟降低了 3-4%
- 新增非文档化命令 `DEBUG SPLIT`，为自动分片/负载均衡做准备

### 修复的错误
- [Issue #584](https://github.com/manticoresoftware/manticoresearch/issues/524) - 不准确且不稳定的 FACET 结果
- [Issue #506](https://github.com/manticoresoftware/manticoresearch/issues/506) - 使用 MATCH 时出现异常行为，受影响的用户需重建索引，因为问题发生在索引构建阶段
- [Issue #387](https://github.com/manticoresoftware/manticoresearch/issues/387) - 运行包含 SNIPPET() 函数的查询时偶发核心转储
- 栈优化，提升复杂查询处理能力：
  - [Issue #469](https://github.com/manticoresoftware/manticoresearch/issues/469) - SELECT 导致崩溃转储
  - [e8420cc7](https://github.com/manticoresoftware/manticoresearch/tree/e8420cc789149c84731be0874be916e33b263bd6) - 过滤器树的栈大小检测
- [Issue #461](https://github.com/manticoresoftware/manticoresearch/issues/461) - 使用 IN 条件的更新操作未能正确生效
- [Issue #464](https://github.com/manticoresoftware/manticoresearch/issues/464) - 在 CALL PQ 之后立即执行 SHOW STATUS 返回 - [Issue #481](https://github.com/manticoresoftware/manticoresearch/issues/481) - 修复静态二进制构建问题
- [Issue #502](https://github.com/manticoresoftware/manticoresearch/issues/502) - 多查询中的错误
- [Issue #514](https://github.com/manticoresoftware/manticoresearch/issues/514) - 使用 'create table' 时无法使用非常规的列名
- [Commit d1db](https://github.com/manticoresoftware/manticoresearch/commit/d1dbe771288328c963ca8512f564c6b7cf4f466c) - 当重放包含字符串属性更新的 binlog 时守护进程崩溃；将 binlog 版本设置为 10
- [Commit 775d](https://github.com/manticoresoftware/manticoresearch/commit/775d0555562f121911d3c30085947e743fadac2d)  - 修复表达式栈帧检测运行时错误（测试 207）
- [Commit 4795](https://github.com/manticoresoftware/manticoresearch/commit/4795dc49194c7745761bb5006ecb266e8a9224fa)  - 空存储查询的 percolate 索引过滤器和标签为空（测试 369）
- [Commit c3f0](https://github.com/manticoresoftware/manticoresearch/commit/c3f0bf4dca0e4eae6ea899dd566878df57eba085)  - 在网络延迟长且错误率高的环境（跨数据中心复制）中，复制 SST 流中断；升级复制命令版本至 1.03
- [Commit ba2d](https://github.com/manticoresoftware/manticoresearch/commit/ba2d6619b8107f3fb177741bbdff3709bac248cd)  - 集群加入后写操作时加入器锁定集群（测试 385）
- [Commit de4d](https://github.com/manticoresoftware/manticoresearch/commit/de4dcb9ff6c316bc2a9ef6c62eb64769f04ee103)  - 使用精确修饰符的通配符匹配问题（测试 321）
- [Commit 6524](https://github.com/manticoresoftware/manticoresearch/commit/6524fc6af032a0932bd42c37874a619e1fb7f075)  - docid 检查点与文档存储问题
- [Commit f4ab](https://github.com/manticoresoftware/manticoresearch/commit/f4ab83c291e65a5873d271c67b5491f629544f1d)  - 解析无效 XML 时索引器表现不一致
- [Commit 7b72](https://github.com/manticoresoftware/manticoresearch/commit/7b727e22fe01b1a180d4a8d3a3fea92c18eac707)  - 存储的包含 NOTNEAR 的 percolate 查询无限运行（测试 349）
- [Commit 812d](https://github.com/manticoresoftware/manticoresearch/commit/812dab745f0c972ecde5d5b3d4a313cd6e1e143e)  - 以通配符开头的短语权重错误
- [Commit 1771](https://github.com/manticoresoftware/manticoresearch/commit/1771afc669c75670b3511bf02ab9bfc8982b94d2)  - 含通配符的 percolate 查询在匹配时生成无 payload 的词项导致命中交错并破坏匹配（测试 417）
- [Commit aa0d](https://github.com/manticoresoftware/manticoresearch/commit/aa0d8c2bcb41428d41dbfb66e42a872a79a4ee4b)  - 修复并行查询情况下的 ‘total’ 计算
- [Commit 18d8](https://github.com/manticoresoftware/manticoresearch/commit/18d81b3cfbb930b7361dae645fe13283d4fbf6ef)  - Windows 下守护进程多会话时崩溃
- [Commit 8443](https://github.com/manticoresoftware/manticoresearch/commit/84432f23d6fbf7a7d0b67c8c9412eb5933b03596)  - 部分索引设置无法复制
- [Commit 9341](https://github.com/manticoresoftware/manticoresearch/commit/93411fe6af4246ca4acce758944d513a1660b92b)  - 高频添加新事件时 netloop 有时冻结，因原子 ‘kick’ 事件被为多个事件处理一次，导致丢失实际动作
查询的状态，而非服务器状态
- [Commit d805](https://github.com/manticoresoftware/manticoresearch/commit/d805fc125244ef08d14fb3d1826e2c356862d539)  - 新刷新磁盘块在提交时可能丢失
- [Commit 63cb](https://github.com/manticoresoftware/manticoresearch/commit/63cbf008d50979cdf7062664fb02a672a3bb1131)  - profiler 中 'net_read' 不准确
- [Commit f537](https://github.com/manticoresoftware/manticoresearch/commit/f5379bb27234b25919966c50ccb6f469ead646e2)  - Percolate 与阿拉伯语（从右向左文本）的问题
- [Commit 49ee](https://github.com/manticoresoftware/manticoresearch/commit/49eeb4202d4ce4875c07c35087d7108c1e177328)  - 重复列名时 ID 选择不正确
- [Commit refa](https://github.com/manticoresoftware/manticoresearch/commit/4b7df262bbfec9b9e95a44e72ad326ac8a786c49) 对网络事件的修复以解决少见情况下崩溃
- [e8420cc7](https://github.com/manticoresoftware/manticoresearch/tree/e8420cc789149c84731be0874be916e33b263bd6) 修复 `indextool --dumpheader`
- [Commit ff71](https://github.com/manticoresoftware/manticoresearch/commit/ff7163532ed39042c0d3c3c2b902ff41f94d7f0c) - 带有 RECONFIGURE 的 TRUNCATE 存储字段执行错误

### 重大变更：
- 新的 binlog 格式：升级前需要干净地停止 Manticore
- 索引格式略有变化：新版可正常读取现有索引，但如果你决定从 3.6.0 降级到旧版本，较新的索引将无法读取
- 复制格式更改：不要从旧版本复制到 3.6.0 或反之，必须在所有节点同时切换到新版本
- `reverse_scan` 已废弃。确保自 3.6.0 起你的查询中不再使用此选项，否则将会失败
- 本次发布后，不再提供 RHEL6、Debian Jessie 和 Ubuntu Trusty 的构建版本。如有紧急需求支持，[请联系我们](https://manticoresearch.com/contact-us/)

### 弃用说明
- 不再隐式按 ID 排序。若依赖此功能请相应更新查询
- 搜索选项 `reverse_scan` 已被废弃

## 版本 3.5.4，2020年12月10日

### 新功能
- 新的 Python、Javascript 和 Java 客户端现已普遍可用，并在本手册中有良好文档说明。
- 实时索引的磁盘块自动丢弃。该优化使得在对实时索引进行[OPTIMIZE](Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE)操作时，可以在明显不再需要该块（所有文档已被抑制）时自动丢弃磁盘块。之前它仍然需要合并，现在该磁盘块可以立即丢弃。[cutoff](Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks)选项被忽略，即使没有实际合并，过时的磁盘块也会被移除。这对于在索引中维护保留策略并删除较旧文档的情况非常有用。现在对这类索引进行压缩会更快。
- 作为 SELECT 的选项，[standalone NOT](Searching/Options.md#not_terms_only_allowed) 支持

### 小改动
- [问题 #453](https://github.com/manticoresoftware/manticoresearch/issues/453) 新增选项 [indexer.ignore_non_plain=1](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#ignore_non_plain)，当运行 `indexer --all` 且配置文件中包含不仅仅是普通索引时，此选项非常有用。若未设置 `ignore_non_plain=1`，将会收到警告及相应退出代码。
- [SHOW PLAN ... OPTION format=dot](Node_info_and_management/Profiling/Query_plan.md#Dot-format-for-SHOW-PLAN) 和 [EXPLAIN QUERY ... OPTION format=dot](Searching/Full_text_matching/Profiling.md#Profiling-without-running-a-query) 支持全文查询计划执行的可视化。对于理解复杂查询非常有帮助。

### 弃用
- `indexer --verbose` 被弃用，因为它从未对 indexer 输出增加任何内容。
- 用于转储监视器回溯的信号由 `USR1` 改为 `USR2`。

### Bug修复
- [问题 #423](https://github.com/manticoresoftware/manticoresearch/issues/423) 西里尔字符号召片段模式不高亮的问题。
- [问题 #435](https://github.com/manticoresoftware/manticoresearch/issues/435) RTINDEX - GROUP N BY 表达式选择导致致命崩溃。
- [提交 2b3b](https://github.com/manticoresoftware/manticoresearch/commit/2b3b62bd0ce2b54544733d524a169e81f71b5388) 集群中搜索守护进程状态显示段错误。
- [提交 9dd2](https://github.com/manticoresoftware/manticoresearch/commit/9dd25c193ed2a53750a0e0e68566fd2b833fcb0c) 'SHOW INDEX index.N SETTINGS' 无法显示 >9 的块。
- [问题 #389](https://github.com/manticoresoftware/manticoresearch/issues/389) 导致 Manticore 崩溃的错误。
- [提交 fba1](https://github.com/manticoresoftware/manticoresearch/commit/fba166178ff35c73a0c70d131cf3aca4a635e961) 转换器生成损坏的索引。
- [提交 eecd](https://github.com/manticoresoftware/manticoresearch/commit/eecd61d861c742471175be19a5e5455034b5db5f) stopword_step=0 与 CALL SNIPPETS() 的问题。
- [提交 ea68](https://github.com/manticoresoftware/manticoresearch/commit/ea6850e449601199190f38a387c04b8a7b7e90f6) 在本地索引中低 max_matches下 count distinct 返回 0 的问题。
- [提交 362f](https://github.com/manticoresoftware/manticoresearch/commit/362f27db3421f09a41c6cce04b5baace00fa4c4d) 使用聚合时，命中中不返回存储文本。


## 版本 3.5.2，2020年10月1日

### 新特性

* OPTIMIZE 将磁盘块缩减到一定数量（默认是 `2* CPU核心数`），而非单一块。最优的块数量可通过 [cutoff](Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks) 选项控制。
* NOT 操作符现在可以单独使用。默认情况下禁用，因为意外的单独 NOT 查询可能速度较慢。可通过设置新的 searchd 指令 [not_terms_only_allowed](Server_settings/Searchd.md#not_terms_only_allowed) 为 `0` 启用。
* 新设置 [max_threads_per_query](Server_settings/Searchd.md#max_threads_per_query) 用于设置查询可用线程数。若未设置，则查询线程数最多可达 [threads](Server_settings/Searchd.md#threads) 的值。
每个 `SELECT` 查询的线程数可通过 [OPTION threads=N](Searching/Options.md#threads) 覆盖全局 `max_threads_per_query`。
* 现在可以使用 [IMPORT TABLE](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Importing_table.md) 导入 Percolate 索引。
* HTTP API `/search` 新增对基本[分面](Searching/Faceted_search.md#HTTP-JSON)/[分组](Searching/Grouping.md)的支持，使用新的查询节点 `aggs`。

### 小改动

* 如果未声明复制监听指令，引擎将尝试使用在定义的 'sphinx' 端口之后的端口，最多到 200。
* `listen=...:sphinx` 需要显式设置用于 SphinxSE 连接或 SphinxAPI 客户端。
* [SHOW INDEX STATUS](Node_info_and_management/Table_settings_and_status/SHOW_TABLE_STATUS.md) 输出新增指标：`killed_documents`、`killed_rate`、`disk_mapped_doclists`、`disk_mapped_cached_doclists`、`disk_mapped_hitlists` 及 `disk_mapped_cached_hitlists`。
* SQL 命令 `status` 现在输出 `Queue\Threads` 和 `Tasks\Threads`。

### 弃用：

* `dist_threads` 现已完全弃用，searchd 使用该指令时会记录警告。

### Docker

官方 Docker 镜像现基于 Ubuntu 20.04 LTS。

### 打包

除了常规的 `manticore` 软件包外，还可以按组件安装 Manticore Search：

- `manticore-server-core` - 提供 `searchd`、手册页、日志目录、API 和 galera 模块。此包会自动安装作为依赖的 `manticore-common`。
- `manticore-server` - 提供核心自动化脚本（init.d、systemd）和 `manticore_new_cluster` 包装器。此包会自动安装作为依赖的 `manticore-server-core`。
- `manticore-common` - 提供配置、停用词、通用文档及骨架文件夹（数据目录、模块等）。
- `manticore-tools` - 提供辅助工具（`indexer`、`indextool` 等），相应手册页和示例。此包会自动安装作为依赖的 `manticore-common`。
- `manticore-icudata`（RPM）或 `manticore-icudata-65l`（DEB）- 提供 icu 形态学使用的 ICU 数据文件。
- `manticore-devel`（RPM）或 `manticore-dev`（DEB） - 提供 UDF 的开发头文件。

### Bug 修复

1. [Commit 2a47](https://github.com/manticoresoftware/manticoresearch/commit/2a474dc1a26e8b0f8aaaae95669caf2f1d4b7746) RT 索引中不同块的 grouper 守护进程崩溃
2. [Commit 57a1](https://github.com/manticoresoftware/manticoresearch/commit/57a19e5ad5663ef0ca7436595218fb1221d28c8e) 空远程文档的快速路径
3. [Commit 07dd](https://github.com/manticoresoftware/manticoresearch/commit/07dd3f313c63fb82c22092f9907ef24e3475250e) 表达式堆栈帧检测运行时
4. [Commit 08ae](https://github.com/manticoresoftware/manticoresearch/commit/08ae357cf1012bc8e2da54c20b205b592efda3d4) percolate 索引中匹配超过 32 个字段
5. [Commit 16b9](https://github.com/manticoresoftware/manticoresearch/commit/16b9390fd4cdb07a77ac4497adb935573a1710e5) 复制监听端口范围
6. [Commit 5fa6](https://github.com/manticoresoftware/manticoresearch/commit/5fa671affeacb6441a59c8a88479bfd423df7c81) 显示 pq 上的创建表语句
7. [Commit 54d1](https://github.com/manticoresoftware/manticoresearch/commit/54d133b6449105a9fb0168db3f1fbb05fb5aa1f6) HTTPS 端口行为
8. [Commit fdbb](https://github.com/manticoresoftware/manticoresearch/commit/fdbbe5245cc296cc5c1ae3ae2fb9cb08fb66a248) 替换时文档存储行混合
9. [Commit afb5](https://github.com/manticoresoftware/manticoresearch/commit/afb53f648ef4e64fa1776c58e66f6e716ac730ad) 将 TFO 不可用消息级别切换为“信息”
10. [Commit 59d9](https://github.com/manticoresoftware/manticoresearch/commit/59d94cefc6e88af360b8046726a16b7eaa9f0b71) strcmp 使用无效时崩溃
11. [Commit 04af](https://github.com/manticoresoftware/manticoresearch/commit/04af0349c1e784c2230d204592d6272b440bb375) 将索引添加到包含系统（停用词）文件的集群中
12. [Commit 5014](https://github.com/manticoresoftware/manticoresearch/commit/50148b4eae9f3a924082005cf5d2f9c8339af7f5) 合并具有大型字典的索引；RT 优化大型磁盘块
13. [Commit a2ad](https://github.com/manticoresoftware/manticoresearch/commit/a2adf1582843f63d4dcc1dab8f7598e0917094a2) Indextool 可以从当前版本转储元数据
14. [Commit 69f6](https://github.com/manticoresoftware/manticoresearch/commit/69f6d5f77c64b97e3f395aea8c555b11c77fb732) GROUP N 中分组排序问题
15. [Commit 24d5](https://github.com/manticoresoftware/manticoresearch/commit/24d5d80f508576794b751d70f9f9b524564e3c14) 握手后显式刷新 SphinxSE
16. [Commit 31c4](https://github.com/manticoresoftware/manticoresearch/commit/31c4d78ac03ee85d9a02b86075448a04ac80f9e2) 避免不必要时复制巨大描述
17. [Commit 2959](https://github.com/manticoresoftware/manticoresearch/commit/2959e2caa8610f98fcdf2e5a82160f58687f4a27) show threads 负时间
18. [Commit f0b3](https://github.com/manticoresoftware/manticoresearch/commit/f0b357102c70f3ac1d7da260c60b67322940e647) 令牌过滤插件与零位置增量
19. [Commit a49e](https://github.com/manticoresoftware/manticoresearch/commit/a49e5bc13ea763e6f01dd5993e891858a84d1846) 多重命中时将“FAIL”改为“WARNING”

## 版本 3.5.0，2020 年 7 月 22 日

### 主要新特性：
* 这次发布耗时很长，因为我们致力于将多任务模式从线程改为**协程**。这使配置更简单，查询并行处理更加直接：Manticore 只使用给定数量的线程（见新设置 [threads](Server_settings/Searchd.md#threads)），新模式确保以最优方式执行。
* 在[高亮显示](Searching/Highlighting.md#Highlighting-options)中更改：
  - 任何支持多字段的高亮（`highlight({},'field1, field2'` 或 JSON 查询中的 `highlight`）默认现在按字段应用限制。
  - 任何普通文本型高亮（`highlight({}, string_attr)` 或 `snippet()`）现在对整个文档应用限制。
  - 可通过 `limits_per_field=0` 选项（默认 `1`）将[字段级限制](Searching/Highlighting.md#limits_per_field)切换为全局限制。
  - [allow_empty](Searching/Highlighting.md#allow_empty) 现在通过 HTTP JSON 的高亮默认值为 `0`。

* 同一端口[现在可以用于](Server_settings/Searchd.md#listen) http、https 和二进制 API（接收来自远程 Manticore 实例的连接）。连接通过 mysql 协议仍需 `listen = *:mysql`。Manticore 现在自动检测尝试连接的客户端类型，MySQL 除外（协议限制所致）。

* 在 RT 模式中，字段现在可以同时是[文本和字符串属性](Creating_a_table/Data_types.md#String) — [GitHub issue #331](https://github.com/manticoresoftware/manticoresearch/issues/331)。

  在[普通模式](Read_this_first.md#Real-time-mode-vs-plain-mode)下称为 `sql_field_string`。现在也适用于[RT 模式](Read_this_first.md#Real-time-mode-vs-plain-mode)的实时索引。使用示例如下：

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
* 现在可以[高亮字符串属性](Searching/Highlighting.md#Highlighting-via-SQL)。
* SQL 接口支持 SSL 和压缩
* 支持 mysql 客户端 [`status`](Node_info_and_management/Node_status.md#STATUS) 命令。
* [复制](Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Setting-up-replication) 现在可复制外部文件（停用词、例外等）。
* 过滤操作符 [`in`](Searching/Filters.md#Set-filters) 现已通过 HTTP JSON 接口可用。
* HTTP JSON 中的 [`expressions`](Searching/Expressions.md#expressions)。
* [现在可以在 RT 模式下动态更改 `rt_mem_limit`](https://github.com/manticoresoftware/manticoresearch/issues/344)，即可以执行 `ALTER ... rt_mem_limit=<new value>`。
* 现在可以使用 [单独的 CJK 字符集表](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology)：`chinese`、`japanese` 和 `korean`。
* [thread_stack](Server_settings/Searchd.md#thread_stack) 现在限制最大线程栈，而非初始大小。
* 改进了 `SHOW THREADS` 输出。
* 在 `SHOW THREADS` 中显示长时间 `CALL PQ` 的进度。
* cpustat、iostat、coredump 可在运行时通过 [SET](Server_settings/Setting_variables_online.md#SET) 更改。
* 实现了 `SET [GLOBAL] wait_timeout=NUM` 。

### 重大变更：
* **索引格式已更改。** 在 3.5.0 版本构建的索引无法被低于 3.5.0 版本的 Manticore 加载，但 Manticore 3.5.0 能理解旧格式。
* [`INSERT INTO PQ VALUES()`](Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md)（即未提供列列表）之前期望的值为 `(query, tags)`，现在变更为 `(id,query,tags,filters)`。如果希望自动生成 id，可以将 id 设为 0。
* [`allow_empty=0`](Searching/Highlighting.md#allow_empty) 是通过 HTTP JSON 接口高亮的新默认设置。
* 在 `CREATE TABLE`/`ALTER TABLE` 中，外部文件（如停用词、例外词等）只允许使用绝对路径。

### 废弃事项：
* `ram_chunks_count` 在 `SHOW INDEX STATUS` 中重命名为 `ram_chunk_segments_count`。
* `workers` 已废弃。现在只存在一种 workers 模式。
* `dist_threads` 已废弃。所有查询现在尽可能并行（受到 `threads` 和 `jobs_queue_size` 限制）。
* `max_children` 已废弃。请使用 [threads](Server_settings/Searchd.md#threads) 设置 Manticore 使用的线程数（默认设置为 CPU 核心数）。
* `queue_max_length` 已废弃。如确实需要，请使用 [jobs_queue_size](Server_settings/Searchd.md#jobs_queue_size) 精细调整内部作业队列大小（默认无限制）。
* 所有 `/json/*` 端点现在可直接访问，无需 `/json/` 前缀，例如 `/search`、`/insert`、`/delete`、`/pq` 等。
* `field` （意指“全文字段”）在 `describe` 中已重命名为 “text”。
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
* 西里尔字母 `и` 不再映射为 `i`，在默认的 `non_cjk` charset_table 中，因为这影响了俄语词干提取和词形还原。
* `read_timeout`。应使用同时控制读写的 [network_timeout](Server_settings/Searchd.md#network_timeout)。


### 软件包

* Ubuntu Focal 20.04 官方软件包
* deb 软件包名从 `manticore-bin` 改为 `manticore`

### Bug 修复：
1. [问题 #351](https://github.com/manticoresoftware/manticoresearch/issues/351) searchd 内存泄露
2. [提交 ceab](https://github.com/manticoresoftware/manticoresearch/commit/ceabe44f) 片段读取越界小问题
3. [提交 1c3e](https://github.com/manticoresoftware/manticoresearch/commit/1c3e84a3) 危险的局部变量写入导致崩溃
4. [提交 26e0](https://github.com/manticoresoftware/manticoresearch/commit/26e094ab) 测试 226 中 sorter 轻微内存泄漏
5. [提交 d2c7](https://github.com/manticoresoftware/manticoresearch/commit/d2c7f86a) 测试 226 中严重内存泄漏
6. [提交 0dd8](https://github.com/manticoresoftware/manticoresearch/commit/0dd80122) 集群显示节点同步，但 `count(*)` 显示数量不同
7. [提交 f1c1](https://github.com/manticoresoftware/manticoresearch/commit/f1c1ac3f) 美观：日志中警告消息重复且有时丢失
8. [提交 f1c1](https://github.com/manticoresoftware/manticoresearch/commit/f1c1ac3f) 美观：日志中的 (null) 索引名
9. [提交 359d](https://github.com/manticoresoftware/manticoresearch/commit/359dbd30) 无法检索超过 7000 万条结果
10. [提交 19f3](https://github.com/manticoresoftware/manticoresearch/commit/19f328ee) 不能使用无列语法插入 PQ 规则
11. [提交 bf68](https://github.com/manticoresoftware/manticoresearch/commit/bf685d5d) 向集群索引插入文档时错误信息误导
12. [提交 2cf1](https://github.com/manticoresoftware/manticoresearch/commit/2cf18c83) `/json/replace` 和 `json/update` 返回的 id 以指数形式显示
13. [问题 #324](https://github.com/manticoresoftware/manticoresearch/issues/324) 在同一查询中更新 json 标量属性和 mva
14. [提交 d384](https://github.com/manticoresoftware/manticoresearch/commit/d38409eb) `hitless_words` 在 RT 模式下无效
15. [提交 5813](https://github.com/manticoresoftware/manticoresearch/commit/5813d639) RT 模式下应禁止执行 `ALTER RECONFIGURE`
16. [提交 5813](https://github.com/manticoresoftware/manticoresearch/commit/5813d639) searchd 重启后 `rt_mem_limit` 被重置为 128M
17. highlight() 有时会挂起
18. [提交 7cd8](https://github.com/manticoresoftware/manticoresearch/commit/7cd878f4) RT 模式下无法使用 U+ 代码
19. [提交 2b21](https://github.com/manticoresoftware/manticoresearch/commit/2b213de4) RT 模式下词形变体中的通配符使用失败
20. [提交 e9d0](https://github.com/manticoresoftware/manticoresearch/commit/e9d07e68) 修正了 `SHOW CREATE TABLE` 与多个词形变体文件的兼容问题
21. [提交 fc90](https://github.com/manticoresoftware/manticoresearch/commit/fc90a84f) 没有 "query" 的 JSON 查询导致 searchd 崩溃
22. Manticore [官方 docker](https://hub.docker.com/r/manticoresearch/manticore) 无法从 mysql 8 索引
23. [提交 23e0](https://github.com/manticoresoftware/manticoresearch/commit/23e05d32) HTTP /json/insert 需要 id
24. [提交 bd67](https://github.com/manticoresoftware/manticoresearch/commit/bd679af0) `SHOW CREATE TABLE` 对 PQ 无效
25. [提交 bd67](https://github.com/manticoresoftware/manticoresearch/commit/bd679af0) `CREATE TABLE LIKE` 对 PQ 不能正常工作
26. [提交 5eac](https://github.com/manticoresoftware/manticoresearch/commit/5eacf28f) `show index status` 设置中的行尾符问题
27. [提交 cb15](https://github.com/manticoresoftware/manticoresearch/commit/cb153228) HTTP JSON 响应中 "highlight" 的标题为空
28. [问题 #318](https://github.com/manticoresoftware/manticoresearch/issues/318) `CREATE TABLE LIKE` 中缀错误
29. [提交 9040](https://github.com/manticoresoftware/manticoresearch/commit/9040d22c) RT 在负载下崩溃
30. [cd512c7d ](https://github.com/manticoresoftware/manticoresearch/commit/cd512c7d ) RT 磁盘块崩溃时丢失崩溃日志
31. [问题 #323](https://github.com/manticoresoftware/manticoresearch/issues/323) 导入表失败并关闭连接
32. [提交 6275](https://github.com/manticoresoftware/manticoresearch/commit/6275316a) `ALTER reconfigure` 损坏 PQ 索引
33. [提交 9c1d](https://github.com/manticoresoftware/manticoresearch/commit/9c1d221e) 修改索引类型后 searchd 重载问题
34. [提交 71e2](https://github.com/manticoresoftware/manticoresearch/commit/71e2b5bb) 守护进程在导入表时因缺少文件而崩溃
35. [问题 #322](https://github.com/manticoresoftware/manticoresearch/issues/322) 使用多个索引、group by 和 ranker = none 查询时崩溃
36. [提交 c3f5](https://github.com/manticoresoftware/manticoresearch/commit/c3f58490) `HIGHLIGHT()` 在字符串属性中无法高亮
37. [问题 #320](https://github.com/manticoresoftware/manticoresearch/issues/320) `FACET` 无法在字符串属性上排序
38. [提交 4f1a](https://github.com/manticoresoftware/manticoresearch/commit/4f1a1f25) 缺少数据目录时的错误
39. [提交 04f4](https://github.com/manticoresoftware/manticoresearch/commit/04f4ddd4) access_* 在 RT 模式不支持
40. [提交 1c06](https://github.com/manticoresoftware/manticoresearch/commit/1c0616a2) 字符串中的错误 JSON 对象：1. `CALL PQ` 在 json 大于某值时返回 "Bad JSON objects in strings: 1"。
41. [提交 32f9](https://github.com/manticoresoftware/manticoresearch/commit/32f943d6) RT 模式不一致。在某些情况下，无法删除索引因为索引未知，且无法创建因为目录非空。
42. [问题 #319](https://github.com/manticoresoftware/manticoresearch/issues/319) 查询时崩溃
43. [提交 22a2](https://github.com/manticoresoftware/manticoresearch/commit/22a28dd7) `max_xmlpipe2_field` = 2M 在 2M 字段上返回警告
44. [问题 #342](https://github.com/manticoresoftware/manticoresearch/issues/342) 查询条件执行错误
45. [提交 dd8d](https://github.com/manticoresoftware/manticoresearch/commit/dd8dcab2) 简单的两个词搜索找到了只包含一个词的文档
46. [提交 9091](https://github.com/manticoresoftware/manticoresearch/commit/90919e62) 在 PQ 中无法匹配带有大写字母键的 json
47. [提交 56da](https://github.com/manticoresoftware/manticoresearch/commit/56da086a) 索引器在 csv+docstore 上崩溃
48. [问题 #363](https://github.com/manticoresoftware/manticoresearch/issues/363) 在 centos 7 中 json 属性使用 `[null]` 导致插入数据损坏
49. 重大 [问题 #345](https://github.com/manticoresoftware/manticoresearch/issues/345) 记录未被插入，count() 结果随机，"replace into" 返回 OK
50. max_query_time 使 SELECT 查询过慢
51. [问题 #352](https://github.com/manticoresoftware/manticoresearch/issues/352) Mac OS 上主从通信失败
52. [问题 #328](https://github.com/manticoresoftware/manticoresearch/issues/328) 使用 Connector.Net/Mysql 8.0.19 连接 Manticore 时出错
53. [提交 daa7](https://github.com/manticoresoftware/manticoresearch/commit/daa760d2fe83668038318f0f384ceefc0d075e18) 修正了 \0 的转义并优化性能
54. [提交 9bc5](https://github.com/manticoresoftware/manticoresearch/commit/9bc5c01a493db993b90ff6c1ad51aeae028f629c) 修正了 count distinct 与 json 的问题
55. [提交 4f89](https://github.com/manticoresoftware/manticoresearch/commit/4f89a965a3af0db548ae9d1763731ebaebe21f5a) 修正了其他节点上的 drop table 失败
56. [提交 952a](https://github.com/manticoresoftware/manticoresearch/commit/952af5a5931fd72b87edc169588777778b365231) 修正了高频调用 call pq 时的崩溃


## 版本 3.4.2，2020年4月10日
### 关键 Bug 修复
* [提交 2ffe](https://github.com/manticoresoftware/manticoresearch/commit/2ffe2d261793f9681265bc4abe38d695e72c121e) 修复旧版本 RT 索引无法索引数据的问题

## 版本 3.4.0，2020年3月26日
### 主要变更
* 服务器支持两种模式：rt 模式和纯模式
   * rt 模式需要 data_dir，且配置中不定义索引
   * 纯模式中索引在配置文件中定义；不允许 data_dir
* 复制仅在 rt 模式下支持

### 次要变更
* charset_table 默认使用 non_cjk 别名
* rt 模式下全文字段默认被索引和存储
* rt 模式中全文字段从 'field' 重命名为 'text'
* ALTER RTINDEX 重命名为 ALTER TABLE
* TRUNCATE RTINDEX 重命名为 TRUNCATE TABLE

### 新功能
* 仅存储字段
* SHOW CREATE TABLE, IMPORT TABLE

### 改进
* 更快的无锁 PQ
* /sql 可在 mode=raw 下执行任意类型的 SQL 语句
* mysql41 协议下提供 mysql 别名
* data_dir 中默认的 state.sql

### Bug 修复
* [Commit a533](https://github.com/manticoresoftware/manticoresearch/commit/a5333644e8e0c69746a9f24da8221fc965478e4a) 修复 highlight() 中错误的字段语法导致的崩溃
* [Commit 7fbb](https://github.com/manticoresoftware/manticoresearch/commit/7fbb9f2e618096289cba7f8b834530b46aa94055) 修复服务器在使用 docstore 复制 RT 索引时的崩溃
* [Commit 24a0](https://github.com/manticoresoftware/manticoresearch/commit/24a04687eb43b4f4719c7c4a5fe3c19099240467) 修复对带有 infix 或 prefix 选项的索引进行高亮，且索引未启用存储字段时的崩溃
* [Commit 3465](https://github.com/manticoresoftware/manticoresearch/commit/3465c1ce9c860c09f6ccc5b83a63e3c68c241d8d) 修复空索引时关于空 docstore 和 doc-id 查找的错误提示
* [Commit a707](https://github.com/manticoresoftware/manticoresearch/commit/b8601b4144b9092be43a7caa7cd6b19cb37e14c7) 修复 #314 带有末尾分号的 SQL insert 命令
* [Commit 9562](https://github.com/manticoresoftware/manticoresearch/commit/95628c9b9b8f16fe1bd18599a84e9673575adf1a) 移除查询词不匹配的警告
* [Commit b860](https://github.com/manticoresoftware/manticoresearch/commit/b8601b4144b9092be43a7caa7cd6b19cb37e14c7) 修复通过 ICU 分词的片段查询问题
* [Commit 5275](https://github.com/manticoresoftware/manticoresearch/commit/5275516ca4631c584e6e0127fa161a18d2658fd0) 修复 docstore 块缓存中的查找/添加竞态条件
* [Commit f06e](https://github.com/manticoresoftware/manticoresearch/commit/f06ef97af5c7390427e82525c150535290e42c3e) 修复 docstore 中的内存泄漏
* [Commit a725](https://github.com/manticoresoftware/manticoresearch/commit/a7258ba8340e246c0d8aaeb4a7a74129eba807eb) 修复 #316 INSERT 时 LAST_INSERT_ID 返回空值
* [Commit 1ebd](https://github.com/manticoresoftware/manticoresearch/commit/1ebd5bf8d74113bcfa138ce63ce11966990a4215) 修复 #317 json/update HTTP 端点支持 MVA 数组和 JSON 属性对象
* [Commit e426](https://github.com/manticoresoftware/manticoresearch/commit/e426950a536382a142e2a42ed207c341149ff685) 修复索引器在没有显式 ID 时转储 rt 的问题

## 版本 3.3.0，2020年2月4日
### 新功能
* 并行实时索引搜索
* EXPLAIN QUERY 命令
* 不含索引定义的配置文件（alpha 版本）
* CREATE/DROP TABLE 命令（alpha 版本）
* indexer --print-rt - 可从数据源读取并打印用于实时索引的 INSERT 语句

### 改进
* 更新至 Snowball 2.0 词干提取器
* SHOW INDEX STATUS 支持 LIKE 过滤器
* 高 max_matches 时改进内存使用
* SHOW INDEX STATUS 新增 rt 索引的 ram_chunks_count
* 无锁优先队列（PQ）
* 将 LimitNOFILE 修改为 65536


### Bug 修复
* [Commit 9c33](https://github.com/manticoresoftware/manticoresearch/commit/9c33aab8c1fd9059c8644d23d5f58f03010043c6) 添加索引架构中重复属性的检查 #293
* [Commit a008](https://github.com/manticoresoftware/manticoresearch/commit/a0085f9477e191da9e0e835b91583fb9cfde698e) 修复无命中的词条导致的崩溃
* [Commit 6895](https://github.com/manticoresoftware/manticoresearch/commit/68953740af8dca1833026d0f448c71416ed3044a) 修复 ATTACH 后的松散 docstore 问题
* [Commit d6f6](https://github.com/manticoresoftware/manticoresearch/commit/d6f696edea53ea2b9a68251e4054e4816082c285) 修复分布式设置中的 docstore 问题
* [Commit bce2](https://github.com/manticoresoftware/manticoresearch/commit/bce2b7ec883d208f31094ee42a5b6f01d18a998f) 在排序器中用 OpenHash 替代 FixedHash
* [Commit e0ba](https://github.com/manticoresoftware/manticoresearch/commit/e0baf7392937969e640fb5d10e8ccb312fe36446) 修复索引定义中重复属性名的问题
* [Commit ca81](https://github.com/manticoresoftware/manticoresearch/commit/ca81114bf1ac7ea7adcc8509bdc14d1f30b4f461) 修复 HIGHLIGHT() 中的 html_strip 问题
* [Commit 493a](https://github.com/manticoresoftware/manticoresearch/commit/493a5e916d70fd4a3f14d1ee487234434f4a7ce3) 修复 HIGHLIGHT() 中的 passage 宏问题
* [Commit a82d](https://github.com/manticoresoftware/manticoresearch/commit/a82d41c775692f837f267d554d7d4a4b499d69f0) 修复 RT 索引创建小或大磁盘块时的双缓冲问题
* [Commit a404](https://github.com/manticoresoftware/manticoresearch/commit/a404c85de0d41d9d1c75b7540479d8a3af9e61aa) 修复 kqueue 的事件删除
* [Commit 8bea](https://github.com/manticoresoftware/manticoresearch/commit/8bea0f6fd1666b817995c4dd6e67f6f1c9161638) 修复 RT 索引 rt_mem_limit 大值时磁盘块保存问题
* [Commit 8707](https://github.com/manticoresoftware/manticoresearch/commit/8707f0395b7ace52dc0cba9c8ed92a2bc4392932) 修复索引时的浮点溢出
* [Commit a564](https://github.com/manticoresoftware/manticoresearch/commit/a56434ce71bb7714b4d8e75278aaa075b27ece80) 修复插入带负 ID 到 RT 索引时出错的情况
* [Commit bbeb](https://github.com/manticoresoftware/manticoresearch/commit/bbebfd7551a68641154761cac962053b54806759) 修复 ranker 的 fieldmask 导致服务器崩溃
* [Commit 3809](https://github.com/manticoresoftware/manticoresearch/commit/3809cc1b0863e3a5a200dc423be3f7c38be56113) 修复使用查询缓存时发生的崩溃
* [Commit dc2a](https://github.com/manticoresoftware/manticoresearch/commit/dc2a585bc724678679e4297362200bbd49ce32bb) 修复 RT 索引在并行插入时 RAM 段崩溃

## 版本 3.2.2，2019年12月19日
### 新功能
* RT 索引的自增 ID
* 通过新的 HIGHLIGHT() 函数支持 docstore 高亮，HTTP API 也支持
* SNIPPET() 可使用特殊函数 QUERY()，返回当前 MATCH 查询
* 新增高亮函数的 field_separator 选项

### 改进和变更
* 远程节点的存储字段懒加载（显著提升性能）
* 字符串和表达式不再阻断多查询和 FACET 优化
* RHEL/CentOS 8 版本构建使用 mariadb-connector-c-devel 提供的 mysql libclient
* ICU 数据文件随软件包一同分发，移除 icu_data_dir 选项
* systemd 服务文件包含 'Restart=on-failure' 策略
* indextool 现在支持在线检测实时索引
* 默认配置文件路径改为 /etc/manticoresearch/manticore.conf
* 在 RHEL/CentOS 上的服务从 'searchd' 重命名为 'manticore'
* 移除了 query_mode 和 exact_phrase snippet 的选项

### Bug修复
* [提交 6ae4](https://github.com/manticoresoftware/manticoresearch/commit/6ae474c7894a6bee222d5b18e59a44fdbf57843a) 修复 HTTP 接口上执行 SELECT 查询时的崩溃
* [提交 5957](https://github.com/manticoresoftware/manticoresearch/commit/59577513a49eac5a4a3c5e2cb38394d3246b5d35) 修复 RT 索引保存磁盘块但未标记部分文档为已删除的问题
* [提交 e861](https://github.com/manticoresoftware/manticoresearch/commit/e861f0fca0e88924450695d4e9d6acff7a36558a) 修复使用 dist_threads 多索引或多查询搜索时的崩溃
* [提交 4409](https://github.com/manticoresoftware/manticoresearch/commit/440991fc977b8479800b45cdbc862eeb1ba5d965) 修复对包含宽 UTF8 代码点的长词的中缀生成崩溃
* [提交 5fd5](https://github.com/manticoresoftware/manticoresearch/commit/5fd599b48bba527c023e6aa0b262dca51ffb8a1c) 修复向 IOCP 添加套接字时的竞态
* [提交 cf10](https://github.com/manticoresoftware/manticoresearch/commit/cf10d7d3589f77adfaefbe52a51777bc9c67cf99) 修复 bool 查询与 json 选择列表的兼容问题
* [提交 996d](https://github.com/manticoresoftware/manticoresearch/commit/996de77f4c3ec103d965e28c36098fd07dba50c4) 修正 indextool 检查报错错误跳过列表偏移，修正 doc2row 查找检查
* [提交 6e3f](https://github.com/manticoresoftware/manticoresearch/commit/6e3fc9e88941c9427410f9d0adeebb7dd09a900f) 修复索引器在大数据上产生带负跳过列表偏移的错误索引
* [提交 faed](https://github.com/manticoresoftware/manticoresearch/commit/faed3220b423a96401982bf47117edf1f62c584e) 修复 JSON 只将数字转换为字符串和表达式中 JSON 字符串转数字
* [提交 5331](https://github.com/manticoresoftware/manticoresearch/commit/533197200249d648ebcbdaca5d605861ee566348) 修复 indextool 在命令行设置多个命令时退出并返回错误代码
* [提交 7955](https://github.com/manticoresoftware/manticoresearch/commit/795520ac351d22e4497ebaf6c249bf25081842b6) 修复 #275 binlog 在磁盘空间不足导致错误时状态无效
* [提交 2284](https://github.com/manticoresoftware/manticoresearch/commit/2284da5e3ff77bf7be4038a4d950c0580e4159c8) 修复 #279 IN 过滤器对 JSON 属性导致的崩溃
* [提交 ce2e](https://github.com/manticoresoftware/manticoresearch/commit/ce2e4b4765111a7d331ffc256911f12770a8942d) 修复 #281 错误的管道关闭调用
* [提交 5355](https://github.com/manticoresoftware/manticoresearch/commit/535589ba8a37baf5c6056afb5ff969fd70476feb) 修复服务器在调用带有递归 JSON 属性作为字符串编码的 PQ 时挂起
* [提交 a5fc](https://github.com/manticoresoftware/manticoresearch/commit/a5fc8a36e1644ee051632408ab8b53e79aeb9ff4) 修复 multiand 节点在文档列表尾部越界的问题
* [提交 a362](https://github.com/manticoresoftware/manticoresearch/commit/a3628617ef4ee89a811d4ba2c4a04cbbc298c1b5) 修正线程公共信息的获取
* [提交 f8d2](https://github.com/manticoresoftware/manticoresearch/commit/f8d2d7bbbb90af3d2ee97424f97b05927f09d42b) 修复 docstore 缓存锁

## 版本 3.2.0, 2019年10月17日
### 新特性
* 文档存储
* 新指令 stored_fields、docstore_cache_size、docstore_block_size、docstore_compression、docstore_compression_level

### 改进与变更
* 改进了 SSL 支持
* 更新非 CJK 内置字符集
* 禁止 UPDATE/DELETE 语句在查询日志中记录为 SELECT
* 提供 RHEL/CentOS 8 软件包

### Bug修复
* [提交 301a](https://github.com/manticoresoftware/manticoresearch/commit/301a806b14725e842300ee225521407464374c41) 修复在 RT 索引磁盘块中替换文档时的崩溃
* [提交 46c1](https://github.com/manticoresoftware/manticoresearch/commit/46c1cad8fa11c1fc09cc34b20d978fdfcd4e6d27) 修复 #269 LIMIT N OFFSET M 问题
* [提交 92a4](https://github.com/manticoresoftware/manticoresearch/commit/92a46edaa196ef33f3ac2225fbdba2c2b88ee45d) 修复 DELETE 语句中显式设置 id 或提供 id 列表跳过搜索的问题
* [提交 8ca7](https://github.com/manticoresoftware/manticoresearch/commit/8ca78c138d837caab30dc6e2343a3c4d1687bb87) 修复 Windows poller 下网络循环事件移除后索引错误问题
* [提交 6036](https://github.com/manticoresoftware/manticoresearch/commit/603631e2bff5cb0acf25f68b85621742490eccd9) 修复通过 HTTP 使用 JSON 处理浮点数的四舍五入问题
* [提交 62f6](https://github.com/manticoresoftware/manticoresearch/commit/62f64cb9e75ec28fc3c29f8414ce9656f5580254) 修复远程片段优先检查空路径；修正 Windows 测试
* [提交 aba2](https://github.com/manticoresoftware/manticoresearch/commit/aba274c2c5abc1029894ba9f95b786bda93e8f22) 修复配置重载在 Windows 下与 Linux 保持一致
* [提交 6b8c](https://github.com/manticoresoftware/manticoresearch/commit/6b8c4242ef77c3fa4d0ccb7d76d81714b6728f0b) 修复 #194 PQ 支持词形变化和词干分析器
* [提交 174d](https://github.com/manticoresoftware/manticoresearch/commit/174d312905285aa5746f4f404c53e6dc669ef42f) 修复 RT 退役分段管理

## 版本 3.1.2, 2019年8月22日
### 新特性与改进
* HTTP API 实验性 SSL 支持
* CALL KEYWORDS 的字段过滤
* /json/search 接口 max_matches 参数
* 自动调整默认 Galera 的 gcache.size
* 改进 FreeBSD 支持

### Bug修复
* [提交 0a1a](https://github.com/manticoresoftware/manticoresearch/commit/0a1a2c81038794983b4e30c8aefc7cc20e89b03f) 修复 RT 索引复制到已存在且路径不同的节点时的问题
* [提交 4adc](https://github.com/manticoresoftware/manticoresearch/commit/4adc075294ac823289f745e2cc419f18c7dcf2e2) 修复无活动索引的刷新重调度
* [提交 d6c0](https://github.com/manticoresoftware/manticoresearch/commit/d6c00a6f750c04021747e9c99e0f77707a4cbfc3) 改善 RT/PQ 索引的刷新重调度
* [提交 d0a7](https://github.com/manticoresoftware/manticoresearch/commit/d0a7c95906b9b5ba33c98267d54501cfe27508d6) 修复 #250 TSV 和 CSV 管道源的 index_field_lengths 索引选项
* [Commit 1266](https://github.com/manticoresoftware/manticoresearch/commit/1266d54800242709fbe2c34e72eb69b6595b23bd) 修复 indextool 在空索引上进行块索引检查时错误的报告
* [Commit 553c](https://github.com/manticoresoftware/manticoresearch/commit/553ca73ca111f28e0888d59cd48b0d6df181ecd6) 修复 Manticore SQL 查询日志中空的 select 列表
* [Commit 56c8](https://github.com/manticoresoftware/manticoresearch/commit/56c8584456293665ddd4ce4c94307d74473a78b9) 修复 indexer 的 -h/--help 响应

## 版本 3.1.0，2019 年 7 月 16 日
### 功能和改进
* 针对实时索引的复制
* 用于中文的 ICU 分词器
* 新的形态学选项 icu_chinese
* 新指令 icu_data_dir
* 用于复制的多语句事务
* LAST_INSERT_ID() 和 @session.last_insert_id
* 在 SHOW VARIABLES 中支持 LIKE 'pattern'
* 用于 percolate 索引的多文档 INSERT
* 为配置添加了时间解析器
* 内部任务管理器
* 在文档和命中列表组件中使用 mlock
* jail 片段路径

### 移除
* 为了支持 ICU，放弃了 RLP 库支持；已移除所有 rlp\* 指令
* 禁用使用 UPDATE 更新文档 ID

### 错误修复
* [Commit f047](https://github.com/manticoresoftware/manticoresearch/commit/f047222339fd0e62a07e9ca8b2a829297d5db861) 修复 concat 和 group_concat 中的缺陷
* [Commit b081](https://github.com/manticoresoftware/manticoresearch/commit/b08147ee24712b5eaa51f261626b5d16c6242fdd) 修复 percolate 索引中的查询 uid，使其为 BIGINT 属性类型
* [Commit 4cd8](https://github.com/manticoresoftware/manticoresearch/commit/4cd85afaf76eef0ce9fa1cd073f3fed160279890) 如果预分配新磁盘块失败，则不崩溃
* [Commit 1a55](https://github.com/manticoresoftware/manticoresearch/commit/1a551227886474054111088412f2b2396f485652) 在 ALTER 中添加缺失的 timestamp 数据类型
* [Commit f3a8](https://github.com/manticoresoftware/manticoresearch/commit/f3a8e096fd496a6b001d398aadbb781b2dbad929) 修复因错误的 mmap 读取导致的崩溃
* [Commit 4475](https://github.com/manticoresoftware/manticoresearch/commit/447577115192db7ac7bfea70b5ba58ee2c61229f) 修复复制中集群锁的哈希
* [Commit ff47](https://github.com/manticoresoftware/manticoresearch/commit/ff476df955f057bd38e8b6ad6056325e9d6fbb9c) 修复复制中提供者泄漏
* [Commit 58dc](https://github.com/manticoresoftware/manticoresearch/commit/58dcbb779c4c30ca1560460c78e35ba114b83716) 修复 \#246 indexer 中未定义的 sigmask
* [Commit 3dd8](https://github.com/manticoresoftware/manticoresearch/commit/3dd8278e747d534f69fe9c3d360eef188ab7b6d5) 修复 netloop 报告中的竞争条件
* [Commit a02a](https://github.com/manticoresoftware/manticoresearch/commit/a02aae05a3fa3d1caf2821f43c7e237b6de76336) 将 HA 策略重新平衡器的间隙设为零

## 版本 3.0.2，2019 年 5 月 31 日
### 改进
* 为文档和命中列表添加了 mmap 阅读器
* `/sql` HTTP 端点响应现在与 `/json/search` 响应相同
* 新指令 `access_plain_attrs`, `access_blob_attrs`, `access_doclists`, `access_hitlists`
* 用于复制设置的新指令 `server_id`

### 移除
* 移除了 HTTP `/search` 端点

### 弃用
* `read_buffer`、`ondisk_attrs`、`ondisk_attrs_default`、`mlock` 已被 `access_*` 指令取代

### 错误修复
* [Commit 849c](https://github.com/manticoresoftware/manticoresearch/commit/849c16e1040f382795ba0635cb48686f9db4f2d8) 允许 select 列表中以数字开头的属性名称
* [Commit 48e6](https://github.com/manticoresoftware/manticoresearch/commit/48e6c302ca37bec0bc49c79619c60bf787ed0708) 修复 UDF 中的 MVA，修复 MVA 别名
* [Commit 0555](https://github.com/manticoresoftware/manticoresearch/commit/055586a9a2ebfa8874f68099c5bc798a145fd4ef) 修复在使用带 SENTENCE 的查询时发生的 \#187 崩溃问题
* [Commit 93bf](https://github.com/manticoresoftware/manticoresearch/commit/93bf52f23f1c53c8a93a603387ca5463e1fc2ae8) 修复 \#143，支持 MATCH() 周围的括号
* [Commit 599e](https://github.com/manticoresoftware/manticoresearch/commit/599ee79ccd38cce0023a55cbce8f50c3dca62e38) 修复 ALTER cluster 语句上保存集群状态的问题
* [Commit 230c](https://github.com/manticoresoftware/manticoresearch/commit/230c321e277514b191629538eb3f02d219113d95) 修复在 ALTER index 时带 blob 属性导致的服务器崩溃
* [Commit 5802](https://github.com/manticoresoftware/manticoresearch/commit/5802b85aa93a81caea77073a3381ef912fdd677c) 修复 \#196 按 id 过滤
* [Commit 25d2](https://github.com/manticoresoftware/manticoresearch/commit/25d2dabda4a299a29bd375405862126da9b4bfac) 丢弃对模板索引的搜索
* [Commit 2a30](https://github.com/manticoresoftware/manticoresearch/commit/2a30d5b49d76faab031d195f533974938769b8e6) 修复 id 列在 SQL 回复中使用常规 bigint 类型


## 版本 3.0.0，2019 年 5 月 6 日
### 功能和改进
* 新的索引存储。非标量属性不再受每个索引 4GB 大小的限制
* attr_update_reserve 指令
* 可以使用 UPDATE 更新 String、JSON 和 MVA
* killlists 在索引加载时应用
* killlist_target 指令
* 多 AND 搜索加速
* 更好的平均性能和内存使用
* 用于升级使用 2.x 创建的索引的 convert 工具
* CONCAT() 函数
* JOIN CLUSTER cluster AT 'nodeaddress:port'
* ALTER CLUSTER posts UPDATE nodes
* node_address 指令
* 在 SHOW STATUS 中打印的节点列表

### 行为变更
* 对于带有 killists 的索引，服务器不按 conf 中定义的顺序轮换索引，而是遵循 killlist target 链
* 搜索中索引的顺序不再定义应用 killlists 的顺序
* 文档 ID 现在是有符号大整数

### 移除的指令
* docinfo（现在始终为 extern）、inplace_docinfo_gap、mva_updates_pool

## 版本 2.8.2 GA，2019 年 4 月 2 日
### 功能和改进
* 针对 percolate 索引的 Galera 复制
* OPTION morphology

### 编译说明
CMake 最低版本现在为 3.13。编译需要 boost 和 libssl 开发库
开发库。

### 错误修复
* [Commit 6967](https://github.com/manticoresoftware/manticoresearch/commit/6967fedb2ef818ec1c825d482563edd05e1c9245) 修复了在查询多个分布式索引时，选择列表中出现多个星号导致的崩溃问题
* [Commit 36df](https://github.com/manticoresoftware/manticoresearch/commit/36df1a407dc08263690e3492518613ace82d69ca) 修复了通过 Manticore SQL 接口发送大数据包时的问题 [\#177](https://github.com/manticoresoftware/manticoresearch/issues/177)
* [Commit 5793](https://github.com/manticoresoftware/manticoresearch/commit/57932aec734583fa93359faaf5034b2e2c9d352b) 修复了在 RT 索引上执行优化操作时更新 MVA 导致服务器崩溃的问题 [\#170](https://github.com/manticoresoftware/manticoresearch/issues/170)
* [Commit edb2](https://github.com/manticoresoftware/manticoresearch/commit/edb24b870423add86eba471d361e0e5aff098b18) 修复了因配置重载后 SIGHUP 信号移除 RT 索引导致二进制日志被删除时服务器崩溃的问题
* [Commit bd3e](https://github.com/manticoresoftware/manticoresearch/commit/bd3e66e0085bc7e2e351b817dfc972fd8158fbce) 修复了 MySQL 握手认证插件的负载问题
* [Commit 6a21](https://github.com/manticoresoftware/manticoresearch/commit/6a217f6ed82fb10f752213b15617b9cedf1e8533) 修复了 RT 索引中 phrase_boundary 设置的问题 [\#172](https://github.com/manticoresoftware/manticoresearch/issues/172)
* [Commit 3562](https://github.com/manticoresoftware/manticoresearch/commit/3562f652753e4091fd7b5b0f65b58341c8cbcb31) 修复了在将索引 ATTACH 到自身时发生死锁的问题 [\#168](https://github.com/manticoresoftware/manticoresearch/issues/168)
* [Commit 250b](https://github.com/manticoresoftware/manticoresearch/commit/250b3f0e74ad18dda34ba080aa13ca87798a9ac9) 修复了服务器崩溃后二进制日志保存空元数据的问题
* [Commit 4aa6](https://github.com/manticoresoftware/manticoresearch/commit/4aa6c69ad3a181cd7c5dafb990fb528121f68e59) 修复了 RT 索引带磁盘分块时字符串引发的排序器崩溃问题

## 版本 2.8.1 GA, 2019年3月6日
### 功能和改进
* SUBSTRING_INDEX()
* 支持 percolate 查询中的 SENTENCE 和 PARAGRAPH
* 为 Debian/Ubuntu 添加了 systemd 生成器；还添加了 LimitCORE 以允许生成 core dump

### Bug 修复
* [Commit 84fe](https://github.com/manticoresoftware/manticoresearch/commit/84fe7405d17b59f4dcde15a6c8e8503923503def) 修复了匹配模式为 all 且全文查询为空时服务器崩溃的问题
* [Commit daa8](https://github.com/manticoresoftware/manticoresearch/commit/daa88b579fa373d5d2e869e43a2e178363b0fef1) 修复了删除静态字符串时的崩溃问题
* [Commit 2207](https://github.com/manticoresoftware/manticoresearch/commit/22078537dddbd9ce81a182dcc879f235b6e87004) 修复了 indextool 发生 FATAL 错误时的退出码问题
* [Commit 0721](https://github.com/manticoresoftware/manticoresearch/commit/0721696d6780c200c65d596624a8187983fb7fcb) 修复了由于错误的精确模式检查导致前缀匹配失败的问题 [\#109](https://github.com/manticoresoftware/manticoresearch/issues/109)
* [Commit 8af8](https://github.com/manticoresoftware/manticoresearch/commit/8af810111b8e2f87bc9f378172eff1ab9725c7e7) 修复了 RT 索引配置重新加载的问题 [\#161](https://github.com/manticoresoftware/manticoresearch/issues/161)
* [Commit e2d5](https://github.com/manticoresoftware/manticoresearch/commit/e2d592773f4fd702ace90070a793a842a0fec6ab) 修复了访问大规模 JSON 字符串时服务器崩溃的问题
* [Commit 75cd](https://github.com/manticoresoftware/manticoresearch/commit/75cd1342d05cdc352e3dd145cdbc6f79394a165b) 修复了 JSON 文档中 PQ 字段被索引剥离器修改导致的兄弟字段错误匹配问题
* [Commit e2f7](https://github.com/manticoresoftware/manticoresearch/commit/e2f775437df922674d772ed4417780492502e65a) 修复了 RHEL7 构建中解析 JSON 时的服务器崩溃问题
* [Commit 3a25](https://github.com/manticoresoftware/manticoresearch/commit/3a25a5808feb3f8b80866991b436d6c1241618c2) 修复了在斜杠位于边缘时 JSON 反转义导致崩溃的问题
* [Commit be9f](https://github.com/manticoresoftware/manticoresearch/commit/be9f497872bea62dbdccd64fc28294fd7776c289) 修复了选项 'skip_empty' 跳过空文档且不警告其不是有效 JSON 的问题
* [Commit 266e](https://github.com/manticoresoftware/manticoresearch/commit/266e0e7b088549722a805fc837bf101ff681a5e8) 修复了当6位精度不足以表达时，浮点数输出8位数字的问题 [\#140](https://github.com/manticoresoftware/manticoresearch/issues/161)
* [Commit 3f6d](https://github.com/manticoresoftware/manticoresearch/commit/3f6d23891064ee8e94030c4231497cdd7da33a6a) 修复了空 jsonobj 创建问题
* [Commit f3c7](https://github.com/manticoresoftware/manticoresearch/commit/f3c7848a59b7eedc67c3403d6d4b37ce6fa94dc8) 修复了空 mva 输出为 NULL 而非空字符串的问题 [\#160](https://github.com/manticoresoftware/manticoresearch/issues/161)
* [Commit 0afa](https://github.com/manticoresoftware/manticoresearch/commit/0afa2ed058e5759470b1d7354c722faab34f98bb) 修复了在无 pthread_getname_np 环境下编译失败的问题
* [Commit 9405](https://github.com/manticoresoftware/manticoresearch/commit/9405fccdeb27f0302a8d9a848981d30080216777) 修复了线程池工作线程导致服务器关闭时的崩溃问题

## 版本 2.8.0 GA, 2019年1月28日
### 改进
* 分布式索引支持 percolate 索引
* CALL PQ 新选项和变更：
    *   skip_bad_json
    *   mode (sparsed/sharded)
    *   JSON 文档可以作为 JSON 数组传递
    *   shift
    *   列名 'UID'、'Documents'、'Query'、'Tags'、'Filters' 改为 'id'、'documents'、'query'、'tags'、'filters'
* DESCRIBE pq TABLE
* 不再支持 SELECT FROM pq WHERE UID，改用 'id'
* pq 索引上的 SELECT 与常规索引相同（例如，您可以通过 REGEX() 过滤规则）
* PQ 标签上可以使用 ANY/ALL
* 表达式对 JSON 字段自动类型转换，无需显式类型转换
* 内置 'non_cjk' 字符集表和 'cjk' ngram 字符
* 内置50种语言的停用词集合
* 停用词声明中可用逗号分隔多个文件
* CALL PQ 支持接受文档的 JSON 数组

### Bug 修复
* [Commit a4e1](https://github.com/manticoresoftware/manticoresearch/commit/a4e19afee54dafdc04b0da53372dafd8c0d332d6) 修复了与 csjon 相关的内存泄漏
* [Commit 28d8](https://github.com/manticoresoftware/manticoresearch/commit/28d862774874891a03e361da1b0347ebe8066ce0) 修复了因 json 中缺少值导致的崩溃
* [Commit bf4e](https://github.com/manticoresoftware/manticoresearch/commit/bf4e9ea27c349cdc9ae4e54e960a4a17b90e38fa) 修复了 RT 索引空元数据的保存
* [Commit 33b4](https://github.com/manticoresoftware/manticoresearch/commit/33b4573529e7c7c7bce19bf9d54054866f30d99a) 修复了词形还原序列丢失的表单标志（exact）
* [Commit 6b95](https://github.com/manticoresoftware/manticoresearch/commit/6b95d48240d3b5520afade19c249d79e020a5e88) 修复了字符串属性超过 4M 时使用饱和操作而非溢出
* [Commit 6214](https://github.com/manticoresoftware/manticoresearch/commit/621418b829e70af36aaa322a2f51ece3f86bc499) 修复了服务器在禁用索引时接收到 SIGHUP 信号导致崩溃的问题
* [Commit 3f7e](https://github.com/manticoresoftware/manticoresearch/commit/3f7e35d1482966cc45d5abbcb2de5de3508d66b0) 修复了服务器在同时执行 API 会话状态命令时崩溃
* [Commit cd9e](https://github.com/manticoresoftware/manticoresearch/commit/cd9e4f1709a48ddafc128c450c2d882bc11ba50e) 修复了在带字段过滤器的 RT 索引删除查询时报错崩溃
* [Commit 9376](https://github.com/manticoresoftware/manticoresearch/commit/9376470d455fcd256c6abff79c431a6919dfa7ac) 修复了调用分布式索引的 PQ 时空文档导致的服务器崩溃
* [Commit 8868](https://github.com/manticoresoftware/manticoresearch/commit/8868b207644f4f8cc1ab0c270adb35493fed22e1) 修复了 Manticore SQL 错误信息被截断超过512字符的问题
* [Commit de9d](https://github.com/manticoresoftware/manticoresearch/commit/de9deda9c142823d6dbf529423417b5c670fae94) 修复了未启用二进制日志时保存 percolate 索引导致崩溃
* [Commit 2b21](https://github.com/manticoresoftware/manticoresearch/commit/2b219e1a32791e7740bb210b7d408f96abc6374f) 修复了 OSX 系统上 http 接口不工作的问题
* [Commit e92c](https://github.com/manticoresoftware/manticoresearch/commit/e92c60240f8f9e1756bfa14fba0bbb402586a389) 修复了 indextool 检查 MVA 时的错误警告
* [Commit 238b](https://github.com/manticoresoftware/manticoresearch/commit/238bdea59bad89f097403f1c978658ce45f16c70) 修复了 FLUSH RTINDEX 时写锁，避免保存和 rt_flush_period 例行刷新期间锁住整个索引
* [Commit c26a](https://github.com/manticoresoftware/manticoresearch/commit/c26a236bd42b7082db079a51a84172066d337d64) 修复了 ALTER percolate 索引时卡在等待搜索加载问题
* [Commit 9ee5](https://github.com/manticoresoftware/manticoresearch/commit/9ee5703f6c28f57cd5bff5e705966d93fe30d267) 修复了 max_children 为 0 时使用默认线程池工作线程数量的问题
* [Commit 5138](https://github.com/manticoresoftware/manticoresearch/commit/5138fc048a1bd146b271ce6a72fe954344281f69) 修复了带有 index_token_filter 插件且 stopwords 和 stopword_step=0 时索引数据出错
* [Commit 2add](https://github.com/manticoresoftware/manticoresearch/commit/2add3d319ac62fe450bf60e89033100853dc2ecf) 修复了在索引定义中仍使用 aot 词形还原而缺少 lemmatizer_base 的崩溃

## 版本 2.7.5 GA，2018年12月4日
### 改进
* REGEX 函数
* json API 搜索支持 limit/offset
* qcache 的性能分析点

### Bug 修复
* [Commit eb3c](https://github.com/manticoresoftware/manticoresearch/commit/eb3c7683b9740cff4cc4cadd2ab9f990b9f7d173) 修复了服务器在带有多属性宽类型的 FACET 查询时崩溃
* [Commit d915](https://github.com/manticoresoftware/manticoresearch/commit/d915cf6696514f4be365ffb5981bccaa15983e65) 修复了 FACET 查询主选择列表中隐式 group by 的问题
* [Commit 5c25](https://github.com/manticoresoftware/manticoresearch/commit/5c25dc271a8c11a1c82ca3c88e518acbc6a8c164) 修复了带 GROUP N BY 查询时崩溃
* [Commit 85d3](https://github.com/manticoresoftware/manticoresearch/commit/85d30a232d4ffd89916b673a9b2db946d272a945) 修复了内存操作崩溃处理中的死锁
* [Commit 8516](https://github.com/manticoresoftware/manticoresearch/commit/85166b5ef2be38061d62f2b20f15acaa0311258a) 修复了 indextool 检查期间内存消耗过高
* [Commit 58fb](https://github.com/manticoresoftware/manticoresearch/commit/58fb03166830d8b23e7ab9ce56309db7ddac22f4) 修复了 gmock 引用不再需要的问题，因为上游已自行解决

## 版本 2.7.4 GA，2018年11月1日
### 改进
* 远程分布式索引情况下，SHOW THREADS 显示原始查询而非 API 调用
* SHOW THREADS 新增选项 `format=sphinxql`，以 SQL 格式打印所有查询
* SHOW PROFILE 新增打印 `clone_attrs` 阶段

### Bug 修复
* [Commit 4f15](https://github.com/manticoresoftware/manticoresearch/commit/4f155712a0bccc1bd01cc191647bc8cff814888e) 修复了在无 malloc_stats、malloc_trim 的 libc 上编译失败
* [Commit f974](https://github.com/manticoresoftware/manticoresearch/commit/f974f20bda3214a56877c393a192be1a77150958) 修复了 CALL KEYWORDS 结果集中单词内的特殊符号问题
* [Commit 0920](https://github.com/manticoresoftware/manticoresearch/commit/092083282ea8ae0b2e72fb6989c57ccec81e74ac) 修复了通过 API 或远程代理调用分布式索引的 CALL KEYWORDS 损坏问题
* [Commit fd68](https://github.com/manticoresoftware/manticoresearch/commit/fd686bfe88b720ffd7642e36897ba45161cbd7d2) 修复了分布式索引代理查询超时参数 agent_query_timeout 向代理作为 max_query_time 传播的问题
* [Commit 4ffa](https://github.com/manticoresoftware/manticoresearch/commit/4ffa623e9d357a3b0b441615089e211f92f8de32) 修复了磁盘块中文档总计数受 OPTIMIZE 命令影响，破坏权重计算的问题
* [Commit dcaf](https://github.com/manticoresoftware/manticoresearch/commit/dcaf4e0e3c8e9b0fbec47bbf3307feddec0936a6) 修复了 RT 索引中混合查询时多个尾部命中问题
* [Commit eee3](https://github.com/manticoresoftware/manticoresearch/commit/eee381754e902a43a8f499e1c950198a2a3e6ee0) 修复了轮转时死锁

## 版本 2.7.3 GA，2018年9月26日
### 改进
* CALL KEYWORDS的sort_mode选项
* VIP连接上的DEBUG可以执行‘crash <password>’以在服务器上故意触发SIGEGV操作
* DEBUG可以执行‘malloc_stats’以在searchd.log中转储malloc状态，‘malloc_trim’来执行malloc_trim()
* 如果系统上存在gdb，改进了回溯功能

### Bug修复
* [提交 0f3c](https://github.com/manticoresoftware/manticoresearch/commit/0f3cc333f6129451ad1ae2c97905b6c04ba5d454) 修复了Windows上重命名时崩溃或失败的问题
* [提交 1455](https://github.com/manticoresoftware/manticoresearch/commit/1455ba2bf8079f03ce7f861a8d9662a360d705de) 修复了32位系统上服务器崩溃的问题
* [提交 ad37](https://github.com/manticoresoftware/manticoresearch/commit/ad3710d53b5cd6a28b8c60352d0f9e285b03d9fa) 修复了在空的SNIPPET表达式上服务器崩溃或挂起的问题
* [提交 b36d](https://github.com/manticoresoftware/manticoresearch/commit/b36d79214364500252665860f000ae85343528b6) 修复了非渐进优化的破损并修正了渐进优化不为最旧磁盘区块创建kill-list的问题
* [提交 34b0](https://github.com/manticoresoftware/manticoresearch/commit/34b032499afd42ce47a4c7247814b4031094388a) 修复了线程池工作模式下SQL和API的queue_max_length错误回复
* [提交 ae4b](https://github.com/manticoresoftware/manticoresearch/commit/ae4b3202cbdb8014cabe2b90e269d5cf74f49871) 修复了向带有regexp或rlp选项的PQ索引添加全扫描查询时的崩溃
* [提交 f80f](https://github.com/manticoresoftware/manticoresearch/commit/f80f8d5d7560187078868aed9a9575f4549e98aa) 修复了连续调用PQ时的崩溃
* [提交 9742](https://github.com/manticoresoftware/manticoresearch/commit/9742f5f0866af73f8cd8483ecd18a507ea80dd65) 重构AcquireAccum
* [提交 39e5](https://github.com/manticoresoftware/manticoresearch/commit/39e5bc3751b7295222eb76407c5d72ce1dad545b) 修复了调用pq后内存泄露
* [提交 21bc](https://github.com/manticoresoftware/manticoresearch/commit/21bcc6d17395f0f57dde79f6716ef303b7ea527d) 外观上的重构（使用c++11风格的构造函数、默认值、nullptr）
* [提交 2d69](https://github.com/manticoresoftware/manticoresearch/commit/2d690398f14c736956cfdd66feb4d3091d6b3a4d) 修复了尝试向PQ索引插入重复项时的内存泄漏
* [提交 5ed9](https://github.com/manticoresoftware/manticoresearch/commit/5ed92c4b66da6423fa28f11b1fd59103ed1e1fb9) 修复了JSON字段IN语句带大值时的崩溃
* [提交 4a52](https://github.com/manticoresoftware/manticoresearch/commit/4a5262e2776aa261e34ba5c914ea60804f59de3f) 修复了调用带有扩展限制设置的RT索引的CALL KEYWORDS语句时服务器崩溃的问题
* [提交 5526](https://github.com/manticoresoftware/manticoresearch/commit/552646bb6fefde4e2525298e43e628839b421d3d) 修复了PQ匹配查询中的无效过滤器问题
* [提交 204f](https://github.com/manticoresoftware/manticoresearch/commit/204f521408b730198a5bab2c20407a3d7df94873) 为ptr属性引入了小对象分配器
* [提交 2545](https://github.com/manticoresoftware/manticoresearch/commit/25453e5387152c0575d1eda1b2beb89a49c7493a) 将ISphFieldFilter重构为引用计数风格
* [提交 1366](https://github.com/manticoresoftware/manticoresearch/commit/1366ee06e828397074b69bec9265d7147170c60e) 修复了在非终止字符串上使用strtod时的未定义行为和SIGSEGV
* [提交 94bc](https://github.com/manticoresoftware/manticoresearch/commit/94bc6fcbdae4d08bcf6da2966e3ec2966091289f) 修复了json结果集处理中的内存泄漏
* [提交 e78e](https://github.com/manticoresoftware/manticoresearch/commit/e78e9c948963416caff9e4e46296a58080107835) 修复了应用属性添加时内存块越界读取
* [提交 fad5](https://github.com/manticoresoftware/manticoresearch/commit/fad572fb543606b33e688eb82a485b86011545fd) 重构CSphDict实现引用计数风格
* [提交 fd84](https://github.com/manticoresoftware/manticoresearch/commit/fd841a472b472280c93f4af3506851d19f6adc45) 修复了AOT内部类型外泄
* [提交 5ee7](https://github.com/manticoresoftware/manticoresearch/commit/5ee7f208c7685f2ebb9e5623a8802b721e2a7f0a) 修复了分词器管理的内存泄漏
* [提交 116c](https://github.com/manticoresoftware/manticoresearch/commit/116c5f1abebee9a0d99afe93546a1d8e4c6c6958) 修复了grouper中的内存泄漏
* [提交 56fd](https://github.com/manticoresoftware/manticoresearch/commit/56fdbc9ab38973a9a0ba8ccee45f71cf33c16423) 对matches中的动态指针进行特别的释放/复制（修复grouper内存泄漏）
* [提交 b1fc](https://github.com/manticoresoftware/manticoresearch/commit/31db660f378541aa0d1e7cf2e7209bb0a1f47fd8) 修复了RT动态字符串的内存泄漏
* [提交 517b](https://github.com/manticoresoftware/manticoresearch/commit/517b9e8749f092e923cd2884b674b5bb84e20755) 重构grouper
* [提交 b1fc](https://github.com/manticoresoftware/manticoresearch/commit/b1fc16140e5dc44290686330b476a254e0092107) 小幅重构（c++11构造函数，部分格式调整）
* [提交 7034](https://github.com/manticoresoftware/manticoresearch/commit/7034e073f4cf2844762e0a464b7c3de05d3d122b) 将ISphMatchComparator重构为引用计数风格
* [提交 b1fc](https://github.com/manticoresoftware/manticoresearch/commit/b1fc16140e5dc44290686330b476a254e0092107) 私有化克隆器
* [提交 efbc](https://github.com/manticoresoftware/manticoresearch/commit/efbc0511d6809c4a57453c7283f9bf53f3fb8d97) 简化MVA_UPSIZE、DOCINFO2ID_T、DOCINFOSETID的本地小端格式
* [提交 6da0](https://github.com/manticoresoftware/manticoresearch/commit/6da0df4ac9946fc59f98d229b90e12c9221dd4c0) 为uber测试添加valgrind支持
* [提交 1d17](https://github.com/manticoresoftware/manticoresearch/commit/1d17669cb7ec54ac80b392e101f37a688eb98080) 修复了连接上‘success’标志竞争导致的崩溃
* [提交 5a09](https://github.com/manticoresoftware/manticoresearch/commit/5a09c32d7ded56b2c0bd3e2ad7968cb8d6362919) 将epoll切换为边缘触发模式
* [提交 5d52](https://github.com/manticoresoftware/manticoresearch/commit/5d528682737fca03a4352a3093b1319ec27d598c) 修复了格式化表达式中的IN语句如同过滤器的问题
* [提交 bd8b](https://github.com/manticoresoftware/manticoresearch/commit/bd8b3c976ff8b4667f55dfdd101b20a920137ac5) 修复 RT 索引在提交带有大 docid 的文档时崩溃
* [提交 ce65](https://github.com/manticoresoftware/manticoresearch/commit/ce656b83b747ce7c55795b53915770c13fb99cfe) 修复 indextool 中无参选项的问题
* [提交 08c9](https://github.com/manticoresoftware/manticoresearch/commit/08c9507177820f441f534bf06a11dac5e54bebb4) 修复扩展关键字的内存泄漏
* [提交 30c7](https://github.com/manticoresoftware/manticoresearch/commit/30c75a2f525ec9bda625d903acdc9d4d2e2413dc) 修复 json grouper 的内存泄漏
* [提交 6023](https://github.com/manticoresoftware/manticoresearch/commit/6023f269b6f2080e4d380b0957605ef8107c8c9f) 修复全局用户变量的泄漏
* [提交 7c13](https://github.com/manticoresoftware/manticoresearch/commit/7c138f15ca23c0c8717fa12041e3db7f988fcf15) 修复早期被拒绝的匹配上动态字符串泄漏
* [提交 9154](https://github.com/manticoresoftware/manticoresearch/commit/9154b18eaed5302e8330cb3a95bd968959ccb312) 修复 length(<expression>) 的泄漏
* [提交 43fc](https://github.com/manticoresoftware/manticoresearch/commit/43fca3a4e26139765d0fac8de054200dc4875fc6) 修复在解析器中由于 strdup() 导致的内存泄漏
* [提交 71ff](https://github.com/manticoresoftware/manticoresearch/commit/71ff77737e63d1019b7c9804dca7fa2351025dba) 修复重构表达式解析器以准确跟踪引用计数

## 版本 2.7.2 GA，2018年8月27日
### 改进
* 与 MySQL 8 客户端兼容
* 支持带有 RECONFIGURE 的 [TRUNCATE](Emptying_a_table.md)
* 在 RT 索引的 SHOW STATUS 中移除内存计数器
* 多代理的全局缓存
* 改进 Windows 上的 IOCP
* HTTP 协议的 VIP 连接
* Manticore SQL 的 [DEBUG](Reporting_bugs.md#DEBUG) 命令，可运行各种子命令
* [shutdown_token](Server_settings/Searchd.md#shutdown_token) — 使用 DEBUG 命令调用 `shutdown` 时需要的密码 SHA1 哈希
* SHOW AGENT STATUS 的新统计项 (_ping, _has_perspool, _need_resolve)
* indexer 的 --verbose 选项现在支持 \[debugvv\] 用于打印调试信息

### 修复
* [提交 3900](https://github.com/manticoresoftware/manticoresearch/commit/390082a7be0a1f9539b30361d11d54de35c62a44) 移除 optimize 时的写锁
* [提交 4c33](https://github.com/manticoresoftware/manticoresearch/commit/4c337648329be22373be33333cbc9c5664d18b80) 修复重新加载索引设置时的写锁
* [提交 b5ea](https://github.com/manticoresoftware/manticoresearch/commit/b5ea8dc0b18a23dbd739e92c66dd93b0094df14b) 修复带 JSON 过滤条件的查询的内存泄漏
* [提交 930e](https://github.com/manticoresoftware/manticoresearch/commit/930e831a43fd3d1ce996185be968d1df49a088d0) 修复 PQ 结果集中的空文档问题
* [提交 53de](https://github.com/manticoresoftware/manticoresearch/commit/53deecfecb374431f84f8592a1095a77407b8aea) 修复因删除任务导致的任务混淆
* [提交 cad9](https://github.com/manticoresoftware/manticoresearch/commit/cad9b9c617719aad97e9f5a2ae93491153d8e504) 修复远程主机数量统计错误
* [提交 9000](https://github.com/manticoresoftware/manticoresearch/commit/90008cc15c4684d445d37a391bb4a667bbc2e365) 修复解析的代理描述符内存泄漏
* [提交 978d](https://github.com/manticoresoftware/manticoresearch/commit/978d839641ad6040c22c9fc3b703b1a02685f266) 修复搜索中的内存泄漏
* [提交 0193](https://github.com/manticoresoftware/manticoresearch/commit/0193946105ac08913362b7d7acf0c90f9b38dc85) 对显式/内联 c-trs，override/final 用法的美化修改
* [提交 943e](https://github.com/manticoresoftware/manticoresearch/commit/943e2997b33f9b2eda23d94a66068910f9476ebd) 修复本地/远程 schema 中 json 泄漏
* [提交 02db](https://github.com/manticoresoftware/manticoresearch/commit/02dbdd6f1d0b72e7e458ebebdd2c67f989577e6b) 修复本地/远程 schema 中 json 排序列表达式的泄漏
* [提交 c74d](https://github.com/manticoresoftware/manticoresearch/commit/c74d0b4035e4214510376845b22cb676a8da24a3) 修复 const 别名的泄漏
* [提交 6e5b](https://github.com/manticoresoftware/manticoresearch/commit/6e5b57e1d3952c9695376728a143bf5434208f53) 修复预读线程的泄漏
* [提交 39c7](https://github.com/manticoresoftware/manticoresearch/commit/39c740fef5bf71da0bee60a35f5dd9f471af850f) 修复退出时因为 netloop 中等待卡住的问题
* [提交 adaf](https://github.com/manticoresoftware/manticoresearch/commit/adaf9772f95772ad6d5297acace6c5cb92b56ad5) 修复从 HA 代理切换到普通主机时 'ping' 行为卡住的问题
* [提交 32c4](https://github.com/manticoresoftware/manticoresearch/commit/32c40eac9cdd40b15d79e58b2f15416f164f9701) 分离仪表盘存储的垃圾回收
* [提交 511a](https://github.com/manticoresoftware/manticoresearch/commit/511a3cf2c1f1858f6e94fe9f8175b7216db3cbd6) 修复引用计数指针问题
* [提交 32c4](https://github.com/manticoresoftware/manticoresearch/commit/af2326cd2927effbad7bec88e8dc238a566cf27e) 修复 indextool 在不存在索引时报错
* [提交 156e](https://github.com/manticoresoftware/manticoresearch/commit/156edc717dc63d3ea120f42466eba6e022da88be) 修复 xmlpipe 索引中超出属性/字段输出名问题
* [提交 cdac](https://github.com/manticoresoftware/manticoresearch/commit/cdac6d1f17d384bfc8bae49e91241b2f346c3830) 修复配置无索引器节时默认索引器值
* [提交 e61e](https://github.com/manticoresoftware/manticoresearch/commit/e61ec00b6b27d1d5878247e2ee817f3b1e7fde16) 修复服务器重启后 RT 索引磁盘块中错误嵌入的停用词
* [提交 5fba](https://github.com/manticoresoftware/manticoresearch/commit/5fba49d5e28c17de4c0acbd984466127f42de6e8) 修复跳过幽灵（已关闭但未从轮询器最终删除）连接
* [提交 f22a](https://github.com/manticoresoftware/manticoresearch/commit/f22ae34b623906b3c5a05a06198dad2e548dc541) 修复混合（孤立）网络任务
* [提交 4689](https://github.com/manticoresoftware/manticoresearch/commit/46890e70eb8dcd00c1bf1b030538b8f131c601c2) 修复写入后读取操作崩溃问题
* [Commit 03f9](https://github.com/manticoresoftware/manticoresearch/commit/03f9df904f611c438d5ebcaaeef911b0dc8d6c39) 修复了在 Windows 上运行测试时 searchd 崩溃的问题
* [Commit e925](https://github.com/manticoresoftware/manticoresearch/commit/e9255ed2704790a3d7f5273d38b85433463cb3ff) 修复了通常的 connect() 操作时处理 EINPROGRESS 代码的问题
* [Commit 248b](https://github.com/manticoresoftware/manticoresearch/commit/248b72f1544b27e9e82773284050d44eeb1714e1) 修复了使用 TFO 时连接超时的问题

## 版本 2.7.1 GA，2018年7月4日
### 改进
* 改善了 PQ 上匹配多个文档时的通配符性能
* 支持 PQ 上的全表扫描查询
* 支持 PQ 上的 MVA 属性
* 支持 percolate 索引的正则表达式和 RLP

### 修复
* [Commit 6885](https://github.com/manticoresoftware/manticoresearch/commit/68856261b41f6950666f9e5122726839fb3c71d0) 修复查询字符串丢失的问题
* [Commit 0f17](https://github.com/manticoresoftware/manticoresearch/commit/0f1770943cba53bfd2e0edfdf0b1495ff0dd0e89) 修复 SHOW THREADS 语句中信息为空的问题
* [Commit 53fa](https://github.com/manticoresoftware/manticoresearch/commit/53faa36a916bb87868e83aa79c1a1f972fb20ca0) 修复使用 NOTNEAR 运算符匹配时的崩溃
* [Commit 2602](https://github.com/manticoresoftware/manticoresearch/commit/26029a7cd77b518fb3a27588d7db6c8bffc73b0f) 修复 PQ 删除时错误的过滤器消息


## 版本 2.7.0 GA，2018年6月11日
### 改进
* 减少系统调用次数以避免 Meltdown 和 Spectre 补丁的影响
* 本地索引管理的内部重写
* 远程摘要重构
* 完整配置重载
* 所有节点连接现已独立
* 协议改进
* Windows 通信从 wsapoll 切换到 IO 完成端口
* 主节点与节点之间的通信可使用 TFO
* [SHOW STATUS](Node_info_and_management/Node_status.md#SHOW-STATUS) 现在输出服务器版本和 mysql_version_string
* 为 CALL PQ 中调用的文档新增了 `docs_id` 选项。
* percolate 查询过滤器现在可以包含表达式
* 分布式索引可以与 FEDERATED 一起使用
* 虚拟 SHOW NAMES COLLATE 和 `SET wait_timeout`（以提升 ProxySQL 兼容性）

### 修复
* [Commit 5bcf](https://github.com/manticoresoftware/manticoresearch/commit/5bcff05d19e495f1a44f190885b25cbb6f217c4a) 修复 PQ 中添加的“不等于”标签
* [Commit 9ebc](https://github.com/manticoresoftware/manticoresearch/commit/9ebc58916cd515eaa88da66d0895aebf0d1f2b5f) 修复为 JSON 文档 CALL PQ 语句添加文档 ID 字段的问题
* [Commit 8ae0](https://github.com/manticoresoftware/manticoresearch/commit/8ae0e593a286110d8505d88baa3ac9e433cb38c9) 修复 PQ 索引的刷新语句处理器
* [Commit c24b](https://github.com/manticoresoftware/manticoresearch/commit/c24b152344ecd77661566cddb803487f51d3c1aa) 修复 PQ 上 JSON 和字符串属性的过滤
* [Commit 1b8b](https://github.com/manticoresoftware/manticoresearch/commit/1b8bdde19aaa362785ea4c33bdc019154f8fbac4) 修复空 JSON 字符串的解析
* [Commit 1ad8](https://github.com/manticoresoftware/manticoresearch/commit/1ad8a07dbfc1131913a8d92c261fbb48f934e8b7) 修复多查询使用 OR 过滤器时的崩溃
* [Commit 69b8](https://github.com/manticoresoftware/manticoresearch/commit/69b89806380a229e36287ad9daf6f0ea2b5453eb) 修复 indextool 使用配置公共段（lemmatizer_base 选项）用于命令（dumpheader）
* [Commit 6dbe](https://github.com/manticoresoftware/manticoresearch/commit/6dbeaf2389d2af6a9dfccb56bfc986685b9f203e) 修复结果集和过滤器中的空字符串
* [Commit 39c4](https://github.com/manticoresoftware/manticoresearch/commit/39c4eb55ecc85f6dd54f6c8c6d6dfcf87fd8748e) 修复负文档 ID 值
* [Commit 266b](https://github.com/manticoresoftware/manticoresearch/commit/266b7054f98e7d30fc2829b5c2467dc9f90def92) 修复超长单词索引时的词剪辑长度
* [Commit 4782](https://github.com/manticoresoftware/manticoresearch/commit/47823b0bd06cbd016a24345c99bb8693790b43dd) 修复 PQ 上通配符查询匹配多个文档


## 版本 2.6.4 GA，2018年5月3日
### 功能和改进
* MySQL FEDERATED 引擎[支持](Extensions/FEDERATED.md)
* MySQL 数据包现在返回 SERVER_STATUS_AUTOCOMMIT 标志，增强了 ProxySQL 兼容性
* [listen_tfo](Server_settings/Searchd.md#listen_tfo) - 启用所有监听器的 TCP 快速打开连接
* indexer --dumpheader 也可以从 .meta 文件中导出 RT 头信息
* Ubuntu Bionic 的 cmake 构建脚本

### 修复
* [Commit 355b](https://github.com/manticoresoftware/manticoresearch/commit/355b11629174813abd9cd5bf2233be0783f77745) 修复 RT 索引的无效查询缓存条目；
* [Commit 546e](https://github.com/manticoresoftware/manticoresearch/commit/546e2297a01cc2913bc0d33052d5bcefae8737eb) 修复无缝旋转后丢失索引设置
* [Commit 0c45](https://github.com/manticoresoftware/manticoresearch/commit/0c4509898393993bc87194d2aca2070395ff7f83) 修复中缀与前缀长度设置; 添加对不支持中缀长度的警告
* [Commit 8054](https://github.com/manticoresoftware/manticoresearch/commit/80542fa54d727c781635918e8d4d65a20c2fbd1e) 修复 RT 索引自动刷新顺序
* [Commit 705d](https://github.com/manticoresoftware/manticoresearch/commit/705d8c5f407726ed1c6f9745ecffa375682c8969) 修复具有多个属性且查询多个索引的索引结果集模式问题
* [Commit b0ba](https://github.com/manticoresoftware/manticoresearch/commit/b0ba932fff4a082f642cb737381c8a70215d4cc9) 修复批量插入时文档重复导致部分命中丢失
* [Commit 4510](https://github.com/manticoresoftware/manticoresearch/commit/4510fa44ad54b5bbac53c591144501565130fcae) 修复优化失败以合并带有大量文档的 RT 索引磁盘块

## 版本 2.6.3 GA，2018年3月28日
### 改进
* 编译时使用 jemalloc。如果系统上存在 jemalloc，可以通过 cmake 标志 `-DUSE_JEMALLOC=1` 启用

### 修复
* [Commit 85a6](https://github.com/manticoresoftware/manticoresearch/commit/85a6d7e3db75f28c491042409bb677ed39eba07b) 修复了将 expand_keywords 选项记录到 Manticore SQL 查询日志的问题
* [Commit caaa](https://github.com/manticoresoftware/manticoresearch/commit/caaa3842f4dbd8507fd9e20bf3371a0c8ec0c087) 修复了 HTTP 接口正确处理大尺寸查询的问题
* [Commit e386](https://github.com/manticoresoftware/manticoresearch/commit/e386d842b36195719e459b2db1fb20e402f9a51f) 修复了启用 index_field_lengths 时 DELETE 操作导致服务器崩溃的问题（针对 RT 索引）
* [Commit cd53](https://github.com/manticoresoftware/manticoresearch/commit/ce528f37eb5d8284661928f6f3b340a4a1985253) 修复了 cpustats searchd 命令行选项在不支持的系统上的兼容性问题
* [Commit 8740](https://github.com/manticoresoftware/manticoresearch/commit/8740fd63ae842c2ab0e51a8fcd6180d787ed57ec) 修复了定义了最小长度时 utf8 子字符串匹配的问题


## 版本 2.6.2 GA，2018年2月23日
### 改进
* 提升了 [Percolate Queries](Searching/Percolate_query.md) 在使用 NOT 运算符和批量文档场景下的性能。
* [percolate_query_call](Searching/Percolate_query.md) 现在可根据 [dist_threads](Server_settings/Searchd.md#threads) 使用多个线程
* 新增全文匹配运算符 NOTNEAR/N
* 对 percolate 索引上的 SELECT 语句支持 LIMIT
* [expand_keywords](Searching/Options.md#expand_keywords) 可接受 'start', 'exact'（其中 'start,exact' 效果与 '1' 相同）
* 支持对使用 sql_query_range 定义的范围查询的 [joined fields](Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Processing_fetched_data.md#sql_joined_field) 进行 ranged-main-query

### Bug 修复
* [Commit 72dc](https://github.com/manticoresoftware/manticoresearch/commit/72dcf669744e9b7d636dfc213d24df85ab301f6b) 修复了搜索内存段时的崩溃；修复了保存双缓冲磁盘块死锁的问题；修复了 optimize 期间保存磁盘块死锁
* [Commit 3613](https://github.com/manticoresoftware/manticoresearch/commit/36137149a1c3c0893bdda5a28fc7e8244bf2d4ae) 修复了索引器在嵌入式 XML 模式且属性名为空时崩溃的问题
* [Commit 48d7](https://github.com/manticoresoftware/manticoresearch/commit/48d7e8001d2a66466ca64577f27ddc5421a67251) 修复了错误地取消未拥有的 pid 文件链接的问题
* [Commit a556](https://github.com/manticoresoftware/manticoresearch/commit/a5563a465ddc59ef71e65f17b68bc33f9700e838) 修复了临时文件夹中有时遗留孤立 fifo 文件的问题
* [Commit 2376](https://github.com/manticoresoftware/manticoresearch/commit/2376e8fc4508944b96959bd10686c6d51f5145e8) 修复了 FACET 结果集为空时产生错误的 NULL 行
* [Commit 4842](https://github.com/manticoresoftware/manticoresearch/commit/4842b67acdd5da75672db6c08ac563b48577ddd7) 修复了服务器作为 Windows 服务运行时索引锁损坏的问题
* [Commit be35](https://github.com/manticoresoftware/manticoresearch/commit/be35feef54ded59125750916cd211e56108eddae) 修复了 macOS 上错误的 iconv 库使用问题
* [Commit 8374](https://github.com/manticoresoftware/manticoresearch/commit/83744a977d7837d1d87cd506b88b0e7eb199efe6) 修复了错误的 count(\*) 统计问题


## 版本 2.6.1 GA，2018年1月26日
### 改进
* [agent_retry_count](Server_settings/Searchd.md#agent_retry_count) 在代理有镜像的情况下，返回每个镜像的重试次数，总的每个代理的重试次数为 agent_retry_count\*mirrors。
* [agent_retry_count](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_retry_count) 现在可以针对每个索引指定，覆盖全局值。新增别名 [mirror_retry_count](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#mirror_retry_count)。
* 可以在代理定义中指定 retry_count，表示每个代理的重试次数
* Percolate Queries 现已支持 HTTP JSON API，地址为 [/json/pq](Searching/Percolate_query.md#Performing-a-percolate-query-with-CALL-PQ)。
* 为可执行文件添加了 -h 和 -v 选项（帮助和版本）
* [morphology_skip_fields](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) 支持实时索引

### Bug 修复
* [Commit a40b](https://github.com/manticoresoftware/manticore/commit/a40b0793feff65e40d10062568d9847c08d10f57) 修复了用于 MVA 字段时 ranged-main-query 未正确支持 sql_range_step 的问题
* [Commit f2f5](https://github.com/manticoresoftware/manticore/commit/f2f53757db45bcfb1544263ce0817e856656a621) 修复了黑洞系统循环挂起和黑洞代理断开连接的问题
* [Commit 84e1](https://github.com/manticoresoftware/manticore/commit/84e1f54aef25e0fce98870ad2dd784db5116f1d6) 修复了查询 ID 不一致及存储查询重复 ID 问题
* [Commit 1948](https://github.com/manticoresoftware/manticore/commit/19484231814fcb82b21763a3a4a9f45adc6b2d40) 修复了服务器从各种状态关闭时崩溃的问题
* [Commit 9a70](https://github.com/manticoresoftware/manticore/commit/9a706b499a1d61a90076065a1a703029d49db958)  [Commit 3495](https://github.com/manticoresoftware/manticore/commit/3495fd70cba8846b1a50d55d6679b039414c3d2a) 修复了长查询超时问题
* [Commit 3359](https://github.com/manticoresoftware/manticore/commit/3359bcd89b4f79a645fe84b8cf8616ce0addff02) 重构了在基于 kqueue 的系统（Mac OS X，BSD）上 master-agent 网络轮询


## 版本 2.6.0，2017年12月29日
### 功能和改进
* [HTTP JSON](Connecting_to_the_server/HTTP.md)：JSON 查询现在可对属性做相等匹配，MVA 和 JSON 属性可用于插入和更新，且通过 JSON API 可对分布式索引执行更新和删除操作
* [Percolate Queries](Searching/Percolate_query.md)
* 代码中移除了对 32 位 docid 的支持，同时移除了所有将遗留 32 位 docid 索引转换/加载的代码。
* [仅针对某些字段应用形态学分析](https://github.com/manticoresoftware/manticore/issues/7)。新增索引指令 morphology_skip_fields，用于定义不适用形态学分析的字段列表。
* [expand_keywords 现在可以作为查询运行时指令通过 OPTION 语句设置](https://github.com/manticoresoftware/manticore/issues/8)

### 修复的 Bug
* [提交 0cfa](https://github.com/manticoresoftware/manticore/commit/0cfae4c) 修复了在使用 rlp 构建的服务器调试版本（以及发布版本可能存在的未定义行为）中崩溃的问题
* [提交 3242](https://github.com/manticoresoftware/manticore/commit/324291e) 修复启用 progressive 选项时 RT 索引优化错误合并杀死列表顺序的问题
* [提交 ac0e](https://github.com/manticoresoftware/manticore/commit/ac0efee) 修复 mac 上的少量崩溃问题
* 在彻底的静态代码分析后修复了大量小问题
* 其他小型 bug 修复

### 升级
此版本中我们更改了主节点与代理之间通信的内部协议。如果您在分布式环境下运行 Manticoresearch 且有多个实例，请确保先升级代理，然后升级主节点。

## 版本 2.5.1，2017 年 11 月 23 日
### 功能和改进
* [HTTP API 协议](Connecting_to_the_server/HTTP.md) 支持 JSON 查询。支持搜索、插入、更新、删除、替换操作。数据操作命令也可以批量执行，但目前有一些限制，如 MVA 和 JSON 属性不能用于插入、替换或更新。
* [RELOAD INDEXES](Data_creation_and_modification/Adding_data_from_external_storages/Rotating_a_table.md#RELOAD-TABLES) 命令
* [FLUSH LOGS](Logging/Rotating_query_and_server_logs.md) 命令
* [SHOW THREADS](Node_info_and_management/SHOW_THREADS.md) 可以显示优化、轮换或刷新过程的进度。
* GROUP N BY 正确支持 MVA 属性
* 黑洞代理在独立线程运行，不再影响主查询
* 对索引实现引用计数，避免轮换和高负载时的阻塞
* 实现了 SHA1 哈希，目前尚未对外开放
* 修复了在 FreeBSD、macOS 和 Alpine 上的编译问题

### 修复的 Bug
* [提交 9897](https://github.com/manticoresoftware/manticore/commit/989752b) 修复了块索引的过滤回归问题
* [提交 b1c3](https://github.com/manticoresoftware/manticore/commit/b1c3864) 将 PAGE_SIZE 重命名为 ARENA_PAGE_SIZE 以兼容 musl
* [提交 f213](https://github.com/manticoresoftware/manticore/commit/f2133cc) 禁用 cmake 版本低于 3.1.0 的 googletests
* [提交 f30e](https://github.com/manticoresoftware/manticore/commit/0839de7) 修复服务器重启时绑定套接字失败问题
* [提交 0807](https://github.com/manticoresoftware/manticore/commit/0807240) 修复服务器关闭时崩溃问题
* [提交 3e3a](https://github.com/manticoresoftware/manticore/commit/3e3acc3) 修复显示线程功能中系统黑洞线程的问题
* [提交 262c](https://github.com/manticoresoftware/manticore/commit/262c3fe) 重构了 iconv 的配置检查，修复了 FreeBSD 和 Darwin 上的构建问题

## 版本 2.4.1 GA，2017 年 10 月 16 日
### 功能和改进
* WHERE 子句中支持属性过滤器之间的 OR 运算符
* 维护模式（SET MAINTENANCE=1）
* 分布式索引支持 [CALL KEYWORDS](Searching/Autocomplete.md#CALL-KEYWORDS)
* 支持 [UTC 时间分组](Server_settings/Searchd.md#grouping_in_utc)
* [query_log_mode](Server_settings/Searchd.md#query_log_mode) 支持自定义日志文件权限
* 字段权重可以为零或负数
* [max_query_time](Searching/Options.md#max_query_time) 现在可作用于全扫描
* 新增网络线程微调参数：[net_wait_tm](Server_settings/Searchd.md#net_wait_tm)、[net_throttle_accept](Server_settings/Searchd.md#net_throttle_accept) 和 [net_throttle_action](Server_settings/Searchd.md#net_throttle_action)（当 workers=thread_pool 时）
* COUNT DISTINCT 支持 facet 搜索
* IN 支持 JSON 浮点数组
* 多查询优化不再被整型/浮点表达式破坏
* 使用多查询优化时，[SHOW META](Node_info_and_management/SHOW_META.md) 显示 `multiplier` 行

### 编译
Manticore Search 使用 cmake 构建，最低支持的 gcc 版本为 4.7.2。

### 文件夹和服务
* Manticore Search 以 `manticore` 用户身份运行。
* 默认数据文件夹现为 `/var/lib/manticore/`
* 默认日志文件夹现为 `/var/log/manticore/`
* 默认 pid 文件夹现为 `/var/run/manticore/`

### 修复的 Bug
* [提交 a58c](https://github.com/manticoresoftware/manticore/commit/a58c619) 修复了导致 java 连接器崩溃的 SHOW COLLATION 语句
* [提交 631c](https://github.com/manticoresoftware/manticore/commit/631cf4e) 修复处理分布式索引时崩溃；为分布式索引哈希添加锁；移除代理的移动和复制操作符
* [提交 942b](https://github.com/manticoresoftware/manticore/commit/942bec0) 修复因并行重连导致处理分布式索引时的崩溃
* [提交 e5c1](https://github.com/manticoresoftware/manticore/commit/e5c1ed2) 修复崩溃处理程序在将查询存储到服务器日志时崩溃
* [提交 4a4b](https://github.com/manticoresoftware/manticore/commit/4a4bda5) 修复多查询中使用池属性时的崩溃
* [提交 3873](https://github.com/manticoresoftware/manticore/commit/3873bfb) 通过防止索引页包含在核心文件中，减小核心文件大小
* [提交 11e6](https://github.com/manticoresoftware/manticore/commit/11e6254) 修复在指定无效代理时 searchd 启动崩溃问题
* [提交 4ca6](https://github.com/manticoresoftware/manticore/commit/4ca6350) 修复索引器在 sql_query_killlist 查询中错误报告问题
* [提交 123a](https://github.com/manticoresoftware/manticore/commit/123a9f0) 修复 fold_lemmas=1 与命中计数的问题
* [提交 cb99](https://github.com/manticoresoftware/manticore/commit/cb99164) 修复 html_strip 行为不一致问题
* [提交 e406](https://github.com/manticoresoftware/manticore/commit/e406761) 修复优化实时索引时新设置丢失问题；修复优化同步选项时的锁泄漏；
* [提交 86ae](https://github.com/manticoresoftware/manticore/commit/86aeb82) 修复处理错误多查询时的问题
* [Commit 2645](https://github.com/manticoresoftware/manticore/commit/2645230) 修正了结果集依赖于多查询顺序的问题
* [Commit 7239](https://github.com/manticoresoftware/manticore/commit/72395d9) 修复了多查询中存在错误查询时服务器崩溃的问题
* [Commit f353](https://github.com/manticoresoftware/manticore/commit/f353326) 修正了从共享锁到排他锁的转换
* [Commit 3754](https://github.com/manticoresoftware/manticore/commit/3754785) 修复了无索引查询时服务器崩溃的问题
* [Commit 29f3](https://github.com/manticoresoftware/manticore/commit/29f360e) 修复了服务器死锁的问题

## Version 2.3.3, 2017年7月6日
* Manticore 品牌标识

