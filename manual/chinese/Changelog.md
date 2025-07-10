# 更新日志

## 开发版本

...

## 版本 13.2.3
**发布**：2025年7月8日

### 破坏性变更
* ⚠️ [v13.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.0.0) 更新了 KNN 库 API 以支持空的 [float_vector](https://manual.manticoresearch.com/Creating_a_table/Data_types#Float-vector) 值。此更新不改变数据格式，但增加了 Manticore Search / MCL API 版本。
* ⚠️ [v12.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/12.0.0) [ PR #3516](https://github.com/manticoresoftware/manticoresearch/pull/3516) 修正了 KNN 索引训练和构建过程中出现的错误的源和目标行ID问题。此更新不改变数据格式，但增加了 Manticore Search / MCL API 版本。
* ⚠️ [v11.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.0.0) 添加了对新的 KNN 向量搜索功能的支持，如量化、重评分和过采样。该版本改变了 KNN 数据格式和 [KNN_DIST() SQL 语法](https://manual.manticoresearch.com/Searching/KNN?client=SQL#KNN-vector-search)。新版本可以读取旧数据，但旧版本无法读取新格式。

### 新功能和改进
* 🆕 [v13.2.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.0) [ PR #3549](https://github.com/manticoresoftware/manticoresearch/issues/3525) 修复了 `@@collation_database` 导致某些 mysqldump 版本不兼容的问题
* 🆕 [v13.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.1.0) [ Issue #3489](https://github.com/manticoresoftware/manticoresearch/issues/3489) 修复了模糊搜索中阻止某些 SQL 查询解析的错误
* 🆕 [v12.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/12.1.0) [ Issue #3426](https://github.com/manticoresoftware/manticoresearch/issues/3426) 增加了对 RHEL 10 操作系统的支持
* 🆕 [v11.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.1.0) 添加了对[KNN 搜索](https://manual.manticoresearch.com/Searching/KNN#KNN-vector-search)中空浮点向量的支持
* 🆕 [v10.2.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.0) [ Issue #3252](https://github.com/manticoresoftware/manticoresearch/issues/3252) [log_level](https://manual.manticoresearch.com/Server_settings/Setting_variables_online#SET) 现在也调整 Buddy 的日志详细级别

###  Bug 修复
* 🪲 [v13.2.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.3) [ PR #3556](https://github.com/manticoresoftware/manticoresearch/pull/3556) 修复了 JSON 查询中 "oversampling" 选项解析问题
* 🪲 [v13.2.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.2) 通过移除 Boost stacktrace 的使用，修复了 Linux 和 FreeBSD 上的崩溃日志问题
* 🪲 [v13.2.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.1) [ Issue #3298](https://github.com/manticoresoftware/manticoresearch/issues/3298) 修复在容器内运行时的崩溃日志问题
* 🪲 [v12.0.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/12.0.2) 通过以微秒计数，提高了每表统计的精度
* 🪲 [v12.0.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/12.0.1) [ PR #3522](https://github.com/manticoresoftware/manticoresearch/pull/3522) 修复了在关联查询中按 MVA 分面时的崩溃
* 🪲 [v11.0.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.0.3) [ Issue #3502](https://github.com/manticoresoftware/manticoresearch/issues/3502) 修复了与向量搜索量化相关的崩溃
* 🪲 [v11.0.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.0.2) [ Issue #3493](https://github.com/manticoresoftware/manticoresearch/issues/3493) 将 `SHOW THREADS` 中的 CPU 利用率显示修改为整数
* 🪲 [v11.0.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.0.1) 修正了列存和二级库的路径
* 🪲 [v10.2.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.7) 支持 MCL 5.0.5，包括修复 embeddings 库文件名
* 🪲 [v10.2.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.6) [ Issue #3469](https://github.com/manticoresoftware/manticoresearch/issues/3469) 应用于问题 #3469 的另一个修复
* 🪲 [v10.2.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.4) [ Issue #3469](https://github.com/manticoresoftware/manticoresearch/issues/3469) 修复了使用 bool 查询的 HTTP 请求返回空结果的问题
* 🪲 [v10.2.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.5) 修改了 embeddings 库的默认文件名，并增加了对 KNN 向量中 'from' 字段的检查
* 🪲 [v10.2.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.3) [ PR #3464](https://github.com/manticoresoftware/manticoresearch/pull/3464) 更新 Buddy 到 3.30.2 版本，包含了关于内存使用和错误日志的 [PR #565](https://github.com/manticoresoftware/manticoresearch-buddy/pull/565)
* 🪲 [v10.2.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.2) [ Issue #3470](https://github.com/manticoresoftware/manticoresearch/pull/3470) 修复了 JOIN 查询中 JSON 字符串过滤器、空值过滤器和排序问题
* 🪲 [v10.2.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.1) 修复了 `dist/test_kit_docker_build.sh` 中导致 Buddy 提交缺失的错误
* 🪲 [v10.1.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.4) 修复了在关联查询中按 MVA 分组时的崩溃
* 🪲 [v10.1.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.3) [ Issue #3434](https://github.com/manticoresoftware/manticoresearch/issues/3434) 修复了过滤空字符串的错误
* 🪲 [v10.1.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.2) [ PR #3452](https://github.com/manticoresoftware/manticoresearch/pull/3452) 将 Buddy 更新到版本 3.29.7，解决了 [Buddy #507 - 多查询请求中模糊搜索错误](https://github.com/manticoresoftware/manticoresearch-buddy/issues/507) 和 [Buddy #561 - 修复指标速率](https://github.com/manticoresoftware/manticoresearch-buddy/pull/561)问题，此版本是 [Helm Release 10.1.0](https://github.com/manticoresoftware/manticoresearch-helm/releases/tag/manticoresearch-10.1.0) 所必需的
* 🪲 [v10.1.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.1)  将 Buddy 更新到版本 3.29.4，解决了 [#3388 - “未定义的数组键 'Field'”](https://github.com/manticoresoftware/manticoresearch/issues/3388) 和 [Buddy #547 - layouts='ru' 可能不起作用](https://github.com/manticoresoftware/manticoresearch-buddy/issues/547)

## 版本 10.1.0
**发布时间**：2025 年 6 月 9 日

此版本代表一次包含新特性、一个重大变更以及众多稳定性改进和漏洞修复的更新。改动主要集中在增强监控能力、改进搜索功能以及修复影响系统稳定性和性能的多个关键问题。

**从版本 10.1.0 开始，不再支持 CentOS 7。建议用户升级到受支持的操作系统。**

### 重大变更
* ⚠️ [v10.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.0.0) [ Issue #540](https://github.com/manticoresoftware/manticoresearch-buddy/issues/540) 重大变更：将 [模糊搜索](Searching/Spell_correction.md#Fuzzy-Search) 默认的 `layouts=''` 设置为空

### 新特性和改进
* 🆕 [v10.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.0) [ Issue #537](https://github.com/manticoresoftware/manticoresearch-buddy/issues/537) 添加内置的 [Prometheus 导出器](Node_info_and_management/Node_status.md#Prometheus-Exporter)
* 🆕 [v9.8.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.8.0) [ Issue #3409](https://github.com/manticoresoftware/manticoresearch/issues/3409) 添加 [ALTER TABLE tbl REBUILD KNN](Updating_table_schema_and_settings.md#Rebuilding-a-KNN-index) 功能
* 🆕 [v9.7.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.7.0) [ Issue #1778](https://github.com/manticoresoftware/manticoresearch/issues/1778) 添加自动生成嵌入功能（尚未正式发布，因为代码已在主分支，但仍需更多测试）
* 🆕 [v9.6.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.6.0)  KNN API 版本提升以支持自动嵌入
* 🆕 [v9.5.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.0) [ Issue #1894](https://github.com/manticoresoftware/manticoresearch/issues/1894) 改进集群恢复：周期性保存 `seqno`，加快崩溃后节点重启速度
* 🆕 [v9.4.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.4.0) [ Issue #2400](https://github.com/manticoresoftware/manticoresearch/issues/2400) 添加对最新 [Logstash](Integration/Logstash.md#Integration-with-Logstash) 和 Beats 的支持

### Bug 修复
* 🪲 [v10.0.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.0.1)  修正词形处理：用户定义词形覆盖自动生成词形；新增测试案例测试22
* 🪲 [v9.8.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.8.2)  修正：再次更新 deps.txt 以包含有关嵌入库的 MCL 打包修复
* 🪲 [v9.8.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.8.1)  修正：更新 deps.txt，修复 MCL 和嵌入库的打包问题
* 🪲 [v9.7.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.7.3) [ Issue #3306](https://github.com/manticoresoftware/manticoresearch/issues/3306) 修复索引过程中信号11导致的崩溃
* 🪲 [v9.7.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.7.2) [ Issue #3109](https://github.com/manticoresoftware/manticoresearch/issues/3109) 修正不存在的 `@@variables` 总是返回 0 的问题
* 🪲 [v9.7.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.7.1) [ Issue #3377](https://github.com/manticoresoftware/manticoresearch/issues/3377) 修复与 [remove_repeats()](Functions/Searching_and_ranking_functions.md#REMOVE_REPEATS%28%29) 相关的崩溃
* 🪲 [v9.6.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.6.3) [ PR #3411](https://github.com/manticoresoftware/manticoresearch/pull/3411) 修正：对遥测指标使用动态版本检测
* 🪲 [v9.6.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.6.2)  修正：[SHOW VERSION](Node_info_and_management/SHOW_VERSION.md#SHOW-VERSION) 输出中的小错误
* 🪲 [v9.6.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.6.1)  修复创建带有 KNN 属性但无模型表时崩溃的问题
* 🪲 [v9.5.16](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.16) [ Issue #3342](https://github.com/manticoresoftware/manticoresearch/issues/3342) 修复 `SELECT ... FUZZY=0` 不总是禁用模糊搜索的问题
* 🪲 [v9.5.15](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.15) [ PR #3397](https://github.com/manticoresoftware/manticoresearch/pull/3397) 添加对 [MCL](https://github.com/manticoresoftware/columnar) 4.2.2 的支持；修复旧存储格式相关错误
* 🪲 [v9.5.14](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.14) [ Issue #3392](https://github.com/manticoresoftware/manticoresearch/issues/3392) 修正 HTTP JSON 回复中字符串处理错误
* 🪲 [v9.5.13](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.13) [ Issue #3356](https://github.com/manticoresoftware/manticoresearch/issues/3356) 修复复杂全文查询情况下的崩溃（公共子项）
* 🪲 [v9.5.12](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.12)  修正了磁盘块自动刷新错误信息中的拼写错误
* 🪲 [v9.5.11](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.11) [ Issue #3195](https://github.com/manticoresoftware/manticoresearch/issues/3195) 改进了[自动磁盘块刷新](Securing_and_compacting_a_table/Flushing_RAM_chunk_to_disk.md#Flushing-RAM-chunk-to-disk)：优化运行时跳过保存
* 🪲 [v9.5.10](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.10) [ Issue #3313](https://github.com/manticoresoftware/manticoresearch/issues/3313) 修复了使用[indextool](Miscellaneous_tools.md#indextool)检查RT表中所有磁盘块重复ID的问题
* 🪲 [v9.5.9](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.9) [ Issue #3132](https://github.com/manticoresoftware/manticoresearch/issues/3132) 为JSON API添加了对`_random`排序的支持
* 🪲 [v9.5.8](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.8) [ Issue #3382](https://github.com/manticoresoftware/manticoresearch/issues/3382) 修复了无法通过JSON HTTP API使用uint64文档ID的问题
* 🪲 [v9.5.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.7) [ Issue #3385](https://github.com/manticoresoftware/manticoresearch/issues/3385) 修复了过滤条件为`id != value`时结果错误的问题
* 🪲 [v9.5.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.6) [ PR #538](https://github.com/manticoresoftware/manticoresearch-buddy/pull/538) 修复了部分情况下模糊匹配的严重错误
* 🪲 [v9.5.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.5) [ Issue #3199](https://github.com/manticoresoftware/manticoresearch/issues/3199) 修复了Buddy HTTP查询参数中空格解码问题（如`%20`和`+`）
* 🪲 [v9.5.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.4) [ Issue #3133](https://github.com/manticoresoftware/manticoresearch/issues/3133) 修复了facet搜索中`json.field`排序错误的问题
* 🪲 [v9.5.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.3) [ Issue #3091](https://github.com/manticoresoftware/manticoresearch/issues/3091) 修复了SQL与JSON API在定界符搜索中结果不一致的问题
* 🪲 [v9.5.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.2) [ Issue #2819](https://github.com/manticoresoftware/manticoresearch/issues/2819) 性能改进：将分布式表的`DELETE FROM`替换为`TRUNCATE`
* 🪲 [v9.5.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.1) [ Issue #3080](https://github.com/manticoresoftware/manticoresearch/issues/3080) 修复了使用别名过滤`geodist()`时与JSON属性关联的崩溃问题

## 版本 9.3.2
发布日期：2025年5月2日

本次发布包含多个错误修复和稳定性改进，更好的表使用跟踪，以及内存和资源管理的增强。

❤️ 特别感谢[@cho-m](https://github.com/cho-m)修复了与Boost 1.88.0的构建兼容性问题，以及[@benwills](https://github.com/benwills)对`stored_only_fields`文档的改进。

* 🪲 [v9.3.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.3.2)  修复了“显示线程”列中显示CPU活动为浮点数而非字符串的问题；同时修复了因错误数据类型引起的PyMySQL结果集解析错误。
* 🪲 [v9.3.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.3.1) [ Issue #3343](https://github.com/manticoresoftware/manticoresearch/issues/3343) 修复了优化过程被中断时遗留的`tmp.spidx`文件问题。
* 🆕 [v9.3.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.3.0) [ PR #3337](https://github.com/manticoresoftware/manticoresearch/pull/3337) 增加了每表命令计数器和详细的表使用统计。
* 🪲 [v9.2.39](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.39) [ Issue #3236](https://github.com/manticoresoftware/manticoresearch/issues/3236) 修复：通过移除复杂的块更新防止表损坏。在串行工作线程中使用等待函数破坏了串行处理，可能损坏表。
重新实现了自动刷新。移除了外部轮询队列以避免不必要的表锁。新增了“小表”条件：如果文档数低于“小表限制”（8192）且未使用二级索引（SI），则跳过刷新。

* 🪲 [v9.2.38](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.38)  修复：跳过为在字符串列表上使用`ALL`/`ANY`过滤器创建二级索引（SI），不影响JSON属性。
* 🪲 [v9.2.37](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.37) [ Issue #2898](https://github.com/manticoresoftware/manticoresearch/issues/2898) 为系统表增加了反引号支持。
* 🪲 [v9.2.36](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.36)  修复：在遗留代码中为集群操作使用占位符。在解析器中，我们现在清晰分离了表名和集群名的字段。
* 🪲 [v9.2.35](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.35)  修复：反引号未配对单个`'`时导致的崩溃。
* 🪲 [v9.2.34](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.34) [ Issue #3090](https://github.com/manticoresoftware/manticoresearch/issues/3090) 修复：处理大文档ID时之前可能失败的问题。
* 🪲 [v9.2.33](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.33)  修复：位向量大小使用无符号整型。
* 🪲 [v9.2.32](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.32)  修复：减少合并时的峰值内存使用。docid转rowid的查找现在每个文档使用12字节代替16字节。示例：20亿文档使用24GB内存，而非36GB。
* 🪲 [v9.2.31](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.31) [ Issue #3238](https://github.com/manticoresoftware/manticoresearch/issues/3238) 修复：大型实时表中 `COUNT(*)` 值不正确。
* 🪲 [v9.2.30](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.30) 修复：清零字符串属性时的未定义行为。
* 🪲 [v9.2.29](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.29) 小修正：改进了警告文本。
* 🪲 [v9.2.28](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.28) [ Issue #3290](https://github.com/manticoresoftware/manticoresearch/issues/3290) 改进：增强了 `indextool --buildidf`
* 🪲 [v9.2.27](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.27) [ Issue #3032](https://github.com/manticoresoftware/manticoresearch/issues/3032) 通过 Kafka 集成，现在可以为特定的 Kafka 分区创建数据源。
* 🪲 [v9.2.26](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.26) [ Issue #3301](https://github.com/manticoresoftware/manticoresearch/issues/3301) 修复：对 `id` 使用 `ORDER BY` 和 `WHERE` 可能导致内存溢出（OOM）错误。
* 🪲 [v9.2.25](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.25) [ Issue #3171](https://github.com/manticoresoftware/manticoresearch/issues/3171) 修复：当 RT 表含有多个磁盘块且使用多个 JSON 属性的 grouper 时，导致段错误的崩溃。
* 🪲 [v9.2.24](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.24) [ Issue #3246](https://github.com/manticoresoftware/manticoresearch/issues/3246) 修复：`WHERE string ANY(...)` 查询在 RAM 数据块刷新后失败。
* 🪲 [v9.2.23](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.23) [ PR #518](https://github.com/manticoresoftware/manticoresearch-buddy/pull/518) 小幅自动分片语法改进。
* 🪲 [v9.2.22](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.22) [ Issue #2763](https://github.com/manticoresoftware/manticoresearch/issues/2763) 修复：使用 `ALTER TABLE` 时全局 idf 文件未被加载。
* 🪲 [v9.2.21](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.21) 修复：全局 idf 文件可能很大。现在更早释放表以避免持有不必要的资源。
* 🪲 [v9.2.20](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.20) [ PR #3277](https://github.com/manticoresoftware/manticoresearch/pull/3277) 改进：更好的分片选项验证。

* 🪲 [v9.2.19](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.19) [ PR #3275](https://github.com/manticoresoftware/manticoresearch/pull/3275) 修复：与 Boost 1.88.0 的构建兼容性。
* 🪲 [v9.2.18](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.18) [ Issue #3228](https://github.com/manticoresoftware/manticoresearch/issues/3228) 修复：分布式表创建时崩溃（无效指针问题）。

* 🪲 [v9.2.17](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.17) [ PR #3272](https://github.com/manticoresoftware/manticoresearch/pull/3272) 修复：多行模糊 `FACET` 问题。
* 🪲 [v9.2.16](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.16) [ Issue #3063](https://github.com/manticoresoftware/manticoresearch/issues/3063) 修复：使用 `GEODIST` 函数时距离计算错误的 bug。
* 🪲 [v9.2.15](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.15) [ Issue #3027](https://github.com/manticoresoftware/manticoresearch/issues/3027) 小幅改进：支持 Elastic 的 `query_string` 过滤格式。

## Version 9.2.14
Released: March 28th 2025

### Minor changes
* [Commit ](https://github.com/manticoresoftware/manticoresearch/commit/fe9473dc3f4fcfa0eaae748b538fe853f4ce078b) 实现了 `--mockstack` 标志，用于计算递归操作堆栈需求。新的 `--mockstack` 模式分析并报告递归表达式求值、模式匹配操作、过滤处理所需的堆栈大小。计算出的堆栈需求将输出到控制台，供调试和优化使用。
* [Issue #3058](https://github.com/manticoresoftware/manticoresearch/pull/3058) 默认启用了 [boolean_simplify](Searching/Options.md#boolean_simplify)。
* [Issue #3172](https://github.com/manticoresoftware/manticoresearch/issues/3172) 添加了一个新的配置选项：`searchd.kibana_version_string`，当在特定版本的 Kibana 或 OpenSearch Dashboards 下使用 Manticore 并期望特定的 Elasticsearch 版本时非常有用。
* [Issue #3211](https://github.com/manticoresoftware/manticoresearch/issues/3211) 修复了 [CALL SUGGEST](Searching/Spell_correction.md#CALL-QSUGGEST,-CALL-SUGGEST) 以支持两个字符的单词。
* [Issue #490](https://github.com/manticoresoftware/manticoresearch-buddy/issues/490) 改进了 [模糊搜索](Searching/Spell_correction.md#Fuzzy-Search)：之前在搜索 "def ghi" 时，如果存在另一个匹配的文档，有时无法找到 "defghi"。
* ⚠️ BREAKING [Issue #3165](https://github.com/manticoresoftware/manticoresearch/issues/3165) 在部分 HTTP JSON 响应中将 `_id` 改为 `id` 以保证一致性。请确保相应更新您的应用。
* ⚠️ 重大变更 [Issue #3186](https://github.com/manticoresoftware/manticoresearch/issues/3186) 在集群加入期间增加了对 `server_id` 的检查，以确保每个节点具有唯一ID。现在，当加入节点的 `server_id` 与集群中已有节点相同时，`JOIN CLUSTER` 操作可能会失败，并显示重复 [server_id](Server_settings/Searchd.md#server_id) 的错误消息。为了解决此问题，请确保复制集群中的每个节点具有唯一的 [server_id](Server_settings/Searchd.md#server_id)。您可以在尝试加入集群之前，在配置文件的 "searchd" 部分将默认的 [server_id](Server_settings/Searchd.md#server_id) 修改为唯一值。此更改更新了复制协议。如果您运行的是复制集群，您需要：
  - 首先，干净地停止所有节点
  - 然后，使用 Linux 下的工具 `manticore_new_cluster`，通过 `--new-cluster` 参数启动最后停止的节点。
  - 详细内容请参阅关于[重启集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster)的说明。

### Bug修复
* [Commit 6fda](https://github.com/manticoresoftware/manticoresearch/commit/6fdad3923dd6953f4b781943eed5ec5c28b7f808) 修复了等待后调度器丢失导致崩溃的问题；现在，特定的调度器如 `serializer` 会被正确恢复。
* [Commit c333](https://github.com/manticoresoftware/manticoresearch/commit/c333b277d4a504de5fe298e9ab570a9f9ea31e0a) 修复了无法在 `ORDER BY` 子句中使用右连接表权重的bug。
* [Issue #2644](https://github.com/manticoresoftware/manticoresearch/issues/2644) gcc 14.2.0：修复了 `lower_bound` 调用时 `const knn::DocDist_t*&` 的错误。❤️ 感谢 [@Azq2](https://github.com/Azq2) 的 PR。
* [Issue #3018](https://github.com/manticoresoftware/manticoresearch/issues/3018) 修复了自动模式插入时处理大写表名的问题。
* [Issue #3119](https://github.com/manticoresoftware/manticoresearch/issues/3119) 修复了解码无效 base64 输入时的崩溃。
* [Issue #3121](https://github.com/manticoresoftware/manticoresearch/issues/3121) 修复了两处 ALTER 操作相关的KNN索引问题：浮点向量现在保持其原始维度，KNN索引现在正确生成。
* [Issue #3123](https://github.com/manticoresoftware/manticoresearch/issues/3123) 修复了在空 JSON 列上构建二级索引时的崩溃。
* [Issue #3138](https://github.com/manticoresoftware/manticoresearch/issues/3138) 修复了因重复条目导致的崩溃。
* [Issue #3151](https://github.com/manticoresoftware/manticoresearch/issues/3151) 修复了 `fuzzy=1` 选项无法与 `ranker` 或 `field_weights` 共同使用的问题。
* [Issue #3163](https://github.com/manticoresoftware/manticoresearch/issues/3163) 修复了 `SET GLOBAL timezone` 无效的bug。
* [Issue #3181](https://github.com/manticoresoftware/manticoresearch/issues/3181) 修复了使用超过 2^63 的 ID 时，文本字段值可能丢失的问题。
* [Issue #3189](https://github.com/manticoresoftware/manticoresearch/issues/3189) 修复了 `UPDATE` 语句未正确遵守 `query_log_min_msec` 设置的问题。
* [Issue #3247](https://github.com/manticoresoftware/manticoresearch/issues/3247) 修复了保存实时磁盘块时出现的竞态条件，可能导致 `JOIN CLUSTER` 失败。


## 版本 7.4.6
发布时间：2025年2月28日

### 重大变更
* [Issue #832](https://github.com/manticoresoftware/manticoresearch/issues/832) 集成了 [Kibana](Integration/Kibana.md)，以实现更便捷高效的数据可视化。

### 次要变更
* [Issue #1727](https://github.com/manticoresoftware/manticoresearch/issues/1727) 修正了 arm64 和 x86_64 之间的浮点数精度差异。
* [Issue #2995](https://github.com/manticoresoftware/manticoresearch/issues/2995) 针对连接批处理实现了性能优化。
* [Issue #3039](https://github.com/manticoresoftware/manticoresearch/issues/3039) 针对直方图的 EstimateValues 实现了性能优化。
* [Issue #3099](https://github.com/manticoresoftware/manticoresearch/issues/3099) 增加了对 Boost 1.87.0 的支持。❤️ 感谢 [@cho-m](https://github.com/cho-m) 的 PR。
* [Issue #77](https://github.com/manticoresoftware/columnar/issues/77) 优化了创建多值过滤器时的块数据复用；添加了属性元数据中的最小值/最大值；实现了基于最小值/最大值的过滤器值预过滤。

### Bug修复
* [Commit 73ac](https://github.com/manticoresoftware/manticoresearch/commit/73ac22f358a7a0e734b332c3943c86e6294c06c3) 修复了连接查询中表达式处理的问题（涉及左右表同时使用属性）；修正了右表的 index_weights 选项。
* [Issue #2915](https://github.com/manticoresoftware/manticoresearch/issues/2915) 在 `SELECT ... JOIN` 查询中使用 `avg()` 可能导致结果错误，该问题已修复。
* [Issue #2996](https://github.com/manticoresoftware/manticoresearch/issues/2996) 修复了启用连接批处理时隐式截止线造成的错误结果集。
* [Issue #3031](https://github.com/manticoresoftware/manticoresearch/issues/3031) 修复了守护进程在关闭时进行活动块合并导致的崩溃。
* [Issue #3037](http://github.com/manticoresoftware/manticoresearch/issues/3037) 修复了 `IN(...)` 可能产生错误结果的问题。
* [Issue #3038](https://github.com/manticoresoftware/manticoresearch/issues/3038) 7.0.0 版本中设置 `max_iops` / `max_iosize` 导致索引性能下降的问题已修复。
* [Issue #3042](https://github.com/manticoresoftware/manticoresearch/issues/3042) 修复了连接查询缓存中的内存泄漏。
* [Issue #3052](https://github.com/manticoresoftware/manticoresearch/issues/3052) 修复了连接 JSON 查询中查询选项的处理。
* [Issue #3054](https://github.com/manticoresoftware/manticoresearch/issues/3054) 修复了 ATTACH TABLE 命令的问题。
* [Issue #3079](https://github.com/manticoresoftware/manticoresearch/issues/3079) 修正了错误消息中的不一致问题。
* [Issue #3087](https://github.com/manticoresoftware/manticoresearch/issues/3087) 每个表设置 `diskchunk_flush_write_timeout=-1` 未禁用索引刷新的问题已修复。
* [Issue #3088](https://github.com/manticoresoftware/manticoresearch/issues/3088) 解决了批量替换大ID后出现的重复条目问题。
* [Issue #3126](https://github.com/manticoresoftware/manticoresearch/issues/3126) 修复了由仅含单个 `NOT` 操作符及表达式排序器的全文查询导致的守护进程崩溃问题。
* [Issue #3128](https://github.com/manticoresoftware/manticoresearch/pull/3128) 修复了 CJSON 库中潜在的漏洞。❤️ 感谢 [@tabudz](https://github.com/tabudz) 的 PR。

## 版本 7.0.0
发布：2025年1月30日

### 重大变化
* [Issue #1497](https://github.com/manticoresoftware/manticoresearch/issues/1497) 新增 [模糊搜索](Searching/Spell_correction.md#Fuzzy-Search) 和 [自动补全](Searching/Autocomplete.md#CALL-AUTOCOMPLETE) 功能，简化搜索流程。
* [Issue #1500](https://github.com/manticoresoftware/manticoresearch/issues/1500) [Kafka 集成](Integration/Kafka.md#Syncing-from-Kafka)。
* [Issue #1928](https://github.com/manticoresoftware/manticoresearch/issues/1928) 引入了 JSON 的[二级索引](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#json_secondary_indexes)。
* [Issue #2361](https://github.com/manticoresoftware/manticoresearch/issues/2361) 更新操作及更新时的搜索不再被块合并阻塞。
* [Issue #2787](https://github.com/manticoresoftware/manticoresearch/issues/2787) RT表的自动[磁盘块刷新](Server_settings/Searchd.md#diskchunk_flush_write_timeout)以提升性能；现在自动将内存块刷新到磁盘块，避免由内存块缺少优化引起的性能问题，这有时会根据块大小导致不稳定。
* [Issue #2811](https://github.com/manticoresoftware/manticoresearch/issues/2811) [Scroll](Searching/Pagination.md#Scroll-Search-Option) 选项，简化分页操作。
* [Issue #931](https://github.com/manticoresoftware/manticoresearch/issues/931) 集成了 [结巴](https://github.com/fxsjy/jieba) 以改进[中文分词](Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md)。

### 小幅变化
* ⚠️ 重大变更 [Issue #1111](https://github.com/manticoresoftware/manticoresearch/issues/1111) 修复 RT 表中对 `global_idf` 的支持。需要重新创建表。
* ⚠️ 重大变更 [Issue #2103](https://github.com/manticoresoftware/manticoresearch/issues/2103) 移除了内部 `cjk` 字符集中的泰文字符。请相应更新您的字符集定义：如果定义为 `cjk,non_cjk` 且泰文字符对您重要，请改为 `cjk,thai,non_cjk`，或使用新的连续脚本语言标识 `cont,non_cjk`，其中 `cont` 是指所有连续脚本语言（即 `cjk` + `thai`）。请使用 [ALTER TABLE](Updating_table_schema_and_settings.md#Updating-table-FT-settings-in-RT-mode) 修改现有表。
* ⚠️ 重大变更 [Issue #2468](https://github.com/manticoresoftware/manticoresearch/issues/2468) [CALL SUGGEST / QSUGGEST](Searching/Spell_correction.md#CALL-QSUGGEST,-CALL-SUGGEST) 现支持分布式表。此更改提高了主/代理协议版本。如果您在分布式环境中运行 Manticore Search 多实例，请先升级代理，再升级主节点。
* ⚠️ 重大变更 [Issue #2889](https://github.com/manticoresoftware/manticoresearch/issues/2889) PQ [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META-for-PQ-tables) 的列名由 `Name` 改为 `Variable name`。
* ⚠️ 重大变更 [Issue #879](https://github.com/manticoresoftware/manticoresearch/issues/879) 引入了[按表二进制日志](Logging/Binary_logging.md#Per-table-binary-logging-configuration)的新选项：[binlog_common](Logging/Binary_logging.md#Binary-logging-strategies)、[binlog](Logging/Binary_logging.md#Per-table-binary-logging-configuration) 用于 `create table` / `alter table`。升级前需干净关闭 Manticore 实例。
* ⚠️ 重大变更 [Issue #1789](https://github.com/manticoresoftware/manticoresearch/issues/1789) 修正了节点以错误复制协议版本加入集群时的错误提示信息。此变更升级了复制协议。对运行复制集群的用户：
  - 先依次干净停止所有节点
  - 然后最后停止的节点用 `--new-cluster` 参数启动，Linux下可用工具 `manticore_new_cluster`。
  - 更多详情请阅读[重启集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster)。
* ⚠️ 重大变更 [Issue #2308](https://github.com/manticoresoftware/manticoresearch/issues/2308) 新增支持 [`ALTER CLUSTER ADD` 和 `DROP`](Creating_a_cluster/Setting_up_replication/Adding_and_removing_a_table_from_a_replication_cluster.md) 多表操作。此更改也影响复制协议。请参考上节指导进行处理。
* [Issue #2997](https://github.com/manticoresoftware/manticoresearch/issues/2997) 修复了 Macos 下 dlopen 问题。
* [Commit 4954](https://github.com/manticoresoftware/manticoresearch/commit/4954b5de7341a29902a9b8fbb9a040f7942c77c4) 修改了带KNN索引表的 OPTIMIZE TABLE 默认阈值，提升搜索性能。
* [Commit cfc8](https://github.com/manticoresoftware/manticoresearch/commit/cfc87ecb6e33a8163c2235243b6b40e699dbf526) 为 `FACET` 和 `GROUP BY` 的 `ORDER BY` 添加了 `COUNT(DISTINCT)` 支持。
* [Issue #1103](https://github.com/manticoresoftware/manticoresearch/issues/1103) 改进了[日志](Logging/Server_logging.md#Server-logging)中块合并的清晰度。
* [Issue #1130](https://github.com/manticoresoftware/manticoresearch/issues/1130) 添加了对 [DBeaver](Integration/DBeaver.md) 的支持。
* [Issue #1546](https://github.com/manticoresoftware/manticoresearch/issues/1546) 实现了对 [POLY2D()](Functions/Geo_spatial_functions.md#POLY2D%28%29)/[GEOPOLY2D()](Functions/Geo_spatial_functions.md#GEOPOLY2D%28%29) 函数的二级索引支持。
* [Issue #1630](https://github.com/manticoresoftware/manticoresearch/issues/1630) HTTP 请求现在支持 `Content-Encoding: gzip`。
* [Issue #1831](https://github.com/manticoresoftware/manticoresearch/issues/1831) 添加了 `SHOW LOCKS` 命令。
* [Issue #2187](https://github.com/manticoresoftware/manticoresearch/issues/2197) 允许 Buddy 请求绕过守护进程中的 [searchd.max_connections](Server_settings/Searchd.md#max_connections) 限制。
* [Issue #2208](https://github.com/manticoresoftware/manticoresearch/issues/2208) 增加了通过 JSON HTTP 接口连接表的支持。
* [Issue #2235](https://github.com/manticoresoftware/manticoresearch/issues/2235) 通过 Buddy 以原始形式记录成功处理的查询。
* [Issue #2249](https://github.com/manticoresoftware/manticoresearch/issues/2249) 为复制表添加了运行 `mysqldump` 的特殊模式。
* [Issue #2268](https://github.com/manticoresoftware/manticoresearch/issues/2268) 改进了 `CREATE TABLE` 和 `ALTER TABLE` 语句中外部文件复制时的重命名功能。
* [Issue #2402](https://github.com/manticoresoftware/manticoresearch/issues/2402) 将 [searchd.max_packet_size](Server_settings/Searchd.md#max_packet_size) 的默认值更新为 128MB。
* [Issue #2419](https://github.com/manticoresoftware/manticoresearch/issues/2419) 在 JSON ["match"](Searching/Full_text_matching/Basic_usage.md#match) 中新增了对 [IDF boost modifier](Searching/Full_text_matching/Operators.md#IDF-boost-modifier) 的支持。
* [Issue #2430](https://github.com/manticoresoftware/manticoresearch/issues/2430) 增强了 [binlog](Logging/Binary_logging.md#Binary-logging) 写入同步以防止错误。
* [Issue #2458](https://github.com/manticoresoftware/manticoresearch/issues/2458) 在 Windows 包中集成了 zlib 支持。
* [Issue #2479](https://github.com/manticoresoftware/manticoresearch/issues/2479) 添加了对 SHOW TABLE INDEXES 命令的支持。
* [Issue #2485](https://github.com/manticoresoftware/manticoresearch/issues/2485) 设置了 Buddy 回复的会话元数据。
* [Issue #2490](https://github.com/manticoresoftware/manticoresearch/issues/2490) 兼容端点的聚合支持毫秒分辨率。
* [Issue #2500](https://github.com/manticoresoftware/manticoresearch/issues/2500) 当复制启动失败时，修改了集群操作的错误消息。
* [Issue #2584](https://github.com/manticoresoftware/manticoresearch/pull/2584) 在 SHOW STATUS 中新增了性能指标（Node_info_and_management/Node_status.md#Query-Time-Statistics）：最近 1、5 和 15 分钟内针对每种查询类型的最小/最大/平均/第 95/99 百分位数。
* [Issue #2639](https://github.com/manticoresoftware/manticoresearch/pull/2639) 将请求和响应中所有的 `index` 替换为 `table`。
* [Issue #2643](https://github.com/manticoresoftware/manticoresearch/issues/2643) 在 HTTP `/sql` 端点聚合结果中添加了 `distinct` 列。
* [Issue #268](https://github.com/manticoresoftware/manticoresearch-buddy/issues/268) 实现了从 Elasticsearch 导入数据类型的自动检测。
* [Issue #2744](https://github.com/manticoresoftware/manticoresearch/issues/2744) 为字符串 JSON 字段比较表达式添加了排序规则支持。
* [Issue #2752](https://github.com/manticoresoftware/manticoresearch/issues/2752) 在选择列表中添加了对 `uuid_short` 表达式的支持。
* [Issue #2783](https://github.com/manticoresoftware/manticoresearch/issues/2783) Manticore Search 现在直接运行 Buddy，无需 `manticore-buddy` 包装器。
* [Issue #2785](https://github.com/manticoresoftware/manticoresearch/issues/2785) 区分了缺失表和不支持插入操作的表的错误消息。
* [Issue #2789](https://github.com/manticoresoftware/manticoresearch/issues/2789) OpenSSL 3 已静态构建进 `searchd`。
* [Issue #2790](https://github.com/manticoresoftware/manticoresearch/issues/2790) 添加了 `CALL uuid_short` 语句，用于生成包含多个 `uuid_short` 值的序列。
* [Issue #2803](https://github.com/manticoresoftware/manticoresearch/issues/2803) 为 JOIN 操作的右表增加了单独的选项。
* [Issue #2810](https://github.com/manticoresoftware/manticoresearch/issues/2810) 改进了 HTTP JSON 聚合性能，使其与 SphinxQL 中的 `GROUP BY` 相匹配。
* [Issue #2854](https://github.com/manticoresoftware/manticoresearch/issues/2854) 增加了对 Kibana 日期相关请求中 `fixed_interval` 的支持。
* [Issue #2909](https://github.com/manticoresoftware/manticoresearch/issues/2909) 为 JOIN 查询实现了批处理，极大提升了某些 JOIN 查询的性能，提升幅度达数百倍甚至数千倍。
* [Issue #2937](https://github.com/manticoresoftware/manticoresearch/issues/2937) 允许全扫描查询中使用连接表的权重。
* [Issue #2953](https://github.com/manticoresoftware/manticoresearch/issues/2953) 修正了连接查询的日志记录。
* [Issue #337](https://github.com/manticoresoftware/manticoresearch-buddy/issues/337) 在非调试模式下隐藏了 `searchd` 日志中的 Buddy 异常。
* [Issue #2931](https://github.com/manticoresoftware/manticoresearch/issues/2931) 当用户为复制监听器设置错误端口时，守护进程在关闭时会显示错误消息。

### Bug 修复
* [Commit 0c6b](https://github.com/manticoresoftware/manticoresearch/commit/0c6bdaf41ea3684e05952e1a8273893535b923f5) 修复：当 JOIN 查询中列超过 32 列时返回结果错误。
* [Issue #2335](https://github.com/manticoresoftware/manticoresearch/issues/2335) 解决了当条件中使用两个 json 属性时表连接失败的问题。
* [Issue #2338](https://github.com/manticoresoftware/manticoresearch/issues/2338) 修正了带有 [cutoff](Searching/Options.md#cutoff) 的多查询中的不正确 total_relation。
* [Issue #2366](https://github.com/manticoresoftware/manticoresearch/issues/2366) 纠正了 [表连接](Searching/Joining.md) 中对右表的 `json.string` 的过滤。
* [Issue #2406](https://github.com/manticoresoftware/manticoresearch/issues/2406) 允许在任何 POST HTTP JSON 端点（insert/replace/bulk）中对所有值使用 `null`，此情况下将使用默认值。
* [Issue #2418](https://github.com/manticoresoftware/manticoresearch/issues/2418) 通过调整初始套接字探测的 [max_packet_size](Server_settings/Searchd.md#max_packet_size) 网络缓冲区的分配，优化了内存消耗。
* [Issue #2420](https://github.com/manticoresoftware/manticoresearch/issues/2420) 修复了通过 JSON 接口插入无符号整型到 bigint 属性的问题。
* [Issue #2422](https://github.com/manticoresoftware/manticoresearch/issues/2422) 修正了启用 pseudo_sharding 时二级索引与排除过滤器的兼容问题。
* [Issue #2423](https://github.com/manticoresoftware/manticoresearch/issues/2423) 解决了 [manticore_new_cluster](Starting_the_server/Manually.md#searchd-command-line-options) 中的一个错误。
* [Issue #2448](https://github.com/manticoresoftware/manticoresearch/issues/2448) 解决了在格式错误的 `_update` 请求导致的守护进程崩溃问题。
* [Issue #2452](https://github.com/manticoresoftware/manticoresearch/issues/2452) 修正了直方图不能处理带排除的值过滤的问题。
* [Issue #55](https://github.com/manticoresoftware/columnar/issues/55) 修正了针对分布式表的 [knn](Searching/KNN.md#KNN-vector-search) 查询。
* [Issue #68](https://github.com/manticoresoftware/columnar/issues/68) 增强了列存访问器中对表编码排除过滤的处理。
* [Commit 0eb1](https://github.com/manticoresoftware/manticoresearch/commit/0eb1579462013cb134bcaf74e06f6eeb0c2ecc4d) 修复了表达式解析器未遵守重新定义的 `thread_stack` 的问题。
* [Commit c304](https://github.com/manticoresoftware/manticoresearch/commit/c3041fd1d3e14773f3c5aa635343915a71f5cb95) 修复了克隆列存 IN 表达式时崩溃的问题。
* [Commit edad](https://github.com/manticoresoftware/columnar/commit/edadc694c68d6022bdd13134263667430a42cc1d) 修复了位图迭代器中的取反问题，该问题导致崩溃。
* [Commit fc30](https://github.com/manticoresoftware/executor/commit/fc302ff1117b0b835a6f5a3c07057baf5fba14d9) 修正了一些 Manticore 软件包被 `unattended-upgrades` 自动移除的问题。
* [Issue #1019](https://github.com/manticoresoftware/manticoresearch/issues/1019) 改进了对 DbForge MySQL 工具查询的处理。
* [Issue #1107](https://github.com/manticoresoftware/manticoresearch/issues/1107) 修正了 `CREATE TABLE` 和 `ALTER TABLE` 中特殊字符的转义。❤️ 感谢 [@subnix](https://github.com/subnix) 提交的 PR。
* [Issue #116](https://github.com/manticoresoftware/manticoresearch-backup/issues/116#issuecomment-2216597206) 修复了在冻结索引中更新 blob 属性时的死锁。由于在尝试解冻索引时发生锁冲突，可能导致 manticore-backup 失败。
* [Issue #1818](https://github.com/manticoresoftware/manticoresearch/issues/1818) `OPTIMIZE` 在表被冻结时现在会抛出错误。
* [Issue #2001](https://github.com/manticoresoftware/manticoresearch/issues/2001) 允许函数名用作列名。
* [Issue #2153](https://github.com/manticoresoftware/manticoresearch/issues/2153) 修复了使用未知磁盘块查询表设置时守护进程崩溃的问题。
* [Issue #2184](https://github.com/manticoresoftware/manticoresearch/issues/2184) 修正了在执行 `FREEZE` 和 `FLUSH RAMCHUNK` 后 `searchd` 停止时挂起的问题。
* [Issue #2228](https://github.com/manticoresoftware/manticoresearch/issues/2228) 从保留字中移除了日期/时间相关的标记（及正则表达式）。
* [Issue #2255](https://github.com/manticoresoftware/manticoresearch/issues/2255) 修复了 `FACET` 在超过 5 个排序字段时崩溃的问题。
* [Issue #2265](https://github.com/manticoresoftware/manticoresearch/issues/2265) 解决了启用 `index_field_lengths` 时恢复 `mysqldump` 失败的问题。
* [Issue #2291](https://github.com/manticoresoftware/manticoresearch/issues/2291) 修复了执行 `ALTER TABLE` 命令时崩溃的问题。
* [Issue #2333](https://github.com/manticoresoftware/manticoresearch/issues/2333) 修正了 Windows 包中用于索引器的 MySQL DLL 以确保其正常工作。
* [Issue #2393](https://github.com/manticoresoftware/manticoresearch/issues/2393) 修复了 GCC 编译错误。❤️ 感谢 [@animetosho](https://github.com/animetosho) 提交的 PR。
* [Issue #2447](https://github.com/manticoresoftware/manticoresearch/issues/2447) 修正了 [_update](Data_creation_and_modification/Updating_documents/REPLACE.md?client=Elasticsearch-like+partial) 转义问题。
* [Issue #2460](https://github.com/manticoresoftware/manticoresearch/issues/2460) 修复了在声明多个同名属性或字段时索引器崩溃的问题。
* [Issue #2467](https://github.com/manticoresoftware/manticoresearch/issues/2467) 解决了“兼容”搜索相关端点中嵌套 bool 查询错误转换导致守护进程崩溃的问题。
* [Issue #2493](https://github.com/manticoresoftware/manticoresearch/issues/2493) 修正了带修饰符短语中的扩展问题。
* [Issue #2535](https://github.com/manticoresoftware/manticoresearch/issues/2535) 解决了使用 [ZONE](Searching/Full_text_matching/Operators.md#ZONE-limit-operator) 或 [ZONESPAN](Searching/Full_text_matching/Operators.md#ZONESPAN-limit-operator) 全文操作符时守护进程崩溃的问题。
* [Issue #2552](https://github.com/manticoresoftware/manticoresearch/issues/2552) 修正了带关键字字典的普通表和 RT 表的中缀生成。
* [Issue #2571](https://github.com/manticoresoftware/manticoresearch/issues/2571) 修正了 `FACET` 查询中的错误回复；为包含 `COUNT(*)` 的 `FACET` 设置默认排序顺序为 `DESC`。
* [Issue #2580](https://github.com/manticoresoftware/manticoresearch/issues/2580) 修正了启动时 Windows 平台的守护进程崩溃问题。
* [Issue #2603](https://github.com/manticoresoftware/manticoresearch/issues/2603) 修正了 HTTP 端点 `/sql` 和 `/sql?mode=raw` 的查询截断问题；使这些端点的请求保持一致，无需 `query=` 头。
* [Issue #2623](https://github.com/manticoresoftware/manticoresearch/issues/2623) 修正了自动模式创建表但同时失败的问题。
* [Issue #2627](https://github.com/manticoresoftware/manticoresearch/issues/2627) 修正了 HNSW 库以支持加载多个 KNN 索引。
* [Issue #2630](https://github.com/manticoresoftware/manticoresearch/issues/2630) 修正了多重条件同时发生时导致冻结的问题。
* [Issue #2645](https://github.com/manticoresoftware/manticoresearch/issues/2645) 修正了使用 `OR` 结合 KNN 搜索时导致的致命错误崩溃。
* [Issue #2647](https://github.com/manticoresoftware/manticoresearch/issues/2647) 修正了 `indextool --mergeidf *.idf --out global.idf` 命令创建后删除输出文件的问题。
* [Issue #2658](https://github.com/manticoresoftware/manticoresearch/issues/2658) 修正了带 `ORDER BY` 字符串的子查询在外部查询中导致守护进程崩溃的问题。
* [Issue #2686](https://github.com/manticoresoftware/manticoresearch/issues/2686) 修正了同时更新浮点属性和字符串属性时导致崩溃的问题。
* [Issue #2704](https://github.com/manticoresoftware/manticoresearch/issues/2704) 修正了 `lemmatize_xxx_all` 分词器中多个停用词增加后续标记 `hitpos` 的问题。
* [Issue #2708](https://github.com/manticoresoftware/manticoresearch/issues/2708) 修正了执行 `ALTER ... ADD COLUMN ... TEXT` 时导致崩溃的问题。
* [Issue #2737](https://github.com/manticoresoftware/manticoresearch/issues/2737) 修正了在包含至少一个 RAM 块的冻结表中更新 blob 属性时导致后续 `SELECT` 查询等待表解冻的问题。
* [Issue #2742](https://github.com/manticoresoftware/manticoresearch/issues/2742) 修正了包含打包因子的查询被跳过查询缓存的问题。
* [Issue #2775](https://github.com/manticoresoftware/manticoresearch/issues/2775) Manticore 现对未知操作报告错误，而非在 `_bulk` 请求时崩溃。
* [Issue #2791](https://github.com/manticoresoftware/manticoresearch/issues/2791) 修正了 HTTP `_bulk` 端点插入文档 ID 的返回问题。
* [Issue #2797](https://github.com/manticoresoftware/manticoresearch/issues/2797) 修正了处理多表且其中一表为空，另一表匹配条目数不同的 grouper 崩溃问题。
* [Issue #2835](https://github.com/manticoresoftware/manticoresearch/issues/2835) 修正了复杂 `SELECT` 查询中的崩溃。
* [Issue #2872](https://github.com/manticoresoftware/manticoresearch/issues/2872) 新增错误消息，当 `IN` 表达式中的 `ALL` 或 `ANY` 参数不是 JSON 属性时提示。
* [Issue #2882](https://github.com/manticoresoftware/manticoresearch/issues/2882) 修正了在大型表更新 MVA 时导致守护进程崩溃的问题。
* [Issue #2888](https://github.com/manticoresoftware/manticoresearch/issues/2888) 修正了 `libstemmer` 分词失败时的崩溃。❤️ 感谢 [@subnix](https://github.com/subnix) 的 PR。
* [Issue #2919](https://github.com/manticoresoftware/manticoresearch/issues/2919) 修正了右表连接权重在表达式中无法正确工作的错误。
* [Issue #2919](https://github.com/manticoresoftware/manticoresearch/issues/2919) 修正了右连接表权重在表达式中不生效的问题。
* [Issue #325](https://github.com/manticoresoftware/manticoresearch-buddy/issues/325) 修正了表已存在时 `CREATE TABLE IF NOT EXISTS ... WITH DATA` 失败问题。
* [Issue #351](https://github.com/manticoresoftware/manticoresearch-buddy/issues/351) 修正了使用文档 ID 按 KNN 计数时的未定义数组键 "id" 错误。
* [Issue #359](https://github.com/manticoresoftware/manticoresearch-buddy/issues/359) 修正了 `REPLACE INTO cluster_name:table_name` 功能。
* [Issue #67](https://github.com/manticoresoftware/docker/issues/67) 修正了在使用 `--network=host` 运行 Manticore Docker 容器时的致命错误。

## 版本 6.3.8
发布日期：2024年11月22日

6.3.8 版本延续 6.3 系列，仅包含错误修复。

### 错误修复

* [PR #2777](https://github.com/manticoresoftware/manticoresearch/pull/2777) 修正了当查询并发受 `threads` 或 `max_threads_per_query` 设置限制时，可用线程数的计算。

## manticore-extra v1.1.20

发布日期：2024年10月7日

### 错误修复

* [Issue #64](https://github.com/manticoresoftware/executor/issues/64) 解决了 `unattended-upgrades` 工具在基于 Debian 的系统上自动安装包更新时，错误标记多个 Manticore 包（包括 `manticore-galera`、`manticore-executor` 和 `manticore-columnar-lib`）为待删除的虚拟包 `manticore-extra` 的问题。已做改动，确保 `unattended-upgrades` 不再尝试移除核心 Manticore 组件。

## 版本 6.3.6
发布日期：2024年8月2日

6.3.6 版本延续 6.3 系列，仅包含错误修复。

### 错误修复

* [Issue #2477](https://github.com/manticoresoftware/manticoresearch/issues/2477) 修正了 6.3.4 版本引入的崩溃，涉及表达式以及分布式或多表查询时可能出现的问题。
* [Issue #2352](https://github.com/manticoresoftware/manticoresearch/issues/2352) 修正了当查询多个索引并因 `max_query_time` 提前退出时导致守护进程崩溃或内部错误的问题。

## 版本 6.3.4
发布日期：2024年7月31日

6.3.4 版本延续 6.3 系列，仅包含小幅改进和错误修复。

### 小幅改动
* [Issue #1130](https://github.com/manticoresoftware/manticoresearch/issues/1130) 添加了对 [DBeaver](Integration/DBeaver.md#Integration-with-DBeaver) 的支持。
* [Issue #2146](https://github.com/manticoresoftware/manticoresearch/issues/2146) 改进了词形和例外中分隔符的转义。
* [Issue #2315](https://github.com/manticoresoftware/manticoresearch/issues/2315) 在 SELECT 列表达式中添加了字符串比较运算符。
* [Issue #2363](https://github.com/manticoresoftware/manticoresearch/issues/2363) 增加了对类似 Elastic 的批量请求中 null 值的支持。
* [Issue #2374](https://github.com/manticoresoftware/manticoresearch/issues/2374) 添加了对 mysqldump 版本 9 的支持。
* [Issue #2375](https://github.com/manticoresoftware/manticoresearch/issues/2375) 改进了 HTTP JSON 查询中的错误处理，提供了发生错误节点的 JSON 路径。

### 错误修复
* [Issue #2280](https://github.com/manticoresoftware/manticoresearch/issues/2280) 修复了在 disk_chunks > 1 时通配符查询匹配很多时性能降低的问题。
* [Issue #2332](https://github.com/manticoresoftware/manticoresearch/issues/2332) 修复了针对空 MVA 数组的 MVA MIN 或 MAX SELECT 列表达式中的崩溃。
* [Issue #2339](https://github.com/manticoresoftware/manticoresearch/issues/2339) 修正了 Kibana 无限范围请求的错误处理。
* [Issue #2342](https://github.com/manticoresoftware/manticoresearch/issues/2342) 修复了当属性不在 SELECT 列表中时，对右表列式属性的连接过滤器的问题。
* [Issue #2343](https://github.com/manticoresoftware/manticoresearch/issues/2343) 修复了 Manticore 6.3.2 中多余的 'static' 修饰符。
* [Issue #2344](https://github.com/manticoresoftware/manticoresearch/issues/2344) 修复了使用右表 MATCH() 时 LEFT JOIN 返回不匹配条目问题。
* [Issue #2350](https://github.com/manticoresoftware/manticoresearch/issues/2350) 修复了带有 `hitless_words` 的 RT 索引中磁盘块的保存问题。
* [Issue #2364](https://github.com/manticoresoftware/manticoresearch/issues/2364) 'aggs_node_sort' 属性现在可以在其他属性中任意顺序添加。
* [Issue #2368](https://github.com/manticoresoftware/manticoresearch/issues/2368) 修复了 JSON 查询中文本搜索与过滤器顺序错误的问题。
* [Issue #2376](https://github.com/manticoresoftware/manticoresearch/issues/2376) 修复了长 UTF-8 请求返回不正确 JSON 响应的错误。
* [Issue #2684](https://github.com/manticoresoftware/dev/issues/2684) 修正了依赖连接属性的 presort/prefilter 表达式计算错误。
* [Issue #301](https://github.com/manticoresoftware/manticoresearch-buddy/issues/301) 更改了指标数据大小的计算方法，从检查整个数据目录大小改为读取 `manticore.json` 文件。
* [Issue #302](https://github.com/manticoresoftware/manticoresearch-buddy/issues/302) 添加了对 Vector.dev 验证请求的处理。

## 版本 6.3.2
发布日期：2024 年 6 月 26 日

6.3.2 版本延续了 6.3 系列，包含若干错误修复，其中一些是在 6.3.0 发布后发现的。

### 重大变更
* ⚠️[Issue #2305](https://github.com/manticoresoftware/manticoresearch/issues/2305) 更新了 aggs.range 的值为数字类型。

### 错误修复
* [Commit c51c](https://github.com/manticoresoftware/manticoresearch/commit/c51c7fa4bd0ec4ad8d53188b888461bff29f1417) 修正了基于存储检查与 rset 合并的分组问题。
* [Commit 0e85](https://github.com/manticoresoftware/manticoresearch/commit/0e85b9d5682f51681750d798aef6a11cfb93d69e) 修复了启用 CRC 字典和 `local_df` 时，使用通配符查询 RT 索引导致守护进程崩溃的问题。
* [Issue #2200](https://github.com/manticoresoftware/manticoresearch/issues/2200) 修复了在无 GROUP BY 的 JOIN 上使用 `count(*)` 崩溃的问题。
* [Issue #2201](https://github.com/manticoresoftware/manticoresearch/issues/2201) 修正了尝试对全文字段分组时 JOIN 不返回警告的问题。
* [Issue #2230](https://github.com/manticoresoftware/manticoresearch/issues/2230) 处理了通过 `ALTER TABLE` 添加属性时未考虑 KNN 选项的问题。
* [Issue #2231](https://github.com/manticoresoftware/manticoresearch/issues/2231) 修复了 6.3.0 版本中无法卸载 `manticore-tools` Redhat 包的问题。
* [Issue #2242](https://github.com/manticoresoftware/manticoresearch/issues/2242) 修正了 JOIN 和多个 FACET 语句返回错误结果的问题。
* [Issue #2250](https://github.com/manticoresoftware/manticoresearch/issues/2250) 修复了表位于集群中时 ALTER TABLE 报错的问题。
* [Issue #2252](https://github.com/manticoresoftware/manticoresearch/issues/2252) 修正了从 SphinxQL 接口传递给 buddy 的原始查询问题。
* [Issue #2267](https://github.com/manticoresoftware/manticoresearch/issues/2267) 改进了 RT 索引带磁盘块的 `CALL KEYWORDS` 中的通配符扩展。
* [Issue #271](https://github.com/manticoresoftware/manticoresearch-buddy/issues/271) 修复了错误 `/cli` 请求导致的挂起。
* [Issue #274](https://github.com/manticoresoftware/manticoresearch-buddy/issues/274) 解决了并发请求 Manticore 时可能卡死的问题。
* [Issue #275](https://github.com/manticoresoftware/manticoresearch-buddy/issues/275) 修复了通过 `/cli` 执行 `drop table if exists t; create table t` 时的挂起问题。

### 复制相关
* [Issue #2270](https://github.com/manticoresoftware/manticoresearch/issues/2270) 在 `/_bulk` HTTP 端点添加了对 `cluster:name` 格式的支持。

## 版本 6.3.0
发布日期：2024 年 5 月 23 日

### 主要变更
* [Issue #839](https://github.com/manticoresoftware/manticoresearch/issues/839) 实现了 [float_vector](Creating_a_table/Data_types.md#Float-vector) 数据类型；实现了 [向量搜索](Searching/KNN.md#KNN-vector-search)。
* [Issue #1673](https://github.com/manticoresoftware/manticoresearch/issues/1673) [INNER/LEFT JOIN](Searching/Joining.md) （**测试阶段**）。
* [Issue #1744](https://github.com/manticoresoftware/manticoresearch/issues/1744) 为 [timestamp](Creating_a_table/Data_types.md#Timestamps) 字段实现了日期格式的自动检测。
* [Issue #1720](https://github.com/manticoresoftware/manticoresearch/pull/1720) 将 Manticore Search 许可证从 GPLv2-or-later 更改为 GPLv3-or-later。
* [Commit 7a55](https://github.com/manticoresoftware/manticoresearch/commit/7a5508424) 现在在 Windows 上运行 Manticore 需要 Docker 来运行 Buddy。
* [Issue #1541](https://github.com/manticoresoftware/manticoresearch/issues/1541) 添加了一个 [REGEX](Searching/Full_text_matching/Operators.md#REGEX-operator) 全文操作符。
* [Issue #2091](https://github.com/manticoresoftware/manticoresearch/issues/2091) 支持 Ubuntu Noble 24.04。
* [Commit 514d](https://github.com/manticoresoftware/manticoresearch/commit/514d35b497f4bdb20b6473ff963752a978a4bb8d) 重构时间操作以提升性能并新增日期/时间函数：
  - [CURDATE()](Functions/Date_and_time_functions.md#CURDATE%28%29) - 返回本地时区当前日期
  - [QUARTER()](Functions/Date_and_time_functions.md#QUARTER%28%29) - 返回时间戳参数所在年份的季度整数
  - [DAYNAME()](Functions/Date_and_time_functions.md#DAYNAME%28%29) - 返回时间戳参数对应的星期名称
  - [MONTHNAME()](Functions/Date_and_time_functions.md#MONTHNAME%28%29) - 返回时间戳参数对应的月份名称
  - [DAYOFWEEK()](Functions/Date_and_time_functions.md#DAYOFWEEK%28%29) - 返回时间戳参数对应的星期整数索引
  - [DAYOFYEAR()](Functions/Date_and_time_functions.md#DAYOFYEAR%28%29) - 返回时间戳参数对应的年份中的第几天的整数
  - [YEARWEEK()](Functions/Date_and_time_functions.md#YEARWEEK%28%29) - 返回时间戳参数对应的年份和当前周的第一天的日期代码
  - [DATEDIFF()](Functions/Date_and_time_functions.md#DATEDIFF%28%29) - 返回两个时间戳之间的天数差
  - [DATE()](Functions/Date_and_time_functions.md#DATE%28%29) - 格式化时间戳参数中的日期部分
  - [TIME()](Functions/Date_and_time_functions.md#TIME%28%29) - 格式化时间戳参数中的时间部分
  - [timezone](Server_settings/Searchd.md#timezone) - 日期/时间相关函数使用的时区。
* [Commit 30e7](https://github.com/manticoresoftware/manticoresearch/commit/30e789d9657f9b3093c280c6e5b0e46a021f6dc7) 向 HTTP 接口添加了 [range](Searching/Faceted_search.md#Facet-over-set-of-ranges)、[histogram](Searching/Faceted_search.md#Facet-over-histogram-values)、[date_range](Searching/Faceted_search.md#Facet-over-set-of-date-ranges) 和 [date_histogram](Searching/Faceted_search.md#Facet-over-histogram-date-values) 聚合，同时将类似表达式加入 SQL 支持。

### 次要更新
* [Issue #1285](https://github.com/manticoresoftware/manticoresearch/issues/1285) 支持 Filebeat 版本 8.10 - 8.11。
* [Issue #1771](https://github.com/manticoresoftware/manticoresearch/issues/1771) 支持 [ALTER TABLE ... type='distributed'](Updating_table_schema_and_settings.md#Changing-a-distributed-table)。
* [Issue #1788](https://github.com/manticoresoftware/manticoresearch/issues/1788) 通过 [CREATE TABLE ... LIKE ... WITH DATA](Creating_a_table/Local_tables/Real-time_table.md#CREATE-TABLE-LIKE:) SQL 语句新增复制表的能力。
* [Issue #2072](https://github.com/manticoresoftware/manticoresearch/issues/2072) 优化了 [表压缩算法](Securing_and_compacting_a_table/Compacting_a_table.md#Compacting-a-Table)：之前手动执行的 [OPTIMIZE](Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE) 和自动的 [auto_optimize](Server_settings/Searchd.md#auto_optimize) 过程都会先合并数据块以确保其数量不超限，然后再对所有包含删除文档的其他数据块执行清理删减文档的操作。这种做法有时资源消耗过大，已被禁用。现在数据块合并仅依据 [progressive_merge](Server_settings/Common.md#progressive_merge) 设置执行，但删除文档较多的数据块更容易被合并。
* [Commit ce6c](https://github.com/manticoresoftware/columnar/commit/ce6c5d3381edcdc411a00a5923d13d56dd9b3010) 添加了针对加载新版次级索引的保护措施。
* [Issue #1417](https://github.com/manticoresoftware/manticoresearch/issues/1417) 通过 [REPLACE INTO ... SET](Data_creation_and_modification/Updating_documents/REPLACE.md#SQL-REPLACE) 实现部分替换。
* [Commit 7c16](https://github.com/manticoresoftware/manticoresearch/commit/7c167b934ce416d3dfbd658744a231e638b14d9a) 更新了默认合并缓冲区大小：`.spa` (标量属性): 256KB -> 8MB；`.spb` (二进制大对象属性): 256KB -> 8MB；`.spc` (列式属性): 1MB，未变；`.spds` (文档存储): 256KB -> 8MB；`.spidx` (次级索引): 从256KB缓冲区变更为128MB内存限制；`.spi` (字典): 256KB -> 16MB；`.spd` (文档列表): 8MB，未变；`.spp` (命中列表): 8MB，未变；`.spe` (跳跃列表): 256KB -> 8MB。
* [Issue #1859](https://github.com/manticoresoftware/manticoresearch/issues/1859) 通过 JSON 增加了 [复合聚合](Searching/Grouping.md#GROUP-BY-multiple-fields-at-once) 支持。
* [Commit 216b](https://github.com/manticoresoftware/manticoresearch-buddy/commit/216b824) 由于部分正则表达式匹配问题且无明显性能提升，禁用 PCRE.JIT。
* [Commit 55cd](https://github.com/manticoresoftware/manticoresearch/commit/55cdce32) 添加入门级 Galera v.3 (api v25) (`libgalera_smm.so` 来自 MySQL 5.x) 支持。
* [Commit 86f9](https://github.com/manticoresoftware/manticoresearch-buddy/commit/86f98ab) 指标后缀从 `_rate` 改为 `_rps`。
* [Commit c0c1](https://github.com/manticoresoftware/manticoresearch-helm/commit/c0c1cff780031412ef57cefab987dfd16613368e) 改进了关于负载均衡器 HA 支持的文档。
* [Commit d1d2](https://github.com/manticoresoftware/manticoresearch/commit/d1d2ca18857c68106e89743756260e73285be739) 错误消息中将 `index` 更改为 `table`；修复了 bison 解析器错误消息的修正。
* [Commit fd26](https://github.com/manticoresoftware/manticoresearch/commit/fd26671b) 支持将 `manticore.tbl` 作为表名。
* [Issue #1105](https://github.com/manticoresoftware/manticoresearch/issues/1105) 支持通过 systemd 运行索引器 ([文档](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Schedule-indexer-via-systemd))。❤️ 感谢 [@subnix](https://github.com/subnix) 的 PR。
* [Issue #1294](https://github.com/manticoresoftware/manticoresearch/issues/1294) GEODIST() 中支持二级索引。
* [Issue #1394](https://github.com/manticoresoftware/manticoresearch/issues/1394) 简化了 [SHOW THREADS](Node_info_and_management/SHOW_THREADS.md#SHOW-THREADS)。
* [Issue #1424](https://github.com/manticoresoftware/manticoresearch/issues/1424) 在 `create distributed table` 语句中增加了默认值支持（`agent_connect_timeout` 和 `agent_query_timeout`）。
* [Issue #1442](https://github.com/manticoresoftware/manticoresearch/issues/1442) 增加了覆盖 `searchd.expansion_limit` 的搜索查询选项 [expansion_limit](Searching/Options.md#expansion_limit)。
* [Issue #1448](https://github.com/manticoresoftware/manticoresearch/issues/1448) 实现了整型到大整型的 [ALTER TABLE](Updating_table_schema_and_settings.md#Updating-table-schema-in-RT-mode) 转换。
* [Issue #1456](https://github.com/manticoresoftware/manticoresearch/issues/1456) MySQL 响应中增加元信息。
* [Issue #1494](https://github.com/manticoresoftware/manticoresearch/issues/1494) [SHOW VERSION](Node_info_and_management/SHOW_VERSION.md#SHOW-VERSION)。
* [Issue #1582](https://github.com/manticoresoftware/manticoresearch/issues/1582) 支持通过 JSON 按 ID 数组删除文档 ([删除文档](Data_creation_and_modification/Deleting_documents.md?client=JSON#Deleting-documents))。
* [Issue #1589](https://github.com/manticoresoftware/manticoresearch/issues/1589) 改进“unsupported value type”错误。
* [Issue #1634](https://github.com/manticoresoftware/manticoresearch/issues/1634) 在 `SHOW STATUS` 中增加 Buddy 版本。
* [Issue #1641](https://github.com/manticoresoftware/manticoresearch/issues/1641) 针对关键字文档数为零的情况优化匹配请求。
* [Issue #1712](https://github.com/manticoresoftware/manticoresearch/issues/1712) 在提交数据时支持从字符串值 "true" 和 "false" 转换为布尔属性。
* [Issue #1713](https://github.com/manticoresoftware/manticoresearch/issues/1713) 增加了 [access_dict](Server_settings/Searchd.md#access_dict) 表和 searchd 选项。
* [Issue #1767](https://github.com/manticoresoftware/manticoresearch/issues/1767) 在配置文件的 searchd 部分新增选项：[expansion_merge_threshold_docs](Server_settings/Searchd.md#expansion_merge_threshold_docs) 和 [expansion_merge_threshold_hits](Server_settings/Searchd.md#expansion_merge_threshold_hits)；使扩展词条的微小词合并阈值可配置。
* [Issue #1768](https://github.com/manticoresoftware/manticoresearch/issues/1768) 在 `@@system.sessions` 中增加显示最后命令时间。
* [Issue #1806](https://github.com/manticoresoftware/manticoresearch/issues/1806) 将 Buddy 协议升级为版本 2。
* [Issue #1810](https://github.com/manticoresoftware/manticoresearch/issues/1810) 为 `/sql` 端点实现了更多请求格式，便于库集成。
* [Issue #1825](https://github.com/manticoresoftware/manticoresearch/pull/1825) 在 SHOW BUDDY PLUGINS 中新增信息部分。
* [Issue #1837](https://github.com/manticoresoftware/manticoresearch/issues/1837) 改进在处理大包的 `CALL PQ` 中的内存消耗。
* [Issue #1853](https://github.com/manticoresoftware/manticoresearch/pull/1853) 编译器从 Clang 15 切换到 Clang 16。
* [Issue #1857](https://github.com/manticoresoftware/manticoresearch/issues/1857) 增加字符串比较功能。❤️ 感谢 [@etcd](https://github.com/etcd) 的 PR。
* [Issue #1915](https://github.com/manticoresoftware/manticoresearch/issues/1915) 增加对联接存储字段的支持。
* [Issue #1937](https://github.com/manticoresoftware/manticoresearch/issues/1937) 查询日志中记录客户端主机:端口。
* [Issue #1981](https://github.com/manticoresoftware/manticoresearch/issues/1981) 修正错误的错误信息。
* [Issue #1983](https://github.com/manticoresoftware/manticoresearch/issues/1983) 为 [通过 JSON 查询计划](Node_info_and_management/Profiling/Query_plan.md?client=JSON#Query-plan)引入详细等级支持。
* [Issue #2010](https://github.com/manticoresoftware/manticoresearch/issues/2010) 除非设置 `log_level=debug`，否则禁用 Buddy 查询日志记录。
* [Issue #2035](https://github.com/manticoresoftware/manticoresearch/issues/2035) 支持 Linux Mint 21.3。
* [Issue #2056](https://github.com/manticoresoftware/manticoresearch/issues/2056) 修复 Manticore 无法与 Mysql 8.3+ 一起构建的问题。
* [Issue #2112](https://github.com/manticoresoftware/manticoresearch/issues/2112) 为实时表块增加 `DEBUG DEDUP` 命令，用于处理附加包含重复数据的普通表后可能出现的重复项。
* [Issue #212](https://github.com/manticoresoftware/manticoresearch-buddy/issues/212) 在 [SHOW QUERIES](Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES) 中增加时间。
* [Issue #218](https://github.com/manticoresoftware/manticoresearch-buddy/issues/218) 将 `@timestamp` 列处理为时间戳。
* [Issue #252](https://github.com/manticoresoftware/manticoresearch-buddy/issues/250) 实现启用/禁用 buddy 插件的逻辑。
* [Issue #254](https://github.com/manticoresoftware/manticoresearch-buddy/issues/254) 更新 composer 到修复了近期 CVE 的较新版本。
* [Issue #340](https://github.com/manticoresoftware/manticoresearch/issues/340) Manticore systemd单元中与 `RuntimeDirectory` 相关的轻微优化。
* [Issue #51](https://github.com/manticoresoftware/executor/issues/51) 添加了rdkafka支持并更新到PHP 8.3.3。
* [Issue #527](https://github.com/manticoresoftware/manticoresearch/issues/527) 支持[附加](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Attaching_one_table_to_another.md#Attaching-one-table-to-another)一个RT表。新增命令[ALTER TABLE ... RENAME](Updating_table_schema_and_settings.md#Renaming-a-real-time-table)。

### 重大变更和废弃
* ⚠️[Issue #1436](https://github.com/manticoresoftware/manticoresearch/issues/1436) 修复了IDF计算问题。`local_df`现在为默认值。改进了主从搜索协议（版本已更新）。如果您在分布式环境中运行多个Manticore Search实例，请确保先升级代理，再升级主节点。
* ⚠️[Issue #1572](https://github.com/manticoresoftware/manticoresearch/issues/1572) 添加了分布式表的复制并更新了复制协议。如果您运行复制集群，需要：
  - 首先，干净地停止所有节点
  - 然后，使用Linux工具`manticore_new_cluster`带`--new-cluster`参数启动最后停止的节点。
  - 更多细节请阅读[重启集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster)。
* ⚠️[Issue #1763](https://github.com/manticoresoftware/manticoresearch/pull/1763) HTTP API端点别名`/json/*`已被废弃。
* ⚠️[Issue #1982](https://github.com/manticoresoftware/manticoresearch/issues/1982) 在JSON中将[profile](Node_info_and_management/Profiling/Query_profile.md#Query-profile)改为[plan](Node_info_and_management/Profiling/Query_plan.md#Query-plan)，新增了JSON查询分析功能。
* ⚠️[Commit e235](https://github.com/manticoresoftware/manticoresearch-backup/commit/e23585a) manticore-backup不再备份`plugin_dir`。
* ⚠️[Issue #171](https://github.com/manticoresoftware/manticoresearch-buddy/issues/155) 将Buddy迁移至Swoole以提升性能和稳定性。切换到新版本时，请确保所有Manticore包已更新。
* ⚠️[Issue #196](https://github.com/manticoresoftware/manticoresearch-buddy/pull/205) 将所有核心插件合并到Buddy内并更改了核心逻辑。
* ⚠️[Issue #2107](https://github.com/manticoresoftware/manticoresearch/issues/2107) 在`/search`响应中将文档ID视为数字。
* ⚠️[Issue #38](https://github.com/manticoresoftware/executor/issues/38) 添加了Swoole，禁用了ZTS，并移除了parallel扩展。
* ⚠️[Issue #1929](https://github.com/manticoresoftware/manticoresearch/issues/1929) 某些情况下`charset_table`中的覆盖未生效。

### 复制相关变更
* [Commit 3376](https://github.com/manticoresoftware/manticoresearch/commit/3376351f73db3ff0cb6cb708a304a7bd83e69cfb) 修复了大型文件SST复制错误。
* [Commit 6d36](https://github.com/manticoresoftware/manticoresearch/commit/6d36c68fb7ce3cb274b467aac435788905084952) 为复制命令添加了重试机制；修复了网络忙且丢包时复制加入失败的问题。
* [Commit 842e](https://github.com/manticoresoftware/manticoresearch/commit/842e27e71790833ab4125d49103d2933d5df60c3) 将复制中的FATAL消息改为WARNING消息。
* [Commit 8c32](https://github.com/manticoresoftware/manticoresearch/commit/8c32bce7db39d8ed3b1a06aaa1d2c3ec372084f0) 修正了复制集群中无表或空表时`gcache.page_size`的计算；修复了Galera选项的保存和加载。
* [Commit a2af](https://github.com/manticoresoftware/manticoresearch/commit/a2af06ca36abcbf893e43c8803853130b397c54b) 新增功能：跳过加入集群节点上的更新节点复制命令。
* [Commit c054](https://github.com/manticoresoftware/manticoresearch/commit/c0541e6892f5116b1f957b40060ba8ecd01a5afb) 修复了复制过程中更新blob属性与替换文档间的死锁。
* [Commit e80d](https://github.com/manticoresoftware/manticoresearch/commit/e80d505b963999016033613c40c1cf3151d648e7) 添加了[replication_connect_timeout](Server_settings/Searchd.md#replication_connect_timeout)、[replication_query_timeout](Server_settings/Searchd.md#replication_query_timeout)、[replication_retry_delay](Server_settings/Searchd.md#replication_retry_delay)、[replication_retry_count](Server_settings/Searchd.md#replication_retry_count)等searchd配置选项，用以控制复制网络，类似于`searchd.agent_*`，但默认值不同。
* [Issue #1356](https://github.com/manticoresoftware/manticoresearch/issues/1356) 修复了在某些节点丢失且节点名解析失败后复制节点重试问题。
* [Issue #1445](https://github.com/manticoresoftware/manticoresearch/issues/1445) 修正了`show variables`中复制日志详细级别。
* [Issue #1482](https://github.com/manticoresoftware/manticoresearch/issues/1482) 修复了Kubernetes中pod重启后加入集群节点复制问题。
* [Issue #1962](https://github.com/manticoresoftware/manticoresearch/issues/1962) 修复了空集群中无效节点名导致复制修改长时间等待的问题。

### Bug修复
* [Commit 8a48](https://github.com/manticoresoftware/manticoresearch/commit/8a486c7ba1a293879cdecc41fb52eb7ab0cbf832) 修复了`count distinct`中未使用匹配项清理可能导致崩溃的问题。
* [Issue #1569](https://github.com/manticoresoftware/manticoresearch/issues/1569) 二进制日志现以事务粒度写入。
* [Issue #2089](https://github.com/manticoresoftware/manticoresearch/issues/2089) 修复了与64位ID相关的一个错误，该错误可能导致通过MySQL插入时出现“Malformed packet”错误，进而导致[表损坏和ID重复](https://github.com/manticoresoftware/manticoresearch/issues/2090)。
* [Issue #2160](https://github.com/manticoresoftware/manticoresearch/issues/2160) 修正了日期插入时被当作UTC时间而非本地时区的问题。
* [Issue #2177](https://github.com/manticoresoftware/manticoresearch/issues/2177) 修复了在对具有非空`index_token_filter`的实时表执行搜索时发生的崩溃。
* [Issue #2209](https://github.com/manticoresoftware/manticoresearch/issues/2209) 修改了RT列存储中的重复过滤，以修复崩溃和错误的查询结果。
* [Commit 001d](https://github.com/manticoresoftware/manticoresearch/commit/001d82904e4b887c49387308291aeefb07c0fbc4) 修正了html剥离器在处理连接字段后破坏内存的问题。
* [Commit 00eb](https://github.com/manticoresoftware/manticoresearch/commit/00eb2ebf) 避免了flush后回绕流，以防止与mysqldump的通信错误。
* [Commit 0553](https://github.com/manticoresoftware/manticoresearch/commit/05534114acf08c70ffcc41a23d558dec09a34373) 如果preread未开始，则不等待其完成。
* [Commit 055a](https://github.com/manticoresoftware/manticoresearch/commit/055aac09f877570cd29b6594cb283182eba7f73b) 修正了Buddy输出的长字符串在searchd日志中被拆分为多行的问题。
* [Commit 0a88](https://github.com/manticoresoftware/manticoresearch/commit/0a88056307f82c50fcdc458ad6dbc307a00e3f92) 移动了关于MySQL接口调试`debugv`级别失败的警告。
* [Commit 150a](https://github.com/manticoresoftware/manticoresearch/commit/150a20286b7d98d24b844ae577d7fda4a444798d) 修复了多个集群管理操作的竞态条件；禁止创建同名或同路径的多个集群。
* [Commit 2e40](https://github.com/manticoresoftware/manticoresearch/commit/2e405c1c9ebac6f46206e3df17ac1c19129c9aa3) 修正了全文查询中的隐式截止；将MatchExtended分拆为模板partD。
* [Commit 75f5](https://github.com/manticoresoftware/manticoresearch/commit/75f51421542a2a0658f9e2f9f31048c8c5224479) 修正了`index_exact_words`在索引与加载表到守护进程间的不一致。
* [Commit 7643](https://github.com/manticoresoftware/manticoresearch/commit/7643976bea142e08535d701000eff0d09a99fda2) 修复了删除无效集群时缺失错误消息的问题。
* [Commit 7a03](https://github.com/manticoresoftware/manticoresearch/commit/7a034ac990d5ec9344320c05a6a2315583b9b858) 修正了CBO与队列联合；修复了CBO与RT伪分片。
* [Commit 7b4e](https://github.com/manticoresoftware/manticoresearch/commit/7b4ea95e) 当未加载次级索引（SI）库及配置参数时，防止出现误导性的警告信息“WARNING: secondary_indexes set but failed to initialize secondary library”。
* [Commit 8496](https://github.com/manticoresoftware/manticoresearch/commit/8496317f099fade8d2db463fb88483e7fcdfe85b) 修正了多数投票（quorum）中的命中排序。
* [Commit 8973](https://github.com/manticoresoftware/manticoresearch-buddy/commit/8973ad3) 修复了ModifyTable插件中对大写选项的处理问题。
* [Commit 9935](https://github.com/manticoresoftware/manticoresearch-buddy/commit/99350d4c184379b19f3efa9751d9b5be5e6edfb5) 修正了从包含空json值（表示为NULL）的转储中恢复的问题。
* [Commit a28f](https://github.com/manticoresoftware/manticoresearch/commit/a28f6e087b3d957e48816070be17d0823904936a) 通过使用pcon修复了连接节点接收SST时的SST超时。
* [Commit b5a5](https://github.com/manticoresoftware/manticoresearch/commit/b5a55cc26debcddcda5d9caef8366a707e0dd763) 修复了选择别名字符串属性时的崩溃。
* [Commit c556](https://github.com/manticoresoftware/manticoresearch/commit/c556e7ed8b6da8eac8ef094650df5f0a8977916b) 添加了对全文查询中带有`morphology_skip_fields`字段的词条转换为`=term`的支持。
* [Commit cdc3](https://github.com/manticoresoftware/manticoresearch/commit/cdc38085145cd1025b96fcac2a46188a0425d9e1) 补充了缺失的配置键（skiplist_cache_size）。
* [Commit cf6e](https://github.com/manticoresoftware/manticoresearch/commit/cf6ec3bfdc3f0990fb31ff53822b2331a6667727) 修复了表达式排序器在处理大型复杂查询时的崩溃。
* [Commit e513](https://github.com/manticoresoftware/manticoresearch/commit/e5139ba75eef7223df375ae3cc3f138fa7d925f1) 修正了全文CBO与无效索引提示的兼容问题。
* [Commit eb05](https://github.com/manticoresoftware/manticoresearch/commit/eb0514b3a6f57f278252b0724eebd112ec116e0f) 在关闭时中断preread以实现更快关闭。
* [Commit f945](https://github.com/manticoresoftware/manticoresearch/commit/f94555a293cfa51004ff81135b8bd8500b955115) 修改了全文查询的栈计算，以避免复杂查询时崩溃。
* [Issue #1262](https://github.com/manticoresoftware/manticoresearch/issues/1262) 修复了索引器在索引SQL源时遇到多个同名列导致崩溃的问题。
* [Issue #1273](https://github.com/manticoresoftware/manticoresearch/issues/1273) 对不存在的系统变量返回0而非<empty>。
* [Issue #1289](https://github.com/manticoresoftware/manticoresearch/issues/1289) 修复了indextool检查RT表外部文件时的错误。
* [Issue #1335](https://github.com/manticoresoftware/manticoresearch/issues/1335) 修正了由于短语中的多重词形导致的查询解析错误。
* [Issue #1364](https://github.com/manticoresoftware/manticoresearch/issues/1364) 添加了对旧版本binlog的空binlog文件回放支持。
* [Issue #1365](https://github.com/manticoresoftware/manticoresearch/issues/1365) 修复了删除最后一个空binlog文件的问题。
* [Issue #1372](https://github.com/manticoresoftware/manticoresearch/issues/1372) 修复了在更改 `data_dir` 后，错误的相对路径（从守护进程启动目录转换为绝对路径）影响当前工作目录的问题。
* [Issue #1393](https://github.com/manticoresoftware/manticoresearch/issues/1393) hn_small 中最慢时间降解：在守护进程启动时获取/缓存 CPU 信息。
* [Issue #1395](https://github.com/manticoresoftware/manticoresearch/issues/1395) 修复了索引加载期间关于缺失外部文件的警告。
* [Issue #1402](https://github.com/manticoresoftware/manticoresearch/issues/1402) 修复了在释放数据指针属性时，global groupers 崩溃的问题。
* [Issue #1403](https://github.com/manticoresoftware/manticoresearch/issues/1403) _ADDITIONAL_SEARCHD_PARAMS 无法生效。
* [Issue #1427](https://github.com/manticoresoftware/manticoresearch/issues/1427) 修复了每张表的 `agent_query_timeout` 被默认查询选项 `agent_query_timeout` 替代的问题。
* [Issue #1444](https://github.com/manticoresoftware/manticoresearch/issues/1444) 修复了使用 `packedfactors()` 并为每个匹配项提供多个值时，grouper 和 ranker 崩溃的问题。
* [Issue #1458](https://github.com/manticoresoftware/manticoresearch/issues/1458) 频繁索引更新时 Manticore 崩溃。
* [Issue #1481](https://github.com/manticoresoftware/manticoresearch/issues/1481) 修复了解析错误后清理解析查询时的崩溃。
* [Issue #1484](https://github.com/manticoresoftware/manticoresearch/issues/1484) 修复了 HTTP JSON 请求未路由到 buddy 的问题。
* [Issue #1499](https://github.com/manticoresoftware/manticoresearch/issues/1499) JSON 属性的根值不能是数组。已修复。
* [Issue #1507](https://github.com/manticoresoftware/manticoresearch/issues/1507) 修复了事务中重建表时的崩溃。
* [Issue #1515](https://github.com/manticoresoftware/manticoresearch/issues/1515) 修复了 RU 词形短写形式的展开。
* [Issue #1579](https://github.com/manticoresoftware/manticoresearch/issues/1579) 修复了在 [date_format](Functions/Date_and_time_functions.md#DATE_FORMAT()) 表达式中使用 JSON 和 STRING 属性的问题。
* [Issue #1580](https://github.com/manticoresoftware/manticoresearch/issues/1580) 修复了对 JSON 字段多个别名的 grouper 的问题。
* [Issue #1594](https://github.com/manticoresoftware/manticoresearch/issues/1594) 修复了在开发环境中错误的 total_related：修复隐式 cutoff 与 limit 的问题；在 JSON 查询中添加了更好的全扫描检测。
* [Issue #1603](https://github.com/manticoresoftware/manticoresearch/issues/1603) 修复了所有日期表达式中使用 JSON 和 STRING 属性的问题。
* [Issue #1609](https://github.com/manticoresoftware/manticoresearch/issues/1609) 使用 LEVENSHTEIN() 时崩溃。
* [Issue #1612](https://github.com/manticoresoftware/manticoresearch/issues/1612) 修复了带高亮的搜索查询解析错误后引起的内存破坏。
* [Issue #1614](https://github.com/manticoresoftware/manticoresearch/issues/1614) 禁用了对短于 `min_prefix_len` / `min_infix_len` 词项的通配符扩展。
* [Issue #1617](https://github.com/manticoresoftware/manticoresearch/issues/1617) 修改行为，如果 Buddy 成功处理请求，则不记录错误。
* [Issue #1635](https://github.com/manticoresoftware/manticoresearch/issues/1635) 修复了带有设置 limit 的查询，搜索查询元数据中的 total 值。
* [Issue #1640](https://github.com/manticoresoftware/manticoresearch/issues/1640) 无法在纯模式下通过 JSON 使用带大写的表。
* [Issue #1643](https://github.com/manticoresoftware/manticoresearch/issues/1643) 提供了默认的 `SPH_EXTNODE_STACK_SIZE` 值。
* [Issue #1646](https://github.com/manticoresoftware/manticoresearch/issues/1646) 修复了 MVA 属性上使用 ALL/ANY 的负过滤条件时 SphinxQL 日志问题。
* [Issue #1660](https://github.com/manticoresoftware/manticoresearch/issues/1660) 修复了来自其他索引的 docid killlists 应用问题。❤️ 感谢 [@raxoft](https://github.com/raxoft) 的 PR。
* [Issue #1668](https://github.com/manticoresoftware/manticoresearch/issues/1668) 修复了由于原始索引全扫描（无索引迭代器）早期退出导致的匹配遗漏；移除了纯行迭代器的 cutoff。
* [Issue #1671](https://github.com/manticoresoftware/manticoresearch/issues/1671) 修复了带 agent 和本地表的分布式表查询时 `FACET` 出错。
* [Issue #1690](https://github.com/manticoresoftware/manticoresearch/issues/1690) 修复了大值的直方图估计崩溃。
* [Issue #1692](https://github.com/manticoresoftware/manticoresearch/issues/1692) alter table tbl add column col uint 时崩溃。
* [Issue #1710](https://github.com/manticoresoftware/manticoresearch/issues/1710) 条件 `WHERE json.array IN (<value>)` 返回空结果。
* [Issue #172](https://github.com/manticoresoftware/manticoresearch-buddy/issues/172) 修复了向 `/cli` 发送请求时 TableFormatter 的问题。
* [Issue #1742](https://github.com/manticoresoftware/manticoresearch/issues/1742) 在缺少 wordforms 文件时，`CREATE TABLE` 不会失败。
* [Issue #1762](https://github.com/manticoresoftware/manticoresearch/issues/1762) RT 表中的属性顺序现在遵循配置顺序。
* [Issue #1765](https://github.com/manticoresoftware/manticoresearch/issues/1765) 带 'should' 条件的 HTTP bool 查询返回错误结果。
* [Issue #1769](https://github.com/manticoresoftware/manticoresearch/issues/1769) 按字符串属性排序时不支持 `SPH_SORT_ATTR_DESC` 和 `SPH_SORT_ATTR_ASC`。
* [Issue #177](https://github.com/manticoresoftware/manticoresearch-buddy/issues/177) 禁用了 curl 请求到 Buddy 时的 `Expect: 100-continue` HTTP 头。
* [Issue #1791](https://github.com/manticoresoftware/manticoresearch/issues/1791) 由 GROUP BY 别名引起的崩溃。
* [Issue #1792](https://github.com/manticoresoftware/manticoresearch/issues/1792) Windows 上 SQL meta 摘要显示错误时间。
* [Issue #1794](https://github.com/manticoresoftware/manticoresearch/issues/1794) 修复了 JSON 查询中单词项性能下降的问题。
* [Issue #1798](https://github.com/manticoresoftware/manticoresearch/issues/1798) 不兼容的过滤器在 `/search` 请求中未引发错误。
* [Issue #1802](https://github.com/manticoresoftware/manticoresearch/issues/1802) 修复了 `ALTER CLUSTER ADD` 和 `JOIN CLUSTER` 操作，增加相互等待，防止在 `ALTER` 向集群添加表而捐赠者发送表给加入者节点时发生竞态条件。
* [Issue #1811](https://github.com/manticoresoftware/manticoresearch/issues/1811) 错误处理 `/pq/{table}/*` 请求。
* [Issue #1816](https://github.com/manticoresoftware/manticoresearch/issues/1816) `UNFREEZE` 在某些情况下无法正常工作。
* [Issue #183](https://github.com/manticoresoftware/manticoresearch-buddy/pull/183) 修复了某些情况下 MVA 恢复的问题。
* [Issue #1849](https://github.com/manticoresoftware/manticoresearch/issues/1849) 修复了使用 MCL 时 indextool 在关闭时崩溃的问题。
* [Issue #1866](https://github.com/manticoresoftware/manticoresearch/issues/1866) 修复 `/cli_json` 请求的不必要的 URL 解码。
* [Issue #1872](https://github.com/manticoresoftware/manticoresearch/issues/1872) 更改守护进程启动时的 plugin_dir 设置逻辑。
* [Issue #1874](https://github.com/manticoresoftware/manticoresearch/issues/1874) alter table ... 异常失败。
* [Issue #1891](https://github.com/manticoresoftware/manticoresearch/issues/1891#issuecomment-2051941200) 插入数据时 Manticore 遇到 `signal 11` 崩溃。
* [Issue #1920](https://github.com/manticoresoftware/manticoresearch/issues/1920) 降低了 [low_priority](Searching/Options.md#low_priority) 的节流限制。
* [Issue #1924](https://github.com/manticoresoftware/manticoresearch/issues/1924) Mysqldump + mysql 恢复错误。
* [Issue #1951](https://github.com/manticoresoftware/manticoresearch/issues/1951) 修复了分布式表在缺少本地表或代理配置不正确时创建错误的问题；现在会返回错误信息。
* [Issue #1972](https://github.com/manticoresoftware/manticoresearch/issues/1972) 实现了 `FREEZE` 计数器以避免 freeze/unfreeze 问题。
* [Issue #1980](https://github.com/manticoresoftware/manticoresearch/issues/1980) 遵守 OR 节点中的查询超时。之前在某些情况下 `max_query_time` 可能不起作用。
* [Issue #1986](https://github.com/manticoresoftware/manticoresearch/issues/1986) 重命名 new 到 current [manticore.json] 失败。
* [Issue #1988](https://github.com/manticoresoftware/manticoresearch/issues/1988) 全文查询可能忽略 `SecondaryIndex` CBO 提示。
* [Issue #1990](https://github.com/manticoresoftware/manticoresearch/issues/1990) 修正了 `expansion_limit`，用以对来自多个磁盘块或内存块的调用关键词进行最终结果集切片。
* [Issue #1994](https://github.com/manticoresoftware/manticoresearch/issues/1994) 错误的外部文件。
* [Issue #2021](https://github.com/manticoresoftware/manticoresearch/issues/2021) 停止 Manticore 后，可能仍有部分 manticore-executor 进程残留运行。
* [Issue #2029](https://github.com/manticoresoftware/manticoresearch/issues/2029) 使用 Levenshtein Distance 时崩溃。
* [Issue #2037](https://github.com/manticoresoftware/manticoresearch/issues/2037) 在空索引上运行多个 max 运算符后出现错误。
* [Issue #2052](https://github.com/manticoresoftware/manticoresearch/issues/2052) 使用 JSON.field 的多重分组时崩溃。
* [Issue #2067](https://github.com/manticoresoftware/manticoresearch/issues/2067) 在对 _update 的错误请求时 Manticore 崩溃。
* [Issue #2069](https://github.com/manticoresoftware/manticoresearch/issues/2069) 修复 JSON 接口中字符串过滤比较器在闭区间的使用问题。
* [Issue #2082](https://github.com/manticoresoftware/manticoresearch/issues/2082) 当 data_dir 路径位于符号链接上时，`alter` 操作失败。
* [Issue #2102](https://github.com/manticoresoftware/manticoresearch/pull/2102) 改进 mysqldump 中 SELECT 查询的特殊处理，确保生成的 INSERT 语句兼容 Manticore。
* [Issue #2103](https://github.com/manticoresoftware/manticoresearch/issues/2103) 泰语字符使用了错误的字符集。
* [Issue #2124](https://github.com/manticoresoftware/manticoresearch/issues/2124) 使用含保留字的 SQL 导致崩溃。
* [Issue #2154](https://github.com/manticoresoftware/manticoresearch/issues/2154) 带有词形变化的表无法导入。
* [Issue #2176](https://github.com/manticoresoftware/manticoresearch/issues/2176) 修复当引擎参数设置为 'columnar' 并通过 JSON 添加重复 ID 时发生的崩溃。
* [Issue #223](https://github.com/manticoresoftware/manticoresearch-buddy/issues/223) 当尝试插入无模式且无列名的文档时，返回正确错误。
* [Issue #239](https://github.com/manticoresoftware/manticoresearch-buddy/issues/239) 自动模式多行插入可能失败。
* [Issue #399](https://github.com/manticoresoftware/manticoresearch/issues/399) 如果数据源声明了 id 属性，索引时添加错误信息。
* [Issue #59](https://github.com/manticoresoftware/manticoresearch-helm/issues/59) Manticore 集群崩溃。
* [Issue #68](https://github.com/manticoresoftware/manticoresearch-helm/issues/68) 如果存在 percolate 表，optimize.php 会崩溃。
* [Issue #77](https://github.com/manticoresoftware/manticoresearch-helm/issues/77) 部署到 Kubernetes 时的错误修复。
* [Issue #274](https://github.com/manticoresoftware/manticoresearch-buddy/issues/274) 修复对 Buddy 的并发请求处理错误。

### 与 manticore-backup 相关
* [Issue #97](https://github.com/manticoresoftware/manticoresearch-backup/pull/97) 在可用时，将 VIP HTTP 端口设为默认。
各种改进：改进了版本检查和流式 ZSTD 解压缩；在恢复过程中添加了用户提示以处理版本不匹配；修正了恢复时对不同版本的错误提示行为；增强了解压缩逻辑，改为直接从流读取而非工作内存；添加了 `--force` 标志
* [提交 3b35](https://github.com/manticoresoftware/manticoresearch-backup/commit/3b357ea) 在 Manticore 搜索启动后添加备份版本显示，以便在该阶段识别问题。
* [提交 ad2e](https://github.com/manticoresoftware/manticoresearch-backup/commit/ad2e5da) 更新了守护进程连接失败的错误消息。
* [提交 ce5e](https://github.com/manticoresoftware/manticoresearch-backup/commit/ce5ea85) 修正将绝对根备份路径转换为相对路径的问题，并移除了恢复时的可写检查，以支持从不同路径恢复。
* [提交 db7e](https://github.com/manticoresoftware/manticoresearch-backup/commit/db7e2b9) 为文件迭代器添加排序，确保各种情况下一致性。
* [问题 #106](https://github.com/manticoresoftware/manticoresearch-backup/issues/105) 多配置的备份与恢复。
* [问题 #91](https://github.com/manticoresoftware/manticoresearch-backup/pull/91) 添加了 defattr，以防止 RHEL 安装后文件出现异常用户权限。
* [问题 #91](https://github.com/manticoresoftware/manticoresearch-backup/pull/91) 添加了额外的 chown，确保文件在 Ubuntu 默认属于 root 用户。

### 与 MCL 相关（列存，次级索引，KNN 库）
* [提交 f104](https://github.com/manticoresoftware/columnar/commit/f104a4709bbb144140d09a09bc0fc1db1895e331) 支持向量搜索。
* [提交 2169](https://github.com/manticoresoftware/columnar/commit/216953beef6676f7e34c9f6f1a30c5980d746710) 修正次级索引构建中断时临时文件清理的问题。此修正解决了守护进程在创建 `tmp.spidx` 文件时打开文件数超限的问题。
* [提交 709b](https://github.com/manticoresoftware/columnar/commit/709b9acaaac97d9a1ca8796892f9ad432021c785) 对列存和次级索引分别使用独立的 streamvbyte 库。
* [提交 1c26](https://github.com/manticoresoftware/manticoresearch/commit/1c2623763ced14bb94ad624dde9614a938408d89) 添加了列存储不支持 json 属性的警告。
* [提交 3acd](https://github.com/manticoresoftware/columnar/commit/3acd6714ffce103447d7931dfc75bb1a7449f340) 修正次级索引中的数据解包问题。
* [提交 574c](https://github.com/manticoresoftware/manticoresearch/commit/574c023152564fa72092c18ebd1f594abfd4b6d7) 修正保存包含行存与列存混合的磁盘块时的崩溃。
* [提交 e87f](https://github.com/manticoresoftware/columnar/commit/e87f088b458d4c5b1b6159fc72ed0d57b786891a) 修正次级索引迭代器被提示处理一个已处理过的块。
* [问题 #1474](https://github.com/manticoresoftware/manticoresearch/issues/1474) 行存 MVA 列与列存引擎更新功能损坏。
* [问题 #1510](https://github.com/manticoresoftware/manticoresearch/issues/1510) 修正对用于 `HAVING` 的列存属性进行聚合时的崩溃。
* [问题 #1519](https://github.com/manticoresoftware/manticoresearch/issues/1519) 修正 `expr` 排名器在使用列存属性时的崩溃。

### 与 Docker 相关
* ❗[问题 #42](https://github.com/manticoresoftware/docker/issues/42) 通过环境变量支持[纯索引（plain indexation）](https://github.com/manticoresoftware/docker#building-plain-tables)。
* ❗[问题 #47](https://github.com/manticoresoftware/docker/issues/47) 通过环境变量提升配置的灵活性。
* [问题 #54](https://github.com/manticoresoftware/docker/pull/54) 改进 Docker 的[备份与恢复](https://github.com/manticoresoftware/docker#backup-and-restore)流程。
* [问题 #77](https://github.com/manticoresoftware/docker/issues/76) 改进入口脚本，只在首次启动时处理备份恢复。
* [提交 a27c](https://github.com/manticoresoftware/docker/commit/a27c048dd9e73d8d6bea8d9a4830cafb486b82c5) 修正查询日志输出到 stdout。
* [问题 #38](https://github.com/manticoresoftware/docker/issues/38) 当 EXTRA 未设置时，屏蔽 BUDDY 警告。
* [问题 #71](https://github.com/manticoresoftware/docker/pull/71) 修正 `manticore.conf.sh` 中的主机名。

## 版本 6.2.12
发布时间：2023年8月23日

版本 6.2.12 延续 6.2 系列，修复发布 6.2.0 后发现的问题。

### Bug 修复
* ❗[问题 #1351](https://github.com/manticoresoftware/manticoresearch/issues/1351) “Manticore 6.2.0 在 Centos 7 上无法通过 systemctl 启动”：将 `TimeoutStartSec` 从 `infinity` 修改为 `0`，以提高在 Centos 7 上的兼容性。
* ❗[问题 #1364](https://github.com/manticoresoftware/manticoresearch/issues/1364) “从 6.0.4 升级到 6.2.0 后崩溃”：为旧版本 binlog 的空日志文件添加重放功能。
* [PR #1334](https://github.com/manticoresoftware/manticoresearch/pull/1334) “修正 searchdreplication.cpp 中的拼写错误”：将 beggining 改为 beginning。
* [问题 #1337](https://github.com/manticoresoftware/manticoresearch/issues/1337) “Manticore 6.2.0 警告：conn (local)(12), sock=8088: bailing on failed MySQL header, AsyncNetInputBuffer_c::AppendData: error 11 (资源暂时不可用) 返回 -1”：将 MySQL 接口关于头部的警告日志级别调低至 logdebugv。
* [问题 #1355](https://github.com/manticoresoftware/manticoresearch/issues/1355) “节点地址无法解析时加入集群挂起”：改进当部分节点不可达且名称解析失败时的复制重试，解决 Kubernetes 和 Docker 节点中的复制相关问题。增强复制启动失败的错误消息并更新测试模型 376。此外，提供了名称解析失败的明确错误提示。
* [Issue #1361](https://github.com/manticoresoftware/manticoresearch/issues/1361) "No lower case mapping for "Ø" in charset non_cjk": 调整了字符 'Ø' 的映射。
* [Issue #1365](https://github.com/manticoresoftware/manticoresearch/issues/1365) "searchd leaves binlog.meta and binlog.001 after clean stop": 确保最后一个空的 binlog 文件被正确移除。
* [Commit 0871](https://github.com/manticoresoftware/manticoresearch/commit/0871070845e93ad702ae0dfb89b35e5f24cd0851): 修复了 Windows 上 `Thd_t` 构建中与原子复制限制相关的问题。
* [Commit 1cc0](https://github.com/manticoresoftware/manticoresearch/commit/1cc04bedc174737118fd5a3663e35f4e95bd4f8c): 解决了 FT CBO 与 `ColumnarScan` 相关的问题。
* [Commit c6bf](https://github.com/manticoresoftware/manticoresearch/commit/c6bfb30d53e3f4b67f17769667733677a661c95b): 修正了测试 376 并在测试中添加了 `AF_INET` 错误的替代处理。
* [Commit cbc3](https://github.com/manticoresoftware/manticoresearch/commit/cbc38705e0b4dcc9665e3e8cf64826290ebd11cf): 解决了复制过程中更新 blob 属性与替换文档时的死锁问题。同时，移除了提交时索引的 rlock，因为它已经在更基础的层面被锁定。

### 小改动
* [Commit 4f91](https://github.com/manticoresoftware/manticoresearch/commit/4f913a36e84e01f950b2797051222b0b1094d6fe) 更新了手册中 `/bulk` 端点的信息。

### MCL
* 支持 [Manticore Columnar Library](https://github.com/manticoresoftware/columnar) v2.2.4

## 版本 6.2.0
发布时间：2023 年 8 月 4 日

### 主要变更
* 查询优化器得到了增强，现支持全文查询，显著提升了搜索效率和性能。
* 集成：
  - [mysqldump](https://dev.mysql.com/doc/refman/8.0/en/mysqldump.html)——使用`mysqldump`进行[逻辑备份](Securing_and_compacting_a_table/Backup_and_restore.md#Backup-and-restore-with-mysqldump)
  - [Apache Superset](https://manticoresearch.com/blog/manticoresearch-apache-superset-integration/) 和 [Grafana](https://manticoresearch.com/blog/manticoresearch-grafana-integration/) 用于可视化存储在 Manticore 中的数据
  - [HeidiSQL](https://www.heidisql.com/) 和 [DBForge](https://www.devart.com/dbforge/) 方便与 Manticore 的开发
* 我们开始使用 [GitHub workflows](https://github.com/manticoresoftware/manticoresearch/actions)，使贡献者更容易使用与核心团队准备软件包时相同的持续集成（CI）流程。所有作业均可在 GitHub 托管的 runner 上运行，便于无缝测试你 Fork 的 Manticore Search 修改。
* 我们开始使用 [CLT](https://github.com/manticoresoftware/clt) 测试复杂场景。例如，现在能够确保提交后构建的软件包能在所有支持的 Linux 操作系统上正确安装。命令行测试器（CLT）提供了一种以交互模式录制测试并轻松回放的用户友好方式。
* 通过结合使用哈希表和 HyperLogLog，显著提升了 count distinct 操作的性能。
* 支持对包含二级索引的查询进行多线程执行，线程数限制为物理 CPU 核心数量。这将大幅提升查询执行速度。
* 对 `pseudo_sharding` 进行了调整，使其限制在可用的空闲线程数量内。这一更新显著提升了吞吐量性能。
* 用户现在可以通过配置设置指定[默认属性存储引擎](Server_settings/Searchd.md#engine)，以更好地匹配特定工作负载需求。
* 支持 [Manticore Columnar Library 2.2.0](https://github.com/manticoresoftware/columnar/)，二级索引方面包含众多 bug 修复和改进[Secondary indexes](Server_settings/Searchd.md#secondary_indexes)。

### 小改动
* [Buddy #153](https://github.com/manticoresoftware/manticoresearch-buddy/issues/153): [/pq](Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md?client=JSON#Adding-rules-to-a-percolate-table) HTTP 端点现在作为 `/json/pq` HTTP 端点的别名。
* [Commit 0bf1](https://github.com/manticoresoftware/manticoresearch/commit/0bf17d9e): 确保了 `upper()` 和 `lower()` 的多字节兼容性。
* [Commit 2bb9](https://github.com/manticoresoftware/manticoresearch/commit/2bb92765): 对于 `count(*)` 查询，不再扫描索引，而是返回预先计算的值。
* [Commit 3c84](https://github.com/manticoresoftware/manticoresearch/commit/3c84091f): 现在可以使用 `SELECT` 进行任意计算并显示 `@@sysvars`。与之前不同，不再限于仅做一次计算。因此，类似于 `select user(), database(), @@version_comment, version(), 1+1 as a limit 10` 的查询将返回所有列。请注意，'limit' 可选参数将始终被忽略。
* [Commit 6aca](https://github.com/manticoresoftware/manticoresearch/commit/6aca32f0): 实现了 `CREATE DATABASE` 伪查询。
* [Commit 9dc1](https://github.com/manticoresoftware/manticoresearch/commit/9dc12334): 执行 `ALTER TABLE table REBUILD SECONDARY` 时，即使属性未更新，二级索引也总会被重建。
* [Commit 46ed](https://github.com/manticoresoftware/manticoresearch/commit/46edb089): 在使用 CBO 之前，现在会检测使用了预计算数据的排序器，以避免不必要的 CBO 计算。
* [Commit 102a](https://github.com/manticoresoftware/manticoresearch/commit/102ac604): 实现了全文表达式栈的模拟和使用，防止守护进程崩溃。
* [Commit 979f](https://github.com/manticoresoftware/manticoresearch/commit/979fa27c): 为不使用字符串/mvas/json 属性的匹配，添加了匹配克隆代码的快速路径。
* [提交 a073](https://github.com/manticoresoftware/manticoresearch/commit/a0735ffe): 添加了对 `SELECT DATABASE()` 命令的支持。但它始终返回 `Manticore`。此新增功能对于与各种 MySQL 工具的集成至关重要。
* [提交 bc04](https://github.com/manticoresoftware/manticoresearch/commit/bc04908): 修改了 [/cli](Connecting_to_the_server/HTTP.md#/cli) 端点的响应格式，并新增了 `/cli_json` 端点以实现之前 `/cli` 的功能。
* [提交 d70b](https://github.com/manticoresoftware/manticoresearch/commit/d70b0d58): 现在可以通过 `SET` 语句在运行时更改 `thread_stack`。同时支持会话本地和守护进程全局两种方式。当前值可在 `show variables` 输出中查看。
* [提交 d96e](https://github.com/manticoresoftware/manticoresearch/commit/d96ec6b9): 代码已集成到 CBO 中，以更准确地估算字符串属性上执行过滤器的复杂度。
* [提交 e77d](https://github.com/manticoresoftware/manticoresearch/commit/e77dd72f5a04531c352fad0d7afcd2a1cbae2510): 改进了 DocidIndex 成本计算，提升了整体性能。
* [提交 f3ae](https://github.com/manticoresoftware/manticoresearch/commit/f3ae8bea): 类似于 Linux 上的“uptime”，负载指标现在可通过 `SHOW STATUS` 命令查看。
* [提交 f3cc](https://github.com/manticoresoftware/manticoresearch/commit/f3cc0971): `DESC` 和 `SHOW CREATE TABLE` 的字段和属性顺序现与 `SELECT * FROM` 保持一致。
* [提交 f3d2](https://github.com/manticoresoftware/manticoresearch/commit/f3d248a6): 不同的内部解析器现在在出现各种错误时会提供其内部助记代码（如 `P01`）。此改进帮助识别导致错误的解析器，同时屏蔽不必要的内部细节。
* [问题 #271](https://github.com/manticoresoftware/manticoresearch/issues/271) “有时 CALL SUGGEST 不会建议纠正单字母拼写错误”：改进了 [SUGGEST/QSUGGEST](Searching/Spell_correction.md#CALL-QSUGGEST,-CALL-SUGGEST) 对短词的表现：添加了选项 `sentence` 以显示整句内容。
* [问题 #696](https://github.com/manticoresoftware/manticoresearch/issues/696) “启用词干处理后 Percolate 索引无法正确进行精确短语查询”：修改了 percolate 查询以支持精确词条修饰符，提升搜索功能。
* [问题 #829](https://github.com/manticoresoftware/manticoresearch/issues/829) “日期格式化方法”：新增了 [date_format()](Functions/Date_and_time_functions.md#DATE_FORMAT()) 查询列表表达式，暴露了 `strftime()` 函数。
* [问题 #961](https://github.com/manticoresoftware/manticoresearch/issues/961) “通过 HTTP JSON API 对桶进行排序”：在 HTTP 接口的聚合桶中引入了一个可选的 [sort 属性](Searching/Faceted_search.md#HTTP-JSON)。
* [问题 #1062](https://github.com/manticoresoftware/manticoresearch/issues/1062) “改进 JSON 插入 API 失败的错误日志——‘不支持的值类型’”：当发生错误时，`/bulk` 端点会报告已处理和未处理字符串（文档）的数量信息。
* [问题 #1070](https://github.com/manticoresoftware/manticoresearch/issues/1070) “CBO 提示不支持多个属性”：启用索引提示以处理多个属性。
* [问题 #1106](https://github.com/manticoresoftware/manticoresearch/issues/1106) “为 HTTP 搜索查询添加标签”：为 [HTTP PQ 响应](Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md?client=JSON#Adding-rules-to-a-percolate-table) 添加了标签。
* [问题 #1301](https://github.com/manticoresoftware/manticoresearch/issues/1301) “buddy 不应并行创建表”：修复了导致并行 CREATE TABLE 操作失败的问题。现在一次仅允许一个 `CREATE TABLE` 操作运行。
* [问题 #1303](https://github.com/manticoresoftware/manticoresearch/issues/1303) “添加对列名中 @ 符号的支持”。
* [问题 #1316](https://github.com/manticoresoftware/manticoresearch/issues/1316) “在 taxi 数据集上 ps=1 的查询速度较慢”：改进了 CBO 逻辑，默认直方图分辨率设为 8k，以提高属性随机分布值的准确度。
* [问题 #1317](https://github.com/manticoresoftware/manticoresearch/issues/1317) “修正 CBO 与 hn 数据集上的全文搜索冲突”：改进了逻辑，以确定何时使用位图迭代器交集，何时使用优先队列。
* [问题 #1318](https://github.com/manticoresoftware/manticoresearch/issues/1318) “列式：将迭代器接口改为单次调用”：列式迭代器现在使用单次 `Get` 调用，取代了之前的两步 `AdvanceTo` + `Get` 调用获取值。
* [问题 #1319](https://github.com/manticoresoftware/manticoresearch/issues/1319) “聚合计算加速（移除 CheckReplaceEntry？）”：从分组排序器中移除了 `CheckReplaceEntry` 调用，以加速聚合函数计算。
* [问题 #1320](https://github.com/manticoresoftware/manticoresearch/issues/1320) “create table read_buffer_docs/hits 不识别 k/m/g 语法”：`CREATE TABLE` 选项 `read_buffer_docs` 和 `read_buffer_hits` 现支持 k/m/g 语法。
* [语言包](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) 对英语、德语和俄语的支持现已简单化，在 Linux 上执行命令 `apt/yum install manticore-language-packs` 即可安装。在 macOS 上，使用命令 `brew install manticoresoftware/tap/manticore-language-packs`。
* 字段和属性顺序现在在 `SHOW CREATE TABLE` 和 `DESC` 操作之间保持一致。
* 当执行 `INSERT` 查询时磁盘空间不足，新提交的 `INSERT` 查询将失败，直至磁盘空间足够。
* 新增了 [UINT64()](Functions/Type_casting_functions.md#UINT64%28%29) 类型转换函数。
* `/bulk` 端点现在将空行处理为一个 [commit](Data_creation_and_modification/Transactions.md#BEGIN,-COMMIT,-and-ROLLBACK) 命令。更多信息见 [这里](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md?client=JSON#Bulk-adding-documents)。
* 已为 [无效索引提示](Searching/Options.md#Query-optimizer-hints) 实现警告，提高了透明度并允许错误缓解。
* 当使用带有单个过滤器的 `count(*)` 时，查询现在在可用时利用来自二级索引的预计算数据，大幅加快了查询速度。

### ⚠️ 不兼容变更
* ⚠️ 在版本 6.2.0 中创建或修改的表不能被旧版本读取。
* ⚠️ 文档 ID 在索引和 INSERT 操作时现在作为无符号 64 位整数处理。
* ⚠️ 查询优化器提示的语法已更新。新格式为 `/*+ SecondaryIndex(uid) */`。请注意，旧语法不再支持。
* ⚠️ [问题 #1160](https://github.com/manticoresoftware/manticoresearch/issues/1160)：为避免语法冲突，禁止在表名中使用 `@`。
* ⚠️ 标记为 `indexed` 和 `attribute` 的字符串字段/属性在 `INSERT`、`DESC` 和 `ALTER` 操作中现在被视为一个单独字段。
* ⚠️ [问题 #1057](https://github.com/manticoresoftware/manticoresearch/issues/1057)：不支持 SSE 4.2 的系统将无法加载 MCL 库。
* ⚠️ [问题 #1143](https://github.com/manticoresoftware/manticoresearch/issues/1143)：[agent_query_timeout](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_query_timeout) 已修复，现已生效。

### Bug修复
* [Commit 2a6e](https://github.com/manticoresoftware/manticoresearch/commit/2a6ea8f7) “DROP TABLE 崩溃”：解决了执行 DROP TABLE 语句时，RT 表写入操作（优化、磁盘分块保存）长时间等待完成的问题。添加了在执行 DROP TABLE 后表目录非空时的警告。
* [Commit 2ebd](https://github.com/manticoresoftware/manticoresearch/commit/2ebd424d)：新增了对列式属性支持，此前用于多属性分组的代码缺少此支持。
* [Commit 3be4](https://github.com/manticoresoftware/manticoresearch/commit/3be4503f752216b37b8cde8e47fd2d77bbce63ae) 解决了因磁盘空间耗尽可能导致的崩溃问题，通过正确处理 binlog 写入错误实现。
* [Commit 6adb](https://github.com/manticoresoftware/manticoresearch/commit/6adb0934)：修复了使用多个列式扫描迭代器（或二级索引迭代器）查询时偶发的崩溃问题。
* [Commit 6bd9](https://github.com/manticoresoftware/manticoresearch/commit/6bd9f709)：使用利用预计算数据的排序器时，过滤器未被移除的问题已修复。
* [Commit 6d03](https://github.com/manticoresoftware/manticoresearch/commit/6d03566a)：更新了 CBO 代码，以提供对多线程执行的基于行属性过滤器查询的更好估计。
* [Commit 6dd3](https://github.com/manticoresoftware/manticoresearch/commit/6dd3964), [Helm #56](https://github.com/manticoresoftware/manticoresearch-helm/issues/56) “Kubernetes 集群中的严重崩溃转储”：修复了 JSON 根对象的损坏布隆过滤器；修复了因 JSON 字段过滤导致的守护进程崩溃。
* [Commit 6e1b](https://github.com/manticoresoftware/manticoresearch/commit/6e1b849) 修正了因无效的 `manticore.json` 配置造成的守护进程崩溃。
* [Commit 6fbc](https://github.com/manticoresoftware/manticoresearch/commit/6fbc4189) 修复 json 范围过滤支持 int64 值。
* [Commit 9c67](https://github.com/manticoresoftware/manticoresearch/commit/9c677426) 修复了 `.sph` 文件在 `ALTER` 操作中可能损坏的问题。
* [Commit 77cc](https://github.com/manticoresoftware/manticoresearch/commit/77ccd8f)：新增了替换语句复制的共享密钥，解决了从多个主节点复制替换时出现的 `pre_commit` 错误。
* [Commit 2884](https://github.com/manticoresoftware/manticoresearch/commit/2884519e) 解决了 bigint 检查对类似 'date_format()' 函数的问题。
* [Commit 9513](https://github.com/manticoresoftware/manticoresearch/commit/9513aca5)：当排序器使用预计算数据时，迭代器不再显示在 [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META)。
* [Commit a2a7](https://github.com/manticoresoftware/manticoresearch/commit/a2a70555)：更新全文节点堆栈大小，防止复杂全文查询崩溃。
* [Commit a062](https://github.com/manticoresoftware/manticoresearch/commit/a0626d7e)：修复了带 JSON 和字符串属性的更新复制时导致崩溃的错误。
* [Commit b3e6](https://github.com/manticoresoftware/manticoresearch/commit/b3e63e5d)：字符串构建器更新为使用 64 位整数，避免处理大数据集时崩溃。
* [Commit c472](https://github.com/manticoresoftware/manticoresearch/commit/c472e5b)：解决了跨多个索引的 count distinct 导致崩溃的问题。
* [Commit d073](https://github.com/manticoresoftware/manticoresearch/commit/d0730272)：修复了即使禁用 `pseudo_sharding` 也能在多个线程上执行 RT 索引磁盘分块查询的问题。
* [Commit d205](https://github.com/manticoresoftware/manticoresearch/commit/d205508e) 修改了 `show index status` 命令返回的值集，现视索引类型而异。
* [Commit e9bc](https://github.com/manticoresoftware/manticoresearch/commit/e9bc2f9) 修复了处理批量请求时的 HTTP 错误及网络循环未向客户端返回错误的问题。
* [Commit f77c](https://github.com/manticoresoftware/manticoresearch/commit/f77ce0e6) 为 PQ 使用了扩展堆栈。
* [提交 fac2](https://github.com/manticoresoftware/manticoresearch/commit/fac2d9687a20582765aaa89be9597804c4f845af) 更新了导出排序器输出，使其与 [packedfactors()](Functions/Searching_and_ranking_functions.md#PACKEDFACTORS%28%29) 保持一致。
* [提交 ff87](https://github.com/manticoresoftware/manticoresearch/commit/ff87cc4): 修复了 SphinxQL 查询日志中过滤器中的字符串列表问题。
* [问题 #589](https://github.com/manticoresoftware/manticoresearch/issues/589) “字符集定义似乎依赖于代码的顺序”：修正了重复字符集映射错误。
* [问题 #811](https://github.com/manticoresoftware/manticoresearch/issues/811) “词形多词映射干扰带有 CJK 标点的关键词间短语搜索”：修复了带有词形的短语查询中 ngram 令牌的位置问题。
* [问题 #834](https://github.com/manticoresoftware/manticoresearch/issues/834) “搜索查询中的等号导致请求失败”：确保精确符号可以被转义，并修复了 `expand_keywords` 选项导致的双重精确扩展。
* [问题 #864](https://github.com/manticoresoftware/manticoresearch/issues/864) “exceptions/stopwords 冲突”。
* [问题 #910](https://github.com/manticoresoftware/manticoresearch/issues/910) “调用 call snippets() 时带 libstemmer_fr 和 index_exact_words 导致 Manticore 崩溃”：解决了调用 `SNIPPETS()` 时导致崩溃的内部冲突。
* [问题 #946](https://github.com/manticoresoftware/manticoresearch/issues/946) “SELECT 时出现重复记录”：修复了带有 `not_terms_only_allowed` 选项的查询在 RT 索引中含已删除文档时出现重复文档的问题。
* [问题 #967](https://github.com/manticoresoftware/manticoresearch/issues/967) “在 UDF 函数中使用 JSON 参数导致崩溃”：修正了启用伪分片且使用带 JSON 参数的 UDF 处理搜索时的守护进程崩溃。
* [问题 #1050](https://github.com/manticoresoftware/manticoresearch/issues/1050) “FEDERATED 中的 count(*)”：修复了通过 `FEDERATED` 引擎带聚合查询时导致守护进程崩溃的问题。
* [问题 #1052](https://github.com/manticoresoftware/manticoresearch/issues/1052) 修正了 `rt_attr_json` 列与列存储不兼容的问题。
* [问题 #1072](https://github.com/manticoresoftware/manticoresearch/issues/1072) “搜索查询中的 * 被 ignore_chars 移除”：修复了查询中的通配符不受 `ignore_chars` 影响的问题。
* [问题 #1075](https://github.com/manticoresoftware/manticoresearch/issues/1075) “indextool --check 遇到 distributed 表失败”：indextool 现兼容 JSON 配置中包含 'distributed' 和 'template' 索引的实例。
* [问题 #1081](https://github.com/manticoresoftware/manticoresearch/issues/1081) “对特定 RT 数据集的特定选择导致 searchd 崩溃”：解决了带 packedfactors 及大型内部缓冲区查询时守护进程崩溃问题。
* [问题 #1095](https://github.com/manticoresoftware/manticoresearch/issues/1095) “使用 not_terms_only_allowed 时已删除文档被忽略”。
* [问题 #1099](https://github.com/manticoresoftware/manticoresearch/issues/1099) “indextool --dumpdocids 不工作”：恢复了 `--dumpdocids` 命令功能。
* [问题 #1100](https://github.com/manticoresoftware/manticoresearch/issues/1100) “indextool --buildidf 不工作”：indextool 在完成 globalidf 后现在关闭了文件。
* [问题 #1104](https://github.com/manticoresoftware/manticoresearch/issues/1104) “Count(*) 被错误当作远程表中的 schema set 处理”：解决了当代理返回空结果集时守护进程发送错误消息的问题。
* [问题 #1109](https://github.com/manticoresoftware/manticoresearch/issues/1109) “使用 threads=1 时 FLUSH ATTRIBUTES 会挂起”。
* [问题 #1126](https://github.com/manticoresoftware/manticoresearch/issues/1126) “查询时与 MySQL 服务器连接丢失 - manticore 6.0.5”：解决了对列存储属性多重过滤时导致的崩溃。
* [问题 #1135](https://github.com/manticoresoftware/manticoresearch/issues/1135) “JSON 字符串过滤的大小写敏感问题”：修正了用于 HTTP 搜索请求中的过滤的排序规则。
* [问题 #1140](https://github.com/manticoresoftware/manticoresearch/issues/1140) “错误字段匹配”：修复了与 `morphology_skip_fields` 相关的问题。
* [问题 #1155](https://github.com/manticoresoftware/manticoresearch/issues/1155) “通过 API 的系统远程命令应传递 g_iMaxPacketSize”：更新以跳过节点间复制命令的 `max_packet_size` 检查，并新增了最新集群错误至状态显示。
* [问题 #1302](https://github.com/manticoresoftware/manticoresearch/issues/1302) “失败的优化过程中遗留临时文件”：修正了合并或优化过程出错后遗留临时文件的问题。
* [问题 #1304](https://github.com/manticoresoftware/manticoresearch/issues/1304) “为 buddy 启动超时添加环境变量”：添加了环境变量 `MANTICORE_BUDDY_TIMEOUT`（默认3秒）以控制守护进程启动时等待 buddy 消息的时间。
* [问题 #1305](https://github.com/manticoresoftware/manticoresearch/issues/1305) “保存 PQ 元数据时整数溢出”：缓解了守护进程保存大型 PQ 索引时的过度内存消耗。
* [问题 #1306](https://github.com/manticoresoftware/manticoresearch/issues/1306) “更改外部文件后无法重新创建 RT 表”：纠正了外部文件为空字符串时的 alter 错误；修复了更改外部文件后 RT 索引外部文件残留问题。
* [问题 #1307](https://github.com/manticoresoftware/manticoresearch/issues/1307) “SELECT 语句中 sum(value) as value 工作不正常”：修复了带别名的选择列表表达式遮盖索引属性的问题；并修正了整数类型中 sum 转换为 int64 时的问题。
* [Issue #1308](https://github.com/manticoresoftware/manticoresearch/issues/1308) “避免在复制中绑定到 localhost”：确保复制不会对具有多个 IP 的主机名绑定到 localhost。
* [Issue #1309](https://github.com/manticoresoftware/manticoresearch/issues/1309) “对超过 16Mb 数据的 mysql 客户端回复失败”：修复了向客户端返回大于 16Mb 的 SphinxQL 数据包的问题。
* [Issue #1310](https://github.com/manticoresoftware/manticoresearch/issues/1310) “‘指向外部文件的路径应为绝对路径’中的错误引用”：修正了 `SHOW CREATE TABLE` 中外部文件完整路径的显示。
* [Issue #1311](https://github.com/manticoresoftware/manticoresearch/issues/1311) “调试构建在片段中遇到长字符串时崩溃”：现在允许在 `SNIPPET()` 函数目标文本中使用长度超过255个字符的长字符串。
* [Issue #1312](https://github.com/manticoresoftware/manticoresearch/issues/1312) “kqueue 轮询中用后删除导致的错误崩溃（master-agent）”：修复了在基于 kqueue 的系统（FreeBSD、MacOS 等）上，主节点无法连接代理时导致的崩溃。
* [Issue #1313](https://github.com/manticoresoftware/manticoresearch/issues/1313) “连接自身时间过长”：从主节点连接到 MacOS/BSD 上的代理时，现使用统一的连接+查询超时，而不仅是连接超时。
* [Issue #1314](https://github.com/manticoresoftware/manticoresearch/issues/1314) “pq（json 元数据）中未达到的嵌入同义词加载失败”：修复了 pq 中嵌入同义词的标记。
* [Issue #1315](https://github.com/manticoresoftware/manticoresearch/issues/1315) “允许某些函数（sint, fibonacci, second, minute, hour, day, month, year, yearmonth, yearmonthday）使用隐式提升的参数值”。
* [Issue #1321](https://github.com/manticoresoftware/manticoresearch/issues/1321) “启用全扫描中的多线程 SI，但限制线程数”：已在 CBO 中实现代码，更好地预测在全文查询中使用二级索引时多线程性能。
* [Issue #1322](https://github.com/manticoresoftware/manticoresearch/issues/1322) “使用预计算排序器后，count(*) 查询仍然缓慢”：采用使用预计算数据的排序器时，不再启动迭代器，避免了性能下降。
* [Issue #1411](https://github.com/manticoresoftware/manticoresearch/issues/1141) “sphinxql 中的查询日志未保存 MVA 原始查询”：现在记录了 `all()/any()`。

## 版本 6.0.4
发布时间：2023年3月15日

### 新功能
* 改进了与 Logstash、Beats 等的集成，包括：
  - 支持 Logstash 版本 7.6 - 7.15，Filebeat 版本 7.7 - 7.12
  - 自动模式支持。
  - 添加了对类似 Elasticsearch 格式的批量请求处理。
* [Buddy 提交 ce90](https://github.com/manticoresoftware/manticoresearch-buddy/commit/ce907ea) 在 Manticore 启动时记录 Buddy 版本。

### Bug 修复
* [Issue #588](https://github.com/manticoresoftware/manticoresearch/issues/588), [Issue #942](https://github.com/manticoresoftware/manticoresearch/issues/942) 修复了双字节索引搜索元数据及关键词调用中的错误字符。
* [Issue #1027](https://github.com/manticoresoftware/manticoresearch/issues/1027) 拒绝小写的 HTTP 头。
* ❗[Issue #1039](https://github.com/manticoresoftware/manticoresearch/issues/1039) 修复了读取 Buddy 控制台输出时守护进程的内存泄漏。
* [Issue #1056](https://github.com/manticoresoftware/manticoresearch/issues/1056) 修复了问号的异常行为。
* [Issue #1064](https://github.com/manticoresoftware/manticoresearch/issues/1064) - 修复了分词器小写表中的竞态条件导致崩溃。
* [Commit 59bb](https://github.com/manticoresoftware/manticoresearch/commit/59bb54c) 修复了 JSON 接口中显式设置 id 为 null 的文档批量写入处理。
* [Commit 7b6b](https://github.com/manticoresoftware/manticoresearch/commit/7b6b25f) 修复了 CALL KEYWORDS 中多次相同术语的术语统计。
* [Commit f381](https://github.com/manticoresoftware/manticoresearch/commit/f381ad2) Windows 安装程序现在创建默认配置；路径不再在运行时替换。
* [Commit 6940](https://github.com/manticoresoftware/manticoresearch/commit/6940e95), [Commit cc5a](https://github.com/manticoresoftware/manticoresearch/commit/cc5a480) 修复了集群中节点跨多个网络的复制问题。
* [Commit 4972](https://github.com/manticoresoftware/manticoresearch/commit/49722ab) 修复了 `/pq` HTTP 端点，应作为 `/json/pq` HTTP 端点的别名。
* [Commit 3b53](https://github.com/manticoresoftware/manticoresearch/commit/3b5385a) 修复了 Buddy 重启时守护进程崩溃。
* [Buddy 提交 fba9](https://github.com/manticoresoftware/manticoresearch-buddy/commit/fba9c8c) 收到无效请求时显示原始错误。
* [Buddy 提交 db95](https://github.com/manticoresoftware/manticoresearch-buddy/commit/db9532c) 允许备份路径中包含空格，并给正则表达式添加了一些魔法以支持单引号。

## 版本 6.0.2
发布时间：2023年2月10日

### Bug 修复
* [Issue #1024 crash 2](https://github.com/manticoresoftware/manticoresearch/issues/1024) 分面搜索结果较多时的崩溃/段错误
* ❗[Issue #1029](https://github.com/manticoresoftware/manticoresearch/issues/1029) - 警告：编译时值 KNOWN_CREATE_SIZE (16) 小于测量值 (208)。建议修正该值！
* ❗[Issue #1032](https://github.com/manticoresoftware/manticoresearch/issues/1032) - Manticore 6.0.0 普通索引崩溃
* ❗[Issue #1033](https://github.com/manticoresoftware/manticoresearch/issues/1033) - 守护进程重启时多个分布式索引丢失
* ❗[Issue #1064](https://github.com/manticoresoftware/manticoresearch/issues/1064) - 分词器小写表中的竞态条件

## 版本 6.0.0
发布时间：2023年2月7日

从此版本开始，Manticore Search 配备了 Manticore Buddy，这是一个用 PHP 编写的 sidecar 守护进程，负责处理不需要极低延迟或高吞吐量的高级功能。Manticore Buddy 在幕后运行，您甚至可能不会注意它的存在。虽然对最终用户来说它是隐形的，但使 Manticore Buddy 易于安装且与基于 C++ 的主守护进程兼容是一个重大挑战。这个重大变革将允许团队开发各种新的高级功能，比如分片编排、访问控制和身份验证以及各种集成，如 mysqldump、DBeaver、Grafana mysql 连接器。目前它已经处理了 [SHOW QUERIES](Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES)、[BACKUP](Securing_and_compacting_a_table/Backup_and_restore.md#BACKUP-SQL-command-reference) 和 [自动模式](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema)。

本版本还包含了 130 多个错误修复和众多功能，其中许多功能可以视为重大更新。

### 重大变更
* 🔬 实验性功能：现在您可以执行 Elasticsearch 兼容的 [insert](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md?example=Elasticsearch#Adding-documents-to-a-real-time-table) 和 [replace](Data_creation_and_modification/Updating_documents/REPLACE.md?example=Elasticsearch#REPLACE) JSON 查询，这使得可以使用像 Logstash（版本 < 7.13）、Filebeat 和 Beats 系列的其他工具与 Manticore 结合使用。默认启用。您可以使用 `SET GLOBAL ES_COMPAT=off` 禁用此功能。
* 支持 [Manticore Columnar Library 2.0.0](https://github.com/manticoresoftware/columnar/) ，在[二级索引](Server_settings/Searchd.md#secondary_indexes)中有大量修复和改进。**⚠️ 重大变更**：从本版本开始，二级索引默认开启。如果您从 Manticore 5 升级，请务必执行[ALTER TABLE table_name REBUILD SECONDARY](Updating_table_schema_and_settings.md#Rebuilding-a-secondary-index)。详情请见下文。
* [Commit c436](https://github.com/manticoresoftware/manticoresearch/commit/c436f9023536f767610451911955ae36d90aa638) 自动模式：现在您可以跳过创建表的步骤，只需插入第一条文档，Manticore 将根据其字段自动创建表。详细信息请见 [这里](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema)。您可以通过 [searchd.auto_schema](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema) 开启或关闭此功能。
* 对[基于成本的优化器](Searching/Cost_based_optimizer.md)进行了大幅重构，在许多情况下降低了查询响应时间。
  - [Issue #1008](https://github.com/manticoresoftware/manticoresearch/issues/1008) 基于成本优化器中的并行性能估计。
  - [Issue #1014](https://github.com/manticoresoftware/manticoresearch/issues/1014) 基于成本优化器现在可以识别[二级索引](Server_settings/Searchd.md#secondary_indexes)并能做出更智能的判断。
  - [Commit cef9](https://github.com/manticoresoftware/columnar/commit/cef9b54dc3a4ee9138ebc4df3a671715b996d6b3) 列式表/字段的编码统计信息现存储于元数据中，以帮助基于成本的优化器做出更智能的决策。
  - [Commit 2b95](https://github.com/manticoresoftware/manticoresearch/commit/2b95fb8c2ba422c9c5a66e2b61c1f7922f31c7aa) 新增基于成本优化器提示（CBO hints）以细致调整其行为。
* [遥测功能](Telemetry.md#Telemetry)：我们很高兴在本版本中新增遥测功能。此功能允许我们收集匿名且去标识化的度量数据，帮助我们改进产品的性能和用户体验。请放心，所有收集的数据**完全匿名，不会关联任何个人信息**。如果需要，可以在设置中[轻松关闭](Telemetry.md#Telemetry)该功能。
* [Commit 5aaf](https://github.com/manticoresoftware/manticoresearch/commit/5aafb17d81b601e399e212a344a4738f65576cdd) 新增[ALTER TABLE table_name REBUILD SECONDARY](Updating_table_schema_and_settings.md#Rebuilding-a-secondary-index) 命令，可在任意时间重建二级索引，例如：
  - 从 Manticore 5 迁移到新版本时，
  - 当您对索引中的某个属性进行了 [UPDATE](Data_creation_and_modification/Updating_documents/UPDATE.md#UPDATE)（即[就地更新，而非替换](Data_creation_and_modification/Updating_documents/REPLACE_vs_UPDATE.md)）
* [Issue #821](https://github.com/manticoresoftware/manticoresearch/issues/821) 新工具 `manticore-backup` 用于[备份和恢复 Manticore 实例](Securing_and_compacting_a_table/Backup_and_restore.md)
* SQL 命令 [BACKUP](Securing_and_compacting_a_table/Backup_and_restore.md#BACKUP-SQL-command-reference) 可在 Manticore 内部执行备份。
* SQL 命令 [SHOW QUERIES](Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES) 作为查看正在运行查询的简便方法，替代查看线程。
* [Issue #551](https://github.com/manticoresoftware/manticoresearch/issues/551) SQL 命令 `KILL` 用于终止长时间运行的 `SELECT` 查询。
* 聚合查询支持动态调整 `max_matches`，以提高精度并降低响应时间。

### 小变更
* [Issue #822](https://github.com/manticoresoftware/manticoresearch/issues/822) SQL 命令 [FREEZE/UNFREEZE](Securing_and_compacting_a_table/Freezing_a_table.md) 用于为备份准备实时/普通表。
* [Commit c470](https://github.com/manticoresoftware/manticoresearch/commit/c47052f18ad4868134de1a97e8d7da570e531dfa) 新增设置 `accurate_aggregation` 和 `max_matches_increase_threshold` 以精细控制聚合准确性。
* [Issue #718](https://github.com/manticoresoftware/manticoresearch/issues/718) 支持有符号的负 64 位 ID。注意，仍然无法使用大于 2^63 的 ID，但现在可以使用从 -2^63 到 0 范围内的 ID。
* 由于我们最近添加了对二级索引的支持，“index”一词可能指二级索引、全文索引或普通/实时 `index`，因此可能会引起混淆。为减少混淆，我们将后者重命名为“table”。以下 SQL/命令行命令受此更改影响。其旧版本已弃用，但仍然有效：
  - `index <table name>` => `table <table name>`，
  - `searchd -i / --index` => `searchd -t / --table`，
  - `SHOW INDEX STATUS` => `SHOW TABLE STATUS`，
  - `SHOW INDEX SETTINGS` => `SHOW TABLE SETTINGS`，
  - `FLUSH RTINDEX` => `FLUSH TABLE`，
  - `OPTIMIZE INDEX` => `OPTIMIZE TABLE`，
  - `ATTACH TABLE plain TO RTINDEX rt` => `ATTACH TABLE plain TO TABLE rt`，
  - `RELOAD INDEX` => `RELOAD TABLE`，
  - `RELOAD INDEXES` => `RELOAD TABLES`。

  我们不计划废弃旧的形式，但为确保与文档兼容，建议在应用程序中更改名称。未来版本中将更改各种 SQL 和 JSON 命令输出中的“index”为“table”。
* 带有状态 UDF 的查询现在被强制在单线程中执行。
* [Issue #1011](https://github.com/manticoresoftware/manticoresearch/issues/1011) 对与时间调度相关的所有内容进行了重构，为并行分块合并做准备。
* **⚠️ 重大变更**：列存储格式已更改。您需要重建具有列存属性的那些表。
* **⚠️ 重大变更**：二级索引文件格式已更改，因此如果您使用二级索引进行搜索，并且在配置文件中有 `searchd.secondary_indexes = 1`，请注意新版本 Manticore **将跳过加载拥有二级索引的表**。建议操作如下：
  - 升级前将配置文件中的 `searchd.secondary_indexes` 改为 0。
  - 启动实例。Manticore 会加载表并发出警告。
  - 对每个索引执行 `ALTER TABLE <table name> REBUILD SECONDARY`，以重建二级索引。

  如果您运行的是复制集群，需要在所有节点上执行 `ALTER TABLE <table name> REBUILD SECONDARY`，或者按照 [此说明](Securing_and_compacting_a_table/Compacting_a_table.md#Optimizing-clustered-tables) 进行，只是将 `OPTIMIZE` 替换为 `ALTER .. REBUILD SECONDARY`。
* **⚠️ 重大变更**：binlog 版本已更新，因此之前版本的 binlog 将无法回放。升级过程中需确保 Manticore Search 被干净停止。这意味着停止旧实例后，除了 `binlog.meta` 外，`/var/lib/manticore/binlog/` 不应存在其他 binlog 文件。
* [Issue #849](https://github.com/manticoresoftware/manticoresearch/issues/849) `SHOW SETTINGS`：现在可以从 Manticore 内部查看配置文件中的设置。
* [Issue #1007](https://github.com/manticoresoftware/manticoresearch/issues/1007) 通过 [SET GLOBAL CPUSTATS=1/0](Server_settings/Setting_variables_online.md#SET) 可开启/关闭 CPU 时间追踪；关闭追踪时，[SHOW THREADS](Node_info_and_management/SHOW_THREADS.md) 不显示 CPU 统计信息。
* [Issue #1009](https://github.com/manticoresoftware/manticoresearch/issues/1009) RT 表的 RAM 分块段现在可在 RAM 分块被刷新时合并。
* [Issue #1012](https://github.com/manticoresoftware/manticoresearch/issues/1012) 在 [indexer](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) 输出中增加了二级索引的进度显示。
* [Issue #1013](https://github.com/manticoresoftware/manticoresearch/issues/1013) 之前如果 Manticore 启动时无法开始服务某表记录，则会从索引列表中移除该记录。现行为为保留该记录以在下次启动时尝试加载。
* [indextool --docextract](Miscellaneous_tools.md#indextool) 返回请求文档的所有词和命中。
* [Commit 2b29](https://github.com/manticoresoftware/manticoresearch/commit/2b296ee20e520b85bcbb4383e87095d31e7165dc) 环境变量 `dump_corrupt_meta` 可在 searchd 无法加载索引时，将损坏的表元数据导出到日志。
* [Commit c7a3](https://github.com/manticoresoftware/manticoresearch/commit/c7a3136eefe0eb2f69e3d99fae9c10beb5821ea6) `DEBUG META` 可显示 `max_matches` 和伪分片统计信息。
* [Commit 6bca](https://github.com/manticoresoftware/manticoresearch/commit/6bca4848dc5c8396dbc1e057be4b9b2d579794e8) 用更好的错误提示替代了令人困惑的“Index header format is not json, will try it as binary...”。
* [Commit bef3](https://github.com/manticoresoftware/lemmatizer-uk/commit/bef3ff0386d3ee87ec57619782100972c1122e47) 乌克兰语词干分析器路径已更改。
* [Commit 4ae7](https://github.com/manticoresoftware/manticoresearch/commit/4ae789595329a2951e194d1191ddb3121459a560) 在 [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META) 中增加了二级索引统计信息。
* [Commit 2e7c](https://github.com/manticoresoftware/manticoresearch/commit/2e7c585e) JSON 接口现在可以通过 Swagger Editor https://manual.manticoresearch.com/Openapi#OpenAPI-specification 轻松可视化。
* **⚠️ 重大变更**：复制协议已更改。如果您运行复制集群，升级到 Manticore 5 时需要：
  - 先干净关闭所有节点
  - 然后以 `--new-cluster` 启动最后关闭的节点（Linux 下运行工具 `manticore_new_cluster`）。
  - 详细说明请参见 [restart a cluster](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster)。

### 与 Manticore 列存库相关的更改
* 对二级索引与列存储的集成进行了重构。
* [Commit efe2](https://github.com/manticoresoftware/columnar/commit/efe26b2a) Manticore 列存库优化，通过部分预先的最小/最大值判定，可降低响应时间。
* [提交 2757](https://github.com/manticoresoftware/columnar/commit/2757b99b) 如果磁盘块合并被中断，守护进程现在会清理与MCL相关的临时文件。
* [提交 e9c6](https://github.com/manticoresoftware/manticoresearch/commit/e9c672014b63e2b49e7a5a53880583af86c3ab4e) 在崩溃时，列存储和二级库的版本信息会被输出到日志。
* [提交 f5e8](https://github.com/manticoresoftware/columnar/commit/f5e84eeb8b6cb102f13e3468246702427527fef9) 为二级索引增加了快速文档列表回绕的支持。
* [提交 06df](https://github.com/manticoresoftware/manticoresearch/commit/06dfdd2e668933495dd6cb52d5ba2e0aa9b26e58) 像 `select attr, count(*) from plain_index` （无过滤条件）的查询，在使用MCL时现在速度更快。
* [提交 0a76](https://github.com/manticoresoftware/manticoresearch/commit/0a76ecb9adbe886868b962dd425de5f475bbf55e) 为兼容mysql大于8.25版本的.net连接器，HandleMysqlSelectSysvar中添加了@@autocommit支持。
* **⚠️ 重大变更**：[MCL Issue #17](https://github.com/manticoresoftware/columnar/issues/17) MCL：为列存扫描增加SSE代码。MCL现在至少需要SSE4.2支持。

### 与打包相关的变更
* [提交 4d19](https://github.com/manticoresoftware/manticoresearch/commit/4d19f5cbe49a31228aa09253f061165bfe80e51b) **⚠️ 重大变更**：不再支持Debian Stretch和Ubuntu Xenial。
* 支持RHEL 9，包括Centos 9、Alma Linux 9和Oracle Linux 9。
* [问题 #924](https://github.com/manticoresoftware/manticoresearch/issues/924) 支持Debian Bookworm。
* [问题 #636](https://github.com/manticoresoftware/manticoresearch/issues/636) 打包：Linux和MacOS的arm64构建。
* [PR #26](https://github.com/manticoresoftware/docker/pull/26) 多架构（x86_64 / arm64）docker镜像。
* [为贡献者简化的软件包构建](Installation/Compiling_from_sources.md#Building-using-CI-Docker)。
* 现在可以使用APT安装特定版本。
* [提交 a6b8](https://github.com/manticoresoftware/manticoresearch/commit/51fddca5c2a3ebb8576fae4c18660656ba22de0f) Windows安装程序（以前只提供了归档文件）。
* 切换到使用CLang 15编译。
* **⚠️ 重大变更**：自定义Homebrew公式，包括Manticore列存库的公式。安装Manticore、MCL及其他必要组件，请使用以下命令 `brew install manticoresoftware/manticore/manticoresearch manticoresoftware/manticore/manticore-extra`。

### Bug修复
* [问题 #479](https://github.com/manticoresoftware/manticoresearch/issues/479) 字段名为 `text` 的问题。
* [问题 #501](https://github.com/manticoresoftware/manticoresearch/issues/501) id不能是非bigint类型。
* [问题 #646](https://github.com/manticoresoftware/manticoresearch/issues/646) ALTER操作涉及字段名为"text"的问题。
* ❗[问题 #652](https://github.com/manticoresoftware/manticoresearch/issues/652) 可能的BUG：HTTP（JSON）中的offset和limit影响facet结果。
* ❗[问题 #827](https://github.com/manticoresoftware/manticoresearch/issues/827) searchd在高负载下挂起/崩溃。
* ❗[问题 #996](https://github.com/manticoresoftware/manticoresearch/issues/996) PQ索引内存溢出。
* ❗[提交 1041](https://github.com/manticoresoftware/manticoresearch/commit/10416ef7dddf06c0d759e32ccd6ebaa2468f7cbf) 自Sphinx以来 `binlog_flush = 1` 一直存在问题，已修复。
* [MCL Issue #14](https://github.com/manticoresoftware/columnar/issues/14) MCL：当全文字段太多时，select操作崩溃。
* [问题 #470](https://github.com/manticoresoftware/manticoresearch/issues/470) sql_joined_field无法存储。
* [问题 #713](https://github.com/manticoresoftware/manticoresearch/issues/713) 使用LEVENSHTEIN()时崩溃。
* [问题 #743](https://github.com/manticoresoftware/manticoresearch/issues/743) Manticore意外崩溃且不能正常重启。
* [问题 #788](https://github.com/manticoresoftware/manticoresearch/issues/788) 通过 /sql 调用KEYWORDS返回控制字符，导致json解析错误。
* [问题 #789](https://github.com/manticoresoftware/manticoresearch/issues/789) mariadb无法创建FEDERATED表。
* [问题 #796](https://github.com/manticoresoftware/manticoresearch/issues/796) 警告：dlopen()失败：/usr/bin/lib_manticore_columnar.so找不到共享对象文件。
* [问题 #797](https://github.com/manticoresoftware/manticoresearch/issues/797) 通过API进行带ZONESPAN的搜索时Manticore崩溃。
* [问题 #799](https://github.com/manticoresoftware/manticoresearch/issues/799) 使用多个索引和facet distinct时权重错误。
* [问题 #801](https://github.com/manticoresoftware/manticoresearch/issues/801) SQL索引重新处理后，SphinxQL分组查询挂起。
* [问题 #802](https://github.com/manticoresoftware/manticoresearch/issues/802) MCL：Indexer在5.0.2和manticore-columnar-lib 1.15.4版本中崩溃。
* [问题 #813](https://github.com/manticoresoftware/manticoresearch/issues/813) Manticore 5.0.2 FEDERATED返回空集合（MySQL 8.0.28）。
* [问题 #824](https://github.com/manticoresoftware/manticoresearch/issues/824) 在两个索引上做COUNT DISTINCT且结果为零时抛出内部错误。
* [问题 #826](https://github.com/manticoresoftware/manticoresearch/issues/826) 删除查询时崩溃。
* [问题 #843](https://github.com/manticoresoftware/manticoresearch/issues/843) MCL：长文本字段的BUG。
* [问题 #856](https://github.com/manticoresoftware/manticoresearch/issues/856) 5.0.2 rtindex：聚合搜索限制行为不符合预期。
* [问题 #863](https://github.com/manticoresoftware/manticoresearch/issues/863) 即使搜索应返回多个结果，返回的hits是None类型对象。
* [问题 #870](https://github.com/manticoresoftware/manticoresearch/issues/870) SELECT表达式中使用Attribute和Stored Field时崩溃。
* [问题 #872](https://github.com/manticoresoftware/manticoresearch/issues/872) 崩溃后表变得不可见。
* [问题 #877](https://github.com/manticoresoftware/manticoresearch/issues/877) 搜索查询中两个负面词导致错误：查询不可计算
* [问题 #878](https://github.com/manticoresoftware/manticoresearch/issues/878) 通过 json query_string 使用 a -b -c 无效
* [问题 #886](https://github.com/manticoresoftware/manticoresearch/issues/886) 使用伪分片的查询匹配
* [问题 #893](https://github.com/manticoresoftware/manticoresearch/issues/893) Manticore 5.0.2 中的 min/max 函数不符合预期
* [问题 #896](https://github.com/manticoresoftware/manticoresearch/issues/896) 字段 "weight" 解析不正确
* [问题 #897](https://github.com/manticoresoftware/manticoresearch/issues/897) Manticore 服务启动时崩溃并持续重启
* [问题 #900](https://github.com/manticoresoftware/manticoresearch/issues/900) group by j.a，某些操作错误
* [问题 #913](https://github.com/manticoresoftware/manticoresearch/issues/913) 当表达式用于 ranker 中，且查询包含两个邻近条件时，searchd 崩溃
* [问题 #916](https://github.com/manticoresoftware/manticoresearch/issues/916) net_throttle_action 失效
* [问题 #919](https://github.com/manticoresoftware/manticoresearch/issues/919) MCL：Manticore 在查询执行时崩溃，集群恢复时也出现崩溃
* [问题 #925](https://github.com/manticoresoftware/manticoresearch/issues/925) SHOW CREATE TABLE 输出缺少反引号
* [问题 #930](https://github.com/manticoresoftware/manticoresearch/issues/930) 现在可以通过 JDBC 连接器从 Java 查询 Manticore
* [问题 #933](https://github.com/manticoresoftware/manticoresearch/issues/933) bm25f 排名问题
* [问题 #934](https://github.com/manticoresoftware/manticoresearch/issues/934) 无配置索引在 watchdog 中首次加载时被冻结
* [问题 #937](https://github.com/manticoresoftware/manticoresearch/issues/937) 排序 facet 数据时发生段错误
* [问题 #940](https://github.com/manticoresoftware/manticoresearch/issues/940) CONCAT(TO_STRING) 时崩溃
* [问题 #947](https://github.com/manticoresoftware/manticoresearch/issues/947) 某些情况下单条简单的 select 查询导致整个实例阻塞，无法登录或执行其他查询，直到当前查询完成。
* [问题 #948](https://github.com/manticoresoftware/manticoresearch/issues/948) 索引器崩溃
* [问题 #950](https://github.com/manticoresoftware/manticoresearch/issues/950) facet distinct 计数错误
* [问题 #953](https://github.com/manticoresoftware/manticoresearch/issues/953) 内置 sph04 ranker 中 LCS 计算不正确
* [问题 #955](https://github.com/manticoresoftware/manticoresearch/issues/955) 5.0.3 开发版崩溃
* [问题 #963](https://github.com/manticoresoftware/manticoresearch/issues/963) 在 columnar 引擎中使用 JSON 的 FACET 崩溃
* [问题 #982](https://github.com/manticoresoftware/manticoresearch/issues/982) MCL：5.0.3 版本从二级索引崩溃
* [PR #984](https://github.com/manticoresoftware/manticoresearch/pull/984) @@autocommit 在 HandleMysqlSelectSysvar 中的应用
* [PR #985](https://github.com/manticoresoftware/manticoresearch/pull/985) 修复 RT 索引中的线程块分配
* [问题 #985](https://github.com/manticoresoftware/manticoresearch/issues/985) 修复 RT 索引中的线程块分配
* [问题 #986](https://github.com/manticoresoftware/manticoresearch/issues/986) 默认 max_query_time 错误
* [问题 #987](https://github.com/manticoresoftware/manticoresearch/issues/987) 多线程执行时使用正则表达式表达式导致崩溃
* [问题 #988](https://github.com/manticoresoftware/manticoresearch/issues/988) 倒排索引兼容性破坏
* [问题 #989](https://github.com/manticoresoftware/manticoresearch/issues/989) indextool 检查 columnar 属性时报错
* [问题 #990](https://github.com/manticoresoftware/manticoresearch/issues/990) json grouper 克隆导致内存泄漏
* [问题 #991](https://github.com/manticoresoftware/manticoresearch/issues/991) levenshtein 函数克隆导致内存泄漏
* [问题 #992](https://github.com/manticoresoftware/manticoresearch/issues/992) 加载元数据时错误信息丢失
* [问题 #993](https://github.com/manticoresoftware/manticoresearch/issues/993) 传递动态索引/子键和系统变量中的错误
* [问题 #994](https://github.com/manticoresoftware/manticoresearch/issues/994) 在 columnar 存储中对 columnar 字符串执行 count distinct 导致崩溃
* [问题 #995](https://github.com/manticoresoftware/manticoresearch/issues/995) MCL：从 taxi1 执行 min(pickup_datetime) 崩溃
* [问题 #997](https://github.com/manticoresoftware/manticoresearch/issues/997) 空的 excludes JSON 查询从选择列表中移除列
* [问题 #998](https://github.com/manticoresoftware/manticoresearch/issues/998) 二级任务在当前调度器上运行有时导致异常副作用
* [问题 #999](https://github.com/manticoresoftware/manticoresearch/issues/999) 使用 facet distinct 和不同模式时崩溃
* [问题 #1000](https://github.com/manticoresoftware/manticoresearch/issues/1000) MCL：未使用 columnar 库运行后，columnar rt 索引损坏
* [问题 #1001](https://github.com/manticoresoftware/manticoresearch/issues/1001) json 中隐式 cutoff 不生效
* [问题 #1002](https://github.com/manticoresoftware/manticoresearch/issues/1002) Columnar grouper 问题
* [问题 #1003](https://github.com/manticoresoftware/manticoresearch/issues/1003) 无法删除索引中的最后一个字段
* [问题 #1004](https://github.com/manticoresoftware/manticoresearch/issues/1004) --new-cluster 后行为异常
* [问题 #1005](https://github.com/manticoresoftware/manticoresearch/issues/1005) “columnar library not loaded”，但并非必要
* [问题 #1006](https://github.com/manticoresoftware/manticoresearch/issues/1006) 删除查询无错误提示
* [问题 #1010](https://github.com/manticoresoftware/manticoresearch/issues/1010) 修正 Windows 构建中 ICU 数据文件位置
* [PR #1018](https://github.com/manticoresoftware/manticoresearch/pull/1018) 握手发送问题
* [Issue #1020](https://github.com/manticoresoftware/manticoresearch/issues/1020) 在 show create table 中显示 id
* [Issue #1024 crash 1](https://github.com/manticoresoftware/manticoresearch/issues/1024) Facet 搜索大量结果时崩溃 / 段错误。
* [Issue #1026](https://github.com/manticoresoftware/manticoresearch/issues/1026) RT 索引：当插入大量文档且 RAMchunk 变满时，searchd “卡住” 永远不动
* [Commit 4739](https://github.com/manticoresoftware/manticoresearch/commit/4739dafd) 节点间复制繁忙时，线程在关闭时卡住
* [Commit ab87](https://github.com/manticoresoftware/manticoresearch/commit/ab87836782e7ae43fe5f9dea739eed0d336b42c1) 在 JSON 范围过滤器中混合使用浮点数和整数可能导致 Manticore 忽略该过滤器
* [Commit d001](https://github.com/manticoresoftware/manticoresearch/commit/d00101c2905f6393ce3fca23c4f6dcb2506f4bd9) JSON 中的浮点过滤器不准确
* [Commit 4092](https://github.com/manticoresoftware/manticoresearch/commit/4092c6a8) 如果索引被修改则丢弃未提交的事务 (否则可能崩溃)
* [Commit 9692](https://github.com/manticoresoftware/manticoresearch/commit/9692a840) 在使用反斜杠时查询语法错误
* [Commit 0c19](https://github.com/manticoresoftware/manticoresearch/commit/0c19ad8d) SHOW STATUS 中 workers_clients 可能错误
* [Commit 1772](https://github.com/manticoresoftware/manticoresearch/commit/17720e1e2f2ec38337df565bee78f7ce9d9c744a) 修复合并无文档存储的 RAM 段时崩溃
* [Commit f45b](https://github.com/manticoresoftware/manticoresearch/commit/f45b337b5c265433bb7d6262ad65d02f65edbaf3) 修复 JSON 过滤器中遗失的 ALL/ANY 条件
* [Commit 3e83](https://github.com/manticoresoftware/manticoresearch/commit/3e83c1f0c0e6e7a9d4297113f69a112cbabd9e64) 如果 searchd 是从无写权限的目录启动，复制可能失败并提示 `got exception while reading ist stream: mkstemp(./gmb_pF6TJi) failed: 13 (Permission denied)`
* [Commit 92e5](https://github.com/manticoresoftware/manticoresearch/commit/92e5629f7150f62524b385fca670a01ba9590f5b) 自 4.0.2 起，崩溃日志只包含偏移。本次提交修复了此问题。

## 版本 5.0.2
发布时间：2022年5月30日

### Bug 修复
* ❗[Issue #791](https://github.com/manticoresoftware/manticoresearch/issues/791) - 错误的栈大小可能导致崩溃。

## 版本 5.0.0
发布时间：2022年5月18日


### 主要新特性
* 🔬 支持 [Manticore Columnar Library 1.15.2](https://github.com/manticoresoftware/columnar/)，启用 [Secondary indexes](Server_settings/Searchd.md#secondary_indexes) 测试版。对于普通和实时列式及行式索引，默认开启构建二级索引（如果使用 [Manticore Columnar Library](https://github.com/manticoresoftware/columnar)），但要启用搜索功能，需要在配置文件中或通过 [SET GLOBAL](Server_settings/Setting_variables_online.md) 设置 `secondary_indexes = 1`。该新功能支持除旧版 Debian Stretch 和 Ubuntu Xenial 以外的所有操作系统。
* [只读模式](Security/Read_only.md)：您现在可以指定仅处理读取查询而忽略任何写操作的监听器。
* 新增 [/cli](Connecting_to_the_server/HTTP.md#/cli) 端点，使得通过 HTTP 执行 SQL 查询更加方便。
* 通过 HTTP JSON 实现更快的批量 INSERT/REPLACE/DELETE：以前可以通过 HTTP JSON 协议提供多个写命令，但它们是一个一个处理的，现在作为单个事务处理。
* [#720](https://github.com/manticoresoftware/manticoresearch/issues/720) JSON 协议支持[嵌套过滤器](Searching/Filters.md#Nested-bool-query)。此前 JSON 中不能表达如 `a=1 and (b=2 or c=3)` 的逻辑：`must`（AND）、`should`（OR）和 `must_not`（NOT）只作用于最高层级。现在支持嵌套。
* 支持 HTTP 协议中的 [分块传输编码](https://en.wikipedia.org/wiki/Chunked_transfer_encoding)。您的应用现在可以用分块传输发送大型批次，减少资源占用（无需计算 `Content-Length`）。服务器端，Manticore 始终以流式方式处理接收的 HTTP 数据，不再像以前那样等待整个批次传完，带来的好处有：
  - 降低峰值内存使用，减少 OOM 风险
  - 降低响应时间（我们的测试显示处理 100MB 批次时间减少约 11%）
  - 允许突破 [max_packet_size](Server_settings/Searchd.md#max_packet_size) 限制，传输远大于 `max_packet_size`（128MB）限制的批次，比如一次传输 1GB。
* [#719](https://github.com/manticoresoftware/manticoresearch/issues/719) HTTP 接口支持 `100 Continue`：现在可以从默认发送 `Expect: 100-continue` 并等待的 curl（包含各种编程语言中使用的 curl 库）传输大批次内容。以前需要添加 `Expect: ` 头部才能避免卡住，现在不再需要。

  <details>

  之前（注意响应时间）：

  ```bash
  $ time curl -v -sX POST http://localhost:9318/bulk -H "Content-Type: application/x-ndjson" --data '{"insert": {"index": "user", "doc":  {"name":"Prof. Matt Heaney IV","email":"ibergnaum@yahoo.com","description":"Tempora ullam eaque consequatur. Vero aut minima ut et ut omnis officiis vel. Molestiae quis voluptatum sint numquam.","age":15,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"Prof. Boyd McKenzie","email":"carlotta11@hotmail.com","description":"Blanditiis maiores odio corporis eaque illum. Aut et rerum iste. Neque et ullam quisquam officia dignissimos quo cumque.","age":84,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"约翰·史密斯先生","email":"stiedemann.tristin@ziemann.com","description":"临时事务大规模完成所有事务逃逸。负债自然时间产生真实。哈鲁姆腐败无人在场。","age":89,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"赫克托·普罗斯","email":"hickle.mafalda@hotmail.com","description":"作为发明者的愉快。逃避偏好是身份和仇恨与智慧。","age":64,"active":1}}}'
  *   正在连接 127.0.0.1...
  * 已连接到 localhost (127.0.0.1) 端口 9318 (#0)
  > POST /bulk HTTP/1.1
  > 主机: localhost:9318
  > 用户代理: curl/7.47.0
  > 接受: */*
  > 内容类型: application/x-ndjson
  > 内容长度: 1025
  > 期望: 100-继续
  >
  * 完成等待 100-继续
  * 完全上传且正常
  < HTTP/1.1 200 OK
  < 服务器: 4.2.0 15e927b@211223 发行版 (columnar 1.11.4 327b3d4@211223)
  < 内容类型: application/json; charset=UTF-8
  < 内容长度: 434
  <
  * 与 localhost 的连接 #0 保持不变
  {"items":[{"insert":{"table":"user","_id":2811798918248005633,"created":true,"result":"created","status":201}},{"insert":{"table":"user","_id":2811798918248005634,"created":true,"result":"created","status":201}},{"insert":{"table":"user","_id":2811798918248005635,"created":true,"result":"created","status":201}},{"insert":{"table":"user","_id":2811798918248005636,"created":true,"result":"created","status":201}}],"errors":false}
  实际时间
0m1.022s
  用户时间
0m0.001s

  系统时间
0m0.010s
  ```
  现在：
  ```bash
  $ time curl -v -sX POST http://localhost:9318/bulk -H "Content-Type: application/x-ndjson" --data '{"insert": {"index": "user", "doc":  {"name":"马特·希尼教授 IV","email":"ibergnaum@yahoo.com","description":"临时无形地完成。真实或最小的所有官方。烦恼欲望不经常。","age":15,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"博伊德·麦肯齐教授","email":"carlotta11@hotmail.com","description":"主要憎恨的身体光辉。自己和之事。没有和任何官方尊重。","age":84,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"约翰·史密斯先生","email":"stiedemann.tristin@ziemann.com","description":"临时事务大规模完成所有事务逃逸。负债自然时间产生真实。哈鲁姆腐败无人在场。","age":89,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"赫克托·普罗斯","email":"hickle.mafalda@hotmail.com","description":"作为发明者的愉快。逃避偏好是身份和仇恨与智慧。","age":64,"active":1}}}'
  *   正在连接 127.0.0.1...
  * 已连接到 localhost (127.0.0.1) 端口 9318 (#0)
  > POST /bulk HTTP/1.1
  > 主机: localhost:9318
  > 用户代理: curl/7.47.0
  > 接受: */*
  > 内容类型: application/x-ndjson
  > 内容长度: 1025
  > 期望: 100-继续
  >
  < HTTP/1.1 100 Continue
  < 服务器: 4.2.1 63e5749@220405 开发版
  < 内容类型: application/json; charset=UTF-8
  < 内容长度: 0
  * 完全上传且正常
  < HTTP/1.1 200 OK
  < 服务器: 4.2.1 63e5749@220405 开发版
  < 内容类型: application/json; charset=UTF-8
  < 内容长度: 147
  <
  * 与 localhost 的连接 #0 保持不变
  {"items":[{"bulk":{"table":"user","_id":2811798919590182916,"created":4,"deleted":0,"updated":0,"result":"created","status":201}}],"errors":false}
  实际时间

0m0.015s

  用户时间
0m0.005s
  系统时间
0m0.004s
  ```
  </details>

* **⚠️ 重大变更**: [伪分片](Server_settings/Searchd.md#pseudo_sharding) 默认启用。 如果想禁用，请确保在 Manticore 配置文件的 `searchd` 部分添加 `pseudo_sharding = 0` 。
* 在实时/明文索引中至少包含一个全文字段已不再是强制要求。现在你可以在与全文搜索无关的情况下使用 Manticore。
* 由 [Manticore Columnar Library](https://github.com/manticoresoftware/columnar) 支持的属性实现的[快速读取](Creating_a_table/Data_types.md#fast_fetch): 像 `select * from <columnar table>` 之类的查询现在比以前快得多，尤其是当模式中含有大量字段时。
* **⚠️ 重大变更**: 隐式 [cutoff](Searching/Options.md#cutoff)。Manticore 现在不花时间和资源处理不需要包含在结果集中的数据。缺点是它影响了[SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META)中的 `total_found` 以及 JSON 输出中的 [hits.total](Searching/Full_text_matching/Basic_usage.md#HTTP-JSON)。只有当你看到 `total_relation: eq` 时它才准确，而当是 `total_relation: gte` 时意味着实际匹配文档数大于你得到的 `total_found` 值。要保留以前的行为，可以使用搜索选项 `cutoff=0`，这使得 `total_relation` 总是 `eq`。
* **⚠️ 重大变更**: 现在所有全文字段默认都是[存储](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#stored_fields)的。你需要使用 `stored_fields = ` （空值）使所有字段非存储（即恢复到以前的行为）。
* [#715](https://github.com/manticoresoftware/manticoresearch/issues/715) HTTP JSON 支持了[搜索选项](Searching/Options.md#General-syntax)。
### 次要变更
* **⚠️ 重大变更**: 索引元文件格式发生变化。以前元文件（`.meta`、`.sph`）是二进制格式，现在改为纯 json 格式。新版 Manticore 自动转换旧索引，但：
  - 你可能会收到类似 `WARNING: ... syntax error, unexpected TOK_IDENT` 的警告
  - 旧版本 Manticore 不能运行新版索引，请确保有备份
* **⚠️ 重大变更**：通过 [HTTP keep-alive](Connecting_to_the_server/HTTP.md#Keep-alive) 支持会话状态。当客户端也支持时，这让 HTTP 变得有状态。例如，使用新的 [/cli](Connecting_to_the_server/HTTP.md#/cli) 端点和默认启用的 HTTP keep-alive（所有浏览器默认开启），你可以在 `SELECT` 后调用 `SHOW META`，它的工作方式和通过 mysql 的一样。注意，之前也支持 `Connection: keep-alive` HTTP 头，但它仅导致复用同一连接。自本版本起，它还使会话变为有状态。
* 你现在可以指定 `columnar_attrs = *` 来将所有属性定义为列存储，适用于属性列表较长的情况，参考 [plain 模式](Read_this_first.md#Real-time-mode-vs-plain-mode)。
* 更快的复制 SST。
* **⚠️ 重大变更**：复制协议已更改。如果你运行复制集群，升级到 Manticore 5 时需要：
  - 先干净地停止所有节点
  - 然后使用 `--new-cluster` 启动最后停止的节点（Linux下运行工具 `manticore_new_cluster`）。
  - 详见[重启集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster)。
* 复制改进：
  - 更快的 SST
  - 噪声抗干扰，能帮助在复制节点间网络不稳定时保持稳定
  - 改进日志记录
* 安全改进：如果配置中未指定任何 `listen`，Manticore 现在监听 `127.0.0.1`，而非 `0.0.0.0`。虽然 Manticore Search 默认配置通常指定了 `listen`，且一般不会无 `listen`，但仍有可能存在。之前监听 `0.0.0.0` 不安全，现在监听 `127.0.0.1`，通常不会暴露在互联网。
* 对列存储属性的聚合更快。
* 增强 `AVG()` 精度：之前 Manticore 内部聚合使用 `float`，现在使用 `double`，显著提高精度。
* 改进 JDBC MySQL 驱动支持。
* 支持 jemalloc 的 `DEBUG malloc_stats`。
* [optimize_cutoff](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#optimize_cutoff) 现可作为表级设置，在 CREATE 或 ALTER 表时配置。
* **⚠️ 重大变更**：[query_log_format](Server_settings/Searchd.md#query_log_format) 现在默认是 **`sphinxql`**。如果习惯使用 `plain` 格式，需要在配置文件中添加 `query_log_format = plain`。
* 显著降低内存消耗：在使用存储字段且插入/替换/优化工作量大且持续时间长时，Manticore 的内存使用显著减少。
* [shutdown_timeout](Server_settings/Searchd.md#shutdown_timeout) 默认值从 3 秒增加到 60 秒。

* [Commit ffd0](https://github.com/manticoresoftware/manticoresearch/commit/ffd0499d329d2c383f14c8a44c4cc84338ab56e7) Java mysql 连接器 >= 6.0.3 支持：在 [Java mysql 连接 6.0.3](https://mvnrepository.com/artifact/mysql/mysql-connector-java/6.0.3) 中连接方式更改，导致之前与 Manticore 不兼容。现支持新行为。
* [Commit 1da6](https://github.com/manticoresoftware/manticoresearch/commit/1da6dbec) 禁止在加载索引时（比如 searchd 启动时）保存新的磁盘块。
* [Issue #746](https://github.com/manticoresoftware/manticoresearch/issues/746) 支持 glibc >= 2.34。
* [Issue #784](https://github.com/manticoresoftware/manticoresearch/issues/784) 独立统计 ‘VIP’ 连接数，之前 VIP 连接计入 `max_connections` 限制，可能导致非 VIP 连接 “maxed out” 错误。现在 VIP 连接不计入限制。当前 VIP 连接数可在 `SHOW STATUS` 和 `status` 中查看。
* [ID](Creating_a_table/Data_types.md#Document-ID) 现在可显式指定。
* [Issue #687](https://github.com/manticoresoftware/manticoresearch/issues/687) 支持 mysql 协议的 zstd 压缩。
### ⚠️ 其他小的重大变更
* ⚠️ BM25F 公式略有更新以提升搜索相关性。仅影响使用函数 [BM25F()](Functions/Searching_and_ranking_functions.md#BM25F%28%29) 的搜索结果，不改变默认排序公式行为。
* ⚠️ REST [/sql](Connecting_to_the_server/HTTP.md#mode=raw) 端点行为更改： `/sql?mode=raw` 现在需要转义并返回数组。

* ⚠️ `/bulk` INSERT/REPLACE/DELETE 响应格式更改：
  - 之前每个子查询单独作为一个事务，返回独立响应
  - 现在整个批次视为单一事务，返回单一响应
* ⚠️ 搜索选项 `low_priority` 和 `boolean_simplify` 现在必须赋值（`0/1`），之前可以直接写 `SELECT ... OPTION low_priority, boolean_simplify`，现在需写 `SELECT ... OPTION low_priority=1, boolean_simplify=1`。
* ⚠️ 如果你使用旧版 [php](https://github.com/manticoresoftware/manticoresearch/blob/master/api/sphinxapi.php)、[python](https://github.com/manticoresoftware/manticoresearch/blob/master/api/sphinxapi.py) 或 [java](https://github.com/manticoresoftware/manticoresearch/tree/master/api/java) 客户端，请访问对应链接获取更新版本。**旧版本与 Manticore 5 不完全兼容。**
* ⚠️ HTTP JSON 请求在 `query_log_format=sphinxql` 模式下的日志格式更改。之前只记录全文部分，现在原样记录。
### 新包
* **⚠️ 重大变更**：由于新结构，升级到 Manticore 5 时建议先删除旧包再安装新包：
  - 基于 RPM：`yum remove manticore*`

  - Debian 和 Ubuntu：`apt remove manticore*`
* 新的 deb/rpm 包结构。之前版本提供了：
  - `manticore-server` 包含 `searchd`（主要搜索守护进程）及其所需全部内容
  - `manticore-tools` 包含 `indexer` 和 `indextool`
  - `manticore` 包含所有内容
  - `manticore-all` RPM 作为一个元包，引用了 `manticore-server` 和 `manticore-tools`
  新结构为：
  - `manticore` - deb/rpm 元包，安装上述全部包作为依赖
  - `manticore-server-core` - `searchd` 及其独立运行所需全部内容
  - `manticore-server` - systemd 文件及其他辅助脚本

  - `manticore-tools` - `indexer`、`indextool` 及其他工具
  - `manticore-common` - 默认配置文件、默认数据目录、默认停用词
  - `manticore-icudata`、`manticore-dev`、`manticore-converter` 变化不大
  - 包含所有包的 `.tgz` 捆绑包
* 支持 Ubuntu Jammy
* 通过 [YUM repo](Installation/RHEL_and_Centos.md#YUM-repository) 支持 Amazon Linux 2
### Bug修复
* [问题 #815](https://github.com/manticoresoftware/manticoresearch/issues/815) 使用 UDF 函数时随机崩溃
* [问题 #287](https://github.com/manticoresoftware/manticoresearch/issues/287) 索引 RT 索引时内存溢出
* [问题 #604](https://github.com/manticoresoftware/manticoresearch/issues/604) 3.6.0, 4.2.0 sphinxql-parser 破坏性变更
* [问题 #667](https://github.com/manticoresoftware/manticoresearch/issues/667) 致命错误：内存不足（无法分配 9007199254740992 字节）
* [问题 #676](https://github.com/manticoresoftware/manticoresearch/issues/676) 字符串未正确传递给 UDF
* ❗[问题 #698](https://github.com/manticoresoftware/manticoresearch/issues/698) Searchd 在尝试向 rt 索引添加文本列后崩溃
* [问题 #705](https://github.com/manticoresoftware/manticoresearch/issues/705) Indexer 未能找到所有列
* ❗[问题 #709](https://github.com/manticoresoftware/manticoresearch/issues/705) 按 json.boolean 分组结果错误
* [问题 #716](https://github.com/manticoresoftware/manticoresearch/issues/716) indextool 相关索引命令（如 --dumpdict）失败
* ❗[问题 #724](https://github.com/manticoresoftware/manticoresearch/issues/724) 字段从选择中消失
* [问题 #727](https://github.com/manticoresoftware/manticoresearch/issues/727) .NET HttpClient 使用 `application/x-ndjson` 时 Content-Type 不兼容
* [问题 #729](https://github.com/manticoresoftware/manticoresearch/issues/729) 字段长度计算问题
* ❗[问题 #730](https://github.com/manticoresoftware/manticoresearch/issues/730) 创建/插入/删除列式表存在内存泄漏
* [问题 #731](https://github.com/manticoresoftware/manticoresearch/issues/731) 某些条件下结果出现空列
* ❗[问题 #749](https://github.com/manticoresoftware/manticoresearch/issues/749) 守护进程启动时崩溃
* ❗[问题 #750](https://github.com/manticoresoftware/manticoresearch/issues/750) 守护进程启动时挂起
* ❗[问题 #751](https://github.com/manticoresoftware/manticoresearch/issues/751) SST 阶段崩溃
* [问题 #752](https://github.com/manticoresoftware/manticoresearch/issues/752) engine='columnar' 时 Json 属性被标记为列式
* [问题 #753](https://github.com/manticoresoftware/manticoresearch/issues/753) 复制监听地址为 0
* [问题 #754](https://github.com/manticoresoftware/manticoresearch/issues/754) columnar_attrs = * 在 csvpipe 中无效
* ❗[问题 #755](https://github.com/manticoresoftware/manticoresearch/issues/755) rt 中选择列式浮点数时崩溃
* ❗[问题 #756](https://github.com/manticoresoftware/manticoresearch/issues/756) Indextool 在检查过程中修改 rt 索引
* [问题 #757](https://github.com/manticoresoftware/manticoresearch/issues/757) 需要检查监听端口范围是否有交叉
* [问题 #758](https://github.com/manticoresoftware/manticoresearch/issues/758) RT 索引保存磁盘块失败时记录原始错误日志
* [问题 #759](https://github.com/manticoresoftware/manticoresearch/issues/759) RE2 配置仅报告一个错误
* ❗[问题 #760](https://github.com/manticoresoftware/manticoresearch/issues/760) 提交 5463778558586d2508697fa82e71d657ac36510f 中 RAM 使用量变化
* [问题 #761](https://github.com/manticoresoftware/manticoresearch/issues/761) 第三个节点脏重启后未变成非主集群
* [问题 #762](https://github.com/manticoresoftware/manticoresearch/issues/762) 更新计数器增加两次
* [问题 #763](https://github.com/manticoresoftware/manticoresearch/issues/763) 新版本 4.2.1 损坏了用 4.2.0 创建的带形态学的索引
* [问题 #764](https://github.com/manticoresoftware/manticoresearch/issues/764) json keys 和 /sql?mode=raw 中没有转义
* ❗[问题 #765](https://github.com/manticoresoftware/manticoresearch/issues/765) 使用函数时隐藏了其他值
* ❗[问题 #766](https://github.com/manticoresoftware/manticoresearch/issues/766) FixupAttrForNetwork 中一行代码触发内存泄漏
* ❗[问题 #767](https://github.com/manticoresoftware/manticoresearch/issues/767) 4.2.0 和 4.2.1 中与文档存储缓存相关的内存泄漏
* [问题 #768](https://github.com/manticoresoftware/manticoresearch/issues/768) 网络中存储字段出现奇怪的乒乓效应
* [问题 #769](https://github.com/manticoresoftware/manticoresearch/issues/769) 当 'common' 部分未提及时，lemmatizer_base 重置为空
* [问题 #770](https://github.com/manticoresoftware/manticoresearch/issues/770) pseudo_sharding 使按 id 查询变慢
* [问题 #771](https://github.com/manticoresoftware/manticoresearch/issues/771) 使用 jemalloc 时 DEBUG malloc_stats 输出为零
* [问题 #772](https://github.com/manticoresoftware/manticoresearch/issues/772) 删除/添加列导致值不可见
* [问题 #773](https://github.com/manticoresoftware/manticoresearch/issues/773) 无法向列式表添加 bit(N) 列
* [问题 #774](https://github.com/manticoresoftware/manticoresearch/issues/774) manticore.json 启动时 "cluster" 变为空

* ❗[Commit 1da4](https://github.com/manticoresoftware/manticoresearch/commits/1da4ce891e4ff13727cfd3331d72771c64ee7949) HTTP 操作未在 SHOW STATUS 中跟踪

* [Commit 3810](https://github.com/manticoresoftware/manticoresearch/commits/381000ab4af4c3c98ec8c730699ad7f39039cec8) 禁用针对低频单关键字查询的伪分片
* [Commit 8003](https://github.com/manticoresoftware/manticoresearch/commits/800325cca283a87801b28929d82420fafc76a0ee) 修正存储属性与索引合并的问题

* [Commit cddf](https://github.com/manticoresoftware/manticoresearch/commits/cddfeed6296f2041c6aae18414e5cd9cabf08281) 泛化去重值获取器；为列式字符串添加专用的去重获取器

* [Commit fba4](https://github.com/manticoresoftware/manticoresearch/commits/fba4bb4f) 修正从文档存储中获取空整数属性

* [Commit f300](https://github.com/manticoresoftware/manticoresearch/commit/f3009a9242fa16956adf871c4887e6e0303ba364) 查询日志中 `ranker` 可能被重复指定
## 版本 4.2.0，2021年12月23日

### 主要新功能
* **实时索引和全文查询的伪分片支持**。在之前的版本中我们添加了有限的伪分片支持。从此版本开始，您只需启用 [searchd.pseudo_sharding](Server_settings/Searchd.md#pseudo_sharding) 即可获取伪分片和多核处理器的全部优势。最酷的是，您无需对索引或查询做任何修改，只要启用它，如果有空闲的 CPU，就会被用来降低响应时间。它支持普通和实时索引的全文、过滤和分析查询。例如，启用伪分片可以让大多数查询在 [Hacker news 精选评论数据集](https://zenodo.org/record/45901/) 乘以100倍（116百万文档在普通索引中）上的**平均响应时间降低约10倍**。

![4.2.0中伪分片开/关对比](4.2.0_ps_on_vs_off.png)

* [**Debian Bullseye**](https://manticoresearch.com/install/) 现已支持。

<!-- example pq_transactions_4.2.0 -->
* PQ 事务现在是原子性且隔离的。之前 PQ 事务支持有限。它使得**向 PQ 批量 REPLACE 操作更快**，特别是在一次需要替换大量规则时。性能详细信息：

<!-- intro -->
#### 4.0.2
<!-- request 4.0.2 -->
插入 1M 条 PQ 规则需要**48秒**，分批 10K 进行 REPLACE 仅 40K 条需 **406 秒**。
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
插入 1M 条 PQ 规则需要**34秒**，分批 10K 进行 REPLACE 需要**23秒**。
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
* [optimize_cutoff](Server_settings/Searchd.md#optimize_cutoff) 现作为配置选项在 `searchd` 部分可用。当您想对所有索引的 RT 块数进行统一限制时非常有用。
* [Commit 0087](https://github.com/manticoresoftware/manticoresearch/commit/00874743823eaf43e2a09a088392fd7e0a617f47) 精确支持多个本地物理索引（实时/普通）上具有相同字段集/顺序的 [count(distinct ...)](Searching/Grouping.md#COUNT%28DISTINCT-field%29) 和 [FACET ... distinct](Searching/Faceted_search.md#Faceting-without-duplicates)。
* [PR #598](https://github.com/manticoresoftware/manticoresearch/pull/598) 对 `YEAR()` 及其它时间戳函数支持 bigint。

* [Commit 8e85](https://github.com/manticoresoftware/manticoresearch/commit/8e85d4bce596cc88f9d9615ea243a27e4afde0f7) 自适应 [rt_mem_limit](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_mem_limit)。之前 Manticore Search 会精确收集到 `rt_mem_limit` 大小的数据才保存新磁盘块，保存时仍会收集多达 10%（即双缓冲）数据以减少可能的插入挂起。如果也耗尽该限制，则在磁盘块完全保存前阻塞新文档添加。新自适应限制基于已有的 [auto-optimize](Server_settings/Searchd.md#auto_optimize) 功能，所以磁盘块不严格遵守 `rt_mem_limit`、提前刷新无伤大雅。现在收集不超过 `rt_mem_limit` 的 50% 作为磁盘块，保存时根据统计信息（已保存量、新到达文档数）重新计算下次收集率。例如，保存了 9000 万文档，同时保存期内又来 1000 万文档，比例 90%，即下次可收集到 `rt_mem_limit` 的 90% 后开始刷新。比例自动计算，范围 33.3% 到 95%。
* [Issue #628](https://github.com/manticoresoftware/manticoresearch/issues/628) PostgreSQL 源支持 [unpack_zlib](Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Database_connection.md#unpack_zlib)。感谢 [Dmitry Voronin](https://github.com/dimv36) 的 [贡献](https://github.com/manticoresoftware/manticoresearch/pull/630)。
* [Commit 6d54](https://github.com/manticoresoftware/manticoresearch/commit/6d54cf2b319b131970c29410cc21abcbdf8083b1) 支持 `indexer -v` 和 `--version`。之前虽能查看 indexer 版本，但 `-v`/`--version` 不受支持。
* [Issue #662](https://github.com/manticoresoftware/manticoresearch/issues/662) 通过 systemd 启动 Manticore 时默认无限 mlock 限制。
* [Commit 63c8](https://github.com/manticoresoftware/manticoresearch/commit/63c8cd05cf399e705c2c14939411a92cb784735a) 自旋锁改造为协程读写锁的操作队列。

* [Commit 4113](https://github.com/manticoresoftware/manticoresearch/commit/41130ce30d1734bbf8e6b8b63c4e815caf9464f2) 环境变量 `MANTICORE_TRACK_RT_ERRORS` 有助于调试实时段损坏。
### 重大变更
* Binlog 版本已升级，之前版本的 binlog 无法回放，因此请确保升级期间干净停止 Manticore Search：停止之前的实例后，`/var/lib/manticore/binlog/` 目录中除了 `binlog.meta` 外不应有 binlog 文件。
* [提交 3f65](https://github.com/manticoresoftware/manticoresearch/commit/3f659f36e87d99ee262205a8eae4663a255be630) 在 `show threads option format=all` 中新增了列 "chain"。它显示了一些任务信息票据的堆栈，最适合用于性能分析需求，因此如果你解析 `show threads` 输出，请注意新增的列。
* `searchd.workers` 自 3.5.0 起已废弃，现在为不推荐使用状态，如果你的配置文件中仍存在该项，启动时将触发警告。Manticore Search 会启动，但会有警告提示。
* 如果你使用 PHP 和 PDO 访问 Manticore，你需要设置 `PDO::ATTR_EMULATE_PREPARES`
### Bug 修复
* ❗[Issue #650](https://github.com/manticoresoftware/manticoresearch/issues/650) Manticore 4.0.2 相较于 Manticore 3.6.3 速度变慢。4.0.2 在批量插入方面比之前版本快，但在单文档插入方面明显变慢。该问题已在 4.2.0 修复。
* ❗[提交 22f4](https://github.com/manticoresoftware/manticoresearch/commit/22f4141bed1411c83684ae6e0bae832071cce3df) RT 索引在高强度 REPLACE 负载下可能损坏，或导致崩溃
* [提交 03be](https://github.com/manticoresoftware/manticoresearch/commit/03be91e4f720a3f20bee0278dd8f0d44abd3ef0e) 修正了合并 groupers 和 group N 排序器时的平均数计算；修正了聚合结果合并
* [提交 2ea5](https://github.com/manticoresoftware/manticoresearch/commit/2ea575d34efe38661f14d858f2b552456f29fe42) `indextool --check` 可能崩溃
* [提交 7ec7](https://github.com/manticoresoftware/manticoresearch/commit/7ec76d4abf01b5fca6f10a0000fe64d5d8e5fa52) 由于 UPDATE 导致的内存耗尽问题
* [提交 658a](https://github.com/manticoresoftware/manticoresearch/commit/658a727e41f3c3fbcde9754149528f34ed76ba19) 守护进程在 INSERT 时可能挂起
* [提交 46e4](https://github.com/manticoresoftware/manticoresearch/commit/46e42b9b386e31e53247ed634e158b533e28063e) 守护进程在关闭时可能挂起
* [提交 f8d7](https://github.com/manticoresoftware/manticoresearch/commit/f8d7d5177cc83d250b493b4cfc619732b29cf072) 守护进程在关闭时可能崩溃
* [提交 733a](https://github.com/manticoresoftware/manticoresearch/commit/733accf165142a1d1574689525ccca8907971158) 守护进程在崩溃时可能挂起
* [提交 f7f8](https://github.com/manticoresoftware/manticoresearch/commit/f7f8bd8c0dbb23a59a88502dcba16b0918fb2330) 守护进程启动时尝试用无效节点列表重新加入集群可能崩溃
* [提交 1401](https://github.com/manticoresoftware/manticoresearch/commit/140155613821ecaddcd1a3dfc4f7369b0df6aef7) 在 RT 模式下，分布式索引在启动时无法解析某一代理节点时可能被遗忘
* [Issue #683](https://github.com/manticoresoftware/manticoresearch/issues/683) attr bit(N) engine='columnar' 失败
* [Issue #682](https://github.com/manticoresoftware/manticoresearch/issues/682) 创建表失败，但目录会保留
* [Issue #663](https://github.com/manticoresoftware/manticoresearch/issues/663) 配置失败，提示未知键名 'attr_update_reserve'
* [Issue #632](https://github.com/manticoresoftware/manticoresearch/issues/632) Manticore 在批量查询时崩溃

* [Issue #679](https://github.com/manticoresoftware/manticoresearch/issues/679) 批量查询在 v4.0.3 中再次导致崩溃

* [提交 f7f8](https://github.com/manticoresoftware/manticoresearch/commit/f7f8bd8c0dbb23a59a88502dcba16b0918fb2330) 修复了守护进程启动时因重新加入集群时无效节点列表导致崩溃的问题
* [Issue #643](https://github.com/manticoresoftware/manticoresearch/issues/643) Manticore 4.0.2 在一批插入后不接受连接
* [Issue #635](https://github.com/manticoresoftware/manticoresearch/issues/635) 使用 ORDER BY JSON.field 或字符串属性的 FACET 查询可能崩溃
* [Issue #634](https://github.com/manticoresoftware/manticoresearch/issues/634) 查询时使用 packedfactors 导致SIGSEGV崩溃
* [提交 4165](https://github.com/manticoresoftware/manticoresearch/commits/41657f15f8fcf1e93cfad7f835bcf705d6e8349e) morphology_skip_fields 未被 create table 支持
## 版本 4.0.2，2021 年 9 月 21 日
### 主要新功能
- **完全支持 [Manticore Columnar Library](https://github.com/manticoresoftware/columnar)**。之前 Manticore Columnar Library 仅支持普通索引。现在支持：
  - 实时索引中的 `INSERT`、`REPLACE`、`DELETE`、`OPTIMIZE`

  - 复制

  - `ALTER` 操作
  - `indextool --check`
- **自动索引压缩** ([Issue #478](https://github.com/manticoresoftware/manticoresearch/issues/478))。终于，你不必再通过手动调用 OPTIMIZE 或使用定时任务或其他自动手段来执行。Manticore 现在会自动且默认执行该操作。你可以通过全局变量 [optimize_cutoff](Server_settings/Setting_variables_online.md) 设置默认压缩阈值。
- **块快照与锁系统改造**。这些变化初看可能看不出明显差异，但实质上显著提升了实时索引中诸多操作的行为表现。简而言之，之前大多数 Manticore 数据操作依赖大量锁机制，而现在改为使用磁盘块快照。
- **实时索引中批量 INSERT 的性能大幅提升**。例如，在[Hetzner 的 AX101 服务器](https://www.hetzner.com/dedicated-rootserver/ax101)（SSD，128 GB 内存，AMD Ryzen™ 9 5950X (16*2 核)）上，**3.6.0 版本每秒插入文档达到 236K 条**，表结构为 `name text, email string, description text, age int, active bit(1)`（默认 `rt_mem_limit`，批次大小 25000，16 个并发插入工作线程，总插入 1600 万资料）。4.0.2 版本在相同的并发/批次/数量设置下达到 **357K 条每秒**。
  <details>
  - 读操作（如 SELECT、复制）基于快照执行

  - 仅更改内部索引结构而不修改模式/文档的操作（例如合并RAM段、保存磁盘块、合并磁盘块）使用只读快照执行，最终替换现有块

  - 针对现有块执行UPDATE和DELETE，但在可能发生合并的情况下，写入会被收集，然后应用于新的块
  - UPDATE对每个块按顺序获取排他锁。合并在进入从块收集属性阶段时获取共享锁。因此，同时最多只有一个（合并或更新）操作可以访问块的属性。

  - 当合并进入需要属性的阶段时，会设置一个特殊标志。UPDATE完成后，会检查该标志，如果设置了，整个更新会存储在一个特殊集合中。最终在合并完成时，应用存储的更新到新生成的磁盘块。
  - ALTER通过排他锁运行
  - 复制作为普通读取操作运行，但另外会在SST前保存属性，并禁止在SST期间更新
  </details>
- **[ALTER](Updating_table_schema_and_settings.md) 可以添加/删除全文字段**（在RT模式下）。以前它只能添加/删除一个属性。


- 🔬 **实验性功能：全扫描查询的伪分片** - 允许并行化任何非全文搜索查询。你现在无需手动准备分片，只需启用新选项 [searchd.pseudo_sharding](Server_settings/Searchd.md#pseudo_sharding)，即可期待非全文搜索查询响应时间最多降低至`CPU核心数`倍。注意它可能轻易占满所有现有CPU核心，因此如果你不仅关注延迟还关注吞吐量，请谨慎使用。
### 次要更改

<!-- example -->
- 通过[APT仓库](Installation/Debian_and_Ubuntu.md#APT-repository)支持Linux Mint和Ubuntu Hirsute Hippo
- 在某些情况下，HTTP接口的id更新在大索引中更快（取决于id分布）

- [671e65a2](https://github.com/manticoresoftware/lemmatizer-uk/commit/671e65a296800537123a3f8b324eeaa3f58b9632) - 为lemmatizer-uk添加了缓存
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
- [systemd的自定义启动标志](Starting_the_server/Linux.md#Custom-startup-flags-using-systemd)。现在如果需要用特定启动标志运行Manticore，您无需手动启动searchd

- 新函数 [LEVENSHTEIN()](Functions/String_functions.md#LEVENSHTEIN%28%29) 用于计算莱文斯坦距离
- 新增searchd启动标志 [searchd startup flags](Starting_the_server/Manually.md#searchd-command-line-options) `--replay-flags=ignore-trx-errors` 和 `--replay-flags=ignore-all-errors`，允许在binlog损坏时仍能启动searchd
- [Issue #621](https://github.com/manticoresoftware/manticoresearch/issues/621) - 公开RE2的错误信息
- 对由本地普通索引组成的分布式索引，提供更精确的 [COUNT(DISTINCT)](Searching/Grouping.md#COUNT%28DISTINCT-field%29)
- [FACET DISTINCT](Searching/Faceted_search.md#Faceting-without-duplicates) 用于在做分面搜索时去除重复项
- [精确形式修饰符](Searching/Full_text_matching/Operators.md#Exact-form-modifier) 不再依赖 [形态学](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology)，并支持开启了 [中缀/前缀](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) 搜索的索引
### 重大变更
- 新版本能读取旧索引，但旧版本不能读取Manticore 4的索引

- 移除了隐式按id排序。如有需要请显式排序
- `charset_table` 默认值从 `0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451` 改为 `non_cjk`
- `OPTIMIZE` 操作自动执行。如果不需要，请确保在配置文件中 `searchd` 部分设置 `auto_optimize=0`
- [Issue #616](https://github.com/manticoresoftware/manticoresearch/issues/616) `ondisk_attrs_default` 被废弃并已经移除
- 对贡献者：Linux构建现在改用Clang编译器，根据我们的测试，其构建的Manticore Search和Manticore Columnar Library更快
- 如果搜索查询中未指定 [max_matches](Searching/Options.md#max_matches)，它会隐式更新为性能所需的最小值以适应新列存储。这可能影响 [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META) 的 `total` 指标，但不影响表示实际找到文档数的 `total_found`
### 从Manticore 3迁移
- 请确保您完整正常停止Manticore 3：
  - `/var/lib/manticore/binlog/` 中不应有binlog文件（该目录应仅包含 `binlog.meta`）

  - 否则Manticore 4将无法应答这些binlog对应的索引
- 新版本能读取旧索引，但旧版本不能读取Manticore 4的索引，因此请确保备份，以便于轻松回滚
- 若运行复制集群，请确保：
  - 先完整正常停止所有节点
  - 然后用 `--new-cluster` 启动最后停止的节点（Linux下使用工具 `manticore_new_cluster` 运行）
  - 详情请参见 [重启集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster)
### Bug修复
- 修复了大量复制相关的问题：
  - [Commit 696f](https://github.com/manticoresoftware/manticoresearch/commit/696f8649535f2b5285b878c9581c0d2b1139ae09) - 修复了joiner节点在SST期间带有活动索引时的崩溃；加入了在写入文件块时的sha1校验以加速索引加载；加入了在索引加载时对变更索引文件的轮替；加入了当活动索引被donor节点的新索引替换时，移除joiner节点索引文件；加入donor节点发送文件和块时的复制日志点
  - [Commit b296](https://github.com/manticoresoftware/manticoresearch/commit/b296c55af68314a8fc66f8b104e7301d673c3b68) - 地址不正确时 JOIN CLUSTER 导致崩溃
  - [Commit 418b](https://github.com/manticoresoftware/manticoresearch/commit/418bf880a5e05ffbb68f311a1c23006a9df3220e) - 在大型索引的初始复制期间，加入节点可能因 `ERROR 1064 (42000): invalid GTID, (null)` 失败，且当另一个节点加入时，捐赠者节点可能变得无响应
  - [Commit 6fd3](https://github.com/manticoresoftware/manticoresearch/commit/6fd350d28e4905211cf8a7298cbf7cdda101c83d) - 大型索引的哈希计算可能错误，导致复制失败
  - [Issue #615](https://github.com/manticoresoftware/manticoresearch/issues/615) - 集群重启时复制失败
- [Issue #574](https://github.com/manticoresoftware/manticoresearch/issues/574) - `indextool --help` 未显示参数 `--rotate`
- [Issue #578](https://github.com/manticoresoftware/manticoresearch/issues/578) - searchd 空闲时约一天后 CPU 使用率过高
- [Issue #587](https://github.com/manticoresoftware/manticoresearch/issues/587) - 立即刷新 .meta 文件
- [Issue #617](https://github.com/manticoresoftware/manticoresearch/issues/617) - manticore.json 被清空
- [Issue #618](https://github.com/manticoresoftware/manticoresearch/issues/618) - searchd --stopwait 在 root 下失败，同时修复 systemctl 行为（之前 ExecStop 显示失败，并且等待时间不足以让 searchd 正确停止）
- [Issue #619](https://github.com/manticoresoftware/manticoresearch/issues/619) - INSERT/REPLACE/DELETE 与 SHOW STATUS，`command_insert`、`command_replace` 等显示错误指标
- [Issue #620](https://github.com/manticoresoftware/manticoresearch/issues/620) - 普通索引的 `charset_table` 默认值错误
- [Commit 8f75](https://github.com/manticoresoftware/manticoresearch/commit/8f7536887d4b58a5bab5647fc067b2d0482c2038) - 新的磁盘块未被 mlock
- [Issue #607](https://github.com/manticoresoftware/manticoresearch/issues/607) - 当无法通过名称解析节点时，Manticore 集群节点崩溃


- [Issue #623](https://github.com/manticoresoftware/manticoresearch/issues/623) - 更新索引的复制可能导致未定义状态
- [Commit ca03](https://github.com/manticoresoftware/manticoresearch/commit/ca03d2280c5197e9b311f0f03f551e3b702a8130) - 当索引普通索引源带有 json 属性时，indexer 可能会挂起

- [Commit 53c7](https://github.com/manticoresoftware/manticoresearch/commit/53c753054558ff3f82c7af365273d32537c5338b) - 修正了 PQ 索引中的不等于表达式过滤器
- [Commit ccf9](https://github.com/manticoresoftware/manticoresearch/commit/ccf94e022ba6763d6286ab5bac767b6f6a846846) - 修复了列表查询超过1000条符合时的选择窗口问题。`SELECT * FROM pq ORDER BY id desc LIMIT 1000 , 100 OPTION max_matches=1100` 以前不工作
- [Commit a048](https://github.com/manticoresoftware/manticoresearch/commit/a0483fe9bef3c45dffe96f5df4f4382aee29c072) - 对 Manticore 的 HTTPS 请求可能引发 "max packet size(8388608) exceeded" 警告
- [Issue #648](https://github.com/manticoresoftware/manticoresearch/issues/648) - Manticore 3 在字符串属性更新数次后可能会挂起

## 版本 3.6.0，2021 年 5 月 3 日
**Manticore 4 正式发布前的维护版本**
### 主要新特性
- 支持用于普通索引的 [Manticore 列式库](https://github.com/manticoresoftware/columnar/)。普通索引新增设置 [columnar_attrs](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#columnar_attrs)
- 支持 [乌克兰语词形还原器](https://github.com/manticoresoftware/lemmatizer-uk)
- 直方图重新设计。在构建索引时，Manticore 会为每个字段构建直方图，用以加速过滤。3.6.0 版本彻底改进了算法，当数据量大且过滤操作频繁时，可以获得更高性能。
### 次要更改
- 新工具 `manticore_new_cluster [--force]`，方便通过 systemd 重启复制集群
- `indexer --merge` 新增参数 [--drop-src](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Merging_tables.md#Merging-tables)

- 新模式 [blend_mode='trim_all'](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology)
- 追加对使用反引号转义 JSON 路径的支持 [支持文档](Searching/Full_text_matching/Escaping.md#Escaping-json-node-names-in-SQL)
- [indextool --check](Miscellaneous_tools.md#indextool) 增加支持 RT 模式

- SELECT/UPDATE 新增 [FORCE/IGNORE INDEX(id)](Data_creation_and_modification/Updating_documents/UPDATE.md#Updates-via-SQL) 语法
- 合并磁盘块的块ID现在唯一
- 新增 [indextool --check-disk-chunk CHUNK_NAME](Miscellaneous_tools.md#indextool)
### 优化
- [更快的 JSON 解析](https://github.com/manticoresoftware/manticoresearch/commit/0c25fc1788e3f77f60cb7fb8f1623d8466bba606) ，测试表明查询如 `WHERE json.a = 1` 时延迟降低 3-4%
- 非公开命令 `DEBUG SPLIT`，为自动分片/重平衡做准备
### Bug 修复
- [Issue #584](https://github.com/manticoresoftware/manticoresearch/issues/524) - 不准确且不稳定的 FACET 结果
- [Issue #506](https://github.com/manticoresoftware/manticoresearch/issues/506) - 使用 MATCH 时出现奇怪行为，受影响用户需重建索引，该问题出在索引构建阶段
- [Issue #387](https://github.com/manticoresoftware/manticoresearch/issues/387) - 使用 SNIPPET() 函数查询时偶发核心转储
- 针对处理复杂查询的栈优化：
  - [Issue #469](https://github.com/manticoresoftware/manticoresearch/issues/469) - SELECT 导致崩溃转储
  - [e8420cc7](https://github.com/manticoresoftware/manticoresearch/tree/e8420cc789149c84731be0874be916e33b263bd6) - 对过滤树的栈大小检测
- [Issue #461](https://github.com/manticoresoftware/manticoresearch/issues/461) - 使用 IN 条件的更新未正确生效
- [Issue #464](https://github.com/manticoresoftware/manticoresearch/issues/464) - 在 CALL PQ 之后立即执行 SHOW STATUS 返回 - [Issue #481](https://github.com/manticoresoftware/manticoresearch/issues/481) - 修复静态二进制构建
- [Issue #502](https://github.com/manticoresoftware/manticoresearch/issues/502) - 多查询中的错误
- [Issue #514](https://github.com/manticoresoftware/manticoresearch/issues/514) - 使用 'create table' 时无法使用不寻常的列名
- [Commit d1db](https://github.com/manticoresoftware/manticoresearch/commit/d1dbe771288328c963ca8512f564c6b7cf4f466c) - 在回放带有字符串属性更新的 binlog 时守护进程崩溃；将 binlog 版本设置为 10
- [Commit 775d](https://github.com/manticoresoftware/manticoresearch/commit/775d0555562f121911d3c30085947e743fadac2d)  - 修复表达式堆栈帧检测运行时错误（测试 207）
- [Commit 4795](https://github.com/manticoresoftware/manticoresearch/commit/4795dc49194c7745761bb5006ecb266e8a9224fa)  - 空存储查询的穿透索引过滤器和标签为空（测试 369）
- [Commit c3f0](https://github.com/manticoresoftware/manticoresearch/commit/c3f0bf4dca0e4eae6ea899dd566878df57eba085)  - 长延迟高错误率网络下复制 SST 流中断（不同数据中心复制）；更新复制命令版本至 1.03
- [Commit ba2d](https://github.com/manticoresoftware/manticoresearch/commit/ba2d6619b8107f3fb177741bbdff3709bac248cd)  - 集群加入后写操作时 joiner 锁定集群（测试 385）
- [Commit de4d](https://github.com/manticoresoftware/manticoresearch/commit/de4dcb9ff6c316bc2a9ef6c62eb64769f04ee103)  - 精确修改符的通配符匹配（测试 321）
- [Commit 6524](https://github.com/manticoresoftware/manticoresearch/commit/6524fc6af032a0932bd42c37874a619e1fb7f075)  - docid 检查点与 docstore
- [Commit f4ab](https://github.com/manticoresoftware/manticoresearch/commit/f4ab83c291e65a5873d271c67b5491f629544f1d)  - 解析无效 XML 时索引器行为不一致
- [Commit 7b72](https://github.com/manticoresoftware/manticoresearch/commit/7b727e22fe01b1a180d4a8d3a3fea92c18eac707)  - 带 NOTNEAR 的存储穿透查询无限运行（测试 349）
- [Commit 812d](https://github.com/manticoresoftware/manticoresearch/commit/812dab745f0c972ecde5d5b3d4a313cd6e1e143e)  - 以通配符开头的短语权重错误
- [Commit 1771](https://github.com/manticoresoftware/manticoresearch/commit/1771afc669c75670b3511bf02ab9bfc8982b94d2)  - 带通配符的穿透查询在匹配时生成无载荷的词项导致命中交错且匹配失败（测试 417）
- [Commit aa0d](https://github.com/manticoresoftware/manticoresearch/commit/aa0d8c2bcb41428d41dbfb66e42a872a79a4ee4b)  - 修正并行查询时 'total' 计算
- [Commit 18d8](https://github.com/manticoresoftware/manticoresearch/commit/18d81b3cfbb930b7361dae645fe13283d4fbf6ef)  - Windows 下守护进程多线程会话崩溃
- [Commit 8443](https://github.com/manticoresoftware/manticoresearch/commit/84432f23d6fbf7a7d0b67c8c9412eb5933b03596)  - 一些索引设置无法复制
- [Commit 9341](https://github.com/manticoresoftware/manticoresearch/commit/93411fe6af4246ca4acce758944d513a1660b92b)  - 高速添加新事件时 netloop 有时冻结，因原子 'kick' 事件被多事件同时处理，丢失实际操作
查询的状态，而非服务器状态
- [Commit d805](https://github.com/manticoresoftware/manticoresearch/commit/d805fc125244ef08d14fb3d1826e2c356862d539)  - 新刷写的磁盘块可能在提交时丢失

- [Commit 63cb](https://github.com/manticoresoftware/manticoresearch/commit/63cbf008d50979cdf7062664fb02a672a3bb1131)  - 分析器中不准确的 'net_read'
- [Commit f537](https://github.com/manticoresoftware/manticoresearch/commit/f5379bb27234b25919966c50ccb6f469ead646e2)  - 阿拉伯语（从右向左文本）穿透问题
- [Commit 49ee](https://github.com/manticoresoftware/manticoresearch/commit/49eeb4202d4ce4875c07c35087d7108c1e177328)  - 重复列名时 id 未正确选取
- [Commit refa](https://github.com/manticoresoftware/manticoresearch/commit/4b7df262bbfec9b9e95a44e72ad326ac8a786c49)  修复少见情况的网络事件崩溃
- [e8420cc7](https://github.com/manticoresoftware/manticoresearch/tree/e8420cc789149c84731be0874be916e33b263bd6) 修正 `indextool --dumpheader`
- [Commit ff71](https://github.com/manticoresoftware/manticoresearch/commit/ff7163532ed39042c0d3c3c2b902ff41f94d7f0c) - TRUNCATE WITH RECONFIGURE 在存储字段上工作异常

### 重大变更：
- 新的 binlog 格式：升级前需要干净停止 Manticore
- 索引格式略有变化：新版能正常读取现有索引，但从 3.6.0 降级至旧版本时，新索引将不可读

- 复制格式变更：不要从旧版本复制到 3.6.0 或反向复制，必须同时升级所有节点至新版本

- `reverse_scan` 已弃用。确保 3.6.0 及以后版本中不使用此选项，否则查询将失败
- 本版本起不再提供 RHEL6、Debian Jessie 和 Ubuntu Trusty 的构建。如需支持，请[联系我们](https://manticoresearch.com/contact-us/)
### 废弃功能
- 不再隐式按照 id 排序。如依赖此功能，请相应更新查询

- 搜索选项 `reverse_scan` 已被废弃
## 版本 3.5.4，2020 年 12 月 10 日
### 新功能

- 新的 Python、Javascript 和 Java 客户端现已正式发布，详细文档见本手册。
- 实时索引的磁盘块自动丢弃。这项优化使得在[OPTIMIZing](Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE)实时索引时，当该块显然不再需要（所有文档均已删除）时，能够自动丢弃磁盘块。之前仍需要合并，现在该块可以立即丢弃。[cutoff](Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks)选项被忽略，即使实际上没有合并，也会移除过时的磁盘块。这在维护索引中的保留策略并删除较旧文档时非常有用。现在压缩此类索引将更快。
- [standalone NOT](Searching/Options.md#not_terms_only_allowed) 作为SELECT的一个选项

### 小改动
- [Issue #453](https://github.com/manticoresoftware/manticoresearch/issues/453) 新选项 [indexer.ignore_non_plain=1](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#ignore_non_plain) 在运行 `indexer --all` 且配置文件中不仅包含普通索引时非常有用。没有设置 `ignore_non_plain=1` 会收到警告和相应的退出码。
- [SHOW PLAN ... OPTION format=dot](Node_info_and_management/Profiling/Query_plan.md#Dot-format-for-SHOW-PLAN) 和 [EXPLAIN QUERY ... OPTION format=dot](Searching/Full_text_matching/Profiling.md#Profiling-without-running-a-query) 支持全文查询计划执行的可视化，方便理解复杂查询。
### 弃用
- `indexer --verbose` 已弃用，因为它从未向索引器输出添加内容
- 触发守护进程背踪转储的信号从 `USR1` 改为 `USR2`
### Bug修复
- [Issue #423](https://github.com/manticoresoftware/manticoresearch/issues/423) 西里尔字符句点调用片段保留模式不高亮
- [Issue #435](https://github.com/manticoresoftware/manticoresearch/issues/435) RTINDEX - GROUP N BY 表达式选择导致致命崩溃
- [Commit 2b3b](https://github.com/manticoresoftware/manticoresearch/commit/2b3b62bd0ce2b54544733d524a169e81f71b5388) 集群环境下 searchd 状态显示段错误


- [Commit 9dd2](https://github.com/manticoresoftware/manticoresearch/commit/9dd25c193ed2a53750a0e0e68566fd2b833fcb0c) 'SHOW INDEX index.N SETTINGS' 不支持大于9的块编号

- [Issue #389](https://github.com/manticoresoftware/manticoresearch/issues/389) 导致 Manticore 崩溃的Bug

- [Commit fba1](https://github.com/manticoresoftware/manticoresearch/commit/fba166178ff35c73a0c70d131cf3aca4a635e961) 转换器创建了损坏的索引
- [Commit eecd](https://github.com/manticoresoftware/manticoresearch/commit/eecd61d861c742471175be19a5e5455034b5db5f) stopword_step=0 与 CALL SNIPPETS()
- [Commit ea68](https://github.com/manticoresoftware/manticoresearch/commit/ea6850e449601199190f38a387c04b8a7b7e90f6) 本地索引在低 max_matches 设置时 count distinct 返回0
- [Commit 362f](https://github.com/manticoresoftware/manticoresearch/commit/362f27db3421f09a41c6cce04b5baace00fa4c4d) 使用聚合时，命中文本未返回到 hits 中
## 版本 3.5.2，2020年10月1日
### 新特性

* OPTIMIZE 将磁盘块数量减少到一定数量（默认为 `2* 核数`），而非合并为单个块。最佳块数量可通过 [cutoff](Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks) 选项控制。

* NOT 操作符现在可以单独使用。默认禁用，因为误用单一 NOT 查询可能导致慢查询。可通过设置新的 searchd 指令 [not_terms_only_allowed](Server_settings/Searchd.md#not_terms_only_allowed) 为 `0` 启用。
* 新设置 [max_threads_per_query](Server_settings/Searchd.md#max_threads_per_query) 控制查询可用的最大线程数。如果未设置，则查询最多使用 [threads](Server_settings/Searchd.md#threads) 定义的线程数。
每个 `SELECT` 查询可用 [OPTION threads=N](Searching/Options.md#threads) 覆盖全局 `max_threads_per_query` 限制线程数。
* 现在可以通过 [IMPORT TABLE](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Importing_table.md) 导入 Percolate 索引。

* HTTP API `/search` 新增通过查询节点 `aggs` 基本支持[分面](Searching/Faceted_search.md#HTTP-JSON)/[分组](Searching/Grouping.md)。

### 小改动

* 如果未声明复制监听指令，引擎将尝试使用定义的 'sphinx' 端口之后的端口，最多尝试200个端口。

* `listen=...:sphinx` 需要显式设置用于 SphinxSE 连接或 SphinxAPI 客户端。

* [SHOW INDEX STATUS](Node_info_and_management/Table_settings_and_status/SHOW_TABLE_STATUS.md) 输出新增指标：`killed_documents`、`killed_rate`、`disk_mapped_doclists`、`disk_mapped_cached_doclists`、`disk_mapped_hitlists` 和 `disk_mapped_cached_hitlists`。

* SQL 命令 `status` 现在输出 `Queue\Threads` 和 `Tasks\Threads`。

### 弃用
* `dist_threads` 已完全弃用，若仍使用将记录警告。
### Docker
官方 Docker 镜像现基于 Ubuntu 20.04 LTS
### 打包
除了常规的 `manticore` 包外，还可按组件安装 Manticore Search：

- `manticore-server-core` - 提供 `searchd`、手册页、日志目录、API 和 galera 模块。作为依赖也会安装 `manticore-common`。

- `manticore-server` - 提供 core 的自动化脚本（init.d，systemd）、以及 `manticore_new_cluster` 包装器。作为依赖也会安装 `manticore-server-core`。
- `manticore-common` - 提供配置文件、停用词、通用文档及基础目录（datadir、modules等）。
- `manticore-tools` - 提供辅助工具（`indexer`，`indextool` 等）、手册页和示例。作为依赖也会安装 `manticore-common`。
- `manticore-icudata`(RPM) 或 `manticore-icudata-65l`(DEB) - 提供 ICU 数据文件以供 ICU 形态学使用。
- `manticore-devel`（RPM）或 `manticore-dev`（DEB）- 提供 UDF 的开发头文件。
### Bug 修复
1. [Commit 2a47](https://github.com/manticoresoftware/manticoresearch/commit/2a474dc1a26e8b0f8aaaae95669caf2f1d4b7746) RT 索引中不同块的 grouper 守护进程崩溃
2. [Commit 57a1](https://github.com/manticoresoftware/manticoresearch/commit/57a19e5ad5663ef0ca7436595218fb1221d28c8e) 为空的远程文档的快速通道
3. [Commit 07dd](https://github.com/manticoresoftware/manticoresearch/commit/07dd3f313c63fb82c22092f9907ef24e3475250e) 表达式调用堆栈帧检测运行时
4. [Commit 08ae](https://github.com/manticoresoftware/manticoresearch/commit/08ae357cf1012bc8e2da54c20b205b592efda3d4) 在 percolate 索引中匹配超过 32 个字段
5. [Commit 16b9](https://github.com/manticoresoftware/manticoresearch/commit/16b9390fd4cdb07a77ac4497adb935573a1710e5) 复制监听端口范围
6. [Commit 5fa6](https://github.com/manticoresoftware/manticoresearch/commit/5fa671affeacb6441a59c8a88479bfd423df7c81) 在 pq 上显示建表语句
7. [Commit 54d1](https://github.com/manticoresoftware/manticoresearch/commit/54d133b6449105a9fb0168db3f1fbb05fb5aa1f6) HTTPS 端口行为
8. [Commit fdbb](https://github.com/manticoresoftware/manticoresearch/commit/fdbbe5245cc296cc5c1ae3ae2fb9cb08fb66a248) 替换文档存储行时混合问题
9. [Commit afb5](https://github.com/manticoresoftware/manticoresearch/commit/afb53f648ef4e64fa1776c58e66f6e716ac730ad) 将 TFO 不可用信息级别切换为 'info'
10. [Commit 59d9](https://github.com/manticoresoftware/manticoresearch/commit/59d94cefc6e88af360b8046726a16b7eaa9f0b71) strcmp 无效使用导致崩溃
11. [Commit 04af](https://github.com/manticoresoftware/manticoresearch/commit/04af0349c1e784c2230d204592d6272b440bb375) 向包含系统（停用词）文件的集群添加索引
12. [Commit 5014](https://github.com/manticoresoftware/manticoresearch/commit/50148b4eae9f3a924082005cf5d2f9c8339af7f5) 合并具有大字典的索引；RT 优化大磁盘块
13. [Commit a2ad](https://github.com/manticoresoftware/manticoresearch/commit/a2adf1582843f63d4dcc1dab8f7598e0917094a2) Indextool 可以从当前版本导出元数据

14. [Commit 69f6](https://github.com/manticoresoftware/manticoresearch/commit/69f6d5f77c64b97e3f395aea8c555b11c77fb732) GROUP N 中分组顺序问题

15. [Commit 24d5](https://github.com/manticoresoftware/manticoresearch/commit/24d5d80f508576794b751d70f9f9b524564e3c14) 握手后明确刷新 SphinxSE
16. [Commit 31c4](https://github.com/manticoresoftware/manticoresearch/commit/31c4d78ac03ee85d9a02b86075448a04ac80f9e2) 避免不必要时复制巨大的描述文本
17. [Commit 2959](https://github.com/manticoresoftware/manticoresearch/commit/2959e2caa8610f98fcdf2e5a82160f58687f4a27) show threads 中负时间问题
18. [Commit f0b3](https://github.com/manticoresoftware/manticoresearch/commit/f0b357102c70f3ac1d7da260c60b67322940e647) Token 过滤插件对零位置增量的处理
19. [Commit a49e](https://github.com/manticoresoftware/manticoresearch/commit/a49e5bc13ea763e6f01dd5993e891858a84d1846) 多重匹配时将 'FAIL' 改为 'WARNING'
## 版本 3.5.0，2020年7月22日
### 主要新特性：

* 本版本耗时较长，因为我们一直在努力将多任务模式从线程改为 **协程**。这使配置更简单，查询并行更加直接：Manticore 仅使用给定数量的线程（见新设置 [threads](Server_settings/Searchd.md#threads)），新模式确保以最优方式进行。

* [高亮显示](Searching/Highlighting.md#Highlighting-options)的变动：

  - 任何支持多个字段的高亮（`highlight({},'field1, field2'`）或 JSON 查询中的 `highlight`）现在默认按字段应用限制。

  - 任何处理纯文本的高亮（`highlight({}, string_attr)` 或 `snippet()`）现在对整个文档应用限制。
  - 可以通过 `limits_per_field=0` 选项（默认是 `1`）将 [per-field limits](Searching/Highlighting.md#limits_per_field) 切换为全局限制。
  - 通过 HTTP JSON 进行高亮时，默认 [allow_empty](Searching/Highlighting.md#allow_empty) 为 `0`。
* 同一个端口现在[可以用于](Server_settings/Searchd.md#listen) http、https 和二进制 API（以接受来自远程 Manticore 实例的连接）。仍需 `listen = *:mysql` 来支持基于 mysql 协议的连接。Manticore 现在自动检测客户端连接类型，MySQL 除外（因协议限制）。
* 在 RT 模式下，字段现在可同时为[文本和字符串属性](Creating_a_table/Data_types.md#String) - [GitHub Issue #331](https://github.com/manticoresoftware/manticoresearch/issues/331)。
  在[普通模式](Read_this_first.md#Real-time-mode-vs-plain-mode)中称为 `sql_field_string`。现在它在实时索引的[RT 模式](Read_this_first.md#Real-time-mode-vs-plain-mode)中也可用。示例如下：
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
* 现在支持[字符串属性的高亮](Searching/Highlighting.md#Highlighting-via-SQL)。
* SQL 接口支持 SSL 和压缩
* 支持 mysql 客户端的 [`status`](Node_info_and_management/Node_status.md#STATUS) 命令。
* [复制](Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Setting-up-replication)现可复制外部文件（停用词、例外词等）。
* 过滤操作符[`in`](Searching/Filters.md#Set-filters)现在可以通过HTTP JSON接口使用。
* HTTP JSON中的[`expressions`](Searching/Expressions.md#expressions)。
* [现在可以在RT模式下动态更改`rt_mem_limit`](https://github.com/manticoresoftware/manticoresearch/issues/344)，即可以执行`ALTER ... rt_mem_limit=<new value>`。

* 现在可以使用[独立的CJK字符集表](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology)：`chinese`、`japanese`和`korean`。
* [thread_stack](Server_settings/Searchd.md#thread_stack) 现在限制最大线程栈大小，而非初始大小。
* 改进了`SHOW THREADS`输出。
* 在`SHOW THREADS`中显示长时间`CALL PQ`的进度。
* cpustat、iostat、coredump可以通过运行时的[SET](Server_settings/Setting_variables_online.md#SET)命令修改。

* 实现了`SET [GLOBAL] wait_timeout=NUM`，
### 重大变更：
* **索引格式已更改。** 3.5.0版本构建的索引无法被低于3.5.0版本的Manticore加载，但Manticore 3.5.0支持旧格式。
* [`INSERT INTO PQ VALUES()`](Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md)（即未提供列列表）之前要求值必须是`(query, tags)`。现已更改为`(id,query,tags,filters)`。如果希望自动生成id，可以将id设置为0。
* [`allow_empty=0`](Searching/Highlighting.md#allow_empty)现在是通过HTTP JSON接口高亮的默认设置。
* 在`CREATE TABLE`/`ALTER TABLE`中，只允许使用外部文件的绝对路径（如停用词、例外词等）。
### 弃用事项：
* `ram_chunks_count`在`SHOW INDEX STATUS`中重命名为`ram_chunk_segments_count`。
* `workers`已废弃。现在仅有一种workers模式。
* `dist_threads`已废弃。所有查询现在尽可能并行运行（受`threads`和`jobs_queue_size`限制）。
* `max_children`已废弃。使用[threads](Server_settings/Searchd.md#threads)来设置Manticore使用的线程数（默认设置为CPU核心数）。
* `queue_max_length`已废弃。如确有需要，请使用[jobs_queue_size](Server_settings/Searchd.md#jobs_queue_size)微调内部任务队列大小（默认无限制）。
* 所有`/json/*`端点现在均可不带`/json/`路径访问，如`/search`、`/insert`、`/delete`、`/pq`等。
* `field`（表示“全文字段”）在`describe`中重命名为“text”。
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
* 西里尔字母`и`不再映射到`i`，在默认的`non_cjk`字符集表中（此改动主要影响俄语词干提取和词形还原功能）。
* `read_timeout`被废弃。请改用[network_timeout](Server_settings/Searchd.md#network_timeout)，它控制读写超时。
### 软件包
* 提供Ubuntu Focal 20.04官方软件包
* deb包名称由`manticore-bin`更改为`manticore`
### 修复的Bug：
1. [Issue #351](https://github.com/manticoresoftware/manticoresearch/issues/351) searchd内存泄漏
2. [Commit ceab](https://github.com/manticoresoftware/manticoresearch/commit/ceabe44f) snippet中的轻微越界读取
3. [Commit 1c3e](https://github.com/manticoresoftware/manticoresearch/commit/1c3e84a3) 危险的本地变量写入导致崩溃查询
4. [Commit 26e0](https://github.com/manticoresoftware/manticoresearch/commit/26e094ab) 测试226中的轻微排序器内存泄漏
5. [Commit d2c7](https://github.com/manticoresoftware/manticoresearch/commit/d2c7f86a) 测试226中的严重内存泄漏
6. [Commit 0dd8](https://github.com/manticoresoftware/manticoresearch/commit/0dd80122) 集群显示节点同步，但`count(*)`结果不同
7. [Commit f1c1](https://github.com/manticoresoftware/manticoresearch/commit/f1c1ac3f) 美化：日志中重复及有时丢失的警告消息
8. [Commit f1c1](https://github.com/manticoresoftware/manticoresearch/commit/f1c1ac3f) 美化：日志中的(null)索引名
9. [Commit 359d](https://github.com/manticoresoftware/manticoresearch/commit/359dbd30) 无法检索超过7000万结果
10. [Commit 19f3](https://github.com/manticoresoftware/manticoresearch/commit/19f328ee) 无法使用无列语法插入PQ规则
11. [Commit bf68](https://github.com/manticoresoftware/manticoresearch/commit/bf685d5d) 向集群索引插入文档时误导性错误信息
12. [Commit 2cf1](https://github.com/manticoresoftware/manticoresearch/commit/2cf18c83) `/json/replace`和`json/update`返回指数形式的id
13. [Issue #324](https://github.com/manticoresoftware/manticoresearch/issues/324) 在同一查询中更新json标量属性和mva
14. [Commit d384](https://github.com/manticoresoftware/manticoresearch/commit/d38409eb) RT模式下`hitless_words`不起作用
15. [Commit 5813](https://github.com/manticoresoftware/manticoresearch/commit/5813d639) RT模式下不应允许`ALTER RECONFIGURE`
16. [Commit 5813](https://github.com/manticoresoftware/manticoresearch/commit/5813d639) searchd重启后`rt_mem_limit`重置为128M
17. highlight()有时会挂起
18. [Commit 7cd8](https://github.com/manticoresoftware/manticoresearch/commit/7cd878f4) RT模式下无法使用U+码
19. [Commit 2b21](https://github.com/manticoresoftware/manticoresearch/commit/2b213de4) RT模式下词形变化的通配符无法使用
20. [Commit e9d0](https://github.com/manticoresoftware/manticoresearch/commit/e9d07e68) 修正`SHOW CREATE TABLE`与多个词形变化文件的兼容问题
21. [提交 fc90](https://github.com/manticoresoftware/manticoresearch/commit/fc90a84f) JSON 查询无 "query" 导致 searchd 崩溃
22. Manticore [官方 docker](https://hub.docker.com/r/manticoresearch/manticore) 无法从 mysql 8 索引数据
23. [提交 23e0](https://github.com/manticoresoftware/manticoresearch/commit/23e05d32) HTTP /json/insert 需要 id
24. [提交 bd67](https://github.com/manticoresoftware/manticoresearch/commit/bd679af0) `SHOW CREATE TABLE` 对 PQ 无效
25. [提交 bd67](https://github.com/manticoresoftware/manticoresearch/commit/bd679af0) `CREATE TABLE LIKE` 在 PQ 中无法正确工作
26. [提交 5eac](https://github.com/manticoresoftware/manticoresearch/commit/5eacf28f) show index status 中设置结尾行
27. [提交 cb15](https://github.com/manticoresoftware/manticoresearch/commit/cb153228) HTTP JSON 响应中 "highlight" 标题为空
28. [问题 #318](https://github.com/manticoresoftware/manticoresearch/issues/318) `CREATE TABLE LIKE` 中缀错误
29. [提交 9040](https://github.com/manticoresoftware/manticoresearch/commit/9040d22c) RT 在负载下崩溃
30. [cd512c7d ](https://github.com/manticoresoftware/manticoresearch/commit/cd512c7d ) RT 磁盘块崩溃时丢失崩溃日志
31. [问题 #323](https://github.com/manticoresoftware/manticoresearch/issues/323) 导入表失败并关闭连接
32. [提交 6275](https://github.com/manticoresoftware/manticoresearch/commit/6275316a) `ALTER reconfigure` 损坏 PQ 索引
33. [提交 9c1d](https://github.com/manticoresoftware/manticoresearch/commit/9c1d221e) 更改索引类型后 searchd 重载问题
34. [提交 71e2](https://github.com/manticoresoftware/manticoresearch/commit/71e2b5bb) 守护进程导入表时丢失文件导致崩溃
35. [问题 #322](https://github.com/manticoresoftware/manticoresearch/issues/322) 使用多个索引、分组和 ranker=none 的 select 导致崩溃
36. [提交 c3f5](https://github.com/manticoresoftware/manticoresearch/commit/c3f58490) `HIGHLIGHT()` 无法高亮字符串属性
37. [问题 #320](https://github.com/manticoresoftware/manticoresearch/issues/320) `FACET` 无法对字符串属性排序
38. [提交 4f1a](https://github.com/manticoresoftware/manticoresearch/commit/4f1a1f25) 缺少数据目录时错误
39. [提交 04f4](https://github.com/manticoresoftware/manticoresearch/commit/04f4ddd4) RT 模式下不支持 access_*
40. [提交 1c06](https://github.com/manticoresoftware/manticoresearch/commit/1c0616a2) 字符串中的坏 JSON 对象：1. `CALL PQ` 在 JSON 超过某个值时返回 "Bad JSON objects in strings: 1"。
41. [提交 32f9](https://github.com/manticoresoftware/manticoresearch/commit/32f943d6) RT 模式不一致。在某些情况下无法删除索引（未知），且无法创建索引（目录非空）。
42. [问题 #319](https://github.com/manticoresoftware/manticoresearch/issues/319) select 查询崩溃
43. [提交 22a2](https://github.com/manticoresoftware/manticoresearch/commit/22a28dd7) `max_xmlpipe2_field` = 2M 时，大字段返回警告
44. [问题 #342](https://github.com/manticoresoftware/manticoresearch/issues/342) 查询条件执行错误
45. [提交 dd8d](https://github.com/manticoresoftware/manticoresearch/commit/dd8dcab2) 简单的 2 词搜索找到了一个只包含一个词的文档
46. [提交 9091](https://github.com/manticoresoftware/manticoresearch/commit/90919e62) PQ 中无法匹配键名中带大写字母的 JSON
47. [提交 56da](https://github.com/manticoresoftware/manticoresearch/commit/56da086a) 索引器在使用 csv+docstore 时崩溃
48. [问题 #363](https://github.com/manticoresoftware/manticoresearch/issues/363) centos 7 中 json 属性使用 `[null]` 导致插入数据损坏
49. 重大 [问题 #345](https://github.com/manticoresoftware/manticoresearch/issues/345) 记录未被插入，count() 随机，"replace into" 返回 OK
50. max_query_time 导致 SELECT 过度变慢


51. [问题 #352](https://github.com/manticoresoftware/manticoresearch/issues/352) Mac OS 上主从通信失败
52. [问题 #328](https://github.com/manticoresoftware/manticoresearch/issues/328) 使用 Connector.Net/Mysql 8.0.19 连接 Manticore 时错误
53. [提交 daa7](https://github.com/manticoresoftware/manticoresearch/commit/daa760d2fe83668038318f0f384ceefc0d075e18) 修正 \0 转义并优化性能

54. [提交 9bc5](https://github.com/manticoresoftware/manticoresearch/commit/9bc5c01a493db993b90ff6c1ad51aeae028f629c) 修正 count distinct 与 json 的问题
55. [提交 4f89](https://github.com/manticoresoftware/manticoresearch/commit/4f89a965a3af0db548ae9d1763731ebaebe21f5a) 修正其他节点 drop 表失败
56. [提交 952a](https://github.com/manticoresoftware/manticoresearch/commit/952af5a5931fd72b87edc169588777778b365231) 修正紧密调用 call pq 导致的崩溃
## 版本 3.4.2，2020年4月10日
### 关键错误修复
* [提交 2ffe](https://github.com/manticoresoftware/manticoresearch/commit/2ffe2d261793f9681265bc4abe38d695e72c121e) 修复旧版本 RT 索引无法建立索引数据的问题

## 版本 3.4.0，2020年3月26日
### 主要变化
* 服务器支持两种模式：rt 模式和普通模式
   * rt 模式需要 data_dir，配置中无索引定义
   * 普通模式中索引定义在配置中；不允许 data_dir
* 复制功能仅在 rt 模式下可用

### 次要变化
* charset_table 默认非 cjk 别名
* rt 模式中全文字段默认索引并存储

* rt 模式中全文字段名称由 'field' 改为 'text'
* ALTER RTINDEX 改名为 ALTER TABLE
* TRUNCATE RTINDEX 改名为 TRUNCATE TABLE
### 新功能
* 仅存储字段

* SHOW CREATE TABLE，IMPORT TABLE
### 改进
* 更快的无锁 PQ
* /sql 支持执行任何 SQL 语句，mode=raw
* mysql 作为 mysql41 协议的别名
* data_dir 中默认 state.sql
### 错误修复
* [Commit a533](https://github.com/manticoresoftware/manticoresearch/commit/a5333644e8e0c69746a9f24da8221fc965478e4a) 修复 highlight() 中错误字段语法导致的崩溃
* [Commit 7fbb](https://github.com/manticoresoftware/manticoresearch/commit/7fbb9f2e618096289cba7f8b834530b46aa94055) 修复带有 docstore 的 RT 索引复制时服务器崩溃
* [Commit 24a0](https://github.com/manticoresoftware/manticoresearch/commit/24a04687eb43b4f4719c7c4a5fe3c19099240467) 修复对带有 infix 或 prefix 选项且无存储字段启用的索引进行高亮时崩溃
* [Commit 3465](https://github.com/manticoresoftware/manticoresearch/commit/3465c1ce9c860c09f6ccc5b83a63e3c68c241d8d) 修复关于空 docstore 和空索引的 dock-id 查找的错误提示
* [Commit a707](https://github.com/manticoresoftware/manticoresearch/commit/b8601b4144b9092be43a7caa7cd6b19cb37e14c7) 修复 #314 尾随分号的 SQL insert 命令
* [Commit 9562](https://github.com/manticoresoftware/manticoresearch/commit/95628c9b9b8f16fe1bd18599a84e9673575adf1a) 移除查询词不匹配的警告

* [Commit b860](https://github.com/manticoresoftware/manticoresearch/commit/b8601b4144b9092be43a7caa7cd6b19cb37e14c7) 修复通过 ICU 分词的片段查询问题
* [Commit 5275](https://github.com/manticoresoftware/manticoresearch/commit/5275516ca4631c584e6e0127fa161a18d2658fd0) 修复 docstore 块缓存中的查找/添加竞态条件
* [Commit f06e](https://github.com/manticoresoftware/manticoresearch/commit/f06ef97af5c7390427e82525c150535290e42c3e) 修复 docstore 中的内存泄漏
* [Commit a725](https://github.com/manticoresoftware/manticoresearch/commit/a7258ba8340e246c0d8aaeb4a7a74129eba807eb) 修复 #316 INSERT 时 LAST_INSERT_ID 返回为空的问题
* [Commit 1ebd](https://github.com/manticoresoftware/manticoresearch/commit/1ebd5bf8d74113bcfa138ce63ce11966990a4215) 修复 #317 json/update HTTP 端点支持 MVA 的数组和 JSON 属性的对象
* [Commit e426](https://github.com/manticoresoftware/manticoresearch/commit/e426950a536382a142e2a42ed207c341149ff685) 修复索引器在没有显式 ID 时导出实时索引的错误
## 版本 3.3.0，2020年2月4日

### 新特性
* 并行实时索引搜索
* EXPLAIN QUERY 命令
* 无索引定义的配置文件（测试版）
* CREATE/DROP TABLE 命令（测试版）
* indexer --print-rt - 可从源读取并打印用于实时索引的 INSERT 语句
### 改进


* 更新至 Snowball 2.0 词干处理器
* SHOW INDEX STATUS 增加 LIKE 过滤器
* 高 max_matches 下改进内存使用
* SHOW INDEX STATUS 添加 RT 索引的 ram_chunks_count 字段
* 无锁优先队列（lockless PQ）
* 将 LimitNOFILE 更改为 65536
### 修复
* [Commit 9c33](https://github.com/manticoresoftware/manticoresearch/commit/9c33aab8c1fd9059c8644d23d5f58f03010043c6) 添加索引模式中重复属性的检查 #293
* [Commit a008](https://github.com/manticoresoftware/manticoresearch/commit/a0085f9477e191da9e0e835b91583fb9cfde698e) 修复无命中词导致的崩溃
* [Commit 6895](https://github.com/manticoresoftware/manticoresearch/commit/68953740af8dca1833026d0f448c71416ed3044a) 修复 ATTACH 后松散的 docstore
* [Commit d6f6](https://github.com/manticoresoftware/manticoresearch/commit/d6f696edea53ea2b9a68251e4054e4816082c285) 修复分布式设置中的 docstore 问题
* [Commit bce2](https://github.com/manticoresoftware/manticoresearch/commit/bce2b7ec883d208f31094ee42a5b6f01d18a998f) 用 OpenHash 替换 sorter 中的 FixedHash
* [Commit e0ba](https://github.com/manticoresoftware/manticoresearch/commit/e0baf7392937969e640fb5d10e8ccb312fe36446) 修复索引定义中属性名重复的问题
* [Commit ca81](https://github.com/manticoresoftware/manticoresearch/commit/ca81114bf1ac7ea7adcc8509bdc14d1f30b4f461) 修复 HIGHLIGHT() 中的 html_strip
* [Commit 493a](https://github.com/manticoresoftware/manticoresearch/commit/493a5e916d70fd4a3f14d1ee487234434f4a7ce3) 修复 HIGHLIGHT() 中的 passage 宏
* [Commit a82d](https://github.com/manticoresoftware/manticoresearch/commit/a82d41c775692f837f267d554d7d4a4b499d69f0) 修复 RT 索引创建小盘块或大盘块时双缓冲区问题
* [Commit a404](https://github.com/manticoresoftware/manticoresearch/commit/a404c85de0d41d9d1c75b7540479d8a3af9e61aa) 修复 kqueue 事件删除问题

* [Commit 8bea](https://github.com/manticoresoftware/manticoresearch/commit/8bea0f6fd1666b817995c4dd6e67f6f1c9161638) 修复 RT 索引中 rt_mem_limit 大值下磁盘块保存的问题
* [Commit 8707](https://github.com/manticoresoftware/manticoresearch/commit/8707f0395b7ace52dc0cba9c8ed92a2bc4392932) 修复索引时浮点溢出
* [Commit a564](https://github.com/manticoresoftware/manticoresearch/commit/a56434ce71bb7714b4d8e75278aaa075b27ece80) 修复向 RT 索引插入负 ID 文档时的错误
* [Commit bbeb](https://github.com/manticoresoftware/manticoresearch/commit/bbebfd7551a68641154761cac962053b54806759) 修复基于 ranker 字段掩码的服务器崩溃
* [Commit 3809](https://github.com/manticoresoftware/manticoresearch/commit/3809cc1b0863e3a5a200dc423be3f7c38be56113) 修复使用查询缓存时崩溃
* [Commit dc2a](https://github.com/manticoresoftware/manticoresearch/commit/dc2a585bc724678679e4297362200bbd49ce32bb) 修复使用并行插入的 RT 索引 RAM 段时崩溃

## 版本 3.2.2，2019年12月19日
### 新特性
* RT 索引自动递增 ID
* 通过新的 HIGHLIGHT() 函数支持 docstore 高亮，也可用于 HTTP API
* SNIPPET() 可使用特殊函数 QUERY() 返回当前 MATCH 查询
* 高亮函数新增 field_separator 选项
### 改进和变更
* 远程节点延迟获取存储字段（大幅提升性能）
* 字符串和表达式不再破坏多查询和 FACET 优化
* RHEL/CentOS 8 构建现在使用 mariadb-connector-c-devel 提供的 mysql libclient

* ICU 数据文件随软件包一同发布，移除 icu_data_dir
* systemd 服务文件包含 'Restart=on-failure' 策略
* indextool 现在可在线检查实时索引
* 默认配置文件现为 /etc/manticoresearch/manticore.conf
* RHEL/CentOS上的服务从'searchd'重命名为'manticore'
* 移除了query_mode和exact_phrase snippet的选项
### Bug修复
* [Commit 6ae4](https://github.com/manticoresoftware/manticoresearch/commit/6ae474c7894a6bee222d5b18e59a44fdbf57843a) 修复HTTP接口上执行SELECT查询时崩溃的问题
* [Commit 5957](https://github.com/manticoresoftware/manticoresearch/commit/59577513a49eac5a4a3c5e2cb38394d3246b5d35) 修复RT索引保存磁盘块但未标记某些文档为已删除的问题
* [Commit e861](https://github.com/manticoresoftware/manticoresearch/commit/e861f0fca0e88924450695d4e9d6acff7a36558a) 修复带有dist_threads的多索引或多查询搜索时的崩溃
* [Commit 4409](https://github.com/manticoresoftware/manticoresearch/commit/440991fc977b8479800b45cdbc862eeb1ba5d965) 修复对包含宽UTF8码点的长词生成中缀时的崩溃
* [Commit 5fd5](https://github.com/manticoresoftware/manticoresearch/commit/5fd599b48bba527c023e6aa0b262dca51ffb8a1c) 修复添加socket至IOCP时的竞态条件
* [Commit cf10](https://github.com/manticoresoftware/manticoresearch/commit/cf10d7d3589f77adfaefbe52a51777bc9c67cf99) 修复bool查询与json select列表的兼容问题
* [Commit 996d](https://github.com/manticoresoftware/manticoresearch/commit/996de77f4c3ec103d965e28c36098fd07dba50c4) 修正indextool检查，报告错误的跳过列表偏移以及doc2row查找检查
* [Commit 6e3f](https://github.com/manticoresoftware/manticoresearch/commit/6e3fc9e88941c9427410f9d0adeebb7dd09a900f) 修复Indexer在大数据上生成带有负跳过列表偏移的错误索引
* [Commit faed](https://github.com/manticoresoftware/manticoresearch/commit/faed3220b423a96401982bf47117edf1f62c584e) 修复表达式中JSON仅数字转换为字符串及JSON字符串到数字的转换
* [Commit 5331](https://github.com/manticoresoftware/manticoresearch/commit/533197200249d648ebcbdaca5d605861ee566348) 修正indextool在命令行设置多个命令时退出并返回错误码
* [Commit 7955](https://github.com/manticoresoftware/manticoresearch/commit/795520ac351d22e4497ebaf6c249bf25081842b6) 修复#275 磁盘空间不足错误时binlog进入无效状态

* [Commit 2284](https://github.com/manticoresoftware/manticoresearch/commit/2284da5e3ff77bf7be4038a4d950c0580e4159c8) 修复#279 IN过滤器在JSON属性上的崩溃
* [Commit ce2e](https://github.com/manticoresoftware/manticoresearch/commit/ce2e4b4765111a7d331ffc256911f12770a8942d) 修复#281 错误的管道关闭调用
* [Commit 5355](https://github.com/manticoresoftware/manticoresearch/commit/535589ba8a37baf5c6056afb5ff969fd70476feb) 修复服务器因递归JSON属性作为字符串编码调用PQ时挂起
* [Commit a5fc](https://github.com/manticoresoftware/manticoresearch/commit/a5fc8a36e1644ee051632408ab8b53e79aeb9ff4) 修复multiand节点中文档列表游标越界问题

* [Commit a362](https://github.com/manticoresoftware/manticoresearch/commit/a3628617ef4ee89a811d4ba2c4a04cbbc298c1b5) 修复线程公共信息的获取
* [Commit f8d2](https://github.com/manticoresoftware/manticoresearch/commit/f8d2d7bbbb90af3d2ee97424f97b05927f09d42b) 修复文档存储缓存锁
## 版本 3.2.0，2019年10月17日
### 新功能
* 文档存储

* 新指令：stored_fields，docstore_cache_size，docstore_block_size，docstore_compression，docstore_compression_level
### 改进和变更
* 改进SSL支持
* 更新了non_cjk内置字符集
* 禁用UPDATE/DELETE语句在查询日志中记录SELECT
* RHEL/CentOS 8软件包
### Bug修复
* [Commit 301a](https://github.com/manticoresoftware/manticoresearch/commit/301a806b14725e842300ee225521407464374c41) 修复在RT索引的磁盘块中替换文档时崩溃
* [Commit 46c1](https://github.com/manticoresoftware/manticoresearch/commit/46c1cad8fa11c1fc09cc34b20d978fdfcd4e6d27) 修复#269 LIMIT N OFFSET M问题
* [Commit 92a4](https://github.com/manticoresoftware/manticoresearch/commit/92a46edaa196ef33f3ac2225fbdba2c2b88ee45d) 修复带显式id或id列表的DELETE语句跳过搜索的问题

* [Commit 8ca7](https://github.com/manticoresoftware/manticoresearch/commit/8ca78c138d837caab30dc6e2343a3c4d1687bb87) 修复Windows poller下netloop事件移除后索引错误
* [Commit 6036](https://github.com/manticoresoftware/manticoresearch/commit/603631e2bff5cb0acf25f68b85621742490eccd9) 修复HTTP接口JSON中的浮点数四舍五入问题
* [Commit 62f6](https://github.com/manticoresoftware/manticoresearch/commit/62f64cb9e75ec28fc3c29f8414ce9656f5580254) 修复远程snippet先检查空路径的问题；修正了Windows测试
* [Commit aba2](https://github.com/manticoresoftware/manticoresearch/commit/aba274c2c5abc1029894ba9f95b786bda93e8f22) 修复配置重载以在Windows下表现与Linux一致
* [Commit 6b8c](https://github.com/manticoresoftware/manticoresearch/commit/6b8c4242ef77c3fa4d0ccb7d76d81714b6728f0b) 修复#194 PQ以支持形态学和词干分析器
* [Commit 174d](https://github.com/manticoresoftware/manticoresearch/commit/174d312905285aa5746f4f404c53e6dc669ef42f) 修复RT已退役段管理
## 版本 3.1.2，2019年8月22日

### 新功能与改进
* HTTP API的实验性SSL支持
* CALL KEYWORDS的字段过滤器
* /json/search的max_matches
* 默认Galera gcache.size的自动调整大小
* 改进FreeBSD支持
### Bug修复
* [Commit 0a1a](https://github.com/manticoresoftware/manticoresearch/commit/0a1a2c81038794983b4e30c8aefc7cc20e89b03f) 修复RT索引复制到存在相同RT索引但路径不同的节点问题

* [Commit 4adc](https://github.com/manticoresoftware/manticoresearch/commit/4adc075294ac823289f745e2cc419f18c7dcf2e2) 修复无活动索引的刷新重新调度
* [Commit d6c0](https://github.com/manticoresoftware/manticoresearch/commit/d6c00a6f750c04021747e9c99e0f77707a4cbfc3) 改善RT/PQ索引刷新的重新调度
* [Commit d0a7](https://github.com/manticoresoftware/manticoresearch/commit/d0a7c95906b9b5ba33c98267d54501cfe27508d6) 修复#250 TSV和CSV管道源的index_field_lengths索引选项
* [提交 1266](https://github.com/manticoresoftware/manticoresearch/commit/1266d54800242709fbe2c34e72eb69b6595b23bd) 修复 indextool 在空索引上对区块索引检查错误报告的问题
* [提交 553c](https://github.com/manticoresoftware/manticoresearch/commit/553ca73ca111f28e0888d59cd48b0d6df181ecd6) 修复 Manticore SQL 查询日志中的空选择列表
* [提交 56c8](https://github.com/manticoresoftware/manticoresearch/commit/56c8584456293665ddd4ce4c94307d74473a78b9) 修复 indexer -h/--help 响应
## 版本 3.1.0，2019年7月16日
### 特性和改进
* 实时索引的复制
* 中文的 ICU 分词器
* 新形态学选项 icu_chinese
* 新指令 icu_data_dir
* 复制的多语句事务
* LAST_INSERT_ID() 和 @session.last_insert_id

* SHOW VARIABLES 中的 LIKE 'pattern'
* 多文档插入支持用于编译索引
* 为配置添加时间解析器

* 内部任务管理器
* 为文档和命中列表组件启用 mlock
* 监狱代码片段路径
### 移除
* 放弃 RLP 库支持，转用 ICU；移除所有 rlp* 指令
* 禁用使用 UPDATE 更新文档 ID
### Bug 修复
* [提交 f047](https://github.com/manticoresoftware/manticoresearch/commit/f047222339fd0e62a07e9ca8b2a829297d5db861) 修复 concat 和 group_concat 中的缺陷
* [提交 b081](https://github.com/manticoresoftware/manticoresearch/commit/b08147ee24712b5eaa51f261626b5d16c6242fdd) 修复编译索引中查询 UID 应为 BIGINT 属性类型
* [提交 4cd8](https://github.com/manticoresoftware/manticoresearch/commit/4cd85afaf76eef0ce9fa1cd073f3fed160279890) 预分配新的磁盘块失败时不再崩溃
* [提交 1a55](https://github.com/manticoresoftware/manticoresearch/commit/1a551227886474054111088412f2b2396f485652) 为 ALTER 添加缺失的 timestamp 数据类型

* [提交 f3a8](https://github.com/manticoresoftware/manticoresearch/commit/f3a8e096fd496a6b001d398aadbb781b2dbad929) 修复错误的 mmap 读取导致的崩溃
* [提交 4475](https://github.com/manticoresoftware/manticoresearch/commit/447577115192db7ac7bfea70b5ba58ee2c61229f) 修复复制中集群锁哈希问题
* [提交 ff47](https://github.com/manticoresoftware/manticoresearch/commit/ff476df955f057bd38e8b6ad6056325e9d6fbb9c) 修复复制中提供者泄漏
* [提交 58dc](https://github.com/manticoresoftware/manticoresearch/commit/58dcbb779c4c30ca1560460c78e35ba114b83716) 修复 #246 indexer 中未定义的 sigmask
* [提交 3dd8](https://github.com/manticoresoftware/manticoresearch/commit/3dd8278e747d534f69fe9c3d360eef188ab7b6d5) 修复 netloop 报告中的竞态条件
* [提交 a02a](https://github.com/manticoresoftware/manticoresearch/commit/a02aae05a3fa3d1caf2821f43c7e237b6de76336) HA 策略重新平衡器的零间隙

## 版本 3.0.2，2019年5月31日
### 改进

* 为文档和命中列表添加 mmap 读取器
* `/sql` HTTP 端点响应现与 `/json/search` 相同

* 新指令 `access_plain_attrs`、`access_blob_attrs`、`access_doclists`、`access_hitlists`
* 复制设置中新指令 `server_id`
### 移除
* 移除 HTTP `/search` 端点
### 弃用
* `read_buffer`、`ondisk_attrs`、`ondisk_attrs_default`、`mlock` 被 `access_*` 指令替代
### Bug 修复
* [提交 849c](https://github.com/manticoresoftware/manticoresearch/commit/849c16e1040f382795ba0635cb48686f9db4f2d8) 允许选择列表中以数字开头的属性名
* [提交 48e6](https://github.com/manticoresoftware/manticoresearch/commit/48e6c302ca37bec0bc49c79619c60bf787ed0708) 修复 UDF 中的多值属性，修复多值属性别名
* [提交 0555](https://github.com/manticoresoftware/manticoresearch/commit/055586a9a2ebfa8874f68099c5bc798a145fd4ef) 修复 #187 使用 SENTENCE 查询时崩溃


* [提交 93bf](https://github.com/manticoresoftware/manticoresearch/commit/93bf52f23f1c53c8a93a603387ca5463e1fc2ae8) 修复 #143 支持 MATCH() 周围的括号
* [提交 599e](https://github.com/manticoresoftware/manticoresearch/commit/599ee79ccd38cce0023a55cbce8f50c3dca62e38) 修复 ALTER cluster 语句保存集群状态问题
* [提交 230c](https://github.com/manticoresoftware/manticoresearch/commit/230c321e277514b191629538eb3f02d219113d95) 修复 ALTER index 使用 blob 属性时服务崩溃
* [提交 5802](https://github.com/manticoresoftware/manticoresearch/commit/5802b85aa93a81caea77073a3381ef912fdd677c) 修复 #196 按 ID 过滤问题
* [提交 25d2](https://github.com/manticoresoftware/manticoresearch/commit/25d2dabda4a299a29bd375405862126da9b4bfac) 丢弃模板索引上的搜索
* [提交 2a30](https://github.com/manticoresoftware/manticoresearch/commit/2a30d5b49d76faab031d195f533974938769b8e6) 修复 SQL 回复中 ID 列为普通 bigint 类型
## 版本 3.0.0，2019年5月6日
### 特性和改进
* 新的索引存储。非标量属性不再受每个索引 4GB 大小限制
* attr_update_reserve 指令
* 支持使用 UPDATE 更新字符串、JSON 和多值属性
* killlist 在索引加载时应用
* killlist_target 指令
* 多 AND 查询加速
* 更好的平均性能和 RAM 使用

* 用于升级 2.x 创建的索引的转换工具
* CONCAT() 函数
* JOIN CLUSTER cluster AT 'nodeaddress:port'
* ALTER CLUSTER posts UPDATE nodes

* node_address 指令
* SHOW STATUS 中打印节点列表

### 行为变化
* 有 killlist 的索引中，服务器不再按配置中定义的顺序轮换索引，而是遵循 killlist 目标链
* 搜索中索引排序不再决定 killlist 应用的顺序
* 文档 ID 现在是带符号的大整数

### 移除的指令
* docinfo（总是外部），inplace_docinfo_gap，mva_updates_pool
## 版本 2.8.2 GA，2019年4月2日

### 特性和改进
* 支持针对编译索引的 Galera 复制
* OPTION morphology
### 编译说明
CMake 最低版本现在是 3.13。编译需要 boost 和 libssl
开发库。
### Bug 修复
* [提交 6967](https://github.com/manticoresoftware/manticoresearch/commit/6967fedb2ef818ec1c825d482563edd05e1c9245) 修复了对于多个分布式索引查询中选择列表中多个星号引起的崩溃
* [提交 36df](https://github.com/manticoresoftware/manticoresearch/commit/36df1a407dc08263690e3492518613ace82d69ca) 修复了通过 Manticore SQL 接口传输大包的问题[#177](https://github.com/manticoresoftware/manticoresearch/issues/177)
* [提交 5793](https://github.com/manticoresoftware/manticoresearch/commit/57932aec734583fa93359faaf5034b2e2c9d352b) 修复了 RT 优化时带有更新的 MVA 引起的服务器崩溃[#170](https://github.com/manticoresoftware/manticoresearch/issues/170)

* [提交 edb2](https://github.com/manticoresoftware/manticoresearch/commit/edb24b870423add86eba471d361e0e5aff098b18) 修复了在 SIGHUP 配置重载后，RT 索引移除导致 binlog 删除时服务器崩溃问题
* [提交 bd3e](https://github.com/manticoresoftware/manticoresearch/commit/bd3e66e0085bc7e2e351b817dfc972fd8158fbce) 修复了 mysql 握手认证插件的有效负载
* [提交 6a21](https://github.com/manticoresoftware/manticoresearch/commit/6a217f6ed82fb10f752213b15617b9cedf1e8533) 修复了 RT 索引中 phrase_boundary 配置[#172](https://github.com/manticoresoftware/manticoresearch/issues/172)
* [提交 3562](https://github.com/manticoresoftware/manticoresearch/commit/3562f652753e4091fd7b5b0f65b58341c8cbcb31) 修复了将索引自身 ATTACH 时的死锁问题[#168](https://github.com/manticoresoftware/manticoresearch/issues/168)
* [提交 250b](https://github.com/manticoresoftware/manticoresearch/commit/250b3f0e74ad18dda34ba080aa13ca87798a9ac9) 修复了服务器崩溃后 binlog 保存空元数据的问题

* [提交 4aa6](https://github.com/manticoresoftware/manticoresearch/commit/4aa6c69ad3a181cd7c5dafb990fb528121f68e59) 修复了由于带有磁盘块的 RT 索引中 sorter 字符串导致的服务器崩溃
## 版本 2.8.1 GA, 2019年3月6日
### 特性和改进
* SUBSTRING_INDEX()
* 为 percolate 查询支持 SENTENCE 和 PARAGRAPH
* Debian/Ubuntu 的 systemd 生成器；同时添加 LimitCORE 以允许生成 core dump
### Bug 修复
* [提交 84fe](https://github.com/manticoresoftware/manticoresearch/commit/84fe7405d17b59f4dcde15a6c8e8503923503def) 修复了匹配模式为 all 且全文查询为空时服务器崩溃
* [提交 daa8](https://github.com/manticoresoftware/manticoresearch/commit/daa88b579fa373d5d2e869e43a2e178363b0fef1) 修复了删除静态字符串时的崩溃
* [提交 2207](https://github.com/manticoresoftware/manticoresearch/commit/22078537dddbd9ce81a182dcc879f235b6e87004) 修复了 indextool 致命错误失败时的退出码
* [提交 0721](https://github.com/manticoresoftware/manticoresearch/commit/0721696d6780c200c65d596624a8187983fb7fcb) 修复了由于错误的精确形式检查导致前缀无匹配的问题[#109](https://github.com/manticoresoftware/manticoresearch/issues/109)
* [提交 8af8](https://github.com/manticoresoftware/manticoresearch/commit/8af810111b8e2f87bc9f378172eff1ab9725c7e7) 修复了 RT 索引配置重载问题[#161](https://github.com/manticoresoftware/manticoresearch/issues/161)
* [提交 e2d5](https://github.com/manticoresoftware/manticoresearch/commit/e2d592773f4fd702ace90070a793a842a0fec6ab) 修复了访问大型 JSON 字符串时服务器崩溃
* [提交 75cd](https://github.com/manticoresoftware/manticoresearch/commit/75cd1342d05cdc352e3dd145cdbc6f79394a165b) 修复了 JSON 文档中经索引剥离器改变的 PQ 字段导致的兄弟字段匹配错误
* [提交 e2f7](https://github.com/manticoresoftware/manticoresearch/commit/e2f775437df922674d772ed4417780492502e65a) 修复了 RHEL7 构建上解析 JSON 时的服务器崩溃
* [提交 3a25](https://github.com/manticoresoftware/manticoresearch/commit/3a25a5808feb3f8b80866991b436d6c1241618c2) 修复了当斜杠在边缘位置时 json 反转义崩溃

* [提交 be9f](https://github.com/manticoresoftware/manticoresearch/commit/be9f497872bea62dbdccd64fc28294fd7776c289) 修复了 skip_empty 选项跳过空文档且不警告其不是有效 json
* [提交 266e](https://github.com/manticoresoftware/manticoresearch/commit/266e0e7b088549722a805fc837bf101ff681a5e8) 修复了浮点数精度不足时输出8位数字的问题[#140](https://github.com/manticoresoftware/manticoresearch/issues/161)
* [提交 3f6d](https://github.com/manticoresoftware/manticoresearch/commit/3f6d23891064ee8e94030c4231497cdd7da33a6a) 修复了创建空 jsonobj
* [提交 f3c7](https://github.com/manticoresoftware/manticoresearch/commit/f3c7848a59b7eedc67c3403d6d4b37ce6fa94dc8) 修复了空 mva 输出 NULL 而非空字符串[#160](https://github.com/manticoresoftware/manticoresearch/issues/161)
* [提交 0afa](https://github.com/manticoresoftware/manticoresearch/commit/0afa2ed058e5759470b1d7354c722faab34f98bb) 修复了在无 pthread_getname_np 环境下构建失败
* [提交 9405](https://github.com/manticoresoftware/manticoresearch/commit/9405fccdeb27f0302a8d9a848981d30080216777) 修复了线程池工作线程关闭服务器时崩溃
## 版本 2.8.0 GA, 2019年1月28日
### 改进
* 为 percolate 索引提供分布式索引支持
* CALL PQ 新增选项和变更：
    *   skip_bad_json
    *   模式 (sparsed/sharded)
    *   可传入 JSON 数组形式的 json 文档
    *   shift
    *   列名 'UID', 'Documents', 'Query', 'Tags', 'Filters' 重命名为 'id', 'documents', 'query', 'tags', 'filters'
* 支持 DESCRIBE pq TABLE
* 不再支持 SELECT FROM pq WHERE UID，改用 'id'
* pq 索引的 SELECT 与常规索引持平（例如可以通过 REGEX() 过滤规则）

* PQ 标签支持 ANY/ALL 操作
* 表达式对 JSON 字段自动转换，无需显式转换
* 内置 'non_cjk' charset_table 和 'cjk' ngram_chars
* 内置支持 50 种语言的停用词集合
* 停用词声明中多个文件也可用逗号分隔
* CALL PQ 支持接受 JSON 数组的文档
### Bug 修复
* [Commit a4e1](https://github.com/manticoresoftware/manticoresearch/commit/a4e19afee54dafdc04b0da53372dafd8c0d332d6) 修复了与 csjon 相关的内存泄漏
* [Commit 28d8](https://github.com/manticoresoftware/manticoresearch/commit/28d862774874891a03e361da1b0347ebe8066ce0) 修复了因 JSON 中缺失值导致的崩溃
* [Commit bf4e](https://github.com/manticoresoftware/manticoresearch/commit/bf4e9ea27c349cdc9ae4e54e960a4a17b90e38fa) 修复了 RT 索引空元数据的保存问题
* [Commit 33b4](https://github.com/manticoresoftware/manticoresearch/commit/33b4573529e7c7c7bce19bf9d54054866f30d99a) 修复了词形还原序列中的丢失形态标记（精确）
* [Commit 6b95](https://github.com/manticoresoftware/manticoresearch/commit/6b95d48240d3b5520afade19c249d79e020a5e88) 修复字符串属性 > 4M 时使用饱和处理代替溢出的问题
* [Commit 6214](https://github.com/manticoresoftware/manticoresearch/commit/621418b829e70af36aaa322a2f51ece3f86bc499) 修复了在禁用索引时 SIGHUP 导致的服务器崩溃
* [Commit 3f7e](https://github.com/manticoresoftware/manticoresearch/commit/3f7e35d1482966cc45d5abbcb2de5de3508d66b0) 修复了服务器在同时执行 API 会话状态命令时的崩溃
* [Commit cd9e](https://github.com/manticoresoftware/manticoresearch/commit/cd9e4f1709a48ddafc128c450c2d882bc11ba50e) 修复了带有字段过滤器的 RT 索引删除查询时服务器崩溃
* [Commit 9376](https://github.com/manticoresoftware/manticoresearch/commit/9376470d455fcd256c6abff79c431a6919dfa7ac) 修复了分布式索引的 CALL PQ 传入空文档导致服务器崩溃
* [Commit 8868](https://github.com/manticoresoftware/manticoresearch/commit/8868b207644f4f8cc1ab0c270adb35493fed22e1) 修复了 Manticore SQL 错误信息超过 512 字符被截断的问题
* [Commit de9d](https://github.com/manticoresoftware/manticoresearch/commit/de9deda9c142823d6dbf529423417b5c670fae94) 修复了无 binlog 的 percolate 索引保存时崩溃
* [Commit 2b21](https://github.com/manticoresoftware/manticoresearch/commit/2b219e1a32791e7740bb210b7d408f96abc6374f) 修复了 OSX 上 http 接口无法工作的问题

* [Commit e92c](https://github.com/manticoresoftware/manticoresearch/commit/e92c60240f8f9e1756bfa14fba0bbb402586a389) 修复了 indextool 在检查 MVA 时的错误提示
* [Commit 238b](https://github.com/manticoresoftware/manticoresearch/commit/238bdea59bad89f097403f1c978658ce45f16c70) 修复了 FLUSH RTINDEX 时的写锁问题，避免在保存和定期 flush (rt_flush_period) 时锁住整个索引
* [Commit c26a](https://github.com/manticoresoftware/manticoresearch/commit/c26a236bd42b7082db079a51a84172066d337d64) 修复了 ALTER percolate 索引卡在等待搜索加载的问题
* [Commit 9ee5](https://github.com/manticoresoftware/manticoresearch/commit/9ee5703f6c28f57cd5bff5e705966d93fe30d267) 修复了 max_children 为 0 时使用线程池默认工作线程数的问题
* [Commit 5138](https://github.com/manticoresoftware/manticoresearch/commit/5138fc048a1bd146b271ce6a72fe954344281f69) 修复了使用 index_token_filter 插件且 stopwords 和 stopword_step=0 时索引数据的错误

* [Commit 2add](https://github.com/manticoresoftware/manticoresearch/commit/2add3d319ac62fe450bf60e89033100853dc2ecf) 修复了在索引定义中仍使用 aot 词形还原器但缺失 lemmatizer_base 时的崩溃
## 版本 2.7.5 GA，2018年12月4日
### 改进
* REGEX 函数
* JSON API 搜索中的 limit/offset
* qcache 的性能分析点
### Bug修复

* [Commit eb3c](https://github.com/manticoresoftware/manticoresearch/commit/eb3c7683b9740cff4cc4cadd2ab9f990b9f7d173) 修复了多属性宽类型 FACET 时服务器崩溃
* [Commit d915](https://github.com/manticoresoftware/manticoresearch/commit/d915cf6696514f4be365ffb5981bccaa15983e65) 修复了 FACET 查询主选择列表中隐式分组的问题
* [Commit 5c25](https://github.com/manticoresoftware/manticoresearch/commit/5c25dc271a8c11a1c82ca3c88e518acbc6a8c164) 修复了带 GROUP N BY 的查询崩溃问题
* [Commit 85d3](https://github.com/manticoresoftware/manticoresearch/commit/85d30a232d4ffd89916b673a9b2db946d272a945) 修复了内存操作崩溃时的死锁
* [Commit 8516](https://github.com/manticoresoftware/manticoresearch/commit/85166b5ef2be38061d62f2b20f15acaa0311258a) 修复了 indextool 检查时的内存消耗问题

* [Commit 58fb](https://github.com/manticoresoftware/manticoresearch/commit/58fb03166830d8b23e7ab9ce56309db7ddac22f4) 修复了不再需要 gmock 头文件的问题，上游已自行解决
## 版本 2.7.4 GA，2018年11月1日
### 改进
* 对于远程分布式索引，SHOW THREADS 会打印原始查询语句而非 API 调用
* SHOW THREADS 新选项 `format=sphinxql` 以 SQL 格式打印所有查询
* SHOW PROFILE 添加了额外的 `clone_attrs` 阶段打印
### Bug修复
* [Commit 4f15](https://github.com/manticoresoftware/manticoresearch/commit/4f155712a0bccc1bd01cc191647bc8cff814888e) 修复了在无 malloc_stats, malloc_trim 的 libc 下构建失败的问题

* [Commit f974](https://github.com/manticoresoftware/manticoresearch/commit/f974f20bda3214a56877c393a192be1a77150958) 修复了 CALL KEYWORDS 结果集中的单词内特殊符号问题
* [Commit 0920](https://github.com/manticoresoftware/manticoresearch/commit/092083282ea8ae0b2e72fb6989c57ccec81e74ac) 修复了通过 API 调用分布式索引或远程代理的 CALL KEYWORDS 失效问题
* [Commit fd68](https://github.com/manticoresoftware/manticoresearch/commit/fd686bfe88b720ffd7642e36897ba45161cbd7d2) 修复了分布式索引的 agent_query_timeout 未正确传递给代理的 max_query_time 问题
* [Commit 4ffa](https://github.com/manticoresoftware/manticoresearch/commit/4ffa623e9d357a3b0b441615089e211f92f8de32) 修复了磁盘块总文档计数被 OPTIMIZE 命令影响导致权重计算错误的问题
* [Commit dcaf](https://github.com/manticoresoftware/manticoresearch/commit/dcaf4e0e3c8e9b0fbec47bbf3307feddec0936a6) 修复了 RT 索引中多重尾部命中导致的问题
* [Commit eee3](https://github.com/manticoresoftware/manticoresearch/commit/eee381754e902a43a8f499e1c950198a2a3e6ee0) 修复了轮替时的死锁

## 版本 2.7.3 GA，2018年9月26日
### 改进
* CALL KEYWORDS 的 sort_mode 选项
* VIP 连接的 DEBUG 可以执行 'crash <password>' 来在服务器上故意进行 SIGEGV 操作
* DEBUG 可以执行 'malloc_stats' 以在 searchd.log 中转储 malloc 统计信息，执行 'malloc_trim' 来执行 malloc_trim()
* 如果系统中存在 gdb，改进了回溯信息
### Bug 修复
* [Commit 0f3c](https://github.com/manticoresoftware/manticoresearch/commit/0f3cc333f6129451ad1ae2c97905b6c04ba5d454) 修复了 Windows 上 rename 导致的崩溃或失败
* [Commit 1455](https://github.com/manticoresoftware/manticoresearch/commit/1455ba2bf8079f03ce7f861a8d9662a360d705de) 修复了 32 位系统上服务器崩溃问题
* [Commit ad37](https://github.com/manticoresoftware/manticoresearch/commit/ad3710d53b5cd6a28b8c60352d0f9e285b03d9fa) 修复了空 SNIPPET 表达式导致的服务器崩溃或挂起
* [Commit b36d](https://github.com/manticoresoftware/manticoresearch/commit/b36d79214364500252665860f000ae85343528b6) 修复了非渐进式优化的损坏，并修正了渐进式优化不会为最旧硬盘块创建 kill-list 的问题
* [Commit 34b0](https://github.com/manticoresoftware/manticoresearch/commit/34b032499afd42ce47a4c7247814b4031094388a) 修复了在线程池工作模式下 SQL 和 API 中 queue_max_length 返回错误问题
* [Commit ae4b](https://github.com/manticoresoftware/manticoresearch/commit/ae4b3202cbdb8014cabe2b90e269d5cf74f49871) 修复了向带有 regexp 或 rlp 选项设置的 PQ 索引添加全扫描查询时的崩溃问题
* [Commit f80f](https://github.com/manticoresoftware/manticoresearch/commit/f80f8d5d7560187078868aed9a9575f4549e98aa) 修复了连续调用 PQ 导致的崩溃
* [Commit 9742](https://github.com/manticoresoftware/manticoresearch/commit/9742f5f0866af73f8cd8483ecd18a507ea80dd65) 重构 AcquireAccum
* [Commit 39e5](https://github.com/manticoresoftware/manticoresearch/commit/39e5bc3751b7295222eb76407c5d72ce1dad545b) 修复了调用 pq 后的内存泄漏
* [Commit 21bc](https://github.com/manticoresoftware/manticoresearch/commit/21bcc6d17395f0f57dde79f6716ef303b7ea527d) 外观重构（c++11 风格的构造函数，默认值，nullptr）
* [Commit 2d69](https://github.com/manticoresoftware/manticoresearch/commit/2d690398f14c736956cfdd66feb4d3091d6b3a4d) 修复了尝试向 PQ 索引插入重复项时的内存泄漏
* [Commit 5ed9](https://github.com/manticoresoftware/manticoresearch/commit/5ed92c4b66da6423fa28f11b1fd59103ed1e1fb9) 修复了 JSON 字段 IN 与大量值一起使用时的崩溃问题
* [Commit 4a52](https://github.com/manticoresoftware/manticoresearch/commit/4a5262e2776aa261e34ba5c914ea60804f59de3f) 修复了对带有扩展限制设置的 RT 索引执行 CALL KEYWORDS 语句导致的服务器崩溃
* [Commit 5526](https://github.com/manticoresoftware/manticoresearch/commit/552646bb6fefde4e2525298e43e628839b421d3d) 修复了 PQ matches 查询中的无效过滤器;
* [Commit 204f](https://github.com/manticoresoftware/manticoresearch/commit/204f521408b730198a5bab2c20407a3d7df94873) 为 ptr 属性引入了小对象分配器
* [Commit 2545](https://github.com/manticoresoftware/manticoresearch/commit/25453e5387152c0575d1eda1b2beb89a49c7493a) 重构 ISphFieldFilter 为引用计数风格
* [Commit 1366](https://github.com/manticoresoftware/manticoresearch/commit/1366ee06e828397074b69bec9265d7147170c60e) 修复了对非终止字符串使用 strtod 时的未定义行为 / SIGSEGV
* [Commit 94bc](https://github.com/manticoresoftware/manticoresearch/commit/94bc6fcbdae4d08bcf6da2966e3ec2966091289f) 修复了 JSON 结果集处理中的内存泄漏
* [Commit e78e](https://github.com/manticoresoftware/manticoresearch/commit/e78e9c948963416caff9e4e46296a58080107835) 修复了应用属性添加时越界读取内存块问题
* [Commit fad5](https://github.com/manticoresoftware/manticoresearch/commit/fad572fb543606b33e688eb82a485b86011545fd) 重构 CSphDict 为引用计数风格
* [Commit fd84](https://github.com/manticoresoftware/manticoresearch/commit/fd841a472b472280c93f4af3506851d19f6adc45) 修复了 AOT 内部类型外部泄漏问题
* [Commit 5ee7](https://github.com/manticoresoftware/manticoresearch/commit/5ee7f208c7685f2ebb9e5623a8802b721e2a7f0a) 修复了分词器管理的内存泄漏
* [Commit 116c](https://github.com/manticoresoftware/manticoresearch/commit/116c5f1abebee9a0d99afe93546a1d8e4c6c6958) 修复了 grouper 中的内存泄漏
* [Commit 56fd](https://github.com/manticoresoftware/manticoresearch/commit/56fdbc9ab38973a9a0ba8ccee45f71cf33c16423) 为 matches 中的动态指针提供特殊的释放/复制（grouper 内存泄漏）
* [Commit b1fc](https://github.com/manticoresoftware/manticoresearch/commit/31db660f378541aa0d1e7cf2e7209bb0a1f47fd8) 修复了 RT 动态字符串的内存泄漏
* [Commit 517b](https://github.com/manticoresoftware/manticoresearch/commit/517b9e8749f092e923cd2884b674b5bb84e20755) 重构 grouper
* [Commit b1fc](https://github.com/manticoresoftware/manticoresearch/commit/b1fc16140e5dc44290686330b476a254e0092107) 轻微重构（c++11 构造函数，一些格式调整）
* [Commit 7034](https://github.com/manticoresoftware/manticoresearch/commit/7034e073f4cf2844762e0a464b7c3de05d3d122b) 重构 ISphMatchComparator 为引用计数风格
* [Commit b1fc](https://github.com/manticoresoftware/manticoresearch/commit/b1fc16140e5dc44290686330b476a254e0092107) 私有化克隆器
* [Commit efbc](https://github.com/manticoresoftware/manticoresearch/commit/efbc0511d6809c4a57453c7283f9bf53f3fb8d97) 简化 MVA_UPSIZE、DOCINFO2ID_T、DOCINFOSETID 的本机小端处理
* [Commit 6da0](https://github.com/manticoresoftware/manticoresearch/commit/6da0df4ac9946fc59f98d229b90e12c9221dd4c0) 为 ubertests 添加 valgrind 支持
* [Commit 1d17](https://github.com/manticoresoftware/manticoresearch/commit/1d17669cb7ec54ac80b392e101f37a688eb98080) 修复了连接上的 'success' 标志竞态导致的崩溃
* [Commit 5a09](https://github.com/manticoresoftware/manticoresearch/commit/5a09c32d7ded56b2c0bd3e2ad7968cb8d6362919) 将 epoll 切换为边沿触发模式
* [Commit 5d52](https://github.com/manticoresoftware/manticoresearch/commit/5d528682737fca03a4352a3093b1319ec27d598c) 修复了表达式中 IN 语句的格式化问题，如过滤器中使用
* [提交 bd8b](https://github.com/manticoresoftware/manticoresearch/commit/bd8b3c976ff8b4667f55dfdd101b20a920137ac5) 修复了在提交带有大 docid 的文档到 RT 索引时的崩溃问题
* [提交 ce65](https://github.com/manticoresoftware/manticoresearch/commit/ce656b83b747ce7c55795b53915770c13fb99cfe) 修复了 indextool 中无参数选项的问题
* [提交 08c9](https://github.com/manticoresoftware/manticoresearch/commit/08c9507177820f441f534bf06a11dac5e54bebb4) 修复了扩展关键字的内存泄漏

* [提交 30c7](https://github.com/manticoresoftware/manticoresearch/commit/30c75a2f525ec9bda625d903acdc9d4d2e2413dc) 修复了 json 分组器的内存泄漏
* [提交 6023](https://github.com/manticoresoftware/manticoresearch/commit/6023f269b6f2080e4d380b0957605ef8107c8c9f) 修复了全局用户变量的泄漏
* [提交 7c13](https://github.com/manticoresoftware/manticoresearch/commit/7c138f15ca23c0c8717fa12041e3db7f988fcf15) 修复了在早期拒绝匹配时动态字符串的泄漏
* [提交 9154](https://github.com/manticoresoftware/manticoresearch/commit/9154b18eaed5302e8330cb3a95bd968959ccb312) 修复了 length(<expression>) 中的泄漏
* [提交 43fc](https://github.com/manticoresoftware/manticoresearch/commit/43fca3a4e26139765d0fac8de054200dc4875fc6) 修复了解析器中由于 strdup() 导致的内存泄漏
* [提交 71ff](https://github.com/manticoresoftware/manticoresearch/commit/71ff77737e63d1019b7c9804dca7fa2351025dba) 重构表达式解析器以准确追踪引用计数
## 版本 2.7.2 GA，2018年8月27日
### 改进
* 兼容 MySQL 8 客户端
* 支持带 WITH RECONFIGURE 的 [TRUNCATE](Emptying_a_table.md)
* 移除 RT 索引 SHOW STATUS 中的内存计数器
* 多代理的全局缓存

* 改进 Windows 上的 IOCP
* HTTP 协议的 VIP 连接
* Manticore SQL 的 [DEBUG](Reporting_bugs.md#DEBUG) 命令，可运行多个子命令
* 新增 [shutdown_token](Server_settings/Searchd.md#shutdown_token) - 调用 DEBUG 命令中的 `shutdown` 需要的密码 SHA1 哈希
* 新增 SHOW AGENT STATUS 的统计信息 (_ping, _has_perspool, _need_resolve)
* indexer 的 --verbose 选项现在支持 \[debugvv\] 用于打印调试信息
### Bug 修复
* [提交 3900](https://github.com/manticoresoftware/manticoresearch/commit/390082a7be0a1f9539b30361d11d54de35c62a44) 移除 optimize 中的写锁
* [提交 4c33](https://github.com/manticoresoftware/manticoresearch/commit/4c337648329be22373be33333cbc9c5664d18b80) 修复 reload index 设置时的写锁问题
* [提交 b5ea](https://github.com/manticoresoftware/manticoresearch/commit/b5ea8dc0b18a23dbd739e92c66dd93b0094df14b) 修复带 JSON 过滤器查询时的内存泄漏
* [提交 930e](https://github.com/manticoresoftware/manticoresearch/commit/930e831a43fd3d1ce996185be968d1df49a088d0) 修复 PQ 结果集中空文档的问题
* [提交 53de](https://github.com/manticoresoftware/manticoresearch/commit/53deecfecb374431f84f8592a1095a77407b8aea) 修复由于任务被移除导致的混淆问题
* [提交 cad9](https://github.com/manticoresoftware/manticoresearch/commit/cad9b9c617719aad97e9f5a2ae93491153d8e504) 修复远程主机计数错误
* [提交 9000](https://github.com/manticoresoftware/manticoresearch/commit/90008cc15c4684d445d37a391bb4a667bbc2e365) 修复解析代理描述符时的内存泄漏
* [提交 978d](https://github.com/manticoresoftware/manticoresearch/commit/978d839641ad6040c22c9fc3b703b1a02685f266) 修复搜索中的泄漏
* [提交 0193](https://github.com/manticoresoftware/manticoresearch/commit/0193946105ac08913362b7d7acf0c90f9b38dc85) 对显式/内联 c-trs、override/final 使用进行了美化修改
* [提交 943e](https://github.com/manticoresoftware/manticoresearch/commit/943e2997b33f9b2eda23d94a66068910f9476ebd) 修复本地/远程 schema 中 json 的泄漏
* [提交 02db](https://github.com/manticoresoftware/manticoresearch/commit/02dbdd6f1d0b72e7e458ebebdd2c67f989577e6b) 修复本地/远程 schema 中 json 排序列表达式的泄漏
* [提交 c74d](https://github.com/manticoresoftware/manticoresearch/commit/c74d0b4035e4214510376845b22cb676a8da24a3) 修复常量别名的泄漏
* [提交 6e5b](https://github.com/manticoresoftware/manticoresearch/commit/6e5b57e1d3952c9695376728a143bf5434208f53) 修复预读线程的泄漏
* [提交 39c7](https://github.com/manticoresoftware/manticoresearch/commit/39c740fef5bf71da0bee60a35f5dd9f471af850f) 修复退出时因 netloop 中等待卡住的问题
* [提交 adaf](https://github.com/manticoresoftware/manticoresearch/commit/adaf9772f95772ad6d5297acace6c5cb92b56ad5) 修复更改 HA 代理为普通主机时“ping”行为的卡顿
* [提交 32c4](https://github.com/manticoresoftware/manticoresearch/commit/32c40eac9cdd40b15d79e58b2f15416f164f9701) 为仪表盘存储分离垃圾回收
* [提交 511a](https://github.com/manticoresoftware/manticoresearch/commit/511a3cf2c1f1858f6e94fe9f8175b7216db3cbd6) 修复引用计数指针
* [提交 32c4](https://github.com/manticoresoftware/manticoresearch/commit/af2326cd2927effbad7bec88e8dc238a566cf27e) 修复 indextool 在索引不存在时崩溃
* [提交 156e](https://github.com/manticoresoftware/manticoresearch/commit/156edc717dc63d3ea120f42466eba6e022da88be) 修复 xmlpipe 索引中过长 attr/field 的输出名称
* [提交 cdac](https://github.com/manticoresoftware/manticoresearch/commit/cdac6d1f17d384bfc8bae49e91241b2f346c3830) 修复配置中无 indexer 部分时默认索引器的值
* [提交 e61e](https://github.com/manticoresoftware/manticoresearch/commit/e61ec00b6b27d1d5878247e2ee817f3b1e7fde16) 修复服务器重启后 RT 索引在磁盘块内嵌入的错误停用词

* [提交 5fba](https://github.com/manticoresoftware/manticoresearch/commit/5fba49d5e28c17de4c0acbd984466127f42de6e8) 修复跳过幽灵连接（已关闭但未从轮询器中最终删除）
* [提交 f22a](https://github.com/manticoresoftware/manticoresearch/commit/f22ae34b623906b3c5a05a06198dad2e548dc541) 修复混合（孤立的）网络任务
* [提交 4689](https://github.com/manticoresoftware/manticoresearch/commit/46890e70eb8dcd00c1bf1b030538b8f131c601c2) 修复写操作后读操作崩溃
* [Commit 03f9](https://github.com/manticoresoftware/manticoresearch/commit/03f9df904f611c438d5ebcaaeef911b0dc8d6c39) 修复了在 Windows 上运行测试时 searchd 崩溃的问题
* [Commit e925](https://github.com/manticoresoftware/manticoresearch/commit/e9255ed2704790a3d7f5273d38b85433463cb3ff) 修复了在常规 connect() 处理 EINPROGRESS 代码的问题
* [Commit 248b](https://github.com/manticoresoftware/manticoresearch/commit/248b72f1544b27e9e82773284050d44eeb1714e1) 修复了使用 TFO 时的连接超时问题

## 版本 2.7.1 GA，2018 年 7 月 4 日
### 改进
* 提升了 PQ 上匹配多个文档时的通配符性能
* 支持 PQ 上的全扫描查询
* 支持 PQ 上的 MVA 属性


* 支持 percolate 索引的正则表达式和 RLP
### 错误修复
* [Commit 6885](https://github.com/manticoresoftware/manticoresearch/commit/68856261b41f6950666f9e5122726839fb3c71d0) 修复了查询字符串丢失的问题
* [Commit 0f17](https://github.com/manticoresoftware/manticoresearch/commit/0f1770943cba53bfd2e0edfdf0b1495ff0dd0e89) 修复了 SHOW THREADS 语句中空白信息的问题
* [Commit 53fa](https://github.com/manticoresoftware/manticoresearch/commit/53faa36a916bb87868e83aa79c1a1f972fb20ca0) 修复了使用 NOTNEAR 操作符匹配时的崩溃
* [Commit 2602](https://github.com/manticoresoftware/manticoresearch/commit/26029a7cd77b518fb3a27588d7db6c8bffc73b0f) 修复了对 PQ 删除操作中错误的过滤器所显示的错误信息
## 版本 2.7.0 GA，2018 年 6 月 11 日
### 改进
* 减少系统调用次数以避免 Meltdown 和 Spectre 补丁的影响
* 本地索引管理的内部重写
* 远程摘要重构
* 完整配置重新加载
* 所有节点连接现在都是独立的
* 协议增强
* Windows 通信由 wsapoll 切换为 IO 完成端口

* 主节点与其他节点通信时可使用 TFO
* [SHOW STATUS](Node_info_and_management/Node_status.md#SHOW-STATUS) 现在输出服务器版本和 mysql_version_string
* 增加了 CALL PQ 中用于文档调用的 `docs_id` 选项
* percolate 查询过滤器现在可包含表达式
* 分布式索引可以配合 FEDERATED 使用
* 虚拟的 SHOW NAMES COLLATE 和 `SET wait_timeout`（以更好兼容 ProxySQL）
### 错误修复
* [Commit 5bcf](https://github.com/manticoresoftware/manticoresearch/commit/5bcff05d19e495f1a44f190885b25cbb6f217c4a) 修复了 PQ 中添加不等于标签的问题
* [Commit 9ebc](https://github.com/manticoresoftware/manticoresearch/commit/9ebc58916cd515eaa88da66d0895aebf0d1f2b5f) 修复了在 JSON 文档调用 PQ 语句中缺少文档 ID 字段的问题
* [Commit 8ae0](https://github.com/manticoresoftware/manticoresearch/commit/8ae0e593a286110d8505d88baa3ac9e433cb38c9) 修复了 PQ 索引的刷新语句处理器问题
* [Commit c24b](https://github.com/manticoresoftware/manticoresearch/commit/c24b152344ecd77661566cddb803487f51d3c1aa) 修复了 PQ 过滤 JSON 和字符串属性的问题
* [Commit 1b8b](https://github.com/manticoresoftware/manticoresearch/commit/1b8bdde19aaa362785ea4c33bdc019154f8fbac4) 修复了空 JSON 字符串解析问题


* [Commit 1ad8](https://github.com/manticoresoftware/manticoresearch/commit/1ad8a07dbfc1131913a8d92c261fbb48f934e8b7) 修复了在使用 OR 过滤器的多查询中崩溃的问题
* [Commit 69b8](https://github.com/manticoresoftware/manticoresearch/commit/69b89806380a229e36287ad9daf6f0ea2b5453eb) 修复了 indextool 使用配置公共部分（lemmatizer_base 选项）执行命令（dumpheader）的问题
* [Commit 6dbe](https://github.com/manticoresoftware/manticoresearch/commit/6dbeaf2389d2af6a9dfccb56bfc986685b9f203e) 修复了结果集和过滤器中空字符串的问题
* [Commit 39c4](https://github.com/manticoresoftware/manticoresearch/commit/39c4eb55ecc85f6dd54f6c8c6d6dfcf87fd8748e) 修复了文档 ID 值为负数的问题
* [Commit 266b](https://github.com/manticoresoftware/manticoresearch/commit/266b7054f98e7d30fc2829b5c2467dc9f90def92) 修复了对非常长词进行索引时的词修剪长度问题
* [Commit 4782](https://github.com/manticoresoftware/manticoresearch/commit/47823b0bd06cbd016a24345c99bb8693790b43dd) 修复了 PQ 上通配符查询匹配多个文档的问题
## 版本 2.6.4 GA，2018 年 5 月 3 日

### 功能和改进
* MySQL FEDERATED 引擎[支持](Extensions/FEDERATED.md)
* MySQL 包现在返回 SERVER_STATUS_AUTOCOMMIT 标志，提高了与 ProxySQL 的兼容性
* [listen_tfo](Server_settings/Searchd.md#listen_tfo) - 为所有监听器启用 TCP 快速打开连接
* indexer --dumpheader 现在也可以从 .meta 文件中导出 RT 头信息
* Ubuntu Bionic 的 cmake 构建脚本
### 修复错误
* [Commit 355b](https://github.com/manticoresoftware/manticoresearch/commit/355b11629174813abd9cd5bf2233be0783f77745) 修复了 RT 索引的无效查询缓存条目

* [Commit 546e](https://github.com/manticoresoftware/manticoresearch/commit/546e2297a01cc2913bc0d33052d5bcefae8737eb) 修复了无缝旋转后索引设置丢失的问题
* [Commit 0c45](https://github.com/manticoresoftware/manticoresearch/commit/0c4509898393993bc87194d2aca2070395ff7f83) 修复了固定中缀与前缀长度设置；添加了不支持的中缀长度警告
* [Commit 8054](https://github.com/manticoresoftware/manticoresearch/commit/80542fa54d727c781635918e8d4d65a20c2fbd1e) 修复了 RT 索引自动刷新顺序问题

* [Commit 705d](https://github.com/manticoresoftware/manticoresearch/commit/705d8c5f407726ed1c6f9745ecffa375682c8969) 修复了多属性索引和对多个索引查询时结果集架构的问题
* [Commit b0ba](https://github.com/manticoresoftware/manticoresearch/commit/b0ba932fff4a082f642cb737381c8a70215d4cc9) 修复了批量插入时文档重复导致部分命中丢失的问题
* [Commit 4510](https://github.com/manticoresoftware/manticoresearch/commit/4510fa44ad54b5bbac53c591144501565130fcae) 修复了优化失败无法合并大型文档数量 RT 索引磁盘块的问题
## 版本 2.6.3 GA，2018 年 3 月 28 日
### 改进
* 编译时使用 jemalloc。如果系统中存在 jemalloc，可以通过 cmake 标志 `-DUSE_JEMALLOC=1` 启用


### 错误修复
* [Commit 85a6](https://github.com/manticoresoftware/manticoresearch/commit/85a6d7e3db75f28c491042409bb677ed39eba07b) 修复了将 expand_keywords 选项记录到 Manticore SQL 查询日志中的问题
* [Commit caaa](https://github.com/manticoresoftware/manticoresearch/commit/caaa3842f4dbd8507fd9e20bf3371a0c8ec0c087) 修复了 HTTP 接口正确处理大尺寸查询的问题
* [Commit e386](https://github.com/manticoresoftware/manticoresearch/commit/e386d842b36195719e459b2db1fb20e402f9a51f) 修复了启用 index_field_lengths 时 DELETE RT 索引导致的服务器崩溃
* [Commit cd53](https://github.com/manticoresoftware/manticoresearch/commit/ce528f37eb5d8284661928f6f3b340a4a1985253) 修复了 cpustats searchd 命令行选项在不支持系统上工作的错误
* [Commit 8740](https://github.com/manticoresoftware/manticoresearch/commit/8740fd63ae842c2ab0e51a8fcd6180d787ed57ec) 修复了定义最小长度的 UTF8 子字符串匹配问题
## 版本 2.6.2 GA，2018 年 2 月 23 日
### 改进

* 改进了 [Percolate Queries](Searching/Percolate_query.md) 在使用 NOT 操作符及批量文档时的性能。
* [percolate_query_call](Searching/Percolate_query.md) 可根据 [dist_threads](Server_settings/Searchd.md#threads) 使用多线程
* 新增全文匹配操作符 NOTNEAR/N
* percolate 索引的 SELECT 支持 LIMIT
* [expand_keywords](Searching/Options.md#expand_keywords) 可接受 'start'、'exact'（其中'start,exact' 与 '1' 效果相同）
* ranged-main-query 支持 对 [joined fields](Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Processing_fetched_data.md#sql_joined_field) 使用 sql_query_range 定义的范围查询
### 修复
* [Commit 72dc](https://github.com/manticoresoftware/manticoresearch/commit/72dcf669744e9b7d636dfc213d24df85ab301f6b) 修复了搜索 RAM 分段时崩溃；修复了双缓冲保存磁盘块时死锁；优化时保存磁盘块死锁问题
* [Commit 3613](https://github.com/manticoresoftware/manticoresearch/commit/36137149a1c3c0893bdda5a28fc7e8244bf2d4ae) 修复了索引器在嵌入式 XML schema 中属性名为空时崩溃


* [Commit 48d7](https://github.com/manticoresoftware/manticoresearch/commit/48d7e8001d2a66466ca64577f27ddc5421a67251) 修复了错误的未拥有 pid 文件的取消链接操作
* [Commit a556](https://github.com/manticoresoftware/manticoresearch/commit/a5563a465ddc59ef71e65f17b68bc33f9700e838) 修复了临时文件夹中有时遗留孤立的管道文件
* [Commit 2376](https://github.com/manticoresoftware/manticoresearch/commit/2376e8fc4508944b96959bd10686c6d51f5145e8) 修复了空的 FACET 结果集错误的 NULL 行
* [Commit 4842](https://github.com/manticoresoftware/manticoresearch/commit/4842b67acdd5da75672db6c08ac563b48577ddd7) 修复了以 Windows 服务运行服务器时索引锁失效的问题
* [Commit be35](https://github.com/manticoresoftware/manticoresearch/commit/be35feef54ded59125750916cd211e56108eddae) 修复了 macOS 上错误的 iconv 库问题
* [Commit 8374](https://github.com/manticoresoftware/manticoresearch/commit/83744a977d7837d1d87cd506b88b0e7eb199efe6) 修复了错误的 count(\*) 计算
## 版本 2.6.1 GA，2018 年 1 月 26 日
### 改进

* [agent_retry_count](Server_settings/Searchd.md#agent_retry_count) 对于具有镜像的代理，重试次数现在为每个镜像的次数总和，即 agent_retry_count\*镜像数。
* [agent_retry_count](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_retry_count) 现在可针对每个索引单独指定，覆盖全局值。新增别名 [mirror_retry_count](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#mirror_retry_count)。
* 代理定义中可指定 retry_count 值，表示每个代理的重试次数
* Percolate Queries 现已支持 HTTP JSON API，接口为 [/json/pq](Searching/Percolate_query.md#Performing-a-percolate-query-with-CALL-PQ)。
* 为可执行文件添加了 -h 与 -v 选项（帮助和版本）
* [morphology_skip_fields](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) 支持实时索引
### 修复


* [Commit a40b](https://github.com/manticoresoftware/manticore/commit/a40b0793feff65e40d10062568d9847c08d10f57) 修复了 ranged-main-query 在 sql_range_step 用于 MVA 字段时的正确工作
* [Commit f2f5](https://github.com/manticoresoftware/manticore/commit/f2f53757db45bcfb1544263ce0817e856656a621) 修复了黑洞系统循环挂起和黑洞代理似乎断开的问题
* [Commit 84e1](https://github.com/manticoresoftware/manticore/commit/84e1f54aef25e0fce98870ad2dd784db5116f1d6) 修复了查询 ID 不一致问题，修正了存储查询的重复 ID
* [Commit 1948](https://github.com/manticoresoftware/manticore/commit/19484231814fcb82b21763a3a4a9f45adc6b2d40) 修复了服务器在各种状态关闭时崩溃的问题
* [Commit 9a70](https://github.com/manticoresoftware/manticore/commit/9a706b499a1d61a90076065a1a703029d49db958)  [Commit 3495](https://github.com/manticoresoftware/manticore/commit/3495fd70cba8846b1a50d55d6679b039414c3d2a) 修复了长查询超时问题
* [Commit 3359](https://github.com/manticoresoftware/manticore/commit/3359bcd89b4f79a645fe84b8cf8616ce0addff02) 重构了基于 kqueue 的系统（Mac OS X、BSD）中主代理网络轮询
## 版本 2.6.0，2017 年 12 月 29 日

### 新特性与改进
* [HTTP JSON](Connecting_to_the_server/HTTP.md)：JSON 查询现在支持属性上的等值比较，MVA 和 JSON 属性可用于插入和更新，分布式索引可通过 JSON API 进行更新和删除
* [Percolate Queries](Searching/Percolate_query.md)
* 代码中移除了对 32 位 docid 的支持，同时移除了所有转换/加载含 32 位 docid 的旧索引的代码。
* [仅针对特定字段使用形态学](https://github.com/manticoresoftware/manticore/issues/7)。新增索引指令 morphology_skip_fields，允许定义不应用形态学的字段列表。
* [expand_keywords 现在可以通过 OPTION 语句设置为查询运行时指令](https://github.com/manticoresoftware/manticore/issues/8)

### Bugfixes
* [Commit 0cfa](https://github.com/manticoresoftware/manticore/commit/0cfae4c) 修复了使用 rlp 构建时服务器调试版本（以及可能的发布版本中的未定义行为）崩溃的问题

* [Commit 3242](https://github.com/manticoresoftware/manticore/commit/324291e) 修正启用 progressive 选项时，RT 索引优化合并错误顺序的 kill-lists 问题
* [Commit ac0e](https://github.com/manticoresoftware/manticore/commit/ac0efee) 修复 mac 小崩溃
* 经过彻底的静态代码分析后修复了大量小问题
* 其它小型错误修复
### 升级
在此版本中，我们更改了主服务器与代理之间通信所用的内部协议。如果您在分布式环境中运行多个实例的 Manticoresearch，请务必先升级代理，再升级主服务器。
## Version 2.5.1, 2017年11月23日
### 功能和改进
* [HTTP API 协议](Connecting_to_the_server/HTTP.md) 支持 JSON 查询。支持搜索、插入、更新、删除、替换操作。数据操作命令也可以批量处理，目前存在一些限制，如 MVA 和 JSON 属性不能用于插入、替换或更新。
* [RELOAD INDEXES](Data_creation_and_modification/Adding_data_from_external_storages/Rotating_a_table.md#RELOAD-TABLES) 命令
* [FLUSH LOGS](Logging/Rotating_query_and_server_logs.md) 命令

* [SHOW THREADS](Node_info_and_management/SHOW_THREADS.md) 可显示优化、旋转或刷新进度。
* GROUP N BY 与 MVA 属性正常工作
* blackhole 代理运行在独立线程中，不会再影响主服务器查询
* 实现了索引的引用计数，以避免旋转和高负载时的阻塞
* 实现了 SHA1 哈希，目前尚未对外提供
* 修复了在 FreeBSD、macOS 和 Alpine 上的编译问题
### Bugfixes
* [Commit 9897](https://github.com/manticoresoftware/manticore/commit/989752b) 修复了使用块索引时的过滤器回归问题

* [Commit b1c3](https://github.com/manticoresoftware/manticore/commit/b1c3864) 将 PAGE_SIZE 重命名为 ARENA_PAGE_SIZE，以兼容 musl
* [Commit f213](https://github.com/manticoresoftware/manticore/commit/f2133cc) 禁用 cmake < 3.1.0 版本的 googletests
* [Commit f30e](https://github.com/manticoresoftware/manticore/commit/0839de7) 修复服务器重启时绑定套接字失败的问题
* [Commit 0807](https://github.com/manticoresoftware/manticore/commit/0807240) 修复服务器关闭时崩溃的问题
* [Commit 3e3a](https://github.com/manticoresoftware/manticore/commit/3e3acc3) 修复 system blackhole 线程的 SHOW THREADS 显示问题
* [Commit 262c](https://github.com/manticoresoftware/manticore/commit/262c3fe) 重构了 iconv 的配置检查，修复了在 FreeBSD 和 Darwin 上的构建问题
## Version 2.4.1 GA, 2017年10月16日
### 功能和改进
* WHERE 子句中属性过滤器间支持 OR 操作符
* 维护模式 ( SET MAINTENANCE=1)
* [CALL KEYWORDS](Searching/Autocomplete.md#CALL-KEYWORDS) 在分布式索引上可用
* [UTC 分组](Server_settings/Searchd.md#grouping_in_utc)
* [query_log_mode](Server_settings/Searchd.md#query_log_mode) 用于自定义日志文件权限
* 字段权重可以为零或负数

* [max_query_time](Searching/Options.md#max_query_time) 现在可以影响全表扫描
* 新增 [net_wait_tm](Server_settings/Searchd.md#net_wait_tm), [net_throttle_accept](Server_settings/Searchd.md#net_throttle_accept) 和 [net_throttle_action](Server_settings/Searchd.md#net_throttle_action) 用于网络线程的细粒度调节（当 workers=thread_pool 时）

* COUNT DISTINCT 支持 facet 搜索
* IN 可以用于 JSON 浮点数数组
* 整数/浮点表达式不再破坏多查询优化
* [SHOW META](Node_info_and_management/SHOW_META.md) 当使用多查询优化时显示 `multiplier` 行
### 编译

Manticore Search 使用 cmake 构建，最低 gcc 编译版本要求为 4.7.2。
### 文件夹和服务
* Manticore Search 以 `manticore` 用户身份运行。
* 默认数据文件夹现在为 `/var/lib/manticore/`。
* 默认日志文件夹现在为 `/var/log/manticore/`。
* 默认 pid 文件夹现在为 `/var/run/manticore/`。
### Bugfixes
* [Commit a58c](https://github.com/manticoresoftware/manticore/commit/a58c619) 修复了导致 Java 连接器崩溃的 SHOW COLLATION 语句
* [Commit 631c](https://github.com/manticoresoftware/manticore/commit/631cf4e) 修复分布式索引处理中的崩溃；为分布式索引哈希添加锁；从代理中移除移动和复制运算符
* [Commit 942b](https://github.com/manticoresoftware/manticore/commit/942bec0) 修复由于并行重连导致的分布式索引处理崩溃
* [Commit e5c1](https://github.com/manticoresoftware/manticore/commit/e5c1ed2) 修复崩溃处理程序在将查询存储到服务器日志时崩溃的问题
* [Commit 4a4b](https://github.com/manticoresoftware/manticore/commit/4a4bda5) 修复多查询中使用池化属性时的崩溃
* [Commit 3873](https://github.com/manticoresoftware/manticore/commit/3873bfb) 通过防止索引页被包含进 core 文件，减小了 core 文件大小
* [Commit 11e6](https://github.com/manticoresoftware/manticore/commit/11e6254) 修复在指定无效代理时 searchd 启动崩溃的问题
* [Commit 4ca6](https://github.com/manticoresoftware/manticore/commit/4ca6350) 修复索引器在 sql_query_killlist 查询中报告错误
* [Commit 123a](https://github.com/manticoresoftware/manticore/commit/123a9f0) 修复 fold_lemmas=1 与命中计数的冲突
* [Commit cb99](https://github.com/manticoresoftware/manticore/commit/cb99164) 修复 html_strip 不一致行为
* [Commit e406](https://github.com/manticoresoftware/manticore/commit/e406761) 修复优化 rt 索引时丢失新设置；修复带同步选项的优化锁泄漏；

* [Commit 86ae](https://github.com/manticoresoftware/manticore/commit/86aeb82) 修复处理错误多查询的问题
* [提交 2645](https://github.com/manticoresoftware/manticore/commit/2645230) 修复了结果集依赖于多查询顺序的问题

