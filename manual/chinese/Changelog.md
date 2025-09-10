# 更新日志

## 版本 13.6.7
**发布于**：2025年8月8日

推荐的 [MCL](https://github.com/manticoresoftware/columnar) 版本：8.0.1
推荐的 [Buddy](Installation/Manticore_Buddy.md#Manticore-Buddy) 版本：3.34.2

如果您遵循[官方安装指南](https://manticoresearch.com/install/)，则无需担心此问题。

### 新功能和改进
* 🆕 [v13.6.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.0) [ 问题 #2226](https://github.com/manticoresoftware/manticoresearch/issues/2226) 支持在 PHRASE、PROXIMITY 和 QUORUM 操作符中显式使用 '|'（或）。
* 🆕 [v13.5.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.5.0) [ PR #3591](https://github.com/manticoresoftware/manticoresearch/pull/3591) 查询中自动生成嵌入（正在进行中，尚未达到生产就绪状态）。
* 🆕 [v13.4.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.4.0) [ PR #3585](https://github.com/manticoresoftware/manticoresearch/pull/3585) 修正逻辑，优先使用配置中的 buddy_path 的 buddy 线程数，而非守护进程值。
* 🆕 [v13.3.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.3.0) [ PR #3577](https://github.com/manticoresoftware/manticoresearch/pull/3577) 支持使用本地分布式表进行连接。
* 🆕 [#3647](https://github.com/manticoresoftware/manticoresearch/issues/3647) 添加对 Debian 13 “Trixie”的支持。

### 错误修复
* 🪲 [v13.6.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.7) [ 问题 #3524](https://github.com/manticoresoftware/manticoresearch/issues/3524) 修正了在行存储中保存生成的嵌入的问题。
* 🪲 [v13.6.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.6) [ 问题 #3563](https://github.com/manticoresoftware/manticoresearch/issues/3563) 修复了 Sequel Ace 和其他集成因 “未知 sysvar” 错误失败的问题。
* 🪲 [v13.6.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.5) [ 问题 #3467](https://github.com/manticoresoftware/manticoresearch/issues/3467) 修复了 DBeaver 和其他集成因 “未知 sysvar” 错误失败的问题。
* 🪲 [v13.6.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.4) [ 问题 #3524](https://github.com/manticoresoftware/manticoresearch/issues/3524) 修复了多字段嵌入连接的问题；同时修复了查询中嵌入生成的问题。
* 🪲 [v13.6.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.3) [ 问题 #3641](https://github.com/manticoresoftware/manticoresearch/issues/3641) 修复了 13.6.0 版本中短语除第一个括号关键词外丢失所有括号关键词的错误。
* 🪲 [v13.6.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.2) 修复了 transform_phrase 中的内存泄漏。
* 🪲 [v13.6.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.1) 修复了 13.6.0 版本中的内存泄漏。
* 🪲 [v13.5.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.5.2) [ 问题 #3651](https://github.com/manticoresoftware/manticoresearch/issues/3651) 修复了更多与全文搜索模糊测试相关的问题。
* 🪲 [v13.5.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.5.1) [ 问题 #3560](https://github.com/manticoresoftware/manticoresearch/issues/3560) 修复了带有 KNN 数据时 OPTIMIZE TABLE 可能无限挂起的问题。
* 🪲 [v13.4.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.4.2) [ 问题 #2544](https://github.com/manticoresoftware/manticoresearch/issues/2544) 修复了添加 float_vector 列时可能损坏索引的问题。
* 🪲 [v13.4.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.4.1) [ 问题 #3651](https://github.com/manticoresoftware/manticoresearch/issues/3651) 添加全文解析的模糊测试，并修复测试中发现的若干问题。
* 🪲 [v13.3.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.3.1) [ 问题 #3583](https://github.com/manticoresoftware/manticoresearch/issues/3583) 修复使用复杂布尔过滤器结合高亮时的崩溃问题。
* 🪲 [v13.2.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.7) [ 问题 #3481](https://github.com/manticoresoftware/manticoresearch/issues/3481) 修复在同时使用 HTTP 更新、分布式表和错误的复制集群时的崩溃。
* 🪲 [v13.2.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.6) [ PR #3567](https://github.com/manticoresoftware/manticoresearch/pull/3567) 更新 manticore-backup 依赖至版本 1.9.6。
* 🪲 [v13.2.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.5) [ PR #3565](https://github.com/manticoresoftware/manticoresearch/pull/3565) 修正 CI 设置以提升 Docker 镜像兼容性。
* 🪲 [v13.2.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.4) 修正长词令牌的处理。一些特殊令牌（如正则表达式模式）可能生成过长的词，因此现在在使用前会进行缩短。

## 版本 13.2.3
**发布于**：2025年7月8日

### 重大变更
* ⚠️ [v13.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.0.0) 更新了 KNN 库 API，支持空的 [float_vector](https://manual.manticoresearch.com/Creating_a_table/Data_types#Float-vector) 值。此更新未更改数据格式，但提升了 Manticore Search / MCL API 版本。
* ⚠️ [v12.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/12.0.0) [ PR #3516](https://github.com/manticoresoftware/manticoresearch/pull/3516) 修复了 KNN 索引训练和构建期间源和目标行 ID 错误的问题。此更新未更改数据格式，但提升了 Manticore Search / MCL API 版本。
* ⚠️ [v11.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.0.0) 新增了对量化、重评分和过采样等新的KNN向量搜索功能的支持。此版本更改了KNN数据格式和[KNN_DIST() SQL 语法](https://manual.manticoresearch.com/Searching/KNN?client=SQL#KNN-vector-search)。新版本可以读取旧数据，但旧版本无法读取新格式。

### 新特性和改进
* 🆕 [v13.2.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.0) [ PR #3549](https://github.com/manticoresoftware/manticoresearch/issues/3525) 修复了 `@@collation_database` 相关的问题，该问题导致与某些mysqldump版本不兼容
* 🆕 [v13.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.1.0) [ Issue #3489](https://github.com/manticoresoftware/manticoresearch/issues/3489) 修复了模糊搜索中阻止解析某些SQL查询的错误
* 🆕 [v12.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/12.1.0) [ Issue #3426](https://github.com/manticoresoftware/manticoresearch/issues/3426) 增加了对RHEL 10操作系统的支持
* 🆕 [v11.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.1.0) 新增在[KNN搜索](https://manual.manticoresearch.com/Searching/KNN#KNN-vector-search)中支持空浮点向量
* 🆕 [v10.2.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.0) [ Issue #3252](https://github.com/manticoresoftware/manticoresearch/issues/3252) [log_level](https://manual.manticoresearch.com/Server_settings/Setting_variables_online#SET) 现在也调整 Buddy 的日志详细级别

### Bug 修复
* 🪲 [v13.2.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.3) [ PR #3556](https://github.com/manticoresoftware/manticoresearch/pull/3556) 修复了JSON查询中 “oversampling” 选项的解析问题
* 🪲 [v13.2.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.2) 通过移除Boost stacktrace的使用修复了Linux和FreeBSD上的崩溃日志记录问题
* 🪲 [v13.2.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.1) [ Issue #3298](https://github.com/manticoresoftware/manticoresearch/issues/3298) 修复了容器内运行时的崩溃日志记录问题
* 🪲 [v12.0.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/12.0.2) 通过以微秒计数改进了每表统计数据的准确性
* 🪲 [v12.0.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/12.0.1) [ PR #3522](https://github.com/manticoresoftware/manticoresearch/pull/3522) 修复了连接查询中通过MVA分面时的崩溃
* 🪲 [v11.0.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.0.3) [ Issue #3502](https://github.com/manticoresoftware/manticoresearch/issues/3502) 修复了与向量搜索量化相关的崩溃
* 🪲 [v11.0.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.0.2) [ Issue #3493](https://github.com/manticoresoftware/manticoresearch/issues/3493) 将 `SHOW THREADS` 显示的CPU利用率改为整数
* 🪲 [v11.0.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.0.1) 修复了列存和二级库的路径
* 🪲 [v10.2.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.7) 添加对MCL 5.0.5的支持，包括修正了嵌入库文件名的问题
* 🪲 [v10.2.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.6) [ Issue #3469](https://github.com/manticoresoftware/manticoresearch/issues/3469) 针对问题#3469应用了另一个修复
* 🪲 [v10.2.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.4) [ Issue #3469](https://github.com/manticoresoftware/manticoresearch/issues/3469) 修复了HTTP请求中包含bool查询时返回空结果的问题
* 🪲 [v10.2.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.5) 更改了嵌入库的默认文件名，并增加了对KNN向量中 'from' 字段的检查
* 🪲 [v10.2.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.3) [ PR #3464](https://github.com/manticoresoftware/manticoresearch/pull/3464) 将Buddy更新至3.30.2版本，包含[关于内存使用和错误日志的PR #565](https://github.com/manticoresoftware/manticoresearch-buddy/pull/565)
* 🪲 [v10.2.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.2) [ Issue #3470](https://github.com/manticoresoftware/manticoresearch/pull/3470) 修复了JOIN查询中的JSON字符串过滤、空过滤器及排序问题
* 🪲 [v10.2.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.1) 修复了 `dist/test_kit_docker_build.sh` 脚本中导致Buddy提交丢失的bug
* 🪲 [v10.1.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.4) 修复了连接查询中按MVA分组时的崩溃
* 🪲 [v10.1.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.3) [ Issue #3434](https://github.com/manticoresoftware/manticoresearch/issues/3434) 修复了过滤空字符串的bug
* 🪲 [v10.1.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.2) [ PR #3452](https://github.com/manticoresoftware/manticoresearch/pull/3452) 将Buddy更新至3.29.7版本，解决了[Buddy #507 - 带模糊搜索的多查询请求错误](https://github.com/manticoresoftware/manticoresearch-buddy/issues/507)和[Buddy #561 - 指标速率修复](https://github.com/manticoresoftware/manticoresearch-buddy/pull/561)，这是[Helm Release 10.1.0](https://github.com/manticoresoftware/manticoresearch-helm/releases/tag/manticoresearch-10.1.0)所需的更新。
* 🪲 [v10.1.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.1)  更新 Buddy 至版本 3.29.4，解决了 [#3388 - "未定义的数组键 'Field'"](https://github.com/manticoresoftware/manticoresearch/issues/3388) 和 [Buddy #547 - layouts='ru' 可能不起作用](https://github.com/manticoresoftware/manticoresearch-buddy/issues/547)

## 版本 10.1.0
**发布时间**：2025年6月9日

该版本包含新功能更新，一个破坏性变更以及大量稳定性改进和错误修复。更改重点在于增强监控能力、改进搜索功能以及修复影响系统稳定性和性能的各种关键问题。

**从版本 10.1.0 起，不再支持 CentOS 7。建议用户升级到受支持的操作系统。**

### 破坏性变更
* ⚠️ [v10.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.0.0) [问题 #540](https://github.com/manticoresoftware/manticoresearch-buddy/issues/540) 破坏性变更：将 [模糊搜索](Searching/Spell_correction.md#Fuzzy-Search) 的默认 `layouts=''`

### 新功能和改进
* 🆕 [v10.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.0) [问题 #537](https://github.com/manticoresoftware/manticoresearch-buddy/issues/537) 添加内置 [Prometheus 导出器](Node_info_and_management/Node_status.md#Prometheus-Exporter)
* 🆕 [v9.8.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.8.0) [问题 #3409](https://github.com/manticoresoftware/manticoresearch/issues/3409) 添加 [ALTER TABLE tbl REBUILD KNN](Updating_table_schema_and_settings.md#Rebuilding-a-KNN-index)
* 🆕 [v9.7.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.7.0) [问题 #1778](https://github.com/manticoresoftware/manticoresearch/issues/1778) 添加自动嵌入生成（尚未正式发布，代码在主分支，但仍需更多测试）
* 🆕 [v9.6.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.6.0)  KNN API 版本提升以支持自动嵌入
* 🆕 [v9.5.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.0) [问题 #1894](https://github.com/manticoresoftware/manticoresearch/issues/1894) 改进集群恢复：周期性保存 `seqno`，加快节点崩溃后的重启速度
* 🆕 [v9.4.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.4.0) [问题 #2400](https://github.com/manticoresoftware/manticoresearch/issues/2400) 支持最新的 [Logstash](Integration/Logstash.md#Integration-with-Logstash) 与 Beats

### 错误修复
* 🪲 [v10.0.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.0.1)  修复词形处理：用户定义的形式现在覆盖自动生成的形式；向测试22添加测试用例
* 🪲 [v9.8.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.8.2)  修复：再次更新 deps.txt 以包含与嵌入库相关的 MCL 打包修复
* 🪲 [v9.8.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.8.1)  修复：更新 deps.txt，修复 MCL 和嵌入库的打包问题
* 🪲 [v9.7.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.7.3) [问题 #3306](https://github.com/manticoresoftware/manticoresearch/issues/3306) 修复索引时信号11导致的崩溃
* 🪲 [v9.7.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.7.2) [问题 #3109](https://github.com/manticoresoftware/manticoresearch/issues/3109) 修复不存在的 `@@variables` 总是返回0的问题
* 🪲 [v9.7.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.7.1) [问题 #3377](https://github.com/manticoresoftware/manticoresearch/issues/3377) 修复与 [remove_repeats()](Functions/Searching_and_ranking_functions.md#REMOVE_REPEATS%28%29) 相关的崩溃
* 🪲 [v9.6.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.6.3) [PR #3411](https://github.com/manticoresoftware/manticoresearch/pull/3411) 修复：对遥测指标使用动态版本检测
* 🪲 [v9.6.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.6.2)  修复：[SHOW VERSION](Node_info_and_management/SHOW_VERSION.md#SHOW-VERSION) 输出中的小错误
* 🪲 [v9.6.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.6.1)  修复：创建带有 KNN 属性但无模型的表时崩溃
* 🪲 [v9.5.16](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.16) [问题 #3342](https://github.com/manticoresoftware/manticoresearch/issues/3342) 修复 `SELECT ... FUZZY=0` 未必总是禁用模糊搜索的问题
* 🪲 [v9.5.15](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.15) [PR #3397](https://github.com/manticoresoftware/manticoresearch/pull/3397) 添加对 [MCL](https://github.com/manticoresoftware/columnar) 4.2.2 的支持；修复旧存储格式错误
* 🪲 [v9.5.14](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.14) [问题 #3392](https://github.com/manticoresoftware/manticoresearch/issues/3392) 修复 HTTP JSON 回复字符串处理错误
* 🪲 [v9.5.13](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.13) [问题 #3356](https://github.com/manticoresoftware/manticoresearch/issues/3356) 修复复杂全文查询用例（common-sub-term）崩溃
* 🪲 [v9.5.12](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.12)  修复磁盘块自动刷新错误消息中的拼写错误
* 🪲 [v9.5.11](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.11) [问题 #3195](https://github.com/manticoresoftware/manticoresearch/issues/3195) 改进 [自动磁盘块刷新](Securing_and_compacting_a_table/Flushing_RAM_chunk_to_disk.md#Flushing-RAM-chunk-to-disk)：优化运行时跳过保存
* 🪲 [v9.5.10](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.10) [ Issue #3313](https://github.com/manticoresoftware/manticoresearch/issues/3313) 修复了使用 [indextool](Miscellaneous_tools.md#indextool) 对 RT 表中所有磁盘块的重复 ID 检查问题
* 🪲 [v9.5.9](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.9) [ Issue #3132](https://github.com/manticoresoftware/manticoresearch/issues/3132) 在 JSON API 中添加了对 `_random` 排序的支持
* 🪲 [v9.5.8](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.8) [ Issue #3382](https://github.com/manticoresoftware/manticoresearch/issues/3382) 修复了无法通过 JSON HTTP API 使用 uint64 文档 ID 的问题
* 🪲 [v9.5.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.7) [ Issue #3385](https://github.com/manticoresoftware/manticoresearch/issues/3385) 修复了按 `id != value` 过滤时产生不正确结果的问题
* 🪲 [v9.5.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.6) [ PR #538](https://github.com/manticoresoftware/manticoresearch-buddy/pull/538) 修复了某些情况下模糊匹配的关键错误
* 🪲 [v9.5.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.5) [ Issue #3199](https://github.com/manticoresoftware/manticoresearch/issues/3199) 修正了 Buddy HTTP 查询参数中的空格解码（例如 `%20` 和 `+`）
* 🪲 [v9.5.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.4) [ Issue #3133](https://github.com/manticoresoftware/manticoresearch/issues/3133) 修复了 facets 搜索中 `json.field` 排序错误
* 🪲 [v9.5.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.3) [ Issue #3091](https://github.com/manticoresoftware/manticoresearch/issues/3091) 修复了 SQL 与 JSON API 中定界符搜索结果不一致的问题
* 🪲 [v9.5.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.2) [ Issue #2819](https://github.com/manticoresoftware/manticoresearch/issues/2819) 提升性能：将分布式表的 `DELETE FROM` 替换为 `TRUNCATE`
* 🪲 [v9.5.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.1) [ Issue #3080](https://github.com/manticoresoftware/manticoresearch/issues/3080) 修复了使用 JSON 属性过滤别名 `geodist()` 导致的崩溃问题

## 版本 9.3.2
发布：2025年5月2日

本次发布包含多项错误修复和稳定性改进，更好的表使用跟踪，以及内存和资源管理的增强。

❤️ 特别感谢 [@cho-m](https://github.com/cho-m) 修复了与 Boost 1.88.0 的构建兼容性问题，感谢 [@benwills](https://github.com/benwills) 改进了关于 `stored_only_fields` 的文档。

* 🪲 [v9.3.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.3.2) 修复了“显示线程”列将 CPU 活动显示为浮点数而非字符串的问题；同时修复了 PyMySQL 结果集解析时因数据类型错误导致的错误。
* 🪲 [v9.3.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.3.1) [ Issue #3343](https://github.com/manticoresoftware/manticoresearch/issues/3343) 修复了优化过程被中断时遗留的 `tmp.spidx` 文件。
* 🆕 [v9.3.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.3.0) [ PR #3337](https://github.com/manticoresoftware/manticoresearch/pull/3337) 添加了每个表的命令计数器和详细的表使用统计。
* 🪲 [v9.2.39](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.39) [ Issue #3236](https://github.com/manticoresoftware/manticoresearch/issues/3236) 修复：通过移除复杂的块更新防止表损坏。在串行工作线程中使用等待函数导致串行处理失败，可能损坏表。
重实现了自动刷新。移除外部轮询队列以避免不必要的表锁。新增“小表”条件：如果文档数低于“小表限制”（8192）且未使用二级索引（SI），则跳过刷新。

* 🪲 [v9.2.38](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.38) 修复：跳过为使用 `ALL`/`ANY` 字符串列表的过滤器创建二级索引（SI），不影响 JSON 属性。
* 🪲 [v9.2.37](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.37) [ Issue #2898](https://github.com/manticoresoftware/manticoresearch/issues/2898) 为系统表添加了反引号支持。
* 🪲 [v9.2.36](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.36) 修复：在遗留代码中为集群操作使用占位符。在解析器中，我们现在明确区分了表名和集群名字段。
* 🪲 [v9.2.35](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.35) 修复了解除单引号 `'` 转义时的崩溃问题。
* 🪲 [v9.2.34](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.34) [ Issue #3090](https://github.com/manticoresoftware/manticoresearch/issues/3090) 修复：大型文档 ID 处理问题（之前可能无法正确查找）。
* 🪲 [v9.2.33](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.33) 修复：位向量尺寸使用无符号整数。
* 🪲 [v9.2.32](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.32) 修复：合并过程中的峰值内存使用降低。docid 到 rowid 的查找现在每个文档使用 12 字节而非 16 字节。例如：处理 20 亿文档时 RAM 从 36GB 降至 24GB。
* 🪲 [v9.2.31](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.31) [ Issue #3238](https://github.com/manticoresoftware/manticoresearch/issues/3238) 修复了大规模实时表中 `COUNT(*)` 结果不正确的问题。
* 🪲 [v9.2.30](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.30) 修复：清零字符串属性时的未定义行为。
* 🪲 [v9.2.29](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.29) 小修正：改进了警告文本。
* 🪲 [v9.2.28](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.28) [ Issue #3290](https://github.com/manticoresoftware/manticoresearch/issues/3290) 改进：增强了 `indextool --buildidf`
* 🪲 [v9.2.27](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.27) [ Issue #3032](https://github.com/manticoresoftware/manticoresearch/issues/3032) 通过 Kafka 集成，现在可以为特定的 Kafka 分区创建源。
* 🪲 [v9.2.26](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.26) [ Issue #3301](https://github.com/manticoresoftware/manticoresearch/issues/3301) 修复：`ORDER BY` 和 `WHERE` 语句中针对 `id` 使用可能引起 OOM（内存溢出）错误。
* 🪲 [v9.2.25](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.25) [ Issue #3171](https://github.com/manticoresoftware/manticoresearch/issues/3171) 修复：在 RT 表上使用包含多个 JSON 属性的 grouper 时，多磁盘块导致的段错误崩溃。
* 🪲 [v9.2.24](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.24) [ Issue #3246](https://github.com/manticoresoftware/manticoresearch/issues/3246) 修复：RAM 块刷新后，`WHERE string ANY(...)` 查询失败。
* 🪲 [v9.2.23](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.23) [ PR #518](https://github.com/manticoresoftware/manticoresearch-buddy/pull/518) 轻微的自动分片语法改进。
* 🪲 [v9.2.22](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.22) [ Issue #2763](https://github.com/manticoresoftware/manticoresearch/issues/2763) 修复：使用 `ALTER TABLE` 时全局 idf 文件未被加载。
* 🪲 [v9.2.21](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.21) 修复：全局 idf 文件可能很大。我们现在更早释放表以避免持有不必要的资源。
* 🪲 [v9.2.20](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.20) [ PR #3277](https://github.com/manticoresoftware/manticoresearch/pull/3277) 改进：更好地验证分片选项。

* 🪲 [v9.2.19](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.19) [ PR #3275](https://github.com/manticoresoftware/manticoresearch/pull/3275) 修复：兼容 Boost 1.88.0 的构建问题。
* 🪲 [v9.2.18](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.18) [ Issue #3228](https://github.com/manticoresoftware/manticoresearch/issues/3228) 修复：分布式表创建时的崩溃（无效指针问题）。

* 🪲 [v9.2.17](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.17) [ PR #3272](https://github.com/manticoresoftware/manticoresearch/pull/3272) 修复：多行模糊 `FACET` 问题。
* 🪲 [v9.2.16](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.16) [ Issue #3063](https://github.com/manticoresoftware/manticoresearch/issues/3063) 修复：使用 `GEODIST` 函数时距离计算的错误。
* 🪲 [v9.2.15](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.15) [ Issue #3027](https://github.com/manticoresoftware/manticoresearch/issues/3027) 轻微改进：支持 Elastic 的 `query_string` 过滤格式。

## 版本 9.2.14
发布时间：2025年3月28日

### 小幅更改
* [提交 ](https://github.com/manticoresoftware/manticoresearch/commit/fe9473dc3f4fcfa0eaae748b538fe853f4ce078b) 实现了 `--mockstack` 标志以计算递归操作的栈需求。新的 `--mockstack` 模式分析并报告递归表达式求值、模式匹配操作、过滤器处理所需的栈大小。计算的栈需求会输出到控制台，用于调试和优化。
* [Issue #3058](https://github.com/manticoresoftware/manticoresearch/pull/3058) 默认启用了 [boolean_simplify](Searching/Options.md#boolean_simplify)。
* [Issue #3172](https://github.com/manticoresoftware/manticoresearch/issues/3172) 新增配置选项：`searchd.kibana_version_string`，在使用特定版本的 Kibana 或 OpenSearch Dashboards（期望特定 Elasticsearch 版本）时非常有用。
* [Issue #3211](https://github.com/manticoresoftware/manticoresearch/issues/3211) 修复了 [CALL SUGGEST](Searching/Spell_correction.md#CALL-QSUGGEST,-CALL-SUGGEST) 支持 2 字符词。
* [Issue #490](https://github.com/manticoresoftware/manticoresearch-buddy/issues/490) 改进了 [模糊搜索](Searching/Spell_correction.md#Fuzzy-Search)：以前如果存在另一个匹配文档，搜索“def ghi”有时找不到“defghi”。
* ⚠️ 重大变更 [Issue #3165](https://github.com/manticoresoftware/manticoresearch/issues/3165) 将部分 HTTP JSON 响应中的 `_id` 改为 `id` 以保持一致性。请务必相应更新您的应用。
* ⚠️ 重大变更 [Issue #3186](https://github.com/manticoresoftware/manticoresearch/issues/3186) 在集群加入时增加了 `server_id` 检查，确保每个节点都有唯一 ID。`JOIN CLUSTER` 操作如果加入节点与集群中现有节点 `server_id` 重复，将返回错误信息。要解决此问题，请确保复制集群中每个节点的 [server_id](Server_settings/Searchd.md#server_id) 唯一。您可以在配置文件的 "searchd" 部分更改默认 [server_id](Server_settings/Searchd.md#server_id) 为唯一值，然后再尝试加入集群。此更改更新了复制协议。如果您正在运行复制集群，需要：
  - 首先，干净地停止所有节点
  - 然后，使用 Linux 工具 `manticore_new_cluster`，通过 `--new-cluster` 参数启动最后停止的节点。
  - 更多详情请阅读关于 [重启集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster) 的内容。

### Bug 修复
* [Commit 6fda](https://github.com/manticoresoftware/manticoresearch/commit/6fdad3923dd6953f4b781943eed5ec5c28b7f808) 修复了等待后调度器丢失导致的崩溃；现在，像 `serializer` 这样的特定调度器能够被正确恢复。
* [Commit c333](https://github.com/manticoresoftware/manticoresearch/commit/c333b277d4a504de5fe298e9ab570a9f9ea31e0a) 修复了一个BUG，即无法在 `ORDER BY` 子句中使用右连接表的权重。
* [Issue #2644](https://github.com/manticoresoftware/manticoresearch/issues/2644) gcc 14.2.0：修正了 `lower_bound` 调用时对 `const knn::DocDist_t*&` 的错误。❤️ 感谢 [@Azq2](https://github.com/Azq2) 的 PR。
* [Issue #3018](https://github.com/manticoresoftware/manticoresearch/issues/3018) 修复了自动模式插入时处理大写表名的问题。
* [Issue #3119](https://github.com/manticoresoftware/manticoresearch/issues/3119) 修复了解码无效 base64 输入时的崩溃。
* [Issue #3121](https://github.com/manticoresoftware/manticoresearch/issues/3121) 修复了两个与 KNN 索引有关的 `ALTER` 相关问题：浮点向量现在保留其原始维度，并且 KNN 索引能够正确生成。
* [Issue #3123](https://github.com/manticoresoftware/manticoresearch/issues/3123) 修复了在空 JSON 列上构建二级索引时的崩溃。
* [Issue #3138](https://github.com/manticoresoftware/manticoresearch/issues/3138) 修复了由重复条目导致的崩溃。
* [Issue #3151](https://github.com/manticoresoftware/manticoresearch/issues/3151) 修复了 `fuzzy=1` 选项无法与 `ranker` 或 `field_weights` 一起使用的问题。
* [Issue #3163](https://github.com/manticoresoftware/manticoresearch/issues/3163) 修复了 `SET GLOBAL timezone` 无效的BUG。
* [Issue #3181](https://github.com/manticoresoftware/manticoresearch/issues/3181) 修复了使用大于 2^63 的 ID 时文本字段值可能丢失的问题。
* [Issue #3189](https://github.com/manticoresoftware/manticoresearch/issues/3189) 修复了 `UPDATE` 语句现在能够正确遵守 `query_log_min_msec` 设置。
* [Issue #3247](https://github.com/manticoresoftware/manticoresearch/issues/3247) 修复了保存实时磁盘块时的竞态条件，该问题可能导致 `JOIN CLUSTER` 失败。


## 版本 7.4.6
发布日期：2025年2月28日

### 主要变更
* [Issue #832](https://github.com/manticoresoftware/manticoresearch/issues/832) 集成了 [Kibana](Integration/Kibana.md)，以便更轻松高效地进行数据可视化。

### 次要变更
* [Issue #1727](https://github.com/manticoresoftware/manticoresearch/issues/1727) 修正了 arm64 和 x86_64 之间的浮点精度差异。
* [Issue #2995](https://github.com/manticoresoftware/manticoresearch/issues/2995) 实现了对 join 批处理的性能优化。
* [Issue #3039](https://github.com/manticoresoftware/manticoresearch/issues/3039) 实现了直方图中 EstimateValues 的性能优化。
* [Issue #3099](https://github.com/manticoresoftware/manticoresearch/issues/3099) 添加了对 Boost 1.87.0 的支持。❤️ 感谢 [@cho-m](https://github.com/cho-m) 的 PR。
* [Issue #77](https://github.com/manticoresoftware/columnar/issues/77) 优化了创建多值过滤器时的块数据重用；为属性元数据添加了最小/最大值；基于最小/最大值实现了过滤值的预过滤功能。

### Bug 修复
* [Commit 73ac](https://github.com/manticoresoftware/manticoresearch/commit/73ac22f358a7a0e734b332c3943c86e6294c06c3) 修正了当使用左右表属性时联接查询中表达式的处理；修复了右表的 index_weights 选项。
* [Issue #2915](https://github.com/manticoresoftware/manticoresearch/issues/2915) 在 `SELECT ... JOIN` 查询中使用 `avg()` 可能导致结果错误；现已修复。
* [Issue #2996](https://github.com/manticoresoftware/manticoresearch/issues/2996) 修复了启用 join 批处理时隐式截止导致的错误结果集。
* [Issue #3031](https://github.com/manticoresoftware/manticoresearch/issues/3031) 修复了在守护进程关闭时当正在进行活动块合并时的崩溃。
* [Issue #3037](http://github.com/manticoresoftware/manticoresearch/issues/3037) 修复了 `IN(...)` 可能产生错误结果的问题。
* [Issue #3038](https://github.com/manticoresoftware/manticoresearch/issues/3038) 在版本 7.0.0 中设置 `max_iops` / `max_iosize` 会降低索引性能，现已修复。
* [Issue #3042](https://github.com/manticoresoftware/manticoresearch/issues/3042) 修复了 join 查询缓存中的内存泄漏。
* [Issue #3052](https://github.com/manticoresoftware/manticoresearch/issues/3052) 修复了联合 JSON 查询中查询选项的处理。
* [Issue #3054](https://github.com/manticoresoftware/manticoresearch/issues/3054) 修复了 ATTACH TABLE 命令的问题。
* [Issue #3079](https://github.com/manticoresoftware/manticoresearch/issues/3079) 修正了错误消息的不一致性。
* [Issue #3087](https://github.com/manticoresoftware/manticoresearch/issues/3087) 针对每个表设置 `diskchunk_flush_write_timeout=-1` 未能禁用索引刷新，现已修复。
* [Issue #3088](https://github.com/manticoresoftware/manticoresearch/issues/3088) 解决了批量替换大ID后产生的重复条目问题。
* [Issue #3126](https://github.com/manticoresoftware/manticoresearch/issues/3126) 修复了全文查询使用单个 `NOT` 操作符和表达式排序器时导致守护进程崩溃的问题。
* [Issue #3128](https://github.com/manticoresoftware/manticoresearch/pull/3128) 修复了 CJSON 库中的潜在漏洞。❤️ 感谢 [@tabudz](https://github.com/tabudz) 的 PR。

## 版本 7.0.0
发布日期：2025年1月30日

### 主要变更
* [Issue #1497](https://github.com/manticoresoftware/manticoresearch/issues/1497) 新增了 [模糊搜索](Searching/Spell_correction.md#Fuzzy-Search) 和 [自动补全](Searching/Autocomplete.md#CALL-AUTOCOMPLETE) 功能，以便更轻松搜索。
* [Issue #1500](https://github.com/manticoresoftware/manticoresearch/issues/1500) [与 Kafka 集成](Integration/Kafka.md#Syncing-from-Kafka)。
* [Issue #1928](https://github.com/manticoresoftware/manticoresearch/issues/1928) 引入了 [JSON 的二级索引](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#json_secondary_indexes)。
* [Issue #2361](https://github.com/manticoresoftware/manticoresearch/issues/2361) 更新期间的更新和搜索不再被块合并阻塞。
* [Issue #2787](https://github.com/manticoresoftware/manticoresearch/issues/2787) 为 RT 表自动执行 [磁盘块刷新](Server_settings/Searchd.md#diskchunk_flush_write_timeout)，以提升性能；现在，我们自动将 RAM 块刷新到磁盘块，避免了 RAM 块缺少优化可能导致的性能问题，有时这会根据块大小引发不稳定。
* [Issue #2811](https://github.com/manticoresoftware/manticoresearch/issues/2811) 提供了更简便分页的 [滚动](Searching/Pagination.md#Scroll-Search-Option) 选项。
* [Issue #931](https://github.com/manticoresoftware/manticoresearch/issues/931) 集成了 [Jieba](https://github.com/fxsjy/jieba) 以提升 [中文分词](Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md)效果。

### 次要变更
* ⚠️ 重大变更 [Issue #1111](https://github.com/manticoresoftware/manticoresearch/issues/1111) 修复了 RT 表中对 `global_idf` 的支持。需要重新创建表。
* ⚠️ 重大变更 [Issue #2103](https://github.com/manticoresoftware/manticoresearch/issues/2103) 从内部 `cjk` 字符集移除了泰文字符。请相应更新字符集定义：如果你使用了 `cjk,non_cjk` 并且泰文字符对你很重要，改为使用 `cjk,thai,non_cjk`，或使用 `cont,non_cjk`，其中 `cont` 是所有连续书写语言（即 `cjk` + `thai`）的新标识。通过 [ALTER TABLE](Updating_table_schema_and_settings.md#Updating-table-FT-settings-in-RT-mode) 修改现有表。
* ⚠️ 重大变更 [Issue #2468](https://github.com/manticoresoftware/manticoresearch/issues/2468) [CALL SUGGEST / QSUGGEST](Searching/Spell_correction.md#CALL-QSUGGEST,-CALL-SUGGEST) 现支持分布式表。这提高了主/代理协议版本。如果你在分布式环境中运行 Manticore Search 多实例，请先升级代理，再升级主节点。
* ⚠️ 重大变更 [Issue #2889](https://github.com/manticoresoftware/manticoresearch/issues/2889) PQ [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META-for-PQ-tables) 的列名由 `Name` 改为 `Variable name`。
* ⚠️ 重大变更 [Issue #879](https://github.com/manticoresoftware/manticoresearch/issues/879) 引入了 [每表二进制日志](Logging/Binary_logging.md#Per-table-binary-logging-configuration)，新增选项：[binlog_common](Logging/Binary_logging.md#Binary-logging-strategies)、[binlog](Logging/Binary_logging.md#Per-table-binary-logging-configuration) 用于 `create table` / `alter table`。升级到新版本前，需要对 Manticore 实例进行干净关闭。
* ⚠️ 重大变更 [Issue #1789](https://github.com/manticoresoftware/manticoresearch/issues/1789) 修正了节点加入集群时的错误复制协议版本错误消息。此改动更新了复制协议。如果你运行复制集群，需要：
  - 首先，干净停止所有节点
  - 然后，使用 Linux 工具 `manticore_new_cluster` 启动最后停止的节点并加上 `--new-cluster` 参数。
  - 更多细节请参见 [重启集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster)。
* ⚠️ 重大变更 [Issue #2308](https://github.com/manticoresoftware/manticoresearch/issues/2308) 添加了在 [`ALTER CLUSTER ADD` 和 `DROP`](Creating_a_cluster/Setting_up_replication/Adding_and_removing_a_table_from_a_replication_cluster.md#Adding-and-removing-a-table-from-a-replication-cluster) 中支持多表的功能。此改动也影响复制协议。有关处理此更新的指南，请参阅前一节。
* [Issue #2997](https://github.com/manticoresoftware/manticoresearch/issues/2997) 修复了 Macos 平台上的 dlopen 问题。
* [Commit 4954](https://github.com/manticoresoftware/manticoresearch/commit/4954b5de7341a29902a9b8fbb9a040f7942c77c4) 修改了带 KNN 索引表的 OPTIMIZE TABLE 默认截断阈值，以提升搜索性能。
* [Commit cfc8](https://github.com/manticoresoftware/manticoresearch/commit/cfc87ecb6e33a8163c2235243b6b40e699dbf526) 为 `FACET` 和 `GROUP BY` 中的 `ORDER BY` 添加了 `COUNT(DISTINCT)` 支持。
* [Issue #1103](https://github.com/manticoresoftware/manticoresearch/issues/1103) 改进了 [日志记录](Logging/Server_logging.md#Server-logging)中块合并的清晰度。
* [Issue #1130](https://github.com/manticoresoftware/manticoresearch/issues/1130) 添加了对 [DBeaver](Integration/DBeaver.md) 的支持。
* [Issue #1546](https://github.com/manticoresoftware/manticoresearch/issues/1546) 为 [POLY2D()](Functions/Geo_spatial_functions.md#POLY2D%28%29)/[GEOPOLY2D()](Functions/Geo_spatial_functions.md#GEOPOLY2D%28%29) 函数实现了二级索引。
* [Issue #1630](https://github.com/manticoresoftware/manticoresearch/issues/1630) HTTP 请求现支持 `Content-Encoding: gzip`。
* [Issue #1831](https://github.com/manticoresoftware/manticoresearch/issues/1831) 新增 `SHOW LOCKS` 命令。
* [Issue #2187](https://github.com/manticoresoftware/manticoresearch/issues/2197) 允许 Buddy 请求绕过守护进程的 [searchd.max_connections](Server_settings/Searchd.md#max_connections) 限制。
* [Issue #2208](https://github.com/manticoresoftware/manticoresearch/issues/2208) 增加了通过 JSON HTTP 接口连接表的支持。
* [Issue #2235](https://github.com/manticoresoftware/manticoresearch/issues/2235) 通过 Buddy 记录成功处理的查询，并保持其原始形式。
* [问题 #2249](https://github.com/manticoresoftware/manticoresearch/issues/2249) 增加了用于复制表运行 `mysqldump` 的特殊模式。
* [问题 #2268](https://github.com/manticoresoftware/manticoresearch/issues/2268) 改进了 `CREATE TABLE` 和 `ALTER TABLE` 语句中外部文件复制时的重命名。
* [问题 #2402](https://github.com/manticoresoftware/manticoresearch/issues/2402) 更新了 [searchd.max_packet_size](Server_settings/Searchd.md#max_packet_size) 的默认值为128MB。
* [问题 #2419](https://github.com/manticoresoftware/manticoresearch/issues/2419) 添加了对 JSON 中 [IDF boost modifier](Searching/Full_text_matching/Operators.md#IDF-boost-modifier) 的支持，适用于["match"](Searching/Full_text_matching/Basic_usage.md#match)。
* [问题 #2430](https://github.com/manticoresoftware/manticoresearch/issues/2430) 增强了 [binlog](Logging/Binary_logging.md#Binary-logging) 写入同步机制以防止错误。
* [问题 #2458](https://github.com/manticoresoftware/manticoresearch/issues/2458) Windows 软件包中加入了 zlib 支持。
* [问题 #2479](https://github.com/manticoresoftware/manticoresearch/issues/2479) 添加了对 SHOW TABLE INDEXES 命令的支持。
* [问题 #2485](https://github.com/manticoresoftware/manticoresearch/issues/2485) 设置了 Buddy 回复的会话元数据。
* [问题 #2490](https://github.com/manticoresoftware/manticoresearch/issues/2490) 在兼容性端点支持了毫秒级聚合分辨率。
* [问题 #2500](https://github.com/manticoresoftware/manticoresearch/issues/2500) 修改了当复制启动失败时集群操作的错误信息。
* [问题 #2584](https://github.com/manticoresoftware/manticoresearch/pull/2584) 新增 [SHOW STATUS](Node_info_and_management/Node_status.md#Query-Time-Statistics) 中的性能指标：上一分钟、五分钟和十五分钟每种查询类型的最小/最大/平均/95%/99% 百分位统计。
* [问题 #2639](https://github.com/manticoresoftware/manticoresearch/pull/2639) 请求和响应中所有的 `index` 已替换为 `table`。
* [问题 #2643](https://github.com/manticoresoftware/manticoresearch/issues/2643) 在 HTTP `/sql` 端点的聚合结果中添加了 `distinct` 列。
* [问题 #268](https://github.com/manticoresoftware/manticoresearch-buddy/issues/268) 实现了从 Elasticsearch 导入数据类型的自动检测。
* [问题 #2744](https://github.com/manticoresoftware/manticoresearch/issues/2744) 为字符串 JSON 字段比较表达式添加了排序规则支持。
* [问题 #2752](https://github.com/manticoresoftware/manticoresearch/issues/2752) 添加了对选择列表中 `uuid_short` 表达式的支持。
* [问题 #2783](https://github.com/manticoresoftware/manticoresearch/issues/2783) Manticore Search 现在直接运行 Buddy，无需 `manticore-buddy` 包装器。
* [问题 #2785](https://github.com/manticoresoftware/manticoresearch/issues/2785) 针对缺失表和不支持插入操作的表分别提供了不同的错误信息。
* [问题 #2789](https://github.com/manticoresoftware/manticoresearch/issues/2789) OpenSSL 3 现在静态集成在 `searchd` 中。
* [问题 #2790](https://github.com/manticoresoftware/manticoresearch/issues/2790) 添加了 `CALL uuid_short` 语句，用于生成包含多个 `uuid_short` 值的序列。
* [问题 #2803](https://github.com/manticoresoftware/manticoresearch/issues/2803) 为 JOIN 操作中的右表添加了单独的选项。
* [问题 #2810](https://github.com/manticoresoftware/manticoresearch/issues/2810) 提升了 HTTP JSON 聚合性能，与 SphinxQL 的 `GROUP BY` 性能相当。
* [问题 #2854](https://github.com/manticoresoftware/manticoresearch/issues/2854) 在 Kibana 日期相关请求中添加了对 `fixed_interval` 的支持。
* [问题 #2909](https://github.com/manticoresoftware/manticoresearch/issues/2909) 实现了 JOIN 查询的批处理，大幅提升某些 JOIN 查询的性能，提升幅度达数百倍甚至上千倍。
* [问题 #2937](https://github.com/manticoresoftware/manticoresearch/issues/2937) 启用了在 fullscan 查询中使用加入表的权重。
* [问题 #2953](https://github.com/manticoresoftware/manticoresearch/issues/2953) 修正了 join 查询的日志记录。
* [问题 #337](https://github.com/manticoresoftware/manticoresearch-buddy/issues/337) 在非调试模式下隐藏了 Buddy 异常，不在 `searchd` 日志中显示。
* [问题 #2931](https://github.com/manticoresoftware/manticoresearch/issues/2931) 当用户设置了错误的复制监听端口时，守护进程以错误信息关闭。

### Bug 修复
* [提交 0c6b](https://github.com/manticoresoftware/manticoresearch/commit/0c6bdaf41ea3684e05952e1a8273893535b923f5) 修正：JOIN 查询中列数超过 32 列时返回结果不正确。
* [问题 #2335](https://github.com/manticoresoftware/manticoresearch/issues/2335) 解决了两个 JSON 属性在条件中使用时导致表连接失败的问题。
* [问题 #2338](https://github.com/manticoresoftware/manticoresearch/issues/2338) 修正了多查询中包含 [cutoff](Searching/Options.md#cutoff) 时 total_relation 计算不正确的问题。
* [问题 #2366](https://github.com/manticoresoftware/manticoresearch/issues/2366) 纠正了表连接中右表对 `json.string` 字段的过滤。
* [问题 #2406](https://github.com/manticoresoftware/manticoresearch/issues/2406) 允许在任何 POST HTTP JSON 端点（insert/replace/bulk）中使用 `null` 作为所有字段的值，此时会使用默认值。
* [问题 #2418](https://github.com/manticoresoftware/manticoresearch/issues/2418) 通过调整 [max_packet_size](Server_settings/Searchd.md#max_packet_size) 网络缓冲区的分配优化了内存使用。
* [问题 #2420](https://github.com/manticoresoftware/manticoresearch/issues/2420) 修正了通过 JSON 接口向 bigint 属性插入无符号整数的问题。
* [问题 #2422](https://github.com/manticoresoftware/manticoresearch/issues/2422) 修正了二级索引在启用 exclude filters 和 pseudo_sharding 时的正确性。
* [问题 #2423](https://github.com/manticoresoftware/manticoresearch/issues/2423) 解决了 [manticore_new_cluster](Starting_the_server/Manually.md#searchd-command-line-options) 中的一个错误。
* [问题 #2448](https://github.com/manticoresoftware/manticoresearch/issues/2448) 修复了在格式错误的 `_update` 请求时守护进程崩溃的问题。
* [问题 #2452](https://github.com/manticoresoftware/manticoresearch/issues/2452) 修复了直方图无法处理带有排除项的值过滤器的问题。
* [问题 #55](https://github.com/manticoresoftware/columnar/issues/55) 修复了针对分布式表的 [knn](Searching/KNN.md#KNN-vector-search) 查询问题。
* [问题 #68](https://github.com/manticoresoftware/columnar/issues/68) 增强了 columnar 访问器中对表编码的排除过滤器的处理。
* [提交 0eb1](https://github.com/manticoresoftware/manticoresearch/commit/0eb1579462013cb134bcaf74e06f6eeb0c2ecc4d) 修复了表达式解析器不遵守重新定义的 `thread_stack` 的问题。
* [提交 c304](https://github.com/manticoresoftware/manticoresearch/commit/c3041fd1d3e14773f3c5aa635343915a71f5cb95) 修复了克隆 columnar IN 表达式时的崩溃。
* [提交 edad](https://github.com/manticoresoftware/columnar/commit/edadc694c68d6022bdd13134263667430a42cc1d) 修复了位图迭代器中的反转问题，该问题导致崩溃。
* [提交 fc30](https://github.com/manticoresoftware/executor/commit/fc302ff1117b0b835a6f5a3c07057baf5fba14d9) 修复了一些 Manticore 包被 `unattended-upgrades` 自动移除的问题。
* [问题 #1019](https://github.com/manticoresoftware/manticoresearch/issues/1019) 改进了对 DbForge MySQL 工具查询的处理。
* [问题 #1107](https://github.com/manticoresoftware/manticoresearch/issues/1107) 修复了 `CREATE TABLE` 和 `ALTER TABLE` 中特殊字符的转义问题。❤️ 感谢 [@subnix](https://github.com/subnix) 的 PR。
* [问题 #116](https://github.com/manticoresoftware/manticoresearch-backup/issues/116#issuecomment-2216597206) 修复了在冻结索引中更新 blob 属性时的死锁问题。死锁是由于尝试解冻索引时锁冲突引起的，也可能导致 manticore-backup 失败。
* [问题 #1818](https://github.com/manticoresoftware/manticoresearch/issues/1818) `OPTIMIZE` 命令在表被冻结时现在会抛出错误。
* [问题 #2001](https://github.com/manticoresoftware/manticoresearch/issues/2001) 允许函数名用作列名。
* [问题 #2153](https://github.com/manticoresoftware/manticoresearch/issues/2153) 修复了查询表设置时因未知磁盘块导致守护进程崩溃的问题。
* [问题 #2184](https://github.com/manticoresoftware/manticoresearch/issues/2184) 修复了 `searchd` 在执行 `FREEZE` 和 `FLUSH RAMCHUNK` 后停止时挂起的问题。
* [问题 #2228](https://github.com/manticoresoftware/manticoresearch/issues/2228) 从保留字中移除了日期/时间相关的标记（和正则表达式）。
* [问题 #2255](https://github.com/manticoresoftware/manticoresearch/issues/2255) 修复了 `FACET` 查询中排序字段超过 5 个时崩溃的问题。
* [问题 #2265](https://github.com/manticoresoftware/manticoresearch/issues/2265) 修复了启用 `index_field_lengths` 时恢复 `mysqldump` 失败的问题。
* [问题 #2291](https://github.com/manticoresoftware/manticoresearch/issues/2291) 修复了执行 `ALTER TABLE` 命令时崩溃的问题。
* [问题 #2333](https://github.com/manticoresoftware/manticoresearch/issues/2333) 修复了 Windows 包中的 MySQL DLL，使索引器能正常工作。
* [问题 #2393](https://github.com/manticoresoftware/manticoresearch/issues/2393) 修复了 GCC 编译错误。❤️ 感谢 [@animetosho](https://github.com/animetosho) 的 PR。
* [问题 #2447](https://github.com/manticoresoftware/manticoresearch/issues/2447) 修复了 [_update](Data_creation_and_modification/Updating_documents/REPLACE.md?client=Elasticsearch-like+partial) 中的转义问题。
* [问题 #2460](https://github.com/manticoresoftware/manticoresearch/issues/2460) 修复了索引器在声明多个同名属性或字段时崩溃的问题。
* [问题 #2467](https://github.com/manticoresoftware/manticoresearch/issues/2467) 解决了“compat”搜索相关端点中，嵌套 bool 查询变换错误导致守护进程崩溃的问题。
* [问题 #2493](https://github.com/manticoresoftware/manticoresearch/issues/2493) 修复了带修饰符短语中的扩展问题。
* [问题 #2535](https://github.com/manticoresoftware/manticoresearch/issues/2535) 解决了使用 [ZONE](Searching/Full_text_matching/Operators.md#ZONE-limit-operator) 或 [ZONESPAN](Searching/Full_text_matching/Operators.md#ZONESPAN-limit-operator) 全文操作符时守护进程崩溃的问题。
* [问题 #2552](https://github.com/manticoresoftware/manticoresearch/issues/2552) 修复了带关键字字典的普通和 RT 表的中缀生成问题。
* [问题 #2571](https://github.com/manticoresoftware/manticoresearch/issues/2571) 修复了 `FACET` 查询中错误的回复；为带 `COUNT(*)` 的 `FACET` 设置了默认排序顺序为 `DESC`。
* [问题 #2580](https://github.com/manticoresoftware/manticoresearch/issues/2580) 修复了 Windows 上守护进程启动时崩溃的问题。
* [问题 #2603](https://github.com/manticoresoftware/manticoresearch/issues/2603) 修复了 HTTP 端点 `/sql` 和 `/sql?mode=raw` 的查询截断问题；使这两个端点的请求一致，无需 `query=` 头。
* [问题 #2623](https://github.com/manticoresoftware/manticoresearch/issues/2623) 修复了自动模式创建表但同时失败的问题。
* [问题 #2627](https://github.com/manticoresoftware/manticoresearch/issues/2627) 修复了 HNSW 库以支持加载多个 KNN 索引。
* [问题 #2630](https://github.com/manticoresoftware/manticoresearch/issues/2630) 修复了多个条件同时发生时冻结的问题。
* [问题 #2645](https://github.com/manticoresoftware/manticoresearch/issues/2645) 修复了使用 `OR` 结合 KNN 搜索时的致命错误导致崩溃的问题。
* [Issue #2647](https://github.com/manticoresoftware/manticoresearch/issues/2647) 修复了 `indextool --mergeidf *.idf --out global.idf` 创建后删除输出文件的问题。
* [Issue #2658](https://github.com/manticoresoftware/manticoresearch/issues/2658) 修复了在外层选择中带有 `ORDER BY` 字符串的子查询时守护进程崩溃的问题。
* [Issue #2686](https://github.com/manticoresoftware/manticoresearch/issues/2686) 修复了同时更新浮点属性和字符串属性时崩溃的问题。
* [Issue #2704](https://github.com/manticoresoftware/manticoresearch/issues/2704) 修复了来自 `lemmatize_xxx_all` 分词器的多个停用词增加后续词元的 `hitpos` 的问题。
* [Issue #2708](https://github.com/manticoresoftware/manticoresearch/issues/2708) 修复了 `ALTER ... ADD COLUMN ... TEXT` 时崩溃的问题。
* [Issue #2737](https://github.com/manticoresoftware/manticoresearch/issues/2737) 修复了在冻结表中更新 blob 属性（且表中至少有一个 RAM 分区）导致后续 `SELECT` 查询等待表解冻的问题。
* [Issue #2742](https://github.com/manticoresoftware/manticoresearch/issues/2742) 修复了带有压缩因子的查询跳过缓存的问题。
* [Issue #2775](https://github.com/manticoresoftware/manticoresearch/issues/2775) 现在 Manticore 在 `_bulk` 请求时对未知操作报告错误，而不是崩溃。
* [Issue #2791](https://github.com/manticoresoftware/manticoresearch/issues/2791) 修复了 HTTP `_bulk` 端点返回插入文档 ID 的问题。
* [Issue #2797](https://github.com/manticoresoftware/manticoresearch/issues/2797) 修复了当处理多个表时，其中一个为空，另一个匹配条目数量不同时 grouper 崩溃的问题。
* [Issue #2835](https://github.com/manticoresoftware/manticoresearch/issues/2835) 修复了复杂 `SELECT` 查询中的崩溃问题。
* [Issue #2872](https://github.com/manticoresoftware/manticoresearch/issues/2872) 增加了错误信息，如果 `IN` 表达式中 `ALL` 或 `ANY` 参数不是 JSON 属性。
* [Issue #2882](https://github.com/manticoresoftware/manticoresearch/issues/2882) 修复了在大表中更新 MVA 时守护进程崩溃的问题。
* [Issue #2888](https://github.com/manticoresoftware/manticoresearch/issues/2888) 修复了 `libstemmer` 分词失败时的崩溃。❤️ 感谢 [@subnix](https://github.com/subnix) 的 PR。
* [Issue #2919](https://github.com/manticoresoftware/manticoresearch/issues/2919) 修复了右表连接的权重在表达式中不起作用的问题。
* [Issue #2919](https://github.com/manticoresoftware/manticoresearch/issues/2919) 修复了右连接表权重在表达式中无法使用的问题。
* [Issue #325](https://github.com/manticoresoftware/manticoresearch-buddy/issues/325) 修复了表已存在时 `CREATE TABLE IF NOT EXISTS ... WITH DATA` 失败的问题。
* [Issue #351](https://github.com/manticoresoftware/manticoresearch-buddy/issues/351) 修复了使用带有文档 ID 的 KNN 计数时未定义数组键 "id" 的错误。
* [Issue #359](https://github.com/manticoresoftware/manticoresearch-buddy/issues/359) 修复了 `REPLACE INTO cluster_name:table_name` 功能的问题。
* [Issue #67](https://github.com/manticoresoftware/docker/issues/67) 修复了使用 `--network=host` 运行 Manticore Docker 容器时的致命错误。

## Version 6.3.8
Released: November 22nd 2024

Version 6.3.8 continues the 6.3 series and includes only bug fixes.

### Bug fixes

* [PR #2777](https://github.com/manticoresoftware/manticoresearch/pull/2777) 修正了当通过 `threads` 或 `max_threads_per_query` 设置限制查询并发时，可用线程的计算。

## manticore-extra v1.1.20

Released: Oct 7 2024

### Bug fixes

* [Issue #64](https://github.com/manticoresoftware/executor/issues/64) 解决了 `unattended-upgrades` 工具（在基于 Debian 的系统上自动安装包更新）错误地标记多个 Manticore 包（包括 `manticore-galera`、`manticore-executor` 和 `manticore-columnar-lib`）进行删除的问题。此问题由于 `dpkg` 错误地将虚拟包 `manticore-extra` 视为多余。已进行更改，确保 `unattended-upgrades` 不再尝试删除关键的 Manticore 组件。

## Version 6.3.6
Released: August 2nd 2024

Version 6.3.6 continues the 6.3 series and includes only bug fixes.

### Bug fixes

* [Issue #2477](https://github.com/manticoresoftware/manticoresearch/issues/2477) 修复了版本 6.3.4 引入的崩溃问题，该问题在处理表达式和分布式或多个表时可能发生。
* [Issue #2352](https://github.com/manticoresoftware/manticoresearch/issues/2352) 修复了由于 `max_query_time` 导致的在查询多个索引时守护进程崩溃或内部错误的早期退出问题。

## Version 6.3.4
Released: July 31st 2024

Version 6.3.4 continues the 6.3 series and includes only minor improvements and bug fixes.

### Minor changes
* [Issue #1130](https://github.com/manticoresoftware/manticoresearch/issues/1130) 增加了对 [DBeaver](Integration/DBeaver.md#Integration-with-DBeaver) 的支持。
* [Issue #2146](https://github.com/manticoresoftware/manticoresearch/issues/2146) 改进了词形变化和例外中的分隔符转义。
* [Issue #2315](https://github.com/manticoresoftware/manticoresearch/issues/2315) 在 SELECT 列表达式中增加了字符串比较操作符。
* [Issue #2363](https://github.com/manticoresoftware/manticoresearch/issues/2363) 支持 Elastic 风格的批量请求中的空值。
* [Issue #2374](https://github.com/manticoresoftware/manticoresearch/issues/2374) 增加了对 mysqldump 版本 9 的支持。
* [Issue #2375](https://github.com/manticoresoftware/manticoresearch/issues/2375) 改进了 HTTP JSON 查询中的错误处理，增加了错误发生节点的 JSON 路径。

### Bug fixes
* [Issue #2280](https://github.com/manticoresoftware/manticoresearch/issues/2280) 修复了当 disk_chunks > 1 时，在带有大量匹配的通配符查询中的性能下降问题。
* [问题 #2332](https://github.com/manticoresoftware/manticoresearch/issues/2332) 修复了空 MVA 数组的 MVA MIN 或 MAX SELECT 列表达式中的崩溃。
* [问题 #2339](https://github.com/manticoresoftware/manticoresearch/issues/2339) 修复了 Kibana 无限范围请求的错误处理。
* [问题 #2342](https://github.com/manticoresoftware/manticoresearch/issues/2342) 修复了当属性不在 SELECT 列表中时，右表按列属性的连接过滤器问题。
* [问题 #2343](https://github.com/manticoresoftware/manticoresearch/issues/2343) 修复了 Manticore 6.3.2 中重复的 'static' 说明符。
* [问题 #2344](https://github.com/manticoresoftware/manticoresearch/issues/2344) 修复了使用右表的 MATCH() 时，LEFT JOIN 返回非匹配条目的问题。
* [问题 #2350](https://github.com/manticoresoftware/manticoresearch/issues/2350) 修复了带有 `hitless_words` 的 RT 索引磁盘块保存问题。
* [问题 #2364](https://github.com/manticoresoftware/manticoresearch/issues/2364) 现在 'aggs_node_sort' 属性可以在其他属性中以任意顺序添加。
* [问题 #2368](https://github.com/manticoresoftware/manticoresearch/issues/2368) 修复了 JSON 查询中全文与过滤器顺序错误的问题。
* [问题 #2376](https://github.com/manticoresoftware/manticoresearch/issues/2376) 修复了与长 UTF-8 请求相关的 JSON 响应错误。
* [问题 #2684](https://github.com/manticoresoftware/dev/issues/2684) 修复了依赖于连接属性的 presort/prefilter 表达式的计算问题。
* [问题 #301](https://github.com/manticoresoftware/manticoresearch-buddy/issues/301) 更改指标数据大小的计算方法，改为从 `manticore.json` 文件读取，而不是检查数据目录的整体大小。
* [问题 #302](https://github.com/manticoresoftware/manticoresearch-buddy/issues/302) 增加了对 Vector.dev 验证请求的处理。

## 版本 6.3.2
发布时间：2024年6月26日

版本 6.3.2 继续 6.3 系列，包含多个错误修复，其中一些是在 6.3.0 发布后发现的。

### 破坏性更改
* ⚠️[问题 #2305](https://github.com/manticoresoftware/manticoresearch/issues/2305) 更新 aggs.range 的值为数值类型。

### 错误修复
* [提交 c51c](https://github.com/manticoresoftware/manticoresearch/commit/c51c7fa4bd0ec4ad8d53188b888461bff29f1417) 修复了存储检查与 rset 合并中的分组问题。
* [提交 0e85](https://github.com/manticoresoftware/manticoresearch/commit/0e85b9d5682f51681750d798aef6a11cfb93d69e) 修复了在使用 CRC 字典和启用 `local_df` 的 RT 索引中使用通配符查询时守护进程崩溃的问题。
* [问题 #2200](https://github.com/manticoresoftware/manticoresearch/issues/2200) 修复了在无 GROUP BY 的 `count(*)` JOIN 查询中的崩溃。
* [问题 #2201](https://github.com/manticoresoftware/manticoresearch/issues/2201) 修复了尝试对全文字段进行分组时 JOIN 未返回警告的问题。
* [问题 #2230](https://github.com/manticoresoftware/manticoresearch/issues/2230) 解决了通过 `ALTER TABLE` 添加属性时未考虑 KNN 选项的问题。
* [问题 #2231](https://github.com/manticoresoftware/manticoresearch/issues/2231) 修复了版本 6.3.0 中删除 `manticore-tools` Redhat 包失败的问题。
* [问题 #2242](https://github.com/manticoresoftware/manticoresearch/issues/2242) 修正了 JOIN 和多个 FACET 语句返回错误结果的问题。
* [问题 #2250](https://github.com/manticoresoftware/manticoresearch/issues/2250) 修复了如果表处于集群中，ALTER TABLE 会产生错误的问题。
* [问题 #2252](https://github.com/manticoresoftware/manticoresearch/issues/2252) 修复了 SphinxQL 接口中传入 buddy 的原始查询问题。
* [问题 #2267](https://github.com/manticoresoftware/manticoresearch/issues/2267) 改进了带磁盘块的 RT 索引在 `CALL KEYWORDS` 中的通配符展开。
* [问题 #271](https://github.com/manticoresoftware/manticoresearch-buddy/issues/271) 修复了错误的 `/cli` 请求挂起问题。
* [问题 #274](https://github.com/manticoresoftware/manticoresearch-buddy/issues/274) 解决了对 Manticore 的并发请求可能卡住的问题。
* [问题 #275](https://github.com/manticoresoftware/manticoresearch-buddy/issues/275) 修复了通过 `/cli` 执行 `drop table if exists t; create table t` 命令时的挂起问题。

### 复制相关
* [问题 #2270](https://github.com/manticoresoftware/manticoresearch/issues/2270) 在 `/_bulk` HTTP 端点中新增对 `cluster:name` 格式的支持。

## 版本 6.3.0
发布时间：2024年5月23日

### 重大更改
* [问题 #839](https://github.com/manticoresoftware/manticoresearch/issues/839) 实现了 [float_vector](Creating_a_table/Data_types.md#Float-vector) 数据类型；实现了 [向量搜索](Searching/KNN.md#KNN-vector-search)。
* [问题 #1673](https://github.com/manticoresoftware/manticoresearch/issues/1673) 实现了 [INNER/LEFT JOIN](Searching/Joining.md)（**测试阶段**）。
* [问题 #1744](https://github.com/manticoresoftware/manticoresearch/issues/1744) 实现了 [timestamp](Creating_a_table/Data_types.md#Timestamps) 字段的日期格式自动检测。
* [问题 #1720](https://github.com/manticoresoftware/manticoresearch/pull/1720) 将 Manticore Search 许可证由 GPLv2-or-later 更改为 GPLv3-or-later。
* [提交 7a55](https://github.com/manticoresoftware/manticoresearch/commit/7a5508424) Windows 下运行 Manticore 现在需要通过 Docker 运行 Buddy。
* [问题 #1541](https://github.com/manticoresoftware/manticoresearch/issues/1541) 增加了一个 [REGEX](Searching/Full_text_matching/Operators.md#REGEX-operator) 全文操作符。
* [问题 #2091](https://github.com/manticoresoftware/manticoresearch/issues/2091) 支持 Ubuntu Noble 24.04。
* [提交 514d](https://github.com/manticoresoftware/manticoresearch/commit/514d35b497f4bdb20b6473ff963752a978a4bb8d) 重构了时间操作以提升性能并增加新的日期/时间函数：
  - [CURDATE()](Functions/Date_and_time_functions.md#CURDATE%28%29) - 返回本地时区的当前日期
  - [QUARTER()](Functions/Date_and_time_functions.md#QUARTER%28%29) - 返回时间戳参数对应的年份季度整数
  - [DAYNAME()](Functions/Date_and_time_functions.md#DAYNAME%28%29) - 返回给定时间戳参数对应的星期名称
  - [MONTHNAME()](Functions/Date_and_time_functions.md#MONTHNAME%28%29) - 返回给定时间戳参数对应的月份名称
  - [DAYOFWEEK()](Functions/Date_and_time_functions.md#DAYOFWEEK%28%29) - 返回给定时间戳参数对应的星期整数索引
  - [DAYOFYEAR()](Functions/Date_and_time_functions.md#DAYOFYEAR%28%29) - 返回给定时间戳参数对应的年份中的天数整数
  - [YEARWEEK()](Functions/Date_and_time_functions.md#YEARWEEK%28%29) - 返回给定时间戳参数对应的年份整数和当前周第一天的日期代码
  - [DATEDIFF()](Functions/Date_and_time_functions.md#DATEDIFF%28%29) - 返回两个给定时间戳之间的天数差
  - [DATE()](Functions/Date_and_time_functions.md#DATE%28%29) - 格式化时间戳参数中的日期部分
  - [TIME()](Functions/Date_and_time_functions.md#TIME%28%29) - 格式化时间戳参数中的时间部分
  - [timezone](Server_settings/Searchd.md#timezone) - 日期/时间相关函数使用的时区。
* [Commit 30e7](https://github.com/manticoresoftware/manticoresearch/commit/30e789d9657f9b3093c280c6e5b0e46a021f6dc7) 在 HTTP 接口中添加了 [range](Searching/Faceted_search.md#Facet-over-set-of-ranges)、[histogram](Searching/Faceted_search.md#Facet-over-histogram-values)、[date_range](Searching/Faceted_search.md#Facet-over-set-of-date-ranges) 和 [date_histogram](Searching/Faceted_search.md#Facet-over-histogram-date-values) 聚合，以及类似的 SQL 表达式。

### 小更新
* [Issue #1285](https://github.com/manticoresoftware/manticoresearch/issues/1285) 支持 Filebeat 版本 8.10 - 8.11。
* [Issue #1771](https://github.com/manticoresoftware/manticoresearch/issues/1771) [ALTER TABLE ... type='distributed'](Updating_table_schema_and_settings.md#Changing-a-distributed-table) 支持。
* [Issue #1788](https://github.com/manticoresoftware/manticoresearch/issues/1788) 通过 [CREATE TABLE ... LIKE ... WITH DATA](Creating_a_table/Local_tables/Real-time_table.md#CREATE-TABLE-LIKE:) SQL 语句添加了复制表的能力。
* [Issue #2072](https://github.com/manticoresoftware/manticoresearch/issues/2072) 优化了 [表压缩算法](Securing_and_compacting_a_table/Compacting_a_table.md#Compacting-a-Table)：之前，无论是手动 [OPTIMIZE](Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE) 还是自动 [auto_optimize](Server_settings/Searchd.md#auto_optimize) 都会先合并数据块以确保数量不超过限制，然后清除所有包含已删除文档的数据块中的已删除文档。此方法有时资源消耗过大，已被禁用。现在，数据块合并仅根据 [progressive_merge](Server_settings/Common.md#progressive_merge) 设置进行。然而，包含大量已删除文档的数据块更可能被合并。
* [Commit ce6c](https://github.com/manticoresoftware/columnar/commit/ce6c5d3381edcdc411a00a5923d13d56dd9b3010) 添加了对加载较新版本二级索引的保护。
* [Issue #1417](https://github.com/manticoresoftware/manticoresearch/issues/1417) 通过 [REPLACE INTO ... SET](Data_creation_and_modification/Updating_documents/REPLACE.md#SQL-REPLACE) 支持部分替换。
* [Commit 7c16](https://github.com/manticoresoftware/manticoresearch/commit/7c167b934ce416d3dfbd658744a231e638b14d9a) 更新了默认合并缓冲区大小：`.spa`（标量属性）：256KB -> 8MB；`.spb`（blob属性）：256KB -> 8MB；`.spc`（列式属性）：1MB，无变化；`.spds`（文档存储）：256KB -> 8MB；`.spidx`（二级索引）：256KB 缓冲区 -> 128MB 内存限制；`.spi`（字典）：256KB -> 16MB；`.spd`（文档列表）：8MB，无变化；`.spp`（命中列表）：8MB，无变化；`.spe`（跳跃列表）：256KB -> 8MB。
* [Issue #1859](https://github.com/manticoresoftware/manticoresearch/issues/1859) 通过 JSON 添加了 [复合聚合](Searching/Grouping.md#GROUP-BY-multiple-fields-at-once)。
* [Commit 216b](https://github.com/manticoresoftware/manticoresearch-buddy/commit/216b824) 禁用 PCRE.JIT，原因是某些正则表达式模式存在问题且无显著时间提升。
* [Commit 55cd](https://github.com/manticoresoftware/manticoresearch/commit/55cdce32) 添加了对 vanilla Galera v.3 (api v25)（MySQL 5.x 中的 `libgalera_smm.so`）的支持。
* [Commit 86f9](https://github.com/manticoresoftware/manticoresearch-buddy/commit/86f98ab) 指标后缀从 `_rate` 改为 `_rps`。
* [Commit c0c1](https://github.com/manticoresoftware/manticoresearch-helm/commit/c0c1cff780031412ef57cefab987dfd16613368e) 改进了有关负载均衡 HA 支持的文档。
* [Commit d1d2](https://github.com/manticoresoftware/manticoresearch/commit/d1d2ca18857c68106e89743756260e73285be739) 错误信息中的 `index` 改为 `table`；修复了 bison 解析器错误信息。
* [Commit fd26](https://github.com/manticoresoftware/manticoresearch/commit/fd26671b) 支持将 `manticore.tbl` 作为表名。
* [Issue #1105](https://github.com/manticoresoftware/manticoresearch/issues/1105) 支持通过 systemd 运行索引器（[文档](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Schedule-indexer-via-systemd)）。❤️ 感谢 [@subnix](https://github.com/subnix) 的 PR。
* [Issue #1294](https://github.com/manticoresoftware/manticoresearch/issues/1294) GEODIST() 中支持二级索引。
* [Issue #1394](https://github.com/manticoresoftware/manticoresearch/issues/1394) 简化了 [SHOW THREADS](Node_info_and_management/SHOW_THREADS.md#SHOW-THREADS)。
* [Issue #1424](https://github.com/manticoresoftware/manticoresearch/issues/1424) 为 `create distributed table` 语句添加了默认值支持（`agent_connect_timeout` 和 `agent_query_timeout`）。
* [Issue #1442](https://github.com/manticoresoftware/manticoresearch/issues/1442) 添加了 [expansion_limit](Searching/Options.md#expansion_limit) 搜索查询选项，用于覆盖 `searchd.expansion_limit`。
* [Issue #1448](https://github.com/manticoresoftware/manticoresearch/issues/1448) 实现了 [ALTER TABLE](Updating_table_schema_and_settings.md#Updating-table-schema-in-RT-mode) 用于 int->bigint 的转换。
* [Issue #1456](https://github.com/manticoresoftware/manticoresearch/issues/1456) MySQL 响应中的元信息。
* [Issue #1494](https://github.com/manticoresoftware/manticoresearch/issues/1494) [SHOW VERSION](Node_info_and_management/SHOW_VERSION.md#SHOW-VERSION)。
* [Issue #1582](https://github.com/manticoresoftware/manticoresearch/issues/1582) 支持通过 JSON [按 ID 数组删除文档](Data_creation_and_modification/Deleting_documents.md?client=JSON#Deleting-documents)。
* [Issue #1589](https://github.com/manticoresoftware/manticoresearch/issues/1589) 改进错误信息 “unsupported value type”。
* [Issue #1634](https://github.com/manticoresoftware/manticoresearch/issues/1634) 在 `SHOW STATUS` 中添加了 Buddy 版本信息。
* [Issue #1641](https://github.com/manticoresoftware/manticoresearch/issues/1641) 在关键字文档为零时优化匹配请求。
* [Issue #1712](https://github.com/manticoresoftware/manticoresearch/issues/1712) 添加了将字符串值 "true" 和 "false" 转换为 bool 属性的功能，用于数据提交。
* [Issue #1713](https://github.com/manticoresoftware/manticoresearch/issues/1713) 添加了 [access_dict](Server_settings/Searchd.md#access_dict) 表及 searchd 选项。
* [Issue #1767](https://github.com/manticoresoftware/manticoresearch/issues/1767) 在 searchd 配置节中新增选项：[expansion_merge_threshold_docs](Server_settings/Searchd.md#expansion_merge_threshold_docs) 和 [expansion_merge_threshold_hits](Server_settings/Searchd.md#expansion_merge_threshold_hits)；使合并扩展关键词中的小词项的阈值可配置。
* [Issue #1768](https://github.com/manticoresoftware/manticoresearch/issues/1768) 在 `@@system.sessions` 中新增显示最后命令时间。
* [Issue #1806](https://github.com/manticoresoftware/manticoresearch/issues/1806) 升级 Buddy 协议至版本 2。
* [Issue #1810](https://github.com/manticoresoftware/manticoresearch/issues/1810) 为 `/sql` 端点实现了额外的请求格式，以便于与库的集成。
* [Issue #1825](https://github.com/manticoresoftware/manticoresearch/pull/1825) 为 SHOW BUDDY PLUGINS 添加了信息部分。
* [Issue #1837](https://github.com/manticoresoftware/manticoresearch/issues/1837) 优化 `CALL PQ` 在处理大数据包时的内存消耗。
* [Issue #1853](https://github.com/manticoresoftware/manticoresearch/pull/1853) 编译器从 Clang 15 切换到 Clang 16。
* [Issue #1857](https://github.com/manticoresoftware/manticoresearch/issues/1857) 添加字符串比较功能。❤️ 感谢 [@etcd](https://github.com/etcd) 的 PR。
* [Issue #1915](https://github.com/manticoresoftware/manticoresearch/issues/1915) 添加对联合存储字段的支持。
* [Issue #1937](https://github.com/manticoresoftware/manticoresearch/issues/1937) 在查询日志中记录客户端的主机:端口。
* [Issue #1981](https://github.com/manticoresoftware/manticoresearch/issues/1981) 修正错误的错误信息。
* [Issue #1983](https://github.com/manticoresoftware/manticoresearch/issues/1983) 引入了针对 [JSON 查询计划](Node_info_and_management/Profiling/Query_plan.md?client=JSON#Query-plan)的详细级别支持。
* [Issue #2010](https://github.com/manticoresoftware/manticoresearch/issues/2010) 禁用了来自 Buddy 的查询日志，除非设置了 `log_level=debug`。
* [Issue #2035](https://github.com/manticoresoftware/manticoresearch/issues/2035) 支持 Linux Mint 21.3。
* [Issue #2056](https://github.com/manticoresoftware/manticoresearch/issues/2056) 修复 Manticore 无法使用 Mysql 8.3+ 构建的问题。
* [Issue #2112](https://github.com/manticoresoftware/manticoresearch/issues/2112) 为实时表块添加了 `DEBUG DEDUP` 命令，以处理附加包含重复项的普通表后的重复条目问题。
* [Issue #212](https://github.com/manticoresoftware/manticoresearch-buddy/issues/212) 为 [SHOW QUERIES](Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES) 添加时间显示。
* [Issue #218](https://github.com/manticoresoftware/manticoresearch-buddy/issues/218) 将 `@timestamp` 列处理为时间戳。
* [Issue #252](https://github.com/manticoresoftware/manticoresearch-buddy/issues/250) 实现了启用/禁用 buddy 插件的逻辑。
* [Issue #254](https://github.com/manticoresoftware/manticoresearch-buddy/issues/254) 更新了 composer 至修复近期 CVE 的更新版本。
* [Issue #340](https://github.com/manticoresoftware/manticoresearch/issues/340) Manticore systemd 单元中对 `RuntimeDirectory` 的小幅优化。
* [Issue #51](https://github.com/manticoresoftware/executor/issues/51) 添加了 rdkafka 支持并升级到 PHP 8.3.3。
* [Issue #527](https://github.com/manticoresoftware/manticoresearch/issues/527) 支持 [附加](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Attaching_one_table_to_another.md#Attaching-one-table-to-another) RT 表。新增命令 [ALTER TABLE ... RENAME](Updating_table_schema_and_settings.md#Renaming-a-real-time-table)。

### 重大变更与废弃
* ⚠️[Issue #1436](https://github.com/manticoresoftware/manticoresearch/issues/1436) 修正了 IDF 计算问题。`local_df` 现在为默认。改进了主-代理搜索协议（版本已更新）。如果您在分布式环境中运行多个 Manticore Search 实例，请确保先升级代理，再升级主节点。
* ⚠️[Issue #1572](https://github.com/manticoresoftware/manticoresearch/issues/1572) 添加了分布式表的复制功能，并更新了复制协议。如果您运行复制集群，您需要：
  - 首先，干净利落地停止所有节点
  - 然后，使用 Linux 下的工具 `manticore_new_cluster`，以 `--new-cluster` 参数启动最后停止的节点。
  - 更多细节请阅读关于[重启集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster)的内容。
* ⚠️[Issue #1763](https://github.com/manticoresoftware/manticoresearch/pull/1763) HTTP API 端点别名 `/json/*` 已被废弃。
* ⚠️[Issue #1982](https://github.com/manticoresoftware/manticoresearch/issues/1982) 将 JSON 中的 [profile](Node_info_and_management/Profiling/Query_profile.md#Query-profile) 改为 [plan](Node_info_and_management/Profiling/Query_plan.md#Query-plan)，并新增了 JSON 查询性能分析支持。
* ⚠️[Commit e235](https://github.com/manticoresoftware/manticoresearch-backup/commit/e23585a) manticore-backup 不再备份 `plugin_dir`。
* ⚠️[Issue #171](https://github.com/manticoresoftware/manticoresearch-buddy/issues/155) 将 Buddy 迁移到 Swoole 以提升性能与稳定性。切换版本时请确保所有 Manticore 软件包已更新。
* ⚠️[Issue #196](https://github.com/manticoresoftware/manticoresearch-buddy/pull/205) 将所有核心插件合并到 Buddy 并更改了核心逻辑。
* ⚠️[Issue #2107](https://github.com/manticoresoftware/manticoresearch/issues/2107) 在 `/search` 响应中将文档 ID 视为数字。
* ⚠️[Issue #38](https://github.com/manticoresoftware/executor/issues/38) 添加了 Swoole，禁用了 ZTS，并移除了 parallel 扩展。
* ⚠️[Issue #1929](https://github.com/manticoresoftware/manticoresearch/issues/1929) 在某些情况下 charset_table 中的重写未生效。

### 复制相关变更
* [Commit 3376](https://github.com/manticoresoftware/manticoresearch/commit/3376351f73db3ff0cb6cb708a304a7bd83e69cfb) 修复了大型文件 SST 的复制错误。
* [Commit 6d36](https://github.com/manticoresoftware/manticoresearch/commit/6d36c68fb7ce3cb274b467aac435788905084952) 为复制命令增加了重试机制；修复了在网络繁忙且存在数据包丢失时复制加入失败的问题。
* [Commit 842e](https://github.com/manticoresoftware/manticoresearch/commit/842e27e71790833ab4125d49103d2933d5df60c3) 将复制中的 FATAL 错误消息改为 WARNING 警告消息。
* [Commit 8c32](https://github.com/manticoresoftware/manticoresearch/commit/8c32bce7db39d8ed3b1a06aaa1d2c3ec372084f0) 修正了无表或表为空的复制集群中 `gcache.page_size` 的计算；修正了 Galera 选项的保存和加载。
* [Commit a2af](https://github.com/manticoresoftware/manticoresearch/commit/a2af06ca36abcbf893e43c8803853130b397c54b) 添加了功能，以在节点加入集群时跳过更新节点复制命令。
* [Commit c054](https://github.com/manticoresoftware/manticoresearch/commit/c0541e6892f5116b1f957b40060ba8ecd01a5afb) 修复了在更新 blob 属性与替代文档时复制过程中的死锁。
* [Commit e80d](https://github.com/manticoresoftware/manticoresearch/commit/e80d505b963999016033613c40c1cf3151d648e7) 新增了 [replication_connect_timeout](Server_settings/Searchd.md#replication_connect_timeout)、[replication_query_timeout](Server_settings/Searchd.md#replication_query_timeout)、[replication_retry_delay](Server_settings/Searchd.md#replication_retry_delay)、[replication_retry_count](Server_settings/Searchd.md#replication_retry_count) 这些 searchd 配置选项，用于控制复制过程中的网络行为，类似于 `searchd.agent_*`，但默认值不同。
* [Issue #1356](https://github.com/manticoresoftware/manticoresearch/issues/1356) 修复了部分节点失联并且名称解析失败后复制节点重试的问题。
* [Issue #1445](https://github.com/manticoresoftware/manticoresearch/issues/1445) 修正了 `show variables` 命令中复制日志冗余级别。
* [Issue #1482](https://github.com/manticoresoftware/manticoresearch/issues/1482) 修复了 Kubernetes 中 pod 重启导致节点加入集群时的复制问题。
* [Issue #1962](https://github.com/manticoresoftware/manticoresearch/issues/1962) 修复了空集群上使用无效节点名导致复制修改时长时间等待的问题。

### Bug 修复
* [Commit 8a48](https://github.com/manticoresoftware/manticoresearch/commit/8a486c7ba1a293879cdecc41fb52eb7ab0cbf832) 修复了 `count distinct` 中未使用的匹配项清理可能导致崩溃的问题。
* [Issue #1569](https://github.com/manticoresoftware/manticoresearch/issues/1569) 二进制日志现在以事务粒度写入。
* [Issue #2089](https://github.com/manticoresoftware/manticoresearch/issues/2089) 修复了与 64 位 ID 相关的一个错误，该错误可能导致通过 MySQL 插入时出现“Malformed packet”错误，进而导致[表损坏和重复 ID](https://github.com/manticoresoftware/manticoresearch/issues/2090)。
* [Issue #2160](https://github.com/manticoresoftware/manticoresearch/issues/2160) 修正了日期被错误地当作 UTC 时间而非本地时区插入的问题。
* [Issue #2177](https://github.com/manticoresoftware/manticoresearch/issues/2177) 修复了在实时表使用非空 `index_token_filter` 执行搜索时的崩溃问题。
* [Issue #2209](https://github.com/manticoresoftware/manticoresearch/issues/2209) 改进了 RT 列存储中的重复过滤，修复了崩溃和错误的查询结果。
* [Commit 001d](https://github.com/manticoresoftware/manticoresearch/commit/001d82904e4b887c49387308291aeefb07c0fbc4) 修复了 html 过滤器在处理联合字段后内存破坏的问题。
* [Commit 00eb](https://github.com/manticoresoftware/manticoresearch/commit/00eb2ebf) 避免在 flush 后回绕流，防止与 mysqldump 的通信问题。
* [Commit 0553](https://github.com/manticoresoftware/manticoresearch/commit/05534114acf08c70ffcc41a23d558dec09a34373) 如果 preread 尚未开始则不等待其完成。
* [Commit 055a](https://github.com/manticoresoftware/manticoresearch/commit/055aac09f877570cd29b6594cb283182eba7f73b) 修复了搜索守护进程日志中大型 Buddy 输出字符串按多行拆分的问题。
* [Commit 0a88](https://github.com/manticoresoftware/manticoresearch/commit/0a88056307f82c50fcdc458ad6dbc307a00e3f92) 将 MySQL 接口关于失败的头部 `debugv` 详细级别的警告移动。
* [Commit 150a](https://github.com/manticoresoftware/manticoresearch/commit/150a20286b7d98d24b844ae577d7fda4a444798d) 修复了多集群管理操作中的竞态条件；禁止创建多个具有相同名称或路径的集群。
* [Commit 2e40](https://github.com/manticoresoftware/manticoresearch/commit/2e405c1c9ebac6f46206e3df17ac1c19129c9aa3) 修正了全文查询中的隐式截止；将 MatchExtended 拆分为模板部分 D。
* [Commit 75f5](https://github.com/manticoresoftware/manticoresearch/commit/75f51421542a2a0658f9e2f9f31048c8c5224479) 修复了索引和加载表到守护进程时 `index_exact_words` 的不一致问题。
* [Commit 7643](https://github.com/manticoresoftware/manticoresearch/commit/7643976bea142e08535d701000eff0d09a99fda2) 修复了无效集群删除时缺失的错误消息。
* [Commit 7a03](https://github.com/manticoresoftware/manticoresearch/commit/7a034ac990d5ec9344320c05a6a2315583b9b858) 修正了 CBO 与队列联合的问题；修复了 CBO 与 RT 伪分片的问题。
* [Commit 7b4e](https://github.com/manticoresoftware/manticoresearch/commit/7b4ea95e) 启动时未加载次级索引（SI）库和配置中的参数时，修正了误导性的警告信息“WARNING: secondary_indexes set but failed to initialize secondary library”。
* [Commit 8496](https://github.com/manticoresoftware/manticoresearch/commit/8496317f099fade8d2db463fb88483e7fcdfe85b) 修正了仲裁中的命中排序。
* [Commit 8973](https://github.com/manticoresoftware/manticoresearch-buddy/commit/8973ad3) 修复了 ModifyTable 插件中大写选项的问题。
* [Commit 9935](https://github.com/manticoresoftware/manticoresearch-buddy/commit/99350d4c184379b19f3efa9751d9b5be5e6edfb5) 修复了从包含空 json 值（表示为 NULL）的转储恢复的问题。
* [Commit a28f](https://github.com/manticoresoftware/manticoresearch/commit/a28f6e087b3d957e48816070be17d0823904936a) 使用 pcon 修复了联接节点接收 SST 时的 SST 超时问题。
* [Commit b5a5](https://github.com/manticoresoftware/manticoresearch/commit/b5a55cc26debcddcda5d9caef8366a707e0dd763) 修复了选择别名字符串属性时崩溃的问题。
* [Commit c556](https://github.com/manticoresoftware/manticoresearch/commit/c556e7ed8b6da8eac8ef094650df5f0a8977916b) 添加了在 `morphology_skip_fields` 字段的全文查询中，将词条转换为 `=term` 的查询转换。
* [Commit cdc3](https://github.com/manticoresoftware/manticoresearch/commit/cdc38085145cd1025b96fcac2a46188a0425d9e1) 添加了缺失的配置键（skiplist_cache_size）。
* [Commit cf6e](https://github.com/manticoresoftware/manticoresearch/commit/cf6ec3bfdc3f0990fb31ff53822b2331a6667727) 修复了表达式排名器在处理大型复杂查询时的崩溃。
* [Commit e513](https://github.com/manticoresoftware/manticoresearch/commit/e5139ba75eef7223df375ae3cc3f138fa7d925f1) 修复了全文 CBO 对无效索引提示的处理。
* [Commit eb05](https://github.com/manticoresoftware/manticoresearch/commit/eb0514b3a6f57f278252b0724eebd112ec116e0f) 在关闭时中断预读，以加快关闭速度。
* [Commit f945](https://github.com/manticoresoftware/manticoresearch/commit/f94555a293cfa51004ff81135b8bd8500b955115) 修改了全文查询的堆栈计算，避免复杂查询情况下的崩溃。
* [Issue #1262](https://github.com/manticoresoftware/manticoresearch/issues/1262) 修复了索引器在索引具有多个相同名称列的 SQL 源时崩溃的问题。
* [Issue #1273](https://github.com/manticoresoftware/manticoresearch/issues/1273) 对不存在的系统变量返回 0 而非 <empty>。
* [Issue #1289](https://github.com/manticoresoftware/manticoresearch/issues/1289) 修复了检查 RT 表外部文件时 indextool 的错误。
* [Issue #1335](https://github.com/manticoresoftware/manticoresearch/issues/1335) 修复了因短语内多词变形导致的查询解析错误。
* [Issue #1364](https://github.com/manticoresoftware/manticoresearch/issues/1364) 添加了对旧版二进制日志文件中空日志文件的重放支持。
* [Issue #1365](https://github.com/manticoresoftware/manticoresearch/issues/1365) 修复了删除最后一个空的二进制日志文件的问题。
* [Issue #1372](https://github.com/manticoresoftware/manticoresearch/issues/1372) 修正了 `data_dir` 改变后导致守护进程启动时当前工作目录的相对路径（转换为绝对路径）错误。
* [Issue #1393](https://github.com/manticoresoftware/manticoresearch/issues/1393) 在 hn_small 中的最慢时间退化：守护进程启动时获取/缓存 CPU 信息。
* [Issue #1395](https://github.com/manticoresoftware/manticoresearch/issues/1395) 修复了索引加载时缺少外部文件的警告。
* [Issue #1402](https://github.com/manticoresoftware/manticoresearch/issues/1402) 修复了全局分组器释放数据指针属性时的崩溃。
* [Issue #1403](https://github.com/manticoresoftware/manticoresearch/issues/1403) _ADDITIONAL_SEARCHD_PARAMS 无法生效的问题。
* [Issue #1427](https://github.com/manticoresoftware/manticoresearch/issues/1427) 修复了每表 `agent_query_timeout` 被默认查询选项 `agent_query_timeout` 替代的问题。
* [Issue #1444](https://github.com/manticoresoftware/manticoresearch/issues/1444) 修复了在使用 `packedfactors()` 多值匹配时分组器和排名器崩溃的问题。
* [Issue #1458](https://github.com/manticoresoftware/manticoresearch/issues/1458) Manticore 在频繁索引更新时崩溃的问题。
* [Issue #1481](https://github.com/manticoresoftware/manticoresearch/issues/1481) 修复了解析错误后清理解析查询时的崩溃。
* [Issue #1484](https://github.com/manticoresoftware/manticoresearch/issues/1484) 修复了 HTTP JSON 请求未被路由到 buddy 的问题。
* [Issue #1499](https://github.com/manticoresoftware/manticoresearch/issues/1499) JSON 属性根值不能是数组的问题已修复。
* [Issue #1507](https://github.com/manticoresoftware/manticoresearch/issues/1507) 修复了事务内重建表时的崩溃问题。
* [Issue #1515](https://github.com/manticoresoftware/manticoresearch/issues/1515) 修复了俄语词形简写的展开问题。
* [Issue #1579](https://github.com/manticoresoftware/manticoresearch/issues/1579) 修复了 JSON 和 STRING 属性在 [date_format](Functions/Date_and_time_functions.md#DATE_FORMAT()) 表达式中的使用问题。
* [Issue #1580](https://github.com/manticoresoftware/manticoresearch/issues/1580) 修复了针对多个别名映射到 JSON 字段的分组器错误。
* [Issue #1594](https://github.com/manticoresoftware/manticoresearch/issues/1594) 修复了 dev 中 total_related 错误：隐式 cutoff 与 limit 问题；增强了 json 查询中的全扫描检测。
* [Issue #1603](https://github.com/manticoresoftware/manticoresearch/issues/1603) 修复了所有日期表达式中 JSON 和 STRING 属性的使用问题。
* [Issue #1609](https://github.com/manticoresoftware/manticoresearch/issues/1609) 修复了使用 LEVENSHTEIN() 时的崩溃。
* [Issue #1612](https://github.com/manticoresoftware/manticoresearch/issues/1612) 修复了搜索查询解析错误并带有高亮时的内存损坏问题。
* [Issue #1614](https://github.com/manticoresoftware/manticoresearch/issues/1614) 禁用了对长度短于 `min_prefix_len` / `min_infix_len` 的词进行通配符展开。
* [Issue #1617](https://github.com/manticoresoftware/manticoresearch/issues/1617) 修改行为，当 Buddy 成功处理请求时，不再记录错误日志。
* [Issue #1635](https://github.com/manticoresoftware/manticoresearch/issues/1635) 修复了带有 limit 设置的查询时，搜索查询元信息中的 total 统计错误。
* [Issue #1640](https://github.com/manticoresoftware/manticoresearch/issues/1640) 修复了 plain 模式下通过 JSON 使用大写表名不可能的问题。
* [Issue #1643](https://github.com/manticoresoftware/manticoresearch/issues/1643) 提供了默认的 `SPH_EXTNODE_STACK_SIZE` 值。
* [Issue #1646](https://github.com/manticoresoftware/manticoresearch/issues/1646) 修复了使用 ALL/ANY 操作多值属性（MVA）的负筛选器在 SphinxQL 日志中的错误。
* [Issue #1660](https://github.com/manticoresoftware/manticoresearch/issues/1660) 修复了从其他索引应用 docid killlists 的问题。❤️ 感谢 [@raxoft](https://github.com/raxoft) 的 PR。
* [Issue #1668](https://github.com/manticoresoftware/manticoresearch/issues/1668) 修复了因原始索引全扫描（无任何索引迭代器）导致的过早退出而遗漏匹配项的问题；移除了 plain row 迭代器的 cutoff。
* [Issue #1671](https://github.com/manticoresoftware/manticoresearch/issues/1671) 修复了查询带有代理和本地表的分布式表时 `FACET` 报错的问题。
* [Issue #1690](https://github.com/manticoresoftware/manticoresearch/issues/1690) 修复了大数值直方图估算时崩溃的问题。
* [Issue #1692](https://github.com/manticoresoftware/manticoresearch/issues/1692) 修复了执行 alter table tbl add column col uint 时的崩溃。
* [Issue #1710](https://github.com/manticoresoftware/manticoresearch/issues/1710) 修复了条件 `WHERE json.array IN (<value>)` 返回空结果的问题。
* [Issue #172](https://github.com/manticoresoftware/manticoresearch-buddy/issues/172) 修复了向 `/cli` 发送请求时 TableFormatter 的问题。
* [Issue #1742](https://github.com/manticoresoftware/manticoresearch/issues/1742) 修复了缺少 wordforms 文件时 `CREATE TABLE` 未报错的问题。
* [Issue #1762](https://github.com/manticoresoftware/manticoresearch/issues/1762) 现在 RT 表中属性的顺序遵循配置顺序。
* [Issue #1765](https://github.com/manticoresoftware/manticoresearch/issues/1765) 修复了 HTTP bool 查询中含有 'should' 条件时返回错误结果的问题。
* [Issue #1769](https://github.com/manticoresoftware/manticoresearch/issues/1769) 修复了使用 `SPH_SORT_ATTR_DESC` 和 `SPH_SORT_ATTR_ASC` 按字符串属性排序无效的问题。
* [Issue #177](https://github.com/manticoresoftware/manticoresearch-buddy/issues/177) 禁用了对 Buddy 的 curl 请求中的 `Expect: 100-continue` HTTP 头。
* [Issue #1791](https://github.com/manticoresoftware/manticoresearch/issues/1791) 修复了由 GROUP BY 别名引起的崩溃。
* [Issue #1792](https://github.com/manticoresoftware/manticoresearch/issues/1792) 修复了 Windows 上 SQL meta 汇总显示错误时间的问题。
* [Issue #1794](https://github.com/manticoresoftware/manticoresearch/issues/1794) 修复了 JSON 查询中单词性能下降的问题。
* [Issue #1798](https://github.com/manticoresoftware/manticoresearch/issues/1798) 修复了 `/search` 上 incompatible 过滤器未抛出错误的问题。
* [Issue #1802](https://github.com/manticoresoftware/manticoresearch/issues/1802) 修复了 `ALTER CLUSTER ADD` 和 `JOIN CLUSTER` 操作的等待机制，防止在 `ALTER` 向集群添加表的同时，提供节点向加入节点发送表时的竞态条件。
* [Issue #1811](https://github.com/manticoresoftware/manticoresearch/issues/1811) 修复了 `/pq/{table}/*` 请求处理不正确的问题。
* [Issue #1816](https://github.com/manticoresoftware/manticoresearch/issues/1816) 修复了某些情况下 `UNFREEZE` 无效的问题。
* [Issue #183](https://github.com/manticoresoftware/manticoresearch-buddy/pull/183) 修复了某些情况下多值属性 (MVA) 恢复的问题。
* [Issue #1849](https://github.com/manticoresoftware/manticoresearch/issues/1849) 修复了使用 MCL 时 indextool 在关闭时崩溃的问题。
* [Issue #1866](https://github.com/manticoresoftware/manticoresearch/issues/1866) 修复了对 `/cli_json` 请求的不必要 URL 解码。
* [Issue #1872](https://github.com/manticoresoftware/manticoresearch/issues/1872) 修改了守护进程启动时 plugin_dir 的设置逻辑。
* [Issue #1874](https://github.com/manticoresoftware/manticoresearch/issues/1874) 修复了 alter table ... 异常失败的问题。
* [Issue #1891](https://github.com/manticoresoftware/manticoresearch/issues/1891#issuecomment-2051941200) 当插入数据时，Manticore 发生 `signal 11` 崩溃。
* [Issue #1920](https://github.com/manticoresoftware/manticoresearch/issues/1920) 降低了 [low_priority](Searching/Options.md#low_priority) 的节流限制。
* [Issue #1924](https://github.com/manticoresoftware/manticoresearch/issues/1924) Mysqldump + mysql 恢复错误。
* [Issue #1951](https://github.com/manticoresoftware/manticoresearch/issues/1951) 修正了在缺失本地表或 agent 描述错误时分布式表的错误创建；现在返回错误信息。
* [Issue #1972](https://github.com/manticoresoftware/manticoresearch/issues/1972) 实现了 `FREEZE` 计数器以避免冻结/解冻问题。
* [Issue #1980](https://github.com/manticoresoftware/manticoresearch/issues/1980) 在 OR 节点中遵守查询超时。之前某些情况下 `max_query_time` 可能无效。
* [Issue #1986](https://github.com/manticoresoftware/manticoresearch/issues/1986) 未能将 new 重命名为当前的 [manticore.json]。
* [Issue #1988](https://github.com/manticoresoftware/manticoresearch/issues/1988) 全文查询可能忽略了 `SecondaryIndex` CBO 提示。
* [Issue #1990](https://github.com/manticoresoftware/manticoresearch/issues/1990) 修正 `expansion_limit` 以从多个磁盘块或内存块调用关键字切片最终结果集。
* [Issue #1994](https://github.com/manticoresoftware/manticoresearch/issues/1994) 错误的外部文件。
* [Issue #2021](https://github.com/manticoresoftware/manticoresearch/issues/2021) 停止 Manticore 后可能有少量 manticore-executor 进程仍在运行。
* [Issue #2029](https://github.com/manticoresoftware/manticoresearch/issues/2029) 使用 Levenshtein 距离时崩溃。
* [Issue #2037](https://github.com/manticoresoftware/manticoresearch/issues/2037) 多次在空索引上运行 max 运算符后出现错误。
* [Issue #2052](https://github.com/manticoresoftware/manticoresearch/issues/2052) 在 multi-group 配合 JSON.field 时崩溃。
* [Issue #2067](https://github.com/manticoresoftware/manticoresearch/issues/2067) 对 _update 的错误请求导致 Manticore 崩溃。
* [Issue #2069](https://github.com/manticoresoftware/manticoresearch/issues/2069) 修正了 JSON 接口中字符串过滤器封闭范围比较器的问题。
* [Issue #2082](https://github.com/manticoresoftware/manticoresearch/issues/2082) 当 data_dir 路径位于符号链接上时，`alter` 失败。
* [Issue #2102](https://github.com/manticoresoftware/manticoresearch/pull/2102) 改进了 mysqldump 中 SELECT 查询的特殊处理以确保生成的 INSERT 语句兼容 Manticore。
* [Issue #2103](https://github.com/manticoresoftware/manticoresearch/issues/2103) 泰文字符集错误。
* [Issue #2124](https://github.com/manticoresoftware/manticoresearch/issues/2124) 使用带有保留字的 SQL 查询时崩溃。
* [Issue #2154](https://github.com/manticoresoftware/manticoresearch/issues/2154) 带有词形变化的表无法导入。
* [Issue #2176](https://github.com/manticoresoftware/manticoresearch/issues/2176) 修正当引擎参数设置为 'columnar' 且通过 JSON 添加重复 ID 时的崩溃。
* [Issue #223](https://github.com/manticoresoftware/manticoresearch-buddy/issues/223) 在尝试插入无 schema 和无列名的文档时给出适当错误。
* [Issue #239](https://github.com/manticoresoftware/manticoresearch-buddy/issues/239) 自动 schema 多行插入可能失败。
* [Issue #399](https://github.com/manticoresoftware/manticoresearch/issues/399) 如果数据源声明了 id 属性，索引时增加错误提示。
* [Issue #59](https://github.com/manticoresoftware/manticoresearch-helm/issues/59) Manticore 集群故障。
* [Issue #68](https://github.com/manticoresoftware/manticoresearch-helm/issues/68) 如果存在 percolate 表，optimize.php 会崩溃。
* [Issue #77](https://github.com/manticoresoftware/manticoresearch-helm/issues/77) 修正 Kubernetes 部署时的错误。
* [Issue #274](https://github.com/manticoresoftware/manticoresearch-buddy/issues/274) 修正 Buddy 并发请求处理错误。

### 与 manticore-backup 相关
* [Issue #97](https://github.com/manticoresoftware/manticoresearch-backup/pull/97) 当可用时设置 VIP HTTP 端口为默认。
多项改进：增强版本检查和流式 ZSTD 解压；恢复期间版本不匹配时增加用户提示；修正恢复时对不同版本不正确的提示行为；增强解压逻辑，直接从流读取代替写入工作内存；增加 `--force` 标志。
* [Commit 3b35](https://github.com/manticoresoftware/manticoresearch-backup/commit/3b357ea) 在 Manticore 搜索启动后添加备份版本显示以帮助识别此阶段的问题。
* [Commit ad2e](https://github.com/manticoresoftware/manticoresearch-backup/commit/ad2e5da) 更新守护进程连接失败时的错误信息。
* [Commit ce5e](https://github.com/manticoresoftware/manticoresearch-backup/commit/ce5ea85) 修正将绝对根备份路径转换为相对路径的问题，取消恢复时写权限检查以支持从不同路径恢复。
* [Commit db7e](https://github.com/manticoresoftware/manticoresearch-backup/commit/db7e2b9) 向文件迭代器添加排序以确保各种情况下一致性。
* [Issue #106](https://github.com/manticoresoftware/manticoresearch-backup/issues/105) 多配置的备份和恢复。
* [Issue #91](https://github.com/manticoresoftware/manticoresearch-backup/pull/91) 添加 defattr 以防止 RHEL 安装后文件出现异常用户权限。
* [Issue #91](https://github.com/manticoresoftware/manticoresearch-backup/pull/91) 添加额外的 chown 以确保文件默认属于 Ubuntu 中的 root 用户。

### 与 MCL（列式、二级索引、knn 库）相关
* [提交 f104](https://github.com/manticoresoftware/columnar/commit/f104a4709bbb144140d09a09bc0fc1db1895e331) 支持向量搜索。
* [提交 2169](https://github.com/manticoresoftware/columnar/commit/216953beef6676f7e34c9f6f1a30c5980d746710) 修复二级索引构建中断时临时文件清理的问题。此修复解决了守护进程在创建 `tmp.spidx` 文件时超出打开文件限制的问题。
* [提交 709b](https://github.com/manticoresoftware/columnar/commit/709b9acaaac97d9a1ca8796892f9ad432021c785) 对 columnar 和 SI 使用独立的 streamvbyte 库。
* [提交 1c26](https://github.com/manticoresoftware/manticoresearch/commit/1c2623763ced14bb94ad624dde9614a938408d89) 添加警告：columnar 存储不支持 json 属性。
* [提交 3acd](https://github.com/manticoresoftware/columnar/commit/3acd6714ffce103447d7931dfc75bb1a7449f340) 修复 SI 中数据解包。
* [提交 574c](https://github.com/manticoresoftware/manticoresearch/commit/574c023152564fa72092c18ebd1f594abfd4b6d7) 修复保存包含混合行存储和列存储的磁盘块时的崩溃。
* [提交 e87f](https://github.com/manticoresoftware/columnar/commit/e87f088b458d4c5b1b6159fc72ed0d57b786891a) 修复 SI 迭代器被提示已经处理过的块的问题。
* [问题 #1474](https://github.com/manticoresoftware/manticoresearch/issues/1474) 使用 columnar 引擎时，行式 MVA 列的更新失效。
* [问题 #1510](https://github.com/manticoresoftware/manticoresearch/issues/1510) 修复对 `HAVING` 中使用的 columnar 属性进行聚合时发生崩溃。
* [问题 #1519](https://github.com/manticoresoftware/manticoresearch/issues/1519) 修复 `expr` 排名器在使用 columnar 属性时崩溃。

### 与 Docker 相关
* ❗[问题 #42](https://github.com/manticoresoftware/docker/issues/42) 通过环境变量支持 [plain indexation](https://github.com/manticoresoftware/docker#building-plain-tables)。
* ❗[问题 #47](https://github.com/manticoresoftware/docker/issues/47) 通过环境变量改进配置灵活性。
* [问题 #54](https://github.com/manticoresoftware/docker/pull/54) 改进 Docker 的[备份和恢复](https://github.com/manticoresoftware/docker#backup-and-restore)流程。
* [问题 #77](https://github.com/manticoresoftware/docker/issues/76) 改进入口程序，仅在首次启动时处理备份恢复。
* [提交 a27c](https://github.com/manticoresoftware/docker/commit/a27c048dd9e73d8d6bea8d9a4830cafb486b82c5) 修复查询日志输出至 stdout。
* [问题 #38](https://github.com/manticoresoftware/docker/issues/38) 当未设置 EXTRA 时，禁用 BUDDY 警告。
* [问题 #71](https://github.com/manticoresoftware/docker/pull/71) 修复 `manticore.conf.sh` 中的主机名。

## 版本 6.2.12
发布于：2023年8月23日

版本 6.2.12 继续 6.2 系列，解决了 6.2.0 发布后发现的问题。

### Bug 修复
* ❗[问题 #1351](https://github.com/manticoresoftware/manticoresearch/issues/1351) “Manticore 6.2.0 在 Centos 7 上无法通过 systemctl 启动”：将 `TimeoutStartSec` 从 `infinity` 修改为 `0`，以提高与 Centos 7 的兼容性。
* ❗[问题 #1364](https://github.com/manticoresoftware/manticoresearch/issues/1364) “从 6.0.4 升级到 6.2.0 后崩溃”：为旧版本 Binlog 空文件添加了重放功能。
* [PR #1334](https://github.com/manticoresoftware/manticoresearch/pull/1334) “修正 searchdreplication.cpp 中的拼写错误”：更正 `searchdreplication.cpp` 中的错字 beggining -> beginning。
* [问题 #1337](https://github.com/manticoresoftware/manticoresearch/issues/1337) “Manticore 6.2.0 警告：conn (local)(12), sock=8088: bailing on failed MySQL header, AsyncNetInputBuffer_c::AppendData: error 11 (Resource temporarily unavailable) return -1”：将 MySQL 接口关于头部的警告日志级别降低到 logdebugv。
* [问题 #1355](https://github.com/manticoresoftware/manticoresearch/issues/1355) “节点地址无法解析时 join cluster 卡住”：改进了当某些节点无法访问且名称解析失败时的复制重试机制。此改进解决了 Kubernetes 和 Docker 节点中与复制相关的问题。增强了复制启动失败的错误信息，并更新了测试用例 376。同时为名称解析失败提供了清晰的错误信息。
* [问题 #1361](https://github.com/manticoresoftware/manticoresearch/issues/1361) “charset non_cjk 中 ‘Ø’ 字符没有小写映射”：调整了 ‘Ø’ 字符的映射。
* [问题 #1365](https://github.com/manticoresoftware/manticoresearch/issues/1365) “searchd 清理停止后仍保留 binlog.meta 和 binlog.001”：确保正确删除最后一个空的 binlog 文件。
* [提交 0871](https://github.com/manticoresoftware/manticoresearch/commit/0871070845e93ad702ae0dfb89b35e5f24cd0851)：修复了 Windows 下 `Thd_t` 构建中与原子复制限制相关的问题。
* [提交 1cc0](https://github.com/manticoresoftware/manticoresearch/commit/1cc04bedc174737118fd5a3663e35f4e95bd4f8c)：解决 FT CBO 与 `ColumnarScan` 问题。
* [提交 c6bf](https://github.com/manticoresoftware/manticoresearch/commit/c6bfb30d53e3f4b67f17769667733677a661c95b)：修正测试用例 376，并为 `AF_INET` 错误添加替代方案。
* [提交 cbc3](https://github.com/manticoresoftware/manticoresearch/commit/cbc38705e0b4dcc9665e3e8cf64826290ebd11cf)：解决复制时在更新 blob 属性与替换文档间的死锁问题。还移除了提交时对索引的读锁，因为索引已在更底层被锁定。

### 小变更
* [提交 4f91](https://github.com/manticoresoftware/manticoresearch/commit/4f913a36e84e01f950b2797051222b0b1094d6fe) 更新手册中 `/bulk` 端点的信息。

### MCL
* 支持 [Manticore Columnar Library](https://github.com/manticoresoftware/columnar) v2.2.4

## 版本 6.2.0
发布于：2023年8月4日

### 重大变更
* 查询优化器已增强以支持全文本查询，显著提升了搜索效率和性能。
* 集成了：
  - [mysqldump](https://dev.mysql.com/doc/refman/8.0/en/mysqldump.html) - 使用 `mysqldump` [制作逻辑备份](Securing_and_compacting_a_table/Backup_and_restore.md#Backup-and-restore-with-mysqldump)
  - [Apache Superset](https://manticoresearch.com/blog/manticoresearch-apache-superset-integration/) 和 [Grafana](https://manticoresearch.com/blog/manticoresearch-grafana-integration/) 用于可视化存储在 Manticore 中的数据
  - [HeidiSQL](https://www.heidisql.com/) 和 [DBForge](https://www.devart.com/dbforge/) 便于使用 Manticore 进行开发
* 我们开始使用 [GitHub workflows](https://github.com/manticoresoftware/manticoresearch/actions)，使贡献者更容易使用核心团队在准备包时应用的同一持续集成（CI）流程。所有任务都可以在 GitHub 托管的运行器上运行，便于无缝测试您 fork 的 Manticore Search 中的更改。
* 我们开始使用 [CLT](https://github.com/manticoresoftware/clt) 来测试复杂场景。例如，我们现在能够确保一次提交后构建的包能够在所有支持的 Linux 操作系统上正确安装。命令行测试器（CLT）提供了一种交互式录制测试和轻松回放测试的用户友好方式。
* 通过结合使用哈希表和 HyperLogLog，显著提升了 count distinct 操作的性能。
* 支持包含二级索引的查询的多线程执行，线程数限制为物理 CPU 核心数。这应大幅提升查询执行速度。
* `pseudo_sharding` 已调整为限制在可用空闲线程数内。此更新显著提升了吞吐性能。
* 用户现在可以通过配置设置指定[默认属性存储引擎](Server_settings/Searchd.md#engine)，更好地自定义以满足特定工作负载需求。
* 支持 [Manticore Columnar Library 2.2.0](https://github.com/manticoresoftware/columnar/) 以及在[二级索引](Server_settings/Searchd.md#secondary_indexes)中的众多漏洞修复和改进。

### 次要更改
* [Buddy #153](https://github.com/manticoresoftware/manticoresearch-buddy/issues/153)：[/pq](Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md?client=JSON#Adding-rules-to-a-percolate-table) HTTP 端点现在作为 `/json/pq` HTTP 端点的别名。
* [提交 0bf1](https://github.com/manticoresoftware/manticoresearch/commit/0bf17d9e)：确保了 `upper()` 和 `lower()` 的多字节兼容性。
* [提交 2bb9](https://github.com/manticoresoftware/manticoresearch/commit/2bb92765)：对于 `count(*)` 查询，不再扫描索引，而是返回预计算值。
* [提交 3c84](https://github.com/manticoresoftware/manticoresearch/commit/3c84091f)：现在可以使用 `SELECT` 进行任意计算和显示 `@@sysvars`。与以前不同，您不再局限于单个计算。因此，类似 `select user(), database(), @@version_comment, version(), 1+1 as a limit 10` 的查询将返回所有列。注意，'limit' 选项会被忽略。
* [提交 6aca](https://github.com/manticoresoftware/manticoresearch/commit/6aca32f0)：实现了 `CREATE DATABASE` 存根查询。
* [提交 9dc1](https://github.com/manticoresoftware/manticoresearch/commit/9dc12334)：执行 `ALTER TABLE table REBUILD SECONDARY` 时，将始终重建二级索引，即使属性未更新。
* [提交 46ed](https://github.com/manticoresoftware/manticoresearch/commit/46edb089)：使用预计算数据的排序器现在在使用 CBO 前被识别，以避免不必要的 CBO 计算。
* [提交 102a](https://github.com/manticoresoftware/manticoresearch/commit/102ac604)：实现了全文表达式栈的模拟和利用，防止守护进程崩溃。
* [提交 979f](https://github.com/manticoresoftware/manticoresearch/commit/979fa27c)：为不使用字符串/mvas/json 属性的匹配添加了快速的匹配克隆代码路径。
* [提交 a073](https://github.com/manticoresoftware/manticoresearch/commit/a0735ffe)：添加了对 `SELECT DATABASE()` 命令的支持。但它将始终返回 `Manticore`。此添加对与各类 MySQL 工具的集成非常关键。
* [提交 bc04](https://github.com/manticoresoftware/manticoresearch/commit/bc04908)：修改了 [/cli](Connecting_to_the_server/HTTP.md#/cli) 端点的响应格式，新增 `/cli_json` 端点，以替代之前的 `/cli`。
* [提交 d70b](https://github.com/manticoresoftware/manticoresearch/commit/d70b0d58)：`thread_stack` 现在可以通过 `SET` 语句在运行时修改。支持会话本地和守护进程全局两种变体。当前值可在 `show variables` 输出中访问。
* [提交 d96e](https://github.com/manticoresoftware/manticoresearch/commit/d96ec6b9)：代码已集成到 CBO 中，以更准确地估计在字符串属性上执行过滤器的复杂性。
* [提交 e77d](https://github.com/manticoresoftware/manticoresearch/commit/e77dd72f5a04531c352fad0d7afcd2a1cbae2510)：改进了 DocidIndex 成本计算，提升整体性能。
* [提交 f3ae](https://github.com/manticoresoftware/manticoresearch/commit/f3ae8bea)：类似 Linux 上的 'uptime'，负载指标现在可在 `SHOW STATUS` 命令中查看。
* [提交 f3cc](https://github.com/manticoresoftware/manticoresearch/commit/f3cc0971)：`DESC` 和 `SHOW CREATE TABLE` 的字段和属性顺序现在与 `SELECT * FROM` 一致。
* [提交 f3d2](https://github.com/manticoresoftware/manticoresearch/commit/f3d248a6)：不同的内部解析器现在在各种错误中提供其内部助记代码（例如，`P01`）。此增强有助于识别哪个解析器导致了错误，同时也屏蔽了非必要的内部细节。
* [问题 #271](https://github.com/manticoresoftware/manticoresearch/issues/271) “有时 CALL SUGGEST 不会建议单个字母的拼写错误修正”：改进了短词的 [SUGGEST/QSUGGEST](Searching/Spell_correction.md#CALL-QSUGGEST,-CALL-SUGGEST) 行为：添加了 `sentence` 选项以显示整个句子
* [问题 #696](https://github.com/manticoresoftware/manticoresearch/issues/696) “当启用词干处理时，Percolate 索引不能正确进行精确短语查询”：Percolate 查询已修改以处理精确术语修饰符，改进了搜索功能。
* [问题 #829](https://github.com/manticoresoftware/manticoresearch/issues/829) “日期格式化方法”：新增了 [date_format()](Functions/Date_and_time_functions.md#DATE_FORMAT()) 选择列表表达式，暴露了 `strftime()` 函数。
* [问题 #961](https://github.com/manticoresoftware/manticoresearch/issues/961) “通过 HTTP JSON API 对桶进行排序”：在 HTTP 接口中为每个聚合桶引入了可选的 [sort 属性](Searching/Faceted_search.md#HTTP-JSON)。
* [问题 #1062](https://github.com/manticoresoftware/manticoresearch/issues/1062) “改进 JSON 插入 API 失败时的错误日志 - “不支持的值类型””：当发生错误时，`/bulk` 端点会报告已处理和未处理字符串（文档）的数量信息。
* [问题 #1070](https://github.com/manticoresoftware/manticoresearch/issues/1070) “CBO 提示不支持多个属性”：启用了索引提示以处理多个属性。
* [问题 #1106](https://github.com/manticoresoftware/manticoresearch/issues/1106) “向 HTTP 搜索查询添加标签”：已向 [HTTP PQ 响应](Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md?client=JSON#Adding-rules-to-a-percolate-table) 添加了标签。
* [问题 #1301](https://github.com/manticoresoftware/manticoresearch/issues/1301) “buddy 不应并行创建表”：解决了由并行 CREATE TABLE 操作引起的失败问题。现在，仅允许同时运行一个 `CREATE TABLE` 操作。
* [问题 #1303](https://github.com/manticoresoftware/manticoresearch/issues/1303) “支持在列名中使用 @”。
* [问题 #1316](https://github.com/manticoresoftware/manticoresearch/issues/1316) “在 taxi 数据集上 ps=1 的查询很慢”：改进了 CBO 逻辑，并将默认直方图分辨率设置为 8k，以在随机分布的属性值上提高准确性。
* [问题 #1317](https://github.com/manticoresoftware/manticoresearch/issues/1317) “修正 hn 数据集上的 CBO 与全文索引”：增强了确定何时使用位图迭代器交集及何时使用优先队列的逻辑。
* [问题 #1318](https://github.com/manticoresoftware/manticoresearch/issues/1318) “columnar：更改迭代器接口为单次调用”：列式迭代器现在使用单次 `Get` 调用，取代了以前的两步调用 `AdvanceTo` + `Get` 来检索值。
* [问题 #1319](https://github.com/manticoresoftware/manticoresearch/issues/1319) “聚合计算加速（移除 CheckReplaceEntry？）”：从分组排序器中移除了 `CheckReplaceEntry` 调用，以加快聚合函数的计算速度。
* [问题 #1320](https://github.com/manticoresoftware/manticoresearch/issues/1320) “创建表 read_buffer_docs/hits 不支持 k/m/g 语法”：`CREATE TABLE` 选项 `read_buffer_docs` 和 `read_buffer_hits` 现在支持 k/m/g 语法。
* 英语、德语和俄语的[语言包](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology)现在可以通过在 Linux 上执行命令 `apt/yum install manticore-language-packs` 轻松安装。在 macOS 上使用命令 `brew install manticoresoftware/tap/manticore-language-packs`。
* 字段和属性的顺序现在在 `SHOW CREATE TABLE` 和 `DESC` 操作之间保持一致。
* 当执行 `INSERT` 查询时如果磁盘空间不足，则新的 `INSERT` 查询将失败，直到有足够的磁盘空间。
* 新增了 [UINT64()](Functions/Type_casting_functions.md#UINT64%28%29) 类型转换函数。
* `/bulk` 端点现在将空行视为 [commit](Data_creation_and_modification/Transactions.md#BEGIN,-COMMIT,-and-ROLLBACK) 命令。更多信息见[此处](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md?client=JSON#Bulk-adding-documents)。
* 对于[无效的索引提示](Searching/Options.md#Query-optimizer-hints)已实现警告，提供更多透明度并允许错误缓解。
* 当 `count(*)` 与单个过滤器一起使用时，查询现在会利用次级索引的预计算数据（如果可用），大幅提高查询速度。

### ⚠️ 重大更改
* ⚠️ 在版本 6.2.0 中创建或修改的表不能被旧版本读取。
* ⚠️ 文档 ID 现在在索引和 INSERT 操作中作为无符号 64 位整数处理。
* ⚠️ 查询优化器提示的语法已更新。新格式为 `/*+ SecondaryIndex(uid) */`。请注意，旧语法不再支持。
* ⚠️ [问题 #1160](https://github.com/manticoresoftware/manticoresearch/issues/1160)：为避免语法冲突，禁止在表名中使用 `@`。
* ⚠️ 标记为 `indexed` 和 `attribute` 的字符串字段/属性现在在 `INSERT`、`DESC` 和 `ALTER` 操作中视为单个字段。
* ⚠️ [问题 #1057](https://github.com/manticoresoftware/manticoresearch/issues/1057)：不支持 SSE 4.2 的系统将不再加载 MCL 库。
* ⚠️ [问题 #1143](https://github.com/manticoresoftware/manticoresearch/issues/1143): [agent_query_timeout](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_query_timeout) 已损坏。已修复且现已生效。

### Bug修复
* [提交 2a6e](https://github.com/manticoresoftware/manticoresearch/commit/2a6ea8f7) “DROP TABLE时崩溃”：解决了执行DROP TABLE语句时，在RT表上完成写操作（优化、磁盘块保存）等待时间过长的问题。增加了警告，当执行DROP TABLE命令后表目录非空时提醒。
* [提交 2ebd](https://github.com/manticoresoftware/manticoresearch/commit/2ebd424d): 添加了对缺失的列式属性在多属性分组代码中的支持。
* [提交 3be4](https://github.com/manticoresoftware/manticoresearch/commit/3be4503f752216b37b8cde8e47fd2d77bbce63ae) 通过在binlog中正确处理写错误，解决了磁盘空间耗尽可能导致的崩溃问题。
* [提交 6adb](https://github.com/manticoresoftware/manticoresearch/commit/6adb0934): 修复了在查询中使用多个列式扫描迭代器（或二级索引迭代器）时偶尔发生的崩溃。
* [提交 6bd9](https://github.com/manticoresoftware/manticoresearch/commit/6bd9f709): 使用预计算数据的排序器时，过滤器未被移除的问题已修复。
* [提交 6d03](https://github.com/manticoresoftware/manticoresearch/commit/6d03566a): 更新了CBO代码，以提供在多线程执行基于行属性过滤的查询时更好的估计。
* [提交 6dd3](https://github.com/manticoresoftware/manticoresearch/commit/6dd3964), [Helm #56](https://github.com/manticoresoftware/manticoresearch-helm/issues/56) “Kubernetes集群中致命崩溃转储”：修复了JSON根对象的布隆过滤器缺陷；修复了因JSON字段过滤导致的守护进程崩溃。
* [提交 6e1b](https://github.com/manticoresoftware/manticoresearch/commit/6e1b849) 纠正了因`manticore.json`配置无效导致的守护进程崩溃。
* [提交 6fbc](https://github.com/manticoresoftware/manticoresearch/commit/6fbc4189) 修复了json范围过滤器以支持int64值。
* [提交 9c67](https://github.com/manticoresoftware/manticoresearch/commit/9c677426) 修复了`.sph`文件在`ALTER`操作下可能损坏的问题。
* [提交 77cc](https://github.com/manticoresoftware/manticoresearch/commit/77ccd8f): 增加了用于replace语句复制的共享密钥，以解决从多个主节点进行replace复制时发生的`pre_commit`错误。
* [提交 2884](https://github.com/manticoresoftware/manticoresearch/commit/2884519e) 解决了对诸如'date_format()'等函数的bigint检查问题。
* [提交 9513](https://github.com/manticoresoftware/manticoresearch/commit/9513aca5): 当排序器使用预计算数据时，迭代器不再显示在[SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META)中。
* [提交 a2a7](https://github.com/manticoresoftware/manticoresearch/commit/a2a70555): 更新了全文节点堆栈大小，以防止复杂全文查询时崩溃。
* [提交 a062](https://github.com/manticoresoftware/manticoresearch/commit/a0626d7e): 解决了在复制带有JSON和字符串属性的更新时导致崩溃的错误。
* [提交 b3e6](https://github.com/manticoresoftware/manticoresearch/commit/b3e63e5d): 更新了字符串构建器，使用64位整数以避免处理大型数据集时的崩溃。
* [提交 c472](https://github.com/manticoresoftware/manticoresearch/commit/c472e5b): 解决了跨多个索引进行计数去重时出现的崩溃问题。
* [提交 d073](https://github.com/manticoresoftware/manticoresearch/commit/d0730272): 修正了即使禁用`pseudo_sharding`，也可能在多线程中执行RT索引磁盘块查询的问题。
* [提交 d205](https://github.com/manticoresoftware/manticoresearch/commit/d205508e) 修改了`show index status`命令返回的值集，现在取决于所使用的索引类型。
* [提交 e9bc](https://github.com/manticoresoftware/manticoresearch/commit/e9bc2f9) 修复了处理批量请求时的HTTP错误以及错误未从网络循环返回客户端的问题。
* [提交 f77c](https://github.com/manticoresoftware/manticoresearch/commit/f77ce0e6) PQ使用扩展堆栈。
* [提交 fac2](https://github.com/manticoresoftware/manticoresearch/commit/fac2d9687a20582765aaa89be9597804c4f845af) 更新了导出排序器输出，以与[packedfactors()](Functions/Searching_and_ranking_functions.md#PACKEDFACTORS%28%29)保持一致。
* [提交 ff87](https://github.com/manticoresoftware/manticoresearch/commit/ff87cc4): 修复了SphinxQL查询日志中过滤器字符串列表的问题。
* [问题 #589](https://github.com/manticoresoftware/manticoresearch/issues/589) “字符集定义似乎依赖码排序”：修复了重复字符的错误字符集映射。
* [问题 #811](https://github.com/manticoresoftware/manticoresearch/issues/811) “词形多个词映射干扰带有CJK标点的短语搜索”：修复了带词形变化的短语查询中ngram标记位置的问题。
* [问题 #834](https://github.com/manticoresoftware/manticoresearch/issues/834) “搜索查询中的等号符号导致请求失败”：确保精确符号可以被转义，修复了`expand_keywords`选项导致的双重精确扩展。
* [问题 #864](https://github.com/manticoresoftware/manticoresearch/issues/864) “exceptions/stopwords冲突”
* [问题 #910](https://github.com/manticoresoftware/manticoresearch/issues/910) “使用libstemmer_fr和index_exact_words调用snippets()时导致Manticore崩溃”：解决了调用`SNIPPETS()`时造成崩溃的内部冲突。
* [Issue #946](https://github.com/manticoresoftware/manticoresearch/issues/946) “SELECT 查询中出现重复记录”：修复了在对带有 `not_terms_only_allowed` 选项并含有已删除文档的 RT 索引进行查询时，结果集中出现重复文档的问题。
* [Issue #967](https://github.com/manticoresoftware/manticoresearch/issues/967) “在 UDF 函数中使用 JSON 参数导致崩溃”：修复了在启用伪分片并使用带 JSON 参数的 UDF 处理搜索时导致守护进程崩溃的问题。
* [Issue #1050](https://github.com/manticoresoftware/manticoresearch/issues/1050) “FEDERATED 中的 count(*)”：修复了通过带聚合的 `FEDERATED` 引擎查询时导致守护进程崩溃的问题。
* [Issue #1052](https://github.com/manticoresoftware/manticoresearch/issues/1052) 修复了 `rt_attr_json` 列与列存储不兼容的问题。
* [Issue #1072](https://github.com/manticoresoftware/manticoresearch/issues/1072) “* 被 ignore_chars 从搜索查询中移除”：修复该问题，使查询中的通配符不受 `ignore_chars` 影响。
* [Issue #1075](https://github.com/manticoresoftware/manticoresearch/issues/1075) “indextool --check 在存在分布式表时失败”：indextool 现支持 json 配置中包含 ‘distributed’ 和 ‘template’ 索引的实例。
* [Issue #1081](https://github.com/manticoresoftware/manticoresearch/issues/1081) “特定 RT 数据集上的特定 select 导致 searchd 崩溃”：修复了使用 packedfactors 和大型内部缓冲区的查询导致守护进程崩溃的问题。
* [Issue #1095](https://github.com/manticoresoftware/manticoresearch/issues/1095) “使用 not_terms_only_allowed 时被删除的文档被忽略”
* [Issue #1099](https://github.com/manticoresoftware/manticoresearch/issues/1099) “indextool --dumpdocids 不工作”：恢复了 `--dumpdocids` 命令的功能。
* [Issue #1100](https://github.com/manticoresoftware/manticoresearch/issues/1100) “indextool --buildidf 不工作”：indextool 现在在完成 globalidf 后关闭文件。
* [Issue #1104](https://github.com/manticoresoftware/manticoresearch/issues/1104) “Count(*) 在远程表中被错误地当作 schema 集处理”：修复了当代理返回空结果集时，守护进程为分布式索引查询发送错误信息的问题。
* [Issue #1109](https://github.com/manticoresoftware/manticoresearch/issues/1109) “FLUSH ATTRIBUTES 在 threads=1 时挂起”。
* [Issue #1126](https://github.com/manticoresoftware/manticoresearch/issues/1126) “查询期间与 MySQL 服务器失去连接 - manticore 6.0.5”：修复了对列属性使用多个过滤器时发生的崩溃。
* [Issue #1135](https://github.com/manticoresoftware/manticoresearch/issues/1135) “JSON 字符串过滤大小写敏感问题”：修正了 HTTP 搜索请求中过滤器的排序，使其正确工作。
* [Issue #1140](https://github.com/manticoresoftware/manticoresearch/issues/1140) “匹配了错误的字段”：修复了与 `morphology_skip_fields` 相关的问题。
* [Issue #1155](https://github.com/manticoresoftware/manticoresearch/issues/1155) “通过 API 进行系统远程命令应传递 g_iMaxPacketSize”：更新以绕过节点间复制命令的 `max_packet_size` 检查。此外，状态显示新增了最新的集群错误。
* [Issue #1302](https://github.com/manticoresoftware/manticoresearch/issues/1302) “优化失败时留下临时文件”：修正了合并或优化过程出错后遗留临时文件的问题。
* [Issue #1304](https://github.com/manticoresoftware/manticoresearch/issues/1304) “添加用于 buddy 启动超时的环境变量”：添加了环境变量 `MANTICORE_BUDDY_TIMEOUT`（默认为 3 秒），用于控制守护进程启动时等待 buddy 消息的时长。
* [Issue #1305](https://github.com/manticoresoftware/manticoresearch/issues/1305) “保存 PQ 元数据时整数溢出”：缓解了守护进程保存大型 PQ 索引时的过度内存消耗。
* [Issue #1306](https://github.com/manticoresoftware/manticoresearch/issues/1306) “更改 RT 表的外部文件后无法重新创建 RT 表”：修正了外部文件为空字符串的 alter 操作错误；修复了更改外部文件后遗留的 RT 索引外部文件问题。
* [Issue #1307](https://github.com/manticoresoftware/manticoresearch/issues/1307) “SELECT 语句中 sum(value) 作为 value 工作不正常”：修复了带别名的选择列表表达式可能隐藏索引属性的问题；还修复了整数的 sum 转换为 int64 计数的问题。
* [Issue #1308](https://github.com/manticoresoftware/manticoresearch/issues/1308) “避免在复制时绑定到 localhost”：确保复制在主机名有多个 IP 时不绑定到 localhost。
* [Issue #1309](https://github.com/manticoresoftware/manticoresearch/issues/1309) “向 mysql 客户端响应大于 16Mb 的数据失败”：修正了向客户端返回大于 16Mb 的 SphinxQL 包的问题。
* [Issue #1310](https://github.com/manticoresoftware/manticoresearch/issues/1310) “‘外部文件路径应为绝对路径’中的错误引用”：修正了 `SHOW CREATE TABLE` 中外部文件完整路径的显示。
* [Issue #1311](https://github.com/manticoresoftware/manticoresearch/issues/1311) “调试版本在 snippets 中遇到长字符串时崩溃”：现在允许 `SNIPPET()` 函数处理长度超过 255 字符的文本。
* [Issue #1312](https://github.com/manticoresoftware/manticoresearch/issues/1312) “kqueue 轮询中的用后删除导致的虚假崩溃（主-代理）”：修复了主机在无法连接到代理的 kqueue 驱动系统（FreeBSD、MacOS 等）上发生崩溃的问题。
* [Issue #1313](https://github.com/manticoresoftware/manticoresearch/issues/1313) “连接自身超时过长”：当主机连接到 MacOS/BSD 上的代理时，统一使用连接+查询超时，而非仅使用连接超时。
* [Issue #1314](https://github.com/manticoresoftware/manticoresearch/issues/1314) "pq (json meta) with unreached embedded synonyms fails to load": 修复了 pq 中嵌入的同义词标志。
* [Issue #1315](https://github.com/manticoresoftware/manticoresearch/issues/1315) "允许某些函数（sint、fibonacci、second、minute、hour、day、month、year、yearmonth、yearmonthday）使用隐式提升的参数值"。
* [Issue #1321](https://github.com/manticoresoftware/manticoresearch/issues/1321) "在 fullscan 中启用多线程 SI，但限制线程数": 代码已实现到 CBO 中，以更好地预测在全文查询中使用二级索引时的多线程性能。
* [Issue #1322](https://github.com/manticoresoftware/manticoresearch/issues/1322) "使用预计算排序器后，count(*) 查询仍然缓慢": 在使用预计算数据的排序器时，不再初始化迭代器，避免了性能下降。
* [Issue #1411](https://github.com/manticoresoftware/manticoresearch/issues/1141) "sphinxql 的查询日志未保留 MVA 的原始查询": 现在，会记录 `all()/any()`。

## 版本 6.0.4
发布时间：2023年3月15日

### 新功能
* 改进了与 Logstash、Beats 等的集成，包括：
  - 支持 Logstash 版本 7.6 - 7.15，Filebeat 版本 7.7 - 7.12
  - 自动模式支持。
  - 新增对类似 Elasticsearch 格式批量请求的处理。
* [Buddy 提交 ce90](https://github.com/manticoresoftware/manticoresearch-buddy/commit/ce907ea) 在 Manticore 启动时记录 Buddy 版本。

### 修复
* [Issue #588](https://github.com/manticoresoftware/manticoresearch/issues/588), [Issue #942](https://github.com/manticoresoftware/manticoresearch/issues/942) 修复双字索引搜索元数据和调用关键词中的错误字符。
* [Issue #1027](https://github.com/manticoresoftware/manticoresearch/issues/1027) 拒绝小写的 HTTP 头。
* ❗[Issue #1039](https://github.com/manticoresoftware/manticoresearch/issues/1039) 修复了守护进程在读取 Buddy 控制台输出时的内存泄漏。
* [Issue #1056](https://github.com/manticoresoftware/manticoresearch/issues/1056) 修复问号的异常行为。
* [Issue #1064](https://github.com/manticoresoftware/manticoresearch/issues/1064) - 修复分词器小写表中的竞态条件导致崩溃。
* [Commit 59bb](https://github.com/manticoresoftware/manticoresearch/commit/59bb54c) 修复 JSON 接口中对 id 明确设置为 null 的文档的批量写入处理。
* [Commit 7b6b](https://github.com/manticoresoftware/manticoresearch/commit/7b6b25f) 修复 CALL KEYWORDS 中多个相同术语的术语统计。
* [Commit f381](https://github.com/manticoresoftware/manticoresearch/commit/f381ad2) Windows 安装程序现在创建默认配置；路径不再在运行时替换。
* [Commit 6940](https://github.com/manticoresoftware/manticoresearch/commit/6940e95), [Commit cc5a](https://github.com/manticoresoftware/manticoresearch/commit/cc5a480) 修复跨多个网络节点的集群复制问题。
* [Commit 4972](https://github.com/manticoresoftware/manticoresearch/commit/49722ab) 修复 `/pq` HTTP 端点作为 `/json/pq` HTTP 端点的别名。
* [Commit 3b53](https://github.com/manticoresoftware/manticoresearch/commit/3b5385a) 修复 Buddy 重启导致的守护进程崩溃。
* [Buddy 提交 fba9](https://github.com/manticoresoftware/manticoresearch-buddy/commit/fba9c8c) 显示收到的无效请求的原始错误。
* [Buddy 提交 db95](https://github.com/manticoresoftware/manticoresearch-buddy/commit/db9532c) 允许备份路径中包含空格，并对正则表达式做了一些魔法以支持单引号。

## 版本 6.0.2
发布时间：2023年2月10日

### 修复
* [Issue #1024 crash 2](https://github.com/manticoresoftware/manticoresearch/issues/1024) Facet 搜索在结果数量较多时崩溃/段错误。
* ❗[Issue #1029](https://github.com/manticoresoftware/manticoresearch/issues/1029) - 警告：编译时值 KNOWN_CREATE_SIZE (16) 小于测量值 (208)。建议修正该值！
* ❗[Issue #1032](https://github.com/manticoresoftware/manticoresearch/issues/1032) - Manticore 6.0.0 纯索引崩溃。
* ❗[Issue #1033](https://github.com/manticoresoftware/manticoresearch/issues/1033) - 守护进程重启时多个分布式节点丢失。
* ❗[Issue #1064](https://github.com/manticoresoftware/manticoresearch/issues/1064) - 分词器小写表中的竞态条件。

## 版本 6.0.0
发布时间：2023年2月7日

从此版本开始，Manticore Search 附带 Manticore Buddy，这是一个用 PHP 编写的 sidecar 守护进程，负责处理不需要超低延迟或高吞吐量的高层功能。Manticore Buddy 在后台运行，用户可能察觉不到它的存在。尽管对终端用户不可见，但使 Manticore Buddy 易于安装并与基于 C++ 的主守护进程兼容是一大挑战。此重大变更将使团队能够开发各种新的高级功能，如分片调度、访问控制和身份验证，以及 mysqldump、DBeaver、Grafana mysql 连接器等多种集成。目前它已经支持 [SHOW QUERIES](Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES)、[BACKUP](Securing_and_compacting_a_table/Backup_and_restore.md#BACKUP-SQL-command-reference) 和 [自动模式](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema)。

本次发布还包括 130 多项错误修复和众多新功能，其中许多可视为重大改进。

### 重大变更
* 🔬 实验性功能：您现在可以执行与 Elasticsearch 兼容的 [insert](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md?example=Elasticsearch#Adding-documents-to-a-real-time-table) 和 [replace](Data_creation_and_modification/Updating_documents/REPLACE.md?example=Elasticsearch#REPLACE) JSON 查询，这使得 Manticore 能够与 Logstash（版本 < 7.13）、Filebeat 以及 Beats 系列的其他工具配合使用。默认启用。您可以使用 `SET GLOBAL ES_COMPAT=off` 来禁用它。
* 支持 [Manticore Columnar Library 2.0.0](https://github.com/manticoresoftware/columnar/) 以及在 [次级索引](Server_settings/Searchd.md#secondary_indexes) 中的众多修复和改进。**⚠️ 破坏性更新**：此版本默认启用次级索引。如果您从 Manticore 5 升级，请确保执行 [ALTER TABLE table_name REBUILD SECONDARY](Updating_table_schema_and_settings.md#Rebuilding-a-secondary-index)。详情见下文。
* [提交 c436](https://github.com/manticoresoftware/manticoresearch/commit/c436f9023536f767610451911955ae36d90aa638) 自动模式：您现在可以跳过创建表，直接插入第一条文档，Manticore 会根据其字段自动创建表。更多详细内容请查阅 [此处](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema)。您可以通过 [searchd.auto_schema](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema) 进行开启/关闭。
* 对 [基于成本的优化器](Searching/Cost_based_optimizer.md) 进行了大幅度改进，在许多情况下降低了查询响应时间。
  - [问题 #1008](https://github.com/manticoresoftware/manticoresearch/issues/1008) 成本基优化器中的并行性能估计。
  - [问题 #1014](https://github.com/manticoresoftware/manticoresearch/issues/1014) 成本优化器现在支持识别 [次级索引](Server_settings/Searchd.md#secondary_indexes)，行为更智能。
  - [提交 cef9](https://github.com/manticoresoftware/columnar/commit/cef9b54dc3a4ee9138ebc4df3a671715b996d6b3) 现在将列存表/字段的编码统计信息存储到元数据中，帮助成本优化器更智能地决策。
  - [提交 2b95](https://github.com/manticoresoftware/manticoresearch/commit/2b95fb8c2ba422c9c5a66e2b61c1f7922f31c7aa) 添加了成本优化器的提示参数以精细调整其行为。
* [遥测](Telemetry.md#Telemetry)：我们很高兴在此版本中加入了遥测功能。该功能允许我们收集匿名和脱敏的指标，帮助提升产品性能和用户体验。请放心，所有收集的数据**完全匿名，且不会与任何个人信息相关联**。如需要，可以在设置中 [轻松关闭](Telemetry.md#Telemetry)此功能。
* [提交 5aaf](https://github.com/manticoresoftware/manticoresearch/commit/5aafb17d81b601e399e212a344a4738f65576cdd) 添加了 [ALTER TABLE table_name REBUILD SECONDARY](Updating_table_schema_and_settings.md#Rebuilding-a-secondary-index) 命令，可在需要时重建次级索引，例如：
  - 从 Manticore 5 升级到新版时，
  - 对索引中的属性进行 [UPDATE](Data_creation_and_modification/Updating_documents/UPDATE.md#UPDATE)（即[就地更新，而非替换](Data_creation_and_modification/Updating_documents/REPLACE_vs_UPDATE.md)）时
* [问题 #821](https://github.com/manticoresoftware/manticoresearch/issues/821) 新增工具 `manticore-backup` 用于 [备份和恢复 Manticore 实例](Securing_and_compacting_a_table/Backup_and_restore.md)
* SQL 命令 [BACKUP](Securing_and_compacting_a_table/Backup_and_restore.md#BACKUP-SQL-command-reference) 用于从 Manticore 内部执行备份。
* SQL 命令 [SHOW QUERIES](Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES) 提供查看运行中的查询（而非线程）的简易方式。
* [问题 #551](https://github.com/manticoresoftware/manticoresearch/issues/551) 新增 SQL 命令 `KILL` 用于终止长时间运行的 `SELECT`。
* 聚合查询的动态 `max_matches` 设置，用于提升准确率并降低响应时间。

### 小改动
* [问题 #822](https://github.com/manticoresoftware/manticoresearch/issues/822) SQL 命令 [FREEZE/UNFREEZE](Securing_and_compacting_a_table/Freezing_a_table.md) 用于准备实时/普通表的备份。
* [提交 c470](https://github.com/manticoresoftware/manticoresearch/commit/c47052f18ad4868134de1a97e8d7da570e531dfa) 新增设置 `accurate_aggregation` 和 `max_matches_increase_threshold` 用于控制聚合准确性。
* [问题 #718](https://github.com/manticoresoftware/manticoresearch/issues/718) 支持有符号的负 64 位 ID。注意，您依然不能使用超过 2^63 的 ID，但现在可以使用从 -2^63 到 0 范围内的 ID。
* 由于最近新增了对次级索引的支持，"index" 一词在不同上下文中可能指代次级索引、全文索引或普通/实时 `index`。为减少混淆，我们将后者重新命名为“table”。以下 SQL/命令行命令因此受影响。它们的旧版本已废弃，但仍可用：
  - `index <table name>` => `table <table name>`，
  - `searchd -i / --index` => `searchd -t / --table`，
  - `SHOW INDEX STATUS` => `SHOW TABLE STATUS`，
  - `SHOW INDEX SETTINGS` => `SHOW TABLE SETTINGS`，
  - `FLUSH RTINDEX` => `FLUSH TABLE`，
  - `OPTIMIZE INDEX` => `OPTIMIZE TABLE`，
  - `ATTACH TABLE plain TO RTINDEX rt` => `ATTACH TABLE plain TO TABLE rt`，
  - `RELOAD INDEX` => `RELOAD TABLE`，
  - `RELOAD INDEXES` => `RELOAD TABLES`。

  我们不打算废弃旧形式，但为了确保文档兼容，建议您在应用中更改命令。未来版本中，SQL 和 JSON 命令输出中将把 “index” 改为 “table”。
* 带状态的 UDF 查询现在强制在单线程中执行。
* [Issue #1011](https://github.com/manticoresoftware/manticoresearch/issues/1011) 重构所有与时间调度相关的内容，作为并行块合并的前提条件。
* **⚠️ 不兼容变更**：列存储格式已更改。您需要重建具有列属性的那些表。
* **⚠️ 不兼容变更**：二级索引文件格式已更改，因此如果您使用二级索引进行搜索，并且在配置文件中有 `searchd.secondary_indexes = 1`，请注意新版本的 Manticore **将跳过加载带有二级索引的表**。建议：
  - 升级前在配置文件中将 `searchd.secondary_indexes` 改为 0。
  - 启动实例。Manticore 将加载表，并发出警告。
  - 对每个索引运行 `ALTER TABLE <table name> REBUILD SECONDARY` 以重建二级索引。

  如果您运行的是复制集群，您需要在所有节点上运行 `ALTER TABLE <table name> REBUILD SECONDARY`，或者按照[此说明](Securing_and_compacting_a_table/Compacting_a_table.md#Optimizing-clustered-tables)操作，只需将 `OPTIMIZE` 替换为运行 `ALTER .. REBUILD SECONDARY`。
* **⚠️ 不兼容变更**：binlog 版本已更新，因此之前版本的任何 binlog 都不会被重放。升级过程中请确保 Manticore Search 干净地停止。这意味着停止之前的实例后，`/var/lib/manticore/binlog/` 目录内除 `binlog.meta` 外不应有任何 binlog 文件。
* [Issue #849](https://github.com/manticoresoftware/manticoresearch/issues/849) `SHOW SETTINGS`：现在可以在 Manticore 内部查看配置文件中的设置。
* [Issue #1007](https://github.com/manticoresoftware/manticoresearch/issues/1007) [SET GLOBAL CPUSTATS=1/0](Server_settings/Setting_variables_online.md#SET) 开启/关闭 CPU 时间跟踪；[SHOW THREADS](Node_info_and_management/SHOW_THREADS.md) 在关闭 CPU 时间跟踪时不显示 CPU 统计信息。
* [Issue #1009](https://github.com/manticoresoftware/manticoresearch/issues/1009) RT 表的 RAM 块段现在可以在 RAM 块刷新时进行合并。
* [Issue #1012](https://github.com/manticoresoftware/manticoresearch/issues/1012) 向 [indexer](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) 的输出添加了二级索引进度信息。
* [Issue #1013](https://github.com/manticoresoftware/manticoresearch/issues/1013) 之前如果 Manticore 启动时无法开始提供服务，则可能从索引列表中移除该表记录。新行为是保留其在列表中，以便下次启动时尝试加载。
* [indextool --docextract](Miscellaneous_tools.md#indextool) 返回属于请求文档的所有词和命中。
* [Commit 2b29](https://github.com/manticoresoftware/manticoresearch/commit/2b296ee20e520b85bcbb4383e87095d31e7165dc) 环境变量 `dump_corrupt_meta` 可使得在 searchd 无法加载索引时，将损坏的表元数据转储到日志。
* [Commit c7a3](https://github.com/manticoresoftware/manticoresearch/commit/c7a3136eefe0eb2f69e3d99fae9c10beb5821ea6) `DEBUG META` 现在可以显示 `max_matches` 和伪分片统计信息。
* [Commit 6bca](https://github.com/manticoresoftware/manticoresearch/commit/6bca4848dc5c8396dbc1e057be4b9b2d579794e8) 提供了比“索引头格式不是 JSON，将尝试作为二进制...”更好的错误信息。
* [Commit bef3](https://github.com/manticoresoftware/lemmatizer-uk/commit/bef3ff0386d3ee87ec57619782100972c1122e47) 乌克兰语词形还原库路径已更改。
* [Commit 4ae7](https://github.com/manticoresoftware/manticoresearch/commit/4ae789595329a2951e194d1191ddb3121459a560) [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META) 中添加了二级索引统计信息。
* [Commit 2e7c](https://github.com/manticoresoftware/manticoresearch/commit/2e7c585e) JSON 接口现在可以通过 Swagger Editor 轻松可视化：https://manual.manticoresearch.com/Openapi#OpenAPI-specification。
* **⚠️ 不兼容变更**：复制协议已更改。如果您运行复制集群，升级到 Manticore 5 时需要：
  - 首先干净地停止所有节点
  - 然后用 `--new-cluster` 启动最后停止的节点（Linux 下运行工具 `manticore_new_cluster`）。
  - 详情请阅读[重启集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster)。

### 与 Manticore 列存库相关的更改
* 重构了二级索引与列存存储的集成。
* [Commit efe2](https://github.com/manticoresoftware/columnar/commit/efe26b2a) Manticore 列存库优化，可以通过部分预先的最小/最大值评估降低响应时间。
* [Commit 2757](https://github.com/manticoresoftware/columnar/commit/2757b99b) 如果磁盘块合并被中断，守护进程现在会清理与 MCL 相关的临时文件。
* [Commit e9c6](https://github.com/manticoresoftware/manticoresearch/commit/e9c672014b63e2b49e7a5a53880583af86c3ab4e) 崩溃时日志中会转储列存和二级库版本。
* [Commit f5e8](https://github.com/manticoresoftware/columnar/commit/f5e84eeb8b6cb102f13e3468246702427527fef9) 增加了对二级索引的快速文档列表回绕支持。
* [Commit 06df](https://github.com/manticoresoftware/manticoresearch/commit/06dfdd2e668933495dd6cb52d5ba2e0aa9b26e58) 使用 MCL 时，像 `select attr, count(*) from plain_index`（无过滤条件）的查询现在更快。
* [Commit 0a76](https://github.com/manticoresoftware/manticoresearch/commit/0a76ecb9adbe886868b962dd425de5f475bbf55e) `HandleMysqlSelectSysvar` 中的 @@autocommit 以兼容 mysql 8.25 以上版本的 .net 连接器。
* **⚠️ 不兼容变更**： [MCL Issue #17](https://github.com/manticoresoftware/columnar/issues/17) MCL：为列扫描添加 SSE 代码。MCL 现在至少要求 SSE4.2。

### 与打包相关的更改
* [Commit 4d19](https://github.com/manticoresoftware/manticoresearch/commit/4d19f5cbe49a31228aa09253f061165bfe80e51b) **⚠️ 重大变更**：不再支持 Debian Stretch 和 Ubuntu Xenial。
* 支持 RHEL 9，包括 Centos 9、Alma Linux 9 和 Oracle Linux 9。
* [Issue #924](https://github.com/manticoresoftware/manticoresearch/issues/924) 支持 Debian Bookworm。
* [Issue #636](https://github.com/manticoresoftware/manticoresearch/issues/636) 打包：Linux 和 MacOS 的 arm64 构建。
* [PR #26](https://github.com/manticoresoftware/docker/pull/26) 多架构（x86_64 / arm64）docker 镜像。
* [为贡献者简化的包构建](Installation/Compiling_from_sources.md#Building-using-CI-Docker)。
* 现在可以使用 APT 安装特定版本。
* [Commit a6b8](https://github.com/manticoresoftware/manticoresearch/commit/51fddca5c2a3ebb8576fae4c18660656ba22de0f) Windows 安装程序（之前只提供压缩包）。
* 切换到使用 CLang 15 编译。
* **⚠️ 重大变更**：自定义 Homebrew 配方，包括 Manticore Columnar Library 的配方。安装 Manticore、MCL 及其他必要组件，请使用以下命令 `brew install manticoresoftware/manticore/manticoresearch manticoresoftware/manticore/manticore-extra` 。

### Bug 修复
* [Issue #479](https://github.com/manticoresoftware/manticoresearch/issues/479) 名为 `text` 的字段问题
* [Issue #501](https://github.com/manticoresoftware/manticoresearch/issues/501) id 不能是非 bigint 类型
* [Issue #646](https://github.com/manticoresoftware/manticoresearch/issues/646) ALTER 与名为 "text" 字段的问题
* ❗[Issue #652](https://github.com/manticoresoftware/manticoresearch/issues/652) 可能的 BUG：HTTP (JSON) 的 offset 和 limit 影响了 facet 结果
* ❗[Issue #827](https://github.com/manticoresoftware/manticoresearch/issues/827) Searchd 在高负载下挂起/崩溃
* ❗[Issue #996](https://github.com/manticoresoftware/manticoresearch/issues/996) PQ 索引内存溢出
* ❗[Commit 1041](https://github.com/manticoresoftware/manticoresearch/commit/10416ef7dddf06c0d759e32ccd6ebaa2468f7cbf) `binlog_flush = 1` 从 Sphinx 以来一直有问题。已修复。
* [MCL Issue #14](https://github.com/manticoresoftware/columnar/issues/14) MCL：当 ft 字段太多时，select 导致崩溃
* [Issue #470](https://github.com/manticoresoftware/manticoresearch/issues/470) sql_joined_field 无法存储
* [Issue #713](https://github.com/manticoresoftware/manticoresearch/issues/713) 使用 LEVENSHTEIN() 时崩溃
* [Issue #743](https://github.com/manticoresoftware/manticoresearch/issues/743) Manticore 意外崩溃，且无法正常重启
* [Issue #788](https://github.com/manticoresoftware/manticoresearch/issues/788) 通过 /sql 调用 KEYWORDS 返回控制字符，导致 json 解析失败
* [Issue #789](https://github.com/manticoresoftware/manticoresearch/issues/789) mariadb 无法创建 FEDERATED 表
* [Issue #796](https://github.com/manticoresoftware/manticoresearch/issues/796) 警告：dlopen() 失败：/usr/bin/lib_manticore_columnar.so：无法打开共享对象文件：没有此类文件或目录
* [Issue #797](https://github.com/manticoresoftware/manticoresearch/issues/797) 通过 api 使用 ZONESPAN 搜索时 Manticore 崩溃
* [Issue #799](https://github.com/manticoresoftware/manticoresearch/issues/799) 多索引和 facet distinct 时权重错误
* [Issue #801](https://github.com/manticoresoftware/manticoresearch/issues/801) SQL 索引重处理后，SphinxQL 分组查询挂起
* [Issue #802](https://github.com/manticoresoftware/manticoresearch/issues/802) MCL：1.15.4 版本及 5.0.2 版索引器崩溃
* [Issue #813](https://github.com/manticoresoftware/manticoresearch/issues/813) Manticore 5.0.2 FEDERATED 返回空集合（MySQL 8.0.28）
* [Issue #824](https://github.com/manticoresoftware/manticoresearch/issues/824) 在两个索引上 select COUNT DISTINCT，结果为零时抛出内部错误
* [Issue #826](https://github.com/manticoresoftware/manticoresearch/issues/826) 删除查询时崩溃
* [Issue #843](https://github.com/manticoresoftware/manticoresearch/issues/843) MCL：长文本字段的错误
* [Issue #856](https://github.com/manticoresoftware/manticoresearch/issues/856) 5.0.2 rtindex：聚合搜索限制行为与预期不符
* [Issue #863](https://github.com/manticoresoftware/manticoresearch/issues/863) Hits 返回 Nonetype 对象，即使搜索应返回多个结果
* [Issue #870](https://github.com/manticoresoftware/manticoresearch/issues/870) SELECT 表达式中使用 Attribute 和 Stored Field 导致崩溃
* [Issue #872](https://github.com/manticoresoftware/manticoresearch/issues/872) 崩溃后表变为不可见
* [Issue #877](https://github.com/manticoresoftware/manticoresearch/issues/877) 搜索查询中两个负项导致错误：查询不可计算
* [Issue #878](https://github.com/manticoresoftware/manticoresearch/issues/878) 通过 json query_string 使用 a -b -c 无效
* [Issue #886](https://github.com/manticoresoftware/manticoresearch/issues/886) 伪分片与查询匹配问题
* [Issue #893](https://github.com/manticoresoftware/manticoresearch/issues/893) Manticore 5.0.2 min/max 函数未按预期工作……
* [Issue #896](https://github.com/manticoresoftware/manticoresearch/issues/896) 字段 "weight" 解析不正确
* [Issue #897](https://github.com/manticoresoftware/manticoresearch/issues/897) Manticore 服务启动时崩溃并持续重启
* [Issue #900](https://github.com/manticoresoftware/manticoresearch/issues/900) 按 j.a 分组，结果错误
* [Issue #913](https://github.com/manticoresoftware/manticoresearch/issues/913) 当在 ranker 中使用 expr 时，且查询包含两个接近谓词时，Searchd 崩溃
* [Issue #916](https://github.com/manticoresoftware/manticoresearch/issues/916) net_throttle_action 失效
* [问题 #919](https://github.com/manticoresoftware/manticoresearch/issues/919) MCL：Manticore 在查询执行时崩溃以及集群恢复期间的其他崩溃。
* [问题 #925](https://github.com/manticoresoftware/manticoresearch/issues/925) SHOW CREATE TABLE 不带反引号输出
* [问题 #930](https://github.com/manticoresoftware/manticoresearch/issues/930) 现在可以通过 JDBC 连接器从 Java 查询 Manticore
* [问题 #933](https://github.com/manticoresoftware/manticoresearch/issues/933) bm25f 排名问题
* [问题 #934](https://github.com/manticoresoftware/manticoresearch/issues/934) 无配置索引在 watchdog 的首次加载状态被冻结
* [问题 #937](https://github.com/manticoresoftware/manticoresearch/issues/937) 对 facet 数据排序时段错误
* [问题 #940](https://github.com/manticoresoftware/manticoresearch/issues/940) 使用 CONCAT(TO_STRING) 时崩溃
* [问题 #947](https://github.com/manticoresoftware/manticoresearch/issues/947) 在某些情况下，一个简单的 select 查询可能导致整个实例挂起，直到该查询完成，无法登录或执行其他查询。
* [问题 #948](https://github.com/manticoresoftware/manticoresearch/issues/948) Indexer 崩溃
* [问题 #950](https://github.com/manticoresoftware/manticoresearch/issues/950) facet distinct 计数错误
* [问题 #953](https://github.com/manticoresoftware/manticoresearch/issues/953) 内置 sph04 排名器中 LCS 计算错误
* [问题 #955](https://github.com/manticoresoftware/manticoresearch/issues/955) 5.0.3 版本开发崩溃
* [问题 #963](https://github.com/manticoresoftware/manticoresearch/issues/963) FACET 在引擎 columnar 上使用 JSON 时崩溃
* [问题 #982](https://github.com/manticoresoftware/manticoresearch/issues/982) MCL：5.0.3 版本二级索引崩溃
* [PR #984](https://github.com/manticoresoftware/manticoresearch/pull/984) HandleMysqlSelectSysvar 中的 @@autocommit
* [PR #985](https://github.com/manticoresoftware/manticoresearch/pull/985) 修正 RT 索引中的线程块分配
* [问题 #985](https://github.com/manticoresoftware/manticoresearch/issues/985) 修正 RT 索引中的线程块分配
* [问题 #986](https://github.com/manticoresoftware/manticoresearch/issues/986) 默认 max_query_time 错误
* [问题 #987](https://github.com/manticoresoftware/manticoresearch/issues/987) 多线程执行中使用正则表达式时崩溃
* [问题 #988](https://github.com/manticoresoftware/manticoresearch/issues/988) 反向索引兼容性损坏
* [问题 #989](https://github.com/manticoresoftware/manticoresearch/issues/989) indextool 检查 columnar 属性时报错
* [问题 #990](https://github.com/manticoresoftware/manticoresearch/issues/990) json grouper 克隆时内存泄漏
* [问题 #991](https://github.com/manticoresoftware/manticoresearch/issues/991) levenshtein 函数克隆时内存泄漏
* [问题 #992](https://github.com/manticoresoftware/manticoresearch/issues/992) 加载 meta 时错误信息丢失
* [问题 #993](https://github.com/manticoresoftware/manticoresearch/issues/993) 传播来自动态索引/子键和系统变量的错误
* [问题 #994](https://github.com/manticoresoftware/manticoresearch/issues/994) 在 columnar 存储中对列式字符串进行 count distinct 时崩溃
* [问题 #995](https://github.com/manticoresoftware/manticoresearch/issues/995) MCL：taxi1 的 min(pickup_datetime) 查询崩溃
* [问题 #997](https://github.com/manticoresoftware/manticoresearch/issues/997) 空 excludes JSON 查询从选择列表中移除列
* [问题 #998](https://github.com/manticoresoftware/manticoresearch/issues/998) 当前调度器上运行的次要任务有时导致异常副作用
* [问题 #999](https://github.com/manticoresoftware/manticoresearch/issues/999) facet distinct 与不同模式组合时崩溃
* [问题 #1000](https://github.com/manticoresoftware/manticoresearch/issues/1000) MCL：未使用 columnar 库运行后列式 rt 索引损坏
* [问题 #1001](https://github.com/manticoresoftware/manticoresearch/issues/1001) json 中隐式 cutoff 无效
* [问题 #1002](https://github.com/manticoresoftware/manticoresearch/issues/1002) 列式分组器问题
* [问题 #1003](https://github.com/manticoresoftware/manticoresearch/issues/1003) 无法删除索引中的最后一个字段
* [问题 #1004](https://github.com/manticoresoftware/manticoresearch/issues/1004) --new-cluster 之后行为错误
* [问题 #1005](https://github.com/manticoresoftware/manticoresearch/issues/1005) “columnar library not loaded”，但其实不需要
* [问题 #1006](https://github.com/manticoresoftware/manticoresearch/issues/1006) 删除查询无错误提示
* [问题 #1010](https://github.com/manticoresoftware/manticoresearch/issues/1010) 修复 Windows 构建中的 ICU 数据文件位置
* [PR #1018](https://github.com/manticoresoftware/manticoresearch/pull/1018) 握手发送问题
* [问题 #1020](https://github.com/manticoresoftware/manticoresearch/issues/1020) 在 show create table 中显示 id
* [问题 #1024 crash 1](https://github.com/manticoresoftware/manticoresearch/issues/1024) Facet 搜索大量结果时崩溃/段错误。
* [问题 #1026](https://github.com/manticoresoftware/manticoresearch/issues/1026) RT 索引：大量文档插入且 RAMchunk 满时 searchd “永远卡住”
* [提交 4739](https://github.com/manticoresoftware/manticoresearch/commit/4739dafd) 关闭时线程卡住，复制节点间忙碌导致
* [提交 ab87](https://github.com/manticoresoftware/manticoresearch/commit/ab87836782e7ae43fe5f9dea739eed0d336b42c1) JSON 范围过滤中混合浮点和整数导致 Manticore 忽略过滤器
* [提交 d001](https://github.com/manticoresoftware/manticoresearch/commit/d00101c2905f6393ce3fca23c4f6dcb2506f4bd9) JSON 中的浮点过滤器不准确
* [提交 4092](https://github.com/manticoresoftware/manticoresearch/commit/4092c6a8) 索引修改时丢弃未提交事务（否则可能崩溃）
* [Commit 9692](https://github.com/manticoresoftware/manticoresearch/commit/9692a840) 使用反斜杠时查询语法错误
* [Commit 0c19](https://github.com/manticoresoftware/manticoresearch/commit/0c19ad8d) SHOW STATUS 中的 workers_clients 可能错误
* [Commit 1772](https://github.com/manticoresoftware/manticoresearch/commit/17720e1e2f2ec38337df565bee78f7ce9d9c744a) 修复合并无文档存储的内存段时崩溃的问题
* [Commit f45b](https://github.com/manticoresoftware/manticoresearch/commit/f45b337b5c265433bb7d6262ad65d02f65edbaf3) 纠正缺失的等于 JSON 过滤器中的 ALL/ANY 条件
* [Commit 3e83](https://github.com/manticoresoftware/manticoresearch/commit/3e83c1f0c0e6e7a9d4297113f69a112cbabd9e64) 如果 searchd 是从无法写入的目录启动，复制可能失败并出现 `got exception while reading ist stream: mkstemp(./gmb_pF6TJi) failed: 13 (Permission denied)` 错误。
* [Commit 92e5](https://github.com/manticoresoftware/manticoresearch/commit/92e5629f7150f62524b385fca670a01ba9590f5b) 自 4.0.2 版本以来崩溃日志仅包含偏移量。此提交修复了该问题。

## 版本 5.0.2
发布: 2022年5月30日

### Bug 修复
* ❗[Issue #791](https://github.com/manticoresoftware/manticoresearch/issues/791) - 错误的堆栈大小可能导致崩溃。

## 版本 5.0.0
发布: 2022年5月18日


### 主要新特性
* 🔬 支持 [Manticore Columnar Library 1.15.2](https://github.com/manticoresoftware/columnar/)，启用[二级索引](Server_settings/Searchd.md#secondary_indexes) 测试版。默认情况下，建立二级索引适用于普通和实时列式及行式索引（如果使用 [Manticore Columnar Library](https://github.com/manticoresoftware/columnar)），但要启用搜索功能，需要在配置文件或使用 [SET GLOBAL](Server_settings/Setting_variables_online.md) 设置 `secondary_indexes = 1`。该新功能支持所有操作系统，旧版 Debian Stretch 和 Ubuntu Xenial 除外。
* [只读模式](Security/Read_only.md)：现在可以指定仅处理读取查询而忽略写入的监听器。
* 新增 [/cli](Connecting_to_the_server/HTTP.md#/cli) 端点，使通过 HTTP 运行 SQL 查询更加简便。
* 通过 HTTP JSON 实现更快速的批量 INSERT/REPLACE/DELETE：以前可以通过 HTTP JSON 协议提供多个写入命令，但它们是逐个处理的，现在作为单个事务处理。
* [#720](https://github.com/manticoresoftware/manticoresearch/issues/720) JSON 协议支持[嵌套过滤器](Searching/Filters.md#Nested-bool-query)。此前 JSON 中无法表示类似 `a=1 and (b=2 or c=3)` 的逻辑：`must`（AND）、`should`（OR）和 `must_not`（NOT）仅在最高层级有效。现在它们可以嵌套使用。
* 支持 HTTP 协议的[分块传输编码](https://en.wikipedia.org/wiki/Chunked_transfer_encoding)。现在可在应用中使用分块传输发送大批量数据，节省资源消耗（不需要计算 `Content-Length`）。在服务器端，Manticore 现以流式方式处理传入 HTTP 数据，不像以往需等待整个批量传输完成，这带来：
  - 减少峰值内存使用，降低内存溢出风险
  - 缩短响应时间（测试显示处理 100MB 批量时间减少约 11%）
  - 让您绕过 [max_packet_size](Server_settings/Searchd.md#max_packet_size) 限制，传输超出最大允许值（128MB）的更大数据批次，例如一次传输 1GB。
* [#719](https://github.com/manticoresoftware/manticoresearch/issues/719) HTTP 接口支持 `100 Continue`：现在可使用 `curl`（以及多种编程语言使用的 curl 库）默认的 `Expect: 100-continue` 发送大批量数据。之前需要添加 `Expect: ` 头部，现在无需设置。

  <details>

  之前（注意响应时间）:

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
  * 完全上传，状态良好
  < HTTP/1.1 200 OK
  < Server: 4.2.1 63e5749@220405 dev
  < Content-Type: application/json; charset=UTF-8
  < Content-Length: 147
  <
  * 与主机 localhost 的连接 #0 保持
  {"items":[{"bulk":{"table":"user","_id":2811798919590182916,"created":4,"deleted":0,"updated":0,"result":"created","status":201}}],"errors":false}
  real	0m0.015s
  user	0m0.005s
  sys	0m0.004s
  ```

  </details>

* **⚠️ 重大变更**：[伪分片](Server_settings/Searchd.md#pseudo_sharding) 默认启用。如果您想禁用它，请确保在 Manticore 配置文件的 `searchd` 部分添加 `pseudo_sharding = 0`。
* 在实时/普通索引中不再强制必须包含至少一个全文字段。现在即使不涉及全文搜索，也可以使用 Manticore。
* [快速获取](Creating_a_table/Data_types.md#fast_fetch) 由 [Manticore 列存库](https://github.com/manticoresoftware/columnar) 支持的属性：像 `select * from <columnar table>` 的查询现在比以前快很多，尤其是当模式中字段较多时。
* **⚠️ 重大变更**：隐式的 [cutoff](Searching/Options.md#cutoff)。Manticore 现在不会花时间和资源处理结果集中不需要的数据。缺点是这会影响在 [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META) 中的 `total_found` 和 JSON 输出中的 [hits.total](Searching/Full_text_matching/Basic_usage.md#HTTP-JSON)。只有当你看到 `total_relation: eq` 时数值才准确，`total_relation: gte` 意味着匹配文档的实际数量大于获取的 `total_found`。要恢复之前的行为，可以使用搜索选项 `cutoff=0`，这会使 `total_relation` 始终为 `eq`。
* **⚠️ 重大变更**：所有全文字段现在默认[被存储](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#stored_fields)。如果需要让所有字段不被存储（即恢复之前的行为），你需要使用 `stored_fields = `（空值）。
* [#715](https://github.com/manticoresoftware/manticoresearch/issues/715) HTTP JSON 支持 [搜索选项](Searching/Options.md#General-syntax)。
### 次要更改
* **⚠️ 重大变更**：索引元文件格式变更。以前元文件（`.meta`，`.sph`）是二进制格式，现在变为 json 格式。新版本的 Manticore 会自动转换旧索引，但：
  - 你可能会收到类似 `WARNING: ... syntax error, unexpected TOK_IDENT` 的警告
  - 你将无法在旧版本 Manticore 上运行该索引，请确保有备份
* **⚠️ 重大变更**：使用 [HTTP 长连接](Connecting_to_the_server/HTTP.md#Keep-alive) 支持会话状态。当客户端支持时，HTTP 变为有状态。例如，使用新的 [/cli](Connecting_to_the_server/HTTP.md#/cli) 端点和默认在浏览器开启的 HTTP 长连接，可以在 `SELECT` 后调用 `SHOW META`，其行为与 mysql 相同。注意，此前支持的 `Connection: keep-alive` 头只会重用连接，从此版本开始，它还使得会话保持状态。
* 你现在可以在 [普通模式](Read_this_first.md#Real-time-mode-vs-plain-mode) 中使用 `columnar_attrs = *` 来将所有属性定义为列存储，这在属性列表较长时非常有用。
* 提升了复制 SST 的速度
* **⚠️ 重大变更**：复制协议已更改。如果你运行复制集群，升级到 Manticore 5 时需要：
  - 先干净地停止所有节点
  - 然后最后停止的节点要用 `--new-cluster` 参数启动（在 Linux 上运行工具 `manticore_new_cluster`）
  - 阅读 [重新启动集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster) 了解更多细节。
* 复制改进：
  - SST 更快
  - 噪声抗性，能够帮助处理复制节点之间的不稳定网络问题
  - 改进的日志记录
* 安全性提升：如果配置中完全未指定 `listen`，Manticore 现在默认监听 `127.0.0.1` 而非 `0.0.0.0`。尽管 Manticore Search 附带的默认配置中指定了 `listen`，且通常不会没有配置此项，但仍有可能出现。此前 Manticore 会监听 `0.0.0.0`，这不安全；现在默认监听 `127.0.0.1`，通常不会暴露到互联网。
* 基于列属性的聚合操作更快。
* `AVG()` 准确度提高：之前内部聚合使用 `float`，现在改用 `double`，显著提升准确性。
* 改进了对 JDBC MySQL 驱动的支持。
* 对 [jemalloc](https://github.com/jemalloc/jemalloc) 支持 `DEBUG malloc_stats`。
* [optimize_cutoff](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#optimize_cutoff) 现在作为每表设置可用，可在 CREATE 或 ALTER 表时配置。
* **⚠️ 破坏性变更**：[query_log_format](Server_settings/Searchd.md#query_log_format) 现在默认为 **`sphinxql`**。如果您习惯使用 `plain` 格式，需要在配置文件中添加 `query_log_format = plain`。
* 显著减少内存消耗：在长时间且密集的插入/替换/优化负载情况下，如果使用存储字段，Manticore 现在显著减少 RAM 消耗。
* [shutdown_timeout](Server_settings/Searchd.md#shutdown_timeout) 默认值从 3 秒提高到 60 秒。

* [Commit ffd0](https://github.com/manticoresoftware/manticoresearch/commit/ffd0499d329d2c383f14c8a44c4cc84338ab56e7) 支持 Java mysql connector >= 6.0.3：Java mysql 连接器 6.0.3 [版本](https://mvnrepository.com/artifact/mysql/mysql-connector-java/6.0.3)更改了连接 MySQL 的方式，破坏了与 Manticore 的兼容性。新行为现已支持。
* [Commit 1da6](https://github.com/manticoresoftware/manticoresearch/commit/1da6dbec) 禁用索引加载时（如 searchd 启动时）保存新的磁盘块。
* [Issue #746](https://github.com/manticoresoftware/manticoresearch/issues/746) 支持 glibc >= 2.34。
* [Issue #784](https://github.com/manticoresoftware/manticoresearch/issues/784) 将 'VIP' 连接与普通（非 VIP）连接分开计数。此前 VIP 连接计入 `max_connections` 限制，可能导致非 VIP 连接出现“连接数已满”错误。现在 VIP 连接不计入限制。当前 VIP 连接数可通过 `SHOW STATUS` 和 `status` 查看。
* [ID](Creating_a_table/Data_types.md#Document-ID) 现在可以显式指定。
* [Issue #687](https://github.com/manticoresoftware/manticoresearch/issues/687) 支持 mysql proto 的 zstd 压缩。
### ⚠️ 其他小的破坏性变更
* ⚠️ 稍微调整了 BM25F 公式以提升搜索相关性。仅影响使用函数 [BM25F()](Functions/Searching_and_ranking_functions.md#BM25F%28%29) 的搜索结果，不改变默认排名公式的行为。
* ⚠️ 更改 REST [/sql](Connecting_to_the_server/HTTP.md#mode=raw) 端点行为：`/sql?mode=raw` 现在要求转义，并且返回数组。

* ⚠️ 更改 `/bulk` INSERT/REPLACE/DELETE 请求的响应格式：
  - 之前每个子查询构成一个独立事务，产生单独响应
  - 现在整个批处理视为单一事务，返回单一响应
* ⚠️ 搜索选项 `low_priority` 和 `boolean_simplify` 现在必须带值（`0/1`）：之前可写为 `SELECT ... OPTION low_priority, boolean_simplify`，现在需写为 `SELECT ... OPTION low_priority=1, boolean_simplify=1`。
* ⚠️ 如果您使用旧版 [php](https://github.com/manticoresoftware/manticoresearch/blob/master/api/sphinxapi.php)、[python](https://github.com/manticoresoftware/manticoresearch/blob/master/api/sphinxapi.py) 或 [java](https://github.com/manticoresoftware/manticoresearch/tree/master/api/java) 客户端，请访问相应链接并获取更新版本。**旧版本与 Manticore 5 不完全兼容。**
* ⚠️ HTTP JSON 请求在 `query_log_format=sphinxql` 模式下采用不同格式记录。之前只记录全文部分，现在按原样记录。

### 新增软件包
* **⚠️ 破坏性变更**：由于新结构，升级到 Manticore 5 时建议先移除旧包再安装新包：
  - 基于 RPM：`yum remove manticore*`
  - Debian 和 Ubuntu：`apt remove manticore*`
* 新的 deb/rpm 包结构。此前版本包含：
  - `manticore-server`，内含 `searchd`（主搜索守护进程）及其所需全部内容
  - `manticore-tools`，包含 `indexer` 和 `indextool`
  - `manticore`，包含全部内容
  - `manticore-all` RPM 作为元包，指向 `manticore-server` 和 `manticore-tools`

  新结构为：
  - `manticore` - deb/rpm 元包，安装以上所有依赖
  - `manticore-server-core` - 仅包含 `searchd` 和其运行所需内容
  - `manticore-server` - systemd 文件及其他辅助脚本
  - `manticore-tools` - 包含 `indexer`、`indextool` 和其他工具
  - `manticore-common` - 默认配置文件、默认数据目录、默认停用词
  - `manticore-icudata`、`manticore-dev`、`manticore-converter` 变化不大
  - `.tgz` 包含全部软件包
* 支持 Ubuntu Jammy
* 支持通过 [YUM 仓库](Installation/RHEL_and_Centos.md#YUM-repository) 的 Amazon Linux 2

### Bug 修复
* [Issue #815](https://github.com/manticoresoftware/manticoresearch/issues/815) 使用 UDF 函数时偶发崩溃
* [Issue #287](https://github.com/manticoresoftware/manticoresearch/issues/287) RT 索引构建时内存溢出
* [Issue #604](https://github.com/manticoresoftware/manticoresearch/issues/604) 破坏性变更 3.6.0, 4.2.0 sphinxql-parser
* [Issue #667](https://github.com/manticoresoftware/manticoresearch/issues/667) 致命错误: 内存不足（无法分配 9007199254740992 字节）
* [Issue #676](https://github.com/manticoresoftware/manticoresearch/issues/676) 字符串未正确传递给 UDFs
* ❗[Issue #698](https://github.com/manticoresoftware/manticoresearch/issues/698) 尝试向 rt 索引添加文本列后，searchd 崩溃
* [Issue #705](https://github.com/manticoresoftware/manticoresearch/issues/705) 索引器无法找到所有列
* ❗[Issue #709](https://github.com/manticoresoftware/manticoresearch/issues/705) 按 json.boolean 分组结果错误
* [Issue #716](https://github.com/manticoresoftware/manticoresearch/issues/716) indextool 与索引相关命令（如 --dumpdict）失败
* ❗[Issue #724](https://github.com/manticoresoftware/manticoresearch/issues/724) 选择中字段消失
* [Issue #727](https://github.com/manticoresoftware/manticoresearch/issues/727) .NET HttpClient 使用 `application/x-ndjson` 时 Content-Type 不兼容
* [Issue #729](https://github.com/manticoresoftware/manticoresearch/issues/729) 字段长度计算
* ❗[Issue #730](https://github.com/manticoresoftware/manticoresearch/issues/730) 创建/插入到/删除柱状表导致内存泄漏
* [Issue #731](https://github.com/manticoresoftware/manticoresearch/issues/731) 在某些条件下结果中出现空列
* ❗[Issue #749](https://github.com/manticoresoftware/manticoresearch/issues/749) 守护进程启动时崩溃
* ❗[Issue #750](https://github.com/manticoresoftware/manticoresearch/issues/750) 守护进程启动时挂起
* ❗[Issue #751](https://github.com/manticoresoftware/manticoresearch/issues/751) SST 时崩溃
* [Issue #752](https://github.com/manticoresoftware/manticoresearch/issues/752) 当 engine='columnar' 时，json 属性被标记为柱状
* [Issue #753](https://github.com/manticoresoftware/manticoresearch/issues/753) 复制侦听地址为 0
* [Issue #754](https://github.com/manticoresoftware/manticoresearch/issues/754) columnar_attrs = * 在 csvpipe 中无效
* ❗[Issue #755](https://github.com/manticoresoftware/manticoresearch/issues/755) 在 rt 中选择柱状浮点字段崩溃
* ❗[Issue #756](https://github.com/manticoresoftware/manticoresearch/issues/756) Indextool 在检查时更改 rt 索引
* [Issue #757](https://github.com/manticoresoftware/manticoresearch/issues/757) 需要检查监听端口范围重叠
* [Issue #758](https://github.com/manticoresoftware/manticoresearch/issues/758) 在 RT 索引保存磁盘块失败时记录原始错误
* [Issue #759](https://github.com/manticoresoftware/manticoresearch/issues/759) RE2 配置只报告一个错误
* ❗[Issue #760](https://github.com/manticoresoftware/manticoresearch/issues/760) 提交 5463778558586d2508697fa82e71d657ac36510f 中的内存使用变化
* [Issue #761](https://github.com/manticoresoftware/manticoresearch/issues/761) 第三节点经历非正常重启后无法成为非主集群
* [Issue #762](https://github.com/manticoresoftware/manticoresearch/issues/762) 更新计数增加 2
* [Issue #763](https://github.com/manticoresoftware/manticoresearch/issues/763) 4.2.1 新版本损坏使用形态学的 4.2.0 创建的索引
* [Issue #764](https://github.com/manticoresoftware/manticoresearch/issues/764) json 键 /sql?mode=raw 中无转义
* ❗[Issue #765](https://github.com/manticoresoftware/manticoresearch/issues/765) 使用函数隐藏其他值
* ❗[Issue #766](https://github.com/manticoresoftware/manticoresearch/issues/766) FixupAttrForNetwork 一行代码触发内存泄漏
* ❗[Issue #767](https://github.com/manticoresoftware/manticoresearch/issues/767) 与文档存储缓存相关的 4.2.0 和 4.2.1 版本内存泄漏
* [Issue #768](https://github.com/manticoresoftware/manticoresearch/issues/768) 存储字段网络传输异常的乒乓行为
* [Issue #769](https://github.com/manticoresoftware/manticoresearch/issues/769) 若未在 'common' 部分指定，lemmatizer_base 重置为空
* [Issue #770](https://github.com/manticoresoftware/manticoresearch/issues/770) pseudo_sharding 使按 ID 查询变慢
* [Issue #771](https://github.com/manticoresoftware/manticoresearch/issues/771) 使用 jemalloc 时 DEBUG malloc_stats 输出为零
* [Issue #772](https://github.com/manticoresoftware/manticoresearch/issues/772) 删除/添加列导致值不可见
* [Issue #773](https://github.com/manticoresoftware/manticoresearch/issues/773) 无法向柱状表添加 bit(N) 列
* [Issue #774](https://github.com/manticoresoftware/manticoresearch/issues/774) manticore.json 中启动时“cluster”为空
* ❗[Commit 1da4](https://github.com/manticoresoftware/manticoresearch/commits/1da4ce891e4ff13727cfd3331d72771c64ee7949) SHOW STATUS 中未跟踪 HTTP 操作
* [Commit 3810](https://github.com/manticoresoftware/manticoresearch/commits/381000ab4af4c3c98ec8c730699ad7f39039cec8) 禁用低频单关键字查询的 pseudo_sharding
* [Commit 8003](https://github.com/manticoresoftware/manticoresearch/commits/800325cca283a87801b28929d82420fafc76a0ee) 修复存储属性与索引合并问题
* [Commit cddf](https://github.com/manticoresoftware/manticoresearch/commits/cddfeed6296f2041c6aae18414e5cd9cabf08281) 通用去重值提取器；为柱状字符串添加专用去重提取器
* [Commit fba4](https://github.com/manticoresoftware/manticoresearch/commits/fba4bb4f) 修复从文档存储提取空整数属性
* [Commit f300](https://github.com/manticoresoftware/manticoresearch/commit/f3009a9242fa16956adf871c4887e6e0303ba364) `ranker` 可在查询日志中重复指定

## 版本 4.2.0，2021年12月23日

### 主要新特性
* **伪分片支持实时索引和全文查询**。在之前的版本中我们添加了有限的伪分片支持。从这个版本开始，你只需启用 [searchd.pseudo_sharding](Server_settings/Searchd.md#pseudo_sharding) 就可以获得伪分片的所有好处和多核处理器的优势。最酷的是，你不需要对索引或查询做任何改动，只需启用它，如果你有空闲的CPU，系统会利用它来降低响应时间。它支持普通索引和实时索引，用于全文、过滤和分析查询。例如，启用伪分片后，在 [Hacker news curated comments dataset](https://zenodo.org/record/45901/) 放大100倍（普通索引中有1.16亿文档）的情况下，大多数查询的**平均响应时间降低约10倍**。

![4.2.0中伪分片开启与关闭对比](4.2.0_ps_on_vs_off.png)

* [**Debian Bullseye**](https://manticoresearch.com/install/) 现已支持。

<!-- example pq_transactions_4.2.0 -->
* PQ事务现已支持原子性和隔离性。此前PQ事务支持有限。这使得大量规则的REPLACE操作速度明显更快。性能细节：

<!-- intro -->
#### 4.0.2

<!-- request 4.0.2 -->

插入100万PQ规则需要**48秒**，而分批次（每批1万）REPLACE仅4万条规则需要**406秒**。

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

插入100万PQ规则需要**34秒**，而分批次（每批1万）REPLACE这些规则只需**23秒**。

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

### 小幅变更
*  [optimize_cutoff](Server_settings/Searchd.md#optimize_cutoff) 现已作为 `searchd` 部分的配置选项提供。当你想统一限制所有索引中RT块的数量时非常有用。
* [提交 0087](https://github.com/manticoresoftware/manticoresearch/commit/00874743823eaf43e2a09a088392fd7e0a617f47) 实现了多个具有相同字段集合或顺序的本地物理索引（实时/普通）上的准确 [count(distinct ...)](Searching/Grouping.md#COUNT%28DISTINCT-field%29) 和 [FACET ... distinct](Searching/Faceted_search.md#Faceting-without-duplicates)。
* [PR #598](https://github.com/manticoresoftware/manticoresearch/pull/598) 支持 `YEAR()` 和其他时间戳函数的 bigint 类型。

* [提交 8e85](https://github.com/manticoresoftware/manticoresearch/commit/8e85d4bce596cc88f9d9615ea243a27e4afde0f7) 自适应 [rt_mem_limit](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_mem_limit)。此前Manticore Search会精确收集到 `rt_mem_limit` 大小数据后保存为磁盘块，在保存时仍会收集多达10%（即双缓冲）额外数据以减少插入暂停。如果此限制也被耗尽，则在磁盘块完全保存到磁盘前新文档的添加会被阻塞。新的自适应限制基于我们现在拥有的 [auto-optimize](Server_settings/Searchd.md#auto_optimize) 功能，因此即使磁盘块不完全遵守 `rt_mem_limit` 而提前刷新也不严重。现在我们先收集 `rt_mem_limit` 的50%，并保存为磁盘块。保存时，我们查看统计数据（已保存多少，保存时到达多少新文档），并重新计算下次使用的初始比率。例如，如果保存了9000万文档，保存时又到达1000万文档，则比率为90%，下一次我们将在刷新下一个磁盘块前收集最多90%的 `rt_mem_limit`。该比率自动计算，范围在33.3%到95%。
* [问题 #628](https://github.com/manticoresoftware/manticoresearch/issues/628) PostgreSQL源的 [unpack_zlib](Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Database_connection.md#unpack_zlib)。感谢 [Dmitry Voronin](https://github.com/dimv36) 的贡献 ([PR链接](https://github.com/manticoresoftware/manticoresearch/pull/630))。
* [提交 6d54](https://github.com/manticoresoftware/manticoresearch/commit/6d54cf2b319b131970c29410cc21abcbdf8083b1) `indexer -v` 和 `--version`。此前你仍可看到indexer版本，但不支持 `-v`/`--version` 参数。
* [问题 #662](https://github.com/manticoresoftware/manticoresearch/issues/662) 当Manticore通过systemd启动时，默认无限制 mlock。
* [提交 63c8](https://github.com/manticoresoftware/manticoresearch/commit/63c8cd05cf399e705c2c14939411a92cb784735a) 协程读写锁从自旋锁改为操作队列。

* [提交 4113](https://github.com/manticoresoftware/manticoresearch/commit/41130ce30d1734bbf8e6b8b63c4e815caf9464f2) 用于调试RT段损坏的环境变量 `MANTICORE_TRACK_RT_ERRORS` 。
### 重大变更
* Binlog版本已升级，之前版本的binlog不会被重放，因此升级时务必干净停止Manticore Search：停止前`/var/lib/manticore/binlog/`目录下除了 `binlog.meta` 不应有其他binlog文件。
* [提交 3f65](https://github.com/manticoresoftware/manticoresearch/commit/3f659f36e87d99ee262205a8eae4663a255be630) `show threads option format=all`新增“chain”列。该列显示一些任务信息票据的堆栈，对性能分析非常有用，解析 `show threads` 输出时请注意新增列。
* 自3.5.0版本起，`searchd.workers` 已过时，目前已废弃，配置文件中仍有该项启动时将触发警告。Manticore Search仍会启动，但会有警告提示。
* 使用PHP和PDO访问Manticore时需要设置 `PDO::ATTR_EMULATE_PREPARES` 。
### Bug修复
* ❗[问题 #650](https://github.com/manticoresoftware/manticoresearch/issues/650) Manticore 4.0.2比3.6.3慢。4.0.2版本在批量插入方面快于以前版本，但单条文档插入显著变慢。此问题已在4.2.0中修复。
* ❗[提交 22f4](https://github.com/manticoresoftware/manticoresearch/commit/22f4141bed1411c83684ae6e0bae832071cce3df) 在密集的 REPLACE 负载下，RT 索引可能会损坏，或导致崩溃
* [提交 03be](https://github.com/manticoresoftware/manticoresearch/commit/03be91e4f720a3f20bee0278dd8f0d44abd3ef0e) 修正合并聚合器和组 N 排序器时的平均值；修正聚合合并
* [提交 2ea5](https://github.com/manticoresoftware/manticoresearch/commit/2ea575d34efe38661f14d858f2b552456f29fe42) `indextool --check` 可能崩溃
* [提交 7ec7](https://github.com/manticoresoftware/manticoresearch/commit/7ec76d4abf01b5fca6f10a0000fe64d5d8e5fa52) UPDATE 操作引起的内存耗尽问题
* [提交 658a](https://github.com/manticoresoftware/manticoresearch/commit/658a727e41f3c3fbcde9754149528f34ed76ba19) 守护进程在 INSERT 时可能挂起
* [提交 46e4](https://github.com/manticoresoftware/manticoresearch/commit/46e42b9b386e31e53247ed634e158b533e28063e) 守护进程在关闭时可能挂起
* [提交 f8d7](https://github.com/manticoresoftware/manticoresearch/commit/f8d7d5177cc83d250b493b4cfc619732b29cf072) 守护进程关闭时可能崩溃
* [提交 733a](https://github.com/manticoresoftware/manticoresearch/commit/733accf165142a1d1574689525ccca8907971158) 守护进程崩溃时可能挂起
* [提交 f7f8](https://github.com/manticoresoftware/manticoresearch/commit/f7f8bd8c0dbb23a59a88502dcba16b0918fb2330) 守护进程启动时尝试用无效节点列表重新加入集群可能崩溃
* [提交 1401](https://github.com/manticoresoftware/manticoresearch/commit/140155613821ecaddcd1a3dfc4f7369b0df6aef7) 分布式索引在 RT 模式下启动时若无法解析其某代理，可能被完全遗忘
* [问题 #683](https://github.com/manticoresoftware/manticoresearch/issues/683) 属性 bit(N) engine='columnar' 失败
* [问题 #682](https://github.com/manticoresoftware/manticoresearch/issues/682) 创建表失败，但留下目录
* [问题 #663](https://github.com/manticoresoftware/manticoresearch/issues/663) 配置失败，提示未知键名 'attr_update_reserve'
* [问题 #632](https://github.com/manticoresoftware/manticoresearch/issues/632) Manticore 批量查询崩溃
* [问题 #679](https://github.com/manticoresoftware/manticoresearch/issues/679) 批量查询在 v4.0.3 版本再次导致崩溃
* [提交 f7f8](https://github.com/manticoresoftware/manticoresearch/commit/f7f8bd8c0dbb23a59a88502dcba16b0918fb2330) 修正守护进程启动时尝试用无效节点列表重新加入集群崩溃问题
* [问题 #643](https://github.com/manticoresoftware/manticoresearch/issues/643) Manticore 4.0.2 在批量插入后不接受连接
* [问题 #635](https://github.com/manticoresoftware/manticoresearch/issues/635) 带 ORDER BY JSON.field 或字符串属性的 FACET 查询可能崩溃
* [问题 #634](https://github.com/manticoresoftware/manticoresearch/issues/634) 查询含 packedfactors 时崩溃（SIGSEGV）
* [提交 4165](https://github.com/manticoresoftware/manticoresearch/commits/41657f15f8fcf1e93cfad7f835bcf705d6e8349e) create table 未支持 morphology_skip_fields

## 版本 4.0.2，2021年9月21日

### 主要新功能
- **全面支持 [Manticore Columnar Library](https://github.com/manticoresoftware/columnar)**。此前 Manticore Columnar Library 仅支持普通索引。现在支持：
  - 实时索引的 `INSERT`、`REPLACE`、`DELETE`、`OPTIMIZE`
  - 复制
  - `ALTER`
  - `indextool --check`
- **自动索引压缩** ([问题 #478](https://github.com/manticoresoftware/manticoresearch/issues/478))。现在您无需手动调用 OPTIMIZE 或通过定时任务或其他自动化方式进行。Manticore 现在默认自动执行。您可以通过全局变量 [optimize_cutoff](Server_settings/Setting_variables_online.md) 设置默认压缩阈值。
- **块快照和锁系统的重构**。这些变化从表面上可能不明显，但大大改善了实时索引中许多操作的表现。简而言之，此前大多数 Manticore 数据操作依赖于大量锁定，现在改用磁盘块快照机制。
- **实时索引大批量插入性能显著提升**。举例说明，在 [Hetzner 服务器 AX101](https://www.hetzner.com/dedicated-rootserver/ax101)（配备 SSD，128 GB 内存和 AMD Ryzen™ 9 5950X（16*2 核））上，**3.6.0 版本中向架构为 `name text, email string, description text, age int, active bit(1)` 的表插入文档时，默认 `rt_mem_limit`，批大小 25000，16 并发插入工作者，总插入了 1600 万文档，速度可达 236K 文档/秒**。而在 4.0.2 版本下，相同的并发/批大小/数量下，插入速度提升到 **357K 文档/秒**。

  <details>

  - 读操作（如 SELECT、复制）使用快照执行
  - 仅改变内部索引结构且不修改模式或文档的操作（如合并 RAM 段、保存磁盘块、合并磁盘块）使用只读快照执行，并最终替换现有块
  - UPDATE 和 DELETE 针对现有块执行；若进行合并，写入先收集，后应用到新块
  - UPDATE 会依次为每个块获取排他锁。合并在收集块属性阶段获取共享锁。因此同时只有一个（合并或更新）操作可以访问该块的属性。
  - 合并进入需要属性的阶段时会设置特殊标志。UPDATE 完成后检查该标志，若设置了，则将整个更新存入特殊集合。合并完成后将更新应用到新生成的磁盘块。
  - ALTER 通过排他锁执行
  - 复制作为常规读操作运行，且在 SST 前保存属性，禁止 SST 期间的更新

  </details>

- **[ALTER](Updating_table_schema_and_settings.md) 可以添加/删除全文字段**（在 RT 模式下）。以前它只能添加/删除属性。
- 🔬 **实验性功能：用于全扫描查询的伪分片** - 允许并行化任何非全文搜索查询。您现在只需启用新选项 [searchd.pseudo_sharding](Server_settings/Searchd.md#pseudo_sharding)，即可期望非全文搜索查询的响应时间降低到 `CPU 核心数` 级别。注意，这可能会占用所有现有的 CPU 核心，因此如果您不仅关心延迟，还关心吞吐量，请谨慎使用。

### 小改动
<!-- example -->
- 通过 [APT 仓库](Installation/Debian_and_Ubuntu.md#APT-repository) 支持 Linux Mint 和 Ubuntu Hirsute Hippo
- 在某些情况下，通过 HTTP 根据 id 更新大型索引更快（取决于 id 的分布）
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
- [systemd 的自定义启动标志](Starting_the_server/Linux.md#Custom-startup-flags-using-systemd)。现在如果需要以某些特定启动标志运行 Manticore，您无需手动启动 searchd
- 新函数 [LEVENSHTEIN()](Functions/String_functions.md#LEVENSHTEIN%28%29)，用于计算 Levenshtein 距离
- 新增 [searchd 启动标志](Starting_the_server/Manually.md#searchd-command-line-options) `--replay-flags=ignore-trx-errors` 和 `--replay-flags=ignore-all-errors`，使得即使 binlog 损坏仍能启动 searchd
- [Issue #621](https://github.com/manticoresoftware/manticoresearch/issues/621) - 公开 RE2 的错误信息
- 针对由本地普通索引组成的分布式索引，[COUNT(DISTINCT)](Searching/Grouping.md#COUNT%28DISTINCT-field%29) 更加准确
- [FACET DISTINCT](Searching/Faceted_search.md#Faceting-without-duplicates) 用于在进行分面搜索时去重
- [精确形式修饰符](Searching/Full_text_matching/Operators.md#Exact-form-modifier) 现在不再依赖 [形态学](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology)，并且支持启用了 [infix/prefix](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) 搜索的索引

### 重大变更
- 新版本可以读取旧索引，但旧版本无法读取 Manticore 4 的索引
- 移除默认根据 id 进行隐式排序。如有需求，请显式排序
- `charset_table` 默认值由 `0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451` 改为 `non_cjk`
- `OPTIMIZE` 会自动执行。如不需要，请确保在配置文件的 `searchd` 部分设置 `auto_optimize=0`
- [Issue #616](https://github.com/manticoresoftware/manticoresearch/issues/616) `ondisk_attrs_default` 已弃用，现已移除
- 针对贡献者：Linux 构建现在使用 Clang 编译器，根据我们的测试它能构建出更快的 Manticore Search 和 Manticore Columnar 库
- 如果搜索查询中未指定 [max_matches](Searching/Options.md#max_matches)，将隐式更新为性能所需的最低值，以提升新列存储的性能。这可能影响 [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META) 中的 `total` 指标，但不会影响表示实际找到文档数的 `total_found`

### 从 Manticore 3 迁移
- 确保干净地停止 Manticore 3：
  - `/var/lib/manticore/binlog/` 中不应有 binlog 文件（目录中仅应有 `binlog.meta`）

  - 否则 Manticore 4 无法回复 binlog，相关索引将无法运行
- 新版本可以读取旧索引，但旧版本不能读取 Manticore 4 的索引，因此如果想轻松回滚，请务必做好备份
- 如果运行复制集群，请确保：
  - 先干净停止所有节点
  - 然后通过 `--new-cluster` 启动最后停止的节点（在 Linux 上运行 `manticore_new_cluster` 工具）
  - 更多细节请参阅 [重启集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster)
### Bug 修复
- 修复了许多复制问题：
  - [Commit 696f](https://github.com/manticoresoftware/manticoresearch/commit/696f8649535f2b5285b878c9581c0d2b1139ae09) - 修复带有活动索引的 joiner 节点 SST 过程中的崩溃；新增写入文件块时 SHA1 验证以加速索引加载；加入节点加载索引时新增已更改索引文件旋转；当活动索引被捐赠节点的新索引替换时，加入节点会移除相关索引文件；在捐赠节点新增复制日志点以发送文件和块
  - [Commit b296](https://github.com/manticoresoftware/manticoresearch/commit/b296c55af68314a8fc66f8b104e7301d673c3b68) - 修复 JOIN CLUSTER 时地址错误导致崩溃
  - [Commit 418b](https://github.com/manticoresoftware/manticoresearch/commit/418bf880a5e05ffbb68f311a1c23006a9df3220e) - 大型索引初始复制期间，加入节点可能因 `ERROR 1064 (42000): invalid GTID, (null)` 失败，捐赠节点在另一个节点加入时可能无响应
  - [Commit 6fd3](https://github.com/manticoresoftware/manticoresearch/commit/6fd350d28e4905211cf8a7298cbf7cdda101c83d) - 修正大型索引计算哈希错误导致复制失败的问题
  - [Issue #615](https://github.com/manticoresoftware/manticoresearch/issues/615) - 修复集群重启后复制失败的问题
- [Issue #574](https://github.com/manticoresoftware/manticoresearch/issues/574) - `indextool --help` 不显示参数 `--rotate`
- [Issue #578](https://github.com/manticoresoftware/manticoresearch/issues/578) - searchd 在空闲约一天后 CPU 占用率过高
- [Issue #587](https://github.com/manticoresoftware/manticoresearch/issues/587) - 立即刷新 .meta
- [Issue #617](https://github.com/manticoresoftware/manticoresearch/issues/617) - manticore.json 被清空
- [Issue #618](https://github.com/manticoresoftware/manticoresearch/issues/618) - searchd --stopwait 在 root 用户下失败。它还修复了 systemctl 的行为（之前 ExecStop 显示失败且没有足够时间让 searchd 正确停止）
- [Issue #619](https://github.com/manticoresoftware/manticoresearch/issues/619) - INSERT/REPLACE/DELETE 与 SHOW STATUS。`command_insert`、`command_replace` 和其他命令显示了错误的指标
- [Issue #620](https://github.com/manticoresoftware/manticoresearch/issues/620) - 纯索引的 `charset_table` 默认值错误
- [Commit 8f75](https://github.com/manticoresoftware/manticoresearch/commit/8f7536887d4b58a5bab5647fc067b2d0482c2038) - 新的磁盘块不会被 mlock
- [Issue #607](https://github.com/manticoresoftware/manticoresearch/issues/607) - 当无法根据名称解析节点时，Manticore 集群节点崩溃
- [Issue #623](https://github.com/manticoresoftware/manticoresearch/issues/623) - 更新后的索引复制可能导致未定义状态
- [Commit ca03](https://github.com/manticoresoftware/manticoresearch/commit/ca03d2280c5197e9b311f0f03f551e3b702a8130) - 索引器在索引带有 json 属性的纯索引源时可能挂起
- [Commit 53c7](https://github.com/manticoresoftware/manticoresearch/commit/53c753054558ff3f82c7af365273d32537c5338b) - 修复 PQ 索引上的不等于表达式过滤器
- [Commit ccf9](https://github.com/manticoresoftware/manticoresearch/commit/ccf94e022ba6763d6286ab5bac767b6f6a846846) - 修复了列表查询中超过 1000 条匹配的选择窗口。之前 `SELECT * FROM pq ORDER BY id desc LIMIT 1000 , 100 OPTION max_matches=1100` 不工作
- [Commit a048](https://github.com/manticoresoftware/manticoresearch/commit/a0483fe9bef3c45dffe96f5df4f4382aee29c072) - 向 Manticore 发送 HTTPS 请求可能导致警告 "max packet size(8388608) exceeded"
- [Issue #648](https://github.com/manticoresoftware/manticoresearch/issues/648) - Manticore 3 在字符串属性更新几次后可能挂起


## Version 3.6.0, 2021年5月3日
**Manticore 4 发布前的维护版本**

### 主要新功能
- 支持用于纯索引的 [Manticore Columnar Library](https://github.com/manticoresoftware/columnar/)。纯索引新增设置 [columnar_attrs](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#columnar_attrs)
- 支持 [乌克兰语词形还原器](https://github.com/manticoresoftware/lemmatizer-uk)
- 完全改进的直方图。在建立索引时，Manticore 会为其中的每个字段构建直方图，以加快过滤速度。在 3.6.0 中，算法全面修订，对于大量数据和频繁过滤可以获得更高性能。

### 次要变更
- 工具 `manticore_new_cluster [--force]`，用于通过 systemd 重启复制集群
- `indexer --merge` 命令新增参数 [--drop-src](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Merging_tables.md#Merging-tables)
- [新模式](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) `blend_mode='trim_all'`
- 增加了使用反引号转义 JSON 路径的支持 [支持转义JSON路径](Searching/Full_text_matching/Escaping.md#Escaping-json-node-names-in-SQL)
- [indextool --check](Miscellaneous_tools.md#indextool) 可以在 RT 模式下工作
- SELECT/UPDATE 支持 [FORCE/IGNORE INDEX(id)](Data_creation_and_modification/Updating_documents/UPDATE.md#Updates-via-SQL)
- 合并后的磁盘块 chunk id 现在唯一
- [indextool --check-disk-chunk CHUNK_NAME](Miscellaneous_tools.md#indextool)

### 优化
- [更快的 JSON 解析](https://github.com/manticoresoftware/manticoresearch/commit/0c25fc1788e3f77f60cb7fb8f1623d8466bba606)，我们的测试显示对于 `WHERE json.a = 1` 这类查询延迟降低了 3-4%
- 未记录命令 `DEBUG SPLIT` 作为自动分片/负载均衡的先决条件

### Bug 修复
- [Issue #584](https://github.com/manticoresoftware/manticoresearch/issues/524) - FACET 结果不准确且不稳定
- [Issue #506](https://github.com/manticoresoftware/manticoresearch/issues/506) - 使用 MATCH 时的异常行为，受影响用户需重建索引，此问题发生在索引构建阶段
- [Issue #387](https://github.com/manticoresoftware/manticoresearch/issues/387) - 在运行带有 SNIPPET() 函数的查询时间歇性核心转储
- 用于处理复杂查询的堆栈优化：
  - [Issue #469](https://github.com/manticoresoftware/manticoresearch/issues/469) - SELECT 导致崩溃转储
  - [e8420cc7](https://github.com/manticoresoftware/manticoresearch/tree/e8420cc789149c84731be0874be916e33b263bd6) - 过滤树的堆栈大小检测
- [Issue #461](https://github.com/manticoresoftware/manticoresearch/issues/461) - 使用 IN 条件的更新无法正确生效
- [Issue #464](https://github.com/manticoresoftware/manticoresearch/issues/464) - CALL PQ 后立即执行 SHOW STATUS 返回 - [Issue #481](https://github.com/manticoresoftware/manticoresearch/issues/481) - 修复静态二进制构建
- [Issue #502](https://github.com/manticoresoftware/manticoresearch/issues/502) - 多查询中的错误
- [Issue #514](https://github.com/manticoresoftware/manticoresearch/issues/514) - 使用 'create table' 时无法使用非标准列名
- [Commit d1db](https://github.com/manticoresoftware/manticoresearch/commit/d1dbe771288328c963ca8512f564c6b7cf4f466c) - 重播带有字符串属性更新的 binlog 时守护进程崩溃；将 binlog 版本设为 10
- [Commit 775d](https://github.com/manticoresoftware/manticoresearch/commit/775d0555562f121911d3c30085947e743fadac2d) - 修复表达式堆栈帧检测运行时错误（测试 207）
- [Commit 4795](https://github.com/manticoresoftware/manticoresearch/commit/4795dc49194c7745761bb5006ecb266e8a9224fa) - 空存储查询时泄漏的 percolate 索引过滤器和标签（测试 369）
- [Commit c3f0](https://github.com/manticoresoftware/manticoresearch/commit/c3f0bf4dca0e4eae6ea899dd566878df57eba085)  - 解决网络延迟长且错误率高的情况下（不同数据中心复制）复制SST流中断问题；更新复制命令版本至1.03
- [Commit ba2d](https://github.com/manticoresoftware/manticoresearch/commit/ba2d6619b8107f3fb177741bbdff3709bac248cd)  - 加入集群后写操作时joiner锁集群（测试385）
- [Commit de4d](https://github.com/manticoresoftware/manticoresearch/commit/de4dcb9ff6c316bc2a9ef6c62eb64769f04ee103)  - 使用exact修饰符的通配符匹配（测试321）
- [Commit 6524](https://github.com/manticoresoftware/manticoresearch/commit/6524fc6af032a0932bd42c37874a619e1fb7f075)  - docid检查点与文档存储的关系
- [Commit f4ab](https://github.com/manticoresoftware/manticoresearch/commit/f4ab83c291e65a5873d271c67b5491f629544f1d)  - 解析无效XML时索引器行为不一致
- [Commit 7b72](https://github.com/manticoresoftware/manticoresearch/commit/7b727e22fe01b1a180d4a8d3a3fea92c18eac707)  - 存储的包含NOTNEAR的渗透查询无限运行（测试349）
- [Commit 812d](https://github.com/manticoresoftware/manticoresearch/commit/812dab745f0c972ecde5d5b3d4a313cd6e1e143e)  - 以通配符开头的短语权重错误
- [Commit 1771](https://github.com/manticoresoftware/manticoresearch/commit/1771afc669c75670b3511bf02ab9bfc8982b94d2)  - 含通配符的渗透查询在匹配时生成无payload的词项导致命中交织及匹配中断（测试417）
- [Commit aa0d](https://github.com/manticoresoftware/manticoresearch/commit/aa0d8c2bcb41428d41dbfb66e42a872a79a4ee4b)  - 修正并行查询情况下'total'的计算
- [Commit 18d8](https://github.com/manticoresoftware/manticoresearch/commit/18d81b3cfbb930b7361dae645fe13283d4fbf6ef)  - Windows平台守护进程多并发会话下崩溃
- [Commit 8443](https://github.com/manticoresoftware/manticoresearch/commit/84432f23d6fbf7a7d0b67c8c9412eb5933b03596)  - 某些索引设置无法复制
- [Commit 9341](https://github.com/manticoresoftware/manticoresearch/commit/93411fe6af4246ca4acce758944d513a1660b92b)  - 在高速新增事件时，netloop偶尔因原子'kick'事件在多个事件中只处理一次而丢失实际动作
查询状态，不是服务器状态
- [Commit d805](https://github.com/manticoresoftware/manticoresearch/commit/d805fc125244ef08d14fb3d1826e2c356862d539)  - 新刷新磁盘块可能在提交时丢失
- [Commit 63cb](https://github.com/manticoresoftware/manticoresearch/commit/63cbf008d50979cdf7062664fb02a672a3bb1131)  - profiler中的'net_read'不精确
- [Commit f537](https://github.com/manticoresoftware/manticoresearch/commit/f5379bb27234b25919966c50ccb6f469ead646e2)  - 阿拉伯语（从右向左文本）的渗透问题
- [Commit 49ee](https://github.com/manticoresoftware/manticoresearch/commit/49eeb4202d4ce4875c07c35087d7108c1e177328)  - 重复列名时id未正确选取
- [Commit refa](https://github.com/manticoresoftware/manticoresearch/commit/4b7df262bbfec9b9e95a44e72ad326ac8a786c49) 的网络事件修复稀有情况下崩溃
- [e8420cc7](https://github.com/manticoresoftware/manticoresearch/tree/e8420cc789149c84731be0874be916e33b263bd6) 修复了 `indextool --dumpheader`
- [Commit ff71](https://github.com/manticoresoftware/manticoresearch/commit/ff7163532ed39042c0d3c3c2b902ff41f94d7f0c) - TRUNCATE WITH RECONFIGURE 在带有存储字段时工作异常

### 破坏性更改：
- 新的binlog格式：升级前需要干净停止Manticore
- 索引格式略有变化：新版本可以正常读取现有索引，但如果从3.6.0降级至老版本，新索引将无法读取
- 复制格式变化：不要从老版本复制到3.6.0或反之，必须同时将所有节点切换到新版本
- `reverse_scan`已弃用。确保自3.6.0以来您的查询中不使用该选项，否则查询将失败
- 本版本起不再提供RHEL6、Debian Jessie及Ubuntu Trusty的构建版本。如您亟需支持，请[联系我们](https://manticoresearch.com/contact-us/)

### 弃用
- 不再默认按id隐式排序。如依赖此功能，请更新查询
- 搜索选项`reverse_scan`已弃用

## 版本 3.5.4，2020年12月10日

### 新功能
- 新的Python、Javascript和Java客户端现已正式发布，手册中有详细文档
- 实时索引的磁盘块自动丢弃。此优化允许在对实时索引进行[OPTIMIZE](Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE)时自动丢弃不再需要的磁盘块（所有文档均被删除）。过去仍需合并，现在可即时丢弃磁盘块。[cutoff](Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks)选项被忽略，即使没有实际合并，过时的磁盘块也会被移除。此功能在保留索引中删除旧文档时非常有用，压缩此类索引更快。
- 作为SELECT的选项支持独立的NOT（[standalone NOT](Searching/Options.md#not_terms_only_allowed)）

### 小改动
- [Issue #453](https://github.com/manticoresoftware/manticoresearch/issues/453) 新增选项 [indexer.ignore_non_plain=1](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#ignore_non_plain)，适用于运行 `indexer --all` 且配置文件中包含非普通索引的情况。若无此选项，会产生警告并返回相应退出码。
- [SHOW PLAN ... OPTION format=dot](Node_info_and_management/Profiling/Query_plan.md#Dot-format-for-SHOW-PLAN) 和 [EXPLAIN QUERY ... OPTION format=dot](Searching/Full_text_matching/Profiling.md#Profiling-without-running-a-query) 支持全文查询计划执行的可视化。对理解复杂查询非常有用。

### 弃用
- `indexer --verbose` 已被弃用，因为它从未为索引器输出添加任何内容
- 转储 watchdog 的回溯信号现在应使用 `USR2`，而不是 `USR1`

###  修复的错误
- [Issue #423](https://github.com/manticoresoftware/manticoresearch/issues/423) 西里尔字符期间调用片段保留模式不高亮
- [Issue #435](https://github.com/manticoresoftware/manticoresearch/issues/435) RTINDEX - GROUP N BY 表达式 select 导致致命崩溃
- [Commit 2b3b](https://github.com/manticoresoftware/manticoresearch/commit/2b3b62bd0ce2b54544733d524a169e81f71b5388) 当处于集群中时，searchd 状态显示段错误
- [Commit 9dd2](https://github.com/manticoresoftware/manticoresearch/commit/9dd25c193ed2a53750a0e0e68566fd2b833fcb0c) 'SHOW INDEX index.N SETTINGS' 不能处理大于9的块
- [Issue #389](https://github.com/manticoresoftware/manticoresearch/issues/389) 导致 Manticore 崩溃的错误
- [Commit fba1](https://github.com/manticoresoftware/manticoresearch/commit/fba166178ff35c73a0c70d131cf3aca4a635e961) 转换器创建了损坏的索引
- [Commit eecd](https://github.com/manticoresoftware/manticoresearch/commit/eecd61d861c742471175be19a5e5455034b5db5f) stopword_step=0 与 CALL SNIPPETS()
- [Commit ea68](https://github.com/manticoresoftware/manticoresearch/commit/ea6850e449601199190f38a387c04b8a7b7e90f6) 在本地索引上低 max_matches 时 count distinct 返回0
- [Commit 362f](https://github.com/manticoresoftware/manticoresearch/commit/362f27db3421f09a41c6cce04b5baace00fa4c4d) 使用聚合时，命中中不返回存储的文本


## 版本 3.5.2，2020年10月1日

### 新功能

* OPTIMIZE 将磁盘块减少到一定数量（默认是 `2* 核心数`），而不是合并为单块。最优块数可通过 [cutoff](Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks) 选项控制。
* NOT 操作符现在可以单独使用。默认禁用，因单独 NOT 查询可能导致缓慢。可通过设置新的 searchd 指令 [not_terms_only_allowed](Server_settings/Searchd.md#not_terms_only_allowed) 为 `0` 启用。
* 新设置 [max_threads_per_query](Server_settings/Searchd.md#max_threads_per_query) 控制查询可使用的最大线程数。若未设置，查询可用线程数上限为 [threads](Server_settings/Searchd.md#threads) 的值。
每个 `SELECT` 查询可用线程数可通过 [OPTION threads=N](Searching/Options.md#threads) 覆盖全局 `max_threads_per_query`。
* Percolate 索引现在可通过 [IMPORT TABLE](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Importing_table.md) 导入。
* HTTP API `/search` 通过新的查询结点 `aggs` 获得对[分面](Searching/Faceted_search.md#HTTP-JSON)/[分组](Searching/Grouping.md)的基本支持。

### 小改动

* 若未声明复制监听指令，引擎会尝试使用定义的 'sphinx' 端口之后最多200个端口。
* `listen=...:sphinx` 必须明确设置用于 SphinxSE 连接或 SphinxAPI 客户端。
* [SHOW INDEX STATUS](Node_info_and_management/Table_settings_and_status/SHOW_TABLE_STATUS.md) 新增指标输出：`killed_documents`、`killed_rate`、`disk_mapped_doclists`、`disk_mapped_cached_doclists`、`disk_mapped_hitlists` 和 `disk_mapped_cached_hitlists`。
* SQL 命令 `status` 现输出 `Queue\Threads` 和 `Tasks\Threads`。

### 弃用：

* `dist_threads` 现已完全弃用，若仍使用将由 searchd 记录警告。

### Docker

官方 Docker 镜像基于 Ubuntu 20.04 LTS

### 打包

除了通常的 `manticore` 包，还可按组件安装 Manticore Search：

- `manticore-server-core` – 提供 `searchd`、手册页、日志目录、API 和 galera 模块。还会安装依赖 `manticore-common`。
- `manticore-server` – 为核心提供自动化脚本（init.d, systemd）和 `manticore_new_cluster` 包装器。还依赖安装 `manticore-server-core`。
- `manticore-common` – 提供配置、停用词、通用文档和骨架文件夹（datadir，modules 等）。
- `manticore-tools` – 提供辅助工具（`indexer`、`indextool` 等）、其手册页和示例。也依赖 `manticore-common`。
- `manticore-icudata`（RPM）或 `manticore-icudata-65l`（DEB）– 提供用于 ICU 形态学的 ICU 数据文件。
- `manticore-devel`（RPM）或 `manticore-dev`（DEB）– 提供 UDF 的开发头文件。

### Bug修复

1. [Commit 2a47](https://github.com/manticoresoftware/manticoresearch/commit/2a474dc1a26e8b0f8aaaae95669caf2f1d4b7746) RT 索引不同块时 grouper 守护进程崩溃
2. [Commit 57a1](https://github.com/manticoresoftware/manticoresearch/commit/57a19e5ad5663ef0ca7436595218fb1221d28c8e) 为空的远程文档的快速路径
3. [Commit 07dd](https://github.com/manticoresoftware/manticoresearch/commit/07dd3f313c63fb82c22092f9907ef24e3475250e) 表达式堆栈帧检测运行时
4. [Commit 08ae](https://github.com/manticoresoftware/manticoresearch/commit/08ae357cf1012bc8e2da54c20b205b592efda3d4) percolate 索引匹配超过32字段
5. [Commit 16b9](https://github.com/manticoresoftware/manticoresearch/commit/16b9390fd4cdb07a77ac4497adb935573a1710e5) 复制监听端口范围
6. [Commit 5fa6](https://github.com/manticoresoftware/manticoresearch/commit/5fa671affeacb6441a59c8a88479bfd423df7c81) 在 pq 上显示建表语句
7. [Commit 54d1](https://github.com/manticoresoftware/manticoresearch/commit/54d133b6449105a9fb0168db3f1fbb05fb5aa1f6) HTTPS 端口行为
8. [提交 fdbb](https://github.com/manticoresoftware/manticoresearch/commit/fdbbe5245cc296cc5c1ae3ae2fb9cb08fb66a248) 替换时混合 docstore 行
9. [提交 afb5](https://github.com/manticoresoftware/manticoresearch/commit/afb53f648ef4e64fa1776c58e66f6e716ac730ad) 将 TFO 不可用消息级别切换为 'info'
10. [提交 59d9](https://github.com/manticoresoftware/manticoresearch/commit/59d94cefc6e88af360b8046726a16b7eaa9f0b71) strcmp 非法使用导致崩溃
11. [提交 04af](https://github.com/manticoresoftware/manticoresearch/commit/04af0349c1e784c2230d204592d6272b440bb375) 向包含系统（停用词）文件的集群添加索引
12. [提交 5014](https://github.com/manticoresoftware/manticoresearch/commit/50148b4eae9f3a924082005cf5d2f9c8339af7f5) 合并含大型词典的索引；RT 优化大型磁盘块
13. [提交 a2ad](https://github.com/manticoresoftware/manticoresearch/commit/a2adf1582843f63d4dcc1dab8f7598e0917094a2) Indextool 可导出当前版本的元数据
14. [提交 69f6](https://github.com/manticoresoftware/manticoresearch/commit/69f6d5f77c64b97e3f395aea8c555b11c77fb732) GROUP N 中的分组顺序问题
15. [提交 24d5](https://github.com/manticoresoftware/manticoresearch/commit/24d5d80f508576794b751d70f9f9b524564e3c14) 握手后对 SphinxSE 进行显式刷新
16. [提交 31c4](https://github.com/manticoresoftware/manticoresearch/commit/31c4d78ac03ee85d9a02b86075448a04ac80f9e2) 仅在必要时避免复制超大描述
17. [提交 2959](https://github.com/manticoresoftware/manticoresearch/commit/2959e2caa8610f98fcdf2e5a82160f58687f4a27) show threads 中出现负时间
18. [提交 f0b3](https://github.com/manticoresoftware/manticoresearch/commit/f0b357102c70f3ac1d7da260c60b67322940e647) 令牌过滤插件对零位置增量的处理
19. [提交 a49e](https://github.com/manticoresoftware/manticoresearch/commit/a49e5bc13ea763e6f01dd5993e891858a84d1846) 多重命中时将 'FAIL' 改为 'WARNING'

## 版本 3.5.0，2020年7月22日

### 主要新功能：
* 本次发布耗时较长，因为我们致力于将多任务模式从线程切换到**协程**。这使配置更简单，查询并行化更直观：Manticore 仅使用指定数量的线程（参见新设置 [threads](Server_settings/Searchd.md#threads)），新模式确保以最优方式完成任务。
* [高亮](Searching/Highlighting.md#Highlighting-options)方面的变更：
  - 任何支持多个字段的高亮（`highlight({},'field1, field2'`）或 JSON 查询中的 `highlight`）现在默认对每个字段应用限制。
  - 任何支持纯文本的高亮（`highlight({}, string_attr)` 或 `snippet()`）现在对整个文档应用限制。
  - [每字段限制](Searching/Highlighting.md#limits_per_field)可通过 `limits_per_field=0` 选项切换为全局限制（默认为 `1`）。
  - 通过 HTTP JSON 进行的高亮中，[allow_empty](Searching/Highlighting.md#allow_empty) 默认值改为 `0`。

* 同一端口[现在可用于](Server_settings/Searchd.md#listen) http、https 和二进制 API（以接受来自远程 Manticore 实例的连接）。通过 mysql 协议的连接仍需使用 `listen = *:mysql`。Manticore 现在能自动检测尝试连接的客户端类型，MySQL 除外（因协议限制）。

* 在 RT 模式中，一个字段现在可以同时是[文本和字符串属性](Creating_a_table/Data_types.md#String) — [GitHub issue #331](https://github.com/manticoresoftware/manticoresearch/issues/331)。

  在[普通模式](Read_this_first.md#Real-time-mode-vs-plain-mode)中称为 `sql_field_string`，现已在[RT 模式](Read_this_first.md#Real-time-mode-vs-plain-mode)的实时索引中支持。示例用法如下：

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
* 现在您可以[高亮字符串属性](Searching/Highlighting.md#Highlighting-via-SQL)。
* SQL 接口支持 SSL 和压缩。
* 支持 mysql 客户端 [`status`](Node_info_and_management/Node_status.md#STATUS) 命令。
* [复制](Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Setting-up-replication)现在可以复制外部文件（停用词、例外等）。
* 通过 HTTP JSON 接口提供过滤器操作符 [`in`](Searching/Filters.md#Set-filters)。
* HTTP JSON 支持 [`expressions`](Searching/Expressions.md#expressions)。
* 现在可以在 RT 模式下[动态修改 `rt_mem_limit`](https://github.com/manticoresoftware/manticoresearch/issues/344)，即可以执行 `ALTER ... rt_mem_limit=<new value>`。
* 你现在可以使用[独立的中日韩字符集表](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology)：`chinese`、`japanese` 和 `korean`。
* [thread_stack](Server_settings/Searchd.md#thread_stack) 现在限制最大线程栈而非初始栈。
* 改进了 `SHOW THREADS` 的输出。
* 在 `SHOW THREADS` 中显示长时间运行的 `CALL PQ` 执行进度。
* 运行时可通过 [SET](Server_settings/Setting_variables_online.md#SET) 修改 cpustat、iostat、coredump。
* 实现了 `SET [GLOBAL] wait_timeout=NUM`。

### 破坏性变更：
* **索引格式已更改。** 3.5.0 版本构建的索引无法被低于 3.5.0 版本的 Manticore 加载，但 Manticore 3.5.0 可识别旧格式。
* [`INSERT INTO PQ VALUES()`](Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md)（即不提供列列表）此前期望的值正好是 `(query, tags)`。现在改为 `(id,query,tags,filters)`。如果希望自动生成 id，可以将其设置为 0。
* [`allow_empty=0`](Searching/Highlighting.md#allow_empty) 是通过 HTTP JSON 接口高亮显示时的新默认值。
* 在 `CREATE TABLE`/`ALTER TABLE` 中，外部文件（停用词、例外等）只能使用绝对路径。

### 弃用项目：
* `ram_chunks_count` 在 `SHOW INDEX STATUS` 中重命名为 `ram_chunk_segments_count`。
* `workers` 已废弃。现在只支持单一工作模式。
* `dist_threads` 已废弃。所有查询现在尽可能并行（受 `threads` 和 `jobs_queue_size` 限制）。
* `max_children` 已废弃。使用 [threads](Server_settings/Searchd.md#threads) 来设置 Manticore 将使用的线程数（默认设置为 CPU 核心数）。
* `queue_max_length` 已废弃。如确有需要，请改用 [jobs_queue_size](Server_settings/Searchd.md#jobs_queue_size) 来微调内部作业队列大小（默认无限制）。
* 所有 `/json/*` 端点现在无需 `/json/` 前缀即可访问，例如 `/search`、`/insert`、`/delete`、`/pq` 等。
* `field` 表示“全文字段”的含义在 `describe` 中改为“text”。
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
* 西里尔字母 `и` 不会映射到 `i`，在 `non_cjk` charset_table（默认）中如此，因为这对俄语词干提取和词形还原的影响过大。
* `read_timeout`。改用控制读写的 [network_timeout](Server_settings/Searchd.md#network_timeout)。

### 软件包

* Ubuntu Focal 20.04 官方软件包
* deb 包名从 `manticore-bin` 变更为 `manticore`

### Bug 修复：
1. [Issue #351](https://github.com/manticoresoftware/manticoresearch/issues/351) searchd 内存泄漏
2. [Commit ceab](https://github.com/manticoresoftware/manticoresearch/commit/ceabe44f) 碎片中的小范围越界读取
3. [Commit 1c3e](https://github.com/manticoresoftware/manticoresearch/commit/1c3e84a3) 崩溃查询导致的写入局部变量问题
4. [Commit 26e0](https://github.com/manticoresoftware/manticoresearch/commit/26e094ab) 测试 226 中排序器存在微小内存泄漏
5. [Commit d2c7](https://github.com/manticoresoftware/manticoresearch/commit/d2c7f86a) 测试 226 中大量内存泄漏
6. [Commit 0dd8](https://github.com/manticoresoftware/manticoresearch/commit/0dd80122) 集群显示节点同步，但 `count(*)` 显示不同数值
7. [Commit f1c1](https://github.com/manticoresoftware/manticoresearch/commit/f1c1ac3f) 美化：日志中出现重复且有时丢失的警告信息
8. [Commit f1c1](https://github.com/manticoresoftware/manticoresearch/commit/f1c1ac3f) 美化：日志中的 (null) 索引名
9. [Commit 359d](https://github.com/manticoresoftware/manticoresearch/commit/359dbd30) 无法检索超过 7000 万条结果
10. [Commit 19f3](https://github.com/manticoresoftware/manticoresearch/commit/19f328ee) 不支持无列语法插入 PQ 规则
11. [Commit bf68](https://github.com/manticoresoftware/manticoresearch/commit/bf685d5d) 向集群索引插入文档时误导性错误信息
12. [Commit 2cf1](https://github.com/manticoresoftware/manticoresearch/commit/2cf18c83) `/json/replace` 和 `json/update` 返回的 id 以指数形式显示
13. [Issue #324](https://github.com/manticoresoftware/manticoresearch/issues/324) 在同一查询中更新 json 标量属性和 mva
14. [Commit d384](https://github.com/manticoresoftware/manticoresearch/commit/d38409eb) `hitless_words` 在 RT 模式下无效
15. [Commit 5813](https://github.com/manticoresoftware/manticoresearch/commit/5813d639) 禁止在 rt 模式下执行 `ALTER RECONFIGURE`
16. [Commit 5813](https://github.com/manticoresoftware/manticoresearch/commit/5813d639) `rt_mem_limit` 在 searchd 重启后重置为 128M
17. highlight() 有时会挂起
18. [Commit 7cd8](https://github.com/manticoresoftware/manticoresearch/commit/7cd878f4) 无法在 RT 模式下使用 U+ 代码
19. [Commit 2b21](https://github.com/manticoresoftware/manticoresearch/commit/2b213de4) RT 模式下词形变换无法使用通配符
20. [Commit e9d0](https://github.com/manticoresoftware/manticoresearch/commit/e9d07e68) 修复 `SHOW CREATE TABLE` 与多个词形变换文件的兼容性
21. [Commit fc90](https://github.com/manticoresoftware/manticoresearch/commit/fc90a84f) 缺少 "query" 的 JSON 查询导致 searchd 崩溃
22. Manticore [官方 docker](https://hub.docker.com/r/manticoresearch/manticore) 无法从 mysql 8 索引数据
23. [Commit 23e0](https://github.com/manticoresoftware/manticoresearch/commit/23e05d32) HTTP /json/insert 需要提供 id
24. [Commit bd67](https://github.com/manticoresoftware/manticoresearch/commit/bd679af0) `SHOW CREATE TABLE` 不支持 PQ
25. [Commit bd67](https://github.com/manticoresoftware/manticoresearch/commit/bd679af0) `CREATE TABLE LIKE` 无法正确处理 PQ
26. [Commit 5eac](https://github.com/manticoresoftware/manticoresearch/commit/5eacf28f) show index status 中设置的行尾符号问题
27. [Commit cb15](https://github.com/manticoresoftware/manticoresearch/commit/cb153228) HTTP JSON 响应中 "highlight" 的标题为空
28. [Issue #318](https://github.com/manticoresoftware/manticoresearch/issues/318) `CREATE TABLE LIKE` 存在中缀错误
29. [Commit 9040](https://github.com/manticoresoftware/manticoresearch/commit/9040d22c) RT 在高负载下崩溃
30. [cd512c7d ](https://github.com/manticoresoftware/manticoresearch/commit/cd512c7d ) RT 磁盘块崩溃时丢失崩溃日志
31. [Issue #323](https://github.com/manticoresoftware/manticoresearch/issues/323) 导入表失败并关闭连接
32. [Commit 6275](https://github.com/manticoresoftware/manticoresearch/commit/6275316a) `ALTER reconfigure` 破坏 PQ 索引
33. [Commit 9c1d](https://github.com/manticoresoftware/manticoresearch/commit/9c1d221e) 修改索引类型后 Searchd 重载问题
34. [Commit 71e2](https://github.com/manticoresoftware/manticoresearch/commit/71e2b5bb) 守护进程在导入缺少文件的表时崩溃
35. [Issue #322](https://github.com/manticoresoftware/manticoresearch/issues/322) 使用多个索引、group by 和 ranker = none 的 select 时崩溃
36. [Commit c3f5](https://github.com/manticoresoftware/manticoresearch/commit/c3f58490) `HIGHLIGHT()` 不会高亮字符串属性
37. [Issue #320](https://github.com/manticoresoftware/manticoresearch/issues/320) `FACET` 无法对字符串属性排序
38. [Commit 4f1a](https://github.com/manticoresoftware/manticoresearch/commit/4f1a1f25) 缺少数据目录时出现错误
39. [Commit 04f4](https://github.com/manticoresoftware/manticoresearch/commit/04f4ddd4) access_* 在 RT 模式下不被支持
40. [Commit 1c06](https://github.com/manticoresoftware/manticoresearch/commit/1c0616a2) 字符串中的 JSON 对象错误：1. 当 json 超过某个值时，`CALL PQ` 返回 “Bad JSON objects in strings: 1”。
41. [Commit 32f9](https://github.com/manticoresoftware/manticoresearch/commit/32f943d6) RT 模式不一致。在某些情况下无法删除索引，因为它未知，且无法创建，因为目录非空。
42. [Issue #319](https://github.com/manticoresoftware/manticoresearch/issues/319) select 时崩溃
43. [Commit 22a2](https://github.com/manticoresoftware/manticoresearch/commit/22a28dd7) `max_xmlpipe2_field` = 2M 时，2M 字段返回警告
44. [Issue #342](https://github.com/manticoresoftware/manticoresearch/issues/342) 查询条件执行错误
45. [Commit dd8d](https://github.com/manticoresoftware/manticoresearch/commit/dd8dcab2) 简单的两个词搜索找到仅包含一个词的文档
46. [Commit 9091](https://github.com/manticoresoftware/manticoresearch/commit/90919e62) PQ 中无法匹配键中带大写字母的 json
47. [Commit 56da](https://github.com/manticoresoftware/manticoresearch/commit/56da086a) 索引器在 csv+docstore 上崩溃
48. [Issue #363](https://github.com/manticoresoftware/manticoresearch/issues/363) 在 centos 7 中 json 属性使用 `[null]` 导致插入数据损坏
49. 重大 [Issue #345](https://github.com/manticoresoftware/manticoresearch/issues/345) 记录未被插入，count() 结果随机，"replace into" 返回 OK
50. max_query_time 使 SELECT 过度变慢
51. [Issue #352](https://github.com/manticoresoftware/manticoresearch/issues/352) Master-agent 通信在 Mac OS 上失败
52. [Issue #328](https://github.com/manticoresoftware/manticoresearch/issues/328) 使用 Connector.Net/Mysql 8.0.19 连接 Manticore 出错
53. [Commit daa7](https://github.com/manticoresoftware/manticoresearch/commit/daa760d2fe83668038318f0f384ceefc0d075e18) 修复 \0 转义并优化性能
54. [Commit 9bc5](https://github.com/manticoresoftware/manticoresearch/commit/9bc5c01a493db993b90ff6c1ad51aeae028f629c) 修复 count distinct 与 json 相关问题
55. [Commit 4f89](https://github.com/manticoresoftware/manticoresearch/commit/4f89a965a3af0db548ae9d1763731ebaebe21f5a) 修复在其他节点删除表失败
56. [Commit 952a](https://github.com/manticoresoftware/manticoresearch/commit/952af5a5931fd72b87edc169588777778b365231) 修复频繁调用 call pq 时的崩溃


## 版本 3.4.2，2020 年 4 月 10 日
### 关键漏洞修复
* [Commit 2ffe](https://github.com/manticoresoftware/manticoresearch/commit/2ffe2d261793f9681265bc4abe38d695e72c121e) 修复旧版本 RT 索引无法索引数据的问题

## 版本 3.4.0，2020 年 3 月 26 日
### 重大变更
* 服务器有两种模式：rt 模式和普通模式
   *   rt 模式需配置 data_dir，不允许在配置中定义索引
   *   普通模式定义索引于配置文件；不允许设置 data_dir
* 复制功能仅在 rt 模式可用

### 小改动
* charset_table 默认使用 non_cjk 别名
* rt 模式下全文字段默认被索引和存储
* rt 模式下全文字段由 'field' 改名为 'text'
* ALTER RTINDEX 改名为 ALTER TABLE
* TRUNCATE RTINDEX 改名为 TRUNCATE TABLE

### 新功能
* 仅存储字段
* SHOW CREATE TABLE，IMPORT TABLE

### 改进
* 更快的无锁 PQ
* /sql 支持以 mode=raw 执行任意类型的 SQL 语句
* mysql 协议别名 mysql41
* data_dir 中默认包含 state.sql

### 漏洞修复
* [Commit a533](https://github.com/manticoresoftware/manticoresearch/commit/a5333644e8e0c69746a9f24da8221fc965478e4a) 修复 highlight() 中错误字段语法导致崩溃
* [Commit 7fbb](https://github.com/manticoresoftware/manticoresearch/commit/7fbb9f2e618096289cba7f8b834530b46aa94055) 修复复制带 docstore 的 RT 索引时服务器崩溃
* [Commit 24a0](https://github.com/manticoresoftware/manticoresearch/commit/24a04687eb43b4f4719c7c4a5fe3c19099240467) 修复对带有 infix 或 prefix 选项以及无存储字段索引的高亮导致崩溃
* [Commit 3465](https://github.com/manticoresoftware/manticoresearch/commit/3465c1ce9c860c09f6ccc5b83a63e3c68c241d8d) 修复空 docstore 和空索引的 dock-id 查找错误警告
* [Commit a707](https://github.com/manticoresoftware/manticoresearch/commit/b8601b4144b9092be43a7caa7cd6b19cb37e14c7) 修复 #314 SQL 插入命令尾部分号问题
* [Commit 9562](https://github.com/manticoresoftware/manticoresearch/commit/95628c9b9b8f16fe1bd18599a84e9673575adf1a) 移除查询单词不匹配的警告
* [Commit b860](https://github.com/manticoresoftware/manticoresearch/commit/b8601b4144b9092be43a7caa7cd6b19cb37e14c7) 修复通过 ICU 分段的片段中的查询
* [Commit 5275](https://github.com/manticoresoftware/manticoresearch/commit/5275516ca4631c584e6e0127fa161a18d2658fd0) 修复 docstore 块缓存中的查找/添加竞态条件
* [Commit f06e](https://github.com/manticoresoftware/manticoresearch/commit/f06ef97af5c7390427e82525c150535290e42c3e) 修复 docstore 中的内存泄漏
* [Commit a725](https://github.com/manticoresoftware/manticoresearch/commit/a7258ba8340e246c0d8aaeb4a7a74129eba807eb) 修复 #316 INSERT 时 LAST_INSERT_ID 返回为空
* [Commit 1ebd](https://github.com/manticoresoftware/manticoresearch/commit/1ebd5bf8d74113bcfa138ce63ce11966990a4215) 修复 #317 json/update HTTP 端点支持 MVA 的数组和 JSON 属性的对象
* [Commit e426](https://github.com/manticoresoftware/manticoresearch/commit/e426950a536382a142e2a42ed207c341149ff685) 修复索引器无明确 id 时转储 rt 的问题

## 版本 3.3.0，2020年2月4日
### 新特性
* 并行实时索引搜索
* EXPLAIN QUERY 命令
* 无索引定义的配置文件（测试版）
* CREATE/DROP TABLE 命令（测试版）
* indexer --print-rt - 可从源读取并打印实时索引的 INSERT 语句

### 改进
* 更新为 Snowball 2.0 词干提取器
* SHOW INDEX STATUS 增加 LIKE 过滤器
* 对高 max_matches 的内存使用进行了改进
* SHOW INDEX STATUS 为 RT 索引增加了 ram_chunks_count
* 无锁优先队列
* 将 LimitNOFILE 改为 65536


### Bug 修复
* [Commit 9c33](https://github.com/manticoresoftware/manticoresearch/commit/9c33aab8c1fd9059c8644d23d5f58f03010043c6) 增加索引架构检查，防止属性重复 #293
* [Commit a008](https://github.com/manticoresoftware/manticoresearch/commit/a0085f9477e191da9e0e835b91583fb9cfde698e) 修复无命中的词导致崩溃
* [Commit 6895](https://github.com/manticoresoftware/manticoresearch/commit/68953740af8dca1833026d0f448c71416ed3044a) 修复 ATTACH 后 docstore 松散的问题
* [Commit d6f6](https://github.com/manticoresoftware/manticoresearch/commit/d6f696edea53ea2b9a68251e4054e4816082c285) 修复分布式设置中的 docstore 问题
* [Commit bce2](https://github.com/manticoresoftware/manticoresearch/commit/bce2b7ec883d208f31094ee42a5b6f01d18a998f) 在排序器中用 OpenHash 替换 FixedHash
* [Commit e0ba](https://github.com/manticoresoftware/manticoresearch/commit/e0baf7392937969e640fb5d10e8ccb312fe36446) 修复索引定义中属性名重复的问题
* [Commit ca81](https://github.com/manticoresoftware/manticoresearch/commit/ca81114bf1ac7ea7adcc8509bdc14d1f30b4f461) 修复 HIGHLIGHT() 中的 html_strip
* [Commit 493a](https://github.com/manticoresoftware/manticoresearch/commit/493a5e916d70fd4a3f14d1ee487234434f4a7ce3) 修复 HIGHLIGHT() 中的 passage 宏
* [Commit a82d](https://github.com/manticoresoftware/manticoresearch/commit/a82d41c775692f837f267d554d7d4a4b499d69f0) 修复 RT 索引创建大小不同磁盘块时的双缓冲问题
* [Commit a404](https://github.com/manticoresoftware/manticoresearch/commit/a404c85de0d41d9d1c75b7540479d8a3af9e61aa) 修复 kqueue 的事件删除
* [Commit 8bea](https://github.com/manticoresoftware/manticoresearch/commit/8bea0f6fd1666b817995c4dd6e67f6f1c9161638) 修复 RT 索引 rt_mem_limit 大值时磁盘块保存
* [Commit 8707](https://github.com/manticoresoftware/manticoresearch/commit/8707f0395b7ace52dc0cba9c8ed92a2bc4392932) 修复索引时浮点溢出
* [Commit a564](https://github.com/manticoresoftware/manticoresearch/commit/a56434ce71bb7714b4d8e75278aaa075b27ece80) 修复插入负 ID 文档到 RT 索引时的错误
* [Commit bbeb](https://github.com/manticoresoftware/manticoresearch/commit/bbebfd7551a68641154761cac962053b54806759) 修复服务器在 ranker fieldmask 时崩溃
* [Commit 3809](https://github.com/manticoresoftware/manticoresearch/commit/3809cc1b0863e3a5a200dc423be3f7c38be56113) 修复使用查询缓存崩溃
* [Commit dc2a](https://github.com/manticoresoftware/manticoresearch/commit/dc2a585bc724678679e4297362200bbd49ce32bb) 修复 RT 索引 RAM 段并行插入时崩溃

## 版本 3.2.2，2019年12月19日
### 新特性
* RT 索引的自增 ID
* 通过新 HIGHLIGHT() 函数支持 docstore 高亮，HTTP API 亦可用
* SNIPPET() 可以使用特殊函数 QUERY()，返回当前 MATCH 查询
* 高亮函数新增 field_separator 选项

### 改进与变更
* 远程节点存储字段的延迟获取（可显著提升性能）
* 字符串和表达式不再破坏多查询和 FACET 优化
* RHEL/CentOS 8 版本现在使用 mariadb-connector-c-devel 的 mysql libclient
* ICU 数据文件随软件包发布，移除 icu_data_dir
* systemd 服务文件包含 'Restart=on-failure' 策略
* indextool 现在可以在线检查实时索引
* 默认配置文件改为 /etc/manticoresearch/manticore.conf
* RHEL/CentOS 的服务名由 'searchd' 改为 'manticore'
* 移除 query_mode 和 exact_phrase 片段的选项

### Bug 修复
* [Commit 6ae4](https://github.com/manticoresoftware/manticoresearch/commit/6ae474c7894a6bee222d5b18e59a44fdbf57843a) 修复通过 HTTP 接口执行 SELECT 查询时崩溃
* [Commit 5957](https://github.com/manticoresoftware/manticoresearch/commit/59577513a49eac5a4a3c5e2cb38394d3246b5d35) 修复 RT 索引保存磁盘块但未标记某些文档为删除
* [Commit e861](https://github.com/manticoresoftware/manticoresearch/commit/e861f0fca0e88924450695d4e9d6acff7a36558a) 修复对多索引或多查询使用 dist_threads 时崩溃
* [Commit 4409](https://github.com/manticoresoftware/manticoresearch/commit/440991fc977b8479800b45cdbc862eeb1ba5d965) 修复长词含有宽 UTF8 码点时中缀生成崩溃
* [Commit 5fd5](https://github.com/manticoresoftware/manticoresearch/commit/5fd599b48bba527c023e6aa0b262dca51ffb8a1c) 修复添加套接字到 IOCP 时的竞态条件
* [Commit cf10](https://github.com/manticoresoftware/manticoresearch/commit/cf10d7d3589f77adfaefbe52a51777bc9c67cf99) 修复布尔查询与 json 选择列表的问题
* [Commit 996d](https://github.com/manticoresoftware/manticoresearch/commit/996de77f4c3ec103d965e28c36098fd07dba50c4) 修正 indextool 检查以报告错误的跳跃表偏移，检查 doc2row 查找
* [Commit 6e3f](https://github.com/manticoresoftware/manticoresearch/commit/6e3fc9e88941c9427410f9d0adeebb7dd09a900f) 修正索引器在大量数据上产生负跳跃表偏移的错误索引
* [Commit faed](https://github.com/manticoresoftware/manticoresearch/commit/faed3220b423a96401982bf47117edf1f62c584e) 修正 JSON 仅将数字转换为字符串以及表达式中 JSON 字符串到数字的转换
* [Commit 5331](https://github.com/manticoresoftware/manticoresearch/commit/533197200249d648ebcbdaca5d605861ee566348) 修正 indextool 在命令行设置多个命令时以错误代码退出
* [Commit 7955](https://github.com/manticoresoftware/manticoresearch/commit/795520ac351d22e4497ebaf6c249bf25081842b6) 修正 #275 磁盘空间不足错误时二进制日志处于无效状态
* [Commit 2284](https://github.com/manticoresoftware/manticoresearch/commit/2284da5e3ff77bf7be4038a4d950c0580e4159c8) 修正 #279 JSON 属性的 IN 过滤器崩溃
* [Commit ce2e](https://github.com/manticoresoftware/manticoresearch/commit/ce2e4b4765111a7d331ffc256911f12770a8942d) 修正 #281 错误的管道关闭调用
* [Commit 5355](https://github.com/manticoresoftware/manticoresearch/commit/535589ba8a37baf5c6056afb5ff969fd70476feb) 修正服务器在调用 PQ 时因递归 JSON 属性编码为字符串而挂起
* [Commit a5fc](https://github.com/manticoresoftware/manticoresearch/commit/a5fc8a36e1644ee051632408ab8b53e79aeb9ff4) 修正在 multiand 节点中文档列表越界
* [Commit a362](https://github.com/manticoresoftware/manticoresearch/commit/a3628617ef4ee89a811d4ba2c4a04cbbc298c1b5) 修正检索线程公共信息
* [Commit f8d2](https://github.com/manticoresoftware/manticoresearch/commit/f8d2d7bbbb90af3d2ee97424f97b05927f09d42b) 修正 docstore 缓存锁

## 版本 3.2.0，2019年10月17日
### 新功能
* 文档存储
* 新指令 stored_fields、docstore_cache_size、docstore_block_size、docstore_compression、docstore_compression_level

### 改进和更改
* 改进了 SSL 支持
* 更新了 non_cjk 内置字符集
* 禁止 UPDATE/DELETE 语句在查询日志中记录为 SELECT
* 提供 RHEL/CentOS 8 软件包

### Bug 修复
* [Commit 301a](https://github.com/manticoresoftware/manticoresearch/commit/301a806b14725e842300ee225521407464374c41) 修正 RT 索引的磁盘块中替换文档时崩溃
* [Commit 46c1](https://github.com/manticoresoftware/manticoresearch/commit/46c1cad8fa11c1fc09cc34b20d978fdfcd4e6d27) 修正 #269 LIMIT N OFFSET M
* [Commit 92a4](https://github.com/manticoresoftware/manticoresearch/commit/92a46edaa196ef33f3ac2225fbdba2c2b88ee45d) 修正显式设置 id 或提供 id 列表的 DELETE 语句跳过搜索问题
* [Commit 8ca7](https://github.com/manticoresoftware/manticoresearch/commit/8ca78c138d837caab30dc6e2343a3c4d1687bb87) 修正 Windows poller 中事件移除后的错误索引
* [Commit 6036](https://github.com/manticoresoftware/manticoresearch/commit/603631e2bff5cb0acf25f68b85621742490eccd9) 修正通过 HTTP 请求的 JSON 浮点数四舍五入问题
* [Commit 62f6](https://github.com/manticoresoftware/manticoresearch/commit/62f64cb9e75ec28fc3c29f8414ce9656f5580254) 修正远程片段首先检查空路径问题；修正 Windows 测试
* [Commit aba2](https://github.com/manticoresoftware/manticoresearch/commit/aba274c2c5abc1029894ba9f95b786bda93e8f22) 修正配置重载在 Windows 上的行为与 Linux 一致
* [Commit 6b8c](https://github.com/manticoresoftware/manticoresearch/commit/6b8c4242ef77c3fa4d0ccb7d76d81714b6728f0b) 修正 #194 PQ 支持形态学和词干分析器
* [Commit 174d](https://github.com/manticoresoftware/manticoresearch/commit/174d312905285aa5746f4f404c53e6dc669ef42f) 修正 RT 退役段管理

## 版本 3.1.2，2019年8月22日
### 新功能和改进
* HTTP API 的实验性 SSL 支持
* CALL KEYWORDS 添加字段过滤器
* /json/search 支持 max_matches
* Galera gcache.size 默认大小自动调整
* 改进 FreeBSD 支持

### Bug 修复
* [Commit 0a1a](https://github.com/manticoresoftware/manticoresearch/commit/0a1a2c81038794983b4e30c8aefc7cc20e89b03f) 修正 RT 索引复制到存在相同但路径不同的节点时的问题
* [Commit 4adc](https://github.com/manticoresoftware/manticoresearch/commit/4adc075294ac823289f745e2cc419f18c7dcf2e2) 修正无活动索引的刷新重新调度
* [Commit d6c0](https://github.com/manticoresoftware/manticoresearch/commit/d6c00a6f750c04021747e9c99e0f77707a4cbfc3) 改进 RT/PQ 索引的刷新重新调度
* [Commit d0a7](https://github.com/manticoresoftware/manticoresearch/commit/d0a7c95906b9b5ba33c98267d54501cfe27508d6) 修正 #250 TSV 和 CSV 管道源的 index_field_lengths 索引选项
* [Commit 1266](https://github.com/manticoresoftware/manticoresearch/commit/1266d54800242709fbe2c34e72eb69b6595b23bd) 修正 indextool 对空索引块索引检查的错误报告
* [Commit 553c](https://github.com/manticoresoftware/manticoresearch/commit/553ca73ca111f28e0888d59cd48b0d6df181ecd6) 修正 Manticore SQL 查询日志的空选择列表
* [Commit 56c8](https://github.com/manticoresoftware/manticoresearch/commit/56c8584456293665ddd4ce4c94307d74473a78b9) 修正 indexer 的 -h/--help 响应

## 版本 3.1.0，2019年7月16日
### 新功能和改进
* 实时索引的复制
* 支持中文的 ICU 分词器
* 新的形态学选项 icu_chinese
* 新指令 icu_data_dir
* 支持复制事务的多语句执行
* LAST_INSERT_ID() 和 @session.last_insert_id
* SHOW VARIABLES 支持 LIKE 'pattern'
* percolate 索引支持多文档插入
* 配置新增时间解析器
* 内部任务管理器
* doc 和 hit 列表组件支持 mlock
* 监狱（jail）片段路径

### 移除
* 放弃对RLP库的支持，转而支持ICU；移除所有rlp\*指令
* 禁用使用UPDATE更新文档ID

### Bug修复
* [提交 f047](https://github.com/manticoresoftware/manticoresearch/commit/f047222339fd0e62a07e9ca8b2a829297d5db861) 修正concat和group_concat中的缺陷
* [提交 b081](https://github.com/manticoresoftware/manticoresearch/commit/b08147ee24712b5eaa51f261626b5d16c6242fdd) 修正percolate索引中的查询uid为BIGINT属性类型
* [提交 4cd8](https://github.com/manticoresoftware/manticoresearch/commit/4cd85afaf76eef0ce9fa1cd073f3fed160279890) 如果预分配新的磁盘块失败，不会崩溃
* [提交 1a55](https://github.com/manticoresoftware/manticoresearch/commit/1a551227886474054111088412f2b2396f485652) 为ALTER添加缺失的timestamp数据类型
* [提交 f3a8](https://github.com/manticoresoftware/manticoresearch/commit/f3a8e096fd496a6b001d398aadbb781b2dbad929) 修复错误的mmap读取导致的崩溃
* [提交 4475](https://github.com/manticoresoftware/manticoresearch/commit/447577115192db7ac7bfea70b5ba58ee2c61229f) 修复复制中的集群锁哈希
* [提交 ff47](https://github.com/manticoresoftware/manticoresearch/commit/ff476df955f057bd38e8b6ad6056325e9d6fbb9c) 修复复制中的提供者泄漏
* [提交 58dc](https://github.com/manticoresoftware/manticoresearch/commit/58dcbb779c4c30ca1560460c78e35ba114b83716) 修复 #246 索引器中的未定义sigmask
* [提交 3dd8](https://github.com/manticoresoftware/manticoresearch/commit/3dd8278e747d534f69fe9c3d360eef188ab7b6d5) 修复netloop报告中的竞争
* [提交 a02a](https://github.com/manticoresoftware/manticoresearch/commit/a02aae05a3fa3d1caf2821f43c7e237b6de76336) 为HA策略重新平衡器清零间隙

## 版本 3.0.2，2019年5月31日
### 改进
* 为文档和命中列表添加mmap阅读器
* `/sql` HTTP端点响应现在与`/json/search`响应相同
* 新增指令`access_plain_attrs`、`access_blob_attrs`、`access_doclists`、`access_hitlists`
* 在复制设置中新增指令`server_id`

### 删除
* 删除HTTP `/search` 端点

### 弃用
* `read_buffer`、`ondisk_attrs`、`ondisk_attrs_default`、`mlock`由`access_*`指令替代

### Bug修复
* [提交 849c](https://github.com/manticoresoftware/manticoresearch/commit/849c16e1040f382795ba0635cb48686f9db4f2d8) 允许选择列表中以数字开头的属性名
* [提交 48e6](https://github.com/manticoresoftware/manticoresearch/commit/48e6c302ca37bec0bc49c79619c60bf787ed0708) 修复了UDF中的MVAs，修正了MVA别名
* [提交 0555](https://github.com/manticoresoftware/manticoresearch/commit/055586a9a2ebfa8874f68099c5bc798a145fd4ef) 修复 #187 使用SENTENCE查询时的崩溃
* [提交 93bf](https://github.com/manticoresoftware/manticoresearch/commit/93bf52f23f1c53c8a93a603387ca5463e1fc2ae8) 修复 #143 支持MATCH()周围的()
* [提交 599e](https://github.com/manticoresoftware/manticoresearch/commit/599ee79ccd38cce0023a55cbce8f50c3dca62e38) 修复在ALTER集群语句中的集群状态保存
* [提交 230c](https://github.com/manticoresoftware/manticoresearch/commit/230c321e277514b191629538eb3f02d219113d95) 修复在具有blob属性的ALTER索引时服务器崩溃
* [提交 5802](https://github.com/manticoresoftware/manticoresearch/commit/5802b85aa93a81caea77073a3381ef912fdd677c) 修复 #196 按ID过滤
* [提交 25d2](https://github.com/manticoresoftware/manticoresearch/commit/25d2dabda4a299a29bd375405862126da9b4bfac) 放弃对模板索引进行搜索
* [提交 2a30](https://github.com/manticoresoftware/manticoresearch/commit/2a30d5b49d76faab031d195f533974938769b8e6) 修复SQL回复中ID列具有常规bigint类型


## 版本 3.0.0，2019年5月6日
### 功能和改进
* 新的索引存储。非标量属性不再限制为每个索引4GB大小
* attr_update_reserve指令
* 可以通过UPDATE更新字符串、JSON和MVAs
* 在索引加载时应用杀列表
* killlist_target指令
* 多AND搜索加速
* 更好的平均性能和RAM使用
* 用于升级使用2.x制成的索引的转换工具
* CONCAT()函数
* JOIN CLUSTER cluster AT 'nodeaddress:port'
* ALTER CLUSTER posts UPDATE nodes
* node_address 指令
* SHOW STATUS中打印的节点列表

### 行为变化
* 在带有killlist的索引的情况下，服务器不按照conf中定义的顺序轮换索引，而是遵循killlist目标链
* 搜索索引的顺序不再定义应用杀列表的顺序
* 文档ID现在是有符号的大整数

### 移除指令
* docinfo（现在始终是外部的）、inplace_docinfo_gap、mva_updates_pool

## 版本 2.8.2 GA，2019年4月2日
### 功能和改进
* 每次索引的Galera复制
* OPTION 词形

### 编译说明
Cmake最低版本现在是3.13。编译需要Boost和libssl
开发库。

### Bug修复
* [提交 6967](https://github.com/manticoresoftware/manticoresearch/commit/6967fedb2ef818ec1c825d482563edd05e1c9245) 修复在查询许多分布式索引时选择列表中许多星号的崩溃
* [提交 36df](https://github.com/manticoresoftware/manticoresearch/commit/36df1a407dc08263690e3492518613ace82d69ca) 修复 [\#177](https://github.com/manticoresoftware/manticoresearch/issues/177) 通过Manticore SQL接口的大数据包
* [提交 5793](https://github.com/manticoresoftware/manticoresearch/commit/57932aec734583fa93359faaf5034b2e2c9d352b) 修复 [\#170](https://github.com/manticoresoftware/manticoresearch/issues/170) MVA更新导致的RT优化崩溃
* [提交 edb2](https://github.com/manticoresoftware/manticoresearch/commit/edb24b870423add86eba471d361e0e5aff098b18) 修复在SIGHUP重新加载配置后RT索引删除导致的binlog被移除时的服务器崩溃
* [提交 bd3e](https://github.com/manticoresoftware/manticoresearch/commit/bd3e66e0085bc7e2e351b817dfc972fd8158fbce) 修复mysql握手认证插件payloads修正
* [Commit 6a21](https://github.com/manticoresoftware/manticoresearch/commit/6a217f6ed82fb10f752213b15617b9cedf1e8533) 修复了 RT 索引中的 phrase_boundary 设置问题，详情见 [\#172](https://github.com/manticoresoftware/manticoresearch/issues/172)
* [Commit 3562](https://github.com/manticoresoftware/manticoresearch/commit/3562f652753e4091fd7b5b0f65b58341c8cbcb31) 修复了 ATTACH 索引自身导致的死锁问题，详情见 [\#168](https://github.com/manticoresoftware/manticoresearch/issues/168)
* [Commit 250b](https://github.com/manticoresoftware/manticoresearch/commit/250b3f0e74ad18dda34ba080aa13ca87798a9ac9) 修复了 binlog 在服务器崩溃后保存空元数据的问题
* [Commit 4aa6](https://github.com/manticoresoftware/manticoresearch/commit/4aa6c69ad3a181cd7c5dafb990fb528121f68e59) 修复了由于 RT 索引使用磁盘块时 sorter 中字符串导致的服务器崩溃

## 版本 2.8.1 GA，2019年3月6日
### 新特性与改进
* SUBSTRING_INDEX()
* 支持 SENTENCE 和 PARAGRAPH 形式的 percolate 查询
* 为 Debian/Ubuntu 添加了 systemd 生成器；同时添加了 LimitCORE 以允许生成 core dump

### Bug 修复
* [Commit 84fe](https://github.com/manticoresoftware/manticoresearch/commit/84fe7405d17b59f4dcde15a6c8e8503923503def) 修复了服务器在匹配模式为 all 且全文查询为空时崩溃的问题
* [Commit daa8](https://github.com/manticoresoftware/manticoresearch/commit/daa88b579fa373d5d2e869e43a2e178363b0fef1) 修复了删除静态字符串时发生崩溃的问题
* [Commit 2207](https://github.com/manticoresoftware/manticoresearch/commit/22078537dddbd9ce81a182dcc879f235b6e87004) 修正了 indextool 在致命错误时的退出代码
* [Commit 0721](https://github.com/manticoresoftware/manticoresearch/commit/0721696d6780c200c65d596624a8187983fb7fcb) 修复了 [\#109](https://github.com/manticoresoftware/manticoresearch/issues/109) 中由于错误的精确形式检查导致前缀没有匹配的问题
* [Commit 8af8](https://github.com/manticoresoftware/manticoresearch/commit/8af810111b8e2f87bc9f378172eff1ab9725c7e7) 修复了 [\#161](https://github.com/manticoresoftware/manticoresearch/issues/161) 中 RT 索引配置设置的重新加载问题
* [Commit e2d5](https://github.com/manticoresoftware/manticoresearch/commit/e2d592773f4fd702ace90070a793a842a0fec6ab) 修复了访问大型 JSON 字符串时服务器崩溃的问题
* [Commit 75cd](https://github.com/manticoresoftware/manticoresearch/commit/75cd1342d05cdc352e3dd145cdbc6f79394a165b) 修正了 JSON 文档中 PQ 字段被索引剥离器更改导致来自兄弟字段错误匹配的问题
* [Commit e2f7](https://github.com/manticoresoftware/manticoresearch/commit/e2f775437df922674d772ed4417780492502e65a) 修复了 RHEL7 构建下 JSON 解析导致服务器崩溃的问题
* [Commit 3a25](https://github.com/manticoresoftware/manticoresearch/commit/3a25a5808feb3f8b80866991b436d6c1241618c2) 修复了 JSON 转义时斜杠在边缘位置导致的崩溃
* [Commit be9f](https://github.com/manticoresoftware/manticoresearch/commit/be9f497872bea62dbdccd64fc28294fd7776c289) 修复了选项 'skip_empty' 现在能跳过空文档且不警告它们不是有效的 json
* [Commit 266e](https://github.com/manticoresoftware/manticoresearch/commit/266e0e7b088549722a805fc837bf101ff681a5e8) 修复了 [\#140](https://github.com/manticoresoftware/manticoresearch/issues/161) 中当6位精度不足时浮点数输出8位的问题
* [Commit 3f6d](https://github.com/manticoresoftware/manticoresearch/commit/3f6d23891064ee8e94030c4231497cdd7da33a6a) 修复了空 jsonobj 创建问题
* [Commit f3c7](https://github.com/manticoresoftware/manticoresearch/commit/f3c7848a59b7eedc67c3403d6d4b37ce6fa94dc8) 修复了 [\#160](https://github.com/manticoresoftware/manticoresearch/issues/161) 中空 mva 输出 NULL 而非空字符串的问题
* [Commit 0afa](https://github.com/manticoresoftware/manticoresearch/commit/0afa2ed058e5759470b1d7354c722faab34f98bb) 修复了在无 pthread_getname_np 环境下构建失败的问题
* [Commit 9405](https://github.com/manticoresoftware/manticoresearch/commit/9405fccdeb27f0302a8d9a848981d30080216777) 修复了线程池工作线程导致服务器关闭时崩溃的问题

## 版本 2.8.0 GA，2019年1月28日
### 改进
* 支持分布式 percolate 索引
* CALL PQ 新选项和更改：
    *   skip_bad_json
    *   mode（sparsed/sharded）
    *   JSON 文档可以作为 JSON 数组传递
    *   shift
    *   列名 'UID'、'Documents'、'Query'、'Tags'、'Filters' 改名为 'id'、'documents'、'query'、'tags'、'filters'
* 支持 DESCRIBE pq TABLE
* 不再支持 SELECT FROM pq WHERE UID，请改用 'id'
* pq 索引的 SELECT 查询达到常规模型（例如，可通过 REGEX() 过滤规则）
* ANY/ALL 可用于 PQ 标签
* 表达式对 JSON 字段自动转换，无需显式转换
* 内置 'non_cjk' charset_table 和 'cjk' ngram_chars
* 内置支持 50 种语言的停用词集合
* 停用词声明中的多个文件可用逗号分隔
* CALL PQ 可接受 JSON 数组形式的文档

### Bug 修复
* [Commit a4e1](https://github.com/manticoresoftware/manticoresearch/commit/a4e19afee54dafdc04b0da53372dafd8c0d332d6) 修复了 csjon 相关的内存泄漏
* [Commit 28d8](https://github.com/manticoresoftware/manticoresearch/commit/28d862774874891a03e361da1b0347ebe8066ce0) 修复了 JSON 中缺失值导致的崩溃
* [Commit bf4e](https://github.com/manticoresoftware/manticoresearch/commit/bf4e9ea27c349cdc9ae4e54e960a4a17b90e38fa) 修复了 RT 索引保存空元数据问题
* [Commit 33b4](https://github.com/manticoresoftware/manticoresearch/commit/33b4573529e7c7c7bce19bf9d54054866f30d99a) 修复了词形还原序列丢失形式标记（精确）的错误
* [Commit 6b95](https://github.com/manticoresoftware/manticoresearch/commit/6b95d48240d3b5520afade19c249d79e020a5e88) 修复了字符串属性大于 4M 时使用饱和而非溢出的错误
* [Commit 6214](https://github.com/manticoresoftware/manticoresearch/commit/621418b829e70af36aaa322a2f51ece3f86bc499) 修复了禁用索引时 SIGHUP 导致服务器崩溃的问题
* [Commit 3f7e](https://github.com/manticoresoftware/manticoresearch/commit/3f7e35d1482966cc45d5abbcb2de5de3508d66b0) 修复了同时发送 API 会话状态命令时服务器崩溃的问题
* [Commit cd9e](https://github.com/manticoresoftware/manticoresearch/commit/cd9e4f1709a48ddafc128c450c2d882bc11ba50e) 修复了在对带字段过滤器的RT索引执行删除查询时服务器崩溃的问题
* [Commit 9376](https://github.com/manticoresoftware/manticoresearch/commit/9376470d455fcd256c6abff79c431a6919dfa7ac) 修复了在对分布式索引执行CALL PQ时遇到空文档导致服务器崩溃的问题
* [Commit 8868](https://github.com/manticoresoftware/manticoresearch/commit/8868b207644f4f8cc1ab0c270adb35493fed22e1) 修复了Manticore SQL错误信息超出512字符被截断的问题
* [Commit de9d](https://github.com/manticoresoftware/manticoresearch/commit/de9deda9c142823d6dbf529423417b5c670fae94) 修复了无binlog保存percolate索引时的崩溃问题
* [Commit 2b21](https://github.com/manticoresoftware/manticoresearch/commit/2b219e1a32791e7740bb210b7d408f96abc6374f) 修复了OSX上http接口无法工作的BUG
* [Commit e92c](https://github.com/manticoresoftware/manticoresearch/commit/e92c60240f8f9e1756bfa14fba0bbb402586a389) 修复了indextool在检查MVA时错误的错误信息
* [Commit 238b](https://github.com/manticoresoftware/manticoresearch/commit/238bdea59bad89f097403f1c978658ce45f16c70) 修复了在FLUSH RTINDEX时写锁问题，不再在保存和定期从rt_flush_period刷新时锁定整个索引
* [Commit c26a](https://github.com/manticoresoftware/manticoresearch/commit/c26a236bd42b7082db079a51a84172066d337d64) 修复了ALTER percolate索引时等待搜索加载卡住的问题
* [Commit 9ee5](https://github.com/manticoresoftware/manticoresearch/commit/9ee5703f6c28f57cd5bff5e705966d93fe30d267) 修复了max_children为0时应使用线程池默认工作线程数的问题
* [Commit 5138](https://github.com/manticoresoftware/manticoresearch/commit/5138fc048a1bd146b271ce6a72fe954344281f69) 修复了使用index_token_filter插件且配置了stopwords和stopword_step=0时向索引写入数据出现的错误
* [Commit 2add](https://github.com/manticoresoftware/manticoresearch/commit/2add3d319ac62fe450bf60e89033100853dc2ecf) 修复了索引定义中仍使用aot词形还原器但缺少lemmatizer_base导致崩溃的问题

## 版本 2.7.5 GA，2018年12月4日
### 改进
* 增加了REGEX函数
* json API搜索支持limit/offset
* qcache的性能分析点

### Bug修复
* [Commit eb3c](https://github.com/manticoresoftware/manticoresearch/commit/eb3c7683b9740cff4cc4cadd2ab9f990b9f7d173) 修复了FACET查询中多属性宽类型导致的服务器崩溃
* [Commit d915](https://github.com/manticoresoftware/manticoresearch/commit/d915cf6696514f4be365ffb5981bccaa15983e65) 修复了FACET查询主选择列表中隐式GROUP BY的问题
* [Commit 5c25](https://github.com/manticoresoftware/manticoresearch/commit/5c25dc271a8c11a1c82ca3c88e518acbc6a8c164) 修复了GROUP N BY查询导致的崩溃
* [Commit 85d3](https://github.com/manticoresoftware/manticoresearch/commit/85d30a232d4ffd89916b673a9b2db946d272a945) 修复了内存操作崩溃处理中的死锁问题
* [Commit 8516](https://github.com/manticoresoftware/manticoresearch/commit/85166b5ef2be38061d62f2b20f15acaa0311258a) 修复了indextool检查时的内存消耗问题
* [Commit 58fb](https://github.com/manticoresoftware/manticoresearch/commit/58fb03166830d8b23e7ab9ce56309db7ddac22f4) 修复了gmock包含文件不再需要，因上游已解决该问题

## 版本 2.7.4 GA，2018年11月1日
### 改进
* 远程分布式索引情况下，SHOW THREADS命令打印原始查询而非API调用
* SHOW THREADS新增`format=sphinxql`选项，以SQL格式打印所有查询
* SHOW PROFILE打印新增`clone_attrs`阶段

### Bug修复
* [Commit 4f15](https://github.com/manticoresoftware/manticoresearch/commit/4f155712a0bccc1bd01cc191647bc8cff814888e) 修复了使用无malloc_stats、malloc_trim的libc时构建失败的问题
* [Commit f974](https://github.com/manticoresoftware/manticoresearch/commit/f974f20bda3214a56877c393a192be1a77150958) 修复了CALL KEYWORDS结果集中的单词内特殊符号问题
* [Commit 0920](https://github.com/manticoresoftware/manticoresearch/commit/092083282ea8ae0b2e72fb6989c57ccec81e74ac) 修复了通过API或远程代理对分布式索引执行CALL KEYWORDS失败的问题
* [Commit fd68](https://github.com/manticoresoftware/manticoresearch/commit/fd686bfe88b720ffd7642e36897ba45161cbd7d2) 修复了distributed index中agent_query_timeout未正确传递给代理作为max_query_time的问题
* [Commit 4ffa](https://github.com/manticoresoftware/manticoresearch/commit/4ffa623e9d357a3b0b441615089e211f92f8de32) 修复了磁盘块中文档总数计数被OPTIMIZE命令影响，导致权重计算错误的问题
* [Commit dcaf](https://github.com/manticoresoftware/manticoresearch/commit/dcaf4e0e3c8e9b0fbec47bbf3307feddec0936a6) 修复了blended导致RT索引多重尾部命中的问题
* [Commit eee3](https://github.com/manticoresoftware/manticoresearch/commit/eee381754e902a43a8f499e1c950198a2a3e6ee0) 修复了轮换时的死锁

## 版本 2.7.3 GA，2018年9月26日
### 改进
* CALL KEYWORDS增加sort_mode选项
* VIP连接中的DEBUG可执行'crash <password>'进行服务器的有意SIGEGV操作
* DEBUG支持执行'malloc_stats'以导出searchd.log中的malloc统计信息，执行'malloc_trim'以调用malloc_trim()
* 如果系统中存在gdb，则改进了堆栈回溯信息

### Bug修复
* [Commit 0f3c](https://github.com/manticoresoftware/manticoresearch/commit/0f3cc333f6129451ad1ae2c97905b6c04ba5d454) 修复了Windows下重命名时的崩溃或失败
* [Commit 1455](https://github.com/manticoresoftware/manticoresearch/commit/1455ba2bf8079f03ce7f861a8d9662a360d705de) 修复了32位系统上的服务器崩溃
* [Commit ad37](https://github.com/manticoresoftware/manticoresearch/commit/ad3710d53b5cd6a28b8c60352d0f9e285b03d9fa) 修复了SNIPPET表达式为空时服务器崩溃或挂起的问题
* [Commit b36d](https://github.com/manticoresoftware/manticoresearch/commit/b36d79214364500252665860f000ae85343528b6) 修复了非渐进优化失效和渐进优化对最旧磁盘块不创建kill-list的问题
* [Commit 34b0](https://github.com/manticoresoftware/manticoresearch/commit/34b032499afd42ce47a4c7247814b4031094388a) 修复了线程池工作模式下 SQL 和 API 中 queue_max_length 返回错误的问题
* [Commit ae4b](https://github.com/manticoresoftware/manticoresearch/commit/ae4b3202cbdb8014cabe2b90e269d5cf74f49871) 修复了在 PQ 索引中添加带有 regexp 或 rlp 选项的全扫描查询时崩溃的问题
* [Commit f80f](https://github.com/manticoresoftware/manticoresearch/commit/f80f8d5d7560187078868aed9a9575f4549e98aa) 修复了连续调用多个 PQ 导致崩溃的问题
* [Commit 9742](https://github.com/manticoresoftware/manticoresearch/commit/9742f5f0866af73f8cd8483ecd18a507ea80dd65) 重构 AcquireAccum
* [Commit 39e5](https://github.com/manticoresoftware/manticoresearch/commit/39e5bc3751b7295222eb76407c5d72ce1dad545b) 修复调用 pq 后的内存泄漏问题
* [Commit 21bc](https://github.com/manticoresoftware/manticoresearch/commit/21bcc6d17395f0f57dde79f6716ef303b7ea527d) 美化重构（使用 c++11 风格的 c-trs、默认值、nullptr）
* [Commit 2d69](https://github.com/manticoresoftware/manticoresearch/commit/2d690398f14c736956cfdd66feb4d3091d6b3a4d) 修复尝试向 PQ 索引插入重复项时的内存泄漏
* [Commit 5ed9](https://github.com/manticoresoftware/manticoresearch/commit/5ed92c4b66da6423fa28f11b1fd59103ed1e1fb9) 修复 JSON 字段 IN 操作时传入大值导致的崩溃
* [Commit 4a52](https://github.com/manticoresoftware/manticoresearch/commit/4a5262e2776aa261e34ba5c914ea60804f59de3f) 当调用 RT 索引带有展开限制的 CALL KEYWORDS 语句时修复服务器崩溃
* [Commit 5526](https://github.com/manticoresoftware/manticoresearch/commit/552646bb6fefde4e2525298e43e628839b421d3d) 修复 PQ matches 查询中的无效过滤器；
* [Commit 204f](https://github.com/manticoresoftware/manticoresearch/commit/204f521408b730198a5bab2c20407a3d7df94873) 为指针属性引入小对象分配器
* [Commit 2545](https://github.com/manticoresoftware/manticoresearch/commit/25453e5387152c0575d1eda1b2beb89a49c7493a) 将 ISphFieldFilter 重构为引用计数版本
* [Commit 1366](https://github.com/manticoresoftware/manticoresearch/commit/1366ee06e828397074b69bec9265d7147170c60e) 修复对非终止字符串使用 strtod 导致的未定义行为/段错误
* [Commit 94bc](https://github.com/manticoresoftware/manticoresearch/commit/94bc6fcbdae4d08bcf6da2966e3ec2966091289f) 修复 JSON 结果集处理过程中的内存泄漏
* [Commit e78e](https://github.com/manticoresoftware/manticoresearch/commit/e78e9c948963416caff9e4e46296a58080107835) 修复应用属性添加时内存块越界读取的问题
* [Commit fad5](https://github.com/manticoresoftware/manticoresearch/commit/fad572fb543606b33e688eb82a485b86011545fd) 重构 CSphDict，改为引用计数版本
* [Commit fd84](https://github.com/manticoresoftware/manticoresearch/commit/fd841a472b472280c93f4af3506851d19f6adc45) 修复 AOT 内部类型外泄漏
* [Commit 5ee7](https://github.com/manticoresoftware/manticoresearch/commit/5ee7f208c7685f2ebb9e5623a8802b721e2a7f0a) 修复分词器管理内存泄漏
* [Commit 116c](https://github.com/manticoresoftware/manticoresearch/commit/116c5f1abebee9a0d99afe93546a1d8e4c6c6958) 修复 grouper 中的内存泄漏
* [Commit 56fd](https://github.com/manticoresoftware/manticoresearch/commit/56fdbc9ab38973a9a0ba8ccee45f71cf33c16423) 在 matches 中为动态指针提供特殊的释放/复制（解决 grouper 内存泄漏）
* [Commit b1fc](https://github.com/manticoresoftware/manticoresearch/commit/31db660f378541aa0d1e7cf2e7209bb0a1f47fd8) 修复 RT 动态字符串内存泄漏
* [Commit 517b](https://github.com/manticoresoftware/manticoresearch/commit/517b9e8749f092e923cd2884b674b5bb84e20755) 重构 grouper
* [Commit b1fc](https://github.com/manticoresoftware/manticoresearch/commit/b1fc16140e5dc44290686330b476a254e0092107) 轻微重构（c++11 c-trs，部分格式调整）
* [Commit 7034](https://github.com/manticoresoftware/manticoresearch/commit/7034e073f4cf2844762e0a464b7c3de05d3d122b) 将 ISphMatchComparator 重构为引用计数版本
* [Commit b1fc](https://github.com/manticoresoftware/manticoresearch/commit/b1fc16140e5dc44290686330b476a254e0092107) 将克隆器私有化
* [Commit efbc](https://github.com/manticoresoftware/manticoresearch/commit/efbc0511d6809c4a57453c7283f9bf53f3fb8d97) 简化 MVA_UPSIZE、DOCINFO2ID_T、DOCINFOSETID 的本地小端处理
* [Commit 6da0](https://github.com/manticoresoftware/manticoresearch/commit/6da0df4ac9946fc59f98d229b90e12c9221dd4c0) 为 ubertests 添加 valgrind 支持
* [Commit 1d17](https://github.com/manticoresoftware/manticoresearch/commit/1d17669cb7ec54ac80b392e101f37a688eb98080) 修复连接上 'success' 标志竞争导致的崩溃
* [Commit 5a09](https://github.com/manticoresoftware/manticoresearch/commit/5a09c32d7ded56b2c0bd3e2ad7968cb8d6362919) 将 epoll 切换为边缘触发模式
* [Commit 5d52](https://github.com/manticoresoftware/manticoresearch/commit/5d528682737fca03a4352a3093b1319ec27d598c) 修复表达式中 IN 语句格式错误，类似过滤器格式
* [Commit bd8b](https://github.com/manticoresoftware/manticoresearch/commit/bd8b3c976ff8b4667f55dfdd101b20a920137ac5) 修复 RT 索引提交具有大 docid 的文档时崩溃
* [Commit ce65](https://github.com/manticoresoftware/manticoresearch/commit/ce656b83b747ce7c55795b53915770c13fb99cfe) 修复 indextool 中无参数选项的错误
* [Commit 08c9](https://github.com/manticoresoftware/manticoresearch/commit/08c9507177820f441f534bf06a11dac5e54bebb4) 修复展开关键字时的内存泄漏

* [Commit 30c7](https://github.com/manticoresoftware/manticoresearch/commit/30c75a2f525ec9bda625d903acdc9d4d2e2413dc) 修复 json grouper 中的内存泄漏
* [Commit 6023](https://github.com/manticoresoftware/manticoresearch/commit/6023f269b6f2080e4d380b0957605ef8107c8c9f) 修复全局用户变量泄漏
* [Commit 7c13](https://github.com/manticoresoftware/manticoresearch/commit/7c138f15ca23c0c8717fa12041e3db7f988fcf15) 修复早期拒绝匹配中动态字符串泄漏
* [Commit 9154](https://github.com/manticoresoftware/manticoresearch/commit/9154b18eaed5302e8330cb3a95bd968959ccb312) 修复了 length(<expression>) 的泄漏问题
* [Commit 43fc](https://github.com/manticoresoftware/manticoresearch/commit/43fca3a4e26139765d0fac8de054200dc4875fc6) 修复了 parser 中因 strdup() 导致的内存泄漏
* [Commit 71ff](https://github.com/manticoresoftware/manticoresearch/commit/71ff77737e63d1019b7c9804dca7fa2351025dba) 重构表达式解析器以准确跟踪引用计数
## 版本 2.7.2 GA，2018年8月27日
### 改进
* 与 MySQL 8 客户端兼容
* 支持带 WITH RECONFIGURE 的 [TRUNCATE](Emptying_a_table.md)
* 在 SHOW STATUS 中停用 RT 索引的内存计数器
* 多代理的全局缓存
* 改进了 Windows 下的 IOCP
* HTTP 协议的 VIP 连接
* Manticore SQL 的 [DEBUG](Reporting_bugs.md#DEBUG) 命令，可以运行多种子命令
* [shutdown_token](Server_settings/Searchd.md#shutdown_token) - 调用 DEBUG 命令中的 `shutdown` 所需密码的 SHA1 哈希
* 新增 SHOW AGENT STATUS 的统计信息 (_ping, _has_perspool, _need_resolve)
* indexer 的 --verbose 选项现在支持 \[debugvv\] 来打印调试信息

### 错误修复
* [Commit 3900](https://github.com/manticoresoftware/manticoresearch/commit/390082a7be0a1f9539b30361d11d54de35c62a44) 删除了优化过程中的写锁
* [Commit 4c33](https://github.com/manticoresoftware/manticoresearch/commit/4c337648329be22373be33333cbc9c5664d18b80) 修复了重新加载索引设置时的写锁问题
* [Commit b5ea](https://github.com/manticoresoftware/manticoresearch/commit/b5ea8dc0b18a23dbd739e92c66dd93b0094df14b) 修复了带有 JSON 过滤器的查询中的内存泄漏
* [Commit 930e](https://github.com/manticoresoftware/manticoresearch/commit/930e831a43fd3d1ce996185be968d1df49a088d0) 修复了 PQ 结果集中空文档的问题
* [Commit 53de](https://github.com/manticoresoftware/manticoresearch/commit/53deecfecb374431f84f8592a1095a77407b8aea) 修正了因为移除任务导致的任务混淆
* [Commit cad9](https://github.com/manticoresoftware/manticoresearch/commit/cad9b9c617719aad97e9f5a2ae93491153d8e504) 修复了远程主机统计计数错误
* [Commit 9000](https://github.com/manticoresoftware/manticoresearch/commit/90008cc15c4684d445d37a391bb4a667bbc2e365) 修复了已解析代理描述符的内存泄漏
* [Commit 978d](https://github.com/manticoresoftware/manticoresearch/commit/978d839641ad6040c22c9fc3b703b1a02685f266) 修复了搜索中的内存泄漏
* [Commit 0193](https://github.com/manticoresoftware/manticoresearch/commit/0193946105ac08913362b7d7acf0c90f9b38dc85) 对显式/内联 c-trs 和 override/final 用法进行了修饰性调整
* [Commit 943e](https://github.com/manticoresoftware/manticoresearch/commit/943e2997b33f9b2eda23d94a66068910f9476ebd) 修复了本地/远程 schema 中 json 的内存泄漏
* [Commit 02db](https://github.com/manticoresoftware/manticoresearch/commit/02dbdd6f1d0b72e7e458ebebdd2c67f989577e6b) 修复了本地/远程 schema 中 json 排序列表达式的内存泄漏
* [Commit c74d](https://github.com/manticoresoftware/manticoresearch/commit/c74d0b4035e4214510376845b22cb676a8da24a3) 修复了常量别名的内存泄漏
* [Commit 6e5b](https://github.com/manticoresoftware/manticoresearch/commit/6e5b57e1d3952c9695376728a143bf5434208f53) 修复了预读取线程的内存泄漏
* [Commit 39c7](https://github.com/manticoresoftware/manticoresearch/commit/39c740fef5bf71da0bee60a35f5dd9f471af850f) 修复了网络循环中等待阻塞导致退出卡死的问题
* [Commit adaf](https://github.com/manticoresoftware/manticoresearch/commit/adaf9772f95772ad6d5297acace6c5cb92b56ad5) 修复了将 HA 代理切换为普通主机时 'ping' 行为卡死的问题
* [Commit 32c4](https://github.com/manticoresoftware/manticoresearch/commit/32c40eac9cdd40b15d79e58b2f15416f164f9701) 将仪表盘存储的垃圾回收分离
* [Commit 511a](https://github.com/manticoresoftware/manticoresearch/commit/511a3cf2c1f1858f6e94fe9f8175b7216db3cbd6) 修复了引用计数指针的问题
* [Commit 32c4](https://github.com/manticoresoftware/manticoresearch/commit/af2326cd2927effbad7bec88e8dc238a566cf27e) 修复了 indextool 在索引不存在时崩溃的问题
* [Commit 156e](https://github.com/manticoresoftware/manticoresearch/commit/156edc717dc63d3ea120f42466eba6e022da88be) 修正了 xmlpipe 索引中多出的属性/字段的输出名称
* [Commit cdac](https://github.com/manticoresoftware/manticoresearch/commit/cdac6d1f17d384bfc8bae49e91241b2f346c3830) 修复了配置文件无 indexer 部分时默认 indexer 的值
* [Commit e61e](https://github.com/manticoresoftware/manticoresearch/commit/e61ec00b6b27d1d5878247e2ee817f3b1e7fde16) 修复了服务器重启后 RT 索引磁盘块中错误嵌入的停用词
* [Commit 5fba](https://github.com/manticoresoftware/manticoresearch/commit/5fba49d5e28c17de4c0acbd984466127f42de6e8) 修复了跳过幽灵（已关闭但未从轮询器中最终删除）连接的问题
* [Commit f22a](https://github.com/manticoresoftware/manticoresearch/commit/f22ae34b623906b3c5a05a06198dad2e548dc541) 修复了混合（孤立）网络任务
* [Commit 4689](https://github.com/manticoresoftware/manticoresearch/commit/46890e70eb8dcd00c1bf1b030538b8f131c601c2) 修复了写入后读取操作崩溃的问题
* [Commit 03f9](https://github.com/manticoresoftware/manticoresearch/commit/03f9df904f611c438d5ebcaaeef911b0dc8d6c39) 修复了 Windows 上运行测试时 searchd 崩溃的问题
* [Commit e925](https://github.com/manticoresoftware/manticoresearch/commit/e9255ed2704790a3d7f5273d38b85433463cb3ff) 修复了在常规 connect() 中处理 EINPROGRESS 代码的问题
* [Commit 248b](https://github.com/manticoresoftware/manticoresearch/commit/248b72f1544b27e9e82773284050d44eeb1714e1) 修复了使用 TFO 时的连接超时问题

## 版本 2.7.1 GA，2018年7月4日
### 改进
* 改进了 PQ 上匹配多个文档时的通配符性能
* 支持 PQ 上的全表扫描查询
* 支持 PQ 上的 MVA 属性
* 支持 Percolate 索引的正则表达式和 RLP

### 错误修复
* [Commit 6885](https://github.com/manticoresoftware/manticoresearch/commit/68856261b41f6950666f9e5122726839fb3c71d0) 修复了查询字符串丢失问题
* [Commit 0f17](https://github.com/manticoresoftware/manticoresearch/commit/0f1770943cba53bfd2e0edfdf0b1495ff0dd0e89) 修复了 SHOW THREADS 语句中的空信息问题
* [Commit 53fa](https://github.com/manticoresoftware/manticoresearch/commit/53faa36a916bb87868e83aa79c1a1f972fb20ca0) 修复了使用 NOTNEAR 运算符匹配时崩溃的问题
* [Commit 2602](https://github.com/manticoresoftware/manticoresearch/commit/26029a7cd77b518fb3a27588d7db6c8bffc73b0f) 修复了 PQ 删除时错误的过滤器消息


## 版本 2.7.0 GA，2018年6月11日
### 改进
* 减少系统调用数量以避免 Meltdown 和 Spectre 补丁的影响
* 本地索引管理的内部重写
* 远程片段重构
* 完整配置重载
* 所有节点连接现在都是独立的
* 协议改进
* Windows 通信从 wsapoll 切换到 IO 完成端口
* TFO 可用于主节点和子节点之间的通信
* [SHOW STATUS](Node_info_and_management/Node_status.md#SHOW-STATUS) 现在输出服务器版本和 mysql_version_string
* 为 CALL PQ 中调用的文档添加了 `docs_id` 选项。
* 轮询查询过滤器现在可以包含表达式
* 分布式索引可以与 FEDERATED 一起使用
* 虚拟 SHOW NAMES COLLATE 和 `SET wait_timeout`（以更好兼容 ProxySQL）

### 修复漏洞
* [Commit 5bcf](https://github.com/manticoresoftware/manticoresearch/commit/5bcff05d19e495f1a44f190885b25cbb6f217c4a) 修复了 PQ 的不等于标签添加问题
* [Commit 9ebc](https://github.com/manticoresoftware/manticoresearch/commit/9ebc58916cd515eaa88da66d0895aebf0d1f2b5f) 修复了 CALL PQ 语句中添加文档 ID 字段到 JSON 文档的问题
* [Commit 8ae0](https://github.com/manticoresoftware/manticoresearch/commit/8ae0e593a286110d8505d88baa3ac9e433cb38c9) 修复了 PQ 索引的刷新语句处理程序
* [Commit c24b](https://github.com/manticoresoftware/manticoresearch/commit/c24b152344ecd77661566cddb803487f51d3c1aa) 修复了 PQ 对 JSON 和字符串属性的过滤
* [Commit 1b8b](https://github.com/manticoresoftware/manticoresearch/commit/1b8bdde19aaa362785ea4c33bdc019154f8fbac4) 修复了解析空 JSON 字符串的问题
* [Commit 1ad8](https://github.com/manticoresoftware/manticoresearch/commit/1ad8a07dbfc1131913a8d92c261fbb48f934e8b7) 修复了带 OR 过滤器的多查询时崩溃的问题
* [Commit 69b8](https://github.com/manticoresoftware/manticoresearch/commit/69b89806380a229e36287ad9daf6f0ea2b5453eb) 修复了 indextool 使用公共配置部分（lemmatizer_base 选项）执行命令（dumpheader）的问题
* [Commit 6dbe](https://github.com/manticoresoftware/manticoresearch/commit/6dbeaf2389d2af6a9dfccb56bfc986685b9f203e) 修复了结果集和过滤器中的空字符串
* [Commit 39c4](https://github.com/manticoresoftware/manticoresearch/commit/39c4eb55ecc85f6dd54f6c8c6d6dfcf87fd8748e) 修复了负文档 ID 值的问题
* [Commit 266b](https://github.com/manticoresoftware/manticoresearch/commit/266b7054f98e7d30fc2829b5c2467dc9f90def92) 修复了非常长词索引时词截取长度的问题
* [Commit 4782](https://github.com/manticoresoftware/manticoresearch/commit/47823b0bd06cbd016a24345c99bb8693790b43dd) 修复了 PQ 中通配符查询匹配多文档的问题


## 版本 2.6.4 GA，2018年5月3日
### 新功能和改进
* MySQL FEDERATED 引擎[支持](Extensions/FEDERATED.md)
* MySQL 数据包现在返回 SERVER_STATUS_AUTOCOMMIT 标志，增强了与 ProxySQL 的兼容性
* [listen_tfo](Server_settings/Searchd.md#listen_tfo) - 为所有监听器启用 TCP Fast Open 连接
* indexer --dumpheader 也可以从 .meta 文件中导出 RT 头信息
* Ubuntu Bionic 的 cmake 构建脚本

### 修复漏洞
* [Commit 355b](https://github.com/manticoresoftware/manticoresearch/commit/355b11629174813abd9cd5bf2233be0783f77745) 修复了 RT 索引查询缓存条目无效的问题；

* [Commit 546e](https://github.com/manticoresoftware/manticoresearch/commit/546e2297a01cc2913bc0d33052d5bcefae8737eb) 修复了无缝轮换后索引设置丢失的问题
* [Commit 0c45](https://github.com/manticoresoftware/manticoresearch/commit/0c4509898393993bc87194d2aca2070395ff7f83) 修正了固定前缀与中缀长度设置的问题；增加了对不支持的中缀长度警告
* [Commit 8054](https://github.com/manticoresoftware/manticoresearch/commit/80542fa54d727c781635918e8d4d65a20c2fbd1e) 修复了 RT 索引自动刷新顺序问题

* [Commit 705d](https://github.com/manticoresoftware/manticoresearch/commit/705d8c5f407726ed1c6f9745ecffa375682c8969) 修复了多属性索引及多个索引查询的结果集模式问题
* [Commit b0ba](https://github.com/manticoresoftware/manticoresearch/commit/b0ba932fff4a082f642cb737381c8a70215d4cc9) 修复了批量插入中带有重复文档时部分命中丢失的问题
* [Commit 4510](https://github.com/manticoresoftware/manticoresearch/commit/4510fa44ad54b5bbac53c591144501565130fcae) 修复了优化无法合并大文档数量的 RT 索引磁盘块问题
## 版本 2.6.3 GA，2018年3月28日
### 改进
* 编译时支持 jemalloc。如果系统存在 jemalloc，可通过 cmake 标志 `-DUSE_JEMALLOC=1` 启用
### 修复漏洞
* [Commit 85a6](https://github.com/manticoresoftware/manticoresearch/commit/85a6d7e3db75f28c491042409bb677ed39eba07b) 修复了将 expand_keywords 选项写入 Manticore SQL 查询日志的问题
* [Commit caaa](https://github.com/manticoresoftware/manticoresearch/commit/caaa3842f4dbd8507fd9e20bf3371a0c8ec0c087) 修复了 HTTP 接口正确处理大尺寸查询的问题
* [Commit e386](https://github.com/manticoresoftware/manticoresearch/commit/e386d842b36195719e459b2db1fb20e402f9a51f) 修复了启用 index_field_lengths 的 RT 索引 DELETE 导致服务器崩溃的问题
* [Commit cd53](https://github.com/manticoresoftware/manticoresearch/commit/ce528f37eb5d8284661928f6f3b340a4a1985253) 修复了 cpustats searchd 命令行选项在不支持的系统上工作的问题
* [Commit 8740](https://github.com/manticoresoftware/manticoresearch/commit/8740fd63ae842c2ab0e51a8fcd6180d787ed57ec) 修复了定义了最小长度的 utf8 子串匹配问题


## 版本 2.6.2 GA，2018年2月23日
### 改进
* 在使用 NOT 操作符和批处理文档的情况下，改进了 [Percolate Queries](Searching/Percolate_query.md) 的性能。
* [percolate_query_call](Searching/Percolate_query.md) 可以根据 [dist_threads](Server_settings/Searchd.md#threads) 使用多线程。
* 新增全文匹配操作符 NOTNEAR/N。
* 针对 percolate 索引的 SELECT 支持 LIMIT。
* [expand_keywords](Searching/Options.md#expand_keywords) 可以接受 'start','exact'（其中 'start,exact' 的效果与 '1' 相同）。
* 针对使用 sql_query_range 定义的范围查询的 [joined fields](Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Processing_fetched_data.md#sql_joined_field) 支持范围主查询。

### Bug修复
* [Commit 72dc](https://github.com/manticoresoftware/manticoresearch/commit/72dcf669744e9b7d636dfc213d24df85ab301f6b) 修复了在搜索内存段时的崩溃；修复了使用双缓冲保存磁盘块时的死锁；修复了优化期间保存磁盘块时的死锁。
* [Commit 3613](https://github.com/manticoresoftware/manticoresearch/commit/36137149a1c3c0893bdda5a28fc7e8244bf2d4ae) 修复了带有空属性名的 xml 嵌入式模式导致的索引器崩溃。
* [Commit 48d7](https://github.com/manticoresoftware/manticoresearch/commit/48d7e8001d2a66466ca64577f27ddc5421a67251) 修复了错误地取消链接非拥有的 pid 文件的问题。
* [Commit a556](https://github.com/manticoresoftware/manticoresearch/commit/a5563a465ddc59ef71e65f17b68bc33f9700e838) 修复了有时遗留在临时文件夹中的孤立 fifo。
* [Commit 2376](https://github.com/manticoresoftware/manticoresearch/commit/2376e8fc4508944b96959bd10686c6d51f5145e8) 修复了 FACET 结果集为空且包含错误 NULL 行的问题。
* [Commit 4842](https://github.com/manticoresoftware/manticoresearch/commit/4842b67acdd5da75672db6c08ac563b48577ddd7) 修复了作为 Windows 服务运行服务器时的损坏索引锁问题。
* [Commit be35](https://github.com/manticoresoftware/manticoresearch/commit/be35feef54ded59125750916cd211e56108eddae) 修复了 macOS 上错误的 iconv 库问题。
* [Commit 8374](https://github.com/manticoresoftware/manticoresearch/commit/83744a977d7837d1d87cd506b88b0e7eb199efe6) 修复了错误的 count(*) 统计。


## 版本 2.6.1 GA，2018 年 1 月 26 日
### 改进
* [agent_retry_count](Server_settings/Searchd.md#agent_retry_count) 对于具有镜像的代理，返回每个镜像的重试次数，而非每个代理，代理的总重试次数为 agent_retry_count\*mirrors。
* [agent_retry_count](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_retry_count) 现在可以为每个索引单独指定，覆盖全局值。新增别名 [mirror_retry_count](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#mirror_retry_count)。
* 可以在代理定义中指定 retry_count，该值表示每个代理的重试次数。
* Percolate 查询现已通过 HTTP JSON API 提供，路径为 [/json/pq](Searching/Percolate_query.md#Performing-a-percolate-query-with-CALL-PQ)。
* 为可执行文件新增 -h 和 -v 选项（帮助和版本）。
* [morphology_skip_fields](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) 支持实时索引。

### Bug修复
* [Commit a40b](https://github.com/manticoresoftware/manticore/commit/a40b0793feff65e40d10062568d9847c08d10f57) 修复了带有 sql_range_step 的范围主查询在 MVA 字段上的正确工作。
* [Commit f2f5](https://github.com/manticoresoftware/manticore/commit/f2f53757db45bcfb1544263ce0817e856656a621) 修复了黑洞系统循环挂起问题和黑洞代理看似断开的情况。
* [Commit 84e1](https://github.com/manticoresoftware/manticore/commit/84e1f54aef25e0fce98870ad2dd784db5116f1d6) 修复了查询 ID 不一致的问题，修复了存储查询重复的 ID。
* [Commit 1948](https://github.com/manticoresoftware/manticore/commit/19484231814fcb82b21763a3a4a9f45adc6b2d40) 修复了服务器从各种状态关闭时的崩溃。
* [Commit 9a70](https://github.com/manticoresoftware/manticore/commit/9a706b499a1d61a90076065a1a703029d49db958)  [Commit 3495](https://github.com/manticoresoftware/manticore/commit/3495fd70cba8846b1a50d55d6679b039414c3d2a) 修复了长查询超时问题。
* [Commit 3359](https://github.com/manticoresoftware/manticore/commit/3359bcd89b4f79a645fe84b8cf8616ce0addff02) 重构了基于 kqueue 的系统（Mac OS X、BSD）上的主-代理网络轮询。


## 版本 2.6.0，2017 年 12 月 29 日
### 新功能与改进
* [HTTP JSON](Connecting_to_the_server/HTTP.md)：JSON 查询现在可以对属性进行等值匹配，MVA 和 JSON 属性可用于插入和更新，分布式索引也可以通过 JSON API 进行更新和删除。
* [Percolate Queries](Searching/Percolate_query.md)
* 移除了对 32 位 docid 的支持，也移除了转换/加载带有 32 位 docid 的遗留索引的所有代码。
* [仅针对某些字段启用形态学](https://github.com/manticoresoftware/manticore/issues/7)。新增索引指令 morphology_skip_fields，用于定义不应用形态学的字段列表。
* [expand_keywords 现可作为查询运行时指令通过 OPTION 语句设置](https://github.com/manticoresoftware/manticore/issues/8)

### Bug修复
* [Commit 0cfa](https://github.com/manticoresoftware/manticore/commit/0cfae4c) 修复了服务器（以及可能的发布版本未定义行为）在使用 rlp 构建的调试版本崩溃的问题。
* [Commit 3242](https://github.com/manticoresoftware/manticore/commit/324291e) 修复了启用 progressive 选项的 RT 索引优化中杀死列表合并顺序错误的问题。
* [Commit ac0e](https://github.com/manticoresoftware/manticore/commit/ac0efee) 修复了 Mac 上的轻微崩溃。
* 大量通过深入静态代码分析后的微小修复。
* 其他小幅度 Bug 修复。

### 升级
本次发布更改了主节点与代理之间通信所使用的内部协议。如果您在分布式环境下运行 Manticoresearch，包含多个实例，请确保先升级代理，再升级主节点。

## 版本 2.5.1，2017 年 11 月 23 日
### 新功能与改进
* JSON 查询基于 [HTTP API protocol](Connecting_to_the_server/HTTP.md)。支持搜索、插入、更新、删除、替换操作。数据操作命令也可以批量执行，目前有些限制，比如 MVA 和 JSON 属性不能用于插入、替换或更新。
* [RELOAD INDEXES](Data_creation_and_modification/Adding_data_from_external_storages/Rotating_a_table.md#RELOAD-TABLES) 命令
* [FLUSH LOGS](Logging/Rotating_query_and_server_logs.md) 命令

* [SHOW THREADS](Node_info_and_management/SHOW_THREADS.md) 可以显示优化、轮换或刷新进度。
* GROUP N BY 能正确处理 MVA 属性
* blackhole 代理现在在独立线程运行，不再影响主查询
* 对索引实现了引用计数，以避免因轮换和高负载带来的停顿
* 实现了 SHA1 哈希，尚未对外暴露
* 修复了在 FreeBSD、macOS 和 Alpine 上的编译问题
### Bugfixes
* [Commit 9897](https://github.com/manticoresoftware/manticore/commit/989752b) 修复了块索引的过滤回归问题
* [Commit b1c3](https://github.com/manticoresoftware/manticore/commit/b1c3864) 将 PAGE_SIZE 重命名为 ARENA_PAGE_SIZE 以兼容 musl
* [Commit f213](https://github.com/manticoresoftware/manticore/commit/f2133cc) 禁用 cmake < 3.1.0 的 googletests
* [Commit f30e](https://github.com/manticoresoftware/manticore/commit/0839de7) 服务器重启时绑定套接字失败
* [Commit 0807](https://github.com/manticoresoftware/manticore/commit/0807240) 修复服务器关闭时崩溃
* [Commit 3e3a](https://github.com/manticoresoftware/manticore/commit/3e3acc3) 修复系统 blackhole 线程的 show threads
* [Commit 262c](https://github.com/manticoresoftware/manticore/commit/262c3fe) 重构了 iconv 的配置检查，修复了在 FreeBSD 和 Darwin 上的构建问题

## 版本 2.4.1 GA，2017 年 10 月 16 日
### 功能和改进
* WHERE 子句中属性过滤器间支持 OR 操作符
* 维护模式 ( SET MAINTENANCE=1)
* 分布式索引支持 [CALL KEYWORDS](Searching/Autocomplete.md#CALL-KEYWORDS)
* [UTC 分组](Server_settings/Searchd.md#grouping_in_utc)
* [query_log_mode](Server_settings/Searchd.md#query_log_mode) 可用于自定义日志文件权限
* 字段权重可以为零或负数
* [max_query_time](Searching/Options.md#max_query_time) 现在可影响全扫描
* 添加了 [net_wait_tm](Server_settings/Searchd.md#net_wait_tm)、[net_throttle_accept](Server_settings/Searchd.md#net_throttle_accept) 和 [net_throttle_action](Server_settings/Searchd.md#net_throttle_action) 用于网络线程微调（当 workers=thread_pool 时）
* COUNT DISTINCT 支持 facet 搜索
* IN 操作符可用于 JSON 浮点数组
* 多查询优化不再被整数/浮点表达式破坏
* 使用多查询优化时，[SHOW META](Node_info_and_management/SHOW_META.md) 显示 `multiplier` 行

### 编译
Manticore Search 使用 cmake 构建，最低编译所需的 gcc 版本为 4.7.2。

### 目录和服务
* Manticore Search 以 `manticore` 用户身份运行。
* 默认数据目录现在是 `/var/lib/manticore/`。
* 默认日志目录现在是 `/var/log/manticore/`。
* 默认 pid 目录现在是 `/var/run/manticore/`。

### Bugfixes
* [Commit a58c](https://github.com/manticoresoftware/manticore/commit/a58c619) 修复了破坏 Java 连接器的 SHOW COLLATION 语句
* [Commit 631c](https://github.com/manticoresoftware/manticore/commit/631cf4e) 修复了处理分布式索引时的崩溃；给分布式索引哈希添加了锁；从代理中移除移动和拷贝操作符
* [Commit 942b](https://github.com/manticoresoftware/manticore/commit/942bec0) 修复了因并行重新连接导致的分布式索引处理崩溃
* [Commit e5c1](https://github.com/manticoresoftware/manticore/commit/e5c1ed2) 修复了在存储查询至服务器日志崩溃处理时崩溃问题
* [Commit 4a4b](https://github.com/manticoresoftware/manticore/commit/4a4bda5) 修复了多查询中池属性的崩溃
* [Commit 3873](https://github.com/manticoresoftware/manticore/commit/3873bfb) 通过防止索引页包含在 core 文件中，减少了 core 文件大小
* [Commit 11e6](https://github.com/manticoresoftware/manticore/commit/11e6254) 修复了指定无效代理时搜索守护进程启动崩溃
* [Commit 4ca6](https://github.com/manticoresoftware/manticore/commit/4ca6350) 修复了索引器在 sql_query_killlist 查询中报错
* [Commit 123a](https://github.com/manticoresoftware/manticore/commit/123a9f0) 修复 fold_lemmas=1 与命中计数的不一致问题
* [Commit cb99](https://github.com/manticoresoftware/manticore/commit/cb99164) 修复了 html_strip 行为不一致
* [Commit e406](https://github.com/manticoresoftware/manticore/commit/e406761) 修复优化 rt 索引时丢失新设置；修复带 sync 选项的优化锁泄漏；
* [Commit 86ae](https://github.com/manticoresoftware/manticore/commit/86aeb82) 修复了错误多查询的处理
* [提交 7239](https://github.com/manticoresoftware/manticore/commit/72395d9) 修复了带有错误查询的多查询服务器崩溃问题
* [提交 f353](https://github.com/manticoresoftware/manticore/commit/f353326) 修复了共享锁到独占锁的转换问题
* [提交 3754](https://github.com/manticoresoftware/manticore/commit/3754785) 修复了没有索引的查询导致服务器崩溃的问题
* [提交 29f3](https://github.com/manticoresoftware/manticore/commit/29f360e) 修复了服务器死锁问题

## 版本 2.3.3，2017年7月6日
* Manticore 品牌命名
* [Commit 2645](https://github.com/manticoresoftware/manticore/commit/2645230) 修复结果集依赖多查询顺序的问题

