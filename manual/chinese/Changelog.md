# 更新日志

## 版本 13.6.7
**发布时间**：2025年8月8日

推荐的 [MCL](https://github.com/manticoresoftware/columnar) 版本：8.0.1  
推荐的 [Buddy](Installation/Manticore_Buddy.md#Manticore-Buddy) 版本：3.34.2  

如果您遵循 [官方安装指南](https://manticoresearch.com/install/)，则无需担心此事。

### 新功能和改进
* 🆕 [v13.6.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.0) [ Issue #2226](https://github.com/manticoresoftware/manticoresearch/issues/2226) 支持 PHRASE、PROXIMITY 和 QUORUM 操作符中显式使用 '|'（OR）。
* 🆕 [v13.5.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.5.0) [ PR #3591](https://github.com/manticoresoftware/manticoresearch/pull/3591) 查询中自动生成嵌入（进行中，尚未准备好用于生产环境）。
* 🆕 [v13.4.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.4.0) [ PR #3585](https://github.com/manticoresoftware/manticoresearch/pull/3585) 修正逻辑以优先使用 buddy_path 配置中的 buddy 线程数（如果设置），而不是使用守护进程的值。
* 🆕 [v13.3.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.3.0) [ PR #3577](https://github.com/manticoresoftware/manticoresearch/pull/3577) 支持与本地分布式表的连接（joins）。
* 🆕 [#3647](https://github.com/manticoresoftware/manticoresearch/issues/3647) 添加对 Debian 13 "Trixie" 的支持

### Bug 修复
* 🪲 [v13.6.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.7) [ Issue #3524](https://github.com/manticoresoftware/manticoresearch/issues/3524) 修复了在行存储中保存生成的嵌入时的问题。
* 🪲 [v13.6.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.6) [ Issue #3563](https://github.com/manticoresoftware/manticoresearch/issues/3563) 修复了 Sequel Ace 及其他集成因“未知 sysvar”错误导致失败的问题。
* 🪲 [v13.6.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.5) [ Issue #3467](https://github.com/manticoresoftware/manticoresearch/issues/3467) 修复了 DBeaver 及其他集成因“未知 sysvar”错误导致失败的问题。
* 🪲 [v13.6.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.4) [ Issue #3524](https://github.com/manticoresoftware/manticoresearch/issues/3524) 修复了多字段嵌入拼接的问题；同时修复了查询中嵌入生成的问题。
* 🪲 [v13.6.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.3) [ Issue #3641](https://github.com/manticoresoftware/manticoresearch/issues/3641) 修正了 13.6.0 版本中短语丢失除第一个外所有带括号关键词的错误。
* 🪲 [v13.6.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.2) 修复了 transform_phrase 中的内存泄漏。
* 🪲 [v13.6.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.1) 修复了 13.6.0 版本中的内存泄漏。
* 🪲 [v13.5.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.5.2) [ Issue #3651](https://github.com/manticoresoftware/manticoresearch/issues/3651) 修复了更多与全文搜索模糊测试相关的问题。
* 🪲 [v13.5.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.5.1) [ Issue #3560](https://github.com/manticoresoftware/manticoresearch/issues/3560) 修复了带有 KNN 数据时 OPTIMIZE TABLE 可能无限挂起的情况。
* 🪲 [v13.4.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.4.2) [ Issue #2544](https://github.com/manticoresoftware/manticoresearch/issues/2544) 修复了添加 float_vector 列时可能损坏索引的问题。
* 🪲 [v13.4.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.4.1) [ Issue #3651](https://github.com/manticoresoftware/manticoresearch/issues/3651) 为全文解析添加模糊测试并修复测试中发现的几个问题。
* 🪲 [v13.3.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.3.1) [ Issue #3583](https://github.com/manticoresoftware/manticoresearch/issues/3583) 修复了使用复杂布尔过滤器且启用高亮时的崩溃问题。
* 🪲 [v13.2.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.7) [ Issue #3481](https://github.com/manticoresoftware/manticoresearch/issues/3481) 修复了在同时使用 HTTP 更新、分布式表及错误的复制集群时的崩溃。
* 🪲 [v13.2.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.6) [ PR #3567](https://github.com/manticoresoftware/manticoresearch/pull/3567) 更新 manticore-backup 依赖版本至 1.9.6。
* 🪲 [v13.2.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.5) [ PR #3565](https://github.com/manticoresoftware/manticoresearch/pull/3565) 修正 CI 设置以提升 Docker 镜像兼容性。
* 🪲 [v13.2.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.4) 修正长词 token 的处理。一些特殊 token（如正则表达式模式）可能产生过长的词，因此现在使用前会进行缩短。

## 版本 13.2.3
**发布时间**：2025年7月8日

### 破坏性变更
* ⚠️ [v13.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.0.0) 更新了 KNN 库的 API，以支持空的 [float_vector](https://manual.manticoresearch.com/Creating_a_table/Data_types#Float-vector) 值。此更新未更改数据格式，但提高了 Manticore Search / MCL API 版本。
* ⚠️ [v12.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/12.0.0) [ PR #3516](https://github.com/manticoresoftware/manticoresearch/pull/3516) 修复了 KNN 索引训练和构建过程中源和目的行 ID 错误的 BUG。此更新未更改数据格式，但提高了 Manticore Search / MCL API 版本。
* ⚠️ [v11.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.0.0)  新增对量化、重评分和过采样等新的KNN向量搜索功能的支持。此版本更改了KNN数据格式和[KNN_DIST() SQL语法](https://manual.manticoresearch.com/Searching/KNN?client=SQL#KNN-vector-search)。新版本可以读取旧数据，但旧版本无法读取新格式。

### 新功能和改进
* 🆕 [v13.2.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.0) [ PR #3549](https://github.com/manticoresoftware/manticoresearch/issues/3525) 修复了`@@collation_database`导致与某些mysqldump版本不兼容的问题
* 🆕 [v13.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.1.0) [ Issue #3489](https://github.com/manticoresoftware/manticoresearch/issues/3489) 修复了模糊搜索中导致某些SQL查询无法解析的错误
* 🆕 [v12.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/12.1.0) [ Issue #3426](https://github.com/manticoresoftware/manticoresearch/issues/3426) 添加了对RHEL 10操作系统的支持
* 🆕 [v11.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.1.0)  添加了对空浮点向量在[KNN搜索](https://manual.manticoresearch.com/Searching/KNN#KNN-vector-search)中的支持
* 🆕 [v10.2.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.0) [ Issue #3252](https://github.com/manticoresoftware/manticoresearch/issues/3252) [log_level](https://manual.manticoresearch.com/Server_settings/Setting_variables_online#SET) 现在也调整Buddy的日志详细程度

### Bug修复
* 🪲 [v13.2.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.3) [ PR #3556](https://github.com/manticoresoftware/manticoresearch/pull/3556) 修复了JSON查询中“oversampling”选项的解析问题
* 🪲 [v13.2.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.2)  通过移除Boost stacktrace的使用，修复了Linux和FreeBSD上的崩溃日志记录问题
* 🪲 [v13.2.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.1) [ Issue #3298](https://github.com/manticoresoftware/manticoresearch/issues/3298) 修复了在容器内运行时的崩溃日志记录问题
* 🪲 [v12.0.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/12.0.2)  通过以微秒为单位计数，提高了每表统计数据的准确性
* 🪲 [v12.0.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/12.0.1) [ PR #3522](https://github.com/manticoresoftware/manticoresearch/pull/3522) 修复了在联接查询中按MVA分面时的崩溃
* 🪲 [v11.0.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.0.3) [ Issue #3502](https://github.com/manticoresoftware/manticoresearch/issues/3502) 修复了与向量搜索量化相关的崩溃
* 🪲 [v11.0.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.0.2) [ Issue #3493](https://github.com/manticoresoftware/manticoresearch/issues/3493) 修改了`SHOW THREADS`，以整数显示CPU利用率
* 🪲 [v11.0.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.0.1)  修正了列存和二级库的路径
* 🪲 [v10.2.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.7)  添加了对MCL 5.0.5的支持，包括修复embedding库文件名
* 🪲 [v10.2.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.6) [ Issue #3469](https://github.com/manticoresoftware/manticoresearch/issues/3469) 针对issue #3469应用了另一处修复
* 🪲 [v10.2.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.4) [ Issue #3469](https://github.com/manticoresoftware/manticoresearch/issues/3469) 修复了带布尔查询的HTTP请求返回空结果的问题
* 🪲 [v10.2.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.5)  更改了embedding库的默认文件名并新增对KNN向量中'from'字段的检查
* 🪲 [v10.2.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.3) [ PR #3464](https://github.com/manticoresoftware/manticoresearch/pull/3464) 更新Buddy到版本3.30.2，其中包括关于内存使用和错误日志的[PR #565](https://github.com/manticoresoftware/manticoresearch-buddy/pull/565)
* 🪲 [v10.2.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.2) [ Issue #3470](https://github.com/manticoresoftware/manticoresearch/pull/3470) 修复了JOIN查询中的JSON字符串过滤器、空过滤器和排序问题
* 🪲 [v10.2.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.1)  修复了`dist/test_kit_docker_build.sh`中导致Buddy提交丢失的错误
* 🪲 [v10.1.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.4)  修复了在联接查询中按MVA分组时导致的崩溃
* 🪲 [v10.1.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.3) [ Issue #3434](https://github.com/manticoresoftware/manticoresearch/issues/3434) 修复了过滤空字符串时的错误
* 🪲 [v10.1.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.2) [ PR #3452](https://github.com/manticoresoftware/manticoresearch/pull/3452) 更新Buddy到版本3.29.7，解决了[Buddy #507 - 带模糊搜索的多查询请求错误](https://github.com/manticoresoftware/manticoresearch-buddy/issues/507)和[Buddy #561 - 指标速率修复](https://github.com/manticoresoftware/manticoresearch-buddy/pull/561)，这是[Helm Release 10.1.0](https://github.com/manticoresoftware/manticoresearch-helm/releases/tag/manticoresearch-10.1.0)所必需的
* 🪲 [v10.1.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.1)  将 Buddy 更新到版本 3.29.4，解决了 [#3388 - "Undefined array key 'Field'"](https://github.com/manticoresoftware/manticoresearch/issues/3388) 和 [Buddy #547 - layouts='ru' 可能无法工作](https://github.com/manticoresoftware/manticoresearch-buddy/issues/547)

## 版本 10.1.0
**发布日期**：2025年6月9日

该版本代表一个包含新功能的更新，包含一个破坏性变更以及众多稳定性改进和错误修复。变化主要集中在增强监控能力、改进搜索功能以及修复影响系统稳定性和性能的各种关键问题。

**从版本 10.1.0 开始，不再支持 CentOS 7。建议用户升级到支持的操作系统。**

### 破坏性变更
* ⚠️ [v10.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.0.0) [ 问题 #540](https://github.com/manticoresoftware/manticoresearch-buddy/issues/540) 破坏性变更：将 [模糊搜索](Searching/Spell_correction.md#Fuzzy-Search) 的默认 `layouts=''`

### 新功能和改进
* 🆕 [v10.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.0) [ 问题 #537](https://github.com/manticoresoftware/manticoresearch-buddy/issues/537) 添加内置的 [Prometheus 导出器](Node_info_and_management/Node_status.md#Prometheus-Exporter)
* 🆕 [v9.8.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.8.0) [ 问题 #3409](https://github.com/manticoresoftware/manticoresearch/issues/3409) 添加 [ALTER TABLE tbl REBUILD KNN](Updating_table_schema_and_settings.md#Rebuilding-a-KNN-index)
* 🆕 [v9.7.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.7.0) [ 问题 #1778](https://github.com/manticoresoftware/manticoresearch/issues/1778) 添加自动生成嵌入 (我们暂未正式发布，因为代码已在主分支但还需要更多测试)
* 🆕 [v9.6.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.6.0)  提升 KNN API 版本以支持自动生成嵌入
* 🆕 [v9.5.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.0) [ 问题 #1894](https://github.com/manticoresoftware/manticoresearch/issues/1894) 改进集群恢复：定期保存 `seqno` 以加快崩溃后节点重启速度
* 🆕 [v9.4.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.4.0) [ 问题 #2400](https://github.com/manticoresoftware/manticoresearch/issues/2400) 添加对最新 [Logstash](Integration/Logstash.md#Integration-with-Logstash) 和 Beats 的支持

### 错误修复
* 🪲 [v10.0.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.0.1)  修正词形处理：用户定义的词形现在覆盖自动生成的词形；为测试22添加测试用例
* 🪲 [v9.8.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.8.2)  修复：再次更新 deps.txt，包含 MCL 中与嵌入库相关的打包修复
* 🪲 [v9.8.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.8.1)  修复：更新 deps.txt，修复 MCL 和嵌入库的打包问题
* 🪲 [v9.7.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.7.3) [ 问题 #3306](https://github.com/manticoresoftware/manticoresearch/issues/3306) 修复索引时的信号 11 崩溃
* 🪲 [v9.7.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.7.2) [ 问题 #3109](https://github.com/manticoresoftware/manticoresearch/issues/3109) 修复不存在的 `@@variables` 总是返回 0 的问题
* 🪲 [v9.7.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.7.1) [ 问题 #3377](https://github.com/manticoresoftware/manticoresearch/issues/3377) 修复与 [remove_repeats()](Functions/Searching_and_ranking_functions.md#REMOVE_REPEATS%28%29) 相关的崩溃
* 🪲 [v9.6.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.6.3) [ PR #3411](https://github.com/manticoresoftware/manticoresearch/pull/3411) 修复：使用动态版本检测进行遥测指标
* 🪲 [v9.6.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.6.2)  修复：[SHOW VERSION](Node_info_and_management/SHOW_VERSION.md#SHOW-VERSION) 输出中的小错误
* 🪲 [v9.6.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.6.1)  修复：创建带有 KNN 属性但无模型的表时崩溃
* 🪲 [v9.5.16](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.16) [ 问题 #3342](https://github.com/manticoresoftware/manticoresearch/issues/3342) 修复 `SELECT ... FUZZY=0` 不总是禁用模糊搜索的问题
* 🪲 [v9.5.15](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.15) [ PR #3397](https://github.com/manticoresoftware/manticoresearch/pull/3397) 添加对 [MCL](https://github.com/manticoresoftware/columnar) 4.2.2 的支持；修复旧存储格式的错误
* 🪲 [v9.5.14](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.14) [ 问题 #3392](https://github.com/manticoresoftware/manticoresearch/issues/3392) 修复 HTTP JSON 回复中字符串处理错误
* 🪲 [v9.5.13](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.13) [ 问题 #3356](https://github.com/manticoresoftware/manticoresearch/issues/3356) 修复复杂全文查询（公共子词）中的崩溃
* 🪲 [v9.5.12](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.12)  修正磁盘块自动刷新错误消息中的拼写错误
* 🪲 [v9.5.11](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.11) [ 问题 #3195](https://github.com/manticoresoftware/manticoresearch/issues/3195) 改进 [自动磁盘块刷新](Securing_and_compacting_a_table/Flushing_RAM_chunk_to_disk.md#Flushing-RAM-chunk-to-disk)：优化运行时跳过保存
* 🪲 [v9.5.10](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.10) [ Issue #3313](https://github.com/manticoresoftware/manticoresearch/issues/3313) 修复了使用 [indextool](Miscellaneous_tools.md#indextool) 对 RT 表中所有磁盘块进行重复 ID 检查的问题
* 🪲 [v9.5.9](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.9) [ Issue #3132](https://github.com/manticoresoftware/manticoresearch/issues/3132) 增加了对 JSON API 中 `_random` 排序的支持
* 🪲 [v9.5.8](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.8) [ Issue #3382](https://github.com/manticoresoftware/manticoresearch/issues/3382) 修复了无法通过 JSON HTTP API 使用 uint64 文档 ID 的问题
* 🪲 [v9.5.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.7) [ Issue #3385](https://github.com/manticoresoftware/manticoresearch/issues/3385) 修正了按 `id != value` 过滤时返回错误结果的问题
* 🪲 [v9.5.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.6) [ PR #538](https://github.com/manticoresoftware/manticoresearch-buddy/pull/538) 修复了一些情况下模糊匹配的关键错误
* 🪲 [v9.5.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.5) [ Issue #3199](https://github.com/manticoresoftware/manticoresearch/issues/3199) 修正了 Buddy HTTP 查询参数中的空格解码问题（例如 `%20` 和 `+`）
* 🪲 [v9.5.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.4) [ Issue #3133](https://github.com/manticoresoftware/manticoresearch/issues/3133) 修复了 facet 搜索中 `json.field` 排序错误的问题
* 🪲 [v9.5.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.3) [ Issue #3091](https://github.com/manticoresoftware/manticoresearch/issues/3091) 修复了 SQL 与 JSON API 对分隔符搜索结果不一致的问题
* 🪲 [v9.5.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.2) [ Issue #2819](https://github.com/manticoresoftware/manticoresearch/issues/2819) 性能提升：将分布式表的 `DELETE FROM` 替换为 `TRUNCATE`
* 🪲 [v9.5.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.1) [ Issue #3080](https://github.com/manticoresoftware/manticoresearch/issues/3080) 修复了使用 JSON 属性过滤别名 `geodist()` 时导致的崩溃

## 版本 9.3.2
发布：2025年5月2日

此版本包含多个错误修复和稳定性改进，更好的表使用情况跟踪，以及内存和资源管理的增强。

❤️ 特别感谢 [@cho-m](https://github.com/cho-m) 修复了与 Boost 1.88.0 的构建兼容性，以及 [@benwills](https://github.com/benwills) 改进了 `stored_only_fields` 的文档说明。

* 🪲 [v9.3.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.3.2) 修复了“显示线程”列将 CPU 活动显示为浮点数而非字符串的问题；同时修正了 PyMySQL 结果集因错误数据类型导致的解析错误。
* 🪲 [v9.3.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.3.1) [ Issue #3343](https://github.com/manticoresoftware/manticoresearch/issues/3343) 修复了优化过程被中断时遗留的 `tmp.spidx` 文件。
* 🆕 [v9.3.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.3.0) [ PR #3337](https://github.com/manticoresoftware/manticoresearch/pull/3337) 添加了每个表的命令计数器和详细的表使用统计。
* 🪲 [v9.2.39](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.39) [ Issue #3236](https://github.com/manticoresoftware/manticoresearch/issues/3236) 修复：通过移除复杂的块更新防止表损坏。在串行工作线程中使用等待函数破坏了串行处理，可能导致表损坏。
重新实现自动刷新。移除外部轮询队列以避免不必要的表锁。添加了“小表”条件：如果文档数量低于“小表限制”（8192）且未使用二级索引（SI），则跳过刷新。

* 🪲 [v9.2.38](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.38) 修复：跳过为使用 `ALL`/`ANY` 过滤字符串列表的过滤器创建二级索引（SI），不影响 JSON 属性。
* 🪲 [v9.2.37](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.37) [ Issue #2898](https://github.com/manticoresoftware/manticoresearch/issues/2898) 为系统表添加了反引号支持。
* 🪲 [v9.2.36](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.36) 修复：在旧代码中为集群操作使用占位符。解析器中我们现在清晰分离了表名和集群名的字段。
* 🪲 [v9.2.35](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.35) 修复：单个 `'` 去引用时导致的崩溃。
* 🪲 [v9.2.34](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.34) [ Issue #3090](https://github.com/manticoresoftware/manticoresearch/issues/3090) 修复：处理大文档 ID 时（之前可能找不到它们）。
* 🪲 [v9.2.33](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.33) 修复：位向量大小使用无符号整数。
* 🪲 [v9.2.32](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.32) 修复：减少合并过程中的峰值内存使用。docid-to-rowid 查找现在对每个文档使用 12 字节，而非 16 字节。例如：20亿文档用 RAM 从 36 GB 降至 24 GB。
* 🪲 [v9.2.31](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.31) [ Issue #3238](https://github.com/manticoresoftware/manticoresearch/issues/3238) 修复：大型实时表中不正确的 `COUNT(*)` 值。
* 🪲 [v9.2.30](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.30) 修复：清零字符串属性时的未定义行为。
* 🪲 [v9.2.29](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.29) 小修正：改进了警告文本。
* 🪲 [v9.2.28](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.28) [ Issue #3290](https://github.com/manticoresoftware/manticoresearch/issues/3290) 改进：增强了 `indextool --buildidf`
* 🪲 [v9.2.27](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.27) [ Issue #3032](https://github.com/manticoresoftware/manticoresearch/issues/3032) 通过Kafka集成，您现在可以为特定的Kafka分区创建一个源。
* 🪲 [v9.2.26](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.26) [ Issue #3301](https://github.com/manticoresoftware/manticoresearch/issues/3301) 修复：针对 `id` 的 `ORDER BY` 和 `WHERE` 可能导致OOM（内存溢出）错误。
* 🪲 [v9.2.25](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.25) [ Issue #3171](https://github.com/manticoresoftware/manticoresearch/issues/3171) 修复：当RT表包含多个磁盘块且使用多个JSON属性的grouper时导致的段错误崩溃。
* 🪲 [v9.2.24](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.24) [ Issue #3246](https://github.com/manticoresoftware/manticoresearch/issues/3246) 修复：RAM块刷新后，`WHERE string ANY(...)` 查询失败。
* 🪲 [v9.2.23](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.23) [ PR #518](https://github.com/manticoresoftware/manticoresearch-buddy/pull/518) 次要的自动分片语法改进。
* 🪲 [v9.2.22](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.22) [ Issue #2763](https://github.com/manticoresoftware/manticoresearch/issues/2763) 修复：使用 `ALTER TABLE` 时未加载全局idf文件。
* 🪲 [v9.2.21](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.21)  修复：全局idf文件可能很大。我们现在更早释放表以避免占用不必要的资源。
* 🪲 [v9.2.20](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.20) [ PR #3277](https://github.com/manticoresoftware/manticoresearch/pull/3277) 改进：更好地验证分片选项。

* 🪲 [v9.2.19](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.19) [ PR #3275](https://github.com/manticoresoftware/manticoresearch/pull/3275) 修复：提升与Boost 1.88.0的构建兼容性。
* 🪲 [v9.2.18](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.18) [ Issue #3228](https://github.com/manticoresoftware/manticoresearch/issues/3228) 修复：分布式表创建时崩溃（无效指针问题）。

* 🪲 [v9.2.17](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.17) [ PR #3272](https://github.com/manticoresoftware/manticoresearch/pull/3272) 修复：多行模糊 `FACET` 问题。
* 🪲 [v9.2.16](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.16) [ Issue #3063](https://github.com/manticoresoftware/manticoresearch/issues/3063) 修复：使用 `GEODIST` 函数时距离计算中的错误。
* 🪲 [v9.2.15](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.15) [ Issue #3027](https://github.com/manticoresoftware/manticoresearch/issues/3027) 小型改进：支持Elastic的 `query_string` 过滤格式。

## 版本 9.2.14
发布：2025年3月28日

### 次要更改
* [Commit ](https://github.com/manticoresoftware/manticoresearch/commit/fe9473dc3f4fcfa0eaae748b538fe853f4ce078b) 实现了 `--mockstack` 标志以计算递归操作的堆栈需求。新的 `--mockstack` 模式分析并报告递归表达式求值、模式匹配操作、过滤器处理所需的堆栈大小。计算出的堆栈需求输出到控制台，用于调试和优化。
* [Issue #3058](https://github.com/manticoresoftware/manticoresearch/pull/3058) 默认启用了 [boolean_simplify](Searching/Options.md#boolean_simplify)。
* [Issue #3172](https://github.com/manticoresoftware/manticoresearch/issues/3172) 新增配置选项：`searchd.kibana_version_string`，在使用特定版本的Kibana或OpenSearch仪表盘，且它们期望特定Elasticsearch版本时此选项非常有用。
* [Issue #3211](https://github.com/manticoresoftware/manticoresearch/issues/3211) 修复了 [CALL SUGGEST](Searching/Spell_correction.md#CALL-QSUGGEST,-CALL-SUGGEST) 可以处理2字符词的问题。
* [Issue #490](https://github.com/manticoresoftware/manticoresearch-buddy/issues/490) 改进了[模糊搜索](Searching/Spell_correction.md#Fuzzy-Search)：之前，当存在其他匹配文档时，搜索 "def ghi" 有时无法找到 "defghi"。
* ⚠️ 重大变更 [Issue #3165](https://github.com/manticoresoftware/manticoresearch/issues/3165) 一些HTTP JSON响应中的 `_id` 改为 `id`，以保持一致性。请确保相应更新您的应用程序。
* ⚠️ 重大变更 [Issue #3186](https://github.com/manticoresoftware/manticoresearch/issues/3186) 集群加入时新增了 `server_id` 检查，确保每个节点具有唯一ID。当加入节点的 `server_id` 与集群中已有节点重复时，`JOIN CLUSTER` 操作现在会失败并显示错误信息。解决此问题的方法是确保复制集群中的每个节点具有唯一的 [server_id](Server_settings/Searchd.md#server_id)。您可以在配置文件的 "searchd" 部分更改默认的 [server_id](Server_settings/Searchd.md#server_id) 为唯一值，然后尝试加入集群。此更改更新了复制协议。如果您运行的是复制集群，则需要：
  - 首先，干净地停止所有节点
  - 然后，使用Linux工具 `manticore_new_cluster` 用 `--new-cluster` 启动最后停止的节点。
  - 详情请参阅[重启集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster)。

### Bug修复
* [Commit 6fda](https://github.com/manticoresoftware/manticoresearch/commit/6fdad3923dd6953f4b781943eed5ec5c28b7f808) 修复了等待后调度器丢失导致的崩溃；现在，像 `serializer` 这样的特定调度器可以正确恢复。
* [Commit c333](https://github.com/manticoresoftware/manticoresearch/commit/c333b277d4a504de5fe298e9ab570a9f9ea31e0a) 修复了右表连接中的权重无法在 `ORDER BY` 子句中使用的错误。
* [Issue #2644](https://github.com/manticoresoftware/manticoresearch/issues/2644) gcc 14.2.0：修复了使用 `const knn::DocDist_t*&` 调用 `lower_bound` 的错误。❤️ 感谢 [@Azq2](https://github.com/Azq2) 的 PR。
* [Issue #3018](https://github.com/manticoresoftware/manticoresearch/issues/3018) 修复了自动模式插入时处理大写表名的问题。
* [Issue #3119](https://github.com/manticoresoftware/manticoresearch/issues/3119) 修复了解码无效 base64 输入时的崩溃。
* [Issue #3121](https://github.com/manticoresoftware/manticoresearch/issues/3121) 修复了两个与 KNN 索引在 `ALTER` 操作相关的问题：浮点向量现在保持其原始维度，KNN 索引也能正确生成。
* [Issue #3123](https://github.com/manticoresoftware/manticoresearch/issues/3123) 修复了在空 JSON 列上构建二级索引时的崩溃。
* [Issue #3138](https://github.com/manticoresoftware/manticoresearch/issues/3138) 修复了由于重复条目导致的崩溃。
* [Issue #3151](https://github.com/manticoresoftware/manticoresearch/issues/3151) 修复了 `fuzzy=1` 选项无法与 `ranker` 或 `field_weights` 一起使用的问题。
* [Issue #3163](https://github.com/manticoresoftware/manticoresearch/issues/3163) 修复了 `SET GLOBAL timezone` 无效的错误。
* [Issue #3181](https://github.com/manticoresoftware/manticoresearch/issues/3181) 修复了使用大于 2^63 的 ID 时文本字段值可能丢失的问题。
* [Issue #3189](https://github.com/manticoresoftware/manticoresearch/issues/3189) 修正了 `UPDATE` 语句现在正确遵守 `query_log_min_msec` 设置。
* [Issue #3247](https://github.com/manticoresoftware/manticoresearch/issues/3247) 修复了保存实时磁盘块时的竞态条件，避免了 `JOIN CLUSTER` 失败。


## 版本 7.4.6
发布时间：2025年2月28日

### 主要更改
* [Issue #832](https://github.com/manticoresoftware/manticoresearch/issues/832) 集成了 [Kibana](Integration/Kibana.md)，实现更简单高效的数据可视化。

### 次要更改
* [Issue #1727](https://github.com/manticoresoftware/manticoresearch/issues/1727) 修复了 arm64 和 x86_64 之间浮点精度差异。
* [Issue #2995](https://github.com/manticoresoftware/manticoresearch/issues/2995) 实现了连接批处理的性能优化。
* [Issue #3039](https://github.com/manticoresoftware/manticoresearch/issues/3039) 实现了直方图中 EstimateValues 的性能优化。
* [Issue #3099](https://github.com/manticoresoftware/manticoresearch/issues/3099) 新增对 Boost 1.87.0 的支持。❤️ 感谢 [@cho-m](https://github.com/cho-m) 的 PR。
* [Issue #77](https://github.com/manticoresoftware/columnar/issues/77) 优化了创建多值过滤器时的块数据复用；为属性元数据添加了最小值/最大值；实现了基于最小值/最大值的过滤值预过滤。

### Bug 修复
* [Commit 73ac](https://github.com/manticoresoftware/manticoresearch/commit/73ac22f358a7a0e734b332c3943c86e6294c06c3) 修复了在连接查询中同时使用左右表属性时表达式的处理；修复了右表的 index_weights 选项。
* [Issue #2915](https://github.com/manticoresoftware/manticoresearch/issues/2915) 在 `SELECT ... JOIN` 查询中使用 `avg()` 可能导致结果错误；现已修复。
* [Issue #2996](https://github.com/manticoresoftware/manticoresearch/issues/2996) 修复了启用连接批处理时隐式截止导致的错误结果集。
* [Issue #3031](https://github.com/manticoresoftware/manticoresearch/issues/3031) 修复了守护进程在关闭时若正进行块合并可能崩溃的问题。
* [Issue #3037](http://github.com/manticoresoftware/manticoresearch/issues/3037) 修复了 `IN(...)` 可能产生错误结果的问题。
* [Issue #3038](https://github.com/manticoresoftware/manticoresearch/issues/3038) 7.0.0 版本设置 `max_iops` / `max_iosize` 可能会降低索引性能；现已修复。
* [Issue #3042](https://github.com/manticoresoftware/manticoresearch/issues/3042) 修复了连接查询缓存中的内存泄漏。
* [Issue #3052](https://github.com/manticoresoftware/manticoresearch/issues/3052) 修复了连接 JSON 查询中查询选项的处理。
* [Issue #3054](https://github.com/manticoresoftware/manticoresearch/issues/3054) 修复了 ATTACH TABLE 命令的问题。
* [Issue #3079](https://github.com/manticoresoftware/manticoresearch/issues/3079) 修正了错误信息中的不一致。
* [Issue #3087](https://github.com/manticoresoftware/manticoresearch/issues/3087) 按表设置 `diskchunk_flush_write_timeout=-1` 未禁用索引刷新；现已修复。
* [Issue #3088](https://github.com/manticoresoftware/manticoresearch/issues/3088) 解决了大 ID 批量替换后的重复条目问题。
* [Issue #3126](https://github.com/manticoresoftware/manticoresearch/issues/3126) 修复了使用单个 `NOT` 操作符和表达式排名器的全文查询导致守护进程崩溃的问题。
* [Issue #3128](https://github.com/manticoresoftware/manticoresearch/pull/3128) 修复了 CJSON 库中的潜在漏洞。❤️ 感谢 [@tabudz](https://github.com/tabudz) 的 PR。

## 版本 7.0.0
发布时间：2025年1月30日

### 主要更改
* [Issue #1497](https://github.com/manticoresoftware/manticoresearch/issues/1497) 新增了[模糊搜索](Searching/Spell_correction.md#Fuzzy-Search)和[自动补全](Searching/Autocomplete.md#CALL-AUTOCOMPLETE)功能，方便搜索。
* [Issue #1500](https://github.com/manticoresoftware/manticoresearch/issues/1500) [与 Kafka 集成](Integration/Kafka.md#Syncing-from-Kafka)。
* [Issue #1928](https://github.com/manticoresoftware/manticoresearch/issues/1928) 引入了用于 JSON 的 [二级索引](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#json_secondary_indexes)。
* [Issue #2361](https://github.com/manticoresoftware/manticoresearch/issues/2361) 更新和更新期间的搜索不再被块合并阻塞。
* [Issue #2787](https://github.com/manticoresoftware/manticoresearch/issues/2787) 实现了针对 RT 表的自动 [磁盘块刷新](Server_settings/Searchd.md#diskchunk_flush_write_timeout) 以提升性能；现在，我们会自动将内存块刷新为磁盘块，防止因内存块缺少优化导致的性能问题，有时这种情况会根据块大小导致不稳定。
* [Issue #2811](https://github.com/manticoresoftware/manticoresearch/issues/2811) 添加了用于简化分页的 [Scroll](Searching/Pagination.md#Scroll-Search-Option) 选项。
* [Issue #931](https://github.com/manticoresoftware/manticoresearch/issues/931) 集成了 [Jieba](https://github.com/fxsjy/jieba) 以改进对 [中文分词](Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md) 的支持。

### 次要更改
* ⚠️ 破坏性更改 [Issue #1111](https://github.com/manticoresoftware/manticoresearch/issues/1111) 修正了 RT 表对 `global_idf` 的支持。需要重新创建表。
* ⚠️ 破坏性更改 [Issue #2103](https://github.com/manticoresoftware/manticoresearch/issues/2103) 移除了内部 `cjk` 字符集中的泰文字符。请相应更新您的字符集定义：如果您有 `cjk,non_cjk` 并且泰文字符对您很重要，请改为 `cjk,thai,non_cjk`，或者 `cont,non_cjk`，其中 `cont` 是所有连续脚本语言的新标识（即 `cjk` + `thai`）。使用 [ALTER TABLE](Updating_table_schema_and_settings.md#Updating-table-FT-settings-in-RT-mode) 修改现有表。
* ⚠️ 破坏性更改 [Issue #2468](https://github.com/manticoresoftware/manticoresearch/issues/2468) [CALL SUGGEST / QSUGGEST](Searching/Spell_correction.md#CALL-QSUGGEST,-CALL-SUGGEST) 现已支持分布式表。这增加了主服务器/代理协议版本。如果您在分布式环境中运行 Manticore Search 且有多个实例，请确保先升级代理，再升级主服务器。
* ⚠️ 破坏性更改 [Issue #2889](https://github.com/manticoresoftware/manticoresearch/issues/2889) 更改了 PQ [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META-for-PQ-tables) 中的列名，由 `Name` 改为 `Variable name`。
* ⚠️ 破坏性更改 [Issue #879](https://github.com/manticoresoftware/manticoresearch/issues/879) 引入了[每表二进制日志](Logging/Binary_logging.md#Per-table-binary-logging-configuration)，带有新选项：[binlog_common](Logging/Binary_logging.md#Binary-logging-strategies)、[binlog](Logging/Binary_logging.md#Per-table-binary-logging-configuration) 用于 `create table` / `alter table`。升级到新版本前需干净关闭 Manticore 实例。
* ⚠️ 破坏性更改 [Issue #1789](https://github.com/manticoresoftware/manticoresearch/issues/1789) 修正了节点因复制协议版本错误而加入集群时的错误提示。该修改更新了复制协议。若运行复制集群，您需要：
  - 先干净停止所有节点
  - 然后使用 Linux 工具 `manticore_new_cluster` 以 `--new-cluster` 参数启动最后停止的节点。
  - 详细内容请参阅 [重启集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster)。
* ⚠️ 破坏性更改 [Issue #2308](https://github.com/manticoresoftware/manticoresearch/issues/2308) 在 [`ALTER CLUSTER ADD` 和 `DROP`](Creating_a_cluster/Setting_up_replication/Adding_and_removing_a_table_from_a_replication_cluster.md#Adding-and-removing-a-table-from-a-replication-cluster) 中增加了对多表支持。此更改也影响复制协议。请参考前一个部分了解如何处理此更新。
* [Issue #2997](https://github.com/manticoresoftware/manticoresearch/issues/2997) 修复了 macOS 上的 dlopen 问题。
* [Commit 4954](https://github.com/manticoresoftware/manticoresearch/commit/4954b5de7341a29902a9b8fbb9a040f7942c77c4) 修改了带 KNN 索引表的 OPTIMIZE TABLE 默认截止条件，以提升搜索性能。
* [Commit cfc8](https://github.com/manticoresoftware/manticoresearch/commit/cfc87ecb6e33a8163c2235243b6b40e699dbf526) 为 `FACET` 和 `GROUP BY` 中的 `ORDER BY` 增加了对 `COUNT(DISTINCT)` 的支持。
* [Issue #1103](https://github.com/manticoresoftware/manticoresearch/issues/1103) 改进了 [日志记录](Logging/Server_logging.md#Server-logging) 中块合并的清晰度。
* [Issue #1130](https://github.com/manticoresoftware/manticoresearch/issues/1130) 添加了对 [DBeaver](Integration/DBeaver.md) 的支持。
* [Issue #1546](https://github.com/manticoresoftware/manticoresearch/issues/1546) 为 [POLY2D()](Functions/Geo_spatial_functions.md#POLY2D%28%29)/[GEOPOLY2D()](Functions/Geo_spatial_functions.md#GEOPOLY2D%28%29) 函数实现了二级索引。
* [Issue #1630](https://github.com/manticoresoftware/manticoresearch/issues/1630) HTTP 请求现支持 `Content-Encoding: gzip`。
* [Issue #1831](https://github.com/manticoresoftware/manticoresearch/issues/1831) 添加了 `SHOW LOCKS` 命令。
* [Issue #2187](https://github.com/manticoresoftware/manticoresearch/issues/2197) 允许 Buddy 请求绕过守护进程的 [searchd.max_connections](Server_settings/Searchd.md#max_connections) 限制。
* [Issue #2208](https://github.com/manticoresoftware/manticoresearch/issues/2208) 为通过 JSON HTTP 接口连接表添加了支持。
* [Issue #2235](https://github.com/manticoresoftware/manticoresearch/issues/2235) 通过 Buddy 记录成功处理的查询，以其原始形式。
* [问题 #2249](https://github.com/manticoresoftware/manticoresearch/issues/2249) 为复制表添加了特殊模式以运行 `mysqldump`。
* [问题 #2268](https://github.com/manticoresoftware/manticoresearch/issues/2268) 改进了 `CREATE TABLE` 和 `ALTER TABLE` 语句中外部文件的拷贝重命名。
* [问题 #2402](https://github.com/manticoresoftware/manticoresearch/issues/2402) 更新了 [searchd.max_packet_size](Server_settings/Searchd.md#max_packet_size) 的默认值为128MB。
* [问题 #2419](https://github.com/manticoresoftware/manticoresearch/issues/2419) 在 JSON ["match"](Searching/Full_text_matching/Basic_usage.md#match) 中增加了对 [IDF 提升修饰符](Searching/Full_text_matching/Operators.md#IDF-boost-modifier) 的支持。
* [问题 #2430](https://github.com/manticoresoftware/manticoresearch/issues/2430) 增强了 [binlog](Logging/Binary_logging.md#Binary-logging) 写入同步以防止错误。
* [问题 #2458](https://github.com/manticoresoftware/manticoresearch/issues/2458) 将 zlib 支持整合进 Windows 软件包。
* [问题 #2479](https://github.com/manticoresoftware/manticoresearch/issues/2479) 添加了对 SHOW TABLE INDEXES 命令的支持。
* [问题 #2485](https://github.com/manticoresoftware/manticoresearch/issues/2485) 为 Buddy 回复设置了会话元数据。
* [问题 #2490](https://github.com/manticoresoftware/manticoresearch/issues/2490) 兼容端点的聚合支持毫秒级分辨率。
* [问题 #2500](https://github.com/manticoresoftware/manticoresearch/issues/2500) 复制启动失败时，集群操作的错误信息已更改。
* [问题 #2584](https://github.com/manticoresoftware/manticoresearch/pull/2584) 新增 [SHOW STATUS](Node_info_and_management/Node_status.md#Query-Time-Statistics) 中的性能指标：最近1、5、15分钟内每种查询类型的最小/最大/平均/第95百分位/第99百分位统计。
* [问题 #2639](https://github.com/manticoresoftware/manticoresearch/pull/2639) 请求和响应中所有 `index` 实例改为 `table`。
* [问题 #2643](https://github.com/manticoresoftware/manticoresearch/issues/2643) 在 HTTP `/sql` 端点的聚合结果中添加了 `distinct` 列。
* [问题 #268](https://github.com/manticoresoftware/manticoresearch-buddy/issues/268) 实现了从 Elasticsearch 导入数据类型的自动检测。
* [问题 #2744](https://github.com/manticoresoftware/manticoresearch/issues/2744) 为字符串 JSON 字段比较表达式添加了排序规则支持。
* [问题 #2752](https://github.com/manticoresoftware/manticoresearch/issues/2752) 支持在选择列表中使用 `uuid_short` 表达式。
* [问题 #2783](https://github.com/manticoresoftware/manticoresearch/issues/2783) Manticore Search 现在直接运行 Buddy，无需 `manticore-buddy` 包装器。
* [问题 #2785](https://github.com/manticoresoftware/manticoresearch/issues/2785) 区分了缺少表和不支持插入操作的表的错误信息。
* [问题 #2789](https://github.com/manticoresoftware/manticoresearch/issues/2789) OpenSSL 3现已静态编译进 `searchd`。
* [问题 #2790](https://github.com/manticoresoftware/manticoresearch/issues/2790) 增加了 `CALL uuid_short` 语句，用于生成带有多个 `uuid_short` 值的序列。
* [问题 #2803](https://github.com/manticoresoftware/manticoresearch/issues/2803) 为 JOIN 操作中的右表添加了独立选项。
* [问题 #2810](https://github.com/manticoresoftware/manticoresearch/issues/2810) 改进了 HTTP JSON 聚合性能，使其与 SphinxQL 的 `GROUP BY` 相匹配。
* [问题 #2854](https://github.com/manticoresoftware/manticoresearch/issues/2854) 支持 Kibana 日期相关请求中的 `fixed_interval`。
* [问题 #2909](https://github.com/manticoresoftware/manticoresearch/issues/2909) 实现了 JOIN 查询的批处理，大幅提升部分 JOIN 查询的性能，提升幅度可达数百甚至上千倍。
* [问题 #2937](https://github.com/manticoresoftware/manticoresearch/issues/2937) 允许在 fullscan 查询中使用连接表权重。
* [问题 #2953](https://github.com/manticoresoftware/manticoresearch/issues/2953) 修正了连接查询的日志记录。
* [问题 #337](https://github.com/manticoresoftware/manticoresearch-buddy/issues/337) 在非调试模式下隐藏了 Buddy 异常，不在 `searchd` 日志中显示。
* [问题 #2931](https://github.com/manticoresoftware/manticoresearch/issues/2931) 如果用户为复制监听器设置了错误的端口，守护进程关闭并显示错误信息。

### Bug 修复
* [提交 0c6b](https://github.com/manticoresoftware/manticoresearch/commit/0c6bdaf41ea3684e05952e1a8273893535b923f5) 修复：当 JOIN 查询中含有超过32列时错误返回结果。
* [问题 #2335](https://github.com/manticoresoftware/manticoresearch/issues/2335) 解决了当条件中使用两个 JSON 属性时，表连接失败的问题。
* [问题 #2338](https://github.com/manticoresoftware/manticoresearch/issues/2338) 修正了含 [cutoff](Searching/Options.md#cutoff) 的多查询中的 total_relation 错误。
* [问题 #2366](https://github.com/manticoresoftware/manticoresearch/issues/2366) 修正了在[表连接](Searching/Joining.md)中对右表 `json.string` 的过滤。
* [问题 #2406](https://github.com/manticoresoftware/manticoresearch/issues/2406) 允许所有 POST HTTP JSON 端点（insert/replace/bulk）中使用 `null` 作为值，使用默认值替代。
* [问题 #2418](https://github.com/manticoresoftware/manticoresearch/issues/2418) 通过调整初始 socket 探测时 [max_packet_size](Server_settings/Searchd.md#max_packet_size) 网络缓冲区的分配，优化了内存消耗。
* [问题 #2420](https://github.com/manticoresoftware/manticoresearch/issues/2420) 修复了通过 JSON 接口将无符号整数插入 bigint 属性的问题。
* [问题 #2422](https://github.com/manticoresoftware/manticoresearch/issues/2422) 修正了当启用 exclude filters 和 pseudo_sharding 时，二级索引的正确工作。
* [Issue #2423](https://github.com/manticoresoftware/manticoresearch/issues/2423) 解决了 [manticore_new_cluster](Starting_the_server/Manually.md#searchd-command-line-options) 中的一个错误。
* [Issue #2448](https://github.com/manticoresoftware/manticoresearch/issues/2448) 修复了在格式错误的 `_update` 请求时守护进程崩溃的问题。
* [Issue #2452](https://github.com/manticoresoftware/manticoresearch/issues/2452) 修复了直方图无法处理带有排除项的值过滤器的问题。
* [Issue #55](https://github.com/manticoresoftware/columnar/issues/55) 修复了针对分布式表的 [knn](Searching/KNN.md#KNN-vector-search) 查询的问题。
* [Issue #68](https://github.com/manticoresoftware/columnar/issues/68) 增强了列访问器中对表编码上的排除过滤器的处理。
* [Commit 0eb1](https://github.com/manticoresoftware/manticoresearch/commit/0eb1579462013cb134bcaf74e06f6eeb0c2ecc4d) 修复了表达式解析器未遵守重新定义的 `thread_stack` 的问题。
* [Commit c304](https://github.com/manticoresoftware/manticoresearch/commit/c3041fd1d3e14773f3c5aa635343915a71f5cb95) 修复了克隆列式 IN 表达式时的崩溃。
* [Commit edad](https://github.com/manticoresoftware/columnar/commit/edadc694c68d6022bdd13134263667430a42cc1d) 修复了位图迭代器的反转问题，该问题导致崩溃。
* [Commit fc30](https://github.com/manticoresoftware/executor/commit/fc302ff1117b0b835a6f5a3c07057baf5fba14d9) 修复了某些 Manticore 软件包被 `unattended-upgrades` 自动移除的问题。
* [Issue #1019](https://github.com/manticoresoftware/manticoresearch/issues/1019) 改进了对 DbForge MySQL 工具发出查询的处理。
* [Issue #1107](https://github.com/manticoresoftware/manticoresearch/issues/1107) 修复了 `CREATE TABLE` 和 `ALTER TABLE` 中特殊字符的转义问题。❤️ 感谢 [@subnix](https://github.com/subnix) 的 PR。
* [Issue #116](https://github.com/manticoresoftware/manticoresearch-backup/issues/116#issuecomment-2216597206) 修复了在冻结索引中更新 blob 属性时的死锁。该死锁发生于尝试解冻索引时锁冲突，也可能导致 manticore-backup 失败。
* [Issue #1818](https://github.com/manticoresoftware/manticoresearch/issues/1818) 当表被冻结时，`OPTIMIZE` 现在会抛出错误。
* [Issue #2001](https://github.com/manticoresoftware/manticoresearch/issues/2001) 允许函数名被用作列名。
* [Issue #2153](https://github.com/manticoresoftware/manticoresearch/issues/2153) 修复了在查询未知磁盘块的表设置时守护进程崩溃的问题。
* [Issue #2184](https://github.com/manticoresoftware/manticoresearch/issues/2184) 修复了 `searchd` 在执行 `FREEZE` 和 `FLUSH RAMCHUNK` 后停止时挂起的问题。
* [Issue #2228](https://github.com/manticoresoftware/manticoresearch/issues/2228) 从保留字中移除了日期/时间相关的标记（及正则表达式）。
* [Issue #2255](https://github.com/manticoresoftware/manticoresearch/issues/2255) 修复了在包含超过 5 个排序字段的 `FACET` 操作中崩溃的问题。
* [Issue #2265](https://github.com/manticoresoftware/manticoresearch/issues/2265) 修复了启用 `index_field_lengths` 时 `mysqldump` 恢复失败的问题。
* [Issue #2291](https://github.com/manticoresoftware/manticoresearch/issues/2291) 修复了执行 `ALTER TABLE` 命令时崩溃的问题。
* [Issue #2333](https://github.com/manticoresoftware/manticoresearch/issues/2333) 修复了 Windows 包中 MySQL DLL 以确保索引器正常工作。
* [Issue #2393](https://github.com/manticoresoftware/manticoresearch/issues/2393) 修复了 GCC 编译错误。❤️ 感谢 [@animetosho](https://github.com/animetosho) 的 PR。
* [Issue #2447](https://github.com/manticoresoftware/manticoresearch/issues/2447) 修复了 [_update](Data_creation_and_modification/Updating_documents/REPLACE.md?client=Elasticsearch-like+partial) 中的转义问题。
* [Issue #2460](https://github.com/manticoresoftware/manticoresearch/issues/2460) 修复了声明多个同名属性或字段时索引器崩溃的问题。
* [Issue #2467](https://github.com/manticoresoftware/manticoresearch/issues/2467) 解决了在“兼容”搜索相关端点对于嵌套布尔查询的错误转换导致守护进程崩溃的问题。
* [Issue #2493](https://github.com/manticoresoftware/manticoresearch/issues/2493) 修复了带修饰符的短语展开问题。
* [Issue #2535](https://github.com/manticoresoftware/manticoresearch/issues/2535) 解决了使用 [ZONE](Searching/Full_text_matching/Operators.md#ZONE-limit-operator) 或 [ZONESPAN](Searching/Full_text_matching/Operators.md#ZONESPAN-limit-operator) 全文操作符时守护进程崩溃的问题。
* [Issue #2552](https://github.com/manticoresoftware/manticoresearch/issues/2552) 修复了带关键字字典的普通表和 RT 表的中缀生成问题。
* [Issue #2571](https://github.com/manticoresoftware/manticoresearch/issues/2571) 修复了 `FACET` 查询中的错误回复；对带有 `COUNT(*)` 的 `FACET` 设置默认排序顺序为 `DESC`。
* [Issue #2580](https://github.com/manticoresoftware/manticoresearch/issues/2580) 修复了 Windows 启动时守护进程崩溃的问题。
* [Issue #2603](https://github.com/manticoresoftware/manticoresearch/issues/2603) 修复了 HTTP 端点 `/sql` 和 `/sql?mode=raw` 的查询截断问题；使这些端点的请求无需 `query=` 头部即保持一致。
* [Issue #2623](https://github.com/manticoresoftware/manticoresearch/issues/2623) 修复了自动模式创建表时同时失败的问题。
* [Issue #2627](https://github.com/manticoresoftware/manticoresearch/issues/2627) 修复了 HNSW 库以支持加载多个 KNN 索引。
* [Issue #2630](https://github.com/manticoresoftware/manticoresearch/issues/2630) 修复了多条件同时出现时的冻结问题。
* [Issue #2645](https://github.com/manticoresoftware/manticoresearch/issues/2645) 修复了使用 `OR` 结合 KNN 搜索时导致的致命错误崩溃。
* [Issue #2647](https://github.com/manticoresoftware/manticoresearch/issues/2647) 修复了 `indextool --mergeidf *.idf --out global.idf` 创建输出文件后删除该文件的问题。
* [Issue #2658](https://github.com/manticoresoftware/manticoresearch/issues/2658) 修复了在外层查询中带有 `ORDER BY` 字符串的子查询导致守护进程崩溃的问题。
* [Issue #2686](https://github.com/manticoresoftware/manticoresearch/issues/2686) 修复了同时更新浮点属性和字符串属性时的崩溃问题。
* [Issue #2704](https://github.com/manticoresoftware/manticoresearch/issues/2704) 修复了来自 `lemmatize_xxx_all` 分词器的多个停用词增加后续令牌的 `hitpos` 的问题。
* [Issue #2708](https://github.com/manticoresoftware/manticoresearch/issues/2708) 修复了 `ALTER ... ADD COLUMN ... TEXT` 导致的崩溃。
* [Issue #2737](https://github.com/manticoresoftware/manticoresearch/issues/2737) 修复了在包含至少一个 RAM 块的冻结表中更新 blob 属性导致后续 `SELECT` 查询等待表解冻的问题。
* [Issue #2742](https://github.com/manticoresoftware/manticoresearch/issues/2742) 修复了包含打包因子的查询跳过缓存的问题。
* [Issue #2775](https://github.com/manticoresoftware/manticoresearch/issues/2775) Manticore 现在在未知操作时报告错误，而非在 `_bulk` 请求时崩溃。
* [Issue #2791](https://github.com/manticoresoftware/manticoresearch/issues/2791) 修复了 HTTP `_bulk` 端点插入文档 ID 返回错误的问题。
* [Issue #2797](https://github.com/manticoresoftware/manticoresearch/issues/2797) 修复了在处理多个表时，其中一个为空且另一个匹配条目数量不同，grouper 崩溃的问题。
* [Issue #2835](https://github.com/manticoresoftware/manticoresearch/issues/2835) 修复了复杂 `SELECT` 查询中的崩溃。
* [Issue #2872](https://github.com/manticoresoftware/manticoresearch/issues/2872) 当 `IN` 表达式中的 `ALL` 或 `ANY` 参数不是 JSON 属性时，添加了错误消息。
* [Issue #2882](https://github.com/manticoresoftware/manticoresearch/issues/2882) 修复了在大表中更新 MVA 时报的守护进程崩溃。
* [Issue #2888](https://github.com/manticoresoftware/manticoresearch/issues/2888) 修复了因 `libstemmer` 分词失败导致的崩溃。❤️ 感谢 [@subnix](https://github.com/subnix) 的 PR。
* [Issue #2919](https://github.com/manticoresoftware/manticoresearch/issues/2919) 修复了右表连接权重在表达式中不正确的问题。
* [Issue #2919](https://github.com/manticoresoftware/manticoresearch/issues/2919) 修复了右连接表权重在表达式中不起作用的问题。
* [Issue #325](https://github.com/manticoresoftware/manticoresearch-buddy/issues/325) 修复了表已存在时执行 `CREATE TABLE IF NOT EXISTS ... WITH DATA` 失败的问题。
* [Issue #351](https://github.com/manticoresoftware/manticoresearch-buddy/issues/351) 修复了使用文档 ID 按 KNN 计数时报的未定义数组键 "id" 错误。
* [Issue #359](https://github.com/manticoresoftware/manticoresearch-buddy/issues/359) 修复了 `REPLACE INTO cluster_name:table_name` 功能的问题。
* [Issue #67](https://github.com/manticoresoftware/docker/issues/67) 修复了在使用 `--network=host` 运行 Manticore Docker 容器时的致命错误。

## 版本 6.3.8
发布：2024年11月22日

版本 6.3.8 延续了 6.3 系列，仅包含错误修复。

### 错误修复

* [PR #2777](https://github.com/manticoresoftware/manticoresearch/pull/2777) 修正了当查询并发由 `threads` 或 `max_threads_per_query` 设置限制时可用线程数的计算。

## manticore-extra v1.1.20

发布：2024年10月7日

### 错误修复

* [Issue #64](https://github.com/manticoresoftware/executor/issues/64) 解决了在基于 Debian 的系统中，`unattended-upgrades` 工具错误地将包括 `manticore-galera`、`manticore-executor` 和 `manticore-columnar-lib` 在内的多个 Manticore 软件包标记为移除的问题。原因是 `dpkg` 错误地将虚拟包 `manticore-extra` 视为冗余。已做出更改，确保 `unattended-upgrades` 不再试图移除关键的 Manticore 组件。

## 版本 6.3.6
发布：2024年8月2日

版本 6.3.6 延续了 6.3 系列，仅包含错误修复。

### 错误修复

* [Issue #2477](https://github.com/manticoresoftware/manticoresearch/issues/2477) 修复了 6.3.4 版本引入的崩溃，该崩溃可能在处理表达式及分布式或多个表时发生。
* [Issue #2352](https://github.com/manticoresoftware/manticoresearch/issues/2352) 修复了 `max_query_time` 引起的在查询多个索引时的守护进程崩溃或内部错误。

## 版本 6.3.4
发布：2024年7月31日

版本 6.3.4 延续了 6.3 系列，包含轻微改进和错误修复。

### 次要改动
* [Issue #1130](https://github.com/manticoresoftware/manticoresearch/issues/1130) 增加了对 [DBeaver](Integration/DBeaver.md#Integration-with-DBeaver) 的支持。
* [Issue #2146](https://github.com/manticoresoftware/manticoresearch/issues/2146) 改进了词形和例外中的分隔符转义。
* [Issue #2315](https://github.com/manticoresoftware/manticoresearch/issues/2315) 在 SELECT 列表达式中添加了字符串比较运算符。
* [Issue #2363](https://github.com/manticoresoftware/manticoresearch/issues/2363) 增加了对 Elastic 风格批量请求中 null 值的支持。
* [Issue #2374](https://github.com/manticoresoftware/manticoresearch/issues/2374) 增加了对 mysqldump 9 版本的支持。
* [Issue #2375](https://github.com/manticoresoftware/manticoresearch/issues/2375) 改进了 HTTP JSON 查询中的错误处理，指向发生错误的 JSON 路径节点。

### 错误修复
* [Issue #2280](https://github.com/manticoresoftware/manticoresearch/issues/2280) 修复了当 `disk_chunks > 1` 时，包含大量匹配项的通配符查询性能下降的问题。
* [Issue #2332](https://github.com/manticoresoftware/manticoresearch/issues/2332) 修复了空 MVA 数组的 MVA MIN 或 MAX SELECT 列表达式崩溃的问题。
* [Issue #2339](https://github.com/manticoresoftware/manticoresearch/issues/2339) 修复了 Kibana 无限范围请求的错误处理。
* [Issue #2342](https://github.com/manticoresoftware/manticoresearch/issues/2342) 修复了右表列式属性在属性不在 SELECT 列表中时的连接过滤器问题。
* [Issue #2343](https://github.com/manticoresoftware/manticoresearch/issues/2343) 修复了 Manticore 6.3.2 中重复的 'static' 说明符。
* [Issue #2344](https://github.com/manticoresoftware/manticoresearch/issues/2344) 修复了当对右表使用 MATCH() 时 LEFT JOIN 返回非匹配条目问题。
* [Issue #2350](https://github.com/manticoresoftware/manticoresearch/issues/2350) 修复了带有 `hitless_words` 的 RT 索引磁盘块保存问题。
* [Issue #2364](https://github.com/manticoresoftware/manticoresearch/issues/2364) 'aggs_node_sort' 属性现在可以在其他属性中任意顺序添加。
* [Issue #2368](https://github.com/manticoresoftware/manticoresearch/issues/2368) 修复了 JSON 查询中全文与过滤器顺序错误问题。
* [Issue #2376](https://github.com/manticoresoftware/manticoresearch/issues/2376) 修复了长 UTF-8 请求返回错误 JSON 响应的缺陷。
* [Issue #2684](https://github.com/manticoresoftware/dev/issues/2684) 修复了依赖连接属性的预排序/预过滤表达式计算问题。
* [Issue #301](https://github.com/manticoresoftware/manticoresearch-buddy/issues/301) 更改了度量数据大小计算方法，改从 `manticore.json` 文件读取，而不是检查数据目录整体大小。
* [Issue #302](https://github.com/manticoresoftware/manticoresearch-buddy/issues/302) 添加了对 Vector.dev 验证请求的处理。

## Version 6.3.2
发布：2024年6月26日

6.3.2 版本是 6.3 系列的延续，包含若干错误修复，其中一些是在 6.3.0 发布后发现的。

### 重大变更
* ⚠️[Issue #2305](https://github.com/manticoresoftware/manticoresearch/issues/2305) 更新 aggs.range 值为数值类型。

### 错误修复
* [Commit c51c](https://github.com/manticoresoftware/manticoresearch/commit/c51c7fa4bd0ec4ad8d53188b888461bff29f1417) 修复了基于存储检查与 rset 合并的分组问题。
* [Commit 0e85](https://github.com/manticoresoftware/manticoresearch/commit/0e85b9d5682f51681750d798aef6a11cfb93d69e) 修复了使用带有 CRC 字典和启用 `local_df` 的 RT 索引通配符查询时守护进程崩溃问题。
* [Issue #2200](https://github.com/manticoresoftware/manticoresearch/issues/2200) 修复了未使用 GROUP BY 进行 `count(*)` 的 JOIN 崩溃问题。
* [Issue #2201](https://github.com/manticoresoftware/manticoresearch/issues/2201) 修复了对全文字段尝试分组时 JOIN 未返回警告的问题。
* [Issue #2230](https://github.com/manticoresoftware/manticoresearch/issues/2230) 解决了通过 `ALTER TABLE` 添加属性时未考虑 KNN 选项的问题。
* [Issue #2231](https://github.com/manticoresoftware/manticoresearch/issues/2231) 修复了 6.3.0 版本中无法移除 `manticore-tools` Redhat 包的问题。
* [Issue #2242](https://github.com/manticoresoftware/manticoresearch/issues/2242) 修正了 JOIN 和多个 FACET 语句返回错误结果的问题。
* [Issue #2250](https://github.com/manticoresoftware/manticoresearch/issues/2250) 修复了表处于集群中时 ALTER TABLE 出错的问题。
* [Issue #2252](https://github.com/manticoresoftware/manticoresearch/issues/2252) 修正了 SphinxQL 接口传递给 buddy 的原始查询问题。
* [Issue #2267](https://github.com/manticoresoftware/manticoresearch/issues/2267) 改进了带有磁盘块的 RT 索引中 `CALL KEYWORDS` 的通配符扩展功能。
* [Issue #271](https://github.com/manticoresoftware/manticoresearch-buddy/issues/271) 修复了错误的 `/cli` 请求导致的挂起。
* [Issue #274](https://github.com/manticoresoftware/manticoresearch-buddy/issues/274) 解决了对 Manticore 的并发请求可能出现卡顿的问题。
* [Issue #275](https://github.com/manticoresoftware/manticoresearch-buddy/issues/275) 修复了通过 `/cli` 执行 `drop table if exists t; create table t` 命令时的挂起问题。

### 复制相关
* [Issue #2270](https://github.com/manticoresoftware/manticoresearch/issues/2270) 在 `/_bulk` HTTP 端点添加了对 `cluster:name` 格式的支持。

## Version 6.3.0
发布：2024年5月23日

### 主要变更
* [Issue #839](https://github.com/manticoresoftware/manticoresearch/issues/839) 实现了 [float_vector](Creating_a_table/Data_types.md#Float-vector) 数据类型；实现了 [向量搜索](Searching/KNN.md#KNN-vector-search)。
* [Issue #1673](https://github.com/manticoresoftware/manticoresearch/issues/1673) [INNER/LEFT JOIN](Searching/Joining.md)（**测试阶段**）。
* [Issue #1744](https://github.com/manticoresoftware/manticoresearch/issues/1744) 实现了对 [timestamp](Creating_a_table/Data_types.md#Timestamps) 字段日期格式的自动检测。
* [Issue #1720](https://github.com/manticoresoftware/manticoresearch/pull/1720) 将 Manticore Search 许可证从 GPLv2-or-later 改为 GPLv3-or-later。
* [Commit 7a55](https://github.com/manticoresoftware/manticoresearch/commit/7a5508424) Windows 平台运行 Manticore 现在需要 Docker 来运行 Buddy。
* [Issue #1541](https://github.com/manticoresoftware/manticoresearch/issues/1541) 添加了一个 [REGEX](Searching/Full_text_matching/Operators.md#REGEX-operator) 全文操作符。
* [Issue #2091](https://github.com/manticoresoftware/manticoresearch/issues/2091) 支持 Ubuntu Noble 24.04。
* [Commit 514d](https://github.com/manticoresoftware/manticoresearch/commit/514d35b497f4bdb20b6473ff963752a978a4bb8d) 时间操作改进以提升性能并新增日期/时间函数：
  - [CURDATE()](Functions/Date_and_time_functions.md#CURDATE%28%29) - 返回本地时区当前日期
  - [QUARTER()](Functions/Date_and_time_functions.md#QUARTER%28%29) - 返回时间戳参数对应的年份季度的整数值
  - [DAYNAME()](Functions/Date_and_time_functions.md#DAYNAME%28%29) - 返回给定时间戳参数的星期名称
  - [MONTHNAME()](Functions/Date_and_time_functions.md#MONTHNAME%28%29) - 返回给定时间戳参数对应的月份名称
  - [DAYOFWEEK()](Functions/Date_and_time_functions.md#DAYOFWEEK%28%29) - 返回给定时间戳参数对应的星期整数索引
  - [DAYOFYEAR()](Functions/Date_and_time_functions.md#DAYOFYEAR%28%29) - 返回给定时间戳参数对应的年份中的天数整数
  - [YEARWEEK()](Functions/Date_and_time_functions.md#YEARWEEK%28%29) - 返回给定时间戳参数对应的年份整数和当前周第一天的日期代码
  - [DATEDIFF()](Functions/Date_and_time_functions.md#DATEDIFF%28%29) - 返回两个时间戳之间的天数差
  - [DATE()](Functions/Date_and_time_functions.md#DATE%28%29) - 格式化时间戳参数的日期部分
  - [TIME()](Functions/Date_and_time_functions.md#TIME%28%29) - 格式化时间戳参数的时间部分
  - [timezone](Server_settings/Searchd.md#timezone) - 日期/时间相关函数使用的时区。
* [Commit 30e7](https://github.com/manticoresoftware/manticoresearch/commit/30e789d9657f9b3093c280c6e5b0e46a021f6dc7) 为HTTP接口添加了[range](Searching/Faceted_search.md#Facet-over-set-of-ranges)、[histogram](Searching/Faceted_search.md#Facet-over-histogram-values)、[date_range](Searching/Faceted_search.md#Facet-over-set-of-date-ranges) 和 [date_histogram](Searching/Faceted_search.md#Facet-over-histogram-date-values) 聚合功能，并将类似表达式加入SQL。

### 次要变更
* [Issue #1285](https://github.com/manticoresoftware/manticoresearch/issues/1285) 支持Filebeat版本8.10 - 8.11。
* [Issue #1771](https://github.com/manticoresoftware/manticoresearch/issues/1771) 增加了 [ALTER TABLE ... type='distributed'](Updating_table_schema_and_settings.md#Changing-a-distributed-table) 功能。
* [Issue #1788](https://github.com/manticoresoftware/manticoresearch/issues/1788) 增加了使用 [CREATE TABLE ... LIKE ... WITH DATA](Creating_a_table/Local_tables/Real-time_table.md#CREATE-TABLE-LIKE:) SQL语句复制表的功能。
* [Issue #2072](https://github.com/manticoresoftware/manticoresearch/issues/2072) 优化了 [表压缩算法](Securing_and_compacting_a_table/Compacting_a_table.md#Compacting-a-Table)：此前，无论是手动的 [OPTIMIZE](Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE) 还是自动的 [auto_optimize](Server_settings/Searchd.md#auto_optimize) 过程，都会先合并块以确保计数不超过限制，然后从包含已删除文档的所有其他块中清除已删除文档。这种方式有时资源消耗过大，现已禁用。现在，块合并仅根据 [progressive_merge](Server_settings/Common.md#progressive_merge) 设置进行。但包含大量已删除文档的块合并可能性更大。
* [Commit ce6c](https://github.com/manticoresoftware/columnar/commit/ce6c5d3381edcdc411a00a5923d13d56dd9b3010) 增加了对加载较新版次二级索引的保护机制。
* [Issue #1417](https://github.com/manticoresoftware/manticoresearch/issues/1417) 通过 [REPLACE INTO ... SET](Data_creation_and_modification/Updating_documents/REPLACE.md#SQL-REPLACE) 支持部分替换。
* [Commit 7c16](https://github.com/manticoresoftware/manticoresearch/commit/7c167b934ce416d3dfbd658744a231e638b14d9a) 更新了默认的合并缓冲区大小： `.spa`（标量属性）：256KB -> 8MB；`.spb`（二进制属性）：256KB -> 8MB；`.spc`（列存属性）：1MB，无变化；`.spds`（文档存储）：256KB -> 8MB；`.spidx`（二级索引）：256KB缓冲 -> 128MB内存限制；`.spi`（字典）：256KB -> 16MB；`.spd`（文档列表）：8MB，无变化；`.spp`（命中列表）：8MB，无变化；`.spe`（跳跃列表）：256KB -> 8MB。
* [Issue #1859](https://github.com/manticoresoftware/manticoresearch/issues/1859) 通过JSON添加了 [复合聚合](Searching/Grouping.md#GROUP-BY-multiple-fields-at-once)。
* [Commit 216b](https://github.com/manticoresoftware/manticoresearch-buddy/commit/216b824) 由于某些正则表达式模式存在问题且未带来显著时间优势，禁用了PCRE.JIT。
* [Commit 55cd](https://github.com/manticoresoftware/manticoresearch/commit/55cdce32) 支持原生Galera v.3 (api v25)（MySQL 5.x 的 `libgalera_smm.so`）。
* [Commit 86f9](https://github.com/manticoresoftware/manticoresearch-buddy/commit/86f98ab) 指标后缀由 `_rate` 改为 `_rps`。
* [Commit c0c1](https://github.com/manticoresoftware/manticoresearch-helm/commit/c0c1cff780031412ef57cefab987dfd16613368e) 改进了关于负载均衡器高可用性支持的文档。
* [Commit d1d2](https://github.com/manticoresoftware/manticoresearch/commit/d1d2ca18857c68106e89743756260e73285be739) 错误消息中将 `index` 改为 `table`；修正了bison解析器错误信息。
* [Commit fd26](https://github.com/manticoresoftware/manticoresearch/commit/fd26671b) 支持 `manticore.tbl` 作为表名。
* [Issue #1105](https://github.com/manticoresoftware/manticoresearch/issues/1105) 支持通过systemd运行索引器（[文档](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Schedule-indexer-via-systemd)）。❤️ 感谢 [@subnix](https://github.com/subnix) 的PR。
* [Issue #1294](https://github.com/manticoresoftware/manticoresearch/issues/1294) GEODIST()中支持二级索引。
* [Issue #1394](https://github.com/manticoresoftware/manticoresearch/issues/1394) 简化了 [SHOW THREADS](Node_info_and_management/SHOW_THREADS.md#SHOW-THREADS)。
* [Issue #1424](https://github.com/manticoresoftware/manticoresearch/issues/1424) 为 `create distributed table` 语句增加了对默认值（`agent_connect_timeout` 和 `agent_query_timeout`）的支持。
* [Issue #1442](https://github.com/manticoresoftware/manticoresearch/issues/1442) 添加了 [expansion_limit](Searching/Options.md#expansion_limit) 搜索查询选项，可覆盖 `searchd.expansion_limit`。
* [Issue #1448](https://github.com/manticoresoftware/manticoresearch/issues/1448) 实现了用于 int->bigint 转换的 [ALTER TABLE](Updating_table_schema_and_settings.md#Updating-table-schema-in-RT-mode)。
* [Issue #1456](https://github.com/manticoresoftware/manticoresearch/issues/1456) MySQL 响应中的元信息。
* [Issue #1494](https://github.com/manticoresoftware/manticoresearch/issues/1494) [SHOW VERSION](Node_info_and_management/SHOW_VERSION.md#SHOW-VERSION)。
* [Issue #1582](https://github.com/manticoresoftware/manticoresearch/issues/1582) 支持通过 JSON 按 id 数组 [删除文档](Data_creation_and_modification/Deleting_documents.md?client=JSON#Deleting-documents)。
* [Issue #1589](https://github.com/manticoresoftware/manticoresearch/issues/1589) 改进“unsupported value type”错误提示。
* [Issue #1634](https://github.com/manticoresoftware/manticoresearch/issues/1634) 在 `SHOW STATUS` 中添加了 Buddy 版本信息。
* [Issue #1641](https://github.com/manticoresoftware/manticoresearch/issues/1641) 针对关键词无文档的匹配请求进行了优化。
* [Issue #1712](https://github.com/manticoresoftware/manticoresearch/issues/1712) 在提交数据时增加了从字符串值 "true" 和 "false" 转换为 bool 属性的功能。
* [Issue #1713](https://github.com/manticoresoftware/manticoresearch/issues/1713) 添加了 [access_dict](Server_settings/Searchd.md#access_dict) 表和 searchd 选项。
* [Issue #1767](https://github.com/manticoresoftware/manticoresearch/issues/1767) 在配置的 searchd 部分新增选项：[expansion_merge_threshold_docs](Server_settings/Searchd.md#expansion_merge_threshold_docs) 和 [expansion_merge_threshold_hits](Server_settings/Searchd.md#expansion_merge_threshold_hits)；使合并扩展词中的小词阈值可配置。
* [Issue #1768](https://github.com/manticoresoftware/manticoresearch/issues/1768) 在 `@@system.sessions` 中新增显示最后命令时间。
* [Issue #1806](https://github.com/manticoresoftware/manticoresearch/issues/1806) 将 Buddy 协议升级至版本 2。
* [Issue #1810](https://github.com/manticoresoftware/manticoresearch/issues/1810) 为 `/sql` 端点实现了额外的请求格式，方便与库的集成。
* [Issue #1825](https://github.com/manticoresoftware/manticoresearch/pull/1825) 在 SHOW BUDDY PLUGINS 中新增 Info 部分。
* [Issue #1837](https://github.com/manticoresoftware/manticoresearch/issues/1837) 优化了在 `CALL PQ` 使用大包时的内存消耗。
* [Issue #1853](https://github.com/manticoresoftware/manticoresearch/pull/1853) 编译器从 Clang 15 切换到 Clang 16。
* [Issue #1857](https://github.com/manticoresoftware/manticoresearch/issues/1857) 添加了字符串比较功能。❤️ 感谢 [@etcd](https://github.com/etcd) 的 PR。
* [Issue #1915](https://github.com/manticoresoftware/manticoresearch/issues/1915) 添加了对联结存储字段的支持。
* [Issue #1937](https://github.com/manticoresoftware/manticoresearch/issues/1937) 在查询日志中记录客户端的 host:port。
* [Issue #1981](https://github.com/manticoresoftware/manticoresearch/issues/1981) 修正错误提示。
* [Issue #1983](https://github.com/manticoresoftware/manticoresearch/issues/1983) 引入了针对 [通过 JSON 查询计划的详细级别](Node_info_and_management/Profiling/Query_plan.md?client=JSON#Query-plan)支持的详细程度。
* [Issue #2010](https://github.com/manticoresoftware/manticoresearch/issues/2010) 除非设置了 `log_level=debug`，否则禁用 Buddy 查询日志记录。
* [Issue #2035](https://github.com/manticoresoftware/manticoresearch/issues/2035) 支持 Linux Mint 21.3。
* [Issue #2056](https://github.com/manticoresoftware/manticoresearch/issues/2056) 解决 Manticore 无法用 Mysql 8.3+ 构建的问题。
* [Issue #2112](https://github.com/manticoresoftware/manticoresearch/issues/2112) 为实时表块添加了 `DEBUG DEDUP` 命令，用于处理附加包含重复项的普通表后可能出现的重复条目。
* [Issue #212](https://github.com/manticoresoftware/manticoresearch-buddy/issues/212) 为 [SHOW QUERIES](Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES) 增加时间信息。
* [Issue #218](https://github.com/manticoresoftware/manticoresearch-buddy/issues/218) 将 `@timestamp` 列作为时间戳处理。
* [Issue #252](https://github.com/manticoresoftware/manticoresearch-buddy/issues/250) 实现了启用/禁用 buddy 插件的逻辑。
* [Issue #254](https://github.com/manticoresoftware/manticoresearch-buddy/issues/254) 更新 composer 至修复近期 CVE 的新版本。
* [Issue #340](https://github.com/manticoresoftware/manticoresearch/issues/340) 对 Manticore systemd 单元中与 `RuntimeDirectory` 相关的部分进行轻微优化。
* [Issue #51](https://github.com/manticoresoftware/executor/issues/51) 添加了 rdkafka 支持并升级至 PHP 8.3.3。
* [Issue #527](https://github.com/manticoresoftware/manticoresearch/issues/527) 支持 [附加](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Attaching_one_table_to_another.md#Attaching-one-table-to-another) RT 表。新增命令 [ALTER TABLE ... RENAME](Updating_table_schema_and_settings.md#Renaming-a-real-time-table)。

### 破坏性更改与废弃事项
* ⚠️[Issue #1436](https://github.com/manticoresoftware/manticoresearch/issues/1436) 修复了 IDF 计算问题。`local_df` 现为默认设置。改进了主从搜索协议（版本已更新）。如果你在分布式环境中运行 Manticore Search，且有多个实例，请确保先升级代理，再升级主服务器。
* ⚠️[Issue #1572](https://github.com/manticoresoftware/manticoresearch/issues/1572) 增加了分布式表的复制并更新了复制协议。如果你运行复制集群，需要：
  - 首先，干净利落地停止所有节点
  - 然后，使用 Linux 下的工具 `manticore_new_cluster`，用 `--new-cluster` 参数启动最后停止的节点。
  - 详情请参阅[重启集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster)。
* ⚠️[问题 #1763](https://github.com/manticoresoftware/manticoresearch/pull/1763) HTTP API 端点别名 `/json/*` 已被废弃。
* ⚠️[问题 #1982](https://github.com/manticoresoftware/manticoresearch/issues/1982) 在 JSON 中将 [profile](Node_info_and_management/Profiling/Query_profile.md#Query-profile) 改为 [plan](Node_info_and_management/Profiling/Query_plan.md#Query-plan)，新增了 JSON 查询分析功能。
* ⚠️[提交 e235](https://github.com/manticoresoftware/manticoresearch-backup/commit/e23585a) manticore-backup 不再备份 `plugin_dir`。
* ⚠️[问题 #171](https://github.com/manticoresoftware/manticoresearch-buddy/issues/155) 将 Buddy 迁移到 Swoole 以提升性能和稳定性。切换到新版本时，请确保所有 Manticore 包均已更新。
* ⚠️[问题 #196](https://github.com/manticoresoftware/manticoresearch-buddy/pull/205) 所有核心插件已合并入 Buddy，并更改了核心逻辑。
* ⚠️[问题 #2107](https://github.com/manticoresoftware/manticoresearch/issues/2107) 在 `/search` 响应中将文档 ID 作为数字处理。
* ⚠️[问题 #38](https://github.com/manticoresoftware/executor/issues/38) 添加了 Swoole，禁用了 ZTS，并移除了 parallel 扩展。
* ⚠️[问题 #1929](https://github.com/manticoresoftware/manticoresearch/issues/1929) 在某些情况下 `charset_table` 的覆盖不起作用。

### 复制相关更改
* [提交 3376](https://github.com/manticoresoftware/manticoresearch/commit/3376351f73db3ff0cb6cb708a304a7bd83e69cfb) 修复了大型文件 SST 时的复制错误。
* [提交 6d36](https://github.com/manticoresoftware/manticoresearch/commit/6d36c68fb7ce3cb274b467aac435788905084952) 为复制命令添加重试机制；修复了网络拥堵且丢包时复制加入失败的问题。
* [提交 842e](https://github.com/manticoresoftware/manticoresearch/commit/842e27e71790833ab4125d49103d2933d5df60c3) 将复制中的 FATAL 消息改为 WARNING 消息。
* [提交 8c32](https://github.com/manticoresoftware/manticoresearch/commit/8c32bce7db39d8ed3b1a06aaa1d2c3ec372084f0) 修正了无表或空表复制集群中 `gcache.page_size` 的计算；同时修复了 Galera 选项的保存和加载。
* [提交 a2af](https://github.com/manticoresoftware/manticoresearch/commit/a2af06ca36abcbf893e43c8803853130b397c54b) 添加了在加入集群节点上跳过更新节点复制命令的功能。
* [提交 c054](https://github.com/manticoresoftware/manticoresearch/commit/c0541e6892f5116b1f957b40060ba8ecd01a5afb) 修复了复制时更新 blob 属性与替换文档之间的死锁。
* [提交 e80d](https://github.com/manticoresoftware/manticoresearch/commit/e80d505b963999016033613c40c1cf3151d648e7) 新增了 [replication_connect_timeout](Server_settings/Searchd.md#replication_connect_timeout), [replication_query_timeout](Server_settings/Searchd.md#replication_query_timeout), [replication_retry_delay](Server_settings/Searchd.md#replication_retry_delay), [replication_retry_count](Server_settings/Searchd.md#replication_retry_count) searchd 配置项，以类似于 `searchd.agent_*` 控制复制期间的网络，但默认值不同。
* [问题 #1356](https://github.com/manticoresoftware/manticoresearch/issues/1356) 修复了部分节点丢失且节点名称解析失败时复制节点重试的问题。
* [问题 #1445](https://github.com/manticoresoftware/manticoresearch/issues/1445) 修复了 `show variables` 中复制日志详细级别的问题。
* [问题 #1482](https://github.com/manticoresoftware/manticoresearch/issues/1482) 修复了 Kubernetes 中 pod 重启导致节点加入集群的复制问题。
* [问题 #1962](https://github.com/manticoresoftware/manticoresearch/issues/1962) 修复了无效节点名导致空集群上复制 Alter 命令长时间等待的问题。

### Bug 修复
* [提交 8a48](https://github.com/manticoresoftware/manticoresearch/commit/8a486c7ba1a293879cdecc41fb52eb7ab0cbf832) 修复了 `count distinct` 中未使用匹配项清理可能引发崩溃的问题。
* [问题 #1569](https://github.com/manticoresoftware/manticoresearch/issues/1569) 二进制日志现以事务为粒度写入。
* [问题 #2089](https://github.com/manticoresoftware/manticoresearch/issues/2089) 修复了与 64 位 ID 相关的错误，避免通过 MySQL 插入时出现“格式错误的数据包”错误，防止[表损坏和重复 ID](https://github.com/manticoresoftware/manticoresearch/issues/2090)。
* [问题 #2160](https://github.com/manticoresoftware/manticoresearch/issues/2160) 修正了日期插入时应使用本地时区而非 UTC 的问题。
* [问题 #2177](https://github.com/manticoresoftware/manticoresearch/issues/2177) 修复了在实时表中进行搜索时，如果 `index_token_filter` 非空会导致崩溃的问题。
* [问题 #2209](https://github.com/manticoresoftware/manticoresearch/issues/2209) 修改了 RT 列存存储中的重复过滤，修复了崩溃和错误查询结果。
* [提交 001d](https://github.com/manticoresoftware/manticoresearch/commit/001d82904e4b887c49387308291aeefb07c0fbc4) 修复了 html stripper 处理连接字段后导致内存损坏的问题。
* [提交 00eb](https://github.com/manticoresoftware/manticoresearch/commit/00eb2ebf) 避免 flush 后回绕流以防止与 mysqldump 通信问题。
* [提交 0553](https://github.com/manticoresoftware/manticoresearch/commit/05534114acf08c70ffcc41a23d558dec09a34373) 如果 preread 尚未开始，则不等待其完成。
* [Commit 055a](https://github.com/manticoresoftware/manticoresearch/commit/055aac09f877570cd29b6594cb283182eba7f73b) 修复了大规模 Buddy 输出字符串在 searchd 日志中多行拆分的问题。
* [Commit 0a88](https://github.com/manticoresoftware/manticoresearch/commit/0a88056307f82c50fcdc458ad6dbc307a00e3f92) 将关于失败头部 `debugv` 详细级别的 MySQL 接口警告移至适当位置。
* [Commit 150a](https://github.com/manticoresoftware/manticoresearch/commit/150a20286b7d98d24b844ae577d7fda4a444798d) 修正了多集群管理操作中的竞态条件；禁止创建名称或路径相同的多个集群。
* [Commit 2e40](https://github.com/manticoresoftware/manticoresearch/commit/2e405c1c9ebac6f46206e3df17ac1c19129c9aa3) 修复全文查询中的隐式截止；将 MatchExtended 分拆为模板部分D。
* [Commit 75f5](https://github.com/manticoresoftware/manticoresearch/commit/75f51421542a2a0658f9e2f9f31048c8c5224479) 修正了索引和加载表到守护进程时 `index_exact_words` 的不一致。
* [Commit 7643](https://github.com/manticoresoftware/manticoresearch/commit/7643976bea142e08535d701000eff0d09a99fda2) 修复删除无效集群时遗漏的错误信息。
* [Commit 7a03](https://github.com/manticoresoftware/manticoresearch/commit/7a034ac990d5ec9344320c05a6a2315583b9b858) 修正 CBO 与队列联合的冲突；修正 CBO 与 RT 伪分片。
* [Commit 7b4e](https://github.com/manticoresoftware/manticoresearch/commit/7b4ea95e) 在未加载辅助索引 (SI) 库及配置中无相关参数时，避免误导性的警告信息“WARNING: secondary_indexes set but failed to initialize secondary library”。
* [Commit 8496](https://github.com/manticoresoftware/manticoresearch/commit/8496317f099fade8d2db463fb88483e7fcdfe85b) 修正 quorum 中的命中排序。
* [Commit 8973](https://github.com/manticoresoftware/manticoresearch-buddy/commit/8973ad3) 修复 ModifyTable 插件中大写选项的问题。
* [Commit 9935](https://github.com/manticoresoftware/manticoresearch-buddy/commit/99350d4c184379b19f3efa9751d9b5be5e6edfb5) 修复从包含空 JSON 值（表示为 NULL）的转储恢复的问题。
* [Commit a28f](https://github.com/manticoresoftware/manticoresearch/commit/a28f6e087b3d957e48816070be17d0823904936a) 通过使用 pcon 修复 Joiner 节点接收 SST 时的 SST 超时。
* [Commit b5a5](https://github.com/manticoresoftware/manticoresearch/commit/b5a55cc26debcddcda5d9caef8366a707e0dd763) 修复选择别名字符串属性时的崩溃。
* [Commit c556](https://github.com/manticoresoftware/manticoresearch/commit/c556e7ed8b6da8eac8ef094650df5f0a8977916b) 增加了对带有 `morphology_skip_fields` 字段的全文查询中将词项转换为 `=term` 的查询转换。
* [Commit cdc3](https://github.com/manticoresoftware/manticoresearch/commit/cdc38085145cd1025b96fcac2a46188a0425d9e1) 添加了缺失的配置键（skiplist_cache_size）。
* [Commit cf6e](https://github.com/manticoresoftware/manticoresearch/commit/cf6ec3bfdc3f0990fb31ff53822b2331a6667727) 修复表达式排名器在处理大型复杂查询时的崩溃。
* [Commit e513](https://github.com/manticoresoftware/manticoresearch/commit/e5139ba75eef7223df375ae3cc3f138fa7d925f1) 修复全文 CBO 对无效索引提示的处理。
* [Commit eb05](https://github.com/manticoresoftware/manticoresearch/commit/eb0514b3a6f57f278252b0724eebd112ec116e0f) 在关闭时中断预读以加快关闭速度。
* [Commit f945](https://github.com/manticoresoftware/manticoresearch/commit/f94555a293cfa51004ff81135b8bd8500b955115) 更改全文查询的堆栈计算，避免复杂查询时崩溃。
* [Issue #1262](https://github.com/manticoresoftware/manticoresearch/issues/1262) 修复索引器在索引含有多个同名列的 SQL 源时崩溃。
* [Issue #1273](https://github.com/manticoresoftware/manticoresearch/issues/1273) 对不存在的系统变量返回 0 而非 <empty>。
* [Issue #1289](https://github.com/manticoresoftware/manticoresearch/issues/1289) 修复 indextool 检查 RT 表外部文件时的错误。
* [Issue #1335](https://github.com/manticoresoftware/manticoresearch/issues/1335) 修复多词词形变化在短语内导致的查询解析错误。
* [Issue #1364](https://github.com/manticoresoftware/manticoresearch/issues/1364) 添加旧版 binlog 的空 binlog 文件重放支持。
* [Issue #1365](https://github.com/manticoresoftware/manticoresearch/issues/1365) 修复最后一个空 binlog 文件删除问题。
* [Issue #1372](https://github.com/manticoresoftware/manticoresearch/issues/1372) 修复因更改 `data_dir` 影响守护进程启动时当前工作目录，导致相对路径错误（被转为守护进程启动目录的绝对路径）。
* [Issue #1393](https://github.com/manticoresoftware/manticoresearch/issues/1393) hn_small 中最慢时间降级：在守护进程启动时获取/缓存 CPU 信息。
* [Issue #1395](https://github.com/manticoresoftware/manticoresearch/issues/1395) 修复索引加载期间缺失外部文件的警告。
* [Issue #1402](https://github.com/manticoresoftware/manticoresearch/issues/1402) 修复全局分组器在释放数据指针属性时崩溃。
* [Issue #1403](https://github.com/manticoresoftware/manticoresearch/issues/1403) _ADDITIONAL_SEARCHD_PARAMS 无效的问题。
* [Issue #1427](https://github.com/manticoresoftware/manticoresearch/issues/1427) 修正每表 `agent_query_timeout` 被默认查询选项 `agent_query_timeout` 覆盖的问题。
* [Issue #1444](https://github.com/manticoresoftware/manticoresearch/issues/1444) 修复使用支持多匹配值的 `packedfactors()` 时分组器和排名器的崩溃。
* [Issue #1458](https://github.com/manticoresoftware/manticoresearch/issues/1458) 修复频繁索引更新时 Manticore 崩溃。
* [Issue #1481](https://github.com/manticoresoftware/manticoresearch/issues/1481) 修复解析错误后清理解析查询时的崩溃。
* [Issue #1484](https://github.com/manticoresoftware/manticoresearch/issues/1484) 修复了 HTTP JSON 请求未路由到 buddy 的问题。
* [Issue #1499](https://github.com/manticoresoftware/manticoresearch/issues/1499) JSON 属性根值不能是数组。已修复。
* [Issue #1507](https://github.com/manticoresoftware/manticoresearch/issues/1507) 修复了事务内表重建时崩溃的问题。
* [Issue #1515](https://github.com/manticoresoftware/manticoresearch/issues/1515) 修正了俄语词根短格式的扩展。
* [Issue #1579](https://github.com/manticoresoftware/manticoresearch/issues/1579) 修正了 [date_format](Functions/Date_and_time_functions.md#DATE_FORMAT()) 表达式中 JSON 和 STRING 属性的用法。
* [Issue #1580](https://github.com/manticoresoftware/manticoresearch/issues/1580) 修正了多别名分组到 JSON 字段的 grouper。
* [Issue #1594](https://github.com/manticoresoftware/manticoresearch/issues/1594) dev 中 total_related 错误：修正了隐式 cutoff 与 limit 的问题；在 json 查询中添加了更好的全扫描检测。
* [Issue #1603](https://github.com/manticoresoftware/manticoresearch/issues/1603) 修正了所有日期表达式中 JSON 和 STRING 属性的用法。
* [Issue #1609](https://github.com/manticoresoftware/manticoresearch/issues/1609) 使用 LEVENSHTEIN() 时崩溃。
* [Issue #1612](https://github.com/manticoresoftware/manticoresearch/issues/1612) 修复了带高亮的搜索查询解析错误后的内存损坏。
* [Issue #1614](https://github.com/manticoresoftware/manticoresearch/issues/1614) 禁用了对短于 `min_prefix_len`/`min_infix_len` 的词的通配符扩展。
* [Issue #1617](https://github.com/manticoresoftware/manticoresearch/issues/1617) 修改行为，如果 Buddy 成功处理请求，则不记录错误。
* [Issue #1635](https://github.com/manticoresoftware/manticoresearch/issues/1635) 修正了在设置 limit 的查询中搜索查询元数据中的 total。
* [Issue #1640](https://github.com/manticoresoftware/manticoresearch/issues/1640) 无法通过 JSON 以纯文本模式使用大写表名。
* [Issue #1643](https://github.com/manticoresoftware/manticoresearch/issues/1643) 提供了默认的 `SPH_EXTNODE_STACK_SIZE` 值。
* [Issue #1646](https://github.com/manticoresoftware/manticoresearch/issues/1646) 修正了 SphinxQL 日志负过滤器在 MVA 属性上使用 ALL/ANY 的问题。
* [Issue #1660](https://github.com/manticoresoftware/manticoresearch/issues/1660) 修正了来自其他索引的 docid killlists 的应用。❤️ 感谢 [@raxoft](https://github.com/raxoft) 提交的 PR。
* [Issue #1668](https://github.com/manticoresoftware/manticoresearch/issues/1668) 修正了原始索引全扫描（无任何索引迭代器）中因提前退出导致的匹配遗漏；移除了 plain 行迭代器中的 cutoff。
* [Issue #1671](https://github.com/manticoresoftware/manticoresearch/issues/1671) 修正了在使用代理并查询分布式表时 `FACET` 错误。
* [Issue #1690](https://github.com/manticoresoftware/manticoresearch/issues/1690) 修正了大数值直方图估计时的崩溃。
* [Issue #1692](https://github.com/manticoresoftware/manticoresearch/issues/1692) alter table tbl add column col uint 时崩溃。
* [Issue #1710](https://github.com/manticoresoftware/manticoresearch/issues/1710) 条件 `WHERE json.array IN (<value>)` 返回空结果。
* [Issue #172](https://github.com/manticoresoftware/manticoresearch-buddy/issues/172) 修正了发送请求到 `/cli` 时 TableFormatter 的一个问题。
* [Issue #1742](https://github.com/manticoresoftware/manticoresearch/issues/1742) `CREATE TABLE` 在缺少 wordforms 文件时未失败。
* [Issue #1762](https://github.com/manticoresoftware/manticoresearch/issues/1762) RT 表中属性的顺序现在遵循配置顺序。
* [Issue #1765](https://github.com/manticoresoftware/manticoresearch/issues/1765) HTTP bool 查询包含 'should' 条件返回错误结果。
* [Issue #1769](https://github.com/manticoresoftware/manticoresearch/issues/1769) 按字符串属性排序时，`SPH_SORT_ATTR_DESC` 和 `SPH_SORT_ATTR_ASC` 不生效。
* [Issue #177](https://github.com/manticoresoftware/manticoresearch-buddy/issues/177) 禁用了 curl 请求到 Buddy 时的 `Expect: 100-continue` HTTP 头。
* [Issue #1791](https://github.com/manticoresoftware/manticoresearch/issues/1791) 由 GROUP BY 别名引起的崩溃。
* [Issue #1792](https://github.com/manticoresoftware/manticoresearch/issues/1792) Windows 上 SQL 元数据摘要显示错误时间。
* [Issue #1794](https://github.com/manticoresoftware/manticoresearch/issues/1794) 修正了 JSON 查询中单词性能下降的问题。
* [Issue #1798](https://github.com/manticoresoftware/manticoresearch/issues/1798) 不兼容的过滤器在 `/search` 上未引发错误。
* [Issue #1802](https://github.com/manticoresoftware/manticoresearch/issues/1802) 修正了 `ALTER CLUSTER ADD` 和 `JOIN CLUSTER` 操作，令其相互等待，防止了在捐赠者向连接者节点发送表时，`ALTER` 向集群添加表的竞争条件。
* [Issue #1811](https://github.com/manticoresoftware/manticoresearch/issues/1811) 错误处理了 `/pq/{table}/*` 请求。
* [Issue #1816](https://github.com/manticoresoftware/manticoresearch/issues/1816) `UNFREEZE` 在某些情况下不起作用。
* [Issue #183](https://github.com/manticoresoftware/manticoresearch-buddy/pull/183) 修正了某些情况下 MVA 恢复的问题。
* [Issue #1849](https://github.com/manticoresoftware/manticoresearch/issues/1849) 修正了使用 MCL 时 indextool 关闭时崩溃问题。
* [Issue #1866](https://github.com/manticoresoftware/manticoresearch/issues/1866) 修正了 `/cli_json` 请求时不必要的 URL 解码。
* [Issue #1872](https://github.com/manticoresoftware/manticoresearch/issues/1872) 守护进程启动时更改 plugin_dir 设置逻辑。
* [Issue #1874](https://github.com/manticoresoftware/manticoresearch/issues/1874) alter table ... 异常失败。
* [Issue #1891](https://github.com/manticoresoftware/manticoresearch/issues/1891#issuecomment-2051941200) Manticore 在插入数据时因 `signal 11` 崩溃。
* [Issue #1920](https://github.com/manticoresoftware/manticoresearch/issues/1920) 降低了 [low_priority](Searching/Options.md#low_priority) 的节流。
* [Issue #1924](https://github.com/manticoresoftware/manticoresearch/issues/1924) Mysqldump + mysql 恢复的错误。
* [Issue #1951](https://github.com/manticoresoftware/manticoresearch/issues/1951) 修正了在本地表缺失或代理描述错误的情况下分布式表的错误创建；现在会返回错误信息。
* [Issue #1972](https://github.com/manticoresoftware/manticoresearch/issues/1972) 实现了一个 `FREEZE` 计数器以避免冻结/解冻问题。
* [Issue #1980](https://github.com/manticoresoftware/manticoresearch/issues/1980) 在 OR 节点中遵守查询超时。此前 `max_query_time` 在某些情况下可能不起作用。
* [Issue #1986](https://github.com/manticoresoftware/manticoresearch/issues/1986) 重命名 new 为当前 [manticore.json] 失败。
* [Issue #1988](https://github.com/manticoresoftware/manticoresearch/issues/1988) 全文查询可能会忽略 `SecondaryIndex` 的 CBO 提示。
* [Issue #1990](https://github.com/manticoresoftware/manticoresearch/issues/1990) 修正 `expansion_limit` 以切片多个磁盘块或内存块中调用关键字的最终结果集。
* [Issue #1994](https://github.com/manticoresoftware/manticoresearch/issues/1994) 错误的外部文件。
* [Issue #2021](https://github.com/manticoresoftware/manticoresearch/issues/2021) 停止 Manticore 后，可能会留下几个 manticore-executor 进程仍在运行。
* [Issue #2029](https://github.com/manticoresoftware/manticoresearch/issues/2029) 使用 Levenshtein Distance 时崩溃。
* [Issue #2037](https://github.com/manticoresoftware/manticoresearch/issues/2037) 在空索引上多次运行 max 操作符后出现错误。
* [Issue #2052](https://github.com/manticoresoftware/manticoresearch/issues/2052) 多字段分组时带 JSON.field 崩溃。
* [Issue #2067](https://github.com/manticoresoftware/manticoresearch/issues/2067) 对 _update 的错误请求导致 Manticore 崩溃。
* [Issue #2069](https://github.com/manticoresoftware/manticoresearch/issues/2069) 修复 JSON 接口中字符串过滤比较器处理封闭区间的问题。
* [Issue #2082](https://github.com/manticoresoftware/manticoresearch/issues/2082) 当 data_dir 路径位于符号链接上时，`alter` 操作失败。
* [Issue #2102](https://github.com/manticoresoftware/manticoresearch/pull/2102) 改进 mysqldump 中对 SELECT 查询的特殊处理，确保生成的 INSERT 语句兼容 Manticore。
* [Issue #2103](https://github.com/manticoresoftware/manticoresearch/issues/2103) 泰文字符的字符集错误。
* [Issue #2124](https://github.com/manticoresoftware/manticoresearch/issues/2124) 使用保留字的 SQL 导致崩溃。
* [Issue #2154](https://github.com/manticoresoftware/manticoresearch/issues/2154) 含词形转换表的表无法导入。
* [Issue #2176](https://github.com/manticoresoftware/manticoresearch/issues/2176) 修复在 engine 参数设置为 'columnar' 且通过 JSON 添加重复 ID 时的崩溃。
* [Issue #223](https://github.com/manticoresoftware/manticoresearch-buddy/issues/223) 在尝试插入没有模式且没有列名的文档时，返回正确错误。
* [Issue #239](https://github.com/manticoresoftware/manticoresearch-buddy/issues/239) 自动模式的多行插入可能失败。
* [Issue #399](https://github.com/manticoresoftware/manticoresearch/issues/399) 如果在数据源声明了 id 属性，索引时增加错误消息。
* [Issue #59](https://github.com/manticoresoftware/manticoresearch-helm/issues/59) Manticore 集群崩溃。
* [Issue #68](https://github.com/manticoresoftware/manticoresearch-helm/issues/68) 如果存在 percolate 表，optimize.php 会崩溃。
* [Issue #77](https://github.com/manticoresoftware/manticoresearch-helm/issues/77) 修复 Kubernetes 部署时的错误。
* [Issue #274](https://github.com/manticoresoftware/manticoresearch-buddy/issues/274) 修复并发请求 Buddy 时处理错误。

### 与 manticore-backup 相关
* [Issue #97](https://github.com/manticoresoftware/manticoresearch-backup/pull/97) 在可用时将 VIP HTTP 端口设置为默认。
各种改进：改进版本检查和流式 ZSTD 解压；在恢复过程中添加用户提示以应对版本不匹配；修复恢复时不同版本的错误提示行为；增强了解压逻辑，支持直接从流读取而非加载到工作内存；增加了 `--force` 标志。
* [Commit 3b35](https://github.com/manticoresoftware/manticoresearch-backup/commit/3b357ea) 在启动 Manticore 搜索后显示备份版本，方便识别此阶段的问题。
* [Commit ad2e](https://github.com/manticoresoftware/manticoresearch-backup/commit/ad2e5da) 更新连接守护进程失败时的错误信息。
* [Commit ce5e](https://github.com/manticoresoftware/manticoresearch-backup/commit/ce5ea85) 修复将绝对根备份路径转换为相对路径的问题，并移除了恢复时的可写检查以支持从不同路径恢复。
* [Commit db7e](https://github.com/manticoresoftware/manticoresearch-backup/commit/db7e2b9) 为文件迭代器添加排序功能，确保各种情况下的一致性。
* [Issue #106](https://github.com/manticoresoftware/manticoresearch-backup/issues/105) 多配置的备份和恢复。
* [Issue #91](https://github.com/manticoresoftware/manticoresearch-backup/pull/91) 增加 defattr 防止安装后文件出现非正常用户权限（RHEL）。
* [Issue #91](https://github.com/manticoresoftware/manticoresearch-backup/pull/91) 额外增加 chown，确保文件在 Ubuntu 下默认归属 root 用户。

### 与 MCL（列式，二级索引，KNN 库）相关
* [提交 f104](https://github.com/manticoresoftware/columnar/commit/f104a4709bbb144140d09a09bc0fc1db1895e331) 向量搜索支持。
* [提交 2169](https://github.com/manticoresoftware/columnar/commit/216953beef6676f7e34c9f6f1a30c5980d746710) 修复了在中断建立二级索引时临时文件的清理问题。解决了守护进程在创建 `tmp.spidx` 文件时超出打开文件限制的问题。
* [提交 709b](https://github.com/manticoresoftware/columnar/commit/709b9acaaac97d9a1ca8796892f9ad432021c785) 为 columnar 和 SI 使用独立的 streamvbyte 库。
* [提交 1c26](https://github.com/manticoresoftware/manticoresearch/commit/1c2623763ced14bb94ad624dde9614a938408d89) 添加了一个警告，说明列存储不支持 json 属性。
* [提交 3acd](https://github.com/manticoresoftware/columnar/commit/3acd6714ffce103447d7931dfc75bb1a7449f340) 修复了 SI 中的数据解包问题。
* [提交 574c](https://github.com/manticoresoftware/manticoresearch/commit/574c023152564fa72092c18ebd1f594abfd4b6d7) 修复了保存包含混合行存储和列存储的磁盘块时的崩溃。
* [提交 e87f](https://github.com/manticoresoftware/columnar/commit/e87f088b458d4c5b1b6159fc72ed0d57b786891a) 修复了 SI 迭代器提示已处理块的错误。
* [问题 #1474](https://github.com/manticoresoftware/manticoresearch/issues/1474) 使用列式引擎时行存储 MVA 列的更新失败。
* [问题 #1510](https://github.com/manticoresoftware/manticoresearch/issues/1510) 修复了在 `HAVING` 中使用列式属性聚合时的崩溃。
* [问题 #1519](https://github.com/manticoresoftware/manticoresearch/issues/1519) 修复了在 `expr` 排名器中使用列式属性时的崩溃。

### 相关 Docker
* ❗[问题 #42](https://github.com/manticoresoftware/docker/issues/42) 通过环境变量支持[纯索引](https://github.com/manticoresoftware/docker#building-plain-tables)。
* ❗[问题 #47](https://github.com/manticoresoftware/docker/issues/47) 通过环境变量提升配置灵活性。
* [问题 #54](https://github.com/manticoresoftware/docker/pull/54) 改进了 Docker 的[备份和恢复](https://github.com/manticoresoftware/docker#backup-and-restore)流程。
* [问题 #77](https://github.com/manticoresoftware/docker/issues/76) 改进入口点，仅在首次启动时处理备份恢复。
* [提交 a27c](https://github.com/manticoresoftware/docker/commit/a27c048dd9e73d8d6bea8d9a4830cafb486b82c5) 修复查询日志输出到 stdout。
* [问题 #38](https://github.com/manticoresoftware/docker/issues/38) 如果未设置 EXTRA，则静默 BUDDY 警告。
* [问题 #71](https://github.com/manticoresoftware/docker/pull/71) 修复 `manticore.conf.sh` 中的主机名。

## 版本 6.2.12
发布日期：2023 年 8 月 23 日

版本 6.2.12 继续 6.2 系列，解决了 6.2.0 发布后发现的问题。

### Bug 修复
* ❗[问题 #1351](https://github.com/manticoresoftware/manticoresearch/issues/1351) “Manticore 6.2.0 无法通过 systemctl 在 Centos 7 上启动”：将 `TimeoutStartSec` 从 `infinity` 修改为 `0`，以提高对 Centos 7 的兼容性。
* ❗[问题 #1364](https://github.com/manticoresoftware/manticoresearch/issues/1364) “从 6.0.4 升级到 6.2.0 后崩溃”：为旧版 binlog 版本的空 binlog 文件添加重放功能。
* [PR #1334](https://github.com/manticoresoftware/manticoresearch/pull/1334) “修正 searchdreplication.cpp 中的拼写错误”：修正了 `searchdreplication.cpp` 中的拼写错误：beggining -> beginning。
* [问题 #1337](https://github.com/manticoresoftware/manticoresearch/issues/1337) “Manticore 6.2.0 WARNING: conn (local)(12), sock=8088: 放弃失败的 MySQL 头部，AsyncNetInputBuffer_c::AppendData: 错误 11 (资源暂时不可用) 返回 -1”：将 MySQL 接口关于头部的警告的详细级别降低至 logdebugv。
* [问题 #1355](https://github.com/manticoresoftware/manticoresearch/issues/1355) “当 node_address 无法解析时 join 集群挂起”：改进了在某些节点无法访问且名称解析失败时的复制重试。此修复应解决 Kubernetes 和 Docker 节点中与复制相关的问题。增强了复制启动失败的错误消息，并更新了测试模型 376。此外，提供了名称解析失败的清晰错误消息。
* [问题 #1361](https://github.com/manticoresoftware/manticoresearch/issues/1361) “charset non_cjk 中缺少 'Ø' 的小写映射”：调整了字符 'Ø' 的映射。
* [问题 #1365](https://github.com/manticoresoftware/manticoresearch/issues/1365) “searchd 在干净停止后仍保留 binlog.meta 和 binlog.001 文件”：确保正确删除最后一个空 binlog 文件。
* [提交 0871](https://github.com/manticoresoftware/manticoresearch/commit/0871070845e93ad702ae0dfb89b35e5f24cd0851)：修复了 Windows 上与原子复制限制相关的 `Thd_t` 构建问题。
* [提交 1cc0](https://github.com/manticoresoftware/manticoresearch/commit/1cc04bedc174737118fd5a3663e35f4e95bd4f8c)：解决了 FT CBO 与 `ColumnarScan` 的问题。
* [提交 c6bf](https://github.com/manticoresoftware/manticoresearch/commit/c6bfb30d53e3f4b67f17769667733677a661c95b)：修正了测试 376 并为测试中的 `AF_INET` 错误添加了替代方案。
* [提交 cbc3](https://github.com/manticoresoftware/manticoresearch/commit/cbc38705e0b4dcc9665e3e8cf64826290ebd11cf)：解决了复制过程中更新 blob 属性与替换文档之间的死锁问题。同时移除了提交时索引的读锁，因为它在更基础层次已经被锁定。

### 次要更改
* [提交 4f91](https://github.com/manticoresoftware/manticoresearch/commit/4f913a36e84e01f950b2797051222b0b1094d6fe) 更新了手册中 `/bulk` 端点的信息。

### MCL
* 支持 [Manticore Columnar Library](https://github.com/manticoresoftware/columnar) v2.2.4

## 版本 6.2.0
发布日期：2023 年 8 月 4 日

### 重大更改
* 查询优化器已增强，以支持全文查询，显著提高了搜索效率和性能。
* 集成了：
  - [mysqldump](https://dev.mysql.com/doc/refman/8.0/en/mysqldump.html) - 使用 `mysqldump` [进行逻辑备份](Securing_and_compacting_a_table/Backup_and_restore.md#Backup-and-restore-with-mysqldump)
  - [Apache Superset](https://manticoresearch.com/blog/manticoresearch-apache-superset-integration/) 和 [Grafana](https://manticoresearch.com/blog/manticoresearch-grafana-integration/) 以可视化存储在 Manticore 中的数据
  - [HeidiSQL](https://www.heidisql.com/) 和 [DBForge](https://www.devart.com/dbforge/) 以便更轻松地进行 Manticore 开发
* 我们已开始使用 [GitHub workflows](https://github.com/manticoresoftware/manticoresearch/actions)，使贡献者更容易使用核心团队在准备包时采用的相同持续集成（CI）流程。所有作业均可在 GitHub 托管的运行器上运行，从而便于在您的 Manticore Search 分支中无缝测试更改。
* 我们已开始使用 [CLT](https://github.com/manticoresoftware/clt) 来测试复杂场景。例如，我们现在可以确保提交后构建的包可以在所有受支持的 Linux 操作系统上正确安装。命令行测试器（CLT）提供了一种用户友好方式，以交互模式记录测试并轻松重放它们。
* 通过结合哈希表和 HyperLogLog，count distinct 操作性能显著提升。
* 启用了包含二级索引查询的多线程执行，线程数限制为物理 CPU 核心数。这应大幅提升查询执行速度。
* `pseudo_sharding` 已调整为限制在空闲线程数以内。此更新显著提升了吞吐量性能。
* 用户现在可以通过配置设置指定 [默认属性存储引擎](Server_settings/Searchd.md#engine)，以更好地根据特定工作负载需求进行定制。
* 支持 [Manticore Columnar Library 2.2.0](https://github.com/manticoresoftware/columnar/)，在[二级索引](Server_settings/Searchd.md#secondary_indexes)中进行了众多错误修复和改进。

### 次要更改
* [Buddy #153](https://github.com/manticoresoftware/manticoresearch-buddy/issues/153)：[/pq](Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md?client=JSON#Adding-rules-to-a-percolate-table) HTTP 端点现在作为 `/json/pq` HTTP 端点的别名。
* [Commit 0bf1](https://github.com/manticoresoftware/manticoresearch/commit/0bf17d9e)：我们确保了 `upper()` 和 `lower()` 的多字节兼容性。
* [Commit 2bb9](https://github.com/manticoresoftware/manticoresearch/commit/2bb92765)：对于 `count(*)` 查询，现在返回预先计算的值，而不是扫描索引。
* [Commit 3c84](https://github.com/manticoresoftware/manticoresearch/commit/3c84091f)：现在可以使用 `SELECT` 进行任意计算并显示 `@@sysvars`。与以前不同，您不再局限于单一计算。因此，诸如 `select user(), database(), @@version_comment, version(), 1+1 as a limit 10` 这样的查询将返回所有列。请注意，可选的 'limit' 将始终被忽略。
* [Commit 6aca](https://github.com/manticoresoftware/manticoresearch/commit/6aca32f0)：实现了 `CREATE DATABASE` 伪查询。
* [Commit 9dc1](https://github.com/manticoresoftware/manticoresearch/commit/9dc12334)：执行 `ALTER TABLE table REBUILD SECONDARY` 时，即使未更新属性，也总是重新构建二级索引。
* [Commit 46ed](https://github.com/manticoresoftware/manticoresearch/commit/46edb089)：使用预计算数据的排序器现在会先被识别，从而避免不必要的 CBO 计算。
* [Commit 102a](https://github.com/manticoresoftware/manticoresearch/commit/102ac604)：实现了全文表达式堆栈的模拟和利用，防止守护进程崩溃。
* [Commit 979f](https://github.com/manticoresoftware/manticoresearch/commit/979fa27c)：为不使用字符串/mvas/json 属性的匹配添加了快速代码路径。
* [Commit a073](https://github.com/manticoresoftware/manticoresearch/commit/a0735ffe)：添加了对 `SELECT DATABASE()` 命令的支持。但该命令始终返回 `Manticore`。此新增功能对于与各种 MySQL 工具的集成至关重要。
* [Commit bc04](https://github.com/manticoresoftware/manticoresearch/commit/bc04908)：修改了 [/cli](Connecting_to_the_server/HTTP.md#/cli) 端点的响应格式，并新增了 `/cli_json` 端点，功能相当于之前的 `/cli`。
* [Commit d70b](https://github.com/manticoresoftware/manticoresearch/commit/d70b0d58)：`thread_stack` 现在可通过 `SET` 语句在运行时更改。提供会话本地和守护进程全局两种变体。当前值可在 `show variables` 输出中访问。
* [Commit d96e](https://github.com/manticoresoftware/manticoresearch/commit/d96ec6b9)：代码已集成到 CBO 中，以更准确地估计在字符串属性上执行过滤器的复杂性。
* [Commit e77d](https://github.com/manticoresoftware/manticoresearch/commit/e77dd72f5a04531c352fad0d7afcd2a1cbae2510)：改进了 DocidIndex 的成本计算，提升了整体性能。
* [Commit f3ae](https://github.com/manticoresoftware/manticoresearch/commit/f3ae8bea)：现在可在 `SHOW STATUS` 命令中查看类似于 Linux “uptime”的负载指标。
* [Commit f3cc](https://github.com/manticoresoftware/manticoresearch/commit/f3cc0971)：`DESC` 和 `SHOW CREATE TABLE` 的字段和属性顺序现与 `SELECT * FROM` 保持一致。
* [Commit f3d2](https://github.com/manticoresoftware/manticoresearch/commit/f3d248a6): 不同的内部解析器现在在各种错误发生时提供它们的内部助记码（例如，`P01`）。此增强功能有助于识别哪个解析器引起了错误，同时也隐藏了非必要的内部细节。
* [Issue #271](https://github.com/manticoresoftware/manticoresearch/issues/271) “有时 CALL SUGGEST 不会建议修正单字母的拼写错误”：改进了针对短词的 [SUGGEST/QSUGGEST](Searching/Spell_correction.md#CALL-QSUGGEST,-CALL-SUGGEST) 行为：新增选项 `sentence` 用于显示完整句子
* [Issue #696](https://github.com/manticoresoftware/manticoresearch/issues/696) “开启词干处理时，Percolate 索引无法通过精确短语查询正确搜索”：Percolate 查询已修改，以处理精确词项修饰符，提升搜索功能。
* [Issue #829](https://github.com/manticoresoftware/manticoresearch/issues/829) “日期格式化方法”：新增了 [date_format()](Functions/Date_and_time_functions.md#DATE_FORMAT()) 选择列表表达式，暴露了 `strftime()` 函数。
* [Issue #961](https://github.com/manticoresoftware/manticoresearch/issues/961) “通过 HTTP JSON API 对聚合桶排序”：在 HTTP 接口中为每个聚合桶引入了可选的 [sort 属性](Searching/Faceted_search.md#HTTP-JSON)。
* [Issue #1062](https://github.com/manticoresoftware/manticoresearch/issues/1062) “改进 JSON 插入 API 失败时的错误日志——‘不支持的值类型’”：在发生错误时，`/bulk` 端点会报告处理和未处理字符串（文档）的数量信息。
* [Issue #1070](https://github.com/manticoresoftware/manticoresearch/issues/1070) “CBO 提示不支持多个属性”：启用了索引提示以支持多个属性。
* [Issue #1106](https://github.com/manticoresoftware/manticoresearch/issues/1106) “为 http 搜索查询添加标签”：向 [HTTP PQ 响应](Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md?client=JSON#Adding-rules-to-a-percolate-table) 添加了标签。
* [Issue #1301](https://github.com/manticoresoftware/manticoresearch/issues/1301) “buddy 不应并行创建表”：解决了因并行执行 CREATE TABLE 操作导致的失败问题。现在，只能同时运行一个 `CREATE TABLE` 操作。
* [Issue #1303](https://github.com/manticoresoftware/manticoresearch/issues/1303) “添加对 @ 符号作为列名的支持”。
* [Issue #1316](https://github.com/manticoresoftware/manticoresearch/issues/1316) “使用 ps=1 时 taxi 数据集查询缓慢”：已改进 CBO 逻辑，并将默认直方图分辨率设为 8k，以提高随机分布属性的准确性。
* [Issue #1317](https://github.com/manticoresoftware/manticoresearch/issues/1317) “修复 hn 数据集中 CBO 与全文索引的冲突”：新增判断何时使用位图迭代器交集及何时使用优先队列的强化逻辑。
* [Issue #1318](https://github.com/manticoresoftware/manticoresearch/issues/1318) “columnar：修改迭代器接口为单次调用”：Columnar 迭代器现在使用单个 `Get` 调用，取代之前的两步调用 `AdvanceTo` + `Get` 来获取值。
* [Issue #1319](https://github.com/manticoresoftware/manticoresearch/issues/1319) “聚合计算加速（移除 CheckReplaceEntry？）”：从分组排序器中移除了 `CheckReplaceEntry` 调用，以加快聚合函数计算。
* [Issue #1320](https://github.com/manticoresoftware/manticoresearch/issues/1320) “create table 中 read_buffer_docs/hits 不支持 k/m/g 语法”：`CREATE TABLE` 选项 `read_buffer_docs` 和 `read_buffer_hits` 现在支持 k/m/g 语法。
* 英文、德文和俄文的[语言包](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology)现在可以通过在 Linux 上执行命令 `apt/yum install manticore-language-packs` 轻松安装。在 macOS 上，使用命令 `brew install manticoresoftware/tap/manticore-language-packs`。
* 字段和属性的顺序现在在 `SHOW CREATE TABLE` 和 `DESC` 操作之间保持一致。
* 当执行 `INSERT` 查询时磁盘空间不足，新 `INSERT` 查询将失败，直到有足够的磁盘空间为止。
* 新增了 [UINT64()](Functions/Type_casting_functions.md#UINT64%28%29) 类型转换函数。
* `/bulk` 端点现在将空行处理为 [commit](Data_creation_and_modification/Transactions.md#BEGIN,-COMMIT,-and-ROLLBACK) 命令。更多信息见[这里](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md?client=JSON#Bulk-adding-documents)。
* 针对[无效的索引提示](Searching/Options.md#Query-optimizer-hints)新增了警告，提供更多透明度并有助于错误缓解。
* 当使用带有单个过滤条件的 `count(*)` 时，查询现在会在有可用的情况下利用二级索引中的预计算数据，从而显著加快查询时间。

### ⚠️ 重大变更
* ⚠️ 在版本 6.2.0 中创建或修改的表格无法被旧版本读取。
* ⚠️ 文档 ID 现在在索引和 INSERT 操作中作为无符号 64 位整数处理。
* ⚠️ 查询优化器提示的语法已更新。新格式为 `/*+ SecondaryIndex(uid) */`。请注意旧语法已不再支持。
* ⚠️ [Issue #1160](https://github.com/manticoresoftware/manticoresearch/issues/1160)：禁止在表名中使用 `@`，以避免语法冲突。
* ⚠️ 被标记为 `indexed` 和 `attribute` 的字符串字段/属性，现在在 `INSERT`、`DESC` 和 `ALTER` 操作中视为单字段处理。
* ⚠️ [Issue #1057](https://github.com/manticoresoftware/manticoresearch/issues/1057)：在不支持 SSE 4.2 的系统上，MCL 库将不再加载。
* ⚠️ [问题 #1143](https://github.com/manticoresoftware/manticoresearch/issues/1143)：[agent_query_timeout](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_query_timeout) 出现故障。已修复，现在生效。

### Bug修复
* [提交 2a6e](https://github.com/manticoresoftware/manticoresearch/commit/2a6ea8f7) “DROP TABLE 时崩溃”：解决了执行 DROP TABLE 语句时导致 RT 表上写入操作（优化、磁盘块保存）完成时间过长的问题。添加了警告以通知在执行 DROP TABLE 命令后表目录非空的情况。
* [提交 2ebd](https://github.com/manticoresoftware/manticoresearch/commit/2ebd424d)：新增了对多属性分组代码中缺失的列式属性支持。
* [提交 3be4](https://github.com/manticoresoftware/manticoresearch/commit/3be4503f752216b37b8cde8e47fd2d77bbce63ae) 解决了可能因磁盘空间耗尽导致崩溃的问题，通过正确处理 binlog 写入错误。
* [提交 6adb](https://github.com/manticoresoftware/manticoresearch/commit/6adb0934)：修复了查询中使用多个列式扫描迭代器（或二级索引迭代器）时偶发的崩溃。
* [提交 6bd9](https://github.com/manticoresoftware/manticoresearch/commit/6bd9f709)：使用基于预计算数据的排序器时未能移除过滤器的问题已修复。
* [提交 6d03](https://github.com/manticoresoftware/manticoresearch/commit/6d03566a)：更新了 CBO 代码，以改善多线程执行基于行属性过滤器的查询的估算。
* [提交 6dd3](https://github.com/manticoresoftware/manticoresearch/commit/6dd3964), [Helm #56](https://github.com/manticoresoftware/manticoresearch-helm/issues/56) “Kubernetes 集群中致命崩溃转储”：修复了 JSON 根对象的 Bloom 过滤器缺陷；修复了因 JSON 字段过滤导致的守护进程崩溃。
* [提交 6e1b](https://github.com/manticoresoftware/manticoresearch/commit/6e1b849) 纠正了因无效 `manticore.json` 配置导致的守护进程崩溃。
* [提交 6fbc](https://github.com/manticoresoftware/manticoresearch/commit/6fbc4189) 修复 JSON 范围过滤器以支持 int64 类型值。
* [提交 9c67](https://github.com/manticoresoftware/manticoresearch/commit/9c677426) `.sph` 文件可能在 `ALTER` 操作中损坏的问题已修复。
* [提交 77cc](https://github.com/manticoresoftware/manticoresearch/commit/77ccd8f)：添加了用于 replace 语句复制的共享密钥，解决了多个主节点复制 replace 时出现的 `pre_commit` 错误。
* [提交 2884](https://github.com/manticoresoftware/manticoresearch/commit/2884519e) 解决了对如 'date_format()' 这类函数进行 bigint 检查的问题。
* [提交 9513](https://github.com/manticoresoftware/manticoresearch/commit/9513aca5)：当排序器利用预计算数据时，[SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META) 中将不再显示迭代器。
* [提交 a2a7](https://github.com/manticoresoftware/manticoresearch/commit/a2a70555)：更新全文节点堆栈大小，以防止复杂全文查询时崩溃。
* [提交 a062](https://github.com/manticoresoftware/manticoresearch/commit/a0626d7e)：解决了使用 JSON 和字符串属性复制更新时导致崩溃的错误。
* [提交 b3e6](https://github.com/manticoresoftware/manticoresearch/commit/b3e63e5d)：更新了字符串构建器，使用 64-bit 整数以避免处理大数据集时崩溃。
* [提交 c472](https://github.com/manticoresoftware/manticoresearch/commit/c472e5b)：修复了跨多个索引使用 count distinct 时发生的崩溃。
* [提交 d073](https://github.com/manticoresoftware/manticoresearch/commit/d0730272)：修复了即使禁用 `pseudo_sharding` ，也可能多线程执行 RT 索引磁盘块查询的问题。
* [提交 d205](https://github.com/manticoresoftware/manticoresearch/commit/d205508e) 修改了 `show index status` 命令返回的值集，现在根据所用索引类型而异。
* [提交 e9bc](https://github.com/manticoresoftware/manticoresearch/commit/e9bc2f9) 修复了处理批量请求时的 HTTP 错误及错误未从网络循环返回给客户端的问题。
* [提交 f77c](https://github.com/manticoresoftware/manticoresearch/commit/f77ce0e6) PQ 使用了扩展堆栈。
* [提交 fac2](https://github.com/manticoresoftware/manticoresearch/commit/fac2d9687a20582765aaa89be9597804c4f845af) 更新了导出排名器输出，使其与 [packedfactors()](Functions/Searching_and_ranking_functions.md#PACKEDFACTORS%28%29) 保持一致。
* [提交 ff87](https://github.com/manticoresoftware/manticoresearch/commit/ff87cc4)：修复了 SphinxQL 查询日志中过滤器字符串列表的问题。
* [问题 #589](https://github.com/manticoresoftware/manticoresearch/issues/589) “字符集定义似乎依赖于代码顺序”：修正了重复项的错误字符集映射。
* [问题 #811](https://github.com/manticoresoftware/manticoresearch/issues/811) “多词映射干扰包含 CJK 标点符号的关键词短语搜索”：修正了带词形变化的短语查询中 ngram 词元位置的问题。
* [问题 #834](https://github.com/manticoresoftware/manticoresearch/issues/834) “搜索查询中的等号导致请求失败”：确保了精确符号可被转义，并修复了 `expand_keywords` 选项导致的双重精确扩展。
* [问题 #864](https://github.com/manticoresoftware/manticoresearch/issues/864) “异常/停用词冲突”
* [问题 #910](https://github.com/manticoresoftware/manticoresearch/issues/910) “调用 call snippets() 且使用 libstemmer_fr 和 index_exact_words 时 Manticore 崩溃”：解决了调用 `SNIPPETS()` 时导致崩溃的内部冲突。
* [Issue #946](https://github.com/manticoresoftware/manticoresearch/issues/946) “SELECT 期间的重复记录”：修复了针对带有已删除文档的 RT 索引且设置了 `not_terms_only_allowed` 选项的查询结果集中出现重复文档的问题。
* [Issue #967](https://github.com/manticoresoftware/manticoresearch/issues/967) “UDF 函数中使用 JSON 参数导致崩溃”：修复了启用伪分片并使用带 JSON 参数的 UDF 进行搜索时导致守护进程崩溃的问题。
* [Issue #1050](https://github.com/manticoresoftware/manticoresearch/issues/1050) “FEDERATED 中的 count(*)”：修复了通过带聚合的 `FEDERATED` 引擎查询时导致守护进程崩溃的问题。
* [Issue #1052](https://github.com/manticoresoftware/manticoresearch/issues/1052) 修复了 `rt_attr_json` 列与列式存储不兼容的问题。
* [Issue #1072](https://github.com/manticoresoftware/manticoresearch/issues/1072) “* 被 ignore_chars 从搜索查询中移除”：修复了该问题，确保查询中的通配符不受 `ignore_chars` 的影响。
* [Issue #1075](https://github.com/manticoresoftware/manticoresearch/issues/1075) “如果存在分布式表，indextool --check 失败”：indextool 现已兼容 json 配置中包含 ‘distributed’ 和 ‘template’ 索引的实例。
* [Issue #1081](https://github.com/manticoresoftware/manticoresearch/issues/1081) “对特定 RT 数据集的特定查询导致 searchd 崩溃”：解决了使用 packedfactors 和大型内部缓冲区时的守护进程崩溃问题。
* [Issue #1095](https://github.com/manticoresoftware/manticoresearch/issues/1095) “使用 not_terms_only_allowed 时已删除文档被忽略”
* [Issue #1099](https://github.com/manticoresoftware/manticoresearch/issues/1099) “indextool --dumpdocids 不工作”：恢复了 `--dumpdocids` 命令的功能。
* [Issue #1100](https://github.com/manticoresoftware/manticoresearch/issues/1100) “indextool --buildidf 不工作”：indextool 现在在完成 globalidf 后关闭文件。
* [Issue #1104](https://github.com/manticoresoftware/manticoresearch/issues/1104) “Count(*) 试图被当作远程表中的 schema set”：解决了在分布式索引查询时，当代理返回空结果集时守护进程发送错误消息的问题。
* [Issue #1109](https://github.com/manticoresoftware/manticoresearch/issues/1109) “使用 threads=1 时 FLUSH ATTRIBUTES 卡死”。
* [Issue #1126](https://github.com/manticoresoftware/manticoresearch/issues/1126) “查询期间丢失与 MySQL 服务器的连接 - manticore 6.0.5”：修复了对列式属性使用多重过滤器时发生的崩溃问题。
* [Issue #1135](https://github.com/manticoresoftware/manticoresearch/issues/1135) “JSON 字符串过滤的大小写敏感问题”：修正了排序规则，以便在 HTTP 搜索请求中使用过滤器时正常工作。
* [Issue #1140](https://github.com/manticoresoftware/manticoresearch/issues/1140) “错误字段上的匹配”：修复了与 `morphology_skip_fields` 相关的错误。
* [Issue #1155](https://github.com/manticoresoftware/manticoresearch/issues/1155) “通过 API 发送的系统远程命令应传递 g_iMaxPacketSize”：更新了跳过节点间复制命令的 `max_packet_size` 检查。此外，最新的集群错误已添加到状态显示中。
* [Issue #1302](https://github.com/manticoresoftware/manticoresearch/issues/1302) “优化失败后残留临时文件”：修正了合并或优化过程中出错后，临时文件未被删除的问题。
* [Issue #1304](https://github.com/manticoresoftware/manticoresearch/issues/1304) “为 buddy 启动超时添加环境变量”：新增环境变量 `MANTICORE_BUDDY_TIMEOUT`（默认 3 秒）用于控制守护进程启动时等待 buddy 消息的时长。
* [Issue #1305](https://github.com/manticoresoftware/manticoresearch/issues/1305) “保存 PQ 元数据时整型溢出”：缓解了守护进程在保存大型 PQ 索引时的过度内存消耗。
* [Issue #1306](https://github.com/manticoresoftware/manticoresearch/issues/1306) “修改外部文件后无法重新创建 RT 表”：修正了对外部文件执行 alter 时传入空字符串的错误；修复了修改外部文件后 RT 索引外部文件未正确移除的问题。
* [Issue #1307](https://github.com/manticoresoftware/manticoresearch/issues/1307) “SELECT 语句 sum(value) as value 不正常工作”：修复了查询列表表达式使用别名时可能遮蔽索引属性的问题；另外修正了整数类型的 sum 转为对 int64 的计数。
* [Issue #1308](https://github.com/manticoresoftware/manticoresearch/issues/1308) “避免在复制中绑定到 localhost”：确保复制时，对于有多个 IP 的主机名，不绑定到 localhost。
* [Issue #1309](https://github.com/manticoresoftware/manticoresearch/issues/1309) “对于超过 16Mb 的数据回复给 mysql 客户端失败”：修复了向客户端返回大于 16Mb 的 SphinxQL 包时的问题。
* [Issue #1310](https://github.com/manticoresoftware/manticoresearch/issues/1310) ““指向外部文件的路径应为绝对路径”中的错误引用”：纠正了 `SHOW CREATE TABLE` 中外部文件完整路径的显示。
* [Issue #1311](https://github.com/manticoresoftware/manticoresearch/issues/1311) “调试构建在 snippet 中的长字符串导致崩溃”：现在，`SNIPPET()` 函数目标的文本中允许超过 255 字符的长字符串。
* [Issue #1312](https://github.com/manticoresoftware/manticoresearch/issues/1312) “kqueue 轮询中使用删除后访问导致的虚假崩溃（master-agent）”：修复了在基于 kqueue 的系统（FreeBSD、MacOS 等）中，主节点无法连接到代理时的崩溃。
* [Issue #1313](https://github.com/manticoresoftware/manticoresearch/issues/1313) “连接自身超时过长”：从 MacOS/BSD 上的 master 连接到 agent 时，现在使用统一的连接加查询超时替代仅连接超时。
* [问题 #1314](https://github.com/manticoresoftware/manticoresearch/issues/1314) “pq（json meta）中未达到的嵌入同义词加载失败”：修复了pq中嵌入同义词标志。
* [问题 #1315](https://github.com/manticoresoftware/manticoresearch/issues/1315) “允许某些函数（sint, fibonacci, second, minute, hour, day, month, year, yearmonth, yearmonthday）使用隐式提升的参数值”。
* [问题 #1321](https://github.com/manticoresoftware/manticoresearch/issues/1321) “启用全扫描中的多线程SI，但限制线程数”：代码已实现到CBO中，以更好地预测二级索引在全文查询中使用时的多线程性能。
* [问题 #1322](https://github.com/manticoresoftware/manticoresearch/issues/1322) “使用预计算排序器后count(*)查询仍然缓慢”：使用基于预计算数据的排序器时，不再初始化迭代器，避免了不良性能影响。
* [问题 #1411](https://github.com/manticoresoftware/manticoresearch/issues/1141) “sphinxql中的查询日志不保留MVA的原始查询”：现在，`all()/any()`被记录。

## 版本 6.0.4
发布日期：2023年3月15日

### 新功能
* 改进了与Logstash、Beats等的集成，包括：
  - 支持Logstash 7.6 - 7.15版本，Filebeat 7.7 - 7.12版本
  - 自动模式支持。
  - 新增对类似Elasticsearch格式的批量请求处理。
* [Buddy 提交 ce90](https://github.com/manticoresoftware/manticoresearch-buddy/commit/ce907ea) 在Manticore启动时记录Buddy版本。

### Bug修复
* [问题 #588](https://github.com/manticoresoftware/manticoresearch/issues/588), [问题 #942](https://github.com/manticoresoftware/manticoresearch/issues/942) 修复了双字母索引搜索元数据中出现的错误字符及调用关键词问题。
* [问题 #1027](https://github.com/manticoresoftware/manticoresearch/issues/1027) 拒绝小写HTTP头。
* ❗[问题 #1039](https://github.com/manticoresoftware/manticoresearch/issues/1039) 修复守护进程在读取Buddy控制台输出时的内存泄漏。
* [问题 #1056](https://github.com/manticoresoftware/manticoresearch/issues/1056) 修复问号的不预期行为。
* [问题 #1064](https://github.com/manticoresoftware/manticoresearch/issues/1064) - 修复标记器小写表中的竞态条件导致的崩溃。
* [提交 59bb](https://github.com/manticoresoftware/manticoresearch/commit/59bb54c) 修复JSON接口中对于id显式设置为null的文档的批量写入处理。
* [提交 7b6b](https://github.com/manticoresoftware/manticoresearch/commit/7b6b25f) 修复CALL KEYWORDS中多个相同术语的词项统计。
* [提交 f381](https://github.com/manticoresoftware/manticoresearch/commit/f381ad2) Windows安装程序现创建默认配置；路径不再在运行时替换。
* [提交 6940](https://github.com/manticoresoftware/manticoresearch/commit/6940e95), [提交 cc5a](https://github.com/manticoresoftware/manticoresearch/commit/cc5a480) 修复跨多网络节点集群的复制问题。
* [提交 4972](https://github.com/manticoresoftware/manticoresearch/commit/49722ab) 修复`/pq` HTTP端点为`/json/pq` HTTP端点的别名。
* [提交 3b53](https://github.com/manticoresoftware/manticoresearch/commit/3b5385a) 修复Buddy重启时守护进程崩溃。
* [Buddy 提交 fba9](https://github.com/manticoresoftware/manticoresearch-buddy/commit/fba9c8c) 显示收到的无效请求的原始错误。
* [Buddy 提交 db95](https://github.com/manticoresoftware/manticoresearch-buddy/commit/db9532c) 允许备份路径中有空格，并对正则表达式做了一些魔法以支持单引号。

## 版本 6.0.2
发布日期：2023年2月10日

### Bug修复
* [问题 #1024 crash 2](https://github.com/manticoresoftware/manticoresearch/issues/1024) Facet搜索大量结果时崩溃/段错误。
* ❗[问题 #1029](https://github.com/manticoresoftware/manticoresearch/issues/1029) - 警告：编译时值KNOWN_CREATE_SIZE（16）小于测量值（208）。建议修正该值！
* ❗[问题 #1032](https://github.com/manticoresoftware/manticoresearch/issues/1032) - Manticore 6.0.0 plain索引崩溃。
* ❗[问题 #1033](https://github.com/manticoresoftware/manticoresearch/issues/1033) - 守护进程重启后多个分布式节点丢失。
* ❗[问题 #1064](https://github.com/manticoresoftware/manticoresearch/issues/1064) - 标记器小写表中的竞态条件。

## 版本 6.0.0
发布日期：2023年2月7日

从此版本开始，Manticore Search 配备了 Manticore Buddy，一个用PHP编写的边车守护进程，用于处理不需要极低延迟或高吞吐量的高级功能。Manticore Buddy 在后台运行，用户可能甚至感觉不到它的存在。虽然对最终用户是透明的，但使Manticore Buddy易于安装并与主要的基于C++的守护进程兼容，是一个重大挑战。此重大变更将允许团队开发多种全新的高级功能，例如分片编排、访问控制和身份认证，以及各种集成，如mysqldump、DBeaver、Grafana mysql连接器。当前它已支持 [SHOW QUERIES](Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES)，[BACKUP](Securing_and_compacting_a_table/Backup_and_restore.md#BACKUP-SQL-command-reference) 和 [自动模式](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema)。

本版本还包括130多个漏洞修复和众多功能，其中许多可视为重大更新。

### 重大变更
* 🔬 实验性功能：您现在可以执行兼容 Elasticsearch 的 [insert](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md?example=Elasticsearch#Adding-documents-to-a-real-time-table) 和 [replace](Data_creation_and_modification/Updating_documents/REPLACE.md?example=Elasticsearch#REPLACE) JSON 查询，这使得 Manticore 可以与 Logstash（版本低于 7.13）、Filebeat 以及 Beats 家族的其他工具一起使用。默认启用。您可以使用 `SET GLOBAL ES_COMPAT=off` 禁用它。
* 支持 [Manticore Columnar Library 2.0.0](https://github.com/manticoresoftware/columnar/) ，对 [二级索引](Server_settings/Searchd.md#secondary_indexes) 进行了诸多修复和改进。**⚠️ 重大变更**：从本版本起，二级索引默认开启。如果您是从 Manticore 5 升级，请确保执行 [ALTER TABLE table_name REBUILD SECONDARY](Updating_table_schema_and_settings.md#Rebuilding-a-secondary-index)。详情见下文。
* [Commit c436](https://github.com/manticoresoftware/manticoresearch/commit/c436f9023536f767610451911955ae36d90aa638) 自动模式：您现在可以跳过创建表的步骤，直接插入第一条文档，Manticore 会根据字段自动创建表。详细内容请参阅 [此处](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema)。您可以通过 [searchd.auto_schema](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema) 来开启或关闭此功能。
* 彻底改进了 [基于成本的优化器](Searching/Cost_based_optimizer.md)，在许多情况下降低了查询响应时间。
  - [Issue #1008](https://github.com/manticoresoftware/manticoresearch/issues/1008) CBO 中的并行性能估计。
  - [Issue #1014](https://github.com/manticoresoftware/manticoresearch/issues/1014) CBO 现在支持识别 [二级索引](Server_settings/Searchd.md#secondary_indexes)，能做出更智能的决策。
  - [Commit cef9](https://github.com/manticoresoftware/columnar/commit/cef9b54dc3a4ee9138ebc4df3a671715b996d6b3) 现将列式表/字段的编码统计存储在元数据中，帮助 CBO 做出更智能的决策。
  - [Commit 2b95](https://github.com/manticoresoftware/manticoresearch/commit/2b95fb8c2ba422c9c5a66e2b61c1f7922f31c7aa) 添加了 CBO 提示以微调其行为。
* [遥测](Telemetry.md#Telemetry)：我们很高兴在此版本引入遥测功能。该功能允许我们收集匿名和去标识化的指标，帮助提升产品的性能和用户体验。请放心，所有收集的数据**完全匿名，且不会与任何个人信息关联**。如果需要，此功能可在设置中 [轻松关闭](Telemetry.md#Telemetry)。
* [Commit 5aaf](https://github.com/manticoresoftware/manticoresearch/commit/5aafb17d81b601e399e212a344a4738f65576cdd) [ALTER TABLE table_name REBUILD SECONDARY](Updating_table_schema_and_settings.md#Rebuilding-a-secondary-index) 命令可按需重建二级索引，例如：
  - 从 Manticore 5 迁移到新版本时，
  - 对索引中的属性执行了 [UPDATE](Data_creation_and_modification/Updating_documents/UPDATE.md#UPDATE)（即 [原地更新，而非替换](Data_creation_and_modification/Updating_documents/REPLACE_vs_UPDATE.md)）
* [Issue #821](https://github.com/manticoresoftware/manticoresearch/issues/821) 新工具 `manticore-backup` 用于[备份和恢复 Manticore 实例](Securing_and_compacting_a_table/Backup_and_restore.md)
* SQL 命令 [BACKUP](Securing_and_compacting_a_table/Backup_and_restore.md#BACKUP-SQL-command-reference) 可在 Manticore 内部执行备份。
* SQL 命令 [SHOW QUERIES](Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES) 方便查看正在运行的查询（而非线程）。
* [Issue #551](https://github.com/manticoresoftware/manticoresearch/issues/551) SQL 命令 `KILL` 用于终止长时间运行的 `SELECT`。
* 聚合查询支持动态 `max_matches` 参数，提高精度并降低响应时间。

### 小改动
* [Issue #822](https://github.com/manticoresoftware/manticoresearch/issues/822) SQL 命令 [FREEZE/UNFREEZE](Securing_and_compacting_a_table/Freezing_a_table.md) 用于准备实时表或普通表进行备份。
* [Commit c470](https://github.com/manticoresoftware/manticoresearch/commit/c47052f18ad4868134de1a97e8d7da570e531dfa) 新增设置 `accurate_aggregation` 和 `max_matches_increase_threshold` 用于控制聚合精度。
* [Issue #718](https://github.com/manticoresoftware/manticoresearch/issues/718) 支持带符号的负 64 位 ID。注意，您仍然不能使用 ID > 2^63，但现在可以使用 -2^63 到 0 范围内的 ID。
* 鉴于我们最近支持了二级索引，"index" 一词可能代表二级索引、全文索引或普通/实时 `index`，导致了混淆。为减少混淆，我们将后者重命名为“table”。以下 SQL/命令行命令受到该变更影响。它们的旧版本已废弃，但仍然可用：
  - `index <table name>` => `table <table name>`，
  - `searchd -i / --index` => `searchd -t / --table`，
  - `SHOW INDEX STATUS` => `SHOW TABLE STATUS`，
  - `SHOW INDEX SETTINGS` => `SHOW TABLE SETTINGS`，
  - `FLUSH RTINDEX` => `FLUSH TABLE`，
  - `OPTIMIZE INDEX` => `OPTIMIZE TABLE`，
  - `ATTACH TABLE plain TO RTINDEX rt` => `ATTACH TABLE plain TO TABLE rt`，
  - `RELOAD INDEX` => `RELOAD TABLE`，
  - `RELOAD INDEXES` => `RELOAD TABLES`。

  我们暂不打算废除旧形式，但为了确保文档兼容，建议在您的应用中更改命令名称。后续版本计划将各种 SQL 和 JSON 命令输出中的 "index" 统一改为 "table"。
* 带有有状态 UDF 的查询现在被强制单线程执行。
* [Issue #1011](https://github.com/manticoresoftware/manticoresearch/issues/1011) 重构所有与时间调度相关的内容，作为并行块合并的前提条件。
* **⚠️ 重大更改**：列式存储格式已更改。您需要重建具有列式属性的表。
* **⚠️ 重大更改**：二级索引文件格式已更改，因此如果您使用二级索引进行搜索，并且配置文件中有 `searchd.secondary_indexes = 1`，请注意新的 Manticore 版本**将跳过加载具有二级索引的表**。建议：
  - 在升级前，将配置文件中的 `searchd.secondary_indexes` 改为 0。
  - 启动实例。Manticore 将加载表并发出警告。
  - 对每个索引运行 `ALTER TABLE <表名> REBUILD SECONDARY` 以重建二级索引。

  如果你运行的是复制集群，则需要在所有节点上运行 `ALTER TABLE <表名> REBUILD SECONDARY`，或者按照[此说明](Securing_and_compacting_a_table/Compacting_a_table.md#Optimizing-clustered-tables)操作，只需将 `OPTIMIZE` 替换为 `ALTER .. REBUILD SECONDARY`。
* **⚠️ 重大更改**：binlog 版本已更新，因此任何来自旧版本的 binlog 将不会被重放。升级过程中必须确保 Manticore Search 干净地停止。这意味着停止旧实例后，`/var/lib/manticore/binlog/` 中除了 `binlog.meta` 外不应有其他 binlog 文件。
* [Issue #849](https://github.com/manticoresoftware/manticoresearch/issues/849) `SHOW SETTINGS`：您现在可以在 Manticore 内部查看配置文件中的设置。
* [Issue #1007](https://github.com/manticoresoftware/manticoresearch/issues/1007) [SET GLOBAL CPUSTATS=1/0](Server_settings/Setting_variables_online.md#SET) 用于开启/关闭 CPU 时间跟踪；[SHOW THREADS](Node_info_and_management/SHOW_THREADS.md) 在 CPU 时间跟踪关闭时不再显示 CPU 统计信息。
* [Issue #1009](https://github.com/manticoresoftware/manticoresearch/issues/1009) RT 表内存块段现在可以在内存块刷新时合并。
* [Issue #1012](https://github.com/manticoresoftware/manticoresearch/issues/1012) 在 [indexer](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) 的输出中新增二级索引进度显示。
* [Issue #1013](https://github.com/manticoresoftware/manticoresearch/issues/1013) 以前，如果表记录在启动时无法提供服务，Manticore 可能会从索引列表中移除它。新行为是保留在列表中，尝试在下一次启动时加载。
* [indextool --docextract](Miscellaneous_tools.md#indextool) 返回所有属于请求文档的词和命中。
* [Commit 2b29](https://github.com/manticoresoftware/manticoresearch/commit/2b296ee20e520b85bcbb4383e87095d31e7165dc) 环境变量 `dump_corrupt_meta` 启用在 searchd 无法加载索引时将损坏的表元数据转储到日志。
* [Commit c7a3](https://github.com/manticoresoftware/manticoresearch/commit/c7a3136eefe0eb2f69e3d99fae9c10beb5821ea6) `DEBUG META` 可以显示 `max_matches` 和伪分片统计。
* [Commit 6bca](https://github.com/manticoresoftware/manticoresearch/commit/6bca4848dc5c8396dbc1e057be4b9b2d579794e8) 替换了“索引头格式不是 json，将尝试二进制...”这个令人困惑的错误提示，提供了更清晰的错误信息。
* [Commit bef3](https://github.com/manticoresoftware/lemmatizer-uk/commit/bef3ff0386d3ee87ec57619782100972c1122e47) 乌克兰语词形还原器路径已更改。
* [Commit 4ae7](https://github.com/manticoresoftware/manticoresearch/commit/4ae789595329a2951e194d1191ddb3121459a560) 在 [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META) 中新增二级索引统计信息。
* [Commit 2e7c](https://github.com/manticoresoftware/manticoresearch/commit/2e7c585e) JSON 接口现在可以使用 Swagger Editor (https://manual.manticoresearch.com/Openapi#OpenAPI-specification) 方便地进行可视化。
* **⚠️ 重大更改**：复制协议已更改。如果您运行复制集群，则升级到 Manticore 5 时需要：
  - 首先干净地停止所有节点
  - 然后以 `--new-cluster` 参数启动最后停止的节点（Linux 下运行工具 `manticore_new_cluster`）。
  - 详细内容请阅读[重启集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster)说明。

### 与 Manticore 列式库相关的更改
* 重构二级索引与列式存储的集成。
* [Commit efe2](https://github.com/manticoresoftware/columnar/commit/efe26b2a) Manticore 列式库优化，通过部分预先的最小/最大值计算降低响应时间。
* [Commit 2757](https://github.com/manticoresoftware/columnar/commit/2757b99b) 如果磁盘块合并中断，守护进程现在会清理与 MCL 相关的临时文件。
* [Commit e9c6](https://github.com/manticoresoftware/manticoresearch/commit/e9c672014b63e2b49e7a5a53880583af86c3ab4e) 列式库和二级库版本会在崩溃时打印到日志。
* [Commit f5e8](https://github.com/manticoresoftware/columnar/commit/f5e84eeb8b6cb102f13e3468246702427527fef9) 新增支持二级索引的快速文档列表回绕。
* [Commit 06df](https://github.com/manticoresoftware/manticoresearch/commit/06dfdd2e668933495dd6cb52d5ba2e0aa9b26e58) 对于使用 MCL 的情况，类似 `select attr, count(*) from plain_index`（无过滤）查询现在更快。
* [Commit 0a76](https://github.com/manticoresoftware/manticoresearch/commit/0a76ecb9adbe886868b962dd425de5f475bbf55e) `@@autocommit` 在 HandleMysqlSelectSysvar 中，用于兼容 MySQL 大于 8.25 的 .net 连接器。
* **⚠️ 重大更改**： [MCL Issue #17](https://github.com/manticoresoftware/columnar/issues/17) MCL：为列扫描添加 SSE 代码。MCL 现在要求至少支持 SSE4.2。

### 与打包相关的更改
* [Commit 4d19](https://github.com/manticoresoftware/manticoresearch/commit/4d19f5cbe49a31228aa09253f061165bfe80e51b) **⚠️ 重大变更**：停止支持 Debian Stretch 和 Ubuntu Xenial。
* 支持 RHEL 9，包括 Centos 9、Alma Linux 9 和 Oracle Linux 9。
* [Issue #924](https://github.com/manticoresoftware/manticoresearch/issues/924) 支持 Debian Bookworm。
* [Issue #636](https://github.com/manticoresoftware/manticoresearch/issues/636) 打包：支持 Linux 和 MacOS 的 arm64 构建。
* [PR #26](https://github.com/manticoresoftware/docker/pull/26) 支持多架构（x86_64 / arm64）的 Docker 镜像。
* [为贡献者简化了包的构建](Installation/Compiling_from_sources.md#Building-using-CI-Docker)。
* 现在可以使用 APT 安装特定版本。
* [Commit a6b8](https://github.com/manticoresoftware/manticoresearch/commit/51fddca5c2a3ebb8576fae4c18660656ba22de0f) Windows 安装程序（之前只提供压缩包）。
* 切换到使用 CLang 15 编译。
* **⚠️ 重大变更**：自定义 Homebrew 配方，包括 Manticore Columnar Library 的配方。安装 Manticore、MCL 及其他必要组件请使用以下命令 `brew install manticoresoftware/manticore/manticoresearch manticoresoftware/manticore/manticore-extra`。

### 修复的错误
* [Issue #479](https://github.com/manticoresoftware/manticoresearch/issues/479) 名为 `text` 的字段。
* [Issue #501](https://github.com/manticoresoftware/manticoresearch/issues/501) id 不能非 bigint。
* [Issue #646](https://github.com/manticoresoftware/manticoresearch/issues/646) ALTER 操作与名为 "text" 的字段冲突。
* ❗[Issue #652](https://github.com/manticoresoftware/manticoresearch/issues/652) 可能的 BUG：HTTP（JSON）偏移量和限制影响 facet 结果。
* ❗[Issue #827](https://github.com/manticoresoftware/manticoresearch/issues/827) Searchd 在高负载下挂起/崩溃。
* ❗[Issue #996](https://github.com/manticoresoftware/manticoresearch/issues/996) PQ 索引内存溢出。
* ❗[Commit 1041](https://github.com/manticoresoftware/manticoresearch/commit/10416ef7dddf06c0d759e32ccd6ebaa2468f7cbf) 自 Sphinx 起 `binlog_flush = 1` 就一直有问题。已修复。
* [MCL Issue #14](https://github.com/manticoresoftware/columnar/issues/14) MCL：当 ft 字段过多时，select 崩溃。
* [Issue #470](https://github.com/manticoresoftware/manticoresearch/issues/470) sql_joined_field 无法存储。
* [Issue #713](https://github.com/manticoresoftware/manticoresearch/issues/713) 使用 LEVENSHTEIN() 时崩溃。
* [Issue #743](https://github.com/manticoresoftware/manticoresearch/issues/743) Manticore 意外崩溃且无法正常重启。
* [Issue #788](https://github.com/manticoresoftware/manticoresearch/issues/788) 通过 /sql 调用 KEYWORDS 返回控制字符，破坏 json。
* [Issue #789](https://github.com/manticoresoftware/manticoresearch/issues/789) mariadb 无法创建 FEDERATED 表。
* [Issue #796](https://github.com/manticoresoftware/manticoresearch/issues/796) 警告：dlopen() 失败：/usr/bin/lib_manticore_columnar.so：无法打开共享对象文件：没有此类文件或目录。
* [Issue #797](https://github.com/manticoresoftware/manticoresearch/issues/797) 通过 API 使用 ZONESPAN 搜索时 Manticore 崩溃。
* [Issue #799](https://github.com/manticoresoftware/manticoresearch/issues/799) 使用多个索引且带 facet distinct 时权重错误。
* [Issue #801](https://github.com/manticoresoftware/manticoresearch/issues/801) SphinxQL 分组查询在 SQL 索引重新处理后挂起。
* [Issue #802](https://github.com/manticoresoftware/manticoresearch/issues/802) MCL：Indexer 在 5.0.2 和 manticore-columnar-lib 1.15.4 版本中崩溃。
* [Issue #813](https://github.com/manticoresoftware/manticoresearch/issues/813) Manticore 5.0.2 FEDERATED 返回空集（MySQL 8.0.28）。
* [Issue #824](https://github.com/manticoresoftware/manticoresearch/issues/824) 当 2 个索引上 select COUNT DISTINCT 结果为零时抛出内部错误。
* [Issue #826](https://github.com/manticoresoftware/manticoresearch/issues/826) 删除查询时崩溃。
* [Issue #843](https://github.com/manticoresoftware/manticoresearch/issues/843) MCL：长文本字段的错误。
* [Issue #856](https://github.com/manticoresoftware/manticoresearch/issues/856) 5.0.2 rtindex：聚合搜索限制行为不符合预期。
* [Issue #863](https://github.com/manticoresoftware/manticoresearch/issues/863) 返回的 Hits 是 Nonetype 对象，即使搜索应返回多个结果。
* [Issue #870](https://github.com/manticoresoftware/manticoresearch/issues/870) 在 SELECT 表达式中使用 Attribute 和 Stored Field 崩溃。
* [Issue #872](https://github.com/manticoresoftware/manticoresearch/issues/872) 崩溃后表变为不可见。
* [Issue #877](https://github.com/manticoresoftware/manticoresearch/issues/877) 搜索查询中两个负项产生错误：查询不可计算。
* [Issue #878](https://github.com/manticoresoftware/manticoresearch/issues/878) a -b -c 通过 json query_string 不能正常工作。
* [Issue #886](https://github.com/manticoresoftware/manticoresearch/issues/886) 伪分片与查询匹配。
* [Issue #893](https://github.com/manticoresoftware/manticoresearch/issues/893) Manticore 5.0.2 min/max 函数运行不符合预期…
* [Issue #896](https://github.com/manticoresoftware/manticoresearch/issues/896) 字段 "weight" 解析不正确。
* [Issue #897](https://github.com/manticoresoftware/manticoresearch/issues/897) Manticore 服务启动时崩溃并持续重启。
* [Issue #900](https://github.com/manticoresoftware/manticoresearch/issues/900) group by j.a, smth 行为异常。
* [Issue #913](https://github.com/manticoresoftware/manticoresearch/issues/913) 当 ranker 中使用 expr，且查询包含两个接近度时，Searchd 崩溃。
* [Issue #916](https://github.com/manticoresoftware/manticoresearch/issues/916) net_throttle_action 出现故障。
* [Issue #919](https://github.com/manticoresoftware/manticoresearch/issues/919) MCL：Manticore 在查询执行时崩溃，以及集群恢复期间的其他崩溃。
* [Issue #925](https://github.com/manticoresoftware/manticoresearch/issues/925) SHOW CREATE TABLE 输出不带反引号
* [Issue #930](https://github.com/manticoresoftware/manticoresearch/issues/930) 现在可以通过 JDBC 连接器从 Java 查询 Manticore
* [Issue #933](https://github.com/manticoresoftware/manticoresearch/issues/933) bm25f 排名问题
* [Issue #934](https://github.com/manticoresoftware/manticoresearch/issues/934) 无配置索引在 watchdog 的首次加载状态冻结
* [Issue #937](https://github.com/manticoresoftware/manticoresearch/issues/937) 在排序 facet 数据时出现段错误
* [Issue #940](https://github.com/manticoresoftware/manticoresearch/issues/940) 在 CONCAT(TO_STRING) 上崩溃
* [Issue #947](https://github.com/manticoresoftware/manticoresearch/issues/947) 在某些情况下，单个简单的 select 语句可能导致整个实例停顿，直到该 select 执行完成，无法登录或运行其他查询。
* [Issue #948](https://github.com/manticoresoftware/manticoresearch/issues/948) 索引器崩溃
* [Issue #950](https://github.com/manticoresoftware/manticoresearch/issues/950) facet distinct 返回错误的计数
* [Issue #953](https://github.com/manticoresoftware/manticoresearch/issues/953) 内置 sph04 排序器中 LCS 计算不正确
* [Issue #955](https://github.com/manticoresoftware/manticoresearch/issues/955) 5.0.3 开发版本崩溃
* [Issue #963](https://github.com/manticoresoftware/manticoresearch/issues/963) FACET 在 engine columnar 使用 json 时崩溃
* [Issue #982](https://github.com/manticoresoftware/manticoresearch/issues/982) MCL：5.0.3 因二级索引崩溃
* [PR #984](https://github.com/manticoresoftware/manticoresearch/pull/984) HandleMysqlSelectSysvar 中的 @@autocommit
* [PR #985](https://github.com/manticoresoftware/manticoresearch/pull/985) 修正 RT 索引中的线程块分配
* [Issue #985](https://github.com/manticoresoftware/manticoresearch/issues/985) 修正 RT 索引中的线程块分配
* [Issue #986](https://github.com/manticoresoftware/manticoresearch/issues/986) 错误的默认 max_query_time
* [Issue #987](https://github.com/manticoresoftware/manticoresearch/issues/987) 多线程执行中使用正则表达式时崩溃
* [Issue #988](https://github.com/manticoresoftware/manticoresearch/issues/988) 破坏的反向索引兼容性
* [Issue #989](https://github.com/manticoresoftware/manticoresearch/issues/989) indextool 检查列式属性时报告错误
* [Issue #990](https://github.com/manticoresoftware/manticoresearch/issues/990) json grouper 克隆的内存泄漏
* [Issue #991](https://github.com/manticoresoftware/manticoresearch/issues/991) levenshtein 函数克隆的内存泄漏
* [Issue #992](https://github.com/manticoresoftware/manticoresearch/issues/992) 加载 meta 时错误信息丢失
* [Issue #993](https://github.com/manticoresoftware/manticoresearch/issues/993) 传播动态索引/子键和系统变量的错误
* [Issue #994](https://github.com/manticoresoftware/manticoresearch/issues/994) 在列式存储中，对列式字符串做 count distinct 时崩溃
* [Issue #995](https://github.com/manticoresoftware/manticoresearch/issues/995) MCL：taxi1 的 min(pickup_datetime) 崩溃
* [Issue #997](https://github.com/manticoresoftware/manticoresearch/issues/997) 空的 excludes JSON 查询从选择列表中移除列
* [Issue #998](https://github.com/manticoresoftware/manticoresearch/issues/998) 当前调度器上运行的二级任务有时会引起异常副作用
* [Issue #999](https://github.com/manticoresoftware/manticoresearch/issues/999) facet distinct 和不同模式时崩溃
* [Issue #1000](https://github.com/manticoresoftware/manticoresearch/issues/1000) MCL：未加载列式库的情况下运行后，列式实时索引损坏
* [Issue #1001](https://github.com/manticoresoftware/manticoresearch/issues/1001) 隐式 cutoff 在 json 中无效
* [Issue #1002](https://github.com/manticoresoftware/manticoresearch/issues/1002) 列式 grouper 问题
* [Issue #1003](https://github.com/manticoresoftware/manticoresearch/issues/1003) 无法从索引中删除最后一个字段
* [Issue #1004](https://github.com/manticoresoftware/manticoresearch/issues/1004) --new-cluster 后行为异常
* [Issue #1005](https://github.com/manticoresoftware/manticoresearch/issues/1005) “columnar library not loaded”，但实际上不需要
* [Issue #1006](https://github.com/manticoresoftware/manticoresearch/issues/1006) 删除查询无错误提示
* [Issue #1010](https://github.com/manticoresoftware/manticoresearch/issues/1010) 修正 Windows 构建中的 ICU 数据文件位置
* [PR #1018](https://github.com/manticoresoftware/manticoresearch/pull/1018) 握手发送问题
* [Issue #1020](https://github.com/manticoresoftware/manticoresearch/issues/1020) 在 show create table 中显示 id
* [Issue #1024 crash 1](https://github.com/manticoresoftware/manticoresearch/issues/1024) 大量结果的 Facet 搜索导致崩溃/段错误。
* [Issue #1026](https://github.com/manticoresoftware/manticoresearch/issues/1026) RT 索引：大量文档插入且 RAMchunk 充满时，searchd “永远”卡住
* [Commit 4739](https://github.com/manticoresoftware/manticoresearch/commit/4739dafd) 复制在节点间忙碌时，线程在关闭时卡住
* [Commit ab87](https://github.com/manticoresoftware/manticoresearch/commit/ab87836782e7ae43fe5f9dea739eed0d336b42c1) JSON 范围过滤器中混合浮点数和整数可能导致 Manticore 忽略过滤器
* [Commit d001](https://github.com/manticoresoftware/manticoresearch/commit/d00101c2905f6393ce3fca23c4f6dcb2506f4bd9) JSON 中的浮点过滤器不准确
* [Commit 4092](https://github.com/manticoresoftware/manticoresearch/commit/4092c6a8) 若索引被修改则丢弃未提交的事务（否则可能崩溃）
* [Commit 9692](https://github.com/manticoresoftware/manticoresearch/commit/9692a840) 使用反斜杠时的查询语法错误
* [Commit 0c19](https://github.com/manticoresoftware/manticoresearch/commit/0c19ad8d) workers_clients 在 SHOW STATUS 中可能错误
* [Commit 1772](https://github.com/manticoresoftware/manticoresearch/commit/17720e1e2f2ec38337df565bee78f7ce9d9c744a) 修正合并无文档存储的内存段时的崩溃
* [Commit f45b](https://github.com/manticoresoftware/manticoresearch/commit/f45b337b5c265433bb7d6262ad65d02f65edbaf3) 修正遗漏的针对等式 JSON 过滤器的 ALL/ANY 条件
* [Commit 3e83](https://github.com/manticoresoftware/manticoresearch/commit/3e83c1f0c0e6e7a9d4297113f69a112cbabd9e64) 如果 searchd 从不可写目录启动，复制可能失败并出现 `got exception while reading ist stream: mkstemp(./gmb_pF6TJi) failed: 13 (Permission denied)`。
* [Commit 92e5](https://github.com/manticoresoftware/manticoresearch/commit/92e5629f7150f62524b385fca670a01ba9590f5b) 自 4.0.2 起崩溃日志只包含偏移量。本提交修复了该问题。

## 版本 5.0.2
发布：2022年5月30日

### Bug 修复
* ❗[Issue #791](https://github.com/manticoresoftware/manticoresearch/issues/791) - 错误的堆栈大小可能导致崩溃。

## 版本 5.0.0
发布：2022年5月18日


### 主要新特性
* 🔬 支持 [Manticore Columnar Library 1.15.2](https://github.com/manticoresoftware/columnar/)，启用 [二级索引](Server_settings/Searchd.md#secondary_indexes) 测试版。对于纯文本和实时列存以及行存索引（如果使用 [Manticore Columnar Library](https://github.com/manticoresoftware/columnar)），默认开启二级索引构建，但要启用搜索需要在配置文件或通过 [SET GLOBAL](Server_settings/Setting_variables_online.md) 设置 `secondary_indexes = 1`。该功能支持除了旧版 Debian Stretch 和 Ubuntu Xenial 之外的所有操作系统。
* [只读模式](Security/Read_only.md)：现在可以指定仅处理读取查询而忽略写入的监听器。
* 新增 [/cli](Connecting_to_the_server/HTTP.md#/cli) 端点，使得通过 HTTP 运行 SQL 查询更加简单。
* 通过 HTTP JSON 实现更快的批量 INSERT/REPLACE/DELETE：之前可通过 HTTP JSON 协议提供多条写入命令，但会逐条处理，现在作为单一事务处理。
* [#720](https://github.com/manticoresoftware/manticoresearch/issues/720) JSON 协议支持 [嵌套过滤器](Searching/Filters.md#Nested-bool-query)。此前无法在 JSON 中编码如 `a=1 and (b=2 or c=3)`：`must`（AND）、`should`（OR）和 `must_not`（NOT）仅在最高层级有效。现在支持嵌套。
* 支持 HTTP 协议中的 [分块传输编码](https://en.wikipedia.org/wiki/Chunked_transfer_encoding)。应用可使用分块传输发送大批量数据，降低资源消耗（无需计算 `Content-Length`）。服务器端 Manticore 现始终以流模式处理 HTTP 数据，而非等待完整批量传输完成，带来以下优势：
  - 降低峰值内存使用，降低 OOM 风险
  - 减少响应时间（测试显示处理 100MB 批量减少约11%）
  - 可绕过 [max_packet_size](Server_settings/Searchd.md#max_packet_size) 限制，传输远大于最大允许值（128MB）的批量，如一次传输 1GB。
* [#719](https://github.com/manticoresoftware/manticoresearch/issues/719) HTTP 接口支持 `100 Continue`：现在可从 `curl`（及多种语言使用的 curl 库）传输大批量数据，默认行为为 `Expect: 100-continue` 并等待后续发送。此前需加 `Expect: ` 头部，现在不再需要。

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
  * 完全上传完成且一切正常
  < HTTP/1.1 200 OK
  < Server: 4.2.1 63e5749@220405 dev
  < Content-Type: application/json; charset=UTF-8
  < Content-Length: 147
  <
  * 连接 #0 与主机 localhost 维持不变
  {"items":[{"bulk":{"table":"user","_id":2811798919590182916,"created":4,"deleted":0,"updated":0,"result":"created","status":201}}],"errors":false}
  real

0m0.015s

  user
0m0.005s
  sys
0m0.004s
  ```
  </details>

* **⚠️ 重大变更**：[伪分片](Server_settings/Searchd.md#pseudo_sharding)现在默认启用。如果你想禁用它，请确保在你的Manticore配置文件的`searchd`部分添加 `pseudo_sharding = 0`。
* 现在，实时/普通索引中至少包含一个全文字段不再是强制要求。你现在可以在与全文搜索无关的情况下使用Manticore。
* [快速提取](Creating_a_table/Data_types.md#fast_fetch)支持由[Manticore Columnar Library](https://github.com/manticoresoftware/columnar)支持的属性：类似`select * from <columnar table>`的查询现在比以前快得多，特别是当模式中字段较多时。
* **⚠️ 重大变更**：隐式的[cutoff](Searching/Options.md#cutoff)。Manticore现在不会花费时间和资源处理你不需要在结果集中返回的数据。缺点是这会影响到[SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META)中的`total_found`以及JSON输出中的[hits.total](Searching/Full_text_matching/Basic_usage.md#HTTP-JSON)。只有当你看到`total_relation: eq`时，它才是准确的，而`total_relation: gte`意味着实际匹配文档数量大于你得到的`total_found`值。要保持之前的行为，可以使用搜索选项`cutoff=0`，这会使`total_relation`始终为`eq`。
* **⚠️ 重大变更**：所有全文字段现在默认是[存储](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#stored_fields)的。你需要使用`stored_fields = `（空值）将所有字段设置为不存储（即恢复以前的行为）。
* [#715](https://github.com/manticoresoftware/manticoresearch/issues/715) HTTP JSON支持[搜索选项](Searching/Options.md#General-syntax)。
### 小改动
* **⚠️ 重大变更**：索引元文件格式变化。之前的元文件（`.meta`，`.sph`）是二进制格式，现在改为JSON格式。新版Manticore会自动转换旧索引，但：
  - 你可能会看到类似`WARNING: ... syntax error, unexpected TOK_IDENT`的警告
  - 旧版本的Manticore将无法运行这些索引，确保你有备份
* **⚠️ 重大变更**：借助[HTTP keep-alive](Connecting_to_the_server/HTTP.md#Keep-alive)支持会话状态。这使得HTTP在客户端支持时具备状态。例如，使用新[/cli](Connecting_to_the_server/HTTP.md#/cli)端点并启用HTTP keep-alive（在所有浏览器中默认开启），你可以在`SELECT`之后调用`SHOW META`，它的行为与通过mysql相同。注意，之前`Connection: keep-alive` HTTP头也支持，但只会导致连接复用，从这个版本开始它还使会话具备状态性。
* 你现在可以指定`columnar_attrs = *`，将所有属性定义为列式存储，适用于[普通模式](Read_this_first.md#Real-time-mode-vs-plain-mode)并且属性列表较长的情况。
* SST复制更快
* **⚠️ 重大变更**：复制协议已更改。如果你正在运行复制集群，升级到Manticore 5时需要：
  - 先干净地停止所有节点
  - 然后以`--new-cluster`参数启动最后停止的节点（在Linux上运行工具`manticore_new_cluster`）。
  - 详细信息请参阅[重启集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster)。
* 复制改进：
  - SST更快
  - 噪声抵抗，能够帮助处理复制节点之间不稳定的网络情况
  - 改进日志记录
* 安全性提升：若配置文件中未指定 `listen`，Manticore 现在监听 `127.0.0.1`，而不是 `0.0.0.0`。尽管随 Manticore Search 默认配置中指定了 `listen`，且通常不会没有 `listen` 配置，但仍有可能未指定。此前 Manticore 会监听 `0.0.0.0`，这不安全，现在改为监听通常不对互联网开放的 `127.0.0.1`。
* 基于列存属性的聚合查询性能提升。
* `AVG()` 函数精度提高：之前 Manticore 内部聚合使用 `float`，现在改为使用 `double`，显著提升精度。
* 改进对 JDBC MySQL 驱动的支持。
* 支持 [jemalloc](https://github.com/jemalloc/jemalloc) 的 `DEBUG malloc_stats`。
* [optimize_cutoff](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#optimize_cutoff) 现可作为单表设置，在 CREATE 或 ALTER 表时指定。
* **⚠️ 重大变更**：[query_log_format](Server_settings/Searchd.md#query_log_format) 默认改为 **`sphinxql`**。习惯使用 `plain` 格式的用户需在配置文件加上 `query_log_format = plain`。
* 显著降低内存消耗：在使用存储字段进行长时间高强度插入/替换/优化工作负载时，Manticore 现在显著减少 RAM 占用。
* [shutdown_timeout](Server_settings/Searchd.md#shutdown_timeout) 默认值从 3 秒提升至 60 秒。

* [Commit ffd0](https://github.com/manticoresoftware/manticoresearch/commit/ffd0499d329d2c383f14c8a44c4cc84338ab56e7) 支持 Java mysql connector >= 6.0.3：Java mysql 连接器 6.0.3 版本改变了连接 MySQL 的方式，导致与 Manticore 不兼容，新版本已支持该行为。
* [Commit 1da6](https://github.com/manticoresoftware/manticoresearch/commit/1da6dbec) 禁止在加载索引时（例如 searchd 启动）保存新的磁盘块。
* [Issue #746](https://github.com/manticoresoftware/manticoresearch/issues/746) 支持 glibc >= 2.34。
* [Issue #784](https://github.com/manticoresoftware/manticoresearch/issues/784) 将 'VIP' 连接和普通（非 VIP）连接分别计数。此前 VIP 连接计入 `max_connections` 限制，可能导致非 VIP 连接错误“连接数已达上限”。现在 VIP 连接不计入此限制。当前 VIP 连接数可通过 `SHOW STATUS` 和 `status` 查看。
* [ID](Creating_a_table/Data_types.md#Document-ID) 现可显式指定。
* [Issue #687](https://github.com/manticoresoftware/manticoresearch/issues/687) 支持 mysql 协议的 zstd 压缩。
### ⚠️ 其他次要重大变更
* ⚠️ BM25F 计算公式进行轻微更新以提升搜索相关度。仅影响使用函数 [BM25F()](Functions/Searching_and_ranking_functions.md#BM25F%28%29) 的搜索结果，不改变默认排名公式行为。
* ⚠️ REST [/sql](Connecting_to_the_server/HTTP.md#mode=raw) 接口行为改变：`/sql?mode=raw` 现在需要进行转义，并返回数组。

* ⚠️ `/bulk` INSERT/REPLACE/DELETE 请求响应格式变更：
  - 之前每个子查询为独立事务，分别响应
  - 现在整个批次视为单个事务，仅返回单一响应
* ⚠️ 搜索选项 `low_priority` 和 `boolean_simplify` 现在必须带值（`0/1`）。此前可写 `SELECT ... OPTION low_priority, boolean_simplify`，现在需写 `SELECT ... OPTION low_priority=1, boolean_simplify=1`。
* ⚠️ 若使用旧版 [php](https://github.com/manticoresoftware/manticoresearch/blob/master/api/sphinxapi.php)、[python](https://github.com/manticoresoftware/manticoresearch/blob/master/api/sphinxapi.py) 或 [java](https://github.com/manticoresoftware/manticoresearch/tree/master/api/java) 客户端，请访问对应链接下载更新版本。**旧版与 Manticore 5 兼容性不足。**
* ⚠️ 使用 `query_log_format=sphinxql` 模式时，HTTP JSON 请求日志格式发生变化。此前仅记录全文本部分，现在完整记录。
### 新发行包
* **⚠️ 重大变更**：因新结构，升级到 Manticore 5 推荐先卸载旧版包再安装新包：
  - 基于 RPM：`yum remove manticore*`

  - Debian 和 Ubuntu：`apt remove manticore*`
* 新的 deb/rpm 包结构。之前版本提供：
  - 包含 `searchd`（主搜索守护进程）及相关内容的 `manticore-server`
  - 包含 `indexer` 和 `indextool` 的 `manticore-tools`
  - 包含全部内容的 `manticore`
  - 作为元包，引用 `manticore-server` 和 `manticore-tools` 的 `manticore-all` RPM
  新结构为：
  - `manticore` — deb/rpm 元包，安装上述所有内容作为依赖
  - `manticore-server-core` — 单独运行 `searchd` 及相关组件
  - `manticore-server` — systemd 文件和其他辅助脚本

  - `manticore-tools` — `indexer`、`indextool` 等工具
  - `manticore-common` — 默认配置文件、默认数据目录、默认停用词
  - `manticore-icudata`、`manticore-dev`、`manticore-converter` 变化不大
  - 包括所有包的 `.tgz` 压缩包
* 支持 Ubuntu Jammy
* 通过 [YUM 源](Installation/RHEL_and_Centos.md#YUM-repository) 支持 Amazon Linux 2
### 修复的 Bug
* [Issue #815](https://github.com/manticoresoftware/manticoresearch/issues/815) 使用 UDF 函数时随机崩溃问题
* [Issue #287](https://github.com/manticoresoftware/manticoresearch/issues/287) 创建 RT 索引时内存溢出问题
* [Issue #604](https://github.com/manticoresoftware/manticoresearch/issues/604) 重大变更 3.6.0，4.2.0 sphinxql-parser
* [Issue #667](https://github.com/manticoresoftware/manticoresearch/issues/667) 致命错误：内存不足（无法分配 9007199254740992 字节）
* [Issue #676](https://github.com/manticoresoftware/manticoresearch/issues/676) 字符串未正确传递给 UDFs
* ❗[Issue #698](https://github.com/manticoresoftware/manticoresearch/issues/698) 尝试向 rt 索引添加文本列后 searchd 崩溃
* [Issue #705](https://github.com/manticoresoftware/manticoresearch/issues/705) 索引器无法找到所有列
* ❗[Issue #709](https://github.com/manticoresoftware/manticoresearch/issues/705) 按 json.boolean 分组工作异常
* [Issue #716](https://github.com/manticoresoftware/manticoresearch/issues/716) 与索引相关的 indextool 命令（如 --dumpdict）失败
* ❗[Issue #724](https://github.com/manticoresoftware/manticoresearch/issues/724) 字段在选择中消失
* [Issue #727](https://github.com/manticoresoftware/manticoresearch/issues/727) 使用 `application/x-ndjson` 时 .NET HttpClient Content-Type 不兼容
* [Issue #729](https://github.com/manticoresoftware/manticoresearch/issues/729) 字段长度计算
* ❗[Issue #730](https://github.com/manticoresoftware/manticoresearch/issues/730) 创建/插入到/删除列式表存在内存泄漏
* [Issue #731](https://github.com/manticoresoftware/manticoresearch/issues/731) 在某些条件下结果中的空列
* ❗[Issue #749](https://github.com/manticoresoftware/manticoresearch/issues/749) 守护进程启动时崩溃
* ❗[Issue #750](https://github.com/manticoresoftware/manticoresearch/issues/750) 守护进程启动时挂起
* ❗[Issue #751](https://github.com/manticoresoftware/manticoresearch/issues/751) SST 时崩溃
* [Issue #752](https://github.com/manticoresoftware/manticoresearch/issues/752) 当 engine='columnar' 时，json 属性标记为列式
* [Issue #753](https://github.com/manticoresoftware/manticoresearch/issues/753) 复制监听 0 端口
* [Issue #754](https://github.com/manticoresoftware/manticoresearch/issues/754) columnar_attrs = * 与 csvpipe 不兼容
* ❗[Issue #755](https://github.com/manticoresoftware/manticoresearch/issues/755) 在 rt 中对列式选择 float 时崩溃
* ❗[Issue #756](https://github.com/manticoresoftware/manticoresearch/issues/756) Indextool 在检查过程中修改 rt 索引
* [Issue #757](https://github.com/manticoresoftware/manticoresearch/issues/757) 需要检测监听端口范围重叠
* [Issue #758](https://github.com/manticoresoftware/manticoresearch/issues/758) RT 索引保存磁盘块失败时记录原始错误
* [Issue #759](https://github.com/manticoresoftware/manticoresearch/issues/759) RE2 配置只报告一个错误
* ❗[Issue #760](https://github.com/manticoresoftware/manticoresearch/issues/760) 5463778558586d2508697fa82e71d657ac36510f 提交导致 RAM 消耗变化
* [Issue #761](https://github.com/manticoresoftware/manticoresearch/issues/761) 第3节点脏重启后没有成为非主集群
* [Issue #762](https://github.com/manticoresoftware/manticoresearch/issues/762) 更新计数器增加了2
* [Issue #763](https://github.com/manticoresoftware/manticoresearch/issues/763) 新版本 4.2.1 损坏了使用形态学的 4.2.0 创建的索引
* [Issue #764](https://github.com/manticoresoftware/manticoresearch/issues/764) json key /sql?mode=raw 中无转义
* ❗[Issue #765](https://github.com/manticoresoftware/manticoresearch/issues/765) 使用函数隐藏了其他值
* ❗[Issue #766](https://github.com/manticoresoftware/manticoresearch/issues/766) FixupAttrForNetwork 中一行代码触发内存泄漏
* ❗[Issue #767](https://github.com/manticoresoftware/manticoresearch/issues/767) 4.2.0 和 4.2.1 中与 docstore 缓存相关的内存泄漏
* [Issue #768](https://github.com/manticoresoftware/manticoresearch/issues/768) 网络上存储字段的奇怪乒乓行为
* [Issue #769](https://github.com/manticoresoftware/manticoresearch/issues/769) 如果 'common' 部分未提及，lemmatizer_base 重置为空
* [Issue #770](https://github.com/manticoresoftware/manticoresearch/issues/770) pseudo_sharding 让按 id 查询变慢
* [Issue #771](https://github.com/manticoresoftware/manticoresearch/issues/771) 使用 jemalloc 时 DEBUG malloc_stats 输出为零
* [Issue #772](https://github.com/manticoresoftware/manticoresearch/issues/772) 删除/添加列导致值不可见
* [Issue #773](https://github.com/manticoresoftware/manticoresearch/issues/773) 无法向列式表添加 bit(N) 列
* [Issue #774](https://github.com/manticoresoftware/manticoresearch/issues/774) manticore.json 中启动时 "cluster" 变为空

* ❗[Commit 1da4](https://github.com/manticoresoftware/manticoresearch/commits/1da4ce891e4ff13727cfd3331d72771c64ee7949) HTTP 操作未在 SHOW STATUS 中跟踪

* [Commit 3810](https://github.com/manticoresoftware/manticoresearch/commits/381000ab4af4c3c98ec8c730699ad7f39039cec8) 针对低频单关键字查询禁用 pseudo_sharding
* [Commit 8003](https://github.com/manticoresoftware/manticoresearch/commits/800325cca283a87801b28929d82420fafc76a0ee) 修复存储属性与索引合并问题

* [Commit cddf](https://github.com/manticoresoftware/manticoresearch/commits/cddfeed6296f2041c6aae18414e5cd9cabf08281) 泛化了唯一值提取器；添加了针对列式字符串的专用唯一值提取器

* [Commit fba4](https://github.com/manticoresoftware/manticoresearch/commits/fba4bb4f) 修复从 docstore 获取 null 整数属性的问题

* [Commit f300](https://github.com/manticoresoftware/manticoresearch/commit/f3009a9242fa16956adf871c4887e6e0303ba364) `ranker` 可能在查询日志中被指定两次
## 版本 4.2.0，2021年12月23日

### 主要新特性
* **对实时索引和全文查询的伪分片支持**。在之前的版本中，我们添加了有限的伪分片支持。从这个版本开始，你只需启用[searchd.pseudo_sharding](Server_settings/Searchd.md#pseudo_sharding)，即可充分利用伪分片和多核处理器的所有优势。最棒的是，你不需要对索引或查询做任何改动，只需启用该功能，如果有空闲的CPU资源，它就会被利用以降低响应时间。它支持全文、过滤和分析查询的普通索引和实时索引。例如，启用伪分片能够使大多数查询的**平均响应时间降低约10倍**，数据集基于[Hacker news curated comments dataset](https://zenodo.org/record/45901/)，并乘以100倍（普通索引中1.16亿条文档）。

![Pseudo sharding on vs off in 4.2.0](4.2.0_ps_on_vs_off.png)

* 现在支持[**Debian Bullseye**](https://manticoresearch.com/install/)。

<!-- example pq_transactions_4.2.0 -->
* PQ事务现在是原子和隔离的。之前PQ事务支持有限。它使得**对PQ执行REPLACE操作更快**，特别是在需要一次替换大量规则时。性能细节：

<!-- intro -->
#### 4.0.2
<!-- request 4.0.2 -->
插入100万条PQ规则需要**48秒**，而以每批1万条替换4万条规则需要**406秒**。
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
插入100万条PQ规则需要**34秒**，而以每批1万条替换这些规则只需**23秒**。
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
### 次要变化
* [optimize_cutoff](Server_settings/Searchd.md#optimize_cutoff) 现在作为配置选项可用于 `searchd` 部分。当你想要全局限制所有索引中RT块的数量时，它非常有用。
* [提交 0087](https://github.com/manticoresoftware/manticoresearch/commit/00874743823eaf43e2a09a088392fd7e0a617f47) 对多个具有相同字段集/顺序的本地物理索引（实时/普通）进行准确的[count(distinct ...)](Searching/Grouping.md#COUNT%28DISTINCT-field%29) 和 [FACET ... distinct](Searching/Faceted_search.md#Faceting-without-duplicates) 支持。
* [PR #598](https://github.com/manticoresoftware/manticoresearch/pull/598) 为 `YEAR()` 及其他时间戳函数添加 bigint 支持。

* [提交 8e85](https://github.com/manticoresoftware/manticoresearch/commit/8e85d4bce596cc88f9d9615ea243a27e4afde0f7) 自适应 [rt_mem_limit](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_mem_limit)。之前 Manticore Search 在保存新磁盘块之前，恰好收集到 `rt_mem_limit` 的数据量，并且在保存时仍会收集多达10%（即双缓冲）的额外数据以最小化插入暂停的可能性。如果该限制也被耗尽，添加新文档就会阻塞直到磁盘块完全写入磁盘。新的自适应限制基于我们现有的[自动优化](Server_settings/Searchd.md#auto_optimize)功能，因此磁盘块不必完全遵守 `rt_mem_limit` 并且可以更早开始刷新。现在，我们收集高达 `rt_mem_limit` 的50%并将其保存为磁盘块。保存时，我们查看统计数据（保存了多少文档，保存期间有多少新文档进入），并重新计算下一次将使用的初始率。例如，如果保存了9000万文档，且保存时又进入了1000万文档，则率为90%，表示下一次可以收集高达 `rt_mem_limit` 的90% 才开始刷新另一个磁盘块。该率值自动计算，范围从33.3%到95%。
* [Issue #628](https://github.com/manticoresoftware/manticoresearch/issues/628) PostgreSQL源支持[unpack_zlib](Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Database_connection.md#unpack_zlib)。感谢 [Dmitry Voronin](https://github.com/dimv36) 的[贡献](https://github.com/manticoresoftware/manticoresearch/pull/630)。
* [提交 6d54](https://github.com/manticoresoftware/manticoresearch/commit/6d54cf2b319b131970c29410cc21abcbdf8083b1) `indexer -v` 和 `--version`。之前你仍可查看索引器版本，但不支持 `-v`/`--version`。
* [Issue #662](https://github.com/manticoresoftware/manticoresearch/issues/662) 当通过 systemd 启动 Manticore 时默认无限 mlock 限制。
* [提交 63c8](https://github.com/manticoresoftware/manticoresearch/commit/63c8cd05cf399e705c2c14939411a92cb784735a) 自旋锁 -> 协程 rwlock 的操作队列。

* [提交 4113](https://github.com/manticoresoftware/manticoresearch/commit/41130ce30d1734bbf8e6b8b63c4e815caf9464f2) 环境变量 `MANTICORE_TRACK_RT_ERRORS`，有助于调试实时段错误。
### 破坏性变更
* Binlog 版本已升级，之前版本的binlog不能回放，所以升级时请确保干净停止 Manticore Search：停止前实例后 `/var/lib/manticore/binlog/` 目录内应只剩下 `binlog.meta` 文件。
* [提交 3f65](https://github.com/manticoresoftware/manticoresearch/commit/3f659f36e87d99ee262205a8eae4663a255be630) 在 `show threads option format=all` 中新增列 "chain"。它显示部分任务信息票据的堆栈，对性能分析非常有用，若你解析 `show threads` 输出请注意新增列。
* `searchd.workers` 自3.5.0起被废弃，现在正式弃用。如果配置文件中仍有该项，启动时会触发警告。Manticore Search 会启动，但会显示警告。
* 如果你使用PHP和PDO访问Manticore，需要设置 `PDO::ATTR_EMULATE_PREPARES`。
### Bug修复
* ❗[Issue #650](https://github.com/manticoresoftware/manticoresearch/issues/650) Manticore 4.0.2比3.6.3慢。4.0.2在批量插入方面比之前版本更快，但单文档插入显著变慢。此问题已在4.2.0中修复。
* ❗[Commit 22f4](https://github.com/manticoresoftware/manticoresearch/commit/22f4141bed1411c83684ae6e0bae832071cce3df) RT索引在高强度REPLACE负载下可能会被损坏，或者可能会崩溃
* [Commit 03be](https://github.com/manticoresoftware/manticoresearch/commit/03be91e4f720a3f20bee0278dd8f0d44abd3ef0e) 修复在合并groupers和group N排序器时的平均值计算；修复聚合的合并
* [Commit 2ea5](https://github.com/manticoresoftware/manticoresearch/commit/2ea575d34efe38661f14d858f2b552456f29fe42) `indextool --check` 可能会崩溃
* [Commit 7ec7](https://github.com/manticoresoftware/manticoresearch/commit/7ec76d4abf01b5fca6f10a0000fe64d5d8e5fa52) 因UPDATE引起的内存耗尽问题
* [Commit 658a](https://github.com/manticoresoftware/manticoresearch/commit/658a727e41f3c3fbcde9754149528f34ed76ba19) 守护进程在INSERT时可能会挂起
* [Commit 46e4](https://github.com/manticoresoftware/manticoresearch/commit/46e42b9b386e31e53247ed634e158b533e28063e) 守护进程在关闭时可能会挂起
* [Commit f8d7](https://github.com/manticoresoftware/manticoresearch/commit/f8d7d5177cc83d250b493b4cfc619732b29cf072) 守护进程在关闭时可能会崩溃
* [Commit 733a](https://github.com/manticoresoftware/manticoresearch/commit/733accf165142a1d1574689525ccca8907971158) 守护进程在崩溃时可能会挂起
* [Commit f7f8](https://github.com/manticoresoftware/manticoresearch/commit/f7f8bd8c0dbb23a59a88502dcba16b0918fb2330) 守护进程在尝试用无效节点列表重加入集群时可能会崩溃启动
* [Commit 1401](https://github.com/manticoresoftware/manticoresearch/commit/140155613821ecaddcd1a3dfc4f7369b0df6aef7) 分布式索引在RT模式下，如果启动时无法解析其一个代理，可能会被完全遗忘
* [Issue #683](https://github.com/manticoresoftware/manticoresearch/issues/683) attr bit(N) engine='columnar' 失败
* [Issue #682](https://github.com/manticoresoftware/manticoresearch/issues/682) 创建表失败，但留下目录
* [Issue #663](https://github.com/manticoresoftware/manticoresearch/issues/663) 配置失败，错误消息：未知的键名 'attr_update_reserve'
* [Issue #632](https://github.com/manticoresoftware/manticoresearch/issues/632) Manticore在批量查询时崩溃

* [Issue #679](https://github.com/manticoresoftware/manticoresearch/issues/679) 批量查询在v4.0.3版本中再次导致崩溃

* [Commit f7f8](https://github.com/manticoresoftware/manticoresearch/commit/f7f8bd8c0dbb23a59a88502dcba16b0918fb2330) 修复守护进程在启动时尝试用无效节点列表重新加入集群时崩溃的问题
* [Issue #643](https://github.com/manticoresoftware/manticoresearch/issues/643) Manticore 4.0.2在批量插入后不接受连接
* [Issue #635](https://github.com/manticoresoftware/manticoresearch/issues/635) 带有ORDER BY JSON.field或字符串属性的FACET查询可能崩溃
* [Issue #634](https://github.com/manticoresoftware/manticoresearch/issues/634) 使用packedfactors查询时崩溃SIGSEGV
* [Commit 4165](https://github.com/manticoresoftware/manticoresearch/commits/41657f15f8fcf1e93cfad7f835bcf705d6e8349e) morphology_skip_fields 不被create table支持
## 版本 4.0.2，2021年9月21日
### 主要新功能
- **完全支持 [Manticore列存库](https://github.com/manticoresoftware/columnar)**。之前Manticore列存库仅支持纯索引。现在支持：
  - 在实时索引中进行 `INSERT`、`REPLACE`、`DELETE`、`OPTIMIZE`

  - 在复制中

  - 在 `ALTER`
  - 在 `indextool --check`
- **自动索引压缩**（[Issue #478](https://github.com/manticoresoftware/manticoresearch/issues/478)）。最终，你无需再手动调用OPTIMIZE或通过cron任务或其他自动化方式。Manticore现在默认自动完成该操作。你可以通过全局变量 [optimize_cutoff](Server_settings/Setting_variables_online.md) 设置默认的压缩阈值。
- **块快照和锁定系统重构**。这些变化初看可能对外不可见，但它们显著改善了实时索引中许多操作的行为。简而言之，之前大多数Manticore数据操作高度依赖锁，现在改用磁盘块快照。
- **显著提升实时索引的批量INSERT性能**。例如，在带有SSD、128GB内存和AMD Ryzen™ 9 5950X (16*2核心)的[Hetzner AX101服务器](https://www.hetzner.com/dedicated-rootserver/ax101)上，**在3.6.0版本你每秒可插入236K文档**，表结构为 `name text, email string, description text, age int, active bit(1)` （默认 `rt_mem_limit`，批量大小25000，16并发插入工作线程，整体插入1600万文档）。在4.0.2中，相同的并发/批量/文档数下，性能提升到 **每秒357K文档**。
  <details>
  - 读操作（例如SELECT、复制）通过快照执行

  - 仅更改内部索引结构且不修改模式/文档的操作（例如合并RAM段、保存磁盘块、合并磁盘块）通过只读快照执行，最终替换现有块

  - UPDATE和DELETE针对现有块执行，但如果正在合并，则收集写入操作，随后应用到新块
  - UPDATE为每个块顺序获取独占锁。合并在收集块属性阶段获取共享锁。因此同一时间只能有一个（合并或更新）操作访问块的属性。

  - 当合并进入需要属性的阶段时，会设置一个特殊标志。UPDATE完成时检查该标志，如果设置，则将整个更新存入特殊集合。最终合并完成时，将更新应用到新生成的磁盘块。
  - ALTER通过独占锁执行
  - 复制作为普通读操作运行，但额外在执行SST之前保存属性，并在SST过程中禁止更新
  </details>
- **[ALTER](Updating_table_schema_and_settings.md) 可以添加/删除全文字段**（在 RT 模式下）。之前只能添加/删除属性。


- 🔬 **实验性功能：用于全扫描查询的伪分片** - 允许并行化任何非全文搜索查询。不再需要手动准备分片，只需启用新选项 [searchd.pseudo_sharding](Server_settings/Searchd.md#pseudo_sharding)，即可期望非全文搜索查询的响应时间减少最多达到 `CPU cores` 倍。注意它可能会占用所有现有 CPU 核心，所以如果你不仅关心延迟，还关心吞吐量，请谨慎使用。
### 小更新

<!-- example -->
- 通过 [APT 仓库](Installation/Debian_and_Ubuntu.md#APT-repository) 支持 Linux Mint 和 Ubuntu Hirsute Hippo
- 在某些情况下，大型索引中的通过 id 进行 HTTP 更新更快（依赖于 id 的分布）

- [671e65a2](https://github.com/manticoresoftware/lemmatizer-uk/commit/671e65a296800537123a3f8b324eeaa3f58b9632) - 为 lemmatizer-uk 添加了缓存
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
- [systemd 的自定义启动标志](Starting_the_server/Linux.md#Custom-startup-flags-using-systemd)。现在，如果你需要用特定的启动标志运行 Manticore，就不需要手动启动 searchd 了

- 新函数 [LEVENSHTEIN()](Functions/String_functions.md#LEVENSHTEIN%28%29)，用于计算 Levenshtein 距离
- 新增 [searchd 启动标志](Starting_the_server/Manually.md#searchd-command-line-options) `--replay-flags=ignore-trx-errors` 和 `--replay-flags=ignore-all-errors`，这样在二进制日志损坏时仍可以启动 searchd
- [Issue #621](https://github.com/manticoresoftware/manticoresearch/issues/621) - 暴露 RE2 的错误
- 更准确的分布式索引由本地普通索引组成时的 [COUNT(DISTINCT)](Searching/Grouping.md#COUNT%28DISTINCT-field%29)
- [FACET DISTINCT](Searching/Faceted_search.md#Faceting-without-duplicates) 可在分面搜索时去重
- [精确形态修饰符](Searching/Full_text_matching/Operators.md#Exact-form-modifier) 现在不再需要[morphology](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology)，且支持启用[infix/prefix](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology)搜索的索引
### 重大变更
- 新版本可以读取旧索引，但旧版本无法读取 Manticore 4 的索引

- 移除了对 id 的隐式排序。如有需要，请明确排序
- `charset_table` 的默认值由 `0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451` 改为 `non_cjk`
- `OPTIMIZE` 现在会自动执行。如果不需要，请确保在配置文件的 `searchd` 部分设置 `auto_optimize=0`
- [Issue #616](https://github.com/manticoresoftware/manticoresearch/issues/616) `ondisk_attrs_default` 被弃用，现在已移除
- 对于开发者：我们现在使用 Clang 编译器构建 Linux 版本，经测试可以编译出更快的 Manticore Search 和 Manticore Columnar Library
- 如果搜索查询中未指定 [max_matches](Searching/Options.md#max_matches)，则会隐式更新为所需的最低值，以优化新列存储的性能。这可能影响 [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META) 中的 `total` 指标，但不影响实际找到文档数的 `total_found`
### 从 Manticore 3 迁移
- 确保已经干净地停止了 Manticore 3：
  - `/var/lib/manticore/binlog/` 中不应该有二进制日志文件（目录中只应有 `binlog.meta`）

  - 否则 Manticore 4 无法回复这些二进制日志对应的索引将不会运行
- 新版本可以读取旧索引，但旧版本无法读取 Manticore 4 的索引，确保备份以便需要时可回滚新版本
- 如果运行复制集群，请确保：
  - 先干净地停止所有节点
  - 然后以 `--new-cluster` 启动最后停止的节点（Linux 下使用工具 `manticore_new_cluster`）
  - 更多详情请阅读 [重启集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster)
### Bug 修复
- 修复了大量复制相关问题：
  - [Commit 696f](https://github.com/manticoresoftware/manticoresearch/commit/696f8649535f2b5285b878c9581c0d2b1139ae09) - 修复带有活动索引的 JOINER 进行 SST 过程中的崩溃；在 JOINER 节点写文件块时添加 sha1 校验以加快索引加载；在索引加载时添加变更索引文件的轮换；当活动索引被捐赠节点的新索引替换时，添加删除索引文件操作；在捐赠节点添加发送文件和块的复制日志点
  - [Commit b296](https://github.com/manticoresoftware/manticoresearch/commit/b296c55af68314a8fc66f8b104e7301d673c3b68) - 地址不正确时 JOIN CLUSTER 会崩溃
  - [Commit 418b](https://github.com/manticoresoftware/manticoresearch/commit/418bf880a5e05ffbb68f311a1c23006a9df3220e) - 大索引初始复制时，加入节点可能因 `ERROR 1064 (42000): invalid GTID, (null)` 失败，且捐赠节点在其他节点加入时可能无响应
  - [Commit 6fd3](https://github.com/manticoresoftware/manticoresearch/commit/6fd350d28e4905211cf8a7298cbf7cdda101c83d) - 大索引的哈希计算错误，导致复制失败
  - [Issue #615](https://github.com/manticoresoftware/manticoresearch/issues/615) - 复制在集群重启时失败
- [Issue #574](https://github.com/manticoresoftware/manticoresearch/issues/574) - `indextool --help` 不显示参数 `--rotate`
- [Issue #578](https://github.com/manticoresoftware/manticoresearch/issues/578) - searchd 空闲约一天后 CPU 占用过高
- [Issue #587](https://github.com/manticoresoftware/manticoresearch/issues/587) - 立即刷新 .meta
- [问题 #617](https://github.com/manticoresoftware/manticoresearch/issues/617) - manticore.json 被清空
- [问题 #618](https://github.com/manticoresoftware/manticoresearch/issues/618) - searchd --stopwait 在 root 下失败。同时修复了 systemctl 行为（之前 ExecStop 显示失败，并且未能足够等待 searchd 正确停止）
- [问题 #619](https://github.com/manticoresoftware/manticoresearch/issues/619) - INSERT/REPLACE/DELETE 与 SHOW STATUS。`command_insert`、`command_replace` 等显示了错误的指标
- [问题 #620](https://github.com/manticoresoftware/manticoresearch/issues/620) - plain 索引的 `charset_table`  默认值错误
- [提交 8f75](https://github.com/manticoresoftware/manticoresearch/commit/8f7536887d4b58a5bab5647fc067b2d0482c2038) - 新的磁盘块没有被 mlocked
- [问题 #607](https://github.com/manticoresoftware/manticoresearch/issues/607) - 当无法通过名称解析节点时，Manticore 集群节点崩溃


- [问题 #623](https://github.com/manticoresoftware/manticoresearch/issues/623) - 更新索引的复制可能导致未定义状态
- [提交 ca03](https://github.com/manticoresoftware/manticoresearch/commit/ca03d2280c5197e9b311f0f03f551e3b702a8130) - indexer 在索引带有 json 属性的 plain 索引源时可能挂起

- [提交 53c7](https://github.com/manticoresoftware/manticoresearch/commit/53c753054558ff3f82c7af365273d32537c5338b) - 修复 PQ 索引中的不等表达式过滤器
- [提交 ccf9](https://github.com/manticoresoftware/manticoresearch/commit/ccf94e022ba6763d6286ab5bac767b6f6a846846) - 修复了针对列表查询中超过 1000 个匹配时的选择窗口。之前 `SELECT * FROM pq ORDER BY id desc LIMIT 1000 , 100 OPTION max_matches=1100` 无法正常工作
- [提交 a048](https://github.com/manticoresoftware/manticoresearch/commit/a0483fe9bef3c45dffe96f5df4f4382aee29c072) - 发送到 Manticore 的 HTTPS 请求可能导致类似“max packet size(8388608) exceeded”的警告
- [问题 #648](https://github.com/manticoresoftware/manticoresearch/issues/648) - Manticore 3 在进行几次字符串属性更新后可能挂起

## 版本 3.6.0，2021 年 5 月 3 日
**Manticore 4 发布前的维护版本**
### 主要新功能
- 支持 plain 索引的 [Manticore Columnar Library](https://github.com/manticoresoftware/columnar/)。plain 索引新增设置项 [columnar_attrs](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#columnar_attrs)
- 支持 [乌克兰语形态分析器](https://github.com/manticoresoftware/lemmatizer-uk)
- 完全重写了直方图。在构建索引时，Manticore 也为每个字段构建直方图，用于更快的过滤。3.6.0 版本中该算法已彻底修订，如果您拥有大量数据并进行大量过滤，将获得更高性能。
### 次要更改
- 工具 `manticore_new_cluster [--force]`，在 systemd 环境下用于重启复制集群
- `indexer --merge` 添加了 [--drop-src](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Merging_tables.md#Merging-tables) 选项

- 新模式 [blend_mode='trim_all'](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology)
- 添加了用反引号转义 JSON 路径的支持 [support for escaping JSON path](Searching/Full_text_matching/Escaping.md#Escaping-json-node-names-in-SQL)
- [indextool --check](Miscellaneous_tools.md#indextool) 可在 RT 模式下运行

- SELECT/UPDATE 支持 [FORCE/IGNORE INDEX(id)](Data_creation_and_modification/Updating_documents/UPDATE.md#Updates-via-SQL)
- 合并后的磁盘块的块 ID 现在唯一
- [indextool --check-disk-chunk CHUNK_NAME](Miscellaneous_tools.md#indextool)
### 优化
- [更快的 JSON 解析](https://github.com/manticoresoftware/manticoresearch/commit/0c25fc1788e3f77f60cb7fb8f1623d8466bba606)，我们的测试显示，像 `WHERE json.a = 1` 的查询延迟降低了 3-4%
- 非公开命令 `DEBUG SPLIT` 作为自动分片/重新平衡的前提条件
### Bug 修复
- [问题 #584](https://github.com/manticoresoftware/manticoresearch/issues/524) - FACET 结果不准确且不稳定
- [问题 #506](https://github.com/manticoresoftware/manticoresearch/issues/506) - 使用 MATCH 时出现异常行为。受此问题影响的用户需要重建索引，因为问题出在构建索引阶段
- [问题 #387](https://github.com/manticoresoftware/manticoresearch/issues/387) - 运行带 SNIPPET() 函数的查询时间歇性核心崩溃
- 堆栈优化，利于处理复杂查询：
  - [问题 #469](https://github.com/manticoresoftware/manticoresearch/issues/469) - SELECT 导致崩溃转储
  - [e8420cc7](https://github.com/manticoresoftware/manticoresearch/tree/e8420cc789149c84731be0874be916e33b263bd6) - 过滤树的堆栈大小检测
- [问题 #461](https://github.com/manticoresoftware/manticoresearch/issues/461) - 使用 IN 条件执行更新时未正确生效
- [问题 #464](https://github.com/manticoresoftware/manticoresearch/issues/464) - CALL PQ 后立即执行 SHOW STATUS 返回异常 - [问题 #481](https://github.com/manticoresoftware/manticoresearch/issues/481) - 修复静态二进制构建
- [问题 #502](https://github.com/manticoresoftware/manticoresearch/issues/502) - 多查询中的错误
- [问题 #514](https://github.com/manticoresoftware/manticoresearch/issues/514) - 使用 'create table' 时无法使用不寻常的列名
- [提交 d1db](https://github.com/manticoresoftware/manticoresearch/commit/d1dbe771288328c963ca8512f564c6b7cf4f466c) - 重放带字符串属性更新的 binlog 时守护进程崩溃；将 binlog 版本设置为 10
- [提交 775d](https://github.com/manticoresoftware/manticoresearch/commit/775d0555562f121911d3c30085947e743fadac2d) - 修正了表达式堆栈帧检测运行时错误（测试 207）
- [提交 4795](https://github.com/manticoresoftware/manticoresearch/commit/4795dc49194c7745761bb5006ecb266e8a9224fa) - 对空存储查询时，渗透索引过滤器和标签为空（测试 369）
- [Commit c3f0](https://github.com/manticoresoftware/manticoresearch/commit/c3f0bf4dca0e4eae6ea899dd566878df57eba085)  - 修复了在网络延迟长且错误率高的情况下（不同数据中心复制）复制SST流程的中断；将复制命令版本更新为1.03
- [Commit ba2d](https://github.com/manticoresoftware/manticoresearch/commit/ba2d6619b8107f3fb177741bbdff3709bac248cd)  - 加入集群后，写操作时连接器锁住集群（测试385）
- [Commit de4d](https://github.com/manticoresoftware/manticoresearch/commit/de4dcb9ff6c316bc2a9ef6c62eb64769f04ee103)  - 具有精确修饰符的通配符匹配（测试321）
- [Commit 6524](https://github.com/manticoresoftware/manticoresearch/commit/6524fc6af032a0932bd42c37874a619e1fb7f075)  - docid检查点与文档存储的对比
- [Commit f4ab](https://github.com/manticoresoftware/manticoresearch/commit/f4ab83c291e65a5873d271c67b5491f629544f1d)  - 解析无效xml时索引器行为不一致
- [Commit 7b72](https://github.com/manticoresoftware/manticoresearch/commit/7b727e22fe01b1a180d4a8d3a3fea92c18eac707)  - 含NOTNEAR的存储回溯查询运行时间过长（测试349）
- [Commit 812d](https://github.com/manticoresoftware/manticoresearch/commit/812dab745f0c972ecde5d5b3d4a313cd6e1e143e)  - 以通配符开头的短语权重错误
- [Commit 1771](https://github.com/manticoresoftware/manticoresearch/commit/1771afc669c75670b3511bf02ab9bfc8982b94d2)  - 含通配符的回溯查询生成不带载荷的匹配词，导致交错匹配并破坏匹配（测试417）
- [Commit aa0d](https://github.com/manticoresoftware/manticoresearch/commit/aa0d8c2bcb41428d41dbfb66e42a872a79a4ee4b)  - 修正并行查询情况下“total”的计算
- [Commit 18d8](https://github.com/manticoresoftware/manticoresearch/commit/18d81b3cfbb930b7361dae645fe13283d4fbf6ef)  - Windows环境下守护进程多并发会话崩溃
- [Commit 8443](https://github.com/manticoresoftware/manticoresearch/commit/84432f23d6fbf7a7d0b67c8c9412eb5933b03596)  - 部分索引设置无法被复制
- [Commit 9341](https://github.com/manticoresoftware/manticoresearch/commit/93411fe6af4246ca4acce758944d513a1660b92b)  - 在高频率添加新事件时，因原子“kick”事件一次处理多事件，导致netloop偶尔冻结，从而丢失实际动作
查询状态，不是服务器状态
- [Commit d805](https://github.com/manticoresoftware/manticoresearch/commit/d805fc125244ef08d14fb3d1826e2c356862d539)  - 新的已刷新磁盘块在提交时可能丢失

- [Commit 63cb](https://github.com/manticoresoftware/manticoresearch/commit/63cbf008d50979cdf7062664fb02a672a3bb1131)  - profiler中“net_read”不准确
- [Commit f537](https://github.com/manticoresoftware/manticoresearch/commit/f5379bb27234b25919966c50ccb6f469ead646e2)  - 回溯在阿拉伯语（从右至左文本）中的问题
- [Commit 49ee](https://github.com/manticoresoftware/manticoresearch/commit/49eeb4202d4ce4875c07c35087d7108c1e177328)  - 重复列名时id未正确选取
- [Commit refa](https://github.com/manticoresoftware/manticoresearch/commit/4b7df262bbfec9b9e95a44e72ad326ac8a786c49) 修复网络事件的崩溃（罕见情况）
- [e8420cc7](https://github.com/manticoresoftware/manticoresearch/tree/e8420cc789149c84731be0874be916e33b263bd6) 修正 `indextool --dumpheader`
- [Commit ff71](https://github.com/manticoresoftware/manticoresearch/commit/ff7163532ed39042c0d3c3c2b902ff41f94d7f0c) - TRUNCATE WITH RECONFIGURE 在存储字段时工作异常

### 突破性变更：
- 新的binlog格式：升级前需先干净停止Manticore
- 索引格式稍有变动：新版本能正确读取现有索引，但如果降级回3.6.0之前的版本，则新索引将无法读取

- 复制格式变更：禁止从旧版本复制到3.6.0及反之，需一次性将所有节点切换到新版本

- `reverse_scan`已废弃。从3.6.0开始请确保查询中不使用该选项，否则会失败
- 本版本起不再提供RHEL6、Debian Jessie及Ubuntu Trusty的构建版本。如需保留支持，请[联系我们](https://manticoresearch.com/contact-us/)
### 弃用项
- 不再对id进行隐式排序。若依赖此特性，请相应更新查询

- 搜索选项`reverse_scan`已被弃用
## 版本 3.5.4，2020年12月10日
### 新特性

- 新的Python、Javascript和Java客户端现已正式发布，并在本手册中有详尽文档。
- 实时索引的磁盘块自动丢弃。该优化在优化实时索引时（[OPTIMIZing](Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE)），当磁盘块显然不再需要（所有文档均被删除）时自动丢弃该磁盘块。之前仍需合并，现在可立即丢弃。会忽略[cutoff](Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks)选项，即使未合并任何内容，过时的磁盘块也会被删除。适合保持索引中保留期限并删除旧文档的场景。压缩此类索引将更快速。
- [独立的NOT](Searching/Options.md#not_terms_only_allowed) 作为SELECT的选项

### 小改动
- [问题 #453](https://github.com/manticoresoftware/manticoresearch/issues/453) 新选项[indexer.ignore_non_plain=1](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#ignore_non_plain) 适用于运行 `indexer --all` 时配置文件中含有非纯索引时。没有设置 `ignore_non_plain=1` 会收到警告及相应退出码。
- [SHOW PLAN ... OPTION format=dot](Node_info_and_management/Profiling/Query_plan.md#Dot-format-for-SHOW-PLAN) 和 [EXPLAIN QUERY ... OPTION format=dot](Searching/Full_text_matching/Profiling.md#Profiling-without-running-a-query) 支持全文查询计划执行的可视化。对理解复杂查询很有帮助。
### 弃用
- `indexer --verbose` 已被弃用，因为它从未向索引器输出添加任何内容
- 用于导出 watchdog 的回溯信号现在应使用 `USR2`，而非 `USR1`
### Bug 修复
- [Issue #423](https://github.com/manticoresoftware/manticoresearch/issues/423) 西里尔字符句号调用片段保持模式不高亮
- [Issue #435](https://github.com/manticoresoftware/manticoresearch/issues/435) RTINDEX - GROUP N BY 表达式选择 = 致命崩溃
- [Commit 2b3b](https://github.com/manticoresoftware/manticoresearch/commit/2b3b62bd0ce2b54544733d524a169e81f71b5388) cluster 中 searchd 状态显示分段错误


- [Commit 9dd2](https://github.com/manticoresoftware/manticoresearch/commit/9dd25c193ed2a53750a0e0e68566fd2b833fcb0c) 'SHOW INDEX index.N SETTINGS' 不支持地址大于 9 的块

- [Issue #389](https://github.com/manticoresoftware/manticoresearch/issues/389) 导致 Manticore 崩溃的 Bug

- [Commit fba1](https://github.com/manticoresoftware/manticoresearch/commit/fba166178ff35c73a0c70d131cf3aca4a635e961) 转换器创建了损坏的索引
- [Commit eecd](https://github.com/manticoresoftware/manticoresearch/commit/eecd61d861c742471175be19a5e5455034b5db5f) stopword_step=0 与 CALL SNIPPETS()
- [Commit ea68](https://github.com/manticoresoftware/manticoresearch/commit/ea6850e449601199190f38a387c04b8a7b7e90f6) 本地索引中 max_matches 较低时 count distinct 返回 0
- [Commit 362f](https://github.com/manticoresoftware/manticoresearch/commit/362f27db3421f09a41c6cce04b5baace00fa4c4d) 使用聚合时返回结果中不包含存储文本
## 版本 3.5.2，2020年10月1日
### 新功能

* OPTIMIZE 将磁盘块数减少到一定数量（默认是 `2* 核心数`），而不是合并为单一一个。最佳块数可以通过 [cutoff](Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks) 选项控制。

* NOT 操作符现在可以独立使用。默认禁用，因为单独的 NOT 查询可能导致性能缓慢。可以通过设置新的 searchd 指令 [not_terms_only_allowed](Server_settings/Searchd.md#not_terms_only_allowed) 为 `0` 来启用。
* 新设置 [max_threads_per_query](Server_settings/Searchd.md#max_threads_per_query) 用于设定单个查询可以使用的线程数。如果不设置此指令，查询线程数最大值为 [threads](Server_settings/Searchd.md#threads) 指令的值。
每个 `SELECT` 查询可以通过 [OPTION threads=N](Searching/Options.md#threads) 来限制线程数，从而覆盖全局 `max_threads_per_query`。
* Percolate 索引现在支持通过 [IMPORT TABLE](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Importing_table.md) 导入。

* HTTP API `/search` 新增对 [分面](Searching/Faceted_search.md#HTTP-JSON)/[分组](Searching/Grouping.md) 的基本支持，通过新的查询节点 `aggs` 实现。

### 小改动

* 如果未声明复制监听指令，引擎会尝试使用定义的 'sphinx' 端口之后的端口，直到 200 端口。

* `listen=...:sphinx` 必须明确设置以支持 SphinxSE 连接或 SphinxAPI 客户端。

* [SHOW INDEX STATUS](Node_info_and_management/Table_settings_and_status/SHOW_TABLE_STATUS.md) 添加了新指标：`killed_documents`，`killed_rate`，`disk_mapped_doclists`，`disk_mapped_cached_doclists`，`disk_mapped_hitlists` 和 `disk_mapped_cached_hitlists`。

* SQL 命令 `status` 现在输出 `Queue\Threads` 和 `Tasks\Threads`。

### 弃用：
* `dist_threads` 完全弃用，searchd 若发现仍使用该指令会记录警告。
### Docker
官方 Docker 镜像现在基于 Ubuntu 20.04 LTS
### 打包
除了常规的 `manticore` 包，您还可以选择按组件安装 Manticore Search：

- `manticore-server-core` - 提供 `searchd`，手册页，日志目录，API 及 galera 模块。会自动安装依赖包 `manticore-common`。

- `manticore-server` - 提供核心自动化脚本（init.d，systemd），以及 `manticore_new_cluster` 包装器。依赖于 `manticore-server-core`。
- `manticore-common` - 提供配置文件、停用词、通用文档及骨架文件夹（datadir，modules 等）。
- `manticore-tools` - 提供辅助工具（如 `indexer`，`indextool` 等）、手册页及示例。依赖 `manticore-common`。
- `manticore-icudata`（RPM）或 `manticore-icudata-65l`（DEB） - 提供 ICU 数据文件用于 icu 词法分析。
- `manticore-devel`（RPM）或 `manticore-dev`（DEB） - 提供用于 UDF 开发的头文件。
### Bug 修复
1. [Commit 2a47](https://github.com/manticoresoftware/manticoresearch/commit/2a474dc1a26e8b0f8aaaae95669caf2f1d4b7746) RT 索引不同块时 grouper 守护进程崩溃
2. [Commit 57a1](https://github.com/manticoresoftware/manticoresearch/commit/57a19e5ad5663ef0ca7436595218fb1221d28c8e) 空远程文档快速路径修复
3. [Commit 07dd](https://github.com/manticoresoftware/manticoresearch/commit/07dd3f313c63fb82c22092f9907ef24e3475250e) 表达式堆栈帧运行时检测
4. [Commit 08ae](https://github.com/manticoresoftware/manticoresearch/commit/08ae357cf1012bc8e2da54c20b205b592efda3d4) percolate 索引支持超过 32 字段的匹配
5. [Commit 16b9](https://github.com/manticoresoftware/manticoresearch/commit/16b9390fd4cdb07a77ac4497adb935573a1710e5) 复制监听端口范围修复
6. [Commit 5fa6](https://github.com/manticoresoftware/manticoresearch/commit/5fa671affeacb6441a59c8a88479bfd423df7c81) pq 上支持 SHOW CREATE TABLE
7. [Commit 54d1](https://github.com/manticoresoftware/manticoresearch/commit/54d133b6449105a9fb0168db3f1fbb05fb5aa1f6) HTTPS 端口行为调整
8. [提交 fdbb](https://github.com/manticoresoftware/manticoresearch/commit/fdbbe5245cc296cc5c1ae3ae2fb9cb08fb66a248) 替换时混合docstore行
9. [提交 afb5](https://github.com/manticoresoftware/manticoresearch/commit/afb53f648ef4e64fa1776c58e66f6e716ac730ad) 将 TFO 不可用消息级别切换为“info”
10. [提交 59d9](https://github.com/manticoresoftware/manticoresearch/commit/59d94cefc6e88af360b8046726a16b7eaa9f0b71) strcmp 无效使用时崩溃
11. [提交 04af](https://github.com/manticoresoftware/manticoresearch/commit/04af0349c1e784c2230d204592d6272b440bb375) 向包含系统（停用词）文件的集群添加索引
12. [提交 5014](https://github.com/manticoresoftware/manticoresearch/commit/50148b4eae9f3a924082005cf5d2f9c8339af7f5) 合并拥有大词典的索引；RT 优化大磁盘块
13. [提交 a2ad](https://github.com/manticoresoftware/manticoresearch/commit/a2adf1582843f63d4dcc1dab8f7598e0917094a2) Indextool 可以导出当前版本的元信息

14. [提交 69f6](https://github.com/manticoresoftware/manticoresearch/commit/69f6d5f77c64b97e3f395aea8c555b11c77fb732) GROUP N 中分组排序问题

15. [提交 24d5](https://github.com/manticoresoftware/manticoresearch/commit/24d5d80f508576794b751d70f9f9b524564e3c14) 握手后显式刷新 SphinxSE
16. [提交 31c4](https://github.com/manticoresoftware/manticoresearch/commit/31c4d78ac03ee85d9a02b86075448a04ac80f9e2) 避免不必要时复制巨大描述
17. [提交 2959](https://github.com/manticoresoftware/manticoresearch/commit/2959e2caa8610f98fcdf2e5a82160f58687f4a27) show threads 中负时间
18. [提交 f0b3](https://github.com/manticoresoftware/manticoresearch/commit/f0b357102c70f3ac1d7da260c60b67322940e647) 令牌过滤插件与零位置增量问题
19. [提交 a49e](https://github.com/manticoresoftware/manticoresearch/commit/a49e5bc13ea763e6f01dd5993e891858a84d1846) 多次命中时将“FAIL”改为“WARNING”
## 版本 3.5.0，2020年7月22日
### 主要新特性：

* 由于我们正在努力将多任务模式从线程改为**协程（coroutines）**，所以此次版本发布耗时较长。这样配置更简单，查询并行化更加直接：Manticore 只使用指定数量的线程（参见新设置 [threads](Server_settings/Searchd.md#threads)），新模式确保以最优方式完成任务。

* [高亮功能](Searching/Highlighting.md#Highlighting-options)的变化：

  - 任何支持多个字段的高亮（`highlight({},'field1, field2'` 或 JSON 查询中的 `highlight`）默认按字段分别应用限制。

  - 任何支持纯文本的高亮（`highlight({}, string_attr)` 或 `snippet()`）默认对整个文档应用限制。
  - 可通过 `limits_per_field=0` 选项将[每字段限制](Searching/Highlighting.md#limits_per_field)切换为全局限制（默认是 `1`）。
  - 通过 HTTP JSON 进行高亮时，[allow_empty](Searching/Highlighting.md#allow_empty) 默认为 `0`。
* 现在同一端口[可以用于](Server_settings/Searchd.md#listen) http、https 和二进制 API（以接受远程 Manticore 实例的连接）。连接 mysql 协议仍需 `listen = *:mysql`。Manticore 现在自动检测客户端类型，MySQL 除外（因协议限制）。
* 在 RT 模式下，字段现在可以同时作为[文本和字符串属性](Creating_a_table/Data_types.md#String) — [GitHub issue #331](https://github.com/manticoresoftware/manticoresearch/issues/331)。
  在[普通模式](Read_this_first.md#Real-time-mode-vs-plain-mode)下称作 `sql_field_string`。现在在[RT 模式](Read_this_first.md#Real-time-mode-vs-plain-mode)中也可用，示例如下：
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
* 现在可[高亮字符串属性](Searching/Highlighting.md#Highlighting-via-SQL)。
* SQL 接口支持 SSL 和压缩
* 支持 mysql 客户端 [`status`](Node_info_and_management/Node_status.md#STATUS) 命令。
* [复制](Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Setting-up-replication)现可复制外部文件（停用词、例外等）。
* 通过 HTTP JSON 接口支持过滤操作符 [`in`](Searching/Filters.md#Set-filters)。
* HTTP JSON 中支持[`expressions`](Searching/Expressions.md#expressions)。
* 现在可实时更改 RT 模式的 [`rt_mem_limit`](https://github.com/manticoresoftware/manticoresearch/issues/344)，即可执行 `ALTER ... rt_mem_limit=<new value>`。

* 现在可使用[独立 CJK 字符集表](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology)：`chinese`、`japanese` 和 `korean`。
* [thread_stack](Server_settings/Searchd.md#thread_stack) 现在限制最大线程栈大小，而非初始大小。
* 改进了 `SHOW THREADS` 输出。
* `SHOW THREADS` 中显示长时间 `CALL PQ` 的进度。
* cpustat、iostat、coredump 可在运行时通过 [SET](Server_settings/Setting_variables_online.md#SET) 修改。

* 实现了 `SET [GLOBAL] wait_timeout=NUM`。
### 重大变动：
* **索引格式已改变。** 3.5.0 版本构建的索引不能被低于 3.5.0 的 Manticore 版本加载，但 3.5.0 版本可以识别旧格式。
* [`INSERT INTO PQ VALUES()`](Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md) （即不提供列列表）以前期望值精确为 `(query, tags)`。现在已更改为 `(id,query,tags,filters)`。如果希望自动生成 id，可以将其设置为 0。
* [`allow_empty=0`](Searching/Highlighting.md#allow_empty) 是通过 HTTP JSON 接口进行高亮显示的新默认值。
* 在 `CREATE TABLE`/`ALTER TABLE` 中，外部文件（停止词、例外等）只能使用绝对路径。
### 弃用事项：
* `ram_chunks_count` 在 `SHOW INDEX STATUS` 中重命名为 `ram_chunk_segments_count`。
* `workers` 已过时。现在只有一个工作模式。
* `dist_threads` 已过时。所有查询现在都尽可能并行（受限于 `threads` 和 `jobs_queue_size`）。
* `max_children` 已过时。使用 [threads](Server_settings/Searchd.md#threads) 来设置 Manticore 将使用的线程数（默认为 CPU 内核数）。
* `queue_max_length` 已过时。如果确实需要，请使用 [jobs_queue_size](Server_settings/Searchd.md#jobs_queue_size) 来微调内部作业队列大小（默认无限制）。
* 所有 `/json/*` 端点现在可无需 `/json/` 使用，例如 `/search`、`/insert`、`/delete`、`/pq` 等。
* `field` （意为“全文字段”）在 `describe` 中重命名为 “text”。
  <!-- more -->
  **3.4.2：**
  ```sql
  mysql> describe t;
  +-------+--------+----------------+
  | Field | Type   | Properties     |
  +-------+--------+----------------+
  | id    | bigint |                |
  | f     | field  | indexed stored |
  +-------+--------+----------------+
  ```
  **3.5.0：**
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
* 西里尔字母 `и` 不再映射为 `i` 在 `non_cjk` charset_table （默认值）中，因为这对俄语词干提取器和词形还原器影响过大。
* `read_timeout`。请使用控制读写的 [network_timeout](Server_settings/Searchd.md#network_timeout)。
### 软件包
* Ubuntu Focal 20.04 官方软件包
* deb 软件包名称由 `manticore-bin` 改为 `manticore`
### Bug修复：
1. [Issue #351](https://github.com/manticoresoftware/manticoresearch/issues/351) searchd 内存泄漏
2. [Commit ceab](https://github.com/manticoresoftware/manticoresearch/commit/ceabe44f) 代码段中出现轻微的越界读取
3. [Commit 1c3e](https://github.com/manticoresoftware/manticoresearch/commit/1c3e84a3) 崩溃查询中对局部变量的危险写入
4. [Commit 26e0](https://github.com/manticoresoftware/manticoresearch/commit/26e094ab) 测试 226 中 sorter 的轻微内存泄漏
5. [Commit d2c7](https://github.com/manticoresoftware/manticoresearch/commit/d2c7f86a) 测试 226 中严重的内存泄漏
6. [Commit 0dd8](https://github.com/manticoresoftware/manticoresearch/commit/0dd80122) 集群显示节点同步，但 `count(*)` 显示数量不同
7. [Commit f1c1](https://github.com/manticoresoftware/manticoresearch/commit/f1c1ac3f) 外观：日志中的重复且有时丢失的警告消息
8. [Commit f1c1](https://github.com/manticoresoftware/manticoresearch/commit/f1c1ac3f) 外观：日志中 (null) 索引名
9. [Commit 359d](https://github.com/manticoresoftware/manticoresearch/commit/359dbd30) 无法检索超过 7000 万结果
10. [Commit 19f3](https://github.com/manticoresoftware/manticoresearch/commit/19f328ee) 无法使用无列语法插入 PQ 规则
11. [Commit bf68](https://github.com/manticoresoftware/manticoresearch/commit/bf685d5d) 向集群内索引插入文档时误导性错误信息
12. [Commit 2cf1](https://github.com/manticoresoftware/manticoresearch/commit/2cf18c83) `/json/replace` 和 `json/update` 返回的 id 为指数形式
13. [Issue #324](https://github.com/manticoresoftware/manticoresearch/issues/324) 在同一查询中更新 json 标量属性和 mva
14. [Commit d384](https://github.com/manticoresoftware/manticoresearch/commit/d38409eb) `hitless_words` 在 RT 模式失效
15. [Commit 5813](https://github.com/manticoresoftware/manticoresearch/commit/5813d639) 不应允许在 rt 模式下使用 `ALTER RECONFIGURE`
16. [Commit 5813](https://github.com/manticoresoftware/manticoresearch/commit/5813d639) searchd 重启后 `rt_mem_limit` 被重置为 128M
17. highlight() 有时会挂起
18. [Commit 7cd8](https://github.com/manticoresoftware/manticoresearch/commit/7cd878f4) RT 模式中失败使用 U+编码
19. [Commit 2b21](https://github.com/manticoresoftware/manticoresearch/commit/2b213de4) RT 模式中失败使用通配符词形
20. [Commit e9d0](https://github.com/manticoresoftware/manticoresearch/commit/e9d07e68) 修正 `SHOW CREATE TABLE` 与多个词形文件冲突问题
21. [Commit fc90](https://github.com/manticoresoftware/manticoresearch/commit/fc90a84f) JSON 查询无 “query” 字段时导致 searchd 崩溃
22. Manticore [官方 Docker](https://hub.docker.com/r/manticoresearch/manticore) 无法从 mysql 8 索引
23. [Commit 23e0](https://github.com/manticoresoftware/manticoresearch/commit/23e05d32) HTTP /json/insert 现在必须提供 id
24. [Commit bd67](https://github.com/manticoresoftware/manticoresearch/commit/bd679af0) `SHOW CREATE TABLE` 不适用于 PQ
25. [Commit bd67](https://github.com/manticoresoftware/manticoresearch/commit/bd679af0) `CREATE TABLE LIKE` 对 PQ 支持不正确
26. [Commit 5eac](https://github.com/manticoresoftware/manticoresearch/commit/5eacf28f) show index status 中设置行尾问题
27. [Commit cb15](https://github.com/manticoresoftware/manticoresearch/commit/cb153228) HTTP JSON 响应中 “highlight” 标题为空
28. [Issue #318](https://github.com/manticoresoftware/manticoresearch/issues/318) `CREATE TABLE LIKE` 中缀错误
29. [提交 9040](https://github.com/manticoresoftware/manticoresearch/commit/9040d22c) RT 在高负载下崩溃
30. [cd512c7d ](https://github.com/manticoresoftware/manticoresearch/commit/cd512c7d ) RT 磁盘块崩溃时丢失崩溃日志
31. [问题 #323](https://github.com/manticoresoftware/manticoresearch/issues/323) 导入表失败并关闭连接
32. [提交 6275](https://github.com/manticoresoftware/manticoresearch/commit/6275316a) `ALTER reconfigure` 损坏 PQ 索引
33. [提交 9c1d](https://github.com/manticoresoftware/manticoresearch/commit/9c1d221e) 更改索引类型后 searchd 重新加载问题
34. [提交 71e2](https://github.com/manticoresoftware/manticoresearch/commit/71e2b5bb) 守护进程在导入缺失文件的表时崩溃
35. [问题 #322](https://github.com/manticoresoftware/manticoresearch/issues/322) 使用多个索引、group by 和 ranker = none 的 select 崩溃
36. [提交 c3f5](https://github.com/manticoresoftware/manticoresearch/commit/c3f58490) `HIGHLIGHT()` 不在字符串属性中高亮显示
37. [问题 #320](https://github.com/manticoresoftware/manticoresearch/issues/320) `FACET` 无法在字符串属性上排序
38. [提交 4f1a](https://github.com/manticoresoftware/manticoresearch/commit/4f1a1f25) 缺少数据目录时出错
39. [提交 04f4](https://github.com/manticoresoftware/manticoresearch/commit/04f4ddd4) access_* 不支持 RT 模式
40. [提交 1c06](https://github.com/manticoresoftware/manticoresearch/commit/1c0616a2) JSON 字符串中坏的 JSON 对象：1.`CALL PQ` 当 json 超过某个值时返回 “Bad JSON objects in strings: 1”。
41. [提交 32f9](https://github.com/manticoresoftware/manticoresearch/commit/32f943d6) RT 模式不一致。在某些情况下，无法删除索引，因为索引未知，且无法创建索引，因为目录不为空。
42. [问题 #319](https://github.com/manticoresoftware/manticoresearch/issues/319) select 时崩溃
43. [提交 22a2](https://github.com/manticoresoftware/manticoresearch/commit/22a28dd7) `max_xmlpipe2_field` = 2M 对 2M 字段返回警告
44. [问题 #342](https://github.com/manticoresoftware/manticoresearch/issues/342) 查询条件执行错误
45. [提交 dd8d](https://github.com/manticoresoftware/manticoresearch/commit/dd8dcab2) 简单的两个词搜索返回仅包含一个词的文档
46. [提交 9091](https://github.com/manticoresoftware/manticoresearch/commit/90919e62) PQ 中无法匹配键中带大写字母的 json
47. [提交 56da](https://github.com/manticoresoftware/manticoresearch/commit/56da086a) 索引器在 csv+docstore 上崩溃
48. [问题 #363](https://github.com/manticoresoftware/manticoresearch/issues/363) 在 centos 7 中 json 属性中使用 `[null]` 导致插入数据损坏
49. 重大 [问题 #345](https://github.com/manticoresoftware/manticoresearch/issues/345) 记录未插入，count() 随机，"replace into" 返回成功
50. max_query_time 极大减慢 SELECT 执行速度


51. [问题 #352](https://github.com/manticoresoftware/manticoresearch/issues/352) Mac OS 上主从通信失败
52. [问题 #328](https://github.com/manticoresoftware/manticoresearch/issues/328) 使用 Connector.Net/Mysql 8.0.19 连接 Manticore 时出错
53. [提交 daa7](https://github.com/manticoresoftware/manticoresearch/commit/daa760d2fe83668038318f0f384ceefc0d075e18) 修正 \0 转义并优化性能

54. [提交 9bc5](https://github.com/manticoresoftware/manticoresearch/commit/9bc5c01a493db993b90ff6c1ad51aeae028f629c) 修正 count distinct 与 json 的问题
55. [提交 4f89](https://github.com/manticoresoftware/manticoresearch/commit/4f89a965a3af0db548ae9d1763731ebaebe21f5a) 修正其他节点删除表失败
56. [提交 952a](https://github.com/manticoresoftware/manticoresearch/commit/952af5a5931fd72b87edc169588777778b365231) 修复频繁调用 call pq 时崩溃问题
## 版本 3.4.2，2020 年 4 月 10 日
### 重要修复
* [提交 2ffe](https://github.com/manticoresoftware/manticoresearch/commit/2ffe2d261793f9681265bc4abe38d695e72c121e) 修复旧版本 RT 索引无法索引数据

## 版本 3.4.0，2020 年 3 月 26 日
### 重大变更
* 服务器支持两种模式：rt 模式和普通模式
   * rt 模式需要 data_dir，且配置中无索引定义
   * 普通模式索引在配置中定义；不允许 data_dir
* 复制仅在 rt 模式下可用

### 次要变更
* charset_table 默认为 non_cjk 别名
* rt 模式中全文字段默认被索引和存储

* rt 模式全文字段由 'field' 改名为 'text'
* ALTER RTINDEX 改名为 ALTER TABLE
* TRUNCATE RTINDEX 改名为 TRUNCATE TABLE
### 新特性
* 仅存储字段

* SHOW CREATE TABLE，IMPORT TABLE
### 改进
* 更快的无锁 PQ
* /sql 可在 mode=raw 下执行任何类型 SQL 语句
* mysql 协议别名 mysql41
* data_dir 中默认 state.sql
### 错误修复
* [提交 a533](https://github.com/manticoresoftware/manticoresearch/commit/a5333644e8e0c69746a9f24da8221fc965478e4a) 修正 highlight() 中错误字段语法导致崩溃
* [提交 7fbb](https://github.com/manticoresoftware/manticoresearch/commit/7fbb9f2e618096289cba7f8b834530b46aa94055) 修正复制带 docstore 的 RT 索引时服务器崩溃
* [提交 24a0](https://github.com/manticoresoftware/manticoresearch/commit/24a04687eb43b4f4719c7c4a5fe3c19099240467) 修正对带有 infix 或 prefix 选项且未启用存储字段的索引执行高亮时崩溃
* [提交 3465](https://github.com/manticoresoftware/manticoresearch/commit/3465c1ce9c860c09f6ccc5b83a63e3c68c241d8d) 修正空 docstore 和空索引 dock-id 查找的错误提示
* [提交 a707](https://github.com/manticoresoftware/manticoresearch/commit/b8601b4144b9092be43a7caa7cd6b19cb37e14c7) 修正 #314 SQL 插入命令末尾有分号的问题
* [提交 9562](https://github.com/manticoresoftware/manticoresearch/commit/95628c9b9b8f16fe1bd18599a84e9673575adf1a) 移除查询词不匹配警告

* [Commit b860](https://github.com/manticoresoftware/manticoresearch/commit/b8601b4144b9092be43a7caa7cd6b19cb37e14c7) 修复通过 ICU 分段的片段中的查询
* [Commit 5275](https://github.com/manticoresoftware/manticoresearch/commit/5275516ca4631c584e6e0127fa161a18d2658fd0) 修复 docstore 块缓存中的查找/添加竞态条件
* [Commit f06e](https://github.com/manticoresoftware/manticoresearch/commit/f06ef97af5c7390427e82525c150535290e42c3e) 修复 docstore 中的内存泄漏
* [Commit a725](https://github.com/manticoresoftware/manticoresearch/commit/a7258ba8340e246c0d8aaeb4a7a74129eba807eb) 修复 #316 INSERT 时 LAST_INSERT_ID 返回空
* [Commit 1ebd](https://github.com/manticoresoftware/manticoresearch/commit/1ebd5bf8d74113bcfa138ce63ce11966990a4215) 修复 #317 json/update HTTP 端点支持 MVA 数组和 JSON 属性的对象
* [Commit e426](https://github.com/manticoresoftware/manticoresearch/commit/e426950a536382a142e2a42ed207c341149ff685) 修复索引器在没有显式 id 的情况下转储 RT 数据的问题
## 版本 3.3.0，2020年2月4日

### 新特性
* 并行实时索引搜索
* EXPLAIN QUERY 命令
* 不含索引定义的配置文件（测试版）
* CREATE/DROP TABLE 命令（测试版）
* indexer --print-rt - 可以从源读取并打印用于实时索引的 INSERT 语句
### 改进


* 更新至 Snowball 2.0 词干提取器
* SHOW INDEX STATUS 支持 LIKE 过滤器
* 对高 max_matches 场景下的内存使用进行了优化
* SHOW INDEX STATUS 为 RT 索引增加 ram_chunks_count 显示项
* 无锁优先队列（PQ）
* 将 LimitNOFILE 改为 65536
### 修复
* [Commit 9c33](https://github.com/manticoresoftware/manticoresearch/commit/9c33aab8c1fd9059c8644d23d5f58f03010043c6) 添加索引模式中重复属性的检查 #293
* [Commit a008](https://github.com/manticoresoftware/manticoresearch/commit/a0085f9477e191da9e0e835b91583fb9cfde698e) 修复无命中文档时的崩溃
* [Commit 6895](https://github.com/manticoresoftware/manticoresearch/commit/68953740af8dca1833026d0f448c71416ed3044a) 修复 ATTACH 后的松散 docstore
* [Commit d6f6](https://github.com/manticoresoftware/manticoresearch/commit/d6f696edea53ea2b9a68251e4054e4816082c285) 修复分布式部署中的 docstore 问题
* [Commit bce2](https://github.com/manticoresoftware/manticoresearch/commit/bce2b7ec883d208f31094ee42a5b6f01d18a998f) 在 sorter 中将 FixedHash 替换为 OpenHash
* [Commit e0ba](https://github.com/manticoresoftware/manticoresearch/commit/e0baf7392937969e640fb5d10e8ccb312fe36446) 修复索引定义中属性名重复的问题
* [Commit ca81](https://github.com/manticoresoftware/manticoresearch/commit/ca81114bf1ac7ea7adcc8509bdc14d1f30b4f461) 修复 HIGHLIGHT() 中的 html_strip
* [Commit 493a](https://github.com/manticoresoftware/manticoresearch/commit/493a5e916d70fd4a3f14d1ee487234434f4a7ce3) 修复 HIGHLIGHT() 中的 passage 宏
* [Commit a82d](https://github.com/manticoresoftware/manticoresearch/commit/a82d41c775692f837f267d554d7d4a4b499d69f0) 修复 RT 索引创建小或大磁盘块时的双重缓冲问题
* [Commit a404](https://github.com/manticoresoftware/manticoresearch/commit/a404c85de0d41d9d1c75b7540479d8a3af9e61aa) 修复 kqueue 的事件删除问题

* [Commit 8bea](https://github.com/manticoresoftware/manticoresearch/commit/8bea0f6fd1666b817995c4dd6e67f6f1c9161638) 修复 RT 索引 rt_mem_limit 值较大时，磁盘块保存的问题
* [Commit 8707](https://github.com/manticoresoftware/manticoresearch/commit/8707f0395b7ace52dc0cba9c8ed92a2bc4392932) 修复索引时的浮点数溢出
* [Commit a564](https://github.com/manticoresoftware/manticoresearch/commit/a56434ce71bb7714b4d8e75278aaa075b27ece80) 现在插入带负 ID 的文档到 RT 索引会失败并报错
* [Commit bbeb](https://github.com/manticoresoftware/manticoresearch/commit/bbebfd7551a68641154761cac962053b54806759) 修复服务器在 ranker fieldmask 时崩溃
* [Commit 3809](https://github.com/manticoresoftware/manticoresearch/commit/3809cc1b0863e3a5a200dc423be3f7c38be56113) 修复使用查询缓存时崩溃
* [Commit dc2a](https://github.com/manticoresoftware/manticoresearch/commit/dc2a585bc724678679e4297362200bbd49ce32bb) 修复使用 RT 索引 RAM 段及并行插入时崩溃

## 版本 3.2.2，2019年12月19日
### 新特性
* RT 索引支持自增 ID
* 通过新的 HIGHLIGHT() 函数支持 docstore 高亮，也可用于 HTTP API
* SNIPPET() 可使用特殊函数 QUERY()，返回当前 MATCH 查询
* 新增 field_separator 选项用于高亮函数
### 改进和变更
* 远程节点的存储字段懒加载（可显著提升性能）
* 字符串和表达式不会再破坏多查询和 FACET 优化
* RHEL/CentOS 8 构建现使用 mariadb-connector-c-devel 中的 mysql libclient

* ICU 数据文件随软件包一起发布，移除 icu_data_dir 配置项
* systemd 服务文件包含 'Restart=on-failure' 策略
* indextool 现在可在线检查实时索引
* 默认配置改为 /etc/manticoresearch/manticore.conf
* RHEL/CentOS 的服务名称从 'searchd' 改为 'manticore'
* 移除 query_mode 和 exact_phrase 片段选项
### 修复
* [Commit 6ae4](https://github.com/manticoresoftware/manticoresearch/commit/6ae474c7894a6bee222d5b18e59a44fdbf57843a) 修复通过 HTTP 接口 SELECT 查询时崩溃
* [Commit 5957](https://github.com/manticoresoftware/manticoresearch/commit/59577513a49eac5a4a3c5e2cb38394d3246b5d35) 修复 RT 索引保存磁盘块但未标记部分文档为删除
* [Commit e861](https://github.com/manticoresoftware/manticoresearch/commit/e861f0fca0e88924450695d4e9d6acff7a36558a) 修复使用 dist_threads 搜索多索引或多查询时崩溃
* [Commit 4409](https://github.com/manticoresoftware/manticoresearch/commit/440991fc977b8479800b45cdbc862eeb1ba5d965) 修复长词条包含宽 UTF8 码点时词缀生成崩溃
* [Commit 5fd5](https://github.com/manticoresoftware/manticoresearch/commit/5fd599b48bba527c023e6aa0b262dca51ffb8a1c) 修复添加套接字到 IOCP 的竞态条件
* [提交 cf10](https://github.com/manticoresoftware/manticoresearch/commit/cf10d7d3589f77adfaefbe52a51777bc9c67cf99) 修复布尔查询与 json 选择列表的问题
* [提交 996d](https://github.com/manticoresoftware/manticoresearch/commit/996de77f4c3ec103d965e28c36098fd07dba50c4) 修正 indextool 检查以报告错误的跳表偏移，检查 doc2row 查找
* [提交 6e3f](https://github.com/manticoresoftware/manticoresearch/commit/6e3fc9e88941c9427410f9d0adeebb7dd09a900f) 修复索引器在大量数据情况下因负跳表偏移产生错误索引
* [提交 faed](https://github.com/manticoresoftware/manticoresearch/commit/faed3220b423a96401982bf47117edf1f62c584e) 修正 JSON 只转换数字为字符串及表达式中的 JSON 字符串与数字转换
* [提交 5331](https://github.com/manticoresoftware/manticoresearch/commit/533197200249d648ebcbdaca5d605861ee566348) 修正 indextool 在命令行设置多个命令时以错误码退出
* [提交 7955](https://github.com/manticoresoftware/manticoresearch/commit/795520ac351d22e4497ebaf6c249bf25081842b6) 修正 #275 磁盘空间不足错误导致 binlog 无效状态

* [提交 2284](https://github.com/manticoresoftware/manticoresearch/commit/2284da5e3ff77bf7be4038a4d950c0580e4159c8) 修正 #279 JSON 属性的 IN 过滤器崩溃
* [提交 ce2e](https://github.com/manticoresoftware/manticoresearch/commit/ce2e4b4765111a7d331ffc256911f12770a8942d) 修正 #281 错误的管道关闭调用
* [提交 5355](https://github.com/manticoresoftware/manticoresearch/commit/535589ba8a37baf5c6056afb5ff969fd70476feb) 修复服务器在 CALL PQ 时因递归 JSON 属性以字符串编码而挂起
* [提交 a5fc](https://github.com/manticoresoftware/manticoresearch/commit/a5fc8a36e1644ee051632408ab8b53e79aeb9ff4) 修复 multiand 节点中文档列表超出结尾的推进问题

* [提交 a362](https://github.com/manticoresoftware/manticoresearch/commit/a3628617ef4ee89a811d4ba2c4a04cbbc298c1b5) 修复线程公共信息检索
* [提交 f8d2](https://github.com/manticoresoftware/manticoresearch/commit/f8d2d7bbbb90af3d2ee97424f97b05927f09d42b) 修复文档存储缓存锁
## 版本 3.2.0，2019 年 10 月 17 日
### 新功能
* 文档存储

* 新指令 stored_fields、docstore_cache_size、docstore_block_size、docstore_compression、docstore_compression_level
### 改进和变更
* 改进 SSL 支持
* 更新了非 CJK 内置字符集
* 禁用 UPDATE/DELETE 语句在查询日志中记录 SELECT
* RHEL/CentOS 8 软件包
### Bug 修复
* [提交 301a](https://github.com/manticoresoftware/manticoresearch/commit/301a806b14725e842300ee225521407464374c41) 修复 RT 索引磁盘块中替换文档时崩溃
* [提交 46c1](https://github.com/manticoresoftware/manticoresearch/commit/46c1cad8fa11c1fc09cc34b20d978fdfcd4e6d27) 修复 #269 LIMIT N OFFSET M
* [提交 92a4](https://github.com/manticoresoftware/manticoresearch/commit/92a46edaa196ef33f3ac2225fbdba2c2b88ee45d) 修正带有显式设置 id 或提供 id 列表的 DELETE 语句跳过搜索

* [提交 8ca7](https://github.com/manticoresoftware/manticoresearch/commit/8ca78c138d837caab30dc6e2343a3c4d1687bb87) 修复 windowspoll 轮询器中事件移除后错误的索引
* [提交 6036](https://github.com/manticoresoftware/manticoresearch/commit/603631e2bff5cb0acf25f68b85621742490eccd9) 修正通过 HTTP 的 JSON 浮点数四舍五入
* [提交 62f6](https://github.com/manticoresoftware/manticoresearch/commit/62f64cb9e75ec28fc3c29f8414ce9656f5580254) 修正远程代码片段先检查空路径；修复 Windows 测试
* [提交 aba2](https://github.com/manticoresoftware/manticoresearch/commit/aba274c2c5abc1029894ba9f95b786bda93e8f22) 修正配置重载在 Windows 上行为与 Linux 一致
* [提交 6b8c](https://github.com/manticoresoftware/manticoresearch/commit/6b8c4242ef77c3fa4d0ccb7d76d81714b6728f0b) 修复 #194 PQ 支持形态学和词干提取器
* [提交 174d](https://github.com/manticoresoftware/manticoresearch/commit/174d312905285aa5746f4f404c53e6dc669ef42f) 修复 RT 退役段管理
## 版本 3.1.2，2019 年 8 月 22 日

### 新功能和改进
* HTTP API 的实验性 SSL 支持
* CALL KEYWORDS 字段过滤
* /json/search 的 max_matches 参数
* 默认 Galera gcache.size 的自动调节
* 改进对 FreeBSD 的支持
### Bug 修复
* [提交 0a1a](https://github.com/manticoresoftware/manticoresearch/commit/0a1a2c81038794983b4e30c8aefc7cc20e89b03f) 修正 RT 索引在有相同索引但路径不同节点上的复制

* [提交 4adc](https://github.com/manticoresoftware/manticoresearch/commit/4adc075294ac823289f745e2cc419f18c7dcf2e2) 修正无活动索引的刷新重新调度
* [提交 d6c0](https://github.com/manticoresoftware/manticoresearch/commit/d6c00a6f750c04021747e9c99e0f77707a4cbfc3) 改进 RT/PQ 索引刷新重新调度
* [提交 d0a7](https://github.com/manticoresoftware/manticoresearch/commit/d0a7c95906b9b5ba33c98267d54501cfe27508d6) 修正 #250 TSV 和 CSV 管道源的 index_field_lengths 索引选项
* [提交 1266](https://github.com/manticoresoftware/manticoresearch/commit/1266d54800242709fbe2c34e72eb69b6595b23bd) 修正 indextool 对空索引块索引检查的错误报告
* [提交 553c](https://github.com/manticoresoftware/manticoresearch/commit/553ca73ca111f28e0888d59cd48b0d6df181ecd6) 修正 Manticore SQL 查询日志中空选择列表
* [提交 56c8](https://github.com/manticoresoftware/manticoresearch/commit/56c8584456293665ddd4ce4c94307d74473a78b9) 修正索引器的 -h/--help 响应
## 版本 3.1.0，2019 年 7 月 16 日
### 新功能和改进
* 实时索引复制
* 中文 ICU 分词器
* 新形态学选项 icu_chinese
* 新指令 icu_data_dir
* 复制用多语句事务
* LAST_INSERT_ID() 和 @session.last_insert_id

* SHOW VARIABLES 支持 LIKE 'pattern'
* 多文档插入支持 percolate 索引
* 配置的时间解析器添加

* 内部任务管理器
* 对文档和命中列表组件启用 mlock
* 限制代码片段路径
### 移除
* RLP 库支持已弃用，改为使用 ICU；删除了所有 rlp\* 指令
* 使用 UPDATE 更新文档 ID 已被禁用
### 错误修复
* [Commit f047](https://github.com/manticoresoftware/manticoresearch/commit/f047222339fd0e62a07e9ca8b2a829297d5db861) 修复 concat 和 group_concat 中的缺陷
* [Commit b081](https://github.com/manticoresoftware/manticoresearch/commit/b08147ee24712b5eaa51f261626b5d16c6242fdd) 修复 percolate 索引中的 query uid，使其为 BIGINT 属性类型
* [Commit 4cd8](https://github.com/manticoresoftware/manticoresearch/commit/4cd85afaf76eef0ce9fa1cd073f3fed160279890) 如果预分配新的磁盘块失败，则不会崩溃
* [Commit 1a55](https://github.com/manticoresoftware/manticoresearch/commit/1a551227886474054111088412f2b2396f485652) 在 ALTER 中添加缺失的 timestamp 数据类型

* [Commit f3a8](https://github.com/manticoresoftware/manticoresearch/commit/f3a8e096fd496a6b001d398aadbb781b2dbad929) 修复错误 mmap 读取导致的崩溃
* [Commit 4475](https://github.com/manticoresoftware/manticoresearch/commit/447577115192db7ac7bfea70b5ba58ee2c61229f) 修复复制中集群锁的哈希
* [Commit ff47](https://github.com/manticoresoftware/manticoresearch/commit/ff476df955f057bd38e8b6ad6056325e9d6fbb9c) 修复复制中 providers 泄漏
* [Commit 58dc](https://github.com/manticoresoftware/manticoresearch/commit/58dcbb779c4c30ca1560460c78e35ba114b83716) 修复 \#246 索引器中未定义的 sigmask
* [Commit 3dd8](https://github.com/manticoresoftware/manticoresearch/commit/3dd8278e747d534f69fe9c3d360eef188ab7b6d5) 修复 netloop 报告中的竞争条件
* [Commit a02a](https://github.com/manticoresoftware/manticoresearch/commit/a02aae05a3fa3d1caf2821f43c7e237b6de76336) 为 HA 策略再平衡器设置零间隙

## 版本 3.0.2，2019 年 5 月 31 日
### 改进

* 添加了用于文档和命中列表的 mmap 读取器
* `/sql` HTTP 端点的响应现在与 `/json/search` 响应相同

* 新指令 `access_plain_attrs`、`access_blob_attrs`、`access_doclists`、`access_hitlists`
* 用于复制设置的新指令 `server_id`
### 删除
* 删除了 HTTP `/search` 端点
### 弃用
* `read_buffer`、`ondisk_attrs`、`ondisk_attrs_default`、`mlock` 已被 `access_*` 指令取代
### 错误修复
* [Commit 849c](https://github.com/manticoresoftware/manticoresearch/commit/849c16e1040f382795ba0635cb48686f9db4f2d8) 允许属性名称以数字开头出现在 SELECT 列表中
* [Commit 48e6](https://github.com/manticoresoftware/manticoresearch/commit/48e6c302ca37bec0bc49c79619c60bf787ed0708) 修复了 UDF 中的 MVA 问题，修复了 MVA 别名
* [Commit 0555](https://github.com/manticoresoftware/manticoresearch/commit/055586a9a2ebfa8874f68099c5bc798a145fd4ef) 修复了使用带有 SENTENCE 的查询时的 \#187 崩溃


* [Commit 93bf](https://github.com/manticoresoftware/manticoresearch/commit/93bf52f23f1c53c8a93a603387ca5463e1fc2ae8) 修复了对 MATCH() 周围括号的支持(\#143)
* [Commit 599e](https://github.com/manticoresoftware/manticoresearch/commit/599ee79ccd38cce0023a55cbce8f50c3dca62e38) 修复了 ALTER cluster 语句上保存集群状态的问题
* [Commit 230c](https://github.com/manticoresoftware/manticoresearch/commit/230c321e277514b191629538eb3f02d219113d95) 修复了在带有 blob 属性的 ALTER 索引时服务器崩溃的问题
* [Commit 5802](https://github.com/manticoresoftware/manticoresearch/commit/5802b85aa93a81caea77073a3381ef912fdd677c) 修复了 \#196 通过 id 过滤的问题
* [Commit 25d2](https://github.com/manticoresoftware/manticoresearch/commit/25d2dabda4a299a29bd375405862126da9b4bfac) 在模板索引上取消搜索
* [Commit 2a30](https://github.com/manticoresoftware/manticoresearch/commit/2a30d5b49d76faab031d195f533974938769b8e6) 修复了 SQL 响应中 id 列为常规 bigint 类型
## 版本 3.0.0，2019 年 5 月 6 日
### 特性和改进
* 新的索引存储。非标量属性不再受每个索引 4GB 大小的限制
* `attr_update_reserve` 指令
* 可以使用 UPDATE 更新 String、JSON 和 MVA
* killlists 在索引加载时应用
* `killlist_target` 指令
* 多重 AND 搜索加速
* 提升了平均性能和内存使用效率

* 用于升级 2.x 创建的索引的 convert 工具
* CONCAT() 函数
* JOIN CLUSTER cluster AT 'nodeaddress:port'
* ALTER CLUSTER posts UPDATE nodes

* `node_address` 指令
* 在 SHOW STATUS 中打印节点列表

### 行为变化
* 对于具有 killists 的索引，服务器不会按 conf 中定义的顺序旋转索引，而是遵循 killlist 目标链
* 搜索中索引的顺序不再定义 killlists 应用的顺序
* 文档 ID 现在是带符号的大整数

### 移除的指令
* docinfo （现在始终为 extern）、inplace_docinfo_gap、mva_updates_pool
## 版本 2.8.2 GA，2019 年 4 月 2 日

### 特性和改进
* percolate 索引的 Galera 复制
* OPTION morphology
### 编译说明
Cmake 最低版本现为 3.13。编译需要 boost 和 libssl
开发库。
### 错误修复
* [Commit 6967](https://github.com/manticoresoftware/manticoresearch/commit/6967fedb2ef818ec1c825d482563edd05e1c9245) 修复了在针对多个分布式索引的查询中，SELECT 列表中使用多个星号导致的崩溃
* [Commit 36df](https://github.com/manticoresoftware/manticoresearch/commit/36df1a407dc08263690e3492518613ace82d69ca) 修复了 [\#177](https://github.com/manticoresoftware/manticoresearch/issues/177) 通过 Manticore SQL 接口传输的大包问题
* [Commit 5793](https://github.com/manticoresoftware/manticoresearch/commit/57932aec734583fa93359faaf5034b2e2c9d352b) 修复了 [\#170](https://github.com/manticoresoftware/manticoresearch/issues/170) 在 RT 优化时更新 MVA 导致的服务器崩溃

* [Commit edb2](https://github.com/manticoresoftware/manticoresearch/commit/edb24b870423add86eba471d361e0e5aff098b18) 修复了因在 SIGHUP 后配置重载时 RT 索引被移除而在 binlog 删除时导致的服务器崩溃
* [Commit bd3e](https://github.com/manticoresoftware/manticoresearch/commit/bd3e66e0085bc7e2e351b817dfc972fd8158fbce) 修复了 mysql 握手认证插件的负载
* [Commit 6a21](https://github.com/manticoresoftware/manticoresearch/commit/6a217f6ed82fb10f752213b15617b9cedf1e8533) 修复了 RT 索引中的 phrase_boundary 设置问题[#172](https://github.com/manticoresoftware/manticoresearch/issues/172)
* [Commit 3562](https://github.com/manticoresoftware/manticoresearch/commit/3562f652753e4091fd7b5b0f65b58341c8cbcb31) 修复了 ATTACH 索引附加到自身时的死锁问题[#168](https://github.com/manticoresoftware/manticoresearch/issues/168)
* [Commit 250b](https://github.com/manticoresoftware/manticoresearch/commit/250b3f0e74ad18dda34ba080aa13ca87798a9ac9) 修复了服务器崩溃后 binlog 保存空的 meta 问题

* [Commit 4aa6](https://github.com/manticoresoftware/manticoresearch/commit/4aa6c69ad3a181cd7c5dafb990fb528121f68e59) 修复了由于 RT 索引带磁盘块的排序器字符串导致服务器崩溃的问题
## 版本 2.8.1 GA，2019年3月6日
### 新特性和改进
* SUBSTRING_INDEX()
* percolate 查询支持 SENTENCE 和 PARAGRAPH
* 为 Debian/Ubuntu 添加了 systemd 生成器；还添加了 LimitCORE 以允许生成 core dump
### Bug 修复
* [Commit 84fe](https://github.com/manticoresoftware/manticoresearch/commit/84fe7405d17b59f4dcde15a6c8e8503923503def) 修复了在匹配模式为 all 且全文查询为空时服务器崩溃的问题
* [Commit daa8](https://github.com/manticoresoftware/manticoresearch/commit/daa88b579fa373d5d2e869e43a2e178363b0fef1) 修复了删除静态字符串时的崩溃问题
* [Commit 2207](https://github.com/manticoresoftware/manticoresearch/commit/22078537dddbd9ce81a182dcc879f235b6e87004) 修正了 indextool 失败时的退出代码（FATAL）
* [Commit 0721](https://github.com/manticoresoftware/manticoresearch/commit/0721696d6780c200c65d596624a8187983fb7fcb) 修复了由于错误的精确形式检查导致前缀无匹配的问题[#109](https://github.com/manticoresoftware/manticoresearch/issues/109)
* [Commit 8af8](https://github.com/manticoresoftware/manticoresearch/commit/8af810111b8e2f87bc9f378172eff1ab9725c7e7) 修复了 RT 索引配置设置重新加载问题[#161](https://github.com/manticoresoftware/manticoresearch/issues/161)
* [Commit e2d5](https://github.com/manticoresoftware/manticoresearch/commit/e2d592773f4fd702ace90070a793a842a0fec6ab) 修复了访问大型 JSON 字符串时服务器崩溃
* [Commit 75cd](https://github.com/manticoresoftware/manticoresearch/commit/75cd1342d05cdc352e3dd145cdbc6f79394a165b) 修正了 JSON 文档中 PQ 字段被索引剥离器修改导致从兄弟字段错误匹配的问题
* [Commit e2f7](https://github.com/manticoresoftware/manticoresearch/commit/e2f775437df922674d772ed4417780492502e65a) 修复了 RHEL7 构建下解析 JSON 时服务器崩溃的问题
* [Commit 3a25](https://github.com/manticoresoftware/manticoresearch/commit/3a25a5808feb3f8b80866991b436d6c1241618c2) 修复了 JSON 取消转义中斜杠位于边缘时崩溃的问题

* [Commit be9f](https://github.com/manticoresoftware/manticoresearch/commit/be9f497872bea62dbdccd64fc28294fd7776c289) 修正了 'skip_empty' 选项以跳过空文档且不警告其非有效 JSON
* [Commit 266e](https://github.com/manticoresoftware/manticoresearch/commit/266e0e7b088549722a805fc837bf101ff681a5e8) 修正了浮点数输出为 8 位数字，解决 6 位数不足以精确显示的问题[#140](https://github.com/manticoresoftware/manticoresearch/issues/161)
* [Commit 3f6d](https://github.com/manticoresoftware/manticoresearch/commit/3f6d23891064ee8e94030c4231497cdd7da33a6a) 修复了空 jsonobj 创建问题
* [Commit f3c7](https://github.com/manticoresoftware/manticoresearch/commit/f3c7848a59b7eedc67c3403d6d4b37ce6fa94dc8) 修复了空 mva 输出 NULL 而非空字符串的问题[#160](https://github.com/manticoresoftware/manticoresearch/issues/161)
* [Commit 0afa](https://github.com/manticoresoftware/manticoresearch/commit/0afa2ed058e5759470b1d7354c722faab34f98bb) 修复了在缺少 pthread_getname_np 时编译失败的问题
* [Commit 9405](https://github.com/manticoresoftware/manticoresearch/commit/9405fccdeb27f0302a8d9a848981d30080216777) 修复了线程池工作线程导致服务器关闭时崩溃的问题
## 版本 2.8.0 GA，2019年1月28日
### 改进
* percolate 索引的分布式索引支持
* CALL PQ 新选项和更改：
    *   skip_bad_json
    *   mode (sparsed/sharded)
    *   JSON 文档现在可以作为 JSON 数组传入
    *   shift
    *   列名 'UID', 'Documents', 'Query', 'Tags', 'Filters' 改名为 'id', 'documents', 'query', 'tags', 'filters'
* 支持 DESCRIBE pq TABLE
* 不再支持 SELECT FROM pq WHERE UID，改用 'id'
* 对 pq 索引的 SELECT 支持与普通索引等效（例如，可以通过 REGEX() 过滤规则）

* ANY/ALL 可用于 PQ 标签
* 表达式针对 JSON 字段具有自动转换，无需显式类型转换
* 内置了 'non_cjk' charset_table 与 'cjk' ngram_chars
* 内置支持 50 种语言的停用词集合
* 停用词声明中可以使用逗号分隔多个文件
* CALL PQ 支持接受 JSON 数组的文档
### Bug 修复
* [Commit a4e1](https://github.com/manticoresoftware/manticoresearch/commit/a4e19afee54dafdc04b0da53372dafd8c0d332d6) 修复了 csjon 相关泄漏
* [Commit 28d8](https://github.com/manticoresoftware/manticoresearch/commit/28d862774874891a03e361da1b0347ebe8066ce0) 修复了由于缺失 json 值导致的崩溃
* [Commit bf4e](https://github.com/manticoresoftware/manticoresearch/commit/bf4e9ea27c349cdc9ae4e54e960a4a17b90e38fa) 修复了 RT 索引保存空 meta 问题
* [Commit 33b4](https://github.com/manticoresoftware/manticoresearch/commit/33b4573529e7c7c7bce19bf9d54054866f30d99a) 修复了词形还原连续序列丢失标志（exact）的问题
* [Commit 6b95](https://github.com/manticoresoftware/manticoresearch/commit/6b95d48240d3b5520afade19c249d79e020a5e88) 修复了字符串属性大于 4M 时改用饱和而不是溢出
* [Commit 6214](https://github.com/manticoresoftware/manticoresearch/commit/621418b829e70af36aaa322a2f51ece3f86bc499) 修复了禁用索引时发送 SIGHUP 导致服务器崩溃的问题
* [Commit 3f7e](https://github.com/manticoresoftware/manticoresearch/commit/3f7e35d1482966cc45d5abbcb2de5de3508d66b0) 修复了同时执行 API 会话状态命令时服务器崩溃的问题
* [Commit cd9e](https://github.com/manticoresoftware/manticoresearch/commit/cd9e4f1709a48ddafc128c450c2d882bc11ba50e) 修复了对带字段过滤的 RT 索引执行删除查询时服务器崩溃的问题
* [Commit 9376](https://github.com/manticoresoftware/manticoresearch/commit/9376470d455fcd256c6abff79c431a6919dfa7ac) 修复了对分布式索引执行 CALL PQ 时遇到空文档导致服务器崩溃的问题
* [Commit 8868](https://github.com/manticoresoftware/manticoresearch/commit/8868b207644f4f8cc1ab0c270adb35493fed22e1) 修复了 Manticore SQL 错误消息超过 512 字符被截断的问题
* [Commit de9d](https://github.com/manticoresoftware/manticoresearch/commit/de9deda9c142823d6dbf529423417b5c670fae94) 修复了在无 binlog 的情况下保存 percolate 索引时崩溃的问题
* [Commit 2b21](https://github.com/manticoresoftware/manticoresearch/commit/2b219e1a32791e7740bb210b7d408f96abc6374f) 修复了 OSX 系统下 http 接口无法工作的问题

* [Commit e92c](https://github.com/manticoresoftware/manticoresearch/commit/e92c60240f8f9e1756bfa14fba0bbb402586a389) 修复了 indextool 在检查 MVA 时的错误提示问题
* [Commit 238b](https://github.com/manticoresoftware/manticoresearch/commit/238bdea59bad89f097403f1c978658ce45f16c70) 修复了在对 RTINDEX 执行 FLUSH 时写锁问题，避免在保存及定期由 rt_flush_period 触发的刷新时锁住整个索引
* [Commit c26a](https://github.com/manticoresoftware/manticoresearch/commit/c26a236bd42b7082db079a51a84172066d337d64) 修复了 ALTER percolate 索引时卡在等待搜索加载的问题
* [Commit 9ee5](https://github.com/manticoresoftware/manticoresearch/commit/9ee5703f6c28f57cd5bff5e705966d93fe30d267) 修复了 max_children 为 0 时应使用默认线程池工作线程数的问题
* [Commit 5138](https://github.com/manticoresoftware/manticoresearch/commit/5138fc048a1bd146b271ce6a72fe954344281f69) 修复了在索引插件 index_token_filter 配合停用词与 stopword_step=0 的情况下索引数据错误的问题

* [Commit 2add](https://github.com/manticoresoftware/manticoresearch/commit/2add3d319ac62fe450bf60e89033100853dc2ecf) 修复了使用 aot 词形归一化器但缺失 lemmatizer_base 导致崩溃的问题
## 版本 2.7.5 GA，2018 年 12 月 4 日
### 改进
* 增加了 REGEX 函数
* json API 搜索支持 limit/offset
* 为 qcache 添加 profiler 计时点
###  修复

* [Commit eb3c](https://github.com/manticoresoftware/manticoresearch/commit/eb3c7683b9740cff4cc4cadd2ab9f990b9f7d173) 修复了 FACET 查询多重属性宽类型时服务器崩溃的问题
* [Commit d915](https://github.com/manticoresoftware/manticoresearch/commit/d915cf6696514f4be365ffb5981bccaa15983e65) 修复了 FACET 查询主选择列表隐式 group by 的问题
* [Commit 5c25](https://github.com/manticoresoftware/manticoresearch/commit/5c25dc271a8c11a1c82ca3c88e518acbc6a8c164) 修复了使用 GROUP N BY 查询时的崩溃问题
* [Commit 85d3](https://github.com/manticoresoftware/manticoresearch/commit/85d30a232d4ffd89916b673a9b2db946d272a945) 修复了内存操作异常时的死锁问题
* [Commit 8516](https://github.com/manticoresoftware/manticoresearch/commit/85166b5ef2be38061d62f2b20f15acaa0311258a) 修复了 indextool 检查时的内存消耗问题

* [Commit 58fb](https://github.com/manticoresoftware/manticoresearch/commit/58fb03166830d8b23e7ab9ce56309db7ddac22f4) 修复了不再需要包含 gmock 的问题，上游已经自行解决
## 版本 2.7.4 GA，2018 年 11 月 1 日
### 改进
* 在远程分布式索引情况下，SHOW THREADS 显示原始查询而非 API 调用
* SHOW THREADS 新选项 `format=sphinxql` 以 SQL 格式打印所有查询
* SHOW PROFILE 添加了 `clone_attrs` 阶段
### 修复
* [Commit 4f15](https://github.com/manticoresoftware/manticoresearch/commit/4f155712a0bccc1bd01cc191647bc8cff814888e) 修复了在无 malloc_stats、malloc_trim 的 libc 上构建失败的问题

* [Commit f974](https://github.com/manticoresoftware/manticoresearch/commit/f974f20bda3214a56877c393a192be1a77150958) 修复了 CALL KEYWORDS 结果集中的词中包含特殊符号的问题
* [Commit 0920](https://github.com/manticoresoftware/manticoresearch/commit/092083282ea8ae0b2e72fb6989c57ccec81e74ac) 修复了通过 API 调用到分布式索引或远程代理的 CALL KEYWORDS 失效问题
* [Commit fd68](https://github.com/manticoresoftware/manticoresearch/commit/fd686bfe88b720ffd7642e36897ba45161cbd7d2) 修复了分布式索引的 agent_query_timeout 传递给代理时应映射为 max_query_time 的问题
* [Commit 4ffa](https://github.com/manticoresoftware/manticoresearch/commit/4ffa623e9d357a3b0b441615089e211f92f8de32) 修复了磁盘块上的总文档计数受 OPTIMIZE 命令影响，导致权重计算错误的问题
* [Commit dcaf](https://github.com/manticoresoftware/manticoresearch/commit/dcaf4e0e3c8e9b0fbec47bbf3307feddec0936a6) 修复了来自 blended 的 RT 索引多重尾部命中问题
* [Commit eee3](https://github.com/manticoresoftware/manticoresearch/commit/eee381754e902a43a8f499e1c950198a2a3e6ee0) 修复了轮转操作中的死锁问题

## 版本 2.7.3 GA，2018 年 9 月 26 日
### 改进
* CALL KEYWORDS 新增 sort_mode 选项
* VIP 连接的 DEBUG 可以执行 'crash <password>' 命令以故意触发服务器 SIGEGV
* DEBUG 可执行 'malloc_stats' 以将 malloc 统计信息输出到 searchd.log，执行 'malloc_trim' 来调用 malloc_trim()
* 如系统中存在 gdb，改进了堆栈回溯信息
### 修复
* [Commit 0f3c](https://github.com/manticoresoftware/manticoresearch/commit/0f3cc333f6129451ad1ae2c97905b6c04ba5d454) 修复了 Windows 上重命名时崩溃或失败的问题
* [Commit 1455](https://github.com/manticoresoftware/manticoresearch/commit/1455ba2bf8079f03ce7f861a8d9662a360d705de) 修复了 32 位系统服务器崩溃的问题
* [Commit ad37](https://github.com/manticoresoftware/manticoresearch/commit/ad3710d53b5cd6a28b8c60352d0f9e285b03d9fa) 修复了在空 SNIPPET 表达式时服务器崩溃或挂起的问题
* [Commit b36d](https://github.com/manticoresoftware/manticoresearch/commit/b36d79214364500252665860f000ae85343528b6) 修复了非渐进式优化失效，以及渐进式优化不应为最旧的磁盘块创建 kill-list 的问题
* [Commit 34b0](https://github.com/manticoresoftware/manticoresearch/commit/34b032499afd42ce47a4c7247814b4031094388a) 修复线程池工作模式下SQL和API的queue_max_length错误回复
* [Commit ae4b](https://github.com/manticoresoftware/manticoresearch/commit/ae4b3202cbdb8014cabe2b90e269d5cf74f49871) 修复在PQ索引添加带regexp或rlp选项的全扫描查询时崩溃
* [Commit f80f](https://github.com/manticoresoftware/manticoresearch/commit/f80f8d5d7560187078868aed9a9575f4549e98aa) 修复连续调用多个PQ时崩溃
* [Commit 9742](https://github.com/manticoresoftware/manticoresearch/commit/9742f5f0866af73f8cd8483ecd18a507ea80dd65) 重构AcquireAccum
* [Commit 39e5](https://github.com/manticoresoftware/manticoresearch/commit/39e5bc3751b7295222eb76407c5d72ce1dad545b) 修复调用pq后内存泄漏
* [Commit 21bc](https://github.com/manticoresoftware/manticoresearch/commit/21bcc6d17395f0f57dde79f6716ef303b7ea527d) 美化重构（c++11风格的c-trs、默认值、nullptr）
* [Commit 2d69](https://github.com/manticoresoftware/manticoresearch/commit/2d690398f14c736956cfdd66feb4d3091d6b3a4d) 修复在PQ索引尝试插入重复项时的内存泄漏
* [Commit 5ed9](https://github.com/manticoresoftware/manticoresearch/commit/5ed92c4b66da6423fa28f11b1fd59103ed1e1fb9) 修复JSON字段IN大量值时崩溃
* [Commit 4a52](https://github.com/manticoresoftware/manticoresearch/commit/4a5262e2776aa261e34ba5c914ea60804f59de3f) 修复CALL KEYWORDS语句对设置了扩展限制的RT索引导致服务器崩溃
* [Commit 5526](https://github.com/manticoresoftware/manticoresearch/commit/552646bb6fefde4e2525298e43e628839b421d3d) 修复PQ匹配查询中的无效过滤器；
* [Commit 204f](https://github.com/manticoresoftware/manticoresearch/commit/204f521408b730198a5bab2c20407a3d7df94873) 引入PTR属性的小对象分配器
* [Commit 2545](https://github.com/manticoresoftware/manticoresearch/commit/25453e5387152c0575d1eda1b2beb89a49c7493a) 重构ISphFieldFilter为引用计数风格
* [Commit 1366](https://github.com/manticoresoftware/manticoresearch/commit/1366ee06e828397074b69bec9265d7147170c60e) 修复使用strtod处理非终止字符串时的未定义行为/段错误
* [Commit 94bc](https://github.com/manticoresoftware/manticoresearch/commit/94bc6fcbdae4d08bcf6da2966e3ec2966091289f) 修复JSON结果集处理中的内存泄漏
* [Commit e78e](https://github.com/manticoresoftware/manticoresearch/commit/e78e9c948963416caff9e4e46296a58080107835) 修复应用属性添加时内存块越界读取
* [Commit fad5](https://github.com/manticoresoftware/manticoresearch/commit/fad572fb543606b33e688eb82a485b86011545fd) 重构CSphDict为引用计数风格
* [Commit fd84](https://github.com/manticoresoftware/manticoresearch/commit/fd841a472b472280c93f4af3506851d19f6adc45) 修复AOT内部类型泄露
* [Commit 5ee7](https://github.com/manticoresoftware/manticoresearch/commit/5ee7f208c7685f2ebb9e5623a8802b721e2a7f0a) 修复分词器管理的内存泄漏
* [Commit 116c](https://github.com/manticoresoftware/manticoresearch/commit/116c5f1abebee9a0d99afe93546a1d8e4c6c6958) 修复分组器中的内存泄漏
* [Commit 56fd](https://github.com/manticoresoftware/manticoresearch/commit/56fdbc9ab38973a9a0ba8ccee45f71cf33c16423) 对匹配中的动态指针做特殊释放/复制（修复分组器内存泄漏）
* [Commit b1fc](https://github.com/manticoresoftware/manticoresearch/commit/31db660f378541aa0d1e7cf2e7209bb0a1f47fd8) 修复RT动态字符串的内存泄漏
* [Commit 517b](https://github.com/manticoresoftware/manticoresearch/commit/517b9e8749f092e923cd2884b674b5bb84e20755) 重构分组器
* [Commit b1fc](https://github.com/manticoresoftware/manticoresearch/commit/b1fc16140e5dc44290686330b476a254e0092107) 小规模重构（c++11风格c-trs，部分格式调整）
* [Commit 7034](https://github.com/manticoresoftware/manticoresearch/commit/7034e073f4cf2844762e0a464b7c3de05d3d122b) 重构ISphMatchComparator为引用计数风格
* [Commit b1fc](https://github.com/manticoresoftware/manticoresearch/commit/b1fc16140e5dc44290686330b476a254e0092107) 私有化克隆器
* [Commit efbc](https://github.com/manticoresoftware/manticoresearch/commit/efbc0511d6809c4a57453c7283f9bf53f3fb8d97) 简化MVA_UPSIZE、DOCINFO2ID_T、DOCINFOSETID的本机小端支持
* [Commit 6da0](https://github.com/manticoresoftware/manticoresearch/commit/6da0df4ac9946fc59f98d229b90e12c9221dd4c0) 为uber测试添加valgrind支持
* [Commit 1d17](https://github.com/manticoresoftware/manticoresearch/commit/1d17669cb7ec54ac80b392e101f37a688eb98080) 修复连接上“success”标志的竞争导致崩溃
* [Commit 5a09](https://github.com/manticoresoftware/manticoresearch/commit/5a09c32d7ded56b2c0bd3e2ad7968cb8d6362919) 将epoll切换为边缘触发模式
* [Commit 5d52](https://github.com/manticoresoftware/manticoresearch/commit/5d528682737fca03a4352a3093b1319ec27d598c) 修复表达式中格式如过滤器的IN语句
* [Commit bd8b](https://github.com/manticoresoftware/manticoresearch/commit/bd8b3c976ff8b4667f55dfdd101b20a920137ac5) 修复RT索引中提交大docid文档时崩溃
* [Commit ce65](https://github.com/manticoresoftware/manticoresearch/commit/ce656b83b747ce7c55795b53915770c13fb99cfe) 修复indextool中无参数选项
* [Commit 08c9](https://github.com/manticoresoftware/manticoresearch/commit/08c9507177820f441f534bf06a11dac5e54bebb4) 修复扩展关键字的内存泄漏

* [Commit 30c7](https://github.com/manticoresoftware/manticoresearch/commit/30c75a2f525ec9bda625d903acdc9d4d2e2413dc) 修复json分组器的内存泄漏
* [Commit 6023](https://github.com/manticoresoftware/manticoresearch/commit/6023f269b6f2080e4d380b0957605ef8107c8c9f) 修复全局用户变量泄漏
* [Commit 7c13](https://github.com/manticoresoftware/manticoresearch/commit/7c138f15ca23c0c8717fa12041e3db7f988fcf15) 修复早期拒绝匹配时动态字符串泄漏
* [Commit 9154](https://github.com/manticoresoftware/manticoresearch/commit/9154b18eaed5302e8330cb3a95bd968959ccb312) 修复了 length(<expression>) 的泄漏问题
* [Commit 43fc](https://github.com/manticoresoftware/manticoresearch/commit/43fca3a4e26139765d0fac8de054200dc4875fc6) 修复了解析器中 strdup() 导致的内存泄漏
* [Commit 71ff](https://github.com/manticoresoftware/manticoresearch/commit/71ff77737e63d1019b7c9804dca7fa2351025dba) 重构表达式解析器以准确跟踪引用计数
## 版本 2.7.2 GA，2018年8月27日
### 改进
* 与 MySQL 8 客户端兼容
* 支持带 RECONFIGURE 的 [TRUNCATE](Emptying_a_table.md)
* 取消了 RT 索引的 SHOW STATUS 内存计数器
* 多代理的全局缓存

* 改进了 Windows 平台上的 IOCP
* HTTP 协议的 VIP 连接
* Manticore SQL [DEBUG](Reporting_bugs.md#DEBUG) 命令，可运行各种子命令
* [shutdown_token](Server_settings/Searchd.md#shutdown_token) - 调用 DEBUG 命令执行 `shutdown` 所需的密码的 SHA1 哈希
* 新增 SHOW AGENT STATUS 的统计信息 (_ping, _has_perspool, _need_resolve)
* indexer 的 --verbose 选项现在支持 \[debugvv\] 用于打印调试信息
### 修复
* [Commit 3900](https://github.com/manticoresoftware/manticoresearch/commit/390082a7be0a1f9539b30361d11d54de35c62a44) 移除了优化时的写锁
* [Commit 4c33](https://github.com/manticoresoftware/manticoresearch/commit/4c337648329be22373be33333cbc9c5664d18b80) 修复了重新加载索引设置时写锁的问题
* [Commit b5ea](https://github.com/manticoresoftware/manticoresearch/commit/b5ea8dc0b18a23dbd739e92c66dd93b0094df14b) 修复了 JSON 过滤器查询时的内存泄漏
* [Commit 930e](https://github.com/manticoresoftware/manticoresearch/commit/930e831a43fd3d1ce996185be968d1df49a088d0) 修复了 PQ 结果集中的空文档问题
* [Commit 53de](https://github.com/manticoresoftware/manticoresearch/commit/53deecfecb374431f84f8592a1095a77407b8aea) 修正了因已删除任务导致的任务混淆
* [Commit cad9](https://github.com/manticoresoftware/manticoresearch/commit/cad9b9c617719aad97e9f5a2ae93491153d8e504) 修复了错误的远程主机计数
* [Commit 9000](https://github.com/manticoresoftware/manticoresearch/commit/90008cc15c4684d445d37a391bb4a667bbc2e365) 修复了解析代理描述符时的内存泄漏
* [Commit 978d](https://github.com/manticoresoftware/manticoresearch/commit/978d839641ad6040c22c9fc3b703b1a02685f266) 修复了搜索中的泄漏
* [Commit 0193](https://github.com/manticoresoftware/manticoresearch/commit/0193946105ac08913362b7d7acf0c90f9b38dc85) 对显式/内联 c-trs、override/final 用法进行了美化修改
* [Commit 943e](https://github.com/manticoresoftware/manticoresearch/commit/943e2997b33f9b2eda23d94a66068910f9476ebd) 修复了本地/远程模式中 JSON 的泄漏
* [Commit 02db](https://github.com/manticoresoftware/manticoresearch/commit/02dbdd6f1d0b72e7e458ebebdd2c67f989577e6b) 修复了本地/远程模式中 JSON 排序列表达式的泄漏
* [Commit c74d](https://github.com/manticoresoftware/manticoresearch/commit/c74d0b4035e4214510376845b22cb676a8da24a3) 修复了常量别名的泄漏
* [Commit 6e5b](https://github.com/manticoresoftware/manticoresearch/commit/6e5b57e1d3952c9695376728a143bf5434208f53) 修复了预读线程的泄漏
* [Commit 39c7](https://github.com/manticoresoftware/manticoresearch/commit/39c740fef5bf71da0bee60a35f5dd9f471af850f) 修复了因网络循环中等待阻塞导致的退出卡死
* [Commit adaf](https://github.com/manticoresoftware/manticoresearch/commit/adaf9772f95772ad6d5297acace6c5cb92b56ad5) 修复了将高可用代理切换为普通主机时 'ping' 行为的卡死
* [Commit 32c4](https://github.com/manticoresoftware/manticoresearch/commit/32c40eac9cdd40b15d79e58b2f15416f164f9701) 对仪表盘存储进行了单独的垃圾回收
* [Commit 511a](https://github.com/manticoresoftware/manticoresearch/commit/511a3cf2c1f1858f6e94fe9f8175b7216db3cbd6) 修复了引用计数指针的问题
* [Commit 32c4](https://github.com/manticoresoftware/manticoresearch/commit/af2326cd2927effbad7bec88e8dc238a566cf27e) 修复了 indextool 在索引不存在时崩溃
* [Commit 156e](https://github.com/manticoresoftware/manticoresearch/commit/156edc717dc63d3ea120f42466eba6e022da88be) 修复了 xmlpipe 索引中超出属性/字段的输出名称
* [Commit cdac](https://github.com/manticoresoftware/manticoresearch/commit/cdac6d1f17d384bfc8bae49e91241b2f346c3830) 修复了配置中无索引器节时默认索引器的值
* [Commit e61e](https://github.com/manticoresoftware/manticoresearch/commit/e61ec00b6b27d1d5878247e2ee817f3b1e7fde16) 修复了服务器重启后 RT 索引磁盘块中错误的嵌入停用词

* [Commit 5fba](https://github.com/manticoresoftware/manticoresearch/commit/5fba49d5e28c17de4c0acbd984466127f42de6e8) 修复了跳过虚假（已关闭但未从轮询器中最终删除）连接
* [Commit f22a](https://github.com/manticoresoftware/manticoresearch/commit/f22ae34b623906b3c5a05a06198dad2e548dc541) 修复了混合（孤立）网络任务
* [Commit 4689](https://github.com/manticoresoftware/manticoresearch/commit/46890e70eb8dcd00c1bf1b030538b8f131c601c2) 修复了写操作后读取动作崩溃
* [Commit 03f9](https://github.com/manticoresoftware/manticoresearch/commit/03f9df904f611c438d5ebcaaeef911b0dc8d6c39) 修复了 Windows 测试运行时 searchd 崩溃
* [Commit e925](https://github.com/manticoresoftware/manticoresearch/commit/e9255ed2704790a3d7f5273d38b85433463cb3ff) 修复了常规 connect() 中的 EINPROGRESS 处理
* [Commit 248b](https://github.com/manticoresoftware/manticoresearch/commit/248b72f1544b27e9e82773284050d44eeb1714e1) 修复了使用 TFO 时的连接超时

## 版本 2.7.1 GA，2018年7月4日
### 改进
* 改进了 PQ 中匹配多个文档时通配符的性能
* 支持 PQ 中的全表扫描查询
* 支持 PQ 中的 MVA 属性


* 为 percolate 索引添加了 regexp 和 RLP 支持
### 修复
* [Commit 6885](https://github.com/manticoresoftware/manticoresearch/commit/68856261b41f6950666f9e5122726839fb3c71d0) 修复了查询字符串丢失问题
* [Commit 0f17](https://github.com/manticoresoftware/manticoresearch/commit/0f1770943cba53bfd2e0edfdf0b1495ff0dd0e89) 修复了 SHOW THREADS 语句中空信息的问题
* [Commit 53fa](https://github.com/manticoresoftware/manticoresearch/commit/53faa36a916bb87868e83aa79c1a1f972fb20ca0) 修复了使用 NOTNEAR 操作符匹配时的崩溃问题
* [Commit 2602](https://github.com/manticoresoftware/manticoresearch/commit/26029a7cd77b518fb3a27588d7db6c8bffc73b0f) 修复了对错误过滤器进行 PQ 删除时的错误信息
## 版本 2.7.0 GA，2018年6月11日
### 改进
* 减少系统调用次数以避免 Meltdown 和 Spectre 补丁的影响
* 本地索引管理的内部重写
* 远程摘要重构
* 完整配置重新加载
* 所有节点连接现在都是独立的
* 协议改进
* Windows 通信从 wsapoll 切换到 IO 完成端口

* 主节点与节点间通信可以使用 TFO
* [SHOW STATUS](Node_info_and_management/Node_status.md#SHOW-STATUS) 现在输出服务器版本和 mysql_version_string
* 为 CALL PQ 中调用的文档添加了 `docs_id` 选项。
* percolate 查询过滤器现在可以包含表达式
* 分布式索引可以与 FEDERATED 一起使用
* 虚拟的 SHOW NAMES COLLATE 和 `SET wait_timeout`（为更好地兼容 ProxySQL）
### Bug 修复
* [Commit 5bcf](https://github.com/manticoresoftware/manticoresearch/commit/5bcff05d19e495f1a44f190885b25cbb6f217c4a) 修复了添加不等于 PQ 标签的问题
* [Commit 9ebc](https://github.com/manticoresoftware/manticoresearch/commit/9ebc58916cd515eaa88da66d0895aebf0d1f2b5f) 修复了向 JSON 文档的 CALL PQ 语句中添加文档 ID 字段的问题
* [Commit 8ae0](https://github.com/manticoresoftware/manticoresearch/commit/8ae0e593a286110d8505d88baa3ac9e433cb38c9) 修复了对 PQ 索引的 flush 语句处理器
* [Commit c24b](https://github.com/manticoresoftware/manticoresearch/commit/c24b152344ecd77661566cddb803487f51d3c1aa) 修复了对 JSON 和字符串属性的 PQ 过滤
* [Commit 1b8b](https://github.com/manticoresoftware/manticoresearch/commit/1b8bdde19aaa362785ea4c33bdc019154f8fbac4) 修复了解析空 JSON 字符串的问题


* [Commit 1ad8](https://github.com/manticoresoftware/manticoresearch/commit/1ad8a07dbfc1131913a8d92c261fbb48f934e8b7) 修复了多查询带 OR 过滤器时的崩溃问题
* [Commit 69b8](https://github.com/manticoresoftware/manticoresearch/commit/69b89806380a229e36287ad9daf6f0ea2b5453eb) 修复了 indextool 在命令（dumpheader）中使用配置通用部分（lemmatizer_base 选项）
* [Commit 6dbe](https://github.com/manticoresoftware/manticoresearch/commit/6dbeaf2389d2af6a9dfccb56bfc986685b9f203e) 修复了结果集和过滤器中的空字符串
* [Commit 39c4](https://github.com/manticoresoftware/manticoresearch/commit/39c4eb55ecc85f6dd54f6c8c6d6dfcf87fd8748e) 修复了负文档 ID 值
* [Commit 266b](https://github.com/manticoresoftware/manticoresearch/commit/266b7054f98e7d30fc2829b5c2467dc9f90def92) 修复了对非常长词索引的词剪切长度
* [Commit 4782](https://github.com/manticoresoftware/manticoresearch/commit/47823b0bd06cbd016a24345c99bb8693790b43dd) 修复了 PQ 上通配符查询匹配多个文档的问题
## 版本 2.6.4 GA，2018年5月3日

### 新特性和改进
* MySQL FEDERATED 引擎的[支持](Extensions/FEDERATED.md)
* MySQL 数据包现在返回 SERVER_STATUS_AUTOCOMMIT 标志，增强了与 ProxySQL 的兼容性
* [listen_tfo](Server_settings/Searchd.md#listen_tfo) - 为所有监听器启用 TCP Fast Open 连接
* indexer --dumpheader 现在也可以从 .meta 文件中导出 RT 头部
* Ubuntu Bionic 的 cmake 构建脚本
### Bug 修复
* [Commit 355b](https://github.com/manticoresoftware/manticoresearch/commit/355b11629174813abd9cd5bf2233be0783f77745) 修复了 RT 索引的无效查询缓存条目；

* [Commit 546e](https://github.com/manticoresoftware/manticoresearch/commit/546e2297a01cc2913bc0d33052d5bcefae8737eb) 修复了无缝轮转后索引设置丢失的问题
* [Commit 0c45](https://github.com/manticoresoftware/manticoresearch/commit/0c4509898393993bc87194d2aca2070395ff7f83) 修正了中缀与前缀长度设置；添加了对不支持中缀长度的警告
* [Commit 8054](https://github.com/manticoresoftware/manticoresearch/commit/80542fa54d727c781635918e8d4d65a20c2fbd1e) 修复了 RT 索引的自动刷新顺序

* [Commit 705d](https://github.com/manticoresoftware/manticoresearch/commit/705d8c5f407726ed1c6f9745ecffa375682c8969) 修复了具有多个属性和多个索引查询的索引结果集架构问题
* [Commit b0ba](https://github.com/manticoresoftware/manticoresearch/commit/b0ba932fff4a082f642cb737381c8a70215d4cc9) 修复了批量插入时重复文档导致部分命中丢失的问题
* [Commit 4510](https://github.com/manticoresoftware/manticoresearch/commit/4510fa44ad54b5bbac53c591144501565130fcae) 修复了带大文档计数的 RT 索引优化失败合并磁盘块的问题
## 版本 2.6.3 GA，2018年3月28日
### 改进
* 在编译时支持 jemalloc。如果系统中存在 jemalloc，可以通过 cmake 参数 `-DUSE_JEMALLOC=1` 启用


### Bug 修复
* [Commit 85a6](https://github.com/manticoresoftware/manticoresearch/commit/85a6d7e3db75f28c491042409bb677ed39eba07b) 修复了将 expand_keywords 选项记录到 Manticore SQL 查询日志中
* [Commit caaa](https://github.com/manticoresoftware/manticoresearch/commit/caaa3842f4dbd8507fd9e20bf3371a0c8ec0c087) 修复了 HTTP 接口无法正确处理大型查询的问题
* [Commit e386](https://github.com/manticoresoftware/manticoresearch/commit/e386d842b36195719e459b2db1fb20e402f9a51f) 修复了启用 index_field_lengths 时 DELETE RT 索引时服务器崩溃
* [Commit cd53](https://github.com/manticoresoftware/manticoresearch/commit/ce528f37eb5d8284661928f6f3b340a4a1985253) 修复了 cpustats searchd CLI 选项在不支持的系统上工作的问题
* [Commit 8740](https://github.com/manticoresoftware/manticoresearch/commit/8740fd63ae842c2ab0e51a8fcd6180d787ed57ec) 修复了定义了最小长度的 utf8 子字符串匹配问题
## 版本 2.6.2 GA，2018年2月23日
### 改进

* 改进了在使用 NOT 操作符和批量文档时的 [Percolate Queries](Searching/Percolate_query.md) 性能。
* [percolate_query_call](Searching/Percolate_query.md) 可以根据 [dist_threads](Server_settings/Searchd.md#threads) 使用多线程。
* 新增全文匹配操作符 NOTNEAR/N。
* 对 percolate 索引的 SELECT 支持 LIMIT。
* [expand_keywords](Searching/Options.md#expand_keywords) 可以接受 'start','exact'（其中 'start,exact' 的效果与 '1' 相同）。
* 支持范围主查询的 [joined fields](Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Processing_fetched_data.md#sql_joined_field)，使用由 sql_query_range 定义的范围查询。
### Bug 修复
* [Commit 72dc](https://github.com/manticoresoftware/manticoresearch/commit/72dcf669744e9b7d636dfc213d24df85ab301f6b) 修复了搜索 RAM 段时的崩溃；修复了使用双缓冲保存磁盘块时的死锁；修复了优化期间保存磁盘块的死锁。
* [Commit 3613](https://github.com/manticoresoftware/manticoresearch/commit/36137149a1c3c0893bdda5a28fc7e8244bf2d4ae) 修复了索引器对带有空属性名的 XML 嵌套模式崩溃问题。


* [Commit 48d7](https://github.com/manticoresoftware/manticoresearch/commit/48d7e8001d2a66466ca64577f27ddc5421a67251) 修复了错误地取消链接非所有的 pid 文件的问题。
* [Commit a556](https://github.com/manticoresoftware/manticoresearch/commit/a5563a465ddc59ef71e65f17b68bc33f9700e838) 修复了有时遗留在临时文件夹中的孤立 FIFO 问题。
* [Commit 2376](https://github.com/manticoresoftware/manticoresearch/commit/2376e8fc4508944b96959bd10686c6d51f5145e8) 修复了 FACET 返回空结果集时包含错误的 NULL 行问题。
* [Commit 4842](https://github.com/manticoresoftware/manticoresearch/commit/4842b67acdd5da75672db6c08ac563b48577ddd7) 修复了以 Windows 服务运行服务器时索引锁损坏的问题。
* [Commit be35](https://github.com/manticoresoftware/manticoresearch/commit/be35feef54ded59125750916cd211e56108eddae) 修复了 macOS 上错误的 iconv 库问题。
* [Commit 8374](https://github.com/manticoresoftware/manticoresearch/commit/83744a977d7837d1d87cd506b88b0e7eb199efe6) 修复了错误的 count(*) 计数问题。
## 版本 2.6.1 GA，2018 年 1 月 26 日
### 改进

* [agent_retry_count](Server_settings/Searchd.md#agent_retry_count) 在有镜像的代理情况下，返回每个镜像的重试次数，而非每个代理，总重试次数为 agent_retry_count\*mirrors。
* [agent_retry_count](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_retry_count) 现在可针对每个索引单独指定，覆盖全局值。同时新增别名 [mirror_retry_count](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#mirror_retry_count)。
* 现在可以在代理定义中指定 retry_count，表示每个代理的重试次数。
* Percolate Queries 现支持通过 HTTP JSON API 的 [/json/pq](Searching/Percolate_query.md#Performing-a-percolate-query-with-CALL-PQ) 访问。
* 给可执行文件新增了 -h 和 -v 选项（帮助和版本）。
* [morphology_skip_fields](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) 支持实时索引。
### Bug 修复


* [Commit a40b](https://github.com/manticoresoftware/manticore/commit/a40b0793feff65e40d10062568d9847c08d10f57) 修复了在 MVA 字段使用 sql_range_step 时范围主查询的正确行为。
* [Commit f2f5](https://github.com/manticoresoftware/manticore/commit/f2f53757db45bcfb1544263ce0817e856656a621) 修复了黑洞系统循环假死和黑洞代理似乎断开的问题。
* [Commit 84e1](https://github.com/manticoresoftware/manticore/commit/84e1f54aef25e0fce98870ad2dd784db5116f1d6) 修复了查询 ID 不一致和存储查询 ID 重复的问题。
* [Commit 1948](https://github.com/manticoresoftware/manticore/commit/19484231814fcb82b21763a3a4a9f45adc6b2d40) 修复了服务器从多种状态关闭时的崩溃。
* [Commit 9a70](https://github.com/manticoresoftware/manticore/commit/9a706b499a1d61a90076065a1a703029d49db958) 以及 [Commit 3495](https://github.com/manticoresoftware/manticore/commit/3495fd70cba8846b1a50d55d6679b039414c3d2a) 修复了长查询超时问题。
* [Commit 3359](https://github.com/manticoresoftware/manticore/commit/3359bcd89b4f79a645fe84b8cf8616ce0addff02) 重构了基于 kqueue 的系统（Mac OS X、BSD）上的主代理网络轮询。
## 版本 2.6.0，2017 年 12 月 29 日

### 新特性及改进
* [HTTP JSON](Connecting_to_the_server/HTTP.md)：JSON 查询现在支持属性上的等值比较，MVA 和 JSON 属性可以在插入和更新中使用，分布式索引支持通过 JSON API 进行更新和删除。
* [Percolate Queries](Searching/Percolate_query.md)。
* 移除了对 32 位 docid 的支持，也移除了所有用于转换/加载带有 32 位 docid 的旧版索引的代码。
* [仅对特定字段应用形态学](https://github.com/manticoresoftware/manticore/issues/7)。新增索引指令 morphology_skip_fields，允许定义形态学不适用的字段列表。
* [expand_keywords 现在可以作为查询运行时指令通过 OPTION 语句设置](https://github.com/manticoresoftware/manticore/issues/8)。

### Bug 修复
* [Commit 0cfa](https://github.com/manticoresoftware/manticore/commit/0cfae4c) 修复了在启用 rlp 编译服务器的调试版本时崩溃（可能在发布版存在未定义行为）。

* [Commit 3242](https://github.com/manticoresoftware/manticore/commit/324291e) 修复了启用 progressive 选项时 RT 索引优化合并 kill-lists 顺序错误问题。
* [Commit ac0e](https://github.com/manticoresoftware/manticore/commit/ac0efee) 修复了 mac 上的轻微崩溃。
* 经过全面静态代码分析后的众多轻微修复。
* 其他轻微 Bug 修复。
### 升级说明
本版本更改了主节点和代理之间通信的内部协议。如果您在分布式环境中运行多实例 Manticoresearch，请确保先升级代理，然后升级主节点。
## 版本 2.5.1，2017 年 11 月 23 日
### 新特性及改进
* JSON 查询基于 [HTTP API 协议](Connecting_to_the_server/HTTP.md)。支持搜索、插入、更新、删除、替换操作。数据操作命令也可以批量执行，但目前存在一些限制，如 MVA 和 JSON 属性不能用于插入、替换或更新。
* [RELOAD INDEXES](Data_creation_and_modification/Adding_data_from_external_storages/Rotating_a_table.md#RELOAD-TABLES) 命令
* [FLUSH LOGS](Logging/Rotating_query_and_server_logs.md) 命令

* [SHOW THREADS](Node_info_and_management/SHOW_THREADS.md) 可以显示优化、轮转或刷新进度。
* GROUP N BY 正确支持 MVA 属性
* blackhole 代理运行在独立线程，不再影响主查询
* 对索引实现了引用计数，以避免轮转和高负载导致的停顿
* 实现了 SHA1 哈希，尚未对外暴露
* 修复了在 FreeBSD、macOS 和 Alpine 上的编译问题
### Bugfixes
* [Commit 9897](https://github.com/manticoresoftware/manticore/commit/989752b) 修复了基于块索引的过滤器回归问题

* [Commit b1c3](https://github.com/manticoresoftware/manticore/commit/b1c3864) 将 PAGE_SIZE 重命名为 ARENA_PAGE_SIZE，以兼容 musl
* [Commit f213](https://github.com/manticoresoftware/manticore/commit/f2133cc) 针对 cmake 版本小于 3.1.0 禁用 googletests
* [Commit f30e](https://github.com/manticoresoftware/manticore/commit/0839de7) 修复服务器重启时绑定套接字失败
* [Commit 0807](https://github.com/manticoresoftware/manticore/commit/0807240) 修复服务器关闭时崩溃
* [Commit 3e3a](https://github.com/manticoresoftware/manticore/commit/3e3acc3) 修复了显示黑洞线程的 show threads 问题
* [Commit 262c](https://github.com/manticoresoftware/manticore/commit/262c3fe) 重构了 iconv 的配置检查，修复了在 FreeBSD 和 Darwin 上的构建问题
## 版本 2.4.1 GA，2017年10月16日
### 新特性和改进
* WHERE 子句中属性过滤器间支持 OR 运算符
* 维护模式（ SET MAINTENANCE=1）
* [CALL KEYWORDS](Searching/Autocomplete.md#CALL-KEYWORDS) 支持分布式索引
* [以 UTC 进行分组](Server_settings/Searchd.md#grouping_in_utc)
* [query_log_mode](Server_settings/Searchd.md#query_log_mode) 用于自定义日志文件权限
* 字段权重可为零或负值

* [max_query_time](Searching/Options.md#max_query_time) 现在也可影响全扫描
* 新增 [net_wait_tm](Server_settings/Searchd.md#net_wait_tm)、[net_throttle_accept](Server_settings/Searchd.md#net_throttle_accept) 和 [net_throttle_action](Server_settings/Searchd.md#net_throttle_action) 用于网络线程微调（当 workers=thread_pool 时）

* COUNT DISTINCT 支持 facet 搜索
* IN 现在支持 JSON 浮点数组
* 多查询优化不再被整数/浮点表达式破坏
* [SHOW META](Node_info_and_management/SHOW_META.md) 在多查询优化时显示 `multiplier` 行
### 编译

Manticore Search 使用 cmake 构建，编译所需最低 gcc 版本为 4.7.2。
### 目录和服务
* Manticore Search 以 `manticore` 用户身份运行。
* 默认数据目录现在为 `/var/lib/manticore/`。
* 默认日志目录现在为 `/var/log/manticore/`。
* 默认 pid 目录现在为 `/var/run/manticore/`。
### Bugfixes
* [Commit a58c](https://github.com/manticoresoftware/manticore/commit/a58c619) 修复了导致 Java 连接器错误的 SHOW COLLATION 语句
* [Commit 631c](https://github.com/manticoresoftware/manticore/commit/631cf4e) 修复了处理分布式索引时的崩溃问题；为分布式索引哈希添加锁；移除了 agent 的移动和复制操作符
* [Commit 942b](https://github.com/manticoresoftware/manticore/commit/942bec0) 修复了因并行重连导致的分布式索引处理崩溃
* [Commit e5c1](https://github.com/manticoresoftware/manticore/commit/e5c1ed2) 修复了崩溃处理程序在记录查询到服务器日志时的崩溃问题
* [Commit 4a4b](https://github.com/manticoresoftware/manticore/commit/4a4bda5) 修复了多查询时池化属性导致的崩溃
* [Commit 3873](https://github.com/manticoresoftware/manticore/commit/3873bfb) 通过防止索引页被包含进核心文件，减少了核心文件大小
* [Commit 11e6](https://github.com/manticoresoftware/manticore/commit/11e6254) 修复了在指定无效代理时 searchd 启动崩溃的问题
* [Commit 4ca6](https://github.com/manticoresoftware/manticore/commit/4ca6350) 修复了 indexer 在 sql_query_killlist 查询中报告错误
* [Commit 123a](https://github.com/manticoresoftware/manticore/commit/123a9f0) 修复 fold_lemmas=1 与命中计数的冲突
* [Commit cb99](https://github.com/manticoresoftware/manticore/commit/cb99164) 修复 html_strip 行为不一致的问题
* [Commit e406](https://github.com/manticoresoftware/manticore/commit/e406761) 修复 optimize rt 索引丢失新设置；修复带同步选项的 optimize 锁泄漏

* [Commit 86ae](https://github.com/manticoresoftware/manticore/commit/86aeb82) 修复错误多查询的处理
* [Commit 2645](https://github.com/manticoresoftware/manticore/commit/2645230) 修复结果集依赖多查询顺序的问题

