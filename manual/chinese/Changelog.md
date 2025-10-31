# 更新日志

## 版本 13.13.0
**发布**：2025年10月7日

### 推荐库
- 推荐的 [MCL](https://github.com/manticoresoftware/columnar) 版本：8.1.0
- 推荐的 [Buddy](Installation/Manticore_Buddy.md#Manticore-Buddy) 版本：3.35.1

如果您遵循[官方安装指南](https://manticoresearch.com/install/)，则无需担心此事。

### ⚠️ 重要事项

由于配置文件已更新，**升级期间在 Linux 上您可能会看到一个警告，询问是否保留您的版本或使用包中的新版本**。如果您有自定义（非默认）配置，建议保留您的版本并将 `pid_file` 路径更新为 `/run/manticore/searchd.pid`。不过，即使不更改路径，仍应该可以正常工作。

### 新功能和改进
* 🆕 [v13.13.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.13.0) 添加对带有 SI 块缓存的 MCL 8.1.0 的支持。
* 🆕 [v13.12.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.12.0) 实现了 [secondary_index_block_cache](../Server_settings/Searchd.md#secondary_index_block_cache) 选项，更新了二级索引 API，并内联了排序访问器。

### 修复的错误
* 🪲 [v13.11.8](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.11.8) [问题 #3791](https://github.com/manticoresoftware/manticoresearch/issues/3791) 修复了检查和调用已触发定时器之间的竞态条件。
* 🪲 [v13.11.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.11.7) [问题 #1045](https://github.com/manticoresoftware/manticoresearch/issues/1045) 通过将配置中过时的路径 `/var/run/manticore` 替换为正确的 `/run/manticore`，修复了 RHEL 8 在 systemd 更新期间出现的 systemctl 警告。由于配置文件已更新，升级时您可能会看到关于是否保留旧版本或使用包中新版本的警告。如果您有自定义（非默认）配置，建议保留您的版本并将 `pid_file` 路径更新为 `/run/manticore/searchd.pid`。
* 🪲 [v13.11.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.11.6) [PR #3766](https://github.com/manticoresoftware/manticoresearch/pull/3766) 添加了对 MCL 版本 8.0.6 的支持。
* 🪲 [v13.11.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.11.5) [PR #3767](https://github.com/manticoresoftware/manticoresearch/pull/3767) 改进了中文文档翻译并更新了子模块。
* 🪲 [v13.11.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.11.4) [PR #3765](https://github.com/manticoresoftware/manticoresearch/pull/3765) 修复了对别名连接属性的处理。
* 🪲 [v13.11.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.11.3) [PR #3763](https://github.com/manticoresoftware/manticoresearch/pull/3763) 修复了在对字符串属性进行批量连接时可能发生的崩溃，并解决了过滤器有时无法与 LEFT JOIN 一起工作的情况。
* 🪲 [v13.11.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.11.2) [问题 #3065](https://github.com/manticoresoftware/manticoresearch/issues/3065) 修复了在启用 index_field_lengths 时向列式表插入数据时发生的崩溃。
* 🪲 [v13.11.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.11.1) [问题 #3751](https://github.com/manticoresoftware/manticoresearch/issues/3751) 修复了启用 embeddings 时删除文档造成的崩溃。

## 版本 13.11.1
**发布**：2025年9月13日

### 修复的错误
* 🪲 [v13.11.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.11.1) [问题 #3751](https://github.com/manticoresoftware/manticoresearch/issues/3751) 修复了启用 embeddings 时删除文档造成的崩溃。

## 版本 13.11.0
**发布**：2025年9月13日

本次发布的主要亮点是[自动嵌入](Searching/KNN.md#Auto-Embeddings-%28Recommended%29)——这是一项新功能，使语义搜索变得像 SQL 一样简单。
无需外部服务或复杂的流水线：只需插入文本并使用自然语言搜索。

### 自动嵌入提供的功能

- **自动生成嵌入向量**，直接从您的文本生成
- **自然语言查询**，理解含义而不仅仅是关键词
- **支持多种模型**（OpenAI、Hugging Face、Voyage、Jina）
- **无缝集成** SQL 和 JSON API

### 推荐库
- 推荐的 [MCL](https://github.com/manticoresoftware/columnar) 版本：8.0.3
- 推荐的 [Buddy](Installation/Manticore_Buddy.md#Manticore-Buddy) 版本：3.35.1

如果您遵循[官方安装指南](https://manticoresearch.com/install/)，则无需担心此事。

### 新功能和改进
* 🆕 [v13.11.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.11.0) [PR #3746](https://github.com/manticoresoftware/manticoresearch/pull/3746) 在 JSON 查询中添加了对嵌入生成的“query”支持。
* 🆕 [v13.10.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.10.0) [问题 #3709](https://github.com/manticoresoftware/manticoresearch/issues/3709) manticore-server RPM 包不再拥有 `/run`。
* 🆕 [v13.9.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.9.0) [PR #3716](https://github.com/manticoresoftware/manticoresearch/pull/3716) 在配置中添加了对 `boolean_simplify` 的支持。
* 🆕 [v13.8.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.8.0) [问题 #3253](https://github.com/manticoresoftware/manticoresearch/issues/3253) 安装了 sysctl 配置以增加 vm.max_map_count 以支持大数据集。
* 🆕 [v13.7.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.7.0) [PR #3681](https://github.com/manticoresoftware/manticoresearch/pull/3681) 添加了对 `alter table <table> modify column <column> api_key=<key>` 的支持。

### 修复的错误
* 🪲 [v13.10.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.10.5) [ PR #3737](https://github.com/manticoresoftware/manticoresearch/pull/3737) 现在 scroll 选项可以正确返回具有大 64 位 ID 的所有文档。
* 🪲 [v13.10.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.10.4) [ PR #3736](https://github.com/manticoresoftware/manticoresearch/pull/3736) 修复了使用带过滤树的 KNN 时的崩溃问题。
* 🪲 [v13.10.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.10.3) [ Issue #3520](https://github.com/manticoresoftware/manticoresearch/issues/3520) `/sql` 端点不再允许使用 SHOW VERSION 命令。
* 🪲 [v13.10.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.10.2) [ PR #3637](https://github.com/manticoresoftware/manticoresearch/pull/3637) 更新了 Windows 安装脚本。
* 🪲 [v13.10.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.10.1) 修复了 Linux 上的本地时区检测。
* 🪲 [v13.9.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.9.2) [ PR #3726](https://github.com/manticoresoftware/manticoresearch/pull/3726) 列模式下的重复 ID 现在会正确返回错误。
* 🪲 [v13.9.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.9.1) [ PR #3333](https://github.com/manticoresoftware/manticoresearch/pull/3333) 手册现在实现了自动翻译。
* 🪲 [v13.8.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.8.6) [ PR #3715](https://github.com/manticoresoftware/manticoresearch/pull/3715) 修复了当批处理中的所有文档都被跳过时嵌入生成的问题。
* 🪲 [v13.8.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.8.5) [ PR #3711](https://github.com/manticoresoftware/manticoresearch/pull/3711) 添加了 Jina 和 Voyage 嵌入模型，以及其他自动嵌入相关的更改。
* 🪲 [v13.8.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.8.4) [ PR #3710](https://github.com/manticoresoftware/manticoresearch/pull/3710) 修复了多面查询时的崩溃问题。
* 🪲 [v13.8.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.8.3) 修复了在 _bulk 端点多语句事务中删除/更新提交未被计为错误的问题。
* 🪲 [v13.8.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.8.2) [ PR #3705](https://github.com/manticoresoftware/manticoresearch/pull/3705) 修复了通过非列式字符串属性连接时的崩溃并改进了错误报告。
* 🪲 [v13.8.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.8.1) [ PR #3704](https://github.com/manticoresoftware/manticoresearch/pull/3704) 修复了查询嵌入在未指定模型时的崩溃；增加了嵌入字符串到主从通信；添加了测试。
* 🪲 [v13.7.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.7.2) [ PR #Buddy#589](https://github.com/manticoresoftware/manticoresearch-buddy/pull/589) 移除了模糊搜索的默认 IDF 处理。
* 🪲 [v13.7.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.7.1) [ Issue #3454](https://github.com/manticoresoftware/manticoresearch/issues/3454) 修复了具有大 64 位 ID 的滚动解码错误。
* 🪲 [v13.6.9](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.9) [ Issue #3674](https://github.com/manticoresoftware/manticoresearch/issues/3674) 修复了 JDBC+MySQL 驱动/连接池在 transaction_read_only 设置中的问题。
* 🪲 [v13.6.8](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.8) [ PR #3676](https://github.com/manticoresoftware/manticoresearch/pull/3676) 修复了嵌入模型返回空结果集时的崩溃。

## Version 13.6.7
**Released**: August 8th 2025

推荐的 [MCL](https://github.com/manticoresoftware/columnar) 版本：8.0.1
推荐的 [Buddy](Installation/Manticore_Buddy.md#Manticore-Buddy) 版本：3.34.2

如果您遵循[官方安装指南](https://manticoresearch.com/install/)，则无需担心此事项。

### 新功能及改进
* 🆕 [v13.6.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.0) [ Issue #2226](https://github.com/manticoresoftware/manticoresearch/issues/2226) 在 PHRASE、PROXIMITY 和 QUORUM 运算符中支持显式的 '|'（OR）。
* 🆕 [v13.5.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.5.0) [ PR #3591](https://github.com/manticoresoftware/manticoresearch/pull/3591) 查询中自动生成嵌入（进行中，尚未准备好用于生产环境）。
* 🆕 [v13.4.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.4.0) [ PR #3585](https://github.com/manticoresoftware/manticoresearch/pull/3585) 修正了逻辑：如果设置了 buddy_path 配置，则优先使用 buddy 线程数，而非守护进程的值。
* 🆕 [v13.3.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.3.0) [ PR #3577](https://github.com/manticoresoftware/manticoresearch/pull/3577) 支持与本地分布式表的连接。
* 🆕 [#3647](https://github.com/manticoresoftware/manticoresearch/issues/3647) 增加对 Debian 13 "Trixie" 的支持

### Bug 修复
* 🪲 [v13.6.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.7) [ Issue #3524](https://github.com/manticoresoftware/manticoresearch/issues/3524) 修复了行式存储中保存生成嵌入的问题。
* 🪲 [v13.6.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.6) [ Issue #3563](https://github.com/manticoresoftware/manticoresearch/issues/3563) 修复了 Sequel Ace 及其他集成因“unknown sysvar”错误导致失败的问题。
* 🪲 [v13.6.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.5) [ Issue #3467](https://github.com/manticoresoftware/manticoresearch/issues/3467) 修复了 DBeaver 及其他集成因“unknown sysvar”错误导致失败的问题。
* 🪲 [v13.6.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.4) [ Issue #3524](https://github.com/manticoresoftware/manticoresearch/issues/3524) 修复了多字段嵌入拼接的问题；同时修复了从查询生成嵌入的问题。
* 🪲 [v13.6.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.3) [ Issue #3641](https://github.com/manticoresoftware/manticoresearch/issues/3641) 修复了13.6.0版本中短语除第一个外丢失所有括号内关键词的错误。
* 🪲 [v13.6.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.2) 修复了transform_phrase中的内存泄漏。
* 🪲 [v13.6.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.1) 修复了13.6.0版本中的内存泄漏。
* 🪲 [v13.5.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.5.2) [ Issue #3651](https://github.com/manticoresoftware/manticoresearch/issues/3651) 修复了更多与全文搜索模糊测试相关的问题。
* 🪲 [v13.5.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.5.1) [ Issue #3560](https://github.com/manticoresoftware/manticoresearch/issues/3560) 修复了OPTIMIZE TABLE在带有KNN数据时可能无限挂起的情况。
* 🪲 [v13.4.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.4.2) [ Issue #2544](https://github.com/manticoresoftware/manticoresearch/issues/2544) 修复了添加float_vector列可能导致索引损坏的问题。
* 🪲 [v13.4.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.4.1) [ Issue #3651](https://github.com/manticoresoftware/manticoresearch/issues/3651) 添加了全文解析的模糊测试，并修复了测试过程中发现的多个问题。
* 🪲 [v13.3.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.3.1) [ Issue #3583](https://github.com/manticoresoftware/manticoresearch/issues/3583) 修复了使用复杂布尔过滤器与高亮显示时的崩溃问题。
* 🪲 [v13.2.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.7) [ Issue #3481](https://github.com/manticoresoftware/manticoresearch/issues/3481) 修复了HTTP更新、分布式表和错误复制集群同时使用时的崩溃。
* 🪲 [v13.2.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.6) [ PR #3567](https://github.com/manticoresoftware/manticoresearch/pull/3567) 更新了manticore-backup依赖至1.9.6版本。
* 🪲 [v13.2.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.5) [ PR #3565](https://github.com/manticoresoftware/manticoresearch/pull/3565) 修复了CI设置以提升Docker镜像兼容性。
* 🪲 [v13.2.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.4) 修复了长令牌处理问题。一些特殊令牌（如正则表达式模式）可能生成过长的词语，现已在使用前进行缩短。

## 版本 13.2.3
**发布时间**：2025年7月8日

### 重大变更
* ⚠️ [PR #3586](https://github.com/manticoresoftware/manticoresearch/pull/3586) 停止支持Debian 10（Buster）。Debian 10已于2024年6月30日生命周期结束。建议用户升级到Debian 11（Bullseye）或Debian 12（Bookworm）。
* ⚠️ [v13.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.0.0) 更新了KNN库API以支持空的[float_vector](https://manual.manticoresearch.com/Creating_a_table/Data_types#Float-vector)值。此更新不更改数据格式，但提升了Manticore Search / MCL API版本。
* ⚠️ [v12.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/12.0.0) [ PR #3516](https://github.com/manticoresoftware/manticoresearch/pull/3516) 修复了KNN索引训练和构建过程中源和目标行ID错误的BUG。此更新不变更数据格式，但提升了Manticore Search / MCL API版本。
* ⚠️ [v11.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.0.0) 增加了对新的KNN向量搜索功能的支持，如量化、重新评分和过采样。该版本更改了KNN数据格式和[KNN_DIST() SQL语法](https://manual.manticoresearch.com/Searching/KNN?client=SQL#KNN-vector-search)。新版能读取旧数据，但旧版无法读取新格式。

### 新功能和改进
* 🆕 [v13.2.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.0) [ PR #3549](https://github.com/manticoresoftware/manticoresearch/issues/3525) 修复了`@@collation_database`导致与某些mysqldump版本不兼容的问题。
* 🆕 [v13.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.1.0) [ Issue #3489](https://github.com/manticoresoftware/manticoresearch/issues/3489) 修复了模糊搜索中阻止解析某些SQL查询的BUG。
* 🆕 [v12.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/12.1.0) [ Issue #3426](https://github.com/manticoresoftware/manticoresearch/issues/3426) 增加了对RHEL 10操作系统的支持。
* 🆕 [v11.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.1.0) 增加了[KNN搜索](https://manual.manticoresearch.com/Searching/KNN#KNN-vector-search)中对空浮点向量的支持。
* 🆕 [v10.2.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.0) [ Issue #3252](https://github.com/manticoresoftware/manticoresearch/issues/3252) [log_level](https://manual.manticoresearch.com/Server_settings/Setting_variables_online#SET) 现也能调整Buddy的日志详细程度。

### 修复的BUG
* 🪲 [v13.2.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.3) [ PR #3556](https://github.com/manticoresoftware/manticoresearch/pull/3556) 修复了JSON查询中"oversampling"选项的解析问题。
* 🪲 [v13.2.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.2) 修复了Linux和FreeBSD上崩溃日志记录的问题，移除了对Boost stacktrace的使用。
* 🪲 [v13.2.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.1) [ Issue #3298](https://github.com/manticoresoftware/manticoresearch/issues/3298) 修复容器内运行时日志记录崩溃问题
* 🪲 [v12.0.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/12.0.2)  通过以微秒计数提升每表统计的准确性
* 🪲 [v12.0.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/12.0.1) [ PR #3522](https://github.com/manticoresoftware/manticoresearch/pull/3522) 修复联合查询中按MVA分面时的崩溃问题
* 🪲 [v11.0.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.0.3) [ Issue #3502](https://github.com/manticoresoftware/manticoresearch/issues/3502) 修复与向量搜索量化相关的崩溃问题
* 🪲 [v11.0.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.0.2) [ Issue #3493](https://github.com/manticoresoftware/manticoresearch/issues/3493) 修改 `SHOW THREADS` 显示CPU利用率为整数
* 🪲 [v11.0.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.0.1)  修正列存和次级库的路径
* 🪲 [v10.2.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.7)  添加对MCL 5.0.5的支持，包括修复嵌入库文件名
* 🪲 [v10.2.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.6) [ Issue #3469](https://github.com/manticoresoftware/manticoresearch/issues/3469) 针对问题 #3469 应用另一修复
* 🪲 [v10.2.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.4) [ Issue #3469](https://github.com/manticoresoftware/manticoresearch/issues/3469) 修复带bool查询的HTTP请求返回空结果的问题
* 🪲 [v10.2.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.5)  修改嵌入库默认文件名并增加KNN向量中“from”字段检查
* 🪲 [v10.2.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.3) [ PR #3464](https://github.com/manticoresoftware/manticoresearch/pull/3464) 更新Buddy至3.30.2版本，包含[PR #565关于内存使用和错误日志](https://github.com/manticoresoftware/manticoresearch-buddy/pull/565)
* 🪲 [v10.2.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.2) [ Issue #3470](https://github.com/manticoresoftware/manticoresearch/pull/3470) 修复JOIN查询中JSON字符串过滤器、null过滤器和排序问题
* 🪲 [v10.2.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.1)  修复 `dist/test_kit_docker_build.sh` 中导致缺失Buddy提交的bug
* 🪲 [v10.1.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.4)  修复联合查询中按MVA分组时的崩溃
* 🪲 [v10.1.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.3) [ Issue #3434](https://github.com/manticoresoftware/manticoresearch/issues/3434) 修复空字符串过滤的bug
* 🪲 [v10.1.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.2) [ PR #3452](https://github.com/manticoresoftware/manticoresearch/pull/3452) 更新Buddy至3.29.7版本，解决了[Buddy #507 - 多查询请求模糊搜索错误](https://github.com/manticoresoftware/manticoresearch-buddy/issues/507)和[Buddy #561 - 指标速率修复](https://github.com/manticoresoftware/manticoresearch-buddy/pull/561)，这是[Helm 发布10.1.0版本](https://github.com/manticoresoftware/manticoresearch-helm/releases/tag/manticoresearch-10.1.0)所需
* 🪲 [v10.1.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.1)  更新Buddy至3.29.4版本，解决了[#3388 - “未定义数组键 'Field'”](https://github.com/manticoresoftware/manticoresearch/issues/3388)和[Buddy #547 - 布局 'ru' 可能失效](https://github.com/manticoresoftware/manticoresearch-buddy/issues/547)

## 版本 10.1.0
**发布于**：2025年6月9日

此版本为一次更新，包含新功能、一次破坏性变更以及众多稳定性提升和漏洞修复。更改主要聚焦于增强监控能力、提升搜索功能以及修复影响系统稳定性和性能的关键问题。

**从版本10.1.0起，不再支持CentOS 7。建议用户升级到受支持的操作系统。**

### 破坏性变更
* ⚠️ [v10.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.0.0) [ Issue #540](https://github.com/manticoresoftware/manticoresearch-buddy/issues/540) 破坏性变更：默认将[fuzzy search](Searching/Spell_correction.md#Fuzzy-Search)的`layouts=''`

### 新功能与改进
* 🆕 [v10.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.0) [ Issue #537](https://github.com/manticoresoftware/manticoresearch-buddy/issues/537) 增加内置[Prometheus exporter](Node_info_and_management/Node_status.md#Prometheus-Exporter)
* 🆕 [v9.8.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.8.0) [ Issue #3409](https://github.com/manticoresoftware/manticoresearch/issues/3409) 增加命令[ALTER TABLE tbl REBUILD KNN](Updating_table_schema_and_settings.md#Rebuilding-a-KNN-index)
* 🆕 [v9.7.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.7.0) [ Issue #1778](https://github.com/manticoresoftware/manticoresearch/issues/1778) 增加自动生成嵌入功能（暂未正式发布，代码已合并主分支但仍需测试）
* 🆕 [v9.6.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.6.0)  调整KNN API版本以支持自动嵌入
* 🆕 [v9.5.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.0) [ Issue #1894](https://github.com/manticoresoftware/manticoresearch/issues/1894) 改进集群恢复：定期保存`seqno`以加快节点崩溃后重启速度
* 🆕 [v9.4.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.4.0) [ Issue #2400](https://github.com/manticoresoftware/manticoresearch/issues/2400) 添加了对最新 [Logstash](Integration/Logstash.md#Integration-with-Logstash) 和 Beats 的支持

### Bug 修复
* 🪲 [v10.0.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.0.1)  修正了词形处理：用户定义的词形现在覆盖自动生成的词形；新增测试用例到测试 22
* 🪲 [v9.8.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.8.2)  修复：再次更新 deps.txt 以包含与嵌入库相关的 MCL 打包修复
* 🪲 [v9.8.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.8.1)  修复：更新 deps.txt，修复 MCL 及嵌入库的打包问题
* 🪲 [v9.7.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.7.3) [ Issue #3306](https://github.com/manticoresoftware/manticoresearch/issues/3306) 修复索引过程中信号 11 导致的崩溃
* 🪲 [v9.7.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.7.2) [ Issue #3109](https://github.com/manticoresoftware/manticoresearch/issues/3109) 修正了不存在的 `@@variables` 总是返回 0 的问题
* 🪲 [v9.7.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.7.1) [ Issue #3377](https://github.com/manticoresoftware/manticoresearch/issues/3377) 修复了与 [remove_repeats()](Functions/Searching_and_ranking_functions.md#REMOVE_REPEATS%28%29) 相关的崩溃问题
* 🪲 [v9.6.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.6.3) [ PR #3411](https://github.com/manticoresoftware/manticoresearch/pull/3411) 修复：对遥测指标使用动态版本检测
* 🪲 [v9.6.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.6.2)  修复：[SHOW VERSION](Node_info_and_management/SHOW_VERSION.md#SHOW-VERSION) 输出中的小错误
* 🪲 [v9.6.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.6.1)  修复：创建带 KNN 属性但无模型的表时崩溃
* 🪲 [v9.5.16](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.16) [ Issue #3342](https://github.com/manticoresoftware/manticoresearch/issues/3342) 修复 `SELECT ... FUZZY=0` 并非总是禁用模糊搜索的问题
* 🪲 [v9.5.15](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.15) [ PR #3397](https://github.com/manticoresoftware/manticoresearch/pull/3397) 新增对 [MCL](https://github.com/manticoresoftware/columnar) 4.2.2 的支持；修复旧存储格式错误
* 🪲 [v9.5.14](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.14) [ Issue #3392](https://github.com/manticoresoftware/manticoresearch/issues/3392) 修正 HTTP JSON 回复中字串处理错误
* 🪲 [v9.5.13](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.13) [ Issue #3356](https://github.com/manticoresoftware/manticoresearch/issues/3356) 修复复杂全文查询（公共子词）场景下的崩溃
* 🪲 [v9.5.12](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.12)  修正磁盘数据块自动刷新错误提示中的拼写错误
* 🪲 [v9.5.11](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.11) [ Issue #3195](https://github.com/manticoresoftware/manticoresearch/issues/3195) 改进 [自动磁盘数据块刷新](Securing_and_compacting_a_table/Flushing_RAM_chunk_to_disk.md#Flushing-RAM-chunk-to-disk)：优化运行时跳过保存
* 🪲 [v9.5.10](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.10) [ Issue #3313](https://github.com/manticoresoftware/manticoresearch/issues/3313) 修复使用 [indextool](Miscellaneous_tools.md#indextool) 检查 RT 表所有磁盘数据块中重复 ID 的问题
* 🪲 [v9.5.9](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.9) [ Issue #3132](https://github.com/manticoresoftware/manticoresearch/issues/3132) 新增对 JSON API 中 `_random` 排序的支持
* 🪲 [v9.5.8](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.8) [ Issue #3382](https://github.com/manticoresoftware/manticoresearch/issues/3382) 修复无法通过 JSON HTTP API 使用 uint64 文档 ID 的问题
* 🪲 [v9.5.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.7) [ Issue #3385](https://github.com/manticoresoftware/manticoresearch/issues/3385) 修正按 `id != value` 过滤时结果不正确的问题
* 🪲 [v9.5.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.6) [ PR #538](https://github.com/manticoresoftware/manticoresearch-buddy/pull/538) 修复模糊匹配在某些情况下的严重错误
* 🪲 [v9.5.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.5) [ Issue #3199](https://github.com/manticoresoftware/manticoresearch/issues/3199) 修正 Buddy HTTP 查询参数中的空格解码（如 `%20` 和 `+`）
* 🪲 [v9.5.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.4) [ Issue #3133](https://github.com/manticoresoftware/manticoresearch/issues/3133) 修复面搜索中 `json.field` 排序错误
* 🪲 [v9.5.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.3) [ Issue #3091](https://github.com/manticoresoftware/manticoresearch/issues/3091) 修正 SQL 与 JSON API 中分隔符搜索结果不一致的问题
* 🪲 [v9.5.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.2) [ Issue #2819](https://github.com/manticoresoftware/manticoresearch/issues/2819) 性能改进：分布式表用 `TRUNCATE` 替换 `DELETE FROM`
* 🪲 [v9.5.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.1) [ Issue #3080](https://github.com/manticoresoftware/manticoresearch/issues/3080) 修复使用 JSON 属性筛选别名 `geodist()` 时崩溃

## 版本 9.3.2
发布日期：2025 年 5 月 2 日

此版本包括多个错误修复和稳定性改进，更好的表使用跟踪，以及内存和资源管理的增强。

❤️ 特别感谢 [@cho-m](https://github.com/cho-m) 修复了与 Boost 1.88.0 的构建兼容性问题，以及 [@benwills](https://github.com/benwills) 改进了关于 `stored_only_fields` 的文档。

* 🪲 [v9.3.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.3.2) 修复了“显示线程”列将 CPU 活动显示为浮点数而非字符串的问题；还修复了 PyMySQL 结果集解析错误，该错误由错误的数据类型引起。
* 🪲 [v9.3.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.3.1) [ Issue #3343](https://github.com/manticoresoftware/manticoresearch/issues/3343) 修复了优化过程被中断时遗留的 `tmp.spidx` 文件。
* 🆕 [v9.3.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.3.0) [ PR #3337](https://github.com/manticoresoftware/manticoresearch/pull/3337) 增加了每表命令计数器和详细的表使用统计信息。
* 🪲 [v9.2.39](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.39) [ Issue #3236](https://github.com/manticoresoftware/manticoresearch/issues/3236) 修复：通过移除复杂的块更新防止表损坏。串行工作线程内使用等待函数会破坏串行处理，可能导致表损坏。
	重新实现了自动刷新。移除了外部轮询队列以避免不必要的表锁。添加了“小表”条件：如果文档数低于“小表限制”(8192) 且未使用二级索引(SI)，则跳过刷新。

* 🪲 [v9.2.38](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.38) 修复：跳过为使用 `ALL`/`ANY` 对字符串列表的过滤器创建二级索引（SI），不影响 JSON 属性。
* 🪲 [v9.2.37](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.37) [ Issue #2898](https://github.com/manticoresoftware/manticoresearch/issues/2898) 为系统表添加了反引号支持。
* 🪲 [v9.2.36](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.36) 修复：在遗留代码中为集群操作使用了占位符。现在解析器中清晰区分了表名和集群名字段。
* 🪲 [v9.2.35](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.35) 修复：取消引用单个 `'` 字符时的崩溃。
* 🪲 [v9.2.34](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.34) [ Issue #3090](https://github.com/manticoresoftware/manticoresearch/issues/3090) 修复：处理大型文档 ID 时的错误（之前可能找不到它们）。
* 🪲 [v9.2.33](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.33) 修复：为位向量大小使用无符号整数。
* 🪲 [v9.2.32](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.32) 修复：合并期间减少峰值内存使用。docid 到 rowid 的查找现在每文档使用 12 字节，而非 16 字节。举例：20 亿文档现用 24 GB RAM，而非 36 GB。
* 🪲 [v9.2.31](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.31) [ Issue #3238](https://github.com/manticoresoftware/manticoresearch/issues/3238) 修复：大型实时表中 `COUNT(*)` 值不正确。
* 🪲 [v9.2.30](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.30) 修复：字符串属性归零时的未定义行为。
* 🪲 [v9.2.29](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.29) 轻微修正：改善了警告文本。
* 🪲 [v9.2.28](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.28) [ Issue #3290](https://github.com/manticoresoftware/manticoresearch/issues/3290) 改进：增强了 `indextool --buildidf`。
* 🪲 [v9.2.27](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.27) [ Issue #3032](https://github.com/manticoresoftware/manticoresearch/issues/3032) Kafka 集成中，现在可以为特定 Kafka 分区创建源。
* 🪲 [v9.2.26](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.26) [ Issue #3301](https://github.com/manticoresoftware/manticoresearch/issues/3301) 修复：在 `id` 上使用 `ORDER BY` 和 `WHERE` 可能导致内存溢出（OOM）错误。
* 🪲 [v9.2.25](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.25) [ Issue #3171](https://github.com/manticoresoftware/manticoresearch/issues/3171) 修复：当 RT 表有多个磁盘块且使用多 JSON 属性的 grouper 时，导致的段错误崩溃。
* 🪲 [v9.2.24](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.24) [ Issue #3246](https://github.com/manticoresoftware/manticoresearch/issues/3246) 修复：RAM 块刷新后，`WHERE string ANY(...)` 查询失败。
* 🪲 [v9.2.23](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.23) [ PR #518](https://github.com/manticoresoftware/manticoresearch-buddy/pull/518) 轻微自动分片语法改进。
* 🪲 [v9.2.22](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.22) [ Issue #2763](https://github.com/manticoresoftware/manticoresearch/issues/2763) 修复：使用 `ALTER TABLE` 时未加载全局 idf 文件。
* 🪲 [v9.2.21](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.21) 修复：全局 idf 文件可能很大。现在我们更早释放表以避免持有不必要的资源。
* 🪲 [v9.2.20](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.20) [ PR #3277](https://github.com/manticoresoftware/manticoresearch/pull/3277) 改进：更好地验证分片选项。

* 🪲 [v9.2.19](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.19) [ PR #3275](https://github.com/manticoresoftware/manticoresearch/pull/3275) 修复：与 Boost 1.88.0 的构建兼容性。
* 🪲 [v9.2.18](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.18) [ Issue #3228](https://github.com/manticoresoftware/manticoresearch/issues/3228) 修复：分布式表创建时崩溃（无效指针问题）。

* 🪲 [v9.2.17](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.17) [ PR #3272](https://github.com/manticoresoftware/manticoresearch/pull/3272) 修复：多行模糊 `FACET` 问题。
* 🪲 [v9.2.16](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.16) [ Issue #3063](https://github.com/manticoresoftware/manticoresearch/issues/3063) 修复：使用 `GEODIST` 函数时距离计算的bug。
* 🪲 [v9.2.15](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.15) [ Issue #3027](https://github.com/manticoresoftware/manticoresearch/issues/3027) 小改进：支持Elastic的`query_string`过滤器格式。

## 版本 9.2.14
发布于：2025年3月28日

### 小改动
* [Commit ](https://github.com/manticoresoftware/manticoresearch/commit/fe9473dc3f4fcfa0eaae748b538fe853f4ce078b) 实现了`--mockstack`标志，用于计算递归操作的栈需求。新的`--mockstack`模式分析并报告递归表达式求值、模式匹配操作、过滤处理所需的栈大小。计算出的栈需求将输出到控制台，方便调试和优化。
* [Issue #3058](https://github.com/manticoresoftware/manticoresearch/pull/3058) 默认启用[boolean_simplify](Searching/Options.md#boolean_simplify)。
* [Issue #3172](https://github.com/manticoresoftware/manticoresearch/issues/3172) 添加了新的配置选项：`searchd.kibana_version_string`，当Manticore与特定版本的Kibana或OpenSearch Dashboards配合使用，需要期望特定Elasticsearch版本时，此选项非常有用。
* [Issue #3211](https://github.com/manticoresoftware/manticoresearch/issues/3211) 修复了[CALL SUGGEST](Searching/Spell_correction.md#CALL-QSUGGEST,-CALL-SUGGEST) 对2字符单词的支持。
* [Issue #490](https://github.com/manticoresoftware/manticoresearch-buddy/issues/490) 改进了[模糊搜索](Searching/Spell_correction.md#Fuzzy-Search)：之前当存在另一个匹配文档时，搜索“def ghi”有时无法找到“defghi”。
* ⚠️ 重大变更 [Issue #3165](https://github.com/manticoresoftware/manticoresearch/issues/3165) 某些HTTP JSON响应中的`_id`改为`id`以保持一致性。请确保相应更新你的应用程序。
* ⚠️ 重大变更 [Issue #3186](https://github.com/manticoresoftware/manticoresearch/issues/3186) 集群加入时新增`server_id`检查，确保每个节点具有唯一ID。现在，当加入节点的`server_id`与集群中已有节点相同时，`JOIN CLUSTER`操作会失败并显示重复[server_id](Server_settings/Searchd.md#server_id)错误信息。为解决此问题，请确保复制集群中每个节点具有唯一的[server_id](Server_settings/Searchd.md#server_id)。你可以在配置文件的"searchd"部分将默认[server_id](Server_settings/Searchd.md#server_id)更改为唯一值后，再尝试加入集群。此更改更新了复制协议。如果你在运行复制集群，需要：
  - 首先，干净地停止所有节点
  - 然后，使用Linux工具`manticore_new_cluster`以`--new-cluster`选项启动最后停止的节点。
  - 详细信息请参阅[重启集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster)。

### Bug 修复
* [Commit 6fda](https://github.com/manticoresoftware/manticoresearch/commit/6fdad3923dd6953f4b781943eed5ec5c28b7f808) 修复了等待后调度器丢失导致的崩溃；现在特定调度器如`serializer`能正确恢复。
* [Commit c333](https://github.com/manticoresoftware/manticoresearch/commit/c333b277d4a504de5fe298e9ab570a9f9ea31e0a) 修复了右连接表的权重无法在`ORDER BY`子句中使用的bug。
* [Issue #2644](https://github.com/manticoresoftware/manticoresearch/issues/2644) gcc 14.2.0：修复了使用`const knn::DocDist_t*&`调用`lower_bound`的错误。❤️ 感谢[@Azq2](https://github.com/Azq2)的PR。
* [Issue #3018](https://github.com/manticoresoftware/manticoresearch/issues/3018) 修复了自动模式插入时处理大写表名的问题。
* [Issue #3119](https://github.com/manticoresoftware/manticoresearch/issues/3119) 修复了解码无效base64输入时崩溃的问题。
* [Issue #3121](https://github.com/manticoresoftware/manticoresearch/issues/3121) 修复了两个关联的KNN索引`ALTER`问题：浮点向量现在保持原始维度，KNN索引能正确生成。
* [Issue #3123](https://github.com/manticoresoftware/manticoresearch/issues/3123) 修复了对空JSON列构建二级索引时的崩溃。
* [Issue #3138](https://github.com/manticoresoftware/manticoresearch/issues/3138) 修复了由重复条目导致的崩溃。
* [Issue #3151](https://github.com/manticoresoftware/manticoresearch/issues/3151) 修复：`fuzzy=1`选项不能与`ranker`或`field_weights`一起使用的问题。
* [Issue #3163](https://github.com/manticoresoftware/manticoresearch/issues/3163) 修复了`SET GLOBAL timezone`无效的bug。
* [Issue #3181](https://github.com/manticoresoftware/manticoresearch/issues/3181) 修复了当使用大于2^63的ID时，文本字段值可能丢失的问题。
* [Issue #3189](https://github.com/manticoresoftware/manticoresearch/issues/3189) 修复：`UPDATE`语句现在正确遵守`query_log_min_msec`设置。
* [Issue #3247](https://github.com/manticoresoftware/manticoresearch/issues/3247) 修复了保存实时磁盘块时的竞争条件，防止`JOIN CLUSTER`失败。


## 版本 7.4.6
发布于：2025年2月28日

### 重大变更
* [Issue #832](https://github.com/manticoresoftware/manticoresearch/issues/832) 与[Kibana](Integration/Kibana.md)集成，实现更简单更高效的数据可视化。

### 小改动
* [Issue #1727](https://github.com/manticoresoftware/manticoresearch/issues/1727) 修正了 arm64 和 x86_64 之间的浮点数精度差异。
* [Issue #2995](https://github.com/manticoresoftware/manticoresearch/issues/2995) 实现了 join 批处理的性能优化。
* [Issue #3039](https://github.com/manticoresoftware/manticoresearch/issues/3039) 实现了直方图中 EstimateValues 的性能优化。
* [Issue #3099](https://github.com/manticoresoftware/manticoresearch/issues/3099) 添加了对 Boost 1.87.0 的支持。❤️ 感谢 [@cho-m](https://github.com/cho-m) 的 PR。
* [Issue #77](https://github.com/manticoresoftware/columnar/issues/77) 优化了在创建多值过滤器时的块数据重用；为属性元数据添加了最小/最大值；实现了基于最小/最大值的过滤器值预过滤。

### Bugfixes
* [Commit 73ac](https://github.com/manticoresoftware/manticoresearch/commit/73ac22f358a7a0e734b332c3943c86e6294c06c3) 修正了在连接查询中使用左右表属性时表达式的处理；修复了右表的 index_weights 选项。
* [Issue #2915](https://github.com/manticoresoftware/manticoresearch/issues/2915) 在 `SELECT ... JOIN` 查询中使用 `avg()` 可能导致结果不正确；现已修复。
* [Issue #2996](https://github.com/manticoresoftware/manticoresearch/issues/2996) 修正了启用 join 批处理时隐式截止点导致的错误结果集。
* [Issue #3031](https://github.com/manticoresoftware/manticoresearch/issues/3031) 修复了守护进程在关闭时因活动块合并导致的崩溃。
* [Issue #3037](http://github.com/manticoresoftware/manticoresearch/issues/3037) 修复了 `IN(...)` 可能产生错误结果的问题。
* [Issue #3038](https://github.com/manticoresoftware/manticoresearch/issues/3038) 在版本 7.0.0 中设置 `max_iops` / `max_iosize` 可能降低索引性能，现已修复。
* [Issue #3042](https://github.com/manticoresoftware/manticoresearch/issues/3042) 修复了 join 查询缓存的内存泄漏。
* [Issue #3052](https://github.com/manticoresoftware/manticoresearch/issues/3052) 修正了连接的 JSON 查询中的查询选项处理。
* [Issue #3054](https://github.com/manticoresoftware/manticoresearch/issues/3054) 修复了 ATTACH TABLE 命令的问题。
* [Issue #3079](https://github.com/manticoresoftware/manticoresearch/issues/3079) 修正了错误信息中的不一致性。
* [Issue #3087](https://github.com/manticoresoftware/manticoresearch/issues/3087) 针对每个表设置 `diskchunk_flush_write_timeout=-1` 未能禁用索引刷新；现已修复。
* [Issue #3088](https://github.com/manticoresoftware/manticoresearch/issues/3088) 解决了批量替换大 ID 后的重复条目问题。
* [Issue #3126](https://github.com/manticoresoftware/manticoresearch/issues/3126) 修复了由带单个 `NOT` 运算符和表达式排序器的全文查询导致的守护进程崩溃。
* [Issue #3128](https://github.com/manticoresoftware/manticoresearch/pull/3128) 修复了 CJSON 库中的潜在漏洞。❤️ 感谢 [@tabudz](https://github.com/tabudz) 的 PR。

## 版本 7.0.0
发布：2025年1月30日

### 主要变化
* [Issue #1497](https://github.com/manticoresoftware/manticoresearch/issues/1497) 新增了 [模糊搜索](Searching/Spell_correction.md#Fuzzy-Search) 和 [自动补全](Searching/Autocomplete.md#CALL-AUTOCOMPLETE) 功能，便于搜索。
* [Issue #1500](https://github.com/manticoresoftware/manticoresearch/issues/1500) [Kafka 集成](Integration/Kafka.md#Syncing-from-Kafka)。
* [Issue #1928](https://github.com/manticoresoftware/manticoresearch/issues/1928) 引入了 [JSON 的二级索引](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#json_secondary_indexes)。
* [Issue #2361](https://github.com/manticoresoftware/manticoresearch/issues/2361) 更新和更新期间的搜索不再被块合并阻止。
* [Issue #2787](https://github.com/manticoresoftware/manticoresearch/issues/2787) RT 表的自动 [磁盘块刷新](Server_settings/Searchd.md#diskchunk_flush_write_timeout) 以提高性能；现在我们会自动将内存块刷新成磁盘块，避免由于内存块缺乏优化而导致的性能问题，这些问题有时会根据块大小导致不稳定。
* [Issue #2811](https://github.com/manticoresoftware/manticoresearch/issues/2811) 增加了 [Scroll](Searching/Pagination.md#Scroll-Search-Option) 选项，简化分页操作。
* [Issue #931](https://github.com/manticoresoftware/manticoresearch/issues/931) 集成了 [Jieba](https://github.com/fxsjy/jieba)，提升 [中文分词](Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md) 效果。

### 次要变化
* ⚠️ 重大变更 [Issue #1111](https://github.com/manticoresoftware/manticoresearch/issues/1111) 修复了 RT 表中 `global_idf` 的支持。需要重新创建表。
* ⚠️ 重大变更 [Issue #2103](https://github.com/manticoresoftware/manticoresearch/issues/2103) 从内部 `cjk` 字符集移除了泰文字符。请相应更新字符集定义：如果您有 `cjk,non_cjk` 并且泰文字符对您很重要，请更改为 `cjk,thai,non_cjk`，或使用新的所有连续脚本语言标识 `cont,non_cjk`（即包括 `cjk` + `thai`）。对现有表使用 [ALTER TABLE](Updating_table_schema_and_settings.md#Updating-table-FT-settings-in-RT-mode) 进行修改。
* ⚠️ 重大变更 [Issue #2468](https://github.com/manticoresoftware/manticoresearch/issues/2468) [CALL SUGGEST / QSUGGEST](Searching/Spell_correction.md#CALL-QSUGGEST,-CALL-SUGGEST) 现已支持分布式表。这提升了主从协议版本。如您在多实例分布式环境中运行 Manticore Search，请确保先升级代理(agent)，然后升级主节点(master)。
* ⚠️ 重大变更 [Issue #2889](https://github.com/manticoresoftware/manticoresearch/issues/2889) PQ [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META-for-PQ-tables) 中列名由 `Name` 更改为 `Variable name`。
* ⚠️ 重大变更 [Issue #879](https://github.com/manticoresoftware/manticoresearch/issues/879) 引入了[每表二进制日志](Logging/Binary_logging.md#Per-table-binary-logging-configuration)支持，新增选项：[binlog_common](Logging/Binary_logging.md#Binary-logging-strategies)、[binlog](Logging/Binary_logging.md#Per-table-binary-logging-configuration) 用于 `create table` / `alter table`。升级到新版本前需干净关闭 Manticore 实例。
* ⚠️ 重大变更 [Issue #1789](https://github.com/manticoresoftware/manticoresearch/issues/1789) 修正了当节点使用错误的复制协议版本加入集群时错误信息不正确的问题。此变更更新了复制协议。如果您运行的是复制集群，需要：
  - 首先，干净停止所有节点
  - 然后，使用 Linux 下的工具 `manticore_new_cluster`，带参数 `--new-cluster` 启动最后停止的节点。
  - 详细信息请阅读[重启集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster)。
* ⚠️ 重大变更 [Issue #2308](https://github.com/manticoresoftware/manticoresearch/issues/2308) 在[`ALTER CLUSTER ADD`和`DROP`](Creating_a_cluster/Setting_up_replication/Adding_and_removing_a_table_from_a_replication_cluster.md#Adding-and-removing-a-table-from-a-replication-cluster)命令中支持多个表。此变更也影响复制协议。有关如何处理此更新，请参阅上一节。
* [Issue #2997](https://github.com/manticoresoftware/manticoresearch/issues/2997) 修复了 Macos 上的 dlopen 问题。
* [Commit 4954](https://github.com/manticoresoftware/manticoresearch/commit/4954b5de7341a29902a9b8fbb9a040f7942c77c4) 更改了带有 KNN 索引的表的 OPTIMIZE TABLE 默认截止，提升搜索性能。
* [Commit cfc8](https://github.com/manticoresoftware/manticoresearch/commit/cfc87ecb6e33a8163c2235243b6b40e699dbf526) 为 `FACET` 和 `GROUP BY` 中的 `ORDER BY` 添加了 `COUNT(DISTINCT)` 支持。
* [Issue #1103](https://github.com/manticoresoftware/manticoresearch/issues/1103) 改进了[日志](Logging/Server_logging.md#Server-logging)块合并的清晰度。
* [Issue #1130](https://github.com/manticoresoftware/manticoresearch/issues/1130) 增加了对 [DBeaver](Integration/DBeaver.md) 的支持。
* [Issue #1546](https://github.com/manticoresoftware/manticoresearch/issues/1546) 为 [POLY2D()](Functions/Geo_spatial_functions.md#POLY2D%28%29) / [GEOPOLY2D()](Functions/Geo_spatial_functions.md#GEOPOLY2D%28%29) 函数实现了二级索引。
* [Issue #1630](https://github.com/manticoresoftware/manticoresearch/issues/1630) HTTP 请求现在支持 `Content-Encoding: gzip`。
* [Issue #1831](https://github.com/manticoresoftware/manticoresearch/issues/1831) 添加了 `SHOW LOCKS` 命令。
* [Issue #2187](https://github.com/manticoresoftware/manticoresearch/issues/2197) 允许 Buddy 请求绕过[searchd.max_connections](Server_settings/Searchd.md#max_connections)限制。
* [Issue #2208](https://github.com/manticoresoftware/manticoresearch/issues/2208) 增强了通过 JSON HTTP 接口的表连接支持。
* [Issue #2235](https://github.com/manticoresoftware/manticoresearch/issues/2235) 通过 Buddy 以原始形式记录成功处理的查询。
* [Issue #2249](https://github.com/manticoresoftware/manticoresearch/issues/2249) 增加了针对复制表运行 `mysqldump` 的特殊模式。
* [Issue #2268](https://github.com/manticoresoftware/manticoresearch/issues/2268) 改善了 `CREATE TABLE` 和 `ALTER TABLE` 语句对外部文件复制时的重命名。
* [Issue #2402](https://github.com/manticoresoftware/manticoresearch/issues/2402) 将[searchd.max_packet_size](Server_settings/Searchd.md#max_packet_size)默认值更新为 128MB。
* [Issue #2419](https://github.com/manticoresoftware/manticoresearch/issues/2419) 为 JSON ["match"](Searching/Full_text_matching/Basic_usage.md#match) 增加了[IDF 提升修饰符](Searching/Full_text_matching/Operators.md#IDF-boost-modifier)支持。
* [Issue #2430](https://github.com/manticoresoftware/manticoresearch/issues/2430) 加强了[二进制日志](Logging/Binary_logging.md#Binary-logging)写入同步以防止错误。
* [Issue #2458](https://github.com/manticoresoftware/manticoresearch/issues/2458) 在 Windows 包中集成了 zlib 支持。
* [Issue #2479](https://github.com/manticoresoftware/manticoresearch/issues/2479) 增加了 SHOW TABLE INDEXES 命令的支持。
* [Issue #2485](https://github.com/manticoresoftware/manticoresearch/issues/2485) 为 Buddy 回复设置了会话元数据。
* [Issue #2490](https://github.com/manticoresoftware/manticoresearch/issues/2490) 兼容端点的聚合支持毫秒级分辨率。
* [Issue #2500](https://github.com/manticoresoftware/manticoresearch/issues/2500) 集群操作在复制启动失败时更改了错误信息。
* [Issue #2584](https://github.com/manticoresoftware/manticoresearch/pull/2584) SHOW STATUS 中新增[性能指标](Node_info_and_management/Node_status.md#Query-Time-Statistics)：过去1、5和15分钟按查询类型的最小/最大/平均/第95百分位/第99百分位。
* [Issue #2639](https://github.com/manticoresoftware/manticoresearch/pull/2639) 请求和响应中的所有 `index` 实例均替换为 `table`。
* [Issue #2643](https://github.com/manticoresoftware/manticoresearch/issues/2643) 在 HTTP `/sql` 端点的聚合结果中新增 `distinct` 列。
* [Issue #268](https://github.com/manticoresoftware/manticoresearch-buddy/issues/268) 实现了从 Elasticsearch 导入数据类型的自动检测。
* [Issue #2744](https://github.com/manticoresoftware/manticoresearch/issues/2744) 为字符串 JSON 字段比较表达式添加了排序规则支持。
* [Issue #2752](https://github.com/manticoresoftware/manticoresearch/issues/2752) 在选择列表中添加了对 `uuid_short` 表达式的支持。
* [Issue #2783](https://github.com/manticoresoftware/manticoresearch/issues/2783) Manticore Search 现在直接运行 Buddy，而不再使用 `manticore-buddy` 包装器。
* [Issue #2785](https://github.com/manticoresoftware/manticoresearch/issues/2785) 区分缺失表和不支持插入操作的表的错误信息。
* [Issue #2789](https://github.com/manticoresoftware/manticoresearch/issues/2789) OpenSSL 3 现已静态编译进 `searchd`。
* [Issue #2790](https://github.com/manticoresoftware/manticoresearch/issues/2790) 增加了 `CALL uuid_short` 语句，用于生成包含多个 `uuid_short` 值的序列。
* [Issue #2803](https://github.com/manticoresoftware/manticoresearch/issues/2803) 为 JOIN 操作的右表添加了独立选项。
* [Issue #2810](https://github.com/manticoresoftware/manticoresearch/issues/2810) 提升了 HTTP JSON 聚合性能，使其匹配 SphinxQL 中的 `GROUP BY`。
* [Issue #2854](https://github.com/manticoresoftware/manticoresearch/issues/2854) 在 Kibana 日期相关请求中增加了对 `fixed_interval` 的支持。
* [Issue #2909](https://github.com/manticoresoftware/manticoresearch/issues/2909) 实现了 JOIN 查询的批处理，大幅提升了某些 JOIN 查询的性能，提升幅度达数百甚至上千倍。
* [Issue #2937](https://github.com/manticoresoftware/manticoresearch/issues/2937) 允许在 fullscan 查询中使用连接表的权重。
* [Issue #2953](https://github.com/manticoresoftware/manticoresearch/issues/2953) 修正了 join 查询的日志记录问题。
* [Issue #337](https://github.com/manticoresoftware/manticoresearch-buddy/issues/337) 在非调试模式下，隐藏了 Buddy 异常，不再记录到 `searchd` 日志。
* [Issue #2931](https://github.com/manticoresoftware/manticoresearch/issues/2931) 当用户为复制监听器设置错误端口时，守护进程关闭并显示错误信息。

### Bug 修复
* [Commit 0c6b](https://github.com/manticoresoftware/manticoresearch/commit/0c6bdaf41ea3684e05952e1a8273893535b923f5) 修复：包含超过 32 列的 JOIN 查询返回结果不正确。
* [Issue #2335](https://github.com/manticoresoftware/manticoresearch/issues/2335) 解决使用两个 json 属性作为条件时表连接失败的问题。
* [Issue #2338](https://github.com/manticoresoftware/manticoresearch/issues/2338) 修正多查询中带有 [cutoff](Searching/Options.md#cutoff) 导致的 total_relation 不正确问题。
* [Issue #2366](https://github.com/manticoresoftware/manticoresearch/issues/2366) 修正右表中按 `json.string` 过滤错误的问题，涉及 [表连接](Searching/Joining.md)。
* [Issue #2406](https://github.com/manticoresoftware/manticoresearch/issues/2406) 允许在所有 POST HTTP JSON 端点（插入/替换/批量）中使用 `null` 作为值，这时将采用默认值。
* [Issue #2418](https://github.com/manticoresoftware/manticoresearch/issues/2418) 通过调整初始套接字探测的 [max_packet_size](Server_settings/Searchd.md#max_packet_size) 网络缓冲区分配，优化了内存消耗。
* [Issue #2420](https://github.com/manticoresoftware/manticoresearch/issues/2420) 修正通过 JSON 接口插入无符号整型到 bigint 属性的问题。
* [Issue #2422](https://github.com/manticoresoftware/manticoresearch/issues/2422) 修正次级索引在启用 exclude 过滤器和 pseudo_sharding 时的工作异常。
* [Issue #2423](https://github.com/manticoresoftware/manticoresearch/issues/2423) 处理了 [manticore_new_cluster](Starting_the_server/Manually.md#searchd-command-line-options) 中的一个错误。
* [Issue #2448](https://github.com/manticoresoftware/manticoresearch/issues/2448) 解决了格式错误的 `_update` 请求导致守护进程崩溃的问题。
* [Issue #2452](https://github.com/manticoresoftware/manticoresearch/issues/2452) 修复了直方图无法处理带排除条件的值过滤器的问题。
* [Issue #55](https://github.com/manticoresoftware/columnar/issues/55) 修复了对分布式表进行的 [knn](Searching/KNN.md#KNN-vector-search) 查询问题。
* [Issue #68](https://github.com/manticoresoftware/columnar/issues/68) 改进了列式访问器中对表编码的排除过滤器处理。
* [Commit 0eb1](https://github.com/manticoresoftware/manticoresearch/commit/0eb1579462013cb134bcaf74e06f6eeb0c2ecc4d) 修正表达式解析器未遵循重新定义的 `thread_stack` 设置。
* [Commit c304](https://github.com/manticoresoftware/manticoresearch/commit/c3041fd1d3e14773f3c5aa635343915a71f5cb95) 修复克隆列式 IN 表达式时的崩溃。
* [Commit edad](https://github.com/manticoresoftware/columnar/commit/edadc694c68d6022bdd13134263667430a42cc1d) 修正位图迭代器中的反转问题，该问题导致崩溃。
* [Commit fc30](https://github.com/manticoresoftware/executor/commit/fc302ff1117b0b835a6f5a3c07057baf5fba14d9) 修复部分 Manticore 软件包被 `unattended-upgrades` 自动移除的问题。
* [Issue #1019](https://github.com/manticoresoftware/manticoresearch/issues/1019) 改进对 DbForge MySQL 工具生成查询的处理。
* [Issue #1107](https://github.com/manticoresoftware/manticoresearch/issues/1107) 修正 `CREATE TABLE` 和 `ALTER TABLE` 中特殊字符的转义问题。❤️ 感谢 [@subnix](https://github.com/subnix) 的 PR。
* [Issue #116](https://github.com/manticoresoftware/manticoresearch-backup/issues/116#issuecomment-2216597206) 修复在冻结索引中更新 blob 属性时出现的死锁。死锁由尝试取消冻结索引时锁冲突引起，也可能导致 manticore-backup 失败。
* [Issue #1818](https://github.com/manticoresoftware/manticoresearch/issues/1818) 当表被冻结时，`OPTIMIZE` 现在会抛出错误。
* [Issue #2001](https://github.com/manticoresoftware/manticoresearch/issues/2001) 允许将函数名用作列名。
* [Issue #2153](https://github.com/manticoresoftware/manticoresearch/issues/2153) 修复了查询包含未知磁盘分块的表设置时守护进程崩溃的问题。
* [Issue #2184](https://github.com/manticoresoftware/manticoresearch/issues/2184) 修复了在执行 `FREEZE` 和 `FLUSH RAMCHUNK` 后 `searchd` 停止时挂起的问题。
* [Issue #2228](https://github.com/manticoresoftware/manticoresearch/issues/2228) 从保留字中移除了与日期/时间相关的标记（及正则表达式）。
* [Issue #2255](https://github.com/manticoresoftware/manticoresearch/issues/2255) 修复了 `FACET` 在超过 5 个排序字段时崩溃的问题。
* [Issue #2265](https://github.com/manticoresoftware/manticoresearch/issues/2265) 修复了启用 `index_field_lengths` 时无法恢复 `mysqldump` 的问题。
* [Issue #2291](https://github.com/manticoresoftware/manticoresearch/issues/2291) 修复了执行 `ALTER TABLE` 命令时的崩溃。
* [Issue #2333](https://github.com/manticoresoftware/manticoresearch/issues/2333) 修复了 Windows 包中 MySQL DLL，确保索引器正常工作。
* [Issue #2393](https://github.com/manticoresoftware/manticoresearch/issues/2393) 修复了 GCC 编译错误。❤️ 感谢 [@animetosho](https://github.com/animetosho) 的 PR。
* [Issue #2447](https://github.com/manticoresoftware/manticoresearch/issues/2447) 修复了 [_update](Data_creation_and_modification/Updating_documents/REPLACE.md?client=Elasticsearch-like+partial) 中的转义问题。
* [Issue #2460](https://github.com/manticoresoftware/manticoresearch/issues/2460) 修复了声明多个相同名称的属性或字段时索引器崩溃的问题。
* [Issue #2467](https://github.com/manticoresoftware/manticoresearch/issues/2467) 解决了“兼容性”搜索相关端点中嵌套布尔查询错误转换导致守护进程崩溃的问题。
* [Issue #2493](https://github.com/manticoresoftware/manticoresearch/issues/2493) 修复了带修饰符短语的展开。
* [Issue #2535](https://github.com/manticoresoftware/manticoresearch/issues/2535) 解决了使用 [ZONE](Searching/Full_text_matching/Operators.md#ZONE-limit-operator) 或 [ZONESPAN](Searching/Full_text_matching/Operators.md#ZONESPAN-limit-operator) 全文操作符时守护进程崩溃的问题。
* [Issue #2552](https://github.com/manticoresoftware/manticoresearch/issues/2552) 修复了带关键词字典的普通表和 RT 表的中缀生成。
* [Issue #2571](https://github.com/manticoresoftware/manticoresearch/issues/2571) 修复了 `FACET` 查询中的错误回复；为带 `COUNT(*)` 的 `FACET` 设置默认排序顺序为 `DESC`。
* [Issue #2580](https://github.com/manticoresoftware/manticoresearch/issues/2580) 修复了 Windows 启动期间守护进程崩溃。
* [Issue #2603](https://github.com/manticoresoftware/manticoresearch/issues/2603) 修复了 HTTP 端点 `/sql` 和 `/sql?mode=raw` 的查询截断问题；使这些端点的请求无需 `query=` 头也能保持一致。
* [Issue #2623](https://github.com/manticoresoftware/manticoresearch/issues/2623) 修复了自动模式创建表但同时失败的问题。
* [Issue #2627](https://github.com/manticoresoftware/manticoresearch/issues/2627) 修复了 HNSW 库，支持加载多个 KNN 索引。
* [Issue #2630](https://github.com/manticoresoftware/manticoresearch/issues/2630) 修复了多条件同时发生时的冻结问题。
* [Issue #2645](https://github.com/manticoresoftware/manticoresearch/issues/2645) 修复了使用 `OR` 与 KNN 搜索时的致命错误崩溃。
* [Issue #2647](https://github.com/manticoresoftware/manticoresearch/issues/2647) 修复了 `indextool --mergeidf *.idf --out global.idf` 创建输出文件后删除该文件的问题。
* [Issue #2658](https://github.com/manticoresoftware/manticoresearch/issues/2658) 修复了外层 SELECT 中带 `ORDER BY` 字符串的子查询导致守护进程崩溃。
* [Issue #2686](https://github.com/manticoresoftware/manticoresearch/issues/2686) 修复了同时更新浮点属性和字符串属性时的崩溃。
* [Issue #2704](https://github.com/manticoresoftware/manticoresearch/issues/2704) 修复了 `lemmatize_xxx_all` 分词器中多个停用词导致后续标记 `hitpos` 增加的问题。
* [Issue #2708](https://github.com/manticoresoftware/manticoresearch/issues/2708) 修复了 `ALTER ... ADD COLUMN ... TEXT` 导致的崩溃。
* [Issue #2737](https://github.com/manticoresoftware/manticoresearch/issues/2737) 修复了在有至少一个 RAM 分块的冻结表中更新 blob 属性导致后续 `SELECT` 查询等待表解冻的问题。
* [Issue #2742](https://github.com/manticoresoftware/manticoresearch/issues/2742) 修复了带压缩因子的查询缓存跳过问题。
* [Issue #2775](https://github.com/manticoresoftware/manticoresearch/issues/2775) Manticore 现在在 `_bulk` 请求时对未知操作返回错误，而非崩溃。
* [Issue #2791](https://github.com/manticoresoftware/manticoresearch/issues/2791) 修复了 HTTP `_bulk` 端点返回插入文档 ID 的问题。
* [Issue #2797](https://github.com/manticoresoftware/manticoresearch/issues/2797) 修复了在处理多个表时，当其中有空表且另一个匹配条目数不同导致 grouper 崩溃的问题。
* [Issue #2835](https://github.com/manticoresoftware/manticoresearch/issues/2835) 修复了复杂 `SELECT` 查询中的崩溃。
* [Issue #2872](https://github.com/manticoresoftware/manticoresearch/issues/2872) 添加了错误信息，当 `IN` 表达式中的 `ALL` 或 `ANY` 参数不是 JSON 属性时提示。
* [Issue #2882](https://github.com/manticoresoftware/manticoresearch/issues/2882) 修复了在大表中更新 MVA 时守护进程崩溃的问题。
* [Issue #2888](https://github.com/manticoresoftware/manticoresearch/issues/2888) 修复了 `libstemmer` 分词失败时的崩溃。❤️ 感谢 [@subnix](https://github.com/subnix) 的 PR。
* [Issue #2919](https://github.com/manticoresoftware/manticoresearch/issues/2919) 修复了右表联接权重在表达式中不起作用的问题。
* [Issue #2919](https://github.com/manticoresoftware/manticoresearch/issues/2919) 修正了右连接表的权重在表达式中无效的问题。
* [Issue #325](https://github.com/manticoresoftware/manticoresearch-buddy/issues/325) 修正了当表已存在时，`CREATE TABLE IF NOT EXISTS ... WITH DATA` 失败的问题。
* [Issue #351](https://github.com/manticoresoftware/manticoresearch-buddy/issues/351) 修正了基于 KNN 的计数中，使用文档 ID 时出现的未定义数组键 "id" 错误。
* [Issue #359](https://github.com/manticoresoftware/manticoresearch-buddy/issues/359) 修正了 `REPLACE INTO cluster_name:table_name` 功能的问题。
* [Issue #67](https://github.com/manticoresoftware/docker/issues/67) 修正了使用 `--network=host` 运行 Manticore Docker 容器时出现的致命错误。

## 版本 6.3.8
发布日期：2024年11月22日

6.3.8 版本继续 6.3 系列，仅包含错误修复。

### 错误修复

* [PR #2777](https://github.com/manticoresoftware/manticoresearch/pull/2777) 修正了当查询并发受 `threads` 或 `max_threads_per_query` 设置限制时可用线程数的计算问题。

## manticore-extra v1.1.20

发布日期：2024年10月7日

### 错误修复

* [Issue #64](https://github.com/manticoresoftware/executor/issues/64) 解决了在基于 Debian 的系统中，自动安装包更新的 `unattended-upgrades` 工具错误标记多个 Manticore 包（包括 `manticore-galera`、`manticore-executor` 和 `manticore-columnar-lib`）为移除目标的问题。此问题是由于 `dpkg` 错误地将虚拟包 `manticore-extra` 视为冗余导致的。已作出变更，确保 `unattended-upgrades` 不再尝试移除重要的 Manticore 组件。

## 版本 6.3.6
发布日期：2024年8月2日

6.3.6 版本继续 6.3 系列，仅包含错误修复。

### 错误修复

* [Issue #2477](https://github.com/manticoresoftware/manticoresearch/issues/2477) 修正了 6.3.4 版本中引入的崩溃，该崩溃可在处理表达式以及分布式或多个表时发生。
* [Issue #2352](https://github.com/manticoresoftware/manticoresearch/issues/2352) 修正了在查询多个索引时，由于 `max_query_time` 过早退出导致的守护进程崩溃或内部错误。

## 版本 6.3.4
发布日期：2024年7月31日

6.3.4 版本继续 6.3 系列，仅包含小幅改进和错误修复。

### 小幅改动
* [Issue #1130](https://github.com/manticoresoftware/manticoresearch/issues/1130) 添加了对 [DBeaver](Integration/DBeaver.md#Integration-with-DBeaver) 的支持。
* [Issue #2146](https://github.com/manticoresoftware/manticoresearch/issues/2146) 改进了词形和异常中定界符的转义。
* [Issue #2315](https://github.com/manticoresoftware/manticoresearch/issues/2315) 为 SELECT 列表达式添加了字符串比较运算符。
* [Issue #2363](https://github.com/manticoresoftware/manticoresearch/issues/2363) 增加了对类似 Elastic 批量请求中 null 值的支持。
* [Issue #2374](https://github.com/manticoresoftware/manticoresearch/issues/2374) 增加了对 mysqldump 版本 9 的支持。
* [Issue #2375](https://github.com/manticoresoftware/manticoresearch/issues/2375) 改进了 HTTP JSON 查询中的错误处理，能够显示错误发生的 JSON 路径节点。

### 错误修复
* [Issue #2280](https://github.com/manticoresoftware/manticoresearch/issues/2280) 修正了当 disk_chunks 大于 1，且通配符查询匹配项众多时的性能退化问题。
* [Issue #2332](https://github.com/manticoresoftware/manticoresearch/issues/2332) 修正了空 MVA 数组在 MVA MIN 或 MAX SELECT 列表达式中导致的崩溃。
* [Issue #2339](https://github.com/manticoresoftware/manticoresearch/issues/2339) 修正了对 Kibana 无限范围请求的错误处理。
* [Issue #2342](https://github.com/manticoresoftware/manticoresearch/issues/2342) 修正了当连接过滤条件来自右表的列存属性且该属性未在 SELECT 列表中时的错误。
* [Issue #2343](https://github.com/manticoresoftware/manticoresearch/issues/2343) 修正了 Manticore 6.3.2 中重复声明的 'static' 说明符。
* [Issue #2344](https://github.com/manticoresoftware/manticoresearch/issues/2344) 修正了当 MATCH() 运算符用于右表时，LEFT JOIN 返回了不匹配记录的问题。
* [Issue #2350](https://github.com/manticoresoftware/manticoresearch/issues/2350) 修正了在带有 `hitless_words` 的 RT 索引中磁盘分块保存的问题。
* [Issue #2364](https://github.com/manticoresoftware/manticoresearch/issues/2364) ‘aggs_node_sort’ 属性现在可以与其他属性任意顺序添加。
* [Issue #2368](https://github.com/manticoresoftware/manticoresearch/issues/2368) 修正了 JSON 查询中过滤顺序与全文搜索顺序出错的问题。
* [Issue #2376](https://github.com/manticoresoftware/manticoresearch/issues/2376) 修正了针对长 UTF-8 请求返回错误 JSON 响应的缺陷。
* [Issue #2684](https://github.com/manticoresoftware/dev/issues/2684) 修正了对依赖连接属性的预排序/预过滤表达式计算。
* [Issue #301](https://github.com/manticoresoftware/manticoresearch-buddy/issues/301) 修改了计算指标数据大小的方法，从读取 `manticore.json` 文件改为不再检查整个数据目录大小。
* [Issue #302](https://github.com/manticoresoftware/manticoresearch-buddy/issues/302) 增加了对 Vector.dev 验证请求的处理。

## 版本 6.3.2
发布日期：2024年6月26日

6.3.2 版本继续 6.3 系列，包含多项错误修复，其中部分错误是在 6.3.0 发布后发现的。

### 重大变更
* ⚠️[Issue #2305](https://github.com/manticoresoftware/manticoresearch/issues/2305) 更新了 aggs.range 的值类型为数值型。

### 错误修复
* [Commit c51c](https://github.com/manticoresoftware/manticoresearch/commit/c51c7fa4bd0ec4ad8d53188b888461bff29f1417) 修正了存储检查与 rset 合并时的分组问题。
* [提交 0e85](https://github.com/manticoresoftware/manticoresearch/commit/0e85b9d5682f51681750d798aef6a11cfb93d69e) 修复在使用 CRC 字典和启用 `local_df` 的 RT 索引中使用通配符查询时守护进程崩溃的问题。
* [问题 #2200](https://github.com/manticoresoftware/manticoresearch/issues/2200) 修复在没有 GROUP BY 的情况下，JOIN 对 `count(*)` 导致崩溃的问题。
* [问题 #2201](https://github.com/manticoresoftware/manticoresearch/issues/2201) 修复尝试对全文字段分组时，JOIN 没有返回警告的问题。
* [问题 #2230](https://github.com/manticoresoftware/manticoresearch/issues/2230) 解决通过 `ALTER TABLE` 添加属性时未考虑 KNN 选项的问题。
* [问题 #2231](https://github.com/manticoresoftware/manticoresearch/issues/2231) 修复版本 6.3.0 中删除 `manticore-tools` Redhat 包失败的问题。
* [问题 #2242](https://github.com/manticoresoftware/manticoresearch/issues/2242) 修正 JOIN 和多个 FACET 语句返回结果不正确的问题。
* [问题 #2250](https://github.com/manticoresoftware/manticoresearch/issues/2250) 修复 ALTER TABLE 在表位于集群中时产生错误的问题。
* [问题 #2252](https://github.com/manticoresoftware/manticoresearch/issues/2252) 修复从 SphinxQL 接口将原始查询传递给 buddy 的问题。
* [问题 #2267](https://github.com/manticoresoftware/manticoresearch/issues/2267) 改进了带有磁盘块的 RT 索引中 `CALL KEYWORDS` 的通配符展开。
* [问题 #271](https://github.com/manticoresoftware/manticoresearch-buddy/issues/271) 修复错误的 `/cli` 请求导致的挂起问题。
* [问题 #274](https://github.com/manticoresoftware/manticoresearch-buddy/issues/274) 解决对 Manticore 的并发请求可能卡住的问题。
* [问题 #275](https://github.com/manticoresoftware/manticoresearch-buddy/issues/275) 修复通过 `/cli` 执行 `drop table if exists t; create table t` 导致的挂起问题。

### 复制相关
* [问题 #2270](https://github.com/manticoresoftware/manticoresearch/issues/2270) 在 `/_bulk` HTTP 端点中添加对 `cluster:name` 格式的支持。

## 版本 6.3.0
发布：2024年5月23日

### 主要变更
* [问题 #839](https://github.com/manticoresoftware/manticoresearch/issues/839) 实现了 [float_vector](Creating_a_table/Data_types.md#Float-vector) 数据类型；实现了 [vector search](Searching/KNN.md#KNN-vector-search)。
* [问题 #1673](https://github.com/manticoresoftware/manticoresearch/issues/1673) [INNER/LEFT JOIN](Searching/Joining.md) （**测试阶段**）。
* [问题 #1744](https://github.com/manticoresoftware/manticoresearch/issues/1744) 实现了对 [timestamp](Creating_a_table/Data_types.md#Timestamps) 字段自动检测日期格式。
* [问题 #1720](https://github.com/manticoresoftware/manticoresearch/pull/1720) 将 Manticore Search 许可证从 GPLv2-or-later 更改为 GPLv3-or-later。
* [提交 7a55](https://github.com/manticoresoftware/manticoresearch/commit/7a5508424) 在 Windows 上运行 Manticore 现在需要通过 Docker 运行 Buddy。
* [问题 #1541](https://github.com/manticoresoftware/manticoresearch/issues/1541) 添加了一个 [REGEX](Searching/Full_text_matching/Operators.md#REGEX-operator) 全文操作符。
* [问题 #2091](https://github.com/manticoresoftware/manticoresearch/issues/2091) 支持 Ubuntu Noble 24.04。
* [提交 514d](https://github.com/manticoresoftware/manticoresearch/commit/514d35b497f4bdb20b6473ff963752a978a4bb8d) 改进时间操作以提升性能并新增日期/时间函数：
  - [CURDATE()](Functions/Date_and_time_functions.md#CURDATE%28%29) - 返回本地时区当前日期
  - [QUARTER()](Functions/Date_and_time_functions.md#QUARTER%28%29) - 返回来自时间戳参数的整数季度
  - [DAYNAME()](Functions/Date_and_time_functions.md#DAYNAME%28%29) - 返回给定时间戳参数的星期名
  - [MONTHNAME()](Functions/Date_and_time_functions.md#MONTHNAME%28%29) - 返回给定时间戳参数的月份名称
  - [DAYOFWEEK()](Functions/Date_and_time_functions.md#DAYOFWEEK%28%29) - 返回给定时间戳参数的整数星期几索引
  - [DAYOFYEAR()](Functions/Date_and_time_functions.md#DAYOFYEAR%28%29) - 返回给定时间戳参数的整数年中日
  - [YEARWEEK()](Functions/Date_and_time_functions.md#YEARWEEK%28%29) - 返回给定时间戳参数的整数年和当前周第一天的日代码
  - [DATEDIFF()](Functions/Date_and_time_functions.md#DATEDIFF%28%29) - 返回两个给定时间戳之间的天数
  - [DATE()](Functions/Date_and_time_functions.md#DATE%28%29) - 格式化时间戳参数中的日期部分
  - [TIME()](Functions/Date_and_time_functions.md#TIME%28%29) - 格式化时间戳参数中的时间部分
  - [timezone](Server_settings/Searchd.md#timezone) - 日期/时间相关函数所使用的时区。
* [提交 30e7](https://github.com/manticoresoftware/manticoresearch/commit/30e789d9657f9b3093c280c6e5b0e46a021f6dc7) 向 HTTP 接口添加了 [range](Searching/Faceted_search.md#Facet-over-set-of-ranges)、[histogram](Searching/Faceted_search.md#Facet-over-histogram-values)、[date_range](Searching/Faceted_search.md#Facet-over-set-of-date-ranges) 和 [date_histogram](Searching/Faceted_search.md#Facet-over-histogram-date-values) 聚合，同时在 SQL 中支持类似表达式。

### 次要变更
* [问题 #1285](https://github.com/manticoresoftware/manticoresearch/issues/1285) 支持 Filebeat 版本 8.10 - 8.11。
* [问题 #1771](https://github.com/manticoresoftware/manticoresearch/issues/1771) 支持 [ALTER TABLE ... type='distributed'](Updating_table_schema_and_settings.md#Changing-a-distributed-table)。
* [问题 #1788](https://github.com/manticoresoftware/manticoresearch/issues/1788) 添加了使用 [CREATE TABLE ... LIKE ... WITH DATA](Creating_a_table/Local_tables/Real-time_table.md#CREATE-TABLE-LIKE:) SQL 语句复制表的能力。
* [Issue #2072](https://github.com/manticoresoftware/manticoresearch/issues/2072) 优化了[表压缩算法](Securing_and_compacting_a_table/Compacting_a_table.md#Compacting-a-Table)：此前，手动的[OPTIMIZE](Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE)和自动的[auto_optimize](Server_settings/Searchd.md#auto_optimize)流程都会先合并数据块以确保数量未超过限制，然后对所有包含已删除文档的其他数据块执行清理已删除文档操作。此方法有时资源消耗过大，已被禁用。现在，数据块合并仅按照[progressive_merge](Server_settings/Common.md#progressive_merge)设置进行。然而，包含大量已删除文档的数据块更有可能被合并。
* [Commit ce6c](https://github.com/manticoresoftware/columnar/commit/ce6c5d3381edcdc411a00a5923d13d56dd9b3010) 添加了对加载新版本二级索引的保护措施。
* [Issue #1417](https://github.com/manticoresoftware/manticoresearch/issues/1417) 通过[REPLACE INTO ... SET](Data_creation_and_modification/Updating_documents/REPLACE.md#SQL-REPLACE)实现部分替换。
* [Commit 7c16](https://github.com/manticoresoftware/manticoresearch/commit/7c167b934ce416d3dfbd658744a231e638b14d9a) 更新默认合并缓冲区大小：`.spa`（标量属性）：256KB -> 8MB；`.spb`（二进制大对象属性）：256KB -> 8MB；`.spc`（列式属性）：1MB，无变化；`.spds`（文档存储）：256KB -> 8MB；`.spidx`（二级索引）：256KB缓冲区 -> 128MB内存限制；`.spi`（字典）：256KB -> 16MB；`.spd`（文档列表）：8MB，无变化；`.spp`（命中列表）：8MB，无变化；`.spe`（跳表）：256KB -> 8MB。
* [Issue #1859](https://github.com/manticoresoftware/manticoresearch/issues/1859) 通过 JSON 增加了[复合聚合](Searching/Grouping.md#GROUP-BY-multiple-fields-at-once)。
* [Commit 216b](https://github.com/manticoresoftware/manticoresearch-buddy/commit/216b824) 由于部分正则表达式模式存在问题且未带来显著时间收益，禁用了 PCRE.JIT。
* [Commit 55cd](https://github.com/manticoresoftware/manticoresearch/commit/55cdce32) 增加了对经典 Galera v.3（api v25）（来自 MySQL 5.x 的 `libgalera_smm.so`）的支持。
* [Commit 86f9](https://github.com/manticoresoftware/manticoresearch-buddy/commit/86f98ab) 指标后缀从 `_rate` 改为 `_rps`。
* [Commit c0c1](https://github.com/manticoresoftware/manticoresearch-helm/commit/c0c1cff780031412ef57cefab987dfd16613368e) 改进了关于负载均衡器高可用支持的文档。
* [Commit d1d2](https://github.com/manticoresoftware/manticoresearch/commit/d1d2ca18857c68106e89743756260e73285be739) 错误信息中的 `index` 改为 `table`；修复 bison 解析器错误信息。
* [Commit fd26](https://github.com/manticoresoftware/manticoresearch/commit/fd26671b) 支持 `manticore.tbl` 作为表名。
* [Issue #1105](https://github.com/manticoresoftware/manticoresearch/issues/1105) 支持通过 systemd 运行索引器（[文档](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Schedule-indexer-via-systemd)）。❤️ 感谢 [@subnix](https://github.com/subnix) 的 PR。
* [Issue #1294](https://github.com/manticoresoftware/manticoresearch/issues/1294) GEODIST() 中的二级索引支持。
* [Issue #1394](https://github.com/manticoresoftware/manticoresearch/issues/1394) 简化了[SHOW THREADS](Node_info_and_management/SHOW_THREADS.md#SHOW-THREADS)。
* [Issue #1424](https://github.com/manticoresoftware/manticoresearch/issues/1424) 为 `create distributed table` 语句添加了默认值支持（`agent_connect_timeout` 和 `agent_query_timeout`）。
* [Issue #1442](https://github.com/manticoresoftware/manticoresearch/issues/1442) 新增了搜索查询选项 [expansion_limit](Searching/Options.md#expansion_limit)，可覆盖 `searchd.expansion_limit`。
* [Issue #1448](https://github.com/manticoresoftware/manticoresearch/issues/1448) 实现了 [ALTER TABLE](Updating_table_schema_and_settings.md#Updating-table-schema-in-RT-mode) 进行 int 到 bigint 的转换。
* [Issue #1456](https://github.com/manticoresoftware/manticoresearch/issues/1456) MySQL 响应中增加元信息。
* [Issue #1494](https://github.com/manticoresoftware/manticoresearch/issues/1494) [SHOW VERSION](Node_info_and_management/SHOW_VERSION.md#SHOW-VERSION)。
* [Issue #1582](https://github.com/manticoresoftware/manticoresearch/issues/1582) 通过 JSON 支持[根据 ID 数组删除文档](Data_creation_and_modification/Deleting_documents.md?client=JSON#Deleting-documents)。
* [Issue #1589](https://github.com/manticoresoftware/manticoresearch/issues/1589) 改进“unsupported value type”错误提示。
* [Issue #1634](https://github.com/manticoresoftware/manticoresearch/issues/1634) 在 `SHOW STATUS` 中增加 Buddy 版本信息。
* [Issue #1641](https://github.com/manticoresoftware/manticoresearch/issues/1641) 针对关键词文档数为零时优化匹配请求。
* [Issue #1712](https://github.com/manticoresoftware/manticoresearch/issues/1712) 在提交数据时增加从字符串“true”和“false”转换为布尔属性的功能。
* [Issue #1713](https://github.com/manticoresoftware/manticoresearch/issues/1713) 新增了 [access_dict](Server_settings/Searchd.md#access_dict) 表和 searchd 选项。
* [Issue #1767](https://github.com/manticoresoftware/manticoresearch/issues/1767) 在配置文件的 searchd 部分新增选项：[expansion_merge_threshold_docs](Server_settings/Searchd.md#expansion_merge_threshold_docs) 和 [expansion_merge_threshold_hits](Server_settings/Searchd.md#expansion_merge_threshold_hits)；使展开词的微小词合并阈值可配置。
* [Issue #1768](https://github.com/manticoresoftware/manticoresearch/issues/1768) 在 `@@system.sessions` 中新增显示最后一次命令时间。
* [Issue #1806](https://github.com/manticoresoftware/manticoresearch/issues/1806) 升级 Buddy 协议至版本 2。
* [问题 #1810](https://github.com/manticoresoftware/manticoresearch/issues/1810) 为 `/sql` 端点实现了额外的请求格式，以简化与库的集成。
* [问题 #1825](https://github.com/manticoresoftware/manticoresearch/pull/1825) 在 SHOW BUDDY PLUGINS 中添加了信息部分。
* [问题 #1837](https://github.com/manticoresoftware/manticoresearch/issues/1837) 改进了 `CALL PQ` 在处理大数据包时的内存消耗。
* [问题 #1853](https://github.com/manticoresoftware/manticoresearch/pull/1853) 将编译器从 Clang 15 切换到 Clang 16。
* [问题 #1857](https://github.com/manticoresoftware/manticoresearch/issues/1857) 添加了字符串比较。❤️ 感谢 [@etcd](https://github.com/etcd) 的 PR。
* [问题 #1915](https://github.com/manticoresoftware/manticoresearch/issues/1915) 添加了对连接存储字段的支持。
* [问题 #1937](https://github.com/manticoresoftware/manticoresearch/issues/1937) 在查询日志中记录客户端的主机:端口。
* [问题 #1981](https://github.com/manticoresoftware/manticoresearch/issues/1981) 修正了错误的错误信息。
* [问题 #1983](https://github.com/manticoresoftware/manticoresearch/issues/1983) 为通过 [JSON 查询计划](Node_info_and_management/Profiling/Query_plan.md?client=JSON#Query-plan) 引入了详细级别支持。
* [问题 #2010](https://github.com/manticoresoftware/manticoresearch/issues/2010) 禁止日志记录来自 Buddy 的查询，除非设置了 `log_level=debug`。
* [问题 #2035](https://github.com/manticoresoftware/manticoresearch/issues/2035) 支持 Linux Mint 21.3。
* [问题 #2056](https://github.com/manticoresoftware/manticoresearch/issues/2056) Manticore 无法与 Mysql 8.3+ 一起构建。
* [问题 #2112](https://github.com/manticoresoftware/manticoresearch/issues/2112) 为实时表块添加了 `DEBUG DEDUP` 命令，该命令可应对附加包含重复数据的普通表后出现的重复条目。
* [问题 #212](https://github.com/manticoresoftware/manticoresearch-buddy/issues/212) 在 [SHOW QUERIES](Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES) 中添加了时间。
* [问题 #218](https://github.com/manticoresoftware/manticoresearch-buddy/issues/218) 将 `@timestamp` 列作为时间戳处理。
* [问题 #252](https://github.com/manticoresoftware/manticoresearch-buddy/issues/250) 实现了启用/禁用 buddy 插件的逻辑。
* [问题 #254](https://github.com/manticoresoftware/manticoresearch-buddy/issues/254) 将 composer 更新到修复了最近 CVE 的较新版本。
* [问题 #340](https://github.com/manticoresoftware/manticoresearch/issues/340) 对与 `RuntimeDirectory` 相关的 Manticore systemd 单元进行了小幅优化。
* [问题 #51](https://github.com/manticoresoftware/executor/issues/51) 添加了 rdkafka 支持并升级到 PHP 8.3.3。
* [问题 #527](https://github.com/manticoresoftware/manticoresearch/issues/527) 支持 [附加](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Attaching_one_table_to_another.md#Attaching-one-table-to-another) 一个 RT 表。新增命令 [ALTER TABLE ... RENAME](Updating_table_schema_and_settings.md#Renaming-a-real-time-table)。

### 重大变更与弃用
* ⚠️[问题 #1436](https://github.com/manticoresoftware/manticoresearch/issues/1436) 修复了 IDF 计算问题。`local_df` 现为默认设置。改进了主-代理搜索协议（版本已更新）。如果您在分布式环境中运行多个 Manticore Search 实例，请确保先升级代理端，再升级主节点。
* ⚠️[问题 #1572](https://github.com/manticoresoftware/manticoresearch/issues/1572) 添加了分布式表的复制功能并更新了复制协议。如果您运行复制集群，需要：
  - 首先，干净地停止所有节点
  - 然后，使用 Linux 上的 `manticore_new_cluster` 工具以 `--new-cluster` 参数启动最后停止的节点。
  - 更多信息请参阅关于 [重新启动集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster) 的说明。
* ⚠️[问题 #1763](https://github.com/manticoresoftware/manticoresearch/pull/1763) HTTP API 端点别名 `/json/*` 已被弃用。
* ⚠️[问题 #1982](https://github.com/manticoresoftware/manticoresearch/issues/1982) 将 JSON 中的 [profile](Node_info_and_management/Profiling/Query_profile.md#Query-profile) 改为 [plan](Node_info_and_management/Profiling/Query_plan.md#Query-plan)，并新增了 JSON 查询性能分析。
* ⚠️[提交 e235](https://github.com/manticoresoftware/manticoresearch-backup/commit/e23585a) manticore-backup 不再备份 `plugin_dir`。
* ⚠️[问题 #171](https://github.com/manticoresoftware/manticoresearch-buddy/issues/155) 迁移 Buddy 到 Swoole 以提升性能和稳定性。切换到新版本时，请确保所有 Manticore 包均已更新。
* ⚠️[问题 #196](https://github.com/manticoresoftware/manticoresearch-buddy/pull/205) 将所有核心插件合并到 Buddy，修改了核心逻辑。
* ⚠️[问题 #2107](https://github.com/manticoresoftware/manticoresearch/issues/2107) 在 `/search` 响应中将文档 ID 视为数字。
* ⚠️[问题 #38](https://github.com/manticoresoftware/executor/issues/38) 添加了 Swoole，禁用了 ZTS，移除了 parallel 扩展。
* ⚠️[问题 #1929](https://github.com/manticoresoftware/manticoresearch/issues/1929) 在某些情况下，`charset_table` 中的覆盖未生效。

### 复制相关改动
* [提交 3376](https://github.com/manticoresoftware/manticoresearch/commit/3376351f73db3ff0cb6cb708a304a7bd83e69cfb) 修复了大文件 SST 复制错误。
* [提交 6d36](https://github.com/manticoresoftware/manticoresearch/commit/6d36c68fb7ce3cb274b467aac435788905084952) 为复制命令添加了重试机制；修复了网络繁忙且丢包情况下复制加入失败的问题。
* [提交 842e](https://github.com/manticoresoftware/manticoresearch/commit/842e27e71790833ab4125d49103d2933d5df60c3) 将复制中的 FATAL 消息改为 WARNING 消息。
* [Commit 8c32](https://github.com/manticoresoftware/manticoresearch/commit/8c32bce7db39d8ed3b1a06aaa1d2c3ec372084f0) 修复了在无表或空表的复制集群中计算 `gcache.page_size` 的问题；同时修复了 Galera 选项的保存和加载。
* [Commit a2af](https://github.com/manticoresoftware/manticoresearch/commit/a2af06ca36abcbf893e43c8803853130b397c54b) 添加了功能以跳过加入集群节点上的更新节点复制命令。
* [Commit c054](https://github.com/manticoresoftware/manticoresearch/commit/c0541e6892f5116b1f957b40060ba8ecd01a5afb) 修复了在更新 blob 属性与替换文档时复制过程中的死锁问题。
* [Commit e80d](https://github.com/manticoresoftware/manticoresearch/commit/e80d505b963999016033613c40c1cf3151d648e7) 添加了 [replication_connect_timeout](Server_settings/Searchd.md#replication_connect_timeout)、[replication_query_timeout](Server_settings/Searchd.md#replication_query_timeout)、[replication_retry_delay](Server_settings/Searchd.md#replication_retry_delay)、[replication_retry_count](Server_settings/Searchd.md#replication_retry_count) searchd 配置选项，用于控制复制过程中的网络行为，类似于 `searchd.agent_*` 配置，但默认值不同。
* [Issue #1356](https://github.com/manticoresoftware/manticoresearch/issues/1356) 修复了复制节点在一些节点丢失且名称解析失败后的重试问题。
* [Issue #1445](https://github.com/manticoresoftware/manticoresearch/issues/1445) 修复了在 `show variables` 中复制日志详细级别的问题。
* [Issue #1482](https://github.com/manticoresoftware/manticoresearch/issues/1482) 修复了 Kubernetes 中 pod 重启时，加入节点连接到集群的复制问题。
* [Issue #1962](https://github.com/manticoresoftware/manticoresearch/issues/1962) 修复了在无效节点名的空集群上执行复制修改时长时间等待的问题。

### Bug 修复
* [Commit 8a48](https://github.com/manticoresoftware/manticoresearch/commit/8a486c7ba1a293879cdecc41fb52eb7ab0cbf832) 修复了 `count distinct` 中未使用匹配项清理引发的崩溃问题。
* [Issue #1569](https://github.com/manticoresoftware/manticoresearch/issues/1569) 二进制日志现已以事务粒度进行写入。
* [Issue #2089](https://github.com/manticoresoftware/manticoresearch/issues/2089) 修复了与 64 位 ID 相关的错误，该错误可能在通过 MySQL 插入数据时导致“Malformed packet”错误，进而导致[表损坏和 ID 重复](https://github.com/manticoresoftware/manticoresearch/issues/2090)。
* [Issue #2160](https://github.com/manticoresoftware/manticoresearch/issues/2160) 修复了日期被插入时按 UTC 处理而非本地时区的问题。
* [Issue #2177](https://github.com/manticoresoftware/manticoresearch/issues/2177) 修复了当使用非空 `index_token_filter` 在实时表中执行搜索时的崩溃问题。
* [Issue #2209](https://github.com/manticoresoftware/manticoresearch/issues/2209) 修改了 RT 列存储中的重复过滤，修复了崩溃和错误查询结果的问题。
* [Commit 001d](https://github.com/manticoresoftware/manticoresearch/commit/001d82904e4b887c49387308291aeefb07c0fbc4) 修复了处理联合字段后 html stripper 导致内存损坏的问题。
* [Commit 00eb](https://github.com/manticoresoftware/manticoresearch/commit/00eb2ebf) 避免在 flush 后回绕流，防止与 mysqldump 之间的通信问题。
* [Commit 0553](https://github.com/manticoresoftware/manticoresearch/commit/05534114acf08c70ffcc41a23d558dec09a34373) 若 preread 未启动，则不等待其完成。
* [Commit 055a](https://github.com/manticoresoftware/manticoresearch/commit/055aac09f877570cd29b6594cb283182eba7f73b) 修复了 Buddy 输出字符串过大，未在 searchd 日志中分行显示的问题。
* [Commit 0a88](https://github.com/manticoresoftware/manticoresearch/commit/0a88056307f82c50fcdc458ad6dbc307a00e3f92) 调整了 MySQL 接口有关调试详细级别 `debugv` 失败的警告。
* [Commit 150a](https://github.com/manticoresoftware/manticoresearch/commit/150a20286b7d98d24b844ae577d7fda4a444798d) 修复了多个集群管理操作中的竞态条件；禁止创建同名或同路径的多个集群。
* [Commit 2e40](https://github.com/manticoresoftware/manticoresearch/commit/2e405c1c9ebac6f46206e3df17ac1c19129c9aa3) 修复全文查询中隐式截断问题；将 MatchExtended 分拆为模板部分。
* [Commit 75f5](https://github.com/manticoresoftware/manticoresearch/commit/75f51421542a2a0658f9e2f9f31048c8c5224479) 修复了 `index_exact_words` 在索引和加载表到守护进程之间的不一致。
* [Commit 7643](https://github.com/manticoresoftware/manticoresearch/commit/7643976bea142e08535d701000eff0d09a99fda2) 修复了无效集群删除时缺少错误信息的问题。
* [Commit 7a03](https://github.com/manticoresoftware/manticoresearch/commit/7a034ac990d5ec9344320c05a6a2315583b9b858) 修复了 CBO 与队列联合以及 CBO 与 RT 伪分片的问题。
* [Commit 7b4e](https://github.com/manticoresoftware/manticoresearch/commit/7b4ea95e) 启动时如果未配置辅助索引（SI）库和参数，避免发出误导性警告信息 “WARNING: secondary_indexes set but failed to initialize secondary library”。
* [Commit 8496](https://github.com/manticoresoftware/manticoresearch/commit/8496317f099fade8d2db463fb88483e7fcdfe85b) 修复了仲裁排序中的命中排序问题。
* [Commit 8973](https://github.com/manticoresoftware/manticoresearch-buddy/commit/8973ad3) 修复了 ModifyTable 插件中大写选项的问题。
* [Commit 9935](https://github.com/manticoresoftware/manticoresearch-buddy/commit/99350d4c184379b19f3efa9751d9b5be5e6edfb5) 修复了从包含空 JSON 值（表示为 NULL）的转储恢复的问题。
* [Commit a28f](https://github.com/manticoresoftware/manticoresearch/commit/a28f6e087b3d957e48816070be17d0823904936a) 通过使用 pcon 修复了在接收 SST 时加入节点的 SST 超时问题。
* [提交 b5a5](https://github.com/manticoresoftware/manticoresearch/commit/b5a55cc26debcddcda5d9caef8366a707e0dd763) 修复选择别名字符串属性时的崩溃。
* [提交 c556](https://github.com/manticoresoftware/manticoresearch/commit/c556e7ed8b6da8eac8ef094650df5f0a8977916b) 针对具有 `morphology_skip_fields` 字段的全文查询，添加了将术语转换为 `=term` 的查询转换。
* [提交 cdc3](https://github.com/manticoresoftware/manticoresearch/commit/cdc38085145cd1025b96fcac2a46188a0425d9e1) 添加缺失的配置键（skiplist_cache_size）。
* [提交 cf6e](https://github.com/manticoresoftware/manticoresearch/commit/cf6ec3bfdc3f0990fb31ff53822b2331a6667727) 修复复杂大型查询时表达式排序器的崩溃。
* [提交 e513](https://github.com/manticoresoftware/manticoresearch/commit/e5139ba75eef7223df375ae3cc3f138fa7d925f1) 修复全文CBO与无效索引提示的冲突。
* [提交 eb05](https://github.com/manticoresoftware/manticoresearch/commit/eb0514b3a6f57f278252b0724eebd112ec116e0f) 在关闭时中断预读以加速关闭。
* [提交 f945](https://github.com/manticoresoftware/manticoresearch/commit/f94555a293cfa51004ff81135b8bd8500b955115) 修改全文查询的堆栈计算，避免复杂查询时崩溃。
* [问题 #1262](https://github.com/manticoresoftware/manticoresearch/issues/1262) 修复索引器在索引包含多个同名列的 SQL 源时崩溃。
* [问题 #1273](https://github.com/manticoresoftware/manticoresearch/issues/1273) 非存在的系统变量返回 0 而非 <empty>。
* [问题 #1289](https://github.com/manticoresoftware/manticoresearch/issues/1289) 修复 indextool 检查 RT 表外部文件时报错。
* [问题 #1335](https://github.com/manticoresoftware/manticoresearch/issues/1335) 修复语句中多词形导致的查询解析错误。
* [问题 #1364](https://github.com/manticoresoftware/manticoresearch/issues/1364) 添加对旧版本 binlog 空文件的重放支持。
* [问题 #1365](https://github.com/manticoresoftware/manticoresearch/issues/1365) 修复删除最后一个空 binlog 文件的问题。
* [问题 #1372](https://github.com/manticoresoftware/manticoresearch/issues/1372) 修复数据目录变更影响守护进程启动时当前工作目录，导致相对路径错误转为绝对路径的问题。
* [问题 #1393](https://github.com/manticoresoftware/manticoresearch/issues/1393) hn_small 中最慢时间退化：在守护进程启动时获取/缓存 CPU 信息。
* [问题 #1395](https://github.com/manticoresoftware/manticoresearch/issues/1395) 修复索引加载期间缺少外部文件的警告。
* [问题 #1402](https://github.com/manticoresoftware/manticoresearch/issues/1402) 修复全局分组器在释放数据指针属性时崩溃。
* [问题 #1403](https://github.com/manticoresoftware/manticoresearch/issues/1403) _ADDITIONAL_SEARCHD_PARAMS 不生效。
* [问题 #1427](https://github.com/manticoresoftware/manticoresearch/issues/1427) 修复每表的 `agent_query_timeout` 被默认查询选项的 `agent_query_timeout` 替换的问题。
* [问题 #1444](https://github.com/manticoresoftware/manticoresearch/issues/1444) 修复使用 `packedfactors()` 且每匹配项有多个值时，分组器和排序器崩溃。
* [问题 #1458](https://github.com/manticoresoftware/manticoresearch/issues/1458) Manticore 在频繁索引更新时崩溃。
* [问题 #1481](https://github.com/manticoresoftware/manticoresearch/issues/1481) 修复解析错误后清理解析查询时崩溃。
* [问题 #1484](https://github.com/manticoresoftware/manticoresearch/issues/1484) 修复 HTTP JSON 请求未路由到伙伴节点的问题。
* [问题 #1499](https://github.com/manticoresoftware/manticoresearch/issues/1499) JSON 属性根值不能为数组的问题已修复。
* [问题 #1507](https://github.com/manticoresoftware/manticoresearch/issues/1507) 修复事务中重建表时崩溃。
* [问题 #1515](https://github.com/manticoresoftware/manticoresearch/issues/1515) 修复RU词干短形态的展开。
* [问题 #1579](https://github.com/manticoresoftware/manticoresearch/issues/1579) 修复 [date_format](Functions/Date_and_time_functions.md#DATE_FORMAT()) 表达式中 JSON 和 STRING 属性的使用。
* [问题 #1580](https://github.com/manticoresoftware/manticoresearch/issues/1580) 修复对 JSON 字段多个别名的分组器。
* [问题 #1594](https://github.com/manticoresoftware/manticoresearch/issues/1594) 修复开发中 total_related 错误：修正隐式截止 vs 限制；为 JSON 查询增加更好的全扫描检测。
* [问题 #1603](https://github.com/manticoresoftware/manticoresearch/issues/1603) 修复所有日期表达式中 JSON 和 STRING 属性的使用。
* [问题 #1609](https://github.com/manticoresoftware/manticoresearch/issues/1609) 使用 LEVENSHTEIN() 时崩溃。
* [问题 #1612](https://github.com/manticoresoftware/manticoresearch/issues/1612) 修复带高亮的搜索查询解析错误后导致的内存损坏。
* [问题 #1614](https://github.com/manticoresoftware/manticoresearch/issues/1614) 禁用对长度小于 `min_prefix_len` / `min_infix_len` 的术语的通配符扩展。
* [问题 #1617](https://github.com/manticoresoftware/manticoresearch/issues/1617) 修改行为：如果伙伴节点成功处理请求，不记录错误日志。
* [问题 #1635](https://github.com/manticoresoftware/manticoresearch/issues/1635) 修复带限制条件查询的搜索元数据中的总数。
* [问题 #1640](https://github.com/manticoresoftware/manticoresearch/issues/1640) 无法在普通模式下通过 JSON 使用大写表名的问题。
* [问题 #1643](https://github.com/manticoresoftware/manticoresearch/issues/1643) 提供默认的 `SPH_EXTNODE_STACK_SIZE` 值。
* [问题 #1646](https://github.com/manticoresoftware/manticoresearch/issues/1646) 修复 SphinxQL 记录带有 ALL/ANY 的 MVA 属性负过滤器的日志。
* [问题 #1660](https://github.com/manticoresoftware/manticoresearch/issues/1660) 修正了来自其他索引的 docid killlists 应用。❤️ 感谢 [@raxoft](https://github.com/raxoft) 的 PR。
* [问题 #1668](https://github.com/manticoresoftware/manticoresearch/issues/1668) 修复了在原始索引全扫描（无任何索引迭代器）时因提前退出导致的遗漏匹配；移除了普通行迭代器的截止。
* [问题 #1671](https://github.com/manticoresoftware/manticoresearch/issues/1671) 修复了查询带代理和本地表的分布式表时 `FACET` 错误。
* [问题 #1690](https://github.com/manticoresoftware/manticoresearch/issues/1690) 修复了大数值直方图估计时的崩溃。
* [问题 #1692](https://github.com/manticoresoftware/manticoresearch/issues/1692) 执行 alter table tbl add column col uint 导致崩溃。
* [问题 #1710](https://github.com/manticoresoftware/manticoresearch/issues/1710) 对条件 `WHERE json.array IN (<value>)` 返回空结果。
* [问题 #172](https://github.com/manticoresoftware/manticoresearch-buddy/issues/172) 修复了向 `/cli` 发送请求时 TableFormatter 的问题。
* [问题 #1742](https://github.com/manticoresoftware/manticoresearch/issues/1742) 在缺失 wordforms 文件时，`CREATE TABLE` 不再失败。
* [问题 #1762](https://github.com/manticoresoftware/manticoresearch/issues/1762) RT 表中属性的顺序现遵循配置顺序。
* [问题 #1765](https://github.com/manticoresoftware/manticoresearch/issues/1765) HTTP bool 查询中使用 'should' 条件返回了错误结果。
* [问题 #1769](https://github.com/manticoresoftware/manticoresearch/issues/1769) 使用 `SPH_SORT_ATTR_DESC` 和 `SPH_SORT_ATTR_ASC` 进行字符串属性排序无效。
* [问题 #177](https://github.com/manticoresoftware/manticoresearch-buddy/issues/177) 禁用了对 Buddy 的 curl 请求中的 `Expect: 100-continue` HTTP 头。
* [问题 #1791](https://github.com/manticoresoftware/manticoresearch/issues/1791) 由 GROUP BY 别名引起的崩溃。
* [问题 #1792](https://github.com/manticoresoftware/manticoresearch/issues/1792) Windows 上 SQL 元摘要显示了错误的时间。
* [问题 #1794](https://github.com/manticoresoftware/manticoresearch/issues/1794) 修复了 JSON 查询中单词性能下降的问题。
* [问题 #1798](https://github.com/manticoresoftware/manticoresearch/issues/1798) 不兼容的过滤器未在 `/search` 时抛出错误。
* [问题 #1802](https://github.com/manticoresoftware/manticoresearch/issues/1802) 修复了 `ALTER CLUSTER ADD` 和 `JOIN CLUSTER` 操作等待彼此，防止 ALTER 在捐赠节点向加入节点发送表时添加表到集群的竞态条件。
* [问题 #1811](https://github.com/manticoresoftware/manticoresearch/issues/1811) 错误处理了 `/pq/{table}/*` 请求。
* [问题 #1816](https://github.com/manticoresoftware/manticoresearch/issues/1816) `UNFREEZE` 在某些情况下不起作用。
* [问题 #183](https://github.com/manticoresoftware/manticoresearch-buddy/pull/183) 修复了某些情况下 MVA 恢复的问题。
* [问题 #1849](https://github.com/manticoresoftware/manticoresearch/issues/1849) 修复了使用 MCL 时 indextool 在关闭时崩溃的问题。
* [问题 #1866](https://github.com/manticoresoftware/manticoresearch/issues/1866) 修复了对 `/cli_json` 请求不必要的 URL 解码。
* [问题 #1872](https://github.com/manticoresoftware/manticoresearch/issues/1872) 更改守护进程启动时的 plugin_dir 设置逻辑。
* [问题 #1874](https://github.com/manticoresoftware/manticoresearch/issues/1874) alter table ... 抛出异常失败。
* [问题 #1891](https://github.com/manticoresoftware/manticoresearch/issues/1891#issuecomment-2051941200) 插入数据时 Manticore 发生 `signal 11` 崩溃。
* [问题 #1920](https://github.com/manticoresoftware/manticoresearch/issues/1920) 降低了对 [low_priority](Searching/Options.md#low_priority) 的限流。
* [问题 #1924](https://github.com/manticoresoftware/manticoresearch/issues/1924) Mysqldump + mysql 恢复错误。
* [问题 #1951](https://github.com/manticoresoftware/manticoresearch/issues/1951) 修复了在本地表缺失或代理描述错误时分布式表创建错误；现返回错误信息。
* [问题 #1972](https://github.com/manticoresoftware/manticoresearch/issues/1972) 实现了 `FREEZE` 计数器以避免 freeze/unfreeze 问题。
* [问题 #1980](https://github.com/manticoresoftware/manticoresearch/issues/1980) 在 OR 节点中遵守查询超时。之前 `max_query_time` 在某些情况下可能不生效。
* [问题 #1986](https://github.com/manticoresoftware/manticoresearch/issues/1986) 重命名 new 为 current [manticore.json] 失败。
* [问题 #1988](https://github.com/manticoresoftware/manticoresearch/issues/1988) 全文查询可能忽略 `SecondaryIndex` CBO 提示。
* [问题 #1990](https://github.com/manticoresoftware/manticoresearch/issues/1990) 修复了 `expansion_limit` 用以分片来自多个磁盘块或 RAM 块的调用关键字的最终结果集。
* [问题 #1994](https://github.com/manticoresoftware/manticoresearch/issues/1994) 错误的外部文件。
* [问题 #2021](https://github.com/manticoresoftware/manticoresearch/issues/2021) 停止 Manticore 后，部分 manticore-executor 进程可能未关闭。
* [问题 #2029](https://github.com/manticoresoftware/manticoresearch/issues/2029) 使用 Levenshtein Distance 时崩溃。
* [问题 #2037](https://github.com/manticoresoftware/manticoresearch/issues/2037) 多次在空索引上运行 max 操作后出现错误。
* [问题 #2052](https://github.com/manticoresoftware/manticoresearch/issues/2052) 多组 JSON.field 导致崩溃。
* [问题 #2067](https://github.com/manticoresoftware/manticoresearch/issues/2067) 错误请求 _update 时 Manticore 崩溃。
* [问题 #2069](https://github.com/manticoresoftware/manticoresearch/issues/2069) 修复了 JSON 接口中针对闭区间字符串过滤器比较运算符的问题。
* [问题 #2082](https://github.com/manticoresoftware/manticoresearch/issues/2082) 当 data_dir 路径位于符号链接上时，`alter` 操作失败。
* [问题 #2102](https://github.com/manticoresoftware/manticoresearch/pull/2102) 改进了 mysqldump 中 SELECT 查询的特殊处理，确保生成的 INSERT 语句与 Manticore 兼容。
* [问题 #2103](https://github.com/manticoresoftware/manticoresearch/issues/2103) 泰语字符使用了错误的字符集。
* [问题 #2124](https://github.com/manticoresoftware/manticoresearch/issues/2124) 如果使用具有保留字的 SQL 语句会导致崩溃。
* [问题 #2154](https://github.com/manticoresoftware/manticoresearch/issues/2154) 含有词形变化的表无法导入。
* [问题 #2176](https://github.com/manticoresoftware/manticoresearch/issues/2176) 修复了引擎参数设置为 'columnar' 且通过 JSON 添加重复 ID 时发生崩溃的问题。
* [问题 #223](https://github.com/manticoresoftware/manticoresearch-buddy/issues/223) 在尝试插入无模式且无列名的文档时返回正确的错误。
* [问题 #239](https://github.com/manticoresoftware/manticoresearch-buddy/issues/239) 自动模式的多行插入可能失败。
* [问题 #399](https://github.com/manticoresoftware/manticoresearch/issues/399) 如果数据源声明了 id 属性，索引时添加了错误提示信息。
* [问题 #59](https://github.com/manticoresoftware/manticoresearch-helm/issues/59) Manticore 集群出现故障。
* [问题 #68](https://github.com/manticoresoftware/manticoresearch-helm/issues/68) 如果存在 percolate 表，optimize.php 会崩溃。
* [问题 #77](https://github.com/manticoresoftware/manticoresearch-helm/issues/77) 修正了 Kubernetes 部署时的错误。
* [问题 #274](https://github.com/manticoresoftware/manticoresearch-buddy/issues/274) 修正了对 Buddy 的并发请求处理不正确。

### 与 manticore-backup 相关
* [问题 #97](https://github.com/manticoresoftware/manticoresearch-backup/pull/97) 可用时默认设置 VIP HTTP 端口。
各种改进：改进了版本检查和流式 ZSTD 解压；恢复时增加了版本不匹配的用户提示；修正恢复时不同版本的错误提示行为；增强了解压逻辑，改为直接从流读取，避免读入工作内存；新增 `--force` 标志。
* [提交 3b35](https://github.com/manticoresoftware/manticoresearch-backup/commit/3b357ea) Manticore search 启动后新增备份版本显示，以便识别此阶段的问题。
* [提交 ad2e](https://github.com/manticoresoftware/manticoresearch-backup/commit/ad2e5da) 更新了连接守护进程失败时的错误信息。
* [提交 ce5e](https://github.com/manticoresoftware/manticoresearch-backup/commit/ce5ea85) 修正了将绝对根备份路径转换为相对路径的问题，移除了恢复时的写权限检查，支持从不同路径恢复。
* [提交 db7e](https://github.com/manticoresoftware/manticoresearch-backup/commit/db7e2b9) 为文件迭代器添加排序，确保在多种情况下的一致性。
* [问题 #106](https://github.com/manticoresoftware/manticoresearch-backup/issues/105) 多配置的备份与恢复。
* [问题 #91](https://github.com/manticoresoftware/manticoresearch-backup/pull/91) 添加了 defattr，防止在 RHEL 系统安装后文件权限异常。
* [问题 #91](https://github.com/manticoresoftware/manticoresearch-backup/pull/91) 添加了额外的 chown，确保文件在 Ubuntu 中默认属于 root 用户。

### 与 MCL（列存、二级索引、knn 库）相关
* [提交 f104](https://github.com/manticoresoftware/columnar/commit/f104a4709bbb144140d09a09bc0fc1db1895e331) 支持向量搜索。
* [提交 2169](https://github.com/manticoresoftware/columnar/commit/216953beef6676f7e34c9f6f1a30c5980d746710) 修正了在中断二级索引构建设置时的临时文件清理，解决守护进程在创建 `tmp.spidx` 文件时超过打开文件数限制的问题。
* [提交 709b](https://github.com/manticoresoftware/columnar/commit/709b9acaaac97d9a1ca8796892f9ad432021c785) 为列存和二级索引使用独立的 streamvbyte 库。
* [提交 1c26](https://github.com/manticoresoftware/manticoresearch/commit/1c2623763ced14bb94ad624dde9614a938408d89) 添加警告，列存储不支持 json 属性。
* [提交 3acd](https://github.com/manticoresoftware/columnar/commit/3acd6714ffce103447d7931dfc75bb1a7449f340) 修正了二级索引中的数据解包。
* [提交 574c](https://github.com/manticoresoftware/manticoresearch/commit/574c023152564fa72092c18ebd1f594abfd4b6d7) 修复了混合使用行存和列存储时保存磁盘块导致的崩溃。
* [提交 e87f](https://github.com/manticoresoftware/columnar/commit/e87f088b458d4c5b1b6159fc72ed0d57b786891a) 修正了二级索引迭代器在已处理块上产生的提示。
* [问题 #1474](https://github.com/manticoresoftware/manticoresearch/issues/1474) 使用列存引擎时，行存 MVA 列的更新被破坏。
* [问题 #1510](https://github.com/manticoresoftware/manticoresearch/issues/1510) 修正了聚合时对 `HAVING` 中使用列存属性的崩溃。
* [问题 #1519](https://github.com/manticoresoftware/manticoresearch/issues/1519) 修正了在 `expr` 排名器上使用列存属性时的崩溃。

### 与 Docker 相关
* ❗[问题 #42](https://github.com/manticoresoftware/docker/issues/42) 通过环境变量支持[纯索引](https://github.com/manticoresoftware/docker#building-plain-tables)。
* ❗[问题 #47](https://github.com/manticoresoftware/docker/issues/47) 改进了通过环境变量配置的灵活性。
* [问题 #54](https://github.com/manticoresoftware/docker/pull/54) 改进了 Docker 的[备份与恢复](https://github.com/manticoresoftware/docker#backup-and-restore)流程。
* [问题 #77](https://github.com/manticoresoftware/docker/issues/76) 改进 entrypoint，首次启动时仅处理备份恢复。
* [提交 a27c](https://github.com/manticoresoftware/docker/commit/a27c048dd9e73d8d6bea8d9a4830cafb486b82c5) 修复了查询日志输出到 stdout 的问题。
* [问题 #38](https://github.com/manticoresoftware/docker/issues/38) 如果未设置 EXTRA，则静音 BUDDY 警告。
* [问题 #71](https://github.com/manticoresoftware/docker/pull/71) 修复了 `manticore.conf.sh` 中的主机名。

## 版本 6.2.12
发布时间：2023 年 8 月 23 日

版本 6.2.12 继续 6.2 系列，并解决了在 6.2.0 发布后发现的问题。

### Bug 修复
* ❗[问题 #1351](https://github.com/manticoresoftware/manticoresearch/issues/1351) “Manticore 6.2.0 在 Centos 7 上无法通过 systemctl 启动”：将 `TimeoutStartSec` 从 `infinity` 修改为 `0`，以提高与 Centos 7 的兼容性。
* ❗[问题 #1364](https://github.com/manticoresoftware/manticoresearch/issues/1364) “从 6.0.4 升级到 6.2.0 后崩溃”：为旧版 binlog 的空文件添加了重放功能。
* [PR #1334](https://github.com/manticoresoftware/manticoresearch/pull/1334) “修正 searchdreplication.cpp 中的拼写错误”：修正了 `searchdreplication.cpp` 中的拼写错误：beggining -> beginning。
* [问题 #1337](https://github.com/manticoresoftware/manticoresearch/issues/1337) “Manticore 6.2.0 WARNING: conn (local)(12), sock=8088: bailing on failed MySQL header, AsyncNetInputBuffer_c::AppendData: error 11 (Resource temporarily unavailable) return -1”：降低了 MySQL 接口关于头部的警告日志级别为 logdebugv。
* [问题 #1355](https://github.com/manticoresoftware/manticoresearch/issues/1355) “当 node_address 无法解析时，加入集群卡死”：改进了当某些节点无法访问且名称解析失败时的复制重试机制。此改进应解决 Kubernetes 和 Docker 节点上与复制相关的问题。增强了复制启动失败的错误信息，并对测试模型 376 进行了更新。此外，提供了明确的名称解析失败错误信息。
* [问题 #1361](https://github.com/manticoresoftware/manticoresearch/issues/1361) “charset non_cjk 中对 "Ø" 没有小写映射”：调整了字符 'Ø' 的映射。
* [问题 #1365](https://github.com/manticoresoftware/manticoresearch/issues/1365) “searchd 清理停止后会留下 binlog.meta 和 binlog.001”：确保最后的空 binlog 文件被正确删除。
* [提交 0871](https://github.com/manticoresoftware/manticoresearch/commit/0871070845e93ad702ae0dfb89b35e5f24cd0851)：修复了 Windows 下 `Thd_t` 构建有关原子复制限制的问题。
* [提交 1cc0](https://github.com/manticoresoftware/manticoresearch/commit/1cc04bedc174737118fd5a3663e35f4e95bd4f8c)：解决了 FT CBO 与 `ColumnarScan` 的问题。
* [提交 c6bf](https://github.com/manticoresoftware/manticoresearch/commit/c6bfb30d53e3f4b67f17769667733677a661c95b)：修正了测试 376，并在测试中添加了 `AF_INET` 错误的替代方案。
* [提交 cbc3](https://github.com/manticoresoftware/manticoresearch/commit/cbc38705e0b4dcc9665e3e8cf64826290ebd11cf)：解决了复制过程中更新二进制大对象属性与替换文档时的死锁问题。由于索引已在更底层被锁定，移除了提交期间对索引的 rlock。

### 小变动
* [提交 4f91](https://github.com/manticoresoftware/manticoresearch/commit/4f913a36e84e01f950b2797051222b0b1094d6fe) 更新了手册中有关 `/bulk` 端点的信息。

### MCL
* 支持 [Manticore Columnar Library](https://github.com/manticoresoftware/columnar) v2.2.4

## 版本 6.2.0
发布时间：2023 年 8 月 4 日

### 主要变更
* 查询优化器增强，支持全文查询，显著提升搜索效率和性能。
* 集成了：
  - [mysqldump](https://dev.mysql.com/doc/refman/8.0/en/mysqldump.html) - 使用 `mysqldump` 进行[逻辑备份](Securing_and_compacting_a_table/Backup_and_restore.md#Backup-and-restore-with-mysqldump)
  - [Apache Superset](https://manticoresearch.com/blog/manticoresearch-apache-superset-integration/) 和 [Grafana](https://manticoresearch.com/blog/manticoresearch-grafana-integration/) 来可视化存储在 Manticore 中的数据
  - [HeidiSQL](https://www.heidisql.com/) 和 [DBForge](https://www.devart.com/dbforge/) ，便于使用 Manticore 进行开发
* 开始使用 [GitHub 工作流](https://github.com/manticoresoftware/manticoresearch/actions)，让贡献者可以使用与核心团队相同的持续集成（CI）流程来构建包。所有作业均可在 GitHub 托管的运行器上运行，方便无缝测试您在 Manticore Search 中的分叉修改。
* 开始使用 [CLT](https://github.com/manticoresoftware/clt) 测试复杂场景。例如，我们现在可确保在提交后构建的包可在所有支持的 Linux 操作系统上正确安装。命令行测试器（CLT）提供了以交互模式记录测试并轻松重放测试的用户友好方法。
* 通过结合哈希表与 HyperLogLog，大幅提升了 count distinct 操作的性能。
* 启用了对包含二级索引的查询的多线程执行，线程数限制为物理 CPU 核心数。这将显著提升查询执行速度。
* `pseudo_sharding` 调整为限制为空闲线程数，显著提高吞吐率性能。
* 用户现可通过配置设置指定[默认属性存储引擎](Server_settings/Searchd.md#engine)，提供更好定制以匹配特定工作负载需求。
* 支持 [Manticore Columnar Library 2.2.0](https://github.com/manticoresoftware/columnar/)，在[二级索引](Server_settings/Searchd.md#secondary_indexes)中进行了大量修复和改进。

### 小变动
* [Buddy #153](https://github.com/manticoresoftware/manticoresearch-buddy/issues/153)：`/pq` HTTP端点现作为`/json/pq` HTTP端点的别名。
* [提交 0bf1](https://github.com/manticoresoftware/manticoresearch/commit/0bf17d9e)：我们确保了`upper()`和`lower()`的多字节兼容性。
* [提交 2bb9](https://github.com/manticoresoftware/manticoresearch/commit/2bb92765)：对于`count(*)`查询，不再扫描索引，而是返回预计算值。
* [提交 3c84](https://github.com/manticoresoftware/manticoresearch/commit/3c84091f)：现在可以使用`SELECT`进行任意计算并显示`@@sysvars`。与以往不同，您不再局限于仅执行一次计算。因此，诸如`select user(), database(), @@version_comment, version(), 1+1 as a limit 10`的查询将返回所有列。注意，'limit'选项始终被忽略。
* [提交 6aca](https://github.com/manticoresoftware/manticoresearch/commit/6aca32f0)：实现了`CREATE DATABASE`的存根查询。
* [提交 9dc1](https://github.com/manticoresoftware/manticoresearch/commit/9dc12334)：执行`ALTER TABLE table REBUILD SECONDARY`时，即使属性未更新，辅索引也始终会被重建。
* [提交 46ed](https://github.com/manticoresoftware/manticoresearch/commit/46edb089)：使用预计算数据的排序器在使用CBO之前会被识别，以避免不必要的CBO计算。
* [提交 102a](https://github.com/manticoresoftware/manticoresearch/commit/102ac604)：实现并利用全文表达式堆栈的模拟，以防止守护进程崩溃。
* [提交 979f](https://github.com/manticoresoftware/manticoresearch/commit/979fa27c)：为不使用字符串/mvas/json属性的匹配添加了快速的匹配克隆代码路径。
* [提交 a073](https://github.com/manticoresoftware/manticoresearch/commit/a0735ffe)：添加了对`SELECT DATABASE()`命令的支持，但它始终返回`Manticore`。此项新增对与各种MySQL工具的集成至关重要。
* [提交 bc04](https://github.com/manticoresoftware/manticoresearch/commit/bc04908)：修改了`/cli`端点的响应格式，并新增了`/cli_json`端点以充当之前的`/cli`。
* [提交 d70b](https://github.com/manticoresoftware/manticoresearch/commit/d70b0d58)：现在可以通过`SET`语句在运行时修改`thread_stack`。提供会话本地和守护进程全局两种变量版本。当前值可在`show variables`输出中访问。
* [提交 d96e](https://github.com/manticoresoftware/manticoresearch/commit/d96ec6b9)：代码已集成到CBO中，更准确地估计对字符串属性执行过滤器的复杂度。
* [提交 e77d](https://github.com/manticoresoftware/manticoresearch/commit/e77dd72f5a04531c352fad0d7afcd2a1cbae2510)：DocidIndex成本计算得到改进，提升整体性能。
* [提交 f3ae](https://github.com/manticoresoftware/manticoresearch/commit/f3ae8bea)：类似Linux下的‘uptime’，负载指标现可在`SHOW STATUS`命令中查看。
* [提交 f3cc](https://github.com/manticoresoftware/manticoresearch/commit/f3cc0971)：`DESC`和`SHOW CREATE TABLE`的字段和属性顺序现与`SELECT * FROM`保持一致。
* [提交 f3d2](https://github.com/manticoresoftware/manticoresearch/commit/f3d248a6)：不同的内部解析器现在在各种错误情况下会提供它们的内部助记代码（如`P01`）。此改进帮助识别错误由哪个解析器引起，同时也避免泄露非必要的内部细节。
* [问题 #271](https://github.com/manticoresoftware/manticoresearch/issues/271) “有时CALL SUGGEST不会提示单字母拼写错误的更正”：改进了[SUGGEST/QSUGGEST](Searching/Spell_correction.md#CALL-QSUGGEST,-CALL-SUGGEST)对短词的行为：新增选项`sentence`以显示整句内容。
* [问题 #696](https://github.com/manticoresoftware/manticoresearch/issues/696) “当启用词干处理时，Percolate索引无法正确通过精确短语查询搜索”：对percolate查询进行了修改以处理精确词项修饰符，提升搜索功能。
* [问题 #829](https://github.com/manticoresoftware/manticoresearch/issues/829) “日期格式化方法”：添加了[date_format()](Functions/Date_and_time_functions.md#DATE_FORMAT())选择列表表达式，暴露了`strftime()`函数。
* [问题 #961](https://github.com/manticoresoftware/manticoresearch/issues/961) “通过HTTP JSON API排序桶“：在HTTP接口中为每个聚合桶引入了可选的[排序属性](Searching/Faceted_search.md#HTTP-JSON)。
* [问题 #1062](https://github.com/manticoresoftware/manticoresearch/issues/1062) “改进JSON插入API失败的错误日志 - “不支持的值类型””：`/bulk`端点在出现错误时会报告已处理和未处理字符串（文档）的数量信息。
* [问题 #1070](https://github.com/manticoresoftware/manticoresearch/issues/1070) “CBO提示不支持多个属性”：已启用索引提示支持多个属性。
* [问题 #1106](https://github.com/manticoresoftware/manticoresearch/issues/1106) “为HTTP搜索查询添加标签”：已向[HTTP PQ响应](Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md?client=JSON#Adding-rules-to-a-percolate-table)添加标签。
* [问题 #1301](https://github.com/manticoresoftware/manticoresearch/issues/1301) “buddy不应并行创建表”：解决了因并行CREATE TABLE操作导致的失败问题。现在，CREATE TABLE操作一次只能执行一个。
* [Issue #1303](https://github.com/manticoresoftware/manticoresearch/issues/1303) "为列名添加对 @ 的支持"。
* [Issue #1316](https://github.com/manticoresoftware/manticoresearch/issues/1316) "在 ps=1 时出租车数据集的查询速度缓慢"：CBO 逻辑已被优化，默认的直方图分辨率已设置为 8k，以更准确地处理随机分布值的属性。
* [Issue #1317](https://github.com/manticoresoftware/manticoresearch/issues/1317) "修复 hn 数据集上的 CBO 与全文索引问题"：实现了改进的逻辑，用于确定何时使用位图迭代器交集，何时使用优先队列。
* [Issue #1318](https://github.com/manticoresoftware/manticoresearch/issues/1318) "columnar: 将迭代器接口改为单次调用"：列式迭代器现在使用单次 `Get` 调用，取代了之前的两步调用 `AdvanceTo` + `Get` 来获取值。
* [Issue #1319](https://github.com/manticoresoftware/manticoresearch/issues/1319) "聚合计算加速 (移除 CheckReplaceEntry?)"：从组排序器中移除了 `CheckReplaceEntry` 调用，以加速聚合函数的计算。
* [Issue #1320](https://github.com/manticoresoftware/manticoresearch/issues/1320) "create table 中 read_buffer_docs/hits 不识别 k/m/g 语法"：`CREATE TABLE` 选项 `read_buffer_docs` 和 `read_buffer_hits` 现在支持 k/m/g 语法。
* 英语、德语和俄语的[语言包](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology)现在可以通过在 Linux 上执行命令 `apt/yum install manticore-language-packs` 轻松安装。在 macOS 上，使用命令 `brew install manticoresoftware/tap/manticore-language-packs`。
* 字段和属性的顺序在 `SHOW CREATE TABLE` 和 `DESC` 操作之间现已保持一致。
* 如果执行 `INSERT` 查询时磁盘空间不足，新 `INSERT` 查询将失败，直到有足够的磁盘空间。
* 添加了 [UINT64()](Functions/Type_casting_functions.md#UINT64%28%29) 类型转换函数。
* `/bulk` 端点现在将空行处理为一个[提交](Data_creation_and_modification/Transactions.md#BEGIN,-COMMIT,-and-ROLLBACK)命令。更多信息见[此处](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md?client=JSON#Bulk-adding-documents)。
* 对[无效索引提示](Searching/Options.md#Query-optimizer-hints)实现了警告，提供更多透明度并允许错误缓解。
* 当 `count(*)` 与单个过滤器一起使用时，查询现在利用二级索引的预计算数据（如果可用），大幅提升查询速度。

### ⚠️ 重大变更
* ⚠️ 在版本 6.2.0 中创建或修改的表无法被旧版本读取。
* ⚠️ 文档 ID 现在在索引和 INSERT 操作中作为无符号 64 位整数处理。
* ⚠️ 查询优化器提示的语法已更新。新格式为 `/*+ SecondaryIndex(uid) */`。请注意，旧语法不再支持。
* ⚠️ [Issue #1160](https://github.com/manticoresoftware/manticoresearch/issues/1160)：为了防止语法冲突，不允许在表名中使用 `@`。
* ⚠️ 标记为 `indexed` 和 `attribute` 的字符串字段/属性现在在 `INSERT`、`DESC` 和 `ALTER` 操作中视为单一字段。
* ⚠️ [Issue #1057](https://github.com/manticoresoftware/manticoresearch/issues/1057)：不支持 SSE 4.2 的系统将不再加载 MCL 库。
* ⚠️ [Issue #1143](https://github.com/manticoresoftware/manticoresearch/issues/1143)：[agent_query_timeout](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_query_timeout) 有缺陷。已修复并生效。

### Bug修复
* [Commit 2a6e](https://github.com/manticoresoftware/manticoresearch/commit/2a6ea8f7) "DROP TABLE 崩溃"：解决了执行 DROP TABLE 语句时，RT 表完成写操作（优化、磁盘块保存）需要长时间等待的问题。增加了警告提醒，当执行 DROP TABLE 后表目录非空时通知用户。
* [Commit 2ebd](https://github.com/manticoresoftware/manticoresearch/commit/2ebd424d)：为缺失于多属性分组代码中的列式属性添加了支持。
* [Commit 3be4](https://github.com/manticoresoftware/manticoresearch/commit/3be4503f752216b37b8cde8e47fd2d77bbce63ae) 通过正确处理 binlog 中的写入错误，修复了因磁盘空间不足可能导致的崩溃问题。
* [Commit 6adb](https://github.com/manticoresoftware/manticoresearch/commit/6adb0934)：修复了查询中使用多个列式扫描迭代器（或二级索引迭代器）时偶发的崩溃问题。
* [Commit 6bd9](https://github.com/manticoresoftware/manticoresearch/commit/6bd9f709)：修复了使用基于预计算数据的排序器时过滤器未被移除的问题。
* [Commit 6d03](https://github.com/manticoresoftware/manticoresearch/commit/6d03566a)：更新了 CBO 代码，以更好地估计多线程执行的基于行属性过滤器的查询。
* [Commit 6dd3](https://github.com/manticoresoftware/manticoresearch/commit/6dd3964), [Helm #56](https://github.com/manticoresoftware/manticoresearch-helm/issues/56) "Kubernetes 集群中的致命崩溃转储"：修复了 JSON 根对象的缺陷布隆过滤器；修复了因按 JSON 字段过滤导致的守护进程崩溃。
* [Commit 6e1b](https://github.com/manticoresoftware/manticoresearch/commit/6e1b849) 修正了因无效 `manticore.json` 配置引起的守护进程崩溃。
* [Commit 6fbc](https://github.com/manticoresoftware/manticoresearch/commit/6fbc4189) 修复了 JSON 范围过滤器以支持 int64 值。
* [Commit 9c67](https://github.com/manticoresoftware/manticoresearch/commit/9c677426) 修复了 `.sph` 文件可能在执行 `ALTER` 时损坏的问题。
* [提交 77cc](https://github.com/manticoresoftware/manticoresearch/commit/77ccd8f)：已为替换语句的复制添加了共享密钥，用以解决从多个主节点复制替换时发生的 `pre_commit` 错误。
* [提交 2884](https://github.com/manticoresoftware/manticoresearch/commit/2884519e) 解决了对 'date_format()' 等函数进行 bigint 检查的问题。
* [提交 9513](https://github.com/manticoresoftware/manticoresearch/commit/9513aca5)：当排序器使用预计算数据时，迭代器不再显示在 [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META) 中。
* [提交 a2a7](https://github.com/manticoresoftware/manticoresearch/commit/a2a70555)：更新了全文节点的堆栈大小，以防止复杂全文查询时崩溃。
* [提交 a062](https://github.com/manticoresoftware/manticoresearch/commit/a0626d7e)：修复了在复制带有 JSON 和字符串属性的更新时导致崩溃的错误。
* [提交 b3e6](https://github.com/manticoresoftware/manticoresearch/commit/b3e63e5d)：更新了字符串构建器，使用 64 位整数以避免处理大数据集时崩溃。
* [提交 c472](https://github.com/manticoresoftware/manticoresearch/commit/c472e5b)：修复了多个索引间 count distinct 出现的崩溃问题。
* [提交 d073](https://github.com/manticoresoftware/manticoresearch/commit/d0730272)：修复了即使禁用 `pseudo_sharding`，对 RT 索引的磁盘块查询仍可能在多线程中执行的问题。
* [提交 d205](https://github.com/manticoresoftware/manticoresearch/commit/d205508e) 修改了 `show index status` 命令返回的值集，现在根据所使用的索引类型而异。
* [提交 e9bc](https://github.com/manticoresoftware/manticoresearch/commit/e9bc2f9) 修复了处理批量请求时的 HTTP 错误，以及错误未从网络循环返回给客户端的问题。
* [提交 f77c](https://github.com/manticoresoftware/manticoresearch/commit/f77ce0e6) 对 PQ 使用扩展堆栈。
* [提交 fac2](https://github.com/manticoresoftware/manticoresearch/commit/fac2d9687a20582765aaa89be9597804c4f845af) 更新导出排序器输出以符合 [packedfactors()](Functions/Searching_and_ranking_functions.md#PACKEDFACTORS%28%29)。
* [提交 ff87](https://github.com/manticoresoftware/manticoresearch/commit/ff87cc4)：修复了 SphinxQL 查询日志中过滤器中字符串列表的问题。
* [问题 #589](https://github.com/manticoresoftware/manticoresearch/issues/589) “字符集定义似乎依赖于代码排序”：修正了重复项的字符集映射错误。
* [问题 #811](https://github.com/manticoresoftware/manticoresearch/issues/811) “词形多重映射干扰带有 CJK 标点符号的关键词间短语搜索”：修复了带词形变化的短语查询中 ngram 令牌位置的问题。
* [问题 #834](https://github.com/manticoresoftware/manticoresearch/issues/834) “搜索查询中的等号破坏请求”：确保确切符号可以转义，并修正了 `expand_keywords` 选项导致的双重精确扩展。
* [问题 #864](https://github.com/manticoresoftware/manticoresearch/issues/864) “exceptions/stopwords 冲突”。
* [问题 #910](https://github.com/manticoresoftware/manticoresearch/issues/910) “调用 call snippets() 时使用 libstemmer_fr 和 index_exact_words 导致 Manticore 崩溃”：解决了调用 `SNIPPETS()` 导致的内部冲突和崩溃问题。
* [问题 #946](https://github.com/manticoresoftware/manticoresearch/issues/946) “SELECT 中出现重复记录”：修复了带有 `not_terms_only_allowed` 选项且包含已删除文档的 RT 索引查询结果出现重复文档的问题。
* [问题 #967](https://github.com/manticoresoftware/manticoresearch/issues/967) “UDF 函数中使用 JSON 参数导致崩溃”：修复了启用伪分片和带有 JSON 参数的 UDF 查询导致守护进程崩溃的问题。
* [问题 #1050](https://github.com/manticoresoftware/manticoresearch/issues/1050) “FEDERATED 中的 count(*)”：修复了通过带聚合的 `FEDERATED` 引擎查询导致守护进程崩溃的问题。
* [问题 #1052](https://github.com/manticoresoftware/manticoresearch/issues/1052) 修复了 `rt_attr_json` 列与列存储不兼容的问题。
* [问题 #1072](https://github.com/manticoresoftware/manticoresearch/issues/1072) “* 被 ignore_chars 从搜索查询中移除”：修复此问题，确保查询中的通配符不受 `ignore_chars` 影响。
* [问题 #1075](https://github.com/manticoresoftware/manticoresearch/issues/1075) “indextool --check 在存在分布式表时失败”：indextool 现已兼容 json 配置中包含 'distributed' 和 'template' 索引的实例。
* [问题 #1081](https://github.com/manticoresoftware/manticoresearch/issues/1081) “特定 RT 数据集上的特定 SELECT 导致 searchd 崩溃”：解决了带 packedfactors 和大型内部缓冲区查询时的守护进程崩溃。
* [问题 #1095](https://github.com/manticoresoftware/manticoresearch/issues/1095) “使用 not_terms_only_allowed 时已删除文档被忽略”。
* [问题 #1099](https://github.com/manticoresoftware/manticoresearch/issues/1099) “indextool --dumpdocids 不起作用”：恢复了 `--dumpdocids` 命令的功能。
* [问题 #1100](https://github.com/manticoresoftware/manticoresearch/issues/1100) “indextool --buildidf 不起作用”：indextool 现在在完成 globalidf 后会关闭文件。
* [问题 #1104](https://github.com/manticoresoftware/manticoresearch/issues/1104) “Count(*) 试图被远程表视为 schema 集”：修复了当代理返回空结果集时，守护进程为分布式索引查询发送错误消息的问题。
* [问题 #1109](https://github.com/manticoresoftware/manticoresearch/issues/1109) “FLUSH ATTRIBUTES 在 threads=1 时挂起”。
* [问题 #1126](https://github.com/manticoresoftware/manticoresearch/issues/1126) "查询过程中丢失与 MySQL 服务器的连接 - manticore 6.0.5": 已解决在列式属性上使用多个过滤器时发生的崩溃问题。
* [问题 #1135](https://github.com/manticoresoftware/manticoresearch/issues/1135) "JSON 字符串过滤的大小写敏感性": 修正了排序规则，使其能正确处理 HTTP 搜索请求中的过滤器。
* [问题 #1140](https://github.com/manticoresoftware/manticoresearch/issues/1140) "匹配错误字段": 修复了与 `morphology_skip_fields` 相关的问题。
* [问题 #1155](https://github.com/manticoresoftware/manticoresearch/issues/1155) "系统远程命令通过 API 应传递 g_iMaxPacketSize": 更新以绕过节点间复制命令的 `max_packet_size` 检查。此外，最新的集群错误已添加到状态显示中。
* [问题 #1302](https://github.com/manticoresoftware/manticoresearch/issues/1302) "优化失败时遗留临时文件": 修正了在合并或优化过程中出错后遗留临时文件的问题。
* [问题 #1304](https://github.com/manticoresoftware/manticoresearch/issues/1304) "为 buddy 启动超时添加环境变量": 添加了环境变量 `MANTICORE_BUDDY_TIMEOUT`（默认3秒）来控制守护进程启动时等待 buddy 消息的时长。
* [问题 #1305](https://github.com/manticoresoftware/manticoresearch/issues/1305) "保存 PQ 元数据时整数溢出": 减轻了守护进程保存大型 PQ 索引时的过度内存消耗。
* [问题 #1306](https://github.com/manticoresoftware/manticoresearch/issues/1306) "修改外部文件后无法重建 RT 表": 纠正了修改外部文件时空字符串导入错误；修复了修改外部文件后 RT 索引外部文件残留的问题。
* [问题 #1307](https://github.com/manticoresoftware/manticoresearch/issues/1307) "SELECT 语句 sum(value) as value 无法正常工作": 修复了有别名的选择列表表达式可能遮蔽索引属性的问题；还修正了整数类型中 sum 转为 int64 计数的错误。
* [问题 #1308](https://github.com/manticoresoftware/manticoresearch/issues/1308) "避免在复制时绑定到 localhost": 确保复制在主机名具有多个 IP 时不再绑定到 localhost。
* [问题 #1309](https://github.com/manticoresoftware/manticoresearch/issues/1309) "向 MySQL 客户端回复大于16Mb数据失败": 修复了返回超过16Mb的 SphinxQL 数据包给客户端的问题。
* [问题 #1310](https://github.com/manticoresoftware/manticoresearch/issues/1310) ""指向外部文件的路径应为绝对路径"中的引用错误": 修正了 `SHOW CREATE TABLE` 中外部文件完整路径的显示。
* [问题 #1311](https://github.com/manticoresoftware/manticoresearch/issues/1311) "调试构建模式下，snippet 中的长字符串导致崩溃": 现在允许 `SNIPPET()` 函数处理目标文本中超过255个字符的长字符串。
* [问题 #1312](https://github.com/manticoresoftware/manticoresearch/issues/1312) "kqueue 轮询中使用已删除对象导致的偶发崩溃（master-agent）": 修复了在 kqueue 驱动系统（FreeBSD、MacOS 等）上，master 无法连接代理时的崩溃。
* [问题 #1313](https://github.com/manticoresoftware/manticoresearch/issues/1313) "连接自身超时过长": 当 master 连接 MacOS/BSD 上的代理时，使用了统一的连接+查询超时，而非仅连接超时。
* [问题 #1314](https://github.com/manticoresoftware/manticoresearch/issues/1314) "pq（json元数据）嵌入同义词未达到导致加载失败": 修复了 pq 中嵌入同义词标记的问题。
* [问题 #1315](https://github.com/manticoresoftware/manticoresearch/issues/1315) "允许部分函数（sint、fibonacci、second、minute、hour、day、month、year、yearmonth、yearmonthday）隐式使用提升后的参数值"。
* [问题 #1321](https://github.com/manticoresoftware/manticoresearch/issues/1321) "启用全文扫描中的多线程二级索引，但限制线程数": 在 CBO 中实现了代码来更好预测全文查询中使用二级索引时的多线程性能。
* [问题 #1322](https://github.com/manticoresoftware/manticoresearch/issues/1322) "使用预计算排序器后 count(*) 查询仍然很慢": 使用预计算数据的排序器时不再启动迭代器，避免了性能下降。
* [问题 #1411](https://github.com/manticoresoftware/manticoresearch/issues/1141) "sphinxql 查询日志不能保留多值属性（MVA）的原始查询": 现在记录了 `all()/any()` 函数调用。

## 版本 6.0.4
发布时间：2023年3月15日

### 新特性
* 改进了与 Logstash、Beats 等的集成，包括：
  - 支持 Logstash 版本 7.6 - 7.15，Filebeat 版本 7.7 - 7.12
  - 支持自动模式。
  - 添加了对 Elasticsearch 样式批量请求的处理。
* [Buddy 提交 ce90](https://github.com/manticoresoftware/manticoresearch-buddy/commit/ce907ea) 在 Manticore 启动时记录 Buddy 版本。

### Bug 修复
* [问题 #588](https://github.com/manticoresoftware/manticoresearch/issues/588), [问题 #942](https://github.com/manticoresoftware/manticoresearch/issues/942) 修复了双字母索引搜索元数据和调用关键字中的错误字符。
* [问题 #1027](https://github.com/manticoresoftware/manticoresearch/issues/1027) 拒绝小写 HTTP 头。
* ❗[问题 #1039](https://github.com/manticoresoftware/manticoresearch/issues/1039) 修复了守护进程读取 Buddy 控制台输出时的内存泄漏。
* [问题 #1056](https://github.com/manticoresoftware/manticoresearch/issues/1056) 修复了问号的不预期行为。
* [问题 #1064](https://github.com/manticoresoftware/manticoresearch/issues/1064) 修复了 tokenizer 小写表中的竞争条件导致崩溃的问题。
* [提交 59bb](https://github.com/manticoresoftware/manticoresearch/commit/59bb54c) 修正了 JSON 接口处理中显式设置为 null 的文档 id 的批量写入问题。
* [提交 7b6b](https://github.com/manticoresoftware/manticoresearch/commit/7b6b25f) 修复了 CALL KEYWORDS 中多个相同词项的词项统计问题。
* [提交 f381](https://github.com/manticoresoftware/manticoresearch/commit/f381ad2) Windows 安装程序现在会创建默认配置；路径在运行时不再被替换。
* [提交 6940](https://github.com/manticoresoftware/manticoresearch/commit/6940e95), [提交 cc5a](https://github.com/manticoresoftware/manticoresearch/commit/cc5a480) 修复了多网络中集群节点的复制问题。
* [提交 4972](https://github.com/manticoresoftware/manticoresearch/commit/49722ab) 修复了 `/pq` HTTP 端点，使其成为 `/json/pq` HTTP 端点的别名。
* [提交 3b53](https://github.com/manticoresoftware/manticoresearch/commit/3b5385a) 修复了 Buddy 重启时守护进程崩溃问题。
* [Buddy 提交 fba9](https://github.com/manticoresoftware/manticoresearch-buddy/commit/fba9c8c) 收到无效请求时显示原始错误。
* [Buddy 提交 db95](https://github.com/manticoresoftware/manticoresearch-buddy/commit/db9532c) 允许备份路径中有空格，并对正则表达式做了一些魔法调整以支持单引号。

## 版本 6.0.2
发布：2023年2月10日

### Bug 修复
* [问题 #1024 crash 2](https://github.com/manticoresoftware/manticoresearch/issues/1024) Facet 搜索结果数量较多时发生崩溃 / 段错误
* ❗[问题 #1029](https://github.com/manticoresoftware/manticoresearch/issues/1029) - 警告：编译时值 KNOWN_CREATE_SIZE (16) 小于实际测量值 (208)。建议修正此值！
* ❗[问题 #1032](https://github.com/manticoresoftware/manticoresearch/issues/1032) - Manticore 6.0.0 明文索引崩溃
* ❗[问题 #1033](https://github.com/manticoresoftware/manticoresearch/issues/1033) - 多个分布式索引在守护进程重启后丢失
* ❗[问题 #1064](https://github.com/manticoresoftware/manticoresearch/issues/1064) - 标记器小写表中的竞态条件

## 版本 6.0.0
发布：2023年2月7日

从这个版本开始，Manticore Search 附带了 Manticore Buddy，这是一个用 PHP 编写的附属守护进程，处理不需要极低延迟或高吞吐量的高级功能。Manticore Buddy 在后台运行，你可能甚至不会察觉它的存在。虽然对最终用户是透明的，但为了让 Manticore Buddy 易于安装且与主 C++ 守护进程兼容，这是一个重大挑战。这一重大变化将允许团队开发各种新的高级功能，例如分片编排、访问控制和认证，以及各种集成工具，如 mysqldump、DBeaver、Grafana mysql 连接器。目前它已经支持 [SHOW QUERIES](Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES)、[BACKUP](Securing_and_compacting_a_table/Backup_and_restore.md#BACKUP-SQL-command-reference) 和 [自动模式](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema)。

该版本还修复了 130 多个 bug 和众多新功能，其中许多可以视为重大升级。

### 重大更改
* 🔬 实验性功能：你现在可以执行与 Elasticsearch 兼容的 [insert](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md?example=Elasticsearch#Adding-documents-to-a-real-time-table) 和 [replace](Data_creation_and_modification/Updating_documents/REPLACE.md?example=Elasticsearch#REPLACE) JSON 查询，从而让 Manticore 可与 Logstash（版本低于 7.13）、Filebeat 及 Beats 家族的其它工具一起使用。该功能默认启用。你可以通过 `SET GLOBAL ES_COMPAT=off` 禁用它。
* 支持 [Manticore Columnar Library 2.0.0](https://github.com/manticoresoftware/columnar/)，对[二级索引](Server_settings/Searchd.md#secondary_indexes)做了大量修复和改进。**⚠️ 重大变更**：从本版本开始二级索引默认启用。如果你从 Manticore 5 升级，请务必执行 [ALTER TABLE table_name REBUILD SECONDARY](Updating_table_schema_and_settings.md#Rebuilding-a-secondary-index)。详见下文更多信息。
* [提交 c436](https://github.com/manticoresoftware/manticoresearch/commit/c436f9023536f767610451911955ae36d90aa638) 自动模式：你现在可以跳过创建表的步骤，直接插入第一条文档，Manticore 会根据字段自动创建表。详细介绍请参见[此处](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema)。你可以通过 [searchd.auto_schema](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema) 开启或关闭此功能。
* 大幅度重构了[基于代价的优化器](Searching/Cost_based_optimizer.md)，在许多情况下降低查询响应时间。
  - [问题 #1008](https://github.com/manticoresoftware/manticoresearch/issues/1008) CBO 中的并行性能估计
  - [问题 #1014](https://github.com/manticoresoftware/manticoresearch/issues/1014) CBO 现在支持感知[二级索引](Server_settings/Searchd.md#secondary_indexes)，表现得更聪明。
  - [提交 cef9](https://github.com/manticoresoftware/columnar/commit/cef9b54dc3a4ee9138ebc4df3a671715b996d6b3) 列存表/字段的编码统计数据现存储于元数据中，帮助 CBO 做出更智能的决策。
  - [提交 2b95](https://github.com/manticoresoftware/manticoresearch/commit/2b95fb8c2ba422c9c5a66e2b61c1f7922f31c7aa) 添加了 CBO 的提示参数以便微调其行为。
* [遥测](Telemetry.md#Telemetry)：我们很高兴宣布在此版本中引入遥测功能。该功能允许我们收集匿名且去个人化的指标，帮助我们改进产品的性能和用户体验。请放心，所有收集的数据都是**完全匿名的，不会关联任何个人信息**。如果需要，该功能可以在设置中[轻松关闭](Telemetry.md#Telemetry)。
* [提交 5aaf](https://github.com/manticoresoftware/manticoresearch/commit/5aafb17d81b601e399e212a344a4738f65576cdd) [ALTER TABLE table_name REBUILD SECONDARY](Updating_table_schema_and_settings.md#Rebuilding-a-secondary-index) 以便在任何时候重建辅助索引，例如：
  - 当您从 Manticore 5 迁移到更新的版本时，
  - 当您对索引中的属性进行了 [UPDATE](Data_creation_and_modification/Updating_documents/UPDATE.md#UPDATE) 操作（即[in-place更新，而非替换](Data_creation_and_modification/Updating_documents/REPLACE_vs_UPDATE.md)）
* [问题 #821](https://github.com/manticoresoftware/manticoresearch/issues/821) 新工具 `manticore-backup` 用于[备份和恢复 Manticore 实例](Securing_and_compacting_a_table/Backup_and_restore.md)
* SQL 命令 [BACKUP](Securing_and_compacting_a_table/Backup_and_restore.md#BACKUP-SQL-command-reference) 用于从 Manticore 内部执行备份。
* SQL 命令 [SHOW QUERIES](Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES) 作为查看正在运行查询（而非线程）的简便方法。
* [问题 #551](https://github.com/manticoresoftware/manticoresearch/issues/551) SQL 命令 `KILL` 用于终止长时间运行的 `SELECT`。
* 动态 `max_matches` 用于聚合查询，以提高准确性并降低响应时间。

### 次要变更
* [问题 #822](https://github.com/manticoresoftware/manticoresearch/issues/822) SQL 命令 [FREEZE/UNFREEZE](Securing_and_compacting_a_table/Freezing_a_table.md) 用于准备实时/普通表进行备份。
* [提交 c470](https://github.com/manticoresoftware/manticoresearch/commit/c47052f18ad4868134de1a97e8d7da570e531dfa) 新设置 `accurate_aggregation` 和 `max_matches_increase_threshold` 用于控制聚合准确度。
* [问题 #718](https://github.com/manticoresoftware/manticoresearch/issues/718) 支持有符号的负 64 位 ID。请注意，您仍然不能使用 > 2^63 的 ID，但现在可以使用范围从 -2^63 到 0 的 ID。
* 由于我们最近添加了对辅助索引的支持，索引（index）一词可能指辅助索引、全文索引，或普通/实时 `index`。为了减少混淆，我们将后者重命名为“表”（table）。以下 SQL/命令行命令受到此变更影响。其旧版本已被弃用，但仍可使用：
  - `index <table name>` => `table <table name>`，
  - `searchd -i / --index` => `searchd -t / --table`，
  - `SHOW INDEX STATUS` => `SHOW TABLE STATUS`，
  - `SHOW INDEX SETTINGS` => `SHOW TABLE SETTINGS`，
  - `FLUSH RTINDEX` => `FLUSH TABLE`，
  - `OPTIMIZE INDEX` => `OPTIMIZE TABLE`，
  - `ATTACH TABLE plain TO RTINDEX rt` => `ATTACH TABLE plain TO TABLE rt`，
  - `RELOAD INDEX` => `RELOAD TABLE`，
  - `RELOAD INDEXES` => `RELOAD TABLES`。

  我们不打算废弃旧形式，但为了确保与文档兼容，建议您在应用中更改名称。未来版本将更改各种 SQL 和 JSON 命令的输出中“index”为“table”的名称。
* 带有有状态 UDF 的查询现在强制在单线程中执行。
* [问题 #1011](https://github.com/manticoresoftware/manticoresearch/issues/1011) 与时间调度相关的所有代码重构，为并行分块合并做准备。
* **⚠️ 重大变更**：列存储格式已更改。您需要重建那些具有列属性的表。
* **⚠️ 重大变更**：辅助索引文件格式已更改，因此如果您使用辅助索引进行搜索，并且在配置文件中设置了 `searchd.secondary_indexes = 1`，请注意新版本的 Manticore **将跳过加载具有辅助索引的表**。建议：
  - 升级前将配置文件中的 `searchd.secondary_indexes` 改为 0。
  - 启动实例。Manticore 会加载表并发出警告。
  - 对每个索引运行 `ALTER TABLE <table name> REBUILD SECONDARY` 以重建辅助索引。

  如果您运行的是复制集群，则需要在所有节点上运行 `ALTER TABLE <table name> REBUILD SECONDARY`，或者按照[此说明](Securing_and_compacting_a_table/Compacting_a_table.md#Optimizing-clustered-tables)操作，只是将 `OPTIMIZE` 改为运行 `ALTER .. REBUILD SECONDARY`。
* **⚠️ 重大变更**：binlog 版本已更新，之前版本的所有 binlog 文件将无法重放。升级过程中必须确保 Manticore Search 被干净地停止。停止旧实例后，目录 `/var/lib/manticore/binlog/` 下除 `binlog.meta` 外不应有其他 binlog 文件。
* [问题 #849](https://github.com/manticoresoftware/manticoresearch/issues/849) `SHOW SETTINGS`：现在可以从 Manticore 内部查看配置文件中的设置。
* [问题 #1007](https://github.com/manticoresoftware/manticoresearch/issues/1007) [SET GLOBAL CPUSTATS=1/0](Server_settings/Setting_variables_online.md#SET) 用于开启/关闭 CPU 时间追踪；[SHOW THREADS](Node_info_and_management/SHOW_THREADS.md) 在 CPU 时间追踪关闭时不显示 CPU 统计信息。
* [问题 #1009](https://github.com/manticoresoftware/manticoresearch/issues/1009) 实时表内存块碎片现在可在内存块刷写时进行合并。
* [问题 #1012](https://github.com/manticoresoftware/manticoresearch/issues/1012) 在 [indexer](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) 输出中增加了辅助索引的进度信息。
* [问题 #1013](https://github.com/manticoresoftware/manticoresearch/issues/1013) 之前如果 Manticore 启动时无法开始服务，可能会移除表记录，现新行为是保持其在列表中以便在下一次启动时尝试加载。
* [indextool --docextract](Miscellaneous_tools.md#indextool) 返回请求文档的所有词和匹配次数。
* [Commit 2b29](https://github.com/manticoresoftware/manticoresearch/commit/2b296ee20e520b85bcbb4383e87095d31e7165dc) 环境变量 `dump_corrupt_meta` 启用在 searchd 无法加载索引时，将损坏的表元数据转储到日志。
* [Commit c7a3](https://github.com/manticoresoftware/manticoresearch/commit/c7a3136eefe0eb2f69e3d99fae9c10beb5821ea6) `DEBUG META` 可以显示 `max_matches` 和伪分片统计信息。
* [Commit 6bca](https://github.com/manticoresoftware/manticoresearch/commit/6bca4848dc5c8396dbc1e057be4b9b2d579794e8) 改进错误提示，替代令人困惑的 "Index header format is not json, will try it as binary..."。
* [Commit bef3](https://github.com/manticoresoftware/lemmatizer-uk/commit/bef3ff0386d3ee87ec57619782100972c1122e47) 乌克兰语词形还原器路径已更改。
* [Commit 4ae7](https://github.com/manticoresoftware/manticoresearch/commit/4ae789595329a2951e194d1191ddb3121459a560) 在 [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META) 中新增了二级索引统计信息。
* [Commit 2e7c](https://github.com/manticoresoftware/manticoresearch/commit/2e7c585e) 现在可以通过 Swagger Editor https://manual.manticoresearch.com/Openapi#OpenAPI-specification 轻松可视化 JSON 接口。
* **⚠️ 重大变更**：复制协议已更改。如果您正在运行复制集群，升级到 Manticore 5 时，您需要：
  - 先干净地停止所有节点
  - 然后以 `--new-cluster` 参数启动最后停止的节点（在 Linux 中运行工具 `manticore_new_cluster`）。
  - 有关更多详情，请阅读 [重启集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster)。

### 与 Manticore Columnar 库相关的变更
* 重构二级索引与 Columnar 存储的集成。
* [Commit efe2](https://github.com/manticoresoftware/columnar/commit/efe26b2a) Manticore Columnar 库优化，可通过部分预先的最小/最大值评估降低响应时间。
* [Commit 2757](https://github.com/manticoresoftware/columnar/commit/2757b99b) 如果磁盘块合并中断，守护进程现在会清理与 MCL 相关的临时文件。
* [Commit e9c6](https://github.com/manticoresoftware/manticoresearch/commit/e9c672014b63e2b49e7a5a53880583af86c3ab4e) 在崩溃时，将 Columnar 和二级库版本转储到日志。
* [Commit f5e8](https://github.com/manticoresoftware/columnar/commit/f5e84eeb8b6cb102f13e3468246702427527fef9) 为二级索引添加了快速文档列表倒带支持。
* [Commit 06df](https://github.com/manticoresoftware/manticoresearch/commit/06dfdd2e668933495dd6cb52d5ba2e0aa9b26e58) 如果您使用 MCL，类似 `select attr, count(*) from plain_index`（无过滤）的查询现在更快。
* [Commit 0a76](https://github.com/manticoresoftware/manticoresearch/commit/0a76ecb9adbe886868b962dd425de5f475bbf55e) 为兼容 MySQL 8.25 以上版本的 .net 连接器，在 HandleMysqlSelectSysvar 中添加了 @@autocommit。
* **⚠️ 重大变更**：[MCL Issue #17](https://github.com/manticoresoftware/columnar/issues/17) MCL：为列扫描添加 SSE 代码。MCL 现在要求至少 SSE4.2。

### 与打包相关的变更
* [Commit 4d19](https://github.com/manticoresoftware/manticoresearch/commit/4d19f5cbe49a31228aa09253f061165bfe80e51b) **⚠️ 重大变更**：不再支持 Debian Stretch 和 Ubuntu Xenial。
* 支持 RHEL 9，包括 Centos 9、Alma Linux 9 和 Oracle Linux 9。
* [Issue #924](https://github.com/manticoresoftware/manticoresearch/issues/924) 支持 Debian Bookworm。
* [Issue #636](https://github.com/manticoresoftware/manticoresearch/issues/636) 打包：支持 Linux 和 MacOS 的 arm64 构建。
* [PR #26](https://github.com/manticoresoftware/docker/pull/26) 多架构（x86_64 / arm64）Docker 镜像。
* [贡献者简化的包构建](Installation/Compiling_from_sources.md#Building-using-CI-Docker)。
* 现在可以通过 APT 安装特定版本。
* [Commit a6b8](https://github.com/manticoresoftware/manticoresearch/commit/51fddca5c2a3ebb8576fae4c18660656ba22de0f) Windows 安装程序（之前只提供压缩包）。
* 切换到使用 CLang 15 进行编译。
* **⚠️ 重大变更**：自定义 Homebrew 配方包括 Manticore Columnar 库的配方。要安装 Manticore、MCL 及其他必要组件，请使用以下命令 `brew install manticoresoftware/manticore/manticoresearch manticoresoftware/manticore/manticore-extra`。

### Bug 修复
* [Issue #479](https://github.com/manticoresoftware/manticoresearch/issues/479) 名为 `text` 的字段问题。
* [Issue #501](https://github.com/manticoresoftware/manticoresearch/issues/501) id 不能是非 bigint 类型。
* [Issue #646](https://github.com/manticoresoftware/manticoresearch/issues/646) ALTER 与名为 "text" 的字段冲突。
* ❗[Issue #652](https://github.com/manticoresoftware/manticoresearch/issues/652) 可能的 BUG：HTTP（JSON）的偏移量和限制影响分面结果。
* ❗[Issue #827](https://github.com/manticoresoftware/manticoresearch/issues/827) searchd 在高负载下挂起/崩溃。
* ❗[Issue #996](https://github.com/manticoresoftware/manticoresearch/issues/996) PQ 索引内存溢出。
* ❗[Commit 1041](https://github.com/manticoresoftware/manticoresearch/commit/10416ef7dddf06c0d759e32ccd6ebaa2468f7cbf) `binlog_flush = 1` 从 Sphinx 时代起一直存在问题。已修复。
* [MCL Issue #14](https://github.com/manticoresoftware/columnar/issues/14) MCL：当全文字段过多时，执行 select 会崩溃。
* [Issue #470](https://github.com/manticoresoftware/manticoresearch/issues/470) sql_joined_field 无法存储。
* [Issue #713](https://github.com/manticoresoftware/manticoresearch/issues/713) 使用 LEVENSHTEIN() 时崩溃。
* [Issue #743](https://github.com/manticoresoftware/manticoresearch/issues/743) Manticore 意外崩溃且无法正常重启。
* [Issue #788](https://github.com/manticoresoftware/manticoresearch/issues/788) 通过 /sql 调用 KEYWORDS 返回控制字符，导致 JSON 解析错误。
* [Issue #789](https://github.com/manticoresoftware/manticoresearch/issues/789) mariadb 无法创建 FEDERATED 表
* [Issue #796](https://github.com/manticoresoftware/manticoresearch/issues/796) 警告：dlopen() 失败：/usr/bin/lib_manticore_columnar.so：无法打开共享对象文件：没有该文件或目录
* [Issue #797](https://github.com/manticoresoftware/manticoresearch/issues/797) 通过 api 使用 ZONESPAN 搜索时 Manticore 崩溃
* [Issue #799](https://github.com/manticoresoftware/manticoresearch/issues/799) 使用多个索引和 facet distinct 时权重错误
* [Issue #801](https://github.com/manticoresoftware/manticoresearch/issues/801) SQL 索引重新处理后，SphinxQL 分组查询挂起
* [Issue #802](https://github.com/manticoresoftware/manticoresearch/issues/802) MCL：Indexer 在 5.0.2 和 manticore-columnar-lib 1.15.4 中崩溃
* [Issue #813](https://github.com/manticoresoftware/manticoresearch/issues/813) Manticore 5.0.2 FEDERATED 返回空集（MySQL 8.0.28）
* [Issue #824](https://github.com/manticoresoftware/manticoresearch/issues/824) 在两个索引上选择 COUNT DISTINCT，结果为零时抛出内部错误
* [Issue #826](https://github.com/manticoresoftware/manticoresearch/issues/826) 删除查询时崩溃
* [Issue #843](https://github.com/manticoresoftware/manticoresearch/issues/843) MCL：长文本字段的错误
* [Issue #856](https://github.com/manticoresoftware/manticoresearch/issues/856) 5.0.2 rtindex：聚合搜索限制行为不符合预期
* [Issue #863](https://github.com/manticoresoftware/manticoresearch/issues/863) hits 返回的是 Nonetype 对象，即使搜索应返回多个结果
* [Issue #870](https://github.com/manticoresoftware/manticoresearch/issues/870) 在 SELECT 表达式中使用 Attribute 和 Stored Field 时崩溃
* [Issue #872](https://github.com/manticoresoftware/manticoresearch/issues/872) 崩溃后表变为不可见
* [Issue #877](https://github.com/manticoresoftware/manticoresearch/issues/877) 搜索查询中两个负面项导致错误：查询不可计算
* [Issue #878](https://github.com/manticoresoftware/manticoresearch/issues/878) 通过 json query_string 使用 a -b -c 不起作用
* [Issue #886](https://github.com/manticoresoftware/manticoresearch/issues/886) 带查询匹配的 pseudo_sharding
* [Issue #893](https://github.com/manticoresoftware/manticoresearch/issues/893) Manticore 5.0.2 min/max 函数表现不符合预期...
* [Issue #896](https://github.com/manticoresoftware/manticoresearch/issues/896) 字段 "weight" 解析不正确
* [Issue #897](https://github.com/manticoresoftware/manticoresearch/issues/897) Manticore 服务启动后崩溃并不断重启
* [Issue #900](https://github.com/manticoresoftware/manticoresearch/issues/900) group by j.a, smth 工作异常
* [Issue #913](https://github.com/manticoresoftware/manticoresearch/issues/913) 当 ranker 中使用 expr 且查询包含两个邻近词时 Searchd 崩溃
* [Issue #916](https://github.com/manticoresoftware/manticoresearch/issues/916) net_throttle_action 出现故障
* [Issue #919](https://github.com/manticoresoftware/manticoresearch/issues/919) MCL：Manticore 查询执行时崩溃及集群恢复时其它崩溃
* [Issue #925](https://github.com/manticoresoftware/manticoresearch/issues/925) SHOW CREATE TABLE 输出缺失反引号
* [Issue #930](https://github.com/manticoresoftware/manticoresearch/issues/930) 现在可以通过 JDBC 连接器从 Java 查询 Manticore
* [Issue #933](https://github.com/manticoresoftware/manticoresearch/issues/933) bm25f 排名问题
* [Issue #934](https://github.com/manticoresoftware/manticoresearch/issues/934) 无配置索引首次加载时在 watchdog 中冻结
* [Issue #937](https://github.com/manticoresoftware/manticoresearch/issues/937) 对 facet 数据排序时段错误
* [Issue #940](https://github.com/manticoresoftware/manticoresearch/issues/940) 对 CONCAT(TO_STRING) 崩溃
* [Issue #947](https://github.com/manticoresoftware/manticoresearch/issues/947) 在某些情况下，一个简单的单次选择查询可能导致整个实例卡死，无法登录或运行其它查询，直到该选择完成。
* [Issue #948](https://github.com/manticoresoftware/manticoresearch/issues/948) Indexer 崩溃
* [Issue #950](https://github.com/manticoresoftware/manticoresearch/issues/950) facet distinct 计数错误
* [Issue #953](https://github.com/manticoresoftware/manticoresearch/issues/953) 内置 sph04 ranker 的 LCS 计算不正确
* [Issue #955](https://github.com/manticoresoftware/manticoresearch/issues/955) 5.0.3 开发版崩溃
* [Issue #963](https://github.com/manticoresoftware/manticoresearch/issues/963) 使用 json 的 FACET 在 columnar 引擎上崩溃
* [Issue #982](https://github.com/manticoresoftware/manticoresearch/issues/982) MCL：5.0.3 在二级索引时崩溃
* [PR #984](https://github.com/manticoresoftware/manticoresearch/pull/984) HandleMysqlSelectSysvar 中的 @@autocommit
* [PR #985](https://github.com/manticoresoftware/manticoresearch/pull/985) 修正 RT 索引中的线程分片分布
* [Issue #985](https://github.com/manticoresoftware/manticoresearch/issues/985) 修正 RT 索引中的线程分片分布
* [Issue #986](https://github.com/manticoresoftware/manticoresearch/issues/986) 默认 max_query_time 设置错误
* [Issue #987](https://github.com/manticoresoftware/manticoresearch/issues/987) 多线程执行中使用正则表达式表达式时崩溃
* [Issue #988](https://github.com/manticoresoftware/manticoresearch/issues/988) 反向索引兼容性破损
* [Issue #989](https://github.com/manticoresoftware/manticoresearch/issues/989) indextool 检查 columnar 属性时报错
* [Issue #990](https://github.com/manticoresoftware/manticoresearch/issues/990) json grouper 克隆的内存泄漏
* [Issue #991](https://github.com/manticoresoftware/manticoresearch/issues/991) levenshtein 函数克隆的内存泄漏
* [Issue #992](https://github.com/manticoresoftware/manticoresearch/issues/992) 加载元数据时错误信息丢失
* [Issue #993](https://github.com/manticoresoftware/manticoresearch/issues/993) 传播来自动态索引/子键和系统变量的错误
* [Issue #994](https://github.com/manticoresoftware/manticoresearch/issues/994) 在列式存储中对列式字符串执行count distinct时崩溃
* [Issue #995](https://github.com/manticoresoftware/manticoresearch/issues/995) MCL：taxi1中min(pickup_datetime)导致崩溃
* [Issue #997](https://github.com/manticoresoftware/manticoresearch/issues/997) 空的excludes JSON查询会从选择列表中移除列
* [Issue #998](https://github.com/manticoresoftware/manticoresearch/issues/998) 当前调度器上运行的辅助任务有时会引起异常副作用
* [Issue #999](https://github.com/manticoresoftware/manticoresearch/issues/999) 使用facet distinct和不同schema时崩溃
* [Issue #1000](https://github.com/manticoresoftware/manticoresearch/issues/1000) MCL：在未加载列式库情况下运行后，列式实时索引损坏
* [Issue #1001](https://github.com/manticoresoftware/manticoresearch/issues/1001) JSON中的隐式截断无效
* [Issue #1002](https://github.com/manticoresoftware/manticoresearch/issues/1002) 列式分组器问题
* [Issue #1003](https://github.com/manticoresoftware/manticoresearch/issues/1003) 无法从索引中删除最后一个字段
* [Issue #1004](https://github.com/manticoresoftware/manticoresearch/issues/1004) --new-cluster后行为异常
* [Issue #1005](https://github.com/manticoresoftware/manticoresearch/issues/1005) “未加载列式库”，但其实不需要加载
* [Issue #1006](https://github.com/manticoresoftware/manticoresearch/issues/1006) 删除查询无错误提示
* [Issue #1010](https://github.com/manticoresoftware/manticoresearch/issues/1010) 修正了Windows版本中ICU数据文件位置
* [PR #1018](https://github.com/manticoresoftware/manticoresearch/pull/1018) 握手发送问题
* [Issue #1020](https://github.com/manticoresoftware/manticoresearch/issues/1020) 在show create table中显示id
* [Issue #1024 crash 1](https://github.com/manticoresoftware/manticoresearch/issues/1024) Facet搜索返回大量结果时崩溃/段错误
* [Issue #1026](https://github.com/manticoresoftware/manticoresearch/issues/1026) 实时索引：当大量文档插入且RAMchunk满时，searchd“卡死”
* [Commit 4739](https://github.com/manticoresoftware/manticoresearch/commit/4739dafd) 节点间复制繁忙时，关闭时线程卡住
* [Commit ab87](https://github.com/manticoresoftware/manticoresearch/commit/ab87836782e7ae43fe5f9dea739eed0d336b42c1) JSON范围过滤器中混合使用浮点数和整数可能导致Manticore忽略过滤器
* [Commit d001](https://github.com/manticoresoftware/manticoresearch/commit/d00101c2905f6393ce3fca23c4f6dcb2506f4bd9) JSON中的浮点过滤器不准确
* [Commit 4092](https://github.com/manticoresoftware/manticoresearch/commit/4092c6a8) 索引被修改时舍弃未提交事务（否则可能崩溃）
* [Commit 9692](https://github.com/manticoresoftware/manticoresearch/commit/9692a840) 使用反斜杠时出现查询语法错误
* [Commit 0c19](https://github.com/manticoresoftware/manticoresearch/commit/0c19ad8d) SHOW STATUS 中的 workers_clients 可能错误
* [Commit 1772](https://github.com/manticoresoftware/manticoresearch/commit/17720e1e2f2ec38337df565bee78f7ce9d9c744a) 修正合并无文档存储的内存段时崩溃
* [Commit f45b](https://github.com/manticoresoftware/manticoresearch/commit/f45b337b5c265433bb7d6262ad65d02f65edbaf3) 修正JSON过滤器中缺失的ALL/ANY等于条件
* [Commit 3e83](https://github.com/manticoresoftware/manticoresearch/commit/3e83c1f0c0e6e7a9d4297113f69a112cbabd9e64) 如果searchd从无法写入的目录启动，复制可能失败并提示 `got exception while reading ist stream: mkstemp(./gmb_pF6TJi) failed: 13 (Permission denied)`
* [Commit 92e5](https://github.com/manticoresoftware/manticoresearch/commit/92e5629f7150f62524b385fca670a01ba9590f5b) 从4.0.2起崩溃日志只包含偏移信息，本次提交修正了这一点。

## 版本 5.0.2
发布日期：2022年5月30日

### Bug修复
* ❗[Issue #791](https://github.com/manticoresoftware/manticoresearch/issues/791) - 错误的堆栈大小可能导致崩溃。

## 版本 5.0.0
发布日期：2022年5月18日


### 主要新功能
* 🔬 支持 [Manticore Columnar Library 1.15.2](https://github.com/manticoresoftware/columnar/)，启用[辅助索引](Server_settings/Searchd.md#secondary_indexes)的测试版。对于普通和实时列式及行存储索引（如果使用了[Manticore Columnar Library](https://github.com/manticoresoftware/columnar)），默认开启辅助索引的构建，但要开启查询辅助索引，需要在配置文件或通过[SET GLOBAL](Server_settings/Setting_variables_online.md)设置 `secondary_indexes = 1`。新功能支持所有操作系统，旧版Debian Stretch和Ubuntu Xenial除外。
* [只读模式](Security/Read_only.md)：现在可指定只处理读取查询且丢弃写入请求的监听器。
* 新的 [/cli](Connecting_to_the_server/HTTP.md#/cli) 端点，使得通过HTTP运行SQL查询更加简便。
* 通过HTTP JSON进行批量 INSERT/REPLACE/DELETE 更快：之前支持通过HTTP JSON协议提供多个写操作，但它们是逐条处理的，现在以单个事务处理。
* [#720](https://github.com/manticoresoftware/manticoresearch/issues/720) JSON协议支持[嵌套过滤器](Searching/Filters.md#Nested-bool-query)。之前在JSON中不能表达如 `a=1 and (b=2 or c=3)` 的逻辑：`must`（AND）、`should`（OR）和`must_not`（NOT）只能在顶层生效，现在支持嵌套。
* 支持 HTTP 协议中的 [Chunked transfer encoding](https://en.wikipedia.org/wiki/Chunked_transfer_encoding)（分块传输编码）。现在，您可以在应用程序中使用分块传输来传输大型批次，同时减少资源消耗（因为无需计算 `Content-Length`）。在服务器端，Manticore 现始终以流式方式处理传入的 HTTP 数据，而不是像以前那样等待整个批次传输完毕，这：
  - 降低了峰值内存使用，减少 OOM 风险
  - 缩短响应时间（我们的测试显示处理 100MB 批次时减少了 11%）
  - 允许您绕过 [max_packet_size](Server_settings/Searchd.md#max_packet_size) 限制，传输远大于最大允许值 `max_packet_size`（128MB）的批次，例如一次传输 1GB。
* [#719](https://github.com/manticoresoftware/manticoresearch/issues/719) HTTP 接口支持 `100 Continue`：现在您可以通过 `curl`（包括各种编程语言使用的 curl 库）传输大型批次，这些默认会发送 `Expect: 100-continue` 并在真正发送批次前等待一段时间。以前需要添加 `Expect: ` 头部，现在不用了。

  <details>

  以前（注意响应时间）：

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

* **⚠️ 重大更改**：默认启用了 [Pseudo sharding](Server_settings/Searchd.md#pseudo_sharding)（伪分片）。如果您想禁用它，请确保在 Manticore 配置文件的 `searchd` 部分添加 `pseudo_sharding = 0`。
* 真实时间/普通索引中现在不再强制要求至少有一个全文字段。即使不涉及全文搜索，您也可以使用 Manticore。
* [基于 Manticore Columnar Library](https://github.com/manticoresoftware/columnar) 支持的属性进行了*快速提取*（参见 Creating_a_table/Data_types.md#fast_fetch）：像 `select * from <columnar table>` 这样的查询现在比以前快得多，特别是当模式中字段较多时。
* **⚠️ 重大变更**：隐式的 [cutoff](Searching/Options.md#cutoff)。Manticore 现在不会花费时间和资源处理结果集中不需要的数据。弊端是它会影响 JSON 输出中 [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META) 的 `total_found` 和 [hits.total](Searching/Full_text_matching/Basic_usage.md#HTTP-JSON)。只有在看到 `total_relation: eq` 时才准确，而 `total_relation: gte` 意味着实际匹配文档数大于你得到的 `total_found` 值。要保持之前的行为，可以使用搜索选项 `cutoff=0`，这会使 `total_relation` 始终为 `eq`。
* **⚠️ 重大变更**：所有全文字段默认都是[存储字段](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#stored_fields)。你需要使用 `stored_fields = `（空值）来使所有字段不存储（即恢复以前的行为）。
* [#715](https://github.com/manticoresoftware/manticoresearch/issues/715) HTTP JSON 支持 [搜索选项](Searching/Options.md#General-syntax)。

### 次要更改
* **⚠️ 重大变更**：索引元文件格式变更。之前元文件（`.meta`，`.sph`）是二进制格式，现在只用 json 格式。新版本的 Manticore 会自动转换旧索引，但是：
  - 你可能会看到警告如 `WARNING: ... syntax error, unexpected TOK_IDENT`
  - 你将无法在旧版本 Manticore 中运行该索引，确保你有备份
* **⚠️ 重大变更**：通过 [HTTP keep-alive](Connecting_to_the_server/HTTP.md#Keep-alive) 支持会话状态。这样当客户端也支持时，HTTP 变成有状态的。例如，使用新的 [/cli](Connecting_to_the_server/HTTP.md#/cli) 端点和默认开启的浏览器 HTTP keep-alive，可以在执行 `SELECT` 后调用 `SHOW META`，效果与通过 mysql 连接相同。注意，之前支持 `Connection: keep-alive` HTTP 头，但仅复用连接。从此版本开始，它还使会话保持状态。
* 现在你可以指定 `columnar_attrs = *` 将所有属性定义为列式，适用于属性列表较长的 [plain 模式](Read_this_first.md#Real-time-mode-vs-plain-mode)。
* 更快的复制 SST。
* **⚠️ 重大变更**：复制协议变更。如果你运行复制集群，升级到 Manticore 5 时需要：
  - 先安全停止所有节点
  - 然后用 `--new-cluster` 启动最后停止的节点（Linux 上可运行工具 `manticore_new_cluster`）
  - 详见[重启集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster)
* 复制改进：
  - 更快的 SST
  - 抗噪声能力，改善复制节点间不稳定网络情况
  - 改进日志记录
* 安全改进：若配置中完全没有指定 `listen`，Manticore 现在默认监听 `127.0.0.1` 而非 `0.0.0.0`。虽然默认配置通常会指定 `listen`，但仍有可能不指定。之前 Manticore 会监听 `0.0.0.0`，这很不安全，现在默认改为监听通常不对外开放的 `127.0.0.1`。
* 针对列式属性的聚合速度更快。
* 提升 `AVG()` 精度：之前内部聚合用 `float`，现在改用 `double`，显著提高精度。
* 改进 JDBC MySQL 驱动支持。
* `DEBUG malloc_stats` 支持 [jemalloc](https://github.com/jemalloc/jemalloc)。
* [optimize_cutoff](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#optimize_cutoff) 现在可作为每张表的设置，在 CREATE 或 ALTER 表时指定。
* **⚠️ 重大变更**：默认的 [query_log_format](Server_settings/Searchd.md#query_log_format) 现在是 **`sphinxql`**。如果你习惯了 `plain` 格式，需要在配置文件中添加 `query_log_format = plain`。
* 显著降低内存消耗：在使用存储字段的长时间且密集的插入/替换/优化工作负载中，内存消耗明显减少。
* [shutdown_timeout](Server_settings/Searchd.md#shutdown_timeout) 默认值从 3 秒增加到了 60 秒。
* [提交 ffd0](https://github.com/manticoresoftware/manticoresearch/commit/ffd0499d329d2c383f14c8a44c4cc84338ab56e7) 兼容 Java mysql connector 6.0.3 及以上版本：该版本改变了连接 mysql 的方式，之前与 Manticore 不兼容，现在支持新行为。
* [提交 1da6](https://github.com/manticoresoftware/manticoresearch/commit/1da6dbec) 禁止在加载索引（如 searchd 启动时）保存新的磁盘块。
* [问题 #746](https://github.com/manticoresoftware/manticoresearch/issues/746) 支持 glibc >= 2.34。
* [问题 #784](https://github.com/manticoresoftware/manticoresearch/issues/784) VIP 连接与普通连接计数分开。之前 VIP 连接计入 `max_connections` 限制，可能导致普通连接出现“连接数已满”错误。现在 VIP 连接不计入限制。当前 VIP 连接数可通过 `SHOW STATUS` 和 `status` 查看。
* [ID](Creating_a_table/Data_types.md#Document-ID) 现在可以显式指定。
* [问题 #687](https://github.com/manticoresoftware/manticoresearch/issues/687) 支持 mysql 协议的 zstd 压缩。

### ⚠️ 其他次要破坏性更改
* ⚠️ BM25F 公式已稍作更新以提高搜索相关性。此更改仅影响使用函数 [BM25F()](Functions/Searching_and_ranking_functions.md#BM25F%28%29) 时的搜索结果，不会改变默认排序公式的行为。
* ⚠️ REST [/sql](Connecting_to_the_server/HTTP.md#mode=raw) 端点行为更改：`/sql?mode=raw` 现在需要转义并返回一个数组。
* ⚠️ `/bulk` INSERT/REPLACE/DELETE 请求的响应格式变更：
  - 之前每个子查询构成一个单独事务并返回单独响应
  - 现在整个批次视为一个事务，返回单一响应
* ⚠️ 搜索选项 `low_priority` 和 `boolean_simplify` 现在需要带值（`0/1`）：之前可以写作 `SELECT ... OPTION low_priority, boolean_simplify`，现在需要写作 `SELECT ... OPTION low_priority=1, boolean_simplify=1`。
* ⚠️ 如果你使用旧版 [php](https://github.com/manticoresoftware/manticoresearch/blob/master/api/sphinxapi.php)，[python](https://github.com/manticoresoftware/manticoresearch/blob/master/api/sphinxapi.py) 或 [java](https://github.com/manticoresoftware/manticoresearch/tree/master/api/java) 客户端，请访问对应链接并获取更新版本。**旧版本与 Manticore 5 不完全兼容。**
* ⚠️ HTTP JSON 请求现在在 `query_log_format=sphinxql` 模式下以不同格式记录。之前只记录全文部分，现在按原样记录。

### 新包
* **⚠️ 重大变更**：由于新结构，升级到 Manticore 5 时建议先移除旧包再安装新包：
  - 基于 RPM：`yum remove manticore*`
  - Debian 和 Ubuntu：`apt remove manticore*`
* 新的 deb/rpm 包结构。之前版本提供：
  - `manticore-server` 包含 `searchd`（主搜索守护进程）及其所有必要组件
  - `manticore-tools` 包含 `indexer` 和 `indextool`
  - `manticore` 包含所有内容
  - `manticore-all` RPM 作为元包，引用 `manticore-server` 和 `manticore-tools`

  新结构为：
  - `manticore` - deb/rpm 元包，安装上述所有作为依赖
  - `manticore-server-core` - `searchd` 及运行所需全部组件
  - `manticore-server` - systemd 文件及其他辅助脚本
  - `manticore-tools` - `indexer`、`indextool` 及其他工具
  - `manticore-common` - 默认配置文件、默认数据目录、默认停用词
  - `manticore-icudata`，`manticore-dev`，`manticore-converter` 变化不大
  - `.tgz` 包含所有包的集合
* 支持 Ubuntu Jammy
* 支持通过 [YUM 仓库](Installation/RHEL_and_Centos.md#YUM-repository) 的 Amazon Linux 2

### Bug 修复
* [Issue #815](https://github.com/manticoresoftware/manticoresearch/issues/815) 使用 UDF 函数时随机崩溃
* [Issue #287](https://github.com/manticoresoftware/manticoresearch/issues/287) RT 索引期间内存不足
* [Issue #604](https://github.com/manticoresoftware/manticoresearch/issues/604) 3.6.0，4.2.0 sphinxql-parser 重大变更
* [Issue #667](https://github.com/manticoresoftware/manticoresearch/issues/667) FATAL: 内存不足（无法分配 9007199254740992 字节）
* [Issue #676](https://github.com/manticoresoftware/manticoresearch/issues/676) 字符串未正确传递给 UDF
* ❗[Issue #698](https://github.com/manticoresoftware/manticoresearch/issues/698) 尝试向 rt 索引添加文本列后 searchd 崩溃
* [Issue #705](https://github.com/manticoresoftware/manticoresearch/issues/705) 索引器无法找到所有列
* ❗[Issue #709](https://github.com/manticoresoftware/manticoresearch/issues/705) json.boolean 分组不正确
* [Issue #716](https://github.com/manticoresoftware/manticoresearch/issues/716) indextool 指令与索引相关（如 --dumpdict）失败
* ❗[Issue #724](https://github.com/manticoresoftware/manticoresearch/issues/724) 选择中字段消失
* [Issue #727](https://github.com/manticoresoftware/manticoresearch/issues/727) .NET HttpClient 使用 `application/x-ndjson` 时 Content-Type 不兼容
* [Issue #729](https://github.com/manticoresoftware/manticoresearch/issues/729) 字段长度计算
* ❗[Issue #730](https://github.com/manticoresoftware/manticoresearch/issues/730) 创建/插入到/删除列式表时存在内存泄漏
* [Issue #731](https://github.com/manticoresoftware/manticoresearch/issues/731) 某些条件下结果中存在空列
* ❗[Issue #749](https://github.com/manticoresoftware/manticoresearch/issues/749) 守护进程启动时崩溃
* ❗[Issue #750](https://github.com/manticoresoftware/manticoresearch/issues/750) 守护进程启动时挂起
* ❗[Issue #751](https://github.com/manticoresoftware/manticoresearch/issues/751) SST 崩溃
* [Issue #752](https://github.com/manticoresoftware/manticoresearch/issues/752) 当 engine='columnar' 时 JSON 属性标记为列式
* [Issue #753](https://github.com/manticoresoftware/manticoresearch/issues/753) 复制监听 0 端口
* [Issue #754](https://github.com/manticoresoftware/manticoresearch/issues/754) columnar_attrs = * 不适用于 csvpipe
* ❗[Issue #755](https://github.com/manticoresoftware/manticoresearch/issues/755) rt 中选择列式浮点类型时崩溃
* ❗[Issue #756](https://github.com/manticoresoftware/manticoresearch/issues/756) Indextool 检查时更改 rt 索引
* [Issue #757](https://github.com/manticoresoftware/manticoresearch/issues/757) 监听端口范围交叉检测需求
* [Issue #758](https://github.com/manticoresoftware/manticoresearch/issues/758) RT 索引保存磁盘块失败时记录原始错误
* [Issue #759](https://github.com/manticoresoftware/manticoresearch/issues/759) RE2 配置仅报告一个错误
* ❗[Issue #760](https://github.com/manticoresoftware/manticoresearch/issues/760) commit 5463778558586d2508697fa82e71d657ac36510f 中的内存消耗变化
* [Issue #761](https://github.com/manticoresoftware/manticoresearch/issues/761) 第三节点在脏重启后未能形成非主集群
* [Issue #762](https://github.com/manticoresoftware/manticoresearch/issues/762) 更新计数器增加了2
* [Issue #763](https://github.com/manticoresoftware/manticoresearch/issues/763) 使用形态学时，版本4.2.1新版本破坏了用4.2.0创建的索引
* [Issue #764](https://github.com/manticoresoftware/manticoresearch/issues/764) json键/ sql?mode=raw中无转义
* ❗[Issue #765](https://github.com/manticoresoftware/manticoresearch/issues/765) 使用函数会隐藏其他值
* ❗[Issue #766](https://github.com/manticoresoftware/manticoresearch/issues/766) FixupAttrForNetwork中一行代码触发内存泄漏
* ❗[Issue #767](https://github.com/manticoresoftware/manticoresearch/issues/767) 4.2.0和4.2.1中docstore缓存相关内存泄漏
* [Issue #768](https://github.com/manticoresoftware/manticoresearch/issues/768) 存储字段通过网络传输出现奇怪的乒乓现象
* [Issue #769](https://github.com/manticoresoftware/manticoresearch/issues/769) 当未在“common”部分提及时，lemmatizer_base重置为空
* [Issue #770](https://github.com/manticoresoftware/manticoresearch/issues/770) pseudo_sharding使按id的SELECT变慢
* [Issue #771](https://github.com/manticoresoftware/manticoresearch/issues/771) 使用jemalloc时DEBUG malloc_stats输出全为零
* [Issue #772](https://github.com/manticoresoftware/manticoresearch/issues/772) 删除/添加列使值不可见
* [Issue #773](https://github.com/manticoresoftware/manticoresearch/issues/773) 无法向列式表添加bit(N)列
* [Issue #774](https://github.com/manticoresoftware/manticoresearch/issues/774) manticore.json启动时“cluster”变为空
* ❗[Commit 1da4](https://github.com/manticoresoftware/manticoresearch/commits/1da4ce891e4ff13727cfd3331d72771c64ee7949) HTTP操作未在SHOW STATUS中跟踪
* [Commit 3810](https://github.com/manticoresoftware/manticoresearch/commits/381000ab4af4c3c98ec8c730699ad7f39039cec8) 对低频单关键字查询禁用pseudo_sharding
* [Commit 8003](https://github.com/manticoresoftware/manticoresearch/commits/800325cca283a87801b28929d82420fafc76a0ee) 修正存储属性与索引合并问题
* [Commit cddf](https://github.com/manticoresoftware/manticoresearch/commits/cddfeed6296f2041c6aae18414e5cd9cabf08281) 通用distinct值获取器；新增针对列式字符串的专用distinct获取器
* [Commit fba4](https://github.com/manticoresoftware/manticoresearch/commits/fba4bb4f) 修正从docstore获取空整数属性的问题
* [Commit f300](https://github.com/manticoresoftware/manticoresearch/commit/f3009a9242fa16956adf871c4887e6e0303ba364) 查询日志中可能出现重复指定`ranker`

## 版本 4.2.0，2021年12月23日

### 主要新功能
* **支持实时索引和全文查询的伪分片**。在之前的版本中，我们新增了有限的伪分片支持。从此版本开始，你可以通过启用[searchd.pseudo_sharding](Server_settings/Searchd.md#pseudo_sharding)充分利用伪分片和你的多核处理器。最棒的是，你无需对索引或查询做任何改动，只需要启用该功能，有多余的CPU资源时即可被使用以降低响应时间。该功能支持平面索引和实时索引的全文、过滤以及分析查询。例如，启用伪分片后，你的多数查询在[Hacker news精心筛选评论数据集](https://zenodo.org/record/45901/)（放大100倍，1.16亿文档，纯平面索引）上的**平均响应时间降低约10倍**。

![4.2.0中伪分片开启与关闭对比](4.2.0_ps_on_vs_off.png)

* [**支持Debian Bullseye**](https://manticoresearch.com/install/)。

<!-- example pq_transactions_4.2.0 -->
* PQ事务现在是原子且隔离的。之前PQ事务支持有限。这让**REPLACE进入PQ更快**，特别是当需要一次替换大量规则时。性能详情：

<!-- intro -->
#### 4.0.2

<!-- request 4.0.2 -->

插入100万条PQ规则需要**48秒**，而以1万条批次替换仅4万条规则需要**406秒**。

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

插入100万条PQ规则现在只需**34秒**，而以1万条批次替换它们只需**23秒**。

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
* [optimize_cutoff](Server_settings/Searchd.md#optimize_cutoff) 现可作为`searchd`节的配置选项。当你想对所有索引全局限制RT块数时十分有用。
* [Commit 0087](https://github.com/manticoresoftware/manticoresearch/commit/00874743823eaf43e2a09a088392fd7e0a617f47) 精确支持多个本地物理索引（实时/纯平面）上具有相同字段集和顺序的[count(distinct ...)](Searching/Grouping.md#COUNT%28DISTINCT-field%29)和[FACET ... distinct](Searching/Faceted_search.md#Faceting-without-duplicates)。
* [PR #598](https://github.com/manticoresoftware/manticoresearch/pull/598) 为`YEAR()`及其他时间戳函数添加bigint支持。
* [提交 8e85](https://github.com/manticoresoftware/manticoresearch/commit/8e85d4bce596cc88f9d9615ea243a27e4afde0f7) 自适应 [rt_mem_limit](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_mem_limit)。之前 Manticore Search 在保存新的磁盘块到磁盘之前，会精确收集达到 `rt_mem_limit` 的数据，并且在保存时仍会收集多达 10% 的额外数据（也就是双缓冲）以尽量减少可能的插入挂起。如果该限制也被耗尽，新增文档会被阻止，直至磁盘块完全保存到磁盘。新的自适应限制基于现在有了 [auto-optimize](Server_settings/Searchd.md#auto_optimize) 的事实，因此磁盘块不完全遵守 `rt_mem_limit` 并提前开始刷新磁盘块也没什么大碍。所以，现在我们收集的量最多为 `rt_mem_limit` 的 50%，并将其作为一个磁盘块保存。保存时我们查看统计信息（已经保存了多少文档，保存时又有多少新文档到达），并重新计算下一次使用的初始比率。例如，如果我们保存了 9000 万文档，在保存期间又到达了 1000 万文档，则比率为 90%，所以我们知道下次可以收集到 `rt_mem_limit` 的 90% 后才开始刷新另一个磁盘块。此比率值自动计算，范围从 33.3% 到 95%。
* [问题 #628](https://github.com/manticoresoftware/manticoresearch/issues/628) PostgreSQL 源的 [unpack_zlib](Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Database_connection.md#unpack_zlib)。感谢 [Dmitry Voronin](https://github.com/dimv36) 的 [贡献](https://github.com/manticoresoftware/manticoresearch/pull/630)。
* [提交 6d54](https://github.com/manticoresoftware/manticoresearch/commit/6d54cf2b319b131970c29410cc21abcbdf8083b1) `indexer -v` 和 `--version`。之前你仍可以看到 indexer 的版本，但 `-v`/`--version` 不被支持。
* [问题 #662](https://github.com/manticoresoftware/manticoresearch/issues/662) 通过 systemd 启动 Manticore 时默认无限 mlock 限制。
* [提交 63c8](https://github.com/manticoresoftware/manticoresearch/commit/63c8cd05cf399e705c2c14939411a92cb784735a) 自旋锁 -> 适用于协程读写锁的操作队列。
* [提交 4113](https://github.com/manticoresoftware/manticoresearch/commit/41130ce30d1734bbf8e6b8b63c4e815caf9464f2) 环境变量 `MANTICORE_TRACK_RT_ERRORS` ，用于调试 RT 段损坏。

### 重大变更
* Binlog 版本已提升，旧版本的 binlog 无法重放，因此在升级期间请确保干净停止 Manticore Search：停止前不应有除 `binlog.meta` 以外的 binlog 文件存在于 `/var/lib/manticore/binlog/`。
* [提交 3f65](https://github.com/manticoresoftware/manticoresearch/commit/3f659f36e87d99ee262205a8eae4663a255be630) `show threads option format=all` 增加新列 "chain"。它显示一些任务信息票据的堆栈，主要用于分析，因此如果你在解析 `show threads` 的输出，请注意新增列。
* 自 3.5.0 起 `searchd.workers` 被废弃，现在已弃用，如果配置文件中依然有此项，启动时会触发警告。Manticore Search 会启动，但会提示警告。
* 如果你使用 PHP 和 PDO 访问 Manticore，需要设置 `PDO::ATTR_EMULATE_PREPARES`

### 修复
* ❗[问题 #650](https://github.com/manticoresoftware/manticoresearch/issues/650) Manticore 4.0.2 比 Manticore 3.6.3 慢。4.0.2 在批量插入方面比之前版本快，但单文档插入显著变慢。该问题已在 4.2.0 修复。
* ❗[提交 22f4](https://github.com/manticoresoftware/manticoresearch/commit/22f4141bed1411c83684ae6e0bae832071cce3df) RT 索引在高强度 REPLACE 负载下可能损坏，或导致崩溃。
* [提交 03be](https://github.com/manticoresoftware/manticoresearch/commit/03be91e4f720a3f20bee0278dd8f0d44abd3ef0e) 修正合并聚合器和组 N 排序器时的平均值计算；修正聚合合并。
* [提交 2ea5](https://github.com/manticoresoftware/manticoresearch/commit/2ea575d34efe38661f14d858f2b552456f29fe42) `indextool --check` 可能崩溃。
* [提交 7ec7](https://github.com/manticoresoftware/manticoresearch/commit/7ec76d4abf01b5fca6f10a0000fe64d5d8e5fa52) 由 UPDATE 导致的内存耗尽问题。
* [提交 658a](https://github.com/manticoresoftware/manticoresearch/commit/658a727e41f3c3fbcde9754149528f34ed76ba19) 守护进程可能在 INSERT 时挂起。
* [提交 46e4](https://github.com/manticoresoftware/manticoresearch/commit/46e42b9b386e31e53247ed634e158b533e28063e) 守护进程可能在关闭时挂起。
* [提交 f8d7](https://github.com/manticoresoftware/manticoresearch/commit/f8d7d5177cc83d250b493b4cfc619732b29cf072) 守护进程可能在关闭时崩溃。
* [提交 733a](https://github.com/manticoresoftware/manticoresearch/commit/733accf165142a1d1574689525ccca8907971158) 守护进程可能在崩溃时挂起。
* [提交 f7f8](https://github.com/manticoresoftware/manticoresearch/commit/f7f8bd8c0dbb23a59a88502dcba16b0918fb2330) 守护进程重启时，如因无效节点列表尝试重新加入集群可能崩溃。
* [提交 1401](https://github.com/manticoresoftware/manticoresearch/commit/140155613821ecaddcd1a3dfc4f7369b0df6aef7) 分布式索引在 RT 模式下启动时如果无法解析任一代理节点，可能被完全遗忘。
* [问题 #683](https://github.com/manticoresoftware/manticoresearch/issues/683) attr bit(N) engine='columnar' 失败。
* [问题 #682](https://github.com/manticoresoftware/manticoresearch/issues/682) 创建表失败，但留下目录。
* [问题 #663](https://github.com/manticoresoftware/manticoresearch/issues/663) 配置失败，提示：未知键名 'attr_update_reserve'。
* [问题 #632](https://github.com/manticoresoftware/manticoresearch/issues/632) Manticore 在批量查询时崩溃。
* [问题 #679](https://github.com/manticoresoftware/manticoresearch/issues/679) 批量查询在 v4.0.3 中再次导致崩溃。
* [提交 f7f8](https://github.com/manticoresoftware/manticoresearch/commit/f7f8bd8c0dbb23a59a88502dcba16b0918fb2330) 修复了守护进程启动时尝试用无效节点列表重新加入集群导致崩溃的问题
* [问题 #643](https://github.com/manticoresoftware/manticoresearch/issues/643) Manticore 4.0.2 在批量插入后不接受连接
* [问题 #635](https://github.com/manticoresoftware/manticoresearch/issues/635) 带有 ORDER BY JSON.field 或字符串属性的 FACET 查询可能会崩溃
* [问题 #634](https://github.com/manticoresoftware/manticoresearch/issues/634) 包含 packedfactors 的查询导致 SIGSEGV 崩溃
* [提交 4165](https://github.com/manticoresoftware/manticoresearch/commits/41657f15f8fcf1e93cfad7f835bcf705d6e8349e) create table 不支持 morphology_skip_fields

## 版本 4.0.2, 2021年9月21日

### 主要新功能
- **全面支持 [Manticore Columnar Library](https://github.com/manticoresoftware/columnar)**。此前 Manticore 列式库仅支持普通索引。现支持：
  - 实时索引的 `INSERT`、`REPLACE`、`DELETE`、`OPTIMIZE`
  - 复制
  - `ALTER`
  - `indextool --check`
- **自动索引压缩** ([问题 #478](https://github.com/manticoresoftware/manticoresearch/issues/478))。终于无需手动调用 OPTIMIZE 或通过定时任务等自动化方式。Manticore 现在会自动默认地完成此操作。可通过 [optimize_cutoff](Server_settings/Setting_variables_online.md) 全局变量设置默认的压缩阈值。
- **数据块快照和锁系统重构**。这些变化初看可能无明显外显，但显著提升了实时索引中多项功能的表现。简而言之，之前大多数 Manticore 的数据操作强依赖锁，现在改为使用磁盘块快照。
- **大幅提升实时索引中的批量 INSERT 性能**。例如，在 [Hetzner AX101 服务器](https://www.hetzner.com/dedicated-rootserver/ax101)（SSD，128 GB 内存，AMD Ryzen™ 9 5950X（16*2 核））上，使用 3.6.0 时，向表结构为 `name text, email string, description text, age int, active bit(1)` 的表批量插入文档时（默认 `rt_mem_limit`，批量大小 25000，16 并发写入线程，累计插入 1600 万文档）可达到每秒插入 236K 文档。在 4.0.2 中使用相同并发/批量/计数配置可达到每秒 357K 文档。

  <details>

  - 读取操作（如 SELECT、复制）使用快照执行
  - 只改变内部索引结构但不修改 schema/文档的操作（如合并 RAM 段、保存磁盘块、合并磁盘块）使用只读快照执行，最终替换现有块
  - UPDATE 和 DELETE 操作作用于现有块，但若发生合并时，写入先收集，随后应用到新块上
  - UPDATE 会对每个块顺序获取独占锁。合并操作在收集块属性阶段获取共享锁，因此同时只有一个（合并或更新）操作可以访问该块属性
  - 合并进入需要属性阶段时设置特殊标记。UPDATE 完成后检测此标记，若设置，则整个更新存入特殊集合。合并结束时将这些更新应用到新的磁盘块。
  - ALTER 通过独占锁执行
  - 复制作为普通读取操作运行，另外在 SST 前保存属性并禁止更新期间操作

  </details>

- **[ALTER](Updating_table_schema_and_settings.md) 支持添加/移除全文字段**（RT 模式）。此前仅能添加/移除属性。
- 🔬 **实验性：全扫描查询的伪分片** - 可并行化任何非全文搜索查询。无需手动准备分片，只需启用新选项 [searchd.pseudo_sharding](Server_settings/Searchd.md#pseudo_sharding)，即可对非全文搜索查询获得最高 `CPU cores` 倍的响应时间降低。注意，该功能可能会占满所有 CPU 核心，如关注吞吐量而非仅延迟，请谨慎使用。

### 小幅改动
<!-- example -->
- 通过 [APT 仓库](Installation/Debian_and_Ubuntu.md#APT-repository) 支持 Linux Mint 和 Ubuntu Hirsute Hippo
- 在某些情况下，HTTP 通过 id 更新大索引更快（依赖 id 分布）
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
- [systemd 自定义启动参数](Starting_the_server/Linux.md#Custom-startup-flags-using-systemd)。现在如需使用特定启动参数运行 Manticore，无需手动启动 searchd
- 新增函数 [LEVENSHTEIN()](Functions/String_functions.md#LEVENSHTEIN%28%29)，计算 Levenshtein 距离
- 新增了 `--replay-flags=ignore-trx-errors` 和 `--replay-flags=ignore-all-errors` 两个 [searchd 启动参数](Starting_the_server/Manually.md#searchd-command-line-options)，以便在二进制日志损坏时仍能启动 searchd
- [问题 #621](https://github.com/manticoresoftware/manticoresearch/issues/621) - 公开 RE2 错误信息
- 对由本地普通索引组成的分布式索引，提供更精确的 [COUNT(DISTINCT)](Searching/Grouping.md#COUNT%28DISTINCT-field%29)
- [FACET DISTINCT](Searching/Faceted_search.md#Faceting-without-duplicates) 用于在分面搜索时去重
- [精确形式修饰符](Searching/Full_text_matching/Operators.md#Exact-form-modifier) 现在无需依赖 [morphology](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology)，且对启用 [infix/prefix](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) 搜索的索引有效

### 重大变更
- 新版本可以读取旧的索引，但旧版本无法读取 Manticore 4 的索引
- 移除了按 id 的隐式排序。需要排序时请显式指定
- `charset_table` 的默认值从 `0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451` 改为 `non_cjk`
- `OPTIMIZE` 会自动执行。如果不需要，请确保在配置文件中搜索守护进程 (`searchd`) 部分设置 `auto_optimize=0`
- [Issue #616](https://github.com/manticoresoftware/manticoresearch/issues/616) 弃用的 `ondisk_attrs_default` 现已移除
- 贡献者注意：我们现在在 Linux 构建中使用 Clang 编译器，测试表明它能构建出更快的 Manticore Search 和 Manticore Columnar Library
- 如果搜索查询中未指定 [max_matches](Searching/Options.md#max_matches)，为了新列存储的性能，它会隐式更新为所需的最低值。这可能影响 [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META) 中的 `total` metric，但不会影响 `total_found`，它是实际找到的文档数。

### 从 Manticore 3 迁移
- 确保你干净地停止 Manticore 3：
  - `/var/lib/manticore/binlog/` 中不应有 binlog 文件（目录中只能有 `binlog.meta`）
  - 否则 Manticore 4 无法对这些索引响应 binlog，索引将不会运行
- 新版本可以读取旧索引，但旧版本无法读取 Manticore 4 索引，因此如果要轻松回滚新版本，请确保备份
- 如果运行复制集群，请确保你：
  - 先干净地停止所有节点
  - 然后用 `--new-cluster` 启动最后停止的节点（Linux 下运行工具 `manticore_new_cluster`）
  - 详情请阅读 [重启集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster)

### Bug 修复
- 许多复制问题得到修复：
  - [Commit 696f](https://github.com/manticoresoftware/manticoresearch/commit/696f8649535f2b5285b878c9581c0d2b1139ae09) - 修复带有活动索引的加入节点 SST 期间崩溃；在加入节点写文件块时加入 sha1 校验以加快索引加载；加入节点索引加载时添加更改索引文件的轮换；当活动索引被捐赠节点的新索引替换时，加入节点删除索引文件；捐赠节点发送文件和块时添加复制日志点
  - [Commit b296](https://github.com/manticoresoftware/manticoresearch/commit/b296c55af68314a8fc66f8b104e7301d673c3b68) - 地址不正确时 JOIN CLUSTER 崩溃
  - [Commit 418b](https://github.com/manticoresoftware/manticoresearch/commit/418bf880a5e05ffbb68f311a1c23006a9df3220e) - 大索引最初复制时，加入节点可能出现 `ERROR 1064 (42000): invalid GTID, (null)` 错误，捐赠节点在其他节点加入时可能无响应
  - [Commit 6fd3](https://github.com/manticoresoftware/manticoresearch/commit/6fd350d28e4905211cf8a7298cbf7cdda101c83d) - 大索引可能计算错哈希导致复制失败
  - [Issue #615](https://github.com/manticoresoftware/manticoresearch/issues/615) - 集群重启时复制失败
- [Issue #574](https://github.com/manticoresoftware/manticoresearch/issues/574) - `indextool --help` 没有显示参数 `--rotate`
- [Issue #578](https://github.com/manticoresoftware/manticoresearch/issues/578) - searchd 空闲时 CPU 使用率高，持续约一天
- [Issue #587](https://github.com/manticoresoftware/manticoresearch/issues/587) - 立即刷写 .meta 文件
- [Issue #617](https://github.com/manticoresoftware/manticoresearch/issues/617) - manticore.json 内容被清空
- [Issue #618](https://github.com/manticoresoftware/manticoresearch/issues/618) - root 用户下 searchd --stopwait 失败，修复了 systemctl 行为（之前 ExecStop 显示失败且未能充分等待 searchd 正确停止）
- [Issue #619](https://github.com/manticoresoftware/manticoresearch/issues/619) - INSERT/REPLACE/DELETE 与 SHOW STATUS，`command_insert`、`command_replace` 等指标错误
- [Issue #620](https://github.com/manticoresoftware/manticoresearch/issues/620) - 纯索引的 `charset_table` 默认值错误
- [Commit 8f75](https://github.com/manticoresoftware/manticoresearch/commit/8f7536887d4b58a5bab5647fc067b2d0482c2038) - 新磁盘块未被 mlock
- [Issue #607](https://github.com/manticoresoftware/manticoresearch/issues/607) - Manticore 集群节点无法通过名称解析节点时崩溃
- [Issue #623](https://github.com/manticoresoftware/manticoresearch/issues/623) - 更新的索引复制可能导致未定义状态
- [Commit ca03](https://github.com/manticoresoftware/manticoresearch/commit/ca03d2280c5197e9b311f0f03f551e3b702a8130) - 对带有 json 属性的纯索引源索引时索引器可能挂起
- [Commit 53c7](https://github.com/manticoresoftware/manticoresearch/commit/53c753054558ff3f82c7af365273d32537c5338b) - 修复 PQ 索引的非等值表达式过滤
- [Commit ccf9](https://github.com/manticoresoftware/manticoresearch/commit/ccf94e022ba6763d6286ab5bac767b6f6a846846) - 修复列表查询中超过 1000 条结果的选择窗口。之前 `SELECT * FROM pq ORDER BY id desc LIMIT 1000 , 100 OPTION max_matches=1100` 无法正常工作
- [Commit a048](https://github.com/manticoresoftware/manticoresearch/commit/a0483fe9bef3c45dffe96f5df4f4382aee29c072) - 访问 Manticore 的 HTTPS 请求可能导致“max packet size(8388608) exceeded”警告
- [Issue #648](https://github.com/manticoresoftware/manticoresearch/issues/648) - Manticore 3 在字符串属性更新数次后可能挂起


## 版本 3.6.0，2021年5月3日
**Manticore 4 发布前的维护版本**

### 主要新特性
- 对于普通索引，支持[Manticore Columnar Library](https://github.com/manticoresoftware/columnar/)。为普通索引新增设置[columnar_attrs](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#columnar_attrs)
- 支持[乌克兰词形还原器](https://github.com/manticoresoftware/lemmatizer-uk)
- 完全修订的直方图。当构建索引时，Manticore 还会为每个字段构建直方图，然后用于更快的过滤。在3.6.0中，算法经过全面修订，如果数据量大并且需要大量过滤，您可以获得更高的性能。

### 次要更改
- 工具 `manticore_new_cluster [--force]` 对于通过 systemd 重新启动复制集群非常有用
- [--drop-src](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Merging_tables.md#Merging-tables) 用于 `indexer --merge`
- [新模式](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) `blend_mode='trim_all'`
- 添加[支持转义 JSON 路径](Searching/Full_text_matching/Escaping.md#Escaping-json-node-names-in-SQL)使用反引号
- [indextool --check](Miscellaneous_tools.md#indextool) 可以在RT模式下工作
- [FORCE/IGNORE INDEX(id)](Data_creation_and_modification/Updating_documents/UPDATE.md#Updates-via-SQL) 用于 SELECT/UPDATE
- 合并磁盘块的块 ID 现在是唯一的
- [indextool --check-disk-chunk CHUNK_NAME](Miscellaneous_tools.md#indextool)

### 优化
- [更快的 JSON 解析](https://github.com/manticoresoftware/manticoresearch/commit/0c25fc1788e3f77f60cb7fb8f1623d8466bba606)，我们的测试显示在像 `WHERE json.a = 1` 这样的查询上延迟降低了 3-4%
- 非文档化命令 `DEBUG SPLIT` 作为自动分片/重平衡的前提条件

### 修复
- [问题 #584](https://github.com/manticoresoftware/manticoresearch/issues/524) - 不准确且不稳定的 FACET 结果
- [问题 #506](https://github.com/manticoresoftware/manticoresearch/issues/506) - 使用 MATCH 时的奇怪行为：受到此问题影响的用户需要重建索引，因为问题出现在构建索引的阶段
- [问题 #387](https://github.com/manticoresoftware/manticoresearch/issues/387) - 运行带有 SNIPPET() 函数的查询时间歇性核心转储
- 有助于处理复杂查询的堆栈优化：
  - [问题 #469](https://github.com/manticoresoftware/manticoresearch/issues/469) - SELECT 结果导致崩溃转储
  - [e8420cc7](https://github.com/manticoresoftware/manticoresearch/tree/e8420cc789149c84731be0874be916e33b263bd6) - 过滤树的堆栈大小检测
- [问题 #461](https://github.com/manticoresoftware/manticoresearch/issues/461) - 使用 IN 条件的更新未正确生效
- [问题 #464](https://github.com/manticoresoftware/manticoresearch/issues/464) - CALL PQ 后立即 SHOW STATUS 返回 - [问题 #481](https://github.com/manticoresoftware/manticoresearch/issues/481) - 修复静态二进制构建
- [问题 #502](https://github.com/manticoresoftware/manticoresearch/issues/502) - 多查询中的 bug
- [问题 #514](https://github.com/manticoresoftware/manticoresearch/issues/514) - 使用 'create table' 时无法为列使用不寻常的名称
- [提交 d1db](https://github.com/manticoresoftware/manticoresearch/commit/d1dbe771288328c963ca8512f564c6b7cf4f466c) - 重放 binlog 时字符串属性更新导致守护进程崩溃；将 binlog 版本设置为 10
- [提交 775d](https://github.com/manticoresoftware/manticoresearch/commit/775d0555562f121911d3c30085947e743fadac2d)  - 修复表达式堆栈帧检测运行时 (测试 207)
- [提交 4795](https://github.com/manticoresoftware/manticoresearch/commit/4795dc49194c7745761bb5006ecb266e8a9224fa)  - 对于空存储查询，悬挂索引过滤器和标签为空 (测试 369)
- [提交 c3f0](https://github.com/manticoresoftware/manticoresearch/commit/c3f0bf4dca0e4eae6ea899dd566878df57eba085)  - 网络中长延迟和高错误率导致的复制 SST 流中断（不同数据中心复制）；已更新复制命令版本至 1.03
- [提交 ba2d](https://github.com/manticoresoftware/manticoresearch/commit/ba2d6619b8107f3fb177741bbdff3709bac248cd)  - 写操作后加入集群的连接器锁定集群（测试 385）
- [提交 de4d](https://github.com/manticoresoftware/manticoresearch/commit/de4dcb9ff6c316bc2a9ef6c62eb64769f04ee103)  - 使用精确修饰符的通配符匹配 (测试 321)
- [提交 6524](https://github.com/manticoresoftware/manticoresearch/commit/6524fc6af032a0932bd42c37874a619e1fb7f075)  - docid 检查点与 docstore
- [提交 f4ab](https://github.com/manticoresoftware/manticoresearch/commit/f4ab83c291e65a5873d271c67b5491f629544f1d)  - 在解析无效的 xml 时索引器行为不一致
- [提交 7b72](https://github.com/manticoresoftware/manticoresearch/commit/7b727e22fe01b1a180d4a8d3a3fea92c18eac707)  - 存储的过濾查询使用 NOTNEAR 永远运行（测试 349）
- [提交 812d](https://github.com/manticoresoftware/manticoresearch/commit/812dab745f0c972ecde5d5b3d4a313cd6e1e143e)  - 通配符开头的短语权重错误
- [提交 1771](https://github.com/manticoresoftware/manticoresearch/commit/1771afc669c75670b3511bf02ab9bfc8982b94d2)  - 带有通配符的过濾查询生成的匹配项由于缺乏有效负载而导致交叉命中并破坏匹配（测试 417）
- [提交 aa0d](https://github.com/manticoresoftware/manticoresearch/commit/aa0d8c2bcb41428d41dbfb66e42a872a79a4ee4b)  - 在并行化查询的情况下修复“总计”的计算
- [提交 18d8](https://github.com/manticoresoftware/manticoresearch/commit/18d81b3cfbb930b7361dae645fe13283d4fbf6ef)  - Windows 中的多个并发会话下守护进程崩溃
- [提交 8443](https://github.com/manticoresoftware/manticoresearch/commit/84432f23d6fbf7a7d0b67c8c9412eb5933b03596)  - 某些索引设置无法被复制
- [Commit 9341](https://github.com/manticoresoftware/manticoresearch/commit/93411fe6af4246ca4acce758944d513a1660b92b)  - 在高频率添加新事件时，netloop 有时会冻结，因为原子“kick”事件一次处理多个事件，导致实际动作丢失
查询状态，而非服务器状态
- [Commit d805](https://github.com/manticoresoftware/manticoresearch/commit/d805fc125244ef08d14fb3d1826e2c356862d539)  - 新刷新的磁盘块在提交时可能丢失
- [Commit 63cb](https://github.com/manticoresoftware/manticoresearch/commit/63cbf008d50979cdf7062664fb02a672a3bb1131)  - profiler 中 'net_read' 不准确
- [Commit f537](https://github.com/manticoresoftware/manticoresearch/commit/f5379bb27234b25919966c50ccb6f469ead646e2)  - Arabic（从右向左文本）相关的 Percolate 问题
- [Commit 49ee](https://github.com/manticoresoftware/manticoresearch/commit/49eeb4202d4ce4875c07c35087d7108c1e177328)  - 重复列名时 id 获取不正确
- [Commit refa](https://github.com/manticoresoftware/manticoresearch/commit/4b7df262bbfec9b9e95a44e72ad326ac8a786c49) 修复网络事件在极少数情况下导致崩溃的问题
- [e8420cc7](https://github.com/manticoresoftware/manticoresearch/tree/e8420cc789149c84731be0874be916e33b263bd6) 修复 `indextool --dumpheader`
- [Commit ff71](https://github.com/manticoresoftware/manticoresearch/commit/ff7163532ed39042c0d3c3c2b902ff41f94d7f0c)  - 存储字段的 TRUNCATE WITH RECONFIGURE 功能异常

### 破坏性更改：
- 新的 binlog 格式：升级前需要彻底停止 Manticore
- 索引格式略有变化：新版本能正常读取现有索引，但如果从 3.6.0 降级到旧版本，新索引将无法读取
- 复制格式更改：不要从旧版本复制到 3.6.0，或反向操作，应同时将所有节点切换到新版本
- `reverse_scan` 已被废弃。从 3.6.0 起，请确保查询中不使用此选项，否则查询会失败
- 自本版本起，不再提供 RHEL6、Debian Jessie 和 Ubuntu Trusty 的构建版本。如需继续支持，请[联系我们](https://manticoresearch.com/contact-us/)

### 弃用
- 不再有隐式按 id 排序。如依赖此功能，请相应更新查询
- 搜索选项 `reverse_scan` 已被废弃

## 版本 3.5.4，2020年12月10日

### 新功能
- 新的 Python、Javascript 和 Java 客户端现已正式发布，相关手册中有详细文档。
- 实时索引自动丢弃磁盘块。这一优化允许在[OPTIMIZE实时索引](Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE)时自动丢弃明显不再需要的磁盘块（所有文档已被删除）。之前依然需合并操作，现在可以即时丢弃磁盘块。[cutoff](Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks) 选项被忽略，即使没实际合并，废弃的磁盘块也会被移除。此功能适用于维护索引保留期并删除旧文档的场景，有助于加快索引压缩速度。
- [独立的 NOT](Searching/Options.md#not_terms_only_allowed) 作为 SELECT 的选项

### 小幅改动
- [Issue #453](https://github.com/manticoresoftware/manticoresearch/issues/453) 新增选项 [indexer.ignore_non_plain=1](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#ignore_non_plain)，适用于运行 `indexer --all` 但配置文件中包含非 plain 索引的情况。未设置 `ignore_non_plain=1` 会触发警告并返回相应退出码。
- [SHOW PLAN ... OPTION format=dot](Node_info_and_management/Profiling/Query_plan.md#Dot-format-for-SHOW-PLAN) 和 [EXPLAIN QUERY ... OPTION format=dot](Searching/Full_text_matching/Profiling.md#Profiling-without-running-a-query) 可视化全文查询计划执行，便于理解复杂查询。

### 弃用
- `indexer --verbose` 已废弃，因未对 indexer 输出有任何增加
- watchdog 的堆栈回溯信号由 `USR1` 改为 `USR2`

### Bug修复
- [Issue #423](https://github.com/manticoresoftware/manticoresearch/issues/423) 斯拉夫字符句号调用 snippet 模式时未高亮
- [Issue #435](https://github.com/manticoresoftware/manticoresearch/issues/435) RTINDEX - GROUP N BY 表达式选择导致致命崩溃
- [Commit 2b3b](https://github.com/manticoresoftware/manticoresearch/commit/2b3b62bd0ce2b54544733d524a169e81f71b5388) 集群模式下 searchd 状态显示段错误
- [Commit 9dd2](https://github.com/manticoresoftware/manticoresearch/commit/9dd25c193ed2a53750a0e0e68566fd2b833fcb0c) 'SHOW INDEX index.N SETTINGS' 未正确处理大于9的 chunk
- [Issue #389](https://github.com/manticoresoftware/manticoresearch/issues/389) 导致 Manticore 崩溃的 bug
- [Commit fba1](https://github.com/manticoresoftware/manticoresearch/commit/fba166178ff35c73a0c70d131cf3aca4a635e961) 转换器生成损坏索引
- [Commit eecd](https://github.com/manticoresoftware/manticoresearch/commit/eecd61d861c742471175be19a5e5455034b5db5f) stopword_step=0 与 CALL SNIPPETS() 问题
- [Commit ea68](https://github.com/manticoresoftware/manticoresearch/commit/ea6850e449601199190f38a387c04b8a7b7e90f6) 本地索引上低 max_matches 时 count distinct 返回 0
- [Commit 362f](https://github.com/manticoresoftware/manticoresearch/commit/362f27db3421f09a41c6cce04b5baace00fa4c4d) 使用聚合时，命中结果中未返回存储文本


## 版本 3.5.2，2020年10月1日

### 新功能

* OPTIMIZE 将磁盘块数减少至指定数量（默认值为 `2*核心数`），而非合并到单块。最优块数可通过 [cutoff](Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks) 参数控制。
* NOT 运算符现在可以单独使用。默认情况下它被禁用，因为意外的单个 NOT 查询可能会很慢。可以通过设置新的 searchd 指令 [not_terms_only_allowed](Server_settings/Searchd.md#not_terms_only_allowed) 为 `0` 来启用。
* 新设置 [max_threads_per_query](Server_settings/Searchd.md#max_threads_per_query) 设定一个查询可以使用多少线程。如果未设置该指令，查询可以使用的线程数将达到 [threads](Server_settings/Searchd.md#threads) 的值。
每个 `SELECT` 查询可以通过 [OPTION threads=N](Searching/Options.md#threads) 限制线程数量，覆盖全局的 `max_threads_per_query` 设置。
* Percolate 索引现在可以通过 [IMPORT TABLE](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Importing_table.md) 导入。
* HTTP API `/search` 通过新的查询节点 `aggs` 对 [faceting](Searching/Faceted_search.md#HTTP-JSON)/[grouping](Searching/Grouping.md) 提供基础支持。

### 小改动

* 如果没有声明 replication listen 指令，引擎将尝试使用定义的 'sphinx' 端口之后的端口，最多到 200。
* `listen=...:sphinx` 需要显式设置以支持 SphinxSE 连接或 SphinxAPI 客户端。
* [SHOW INDEX STATUS](Node_info_and_management/Table_settings_and_status/SHOW_TABLE_STATUS.md) 输出了新的指标：`killed_documents`，`killed_rate`，`disk_mapped_doclists`，`disk_mapped_cached_doclists`，`disk_mapped_hitlists` 和 `disk_mapped_cached_hitlists`。
* SQL 命令 `status` 现在输出 `Queue\Threads` 和 `Tasks\Threads`。

### 弃用：

* `dist_threads` 现已完全弃用，如果仍使用该指令，searchd 将记录警告。

### Docker

官方 Docker 镜像现在基于 Ubuntu 20.04 LTS。

### 打包

除了常规的 `manticore` 包外，您还可以通过组件安装 Manticore Search：

- `manticore-server-core` - 提供 `searchd`，手册页，日志目录，API 和 galera 模块。它还将作为依赖安装 `manticore-common`。
- `manticore-server` - 提供核心的自动化脚本（init.d，systemd），以及 `manticore_new_cluster` 包装器。它还将作为依赖安装 `manticore-server-core`。
- `manticore-common` - 提供配置文件，停用词，通用文档和骨架文件夹（数据目录，模块等）。
- `manticore-tools` - 提供辅助工具（`indexer`，`indextool` 等），其手册页和示例。它还将作为依赖安装 `manticore-common`。
- `manticore-icudata`（RPM）或 `manticore-icudata-65l`（DEB） - 提供 ICU 数据文件用于 ICU 形态学功能。
- `manticore-devel`（RPM）或 `manticore-dev`（DEB） - 提供 UDFs 的开发头文件。

### Bug 修复

1. [Commit 2a47](https://github.com/manticoresoftware/manticoresearch/commit/2a474dc1a26e8b0f8aaaae95669caf2f1d4b7746) RT 索引中不同块 grouper 导致守护进程崩溃
2. [Commit 57a1](https://github.com/manticoresoftware/manticoresearch/commit/57a19e5ad5663ef0ca7436595218fb1221d28c8e) 空远程文档的快速路径
3. [Commit 07dd](https://github.com/manticoresoftware/manticoresearch/commit/07dd3f313c63fb82c22092f9907ef24e3475250e) 表达式栈帧检测运行时
4. [Commit 08ae](https://github.com/manticoresoftware/manticoresearch/commit/08ae357cf1012bc8e2da54c20b205b592efda3d4) Percolate 索引中匹配超过 32 个字段
5. [Commit 16b9](https://github.com/manticoresoftware/manticoresearch/commit/16b9390fd4cdb07a77ac4497adb935573a1710e5) 复制监听端口范围
6. [Commit 5fa6](https://github.com/manticoresoftware/manticoresearch/commit/5fa671affeacb6441a59c8a88479bfd423df7c81) pq 上显示创建表
7. [Commit 54d1](https://github.com/manticoresoftware/manticoresearch/commit/54d133b6449105a9fb0168db3f1fbb05fb5aa1f6) HTTPS 端口行为
8. [Commit fdbb](https://github.com/manticoresoftware/manticoresearch/commit/fdbbe5245cc296cc5c1ae3ae2fb9cb08fb66a248) 替换时混合 docstore 行
9. [Commit afb5](https://github.com/manticoresoftware/manticoresearch/commit/afb53f648ef4e64fa1776c58e66f6e716ac730ad) 将 TFO 不可用消息级别切换为 'info'
10. [Commit 59d9](https://github.com/manticoresoftware/manticoresearch/commit/59d94cefc6e88af360b8046726a16b7eaa9f0b71) strcmp 错误使用导致崩溃
11. [Commit 04af](https://github.com/manticoresoftware/manticoresearch/commit/04af0349c1e784c2230d204592d6272b440bb375) 将索引添加到带有系统（停用词）文件的集群
12. [Commit 5014](https://github.com/manticoresoftware/manticoresearch/commit/50148b4eae9f3a924082005cf5d2f9c8339af7f5) 合并具有大型字典的索引；RT 优化大型磁盘块
13. [Commit a2ad](https://github.com/manticoresoftware/manticoresearch/commit/a2adf1582843f63d4dcc1dab8f7598e0917094a2) Indextool 可以从当前版本转储元数据
14. [Commit 69f6](https://github.com/manticoresoftware/manticoresearch/commit/69f6d5f77c64b97e3f395aea8c555b11c77fb732) GROUP N 中组排序问题
15. [Commit 24d5](https://github.com/manticoresoftware/manticoresearch/commit/24d5d80f508576794b751d70f9f9b524564e3c14) 握手后对 SphinxSE 明确刷新
16. [Commit 31c4](https://github.com/manticoresoftware/manticoresearch/commit/31c4d78ac03ee85d9a02b86075448a04ac80f9e2) 避免不必要时复制巨大的描述
17. [Commit 2959](https://github.com/manticoresoftware/manticoresearch/commit/2959e2caa8610f98fcdf2e5a82160f58687f4a27) show threads 中负时间问题
18. [Commit f0b3](https://github.com/manticoresoftware/manticoresearch/commit/f0b357102c70f3ac1d7da260c60b67322940e647) 令牌过滤插件与零位置增量
19. [Commit a49e](https://github.com/manticoresoftware/manticoresearch/commit/a49e5bc13ea763e6f01dd5993e891858a84d1846) 多重命中时将 'FAIL' 改为 'WARNING'

## 版本 3.5.0，2020 年 7 月 22 日

### 主要新特性：
* 这个版本发布花费了很长时间，因为我们一直在努力将多任务模式从线程改为**协程**。这使得配置更简单，查询并行化更加直接：Manticore 只使用给定数量的线程（见新的设置 [threads](Server_settings/Searchd.md#threads)），而新的模式确保以最优的方式完成。
* [高亮](Searching/Highlighting.md#Highlighting-options)中的更改：
  - 任何作用于多个字段的高亮（`highlight({},'field1, field2'`）或 JSON 查询中的 `highlight`）现在默认对每个字段应用限制。
  - 任何作用于纯文本的高亮（`highlight({}, string_attr)` 或 `snippet()`）现在对整个文档应用限制。
  - [每字段限制](Searching/Highlighting.md#limits_per_field)可以通过选项 `limits_per_field=0`（默认值为 `1`）切换为全局限制。
  - 通过 HTTP JSON 实现的高亮，`[allow_empty](Searching/Highlighting.md#allow_empty)` 现在默认值为 `0`。

* 现在同一端口[可以用于](Server_settings/Searchd.md#listen) http、https 和二进制 API（用于接受来自远程 Manticore 实例的连接）。通过 mysql 协议连接仍需 `listen = *:mysql`。Manticore 现在可以自动检测尝试连接的客户端类型，MySQL 除外（因协议限制）。

* 在 RT 模式下，字段现在可以同时是[文本和字符串属性](Creating_a_table/Data_types.md#String) - [GitHub 议题 #331](https://github.com/manticoresoftware/manticoresearch/issues/331)。

  在[普通模式](Read_this_first.md#Real-time-mode-vs-plain-mode)中称为 `sql_field_string`。现在它也在[RT 模式](Read_this_first.md#Real-time-mode-vs-plain-mode)的实时索引中可用。用法示例如下：

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
* SQL 接口支持 SSL 和压缩。
* 支持 mysql 客户端的 [`status`](Node_info_and_management/Node_status.md#STATUS) 命令。
* [复制](Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Setting-up-replication)现在可以复制外部文件（停用词、例外等）。
* 过滤操作符 [`in`](Searching/Filters.md#Set-filters) 现在通过 HTTP JSON 接口可用。
* HTTP JSON 支持 [`expressions`](Searching/Expressions.md#expressions)。
* 现在可以在 RT 模式下[动态修改 `rt_mem_limit`](https://github.com/manticoresoftware/manticoresearch/issues/344)，即可以执行 `ALTER ... rt_mem_limit=<新值>`。
* 现在可以使用[单独的 CJK 字符集表](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology)：`chinese`、`japanese` 和 `korean`。
* [thread_stack](Server_settings/Searchd.md#thread_stack) 现在限制最大线程栈大小，而非初始大小。
* 改进了 `SHOW THREADS` 的输出。
* 在 `SHOW THREADS` 中显示长时间运行的 `CALL PQ` 进度。
* cpustat、iostat、coredump 可通过运行时的 [SET](Server_settings/Setting_variables_online.md#SET) 修改。
* 实现了 `SET [GLOBAL] wait_timeout=NUM`。

### 重大更改：
* **索引格式已更改。** 3.5.0 版本构建的索引无法被低于 3.5.0 版本的 Manticore 加载，但 Manticore 3.5.0 可识别旧格式索引。
* [`INSERT INTO PQ VALUES()`](Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md)（即不提供列名列表）之前只接受 `(query, tags)` 作为值。现在改成 `(id, query, tags, filters)`。如果想自动生成 id，可以将其设为 0。
* [allow_empty=0](Searching/Highlighting.md#allow_empty) 是 HTTP JSON 接口高亮中的新默认值。
* `CREATE TABLE`/`ALTER TABLE` 中只允许使用绝对路径指定外部文件（停用词、例外等）。

### 弃用内容：
* `ram_chunks_count` 在 `SHOW INDEX STATUS` 中被重命名为 `ram_chunk_segments_count`。
* `workers` 已过时。现在只有一个 workers 模式。
* `dist_threads` 已过时。所有查询现在都最大程度并行（由 `threads` 和 `jobs_queue_size` 限制）。
* `max_children` 已过时。请使用 [threads](Server_settings/Searchd.md#threads) 来设置 Manticore 使用的线程数（默认为 CPU 核心数）。
* `queue_max_length` 已过时。如有需要，请使用 [jobs_queue_size](Server_settings/Searchd.md#jobs_queue_size) 微调内部任务队列大小（默认无限制）。
* 所有 `/json/*` 端点现在可直接通过 `/search`、`/insert`、`/delete`、`/pq` 等访问，无需 `/json/` 前缀。
* `field`（意为“全文字段”）在 `describe` 中被重命名为 `text`。
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
* 在 `non_cjk` 字符集表（默认）中，西里尔字母 `и` 不映射为 `i`，因为这会严重影响俄语词干提取和词形还原器。
* `read_timeout`。请改用控制读写的 [network_timeout](Server_settings/Searchd.md#network_timeout)。


### 安装包

* Ubuntu Focal 20.04 官方软件包
* deb 软件包名称由 `manticore-bin` 更改为 `manticore`

### 修复的缺陷：
1. [Issue #351](https://github.com/manticoresoftware/manticoresearch/issues/351) searchd 内存泄漏
2. [Commit ceab](https://github.com/manticoresoftware/manticoresearch/commit/ceabe44f) snippet 中微小的越界读取
3. [Commit 1c3e](https://github.com/manticoresoftware/manticoresearch/commit/1c3e84a3) 对本地变量的危险写入导致查询崩溃
4. [Commit 26e0](https://github.com/manticoresoftware/manticoresearch/commit/26e094ab) 测试 226 中 sorter 的微小内存泄漏
5. [Commit d2c7](https://github.com/manticoresoftware/manticoresearch/commit/d2c7f86a) 测试 226 中严重的内存泄漏
6. [Commit 0dd8](https://github.com/manticoresoftware/manticoresearch/commit/0dd80122) 集群显示节点同步，但 `count(*)` 显示数量不同
7. [Commit f1c1](https://github.com/manticoresoftware/manticoresearch/commit/f1c1ac3f) 外观：日志中重复且有时丢失警告信息
8. [Commit f1c1](https://github.com/manticoresoftware/manticoresearch/commit/f1c1ac3f) 外观：日志中的 (null) 索引名称
9. [Commit 359d](https://github.com/manticoresoftware/manticoresearch/commit/359dbd30) 无法检索超过 7000 万条结果
10. [Commit 19f3](https://github.com/manticoresoftware/manticoresearch/commit/19f328ee) 无法使用无列语法插入 PQ 规则
11. [Commit bf68](https://github.com/manticoresoftware/manticoresearch/commit/bf685d5d) 插入集群中索引文档时误导性错误信息
12. [Commit 2cf1](https://github.com/manticoresoftware/manticoresearch/commit/2cf18c83) `/json/replace` 和 `json/update` 返回指数形式的 id
13. [Issue #324](https://github.com/manticoresoftware/manticoresearch/issues/324) 在同一查询中更新 json 标量属性和 mva
14. [Commit d384](https://github.com/manticoresoftware/manticoresearch/commit/d38409eb) RT 模式下 `hitless_words` 不生效
15. [Commit 5813](https://github.com/manticoresoftware/manticoresearch/commit/5813d639) RT 模式下应禁止使用 `ALTER RECONFIGURE`
16. [Commit 5813](https://github.com/manticoresoftware/manticoresearch/commit/5813d639) searchd 重启后 `rt_mem_limit` 重置为 128M
17. highlight() 有时挂起
18. [Commit 7cd8](https://github.com/manticoresoftware/manticoresearch/commit/7cd878f4) RT 模式下无法使用 U+ 代码
19. [Commit 2b21](https://github.com/manticoresoftware/manticoresearch/commit/2b213de4) RT 模式下无法使用通配符词形变化
20. [Commit e9d0](https://github.com/manticoresoftware/manticoresearch/commit/e9d07e68) 修正了针对多个词形变化文件的 `SHOW CREATE TABLE`
21. [Commit fc90](https://github.com/manticoresoftware/manticoresearch/commit/fc90a84f) JSON 查询缺少 "query" 时 searchd 崩溃
22. Manticore [官方 docker](https://hub.docker.com/r/manticoresearch/manticore) 无法从 mysql 8 索引
23. [Commit 23e0](https://github.com/manticoresoftware/manticoresearch/commit/23e05d32) HTTP /json/insert 需要 id
24. [Commit bd67](https://github.com/manticoresoftware/manticoresearch/commit/bd679af0) `SHOW CREATE TABLE` 不支持 PQ
25. [Commit bd67](https://github.com/manticoresoftware/manticoresearch/commit/bd679af0) PQ 的 `CREATE TABLE LIKE` 行为异常
26. [Commit 5eac](https://github.com/manticoresoftware/manticoresearch/commit/5eacf28f) show index status 的设置中行尾问题
27. [Commit cb15](https://github.com/manticoresoftware/manticoresearch/commit/cb153228) HTTP JSON 响应中的 "highlight" 标题为空
28. [Issue #318](https://github.com/manticoresoftware/manticoresearch/issues/318) `CREATE TABLE LIKE` 中缀错误
29. [Commit 9040](https://github.com/manticoresoftware/manticoresearch/commit/9040d22c) RT 模式下在负载下崩溃
30. [cd512c7d ](https://github.com/manticoresoftware/manticoresearch/commit/cd512c7d ) RT 磁盘块崩溃时丢失崩溃日志
31. [Issue #323](https://github.com/manticoresoftware/manticoresearch/issues/323) 导入表失败且断开连接
32. [Commit 6275](https://github.com/manticoresoftware/manticoresearch/commit/6275316a) `ALTER reconfigure` 损坏 PQ 索引
33. [Commit 9c1d](https://github.com/manticoresoftware/manticoresearch/commit/9c1d221e) 更改索引类型后 searchd 重载问题
34. [Commit 71e2](https://github.com/manticoresoftware/manticoresearch/commit/71e2b5bb) 缺失文件导入表时守护进程崩溃
35. [Issue #322](https://github.com/manticoresoftware/manticoresearch/issues/322) 使用多个索引、group by 和 ranker = none 的 select 崩溃
36. [Commit c3f5](https://github.com/manticoresoftware/manticoresearch/commit/c3f58490) `HIGHLIGHT()` 不会突出显示字符串属性
37. [Issue #320](https://github.com/manticoresoftware/manticoresearch/issues/320) `FACET` 不能对字符串属性排序
38. [Commit 4f1a](https://github.com/manticoresoftware/manticoresearch/commit/4f1a1f25) 数据目录缺失时报错
39. [Commit 04f4](https://github.com/manticoresoftware/manticoresearch/commit/04f4ddd4) RT 模式不支持 access_*
40. [Commit 1c06](https://github.com/manticoresoftware/manticoresearch/commit/1c0616a2) 字符串中坏的 JSON 对象：1. `CALL PQ` 当 JSON 超过某个值时返回 "Bad JSON objects in strings: 1"。
41. [Commit 32f9](https://github.com/manticoresoftware/manticoresearch/commit/32f943d6) RT 模式不一致。某些情况下无法删除索引因未知，且无法创建因目录不为空。
42. [Issue #319](https://github.com/manticoresoftware/manticoresearch/issues/319) select 崩溃
43. [Commit 22a2](https://github.com/manticoresoftware/manticoresearch/commit/22a28dd7) `max_xmlpipe2_field` = 2M 对 2M 字段发出警告
44. [Issue #342](https://github.com/manticoresoftware/manticoresearch/issues/342) 查询条件执行错误
45. [提交 dd8d](https://github.com/manticoresoftware/manticoresearch/commit/dd8dcab2) 简单的 2 词搜索找到仅包含一个词的文档
46. [提交 9091](https://github.com/manticoresoftware/manticoresearch/commit/90919e62) 在 PQ 中无法匹配键包含大写字母的 json
47. [提交 56da](https://github.com/manticoresoftware/manticoresearch/commit/56da086a) Indexer 在 csv+docstore 上崩溃
48. [问题 #363](https://github.com/manticoresoftware/manticoresearch/issues/363) 在 centos 7 中 json 属性使用 `[null]` 导致插入数据损坏
49. 重大 [问题 #345](https://github.com/manticoresoftware/manticoresearch/issues/345) 记录未被插入，count() 随机，"replace into" 返回 OK
50. max_query_time 导致 SELECT 过度变慢
51. [问题 #352](https://github.com/manticoresoftware/manticoresearch/issues/352) Mac OS 上主从通信失败
52. [问题 #328](https://github.com/manticoresoftware/manticoresearch/issues/328) 使用 Connector.Net/Mysql 8.0.19 连接 Manticore 时错误
53. [提交 daa7](https://github.com/manticoresoftware/manticoresearch/commit/daa760d2fe83668038318f0f384ceefc0d075e18) 修复 \0 转义并优化性能
54. [提交 9bc5](https://github.com/manticoresoftware/manticoresearch/commit/9bc5c01a493db993b90ff6c1ad51aeae028f629c) 修复 count distinct 与 json 的问题
55. [提交 4f89](https://github.com/manticoresoftware/manticoresearch/commit/4f89a965a3af0db548ae9d1763731ebaebe21f5a) 修复其他节点 drop table 失败
56. [提交 952a](https://github.com/manticoresoftware/manticoresearch/commit/952af5a5931fd72b87edc169588777778b365231) 修复紧密运行调用 pq 时崩溃


## 版本 3.4.2，2020 年 4 月 10 日
### 关键修复
* [提交 2ffe](https://github.com/manticoresoftware/manticoresearch/commit/2ffe2d261793f9681265bc4abe38d695e72c121e) 修复旧版本 RT 索引无法索引数据的问题

## 版本 3.4.0，2020 年 3 月 26 日
### 重大变更
* 服务器工作在两种模式：rt 模式和普通模式
   *   rt 模式需要 data_dir，配置中不允许定义索引
   *   普通模式中索引在配置中定义；不允许使用 data_dir
* 复制仅在 rt 模式下可用

### 小幅变更
* charset_table 默认使用 non_cjk 别名
* rt 模式中全文字段默认被索引并存储
* rt 模式中全文字段重命名，'field' 改为 'text'
* ALTER RTINDEX 重命名为 ALTER TABLE
* TRUNCATE RTINDEX 重命名为 TRUNCATE TABLE

### 新功能
* 仅存储字段
* SHOW CREATE TABLE，IMPORT TABLE

### 改进
* 更快的无锁 PQ
* /sql 能在 mode=raw 下执行任意类型的 SQL 语句
* mysql 协议别名为 mysql41
* data_dir 中默认提供 state.sql

### Bug修复
* [提交 a533](https://github.com/manticoresoftware/manticoresearch/commit/a5333644e8e0c69746a9f24da8221fc965478e4a) 修复 highlight() 字段语法错误导致崩溃
* [提交 7fbb](https://github.com/manticoresoftware/manticoresearch/commit/7fbb9f2e618096289cba7f8b834530b46aa94055) 修复复制带 docstore 的 RT 索引时服务器崩溃
* [提交 24a0](https://github.com/manticoresoftware/manticoresearch/commit/24a04687eb43b4f4719c7c4a5fe3c19099240467) 修复对带有 infix 或 prefix 选项且未启用存储字段索引的 highlight 导致崩溃
* [提交 3465](https://github.com/manticoresoftware/manticoresearch/commit/3465c1ce9c860c09f6ccc5b83a63e3c68c241d8d) 修复关于空 docstore 和空索引时 dock-id 查找的错误提示
* [提交 a707](https://github.com/manticoresoftware/manticoresearch/commit/b8601b4144b9092be43a7caa7cd6b19cb37e14c7) 修复 #314 SQL 插入命令带尾随分号问题
* [提交 9562](https://github.com/manticoresoftware/manticoresearch/commit/95628c9b9b8f16fe1bd18599a84e9673575adf1a) 移除查询词不匹配的警告
* [提交 b860](https://github.com/manticoresoftware/manticoresearch/commit/b8601b4144b9092be43a7caa7cd6b19cb37e14c7) 修复基于 ICU 分词的片段查询
* [提交 5275](https://github.com/manticoresoftware/manticoresearch/commit/5275516ca4631c584e6e0127fa161a18d2658fd0) 修复 docstore 块缓存中查找/添加的竞争条件
* [提交 f06e](https://github.com/manticoresoftware/manticoresearch/commit/f06ef97af5c7390427e82525c150535290e42c3e) 修复 docstore 中的内存泄漏
* [提交 a725](https://github.com/manticoresoftware/manticoresearch/commit/a7258ba8340e246c0d8aaeb4a7a74129eba807eb) 修复 #316 INSERT 时 LAST_INSERT_ID 返回空的问题
* [提交 1ebd](https://github.com/manticoresoftware/manticoresearch/commit/1ebd5bf8d74113bcfa138ce63ce11966990a4215) 修复 #317 json/update HTTP 接口支持 MVA 数组和 JSON 属性对象
* [提交 e426](https://github.com/manticoresoftware/manticoresearch/commit/e426950a536382a142e2a42ed207c341149ff685) 修复索引器未显式 id 而转储 rt 导致的混乱问题

## 版本 3.3.0，2020 年 2 月 4 日
### 新功能
* 并行实时索引搜索
* EXPLAIN QUERY 命令
* 无索引定义的配置文件（alpha 版本）
* CREATE/DROP TABLE 命令（alpha 版本）
* indexer --print-rt - 可以从源读取并打印 Real-Time 索引的 INSERT 语句

### 改进
* 升级到 Snowball 2.0 词干提取器
* SHOW INDEX STATUS 支持 LIKE 过滤器
* 改进高 max_matches 下的内存使用
* SHOW INDEX STATUS 添加 rt 索引的 ram_chunks_count
* 无锁 PQ
* 将 LimitNOFILE 改为 65536


### Bug修复
* [提交 9c33](https://github.com/manticoresoftware/manticoresearch/commit/9c33aab8c1fd9059c8644d23d5f58f03010043c6) 增加索引架构重复属性检查 #293
* [提交 a008](https://github.com/manticoresoftware/manticoresearch/commit/a0085f9477e191da9e0e835b91583fb9cfde698e) 修复 hitless 词语时崩溃
* [提交 6895](https://github.com/manticoresoftware/manticoresearch/commit/68953740af8dca1833026d0f448c71416ed3044a) 修复 ATTACH 后 docstore 松散问题
* [提交 d6f6](https://github.com/manticoresoftware/manticoresearch/commit/d6f696edea53ea2b9a68251e4054e4816082c285) 修复分布式环境中的 docstore 问题
* [Commit bce2](https://github.com/manticoresoftware/manticoresearch/commit/bce2b7ec883d208f31094ee42a5b6f01d18a998f) 在排序器中用 OpenHash 替换 FixedHash
* [Commit e0ba](https://github.com/manticoresoftware/manticoresearch/commit/e0baf7392937969e640fb5d10e8ccb312fe36446) 修复索引定义中属性名重复问题
* [Commit ca81](https://github.com/manticoresoftware/manticoresearch/commit/ca81114bf1ac7ea7adcc8509bdc14d1f30b4f461) 修复 HIGHLIGHT() 中的 html_strip
* [Commit 493a](https://github.com/manticoresoftware/manticoresearch/commit/493a5e916d70fd4a3f14d1ee487234434f4a7ce3) 修复 HIGHLIGHT() 中的 passage 宏
* [Commit a82d](https://github.com/manticoresoftware/manticoresearch/commit/a82d41c775692f837f267d554d7d4a4b499d69f0) 修复实时索引创建小或大磁盘块时双缓冲问题
* [Commit a404](https://github.com/manticoresoftware/manticoresearch/commit/a404c85de0d41d9d1c75b7540479d8a3af9e61aa) 修复 kqueue 事件删除
* [Commit 8bea](https://github.com/manticoresoftware/manticoresearch/commit/8bea0f6fd1666b817995c4dd6e67f6f1c9161638) 修复 RT 索引 rt_mem_limit 取大值时磁盘块保存问题
* [Commit 8707](https://github.com/manticoresoftware/manticoresearch/commit/8707f0395b7ace52dc0cba9c8ed92a2bc4392932) 修复索引时浮点溢出
* [Commit a564](https://github.com/manticoresoftware/manticoresearch/commit/a56434ce71bb7714b4d8e75278aaa075b27ece80) 修复插入带负 ID 文档到实时索引时出错
* [Commit bbeb](https://github.com/manticoresoftware/manticoresearch/commit/bbebfd7551a68641154761cac962053b54806759) 修复基于 ranker fieldmask 的服务器崩溃
* [Commit 3809](https://github.com/manticoresoftware/manticoresearch/commit/3809cc1b0863e3a5a200dc423be3f7c38be56113) 修复使用查询缓存时崩溃
* [Commit dc2a](https://github.com/manticoresoftware/manticoresearch/commit/dc2a585bc724678679e4297362200bbd49ce32bb) 修复使用并行插入的 RT 索引内存段崩溃

## 版本 3.2.2，2019 年 12 月 19 日
### 新功能
* 实时索引自增 ID
* 通过新增 HIGHLIGHT() 函数支持文档存储高亮，HTTP API 也可用
* SNIPPET() 可使用特殊函数 QUERY()，返回当前 MATCH 查询
* 为高亮函数新增 field_separator 选项

### 改进和变更
* 远程节点存储字段懒惰获取（可显著提升性能）
* 字符串和表达式不再破坏多查询和 FACET 优化
* RHEL/CentOS 8 构建现在使用 mariadb-connector-c-devel 的 mysql libclient
* ICU 数据文件随包一起发布，移除 icu_data_dir 配置
* systemd 服务文件包含 'Restart=on-failure' 策略
* indextool 现在可以在线检查实时索引
* 默认配置现在是 /etc/manticoresearch/manticore.conf
* RHEL/CentOS 上的服务名称由 'searchd' 改为 'manticore'
* 移除 query_mode 和 exact_phrase snippet 选项

### 修复
* [Commit 6ae4](https://github.com/manticoresoftware/manticoresearch/commit/6ae474c7894a6bee222d5b18e59a44fdbf57843a) 修复 HTTP 接口 SELECT 查询时崩溃
* [Commit 5957](https://github.com/manticoresoftware/manticoresearch/commit/59577513a49eac5a4a3c5e2cb38394d3246b5d35) 修复 RT 索引保存磁盘块但未标记某些文档为删除
* [Commit e861](https://github.com/manticoresoftware/manticoresearch/commit/e861f0fca0e88924450695d4e9d6acff7a36558a) 修复带有 dist_threads 的多索引或多查询搜索崩溃
* [Commit 4409](https://github.com/manticoresoftware/manticoresearch/commit/440991fc977b8479800b45cdbc862eeb1ba5d965) 修复长词宽 UTF8 码点中缀生成崩溃
* [Commit 5fd5](https://github.com/manticoresoftware/manticoresearch/commit/5fd599b48bba527c023e6aa0b262dca51ffb8a1c) 修复添加套接字到 IOCP 的竞争
* [Commit cf10](https://github.com/manticoresoftware/manticoresearch/commit/cf10d7d3589f77adfaefbe52a51777bc9c67cf99) 修复 bool 查询与 json 选择列表问题
* [Commit 996d](https://github.com/manticoresoftware/manticoresearch/commit/996de77f4c3ec103d965e28c36098fd07dba50c4) 修复 indextool 检查错误跳过列表偏移和 doc2row 查找
* [Commit 6e3f](https://github.com/manticoresoftware/manticoresearch/commit/6e3fc9e88941c9427410f9d0adeebb7dd09a900f) 修复索引器在大数据时因负跳过列表偏移生成错误索引
* [Commit faed](https://github.com/manticoresoftware/manticoresearch/commit/faed3220b423a96401982bf47117edf1f62c584e) 修复 JSON 仅将数字转换为字符串和表达式中 JSON 字符串与数字转换
* [Commit 5331](https://github.com/manticoresoftware/manticoresearch/commit/533197200249d648ebcbdaca5d605861ee566348) 修复 indextool 多命令时错误退出码
* [Commit 7955](https://github.com/manticoresoftware/manticoresearch/commit/795520ac351d22e4497ebaf6c249bf25081842b6) 修复 #275 磁盘空间不足错误时 binlog 状态失效
* [Commit 2284](https://github.com/manticoresoftware/manticoresearch/commit/2284da5e3ff77bf7be4038a4d950c0580e4159c8) 修复 #279 IN 过滤器对 JSON 属性崩溃
* [Commit ce2e](https://github.com/manticoresoftware/manticoresearch/commit/ce2e4b4765111a7d331ffc256911f12770a8942d) 修复 #281 错误管道关闭调用
* [Commit 5355](https://github.com/manticoresoftware/manticoresearch/commit/535589ba8a37baf5c6056afb5ff969fd70476feb) 修复服务器在调用 PQ 时因递归 JSON 属性编码为字符串而挂起
* [Commit a5fc](https://github.com/manticoresoftware/manticoresearch/commit/a5fc8a36e1644ee051632408ab8b53e79aeb9ff4) 修复 multiand 节点文档列表过度前进
* [Commit a362](https://github.com/manticoresoftware/manticoresearch/commit/a3628617ef4ee89a811d4ba2c4a04cbbc298c1b5) 修复线程公共信息获取
* [Commit f8d2](https://github.com/manticoresoftware/manticoresearch/commit/f8d2d7bbbb90af3d2ee97424f97b05927f09d42b) 修复文档存储缓存锁

## 版本 3.2.0，2019 年 10 月 17 日
### 新功能
* 文档存储
* 新指令 stored_fields, docstore_cache_size, docstore_block_size, docstore_compression, docstore_compression_level

### 改进和更改
* 改进了 SSL 支持
* 更新了 non_cjk 内置字符集
* 禁用 UPDATE/DELETE 语句在查询日志中记录 SELECT
* RHEL/CentOS 8 软件包

### 修复漏洞
* [提交 301a](https://github.com/manticoresoftware/manticoresearch/commit/301a806b14725e842300ee225521407464374c41) 修复在 RT 索引的磁盘块中替换文档时崩溃
* [提交 46c1](https://github.com/manticoresoftware/manticoresearch/commit/46c1cad8fa11c1fc09cc34b20d978fdfcd4e6d27) 修复 \#269 LIMIT N OFFSET M
* [提交 92a4](https://github.com/manticoresoftware/manticoresearch/commit/92a46edaa196ef33f3ac2225fbdba2c2b88ee45d) 修复带有显式设置的 id 或提供的 id 列表以跳过搜索的 DELETE 语句
* [提交 8ca7](https://github.com/manticoresoftware/manticoresearch/commit/8ca78c138d837caab30dc6e2343a3c4d1687bb87) 修复在 windowspoll 轮询器的 netloop 中事件移除后错误的索引
* [提交 6036](https://github.com/manticoresoftware/manticoresearch/commit/603631e2bff5cb0acf25f68b85621742490eccd9) 修复通过 HTTP 的 JSON 浮点舍入问题
* [提交 62f6](https://github.com/manticoresoftware/manticoresearch/commit/62f64cb9e75ec28fc3c29f8414ce9656f5580254) 修复远程片段优先检查空路径；修复 Windows 测试
* [提交 aba2](https://github.com/manticoresoftware/manticoresearch/commit/aba274c2c5abc1029894ba9f95b786bda93e8f22) 修复配置重载，使其在 Windows 上与 Linux 同样工作
* [提交 6b8c](https://github.com/manticoresoftware/manticoresearch/commit/6b8c4242ef77c3fa4d0ccb7d76d81714b6728f0b) 修复 \#194 PQ 以支持形态学和词干提取
* [提交 174d](https://github.com/manticoresoftware/manticoresearch/commit/174d312905285aa5746f4f404c53e6dc669ef42f) 修复 RT 已退役分段管理

## 版本 3.1.2，2019年8月22日
### 特性和改进
* HTTP API 的实验性 SSL 支持
* CALL KEYWORDS 的字段过滤器
* /json/search 的 max_matches
* 默认 Galera gcache.size 的自动调整大小
* 改进了对 FreeBSD 的支持

### 修复漏洞
* [提交 0a1a](https://github.com/manticoresoftware/manticoresearch/commit/0a1a2c81038794983b4e30c8aefc7cc20e89b03f) 修复 RT 索引复制到存在相同但路径不同的 RT 索引节点的问题
* [提交 4adc](https://github.com/manticoresoftware/manticoresearch/commit/4adc075294ac823289f745e2cc419f18c7dcf2e2) 修复无活动索引的刷新重新调度
* [提交 d6c0](https://github.com/manticoresoftware/manticoresearch/commit/d6c00a6f750c04021747e9c99e0f77707a4cbfc3) 改进 RT/PQ 索引刷新重新调度
* [提交 d0a7](https://github.com/manticoresoftware/manticoresearch/commit/d0a7c95906b9b5ba33c98267d54501cfe27508d6) 修复 \#250 TSV 和 CSV 管道源的 index_field_lengths 索引选项
* [提交 1266](https://github.com/manticoresoftware/manticoresearch/commit/1266d54800242709fbe2c34e72eb69b6595b23bd) 修复 indextool 对空索引的块索引检查错误报告
* [提交 553c](https://github.com/manticoresoftware/manticoresearch/commit/553ca73ca111f28e0888d59cd48b0d6df181ecd6) 修复 Manticore SQL 查询日志中空选择列表
* [提交 56c8](https://github.com/manticoresoftware/manticoresearch/commit/56c8584456293665ddd4ce4c94307d74473a78b9) 修复 indexer 的 -h/--help 响应

## 版本 3.1.0，2019年7月16日
### 特性和改进
* 实时索引复制
* 中文 ICU 分词器
* 新的形态学选项 icu_chinese
* 新指令 icu_data_dir
* 支持复制的多语句事务
* LAST_INSERT_ID() 和 @session.last_insert_id
* SHOW VARIABLES 支持 LIKE 'pattern'
* 多文档插入支持 percolate 索引
* 添加配置文件中的时间解析器
* 内部任务管理器
* 对文档和命中列表组件的 mlock
* 监狱片段路径

### 移除
* 弃用 RLP 库支持，转向使用 ICU；所有 rlp* 指令已移除
* 禁用 UPDATE 更新文档 ID

### 修复漏洞
* [提交 f047](https://github.com/manticoresoftware/manticoresearch/commit/f047222339fd0e62a07e9ca8b2a829297d5db861) 修复 concat 和 group_concat 中的缺陷
* [提交 b081](https://github.com/manticoresoftware/manticoresearch/commit/b08147ee24712b5eaa51f261626b5d16c6242fdd) 修复 percolate 索引中的查询 uid 为 BIGINT 属性类型
* [提交 4cd8](https://github.com/manticoresoftware/manticoresearch/commit/4cd85afaf76eef0ce9fa1cd073f3fed160279890) 修复预分配新磁盘块失败时不崩溃
* [提交 1a55](https://github.com/manticoresoftware/manticoresearch/commit/1a551227886474054111088412f2b2396f485652) 为 ALTER 添加缺失的 timestamp 数据类型
* [提交 f3a8](https://github.com/manticoresoftware/manticoresearch/commit/f3a8e096fd496a6b001d398aadbb781b2dbad929) 修复错误的 mmap 读取导致崩溃
* [提交 4475](https://github.com/manticoresoftware/manticoresearch/commit/447577115192db7ac7bfea70b5ba58ee2c61229f) 修复复制中群集锁的哈希问题
* [提交 ff47](https://github.com/manticoresoftware/manticoresearch/commit/ff476df955f057bd38e8b6ad6056325e9d6fbb9c) 修复复制中提供者的内存泄漏
* [提交 58dc](https://github.com/manticoresoftware/manticoresearch/commit/58dcbb779c4c30ca1560460c78e35ba114b83716) 修复 \#246 indexer 中未定义的 sigmask
* [提交 3dd8](https://github.com/manticoresoftware/manticoresearch/commit/3dd8278e747d534f69fe9c3d360eef188ab7b6d5) 修复 netloop 报告中的竞争条件
* [提交 a02a](https://github.com/manticoresoftware/manticoresearch/commit/a02aae05a3fa3d1caf2821f43c7e237b6de76336) HA 策略重新平衡器的零间隙处理

## 版本 3.0.2，2019年5月31日
### 改进
* 为文档和命中列表添加 mmap 读取器
* `/sql` HTTP 端点响应现与 `/json/search` 相同
* 新指令 `access_plain_attrs`, `access_blob_attrs`, `access_doclists`, `access_hitlists`
* 新指令 `server_id` 用于复制设置

### 移除
* 移除 HTTP `/search` 端点

### 弃用
* `read_buffer`, `ondisk_attrs`, `ondisk_attrs_default`, `mlock` 被 `access_*` 指令取代

### 修复漏洞
* [提交 849c](https://github.com/manticoresoftware/manticoresearch/commit/849c16e1040f382795ba0635cb48686f9db4f2d8) 允许 select 列表中的属性名以数字开头
* [提交 48e6](https://github.com/manticoresoftware/manticoresearch/commit/48e6c302ca37bec0bc49c79619c60bf787ed0708) 修复 UDF 中的 MVAs，修复 MVA 别名问题
* [提交 0555](https://github.com/manticoresoftware/manticoresearch/commit/055586a9a2ebfa8874f68099c5bc798a145fd4ef) 修复使用 SENTENCE 查询时 #187 崩溃问题
* [提交 93bf](https://github.com/manticoresoftware/manticoresearch/commit/93bf52f23f1c53c8a93a603387ca5463e1fc2ae8) 修复 #143 支持 MATCH() 外部的括号()
* [提交 599e](https://github.com/manticoresoftware/manticoresearch/commit/599ee79ccd38cce0023a55cbce8f50c3dca62e38) 修复 ALTER cluster 语句时集群状态保存问题
* [提交 230c](https://github.com/manticoresoftware/manticoresearch/commit/230c321e277514b191629538eb3f02d219113d95) 修复 ALTER 索引时带 blob 属性导致的服务器崩溃
* [提交 5802](https://github.com/manticoresoftware/manticoresearch/commit/5802b85aa93a81caea77073a3381ef912fdd677c) 修复 #196 按 id 过滤问题
* [提交 25d2](https://github.com/manticoresoftware/manticoresearch/commit/25d2dabda4a299a29bd375405862126da9b4bfac) 放弃对模板索引的搜索
* [提交 2a30](https://github.com/manticoresoftware/manticoresearch/commit/2a30d5b49d76faab031d195f533974938769b8e6) 修正 SQL 回复中 id 列为常规 bigint 类型


## 版本 3.0.0，2019年5月6日
### 新特性和改进
* 新的索引存储。非标量属性不再限制每个索引4GB大小
* attr_update_reserve 指令
* 字符串、JSON 和 MVA 都可以通过 UPDATE 更新
* killlists 在索引加载时应用
* killlist_target 指令
* 多重 AND 搜索的加速
* 更好的平均性能和内存使用
* 用于升级2.x版本索引的转换工具
* CONCAT() 函数
* JOIN CLUSTER cluster AT 'nodeaddress:port'
* ALTER CLUSTER posts UPDATE nodes
* node_address 指令
* 在 SHOW STATUS 中打印节点列表

### 行为变更
* 对于带有 killist 的索引，服务器不按配置文件中定义的顺序旋转索引，而是遵循 killlist 目标链
* 搜索中的索引顺序不再决定 killlists 应用的顺序
* 文档 ID 现在是有符号的大整数

### 移除的指令
* docinfo（现在总是 extern），inplace_docinfo_gap，mva_updates_pool

## 版本 2.8.2 GA，2019年4月2日
### 新特性和改进
* 用于 percolate 索引的 Galera 复制
* OPTION morphology

### 编译说明
Cmake 最低版本现在为 3.13。编译需要 boost 和 libssl
开发库。

### 修复
* [提交 6967](https://github.com/manticoresoftware/manticoresearch/commit/6967fedb2ef818ec1c825d482563edd05e1c9245) 修复在众多分布式索引的查询中，select 列表多星号导致崩溃
* [提交 36df](https://github.com/manticoresoftware/manticoresearch/commit/36df1a407dc08263690e3492518613ace82d69ca) 修复[#177](https://github.com/manticoresoftware/manticoresearch/issues/177) Manticore SQL 接口的大包问题
* [提交 5793](https://github.com/manticoresoftware/manticoresearch/commit/57932aec734583fa93359faaf5034b2e2c9d352b) 修复[#170](https://github.com/manticoresoftware/manticoresearch/issues/170) 服务器在 RT 优化时更新 MVA 导致崩溃
* [提交 edb2](https://github.com/manticoresoftware/manticoresearch/commit/edb24b870423add86eba471d361e0e5aff098b18) 修复配置重载（SIGHUP）后 RT 索引删除导致 binlog 移除时服务器崩溃
* [提交 bd3e](https://github.com/manticoresoftware/manticoresearch/commit/bd3e66e0085bc7e2e351b817dfc972fd8158fbce) 修复 mysql 握手认证插件负载
* [提交 6a21](https://github.com/manticoresoftware/manticoresearch/commit/6a217f6ed82fb10f752213b15617b9cedf1e8533) 修复[#172](https://github.com/manticoresoftware/manticoresearch/issues/172) RT 索引中的 phrase_boundary 设置
* [提交 3562](https://github.com/manticoresoftware/manticoresearch/commit/3562f652753e4091fd7b5b0f65b58341c8cbcb31) 修复[#168](https://github.com/manticoresoftware/manticoresearch/issues/168) ATTACH 索引自身死锁
* [提交 250b](https://github.com/manticoresoftware/manticoresearch/commit/250b3f0e74ad18dda34ba080aa13ca87798a9ac9) 修复服务器崩溃后 binlog 保存空元数据问题
* [提交 4aa6](https://github.com/manticoresoftware/manticoresearch/commit/4aa6c69ad3a181cd7c5dafb990fb528121f68e59) 修复由于 RT 索引带磁盘块导致 sorter 中字符串崩溃

## 版本 2.8.1 GA，2019年3月6日
### 新特性和改进
* SUBSTRING_INDEX()
* percolate 查询支持 SENTENCE 和 PARAGRAPH
* Debian/Ubuntu 的 systemd 生成器；并新增 LimitCORE 以允许 core dump

### 修复
* [提交 84fe](https://github.com/manticoresoftware/manticoresearch/commit/84fe7405d17b59f4dcde15a6c8e8503923503def) 修复 match 模式为 all 且全文查询为空时服务器崩溃
* [提交 daa8](https://github.com/manticoresoftware/manticoresearch/commit/daa88b579fa373d5d2e869e43a2e178363b0fef1) 修复删除静态字符串时崩溃
* [提交 2207](https://github.com/manticoresoftware/manticoresearch/commit/22078537dddbd9ce81a182dcc879f235b6e87004) 修正 indextool 失败时的退出代码
* [提交 0721](https://github.com/manticoresoftware/manticoresearch/commit/0721696d6780c200c65d596624a8187983fb7fcb) 修复[#109](https://github.com/manticoresoftware/manticoresearch/issues/109) 由于精确形式检查错误导致前缀无匹配
* [提交 8af8](https://github.com/manticoresoftware/manticoresearch/commit/8af810111b8e2f87bc9f378172eff1ab9725c7e7) 修复[#161](https://github.com/manticoresoftware/manticoresearch/issues/161) RT 索引配置重载问题
* [提交 e2d5](https://github.com/manticoresoftware/manticoresearch/commit/e2d592773f4fd702ace90070a793a842a0fec6ab) 修复访问大型 JSON 字符串时服务器崩溃
* [Commit 75cd](https://github.com/manticoresoftware/manticoresearch/commit/75cd1342d05cdc352e3dd145cdbc6f79394a165b) 修正了由索引剥离器修改的 JSON 文档中的 PQ 字段导致兄弟字段错误匹配的问题
* [Commit e2f7](https://github.com/manticoresoftware/manticoresearch/commit/e2f775437df922674d772ed4417780492502e65a) 修复了 RHEL7 构建中解析 JSON 时服务器崩溃的问题
* [Commit 3a25](https://github.com/manticoresoftware/manticoresearch/commit/3a25a5808feb3f8b80866991b436d6c1241618c2) 修复了当斜杠位于边缘时 JSON 反转义崩溃的问题
* [Commit be9f](https://github.com/manticoresoftware/manticoresearch/commit/be9f497872bea62dbdccd64fc28294fd7776c289) 修复了选项 'skip_empty' 应跳过空文档且不警告其非有效 JSON 的问题
* [Commit 266e](https://github.com/manticoresoftware/manticoresearch/commit/266e0e7b088549722a805fc837bf101ff681a5e8) 修正了[#140](https://github.com/manticoresoftware/manticoresearch/issues/161) 当 6 位小数不足以表示精度时，浮点数输出 8 位数字的问题
* [Commit 3f6d](https://github.com/manticoresoftware/manticoresearch/commit/3f6d23891064ee8e94030c4231497cdd7da33a6a) 修复了空 jsonobj 创建的问题
* [Commit f3c7](https://github.com/manticoresoftware/manticoresearch/commit/f3c7848a59b7eedc67c3403d6d4b37ce6fa94dc8) 修复了[#160](https://github.com/manticoresoftware/manticoresearch/issues/161) 空 mva 输出 NULL 而非空字符串的问题
* [Commit 0afa](https://github.com/manticoresoftware/manticoresearch/commit/0afa2ed058e5759470b1d7354c722faab34f98bb) 修复了在无 pthread_getname_np 环境下构建失败的问题
* [Commit 9405](https://github.com/manticoresoftware/manticoresearch/commit/9405fccdeb27f0302a8d9a848981d30080216777) 修复了线程池工作人员导致服务器关闭时崩溃的问题

## 版本 2.8.0 GA，2019 年 1 月 28 日
### 改进
* 支持 percolate 索引的分布式索引
* CALL PQ 新选项和更改：
    *   skip_bad_json
    *   mode (稀疏/sparsed、分片/sharded)
    *   JSON 文档现在可以作为 JSON 数组传递
    *   shift
    *   列名 'UID'、'Documents'、'Query'、'Tags'、'Filters' 被重命名为 'id'、'documents'、'query'、'tags'、'filters'
* 支持 DESCRIBE pq TABLE
* 不再支持 WHERE 使用 UID 查询 pq 表，请使用 'id' 代替
* pq 索引的 SELECT 表现与常规模拟索引相当（例如可通过 REGEX() 过滤规则）
* PQ 标签支持 ANY/ALL
* 表达式自动转换 JSON 字段，无需显式类型转换
* 内置 'non_cjk' 字符集表和 'cjk' ngram_chars
* 提供 50 种语言的内置停用词集合
* 停用词声明中多文件也可用逗号分隔
* CALL PQ 支持接受 JSON 文档数组

### Bug 修复
* [Commit a4e1](https://github.com/manticoresoftware/manticoresearch/commit/a4e19afee54dafdc04b0da53372dafd8c0d332d6) 修复了 csjon 相关的内存泄露
* [Commit 28d8](https://github.com/manticoresoftware/manticoresearch/commit/28d862774874891a03e361da1b0347ebe8066ce0) 修复了 JSON 缺失值导致崩溃的问题
* [Commit bf4e](https://github.com/manticoresoftware/manticoresearch/commit/bf4e9ea27c349cdc9ae4e54e960a4a17b90e38fa) 修复了 RT 索引保存时空元数据的问题
* [Commit 33b4](https://github.com/manticoresoftware/manticoresearch/commit/33b4573529e7c7c7bce19bf9d54054866f30d99a) 修复形态分析序列丢失形式标记（exact）的问题
* [Commit 6b95](https://github.com/manticoresoftware/manticoresearch/commit/6b95d48240d3b5520afade19c249d79e020a5e88) 修复字符串属性超过 4M 时应使用饱和而非溢出的问题
* [Commit 6214](https://github.com/manticoresoftware/manticoresearch/commit/621418b829e70af36aaa322a2f51ece3f86bc499) 修复禁用索引后服务器在接收 SIGHUP 信号时崩溃的问题
* [Commit 3f7e](https://github.com/manticoresoftware/manticoresearch/commit/3f7e35d1482966cc45d5abbcb2de5de3508d66b0) 修复同时执行 API 会话状态命令时服务器崩溃问题
* [Commit cd9e](https://github.com/manticoresoftware/manticoresearch/commit/cd9e4f1709a48ddafc128c450c2d882bc11ba50e) 修复带字段过滤条件向 RT 索引删除查询时服务器崩溃的问题
* [Commit 9376](https://github.com/manticoresoftware/manticoresearch/commit/9376470d455fcd256c6abff79c431a6919dfa7ac) 修复调用分布式索引的 CALL PQ 时空文档导致服务器崩溃
* [Commit 8868](https://github.com/manticoresoftware/manticoresearch/commit/8868b207644f4f8cc1ab0c270adb35493fed22e1) 修复 Manticore SQL 错误消息超过 512 字符被截断的问题
* [Commit de9d](https://github.com/manticoresoftware/manticoresearch/commit/de9deda9c142823d6dbf529423417b5c670fae94) 修复无 binlog 情况下保存 percolate 索引时崩溃
* [Commit 2b21](https://github.com/manticoresoftware/manticoresearch/commit/2b219e1a32791e7740bb210b7d408f96abc6374f) 修复 OSX 系统中 HTTP 接口不可用的问题
* [Commit e92c](https://github.com/manticoresoftware/manticoresearch/commit/e92c60240f8f9e1756bfa14fba0bbb402586a389) 修复 indextool 检查 MVA 时误报错误
* [Commit 238b](https://github.com/manticoresoftware/manticoresearch/commit/238bdea59bad89f097403f1c978658ce45f16c70) 修复 FLUSH RTINDEX 的写锁，避免保存和定期刷新时加写锁锁住整个索引
* [Commit c26a](https://github.com/manticoresoftware/manticoresearch/commit/c26a236bd42b7082db079a51a84172066d337d64) 修复 ALTER percolate 索引时等待搜索加载卡住的问题
* [Commit 9ee5](https://github.com/manticoresoftware/manticoresearch/commit/9ee5703f6c28f57cd5bff5e705966d93fe30d267) 修复 max_children 为 0 时应使用默认线程池工作线程数
* [Commit 5138](https://github.com/manticoresoftware/manticoresearch/commit/5138fc048a1bd146b271ce6a72fe954344281f69) 修复使用 index_token_filter 插件搭配停用词且 stopword_step=0 时索引数据出错
* [Commit 2add](https://github.com/manticoresoftware/manticoresearch/commit/2add3d319ac62fe450bf60e89033100853dc2ecf) 修复在索引定义中仍使用 aot 形态分析器时缺失 lemmatizer_base 导致崩溃

## 版本 2.7.5 GA，2018 年 12 月 4 日
### 改进
* REGEX 函数
* JSON API 搜索支持 limit/offset
* qcache 的性能分析点

### Bug 修复
* [Commit eb3c](https://github.com/manticoresoftware/manticoresearch/commit/eb3c7683b9740cff4cc4cadd2ab9f990b9f7d173) 修复了在FACET中使用多个宽类型属性时服务器崩溃的问题
* [Commit d915](https://github.com/manticoresoftware/manticoresearch/commit/d915cf6696514f4be365ffb5981bccaa15983e65) 修复了FACET查询主选择列表中隐式GROUP BY的问题
* [Commit 5c25](https://github.com/manticoresoftware/manticoresearch/commit/5c25dc271a8c11a1c82ca3c88e518acbc6a8c164) 修复了使用GROUP N BY查询时的崩溃问题
* [Commit 85d3](https://github.com/manticoresoftware/manticoresearch/commit/85d30a232d4ffd89916b673a9b2db946d272a945) 修复了内存操作处理崩溃时造成的死锁问题
* [Commit 8516](https://github.com/manticoresoftware/manticoresearch/commit/85166b5ef2be38061d62f2b20f15acaa0311258a) 修复了indextool在检查时的内存消耗问题
* [Commit 58fb](https://github.com/manticoresoftware/manticoresearch/commit/58fb03166830d8b23e7ab9ce56309db7ddac22f4) 修复了gmock的include不再需要，因为上游已解决

## 版本 2.7.4 GA，2018年11月1日
### 改进
* 在远程分布式索引的情况下，SHOW THREADS显示原始查询而非API调用
* SHOW THREADS新增选项`format=sphinxql`，以SQL格式打印所有查询
* SHOW PROFILE打印额外的`clone_attrs`阶段

### Bug修复
* [Commit 4f15](https://github.com/manticoresoftware/manticoresearch/commit/4f155712a0bccc1bd01cc191647bc8cff814888e) 修复了在libc缺少malloc_stats、malloc_trim时构建失败的问题
* [Commit f974](https://github.com/manticoresoftware/manticoresearch/commit/f974f20bda3214a56877c393a192be1a77150958) 修复了CALL KEYWORDS结果集内词语中的特殊字符问题
* [Commit 0920](https://github.com/manticoresoftware/manticoresearch/commit/092083282ea8ae0b2e72fb6989c57ccec81e74ac) 修复了通过API调用分布式索引或远程代理时CALL KEYWORDS失效问题
* [Commit fd68](https://github.com/manticoresoftware/manticoresearch/commit/fd686bfe88b720ffd7642e36897ba45161cbd7d2) 修复了分布式索引的agent_query_timeout未作为max_query_time传递给代理的问题
* [Commit 4ffa](https://github.com/manticoresoftware/manticoresearch/commit/4ffa623e9d357a3b0b441615089e211f92f8de32) 修复了磁盘块的总文档计数被OPTIMIZE命令影响，导致权重计算错误的问题
* [Commit dcaf](https://github.com/manticoresoftware/manticoresearch/commit/dcaf4e0e3c8e9b0fbec47bbf3307feddec0936a6) 修复了来自blended的多个尾部命中问题在实时索引中的表现
* [Commit eee3](https://github.com/manticoresoftware/manticoresearch/commit/eee381754e902a43a8f499e1c950198a2a3e6ee0) 修复了旋转时的死锁问题

## 版本 2.7.3 GA，2018年9月26日
### 改进
* 为CALL KEYWORDS添加sort_mode选项
* VIP连接下的DEBUG可以执行'crash <password>'故意触发服务器SIGEGV
* DEBUG支持执行'malloc_stats'以将malloc统计信息转储到searchd.log，及执行'malloc_trim'调用malloc_trim()
* 如果系统中存在gdb，改进了回溯信息

### Bug修复
* [Commit 0f3c](https://github.com/manticoresoftware/manticoresearch/commit/0f3cc333f6129451ad1ae2c97905b6c04ba5d454) 修复了Windows下重命名导致的崩溃或失败问题
* [Commit 1455](https://github.com/manticoresoftware/manticoresearch/commit/1455ba2bf8079f03ce7f861a8d9662a360d705de) 修复了32位系统服务器崩溃问题
* [Commit ad37](https://github.com/manticoresoftware/manticoresearch/commit/ad3710d53b5cd6a28b8c60352d0f9e285b03d9fa) 修复了空SNIPPET表达式导致的服务器崩溃或挂起问题
* [Commit b36d](https://github.com/manticoresoftware/manticoresearch/commit/b36d79214364500252665860f000ae85343528b6) 修复了非渐进式OPTIMIZE损坏的问题，并修正了渐进式OPTIMIZE不为最旧磁盘块创建kill-list的问题
* [Commit 34b0](https://github.com/manticoresoftware/manticoresearch/commit/34b032499afd42ce47a4c7247814b4031094388a) 修复了线程池工作模式下SQL和API查询queue_max_length返回错误的问题
* [Commit ae4b](https://github.com/manticoresoftware/manticoresearch/commit/ae4b3202cbdb8014cabe2b90e269d5cf74f49871) 修复了添加带regexp或rlp选项的全扫描查询到PQ索引时崩溃的问题
* [Commit f80f](https://github.com/manticoresoftware/manticoresearch/commit/f80f8d5d7560187078868aed9a9575f4549e98aa) 修复了连续调用PQ导致的崩溃
* [Commit 9742](https://github.com/manticoresoftware/manticoresearch/commit/9742f5f0866af73f8cd8483ecd18a507ea80dd65) 重构了AcquireAccum
* [Commit 39e5](https://github.com/manticoresoftware/manticoresearch/commit/39e5bc3751b7295222eb76407c5d72ce1dad545b) 修复了调用pq后内存泄漏
* [Commit 21bc](https://github.com/manticoresoftware/manticoresearch/commit/21bcc6d17395f0f57dde79f6716ef303b7ea527d) 代码风格优化（c++11风格的C构造函数、默认参数、nullptr）
* [Commit 2d69](https://github.com/manticoresoftware/manticoresearch/commit/2d690398f14c736956cfdd66feb4d3091d6b3a4d) 修复了尝试向PQ索引插入重复项时的内存泄漏
* [Commit 5ed9](https://github.com/manticoresoftware/manticoresearch/commit/5ed92c4b66da6423fa28f11b1fd59103ed1e1fb9) 修复了JSON字段IN条件中存在大值时导致的崩溃
* [Commit 4a52](https://github.com/manticoresoftware/manticoresearch/commit/4a5262e2776aa261e34ba5c914ea60804f59de3f) 修复了向设置了扩展限制的RT索引发送CALL KEYWORDS语句时服务器崩溃
* [Commit 5526](https://github.com/manticoresoftware/manticoresearch/commit/552646bb6fefde4e2525298e43e628839b421d3d) 修复了PQ匹配查询中的无效过滤器问题
* [Commit 204f](https://github.com/manticoresoftware/manticoresearch/commit/204f521408b730198a5bab2c20407a3d7df94873) 为ptr属性引入了小对象分配器
* [Commit 2545](https://github.com/manticoresoftware/manticoresearch/commit/25453e5387152c0575d1eda1b2beb89a49c7493a) 将ISphFieldFilter重构为带引用计数的版本
* [Commit 1366](https://github.com/manticoresoftware/manticoresearch/commit/1366ee06e828397074b69bec9265d7147170c60e) 修复了使用strtod处理非终止字符串时的未定义行为/段错误
* [Commit 94bc](https://github.com/manticoresoftware/manticoresearch/commit/94bc6fcbdae4d08bcf6da2966e3ec2966091289f) 修复了 json 结果集处理中的内存泄漏
* [Commit e78e](https://github.com/manticoresoftware/manticoresearch/commit/e78e9c948963416caff9e4e46296a58080107835) 修复了在应用属性添加时对内存块末尾的越界读取
* [Commit fad5](https://github.com/manticoresoftware/manticoresearch/commit/fad572fb543606b33e688eb82a485b86011545fd) 修复并重构了 CSphDict 的引用计数版本
* [Commit fd84](https://github.com/manticoresoftware/manticoresearch/commit/fd841a472b472280c93f4af3506851d19f6adc45) 修复了 AOT 内部类型在外部的泄漏
* [Commit 5ee7](https://github.com/manticoresoftware/manticoresearch/commit/5ee7f208c7685f2ebb9e5623a8802b721e2a7f0a) 修复了分词器管理中的内存泄漏
* [Commit 116c](https://github.com/manticoresoftware/manticoresearch/commit/116c5f1abebee9a0d99afe93546a1d8e4c6c6958) 修复了 grouper 中的内存泄漏
* [Commit 56fd](https://github.com/manticoresoftware/manticoresearch/commit/56fdbc9ab38973a9a0ba8ccee45f71cf33c16423) 为 matches 中的动态指针提供特殊的释放/复制（修复 grouper 的内存泄漏）
* [Commit b1fc](https://github.com/manticoresoftware/manticoresearch/commit/31db660f378541aa0d1e7cf2e7209bb0a1f47fd8) 修复了 RT 动态字符串的内存泄漏
* [Commit 517b](https://github.com/manticoresoftware/manticoresearch/commit/517b9e8749f092e923cd2884b674b5bb84e20755) 重构了 grouper
* [Commit b1fc](https://github.com/manticoresoftware/manticoresearch/commit/b1fc16140e5dc44290686330b476a254e0092107) 小规模重构（c++11 c-trs，一些格式调整）
* [Commit 7034](https://github.com/manticoresoftware/manticoresearch/commit/7034e073f4cf2844762e0a464b7c3de05d3d122b) 重构 ISphMatchComparator 为引用计数版本
* [Commit b1fc](https://github.com/manticoresoftware/manticoresearch/commit/b1fc16140e5dc44290686330b476a254e0092107) 私有化 cloner
* [Commit efbc](https://github.com/manticoresoftware/manticoresearch/commit/efbc0511d6809c4a57453c7283f9bf53f3fb8d97) 简化了 MVA_UPSIZE、DOCINFO2ID_T、DOCINFOSETID 的本地小端表示
* [Commit 6da0](https://github.com/manticoresoftware/manticoresearch/commit/6da0df4ac9946fc59f98d229b90e12c9221dd4c0) 为 ubertests 增加 valgrind 支持
* [Commit 1d17](https://github.com/manticoresoftware/manticoresearch/commit/1d17669cb7ec54ac80b392e101f37a688eb98080) 修复了连接中 'success' 标志的竞态导致的崩溃
* [Commit 5a09](https://github.com/manticoresoftware/manticoresearch/commit/5a09c32d7ded56b2c0bd3e2ad7968cb8d6362919) 将 epoll 切换为边缘触发模式
* [Commit 5d52](https://github.com/manticoresoftware/manticoresearch/commit/5d528682737fca03a4352a3093b1319ec27d598c) 修复了类似过滤器格式表达式中 IN 语句的问题
* [Commit bd8b](https://github.com/manticoresoftware/manticoresearch/commit/bd8b3c976ff8b4667f55dfdd101b20a920137ac5) 修复在 RT 索引提交大文档 ID 文档时的崩溃
* [Commit ce65](https://github.com/manticoresoftware/manticoresearch/commit/ce656b83b747ce7c55795b53915770c13fb99cfe) 修复 indextool 中无参数选项的问题
* [Commit 08c9](https://github.com/manticoresoftware/manticoresearch/commit/08c9507177820f441f534bf06a11dac5e54bebb4) 修复扩展关键字的内存泄漏
* [Commit 30c7](https://github.com/manticoresoftware/manticoresearch/commit/30c75a2f525ec9bda625d903acdc9d4d2e2413dc) 修复 json grouper 的内存泄漏
* [Commit 6023](https://github.com/manticoresoftware/manticoresearch/commit/6023f269b6f2080e4d380b0957605ef8107c8c9f) 修复全局用户变量泄漏
* [Commit 7c13](https://github.com/manticoresoftware/manticoresearch/commit/7c138f15ca23c0c8717fa12041e3db7f988fcf15) 修复早期拒绝的匹配项中动态字符串泄漏
* [Commit 9154](https://github.com/manticoresoftware/manticoresearch/commit/9154b18eaed5302e8330cb3a95bd968959ccb312) 修复 length(<expression>) 的泄漏
* [Commit 43fc](https://github.com/manticoresoftware/manticoresearch/commit/43fca3a4e26139765d0fac8de054200dc4875fc6) 修复 parser 中因 strdup() 造成的内存泄漏
* [Commit 71ff](https://github.com/manticoresoftware/manticoresearch/commit/71ff77737e63d1019b7c9804dca7fa2351025dba) 修复表达式解析器的重构以精准跟踪引用计数

## 版本 2.7.2 GA，2018年8月27日
### 改进
* 兼容 MySQL 8 客户端
* [TRUNCATE](Emptying_a_table.md) 支持 WITH RECONFIGURE
* 移除了 RT 索引 SHOW STATUS 中的内存计数器
* 多代理的全局缓存
* 改进了 Windows 下的 IOCP
* HTTP 协议的 VIP 连接
* Manticore SQL 的 [DEBUG](Reporting_bugs.md#DEBUG) 命令，可运行多种子命令
* [shutdown_token](Server_settings/Searchd.md#shutdown_token) - 需要使用 SHA1 密码哈希通过 DEBUG 命令调用 `shutdown`
* 新增 SHOW AGENT STATUS 的统计项 (_ping, _has_perspool, _need_resolve)
* indexer 的 --verbose 选项现支持 \[debugvv\] 以打印调试信息

### Bug修复
* [Commit 3900](https://github.com/manticoresoftware/manticoresearch/commit/390082a7be0a1f9539b30361d11d54de35c62a44) 移除了优化时的写锁
* [Commit 4c33](https://github.com/manticoresoftware/manticoresearch/commit/4c337648329be22373be33333cbc9c5664d18b80) 修复重载索引设置时的写锁
* [Commit b5ea](https://github.com/manticoresoftware/manticoresearch/commit/b5ea8dc0b18a23dbd739e92c66dd93b0094df14b) 修复带 JSON 过滤条件查询时的内存泄漏
* [Commit 930e](https://github.com/manticoresoftware/manticoresearch/commit/930e831a43fd3d1ce996185be968d1df49a088d0) 修复 PQ 结果集中空文档
* [Commit 53de](https://github.com/manticoresoftware/manticoresearch/commit/53deecfecb374431f84f8592a1095a77407b8aea) 修复因删除任务导致的任务混乱
* [Commit cad9](https://github.com/manticoresoftware/manticoresearch/commit/cad9b9c617719aad97e9f5a2ae93491153d8e504) 修复远程主机计数错误
* [Commit 9000](https://github.com/manticoresoftware/manticoresearch/commit/90008cc15c4684d445d37a391bb4a667bbc2e365) 修复解析的代理描述符内存泄漏
* [Commit 978d](https://github.com/manticoresoftware/manticoresearch/commit/978d839641ad6040c22c9fc3b703b1a02685f266) 修复了搜索中的内存泄漏
* [Commit 0193](https://github.com/manticoresoftware/manticoresearch/commit/0193946105ac08913362b7d7acf0c90f9b38dc85) 对显式/内联构造函数、override/final 的使用进行美化更改
* [Commit 943e](https://github.com/manticoresoftware/manticoresearch/commit/943e2997b33f9b2eda23d94a66068910f9476ebd) 修复了本地/远程模式中 json 的内存泄漏
* [Commit 02db](https://github.com/manticoresoftware/manticoresearch/commit/02dbdd6f1d0b72e7e458ebebdd2c67f989577e6b) 修复了本地/远程模式中 json 排序列表达式的内存泄漏
* [Commit c74d](https://github.com/manticoresoftware/manticoresearch/commit/c74d0b4035e4214510376845b22cb676a8da24a3) 修复了 const alias 的内存泄漏
* [Commit 6e5b](https://github.com/manticoresoftware/manticoresearch/commit/6e5b57e1d3952c9695376728a143bf5434208f53) 修复了预读取线程的内存泄漏
* [Commit 39c7](https://github.com/manticoresoftware/manticoresearch/commit/39c740fef5bf71da0bee60a35f5dd9f471af850f) 修复了因为网络循环中等待阻塞导致退出卡住的问题
* [Commit adaf](https://github.com/manticoresoftware/manticoresearch/commit/adaf9772f95772ad6d5297acace6c5cb92b56ad5) 修复了 HA 代理切换到普通主机时“ping”行为卡住的问题
* [Commit 32c4](https://github.com/manticoresoftware/manticoresearch/commit/32c40eac9cdd40b15d79e58b2f15416f164f9701) 为仪表盘存储分离了垃圾回收
* [Commit 511a](https://github.com/manticoresoftware/manticoresearch/commit/511a3cf2c1f1858f6e94fe9f8175b7216db3cbd6) 修复了引用计数指针的问题
* [Commit 32c4](https://github.com/manticoresoftware/manticoresearch/commit/af2326cd2927effbad7bec88e8dc238a566cf27e) 修复了 indextool 在不存在索引时崩溃的问题
* [Commit 156e](https://github.com/manticoresoftware/manticoresearch/commit/156edc717dc63d3ea120f42466eba6e022da88be) 修复了 xmlpipe 索引中过多属性/字段的输出名称
* [Commit cdac](https://github.com/manticoresoftware/manticoresearch/commit/cdac6d1f17d384bfc8bae49e91241b2f346c3830) 修复了配置中无索引器段时默认索引器的值
* [Commit e61e](https://github.com/manticoresoftware/manticoresearch/commit/e61ec00b6b27d1d5878247e2ee817f3b1e7fde16) 修复了服务器重启后 RT 索引磁盘块中嵌入错误的停用词
* [Commit 5fba](https://github.com/manticoresoftware/manticoresearch/commit/5fba49d5e28c17de4c0acbd984466127f42de6e8) 修复了跳过幻影连接（已关闭但尚未完全从轮询器中删除）
* [Commit f22a](https://github.com/manticoresoftware/manticoresearch/commit/f22ae34b623906b3c5a05a06198dad2e548dc541) 修复了混杂（孤立）网络任务
* [Commit 4689](https://github.com/manticoresoftware/manticoresearch/commit/46890e70eb8dcd00c1bf1b030538b8f131c601c2) 修复了写入后读取操作崩溃
* [Commit 03f9](https://github.com/manticoresoftware/manticoresearch/commit/03f9df904f611c438d5ebcaaeef911b0dc8d6c39) 修复了在 Windows 上运行测试时 searchd 崩溃的问题
* [Commit e925](https://github.com/manticoresoftware/manticoresearch/commit/e9255ed2704790a3d7f5273d38b85433463cb3ff) 修复了常规 connect() 处理中 EINPROGRESS 代码的问题
* [Commit 248b](https://github.com/manticoresoftware/manticoresearch/commit/248b72f1544b27e9e82773284050d44eeb1714e1) 修复了使用 TFO 通信时连接超时的问题

## 版本 2.7.1 GA，2018年7月4日
### 改进
* 改善了 PQ 上多文档匹配时的通配符性能
* 支持 PQ 上的全扫描查询
* 支持 PQ 上的多值属性（MVA）
* 支持 percolate 索引的正则表达式和 RLP

### BUG 修复
* [Commit 6885](https://github.com/manticoresoftware/manticoresearch/commit/68856261b41f6950666f9e5122726839fb3c71d0) 修复了查询字符串丢失的问题
* [Commit 0f17](https://github.com/manticoresoftware/manticoresearch/commit/0f1770943cba53bfd2e0edfdf0b1495ff0dd0e89) 修复了 SHOW THREADS 语句中信息为空的问题
* [Commit 53fa](https://github.com/manticoresoftware/manticoresearch/commit/53faa36a916bb87868e83aa79c1a1f972fb20ca0) 修复了使用 NOTNEAR 操作符匹配时崩溃的问题
* [Commit 2602](https://github.com/manticoresoftware/manticoresearch/commit/26029a7cd77b518fb3a27588d7db6c8bffc73b0f) 修复了对 PQ 删除的不正确过滤错误信息


## 版本 2.7.0 GA，2018年6月11日
### 改进
* 减少系统调用次数以避免 Meltdown 和 Spectre 补丁的影响
* 本地索引管理的内部重写
* 远程摘要重构
* 完整配置重载
* 所有节点连接现为独立连接
* 协议改进
* Windows 通信从 wsapoll 切换到 IO 完成端口
* 主节点与节点间通信可使用 TFO
* [SHOW STATUS](Node_info_and_management/Node_status.md#SHOW-STATUS) 现在输出服务器版本和 mysql_version_string
* 增加了用于 CALL PQ 中调用文档的 `docs_id` 选项
* percolate 查询过滤器现在支持包含表达式
* 分布式索引可与 FEDERATED 一起使用
* 虚拟 SHOW NAMES COLLATE 和 `SET wait_timeout`（提升 ProxySQL 兼容性）

### BUG 修复
* [Commit 5bcf](https://github.com/manticoresoftware/manticoresearch/commit/5bcff05d19e495f1a44f190885b25cbb6f217c4a) 修复了 PQ 中添加“不等于”标签的问题
* [Commit 9ebc](https://github.com/manticoresoftware/manticoresearch/commit/9ebc58916cd515eaa88da66d0895aebf0d1f2b5f) 修复了添加文档 ID 字段到 JSON 文档 CALL PQ 语句的问题
* [Commit 8ae0](https://github.com/manticoresoftware/manticoresearch/commit/8ae0e593a286110d8505d88baa3ac9e433cb38c9) 修复了 PQ 索引的刷新语句处理器问题
* [Commit c24b](https://github.com/manticoresoftware/manticoresearch/commit/c24b152344ecd77661566cddb803487f51d3c1aa) 修复了 PQ 对 JSON 和字符串属性的过滤问题
* [Commit 1b8b](https://github.com/manticoresoftware/manticoresearch/commit/1b8bdde19aaa362785ea4c33bdc019154f8fbac4) 修复了空 JSON 字符串的解析问题
* [Commit 1ad8](https://github.com/manticoresoftware/manticoresearch/commit/1ad8a07dbfc1131913a8d92c261fbb48f934e8b7) 修复了多查询中使用 OR 过滤器时的崩溃问题
* [Commit 69b8](https://github.com/manticoresoftware/manticoresearch/commit/69b89806380a229e36287ad9daf6f0ea2b5453eb) 修复 indextool 使用配置公共部分（lemmatizer_base 选项）进行命令（dumpheader）
* [Commit 6dbe](https://github.com/manticoresoftware/manticoresearch/commit/6dbeaf2389d2af6a9dfccb56bfc986685b9f203e) 修复结果集和过滤器中的空字符串
* [Commit 39c4](https://github.com/manticoresoftware/manticoresearch/commit/39c4eb55ecc85f6dd54f6c8c6d6dfcf87fd8748e) 修复负文档ID值问题
* [Commit 266b](https://github.com/manticoresoftware/manticoresearch/commit/266b7054f98e7d30fc2829b5c2467dc9f90def92) 修复非常长单词索引时的单词裁剪长度
* [Commit 4782](https://github.com/manticoresoftware/manticoresearch/commit/47823b0bd06cbd016a24345c99bb8693790b43dd) 修复PQ中通配符查询匹配多个文档的问题


## 版本 2.6.4 GA, 2018年5月3日
### 功能和改进
* MySQL FEDERATED 引擎[支持](Extensions/FEDERATED.md)
* MySQL 数据包现在返回 SERVER_STATUS_AUTOCOMMIT 标志，增加了与 ProxySQL 的兼容性
* [listen_tfo](Server_settings/Searchd.md#listen_tfo) - 为所有监听器启用 TCP 快速打开连接
* indexer --dumpheader 也可以从 .meta 文件中转储 RT 头信息
* 针对 Ubuntu Bionic 的 cmake 构建脚本

### Bug 修复
* [Commit 355b](https://github.com/manticoresoftware/manticoresearch/commit/355b11629174813abd9cd5bf2233be0783f77745) 修复了 RT 索引的无效查询缓存条目；
* [Commit 546e](https://github.com/manticoresoftware/manticoresearch/commit/546e2297a01cc2913bc0d33052d5bcefae8737eb) 修复无限旋转后索引设置丢失的问题
* [Commit 0c45](https://github.com/manticoresoftware/manticoresearch/commit/0c4509898393993bc87194d2aca2070395ff7f83) 修正固定后缀与前缀长度设定；增加对不支持后缀长度的警告
* [Commit 8054](https://github.com/manticoresoftware/manticoresearch/commit/80542fa54d727c781635918e8d4d65a20c2fbd1e) 修正RT索引自动刷新顺序
* [Commit 705d](https://github.com/manticoresoftware/manticoresearch/commit/705d8c5f407726ed1c6f9745ecffa375682c8969) 修正多属性索引和多个索引查询的结果集架构问题
* [Commit b0ba](https://github.com/manticoresoftware/manticoresearch/commit/b0ba932fff4a082f642cb737381c8a70215d4cc9) 修复批量插入时部分重复文档丢失的问题
* [Commit 4510](https://github.com/manticoresoftware/manticoresearch/commit/4510fa44ad54b5bbac53c591144501565130fcae) 修复优化时合并大量文档的RT索引磁盘块失败

## 版本 2.6.3 GA, 2018年3月28日
### 改进
* 编译时支持 jemalloc。如果系统中存在 jemalloc，可通过 cmake 标志 `-DUSE_JEMALLOC=1` 启用

### Bug修复
* [Commit 85a6](https://github.com/manticoresoftware/manticoresearch/commit/85a6d7e3db75f28c491042409bb677ed39eba07b) 修复日志中的 expand_keywords 选项，显示为 Manticore SQL 查询日志
* [Commit caaa](https://github.com/manticoresoftware/manticoresearch/commit/caaa3842f4dbd8507fd9e20bf3371a0c8ec0c087) 修复 HTTP 接口正确处理大尺寸查询
* [Commit e386](https://github.com/manticoresoftware/manticoresearch/commit/e386d842b36195719e459b2db1fb20e402f9a51f) 修复启用 index_field_lengths 时，DELETE RT 索引导致的服务器崩溃
* [Commit cd53](https://github.com/manticoresoftware/manticoresearch/commit/ce528f37eb5d8284661928f6f3b340a4a1985253) 修正 cpustats searchd 命令行选项，使其能在不支持系统上运行
* [Commit 8740](https://github.com/manticoresoftware/manticoresearch/commit/8740fd63ae842c2ab0e51a8fcd6180d787ed57ec) 修正定义有最小长度时的 utf8 子串匹配


## 版本 2.6.2 GA, 2018年2月23日
### 改进
* 在使用 NOT 操作符和批量文档时，提升[Percolate Queries](Searching/Percolate_query.md)性能
* [percolate_query_call](Searching/Percolate_query.md) 根据 [dist_threads](Server_settings/Searchd.md#threads) 支持多线程
* 新增全文匹配操作符 NOTNEAR/N
* percolate 索引的 SELECT 支持 LIMIT
* [expand_keywords](Searching/Options.md#expand_keywords) 可接受 'start','exact'（其中 'start,exact' 的效果等同于 '1'）
* 对使用 sql_query_range 定义的范围查询，支持 [joined fields](Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Processing_fetched_data.md#sql_joined_field)

### Bug修复
* [Commit 72dc](https://github.com/manticoresoftware/manticoresearch/commit/72dcf669744e9b7d636dfc213d24df85ab301f6b) 修复搜索内存段崩溃；使用双缓冲保存磁盘块死锁；优化期间保存磁盘块死锁
* [Commit 3613](https://github.com/manticoresoftware/manticoresearch/commit/36137149a1c3c0893bdda5a28fc7e8244bf2d4ae) 修复 indexer 在xml嵌入式模式中空属性名导致崩溃
* [Commit 48d7](https://github.com/manticoresoftware/manticoresearch/commit/48d7e8001d2a66466ca64577f27ddc5421a67251) 修正错误地卸载非所拥有的 pid 文件
* [Commit a556](https://github.com/manticoresoftware/manticoresearch/commit/a5563a465ddc59ef71e65f17b68bc33f9700e838) 修复偶尔在临时文件夹遗留孤立 FIFO 的问题
* [Commit 2376](https://github.com/manticoresoftware/manticoresearch/commit/2376e8fc4508944b96959bd10686c6d51f5145e8) 修复 FACET 结果集因错误的 NULL 行导致为空
* [Commit 4842](https://github.com/manticoresoftware/manticoresearch/commit/4842b67acdd5da75672db6c08ac563b48577ddd7) 修正 Windows 服务模式下的索引锁损坏问题
* [Commit be35](https://github.com/manticoresoftware/manticoresearch/commit/be35feef54ded59125750916cd211e56108eddae) 修正 mac os 上错误的 iconv 库引用
* [Commit 8374](https://github.com/manticoresoftware/manticoresearch/commit/83744a977d7837d1d87cd506b88b0e7eb199efe6) 修正错误的 count(\*)


## 版本 2.6.1 GA, 2018年1月26日
### 改进
* [agent_retry_count](Server_settings/Searchd.md#agent_retry_count) 在带镜像的代理情况下，返回的是每个镜像的重试次数，而不是每个代理的重试次数，每个代理的总重试次数是 agent_retry_count\*mirrors。
* [agent_retry_count](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_retry_count) 现在可以针对每个索引指定，覆盖全局值。新增别名 [mirror_retry_count](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#mirror_retry_count)。
* 在代理定义中可以指定 retry_count，且该值代表每个代理的重试次数。
* 现在在 HTTP JSON API 的 [/json/pq](Searching/Percolate_query.md#Performing-a-percolate-query-with-CALL-PQ) 支持 Percolate 查询。
* 给可执行程序添加了 -h 和 -v 选项（帮助和版本）。
* 支持 Real-Time 索引的 [morphology_skip_fields](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology)。

### Bugfixes
* [Commit a40b](https://github.com/manticoresoftware/manticore/commit/a40b0793feff65e40d10062568d9847c08d10f57) 修复了 ranged-main-query 正确支持 sql_range_step 用于 MVA 字段的问题。
* [Commit f2f5](https://github.com/manticoresoftware/manticore/commit/f2f53757db45bcfb1544263ce0817e856656a621) 修复了黑洞系统循环阻塞问题及黑洞代理看起来断开连接的问题。
* [Commit 84e1](https://github.com/manticoresoftware/manticore/commit/84e1f54aef25e0fce98870ad2dd784db5116f1d6) 修正查询 ID 保持一致，修复了存储查询重复 ID 问题。
* [Commit 1948](https://github.com/manticoresoftware/manticore/commit/19484231814fcb82b21763a3a4a9f45adc6b2d40) 修复了服务器在多种状态下关闭时崩溃的问题。
* [Commit 9a70](https://github.com/manticoresoftware/manticore/commit/9a706b499a1d61a90076065a1a703029d49db958)  [Commit 3495](https://github.com/manticoresoftware/manticore/commit/3495fd70cba8846b1a50d55d6679b039414c3d2a) 修复长查询时超时问题。
* [Commit 3359](https://github.com/manticoresoftware/manticore/commit/3359bcd89b4f79a645fe84b8cf8616ce0addff02) 重构了基于 kqueue 系统（Mac OS X、BSD）上 master-agent 网络轮询。


## Version 2.6.0, 2017年12月29日
### Features and improvements
* [HTTP JSON](Connecting_to_the_server/HTTP.md)：JSON 查询现在支持属性、MVA 和 JSON 属性上的等值匹配，插入和更新操作可以使用 MVA 和 JSON 属性，分布式索引支持通过 JSON API 执行更新和删除。
* [Percolate Queries](Searching/Percolate_query.md)
* 移除了对 32 位 docid 的支持，同时移除了所有转换/加载 32 位 docid 的旧索引的代码。
* [仅为特定字段应用形态学](https://github.com/manticoresoftware/manticore/issues/7)。新增索引指令 morphology_skip_fields，允许定义不应用形态学的字段列表。
* [expand_keywords 现在可以通过 OPTION 语句作为查询运行时指令设置](https://github.com/manticoresoftware/manticore/issues/8)

### Bugfixes
* [Commit 0cfa](https://github.com/manticoresoftware/manticore/commit/0cfae4c) 修复构建为调试版本时服务器崩溃（发行版可能存在未定义行为）的问题（在启用 rlp 时）。
* [Commit 3242](https://github.com/manticoresoftware/manticore/commit/324291e) 修复启用 progressive 选项时，RT 索引优化合并 kill-lists 顺序错误的问题。
* [Commit ac0e](https://github.com/manticoresoftware/manticore/commit/ac0efee) 修复了 Mac 系统上的小崩溃。
* 经过详尽的静态代码分析后，进行了许多小修复。
* 其他小的 Bug 修复。

### Upgrade
本版本更改了 master 和 agent 之间通信使用的内部协议。如果您在分布式环境运行多个 Manticoresearch 实例，请确保升级时先升级代理，然后再升级 master。

## Version 2.5.1, 2017年11月23日
### Features and improvements
* 基于 [HTTP API 协议](Connecting_to_the_server/HTTP.md) 的 JSON 查询。支持搜索、插入、更新、删除、替换操作。数据修改命令支持批量执行，目前有一些限制，如 MVA 和 JSON 属性不能用于插入、替换或更新。
* [RELOAD INDEXES](Data_creation_and_modification/Adding_data_from_external_storages/Rotating_a_table.md#RELOAD-TABLES) 命令。
* [FLUSH LOGS](Logging/Rotating_query_and_server_logs.md) 命令。
* [SHOW THREADS](Node_info_and_management/SHOW_THREADS.md) 可显示优化、轮换或刷新进度。
* GROUP N BY 能正确处理 MVA 属性。
* 黑洞代理在独立线程运行，不再影响 master 查询。
* 实现了索引的引用计数，避免旋转和高负载时出现停顿。
* 实现了 SHA1 哈希，目前尚未对外暴露。
* 修复了 FreeBSD、macOS 和 Alpine 上的编译问题。

### Bugfixes
* [Commit 9897](https://github.com/manticoresoftware/manticore/commit/989752b) 修复了使用块索引时的过滤回归。
* [Commit b1c3](https://github.com/manticoresoftware/manticore/commit/b1c3864) 将 PAGE_SIZE 重命名为 ARENA_PAGE_SIZE，以兼容 musl。
* [Commit f213](https://github.com/manticoresoftware/manticore/commit/f2133cc) 禁用了 cmake < 3.1.0 版本的 googletests。
* [Commit f30e](https://github.com/manticoresoftware/manticore/commit/0839de7) 修复了服务器重启时绑定套接字失败的问题。
* [Commit 0807](https://github.com/manticoresoftware/manticore/commit/0807240) 修复服务器关闭时崩溃的问题。
* [Commit 3e3a](https://github.com/manticoresoftware/manticore/commit/3e3acc3) 修复了 show threads 命令显示系统黑洞线程的问题。
* [Commit 262c](https://github.com/manticoresoftware/manticore/commit/262c3fe) 重构了 iconv 的配置检测，修复了在 FreeBSD 和 Darwin 平台上的构建问题。

## Version 2.4.1 GA, 2017年10月16日
### Features and improvements
* WHERE 子句中支持属性过滤器之间的 OR 运算符。
* 维护模式（ SET MAINTENANCE=1）。
* [CALL KEYWORDS](Searching/Autocomplete.md#CALL-KEYWORDS) 支持分布式索引。
* [分组使用 UTC 时间](Server_settings/Searchd.md#grouping_in_utc)。
* [query_log_mode](Server_settings/Searchd.md#query_log_mode) 用于自定义日志文件权限。
* 字段权重可以为零或负值。
* [max_query_time](Searching/Options.md#max_query_time) 现在可以影响全表扫描
* 添加了 [net_wait_tm](Server_settings/Searchd.md#net_wait_tm)、[net_throttle_accept](Server_settings/Searchd.md#net_throttle_accept) 和 [net_throttle_action](Server_settings/Searchd.md#net_throttle_action) 用于网络线程微调（当 workers=thread_pool 时）
* COUNT DISTINCT 可以用于分面搜索
* IN 可以用于 JSON 浮点数组
* 多查询优化不再被整数/浮点表达式破坏
* 使用多查询优化时，[SHOW META](Node_info_and_management/SHOW_META.md) 显示一个 `multiplier` 行

### 编译
Manticore Search 使用 cmake 构建，编译所需的最低 gcc 版本是 4.7.2。

### 文件夹和服务
* Manticore Search 以 `manticore` 用户身份运行。
* 默认数据文件夹现在是 `/var/lib/manticore/`。
* 默认日志文件夹现在是 `/var/log/manticore/`。
* 默认 pid 文件夹现在是 `/var/run/manticore/`。

### Bug 修复
* [Commit a58c](https://github.com/manticoresoftware/manticore/commit/a58c619) 修复了导致 java 连接器崩溃的 SHOW COLLATION 语句
* [Commit 631c](https://github.com/manticoresoftware/manticore/commit/631cf4e) 修复了处理分布式索引时的崩溃；为分布式索引哈希添加了锁；从 agent 中移除了移动和复制运算符
* [Commit 942b](https://github.com/manticoresoftware/manticore/commit/942bec0) 修复了因并行重连导致的处理分布式索引崩溃
* [Commit e5c1](https://github.com/manticoresoftware/manticore/commit/e5c1ed2) 修复了在服务器日志存储查询时崩溃处理程序崩溃的问题
* [Commit 4a4b](https://github.com/manticoresoftware/manticore/commit/4a4bda5) 修复了多查询中池化属性的崩溃
* [Commit 3873](https://github.com/manticoresoftware/manticore/commit/3873bfb) 通过防止索引页包含到核心文件中，缩小了核心文件大小
* [Commit 11e6](https://github.com/manticoresoftware/manticore/commit/11e6254) 修复了启动时指定无效 agent 导致的 searchd 崩溃
* [Commit 4ca6](https://github.com/manticoresoftware/manticore/commit/4ca6350) 修复了 indexer 在 sql_query_killlist 查询中报告错误
* [Commit 123a](https://github.com/manticoresoftware/manticore/commit/123a9f0) 修复了 fold_lemmas=1 与命中计数的问题
* [Commit cb99](https://github.com/manticoresoftware/manticore/commit/cb99164) 修复了 html_strip 不一致的行为
* [Commit e406](https://github.com/manticoresoftware/manticore/commit/e406761) 修复了优化实时索引时丢失新设置；修复了带同步选项的优化时锁泄露
* [Commit 86ae](https://github.com/manticoresoftware/manticore/commit/86aeb82) 修复了错误的多查询处理
* [Commit 2645](https://github.com/manticoresoftware/manticore/commit/2645230) 修复了结果集依赖多查询顺序的问题
* [Commit 7239](https://github.com/manticoresoftware/manticore/commit/72395d9) 修复了带有错误查询的多查询导致的服务器崩溃
* [Commit f353](https://github.com/manticoresoftware/manticore/commit/f353326) 修复了共享锁到独占锁的转换
* [Commit 3754](https://github.com/manticoresoftware/manticore/commit/3754785) 修复了无索引查询导致的服务器崩溃
* [Commit 29f3](https://github.com/manticoresoftware/manticore/commit/29f360e) 修复了服务器死锁

## 版本 2.3.3，2017年7月6日
* Manticore 品牌标识
