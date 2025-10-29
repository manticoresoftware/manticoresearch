# 更新日志

## 版本 13.13.0
**发布于**：2025年10月7日
 
### 推荐库
- 推荐的 [MCL](https://github.com/manticoresoftware/columnar) 版本：8.1.0
- 推荐的 [Buddy](Installation/Manticore_Buddy.md#Manticore-Buddy) 版本：3.35.1
 
如果您遵循 [官方安装指南](https://manticoresearch.com/install/)，则无需担心此问题。
 
 
由于配置文件已更新，**您可能会在 Linux 升级期间看到一个警告，询问是否保留您的版本或使用来自软件包的新版本**。如果您有自定义（非默认）配置，建议保留您的版本并将 `pid_file` 路径更新为 `/run/manticore/searchd.pid`。不过，即使您不更改路径，也应能正常工作。
 
### 新功能和改进
* 🆕 [v13.13.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.13.0) 添加了对带有 SI 块缓存的 MCL 8.1.0 的支持。
 
### Bug 修复
* 🪲 [v13.11.8](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.11.8) [问题 #3791](https://github.com/manticoresoftware/manticoresearch/issues/3791) 修复了检查和调用已触发计时器之间的竞态条件。
* 🪲 [v13.11.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.11.7) [问题 #1045](https://github.com/manticoresoftware/manticoresearch/issues/1045) 通过将配置中的过时路径 `/var/run/manticore` 替换为正确的 `/run/manticore`，修复了 RHEL 8 在 systemd 更新期间 systemctl 警告。由于配置文件已更新，升级期间可能会看到一个警告，询问是否保留您的版本或使用来自软件包的新版本。如果您有自定义（非默认）配置，建议保留您的版本并将 `pid_file` 路径更新为 `/run/manticore/searchd.pid`。
* 🪲 [v13.11.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.11.6) [PR #3766](https://github.com/manticoresoftware/manticoresearch/pull/3766) 添加了对 MCL 版本 8.0.6 的支持。
* 🪲 [v13.11.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.11.5) [PR #3767](https://github.com/manticoresoftware/manticoresearch/pull/3767) 改进了中文文档翻译并更新了子模块。
* 🪲 [v13.11.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.11.4) [PR #3765](https://github.com/manticoresoftware/manticoresearch/pull/3765) 修复了别名连接属性的处理。
* 🪲 [v13.11.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.11.3) [PR #3763](https://github.com/manticoresoftware/manticoresearch/pull/3763) 修复了字符串属性批量连接时可能发生的崩溃问题，并解决了过滤器有时无法与 LEFT JOIN 一起使用的问题。
* 🪲 [v13.11.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.11.2) [问题 #3065](https://github.com/manticoresoftware/manticoresearch/issues/3065) 修复了在启用 index_field_lengths 的列式表中插入数据时的崩溃。
* 🪲 [v13.11.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.11.1) [问题 #3751](https://github.com/manticoresoftware/manticoresearch/issues/3751) 修复了启用 embeddings 时删除文档导致的崩溃。

## 版本 13.11.1

### Bug 修复
* 🪲 [v13.11.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.11.1) [问题 #3751](https://github.com/manticoresoftware/manticoresearch/issues/3751) 修复了启用 embeddings 时删除文档导致的崩溃。

## 版本 13.11.0
**发布时间**：2025年9月13日

本次发布的主要亮点是[自动嵌入](Searching/KNN.md#Auto-Embeddings-%28Recommended%29)——一项使语义搜索如同使用 SQL 一样简单的新功能。
无需外部服务或复杂管道：只需插入文本并用自然语言搜索。

### 自动嵌入的功能

- **自动从文本生成嵌入**
- **自然语言查询**，理解语义而非仅仅关键词
- **支持多个模型**（OpenAI、Hugging Face、Voyage、Jina）
- **与 SQL 和 JSON API 无缝集成**

### 推荐库
- 推荐 [MCL](https://github.com/manticoresoftware/columnar) 版本：8.0.3
- 推荐 [Buddy](Installation/Manticore_Buddy.md#Manticore-Buddy) 版本：3.35.1

如果您遵循[官方安装指南](https://manticoresearch.com/install/)，则无需担心此问题。

推荐的 [MCL](https://github.com/manticoresoftware/columnar) 版本：8.0.1
推荐的 [Buddy](Installation/Manticore_Buddy.md#Manticore-Buddy) 版本：3.34.2
### 新功能和改进
* 🆕 [v13.11.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.11.0) [PR #3746](https://github.com/manticoresoftware/manticoresearch/pull/3746) 在 JSON 查询中新增了用于嵌入生成的 “query” 支持。
如果您遵循 [官方安装指南](https://manticoresearch.com/install/)，则无需担心此问题。
* 🆕 [v13.10.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.10.0) [问题 #3709](https://github.com/manticoresoftware/manticoresearch/issues/3709) manticore-server RPM 包不再拥有 `/run` 目录。
* 🆕 [v13.9.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.9.0) [PR #3716](https://github.com/manticoresoftware/manticoresearch/pull/3716) 配置中新增对 `boolean_simplify` 的支持。
* 🆕 [v13.8.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.8.0) [问题 #3253](https://github.com/manticoresoftware/manticoresearch/issues/3253) 安装了 sysctl 配置以增加 vm.max_map_count，适用于大数据集。

### Bug 修复
* 🪲 [v13.10.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.10.5) [ PR #3737](https://github.com/manticoresoftware/manticoresearch/pull/3737) 现在，scroll 选项可以正确返回具有大型 64 位 ID 的所有文档。
* 🪲 [v13.10.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.10.4) [ PR #3736](https://github.com/manticoresoftware/manticoresearch/pull/3736) 修复了使用带过滤树的 KNN 时崩溃的问题。
* 🪲 [v13.10.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.10.3) [ Issue #3520](https://github.com/manticoresoftware/manticoresearch/issues/3520) `/sql` 端点不再允许 SHOW VERSION 命令。
* 🪲 [v13.10.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.10.2) [ PR #3637](https://github.com/manticoresoftware/manticoresearch/pull/3637) 更新了 Windows 安装脚本。
* 🪲 [v13.10.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.10.1) 修正了 Linux 上的本地时区检测。
* 🪲 [v13.9.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.9.2) [ PR #3726](https://github.com/manticoresoftware/manticoresearch/pull/3726) 列式模式下重复的 ID 现在正确返回错误。
* 🪲 [v13.9.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.9.1) [ PR #3333](https://github.com/manticoresoftware/manticoresearch/pull/3333) 手册现在支持自动翻译。
* 🪲 [v13.8.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.8.6) [ PR #3715](https://github.com/manticoresoftware/manticoresearch/pull/3715) 修复了当批处理中所有文档被跳过时嵌入生成的问题。
* 🪲 [v13.8.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.8.5) [ PR #3711](https://github.com/manticoresoftware/manticoresearch/pull/3711) 添加了 Jina 和 Voyage 嵌入模型，以及其他自动嵌入相关的改进。
* 🪲 [v13.8.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.8.4) [ PR #3710](https://github.com/manticoresoftware/manticoresearch/pull/3710) 修复了多维度联合查询时的崩溃问题。
* 🪲 [v13.8.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.8.3) 修复了在_bulk端点使用多个语句的事务中，delete/update 提交未被计为错误的问题。
* 🪲 [v13.8.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.8.2) [ PR #3705](https://github.com/manticoresoftware/manticoresearch/pull/3705) 修复了按非列式字符串属性联接时的崩溃，并改进了错误报告。
* 🪲 [v13.8.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.8.1) [ PR #3704](https://github.com/manticoresoftware/manticoresearch/pull/3704) 修复了查询嵌入中当未指定模型时的崩溃；向主服务器-代理通信中添加了嵌入字符串；新增测试。
* 🪲 [v13.7.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.7.2) [ PR #Buddy#589](https://github.com/manticoresoftware/manticoresearch-buddy/pull/589) 移除了模糊搜索的默认 IDF 处理。
* 🪲 [v13.7.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.7.1) [ Issue #3454](https://github.com/manticoresoftware/manticoresearch/issues/3454) 修正了大型 64 位 ID 导致的滚动解码错误。
* 🪲 [v13.6.9](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.9) [ Issue #3674](https://github.com/manticoresoftware/manticoresearch/issues/3674) 修复了 JDBC+MySQL 驱动/连接池因 transaction_read_only 设置而导致的问题。
* 🪲 [v13.6.8](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.8) [ PR #3676](https://github.com/manticoresoftware/manticoresearch/pull/3676) 修复了嵌入模型返回空结果集时的崩溃。

## Version 13.6.7
**Released**: August 8th 2025

Recommended [MCL](https://github.com/manticoresoftware/columnar) version: 8.0.1  
Recommended [Buddy](Installation/Manticore_Buddy.md#Manticore-Buddy) version: 3.34.2  

If you follow the [official installation guide](https://manticoresearch.com/install/), you don't need to worry about this.
* ⚠️ [PR #3586](https://github.com/manticoresoftware/manticoresearch/pull/3586) 停止支持 Debian 10 (Buster)。Debian 10 已于 2024 年 6 月 30 日停止生命周期。建议用户升级到 Debian 11 (Bullseye) 或 Debian 12 (Bookworm)。
* ⚠️ [v13.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.0.0) 更新了 KNN 库 API，支持空的 [float_vector](https://manual.manticoresearch.com/Creating_a_table/Data_types#Float-vector) 值。此更新不更改数据格式，但提高了 Manticore Search / MCL API 版本。
* ⚠️ [v12.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/12.0.0) [PR #3516](https://github.com/manticoresoftware/manticoresearch/pull/3516) 修复了 KNN 索引训练和构建过程中源和目标行 ID 错误的 Bug。此更新不更改数据格式，但提高了 Manticore Search / MCL API 版本。

### New Features and Improvements
* 🆕 [v13.6.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.0) [ Issue #2226](https://github.com/manticoresoftware/manticoresearch/issues/2226) 支持在 PHRASE、PROXIMITY 和 QUORUM 操作符中显式使用“|”（或）。
* 🆕 [v13.5.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.5.0) [ PR #3591](https://github.com/manticoresoftware/manticoresearch/pull/3591) 查询中自动生成嵌入（进行中，尚未准备好用于生产）。
* 🆕 [v13.4.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.4.0) [ PR #3585](https://github.com/manticoresoftware/manticoresearch/pull/3585) 修正了优先使用 buddy_path 配置中的 buddy 线程数的逻辑，而非守护进程的值。
* 🆕 [v13.3.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.3.0) [ PR #3577](https://github.com/manticoresoftware/manticoresearch/pull/3577) 支持使用本地分布式表进行连接。
* 🆕 [#3647](https://github.com/manticoresoftware/manticoresearch/issues/3647) 增加对 Debian 13 “Trixie” 的支持。

### Bug Fixes
* 🪲 [v13.6.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.7) [ Issue #3524](https://github.com/manticoresoftware/manticoresearch/issues/3524) 修复了行存储中保存生成嵌入的问题。
* 🪲 [v13.6.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.6) [ Issue #3563](https://github.com/manticoresoftware/manticoresearch/issues/3563) 修复了 Sequel Ace 和其他集成因“未知 sysvar”错误导致失败的问题。
* 🪲 [v13.6.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.5) [ Issue #3467](https://github.com/manticoresoftware/manticoresearch/issues/3467) 修复了由于“未知 sysvar”错误导致 DBeaver 和其他集成失败的问题。
* 🪲 [v13.6.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.4) [ Issue #3524](https://github.com/manticoresoftware/manticoresearch/issues/3524) 修复了多字段嵌入连接的问题；同时修复了从查询生成嵌入的问题。
* 🪲 [v13.6.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.3) [ Issue #3641](https://github.com/manticoresoftware/manticoresearch/issues/3641) 修复了 13.6.0 版本中一个短语丢失除第一个之外所有括号内关键词的 bug。
* 🪲 [v13.6.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.2) 修复了 transform_phrase 中的内存泄漏。
* 🪲 [v13.6.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.1) 修复了 13.6.0 版本中的内存泄漏。
* 🪲 [v13.5.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.5.2) [ Issue #3651](https://github.com/manticoresoftware/manticoresearch/issues/3651) 修复了与全文搜索模糊测试相关的更多问题。
* 🪲 [v13.5.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.5.1) [ Issue #3560](https://github.com/manticoresoftware/manticoresearch/issues/3560) 修复了使用 KNN 数据时 OPTIMIZE TABLE 可能无限挂起的问题。
* 🪲 [v13.4.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.4.2) [ Issue #2544](https://github.com/manticoresoftware/manticoresearch/issues/2544) 修复了添加 float_vector 列可能损坏索引的问题。
* 🪲 [v13.4.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.4.1) [ Issue #3651](https://github.com/manticoresoftware/manticoresearch/issues/3651) 增加了全文解析的模糊测试，并修复了测试中发现的多个问题。
* 🪲 [v13.3.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.3.1) [ Issue #3583](https://github.com/manticoresoftware/manticoresearch/issues/3583) 修复了使用复杂布尔过滤器进行高亮显示时导致的崩溃。
* 🪲 [v13.2.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.7) [ Issue #3481](https://github.com/manticoresoftware/manticoresearch/issues/3481) 修复了在同时使用 HTTP 更新、分布式表和错误复制集群时导致的崩溃。
* 🪲 [v13.2.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.6) [ PR #3567](https://github.com/manticoresoftware/manticoresearch/pull/3567) 更新了 manticore-backup 依赖到 1.9.6 版本。
* 🪲 [v13.2.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.5) [ PR #3565](https://github.com/manticoresoftware/manticoresearch/pull/3565) 修复了 CI 设置以改进 Docker 镜像兼容性。
* 🪲 [v13.2.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.4) 修复了长令牌的处理。一些特殊令牌（如正则表达式模式）可能创建过长的词语，现已在使用前缩短。

## Version 13.2.3
**Released**: 2025年7月8日

### 重大变更
* ⚠️ [v13.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.0.0) 更新了 KNN 库 API，支持空的 [float_vector](https://manual.manticoresearch.com/Creating_a_table/Data_types#Float-vector) 值。此更新不更改数据格式，但提高了 Manticore Search / MCL API 版本。
* ⚠️ [v12.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/12.0.0) [ PR #3516](https://github.com/manticoresoftware/manticoresearch/pull/3516) 修复了 KNN 索引训练和构建过程中源和目标行 ID 不正确的 bug。此更新不更改数据格式，但提高了 Manticore Search / MCL API 版本。
* ⚠️ [v11.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.0.0) 新增支持 KNN 向量搜索的新特性，如量化、重新评分和过采样。此版本更改了 KNN 数据格式和 [KNN_DIST() SQL 语法](https://manual.manticoresearch.com/Searching/KNN?client=SQL#KNN-vector-search)。新版本可读取旧数据，而旧版本无法读取新格式。

### 新特性和改进
* 🆕 [v13.2.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.0) [ PR #3549](https://github.com/manticoresoftware/manticoresearch/issues/3525) 修复了导致某些 mysqldump 版本不兼容的 `@@collation_database` 问题。
* 🆕 [v13.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.1.0) [ Issue #3489](https://github.com/manticoresoftware/manticoresearch/issues/3489) 修复了模糊搜索中阻止解析某些 SQL 查询的 bug。
* 🆕 [v12.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/12.1.0) [ Issue #3426](https://github.com/manticoresoftware/manticoresearch/issues/3426) 增加了对 RHEL 10 操作系统的支持。
* 🆕 [v11.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.1.0) 增加了对 [KNN 搜索](https://manual.manticoresearch.com/Searching/KNN#KNN-vector-search) 中空 float 向量的支持。
* 🆕 [v10.2.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.0) [ Issue #3252](https://github.com/manticoresoftware/manticoresearch/issues/3252) [log_level](https://manual.manticoresearch.com/Server_settings/Setting_variables_online#SET) 现在也调整 Buddy 的日志详细程度。

### Bug 修复
* 🪲 [v13.2.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.3) [ PR #3556](https://github.com/manticoresoftware/manticoresearch/pull/3556) 修复了 JSON 查询中“oversampling”选项解析的问题。
* 🪲 [v13.2.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.2) 修复了 Linux 和 FreeBSD 上的崩溃日志问题，去除了对 Boost stacktrace 的使用。
* 🪲 [v13.2.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.1) [ Issue #3298](https://github.com/manticoresoftware/manticoresearch/issues/3298) 修复了在容器内运行时的崩溃日志记录问题
* 🪲 [v12.0.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/12.0.2)  通过以微秒为单位计数，提高了每表统计数据的准确性
* 🪲 [v12.0.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/12.0.1) [ PR #3522](https://github.com/manticoresoftware/manticoresearch/pull/3522) 修复了在联合查询中按 MVA 分面时的崩溃
* 🪲 [v11.0.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.0.3) [ Issue #3502](https://github.com/manticoresoftware/manticoresearch/issues/3502) 修复了与向量搜索量化相关的崩溃
* 🪲 [v11.0.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.0.2) [ Issue #3493](https://github.com/manticoresoftware/manticoresearch/issues/3493) 修改 `SHOW THREADS` 命令以显示整数形式的 CPU 利用率
* 🪲 [v11.0.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.0.1)  修正了列式存储和二级库的路径
* 🪲 [v10.2.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.7)  添加了对 MCL 5.0.5 的支持，包括修复嵌入库文件名的问题
* 🪲 [v10.2.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.6) [ Issue #3469](https://github.com/manticoresoftware/manticoresearch/issues/3469) 针对问题 #3469 应用另一项修复
* 🪲 [v10.2.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.4) [ Issue #3469](https://github.com/manticoresoftware/manticoresearch/issues/3469) 修复了带布尔查询的 HTTP 请求返回空结果的问题
* 🪲 [v10.2.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.5)  修改了嵌入库的默认文件名，并添加了 KNN 向量中 'from' 字段的检查
* 🪲 [v10.2.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.3) [ PR #3464](https://github.com/manticoresoftware/manticoresearch/pull/3464) 将 Buddy 升级到 3.30.2 版本，其中包含关于内存使用和错误日志的 [PR #565](https://github.com/manticoresoftware/manticoresearch-buddy/pull/565)
* 🪲 [v10.2.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.2) [ Issue #3470](https://github.com/manticoresoftware/manticoresearch/pull/3470) 修复了 JSON 字符串过滤器、空过滤器及 JOIN 查询中的排序问题
* 🪲 [v10.2.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.1)  修复了 `dist/test_kit_docker_build.sh` 中导致 Buddy 提交缺失的错误
* 🪲 [v10.1.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.4)  修复了在联合查询中按 MVA 分组时的崩溃
* 🪲 [v10.1.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.3) [ Issue #3434](https://github.com/manticoresoftware/manticoresearch/issues/3434) 修复了过滤空字符串的错误
* 🪲 [v10.1.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.2) [ PR #3452](https://github.com/manticoresoftware/manticoresearch/pull/3452) 升级 Buddy 到 3.29.7 版本，解决了 [Buddy #507 - 多查询中的模糊搜索错误](https://github.com/manticoresoftware/manticoresearch-buddy/issues/507) 和 [Buddy #561 - 指标速率修复](https://github.com/manticoresoftware/manticoresearch-buddy/pull/561)，该版本为 [Helm 发行版 10.1.0](https://github.com/manticoresoftware/manticoresearch-helm/releases/tag/manticoresearch-10.1.0) 所必需
* 🪲 [v10.1.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.1)  升级 Buddy 到 3.29.4 版本，解决了 [#3388 - “未定义的数组键 'Field'”](https://github.com/manticoresoftware/manticoresearch/issues/3388) 和 [Buddy #547 - layouts='ru' 可能无法工作](https://github.com/manticoresoftware/manticoresearch-buddy/issues/547)

## 版本 10.1.0
**发布于**：2025年6月9日

该版本为更新版本，包含新功能、一项破坏性变更以及大量稳定性改进和错误修复。修改重点为增强监控能力、改进搜索功能以及修复影响系统稳定性和性能的多个关键问题。

**从版本 10.1.0 起，不再支持 CentOS 7。建议用户升级到受支持的操作系统。**

### 破坏性变更
* ⚠️ [v10.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.0.0) [ Issue #540](https://github.com/manticoresoftware/manticoresearch-buddy/issues/540) 破坏性变更：将 [模糊搜索](Searching/Spell_correction.md#Fuzzy-Search) 的默认 `layouts=''`

### 新功能和改进
* 🆕 [v10.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.0) [ Issue #537](https://github.com/manticoresoftware/manticoresearch-buddy/issues/537) 添加内置的 [Prometheus 导出器](Node_info_and_management/Node_status.md#Prometheus-Exporter)
* 🆕 [v9.8.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.8.0) [ Issue #3409](https://github.com/manticoresoftware/manticoresearch/issues/3409) 添加了 [ALTER TABLE tbl REBUILD KNN](Updating_table_schema_and_settings.md#Rebuilding-a-KNN-index) 功能
* 🆕 [v9.7.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.7.0) [ Issue #1778](https://github.com/manticoresoftware/manticoresearch/issues/1778) 添加了自动生成嵌入功能（尚未正式发布，该功能代码已在主分支，但仍需更多测试）
* 🆕 [v9.6.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.6.0)  提升了 KNN API 版本以支持自动嵌入
* 🆕 [v9.5.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.0) [ Issue #1894](https://github.com/manticoresoftware/manticoresearch/issues/1894) 改进了集群恢复：定期保存 `seqno`，加快崩溃后节点重启速度
* 🆕 [v9.4.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.4.0) [ Issue #2400](https://github.com/manticoresoftware/manticoresearch/issues/2400) 增加对最新 [Logstash](Integration/Logstash.md#Integration-with-Logstash) 和 Beats 的支持

### Bug 修复
* 🪲 [v10.0.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.0.1) 修正了词形处理：用户定义的形式现在覆盖自动生成的形式；增加了测试用例测试22
* 🪲 [v9.8.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.8.2) 修复：再次更新 deps.txt 以包含关于嵌入库的 MCL 打包修复
* 🪲 [v9.8.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.8.1) 修复：更新 deps.txt，修正 MCL 和嵌入库的打包问题
* 🪲 [v9.7.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.7.3) [ Issue #3306](https://github.com/manticoresoftware/manticoresearch/issues/3306) 修正索引时的信号 11 崩溃问题
* 🪲 [v9.7.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.7.2) [ Issue #3109](https://github.com/manticoresoftware/manticoresearch/issues/3109) 修复非存在的 `@@variables` 始终返回 0 的问题
* 🪲 [v9.7.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.7.1) [ Issue #3377](https://github.com/manticoresoftware/manticoresearch/issues/3377) 修正与 [remove_repeats()](Functions/Searching_and_ranking_functions.md#REMOVE_REPEATS%28%29) 相关的崩溃问题
* 🪲 [v9.6.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.6.3) [ PR #3411](https://github.com/manticoresoftware/manticoresearch/pull/3411) 修复：使用动态版本检测来获取遥测指标
* 🪲 [v9.6.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.6.2) 修复：[SHOW VERSION](Node_info_and_management/SHOW_VERSION.md#SHOW-VERSION) 输出的小问题
* 🪲 [v9.6.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.6.1) 修复：创建带有 KNN 属性但未指定模型的表时崩溃的问题
* 🪲 [v9.5.16](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.16) [ Issue #3342](https://github.com/manticoresoftware/manticoresearch/issues/3342) 修复 `SELECT ... FUZZY=0` 未总是禁用模糊搜索的问题
* 🪲 [v9.5.15](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.15) [ PR #3397](https://github.com/manticoresoftware/manticoresearch/pull/3397) 增加对 [MCL](https://github.com/manticoresoftware/columnar) 4.2.2 的支持；修正旧存储格式的错误
* 🪲 [v9.5.14](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.14) [ Issue #3392](https://github.com/manticoresoftware/manticoresearch/issues/3392) 修正 HTTP JSON 回复中字符串处理错误
* 🪲 [v9.5.13](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.13) [ Issue #3356](https://github.com/manticoresoftware/manticoresearch/issues/3356) 修复复杂全文查询中（公用子词项）导致的崩溃
* 🪲 [v9.5.12](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.12) 修正磁盘块自动刷新错误消息中的拼写错误
* 🪲 [v9.5.11](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.11) [ Issue #3195](https://github.com/manticoresoftware/manticoresearch/issues/3195) 改进 [自动磁盘块刷新](Securing_and_compacting_a_table/Flushing_RAM_chunk_to_disk.md#Flushing-RAM-chunk-to-disk)：在优化运行时跳过保存
* 🪲 [v9.5.10](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.10) [ Issue #3313](https://github.com/manticoresoftware/manticoresearch/issues/3313) 修正 RT 表中所有磁盘块的重复 ID 检查，使用 [indextool](Miscellaneous_tools.md#indextool)
* 🪲 [v9.5.9](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.9) [ Issue #3132](https://github.com/manticoresoftware/manticoresearch/issues/3132) 增加 JSON API 中对 `_random` 排序的支持
* 🪲 [v9.5.8](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.8) [ Issue #3382](https://github.com/manticoresoftware/manticoresearch/issues/3382) 修复无法通过 JSON HTTP API 使用 uint64 文档 ID 的问题
* 🪲 [v9.5.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.7) [ Issue #3385](https://github.com/manticoresoftware/manticoresearch/issues/3385) 修正根据 `id != value` 过滤时结果错误的问题
* 🪲 [v9.5.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.6) [ PR #538](https://github.com/manticoresoftware/manticoresearch-buddy/pull/538) 修复某些情况下模糊匹配的严重漏洞
* 🪲 [v9.5.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.5) [ Issue #3199](https://github.com/manticoresoftware/manticoresearch/issues/3199) 修正 Buddy HTTP 查询参数中的空格解码问题（如 `%20` 和 `+`）
* 🪲 [v9.5.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.4) [ Issue #3133](https://github.com/manticoresoftware/manticoresearch/issues/3133) 修复 JSON 字段索引中的错误排序问题
* 🪲 [v9.5.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.3) [ Issue #3091](https://github.com/manticoresoftware/manticoresearch/issues/3091) 修正 SQL 与 JSON API 中分隔符搜索结果不一致的问题
* 🪲 [v9.5.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.2) [ Issue #2819](https://github.com/manticoresoftware/manticoresearch/issues/2819) 性能改进：为分布式表替换 `DELETE FROM` 为 `TRUNCATE`
* 🪲 [v9.5.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.1) [ Issue #3080](https://github.com/manticoresoftware/manticoresearch/issues/3080) 修正使用 JSON 属性过滤别名 `geodist()` 时的崩溃问题

## 版本 9.3.2
发布日期：2025年5月2日

本次发布包括多个 Bug 修复和稳定性改进，更好的表使用跟踪，以及内存和资源管理的增强。

❤️ 特别感谢 [@cho-m](https://github.com/cho-m) 解决了 Boost 1.88.0 的构建兼容性问题，感谢 [@benwills](https://github.com/benwills) 改进了有关 `stored_only_fields` 的文档。

* 🪲 [v9.3.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.3.2) 修复了“显示线程”列将 CPU 活动显示为浮点数而非字符串的问题；还修复了由错误数据类型引起的 PyMySQL 结果集解析错误。
* 🪲 [v9.3.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.3.1) [ Issue #3343](https://github.com/manticoresoftware/manticoresearch/issues/3343) 修复了优化过程被中断时遗留的 `tmp.spidx` 文件。
* 🆕 [v9.3.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.3.0) [ PR #3337](https://github.com/manticoresoftware/manticoresearch/pull/3337) 新增了每表命令计数器和详细的表使用统计。
* 🪲 [v9.2.39](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.39) [ Issue #3236](https://github.com/manticoresoftware/manticoresearch/issues/3236) 修复：通过移除复杂的块更新防止表受损。串行工作线程中的等待函数使用破坏了串行处理，可能导致表损坏。
	重新实现了自动刷新。移除了外部轮询队列以避免不必要的表锁。新增“小表”条件：如果文档数低于“小表限制”（8192）且未使用二级索引（SI），则跳过刷新。

* 🪲 [v9.2.38](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.38) 修复：跳过为使用 `ALL`/`ANY` 作用于字符串列表的过滤器创建二级索引（SI），不影响 JSON 属性。
* 🪲 [v9.2.37](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.37) [ Issue #2898](https://github.com/manticoresoftware/manticoresearch/issues/2898) 增加了系统表的反引号支持。
* 🪲 [v9.2.36](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.36) 修复：在旧代码中为集群操作使用了占位符。解析器中现在明确区分了表名和集群名的字段。
* 🪲 [v9.2.35](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.35) 修复：取消单个 `'` 引用时崩溃。
* 🪲 [v9.2.34](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.34) [ Issue #3090](https://github.com/manticoresoftware/manticoresearch/issues/3090) 修复：大文档 ID 处理（之前可能找不到它们）。
* 🪲 [v9.2.33](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.33) 修复：位向量大小使用无符号整数。
* 🪲 [v9.2.32](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.32) 修复：减少合并期间的峰值内存使用。docid 到 rowid 查找现在每个文档使用 12 字节而非 16 字节。例如：20 亿文档使用 24 GB 内存，而非 36 GB。
* 🪲 [v9.2.31](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.31) [ Issue #3238](https://github.com/manticoresoftware/manticoresearch/issues/3238) 修复：大型实时表中 `COUNT(*)` 统计值错误。
* 🪲 [v9.2.30](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.30) 修复：零值字符串属性时的未定义行为。
* 🪲 [v9.2.29](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.29) 小修正：改进了警告文本。
* 🪲 [v9.2.28](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.28) [ Issue #3290](https://github.com/manticoresoftware/manticoresearch/issues/3290) 改进：增强了 `indextool --buildidf`。
* 🪲 [v9.2.27](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.27) [ Issue #3032](https://github.com/manticoresoftware/manticoresearch/issues/3032) 通过 Kafka 集成，现在可以为特定 Kafka 分区创建数据源。
* 🪲 [v9.2.26](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.26) [ Issue #3301](https://github.com/manticoresoftware/manticoresearch/issues/3301) 修复：针对 `id` 的 `ORDER BY` 和 `WHERE` 可能导致内存溢出（OOM）错误。
* 🪲 [v9.2.25](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.25) [ Issue #3171](https://github.com/manticoresoftware/manticoresearch/issues/3171) 修复：在具有多个磁盘块的 RT 表上使用多个 JSON 属性的 grouper 造成的段错误崩溃。
* 🪲 [v9.2.24](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.24) [ Issue #3246](https://github.com/manticoresoftware/manticoresearch/issues/3246) 修复：`WHERE string ANY(...)` 查询在 RAM 块刷新后失败。
* 🪲 [v9.2.23](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.23) [ PR #518](https://github.com/manticoresoftware/manticoresearch-buddy/pull/518) 小幅改进了自动分片语法。
* 🪲 [v9.2.22](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.22) [ Issue #2763](https://github.com/manticoresoftware/manticoresearch/issues/2763) 修复：使用 `ALTER TABLE` 时未加载全局 idf 文件。
* 🪲 [v9.2.21](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.21) 修复：全局 idf 文件可能较大。现在更早释放表以避免占用不必要的资源。
* 🪲 [v9.2.20](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.20) [ PR #3277](https://github.com/manticoresoftware/manticoresearch/pull/3277) 改进：更好的分片选项验证。

* 🪲 [v9.2.19](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.19) [ PR #3275](https://github.com/manticoresoftware/manticoresearch/pull/3275) 修复：兼容 Boost 1.88.0 的构建。
* 🪲 [v9.2.18](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.18) [ Issue #3228](https://github.com/manticoresoftware/manticoresearch/issues/3228) 修复：分布式表创建时崩溃（无效指针问题）。

* 🪲 [v9.2.17](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.17) [ PR #3272](https://github.com/manticoresoftware/manticoresearch/pull/3272) 修复：多行模糊`FACET`问题。
* 🪲 [v9.2.16](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.16) [ Issue #3063](https://github.com/manticoresoftware/manticoresearch/issues/3063) 修复：使用`GEODIST`函数时距离计算的错误。
* 🪲 [v9.2.15](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.15) [ Issue #3027](https://github.com/manticoresoftware/manticoresearch/issues/3027) 小幅改进：支持Elastic的`query_string`过滤器格式。

## 版本 9.2.14
发布日期：2025年3月28日

### 小幅改进
* [提交 ](https://github.com/manticoresoftware/manticoresearch/commit/fe9473dc3f4fcfa0eaae748b538fe853f4ce078b) 实现了`--mockstack`标志以计算递归操作栈需求。新的`--mockstack`模式分析并报告递归表达式求值、模式匹配操作、过滤器处理所需的栈大小。计算出的栈需求输出到控制台，以便调试和优化。
* [Issue #3058](https://github.com/manticoresoftware/manticoresearch/pull/3058) 默认启用[boolean_simplify](Searching/Options.md#boolean_simplify)。
* [Issue #3172](https://github.com/manticoresoftware/manticoresearch/issues/3172) 新增配置选项：`searchd.kibana_version_string`，在使用特定版本的Kibana或OpenSearch Dashboards，需指明特定Elasticsearch 版本时很有用。
* [Issue #3211](https://github.com/manticoresoftware/manticoresearch/issues/3211) 修复[CALL SUGGEST](Searching/Spell_correction.md#CALL-QSUGGEST,-CALL-SUGGEST)以支持2字符单词。
* [Issue #490](https://github.com/manticoresoftware/manticoresearch-buddy/issues/490) 改进[fuzzy search](Searching/Spell_correction.md#Fuzzy-Search)：之前当存在其他匹配文档时，有时无法查到搜索“def ghi”时的“defghi”。
* ⚠️ 破坏性变更 [Issue #3165](https://github.com/manticoresoftware/manticoresearch/issues/3165) 在部分HTTP JSON响应中将`_id`改为`id`以保持一致。请确保相应更新您的应用。
* ⚠️ 破坏性变更 [Issue #3186](https://github.com/manticoresoftware/manticoresearch/issues/3186) 新增集群加入时对`server_id`的检查，确保每个节点有唯一ID。若加入节点`server_id`与现有节点重复，`JOIN CLUSTER`操作会返回重复[server_id](Server_settings/Searchd.md#server_id)的错误。为解决此问题，确保复制集群中每个节点都有唯一[server_id](Server_settings/Searchd.md#server_id)。您可以在配置文件“searchd”部分将默认[server_id](Server_settings/Searchd.md#server_id)更改为唯一值后再尝试加入集群。此更改更新了复制协议。如果您运行复制集群，请执行：
  - 首先，干净关闭所有节点
  - 然后，使用Linux工具`manticore_new_cluster`，带`--new-cluster`参数启动最后停下的节点。
  - 更多详细信息参见[重启集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster)。

### Bug修复
* [提交 6fda](https://github.com/manticoresoftware/manticoresearch/commit/6fdad3923dd6953f4b781943eed5ec5c28b7f808) 修复因等待后丢失调度器导致的崩溃；现在诸如`serializer`等特定调度器可被正确恢复。
* [提交 c333](https://github.com/manticoresoftware/manticoresearch/commit/c333b277d4a504de5fe298e9ab570a9f9ea31e0a) 修复右连接表权重无法在`ORDER BY`子句中使用的错误。
* [Issue #2644](https://github.com/manticoresoftware/manticoresearch/issues/2644) gcc 14.2.0：修复`const knn::DocDist_t*&`导致的`lower_bound`调用错误。❤️ 感谢[@Azq2](https://github.com/Azq2)的PR。
* [Issue #3018](https://github.com/manticoresoftware/manticoresearch/issues/3018) 修复自动架构插入时对大写表名处理的问题。
* [Issue #3119](https://github.com/manticoresoftware/manticoresearch/issues/3119) 修复解码无效base64输入时导致的崩溃。
* [Issue #3121](https://github.com/manticoresoftware/manticoresearch/issues/3121) 修复与`ALTER`相关的两个KNN索引问题：浮点向量保持原始维度，KNN索引正确生成。
* [Issue #3123](https://github.com/manticoresoftware/manticoresearch/issues/3123) 修复针对空JSON列构建二级索引时的崩溃。
* [Issue #3138](https://github.com/manticoresoftware/manticoresearch/issues/3138) 修复重复条目导致的崩溃。
* [Issue #3151](https://github.com/manticoresoftware/manticoresearch/issues/3151) 修复`fuzzy=1`选项无法与`ranker`或`field_weights`配合使用的问题。
* [Issue #3163](https://github.com/manticoresoftware/manticoresearch/issues/3163) 修复`SET GLOBAL timezone`无效的Bug。
* [Issue #3181](https://github.com/manticoresoftware/manticoresearch/issues/3181) 修复使用大于2^63的ID时文本字段值丢失的问题。
* [Issue #3189](https://github.com/manticoresoftware/manticoresearch/issues/3189) 修复`UPDATE`语句现在正确遵守`query_log_min_msec`设置。
* [Issue #3247](https://github.com/manticoresoftware/manticoresearch/issues/3247) 修复保存实时磁盘块时的竞态条件，防止`JOIN CLUSTER`失败。


## 版本 7.4.6
发布日期：2025年2月28日

### 重大变更
* [Issue #832](https://github.com/manticoresoftware/manticoresearch/issues/832) 集成[Kibana](Integration/Kibana.md)以实现更简便高效的数据可视化。

### 小幅改进
* [Issue #1727](https://github.com/manticoresoftware/manticoresearch/issues/1727) 修复了 arm64 和 x86_64 之间的浮点精度差异。
* [Issue #2995](https://github.com/manticoresoftware/manticoresearch/issues/2995) 实现了 join 批处理的性能优化。
* [Issue #3039](https://github.com/manticoresoftware/manticoresearch/issues/3039) 实现了直方图中 EstimateValues 的性能优化。
* [Issue #3099](https://github.com/manticoresoftware/manticoresearch/issues/3099) 添加了对 Boost 1.87.0 的支持。❤️ 感谢 [@cho-m](https://github.com/cho-m) 的 PR。
* [Issue #77](https://github.com/manticoresoftware/columnar/issues/77) 优化了创建多值过滤器时的块数据重用；向属性元数据增加了最小/最大值；基于最小/最大值实现了过滤值的预过滤。

### Bugfixes
* [Commit 73ac](https://github.com/manticoresoftware/manticoresearch/commit/73ac22f358a7a0e734b332c3943c86e6294c06c3) 修复了连接查询中当同时使用左表和右表属性时表达式的处理；修复了右表的 index_weights 选项。
* [Issue #2915](https://github.com/manticoresoftware/manticoresearch/issues/2915) 在 `SELECT ... JOIN` 查询中使用 `avg()` 可能导致结果错误；现已修复。
* [Issue #2996](https://github.com/manticoresoftware/manticoresearch/issues/2996) 修复了由于启用 join 批处理时隐式截断导致的错误结果集。
* [Issue #3031](https://github.com/manticoresoftware/manticoresearch/issues/3031) 修复了守护进程关闭时当有活动块合并进行中导致的崩溃。
* [Issue #3037](http://github.com/manticoresoftware/manticoresearch/issues/3037) 修复了 `IN(...)` 产生错误结果的问题。
* [Issue #3038](https://github.com/manticoresoftware/manticoresearch/issues/3038) 在版本7.0.0中设置 `max_iops` / `max_iosize` 可能降低索引性能；现已修复。
* [Issue #3042](https://github.com/manticoresoftware/manticoresearch/issues/3042) 修复了连接查询缓存中的内存泄漏。
* [Issue #3052](https://github.com/manticoresoftware/manticoresearch/issues/3052) 修复了连接 JSON 查询中查询选项的处理。
* [Issue #3054](https://github.com/manticoresoftware/manticoresearch/issues/3054) 修复了 ATTACH TABLE 命令的问题。
* [Issue #3079](https://github.com/manticoresoftware/manticoresearch/issues/3079) 修正了错误信息中的不一致问题。
* [Issue #3087](https://github.com/manticoresoftware/manticoresearch/issues/3087) 按表设置 `diskchunk_flush_write_timeout=-1` 未能禁用索引刷新；现已修复。
* [Issue #3088](https://github.com/manticoresoftware/manticoresearch/issues/3088) 解决了批量替换大 ID 后的重复条目问题。
* [Issue #3126](https://github.com/manticoresoftware/manticoresearch/issues/3126) 修复了带有单个 `NOT` 操作符和表达式排序器的全文查询导致守护进程崩溃的问题。
* [Issue #3128](https://github.com/manticoresoftware/manticoresearch/pull/3128) 修复了 CJSON 库中的潜在漏洞。❤️ 感谢 [@tabudz](https://github.com/tabudz) 的 PR。

## Version 7.0.0
发布于：2025年1月30日

### 主要变更
* [Issue #1497](https://github.com/manticoresoftware/manticoresearch/issues/1497) 新增[模糊搜索](Searching/Spell_correction.md#Fuzzy-Search)和[自动补全](Searching/Autocomplete.md#CALL-AUTOCOMPLETE)功能，提升搜索便捷性。
* [Issue #1500](https://github.com/manticoresoftware/manticoresearch/issues/1500) [Kafka 集成](Integration/Kafka.md#Syncing-from-Kafka)。
* [Issue #1928](https://github.com/manticoresoftware/manticoresearch/issues/1928) 引入了[JSON 的二级索引](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#json_secondary_indexes)。
* [Issue #2361](https://github.com/manticoresoftware/manticoresearch/issues/2361) 更新和更新期间的搜索不再被块合并阻塞。
* [Issue #2787](https://github.com/manticoresoftware/manticoresearch/issues/2787) 实时表的自动[磁盘块刷新](Server_settings/Searchd.md#diskchunk_flush_write_timeout)，提升性能；现在我们会自动将内存块刷新到磁盘块，避免因内存块缺乏优化导致的性能问题，这些问题有时会根据块大小导致不稳定。
* [Issue #2811](https://github.com/manticoresoftware/manticoresearch/issues/2811) 新增[滚动](Searching/Pagination.md#Scroll-Search-Option)选项，实现更便捷的分页。
* [Issue #931](https://github.com/manticoresoftware/manticoresearch/issues/931) 集成[结巴分词](https://github.com/fxsjy/jieba)，提升[中文分词](Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md)效果。

### 次要变更
* ⚠️ 破坏性变更 [Issue #1111](https://github.com/manticoresoftware/manticoresearch/issues/1111) 修复了 RT 表中 `global_idf` 的支持，需重新创建表。
* ⚠️ 破坏性变更 [Issue #2103](https://github.com/manticoresoftware/manticoresearch/issues/2103) 从内部 `cjk` 字符集移除了泰文字符。请相应更新字符集定义：如果使用了 `cjk,non_cjk` 并且泰文字符重要，请更改为 `cjk,thai,non_cjk`，或者使用 `cont,non_cjk`，其中 `cont` 是对所有连续书写语言（即 `cjk` + `thai`）的新称呼。修改现有表请参阅 [ALTER TABLE](Updating_table_schema_and_settings.md#Updating-table-FT-settings-in-RT-mode)。
* ⚠️ 破坏性变更 [Issue #2468](https://github.com/manticoresoftware/manticoresearch/issues/2468) [CALL SUGGEST / QSUGGEST](Searching/Spell_correction.md#CALL-QSUGGEST,-CALL-SUGGEST) 现在兼容分布式表。此更改提高了主/代理协议版本。如果您在分布式环境中运行 Manticore Search 多实例，请确保先升级代理，再升级主节点。
* ⚠️ 重大变更 [Issue #2889](https://github.com/manticoresoftware/manticoresearch/issues/2889) 将 PQ [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META-for-PQ-tables) 中的列名从 `Name` 改为 `Variable name`。
* ⚠️ 重大变更 [Issue #879](https://github.com/manticoresoftware/manticoresearch/issues/879) 引入了 [每表 binlog](Logging/Binary_logging.md#Per-table-binary-logging-configuration)，新增选项：[binlog_common](Logging/Binary_logging.md#Binary-logging-strategies)、[binlog](Logging/Binary_logging.md#Per-table-binary-logging-configuration)，用于 `create table` / `alter table`。升级到新版本前须先干净关闭 Manticore 实例。
* ⚠️ 重大变更 [Issue #1789](https://github.com/manticoresoftware/manticoresearch/issues/1789) 修正节点以错误复制协议版本加入集群时的错误信息。该变更更新了复制协议。运行复制集群时，请：
  - 首先，干净停止所有节点
  - 然后，使用 Linux 上的工具 `manticore_new_cluster`，以 `--new-cluster` 参数启动最后停止的节点。
  - 更多详情请参考关于 [重启集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster) 的说明。
* ⚠️ 重大变更 [Issue #2308](https://github.com/manticoresoftware/manticoresearch/issues/2308) 在 [`ALTER CLUSTER ADD` 和 `DROP`](Creating_a_cluster/Setting_up_replication/Adding_and_removing_a_table_from_a_replication_cluster.md#Adding-and-removing-a-table-from-a-replication-cluster)中新增多表支持。此变更也涉及复制协议。请参照前面章节操作此更新。
* [Issue #2997](https://github.com/manticoresoftware/manticoresearch/issues/2997) 修复了 Macos 上的 dlopen 问题。
* [Commit 4954](https://github.com/manticoresoftware/manticoresearch/commit/4954b5de7341a29902a9b8fbb9a040f7942c77c4) 修改了带有 KNN 索引表的 OPTIMIZE TABLE 默认截止值，以提升搜索性能。
* [Commit cfc8](https://github.com/manticoresoftware/manticoresearch/commit/cfc87ecb6e33a8163c2235243b6b40e699dbf526) 为 `FACET` 和 `GROUP BY` 的 `ORDER BY` 增加了对 `COUNT(DISTINCT)` 的支持。
* [Issue #1103](https://github.com/manticoresoftware/manticoresearch/issues/1103) 改进了[日志](Logging/Server_logging.md#Server-logging)分块合并的清晰度。
* [Issue #1130](https://github.com/manticoresoftware/manticoresearch/issues/1130) 增加了对 [DBeaver](Integration/DBeaver.md) 的支持。
* [Issue #1546](https://github.com/manticoresoftware/manticoresearch/issues/1546) 为 [POLY2D()](Functions/Geo_spatial_functions.md#POLY2D%28%29)/[GEOPOLY2D()](Functions/Geo_spatial_functions.md#GEOPOLY2D%28%29) 函数实现了二级索引。
* [Issue #1630](https://github.com/manticoresoftware/manticoresearch/issues/1630) HTTP 请求现支持 `Content-Encoding: gzip`。
* [Issue #1831](https://github.com/manticoresoftware/manticoresearch/issues/1831) 新增 `SHOW LOCKS` 命令。
* [Issue #2187](https://github.com/manticoresoftware/manticoresearch/issues/2197) 允许 Buddy 请求绕过守护进程的 [searchd.max_connections](Server_settings/Searchd.md#max_connections) 约束。
* [Issue #2208](https://github.com/manticoresoftware/manticoresearch/issues/2208) JSON HTTP 接口新增表连接支持。
* [Issue #2235](https://github.com/manticoresoftware/manticoresearch/issues/2235) 通过 Buddy 以原始形式记录成功处理的查询。
* [Issue #2249](https://github.com/manticoresoftware/manticoresearch/issues/2249) 为复制表运行 `mysqldump` 增加了特殊模式。
* [Issue #2268](https://github.com/manticoresoftware/manticoresearch/issues/2268) 改进了 `CREATE TABLE` 和 `ALTER TABLE` 语句中外部文件复制时的重命名。
* [Issue #2402](https://github.com/manticoresoftware/manticoresearch/issues/2402) 将 [searchd.max_packet_size](Server_settings/Searchd.md#max_packet_size) 默认值更新为 128MB。
* [Issue #2419](https://github.com/manticoresoftware/manticoresearch/issues/2419) 在 JSON ["match"](Searching/Full_text_matching/Basic_usage.md#match) 中新增对[IDF 提升修饰符](Searching/Full_text_matching/Operators.md#IDF-boost-modifier)的支持。
* [Issue #2430](https://github.com/manticoresoftware/manticoresearch/issues/2430) 加强了[binlog](Logging/Binary_logging.md#Binary-logging)写入同步，防止错误发生。
* [Issue #2458](https://github.com/manticoresoftware/manticoresearch/issues/2458) 在 Windows 包中集成了 zlib 支持。
* [Issue #2479](https://github.com/manticoresoftware/manticoresearch/issues/2479) 增加了 SHOW TABLE INDEXES 命令支持。
* [Issue #2485](https://github.com/manticoresoftware/manticoresearch/issues/2485) 设置了 Buddy 回复的会话元数据。
* [Issue #2490](https://github.com/manticoresoftware/manticoresearch/issues/2490) 兼容端点聚合支持毫秒级别精度。
* [Issue #2500](https://github.com/manticoresoftware/manticoresearch/issues/2500) 修改了复制启动失败时集群操作的错误信息。
* [Issue #2584](https://github.com/manticoresoftware/manticoresearch/pull/2584) 在 SHOW STATUS 中新增[性能指标](Node_info_and_management/Node_status.md#Query-Time-Statistics)：最近 1、5、15 分钟内按查询类型统计的最小/最大/平均/95th/99th 百分位响应时间。
* [Issue #2639](https://github.com/manticoresoftware/manticoresearch/pull/2639) 请求和响应中的所有 `index` 词汇替换为 `table`。
* [Issue #2643](https://github.com/manticoresoftware/manticoresearch/issues/2643) 在 HTTP `/sql` 接口聚合结果中新增 `distinct` 列。
* [Issue #268](https://github.com/manticoresoftware/manticoresearch-buddy/issues/268) 实现了导入自 Elasticsearch 的数据类型自动检测。
* [Issue #2744](https://github.com/manticoresoftware/manticoresearch/issues/2744) 为字符串 JSON 字段比较表达式增加了排序规则支持。
* [Issue #2752](https://github.com/manticoresoftware/manticoresearch/issues/2752) 在选择列表中添加了对 `uuid_short` 表达式的支持。
* [Issue #2783](https://github.com/manticoresoftware/manticoresearch/issues/2783) Manticore Search 现在直接运行 Buddy，不再使用 `manticore-buddy` 包装器。
* [Issue #2785](https://github.com/manticoresoftware/manticoresearch/issues/2785) 对缺少的表和不支持插入操作的表区分了错误消息。
* [Issue #2789](https://github.com/manticoresoftware/manticoresearch/issues/2789) OpenSSL 3 现已静态编译进 `searchd`。
* [Issue #2790](https://github.com/manticoresoftware/manticoresearch/issues/2790) 添加了 `CALL uuid_short` 语句，可以生成多个 `uuid_short` 值的序列。
* [Issue #2803](https://github.com/manticoresoftware/manticoresearch/issues/2803) 为 JOIN 操作中的右表添加了单独的选项。
* [Issue #2810](https://github.com/manticoresoftware/manticoresearch/issues/2810) 提升了 HTTP JSON 聚合性能，使其达到 SphinxQL 中 `GROUP BY` 的水平。
* [Issue #2854](https://github.com/manticoresoftware/manticoresearch/issues/2854) 增加了对 Kibana 日期相关请求中 `fixed_interval` 的支持。
* [Issue #2909](https://github.com/manticoresoftware/manticoresearch/issues/2909) 实现了 JOIN 查询的批处理，显著提升了部分 JOIN 查询的性能，提升幅度达数百甚至数千倍。
* [Issue #2937](https://github.com/manticoresoftware/manticoresearch/issues/2937) 允许在全扫描查询中使用连接表权重。
* [Issue #2953](https://github.com/manticoresoftware/manticoresearch/issues/2953) 修正了连接查询的日志记录。
* [Issue #337](https://github.com/manticoresoftware/manticoresearch-buddy/issues/337) 在非调试模式下隐藏了 `searchd` 中的 Buddy 异常。
* [Issue #2931](https://github.com/manticoresoftware/manticoresearch/issues/2931) 如果用户为复制监听器设置错误端口，守护进程将带错误信息关闭。

### Bug 修复
* [Commit 0c6b](https://github.com/manticoresoftware/manticoresearch/commit/0c6bdaf41ea3684e05952e1a8273893535b923f5) 修复：当 JOIN 查询列数超过 32 时返回不正确的结果。
* [Issue #2335](https://github.com/manticoresoftware/manticoresearch/issues/2335) 解决了在条件中使用两个 json 属性时表连接失败的问题。
* [Issue #2338](https://github.com/manticoresoftware/manticoresearch/issues/2338) 修正了在带有 [cutoff](Searching/Options.md#cutoff) 的多查询中的 total_relation 计算错误。
* [Issue #2366](https://github.com/manticoresoftware/manticoresearch/issues/2366) 纠正了在 [表连接](Searching/Joining.md) 中右表按 `json.string` 过滤的问题。
* [Issue #2406](https://github.com/manticoresoftware/manticoresearch/issues/2406) 允许在任何 POST HTTP JSON 端点（insert/replace/bulk）使用 `null` 作为所有值的占位符，此时将使用默认值。
* [Issue #2418](https://github.com/manticoresoftware/manticoresearch/issues/2418) 通过调整 [max_packet_size](Server_settings/Searchd.md#max_packet_size) 网络缓冲区的分配，优化了内存使用，尤其是初始套接字探测阶段。
* [Issue #2420](https://github.com/manticoresoftware/manticoresearch/issues/2420) 修正了通过 JSON 接口将无符号整型插入 bigint 属性的问题。
* [Issue #2422](https://github.com/manticoresoftware/manticoresearch/issues/2422) 修复二级索引在启用 exclude 过滤器和 pseudo_sharding 时的正确工作。
* [Issue #2423](https://github.com/manticoresoftware/manticoresearch/issues/2423) 解决了 [manticore_new_cluster](Starting_the_server/Manually.md#searchd-command-line-options) 中的一个错误。
* [Issue #2448](https://github.com/manticoresoftware/manticoresearch/issues/2448) 解决了在 `_update` 请求格式错误时导致守护进程崩溃的问题。
* [Issue #2452](https://github.com/manticoresoftware/manticoresearch/issues/2452) 修正了直方图无法处理带 exclude 的值过滤器的问题。
* [Issue #55](https://github.com/manticoresoftware/columnar/issues/55) 修复了针对分布式表的 [knn](Searching/KNN.md#KNN-vector-search) 查询。
* [Issue #68](https://github.com/manticoresoftware/columnar/issues/68) 改善了列存取器在表编码上的 exclude 过滤器处理。
* [Commit 0eb1](https://github.com/manticoresoftware/manticoresearch/commit/0eb1579462013cb134bcaf74e06f6eeb0c2ecc4d) 修正表达式解析器未遵守重新定义的 `thread_stack`。
* [Commit c304](https://github.com/manticoresoftware/manticoresearch/commit/c3041fd1d3e14773f3c5aa635343915a71f5cb95) 修复了克隆列存 IN 表达式时崩溃的问题。
* [Commit edad](https://github.com/manticoresoftware/columnar/commit/edadc694c68d6022bdd13134263667430a42cc1d) 修正了位图迭代器中的反转问题，该问题导致崩溃。
* [Commit fc30](https://github.com/manticoresoftware/executor/commit/fc302ff1117b0b835a6f5a3c07057baf5fba14d9) 修复了某些 Manticore 包被 `unattended-upgrades` 自动移除的问题。
* [Issue #1019](https://github.com/manticoresoftware/manticoresearch/issues/1019) 改进了对 DbForge MySQL 工具生成查询的处理。
* [Issue #1107](https://github.com/manticoresoftware/manticoresearch/issues/1107) 修正了 `CREATE TABLE` 和 `ALTER TABLE` 中特殊字符转义的问题。❤️ 感谢 [@subnix](https://github.com/subnix) 的 PR。
* [Issue #116](https://github.com/manticoresoftware/manticoresearch-backup/issues/116#issuecomment-2216597206) 修复了在冻结索引中更新 blob 属性时出现的死锁。该死锁由于尝试解冻索引时的锁冲突引起，也可能导致 manticore-backup 出错。
* [Issue #1818](https://github.com/manticoresoftware/manticoresearch/issues/1818) 当表被冻结时，`OPTIMIZE` 现在会抛出错误。
* [Issue #2001](https://github.com/manticoresoftware/manticoresearch/issues/2001) 允许将函数名用作列名。
* [Issue #2153](https://github.com/manticoresoftware/manticoresearch/issues/2153) 修复了在查询未知磁盘块的表设置时守护进程崩溃的问题。
* [Issue #2184](https://github.com/manticoresoftware/manticoresearch/issues/2184) 修复了 `searchd` 在执行 `FREEZE` 和 `FLUSH RAMCHUNK` 后停止时挂起的问题。
* [Issue #2228](https://github.com/manticoresoftware/manticoresearch/issues/2228) 从保留字中移除了日期/时间相关的标记（和正则表达式）。
* [Issue #2255](https://github.com/manticoresoftware/manticoresearch/issues/2255) 修复了在 `FACET` 中使用超过 5 个排序字段时的崩溃问题。
* [Issue #2265](https://github.com/manticoresoftware/manticoresearch/issues/2265) 修复了启用 `index_field_lengths` 时无法恢复 `mysqldump` 的问题。
* [Issue #2291](https://github.com/manticoresoftware/manticoresearch/issues/2291) 修复了执行 `ALTER TABLE` 命令时的崩溃问题。
* [Issue #2333](https://github.com/manticoresoftware/manticoresearch/issues/2333) 修复了 Windows 包中 MySQL DLL 以保证索引器正常工作。
* [Issue #2393](https://github.com/manticoresoftware/manticoresearch/issues/2393) 修复了 GCC 编译错误。❤️ 感谢 [@animetosho](https://github.com/animetosho) 的 PR。
* [Issue #2447](https://github.com/manticoresoftware/manticoresearch/issues/2447) 修复了 [_update](Data_creation_and_modification/Updating_documents/REPLACE.md?client=Elasticsearch-like+partial) 中的转义问题。
* [Issue #2460](https://github.com/manticoresoftware/manticoresearch/issues/2460) 修复了在声明多个同名属性或字段时索引器崩溃的问题。
* [Issue #2467](https://github.com/manticoresoftware/manticoresearch/issues/2467) 解决了在 "compat" 搜索相关端点对于嵌套 bool 查询错误转换导致守护进程崩溃的问题。
* [Issue #2493](https://github.com/manticoresoftware/manticoresearch/issues/2493) 修复了带修饰符短语的展开问题。
* [Issue #2535](https://github.com/manticoresoftware/manticoresearch/issues/2535) 解决了使用 [ZONE](Searching/Full_text_matching/Operators.md#ZONE-limit-operator) 或 [ZONESPAN](Searching/Full_text_matching/Operators.md#ZONESPAN-limit-operator) 全文操作符时守护进程崩溃的问题。
* [Issue #2552](https://github.com/manticoresoftware/manticoresearch/issues/2552) 修复了带关键词字典的普通表和 RT 表的中缀生成问题。
* [Issue #2571](https://github.com/manticoresoftware/manticoresearch/issues/2571) 修复了 `FACET` 查询中的错误回复；为带 `COUNT(*)` 的 `FACET` 设置默认排序顺序为 `DESC`。
* [Issue #2580](https://github.com/manticoresoftware/manticoresearch/issues/2580) 修复了 Windows 启动时守护进程崩溃的问题。
* [Issue #2603](https://github.com/manticoresoftware/manticoresearch/issues/2603) 修复了 HTTP 端点 `/sql` 和 `/sql?mode=raw` 查询截断问题；使来自这些端点的请求一致，无需 `query=` 头。
* [Issue #2623](https://github.com/manticoresoftware/manticoresearch/issues/2623) 修复了自动模式创建表但同时失败的问题。
* [Issue #2627](https://github.com/manticoresoftware/manticoresearch/issues/2627) 修复了 HNSW 库以支持加载多个 KNN 索引。
* [Issue #2630](https://github.com/manticoresoftware/manticoresearch/issues/2630) 修复了多个条件同时出现时的冻结问题。
* [Issue #2645](https://github.com/manticoresoftware/manticoresearch/issues/2645) 修复了使用 `OR` 进行 KNN 搜索时致命错误的崩溃。
* [Issue #2647](https://github.com/manticoresoftware/manticoresearch/issues/2647) 修复了 `indextool --mergeidf *.idf --out global.idf` 在创建后删除输出文件的问题。
* [Issue #2658](https://github.com/manticoresoftware/manticoresearch/issues/2658) 修复了外层选择中带有 `ORDER BY` 字符串的子查询导致守护进程崩溃。
* [Issue #2686](https://github.com/manticoresoftware/manticoresearch/issues/2686) 修复了更新浮点属性和字符串属性时的崩溃。
* [Issue #2704](https://github.com/manticoresoftware/manticoresearch/issues/2704) 修复了来自 `lemmatize_xxx_all` 分词器的多个停用词增加后续标记的 `hitpos` 的问题。
* [Issue #2708](https://github.com/manticoresoftware/manticoresearch/issues/2708) 修复了 `ALTER ... ADD COLUMN ... TEXT` 时崩溃的问题。
* [Issue #2737](https://github.com/manticoresoftware/manticoresearch/issues/2737) 修复了在冻结表中更新 Blob 属性（且至少有一个 RAM 块）导致后续 `SELECT` 查询等待表解冻的问题。
* [Issue #2742](https://github.com/manticoresoftware/manticoresearch/issues/2742) 修复了带压缩因子查询跳过查询缓存的问题。
* [Issue #2775](https://github.com/manticoresoftware/manticoresearch/issues/2775) Manticore 现在在 `_bulk` 请求时对未知操作报错，而不是崩溃。
* [Issue #2791](https://github.com/manticoresoftware/manticoresearch/issues/2791) 修复了 HTTP `_bulk` 端点返回插入文档 ID 的问题。
* [Issue #2797](https://github.com/manticoresoftware/manticoresearch/issues/2797) 修复了处理多个表（其中一个为空另一个匹配条目数不同）时 grouper 的崩溃。
* [Issue #2835](https://github.com/manticoresoftware/manticoresearch/issues/2835) 修复了复杂 `SELECT` 查询中的崩溃。
* [Issue #2872](https://github.com/manticoresoftware/manticoresearch/issues/2872) 添加了错误信息，如果 `IN` 表达式中的 `ALL` 或 `ANY` 参数不是 JSON 属性。
* [Issue #2882](https://github.com/manticoresoftware/manticoresearch/issues/2882) 修复了在大表中更新 MVA 时守护进程崩溃。
* [Issue #2888](https://github.com/manticoresoftware/manticoresearch/issues/2888) 修复了 `libstemmer` 分词失败时的崩溃。❤️ 感谢 [@subnix](https://github.com/subnix) 的 PR。
* [Issue #2919](https://github.com/manticoresoftware/manticoresearch/issues/2919) 修复了右表连接权重在表达式中无法正确工作的缺陷。
* [问题 #2919](https://github.com/manticoresoftware/manticoresearch/issues/2919) 修复了右连接表权重在表达式中不起作用的问题。
* [问题 #325](https://github.com/manticoresoftware/manticoresearch-buddy/issues/325) 修复了当表已存在时，`CREATE TABLE IF NOT EXISTS ... WITH DATA` 失败的问题。
* [问题 #351](https://github.com/manticoresoftware/manticoresearch-buddy/issues/351) 修复了使用文档 ID 进行 KNN 计数时出现的未定义数组键 "id" 错误。
* [问题 #359](https://github.com/manticoresoftware/manticoresearch-buddy/issues/359) 修复了 `REPLACE INTO cluster_name:table_name` 的功能。
* [问题 #67](https://github.com/manticoresoftware/docker/issues/67) 修复了使用 `--network=host` 运行 Manticore Docker 容器时出现的 FATAL 错误。

## 版本 6.3.8
发布日期：2024年11月22日

版本 6.3.8 继续 6.3 系列，仅包含错误修复。

### 错误修复

* [PR #2777](https://github.com/manticoresoftware/manticoresearch/pull/2777) 修正了当查询并发受限于 `threads` 或 `max_threads_per_query` 设置时可用线程的计算。

## manticore-extra v1.1.20

发布日期：2024年10月7日

### 错误修复

* [问题 #64](https://github.com/manticoresoftware/executor/issues/64) 解决了 `unattended-upgrades` 工具在基于 Debian 的系统上自动安装软件包更新时，会错误地标记多个 Manticore 软件包（包括 `manticore-galera`、`manticore-executor` 和 `manticore-columnar-lib`）为待删除的问题。此问题是由于 `dpkg` 错误地将虚拟软件包 `manticore-extra` 视为冗余所致。已进行更改，确保 `unattended-upgrades` 不再尝试删除关键的 Manticore 组件。

## 版本 6.3.6
发布日期：2024年8月2日

版本 6.3.6 继续 6.3 系列，仅包含错误修复。

### 错误修复

* [问题 #2477](https://github.com/manticoresoftware/manticoresearch/issues/2477) 修复了 6.3.4 版本引入的崩溃问题，该问题可能在处理表达式和分布式或多表查询时发生。
* [问题 #2352](https://github.com/manticoresoftware/manticoresearch/issues/2352) 修复了查询多个索引时，由于 `max_query_time` 导致的守护进程崩溃或内部错误的早期退出问题。

## 版本 6.3.4
发布日期：2024年7月31日

版本 6.3.4 继续 6.3 系列，包含一些小改进和错误修复。

### 小改动
* [问题 #1130](https://github.com/manticoresoftware/manticoresearch/issues/1130) 添加了对 [DBeaver](Integration/DBeaver.md#Integration-with-DBeaver) 的支持。
* [问题 #2146](https://github.com/manticoresoftware/manticoresearch/issues/2146) 改进了词形和例外中的分隔符转义。
* [问题 #2315](https://github.com/manticoresoftware/manticoresearch/issues/2315) 向 SELECT 列表达式中添加了字符串比较运算符。
* [问题 #2363](https://github.com/manticoresoftware/manticoresearch/issues/2363) 支持 Elastic 风格的批量请求中的空值。
* [问题 #2374](https://github.com/manticoresoftware/manticoresearch/issues/2374) 增加了对 mysqldump 版本 9 的支持。
* [问题 #2375](https://github.com/manticoresoftware/manticoresearch/issues/2375) 改进了 HTTP JSON 查询中的错误处理，增加了指向发生错误节点的 JSON 路径。

### 错误修复
* [问题 #2280](https://github.com/manticoresoftware/manticoresearch/issues/2280) 修复了当 disk_chunks > 1 且匹配数多的通配符查询中的性能下降问题。
* [问题 #2332](https://github.com/manticoresoftware/manticoresearch/issues/2332) 修复了针对空 MVA 数组的 MVA MIN 或 MAX SELECT 列表达式中的崩溃。
* [问题 #2339](https://github.com/manticoresoftware/manticoresearch/issues/2339) 修复了 Kibana 无限范围请求的错误处理。
* [问题 #2342](https://github.com/manticoresoftware/manticoresearch/issues/2342) 修正了当属性不在 SELECT 列表中时，对右表列式属性的连接筛选器问题。
* [问题 #2343](https://github.com/manticoresoftware/manticoresearch/issues/2343) 修正了 Manticore 6.3.2 中重复的 'static' 说明符。
* [问题 #2344](https://github.com/manticoresoftware/manticoresearch/issues/2344) 修复了当 MATCH() 用于右表时 LEFT JOIN 返回非匹配条目的问题。
* [问题 #2350](https://github.com/manticoresoftware/manticoresearch/issues/2350) 修复了在 RT 索引中使用 `hitless_words` 保存磁盘块的问题。
* [问题 #2364](https://github.com/manticoresoftware/manticoresearch/issues/2364) 现在可以在其他属性之间以任意顺序添加 'aggs_node_sort' 属性。
* [问题 #2368](https://github.com/manticoresoftware/manticoresearch/issues/2368) 修正了 JSON 查询中全文本与过滤器顺序的错误。
* [问题 #2376](https://github.com/manticoresoftware/manticoresearch/issues/2376) 修复了针对长 UTF-8 请求的不正确 JSON 响应的错误。
* [问题 #2684](https://github.com/manticoresoftware/dev/issues/2684) 修正了依赖连接属性的预排序/预过滤表达式的计算。
* [问题 #301](https://github.com/manticoresoftware/manticoresearch-buddy/issues/301) 将指标数据大小计算方法改为从 `manticore.json` 文件读取，而不是检查数据目录的整体大小。
* [问题 #302](https://github.com/manticoresoftware/manticoresearch-buddy/issues/302) 增加了对 Vector.dev 验证请求的处理。

## 版本 6.3.2
发布日期：2024年6月26日

版本 6.3.2 继续 6.3 系列，包含若干错误修复，其中一些在 6.3.0 发布后发现。

### 重大变更
* ⚠️[问题 #2305](https://github.com/manticoresoftware/manticoresearch/issues/2305) 更新了 aggs.range 的值为数值型。

### 错误修复
* [提交 c51c](https://github.com/manticoresoftware/manticoresearch/commit/c51c7fa4bd0ec4ad8d53188b888461bff29f1417) 修正了存储检查与 rset 合并时的分组问题。
* [提交 0e85](https://github.com/manticoresoftware/manticoresearch/commit/0e85b9d5682f51681750d798aef6a11cfb93d69e) 修复了在使用 CRC 字典并启用 `local_df` 时，在 RT 索引中使用通配符查询导致守护进程崩溃的问题。
* [问题 #2200](https://github.com/manticoresoftware/manticoresearch/issues/2200) 修复了在没有 GROUP BY 的情况下使用 `count(*)` 进行 JOIN 时的崩溃。
* [问题 #2201](https://github.com/manticoresoftware/manticoresearch/issues/2201) 修复了在尝试按全文字段分组时，JOIN 未返回警告的问题。
* [问题 #2230](https://github.com/manticoresoftware/manticoresearch/issues/2230) 解决了通过 `ALTER TABLE` 添加属性时未考虑 KNN 选项的问题。
* [问题 #2231](https://github.com/manticoresoftware/manticoresearch/issues/2231) 修复了在 6.3.0 版本中无法删除 `manticore-tools` Redhat 包的问题。
* [问题 #2242](https://github.com/manticoresoftware/manticoresearch/issues/2242) 修正了 JOIN 和多个 FACET 语句返回错误结果的问题。
* [问题 #2250](https://github.com/manticoresoftware/manticoresearch/issues/2250) 修复了当表属于集群时执行 ALTER TABLE 报错的问题。
* [问题 #2252](https://github.com/manticoresoftware/manticoresearch/issues/2252) 修复了通过 SphinxQL 接口传递给 buddy 的原始查询。
* [问题 #2267](https://github.com/manticoresoftware/manticoresearch/issues/2267) 改善了针对带有磁盘块的 RT 索引的 `CALL KEYWORDS` 的通配符扩展。
* [问题 #271](https://github.com/manticoresoftware/manticoresearch-buddy/issues/271) 修复了错误的 `/cli` 请求导致的挂起。
* [问题 #274](https://github.com/manticoresoftware/manticoresearch-buddy/issues/274) 解决了对 Manticore 并发请求可能阻塞的问题。
* [问题 #275](https://github.com/manticoresoftware/manticoresearch-buddy/issues/275) 修复了通过 `/cli` 执行 `drop table if exists t; create table t` 导致的挂起。

### 复制相关
* [问题 #2270](https://github.com/manticoresoftware/manticoresearch/issues/2270) 在 `/_bulk` HTTP 端点中添加了对 `cluster:name` 格式的支持。

## 版本 6.3.0
发布：2024年5月23日

### 主要变化
* [问题 #839](https://github.com/manticoresoftware/manticoresearch/issues/839) 实现了 [float_vector](Creating_a_table/Data_types.md#Float-vector) 数据类型；实现了 [向量搜索](Searching/KNN.md#KNN-vector-search)。
* [问题 #1673](https://github.com/manticoresoftware/manticoresearch/issues/1673) 实现了 [INNER/LEFT JOIN](Searching/Joining.md)（**测试阶段**）。
* [问题 #1744](https://github.com/manticoresoftware/manticoresearch/issues/1744) 实现了对 [timestamp](Creating_a_table/Data_types.md#Timestamps) 字段日期格式的自动检测。
* [问题 #1720](https://github.com/manticoresoftware/manticoresearch/pull/1720) 将 Manticore Search 许可从 GPLv2-or-later 改为 GPLv3-or-later。
* [提交 7a55](https://github.com/manticoresoftware/manticoresearch/commit/7a5508424) 现在在 Windows 上运行 Manticore 需要使用 Docker 来运行 Buddy。
* [问题 #1541](https://github.com/manticoresoftware/manticoresearch/issues/1541) 添加了一个 [REGEX](Searching/Full_text_matching/Operators.md#REGEX-operator) 全文操作符。
* [问题 #2091](https://github.com/manticoresoftware/manticoresearch/issues/2091) 支持 Ubuntu Noble 24.04。
* [提交 514d](https://github.com/manticoresoftware/manticoresearch/commit/514d35b497f4bdb20b6473ff963752a978a4bb8d) 重构了时间操作以提升性能并新增日期/时间函数：
  - [CURDATE()](Functions/Date_and_time_functions.md#CURDATE%28%29) - 返回本地时区的当前日期
  - [QUARTER()](Functions/Date_and_time_functions.md#QUARTER%28%29) - 返回时间戳参数对应的年份季度（整数）
  - [DAYNAME()](Functions/Date_and_time_functions.md#DAYNAME%28%29) - 返回时间戳参数对应的星期名称
  - [MONTHNAME()](Functions/Date_and_time_functions.md#MONTHNAME%28%29) - 返回时间戳参数对应的月份名称
  - [DAYOFWEEK()](Functions/Date_and_time_functions.md#DAYOFWEEK%28%29) - 返回时间戳参数对应的星期整数索引
  - [DAYOFYEAR()](Functions/Date_and_time_functions.md#DAYOFYEAR%28%29) - 返回时间戳参数对应的年份中的天数（整数）
  - [YEARWEEK()](Functions/Date_and_time_functions.md#YEARWEEK%28%29) - 返回时间戳参数对应当前周第一天的年份和日代码（整数）
  - [DATEDIFF()](Functions/Date_and_time_functions.md#DATEDIFF%28%29) - 返回两个时间戳之间的天数差
  - [DATE()](Functions/Date_and_time_functions.md#DATE%28%29) - 格式化时间戳参数中的日期部分
  - [TIME()](Functions/Date_and_time_functions.md#TIME%28%29) - 格式化时间戳参数中的时间部分
  - [timezone](Server_settings/Searchd.md#timezone) - 日期/时间相关函数所用的时区。
* [提交 30e7](https://github.com/manticoresoftware/manticoresearch/commit/30e789d9657f9b3093c280c6e5b0e46a021f6dc7) 向 HTTP 接口添加了 [range](Searching/Faceted_search.md#Facet-over-set-of-ranges)、[histogram](Searching/Faceted_search.md#Facet-over-histogram-values)、[date_range](Searching/Faceted_search.md#Facet-over-set-of-date-ranges) 和 [date_histogram](Searching/Faceted_search.md#Facet-over-histogram-date-values) 聚合，并支持将类似表达式转换为 SQL。

### 次要变化
* [问题 #1285](https://github.com/manticoresoftware/manticoresearch/issues/1285) 支持 Filebeat 版本 8.10 - 8.11。
* [问题 #1771](https://github.com/manticoresoftware/manticoresearch/issues/1771) 支持 [ALTER TABLE ... type='distributed'](Updating_table_schema_and_settings.md#Changing-a-distributed-table)。
* [问题 #1788](https://github.com/manticoresoftware/manticoresearch/issues/1788) 添加了通过 [CREATE TABLE ... LIKE ... WITH DATA](Creating_a_table/Local_tables/Real-time_table.md#CREATE-TABLE-LIKE:) SQL 语句复制表的能力。
* [问题 #2072](https://github.com/manticoresoftware/manticoresearch/issues/2072) 优化了 [表压缩算法](Securing_and_compacting_a_table/Compacting_a_table.md#Compacting-a-Table)：之前，手动的 [OPTIMIZE](Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE) 和自动的 [auto_optimize](Server_settings/Searchd.md#auto_optimize) 过程都会先合并分块以确保数量不超过限制，然后从所有包含已删除文档的分块中清除已删除的文档。这种方法有时资源消耗过大，因此已被禁用。现在，分块合并仅根据 [progressive_merge](Server_settings/Common.md#progressive_merge) 设置进行。然而，包含大量已删除文档的分块更可能被合并。
* [提交 ce6c](https://github.com/manticoresoftware/columnar/commit/ce6c5d3381edcdc411a00a5923d13d56dd9b3010) 添加了防止加载新版二级索引的保护。
* [问题 #1417](https://github.com/manticoresoftware/manticoresearch/issues/1417) 通过 [REPLACE INTO ... SET](Data_creation_and_modification/Updating_documents/REPLACE.md#SQL-REPLACE) 实现部分替换。
* [提交 7c16](https://github.com/manticoresoftware/manticoresearch/commit/7c167b934ce416d3dfbd658744a231e638b14d9a) 更新了默认合并缓冲区大小：`.spa` (标量属性)：256KB -> 8MB；`.spb` (二进制属性)：256KB -> 8MB；`.spc` (列属性)：1MB，未变；`.spds` (文档存储)：256KB -> 8MB；`.spidx` (二级索引)：256KB缓冲区 -> 128MB内存限制；`.spi` (字典)：256KB -> 16MB；`.spd` (文档列表)：8MB，未变；`.spp` (匹配列表)：8MB，未变；`.spe` (跳表)：256KB -> 8MB。
* [问题 #1859](https://github.com/manticoresoftware/manticoresearch/issues/1859) 通过 JSON 添加了 [复合聚合](Searching/Grouping.md#GROUP-BY-multiple-fields-at-once)。
* [提交 216b](https://github.com/manticoresoftware/manticoresearch-buddy/commit/216b824) 因部分正则表达式模式存在问题且无明显时间优势，禁用了 PCRE.JIT。
* [提交 55cd](https://github.com/manticoresoftware/manticoresearch/commit/55cdce32) 增加了对原生 Galera v.3 (api v25) (`libgalera_smm.so` 来自 MySQL 5.x) 的支持。
* [提交 86f9](https://github.com/manticoresoftware/manticoresearch-buddy/commit/86f98ab) 指标后缀从 `_rate` 改为 `_rps`。
* [提交 c0c1](https://github.com/manticoresoftware/manticoresearch-helm/commit/c0c1cff780031412ef57cefab987dfd16613368e) 改进了负载均衡器高可用性支持的文档。
* [提交 d1d2](https://github.com/manticoresoftware/manticoresearch/commit/d1d2ca18857c68106e89743756260e73285be739) 错误信息中的 `index` 改为 `table`；修正了 bison 解析器错误信息。
* [提交 fd26](https://github.com/manticoresoftware/manticoresearch/commit/fd26671b) 支持 `manticore.tbl` 作为表名。
* [问题 #1105](https://github.com/manticoresoftware/manticoresearch/issues/1105) 支持通过 systemd 运行索引器 ([文档](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Schedule-indexer-via-systemd))。❤️ 感谢 [@subnix](https://github.com/subnix) 的拉取请求。
* [问题 #1294](https://github.com/manticoresoftware/manticoresearch/issues/1294) 在 GEODIST() 中支持二级索引。
* [问题 #1394](https://github.com/manticoresoftware/manticoresearch/issues/1394) 简化了 [SHOW THREADS](Node_info_and_management/SHOW_THREADS.md#SHOW-THREADS)。
* [问题 #1424](https://github.com/manticoresoftware/manticoresearch/issues/1424) 在 `create distributed table` 语句中添加了默认值支持 (`agent_connect_timeout` 和 `agent_query_timeout`)。
* [问题 #1442](https://github.com/manticoresoftware/manticoresearch/issues/1442) 添加了 [expansion_limit](Searching/Options.md#expansion_limit) 搜索查询选项，可覆盖 `searchd.expansion_limit`。
* [问题 #1448](https://github.com/manticoresoftware/manticoresearch/issues/1448) 实现了用于 int->bigint 转换的 [ALTER TABLE](Updating_table_schema_and_settings.md#Updating-table-schema-in-RT-mode)。
* [问题 #1456](https://github.com/manticoresoftware/manticoresearch/issues/1456) 在 MySQL 响应中添加了元信息。
* [问题 #1494](https://github.com/manticoresoftware/manticoresearch/issues/1494) [SHOW VERSION](Node_info_and_management/SHOW_VERSION.md#SHOW-VERSION)。
* [问题 #1582](https://github.com/manticoresoftware/manticoresearch/issues/1582) 通过 JSON 支持 [按 id 数组删除文档](Data_creation_and_modification/Deleting_documents.md?client=JSON#Deleting-documents)。
* [问题 #1589](https://github.com/manticoresoftware/manticoresearch/issues/1589) 改进“unsupported value type”错误。
* [问题 #1634](https://github.com/manticoresoftware/manticoresearch/issues/1634) 在 `SHOW STATUS` 中加入 Buddy 版本信息。
* [问题 #1641](https://github.com/manticoresoftware/manticoresearch/issues/1641) 针对关键词文档数为零的情况优化匹配请求。
* [问题 #1712](https://github.com/manticoresoftware/manticoresearch/issues/1712) 在发送数据时支持将字符串值 "true" 和 "false" 转换为布尔属性。
* [问题 #1713](https://github.com/manticoresoftware/manticoresearch/issues/1713) 添加了 [access_dict](Server_settings/Searchd.md#access_dict) 表和 searchd 选项。
* [问题 #1767](https://github.com/manticoresoftware/manticoresearch/issues/1767) 在 config 的 searchd 部分添加了新选项：[expansion_merge_threshold_docs](Server_settings/Searchd.md#expansion_merge_threshold_docs) 和 [expansion_merge_threshold_hits](Server_settings/Searchd.md#expansion_merge_threshold_hits)；使得扩展词条中小频次合并的阈值可配置。
* [问题 #1768](https://github.com/manticoresoftware/manticoresearch/issues/1768) 在 `@@system.sessions` 显示最近命令时间。
* [问题 #1806](https://github.com/manticoresoftware/manticoresearch/issues/1806) 升级 Buddy 协议至版本 2。
* [Issue #1810](https://github.com/manticoresoftware/manticoresearch/issues/1810) 实现了对 `/sql` 端点的额外请求格式支持，以简化与库的集成。
* [Issue #1825](https://github.com/manticoresoftware/manticoresearch/pull/1825) 在 SHOW BUDDY PLUGINS 中添加了信息部分。
* [Issue #1837](https://github.com/manticoresoftware/manticoresearch/issues/1837) 改进了 `CALL PQ` 大包时的内存消耗。
* [Issue #1853](https://github.com/manticoresoftware/manticoresearch/pull/1853) 将编译器从 Clang 15 切换到 Clang 16。
* [Issue #1857](https://github.com/manticoresoftware/manticoresearch/issues/1857) 添加了字符串比较。❤️ 感谢 [@etcd](https://github.com/etcd) 的 PR。
* [Issue #1915](https://github.com/manticoresoftware/manticoresearch/issues/1915) 添加了对连接存储字段的支持。
* [Issue #1937](https://github.com/manticoresoftware/manticoresearch/issues/1937) 在查询日志中记录客户端的 host:port。
* [Issue #1981](https://github.com/manticoresoftware/manticoresearch/issues/1981) 修正了错误的错误信息。
* [Issue #1983](https://github.com/manticoresoftware/manticoresearch/issues/1983) 引入了针对 [通过 JSON 的查询计划](Node_info_and_management/Profiling/Query_plan.md?client=JSON#Query-plan) 的详细等级支持。
* [Issue #2010](https://github.com/manticoresoftware/manticoresearch/issues/2010) 除非设置了 `log_level=debug`，否则禁用 Buddy 查询日志记录。
* [Issue #2035](https://github.com/manticoresoftware/manticoresearch/issues/2035) 支持 Linux Mint 21.3。
* [Issue #2056](https://github.com/manticoresoftware/manticoresearch/issues/2056) 修复 Manticore 无法用 Mysql 8.3+ 构建的问题。
* [Issue #2112](https://github.com/manticoresoftware/manticoresearch/issues/2112) 为实时表块添加 `DEBUG DEDUP` 命令，解决附加含重复项的普通表造成重复条目的问题。
* [Issue #212](https://github.com/manticoresoftware/manticoresearch-buddy/issues/212) 在 [SHOW QUERIES](Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES) 中添加了时间。
* [Issue #218](https://github.com/manticoresoftware/manticoresearch-buddy/issues/218) 将 `@timestamp` 列作为时间戳处理。
* [Issue #252](https://github.com/manticoresoftware/manticoresearch-buddy/issues/250) 实现了开启/禁用 buddy 插件的逻辑。
* [Issue #254](https://github.com/manticoresoftware/manticoresearch-buddy/issues/254) 更新了 composer 至更新版本，修复了近期的 CVE。
* [Issue #340](https://github.com/manticoresoftware/manticoresearch/issues/340) Manticore systemd 单元中关于 `RuntimeDirectory` 的小优化。
* [Issue #51](https://github.com/manticoresoftware/executor/issues/51) 添加了 rdkafka 支持并更新至 PHP 8.3.3。
* [Issue #527](https://github.com/manticoresoftware/manticoresearch/issues/527) 支持 [附加](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Attaching_one_table_to_another.md#Attaching-one-table-to-another) 实时表。新命令 [ALTER TABLE ... RENAME](Updating_table_schema_and_settings.md#Renaming-a-real-time-table)。

### 重大变更与废弃项
* ⚠️[Issue #1436](https://github.com/manticoresoftware/manticoresearch/issues/1436) 修正了 IDF 计算问题。`local_df` 现在为默认。改进了主-代理搜寻协议（版本更新）。如果在分布式环境中运行多个实例，请先升级代理，再升级主节点。
* ⚠️[Issue #1572](https://github.com/manticoresoftware/manticoresearch/issues/1572) 添加分布式表的复制及更新复制协议。如果运行复制集群，需：
  - 首先，干净地停止所有节点
  - 然后，使用 Linux 下工具 `manticore_new_cluster` 以 `--new-cluster` 参数启动最后停止的节点。
  - 详情请阅读 [重启集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster)。
* ⚠️[Issue #1763](https://github.com/manticoresoftware/manticoresearch/pull/1763) HTTP API 端点别名 `/json/*` 已废弃。
* ⚠️[Issue #1982](https://github.com/manticoresoftware/manticoresearch/issues/1982) JSON 中的查询 [profile](Node_info_and_management/Profiling/Query_profile.md#Query-profile) 改为 [plan](Node_info_and_management/Profiling/Query_plan.md#Query-plan)，新增了 JSON 查询分析。
* ⚠️[Commit e235](https://github.com/manticoresoftware/manticoresearch-backup/commit/e23585a) manticore-backup 不再备份 `plugin_dir`。
* ⚠️[Issue #171](https://github.com/manticoresoftware/manticoresearch-buddy/issues/155) Buddy 迁移至 Swoole 以提升性能和稳定性。版本切换时，确保所有 Manticore 包均已更新。
* ⚠️[Issue #196](https://github.com/manticoresoftware/manticoresearch-buddy/pull/205) 合并所有核心插件到 Buddy，并更改核心逻辑。
* ⚠️[Issue #2107](https://github.com/manticoresoftware/manticoresearch/issues/2107) `/search` 响应中将文档 ID 视为数字。
* ⚠️[Issue #38](https://github.com/manticoresoftware/executor/issues/38) 添加 Swoole，禁用 ZTS，移除并行扩展。
* ⚠️[Issue #1929](https://github.com/manticoresoftware/manticoresearch/issues/1929) 在某些情况下，`charset_table` 中的覆盖未生效。

### 复制相关变更
* [Commit 3376](https://github.com/manticoresoftware/manticoresearch/commit/3376351f73db3ff0cb6cb708a304a7bd83e69cfb) 修复了大文件 SST 复制错误。
* [Commit 6d36](https://github.com/manticoresoftware/manticoresearch/commit/6d36c68fb7ce3cb274b467aac435788905084952) 为复制命令添加重试机制；修复了在数据包丢失的繁忙网络上复制连接失败的问题。
* [Commit 842e](https://github.com/manticoresoftware/manticoresearch/commit/842e27e71790833ab4125d49103d2933d5df60c3) 将复制中的 FATAL 信息改为 WARNING 信息。
* [Commit 8c32](https://github.com/manticoresoftware/manticoresearch/commit/8c32bce7db39d8ed3b1a06aaa1d2c3ec372084f0) 修正了复制集群中没有表或表为空时 `gcache.page_size` 的计算；同时修复了 Galera 选项的保存和加载。
* [Commit a2af](https://github.com/manticoresoftware/manticoresearch/commit/a2af06ca36abcbf893e43c8803853130b397c54b) 添加了跳过加入集群节点上的更新节点复制命令的功能。
* [Commit c054](https://github.com/manticoresoftware/manticoresearch/commit/c0541e6892f5116b1f957b40060ba8ecd01a5afb) 修复了更新 blob 属性与替换文档时复制期间的死锁问题。
* [Commit e80d](https://github.com/manticoresoftware/manticoresearch/commit/e80d505b963999016033613c40c1cf3151d648e7) 添加了 [replication_connect_timeout](Server_settings/Searchd.md#replication_connect_timeout)、[replication_query_timeout](Server_settings/Searchd.md#replication_query_timeout)、[replication_retry_delay](Server_settings/Searchd.md#replication_retry_delay)、[replication_retry_count](Server_settings/Searchd.md#replication_retry_count) searchd 配置选项，用于控制复制过程中的网络，与 `searchd.agent_*` 类似但默认值不同。
* [Issue #1356](https://github.com/manticoresoftware/manticoresearch/issues/1356) 修复了复制节点在部分节点丢失且节点名称解析失败时的重试问题。
* [Issue #1445](https://github.com/manticoresoftware/manticoresearch/issues/1445) 修正了 `show variables` 中复制日志的冗余级别。
* [Issue #1482](https://github.com/manticoresoftware/manticoresearch/issues/1482) 修复了 Kubernetes 中 pod 重启导致加入节点连接集群的复制问题。
* [Issue #1962](https://github.com/manticoresoftware/manticoresearch/issues/1962) 修复了在空集群中使用无效节点名调整复制时的长时间等待问题。

### Bug 修复
* [Commit 8a48](https://github.com/manticoresoftware/manticoresearch/commit/8a486c7ba1a293879cdecc41fb52eb7ab0cbf832) 修正了 `count distinct` 中未使用匹配项清理可能导致崩溃的问题。
* [Issue #1569](https://github.com/manticoresoftware/manticoresearch/issues/1569) 二进制日志现在以事务粒度写入。
* [Issue #2089](https://github.com/manticoresoftware/manticoresearch/issues/2089) 修复了与 64 位 ID 相关的错误，插入时可能导致 “Malformed packet” 错误，从而引发 [表损坏和重复 ID](https://github.com/manticoresoftware/manticoresearch/issues/2090)。
* [Issue #2160](https://github.com/manticoresoftware/manticoresearch/issues/2160) 修复了日期插入时被当作 UTC 而非本地时区的问题。
* [Issue #2177](https://github.com/manticoresoftware/manticoresearch/issues/2177) 修复了在实时表中使用非空 `index_token_filter` 执行搜索时的崩溃。
* [Issue #2209](https://github.com/manticoresoftware/manticoresearch/issues/2209) 修改了 RT 列存储中的重复过滤，修复了崩溃和错误的查询结果。
* [Commit 001d](https://github.com/manticoresoftware/manticoresearch/commit/001d82904e4b887c49387308291aeefb07c0fbc4) 修复了 html 去除器在处理联合字段后损坏内存的问题。
* [Commit 00eb](https://github.com/manticoresoftware/manticoresearch/commit/00eb2ebf) 避免执行 flush 后回绕流，防止与 mysqldump 的通信故障。
* [Commit 0553](https://github.com/manticoresoftware/manticoresearch/commit/05534114acf08c70ffcc41a23d558dec09a34373) 未启动预读时不等待预读完成。
* [Commit 055a](https://github.com/manticoresoftware/manticoresearch/commit/055aac09f877570cd29b6594cb283182eba7f73b) 修正了搜索守护进程日志中大型 Buddy 输出字符串的多行拆分。
* [Commit 0a88](https://github.com/manticoresoftware/manticoresearch/commit/0a88056307f82c50fcdc458ad6dbc307a00e3f92) 移动了 MySQL 接口关于头部 `debugv` 冗余级别失败的警告。
* [Commit 150a](https://github.com/manticoresoftware/manticoresearch/commit/150a20286b7d98d24b844ae577d7fda4a444798d) 修复了多个集群管理操作中的竞态条件；禁止创建同名或同路径的多个集群。
* [Commit 2e40](https://github.com/manticoresoftware/manticoresearch/commit/2e405c1c9ebac6f46206e3df17ac1c19129c9aa3) 修复全文查询中的隐式截断；拆分 MatchExtended 为模板 partD。
* [Commit 75f5](https://github.com/manticoresoftware/manticoresearch/commit/75f51421542a2a0658f9e2f9f31048c8c5224479) 修复了索引和加载表到守护进程时 `index_exact_words` 不一致的问题。
* [Commit 7643](https://github.com/manticoresoftware/manticoresearch/commit/7643976bea142e08535d701000eff0d09a99fda2) 修正了无效集群删除时缺失的错误消息。
* [Commit 7a03](https://github.com/manticoresoftware/manticoresearch/commit/7a034ac990d5ec9344320c05a6a2315583b9b858) 修正了 CBO 与队列联合问题；修正了 CBO 与 RT 伪分片问题。
* [Commit 7b4e](https://github.com/manticoresoftware/manticoresearch/commit/7b4ea95e) 在没有配置二级索引（SI）库和相关参数时，避免发出误导性警告信息 'WARNING: secondary_indexes set but failed to initialize secondary library'。
* [Commit 8496](https://github.com/manticoresoftware/manticoresearch/commit/8496317f099fade8d2db463fb88483e7fcdfe85b) 修正了仲裁中的命中排序。
* [Commit 8973](https://github.com/manticoresoftware/manticoresearch-buddy/commit/8973ad3) 修复了 ModifyTable 插件中大写选项的问题。
* [Commit 9935](https://github.com/manticoresoftware/manticoresearch-buddy/commit/99350d4c184379b19f3efa9751d9b5be5e6edfb5) 修复了从含空 json 值（用 NULL 表示）的转储中恢复的问题。
* [Commit a28f](https://github.com/manticoresoftware/manticoresearch/commit/a28f6e087b3d957e48816070be17d0823904936a) 修复了使用 pcon 接收 SST 时加入节点的 SST 超时问题。
* [Commit b5a5](https://github.com/manticoresoftware/manticoresearch/commit/b5a55cc26debcddcda5d9caef8366a707e0dd763) 修复了选择别名字符串属性时的崩溃问题。
* [Commit c556](https://github.com/manticoresoftware/manticoresearch/commit/c556e7ed8b6da8eac8ef094650df5f0a8977916b) 为带有 `morphology_skip_fields` 字段的全文查询添加了将术语转换为 `=term` 的查询转换。
* [Commit cdc3](https://github.com/manticoresoftware/manticoresearch/commit/cdc38085145cd1025b96fcac2a46188a0425d9e1) 添加了缺失的配置键（skiplist_cache_size）。
* [Commit cf6e](https://github.com/manticoresoftware/manticoresearch/commit/cf6ec3bfdc3f0990fb31ff53822b2331a6667727) 修复了在表达式排序器处理大型复杂查询时的崩溃。
* [Commit e513](https://github.com/manticoresoftware/manticoresearch/commit/e5139ba75eef7223df375ae3cc3f138fa7d925f1) 修复了全文CBO与无效索引提示之间的问题。
* [Commit eb05](https://github.com/manticoresoftware/manticoresearch/commit/eb0514b3a6f57f278252b0724eebd112ec116e0f) 中断了预读以实现更快速的关闭。
* [Commit f945](https://github.com/manticoresoftware/manticoresearch/commit/f94555a293cfa51004ff81135b8bd8500b955115) 修改了全文查询的堆栈计算，以避免复杂查询时崩溃。
* [Issue #1262](https://github.com/manticoresoftware/manticoresearch/issues/1262) 修复了索引器在为具有相同名称多个列的SQL源建立索引时的崩溃。
* [Issue #1273](https://github.com/manticoresoftware/manticoresearch/issues/1273) 对于不存在的 sysvars，返回 0 而不是 <empty>。
* [Issue #1289](https://github.com/manticoresoftware/manticoresearch/issues/1289) 修复了检查RT表外部文件时 indextool 报错的问题。
* [Issue #1335](https://github.com/manticoresoftware/manticoresearch/issues/1335) 修复了由于短语中存在多词形态导致的查询解析错误。
* [Issue #1364](https://github.com/manticoresoftware/manticoresearch/issues/1364) 增加了对旧版本binlog空文件的重放支持。
* [Issue #1365](https://github.com/manticoresoftware/manticoresearch/issues/1365) 修复了删除最后一个空binlog文件的问题。
* [Issue #1372](https://github.com/manticoresoftware/manticoresearch/issues/1372) 修复了修改 `data_dir` 后启动守护进程时相对路径不正确（由守护进程启动目录的绝对路径替代）的问题。
* [Issue #1393](https://github.com/manticoresoftware/manticoresearch/issues/1393) hn_small 中最慢的时间退化：守护进程启动时提取/缓存CPU信息。
* [Issue #1395](https://github.com/manticoresoftware/manticoresearch/issues/1395) 修复了索引加载时关于缺失外部文件的警告。
* [Issue #1402](https://github.com/manticoresoftware/manticoresearch/issues/1402) 修复了释放数据指针属性时全局分组器的方法崩溃。
* [Issue #1403](https://github.com/manticoresoftware/manticoresearch/issues/1403) _ADDITIONAL_SEARCHD_PARAMS 不生效的问题。
* [Issue #1427](https://github.com/manticoresoftware/manticoresearch/issues/1427) 修复了每表的 `agent_query_timeout` 被默认查询选项 `agent_query_timeout` 替换的问题。
* [Issue #1444](https://github.com/manticoresoftware/manticoresearch/issues/1444) 修复了使用 `packedfactors()` 并为每个匹配多值时分组器和排序器崩溃问题。
* [Issue #1458](https://github.com/manticoresoftware/manticoresearch/issues/1458) Manticore 在频繁更新索引时崩溃。
* [Issue #1481](https://github.com/manticoresoftware/manticoresearch/issues/1481) 修复了解析错误后清理解析查询时的崩溃。
* [Issue #1484](https://github.com/manticoresoftware/manticoresearch/issues/1484) 修复了HTTP JSON请求未被转发给buddy的问题。
* [Issue #1499](https://github.com/manticoresoftware/manticoresearch/issues/1499) 修复了JSON属性根值不能是数组的问题。
* [Issue #1507](https://github.com/manticoresoftware/manticoresearch/issues/1507) 修复了事务内重建表时的崩溃。
* [Issue #1515](https://github.com/manticoresoftware/manticoresearch/issues/1515) 修复了RU词形的简写扩展问题。
* [Issue #1579](https://github.com/manticoresoftware/manticoresearch/issues/1579) 修复了在 [date_format](Functions/Date_and_time_functions.md#DATE_FORMAT()) 表达式中 JSON 和 STRING 属性的使用问题。
* [Issue #1580](https://github.com/manticoresoftware/manticoresearch/issues/1580) 修复了针对JSON字段多个别名的分组器问题。
* [Issue #1594](https://github.com/manticoresoftware/manticoresearch/issues/1594) 开发版中 total_related 错误：修复了隐式截断与限制冲突；增加了JSON查询中更好的全扫描检测。
* [Issue #1603](https://github.com/manticoresoftware/manticoresearch/issues/1603) 修复了所有日期表达式中JSON和STRING属性的使用问题。
* [Issue #1609](https://github.com/manticoresoftware/manticoresearch/issues/1609) 使用 LEVENSHTEIN() 时崩溃。
* [Issue #1612](https://github.com/manticoresoftware/manticoresearch/issues/1612) 修复了带高亮的搜索查询解析错误后引起的内存损坏。
* [Issue #1614](https://github.com/manticoresoftware/manticoresearch/issues/1614) 禁用对短于 `min_prefix_len` / `min_infix_len` 术语的通配符展开。
* [Issue #1617](https://github.com/manticoresoftware/manticoresearch/issues/1617) 修改了行为，如果Buddy成功处理请求，则不再记录错误日志。
* [Issue #1635](https://github.com/manticoresoftware/manticoresearch/issues/1635) 修复了带有限制（limit）设置的查询在元数据中的总数不正确。
* [Issue #1640](https://github.com/manticoresoftware/manticoresearch/issues/1640) 通过JSON以纯文本模式使用大写表名不可能。
* [Issue #1643](https://github.com/manticoresoftware/manticoresearch/issues/1643) 提供了默认的 `SPH_EXTNODE_STACK_SIZE` 值。
* [Issue #1646](https://github.com/manticoresoftware/manticoresearch/issues/1646) 修复了SphinxQL中关于MVA属性使用ALL/ANY的负过滤器日志问题。
* [Issue #1660](https://github.com/manticoresoftware/manticoresearch/issues/1660) 修正了从其他索引应用 docid killlists 的问题。❤️ 感谢 [@raxoft](https://github.com/raxoft) 提交的 PR。
* [Issue #1668](https://github.com/manticoresoftware/manticoresearch/issues/1668) 修复了由于原始索引全表扫描（无索引迭代器）时的提前退出导致的匹配遗漏问题；移除了普通行迭代器中的 cutoff。
* [Issue #1671](https://github.com/manticoresoftware/manticoresearch/issues/1671) 修复了在使用代理和本地表查询分布式表时的 `FACET` 错误。
* [Issue #1690](https://github.com/manticoresoftware/manticoresearch/issues/1690) 修复了大值直方图估算时的崩溃。
* [Issue #1692](https://github.com/manticoresoftware/manticoresearch/issues/1692) 在 alter table tbl add column col uint 操作中出现崩溃。
* [Issue #1710](https://github.com/manticoresoftware/manticoresearch/issues/1710) 条件 `WHERE json.array IN (<value>)` 结果为空。
* [Issue #172](https://github.com/manticoresoftware/manticoresearch-buddy/issues/172) 修复了向 `/cli` 发送请求时 TableFormatter 的问题。
* [Issue #1742](https://github.com/manticoresoftware/manticoresearch/issues/1742) 当缺少 wordforms 文件时，`CREATE TABLE` 不再失败。
* [Issue #1762](https://github.com/manticoresoftware/manticoresearch/issues/1762) RT 表中属性的顺序现在遵循配置顺序。
* [Issue #1765](https://github.com/manticoresoftware/manticoresearch/issues/1765) HTTP bool 查询中带 'should' 条件时返回了错误的结果。
* [Issue #1769](https://github.com/manticoresoftware/manticoresearch/issues/1769) 使用 `SPH_SORT_ATTR_DESC` 和 `SPH_SORT_ATTR_ASC` 按字符串属性排序时无效。
* [Issue #177](https://github.com/manticoresoftware/manticoresearch-buddy/issues/177) 禁用了对 Buddy 的 curl 请求中的 `Expect: 100-continue` HTTP 头。
* [Issue #1791](https://github.com/manticoresoftware/manticoresearch/issues/1791) 由 GROUP BY 别名引起的崩溃。
* [Issue #1792](https://github.com/manticoresoftware/manticoresearch/issues/1792) Windows 上 SQL meta summary 显示了错误的时间。
* [Issue #1794](https://github.com/manticoresoftware/manticoresearch/issues/1794) 修复了使用 JSON 查询时的单词性能下降。
* [Issue #1798](https://github.com/manticoresoftware/manticoresearch/issues/1798) 不兼容的过滤器在 `/search` 上未抛出错误。
* [Issue #1802](https://github.com/manticoresoftware/manticoresearch/issues/1802) 修正了 `ALTER CLUSTER ADD` 和 `JOIN CLUSTER` 操作相互等待，防止在 ALTER 添加表到集群时捐赠节点向加入节点发送表的竞态条件。
* [Issue #1811](https://github.com/manticoresoftware/manticoresearch/issues/1811) 错误处理 `/pq/{table}/*` 请求。
* [Issue #1816](https://github.com/manticoresoftware/manticoresearch/issues/1816) 在某些情况下 `UNFREEZE` 不工作。
* [Issue #183](https://github.com/manticoresoftware/manticoresearch-buddy/pull/183) 修复了某些情况下 MVA 恢复的问题。
* [Issue #1849](https://github.com/manticoresoftware/manticoresearch/issues/1849) 修复了在 shutdown 时如果与 MCL 一起使用 indextool 崩溃的问题。
* [Issue #1866](https://github.com/manticoresoftware/manticoresearch/issues/1866) 修正了 `/cli_json` 请求中不必要的 URL 解码。
* [Issue #1872](https://github.com/manticoresoftware/manticoresearch/issues/1872) 更改了守护进程启动时 plugin_dir 的设置逻辑。
* [Issue #1874](https://github.com/manticoresoftware/manticoresearch/issues/1874) alter table ... exceptions 失败。
* [Issue #1891](https://github.com/manticoresoftware/manticoresearch/issues/1891#issuecomment-2051941200) 插入数据时 Manticore 崩溃并显示 `signal 11`。
* [Issue #1920](https://github.com/manticoresoftware/manticoresearch/issues/1920) 降低了 [low_priority](Searching/Options.md#low_priority) 的限速。
* [Issue #1924](https://github.com/manticoresoftware/manticoresearch/issues/1924) Mysqldump + mysql 恢复的错误。
* [Issue #1951](https://github.com/manticoresoftware/manticoresearch/issues/1951) 修正了在本地表缺失或代理描述不正确的情况下分布式表错误创建的问题；现在返回错误信息。
* [Issue #1972](https://github.com/manticoresoftware/manticoresearch/issues/1972) 实现了 `FREEZE` 计数器以避免 freeze/unfreeze 问题。
* [Issue #1980](https://github.com/manticoresoftware/manticoresearch/issues/1980) 遵守 OR 节点中的查询超时。此前 `max_query_time` 在某些情况下可能不起作用。
* [Issue #1986](https://github.com/manticoresoftware/manticoresearch/issues/1986) 未能将 new 重命名为当前的 [manticore.json]。
* [Issue #1988](https://github.com/manticoresoftware/manticoresearch/issues/1988) 全文查询可能忽略了 `SecondaryIndex` 的 CBO 提示。
* [Issue #1990](https://github.com/manticoresoftware/manticoresearch/issues/1990) 修正了 `expansion_limit` 用于对来自多个磁盘块或内存块的调用关键字切片最终结果集的问题。
* [Issue #1994](https://github.com/manticoresoftware/manticoresearch/issues/1994) 错误的外部文件。
* [Issue #2021](https://github.com/manticoresoftware/manticoresearch/issues/2021) 停止 Manticore 后可能仍有少数 manticore-executor 进程残留运行。
* [Issue #2029](https://github.com/manticoresoftware/manticoresearch/issues/2029) 使用 Levenshtein Distance 时崩溃。
* [Issue #2037](https://github.com/manticoresoftware/manticoresearch/issues/2037) 多个 max 操作执行在空索引上后出现错误。
* [Issue #2052](https://github.com/manticoresoftware/manticoresearch/issues/2052) 多组使用 JSON.field 时崩溃。
* [Issue #2067](https://github.com/manticoresoftware/manticoresearch/issues/2067) 对 _update 的错误请求导致 Manticore 崩溃。
* [Issue #2069](https://github.com/manticoresoftware/manticoresearch/issues/2069) 修正了 JSON 接口中闭区间字符串过滤比较器的问题。
* [问题 #2082](https://github.com/manticoresoftware/manticoresearch/issues/2082) 当 data_dir 路径位于符号链接上时，`alter` 失败。
* [问题 #2102](https://github.com/manticoresoftware/manticoresearch/pull/2102) 改进了 mysqldump 中对 SELECT 查询的特殊处理，以确保生成的 INSERT 语句与 Manticore 兼容。
* [问题 #2103](https://github.com/manticoresoftware/manticoresearch/issues/2103) 泰文字字符集错误。
* [问题 #2124](https://github.com/manticoresoftware/manticoresearch/issues/2124) 如果使用包含保留字的 SQL 会导致崩溃。
* [问题 #2154](https://github.com/manticoresoftware/manticoresearch/issues/2154) 带有词形变化的表无法导入。
* [问题 #2176](https://github.com/manticoresoftware/manticoresearch/issues/2176) 修复了当引擎参数设置为 'columnar' 且通过 JSON 添加重复 ID 时发生的崩溃。
* [问题 #223](https://github.com/manticoresoftware/manticoresearch-buddy/issues/223) 试图插入无模式且无列名的文档时显示适当错误。
* [问题 #239](https://github.com/manticoresoftware/manticoresearch-buddy/issues/239) 自动模式多行插入可能失败。
* [问题 #399](https://github.com/manticoresoftware/manticoresearch/issues/399) 在索引时如果数据源中声明了 id 属性，增加错误消息。
* [问题 #59](https://github.com/manticoresoftware/manticoresearch-helm/issues/59) Manticore 集群崩溃。
* [问题 #68](https://github.com/manticoresoftware/manticoresearch-helm/issues/68) 如果存在 percolate 表，optimize.php 会崩溃。
* [问题 #77](https://github.com/manticoresoftware/manticoresearch-helm/issues/77) 修复了在 Kubernetes 上部署时的错误。
* [问题 #274](https://github.com/manticoresoftware/manticoresearch-buddy/issues/274) 修复了对 Buddy 的并发请求处理不正确的问题。

### 与 manticore-backup 相关
* [问题 #97](https://github.com/manticoresoftware/manticoresearch-backup/pull/97) 在可用时将 VIP HTTP 端口作为默认端口。
各种改进：改进版本检查和流式 ZSTD 解压；在恢复过程中针对版本不匹配添加用户提示；修复恢复时不同版本提示行为不正确的问题；增强了解压缩逻辑，直接从流读取而非先读入工作内存；新增 `--force` 标志。
* [提交 3b35](https://github.com/manticoresoftware/manticoresearch-backup/commit/3b357ea) Manticore search 启动后添加备份版本显示，以便在此阶段识别问题。
* [提交 ad2e](https://github.com/manticoresoftware/manticoresearch-backup/commit/ad2e5da) 更新连接守护进程失败时的错误消息。
* [提交 ce5e](https://github.com/manticoresoftware/manticoresearch-backup/commit/ce5ea85) 修正将绝对根备份路径转换为相对路径的问题，移除恢复时的可写检查以允许从不同路径恢复。
* [提交 db7e](https://github.com/manticoresoftware/manticoresearch-backup/commit/db7e2b9) 新增文件迭代器排序以确保各种情况下的一致性。
* [问题 #106](https://github.com/manticoresoftware/manticoresearch-backup/issues/105) 多配置的备份和恢复。
* [问题 #91](https://github.com/manticoresoftware/manticoresearch-backup/pull/91) 添加 defattr 防止 RHEL 上安装后文件出现异常用户权限。
* [问题 #91](https://github.com/manticoresoftware/manticoresearch-backup/pull/91) 增加额外 chown 操作确保 Ubuntu 上文件默认归 root 用户所有。

### 与 MCL（列式、二级索引、knn 库）相关
* [提交 f104](https://github.com/manticoresoftware/columnar/commit/f104a4709bbb144140d09a09bc0fc1db1895e331) 支持向量搜索。
* [提交 2169](https://github.com/manticoresoftware/columnar/commit/216953beef6676f7e34c9f6f1a30c5980d746710) 修复在中断的二级索引构建设置期间清理临时文件的问题。解决了守护程序创建 `tmp.spidx` 文件时超过打开文件限制的问题。
* [提交 709b](https://github.com/manticoresoftware/columnar/commit/709b9acaaac97d9a1ca8796892f9ad432021c785) 为列式和二级索引使用独立的 streamvbyte 库。
* [提交 1c26](https://github.com/manticoresoftware/manticoresearch/commit/1c2623763ced14bb94ad624dde9614a938408d89) 添加警告，指列式存储不支持 json 属性。
* [提交 3acd](https://github.com/manticoresoftware/columnar/commit/3acd6714ffce103447d7931dfc75bb1a7449f340) 修正二级索引中的数据解包。
* [提交 574c](https://github.com/manticoresoftware/manticoresearch/commit/574c023152564fa72092c18ebd1f594abfd4b6d7) 修复混合行存储和列存储保存磁盘块时的崩溃。
* [提交 e87f](https://github.com/manticoresoftware/columnar/commit/e87f088b458d4c5b1b6159fc72ed0d57b786891a) 修复二级索引迭代器被提示在已经处理的块上。
* [问题 #1474](https://github.com/manticoresoftware/manticoresearch/issues/1474) 使用列式引擎时，行向多值属性更新功能失效。
* [问题 #1510](https://github.com/manticoresoftware/manticoresearch/issues/1510) 修复在 `HAVING` 中聚合列式属性时崩溃。
* [问题 #1519](https://github.com/manticoresoftware/manticoresearch/issues/1519) 修复在 `expr` 排名器中使用列式属性时的崩溃。

### 与 Docker 相关
* ❗[问题 #42](https://github.com/manticoresoftware/docker/issues/42) 支持通过环境变量进行[纯索引化](https://github.com/manticoresoftware/docker#building-plain-tables)。
* ❗[问题 #47](https://github.com/manticoresoftware/docker/issues/47) 通过环境变量改善配置灵活性。
* [问题 #54](https://github.com/manticoresoftware/docker/pull/54) 改进 Docker 的[备份和恢复](https://github.com/manticoresoftware/docker#backup-and-restore)流程。
* [问题 #77](https://github.com/manticoresoftware/docker/issues/76) 改进入口点，仅在首次启动时处理备份恢复。
* [提交 a27c](https://github.com/manticoresoftware/docker/commit/a27c048dd9e73d8d6bea8d9a4830cafb486b82c5) 修复了查询日志输出到 stdout 的问题。
* [问题 #38](https://github.com/manticoresoftware/docker/issues/38) 当未设置 EXTRA 时，静音 BUDDY 警告。
* [问题 #71](https://github.com/manticoresoftware/docker/pull/71) 修复了 `manticore.conf.sh` 中的主机名。

## 版本 6.2.12
发布：2023年8月23日

版本 6.2.12 继续 6.2 系列，解决了 6.2.0 版本发布后发现的问题。

### Bug 修复
* ❗[问题 #1351](https://github.com/manticoresoftware/manticoresearch/issues/1351) “Manticore 6.2.0 无法通过 systemctl 在 Centos 7 上启动”：将 `TimeoutStartSec` 从 `infinity` 修改为 `0`，以提高与 Centos 7 的兼容性。
* ❗[问题 #1364](https://github.com/manticoresoftware/manticoresearch/issues/1364) “从 6.0.4 升级到 6.2.0 后崩溃”：为来自较旧 binlog 版本的空 binlog 文件添加了重放功能。
* [PR #1334](https://github.com/manticoresoftware/manticoresearch/pull/1334) “修正 searchdreplication.cpp 中的拼写错误”：纠正了 `searchdreplication.cpp` 中的拼写错误：beggining -> beginning。
* [问题 #1337](https://github.com/manticoresoftware/manticoresearch/issues/1337) “Manticore 6.2.0 警告：conn (local)(12), sock=8088: 放弃失败的 MySQL 头部，AsyncNetInputBuffer_c::AppendData: 错误 11（资源临时不可用）返回 -1”：降低了 MySQL 接口关于头部警告的详细级别，改为 logdebugv。
* [问题 #1355](https://github.com/manticoresoftware/manticoresearch/issues/1355) “当 node_address 无法解析时加入集群挂起”：改进了当某些节点不可达且其名称解析失败时的复制重试机制。此修复解决了 Kubernetes 和 Docker 节点中与复制相关的问题。增强了复制启动失败的错误信息，并更新了测试模型 376。此外，为名称解析失败提供了清晰的错误信息。
* [问题 #1361](https://github.com/manticoresoftware/manticoresearch/issues/1361) “charset non_cjk 中 'Ø' 无小写映射”：调整了字符 'Ø' 的映射。
* [问题 #1365](https://github.com/manticoresoftware/manticoresearch/issues/1365) “searchd 在干净停止后留下 binlog.meta 和 binlog.001”：确保最后一个空 binlog 文件被正确删除。
* [提交 0871](https://github.com/manticoresoftware/manticoresearch/commit/0871070845e93ad702ae0dfb89b35e5f24cd0851)：修复了 Windows 上与原子复制限制相关的 `Thd_t` 构建问题。
* [提交 1cc0](https://github.com/manticoresoftware/manticoresearch/commit/1cc04bedc174737118fd5a3663e35f4e95bd4f8c)：解决了 FT CBO 与 `ColumnarScan` 的问题。
* [提交 c6bf](https://github.com/manticoresoftware/manticoresearch/commit/c6bfb30d53e3f4b67f17769667733677a661c95b)：修正了测试 376，并为测试中 `AF_INET` 错误添加了替代方案。
* [提交 cbc3](https://github.com/manticoresoftware/manticoresearch/commit/cbc38705e0b4dcc9665e3e8cf64826290ebd11cf)：解决了复制过程中更新 blob 属性与替换文档时的死锁问题，同时因索引已在更基础级别锁定，故移除了提交时的 rlock。

### 小改动
* [提交 4f91](https://github.com/manticoresoftware/manticoresearch/commit/4f913a36e84e01f950b2797051222b0b1094d6fe) 更新了手册中 `/bulk` 端点的说明。

### MCL
* 支持 [Manticore Columnar Library](https://github.com/manticoresoftware/columnar) v2.2.4

## 版本 6.2.0
发布：2023年8月4日

### 重大变更
* 查询优化器增强，支持全文查询，显著提升了搜索效率和性能。
* 集成：
  - [mysqldump](https://dev.mysql.com/doc/refman/8.0/en/mysqldump.html) — 使用 `mysqldump` 实现[逻辑备份](Securing_and_compacting_a_table/Backup_and_restore.md#Backup-and-restore-with-mysqldump)
  - [Apache Superset](https://manticoresearch.com/blog/manticoresearch-apache-superset-integration/) 和 [Grafana](https://manticoresearch.com/blog/manticoresearch-grafana-integration/) 用于可视化存储在 Manticore 中的数据
  - [HeidiSQL](https://www.heidisql.com/) 和 [DBForge](https://www.devart.com/dbforge/) 以简化 Manticore 的开发
* 我们开始使用 [GitHub 工作流](https://github.com/manticoresoftware/manticoresearch/actions)，使贡献者更容易采用核心团队准备包时使用的同一持续集成（CI）流程。所有任务都可在 GitHub 托管的运行器上执行，方便在您的 Manticore Search 分支中无缝测试更改。
* 我们开始使用 [CLT](https://github.com/manticoresoftware/clt) 测试复杂场景。例如，我们现在能够确保在提交后构建的包能够在所有支持的 Linux 操作系统上正确安装。命令行测试器（CLT）提供了交互式记录测试及无缝重放的用户友好方式。
* 通过结合哈希表和 HyperLogLog，大幅提升了 count distinct 操作的性能。
* 启用了包含二级索引的查询的多线程执行，线程数限制为物理 CPU 核心数。此举应显著提高查询执行速度。
* `pseudo_sharding` 调整为限制在空闲线程数以内。此更新显著提升了吞吐性能。
* 用户现在可以通过配置设置指定[默认属性存储引擎](Server_settings/Searchd.md#engine)，提供了更好的定制化以匹配特定工作负载需求。
* 支持 [Manticore Columnar Library 2.2.0](https://github.com/manticoresoftware/columnar/) ，并在[二级索引](Server_settings/Searchd.md#secondary_indexes)中做了众多错误修复和改进。

### 小改动
* [Buddy #153](https://github.com/manticoresoftware/manticoresearch-buddy/issues/153): [/pq](Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md?client=JSON#Adding-rules-to-a-percolate-table) HTTP端点现在作为`/json/pq` HTTP端点的别名。
* [Commit 0bf1](https://github.com/manticoresoftware/manticoresearch/commit/0bf17d9e): 我们已经确保了`upper()`和`lower()`的多字节兼容性。
* [Commit 2bb9](https://github.com/manticoresoftware/manticoresearch/commit/2bb92765): 对于`count(*)`查询，不再扫描索引，而是返回预计算的值。
* [Commit 3c84](https://github.com/manticoresoftware/manticoresearch/commit/3c84091f): 现在可以使用`SELECT`进行任意计算和显示`@@sysvars`。不同于以往，现在不再限制只能进行一次计算。因此，像`select user(), database(), @@version_comment, version(), 1+1 as a limit 10`这样的查询将返回所有列。请注意，可选的`limit`将始终被忽略。
* [Commit 6aca](https://github.com/manticoresoftware/manticoresearch/commit/6aca32f0): 实现了`CREATE DATABASE`的存根查询。
* [Commit 9dc1](https://github.com/manticoresoftware/manticoresearch/commit/9dc12334): 执行`ALTER TABLE table REBUILD SECONDARY`时，即使属性未更新，二级索引现在也总是会被重建。
* [Commit 46ed](https://github.com/manticoresoftware/manticoresearch/commit/46edb089): 使用预计算数据的排序器现在在使用CBO之前被识别，以避免不必要的CBO计算。
* [Commit 102a](https://github.com/manticoresoftware/manticoresearch/commit/102ac604): 实现了全文表达式栈的模拟和利用，以防止守护进程崩溃。
* [Commit 979f](https://github.com/manticoresoftware/manticoresearch/commit/979fa27c): 为不使用字符串/mvas/json属性的匹配克隆代码添加了快速路径。
* [Commit a073](https://github.com/manticoresoftware/manticoresearch/commit/a0735ffe): 添加了对`SELECT DATABASE()`命令的支持。但它将始终返回`Manticore`。此添加对于与各种MySQL工具的集成至关重要。
* [Commit bc04](https://github.com/manticoresoftware/manticoresearch/commit/bc04908): 修改了[/cli](Connecting_to_the_server/HTTP.md#/cli)端点的响应格式，并添加了`/cli_json`端点以实现之前`/cli`的功能。
* [Commit d70b](https://github.com/manticoresoftware/manticoresearch/commit/d70b0d58): 现在可以通过`SET`语句在运行时更改`thread_stack`。提供了会话本地和守护进程全局两种变体。当前值可以在`show variables`输出中查看。
* [Commit d96e](https://github.com/manticoresoftware/manticoresearch/commit/d96ec6b9): 集成代码到CBO以更准确地估计字符串属性过滤器的执行复杂度。
* [Commit e77d](https://github.com/manticoresoftware/manticoresearch/commit/e77dd72f5a04531c352fad0d7afcd2a1cbae2510): 改进了DocidIndex成本计算，提升了整体性能。
* [Commit f3ae](https://github.com/manticoresoftware/manticoresearch/commit/f3ae8bea): 负载指标（类似Linux中的'uptime'）现在可以在`SHOW STATUS`命令中查看。
* [Commit f3cc](https://github.com/manticoresoftware/manticoresearch/commit/f3cc0971): `DESC`和`SHOW CREATE TABLE`的字段及属性顺序现在与`SELECT * FROM`一致。
* [Commit f3d2](https://github.com/manticoresoftware/manticoresearch/commit/f3d248a6): 不同的内部解析器现在在发生各种错误时提供其内部助记码（例如`P01`）。该增强有助于识别导致错误的解析器，同时掩盖非必要的内部细节。
* [Issue #271](https://github.com/manticoresoftware/manticoresearch/issues/271) “有时CALL SUGGEST不会为单字母拼写错误建议更正”：改进了[SUGGEST/QSUGGEST](Searching/Spell_correction.md#CALL-QSUGGEST,-CALL-SUGGEST)对短词的行为：添加了选项`sentence`以显示整句。
* [Issue #696](https://github.com/manticoresoftware/manticoresearch/issues/696) “当启用词干时，Percolate索引无法通过精确短语查询正确搜索”：修改了Percolate查询以处理精确词项修饰符，提升了搜索功能。
* [Issue #829](https://github.com/manticoresoftware/manticoresearch/issues/829) “日期格式化方法”：添加了[date_format()](Functions/Date_and_time_functions.md#DATE_FORMAT())选择列表表达式，暴露了`strftime()`函数。
* [Issue #961](https://github.com/manticoresoftware/manticoresearch/issues/961) “通过HTTP JSON API排序桶”：在HTTP接口中为每个聚合桶引入了可选的[sort属性](Searching/Faceted_search.md#HTTP-JSON)。
* [Issue #1062](https://github.com/manticoresoftware/manticoresearch/issues/1062) “改进JSON插入API失败的错误日志 - "不支持的值类型"”：当出现错误时，`/bulk`端点会报告已处理和未处理字符串（文档）的数量信息。
* [Issue #1070](https://github.com/manticoresoftware/manticoresearch/issues/1070) “CBO提示不支持多个属性”：使索引提示支持多个属性。
* [Issue #1106](https://github.com/manticoresoftware/manticoresearch/issues/1106) “为HTTP搜索查询添加标签”：已向[HTTP PQ响应](Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md?client=JSON#Adding-rules-to-a-percolate-table)添加标签。
* [Issue #1301](https://github.com/manticoresoftware/manticoresearch/issues/1301) “buddy不应并行创建表”：解决了导致CREATE TABLE并行操作失败的问题。现在一次只能运行一个`CREATE TABLE`操作。
* [Issue #1303](https://github.com/manticoresoftware/manticoresearch/issues/1303) “添加对列名中@符号的支持”。
* [Issue #1316](https://github.com/manticoresoftware/manticoresearch/issues/1316) “使用 ps=1 查询出租车数据集时速度缓慢”：CBO逻辑已被优化，默认的直方图分辨率设置为8k，以提高对随机分布值属性的准确性。
* [Issue #1317](https://github.com/manticoresoftware/manticoresearch/issues/1317) “修复 hn 数据集上的 CBO 与全文索引冲突”：实现了改进的逻辑，用于确定何时使用位图迭代器交集，何时使用优先队列。
* [Issue #1318](https://github.com/manticoresoftware/manticoresearch/issues/1318) “columnar：将迭代器接口改为单次调用”：列式迭代器现在使用单次 `Get` 调用，替代之前的两步调用 `AdvanceTo` + `Get` 来获取值。
* [Issue #1319](https://github.com/manticoresoftware/manticoresearch/issues/1319) “聚合计算提速（移除 CheckReplaceEntry？）”：从分组排序器中移除了 `CheckReplaceEntry` 调用，以加速聚合函数的计算。
* [Issue #1320](https://github.com/manticoresoftware/manticoresearch/issues/1320) “创建表时 read_buffer_docs/hits 不支持 k/m/g 语法”：`CREATE TABLE` 选项 `read_buffer_docs` 和 `read_buffer_hits` 现已支持 k/m/g 语法。
* 英文、德文和俄文的[语言包](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology)现在可通过在 Linux 上执行命令 `apt/yum install manticore-language-packs` 轻松安装。在 macOS 上，使用命令 `brew install manticoresoftware/tap/manticore-language-packs`。
* 字段和属性顺序现在在 `SHOW CREATE TABLE` 和 `DESC` 操作之间保持一致。
* 执行 `INSERT` 查询时，如果磁盘空间不足，新的 `INSERT` 查询将失败，直到有足够的磁盘空间。
* 新增了 [UINT64()](Functions/Type_casting_functions.md#UINT64%28%29) 类型转换函数。
* `/bulk` 端点现在将空行处理为 [commit](Data_creation_and_modification/Transactions.md#BEGIN,-COMMIT,-and-ROLLBACK) 命令。更多信息请见[这里](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md?client=JSON#Bulk-adding-documents)。
* 已对[无效的索引提示](Searching/Options.md#Query-optimizer-hints)添加警告，提升透明度并允许错误缓解。
* 当使用单一过滤条件的 `count(*)` 时，查询现在会利用二级索引中的预计算数据（如果可用），显著加快查询速度。

### ⚠️ 重大变更
* ⚠️ 6.2.0 版本中创建或修改的表无法被旧版本读取。
* ⚠️ 文档 ID 现在在索引和 INSERT 操作中作为无符号的 64 位整数处理。
* ⚠️ 查询优化器提示的语法已更新。新的格式为 `/*+ SecondaryIndex(uid) */`。请注意旧语法不再支持。
* ⚠️ [Issue #1160](https://github.com/manticoresoftware/manticoresearch/issues/1160)：为防止语法冲突，禁止在表名中使用 `@` 符号。
* ⚠️ 标记为 `indexed` 和 `attribute` 的字符串字段/属性在 `INSERT`、`DESC` 和 `ALTER` 操作中现在视为单个字段。
* ⚠️ [Issue #1057](https://github.com/manticoresoftware/manticoresearch/issues/1057)：MCL 库将不再在不支持 SSE 4.2 的系统上加载。
* ⚠️ [Issue #1143](https://github.com/manticoresoftware/manticoresearch/issues/1143)：[agent_query_timeout](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_query_timeout) 之前失效，现已修复并生效。

### Bug修复
* [Commit 2a6e](https://github.com/manticoresoftware/manticoresearch/commit/2a6ea8f7) “DROP TABLE 时崩溃”：解决了执行 DROP TABLE 语句时，RT 表写操作（优化、磁盘块保存）结束等待时间过长的问题。新增警告，当执行 DROP TABLE 后表目录不为空时提醒用户。
* [Commit 2ebd](https://github.com/manticoresoftware/manticoresearch/commit/2ebd424d)：添加了对缺失的用于多属性分组的列式属性的支持。
* [Commit 3be4](https://github.com/manticoresoftware/manticoresearch/commit/3be4503f752216b37b8cde8e47fd2d77bbce63ae) 解决了因磁盘空间不足导致的崩溃问题，通过正确处理 binlog 写错误避免崩溃。
* [Commit 6adb](https://github.com/manticoresoftware/manticoresearch/commit/6adb0934)：修复了在查询中使用多个列式扫描迭代器（或二级索引迭代器）时偶尔导致的崩溃。
* [Commit 6bd9](https://github.com/manticoresoftware/manticoresearch/commit/6bd9f709)：修复了使用预计算数据的排序器时过滤条件未被移除的问题。
* [Commit 6d03](https://github.com/manticoresoftware/manticoresearch/commit/6d03566a)：更新了 CBO 代码，更好地估计多线程执行的基于行属性过滤器的查询。
* [Commit 6dd3](https://github.com/manticoresoftware/manticoresearch/commit/6dd3964), [Helm #56](https://github.com/manticoresoftware/manticoresearch-helm/issues/56) “Kubernetes 集群中的致命崩溃转储”：修复了 JSON 根对象的错误布隆过滤器；修正了因 JSON 字段过滤导致的守护进程崩溃。
* [Commit 6e1b](https://github.com/manticoresoftware/manticoresearch/commit/6e1b849) 修正了因无效的 `manticore.json` 配置导致的守护进程崩溃。
* [Commit 6fbc](https://github.com/manticoresoftware/manticoresearch/commit/6fbc4189) 修复了 json 范围过滤器，使其支持 int64 类型值。
* [Commit 9c67](https://github.com/manticoresoftware/manticoresearch/commit/9c677426) 修复了 `.sph` 文件在 `ALTER` 操作时可能损坏的问题。
* [Commit 77cc](https://github.com/manticoresoftware/manticoresearch/commit/77ccd8f): 添加了一个共享密钥，用于替换语句的复制，以解决当替换操作从多个主节点复制时发生的 `pre_commit` 错误。
* [Commit 2884](https://github.com/manticoresoftware/manticoresearch/commit/2884519e) 解决了对像 'date_format()' 这类函数进行 bigint 校验的问题。
* [Commit 9513](https://github.com/manticoresoftware/manticoresearch/commit/9513aca5): 当排序器使用预计算数据时，迭代器不会再显示在 [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META) 中。
* [Commit a2a7](https://github.com/manticoresoftware/manticoresearch/commit/a2a70555): 更新了全文节点堆栈大小，防止复杂全文查询导致的崩溃。
* [Commit a062](https://github.com/manticoresoftware/manticoresearch/commit/a0626d7e): 解决了在复制带有 JSON 和字符串属性的更新时导致崩溃的错误。
* [Commit b3e6](https://github.com/manticoresoftware/manticoresearch/commit/b3e63e5d): 更新了字符串构建器以使用 64 位整数，避免处理大型数据集时的崩溃。
* [Commit c472](https://github.com/manticoresoftware/manticoresearch/commit/c472e5b): 解决了多个索引上的 count distinct 导致的崩溃问题。
* [Commit d073](https://github.com/manticoresoftware/manticoresearch/commit/d0730272): 修复了在禁用 `pseudo_sharding` 时，针对 RT 索引的磁盘块进行查询时，可被多线程执行的问题。
* [Commit d205](https://github.com/manticoresoftware/manticoresearch/commit/d205508e) 修改了 `show index status` 命令返回值的集合，现在根据所使用的索引类型有所不同。
* [Commit e9bc](https://github.com/manticoresoftware/manticoresearch/commit/e9bc2f9) 修复了处理批量请求时的 HTTP 错误，以及错误未能从网络循环返回客户端的问题。
* [Commit f77c](https://github.com/manticoresoftware/manticoresearch/commit/f77ce0e6) 对 PQ 使用了扩展栈。
* [Commit fac2](https://github.com/manticoresoftware/manticoresearch/commit/fac2d9687a20582765aaa89be9597804c4f845af) 更新了导出排序器的输出，以与 [packedfactors()](Functions/Searching_and_ranking_functions.md#PACKEDFACTORS%28%29) 保持一致。
* [Commit ff87](https://github.com/manticoresoftware/manticoresearch/commit/ff87cc4): 修复了 SphinxQL 查询日志中过滤器中的字符串列表问题。
* [Issue #589](https://github.com/manticoresoftware/manticoresearch/issues/589) “字符集定义似乎依赖于代码的顺序”：修正了重复项的错误字符集映射。
* [Issue #811](https://github.com/manticoresoftware/manticoresearch/issues/811) “词形映射多个词干干扰带 CJK 标点符号的短语搜索”：修复了带词形变换时 ngram 令牌位置在短语查询中的问题。
* [Issue #834](https://github.com/manticoresoftware/manticoresearch/issues/834) “搜索查询中的等号导致请求失败”：确保了可对精确符号进行转义，并修复了 `expand_keywords` 选项导致的双重精确扩展问题。
* [Issue #864](https://github.com/manticoresoftware/manticoresearch/issues/864) “异常/停用词冲突”
* [Issue #910](https://github.com/manticoresoftware/manticoresearch/issues/910) “使用 libstemmer_fr 和 index_exact_words 调用 call snippets() 时 Manticore 崩溃”：解决了调用 `SNIPPETS()` 时导致崩溃的内部冲突。
* [Issue #946](https://github.com/manticoresoftware/manticoresearch/issues/946) “SELECT 中的重复记录”：修复了在带有 `not_terms_only_allowed` 选项且 RT 索引中包含已删除文档的查询结果中出现重复文档的问题。
* [Issue #967](https://github.com/manticoresoftware/manticoresearch/issues/967) “在 UDF 函数中使用 JSON 参数导致崩溃”：修复了处理启用伪分片和带 JSON 参数的 UDF 搜索时的守护进程崩溃。
* [Issue #1050](https://github.com/manticoresoftware/manticoresearch/issues/1050) “FEDERATED 中的 count(*)”：修复了通过 `FEDERATED` 引擎带聚合的查询导致守护进程崩溃的问题。
* [Issue #1052](https://github.com/manticoresoftware/manticoresearch/issues/1052) 修复了 `rt_attr_json` 列与列存储不兼容的问题。
* [Issue #1072](https://github.com/manticoresoftware/manticoresearch/issues/1072) “ignore_chars 删除搜索查询中的 *”：修复了该问题，使查询中的通配符不受 `ignore_chars` 影响。
* [Issue #1075](https://github.com/manticoresoftware/manticoresearch/issues/1075) “当存在分布式表时 indextool --check 失败”：indextool 现在兼容 JSON 配置中带有 'distributed' 和 'template' 索引的实例。
* [Issue #1081](https://github.com/manticoresoftware/manticoresearch/issues/1081) “针对特定 RT 数据集的特定 SELECT 导致 searchd 崩溃”：解决了带 packedfactors 和大型内部缓冲区的查询导致守护进程崩溃的问题。
* [Issue #1095](https://github.com/manticoresoftware/manticoresearch/issues/1095) “带 not_terms_only_allowed 时忽略已删除文档”
* [Issue #1099](https://github.com/manticoresoftware/manticoresearch/issues/1099) “indextool --dumpdocids 不工作”：恢复了 `--dumpdocids` 命令的功能。
* [Issue #1100](https://github.com/manticoresoftware/manticoresearch/issues/1100) “indextool --buildidf 不工作”：indextool 现在在完成 globalidf 后关闭文件。
* [Issue #1104](https://github.com/manticoresoftware/manticoresearch/issues/1104) “Count(*) 试图被当作远程表的模式集处理”：解决了在代理返回空结果集时，分布式索引查询中守护进程发送错误消息的问题。
* [Issue #1109](https://github.com/manticoresoftware/manticoresearch/issues/1109) “当 threads=1 时刷新属性（FLUSH ATTRIBUTES）挂起”。
* [Issue #1126](https://github.com/manticoresoftware/manticoresearch/issues/1126) “查询过程中与 MySQL 服务器失去连接 - manticore 6.0.5”：解决了在列式属性上使用多个过滤器时发生的崩溃问题。
* [Issue #1135](https://github.com/manticoresoftware/manticoresearch/issues/1135) “JSON 字符串过滤的大小写敏感性”：修正了排序规则，使其能正确处理 HTTP 搜索请求中的过滤条件。
* [Issue #1140](https://github.com/manticoresoftware/manticoresearch/issues/1140) “错误字段中的匹配”：修复了与 `morphology_skip_fields` 相关的损坏问题。
* [Issue #1155](https://github.com/manticoresoftware/manticoresearch/issues/1155) “通过 API 执行系统远程命令应传递 g_iMaxPacketSize”：更新了跨节点复制命令时绕过 `max_packet_size` 检查的机制。此外，最新的集群错误已被添加到状态显示中。
* [Issue #1302](https://github.com/manticoresoftware/manticoresearch/issues/1302) “优化失败时留下的临时文件”：修正了合并或优化过程出错后遗留临时文件的问题。
* [Issue #1304](https://github.com/manticoresoftware/manticoresearch/issues/1304) “为 buddy 启动超时添加环境变量”：添加了环境变量 `MANTICORE_BUDDY_TIMEOUT`（默认 3 秒），用于控制守护进程启动时等待 buddy 消息的时长。
* [Issue #1305](https://github.com/manticoresoftware/manticoresearch/issues/1305) “保存 PQ 元数据时的整型溢出”：缓解了守护进程在保存大型 PQ 索引时的过度内存消耗问题。
* [Issue #1306](https://github.com/manticoresoftware/manticoresearch/issues/1306) “修改其外部文件后无法重建 RT 表”：修复了使用空字符串修改外部文件时的错误；修正了修改外部文件后遗留的 RT 索引外部文件问题。
* [Issue #1307](https://github.com/manticoresoftware/manticoresearch/issues/1307) “SELECT 语句 sum(value) 作为 value 不能正确工作”：修正了使用别名的选择列表表达式可能隐藏索引属性的问题；同时修复了对整数使用 sum 时应转为 int64 计数的错误。
* [Issue #1308](https://github.com/manticoresoftware/manticoresearch/issues/1308) “避免在复制中绑定到 localhost”：确保在主机名有多个 IP 地址时，复制不会绑定到 localhost。
* [Issue #1309](https://github.com/manticoresoftware/manticoresearch/issues/1309) “返回给 MySQL 客户端的数据超过 16Mb 失败”：修复了向客户端返回超过 16Mb 的 SphinxQL 数据包的问题。
* [Issue #1310](https://github.com/manticoresoftware/manticoresearch/issues/1310) “‘指向外部文件的路径应为绝对路径’中的错误引用”：修正了 `SHOW CREATE TABLE` 中外部文件完整路径的显示。
* [Issue #1311](https://github.com/manticoresoftware/manticoresearch/issues/1311) “调试版本在片段处理中长字符串导致崩溃”：现在允许 `SNIPPET()` 函数处理的文本中包含超过 255 个字符的长字符串。
* [Issue #1312](https://github.com/manticoresoftware/manticoresearch/issues/1312) “在 kqueue 轮询中使用后删除导致的偶发崩溃（master-agent）”：修复了在基于 kqueue 的系统（FreeBSD、MacOS 等）上主节点无法连接到代理时的崩溃。
* [Issue #1313](https://github.com/manticoresoftware/manticoresearch/issues/1313) “连接自身超时过长”：在 MacOS/BSD 上从主节点连接到代理时，使用统一的连接+查询超时，而不仅仅是连接超时。
* [Issue #1314](https://github.com/manticoresoftware/manticoresearch/issues/1314) “pq（json 元数据）中未达到的嵌入式同义词导致加载失败”：修复了 pq 中的嵌入同义词标记。
* [Issue #1315](https://github.com/manticoresoftware/manticoresearch/issues/1315) “允许某些函数（sint、fibonacci、second、minute、hour、day、month、year、yearmonth、yearmonthday）使用隐式提升的参数值”。
* [Issue #1321](https://github.com/manticoresoftware/manticoresearch/issues/1321) “在 fullscan 中启用多线程的二级索引，但限制线程数”：在 CBO 中实现了代码，使其能更好地预测在全文查询中使用二级索引时的多线程性能。
* [Issue #1322](https://github.com/manticoresoftware/manticoresearch/issues/1322) “使用预计算排序器后 count(*) 查询依然缓慢”：当使用预计算数据的排序器时，不再启动迭代器，以避免性能下降。
* [Issue #1411](https://github.com/manticoresoftware/manticoresearch/issues/1141) “sphinxql 中的查询日志未保留 MVA 的原始查询”：现在，`all()/any()` 被正确记录。

## 版本 6.0.4
发布时间：2023 年 3 月 15 日

### 新功能
* 改进了与 Logstash、Beats 等的集成，包括：
  - 支持 Logstash 版本 7.6 - 7.15，Filebeat 版本 7.7 - 7.12
  - 支持自动模式。
  - 添加了对 Elasticsearch 风格批量请求的处理。
* [Buddy 提交 ce90](https://github.com/manticoresoftware/manticoresearch-buddy/commit/ce907ea) 在 Manticore 启动时记录 Buddy 版本。

### Bug 修复
* [Issue #588](https://github.com/manticoresoftware/manticoresearch/issues/588)、[Issue #942](https://github.com/manticoresoftware/manticoresearch/issues/942) 修复了 bigram 索引中搜索元数据与调用关键字的不正确字符。
* [Issue #1027](https://github.com/manticoresoftware/manticoresearch/issues/1027) 拒绝小写的 HTTP 头。
* ❗[Issue #1039](https://github.com/manticoresoftware/manticoresearch/issues/1039) 修复了守护进程读取 Buddy 控制台输出时的内存泄漏。
* [Issue #1056](https://github.com/manticoresoftware/manticoresearch/issues/1056) 修正了问号的异常行为。
* [Issue #1064](https://github.com/manticoresoftware/manticoresearch/issues/1064) 修复了小写表中存在的竞争条件导致的崩溃。
* [Commit 59bb](https://github.com/manticoresoftware/manticoresearch/commit/59bb54c) 修正了 JSON 接口中，id 显式设置为 null 的文档批量写入处理问题。
* [提交 7b6b](https://github.com/manticoresoftware/manticoresearch/commit/7b6b25f) 修复了 CALL KEYWORDS 中多个相同词项的词汇统计问题。
* [提交 f381](https://github.com/manticoresoftware/manticoresearch/commit/f381ad2) Windows 安装程序现在会创建默认配置；运行时不再替换路径。
* [提交 6940](https://github.com/manticoresoftware/manticoresearch/commit/6940e95), [提交 cc5a](https://github.com/manticoresoftware/manticoresearch/commit/cc5a480) 修复了多网络节点集群的复制问题。
* [提交 4972](https://github.com/manticoresoftware/manticoresearch/commit/49722ab) 修复了 `/pq` HTTP 端点，使其成为 `/json/pq` HTTP 端点的别名。
* [提交 3b53](https://github.com/manticoresoftware/manticoresearch/commit/3b5385a) 修复了 Buddy 重启时的守护进程崩溃。
* [Buddy 提交 fba9](https://github.com/manticoresoftware/manticoresearch-buddy/commit/fba9c8c) 在接收到无效请求时显示原始错误。
* [Buddy 提交 db95](https://github.com/manticoresoftware/manticoresearch-buddy/commit/db9532c) 允许备份路径中包含空格，并对正则表达式进行了一些魔法处理以支持单引号。

## 版本 6.0.2
发布日期：2023年2月10日

### Bug修复
* [问题 #1024 crash 2](https://github.com/manticoresoftware/manticoresearch/issues/1024) 在大量结果的 Facet 搜索时崩溃/段错误
* ❗[问题 #1029](https://github.com/manticoresoftware/manticoresearch/issues/1029) - 警告：编译时内置值 KNOWN_CREATE_SIZE (16) 小于测量值 (208)。建议修正此值！
* ❗[问题 #1032](https://github.com/manticoresoftware/manticoresearch/issues/1032) - Manticore 6.0.0 普通索引崩溃
* ❗[问题 #1033](https://github.com/manticoresoftware/manticoresearch/issues/1033) - 多个分布式索引在守护进程重启后丢失
* ❗[问题 #1064](https://github.com/manticoresoftware/manticoresearch/issues/1064) - 分词小写表中的竞态条件

## 版本 6.0.0
发布日期：2023年2月7日

从此版本开始，Manticore Search 配备了 Manticore Buddy，这是一个用 PHP 编写的 sidecar 守护进程，处理无需超级低延迟或高吞吐量的高阶功能。Manticore Buddy 在后台运行，你甚至可能感觉不到它的存在。虽然对最终用户是透明的，但为了让 Manticore Buddy 易于安装并与基于 C++ 的主守护进程兼容，团队付出了巨大努力。此重大变更将允许团队开发各种新的高阶功能，如分片编排、访问控制和身份验证，以及 mysqldump、DBeaver、Grafana mysql 连接器等多种集成。目前它已支持 [SHOW QUERIES](Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES)、[BACKUP](Securing_and_compacting_a_table/Backup_and_restore.md#BACKUP-SQL-command-reference) 和 [自动模式](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema)。

此版本还包括 130 多项缺陷修复和众多新功能，其中许多可视为重大更新。

### 重大变更
* 🔬 实验性功能：现在可以执行兼容 Elasticsearch 的 [insert](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md?example=Elasticsearch#Adding-documents-to-a-real-time-table) 和 [replace](Data_creation_and_modification/Updating_documents/REPLACE.md?example=Elasticsearch#REPLACE) JSON 查询，这使得 Manticore 可用于 Logstash（7.13 版本以前）、Filebeat 及 Beats 家族的其他工具。默认启用。可通过 `SET GLOBAL ES_COMPAT=off` 禁用。
* 支持 [Manticore Columnar Library 2.0.0](https://github.com/manticoresoftware/columnar/)，在[二级索引](Server_settings/Searchd.md#secondary_indexes)方面进行了众多修复和改进。**⚠️ 破坏性变更**：此版本起二级索引默认开启。如果你从 Manticore 5 升级，请确保执行 [ALTER TABLE table_name REBUILD SECONDARY](Updating_table_schema_and_settings.md#Rebuilding-a-secondary-index)。详情见下文。
* [提交 c436](https://github.com/manticoresoftware/manticoresearch/commit/c436f9023536f767610451911955ae36d90aa638) 自动模式：现在可以跳过建表，直接插入首个文档，Manticore 将根据字段自动创建表。详细内容请参见 [这里](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema)。可通过 [searchd.auto_schema](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema) 开关控制。
* 彻底改进了[基于成本的优化器](Searching/Cost_based_optimizer.md)，在许多场景下降低查询响应时间。
  - [问题 #1008](https://github.com/manticoresoftware/manticoresearch/issues/1008) CBO 中的并行性能估计。
  - [问题 #1014](https://github.com/manticoresoftware/manticoresearch/issues/1014) CBO现能识别[二级索引](Server_settings/Searchd.md#secondary_indexes)，行为更智能。
  - [提交 cef9](https://github.com/manticoresoftware/columnar/commit/cef9b54dc3a4ee9138ebc4df3a671715b996d6b3) 列存表/字段的编码统计式存储于元数据中，以帮助 CBO 做出更智能决策。
  - [提交 2b95](https://github.com/manticoresoftware/manticoresearch/commit/2b95fb8c2ba422c9c5a66e2b61c1f7922f31c7aa) 添加了用于微调 CBO 行为的提示。
* [遥测](Telemetry.md#Telemetry)：我们很高兴在此版本引入遥测功能。该功能允许我们收集匿名且去标识化的指标，有助于提升产品性能和用户体验。请放心，所有采集数据**完全匿名且不会关联任何个人信息**。该功能可在设置中[轻松关闭](Telemetry.md#Telemetry)，如有需求。
* [Commit 5aaf](https://github.com/manticoresoftware/manticoresearch/commit/5aafb17d81b601e399e212a344a4738f65576cdd) [ALTER TABLE table_name REBUILD SECONDARY](Updating_table_schema_and_settings.md#Rebuilding-a-secondary-index) 用于在任何时候重建二级索引，例如：
  - 当你从 Manticore 5 迁移到新版本时，
  - 当你对索引中的属性做了 [UPDATE](Data_creation_and_modification/Updating_documents/UPDATE.md#UPDATE)（即[原位更新，不是替换](Data_creation_and_modification/Updating_documents/REPLACE_vs_UPDATE.md)）
* [Issue #821](https://github.com/manticoresoftware/manticoresearch/issues/821) 新工具 `manticore-backup` 用于 [备份和恢复 Manticore 实例](Securing_and_compacting_a_table/Backup_and_restore.md)
* SQL 命令 [BACKUP](Securing_and_compacting_a_table/Backup_and_restore.md#BACKUP-SQL-command-reference) 用于在 Manticore 内部执行备份。
* SQL 命令 [SHOW QUERIES](Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES) 作为查看正在执行查询（而非线程）的简便方式。
* [Issue #551](https://github.com/manticoresoftware/manticoresearch/issues/551) SQL 命令 `KILL` 用于终止长时间运行的 `SELECT`。
* 针对聚合查询增加了动态 `max_matches` 以提升准确度并降低响应时间。

### 小改动
* [Issue #822](https://github.com/manticoresoftware/manticoresearch/issues/822) SQL 命令 [FREEZE/UNFREEZE](Securing_and_compacting_a_table/Freezing_a_table.md) 用于准备实时或普通表进行备份。
* [Commit c470](https://github.com/manticoresoftware/manticoresearch/commit/c47052f18ad4868134de1a97e8d7da570e531dfa) 新设置 `accurate_aggregation` 和 `max_matches_increase_threshold` 用于控制聚合准确度。
* [Issue #718](https://github.com/manticoresoftware/manticoresearch/issues/718) 支持有符号负64位 ID。注意，你仍然不能使用大于 2^63 的 ID，但现在可以使用 -2^63 到 0 范围内的 ID。
* 由于我们最近增加了对二级索引的支持，“index”一词可能指代二级索引、全文索引或普通/实时 `index`，导致混淆。为减少混淆，我们将后者重命名为“table”。受此影响的 SQL/命令行命令如下。旧版命令仍被支持但不推荐使用：
  - `index <表名>` => `table <表名>`，
  - `searchd -i / --index` => `searchd -t / --table`，
  - `SHOW INDEX STATUS` => `SHOW TABLE STATUS`，
  - `SHOW INDEX SETTINGS` => `SHOW TABLE SETTINGS`，
  - `FLUSH RTINDEX` => `FLUSH TABLE`，
  - `OPTIMIZE INDEX` => `OPTIMIZE TABLE`，
  - `ATTACH TABLE plain TO RTINDEX rt` => `ATTACH TABLE plain TO TABLE rt`，
  - `RELOAD INDEX` => `RELOAD TABLE`，
  - `RELOAD INDEXES` => `RELOAD TABLES`。

  我们不打算废除旧形式，但为了文档兼容，建议在你的应用程序中更改命令名称。未来版本会在各种 SQL 和 JSON 命令的输出中将“index”改为“table”。
* 包含有状态 UDF 的查询现在强制在单线程中执行。
* [Issue #1011](https://github.com/manticoresoftware/manticoresearch/issues/1011) 重构了所有与时间调度相关的部分，为并行分片合并做准备。
* **⚠️ 破坏性变更**：列存储格式已更改，需要重建含列属性的表。
* **⚠️ 破坏性变更**：二级索引文件格式已更改，如果你使用二级索引搜索且配置文件中有 `searchd.secondary_indexes = 1`，新版本 Manticore **将跳过加载含二级索引的表**。建议：
  - 升级前将配置文件中的 `searchd.secondary_indexes` 修改为 0。
  - 启动实例，Manticore 会加载表并给出警告。
  - 对每个索引执行 `ALTER TABLE <表名> REBUILD SECONDARY` 重新构建二级索引。

  如果你运行的是复制集群，需在所有节点执行 `ALTER TABLE <表名> REBUILD SECONDARY`，或者按照[此处说明](Securing_and_compacting_a_table/Compacting_a_table.md#Optimizing-clustered-tables)，但将 `OPTIMIZE` 改成执行 `ALTER .. REBUILD SECONDARY`。
* **⚠️ 破坏性变更**：二进制日志版本已更新，之前版本的 binlog 将不能回放。升级过程中必须确保 Manticore Search 正常停止。也就是说，停止实例后 `/var/lib/manticore/binlog/` 目录下应只有 `binlog.meta`，没有其他 binlog 文件。
* [Issue #849](https://github.com/manticoresoftware/manticoresearch/issues/849) `SHOW SETTINGS`：你现在可从 Manticore 内部查看配置文件的设置。
* [Issue #1007](https://github.com/manticoresoftware/manticoresearch/issues/1007) [SET GLOBAL CPUSTATS=1/0](Server_settings/Setting_variables_online.md#SET) 可开启/关闭 CPU 时间跟踪；[SHOW THREADS](Node_info_and_management/SHOW_THREADS.md) 在 CPU 时间跟踪关闭时将不显示 CPU 统计信息。
* [Issue #1009](https://github.com/manticoresoftware/manticoresearch/issues/1009) RT 表的 RAM 区块段现在可以在 RAM 区块刷新时合并。
* [Issue #1012](https://github.com/manticoresoftware/manticoresearch/issues/1012) 新增二级索引进度显示，可在 [indexer](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) 输出中看到。
* [Issue #1013](https://github.com/manticoresoftware/manticoresearch/issues/1013) 之前如果 Manticore 启动时无法开始服务某表，这个表的记录会从索引列表中移除。新行为是保留记录，尝试在下一次启动时加载。
* [indextool --docextract](Miscellaneous_tools.md#indextool) 返回所有属于请求文档的词和命中。
* [Commit 2b29](https://github.com/manticoresoftware/manticoresearch/commit/2b296ee20e520b85bcbb4383e87095d31e7165dc) 环境变量 `dump_corrupt_meta` 启用在 searchd 无法加载索引时，将损坏的表元数据转储到日志。
* [Commit c7a3](https://github.com/manticoresoftware/manticoresearch/commit/c7a3136eefe0eb2f69e3d99fae9c10beb5821ea6) `DEBUG META` 可以显示 `max_matches` 和伪分片统计信息。
* [Commit 6bca](https://github.com/manticoresoftware/manticoresearch/commit/6bca4848dc5c8396dbc1e057be4b9b2d579794e8) 一个比“Index header format is not json, will try it as binary...”更清晰的错误提示。
* [Commit bef3](https://github.com/manticoresoftware/lemmatizer-uk/commit/bef3ff0386d3ee87ec57619782100972c1122e47) 乌克兰语词形还原器路径已更改。
* [Commit 4ae7](https://github.com/manticoresoftware/manticoresearch/commit/4ae789595329a2951e194d1191ddb3121459a560) 给[SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META)添加了二级索引统计信息。
* [Commit 2e7c](https://github.com/manticoresoftware/manticoresearch/commit/2e7c585e) JSON 接口现在可以通过 Swagger Editor 轻松可视化 https://manual.manticoresearch.com/Openapi#OpenAPI-specification。
* **⚠️ 不兼容变更**：复制协议已更改。如果您运行复制集群，升级到 Manticore 5 时需要：
  - 先干净地停止所有节点
  - 然后用 `--new-cluster` 启动最后停止的节点（Linux 上运行工具 `manticore_new_cluster`）。
  - 详见[重启集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster)。

### 与 Manticore 列存库相关的变更
* 对二级索引与列存储集成进行了重构。
* [Commit efe2](https://github.com/manticoresoftware/columnar/commit/efe26b2a) Manticore 列存库优化，可通过部分预先的最小/最大值评估降低响应时间。
* [Commit 2757](https://github.com/manticoresoftware/columnar/commit/2757b99b) 如果磁盘块合并中断，守护进程现在会清理与 MCL 相关的临时文件。
* [Commit e9c6](https://github.com/manticoresoftware/manticoresearch/commit/e9c672014b63e2b49e7a5a53880583af86c3ab4e) 崩溃时会将列存和二级库版本转储到日志。
* [Commit f5e8](https://github.com/manticoresoftware/columnar/commit/f5e84eeb8b6cb102f13e3468246702427527fef9) 为二级索引添加了快速 doclist 回绕支持。
* [Commit 06df](https://github.com/manticoresoftware/manticoresearch/commit/06dfdd2e668933495dd6cb52d5ba2e0aa9b26e58) 使用 MCL 时，类似 `select attr, count(*) from plain_index`（无过滤）查询现在更快。
* [Commit 0a76](https://github.com/manticoresoftware/manticoresearch/commit/0a76ecb9adbe886868b962dd425de5f475bbf55e) 在 HandleMysqlSelectSysvar 中为兼容 mysql 8.25 以上的 .net 连接器添加了 @@autocommit。
* **⚠️ 不兼容变更**：[MCL Issue #17](https://github.com/manticoresoftware/columnar/issues/17) MCL：为列存扫描添加 SSE 代码。MCL 现至少要求 SSE4.2。

### 与打包相关的变更
* [Commit 4d19](https://github.com/manticoresoftware/manticoresearch/commit/4d19f5cbe49a31228aa09253f061165bfe80e51b) **⚠️ 不兼容变更**：不再支持 Debian Stretch 和 Ubuntu Xenial。
* 支持 RHEL 9，包括 Centos 9、Alma Linux 9 和 Oracle Linux 9。
* [Issue #924](https://github.com/manticoresoftware/manticoresearch/issues/924) 支持 Debian Bookworm。
* [Issue #636](https://github.com/manticoresoftware/manticoresearch/issues/636) 打包：Linux 和 MacOS 的 arm64 构建。
* [PR #26](https://github.com/manticoresoftware/docker/pull/26) 多架构（x86_64 / arm64）docker 镜像。
* [简化贡献者的包构建流程](Installation/Compiling_from_sources.md#Building-using-CI-Docker)。
* 现在可以使用 APT 安装特定版本。
* [Commit a6b8](https://github.com/manticoresoftware/manticoresearch/commit/51fddca5c2a3ebb8576fae4c18660656ba22de0f) Windows 安装程序（之前只提供压缩包）。
* 改为使用 CLang 15 编译。
* **⚠️ 不兼容变更**：自定义 Homebrew 配方，包括 Manticore 列存库配方。安装 Manticore、MCL 及其它必须组件，请使用如下命令 `brew install manticoresoftware/manticore/manticoresearch manticoresoftware/manticore/manticore-extra`。

### Bug 修复
* [Issue #479](https://github.com/manticoresoftware/manticoresearch/issues/479) 名为 `text` 的字段问题
* [Issue #501](https://github.com/manticoresoftware/manticoresearch/issues/501) id 不能是非 bigint 类型
* [Issue #646](https://github.com/manticoresoftware/manticoresearch/issues/646) ALTER 与名为 “text” 的字段冲突
* ❗[Issue #652](https://github.com/manticoresoftware/manticoresearch/issues/652) 可能的 BUG：HTTP（JSON）offset 和 limit 影响 facet 结果
* ❗[Issue #827](https://github.com/manticoresoftware/manticoresearch/issues/827) Searchd 在高强度负载下挂起/崩溃
* ❗[Issue #996](https://github.com/manticoresoftware/manticoresearch/issues/996) PQ 索引内存溢出
* ❗[Commit 1041](https://github.com/manticoresoftware/manticoresearch/commit/10416ef7dddf06c0d759e32ccd6ebaa2468f7cbf) 自 Sphinx 起 `binlog_flush = 1` 一直失效，已修复。
* [MCL Issue #14](https://github.com/manticoresoftware/columnar/issues/14) MCL：当 ft 字段过多时，select 崩溃
* [Issue #470](https://github.com/manticoresoftware/manticoresearch/issues/470) sql_joined_field 不能存储
* [Issue #713](https://github.com/manticoresoftware/manticoresearch/issues/713) 使用 LEVENSHTEIN() 时崩溃
* [Issue #743](https://github.com/manticoresoftware/manticoresearch/issues/743) Manticore 意外崩溃且无法正常重启
* [Issue #788](https://github.com/manticoresoftware/manticoresearch/issues/788) 通过 /sql 调用 KEYWORDS 返回控制字符，导致 json 解析失败
* [Issue #789](https://github.com/manticoresoftware/manticoresearch/issues/789) mariadb 不能创建表 FEDERATED
* [Issue #796](https://github.com/manticoresoftware/manticoresearch/issues/796) 警告：dlopen() 失败：/usr/bin/lib_manticore_columnar.so：无法打开共享对象文件：没有此类文件或目录
* [Issue #797](https://github.com/manticoresoftware/manticoresearch/issues/797) 通过 api 使用 ZONESPAN 搜索时 Manticore 崩溃
* [Issue #799](https://github.com/manticoresoftware/manticoresearch/issues/799) 使用多个索引和 facet distinct 时权重错误
* [Issue #801](https://github.com/manticoresoftware/manticoresearch/issues/801) SQL 组查询在 SQL 索引重新处理后挂起
* [Issue #802](https://github.com/manticoresoftware/manticoresearch/issues/802) MCL：Indexer 在 5.0.2 及 manticore-columnar-lib 1.15.4 中崩溃
* [Issue #813](https://github.com/manticoresoftware/manticoresearch/issues/813) Manticore 5.0.2 FEDERATED 返回空集（MySQL 8.0.28）
* [Issue #824](https://github.com/manticoresoftware/manticoresearch/issues/824) 在 2 个索引上执行 COUNT DISTINCT，当结果为零时抛出内部错误
* [Issue #826](https://github.com/manticoresoftware/manticoresearch/issues/826) 删除查询时崩溃
* [Issue #843](https://github.com/manticoresoftware/manticoresearch/issues/843) MCL：长文本字段的 Bug
* [Issue #856](https://github.com/manticoresoftware/manticoresearch/issues/856) 5.0.2 rtindex：聚合搜索限制行为与预期不符
* [Issue #863](https://github.com/manticoresoftware/manticoresearch/issues/863) 即便是应该返回多个结果的搜索，返回的 Hits 仍然是 Nonetype 对象
* [Issue #870](https://github.com/manticoresoftware/manticoresearch/issues/870) SELECT 表达式中使用 Attribute 和 Stored Field 导致崩溃
* [Issue #872](https://github.com/manticoresoftware/manticoresearch/issues/872) 崩溃后表变为不可见
* [Issue #877](https://github.com/manticoresoftware/manticoresearch/issues/877) 搜索查询中两个负词导致错误：查询不可计算
* [Issue #878](https://github.com/manticoresoftware/manticoresearch/issues/878) 通过 json query_string 查询 a -b -c 不起作用
* [Issue #886](https://github.com/manticoresoftware/manticoresearch/issues/886) pseudo_sharding 与 query match 问题
* [Issue #893](https://github.com/manticoresoftware/manticoresearch/issues/893) Manticore 5.0.2 min/max 函数不能按预期工作...
* [Issue #896](https://github.com/manticoresoftware/manticoresearch/issues/896) 字段 "weight" 解析不正确
* [Issue #897](https://github.com/manticoresoftware/manticoresearch/issues/897) Manticore 服务启动时崩溃并不断重启
* [Issue #900](https://github.com/manticoresoftware/manticoresearch/issues/900) group by j.a, smth 存在错误
* [Issue #913](https://github.com/manticoresoftware/manticoresearch/issues/913) 当排名器中使用 expr 时 Searchd 崩溃，但只针对有两个邻近的查询
* [Issue #916](https://github.com/manticoresoftware/manticoresearch/issues/916) net_throttle_action 失效
* [Issue #919](https://github.com/manticoresoftware/manticoresearch/issues/919) MCL：查询执行时 Manticore 崩溃，集群恢复时也有其他崩溃
* [Issue #925](https://github.com/manticoresoftware/manticoresearch/issues/925) SHOW CREATE TABLE 输出缺少反引号
* [Issue #930](https://github.com/manticoresoftware/manticoresearch/issues/930) 现在可以通过 JDBC 连接器从 Java 查询 Manticore
* [Issue #933](https://github.com/manticoresoftware/manticoresearch/issues/933) bm25f 排名问题
* [Issue #934](https://github.com/manticoresoftware/manticoresearch/issues/934) 无配置索引在 watchdog 的首次加载状态冻结
* [Issue #937](https://github.com/manticoresoftware/manticoresearch/issues/937) 对 facet 数据排序时发生段错误
* [Issue #940](https://github.com/manticoresoftware/manticoresearch/issues/940) CONCAT(TO_STRING) 时崩溃
* [Issue #947](https://github.com/manticoresoftware/manticoresearch/issues/947) 在某些情况下，单个简单 select 可能导致整个实例挂起，无法登录或执行其他查询，直到该 select 结束。
* [Issue #948](https://github.com/manticoresoftware/manticoresearch/issues/948) Indexer 崩溃
* [Issue #950](https://github.com/manticoresoftware/manticoresearch/issues/950) facet distinct 计数错误
* [Issue #953](https://github.com/manticoresoftware/manticoresearch/issues/953) 内置 sph04 排名器中的 LCS 计算错误
* [Issue #955](https://github.com/manticoresoftware/manticoresearch/issues/955) 5.0.3 开发版崩溃
* [Issue #963](https://github.com/manticoresoftware/manticoresearch/issues/963) 使用 json 的 FACET 在引擎 columnar 中崩溃
* [Issue #982](https://github.com/manticoresoftware/manticoresearch/issues/982) MCL：5.0.3 次级索引崩溃
* [PR #984](https://github.com/manticoresoftware/manticoresearch/pull/984) @@autocommit 在 HandleMysqlSelectSysvar 中
* [PR #985](https://github.com/manticoresoftware/manticoresearch/pull/985) 修正 RT 索引中的线程分块分配
* [Issue #985](https://github.com/manticoresoftware/manticoresearch/issues/985) 修正 RT 索引中的线程分块分配
* [Issue #986](https://github.com/manticoresoftware/manticoresearch/issues/986) 错误的默认 max_query_time
* [Issue #987](https://github.com/manticoresoftware/manticoresearch/issues/987) 多线程执行中使用正则表达式时崩溃
* [Issue #988](https://github.com/manticoresoftware/manticoresearch/issues/988) 后向索引兼容性破坏
* [Issue #989](https://github.com/manticoresoftware/manticoresearch/issues/989) indextool 报告检查 columnar 属性时出错
* [Issue #990](https://github.com/manticoresoftware/manticoresearch/issues/990) json grouper 克隆的内存泄漏
* [Issue #991](https://github.com/manticoresoftware/manticoresearch/issues/991) levenshtein 函数克隆的内存泄漏
* [Issue #992](https://github.com/manticoresoftware/manticoresearch/issues/992) 加载元数据时错误信息丢失
* [Issue #993](https://github.com/manticoresoftware/manticoresearch/issues/993) 传播动态索引/子键和系统变量的错误
* [Issue #994](https://github.com/manticoresoftware/manticoresearch/issues/994) 在列式存储中对列式字符串进行count distinct时崩溃
* [Issue #995](https://github.com/manticoresoftware/manticoresearch/issues/995) MCL：taxi1 中 min(pickup_datetime) 导致崩溃
* [Issue #997](https://github.com/manticoresoftware/manticoresearch/issues/997) 空的 excludes JSON 查询会从选择列表中移除列
* [Issue #998](https://github.com/manticoresoftware/manticoresearch/issues/998) 当前调度器上运行的次级任务有时导致异常副作用
* [Issue #999](https://github.com/manticoresoftware/manticoresearch/issues/999) facet distinct 和不同模式导致崩溃
* [Issue #1000](https://github.com/manticoresoftware/manticoresearch/issues/1000) MCL：在未加载列式库的情况下运行后，列式实时索引损坏
* [Issue #1001](https://github.com/manticoresoftware/manticoresearch/issues/1001) JSON中隐式截断不起作用
* [Issue #1002](https://github.com/manticoresoftware/manticoresearch/issues/1002) 列式聚合器问题
* [Issue #1003](https://github.com/manticoresoftware/manticoresearch/issues/1003) 无法从索引中删除最后一个字段
* [Issue #1004](https://github.com/manticoresoftware/manticoresearch/issues/1004) --new-cluster 后行为异常
* [Issue #1005](https://github.com/manticoresoftware/manticoresearch/issues/1005) “没有加载列式库”，但其实不需要加载
* [Issue #1006](https://github.com/manticoresoftware/manticoresearch/issues/1006) 删除查询无错误提示
* [Issue #1010](https://github.com/manticoresoftware/manticoresearch/issues/1010) 修复Windows构建中ICU数据文件位置
* [PR #1018](https://github.com/manticoresoftware/manticoresearch/pull/1018) 握手发送问题
* [Issue #1020](https://github.com/manticoresoftware/manticoresearch/issues/1020) 在 show create table 中显示id
* [Issue #1024 crash 1](https://github.com/manticoresoftware/manticoresearch/issues/1024) Facet搜索返回大量结果时崩溃/段错误。
* [Issue #1026](https://github.com/manticoresoftware/manticoresearch/issues/1026) 实时索引：当插入大量文档且RAM块满时，searchd 永远“卡住”
* [Commit 4739](https://github.com/manticoresoftware/manticoresearch/commit/4739dafd) 当节点间复制忙时，线程在关闭时卡住
* [Commit ab87](https://github.com/manticoresoftware/manticoresearch/commit/ab87836782e7ae43fe5f9dea739eed0d336b42c1) JSON范围过滤器中混合使用浮点数和整数可能导致 Manticore 忽略过滤器
* [Commit d001](https://github.com/manticoresoftware/manticoresearch/commit/d00101c2905f6393ce3fca23c4f6dcb2506f4bd9) JSON中的浮点过滤器不准确
* [Commit 4092](https://github.com/manticoresoftware/manticoresearch/commit/4092c6a8) 如果索引被修改则丢弃未提交事务（否则会崩溃）
* [Commit 9692](https://github.com/manticoresoftware/manticoresearch/commit/9692a840) 使用反斜杠时查询语法错误
* [Commit 0c19](https://github.com/manticoresoftware/manticoresearch/commit/0c19ad8d) SHOW STATUS 中 workers_clients 可能错误
* [Commit 1772](https://github.com/manticoresoftware/manticoresearch/commit/17720e1e2f2ec38337df565bee78f7ce9d9c744a) 修复合并无文档存储的RAM段时的崩溃
* [Commit f45b](https://github.com/manticoresoftware/manticoresearch/commit/f45b337b5c265433bb7d6262ad65d02f65edbaf3) 修复 equals JSON 过滤器漏掉的 ALL/ANY 条件
* [Commit 3e83](https://github.com/manticoresoftware/manticoresearch/commit/3e83c1f0c0e6e7a9d4297113f69a112cbabd9e64) 如果searchd从无写权限的目录启动，复制可能因 `got exception while reading ist stream: mkstemp(./gmb_pF6TJi) failed: 13 (Permission denied)` 失败。
* [Commit 92e5](https://github.com/manticoresoftware/manticoresearch/commit/92e5629f7150f62524b385fca670a01ba9590f5b) 自4.0.2起崩溃日志仅包含偏移量。此提交修复此问题。

## 版本 5.0.2
发布：2022年5月30日

### 错误修复
* ❗[Issue #791](https://github.com/manticoresoftware/manticoresearch/issues/791) - 错误的堆栈大小可能导致崩溃。

## 版本 5.0.0
发布：2022年5月18日


### 主要新功能
* 🔬 支持 [Manticore 列式库 1.15.2](https://github.com/manticoresoftware/columnar/)，启用[二级索引](Server_settings/Searchd.md#secondary_indexes)测试版。对于纯列式及实时列式和行式索引（如果使用了[Manticore 列式库](https://github.com/manticoresoftware/columnar)），默认启用构建二级索引，但要启用搜索需在配置文件或通过[SET GLOBAL](Server_settings/Setting_variables_online.md)设置 `secondary_indexes = 1`。该功能支持除旧版 Debian Stretch 和 Ubuntu Xenial 外的所有操作系统。
* [只读模式](Security/Read_only.md)：现在可以指定仅处理读取查询的监听器，丢弃所有写入。
* 新的 [/cli](Connecting_to_the_server/HTTP.md#/cli) 端点，使通过HTTP运行SQL查询更简单。
* 通过HTTP JSON协议进行更快的批量 INSERT/REPLACE/DELETE：之前多条写入命令逐条处理，现在作为单一事务处理。
* [#720](https://github.com/manticoresoftware/manticoresearch/issues/720) JSON协议支持 [嵌套过滤器](Searching/Filters.md#Nested-bool-query)。之前无法在JSON中表示如 `a=1 且 (b=2 或 c=3)` 的逻辑：`must`（与）、`should`（或）和 `must_not`（非）仅支持顶层。现在支持嵌套。
* 支持 HTTP 协议中的 [分块传输编码 (Chunked transfer encoding)](https://en.wikipedia.org/wiki/Chunked_transfer_encoding)。现在您可以在应用程序中使用分块传输以传输大批量数据时减少资源消耗（因为无需计算 `Content-Length`）。在服务器端，Manticore 现在始终以流式方式处理传入的 HTTP 数据，无需像以前那样等待整个批量传输完成，这：
  - 降低峰值内存使用，减少 OOM（内存溢出）风险
  - 缩短响应时间（我们的测试表明处理 100MB 批量时响应时间减少了 11%）
  - 允许您绕过 [max_packet_size](Server_settings/Searchd.md#max_packet_size)，传输远大于 `max_packet_size`（128MB）最大允许值的批量，比如一次传输 1GB。
* [#719](https://github.com/manticoresoftware/manticoresearch/issues/719) HTTP 接口支持 `100 Continue`：现在您可以使用 `curl`（包括各种编程语言使用的 curl 库）传输大型批量数据，这些工具默认发送 `Expect: 100-continue` 并在实际发送数据前等待一段时间。之前您需要添加 `Expect: ` 头，现在不需要了。

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

* **⚠️ 不兼容变更**：[伪分片 (Pseudo sharding)](Server_settings/Searchd.md#pseudo_sharding) 默认启用。如果您想禁用它，请确保在 Manticore 配置文件的 `searchd` 部分添加 `pseudo_sharding = 0`。
* 在实时/普通索引中至少包含一个全文字段不再是强制的。您现在可以在不涉及全文搜索的情况下使用 Manticore。
* [快速抓取](Creating_a_table/Data_types.md#fast_fetch) 由 [Manticore Columnar Library](https://github.com/manticoresoftware/columnar) 支持的属性：像 `select * from <columnar table>` 这样的查询现在比以前快得多，尤其是在架构中有许多字段的情况下。
* **⚠️ 重大变更**：隐式 [cutoff](Searching/Options.md#cutoff)。Manticore 现在不会花时间和资源处理你不需要包含在结果集中的数据。缺点是它会影响 [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META) 中的 `total_found` 和 JSON 输出中的 [hits.total](Searching/Full_text_matching/Basic_usage.md#HTTP-JSON)。只有当你看到 `total_relation: eq` 时，结果才是准确的，而 `total_relation: gte` 表示实际匹配文档数大于你获得的 `total_found` 值。要保持之前的行为，你可以使用搜索选项 `cutoff=0`，这样 `total_relation` 始终为 `eq`。
* **⚠️ 重大变更**：所有全文字段现在默认是 [存储](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#stored_fields) 的。你需要使用 `stored_fields = `（空值）将所有字段设为非存储（即恢复以前的行为）。
* [#715](https://github.com/manticoresoftware/manticoresearch/issues/715) HTTP JSON 支持 [搜索选项](Searching/Options.md#General-syntax)。

### 小改动
* **⚠️ 重大变更**：索引元文件格式更改。以前的元文件（`.meta`，`.sph`）是二进制格式，现在变为 json。新的 Manticore 版本会自动转换旧索引，但：
  - 你可能会收到类似 `WARNING: ... syntax error, unexpected TOK_IDENT` 的警告
  - 旧版本 Manticore 将无法运行新格式索引，请确保有备份
* **⚠️ 重大变更**：通过 [HTTP keep-alive](Connecting_to_the_server/HTTP.md#Keep-alive) 增加会话状态支持。当客户端也支持时，这使 HTTP 变为有状态。例如，使用新的 [/cli](Connecting_to_the_server/HTTP.md#/cli) 端点和默认开启的 HTTP keep-alive，可以在执行 `SELECT` 后调用 `SHOW META`，行为与通过 mysql 相同。注意，以前虽然支持 `Connection: keep-alive` HTTP 头，但仅仅是复用连接。从此版本开始，它也使会话具有状态。
* 现在你可以在[普通模式](Read_this_first.md#Real-time-mode-vs-plain-mode)中使用 `columnar_attrs = *` 将所有属性定义为列式存储，对于字段很多的情况非常有用。
* 更快的复制 SST
* **⚠️ 重大变更**：复制协议已更改。如果你运行复制集群，升级到 Manticore 5 时需要：
  - 先干净地停止所有节点
  - 然后用 `--new-cluster` 启动最后停止的节点（Linux 系统下运行工具 `manticore_new_cluster`）。
  - 更多详情请阅读 [重启集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster)。
* 复制功能改进：
  - 更快的 SST
  - 噪声抗扰性提升，有助于不稳定网络下的复制节点连接
  - 改进日志记录
* 安全改进：如果配置中未指定 `listen`，Manticore 现在监听 `127.0.0.1` 而不是 `0.0.0.0`。虽然 Manticore Search 默认配置文件有指定 `listen`，且通常不会完全缺少它，但仍然有可能以前监听的是 `0.0.0.0`，这不安全，现在改为监听 `127.0.0.1`，这通常不对外网开放。
* 对列式属性的聚合更快。
* `AVG()` 精度提升：之前 Manticore 聚合使用的是 `float`，现在改为 `double`，显著提高了精度。
* 改进了对 JDBC MySQL 驱动的支持。
* 支持 [jemalloc](https://github.com/jemalloc/jemalloc) 的 `DEBUG malloc_stats`。
* [optimize_cutoff](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#optimize_cutoff) 现可作为每表设置，在 CREATE 或 ALTER 表时设定。
* **⚠️ 重大变更**：默认的 [query_log_format](Server_settings/Searchd.md#query_log_format) 现在是 **`sphinxql`**。如果习惯使用 `plain` 格式，需要在配置文件中添加 `query_log_format = plain`。
* 显著减少内存消耗：在长时间高强度插入/替换/优化操作中，使用存储字段时，Manticore 现在消耗的 RAM 更少。
* [shutdown_timeout](Server_settings/Searchd.md#shutdown_timeout) 默认值从 3 秒增加到 60 秒。
* [提交 ffd0](https://github.com/manticoresoftware/manticoresearch/commit/ffd0499d329d2c383f14c8a44c4cc84338ab56e7) 支持 Java mysql connector >= 6.0.3：在 [Java mysql 连接器 6.0.3](https://mvnrepository.com/artifact/mysql/mysql-connector-java/6.0.3) 中改变了连接 mysql 的方式，导致与 Manticore 不兼容。现在支持新行为。
* [提交 1da6](https://github.com/manticoresoftware/manticoresearch/commit/1da6dbec) 禁止在加载索引（如 searchd 启动时）时保存新的磁盘块。
* [问题 #746](https://github.com/manticoresoftware/manticoresearch/issues/746) 支持 glibc >= 2.34。
* [问题 #784](https://github.com/manticoresoftware/manticoresearch/issues/784) 将“VIP”连接与普通连接区分计数。之前 VIP 连接计入 `max_connections` 限制，可能导致普通连接出现“maxed out”错误。现在 VIP 连接不计入限制。当前 VIP 连接数可通过 `SHOW STATUS` 和 `status` 查看。
* 现在可以显式指定 [ID](Creating_a_table/Data_types.md#Document-ID)。
* [问题 #687](https://github.com/manticoresoftware/manticoresearch/issues/687) 支持 mysql 协议中的 zstd 压缩。

### ⚠️ 其他小的破坏性变更
* ⚠️ BM25F 公式已稍作更新以提升搜索相关性。此更改仅影响使用函数 [BM25F()](Functions/Searching_and_ranking_functions.md#BM25F%28%29) 的搜索结果，不会改变默认排序公式的行为。
* ⚠️ REST [/sql](Connecting_to_the_server/HTTP.md#mode=raw) 端点行为变更：`/sql?mode=raw` 现在需要转义并返回数组。
* ⚠️ `/bulk` INSERT/REPLACE/DELETE 请求响应格式变更：
  - 以前每个子查询构成一个独立事务，返回各自响应
  - 现在整个批次被视为单一事务，返回单一响应
* ⚠️ 搜索选项 `low_priority` 和 `boolean_simplify` 现在需要提供值（`0/1`）：之前可以写成 `SELECT ... OPTION low_priority, boolean_simplify`，现在需写成 `SELECT ... OPTION low_priority=1, boolean_simplify=1`。
* ⚠️ 如果您正在使用旧版 [php](https://github.com/manticoresoftware/manticoresearch/blob/master/api/sphinxapi.php)、[python](https://github.com/manticoresoftware/manticoresearch/blob/master/api/sphinxapi.py) 或 [java](https://github.com/manticoresoftware/manticoresearch/tree/master/api/java) 客户端，请访问相应链接下载更新版本。**旧版本与 Manticore 5 不完全兼容。**
* ⚠️ HTTP JSON 请求在 `query_log_format=sphinxql` 模式下的日志格式发生变化。此前仅记录全文部分，现在按原样记录。

### 新软件包
* **⚠️ 重大变更**：因新结构，升级到 Manticore 5 时建议先卸载旧版软件包再安装新版：
  - 基于 RPM：`yum remove manticore*`
  - Debian 和 Ubuntu：`apt remove manticore*`
* 新的 deb/rpm 软件包结构。之前版本包含：
  - `manticore-server`，含 `searchd`（主搜索守护进程）及相关组件
  - `manticore-tools`，含 `indexer` 和 `indextool`
  - `manticore`，包含全部内容
  - `manticore-all` RPM 作为元包，指向 `manticore-server` 和 `manticore-tools`

  新结构为：
  - `manticore` - deb/rpm 元包，安装以上所有作为依赖
  - `manticore-server-core` - 仅含 `searchd` 及其运行所需组件
  - `manticore-server` - systemd 文件和其他补充脚本
  - `manticore-tools` - `indexer`、`indextool` 及其他工具
  - `manticore-common` - 默认配置文件、默认数据目录、默认停用词列表
  - `manticore-icudata`、`manticore-dev`、`manticore-converter` 变化不大
  - `.tgz` 包含所有软件包的压缩包
* 支持 Ubuntu Jammy
* 支持通过 [YUM 仓库](Installation/RHEL_and_Centos.md#YUM-repository) 的 Amazon Linux 2

### Bug修复
* [Issue #815](https://github.com/manticoresoftware/manticoresearch/issues/815) 使用 UDF 函数时随机崩溃
* [Issue #287](https://github.com/manticoresoftware/manticoresearch/issues/287) RT 索引过程中内存溢出
* [Issue #604](https://github.com/manticoresoftware/manticoresearch/issues/604) 3.6.0，4.2.0 sphinxql-parser 重大变更
* [Issue #667](https://github.com/manticoresoftware/manticoresearch/issues/667) 致命错误：内存溢出（无法分配 9007199254740992 字节）
* [Issue #676](https://github.com/manticoresoftware/manticoresearch/issues/676) 字符串未正确传递给 UDF
* ❗[Issue #698](https://github.com/manticoresoftware/manticoresearch/issues/698) 尝试向 rt 索引添加文本列后 Searchd 崩溃
* [Issue #705](https://github.com/manticoresoftware/manticoresearch/issues/705) Indexer 无法找到所有列
* ❗[Issue #709](https://github.com/manticoresoftware/manticoresearch/issues/705) 按 json.boolean 分组行为异常
* [Issue #716](https://github.com/manticoresoftware/manticoresearch/issues/716) indextool 相关命令（例如 --dumpdict）失败
* ❗[Issue #724](https://github.com/manticoresoftware/manticoresearch/issues/724) 选取字段时字段消失
* [Issue #727](https://github.com/manticoresoftware/manticoresearch/issues/727) .NET HttpClient 使用 `application/x-ndjson` 时 Content-Type 不兼容问题
* [Issue #729](https://github.com/manticoresoftware/manticoresearch/issues/729) 字段长度计算问题
* ❗[Issue #730](https://github.com/manticoresoftware/manticoresearch/issues/730) 创建/插入/删除列存表存在内存泄漏
* [Issue #731](https://github.com/manticoresoftware/manticoresearch/issues/731) 特定条件下结果中出现空列
* ❗[Issue #749](https://github.com/manticoresoftware/manticoresearch/issues/749) 守护进程启动时崩溃
* ❗[Issue #750](https://github.com/manticoresoftware/manticoresearch/issues/750) 守护进程启动时挂起
* ❗[Issue #751](https://github.com/manticoresoftware/manticoresearch/issues/751) SST 阶段崩溃
* [Issue #752](https://github.com/manticoresoftware/manticoresearch/issues/752) 引擎为 'columnar' 时 json 属性标记为列存储
* [Issue #753](https://github.com/manticoresoftware/manticoresearch/issues/753) 复制监听地址为 0
* [Issue #754](https://github.com/manticoresoftware/manticoresearch/issues/754) columnar_attrs = * 在 csvpipe 下不起作用
* ❗[Issue #755](https://github.com/manticoresoftware/manticoresearch/issues/755) rt 中列存储选择 float 时崩溃
* ❗[Issue #756](https://github.com/manticoresoftware/manticoresearch/issues/756) Indextool 校验期间修改 rt 索引
* [Issue #757](https://github.com/manticoresoftware/manticoresearch/issues/757) 需要检查监听端口范围的冲突
* [Issue #758](https://github.com/manticoresoftware/manticoresearch/issues/758) RT 索引保存磁盘块失败时记录原始错误
* [Issue #759](https://github.com/manticoresoftware/manticoresearch/issues/759) RE2 配置只报告一个错误
* ❗[Issue #760](https://github.com/manticoresoftware/manticoresearch/issues/760) 提交 5463778558586d2508697fa82e71d657ac36510f 导致内存使用变化
* [Issue #761](https://github.com/manticoresoftware/manticoresearch/issues/761) 第三个节点在脏重启后不生成非主集群
* [Issue #762](https://github.com/manticoresoftware/manticoresearch/issues/762) 更新计数器增加了2
* [Issue #763](https://github.com/manticoresoftware/manticoresearch/issues/763) 使用4.2.0版本的形态学创建的新4.2.1版本索引损坏
* [Issue #764](https://github.com/manticoresoftware/manticoresearch/issues/764) json键中没有转义 /sql?mode=raw
* ❗[Issue #765](https://github.com/manticoresoftware/manticoresearch/issues/765) 使用函数隐藏了其他值
* ❗[Issue #766](https://github.com/manticoresoftware/manticoresearch/issues/766) 由FixupAttrForNetwork中的一行触发的内存泄漏
* ❗[Issue #767](https://github.com/manticoresoftware/manticoresearch/issues/767) 4.2.0 和 4.2.1版本中与文档存储缓存相关的内存泄漏
* [Issue #768](https://github.com/manticoresoftware/manticoresearch/issues/768) 通过网络存储字段时出现奇怪的来回波动
* [Issue #769](https://github.com/manticoresoftware/manticoresearch/issues/769) 如果在‘common’部分未提及，lemmatizer_base重置为空
* [Issue #770](https://github.com/manticoresoftware/manticoresearch/issues/770) pseudo_sharding使通过id的SELECT操作变慢
* [Issue #771](https://github.com/manticoresoftware/manticoresearch/issues/771) 使用jemalloc时DEBUG malloc_stats输出为零
* [Issue #772](https://github.com/manticoresoftware/manticoresearch/issues/772) 删除/添加列导致值不可见
* [Issue #773](https://github.com/manticoresoftware/manticoresearch/issues/773) 无法将bit(N)列添加到列式表中
* [Issue #774](https://github.com/manticoresoftware/manticoresearch/issues/774) 启动时manticore.json中的"cluster"变为空
* ❗[Commit 1da4](https://github.com/manticoresoftware/manticoresearch/commits/1da4ce891e4ff13727cfd3331d72771c64ee7949) HTTP操作未在SHOW STATUS中被跟踪
* [Commit 3810](https://github.com/manticoresoftware/manticoresearch/commits/381000ab4af4c3c98ec8c730699ad7f39039cec8) 为低频单关键词查询禁用pseudo_sharding
* [Commit 8003](https://github.com/manticoresoftware/manticoresearch/commits/800325cca283a87801b28929d82420fafc76a0ee) 修正了存储属性与索引合并的问题
* [Commit cddf](https://github.com/manticoresoftware/manticoresearch/commits/cddfeed6296f2041c6aae18414e5cd9cabf08281) 泛化了不同值的获取器；为列式字符串添加了专门的不同值获取器
* [Commit fba4](https://github.com/manticoresoftware/manticoresearch/commits/fba4bb4f) 修复了从文档存储中获取空整数属性的问题
* [Commit f300](https://github.com/manticoresoftware/manticoresearch/commit/f3009a9242fa16956adf871c4887e6e0303ba364) 查询日志中`ranker`可能被指定两次

## 版本 4.2.0，2021年12月23日

### 主要新功能
* **实时索引和全文查询的伪分片支持**。在上一个版本中我们添加了有限的伪分片支持。从该版本开始，只需启用[searchd.pseudo_sharding](Server_settings/Searchd.md#pseudo_sharding)即可获取伪分片及多核处理器的全部优势。最酷的是你无需对索引或查询做任何操作，只需启用它，如果你有空闲CPU，它将被用来降低响应时间。它支持对全文、本地和实时索引的过滤和分析查询。例如，开启伪分片可以使大多数查询在[Hacker news精选评论数据集](https://zenodo.org/record/45901/)（放大100倍，116百万文档的普通索引）上的**平均响应时间降低约10倍**。

![4.2.0中伪分片开启与关闭](4.2.0_ps_on_vs_off.png)

* [**Debian Bullseye**](https://manticoresearch.com/install/) 现已支持。

<!-- example pq_transactions_4.2.0 -->
* PQ事务现在是原子且隔离的。之前PQ事务支持有限。它实现了更**快的REPLACE into PQ**，特别是在一次需要替换大量规则时。性能详情：

<!-- intro -->
#### 4.0.2

<!-- request 4.0.2 -->

插入100万条PQ规则需要**48秒**，而分10K批次REPLACE仅40K条规则需**406秒**。

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

插入100万条PQ规则需**34秒**，而分10K批次REPLACE这些规则仅需**23秒**。

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
* [optimize_cutoff](Server_settings/Searchd.md#optimize_cutoff) 现可作为`searchd`配置节中的选项。它用于当你想在所有索引中全局限制RT块的数量时。
* [Commit 0087](https://github.com/manticoresoftware/manticoresearch/commit/00874743823eaf43e2a09a088392fd7e0a617f47) 精确实现了对多个本地物理索引（实时/普通）具有相同字段集和顺序的[count(distinct ...)](Searching/Grouping.md#COUNT%28DISTINCT-field%29)和[FACET ... distinct](Searching/Faceted_search.md#Faceting-without-duplicates)的支持。
* [PR #598](https://github.com/manticoresoftware/manticoresearch/pull/598) 为`YEAR()`及其他时间戳函数添加了bigint支持。
* [Commit 8e85](https://github.com/manticoresoftware/manticoresearch/commit/8e85d4bce596cc88f9d9615ea243a27e4afde0f7) 自适应 [rt_mem_limit](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_mem_limit)。以前 Manticore Search 会收集恰好达到 `rt_mem_limit` 的数据后才保存一个新的磁盘块到磁盘，而保存期间仍然会收集最多 10% 的额外数据（即双缓冲）以最小化可能的插入暂停。如果该限制也被用尽，则添加新文档会被阻塞，直到磁盘块完全保存到磁盘。新的自适应限制基于我们现在拥有 [auto-optimize](Server_settings/Searchd.md#auto_optimize) 的事实，因此磁盘块不完全遵守 `rt_mem_limit` 并提前开始刷新一个磁盘块并不是什么大问题。因此，现在我们收集最多 `rt_mem_limit` 的 50% 并保存为一个磁盘块。在保存过程中，我们会查看统计数据（我们保存了多少，有多少新文档在保存时到达）并重新计算下次使用的初始比率。例如，如果我们保存了 9000 万文档，同时保存时又有 1000 万文档到达，比率就是 90%，所以我们知道下次可以收集最多 90% 的 `rt_mem_limit` 后再开始刷新另一个磁盘块。比率值自动计算，范围从 33.3% 到 95%。
* [Issue #628](https://github.com/manticoresoftware/manticoresearch/issues/628) PostgreSQL 源的 [unpack_zlib](Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Database_connection.md#unpack_zlib)。感谢 [Dmitry Voronin](https://github.com/dimv36) 的 [贡献](https://github.com/manticoresoftware/manticoresearch/pull/630)。
* [Commit 6d54](https://github.com/manticoresoftware/manticoresearch/commit/6d54cf2b319b131970c29410cc21abcbdf8083b1) `indexer -v` 和 `--version`。之前你仍然可以看到 indexer 的版本，但不支持 `-v`/`--version`。
* [Issue #662](https://github.com/manticoresoftware/manticoresearch/issues/662) 通过 systemd 启动时默认无限制的 mlock 限制。
* [Commit 63c8](https://github.com/manticoresoftware/manticoresearch/commit/63c8cd05cf399e705c2c14939411a92cb784735a) spinlock -> 用于 coro rwlock 的操作队列。
* [Commit 4113](https://github.com/manticoresoftware/manticoresearch/commit/41130ce30d1734bbf8e6b8b63c4e815caf9464f2) 环境变量 `MANTICORE_TRACK_RT_ERRORS` 方便调试 RT 分段损坏。

### 重大变更
* Binlog 版本已升级，之前版本的 binlog 不会被重放，因此升级期间请确保干净停止 Manticore Search：停止之前的实例后，`/var/lib/manticore/binlog/` 目录中除了 `binlog.meta` 外不应有其他 binlog 文件。
* [Commit 3f65](https://github.com/manticoresoftware/manticoresearch/commit/3f659f36e87d99ee262205a8eae4663a255be630) `show threads option format=all` 新增列 "chain"。显示某些任务信息票据的堆栈，主要用于分析需求，如果你正在解析 `show threads` 输出，请注意新增列。
* `searchd.workers` 自 3.5.0 起已废弃，现在是弃用状态，如果配置文件中仍有该设置，则启动时会触发警告。Manticore Search 会启动，但会有警告。
* 如果你使用 PHP 和 PDO 访问 Manticore，需要设置 `PDO::ATTR_EMULATE_PREPARES`

### Bug 修复
* ❗[Issue #650](https://github.com/manticoresoftware/manticoresearch/issues/650) Manticore 4.0.2 比 Manticore 3.6.3 慢。4.0.2 的批量插入比以前版本快，但单文档插入明显变慢。该问题已在 4.2.0 修复。
* ❗[Commit 22f4](https://github.com/manticoresoftware/manticoresearch/commit/22f4141bed1411c83684ae6e0bae832071cce3df) RT 索引在高强度 REPLACE 负载下可能损坏，或导致崩溃。
* [Commit 03be](https://github.com/manticoresoftware/manticoresearch/commit/03be91e4f720a3f20bee0278dd8f0d44abd3ef0e) 修正合并 groupers 和 group N 排序器时的平均值计算；修正聚合合并。
* [Commit 2ea5](https://github.com/manticoresoftware/manticoresearch/commit/2ea575d34efe38661f14d858f2b552456f29fe42) `indextool --check` 可能崩溃。
* [Commit 7ec7](https://github.com/manticoresoftware/manticoresearch/commit/7ec76d4abf01b5fca6f10a0000fe64d5d8e5fa52) 更新导致的内存耗尽问题。
* [Commit 658a](https://github.com/manticoresoftware/manticoresearch/commit/658a727e41f3c3fbcde9754149528f34ed76ba19) 守护进程可能在插入时挂起。
* [Commit 46e4](https://github.com/manticoresoftware/manticoresearch/commit/46e42b9b386e31e53247ed634e158b533e28063e) 守护进程可能在关闭时挂起。
* [Commit f8d7](https://github.com/manticoresoftware/manticoresearch/commit/f8d7d5177cc83d250b493b4cfc619732b29cf072) 守护进程可能在关闭时崩溃。
* [Commit 733a](https://github.com/manticoresoftware/manticoresearch/commit/733accf165142a1d1574689525ccca8907971158) 守护进程崩溃时可能挂起。
* [Commit f7f8](https://github.com/manticoresoftware/manticoresearch/commit/f7f8bd8c0dbb23a59a88502dcba16b0918fb2330) 启动时尝试以无效节点列表重新加入集群可能导致守护进程崩溃。
* [Commit 1401](https://github.com/manticoresoftware/manticoresearch/commit/140155613821ecaddcd1a3dfc4f7369b0df6aef7) 分布式索引在 RT 模式下如果启动时无法解析其中一个代理，可能会被完全遗忘。
* [Issue #683](https://github.com/manticoresoftware/manticoresearch/issues/683) attr bit(N) engine='columnar' 失败。
* [Issue #682](https://github.com/manticoresoftware/manticoresearch/issues/682) 创建表失败，但留下目录。
* [Issue #663](https://github.com/manticoresoftware/manticoresearch/issues/663) 配置失败，未知键名 'attr_update_reserve'。
* [Issue #632](https://github.com/manticoresoftware/manticoresearch/issues/632) Manticore 在批量查询时崩溃。
* [Issue #679](https://github.com/manticoresoftware/manticoresearch/issues/679) 批量查询再次导致 v4.0.3 崩溃。
* [提交 f7f8](https://github.com/manticoresoftware/manticoresearch/commit/f7f8bd8c0dbb23a59a88502dcba16b0918fb2330) 修复了在启动时尝试使用无效节点列表重新加入集群导致的守护进程崩溃问题
* [问题 #643](https://github.com/manticoresoftware/manticoresearch/issues/643) Manticore 4.0.2 在批量插入后不接受连接
* [问题 #635](https://github.com/manticoresoftware/manticoresearch/issues/635) 带有 ORDER BY JSON.field 或字符串属性的 FACET 查询可能崩溃
* [问题 #634](https://github.com/manticoresoftware/manticoresearch/issues/634) 查询带 packedfactors 时发生 SIGSEGV 崩溃
* [提交 4165](https://github.com/manticoresoftware/manticoresearch/commits/41657f15f8fcf1e93cfad7f835bcf705d6e8349e) morphology_skip_fields 不被 create table 支持

## 版本 4.0.2，2021年9月21日

### 主要新特性
- **完全支持 [Manticore Columnar Library](https://github.com/manticoresoftware/columnar)**。之前 Manticore Columnar Library 只支持普通索引。现在支持：
  - 实时索引的 `INSERT`、`REPLACE`、`DELETE`、`OPTIMIZE`
  - 复制功能
  - `ALTER`
  - `indextool --check`
- **自动索引压缩** ([问题 #478](https://github.com/manticoresoftware/manticoresearch/issues/478))。终于，您无需手动或通过定时任务等方式调用 OPTIMIZE，Manticore 现在默认自动完成。您可以通过全局变量 [optimize_cutoff](Server_settings/Setting_variables_online.md) 设置默认压缩阈值。
- **区块快照和锁系统重构**。这些改动初看可能无明显变化，但极大改善了实时索引中许多操作的行为。简而言之，之前大多数 Manticore 的数据操作严重依赖锁，现在我们改用磁盘区块快照。
- **向实时索引批量插入性能显著提升**。例如在 [Hetzner 的服务器 AX101](https://www.hetzner.com/dedicated-rootserver/ax101) （SSD，128GB 内存，AMD Ryzen™ 9 5950X（16*2核））上，**3.6.0 版本每秒插入 236K 文档**，表结构为 `name text, email string, description text, age int, active bit(1)`（默认 `rt_mem_limit`，批量大小 25000，16 并发插入工作线程，总计插入 1600万文档)。而 4.0.2 相同并发/批量/数量下达到 **357K 文档每秒**。

  <details>

  - 读取操作（例如 SELECT、复制）使用快照执行
  - 只改变内部索引结构但不修改模式/文档的操作（如合并 RAM 段、保存磁盘区块、合并磁盘区块）使用只读快照，最终替换现有区块
  - UPDATE 和 DELETE 针对现有区块执行，但在可能发生合并的情况下，写入操作被收集，然后应用到新区块
  - UPDATE 针对每个区块按顺序获取独占锁。合并在收集区块属性阶段获取共享锁。因此同时只有一个（合并或更新）操作访问区块属性
  - 当合并进入需要属性阶段时，设置特定标志。UPDATE 完成后检查该标志，若已设定，整个更新被存储到特殊集合中。合并结束时，将更新应用到新生成的磁盘区块
  - ALTER 通过独占锁运行
  - 复制作为常规读取操作运行，但额外在 SST 前保存属性，并禁止 SST 期间更新

  </details>

- **[ALTER](Updating_table_schema_and_settings.md) 支持添加或删除全文字段**（RT 模式）。之前仅支持添加或删除属性。
- 🔬 **实验性质：全文扫描查询的伪分片**——允许并行化任何非全文本搜索查询。不再需要手动准备分片，只需启用新选项 [searchd.pseudo_sharding](Server_settings/Searchd.md#pseudo_sharding)，非全文搜索查询响应时间可降低至最多 CPU 核心数倍。注意它可能占用所有 CPU 核心，因此若您关注不仅是延迟，还有吞吐量，请谨慎使用。

### 次要改动
<!-- example -->
- 通过 [APT 仓库](Installation/Debian_and_Ubuntu.md#APT-repository) 支持 Linux Mint 和 Ubuntu Hirsute Hippo
- 在某些情况下通过 HTTP 按 id 快速更新大索引（依赖 id 分布）
- [671e65a2](https://github.com/manticoresoftware/lemmatizer-uk/commit/671e65a296800537123a3f8b324eeaa3f58b9632) - 为乌克兰语词形还原器添加缓存


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
- [systemd 的自定义启动标志](Starting_the_server/Linux.md#Custom-startup-flags-using-systemd)。现在如果需要用特定启动标志运行 Manticore，无需手动启动 searchd
- 新增函数 [LEVENSHTEIN()](Functions/String_functions.md#LEVENSHTEIN%28%29)，用于计算 Levenshtein 距离
- 新增 [searchd 启动选项](Starting_the_server/Manually.md#searchd-command-line-options) `--replay-flags=ignore-trx-errors` 和 `--replay-flags=ignore-all-errors`，允许在 binlog 损坏时仍然启动 searchd
- [问题 #621](https://github.com/manticoresoftware/manticoresearch/issues/621) - 暴露来自 RE2 的错误信息
- 更精确的针对包含本地普通索引的分布式索引的 [COUNT(DISTINCT)](Searching/Grouping.md#COUNT%28DISTINCT-field%29) 实现
- [FACET DISTINCT](Searching/Faceted_search.md#Faceting-without-duplicates)，用于执行分面搜索时去重
- [exact form modifier](Searching/Full_text_matching/Operators.md#Exact-form-modifier) 现在无需 [morphology](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology)，且支持启用 [infix/prefix](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) 搜索的索引

### 破坏性变更
- 新版本可以读取旧索引，但旧版本无法读取 Manticore 4 的索引
- 移除了隐式按 id 排序。如果需要排序，请显式指定
- `charset_table` 的默认值从 `0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451` 改为 `non_cjk`
- `OPTIMIZE` 会自动执行。如果不需要，请确保在配置文件的 `searchd` 部分设置 `auto_optimize=0`
- [Issue #616](https://github.com/manticoresoftware/manticoresearch/issues/616) 中的 `ondisk_attrs_default` 已被废弃，现已移除
- 对贡献者：我们现在使用 Clang 编译器进行 Linux 构建，根据测试它可以构建更快速的 Manticore Search 和 Manticore Columnar Library
- 如果搜索查询中未指定 [max_matches](Searching/Options.md#max_matches)，则会隐式地更新为性能所需的最低值，以优化新的列式存储。这可能会影响 [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META) 中的 `total` 指标，但不会影响实际找到文档数量的 `total_found`

### 从 Manticore 3 迁移
- 确保你已干净地停止 Manticore 3：
  - `/var/lib/manticore/binlog/` 目录中不应有 binlog 文件（目录中只能有 `binlog.meta`）
  - 否则 Manticore 4 无法对无法回复 binlog 的索引进行操作
- 新版本可以读取旧索引，但旧版本无法读取 Manticore 4 的索引，因此如果你希望能够方便回滚新版本，请务必备份
- 如果使用复制集群，请确保：
  - 先干净停止所有节点
  - 然后以 `--new-cluster` 参数启动最后停下的节点（Linux 下运行工具 `manticore_new_cluster`）
  - 更多详情请阅读 [重启集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster)

### 修复内容
- 修复了大量复制相关的问题：
  - [Commit 696f](https://github.com/manticoresoftware/manticoresearch/commit/696f8649535f2b5285b878c9581c0d2b1139ae09) - 修复了在 joiner 上 SST 期间遇到活动索引时崩溃的问题；加入节点在写入文件块时新增 sha1 校验以加速索引加载；加入节点加载索引时新增已更改索引文件的轮换；加入节点在活动索引被 donor 节点的新索引替换时移除索引文件；donor 节点为发送文件和块新增复制日志点
  - [Commit b296](https://github.com/manticoresoftware/manticoresearch/commit/b296c55af68314a8fc66f8b104e7301d673c3b68) - 当 JOIN CLUSTER 时地址错误会导致崩溃
  - [Commit 418b](https://github.com/manticoresoftware/manticoresearch/commit/418bf880a5e05ffbb68f311a1c23006a9df3220e) - 在大型索引的初始复制时，加入节点可能以 `ERROR 1064 (42000): invalid GTID, (null)` 失败，donor 在另一个节点加入期间可能变得无响应
  - [Commit 6fd3](https://github.com/manticoresoftware/manticoresearch/commit/6fd350d28e4905211cf8a7298cbf7cdda101c83d) - 大索引的哈希可能计算错误，导致复制失败
  - [Issue #615](https://github.com/manticoresoftware/manticoresearch/issues/615) - 集群重启时复制失败
- [Issue #574](https://github.com/manticoresoftware/manticoresearch/issues/574) - `indextool --help` 未显示参数 `--rotate`
- [Issue #578](https://github.com/manticoresoftware/manticoresearch/issues/578) - searchd 空闲时高 CPU 占用，持续约一天后出现
- [Issue #587](https://github.com/manticoresoftware/manticoresearch/issues/587) - 立即刷新 .meta 文件
- [Issue #617](https://github.com/manticoresoftware/manticoresearch/issues/617) - manticore.json 文件被清空
- [Issue #618](https://github.com/manticoresoftware/manticoresearch/issues/618) - searchd --stopwait 在 root 下失败。同时修复了 systemctl 行为（之前 ExecStop 显示失败且等待时间不足以让 searchd 正常停止）
- [Issue #619](https://github.com/manticoresoftware/manticoresearch/issues/619) - INSERT/REPLACE/DELETE 与 SHOW STATUS 中的指标不正确，`command_insert`、`command_replace` 及其他指标显示错误
- [Issue #620](https://github.com/manticoresoftware/manticoresearch/issues/620) - 普通索引的 `charset_table` 默认值错误
- [Commit 8f75](https://github.com/manticoresoftware/manticoresearch/commit/8f7536887d4b58a5bab5647fc067b2d0482c2038) - 新的磁盘块未上锁内存（mlocked）
- [Issue #607](https://github.com/manticoresoftware/manticoresearch/issues/607) - 当无法通过名称解析节点时，Manticore 集群节点崩溃
- [Issue #623](https://github.com/manticoresoftware/manticoresearch/issues/623) - 更新索引的复制可能导致未定义状态
- [Commit ca03](https://github.com/manticoresoftware/manticoresearch/commit/ca03d2280c5197e9b311f0f03f551e3b702a8130) - 索引器在索引含 json 属性的普通索引源时可能挂起
- [Commit 53c7](https://github.com/manticoresoftware/manticoresearch/commit/53c753054558ff3f82c7af365273d32537c5338b) - 修复 PQ 索引中不等表达式过滤器的问题
- [Commit ccf9](https://github.com/manticoresoftware/manticoresearch/commit/ccf94e022ba6763d6286ab5bac767b6f6a846846) - 修复列表查询中超过 1000 个匹配结果时的 Select 窗口问题。之前 `SELECT * FROM pq ORDER BY id desc LIMIT 1000 , 100 OPTION max_matches=1100` 无法工作
- [Commit a048](https://github.com/manticoresoftware/manticoresearch/commit/a0483fe9bef3c45dffe96f5df4f4382aee29c072) - 向 Manticore 发起 HTTPS 请求时可能触发类似 “max packet size(8388608) exceeded” 的警告
- [Issue #648](https://github.com/manticoresoftware/manticoresearch/issues/648) - Manticore 3 在字符串属性更新数次后可能挂起


## 版本 3.6.0，2021 年 5 月 3 日
**Manticore 4 之前的维护版本**

### 主要新功能
- 对纯索引支持[Manticore Columnar Library](https://github.com/manticoresoftware/columnar/)。纯索引新增设置[columnar_attrs](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#columnar_attrs)
- 支持[乌克兰语词形还原器](https://github.com/manticoresoftware/lemmatizer-uk)
- 全面修订了直方图。构建索引时，Manticore 还会为每个字段构建直方图，以便更快地过滤。在3.6.0版本中，算法进行了全面修订，如果有大量数据并且频繁过滤，可以获得更高性能。

### 小改动
- 工具 `manticore_new_cluster [--force]`，用于通过 systemd 重启复制集群
- `indexer --merge` 新增 [--drop-src](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Merging_tables.md#Merging-tables) 参数
- [新模式](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) `blend_mode='trim_all'`
- 添加了用反引号支持转义 JSON 路径的功能 [支持转义 JSON 路径](Searching/Full_text_matching/Escaping.md#Escaping-json-node-names-in-SQL)
- [indextool --check](Miscellaneous_tools.md#indextool) 可用于 RT 模式
- SELECT/UPDATE 支持 [FORCE/IGNORE INDEX(id)](Data_creation_and_modification/Updating_documents/UPDATE.md#Updates-via-SQL)
- 合并的磁盘块的 chunk id 现在唯一
- [indextool --check-disk-chunk CHUNK_NAME](Miscellaneous_tools.md#indextool)

### 优化
- [更快的 JSON 解析](https://github.com/manticoresoftware/manticoresearch/commit/0c25fc1788e3f77f60cb7fb8f1623d8466bba606)，我们的测试显示诸如 `WHERE json.a = 1` 这类查询延迟降低了3-4%
- 非公开命令 `DEBUG SPLIT`，为自动分片/重新平衡的前提条件

### Bug修复
- [Issue #584](https://github.com/manticoresoftware/manticoresearch/issues/524) - FACET 结果不准确且不稳定
- [Issue #506](https://github.com/manticoresoftware/manticoresearch/issues/506) - 使用 MATCH 时异常行为：受此问题影响的用户需要重建索引，因为问题出在构建索引阶段
- [Issue #387](https://github.com/manticoresoftware/manticoresearch/issues/387) - 使用 SNIPPET() 函数运行查询时偶发核心转储
- 针对处理复杂查询的栈优化：
  - [Issue #469](https://github.com/manticoresoftware/manticoresearch/issues/469) - SELECT 导致崩溃转储
  - [e8420cc7](https://github.com/manticoresoftware/manticoresearch/tree/e8420cc789149c84731be0874be916e33b263bd6) - 过滤器树的栈大小检测
- [Issue #461](https://github.com/manticoresoftware/manticoresearch/issues/461) - 使用 IN 条件更新时更新未正确生效
- [Issue #464](https://github.com/manticoresoftware/manticoresearch/issues/464) - CALL PQ 后立即执行 SHOW STATUS 返回异常 - [Issue #481](https://github.com/manticoresoftware/manticoresearch/issues/481) - 修复了静态二进制构建
- [Issue #502](https://github.com/manticoresoftware/manticoresearch/issues/502) - 多查询存在 Bug
- [Issue #514](https://github.com/manticoresoftware/manticoresearch/issues/514) - 使用 'create table' 时无法使用非常规列名
- [Commit d1db](https://github.com/manticoresoftware/manticoresearch/commit/d1dbe771288328c963ca8512f564c6b7cf4f466c) - 更新字符串属性时重放 binlog 导致守护进程崩溃；将 binlog 版本设置为 10
- [Commit 775d](https://github.com/manticoresoftware/manticoresearch/commit/775d0555562f121911d3c30085947e743fadac2d)  - 修复表达式栈帧检测运行时错误（测试207）
- [Commit 4795](https://github.com/manticoresoftware/manticoresearch/commit/4795dc49194c7745761bb5006ecb266e8a9224fa)  - 空存储查询的穿透索引过滤器和标签为空（测试369）
- [Commit c3f0](https://github.com/manticoresoftware/manticoresearch/commit/c3f0bf4dca0e4eae6ea899dd566878df57eba085)  - 网络高延迟和高错误率下复制 SST 流中断（跨数据中心复制）；更新复制命令版本为1.03
- [Commit ba2d](https://github.com/manticoresoftware/manticoresearch/commit/ba2d6619b8107f3fb177741bbdff3709bac248cd)  - 集群写操作时合并锁定（测试385）
- [Commit de4d](https://github.com/manticoresoftware/manticoresearch/commit/de4dcb9ff6c316bc2a9ef6c62eb64769f04ee103)  - 带有精确修饰符的通配符匹配（测试321）
- [Commit 6524](https://github.com/manticoresoftware/manticoresearch/commit/6524fc6af032a0932bd42c37874a619e1fb7f075)  - docid 检查点与文档存储
- [Commit f4ab](https://github.com/manticoresoftware/manticoresearch/commit/f4ab83c291e65a5873d271c67b5491f629544f1d)  - 解析无效 XML 时索引器行为不一致
- [Commit 7b72](https://github.com/manticoresoftware/manticoresearch/commit/7b727e22fe01b1a180d4a8d3a3fea92c18eac707)  - 含 NOTNEAR 的存储穿透查询永远运行（测试349）
- [Commit 812d](https://github.com/manticoresoftware/manticoresearch/commit/812dab745f0c972ecde5d5b3d4a313cd6e1e143e)  - 以通配符开头的短语权重错误
- [Commit 1771](https://github.com/manticoresoftware/manticoresearch/commit/1771afc669c75670b3511bf02ab9bfc8982b94d2)  - 通配符穿透查询在匹配时生成无负载的术语导致命中交错和匹配破坏（测试417）
- [Commit aa0d](https://github.com/manticoresoftware/manticoresearch/commit/aa0d8c2bcb41428d41dbfb66e42a872a79a4ee4b)  - 修正并行查询情况下 'total' 的计算
- [Commit 18d8](https://github.com/manticoresoftware/manticoresearch/commit/18d81b3cfbb930b7361dae645fe13283d4fbf6ef)  - Windows 多会话守护进程崩溃
- [Commit 8443](https://github.com/manticoresoftware/manticoresearch/commit/84432f23d6fbf7a7d0b67c8c9412eb5933b03596)  - 修复部分索引设置无法被复制问题
- [Commit 9341](https://github.com/manticoresoftware/manticoresearch/commit/93411fe6af4246ca4acce758944d513a1660b92b)  - 在高频率添加新事件时，netloop 有时会冻结，因为原子“kick”事件一次处理多个事件，导致实际操作丢失
查询的状态，不是服务器状态
- [Commit d805](https://github.com/manticoresoftware/manticoresearch/commit/d805fc125244ef08d14fb3d1826e2c356862d539)  - 新刷新写入的磁盘块可能在提交时丢失
- [Commit 63cb](https://github.com/manticoresoftware/manticoresearch/commit/63cbf008d50979cdf7062664fb02a672a3bb1131)  - profiler 中不准确的 'net_read'
- [Commit f537](https://github.com/manticoresoftware/manticoresearch/commit/f5379bb27234b25919966c50ccb6f469ead646e2)  - 阿拉伯语（从右到左文本）对 Percolate 的问题
- [Commit 49ee](https://github.com/manticoresoftware/manticoresearch/commit/49eeb4202d4ce4875c07c35087d7108c1e177328)  - 重复列名时 id 未正确选取
- [Commit refa](https://github.com/manticoresoftware/manticoresearch/commit/4b7df262bbfec9b9e95a44e72ad326ac8a786c49) 修正网络事件中一个罕见崩溃的问题
- [e8420cc7](https://github.com/manticoresoftware/manticoresearch/tree/e8420cc789149c84731be0874be916e33b263bd6) `indextool --dumpheader` 的修复
- [Commit ff71](https://github.com/manticoresoftware/manticoresearch/commit/ff7163532ed39042c0d3c3c2b902ff41f94d7f0c) - TRUNCATE WITH RECONFIGURE 在存储字段处理时工作异常

### 不兼容变更：
- 新的 binlog 格式：升级前需要先干净停止 Manticore
- 索引格式略微变化：新版可以正常读取您现有索引，但如果您决定从 3.6.0 降级到老版本，使用新版生成的索引将无法阅读
- 复制格式更改：不要从旧版本复制到 3.6.0，反之亦然，应同时在所有节点切换到新版本
- `reverse_scan` 已被弃用。确保从 3.6.0 起查询中不再使用此选项，否则会失败
- 从此版本起，我们不再提供 RHEL6、Debian Jessie 和 Ubuntu Trusty 的构建版本。如果您必须支持这些版本，请[联系我们](https://manticoresearch.com/contact-us/)

### 弃用事项
- 不再隐式按 id 排序。如果您依赖此行为，请相应更新查询
- 搜索选项 `reverse_scan` 已被弃用

## 版本 3.5.4，2020年12月10日

### 新特性
- 新的 Python、Javascript 和 Java 客户端已全面发布，并在本手册中有详细文档。
- 实时索引的磁盘块自动丢弃。这项优化使得在实时索引进行 [OPTIMIZE](Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE) 操作时，若磁盘块显然不再需要（所有文档均被删除），可以自动丢弃该磁盘块。之前仍需合并，现在磁盘块可立即丢弃。[cutoff](Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks) 选项被忽略，即使没有实际合并，过时的磁盘块也会被移除。这在您维护索引保留策略、删除旧文档时特别有用。现在压缩这类索引会更快。
- [独立 NOT](Searching/Options.md#not_terms_only_allowed) 作为 SELECT 的新选项

### 小改动
- [Issue #453](https://github.com/manticoresoftware/manticoresearch/issues/453) 新选项 [indexer.ignore_non_plain=1](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#ignore_non_plain) 方便在使用 `indexer --all` 时同时配置多个索引类型。未加此选项会输出警告并返回对应退出码。
- [SHOW PLAN ... OPTION format=dot](Node_info_and_management/Profiling/Query_plan.md#Dot-format-for-SHOW-PLAN) 和 [EXPLAIN QUERY ... OPTION format=dot](Searching/Full_text_matching/Profiling.md#Profiling-without-running-a-query) 支持完整文本查询计划执行的可视化，助于理解复杂查询。

### 弃用事项
- `indexer --verbose` 被弃用，因为它对 indexer 输出无实际补充
- 倒转后台监控（watchdog）的回溯信号由 `USR1` 改为 `USR2`

### Bug修复
- [Issue #423](https://github.com/manticoresoftware/manticoresearch/issues/423) 斯拉夫字母点调用片段模式不高亮问题
- [Issue #435](https://github.com/manticoresoftware/manticoresearch/issues/435) RTINDEX - GROUP N BY 表达式选择导致致命崩溃
- [Commit 2b3b](https://github.com/manticoresoftware/manticoresearch/commit/2b3b62bd0ce2b54544733d524a169e81f71b5388) 集群中 searchd 状态显示段错误
- [Commit 9dd2](https://github.com/manticoresoftware/manticoresearch/commit/9dd25c193ed2a53750a0e0e68566fd2b833fcb0c) 'SHOW INDEX index.N SETTINGS' 不支持处理大于 9 的 chunk
- [Issue #389](https://github.com/manticoresoftware/manticoresearch/issues/389) 导致 Manticore 崩溃的 Bug
- [Commit fba1](https://github.com/manticoresoftware/manticoresearch/commit/fba166178ff35c73a0c70d131cf3aca4a635e961) 转换器生成损坏的索引
- [Commit eecd](https://github.com/manticoresoftware/manticoresearch/commit/eecd61d861c742471175be19a5e5455034b5db5f) stopword_step=0 与 CALL SNIPPETS() 的兼容性问题
- [Commit ea68](https://github.com/manticoresoftware/manticoresearch/commit/ea6850e449601199190f38a387c04b8a7b7e90f6) 本地索引低 max_matches 情况下 count distinct 返回 0
- [Commit 362f](https://github.com/manticoresoftware/manticoresearch/commit/362f27db3421f09a41c6cce04b5baace00fa4c4d) 使用聚合时，存储的文本未返回在命中结果中


## 版本 3.5.2，2020年10月1日

### 新特性

* OPTIMIZE 将磁盘块减少为指定数量（默认是 `2 * 核心数`），而非合并成单个块。最优块数可通过 [cutoff](Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks) 选项控制。
* NOT 操作符现在可以独立使用。默认情况下它是禁用的，因为意外的单个 NOT 查询可能会很慢。可以通过将新的 searchd 指令 [not_terms_only_allowed](Server_settings/Searchd.md#not_terms_only_allowed) 设置为 `0` 来启用。
* 新设置 [max_threads_per_query](Server_settings/Searchd.md#max_threads_per_query) 用于设置查询可以使用的线程数。如果未设置该指令，查询可以使用的线程数将达到 [threads](Server_settings/Searchd.md#threads) 的值。
每个 `SELECT` 查询可以通过 [OPTION threads=N](Searching/Options.md#threads) 来限制线程数，从而覆盖全局的 `max_threads_per_query`。
* Percolate 索引现在可以通过 [IMPORT TABLE](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Importing_table.md) 导入。
* HTTP API `/search` 通过新的查询节点 `aggs` 获得了对[分面搜索](Searching/Faceted_search.md#HTTP-JSON)/[分组](Searching/Grouping.md)的基本支持。

### 次要更改

* 如果未声明 replication listen 指令，引擎将尝试使用定义的 'sphinx' 端口之后的端口，最多到 200。
* 对于 SphinxSE 连接或 SphinxAPI 客户端，需要显式设置 `listen=...:sphinx`。
* [SHOW INDEX STATUS](Node_info_and_management/Table_settings_and_status/SHOW_TABLE_STATUS.md) 输出了新的指标：`killed_documents`，`killed_rate`，`disk_mapped_doclists`，`disk_mapped_cached_doclists`，`disk_mapped_hitlists` 和 `disk_mapped_cached_hitlists`。
* SQL 命令 `status` 现在输出 `Queue\Threads` 和 `Tasks\Threads`。

### 废弃项：

* `dist_threads` 现已完全废弃，如果仍然使用该指令，searchd 会记录警告。

### Docker

官方 Docker 镜像现在基于 Ubuntu 20.04 LTS。

### 打包

除了常规的 `manticore` 包，您还可以按组件安装 Manticore Search：

- `manticore-server-core` - 提供 `searchd`、手册页、日志目录、API 和 galera 模块。它还会安装 `manticore-common` 作为依赖。
- `manticore-server` - 提供核心的自动化脚本（init.d，systemd）和 `manticore_new_cluster` 包装器。它还会安装 `manticore-server-core` 作为依赖。
- `manticore-common` - 提供配置、停用词、通用文档和骨架文件夹（数据目录、模块等）。
- `manticore-tools` - 提供辅助工具（`indexer`，`indextool` 等）、它们的手册页和示例。它还会安装 `manticore-common` 作为依赖。
- `manticore-icudata`（RPM）或 `manticore-icudata-65l`（DEB） - 提供 ICU 数据文件以供 icu 形态学使用。
- `manticore-devel`（RPM）或 `manticore-dev`（DEB） - 提供 UDF 开发头文件。

### Bug 修复

1. [Commit 2a47](https://github.com/manticoresoftware/manticoresearch/commit/2a474dc1a26e8b0f8aaaae95669caf2f1d4b7746) RT 索引中带不同 chunks 的 grouper 进程崩溃
2. [Commit 57a1](https://github.com/manticoresoftware/manticoresearch/commit/57a19e5ad5663ef0ca7436595218fb1221d28c8e) 空远程文档的快速路径
3. [Commit 07dd](https://github.com/manticoresoftware/manticoresearch/commit/07dd3f313c63fb82c22092f9907ef24e3475250e) 表达式堆栈帧检测时运行时错误
4. [Commit 08ae](https://github.com/manticoresoftware/manticoresearch/commit/08ae357cf1012bc8e2da54c20b205b592efda3d4) Percolate 索引中匹配超过 32 个字段
5. [Commit 16b9](https://github.com/manticoresoftware/manticoresearch/commit/16b9390fd4cdb07a77ac4497adb935573a1710e5) 复制监听端口范围
6. [Commit 5fa6](https://github.com/manticoresoftware/manticoresearch/commit/5fa671affeacb6441a59c8a88479bfd423df7c81) pq 上显示创建表语句
7. [Commit 54d1](https://github.com/manticoresoftware/manticoresearch/commit/54d133b6449105a9fb0168db3f1fbb05fb5aa1f6) HTTPS 端口行为
8. [Commit fdbb](https://github.com/manticoresoftware/manticoresearch/commit/fdbbe5245cc296cc5c1ae3ae2fb9cb08fb66a248) 替换时混淆文档存储行
9. [Commit afb5](https://github.com/manticoresoftware/manticoresearch/commit/afb53f648ef4e64fa1776c58e66f6e716ac730ad) 将 TFO 不可用消息级别切换为 'info'
10. [Commit 59d9](https://github.com/manticoresoftware/manticoresearch/commit/59d94cefc6e88af360b8046726a16b7eaa9f0b71) strcmp 失效使用导致崩溃
11. [Commit 04af](https://github.com/manticoresoftware/manticoresearch/commit/04af0349c1e784c2230d204592d6272b440bb375) 将索引添加到含系统（停用词）文件的集群
12. [Commit 5014](https://github.com/manticoresoftware/manticoresearch/commit/50148b4eae9f3a924082005cf5d2f9c8339af7f5) 合并含大词典的索引；RT 优化大磁盘块
13. [Commit a2ad](https://github.com/manticoresoftware/manticoresearch/commit/a2adf1582843f63d4dcc1dab8f7598e0917094a2) Indextool 能从当前版本转储元数据
14. [Commit 69f6](https://github.com/manticoresoftware/manticoresearch/commit/69f6d5f77c64b97e3f395aea8c555b11c77fb732) GROUP N 中的分组排序问题
15. [Commit 24d5](https://github.com/manticoresoftware/manticoresearch/commit/24d5d80f508576794b751d70f9f9b524564e3c14) SphinxSE 握手后显式刷新
16. [Commit 31c4](https://github.com/manticoresoftware/manticoresearch/commit/31c4d78ac03ee85d9a02b86075448a04ac80f9e2) 避免不必要时拷贝大描述
17. [Commit 2959](https://github.com/manticoresoftware/manticoresearch/commit/2959e2caa8610f98fcdf2e5a82160f58687f4a27) show threads 中的负时间
18. [Commit f0b3](https://github.com/manticoresoftware/manticoresearch/commit/f0b357102c70f3ac1d7da260c60b67322940e647) 令牌过滤插件与零位置增量
19. [Commit a49e](https://github.com/manticoresoftware/manticoresearch/commit/a49e5bc13ea763e6f01dd5993e891858a84d1846) 多重命中时将 'FAIL' 改为 'WARNING'

## 版本 3.5.0，2020年7月22日

### 主要新功能：
* 这个版本发布花了很长时间，因为我们一直在努力将多任务模式从线程改为**协程**。这使得配置更简单，查询的并行化更加直接：Manticore 只使用给定数量的线程（参见新设置 [threads](Server_settings/Searchd.md#threads)），新模式确保以最优方式完成。
* [高亮显示](Searching/Highlighting.md#Highlighting-options) 的更改：
  - 任何在多个字段上工作的高亮（`highlight({},'field1, field2'`）或 JSON 查询中的 `highlight`）现在默认按字段应用限制。
  - 任何处理纯文本的高亮（`highlight({}, string_attr)` 或 `snippet()`）现在对整个文档应用限制。
  - [每字段限制](Searching/Highlighting.md#limits_per_field) 可以通过 `limits_per_field=0` 选项（默认 `1`）切换为全局限制。
  - [allow_empty](Searching/Highlighting.md#allow_empty) 现在通过 HTTP JSON 接口高亮时默认值为 `0`。

* 现在同一端口[可以使用](Server_settings/Searchd.md#listen)于 http、https 和二进制 API（以接受来自远程 Manticore 实例的连接）。通过 mysql 协议连接仍需要 `listen = *:mysql`。Manticore 现在能自动检测尝试连接它的客户端类型（MySQL 除外，受协议限制）。

* 在 RT 模式下，字段现在可以同时是[文本和值属性](Creating_a_table/Data_types.md#String) - [GitHub issue #331](https://github.com/manticoresoftware/manticoresearch/issues/331)。

  在[纯模式](Read_this_first.md#Real-time-mode-vs-plain-mode)中称为 `sql_field_string`。现在也可用于实时索引的[RT 模式](Read_this_first.md#Real-time-mode-vs-plain-mode)。用法示例如下：

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

### 小幅更改
* 现在可以[高亮字符串属性](Searching/Highlighting.md#Highlighting-via-SQL)。
* SQL 接口支持 SSL 和压缩。
* 支持 mysql 客户端的 [`status`](Node_info_and_management/Node_status.md#STATUS) 命令。
* [复制](Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Setting-up-replication) 现在可复制外部文件（停用词、例外等）。
* 过滤操作符 [`in`](Searching/Filters.md#Set-filters) 现在支持 HTTP JSON 接口。
* HTTP JSON 中支持 [`expressions`](Searching/Expressions.md#expressions)。
* 现在可以在 RT 模式中[动态修改 `rt_mem_limit`](https://github.com/manticoresoftware/manticoresearch/issues/344)，即执行 `ALTER ... rt_mem_limit=<新值>`。
* 现在可以使用[独立的 CJK 字符集表](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology)：`chinese`、`japanese` 和 `korean`。
* [thread_stack](Server_settings/Searchd.md#thread_stack) 现在限制最大线程栈大小，而非初始大小。
* 改进了 `SHOW THREADS` 输出。
* 在 `SHOW THREADS` 中显示长时间运行的 `CALL PQ` 进度。
* cpustat、iostat、coredump 运行时可用 [SET](Server_settings/Setting_variables_online.md#SET) 进行更改。
* 实现了 `SET [GLOBAL] wait_timeout=NUM`。

### 重大改动：
* **索引格式已更改。** 用 3.5.0 构建的索引无法被 Manticore 3.5.0 以下版本加载，但 Manticore 3.5.0 可以识别旧格式。
* [`INSERT INTO PQ VALUES()`](Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md)（即不提供列列表）之前期望值精确为 `(query, tags)`。现更改为 `(id,query,tags,filters)`。如果想自动生成 id，可以设置为 0。
* 通过 HTTP JSON 接口高亮时，`allow_empty=0` 是新的默认值。
* `CREATE TABLE`/`ALTER TABLE` 中外部文件（停用词、例外等）只允许使用绝对路径。

### 废弃项：
* `ram_chunks_count` 在 `SHOW INDEX STATUS` 中已重命名为 `ram_chunk_segments_count`。
* `workers` 已废弃。现在仅有一种 workers 模式。
* `dist_threads` 已废弃。所有查询均尽可能并行（受 `threads` 和 `jobs_queue_size` 限制）。
* `max_children` 已废弃。用 [threads](Server_settings/Searchd.md#threads) 设置 Manticore 将使用的线程数（默认设置为 CPU 核数）。
* `queue_max_length` 已废弃。若确实需要，用 [jobs_queue_size](Server_settings/Searchd.md#jobs_queue_size) 微调内部作业队列大小（默认无限制）。
* 所有 `/json/*` 端点现在可以不含 `/json/` 访问，例如 `/search`、`/insert`、`/delete`、`/pq` 等。
* 在 `describe` 中，`field`（全文字段含义）已重命名为 `text`。
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
* 西里尔字母 `и` 在 `non_cjk` charset_table（默认）中不再映射为 `i`，因为这对俄语词干提取和词形还原影响太大。
* `read_timeout`。请改用 [network_timeout](Server_settings/Searchd.md#network_timeout)，它控制读取和写入。


### 软件包

* Ubuntu Focal 20.04 官方软件包
* deb 软件包名称从 `manticore-bin` 改为 `manticore`

### Bug 修复：
1. [Issue #351](https://github.com/manticoresoftware/manticoresearch/issues/351) searchd 内存泄漏
2. [Commit ceab](https://github.com/manticoresoftware/manticoresearch/commit/ceabe44f) 片段中微小的读取越界
3. [Commit 1c3e](https://github.com/manticoresoftware/manticoresearch/commit/1c3e84a3) 危险地写入局部变量导致查询崩溃
4. [Commit 26e0](https://github.com/manticoresoftware/manticoresearch/commit/26e094ab) 测试 226 中 sorter 的微小内存泄漏
5. [Commit d2c7](https://github.com/manticoresoftware/manticoresearch/commit/d2c7f86a) 测试 226 中大规模内存泄漏
6. [Commit 0dd8](https://github.com/manticoresoftware/manticoresearch/commit/0dd80122) 集群显示节点同步，但 `count(*)` 显示不同数字
7. [Commit f1c1](https://github.com/manticoresoftware/manticoresearch/commit/f1c1ac3f) 美观：日志中重复且有时丢失警告消息
8. [Commit f1c1](https://github.com/manticoresoftware/manticoresearch/commit/f1c1ac3f) 美观：日志中出现 (null) 索引名称
9. [Commit 359d](https://github.com/manticoresoftware/manticoresearch/commit/359dbd30) 无法检索超过 7000 万条结果
10. [Commit 19f3](https://github.com/manticoresoftware/manticoresearch/commit/19f328ee) 无法插入无列语法的 PQ 规则
11. [Commit bf68](https://github.com/manticoresoftware/manticoresearch/commit/bf685d5d) 插入集群索引文档时误导性的错误消息
12. [Commit 2cf1](https://github.com/manticoresoftware/manticoresearch/commit/2cf18c83) `/json/replace` 和 `json/update` 返回指数形式的 id
13. [Issue #324](https://github.com/manticoresoftware/manticoresearch/issues/324) 同一查询中更新 json 标量属性和 mva
14. [Commit d384](https://github.com/manticoresoftware/manticoresearch/commit/d38409eb) `hitless_words` 在 RT 模式无效
15. [Commit 5813](https://github.com/manticoresoftware/manticoresearch/commit/5813d639) RT 模式下应禁止 `ALTER RECONFIGURE`
16. [Commit 5813](https://github.com/manticoresoftware/manticoresearch/commit/5813d639) searchd 重启后 `rt_mem_limit` 被重置为 128M
17. highlight() 有时会挂起
18. [Commit 7cd8](https://github.com/manticoresoftware/manticoresearch/commit/7cd878f4) RT 模式无法使用 U+ 码
19. [Commit 2b21](https://github.com/manticoresoftware/manticoresearch/commit/2b213de4) RT 模式下 wildcard 无法在 wordforms 中使用
20. [Commit e9d0](https://github.com/manticoresoftware/manticoresearch/commit/e9d07e68) 修复 `SHOW CREATE TABLE` 与多个 wordform 文件的冲突
21. [Commit fc90](https://github.com/manticoresoftware/manticoresearch/commit/fc90a84f) JSON 查询无 "query" 字段导致 searchd 崩溃
22. Manticore [官方 Docker](https://hub.docker.com/r/manticoresearch/manticore) 无法从 mysql 8 索引
23. [Commit 23e0](https://github.com/manticoresoftware/manticoresearch/commit/23e05d32) HTTP /json/insert 需要 id
24. [Commit bd67](https://github.com/manticoresoftware/manticoresearch/commit/bd679af0) `SHOW CREATE TABLE` 不支持 PQ
25. [Commit bd67](https://github.com/manticoresoftware/manticoresearch/commit/bd679af0) `CREATE TABLE LIKE` 对 PQ 不完全支持
26. [Commit 5eac](https://github.com/manticoresoftware/manticoresearch/commit/5eacf28f) show index status 中设置结尾的换行符问题
27. [Commit cb15](https://github.com/manticoresoftware/manticoresearch/commit/cb153228) HTTP JSON 响应中 "highlight" 的标题为空
28. [Issue #318](https://github.com/manticoresoftware/manticoresearch/issues/318) `CREATE TABLE LIKE` 中的中缀错误
29. [Commit 9040](https://github.com/manticoresoftware/manticoresearch/commit/9040d22c) RT 模式负载下崩溃
30. [cd512c7d ](https://github.com/manticoresoftware/manticoresearch/commit/cd512c7d ) RT 磁盘块崩溃时丢失崩溃日志
31. [Issue #323](https://github.com/manticoresoftware/manticoresearch/issues/323) 导入表失败并关闭连接
32. [Commit 6275](https://github.com/manticoresoftware/manticoresearch/commit/6275316a) `ALTER reconfigure` 损坏 PQ 索引
33. [Commit 9c1d](https://github.com/manticoresoftware/manticoresearch/commit/9c1d221e) searchd 在更改索引类型后重新加载问题
34. [Commit 71e2](https://github.com/manticoresoftware/manticoresearch/commit/71e2b5bb) 守护进程在导入缺失文件的表时崩溃
35. [Issue #322](https://github.com/manticoresoftware/manticoresearch/issues/322) 使用多个索引、group by 和 ranker=none 查询时崩溃
36. [Commit c3f5](https://github.com/manticoresoftware/manticoresearch/commit/c3f58490) `HIGHLIGHT()` 无法高亮字符串属性
37. [Issue #320](https://github.com/manticoresoftware/manticoresearch/issues/320) `FACET` 无法在字符串属性上排序
38. [Commit 4f1a](https://github.com/manticoresoftware/manticoresearch/commit/4f1a1f25) 缺少数据目录时的错误
39. [Commit 04f4](https://github.com/manticoresoftware/manticoresearch/commit/04f4ddd4) RT 模式不支持 access_*
40. [Commit 1c06](https://github.com/manticoresoftware/manticoresearch/commit/1c0616a2) 字符串中 JSON 对象错误：1. `CALL PQ` 返回 "Bad JSON objects in strings: 1" 当 json 大于某个值。
41. [Commit 32f9](https://github.com/manticoresoftware/manticoresearch/commit/32f943d6) RT 模式不一致。在某些情况下无法删除索引因为未知，且无法创建因为目录非空。
42. [Issue #319](https://github.com/manticoresoftware/manticoresearch/issues/319) 查询时崩溃
43. [Commit 22a2](https://github.com/manticoresoftware/manticoresearch/commit/22a28dd7) `max_xmlpipe2_field` = 2M 对 2M 字段产生警告
44. [Issue #342](https://github.com/manticoresoftware/manticoresearch/issues/342) 查询条件执行BUG
45. [提交 dd8d](https://github.com/manticoresoftware/manticoresearch/commit/dd8dcab2) 简单的两个词搜索能找到只包含一个词的文档
46. [提交 9091](https://github.com/manticoresoftware/manticoresearch/commit/90919e62) 在PQ中无法匹配键中带大写字母的json
47. [提交 56da](https://github.com/manticoresoftware/manticoresearch/commit/56da086a) 索引器在csv+docstore时崩溃
48. [问题 #363](https://github.com/manticoresoftware/manticoresearch/issues/363) 在centos 7的json属性中使用`[null]`导致插入数据损坏
49. 重大[问题 #345](https://github.com/manticoresoftware/manticoresearch/issues/345) 记录未被插入，count()结果随机，"replace into"返回OK
50. max_query_time导致SELECT查询变慢过多
51. [问题 #352](https://github.com/manticoresoftware/manticoresearch/issues/352) Mac OS下主-代理通信失败
52. [问题 #328](https://github.com/manticoresoftware/manticoresearch/issues/328) 使用Connector.Net/Mysql 8.0.19连接Manticore时报错
53. [提交 daa7](https://github.com/manticoresoftware/manticoresearch/commit/daa760d2fe83668038318f0f384ceefc0d075e18) 修复了对\0的转义并优化了性能
54. [提交 9bc5](https://github.com/manticoresoftware/manticoresearch/commit/9bc5c01a493db993b90ff6c1ad51aeae028f629c) 修正了count distinct与json的兼容问题
55. [提交 4f89](https://github.com/manticoresoftware/manticoresearch/commit/4f89a965a3af0db548ae9d1763731ebaebe21f5a) 修复在其他节点删除表失败的问题
56. [提交 952a](https://github.com/manticoresoftware/manticoresearch/commit/952af5a5931fd72b87edc169588777778b365231) 修复紧密调用pq时崩溃的问题


## 版本 3.4.2，2020年4月10日
### 关键漏洞修复
* [提交 2ffe](https://github.com/manticoresoftware/manticoresearch/commit/2ffe2d261793f9681265bc4abe38d695e72c121e) 修复旧版本RT索引无法索引数据的问题

## 版本 3.4.0，2020年3月26日
### 主要变更
* 服务器支持两种模式：rt模式和plain模式
   * rt模式需要配置data_dir且配置中无索引定义
   * plain模式索引在配置中定义；不允许data_dir
* 复制功能仅在rt模式可用

### 次要变更
* charset_table默认使用non_cjk别名
* rt模式下全文字段默认被索引和存储
* rt模式下全文字段名从'field'改为'text'
* ALTER RTINDEX更名为ALTER TABLE
* TRUNCATE RTINDEX更名为TRUNCATE TABLE

### 新功能
* 仅存储字段
* SHOW CREATE TABLE，IMPORT TABLE

### 改进
* 更快的无锁PQ
* /sql可在mode=raw模式执行任何类型的SQL语句
* 为mysql41协议添加mysql别名
* data_dir中的默认state.sql文件

### 修复BUG
* [提交 a533](https://github.com/manticoresoftware/manticoresearch/commit/a5333644e8e0c69746a9f24da8221fc965478e4a) 修复highlight()中错误字段语法导致崩溃
* [提交 7fbb](https://github.com/manticoresoftware/manticoresearch/commit/7fbb9f2e618096289cba7f8b834530b46aa94055) 修复复制带docstore的RT索引时服务器崩溃
* [提交 24a0](https://github.com/manticoresoftware/manticoresearch/commit/24a04687eb43b4f4719c7c4a5fe3c19099240467) 修复对启用infix或prefix选项且无存储字段索引的highlight崩溃
* [提交 3465](https://github.com/manticoresoftware/manticoresearch/commit/3465c1ce9c860c09f6ccc5b83a63e3c68c241d8d) 修复空docstore和空索引的doc-id查找错误提示问题
* [提交 a707](https://github.com/manticoresoftware/manticoresearch/commit/b8601b4144b9092be43a7caa7cd6b19cb37e14c7) 修复带尾随分号的SQL插入命令 (#314)
* [提交 9562](https://github.com/manticoresoftware/manticoresearch/commit/95628c9b9b8f16fe1bd18599a84e9673575adf1a) 移除查询词不匹配的警告
* [提交 b860](https://github.com/manticoresoftware/manticoresearch/commit/b8601b4144b9092be43a7caa7cd6b19cb37e14c7) 修复在片段中通过ICU分词的查询问题
* [提交 5275](https://github.com/manticoresoftware/manticoresearch/commit/5275516ca4631c584e6e0127fa161a18d2658fd0) 修复docstore块缓存中查找/添加的竞态条件
* [提交 f06e](https://github.com/manticoresoftware/manticoresearch/commit/f06ef97af5c7390427e82525c150535290e42c3e) 修复docstore中的内存泄漏
* [提交 a725](https://github.com/manticoresoftware/manticoresearch/commit/a7258ba8340e246c0d8aaeb4a7a74129eba807eb) 修复INSERT语句中LAST_INSERT_ID返回空 (#316)
* [提交 1ebd](https://github.com/manticoresoftware/manticoresearch/commit/1ebd5bf8d74113bcfa138ce63ce11966990a4215) 修复json/update HTTP端点支持MVA数组和JSON属性对象 (#317)
* [提交 e426](https://github.com/manticoresoftware/manticoresearch/commit/e426950a536382a142e2a42ed207c341149ff685) 修复索引器无明确id时乱倒rt索引问题

## 版本 3.3.0，2020年2月4日
### 新功能
* 并行实时索引搜索
* EXPLAIN QUERY 命令
* 无索引定义的配置文件（alpha版）
* CREATE/DROP TABLE 命令（alpha版）
* indexer --print-rt - 可读取源并打印Real-Time索引的INSERT语句

### 改进
* 升级至Snowball 2.0词干提取器
* SHOW INDEX STATUS支持LIKE过滤器
* 对高max_matches优化内存使用
* SHOW INDEX STATUS为RT索引添加ram_chunks_count
* 无锁PQ
* 将LimitNOFILE改为65536


### 修复BUG
* [提交 9c33](https://github.com/manticoresoftware/manticoresearch/commit/9c33aab8c1fd9059c8644d23d5f58f03010043c6) 添加索引模式重复属性检查 #293
* [提交 a008](https://github.com/manticoresoftware/manticoresearch/commit/a0085f9477e191da9e0e835b91583fb9cfde698e) 修复hitless terms时崩溃
* [提交 6895](https://github.com/manticoresoftware/manticoresearch/commit/68953740af8dca1833026d0f448c71416ed3044a) 修复ATTACH后docstore松散问题
* [提交 d6f6](https://github.com/manticoresoftware/manticoresearch/commit/d6f696edea53ea2b9a68251e4054e4816082c285) 修复分布式环境中的docstore问题
* [Commit bce2](https://github.com/manticoresoftware/manticoresearch/commit/bce2b7ec883d208f31094ee42a5b6f01d18a998f) 在排序器中将 FixedHash 替换为 OpenHash
* [Commit e0ba](https://github.com/manticoresoftware/manticoresearch/commit/e0baf7392937969e640fb5d10e8ccb312fe36446) 修复索引定义中属性名重复的问题
* [Commit ca81](https://github.com/manticoresoftware/manticoresearch/commit/ca81114bf1ac7ea7adcc8509bdc14d1f30b4f461) 修复 HIGHLIGHT() 中的 html_strip
* [Commit 493a](https://github.com/manticoresoftware/manticoresearch/commit/493a5e916d70fd4a3f14d1ee487234434f4a7ce3) 修复 HIGHLIGHT() 中的 passage 宏
* [Commit a82d](https://github.com/manticoresoftware/manticoresearch/commit/a82d41c775692f837f267d554d7d4a4b499d69f0) 修复 RT 索引创建小或大磁盘块时的双缓冲问题
* [Commit a404](https://github.com/manticoresoftware/manticoresearch/commit/a404c85de0d41d9d1c75b7540479d8a3af9e61aa) 修复 kqueue 的事件删除问题
* [Commit 8bea](https://github.com/manticoresoftware/manticoresearch/commit/8bea0f6fd1666b817995c4dd6e67f6f1c9161638) 修复当 RT 索引的 rt_mem_limit 取大值时磁盘块保存问题
* [Commit 8707](https://github.com/manticoresoftware/manticoresearch/commit/8707f0395b7ace52dc0cba9c8ed92a2bc4392932) 修复索引时浮点数溢出
* [Commit a564](https://github.com/manticoresoftware/manticoresearch/commit/a56434ce71bb7714b4d8e75278aaa075b27ece80) 修复向 RT 索引插入负 ID 文档时现在会报错
* [Commit bbeb](https://github.com/manticoresoftware/manticoresearch/commit/bbebfd7551a68641154761cac962053b54806759) 修复服务器在使用 ranker fieldmask 时崩溃
* [Commit 3809](https://github.com/manticoresoftware/manticoresearch/commit/3809cc1b0863e3a5a200dc423be3f7c38be56113) 修复使用查询缓存时崩溃
* [Commit dc2a](https://github.com/manticoresoftware/manticoresearch/commit/dc2a585bc724678679e4297362200bbd49ce32bb) 修复使用并行插入的 RT 索引 RAM 段时崩溃

## 版本 3.2.2，2019年12月19日
### 新功能
* RT 索引支持自增 ID
* 通过新的 HIGHLIGHT() 函数支持文档存储的高亮显示，也可用于 HTTP API
* SNIPPET() 可以使用特殊函数 QUERY()，返回当前的 MATCH 查询
* 高亮函数新增 field_separator 选项

### 改进与变更
* 远程节点的存储字段采用延迟获取（显著提升性能）
* 字符串和表达式不再破坏多查询和 FACET 优化
* RHEL/CentOS 8 构建现在使用 mariadb-connector-c-devel 提供的 mysql libclient
* ICU 数据文件已随包分发，移除 icu_data_dir
* systemd 服务文件包含 'Restart=on-failure' 策略
* indextool 可在线检测实时索引
* 默认配置文件改为 /etc/manticoresearch/manticore.conf
* RHEL/CentOS 服务名称由 'searchd' 改为 'manticore'
* 移除 query_mode 和 exact_phrase snippet 的选项

### Bug 修复
* [Commit 6ae4](https://github.com/manticoresoftware/manticoresearch/commit/6ae474c7894a6bee222d5b18e59a44fdbf57843a) 修复通过 HTTP 接口执行 SELECT 查询时崩溃
* [Commit 5957](https://github.com/manticoresoftware/manticoresearch/commit/59577513a49eac5a4a3c5e2cb38394d3246b5d35) 修复 RT 索引保存磁盘块但未标记部分文档为已删除
* [Commit e861](https://github.com/manticoresoftware/manticoresearch/commit/e861f0fca0e88924450695d4e9d6acff7a36558a) 修复多索引或多查询使用 dist_threads 搜索时崩溃
* [Commit 4409](https://github.com/manticoresoftware/manticoresearch/commit/440991fc977b8479800b45cdbc862eeb1ba5d965) 修复生成中缀时，长词包含宽 UTF8 码点导致崩溃
* [Commit 5fd5](https://github.com/manticoresoftware/manticoresearch/commit/5fd599b48bba527c023e6aa0b262dca51ffb8a1c) 修复添加套接字到 IOCP 时竞态条件
* [Commit cf10](https://github.com/manticoresoftware/manticoresearch/commit/cf10d7d3589f77adfaefbe52a51777bc9c67cf99) 修复布尔查询与 JSON SELECT 列表的问题
* [Commit 996d](https://github.com/manticoresoftware/manticoresearch/commit/996de77f4c3ec103d965e28c36098fd07dba50c4) 修复 indextool 检查错误跳过列表偏移及 doc2row 查找
* [Commit 6e3f](https://github.com/manticoresoftware/manticoresearch/commit/6e3fc9e88941c9427410f9d0adeebb7dd09a900f) 修复大数据情况下索引器用负跳过列表偏移生成错误索引
* [Commit faed](https://github.com/manticoresoftware/manticoresearch/commit/faed3220b423a96401982bf47117edf1f62c584e) 修复 JSON 仅转换数字到字符串以及表达式中的 JSON 字符串转数字
* [Commit 5331](https://github.com/manticoresoftware/manticoresearch/commit/533197200249d648ebcbdaca5d605861ee566348) 修复 indextool 多命令行参数时错误退出问题
* [Commit 7955](https://github.com/manticoresoftware/manticoresearch/commit/795520ac351d22e4497ebaf6c249bf25081842b6) 修复 #275 磁盘空间不足时 binlog 状态无效
* [Commit 2284](https://github.com/manticoresoftware/manticoresearch/commit/2284da5e3ff77bf7be4038a4d950c0580e4159c8) 修复 #279 JSON 属性使用 IN 过滤器时崩溃
* [Commit ce2e](https://github.com/manticoresoftware/manticoresearch/commit/ce2e4b4765111a7d331ffc256911f12770a8942d) 修复 #281 错误的管道关闭调用
* [Commit 5355](https://github.com/manticoresoftware/manticoresearch/commit/535589ba8a37baf5c6056afb5ff969fd70476feb) 修复调用带有递归 JSON 属性编码为字符串的 PQ 时服务器挂起
* [Commit a5fc](https://github.com/manticoresoftware/manticoresearch/commit/a5fc8a36e1644ee051632408ab8b53e79aeb9ff4) 修复 multiand 节点中在文档列表末尾推进问题
* [Commit a362](https://github.com/manticoresoftware/manticoresearch/commit/a3628617ef4ee89a811d4ba2c4a04cbbc298c1b5) 修复线程公共信息获取
* [Commit f8d2](https://github.com/manticoresoftware/manticoresearch/commit/f8d2d7bbbb90af3d2ee97424f97b05927f09d42b) 修复文档存储缓存锁

## 版本 3.2.0，2019年10月17日
### 新功能
* 文档存储
* 新增指令 stored_fields、docstore_cache_size、docstore_block_size、docstore_compression、docstore_compression_level

### 改进与变更
* 改善了 SSL 支持
* 更新了 non_cjk 内置字符集
* 禁止 UPDATE/DELETE 语句在查询日志中记录 SELECT
* 增加 RHEL/CentOS 8 软件包

### 修复问题
* [Commit 301a](https://github.com/manticoresoftware/manticoresearch/commit/301a806b14725e842300ee225521407464374c41) 修复在 RT 索引磁盘块中替换文档时崩溃问题
* [Commit 46c1](https://github.com/manticoresoftware/manticoresearch/commit/46c1cad8fa11c1fc09cc34b20d978fdfcd4e6d27) 修复 \#269 LIMIT N OFFSET M 问题
* [Commit 92a4](https://github.com/manticoresoftware/manticoresearch/commit/92a46edaa196ef33f3ac2225fbdba2c2b88ee45d) 修复 DELETE 语句在显式设置 id 或提供 id 列表跳过搜索时的问题
* [Commit 8ca7](https://github.com/manticoresoftware/manticoresearch/commit/8ca78c138d837caab30dc6e2343a3c4d1687bb87) 修复 Windows poller 的 netloop 移除事件后索引错误
* [Commit 6036](https://github.com/manticoresoftware/manticoresearch/commit/603631e2bff5cb0acf25f68b85621742490eccd9) 修复通过 HTTP 访问 JSON 时浮点数四舍五入问题
* [Commit 62f6](https://github.com/manticoresoftware/manticoresearch/commit/62f64cb9e75ec28fc3c29f8414ce9656f5580254) 修正远程片段先检查空路径；修复 Windows 测试
* [Commit aba2](https://github.com/manticoresoftware/manticoresearch/commit/aba274c2c5abc1029894ba9f95b786bda93e8f22) 修复配置重载以便 Windows 下表现同 Linux 一致
* [Commit 6b8c](https://github.com/manticoresoftware/manticoresearch/commit/6b8c4242ef77c3fa4d0ccb7d76d81714b6728f0b) 修复 \#194 PQ 支持形态和词干处理器
* [Commit 174d](https://github.com/manticoresoftware/manticoresearch/commit/174d312905285aa5746f4f404c53e6dc669ef42f) 修复 RT 退役段管理

## 版本 3.1.2，2019 年 8 月 22 日
### 功能与改进
* HTTP API 实验性 SSL 支持
* CALL KEYWORDS 的字段过滤器
* /json/search 的 max_matches
* 自动调整默认 Galera gcache.size 大小
* 改进 FreeBSD 支持

### 修复问题
* [Commit 0a1a](https://github.com/manticoresoftware/manticoresearch/commit/0a1a2c81038794983b4e30c8aefc7cc20e89b03f) 修复将 RT 索引复制到已存在且路径不同的节点出错
* [Commit 4adc](https://github.com/manticoresoftware/manticoresearch/commit/4adc075294ac823289f745e2cc419f18c7dcf2e2) 修正无活动索引的刷新重新调度
* [Commit d6c0](https://github.com/manticoresoftware/manticoresearch/commit/d6c00a6f750c04021747e9c99e0f77707a4cbfc3) 改进 RT/PQ 索引刷新重新调度
* [Commit d0a7](https://github.com/manticoresoftware/manticoresearch/commit/d0a7c95906b9b5ba33c98267d54501cfe27508d6) 修复 \#250 TSV 和 CSV 管道源的 index_field_lengths 索引选项
* [Commit 1266](https://github.com/manticoresoftware/manticoresearch/commit/1266d54800242709fbe2c34e72eb69b6595b23bd) 修正空索引下 indextool 对块索引检查报错
* [Commit 553c](https://github.com/manticoresoftware/manticoresearch/commit/553ca73ca111f28e0888d59cd48b0d6df181ecd6) 修正 Manticore SQL 查询日志的空选择列表
* [Commit 56c8](https://github.com/manticoresoftware/manticoresearch/commit/56c8584456293665ddd4ce4c94307d74473a78b9) 修复 indexer 的 -h/--help 响应

## 版本 3.1.0，2019 年 7 月 16 日
### 功能与改进
* 实时（RealTime）索引的复制支持
* 针对中文的 ICU 分词器
* 新形态学选项 icu_chinese
* 新指令 icu_data_dir
* 复制的多语句事务支持
* LAST_INSERT_ID() 与 @session.last_insert_id 支持
* SHOW VARIABLES 支持 LIKE 'pattern'
* 允许多文档插入到 percolate 索引
* 配置新增时间解析器
* 内部任务管理器
* 对文档和命中列表组件使用 mlock
* 限制片段路径

### 移除
* 放弃 RLP 库支持，改用 ICU；所有 rlp\* 指令移除
* 禁止通过 UPDATE 更新文档 ID

### 修复问题
* [Commit f047](https://github.com/manticoresoftware/manticoresearch/commit/f047222339fd0e62a07e9ca8b2a829297d5db861) 修复 concat 和 group_concat 缺陷
* [Commit b081](https://github.com/manticoresoftware/manticoresearch/commit/b08147ee24712b5eaa51f261626b5d16c6242fdd) 修正 percolate 索引中查询 uid 属性类型为 BIGINT
* [Commit 4cd8](https://github.com/manticoresoftware/manticoresearch/commit/4cd85afaf76eef0ce9fa1cd073f3fed160279890) 修复预分配新的磁盘块失败时程序崩溃
* [Commit 1a55](https://github.com/manticoresoftware/manticoresearch/commit/1a551227886474054111088412f2b2396f485652) 在 ALTER 中添加缺失的 timestamp 数据类型
* [Commit f3a8](https://github.com/manticoresoftware/manticoresearch/commit/f3a8e096fd496a6b001d398aadbb781b2dbad929) 修复错误 mmap 读取导致崩溃
* [Commit 4475](https://github.com/manticoresoftware/manticoresearch/commit/447577115192db7ac7bfea70b5ba58ee2c61229f) 修复复制中集群锁的哈希问题
* [Commit ff47](https://github.com/manticoresoftware/manticoresearch/commit/ff476df955f057bd38e8b6ad6056325e9d6fbb9c) 修复复制中提供者泄漏
* [Commit 58dc](https://github.com/manticoresoftware/manticoresearch/commit/58dcbb779c4c30ca1560460c78e35ba114b83716) 修复 \#246 indexer 中未定义的 sigmask 问题
* [Commit 3dd8](https://github.com/manticoresoftware/manticoresearch/commit/3dd8278e747d534f69fe9c3d360eef188ab7b6d5) 修复 netloop 报告的竞态条件
* [Commit a02a](https://github.com/manticoresoftware/manticoresearch/commit/a02aae05a3fa3d1caf2821f43c7e237b6de76336) 为 HA 策略重新平衡器设定零间隙

## 版本 3.0.2，2019 年 5 月 31 日
### 改进
* 增加对文档和命中列表的 mmap 读取支持
* `/sql` HTTP 端点响应现与 `/json/search` 一致
* 新增指令 `access_plain_attrs`、`access_blob_attrs`、`access_doclists`、`access_hitlists`
* 新增复制配置指令 `server_id`

### 移除
* 移除 HTTP `/search` 端点

### 弃用
* `read_buffer`、`ondisk_attrs`、`ondisk_attrs_default`、`mlock` 被 `access_*` 指令取代

### 修复问题
* [Commit 849c](https://github.com/manticoresoftware/manticoresearch/commit/849c16e1040f382795ba0635cb48686f9db4f2d8) 允许在选择列表中使用以数字开头的属性名称
* [Commit 48e6](https://github.com/manticoresoftware/manticoresearch/commit/48e6c302ca37bec0bc49c79619c60bf787ed0708) 修复了 UDF 中的 MVAs，修复了 MVA 别名
* [Commit 0555](https://github.com/manticoresoftware/manticoresearch/commit/055586a9a2ebfa8874f68099c5bc798a145fd4ef) 修复了 \#187 在使用 SENTENCE 查询时的崩溃
* [Commit 93bf](https://github.com/manticoresoftware/manticoresearch/commit/93bf52f23f1c53c8a93a603387ca5463e1fc2ae8) 修复了 \#143：支持 MATCH() 周围的 ()
* [Commit 599e](https://github.com/manticoresoftware/manticoresearch/commit/599ee79ccd38cce0023a55cbce8f50c3dca62e38) 修复了在 ALTER cluster 语句中保存集群状态的问题
* [Commit 230c](https://github.com/manticoresoftware/manticoresearch/commit/230c321e277514b191629538eb3f02d219113d95) 修复了在 ALTER index 时带有 blob 属性导致服务器崩溃的问题
* [Commit 5802](https://github.com/manticoresoftware/manticoresearch/commit/5802b85aa93a81caea77073a3381ef912fdd677c) 修复了 \#196 按 id 过滤的问题
* [Commit 25d2](https://github.com/manticoresoftware/manticoresearch/commit/25d2dabda4a299a29bd375405862126da9b4bfac) 丢弃对模板索引的搜索
* [Commit 2a30](https://github.com/manticoresoftware/manticoresearch/commit/2a30d5b49d76faab031d195f533974938769b8e6) 修复了 SQL 响应中 id 列为常规 bigint 类型


## 版本 3.0.0，2019 年 5 月 6 日
### 功能和改进
* 新的索引存储。非标量属性每个索引不再受 4GB 大小限制
* attr_update_reserve 指令
* 可以使用 UPDATE 更新 String、JSON 和 MVAs
* killlists 在索引加载时应用
* killlist_target 指令
* 多重 AND 搜索加速
* 更佳的平均性能和 RAM 使用
* 用于升级由 2.x 创建的索引的 convert 工具
* CONCAT() 函数
* JOIN CLUSTER cluster AT 'nodeaddress:port'
* ALTER CLUSTER posts UPDATE nodes
* node_address 指令
* 在 SHOW STATUS 中打印的节点列表

### 行为变化
* 对于具有 killlists 的索引，服务器不再按照 conf 中定义的顺序轮换索引，而是遵循 killlist 目标链
* 搜索中索引的顺序不再决定 killlists 的应用顺序
* 文档 ID 现在为有符号大整数

### 移除的指令
* docinfo（现在始终为 extern）、inplace_docinfo_gap、mva_updates_pool

## 版本 2.8.2 GA，2019 年 4 月 2 日
### 功能和改进
* 用于 percolate 索引的 Galera 复制
* OPTION morphology

### 编译说明
Cmake 的最低版本现在为 3.13。编译需要 boost 和 libssl
开发库。

### Bug 修复
* [Commit 6967](https://github.com/manticoresoftware/manticoresearch/commit/6967fedb2ef818ec1c825d482563edd05e1c9245) 修复了在针对多个分布式索引进行查询时，选择列表中多个星号导致的崩溃
* [Commit 36df](https://github.com/manticoresoftware/manticoresearch/commit/36df1a407dc08263690e3492518613ace82d69ca) 修复了 [\#177](https://github.com/manticoresoftware/manticoresearch/issues/177) 通过 Manticore SQL 接口的大数据包问题
* [Commit 5793](https://github.com/manticoresoftware/manticoresearch/commit/57932aec734583fa93359faaf5034b2e2c9d352b) 修复了 [\#170](https://github.com/manticoresoftware/manticoresearch/issues/170) 在 RT 优化时更新 MVA 导致的服务器崩溃
* [Commit edb2](https://github.com/manticoresoftware/manticoresearch/commit/edb24b870423add86eba471d361e0e5aff098b18) 修复了在 SIGHUP 配置重载后移除 RT 索引导致 binlog 被移除时的服务器崩溃问题
* [Commit bd3e](https://github.com/manticoresoftware/manticoresearch/commit/bd3e66e0085bc7e2e351b817dfc972fd8158fbce) 修复了 mysql 握手身份验证插件的负载问题
* [Commit 6a21](https://github.com/manticoresoftware/manticoresearch/commit/6a217f6ed82fb10f752213b15617b9cedf1e8533) 修复了 [\#172](https://github.com/manticoresoftware/manticoresearch/issues/172) RT 索引上的 phrase_boundary 设置问题
* [Commit 3562](https://github.com/manticoresoftware/manticoresearch/commit/3562f652753e4091fd7b5b0f65b58341c8cbcb31) 修复了 [\#168](https://github.com/manticoresoftware/manticoresearch/issues/168) 在将索引 ATTACH 到自身时的死锁问题
* [Commit 250b](https://github.com/manticoresoftware/manticoresearch/commit/250b3f0e74ad18dda34ba080aa13ca87798a9ac9) 修复了服务器崩溃后 binlog 保存空元数据的问题
* [Commit 4aa6](https://github.com/manticoresoftware/manticoresearch/commit/4aa6c69ad3a181cd7c5dafb990fb528121f68e59) 修复了由于 RT 索引带有磁盘块的 sorter 中字符串导致的服务器崩溃

## 版本 2.8.1 GA，2019 年 3 月 6 日
### 功能和改进
* SUBSTRING_INDEX()
* 为 percolate 查询添加 SENTENCE 和 PARAGRAPH 支持
* 用于 Debian/Ubuntu 的 systemd 生成器；同时添加了 LimitCORE 以允许生成 core dump

### Bug 修复
* [Commit 84fe](https://github.com/manticoresoftware/manticoresearch/commit/84fe7405d17b59f4dcde15a6c8e8503923503def) 修复了在 match mode all 和空全文查询时服务器崩溃的问题
* [Commit daa8](https://github.com/manticoresoftware/manticoresearch/commit/daa88b579fa373d5d2e869e43a2e178363b0fef1) 修复了删除静态字符串时的崩溃
* [Commit 2207](https://github.com/manticoresoftware/manticoresearch/commit/22078537dddbd9ce81a182dcc879f235b6e87004) 修复了 indextool 在出现 FATAL 时的退出代码
* [Commit 0721](https://github.com/manticoresoftware/manticoresearch/commit/0721696d6780c200c65d596624a8187983fb7fcb) 修复了 [\#109](https://github.com/manticoresoftware/manticoresearch/issues/109) 由于错误的精确形式检查导致前缀无匹配的问题
* [Commit 8af8](https://github.com/manticoresoftware/manticoresearch/commit/8af810111b8e2f87bc9f378172eff1ab9725c7e7) 修复了 [\#161](https://github.com/manticoresoftware/manticoresearch/issues/161) RT 索引的配置设置重新加载问题
* [Commit e2d5](https://github.com/manticoresoftware/manticoresearch/commit/e2d592773f4fd702ace90070a793a842a0fec6ab) 修复了访问大型 JSON 字符串时服务器崩溃的问题
* [提交 75cd](https://github.com/manticoresoftware/manticoresearch/commit/75cd1342d05cdc352e3dd145cdbc6f79394a165b) 修复了索引剥离器修改的 JSON 文档中的 PQ 字段导致来自同级字段错误匹配的问题
* [提交 e2f7](https://github.com/manticoresoftware/manticoresearch/commit/e2f775437df922674d772ed4417780492502e65a) 修复了 RHEL7 构建上解析 JSON 时服务器崩溃的问题
* [提交 3a25](https://github.com/manticoresoftware/manticoresearch/commit/3a25a5808feb3f8b80866991b436d6c1241618c2) 修复了当斜杠位于边缘时 JSON 反转义崩溃的问题
* [提交 be9f](https://github.com/manticoresoftware/manticoresearch/commit/be9f497872bea62dbdccd64fc28294fd7776c289) 修正了参数 'skip_empty' 以跳过空文档并且不警告其非有效 JSON
* [提交 266e](https://github.com/manticoresoftware/manticoresearch/commit/266e0e7b088549722a805fc837bf101ff681a5e8) 修复了[#140](https://github.com/manticoresoftware/manticoresearch/issues/161) 使浮点数在六位精度不够时输出八位数
* [提交 3f6d](https://github.com/manticoresoftware/manticoresearch/commit/3f6d23891064ee8e94030c4231497cdd7da33a6a) 修复了空 jsonobj 创建
* [提交 f3c7](https://github.com/manticoresoftware/manticoresearch/commit/f3c7848a59b7eedc67c3403d6d4b37ce6fa94dc8) 修复了[#160](https://github.com/manticoresoftware/manticoresearch/issues/161) 空的 mva 输出 NULL 而非空字符串
* [提交 0afa](https://github.com/manticoresoftware/manticoresearch/commit/0afa2ed058e5759470b1d7354c722faab34f98bb) 修复了在无 pthread_getname_np 的情况下构建失败的问题
* [提交 9405](https://github.com/manticoresoftware/manticoresearch/commit/9405fccdeb27f0302a8d9a848981d30080216777) 修复了服务器关闭时线程池 worker 引起的崩溃

## 版本 2.8.0 GA，2019年1月28日
### 改进
* 针对穿透索引的分布式索引
* CALL PQ 新选项及变更：
    *   skip_bad_json
    *   mode（sparsed/sharded）
    *   JSON 文档可作为 JSON 数组传递
    *   shift
    *   列名 'UID', 'Documents', 'Query', 'Tags', 'Filters' 重命名为 'id', 'documents', 'query', 'tags', 'filters'
* 支持 DESCRIBE pq TABLE
* 不再支持 SELECT FROM pq WHERE UID，改用 'id'
* pq 索引上的 SELECT 功能已与常规索引持平（例如可通过 REGEX() 过滤规则）
* PQ 标签上可使用 ANY/ALL
* 表达式对 JSON 字段自动转换，无需显式类型转换
* 内置 'non_cjk' charset_table 和 'cjk' ngram_chars
* 内置50种语言的停用词集合
* 停用词声明中多个文件可用逗号分隔
* CALL PQ 可接受 JSON 文档数组

### Bug修复
* [提交 a4e1](https://github.com/manticoresoftware/manticoresearch/commit/a4e19afee54dafdc04b0da53372dafd8c0d332d6) 修复了与 csjon 相关的内存泄漏
* [提交 28d8](https://github.com/manticoresoftware/manticoresearch/commit/28d862774874891a03e361da1b0347ebe8066ce0) 修复了由于 JSON 中缺失值导致的崩溃
* [提交 bf4e](https://github.com/manticoresoftware/manticoresearch/commit/bf4e9ea27c349cdc9ae4e54e960a4a17b90e38fa) 修复了 RT 索引空元数据保存
* [提交 33b4](https://github.com/manticoresoftware/manticoresearch/commit/33b4573529e7c7c7bce19bf9d54054866f30d99a) 修复了词形还原序列丢失形态标志 (exact)
* [提交 6b95](https://github.com/manticoresoftware/manticoresearch/commit/6b95d48240d3b5520afade19c249d79e020a5e88) 修正字符串属性 > 4M 时使用饱和溢出而非溢出
* [提交 6214](https://github.com/manticoresoftware/manticoresearch/commit/621418b829e70af36aaa322a2f51ece3f86bc499) 修复了在禁用索引时服务器接收 SIGHUP 崩溃
* [提交 3f7e](https://github.com/manticoresoftware/manticoresearch/commit/3f7e35d1482966cc45d5abbcb2de5de3508d66b0) 修复了同时执行 API 会话状态命令时服务器崩溃
* [提交 cd9e](https://github.com/manticoresoftware/manticoresearch/commit/cd9e4f1709a48ddafc128c450c2d882bc11ba50e) 修复了删除带字段过滤的 RT 索引查询时服务器崩溃
* [提交 9376](https://github.com/manticoresoftware/manticoresearch/commit/9376470d455fcd256c6abff79c431a6919dfa7ac) 修复了在分布式索引中使用空文档调用 CALL PQ 导致服务器崩溃
* [提交 8868](https://github.com/manticoresoftware/manticoresearch/commit/8868b207644f4f8cc1ab0c270adb35493fed22e1) 修正 Manticore SQL 错误消息超过512字符被截断
* [提交 de9d](https://github.com/manticoresoftware/manticoresearch/commit/de9deda9c142823d6dbf529423417b5c670fae94) 修复了未启用 binlog 时保存穿透索引崩溃
* [提交 2b21](https://github.com/manticoresoftware/manticoresearch/commit/2b219e1a32791e7740bb210b7d408f96abc6374f) 修复了 OSX 平台的 HTTP 接口不可用
* [提交 e92c](https://github.com/manticoresoftware/manticoresearch/commit/e92c60240f8f9e1756bfa14fba0bbb402586a389) 修正了 indextool 检查 MVA 时错误的错误提示
* [提交 238b](https://github.com/manticoresoftware/manticoresearch/commit/238bdea59bad89f097403f1c978658ce45f16c70) 修复了 FLUSH RTINDEX 写锁，避免在保存和定期刷新时锁定整个索引
* [提交 c26a](https://github.com/manticoresoftware/manticoresearch/commit/c26a236bd42b7082db079a51a84172066d337d64) 修复了 ALTER 穿透索引时等待搜索加载卡住的问题
* [提交 9ee5](https://github.com/manticoresoftware/manticoresearch/commit/9ee5703f6c28f57cd5bff5e705966d93fe30d267) 修正 max_children 为 0 时使用线程池默认线程数
* [提交 5138](https://github.com/manticoresoftware/manticoresearch/commit/5138fc048a1bd146b271ce6a72fe954344281f69) 修复了在使用 index_token_filter 插件及停用词且 stopword_step=0 时向索引写入数据出错
* [提交 2add](https://github.com/manticoresoftware/manticoresearch/commit/2add3d319ac62fe450bf60e89033100853dc2ecf) 修复了在索引定义中仍使用 aot 词形还原器而缺失 lemmatizer_base 导致崩溃

## 版本 2.7.5 GA，2018年12月4日
### 改进
* REGEX 函数
* JSON API 搜索的 limit/offset
* qcache 的性能分析点

### Bug修复
* [Commit eb3c](https://github.com/manticoresoftware/manticoresearch/commit/eb3c7683b9740cff4cc4cadd2ab9f990b9f7d173) 修复了服务器在 FACET 使用多个属性宽类型时崩溃的问题
* [Commit d915](https://github.com/manticoresoftware/manticoresearch/commit/d915cf6696514f4be365ffb5981bccaa15983e65) 修复了 FACET 查询主选择列表中隐式 group by 的问题
* [Commit 5c25](https://github.com/manticoresoftware/manticoresearch/commit/5c25dc271a8c11a1c82ca3c88e518acbc6a8c164) 修复了使用 GROUP N BY 查询时的崩溃
* [Commit 85d3](https://github.com/manticoresoftware/manticoresearch/commit/85d30a232d4ffd89916b673a9b2db946d272a945) 修复了内存操作崩溃处理时的死锁问题
* [Commit 8516](https://github.com/manticoresoftware/manticoresearch/commit/85166b5ef2be38061d62f2b20f15acaa0311258a) 修复了 indextool 检查过程中的内存消耗问题
* [Commit 58fb](https://github.com/manticoresoftware/manticoresearch/commit/58fb03166830d8b23e7ab9ce56309db7ddac22f4) 修复了无需再包含 gmock，因上游已自行解决

## 版本 2.7.4 GA，2018年11月1日
### 改进
* 对于远程分布式索引，SHOW THREADS 会打印原始查询而非 API 调用
* SHOW THREADS 新增选项 `format=sphinxql` 以 SQL 格式打印所有查询
* SHOW PROFILE 增加了 `clone_attrs` 阶段的打印

### 修复
* [Commit 4f15](https://github.com/manticoresoftware/manticoresearch/commit/4f155712a0bccc1bd01cc191647bc8cff814888e) 修复了在无 malloc_stats、malloc_trim 的 libc 环境下构建失败的问题
* [Commit f974](https://github.com/manticoresoftware/manticoresearch/commit/f974f20bda3214a56877c393a192be1a77150958) 修复了 CALL KEYWORDS 结果集内单词中的特殊符号
* [Commit 0920](https://github.com/manticoresoftware/manticoresearch/commit/092083282ea8ae0b2e72fb6989c57ccec81e74ac) 修复通过 API 对分布式索引或远程 agent 进行 CALL KEYWORDS 调用失败的问题
* [Commit fd68](https://github.com/manticoresoftware/manticoresearch/commit/fd686bfe88b720ffd7642e36897ba45161cbd7d2) 修复了 distributed index 的 agent_query_timeout 未正确传递给 agent 作为 max_query_time 的问题
* [Commit 4ffa](https://github.com/manticoresoftware/manticoresearch/commit/4ffa623e9d357a3b0b441615089e211f92f8de32) 修复了磁盘分片上文档总数统计受 OPTIMIZE 命令影响，导致权重计算错误的问题
* [Commit dcaf](https://github.com/manticoresoftware/manticoresearch/commit/dcaf4e0e3c8e9b0fbec47bbf3307feddec0936a6) 修复了来自 blended 的 RT 索引多重尾部命中问题
* [Commit eee3](https://github.com/manticoresoftware/manticoresearch/commit/eee381754e902a43a8f499e1c950198a2a3e6ee0) 修复了旋转时的死锁问题

## 版本 2.7.3 GA，2018年9月26日
### 改进
* CALL KEYWORDS 支持 sort_mode 选项
* VIP 连接的 DEBUG 可以执行 'crash <password>' 来故意触发服务器的 SIGEGV
* DEBUG 可以执行 'malloc_stats' 将 malloc 状态转储到 searchd.log，'malloc_trim' 用于执行 malloc_trim()
* 改进了当系统存在 gdb 时的回溯信息质量

### 修复
* [Commit 0f3c](https://github.com/manticoresoftware/manticoresearch/commit/0f3cc333f6129451ad1ae2c97905b6c04ba5d454) 修复了 Windows 平台下 rename 操作崩溃或失败的问题
* [Commit 1455](https://github.com/manticoresoftware/manticoresearch/commit/1455ba2bf8079f03ce7f861a8d9662a360d705de) 修复了 32 位系统上服务器崩溃的问题
* [Commit ad37](https://github.com/manticoresoftware/manticoresearch/commit/ad3710d53b5cd6a28b8c60352d0f9e285b03d9fa) 修复了空 SNIPPET 表达式导致服务器崩溃或挂起的问题
* [Commit b36d](https://github.com/manticoresoftware/manticoresearch/commit/b36d79214364500252665860f000ae85343528b6) 修复了非渐进式优化失效的问题，并修正了渐进式优化不应为最旧磁盘分片创建 kill-list 的问题
* [Commit 34b0](https://github.com/manticoresoftware/manticoresearch/commit/34b032499afd42ce47a4c7247814b4031094388a) 修复了在线程池工作模式下，SQL 和 API 对 queue_max_length 返回错误的问题
* [Commit ae4b](https://github.com/manticoresoftware/manticoresearch/commit/ae4b3202cbdb8014cabe2b90e269d5cf74f49871) 修复了在 PQ 索引上添加带 regexp 或 rlp 选项的全扫描查询时崩溃的问题
* [Commit f80f](https://github.com/manticoresoftware/manticoresearch/commit/f80f8d5d7560187078868aed9a9575f4549e98aa) 修复了连续调用 PQ 导致崩溃的问题
* [Commit 9742](https://github.com/manticoresoftware/manticoresearch/commit/9742f5f0866af73f8cd8483ecd18a507ea80dd65) 重构了 AcquireAccum
* [Commit 39e5](https://github.com/manticoresoftware/manticoresearch/commit/39e5bc3751b7295222eb76407c5d72ce1dad545b) 修复了调用 pq 后的内存泄漏问题
* [Commit 21bc](https://github.com/manticoresoftware/manticoresearch/commit/21bcc6d17395f0f57dde79f6716ef303b7ea527d) 美化重构（使用 c++11 风格的构造函数、默认值、nullptr）
* [Commit 2d69](https://github.com/manticoresoftware/manticoresearch/commit/2d690398f14c736956cfdd66feb4d3091d6b3a4d) 修复尝试向 PQ 索引插入重复项时的内存泄漏问题
* [Commit 5ed9](https://github.com/manticoresoftware/manticoresearch/commit/5ed92c4b66da6423fa28f11b1fd59103ed1e1fb9) 修复了 JSON 字段 IN 查询处理大量值时崩溃的问题
* [Commit 4a52](https://github.com/manticoresoftware/manticoresearch/commit/4a5262e2776aa261e34ba5c914ea60804f59de3f) 修复了对设置了扩展限制的 RT 索引执行 CALL KEYWORDS 语句时服务器崩溃的问题
* [Commit 5526](https://github.com/manticoresoftware/manticoresearch/commit/552646bb6fefde4e2525298e43e628839b421d3d) 修复了 PQ 匹配查询中的无效过滤器
* [Commit 204f](https://github.com/manticoresoftware/manticoresearch/commit/204f521408b730198a5bab2c20407a3d7df94873) 引入了用于指针属性的小型对象分配器
* [Commit 2545](https://github.com/manticoresoftware/manticoresearch/commit/25453e5387152c0575d1eda1b2beb89a49c7493a) 重构了 ISphFieldFilter 使其支持引用计数风格
* [Commit 1366](https://github.com/manticoresoftware/manticoresearch/commit/1366ee06e828397074b69bec9265d7147170c60e) 修复了在非终止字符串上使用 strtod 时导致未定义行为或 SIGSEGV 的问题
* [Commit 94bc](https://github.com/manticoresoftware/manticoresearch/commit/94bc6fcbdae4d08bcf6da2966e3ec2966091289f) 修复了 json 结果集处理中的内存泄漏
* [Commit e78e](https://github.com/manticoresoftware/manticoresearch/commit/e78e9c948963416caff9e4e46296a58080107835) 修复了应用属性添加时对内存块末尾的超读
* [Commit fad5](https://github.com/manticoresoftware/manticoresearch/commit/fad572fb543606b33e688eb82a485b86011545fd) 重新构造了适用于引用计数版本的 CSphDict
* [Commit fd84](https://github.com/manticoresoftware/manticoresearch/commit/fd841a472b472280c93f4af3506851d19f6adc45) 修复了 AOT 内部类型外泄漏问题
* [Commit 5ee7](https://github.com/manticoresoftware/manticoresearch/commit/5ee7f208c7685f2ebb9e5623a8802b721e2a7f0a) 修复了分词器管理中的内存泄漏
* [Commit 116c](https://github.com/manticoresoftware/manticoresearch/commit/116c5f1abebee9a0d99afe93546a1d8e4c6c6958) 修复了 grouper 中的内存泄漏
* [Commit 56fd](https://github.com/manticoresoftware/manticoresearch/commit/56fdbc9ab38973a9a0ba8ccee45f71cf33c16423) 在匹配中对动态指针实现特殊的释放/复制（grouper 内存泄漏）
* [Commit b1fc](https://github.com/manticoresoftware/manticoresearch/commit/31db660f378541aa0d1e7cf2e7209bb0a1f47fd8) 修复了实时索引动态字符串的内存泄漏
* [Commit 517b](https://github.com/manticoresoftware/manticoresearch/commit/517b9e8749f092e923cd2884b674b5bb84e20755) 重新构造了 grouper
* [Commit b1fc](https://github.com/manticoresoftware/manticoresearch/commit/b1fc16140e5dc44290686330b476a254e0092107) 小幅重构（c++11 c-trs，一些格式调整）
* [Commit 7034](https://github.com/manticoresoftware/manticoresearch/commit/7034e073f4cf2844762e0a464b7c3de05d3d122b) 重新构造了 ISphMatchComparator 为引用计数版本
* [Commit b1fc](https://github.com/manticoresoftware/manticoresearch/commit/b1fc16140e5dc44290686330b476a254e0092107) 私有化克隆器
* [Commit efbc](https://github.com/manticoresoftware/manticoresearch/commit/efbc0511d6809c4a57453c7283f9bf53f3fb8d97) 简化了 MVA_UPSIZE、DOCINFO2ID_T、DOCINFOSETID 的本地小端处理
* [Commit 6da0](https://github.com/manticoresoftware/manticoresearch/commit/6da0df4ac9946fc59f98d229b90e12c9221dd4c0) 为 ubertests 添加了 valgrind 支持
* [Commit 1d17](https://github.com/manticoresoftware/manticoresearch/commit/1d17669cb7ec54ac80b392e101f37a688eb98080) 修复了由于连接上 'success' 标志的竞态导致的崩溃
* [Commit 5a09](https://github.com/manticoresoftware/manticoresearch/commit/5a09c32d7ded56b2c0bd3e2ad7968cb8d6362919) 将 epoll 切换为边缘触发模式
* [Commit 5d52](https://github.com/manticoresoftware/manticoresearch/commit/5d528682737fca03a4352a3093b1319ec27d598c) 修复了表达式中 IN 语句格式类似 at filter 的问题
* [Commit bd8b](https://github.com/manticoresoftware/manticoresearch/commit/bd8b3c976ff8b4667f55dfdd101b20a920137ac5) 修复了在提交具有大 docid 的文档时 RT 索引崩溃问题
* [Commit ce65](https://github.com/manticoresoftware/manticoresearch/commit/ce656b83b747ce7c55795b53915770c13fb99cfe) 修复了 indextool 中无参数选项的问题
* [Commit 08c9](https://github.com/manticoresoftware/manticoresearch/commit/08c9507177820f441f534bf06a11dac5e54bebb4) 修复了扩展关键字的内存泄漏
* [Commit 30c7](https://github.com/manticoresoftware/manticoresearch/commit/30c75a2f525ec9bda625d903acdc9d4d2e2413dc) 修复了 json grouper 的内存泄漏
* [Commit 6023](https://github.com/manticoresoftware/manticoresearch/commit/6023f269b6f2080e4d380b0957605ef8107c8c9f) 修复了全局用户变量泄漏
* [Commit 7c13](https://github.com/manticoresoftware/manticoresearch/commit/7c138f15ca23c0c8717fa12041e3db7f988fcf15) 修复了早期拒绝匹配时动态字符串泄漏
* [Commit 9154](https://github.com/manticoresoftware/manticoresearch/commit/9154b18eaed5302e8330cb3a95bd968959ccb312) 修复了 length(<expression>) 的泄漏问题
* [Commit 43fc](https://github.com/manticoresoftware/manticoresearch/commit/43fca3a4e26139765d0fac8de054200dc4875fc6) 修复了 parser 中 strdup() 导致的内存泄漏
* [Commit 71ff](https://github.com/manticoresoftware/manticoresearch/commit/71ff77737e63d1019b7c9804dca7fa2351025dba) 重新构造表达式解析器以准确追踪引用计数

## 版本 2.7.2 GA，2018年8月27日
### 改进
* 与 MySQL 8 客户端兼容
* 支持带 WITH RECONFIGURE 的 [TRUNCATE](Emptying_a_table.md)
* 退役了 RT 索引的 SHOW STATUS 中的内存计数器
* 多代理的全局缓存
* 改进了 Windows 平台上的 IOCP
* HTTP 协议的 VIP 连接
* Manticore SQL 的 [DEBUG](Reporting_bugs.md#DEBUG) 命令，可运行多种子命令
* [shutdown_token](Server_settings/Searchd.md#shutdown_token) - 运行 DEBUG 命令中的 `shutdown` 需要的密码 SHA1 哈希
* 新增 SHOW AGENT STATUS 的统计项 (_ping, _has_perspool, _need_resolve)
* indexer 的 --verbose 选项现在支持 \[debugvv\] 用于打印调试信息

### 错误修复
* [Commit 3900](https://github.com/manticoresoftware/manticoresearch/commit/390082a7be0a1f9539b30361d11d54de35c62a44) 移除了优化时的写锁
* [Commit 4c33](https://github.com/manticoresoftware/manticoresearch/commit/4c337648329be22373be33333cbc9c5664d18b80) 修复了重新加载索引设置时的写锁
* [Commit b5ea](https://github.com/manticoresoftware/manticoresearch/commit/b5ea8dc0b18a23dbd739e92c66dd93b0094df14b) 修复了使用 JSON 过滤器查询时的内存泄漏
* [Commit 930e](https://github.com/manticoresoftware/manticoresearch/commit/930e831a43fd3d1ce996185be968d1df49a088d0) 修复了 PQ 结果集中的空文档问题
* [Commit 53de](https://github.com/manticoresoftware/manticoresearch/commit/53deecfecb374431f84f8592a1095a77407b8aea) 修复了由于任务被移除导致的任务混淆
* [Commit cad9](https://github.com/manticoresoftware/manticoresearch/commit/cad9b9c617719aad97e9f5a2ae93491153d8e504) 修复了远程主机计数错误
* [Commit 9000](https://github.com/manticoresoftware/manticoresearch/commit/90008cc15c4684d445d37a391bb4a667bbc2e365) 修复了解析的代理描述符内存泄漏
* [Commit 978d](https://github.com/manticoresoftware/manticoresearch/commit/978d839641ad6040c22c9fc3b703b1a02685f266) 修复了搜索中的内存泄漏
* [Commit 0193](https://github.com/manticoresoftware/manticoresearch/commit/0193946105ac08913362b7d7acf0c90f9b38dc85) 对显式/内联构造函数、override/final用法进行了代码美化
* [Commit 943e](https://github.com/manticoresoftware/manticoresearch/commit/943e2997b33f9b2eda23d94a66068910f9476ebd) 修复了本地/远程schema中json的内存泄漏
* [Commit 02db](https://github.com/manticoresoftware/manticoresearch/commit/02dbdd6f1d0b72e7e458ebebdd2c67f989577e6b) 修复了本地/远程schema中json排序列表达式的内存泄漏
* [Commit c74d](https://github.com/manticoresoftware/manticoresearch/commit/c74d0b4035e4214510376845b22cb676a8da24a3) 修复了常量别名的内存泄漏
* [Commit 6e5b](https://github.com/manticoresoftware/manticoresearch/commit/6e5b57e1d3952c9695376728a143bf5434208f53) 修复了预读取线程的内存泄漏
* [Commit 39c7](https://github.com/manticoresoftware/manticoresearch/commit/39c740fef5bf71da0bee60a35f5dd9f471af850f) 修复了由于netloop中等待阻塞导致的退出卡死问题
* [Commit adaf](https://github.com/manticoresoftware/manticoresearch/commit/adaf9772f95772ad6d5297acace6c5cb92b56ad5) 修复了HA代理切换到普通主机时“ping”行为卡死的问题
* [Commit 32c4](https://github.com/manticoresoftware/manticoresearch/commit/32c40eac9cdd40b15d79e58b2f15416f164f9701) 为仪表板存储分离垃圾回收机制
* [Commit 511a](https://github.com/manticoresoftware/manticoresearch/commit/511a3cf2c1f1858f6e94fe9f8175b7216db3cbd6) 修复了引用计数指针问题
* [Commit 32c4](https://github.com/manticoresoftware/manticoresearch/commit/af2326cd2927effbad7bec88e8dc238a566cf27e) 修复了indextool在索引不存在时崩溃的问题
* [Commit 156e](https://github.com/manticoresoftware/manticoresearch/commit/156edc717dc63d3ea120f42466eba6e022da88be) 修复了xmlpipe索引中过滤器/字段超出的输出名称
* [Commit cdac](https://github.com/manticoresoftware/manticoresearch/commit/cdac6d1f17d384bfc8bae49e91241b2f346c3830) 修复了配置中无索引器段时默认索引器的值
* [Commit e61e](https://github.com/manticoresoftware/manticoresearch/commit/e61ec00b6b27d1d5878247e2ee817f3b1e7fde16) 修复了服务器重启后RT索引中磁盘块错误嵌入的停用词
* [Commit 5fba](https://github.com/manticoresoftware/manticoresearch/commit/5fba49d5e28c17de4c0acbd984466127f42de6e8) 修复了跳过幽灵连接（已关闭但未从轮询器中彻底删除）
* [Commit f22a](https://github.com/manticoresoftware/manticoresearch/commit/f22ae34b623906b3c5a05a06198dad2e548dc541) 修复了混合（孤立）的网络任务
* [Commit 4689](https://github.com/manticoresoftware/manticoresearch/commit/46890e70eb8dcd00c1bf1b030538b8f131c601c2) 修复了写操作后读取动作崩溃
* [Commit 03f9](https://github.com/manticoresoftware/manticoresearch/commit/03f9df904f611c438d5ebcaaeef911b0dc8d6c39) 修复了Windows上运行测试时searchd崩溃
* [Commit e925](https://github.com/manticoresoftware/manticoresearch/commit/e9255ed2704790a3d7f5273d38b85433463cb3ff) 修复了普通connect()时处理EINPROGRESS代码
* [Commit 248b](https://github.com/manticoresoftware/manticoresearch/commit/248b72f1544b27e9e82773284050d44eeb1714e1) 修复了使用TFO时连接超时问题

## 版本 2.7.1 GA，2018年7月4日
### 改进
* 改进了在PQ中匹配多个文档时的通配符性能
* 支持PQ上的全扫描查询
* 支持PQ上的MVA属性
* 支持percolate索引的正则和RLP

### 修复
* [Commit 6885](https://github.com/manticoresoftware/manticoresearch/commit/68856261b41f6950666f9e5122726839fb3c71d0) 修复了查询字符串丢失的问题
* [Commit 0f17](https://github.com/manticoresoftware/manticoresearch/commit/0f1770943cba53bfd2e0edfdf0b1495ff0dd0e89) 修复了SHOW THREADS语句中信息为空的问题
* [Commit 53fa](https://github.com/manticoresoftware/manticoresearch/commit/53faa36a916bb87868e83aa79c1a1f972fb20ca0) 修复了使用NOTNEAR操作符匹配时崩溃的问题
* [Commit 2602](https://github.com/manticoresoftware/manticoresearch/commit/26029a7cd77b518fb3a27588d7db6c8bffc73b0f) 修复了PQ删除时错误的过滤器错误消息


## 版本 2.7.0 GA，2018年6月11日
### 改进
* 降低了系统调用次数，减轻Meltdown和Spectre补丁的影响
* 重写了本地索引管理
* 远程摘要重构
* 支持完整配置重载
* 所有节点连接现在均独立
* 协议改进
* Windows通信从wsapoll切换到IO完成端口
* 支持主节点与节点间使用TFO通信
* [SHOW STATUS](Node_info_and_management/Node_status.md#SHOW-STATUS) 现在输出服务器版本和mysql_version_string
* 为调用CALL PQ的文档添加了 `docs_id` 选项
* percolate查询过滤器现在可以包含表达式
* 分布式索引支持FEDERATED
* 虚拟SHOW NAMES COLLATE和`SET wait_timeout`（提升ProxySQL兼容性）

### 修复
* [Commit 5bcf](https://github.com/manticoresoftware/manticoresearch/commit/5bcff05d19e495f1a44f190885b25cbb6f217c4a) 修复了PQ标签中不等号的添加问题
* [Commit 9ebc](https://github.com/manticoresoftware/manticoresearch/commit/9ebc58916cd515eaa88da66d0895aebf0d1f2b5f) 修复了CALL PQ语句中添加文档ID字段的问题
* [Commit 8ae0](https://github.com/manticoresoftware/manticoresearch/commit/8ae0e593a286110d8505d88baa3ac9e433cb38c9) 修复了PQ索引的刷新语句处理
* [Commit c24b](https://github.com/manticoresoftware/manticoresearch/commit/c24b152344ecd77661566cddb803487f51d3c1aa) 修复了PQ对JSON和字符串属性的过滤问题
* [Commit 1b8b](https://github.com/manticoresoftware/manticoresearch/commit/1b8bdde19aaa362785ea4c33bdc019154f8fbac4) 修复了空JSON字符串的解析问题
* [Commit 1ad8](https://github.com/manticoresoftware/manticoresearch/commit/1ad8a07dbfc1131913a8d92c261fbb48f934e8b7) 修复了多查询中使用OR过滤器导致崩溃的问题
* [Commit 69b8](https://github.com/manticoresoftware/manticoresearch/commit/69b89806380a229e36287ad9daf6f0ea2b5453eb) 修复了 indextool 使用配置公共部分（lemmatizer_base 选项）针对命令（dumpheader）
* [Commit 6dbe](https://github.com/manticoresoftware/manticoresearch/commit/6dbeaf2389d2af6a9dfccb56bfc986685b9f203e) 修复了结果集和过滤器中的空字符串
* [Commit 39c4](https://github.com/manticoresoftware/manticoresearch/commit/39c4eb55ecc85f6dd54f6c8c6d6dfcf87fd8748e) 修复了负文档 ID 值
* [Commit 266b](https://github.com/manticoresoftware/manticoresearch/commit/266b7054f98e7d30fc2829b5c2467dc9f90def92) 修复了对非常长索引词的词语截断长度
* [Commit 4782](https://github.com/manticoresoftware/manticoresearch/commit/47823b0bd06cbd016a24345c99bb8693790b43dd) 修复了 PQ 上通配符查询匹配多个文档的问题


## 版本 2.6.4 GA，2018年5月3日
### 新特性和改进
* MySQL FEDERATED 引擎[支持](Extensions/FEDERATED.md)
* MySQL 数据包现在返回 SERVER_STATUS_AUTOCOMMIT 标志，增强了与 ProxySQL 的兼容性
* [listen_tfo](Server_settings/Searchd.md#listen_tfo) - 为所有监听器启用 TCP 快速打开连接
* indexer --dumpheader 也可以从 .meta 文件中导出 RT 头信息
* Ubuntu Bionic 的 cmake 构建脚本

### Bug 修复
* [Commit 355b](https://github.com/manticoresoftware/manticoresearch/commit/355b11629174813abd9cd5bf2233be0783f77745) 修复了 RT 索引的无效查询缓存条目；
* [Commit 546e](https://github.com/manticoresoftware/manticoresearch/commit/546e2297a01cc2913bc0d33052d5bcefae8737eb) 修复了无缝轮换后索引设置丢失的问题
* [Commit 0c45](https://github.com/manticoresoftware/manticoresearch/commit/0c4509898393993bc87194d2aca2070395ff7f83) 修正了固定中缀与前缀长度设置；增加了对不支持中缀长度的警告
* [Commit 8054](https://github.com/manticoresoftware/manticoresearch/commit/80542fa54d727c781635918e8d4d65a20c2fbd1e) 修正了 RT 索引的自动刷新顺序
* [Commit 705d](https://github.com/manticoresoftware/manticoresearch/commit/705d8c5f407726ed1c6f9745ecffa375682c8969) 修正了针对多个属性索引和对多个索引查询时结果集模式的问题
* [Commit b0ba](https://github.com/manticoresoftware/manticoresearch/commit/b0ba932fff4a082f642cb737381c8a70215d4cc9) 修复了批量插入时部分重复文档缺失的问题
* [Commit 4510](https://github.com/manticoresoftware/manticoresearch/commit/4510fa44ad54b5bbac53c591144501565130fcae) 修复了优化失败无法合并包含大量文档的 RT 索引磁盘块的问题

## 版本 2.6.3 GA，2018年3月28日
### 改进
* 编译时支持 jemalloc。如果系统中存在 jemalloc，可以通过 cmake 参数 `-DUSE_JEMALLOC=1` 启用

### Bug 修复
* [Commit 85a6](https://github.com/manticoresoftware/manticoresearch/commit/85a6d7e3db75f28c491042409bb677ed39eba07b) 修复了将 expand_keywords 选项记录到 Manticore SQL 查询日志的问题
* [Commit caaa](https://github.com/manticoresoftware/manticoresearch/commit/caaa3842f4dbd8507fd9e20bf3371a0c8ec0c087) 修正了 HTTP 接口处理大尺寸查询的问题
* [Commit e386](https://github.com/manticoresoftware/manticoresearch/commit/e386d842b36195719e459b2db1fb20e402f9a51f) 修复了带 index_field_lengths 启用时对 RT 索引执行 DELETE 导致服务器崩溃的问题
* [Commit cd53](https://github.com/manticoresoftware/manticoresearch/commit/ce528f37eb5d8284661928f6f3b340a4a1985253) 修复了不支持的系统下 cpustats searchd 命令行选项的问题
* [Commit 8740](https://github.com/manticoresoftware/manticoresearch/commit/8740fd63ae842c2ab0e51a8fcd6180d787ed57ec) 修复了定义了最小长度时 utf8 子串匹配的问题


## 版本 2.6.2 GA，2018年2月23日
### 改进
* 在使用 NOT 运算符及批量文档情况下，提升了[感知查询](Searching/Percolate_query.md)性能。
* [percolate_query_call](Searching/Percolate_query.md) 能根据 [dist_threads](Server_settings/Searchd.md#threads) 使用多线程
* 新增全文匹配操作符 NOTNEAR/N
* 对感知索引支持 SELECT 的 LIMIT
* [expand_keywords](Searching/Options.md#expand_keywords) 可接受 'start','exact' （其中 'start,exact' 与 '1' 效果相同）
* 支持基于 sql_query_range 定义范围查询的[连接字段](Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Processing_fetched_data.md#sql_joined_field)主查询功能

### Bug 修复
* [Commit 72dc](https://github.com/manticoresoftware/manticoresearch/commit/72dcf669744e9b7d636dfc213d24df85ab301f6b) 修复了内存段搜索崩溃；双缓冲保存磁盘块死锁；优化过程中保存磁盘块死锁
* [Commit 3613](https://github.com/manticoresoftware/manticoresearch/commit/36137149a1c3c0893bdda5a28fc7e8244bf2d4ae) 修复了索引器因空属性名而在 XML 嵌入模式下崩溃
* [Commit 48d7](https://github.com/manticoresoftware/manticoresearch/commit/48d7e8001d2a66466ca64577f27ddc5421a67251) 修复了错误地删除非所有权 pid 文件的问题
* [Commit a556](https://github.com/manticoresoftware/manticoresearch/commit/a5563a465ddc59ef71e65f17b68bc33f9700e838) 修复了临时目录中遗留孤儿 FIFO 文件的问题
* [Commit 2376](https://github.com/manticoresoftware/manticoresearch/commit/2376e8fc4508944b96959bd10686c6d51f5145e8) 修复了 FACET 返回空结果集却包含错误 NULL 行的问题
* [Commit 4842](https://github.com/manticoresoftware/manticoresearch/commit/4842b67acdd5da75672db6c08ac563b48577ddd7) 修复了在 Windows 服务模式运行时索引锁损坏的问题
* [Commit be35](https://github.com/manticoresoftware/manticoresearch/commit/be35feef54ded59125750916cd211e56108eddae) 修正了 macOS 上错误的 iconv 库
* [Commit 8374](https://github.com/manticoresoftware/manticoresearch/commit/83744a977d7837d1d87cd506b88b0e7eb199efe6) 修复了错误的 count(\*)


## 版本 2.6.1 GA，2018年1月26日
### 改进
* [agent_retry_count](Server_settings/Searchd.md#agent_retry_count) 对于带有镜像的代理，返回每个镜像的重试次数，而不是每个代理的重试次数，总重试次数为 agent_retry_count\*mirrors。
* [agent_retry_count](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_retry_count) 现在可以针对每个索引指定，覆盖全局值。添加了别名 [mirror_retry_count](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#mirror_retry_count)。
* 可以在代理定义中指定重试次数 retry_count，该值表示每个代理的重试次数。
* Percolate 查询现在支持 HTTP JSON API 的 [/json/pq](Searching/Percolate_query.md#Performing-a-percolate-query-with-CALL-PQ)。
* 为可执行文件增加了 -h 和 -v 选项（帮助和版本）。
* 支持 Real-Time 索引的 [morphology_skip_fields](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology)。

### Bugfixes
* [Commit a40b](https://github.com/manticoresoftware/manticore/commit/a40b0793feff65e40d10062568d9847c08d10f57) 修复了 ranged-main-query 与 sql_range_step 在 MVA 字段使用时的兼容性。
* [Commit f2f5](https://github.com/manticoresoftware/manticore/commit/f2f53757db45bcfb1544263ce0817e856656a621) 修复了 blackhole 系统循环挂起及 blackhole 代理看似断开连接的问题。
* [Commit 84e1](https://github.com/manticoresoftware/manticore/commit/84e1f54aef25e0fce98870ad2dd784db5116f1d6) 修正了查询 ID 的一致性问题，修复了存储查询的重复 ID。
* [Commit 1948](https://github.com/manticoresoftware/manticore/commit/19484231814fcb82b21763a3a4a9f45adc6b2d40) 修复了服务器从各种状态关闭时的崩溃。
* [Commit 9a70](https://github.com/manticoresoftware/manticore/commit/9a706b499a1d61a90076065a1a703029d49db958)  [Commit 3495](https://github.com/manticoresoftware/manticore/commit/3495fd70cba8846b1a50d55d6679b039414c3d2a) 修正了长查询的超时问题。
* [Commit 3359](https://github.com/manticoresoftware/manticore/commit/3359bcd89b4f79a645fe84b8cf8616ce0addff02) 重构了基于 kqueue 系统（Mac OS X，BSD）的 master-agent 网络轮询。


## 版本 2.6.0，2017年12月29日
### 功能和改进
* [HTTP JSON](Connecting_to_the_server/HTTP.md)：JSON 查询现支持属性的等值匹配，MVA 和 JSON 属性可用于插入和更新，通过 JSON API 也可对分布式索引执行更新和删除操作。
* [Percolate 查询](Searching/Percolate_query.md)
* 代码中移除对 32 位 docids 的支持，同时删除所有用于转换/加载拥有 32 位 docids 的遗留索引的代码。
* [仅对特定字段应用形态学](https://github.com/manticoresoftware/manticore/issues/7)。新增索引指令 morphology_skip_fields，允许定义不应用形态学的字段列表。
* [expand_keywords 现在可作为查询运行时指令通过 OPTION 语句设置](https://github.com/manticoresoftware/manticore/issues/8)

### Bugfixes
* [Commit 0cfa](https://github.com/manticoresoftware/manticore/commit/0cfae4c) 修复了使用 rlp 构建的服务器调试版本（以及可能的发布版未定义行为）崩溃问题。
* [Commit 3242](https://github.com/manticoresoftware/manticore/commit/324291e) 修正了启用 progressive 选项时 RT 索引优化中合并 kill-lists 的顺序错误。
* [Commit ac0e](https://github.com/manticoresoftware/manticore/commit/ac0efee) mac 上的轻微崩溃修复。
* 经过详尽静态代码分析后进行的多处细微修复。
* 其他小的 bug 修复。

### 升级说明
本版本更改了主节点与代理之间通信使用的内部协议。如果在分布式环境下运行 Manticoresearch 多实例，请确保先升级代理，然后升级主节点。

## 版本 2.5.1，2017年11月23日
### 功能和改进
* 基于 [HTTP API 协议](Connecting_to_the_server/HTTP.md) 的 JSON 查询。支持搜索、插入、更新、删除、替换操作。数据操作命令也支持批处理，目前存在一些限制，如 MVA 和 JSON 属性不能用于插入、替换或更新。
* [RELOAD INDEXES](Data_creation_and_modification/Adding_data_from_external_storages/Rotating_a_table.md#RELOAD-TABLES) 命令。
* [FLUSH LOGS](Logging/Rotating_query_and_server_logs.md) 命令。
* [SHOW THREADS](Node_info_and_management/SHOW_THREADS.md) 能显示优化、轮换或刷新进度。
* GROUP N BY 正确支持 MVA 属性。
* blackhole 代理在独立线程运行，不再影响主节点查询。
* 实现了索引的引用计数，避免因轮换和高负载导致的停滞。
* 实现了 SHA1 哈希算法，尚未对外开放。
* 适配 FreeBSD、macOS 和 Alpine 的编译修正。

### Bugfixes
* [Commit 9897](https://github.com/manticoresoftware/manticore/commit/989752b) 修复了基于块索引的过滤器回归问题。
* [Commit b1c3](https://github.com/manticoresoftware/manticore/commit/b1c3864) 将 PAGE_SIZE 重命名为 ARENA_PAGE_SIZE 以兼容 musl。
* [Commit f213](https://github.com/manticoresoftware/manticore/commit/f2133cc) 在 cmake < 3.1.0 时禁用 googletests。
* [Commit f30e](https://github.com/manticoresoftware/manticore/commit/0839de7) 修复服务器重启时绑定套接字失败。
* [Commit 0807](https://github.com/manticoresoftware/manticore/commit/0807240) 修复服务器关闭时崩溃的问题。
* [Commit 3e3a](https://github.com/manticoresoftware/manticore/commit/3e3acc3) 修复黑洞线程的 show threads 显示。
* [Commit 262c](https://github.com/manticoresoftware/manticore/commit/262c3fe) 重构 iconv 配置检查，修复 FreeBSD 和 Darwin 平台构建问题。

## 版本 2.4.1 GA，2017年10月16日
### 功能和改进
* WHERE 子句中在属性过滤器之间支持 OR 操作符。
* 维护模式（ SET MAINTENANCE=1）
* [CALL KEYWORDS](Searching/Autocomplete.md#CALL-KEYWORDS) 支持分布式索引。
* [UTC 分组](Server_settings/Searchd.md#grouping_in_utc)
* [query_log_mode](Server_settings/Searchd.md#query_log_mode) 支持自定义日志文件权限。
* 字段权重支持为零或负值。
* [max_query_time](Searching/Options.md#max_query_time) 现在可以影响全扫描
* 添加了 [net_wait_tm](Server_settings/Searchd.md#net_wait_tm)、[net_throttle_accept](Server_settings/Searchd.md#net_throttle_accept) 和 [net_throttle_action](Server_settings/Searchd.md#net_throttle_action) 用于网络线程的微调（在 workers=thread_pool 情况下）
* COUNT DISTINCT 适用于 facet 搜索
* IN 可以用于 JSON 浮点数组
* 多查询优化不再被整数/浮点表达式破坏
* 使用多查询优化时，[SHOW META](Node_info_and_management/SHOW_META.md) 显示一个 `multiplier` 行

### 编译
Manticore Search 使用 cmake 构建，编译所需的最低 gcc 版本是 4.7.2。

### 目录和服务
* Manticore Search 以 `manticore` 用户身份运行。
* 默认数据文件夹现在是 `/var/lib/manticore/`。
* 默认日志文件夹现在是 `/var/log/manticore/`。
* 默认 pid 文件夹现在是 `/var/run/manticore/`。

### 修复的错误
* [Commit a58c](https://github.com/manticoresoftware/manticore/commit/a58c619) 修复了破坏 java 连接器的 SHOW COLLATION 语句
* [Commit 631c](https://github.com/manticoresoftware/manticore/commit/631cf4e) 修复了处理分布式索引时的崩溃；为分布式索引哈希添加锁；从 agent 中移除移动和复制操作符
* [Commit 942b](https://github.com/manticoresoftware/manticore/commit/942bec0) 修复了因并行重连导致处理分布式索引时的崩溃
* [Commit e5c1](https://github.com/manticoresoftware/manticore/commit/e5c1ed2) 修复了服务器日志存储查询时崩溃处理器的崩溃
* [Commit 4a4b](https://github.com/manticoresoftware/manticore/commit/4a4bda5) 修复了多查询中池化属性的崩溃
* [Commit 3873](https://github.com/manticoresoftware/manticore/commit/3873bfb) 通过防止索引页面被包含进核心文件修复了核心文件大小减小的问题
* [Commit 11e6](https://github.com/manticoresoftware/manticore/commit/11e6254) 修复了无效 agent 指定时搜索守护进程启动崩溃
* [Commit 4ca6](https://github.com/manticoresoftware/manticore/commit/4ca6350) 修复了 indexer 在 sql_query_killlist 查询中报错
* [Commit 123a](https://github.com/manticoresoftware/manticore/commit/123a9f0) 修复了 fold_lemmas=1 与命中计数的问题
* [Commit cb99](https://github.com/manticoresoftware/manticore/commit/cb99164) 修复了 html_strip 行为不一致的问题
* [Commit e406](https://github.com/manticoresoftware/manticore/commit/e406761) 修复了优化实时索引时丢失新设置的问题；修复了带同步选项锁泄漏的问题；
* [Commit 86ae](https://github.com/manticoresoftware/manticore/commit/86aeb82) 修复了错误多查询的处理
* [Commit 2645](https://github.com/manticoresoftware/manticore/commit/2645230) 修复了结果集依赖多查询顺序的问题
* [Commit 7239](https://github.com/manticoresoftware/manticore/commit/72395d9) 修复了多查询中错误查询导致的服务器崩溃
* [Commit f353](https://github.com/manticoresoftware/manticore/commit/f353326) 修复了共享锁转独占锁的问题
* [Commit 3754](https://github.com/manticoresoftware/manticore/commit/3754785) 修复了查询无索引时的服务器崩溃
* [Commit 29f3](https://github.com/manticoresoftware/manticore/commit/29f360e) 修复了服务器死锁

## 版本 2.3.3，2017年7月6日
* Manticore 品牌化

