# 更新日志

## 开发版本

* 🪲 [v13.6.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.5) [ Issue #3467](https://github.com/manticoresoftware/manticoresearch/issues/3467) 修复了DBeaver及其他集成因“unknown sysvar”错误导致失败的问题。
* 🪲 [v13.6.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.4) [ Issue #3524](https://github.com/manticoresoftware/manticoresearch/issues/3524) 修复了多字段嵌入拼接的问题；同时修复了查询中嵌入生成的问题。
* 🪲 [v13.6.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.3) [ Issue #3641](https://github.com/manticoresoftware/manticoresearch/issues/3641) 修复了13.6.0版本中短语丢失除第一个之外所有括号关键词的错误。
* 🪲 [v13.6.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.2) 修复了transform_phrase中的内存泄漏。
* 🪲 [v13.6.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.1) 修复了13.6.0版本中的内存泄漏。
* 🆕 [v13.6.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.0) [ Issue #2226](https://github.com/manticoresoftware/manticoresearch/issues/2226) 支持在PHRASE、PROXIMITY和QUORUM操作符中显式使用 '|'（或）。
* 🪲 [v13.5.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.5.2) [ Issue #3651](https://github.com/manticoresoftware/manticoresearch/issues/3651) 修复了更多与全文搜索模糊测试相关的问题。
* 🪲 [v13.5.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.5.1) [ Issue #3560](https://github.com/manticoresoftware/manticoresearch/issues/3560) 修复了OPTIMIZE TABLE在处理KNN数据时可能无限挂起的情况。
* 🆕 [v13.5.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.5.0) [ PR #3591](https://github.com/manticoresoftware/manticoresearch/pull/3591) 查询中自动生成嵌入（正在开发中）。
* 🪲 [v13.4.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.4.2) [ Issue #2544](https://github.com/manticoresoftware/manticoresearch/issues/2544) 修正了添加float_vector列时可能破坏索引的问题。
* 🪲 [v13.4.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.4.1) 增加全文解析的模糊测试，并修复测试中发现的多个问题。
* 🆕 [v13.4.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.4.0) [ PR #3585](https://github.com/manticoresoftware/manticoresearch/pull/3585) 修正了当设置了buddy_path配置时，优先使用该配置的buddy线程数的逻辑，而非守护进程值。
* 🪲 [v13.3.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.3.1) [ Issue #3583](https://github.com/manticoresoftware/manticoresearch/issues/3583) 修复了使用复杂布尔过滤器与高亮显示时崩溃的问题。
* 🆕 [v13.3.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.3.0) [ PR #3577](https://github.com/manticoresoftware/manticoresearch/pull/3577) 支持与本地分布式表的连接。
* 🪲 [v13.2.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.7) [ Issue #3481](https://github.com/manticoresoftware/manticoresearch/issues/3481) 修复了在HTTP更新、分布式表和错误复制集群同时使用时崩溃的问题。
* 🪲 [v13.2.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.6) [ PR #3567](https://github.com/manticoresoftware/manticoresearch/pull/3567) 将manticore-backup依赖更新至1.9.6版本。
* 🪲 [v13.2.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.5) [ PR #3565](https://github.com/manticoresoftware/manticoresearch/pull/3565) 修正了CI配置以提升Docker镜像兼容性。
* 🪲 [v13.2.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.4) 修正了长词的处理。有些特殊词（如正则表达式模式）可能生成过长的词，因此现在使用前会被截短。

## 版本 13.2.3
**发布日期**：2025年7月8日

### 不兼容更改
* ⚠️ [v13.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.0.0) 更新了KNN库API以支持空的 [float_vector](https://manual.manticoresearch.com/Creating_a_table/Data_types#Float-vector) 值。此更新不改变数据格式，但提高了Manticore Search / MCL API版本。
* ⚠️ [v12.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/12.0.0) [ PR #3516](https://github.com/manticoresoftware/manticoresearch/pull/3516) 修复了KNN索引训练和构建过程中错误的源和目标行ID的bug。此更新不改变数据格式，但提高了Manticore Search / MCL API版本。
* ⚠️ [v11.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.0.0) 新增对KNN向量搜索的新功能支持，如量化、再评分和过采样。该版本改变了KNN数据格式和 [KNN_DIST() SQL语法](https://manual.manticoresearch.com/Searching/KNN?client=SQL#KNN-vector-search)。新版可以读取旧数据，但旧版不能读取新版格式。

### 新功能和改进
* 🆕 [v13.2.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.0) [ PR #3549](https://github.com/manticoresoftware/manticoresearch/issues/3525) 修正了 `@@collation_database` 的问题，该问题导致与某些mysqldump版本不兼容。
* 🆕 [v13.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.1.0) [ Issue #3489](https://github.com/manticoresoftware/manticoresearch/issues/3489) 修复了模糊搜索中的bug，该bug导致某些SQL查询无法解析。
* 🆕 [v12.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/12.1.0) [ Issue #3426](https://github.com/manticoresoftware/manticoresearch/issues/3426) 新增对RHEL 10操作系统的支持。
* 🆕 [v11.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.1.0)  在 [KNN 搜索](https://manual.manticoresearch.com/Searching/KNN#KNN-vector-search) 中添加了对空浮点向量的支持
* 🆕 [v10.2.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.0) [ Issue #3252](https://github.com/manticoresoftware/manticoresearch/issues/3252) [log_level](https://manual.manticoresearch.com/Server_settings/Setting_variables_online#SET) 现在也调整 Buddy 的日志详细级别

### Bug 修复
* 🪲 [v13.2.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.3) [ PR #3556](https://github.com/manticoresoftware/manticoresearch/pull/3556) 修正了 JSON 查询中 “oversampling” 选项的解析
* 🪲 [v13.2.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.2)  通过移除 Boost stacktrace 的使用，修复了 Linux 和 FreeBSD 上的崩溃日志问题
* 🪲 [v13.2.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.1) [ Issue #3298](https://github.com/manticoresoftware/manticoresearch/issues/3298) 修复了容器内运行时的崩溃日志问题
* 🪲 [v12.0.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/12.0.2)  通过以微秒计数，提高了每表统计数据的准确性
* 🪲 [v12.0.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/12.0.1) [ PR #3522](https://github.com/manticoresoftware/manticoresearch/pull/3522) 修复了联接查询中基于 MVA 的分面时的崩溃
* 🪲 [v11.0.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.0.3) [ Issue #3502](https://github.com/manticoresoftware/manticoresearch/issues/3502) 修复了与向量搜索量化相关的崩溃
* 🪲 [v11.0.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.0.2) [ Issue #3493](https://github.com/manticoresoftware/manticoresearch/issues/3493) 将 `SHOW THREADS` 修改为以整数显示 CPU 利用率
* 🪲 [v11.0.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.0.1)  修复了列式和二级库的路径问题
* 🪲 [v10.2.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.7)  添加了对 MCL 5.0.5 的支持，包括修复嵌入库文件名的问题
* 🪲 [v10.2.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.6) [ Issue #3469](https://github.com/manticoresoftware/manticoresearch/issues/3469) 另行修复了与问题 #3469 相关的问题
* 🪲 [v10.2.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.4) [ Issue #3469](https://github.com/manticoresoftware/manticoresearch/issues/3469) 修复了 HTTP 请求中带布尔查询返回空结果的问题
* 🪲 [v10.2.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.5)  更改了嵌入库的默认文件名，并添加了对 KNN 向量中 'from' 字段的检查
* 🪲 [v10.2.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.3) [ PR #3464](https://github.com/manticoresoftware/manticoresearch/pull/3464) 将 Buddy 更新至 3.30.2 版本，其中包含 [PR #565 关于内存使用和错误日志](https://github.com/manticoresoftware/manticoresearch-buddy/pull/565)
* 🪲 [v10.2.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.2) [ Issue #3470](https://github.com/manticoresoftware/manticoresearch/pull/3470) 修复了联接查询中 JSON 字符串过滤器、空过滤器和排序问题
* 🪲 [v10.2.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.1)  修复了 `dist/test_kit_docker_build.sh` 中导致 Buddy 提交缺失的错误
* 🪲 [v10.1.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.4)  修复了联接查询中基于 MVA 的分组时崩溃的问题
* 🪲 [v10.1.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.3) [ Issue #3434](https://github.com/manticoresoftware/manticoresearch/issues/3434) 修复了过滤空字符串的错误
* 🪲 [v10.1.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.2) [ PR #3452](https://github.com/manticoresoftware/manticoresearch/pull/3452) 将 Buddy 更新至 3.29.7 版本，解决了 [Buddy #507 - 多查询请求模糊搜索错误](https://github.com/manticoresoftware/manticoresearch-buddy/issues/507) 和 [Buddy #561 - 指标速率修复](https://github.com/manticoresoftware/manticoresearch-buddy/pull/561)，该版本为 [Helm 10.1.0 版本](https://github.com/manticoresoftware/manticoresearch-helm/releases/tag/manticoresearch-10.1.0) 所需
* 🪲 [v10.1.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.1)  将 Buddy 更新至 3.29.4 版本，解决了 [#3388 - “未定义的数组键 'Field'”](https://github.com/manticoresoftware/manticoresearch/issues/3388) 和 [Buddy #547 - layouts='ru' 可能无法工作](https://github.com/manticoresoftware/manticoresearch-buddy/issues/547)

## 版本 10.1.0
**发布**：2025年6月9日

该版本是一次带有新功能的更新，包含一个破坏性变更以及大量稳定性改进和错误修复。更改重点在于增强监控能力、改进搜索功能以及修复影响系统稳定性和性能的多个关键问题。

**从版本 10.1.0 起，不再支持 CentOS 7。建议用户升级到受支持的操作系统。**

### 破坏性变更
* ⚠️ [v10.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.0.0) [ Issue #540](https://github.com/manticoresoftware/manticoresearch-buddy/issues/540) 破坏性变更：将 [模糊搜索](Searching/Spell_correction.md#Fuzzy-Search) 的默认 `layouts=''`

### 新功能和改进
* 🆕 [v10.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.0) [ Issue #537](https://github.com/manticoresoftware/manticoresearch-buddy/issues/537) 添加内置的 [Prometheus 导出器](Node_info_and_management/Node_status.md#Prometheus-Exporter)
* 🆕 [v9.8.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.8.0) [ Issue #3409](https://github.com/manticoresoftware/manticoresearch/issues/3409) 添加了 [ALTER TABLE tbl REBUILD KNN](Updating_table_schema_and_settings.md#Rebuilding-a-KNN-index)
* 🆕 [v9.7.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.7.0) [ Issue #1778](https://github.com/manticoresoftware/manticoresearch/issues/1778) 添加了自动生成嵌入（我们尚未正式公布，因为代码已在主分支，但仍需更多测试）
* 🆕 [v9.6.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.6.0)  提升了 KNN API 版本以支持自动嵌入
* 🆕 [v9.5.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.0) [ Issue #1894](https://github.com/manticoresoftware/manticoresearch/issues/1894) 改进了集群恢复：定期保存 `seqno`，加快崩溃后节点重启速度
* 🆕 [v9.4.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.4.0) [ Issue #2400](https://github.com/manticoresoftware/manticoresearch/issues/2400) 添加了对最新 [Logstash](Integration/Logstash.md#Integration-with-Logstash) 和 Beats 的支持

### Bug 修复
* 🪲 [v10.0.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.0.1)  修复词形处理：用户定义的形式现在会覆盖自动生成的形式；添加了测试用例至测试 22
* 🪲 [v9.8.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.8.2)  修复：再次更新 deps.txt 以包括 MCL 中与嵌入库相关的打包修复
* 🪲 [v9.8.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.8.1)  修复：更新 deps.txt，包含 MCL 和嵌入库的打包修复
* 🪲 [v9.7.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.7.3) [ Issue #3306](https://github.com/manticoresoftware/manticoresearch/issues/3306) 修复索引过程中出现的信号 11 崩溃
* 🪲 [v9.7.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.7.2) [ Issue #3109](https://github.com/manticoresoftware/manticoresearch/issues/3109) 修复不存在的 `@@variables` 总是返回 0 的问题
* 🪲 [v9.7.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.7.1) [ Issue #3377](https://github.com/manticoresoftware/manticoresearch/issues/3377) 修复与 [remove_repeats()](Functions/Searching_and_ranking_functions.md#REMOVE_REPEATS%28%29) 相关的崩溃
* 🪲 [v9.6.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.6.3) [ PR #3411](https://github.com/manticoresoftware/manticoresearch/pull/3411) 修复：对遥测指标使用动态版本检测
* 🪲 [v9.6.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.6.2)  修复：[SHOW VERSION](Node_info_and_management/SHOW_VERSION.md#SHOW-VERSION) 输出中的小问题
* 🪲 [v9.6.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.6.1)  修复：当创建包含 KNN 属性但无模型的表时崩溃
* 🪲 [v9.5.16](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.16) [ Issue #3342](https://github.com/manticoresoftware/manticoresearch/issues/3342) 修复 `SELECT ... FUZZY=0` 不总是禁用模糊搜索的问题
* 🪲 [v9.5.15](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.15) [ PR #3397](https://github.com/manticoresoftware/manticoresearch/pull/3397) 添加对 [MCL](https://github.com/manticoresoftware/columnar) 4.2.2 的支持；修复旧存储格式的错误
* 🪲 [v9.5.14](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.14) [ Issue #3392](https://github.com/manticoresoftware/manticoresearch/issues/3392) 修复 HTTP JSON 回复中字符串处理不正确的问题
* 🪲 [v9.5.13](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.13) [ Issue #3356](https://github.com/manticoresoftware/manticoresearch/issues/3356) 修复复杂全文查询（common-sub-term）情况下的崩溃
* 🪲 [v9.5.12](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.12)  修正磁盘块自动刷新错误信息中的拼写错误
* 🪲 [v9.5.11](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.11) [ Issue #3195](https://github.com/manticoresoftware/manticoresearch/issues/3195) 改进 [自动磁盘块刷新](Securing_and_compacting_a_table/Flushing_RAM_chunk_to_disk.md#Flushing-RAM-chunk-to-disk)：优化运行时跳过保存
* 🪲 [v9.5.10](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.10) [ Issue #3313](https://github.com/manticoresoftware/manticoresearch/issues/3313) 修复使用 [indextool](Miscellaneous_tools.md#indextool) 检查 RT 表所有磁盘块重复 ID 的问题
* 🪲 [v9.5.9](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.9) [ Issue #3132](https://github.com/manticoresoftware/manticoresearch/issues/3132) 添加对 JSON API 中 `_random` 排序的支持
* 🪲 [v9.5.8](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.8) [ Issue #3382](https://github.com/manticoresoftware/manticoresearch/issues/3382) 修复通过 JSON HTTP API 无法使用 uint64 文档 ID 的问题
* 🪲 [v9.5.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.7) [ Issue #3385](https://github.com/manticoresoftware/manticoresearch/issues/3385) 修复使用 `id != value` 过滤时结果不正确的问题
* 🪲 [v9.5.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.6) [ PR #538](https://github.com/manticoresoftware/manticoresearch-buddy/pull/538) 修复某些情况下模糊匹配的严重漏洞
* 🪲 [v9.5.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.5) [ Issue #3199](https://github.com/manticoresoftware/manticoresearch/issues/3199) 修复 Buddy HTTP 查询参数中的空格解码问题（例如 `%20` 和 `+`）
* 🪲 [v9.5.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.4) [ Issue #3133](https://github.com/manticoresoftware/manticoresearch/issues/3133) 修复了 facet 搜索中 `json.field` 的排序错误
* 🪲 [v9.5.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.3) [ Issue #3091](https://github.com/manticoresoftware/manticoresearch/issues/3091) 修复了 SQL 与 JSON API 中分隔符搜索结果不一致的问题
* 🪲 [v9.5.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.2) [ Issue #2819](https://github.com/manticoresoftware/manticoresearch/issues/2819) 性能提升：针对分布式表，用 `TRUNCATE` 替代了 `DELETE FROM`
* 🪲 [v9.5.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.1) [ Issue #3080](https://github.com/manticoresoftware/manticoresearch/issues/3080) 修复了使用 JSON 属性过滤别名 `geodist()` 时导致的崩溃

## 版本 9.3.2
发布日期：2025年5月2日

本次发布包括多个 bug 修复和稳定性改进，更好的表使用跟踪，以及内存和资源管理的增强。

❤️ 特别感谢 [@cho-m](https://github.com/cho-m) 修复了 Boost 1.88.0 的构建兼容性问题，感谢 [@benwills](https://github.com/benwills) 改进了关于 `stored_only_fields` 的文档。

* 🪲 [v9.3.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.3.2) 修复了“显示线程”列显示 CPU 活动为浮点数而非字符串的问题；还修复了因数据类型错误导致的 PyMySQL 结果集解析错误。
* 🪲 [v9.3.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.3.1) [ Issue #3343](https://github.com/manticoresoftware/manticoresearch/issues/3343) 修复优化过程被中断时遗留的 `tmp.spidx` 文件问题。
* 🆕 [v9.3.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.3.0) [ PR #3337](https://github.com/manticoresoftware/manticoresearch/pull/3337) 添加了每表命令计数器和详细的表使用统计。
* 🪲 [v9.2.39](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.39) [ Issue #3236](https://github.com/manticoresoftware/manticoresearch/issues/3236) 修复：通过移除复杂的块更新防止表破坏。串行工作线程内使用等待函数会破坏串行处理，可能导致表损坏。
重新实现自动刷新。移除外部轮询队列以避免不必要的表锁。新增“小表”判定条件：如果文档数量低于“小表限制”（8192）且未使用二级索引 (SI)，则跳过刷新。

* 🪲 [v9.2.38](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.38) 修复：跳过为使用 `ALL`/`ANY` 过滤器的字符串列表创建二级索引 (SI)，不影响 JSON 属性。
* 🪲 [v9.2.37](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.37) [ Issue #2898](https://github.com/manticoresoftware/manticoresearch/issues/2898) 为系统表添加反引号支持。
* 🪲 [v9.2.36](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.36) 修复：在旧代码中为集群操作使用占位符。解析器中明确区分了表名和集群名字段。
* 🪲 [v9.2.35](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.35) 修复：取消引用单个 `'` 时崩溃。
* 🪲 [v9.2.34](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.34) [ Issue #3090](https://github.com/manticoresoftware/manticoresearch/issues/3090) 修复：对大型文档 ID 的处理（之前可能找不到它们）。
* 🪲 [v9.2.33](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.33) 修复：bit 向量大小使用无符号整数。
* 🪲 [v9.2.32](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.32) 修复：合并期间降低峰值内存使用。docid 到 rowid 的查找现在每个文档用 12 字节而非 16 字节。例如：处理 20 亿文档时 RAM 从 36 GB 降至 24 GB。
* 🪲 [v9.2.31](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.31) [ Issue #3238](https://github.com/manticoresoftware/manticoresearch/issues/3238) 修复：大型实时表中 `COUNT(*)` 结果不正确。
* 🪲 [v9.2.30](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.30) 修复：零值字符串属性时的未定义行为。
* 🪲 [v9.2.29](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.29) 小修正：改进警告文本。
* 🪲 [v9.2.28](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.28) [ Issue #3290](https://github.com/manticoresoftware/manticoresearch/issues/3290) 改进：增强了 `indextool --buildidf`
* 🪲 [v9.2.27](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.27) [ Issue #3032](https://github.com/manticoresoftware/manticoresearch/issues/3032) 通过 Kafka 集成，现在可以为指定 Kafka 分区创建数据源。
* 🪲 [v9.2.26](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.26) [ Issue #3301](https://github.com/manticoresoftware/manticoresearch/issues/3301) 修复：对 `id` 使用 `ORDER BY` 和 `WHERE` 可能导致内存溢出 (OOM) 错误。
* 🪲 [v9.2.25](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.25) [ Issue #3171](https://github.com/manticoresoftware/manticoresearch/issues/3171) 修复：在 RT 表上使用带多个 JSON 属性的 grouper 时，多磁盘块导致的段错误崩溃。
* 🪲 [v9.2.24](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.24) [ Issue #3246](https://github.com/manticoresoftware/manticoresearch/issues/3246) 修复：RAM 块刷新后 `WHERE string ANY(...)` 查询失败。
* 🪲 [v9.2.23](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.23) [ PR #518](https://github.com/manticoresoftware/manticoresearch-buddy/pull/518) 小幅改进自动分片语法。
* 🪲 [v9.2.22](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.22) [ Issue #2763](https://github.com/manticoresoftware/manticoresearch/issues/2763) 修复：使用 `ALTER TABLE` 时未加载全局 idf 文件。
* 🪲 [v9.2.21](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.21) 修复：全局 idf 文件可能很大。我们现在更早释放表以避免占用不必要的资源。
* 🪲 [v9.2.20](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.20) [ PR #3277](https://github.com/manticoresoftware/manticoresearch/pull/3277) 改进：更好的分片选项验证。

* 🪲 [v9.2.19](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.19) [ PR #3275](https://github.com/manticoresoftware/manticoresearch/pull/3275) 修复：提升与 Boost 1.88.0 的构建兼容性。
* 🪲 [v9.2.18](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.18) [ Issue #3228](https://github.com/manticoresoftware/manticoresearch/issues/3228) 修复：分布式表创建时崩溃（无效指针问题）。

* 🪲 [v9.2.17](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.17) [ PR #3272](https://github.com/manticoresoftware/manticoresearch/pull/3272) 修复：多行模糊 `FACET` 问题。
* 🪲 [v9.2.16](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.16) [ Issue #3063](https://github.com/manticoresoftware/manticoresearch/issues/3063) 修复：使用 `GEODIST` 函数时距离计算的错误。
* 🪲 [v9.2.15](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.15) [ Issue #3027](https://github.com/manticoresoftware/manticoresearch/issues/3027) 小改进：支持 Elastic 的 `query_string` 过滤器格式。

## 版本 9.2.14
发布时间：2025年3月28日

### 小改动
* [提交](https://github.com/manticoresoftware/manticoresearch/commit/fe9473dc3f4fcfa0eaae748b538fe853f4ce078b) 实现了 `--mockstack` 标志，用于计算递归操作的堆栈需求。新的 `--mockstack` 模式分析并报告递归表达式求值、模式匹配操作、过滤器处理所需的堆栈大小。计算的堆栈需求输出到控制台，用于调试和优化。
* [Issue #3058](https://github.com/manticoresoftware/manticoresearch/pull/3058) 默认启用 [boolean_simplify](Searching/Options.md#boolean_simplify)。
* [Issue #3172](https://github.com/manticoresoftware/manticoresearch/issues/3172) 添加了新配置选项：`searchd.kibana_version_string`，当使用期望特定 Elasticsearch 版本的 Kibana 或 OpenSearch Dashboards 版本时，这一选项非常有用。
* [Issue #3211](https://github.com/manticoresoftware/manticoresearch/issues/3211) 修复了 [CALL SUGGEST](Searching/Spell_correction.md#CALL-QSUGGEST,-CALL-SUGGEST) 对两字符词的支持问题。
* [Issue #490](https://github.com/manticoresoftware/manticoresearch-buddy/issues/490) 改进了 [模糊搜索](Searching/Spell_correction.md#Fuzzy-Search)：之前在存在其他匹配文档时，搜索 "def ghi" 有时找不到 "defghi"。
* ⚠️ 重大变更 [Issue #3165](https://github.com/manticoresoftware/manticoresearch/issues/3165) 部分 HTTP JSON 响应中的 `_id` 改为 `id`，以统一命名。请确保相应更新你的应用。
* ⚠️ 重大变更 [Issue #3186](https://github.com/manticoresoftware/manticoresearch/issues/3186) 集群加入时新增 `server_id` 检查，确保每个节点有唯一 ID。现在，如果加入的节点 `server_id` 与集群中已存在节点相同，`JOIN CLUSTER` 操作将失败并显示重复 [server_id](Server_settings/Searchd.md#server_id) 的错误信息。解决此问题的方法是确保复制集群中的每个节点有唯一的 [server_id](Server_settings/Searchd.md#server_id)。你可以在配置文件的 "searchd" 部分将默认 [server_id](Server_settings/Searchd.md#server_id) 改为唯一值，然后再尝试加入集群。此变更更新了复制协议。如果你正在运行复制集群，需要：
  - 首先，干净地停止所有节点
  - 然后，用 `--new-cluster` 启动最后停止的节点，在 Linux 中使用工具 `manticore_new_cluster`。
  - 更多详情请阅读关于[重启集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster)。

### Bug 修复
* [提交 6fda](https://github.com/manticoresoftware/manticoresearch/commit/6fdad3923dd6953f4b781943eed5ec5c28b7f808) 修复等待后调度器丢失导致崩溃的问题；现在特定调度器如 `serializer` 能被正确恢复。
* [提交 c333](https://github.com/manticoresoftware/manticoresearch/commit/c333b277d4a504de5fe298e9ab570a9f9ea31e0a) 修复了不能在 `ORDER BY` 子句中使用右连接表权重的错误。
* [Issue #2644](https://github.com/manticoresoftware/manticoresearch/issues/2644) gcc 14.2.0：修复了 `lower_bound` 调用时针对 `const knn::DocDist_t*&` 的错误。❤️ 感谢 [@Azq2](https://github.com/Azq2) 的 PR。
* [Issue #3018](https://github.com/manticoresoftware/manticoresearch/issues/3018) 修复了自动模式插入时处理大写表名的问题。
* [Issue #3119](https://github.com/manticoresoftware/manticoresearch/issues/3119) 修复了解码无效 base64 输入时的崩溃。
* [Issue #3121](https://github.com/manticoresoftware/manticoresearch/issues/3121) 修复了两个与 KNN 索引相关的 `ALTER` 问题：浮点向量现在保留原始维度，且 KNN 索引能正确生成。
* [Issue #3123](https://github.com/manticoresoftware/manticoresearch/issues/3123) 修复了在空 JSON 列上构建二级索引时的崩溃。
* [Issue #3138](https://github.com/manticoresoftware/manticoresearch/issues/3138) 修复了因重复条目引发的崩溃。
* [Issue #3151](https://github.com/manticoresoftware/manticoresearch/issues/3151) 修复：`fuzzy=1` 选项不能与 `ranker` 或 `field_weights` 一起使用的问题。
* [Issue #3163](https://github.com/manticoresoftware/manticoresearch/issues/3163) 修复了 `SET GLOBAL timezone` 无效的错误。
* [Issue #3181](https://github.com/manticoresoftware/manticoresearch/issues/3181) 修复了使用大于 2^63 的 ID 时文本字段值可能丢失的问题。
* [Issue #3189](https://github.com/manticoresoftware/manticoresearch/issues/3189) 修复：`UPDATE` 语句现在正确遵守 `query_log_min_msec` 设置。
* [Issue #3247](https://github.com/manticoresoftware/manticoresearch/issues/3247) 修复了保存实时磁盘块时导致 `JOIN CLUSTER` 失败的竞态条件。


## 版本 7.4.6
发布时间：2025 年 2 月 28 日

### 主要变更
* [Issue #832](https://github.com/manticoresoftware/manticoresearch/issues/832) 集成 [Kibana](Integration/Kibana.md)，实现更简单高效的数据可视化。

### 次要变更
* [Issue #1727](https://github.com/manticoresoftware/manticoresearch/issues/1727) 修复了 arm64 和 x86_64 之间浮点数精度差异的问题。
* [Issue #2995](https://github.com/manticoresoftware/manticoresearch/issues/2995) 实现了 join 批处理的性能优化。
* [Issue #3039](https://github.com/manticoresoftware/manticoresearch/issues/3039) 实现了直方图中 EstimateValues 的性能优化。
* [Issue #3099](https://github.com/manticoresoftware/manticoresearch/issues/3099) 增加了对 Boost 1.87.0 的支持。❤️ 感谢 [@cho-m](https://github.com/cho-m) 的 PR。
* [Issue #77](https://github.com/manticoresoftware/columnar/issues/77) 优化了创建多值过滤器时的块数据重用；向属性元数据添加了最小/最大值；实现了基于最小/最大值的过滤值预过滤。

### Bug 修复
* [Commit 73ac](https://github.com/manticoresoftware/manticoresearch/commit/73ac22f358a7a0e734b332c3943c86e6294c06c3) 修复了在使用左右表的属性时，连接查询中表达式的处理；修复了右表的 index_weights 选项。
* [Issue #2915](https://github.com/manticoresoftware/manticoresearch/issues/2915) 在 `SELECT ... JOIN` 查询中使用 `avg()` 可能导致结果不正确的问题已修复。
* [Issue #2996](https://github.com/manticoresoftware/manticoresearch/issues/2996) 修复了启用 join 批处理时隐式截止导致结果集错误的问题。
* [Issue #3031](https://github.com/manticoresoftware/manticoresearch/issues/3031) 修复了守护进程关闭时在活动 chunk 合并过程中崩溃的问题。
* [Issue #3037](http://github.com/manticoresoftware/manticoresearch/issues/3037) 修复了 `IN(...)` 可能产生错误结果的问题。
* [Issue #3038](https://github.com/manticoresoftware/manticoresearch/issues/3038) 修复了 7.0.0 版本中设置 `max_iops` / `max_iosize` 导致索引性能下降的问题。
* [Issue #3042](https://github.com/manticoresoftware/manticoresearch/issues/3042) 修复了 join 查询缓存中的内存泄漏。
* [Issue #3052](https://github.com/manticoresoftware/manticoresearch/issues/3052) 修复了连接 JSON 查询中查询选项的处理。
* [Issue #3054](https://github.com/manticoresoftware/manticoresearch/issues/3054) 修复了 ATTACH TABLE 命令的问题。
* [Issue #3079](https://github.com/manticoresoftware/manticoresearch/issues/3079) 修复了错误信息不一致的问题。
* [Issue #3087](https://github.com/manticoresoftware/manticoresearch/issues/3087) 修复了按表设置 `diskchunk_flush_write_timeout=-1` 未禁用索引刷新功能的问题。
* [Issue #3088](https://github.com/manticoresoftware/manticoresearch/issues/3088) 解决了大 ID 批量替换后出现重复条目的问题。
* [Issue #3126](https://github.com/manticoresoftware/manticoresearch/issues/3126) 修复了带有单个 `NOT` 操作符和表达式排序器的全文查询导致守护进程崩溃的问题。
* [Issue #3128](https://github.com/manticoresoftware/manticoresearch/pull/3128) 修复了 CJSON 库中的潜在漏洞。❤️ 感谢 [@tabudz](https://github.com/tabudz) 的 PR。

## 版本 7.0.0
发布时间：2025 年 1 月 30 日

### 主要变更
* [Issue #1497](https://github.com/manticoresoftware/manticoresearch/issues/1497) 新增 [模糊搜索](Searching/Spell_correction.md#Fuzzy-Search) 和 [自动补全](Searching/Autocomplete.md#CALL-AUTOCOMPLETE) 功能，简化搜索操作。
* [Issue #1500](https://github.com/manticoresoftware/manticoresearch/issues/1500) [与 Kafka 集成](Integration/Kafka.md#Syncing-from-Kafka)。
* [Issue #1928](https://github.com/manticoresoftware/manticoresearch/issues/1928) 引入了用于 JSON 的 [二级索引](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#json_secondary_indexes)。
* [Issue #2361](https://github.com/manticoresoftware/manticoresearch/issues/2361) 更新和更新期间的搜索不再被块合并阻塞。
* [Issue #2787](https://github.com/manticoresoftware/manticoresearch/issues/2787) 对 RT 表实现了自动 [磁盘块刷写](Server_settings/Searchd.md#diskchunk_flush_write_timeout)，提升性能；现在我们会自动将内存块刷写到磁盘块，避免未优化内存块可能导致的性能问题，有时会因块大小不同而导致不稳定。
* [Issue #2811](https://github.com/manticoresoftware/manticoresearch/issues/2811) 新增 [滚动滚动](Searching/Pagination.md#Scroll-Search-Option) 选项，实现更方便的分页。
* [Issue #931](https://github.com/manticoresoftware/manticoresearch/issues/931) 集成 [结巴分词](https://github.com/fxsjy/jieba)，改善对[中文分词](Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md)的支持。

### 次要变更
* ⚠️ 重大变更 [Issue #1111](https://github.com/manticoresoftware/manticoresearch/issues/1111) 修复了 RT 表中对 `global_idf` 的支持。需要重新创建表。
* ⚠️ 重大变更 [Issue #2103](https://github.com/manticoresoftware/manticoresearch/issues/2103) 从内部 `cjk` 字符集移除了泰文字符。请相应地更新您的字符集定义：如果您有 `cjk,non_cjk` 并且泰文字符对您很重要，将其更改为 `cjk,thai,non_cjk`，或者 `cont,non_cjk`，其中 `cont` 是所有连续书写语言的新标识（即 `cjk` + `thai`）。使用 [ALTER TABLE](Updating_table_schema_and_settings.md#Updating-table-FT-settings-in-RT-mode) 修改现有表。
* ⚠️ 重大变更 [Issue #2468](https://github.com/manticoresoftware/manticoresearch/issues/2468) [CALL SUGGEST / QSUGGEST](Searching/Spell_correction.md#CALL-QSUGGEST,-CALL-SUGGEST) 现在兼容分布式表。此更改提升了主服务器/代理的协议版本。如果您在一个拥有多个实例的分布式环境中运行 Manticore Search，请确保先升级代理，然后升级主服务器。
* ⚠️ 重大变更 [Issue #2889](https://github.com/manticoresoftware/manticoresearch/issues/2889) 将 PQ [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META-for-PQ-tables) 中的列名从 `Name` 改为 `Variable name`。
* ⚠️ 重大变更 [Issue #879](https://github.com/manticoresoftware/manticoresearch/issues/879) 引入了新选项的[每表二进制日志](Logging/Binary_logging.md#Per-table-binary-logging-configuration)：[binlog_common](Logging/Binary_logging.md#Binary-logging-strategies)、[binlog](Logging/Binary_logging.md#Per-table-binary-logging-configuration)，用于 `create table` / `alter table`。升级到新版本前，需要对 Manticore 实例进行干净关闭。
* ⚠️ 重大变更 [Issue #1789](https://github.com/manticoresoftware/manticoresearch/issues/1789) 修正了节点以错误复制协议版本加入集群时错误的错误信息。此次更改更新了复制协议。如果您运行复制集群，需要：
  - 首先，干净地停止所有节点
  - 然后，使用 Linux 中的工具 `manticore_new_cluster`，以 `--new-cluster` 选项启动最后停止的节点。
  - 更多细节请参见[重新启动集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster)。
* ⚠️ 重大变更 [Issue #2308](https://github.com/manticoresoftware/manticoresearch/issues/2308) 在 [`ALTER CLUSTER ADD` 和 `DROP`](Creating_a_cluster/Setting_up_replication/Adding_and_removing_a_table_from_a_replication_cluster.md#Adding-and-removing-a-table-from-a-replication-cluster) 中新增对多表的支持。此更改也影响复制协议。请参照上一节指导进行更新处理。
* [Issue #2997](https://github.com/manticoresoftware/manticoresearch/issues/2997) 修复了 Macos 上的 dlopen 问题。
* [Commit 4954](https://github.com/manticoresoftware/manticoresearch/commit/4954b5de7341a29902a9b8fbb9a040f7942c77c4) 更改了包含 KNN 索引的表的 OPTIMIZE TABLE 默认截止标准以提升搜索性能。
* [Commit cfc8](https://github.com/manticoresoftware/manticoresearch/commit/cfc87ecb6e33a8163c2235243b6b40e699dbf526) 为 `FACET` 和 `GROUP BY` 中的 `ORDER BY` 增加了 `COUNT(DISTINCT)` 支持。
* [Issue #1103](https://github.com/manticoresoftware/manticoresearch/issues/1103) 改进了[日志](Logging/Server_logging.md#Server-logging)块合并的清晰度。
* [Issue #1130](https://github.com/manticoresoftware/manticoresearch/issues/1130) 增加了对 [DBeaver](Integration/DBeaver.md) 的支持。
* [Issue #1546](https://github.com/manticoresoftware/manticoresearch/issues/1546) 为 [POLY2D()](Functions/Geo_spatial_functions.md#POLY2D%28%29)/[GEOPOLY2D()](Functions/Geo_spatial_functions.md#GEOPOLY2D%28%29) 函数实现了二级索引。
* [Issue #1630](https://github.com/manticoresoftware/manticoresearch/issues/1630) HTTP 请求现支持 `Content-Encoding: gzip`。
* [Issue #1831](https://github.com/manticoresoftware/manticoresearch/issues/1831) 新增了 `SHOW LOCKS` 命令。
* [Issue #2187](https://github.com/manticoresoftware/manticoresearch/issues/2197) 允许 Buddy 请求跳过 [searchd.max_connections](Server_settings/Searchd.md#max_connections) 限制。
* [Issue #2208](https://github.com/manticoresoftware/manticoresearch/issues/2208) 支持通过 JSON HTTP 接口连接表。
* [Issue #2235](https://github.com/manticoresoftware/manticoresearch/issues/2235) 通过 Buddy 成功处理的查询以原始形式记录日志。
* [Issue #2249](https://github.com/manticoresoftware/manticoresearch/issues/2249) 为复制表添加了用于运行 `mysqldump` 的特殊模式。
* [Issue #2268](https://github.com/manticoresoftware/manticoresearch/issues/2268) 改进了对 `CREATE TABLE` 和 `ALTER TABLE` 语句复制外部文件重命名的处理。
* [Issue #2402](https://github.com/manticoresoftware/manticoresearch/issues/2402) 更新了 [searchd.max_packet_size](Server_settings/Searchd.md#max_packet_size) 默认值至 128MB。
* [Issue #2419](https://github.com/manticoresoftware/manticoresearch/issues/2419) 支持 JSON 中 [IDF boost modifier](Searching/Full_text_matching/Operators.md#IDF-boost-modifier) 的["match"](Searching/Full_text_matching/Basic_usage.md#match)。
* [Issue #2430](https://github.com/manticoresoftware/manticoresearch/issues/2430) 增强了 [binlog](Logging/Binary_logging.md#Binary-logging) 写入同步以防止错误。
* [Issue #2458](https://github.com/manticoresoftware/manticoresearch/issues/2458) Windows 包中加入了 zlib 支持。
* [Issue #2479](https://github.com/manticoresoftware/manticoresearch/issues/2479) 增加了 SHOW TABLE INDEXES 命令支持。
* [Issue #2485](https://github.com/manticoresoftware/manticoresearch/issues/2485) 为 Buddy 回复设置了会话元数据。
* [Issue #2490](https://github.com/manticoresoftware/manticoresearch/issues/2490) 兼容性端点聚合的毫秒级分辨率。
* [Issue #2500](https://github.com/manticoresoftware/manticoresearch/issues/2500) 更改了集群操作在复制启动失败时的错误消息。
* [Issue #2584](https://github.com/manticoresoftware/manticoresearch/pull/2584) 在 SHOW STATUS 中新增了[性能指标](Node_info_and_management/Node_status.md#Query-Time-Statistics)：过去1、5和15分钟内每种查询类型的最小/最大/平均/第95/99百分位。
* [Issue #2639](https://github.com/manticoresoftware/manticoresearch/pull/2639) 请求和响应中所有的 `index` 都替换为 `table`。
* [Issue #2643](https://github.com/manticoresoftware/manticoresearch/issues/2643) 在HTTP `/sql` 端点的聚合结果中添加了 `distinct` 列。
* [Issue #268](https://github.com/manticoresoftware/manticoresearch-buddy/issues/268) 实现了从 Elasticsearch 导入数据类型的自动检测。
* [Issue #2744](https://github.com/manticoresoftware/manticoresearch/issues/2744) 为字符串 JSON 字段比较表达式添加了排序规则支持。
* [Issue #2752](https://github.com/manticoresoftware/manticoresearch/issues/2752) 增加对 select 列表中 `uuid_short` 表达式的支持。
* [Issue #2783](https://github.com/manticoresoftware/manticoresearch/issues/2783) Manticore Search 现在直接运行 Buddy，无需 `manticore-buddy` 包装器。
* [Issue #2785](https://github.com/manticoresoftware/manticoresearch/issues/2785) 区分了缺少表和不支持插入操作的表的错误消息。
* [Issue #2789](https://github.com/manticoresoftware/manticoresearch/issues/2789) OpenSSL 3 现在静态编译进了 `searchd`。
* [Issue #2790](https://github.com/manticoresoftware/manticoresearch/issues/2790) 添加了 `CALL uuid_short` 语句以生成多个 `uuid_short` 值的序列。
* [Issue #2803](https://github.com/manticoresoftware/manticoresearch/issues/2803) 为 JOIN 操作的右表添加了单独的选项。
* [Issue #2810](https://github.com/manticoresoftware/manticoresearch/issues/2810) 改进了 HTTP JSON 聚合性能，以匹配 SphinxQL 中的 `GROUP BY`。
* [Issue #2854](https://github.com/manticoresoftware/manticoresearch/issues/2854) 在 Kibana 日期相关请求中支持 `fixed_interval`。
* [Issue #2909](https://github.com/manticoresoftware/manticoresearch/issues/2909) 实现了 JOIN 查询的批处理，显著提升了某些 JOIN 查询的性能，提升幅度达到数百甚至数千倍。
* [Issue #2937](https://github.com/manticoresoftware/manticoresearch/issues/2937) 启用了在 fullscan 查询中使用连接表权重。
* [Issue #2953](https://github.com/manticoresoftware/manticoresearch/issues/2953) 修复了 join 查询的日志记录。
* [Issue #337](https://github.com/manticoresoftware/manticoresearch-buddy/issues/337) 在非调试模式下隐藏了 Buddy 异常，不在 `searchd` 日志中显示。
* [Issue #2931](https://github.com/manticoresoftware/manticoresearch/issues/2931) 当用户为复制监听器设置错误端口时，守护进程带错误消息关闭。

### Bug 修复
* [Commit 0c6b](https://github.com/manticoresoftware/manticoresearch/commit/0c6bdaf41ea3684e05952e1a8273893535b923f5) 修复了包含超过32列的 JOIN 查询返回错误结果的问题。
* [Issue #2335](https://github.com/manticoresoftware/manticoresearch/issues/2335) 解决了当条件中使用两个 json 属性时表连接失败的问题。
* [Issue #2338](https://github.com/manticoresoftware/manticoresearch/issues/2338) 修正了带 [cutoff](Searching/Options.md#cutoff) 的多查询中 total_relation 不正确的问题。
* [Issue #2366](https://github.com/manticoresoftware/manticoresearch/issues/2366) 修正了[表连接](Searching/Joining.md)中右表通过 `json.string` 筛选的问题。
* [Issue #2406](https://github.com/manticoresoftware/manticoresearch/issues/2406) 允许在所有 POST HTTP JSON 端点（insert/replace/bulk）中使用 `null` 作为所有值，使用默认值替代。
* [Issue #2418](https://github.com/manticoresoftware/manticoresearch/issues/2418) 通过调整初始套接字探针的 [max_packet_size](Server_settings/Searchd.md#max_packet_size) 网络缓冲区分配优化内存消耗。
* [Issue #2420](https://github.com/manticoresoftware/manticoresearch/issues/2420) 修复了通过 JSON 接口将无符号整型插入 bigint 属性的问题。
* [Issue #2422](https://github.com/manticoresoftware/manticoresearch/issues/2422) 修复了启用 exclude filters 和 pseudo_sharding 时二级索引的正确工作。
* [Issue #2423](https://github.com/manticoresoftware/manticoresearch/issues/2423) 修复了 [manticore_new_cluster](Starting_the_server/Manually.md#searchd-command-line-options) 中的一个错误。
* [Issue #2448](https://github.com/manticoresoftware/manticoresearch/issues/2448) 解决了格式错误的 `_update` 请求导致守护进程崩溃的问题。
* [Issue #2452](https://github.com/manticoresoftware/manticoresearch/issues/2452) 修复了直方图无法处理带排除条件的值过滤。
* [Issue #55](https://github.com/manticoresoftware/columnar/issues/55) 修复了针对分布式表的 [knn](Searching/KNN.md#KNN-vector-search) 查询。
* [Issue #68](https://github.com/manticoresoftware/columnar/issues/68) 改进了 columnar accessor 中针对表编码的排除过滤器处理。
* [Commit 0eb1](https://github.com/manticoresoftware/manticoresearch/commit/0eb1579462013cb134bcaf74e06f6eeb0c2ecc4d) 修正了表达式解析器未遵守重新定义的 `thread_stack` 的问题。
* [Commit c304](https://github.com/manticoresoftware/manticoresearch/commit/c3041fd1d3e14773f3c5aa635343915a71f5cb95) 修复了克隆 columnar IN 表达式时的崩溃。
* [Commit edad](https://github.com/manticoresoftware/columnar/commit/edadc694c68d6022bdd13134263667430a42cc1d) 修复了导致崩溃的位图迭代器反转问题。
* [Commit fc30](https://github.com/manticoresoftware/executor/commit/fc302ff1117b0b835a6f5a3c07057baf5fba14d9) 修复了 `unattended-upgrades` 自动移除某些 Manticore 软件包的问题。
* [Issue #1019](https://github.com/manticoresoftware/manticoresearch/issues/1019) 改进了对 DbForge MySQL 工具发起查询的处理。
* [Issue #1107](https://github.com/manticoresoftware/manticoresearch/issues/1107) 修复了 `CREATE TABLE` 和 `ALTER TABLE` 中特殊字符的转义问题。❤️ 感谢 [@subnix](https://github.com/subnix) 的 PR。
* [Issue #116](https://github.com/manticoresoftware/manticoresearch-backup/issues/116#issuecomment-2216597206) 修复了冻结索引中更新 blob 属性时死锁的问题。死锁是由于在尝试解冻索引时锁冲突导致的，也可能引起 manticore-backup 失败。
* [Issue #1818](https://github.com/manticoresoftware/manticoresearch/issues/1818) 当表被冻结时，`OPTIMIZE` 现在会抛出错误。
* [Issue #2001](https://github.com/manticoresoftware/manticoresearch/issues/2001) 允许使用函数名作为列名。
* [Issue #2153](https://github.com/manticoresoftware/manticoresearch/issues/2153) 修复了查询表设置时使用未知磁盘块导致守护进程崩溃的问题。
* [Issue #2184](https://github.com/manticoresoftware/manticoresearch/issues/2184) 修复了 `searchd` 在执行 `FREEZE` 和 `FLUSH RAMCHUNK` 后停止时挂起的问题。
* [Issue #2228](https://github.com/manticoresoftware/manticoresearch/issues/2228) 从保留字中移除了日期/时间相关的标记（及正则表达式）。
* [Issue #2255](https://github.com/manticoresoftware/manticoresearch/issues/2255) 修复了使用超过 5 个排序字段的 `FACET` 导致的崩溃问题。
* [Issue #2265](https://github.com/manticoresoftware/manticoresearch/issues/2265) 修复了启用 `index_field_lengths` 时无法恢复 `mysqldump` 的问题。
* [Issue #2291](https://github.com/manticoresoftware/manticoresearch/issues/2291) 修复了执行 `ALTER TABLE` 命令时崩溃的问题。
* [Issue #2333](https://github.com/manticoresoftware/manticoresearch/issues/2333) 修复了 Windows 包中 MySQL DLL 以使索引器能正常工作的错误。
* [Issue #2393](https://github.com/manticoresoftware/manticoresearch/issues/2393) 修复了 GCC 编译错误。❤️ 感谢 [@animetosho](https://github.com/animetosho) 的 PR。
* [Issue #2447](https://github.com/manticoresoftware/manticoresearch/issues/2447) 修复了 [_update](Data_creation_and_modification/Updating_documents/REPLACE.md?client=Elasticsearch-like+partial) 中的转义问题。
* [Issue #2460](https://github.com/manticoresoftware/manticoresearch/issues/2460) 修复了在声明多个同名属性或字段时索引器崩溃的问题。
* [Issue #2467](https://github.com/manticoresoftware/manticoresearch/issues/2467) 解决了针对“兼容性”搜索相关端点中嵌套布尔查询转换错误导致的守护进程崩溃问题。
* [Issue #2493](https://github.com/manticoresoftware/manticoresearch/issues/2493) 修复了带修饰符的短语展开问题。
* [Issue #2535](https://github.com/manticoresoftware/manticoresearch/issues/2535) 解决了在使用 [ZONE](Searching/Full_text_matching/Operators.md#ZONE-limit-operator) 或 [ZONESPAN](Searching/Full_text_matching/Operators.md#ZONESPAN-limit-operator) 全文运算符时守护进程崩溃的问题。
* [Issue #2552](https://github.com/manticoresoftware/manticoresearch/issues/2552) 修复了带关键词字典的普通表和 RT 表的中缀生成问题。
* [Issue #2571](https://github.com/manticoresoftware/manticoresearch/issues/2571) 修复了 `FACET` 查询中的错误回复；将使用 `COUNT(*)` 的 `FACET` 默认排序设置为 `DESC`。
* [Issue #2580](https://github.com/manticoresoftware/manticoresearch/issues/2580) 修复了 Windows 启动期间守护进程崩溃的问题。
* [Issue #2603](https://github.com/manticoresoftware/manticoresearch/issues/2603) 修复了 HTTP 端点 `/sql` 和 `/sql?mode=raw` 的查询截断问题；使这些端点的请求保持一致，无需 `query=` 头。
* [Issue #2623](https://github.com/manticoresoftware/manticoresearch/issues/2623) 修复了自动架构创建表时出现的失败问题。
* [Issue #2627](https://github.com/manticoresoftware/manticoresearch/issues/2627) 修复了 HNSW 库支持加载多个 KNN 索引的问题。
* [Issue #2630](https://github.com/manticoresoftware/manticoresearch/issues/2630) 修复了同时发生多个条件时的冻结问题。
* [Issue #2645](https://github.com/manticoresoftware/manticoresearch/issues/2645) 修复了使用 `OR` 结合 KNN 搜索时的致命错误崩溃。
* [Issue #2647](https://github.com/manticoresoftware/manticoresearch/issues/2647) 修复了 `indextool --mergeidf *.idf --out global.idf` 创建输出文件后删除该文件的问题。
* [Issue #2658](https://github.com/manticoresoftware/manticoresearch/issues/2658) 修复了带外层查询中包含 `ORDER BY` 字符串的子查询导致守护进程崩溃的问题。
* [Issue #2686](https://github.com/manticoresoftware/manticoresearch/issues/2686) 修复了同时更新浮点属性和字符串属性时崩溃的问题。
* [Issue #2704](https://github.com/manticoresoftware/manticoresearch/issues/2704) 修复了 `lemmatize_xxx_all` 分词器中的多个停用词导致后续标记 `hitpos` 增加的问题。
* [Issue #2708](https://github.com/manticoresoftware/manticoresearch/issues/2708) 修复了 `ALTER ... ADD COLUMN ... TEXT` 操作崩溃的问题。
* [Issue #2737](https://github.com/manticoresoftware/manticoresearch/issues/2737) 修复了在带至少一个 RAM 块的冻结表中更新 blob 属性时，后续 `SELECT` 查询需等待表解冻的问题。
* [Issue #2742](https://github.com/manticoresoftware/manticoresearch/issues/2742) 修复了查询缓存跳过带压缩因子的查询的问题。
* [Issue #2775](https://github.com/manticoresoftware/manticoresearch/issues/2775) 当遇到未知操作时，Manticore 现在会报错，而不是在 `_bulk` 请求时崩溃。
* [Issue #2791](https://github.com/manticoresoftware/manticoresearch/issues/2791) 修复了 HTTP `_bulk` 端点插入文档 ID 返回问题。
* [Issue #2797](https://github.com/manticoresoftware/manticoresearch/issues/2797) 修复了在处理多个表时，grouper 崩溃的问题，其中一个表为空，另一个表匹配的条目数不同。
* [Issue #2835](https://github.com/manticoresoftware/manticoresearch/issues/2835) 修复了复杂 `SELECT` 查询中的崩溃问题。
* [Issue #2872](https://github.com/manticoresoftware/manticoresearch/issues/2872) 如果 `IN` 表达式中的 `ALL` 或 `ANY` 参数不是 JSON 属性，则增加错误消息提示。
* [Issue #2882](https://github.com/manticoresoftware/manticoresearch/issues/2882) 修复了在大表中更新 MVA 时守护进程崩溃的问题。
* [Issue #2888](https://github.com/manticoresoftware/manticoresearch/issues/2888) 修复了 `libstemmer` 分词失败时的崩溃问题。❤️ 感谢 [@subnix](https://github.com/subnix) 的 PR。
* [Issue #2919](https://github.com/manticoresoftware/manticoresearch/issues/2919) 修复了右表连接权重在表达式中未正确工作的情况。
* [Issue #2919](https://github.com/manticoresoftware/manticoresearch/issues/2919) 修复了右连接表权重在表达式中不生效的问题。
* [Issue #325](https://github.com/manticoresoftware/manticoresearch-buddy/issues/325) 修复了当表已存在时 `CREATE TABLE IF NOT EXISTS ... WITH DATA` 失败的问题。
* [Issue #351](https://github.com/manticoresoftware/manticoresearch-buddy/issues/351) 修复了基于文档 ID 的 KNN 计数时未定义数组键 "id" 的错误。
* [Issue #359](https://github.com/manticoresoftware/manticoresearch-buddy/issues/359) 修复了 `REPLACE INTO cluster_name:table_name` 功能。
* [Issue #67](https://github.com/manticoresoftware/docker/issues/67) 修复了运行 Manticore Docker 容器时使用 `--network=host` 出现的致命错误。

## Version 6.3.8
发布日期：2024年11月22日

版本 6.3.8 继续 6.3 系列，仅包含错误修复。

### 错误修复

* [PR #2777](https://github.com/manticoresoftware/manticoresearch/pull/2777) 修复了当查询并发受 `threads` 或 `max_threads_per_query` 设置限制时，可用线程计算错误的问题。

## manticore-extra v1.1.20

发布日期：2024年10月7日

### 错误修复

* [Issue #64](https://github.com/manticoresoftware/executor/issues/64) 解决了 `unattended-upgrades` 工具在基于 Debian 的系统上自动安装软件包更新时，错误地将多个 Manticore 软件包（包括 `manticore-galera`、`manticore-executor` 和 `manticore-columnar-lib`）标记为待移除的问题。此问题由 dpkg 错误地将虚拟软件包 `manticore-extra` 认为是多余的软件包引起。相关更改确保 `unattended-upgrades` 不再尝试移除必要的 Manticore 组件。

## Version 6.3.6
发布日期：2024年8月2日

版本 6.3.6 继续 6.3 系列，仅包含错误修复。

### 错误修复

* [Issue #2477](https://github.com/manticoresoftware/manticoresearch/issues/2477) 修复了在版本 6.3.4 中引入的崩溃，该崩溃可能在处理表达式及分布式或多个表时发生。
* [Issue #2352](https://github.com/manticoresoftware/manticoresearch/issues/2352) 修复了因 `max_query_time` 触发提前退出时，查询多个索引导致的守护进程崩溃或内部错误。

## Version 6.3.4
发布日期：2024年7月31日

版本 6.3.4 继续 6.3 系列，仅包含小幅改进和错误修复。

### 小幅改进
* [Issue #1130](https://github.com/manticoresoftware/manticoresearch/issues/1130) 增加了对 [DBeaver](Integration/DBeaver.md#Integration-with-DBeaver) 的支持。
* [Issue #2146](https://github.com/manticoresoftware/manticoresearch/issues/2146) 改进了单词形式和例外中分隔符的转义处理。
* [Issue #2315](https://github.com/manticoresoftware/manticoresearch/issues/2315) 为 SELECT 列表达式增加了字符串比较运算符。
* [Issue #2363](https://github.com/manticoresoftware/manticoresearch/issues/2363) 支持 Elastic 风格批量请求中的 null 值。
* [Issue #2374](https://github.com/manticoresoftware/manticoresearch/issues/2374) 增加了对 mysqldump 版本 9 的支持。
* [Issue #2375](https://github.com/manticoresoftware/manticoresearch/issues/2375) 改进了 HTTP JSON 查询中的错误处理，增加了错误发生节点的 JSON 路径。

### 错误修复
* [Issue #2280](https://github.com/manticoresoftware/manticoresearch/issues/2280) 修复了当 disk_chunks > 1 时，使用通配符查询且匹配项众多导致的性能下降问题。
* [Issue #2332](https://github.com/manticoresoftware/manticoresearch/issues/2332) 修复了空 MVA 数组导致的 MVA MIN 或 MAX SELECT 列表达式崩溃问题。
* [Issue #2339](https://github.com/manticoresoftware/manticoresearch/issues/2339) 修复了 Kibana 无限范围请求处理不正确的问题。
* [Issue #2342](https://github.com/manticoresoftware/manticoresearch/issues/2342) 修复了当右表的列式属性不在 SELECT 列表中时，联接过滤器失效的问题。
* [Issue #2343](https://github.com/manticoresoftware/manticoresearch/issues/2343) 修复了 Manticore 6.3.2 中重复的 'static' 声明符。
* [Issue #2344](https://github.com/manticoresoftware/manticoresearch/issues/2344) 修复了使用右表 MATCH() 时，LEFT JOIN 返回非匹配条目的问题。
* [Issue #2350](https://github.com/manticoresoftware/manticoresearch/issues/2350) 修复了 RT 索引中使用 `hitless_words` 时磁盘块保存的问题。
* [Issue #2364](https://github.com/manticoresoftware/manticoresearch/issues/2364) 现在 'aggs_node_sort' 属性可以与其他属性以任意顺序添加。
* [Issue #2368](https://github.com/manticoresoftware/manticoresearch/issues/2368) 修复了 JSON 查询中文本搜索与过滤器顺序错误的问题。
* [Issue #2376](https://github.com/manticoresoftware/manticoresearch/issues/2376) 修复了针对长 UTF-8 请求返回错误 JSON 相应的缺陷。
* [Issue #2684](https://github.com/manticoresoftware/dev/issues/2684) 修正了依赖于连接属性的预排序/预过滤表达式的计算。
* [Issue #301](https://github.com/manticoresoftware/manticoresearch-buddy/issues/301) 更改了指标数据大小的计算方法，从读取 `manticore.json` 文件改为检查数据目录的整体大小。
* [Issue #302](https://github.com/manticoresoftware/manticoresearch-buddy/issues/302) 添加了对 Vector.dev 验证请求的处理。

## Version 6.3.2
发布：2024年6月26日

版本6.3.2延续了6.3系列，包括若干错误修复，其中部分是在6.3.0发布后发现的。

### 重大变更
* ⚠️[Issue #2305](https://github.com/manticoresoftware/manticoresearch/issues/2305) 更新了 aggs.range 的值为数值类型。

### 错误修复
* [Commit c51c](https://github.com/manticoresoftware/manticoresearch/commit/c51c7fa4bd0ec4ad8d53188b888461bff29f1417) 修正了存储检查与 rset 合并时的分组问题。
* [Commit 0e85](https://github.com/manticoresoftware/manticoresearch/commit/0e85b9d5682f51681750d798aef6a11cfb93d69e) 修复了在使用 CRC 字典和开启 `local_df` 的RT索引中含通配符查询时守护进程崩溃的问题。
* [Issue #2200](https://github.com/manticoresoftware/manticoresearch/issues/2200) 修复了在无 GROUP BY 的 JOIN 中 `count(*)` 导致崩溃的错误。
* [Issue #2201](https://github.com/manticoresoftware/manticoresearch/issues/2201) 修正了JOIN尝试按全文字段分组时未返回警告的错误。
* [Issue #2230](https://github.com/manticoresoftware/manticoresearch/issues/2230) 修复了通过 `ALTER TABLE` 添加属性时未考虑 KNN 选项的问题。
* [Issue #2231](https://github.com/manticoresoftware/manticoresearch/issues/2231) 修正了6.3.0版本中无法卸载 `manticore-tools` Redhat软件包的问题。
* [Issue #2242](https://github.com/manticoresoftware/manticoresearch/issues/2242) 修正了JOIN与多个FACET语句返回错误结果的问题。
* [Issue #2250](https://github.com/manticoresoftware/manticoresearch/issues/2250) 修复了当表在集群中时执行 ALTER TABLE 报错的问题。
* [Issue #2252](https://github.com/manticoresoftware/manticoresearch/issues/2252) 修正了从 SphinxQL 接口传递给 buddy 的原始查询。
* [Issue #2267](https://github.com/manticoresoftware/manticoresearch/issues/2267) 改进了针对拥有磁盘块的RT索引中 `CALL KEYWORDS` 的通配符扩展。
* [Issue #271](https://github.com/manticoresoftware/manticoresearch-buddy/issues/271) 修复了错误的 `/cli` 请求导致的挂起。
* [Issue #274](https://github.com/manticoresoftware/manticoresearch-buddy/issues/274) 解决了向 Manticore 并发请求可能被卡住的问题。
* [Issue #275](https://github.com/manticoresoftware/manticoresearch-buddy/issues/275) 修复了通过 `/cli` 执行 `drop table if exists t; create table t` 时的挂起问题。

### 复制相关
* [Issue #2270](https://github.com/manticoresoftware/manticoresearch/issues/2270) 在 `/_bulk` HTTP端点支持了 `cluster:name` 格式。

## Version 6.3.0
发布：2024年5月23日

### 主要变更
* [Issue #839](https://github.com/manticoresoftware/manticoresearch/issues/839) 实现了[float_vector](Creating_a_table/Data_types.md#Float-vector)数据类型；实现了[向量搜索](Searching/KNN.md#KNN-vector-search)。
* [Issue #1673](https://github.com/manticoresoftware/manticoresearch/issues/1673) [INNER/LEFT JOIN](Searching/Joining.md)（**测试阶段**）。
* [Issue #1744](https://github.com/manticoresoftware/manticoresearch/issues/1744) 实现了[timestamp](Creating_a_table/Data_types.md#Timestamps)字段的日期格式自动检测。
* [Issue #1720](https://github.com/manticoresoftware/manticoresearch/pull/1720) 将 Manticore Search 许可证从 GPLv2或更新版本更改为 GPLv3或更新版本。
* [Commit 7a55](https://github.com/manticoresoftware/manticoresearch/commit/7a5508424) Windows 上运行 Manticore 现在需要使用 Docker 来运行 Buddy。
* [Issue #1541](https://github.com/manticoresoftware/manticoresearch/issues/1541) 新增了[REGEX](Searching/Full_text_matching/Operators.md#REGEX-operator)全文操作符。
* [Issue #2091](https://github.com/manticoresoftware/manticoresearch/issues/2091) 支持 Ubuntu Noble 24.04。
* [Commit 514d](https://github.com/manticoresoftware/manticoresearch/commit/514d35b497f4bdb20b6473ff963752a978a4bb8d) 重构了时间操作以提升性能并新增日期/时间函数：
  - [CURDATE()](Functions/Date_and_time_functions.md#CURDATE%28%29) - 返回本地时区的当前日期
  - [QUARTER()](Functions/Date_and_time_functions.md#QUARTER%28%29) - 返回时间戳参数所在年的季度（整数）
  - [DAYNAME()](Functions/Date_and_time_functions.md#DAYNAME%28%29) - 返回时间戳参数的星期名称
  - [MONTHNAME()](Functions/Date_and_time_functions.md#MONTHNAME%28%29) - 返回时间戳参数的月份名称
  - [DAYOFWEEK()](Functions/Date_and_time_functions.md#DAYOFWEEK%28%29) - 返回时间戳参数的星期索引（整数）
  - [DAYOFYEAR()](Functions/Date_and_time_functions.md#DAYOFYEAR%28%29) - 返回时间戳参数的年份中的日数（整数）
  - [YEARWEEK()](Functions/Date_and_time_functions.md#YEARWEEK%28%29) - 返回时间戳参数的年份及当前周第一天的日代码（整数）
  - [DATEDIFF()](Functions/Date_and_time_functions.md#DATEDIFF%28%29) - 返回两个给定时间戳之间的天数差
  - [DATE()](Functions/Date_and_time_functions.md#DATE%28%29) - 格式化时间戳中的日期部分
  - [TIME()](Functions/Date_and_time_functions.md#TIME%28%29) - 格式化时间戳中的时间部分
  - [timezone](Server_settings/Searchd.md#timezone) - 日期/时间相关函数使用的时区。
* [Commit 30e7](https://github.com/manticoresoftware/manticoresearch/commit/30e789d9657f9b3093c280c6e5b0e46a021f6dc7) 向 HTTP 接口添加了 [range](Searching/Faceted_search.md#Facet-over-set-of-ranges)、[histogram](Searching/Faceted_search.md#Facet-over-histogram-values)、[date_range](Searching/Faceted_search.md#Facet-over-set-of-date-ranges) 和 [date_histogram](Searching/Faceted_search.md#Facet-over-histogram-date-values) 聚合，并将类似表达式添加到 SQL 中。

### 细微更改
* [Issue #1285](https://github.com/manticoresoftware/manticoresearch/issues/1285) 支持 Filebeat 版本 8.10 - 8.11。
* [Issue #1771](https://github.com/manticoresoftware/manticoresearch/issues/1771) [ALTER TABLE ... type='distributed'](Updating_table_schema_and_settings.md#Changing-a-distributed-table)。
* [Issue #1788](https://github.com/manticoresoftware/manticoresearch/issues/1788) 通过 [CREATE TABLE ... LIKE ... WITH DATA](Creating_a_table/Local_tables/Real-time_table.md#CREATE-TABLE-LIKE:) SQL 语句新增了复制表的能力。
* [Issue #2072](https://github.com/manticoresoftware/manticoresearch/issues/2072) 优化了 [表压缩算法](Securing_and_compacting_a_table/Compacting_a_table.md#Compacting-a-Table)：之前，手动的 [OPTIMIZE](Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE) 和自动的 [auto_optimize](Server_settings/Searchd.md#auto_optimize) 过程都会先合并块以确保数量不超过限制，然后从所有包含已删除文档的其它块中清除已删除文档。该方法有时资源消耗过大，现已禁用。现在，块合并仅根据 [progressive_merge](Server_settings/Common.md#progressive_merge) 设置进行。然而，包含大量已删除文档的块更容易被合并。
* [Commit ce6c](https://github.com/manticoresoftware/columnar/commit/ce6c5d3381edcdc411a00a5923d13d56dd9b3010) 添加了防止加载新版次二级索引的保护。
* [Issue #1417](https://github.com/manticoresoftware/manticoresearch/issues/1417) 通过 [REPLACE INTO ... SET](Data_creation_and_modification/Updating_documents/REPLACE.md#SQL-REPLACE) 实现了部分替换。
* [Commit 7c16](https://github.com/manticoresoftware/manticoresearch/commit/7c167b934ce416d3dfbd658744a231e638b14d9a) 更新了默认合并缓存大小：`.spa`（标量属性）：256KB -> 8MB；`.spb`（二进制属性）：256KB -> 8MB；`.spc`（列式属性）：1MB，未变；`.spds`（文档存储）：256KB -> 8MB；`.spidx`（二级索引）：256KB 缓存 -> 128MB 内存限制；`.spi`（字典）：256KB -> 16MB；`.spd`（文档列表）：8MB，未变；`.spp`（命中列表）：8MB，未变；`.spe`（跳表）：256KB -> 8MB。
* [Issue #1859](https://github.com/manticoresoftware/manticoresearch/issues/1859) 通过 JSON 添加了 [复合聚合](Searching/Grouping.md#GROUP-BY-multiple-fields-at-once)。
* [Commit 216b](https://github.com/manticoresoftware/manticoresearch-buddy/commit/216b824) 由于某些正则表达式模式的问题且时间无明显收益，禁用了 PCRE.JIT。
* [Commit 55cd](https://github.com/manticoresoftware/manticoresearch/commit/55cdce32) 添加了对 vanilla Galera v.3（API v25）（来自 MySQL 5.x 的 `libgalera_smm.so`）的支持。
* [Commit 86f9](https://github.com/manticoresoftware/manticoresearch-buddy/commit/86f98ab) 指标后缀由 `_rate` 改为 `_rps`。
* [Commit c0c1](https://github.com/manticoresoftware/manticoresearch-helm/commit/c0c1cff780031412ef57cefab987dfd16613368e) 改进了关于负载均衡器高可用性支持的文档。
* [Commit d1d2](https://github.com/manticoresoftware/manticoresearch/commit/d1d2ca18857c68106e89743756260e73285be739) 将错误消息中的 `index` 改为 `table`；修复了 bison 解析器的错误消息修正。
* [Commit fd26](https://github.com/manticoresoftware/manticoresearch/commit/fd26671b) 支持 `manticore.tbl` 作为表名。
* [Issue #1105](https://github.com/manticoresoftware/manticoresearch/issues/1105) 支持通过 systemd 运行索引器（[文档](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Schedule-indexer-via-systemd)）。❤️ 感谢 [@subnix](https://github.com/subnix) 的 PR。
* [Issue #1294](https://github.com/manticoresoftware/manticoresearch/issues/1294) GEODIST() 支持二级索引。
* [Issue #1394](https://github.com/manticoresoftware/manticoresearch/issues/1394) 简化了 [SHOW THREADS](Node_info_and_management/SHOW_THREADS.md#SHOW-THREADS)。
* [Issue #1424](https://github.com/manticoresoftware/manticoresearch/issues/1424) 增加了支持 `create distributed table` 语句默认值（`agent_connect_timeout` 和 `agent_query_timeout`）。
* [Issue #1442](https://github.com/manticoresoftware/manticoresearch/issues/1442) 添加了覆盖 `searchd.expansion_limit` 的搜索查询选项 [expansion_limit](Searching/Options.md#expansion_limit)。
* [Issue #1448](https://github.com/manticoresoftware/manticoresearch/issues/1448) 实现了 int 到 bigint 转换的 [ALTER TABLE](Updating_table_schema_and_settings.md#Updating-table-schema-in-RT-mode)。
* [Issue #1456](https://github.com/manticoresoftware/manticoresearch/issues/1456) MySQL 响应中的元信息。
* [Issue #1494](https://github.com/manticoresoftware/manticoresearch/issues/1494) [SHOW VERSION](Node_info_and_management/SHOW_VERSION.md#SHOW-VERSION)。
* [Issue #1582](https://github.com/manticoresoftware/manticoresearch/issues/1582) 通过 JSON 支持根据 id 数组删除文档 ([文档](Data_creation_and_modification/Deleting_documents.md?client=JSON#Deleting-documents))。
* [Issue #1589](https://github.com/manticoresoftware/manticoresearch/issues/1589) 改进了“unsupported value type”错误。
* [Issue #1634](https://github.com/manticoresoftware/manticoresearch/issues/1634) 在 `SHOW STATUS` 中添加了 Buddy 版本。
* [Issue #1641](https://github.com/manticoresoftware/manticoresearch/issues/1641) 当关键词没有文档时，优化 Match 请求。
* [Issue #1712](https://github.com/manticoresoftware/manticoresearch/issues/1712) 在提交数据时，新增了从字符串值 "true" 和 "false" 转换为布尔属性的功能。
* [Issue #1713](https://github.com/manticoresoftware/manticoresearch/issues/1713) 新增了 [access_dict](Server_settings/Searchd.md#access_dict) 表和 searchd 选项。
* [Issue #1767](https://github.com/manticoresoftware/manticoresearch/issues/1767) 在配置的 searchd 部分添加了新选项：[expansion_merge_threshold_docs](Server_settings/Searchd.md#expansion_merge_threshold_docs) 和 [expansion_merge_threshold_hits](Server_settings/Searchd.md#expansion_merge_threshold_hits)；使合并扩展词的微小词阈值可配置。
* [Issue #1768](https://github.com/manticoresoftware/manticoresearch/issues/1768) 在 `@@system.sessions` 中增加显示上次命令时间。
* [Issue #1806](https://github.com/manticoresoftware/manticoresearch/issues/1806) 升级 Buddy 协议到版本 2。
* [Issue #1810](https://github.com/manticoresoftware/manticoresearch/issues/1810) 对 `/sql` 端点实现了额外的请求格式，方便与库集成。
* [Issue #1825](https://github.com/manticoresoftware/manticoresearch/pull/1825) 在 SHOW BUDDY PLUGINS 中新增了信息部分。
* [Issue #1837](https://github.com/manticoresoftware/manticoresearch/issues/1837) 在 `CALL PQ` 处理大包时改进了内存消耗。
* [Issue #1853](https://github.com/manticoresoftware/manticoresearch/pull/1853) 编译器从 Clang 15 切换到 Clang 16。
* [Issue #1857](https://github.com/manticoresoftware/manticoresearch/issues/1857) 新增字符串比较功能。❤️ 感谢 [@etcd](https://github.com/etcd) 的 PR。
* [Issue #1915](https://github.com/manticoresoftware/manticoresearch/issues/1915) 增加了对联接存储字段的支持。
* [Issue #1937](https://github.com/manticoresoftware/manticoresearch/issues/1937) 在查询日志中记录客户端的 host:port。
* [Issue #1981](https://github.com/manticoresoftware/manticoresearch/issues/1981) 修正了错误信息。
* [Issue #1983](https://github.com/manticoresoftware/manticoresearch/issues/1983) 为通过 JSON 查看[查询计划](Node_info_and_management/Profiling/Query_plan.md?client=JSON#Query-plan)引入了详细程度等级支持。
* [Issue #2010](https://github.com/manticoresoftware/manticoresearch/issues/2010) 除非设置了 `log_level=debug`，否则禁用 Buddy 的查询日志记录。
* [Issue #2035](https://github.com/manticoresoftware/manticoresearch/issues/2035) 增加对 Linux Mint 21.3 的支持。
* [Issue #2056](https://github.com/manticoresoftware/manticoresearch/issues/2056) 修复 Manticore 无法使用 Mysql 8.3+ 构建的问题。
* [Issue #2112](https://github.com/manticoresoftware/manticoresearch/issues/2112) 为可能在附加包含重复项的普通表后产生重复条目的实时表块新增了 `DEBUG DEDUP` 命令。
* [Issue #212](https://github.com/manticoresoftware/manticoresearch-buddy/issues/212) 在 [SHOW QUERIES](Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES) 中增加时间显示。
* [Issue #218](https://github.com/manticoresoftware/manticoresearch-buddy/issues/218) 将 `@timestamp` 列作为时间戳处理。
* [Issue #252](https://github.com/manticoresoftware/manticoresearch-buddy/issues/250) 实现了启用/禁用 Buddy 插件的逻辑。
* [Issue #254](https://github.com/manticoresoftware/manticoresearch-buddy/issues/254) 升级了 composer 到更新版本，修复近期 CVE。
* [Issue #340](https://github.com/manticoresoftware/manticoresearch/issues/340) 对与 `RuntimeDirectory` 相关的 Manticore systemd 单元进行了小幅优化。
* [Issue #51](https://github.com/manticoresoftware/executor/issues/51) 增加了 rdkafka 支持并升级到 PHP 8.3.3。
* [Issue #527](https://github.com/manticoresoftware/manticoresearch/issues/527) 支持[附加](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Attaching_one_table_to_another.md#Attaching-one-table-to-another) RT 表。新增命令 [ALTER TABLE ... RENAME](Updating_table_schema_and_settings.md#Renaming-a-real-time-table)。

### 重大变化和弃用
* ⚠️[Issue #1436](https://github.com/manticoresoftware/manticoresearch/issues/1436) 修复了 IDF 计算问题。`local_df` 现在为默认值。改进了主从代理的搜索协议（版本已更新）。如果你在分布式环境中运行多个 Manticore Search 实例，请确保先升级代理，再升级主节点。
* ⚠️[Issue #1572](https://github.com/manticoresoftware/manticoresearch/issues/1572) 新增了分布式表的复制功能并更新了复制协议。如果你运行复制集群，需要：
  - 首先，干净地停止所有节点
  - 然后，使用 Linux 下的工具 `manticore_new_cluster`，以 `--new-cluster` 选项启动最后停止的节点。
  - 更多详情请阅读 [重启集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster)。
* ⚠️[Issue #1763](https://github.com/manticoresoftware/manticoresearch/pull/1763) HTTP API 端点别名 `/json/*` 已弃用。
* ⚠️[Issue #1982](https://github.com/manticoresoftware/manticoresearch/issues/1982) JSON 格式中，将[配置文件](Node_info_and_management/Profiling/Query_profile.md#Query-profile)字段更改为[计划](Node_info_and_management/Profiling/Query_plan.md#Query-plan)，且新增了 JSON 查询分析功能。
* ⚠️[Commit e235](https://github.com/manticoresoftware/manticoresearch-backup/commit/e23585a) manticore-backup 不再备份 `plugin_dir`。
* ⚠️[Issue #171](https://github.com/manticoresoftware/manticoresearch-buddy/issues/155) 迁移 Buddy 到 Swoole，以提升性能与稳定性。升级到新版本时，请确保所有 Manticore 包已更新。
* ⚠️[Issue #196](https://github.com/manticoresoftware/manticoresearch-buddy/pull/205) 将所有核心插件合并至 Buddy，并更改了核心逻辑。
* ⚠️[Issue #2107](https://github.com/manticoresoftware/manticoresearch/issues/2107) 在 `/search` 响应中将文档 ID 视为数字的问题。
* ⚠️[Issue #38](https://github.com/manticoresoftware/executor/issues/38) 添加了 Swoole，禁用了 ZTS，并移除了 parallel 扩展。
* ⚠️[Issue #1929](https://github.com/manticoresoftware/manticoresearch/issues/1929) 在某些情况下，`charset_table` 中的覆盖功能未能正常工作。

### 复制相关更改
* [Commit 3376](https://github.com/manticoresoftware/manticoresearch/commit/3376351f73db3ff0cb6cb708a304a7bd83e69cfb) 修复了大文件 SST 中的复制错误。
* [Commit 6d36](https://github.com/manticoresoftware/manticoresearch/commit/6d36c68fb7ce3cb274b467aac435788905084952) 为复制命令添加了重试机制；修复了在网络繁忙且丢包情况下复制加入失败的问题。
* [Commit 842e](https://github.com/manticoresoftware/manticoresearch/commit/842e27e71790833ab4125d49103d2933d5df60c3) 将复制中的 FATAL 消息改为 WARNING 消息。
* [Commit 8c32](https://github.com/manticoresoftware/manticoresearch/commit/8c32bce7db39d8ed3b1a06aaa1d2c3ec372084f0) 修正了无表或空表的复制集群中 `gcache.page_size` 的计算；修复了保存和加载 Galera 选项的问题。
* [Commit a2af](https://github.com/manticoresoftware/manticoresearch/commit/a2af06ca36abcbf893e43c8803853130b397c54b) 添加了跳过集群加入节点上更新节点复制命令的功能。
* [Commit c054](https://github.com/manticoresoftware/manticoresearch/commit/c0541e6892f5116b1f957b40060ba8ecd01a5afb) 修复了更新 blob 属性与替换文档时复制过程中的死锁。
* [Commit e80d](https://github.com/manticoresoftware/manticoresearch/commit/e80d505b963999016033613c40c1cf3151d648e7) 新增了 [replication_connect_timeout](Server_settings/Searchd.md#replication_connect_timeout)、[replication_query_timeout](Server_settings/Searchd.md#replication_query_timeout)、[replication_retry_delay](Server_settings/Searchd.md#replication_retry_delay)、[replication_retry_count](Server_settings/Searchd.md#replication_retry_count) searchd 配置选项，用以控制复制过程中的网络，类似于 `searchd.agent_*`，但默认值不同。
* [Issue #1356](https://github.com/manticoresoftware/manticoresearch/issues/1356) 修正了在某些节点丢失且名称解析失败后复制节点重试的问题。
* [Issue #1445](https://github.com/manticoresoftware/manticoresearch/issues/1445) 修正了 `show variables` 中复制日志的详细级别。
* [Issue #1482](https://github.com/manticoresoftware/manticoresearch/issues/1482) 修复了 Kubernetes 中在 Pod 重启后，加入节点连接集群的复制问题。
* [Issue #1962](https://github.com/manticoresoftware/manticoresearch/issues/1962) 修正了无效节点名称的空集群上复制 alter 时的长时间等待问题。

### Bug 修复
* [Commit 8a48](https://github.com/manticoresoftware/manticoresearch/commit/8a486c7ba1a293879cdecc41fb52eb7ab0cbf832) 修复了 `count distinct` 中未使用匹配清理导致的崩溃问题。
* [Issue #1569](https://github.com/manticoresoftware/manticoresearch/issues/1569) 二进制日志现按事务粒度写入。
* [Issue #2089](https://github.com/manticoresoftware/manticoresearch/issues/2089) 修复了与 64 位 ID 相关的错误，避免通过 MySQL 插入时出现“Malformed packet”错误，防止了[表损坏和 ID 重复](https://github.com/manticoresoftware/manticoresearch/issues/2090)。
* [Issue #2160](https://github.com/manticoresoftware/manticoresearch/issues/2160) 修正了日期插入时错误地当作 UTC 处理而非本地时区。
* [Issue #2177](https://github.com/manticoresoftware/manticoresearch/issues/2177) 修复了在实时表中使用非空 `index_token_filter` 执行搜索时导致的崩溃。
* [Issue #2209](https://github.com/manticoresoftware/manticoresearch/issues/2209) 改变了 RT 列存储中的去重过滤，修复了崩溃和错误查询结果。
* [Commit 001d](https://github.com/manticoresoftware/manticoresearch/commit/001d82904e4b887c49387308291aeefb07c0fbc4) 修复了处理连接字段后 html 去除器破坏内存的问题。
* [Commit 00eb](https://github.com/manticoresoftware/manticoresearch/commit/00eb2ebf) 避免了刷新后倒带流，以防止与 mysqldump 发生通信错误。
* [Commit 0553](https://github.com/manticoresoftware/manticoresearch/commit/05534114acf08c70ffcc41a23d558dec09a34373) 如果预读尚未启动，则不等待其完成。
* [Commit 055a](https://github.com/manticoresoftware/manticoresearch/commit/055aac09f877570cd29b6594cb283182eba7f73b) 修正了大型 Buddy 输出字符串在 searchd 日志中分行显示的问题。
* [Commit 0a88](https://github.com/manticoresoftware/manticoresearch/commit/0a88056307f82c50fcdc458ad6dbc307a00e3f92) 将有关 MySQL 接口警告失败的 header `debugv` 详细级别移出。
* [Commit 150a](https://github.com/manticoresoftware/manticoresearch/commit/150a20286b7d98d24b844ae577d7fda4a444798d) 修复了多集群管理操作中的竞争条件；禁止创建同名或路径相同的多个集群。
* [Commit 2e40](https://github.com/manticoresoftware/manticoresearch/commit/2e405c1c9ebac6f46206e3df17ac1c19129c9aa3) 修正全文查询中的隐式截断；将 MatchExtended 拆分成模板部分 D。
* [Commit 75f5](https://github.com/manticoresoftware/manticoresearch/commit/75f51421542a2a0658f9e2f9f31048c8c5224479) 修正 `index_exact_words` 在索引和加载表到守护进程间的不一致。
* [Commit 7643](https://github.com/manticoresoftware/manticoresearch/commit/7643976bea142e08535d701000eff0d09a99fda2) 修复了删除无效集群时缺失的错误消息。
* [提交 7a03](https://github.com/manticoresoftware/manticoresearch/commit/7a034ac990d5ec9344320c05a6a2315583b9b858) 修复了 CBO 与队列联合的问题；修复了 CBO 与 RT 伪分片的问题。
* [提交 7b4e](https://github.com/manticoresoftware/manticoresearch/commit/7b4ea95e) 在未加载二级索引（SI）库和配置中的参数时，修正了误导性的警告信息“WARNING: secondary_indexes set but failed to initialize secondary library”。
* [提交 8496](https://github.com/manticoresoftware/manticoresearch/commit/8496317f099fade8d2db463fb88483e7fcdfe85b) 修正了仲裁中的命中排序。
* [提交 8973](https://github.com/manticoresoftware/manticoresearch-buddy/commit/8973ad3) 修复了 ModifyTable 插件中大写选项的问题。
* [提交 9935](https://github.com/manticoresoftware/manticoresearch-buddy/commit/99350d4c184379b19f3efa9751d9b5be5e6edfb5) 修复了从包含空 json 值（表示为 NULL）的转储恢复时的问题。
* [提交 a28f](https://github.com/manticoresoftware/manticoresearch/commit/a28f6e087b3d957e48816070be17d0823904936a) 通过使用 pcon，修正了 joiner 节点接收 SST 时的 SST 超时问题。
* [提交 b5a5](https://github.com/manticoresoftware/manticoresearch/commit/b5a55cc26debcddcda5d9caef8366a707e0dd763) 修复了选择带别名的字符串属性时的崩溃。
* [提交 c556](https://github.com/manticoresoftware/manticoresearch/commit/c556e7ed8b6da8eac8ef094650df5f0a8977916b) 增加了针对带有 `morphology_skip_fields` 字段的全文查询中，将词项转换为 `=term` 的查询转换。
* [提交 cdc3](https://github.com/manticoresoftware/manticoresearch/commit/cdc38085145cd1025b96fcac2a46188a0425d9e1) 添加了缺失的配置键（skiplist_cache_size）。
* [提交 cf6e](https://github.com/manticoresoftware/manticoresearch/commit/cf6ec3bfdc3f0990fb31ff53822b2331a6667727) 修复了在表达式排名器处理中遇到大型复杂查询时的崩溃。
* [提交 e513](https://github.com/manticoresoftware/manticoresearch/commit/e5139ba75eef7223df375ae3cc3f138fa7d925f1) 修正了全文 CBO 针对无效索引提示的问题。
* [提交 eb05](https://github.com/manticoresoftware/manticoresearch/commit/eb0514b3a6f57f278252b0724eebd112ec116e0f) 在关闭时中断预读，以加快关闭速度。
* [提交 f945](https://github.com/manticoresoftware/manticoresearch/commit/f94555a293cfa51004ff81135b8bd8500b955115) 修改全文查询的栈计算，避免复杂查询情况下发生崩溃。
* [问题 #1262](https://github.com/manticoresoftware/manticoresearch/issues/1262) 修复了在对具有多个同名列的 SQL 源进行索引时索引器的崩溃。
* [问题 #1273](https://github.com/manticoresoftware/manticoresearch/issues/1273) 对不存在的系统变量返回 0 而不是<empty>。
* [问题 #1289](https://github.com/manticoresoftware/manticoresearch/issues/1289) 修复了在检查 RT 表的外部文件时 indextool 的错误。
* [问题 #1335](https://github.com/manticoresoftware/manticoresearch/issues/1335) 修正了由于短语内包含多词形态导致的查询解析错误。
* [问题 #1364](https://github.com/manticoresoftware/manticoresearch/issues/1364) 增加了旧版 binlog 版本的空 binlog 文件重放功能。
* [问题 #1365](https://github.com/manticoresoftware/manticoresearch/issues/1365) 修复了最后一个空 binlog 文件的删除问题。
* [问题 #1372](https://github.com/manticoresoftware/manticoresearch/issues/1372) 修复了 `data_dir` 变更后影响守护进程启动的当前工作目录时，错误的相对路径（被转换为守护进程启动目录的绝对路径）问题。
* [问题 #1393](https://github.com/manticoresoftware/manticoresearch/issues/1393) hn_small 中最慢时间性能降低：改为在守护进程启动时抓取/缓存 CPU 信息。
* [问题 #1395](https://github.com/manticoresoftware/manticoresearch/issues/1395) 修正了索引加载时关于缺失外部文件的警告。
* [问题 #1402](https://github.com/manticoresoftware/manticoresearch/issues/1402) 修复了在释放数据指针属性时 global groupers 崩溃的问题。
* [问题 #1403](https://github.com/manticoresoftware/manticoresearch/issues/1403) _ADDITIONAL_SEARCHD_PARAMS 不生效的问题。
* [问题 #1427](https://github.com/manticoresoftware/manticoresearch/issues/1427) 修复了每表 `agent_query_timeout` 被默认查询选项 `agent_query_timeout` 替代的问题。
* [问题 #1444](https://github.com/manticoresoftware/manticoresearch/issues/1444) 修正了使用 `packedfactors()` 时多值匹配导致的 grouper 和 ranker 崩溃。
* [问题 #1458](https://github.com/manticoresoftware/manticoresearch/issues/1458) 频繁索引更新时 Manticore 崩溃。
* [问题 #1481](https://github.com/manticoresoftware/manticoresearch/issues/1481) 修复了解析错误后清理解析查询时的崩溃。
* [问题 #1484](https://github.com/manticoresoftware/manticoresearch/issues/1484) 修复 HTTP JSON 请求未路由到 buddy 的问题。
* [问题 #1499](https://github.com/manticoresoftware/manticoresearch/issues/1499) JSON 属性根值不能为数组的问题，已修复。
* [问题 #1507](https://github.com/manticoresoftware/manticoresearch/issues/1507) 修复了事务内重建表时的崩溃。
* [问题 #1515](https://github.com/manticoresoftware/manticoresearch/issues/1515) 修正了俄语词形短形式的展开问题。
* [问题 #1579](https://github.com/manticoresoftware/manticoresearch/issues/1579) 修复了 [date_format](Functions/Date_and_time_functions.md#DATE_FORMAT()) 表达式中 JSON 和 STRING 属性的使用问题。
* [问题 #1580](https://github.com/manticoresoftware/manticoresearch/issues/1580) 修正了对 JSON 字段多个别名的 grouper 处理。
* [问题 #1594](https://github.com/manticoresoftware/manticoresearch/issues/1594) dev 中错误的 total_related：修正了隐式 cutoff 与 limit 的问题；增加了对 json 查询中更好的全扫描检测。
* [问题 #1603](https://github.com/manticoresoftware/manticoresearch/issues/1603) 修复了所有日期表达式中 JSON 和 STRING 属性的使用问题。
* [Issue #1609](https://github.com/manticoresoftware/manticoresearch/issues/1609) 使用 LEVENSHTEIN() 导致崩溃。
* [Issue #1612](https://github.com/manticoresoftware/manticoresearch/issues/1612) 修复了带有高亮的搜索查询解析错误后导致的内存破坏。
* [Issue #1614](https://github.com/manticoresoftware/manticoresearch/issues/1614) 禁用了对短于 `min_prefix_len` / `min_infix_len` 词项的通配符扩展。
* [Issue #1617](https://github.com/manticoresoftware/manticoresearch/issues/1617) 修改行为：如果 Buddy 成功处理请求，则不记录错误。
* [Issue #1635](https://github.com/manticoresoftware/manticoresearch/issues/1635) 修正了设有限制时搜索查询元数据中的总数。
* [Issue #1640](https://github.com/manticoresoftware/manticoresearch/issues/1640) 通过 JSON 在纯模式下无法使用大写表名。
* [Issue #1643](https://github.com/manticoresoftware/manticoresearch/issues/1643) 提供了默认的 `SPH_EXTNODE_STACK_SIZE` 值。
* [Issue #1646](https://github.com/manticoresoftware/manticoresearch/issues/1646) 修复了 SphinxQL 中对于带有 ALL/ANY 的 MVA 属性负过滤器的日志问题。
* [Issue #1660](https://github.com/manticoresoftware/manticoresearch/issues/1660) 修正了来自其他索引的 docid 杀列表的应用。❤️ 感谢 [@raxoft](https://github.com/raxoft) 提交的 PR。
* [Issue #1668](https://github.com/manticoresoftware/manticoresearch/issues/1668) 修复了由于对原始索引全扫描过早退出导致的匹配丢失；移除了纯行迭代器的截止限制。
* [Issue #1671](https://github.com/manticoresoftware/manticoresearch/issues/1671) 修复了查询带有代理和本地表的分布式表时 `FACET` 错误。
* [Issue #1690](https://github.com/manticoresoftware/manticoresearch/issues/1690) 修复了大值直方图估计时导致的崩溃。
* [Issue #1692](https://github.com/manticoresoftware/manticoresearch/issues/1692) alter table tbl add column col uint 导致崩溃。
* [Issue #1710](https://github.com/manticoresoftware/manticoresearch/issues/1710) 条件 `WHERE json.array IN (<value>)` 返回空结果。
* [Issue #172](https://github.com/manticoresoftware/manticoresearch-buddy/issues/172) 修复了向 `/cli` 发送请求时 TableFormatter 的问题。
* [Issue #1742](https://github.com/manticoresoftware/manticoresearch/issues/1742) 在缺少 wordforms 文件的情况下，`CREATE TABLE` 不会失败。
* [Issue #1762](https://github.com/manticoresoftware/manticoresearch/issues/1762) RT 表中属性的顺序现在遵循配置顺序。
* [Issue #1765](https://github.com/manticoresoftware/manticoresearch/issues/1765) 带有 'should' 条件的 HTTP bool 查询返回不正确的结果。
* [Issue #1769](https://github.com/manticoresoftware/manticoresearch/issues/1769) 按字符串属性排序在使用 `SPH_SORT_ATTR_DESC` 和 `SPH_SORT_ATTR_ASC` 时不起作用。
* [Issue #177](https://github.com/manticoresoftware/manticoresearch-buddy/issues/177) 禁用了 curl 请求到 Buddy 的 `Expect: 100-continue` HTTP 头。
* [Issue #1791](https://github.com/manticoresoftware/manticoresearch/issues/1791) 由 GROUP BY 别名引起的崩溃。
* [Issue #1792](https://github.com/manticoresoftware/manticoresearch/issues/1792) Windows 上 SQL 元信息摘要显示错误的时间。
* [Issue #1794](https://github.com/manticoresoftware/manticoresearch/issues/1794) 修正了 JSON 查询的单词性能下降。
* [Issue #1798](https://github.com/manticoresoftware/manticoresearch/issues/1798) 不兼容的过滤器在 `/search` 上没有引发错误。
* [Issue #1802](https://github.com/manticoresoftware/manticoresearch/issues/1802) 修复了 `ALTER CLUSTER ADD` 和 `JOIN CLUSTER` 操作等待彼此的问题，防止 `ALTER` 在捐赠者将表发送到加入节点时向集群中添加表，导致竞争条件。
* [Issue #1811](https://github.com/manticoresoftware/manticoresearch/issues/1811) 错误处理 `/pq/{table}/*` 请求。
* [Issue #1816](https://github.com/manticoresoftware/manticoresearch/issues/1816) `UNFREEZE` 在某些情况下不起作用。
* [Issue #183](https://github.com/manticoresoftware/manticoresearch-buddy/pull/183) 修复了某些情况下 MVA 恢复的问题。
* [Issue #1849](https://github.com/manticoresoftware/manticoresearch/issues/1849) 修复了使用 MCL 时 indextool 关闭时崩溃的问题。
* [Issue #1866](https://github.com/manticoresoftware/manticoresearch/issues/1866) 修复了 `/cli_json` 请求的不必要的 URL 解码。
* [Issue #1872](https://github.com/manticoresoftware/manticoresearch/issues/1872) 调整了守护进程启动时 plugin_dir 的设置逻辑。
* [Issue #1874](https://github.com/manticoresoftware/manticoresearch/issues/1874) alter table ... exceptions 失败。
* [Issue #1891](https://github.com/manticoresoftware/manticoresearch/issues/1891#issuecomment-2051941200) 插入数据时 Manticore 崩溃并显示 `signal 11`。
* [Issue #1920](https://github.com/manticoresoftware/manticoresearch/issues/1920) 减少了对 [low_priority](Searching/Options.md#low_priority) 的节流。
* [Issue #1924](https://github.com/manticoresoftware/manticoresearch/issues/1924) Mysqldump + mysql 恢复的错误。
* [Issue #1951](https://github.com/manticoresoftware/manticoresearch/issues/1951) 修复当本地表缺失或代理描述错误时分布式表创建不正确的问题；现在会返回错误信息。
* [Issue #1972](https://github.com/manticoresoftware/manticoresearch/issues/1972) 实现了一个 `FREEZE` 计数器以避免冻结/解冻问题。
* [Issue #1980](https://github.com/manticoresoftware/manticoresearch/issues/1980) 在 OR 节点中遵守查询超时。之前 `max_query_time` 在某些情况下可能不起作用。
* [Issue #1986](https://github.com/manticoresoftware/manticoresearch/issues/1986) 重命名 new 为 current [manticore.json] 失败。
* [Issue #1988](https://github.com/manticoresoftware/manticoresearch/issues/1988) 全文查询可能忽略 `SecondaryIndex` CBO 提示。
* [Issue #1990](https://github.com/manticoresoftware/manticoresearch/issues/1990) 修复了 `expansion_limit` 用于从多个磁盘块或内存块中切片调用关键词的最终结果集的问题。
* [Issue #1994](https://github.com/manticoresoftware/manticoresearch/issues/1994) 错误的外部文件。
* [Issue #2021](https://github.com/manticoresoftware/manticoresearch/issues/2021) 停止 Manticore 后，部分 manticore-executor 进程可能仍在运行。
* [Issue #2029](https://github.com/manticoresoftware/manticoresearch/issues/2029) 使用 Levenshtein 距离时崩溃。
* [Issue #2037](https://github.com/manticoresoftware/manticoresearch/issues/2037) 在空索引上多次运行 max 操作符后出现错误。
* [Issue #2052](https://github.com/manticoresoftware/manticoresearch/issues/2052) 使用 JSON.field 的多分组时崩溃。
* [Issue #2067](https://github.com/manticoresoftware/manticoresearch/issues/2067) Manticore 在不正确请求 _update 时崩溃。
* [Issue #2069](https://github.com/manticoresoftware/manticoresearch/issues/2069) 修复了 JSON 接口中对封闭区间的字符串过滤比较器的问题。
* [Issue #2082](https://github.com/manticoresoftware/manticoresearch/issues/2082) 当 data_dir 路径位于符号链接上时，`alter` 操作失败。
* [Issue #2102](https://github.com/manticoresoftware/manticoresearch/pull/2102) 改进 mysqldump 中对 SELECT 查询的特殊处理，确保生成的 INSERT 语句兼容 Manticore。
* [Issue #2103](https://github.com/manticoresoftware/manticoresearch/issues/2103) 泰语字符使用了错误的字符集。
* [Issue #2124](https://github.com/manticoresoftware/manticoresearch/issues/2124) 使用包含保留字的 SQL 时崩溃。
* [Issue #2154](https://github.com/manticoresoftware/manticoresearch/issues/2154) 含有词形变化表的表无法导入。
* [Issue #2176](https://github.com/manticoresoftware/manticoresearch/issues/2176) 修复了当引擎参数设置为 'columnar' 并通过 JSON 添加重复 ID 时发生崩溃的问题。
* [Issue #223](https://github.com/manticoresoftware/manticoresearch-buddy/issues/223) 在尝试插入无模式且无列名的文档时显示正确的错误。
* [Issue #239](https://github.com/manticoresoftware/manticoresearch-buddy/issues/239) 自动模式的多行插入可能失败。
* [Issue #399](https://github.com/manticoresoftware/manticoresearch/issues/399) 如果数据源声明了 id 属性，索引时添加错误消息。
* [Issue #59](https://github.com/manticoresoftware/manticoresearch-helm/issues/59) Manticore 集群崩溃。
* [Issue #68](https://github.com/manticoresoftware/manticoresearch-helm/issues/68) 如果存在 percolate 表，optimize.php 会崩溃。
* [Issue #77](https://github.com/manticoresoftware/manticoresearch-helm/issues/77) 修复 Kubernetes 部署时的错误。
* [Issue #274](https://github.com/manticoresoftware/manticoresearch-buddy/issues/274) 修复 Buddy 并发请求处理不正确的问题。

### 与 manticore-backup 相关
* [Issue #97](https://github.com/manticoresoftware/manticoresearch-backup/pull/97) 当可用时设置 VIP HTTP 端口为默认端口。
多个改进：改进版本检查和流式 ZSTD 解压；恢复过程中为版本不匹配添加用户提示；修复恢复时不同版本的提示行为；增强解压逻辑，直接从流中读取而非加载到工作内存；添加 `--force` 标志
* [Commit 3b35](https://github.com/manticoresoftware/manticoresearch-backup/commit/3b357ea) 在 Manticore 搜索启动后显示备份版本，以识别此阶段的问题。
* [Commit ad2e](https://github.com/manticoresoftware/manticoresearch-backup/commit/ad2e5da) 更新连接守护进程失败时的错误消息。
* [Commit ce5e](https://github.com/manticoresoftware/manticoresearch-backup/commit/ce5ea85) 修复将绝对根备份路径转换为相对路径的问题，并移除恢复时的可写性检查，以支持从不同路径恢复。
* [Commit db7e](https://github.com/manticoresoftware/manticoresearch-backup/commit/db7e2b9) 为文件迭代器添加排序，确保各种情况下的一致性。
* [Issue #106](https://github.com/manticoresoftware/manticoresearch-backup/issues/105) 多配置的备份和恢复。
* [Issue #91](https://github.com/manticoresoftware/manticoresearch-backup/pull/91) 添加 defattr 以防止 RHEL 安装后文件出现异常用户权限。
* [Issue #91](https://github.com/manticoresoftware/manticoresearch-backup/pull/91) 添加额外 chown，确保 Ubuntu 中的文件默认归 root 用户。

### 与 MCL（列式存储、二级索引、近邻库）相关
* [Commit f104](https://github.com/manticoresoftware/columnar/commit/f104a4709bbb144140d09a09bc0fc1db1895e331) 支持向量搜索。
* [Commit 2169](https://github.com/manticoresoftware/columnar/commit/216953beef6676f7e34c9f6f1a30c5980d746710) 修复二级索引构建中断时临时文件的清理问题。解决守护进程在创建 `tmp.spidx` 文件时超过打开文件限制的问题。
* [Commit 709b](https://github.com/manticoresoftware/columnar/commit/709b9acaaac97d9a1ca8796892f9ad432021c785) 对列式和二级索引使用独立的 streamvbyte 库。
* [Commit 1c26](https://github.com/manticoresoftware/manticoresearch/commit/1c2623763ced14bb94ad624dde9614a938408d89) 添加警告，列式存储不支持 json 属性。
* [Commit 3acd](https://github.com/manticoresoftware/columnar/commit/3acd6714ffce103447d7931dfc75bb1a7449f340) 修复二级索引中的数据解包问题。
* [Commit 574c](https://github.com/manticoresoftware/manticoresearch/commit/574c023152564fa72092c18ebd1f594abfd4b6d7) 修复以混合行式和列式存储保存磁盘块时崩溃的问题。
* [Commit e87f](https://github.com/manticoresoftware/columnar/commit/e87f088b458d4c5b1b6159fc72ed0d57b786891a) 修复二级索引迭代器提示已处理块的问题。
* [Issue #1474](https://github.com/manticoresoftware/manticoresearch/issues/1474) 使用列存引擎时，行式 MVA 列的更新功能损坏。
* [Issue #1510](https://github.com/manticoresoftware/manticoresearch/issues/1510) 修复了在对 `HAVING` 中使用的列存属性进行聚合时的崩溃问题。
* [Issue #1519](https://github.com/manticoresoftware/manticoresearch/issues/1519) 修复了在使用列存属性时 `expr` 排序器的崩溃问题。

### 与 Docker 相关
* ❗[Issue #42](https://github.com/manticoresoftware/docker/issues/42) 通过环境变量支持[纯索引](https://github.com/manticoresoftware/docker#building-plain-tables)。
* ❗[Issue #47](https://github.com/manticoresoftware/docker/issues/47) 通过环境变量改进配置的灵活性。
* [Issue #54](https://github.com/manticoresoftware/docker/pull/54) 改进了 Docker 的[备份和恢复](https://github.com/manticoresoftware/docker#backup-and-restore)流程。
* [Issue #77](https://github.com/manticoresoftware/docker/issues/76) 改进入口点，仅在首次启动时处理备份恢复。
* [Commit a27c](https://github.com/manticoresoftware/docker/commit/a27c048dd9e73d8d6bea8d9a4830cafb486b82c5) 修正了查询日志输出到 stdout 的问题。
* [Issue #38](https://github.com/manticoresoftware/docker/issues/38) 如果未设置 EXTRA，则静默 BUDDY 警告。
* [Issue #71](https://github.com/manticoresoftware/docker/pull/71) 修正了 `manticore.conf.sh` 中的主机名。

## 版本 6.2.12
发布时间：2023年8月23日

版本 6.2.12 继续 6.2 系列，解决了 6.2.0 发布后发现的问题。

### Bug 修复
* ❗[Issue #1351](https://github.com/manticoresoftware/manticoresearch/issues/1351) “Manticore 6.2.0 无法通过 systemctl 在 Centos 7 上启动”：将 `TimeoutStartSec` 从 `infinity` 修改为 `0`，以提高在 Centos 7 上的兼容性。
* ❗[Issue #1364](https://github.com/manticoresoftware/manticoresearch/issues/1364) “从 6.0.4 升级到 6.2.0 后崩溃”：为旧版本 binlog 的空日志文件添加重放功能。
* [PR #1334](https://github.com/manticoresoftware/manticoresearch/pull/1334) “修正 searchdreplication.cpp 中的拼写错误”：纠正 `searchdreplication.cpp` 中的拼写错误：beggining -> beginning。
* [Issue #1337](https://github.com/manticoresoftware/manticoresearch/issues/1337) “Manticore 6.2.0 WARNING: conn (local)(12), sock=8088: bailing on failed MySQL header, AsyncNetInputBuffer_c::AppendData: error 11 (Resource temporarily unavailable) return -1”：将关于 MySQL 接口头部警告的详细级别降低到 logdebugv。
* [Issue #1355](https://github.com/manticoresoftware/manticoresearch/issues/1355) “当 node_address 无法解析时 join cluster 挂起”：改进了当部分节点不可达且名称解析失败时的复制重试，解决了 Kubernetes 和 Docker 节点中与复制相关的问题。增强了复制启动失败的错误信息，并更新了测试模型 376。此外，提供了名称解析失败的清晰错误消息。
* [Issue #1361](https://github.com/manticoresoftware/manticoresearch/issues/1361) “非 CJK 字符集对 'Ø' 没有小写映射”：调整了 'Ø' 字符的映射。
* [Issue #1365](https://github.com/manticoresoftware/manticoresearch/issues/1365) “searchd 正常停止后仍留下 binlog.meta 和 binlog.001”：确保最后一个空 binlog 文件被正确移除。
* [Commit 0871](https://github.com/manticoresoftware/manticoresearch/commit/0871070845e93ad702ae0dfb89b35e5f24cd0851)：修复了 Windows 上 `Thd_t` 构建问题，解决了原子复制限制相关的问题。
* [Commit 1cc0](https://github.com/manticoresoftware/manticoresearch/commit/1cc04bedc174737118fd5a3663e35f4e95bd4f8c)：解决了 FT CBO 与 `ColumnarScan` 的问题。
* [Commit c6bf](https://github.com/manticoresoftware/manticoresearch/commit/c6bfb30d53e3f4b67f17769667733677a661c95b)：修正了测试 376，并为测试中的 `AF_INET` 错误添加了替代方案。
* [Commit cbc3](https://github.com/manticoresoftware/manticoresearch/commit/cbc38705e0b4dcc9665e3e8cf64826290ebd11cf)：解决了复制期间更新 blob 属性与替换文档时的死锁问题。还移除了提交时索引的读锁，因为其已被更底层级别锁定。

### 小改动
* [Commit 4f91](https://github.com/manticoresoftware/manticoresearch/commit/4f913a36e84e01f950b2797051222b0b1094d6fe) 更新了手册中关于 `/bulk` 端点的信息。

### MCL
* 支持 [Manticore Columnar Library](https://github.com/manticoresoftware/columnar) v2.2.4

## 版本 6.2.0
发布时间：2023年8月4日

### 主要变更
* 查询优化器增强，支持全文查询，显著提高搜索效率和性能。
* 集成了：
  - [mysqldump](https://dev.mysql.com/doc/refman/8.0/en/mysqldump.html) —— 使用 `mysqldump` 进行[逻辑备份](Securing_and_compacting_a_table/Backup_and_restore.md#Backup-and-restore-with-mysqldump)
  - [Apache Superset](https://manticoresearch.com/blog/manticoresearch-apache-superset-integration/) 和 [Grafana](https://manticoresearch.com/blog/manticoresearch-grafana-integration/)，用于可视化存储在 Manticore 中的数据
  - [HeidiSQL](https://www.heidisql.com/) 和 [DBForge](https://www.devart.com/dbforge/)，便于使用 Manticore 进行开发
* 我们开始使用 [GitHub workflows](https://github.com/manticoresoftware/manticoresearch/actions)，使贡献者能够使用与核心团队准备软件包时相同的持续集成（CI）流程。所有作业都可在 GitHub 托管的运行器上运行，方便无缝测试你在 Manticore Search 分支上的变更。
* 我们开始使用 [CLT](https://github.com/manticoresoftware/clt) 来测试复杂场景。例如，我们现在能够确保提交后的包能在所有支持的 Linux 操作系统上正确安装。命令行测试器（CLT）提供了一种用户友好的方式，以交互模式记录测试并轻松重放。
* 通过采用哈希表和 HyperLogLog 结合的方式，显著提升了 count distinct 操作的性能。
* 启用了包含二级索引查询的多线程执行，线程数限制为物理 CPU 核心数。这将大幅提升查询执行速度。
* `pseudo_sharding` 已调整为受限于空闲线程数。此更新显著提升了吞吐量性能。
* 用户现在可以通过配置设置指定[默认属性存储引擎](Server_settings/Searchd.md#engine)，以更好地满足特定工作负载需求。
* 支持 [Manticore Columnar Library 2.2.0](https://github.com/manticoresoftware/columnar/)，在[二级索引](Server_settings/Searchd.md#secondary_indexes)中带来多项错误修复和改进。

### 小改动
* [补丁 #153](https://github.com/manticoresoftware/manticoresearch-buddy/issues/153)：[/pq](Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md?client=JSON#Adding-rules-to-a-percolate-table) HTTP 端点现在作为 `/json/pq` HTTP 端点的别名。
* [提交 0bf1](https://github.com/manticoresoftware/manticoresearch/commit/0bf17d9e)：确保 `upper()` 和 `lower()` 的多字节兼容性。
* [提交 2bb9](https://github.com/manticoresoftware/manticoresearch/commit/2bb92765)：`count(*)` 查询不再扫描索引，而是返回预计算值。
* [提交 3c84](https://github.com/manticoresoftware/manticoresearch/commit/3c84091f)：现在可以使用 `SELECT` 进行任意计算并显示 `@@sysvars`。不再局限于仅单个计算。因此，查询如 `select user(), database(), @@version_comment, version(), 1+1 as a limit 10` 将返回所有列。注意，可选的 'limit' 会被忽略。
* [提交 6aca](https://github.com/manticoresoftware/manticoresearch/commit/6aca32f0)：实现了 `CREATE DATABASE` 的存根查询。
* [提交 9dc1](https://github.com/manticoresoftware/manticoresearch/commit/9dc12334)：执行 `ALTER TABLE table REBUILD SECONDARY` 时，即使属性未更新，也会始终重建二级索引。
* [提交 46ed](https://github.com/manticoresoftware/manticoresearch/commit/46edb089)：使用预计算数据的排序器在应用 CBO 之前现在会被识别，以避免不必要的 CBO 计算。
* [提交 102a](https://github.com/manticoresoftware/manticoresearch/commit/102ac604)：实现并使用全文表达式堆栈的模拟，防止守护进程崩溃。
* [提交 979f](https://github.com/manticoresoftware/manticoresearch/commit/979fa27c)：为不使用字符串/mvas/json 属性的匹配，添加了快速匹配克隆代码路径。
* [提交 a073](https://github.com/manticoresoftware/manticoresearch/commit/a0735ffe)：添加了对 `SELECT DATABASE()` 命令的支持，但该命令始终返回 `Manticore`。此新增对与各种 MySQL 工具的集成至关重要。
* [提交 bc04](https://github.com/manticoresoftware/manticoresearch/commit/bc04908)：修改了 [/cli](Connecting_to_the_server/HTTP.md#/cli) 端点的响应格式，并新增 `/cli_json` 端点以保持原有 `/cli` 的功能。
* [提交 d70b](https://github.com/manticoresoftware/manticoresearch/commit/d70b0d58)：现在可通过 `SET` 语句在运行时更改 `thread_stack`。支持会话本地和守护全局两种变体。当前值可通过 `show variables` 查看。
* [提交 d96e](https://github.com/manticoresoftware/manticoresearch/commit/d96ec6b9)：将代码集成到 CBO 中，更准确估算对字符串属性过滤的执行复杂性。
* [提交 e77d](https://github.com/manticoresoftware/manticoresearch/commit/e77dd72f5a04531c352fad0d7afcd2a1cbae2510)：改进了 DocidIndex 代价计算，提升整体性能。
* [提交 f3ae](https://github.com/manticoresoftware/manticoresearch/commit/f3ae8bea)：负载指标（类似 Linux 下的 'uptime'）现在显示在 `SHOW STATUS` 命令中。
* [提交 f3cc](https://github.com/manticoresoftware/manticoresearch/commit/f3cc0971)：`DESC` 和 `SHOW CREATE TABLE` 中的字段及属性顺序现与 `SELECT * FROM` 保持一致。
* [提交 f3d2](https://github.com/manticoresoftware/manticoresearch/commit/f3d248a6)：不同内部解析器在各种错误时提供内部助记码（例如 `P01`），以帮助识别出错解析器并屏蔽非必要内部细节。
* [问题 #271](https://github.com/manticoresoftware/manticoresearch/issues/271) “有时 CALL SUGGEST 不会建议单字母拼写错误的更正”：改进了 [SUGGEST/QSUGGEST](Searching/Spell_correction.md#CALL-QSUGGEST,-CALL-SUGGEST) 对短词的表现：新增 `sentence` 选项以展示整句话。
* [问题 #696](https://github.com/manticoresoftware/manticoresearch/issues/696) “当启用词干时，Percolate 索引无法正确按精确短语查询搜索”：修改了 percolate 查询以处理精确定语修饰符，提升搜索功能。
* [问题 #829](https://github.com/manticoresoftware/manticoresearch/issues/829) “日期格式化方法”：新增 [date_format()](Functions/Date_and_time_functions.md#DATE_FORMAT()) 选择列表表达式，暴露 `strftime()` 函数。
* [Issue #961](https://github.com/manticoresoftware/manticoresearch/issues/961) “通过 HTTP JSON API 排序桶”：在 HTTP 接口中为每个聚合桶引入了可选的[排序属性](Searching/Faceted_search.md#HTTP-JSON)。
* [Issue #1062](https://github.com/manticoresoftware/manticoresearch/issues/1062) “改进 JSON 插入 API 失败时的错误日志 - ‘不支持的值类型’”：在出现错误时，`/bulk` 端点会报告已处理和未处理字符串（文档）的数量信息。
* [Issue #1070](https://github.com/manticoresoftware/manticoresearch/issues/1070) “CBO 提示不支持多个属性”：启用了索引提示以处理多个属性。
* [Issue #1106](https://github.com/manticoresoftware/manticoresearch/issues/1106) “为 http 搜索查询添加标签”：标签已被添加到[HTTP PQ 响应](Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md?client=JSON#Adding-rules-to-a-percolate-table)中。
* [Issue #1301](https://github.com/manticoresoftware/manticoresearch/issues/1301) “buddy 不应并行创建表”：解决了因并行执行 CREATE TABLE 操作导致失败的问题。现在，任意时刻只允许运行一个 `CREATE TABLE` 操作。
* [Issue #1303](https://github.com/manticoresoftware/manticoresearch/issues/1303) “增加对列名中 @ 的支持”。
* [Issue #1316](https://github.com/manticoresoftware/manticoresearch/issues/1316) “在 taxi 数据集上 ps=1 时查询缓慢”：优化了 CBO 逻辑，默认直方图分辨率设置为 8k，以提高随机分布值属性的准确性。
* [Issue #1317](https://github.com/manticoresoftware/manticoresearch/issues/1317) “修复 hn 数据集上的 CBO 与全文索引问题”：改进了确定何时使用位图迭代器交集及何时使用优先队列的逻辑。
* [Issue #1318](https://github.com/manticoresoftware/manticoresearch/issues/1318) “columnar: 更改迭代器接口为单次调用”：Columnar 迭代器现在使用单次的 `Get` 调用，替代此前的两步 `AdvanceTo` + `Get` 调用来获取值。
* [Issue #1319](https://github.com/manticoresoftware/manticoresearch/issues/1319) “加快聚合计算速度（移除 CheckReplaceEntry？）”：从分组排序器中移除了 `CheckReplaceEntry` 调用，加快了聚合函数的计算。
* [Issue #1320](https://github.com/manticoresoftware/manticoresearch/issues/1320) “创建表 read_buffer_docs/hits 不支持 k/m/g 语法”：`CREATE TABLE` 选项 `read_buffer_docs` 和 `read_buffer_hits` 现在支持 k/m/g 语法。
* 英语、德语和俄语[语言包](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology)现在可通过执行命令 `apt/yum install manticore-language-packs` 在 Linux 上轻松安装。在 macOS 上使用命令 `brew install manticoresoftware/tap/manticore-language-packs`。
* 字段和属性的顺序现在在 `SHOW CREATE TABLE` 和 `DESC` 操作间保持一致。
* 如果执行 `INSERT` 查询时磁盘空间不足，新 `INSERT` 查询将失败，直到有足够的磁盘空间。
* 添加了类型转换函数 [UINT64()](Functions/Type_casting_functions.md#UINT64%28%29)。
* `/bulk` 端点现在将空行视为[提交](Data_creation_and_modification/Transactions.md#BEGIN,-COMMIT,-and-ROLLBACK)命令。更多信息见[这里](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md?client=JSON#Bulk-adding-documents)。
* 对[无效的索引提示](Searching/Options.md#Query-optimizer-hints)实现了警告，提供更多透明度并允许错误缓解。
* 当使用 `count(*)` 搭配单一过滤器时，查询现在会利用来自二级索引的预计算数据（如果可用），大幅加快查询速度。

### ⚠️ 重大变更
* ⚠️ 版本 6.2.0 中创建或修改的表不能被旧版本读取。
* ⚠️ 文档 ID 现在在索引和 INSERT 操作中作为无符号 64 位整数处理。
* ⚠️ 查询优化器提示的语法已更新。新格式为 `/*+ SecondaryIndex(uid) */`。请注意旧语法已不再支持。
* ⚠️ [Issue #1160](https://github.com/manticoresoftware/manticoresearch/issues/1160)：为防止语法冲突，不允许在表名中使用 `@`。
* ⚠️ 标记为 `indexed` 和 `attribute` 的字符串字段/属性在 `INSERT`、`DESC` 和 `ALTER` 操作中现在视为单一字段。
* ⚠️ [Issue #1057](https://github.com/manticoresoftware/manticoresearch/issues/1057)：MCL 库将在不支持 SSE 4.2 的系统上不再加载。
* ⚠️ [Issue #1143](https://github.com/manticoresoftware/manticoresearch/issues/1143)：[agent_query_timeout](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_query_timeout) 已修复，以前存在的错误现已生效。

### Bug 修复
* [Commit 2a6e](https://github.com/manticoresoftware/manticoresearch/commit/2a6ea8f7) “DROP TABLE 崩溃”：解决了执行 DROP TABLE 语句时导致 RT 表上写操作（优化、磁盘块保存）耗时过长的问题。新增警告，当执行 DROP TABLE 命令后表目录不为空时予以提示。
* [Commit 2ebd](https://github.com/manticoresoftware/manticoresearch/commit/2ebd424d)：为支持多属性分组中缺失的 columnar 属性补充了支持。
* [Commit 3be4](https://github.com/manticoresoftware/manticoresearch/commit/3be4503f752216b37b8cde8e47fd2d77bbce63ae) 修复了因磁盘空间耗尽可能导致的崩溃问题，通过正确处理 binlog 中的写入错误解决。
* [提交 6adb](https://github.com/manticoresoftware/manticoresearch/commit/6adb0934)：修复了在查询中使用多个列存扫描迭代器（或二级索引迭代器）时偶尔发生的崩溃问题。
* [提交 6bd9](https://github.com/manticoresoftware/manticoresearch/commit/6bd9f709)：使用预计算数据的排序器时，过滤器未被移除的问题已修复。
* [提交 6d03](https://github.com/manticoresoftware/manticoresearch/commit/6d03566a)：更新了CBO代码，以便为多线程执行的基于行属性过滤器的查询提供更好的估算。
* [提交 6dd3](https://github.com/manticoresoftware/manticoresearch/commit/6dd3964), [Helm #56](https://github.com/manticoresoftware/manticoresearch-helm/issues/56) “Kubernetes集群中的致命崩溃转储”：修复了JSON根对象的缺陷Bloom过滤器；修复了因按JSON字段过滤导致的守护进程崩溃。
* [提交 6e1b](https://github.com/manticoresoftware/manticoresearch/commit/6e1b849) 修正了由无效 `manticore.json` 配置引起的守护进程崩溃。
* [提交 6fbc](https://github.com/manticoresoftware/manticoresearch/commit/6fbc4189) 修复了json范围过滤器以支持int64值。
* [提交 9c67](https://github.com/manticoresoftware/manticoresearch/commit/9c677426) `.sph` 文件可能因 `ALTER` 命令而损坏的问题已修复。
* [提交 77cc](https://github.com/manticoresoftware/manticoresearch/commit/77ccd8f)：为替换语句的复制添加了共享密钥，以解决当替换语句从多个主节点复制时出现的 `pre_commit` 错误。
* [提交 2884](https://github.com/manticoresoftware/manticoresearch/commit/2884519e) 解决了对诸如 'date_format()' 之类函数的bigint检查问题。
* [提交 9513](https://github.com/manticoresoftware/manticoresearch/commit/9513aca5)：当排序器使用预计算数据时，迭代器将不再显示在 [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META) 中。
* [提交 a2a7](https://github.com/manticoresoftware/manticoresearch/commit/a2a70555)：更新全文节点堆栈大小，以防止复杂全文查询时崩溃。
* [提交 a062](https://github.com/manticoresoftware/manticoresearch/commit/a0626d7e)：解决了带有JSON和字符串属性的更新复制时导致崩溃的错误。
* [提交 b3e6](https://github.com/manticoresoftware/manticoresearch/commit/b3e63e5d)：字符串构建器已更新为使用64位整数，以避免处理大数据集时崩溃。
* [提交 c472](https://github.com/manticoresoftware/manticoresearch/commit/c472e5b)：修复了跨多个索引执行count distinct时发生的崩溃。
* [提交 d073](https://github.com/manticoresoftware/manticoresearch/commit/d0730272)：修复了即使禁用 `pseudo_sharding`，查询实时索引磁盘块时仍可多线程执行的问题。
* [提交 d205](https://github.com/manticoresoftware/manticoresearch/commit/d205508e) `show index status` 命令返回的值集已修改，现在根据所使用的索引类型而变化。
* [提交 e9bc](https://github.com/manticoresoftware/manticoresearch/commit/e9bc2f9) 修复了处理批量请求时的HTTP错误及错误未从网络循环返回给客户端的问题。
* [提交 f77c](https://github.com/manticoresoftware/manticoresearch/commit/f77ce0e6) 使用了PQ的扩展堆栈。
* [提交 fac2](https://github.com/manticoresoftware/manticoresearch/commit/fac2d9687a20582765aaa89be9597804c4f845af) 更新了导出排名器输出，以符合 [packedfactors()](Functions/Searching_and_ranking_functions.md#PACKEDFACTORS%28%29)。
* [提交 ff87](https://github.com/manticoresoftware/manticoresearch/commit/ff87cc4)：修复了SphinxQL查询日志中过滤器的字符串列表问题。
* [问题 #589](https://github.com/manticoresoftware/manticoresearch/issues/589) “字符集定义似乎依赖于代码顺序”：修正了重复项的字符集映射错误。
* [问题 #811](https://github.com/manticoresoftware/manticoresearch/issues/811) “词形变化中映射多个词干干扰带有CJK标点符号的短语搜索”：修复了带有词形变化的短语查询中的ngram标记位置问题。
* [问题 #834](https://github.com/manticoresoftware/manticoresearch/issues/834) “搜索查询中的等号导致请求失败”：确保精确符号可以转义，并修复了 `expand_keywords` 选项导致的双重精确扩展。
* [问题 #864](https://github.com/manticoresoftware/manticoresearch/issues/864) “异常/停用词冲突”
* [问题 #910](https://github.com/manticoresoftware/manticoresearch/issues/910) “调用call snippets()时，使用libstemmer_fr和index_exact_words导致Manticore崩溃”：解决了调用 `SNIPPETS()` 时导致崩溃的内部冲突。
* [问题 #946](https://github.com/manticoresoftware/manticoresearch/issues/946) “SELECT时重复记录”：修复了针对带有已删除文档的RT索引且设置了 `not_terms_only_allowed` 选项的查询结果中重复文档的问题。
* [问题 #967](https://github.com/manticoresoftware/manticoresearch/issues/967) “UDF函数中使用JSON参数导致崩溃”：修复了启用伪分片和带有JSON参数的UDF处理搜索时的守护进程崩溃。
* [问题 #1050](https://github.com/manticoresoftware/manticoresearch/issues/1050) “FEDERATED中的count(*)”：修复了通过 `FEDERATED` 引擎使用聚合查询时导致的守护进程崩溃。
* [问题 #1052](https://github.com/manticoresoftware/manticoresearch/issues/1052) 修复了 `rt_attr_json` 列与列存储不兼容的问题。
* [问题 #1072](https://github.com/manticoresoftware/manticoresearch/issues/1072) “ignore_chars会移除搜索查询中的 * ”：修复了此问题，使查询中的通配符不会被 `ignore_chars` 影响。
* [Issue #1075](https://github.com/manticoresoftware/manticoresearch/issues/1075) "indextool --check 如果有分布式表会失败": indextool 现在兼容 JSON 配置中包含 ‘distributed’ 和 ‘template’ 索引的实例。
* [Issue #1081](https://github.com/manticoresoftware/manticoresearch/issues/1081) "对特定 RT 数据集的特定查询导致 searchd 崩溃": 解决了使用 packedfactors 和大内部缓冲区的查询导致守护进程崩溃的问题。
* [Issue #1095](https://github.com/manticoresoftware/manticoresearch/issues/1095) "启用 not_terms_only_allowed 时已删除文档被忽略"
* [Issue #1099](https://github.com/manticoresoftware/manticoresearch/issues/1099) "indextool --dumpdocids 不工作": 恢复了 `--dumpdocids` 命令的功能。
* [Issue #1100](https://github.com/manticoresoftware/manticoresearch/issues/1100) "indextool --buildidf 不工作": indextool 现在在完成 globalidf 操作后关闭文件。
* [Issue #1104](https://github.com/manticoresoftware/manticoresearch/issues/1104) "Count(*) 在远程表中被错误地当作 schema 设置": 解决了当代理返回空结果集时，守护进程对分布式索引查询发出错误消息的问题。
* [Issue #1109](https://github.com/manticoresoftware/manticoresearch/issues/1109) "FLUSH ATTRIBUTES 在 threads=1 时挂起".
* [Issue #1126](https://github.com/manticoresoftware/manticoresearch/issues/1126) "查询中与 MySQL 服务器连接丢失 - manticore 6.0.5": 解决了在使用多个列式属性过滤器时引起的崩溃问题。
* [Issue #1135](https://github.com/manticoresoftware/manticoresearch/issues/1135) "JSON 字符串过滤大小写敏感问题": 修正了 HTTP 搜索请求中过滤器的排序规则以正常工作。
* [Issue #1140](https://github.com/manticoresoftware/manticoresearch/issues/1140) "错误字段中的匹配": 修复了与 `morphology_skip_fields` 相关的问题。
* [Issue #1155](https://github.com/manticoresoftware/manticoresearch/issues/1155) "通过 API 的系统远程命令应传递 g_iMaxPacketSize": 更新了绕过节点间复制命令的 `max_packet_size` 检查。同时，将最新群集错误添加到了状态显示中。
* [Issue #1302](https://github.com/manticoresoftware/manticoresearch/issues/1302) "失败的 optimize 后遗留临时文件": 修正了合并或优化过程中出错后遗留临时文件的问题。
* [Issue #1304](https://github.com/manticoresoftware/manticoresearch/issues/1304) "为 buddy 启动超时添加环境变量": 添加环境变量 `MANTICORE_BUDDY_TIMEOUT`（默认 3 秒），用于控制守护进程启动时等待 buddy 消息的时间。
* [Issue #1305](https://github.com/manticoresoftware/manticoresearch/issues/1305) "保存 PQ 元数据时整数溢出": 减轻了守护进程在保存大型 PQ 索引时的过度内存消耗。
* [Issue #1306](https://github.com/manticoresoftware/manticoresearch/issues/1306) "修改外部文件后无法重建 RT 表": 纠正了使用空字符串修改外部文件时的错误；修复了修改外部文件后 RT 索引外部文件未被清理的问题。
* [Issue #1307](https://github.com/manticoresoftware/manticoresearch/issues/1307) "SELECT 语句 sum(value) as value 功能异常": 修正了带别名的选择列表表达式可能遮蔽索引属性的问题；并修复了整型 sum 转 int64 的统计问题。
* [Issue #1308](https://github.com/manticoresoftware/manticoresearch/issues/1308) "避免在复制中绑定到 localhost": 确保复制时，对于具有多个 IP 的主机名不绑定到 localhost。
* [Issue #1309](https://github.com/manticoresoftware/manticoresearch/issues/1309) "数据超过 16Mb 返回 MySQL 客户端失败": 修复了向客户端返回超过 16Mb SphinxQL 包的问题。
* [Issue #1310](https://github.com/manticoresoftware/manticoresearch/issues/1310) "外部文件路径应为绝对路径中的错误引用": 修正了 `SHOW CREATE TABLE` 中外部文件完整路径的显示。
* [Issue #1311](https://github.com/manticoresoftware/manticoresearch/issues/1311) "调试版本在处理长字符串片段时崩溃": 现在，`SNIPPET()` 函数可接受超过 255 字符的长字符串。
* [Issue #1312](https://github.com/manticoresoftware/manticoresearch/issues/1312) "kqueue 轮询中使用后删除导致的假崩溃（master-agent）": 修复了在 kqueue 驱动系统（FreeBSD、MacOS 等）中，当 master 无法连接 agent 时导致的崩溃。
* [Issue #1313](https://github.com/manticoresoftware/manticoresearch/issues/1313) "连接自身时间过长": master 连接 MacOS/BSD 上的 agent 时，现在使用统一的连接+查询超时替代仅连接超时。
* [Issue #1314](https://github.com/manticoresoftware/manticoresearch/issues/1314) "含未达成嵌入式同义词的 pq (json 元数据) 加载失败": 修正了 pq 中的嵌入式同义词标记。
* [Issue #1315](https://github.com/manticoresoftware/manticoresearch/issues/1315) "允许部分函数（sint、fibonacci、second、minute、hour、day、month、year、yearmonth、yearmonthday）使用隐式提升的参数值".
* [Issue #1321](https://github.com/manticoresoftware/manticoresearch/issues/1321) "启用全文扫描中辅助索引多线程，但限制线程数": 在 CBO 中实现代码，以更好预测辅助索引在全文查询中的多线程性能。
* [Issue #1322](https://github.com/manticoresoftware/manticoresearch/issues/1322) "使用预计算排序器后 count(*) 查询仍然缓慢": 使用基于预计算数据的排序器时不再初始化迭代器，避免了性能下降。
* [Issue #1411](https://github.com/manticoresoftware/manticoresearch/issues/1141) "sphinxql 查询日志未保留 MVA 的原始查询": 现在，`all()/any()` 也会被记录。

## 版本 6.0.4
发布于：2023年3月15日

### 新功能
* 改进了与 Logstash、Beats 等的集成，包括：
  - 支持 Logstash 版本 7.6 - 7.15，Filebeat 版本 7.7 - 7.12
  - 自动模式支持。
  - 添加了对 Elasticsearch 格式的批量请求的处理。
* [Buddy 提交 ce90](https://github.com/manticoresoftware/manticoresearch-buddy/commit/ce907ea) Manticore 启动时显示 Buddy 版本。

### Bug修复
* [问题 #588](https://github.com/manticoresoftware/manticoresearch/issues/588), [问题 #942](https://github.com/manticoresoftware/manticoresearch/issues/942) 修复了大ram索引中搜索元数据和调用关键字的错误字符。
* [问题 #1027](https://github.com/manticoresoftware/manticoresearch/issues/1027) 拒绝小写 HTTP 头。
* ❗[问题 #1039](https://github.com/manticoresoftware/manticoresearch/issues/1039) 修复了守护进程在读取 Buddy 控制台输出时的内存泄漏。
* [问题 #1056](https://github.com/manticoresoftware/manticoresearch/issues/1056) 修复问号的异常行为。
* [问题 #1064](https://github.com/manticoresoftware/manticoresearch/issues/1064) - 修复了标记器小写表的竞争条件导致崩溃的问题。
* [提交 59bb](https://github.com/manticoresoftware/manticoresearch/commit/59bb54c) 修复了 JSON 接口中对 id 明确设置为 null 文档的批量写入处理。
* [提交 7b6b](https://github.com/manticoresoftware/manticoresearch/commit/7b6b25f) 修复了 CALL KEYWORDS 中多次出现同一术语的术语统计。
* [提交 f381](https://github.com/manticoresoftware/manticoresearch/commit/f381ad2) Windows 安装程序现在创建默认配置；运行时不再替换路径。
* [提交 6940](https://github.com/manticoresoftware/manticoresearch/commit/6940e95), [提交 cc5a](https://github.com/manticoresoftware/manticoresearch/commit/cc5a480) 修复了多网络节点集群的复制问题。
* [提交 4972](https://github.com/manticoresoftware/manticoresearch/commit/49722ab) 修复 `/pq` HTTP 端点为 `/json/pq` HTTP 端点的别名。
* [提交 3b53](https://github.com/manticoresoftware/manticoresearch/commit/3b5385a) 修复了 Buddy 重启时的守护进程崩溃。
* [Buddy 提交 fba9](https://github.com/manticoresoftware/manticoresearch-buddy/commit/fba9c8c) 在收到无效请求时显示原始错误。
* [Buddy 提交 db95](https://github.com/manticoresoftware/manticoresearch-buddy/commit/db9532c) 允许备份路径中含空格并对正则表达式做了优化以支持单引号。

## 版本 6.0.2
发布于：2023年2月10日

### Bug修复
* [问题 #1024 crash 2](https://github.com/manticoresoftware/manticoresearch/issues/1024) 在较大数量结果的 Facet 搜索时发生崩溃/段错误。
* ❗[问题 #1029](https://github.com/manticoresoftware/manticoresearch/issues/1029) - 警告：编译时值 KNOWN_CREATE_SIZE（16）小于测量值（208）。请考虑修正此值！
* ❗[问题 #1032](https://github.com/manticoresoftware/manticoresearch/issues/1032) - Manticore 6.0.0 plain 索引崩溃。
* ❗[问题 #1033](https://github.com/manticoresoftware/manticoresearch/issues/1033) - 守护进程重启后分布式丢失多个。
* ❗[问题 #1064](https://github.com/manticoresoftware/manticoresearch/issues/1064) - 标记器小写表中的竞争条件。

## 版本 6.0.0
发布于：2023年2月7日

从此版本开始，Manticore Search 配备了 Manticore Buddy，这是一个用 PHP 编写的 sidecar 守护进程，处理不需要超低延迟或高吞吐量的高级功能。Manticore Buddy 在后台运行，用户甚至可能察觉不到它的存在。尽管对终端用户是隐形的，但让 Manticore Buddy 易于安装并与基于 C++ 的主守护进程兼容是一个重大挑战。这一重大变化将允许团队开发一系列新的高级功能，如分片协调、访问控制和身份验证，以及各种集成，如 mysqldump、DBeaver、Grafana mysql 连接器。目前它已经处理了 [SHOW QUERIES](Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES)、[BACKUP](Securing_and_compacting_a_table/Backup_and_restore.md#BACKUP-SQL-command-reference) 和 [自动模式](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema)。

该版本还包含了 130 多个错误修复和众多功能，其中许多可视为重大更新。

### 重大变更
* 🔬 实验性功能：您现在可以执行与 Elasticsearch 兼容的[插入](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md?example=Elasticsearch#Adding-documents-to-a-real-time-table)和[替换](Data_creation_and_modification/Updating_documents/REPLACE.md?example=Elasticsearch#REPLACE) JSON 查询，从而使 Manticore 能够与 Logstash（版本 < 7.13）、Filebeat 及 Beats 家族的其他工具兼容。默认启用。您可以使用 `SET GLOBAL ES_COMPAT=off` 来禁用此功能。
* 支持 [Manticore Columnar Library 2.0.0](https://github.com/manticoresoftware/columnar/)，在[二级索引](Server_settings/Searchd.md#secondary_indexes)方面带来众多修复和改进。**⚠️ 破坏性变更**：从此版本起，二级索引默认开启。如果您是从 Manticore 5 升级，请确保执行 [ALTER TABLE table_name REBUILD SECONDARY](Updating_table_schema_and_settings.md#Rebuilding-a-secondary-index)。详情见下文。
* [提交 c436](https://github.com/manticoresoftware/manticoresearch/commit/c436f9023536f767610451911955ae36d90aa638) 自动模式：您现在可以跳过创建表，只需插入第一个文档，Manticore 将根据该文档的字段自动创建表。详细内容请阅读[这里](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema)。您可以使用 [searchd.auto_schema](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema) 来开启/关闭此功能。
* 对[基于成本的优化器](Searching/Cost_based_optimizer.md)进行了大幅改造，在许多情况下降低了查询响应时间。
  - [问题 #1008](https://github.com/manticoresoftware/manticoresearch/issues/1008) CBO 中的并行化性能估计。
  - [问题 #1014](https://github.com/manticoresoftware/manticoresearch/issues/1014) CBO 现在支持识别[二级索引](Server_settings/Searchd.md#secondary_indexes)，并能够更智能地工作。
  - [提交 cef9](https://github.com/manticoresoftware/columnar/commit/cef9b54dc3a4ee9138ebc4df3a671715b996d6b3) 将列存表/字段的编码统计存储在元数据中，以帮助 CBO 作出更智能的决策。
  - [提交 2b95](https://github.com/manticoresoftware/manticoresearch/commit/2b95fb8c2ba422c9c5a66e2b61c1f7922f31c7aa) 添加了用于微调 CBO 行为的提示。
* [遥测](Telemetry.md#Telemetry)：我们高兴地宣布在此版本中加入遥测功能。此功能允许我们收集匿名和去标识化的指标，帮助我们提升产品的性能和用户体验。请放心，所有收集的数据都是**完全匿名的，且不会与任何个人信息关联**。如果需要，该功能可以在设置中[轻松关闭](Telemetry.md#Telemetry)。
* [提交 5aaf](https://github.com/manticoresoftware/manticoresearch/commit/5aafb17d81b601e399e212a344a4738f65576cdd) [ALTER TABLE table_name REBUILD SECONDARY](Updating_table_schema_and_settings.md#Rebuilding-a-secondary-index) 命令，用于在任何时候重建二级索引，例如：
  - 从 Manticore 5 迁移到新版本时，
  - 对索引中的属性进行[UPDATE](Data_creation_and_modification/Updating_documents/UPDATE.md#UPDATE)（即[就地更新，而非替换](Data_creation_and_modification/Updating_documents/REPLACE_vs_UPDATE.md)）时
* [问题 #821](https://github.com/manticoresoftware/manticoresearch/issues/821) 新工具 `manticore-backup`，用于[备份和恢复 Manticore 实例](Securing_and_compacting_a_table/Backup_and_restore.md)
* SQL 命令 [BACKUP](Securing_and_compacting_a_table/Backup_and_restore.md#BACKUP-SQL-command-reference) 支持在 Manticore 内部进行备份。
* SQL 命令 [SHOW QUERIES](Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES) 作为查看当前正在运行查询的简便方式，区分于线程查看。
* [问题 #551](https://github.com/manticoresoftware/manticoresearch/issues/551) SQL 命令 `KILL` 用于终止长时间运行的 `SELECT` 查询。
* 聚合查询支持动态 `max_matches`，以提高准确性并降低响应时间。

### 次要改动
* [问题 #822](https://github.com/manticoresoftware/manticoresearch/issues/822) SQL 命令 [FREEZE/UNFREEZE](Securing_and_compacting_a_table/Freezing_a_table.md) 用于准备实时表/普通表进行备份。
* [提交 c470](https://github.com/manticoresoftware/manticoresearch/commit/c47052f18ad4868134de1a97e8d7da570e531dfa) 新增设置 `accurate_aggregation` 和 `max_matches_increase_threshold`，用于控制聚合查询的准确度。
* [问题 #718](https://github.com/manticoresoftware/manticoresearch/issues/718) 支持有符号的负 64 位 ID。请注意，仍然无法使用大于 2^63 的 ID，但现在可以使用范围从 -2^63 到 0 的 ID。
* 由于最近新增了二级索引支持，“index”一词变得容易引起混淆，因为它可能指二级索引、全文索引，或普通/实时索引。为减少混淆，我们将后者重命名为“table”。以下 SQL/命令行命令受此变更影响。旧版本已弃用，但仍可用：
  - `index <表名>` => `table <表名>`，
  - `searchd -i / --index` => `searchd -t / --table`，
  - `SHOW INDEX STATUS` => `SHOW TABLE STATUS`，
  - `SHOW INDEX SETTINGS` => `SHOW TABLE SETTINGS`，
  - `FLUSH RTINDEX` => `FLUSH TABLE`，
  - `OPTIMIZE INDEX` => `OPTIMIZE TABLE`，
  - `ATTACH TABLE plain TO RTINDEX rt` => `ATTACH TABLE plain TO TABLE rt`，
  - `RELOAD INDEX` => `RELOAD TABLE`，
  - `RELOAD INDEXES` => `RELOAD TABLES`。

  我们无计划废弃旧形式，但为确保文档兼容性，建议在应用中更改相应命令名称。将来版本中，“index”到“table”的替换也将体现在各类 SQL 和 JSON 命令的输出中。
* 状态 UDF 查询现在被强制在单线程中执行。
* [问题 #1011](https://github.com/manticoresoftware/manticoresearch/issues/1011) 对时间调度相关功能进行了重构，为并行块合并做准备。
* **⚠️ 重大变更**：列式存储格式已更改。您需要重建包含列属性的表。
* **⚠️ 重大变更**：二级索引的文件格式已更改，因此如果您使用二级索引进行搜索，并且配置文件中设置了 `searchd.secondary_indexes = 1`，请注意新版本的 Manticore **将跳过加载具有二级索引的表**。建议操作如下：
  - 升级前，将配置文件中的 `searchd.secondary_indexes` 修改为 0。
  - 启动实例，Manticore 会加载表并给出警告。
  - 对每个索引运行 `ALTER TABLE <table name> REBUILD SECONDARY` 命令以重建二级索引。

  如果您正在运行复制集群，则需要在所有节点上运行 `ALTER TABLE <table name> REBUILD SECONDARY`，或者按照[此说明](Securing_and_compacting_a_table/Compacting_a_table.md#Optimizing-clustered-tables)操作，但改为运行 `ALTER .. REBUILD SECONDARY` 而不是 `OPTIMIZE`。
* **⚠️ 重大变更**：binlog 版本已更新，因此无法重放先前版本的任何 binlog。升级过程中务必确保 Manticore Search 干净停止。这意味着在停止之前的实例后，`/var/lib/manticore/binlog/` 中除 `binlog.meta` 外不应有任何 binlog 文件。
* [Issue #849](https://github.com/manticoresoftware/manticoresearch/issues/849) `SHOW SETTINGS`：现在可以从 Manticore 内部查看配置文件中的设置。
* [Issue #1007](https://github.com/manticoresoftware/manticoresearch/issues/1007) [SET GLOBAL CPUSTATS=1/0](Server_settings/Setting_variables_online.md#SET) 开启/关闭 CPU 时间跟踪；当 CPU 时间跟踪关闭时，[SHOW THREADS](Node_info_and_management/SHOW_THREADS.md) 不再显示 CPU 统计信息。
* [Issue #1009](https://github.com/manticoresoftware/manticoresearch/issues/1009) RT 表的 RAM chunk 段现在可以在 RAM chunk 刷新时合并。
* [Issue #1012](https://github.com/manticoresoftware/manticoresearch/issues/1012) 在 [indexer](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) 输出中新增了二级索引的进度信息。
* [Issue #1013](https://github.com/manticoresoftware/manticoresearch/issues/1013) 之前如果启动时无法服务某个表记录，Manticore 会从索引列表中删除它。新行为是将其保留在列表中，尝试在下次启动时加载。
* [indextool --docextract](Miscellaneous_tools.md#indextool) 返回属于请求文档的所有词和命中。
* [Commit 2b29](https://github.com/manticoresoftware/manticoresearch/commit/2b296ee20e520b85bcbb4383e87095d31e7165dc) 环境变量 `dump_corrupt_meta` 使得当 searchd 无法加载索引时，将损坏的表元数据转储到日志。
* [Commit c7a3](https://github.com/manticoresoftware/manticoresearch/commit/c7a3136eefe0eb2f69e3d99fae9c10beb5821ea6) `DEBUG META` 可以显示 `max_matches` 和伪分片统计信息。
* [Commit 6bca](https://github.com/manticoresoftware/manticoresearch/commit/6bca4848dc5c8396dbc1e057be4b9b2d579794e8) 替换了“Index header format is not json, will try it as binary...”的模糊错误提示为更准确的错误信息。
* [Commit bef3](https://github.com/manticoresoftware/lemmatizer-uk/commit/bef3ff0386d3ee87ec57619782100972c1122e47) 乌克兰语词形还原器路径已更改。
* [Commit 4ae7](https://github.com/manticoresoftware/manticoresearch/commit/4ae789595329a2951e194d1191ddb3121459a560) 二级索引统计信息已添加到 [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META)。
* [Commit 2e7c](https://github.com/manticoresoftware/manticoresearch/commit/2e7c585e) 现在可使用 Swagger Editor 轻松可视化 JSON 接口 https://manual.manticoresearch.com/Openapi#OpenAPI-specification。
* **⚠️ 重大变更**：复制协议已更改。如果您正在运行复制集群，升级到 Manticore 5 时需要：
  - 首先干净地停止所有节点
  - 然后使用 `--new-cluster` 启动最后停止的节点（在 Linux 上运行工具 `manticore_new_cluster`）。
  - 详情请参阅[重启集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster)。

### 与 Manticore Columnar Library 相关的更改
* 重构了二级索引与 Columnar 存储的集成。
* [Commit efe2](https://github.com/manticoresoftware/columnar/commit/efe26b2a) Manticore Columnar Library 优化，通过部分预先的最小/最大值评估降低响应时间。
* [Commit 2757](https://github.com/manticoresoftware/columnar/commit/2757b99b) 如果磁盘 chunk 合并被中断，守护进程现在会清理与 MCL 相关的临时文件。
* [Commit e9c6](https://github.com/manticoresoftware/manticoresearch/commit/e9c672014b63e2b49e7a5a53880583af86c3ab4e) 在崩溃时将 Columnar 和二级库版本转储到日志。
* [Commit f5e8](https://github.com/manticoresoftware/columnar/commit/f5e84eeb8b6cb102f13e3468246702427527fef9) 为二级索引添加了快速文档列表回绕支持。
* [Commit 06df](https://github.com/manticoresoftware/manticoresearch/commit/06dfdd2e668933495dd6cb52d5ba2e0aa9b26e58) 使用 MCL 时，类似 `select attr, count(*) from plain_index`（无过滤条件）的查询现在更快。
* [Commit 0a76](https://github.com/manticoresoftware/manticoresearch/commit/0a76ecb9adbe886868b962dd425de5f475bbf55e) 在 HandleMysqlSelectSysvar 中引入 @@autocommit，以兼容 .net 连接器 8.25 以上版本。
* **⚠️ 重大变更**：[MCL Issue #17](https://github.com/manticoresoftware/columnar/issues/17) MCL：为 columnar 扫描添加 SSE 代码。MCL 现在至少要求 SSE4.2。

### 与打包相关的更改
* [Commit 4d19](https://github.com/manticoresoftware/manticoresearch/commit/4d19f5cbe49a31228aa09253f061165bfe80e51b) **⚠️ 重大变更**：不再支持 Debian Stretch 和 Ubuntu Xenial。
* 支持 RHEL 9，包括 Centos 9、Alma Linux 9 和 Oracle Linux 9。
* [Issue #924](https://github.com/manticoresoftware/manticoresearch/issues/924) 支持 Debian Bookworm。
* [Issue #636](https://github.com/manticoresoftware/manticoresearch/issues/636) 打包：Linux 和 MacOS 的 arm64 构建。
* [PR #26](https://github.com/manticoresoftware/docker/pull/26) 多架构（x86_64 / arm64）docker 镜像。
* 为贡献者简化包构建流程（[编译指南](Installation/Compiling_from_sources.md#Building-using-CI-Docker)）。
* 现在可以使用 APT 安装特定版本。
* [提交 a6b8](https://github.com/manticoresoftware/manticoresearch/commit/51fddca5c2a3ebb8576fae4c18660656ba22de0f) Windows 安装程序（之前只提供了压缩包）。
* 切换至使用 CLang 15 编译。
* **⚠️ 重大变更**：自定义 Homebrew 配方，包括 Manticore Columnar Library 的配方。要安装 Manticore、MCL 以及任何其他必要组件，请使用以下命令 `brew install manticoresoftware/manticore/manticoresearch manticoresoftware/manticore/manticore-extra`。

### 修复的缺陷
* [问题 #479](https://github.com/manticoresoftware/manticoresearch/issues/479) 名为 `text` 的字段
* [问题 #501](https://github.com/manticoresoftware/manticoresearch/issues/501) id 不能是非 bigint 类型
* [问题 #646](https://github.com/manticoresoftware/manticoresearch/issues/646) ALTER 与名为 "text" 的字段冲突
* ❗[问题 #652](https://github.com/manticoresoftware/manticoresearch/issues/652) 可能的 BUG：HTTP (JSON) 的 offset 和 limit 影响 facet 结果
* ❗[问题 #827](https://github.com/manticoresoftware/manticoresearch/issues/827) Searchd 在高负载下挂起/崩溃
* ❗[问题 #996](https://github.com/manticoresoftware/manticoresearch/issues/996) PQ 索引内存不足
* ❗[提交 1041](https://github.com/manticoresoftware/manticoresearch/commit/10416ef7dddf06c0d759e32ccd6ebaa2468f7cbf) `binlog_flush = 1` 自 Sphinx 以来一直有问题。已修复。
* [MCL 问题 #14](https://github.com/manticoresoftware/columnar/issues/14) MCL：当 ft 字段过多时查询崩溃
* [问题 #470](https://github.com/manticoresoftware/manticoresearch/issues/470) sql_joined_field 不能被存储
* [问题 #713](https://github.com/manticoresoftware/manticoresearch/issues/713) 使用 LEVENSHTEIN() 时崩溃
* [问题 #743](https://github.com/manticoresoftware/manticoresearch/issues/743) Manticore 意外崩溃且无法正常重启
* [问题 #788](https://github.com/manticoresoftware/manticoresearch/issues/788) 通过 /sql 调用 KEYWORDS 返回控制字符，导致 json 出错
* [问题 #789](https://github.com/manticoresoftware/manticoresearch/issues/789) mariadb 无法创建 FEDERATED 表
* [问题 #796](https://github.com/manticoresoftware/manticoresearch/issues/796) 警告：dlopen() 失败：/usr/bin/lib_manticore_columnar.so：无法打开共享对象文件：没有那个文件或目录
* [问题 #797](https://github.com/manticoresoftware/manticoresearch/issues/797) 通过 api 使用 ZONESPAN 查询时 Manticore 崩溃
* [问题 #799](https://github.com/manticoresoftware/manticoresearch/issues/799) 使用多个索引和 facet distinct 时权重错误
* [问题 #801](https://github.com/manticoresoftware/manticoresearch/issues/801) SphinxQL group 查询在重新处理 SQL 索引后挂起
* [问题 #802](https://github.com/manticoresoftware/manticoresearch/issues/802) MCL：Indexer 在 5.0.2 和 manticore-columnar-lib 1.15.4 中崩溃
* [问题 #813](https://github.com/manticoresoftware/manticoresearch/issues/813) Manticore 5.0.2 FEDERATED 返回空集（MySQL 8.0.28）
* [问题 #824](https://github.com/manticoresoftware/manticoresearch/issues/824) 在两个索引上进行 COUNT DISTINCT 查询且结果为零时抛出内部错误
* [问题 #826](https://github.com/manticoresoftware/manticoresearch/issues/826) 删除查询导致崩溃
* [问题 #843](https://github.com/manticoresoftware/manticoresearch/issues/843) MCL：长文本字段的 Bug
* [问题 #856](https://github.com/manticoresoftware/manticoresearch/issues/856) 5.0.2 rtindex：聚合搜索限制行为不符合预期
* [问题 #863](https://github.com/manticoresoftware/manticoresearch/issues/863) 返回的 Hits 是 Nonetype 对象，即使搜索应返回多个结果
* [问题 #870](https://github.com/manticoresoftware/manticoresearch/issues/870) 在 SELECT 表达式中使用 Attribute 和 Stored Field 时崩溃
* [问题 #872](https://github.com/manticoresoftware/manticoresearch/issues/872) 崩溃后表变得不可见
* [问题 #877](https://github.com/manticoresoftware/manticoresearch/issues/877) 查询中两个否定词导致错误：查询不可计算
* [问题 #878](https://github.com/manticoresoftware/manticoresearch/issues/878) 通过 json query_string 使用 a -b -c 无效
* [问题 #886](https://github.com/manticoresoftware/manticoresearch/issues/886) pseudo_sharding 与查询匹配
* [问题 #893](https://github.com/manticoresoftware/manticoresearch/issues/893) Manticore 5.0.2 min/max 函数行为不符合预期...
* [问题 #896](https://github.com/manticoresoftware/manticoresearch/issues/896) 字段 "weight" 未被正确解析
* [问题 #897](https://github.com/manticoresoftware/manticoresearch/issues/897) Manticore 服务启动时崩溃并持续重启
* [问题 #900](https://github.com/manticoresoftware/manticoresearch/issues/900) group by j.a、smth 结果错误
* [问题 #913](https://github.com/manticoresoftware/manticoresearch/issues/913) 当排名器中使用 expr 且查询有两个近似词时，Searchd 崩溃
* [问题 #916](https://github.com/manticoresoftware/manticoresearch/issues/916) net_throttle_action 出错
* [问题 #919](https://github.com/manticoresoftware/manticoresearch/issues/919) MCL：Manticore 查询执行时崩溃，以及集群恢复过程中其他崩溃。
* [问题 #925](https://github.com/manticoresoftware/manticoresearch/issues/925) SHOW CREATE TABLE 输出时缺少反引号
* [问题 #930](https://github.com/manticoresoftware/manticoresearch/issues/930) 现在可以通过 JDBC 连接器从 Java 查询 Manticore
* [问题 #933](https://github.com/manticoresoftware/manticoresearch/issues/933) bm25f 排序问题
* [问题 #934](https://github.com/manticoresoftware/manticoresearch/issues/934) 无配置索引在 watchdog 的初始加载状态冻结
* [问题 #937](https://github.com/manticoresoftware/manticoresearch/issues/937) 对 facet 数据排序时出现段错误
* [问题 #940](https://github.com/manticoresoftware/manticoresearch/issues/940) CONCAT(TO_STRING) 崩溃
* [问题 #947](https://github.com/manticoresoftware/manticoresearch/issues/947) 在某些情况下，单个简单的select查询可能导致整个实例挂起，因此在运行的select完成之前，您无法登录或运行任何其他查询。
* [问题 #948](https://github.com/manticoresoftware/manticoresearch/issues/948) 索引器崩溃
* [问题 #950](https://github.com/manticoresoftware/manticoresearch/issues/950) facet distinct 返回错误的计数
* [问题 #953](https://github.com/manticoresoftware/manticoresearch/issues/953) 内置 sph04 排序器中的 LCS 计算不正确
* [问题 #955](https://github.com/manticoresoftware/manticoresearch/issues/955) 5.0.3 开发版本崩溃
* [问题 #963](https://github.com/manticoresoftware/manticoresearch/issues/963) 在引擎列存储中，带有 json 的 FACET 导致崩溃
* [问题 #982](https://github.com/manticoresoftware/manticoresearch/issues/982) MCL: 5.0.3 来自次级索引的崩溃
* [PR #984](https://github.com/manticoresoftware/manticoresearch/pull/984) @@autocommit 在 HandleMysqlSelectSysvar 中
* [PR #985](https://github.com/manticoresoftware/manticoresearch/pull/985) 修复 RT 索引中的线程-块分配
* [问题 #985](https://github.com/manticoresoftware/manticoresearch/issues/985) 修复 RT 索引中的线程-块分配
* [问题 #986](https://github.com/manticoresoftware/manticoresearch/issues/986) 错误的默认 max_query_time
* [问题 #987](https://github.com/manticoresoftware/manticoresearch/issues/987) 多线程执行时使用正则表达式导致崩溃
* [问题 #988](https://github.com/manticoresoftware/manticoresearch/issues/988) 向后索引兼容性破坏
* [问题 #989](https://github.com/manticoresoftware/manticoresearch/issues/989) indextool 报告检查列存储属性时出错
* [问题 #990](https://github.com/manticoresoftware/manticoresearch/issues/990) json grouper 克隆的内存泄漏
* [问题 #991](https://github.com/manticoresoftware/manticoresearch/issues/991) levenshtein 函数克隆的内存泄漏
* [问题 #992](https://github.com/manticoresoftware/manticoresearch/issues/992) 加载元数据时错误信息丢失
* [问题 #993](https://github.com/manticoresoftware/manticoresearch/issues/993) 传递动态索引/子键和系统变量的错误
* [问题 #994](https://github.com/manticoresoftware/manticoresearch/issues/994) 在列存储中对列存储字符串进行 count distinct 导致崩溃
* [问题 #995](https://github.com/manticoresoftware/manticoresearch/issues/995) MCL: 从 taxi1 获取 min(pickup_datetime) 时崩溃
* [问题 #997](https://github.com/manticoresoftware/manticoresearch/issues/997) 空的 excludes JSON 查询从选择列表中移除列
* [问题 #998](https://github.com/manticoresoftware/manticoresearch/issues/998) 次级任务在当前调度器上运行有时会导致异常副作用
* [问题 #999](https://github.com/manticoresoftware/manticoresearch/issues/999) facet distinct 与不同模式的崩溃
* [问题 #1000](https://github.com/manticoresoftware/manticoresearch/issues/1000) MCL: 在没有列存库情况下运行后，列存实时索引损坏
* [问题 #1001](https://github.com/manticoresoftware/manticoresearch/issues/1001) json 中隐式 cutoff 不起作用
* [问题 #1002](https://github.com/manticoresoftware/manticoresearch/issues/1002) 列存 grouper 问题
* [问题 #1003](https://github.com/manticoresoftware/manticoresearch/issues/1003) 无法从索引中删除最后一个字段
* [问题 #1004](https://github.com/manticoresoftware/manticoresearch/issues/1004) --new-cluster 之后行为异常
* [问题 #1005](https://github.com/manticoresoftware/manticoresearch/issues/1005) “columnar library not loaded”，但其实不需要
* [问题 #1006](https://github.com/manticoresoftware/manticoresearch/issues/1006) 删除查询没有错误提示
* [问题 #1010](https://github.com/manticoresoftware/manticoresearch/issues/1010) 修复 Windows 构建中 ICU 数据文件位置
* [PR #1018](https://github.com/manticoresoftware/manticoresearch/pull/1018) 握手发送问题
* [问题 #1020](https://github.com/manticoresoftware/manticoresearch/issues/1020) 在 show create table 中显示 id
* [问题 #1024 崩溃 1](https://github.com/manticoresoftware/manticoresearch/issues/1024) Facet 搜索大量结果时崩溃 / 段错误。
* [问题 #1026](https://github.com/manticoresoftware/manticoresearch/issues/1026) RT 索引：当大量文档插入且 RAMchunk 满时，searchd “永久卡住”
* [提交 4739](https://github.com/manticoresoftware/manticoresearch/commit/4739dafd) 节点间复制繁忙时关闭时线程卡住
* [提交 ab87](https://github.com/manticoresoftware/manticoresearch/commit/ab87836782e7ae43fe5f9dea739eed0d336b42c1) JSON 范围过滤器中混合使用浮点数和整数可能导致 Manticore 忽略该过滤器
* [提交 d001](https://github.com/manticoresoftware/manticoresearch/commit/d00101c2905f6393ce3fca23c4f6dcb2506f4bd9) JSON 中的浮点过滤器不准确
* [提交 4092](https://github.com/manticoresoftware/manticoresearch/commit/4092c6a8) 索引更改时丢弃未提交事务（否则可能崩溃）
* [提交 9692](https://github.com/manticoresoftware/manticoresearch/commit/9692a840) 使用反斜杠时查询语法错误
* [提交 0c19](https://github.com/manticoresoftware/manticoresearch/commit/0c19ad8d) SHOW STATUS 中的 workers_clients 可能错误
* [提交 1772](https://github.com/manticoresoftware/manticoresearch/commit/17720e1e2f2ec38337df565bee78f7ce9d9c744a) 修复合并无文档存储的内存段时崩溃
* [提交 f45b](https://github.com/manticoresoftware/manticoresearch/commit/f45b337b5c265433bb7d6262ad65d02f65edbaf3) 修复 JSON 过滤器中遗漏的 ALL/ANY 等号条件
* [提交 3e83](https://github.com/manticoresoftware/manticoresearch/commit/3e83c1f0c0e6e7a9d4297113f69a112cbabd9e64) 如果 searchd 从不可写目录启动，复制可能失败并报错 `got exception while reading ist stream: mkstemp(./gmb_pF6TJi) failed: 13 (Permission denied)`。
* [Commit 92e5](https://github.com/manticoresoftware/manticoresearch/commit/92e5629f7150f62524b385fca670a01ba9590f5b) 自 4.0.2 版本起崩溃日志仅包含偏移量。本提交修复了该问题。

## 版本 5.0.2
发布：2022 年 5 月 30 日

### Bug 修复
* ❗[Issue #791](https://github.com/manticoresoftware/manticoresearch/issues/791) - 错误的栈大小可能导致崩溃。

## 版本 5.0.0
发布：2022 年 5 月 18 日


### 主要新功能
* 🔬 支持 [Manticore Columnar Library 1.15.2](https://github.com/manticoresoftware/columnar/)，启用 [二级索引](Server_settings/Searchd.md#secondary_indexes) 测试版。对于普通和实时的列式及行式索引（如果使用了 [Manticore Columnar Library](https://github.com/manticoresoftware/columnar)），默认开启二级索引的构建，但要启用搜索需要在配置文件中设置 `secondary_indexes = 1` 或使用 [SET GLOBAL](Server_settings/Setting_variables_online.md)。此新功能支持所有操作系统，除了较旧的 Debian Stretch 和 Ubuntu Xenial。
* [只读模式](Security/Read_only.md)：现在可以指定仅处理读取查询并丢弃任何写操作的监听器。
* 新增 [/cli](Connecting_to_the_server/HTTP.md#/cli) 端点，使通过 HTTP 运行 SQL 查询更简单。
* 通过 HTTP JSON 实现更快的批量 INSERT/REPLACE/DELETE：之前可以通过 HTTP JSON 协议提供多个写命令，但它们是逐条处理的，现在作为单个事务处理。
* [#720](https://github.com/manticoresoftware/manticoresearch/issues/720) JSON 协议支持 [嵌套过滤器](Searching/Filters.md#Nested-bool-query)。之前在 JSON 中无法编写类似 `a=1 and (b=2 or c=3)` 的语句：`must`（AND）、`should`（OR）和 `must_not`（NOT）仅支持最高层级，现在支持嵌套。
* 支持 HTTP 协议中的 [分块传输编码](https://en.wikipedia.org/wiki/Chunked_transfer_encoding)。你现在可以在应用中使用分块传输来传输大型批次，降低资源消耗（无需计算 `Content-Length`）。服务器端，Manticore 现在始终以流式方式处理传入的 HTTP 数据，不再等待整个批次传输完成，这：
  - 降低峰值内存使用，减少 OOM 风险
  - 缩短响应时间（我们的测试显示处理 100MB 批次时间减少约 11%）
  - 允许绕过 [max_packet_size](Server_settings/Searchd.md#max_packet_size)，传输超过 `max_packet_size`（128MB）限制的大批次，例如一次传输 1GB。
* [#719](https://github.com/manticoresoftware/manticoresearch/issues/719) HTTP 接口支持 `100 Continue`：现在可以使用 `curl` （包括多种编程语言使用的 curl 库）传输大批次，curl 默认使用 `Expect: 100-continue` 并在发送批次前等待一段时间。之前需要添加 `Expect: ` 头部，现在不再需要。

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
  {"insert": {"index": "user", "doc":  {"name":"约翰·史密斯先生","email":"stiedemann.tristin@ziemann.com","description":"Temporibus amet magnam consequatur omnis consequatur illo fugit. Debitis natus doloremque est tempore deserunt vero. Harum eos corrupti nemo ut.","age":89,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"赫克托·普罗斯","email":"hickle.mafalda@hotmail.com","description":" as voluptatem inventore sit. Aliquam fugit perferendis est id aut odio et sapiente.","age":64,"active":1}}}'
  *   尝试连接 127.0.0.1...
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
  * 完全上传完毕，状态良好
  < HTTP/1.1 200 OK
  < Server: 4.2.1 63e5749@220405 dev
  < Content-Type: application/json; charset=UTF-8
  < Content-Length: 147
  <
  * 与主机 localhost 的连接 #0 保持不变
  {"items":[{"bulk":{"table":"user","_id":2811798919590182916,"created":4,"deleted":0,"updated":0,"result":"created","status":201}}],"errors":false}
  real

0m0.015s

  user
0m0.005s
  sys
0m0.004s
  ```
  </details>

* **⚠️ 重大变更**: [伪分片](Server_settings/Searchd.md#pseudo_sharding) 默认启用。若要禁用，确保在 Manticore 配置文件的 `searchd` 部分添加 `pseudo_sharding = 0`。
* 实时/普通索引中至少有一个全文字段不再是强制要求。现在即使不涉及全文搜索，也可以使用 Manticore。
* 对由 [Manticore 列存库](https://github.com/manticoresoftware/columnar) 支持的属性实现了[快速抓取](Creating_a_table/Data_types.md#fast_fetch)：像 `select * from <columnar table>` 这样的查询现在比以前快很多，特别是当架构中有许多字段时。
* **⚠️ 重大变更**: 隐式[截止](Searching/Options.md#cutoff)。Manticore 现在不会花费时间和资源去处理结果集中不需要的数据。缺点是它会影响 [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META) 中的 `total_found` 和 JSON 输出中的 [hits.total](Searching/Full_text_matching/Basic_usage.md#HTTP-JSON)。只有在看到 `total_relation: eq` 时才准确，而 `total_relation: gte` 表示匹配的实际文档数量大于 `total_found` 体现的值。想保留之前的行为，可以使用搜索选项 `cutoff=0`，这样 `total_relation` 总是 `eq`。
* **⚠️ 重大变更**: 所有全文字段现在默认[存储](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#stored_fields)。需要使用 `stored_fields = `（空值）将所有字段设置为非存储（即恢复之前的行为）。
* [#715](https://github.com/manticoresoftware/manticoresearch/issues/715) HTTP JSON 支持[搜索选项](Searching/Options.md#General-syntax)。
### 小幅变更
* **⚠️ 重大变更**: 索引元数据文件格式变更。之前的元文件（`.meta`，`.sph`）是二进制格式，现在改为 json。新版本 Manticore 会自动转换旧索引，但是：
  - 可能会出现类似 `WARNING: ... syntax error, unexpected TOK_IDENT` 的警告
  - 旧版本 Manticore 无法运行转换后的索引，请确保备份
* **⚠️ 重大变更**: 通过 [HTTP keep-alive](Connecting_to_the_server/HTTP.md#Keep-alive) 实现会话状态支持。当客户端也支持时，HTTP 变为有状态。例如，使用新的 [/cli](Connecting_to_the_server/HTTP.md#/cli) 端点和默认开启的 HTTP keep-alive（所有浏览器默认开启），你可以在 `SELECT` 后调用 `SHOW META`，效果与通过 mysql 方式相同。注意，之前支持的 `Connection: keep-alive` HTTP 头只会重用连接，从本版本起它也实现了会话状态。
* 现在可以使用 `columnar_attrs = *` 在[普通模式](Read_this_first.md#Real-time-mode-vs-plain-mode)下将所有属性定义为列存，适合属性列表较长的场景。
* 更快的复制 SST
* **⚠️ 重大变更**: 复制协议已更改。如果运行复制集群，升级到 Manticore 5 时需要：
  - 首先干净停止所有节点
  - 之后使用 `--new-cluster` 启动最后停止的节点（Linux 下运行工具 `manticore_new_cluster`）
  - 更多细节请阅读 [重启集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster)。
* 复制相关改进：
  - 更快的 SST
  - 抗干扰性提升，有助于应对复制节点间不稳定的网络
  - 日志记录改进
* 安全性提升：当配置文件中没有指定 `listen` 时，Manticore 现在监听 `127.0.0.1` 而非 `0.0.0.0`。虽然 Manticore 搜索默认配置指定了 `listen`，且无 `listen` 配置并不常见，但仍可能出现。之前监听 `0.0.0.0` 不安全，现在监听 `127.0.0.1`，通常不对公网开放。
* 针对列存属性的聚合更快。
* `AVG()` 准确性提高：之前聚合内部使用 `float`，现在改为使用 `double`，显著提高准确度。
* 改进对 JDBC MySQL 驱动的支持。
* 为 [jemalloc](https://github.com/jemalloc/jemalloc) 提供 `DEBUG malloc_stats` 支持。
* [optimize_cutoff](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#optimize_cutoff) 现在可作为表级设置，在创建或修改表时配置。
* **⚠️ 重大变更**：[query_log_format](Server_settings/Searchd.md#query_log_format) 现在默认是 **`sphinxql`**。如果您习惯使用 `plain` 格式，需要在配置文件中添加 `query_log_format = plain`。
* 显著的内存消耗改进：在使用存储字段且执行长时间且密集的插入/替换/优化工作负载时，Manticore 现在显著减少了 RAM 的使用。
* [shutdown_timeout](Server_settings/Searchd.md#shutdown_timeout) 默认值从3秒增加到60秒。

* [提交 ffd0](https://github.com/manticoresoftware/manticoresearch/commit/ffd0499d329d2c383f14c8a44c4cc84338ab56e7) 支持 Java mysql 连接器 >= 6.0.3：[Java mysql 连接 6.0.3](https://mvnrepository.com/artifact/mysql/mysql-connector-java/6.0.3) 中他们更改了连接 mysql 的方式，破坏了与 Manticore 的兼容性。如今支持了新的行为。
* [提交 1da6](https://github.com/manticoresoftware/manticoresearch/commit/1da6dbec) 禁止在加载索引时（例如 searchd 启动时）保存新的磁盘块。
* [问题 #746](https://github.com/manticoresoftware/manticoresearch/issues/746) 支持 glibc >= 2.34。
* [问题 #784](https://github.com/manticoresoftware/manticoresearch/issues/784) 将“VIP”连接数量单独计数，与普通（非VIP）连接分开。此前 VIP 连接计入 `max_connections` 限制，可能导致非 VIP 连接出现“连接数已满”错误。现在 VIP 连接不计入该限制。当前 VIP 连接数也可以在 `SHOW STATUS` 和 `status` 中查看。
* [ID](Creating_a_table/Data_types.md#Document-ID) 现在可以显式指定。
* [问题 #687](https://github.com/manticoresoftware/manticoresearch/issues/687) 支持 mysql 协议的 zstd 压缩。
### ⚠️ 其他小的重大变更
* ⚠️ BM25F 公式稍作更新以提升搜索相关性。仅影响使用 [BM25F()](Functions/Searching_and_ranking_functions.md#BM25F%28%29) 函数时的搜索结果，对默认排名公式行为无影响。
* ⚠️ 更改 REST [/sql](Connecting_to_the_server/HTTP.md#mode=raw) 端点行为：`/sql?mode=raw` 现在要求转义，并返回数组。

* ⚠️ `/bulk` 的 INSERT/REPLACE/DELETE 请求响应格式变更：
  - 以前每个子查询都是单独的事务，且产生单独响应
  - 现在整个批处理视为单一事务，只返回一个响应
* ⚠️ 搜索选项 `low_priority` 和 `boolean_simplify` 现在需要赋值（`0/1`），以前可以写成 `SELECT ... OPTION low_priority, boolean_simplify`，现在必须写成 `SELECT ... OPTION low_priority=1, boolean_simplify=1`。
* ⚠️ 如果您使用旧版本的 [php](https://github.com/manticoresoftware/manticoresearch/blob/master/api/sphinxapi.php)、[python](https://github.com/manticoresoftware/manticoresearch/blob/master/api/sphinxapi.py) 或者 [java](https://github.com/manticoresoftware/manticoresearch/tree/master/api/java) 客户端，请访问相应链接查找更新版本。**旧版本与 Manticore 5 不完全兼容。**
* ⚠️ HTTP JSON 请求在 `query_log_format=sphinxql` 模式下现在用不同格式记录。之前仅记录全文部分，现在按原样记录。
### 新软件包
* **⚠️ 重大变更**：由于结构变化，升级到 Manticore 5 时，建议先删除旧软件包，再安装新软件包：
  - 基于 RPM 的系统：`yum remove manticore*`

  - Debian 和 Ubuntu：`apt remove manticore*`
* 新的 deb/rpm 软件包结构。之前的版本提供：
  - `manticore-server`，包含 `searchd` （主搜索守护进程）及其所有依赖
  - `manticore-tools`，包含 `indexer` 和 `indextool`
  - `manticore`，包含全部内容
  - `manticore-all` RPM 作为指向 `manticore-server` 和 `manticore-tools` 的元包
  新结构为：
  - `manticore` - deb/rpm 元包，安装上述所有包作为依赖
  - `manticore-server-core` - `searchd` 及其独立运行所需组件
  - `manticore-server` - systemd 文件及其它辅助脚本

  - `manticore-tools` - `indexer`、`indextool` 及其他工具
  - `manticore-common` - 默认配置文件、默认数据目录、默认停用词
  - `manticore-icudata`、`manticore-dev`、`manticore-converter` 没有大的改变
  - `.tgz` 包含所有软件包的捆绑包
* 支持 Ubuntu Jammy
* 通过 [YUM 仓库](Installation/RHEL_and_Centos.md#YUM-repository) 支持 Amazon Linux 2
### Bug 修复
* [问题 #815](https://github.com/manticoresoftware/manticoresearch/issues/815) 使用 UDF 函数时随机崩溃
* [问题 #287](https://github.com/manticoresoftware/manticoresearch/issues/287) RT 索引时内存溢出
* [问题 #604](https://github.com/manticoresoftware/manticoresearch/issues/604) 3.6.0、4.2.0 sphinxql-parser 的重大变更
* [问题 #667](https://github.com/manticoresoftware/manticoresearch/issues/667) 致命错误：内存不足（无法分配 9007199254740992 字节）
* [问题 #676](https://github.com/manticoresoftware/manticoresearch/issues/676) 字符串未正确传递给 UDF
* ❗[问题 #698](https://github.com/manticoresoftware/manticoresearch/issues/698) 尝试向 rt 索引添加文本列时 Searchd 崩溃
* [问题 #705](https://github.com/manticoresoftware/manticoresearch/issues/705) Indexer 未能找到所有列
* ❗[问题 #709](https://github.com/manticoresoftware/manticoresearch/issues/705) 按 json.boolean 分组错误
* [问题 #716](https://github.com/manticoresoftware/manticoresearch/issues/716) indextool 与索引相关的命令（如 --dumpdict）失败
* ❗[问题 #724](https://github.com/manticoresoftware/manticoresearch/issues/724) 字段从选择中消失
* [问题 #727](https://github.com/manticoresoftware/manticoresearch/issues/727) .NET HttpClient 使用 `application/x-ndjson` 时 Content-Type 不兼容
* [Issue #729](https://github.com/manticoresoftware/manticoresearch/issues/729) 字段长度计算
* ❗[Issue #730](https://github.com/manticoresoftware/manticoresearch/issues/730) 创建/插入到/删除列式表存在内存泄漏
* [Issue #731](https://github.com/manticoresoftware/manticoresearch/issues/731) 在某些条件下结果中的空列
* ❗[Issue #749](https://github.com/manticoresoftware/manticoresearch/issues/749) 守护进程启动时崩溃
* ❗[Issue #750](https://github.com/manticoresoftware/manticoresearch/issues/750) 守护进程启动时挂起
* ❗[Issue #751](https://github.com/manticoresoftware/manticoresearch/issues/751) SST 时崩溃
* [Issue #752](https://github.com/manticoresoftware/manticoresearch/issues/752) 当 engine='columnar' 时，Json 属性标记为列式
* [Issue #753](https://github.com/manticoresoftware/manticoresearch/issues/753) 复制监听在 0 端口
* [Issue #754](https://github.com/manticoresoftware/manticoresearch/issues/754) columnar_attrs = * 在 csvpipe 中无效
* ❗[Issue #755](https://github.com/manticoresoftware/manticoresearch/issues/755) 在 rt 中的列式中选择 float 时崩溃
* ❗[Issue #756](https://github.com/manticoresoftware/manticoresearch/issues/756) Indextool 在检查期间更改 rt 索引
* [Issue #757](https://github.com/manticoresoftware/manticoresearch/issues/757) 需要检查监听器端口范围交叉
* [Issue #758](https://github.com/manticoresoftware/manticoresearch/issues/758) RT 索引保存磁盘块失败时记录原始错误
* [Issue #759](https://github.com/manticoresoftware/manticoresearch/issues/759) 对 RE2 配置只报告一个错误
* ❗[Issue #760](https://github.com/manticoresoftware/manticoresearch/issues/760) 提交 5463778558586d2508697fa82e71d657ac36510f 中 RAM 消耗变化
* [Issue #761](https://github.com/manticoresoftware/manticoresearch/issues/761) 第3节点脏重启后不创建非主集群
* [Issue #762](https://github.com/manticoresoftware/manticoresearch/issues/762) 更新计数器增加了 2
* [Issue #763](https://github.com/manticoresoftware/manticoresearch/issues/763) 新版本 4.2.1 损坏了用 4.2.0 及形态学创建的索引
* [Issue #764](https://github.com/manticoresoftware/manticoresearch/issues/764) json keys /sql?mode=raw 中无转义
* ❗[Issue #765](https://github.com/manticoresoftware/manticoresearch/issues/765) 使用函数隐藏其他值
* ❗[Issue #766](https://github.com/manticoresoftware/manticoresearch/issues/766) FixupAttrForNetwork 中一行触发内存泄漏
* ❗[Issue #767](https://github.com/manticoresoftware/manticoresearch/issues/767) 4.2.0 与 4.2.1 中与 docstore 缓存相关的内存泄漏
* [Issue #768](https://github.com/manticoresoftware/manticoresearch/issues/768) 通过网络的存储字段出现奇怪的乒乓现象
* [Issue #769](https://github.com/manticoresoftware/manticoresearch/issues/769) lemmatizer_base 若未在 'common' 节中提及则重置为空
* [Issue #770](https://github.com/manticoresoftware/manticoresearch/issues/770) pseudo_sharding 使按 id 的 SELECT 变慢
* [Issue #771](https://github.com/manticoresoftware/manticoresearch/issues/771) 使用 jemalloc 时 DEBUG malloc_stats 输出为零
* [Issue #772](https://github.com/manticoresoftware/manticoresearch/issues/772) 删除/添加列使值不可见
* [Issue #773](https://github.com/manticoresoftware/manticoresearch/issues/773) 无法添加 bit(N) 列到列式表
* [Issue #774](https://github.com/manticoresoftware/manticoresearch/issues/774) manticore.json 启动时“cluster”变为空

* ❗[Commit 1da4](https://github.com/manticoresoftware/manticoresearch/commits/1da4ce891e4ff13727cfd3331d72771c64ee7949) HTTP 操作未在 SHOW STATUS 中跟踪

* [Commit 3810](https://github.com/manticoresoftware/manticoresearch/commits/381000ab4af4c3c98ec8c730699ad7f39039cec8) 对低频单关键词查询禁用 pseudo_sharding
* [Commit 8003](https://github.com/manticoresoftware/manticoresearch/commits/800325cca283a87801b28929d82420fafc76a0ee) 修正在存储属性与索引合并时的问题

* [Commit cddf](https://github.com/manticoresoftware/manticoresearch/commits/cddfeed6296f2041c6aae18414e5cd9cabf08281) 泛化了不同值获取器；为列式字符串添加了专门的不同值获取器

* [Commit fba4](https://github.com/manticoresoftware/manticoresearch/commits/fba4bb4f) 修复从文档存储获取空整数属性的问题

* [Commit f300](https://github.com/manticoresoftware/manticoresearch/commit/f3009a9242fa16956adf871c4887e6e0303ba364) 查询日志中 `ranker` 可能被重复指定
## 版本 4.2.0，2021年12月23日

### 主要新功能
* **实时索引及全文查询的伪分片支持**。在之前的版本中我们添加了有限的伪分片支持。从本版本开始，你可以通过启用 [searchd.pseudo_sharding](Server_settings/Searchd.md#pseudo_sharding) 来获取伪分片的所有好处以及多核处理器的优势。最棒的是你不需要对索引或查询做任何改动，只需启用它，如果有空闲的 CPU，则会被用来降低响应时间。它支持普通和实时索引的全文、过滤及分析查询。例如，以下是在 [Hacker news curated comments 数据集](https://zenodo.org/record/45901/)（放大100倍，含1.16亿文档的普通索引）上启用伪分片如何使大多数查询的**平均响应时间约降低10倍**。

![4.2.0 中伪分片开启与关闭](4.2.0_ps_on_vs_off.png)

* [**Debian Bullseye**](https://manticoresearch.com/install/) 版本现已支持。

<!-- example pq_transactions_4.2.0 -->
* PQ 事务现在是原子和隔离的。之前 PQ 事务支持有限。这极大地**加快了 PQ 的 REPLACE 操作**，尤其是在需要一次替换大量规则时。性能细节：

<!-- intro -->
#### 4.0.2
<!-- request 4.0.2 -->
插入 100 万条 PQ 规则需要 **48 秒**，而每批 10K 条替换 4 万条则需要 **406 秒**。
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
插入 100 万条 PQ 规则需要 **34 秒**，而每批 10K 条替换这些规则仅需 **23 秒**。
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
* 现在在 `searchd` 配置区提供了配置选项 [optimize_cutoff](Server_settings/Searchd.md#optimize_cutoff)。当你想全局限制所有索引中的实时分片数量到一个特定数目时非常有用。
* [提交 0087](https://github.com/manticoresoftware/manticoresearch/commit/00874743823eaf43e2a09a088392fd7e0a617f47) 对几个具有相同字段集/顺序的本地物理索引（实时/普通）进行了准确的 [count(distinct ...)](Searching/Grouping.md#COUNT%28DISTINCT-field%29) 和 [FACET ... distinct](Searching/Faceted_search.md#Faceting-without-duplicates) 统计。
* [合并请求 #598](https://github.com/manticoresoftware/manticoresearch/pull/598) 为 `YEAR()` 和其他时间戳函数增加 bigint 支持。

* [提交 8e85](https://github.com/manticoresoftware/manticoresearch/commit/8e85d4bce596cc88f9d9615ea243a27e4afde0f7) 自适应 [rt_mem_limit](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_mem_limit)。之前 Manticore Search 会精确收集到 `rt_mem_limit` 大小的数据量后才保存一个新的磁盘分片，在保存期间仍会收集多达 10%（即双缓冲）以最大限度减少可能的插入暂停。如果这限制也被用尽，则直到磁盘分片完全写入磁盘之前，添加新文档操作会被阻塞。新的自适应限制基于现在已有的 [auto-optimize](Server_settings/Searchd.md#auto_optimize)，因此磁盘分片不完全遵守 `rt_mem_limit` 并提前开始刷新磁盘分片不是大问题。所以，我们现在收集到 `rt_mem_limit` 的 50% 后就保存为磁盘分片。保存时会查看统计数据（保存了多少，保存期间来了多少新文档）并重新计算下一次使用的初始比例。例如，如果保存了 9000 万条文档，保存期间又来了 1000 万条文档，比例是 90%，那么下一次我们就知道可以收集最多 90% 的 `rt_mem_limit` 再开始刷新另一个磁盘分片。该比例自动计算，范围是 33.3% 到 95%。
* [问题 #628](https://github.com/manticoresoftware/manticoresearch/issues/628) PostgreSQL 源支持 [unpack_zlib](Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Database_connection.md#unpack_zlib)。感谢 [Dmitry Voronin](https://github.com/dimv36) 的[贡献](https://github.com/manticoresoftware/manticoresearch/pull/630)。
* [提交 6d54](https://github.com/manticoresoftware/manticoresearch/commit/6d54cf2b319b131970c29410cc21abcbdf8083b1) 支持 `indexer -v` 和 `--version`。之前可以看到 indexer 版本，但 `-v` / `--version` 不支持。
* [问题 #662](https://github.com/manticoresoftware/manticoresearch/issues/662) 通过 systemd 启动时默认无限制 mlock。
* [提交 63c8](https://github.com/manticoresoftware/manticoresearch/commit/63c8cd05cf399e705c2c14939411a92cb784735a) 协程 rwlock 的自旋锁改为操作队列。

* [提交 4113](https://github.com/manticoresoftware/manticoresearch/commit/41130ce30d1734bbf8e6b8b63c4e815caf9464f2) 增加环境变量 `MANTICORE_TRACK_RT_ERRORS`，方便调试 RT 段的损坏。
### 重大改动
* binlog 版本提升，旧版本的 binlog 不再重放，因此升级时请确保干净停止 Manticore Search：停止之前的实例后，`/var/lib/manticore/binlog/` 目录里除了 `binlog.meta` 不应有其它 binlog 文件。
* [提交 3f65](https://github.com/manticoresoftware/manticoresearch/commit/3f659f36e87d99ee262205a8eae4663a255be630) `show threads option format=all` 添加新列 `"chain"`。它显示一些任务信息票据的堆栈，对性能分析特别有用。如果你解析 `show threads` 输出，请注意新列。
* `searchd.workers` 从 3.5.0 起已废弃，现在变为弃用状态。如果你的配置还包含该项，启动时会有警告。Manticore Search 会启动，但会有提示警告。
* 如果你使用 PHP 和 PDO 访问 Manticore，需要设置 `PDO::ATTR_EMULATE_PREPARES`。
### Bug修复
* ❗[问题 #650](https://github.com/manticoresoftware/manticoresearch/issues/650) Manticore 4.0.2 比 3.6.3 慢。4.0.2 在批量插入上快于旧版本，但单条插入明显慢。此问题在 4.2.0 版本修复。
* ❗[提交 22f4](https://github.com/manticoresoftware/manticoresearch/commit/22f4141bed1411c83684ae6e0bae832071cce3df) RT 索引在高强度 REPLACE 负载下可能损坏或崩溃。
* [提交 03be](https://github.com/manticoresoftware/manticoresearch/commit/03be91e4f720a3f20bee0278dd8f0d44abd3ef0e) 修复合并分组平均值及分组 N 排序器的平均值；修复聚合合并。
* [提交 2ea5](https://github.com/manticoresoftware/manticoresearch/commit/2ea575d34efe38661f14d858f2b552456f29fe42) `indextool --check` 崩溃问题。
* [提交 7ec7](https://github.com/manticoresoftware/manticoresearch/commit/7ec76d4abf01b5fca6f10a0000fe64d5d8e5fa52) 由 UPDATE 引起的内存耗尽问题。
* [提交 658a](https://github.com/manticoresoftware/manticoresearch/commit/658a727e41f3c3fbcde9754149528f34ed76ba19) 守护进程在 INSERT 操作时可能挂起。
* [提交 46e4](https://github.com/manticoresoftware/manticoresearch/commit/46e42b9b386e31e53247ed634e158b533e28063e) 守护进程关闭时可能挂起。
* [提交 f8d7](https://github.com/manticoresoftware/manticoresearch/commit/f8d7d5177cc83d250b493b4cfc619732b29cf072) 守护进程关闭时可能崩溃。
* [提交 733a](https://github.com/manticoresoftware/manticoresearch/commit/733accf165142a1d1574689525ccca8907971158) 守护进程崩溃时可能挂起。
* [提交 f7f8](https://github.com/manticoresoftware/manticoresearch/commit/f7f8bd8c0dbb23a59a88502dcba16b0918fb2330) 守护进程在启动时尝试重新加入无效节点列表的集群时可能崩溃
* [提交 1401](https://github.com/manticoresoftware/manticoresearch/commit/140155613821ecaddcd1a3dfc4f7369b0df6aef7) 分布式索引在 RT 模式下，如果启动时无法解析其某个代理，可能会完全丢失
* [问题 #683](https://github.com/manticoresoftware/manticoresearch/issues/683) attr bit(N) engine='columnar' 失败
* [问题 #682](https://github.com/manticoresoftware/manticoresearch/issues/682) 创建表失败，但留下了目录
* [问题 #663](https://github.com/manticoresoftware/manticoresearch/issues/663) 配置失败，提示：未知键名 'attr_update_reserve'
* [问题 #632](https://github.com/manticoresoftware/manticoresearch/issues/632) Manticore 在批量查询时崩溃

* [问题 #679](https://github.com/manticoresoftware/manticoresearch/issues/679) 使用 v4.0.3 版本时批量查询再次导致崩溃

* [提交 f7f8](https://github.com/manticoresoftware/manticoresearch/commit/f7f8bd8c0dbb23a59a88502dcba16b0918fb2330) 修复了守护进程启动时尝试重新加入含无效节点列表的集群崩溃的问题
* [问题 #643](https://github.com/manticoresoftware/manticoresearch/issues/643) Manticore 4.0.2 在一批插入操作后不接受连接
* [问题 #635](https://github.com/manticoresoftware/manticoresearch/issues/635) 带有 ORDER BY JSON.field 或字符串属性的 FACET 查询可能崩溃
* [问题 #634](https://github.com/manticoresoftware/manticoresearch/issues/634) 使用 packedfactors 查询时出现 SIGSEGV 崩溃
* [提交 4165](https://github.com/manticoresoftware/manticoresearch/commits/41657f15f8fcf1e93cfad7f835bcf705d6e8349e) morphology_skip_fields 之前在创建表时不被支持
## 版本 4.0.2，2021年9月21日
### 主要新功能
- **全面支持 [Manticore Columnar Library](https://github.com/manticoresoftware/columnar)**。此前 Manticore Columnar Library 仅支持普通索引。现在支持：
  - 实时索引的 `INSERT`、`REPLACE`、`DELETE`、`OPTIMIZE`

  - 复制功能

  - `ALTER` 操作
  - `indextool --check`
- **自动索引压缩** ([问题 #478](https://github.com/manticoresoftware/manticoresearch/issues/478))。终于，您不必手动调用 OPTIMIZE，或通过定时任务或其他自动化方式。Manticore 现在会自动且默认进行。您可以通过 [optimize_cutoff](Server_settings/Setting_variables_online.md) 全局变量设置默认的压缩阈值。
- **块快照和锁系统重构**。这些更改乍看可能对外不可见，但显著改进了实时索引中许多操作的行为。简而言之，此前大多数 Manticore 数据操作严重依赖锁，现在改用了磁盘块快照。
- **实时索引的批量插入性能大幅提升**。例如在 [Hetzner 的 AX101 服务器](https://www.hetzner.com/dedicated-rootserver/ax101)（SSD，128 GB 内存，AMD Ryzen™ 9 5950X（16*2核））上，**3.6.0 版本在表 schema 为 `name text, email string, description text, age int, active bit(1)` 且默认 `rt_mem_limit`、批量大小 25000、16 个并发插入工作线程、总插入文档 1600 万时，插入速度可达 236K 文档每秒**。4.0.2 在同样的并发量/批量/文档数情况下，**插入速度提升至 357K 文档每秒**。
  <details>
  - 读取操作（如 SELECT、复制）使用快照执行

  - 仅改变内部索引结构但不修改 schema/文档的操作（如合并 RAM 段、保存磁盘块、合并磁盘块）使用只读快照执行，并最终替换现有的块

  - UPDATE 和 DELETE 针对现有块执行，但合并时写入会被暂存，随后应用于新块
  - UPDATE 为每个块按顺序获得独占锁；合并在收集快照属性阶段获得共享锁。这样同时只有一个合并或更新操作能访问该块的属性

  - 合并进入需要属性的阶段时设置特殊标志，UPDATE 完成后检查该标志，如果已设置，整个更新集存储于特殊集合。合并结束时，将更新应用至新生成的磁盘块
  - ALTER 通过独占锁运行
  - 复制作为普通读取操作运行，但在 SST 前保存属性，并禁止 SST 期间的更新
  </details>
- **[ALTER](Updating_table_schema_and_settings.md) 可添加/删除全文字段**（RT 模式）。之前只能添加/删除属性。


- 🔬 **实验性：全文扫描查询的伪分片**——支持将任何非全文搜索查询并行化。您无需手动准备分片，只需启用新的选项 [searchd.pseudo_sharding](Server_settings/Searchd.md#pseudo_sharding)，即可期待非全文搜索查询响应时间降低至最多 CPU 核数的倍数。请注意，它可能会占用所有 CPU 核心，若您兼顾吞吐量，请谨慎使用。
### 次要更改

<!-- example -->
- 通过 [APT 仓库](Installation/Debian_and_Ubuntu.md#APT-repository) 支持 Linux Mint 和 Ubuntu Hirsute Hippo
- 在某些情况下，大型索引通过 HTTP 按 ID 更新更快（依赖 ID 分布）

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
- [systemd 的自定义启动标志](Starting_the_server/Linux.md#Custom-startup-flags-using-systemd)。现在如果需要使用特定的启动标志运行 Manticore，就不必手动启动 searchd 了。

- 新函数 [LEVENSHTEIN()](Functions/String_functions.md#LEVENSHTEIN%28%29)，用于计算 Levenshtein 距离。
- 新增 [searchd 启动标志](Starting_the_server/Manually.md#searchd-command-line-options) `--replay-flags=ignore-trx-errors` 和 `--replay-flags=ignore-all-errors`，这样即使 binlog 损坏，也可以启动 searchd。
- [Issue #621](https://github.com/manticoresoftware/manticoresearch/issues/621) - 公开 RE2 的错误信息。
- 分布式索引由本地普通索引组成时，更准确的 [COUNT(DISTINCT)](Searching/Grouping.md#COUNT%28DISTINCT-field%29)。
- [FACET DISTINCT](Searching/Faceted_search.md#Faceting-without-duplicates) 用于在做分面搜索时去重。
- [精确形式修饰符](Searching/Full_text_matching/Operators.md#Exact-form-modifier) 现在不再需要 [形态学](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology)，并且支持启用 [中缀/前缀](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) 搜索的索引。
### 重大变更
- 新版本能读取旧索引，但旧版本无法读取 Manticore 4 的索引。

- 移除隐式按 id 排序。如需排序请显式指定。
- `charset_table` 的默认值从 `0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451` 改为 `non_cjk`。
- `OPTIMIZE` 会自动执行。如果不需要，请确保在配置文件的 `searchd` 部分设置 `auto_optimize=0`。
- [Issue #616](https://github.com/manticoresoftware/manticoresearch/issues/616) `ondisk_attrs_default` 已废弃并移除。
- 对贡献者：我们现在使用 Clang 编译器用于 Linux 构建，因为根据我们的测试它能构建更快的 Manticore Search 和 Manticore Columnar Library。
- 如果在搜索查询中未指定 [max_matches](Searching/Options.md#max_matches)，为提升新列存储性能，会隐式更新为最低所需值。这可能影响 [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META) 中的 `total` 指标，但不会影响实际找到文档数的 `total_found`。
### 从 Manticore 3 迁移
- 确保您已干净停止 Manticore 3：
  - `/var/lib/manticore/binlog/` 目录下不应有 binlog 文件（该目录内应仅有 `binlog.meta` 文件）

  - 否则 Manticore 4 无法重放这些 binlog 的索引将无法运行
- 新版本可读取旧索引，但旧版本无法读取 Manticore 4 的索引，请确保做备份以便轻松回滚。
- 如果运行复制集群，请确保：
  - 先干净停止所有节点
  - 然后以 `--new-cluster` 启动最后停止的节点（Linux 下运行工具 `manticore_new_cluster`）。
  - 更多细节请阅读 [重新启动集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster)。
### Bug 修复
- 修复了大量复制问题：
  - [提交 696f](https://github.com/manticoresoftware/manticoresearch/commit/696f8649535f2b5285b878c9581c0d2b1139ae09) - 修复 joiner 在 SST 时崩溃；增加 joiner 节点写入文件块时 SHA1 校验加速索引加载；增加索引加载时 joiner 节点变更索引文件轮换；当新的索引从 donor 节点替换 active 索引时，joiner 节点移除旧索引文件；donor 节点增加发送文件和块的复制日志点。
  - [提交 b296](https://github.com/manticoresoftware/manticoresearch/commit/b296c55af68314a8fc66f8b104e7301d673c3b68) - 地址错误时 JOIN CLUSTER 崩溃。
  - [提交 418b](https://github.com/manticoresoftware/manticoresearch/commit/418bf880a5e05ffbb68f311a1c23006a9df3220e) - 初始复制大索引时，加入节点可能因 `ERROR 1064 (42000): invalid GTID, (null)` 失败，donor 节点可能无响应。
  - [提交 6fd3](https://github.com/manticoresoftware/manticoresearch/commit/6fd350d28e4905211cf8a7298cbf7cdda101c83d) - 大索引可能计算出错误的哈希，导致复制失败。
  - [Issue #615](https://github.com/manticoresoftware/manticoresearch/issues/615) - 复制失败导致集群重启问题。
- [Issue #574](https://github.com/manticoresoftware/manticoresearch/issues/574) - `indextool --help` 未显示参数 `--rotate`。
- [Issue #578](https://github.com/manticoresoftware/manticoresearch/issues/578) - searchd 闲置约一天后 CPU 占用高。
- [Issue #587](https://github.com/manticoresoftware/manticoresearch/issues/587) - 立即刷新 .meta 文件。
- [Issue #617](https://github.com/manticoresoftware/manticoresearch/issues/617) - manticore.json 文件被清空。
- [Issue #618](https://github.com/manticoresoftware/manticoresearch/issues/618) - searchd --stopwait 在 root 下失败。也修复 systemctl 行为（之前 ExecStop 显示失败且无法正确等待 searchd 停止）。
- [Issue #619](https://github.com/manticoresoftware/manticoresearch/issues/619) - INSERT/REPLACE/DELETE 与 SHOW STATUS。`command_insert`、`command_replace` 等指标显示错误。
- [Issue #620](https://github.com/manticoresoftware/manticoresearch/issues/620) - 普通索引的 `charset_table` 默认值错误。
- [提交 8f75](https://github.com/manticoresoftware/manticoresearch/commit/8f7536887d4b58a5bab5647fc067b2d0482c2038) - 新的磁盘块未被 mlock。
- [Issue #607](https://github.com/manticoresoftware/manticoresearch/issues/607) - 节点无法按名称解析时 Manticore 集群节点崩溃。


- [Issue #623](https://github.com/manticoresoftware/manticoresearch/issues/623) - 更新索引的复制可能导致未定义状态。
- [提交 ca03](https://github.com/manticoresoftware/manticoresearch/commit/ca03d2280c5197e9b311f0f03f551e3b702a8130) - 索引器在索引带有json属性的普通索引源时可能挂起

- [提交 53c7](https://github.com/manticoresoftware/manticoresearch/commit/53c753054558ff3f82c7af365273d32537c5338b) - 修复了PQ索引中不等表达式过滤器的问题
- [提交 ccf9](https://github.com/manticoresoftware/manticoresearch/commit/ccf94e022ba6763d6286ab5bac767b6f6a846846) - 修复了列表查询中超过1000条匹配时的选择视窗问题。之前 `SELECT * FROM pq ORDER BY id desc LIMIT 1000 , 100 OPTION max_matches=1100` 不工作
- [提交 a048](https://github.com/manticoresoftware/manticoresearch/commit/a0483fe9bef3c45dffe96f5df4f4382aee29c072) - 发送HTTPS请求到Manticore时可能会造成类似“max packet size(8388608) exceeded”的警告
- [问题 #648](https://github.com/manticoresoftware/manticoresearch/issues/648) - Manticore 3 在更新字符串属性几次后可能会挂起

## 版本 3.6.0，2021年5月3日
**Manticore 4之前的维护版本**
### 主要新功能
- 支持用于普通索引的 [Manticore 列式库](https://github.com/manticoresoftware/columnar/)。普通索引新增设置 [columnar_attrs](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#columnar_attrs)
- 支持 [乌克兰语词形还原器](https://github.com/manticoresoftware/lemmatizer-uk)
- 全面修订了直方图。在构建索引时，Manticore 同时为其中每个字段构建直方图，随后用于更快的过滤。在3.6.0中算法已全面重写，如果您有大量数据并执行很多过滤操作，可以获得更高性能。
### 小改动
- 工具 `manticore_new_cluster [--force]`，用于通过systemd重启复制集群
- `indexer --merge` 支持 [--drop-src](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Merging_tables.md#Merging-tables)

- 新模式 [new mode](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) `blend_mode='trim_all'`
- 新增对使用反引号转义JSON路径的支持 [support for escaping JSON path](Searching/Full_text_matching/Escaping.md#Escaping-json-node-names-in-SQL)
- [indextool --check](Miscellaneous_tools.md#indextool) 可以在RT模式下工作

- SELECT/UPDATE支持 [FORCE/IGNORE INDEX(id)](Data_creation_and_modification/Updating_documents/UPDATE.md#Updates-via-SQL)
- 合并后的磁盘块的chunk id现在唯一
- 新命令 [indextool --check-disk-chunk CHUNK_NAME](Miscellaneous_tools.md#indextool)
### 优化
- [更快的JSON解析](https://github.com/manticoresoftware/manticoresearch/commit/0c25fc1788e3f77f60cb7fb8f1623d8466bba606)，我们的测试显示在诸如 `WHERE json.a = 1` 的查询中延迟降低3-4%
- 非公开命令 `DEBUG SPLIT`，作为自动分片/重新平衡的前置条件
### Bug修复
- [问题 #584](https://github.com/manticoresoftware/manticoresearch/issues/524) - FACET结果不准确且不稳定
- [问题 #506](https://github.com/manticoresoftware/manticoresearch/issues/506) - 使用MATCH时异常行为：受此问题影响的用户需要重建索引，因为问题出在索引构建阶段
- [问题 #387](https://github.com/manticoresoftware/manticoresearch/issues/387) - 使用SNIPPET()函数运行查询时间歇性核心转储
- 针对复杂查询的堆栈优化：
  - [问题 #469](https://github.com/manticoresoftware/manticoresearch/issues/469) - SELECT导致崩溃转储
  - [e8420cc7](https://github.com/manticoresoftware/manticoresearch/tree/e8420cc789149c84731be0874be916e33b263bd6) - 过滤器树的堆栈大小检测
- [问题 #461](https://github.com/manticoresoftware/manticoresearch/issues/461) - 使用IN条件更新时未正确生效
- [问题 #464](https://github.com/manticoresoftware/manticoresearch/issues/464) - CALL PQ之后立即执行SHOW STATUS返回 - [问题 #481](https://github.com/manticoresoftware/manticoresearch/issues/481) - 修复静态二进制构建
- [问题 #502](https://github.com/manticoresoftware/manticoresearch/issues/502) - 多查询中的bug
- [问题 #514](https://github.com/manticoresoftware/manticoresearch/issues/514) - 使用'create table'时无法使用不寻常的列名
- [提交 d1db](https://github.com/manticoresoftware/manticoresearch/commit/d1dbe771288328c963ca8512f564c6b7cf4f466c) - 守护进程在重放带有字符串属性更新的binlog时崩溃；将binlog版本设置为10
- [提交 775d](https://github.com/manticoresoftware/manticoresearch/commit/775d0555562f121911d3c30085947e743fadac2d)  - 修复表达式堆栈帧检测运行时错误（测试207）
- [提交 4795](https://github.com/manticoresoftware/manticoresearch/commit/4795dc49194c7745761bb5006ecb266e8a9224fa)  - 当存储查询为空时，渗透索引过滤器和标签为空（测试369）
- [提交 c3f0](https://github.com/manticoresoftware/manticoresearch/commit/c3f0bf4dca0e4eae6ea899dd566878df57eba085)  - 网络延迟高且错误率高时（不同数据中心复制）复制SST流程中断；更新复制命令版本为1.03
- [提交 ba2d](https://github.com/manticoresoftware/manticoresearch/commit/ba2d6619b8107f3fb177741bbdff3709bac248cd)  - 加入集群后写操作时joiner锁集群（测试385）
- [提交 de4d](https://github.com/manticoresoftware/manticoresearch/commit/de4dcb9ff6c316bc2a9ef6c62eb64769f04ee103)  - 支持带exact修饰符的通配符匹配（测试321）
- [提交 6524](https://github.com/manticoresoftware/manticoresearch/commit/6524fc6af032a0932bd42c37874a619e1fb7f075)  - docid检查点与docstore
- [提交 f4ab](https://github.com/manticoresoftware/manticoresearch/commit/f4ab83c291e65a5873d271c67b5491f629544f1d)  - 解析无效xml时索引器行为不一致
- [提交 7b72](https://github.com/manticoresoftware/manticoresearch/commit/7b727e22fe01b1a180d4a8d3a3fea92c18eac707)  - 存储的带NOTNEAR的渗透查询会永远运行（测试349）
- [Commit 812d](https://github.com/manticoresoftware/manticoresearch/commit/812dab745f0c972ecde5d5b3d4a313cd6e1e143e)  - 以通配符开头的短语权重错误
- [Commit 1771](https://github.com/manticoresoftware/manticoresearch/commit/1771afc669c75670b3511bf02ab9bfc8982b94d2)  - 带通配符的渗透查询在匹配时生成无负载的词项导致交织命中并破坏匹配（测试417）
- [Commit aa0d](https://github.com/manticoresoftware/manticoresearch/commit/aa0d8c2bcb41428d41dbfb66e42a872a79a4ee4b)  - 修正并行查询情况下'total'的计算
- [Commit 18d8](https://github.com/manticoresoftware/manticoresearch/commit/18d81b3cfbb930b7361dae645fe13283d4fbf6ef)  - Windows上守护进程多会话并发导致崩溃
- [Commit 8443](https://github.com/manticoresoftware/manticoresearch/commit/84432f23d6fbf7a7d0b67c8c9412eb5933b03596)  - 某些索引设置无法复制
- [Commit 9341](https://github.com/manticoresoftware/manticoresearch/commit/93411fe6af4246ca4acce758944d513a1660b92b)  - 高速添加新事件时netloop有时冻结，因为原子“kick”事件被多事件同时处理一次，导致实际动作丢失
状态表示查询状态，不是服务器状态
- [Commit d805](https://github.com/manticoresoftware/manticoresearch/commit/d805fc125244ef08d14fb3d1826e2c356862d539)  - 新刷新磁盘块可能在提交时丢失

- [Commit 63cb](https://github.com/manticoresoftware/manticoresearch/commit/63cbf008d50979cdf7062664fb02a672a3bb1131)  - profiler中'net_read'不准确
- [Commit f537](https://github.com/manticoresoftware/manticoresearch/commit/f5379bb27234b25919966c50ccb6f469ead646e2)  - 阿拉伯语（从右向左文本）渗透问题
- [Commit 49ee](https://github.com/manticoresoftware/manticoresearch/commit/49eeb4202d4ce4875c07c35087d7108c1e177328)  - 重复列名时id未正确选取
- [Commit refa](https://github.com/manticoresoftware/manticoresearch/commit/4b7df262bbfec9b9e95a44e72ad326ac8a786c49) 修复网络事件罕见情况下崩溃
- [e8420cc7](https://github.com/manticoresoftware/manticoresearch/tree/e8420cc789149c84731be0874be916e33b263bd6) 修复 `indextool --dumpheader`
- [Commit ff71](https://github.com/manticoresoftware/manticoresearch/commit/ff7163532ed39042c0d3c3c2b902ff41f94d7f0c) - 带存储字段的 TRUNCATE WITH RECONFIGURE 工作异常

### 破坏性变更：
- 新的binlog格式：升级前需要干净停止Manticore
- 索引格式轻微变更：新版本可正常读取现有索引，但如果从3.6.0降级到旧版本，则新索引无法读取

- 复制格式变更：不要从旧版本复制到3.6.0或反向复制，所有节点应同时切换到新版本

- `reverse_scan` 已弃用。从3.6.0起请确保查询中不使用此选项，否则会失败
- 自本版本起，不再提供RHEL6、Debian Jessie和Ubuntu Trusty的构建。如果业务关键需要支持，请[联系我们](https://manticoresearch.com/contact-us/)
### 弃用
- 不再隐式按id排序。如果依赖此行为请相应更新查询

- 搜索选项 `reverse_scan` 已弃用
## 版本 3.5.4，2020年12月10日
### 新功能

- 新的Python、Javascript和Java客户端现已普遍可用，且在本手册中有良好文档说明。
- 实时索引磁盘块自动丢弃。此优化允许在[优化](Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE)实时索引时，自动丢弃明显不再需要的磁盘块（所有文档已删除）。之前仍需合并，现在可即时丢弃该块。[cutoff](Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks)选项被忽略，即使无需合并，过时的磁盘块也会被移除。对维护索引保留期和删除旧文档有用，使压缩此类索引更快。
- 作为SELECT选项支持[独立NOT](Searching/Options.md#not_terms_only_allowed)

### 小改动
- [Issue #453](https://github.com/manticoresoftware/manticoresearch/issues/453) 新选项[indexer.ignore_non_plain=1](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#ignore_non_plain)适用于运行`indexer --all`且配置中非纯索引的情况。若无此选项会出现警告和相应退出代码。
- [SHOW PLAN ... OPTION format=dot](Node_info_and_management/Profiling/Query_plan.md#Dot-format-for-SHOW-PLAN) 和 [EXPLAIN QUERY ... OPTION format=dot](Searching/Full_text_matching/Profiling.md#Profiling-without-running-a-query)支持全文查询计划执行可视化，有助于理解复杂查询。
### 弃用
- `indexer --verbose` 被弃用，因未对indexer输出增加内容
- 转储watchdog回溯信号改用`USR2`代替`USR1`
### 修复
- [Issue #423](https://github.com/manticoresoftware/manticoresearch/issues/423) 斯拉夫字母句点调用片段保留模式不高亮
- [Issue #435](https://github.com/manticoresoftware/manticoresearch/issues/435) RTINDEX - 使用GROUP N BY表达式导致致命崩溃
- [Commit 2b3b](https://github.com/manticoresoftware/manticoresearch/commit/2b3b62bd0ce2b54544733d524a169e81f71b5388) 集群中searchd状态显示段错误


- [Commit 9dd2](https://github.com/manticoresoftware/manticoresearch/commit/9dd25c193ed2a53750a0e0e68566fd2b833fcb0c) 'SHOW INDEX index.N SETTINGS' 没有显示大于9的块

- [Issue #389](https://github.com/manticoresoftware/manticoresearch/issues/389) 导致Manticore崩溃的Bug

- [Commit fba1](https://github.com/manticoresoftware/manticoresearch/commit/fba166178ff35c73a0c70d131cf3aca4a635e961) 转换器创建了损坏的索引
- [Commit eecd](https://github.com/manticoresoftware/manticoresearch/commit/eecd61d861c742471175be19a5e5455034b5db5f) stopword_step=0 与 CALL SNIPPETS()
- [Commit ea68](https://github.com/manticoresoftware/manticoresearch/commit/ea6850e449601199190f38a387c04b8a7b7e90f6) 本地索引上的低 max_matches 下 count distinct 返回 0
- [Commit 362f](https://github.com/manticoresoftware/manticoresearch/commit/362f27db3421f09a41c6cce04b5baace00fa4c4d) 使用聚合时存储的文本不会在命中结果中返回
## 版本 3.5.2, 2020 年 10 月 1 日
### 新特性

* OPTIMIZE 将磁盘块减少到特定数量的块（默认值是 `2* 核心数量`）而不是单一块。最优块数可以通过 [cutoff](Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks) 选项控制。

* NOT 操作符现在可以独立使用。默认情况下禁用，因为意外的单个 NOT 查询可能导致速度变慢。通过设置新的 searchd 指令 [not_terms_only_allowed](Server_settings/Searchd.md#not_terms_only_allowed) 为 `0` 可启用。
* 新设置 [max_threads_per_query](Server_settings/Searchd.md#max_threads_per_query) 设置查询可以使用的线程数。如果未设置该指令，查询线程数最多可达到 [threads](Server_settings/Searchd.md#threads) 的值。
每个 `SELECT` 查询可通过 [OPTION threads=N](Searching/Options.md#threads) 来限制线程数，覆盖全局 `max_threads_per_query`。
* Percolate 索引现在可以通过 [IMPORT TABLE](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Importing_table.md) 导入。

* HTTP API `/search` 新增基本支持基于新查询节点 `aggs` 的 [分面搜索](Searching/Faceted_search.md#HTTP-JSON)/[分组](Searching/Grouping.md)。

### 小改动

* 如果未声明复制监听指令，引擎将尝试使用定义的 'sphinx' 端口之后的端口，最多尝试 200 个端口。

* `listen=...:sphinx` 需要明确设置以支持 SphinxSE 连接或 SphinxAPI 客户端。

* [SHOW INDEX STATUS](Node_info_and_management/Table_settings_and_status/SHOW_TABLE_STATUS.md) 输出了新指标：`killed_documents`，`killed_rate`，`disk_mapped_doclists`，`disk_mapped_cached_doclists`，`disk_mapped_hitlists` 和 `disk_mapped_cached_hitlists`。

* SQL 命令 `status` 现在输出 `Queue\Threads` 和 `Tasks\Threads`。

### 弃用：
* `dist_threads` 已完全弃用，若指令仍被使用，searchd 会记录警告。
### Docker
官方 Docker 镜像现在基于 Ubuntu 20.04 LTS
### 打包
除了常规的 `manticore` 包外，您还可以按组件安装 Manticore Search：

- `manticore-server-core` - 提供 `searchd`，man 页面，日志目录，API 及 galera 模块。此包将同时安装依赖项 `manticore-common`。

- `manticore-server` - 提供核心自动化脚本（init.d，systemd）和 `manticore_new_cluster` 封装器。此包将同时安装依赖项 `manticore-server-core`。
- `manticore-common` - 提供配置文件，停用词，通用文档和骨架文件夹（datadir，modules，等）。
- `manticore-tools` - 提供辅助工具（`indexer`，`indextool` 等），其 man 页面和示例。此包将同时安装依赖项 `manticore-common`。
- `manticore-icudata` (RPM) 或 `manticore-icudata-65l` (DEB) - 提供 ICU 数据文件用于 ICU 形态学功能。
- `manticore-devel` (RPM) 或 `manticore-dev` (DEB) - 提供 UDF 的开发头文件。
### Bug 修复
1. [Commit 2a47](https://github.com/manticoresoftware/manticoresearch/commit/2a474dc1a26e8b0f8aaaae95669caf2f1d4b7746) 不同块的 RT 索引中 grouper 导致守护进程崩溃
2. [Commit 57a1](https://github.com/manticoresoftware/manticoresearch/commit/57a19e5ad5663ef0ca7436595218fb1221d28c8e) 空远程文档的快速路径
3. [Commit 07dd](https://github.com/manticoresoftware/manticoresearch/commit/07dd3f313c63fb82c22092f9907ef24e3475250e) 表达式栈帧检测运行时
4. [Commit 08ae](https://github.com/manticoresoftware/manticoresearch/commit/08ae357cf1012bc8e2da54c20b205b592efda3d4) Percolate 索引中匹配超过 32 个字段
5. [Commit 16b9](https://github.com/manticoresoftware/manticoresearch/commit/16b9390fd4cdb07a77ac4497adb935573a1710e5) 复制监听端口范围
6. [Commit 5fa6](https://github.com/manticoresoftware/manticoresearch/commit/5fa671affeacb6441a59c8a88479bfd423df7c81) 在 pq 上显示创建表
7. [Commit 54d1](https://github.com/manticoresoftware/manticoresearch/commit/54d133b6449105a9fb0168db3f1fbb05fb5aa1f6) HTTPS 端口行为
8. [Commit fdbb](https://github.com/manticoresoftware/manticoresearch/commit/fdbbe5245cc296cc5c1ae3ae2fb9cb08fb66a248) 替换时文档存储行混合问题
9. [Commit afb5](https://github.com/manticoresoftware/manticoresearch/commit/afb53f648ef4e64fa1776c58e66f6e716ac730ad) 将 TFO 不可用消息级别切换为 'info'
10. [Commit 59d9](https://github.com/manticoresoftware/manticoresearch/commit/59d94cefc6e88af360b8046726a16b7eaa9f0b71) strcmp 非法使用导致崩溃
11. [Commit 04af](https://github.com/manticoresoftware/manticoresearch/commit/04af0349c1e784c2230d204592d6272b440bb375) 将带有系统（停用词）文件的索引添加至集群
12. [Commit 5014](https://github.com/manticoresoftware/manticoresearch/commit/50148b4eae9f3a924082005cf5d2f9c8339af7f5) 合并带有大型字典的索引；大磁盘块的 RT 优化
13. [Commit a2ad](https://github.com/manticoresoftware/manticoresearch/commit/a2adf1582843f63d4dcc1dab8f7598e0917094a2) Indextool 可以导出当前版本的元数据

14. [Commit 69f6](https://github.com/manticoresoftware/manticoresearch/commit/69f6d5f77c64b97e3f395aea8c555b11c77fb732) GROUP N 中分组顺序的问题

15. [Commit 24d5](https://github.com/manticoresoftware/manticoresearch/commit/24d5d80f508576794b751d70f9f9b524564e3c14) 握手后显式刷写 SphinxSE
16. [Commit 31c4](https://github.com/manticoresoftware/manticoresearch/commit/31c4d78ac03ee85d9a02b86075448a04ac80f9e2) 避免在不必要时复制巨大的描述
17. [Commit 2959](https://github.com/manticoresoftware/manticoresearch/commit/2959e2caa8610f98fcdf2e5a82160f58687f4a27) show threads 中的负时间
18. [Commit f0b3](https://github.com/manticoresoftware/manticoresearch/commit/f0b357102c70f3ac1d7da260c60b67322940e647) 令牌过滤插件与零位置增量
19. [Commit a49e](https://github.com/manticoresoftware/manticoresearch/commit/a49e5bc13ea763e6f01dd5993e891858a84d1846) 多重命中时将 'FAIL' 改为 'WARNING'
## 版本 3.5.0，2020年7月22日
### 主要新功能：

* 这个版本发布耗时较长，因为我们一直在努力将多任务模式从线程改为**协程**。它使配置更简单，查询并行化更直接：Manticore 只使用给定数量的线程（见新设置 [threads](Server_settings/Searchd.md#threads)），新模式确保以最优方式完成。

* 在[高亮显示](Searching/Highlighting.md#Highlighting-options)方面的变化：

  - 支持多个字段的任何高亮显示（`highlight({},'field1, field2'` 或 JSON 查询中的 `highlight`）现在默认按字段应用限制。

  - 支持纯文本的任何高亮显示（`highlight({}, string_attr)` 或 `snippet()`）现在对整个文档应用限制。
  - [每字段限制](Searching/Highlighting.md#limits_per_field) 可以通过 `limits_per_field=0` 选项切换为全局限制（默认为 `1`）。
  - [allow_empty](Searching/Highlighting.md#allow_empty) 在通过 HTTP JSON 进行高亮显示时默认值为 `0`。
* 现在同一端口[可用于](Server_settings/Searchd.md#listen) http、https 和二进制 API（以接受来自远程 Manticore 实例的连接）。通过 mysql 协议连接仍需要 `listen = *:mysql`。Manticore 现在自动检测尝试连接的客户端类型，MySQL 除外（协议限制导致）。
* 在实时模式下，一个字段现在可以同时是[文本和字符串属性](Creating_a_table/Data_types.md#String) - 详见 [GitHub issue #331](https://github.com/manticoresoftware/manticoresearch/issues/331)。
  在[普通模式](Read_this_first.md#Real-time-mode-vs-plain-mode)下称为 `sql_field_string`。现在它也可用于实时索引的[RT模式](Read_this_first.md#Real-time-mode-vs-plain-mode)。可以参照示例使用：
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
### 次要变更
* 现在可以[高亮显示字符串属性](Searching/Highlighting.md#Highlighting-via-SQL)。
* SQL 接口支持 SSL 和压缩。
* 支持 mysql 客户端 [`status`](Node_info_and_management/Node_status.md#STATUS) 命令。
* [复制](Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Setting-up-replication) 现在可以复制外部文件（停用词、例外词等）。
* 过滤器操作符 [`in`](Searching/Filters.md#Set-filters) 现在通过 HTTP JSON 接口可用。
* HTTP JSON 中支持 [`expressions`](Searching/Expressions.md#expressions)。
* 现在可以在 RT 模式下[动态更改 `rt_mem_limit`](https://github.com/manticoresoftware/manticoresearch/issues/344)，即执行 `ALTER ... rt_mem_limit=<new value>`。

* 现在可以使用[独立的 CJK 字符集表](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology)：`chinese`、`japanese` 和 `korean`。
* [thread_stack](Server_settings/Searchd.md#thread_stack) 现在限制最大线程栈，而非初始线程栈。
* 改进了 `SHOW THREADS` 输出。
* 在 `SHOW THREADS` 中显示长时间 `CALL PQ` 的进度。
* cpustat、iostat、coredump 可以通过 [SET](Server_settings/Setting_variables_online.md#SET) 在运行时更改。

* 实现了 `SET [GLOBAL] wait_timeout=NUM`。
### 重大变更：
* **索引格式发生了变化。** 3.5.0 版本构建的索引无法被低于 3.5.0 的 Manticore 加载，但 3.5.0 能理解旧格式。
* [`INSERT INTO PQ VALUES()`](Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md)（即不提供列清单）之前只接受 `(query, tags)`。现已更改为 `(id,query,tags,filters)`。如果希望自动生成 id，可将其设为 0。
* [`allow_empty=0`](Searching/Highlighting.md#allow_empty) 现为通过 HTTP JSON 高亮显示的新默认值。
* `CREATE TABLE`/`ALTER TABLE` 中外部文件（停用词、例外词等）只允许使用绝对路径。
### 弃用：
* `ram_chunks_count` 在 `SHOW INDEX STATUS` 中已重命名为 `ram_chunk_segments_count`。
* `workers` 已废弃。现在仅有单一工作模式。
* `dist_threads` 已废弃。所有查询均尽可能并行（受 `threads` 和 `jobs_queue_size` 限制）。
* `max_children` 已废弃。请使用 [threads](Server_settings/Searchd.md#threads) 设置 Manticore 使用的线程数（默认为 CPU 核心数）。
* `queue_max_length` 已废弃。如确有需要，请使用 [jobs_queue_size](Server_settings/Searchd.md#jobs_queue_size) 调整内部作业队列大小（默认无限制）。
* 所有 `/json/*` 端点现可不带 `/json/` 访问，如 `/search`、`/insert`、`/delete`、`/pq` 等。
* `field` 表示“全文字段”已在 `describe` 中重命名为“text”。
  <!-- more -->
  **3.4.2:**
  ```sql
  mysql> describe t;
  +-------+--------+----------------+
  | 字段 | 类型   | 属性         |
  +-------+--------+----------------+
  | id    | bigint |                |
  | f     | field  | indexed stored |
  +-------+--------+----------------+
  ```
  **3.5.0**:
  ```sql
  mysql> describe t;
  +-------+--------+----------------+
  | 字段   | 类型   | 属性         |
  +-------+--------+----------------+
  | id    | bigint |                |


  | f     | text   | indexed stored |

  +-------+--------+----------------+
  ```

  <!-- \more -->
* 西里尔字母 `и` 在 `non_cjk` charset_table（默认设置）中不会映射为 `i`，因为这对俄语词干提取器和词形还原器影响过大。
* `read_timeout`。请改用同时控制读写的 [network_timeout](Server_settings/Searchd.md#network_timeout)。
### 软件包
* Ubuntu Focal 20.04 官方软件包
* deb 软件包名称从 `manticore-bin` 更改为 `manticore`
### Bug 修复：
1. [Issue #351](https://github.com/manticoresoftware/manticoresearch/issues/351) searchd 内存泄漏
2. [Commit ceab](https://github.com/manticoresoftware/manticoresearch/commit/ceabe44f) 片段中的微小越界读取
3. [Commit 1c3e](https://github.com/manticoresoftware/manticoresearch/commit/1c3e84a3) 导致崩溃查询的危险本地变量写入
4. [Commit 26e0](https://github.com/manticoresoftware/manticoresearch/commit/26e094ab) 测试 226 中 sorter 的轻微内存泄漏
5. [Commit d2c7](https://github.com/manticoresoftware/manticoresearch/commit/d2c7f86a) 测试 226 中严重内存泄漏
6. [Commit 0dd8](https://github.com/manticoresoftware/manticoresearch/commit/0dd80122) 集群显示节点同步，但 `count(*)` 显示不同数字
7. [Commit f1c1](https://github.com/manticoresoftware/manticoresearch/commit/f1c1ac3f) 外观优化：日志中重复且有时丢失的警告消息
8. [Commit f1c1](https://github.com/manticoresoftware/manticoresearch/commit/f1c1ac3f) 外观优化：日志中的 (null) 索引名称
9. [Commit 359d](https://github.com/manticoresoftware/manticoresearch/commit/359dbd30) 无法检索超过 7000 万条结果
10. [Commit 19f3](https://github.com/manticoresoftware/manticoresearch/commit/19f328ee) 无法使用无列语法插入 PQ 规则
11. [Commit bf68](https://github.com/manticoresoftware/manticoresearch/commit/bf685d5d) 向集群索引插入文档时误导性错误信息
12. [Commit 2cf1](https://github.com/manticoresoftware/manticoresearch/commit/2cf18c83) `/json/replace` 和 `json/update` 返回指数形式的 id
13. [Issue #324](https://github.com/manticoresoftware/manticoresearch/issues/324) 在同一查询中更新json标量属性和 mva
14. [Commit d384](https://github.com/manticoresoftware/manticoresearch/commit/d38409eb) `hitless_words` 在 RT 模式下失效
15. [Commit 5813](https://github.com/manticoresoftware/manticoresearch/commit/5813d639) RT 模式下禁止使用 `ALTER RECONFIGURE`
16. [Commit 5813](https://github.com/manticoresoftware/manticoresearch/commit/5813d639) searchd 重启后 `rt_mem_limit` 重置为 128M
17. highlight() 有时会挂起
18. [Commit 7cd8](https://github.com/manticoresoftware/manticoresearch/commit/7cd878f4) RT 模式下无法使用 U+code
19. [Commit 2b21](https://github.com/manticoresoftware/manticoresearch/commit/2b213de4) RT 模式下无法在 wordforms 中使用通配符
20. [Commit e9d0](https://github.com/manticoresoftware/manticoresearch/commit/e9d07e68) 修复 `SHOW CREATE TABLE` 与多个词形文件不兼容的问题
21. [Commit fc90](https://github.com/manticoresoftware/manticoresearch/commit/fc90a84f) 不带 "query" 的 JSON 查询导致 searchd 崩溃
22. Manticore [官方 Docker](https://hub.docker.com/r/manticoresearch/manticore) 无法从 mysql 8 索引数据
23. [Commit 23e0](https://github.com/manticoresoftware/manticoresearch/commit/23e05d32) HTTP /json/insert 需指定 id
24. [Commit bd67](https://github.com/manticoresoftware/manticoresearch/commit/bd679af0) `SHOW CREATE TABLE` 不适用于 PQ
25. [Commit bd67](https://github.com/manticoresoftware/manticoresearch/commit/bd679af0) `CREATE TABLE LIKE` 对 PQ 支持不完整
26. [Commit 5eac](https://github.com/manticoresoftware/manticoresearch/commit/5eacf28f) show index status 中设置项末尾的换行符问题
27. [Commit cb15](https://github.com/manticoresoftware/manticoresearch/commit/cb153228) HTTP JSON 响应中 "highlight" 的标题为空
28. [Issue #318](https://github.com/manticoresoftware/manticoresearch/issues/318) `CREATE TABLE LIKE` 中间错误
29. [Commit 9040](https://github.com/manticoresoftware/manticoresearch/commit/9040d22c) RT 在负载下崩溃
30. [cd512c7d ](https://github.com/manticoresoftware/manticoresearch/commit/cd512c7d ) RT 磁盘块崩溃时丢失崩溃日志
31. [Issue #323](https://github.com/manticoresoftware/manticoresearch/issues/323) 导入表失败且关闭连接
32. [Commit 6275](https://github.com/manticoresoftware/manticoresearch/commit/6275316a) `ALTER reconfigure` 损坏 PQ 索引
33. [Commit 9c1d](https://github.com/manticoresoftware/manticoresearch/commit/9c1d221e) 修改索引类型后 searchd 重新加载问题
34. [Commit 71e2](https://github.com/manticoresoftware/manticoresearch/commit/71e2b5bb) 守护进程导入缺失文件表时崩溃
35. [Issue #322](https://github.com/manticoresoftware/manticoresearch/issues/322) 多索引、group by 和 ranker=none 导致崩溃
36. [Commit c3f5](https://github.com/manticoresoftware/manticoresearch/commit/c3f58490) `HIGHLIGHT()` 无法对字符串属性高亮
37. [Issue #320](https://github.com/manticoresoftware/manticoresearch/issues/320) `FACET` 不能对字符串属性排序
38. [Commit 4f1a](https://github.com/manticoresoftware/manticoresearch/commit/4f1a1f25) 缺失数据目录时出错
39. [Commit 04f4](https://github.com/manticoresoftware/manticoresearch/commit/04f4ddd4) RT 模式下不支持 access_*
40. [提交 1c06](https://github.com/manticoresoftware/manticoresearch/commit/1c0616a2) 字符串中的错误 JSON 对象：1. `CALL PQ` 当 json 超过某个值时返回 "字符串中的错误 JSON 对象: 1"。
41. [提交 32f9](https://github.com/manticoresoftware/manticoresearch/commit/32f943d6) RT 模式不一致。在某些情况下，我无法删除索引，因为它未知，也无法创建索引，因为目录不为空。
42. [问题 #319](https://github.com/manticoresoftware/manticoresearch/issues/319) 选择时崩溃
43. [提交 22a2](https://github.com/manticoresoftware/manticoresearch/commit/22a28dd7) `max_xmlpipe2_field` = 2M 在 2M 字段上返回警告
44. [问题 #342](https://github.com/manticoresoftware/manticoresearch/issues/342) 查询条件执行错误
45. [提交 dd8d](https://github.com/manticoresoftware/manticoresearch/commit/dd8dcab2) 简单的两个词搜索找到了只包含一个词的文档
46. [提交 9091](https://github.com/manticoresoftware/manticoresearch/commit/90919e62) 在 PQ 中无法匹配键名带大写字母的 json
47. [提交 56da](https://github.com/manticoresoftware/manticoresearch/commit/56da086a) 索引器在 csv+docstore 上崩溃
48. [问题 #363](https://github.com/manticoresoftware/manticoresearch/issues/363) 在 centos 7 中使用 `[null]` 作为 json 属性会导致插入数据损坏
49. 重大 [问题 #345](https://github.com/manticoresoftware/manticoresearch/issues/345) 记录未插入，count() 是随机的，"replace into" 返回 OK
50. max_query_time 使 SELECT 查询变得过于缓慢


51. [问题 #352](https://github.com/manticoresoftware/manticoresearch/issues/352) Mac OS 上 Master-agent 通信失败
52. [问题 #328](https://github.com/manticoresoftware/manticoresearch/issues/328) 使用 Connector.Net/Mysql 8.0.19 连接 Manticore 时出错
53. [提交 daa7](https://github.com/manticoresoftware/manticoresearch/commit/daa760d2fe83668038318f0f384ceefc0d075e18) 修复 \0 的转义并优化性能

54. [提交 9bc5](https://github.com/manticoresoftware/manticoresearch/commit/9bc5c01a493db993b90ff6c1ad51aeae028f629c) 修复 count distinct 与 json 问题
55. [提交 4f89](https://github.com/manticoresoftware/manticoresearch/commit/4f89a965a3af0db548ae9d1763731ebaebe21f5a) 修复在其他节点丢弃表失败
56. [提交 952a](https://github.com/manticoresoftware/manticoresearch/commit/952af5a5931fd72b87edc169588777778b365231) 修复紧密运行 call pq 时崩溃
## 版本 3.4.2，2020 年 4 月 10 日
### 关键错误修复
* [提交 2ffe](https://github.com/manticoresoftware/manticoresearch/commit/2ffe2d261793f9681265bc4abe38d695e72c121e) 修复旧版本 RT 索引无法索引数据的问题

## 版本 3.4.0，2020 年 3 月 26 日
### 重大变更
* 服务器支持两种模式：rt 模式和普通模式
   *   rt 模式需要 data_dir，并且配置中不允许定义索引
   *   普通模式中索引在配置中定义；不允许使用 data_dir
* 复制功能仅在 rt 模式下可用

### 次要变更
* charset_table 默认使用 non_cjk 别名
* rt 模式下全文字段默认被索引和存储

* rt 模式下全文字段名称从 'field' 改为 'text'
* ALTER RTINDEX 更名为 ALTER TABLE
* TRUNCATE RTINDEX 更名为 TRUNCATE TABLE
### 新功能
* 仅存储字段

* SHOW CREATE TABLE，IMPORT TABLE
### 改进
* 更快的无锁 PQ
* /sql 可以在 mode=raw 模式下执行任意类型的 SQL 语句
* mysql41 协议增加 mysql 别名
* 默认在 data_dir 中生成 state.sql
### 错误修复
* [提交 a533](https://github.com/manticoresoftware/manticoresearch/commit/a5333644e8e0c69746a9f24da8221fc965478e4a) 修复 highlight() 中错误字段语法导致的崩溃
* [提交 7fbb](https://github.com/manticoresoftware/manticoresearch/commit/7fbb9f2e618096289cba7f8b834530b46aa94055) 修复复制带 docstore 的 RT 索引时服务器崩溃
* [提交 24a0](https://github.com/manticoresoftware/manticoresearch/commit/24a04687eb43b4f4719c7c4a5fe3c19099240467) 修复对带有 infix 或 prefix 选项并且未启用存储字段的索引进行高亮时崩溃
* [提交 3465](https://github.com/manticoresoftware/manticoresearch/commit/3465c1ce9c860c09f6ccc5b83a63e3c68c241d8d) 修复空索引导致的虚假空 docstore 和 dock-id 查找错误
* [提交 a707](https://github.com/manticoresoftware/manticoresearch/commit/b8601b4144b9092be43a7caa7cd6b19cb37e14c7) 修复 #314 带尾部分号的 SQL insert 命令
* [提交 9562](https://github.com/manticoresoftware/manticoresearch/commit/95628c9b9b8f16fe1bd18599a84e9673575adf1a) 移除查询词不匹配的警告

* [提交 b860](https://github.com/manticoresoftware/manticoresearch/commit/b8601b4144b9092be43a7caa7cd6b19cb37e14c7) 修复通过 ICU 分词的片段查询
* [提交 5275](https://github.com/manticoresoftware/manticoresearch/commit/5275516ca4631c584e6e0127fa161a18d2658fd0) 修复 docstore 块缓存中查找/添加的竞态条件
* [提交 f06e](https://github.com/manticoresoftware/manticoresearch/commit/f06ef97af5c7390427e82525c150535290e42c3e) 修复 docstore 内存泄漏
* [提交 a725](https://github.com/manticoresoftware/manticoresearch/commit/a7258ba8340e246c0d8aaeb4a7a74129eba807eb) 修复 #316 INSERT 时 LAST_INSERT_ID 返回空
* [提交 1ebd](https://github.com/manticoresoftware/manticoresearch/commit/1ebd5bf8d74113bcfa138ce63ce1196696690a4215) 修复 #317 json/update HTTP 端点支持 MVA 的数组和 JSON 属性的对象
* [提交 e426](https://github.com/manticoresoftware/manticoresearch/commit/e426950a536382a142e2a42ed207c341149ff685) 修复索引器未显式 id 时转储 rt 的问题
## 版本 3.3.0，2020 年 2 月 4 日

### 新功能
* 并行实时索引搜索
* EXPLAIN QUERY 命令
* 无索引定义的配置文件（alpha 版本）
* CREATE/DROP TABLE 命令（alpha 版本）
* indexer --print-rt - 可以从源读取并打印 Real-Time 索引的 INSERT 语句
### 改进


* 更新到 Snowball 2.0 词干器
* SHOW INDEX STATUS 支持 LIKE 过滤器
* 为高 max_matches 情况下的内存使用做了改进
* SHOW INDEX STATUS 为 RT 索引添加了 ram_chunks_count
* 无锁优先队列
* 将 LimitNOFILE 更改为 65536
### Bug修复
* [提交 9c33](https://github.com/manticoresoftware/manticoresearch/commit/9c33aab8c1fd9059c8644d23d5f58f03010043c6) 添加了索引模式重复属性检查 #293
* [提交 a008](https://github.com/manticoresoftware/manticoresearch/commit/a0085f9477e191da9e0e835b91583fb9cfde698e) 修复 hitless terms 崩溃
* [提交 6895](https://github.com/manticoresoftware/manticoresearch/commit/68953740af8dca1833026d0f448c71416ed3044a) 修复 ATTACH 后松散的 docstore
* [提交 d6f6](https://github.com/manticoresoftware/manticoresearch/commit/d6f696edea53ea2b9a68251e4054e4816082c285) 修复分布式设置中的 docstore 问题
* [提交 bce2](https://github.com/manticoresoftware/manticoresearch/commit/bce2b7ec883d208f31094ee42a5b6f01d18a998f) 在 sorter 中用 OpenHash 替换 FixedHash
* [提交 e0ba](https://github.com/manticoresoftware/manticoresearch/commit/e0baf7392937969e640fb5d10e8ccb312fe36446) 修复索引定义中属性名称重复的问题
* [提交 ca81](https://github.com/manticoresoftware/manticoresearch/commit/ca81114bf1ac7ea7adcc8509bdc14d1f30b4f461) 修复 HIGHLIGHT() 中的 html_strip
* [提交 493a](https://github.com/manticoresoftware/manticoresearch/commit/493a5e916d70fd4a3f14d1ee487234434f4a7ce3) 修复 HIGHLIGHT() 中的 passage 宏
* [提交 a82d](https://github.com/manticoresoftware/manticoresearch/commit/a82d41c775692f837f267d554d7d4a4b499d69f0) 修复 RT 索引创建小或大磁盘块时的双缓冲问题
* [提交 a404](https://github.com/manticoresoftware/manticoresearch/commit/a404c85de0d41d9d1c75b7540479d8a3af9e61aa) 修复 kqueue 的事件删除

* [提交 8bea](https://github.com/manticoresoftware/manticoresearch/commit/8bea0f6fd1666b817995c4dd6e67f6f1c9161638) 修复 RT 索引的 rt_mem_limit 大值时磁盘块保存问题
* [提交 8707](https://github.com/manticoresoftware/manticoresearch/commit/8707f0395b7ace52dc0cba9c8ed92a2bc4392932) 修复索引时浮点溢出
* [提交 a564](https://github.com/manticoresoftware/manticoresearch/commit/a56434ce71bb7714b4d8e75278aaa075b27ece80) 修复向 RT 索引插入负 ID 文档时出错失败的问题
* [提交 bbeb](https://github.com/manticoresoftware/manticoresearch/commit/bbebfd7551a68641154761cac962053b54806759) 修复 ranker fieldmask 导致的服务器崩溃
* [提交 3809](https://github.com/manticoresoftware/manticoresearch/commit/3809cc1b0863e3a5a200dc423be3f7c38be56113) 修复使用查询缓存时崩溃
* [提交 dc2a](https://github.com/manticoresoftware/manticoresearch/commit/dc2a585bc724678679e4297362200bbd49ce32bb) 修复使用 RT 索引 RAM 段并行插入时崩溃

## 版本 3.2.2，2019年12月19日
### 功能
* RT 索引的自增 ID
* 通过新的 HIGHLIGHT() 函数支持 docstore 高亮，也可在 HTTP API 中使用
* SNIPPET() 现可使用特殊函数 QUERY()，返回当前 MATCH 查询
* 新增高亮函数的 field_separator 选项
### 改进和变更
* 远程节点存储字段的惰性提取（可显著提升性能）
* 字符串和表达式不再破坏多查询和 FACET 优化
* RHEL/CentOS 8 构建现使用 mariadb-connector-c-devel 中的 mysql libclient

* ICU 数据文件现随安装包提供，移除了 icu_data_dir
* systemd 服务文件包含 'Restart=on-failure' 策略
* indextool 现可在线检查实时索引
* 默认配置现为 /etc/manticoresearch/manticore.conf
* RHEL/CentOS 上的服务由 'searchd' 更名为 'manticore'
* 移除了 query_mode 和 exact_phrase snippet 选项
### Bug修复
* [提交 6ae4](https://github.com/manticoresoftware/manticoresearch/commit/6ae474c7894a6bee222d5b18e59a44fdbf57843a) 修复通过 HTTP 接口执行 SELECT 查询的崩溃
* [提交 5957](https://github.com/manticoresoftware/manticoresearch/commit/59577513a49eac5a4a3c5e2cb38394d3246b5d35) 修复 RT 索引保存磁盘块但未标记部分文档已删除
* [提交 e861](https://github.com/manticoresoftware/manticoresearch/commit/e861f0fca0e88924450695d4e9d6acff7a36558a) 修复使用 dist_threads 多索引或多查询时崩溃
* [提交 4409](https://github.com/manticoresoftware/manticoresearch/commit/440991fc977b8479800b45cdbc862eeb1ba5d965) 修复生成长词中宽 UTF8 码点时的中缀崩溃
* [提交 5fd5](https://github.com/manticoresoftware/manticoresearch/commit/5fd599b48bba527c023e6aa0b262dca51ffb8a1c) 修复向 IOCP 添加套接字时的竞态条件
* [提交 cf10](https://github.com/manticoresoftware/manticoresearch/commit/cf10d7d3589f77adfaefbe52a51777bc9c67cf99) 修复 bool 查询与 json 选择列表的冲突
* [提交 996d](https://github.com/manticoresoftware/manticoresearch/commit/996de77f4c3ec103d965e28c36098fd07dba50c4) 修复 indextool 检查报告错误的跳过列表偏移和 doc2row 查找的检查
* [提交 6e3f](https://github.com/manticoresoftware/manticoresearch/commit/6e3fc9e88941c9427410f9d0adeebb7dd09a900f) 修复索引器在大数据下因负跳过列表偏移产生坏索引
* [提交 faed](https://github.com/manticoresoftware/manticoresearch/commit/faed3220b423a96401982bf47117edf1f62c584e) 修复 JSON 仅将数字转换为字符串及表达式中的 JSON 字符串到数字转换
* [提交 5331](https://github.com/manticoresoftware/manticoresearch/commit/533197200249d648ebcbdaca5d605861ee566348) 修复 indextool 在命令行设置多个命令时退出时错误码问题
* [提交 7955](https://github.com/manticoresoftware/manticoresearch/commit/795520ac351d22e4497ebaf6c249bf25081842b6) 修复 #275 磁盘空间不足时 binlog 状态无效

* [提交 2284](https://github.com/manticoresoftware/manticoresearch/commit/2284da5e3ff77bf7be4038a4d950c0580e4159c8) 修复 #279 在 JSON 属性的 IN 过滤器崩溃
* [提交 ce2e](https://github.com/manticoresoftware/manticoresearch/commit/ce2e4b4765111a7d331ffc256911f12770a8942d) 修复 #281 错误的管道关闭调用
* [Commit 5355](https://github.com/manticoresoftware/manticoresearch/commit/535589ba8a37baf5c6056afb5ff969fd70476feb) 修复服务器在处理以字符串编码的递归 JSON 属性的 CALL PQ 时挂起的问题
* [Commit a5fc](https://github.com/manticoresoftware/manticoresearch/commit/a5fc8a36e1644ee051632408ab8b53e79aeb9ff4) 修复 multiand 节点在文档列表末尾超前的问题

* [Commit a362](https://github.com/manticoresoftware/manticoresearch/commit/a3628617ef4ee89a811d4ba2c4a04cbbc298c1b5) 修复线程公共信息的检索
* [Commit f8d2](https://github.com/manticoresoftware/manticoresearch/commit/f8d2d7bbbb90af3d2ee97424f97b05927f09d42b) 修复文档存储缓存锁
## 版本 3.2.0，2019年10月17日
### 功能
* 文档存储

* 新的指令 stored_fields、docstore_cache_size、docstore_block_size、docstore_compression、docstore_compression_level
### 改进和变更
* 改进了 SSL 支持
* 更新了非 CJK 内置字符集
* 禁用了 UPDATE/DELETE 语句在查询日志中记录为 SELECT
* 支持 RHEL/CentOS 8 软件包
### Bug 修复
* [Commit 301a](https://github.com/manticoresoftware/manticoresearch/commit/301a806b14725e842300ee225521407464374c41) 修复 RT 索引磁盘块中替换文档时的崩溃
* [Commit 46c1](https://github.com/manticoresoftware/manticoresearch/commit/46c1cad8fa11c1fc09cc34b20d978fdfcd4e6d27) 修复 \#269 LIMIT N OFFSET M 问题
* [Commit 92a4](https://github.com/manticoresoftware/manticoresearch/commit/92a46edaa196ef33f3ac2225fbdba2c2b88ee45d) 修复 DELETE 语句中显式设置 id 或提供 id 列表以跳过搜索的情况

* [Commit 8ca7](https://github.com/manticoresoftware/manticoresearch/commit/8ca78c138d837caab30dc6e2343a3c4d1687bb87) 修复 Windows Poller 中 netloop 事件移除后的错误索引
* [Commit 6036](https://github.com/manticoresoftware/manticoresearch/commit/603631e2bff5cb0acf25f68b85621742490eccd9) 修复通过 HTTP 传输的 JSON 浮点数舍入问题
* [Commit 62f6](https://github.com/manticoresoftware/manticoresearch/commit/62f64cb9e75ec28fc3c29f8414ce9656f5580254) 修复远程片段先检查空路径的问题；修复 Windows 测试
* [Commit aba2](https://github.com/manticoresoftware/manticoresearch/commit/aba274c2c5abc1029894ba9f95b786bda93e8f22) 修复配置重新加载，使其在 Windows 上行为与 Linux 相同
* [Commit 6b8c](https://github.com/manticoresoftware/manticoresearch/commit/6b8c4242ef77c3fa4d0ccb7d76d81714b6728f0b) 修复 \#194 PQ 以支持形态学和词干提取器
* [Commit 174d](https://github.com/manticoresoftware/manticoresearch/commit/174d312905285aa5746f4f404c53e6dc669ef42f) 修复 RT 废弃段管理
## 版本 3.1.2，2019年8月22日

### 功能和改进
* 实验性 HTTP API 的 SSL 支持
* CALL KEYWORDS 的字段过滤器
* /json/search 的 max_matches
* 默认 Galera gcache.size 的自动调整
* 改进了 FreeBSD 支持
### Bug 修复
* [Commit 0a1a](https://github.com/manticoresoftware/manticoresearch/commit/0a1a2c81038794983b4e30c8aefc7cc20e89b03f) 修复 RT 索引复制到存在相同 RT 索引但路径不同的节点的问题

* [Commit 4adc](https://github.com/manticoresoftware/manticoresearch/commit/4adc075294ac823289f745e2cc419f18c7dcf2e2) 修复无活动索引的刷新重新调度问题
* [Commit d6c0](https://github.com/manticoresoftware/manticoresearch/commit/d6c00a6f750c04021747e9c99e0f77707a4cbfc3) 改进 RT/PQ 索引刷新重新调度
* [Commit d0a7](https://github.com/manticoresoftware/manticoresearch/commit/d0a7c95906b9b5ba33c98267d54501cfe27508d6) 修复 \#250 TSV 和 CSV 管道源的 index_field_lengths 索引选项
* [Commit 1266](https://github.com/manticoresoftware/manticoresearch/commit/1266d54800242709fbe2c34e72eb69b6595b23bd) 修复 indextool 对空索引块索引检查的错误报告
* [Commit 553c](https://github.com/manticoresoftware/manticoresearch/commit/553ca73ca111f28e0888d59cd48b0d6df181ecd6) 修复 Manticore SQL 查询日志中的空选择列表
* [Commit 56c8](https://github.com/manticoresoftware/manticoresearch/commit/56c8584456293665ddd4ce4c94307d74473a78b9) 修复 indexer -h/--help 响应
## 版本 3.1.0，2019年7月16日
### 功能和改进
* 实时索引复制
* 中文 ICU 分词器
* 新的形态学选项 icu_chinese
* 新指令 icu_data_dir
* 复制用的多语句事务
* LAST_INSERT_ID() 和 @session.last_insert_id

* SHOW VARIABLES 支持 LIKE 'pattern'
* Percolate 索引的多文档插入
* 配置文件中添加时间解析器

* 内部任务管理器
* doc 和 hit 列表组件的 mlock
* 监狱环境下的片段路径限制
### 移除
* 放弃 RLP 库支持，转而使用 ICU；所有 rlp\* 指令已移除
* 禁止通过 UPDATE 更新文档 ID
### Bug 修复
* [Commit f047](https://github.com/manticoresoftware/manticoresearch/commit/f047222339fd0e62a07e9ca8b2a829297d5db861) 修复 concat 和 group_concat 中的缺陷
* [Commit b081](https://github.com/manticoresoftware/manticoresearch/commit/b08147ee24712b5eaa51f261626b5d16c6242fdd) 修复 percolate 索引中的查询 uid 为 BIGINT 属性类型
* [Commit 4cd8](https://github.com/manticoresoftware/manticoresearch/commit/4cd85afaf76eef0ce9fa1cd073f3fed160279890) 预分配新磁盘块失败时不崩溃
* [Commit 1a55](https://github.com/manticoresoftware/manticoresearch/commit/1a551227886474054111088412f2b2396f485652) 为 ALTER 添加缺失的 timestamp 数据类型

* [Commit f3a8](https://github.com/manticoresoftware/manticoresearch/commit/f3a8e096fd496a6b001d398aadbb781b2dbad929) 修复错误 mmap 读取导致的崩溃
* [Commit 4475](https://github.com/manticoresoftware/manticoresearch/commit/447577115192db7ac7bfea70b5ba58ee2c61229f) 修复复制中集群锁的哈希
* [Commit ff47](https://github.com/manticoresoftware/manticoresearch/commit/ff476df955f057bd38e8b6ad6056325e9d6fbb9c) 修复复制中 provider 泄漏
* [Commit 58dc](https://github.com/manticoresoftware/manticoresearch/commit/58dcbb779c4c30ca1560460c78e35ba114b83716) 修复 \#246 未定义的 sigmask 在 indexer 中的问题
* [提交 3dd8](https://github.com/manticoresoftware/manticoresearch/commit/3dd8278e747d534f69fe9c3d360eef188ab7b6d5) 修复 netloop 报告中的竞争条件
* [提交 a02a](https://github.com/manticoresoftware/manticoresearch/commit/a02aae05a3fa3d1caf2821f43c7e237b6de76336) 为高可用策略的再平衡器设置零间隙

## 版本 3.0.2, 2019年5月31日
### 改进

* 为文档和命中列表添加了 mmap 读取器
* `/sql` HTTP 端点响应现在与 `/json/search` 响应相同

* 新增指令 `access_plain_attrs`、`access_blob_attrs`、`access_doclists`、`access_hitlists`
* 新指令 `server_id` 用于复制设置
### 移除项
* 移除了 HTTP `/search` 端点
### 弃用项
* `read_buffer`、`ondisk_attrs`、`ondisk_attrs_default`、`mlock` 已被 `access_*` 指令替代
### Bug 修复
* [提交 849c](https://github.com/manticoresoftware/manticoresearch/commit/849c16e1040f382795ba0635cb48686f9db4f2d8) 允许在 select 列表中以数字开头的属性名
* [提交 48e6](https://github.com/manticoresoftware/manticoresearch/commit/48e6c302ca37bec0bc49c79619c60bf787ed0708) 修复 UDF 中的 MVA，修复 MVA 的别名问题
* [提交 0555](https://github.com/manticoresoftware/manticoresearch/commit/055586a9a2ebfa8874f68099c5bc798a145fd4ef) 修复 \#187 在使用带有 SENTENCE 的查询时崩溃问题


* [提交 93bf](https://github.com/manticoresoftware/manticoresearch/commit/93bf52f23f1c53c8a93a603387ca5463e1fc2ae8) 修复 \#143 支持 MATCH() 外的括号
* [提交 599e](https://github.com/manticoresoftware/manticoresearch/commit/599ee79ccd38cce0023a55cbce8f50c3dca62e38) 修复 ALTER cluster 语句保存集群状态的问题
* [提交 230c](https://github.com/manticoresoftware/manticoresearch/commit/230c321e277514b191629538eb3f02d219113d95) 修复带 blob 属性的 ALTER index 导致服务器崩溃的问题
* [提交 5802](https://github.com/manticoresoftware/manticoresearch/commit/5802b85aa93a81caea77073a3381ef912fdd677c) 修复 \#196 通过 id 过滤的问题
* [提交 25d2](https://github.com/manticoresoftware/manticoresearch/commit/25d2dabda4a299a29bd375405862126da9b4bfac) 丢弃对模板索引的搜索
* [提交 2a30](https://github.com/manticoresoftware/manticoresearch/commit/2a30d5b49d76faab031d195f533974938769b8e6) 修复 SQL 响应中 id 列为常规 bigint 类型
## 版本 3.0.0, 2019年5月6日
### 功能和改进
* 新的索引存储。非标量属性在每个索引上的大小不再限制为 4GB
* attr_update_reserve 指令
* 字符串、JSON 和 MVA 可通过 UPDATE 更新
* killlists 在加载索引时应用
* killlist_target 指令
* 多 AND 搜索提速
* 提高平均性能和内存使用效率

* 提供用于从 2.x 升级索引的 convert 工具
* CONCAT() 函数
* JOIN CLUSTER cluster AT 'nodeaddress:port'
* ALTER CLUSTER posts UPDATE nodes

* node_address 指令
* 在 SHOW STATUS 中打印节点列表

### 行为变更
* 对于带 killlists 的索引，服务器不再按照 conf 中定义的顺序轮换索引，而是遵循 killlist 目标链
* 搜索中索引的顺序不再定义 killlists 的应用顺序
* 文档 ID 现在为有符号的大整数

### 移除的指令
* docinfo（现在始终为 extern）、inplace_docinfo_gap、mva_updates_pool
## 版本 2.8.2 GA, 2019年4月2日

### 功能和改进
* Percolate 索引的 Galera 复制
* OPTION morphology
### 编译说明
Cmake 最低版本现为 3.13。编译需依赖 boost 和 libssl
开发库。
### Bug 修复
* [提交 6967](https://github.com/manticoresoftware/manticoresearch/commit/6967fedb2ef818ec1c825d482563edd05e1c9245) 修复针对多个分布式索引查询时 select 列表中有多个星号导致崩溃
* [提交 36df](https://github.com/manticoresoftware/manticoresearch/commit/36df1a407dc08263690e3492518613ace82d69ca) 修复 [\#177](https://github.com/manticoresoftware/manticoresearch/issues/177) 通过 Manticore SQL 接口的大数据包问题
* [提交 5793](https://github.com/manticoresoftware/manticoresearch/commit/57932aec734583fa93359faaf5034b2e2c9d352b) 修复 [\#170](https://github.com/manticoresoftware/manticoresearch/issues/170) 实时优化和更新 MVA 时服务器崩溃

* [提交 edb2](https://github.com/manticoresoftware/manticoresearch/commit/edb24b870423add86eba471d361e0e5aff098b18) 修复在 SIGHUP 后配置重载导致 RT 索引移除后 binlog 被移除时服务器崩溃
* [提交 bd3e](https://github.com/manticoresoftware/manticoresearch/commit/bd3e66e0085bc7e2e351b817dfc972fd8158fbce) 修复 mysql 握手认证插件负载
* [提交 6a21](https://github.com/manticoresoftware/manticoresearch/commit/6a217f6ed82fb10f752213b15617b9cedf1e8533) 修复 [\#172](https://github.com/manticoresoftware/manticoresearch/issues/172) RT 索引的 phrase_boundary 设置
* [提交 3562](https://github.com/manticoresoftware/manticoresearch/commit/3562f652753e4091fd7b5b0f65b58341c8cbcb31) 修复 [\#168](https://github.com/manticoresoftware/manticoresearch/issues/168) ATTACH 索引自身时死锁
* [提交 250b](https://github.com/manticoresoftware/manticoresearch/commit/250b3f0e74ad18dda34ba080aa13ca87798a9ac9) 修复服务器崩溃后 binlog 保存空元数据

* [提交 4aa6](https://github.com/manticoresoftware/manticoresearch/commit/4aa6c69ad3a181cd7c5dafb990fb528121f68e59) 修复由于来自 RT 索引的磁盘块字符串导致排序器崩溃
## 版本 2.8.1 GA, 2019年3月6日
### 功能和改进
* SUBSTRING_INDEX()
* Percolate 查询支持 SENTENCE 和 PARAGRAPH
* 用于 Debian/Ubuntu 的 systemd 生成器；还添加了 LimitCORE 以支持 core dump
### Bug 修复
* [提交 84fe](https://github.com/manticoresoftware/manticoresearch/commit/84fe7405d17b59f4dcde15a6c8e8503923503def) 修复在全部匹配模式且全文查询为空时服务器崩溃
* [提交 daa8](https://github.com/manticoresoftware/manticoresearch/commit/daa88b579fa373d5d2e869e43a2e178363b0fef1) 修复删除静态字符串时崩溃
* [Commit 2207](https://github.com/manticoresoftware/manticoresearch/commit/22078537dddbd9ce81a182dcc879f235b6e87004) 修复 indextool 失败时的退出代码（FATAL）
* [Commit 0721](https://github.com/manticoresoftware/manticoresearch/commit/0721696d6780c200c65d596624a8187983fb7fcb) 修复 [\#109](https://github.com/manticoresoftware/manticoresearch/issues/109) 前缀无匹配的问题，原因是精确匹配检查错误
* [Commit 8af8](https://github.com/manticoresoftware/manticoresearch/commit/8af810111b8e2f87bc9f378172eff1ab9725c7e7) 修复 [\#161](https://github.com/manticoresoftware/manticoresearch/issues/161) 重新加载 RT 索引的配置设置
* [Commit e2d5](https://github.com/manticoresoftware/manticoresearch/commit/e2d592773f4fd702ace90070a793a842a0fec6ab) 修复服务器访问大 JSON 字符串时崩溃
* [Commit 75cd](https://github.com/manticoresoftware/manticoresearch/commit/75cd1342d05cdc352e3dd145cdbc6f79394a165b) 修复 JSON 文档中 PQ 字段被索引剥离器修改导致同级字段匹配错误的问题
* [Commit e2f7](https://github.com/manticoresoftware/manticoresearch/commit/e2f775437df922674d772ed4417780492502e65a) 修复 RHEL7 构建下解析 JSON 时服务器崩溃
* [Commit 3a25](https://github.com/manticoresoftware/manticoresearch/commit/3a25a5808feb3f8b80866991b436d6c1241618c2) 修复反斜杠在边缘时 JSON 解除转义崩溃

* [Commit be9f](https://github.com/manticoresoftware/manticoresearch/commit/be9f497872bea62dbdccd64fc28294fd7776c289) 修复选项 'skip_empty' 跳过空文档且不警告它们不是有效的 JSON
* [Commit 266e](https://github.com/manticoresoftware/manticoresearch/commit/266e0e7b088549722a805fc837bf101ff681a5e8) 修复 [\#140](https://github.com/manticoresoftware/manticoresearch/issues/161) 当浮点数精度为6位不够时输出8位数字
* [Commit 3f6d](https://github.com/manticoresoftware/manticoresearch/commit/3f6d23891064ee8e94030c4231497cdd7da33a6a) 修复空 jsonobj 创建
* [Commit f3c7](https://github.com/manticoresoftware/manticoresearch/commit/f3c7848a59b7eedc67c3403d6d4b37ce6fa94dc8) 修复 [\#160](https://github.com/manticoresoftware/manticoresearch/issues/161) 空 mva 输出 NULL 而非空字符串
* [Commit 0afa](https://github.com/manticoresoftware/manticoresearch/commit/0afa2ed058e5759470b1d7354c722faab34f98bb) 修复无 pthread_getname_np 时编译失败
* [Commit 9405](https://github.com/manticoresoftware/manticoresearch/commit/9405fccdeb27f0302a8d9a848981d30080216777) 修复线程池工作线程服务器关闭时崩溃
## 版本 2.8.0 GA，2019年1月28日
### 改进
* 针对逐条索引的分布式索引支持
* CALL PQ 新选项及变更：
    *   skip_bad_json
    *   mode（稀疏 / 分片）
    *   JSON 文档可传递为 JSON 数组
    *   shift
    *   列名 'UID', 'Documents', 'Query', 'Tags', 'Filters' 被重命名为 'id', 'documents', 'query', 'tags', 'filters'
* 支持 DESCRIBE pq TABLE
* 不再支持 SELECT FROM pq WHERE UID，改用 'id'
* 对 pq 索引的 SELECT 功能与常规索引持平（例如支持用 REGEX() 过滤规则）

* PQ 标签支持 ANY/ALL
* 表达式对 JSON 字段自动转换，无需显式转换
* 内置 'non_cjk' 字符集表和 'cjk' ngram 字符
* 内置 50 种语言的停用词集合
* 停用词声明中多个文件也可用逗号分隔
* CALL PQ 接受 JSON 文档数组
### 修复
* [Commit a4e1](https://github.com/manticoresoftware/manticoresearch/commit/a4e19afee54dafdc04b0da53372dafd8c0d332d6) 修复 csjon 相关内存泄漏
* [Commit 28d8](https://github.com/manticoresoftware/manticoresearch/commit/28d862774874891a03e361da1b0347ebe8066ce0) 修复 JSON 缺失值导致崩溃
* [Commit bf4e](https://github.com/manticoresoftware/manticoresearch/commit/bf4e9ea27c349cdc9ae4e54e960a4a17b90e38fa) 修复 RT 索引保存时空元数据问题
* [Commit 33b4](https://github.com/manticoresoftware/manticoresearch/commit/33b4573529e7c7c7bce19bf9d54054866f30d99a) 修复词形还原序列丢失形态标志（exact）
* [Commit 6b95](https://github.com/manticoresoftware/manticoresearch/commit/6b95d48240d3b5520afade19c249d79e020a5e88) 修复字符串属性超过4M时使用饱和处理替代溢出
* [Commit 6214](https://github.com/manticoresoftware/manticoresearch/commit/621418b829e70af36aaa322a2f51ece3f86bc499) 修复 SIGHUP 信号关闭已禁用索引时服务器崩溃
* [Commit 3f7e](https://github.com/manticoresoftware/manticoresearch/commit/3f7e35d1482966cc45d5abbcb2de5de3508d66b0) 修复同时执行 API 会话状态命令时服务器崩溃
* [Commit cd9e](https://github.com/manticoresoftware/manticoresearch/commit/cd9e4f1709a48ddafc128c450c2d882bc11ba50e) 修复删除带字段过滤的 RT 索引查询时服务器崩溃
* [Commit 9376](https://github.com/manticoresoftware/manticoresearch/commit/9376470d455fcd256c6abff79c431a6919dfa7ac) 修复 CALL PQ 调用分布式索引空文档时崩溃
* [Commit 8868](https://github.com/manticoresoftware/manticoresearch/commit/8868b207644f4f8cc1ab0c270adb35493fed22e1) 修复 Manticore SQL 错误消息超过512字符被截断
* [Commit de9d](https://github.com/manticoresoftware/manticoresearch/commit/de9deda9c142823d6dbf529423417b5c670fae94) 修复无 binlog 保存逐条索引时崩溃
* [Commit 2b21](https://github.com/manticoresoftware/manticoresearch/commit/2b219e1a32791e7740bb210b7d408f96abc6374f) 修复 OSX 下 HTTP 接口无法工作

* [Commit e92c](https://github.com/manticoresoftware/manticoresearch/commit/e92c60240f8f9e1756bfa14fba0bbb402586a389) 修复 indextool 检查 MVA 时的错误错误消息
* [Commit 238b](https://github.com/manticoresoftware/manticoresearch/commit/238bdea59bad89f097403f1c978658ce45f16c70) 修复 FLUSH RTINDEX 写锁，防止保存及 rt_flush_period 期间锁定整个索引
* [Commit c26a](https://github.com/manticoresoftware/manticoresearch/commit/c26a236bd42b7082db079a51a84172066d337d64) 修复了 ALTER percolate 索引在等待搜索加载时卡住的问题
* [Commit 9ee5](https://github.com/manticoresoftware/manticoresearch/commit/9ee5703f6c28f57cd5bff5e705966d93fe30d267) 修复了当 max_children 值为 0 时，使用线程池默认工作线程数的问题
* [Commit 5138](https://github.com/manticoresoftware/manticoresearch/commit/5138fc048a1bd146b271ce6a72fe954344281f69) 修复了在带有 index_token_filter 插件、停用词和 stopword_step=0 的索引中索引数据时的错误

* [Commit 2add](https://github.com/manticoresoftware/manticoresearch/commit/2add3d319ac62fe450bf60e89033100853dc2ecf) 修复了当索引定义中依然使用 aot 词形还原器但缺少 lemmatizer_base 时导致的崩溃
## 版本 2.7.5 GA，2018年12月4日
### 改进
* REGEX 函数
* json API 搜索的 limit/offset 支持
* qcache 的性能分析点
### 修复

* [Commit eb3c](https://github.com/manticoresoftware/manticoresearch/commit/eb3c7683b9740cff4cc4cadd2ab9f990b9f7d173) 修复了在 FACET 查询时，使用多属性宽类型导致服务器崩溃的问题
* [Commit d915](https://github.com/manticoresoftware/manticoresearch/commit/d915cf6696514f4be365ffb5981bccaa15983e65) 修复了 FACET 查询主选择列表中隐式 group by 的问题
* [Commit 5c25](https://github.com/manticoresoftware/manticoresearch/commit/5c25dc271a8c11a1c82ca3c88e518acbc6a8c164) 修复了带有 GROUP N BY 的查询导致崩溃的问题
* [Commit 85d3](https://github.com/manticoresoftware/manticoresearch/commit/85d30a232d4ffd89916b673a9b2db946d272a945) 修复了内存操作崩溃时的死锁问题
* [Commit 8516](https://github.com/manticoresoftware/manticoresearch/commit/85166b5ef2be38061d62f2b20f15acaa0311258a) 修复了 indextool 检查期间的内存消耗问题

* [Commit 58fb](https://github.com/manticoresoftware/manticoresearch/commit/58fb03166830d8b23e7ab9ce56309db7ddac22f4) 修复了不再需要包含 gmock 的问题，因为上游已自行解决
## 版本 2.7.4 GA，2018年11月1日
### 改进
* 对远程分布式索引执行 SHOW THREADS 时打印原始查询，而非 API 调用
* SHOW THREADS 新选项 `format=sphinxql` 以 SQL 格式打印所有查询
* SHOW PROFILE 增加了额外的 `clone_attrs` 阶段
### 修复
* [Commit 4f15](https://github.com/manticoresoftware/manticoresearch/commit/4f155712a0bccc1bd01cc191647bc8cff814888e) 修复了在无 malloc_stats 和 malloc_trim 的 libc 下构建失败的问题

* [Commit f974](https://github.com/manticoresoftware/manticoresearch/commit/f974f20bda3214a56877c393a192be1a77150958) 修复了 CALL KEYWORDS 结果集中的词内特殊符号问题
* [Commit 0920](https://github.com/manticoresoftware/manticoresearch/commit/092083282ea8ae0b2e72fb6989c57ccec81e74ac) 修复了通过 API 或远程代理调用分布式索引时 CALL KEYWORDS 失效的问题
* [Commit fd68](https://github.com/manticoresoftware/manticoresearch/commit/fd686bfe88b720ffd7642e36897ba45161cbd7d2) 修复了分布式索引 agent_query_timeout 未作为 max_query_time 传递给代理的问题
* [Commit 4ffa](https://github.com/manticoresoftware/manticoresearch/commit/4ffa623e9d357a3b0b441615089e211f92f8de32) 修复了磁盘分块的总文档计数被 OPTIMIZE 命令影响，导致权重计算错误的问题
* [Commit dcaf](https://github.com/manticoresoftware/manticoresearch/commit/dcaf4e0e3c8e9b0fbec47bbf3307feddec0936a6) 修复了 RT 索引在混合查询时多重尾部命中问题
* [Commit eee3](https://github.com/manticoresoftware/manticoresearch/commit/eee381754e902a43a8f499e1c950198a2a3e6ee0) 修复了轮换时的死锁问题

## 版本 2.7.3 GA，2018年9月26日
### 改进
* CALL KEYWORDS 新增 sort_mode 选项
* VIP 连接的 DEBUG 可执行 'crash <password>' 发起服务器的有意 SIGEGV 操作
* DEBUG 支持执行 'malloc_stats' 将 malloc 统计信息输出至 searchd.log，'malloc_trim' 执行 malloc_trim()
* 如果系统中存在 gdb，改进了回溯信息
### 修复
* [Commit 0f3c](https://github.com/manticoresoftware/manticoresearch/commit/0f3cc333f6129451ad1ae2c97905b6c04ba5d454) 修复了 Windows 上重命名操作崩溃或失败的问题
* [Commit 1455](https://github.com/manticoresoftware/manticoresearch/commit/1455ba2bf8079f03ce7f861a8d9662a360d705de) 修复了 32 位系统上服务器崩溃的问题
* [Commit ad37](https://github.com/manticoresoftware/manticoresearch/commit/ad3710d53b5cd6a28b8c60352d0f9e285b03d9fa) 修复了空 SNIPPET 表达式导致服务器崩溃或挂起的问题
* [Commit b36d](https://github.com/manticoresoftware/manticoresearch/commit/b36d79214364500252665860f000ae85343528b6) 修复了非渐进式优化失效问题，且修正了渐进式优化不为最旧磁盘分块创建杀死列表的缺陷
* [Commit 34b0](https://github.com/manticoresoftware/manticoresearch/commit/34b032499afd42ce47a4c7247814b4031094388a) 修复了线程池工作模式下 SQL 和 API 对 queue_max_length 的错误响应
* [Commit ae4b](https://github.com/manticoresoftware/manticoresearch/commit/ae4b3202cbdb8014cabe2b90e269d5cf74f49871) 修复了在 PQ 索引中设置 regexp 或 rlp 选项时添加全扫描查询导致崩溃的问题
* [Commit f80f](https://github.com/manticoresoftware/manticoresearch/commit/f80f8d5d7560187078868aed9a9575f4549e98aa) 修复了连续调用 PQ 导致崩溃的问题
* [Commit 9742](https://github.com/manticoresoftware/manticoresearch/commit/9742f5f0866af73f8cd8483ecd18a507ea80dd65) 重构了 AcquireAccum
* [Commit 39e5](https://github.com/manticoresoftware/manticoresearch/commit/39e5bc3751b7295222eb76407c5d72ce1dad545b) 修复了调用 pq 后的内存泄漏
* [Commit 21bc](https://github.com/manticoresoftware/manticoresearch/commit/21bcc6d17395f0f57dde79f6716ef303b7ea527d) 外观重构（采用 c++11 风格的 c-trs、默认参数、nullptr）
* [Commit 2d69](https://github.com/manticoresoftware/manticoresearch/commit/2d690398f14c736956cfdd66feb4d3091d6b3a4d) 修复了向 PQ 索引插入重复项时的内存泄漏
* [Commit 5ed9](https://github.com/manticoresoftware/manticoresearch/commit/5ed92c4b66da6423fa28f11b1fd59103ed1e1fb9) 修复了 JSON 字段 IN 操作中大值导致的崩溃问题
* [提交 4a52](https://github.com/manticoresoftware/manticoresearch/commit/4a5262e2776aa261e34ba5c914ea60804f59de3f) 修复了在对设置了扩展限制的 RT 索引执行 CALL KEYWORDS 语句时服务器崩溃的问题
* [提交 5526](https://github.com/manticoresoftware/manticoresearch/commit/552646bb6fefde4e2525298e43e628839b421d3d) 修复了 PQ matches 查询中的无效过滤器；
* [提交 204f](https://github.com/manticoresoftware/manticoresearch/commit/204f521408b730198a5bab2c20407a3d7df94873) 引入了指针属性的小对象分配器
* [提交 2545](https://github.com/manticoresoftware/manticoresearch/commit/25453e5387152c0575d1eda1b2beb89a49c7493a) 将 ISphFieldFilter 重构为引用计数风格
* [提交 1366](https://github.com/manticoresoftware/manticoresearch/commit/1366ee06e828397074b69bec9265d7147170c60e) 修复了对非终止字符串使用 strtod 时的未定义行为/段错误
* [提交 94bc](https://github.com/manticoresoftware/manticoresearch/commit/94bc6fcbdae4d08bcf6da2966e3ec2966091289f) 修复了 JSON 结果集处理中的内存泄漏
* [提交 e78e](https://github.com/manticoresoftware/manticoresearch/commit/e78e9c948963416caff9e4e46296a58080107835) 修复了在应用属性添加时超出内存块末尾读取的问题
* [提交 fad5](https://github.com/manticoresoftware/manticoresearch/commit/fad572fb543606b33e688eb82a485b86011545fd) 修复了 CSphDict 的引用计数风格重构
* [提交 fd84](https://github.com/manticoresoftware/manticoresearch/commit/fd841a472b472280c93f4af3506851d19f6adc45) 修复了 AOT 内部类型外泄漏
* [提交 5ee7](https://github.com/manticoresoftware/manticoresearch/commit/5ee7f208c7685f2ebb9e5623a8802b721e2a7f0a) 修复了分词器管理中的内存泄漏
* [提交 116c](https://github.com/manticoresoftware/manticoresearch/commit/116c5f1abebee9a0d99afe93546a1d8e4c6c6958) 修复了分组器中的内存泄漏
* [提交 56fd](https://github.com/manticoresoftware/manticoresearch/commit/56fdbc9ab38973a9a0ba8ccee45f71cf33c16423) 对匹配中的动态指针进行了特殊的释放/复制（修复了分组器中的内存泄漏）
* [提交 b1fc](https://github.com/manticoresoftware/manticoresearch/commit/31db660f378541aa0d1e7cf2e7209bb0a1f47fd8) 修复了 RT 动态字符串的内存泄漏
* [提交 517b](https://github.com/manticoresoftware/manticoresearch/commit/517b9e8749f092e923cd2884b674b5bb84e20755) 重构了分组器
* [提交 b1fc](https://github.com/manticoresoftware/manticoresearch/commit/b1fc16140e5dc44290686330b476a254e0092107) 小幅重构（C++11 的 c-trs，部分格式调整）
* [提交 7034](https://github.com/manticoresoftware/manticoresearch/commit/7034e073f4cf2844762e0a464b7c3de05d3d122b) 将 ISphMatchComparator 重构为引用计数风格
* [提交 b1fc](https://github.com/manticoresoftware/manticoresearch/commit/b1fc16140e5dc44290686330b476a254e0092107) 将克隆器私有化
* [提交 efbc](https://github.com/manticoresoftware/manticoresearch/commit/efbc0511d6809c4a57453c7283f9bf53f3fb8d97) 简化了 MVA_UPSIZE、DOCINFO2ID_T、DOCINFOSETID 的本地小端处理
* [提交 6da0](https://github.com/manticoresoftware/manticoresearch/commit/6da0df4ac9946fc59f98d229b90e12c9221dd4c0) 为超级测试增加了 valgrind 支持
* [提交 1d17](https://github.com/manticoresoftware/manticoresearch/commit/1d17669cb7ec54ac80b392e101f37a688eb98080) 修复了由于连接中“success”标志竞争导致的崩溃
* [提交 5a09](https://github.com/manticoresoftware/manticoresearch/commit/5a09c32d7ded56b2c0bd3e2ad7968cb8d6362919) 将 epoll 切换为边缘触发模式
* [提交 5d52](https://github.com/manticoresoftware/manticoresearch/commit/5d528682737fca03a4352a3093b1319ec27d598c) 修复了表达式中的 IN 语句，格式类似过滤器
* [提交 bd8b](https://github.com/manticoresoftware/manticoresearch/commit/bd8b3c976ff8b4667f55dfdd101b20a920137ac5) 修复了在提交带有大 docid 的文档时 RT 索引崩溃的问题
* [提交 ce65](https://github.com/manticoresoftware/manticoresearch/commit/ce656b83b747ce7c55795b53915770c13fb99cfe) 修复了 indextool 中无参数选项的问题
* [提交 08c9](https://github.com/manticoresoftware/manticoresearch/commit/08c9507177820f441f534bf06a11dac5e54bebb4) 修复了扩展关键字的内存泄漏

* [提交 30c7](https://github.com/manticoresoftware/manticoresearch/commit/30c75a2f525ec9bda625d903acdc9d4d2e2413dc) 修复了JSON分组器的内存泄漏
* [提交 6023](https://github.com/manticoresoftware/manticoresearch/commit/6023f269b6f2080e4d380b0957605ef8107c8c9f) 修复了全局用户变量泄漏
* [提交 7c13](https://github.com/manticoresoftware/manticoresearch/commit/7c138f15ca23c0c8717fa12041e3db7f988fcf15) 修复了早期拒绝匹配时动态字符串的泄漏
* [提交 9154](https://github.com/manticoresoftware/manticoresearch/commit/9154b18eaed5302e8330cb3a95bd968959ccb312) 修复了 length(<expression>) 的泄漏问题
* [提交 43fc](https://github.com/manticoresoftware/manticoresearch/commit/43fca3a4e26139765d0fac8de054200dc4875fc6) 修复了解析器中因 strdup() 导致的内存泄漏
* [提交 71ff](https://github.com/manticoresoftware/manticoresearch/commit/71ff77737e63d1019b7c9804dca7fa2351025dba) 修复了表达式解析器的重构，以准确跟踪引用计数
## 版本 2.7.2 GA，2018年8月27日
### 改进
* 兼容 MySQL 8 客户端
* 支持带重新配置的 [TRUNCATE](Emptying_a_table.md)
* 退役 RT 索引 SHOW STATUS 上的内存计数器
* 多代理的全局缓存

* 提升了 Windows 上的 IOCP 性能
* HTTP 协议的 VIP 连接支持
* Manticore SQL [DEBUG](Reporting_bugs.md#DEBUG) 命令，可运行多种子命令
* [shutdown_token](Server_settings/Searchd.md#shutdown_token) - 使用 DEBUG 命令调用 `shutdown` 时需要的密码的 SHA1 哈希
* 新增 SHOW AGENT STATUS 的统计项 (_ping, _has_perspool, _need_resolve)
* indexer 的 --verbose 选项现在支持 \[debugvv\] 用于打印调试信息
### 修复
* [提交 3900](https://github.com/manticoresoftware/manticoresearch/commit/390082a7be0a1f9539b30361d11d54de35c62a44) 移除了 optimize 中的写锁
* [提交 4c33](https://github.com/manticoresoftware/manticoresearch/commit/4c337648329be22373be33333cbc9c5664d18b80) 修复了重新加载索引设置时的写锁问题
* [Commit b5ea](https://github.com/manticoresoftware/manticoresearch/commit/b5ea8dc0b18a23dbd739e92c66dd93b0094df14b) 修复了带有 JSON 过滤器的查询中的内存泄漏
* [Commit 930e](https://github.com/manticoresoftware/manticoresearch/commit/930e831a43fd3d1ce996185be968d1df49a088d0) 修复了 PQ 结果集中空文档的问题
* [Commit 53de](https://github.com/manticoresoftware/manticoresearch/commit/53deecfecb374431f84f8592a1095a77407b8aea) 修复了因移除任务导致的任务混淆问题
* [Commit cad9](https://github.com/manticoresoftware/manticoresearch/commit/cad9b9c617719aad97e9f5a2ae93491153d8e504) 修复了远程主机计数错误
* [Commit 9000](https://github.com/manticoresoftware/manticoresearch/commit/90008cc15c4684d445d37a391bb4a667bbc2e365) 修复了解析代理描述符的内存泄漏
* [Commit 978d](https://github.com/manticoresoftware/manticoresearch/commit/978d839641ad6040c22c9fc3b703b1a02685f266) 修复了搜索中的泄漏
* [Commit 0193](https://github.com/manticoresoftware/manticoresearch/commit/0193946105ac08913362b7d7acf0c90f9b38dc85) 对显式/内联 c-trs，覆盖/最终使用进行了美化修改
* [Commit 943e](https://github.com/manticoresoftware/manticoresearch/commit/943e2997b33f9b2eda23d94a66068910f9476ebd) 修复了本地/远程模式下 json 的泄漏
* [Commit 02db](https://github.com/manticoresoftware/manticoresearch/commit/02dbdd6f1d0b72e7e458ebebdd2c67f989577e6b) 修复了本地/远程模式下 json 排序列表达式的泄漏
* [Commit c74d](https://github.com/manticoresoftware/manticoresearch/commit/c74d0b4035e4214510376845b22cb676a8da24a3) 修复了常量别名的泄漏
* [Commit 6e5b](https://github.com/manticoresoftware/manticoresearch/commit/6e5b57e1d3952c9695376728a143bf5434208f53) 修复了预读取线程的泄漏
* [Commit 39c7](https://github.com/manticoresoftware/manticoresearch/commit/39c740fef5bf71da0bee60a35f5dd9f471af850f) 修复了由于 netloop 中等待卡住导致退出卡死的问题
* [Commit adaf](https://github.com/manticoresoftware/manticoresearch/commit/adaf9772f95772ad6d5297acace6c5cb92b56ad5) 修复了将 HA 代理更改为普通主机时 "ping" 行为卡住的问题
* [Commit 32c4](https://github.com/manticoresoftware/manticoresearch/commit/32c40eac9cdd40b15d79e58b2f15416f164f9701) 分离了仪表板存储的垃圾回收
* [Commit 511a](https://github.com/manticoresoftware/manticoresearch/commit/511a3cf2c1f1858f6e94fe9f8175b7216db3cbd6) 修复了引用计数指针的问题
* [Commit 32c4](https://github.com/manticoresoftware/manticoresearch/commit/af2326cd2927effbad7bec88e8dc238a566cf27e) 修复了 indextool 在非存在索引时的崩溃
* [Commit 156e](https://github.com/manticoresoftware/manticoresearch/commit/156edc717dc63d3ea120f42466eba6e022da88be) 修复了 xmlpipe 索引中超出属性/字段的输出名称
* [Commit cdac](https://github.com/manticoresoftware/manticoresearch/commit/cdac6d1f17d384bfc8bae49e91241b2f346c3830) 修复了配置中无索引器节时默认索引器的值
* [Commit e61e](https://github.com/manticoresoftware/manticoresearch/commit/e61ec00b6b27d1d5878247e2ee817f3b1e7fde16) 修复了服务器重启后 RT 索引磁盘块中错误的嵌入停止词

* [Commit 5fba](https://github.com/manticoresoftware/manticoresearch/commit/5fba49d5e28c17de4c0acbd984466127f42de6e8) 修复了跳过幻影（已关闭但未最终从轮询器中删除）连接的问题
* [Commit f22a](https://github.com/manticoresoftware/manticoresearch/commit/f22ae34b623906b3c5a05a06198dad2e548dc541) 修复了混合（孤立）的网络任务
* [Commit 4689](https://github.com/manticoresoftware/manticoresearch/commit/46890e70eb8dcd00c1bf1b030538b8f131c601c2) 修复了写入后读取操作时的崩溃
* [Commit 03f9](https://github.com/manticoresoftware/manticoresearch/commit/03f9df904f611c438d5ebcaaeef911b0dc8d6c39) 修复了在 Windows 上运行测试时 searchd 崩溃的问题
* [Commit e925](https://github.com/manticoresoftware/manticoresearch/commit/e9255ed2704790a3d7f5273d38b85433463cb3ff) 修复了对常规 connect() 中 EINPROGRESS 代码的处理
* [Commit 248b](https://github.com/manticoresoftware/manticoresearch/commit/248b72f1544b27e9e82773284050d44eeb1714e1) 修复了使用 TFO 通信时的连接超时问题

## 版本 2.7.1 GA，2018 年 7 月 4 日
### 改进
* 提升了 PQ 多文档匹配中通配符的性能
* 支持 PQ 中的全扫描查询
* 支持 PQ 中的 MVA 属性


* 对穿透索引支持正则表达式和 RLP
### Bug 修复
* [Commit 6885](https://github.com/manticoresoftware/manticoresearch/commit/68856261b41f6950666f9e5122726839fb3c71d0) 修复了查询字符串丢失的问题
* [Commit 0f17](https://github.com/manticoresoftware/manticoresearch/commit/0f1770943cba53bfd2e0edfdf0b1495ff0dd0e89) 修复了 SHOW THREADS 语句中信息为空的问题
* [Commit 53fa](https://github.com/manticoresoftware/manticoresearch/commit/53faa36a916bb87868e83aa79c1a1f972fb20ca0) 修复了使用 NOTNEAR 操作符匹配时的崩溃
* [Commit 2602](https://github.com/manticoresoftware/manticoresearch/commit/26029a7cd77b518fb3a27588d7db6c8bffc73b0f) 修复了 PQ 删除时错误过滤器的错误提示
## 版本 2.7.0 GA，2018 年 6 月 11 日
### 改进
* 减少系统调用次数以避免 Meltdown 和 Spectre 补丁的影响
* 本地索引管理的内部重写
* 远程摘要的重构
* 完整配置重新加载
* 所有节点连接现在都是独立的
* 协议改进
* Windows 通信方式由 wsapoll 切换到 IO 完成端口

* TFO 可用于主服务器和节点间的通信
* [SHOW STATUS](Node_info_and_management/Node_status.md#SHOW-STATUS) 现输出服务器版本和 mysql_version_string
* 在 CALL PQ 中新增了 `docs_id` 选项用于文档调用
* 穿透查询过滤器现在可包含表达式
* 分布式索引可与 FEDERATED 配合使用
* 虚拟的 SHOW NAMES COLLATE 和 `SET wait_timeout`（提升与 ProxySQL 的兼容性）
### Bug 修复
* [Commit 5bcf](https://github.com/manticoresoftware/manticoresearch/commit/5bcff05d19e495f1a44f190885b25cbb6f217c4a) 修复了 PQ 中添加不等于标签的问题
* [Commit 9ebc](https://github.com/manticoresoftware/manticoresearch/commit/9ebc58916cd515eaa88da66d0895aebf0d1f2b5f) 修复了将文档 ID 字段添加到 JSON 文档 CALL PQ 语句
* [Commit 8ae0](https://github.com/manticoresoftware/manticoresearch/commit/8ae0e593a286110d8505d88baa3ac9e433cb38c9) 修复了 PQ 索引的刷新语句处理程序
* [Commit c24b](https://github.com/manticoresoftware/manticoresearch/commit/c24b152344ecd77661566cddb803487f51d3c1aa) 修复了 JSON 和字符串属性的 PQ 过滤
* [Commit 1b8b](https://github.com/manticoresoftware/manticoresearch/commit/1b8bdde19aaa362785ea4c33bdc019154f8fbac4) 修复了空 JSON 字符串的解析


* [Commit 1ad8](https://github.com/manticoresoftware/manticoresearch/commit/1ad8a07dbfc1131913a8d92c261fbb48f934e8b7) 修复了使用 OR 过滤器进行多查询时的崩溃
* [Commit 69b8](https://github.com/manticoresoftware/manticoresearch/commit/69b89806380a229e36287ad9daf6f0ea2b5453eb) 修复了 indextool 使用配置公共部分 (lemmatizer_base 选项) 进行命令 (dumpheader)
* [Commit 6dbe](https://github.com/manticoresoftware/manticoresearch/commit/6dbeaf2389d2af6a9dfccb56bfc986685b9f203e) 修复了结果集和过滤中的空字符串
* [Commit 39c4](https://github.com/manticoresoftware/manticoresearch/commit/39c4eb55ecc85f6dd54f6c8c6d6dfcf87fd8748e) 修复了负文档 ID 值
* [Commit 266b](https://github.com/manticoresoftware/manticoresearch/commit/266b7054f98e7d30fc2829b5c2467dc9f90def92) 修复了非常长单词索引的单词剪切长度
* [Commit 4782](https://github.com/manticoresoftware/manticoresearch/commit/47823b0bd06cbd016a24345c99bb8693790b43dd) 修复了 PQ 中通配符查询的多个文档匹配
## 版本 2.6.4 GA，2018 年 5 月 3 日

### 特性与改进
* MySQL FEDERATED 引擎 [支持](Extensions/FEDERATED.md)
* MySQL 数据包现在返回 SERVER_STATUS_AUTOCOMMIT 标志，增加与 ProxySQL 的兼容性
* [listen_tfo](Server_settings/Searchd.md#listen_tfo) - 为所有监听器启用 TCP 快速打开连接
* indexer --dumpheader 还可以从 .meta 文件转储 RT 头
* Ubuntu Bionic 的 cmake 构建脚本
### 缺陷修复
* [Commit 355b](https://github.com/manticoresoftware/manticoresearch/commit/355b11629174813abd9cd5bf2233be0783f77745) 修复了 RT 索引的无效查询缓存条目；

* [Commit 546e](https://github.com/manticoresoftware/manticoresearch/commit/546e2297a01cc2913bc0d33052d5bcefae8737eb) 修复了无缝轮换后的索引设置丢失
* [Commit 0c45](https://github.com/manticoresoftware/manticoresearch/commit/0c4509898393993bc87194d2aca2070395ff7f83) 修复了固定词中与前缀长度设置有关的问题；增加了对不支持的词语长度的警告
* [Commit 8054](https://github.com/manticoresoftware/manticoresearch/commit/80542fa54d727c781635918e8d4d65a20c2fbd1e) 修复了 RT 索引的自动刷新顺序

* [Commit 705d](https://github.com/manticoresoftware/manticoresearch/commit/705d8c5f407726ed1c6f9745ecffa375682c8969) 修复了多个属性和多个索引查询的结果集架构问题
* [Commit b0ba](https://github.com/manticoresoftware/manticoresearch/commit/b0ba932fff4a082f642cb737381c8a70215d4cc9) 修复了批量插入时文档重复导致的一些命中丢失
* [Commit 4510](https://github.com/manticoresoftware/manticoresearch/commit/4510fa44ad54b5bbac53c591144501565130fcae) 修复了优化未能合并大型文档计数的 RT 索引的磁盘块
## 版本 2.6.3 GA，2018 年 3 月 28 日
### 改进
* 编译时使用 jemalloc。如果系统中存在 jemalloc，可以使用 cmake 标志 `-DUSE_JEMALLOC=1` 启用


### 缺陷修复
* [Commit 85a6](https://github.com/manticoresoftware/manticoresearch/commit/85a6d7e3db75f28c491042409bb677ed39eba07b) 修复了在 Manticore SQL 查询日志中扩展关键字选项的日志
* [Commit caaa](https://github.com/manticoresoftware/manticoresearch/commit/caaa3842f4dbd8507fd9e20bf3371a0c8ec0c087) 修复了 HTTP 接口以正确处理大型查询
* [Commit e386](https://github.com/manticoresoftware/manticoresearch/commit/e386d842b36195719e459b2db1fb20e402f9a51f) 修复了在启用 index_field_lengths 的 RT 索引上执行 DELETE 时服务器崩溃
* [Commit cd53](https://github.com/manticoresoftware/manticoresearch/commit/ce528f37eb5d8284661928f6f3b340a4a1985253) 修复了 cpustats searchd cli 选项在不支持的系统上工作
* [Commit 8740](https://github.com/manticoresoftware/manticoresearch/commit/8740fd63ae842c2ab0e51a8fcd6180d787ed57ec) 修复了定义最小长度的 utf8 子字符串匹配
## 版本 2.6.2 GA，2018 年 2 月 23 日
### 改进

* 在使用 NOT 运算符和批量文档时，改进了 [Percolate Queries](Searching/Percolate_query.md) 的性能。
* [percolate_query_call](Searching/Percolate_query.md) 可以根据 [dist_threads](Server_settings/Searchd.md#threads) 使用多个线程
* 新增全文匹配运算符 NOTNEAR/N
* 在渗透索引上执行 SELECT 时的 LIMIT
* [expand_keywords](Searching/Options.md#expand_keywords) 可以接受 'start','exact'（其中 'star,exact' 与 '1' 的效果相同）
* 用于 [joined fields](Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Processing_fetched_data.md#sql_joined_field) 的 ranged-main-query 使用 sql_query_range 定义的范围查询
### 缺陷修复
* [Commit 72dc](https://github.com/manticoresoftware/manticoresearch/commit/72dcf669744e9b7d636dfc213d24df85ab301f6b) 修复了搜索 RAM 段时的崩溃；双缓冲时保存磁盘块的死锁；在优化期间保存磁盘块时的死锁
* [Commit 3613](https://github.com/manticoresoftware/manticoresearch/commit/36137149a1c3c0893bdda5a28fc7e8244bf2d4ae) 修复了具有空属性名称的嵌入式 XML 架构的索引器崩溃


* [Commit 48d7](https://github.com/manticoresoftware/manticoresearch/commit/48d7e8001d2a66466ca64577f27ddc5421a67251) 修复了不拥有的 pid-file 不正确取消链接
* [Commit a556](https://github.com/manticoresoftware/manticoresearch/commit/a5563a465ddc59ef71e65f17b68bc33f9700e838) 修复了有时在临时文件夹中留下孤立的 fifo
* [提交 2376](https://github.com/manticoresoftware/manticoresearch/commit/2376e8fc4508944b96959bd10686c6d51f5145e8) 修复空FACET结果集出现错误的NULL行问题
* [提交 4842](https://github.com/manticoresoftware/manticoresearch/commit/4842b67acdd5da75672db6c08ac563b48577ddd7) 修复作为Windows服务运行时索引锁损坏问题
* [提交 be35](https://github.com/manticoresoftware/manticoresearch/commit/be35feef54ded59125750916cd211e56108eddae) 修复Mac OS上错误的iconv库
* [提交 8374](https://github.com/manticoresoftware/manticoresearch/commit/83744a977d7837d1d87cd506b88b0e7eb199efe6) 修复错误的count(\*)
## 版本 2.6.1 GA，2018年1月26日
### 改进

* [agent_retry_count](Server_settings/Searchd.md#agent_retry_count) 在代理存在镜像时，返回每个镜像的重试次数，而非每个代理，总重试次数为 agent_retry_count\*mirrors。
* [agent_retry_count](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_retry_count) 现在可以对每个索引单独指定，覆盖全局值。新增别名 [mirror_retry_count](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#mirror_retry_count)。
* 在代理定义中可以指定retry_count，表示每个代理的重试次数。
* Percolate查询现在通过HTTP JSON API支持，路径为 [/json/pq](Searching/Percolate_query.md#Performing-a-percolate-query-with-CALL-PQ)。
* 为可执行文件添加了 -h 和 -v 选项（帮助和版本信息）
* [morphology_skip_fields](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) 支持实时索引
### 修复


* [提交 a40b](https://github.com/manticoresoftware/manticore/commit/a40b0793feff65e40d10062568d9847c08d10f57) 修复 ranged-main-query 在MVA字段上使用 sql_range_step 时的工作问题
* [提交 f2f5](https://github.com/manticoresoftware/manticore/commit/f2f53757db45bcfb1544263ce0817e856656a621) 修复黑洞系统循环挂起问题，黑洞代理似乎断开连接
* [提交 84e1](https://github.com/manticoresoftware/manticore/commit/84e1f54aef25e0fce98870ad2dd784db5116f1d6) 修复查询ID一致性，修正存储查询的重复ID
* [提交 1948](https://github.com/manticoresoftware/manticore/commit/19484231814fcb82b21763a3a4a9f45adc6b2d40) 修复服务器在多种状态下关闭时崩溃问题
* [提交 9a70](https://github.com/manticoresoftware/manticore/commit/9a706b499a1d61a90076065a1a703029d49db958)  [提交 3495](https://github.com/manticoresoftware/manticore/commit/3495fd70cba8846b1a50d55d6679b039414c3d2a) 修复长查询超时问题
* [提交 3359](https://github.com/manticoresoftware/manticore/commit/3359bcd89b4f79a645fe84b8cf8616ce0addff02) 重构了基于kqueue系统（Mac OS X、BSD）的主控与代理网络监听机制
## 版本 2.6.0，2017年12月29日

### 功能与改进
* [HTTP JSON](Connecting_to_the_server/HTTP.md)：JSON查询现在支持对属性做相等条件，MVA和JSON属性可用于插入和更新，通过JSON API对分布式索引也可以执行更新和删除操作
* [Percolate查询](Searching/Percolate_query.md)
* 移除对32位docid的支持，同时删除所有将旧版32位docid索引转换/加载的代码
* [仅对特定字段启用词态变化](https://github.com/manticoresoftware/manticore/issues/7) 。新增索引指令 morphology_skip_fields，可定义不应用词态变化的字段列表
* [expand_keywords 现在可以作为查询运行时指令通过 OPTION 语句设置](https://github.com/manticoresoftware/manticore/issues/8)

### 修复
* [提交 0cfa](https://github.com/manticoresoftware/manticore/commit/0cfae4c) 修复使用rlp构建时服务器调试版本崩溃（发布版本可能有未定义行为）

* [提交 3242](https://github.com/manticoresoftware/manticore/commit/324291e) 修复启用progressive选项时，RT索引优化合并删除列表顺序错误问题
* [提交 ac0e](https://github.com/manticoresoftware/manticore/commit/ac0efee) 修复Mac上的小崩溃
* 经过深入静态代码分析后的大量小修复
* 其他小缺陷修复
### 升级
在此版本中，我们修改了主控和代理之间的内部通信协议。如果您在分布式环境中运行多个Manticoresearch实例，请确保先升级代理，然后升级主控。
## 版本 2.5.1，2017年11月23日
### 功能与改进
* [HTTP API协议](Connecting_to_the_server/HTTP.md)上的JSON查询。支持搜索、插入、更新、删除和替换操作。数据操作命令也支持批量执行，目前有些限制，如MVA及JSON属性不能用于插入、替换或更新。
* [RELOAD INDEXES](Data_creation_and_modification/Adding_data_from_external_storages/Rotating_a_table.md#RELOAD-TABLES) 命令
* [FLUSH LOGS](Logging/Rotating_query_and_server_logs.md) 命令

* [SHOW THREADS](Node_info_and_management/SHOW_THREADS.md) 可以显示优化、轮换或刷新进度。
* GROUP N BY 能正确处理MVA属性
* 黑洞代理运行在独立线程上，不再影响主控查询
* 为索引实现引用计数，避免因轮换和高负载造成停顿
* 实现SHA1哈希，尚未对外开放
* 修复在FreeBSD、macOS和Alpine上的编译问题
### 修复
* [提交 9897](https://github.com/manticoresoftware/manticore/commit/989752b) 修复块索引过滤回归问题

* [提交 b1c3](https://github.com/manticoresoftware/manticore/commit/b1c3864) 重命名 PAGE_SIZE -> ARENA_PAGE_SIZE 以兼容musl
* [提交 f213](https://github.com/manticoresoftware/manticore/commit/f2133cc) 在cmake版本低于3.1.0时禁用googletests
* [提交 f30e](https://github.com/manticoresoftware/manticore/commit/0839de7) 修复服务器重启时绑定套接字失败问题
* [提交 0807](https://github.com/manticoresoftware/manticore/commit/0807240) 修复服务器关闭时崩溃问题
* [Commit 3e3a](https://github.com/manticoresoftware/manticore/commit/3e3acc3) 修复了系统黑洞线程的显示线程问题
* [Commit 262c](https://github.com/manticoresoftware/manticore/commit/262c3fe) 重构了 iconv 的配置检查，修复了在 FreeBSD 和 Darwin 上的构建问题
## 版本 2.4.1 GA，2017年10月16日
### 功能和改进
* 属性过滤器之间的 WHERE 子句中的 OR 操作符
* 维护模式（ SET MAINTENANCE=1）
* 分布式索引中可用的 [CALL KEYWORDS](Searching/Autocomplete.md#CALL-KEYWORDS)
* [UTC 时间分组](Server_settings/Searchd.md#grouping_in_utc)
* 用于自定义日志文件权限的 [query_log_mode](Server_settings/Searchd.md#query_log_mode)
* 字段权重可以是零或负数

* [max_query_time](Searching/Options.md#max_query_time) 现在可以影响全扫描
* 为网络线程精细调优添加了 [net_wait_tm](Server_settings/Searchd.md#net_wait_tm)、[net_throttle_accept](Server_settings/Searchd.md#net_throttle_accept) 和 [net_throttle_action](Server_settings/Searchd.md#net_throttle_action)（当 workers=thread_pool 时）

* COUNT DISTINCT 支持分面搜索
* IN 可以与 JSON 浮点数组一起使用
* 多查询优化不再被整数/浮点表达式破坏
* 使用多查询优化时，[SHOW META](Node_info_and_management/SHOW_META.md) 显示 `multiplier` 行
### 编译

Manticore Search 使用 cmake 构建，编译所需的最低 gcc 版本为 4.7.2。
### 文件夹和服务
* Manticore Search 以 `manticore` 用户身份运行。
* 默认数据文件夹现在是 `/var/lib/manticore/`。
* 默认日志文件夹现在是 `/var/log/manticore/`。
* 默认 pid 文件夹现在是 `/var/run/manticore/`。
### Bug 修复
* [Commit a58c](https://github.com/manticoresoftware/manticore/commit/a58c619) 修复了破坏 java 连接器的 SHOW COLLATION 语句
* [Commit 631c](https://github.com/manticoresoftware/manticore/commit/631cf4e) 修复了处理分布式索引时的崩溃；为分布式索引哈希添加了锁；从 agent 中移除了移动和复制操作符
* [Commit 942b](https://github.com/manticoresoftware/manticore/commit/942bec0) 修复了由于并行重连导致的处理分布式索引时崩溃
* [Commit e5c1](https://github.com/manticoresoftware/manticore/commit/e5c1ed2) 修复了服务器日志存储查询时崩溃处理程序中的崩溃
* [Commit 4a4b](https://github.com/manticoresoftware/manticore/commit/4a4bda5) 修复了多查询中使用池属性时的崩溃
* [Commit 3873](https://github.com/manticoresoftware/manticore/commit/3873bfb) 通过防止索引页被包含进 core 文件，减小了 core 文件大小
* [Commit 11e6](https://github.com/manticoresoftware/manticore/commit/11e6254) 修复了当指定无效代理时启动 searchd 崩溃
* [Commit 4ca6](https://github.com/manticoresoftware/manticore/commit/4ca6350) 修复了 indexer 在 sql_query_killlist 查询中报告错误
* [Commit 123a](https://github.com/manticoresoftware/manticore/commit/123a9f0) 修复了 fold_lemmas=1 与命中计数的问题
* [Commit cb99](https://github.com/manticoresoftware/manticore/commit/cb99164) 修复了 html_strip 行为不一致的问题
* [Commit e406](https://github.com/manticoresoftware/manticore/commit/e406761) 修复了 optimize rt 索引丢失新设置的问题；修复了带同步选项的优化锁泄漏；

* [Commit 86ae](https://github.com/manticoresoftware/manticore/commit/86aeb82) 修复了错误多查询的处理
* [Commit 2645](https://github.com/manticoresoftware/manticore/commit/2645230) 修复了结果集依赖多查询顺序的问题

