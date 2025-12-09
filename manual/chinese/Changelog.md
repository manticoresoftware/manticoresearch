# 更新日志

## 版本 15.1.0
**发布**：2025年12月7日

### 推荐库
- 推荐的 [MCL](https://github.com/manticoresoftware/columnar) 版本：9.0.0
- 推荐的 [Buddy](Installation/Manticore_Buddy.md#Manticore-Buddy) 版本：3.40.2

如果您遵循[官方安装指南](https://manticoresearch.com/install/)，则无需担心此问题。

## 重大变更
* ⚠️ [v15.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/15.0.0) [ PR #4003](https://github.com/manticoresoftware/manticoresearch/pull/4003) 更新了 MCL 要求至 9.0.0，带来了未压缩的浮点向量存储、KNN 向量的块大小更改以及非缓冲读取。此更新更改了数据格式。旧版本的 MCL 无法读取，但新版本仍可无问题读取您现有的列式表。

### 新功能和改进
* 🆕 [v15.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/15.1.0) [ PR #3990](https://github.com/manticoresoftware/manticoresearch/pull/3990) 通过将总时间拆分为更清晰的部分，改进了磁盘块刷新日志记录。
* 🆕 [v14.7.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/14.7.0) [ Issue #3860](https://github.com/manticoresoftware/manticoresearch/issues/3860) 更新了 Buddy 要求至 3.40.1，改进了自动补全：将归一化的二元分隔符字符转换为空格，过滤掉重复或格式错误的建议组合以提升建议质量。还修复了 Kafka 视图中的无效 JSON 错误，以及自动补全中排序组合在分数映射缺少某些键时不再抛出错误。
* 🆕 [v14.6.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/14.6.0) [ Issue #615](https://github.com/manticoresoftware/manticoresearch-buddy/issues/615) 更新了 Manticore Buddy 要求至版本 3.39.1，修复了 KNN 插件中的无效 JSON 错误，并允许 Buddy 处理程序覆盖 HTTP `Content-Type`，使 `/metrics` 现在返回 Prometheus 文本格式（`text/plain; version=0.0.4`）而非 JSON，修复了抓取失败问题。
* 🆕 [v14.4.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/14.4.0) [ PR #3942](https://github.com/manticoresoftware/manticoresearch/pull/3942) 更新了 Manticore Buddy 要求至 3.38.0，过滤掉零文档建议，改进了 Ds\Map 的字符串键处理，将 Buddy 中的内存使用报告格式从 KB 改为字节以提高精度，并提升了性能、稳定性和可维护性。
* 🆕 [v14.5.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/14.5.0) [ Issue #3329](https://github.com/manticoresoftware/manticoresearch/issues/3329) 在记录查询请求的 JSON 负载时，修剪多余的空白和换行符——跳过前后空格以避免记录格式错误的 JSON。
* 🆕 [v14.3.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/14.3.0) [ PR #3932](https://github.com/manticoresoftware/manticoresearch/pull/3932) 改进了对 `LOCK TABLES` / `UNLOCK TABLES` 的处理：写锁现在返回警告而非错误，读锁在 `SHOW LOCKS` 中正确显示，整体锁逻辑表现一致。
* 🆕 [v14.2.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/14.2.0) [ Issue #3891](https://github.com/manticoresoftware/manticoresearch/issues/3891) 增加了对 `JOIN ON` 子句中任意过滤表达式的支持（不仅限于等值比较），支持如 `... ON t1.id = t2.t1_id AND t2.value = 5` 的查询。

## Bug 修复
* 🪲 [v15.0.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/15.0.6) [ Issue #3601](https://github.com/manticoresoftware/manticoresearch/issues/3601) 修复了一个回归问题：使用自定义配置路径安装时，原生 Windows 服务无法启动。
* 🪲 [v15.0.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/15.0.5) [ Issue #3864](https://github.com/manticoresoftware/manticoresearch/issues/3864) 修复了基于 SQL 的源中“连接字段”的处理，使迭代连接命中时正确设置“结束标记”。
* 🪲 [v15.0.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/15.0.4) [ Issue #4004](https://github.com/manticoresoftware/manticoresearch/issues/4004) 修复了一个回归问题：守护进程的 HTTP `/sql` 响应错误地使用了 `Content-Type: text/html` 头，而非 `application/json`。
* 🪲 [v15.0.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/15.0.3) [ Issue #2727](https://github.com/manticoresoftware/manticoresearch/issues/2727) 修复了使用 JSON 到属性映射生成的属性进行 `GROUP BY` / `FACET` 分组时失败的问题。
* 🪲 [v15.0.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/15.0.2) [ Issue #3962](https://github.com/manticoresoftware/manticoresearch/issues/3962) 更新了 Buddy 要求至 3.40.2，新增了 `/sql` 端点的模糊搜索支持，并包含其他模糊搜索修复。
* 🪲 [v15.0.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/15.0.1) [ PR #3922](https://github.com/manticoresoftware/manticoresearch/pull/3922) 更新了支持 Logstash 9.2 的文档和测试。
* 🪲 [v14.7.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/14.7.6) [ PR #4002](https://github.com/manticoresoftware/manticoresearch/pull/4002) 修复了 KNN 过采样行为：当不请求重评分时不再计算重评分的 KNN 距离，并将浮点向量访问提示传递到列式存储层。
* 🪲 [v14.7.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/14.7.5) [ PR #3999](https://github.com/manticoresoftware/manticoresearch/pull/3999) 修复了“test_298”模型以解决失败的 KNN 相关测试。
* 🪲 [v14.7.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/14.7.4) [ Issue #3977](https://github.com/manticoresoftware/manticoresearch/issues/3977) Windows 上的测试有时会生成 .mdmp 崩溃转储 — 已修复，"ubertests" 完成后不再留下 minidumps。
* 🪲 [v14.7.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/14.7.3) [ Issue #3832](https://github.com/manticoresoftware/manticoresearch/issues/3832) 修复了通过 `/cli_json` 发送包含分号（例如组合 SQL 语句）的多查询请求失败的错误 — 处理前不再将分号替换为空字符。
* 🪲 [v14.7.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/14.7.2) [ Issue #1613](https://github.com/manticoresoftware/manticoresearch/issues/1613) 记录了用于计算短语/邻近/NEAR 操作符排名因素的内部 32 位掩码，以及它如何导致超过第 31 个关键字的词项计数不足。
* 🪲 [v14.7.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/14.7.1) [ PR #3992](https://github.com/manticoresoftware/manticoresearch/pull/3992) 修复了通过代理处理的分布式表的 HTTP UPDATE 和 DELETE 请求缺失错误消息的问题，客户端现在在操作失败时能正确接收错误。
* 🪲 [v14.6.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/14.6.4) [ Issue #3478](https://github.com/manticoresoftware/manticoresearch/issues/3478) 增强了更新验证，检查更新的属性是否与全文字段冲突；如果目标是全文字段，更新将被拒绝。
* 🪲 [v14.6.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/14.6.3) [ Issue #2352](https://github.com/manticoresoftware/manticoresearch/issues/2352) 修复了使用带有 `persistent_connections_limit` 的分布式表时的内部错误。
* 🪲 [v14.6.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/14.6.2) [ Issue #3757](https://github.com/manticoresoftware/manticoresearch/issues/3757) 修复了表状态计数器误标为 "_sec"（例如 `query_time_1min`）但实际上报告的是毫秒而非秒的错误。
* 🪲 [v14.6.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/14.6.1) [ Issue #3979](https://github.com/manticoresoftware/manticoresearch/issues/3979) 修复了 `SHOW INDEX <name> STATUS` 搜索时间统计（search_stats_ms_*）与查询日志计算值不匹配的问题；报告的时间现在反映真实的日志查询时间。
* 🪲 [v14.5.8](https://github.com/manticoresoftware/manticoresearch/releases/tag/14.5.8) [ Issue #3517](https://github.com/manticoresoftware/manticoresearch/issues/3517) 更新了 HTTP 头处理，使 Buddy 集成可以定义或覆盖头部，而不总是强制使用 `application/json`。
* 🪲 [v14.5.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/14.5.7) 修正了测试 226 以强制稳定的结果顺序，并修正了 GTest 模型中 JSON 混合数组的处理。
* 🪲 [v14.5.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/14.5.6) 通过修正 `binlog.cpp` 中的类型不匹配，修复了 Windows 构建，确保 `DoSaveMeta()` 现在能正确编译于 Windows 平台。
* 🪲 [v14.5.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/14.5.5) [Issue #805](https://github.com/manticoresoftware/manticoresearch/issues/805) [Issue #807](https://github.com/manticoresoftware/manticoresearch/issues/807) [Issue #3924](https://github.com/manticoresoftware/manticoresearch/issues/3924) 修复了 JSON 属性分面中的不一致行为：在 FACET 子句中为 JSON 数组设置别名现在与直接对数组分面行为一致。
* 🪲 [v14.5.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/14.5.4) [ Issue #3927](https://github.com/manticoresoftware/manticoresearch/issues/3927) 通过跳过空 HNSW 索引的搜索，修复了 KNN 向量搜索中的崩溃。
* 🪲 [v14.5.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/14.5.2) [ Issue #3669](https://github.com/manticoresoftware/manticoresearch/issues/3669) 修复了 `MATCH()` 解析，空组如 `()` 不再触发错误（例如 `camera()` 查询现在能正确工作）。
* 🪲 [v14.5.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/14.5.1) [ PR #3961](https://github.com/manticoresoftware/manticoresearch/pull/3961) 更新了 mysqldump 复制模式的备份文档：说明了转储复制表时使用 `--skip-lock-tables`，并修复了多个损坏的手册链接。
* 🪲 [v14.3.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/14.3.2) [ Issue #2772](https://github.com/manticoresoftware/manticoresearch/issues/2772) 修复了通过 MySQL 客户端 9 执行某些命令时，查询日志中出现 "unexpected $undefined near '$$'" 错误的 bug。
* 🪲 [v14.3.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/14.3.1) [ PR #3934](https://github.com/manticoresoftware/manticoresearch/pull/3934) 更新了 Manticore Buddy 要求从 3.37.0 到 3.37.2，移除了多余的 `LOCK/UNLOCK TABLES` 检查（现由守护进程处理），并修复了自动补全解析，使带转义引号（如 `\"` 或 `\'`）的查询能正确处理而非失败。
* 🪲 [v14.2.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/14.2.1) [ Issue #3602](https://github.com/manticoresoftware/manticoresearch/issues/3602) 修复了在 KNN+MATCH 查询中使用自定义排序表达式 `knn_dist()` 时的崩溃；此类查询现在返回明确错误而非崩溃。
* 🪲 [v14.1.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/14.1.1) 文档：修正了小的翻译问题。

## 版本 14.1.0

**发布**：2025年11月7日

❤️ 我们衷心感谢 [@ricardopintottrdata](https://github.com/ricardopintottrdata) 在 [PR #3792](https://github.com/manticoresoftware/manticoresearch/pull/3792) 和 [PR #3828](https://github.com/manticoresoftware/manticoresearch/pull/3828) 中的工作——解决了关于 `HAVING` 总计数和 `filter with empty name` 错误的问题——以及感谢 [@jdelStrother](https://github.com/jdelStrother) 通过 [PR #3819](https://github.com/manticoresoftware/manticoresearch/pull/3819) 的贡献，改进了在没有 Jieba 支持时对 `ParseCJKSegmentation` 的处理。

你们的努力使项目更加强大——非常感谢！

### 推荐的库
- 推荐的 [MCL](https://github.com/manticoresoftware/columnar) 版本：8.1.0
- 推荐的 [Buddy](Installation/Manticore_Buddy.md#Manticore-Buddy) 版本：3.37.0

如果你遵循 [官方安装指南](https://manticoresearch.com/install/)，则无需担心此问题。

### ⚠️ 重要
版本 [v14.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/14.0.0) 更新了复制协议。如果你正在运行复制集群，需要：
  - 首先，干净地停止所有节点
  - 然后，使用 Linux 下的工具 `manticore_new_cluster`，以 `--new-cluster` 参数启动最后停止的节点。
  - 详细信息请阅读关于 [重启集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster) 的内容。

### 新功能和改进
* 🆕 [v14.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/14.1.0) [ Issue #3047](https://github.com/manticoresoftware/manticoresearch/issues/3047) 添加了对 mysqldump 生成的 `LOCK TABLES` 语句的支持，提高了逻辑备份的安全性。
* ⚠️ [v14.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/14.0.0) [ PR #3896](https://github.com/manticoresoftware/manticoresearch/pull/3896) 为复制 SST 中的捐赠者和加入者节点添加了 [进度计量器](../Creating_a_cluster/Setting_up_replication/Replication_cluster_status.md#SST-Progress-Metrics)，显示在 SHOW STATUS 中。
* 🆕 [v13.16.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.16.0) [ PR #3894](https://github.com/manticoresoftware/manticoresearch/pull/3894) 将 buddy 从 3.36.1 更新到 3.37.0，新增了“quorum”选项。
* 🆕 [v13.15.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.15.0) [ PR #3842](https://github.com/manticoresoftware/manticoresearch/pull/3842) 为模糊和自动完成插件添加了 [force_bigrams](../Searching/Spell_correction.md#Using-force_bigrams-for-better-transposition-handling) 选项。

### Bug 修复
* 🪲 [ Issue #3994](https://github.com/manticoresoftware/manticoresearch/issues/3994) 添加了对 Grafana 版本 12.3 的测试。
* 🪲 [v14.0.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/14.0.1) [ Issue #3844](https://github.com/manticoresoftware/manticoresearch/issues/3844) 修复了使用 `max(ft field)` 导致的崩溃。
* 🪲 [v13.15.13](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.15.13) [ PR #3828](https://github.com/manticoresoftware/manticoresearch/pull/3828) 修复了使用空过滤器名称时的错误。
* 🪲 [v13.15.12](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.15.12) [ PR #3873](https://github.com/manticoresoftware/manticoresearch/pull/3873) 将 buddy 从 3.36.0 更新到 3.36.1，EmulateElastic 插件中增加了 RT 模式检查。
* 🪲 [v13.15.11](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.15.11) [ PR #3857](https://github.com/manticoresoftware/manticoresearch/pull/3857) 添加了对 Filebeat 版本 9.2 的测试。
* 🪲 [v13.15.10](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.15.10) [ PR #3880](https://github.com/manticoresoftware/manticoresearch/pull/3880) 修复后测试了自动文档翻译。
* 🪲 [v13.15.9](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.15.9) [ Issue #3783](https://github.com/manticoresoftware/manticoresearch/issues/3783) 修复了阻止本地 FreeBSD 编译的问题。
* 🪲 [v13.15.8](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.15.8) 修复了文档翻译。
* 🪲 [v13.15.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.15.7) [ PR #3868](https://github.com/manticoresoftware/manticoresearch/pull/3868) 将 executor 从 1.3.5 更新到 1.3.6，增加了对 iconv 扩展的支持。
* 🪲 [v13.15.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.15.6) 修复了与 [issue 3817](https://github.com/manticoresoftware/manticoresearch/issues/3817) 相关的 fuzzer 构建问题。
* 🪲 [v13.15.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.15.5) [ Issue #3644](https://github.com/manticoresoftware/manticoresearch/issues/3644) 修复了由特定全文查询导致的崩溃。
* 🪲 [v13.15.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.15.4) [ Issue #3686](https://github.com/manticoresoftware/manticoresearch/issues/3686) 修复了全文查询 `"(abc|def)"` 无法正常工作的情况。
* 🪲 [v13.15.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.15.3) [ Issue #3428](https://github.com/manticoresoftware/manticoresearch/issues/3428) 增加了使用 `HAVING` 查询时获取结果总数的功能。
* 🪲 [v13.15.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.15.2) [ Issue #3817](https://github.com/manticoresoftware/manticoresearch/issues/3817) 添加了选项 [searchd.expansion_phrase_warning](../Server_settings/Searchd.md#expansion_phrase_warning)。
* 🪲 [v13.15.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.15.1) [ PR #3848](https://github.com/manticoresoftware/manticoresearch/pull/3848) 修复了复制事务密钥生成和冲突事务的处理。
* 🪲 [v13.14.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.14.0) [ Issue #3806](https://github.com/manticoresoftware/manticoresearch/issues/3806) 修复了 `CALL SUGGEST` 无法匹配三元组的问题。
* 🪲 [v13.13.8](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.13.8) [ PR #3839](https://github.com/manticoresoftware/manticoresearch/pull/3839) 将 buddy 从 3.35.4 更新到 3.35.5，以修正 SQL 查询中匹配连接的正则表达式。
* 🪲 [v13.13.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.13.7) [ Issue #3815](https://github.com/manticoresoftware/manticoresearch/issues/3815) 将 buddy 从 3.35.3 更新到 3.35.4，修复了 REPLACE 中负 ID 的问题。
* 🪲 [v13.13.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.13.6) [ PR #3830](https://github.com/manticoresoftware/manticoresearch/pull/3830) 将 buddy 从 3.35.1 更新到 3.35.3。
* 🪲 [v13.13.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.13.5) [ PR #3823](https://github.com/manticoresoftware/manticoresearch/pull/3823) 添加了 Grafana 集成测试。
* 🪲 [v13.13.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.13.4) [ PR #3819](https://github.com/manticoresoftware/manticoresearch/pull/3819) 修复了在没有 Jieba 支持时 `ParseCJKSegmentation` 的问题。
* 🪲 [v13.13.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.13.3) [ PR #3808](https://github.com/manticoresoftware/manticoresearch/pull/3808) 修复了在右连接 JSON 查询中使用过滤器时的错误处理。
* 🪲 [v13.13.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.13.2) [ PR #3789](https://github.com/manticoresoftware/manticoresearch/pull/3789) 验证了 KNN 参数。
* 🪲 [v13.13.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.13.1) [ Issue #3800](https://github.com/manticoresoftware/manticoresearch/issues/3800) 修复了在无 cjk/jieba 支持下编译时的构建问题。

## 版本 13.13.0
**发布于**：2025年10月7日

### 推荐库
- 推荐的 [MCL](https://github.com/manticoresoftware/columnar) 版本：8.1.0
- 推荐的 [Buddy](Installation/Manticore_Buddy.md#Manticore-Buddy) 版本：3.35.1

如果您遵循[官方安装指南](https://manticoresearch.com/install/)，则无需担心此问题。

### ⚠️ 重要提示

由于配置文件已更新，**在 Linux 升级过程中您可能会看到一个警告，询问是否保留您当前的版本或使用软件包中的新版本**。如果您有自定义（非默认）配置，建议保留您的版本，并将 `pid_file` 路径更新为 `/run/manticore/searchd.pid`。不过，即使不更改路径，系统仍应正常工作。

### 新功能和改进
* 🆕 [v13.13.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.13.0) 添加了对带有 SI 块缓存的 MCL 8.1.0 的支持。
* 🆕 [v13.12.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.12.0) 实现了 [secondary_index_block_cache](../Server_settings/Searchd.md#secondary_index_block_cache) 选项，更新了二级索引 API，并内联了排序访问器。

### Bug 修复
* 🪲 [v13.11.8](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.11.8) [ Issue #3791](https://github.com/manticoresoftware/manticoresearch/issues/3791) 修复了检查和调用已触发定时器之间的竞态条件。
* 🪲 [v13.11.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.11.7) [ Issue #1045](https://github.com/manticoresoftware/manticoresearch/issues/1045) 通过将配置中的过时路径 `/var/run/manticore` 替换为正确的 `/run/manticore`，修复了 RHEL 8 在 systemd 更新期间的 systemctl 警告。由于配置文件已更新，升级时可能会看到警告，询问是否保留您当前的版本或使用软件包中的新版本。如果您有自定义（非默认）配置，建议保留您的版本，并将 `pid_file` 路径更新为 `/run/manticore/searchd.pid`。
* 🪲 [v13.11.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.11.6) [ PR #3766](https://github.com/manticoresoftware/manticoresearch/pull/3766) 添加了对 MCL 版本 8.0.6 的支持。
* 🪲 [v13.11.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.11.5) [ PR #3767](https://github.com/manticoresoftware/manticoresearch/pull/3767) 改进了中文文档翻译并更新了子模块。
* 🪲 [v13.11.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.11.4) [ PR #3765](https://github.com/manticoresoftware/manticoresearch/pull/3765) 修复了别名连接属性的处理。
* 🪲 [v13.11.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.11.3) [ PR #3763](https://github.com/manticoresoftware/manticoresearch/pull/3763) 修复了在字符串属性的批量连接中可能发生的崩溃，并解决了过滤器有时在 LEFT JOIN 中不起作用的问题。
* 🪲 [v13.11.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.11.2) [ Issue #3065](https://github.com/manticoresoftware/manticoresearch/issues/3065) 修复了在启用 index_field_lengths 的列式表中插入数据时的崩溃。
* 🪲 [v13.11.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.11.1) [ Issue #3751](https://github.com/manticoresoftware/manticoresearch/issues/3751) 修复了启用嵌入时删除文档导致的崩溃。

## 版本 13.11.1
**发布于**：2025年9月13日

### Bug 修复
* 🪲 [v13.11.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.11.1) [ Issue #3751](https://github.com/manticoresoftware/manticoresearch/issues/3751) 修复了启用嵌入时删除文档导致的崩溃。

## 版本 13.11.0
**发布于**：2025年9月13日

此次发布的主要亮点是[自动嵌入](Searching/KNN.md#Auto-Embeddings-%28Recommended%29)——一项使语义搜索像SQL一样简单的新功能。
无需外部服务或复杂的管道：只需插入文本并使用自然语言进行搜索。

### 自动嵌入提供的功能

- **直接从文本自动生成嵌入**
- **理解语义的自然语言查询，而不仅仅是关键词**
- **支持多种模型**（OpenAI、Hugging Face、Voyage、Jina）
- **与SQL和JSON API的无缝集成**

### 推荐的库
- 推荐的 [MCL](https://github.com/manticoresoftware/columnar) 版本：8.0.3
- 推荐的 [Buddy](Installation/Manticore_Buddy.md#Manticore-Buddy) 版本：3.35.1

如果您遵循[官方安装指南](https://manticoresearch.com/install/)，则无需担心这些。

推荐的 [MCL](https://github.com/manticoresoftware/columnar) 版本：8.0.1
推荐的 [Buddy](Installation/Manticore_Buddy.md#Manticore-Buddy) 版本：3.34.2

### 新功能和改进
* 🆕 [v13.11.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.11.0) [ PR #3746](https://github.com/manticoresoftware/manticoresearch/pull/3746) 在JSON查询中添加了“query”支持，用于嵌入生成。
* 🆕 [v13.10.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.10.0) [ Issue #3709](https://github.com/manticoresoftware/manticoresearch/issues/3709) manticore-server RPM包不再拥有`/run`目录。
* 🆕 [v13.9.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.9.0) [ PR #3716](https://github.com/manticoresoftware/manticoresearch/pull/3716) 在配置中添加了对`boolean_simplify`的支持。
* 🆕 [v13.8.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.8.0) [ Issue #3253](https://github.com/manticoresoftware/manticoresearch/issues/3253) 安装了sysctl配置以增加vm.max_map_count，适用于大型数据集。
* 🆕 [v13.7.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.7.0) [ PR #3681](https://github.com/manticoresoftware/manticoresearch/pull/3681) 添加了对`alter table <table> modify column <column> api_key=<key>`的支持。

### Bug修复
* 🪲 [v13.10.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.10.5) [ PR #3737](https://github.com/manticoresoftware/manticoresearch/pull/3737) scroll选项现在能正确返回所有具有大64位ID的文档。
* 🪲 [v13.10.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.10.4) [ PR #3736](https://github.com/manticoresoftware/manticoresearch/pull/3736) 修复了使用带过滤树的KNN时的崩溃问题。
* 🪲 [v13.10.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.10.3) [ Issue #3520](https://github.com/manticoresoftware/manticoresearch/issues/3520) `/sql`端点不再允许SHOW VERSION命令。
* 🪲 [v13.10.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.10.2) [ PR #3637](https://github.com/manticoresoftware/manticoresearch/pull/3637) 更新了Windows安装脚本。
* 🪲 [v13.10.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.10.1) 修复了Linux上的本地时区检测。
* 🪲 [v13.9.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.9.2) [ PR #3726](https://github.com/manticoresoftware/manticoresearch/pull/3726) 列式模式下重复ID现在正确返回错误。
* 🪲 [v13.9.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.9.1) [ PR #3333](https://github.com/manticoresoftware/manticoresearch/pull/3333) 手册现已自动翻译。
* 🪲 [v13.8.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.8.6) [ PR #3715](https://github.com/manticoresoftware/manticoresearch/pull/3715) 修复了当批处理中所有文档被跳过时的嵌入生成问题。
* 🪲 [v13.8.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.8.5) [ PR #3711](https://github.com/manticoresoftware/manticoresearch/pull/3711) 添加了Jina和Voyage嵌入模型，以及其他自动嵌入相关的更改。
* 🪲 [v13.8.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.8.4) [ PR #3710](https://github.com/manticoresoftware/manticoresearch/pull/3710) 修复了多面查询连接时的崩溃。
* 🪲 [v13.8.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.8.3) 修复了在_bulk端点多语句事务中删除/更新提交未计为错误的问题。
* 🪲 [v13.8.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.8.2) [ PR #3705](https://github.com/manticoresoftware/manticoresearch/pull/3705) 修复了按非列式字符串属性连接时的崩溃，并改进了错误报告。
* 🪲 [v13.8.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.8.1) [ PR #3704](https://github.com/manticoresoftware/manticoresearch/pull/3704) 修复了未指定模型时查询嵌入的崩溃；添加了嵌入字符串到主代理通信；添加了测试。
* 🪲 [v13.7.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.7.2) [ PR #Buddy#589](https://github.com/manticoresoftware/manticoresearch-buddy/pull/589) 移除了模糊搜索的默认IDF技巧。
* 🪲 [v13.7.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.7.1) [ Issue #3454](https://github.com/manticoresoftware/manticoresearch/issues/3454) 修复了大64位ID的滚动解码错误。
* 🪲 [v13.6.9](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.9) [ Issue #3674](https://github.com/manticoresoftware/manticoresearch/issues/3674) 修复了JDBC+MySQL驱动/连接池在transaction_read_only设置下的问题。
* 🪲 [v13.6.8](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.8) [ PR #3676](https://github.com/manticoresoftware/manticoresearch/pull/3676) 修复了嵌入模型返回空结果集时的崩溃问题。

## 版本 13.6.7
**发布**：2025年8月8日

推荐的 [MCL](https://github.com/manticoresoftware/columnar) 版本：8.0.1
推荐的 [Buddy](Installation/Manticore_Buddy.md#Manticore-Buddy) 版本：3.34.2

如果您遵循[官方安装指南](https://manticoresearch.com/install/)，则无需担心此问题。

### 新功能和改进
* 🆕 [v13.6.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.0) [ Issue #2226](https://github.com/manticoresoftware/manticoresearch/issues/2226) 支持在 PHRASE、PROXIMITY 和 QUORUM 操作符中显式使用 '|'（或）。
* 🆕 [v13.5.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.5.0) [ PR #3591](https://github.com/manticoresoftware/manticoresearch/pull/3591) 查询中自动生成嵌入（进行中，尚未准备好用于生产环境）。
* 🆕 [v13.4.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.4.0) [ PR #3585](https://github.com/manticoresoftware/manticoresearch/pull/3585) 修正逻辑，优先使用 buddy_path 配置中的 buddy 线程数（如果设置），而不是使用守护进程的值。
* 🆕 [v13.3.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.3.0) [ PR #3577](https://github.com/manticoresoftware/manticoresearch/pull/3577) 支持与本地分布式表的连接。
* 🆕 [#3647](https://github.com/manticoresoftware/manticoresearch/issues/3647) 添加对 Debian 13 “Trixie”的支持。

### Bug 修复
* 🪲 [v13.6.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.7) [ Issue #3524](https://github.com/manticoresoftware/manticoresearch/issues/3524) 修复了在行存储中保存生成的嵌入时的问题。
* 🪲 [v13.6.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.6) [ Issue #3563](https://github.com/manticoresoftware/manticoresearch/issues/3563) 修复了 Sequel Ace 和其他集成因“未知 sysvar”错误而失败的问题。
* 🪲 [v13.6.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.5) [ Issue #3467](https://github.com/manticoresoftware/manticoresearch/issues/3467) 修复了 DBeaver 和其他集成因“未知 sysvar”错误而失败的问题。
* 🪲 [v13.6.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.4) [ Issue #3524](https://github.com/manticoresoftware/manticoresearch/issues/3524) 修复了多字段嵌入拼接的问题；同时修复了查询中嵌入生成的问题。
* 🪲 [v13.6.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.3) [ Issue #3641](https://github.com/manticoresoftware/manticoresearch/issues/3641) 修复了 13.6.0 版本中短语丢失除第一个外所有括号内关键字的错误。
* 🪲 [v13.6.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.2) 修复了 transform_phrase 中的内存泄漏。
* 🪲 [v13.6.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.1) 修复了 13.6.0 版本中的内存泄漏。
* 🪲 [v13.5.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.5.2) [ Issue #3651](https://github.com/manticoresoftware/manticoresearch/issues/3651) 修复了与全文搜索模糊测试相关的更多问题。
* 🪲 [v13.5.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.5.1) [ Issue #3560](https://github.com/manticoresoftware/manticoresearch/issues/3560) 修复了 OPTIMIZE TABLE 在使用 KNN 数据时可能无限挂起的情况。
* 🪲 [v13.4.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.4.2) [ Issue #2544](https://github.com/manticoresoftware/manticoresearch/issues/2544) 修复了添加 float_vector 列可能导致索引损坏的问题。
* 🪲 [v13.4.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.4.1) [ Issue #3651](https://github.com/manticoresoftware/manticoresearch/issues/3651) 添加了全文解析的模糊测试，并修复了测试中发现的多个问题。
* 🪲 [v13.3.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.3.1) [ Issue #3583](https://github.com/manticoresoftware/manticoresearch/issues/3583) 修复了使用复杂布尔过滤器和高亮显示时的崩溃。
* 🪲 [v13.2.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.7) [ Issue #3481](https://github.com/manticoresoftware/manticoresearch/issues/3481) 修复了 HTTP 更新、分布式表和错误复制集群同时使用时的崩溃。
* 🪲 [v13.2.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.6) [ PR #3567](https://github.com/manticoresoftware/manticoresearch/pull/3567) 更新了 manticore-backup 依赖到版本 1.9.6。
* 🪲 [v13.2.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.5) [ PR #3565](https://github.com/manticoresoftware/manticoresearch/pull/3565) 修复了 CI 设置以提升 Docker 镜像兼容性。
* 🪲 [v13.2.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.4) 修复了长词元的处理。一些特殊词元（如正则表达式模式）可能生成过长的词，因此现在在使用前会进行缩短。

## 版本 13.2.3
**发布**：2025年7月8日

### 重大变更
* ⚠️ [PR #3586](https://github.com/manticoresoftware/manticoresearch/pull/3586) 停止支持 Debian 10 (Buster)。Debian 10 于 2024年6月30日停止维护。建议用户升级到 Debian 11 (Bullseye) 或 Debian 12 (Bookworm)。
* ⚠️ [v13.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.0.0) 更新了 KNN 库 API，以支持空的 [float_vector](https://manual.manticoresearch.com/Creating_a_table/Data_types#Float-vector) 值。此更新不改变数据格式，但增加了 Manticore Search / MCL API 版本。
* ⚠️ [v12.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/12.0.0) [ PR #3516](https://github.com/manticoresoftware/manticoresearch/pull/3516) 修复了 KNN 索引训练和构建过程中源和目标行 ID 不正确的错误。此更新不更改数据格式，但提高了 Manticore Search / MCL API 版本。
* ⚠️ [v11.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.0.0) 添加了对新的 KNN 向量搜索功能的支持，如量化、重评分和过采样。此版本更改了 KNN 数据格式和 [KNN_DIST() SQL 语法](https://manual.manticoresearch.com/Searching/KNN?client=SQL#KNN-vector-search)。新版本可以读取旧数据，但旧版本无法读取新格式。

### 新功能和改进
* 🆕 [v13.2.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.0) [ PR #3549](https://github.com/manticoresoftware/manticoresearch/issues/3525) 修复了 `@@collation_database` 的问题，该问题导致与某些 mysqldump 版本不兼容
* 🆕 [v13.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.1.0) [ Issue #3489](https://github.com/manticoresoftware/manticoresearch/issues/3489) 修复了模糊搜索中的一个错误，该错误阻止了解析某些 SQL 查询
* 🆕 [v12.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/12.1.0) [ Issue #3426](https://github.com/manticoresoftware/manticoresearch/issues/3426) 添加了对 RHEL 10 操作系统的支持
* 🆕 [v11.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.1.0) 添加了对 [KNN 搜索](https://manual.manticoresearch.com/Searching/KNN#KNN-vector-search) 中空浮点向量的支持
* 🆕 [v10.2.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.0) [ Issue #3252](https://github.com/manticoresoftware/manticoresearch/issues/3252) [log_level](https://manual.manticoresearch.com/Server_settings/Setting_variables_online#SET) 现在也调整 Buddy 的日志详细程度

### Bug 修复
* 🪲 [v13.2.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.3) [ PR #3556](https://github.com/manticoresoftware/manticoresearch/pull/3556) 修复了 JSON 查询中“过采样”选项的解析问题
* 🪲 [v13.2.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.2) 通过移除 Boost stacktrace 的使用，修复了 Linux 和 FreeBSD 上的崩溃日志记录问题
* 🪲 [v13.2.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.1) [ Issue #3298](https://github.com/manticoresoftware/manticoresearch/issues/3298) 修复了在容器内运行时的崩溃日志记录问题
* 🪲 [v12.0.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/12.0.2) 通过以微秒计数，提高了每表统计的准确性
* 🪲 [v12.0.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/12.0.1) [ PR #3522](https://github.com/manticoresoftware/manticoresearch/pull/3522) 修复了在联接查询中按 MVA 分面时的崩溃
* 🪲 [v11.0.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.0.3) [ Issue #3502](https://github.com/manticoresoftware/manticoresearch/issues/3502) 修复了与向量搜索量化相关的崩溃
* 🪲 [v11.0.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.0.2) [ Issue #3493](https://github.com/manticoresoftware/manticoresearch/issues/3493) 将 `SHOW THREADS` 中的 CPU 利用率显示更改为整数
* 🪲 [v11.0.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.0.1) 修正了列式和二级库的路径
* 🪲 [v10.2.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.7) 添加了对 MCL 5.0.5 的支持，包括对嵌入库文件名的修复
* 🪲 [v10.2.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.6) [ Issue #3469](https://github.com/manticoresoftware/manticoresearch/issues/3469) 针对问题 #3469 应用另一个修复
* 🪲 [v10.2.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.4) [ Issue #3469](https://github.com/manticoresoftware/manticoresearch/issues/3469) 修复了带有布尔查询的 HTTP 请求返回空结果的问题
* 🪲 [v10.2.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.5) 更改了嵌入库的默认文件名，并添加了对 KNN 向量中 'from' 字段的检查
* 🪲 [v10.2.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.3) [ PR #3464](https://github.com/manticoresoftware/manticoresearch/pull/3464) 将 Buddy 更新到版本 3.30.2，其中包括关于内存使用和错误日志的 [PR #565](https://github.com/manticoresoftware/manticoresearch-buddy/pull/565)
* 🪲 [v10.2.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.2) [ Issue #3470](https://github.com/manticoresoftware/manticoresearch/pull/3470) 修复了 JOIN 查询中的 JSON 字符串过滤器、空过滤器和排序问题
* 🪲 [v10.2.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.1) 修复了 `dist/test_kit_docker_build.sh` 中导致 Buddy 提交缺失的错误
* 🪲 [v10.1.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.4) 修复了在联接查询中按 MVA 分组时的崩溃
* 🪲 [v10.1.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.3) [ Issue #3434](https://github.com/manticoresoftware/manticoresearch/issues/3434) 修复了过滤空字符串的错误
* 🪲 [v10.1.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.2) [ PR #3452](https://github.com/manticoresoftware/manticoresearch/pull/3452) 更新 Buddy 至版本 3.29.7，解决了 [Buddy #507 - 多查询请求中模糊搜索错误](https://github.com/manticoresoftware/manticoresearch-buddy/issues/507) 和 [Buddy #561 - 指标速率修复](https://github.com/manticoresoftware/manticoresearch-buddy/pull/561) 问题，适用于 [Helm Release 10.1.0](https://github.com/manticoresoftware/manticoresearch-helm/releases/tag/manticoresearch-10.1.0)
* 🪲 [v10.1.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.1)  更新 Buddy 至版本 3.29.4，解决了 [#3388 - “未定义的数组键 'Field'”](https://github.com/manticoresoftware/manticoresearch/issues/3388) 和 [Buddy #547 - layouts='ru' 可能无法工作](https://github.com/manticoresoftware/manticoresearch-buddy/issues/547)

## 版本 10.1.0
**发布**：2025年6月9日

此版本为更新版本，包含新功能、一个重大变更以及众多稳定性改进和错误修复。更改重点在于增强监控能力、改进搜索功能以及修复影响系统稳定性和性能的各种关键问题。

**从版本 10.1.0 开始，不再支持 CentOS 7。建议用户升级到受支持的操作系统。**

### 重大变更
* ⚠️ [v10.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.0.0) [ Issue #540](https://github.com/manticoresoftware/manticoresearch-buddy/issues/540) 重大变更：将 [模糊搜索](Searching/Spell_correction.md#Fuzzy-Search) 的默认 `layouts=''`

### 新功能和改进
* 🆕 [v10.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.0) [ Issue #537](https://github.com/manticoresoftware/manticoresearch-buddy/issues/537) 新增内置的 [Prometheus 导出器](Node_info_and_management/Node_status.md#Prometheus-Exporter)
* 🆕 [v9.8.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.8.0) [ Issue #3409](https://github.com/manticoresoftware/manticoresearch/issues/3409) 新增 [ALTER TABLE tbl REBUILD KNN](Updating_table_schema_and_settings.md#Rebuilding-a-KNN-index) 功能
* 🆕 [v9.7.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.7.0) [ Issue #1778](https://github.com/manticoresoftware/manticoresearch/issues/1778) 新增自动生成嵌入功能（尚未正式发布，代码已合入主分支，但仍需更多测试）
* 🆕 [v9.6.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.6.0)  提升 KNN API 版本以支持自动嵌入
* 🆕 [v9.5.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.0) [ Issue #1894](https://github.com/manticoresoftware/manticoresearch/issues/1894) 改进集群恢复：定期保存 `seqno`，加快节点崩溃后的重启速度
* 🆕 [v9.4.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.4.0) [ Issue #2400](https://github.com/manticoresoftware/manticoresearch/issues/2400) 新增对最新 [Logstash](Integration/Logstash.md#Integration-with-Logstash) 和 Beats 的支持

### 错误修复
* 🪲 [v10.0.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.0.1)  修复词形处理：用户定义的词形覆盖自动生成的词形；新增测试用例测试22
* 🪲 [v9.8.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.8.2)  修复：再次更新 deps.txt，包含与嵌入库相关的 MCL 打包修复
* 🪲 [v9.8.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.8.1)  修复：更新 deps.txt，修复 MCL 和嵌入库的打包问题
* 🪲 [v9.7.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.7.3) [ Issue #3306](https://github.com/manticoresoftware/manticoresearch/issues/3306) 修复索引时信号11崩溃问题
* 🪲 [v9.7.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.7.2) [ Issue #3109](https://github.com/manticoresoftware/manticoresearch/issues/3109) 修复不存在的 `@@variables` 总是返回0的问题
* 🪲 [v9.7.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.7.1) [ Issue #3377](https://github.com/manticoresoftware/manticoresearch/issues/3377) 修复与 [remove_repeats()](Functions/Searching_and_ranking_functions.md#REMOVE_REPEATS%28%29) 相关的崩溃
* 🪲 [v9.6.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.6.3) [ PR #3411](https://github.com/manticoresoftware/manticoresearch/pull/3411) 修复：使用动态版本检测以支持遥测指标
* 🪲 [v9.6.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.6.2)  修复：[SHOW VERSION](Node_info_and_management/SHOW_VERSION.md#SHOW-VERSION) 输出的小问题
* 🪲 [v9.6.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.6.1)  修复：创建带有 KNN 属性但无模型的表时崩溃
* 🪲 [v9.5.16](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.16) [ Issue #3342](https://github.com/manticoresoftware/manticoresearch/issues/3342) 修复 `SELECT ... FUZZY=0` 不总是禁用模糊搜索的问题
* 🪲 [v9.5.15](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.15) [ PR #3397](https://github.com/manticoresoftware/manticoresearch/pull/3397) 新增对 [MCL](https://github.com/manticoresoftware/columnar) 4.2.2 的支持；修复旧存储格式错误
* 🪲 [v9.5.14](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.14) [ Issue #3392](https://github.com/manticoresoftware/manticoresearch/issues/3392) 修复 HTTP JSON 回复中字符串处理错误
* 🪲 [v9.5.13](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.13) [ Issue #3356](https://github.com/manticoresoftware/manticoresearch/issues/3356) 修复复杂全文查询（公共子词）情况下的崩溃问题
* 🪲 [v9.5.12](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.12)  修正了磁盘块自动刷新错误消息中的拼写错误
* 🪲 [v9.5.11](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.11) [ Issue #3195](https://github.com/manticoresoftware/manticoresearch/issues/3195) 改进了[自动磁盘块刷新](Securing_and_compacting_a_table/Flushing_RAM_chunk_to_disk.md#Flushing-RAM-chunk-to-disk)：如果正在运行优化，则跳过保存
* 🪲 [v9.5.10](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.10) [ Issue #3313](https://github.com/manticoresoftware/manticoresearch/issues/3313) 修复了使用[indextool](Miscellaneous_tools.md#indextool)检查RT表中所有磁盘块重复ID的问题
* 🪲 [v9.5.9](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.9) [ Issue #3132](https://github.com/manticoresoftware/manticoresearch/issues/3132) 在JSON API中添加了对`_random`排序的支持
* 🪲 [v9.5.8](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.8) [ Issue #3382](https://github.com/manticoresoftware/manticoresearch/issues/3382) 修复了无法通过JSON HTTP API使用uint64文档ID的问题
* 🪲 [v9.5.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.7) [ Issue #3385](https://github.com/manticoresoftware/manticoresearch/issues/3385) 修正了按`id != value`过滤时结果不正确的问题
* 🪲 [v9.5.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.6) [ PR #538](https://github.com/manticoresoftware/manticoresearch-buddy/pull/538) 修复了某些情况下模糊匹配的严重错误
* 🪲 [v9.5.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.5) [ Issue #3199](https://github.com/manticoresoftware/manticoresearch/issues/3199) 修复了Buddy HTTP查询参数中的空格解码问题（例如 `%20` 和 `+`）
* 🪲 [v9.5.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.4) [ Issue #3133](https://github.com/manticoresoftware/manticoresearch/issues/3133) 修正了facet搜索中`json.field`排序不正确的问题
* 🪲 [v9.5.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.3) [ Issue #3091](https://github.com/manticoresoftware/manticoresearch/issues/3091) 修复了SQL与JSON API中分隔符搜索结果不一致的问题
* 🪲 [v9.5.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.2) [ Issue #2819](https://github.com/manticoresoftware/manticoresearch/issues/2819) 性能改进：将分布式表的`DELETE FROM`替换为`TRUNCATE`
* 🪲 [v9.5.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.1) [ Issue #3080](https://github.com/manticoresoftware/manticoresearch/issues/3080) 修复了使用JSON属性过滤别名`geodist()`时的崩溃问题

## 版本 9.3.2
发布日期：2025年5月2日

本次发布包含多个错误修复和稳定性改进，更好的表使用跟踪，以及内存和资源管理的增强。

❤️ 特别感谢 [@cho-m](https://github.com/cho-m) 修复了与Boost 1.88.0的构建兼容性问题，以及 [@benwills](https://github.com/benwills) 改进了关于`stored_only_fields`的文档。

* 🪲 [v9.3.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.3.2)  修复了“显示线程”列显示CPU活动为浮点数而非字符串的问题；同时修正了PyMySQL结果集解析错误，原因是数据类型错误。
* 🪲 [v9.3.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.3.1) [ Issue #3343](https://github.com/manticoresoftware/manticoresearch/issues/3343) 修复了优化过程被中断时遗留的`tmp.spidx`文件问题。
* 🆕 [v9.3.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.3.0) [ PR #3337](https://github.com/manticoresoftware/manticoresearch/pull/3337) 新增了每表命令计数器和详细的表使用统计。
* 🪲 [v9.2.39](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.39) [ Issue #3236](https://github.com/manticoresoftware/manticoresearch/issues/3236) 修复：通过移除复杂块更新防止表损坏。在串行工作线程中使用等待函数破坏了串行处理，可能导致表损坏。
	重新实现了自动刷新。移除了外部轮询队列以避免不必要的表锁。新增“小表”条件：如果文档数低于“小表限制”（8192）且未使用二级索引（SI），则跳过刷新。

* 🪲 [v9.2.38](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.38)  修复：跳过为使用`ALL`/`ANY`字符串列表过滤器创建二级索引（SI），不影响JSON属性。
* 🪲 [v9.2.37](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.37) [ Issue #2898](https://github.com/manticoresoftware/manticoresearch/issues/2898) 为系统表添加了反引号支持。
* 🪲 [v9.2.36](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.36)  修复：在旧代码中为集群操作使用占位符。在解析器中，我们现在清晰区分表名和集群名字段。
* 🪲 [v9.2.35](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.35)  修复：取消引用单个`'`时崩溃。
* 🪲 [v9.2.34](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.34) [ Issue #3090](https://github.com/manticoresoftware/manticoresearch/issues/3090) 修复：大文档ID处理（之前可能无法找到）。
* 🪲 [v9.2.33](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.33)  修复：位向量大小使用无符号整数。
* 🪲 [v9.2.32](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.32)  修复：减少合并期间的峰值内存使用。docid到rowid的查找现在每个文档使用12字节，而非16字节。例如：20亿文档使用24 GB内存，而非36 GB。
* 🪲 [v9.2.31](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.31) [ Issue #3238](https://github.com/manticoresoftware/manticoresearch/issues/3238) 修复：大型实时表中 `COUNT(*)` 值不正确的问题。
* 🪲 [v9.2.30](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.30)  修复：清零字符串属性时的未定义行为。
* 🪲 [v9.2.29](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.29)  小修正：改进了警告文本。
* 🪲 [v9.2.28](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.28) [ Issue #3290](https://github.com/manticoresoftware/manticoresearch/issues/3290) 改进：增强了 `indextool --buildidf`
* 🪲 [v9.2.27](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.27) [ Issue #3032](https://github.com/manticoresoftware/manticoresearch/issues/3032) 通过 Kafka 集成，现在可以为特定 Kafka 分区创建数据源。
* 🪲 [v9.2.26](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.26) [ Issue #3301](https://github.com/manticoresoftware/manticoresearch/issues/3301) 修复：`ORDER BY` 和 `WHERE` 在 `id` 上可能导致 OOM（内存溢出）错误。
* 🪲 [v9.2.25](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.25) [ Issue #3171](https://github.com/manticoresoftware/manticoresearch/issues/3171) 修复：在 RT 表上使用带有多个 JSON 属性的 grouper 并且该表有多个磁盘块时导致的段错误崩溃。
* 🪲 [v9.2.24](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.24) [ Issue #3246](https://github.com/manticoresoftware/manticoresearch/issues/3246) 修复：`WHERE string ANY(...)` 查询在 RAM 块刷新后失败。
* 🪲 [v9.2.23](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.23) [ PR #518](https://github.com/manticoresoftware/manticoresearch-buddy/pull/518) 小幅自动分片语法改进。
* 🪲 [v9.2.22](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.22) [ Issue #2763](https://github.com/manticoresoftware/manticoresearch/issues/2763) 修复：使用 `ALTER TABLE` 时未加载全局 idf 文件。
* 🪲 [v9.2.21](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.21)  修复：全局 idf 文件可能很大。我们现在更早释放表以避免持有不必要的资源。
* 🪲 [v9.2.20](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.20) [ PR #3277](https://github.com/manticoresoftware/manticoresearch/pull/3277) 改进：更好的分片选项验证。

* 🪲 [v9.2.19](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.19) [ PR #3275](https://github.com/manticoresoftware/manticoresearch/pull/3275) 修复：与 Boost 1.88.0 的构建兼容性。
* 🪲 [v9.2.18](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.18) [ Issue #3228](https://github.com/manticoresoftware/manticoresearch/issues/3228) 修复：分布式表创建时崩溃（无效指针问题）。

* 🪲 [v9.2.17](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.17) [ PR #3272](https://github.com/manticoresoftware/manticoresearch/pull/3272) 修复：多行模糊 `FACET` 问题。
* 🪲 [v9.2.16](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.16) [ Issue #3063](https://github.com/manticoresoftware/manticoresearch/issues/3063) 修复：使用 `GEODIST` 函数时距离计算的错误。
* 🪲 [v9.2.15](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.15) [ Issue #3027](https://github.com/manticoresoftware/manticoresearch/issues/3027) 小幅改进：支持 Elastic 的 `query_string` 过滤格式。

## 版本 9.2.14
发布：2025年3月28日

### 小改动
* [提交 ](https://github.com/manticoresoftware/manticoresearch/commit/fe9473dc3f4fcfa0eaae748b538fe853f4ce078b) 实现了 `--mockstack` 标志，用于计算递归操作的栈需求。新的 `--mockstack` 模式分析并报告递归表达式求值、模式匹配操作、过滤处理所需的栈大小。计算出的栈需求输出到控制台，便于调试和优化。
* [Issue #3058](https://github.com/manticoresoftware/manticoresearch/pull/3058) 默认启用 [boolean_simplify](Searching/Options.md#boolean_simplify)。
* [Issue #3172](https://github.com/manticoresoftware/manticoresearch/issues/3172) 添加了新的配置选项：`searchd.kibana_version_string`，当使用特定版本的 Kibana 或 OpenSearch Dashboards 并期望特定 Elasticsearch 版本时非常有用。
* [Issue #3211](https://github.com/manticoresoftware/manticoresearch/issues/3211) 修复了 [CALL SUGGEST](Searching/Spell_correction.md#CALL-QSUGGEST,-CALL-SUGGEST) 以支持 2 字符单词。
* [Issue #490](https://github.com/manticoresoftware/manticoresearch-buddy/issues/490) 改进了 [模糊搜索](Searching/Spell_correction.md#Fuzzy-Search)：之前在搜索 "def ghi" 时，如果存在另一个匹配文档，有时无法找到 "defghi"。
* ⚠️ 重大变更 [Issue #3165](https://github.com/manticoresoftware/manticoresearch/issues/3165) 在部分 HTTP JSON 响应中将 `_id` 改为 `id` 以保持一致性。请确保相应更新您的应用程序。
* ⚠️ 重大变更 [Issue #3186](https://github.com/manticoresoftware/manticoresearch/issues/3186) 在集群加入时增加了对 `server_id` 的检查，以确保每个节点具有唯一的 ID。现在，当加入节点的 `server_id` 与集群中已有节点的 `server_id` 相同时，`JOIN CLUSTER` 操作可能会失败并显示重复 [server_id](Server_settings/Searchd.md#server_id) 的错误信息。为解决此问题，请确保复制集群中的每个节点具有唯一的 [server_id](Server_settings/Searchd.md#server_id)。您可以在配置文件的 "searchd" 部分将默认的 [server_id](Server_settings/Searchd.md#server_id) 更改为唯一值，然后再尝试加入集群。此更改更新了复制协议。如果您正在运行复制集群，需要：
  - 首先，干净地停止所有节点
  - 然后，使用 Linux 下的工具 `manticore_new_cluster`，以 `--new-cluster` 参数启动最后停止的节点。
  - 详细信息请参阅 [重启集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster)。

### Bug修复
* [Commit 6fda](https://github.com/manticoresoftware/manticoresearch/commit/6fdad3923dd6953f4b781943eed5ec5c28b7f808) 修复了等待后调度器丢失导致的崩溃；现在，像 `serializer` 这样的特定调度器能够正确恢复。
* [Commit c333](https://github.com/manticoresoftware/manticoresearch/commit/c333b277d4a504de5fe298e9ab570a9f9ea31e0a) 修复了右表权重无法在 `ORDER BY` 子句中使用的错误。
* [Issue #2644](https://github.com/manticoresoftware/manticoresearch/issues/2644) gcc 14.2.0：修复了 `lower_bound` 调用时 `const knn::DocDist_t*&` 的错误。❤️ 感谢 [@Azq2](https://github.com/Azq2) 的 PR。
* [Issue #3018](https://github.com/manticoresoftware/manticoresearch/issues/3018) 修复了自动模式插入时处理大写表名的问题。
* [Issue #3119](https://github.com/manticoresoftware/manticoresearch/issues/3119) 修复了解码无效 base64 输入时的崩溃。
* [Issue #3121](https://github.com/manticoresoftware/manticoresearch/issues/3121) 修复了两个与 KNN 索引 ALTER 相关的问题：浮点向量现在保持原始维度，KNN 索引生成正确。
* [Issue #3123](https://github.com/manticoresoftware/manticoresearch/issues/3123) 修复了在空 JSON 列上构建二级索引时的崩溃。
* [Issue #3138](https://github.com/manticoresoftware/manticoresearch/issues/3138) 修复了由重复条目引起的崩溃。
* [Issue #3151](https://github.com/manticoresoftware/manticoresearch/issues/3151) 修复了 `fuzzy=1` 选项无法与 `ranker` 或 `field_weights` 一起使用的问题。
* [Issue #3163](https://github.com/manticoresoftware/manticoresearch/issues/3163) 修复了 `SET GLOBAL timezone` 无效的错误。
* [Issue #3181](https://github.com/manticoresoftware/manticoresearch/issues/3181) 修复了使用大于 2^63 的 ID 时文本字段值可能丢失的问题。
* [Issue #3189](https://github.com/manticoresoftware/manticoresearch/issues/3189) 修复了 `UPDATE` 语句现在正确遵守 `query_log_min_msec` 设置的问题。
* [Issue #3247](https://github.com/manticoresoftware/manticoresearch/issues/3247) 修复了保存实时磁盘块时的竞态条件，可能导致 `JOIN CLUSTER` 失败。


## 版本 7.4.6
发布：2025年2月28日

### 主要变更
* [Issue #832](https://github.com/manticoresoftware/manticoresearch/issues/832) 集成了 [Kibana](Integration/Kibana.md)，实现更简便高效的数据可视化。

### 次要变更
* [Issue #1727](https://github.com/manticoresoftware/manticoresearch/issues/1727) 修复了 arm64 和 x86_64 之间的浮点精度差异。
* [Issue #2995](https://github.com/manticoresoftware/manticoresearch/issues/2995) 实现了 join 批处理的性能优化。
* [Issue #3039](https://github.com/manticoresoftware/manticoresearch/issues/3039) 实现了直方图中 EstimateValues 的性能优化。
* [Issue #3099](https://github.com/manticoresoftware/manticoresearch/issues/3099) 增加了对 Boost 1.87.0 的支持。❤️ 感谢 [@cho-m](https://github.com/cho-m) 的 PR。
* [Issue #77](https://github.com/manticoresoftware/columnar/issues/77) 优化了创建多值过滤器时的块数据重用；为属性元数据添加了最小/最大值；实现了基于最小/最大值的过滤值预过滤。

### Bug修复
* [Commit 73ac](https://github.com/manticoresoftware/manticoresearch/commit/73ac22f358a7a0e734b332c3943c86e6294c06c3) 修复了在连接查询中使用左右表属性时表达式的处理；修复了右表的 index_weights 选项。
* [Issue #2915](https://github.com/manticoresoftware/manticoresearch/issues/2915) 在 `SELECT ... JOIN` 查询中使用 `avg()` 可能导致结果错误；现已修复。
* [Issue #2996](https://github.com/manticoresoftware/manticoresearch/issues/2996) 修复了启用 join 批处理时隐式截断导致的错误结果集。
* [Issue #3031](https://github.com/manticoresoftware/manticoresearch/issues/3031) 修复了守护进程关闭时活跃块合并导致的崩溃。
* [Issue #3037](http://github.com/manticoresoftware/manticoresearch/issues/3037) 修复了 `IN(...)` 可能产生错误结果的问题。
* [Issue #3038](https://github.com/manticoresoftware/manticoresearch/issues/3038) 7.0.0 版本中设置 `max_iops` / `max_iosize` 可能降低索引性能；现已修复。
* [Issue #3042](https://github.com/manticoresoftware/manticoresearch/issues/3042) 修复了连接查询缓存中的内存泄漏。
* [Issue #3052](https://github.com/manticoresoftware/manticoresearch/issues/3052) 修复了连接 JSON 查询中查询选项的处理。
* [Issue #3054](https://github.com/manticoresoftware/manticoresearch/issues/3054) 修复了 ATTACH TABLE 命令的问题。
* [Issue #3079](https://github.com/manticoresoftware/manticoresearch/issues/3079) 修正了错误信息中的不一致问题。
* [Issue #3087](https://github.com/manticoresoftware/manticoresearch/issues/3087) 每个表设置 `diskchunk_flush_write_timeout=-1` 未能禁用索引刷新；现已修复。
* [Issue #3088](https://github.com/manticoresoftware/manticoresearch/issues/3088) 解决了批量替换大ID后出现的重复条目问题。
* [Issue #3126](https://github.com/manticoresoftware/manticoresearch/issues/3126) 修复了由单个 `NOT` 操作符和表达式排序器的全文查询导致的守护进程崩溃。
* [Issue #3128](https://github.com/manticoresoftware/manticoresearch/pull/3128) 修复了CJSON库中的潜在漏洞。❤️ 感谢 [@tabudz](https://github.com/tabudz) 的PR。

## 版本 7.0.0
发布：2025年1月30日

### 主要变更
* [Issue #1497](https://github.com/manticoresoftware/manticoresearch/issues/1497) 新增了便于搜索的 [模糊搜索](Searching/Spell_correction.md#Fuzzy-Search) 和 [自动补全](Searching/Autocomplete.md#CALL-AUTOCOMPLETE) 功能。
* [Issue #1500](https://github.com/manticoresoftware/manticoresearch/issues/1500) [Kafka 集成](Integration/Kafka.md#Syncing-from-Kafka)。
* [Issue #1928](https://github.com/manticoresoftware/manticoresearch/issues/1928) 引入了 [JSON 的二级索引](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#json_secondary_indexes)。
* [Issue #2361](https://github.com/manticoresoftware/manticoresearch/issues/2361) 更新和更新期间的搜索不再被块合并阻塞。
* [Issue #2787](https://github.com/manticoresoftware/manticoresearch/issues/2787) 实时表的自动 [磁盘块刷新](Server_settings/Searchd.md#diskchunk_flush_write_timeout) 以提升性能；现在我们会自动将内存块刷新到磁盘块，防止因内存块缺乏优化而导致的性能问题，这有时会根据块大小导致不稳定。
* [Issue #2811](https://github.com/manticoresoftware/manticoresearch/issues/2811) 新增了便于分页的 [滚动](Searching/Pagination.md#Scroll-Search-Option) 选项。
* [Issue #931](https://github.com/manticoresoftware/manticoresearch/issues/931) 集成了 [Jieba](https://github.com/fxsjy/jieba) 以改善 [中文分词](Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md)。

### 次要变更
* ⚠️ 破坏性变更 [Issue #1111](https://github.com/manticoresoftware/manticoresearch/issues/1111) 修复了实时表中对 `global_idf` 的支持。需要重新创建表。
* ⚠️ 破坏性变更 [Issue #2103](https://github.com/manticoresoftware/manticoresearch/issues/2103) 从内部 `cjk` 字符集移除了泰文字符。请相应更新您的字符集定义：如果您有 `cjk,non_cjk` 并且泰文字符对您很重要，请改为 `cjk,thai,non_cjk`，或使用 `cont,non_cjk`，其中 `cont` 是所有连续书写语言（即 `cjk` + `thai`）的新标识。使用 [ALTER TABLE](Updating_table_schema_and_settings.md#Updating-table-FT-settings-in-RT-mode) 修改现有表。
* ⚠️ 破坏性变更 [Issue #2468](https://github.com/manticoresoftware/manticoresearch/issues/2468) [CALL SUGGEST / QSUGGEST](Searching/Spell_correction.md#CALL-QSUGGEST,-CALL-SUGGEST) 现支持分布式表。这增加了主/代理协议版本。如果您在多实例分布式环境中运行 Manticore Search，请确保先升级代理，再升级主节点。
* ⚠️ 破坏性变更 [Issue #2889](https://github.com/manticoresoftware/manticoresearch/issues/2889) PQ [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META-for-PQ-tables) 的列名由 `Name` 改为 `Variable name`。
* ⚠️ 破坏性变更 [Issue #879](https://github.com/manticoresoftware/manticoresearch/issues/879) 引入了带有新选项的 [每表二进制日志](Logging/Binary_logging.md#Per-table-binary-logging-configuration)：[binlog_common](Logging/Binary_logging.md#Binary-logging-strategies)、[binlog](Logging/Binary_logging.md#Per-table-binary-logging-configuration) 用于 `create table` / `alter table`。升级到新版本前需干净关闭 Manticore 实例。
* ⚠️ 破坏性变更 [Issue #1789](https://github.com/manticoresoftware/manticoresearch/issues/1789) 修正了节点以错误的复制协议版本加入集群时的错误信息。此更改更新了复制协议。如果您运行复制集群，需要：
  - 首先，干净停止所有节点
  - 然后，使用 Linux 下的工具 `manticore_new_cluster`，以 `--new-cluster` 参数启动最后停止的节点。
  - 详情请参阅 [重启集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster)。
* ⚠️ 破坏性变更 [Issue #2308](https://github.com/manticoresoftware/manticoresearch/issues/2308) 在 [`ALTER CLUSTER ADD` 和 `DROP`](Creating_a_cluster/Setting_up_replication/Adding_and_removing_a_table_from_a_replication_cluster.md#Adding-and-removing-a-table-from-a-replication-cluster) 中增加了对多表的支持。此更改也影响复制协议。请参考前一节了解如何处理此更新。
* [Issue #2997](https://github.com/manticoresoftware/manticoresearch/issues/2997) 修复了 Macos 上的 dlopen 问题。
* [Commit 4954](https://github.com/manticoresoftware/manticoresearch/commit/4954b5de7341a29902a9b8fbb9a040f7942c77c4) 更改了带有 KNN 索引的表的 OPTIMIZE TABLE 默认截止点，以提升搜索性能。
* [Commit cfc8](https://github.com/manticoresoftware/manticoresearch/commit/cfc87ecb6e33a8163c2235243b6b40e699dbf526) 为 `FACET` 和 `GROUP BY` 中的 `ORDER BY` 添加了 `COUNT(DISTINCT)` 支持。
* [Issue #1103](https://github.com/manticoresoftware/manticoresearch/issues/1103) 改进了 [日志](Logging/Server_logging.md#Server-logging) 中块合并的清晰度。
* [Issue #1130](https://github.com/manticoresoftware/manticoresearch/issues/1130) 添加了对 [DBeaver](Integration/DBeaver.md) 的支持。
* [Issue #1546](https://github.com/manticoresoftware/manticoresearch/issues/1546) 为 [POLY2D()](Functions/Geo_spatial_functions.md#POLY2D%28%29)/[GEOPOLY2D()](Functions/Geo_spatial_functions.md#GEOPOLY2D%28%29) 函数实现了二级索引。
* [Issue #1630](https://github.com/manticoresoftware/manticoresearch/issues/1630) HTTP 请求现在支持 `Content-Encoding: gzip`。
* [Issue #1831](https://github.com/manticoresoftware/manticoresearch/issues/1831) 添加了 `SHOW LOCKS` 命令。
* [Issue #2187](https://github.com/manticoresoftware/manticoresearch/issues/2197) 允许 Buddy 请求绕过守护进程的 [searchd.max_connections](Server_settings/Searchd.md#max_connections) 限制。
* [Issue #2208](https://github.com/manticoresoftware/manticoresearch/issues/2208) 增加了通过 JSON HTTP 接口连接表的支持。
* [Issue #2235](https://github.com/manticoresoftware/manticoresearch/issues/2235) 通过 Buddy 以原始形式记录成功处理的查询。
* [Issue #2249](https://github.com/manticoresoftware/manticoresearch/issues/2249) 为复制表添加了运行 `mysqldump` 的特殊模式。
* [Issue #2268](https://github.com/manticoresoftware/manticoresearch/issues/2268) 改进了 `CREATE TABLE` 和 `ALTER TABLE` 语句中外部文件的重命名复制操作。
* [Issue #2402](https://github.com/manticoresoftware/manticoresearch/issues/2402) 更新了 [searchd.max_packet_size](Server_settings/Searchd.md#max_packet_size) 的默认值为 128MB。
* [Issue #2419](https://github.com/manticoresoftware/manticoresearch/issues/2419) 在 JSON ["match"](Searching/Full_text_matching/Basic_usage.md#match) 中添加了对 [IDF boost modifier](Searching/Full_text_matching/Operators.md#IDF-boost-modifier) 的支持。
* [Issue #2430](https://github.com/manticoresoftware/manticoresearch/issues/2430) 加强了 [binlog](Logging/Binary_logging.md#Binary-logging) 写入同步以防止错误。
* [Issue #2458](https://github.com/manticoresoftware/manticoresearch/issues/2458) 在 Windows 包中集成了 zlib 支持。
* [Issue #2479](https://github.com/manticoresoftware/manticoresearch/issues/2479) 添加了对 SHOW TABLE INDEXES 命令的支持。
* [Issue #2485](https://github.com/manticoresoftware/manticoresearch/issues/2485) 为 Buddy 回复设置了会话元数据。
* [Issue #2490](https://github.com/manticoresoftware/manticoresearch/issues/2490) 兼容端点的聚合支持毫秒级分辨率。
* [Issue #2500](https://github.com/manticoresoftware/manticoresearch/issues/2500) 当复制启动失败时，修改了集群操作的错误消息。
* [Issue #2584](https://github.com/manticoresoftware/manticoresearch/pull/2584) 新增 [SHOW STATUS](Node_info_and_management/Node_status.md#Query-Time-Statistics) 中的性能指标：最近 1、5 和 15 分钟内每种查询类型的最小/最大/平均/95%/99% 百分位数。
* [Issue #2639](https://github.com/manticoresoftware/manticoresearch/pull/2639) 在请求和响应中将所有 `index` 替换为 `table`。
* [Issue #2643](https://github.com/manticoresoftware/manticoresearch/issues/2643) 在 HTTP `/sql` 端点的聚合结果中添加了 `distinct` 列。
* [Issue #268](https://github.com/manticoresoftware/manticoresearch-buddy/issues/268) 实现了从 Elasticsearch 导入数据类型的自动检测。
* [Issue #2744](https://github.com/manticoresoftware/manticoresearch/issues/2744) 为字符串 JSON 字段比较表达式添加了排序规则支持。
* [Issue #2752](https://github.com/manticoresoftware/manticoresearch/issues/2752) 支持在选择列表中使用 `uuid_short` 表达式。
* [Issue #2783](https://github.com/manticoresoftware/manticoresearch/issues/2783) Manticore Search 现在直接运行 Buddy，无需 `manticore-buddy` 包装器。
* [Issue #2785](https://github.com/manticoresoftware/manticoresearch/issues/2785) 区分了缺少表和不支持插入操作的表的错误消息。
* [Issue #2789](https://github.com/manticoresoftware/manticoresearch/issues/2789) OpenSSL 3 现已静态编译进 `searchd`。
* [Issue #2790](https://github.com/manticoresoftware/manticoresearch/issues/2790) 添加了 `CALL uuid_short` 语句，用于生成包含多个 `uuid_short` 值的序列。
* [Issue #2803](https://github.com/manticoresoftware/manticoresearch/issues/2803) 为 JOIN 操作中的右表添加了单独的选项。
* [Issue #2810](https://github.com/manticoresoftware/manticoresearch/issues/2810) 改进了 HTTP JSON 聚合性能，使其与 SphinxQL 中的 `GROUP BY` 相匹配。
* [Issue #2854](https://github.com/manticoresoftware/manticoresearch/issues/2854) 支持 Kibana 日期相关请求中的 `fixed_interval`。
* [Issue #2909](https://github.com/manticoresoftware/manticoresearch/issues/2909) 实现了 JOIN 查询的批处理，显著提升了某些 JOIN 查询的性能，提升幅度达数百甚至数千倍。
* [Issue #2937](https://github.com/manticoresoftware/manticoresearch/issues/2937) 允许在 fullscan 查询中使用连接表的权重。
* [Issue #2953](https://github.com/manticoresoftware/manticoresearch/issues/2953) 修复了连接查询的日志记录。
* [Issue #337](https://github.com/manticoresoftware/manticoresearch-buddy/issues/337) 在非调试模式下隐藏了 Buddy 异常，不再显示在 `searchd` 日志中。
* [Issue #2931](https://github.com/manticoresoftware/manticoresearch/issues/2931) 当用户为复制监听器设置错误端口时，守护进程关闭并显示错误消息。

### Bug 修复
* [Commit 0c6b](https://github.com/manticoresoftware/manticoresearch/commit/0c6bdaf41ea3684e05952e1a8273893535b923f5) 修复：当 JOIN 查询中列数超过 32 时返回错误结果。
* [Issue #2335](https://github.com/manticoresoftware/manticoresearch/issues/2335) 解决了当条件中使用两个 json 属性时表连接失败的问题。
* [Issue #2338](https://github.com/manticoresoftware/manticoresearch/issues/2338) 修正了带有 [cutoff](Searching/Options.md#cutoff) 的多查询中 total_relation 计算错误的问题。
* [Issue #2366](https://github.com/manticoresoftware/manticoresearch/issues/2366) 修正了在 [表连接](Searching/Joining.md) 中对右表 `json.string` 的过滤。
* [Issue #2406](https://github.com/manticoresoftware/manticoresearch/issues/2406) 允许在任何 POST HTTP JSON 端点（insert/replace/bulk）中使用 `null` 作为所有值，此时会使用默认值。
* [Issue #2418](https://github.com/manticoresoftware/manticoresearch/issues/2418) 通过调整初始 socket 探测的 [max_packet_size](Server_settings/Searchd.md#max_packet_size) 网络缓冲区分配，优化了内存消耗。
* [Issue #2420](https://github.com/manticoresoftware/manticoresearch/issues/2420) 修正了通过 JSON 接口将无符号整数插入 bigint 属性的问题。
* [Issue #2422](https://github.com/manticoresoftware/manticoresearch/issues/2422) 修正了启用 pseudo_sharding 时，二级索引与排除过滤器配合使用的问题。
* [Issue #2423](https://github.com/manticoresoftware/manticoresearch/issues/2423) 解决了 [manticore_new_cluster](Starting_the_server/Manually.md#searchd-command-line-options) 中的一个错误。
* [Issue #2448](https://github.com/manticoresoftware/manticoresearch/issues/2448) 解决了在格式错误的 `_update` 请求时守护进程崩溃的问题。
* [Issue #2452](https://github.com/manticoresoftware/manticoresearch/issues/2452) 修正了直方图无法处理带排除的值过滤器的问题。
* [Issue #55](https://github.com/manticoresoftware/columnar/issues/55) 修正了针对分布式表的 [knn](Searching/KNN.md#KNN-vector-search) 查询。
* [Issue #68](https://github.com/manticoresoftware/columnar/issues/68) 增强了列存访问器中对表编码的排除过滤器处理。
* [Commit 0eb1](https://github.com/manticoresoftware/manticoresearch/commit/0eb1579462013cb134bcaf74e06f6eeb0c2ecc4d) 修正了表达式解析器未遵守重新定义的 `thread_stack` 的问题。
* [Commit c304](https://github.com/manticoresoftware/manticoresearch/commit/c3041fd1d3e14773f3c5aa635343915a71f5cb95) 修正了克隆列存 IN 表达式时的崩溃。
* [Commit edad](https://github.com/manticoresoftware/columnar/commit/edadc694c68d6022bdd13134263667430a42cc1d) 修正了位图迭代器中的反转问题，该问题导致崩溃。
* [Commit fc30](https://github.com/manticoresoftware/executor/commit/fc302ff1117b0b835a6f5a3c07057baf5fba14d9) 修正了部分 Manticore 软件包被 `unattended-upgrades` 自动移除的问题。
* [Issue #1019](https://github.com/manticoresoftware/manticoresearch/issues/1019) 改进了对 DbForge MySQL 工具发起查询的处理。
* [Issue #1107](https://github.com/manticoresoftware/manticoresearch/issues/1107) 修正了 `CREATE TABLE` 和 `ALTER TABLE` 中特殊字符的转义。❤️ 感谢 [@subnix](https://github.com/subnix) 的 PR。
* [Issue #116](https://github.com/manticoresoftware/manticoresearch-backup/issues/116#issuecomment-2216597206) 修正了在冻结索引中更新 blob 属性时的死锁。死锁是由于尝试解冻索引时锁冲突引起的，也可能导致 manticore-backup 失败。
* [Issue #1818](https://github.com/manticoresoftware/manticoresearch/issues/1818) 当表被冻结时，`OPTIMIZE` 现在会抛出错误。
* [Issue #2001](https://github.com/manticoresoftware/manticoresearch/issues/2001) 允许函数名用作列名。
* [Issue #2153](https://github.com/manticoresoftware/manticoresearch/issues/2153) 修正了查询表设置时遇到未知磁盘块导致守护进程崩溃的问题。
* [Issue #2184](https://github.com/manticoresoftware/manticoresearch/issues/2184) 修正了 `searchd` 在执行 `FREEZE` 和 `FLUSH RAMCHUNK` 后停止时挂起的问题。
* [Issue #2228](https://github.com/manticoresoftware/manticoresearch/issues/2228) 从保留字中移除了日期/时间相关的标记（及正则表达式）。
* [Issue #2255](https://github.com/manticoresoftware/manticoresearch/issues/2255) 修正了 `FACET` 使用超过 5 个排序字段时的崩溃。
* [Issue #2265](https://github.com/manticoresoftware/manticoresearch/issues/2265) 修正了启用 `index_field_lengths` 时无法恢复 `mysqldump` 的问题。
* [Issue #2291](https://github.com/manticoresoftware/manticoresearch/issues/2291) 修正了执行 `ALTER TABLE` 命令时的崩溃。
* [Issue #2333](https://github.com/manticoresoftware/manticoresearch/issues/2333) 修正了 Windows 包中 MySQL DLL 使索引器正常工作的错误。
* [Issue #2393](https://github.com/manticoresoftware/manticoresearch/issues/2393) 修正了 GCC 编译错误。❤️ 感谢 [@animetosho](https://github.com/animetosho) 的 PR。
* [Issue #2447](https://github.com/manticoresoftware/manticoresearch/issues/2447) 修正了 [_update](Data_creation_and_modification/Updating_documents/REPLACE.md?client=Elasticsearch-like+partial) 中的转义问题。
* [Issue #2460](https://github.com/manticoresoftware/manticoresearch/issues/2460) 修正了声明多个同名属性或字段时索引器崩溃的问题。
* [Issue #2467](https://github.com/manticoresoftware/manticoresearch/issues/2467) 解决了针对“兼容”搜索相关端点的嵌套布尔查询错误转换导致守护进程崩溃的问题。
* [Issue #2493](https://github.com/manticoresoftware/manticoresearch/issues/2493) 修正了带修饰符短语中的扩展问题。
* [Issue #2535](https://github.com/manticoresoftware/manticoresearch/issues/2535) 解决了使用 [ZONE](Searching/Full_text_matching/Operators.md#ZONE-limit-operator) 或 [ZONESPAN](Searching/Full_text_matching/Operators.md#ZONESPAN-limit-operator) 全文操作符时守护进程崩溃的问题。
* [Issue #2552](https://github.com/manticoresoftware/manticoresearch/issues/2552) 修正了带关键词字典的普通表和 RT 表的中缀生成问题。
* [Issue #2571](https://github.com/manticoresoftware/manticoresearch/issues/2571) 修复了 `FACET` 查询中的错误回复；为带有 `COUNT(*)` 的 `FACET` 设置默认排序顺序为 `DESC`。
* [Issue #2580](https://github.com/manticoresoftware/manticoresearch/issues/2580) 修复了 Windows 启动时守护进程崩溃的问题。
* [Issue #2603](https://github.com/manticoresoftware/manticoresearch/issues/2603) 修复了 HTTP 端点 `/sql` 和 `/sql?mode=raw` 的查询截断问题；使这些端点的请求一致，无需 `query=` 头。
* [Issue #2623](https://github.com/manticoresoftware/manticoresearch/issues/2623) 修复了自动模式创建表但同时失败的问题。
* [Issue #2627](https://github.com/manticoresoftware/manticoresearch/issues/2627) 修复了 HNSW 库以支持加载多个 KNN 索引。
* [Issue #2630](https://github.com/manticoresoftware/manticoresearch/issues/2630) 修复了多个条件同时发生时的冻结问题。
* [Issue #2645](https://github.com/manticoresoftware/manticoresearch/issues/2645) 修复了使用 `OR` 与 KNN 搜索时导致的致命错误崩溃。
* [Issue #2647](https://github.com/manticoresoftware/manticoresearch/issues/2647) 修复了 `indextool --mergeidf *.idf --out global.idf` 在创建输出文件后删除该文件的问题。
* [Issue #2658](https://github.com/manticoresoftware/manticoresearch/issues/2658) 修复了带有外层选择中 `ORDER BY` 字符串的子选择导致守护进程崩溃的问题。
* [Issue #2686](https://github.com/manticoresoftware/manticoresearch/issues/2686) 修复了更新浮点属性和字符串属性时的崩溃问题。
* [Issue #2704](https://github.com/manticoresoftware/manticoresearch/issues/2704) 修复了 `lemmatize_xxx_all` 分词器中多个停用词增加后续标记 `hitpos` 的问题。
* [Issue #2708](https://github.com/manticoresoftware/manticoresearch/issues/2708) 修复了 `ALTER ... ADD COLUMN ... TEXT` 时的崩溃。
* [Issue #2737](https://github.com/manticoresoftware/manticoresearch/issues/2737) 修复了在至少有一个 RAM 块的冻结表中更新 blob 属性导致后续 `SELECT` 查询等待表解冻的问题。
* [Issue #2742](https://github.com/manticoresoftware/manticoresearch/issues/2742) 修复了带有打包因子的查询跳过查询缓存的问题。
* [Issue #2775](https://github.com/manticoresoftware/manticoresearch/issues/2775) Manticore 现在在 `_bulk` 请求中遇到未知操作时报告错误，而不是崩溃。
* [Issue #2791](https://github.com/manticoresoftware/manticoresearch/issues/2791) 修复了 HTTP `_bulk` 端点返回插入文档 ID 的问题。
* [Issue #2797](https://github.com/manticoresoftware/manticoresearch/issues/2797) 修复了在处理多个表时，某个表为空且另一个表匹配条目数不同导致 grouper 崩溃的问题。
* [Issue #2835](https://github.com/manticoresoftware/manticoresearch/issues/2835) 修复了复杂 `SELECT` 查询中的崩溃。
* [Issue #2872](https://github.com/manticoresoftware/manticoresearch/issues/2872) 添加了错误信息，当 `IN` 表达式中的 `ALL` 或 `ANY` 参数不是 JSON 属性时提示错误。
* [Issue #2882](https://github.com/manticoresoftware/manticoresearch/issues/2882) 修复了在大表中更新 MVA 时守护进程崩溃的问题。
* [Issue #2888](https://github.com/manticoresoftware/manticoresearch/issues/2888) 修复了 `libstemmer` 分词失败时的崩溃。❤️ 感谢 [@subnix](https://github.com/subnix) 的 PR。
* [Issue #2919](https://github.com/manticoresoftware/manticoresearch/issues/2919) 修复了右表连接权重在表达式中未正确工作的情况。
* [Issue #2919](https://github.com/manticoresoftware/manticoresearch/issues/2919) 修复了右连接表权重在表达式中不起作用的问题。
* [Issue #325](https://github.com/manticoresoftware/manticoresearch-buddy/issues/325) 修复了当表已存在时，`CREATE TABLE IF NOT EXISTS ... WITH DATA` 失败的问题。
* [Issue #351](https://github.com/manticoresoftware/manticoresearch-buddy/issues/351) 修复了按 KNN 使用文档 ID 计数时出现的未定义数组键 "id" 错误。
* [Issue #359](https://github.com/manticoresoftware/manticoresearch-buddy/issues/359) 修复了 `REPLACE INTO cluster_name:table_name` 功能。
* [Issue #67](https://github.com/manticoresoftware/docker/issues/67) 修复了使用 `--network=host` 运行 Manticore Docker 容器时的致命错误。

## 版本 6.3.8
发布：2024年11月22日

版本 6.3.8 继续 6.3 系列，仅包含错误修复。

### 错误修复

* [PR #2777](https://github.com/manticoresoftware/manticoresearch/pull/2777) 修复了当查询并发受 `threads` 或 `max_threads_per_query` 设置限制时可用线程数的计算。

## manticore-extra v1.1.20

发布：2024年10月7日

### 错误修复

* [Issue #64](https://github.com/manticoresoftware/executor/issues/64) 解决了 `unattended-upgrades` 工具在基于 Debian 的系统上自动安装软件包更新时，错误地将多个 Manticore 软件包（包括 `manticore-galera`、`manticore-executor` 和 `manticore-columnar-lib`）标记为删除的问题。该问题是由于 `dpkg` 错误地将虚拟包 `manticore-extra` 视为冗余。已做出更改，确保 `unattended-upgrades` 不再尝试删除关键的 Manticore 组件。

## 版本 6.3.6
发布：2024年8月2日

版本 6.3.6 继续 6.3 系列，仅包含错误修复。

### 错误修复

* [Issue #2477](https://github.com/manticoresoftware/manticoresearch/issues/2477) 修复了 6.3.4 版本引入的崩溃，该崩溃可能在处理表达式和分布式或多表时发生。
* [Issue #2352](https://github.com/manticoresoftware/manticoresearch/issues/2352) 修复了因 `max_query_time` 导致查询多个索引时提前退出引起的守护进程崩溃或内部错误。

## 版本 6.3.4
发布：2024年7月31日

版本 6.3.4 继续 6.3 系列，仅包含小幅改进和错误修复。

### 小幅更改
* [Issue #1130](https://github.com/manticoresoftware/manticoresearch/issues/1130) 添加了对 [DBeaver](Integration/DBeaver.md#Integration-with-DBeaver) 的支持。
* [Issue #2146](https://github.com/manticoresoftware/manticoresearch/issues/2146) 改进了词形和例外中分隔符的转义。
* [Issue #2315](https://github.com/manticoresoftware/manticoresearch/issues/2315) 在 SELECT 列表达式中添加了字符串比较运算符。
* [Issue #2363](https://github.com/manticoresoftware/manticoresearch/issues/2363) 添加了对 Elastic 风格批量请求中 null 值的支持。
* [Issue #2374](https://github.com/manticoresoftware/manticoresearch/issues/2374) 添加了对 mysqldump 版本 9 的支持。
* [Issue #2375](https://github.com/manticoresoftware/manticoresearch/issues/2375) 改进了 HTTP JSON 查询中的错误处理，增加了错误发生节点的 JSON 路径。

### Bug 修复
* [Issue #2280](https://github.com/manticoresoftware/manticoresearch/issues/2280) 修复了当 disk_chunks > 1 时，通配符查询匹配项多导致的性能下降问题。
* [Issue #2332](https://github.com/manticoresoftware/manticoresearch/issues/2332) 修复了空 MVA 数组在 MVA MIN 或 MAX SELECT 列表达式中导致的崩溃。
* [Issue #2339](https://github.com/manticoresoftware/manticoresearch/issues/2339) 修复了 Kibana 无限范围请求的错误处理。
* [Issue #2342](https://github.com/manticoresoftware/manticoresearch/issues/2342) 修复了当属性不在 SELECT 列表中时，右表的列式属性上的连接过滤器问题。
* [Issue #2343](https://github.com/manticoresoftware/manticoresearch/issues/2343) 修复了 Manticore 6.3.2 中重复的 'static' 说明符。
* [Issue #2344](https://github.com/manticoresoftware/manticoresearch/issues/2344) 修复了使用 MATCH() 作用于右表时，LEFT JOIN 返回非匹配条目的问题。
* [Issue #2350](https://github.com/manticoresoftware/manticoresearch/issues/2350) 修复了带有 `hitless_words` 的 RT 索引磁盘块保存问题。
* [Issue #2364](https://github.com/manticoresoftware/manticoresearch/issues/2364) 现在 'aggs_node_sort' 属性可以在其他属性中任意顺序添加。
* [Issue #2368](https://github.com/manticoresoftware/manticoresearch/issues/2368) 修复了 JSON 查询中全文与过滤器顺序错误的问题。
* [Issue #2376](https://github.com/manticoresoftware/manticoresearch/issues/2376) 修复了长 UTF-8 请求导致的 JSON 响应错误的 bug。
* [Issue #2684](https://github.com/manticoresoftware/dev/issues/2684) 修复了依赖连接属性的 presort/prefilter 表达式计算错误。
* [Issue #301](https://github.com/manticoresoftware/manticoresearch-buddy/issues/301) 修改了指标数据大小的计算方法，改为从 `manticore.json` 文件读取，而非检查整个数据目录大小。
* [Issue #302](https://github.com/manticoresoftware/manticoresearch-buddy/issues/302) 添加了对 Vector.dev 验证请求的处理。

## 版本 6.3.2
发布：2024年6月26日

版本 6.3.2 继续 6.3 系列，包含多个 bug 修复，其中一些是在 6.3.0 发布后发现的。

### 重大变更
* ⚠️[Issue #2305](https://github.com/manticoresoftware/manticoresearch/issues/2305) 更新了 aggs.range 的值为数值类型。

### Bug 修复
* [Commit c51c](https://github.com/manticoresoftware/manticoresearch/commit/c51c7fa4bd0ec4ad8d53188b888461bff29f1417) 修复了存储检查与 rset 合并时的分组问题。
* [Commit 0e85](https://github.com/manticoresoftware/manticoresearch/commit/0e85b9d5682f51681750d798aef6a11cfb93d69e) 修复了在 RT 索引中使用 CRC 字典和启用 `local_df` 时，带通配符查询导致守护进程崩溃的问题。
* [Issue #2200](https://github.com/manticoresoftware/manticoresearch/issues/2200) 修复了在没有 GROUP BY 的 JOIN 中使用 `count(*)` 导致的崩溃。
* [Issue #2201](https://github.com/manticoresoftware/manticoresearch/issues/2201) 修复了尝试按全文字段分组时，JOIN 未返回警告的问题。
* [Issue #2230](https://github.com/manticoresoftware/manticoresearch/issues/2230) 解决了通过 `ALTER TABLE` 添加属性时未考虑 KNN 选项的问题。
* [Issue #2231](https://github.com/manticoresoftware/manticoresearch/issues/2231) 修复了 6.3.0 版本中无法卸载 `manticore-tools` Redhat 包的问题。
* [Issue #2242](https://github.com/manticoresoftware/manticoresearch/issues/2242) 修正了 JOIN 和多个 FACET 语句返回错误结果的问题。
* [Issue #2250](https://github.com/manticoresoftware/manticoresearch/issues/2250) 修复了当表在集群中时，ALTER TABLE 报错的问题。
* [Issue #2252](https://github.com/manticoresoftware/manticoresearch/issues/2252) 修复了 SphinxQL 接口传递给 buddy 的原始查询问题。
* [Issue #2267](https://github.com/manticoresoftware/manticoresearch/issues/2267) 改进了带磁盘块的 RT 索引中 `CALL KEYWORDS` 的通配符扩展。
* [Issue #271](https://github.com/manticoresoftware/manticoresearch-buddy/issues/271) 修复了错误的 `/cli` 请求导致的挂起。
* [Issue #274](https://github.com/manticoresoftware/manticoresearch-buddy/issues/274) 解决了对 Manticore 的并发请求可能卡住的问题。
* [Issue #275](https://github.com/manticoresoftware/manticoresearch-buddy/issues/275) 修复了通过 `/cli` 执行 `drop table if exists t; create table t` 导致的挂起。

### 复制相关
* [Issue #2270](https://github.com/manticoresoftware/manticoresearch/issues/2270) 在 `/_bulk` HTTP 端点中添加了对 `cluster:name` 格式的支持。

## 版本 6.3.0
发布：2024年5月23日

### 主要变更
* [Issue #839](https://github.com/manticoresoftware/manticoresearch/issues/839) 实现了 [float_vector](Creating_a_table/Data_types.md#Float-vector) 数据类型；实现了 [向量搜索](Searching/KNN.md#KNN-vector-search)。
* [Issue #1673](https://github.com/manticoresoftware/manticoresearch/issues/1673) 实现了 [INNER/LEFT JOIN](Searching/Joining.md)（**测试阶段**）。
* [Issue #1744](https://github.com/manticoresoftware/manticoresearch/issues/1744) 实现了对 [timestamp](Creating_a_table/Data_types.md#Timestamps) 字段日期格式的自动检测。
* [Issue #1720](https://github.com/manticoresoftware/manticoresearch/pull/1720) 将 Manticore Search 许可证从 GPLv2-or-later 更改为 GPLv3-or-later。
* [Commit 7a55](https://github.com/manticoresoftware/manticoresearch/commit/7a5508424) 现在在 Windows 上运行 Manticore 需要 Docker 来运行 Buddy。
* [Issue #1541](https://github.com/manticoresoftware/manticoresearch/issues/1541) 添加了一个 [REGEX](Searching/Full_text_matching/Operators.md#REGEX-operator) 全文操作符。
* [Issue #2091](https://github.com/manticoresoftware/manticoresearch/issues/2091) 支持 Ubuntu Noble 24.04。
* [Commit 514d](https://github.com/manticoresoftware/manticoresearch/commit/514d35b497f4bdb20b6473ff963752a978a4bb8d) 重构时间操作以提升性能并新增日期/时间函数：
  - [CURDATE()](Functions/Date_and_time_functions.md#CURDATE%28%29) - 返回本地时区的当前日期
  - [QUARTER()](Functions/Date_and_time_functions.md#QUARTER%28%29) - 返回时间戳参数对应的年份季度整数
  - [DAYNAME()](Functions/Date_and_time_functions.md#DAYNAME%28%29) - 返回时间戳参数对应的星期名称
  - [MONTHNAME()](Functions/Date_and_time_functions.md#MONTHNAME%28%29) - 返回时间戳参数对应的月份名称
  - [DAYOFWEEK()](Functions/Date_and_time_functions.md#DAYOFWEEK%28%29) - 返回时间戳参数对应的星期整数索引
  - [DAYOFYEAR()](Functions/Date_and_time_functions.md#DAYOFYEAR%28%29) - 返回时间戳参数对应的年份中的天数整数
  - [YEARWEEK()](Functions/Date_and_time_functions.md#YEARWEEK%28%29) - 返回时间戳参数对应的年份整数及当前周第一天的日期代码
  - [DATEDIFF()](Functions/Date_and_time_functions.md#DATEDIFF%28%29) - 返回两个时间戳之间的天数差
  - [DATE()](Functions/Date_and_time_functions.md#DATE%28%29) - 格式化时间戳参数的日期部分
  - [TIME()](Functions/Date_and_time_functions.md#TIME%28%29) - 格式化时间戳参数的时间部分
  - [timezone](Server_settings/Searchd.md#timezone) - 日期/时间相关函数使用的时区。
* [Commit 30e7](https://github.com/manticoresoftware/manticoresearch/commit/30e789d9657f9b3093c280c6e5b0e46a021f6dc7) 向 HTTP 接口添加了 [range](Searching/Faceted_search.md#Facet-over-set-of-ranges)、[histogram](Searching/Faceted_search.md#Facet-over-histogram-values)、[date_range](Searching/Faceted_search.md#Facet-over-set-of-date-ranges) 和 [date_histogram](Searching/Faceted_search.md#Facet-over-histogram-date-values) 聚合，并将类似表达式引入 SQL。

### 小改动
* [Issue #1285](https://github.com/manticoresoftware/manticoresearch/issues/1285) 支持 Filebeat 版本 8.10 - 8.11。
* [Issue #1771](https://github.com/manticoresoftware/manticoresearch/issues/1771) 支持 [ALTER TABLE ... type='distributed'](Updating_table_schema_and_settings.md#Changing-a-distributed-table)。
* [Issue #1788](https://github.com/manticoresoftware/manticoresearch/issues/1788) 通过 [CREATE TABLE ... LIKE ... WITH DATA](Creating_a_table/Local_tables/Real-time_table.md#CREATE-TABLE-LIKE:) SQL 语句新增复制表的功能。
* [Issue #2072](https://github.com/manticoresoftware/manticoresearch/issues/2072) 优化了 [表压缩算法](Securing_and_compacting_a_table/Compacting_a_table.md#Compacting-a-Table)：之前，手动的 [OPTIMIZE](Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE) 和自动的 [auto_optimize](Server_settings/Searchd.md#auto_optimize) 过程都会先合并数据块以确保数量不超过限制，然后从所有包含已删除文档的数据块中清除已删除文档。此方法有时资源消耗过大，现已禁用。现在，数据块合并仅根据 [progressive_merge](Server_settings/Common.md#progressive_merge) 设置进行。但包含大量已删除文档的数据块更可能被合并。
* [Commit ce6c](https://github.com/manticoresoftware/columnar/commit/ce6c5d3381edcdc411a00a5923d13d56dd9b3010) 增加了防止加载新版二级索引的保护。
* [Issue #1417](https://github.com/manticoresoftware/manticoresearch/issues/1417) 通过 [REPLACE INTO ... SET](Data_creation_and_modification/Updating_documents/REPLACE.md#SQL-REPLACE) 实现部分替换。
* [Commit 7c16](https://github.com/manticoresoftware/manticoresearch/commit/7c167b934ce416d3dfbd658744a231e638b14d9a) 更新了默认合并缓冲区大小：`.spa`（标量属性）：256KB -> 8MB；`.spb`（blob 属性）：256KB -> 8MB；`.spc`（列式属性）：1MB，未变；`.spds`（文档存储）：256KB -> 8MB；`.spidx`（二级索引）：256KB 缓冲区 -> 128MB 内存限制；`.spi`（字典）：256KB -> 16MB；`.spd`（文档列表）：8MB，未变；`.spp`（命中列表）：8MB，未变；`.spe`（跳跃列表）：256KB -> 8MB。
* [Issue #1859](https://github.com/manticoresoftware/manticoresearch/issues/1859) 通过 JSON 添加了 [复合聚合](Searching/Grouping.md#GROUP-BY-multiple-fields-at-once)。
* [Commit 216b](https://github.com/manticoresoftware/manticoresearch-buddy/commit/216b824) 由于某些正则表达式模式存在问题且无显著时间优势，禁用了 PCRE.JIT。
* [Commit 55cd](https://github.com/manticoresoftware/manticoresearch/commit/55cdce32) 添加了对原生 Galera v.3（api v25）（MySQL 5.x 的 `libgalera_smm.so`）的支持。
* [Commit 86f9](https://github.com/manticoresoftware/manticoresearch-buddy/commit/86f98ab) 将指标后缀从 `_rate` 改为 `_rps`。
* [Commit c0c1](https://github.com/manticoresoftware/manticoresearch-helm/commit/c0c1cff780031412ef57cefab987dfd16613368e) 改进了关于负载均衡器高可用性支持的文档。
* [提交 d1d2](https://github.com/manticoresoftware/manticoresearch/commit/d1d2ca18857c68106e89743756260e73285be739) 将错误消息中的 `index` 改为 `table`；修复了 bison 解析器错误消息。
* [提交 fd26](https://github.com/manticoresoftware/manticoresearch/commit/fd26671b) 支持将 `manticore.tbl` 作为表名。
* [问题 #1105](https://github.com/manticoresoftware/manticoresearch/issues/1105) 支持通过 systemd 运行索引器 ([文档](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Schedule-indexer-via-systemd))。❤️ 感谢 [@subnix](https://github.com/subnix) 的 PR。
* [问题 #1294](https://github.com/manticoresoftware/manticoresearch/issues/1294) GEODIST() 中支持二级索引。
* [问题 #1394](https://github.com/manticoresoftware/manticoresearch/issues/1394) 简化了 [SHOW THREADS](Node_info_and_management/SHOW_THREADS.md#SHOW-THREADS)。
* [问题 #1424](https://github.com/manticoresoftware/manticoresearch/issues/1424) 为 `create distributed table` 语句添加了默认值支持（`agent_connect_timeout` 和 `agent_query_timeout`）。
* [问题 #1442](https://github.com/manticoresoftware/manticoresearch/issues/1442) 添加了 [expansion_limit](Searching/Options.md#expansion_limit) 搜索查询选项，可覆盖 `searchd.expansion_limit`。
* [问题 #1448](https://github.com/manticoresoftware/manticoresearch/issues/1448) 实现了 [ALTER TABLE](Updating_table_schema_and_settings.md#Updating-table-schema-in-RT-mode) 用于 int->bigint 转换。
* [问题 #1456](https://github.com/manticoresoftware/manticoresearch/issues/1456) MySQL 响应中添加元信息。
* [问题 #1494](https://github.com/manticoresoftware/manticoresearch/issues/1494) [SHOW VERSION](Node_info_and_management/SHOW_VERSION.md#SHOW-VERSION)。
* [问题 #1582](https://github.com/manticoresoftware/manticoresearch/issues/1582) 支持通过 JSON 按 id 数组删除文档 ([文档](Data_creation_and_modification/Deleting_documents.md?client=JSON#Deleting-documents))。
* [问题 #1589](https://github.com/manticoresoftware/manticoresearch/issues/1589) 改进错误信息“unsupported value type”。
* [问题 #1634](https://github.com/manticoresoftware/manticoresearch/issues/1634) 在 `SHOW STATUS` 中添加 Buddy 版本。
* [问题 #1641](https://github.com/manticoresoftware/manticoresearch/issues/1641) 针对关键词无文档情况优化匹配请求。
* [问题 #1712](https://github.com/manticoresoftware/manticoresearch/issues/1712) 在提交数据时添加从字符串 "true" 和 "false" 转换为布尔属性的支持。
* [问题 #1713](https://github.com/manticoresoftware/manticoresearch/issues/1713) 添加了 [access_dict](Server_settings/Searchd.md#access_dict) 表和 searchd 选项。
* [问题 #1767](https://github.com/manticoresoftware/manticoresearch/issues/1767) 在配置的 searchd 部分添加了新选项：[expansion_merge_threshold_docs](Server_settings/Searchd.md#expansion_merge_threshold_docs) 和 [expansion_merge_threshold_hits](Server_settings/Searchd.md#expansion_merge_threshold_hits)；使扩展词的微小词合并阈值可配置。
* [问题 #1768](https://github.com/manticoresoftware/manticoresearch/issues/1768) 在 `@@system.sessions` 中添加显示最后命令时间。
* [问题 #1806](https://github.com/manticoresoftware/manticoresearch/issues/1806) 升级 Buddy 协议至版本 2。
* [问题 #1810](https://github.com/manticoresoftware/manticoresearch/issues/1810) 为 `/sql` 端点实现了额外的请求格式，便于与库集成。
* [问题 #1825](https://github.com/manticoresoftware/manticoresearch/pull/1825) 在 SHOW BUDDY PLUGINS 中添加信息部分。
* [问题 #1837](https://github.com/manticoresoftware/manticoresearch/issues/1837) 改进了 `CALL PQ` 处理大包时的内存消耗。
* [问题 #1853](https://github.com/manticoresoftware/manticoresearch/pull/1853) 编译器从 Clang 15 切换到 Clang 16。
* [问题 #1857](https://github.com/manticoresoftware/manticoresearch/issues/1857) 添加字符串比较。❤️ 感谢 [@etcd](https://github.com/etcd) 的 PR。
* [问题 #1915](https://github.com/manticoresoftware/manticoresearch/issues/1915) 添加对连接存储字段的支持。
* [问题 #1937](https://github.com/manticoresoftware/manticoresearch/issues/1937) 在查询日志中记录客户端的 host:port。
* [问题 #1981](https://github.com/manticoresoftware/manticoresearch/issues/1981) 修正错误的错误信息。
* [问题 #1983](https://github.com/manticoresoftware/manticoresearch/issues/1983) 引入了针对 [通过 JSON 的查询计划](Node_info_and_management/Profiling/Query_plan.md?client=JSON#Query-plan) 的详细级别支持。
* [问题 #2010](https://github.com/manticoresoftware/manticoresearch/issues/2010) 除非设置了 `log_level=debug`，否则禁用 Buddy 查询的日志记录。
* [问题 #2035](https://github.com/manticoresoftware/manticoresearch/issues/2035) 支持 Linux Mint 21.3。
* [问题 #2056](https://github.com/manticoresoftware/manticoresearch/issues/2056) Manticore 无法用 Mysql 8.3+ 构建。
* [问题 #2112](https://github.com/manticoresoftware/manticoresearch/issues/2112) 针对实时表块添加 `DEBUG DEDUP` 命令，用于处理附加包含重复项的普通表后可能出现的重复条目。
* [问题 #212](https://github.com/manticoresoftware/manticoresearch-buddy/issues/212) 在 [SHOW QUERIES](Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES) 中添加时间。
* [问题 #218](https://github.com/manticoresoftware/manticoresearch-buddy/issues/218) 将 `@timestamp` 列处理为时间戳。
* [问题 #252](https://github.com/manticoresoftware/manticoresearch-buddy/issues/250) 实现启用/禁用 buddy 插件的逻辑。
* [问题 #254](https://github.com/manticoresoftware/manticoresearch-buddy/issues/254) 更新 composer 至修复了近期 CVE 的较新版本。
* [Issue #340](https://github.com/manticoresoftware/manticoresearch/issues/340) Manticore systemd 单元中与 `RuntimeDirectory` 相关的轻微优化。
* [Issue #51](https://github.com/manticoresoftware/executor/issues/51) 添加了 rdkafka 支持并更新到 PHP 8.3.3。
* [Issue #527](https://github.com/manticoresoftware/manticoresearch/issues/527) 支持[附加](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Attaching_one_table_to_another.md#Attaching-one-table-to-another) RT 表。新增命令 [ALTER TABLE ... RENAME](Updating_table_schema_and_settings.md#Renaming-a-real-time-table)。

### 重大变更和废弃
* ⚠️[Issue #1436](https://github.com/manticoresoftware/manticoresearch/issues/1436) 修正了 IDF 计算问题。`local_df` 现在为默认值。改进了主从搜索协议（版本已更新）。如果您在分布式环境中运行多个 Manticore Search 实例，请确保先升级代理，再升级主节点。
* ⚠️[Issue #1572](https://github.com/manticoresoftware/manticoresearch/issues/1572) 添加了分布式表的复制功能并更新了复制协议。如果您运行复制集群，需要：
  - 首先，干净地停止所有节点
  - 然后，使用 Linux 下的工具 `manticore_new_cluster`，以 `--new-cluster` 参数启动最后停止的节点。
  - 详细信息请参阅[重启集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster)。
* ⚠️[Issue #1763](https://github.com/manticoresoftware/manticoresearch/pull/1763) HTTP API 端点别名 `/json/*` 已被废弃。
* ⚠️[Issue #1982](https://github.com/manticoresoftware/manticoresearch/issues/1982) JSON 中将[profile](Node_info_and_management/Profiling/Query_profile.md#Query-profile) 改为 [plan](Node_info_and_management/Profiling/Query_plan.md#Query-plan)，并新增了 JSON 查询分析功能。
* ⚠️[Commit e235](https://github.com/manticoresoftware/manticoresearch-backup/commit/e23585a) manticore-backup 不再备份 `plugin_dir`。
* ⚠️[Issue #171](https://github.com/manticoresoftware/manticoresearch-buddy/issues/155) 将 Buddy 迁移到 Swoole 以提升性能和稳定性。切换到新版本时，请确保所有 Manticore 包均已更新。
* ⚠️[Issue #196](https://github.com/manticoresoftware/manticoresearch-buddy/pull/205) 将所有核心插件合并到 Buddy 中，并更改了核心逻辑。
* ⚠️[Issue #2107](https://github.com/manticoresoftware/manticoresearch/issues/2107) 在 `/search` 响应中将文档 ID 视为数字。
* ⚠️[Issue #38](https://github.com/manticoresoftware/executor/issues/38) 添加了 Swoole，禁用了 ZTS，并移除了 parallel 扩展。
* ⚠️[Issue #1929](https://github.com/manticoresoftware/manticoresearch/issues/1929) 在某些情况下，`charset_table` 中的覆盖未生效。

### 复制相关变更
* [Commit 3376](https://github.com/manticoresoftware/manticoresearch/commit/3376351f73db3ff0cb6cb708a304a7bd83e69cfb) 修复了大文件 SST 复制错误。
* [Commit 6d36](https://github.com/manticoresoftware/manticoresearch/commit/6d36c68fb7ce3cb274b467aac435788905084952) 为复制命令添加了重试机制；修复了网络繁忙且丢包时复制加入失败的问题。
* [Commit 842e](https://github.com/manticoresoftware/manticoresearch/commit/842e27e71790833ab4125d49103d2933d5df60c3) 将复制中的 FATAL 消息改为 WARNING 消息。
* [Commit 8c32](https://github.com/manticoresoftware/manticoresearch/commit/8c32bce7db39d8ed3b1a06aaa1d2c3ec372084f0) 修正了无表或空表的复制集群中 `gcache.page_size` 的计算；同时修复了 Galera 选项的保存和加载。
* [Commit a2af](https://github.com/manticoresoftware/manticoresearch/commit/a2af06ca36abcbf893e43c8803853130b397c54b) 新增功能，跳过加入集群节点上的更新节点复制命令。
* [Commit c054](https://github.com/manticoresoftware/manticoresearch/commit/c0541e6892f5116b1f957b40060ba8ecd01a5afb) 修复了更新 blob 属性与替换文档时复制过程中的死锁。
* [Commit e80d](https://github.com/manticoresoftware/manticoresearch/commit/e80d505b963999016033613c40c1cf3151d648e7) 添加了 [replication_connect_timeout](Server_settings/Searchd.md#replication_connect_timeout)、[replication_query_timeout](Server_settings/Searchd.md#replication_query_timeout)、[replication_retry_delay](Server_settings/Searchd.md#replication_retry_delay)、[replication_retry_count](Server_settings/Searchd.md#replication_retry_count) 等 searchd 配置选项，用于控制复制期间的网络，类似于 `searchd.agent_*`，但默认值不同。
* [Issue #1356](https://github.com/manticoresoftware/manticoresearch/issues/1356) 修复了复制节点在部分节点丢失且名称解析失败后的重试问题。
* [Issue #1445](https://github.com/manticoresoftware/manticoresearch/issues/1445) 修正了 `show variables` 中复制日志的详细级别。
* [Issue #1482](https://github.com/manticoresoftware/manticoresearch/issues/1482) 修复了 Kubernetes 中 pod 重启时加入节点连接集群的复制问题。
* [Issue #1962](https://github.com/manticoresoftware/manticoresearch/issues/1962) 修复了空集群中无效节点名导致复制修改长时间等待的问题。

### Bug 修复
* [Commit 8a48](https://github.com/manticoresoftware/manticoresearch/commit/8a486c7ba1a293879cdecc41fb52eb7ab0cbf832) 修复了 `count distinct` 中未使用匹配项清理可能导致崩溃的问题。
* [Issue #1569](https://github.com/manticoresoftware/manticoresearch/issues/1569) 二进制日志现在以事务粒度写入。
* [Issue #2089](https://github.com/manticoresoftware/manticoresearch/issues/2089) 修复了与64位ID相关的一个错误，该错误可能导致通过MySQL插入时出现“Malformed packet”错误，进而导致[表损坏和ID重复](https://github.com/manticoresoftware/manticoresearch/issues/2090)。
* [Issue #2160](https://github.com/manticoresoftware/manticoresearch/issues/2160) 修正了日期被插入时错误地当作UTC时间而非本地时区的问题。
* [Issue #2177](https://github.com/manticoresoftware/manticoresearch/issues/2177) 修复了在实时表中使用非空的`index_token_filter`执行搜索时发生的崩溃。
* [Issue #2209](https://github.com/manticoresoftware/manticoresearch/issues/2209) 更改了RT列存储中的重复过滤，修复了崩溃和错误的查询结果。
* [Commit 001d](https://github.com/manticoresoftware/manticoresearch/commit/001d82904e4b887c49387308291aeefb07c0fbc4) 修复了html剥离器在处理连接字段后导致内存损坏的问题。
* [Commit 00eb](https://github.com/manticoresoftware/manticoresearch/commit/00eb2ebf) 避免在刷新后回绕流，防止与mysqldump的通信错误。
* [Commit 0553](https://github.com/manticoresoftware/manticoresearch/commit/05534114acf08c70ffcc41a23d558dec09a34373) 如果预读尚未开始，则不等待其完成。
* [Commit 055a](https://github.com/manticoresoftware/manticoresearch/commit/055aac09f877570cd29b6594cb283182eba7f73b) 修复了Buddy输出字符串过大时在searchd日志中分多行显示的问题。
* [Commit 0a88](https://github.com/manticoresoftware/manticoresearch/commit/0a88056307f82c50fcdc458ad6dbc307a00e3f92) 移动了MySQL接口关于失败的header `debugv`详细级别的警告。
* [Commit 150a](https://github.com/manticoresoftware/manticoresearch/commit/150a20286b7d98d24b844ae577d7fda4a444798d) 修复了多集群管理操作中的竞态条件；禁止创建同名或同路径的多个集群。
* [Commit 2e40](https://github.com/manticoresoftware/manticoresearch/commit/2e405c1c9ebac6f46206e3df17ac1c19129c9aa3) 修复了全文查询中的隐式截断；将MatchExtended拆分为模板部分D。
* [Commit 75f5](https://github.com/manticoresoftware/manticoresearch/commit/75f51421542a2a0658f9e2f9f31048c8c5224479) 修正了`index_exact_words`在索引和加载表到守护进程时的不一致。
* [Commit 7643](https://github.com/manticoresoftware/manticoresearch/commit/7643976bea142e08535d701000eff0d09a99fda2) 修复了删除无效集群时缺失的错误信息。
* [Commit 7a03](https://github.com/manticoresoftware/manticoresearch/commit/7a034ac990d5ec9344320c05a6a2315583b9b858) 修复了CBO与队列联合；修正了CBO与RT伪分片。
* [Commit 7b4e](https://github.com/manticoresoftware/manticoresearch/commit/7b4ea95e) 在未加载二级索引（SI）库和配置参数时，避免发出误导性的警告信息“WARNING: secondary_indexes set but failed to initialize secondary library”。
* [Commit 8496](https://github.com/manticoresoftware/manticoresearch/commit/8496317f099fade8d2db463fb88483e7fcdfe85b) 修正了quorum中的命中排序。
* [Commit 8973](https://github.com/manticoresoftware/manticoresearch-buddy/commit/8973ad3) 修复了ModifyTable插件中大写选项的问题。
* [Commit 9935](https://github.com/manticoresoftware/manticoresearch-buddy/commit/99350d4c184379b19f3efa9751d9b5be5e6edfb5) 修复了从包含空json值（表示为NULL）的转储恢复的问题。
* [Commit a28f](https://github.com/manticoresoftware/manticoresearch/commit/a28f6e087b3d957e48816070be17d0823904936a) 通过使用pcon修复了连接节点接收SST时的SST超时。
* [Commit b5a5](https://github.com/manticoresoftware/manticoresearch/commit/b5a55cc26debcddcda5d9caef8366a707e0dd763) 修复了选择别名字符串属性时的崩溃。
* [Commit c556](https://github.com/manticoresoftware/manticoresearch/commit/c556e7ed8b6da8eac8ef094650df5f0a8977916b) 添加了对带有`morphology_skip_fields`字段的全文查询中，将词项转换为`=term`的查询转换。
* [Commit cdc3](https://github.com/manticoresoftware/manticoresearch/commit/cdc38085145cd1025b96fcac2a46188a0425d9e1) 添加了缺失的配置键（skiplist_cache_size）。
* [Commit cf6e](https://github.com/manticoresoftware/manticoresearch/commit/cf6ec3bfdc3f0990fb31ff53822b2331a6667727) 修复了表达式排序器在处理大型复杂查询时的崩溃。
* [Commit e513](https://github.com/manticoresoftware/manticoresearch/commit/e5139ba75eef7223df375ae3cc3f138fa7d925f1) 修正了全文CBO与无效索引提示的冲突。
* [Commit eb05](https://github.com/manticoresoftware/manticoresearch/commit/eb0514b3a6f57f278252b0724eebd112ec116e0f) 在关闭时中断预读以加快关闭速度。
* [Commit f945](https://github.com/manticoresoftware/manticoresearch/commit/f94555a293cfa51004ff81135b8bd8500b955115) 更改了全文查询的堆栈计算，避免复杂查询时崩溃。
* [Issue #1262](https://github.com/manticoresoftware/manticoresearch/issues/1262) 修复了索引器在索引具有多个同名列的SQL源时崩溃的问题。
* [Issue #1273](https://github.com/manticoresoftware/manticoresearch/issues/1273) 对不存在的系统变量返回0而非<empty>。
* [Issue #1289](https://github.com/manticoresoftware/manticoresearch/issues/1289) 修复了indextool检查RT表外部文件时的错误。
* [Issue #1335](https://github.com/manticoresoftware/manticoresearch/issues/1335) 修正了由于短语内多词形导致的查询解析错误。
* [Issue #1364](https://github.com/manticoresoftware/manticoresearch/issues/1364) 添加了对旧版本binlog的空binlog文件重放支持。
* [Issue #1365](https://github.com/manticoresoftware/manticoresearch/issues/1365) 修复了删除最后一个空binlog文件的问题。
* [Issue #1372](https://github.com/manticoresoftware/manticoresearch/issues/1372) 修正了在 `data_dir` 变更后，守护进程启动时当前工作目录受影响导致的相对路径错误（转换为守护进程启动目录的绝对路径）。
* [Issue #1393](https://github.com/manticoresoftware/manticoresearch/issues/1393) hn_small 中最慢时间退化：守护进程启动时获取/缓存 CPU 信息。
* [Issue #1395](https://github.com/manticoresoftware/manticoresearch/issues/1395) 修正了索引加载时关于缺失外部文件的警告。
* [Issue #1402](https://github.com/manticoresoftware/manticoresearch/issues/1402) 修正了在释放数据指针属性时全局分组器崩溃的问题。
* [Issue #1403](https://github.com/manticoresoftware/manticoresearch/issues/1403) _ADDITIONAL_SEARCHD_PARAMS 无效。
* [Issue #1427](https://github.com/manticoresoftware/manticoresearch/issues/1427) 修正了每表 `agent_query_timeout` 被默认查询选项 `agent_query_timeout` 替换的问题。
* [Issue #1444](https://github.com/manticoresoftware/manticoresearch/issues/1444) 修正了使用 `packedfactors()` 并且每个匹配有多个值时，分组器和排序器崩溃的问题。
* [Issue #1458](https://github.com/manticoresoftware/manticoresearch/issues/1458) Manticore 在频繁索引更新时崩溃。
* [Issue #1481](https://github.com/manticoresoftware/manticoresearch/issues/1481) 修正了解析错误后解析查询清理时的崩溃。
* [Issue #1484](https://github.com/manticoresoftware/manticoresearch/issues/1484) 修正了 HTTP JSON 请求未被路由到 buddy 的问题。
* [Issue #1499](https://github.com/manticoresoftware/manticoresearch/issues/1499) JSON 属性根值不能是数组的问题已修复。
* [Issue #1507](https://github.com/manticoresoftware/manticoresearch/issues/1507) 修正了事务内表重建时崩溃的问题。
* [Issue #1515](https://github.com/manticoresoftware/manticoresearch/issues/1515) 修正了俄语词干短形式的扩展。
* [Issue #1579](https://github.com/manticoresoftware/manticoresearch/issues/1579) 修正了在 [date_format](Functions/Date_and_time_functions.md#DATE_FORMAT()) 表达式中 JSON 和 STRING 属性的使用。
* [Issue #1580](https://github.com/manticoresoftware/manticoresearch/issues/1580) 修正了对 JSON 字段多个别名的分组器。
* [Issue #1594](https://github.com/manticoresoftware/manticoresearch/issues/1594) 修正了 dev 中 total_related 错误：修正了隐式 cutoff 与 limit；增加了 JSON 查询中更好的全扫描检测。
* [Issue #1603](https://github.com/manticoresoftware/manticoresearch/issues/1603) 修正了所有日期表达式中 JSON 和 STRING 属性的使用。
* [Issue #1609](https://github.com/manticoresoftware/manticoresearch/issues/1609) 使用 LEVENSHTEIN() 时崩溃。
* [Issue #1612](https://github.com/manticoresoftware/manticoresearch/issues/1612) 修正了带高亮的搜索查询解析错误后内存损坏的问题。
* [Issue #1614](https://github.com/manticoresoftware/manticoresearch/issues/1614) 禁用了对短于 `min_prefix_len` / `min_infix_len` 的词的通配符扩展。
* [Issue #1617](https://github.com/manticoresoftware/manticoresearch/issues/1617) 修改行为：如果 Buddy 成功处理请求，则不记录错误。
* [Issue #1635](https://github.com/manticoresoftware/manticoresearch/issues/1635) 修正了设置了 limit 的查询的搜索查询元信息中的 total。
* [Issue #1640](https://github.com/manticoresoftware/manticoresearch/issues/1640) 无法在纯模式下通过 JSON 使用大写表名。
* [Issue #1643](https://github.com/manticoresoftware/manticoresearch/issues/1643) 提供了默认的 `SPH_EXTNODE_STACK_SIZE` 值。
* [Issue #1646](https://github.com/manticoresoftware/manticoresearch/issues/1646) 修正了 MVA 属性上带 ALL/ANY 的负过滤器的 SphinxQL 日志。
* [Issue #1660](https://github.com/manticoresoftware/manticoresearch/issues/1660) 修正了来自其他索引的 docid killlists 应用。❤️ 感谢 [@raxoft](https://github.com/raxoft) 的 PR。
* [Issue #1668](https://github.com/manticoresoftware/manticoresearch/issues/1668) 修正了因原始索引全扫描（无任何索引迭代器）提前退出导致的匹配遗漏；移除了纯行迭代器中的 cutoff。
* [Issue #1671](https://github.com/manticoresoftware/manticoresearch/issues/1671) 修正了使用 agent 和本地表查询分布式表时的 `FACET` 错误。
* [Issue #1690](https://github.com/manticoresoftware/manticoresearch/issues/1690) 修正了大值直方图估计时崩溃。
* [Issue #1692](https://github.com/manticoresoftware/manticoresearch/issues/1692) alter table tbl add column col uint 时崩溃。
* [Issue #1710](https://github.com/manticoresoftware/manticoresearch/issues/1710) 条件 `WHERE json.array IN (<value>)` 返回空结果。
* [Issue #172](https://github.com/manticoresoftware/manticoresearch-buddy/issues/172) 修正了向 `/cli` 发送请求时 TableFormatter 的问题。
* [Issue #1742](https://github.com/manticoresoftware/manticoresearch/issues/1742) 缺少 wordforms 文件时 `CREATE TABLE` 不会失败。
* [Issue #1762](https://github.com/manticoresoftware/manticoresearch/issues/1762) RT 表中属性的顺序现在遵循配置顺序。
* [Issue #1765](https://github.com/manticoresoftware/manticoresearch/issues/1765) 带 'should' 条件的 HTTP bool 查询返回错误结果。
* [Issue #1769](https://github.com/manticoresoftware/manticoresearch/issues/1769) 按字符串属性排序时，`SPH_SORT_ATTR_DESC` 和 `SPH_SORT_ATTR_ASC` 不起作用。
* [Issue #177](https://github.com/manticoresoftware/manticoresearch-buddy/issues/177) 禁用了 curl 请求到 Buddy 的 `Expect: 100-continue` HTTP 头。
* [Issue #1791](https://github.com/manticoresoftware/manticoresearch/issues/1791) 由 GROUP BY 别名引起的崩溃。
* [Issue #1792](https://github.com/manticoresoftware/manticoresearch/issues/1792) Windows 上 SQL 元摘要显示错误时间。
* [Issue #1794](https://github.com/manticoresoftware/manticoresearch/issues/1794) 修复了 JSON 查询中单词性能下降的问题。
* [Issue #1798](https://github.com/manticoresoftware/manticoresearch/issues/1798) 不兼容的过滤器在 `/search` 上未引发错误。
* [Issue #1802](https://github.com/manticoresoftware/manticoresearch/issues/1802) 修复了 `ALTER CLUSTER ADD` 和 `JOIN CLUSTER` 操作相互等待的问题，防止了 `ALTER` 在捐赠节点向加入节点发送表时将表添加到集群的竞态条件。
* [Issue #1811](https://github.com/manticoresoftware/manticoresearch/issues/1811) 错误处理了 `/pq/{table}/*` 请求。
* [Issue #1816](https://github.com/manticoresoftware/manticoresearch/issues/1816) `UNFREEZE` 在某些情况下不起作用。
* [Issue #183](https://github.com/manticoresoftware/manticoresearch-buddy/pull/183) 修复了某些情况下 MVA 恢复的问题。
* [Issue #1849](https://github.com/manticoresoftware/manticoresearch/issues/1849) 修复了在与 MCL 一起使用时 indextool 关闭时崩溃的问题。
* [Issue #1866](https://github.com/manticoresoftware/manticoresearch/issues/1866) 修复了 `/cli_json` 请求的不必要的 URL 解码。
* [Issue #1872](https://github.com/manticoresoftware/manticoresearch/issues/1872) 更改了守护进程启动时的 plugin_dir 设置逻辑。
* [Issue #1874](https://github.com/manticoresoftware/manticoresearch/issues/1874) alter table ... 异常失败。
* [Issue #1891](https://github.com/manticoresoftware/manticoresearch/issues/1891#issuecomment-2051941200) 插入数据时 Manticore 崩溃，信号为 `signal 11`。
* [Issue #1920](https://github.com/manticoresoftware/manticoresearch/issues/1920) 降低了 [low_priority](Searching/Options.md#low_priority) 的节流。
* [Issue #1924](https://github.com/manticoresoftware/manticoresearch/issues/1924) Mysqldump + mysql 恢复的错误。
* [Issue #1951](https://github.com/manticoresoftware/manticoresearch/issues/1951) 修复了在本地表缺失或代理描述不正确时分布式表创建错误的问题；现在返回错误信息。
* [Issue #1972](https://github.com/manticoresoftware/manticoresearch/issues/1972) 实现了 `FREEZE` 计数器以避免 freeze/unfreeze 问题。
* [Issue #1980](https://github.com/manticoresoftware/manticoresearch/issues/1980) 在 OR 节点中遵守查询超时。之前 `max_query_time` 在某些情况下可能不起作用。
* [Issue #1986](https://github.com/manticoresoftware/manticoresearch/issues/1986) 无法将 new 重命名为当前的 [manticore.json]。
* [Issue #1988](https://github.com/manticoresoftware/manticoresearch/issues/1988) 全文查询可能忽略 `SecondaryIndex` CBO 提示。
* [Issue #1990](https://github.com/manticoresoftware/manticoresearch/issues/1990) 修复了 `expansion_limit`，用于从多个磁盘块或内存块调用关键字时切片最终结果集。
* [Issue #1994](https://github.com/manticoresoftware/manticoresearch/issues/1994) 错误的外部文件。
* [Issue #2021](https://github.com/manticoresoftware/manticoresearch/issues/2021) 停止 Manticore 后可能会留下几个 manticore-executor 进程。
* [Issue #2029](https://github.com/manticoresoftware/manticoresearch/issues/2029) 使用 Levenshtein 距离时崩溃。
* [Issue #2037](https://github.com/manticoresoftware/manticoresearch/issues/2037) 在空索引上多次运行 max 操作符后出现错误。
* [Issue #2052](https://github.com/manticoresoftware/manticoresearch/issues/2052) 使用 JSON.field 的多组查询时崩溃。
* [Issue #2067](https://github.com/manticoresoftware/manticoresearch/issues/2067) 对 _update 的错误请求导致 Manticore 崩溃。
* [Issue #2069](https://github.com/manticoresoftware/manticoresearch/issues/2069) 修复了 JSON 接口中字符串过滤比较器对闭区间的处理问题。
* [Issue #2082](https://github.com/manticoresoftware/manticoresearch/issues/2082) 当 data_dir 路径位于符号链接上时，`alter` 失败。
* [Issue #2102](https://github.com/manticoresoftware/manticoresearch/pull/2102) 改进了 mysqldump 中 SELECT 查询的特殊处理，确保生成的 INSERT 语句与 Manticore 兼容。
* [Issue #2103](https://github.com/manticoresoftware/manticoresearch/issues/2103) 泰文字符使用了错误的字符集。
* [Issue #2124](https://github.com/manticoresoftware/manticoresearch/issues/2124) 使用包含保留字的 SQL 时崩溃。
* [Issue #2154](https://github.com/manticoresoftware/manticoresearch/issues/2154) 含有词形变化的表无法导入。
* [Issue #2176](https://github.com/manticoresoftware/manticoresearch/issues/2176) 修复了当引擎参数设置为 'columnar' 且通过 JSON 添加重复 ID 时发生的崩溃。
* [Issue #223](https://github.com/manticoresoftware/manticoresearch-buddy/issues/223) 尝试插入无模式且无列名的文档时返回正确错误。
* [Issue #239](https://github.com/manticoresoftware/manticoresearch-buddy/issues/239) 自动模式多行插入可能失败。
* [Issue #399](https://github.com/manticoresoftware/manticoresearch/issues/399) 如果数据源声明了 id 属性，索引时添加错误信息。
* [Issue #59](https://github.com/manticoresoftware/manticoresearch-helm/issues/59) Manticore 集群崩溃。
* [Issue #68](https://github.com/manticoresoftware/manticoresearch-helm/issues/68) 如果存在 percolate 表，optimize.php 会崩溃。
* [Issue #77](https://github.com/manticoresoftware/manticoresearch-helm/issues/77) 修复了在 Kubernetes 上部署时的错误。
* [Issue #274](https://github.com/manticoresoftware/manticoresearch-buddy/issues/274) 修复了对 Buddy 并发请求的错误处理。

### 与 manticore-backup 相关
* [Issue #97](https://github.com/manticoresoftware/manticoresearch-backup/pull/97) 当可用时，将 VIP HTTP 端口设置为默认。
各种改进：改进了版本检查和流式ZSTD解压缩；在恢复过程中添加了版本不匹配的用户提示；修正了恢复时不同版本的错误提示行为；增强了解压缩逻辑，直接从流中读取而不是进入工作内存；添加了`--force`标志
* [提交 3b35](https://github.com/manticoresoftware/manticoresearch-backup/commit/3b357ea) 在Manticore搜索启动后添加备份版本显示，以便在此阶段识别问题。
* [提交 ad2e](https://github.com/manticoresoftware/manticoresearch-backup/commit/ad2e5da) 更新了连接守护进程失败时的错误信息。
* [提交 ce5e](https://github.com/manticoresoftware/manticoresearch-backup/commit/ce5ea85) 修正了将绝对根备份路径转换为相对路径的问题，并移除了恢复时的可写检查，以支持从不同路径恢复。
* [提交 db7e](https://github.com/manticoresoftware/manticoresearch-backup/commit/db7e2b9) 为文件迭代器添加排序，确保各种情况下的一致性。
* [问题 #106](https://github.com/manticoresoftware/manticoresearch-backup/issues/105) 多配置的备份和恢复。
* [问题 #91](https://github.com/manticoresoftware/manticoresearch-backup/pull/91) 添加defattr以防止RHEL安装后文件出现异常用户权限。
* [问题 #91](https://github.com/manticoresoftware/manticoresearch-backup/pull/91) 添加额外的chown，确保Ubuntu中默认文件归属root用户。

### 与MCL（列式、二级、knn库）相关
* [提交 f104](https://github.com/manticoresoftware/columnar/commit/f104a4709bbb144140d09a09bc0fc1db1895e331) 支持向量搜索。
* [提交 2169](https://github.com/manticoresoftware/columnar/commit/216953beef6676f7e34c9f6f1a30c5980d746710) 修正了在中断的二级索引构建设置期间临时文件的清理问题。解决了守护进程在创建`tmp.spidx`文件时超出打开文件限制的问题。
* [提交 709b](https://github.com/manticoresoftware/columnar/commit/709b9acaaac97d9a1ca8796892f9ad432021c785) 为列式和SI使用独立的streamvbyte库。
* [提交 1c26](https://github.com/manticoresoftware/manticoresearch/commit/1c2623763ced14bb94ad624dde9614a938408d89) 添加警告，指出列式存储不支持json属性。
* [提交 3acd](https://github.com/manticoresoftware/columnar/commit/3acd6714ffce103447d7931dfc75bb1a7449f340) 修正了SI中的数据解包。
* [提交 574c](https://github.com/manticoresoftware/manticoresearch/commit/574c023152564fa72092c18ebd1f594abfd4b6d7) 修正了保存混合行式和列式存储的磁盘块时的崩溃。
* [提交 e87f](https://github.com/manticoresoftware/columnar/commit/e87f088b458d4c5b1b6159fc72ed0d57b786891a) 修正了SI迭代器被提示处理已处理块的问题。
* [问题 #1474](https://github.com/manticoresoftware/manticoresearch/issues/1474) 使用列式引擎时，行式MVA列的更新失败。
* [问题 #1510](https://github.com/manticoresoftware/manticoresearch/issues/1510) 修正了在`HAVING`中使用列式属性时的聚合崩溃。
* [问题 #1519](https://github.com/manticoresoftware/manticoresearch/issues/1519) 修正了在使用列式属性时`expr`排序器的崩溃。

### 与Docker相关
* ❗[问题 #42](https://github.com/manticoresoftware/docker/issues/42) 通过环境变量支持[纯索引](https://github.com/manticoresoftware/docker#building-plain-tables)。
* ❗[问题 #47](https://github.com/manticoresoftware/docker/issues/47) 通过环境变量改进配置的灵活性。
* [问题 #54](https://github.com/manticoresoftware/docker/pull/54) 改进了Docker的[备份和恢复](https://github.com/manticoresoftware/docker#backup-and-restore)流程。
* [问题 #77](https://github.com/manticoresoftware/docker/issues/76) 改进入口点，仅在首次启动时处理备份恢复。
* [提交 a27c](https://github.com/manticoresoftware/docker/commit/a27c048dd9e73d8d6bea8d9a4830cafb486b82c5) 修正查询日志输出到stdout。
* [问题 #38](https://github.com/manticoresoftware/docker/issues/38) 如果未设置EXTRA，静音BUDDY警告。
* [问题 #71](https://github.com/manticoresoftware/docker/pull/71) 修正`manticore.conf.sh`中的主机名。

## 版本 6.2.12
发布：2023年8月23日

版本6.2.12延续6.2系列，解决了6.2.0发布后发现的问题。

### Bug修复
* ❗[问题 #1351](https://github.com/manticoresoftware/manticoresearch/issues/1351) “Manticore 6.2.0在Centos 7上无法通过systemctl启动”：将`TimeoutStartSec`从`infinity`修改为`0`，以提高与Centos 7的兼容性。
* ❗[问题 #1364](https://github.com/manticoresoftware/manticoresearch/issues/1364) “从6.0.4升级到6.2.0后崩溃”：为旧版本binlog的空binlog文件添加重放功能。
* [PR #1334](https://github.com/manticoresoftware/manticoresearch/pull/1334) “修正searchdreplication.cpp中的拼写错误”：修正`searchdreplication.cpp`中的拼写错误：beggining -> beginning。
* [问题 #1337](https://github.com/manticoresoftware/manticoresearch/issues/1337) “Manticore 6.2.0 WARNING: conn (local)(12), sock=8088: bailing on failed MySQL header, AsyncNetInputBuffer_c::AppendData: error 11 (Resource temporarily unavailable) return -1”：降低MySQL接口关于头部的警告日志级别为logdebugv。
* [问题 #1355](https://github.com/manticoresoftware/manticoresearch/issues/1355) “当node_address无法解析时，join cluster挂起”：改进了当某些节点不可达且名称解析失败时的复制重试。此改进应解决Kubernetes和Docker节点中与复制相关的问题。增强了复制启动失败的错误信息，并更新了测试模型376。此外，提供了名称解析失败的明确错误信息。
* [Issue #1361](https://github.com/manticoresoftware/manticoresearch/issues/1361) "No lower case mapping for "Ø" in charset non_cjk": 调整了字符 'Ø' 的映射。
* [Issue #1365](https://github.com/manticoresoftware/manticoresearch/issues/1365) "searchd leaves binlog.meta and binlog.001 after clean stop": 确保最后一个空的 binlog 文件被正确删除。
* [Commit 0871](https://github.com/manticoresoftware/manticoresearch/commit/0871070845e93ad702ae0dfb89b35e5f24cd0851): 修复了 Windows 上与原子复制限制相关的 `Thd_t` 构建问题。
* [Commit 1cc0](https://github.com/manticoresoftware/manticoresearch/commit/1cc04bedc174737118fd5a3663e35f4e95bd4f8c): 解决了 FT CBO 与 `ColumnarScan` 相关的问题。
* [Commit c6bf](https://github.com/manticoresoftware/manticoresearch/commit/c6bfb30d53e3f4b67f17769667733677a661c95b): 修正了测试 376 并为测试中的 `AF_INET` 错误添加了替代方案。
* [Commit cbc3](https://github.com/manticoresoftware/manticoresearch/commit/cbc38705e0b4dcc9665e3e8cf64826290ebd11cf): 解决了复制过程中更新 blob 属性与替换文档时的死锁问题。同时移除了提交期间索引的 rlock，因为它已经在更基础的层面被锁定。

### 小改动
* [Commit 4f91](https://github.com/manticoresoftware/manticoresearch/commit/4f913a36e84e01f950b2797051222b0b1094d6fe) 更新了手册中关于 `/bulk` 端点的信息。

### MCL
* 支持 [Manticore Columnar Library](https://github.com/manticoresoftware/columnar) v2.2.4

## 版本 6.2.0
发布：2023年8月4日

### 主要改动
* 查询优化器增强，支持全文查询，显著提升搜索效率和性能。
* 集成：
  - [mysqldump](https://dev.mysql.com/doc/refman/8.0/en/mysqldump.html) - 使用 `mysqldump` 进行[逻辑备份](Securing_and_compacting_a_table/Backup_and_restore.md#Backup-and-restore-with-mysqldump)
  - [Apache Superset](https://manticoresearch.com/blog/manticoresearch-apache-superset-integration/) 和 [Grafana](https://manticoresearch.com/blog/manticoresearch-grafana-integration/) 用于可视化存储在 Manticore 中的数据
  - [HeidiSQL](https://www.heidisql.com/) 和 [DBForge](https://www.devart.com/dbforge/) 以便更轻松地进行 Manticore 开发
* 我们开始使用 [GitHub workflows](https://github.com/manticoresoftware/manticoresearch/actions)，使贡献者更容易使用核心团队在准备软件包时采用的相同持续集成（CI）流程。所有作业都可以在 GitHub 托管的运行器上运行，方便在您的 Manticore Search 分支中无缝测试更改。
* 我们开始使用 [CLT](https://github.com/manticoresoftware/clt) 来测试复杂场景。例如，我们现在能够确保提交后构建的软件包可以在所有支持的 Linux 操作系统上正确安装。命令行测试器（CLT）提供了一种用户友好的方式，以交互模式录制测试并轻松重放。
* 通过结合使用哈希表和 HyperLogLog，显著提升了 count distinct 操作的性能。
* 启用了包含二级索引的查询的多线程执行，线程数限制为物理 CPU 核心数。这将大幅提升查询执行速度。
* `pseudo_sharding` 已调整为限制在空闲线程数内。此更新显著提升了吞吐性能。
* 用户现在可以通过配置设置指定[默认属性存储引擎](Server_settings/Searchd.md#engine)，更好地定制以匹配特定工作负载需求。
* 支持 [Manticore Columnar Library 2.2.0](https://github.com/manticoresoftware/columnar/)，在[二级索引](Server_settings/Searchd.md#secondary_indexes)中修复了众多错误并进行了改进。

### 小改动
* [Buddy #153](https://github.com/manticoresoftware/manticoresearch-buddy/issues/153): [/pq](Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md?client=JSON#Adding-rules-to-a-percolate-table) HTTP 端点现在作为 `/json/pq` HTTP 端点的别名。
* [Commit 0bf1](https://github.com/manticoresoftware/manticoresearch/commit/0bf17d9e): 确保了 `upper()` 和 `lower()` 的多字节兼容性。
* [Commit 2bb9](https://github.com/manticoresoftware/manticoresearch/commit/2bb92765): 对于 `count(*)` 查询，不再扫描索引，而是返回预计算值。
* [Commit 3c84](https://github.com/manticoresoftware/manticoresearch/commit/3c84091f): 现在可以使用 `SELECT` 进行任意计算和显示 `@@sysvars`。与之前不同，您不再局限于单个计算。因此，像 `select user(), database(), @@version_comment, version(), 1+1 as a limit 10` 这样的查询将返回所有列。注意，可选的 'limit' 总是被忽略。
* [Commit 6aca](https://github.com/manticoresoftware/manticoresearch/commit/6aca32f0): 实现了 `CREATE DATABASE` 存根查询。
* [Commit 9dc1](https://github.com/manticoresoftware/manticoresearch/commit/9dc12334): 执行 `ALTER TABLE table REBUILD SECONDARY` 时，即使属性未更新，二级索引也总是被重建。
* [Commit 46ed](https://github.com/manticoresoftware/manticoresearch/commit/46edb089): 使用预计算数据的排序器现在在使用 CBO 之前被识别，以避免不必要的 CBO 计算。
* [Commit 102a](https://github.com/manticoresoftware/manticoresearch/commit/102ac604): 实现了全文表达式栈的模拟和使用，以防止守护进程崩溃。
* [Commit 979f](https://github.com/manticoresoftware/manticoresearch/commit/979fa27c): 为不使用字符串/mvas/json 属性的匹配添加了快速路径的匹配克隆代码。
* [提交 a073](https://github.com/manticoresoftware/manticoresearch/commit/a0735ffe)：添加了对 `SELECT DATABASE()` 命令的支持。但它始终返回 `Manticore`。此新增功能对于与各种 MySQL 工具的集成至关重要。
* [提交 bc04](https://github.com/manticoresoftware/manticoresearch/commit/bc04908)：修改了 [/cli](Connecting_to_the_server/HTTP.md#/cli) 端点的响应格式，并添加了 `/cli_json` 端点以实现之前 `/cli` 的功能。
* [提交 d70b](https://github.com/manticoresoftware/manticoresearch/commit/d70b0d58)：`thread_stack` 现在可以在运行时通过 `SET` 语句修改。提供了会话本地和守护进程范围的两种变体。当前值可在 `show variables` 输出中查看。
* [提交 d96e](https://github.com/manticoresoftware/manticoresearch/commit/d96ec6b9)：代码已集成到 CBO 中，以更准确地估计在字符串属性上执行过滤器的复杂度。
* [提交 e77d](https://github.com/manticoresoftware/manticoresearch/commit/e77dd72f5a04531c352fad0d7afcd2a1cbae2510)：改进了 DocidIndex 成本计算，提升了整体性能。
* [提交 f3ae](https://github.com/manticoresoftware/manticoresearch/commit/f3ae8bea)：类似于 Linux 上的“uptime”，负载指标现在可在 `SHOW STATUS` 命令中查看。
* [提交 f3cc](https://github.com/manticoresoftware/manticoresearch/commit/f3cc0971)：`DESC` 和 `SHOW CREATE TABLE` 的字段和属性顺序现在与 `SELECT * FROM` 保持一致。
* [提交 f3d2](https://github.com/manticoresoftware/manticoresearch/commit/f3d248a6)：不同的内部解析器现在在各种错误中提供其内部助记代码（例如 `P01`）。此改进有助于识别导致错误的解析器，同时隐藏非必要的内部细节。
* [问题 #271](https://github.com/manticoresoftware/manticoresearch/issues/271) “有时 CALL SUGGEST 不会建议单字母拼写错误的更正”：改进了 [SUGGEST/QSUGGEST](Searching/Spell_correction.md#CALL-QSUGGEST,-CALL-SUGGEST) 对短词的行为：添加了 `sentence` 选项以显示整句。
* [问题 #696](https://github.com/manticoresoftware/manticoresearch/issues/696) “启用词干处理时，Percolate 索引无法通过精确短语查询正确搜索”：修改了 percolate 查询以处理精确词项修饰符，提升了搜索功能。
* [问题 #829](https://github.com/manticoresoftware/manticoresearch/issues/829) “日期格式化方法”：添加了 [date_format()](Functions/Date_and_time_functions.md#DATE_FORMAT()) 选择列表表达式，暴露了 `strftime()` 函数。
* [问题 #961](https://github.com/manticoresoftware/manticoresearch/issues/961) “通过 HTTP JSON API 对桶进行排序”：在 HTTP 接口中为每个聚合桶引入了可选的 [sort 属性](Searching/Faceted_search.md#HTTP-JSON)。
* [问题 #1062](https://github.com/manticoresoftware/manticoresearch/issues/1062) “改进 JSON 插入 API 失败的错误日志 - “不支持的值类型””：当发生错误时，`/bulk` 端点会报告已处理和未处理字符串（文档）的数量信息。
* [问题 #1070](https://github.com/manticoresoftware/manticoresearch/issues/1070) “CBO 提示不支持多个属性”：启用了索引提示以处理多个属性。
* [问题 #1106](https://github.com/manticoresoftware/manticoresearch/issues/1106) “向 HTTP 搜索查询添加标签”：已向 [HTTP PQ 响应](Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md?client=JSON#Adding-rules-to-a-percolate-table) 添加标签。
* [问题 #1301](https://github.com/manticoresoftware/manticoresearch/issues/1301) “buddy 不应并行创建表”：解决了导致并行 CREATE TABLE 操作失败的问题。现在一次只能运行一个 `CREATE TABLE` 操作。
* [问题 #1303](https://github.com/manticoresoftware/manticoresearch/issues/1303) “添加对 @ 到列名的支持”。
* [问题 #1316](https://github.com/manticoresoftware/manticoresearch/issues/1316) “使用 ps=1 时出租车数据集查询缓慢”：优化了 CBO 逻辑，默认直方图分辨率设置为 8k，以提高对随机分布值属性的准确性。
* [问题 #1317](https://github.com/manticoresoftware/manticoresearch/issues/1317) “修复 hn 数据集上的 CBO 与全文搜索”：实现了改进的逻辑，用于确定何时使用位图迭代器交集，何时使用优先队列。
* [问题 #1318](https://github.com/manticoresoftware/manticoresearch/issues/1318) “columnar：更改迭代器接口为单次调用”：列式迭代器现在使用单次 `Get` 调用，替代之前的两步 `AdvanceTo` + `Get` 调用以获取值。
* [问题 #1319](https://github.com/manticoresoftware/manticoresearch/issues/1319) “聚合计算加速（移除 CheckReplaceEntry？）”：从组排序器中移除了 `CheckReplaceEntry` 调用，以加快聚合函数的计算。
* [问题 #1320](https://github.com/manticoresoftware/manticoresearch/issues/1320) “创建表时 read_buffer_docs/hits 不支持 k/m/g 语法”：`CREATE TABLE` 选项 `read_buffer_docs` 和 `read_buffer_hits` 现在支持 k/m/g 语法。
* 英语、德语和俄语的[语言包](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology)现在可通过在 Linux 上执行命令 `apt/yum install manticore-language-packs` 轻松安装。在 macOS 上，使用命令 `brew install manticoresoftware/tap/manticore-language-packs`。
* 字段和属性顺序现在在 `SHOW CREATE TABLE` 和 `DESC` 操作之间保持一致。
* 如果执行 `INSERT` 查询时磁盘空间不足，新 `INSERT` 查询将失败，直到有足够的磁盘空间可用。
* 添加了 [UINT64()](Functions/Type_casting_functions.md#UINT64%28%29) 类型转换函数。
* `/bulk` 端点现在将空行处理为一个 [commit](Data_creation_and_modification/Transactions.md#BEGIN,-COMMIT,-and-ROLLBACK) 命令。更多信息见 [这里](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md?client=JSON#Bulk-adding-documents)。
* 已为 [无效的索引提示](Searching/Options.md#Query-optimizer-hints) 实现了警告，提供了更多透明度并允许错误缓解。
* 当 `count(*)` 与单个过滤器一起使用时，查询现在在可用时利用来自二级索引的预计算数据，大大加快了查询时间。

### ⚠️ 重大变更
* ⚠️ 在版本 6.2.0 中创建或修改的表无法被旧版本读取。
* ⚠️ 文档 ID 现在在索引和 INSERT 操作中作为无符号 64 位整数处理。
* ⚠️ 查询优化器提示的语法已更新。新格式为 `/*+ SecondaryIndex(uid) */`。请注意，旧语法不再支持。
* ⚠️ [问题 #1160](https://github.com/manticoresoftware/manticoresearch/issues/1160)：为了防止语法冲突，禁止在表名中使用 `@`。
* ⚠️ 标记为 `indexed` 和 `attribute` 的字符串字段/属性现在在 `INSERT`、`DESC` 和 `ALTER` 操作中被视为单个字段。
* ⚠️ [问题 #1057](https://github.com/manticoresoftware/manticoresearch/issues/1057)：不支持 SSE 4.2 的系统将不再加载 MCL 库。
* ⚠️ [问题 #1143](https://github.com/manticoresoftware/manticoresearch/issues/1143)：[agent_query_timeout](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_query_timeout) 出现故障。已修复并生效。

### Bug 修复
* [提交 2a6e](https://github.com/manticoresoftware/manticoresearch/commit/2a6ea8f7) “DROP TABLE 时崩溃”：解决了在执行 DROP TABLE 语句时，RT 表写操作（优化、磁盘块保存）完成时间过长的问题。添加了警告，当执行 DROP TABLE 命令后表目录非空时进行通知。
* [提交 2ebd](https://github.com/manticoresoftware/manticoresearch/commit/2ebd424d)：添加了对列式属性的支持，此前在用于多属性分组的代码中缺失。
* [提交 3be4](https://github.com/manticoresoftware/manticoresearch/commit/3be4503f752216b37b8cde8e47fd2d77bbce63ae) 解决了因磁盘空间耗尽可能导致的崩溃问题，通过正确处理 binlog 中的写入错误。
* [提交 6adb](https://github.com/manticoresoftware/manticoresearch/commit/6adb0934)：修复了在查询中使用多个列式扫描迭代器（或二级索引迭代器）时偶发的崩溃。
* [提交 6bd9](https://github.com/manticoresoftware/manticoresearch/commit/6bd9f709)：使用预计算数据的排序器时，过滤器未被移除的问题已修复。
* [提交 6d03](https://github.com/manticoresoftware/manticoresearch/commit/6d03566a)：更新了 CBO 代码，以更好地估计多线程执行的基于行属性过滤的查询。
* [提交 6dd3](https://github.com/manticoresoftware/manticoresearch/commit/6dd3964), [Helm #56](https://github.com/manticoresoftware/manticoresearch-helm/issues/56) “Kubernetes 集群中的致命崩溃转储”：修复了 JSON 根对象的布隆过滤器缺陷；修复了因按 JSON 字段过滤导致的守护进程崩溃。
* [提交 6e1b](https://github.com/manticoresoftware/manticoresearch/commit/6e1b849) 修正了因无效的 `manticore.json` 配置导致的守护进程崩溃。
* [提交 6fbc](https://github.com/manticoresoftware/manticoresearch/commit/6fbc4189) 修复了 json 范围过滤器以支持 int64 值。
* [提交 9c67](https://github.com/manticoresoftware/manticoresearch/commit/9c677426) 修复了 `.sph` 文件在 `ALTER` 时可能损坏的问题。
* [提交 77cc](https://github.com/manticoresoftware/manticoresearch/commit/77ccd8f)：为替换语句的复制添加了共享密钥，解决了从多个主节点复制替换时出现的 `pre_commit` 错误。
* [提交 2884](https://github.com/manticoresoftware/manticoresearch/commit/2884519e) 解决了对 'date_format()' 等函数进行 bigint 检查的问题。
* [提交 9513](https://github.com/manticoresoftware/manticoresearch/commit/9513aca5)：当排序器使用预计算数据时，迭代器不再显示在 [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META) 中。
* [提交 a2a7](https://github.com/manticoresoftware/manticoresearch/commit/a2a70555)：更新了全文节点堆栈大小，防止复杂全文查询时崩溃。
* [提交 a062](https://github.com/manticoresoftware/manticoresearch/commit/a0626d7e)：修复了在复制带有 JSON 和字符串属性的更新时导致崩溃的错误。
* [提交 b3e6](https://github.com/manticoresoftware/manticoresearch/commit/b3e63e5d)：更新了字符串构建器，使用 64 位整数以避免处理大数据集时崩溃。
* [提交 c472](https://github.com/manticoresoftware/manticoresearch/commit/c472e5b)：解决了跨多个索引进行 count distinct 时的崩溃问题。
* [提交 d073](https://github.com/manticoresoftware/manticoresearch/commit/d0730272)：修复了即使禁用 `pseudo_sharding`，对 RT 索引的磁盘块查询仍可多线程执行的问题。
* [提交 d205](https://github.com/manticoresoftware/manticoresearch/commit/d205508e) 修改了 `show index status` 命令返回的值集合，现在根据所用索引类型而异。
* [提交 e9bc](https://github.com/manticoresoftware/manticoresearch/commit/e9bc2f9) 修复了处理批量请求时的 HTTP 错误及错误未从网络循环返回给客户端的问题。
* [提交 f77c](https://github.com/manticoresoftware/manticoresearch/commit/f77ce0e6) PQ 使用了扩展堆栈。
* [提交 fac2](https://github.com/manticoresoftware/manticoresearch/commit/fac2d9687a20582765aaa89be9597804c4f845af) 更新了导出排序器输出以与 [packedfactors()](Functions/Searching_and_ranking_functions.md#PACKEDFACTORS%28%29) 对齐。
* [提交 ff87](https://github.com/manticoresoftware/manticoresearch/commit/ff87cc4): 修复了 SphinxQL 查询日志中过滤器中字符串列表的问题。
* [问题 #589](https://github.com/manticoresoftware/manticoresearch/issues/589) “字符集定义似乎依赖于代码的排序顺序”：修正了重复项的错误字符集映射。
* [问题 #811](https://github.com/manticoresoftware/manticoresearch/issues/811) “词形变化中映射多个词干干扰带有 CJK 标点符号的短语搜索”：修复了带有词形变化的短语查询中 ngram 令牌位置的问题。
* [问题 #834](https://github.com/manticoresoftware/manticoresearch/issues/834) “搜索查询中的等号导致请求失败”：确保精确符号可以被转义，并修复了 `expand_keywords` 选项导致的双重精确扩展。
* [问题 #864](https://github.com/manticoresoftware/manticoresearch/issues/864) “exceptions/stopwords 冲突”
* [问题 #910](https://github.com/manticoresoftware/manticoresearch/issues/910) “调用 call snippets() 时使用 libstemmer_fr 和 index_exact_words 导致 Manticore 崩溃”：解决了调用 `SNIPPETS()` 时导致崩溃的内部冲突。
* [问题 #946](https://github.com/manticoresoftware/manticoresearch/issues/946) “SELECT 期间出现重复记录”：修复了带有 `not_terms_only_allowed` 选项且包含已删除文档的 RT 索引查询结果中出现重复文档的问题。
* [问题 #967](https://github.com/manticoresoftware/manticoresearch/issues/967) “在 UDF 函数中使用 JSON 参数导致崩溃”：修复了启用伪分片和带 JSON 参数的 UDF 搜索时守护进程崩溃的问题。
* [问题 #1050](https://github.com/manticoresoftware/manticoresearch/issues/1050) “FEDERATED 中的 count(*)”：修复了通过 `FEDERATED` 引擎带聚合的查询导致守护进程崩溃的问题。
* [问题 #1052](https://github.com/manticoresoftware/manticoresearch/issues/1052) 修复了 `rt_attr_json` 列与列存储不兼容的问题。
* [问题 #1072](https://github.com/manticoresoftware/manticoresearch/issues/1072) “* 被 ignore_chars 从搜索查询中移除”：修复了查询中的通配符不受 `ignore_chars` 影响的问题。
* [问题 #1075](https://github.com/manticoresoftware/manticoresearch/issues/1075) “indextool --check 在存在分布式表时失败”：indextool 现兼容 json 配置中包含 'distributed' 和 'template' 索引的实例。
* [问题 #1081](https://github.com/manticoresoftware/manticoresearch/issues/1081) “特定 RT 数据集上的特定 select 导致 searchd 崩溃”：解决了带有 packedfactors 和大内部缓冲区的查询导致守护进程崩溃的问题。
* [问题 #1095](https://github.com/manticoresoftware/manticoresearch/issues/1095) “使用 not_terms_only_allowed 时已删除文档被忽略”
* [问题 #1099](https://github.com/manticoresoftware/manticoresearch/issues/1099) “indextool --dumpdocids 不工作”：恢复了 `--dumpdocids` 命令的功能。
* [问题 #1100](https://github.com/manticoresoftware/manticoresearch/issues/1100) “indextool --buildidf 不工作”：indextool 现在在完成 globalidf 后关闭文件。
* [问题 #1104](https://github.com/manticoresoftware/manticoresearch/issues/1104) “Count(*) 试图被当作远程表中的 schema set 处理”：解决了当代理返回空结果集时，守护进程对分布式索引查询发送错误消息的问题。
* [问题 #1109](https://github.com/manticoresoftware/manticoresearch/issues/1109) “FLUSH ATTRIBUTES 在 threads=1 时挂起”。
* [问题 #1126](https://github.com/manticoresoftware/manticoresearch/issues/1126) “查询期间与 MySQL 服务器断开连接 - manticore 6.0.5”：解决了使用多个列存储属性过滤器时发生的崩溃。
* [问题 #1135](https://github.com/manticoresoftware/manticoresearch/issues/1135) “JSON 字符串过滤的大小写敏感性”：修正了 HTTP 搜索请求中过滤器的排序规则以正确工作。
* [问题 #1140](https://github.com/manticoresoftware/manticoresearch/issues/1140) “错误字段中的匹配”：修复了与 `morphology_skip_fields` 相关的问题。
* [问题 #1155](https://github.com/manticoresoftware/manticoresearch/issues/1155) “通过 API 的系统远程命令应传递 g_iMaxPacketSize”：更新以绕过节点间复制命令的 `max_packet_size` 检查。此外，最新的集群错误已添加到状态显示中。
* [问题 #1302](https://github.com/manticoresoftware/manticoresearch/issues/1302) “优化失败时遗留临时文件”：修正了合并或优化过程中出错后遗留临时文件的问题。
* [问题 #1304](https://github.com/manticoresoftware/manticoresearch/issues/1304) “为 buddy 启动超时添加环境变量”：添加了环境变量 `MANTICORE_BUDDY_TIMEOUT`（默认 3 秒）以控制守护进程启动时等待 buddy 消息的时长。
* [问题 #1305](https://github.com/manticoresoftware/manticoresearch/issues/1305) “保存 PQ 元数据时整数溢出”：缓解了守护进程在保存大型 PQ 索引时的过度内存消耗。
* [问题 #1306](https://github.com/manticoresoftware/manticoresearch/issues/1306) “修改外部文件后无法重新创建 RT 表”：修正了外部文件为空字符串时的 alter 错误；修复了修改外部文件后 RT 索引外部文件未清理的问题。
* [问题 #1307](https://github.com/manticoresoftware/manticoresearch/issues/1307) “SELECT 语句 sum(value) as value 无法正常工作”：修复了带别名的选择列表表达式可能隐藏索引属性的问题；还修正了整数类型的 sum 转换为 int64 计数的问题。
* [Issue #1308](https://github.com/manticoresoftware/manticoresearch/issues/1308) “避免在复制中绑定到localhost”：确保复制不会绑定到具有多个IP的主机名的localhost。
* [Issue #1309](https://github.com/manticoresoftware/manticoresearch/issues/1309) “回复mysql客户端失败，数据大于16Mb”：修复了向客户端返回大于16Mb的SphinxQL数据包的问题。
* [Issue #1310](https://github.com/manticoresoftware/manticoresearch/issues/1310) “‘指向外部文件的路径应为绝对路径’中的错误引用”：修正了`SHOW CREATE TABLE`中外部文件完整路径的显示。
* [Issue #1311](https://github.com/manticoresoftware/manticoresearch/issues/1311) “调试构建在片段中长字符串时崩溃”：现在，`SNIPPET()`函数目标文本中允许长字符串（>255字符）。
* [Issue #1312](https://github.com/manticoresoftware/manticoresearch/issues/1312) “kqueue轮询中使用后删除导致的虚假崩溃（master-agent）”：修复了在kqueue驱动系统（FreeBSD、MacOS等）上主节点无法连接代理时的崩溃。
* [Issue #1313](https://github.com/manticoresoftware/manticoresearch/issues/1313) “连接自身时间过长”：在MacOS/BSD上从主节点连接代理时，现在使用统一的连接+查询超时，而不仅仅是连接超时。
* [Issue #1314](https://github.com/manticoresoftware/manticoresearch/issues/1314) “pq（json元数据）中未达到的嵌入同义词加载失败”：修复了pq中的嵌入同义词标志。
* [Issue #1315](https://github.com/manticoresoftware/manticoresearch/issues/1315) “允许某些函数（sint、fibonacci、second、minute、hour、day、month、year、yearmonth、yearmonthday）使用隐式提升的参数值”。
* [Issue #1321](https://github.com/manticoresoftware/manticoresearch/issues/1321) “在fullscan中启用多线程SI，但限制线程数”：已在CBO中实现代码，以更好地预测在全文查询中使用二级索引时的多线程性能。
* [Issue #1322](https://github.com/manticoresoftware/manticoresearch/issues/1322) “使用预计算排序器后count(*)查询仍然缓慢”：使用预计算数据的排序器时不再初始化迭代器，避免了性能下降。
* [Issue #1411](https://github.com/manticoresoftware/manticoresearch/issues/1141) “sphinxql中的查询日志不保留MVA的原始查询”：现在，`all()/any()`被记录。

## 版本 6.0.4
发布：2023年3月15日

### 新功能
* 改进了与Logstash、Beats等的集成，包括：
  - 支持Logstash版本7.6 - 7.15，Filebeat版本7.7 - 7.12
  - 自动模式支持。
  - 添加了对类似Elasticsearch格式的批量请求的处理。
* [Buddy提交 ce90](https://github.com/manticoresoftware/manticoresearch-buddy/commit/ce907ea) Manticore启动时记录Buddy版本。

### Bug修复
* [Issue #588](https://github.com/manticoresoftware/manticoresearch/issues/588), [Issue #942](https://github.com/manticoresoftware/manticoresearch/issues/942) 修复了bigram索引中搜索元数据和调用关键词的错误字符。
* [Issue #1027](https://github.com/manticoresoftware/manticoresearch/issues/1027) 拒绝小写HTTP头。
* ❗[Issue #1039](https://github.com/manticoresoftware/manticoresearch/issues/1039) 修复了守护进程在读取Buddy控制台输出时的内存泄漏。
* [Issue #1056](https://github.com/manticoresoftware/manticoresearch/issues/1056) 修复了问号的异常行为。
* [Issue #1064](https://github.com/manticoresoftware/manticoresearch/issues/1064) - 修复了分词器小写表中的竞态条件导致的崩溃。
* [Commit 59bb](https://github.com/manticoresoftware/manticoresearch/commit/59bb54c) 修复了JSON接口中对id显式设置为null的文档的批量写入处理。
* [Commit 7b6b](https://github.com/manticoresoftware/manticoresearch/commit/7b6b25f) 修复了CALL KEYWORDS中多个相同词项的词项统计。
* [Commit f381](https://github.com/manticoresoftware/manticoresearch/commit/f381ad2) Windows安装程序现在创建默认配置；运行时不再替换路径。
* [Commit 6940](https://github.com/manticoresoftware/manticoresearch/commit/6940e95), [Commit cc5a](https://github.com/manticoresoftware/manticoresearch/commit/cc5a480) 修复了多网络节点集群的复制问题。
* [Commit 4972](https://github.com/manticoresoftware/manticoresearch/commit/49722ab) 修复了`/pq` HTTP端点为`/json/pq` HTTP端点的别名。
* [Commit 3b53](https://github.com/manticoresoftware/manticoresearch/commit/3b5385a) 修复了Buddy重启时守护进程崩溃。
* [Buddy提交 fba9](https://github.com/manticoresoftware/manticoresearch-buddy/commit/fba9c8c) 显示无效请求接收时的原始错误。
* [Buddy提交 db95](https://github.com/manticoresoftware/manticoresearch-buddy/commit/db9532c) 允许备份路径中有空格，并对正则表达式做了一些魔法以支持单引号。

## 版本 6.0.2
发布：2023年2月10日

### Bug修复
* [Issue #1024 crash 2](https://github.com/manticoresoftware/manticoresearch/issues/1024) Facet搜索结果较多时崩溃/段错误
* ❗[Issue #1029](https://github.com/manticoresoftware/manticoresearch/issues/1029) - 警告：编译时值KNOWN_CREATE_SIZE（16）小于测量值（208）。建议修正该值！
* ❗[Issue #1032](https://github.com/manticoresoftware/manticoresearch/issues/1032) - Manticore 6.0.0纯索引崩溃
* ❗[Issue #1033](https://github.com/manticoresoftware/manticoresearch/issues/1033) - 守护进程重启时多个分布式丢失
* ❗[Issue #1064](https://github.com/manticoresoftware/manticoresearch/issues/1064) - 分词器小写表中的竞态条件

## 版本 6.0.0
发布：2023年2月7日

从本版本开始，Manticore Search 附带了 Manticore Buddy，这是一个用 PHP 编写的 sidecar 守护进程，负责处理不需要超低延迟或高吞吐量的高级功能。Manticore Buddy 在后台运行，您甚至可能不会意识到它正在运行。尽管对最终用户来说是不可见的，但使 Manticore Buddy 易于安装并与基于 C++ 的主守护进程兼容是一个重大挑战。这个重大变更将允许团队开发各种新的高级功能，例如分片编排、访问控制和身份验证，以及各种集成，如 mysqldump、DBeaver、Grafana mysql 连接器。目前它已经处理了 [SHOW QUERIES](Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES)、[BACKUP](Securing_and_compacting_a_table/Backup_and_restore.md#BACKUP-SQL-command-reference) 和 [自动模式](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema)。

本版本还包括 130 多个错误修复和众多功能，其中许多可以视为重大更新。

### 重大变更
* 🔬 实验性功能：您现在可以执行与 Elasticsearch 兼容的 [insert](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md?example=Elasticsearch#Adding-documents-to-a-real-time-table) 和 [replace](Data_creation_and_modification/Updating_documents/REPLACE.md?example=Elasticsearch#REPLACE) JSON 查询，这使得可以使用 Manticore 与 Logstash（版本 < 7.13）、Filebeat 以及 Beats 家族的其他工具配合使用。默认启用。您可以使用 `SET GLOBAL ES_COMPAT=off` 来禁用它。
* 支持 [Manticore Columnar Library 2.0.0](https://github.com/manticoresoftware/columnar/) ，在 [二级索引](Server_settings/Searchd.md#secondary_indexes) 中进行了大量修复和改进。**⚠️ 重大变更**：从本版本开始，二级索引默认开启。如果您是从 Manticore 5 升级，请务必执行 [ALTER TABLE table_name REBUILD SECONDARY](Updating_table_schema_and_settings.md#Rebuilding-a-secondary-index)。详情见下文。
* [提交 c436](https://github.com/manticoresoftware/manticoresearch/commit/c436f9023536f767610451911955ae36d90aa638) 自动模式：您现在可以跳过创建表的步骤，只需插入第一条文档，Manticore 将根据其字段自动创建表。详细信息请参见 [这里](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema)。您可以通过 [searchd.auto_schema](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema) 开启或关闭此功能。
* 对 [基于成本的优化器](Searching/Cost_based_optimizer.md) 进行了大幅改进，在许多情况下降低了查询响应时间。
  - [问题 #1008](https://github.com/manticoresoftware/manticoresearch/issues/1008) CBO 中的并行性能估计。
  - [问题 #1014](https://github.com/manticoresoftware/manticoresearch/issues/1014) CBO 现在支持识别 [二级索引](Server_settings/Searchd.md#secondary_indexes)，并能更智能地工作。
  - [提交 cef9](https://github.com/manticoresoftware/columnar/commit/cef9b54dc3a4ee9138ebc4df3a671715b996d6b3) 列式表/字段的编码统计信息现在存储在元数据中，帮助 CBO 做出更智能的决策。
  - [提交 2b95](https://github.com/manticoresoftware/manticoresearch/commit/2b95fb8c2ba422c9c5a66e2b61c1f7922f31c7aa) 添加了 CBO 提示，用于微调其行为。
* [遥测](Telemetry.md#Telemetry)：我们很高兴宣布本版本新增了遥测功能。该功能允许我们收集匿名且去标识化的指标，帮助我们提升产品的性能和用户体验。请放心，所有收集的数据都是**完全匿名的，不会与任何个人信息关联**。如果需要，可以在设置中[轻松关闭](Telemetry.md#Telemetry)此功能。
* [提交 5aaf](https://github.com/manticoresoftware/manticoresearch/commit/5aafb17d81b601e399e212a344a4738f65576cdd) [ALTER TABLE table_name REBUILD SECONDARY](Updating_table_schema_and_settings.md#Rebuilding-a-secondary-index) 命令，允许您随时重建二级索引，例如：
  - 从 Manticore 5 迁移到新版本时，
  - 对索引中的属性执行了 [UPDATE](Data_creation_and_modification/Updating_documents/UPDATE.md#UPDATE)（即[原地更新，而非替换](Data_creation_and_modification/Updating_documents/REPLACE_vs_UPDATE.md)）时
* [问题 #821](https://github.com/manticoresoftware/manticoresearch/issues/821) 新工具 `manticore-backup` 用于[备份和恢复 Manticore 实例](Securing_and_compacting_a_table/Backup_and_restore.md)
* SQL 命令 [BACKUP](Securing_and_compacting_a_table/Backup_and_restore.md#BACKUP-SQL-command-reference) 用于在 Manticore 内部执行备份。
* SQL 命令 [SHOW QUERIES](Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES) 作为查看正在运行查询的简便方式，替代查看线程。
* [问题 #551](https://github.com/manticoresoftware/manticoresearch/issues/551) SQL 命令 `KILL` 用于终止长时间运行的 `SELECT`。
* 聚合查询的动态 `max_matches`，以提高准确性并降低响应时间。

### 次要变更
* [问题 #822](https://github.com/manticoresoftware/manticoresearch/issues/822) SQL 命令 [FREEZE/UNFREEZE](Securing_and_compacting_a_table/Freezing_and_locking_a_table.md) 用于准备实时/普通表进行备份。
* [提交 c470](https://github.com/manticoresoftware/manticoresearch/commit/c47052f18ad4868134de1a97e8d7da570e531dfa) 新设置 `accurate_aggregation` 和 `max_matches_increase_threshold` 用于控制聚合准确性。
* [问题 #718](https://github.com/manticoresoftware/manticoresearch/issues/718) 支持有符号的负 64 位 ID。注意，您仍然不能使用大于 2^63 的 ID，但现在可以使用范围从 -2^63 到 0 的 ID。
* 由于我们最近添加了对二级索引的支持，"index" 一词可能指代二级索引、全文索引或普通/实时 `index`，这导致了混淆。为减少混淆，我们将后者重命名为 "table"。以下 SQL/命令行命令受此更改影响。它们的旧版本已被弃用，但仍可使用：
  - `index <table name>` => `table <table name>`，
  - `searchd -i / --index` => `searchd -t / --table`，
  - `SHOW INDEX STATUS` => `SHOW TABLE STATUS`，
  - `SHOW INDEX SETTINGS` => `SHOW TABLE SETTINGS`，
  - `FLUSH RTINDEX` => `FLUSH TABLE`，
  - `OPTIMIZE INDEX` => `OPTIMIZE TABLE`，
  - `ATTACH TABLE plain TO RTINDEX rt` => `ATTACH TABLE plain TO TABLE rt`，
  - `RELOAD INDEX` => `RELOAD TABLE`，
  - `RELOAD INDEXES` => `RELOAD TABLES`。

  我们不打算使旧形式过时，但为了确保与文档的兼容性，建议在您的应用程序中更改名称。未来版本中将更改的是各种 SQL 和 JSON 命令输出中的 "index" 到 "table" 的重命名。
* 带有有状态 UDF 的查询现在被强制在单线程中执行。
* [Issue #1011](https://github.com/manticoresoftware/manticoresearch/issues/1011) 重构了所有与时间调度相关的内容，为并行块合并做准备。
* **⚠️ 重大变更**：列存储格式已更改。您需要重建具有列属性的表。
* **⚠️ 重大变更**：二级索引文件格式已更改，因此如果您使用二级索引进行搜索，并且配置文件中有 `searchd.secondary_indexes = 1`，请注意新版本的 Manticore **将跳过加载具有二级索引的表**。建议：
  - 升级前将配置文件中的 `searchd.secondary_indexes` 改为 0。
  - 运行实例。Manticore 会加载表并发出警告。
  - 对每个索引运行 `ALTER TABLE <table name> REBUILD SECONDARY` 以重建二级索引。

  如果您运行的是复制集群，则需要在所有节点上运行 `ALTER TABLE <table name> REBUILD SECONDARY`，或按照[此说明](Securing_and_compacting_a_table/Compacting_a_table.md#Optimizing-clustered-tables)操作，只需将 `OPTIMIZE` 替换为 `ALTER .. REBUILD SECONDARY`。
* **⚠️ 重大变更**：binlog 版本已更新，因此之前版本的 binlog 将无法重放。升级过程中务必确保 Manticore Search 干净停止。停止之前的实例后，`/var/lib/manticore/binlog/` 中除了 `binlog.meta` 外不应有其他 binlog 文件。
* [Issue #849](https://github.com/manticoresoftware/manticoresearch/issues/849) `SHOW SETTINGS`：现在可以在 Manticore 内部查看配置文件中的设置。
* [Issue #1007](https://github.com/manticoresoftware/manticoresearch/issues/1007) [SET GLOBAL CPUSTATS=1/0](Server_settings/Setting_variables_online.md#SET) 用于开启/关闭 CPU 时间跟踪；[SHOW THREADS](Node_info_and_management/SHOW_THREADS.md) 在 CPU 时间跟踪关闭时不再显示 CPU 统计信息。
* [Issue #1009](https://github.com/manticoresoftware/manticoresearch/issues/1009) RT 表的 RAM 块段现在可以在 RAM 块刷新时合并。
* [Issue #1012](https://github.com/manticoresoftware/manticoresearch/issues/1012) 在 [indexer](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) 输出中添加了二级索引进度。
* [Issue #1013](https://github.com/manticoresoftware/manticoresearch/issues/1013) 之前如果 Manticore 启动时无法开始服务某个表记录，会将其从索引列表中移除。新行为是保留该记录，尝试在下次启动时加载。
* [indextool --docextract](Miscellaneous_tools.md#indextool) 返回请求文档的所有词和命中。
* [Commit 2b29](https://github.com/manticoresoftware/manticoresearch/commit/2b296ee20e520b85bcbb4383e87095d31e7165dc) 环境变量 `dump_corrupt_meta` 允许在 searchd 无法加载索引时将损坏的表元数据转储到日志。
* [Commit c7a3](https://github.com/manticoresoftware/manticoresearch/commit/c7a3136eefe0eb2f69e3d99fae9c10beb5821ea6) `DEBUG META` 可以显示 `max_matches` 和伪分片统计信息。
* [Commit 6bca](https://github.com/manticoresoftware/manticoresearch/commit/6bca4848dc5c8396dbc1e057be4b9b2d579794e8) 提供了比“Index header format is not json, will try it as binary...”更清晰的错误信息。
* [Commit bef3](https://github.com/manticoresoftware/lemmatizer-uk/commit/bef3ff0386d3ee87ec57619782100972c1122e47) 乌克兰语词形还原器路径已更改。
* [Commit 4ae7](https://github.com/manticoresoftware/manticoresearch/commit/4ae789595329a2951e194d1191ddb3121459a560) 在 [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META) 中添加了二级索引统计信息。
* [Commit 2e7c](https://github.com/manticoresoftware/manticoresearch/commit/2e7c585e) JSON 接口现在可以通过 Swagger Editor 轻松可视化 https://manual.manticoresearch.com/Openapi#OpenAPI-specification。
* **⚠️ 重大变更**：复制协议已更改。如果您运行复制集群，升级到 Manticore 5 时需要：
  - 先干净停止所有节点
  - 然后用 `--new-cluster` 启动最后停止的节点（Linux 下运行工具 `manticore_new_cluster`）。
  - 详细信息请参阅[重启集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster)。

### 与 Manticore 列存储库相关的更改
* 重构了二级索引与列存储的集成。
* [Commit efe2](https://github.com/manticoresoftware/columnar/commit/efe26b2a) Manticore 列存储库优化，通过部分预先的最小/最大值评估降低响应时间。
* [Commit 2757](https://github.com/manticoresoftware/columnar/commit/2757b99b) 如果磁盘块合并被中断，守护进程现在会清理与 MCL 相关的临时文件。
* [Commit e9c6](https://github.com/manticoresoftware/manticoresearch/commit/e9c672014b63e2b49e7a5a53880583af86c3ab4e) 在崩溃时，Columnar 和二级库版本会被写入日志。
* [Commit f5e8](https://github.com/manticoresoftware/columnar/commit/f5e84eeb8b6cb102f13e3468246702427527fef9) 为二级索引添加了快速文档列表回绕支持。
* [Commit 06df](https://github.com/manticoresoftware/manticoresearch/commit/06dfdd2e668933495dd6cb52d5ba2e0aa9b26e58) 类似 `select attr, count(*) from plain_index`（无过滤）的查询在使用 MCL 时现在更快。
* [Commit 0a76](https://github.com/manticoresoftware/manticoresearch/commit/0a76ecb9adbe886868b962dd425de5f475bbf55e) 在 HandleMysqlSelectSysvar 中添加 @@autocommit，以兼容 mysql 8.25 以上的 .net 连接器。
* **⚠️ 重大变更**：[MCL Issue #17](https://github.com/manticoresoftware/columnar/issues/17) MCL：为 columnar 扫描添加 SSE 代码。MCL 现在至少需要 SSE4.2。

### 与打包相关的变更
* [Commit 4d19](https://github.com/manticoresoftware/manticoresearch/commit/4d19f5cbe49a31228aa09253f061165bfe80e51b) **⚠️ 重大变更**：不再支持 Debian Stretch 和 Ubuntu Xenial。
* 支持 RHEL 9，包括 Centos 9、Alma Linux 9 和 Oracle Linux 9。
* [Issue #924](https://github.com/manticoresoftware/manticoresearch/issues/924) 支持 Debian Bookworm。
* [Issue #636](https://github.com/manticoresoftware/manticoresearch/issues/636) 打包：为 Linux 和 MacOS 提供 arm64 构建。
* [PR #26](https://github.com/manticoresoftware/docker/pull/26) 多架构（x86_64 / arm64）docker 镜像。
* [为贡献者简化的包构建](Installation/Compiling_from_sources.md#Building-using-CI-Docker)。
* 现在可以使用 APT 安装特定版本。
* [Commit a6b8](https://github.com/manticoresoftware/manticoresearch/commit/51fddca5c2a3ebb8576fae4c18660656ba22de0f) Windows 安装程序（之前只提供压缩包）。
* 切换到使用 CLang 15 编译。
* **⚠️ 重大变更**：自定义 Homebrew 配方，包括 Manticore Columnar Library 的配方。安装 Manticore、MCL 及其他必要组件，请使用以下命令 `brew install manticoresoftware/manticore/manticoresearch manticoresoftware/manticore/manticore-extra`。

### Bug 修复
* [Issue #479](https://github.com/manticoresoftware/manticoresearch/issues/479) 名为 `text` 的字段
* [Issue #501](https://github.com/manticoresoftware/manticoresearch/issues/501) id 不能是非 bigint 类型
* [Issue #646](https://github.com/manticoresoftware/manticoresearch/issues/646) ALTER 与名为 "text" 的字段冲突
* ❗[Issue #652](https://github.com/manticoresoftware/manticoresearch/issues/652) 可能的 BUG：HTTP（JSON）偏移和限制影响 facet 结果
* ❗[Issue #827](https://github.com/manticoresoftware/manticoresearch/issues/827) searchd 在高负载下挂起/崩溃
* ❗[Issue #996](https://github.com/manticoresoftware/manticoresearch/issues/996) PQ 索引内存溢出
* ❗[Commit 1041](https://github.com/manticoresoftware/manticoresearch/commit/10416ef7dddf06c0d759e32ccd6ebaa2468f7cbf) `binlog_flush = 1` 自 Sphinx 起一直有问题。已修复。
* [MCL Issue #14](https://github.com/manticoresoftware/columnar/issues/14) MCL：当 ft 字段过多时选择崩溃
* [Issue #470](https://github.com/manticoresoftware/manticoresearch/issues/470) sql_joined_field 无法存储
* [Issue #713](https://github.com/manticoresoftware/manticoresearch/issues/713) 使用 LEVENSHTEIN() 时崩溃
* [Issue #743](https://github.com/manticoresoftware/manticoresearch/issues/743) Manticore 意外崩溃且无法正常重启
* [Issue #788](https://github.com/manticoresoftware/manticoresearch/issues/788) 通过 /sql 调用 KEYWORDS 返回控制字符，导致 json 解析失败
* [Issue #789](https://github.com/manticoresoftware/manticoresearch/issues/789) mariadb 无法创建 FEDERATED 表
* [Issue #796](https://github.com/manticoresoftware/manticoresearch/issues/796) 警告：dlopen() 失败：/usr/bin/lib_manticore_columnar.so：无法打开共享对象文件：没有此文件或目录
* [Issue #797](https://github.com/manticoresoftware/manticoresearch/issues/797) 通过 api 使用 ZONESPAN 搜索时 Manticore 崩溃
* [Issue #799](https://github.com/manticoresoftware/manticoresearch/issues/799) 使用多个索引和 facet distinct 时权重错误
* [Issue #801](https://github.com/manticoresoftware/manticoresearch/issues/801) SQL 索引重处理后 SphinxQL 分组查询挂起
* [Issue #802](https://github.com/manticoresoftware/manticoresearch/issues/802) MCL：Indexer 在 5.0.2 和 manticore-columnar-lib 1.15.4 中崩溃
* [Issue #813](https://github.com/manticoresoftware/manticoresearch/issues/813) Manticore 5.0.2 FEDERATED 返回空集（MySQL 8.0.28）
* [Issue #824](https://github.com/manticoresoftware/manticoresearch/issues/824) 在两个索引上执行 COUNT DISTINCT，结果为零时抛出内部错误
* [Issue #826](https://github.com/manticoresoftware/manticoresearch/issues/826) 删除查询时崩溃
* [Issue #843](https://github.com/manticoresoftware/manticoresearch/issues/843) MCL：长文本字段的 Bug
* [Issue #856](https://github.com/manticoresoftware/manticoresearch/issues/856) 5.0.2 rtindex：聚合搜索限制行为不符合预期
* [Issue #863](https://github.com/manticoresoftware/manticoresearch/issues/863) 即使搜索应返回多个结果，返回的 Hits 也是 Nonetype 对象
* [Issue #870](https://github.com/manticoresoftware/manticoresearch/issues/870) 在 SELECT 表达式中使用 Attribute 和 Stored Field 时崩溃
* [Issue #872](https://github.com/manticoresoftware/manticoresearch/issues/872) 崩溃后表变得不可见
* [Issue #877](https://github.com/manticoresoftware/manticoresearch/issues/877) 搜索查询中两个负面词导致错误：查询不可计算
* [Issue #878](https://github.com/manticoresoftware/manticoresearch/issues/878) 通过 json query_string 使用 a -b -c 无效
* [Issue #886](https://github.com/manticoresoftware/manticoresearch/issues/886) 使用查询匹配的伪分片
* [Issue #893](https://github.com/manticoresoftware/manticoresearch/issues/893) Manticore 5.0.2 的 min/max 函数未按预期工作...
* [Issue #896](https://github.com/manticoresoftware/manticoresearch/issues/896) 字段 "weight" 解析不正确
* [Issue #897](https://github.com/manticoresoftware/manticoresearch/issues/897) Manticore 服务启动时崩溃并持续重启
* [Issue #900](https://github.com/manticoresoftware/manticoresearch/issues/900) group by j.a, smth 工作异常
* [Issue #913](https://github.com/manticoresoftware/manticoresearch/issues/913) 当在 ranker 中使用 expr 时，searchd 崩溃，但仅限于带有两个邻近条件的查询
* [Issue #916](https://github.com/manticoresoftware/manticoresearch/issues/916) net_throttle_action 失效
* [Issue #919](https://github.com/manticoresoftware/manticoresearch/issues/919) MCL：Manticore 在查询执行时崩溃，集群恢复期间也有崩溃
* [Issue #925](https://github.com/manticoresoftware/manticoresearch/issues/925) SHOW CREATE TABLE 输出缺少反引号
* [Issue #930](https://github.com/manticoresoftware/manticoresearch/issues/930) 现在可以通过 JDBC 连接器从 Java 查询 Manticore
* [Issue #933](https://github.com/manticoresoftware/manticoresearch/issues/933) bm25f 排名问题
* [Issue #934](https://github.com/manticoresoftware/manticoresearch/issues/934) 无配置索引在 watchdog 中冻结于首次加载状态
* [Issue #937](https://github.com/manticoresoftware/manticoresearch/issues/937) 排序 facet 数据时发生段错误
* [Issue #940](https://github.com/manticoresoftware/manticoresearch/issues/940) CONCAT(TO_STRING) 导致崩溃
* [Issue #947](https://github.com/manticoresoftware/manticoresearch/issues/947) 在某些情况下，单个简单的 select 查询可能导致整个实例停滞，无法登录或执行其他查询，直到该查询完成
* [Issue #948](https://github.com/manticoresoftware/manticoresearch/issues/948) 索引器崩溃
* [Issue #950](https://github.com/manticoresoftware/manticoresearch/issues/950) facet distinct 计数错误
* [Issue #953](https://github.com/manticoresoftware/manticoresearch/issues/953) 内置 sph04 排名器中 LCS 计算不正确
* [Issue #955](https://github.com/manticoresoftware/manticoresearch/issues/955) 5.0.3 开发版崩溃
* [Issue #963](https://github.com/manticoresoftware/manticoresearch/issues/963) 在 engine columnar 上使用 json 的 FACET 导致崩溃
* [Issue #982](https://github.com/manticoresoftware/manticoresearch/issues/982) MCL：5.0.3 从二级索引崩溃
* [PR #984](https://github.com/manticoresoftware/manticoresearch/pull/984) HandleMysqlSelectSysvar 中的 @@autocommit
* [PR #985](https://github.com/manticoresoftware/manticoresearch/pull/985) 修复 RT 索引中的线程块分配
* [Issue #985](https://github.com/manticoresoftware/manticoresearch/issues/985) 修复 RT 索引中的线程块分配
* [Issue #986](https://github.com/manticoresoftware/manticoresearch/issues/986) 错误的默认 max_query_time
* [Issue #987](https://github.com/manticoresoftware/manticoresearch/issues/987) 多线程执行中使用正则表达式时崩溃
* [Issue #988](https://github.com/manticoresoftware/manticoresearch/issues/988) 反向索引兼容性破损
* [Issue #989](https://github.com/manticoresoftware/manticoresearch/issues/989) indextool 报告检查 columnar 属性时出错
* [Issue #990](https://github.com/manticoresoftware/manticoresearch/issues/990) json grouper 克隆的内存泄漏
* [Issue #991](https://github.com/manticoresoftware/manticoresearch/issues/991) levenshtein 函数克隆的内存泄漏
* [Issue #992](https://github.com/manticoresoftware/manticoresearch/issues/992) 加载元数据时错误信息丢失
* [Issue #993](https://github.com/manticoresoftware/manticoresearch/issues/993) 传播来自动态索引/子键和系统变量的错误
* [Issue #994](https://github.com/manticoresoftware/manticoresearch/issues/994) 在列存储中对列式字符串执行 count distinct 导致崩溃
* [Issue #995](https://github.com/manticoresoftware/manticoresearch/issues/995) MCL：taxi1 的 min(pickup_datetime) 导致崩溃
* [Issue #997](https://github.com/manticoresoftware/manticoresearch/issues/997) 空的 excludes JSON 查询从选择列表中移除列
* [Issue #998](https://github.com/manticoresoftware/manticoresearch/issues/998) 当前调度器上运行的二级任务有时会导致异常副作用
* [Issue #999](https://github.com/manticoresoftware/manticoresearch/issues/999) facet distinct 与不同模式时崩溃
* [Issue #1000](https://github.com/manticoresoftware/manticoresearch/issues/1000) MCL：未加载列存储库时运行后列式 rt 索引损坏
* [Issue #1001](https://github.com/manticoresoftware/manticoresearch/issues/1001) json 中隐式 cutoff 不起作用
* [Issue #1002](https://github.com/manticoresoftware/manticoresearch/issues/1002) 列式分组器问题
* [Issue #1003](https://github.com/manticoresoftware/manticoresearch/issues/1003) 无法删除索引中的最后一个字段
* [Issue #1004](https://github.com/manticoresoftware/manticoresearch/issues/1004) --new-cluster 后行为异常
* [Issue #1005](https://github.com/manticoresoftware/manticoresearch/issues/1005) “columnar library not loaded”，但其实不需要
* [Issue #1006](https://github.com/manticoresoftware/manticoresearch/issues/1006) 删除查询无错误提示
* [Issue #1010](https://github.com/manticoresoftware/manticoresearch/issues/1010) 修正 Windows 构建中的 ICU 数据文件位置
* [PR #1018](https://github.com/manticoresoftware/manticoresearch/pull/1018) 握手发送问题
* [Issue #1020](https://github.com/manticoresoftware/manticoresearch/issues/1020) 在 show create table 中显示 id
* [Issue #1024 crash 1](https://github.com/manticoresoftware/manticoresearch/issues/1024) 在 Facet 搜索大量结果时崩溃 / 段错误。
* [Issue #1026](https://github.com/manticoresoftware/manticoresearch/issues/1026) RT 索引：当插入大量文档且 RAMchunk 满时，searchd “卡住” 永远不动
* [Commit 4739](https://github.com/manticoresoftware/manticoresearch/commit/4739dafd) 复制节点间忙碌时，关闭时线程卡住
* [Commit ab87](https://github.com/manticoresoftware/manticoresearch/commit/ab87836782e7ae43fe5f9dea739eed0d336b42c1) JSON 范围过滤器中混合浮点数和整数可能导致 Manticore 忽略过滤器
* [Commit d001](https://github.com/manticoresoftware/manticoresearch/commit/d00101c2905f6393ce3fca23c4f6dcb2506f4bd9) JSON 中的浮点过滤器不准确
* [Commit 4092](https://github.com/manticoresoftware/manticoresearch/commit/4092c6a8) 如果索引被修改，丢弃未提交的事务（否则可能崩溃）
* [Commit 9692](https://github.com/manticoresoftware/manticoresearch/commit/9692a840) 使用反斜杠时查询语法错误
* [Commit 0c19](https://github.com/manticoresoftware/manticoresearch/commit/0c19ad8d) SHOW STATUS 中 workers_clients 可能错误
* [Commit 1772](https://github.com/manticoresoftware/manticoresearch/commit/17720e1e2f2ec38337df565bee78f7ce9d9c744a) 修复合并无文档存储的内存段时崩溃
* [Commit f45b](https://github.com/manticoresoftware/manticoresearch/commit/f45b337b5c265433bb7d6262ad65d02f65edbaf3) 修复 JSON 过滤器中遗漏的 ALL/ANY 条件
* [Commit 3e83](https://github.com/manticoresoftware/manticoresearch/commit/3e83c1f0c0e6e7a9d4297113f69a112cbabd9e64) 如果 searchd 从无法写入的目录启动，复制可能失败并报错 `got exception while reading ist stream: mkstemp(./gmb_pF6TJi) failed: 13 (Permission denied)`
* [Commit 92e5](https://github.com/manticoresoftware/manticoresearch/commit/92e5629f7150f62524b385fca670a01ba9590f5b) 自 4.0.2 版本起崩溃日志只包含偏移量，本次提交修复了该问题。

## 版本 5.0.2
发布：2022年5月30日

### Bug 修复
* ❗[Issue #791](https://github.com/manticoresoftware/manticoresearch/issues/791) - 错误的堆栈大小可能导致崩溃。

## 版本 5.0.0
发布：2022年5月18日


### 主要新功能
* 🔬 支持 [Manticore Columnar Library 1.15.2](https://github.com/manticoresoftware/columnar/)，启用 [二级索引](Server_settings/Searchd.md#secondary_indexes) 测试版。对于普通和实时的列式及行式索引（如果使用了 [Manticore Columnar Library](https://github.com/manticoresoftware/columnar)），默认开启二级索引构建，但要启用搜索功能，需要在配置文件中或通过 [SET GLOBAL](Server_settings/Setting_variables_online.md) 设置 `secondary_indexes = 1`。该新功能支持除旧版 Debian Stretch 和 Ubuntu Xenial 以外的所有操作系统。
* [只读模式](Security/Read_only.md)：现在可以指定只处理读取查询、忽略任何写入的监听器。
* 新增 [/cli](Connecting_to_the_server/HTTP.md#/cli) 端点，使通过 HTTP 运行 SQL 查询更简单。
* 通过 HTTP JSON 进行批量 INSERT/REPLACE/DELETE 更快：之前可以通过 HTTP JSON 协议提供多个写入命令，但它们是逐个处理的，现在作为单个事务处理。
* [#720](https://github.com/manticoresoftware/manticoresearch/issues/720) JSON 协议支持 [嵌套过滤器](Searching/Filters.md#Nested-bool-query)。之前无法在 JSON 中编码如 `a=1 and (b=2 or c=3)` 的逻辑：`must`（AND）、`should`（OR）和 `must_not`（NOT）仅在最高层级生效。现在支持嵌套。
* 支持 HTTP 协议中的 [分块传输编码](https://en.wikipedia.org/wiki/Chunked_transfer_encoding)。现在可以在应用中使用分块传输发送大型批次，减少资源消耗（无需计算 `Content-Length`）。服务器端，Manticore 现在始终以流式方式处理传入的 HTTP 数据，无需等待整个批次传输完成，这：
  - 降低峰值内存使用，减少 OOM 风险
  - 缩短响应时间（我们的测试显示处理 100MB 批次时间减少约 11%）
  - 允许绕过 [max_packet_size](Server_settings/Searchd.md#max_packet_size) 限制，传输远大于最大允许值（128MB）的批次，例如一次传输 1GB。
* [#719](https://github.com/manticoresoftware/manticoresearch/issues/719) HTTP 接口支持 `100 Continue`：现在可以从 `curl`（包括各种编程语言使用的 curl 库）传输大批次，这些默认使用 `Expect: 100-continue` 并在实际发送批次前等待一段时间。之前需要添加 `Expect: ` 头部，现在不再需要。

  <details>

  之前（注意响应时间）：

  ```bash
  $ time curl -v -sX POST http://localhost:9318/bulk -H "Content-Type: application/x-ndjson" --data '{"insert": {"index": "user", "doc":  {"name":"Prof. Matt Heaney IV","email":"ibergnaum@yahoo.com","description":"Tempora ullam eaque consequatur. Vero aut minima ut et ut omnis officiis vel. Molestiae quis voluptatum sint numquam.","age":15,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"Prof. Boyd McKenzie","email":"carlotta11@hotmail.com","description":"Blanditiis maiores odio corporis eaque illum. Aut et rerum iste. Neque et ullam quisquam officia dignissimos quo cumque.","age":84,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"约翰·史密斯先生","email":"stiedemann.tristin@ziemann.com","description":"Temporibus amet magnam consequatur omnis consequatur illo fugit. Debitis natus doloremque est tempore deserunt vero. Harum eos corrupti nemo ut.","age":89,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"赫克托·普罗斯","email":"hickle.mafalda@hotmail.com","description":" as voluptatem inventore sit. Aliquam fugit perferendis est id aut odio et sapiente.","age":64,"active":1}}}'
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

  Now:
  ```bash
  $ time curl -v -sX POST http://localhost:9318/bulk -H "Content-Type: application/x-ndjson" --data '{"insert": {"index": "user", "doc":  {"name":"马特·希尼教授 IV","email":"ibergnaum@yahoo.com","description":"Tempora ullam eaque consequatur. Vero aut minima ut et ut omnis officiis vel. Molestiae quis voluptatum sint numquam.","age":15,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"博伊德·麦肯齐教授","email":"carlotta11@hotmail.com","description":"Blanditiis maiores odio corporis eaque illum. Aut et rerum iste. Neque et ullam quisquam officia dignissimos quo cumque.","age":84,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"约翰·史密斯先生","email":"stiedemann.tristin@ziemann.com","description":"Temporibus amet magnam consequatur omnis consequatur illo fugit. Debitis natus doloremque est tempore deserunt vero. Harum eos corrupti nemo ut.","age":89,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"赫克托·普罗斯","email":"hickle.mafalda@hotmail.com","description":" as voluptatem inventore sit. Aliquam fugit perferendis est id aut odio et sapiente.","age":64,"active":1}}}'
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

* **⚠️ 重大变更**: [伪分片](Server_settings/Searchd.md#pseudo_sharding) 默认启用。如果您想禁用它，请确保在 Manticore 配置文件的 `searchd` 部分添加 `pseudo_sharding = 0`。
* 真实时间/普通索引中至少包含一个全文字段不再是强制性的。现在即使不涉及全文搜索的场景，也可以使用 Manticore。
* 由 [Manticore Columnar Library](https://github.com/manticoresoftware/columnar) 支持的属性的[快速获取](Creating_a_table/Data_types.md#fast_fetch)：像 `select * from <columnar table>` 这样的查询现在比以前快得多，尤其是在模式中有许多字段时。
* **⚠️ 重大变更**: 隐式的[cutoff](Searching/Options.md#cutoff)。Manticore 现在不会花费时间和资源处理您不需要返回结果集中的数据。缺点是它会影响 [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META) 中的 `total_found` 和 JSON 输出中的 [hits.total](Searching/Full_text_matching/Basic_usage.md#HTTP-JSON)。只有当您看到 `total_relation: eq` 时，数值才准确；而 `total_relation: gte` 表示实际匹配文档数大于您获得的 `total_found` 值。要保持之前的行为，可以使用搜索选项 `cutoff=0`，这会使 `total_relation` 始终为 `eq`。
* **⚠️ 重大变更**: 所有全文字段现在默认[存储](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#stored_fields)。您需要使用 `stored_fields = `（空值）来使所有字段不存储（即恢复之前的行为）。
* [#715](https://github.com/manticoresoftware/manticoresearch/issues/715) HTTP JSON 支持[搜索选项](Searching/Options.md#General-syntax)。

### 小改动
* **⚠️ 重大变更**: 索引元文件格式变更。之前元文件（`.meta`，`.sph`）是二进制格式，现在改为 JSON 格式。新版本的 Manticore 会自动转换旧索引，但：
  - 您可能会看到类似 `WARNING: ... syntax error, unexpected TOK_IDENT` 的警告
  - 您将无法使用旧版本的 Manticore 运行该索引，请确保您有备份
* **⚠️ 重大变更**：通过[HTTP keep-alive](Connecting_to_the_server/HTTP.md#Keep-alive)支持会话状态。当客户端也支持时，这使得HTTP变为有状态。例如，使用新的[/cli](Connecting_to_the_server/HTTP.md#/cli)端点和HTTP keep-alive（所有浏览器默认开启），你可以在`SELECT`之后调用`SHOW META`，其行为与通过mysql相同。注意，之前也支持`Connection: keep-alive` HTTP头，但它仅导致复用同一连接。从此版本开始，它还使会话变为有状态。
* 现在你可以指定`columnar_attrs = *`，在[plain模式](Read_this_first.md#Real-time-mode-vs-plain-mode)下将所有属性定义为列式存储，这在属性列表较长时非常有用。
* 更快的复制SST
* **⚠️ 重大变更**：复制协议已更改。如果你运行复制集群，升级到Manticore 5时需要：
  - 先干净地停止所有节点
  - 然后用`--new-cluster`启动最后停止的节点（Linux下运行工具`manticore_new_cluster`）。
  - 详细信息请阅读[重启集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster)。
* 复制改进：
  - 更快的SST
  - 噪声抗干扰能力，帮助在复制节点间网络不稳定时保持稳定
  - 改进的日志记录
* 安全改进：如果配置中未指定`listen`，Manticore现在监听`127.0.0.1`而非`0.0.0.0`。尽管Manticore Search默认配置中指定了`listen`，且通常不会出现无`listen`配置的情况，但仍有可能。之前Manticore监听`0.0.0.0`，这不安全，现在监听`127.0.0.1`，通常不会暴露于互联网。
* 对列式属性的聚合更快。
* 提高`AVG()`的准确性：之前Manticore内部聚合使用`float`，现在改用`double`，显著提高准确度。
* 改进对JDBC MySQL驱动的支持。
* 支持[jemalloc](https://github.com/jemalloc/jemalloc)的`DEBUG malloc_stats`。
* [optimize_cutoff](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#optimize_cutoff)现在可作为每表设置，在CREATE或ALTER表时设置。
* **⚠️ 重大变更**：[query_log_format](Server_settings/Searchd.md#query_log_format)默认值现在为**`sphinxql`**。如果你习惯`plain`格式，需要在配置文件中添加`query_log_format = plain`。
* 显著减少内存消耗：在使用存储字段且进行长时间高强度插入/替换/优化工作负载时，Manticore显著减少RAM使用。
* [shutdown_timeout](Server_settings/Searchd.md#shutdown_timeout)默认值从3秒增加到60秒。
* [Commit ffd0](https://github.com/manticoresoftware/manticoresearch/commit/ffd0499d329d2c383f14c8a44c4cc84338ab56e7) 支持Java mysql连接器 >= 6.0.3：在[Java mysql连接6.0.3](https://mvnrepository.com/artifact/mysql/mysql-connector-java/6.0.3)中，他们更改了连接mysql的方式，导致与Manticore不兼容。现在支持新行为。
* [Commit 1da6](https://github.com/manticoresoftware/manticoresearch/commit/1da6dbec) 禁止在加载索引时（如searchd启动时）保存新的磁盘块。
* [Issue #746](https://github.com/manticoresoftware/manticoresearch/issues/746) 支持glibc >= 2.34。
* [Issue #784](https://github.com/manticoresoftware/manticoresearch/issues/784) 将“VIP”连接与普通（非VIP）连接分开计数。之前VIP连接计入`max_connections`限制，可能导致非VIP连接出现“已达最大”错误。现在VIP连接不计入限制。当前VIP连接数可通过`SHOW STATUS`和`status`查看。
* [ID](Creating_a_table/Data_types.md#Document-ID)现在可以显式指定。
* [Issue #687](https://github.com/manticoresoftware/manticoresearch/issues/687) 支持mysql协议的zstd压缩。

### ⚠️ 其他小的重大变更
* ⚠️ BM25F公式略有更新以提升搜索相关性。仅影响使用函数[BM25F()](Functions/Searching_and_ranking_functions.md#BM25F%28%29)的搜索结果，不改变默认排名公式行为。
* ⚠️ REST [/sql](Connecting_to_the_server/HTTP.md#mode=raw)端点行为更改：`/sql?mode=raw`现在需要转义并返回数组。
* ⚠️ `/bulk` INSERT/REPLACE/DELETE请求响应格式更改：
  - 之前每个子查询构成单独事务，返回单独响应
  - 现在整个批次视为单一事务，返回单一响应
* ⚠️ 搜索选项`low_priority`和`boolean_simplify`现在需要赋值（`0/1`）：之前可写`SELECT ... OPTION low_priority, boolean_simplify`，现在需写`SELECT ... OPTION low_priority=1, boolean_simplify=1`。
* ⚠️ 如果你使用旧版[php](https://github.com/manticoresoftware/manticoresearch/blob/master/api/sphinxapi.php)、[python](https://github.com/manticoresoftware/manticoresearch/blob/master/api/sphinxapi.py)或[java](https://github.com/manticoresoftware/manticoresearch/tree/master/api/java)客户端，请访问相应链接获取更新版本。**旧版本与Manticore 5不完全兼容。**
* ⚠️ HTTP JSON请求在`query_log_format=sphinxql`模式下以不同格式记录。之前仅记录全文部分，现在按原样记录。

### 新软件包
* **⚠️ 重大变更**：由于新结构，升级到Manticore 5时建议先卸载旧软件包再安装新包：
  - 基于RPM的系统：`yum remove manticore*`
  - Debian和Ubuntu：`apt remove manticore*`
* 新的 deb/rpm 包结构。之前的版本提供了：
  - `manticore-server` 包含 `searchd`（主搜索守护进程）及其所需的所有内容
  - `manticore-tools` 包含 `indexer` 和 `indextool`
  - `manticore` 包含所有内容
  - `manticore-all` RPM 作为元包，引用 `manticore-server` 和 `manticore-tools`

  新结构为：
  - `manticore` - deb/rpm 元包，安装上述所有包作为依赖
  - `manticore-server-core` - `searchd` 及其独立运行所需的所有内容
  - `manticore-server` - systemd 文件及其他辅助脚本
  - `manticore-tools` - `indexer`、`indextool` 及其他工具
  - `manticore-common` - 默认配置文件、默认数据目录、默认停用词
  - `manticore-icudata`、`manticore-dev`、`manticore-converter` 变化不大
  - 包含所有包的 `.tgz` 捆绑包
* 支持 Ubuntu Jammy
* 通过 [YUM repo](Installation/RHEL_and_Centos.md#YUM-repository) 支持 Amazon Linux 2

### Bug 修复
* [Issue #815](https://github.com/manticoresoftware/manticoresearch/issues/815) 使用 UDF 函数时随机崩溃
* [Issue #287](https://github.com/manticoresoftware/manticoresearch/issues/287) 索引 RT 索引时内存溢出
* [Issue #604](https://github.com/manticoresoftware/manticoresearch/issues/604) 3.6.0，4.2.0 sphinxql-parser 破坏性更改
* [Issue #667](https://github.com/manticoresoftware/manticoresearch/issues/667) FATAL: 内存不足（无法分配 9007199254740992 字节）
* [Issue #676](https://github.com/manticoresoftware/manticoresearch/issues/676) 字符串未正确传递给 UDF
* ❗[Issue #698](https://github.com/manticoresoftware/manticoresearch/issues/698) 尝试向 rt 索引添加文本列后 searchd 崩溃
* [Issue #705](https://github.com/manticoresoftware/manticoresearch/issues/705) Indexer 无法找到所有列
* ❗[Issue #709](https://github.com/manticoresoftware/manticoresearch/issues/705) 按 json.boolean 分组错误
* [Issue #716](https://github.com/manticoresoftware/manticoresearch/issues/716) indextool 与索引相关的命令（如 --dumpdict）失败
* ❗[Issue #724](https://github.com/manticoresoftware/manticoresearch/issues/724) 字段从选择中消失
* [Issue #727](https://github.com/manticoresoftware/manticoresearch/issues/727) .NET HttpClient 使用 `application/x-ndjson` 时 Content-Type 不兼容
* [Issue #729](https://github.com/manticoresoftware/manticoresearch/issues/729) 字段长度计算
* ❗[Issue #730](https://github.com/manticoresoftware/manticoresearch/issues/730) 创建/插入/删除列式表时存在内存泄漏
* [Issue #731](https://github.com/manticoresoftware/manticoresearch/issues/731) 某些条件下结果中出现空列
* ❗[Issue #749](https://github.com/manticoresoftware/manticoresearch/issues/749) 守护进程启动时崩溃
* ❗[Issue #750](https://github.com/manticoresoftware/manticoresearch/issues/750) 守护进程启动时挂起
* ❗[Issue #751](https://github.com/manticoresoftware/manticoresearch/issues/751) SST 时崩溃
* [Issue #752](https://github.com/manticoresoftware/manticoresearch/issues/752) 当 engine='columnar' 时，json 属性被标记为列式
* [Issue #753](https://github.com/manticoresoftware/manticoresearch/issues/753) 复制监听地址为 0
* [Issue #754](https://github.com/manticoresoftware/manticoresearch/issues/754) columnar_attrs = * 在 csvpipe 中不起作用
* ❗[Issue #755](https://github.com/manticoresoftware/manticoresearch/issues/755) rt 中选择列式浮点数时崩溃
* ❗[Issue #756](https://github.com/manticoresoftware/manticoresearch/issues/756) Indextool 在检查时更改 rt 索引
* [Issue #757](https://github.com/manticoresoftware/manticoresearch/issues/757) 需要检查监听端口范围是否有交叉
* [Issue #758](https://github.com/manticoresoftware/manticoresearch/issues/758) RT 索引保存磁盘块失败时记录原始错误
* [Issue #759](https://github.com/manticoresoftware/manticoresearch/issues/759) RE2 配置只报告一个错误
* ❗[Issue #760](https://github.com/manticoresoftware/manticoresearch/issues/760) 提交 5463778558586d2508697fa82e71d657ac36510f 中的内存消耗变化
* [Issue #761](https://github.com/manticoresoftware/manticoresearch/issues/761) 第三节点脏重启后未成为非主集群
* [Issue #762](https://github.com/manticoresoftware/manticoresearch/issues/762) 更新计数器增加了 2
* [Issue #763](https://github.com/manticoresoftware/manticoresearch/issues/763) 新版本 4.2.1 损坏了使用形态学的 4.2.0 创建的索引
* [Issue #764](https://github.com/manticoresoftware/manticoresearch/issues/764) json 键中无转义 /sql?mode=raw
* ❗[Issue #765](https://github.com/manticoresoftware/manticoresearch/issues/765) 使用函数隐藏其他值
* ❗[Issue #766](https://github.com/manticoresoftware/manticoresearch/issues/766) FixupAttrForNetwork 中一行代码触发内存泄漏
* ❗[Issue #767](https://github.com/manticoresoftware/manticoresearch/issues/767) 4.2.0 和 4.2.1 中与文档存储缓存相关的内存泄漏
* [Issue #768](https://github.com/manticoresoftware/manticoresearch/issues/768) 网络上传输存储字段时出现奇怪的乒乓现象
* [Issue #769](https://github.com/manticoresoftware/manticoresearch/issues/769) 如果 'common' 部分未提及，lemmatizer_base 重置为空
* [Issue #770](https://github.com/manticoresoftware/manticoresearch/issues/770) pseudo_sharding 使按 id 查询变慢
* [Issue #771](https://github.com/manticoresoftware/manticoresearch/issues/771) 使用 jemalloc 时 DEBUG malloc_stats 输出为零
* [Issue #772](https://github.com/manticoresoftware/manticoresearch/issues/772) 删除/添加列使值不可见
* [Issue #773](https://github.com/manticoresoftware/manticoresearch/issues/773) 无法向列式表添加 bit(N) 列
* [Issue #774](https://github.com/manticoresoftware/manticoresearch/issues/774) manticore.json 启动时 "cluster" 变为空
* ❗[提交 1da4](https://github.com/manticoresoftware/manticoresearch/commits/1da4ce891e4ff13727cfd3331d72771c64ee7949) HTTP 操作未在 SHOW STATUS 中跟踪
* [提交 3810](https://github.com/manticoresoftware/manticoresearch/commits/381000ab4af4c3c98ec8c730699ad7f39039cec8) 禁用低频单关键词查询的伪分片
* [提交 8003](https://github.com/manticoresoftware/manticoresearch/commits/800325cca283a87801b28929d82420fafc76a0ee) 修复存储属性与索引合并的问题
* [提交 cddf](https://github.com/manticoresoftware/manticoresearch/commits/cddfeed6296f2041c6aae18414e5cd9cabf08281) 泛化了不同值的获取器；为列式字符串添加了专门的不同值获取器
* [提交 fba4](https://github.com/manticoresoftware/manticoresearch/commits/fba4bb4f) 修复从文档存储中获取空整数属性的问题
* [提交 f300](https://github.com/manticoresoftware/manticoresearch/commit/f3009a9242fa16956adf871c4887e6e0303ba364) 查询日志中 `ranker` 可能被指定两次

## 版本 4.2.0，2021年12月23日

### 主要新特性
* **实时索引和全文查询的伪分片支持**。在之前的版本中我们添加了有限的伪分片支持。从此版本开始，只需启用 [searchd.pseudo_sharding](Server_settings/Searchd.md#pseudo_sharding)，即可获得伪分片和多核处理器的所有优势。最酷的是你无需对索引或查询做任何修改，只需启用它，如果有空闲 CPU，系统将利用它来降低响应时间。它支持全文、过滤和分析查询的普通和实时索引。例如，启用伪分片后，在 [Hacker news 精选评论数据集](https://zenodo.org/record/45901/)（放大100倍，含1.16亿文档的普通索引）上，大多数查询的**平均响应时间降低约10倍**。

![4.2.0 中伪分片开启与关闭对比](4.2.0_ps_on_vs_off.png)

* 现已支持 [**Debian Bullseye**](https://manticoresearch.com/install/)。

<!-- example pq_transactions_4.2.0 -->
* PQ 事务现在是原子且隔离的。之前 PQ 事务支持有限。此改进使得 **REPLACE 到 PQ 的速度更快**，尤其是在需要一次替换大量规则时。性能详情：

<!-- intro -->
#### 4.0.2

<!-- request 4.0.2 -->

插入 100 万条 PQ 规则需要 **48 秒**，而以 1 万条批次替换仅 4 万条规则需要 **406 秒**。

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

插入 100 万条 PQ 规则需要 **34 秒**，以 1 万条批次替换它们只需 **23 秒**。

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
* [optimize_cutoff](Server_settings/Searchd.md#optimize_cutoff) 现作为 `searchd` 配置节中的选项提供。当你想全局限制所有索引中的 RT 块数量时非常有用。
* [提交 0087](https://github.com/manticoresoftware/manticoresearch/commit/00874743823eaf43e2a09a088392fd7e0a617f47) 精确支持多个本地物理索引（实时/普通）中具有相同字段集/顺序的 [count(distinct ...)](Searching/Grouping.md#COUNT%28DISTINCT-field%29) 和 [FACET ... distinct](Searching/Faceted_search.md#Faceting-without-duplicates)。
* [PR #598](https://github.com/manticoresoftware/manticoresearch/pull/598) 为 `YEAR()` 和其他时间戳函数添加 bigint 支持。
* [提交 8e85](https://github.com/manticoresoftware/manticoresearch/commit/8e85d4bce596cc88f9d9615ea243a27e4afde0f7) 自适应 [rt_mem_limit](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_mem_limit)。之前 Manticore Search 在保存新的磁盘块之前，严格收集到 `rt_mem_limit` 大小的数据，保存时仍会收集多达 10%（即双缓冲）以减少插入暂停。如果该限制也被耗尽，则在磁盘块完全保存之前阻止添加新文档。新的自适应限制基于我们现在有了 [auto-optimize](Server_settings/Searchd.md#auto_optimize) 的事实，因此磁盘块不完全遵守 `rt_mem_limit` 并提前开始刷新并不严重。现在我们收集最多 50% 的 `rt_mem_limit` 并保存为磁盘块。保存时，我们查看统计数据（保存了多少，保存时有多少新文档到达）并重新计算下次使用的初始比率。例如，如果保存了 9000 万文档，保存时又到达了 1000 万文档，则比率为 90%，因此下次可以收集最多 90% 的 `rt_mem_limit` 后再开始刷新另一个磁盘块。比率值自动计算，范围从 33.3% 到 95%。
* [问题 #628](https://github.com/manticoresoftware/manticoresearch/issues/628) PostgreSQL 源的 [unpack_zlib](Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Database_connection.md#unpack_zlib)。感谢 [Dmitry Voronin](https://github.com/dimv36) 的 [贡献](https://github.com/manticoresoftware/manticoresearch/pull/630)。
* [提交 6d54](https://github.com/manticoresoftware/manticoresearch/commit/6d54cf2b319b131970c29410cc21abcbdf8083b1) `indexer -v` 和 `--version`。之前你仍能看到 indexer 的版本，但 `-v`/`--version` 不被支持。
* [问题 #662](https://github.com/manticoresoftware/manticoresearch/issues/662) 当通过 systemd 启动 Manticore 时，默认无限 mlock 限制。
* [提交 63c8](https://github.com/manticoresoftware/manticoresearch/commit/63c8cd05cf399e705c2c14939411a92cb784735a) 协程读写锁的自旋锁改为操作队列。
* [提交 4113](https://github.com/manticoresoftware/manticoresearch/commit/41130ce30d1734bbf8e6b8b63c4e815caf9464f2) 环境变量 `MANTICORE_TRACK_RT_ERRORS`，用于调试 RT 段损坏。

### 重大变更
* Binlog 版本已提升，之前版本的 binlog 不会被重放，因此请确保在升级过程中干净地停止 Manticore Search：停止之前的实例后，`/var/lib/manticore/binlog/` 目录中除了 `binlog.meta` 外不应有 binlog 文件。
* [提交 3f65](https://github.com/manticoresoftware/manticoresearch/commit/3f659f36e87d99ee262205a8eae4663a255be630) 在 `show threads option format=all` 中新增了 "chain" 列。它显示了一些任务信息票据的堆栈，对性能分析非常有用，因此如果你在解析 `show threads` 输出，请注意新增的列。
* `searchd.workers` 自 3.5.0 起已废弃，现在是弃用状态，如果你的配置文件中仍有该项，启动时会触发警告。Manticore Search 会启动，但会有警告提示。
* 如果你使用 PHP 和 PDO 访问 Manticore，需要设置 `PDO::ATTR_EMULATE_PREPARES`

### Bug 修复
* ❗[问题 #650](https://github.com/manticoresoftware/manticoresearch/issues/650) Manticore 4.0.2 比 Manticore 3.6.3 慢。4.0.2 在批量插入方面比之前版本更快，但单条文档插入明显变慢。此问题已在 4.2.0 中修复。
* ❗[提交 22f4](https://github.com/manticoresoftware/manticoresearch/commit/22f4141bed1411c83684ae6e0bae832071cce3df) RT 索引在高强度 REPLACE 负载下可能损坏，或导致崩溃
* [提交 03be](https://github.com/manticoresoftware/manticoresearch/commit/03be91e4f720a3f20bee0278dd8f0d44abd3ef0e) 修正了合并分组器和 group N 排序器时的平均值计算；修正了聚合合并
* [提交 2ea5](https://github.com/manticoresoftware/manticoresearch/commit/2ea575d34efe38661f14d858f2b552456f29fe42) `indextool --check` 可能崩溃
* [提交 7ec7](https://github.com/manticoresoftware/manticoresearch/commit/7ec76d4abf01b5fca6f10a0000fe64d5d8e5fa52) 由 UPDATE 导致的内存耗尽问题
* [提交 658a](https://github.com/manticoresoftware/manticoresearch/commit/658a727e41f3c3fbcde9754149528f34ed76ba19) 守护进程在 INSERT 时可能挂起
* [提交 46e4](https://github.com/manticoresoftware/manticoresearch/commit/46e42b9b386e31e53247ed634e158b533e28063e) 守护进程在关闭时可能挂起
* [提交 f8d7](https://github.com/manticoresoftware/manticoresearch/commit/f8d7d5177cc83d250b493b4cfc619732b29cf072) 守护进程在关闭时可能崩溃
* [提交 733a](https://github.com/manticoresoftware/manticoresearch/commit/733accf165142a1d1574689525ccca8907971158) 守护进程崩溃时可能挂起
* [提交 f7f8](https://github.com/manticoresoftware/manticoresearch/commit/f7f8bd8c0dbb23a59a88502dcba16b0918fb2330) 守护进程启动时尝试用无效节点列表重新加入集群可能崩溃
* [提交 1401](https://github.com/manticoresoftware/manticoresearch/commit/140155613821ecaddcd1a3dfc4f7369b0df6aef7) 分布式索引在 RT 模式下启动时如果无法解析其中一个代理，可能会被完全遗忘
* [问题 #683](https://github.com/manticoresoftware/manticoresearch/issues/683) attr bit(N) engine='columnar' 失败
* [问题 #682](https://github.com/manticoresoftware/manticoresearch/issues/682) 创建表失败，但留下目录
* [问题 #663](https://github.com/manticoresoftware/manticoresearch/issues/663) 配置失败，提示未知键名 'attr_update_reserve'
* [问题 #632](https://github.com/manticoresoftware/manticoresearch/issues/632) Manticore 在批量查询时崩溃
* [问题 #679](https://github.com/manticoresoftware/manticoresearch/issues/679) 批量查询在 v4.0.3 中再次导致崩溃
* [提交 f7f8](https://github.com/manticoresoftware/manticoresearch/commit/f7f8bd8c0dbb23a59a88502dcba16b0918fb2330) 修复了守护进程启动时尝试用无效节点列表重新加入集群导致崩溃的问题
* [问题 #643](https://github.com/manticoresoftware/manticoresearch/issues/643) Manticore 4.0.2 在批量插入后不接受连接
* [问题 #635](https://github.com/manticoresoftware/manticoresearch/issues/635) FACET 查询中使用 ORDER BY JSON.field 或字符串属性可能崩溃
* [问题 #634](https://github.com/manticoresoftware/manticoresearch/issues/634) 使用 packedfactors 查询时崩溃 SIGSEGV
* [提交 4165](https://github.com/manticoresoftware/manticoresearch/commits/41657f15f8fcf1e93cfad7f835bcf705d6e8349e) morphology_skip_fields 不支持 create table

## 版本 4.0.2，2021年9月21日

### 主要新特性
- **全面支持 [Manticore Columnar Library](https://github.com/manticoresoftware/columnar)**。之前 Manticore Columnar Library 仅支持普通索引。现在支持：
  - 实时索引的 `INSERT`、`REPLACE`、`DELETE`、`OPTIMIZE`
  - 复制功能
  - `ALTER`
  - `indextool --check`
- **自动索引压缩** ([问题 #478](https://github.com/manticoresoftware/manticoresearch/issues/478))。终于，你不必手动调用 OPTIMIZE 或通过定时任务或其他自动化方式执行。Manticore 现在默认自动完成此操作。你可以通过全局变量 [optimize_cutoff](Server_settings/Setting_variables_online.md) 设置默认压缩阈值。
- **块快照和锁系统重构**。这些改动初看可能无明显变化，但显著改善了实时索引中许多操作的行为。简而言之，之前大多数 Manticore 数据操作严重依赖锁，现在改用磁盘块快照。
- **实时索引批量 INSERT 性能显著提升**。例如，在 [Hetzner 的 AX101 服务器](https://www.hetzner.com/dedicated-rootserver/ax101)（SSD，128 GB 内存，AMD Ryzen™ 9 5950X（16*2 核））上，**3.6.0 版本可实现每秒插入 236K 文档**，表结构为 `name text, email string, description text, age int, active bit(1)`（默认 `rt_mem_limit`，批量大小 25000，16 并发插入工作线程，总计插入 1600 万文档）。4.0.2 版本在相同并发/批量/数量下达到 **每秒 357K 文档**。

  <details>

  - 读取操作（如 SELECT、复制）使用快照执行
  - 仅更改内部索引结构而不修改模式/文档的操作（例如合并RAM段、保存磁盘块、合并磁盘块）使用只读快照执行，最终替换现有块
  - UPDATE和DELETE针对现有块执行，但在可能发生合并的情况下，写入操作会被收集，然后应用到新块上
  - UPDATE对每个块依次获取独占锁。合并在进入收集块属性阶段时获取共享锁。因此同时只有一个（合并或更新）操作可以访问块的属性。
  - 当合并进入需要属性的阶段时，会设置一个特殊标志。UPDATE完成后检查该标志，如果设置了，整个更新会存储在一个特殊集合中。最后，合并完成时，将更新应用到新生成的磁盘块。
  - ALTER通过独占锁运行
  - 复制作为常规读取操作运行，但额外在SST之前保存属性，并禁止SST期间的更新

  </details>

- **[ALTER](Updating_table_schema_and_settings.md) 可以添加/删除全文字段**（在RT模式下）。之前只能添加/删除属性。
- 🔬 **实验性：全扫描查询的伪分片** - 允许并行化任何非全文搜索查询。现在无需手动准备分片，只需启用新选项 [searchd.pseudo_sharding](Server_settings/Searchd.md#pseudo_sharding)，即可期望非全文搜索查询响应时间降低至最多 `CPU核心数` 倍。注意它可能会占用所有现有CPU核心，因此如果你不仅关心延迟，还关心吞吐量，请谨慎使用。

### 小改动
<!-- example -->
- 通过 [APT仓库](Installation/Debian_and_Ubuntu.md#APT-repository) 支持 Linux Mint 和 Ubuntu Hirsute Hippo
- 在某些情况下，通过HTTP按ID更新大索引更快（取决于ID分布）
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
- [systemd的自定义启动标志](Starting_the_server/Linux.md#Custom-startup-flags-using-systemd)。现在如果需要用特定启动标志运行Manticore，无需手动启动searchd
- 新函数 [LEVENSHTEIN()](Functions/String_functions.md#LEVENSHTEIN%28%29)，计算Levenshtein距离
- 新增 [searchd启动标志](Starting_the_server/Manually.md#searchd-command-line-options) `--replay-flags=ignore-trx-errors` 和 `--replay-flags=ignore-all-errors`，即使binlog损坏也能启动searchd
- [Issue #621](https://github.com/manticoresoftware/manticoresearch/issues/621) - 公开RE2的错误信息
- 对由本地普通索引组成的分布式索引，提供更准确的 [COUNT(DISTINCT)](Searching/Grouping.md#COUNT%28DISTINCT-field%29)
- [FACET DISTINCT](Searching/Faceted_search.md#Faceting-without-duplicates) 用于在分面搜索时去重
- [精确形式修饰符](Searching/Full_text_matching/Operators.md#Exact-form-modifier) 现在不再依赖 [形态学](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology)，且适用于启用 [中缀/前缀](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) 搜索的索引

### 重大变更
- 新版本可以读取旧索引，但旧版本无法读取Manticore 4的索引
- 移除隐式按id排序。如需排序请显式指定
- `charset_table` 默认值从 `0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451` 改为 `non_cjk`
- `OPTIMIZE` 自动执行。如不需要，请确保在配置文件的 `searchd` 部分设置 `auto_optimize=0`
- [Issue #616](https://github.com/manticoresoftware/manticoresearch/issues/616) `ondisk_attrs_default` 已弃用，现已移除
- 对贡献者：Linux构建现在使用Clang编译器，根据测试它能构建更快的Manticore Search和Manticore Columnar库
- 如果搜索查询中未指定 [max_matches](Searching/Options.md#max_matches)，则会隐式更新为满足新列存储性能所需的最低值。这可能影响 [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META) 中的 `total` 指标，但不影响实际找到文档数的 `total_found`

### 从Manticore 3迁移
- 确保干净停止Manticore 3：
  - `/var/lib/manticore/binlog/` 中不应有binlog文件（目录中只应有 `binlog.meta`）
  - 否则Manticore 4无法回复binlog的索引将无法运行
- 新版本可以读取旧索引，但旧版本无法读取Manticore 4的索引，因此如果想轻松回滚新版本，请务必备份
- 如果运行复制集群，请确保：
  - 先干净停止所有节点
  - 然后用 `--new-cluster` 启动最后停止的节点（Linux下运行工具 `manticore_new_cluster`）
  - 更多细节请阅读 [重启集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster)

### Bug修复
- 修复了大量复制问题：
  - [Commit 696f](https://github.com/manticoresoftware/manticoresearch/commit/696f8649535f2b5285b878c9581c0d2b1139ae09) - 修复了joiner节点在SST期间激活索引时的崩溃；在joiner节点写入文件块时添加sha1校验以加速索引加载；在索引加载时添加了joiner节点的变更索引文件轮换；当激活索引被donor节点的新索引替换时，添加了joiner节点的索引文件删除；为donor节点发送文件和块添加了复制日志点
  - [提交 b296](https://github.com/manticoresoftware/manticoresearch/commit/b296c55af68314a8fc66f8b104e7301d673c3b68) - 当地址不正确时，JOIN CLUSTER 导致崩溃
  - [提交 418b](https://github.com/manticoresoftware/manticoresearch/commit/418bf880a5e05ffbb68f311a1c23006a9df3220e) - 在大型索引的初始复制期间，加入节点可能会失败并显示 `ERROR 1064 (42000): invalid GTID, (null)`，当另一个节点加入时，捐赠节点可能变得无响应
  - [提交 6fd3](https://github.com/manticoresoftware/manticoresearch/commit/6fd350d28e4905211cf8a7298cbf7cdda101c83d) - 大型索引的哈希可能计算错误，导致复制失败
  - [问题 #615](https://github.com/manticoresoftware/manticoresearch/issues/615) - 集群重启时复制失败
- [问题 #574](https://github.com/manticoresoftware/manticoresearch/issues/574) - `indextool --help` 不显示参数 `--rotate`
- [问题 #578](https://github.com/manticoresoftware/manticoresearch/issues/578) - searchd 空闲时 CPU 使用率高，约一天后出现
- [问题 #587](https://github.com/manticoresoftware/manticoresearch/issues/587) - 立即刷新 .meta 文件
- [问题 #617](https://github.com/manticoresoftware/manticoresearch/issues/617) - manticore.json 被清空
- [问题 #618](https://github.com/manticoresoftware/manticoresearch/issues/618) - searchd --stopwait 在 root 下失败。它还修复了 systemctl 行为（之前 ExecStop 显示失败且未能充分等待 searchd 正确停止）
- [问题 #619](https://github.com/manticoresoftware/manticoresearch/issues/619) - INSERT/REPLACE/DELETE 与 SHOW STATUS。`command_insert`、`command_replace` 等显示了错误的指标
- [问题 #620](https://github.com/manticoresoftware/manticoresearch/issues/620) - 纯索引的 `charset_table` 默认值错误
- [提交 8f75](https://github.com/manticoresoftware/manticoresearch/commit/8f7536887d4b58a5bab5647fc067b2d0482c2038) - 新的磁盘块未被 mlocked
- [问题 #607](https://github.com/manticoresoftware/manticoresearch/issues/607) - 当无法通过名称解析节点时，Manticore 集群节点崩溃
- [问题 #623](https://github.com/manticoresoftware/manticoresearch/issues/623) - 更新索引的复制可能导致未定义状态
- [提交 ca03](https://github.com/manticoresoftware/manticoresearch/commit/ca03d2280c5197e9b311f0f03f551e3b702a8130) - 索引器在索引带有 json 属性的纯索引源时可能挂起
- [提交 53c7](https://github.com/manticoresoftware/manticoresearch/commit/53c753054558ff3f82c7af365273d32537c5338b) - 修复 PQ 索引中的不等表达式过滤器
- [提交 ccf9](https://github.com/manticoresoftware/manticoresearch/commit/ccf94e022ba6763d6286ab5bac767b6f6a846846) - 修复列表查询中超过 1000 条匹配的选择窗口。之前 `SELECT * FROM pq ORDER BY id desc LIMIT 1000 , 100 OPTION max_matches=1100` 无法正常工作
- [提交 a048](https://github.com/manticoresoftware/manticoresearch/commit/a0483fe9bef3c45dffe96f5df4f4382aee29c072) - 对 Manticore 的 HTTPS 请求可能导致类似 "max packet size(8388608) exceeded" 的警告
- [问题 #648](https://github.com/manticoresoftware/manticoresearch/issues/648) - Manticore 3 在字符串属性更新几次后可能挂起


## 版本 3.6.0，2021年5月3日
**Manticore 4 之前的维护版本**

### 主要新功能
- 支持纯索引的 [Manticore Columnar Library](https://github.com/manticoresoftware/columnar/)。纯索引新增设置 [columnar_attrs](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#columnar_attrs)
- 支持 [乌克兰语词形还原器](https://github.com/manticoresoftware/lemmatizer-uk)
- 完全修订的直方图。在构建索引时，Manticore 也会为其中的每个字段构建直方图，随后用于更快的过滤。3.6.0 中算法被完全修订，如果你有大量数据并进行大量过滤，可以获得更高性能。

### 次要更改
- 工具 `manticore_new_cluster [--force]`，用于通过 systemd 重启复制集群
- `indexer --merge` 的 [--drop-src](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Merging_tables.md#Merging-tables)
- 新模式 [blend_mode='trim_all'](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology)
- 添加了用反引号转义 JSON 路径的支持 [支持转义 JSON 路径](Searching/Full_text_matching/Escaping.md#Escaping-json-node-names-in-SQL)
- [indextool --check](Miscellaneous_tools.md#indextool) 可以在 RT 模式下工作
- SELECT/UPDATE 支持 [FORCE/IGNORE INDEX(id)](Data_creation_and_modification/Updating_documents/UPDATE.md#Updates-via-SQL)
- 合并的磁盘块的块 ID 现在唯一
- [indextool --check-disk-chunk CHUNK_NAME](Miscellaneous_tools.md#indextool)

### 优化
- [更快的 JSON 解析](https://github.com/manticoresoftware/manticoresearch/commit/0c25fc1788e3f77f60cb7fb8f1623d8466bba606)，我们的测试显示在类似 `WHERE json.a = 1` 的查询中延迟降低了 3-4%
- 非文档化命令 `DEBUG SPLIT`，作为自动分片/重新平衡的前提条件

### Bug 修复
- [问题 #584](https://github.com/manticoresoftware/manticoresearch/issues/524) - 不准确且不稳定的 FACET 结果
- [问题 #506](https://github.com/manticoresoftware/manticoresearch/issues/506) - 使用 MATCH 时的奇怪行为：受此问题影响的用户需要重建索引，因为问题出在索引构建阶段
- [问题 #387](https://github.com/manticoresoftware/manticoresearch/issues/387) - 运行带有 SNIPPET() 函数的查询时间歇性核心转储
- 用于处理复杂查询的堆栈优化：
  - [问题 #469](https://github.com/manticoresoftware/manticoresearch/issues/469) - SELECT 导致崩溃转储
  - [e8420cc7](https://github.com/manticoresoftware/manticoresearch/tree/e8420cc789149c84731be0874be916e33b263bd6) - 过滤树的堆栈大小检测
- [Issue #461](https://github.com/manticoresoftware/manticoresearch/issues/461) - 使用 IN 条件的更新未正确生效
- [Issue #464](https://github.com/manticoresoftware/manticoresearch/issues/464) - 在 CALL PQ 之后立即执行 SHOW STATUS 返回 - [Issue #481](https://github.com/manticoresoftware/manticoresearch/issues/481) - 修复静态二进制构建
- [Issue #502](https://github.com/manticoresoftware/manticoresearch/issues/502) - 多查询中的错误
- [Issue #514](https://github.com/manticoresoftware/manticoresearch/issues/514) - 使用 'create table' 时无法使用不寻常的列名
- [Commit d1db](https://github.com/manticoresoftware/manticoresearch/commit/d1dbe771288328c963ca8512f564c6b7cf4f466c) - 在重放带有字符串属性更新的 binlog 时守护进程崩溃；将 binlog 版本设置为 10
- [Commit 775d](https://github.com/manticoresoftware/manticoresearch/commit/775d0555562f121911d3c30085947e743fadac2d)  - 修复表达式堆栈帧检测运行时错误（测试 207）
- [Commit 4795](https://github.com/manticoresoftware/manticoresearch/commit/4795dc49194c7745761bb5006ecb266e8a9224fa)  - 空存储查询的 percolate 索引过滤器和标签为空（测试 369）
- [Commit c3f0](https://github.com/manticoresoftware/manticoresearch/commit/c3f0bf4dca0e4eae6ea899dd566878df57eba085)  - 在网络延迟长且错误率高的情况下（不同数据中心复制）复制 SST 流中断；更新复制命令版本至 1.03
- [Commit ba2d](https://github.com/manticoresoftware/manticoresearch/commit/ba2d6619b8107f3fb177741bbdff3709bac248cd)  - 加入集群后写操作时 joiner 锁定集群（测试 385）
- [Commit de4d](https://github.com/manticoresoftware/manticoresearch/commit/de4dcb9ff6c316bc2a9ef6c62eb64769f04ee103)  - 使用 exact 修饰符的通配符匹配（测试 321）
- [Commit 6524](https://github.com/manticoresoftware/manticoresearch/commit/6524fc6af032a0932bd42c37874a619e1fb7f075)  - docid 检查点与 docstore
- [Commit f4ab](https://github.com/manticoresoftware/manticoresearch/commit/f4ab83c291e65a5873d271c67b5491f629544f1d)  - 解析无效 xml 时索引器行为不一致
- [Commit 7b72](https://github.com/manticoresoftware/manticoresearch/commit/7b727e22fe01b1a180d4a8d3a3fea92c18eac707)  - 带 NOTNEAR 的存储 percolate 查询无限运行（测试 349）
- [Commit 812d](https://github.com/manticoresoftware/manticoresearch/commit/812dab745f0c972ecde5d5b3d4a313cd6e1e143e)  - 以通配符开头的短语权重错误
- [Commit 1771](https://github.com/manticoresoftware/manticoresearch/commit/1771afc669c75670b3511bf02ab9bfc8982b94d2)  - 带通配符的 percolate 查询生成无 payload 的匹配词导致交错命中并破坏匹配（测试 417）
- [Commit aa0d](https://github.com/manticoresoftware/manticoresearch/commit/aa0d8c2bcb41428d41dbfb66e42a872a79a4ee4b)  - 修正并行查询情况下 'total' 的计算
- [Commit 18d8](https://github.com/manticoresoftware/manticoresearch/commit/18d81b3cfbb930b7361dae645fe13283d4fbf6ef)  - Windows 下守护进程多并发会话崩溃
- [Commit 8443](https://github.com/manticoresoftware/manticoresearch/commit/84432f23d6fbf7a7d0b67c8c9412eb5933b03596)  - 某些索引设置无法复制
- [Commit 9341](https://github.com/manticoresoftware/manticoresearch/commit/93411fe6af4246ca4acce758944d513a1660b92b)  - 在高频率添加新事件时，netloop 有时冻结，因为原子 'kick' 事件被一次处理多个事件，导致丢失实际操作
查询状态，而非服务器状态
- [Commit d805](https://github.com/manticoresoftware/manticoresearch/commit/d805fc125244ef08d14fb3d1826e2c356862d539)  - 新刷新磁盘块可能在提交时丢失
- [Commit 63cb](https://github.com/manticoresoftware/manticoresearch/commit/63cbf008d50979cdf7062664fb02a672a3bb1131)  - profiler 中 'net_read' 不准确
- [Commit f537](https://github.com/manticoresoftware/manticoresearch/commit/f5379bb27234b25919966c50ccb6f469ead646e2)  - Percolate 处理阿拉伯语（从右到左文本）的问题
- [Commit 49ee](https://github.com/manticoresoftware/manticoresearch/commit/49eeb4202d4ce4875c07c35087d7108c1e177328)  - 重复列名时 id 未正确选取
- [Commit refa](https://github.com/manticoresoftware/manticoresearch/commit/4b7df262bbfec9b9e95a44e72ad326ac8a786c49) 修复网络事件在罕见情况下崩溃
- [e8420cc7](https://github.com/manticoresoftware/manticoresearch/tree/e8420cc789149c84731be0874be916e33b263bd6) 修复 `indextool --dumpheader`
- [Commit ff71](https://github.com/manticoresoftware/manticoresearch/commit/ff7163532ed39042c0d3c3c2b902ff41f94d7f0c) - TRUNCATE WITH RECONFIGURE 在存储字段上工作异常

### 重大变更：
- 新 binlog 格式：升级前需干净停止 Manticore
- 索引格式略有变化：新版本可正常读取现有索引，但若从 3.6.0 降级到旧版本，新索引将无法读取
- 复制格式变更：不要从旧版本复制到 3.6.0 或反向，需同时切换所有节点到新版本
- `reverse_scan` 已弃用。确保自 3.6.0 起查询中不使用此选项，否则会失败
- 本版本起不再提供 RHEL6、Debian Jessie 和 Ubuntu Trusty 的构建。如需支持，请[联系我们](https://manticoresearch.com/contact-us/)

### 弃用
- 不再隐式按 id 排序。如依赖此功能，请相应更新查询
- 搜索选项 `reverse_scan` 已弃用

## 版本 3.5.4，2020年12月10日

### 新功能
- 新的 Python、Javascript 和 Java 客户端现已正式发布，并在本手册中有详细文档。
- 实时索引的磁盘块自动丢弃。此优化使得在对实时索引进行[OPTIMIZE](Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE)操作时，当磁盘块显然不再需要（所有文档都被删除）时，可以自动丢弃该磁盘块。之前仍然需要合并，现在可以立即丢弃该磁盘块。[cutoff](Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks)选项被忽略，即使实际上没有合并，过时的磁盘块也会被移除。这在您维护索引中的保留策略并删除较旧文档时非常有用。现在压缩此类索引将更快。
- [standalone NOT](Searching/Options.md#not_terms_only_allowed) 作为 SELECT 的一个选项

### 小改动
- [Issue #453](https://github.com/manticoresoftware/manticoresearch/issues/453) 新选项 [indexer.ignore_non_plain=1](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#ignore_non_plain) 在运行 `indexer --all` 且配置文件中不仅有普通索引时非常有用。没有设置 `ignore_non_plain=1` 会收到警告和相应的退出代码。
- [SHOW PLAN ... OPTION format=dot](Node_info_and_management/Profiling/Query_plan.md#Dot-format-for-SHOW-PLAN) 和 [EXPLAIN QUERY ... OPTION format=dot](Searching/Full_text_matching/Profiling.md#Profiling-without-running-a-query) 支持全文查询计划执行的可视化。对理解复杂查询很有帮助。

### 弃用
- `indexer --verbose` 已弃用，因为它从未为 indexer 输出添加任何内容
- 用于转储 watchdog 的回溯信号现在应使用 `USR2`，而非 `USR1`

### Bug修复
- [Issue #423](https://github.com/manticoresoftware/manticoresearch/issues/423) 西里尔字符句点调用片段保留模式不高亮
- [Issue #435](https://github.com/manticoresoftware/manticoresearch/issues/435) RTINDEX - GROUP N BY 表达式选择导致致命崩溃
- [Commit 2b3b](https://github.com/manticoresoftware/manticoresearch/commit/2b3b62bd0ce2b54544733d524a169e81f71b5388) 集群中 searchd 状态显示段错误
- [Commit 9dd2](https://github.com/manticoresoftware/manticoresearch/commit/9dd25c193ed2a53750a0e0e68566fd2b833fcb0c) 'SHOW INDEX index.N SETTINGS' 不支持大于9的块
- [Issue #389](https://github.com/manticoresoftware/manticoresearch/issues/389) 导致 Manticore 崩溃的 Bug
- [Commit fba1](https://github.com/manticoresoftware/manticoresearch/commit/fba166178ff35c73a0c70d131cf3aca4a635e961) 转换器创建了损坏的索引
- [Commit eecd](https://github.com/manticoresoftware/manticoresearch/commit/eecd61d861c742471175be19a5e5455034b5db5f) stopword_step=0 与 CALL SNIPPETS() 的问题
- [Commit ea68](https://github.com/manticoresoftware/manticoresearch/commit/ea6850e449601199190f38a387c04b8a7b7e90f6) 本地索引在低 max_matches 时 count distinct 返回 0
- [Commit 362f](https://github.com/manticoresoftware/manticoresearch/commit/362f27db3421f09a41c6cce04b5baace00fa4c4d) 使用聚合时，命中结果中不返回存储的文本


## 版本 3.5.2，2020年10月1日

### 新功能

* OPTIMIZE 将磁盘块减少到一定数量（默认是 `2* 核心数`），而非合并为单个块。最优块数可通过 [cutoff](Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks) 选项控制。
* NOT 操作符现在可以单独使用。默认禁用，因为意外的单独 NOT 查询可能很慢。可通过设置新的 searchd 指令 [not_terms_only_allowed](Server_settings/Searchd.md#not_terms_only_allowed) 为 `0` 启用。
* 新设置 [max_threads_per_query](Server_settings/Searchd.md#max_threads_per_query) 设置查询可使用的最大线程数。如果未设置该指令，查询可使用的线程数上限为 [threads](Server_settings/Searchd.md#threads) 的值。
每个 `SELECT` 查询可通过 [OPTION threads=N](Searching/Options.md#threads) 限制线程数，覆盖全局 `max_threads_per_query`。
* 现在可以通过 [IMPORT TABLE](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Importing_table.md) 导入 Percolate 索引。
* HTTP API `/search` 新增对[分面](Searching/Faceted_search.md#HTTP-JSON)/[分组](Searching/Grouping.md)的基本支持，通过新的查询节点 `aggs` 实现。

### 小改动

* 如果未声明复制监听指令，引擎将尝试使用定义的 'sphinx' 端口之后的端口，最多到200。
* `listen=...:sphinx` 需要显式设置以支持 SphinxSE 连接或 SphinxAPI 客户端。
* [SHOW INDEX STATUS](Node_info_and_management/Table_settings_and_status/SHOW_TABLE_STATUS.md) 输出新增指标：`killed_documents`、`killed_rate`、`disk_mapped_doclists`、`disk_mapped_cached_doclists`、`disk_mapped_hitlists` 和 `disk_mapped_cached_hitlists`。
* SQL 命令 `status` 现在输出 `Queue\Threads` 和 `Tasks\Threads`。

### 弃用：

* `dist_threads` 现已完全弃用，searchd 如果仍使用该指令将记录警告。

### Docker

官方 Docker 镜像现基于 Ubuntu 20.04 LTS

### 打包

除了常规的 `manticore` 包外，您还可以按组件安装 Manticore Search：

- `manticore-server-core` - 提供 `searchd`、手册页、日志目录、API 和 galera 模块。它还会作为依赖安装 `manticore-common`。
- `manticore-server` - 提供核心的自动化脚本（init.d、systemd）和 `manticore_new_cluster` 包装器。它还会作为依赖安装 `manticore-server-core`。
- `manticore-common` - 提供配置、停用词、通用文档和骨架文件夹（datadir、modules 等）。
- `manticore-tools` - 提供辅助工具（`indexer`、`indextool` 等）、它们的手册页和示例。它还会作为依赖安装 `manticore-common`。
- `manticore-icudata`（RPM）或 `manticore-icudata-65l`（DEB） - 提供 ICU 数据文件以供 icu 形态学使用。
- `manticore-devel` (RPM) 或 `manticore-dev` (DEB) - 提供 UDF 的开发头文件。

### Bug修复

1. [提交 2a47](https://github.com/manticoresoftware/manticoresearch/commit/2a474dc1a26e8b0f8aaaae95669caf2f1d4b7746) RT索引中不同块的grouper守护进程崩溃
2. [提交 57a1](https://github.com/manticoresoftware/manticoresearch/commit/57a19e5ad5663ef0ca7436595218fb1221d28c8e) 空远程文档的快速路径
3. [提交 07dd](https://github.com/manticoresoftware/manticoresearch/commit/07dd3f313c63fb82c22092f9907ef24e3475250e) 表达式堆栈帧检测运行时
4. [提交 08ae](https://github.com/manticoresoftware/manticoresearch/commit/08ae357cf1012bc8e2da54c20b205b592efda3d4) percolate索引中匹配超过32个字段
5. [提交 16b9](https://github.com/manticoresoftware/manticoresearch/commit/16b9390fd4cdb07a77ac4497adb935573a1710e5) 复制监听端口范围
6. [提交 5fa6](https://github.com/manticoresoftware/manticoresearch/commit/5fa671affeacb6441a59c8a88479bfd423df7c81) 在pq上显示创建表
7. [提交 54d1](https://github.com/manticoresoftware/manticoresearch/commit/54d133b6449105a9fb0168db3f1fbb05fb5aa1f6) HTTPS端口行为
8. [提交 fdbb](https://github.com/manticoresoftware/manticoresearch/commit/fdbbe5245cc296cc5c1ae3ae2fb9cb08fb66a248) 替换时混合docstore行
9. [提交 afb5](https://github.com/manticoresoftware/manticoresearch/commit/afb53f648ef4e64fa1776c58e66f6e716ac730ad) 将TFO不可用消息级别切换为“info”
10. [提交 59d9](https://github.com/manticoresoftware/manticoresearch/commit/59d94cefc6e88af360b8046726a16b7eaa9f0b71) strcmp无效使用导致崩溃
11. [提交 04af](https://github.com/manticoresoftware/manticoresearch/commit/04af0349c1e784c2230d204592d6272b440bb375) 向带有系统（停用词）文件的集群添加索引
12. [提交 5014](https://github.com/manticoresoftware/manticoresearch/commit/50148b4eae9f3a924082005cf5d2f9c8339af7f5) 合并带有大型字典的索引；RT优化大型磁盘块
13. [提交 a2ad](https://github.com/manticoresoftware/manticoresearch/commit/a2adf1582843f63d4dcc1dab8f7598e0917094a2) Indextool可以从当前版本转储元数据
14. [提交 69f6](https://github.com/manticoresoftware/manticoresearch/commit/69f6d5f77c64b97e3f395aea8c555b11c77fb732) GROUP N中组顺序问题
15. [提交 24d5](https://github.com/manticoresoftware/manticoresearch/commit/24d5d80f508576794b751d70f9f9b524564e3c14) 握手后对SphinxSE显式刷新
16. [提交 31c4](https://github.com/manticoresoftware/manticoresearch/commit/31c4d78ac03ee85d9a02b86075448a04ac80f9e2) 避免不必要时复制巨大描述
17. [提交 2959](https://github.com/manticoresoftware/manticoresearch/commit/2959e2caa8610f98fcdf2e5a82160f58687f4a27) show threads中的负时间
18. [提交 f0b3](https://github.com/manticoresoftware/manticoresearch/commit/f0b357102c70f3ac1d7da260c60b67322940e647) 令牌过滤插件与零位置增量
19. [提交 a49e](https://github.com/manticoresoftware/manticoresearch/commit/a49e5bc13ea763e6f01dd5993e891858a84d1846) 多重命中时将“FAIL”改为“WARNING”

## 版本 3.5.0，2020年7月22日

### 主要新特性：
* 此版本耗时较长，因为我们一直在努力将多任务模式从线程改为**协程**。这使配置更简单，查询并行化更直接：Manticore只使用给定数量的线程（参见新设置 [threads](Server_settings/Searchd.md#threads)），新模式确保以最优方式完成。
* [高亮](Searching/Highlighting.md#Highlighting-options)的变化：
  - 任何支持多个字段的高亮（`highlight({},'field1, field2'`）或json查询中的`highlight`）现在默认按字段应用限制。
  - 任何支持纯文本的高亮（`highlight({}, string_attr)`或`snippet()`）现在对整个文档应用限制。
  - [每字段限制](Searching/Highlighting.md#limits_per_field)可以通过`limits_per_field=0`选项切换为全局限制（默认是`1`）。
  - [allow_empty](Searching/Highlighting.md#allow_empty)现在通过HTTP JSON高亮默认是`0`。

* 同一端口[现在可以用于](Server_settings/Searchd.md#listen) http、https和二进制API（以接受来自远程Manticore实例的连接）。通过mysql协议连接仍需`listen = *:mysql`。Manticore现在自动检测尝试连接的客户端类型，MySQL除外（由于协议限制）。

* 在RT模式下，字段现在可以同时是[文本和字符串属性](Creating_a_table/Data_types.md#String) - [GitHub issue #331](https://github.com/manticoresoftware/manticoresearch/issues/331)。

  在[普通模式](Read_this_first.md#Real-time-mode-vs-plain-mode)中称为`sql_field_string`。现在它也可用于[RT模式](Read_this_first.md#Real-time-mode-vs-plain-mode)的实时索引。示例如下：

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
* SQL接口支持SSL和压缩
* 支持mysql客户端的[`status`](Node_info_and_management/Node_status.md#STATUS)命令。
* [复制](Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Setting-up-replication)现在可以复制外部文件（停用词、例外等）。
* 过滤操作符 [`in`](Searching/Filters.md#Set-filters) 现在可以通过 HTTP JSON 接口使用。
* HTTP JSON 中的 [`expressions`](Searching/Expressions.md#expressions)。
* [现在可以在 RT 模式下动态更改 `rt_mem_limit`](https://github.com/manticoresoftware/manticoresearch/issues/344)，即可以执行 `ALTER ... rt_mem_limit=<new value>`。
* 现在可以使用 [单独的 CJK 字符集表](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology)：`chinese`、`japanese` 和 `korean`。
* [thread_stack](Server_settings/Searchd.md#thread_stack) 现在限制最大线程栈大小，而非初始大小。
* 改进了 `SHOW THREADS` 输出。
* 在 `SHOW THREADS` 中显示长时间 `CALL PQ` 的进度。
* cpustat、iostat、coredump 可以在运行时通过 [SET](Server_settings/Setting_variables_online.md#SET) 更改。
* 实现了 `SET [GLOBAL] wait_timeout=NUM`，

### 重大变更：
* **索引格式已更改。** 3.5.0 版本构建的索引无法被低于 3.5.0 版本的 Manticore 加载，但 Manticore 3.5.0 可以识别旧格式。
* [`INSERT INTO PQ VALUES()`](Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md)（即不提供列列表）之前期望的值正好是 `(query, tags)`。现已更改为 `(id,query,tags,filters)`。如果希望自动生成 id，可以将其设置为 0。
* [`allow_empty=0`](Searching/Highlighting.md#allow_empty) 是通过 HTTP JSON 接口高亮显示的新默认值。
* `CREATE TABLE`/`ALTER TABLE` 中外部文件（停用词、例外等）只允许使用绝对路径。

### 弃用：
* `ram_chunks_count` 在 `SHOW INDEX STATUS` 中重命名为 `ram_chunk_segments_count`。
* `workers` 已废弃。现在只有一种 workers 模式。
* `dist_threads` 已废弃。所有查询现在尽可能并行（受 `threads` 和 `jobs_queue_size` 限制）。
* `max_children` 已废弃。使用 [threads](Server_settings/Searchd.md#threads) 设置 Manticore 使用的线程数（默认设置为 CPU 核心数）。
* `queue_max_length` 已废弃。如果确实需要，请使用 [jobs_queue_size](Server_settings/Searchd.md#jobs_queue_size) 来微调内部作业队列大小（默认无限制）。
* 所有 `/json/*` 端点现在可在无 `/json/` 前缀的情况下访问，例如 `/search`、`/insert`、`/delete`、`/pq` 等。
* `field` 表示“全文字段”的含义在 `describe` 中重命名为 “text”。
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
* 西里尔字母 `и` 在 `non_cjk` charset_table（默认）中不再映射为 `i`，因为这对俄语词干提取器和词形还原器影响过大。
* `read_timeout`。请改用 [network_timeout](Server_settings/Searchd.md#network_timeout)，它同时控制读写超时。


### 软件包

* Ubuntu Focal 20.04 官方软件包
* deb 软件包名称由 `manticore-bin` 改为 `manticore`

### Bug 修复：
1. [Issue #351](https://github.com/manticoresoftware/manticoresearch/issues/351) searchd 内存泄漏
2. [Commit ceab](https://github.com/manticoresoftware/manticoresearch/commit/ceabe44f) 片段中轻微越界读取
3. [Commit 1c3e](https://github.com/manticoresoftware/manticoresearch/commit/1c3e84a3) 危险地写入局部变量导致查询崩溃
4. [Commit 26e0](https://github.com/manticoresoftware/manticoresearch/commit/26e094ab) 测试 226 中排序器的轻微内存泄漏
5. [Commit d2c7](https://github.com/manticoresoftware/manticoresearch/commit/d2c7f86a) 测试 226 中严重内存泄漏
6. [Commit 0dd8](https://github.com/manticoresoftware/manticoresearch/commit/0dd80122) 集群显示节点同步，但 `count(*)` 显示数字不同
7. [Commit f1c1](https://github.com/manticoresoftware/manticoresearch/commit/f1c1ac3f) 美观：日志中重复且有时丢失的警告信息
8. [Commit f1c1](https://github.com/manticoresoftware/manticoresearch/commit/f1c1ac3f) 美观：日志中出现 (null) 索引名
9. [Commit 359d](https://github.com/manticoresoftware/manticoresearch/commit/359dbd30) 无法检索超过 7000 万条结果
10. [Commit 19f3](https://github.com/manticoresoftware/manticoresearch/commit/19f328ee) 无法使用无列语法插入 PQ 规则
11. [Commit bf68](https://github.com/manticoresoftware/manticoresearch/commit/bf685d5d) 向集群中的索引插入文档时错误信息误导
12. [Commit 2cf1](https://github.com/manticoresoftware/manticoresearch/commit/2cf18c83) `/json/replace` 和 `json/update` 返回的 id 为指数形式
13. [Issue #324](https://github.com/manticoresoftware/manticoresearch/issues/324) 在同一查询中更新 json 标量属性和 mva
14. [Commit d384](https://github.com/manticoresoftware/manticoresearch/commit/d38409eb) `hitless_words` 在 RT 模式下不起作用
15. [Commit 5813](https://github.com/manticoresoftware/manticoresearch/commit/5813d639) RT 模式下应禁止 `ALTER RECONFIGURE`
16. [Commit 5813](https://github.com/manticoresoftware/manticoresearch/commit/5813d639) searchd 重启后 `rt_mem_limit` 被重置为 128M
17. highlight() 有时会挂起
18. [Commit 7cd8](https://github.com/manticoresoftware/manticoresearch/commit/7cd878f4) RT 模式下无法使用 U+ 代码
19. [Commit 2b21](https://github.com/manticoresoftware/manticoresearch/commit/2b213de4) RT 模式下无法在 wordforms 中使用通配符
20. [Commit e9d0](https://github.com/manticoresoftware/manticoresearch/commit/e9d07e68) 修复 `SHOW CREATE TABLE` 与多个 wordform 文件的兼容问题
21. [提交 fc90](https://github.com/manticoresoftware/manticoresearch/commit/fc90a84f) JSON 查询缺少 "query" 导致 searchd 崩溃
22. Manticore [官方 docker](https://hub.docker.com/r/manticoresearch/manticore) 无法从 mysql 8 索引数据
23. [提交 23e0](https://github.com/manticoresoftware/manticoresearch/commit/23e05d32) HTTP /json/insert 需要 id
24. [提交 bd67](https://github.com/manticoresoftware/manticoresearch/commit/bd679af0) `SHOW CREATE TABLE` 对 PQ 无效
25. [提交 bd67](https://github.com/manticoresoftware/manticoresearch/commit/bd679af0) `CREATE TABLE LIKE` 对 PQ 工作不正常
26. [提交 5eac](https://github.com/manticoresoftware/manticoresearch/commit/5eacf28f) show index status 中设置的行尾问题
27. [提交 cb15](https://github.com/manticoresoftware/manticoresearch/commit/cb153228) HTTP JSON 响应中 "highlight" 的标题为空
28. [问题 #318](https://github.com/manticoresoftware/manticoresearch/issues/318) `CREATE TABLE LIKE` 中缀错误
29. [提交 9040](https://github.com/manticoresoftware/manticoresearch/commit/9040d22c) RT 在负载下崩溃
30. [cd512c7d ](https://github.com/manticoresoftware/manticoresearch/commit/cd512c7d ) RT 磁盘块崩溃时丢失崩溃日志
31. [问题 #323](https://github.com/manticoresoftware/manticoresearch/issues/323) 导入表失败并关闭连接
32. [提交 6275](https://github.com/manticoresoftware/manticoresearch/commit/6275316a) `ALTER reconfigure` 损坏 PQ 索引
33. [提交 9c1d](https://github.com/manticoresoftware/manticoresearch/commit/9c1d221e) 更改索引类型后 searchd 重载问题
34. [提交 71e2](https://github.com/manticoresoftware/manticoresearch/commit/71e2b5bb) 守护进程在导入表时因缺失文件崩溃
35. [问题 #322](https://github.com/manticoresoftware/manticoresearch/issues/322) 使用多个索引、group by 和 ranker = none 的 select 导致崩溃
36. [提交 c3f5](https://github.com/manticoresoftware/manticoresearch/commit/c3f58490) `HIGHLIGHT()` 不会高亮字符串属性
37. [问题 #320](https://github.com/manticoresoftware/manticoresearch/issues/320) `FACET` 无法对字符串属性排序
38. [提交 4f1a](https://github.com/manticoresoftware/manticoresearch/commit/4f1a1f25) 缺少数据目录时的错误
39. [提交 04f4](https://github.com/manticoresoftware/manticoresearch/commit/04f4ddd4) access_* 在 RT 模式下不支持
40. [提交 1c06](https://github.com/manticoresoftware/manticoresearch/commit/1c0616a2) 字符串中的坏 JSON 对象：1. `CALL PQ` 当 json 超过某个值时返回 "Bad JSON objects in strings: 1"。
41. [提交 32f9](https://github.com/manticoresoftware/manticoresearch/commit/32f943d6) RT 模式不一致。在某些情况下无法删除索引因为未知，且无法创建因为目录非空。
42. [问题 #319](https://github.com/manticoresoftware/manticoresearch/issues/319) select 时崩溃
43. [提交 22a2](https://github.com/manticoresoftware/manticoresearch/commit/22a28dd7) `max_xmlpipe2_field` = 2M 在 2M 字段时返回警告
44. [问题 #342](https://github.com/manticoresoftware/manticoresearch/issues/342) 查询条件执行错误
45. [提交 dd8d](https://github.com/manticoresoftware/manticoresearch/commit/dd8dcab2) 简单的两个词搜索找到只包含一个词的文档
46. [提交 9091](https://github.com/manticoresoftware/manticoresearch/commit/90919e62) PQ 中无法匹配键中带大写字母的 json
47. [提交 56da](https://github.com/manticoresoftware/manticoresearch/commit/56da086a) 索引器在 csv+docstore 上崩溃
48. [问题 #363](https://github.com/manticoresoftware/manticoresearch/issues/363) 在 centos 7 中 json 属性使用 `[null]` 导致插入数据损坏
49. 重大 [问题 #345](https://github.com/manticoresoftware/manticoresearch/issues/345) 记录未被插入，count() 随机，"replace into" 返回 OK
50. max_query_time 过度降低 SELECT 速度
51. [问题 #352](https://github.com/manticoresoftware/manticoresearch/issues/352) Mac OS 上主从通信失败
52. [问题 #328](https://github.com/manticoresoftware/manticoresearch/issues/328) 使用 Connector.Net/Mysql 8.0.19 连接 Manticore 时出错
53. [提交 daa7](https://github.com/manticoresoftware/manticoresearch/commit/daa760d2fe83668038318f0f384ceefc0d075e18) 修正 \0 转义并优化性能
54. [提交 9bc5](https://github.com/manticoresoftware/manticoresearch/commit/9bc5c01a493db993b90ff6c1ad51aeae028f629c) 修正 count distinct 与 json 的问题
55. [提交 4f89](https://github.com/manticoresoftware/manticoresearch/commit/4f89a965a3af0db548ae9d1763731ebaebe21f5a) 修正其他节点上 drop table 失败
56. [提交 952a](https://github.com/manticoresoftware/manticoresearch/commit/952af5a5931fd72b87edc169588777778b365231) 修复紧密运行 call pq 时崩溃


## 版本 3.4.2，2020年4月10日
### 关键错误修复
* [提交 2ffe](https://github.com/manticoresoftware/manticoresearch/commit/2ffe2d261793f9681265bc4abe38d695e72c121e) 修复旧版本 RT 索引无法索引数据的问题

## 版本 3.4.0，2020年3月26日
### 重大变更
* 服务器工作在两种模式：rt 模式和普通模式
   *   rt 模式需要 data_dir，且配置中无索引定义
   *   普通模式中索引在配置中定义；不允许 data_dir
* 复制仅在 rt 模式下可用

### 次要变更
* charset_table 默认为 non_cjk 别名
* rt 模式下全文字段默认被索引和存储
* rt 模式下全文字段名称由 'field' 改为 'text'
* ALTER RTINDEX 改名为 ALTER TABLE
* TRUNCATE RTINDEX 改名为 TRUNCATE TABLE

### 新功能
* 仅存储字段
* SHOW CREATE TABLE，IMPORT TABLE

### 改进
* 更快的无锁 PQ
* /sql 可在 mode=raw 下执行任何类型的 SQL 语句
* mysql41 协议的 mysql 别名
* data_dir 中默认的 state.sql

### 错误修复
* [提交 a533](https://github.com/manticoresoftware/manticoresearch/commit/a5333644e8e0c69746a9f24da8221fc965478e4a) 修复 highlight() 中错误字段语法导致的崩溃
* [提交 7fbb](https://github.com/manticoresoftware/manticoresearch/commit/7fbb9f2e618096289cba7f8b834530b46aa94055) 修复带有 docstore 的 RT 索引复制时服务器崩溃
* [提交 24a0](https://github.com/manticoresoftware/manticoresearch/commit/24a04687eb43b4f4719c7c4a5fe3c19099240467) 修复对带有 infix 或 prefix 选项且未启用存储字段的索引进行高亮时崩溃
* [提交 3465](https://github.com/manticoresoftware/manticoresearch/commit/3465c1ce9c860c09f6ccc5b83a63e3c68c241d8d) 修复空索引时关于空 docstore 和 doc-id 查找的错误提示
* [提交 a707](https://github.com/manticoresoftware/manticoresearch/commit/b8601b4144b9092be43a7caa7cd6b19cb37e14c7) 修复 #314 SQL insert 命令末尾带分号的问题
* [提交 9562](https://github.com/manticoresoftware/manticoresearch/commit/95628c9b9b8f16fe1bd18599a84e9673575adf1a) 移除查询词不匹配的警告
* [提交 b860](https://github.com/manticoresoftware/manticoresearch/commit/b8601b4144b9092be43a7caa7cd6b19cb37e14c7) 修复通过 ICU 分词的片段查询
* [提交 5275](https://github.com/manticoresoftware/manticoresearch/commit/5275516ca4631c584e6e0127fa161a18d2658fd0) 修复 docstore 块缓存中的查找/添加竞态条件
* [提交 f06e](https://github.com/manticoresoftware/manticoresearch/commit/f06ef97af5c7390427e82525c150535290e42c3e) 修复 docstore 中的内存泄漏
* [提交 a725](https://github.com/manticoresoftware/manticoresearch/commit/a7258ba8340e246c0d8aaeb4a7a74129eba807eb) 修复 #316 INSERT 时 LAST_INSERT_ID 返回空值
* [提交 1ebd](https://github.com/manticoresoftware/manticoresearch/commit/1ebd5bf8d74113bcfa138ce63ce11966990a4215) 修复 #317 json/update HTTP 端点支持 MVA 数组和 JSON 属性对象
* [提交 e426](https://github.com/manticoresoftware/manticoresearch/commit/e426950a536382a142e2a42ed207c341149ff685) 修复索引器在未显式指定 id 时转储 rt 的问题

## 版本 3.3.0，2020年2月4日
### 新特性
* 并行实时索引搜索
* EXPLAIN QUERY 命令
* 无索引定义的配置文件（alpha 版本）
* CREATE/DROP TABLE 命令（alpha 版本）
* indexer --print-rt - 可从源读取并打印 Real-Time 索引的 INSERT 语句

### 改进
* 更新至 Snowball 2.0 词干提取器
* SHOW INDEX STATUS 支持 LIKE 过滤器
* 高 max_matches 时改进内存使用
* SHOW INDEX STATUS 为 RT 索引添加 ram_chunks_count
* 无锁优先队列（PQ）
* 将 LimitNOFILE 改为 65536


### Bug 修复
* [提交 9c33](https://github.com/manticoresoftware/manticoresearch/commit/9c33aab8c1fd9059c8644d23d5f58f03010043c6) 添加索引模式重复属性检查 #293
* [提交 a008](https://github.com/manticoresoftware/manticoresearch/commit/a0085f9477e191da9e0e835b91583fb9cfde698e) 修复无命中词导致的崩溃
* [提交 6895](https://github.com/manticoresoftware/manticoresearch/commit/68953740af8dca1833026d0f448c71416ed3044a) 修复 ATTACH 后松散的 docstore
* [提交 d6f6](https://github.com/manticoresoftware/manticoresearch/commit/d6f696edea53ea2b9a68251e4054e4816082c285) 修复分布式设置中的 docstore 问题
* [提交 bce2](https://github.com/manticoresoftware/manticoresearch/commit/bce2b7ec883d208f31094ee42a5b6f01d18a998f) 在排序器中用 OpenHash 替换 FixedHash
* [提交 e0ba](https://github.com/manticoresoftware/manticoresearch/commit/e0baf7392937969e640fb5d10e8ccb312fe36446) 修复索引定义中重复命名的属性
* [提交 ca81](https://github.com/manticoresoftware/manticoresearch/commit/ca81114bf1ac7ea7adcc8509bdc14d1f30b4f461) 修复 HIGHLIGHT() 中的 html_strip
* [提交 493a](https://github.com/manticoresoftware/manticoresearch/commit/493a5e916d70fd4a3f14d1ee487234434f4a7ce3) 修复 HIGHLIGHT() 中的 passage 宏
* [提交 a82d](https://github.com/manticoresoftware/manticoresearch/commit/a82d41c775692f837f267d554d7d4a4b499d69f0) 修复 RT 索引创建小或大磁盘块时的双缓冲问题
* [提交 a404](https://github.com/manticoresoftware/manticoresearch/commit/a404c85de0d41d9d1c75b7540479d8a3af9e61aa) 修复 kqueue 的事件删除
* [提交 8bea](https://github.com/manticoresoftware/manticoresearch/commit/8bea0f6fd1666b817995c4dd6e67f6f1c9161638) 修复 RT 索引 rt_mem_limit 大值时磁盘块保存问题
* [提交 8707](https://github.com/manticoresoftware/manticoresearch/commit/8707f0395b7ace52dc0cba9c8ed92a2bc4392932) 修复索引时浮点溢出
* [提交 a564](https://github.com/manticoresoftware/manticoresearch/commit/a56434ce71bb7714b4d8e75278aaa075b27ece80) 修复插入负 ID 文档到 RT 索引时的错误
* [提交 bbeb](https://github.com/manticoresoftware/manticoresearch/commit/bbebfd7551a68641154761cac962053b54806759) 修复 ranker fieldmask 导致的服务器崩溃
* [提交 3809](https://github.com/manticoresoftware/manticoresearch/commit/3809cc1b0863e3a5a200dc423be3f7c38be56113) 修复使用查询缓存时崩溃
* [提交 dc2a](https://github.com/manticoresoftware/manticoresearch/commit/dc2a585bc724678679e4297362200bbd49ce32bb) 修复使用 RT 索引 RAM 段并行插入时崩溃

## 版本 3.2.2，2019年12月19日
### 新特性
* RT 索引的自增 ID
* 通过新的 HIGHLIGHT() 函数支持 docstore 高亮，也可用于 HTTP API
* SNIPPET() 可使用特殊函数 QUERY() 返回当前 MATCH 查询
* 新增 field_separator 选项用于高亮函数

### 改进和变更
* 远程节点的存储字段延迟获取（可显著提升性能）
* 字符串和表达式不再破坏多查询和 FACET 优化
* RHEL/CentOS 8 构建现在使用 mariadb-connector-c-devel 中的 mysql libclient
* ICU 数据文件随软件包一起发布，移除 icu_data_dir
* systemd 服务文件包含 'Restart=on-failure' 策略
* indextool 现在可以在线检查实时索引
* 默认配置文件现在为 /etc/manticoresearch/manticore.conf
* RHEL/CentOS 上的服务从 'searchd' 重命名为 'manticore'
* 移除了 query_mode 和 exact_phrase snippet 的选项

### Bug修复
* [Commit 6ae4](https://github.com/manticoresoftware/manticoresearch/commit/6ae474c7894a6bee222d5b18e59a44fdbf57843a) 修复通过 HTTP 接口执行 SELECT 查询时崩溃的问题
* [Commit 5957](https://github.com/manticoresoftware/manticoresearch/commit/59577513a49eac5a4a3c5e2cb38394d3246b5d35) 修复 RT 索引保存磁盘块但未标记某些文档为已删除的问题
* [Commit e861](https://github.com/manticoresoftware/manticoresearch/commit/e861f0fca0e88924450695d4e9d6acff7a36558a) 修复使用 dist_threads 搜索多索引或多查询时崩溃的问题
* [Commit 4409](https://github.com/manticoresoftware/manticoresearch/commit/440991fc977b8479800b45cdbc862eeb1ba5d965) 修复对包含宽 UTF8 码点的长词生成中缀时崩溃的问题
* [Commit 5fd5](https://github.com/manticoresoftware/manticoresearch/commit/5fd599b48bba527c023e6aa0b262dca51ffb8a1c) 修复添加套接字到 IOCP 时的竞态条件
* [Commit cf10](https://github.com/manticoresoftware/manticoresearch/commit/cf10d7d3589f77adfaefbe52a51777bc9c67cf99) 修复布尔查询与 JSON 选择列表的问题
* [Commit 996d](https://github.com/manticoresoftware/manticoresearch/commit/996de77f4c3ec103d965e28c36098fd07dba50c4) 修复 indextool 检查错误跳过列表偏移和 doc2row 查找的检查
* [Commit 6e3f](https://github.com/manticoresoftware/manticoresearch/commit/6e3fc9e88941c9427410f9d0adeebb7dd09a900f) 修复索引器在大数据上产生带有负跳过列表偏移的错误索引
* [Commit faed](https://github.com/manticoresoftware/manticoresearch/commit/faed3220b423a96401982bf47117edf1f62c584e) 修复 JSON 仅将数字转换为字符串以及表达式中 JSON 字符串转换为数字的问题
* [Commit 5331](https://github.com/manticoresoftware/manticoresearch/commit/533197200249d648ebcbdaca5d605861ee566348) 修复 indextool 在命令行设置多个命令时以错误代码退出的问题
* [Commit 7955](https://github.com/manticoresoftware/manticoresearch/commit/795520ac351d22e4497ebaf6c249bf25081842b6) 修复 #275 磁盘空间不足错误时 binlog 状态无效的问题
* [Commit 2284](https://github.com/manticoresoftware/manticoresearch/commit/2284da5e3ff77bf7be4038a4d950c0580e4159c8) 修复 #279 IN 过滤器应用于 JSON 属性时崩溃的问题
* [Commit ce2e](https://github.com/manticoresoftware/manticoresearch/commit/ce2e4b4765111a7d331ffc256911f12770a8942d) 修复 #281 错误的管道关闭调用
* [Commit 5355](https://github.com/manticoresoftware/manticoresearch/commit/535589ba8a37baf5c6056afb5ff969fd70476feb) 修复服务器在调用 PQ 时因递归 JSON 属性编码为字符串而挂起的问题
* [Commit a5fc](https://github.com/manticoresoftware/manticoresearch/commit/a5fc8a36e1644ee051632408ab8b53e79aeb9ff4) 修复 multiand 节点中文档列表超出末尾的问题
* [Commit a362](https://github.com/manticoresoftware/manticoresearch/commit/a3628617ef4ee89a811d4ba2c4a04cbbc298c1b5) 修复线程公共信息的检索
* [Commit f8d2](https://github.com/manticoresoftware/manticoresearch/commit/f8d2d7bbbb90af3d2ee97424f97b05927f09d42b) 修复文档存储缓存锁

## 版本 3.2.0，2019年10月17日
### 新特性
* 文档存储
* 新指令 stored_fields、docstore_cache_size、docstore_block_size、docstore_compression、docstore_compression_level

### 改进和变更
* 改进了 SSL 支持
* 更新了 non_cjk 内置字符集
* 禁用 UPDATE/DELETE 语句在查询日志中记录为 SELECT
* 支持 RHEL/CentOS 8 软件包

### Bug修复
* [Commit 301a](https://github.com/manticoresoftware/manticoresearch/commit/301a806b14725e842300ee225521407464374c41) 修复在 RT 索引的磁盘块中替换文档时崩溃的问题
* [Commit 46c1](https://github.com/manticoresoftware/manticoresearch/commit/46c1cad8fa11c1fc09cc34b20d978fdfcd4e6d27) 修复 #269 LIMIT N OFFSET M
* [Commit 92a4](https://github.com/manticoresoftware/manticoresearch/commit/92a46edaa196ef33f3ac2225fbdba2c2b88ee45d) 修复 DELETE 语句中显式设置 id 或提供 id 列表时跳过搜索的问题
* [Commit 8ca7](https://github.com/manticoresoftware/manticoresearch/commit/8ca78c138d837caab30dc6e2343a3c4d1687bb87) 修复 Windows poller 中事件移除后索引错误的问题
* [Commit 6036](https://github.com/manticoresoftware/manticoresearch/commit/603631e2bff5cb0acf25f68b85621742490eccd9) 修复通过 HTTP 访问 JSON 时浮点数四舍五入的问题
* [Commit 62f6](https://github.com/manticoresoftware/manticoresearch/commit/62f64cb9e75ec28fc3c29f8414ce9656f5580254) 修复远程片段先检查空路径的问题；修复 Windows 测试
* [Commit aba2](https://github.com/manticoresoftware/manticoresearch/commit/aba274c2c5abc1029894ba9f95b786bda93e8f22) 修复配置重载在 Windows 上的行为与 Linux 一致
* [Commit 6b8c](https://github.com/manticoresoftware/manticoresearch/commit/6b8c4242ef77c3fa4d0ccb7d76d81714b6728f0b) 修复 #194 PQ 支持形态学和词干分析器
* [Commit 174d](https://github.com/manticoresoftware/manticoresearch/commit/174d312905285aa5746f4f404c53e6dc669ef42f) 修复 RT 退役段管理

## 版本 3.1.2，2019年8月22日
### 新特性和改进
* HTTP API 的实验性 SSL 支持
* CALL KEYWORDS 的字段过滤器
* /json/search 的 max_matches
* 默认 Galera gcache.size 的自动调整
* 改进了 FreeBSD 支持

### Bug修复
* [Commit 0a1a](https://github.com/manticoresoftware/manticoresearch/commit/0a1a2c81038794983b4e30c8aefc7cc20e89b03f) 修复 RT 索引复制到存在相同 RT 索引但路径不同的节点的问题
* [Commit 4adc](https://github.com/manticoresoftware/manticoresearch/commit/4adc075294ac823289f745e2cc419f18c7dcf2e2) 修复无活动索引的刷新重新调度
* [Commit d6c0](https://github.com/manticoresoftware/manticoresearch/commit/d6c00a6f750c04021747e9c99e0f77707a4cbfc3) 改进 RT/PQ 索引刷新重新调度
* [Commit d0a7](https://github.com/manticoresoftware/manticoresearch/commit/d0a7c95906b9b5ba33c98267d54501cfe27508d6) 修复 #250 TSV 和 CSV 管道源的 index_field_lengths 索引选项
* [Commit 1266](https://github.com/manticoresoftware/manticoresearch/commit/1266d54800242709fbe2c34e72eb69b6595b23bd) 修复 indextool 对空索引的块索引检查错误报告
* [Commit 553c](https://github.com/manticoresoftware/manticoresearch/commit/553ca73ca111f28e0888d59cd48b0d6df181ecd6) 修复 Manticore SQL 查询日志中空选择列表的问题
* [Commit 56c8](https://github.com/manticoresoftware/manticoresearch/commit/56c8584456293665ddd4ce4c94307d74473a78b9) 修复 indexer -h/--help 响应

## 版本 3.1.0，2019年7月16日
### 功能和改进
* 实时索引的复制
* 中文的 ICU 分词器
* 新的形态学选项 icu_chinese
* 新指令 icu_data_dir
* 复制的多语句事务
* LAST_INSERT_ID() 和 @session.last_insert_id
* SHOW VARIABLES 支持 LIKE 'pattern'
* 多文档插入用于 percolate 索引
* 为配置添加时间解析器
* 内部任务管理器
* 对文档和命中列表组件使用 mlock
* 监狱代码片段路径

### 移除
* 放弃 RLP 库支持，转而使用 ICU；移除所有 rlp\* 指令
* 禁用使用 UPDATE 更新文档 ID

### 修复
* [Commit f047](https://github.com/manticoresoftware/manticoresearch/commit/f047222339fd0e62a07e9ca8b2a829297d5db861) 修复 concat 和 group_concat 中的缺陷
* [Commit b081](https://github.com/manticoresoftware/manticoresearch/commit/b08147ee24712b5eaa51f261626b5d16c6242fdd) 修复 percolate 索引中查询 uid 为 BIGINT 属性类型
* [Commit 4cd8](https://github.com/manticoresoftware/manticoresearch/commit/4cd85afaf76eef0ce9fa1cd073f3fed160279890) 预分配新磁盘块失败时不崩溃
* [Commit 1a55](https://github.com/manticoresoftware/manticoresearch/commit/1a551227886474054111088412f2b2396f485652) 为 ALTER 添加缺失的 timestamp 数据类型
* [Commit f3a8](https://github.com/manticoresoftware/manticoresearch/commit/f3a8e096fd496a6b001d398aadbb781b2dbad929) 修复错误 mmap 读取导致的崩溃
* [Commit 4475](https://github.com/manticoresoftware/manticoresearch/commit/447577115192db7ac7bfea70b5ba58ee2c61229f) 修复复制中集群锁的哈希问题
* [Commit ff47](https://github.com/manticoresoftware/manticoresearch/commit/ff476df955f057bd38e8b6ad6056325e9d6fbb9c) 修复复制中提供者泄漏
* [Commit 58dc](https://github.com/manticoresoftware/manticoresearch/commit/58dcbb779c4c30ca1560460c78e35ba114b83716) 修复 \#246 indexer 中未定义的 sigmask
* [Commit 3dd8](https://github.com/manticoresoftware/manticoresearch/commit/3dd8278e747d534f69fe9c3d360eef188ab7b6d5) 修复 netloop 报告中的竞态条件
* [Commit a02a](https://github.com/manticoresoftware/manticoresearch/commit/a02aae05a3fa3d1caf2821f43c7e237b6de76336) HA 策略重平衡器的零间隙

## 版本 3.0.2，2019年5月31日
### 改进
* 为文档和命中列表添加 mmap 读取器
* `/sql` HTTP 端点响应现在与 `/json/search` 响应相同
* 新指令 `access_plain_attrs`、`access_blob_attrs`、`access_doclists`、`access_hitlists`
* 复制设置中新指令 `server_id`

### 移除
* 移除 HTTP `/search` 端点

### 弃用
* `read_buffer`、`ondisk_attrs`、`ondisk_attrs_default`、`mlock` 被 `access_*` 指令替代

### 修复
* [Commit 849c](https://github.com/manticoresoftware/manticoresearch/commit/849c16e1040f382795ba0635cb48686f9db4f2d8) 允许选择列表中以数字开头的属性名
* [Commit 48e6](https://github.com/manticoresoftware/manticoresearch/commit/48e6c302ca37bec0bc49c79619c60bf787ed0708) 修复 UDF 中的 MVA，修复 MVA 别名
* [Commit 0555](https://github.com/manticoresoftware/manticoresearch/commit/055586a9a2ebfa8874f68099c5bc798a145fd4ef) 修复 \#187 使用 SENTENCE 查询时崩溃
* [Commit 93bf](https://github.com/manticoresoftware/manticoresearch/commit/93bf52f23f1c53c8a93a603387ca5463e1fc2ae8) 修复 \#143 支持 MATCH() 周围的括号
* [Commit 599e](https://github.com/manticoresoftware/manticoresearch/commit/599ee79ccd38cce0023a55cbce8f50c3dca62e38) 修复 ALTER cluster 语句中集群状态保存
* [Commit 230c](https://github.com/manticoresoftware/manticoresearch/commit/230c321e277514b191629538eb3f02d219113d95) 修复 ALTER index 时带有 blob 属性导致服务器崩溃
* [Commit 5802](https://github.com/manticoresoftware/manticoresearch/commit/5802b85aa93a81caea77073a3381ef912fdd677c) 修复 \#196 按 id 过滤
* [Commit 25d2](https://github.com/manticoresoftware/manticoresearch/commit/25d2dabda4a299a29bd375405862126da9b4bfac) 丢弃模板索引上的搜索
* [Commit 2a30](https://github.com/manticoresoftware/manticoresearch/commit/2a30d5b49d76faab031d195f533974938769b8e6) 修复 SQL 回复中 id 列为常规 bigint 类型


## 版本 3.0.0，2019年5月6日
### 功能和改进
* 新的索引存储。非标量属性不再限制每个索引最大 4GB 大小
* attr_update_reserve 指令
* 支持使用 UPDATE 更新字符串、JSON 和 MVA
* killlists 在索引加载时应用
* killlist_target 指令
* 多重 AND 搜索加速
* 更好的平均性能和内存使用
* 用于升级 2.x 创建的索引的转换工具
* CONCAT() 函数
* JOIN CLUSTER cluster AT 'nodeaddress:port'
* ALTER CLUSTER posts UPDATE nodes
* node_address 指令
* SHOW STATUS 中打印节点列表

### 行为变化
* 对于带有 killlist 的索引，服务器不再按配置中定义的顺序轮换索引，而是遵循 killlist 目标链
* 搜索中索引的顺序不再定义 killlist 应用的顺序
* 文档 ID 现在为有符号大整数

### 移除的指令
* docinfo（现在始终为外部），inplace_docinfo_gap，mva_updates_pool

## 版本 2.8.2 GA，2019年4月2日
### 功能和改进
* Galera 复制用于 percolate 索引
* OPTION morphology

### 编译说明
Cmake 最低版本现在为 3.13。编译需要 boost 和 libssl 开发库。
### 修复

* [Commit 6967](https://github.com/manticoresoftware/manticoresearch/commit/6967fedb2ef818ec1c825d482563edd05e1c9245) 修复了在查询多个分布式索引时，选择列表中多个星号导致的崩溃问题
* [Commit 36df](https://github.com/manticoresoftware/manticoresearch/commit/36df1a407dc08263690e3492518613ace82d69ca) 修复了 [\#177](https://github.com/manticoresoftware/manticoresearch/issues/177) 通过 Manticore SQL 接口传输大数据包的问题
* [Commit 5793](https://github.com/manticoresoftware/manticoresearch/commit/57932aec734583fa93359faaf5034b2e2c9d352b) 修复了 [\#170](https://github.com/manticoresoftware/manticoresearch/issues/170) 在 RT 优化时更新 MVA 导致服务器崩溃的问题
* [Commit edb2](https://github.com/manticoresoftware/manticoresearch/commit/edb24b870423add86eba471d361e0e5aff098b18) 修复了在 SIGHUP 配置重载后，RT 索引移除导致 binlog 移除时服务器崩溃的问题
* [Commit bd3e](https://github.com/manticoresoftware/manticoresearch/commit/bd3e66e0085bc7e2e351b817dfc972fd8158fbce) 修复了 MySQL 握手认证插件的负载问题
* [Commit 6a21](https://github.com/manticoresoftware/manticoresearch/commit/6a217f6ed82fb10f752213b15617b9cedf1e8533) 修复了 [\#172](https://github.com/manticoresoftware/manticoresearch/issues/172) RT 索引中的 phrase_boundary 设置问题
* [Commit 3562](https://github.com/manticoresoftware/manticoresearch/commit/3562f652753e4091fd7b5b0f65b58341c8cbcb31) 修复了 [\#168](https://github.com/manticoresoftware/manticoresearch/issues/168) 在 ATTACH 索引自身时的死锁问题
* [Commit 250b](https://github.com/manticoresoftware/manticoresearch/commit/250b3f0e74ad18dda34ba080aa13ca87798a9ac9) 修复了服务器崩溃后 binlog 保存空元数据的问题
* [Commit 4aa6](https://github.com/manticoresoftware/manticoresearch/commit/4aa6c69ad3a181cd7c5dafb990fb528121f68e59) 修复了由于 RT 索引带磁盘块的字符串排序器导致的服务器崩溃问题
## 版本 2.8.1 GA，2019年3月6日

### 新功能和改进
* SUBSTRING_INDEX()
* 支持 percolate 查询的 SENTENCE 和 PARAGRAPH
* Debian/Ubuntu 的 systemd 生成器；同时添加了 LimitCORE 以允许生成 core dump
### Bug 修复

* [Commit 84fe](https://github.com/manticoresoftware/manticoresearch/commit/84fe7405d17b59f4dcde15a6c8e8503923503def) 修复了在匹配模式为 all 且全文查询为空时服务器崩溃的问题
* [Commit daa8](https://github.com/manticoresoftware/manticoresearch/commit/daa88b579fa373d5d2e869e43a2e178363b0fef1) 修复了删除静态字符串时的崩溃问题
* [Commit 2207](https://github.com/manticoresoftware/manticoresearch/commit/22078537dddbd9ce81a182dcc879f235b6e87004) 修复了 indextool 失败时的退出码问题（FATAL）
* [Commit 0721](https://github.com/manticoresoftware/manticoresearch/commit/0721696d6780c200c65d596624a8187983fb7fcb) 修复了 [\#109](https://github.com/manticoresoftware/manticoresearch/issues/109) 由于错误的精确形式检查导致前缀无匹配的问题
* [Commit 8af8](https://github.com/manticoresoftware/manticoresearch/commit/8af810111b8e2f87bc9f378172eff1ab9725c7e7) 修复了 [\#161](https://github.com/manticoresoftware/manticoresearch/issues/161) RT 索引配置重载的问题
* [Commit e2d5](https://github.com/manticoresoftware/manticoresearch/commit/e2d592773f4fd702ace90070a793a842a0fec6ab) 修复了访问大型 JSON 字符串时服务器崩溃的问题
* [Commit 75cd](https://github.com/manticoresoftware/manticoresearch/commit/75cd1342d05cdc352e3dd145cdbc6f79394a165b) 修复了 JSON 文档中 PQ 字段被索引剥离器修改导致兄弟字段匹配错误的问题
* [Commit e2f7](https://github.com/manticoresoftware/manticoresearch/commit/e2f775437df922674d772ed4417780492502e65a) 修复了 RHEL7 构建中解析 JSON 时服务器崩溃的问题
* [Commit 3a25](https://github.com/manticoresoftware/manticoresearch/commit/3a25a5808feb3f8b80866991b436d6c1241618c2) 修复了当斜杠位于边缘时 JSON 反转义崩溃的问题
* [Commit be9f](https://github.com/manticoresoftware/manticoresearch/commit/be9f497872bea62dbdccd64fc28294fd7776c289) 修复了选项 'skip_empty' 跳过空文档且不警告其非有效 JSON 的问题
* [Commit 266e](https://github.com/manticoresoftware/manticoresearch/commit/266e0e7b088549722a805fc837bf101ff681a5e8) 修复了 [\#140](https://github.com/manticoresoftware/manticoresearch/issues/161) 当 6 位小数不足以精确时，浮点数输出 8 位数字的问题
* [Commit 3f6d](https://github.com/manticoresoftware/manticoresearch/commit/3f6d23891064ee8e94030c4231497cdd7da33a6a) 修复了空 jsonobj 创建的问题
* [Commit f3c7](https://github.com/manticoresoftware/manticoresearch/commit/f3c7848a59b7eedc67c3403d6d4b37ce6fa94dc8) 修复了 [\#160](https://github.com/manticoresoftware/manticoresearch/issues/161) 空 mva 输出 NULL 而非空字符串的问题
* [Commit 0afa](https://github.com/manticoresoftware/manticoresearch/commit/0afa2ed058e5759470b1d7354c722faab34f98bb) 修复了在没有 pthread_getname_np 时构建失败的问题
* [Commit 9405](https://github.com/manticoresoftware/manticoresearch/commit/9405fccdeb27f0302a8d9a848981d30080216777) 修复了带有 thread_pool 工作线程时服务器关闭崩溃的问题
## 版本 2.8.0 GA，2019年1月28日

### 改进
* percolate 索引的分布式索引支持
* CALL PQ 新选项和变更：
    *   skip_bad_json
    *   mode（sparsed/sharded）
    *   JSON 文档可以作为 JSON 数组传递
    *   shift
    *   列名 'UID'、'Documents'、'Query'、'Tags'、'Filters' 重命名为 'id'、'documents'、'query'、'tags'、'filters'
* DESCRIBE pq TABLE
* 不再支持 SELECT FROM pq WHERE UID，改用 'id'
* pq 索引上的 SELECT 与常规索引等效（例如，可以通过 REGEX() 过滤规则）
* ANY/ALL 可用于 PQ 标签
* 表达式对 JSON 字段自动转换，无需显式类型转换
* 内置 'non_cjk' charset_table 和 'cjk' ngram_chars
* 内置 50 种语言的停用词集合
* 停用词声明中的多个文件也可以用逗号分隔
* CALL PQ 可以接受 JSON 数组的文档
### Bug 修复

* [Commit a4e1](https://github.com/manticoresoftware/manticoresearch/commit/a4e19afee54dafdc04b0da53372dafd8c0d332d6) 修复了与 csjon 相关的内存泄漏
* [Commit 28d8](https://github.com/manticoresoftware/manticoresearch/commit/28d862774874891a03e361da1b0347ebe8066ce0) 修复了因 json 中缺失值导致的崩溃
* [Commit bf4e](https://github.com/manticoresoftware/manticoresearch/commit/bf4e9ea27c349cdc9ae4e54e960a4a17b90e38fa) 修复了 RT 索引空元数据保存问题
* [Commit 33b4](https://github.com/manticoresoftware/manticoresearch/commit/33b4573529e7c7c7bce19bf9d54054866f30d99a) 修复了词形还原序列中丢失的形式标志（exact）
* [Commit 6b95](https://github.com/manticoresoftware/manticoresearch/commit/6b95d48240d3b5520afade19c249d79e020a5e88) 修复了字符串属性大于 4M 时使用饱和而非溢出
* [Commit 6214](https://github.com/manticoresoftware/manticoresearch/commit/621418b829e70af36aaa322a2f51ece3f86bc499) 修复了禁用索引时服务器在接收到 SIGHUP 信号崩溃的问题
* [Commit 3f7e](https://github.com/manticoresoftware/manticoresearch/commit/3f7e35d1482966cc45d5abbcb2de5de3508d66b0) 修复了服务器在同时执行 API 会话状态命令时崩溃的问题
* [Commit cd9e](https://github.com/manticoresoftware/manticoresearch/commit/cd9e4f1709a48ddafc128c450c2d882bc11ba50e) 修复了带字段过滤器的 RT 索引删除查询导致服务器崩溃的问题
* [Commit 9376](https://github.com/manticoresoftware/manticoresearch/commit/9376470d455fcd256c6abff79c431a6919dfa7ac) 修复了对分布式索引执行 CALL PQ 时空文档导致服务器崩溃的问题
* [Commit 8868](https://github.com/manticoresoftware/manticoresearch/commit/8868b207644f4f8cc1ab0c270adb35493fed22e1) 修复了 Manticore SQL 错误信息超过 512 字符被截断的问题
* [Commit de9d](https://github.com/manticoresoftware/manticoresearch/commit/de9deda9c142823d6dbf529423417b5c670fae94) 修复了无 binlog 时保存 percolate 索引导致崩溃的问题
* [Commit 2b21](https://github.com/manticoresoftware/manticoresearch/commit/2b219e1a32791e7740bb210b7d408f96abc6374f) 修复了 OSX 上 http 接口无法工作的问题
* [Commit e92c](https://github.com/manticoresoftware/manticoresearch/commit/e92c60240f8f9e1756bfa14fba0bbb402586a389) 修复了 indextool 检查 MVA 时的错误提示
* [Commit 238b](https://github.com/manticoresoftware/manticoresearch/commit/238bdea59bad89f097403f1c978658ce45f16c70) 修复了 FLUSH RTINDEX 时写锁问题，避免保存和定期刷新时锁定整个索引
* [Commit c26a](https://github.com/manticoresoftware/manticoresearch/commit/c26a236bd42b7082db079a51a84172066d337d64) 修复了 ALTER percolate 索引时卡在等待搜索加载的问题
* [Commit 9ee5](https://github.com/manticoresoftware/manticoresearch/commit/9ee5703f6c28f57cd5bff5e705966d93fe30d267) 修复了 max_children 为 0 时应使用线程池默认工作线程数的问题
* [Commit 5138](https://github.com/manticoresoftware/manticoresearch/commit/5138fc048a1bd146b271ce6a72fe954344281f69) 修复了带有 index_token_filter 插件且 stopwords 和 stopword_step=0 的索引数据索引错误
* [Commit 2add](https://github.com/manticoresoftware/manticoresearch/commit/2add3d319ac62fe450bf60e89033100853dc2ecf) 修复了索引定义中仍使用 aot 词形还原器时缺少 lemmatizer_base 导致崩溃的问题
## 版本 2.7.5 GA，2018年12月4日

### 改进
* REGEX 函数
* json API 搜索的 limit/offset 支持
* qcache 的性能分析点
### 修复

* [Commit eb3c](https://github.com/manticoresoftware/manticoresearch/commit/eb3c7683b9740cff4cc4cadd2ab9f990b9f7d173) 修复了 FACET 多属性宽类型时服务器崩溃的问题
* [Commit d915](https://github.com/manticoresoftware/manticoresearch/commit/d915cf6696514f4be365ffb5981bccaa15983e65) 修复了 FACET 查询主选择列表中隐式 group by 问题
* [Commit 5c25](https://github.com/manticoresoftware/manticoresearch/commit/5c25dc271a8c11a1c82ca3c88e518acbc6a8c164) 修复了带 GROUP N BY 查询时崩溃的问题
* [Commit 85d3](https://github.com/manticoresoftware/manticoresearch/commit/85d30a232d4ffd89916b673a9b2db946d272a945) 修复了内存操作崩溃处理时的死锁问题
* [Commit 8516](https://github.com/manticoresoftware/manticoresearch/commit/85166b5ef2be38061d62f2b20f15acaa0311258a) 修复了 indextool 检查时的内存消耗问题
* [Commit 58fb](https://github.com/manticoresoftware/manticoresearch/commit/58fb03166830d8b23e7ab9ce56309db7ddac22f4) 修复了不再需要 gmock 头文件的问题，上游已自行解决
## 版本 2.7.4 GA，2018年11月1日

### 改进
* 远程分布式索引的 SHOW THREADS 显示原始查询而非 API 调用
* SHOW THREADS 新增选项 `format=sphinxql`，以 SQL 格式打印所有查询
* SHOW PROFILE 增加了 `clone_attrs` 阶段的打印
### 修复

* [Commit 4f15](https://github.com/manticoresoftware/manticoresearch/commit/4f155712a0bccc1bd01cc191647bc8cff814888e) 修复了在无 malloc_stats、malloc_trim 的 libc 上构建失败的问题
* [Commit f974](https://github.com/manticoresoftware/manticoresearch/commit/f974f20bda3214a56877c393a192be1a77150958) 修复了 CALL KEYWORDS 结果集中单词内特殊符号的问题
* [Commit 0920](https://github.com/manticoresoftware/manticoresearch/commit/092083282ea8ae0b2e72fb6989c57ccec81e74ac) 修复了通过 API 调用分布式索引或远程代理的 CALL KEYWORDS 失效问题
* [Commit fd68](https://github.com/manticoresoftware/manticoresearch/commit/fd686bfe88b720ffd7642e36897ba45161cbd7d2) 修复了分布式索引 agent_query_timeout 未正确传递给代理作为 max_query_time 的问题
* [Commit 4ffa](https://github.com/manticoresoftware/manticoresearch/commit/4ffa623e9d357a3b0b441615089e211f92f8de32) 修复了磁盘块总文档计数被 OPTIMIZE 命令影响，导致权重计算错误的问题
* [Commit dcaf](https://github.com/manticoresoftware/manticoresearch/commit/dcaf4e0e3c8e9b0fbec47bbf3307feddec0936a6) 修复了混合 RT 索引中多个尾部命中问题
* [Commit eee3](https://github.com/manticoresoftware/manticoresearch/commit/eee381754e902a43a8f499e1c950198a2a3e6ee0) 修复了轮换时的死锁问题
## 版本 2.7.3 GA，2018年9月26日

### 改进
* CALL KEYWORDS 的 sort_mode 选项
* VIP 连接上的 DEBUG 可以执行 'crash <password>' 来在服务器上故意触发 SIGEGV 操作
* DEBUG 可以执行 'malloc_stats' 来在 searchd.log 中转储 malloc 统计信息，'malloc_trim' 来执行 malloc_trim()
* 如果系统上存在 gdb，则改进了回溯功能
### Bug 修复

* [Commit 0f3c](https://github.com/manticoresoftware/manticoresearch/commit/0f3cc333f6129451ad1ae2c97905b6c04ba5d454) 修复了 Windows 上 rename 导致的崩溃或失败
* [Commit 1455](https://github.com/manticoresoftware/manticoresearch/commit/1455ba2bf8079f03ce7f861a8d9662a360d705de) 修复了 32 位系统上服务器崩溃的问题
* [Commit ad37](https://github.com/manticoresoftware/manticoresearch/commit/ad3710d53b5cd6a28b8c60352d0f9e285b03d9fa) 修复了空 SNIPPET 表达式导致的服务器崩溃或挂起
* [Commit b36d](https://github.com/manticoresoftware/manticoresearch/commit/b36d79214364500252665860f000ae85343528b6) 修复了非渐进式优化的损坏，并修正了渐进式优化不为最旧磁盘块创建 kill-list 的问题
* [Commit 34b0](https://github.com/manticoresoftware/manticoresearch/commit/34b032499afd42ce47a4c7247814b4031094388a) 修复了线程池工作模式下 SQL 和 API 的 queue_max_length 错误回复
* [Commit ae4b](https://github.com/manticoresoftware/manticoresearch/commit/ae4b3202cbdb8014cabe2b90e269d5cf74f49871) 修复了向设置了 regexp 或 rlp 选项的 PQ 索引添加全扫描查询时的崩溃
* [Commit f80f](https://github.com/manticoresoftware/manticoresearch/commit/f80f8d5d7560187078868aed9a9575f4549e98aa) 修复了连续调用多个 PQ 时的崩溃
* [Commit 9742](https://github.com/manticoresoftware/manticoresearch/commit/9742f5f0866af73f8cd8483ecd18a507ea80dd65) 重构 AcquireAccum
* [Commit 39e5](https://github.com/manticoresoftware/manticoresearch/commit/39e5bc3751b7295222eb76407c5d72ce1dad545b) 修复了调用 pq 后的内存泄漏
* [Commit 21bc](https://github.com/manticoresoftware/manticoresearch/commit/21bcc6d17395f0f57dde79f6716ef303b7ea527d) 美化重构（c++11 风格的构造函数、默认值、nullptr）
* [Commit 2d69](https://github.com/manticoresoftware/manticoresearch/commit/2d690398f14c736956cfdd66feb4d3091d6b3a4d) 修复了尝试向 PQ 索引插入重复项时的内存泄漏
* [Commit 5ed9](https://github.com/manticoresoftware/manticoresearch/commit/5ed92c4b66da6423fa28f11b1fd59103ed1e1fb9) 修复了 JSON 字段 IN 语句中大值导致的崩溃
* [Commit 4a52](https://github.com/manticoresoftware/manticoresearch/commit/4a5262e2776aa261e34ba5c914ea60804f59de3f) 修复了对设置了扩展限制的 RT 索引执行 CALL KEYWORDS 语句时服务器崩溃
* [Commit 5526](https://github.com/manticoresoftware/manticoresearch/commit/552646bb6fefde4e2525298e43e628839b421d3d) 修复了 PQ 匹配查询中的无效过滤器
* [Commit 204f](https://github.com/manticoresoftware/manticoresearch/commit/204f521408b730198a5bab2c20407a3d7df94873) 为 ptr 属性引入小对象分配器
* [Commit 2545](https://github.com/manticoresoftware/manticoresearch/commit/25453e5387152c0575d1eda1b2beb89a49c7493a) 将 ISphFieldFilter 重构为引用计数版本
* [Commit 1366](https://github.com/manticoresoftware/manticoresearch/commit/1366ee06e828397074b69bec9265d7147170c60e) 修复了在非终止字符串上使用 strtod 导致的未定义行为/SIGSEGV
* [Commit 94bc](https://github.com/manticoresoftware/manticoresearch/commit/94bc6fcbdae4d08bcf6da2966e3ec2966091289f) 修复了 json 结果集处理中的内存泄漏
* [Commit e78e](https://github.com/manticoresoftware/manticoresearch/commit/e78e9c948963416caff9e4e46296a58080107835) 修复了应用属性添加时对内存块末尾的越界读取
* [Commit fad5](https://github.com/manticoresoftware/manticoresearch/commit/fad572fb543606b33e688eb82a485b86011545fd) 重构 CSphDict 为引用计数版本
* [Commit fd84](https://github.com/manticoresoftware/manticoresearch/commit/fd841a472b472280c93f4af3506851d19f6adc45) 修复了 AOT 内部类型外泄
* [Commit 5ee7](https://github.com/manticoresoftware/manticoresearch/commit/5ee7f208c7685f2ebb9e5623a8802b721e2a7f0a) 修复了分词器管理的内存泄漏
* [Commit 116c](https://github.com/manticoresoftware/manticoresearch/commit/116c5f1abebee9a0d99afe93546a1d8e4c6c6958) 修复了 grouper 中的内存泄漏
* [Commit 56fd](https://github.com/manticoresoftware/manticoresearch/commit/56fdbc9ab38973a9a0ba8ccee45f71cf33c16423) 为 matches 中的动态指针提供特殊的释放/复制（grouper 内存泄漏）
* [Commit b1fc](https://github.com/manticoresoftware/manticoresearch/commit/31db660f378541aa0d1e7cf2e7209bb0a1f47fd8) 修复了 RT 动态字符串的内存泄漏
* [Commit 517b](https://github.com/manticoresoftware/manticoresearch/commit/517b9e8749f092e923cd2884b674b5bb84e20755) 重构 grouper
* [Commit b1fc](https://github.com/manticoresoftware/manticoresearch/commit/b1fc16140e5dc44290686330b476a254e0092107) 小幅重构（c++11 构造函数，一些格式调整）
* [Commit 7034](https://github.com/manticoresoftware/manticoresearch/commit/7034e073f4cf2844762e0a464b7c3de05d3d122b) 将 ISphMatchComparator 重构为引用计数版本
* [Commit b1fc](https://github.com/manticoresoftware/manticoresearch/commit/b1fc16140e5dc44290686330b476a254e0092107) 私有化 cloner
* [Commit efbc](https://github.com/manticoresoftware/manticoresearch/commit/efbc0511d6809c4a57453c7283f9bf53f3fb8d97) 简化 MVA_UPSIZE、DOCINFO2ID_T、DOCINFOSETID 的本地小端处理
* [Commit 6da0](https://github.com/manticoresoftware/manticoresearch/commit/6da0df4ac9946fc59f98d229b90e12c9221dd4c0) 为 ubertests 添加 valgrind 支持
* [Commit 1d17](https://github.com/manticoresoftware/manticoresearch/commit/1d17669cb7ec54ac80b392e101f37a688eb98080) 修复了连接上 'success' 标志的竞态导致的崩溃
* [Commit 5a09](https://github.com/manticoresoftware/manticoresearch/commit/5a09c32d7ded56b2c0bd3e2ad7968cb8d6362919) 将 epoll 切换为边缘触发模式
* [Commit 5d52](https://github.com/manticoresoftware/manticoresearch/commit/5d528682737fca03a4352a3093b1319ec27d598c) 修复了表达式中 IN 语句格式类似过滤器时的问题
* [Commit bd8b](https://github.com/manticoresoftware/manticoresearch/commit/bd8b3c976ff8b4667f55dfdd101b20a920137ac5) 修复了在提交具有大 docid 的文档时 RT 索引崩溃的问题
* [Commit ce65](https://github.com/manticoresoftware/manticoresearch/commit/ce656b83b747ce7c55795b53915770c13fb99cfe) 修复了 indextool 中无参数选项的问题
* [Commit 08c9](https://github.com/manticoresoftware/manticoresearch/commit/08c9507177820f441f534bf06a11dac5e54bebb4) 修复了扩展关键字的内存泄漏
* [Commit 30c7](https://github.com/manticoresoftware/manticoresearch/commit/30c75a2f525ec9bda625d903acdc9d4d2e2413dc) 修复了 json grouper 的内存泄漏
* [Commit 6023](https://github.com/manticoresoftware/manticoresearch/commit/6023f269b6f2080e4d380b0957605ef8107c8c9f) 修复了全局用户变量的泄漏
* [Commit 7c13](https://github.com/manticoresoftware/manticoresearch/commit/7c138f15ca23c0c8717fa12041e3db7f988fcf15) 修复了早期拒绝匹配时动态字符串的泄漏
* [Commit 9154](https://github.com/manticoresoftware/manticoresearch/commit/9154b18eaed5302e8330cb3a95bd968959ccb312) 修复了 length(<expression>) 的泄漏
* [Commit 43fc](https://github.com/manticoresoftware/manticoresearch/commit/43fca3a4e26139765d0fac8de054200dc4875fc6) 修复了 parser 中因 strdup() 导致的内存泄漏
* [Commit 71ff](https://github.com/manticoresoftware/manticoresearch/commit/71ff77737e63d1019b7c9804dca7fa2351025dba) 重构表达式解析器以准确跟踪引用计数
## 版本 2.7.2 GA，2018年8月27日

### 改进
* 与 MySQL 8 客户端兼容
* 支持带 RECONFIGURE 的 [TRUNCATE](Emptying_a_table.md)
* 取消了 RT 索引的 SHOW STATUS 中的内存计数器
* 多代理的全局缓存
* 改进了 Windows 上的 IOCP
* HTTP 协议的 VIP 连接
* Manticore SQL 的 [DEBUG](Reporting_bugs.md#DEBUG) 命令，可运行各种子命令
* [shutdown_token](Server_settings/Searchd.md#shutdown_token) - 使用 DEBUG 命令调用 `shutdown` 所需的密码 SHA1 哈希
* 新增 SHOW AGENT STATUS 的统计信息 (_ping, _has_perspool, _need_resolve)
* indexer 的 --verbose 选项现在支持 \[debugvv\] 用于打印调试信息
### Bug 修复

* [Commit 3900](https://github.com/manticoresoftware/manticoresearch/commit/390082a7be0a1f9539b30361d11d54de35c62a44) 移除了 optimize 时的写锁
* [Commit 4c33](https://github.com/manticoresoftware/manticoresearch/commit/4c337648329be22373be33333cbc9c5664d18b80) 修复了重新加载索引设置时的写锁
* [Commit b5ea](https://github.com/manticoresoftware/manticoresearch/commit/b5ea8dc0b18a23dbd739e92c66dd93b0094df14b) 修复了带 JSON 过滤器查询时的内存泄漏
* [Commit 930e](https://github.com/manticoresoftware/manticoresearch/commit/930e831a43fd3d1ce996185be968d1df49a088d0) 修复了 PQ 结果集中空文档的问题
* [Commit 53de](https://github.com/manticoresoftware/manticoresearch/commit/53deecfecb374431f84f8592a1095a77407b8aea) 修复了因移除任务导致的任务混淆
* [Commit cad9](https://github.com/manticoresoftware/manticoresearch/commit/cad9b9c617719aad97e9f5a2ae93491153d8e504) 修复了远程主机计数错误
* [Commit 9000](https://github.com/manticoresoftware/manticoresearch/commit/90008cc15c4684d445d37a391bb4a667bbc2e365) 修复了解析代理描述符的内存泄漏
* [Commit 978d](https://github.com/manticoresoftware/manticoresearch/commit/978d839641ad6040c22c9fc3b703b1a02685f266) 修复了搜索中的泄漏
* [Commit 0193](https://github.com/manticoresoftware/manticoresearch/commit/0193946105ac08913362b7d7acf0c90f9b38dc85) 对显式/内联 c-trs，override/final 用法进行了美化修改
* [Commit 943e](https://github.com/manticoresoftware/manticoresearch/commit/943e2997b33f9b2eda23d94a66068910f9476ebd) 修复了本地/远程 schema 中 json 的泄漏
* [Commit 02db](https://github.com/manticoresoftware/manticoresearch/commit/02dbdd6f1d0b72e7e458ebebdd2c67f989577e6b) 修复了本地/远程 schema 中 json 排序列表达式的泄漏
* [Commit c74d](https://github.com/manticoresoftware/manticoresearch/commit/c74d0b4035e4214510376845b22cb676a8da24a3) 修复了常量别名的泄漏
* [Commit 6e5b](https://github.com/manticoresoftware/manticoresearch/commit/6e5b57e1d3952c9695376728a143bf5434208f53) 修复了预读线程的泄漏
* [Commit 39c7](https://github.com/manticoresoftware/manticoresearch/commit/39c740fef5bf71da0bee60a35f5dd9f471af850f) 修复了因 netloop 中等待卡住导致退出卡死的问题
* [Commit adaf](https://github.com/manticoresoftware/manticoresearch/commit/adaf9772f95772ad6d5297acace6c5cb92b56ad5) 修复了将 HA 代理改为普通主机时 'ping' 行为卡死的问题
* [Commit 32c4](https://github.com/manticoresoftware/manticoresearch/commit/32c40eac9cdd40b15d79e58b2f15416f164f9701) 为仪表盘存储分离了垃圾回收
* [Commit 511a](https://github.com/manticoresoftware/manticoresearch/commit/511a3cf2c1f1858f6e94fe9f8175b7216db3cbd6) 修复了引用计数指针的问题
* [Commit 32c4](https://github.com/manticoresoftware/manticoresearch/commit/af2326cd2927effbad7bec88e8dc238a566cf27e) 修复了 indextool 在索引不存在时崩溃的问题
* [Commit 156e](https://github.com/manticoresoftware/manticoresearch/commit/156edc717dc63d3ea120f42466eba6e022da88be) 修复了 xmlpipe 索引中超出属性/字段的输出名称
* [Commit cdac](https://github.com/manticoresoftware/manticoresearch/commit/cdac6d1f17d384bfc8bae49e91241b2f346c3830) 修复了配置中无索引器段时默认索引器的值
* [Commit e61e](https://github.com/manticoresoftware/manticoresearch/commit/e61ec00b6b27d1d5878247e2ee817f3b1e7fde16) 修复了服务器重启后 RT 索引磁盘块中错误嵌入的停用词
* [Commit 5fba](https://github.com/manticoresoftware/manticoresearch/commit/5fba49d5e28c17de4c0acbd984466127f42de6e8) 修复了跳过幽灵（已关闭但未从轮询器中最终删除）连接的问题
* [Commit f22a](https://github.com/manticoresoftware/manticoresearch/commit/f22ae34b623906b3c5a05a06198dad2e548dc541) 修复了混合（孤立）网络任务
* [Commit 4689](https://github.com/manticoresoftware/manticoresearch/commit/46890e70eb8dcd00c1bf1b030538b8f131c601c2) 修复了写入后读取操作崩溃的问题
* [Commit 03f9](https://github.com/manticoresoftware/manticoresearch/commit/03f9df904f611c438d5ebcaaeef911b0dc8d6c39) 修复了在 Windows 上运行测试时 searchd 崩溃的问题
* [Commit e925](https://github.com/manticoresoftware/manticoresearch/commit/e9255ed2704790a3d7f5273d38b85433463cb3ff) 修复了在常规 connect() 中处理 EINPROGRESS 代码的问题
* [Commit 248b](https://github.com/manticoresoftware/manticoresearch/commit/248b72f1544b27e9e82773284050d44eeb1714e1) 修复了使用 TFO 时的连接超时问题
## 版本 2.7.1 GA，2018年7月4日

### 改进
* 改进了 PQ 上匹配多个文档时的通配符性能
* 支持 PQ 上的全扫描查询
* 支持 PQ 上的 MVA 属性
* 支持 percolate 索引的正则表达式和 RLP
### Bug 修复

* [Commit 6885](https://github.com/manticoresoftware/manticoresearch/commit/68856261b41f6950666f9e5122726839fb3c71d0) 修复了查询字符串丢失的问题
* [Commit 0f17](https://github.com/manticoresoftware/manticoresearch/commit/0f1770943cba53bfd2e0edfdf0b1495ff0dd0e89) 修复了 SHOW THREADS 语句中信息为空的问题
* [Commit 53fa](https://github.com/manticoresoftware/manticoresearch/commit/53faa36a916bb87868e83aa79c1a1f972fb20ca0) 修复了使用 NOTNEAR 操作符匹配时的崩溃
* [Commit 2602](https://github.com/manticoresoftware/manticoresearch/commit/26029a7cd77b518fb3a27588d7db6c8bffc73b0f) 修复了 PQ 删除时错误的过滤器错误信息
## 版本 2.7.0 GA，2018年6月11日


### 改进
* 减少系统调用次数以避免 Meltdown 和 Spectre 补丁的影响
* 本地索引管理的内部重写
* 远程摘要重构
* 完整配置重载
* 所有节点连接现在都是独立的
* 协议改进
* Windows 通信从 wsapoll 切换到 IO 完成端口
* 主节点和节点之间的通信可以使用 TFO
* [SHOW STATUS](Node_info_and_management/Node_status.md#SHOW-STATUS) 现在输出服务器版本和 mysql_version_string
* 为 CALL PQ 中调用的文档添加了 `docs_id` 选项
* percolate 查询过滤器现在可以包含表达式
* 分布式索引可以与 FEDERATED 一起工作
* 虚拟的 SHOW NAMES COLLATE 和 `SET wait_timeout`（以更好地兼容 ProxySQL）
### Bug 修复

* [Commit 5bcf](https://github.com/manticoresoftware/manticoresearch/commit/5bcff05d19e495f1a44f190885b25cbb6f217c4a) 修复了 PQ 中添加的不等于标签
* [Commit 9ebc](https://github.com/manticoresoftware/manticoresearch/commit/9ebc58916cd515eaa88da66d0895aebf0d1f2b5f) 修复了 CALL PQ 语句中添加文档 ID 字段到 JSON 文档的问题
* [Commit 8ae0](https://github.com/manticoresoftware/manticoresearch/commit/8ae0e593a286110d8505d88baa3ac9e433cb38c9) 修复了 PQ 索引的刷新语句处理器
* [Commit c24b](https://github.com/manticoresoftware/manticoresearch/commit/c24b152344ecd77661566cddb803487f51d3c1aa) 修复了 PQ 对 JSON 和字符串属性的过滤
* [Commit 1b8b](https://github.com/manticoresoftware/manticoresearch/commit/1b8bdde19aaa362785ea4c33bdc019154f8fbac4) 修复了空 JSON 字符串的解析
* [Commit 1ad8](https://github.com/manticoresoftware/manticoresearch/commit/1ad8a07dbfc1131913a8d92c261fbb48f934e8b7) 修复了多查询中带 OR 过滤器时的崩溃
* [Commit 69b8](https://github.com/manticoresoftware/manticoresearch/commit/69b89806380a229e36287ad9daf6f0ea2b5453eb) 修复了 indextool 使用配置公共部分（lemmatizer_base 选项）进行命令（dumpheader）
* [Commit 6dbe](https://github.com/manticoresoftware/manticoresearch/commit/6dbeaf2389d2af6a9dfccb56bfc986685b9f203e) 修复了结果集和过滤器中的空字符串
* [Commit 39c4](https://github.com/manticoresoftware/manticoresearch/commit/39c4eb55ecc85f6dd54f6c8c6d6dfcf87fd8748e) 修复了负文档 ID 值
* [Commit 266b](https://github.com/manticoresoftware/manticoresearch/commit/266b7054f98e7d30fc2829b5c2467dc9f90def92) 修复了非常长单词索引时的单词截断长度
* [Commit 4782](https://github.com/manticoresoftware/manticoresearch/commit/47823b0bd06cbd016a24345c99bb8693790b43dd) 修复了 PQ 上通配符查询匹配多个文档的问题
## 版本 2.6.4 GA，2018年5月3日


### 功能和改进
* MySQL FEDERATED 引擎[支持](Extensions/FEDERATED.md)
* MySQL 数据包现在返回 SERVER_STATUS_AUTOCOMMIT 标志，增强了与 ProxySQL 的兼容性
* [listen_tfo](Server_settings/Searchd.md#listen_tfo) - 为所有监听器启用 TCP Fast Open 连接
* indexer --dumpheader 现在也可以从 .meta 文件中转储 RT 头信息
* Ubuntu Bionic 的 cmake 构建脚本
### Bug 修复

* [Commit 355b](https://github.com/manticoresoftware/manticoresearch/commit/355b11629174813abd9cd5bf2233be0783f77745) 修复了 RT 索引的无效查询缓存条目；
* [Commit 546e](https://github.com/manticoresoftware/manticoresearch/commit/546e2297a01cc2913bc0d33052d5bcefae8737eb) 修复了无缝轮换后索引设置丢失的问题
* [Commit 0c45](https://github.com/manticoresoftware/manticoresearch/commit/0c4509898393993bc87194d2aca2070395ff7f83) 修复了中缀与前缀长度设置问题；添加了对不支持中缀长度的警告
* [Commit 8054](https://github.com/manticoresoftware/manticoresearch/commit/80542fa54d727c781635918e8d4d65a20c2fbd1e) 修复了 RT 索引自动刷新顺序
* [Commit 705d](https://github.com/manticoresoftware/manticoresearch/commit/705d8c5f407726ed1c6f9745ecffa375682c8969) 修复了多属性索引和多索引查询的结果集模式问题
* [Commit b0ba](https://github.com/manticoresoftware/manticoresearch/commit/b0ba932fff4a082f642cb737381c8a70215d4cc9) 修复了批量插入时文档重复导致部分命中丢失的问题
* [Commit 4510](https://github.com/manticoresoftware/manticoresearch/commit/4510fa44ad54b5bbac53c591144501565130fcae) 修复了优化失败合并包含大量文档的 RT 索引磁盘块的问题
## 版本 2.6.3 GA，2018年3月28日

### 改进
* 编译时使用 jemalloc。如果系统中存在 jemalloc，可以通过 cmake 标志 `-DUSE_JEMALLOC=1` 启用
### Bug 修复

* [提交 85a6](https://github.com/manticoresoftware/manticoresearch/commit/85a6d7e3db75f28c491042409bb677ed39eba07b) 修复了 Manticore SQL 查询日志中的 expand_keywords 选项
* [提交 caaa](https://github.com/manticoresoftware/manticoresearch/commit/caaa3842f4dbd8507fd9e20bf3371a0c8ec0c087) 修复了 HTTP 接口以正确处理大尺寸查询
* [提交 e386](https://github.com/manticoresoftware/manticoresearch/commit/e386d842b36195719e459b2db1fb20e402f9a51f) 修复了启用 index_field_lengths 时对 RT 索引执行 DELETE 导致服务器崩溃的问题
* [提交 cd53](https://github.com/manticoresoftware/manticoresearch/commit/ce528f37eb5d8284661928f6f3b340a4a1985253) 修复了 cpustats searchd CLI 选项在不支持的系统上的工作问题
* [提交 8740](https://github.com/manticoresoftware/manticoresearch/commit/8740fd63ae842c2ab0e51a8fcd6180d787ed57ec) 修复了定义了最小长度时 utf8 子串匹配的问题
## 版本 2.6.2 GA，2018年2月23日


### 改进
* 在使用 NOT 操作符和批量文档时，改进了[Percolate 查询](Searching/Percolate_query.md)的性能。
* [percolate_query_call](Searching/Percolate_query.md) 可以根据 [dist_threads](Server_settings/Searchd.md#threads) 使用多线程
* 新增全文匹配操作符 NOTNEAR/N
* 对 percolate 索引的 SELECT 支持 LIMIT
* [expand_keywords](Searching/Options.md#expand_keywords) 可以接受 'start','exact'（其中 'start,exact' 与 '1' 效果相同）
* 对使用 sql_query_range 定义的范围查询的 [joined fields](Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Processing_fetched_data.md#sql_joined_field) 支持 ranged-main-query
### Bug 修复

* [提交 72dc](https://github.com/manticoresoftware/manticoresearch/commit/72dcf669744e9b7d636dfc213d24df85ab301f6b) 修复了搜索内存段时崩溃；双缓冲保存磁盘块时死锁；优化期间保存磁盘块时死锁
* [提交 3613](https://github.com/manticoresoftware/manticoresearch/commit/36137149a1c3c0893bdda5a28fc7e8244bf2d4ae) 修复了索引器在 xml 嵌入式 schema 中属性名为空时崩溃的问题
* [提交 48d7](https://github.com/manticoresoftware/manticoresearch/commit/48d7e8001d2a66466ca64577f27ddc5421a67251) 修复了错误地取消链接非拥有的 pid 文件的问题
* [提交 a556](https://github.com/manticoresoftware/manticoresearch/commit/a5563a465ddc59ef71e65f17b68bc33f9700e838) 修复了临时文件夹中有时遗留孤立 FIFO 的问题
* [提交 2376](https://github.com/manticoresoftware/manticoresearch/commit/2376e8fc4508944b96959bd10686c6d51f5145e8) 修复了 FACET 结果集为空时错误的 NULL 行问题
* [提交 4842](https://github.com/manticoresoftware/manticoresearch/commit/4842b67acdd5da75672db6c08ac563b48577ddd7) 修复了作为 Windows 服务运行服务器时索引锁损坏的问题
* [提交 be35](https://github.com/manticoresoftware/manticoresearch/commit/be35feef54ded59125750916cd211e56108eddae) 修复了 mac os 上错误的 iconv 库问题
* [提交 8374](https://github.com/manticoresoftware/manticoresearch/commit/83744a977d7837d1d87cd506b88b0e7eb199efe6) 修复了错误的 count(\*) 计算
## 版本 2.6.1 GA，2018年1月26日


### 改进
* [agent_retry_count](Server_settings/Searchd.md#agent_retry_count) 对于带镜像的代理，重试次数现在表示每个镜像的重试次数，总重试次数为 agent_retry_count\*镜像数。
* [agent_retry_count](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_retry_count) 现在可以针对每个索引指定，覆盖全局值。新增别名 [mirror_retry_count](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#mirror_retry_count)。
* 可以在代理定义中指定 retry_count，表示每个代理的重试次数
* Percolate 查询现在支持 HTTP JSON API，路径为 [/json/pq](Searching/Percolate_query.md#Performing-a-percolate-query-with-CALL-PQ)。
* 为可执行文件添加了 -h 和 -v 选项（帮助和版本）
* [morphology_skip_fields](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) 支持实时索引
### Bug 修复

* [提交 a40b](https://github.com/manticoresoftware/manticore/commit/a40b0793feff65e40d10062568d9847c08d10f57) 修复了 ranged-main-query 在 MVA 字段使用 sql_range_step 时的正确工作
* [提交 f2f5](https://github.com/manticoresoftware/manticore/commit/f2f53757db45bcfb1544263ce0817e856656a621) 修复了黑洞系统循环挂起和黑洞代理似乎断开连接的问题
* [提交 84e1](https://github.com/manticoresoftware/manticore/commit/84e1f54aef25e0fce98870ad2dd784db5116f1d6) 修复了查询 ID 不一致，修复了存储查询的重复 ID
* [提交 1948](https://github.com/manticoresoftware/manticore/commit/19484231814fcb82b21763a3a4a9f45adc6b2d40) 修复了服务器在各种状态下关闭时崩溃的问题
* [提交 9a70](https://github.com/manticoresoftware/manticore/commit/9a706b499a1d61a90076065a1a703029d49db958)  [提交 3495](https://github.com/manticoresoftware/manticore/commit/3495fd70cba8846b1a50d55d6679b039414c3d2a) 修复了长查询超时问题
* [提交 3359](https://github.com/manticoresoftware/manticore/commit/3359bcd89b4f79a645fe84b8cf8616ce0addff02) 重构了基于 kqueue 的系统（Mac OS X，BSD）上的主代理网络轮询
## 版本 2.6.0，2017年12月29日


### 功能和改进
* [HTTP JSON](Connecting_to_the_server/HTTP.md)：JSON 查询现在可以对属性进行等值查询，MVA 和 JSON 属性可以用于插入和更新，通过 JSON API 可以对分布式索引执行更新和删除
* [Percolate 查询](Searching/Percolate_query.md)
* 移除了对 32 位 docid 的支持，同时移除了所有转换/加载带有 32 位 docid 的旧索引的代码。
* [仅对特定字段应用形态学](https://github.com/manticoresoftware/manticore/issues/7)。新增索引指令 morphology_skip_fields，允许定义不应用形态学的字段列表。
* [expand_keywords 现在可以通过 OPTION 语句设置为查询运行时指令](https://github.com/manticoresoftware/manticore/issues/8)
### Bugfixes

* [Commit 0cfa](https://github.com/manticoresoftware/manticore/commit/0cfae4c) 修复了使用 rlp 构建时服务器调试版本崩溃（以及发布版本可能的未定义行为）
* [Commit 3242](https://github.com/manticoresoftware/manticore/commit/324291e) 修复启用 progressive 选项时 RT 索引优化合并 kill-list 顺序错误的问题
* [Commit ac0e](https://github.com/manticoresoftware/manticore/commit/ac0efee) mac 上的轻微崩溃修复
* 经过彻底的静态代码分析后修复了许多小问题
* 其他小的 bug 修复
### 升级

在此版本中，我们更改了主节点和代理之间通信所使用的内部协议。如果您在分布式环境中运行 Manticoresearch 并有多个实例，请确保先升级代理，然后升级主节点。
## 版本 2.5.1，2017年11月23日

### 功能和改进
* [HTTP API 协议](Connecting_to_the_server/HTTP.md) 支持 JSON 查询。支持搜索、插入、更新、删除、替换操作。数据操作命令也可以批量执行，目前有一些限制，如 MVA 和 JSON 属性不能用于插入、替换或更新。
* [RELOAD INDEXES](Data_creation_and_modification/Adding_data_from_external_storages/Rotating_a_table.md#RELOAD-TABLES) 命令
* [FLUSH LOGS](Logging/Rotating_query_and_server_logs.md) 命令
* [SHOW THREADS](Node_info_and_management/SHOW_THREADS.md) 可以显示优化、轮换或刷新进度。
* GROUP N BY 正确支持 MVA 属性
* 黑洞代理运行在独立线程，不再影响主查询
* 实现了索引的引用计数，避免轮换和高负载时的停顿
* 实现了 SHA1 哈希，尚未对外暴露
* 修复了在 FreeBSD、macOS 和 Alpine 上的编译问题
### Bugfixes

* [Commit 9897](https://github.com/manticoresoftware/manticore/commit/989752b) 修复了块索引的过滤回归问题
* [Commit b1c3](https://github.com/manticoresoftware/manticore/commit/b1c3864) 将 PAGE_SIZE 重命名为 ARENA_PAGE_SIZE 以兼容 musl
* [Commit f213](https://github.com/manticoresoftware/manticore/commit/f2133cc) 禁用 cmake 版本低于 3.1.0 的 googletests
* [Commit f30e](https://github.com/manticoresoftware/manticore/commit/0839de7) 修复服务器重启时绑定套接字失败
* [Commit 0807](https://github.com/manticoresoftware/manticore/commit/0807240) 修复服务器关闭时崩溃
* [Commit 3e3a](https://github.com/manticoresoftware/manticore/commit/3e3acc3) 修复系统黑洞线程的 show threads
* [Commit 262c](https://github.com/manticoresoftware/manticore/commit/262c3fe) 重构了 iconv 的配置检查，修复了 FreeBSD 和 Darwin 上的构建问题
## 版本 2.4.1 GA，2017年10月16日

### 功能和改进
* WHERE 子句中属性过滤器之间支持 OR 操作符
* 维护模式（SET MAINTENANCE=1）
* [CALL KEYWORDS](Searching/Autocomplete.md#CALL-KEYWORDS) 支持分布式索引
* [UTC 分组](Server_settings/Searchd.md#grouping_in_utc)
* [query_log_mode](Server_settings/Searchd.md#query_log_mode) 支持自定义日志文件权限
* 字段权重可以为零或负数
* [max_query_time](Searching/Options.md#max_query_time) 现在可以影响全扫描
* 新增 [net_wait_tm](Server_settings/Searchd.md#net_wait_tm)、[net_throttle_accept](Server_settings/Searchd.md#net_throttle_accept) 和 [net_throttle_action](Server_settings/Searchd.md#net_throttle_action) 用于网络线程的细粒度调优（当 workers=thread_pool 时）
* COUNT DISTINCT 支持 facet 搜索
* IN 可以用于 JSON 浮点数组
* 多查询优化不再被整数/浮点表达式破坏
* [SHOW META](Node_info_and_management/SHOW_META.md) 在使用多查询优化时显示 `multiplier` 行
### 编译

Manticore Search 使用 cmake 构建，最低 gcc 版本要求为 4.7.2。
### 文件夹和服务

* Manticore Search 以 `manticore` 用户身份运行。
* 默认数据文件夹现在是 `/var/lib/manticore/`。
* 默认日志文件夹现在是 `/var/log/manticore/`。
* 默认 pid 文件夹现在是 `/var/run/manticore/`。
### Bugfixes

* [Commit a58c](https://github.com/manticoresoftware/manticore/commit/a58c619) 修复了导致 Java 连接器出错的 SHOW COLLATION 语句
* [Commit 631c](https://github.com/manticoresoftware/manticore/commit/631cf4e) 修复了处理分布式索引时的崩溃；为分布式索引哈希添加锁；移除了代理的移动和复制操作符
* [Commit 942b](https://github.com/manticoresoftware/manticore/commit/942bec0) 修复了因并行重连导致的分布式索引处理崩溃
* [Commit e5c1](https://github.com/manticoresoftware/manticore/commit/e5c1ed2) 修复了在将查询存储到服务器日志时崩溃处理器崩溃的问题
* [Commit 4a4b](https://github.com/manticoresoftware/manticore/commit/4a4bda5) 修复了多查询中使用池化属性时的崩溃
* [Commit 3873](https://github.com/manticoresoftware/manticore/commit/3873bfb) 通过防止索引页被包含进核心文件，减少了核心文件大小
* [Commit 11e6](https://github.com/manticoresoftware/manticore/commit/11e6254) 修复了指定无效代理时 searchd 启动崩溃
* [Commit 4ca6](https://github.com/manticoresoftware/manticore/commit/4ca6350) 修复了索引器在 sql_query_killlist 查询中报告错误
* [Commit 123a](https://github.com/manticoresoftware/manticore/commit/123a9f0) 修复了 fold_lemmas=1 与命中计数的问题
* [Commit cb99](https://github.com/manticoresoftware/manticore/commit/cb99164) 修复了 html_strip 行为不一致的问题
* [Commit e406](https://github.com/manticoresoftware/manticore/commit/e406761) 修复了优化 rt 索引时丢失新设置的问题；修复了带同步选项的优化锁泄漏；
* [Commit 86ae](https://github.com/manticoresoftware/manticore/commit/86aeb82) 修复了错误多查询的处理问题
* [Commit 2645](https://github.com/manticoresoftware/manticore/commit/2645230) 修复了结果集依赖于多查询顺序的问题
* [Commit 7239](https://github.com/manticoresoftware/manticore/commit/72395d9) 修复了多查询中出现错误查询时服务器崩溃的问题
* [Commit f353](https://github.com/manticoresoftware/manticore/commit/f353326) 修复了从共享锁到独占锁的问题
* [Commit 3754](https://github.com/manticoresoftware/manticore/commit/3754785) 修复了无索引查询导致的服务器崩溃
* [Commit 29f3](https://github.com/manticoresoftware/manticore/commit/29f360e) 修复了服务器死锁问题
## Version 2.3.3, 2017年7月6日

* Manticore 品牌标识
* Manticore branding

