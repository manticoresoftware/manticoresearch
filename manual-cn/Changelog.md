# 更新日志

# Version x.y.z (dev)

### 主要问题
* [Issue #1928](https://github.com/manticoresoftware/manticoresearch/issues/1928) 引入了 [JSON 的二级索引](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#json_secondary_indexes)。
* [Issue #2361](https://github.com/manticoresoftware/manticoresearch/issues/2361) 在更新期间进行的[更新](Data_creation_and_modification/Updating_documents/UPDATE.md#UPDATE)和搜索不再因块合并而被阻塞。
* ⚠️ 重大变更 [Issue #879](https://github.com/manticoresoftware/manticoresearch/issues/879) 引入了 [按表的 binlog](Logging/Binary_logging.md#Per-table-binary-logging-configuration) 和新选项：[binlog_common](Logging/Binary_logging.md#Binary-logging-strategies)，[binlog](Logging/Binary_logging.md#Per-table-binary-logging-configuration) 用于 `create table` / `alter table`。

### 次要问题
* [Issue #1103](https://github.com/manticoresoftware/manticoresearch/issues/1103) 改进了 [日志记录](Logging/Server_logging.md#Server-logging) 块合并的清晰度。
* [Issue #1130](https://github.com/manticoresoftware/manticoresearch/issues/1130) 增加了对 [DBeaver](Integration/DBeaver.md) 的支持。
* [Issue #1546](https://github.com/manticoresoftware/manticoresearch/issues/1546) 为 [POLY2D()](Functions/Geo_spatial_functions.md#POLY2D())/[GEOPOLY2D()](Functions/Geo_spatial_functions.md#GEOPOLY2D()) 函数实现了二级索引。
* [Issue #1630](https://github.com/manticoresoftware/manticoresearch/issues/1630) HTTP 请求现在支持 `Content-Encoding: gzip`。
* [Issue #2187](https://github.com/manticoresoftware/manticoresearch/issues/2197) 允许 Buddy 请求绕过 [searchd.max_connections](Server_settings/Searchd.md#max_connections) 约束。
* [Issue #2402](https://github.com/manticoresoftware/manticoresearch/issues/2402) 将 [searchd.max_packet_size](Server_settings/Searchd.md#max_packet_size) 默认值更新为 128MB。
* [Issue #2419](https://github.com/manticoresoftware/manticoresearch/issues/2419) 增加了对 JSON ["match"](Searching/Full_text_matching/Basic_usage.md#match) 中 [IDF 增强修饰符](Searching/Full_text_matching/Operators.md#IDF-boost-modifier) 的支持。
* [Issue #2430](https://github.com/manticoresoftware/manticoresearch/issues/2430) 增强了 [binlog](Logging/Binary_logging.md#Binary-logging) 写入同步以防止错误。
* [Issue #2490](https://github.com/manticoresoftware/manticoresearch/issues/2490) 在兼容端点的 aggs 中实现了 msc 缩放。
* [Issue #268](https://github.com/manticoresoftware/manticoresearch-buddy/issues/268) 实现了从 Elasticsearch 导入数据类型的自动检测。
* ⚠️ 重大变更 [Issue #2458](https://github.com/manticoresoftware/manticoresearch/issues/2458) 在 Windows 包中加入了 zlib 支持。
* ⚠️ 重大变更 [Issue #2468](https://github.com/manticoresoftware/manticoresearch/issues/2468) [CALL SUGGEST / QSUGGEST](Searching/Spell_correction.md#CALL-QSUGGEST,-CALL-SUGGEST) 现已兼容分布式表。

### Bug 修复
* [Issue #2335](https://github.com/manticoresoftware/manticoresearch/issues/2335) 修复了在条件中使用两个 json 属性时表连接失败的问题。
* [Issue #2338](https://github.com/manticoresoftware/manticoresearch/issues/2338) 修正了带有 [cutoff](Searching/Options.md#cutoff) 的多查询中的错误 total_relation。
* [Issue #2366](https://github.com/manticoresoftware/manticoresearch/issues/2366) 修正了在 [表连接](Searching/Joining.md) 中根据 `json.string` 进行的过滤。
* [Issue #2406](https://github.com/manticoresoftware/manticoresearch/issues/2406) 启用了在所有 POST HTTP JSON 端点 (insert/replace/bulk) 中使用 `null` 值，在这种情况下将使用默认值。
* [Issue #2418](https://github.com/manticoresoftware/manticoresearch/issues/2418) 通过调整初始套接字探测的 [max_packet_size](Server_settings/Searchd.md#max_packet_size) 网络缓冲区分配优化了内存消耗。
* [Issue #2420](https://github.com/manticoresoftware/manticoresearch/issues/2420) 修正了通过 JSON 接口向 bigint 属性插入无符号整数的问题。
* [Issue #2422](https://github.com/manticoresoftware/manticoresearch/issues/2422) 修复了带有排除过滤器和伪分片启用的二级索引问题。
* [Issue #2423](https://github.com/manticoresoftware/manticoresearch/issues/2423) 解决了 [manticore_new_cluster](Starting_the_server/Manually.md#searchd-command-line-options) 中的错误。
* [Issue #2448](https://github.com/manticoresoftware/manticoresearch/issues/2448) 修复了格式错误的 `_update` 请求导致的守护程序崩溃。
* [Issue #2452](https://github.com/manticoresoftware/manticoresearch/issues/2452) 修复了直方图无法处理带有排除的值过滤器的问题。
* [Issue #2524](https://github.com/manticoresoftware/manticoresearch/issues/2524) 修复了带有排除过滤器的 JSON 属性的二级索引在别名表达式中失效的问题。
* [Issue #55](https://github.com/manticoresoftware/columnar/issues/55) 修复了针对分布式表的 [knn](Searching/KNN.md#KNN-vector-search) 查询。
* [Issue #68](https://github.com/manticoresoftware/columnar/issues/68) 增强了在列访问器中对表编码排除过滤器的处理。
* [Issue #1107](https://github.com/manticoresoftware/manticoresearch/issues/1107) 修复了在 `CREATE TABLE` 和 `ALTER TABLE` 中特殊字符的转义问题。
* [Issue #2001](https://github.com/manticoresoftware/manticoresearch/issues/2001) 允许函数名用作列名。
* [Issue #2153](https://github.com/manticoresoftware/manticoresearch/issues/2153) 修复了在查询具有未知磁盘块的表设置时守护程序崩溃的问题。
* [Issue #2228](https://github.com/manticoresoftware/manticoresearch/issues/2228) 从保留字中移除了日期/时间相关的标记和正则表达式。
* [Issue #2447](https://github.com/manticoresoftware/manticoresearch/issues/2447) 修复了 [_update](Data_creation_and_modification/Updating_documents/REPLACE.md?client=Elasticsearch-like+partial) 中的转义问题。
* [Issue #2460](https://github.com/manticoresoftware/manticoresearch/issues/2460) 修复了在索引器中声明多个相同名称的属性或字段时的崩溃问题。
* [Issue #2467](https://github.com/manticoresoftware/manticoresearch/issues/2467) 解决了在“兼容”搜索相关端点中嵌套布尔查询转换错误导致的守护程序崩溃。
* [Issue #2493](https://github.com/manticoresoftware/manticoresearch/issues/2493) 修复了带修饰符的短语扩展问题。
* [Issue #2509](https://github.com/manticoresoftware/manticoresearch/issues/2509) 修正了关于“ubertests”中空 binlog 的警告消息。
* [Issue #2535](https://github.com/manticoresoftware/manticoresearch/issues/2535) 解决了在使用 [ZONE](../Searching/Full_text_matching/Operators.md#ZONE-limit-operator) 或 [ZONESPAN](Searching/Full_text_matching/Operators.md#ZONESPAN-limit-operator) 全文操作符时守护程序崩溃的问题。
* ⚠️ 重大变更 [Issue #2103](https://github.com/manticoresoftware/manticoresearch/issues/2103) 从内部 `cjk` 字符集中移除了泰语字符。请相应地更新您的字符集定义：如果使用 `cjk,non_cjk`，请更改为 `cjk,thai,non_cjk` 或 `cont,non_cjk`，其中 `cont` 是所有连续书写语言的新标识（即 `cjk` + `thai`）。使用 [ALTER TABLE](Updating_table_schema_and_settings.md#Updating-table-FT-settings-in-RT-mode) 修改现有表。

### 复制相关
* ⚠️ 重大变更 [Issue #1789](https://github.com/manticoresoftware/manticoresearch/issues/1789) Fixed incorrect error message when a node joins a cluster with the wrong replication protocol version.
* ⚠️ 重大变更 [Issue #2308](https://github.com/manticoresoftware/manticoresearch/issues/2308) Added support for multiple tables in [ALTER CLUSTER ADD and DROP](../Creating_a_cluster/Setting_up_replication/Adding_and_removing_a_table_from_a_replication_cluster.md#Adding-and-removing-a-table-from-a-replication-cluster).

# manticore-extra v1.1.20

发布于：2024年10月7日

### Bug 修复

* [Issue #64](https://github.com/manticoresoftware/executor/issues/64) 解决了 `unattended-upgrades` 工具在 Debian 系统中自动安装包更新时错误地将多个 Manticore 包（如 `manticore-galera`、`manticore-executor` 和 `manticore-columnar-lib`）标记为可移除的问题。这是由于 `dpkg` 错误地将虚拟包 `manticore-extra` 视为冗余而引起的。已作出调整，确保 `unattended-upgrades` 不再试图移除重要的 Manticore 组件。

# Version 6.3.6
发布于：2024年8月2日

6.3.6 版延续了 6.3 系列，仅包括 Bug 修复。

### Bug 修复

* [Issue #2477](https://github.com/manticoresoftware/manticoresearch/issues/2477) 修复了在 6.3.4 版本中引入的导致分布式或多表中处理表达式时可能发生的崩溃问题。
* [Issue #2352](https://github.com/manticoresoftware/manticoresearch/issues/2352) 修复了在查询多个索引时，由于 `max_query_time` 导致的提前退出引发的守护进程崩溃或内部错误。

# Version 6.3.4
发布于：2024年7月31日

6.3.4 版延续了 6.3 系列，仅包含次要改进和 Bug 修复。

### 次要问题
* [Issue #1130](https://github.com/manticoresoftware/manticoresearch/issues/1130) 增加了对 [DBeaver](Integration/DBeaver.md#Integration-with-DBeaver) 的支持。
* [Issue #2146](https://github.com/manticoresoftware/manticoresearch/issues/2146) 改进了词形变化和例外中的分隔符转义。
* [Issue #2268](https://github.com/manticoresoftware/manticoresearch/issues/2268) 改进了在 CREATE 和 ALTER TABLE 语句中复制外部文件时的重命名操作。
* [Issue #2315](https://github.com/manticoresoftware/manticoresearch/issues/2315) 为 SELECT 列表表达式添加了字符串比较运算符。
* [Issue #2363](https://github.com/manticoresoftware/manticoresearch/issues/2363) 在 Elastic-like 批量请求中增加了对 null 值的支持。
* [Issue #2374](https://github.com/manticoresoftware/manticoresearch/issues/2374) 增加了对 mysqldump 版本 9 的支持。
* [Issue #2375](https://github.com/manticoresoftware/manticoresearch/issues/2375) 改进了 HTTP JSON 查询中的错误处理，以提供 JSON 路径到错误发生的节点。

### Bug 修复
* [Issue #2280](https://github.com/manticoresoftware/manticoresearch/issues/2280) 修复了当 disk_chunks > 1 时，带有许多匹配项的通配符查询中的性能下降问题。
* [Issue #2332](https://github.com/manticoresoftware/manticoresearch/issues/2332) 修复了 MVA MIN 或 MAX SELECT 列表表达式在空 MVA 数组中的崩溃问题。
* [Issue #2339](https://github.com/manticoresoftware/manticoresearch/issues/2339) 修正了 Kibana 无穷范围请求的错误处理。
* [Issue #2342](https://github.com/manticoresoftware/manticoresearch/issues/2342) 修复了右表的 columnar 属性不在 SELECT 列表中时的连接过滤问题。
* [Issue #2343](https://github.com/manticoresoftware/manticoresearch/issues/2343) 修正了 Manticore 6.3.2 中重复的 'static' 修饰符。
* [Issue #2344](https://github.com/manticoresoftware/manticoresearch/issues/2344) 修复了在使用右表的 MATCH() 时 LEFT JOIN 返回不匹配条目。
* [Issue #2350](https://github.com/manticoresoftware/manticoresearch/issues/2350) 修复了 RT 索引中带 `hitless_words` 的磁盘块保存问题。
* [Issue #2364](https://github.com/manticoresoftware/manticoresearch/issues/2364) 允许在其他属性中以任意顺序添加 'aggs_node_sort' 属性。
* [Issue #2368](https://github.com/manticoresoftware/manticoresearch/issues/2368) 修正了 JSON 查询中全文与过滤顺序的错误。
* [Issue #2376](https://github.com/manticoresoftware/manticoresearch/issues/2376) 修复了长 UTF-8 请求的 JSON 响应错误。
* [Issue #2684](https://github.com/manticoresoftware/dev/issues/2684) 修复了依赖于连接属性的 presort/prefilter 表达式的计算问题。
* [Issue #301](https://github.com/manticoresoftware/manticoresearch-buddy/issues/301) 更改了读取 `manticore.json` 文件中数据大小的计算方法，而不是检查整个数据目录的大小。
* [Issue #302](https://github.com/manticoresoftware/manticoresearch-buddy/issues/302) 增加了对 Vector.dev 中验证请求的处理。

# Version 6.3.2
发布于：2024年6月26日

6.3.2 版本延续了 6.3 系列，并包含一些在 6.3.0 发布后发现的 Bug 修复。

### 重大变更
* ⚠️[Issue #2305](https://github.com/manticoresoftware/manticoresearch/issues/2305) 将 `aggs.range` 值更新为数值格式。

### Bug 修复
* [Commit c51c](https://github.com/manticoresoftware/manticoresearch/commit/c51c7fa4bd0ec4ad8d53188b888461bff29f1417) 修复了分组时的存储检查与结果集合并问题。
* [Commit 0e85](https://github.com/manticoresoftware/manticoresearch/commit/0e85b9d5682f51681750d798aef6a11cfb93d69e) 修复了在启用 CRC 字典和 `local_df` 的 RT 索引中查询带有通配符字符时守护进程的崩溃问题。
* [Issue #2200](https://github.com/manticoresoftware/manticoresearch/issues/2200) 修复了在 `count(*)` 没有 GROUP BY 时的 JOIN 崩溃问题。
* [Issue #2201](https://github.com/manticoresoftware/manticoresearch/issues/2201) 修复了尝试按全文字段分组时 JOIN 未返回警告的问题。
* [Issue #2230](https://github.com/manticoresoftware/manticoresearch/issues/2230) 解决了通过 `ALTER TABLE` 添加属性未考虑 KNN 选项的问题。
* [Issue #2231](https://github.com/manticoresoftware/manticoresearch/issues/2231) 修复了 6.3.0 版本中无法删除 Redhat 的 `manticore-tools` 包的问题。
* [Issue #2242](https://github.com/manticoresoftware/manticoresearch/issues/2242) 修正了多 FACET 语句和 JOIN 返回错误结果的问题。
* [Issue #2250](https://github.com/manticoresoftware/manticoresearch/issues/2250) 修复了在集群中的表执行 ALTER TABLE 时出现的错误。
* [Issue #2252](https://github.com/manticoresoftware/manticoresearch/issues/2252) 修复了 SphinxQL 接口将原始查询传递到 buddy 中的问题。
* [Issue #2267](https://github.com/manticoresoftware/manticoresearch/issues/2267) 改进了带磁盘块的 RT 索引中 `CALL KEYWORDS` 的通配符扩展。
* [Issue #271](https://github.com/manticoresoftware/manticoresearch-buddy/issues/271) 修复了错误的 `/cli` 请求导致挂起的问题。
* [Issue #274](https://github.com/manticoresoftware/manticoresearch-buddy/issues/274) 解决了对 Manticore 的并发请求可能卡住的问题。
* [Issue #275](https://github.com/manticoresoftware/manticoresearch-buddy/issues/275) 修复了通过 `/cli` 执行 `drop table if exists t; create table t` 时挂起的问题。

### 复制相关
* [Issue #2270](https://github.com/manticoresoftware/manticoresearch/issues/2270) 增加了 `/_bulk` HTTP 端点中 `cluster:name` 格式的支持。

# Version 6.3.0
发布日期：2024年5月23日

### 主要变更
* [Issue #839](https://github.com/manticoresoftware/manticoresearch/issues/839) 实现了 [float_vector](Creating_a_table/Data_types.md#Float-vector) 数据类型；实现了 [向量搜索](Searching/KNN.md#KNN-vector-search)。
* [Issue #1673](https://github.com/manticoresoftware/manticoresearch/issues/1673) 支持 [INNER/LEFT JOIN](Searching/Joining.md) (**beta 阶段**)。
* [Issue #1744](https://github.com/manticoresoftware/manticoresearch/issues/1744) 为 [timestamp](../Creating_a_table/Data_types.md#Timestamps) 字段实现了日期格式自动检测。
* [Issue #1720](https://github.com/manticoresoftware/manticoresearch/pull/1720) 将 Manticore Search 许可证从 GPLv2-or-later 改为 GPLv3-or-later。
* [Commit 7a55](https://github.com/manticoresoftware/manticoresearch/commit/7a5508424) Windows 上运行 Manticore 现在需要 Docker 来运行 Buddy。
* [Issue #1541](https://github.com/manticoresoftware/manticoresearch/issues/1541) 添加了 [REGEX](Searching/Full_text_matching/Operators.md#REGEX-operator) 全文操作符。
* [Issue #2091](https://github.com/manticoresoftware/manticoresearch/issues/2091) 支持 Ubuntu Noble 24.04。
* [Commit 514d](https://github.com/manticoresoftware/manticoresearch/commit/514d35b497f4bdb20b6473ff963752a978a4bb8d) 改进了时间操作的性能，并添加了新的日期/时间函数：
  - [CURDATE()](Functions/Date_and_time_functions.md#CURDATE%28%29) - 返回当前时区的当前日期
  - [QUARTER()](Functions/Date_and_time_functions.md#QUARTER%28%29) - 从时间戳参数返回当前季度（整数）
  - [DAYNAME()](Functions/Date_and_time_functions.md#DAYNAME%28%29) - 从时间戳参数返回星期几的名称
  - [MONTHNAME()](Functions/Date_and_time_functions.md#MONTHNAME%28%29) - 从时间戳参数返回月份名称
  - [DAYOFWEEK()](Functions/Date_and_time_functions.md#DAYOFWEEK%28%29) - 从时间戳参数返回星期几的索引（整数）
  - [DAYOFYEAR()](Functions/Date_and_time_functions.md#DAYOFYEAR%28%29) - 从时间戳参数返回一年中的第几天（整数）
  - [YEARWEEK()](Functions/Date_and_time_functions.md#YEARWEEK%28%29) - 从时间戳参数返回当前周的年份和首日代码（整数）
  - [DATEDIFF()](Functions/Date_and_time_functions.md#DATEDIFF%28%29) - 返回两个时间戳之间的天数差
  - [DATE()](Functions/Date_and_time_functions.md#DATE%28%29) - 格式化时间戳参数中的日期部分
  - [TIME()](Functions/Date_and_time_functions.md#TIME%28%29) - 格式化时间戳参数中的时间部分
  - [timezone](Server_settings/Searchd.md#timezone) - 日期/时间相关函数使用的时区设置。
* [Commit 30e7](https://github.com/manticoresoftware/manticoresearch/commit/30e789d9657f9b3093c280c6e5b0e46a021f6dc7) 在 HTTP 接口中添加了 [range](Searching/Faceted_search.md#Facet-over-set-of-ranges)、[histogram](Searching/Faceted_search.md#Facet-over-histogram-values)、[date_range](Searching/Faceted_search.md#Facet-over-set-of-date-ranges) 和 [date_histogram](Searching/Faceted_search.md#Facet-over-histogram-date-values) 聚合功能，并在 SQL 中提供了类似的表达式。

### 次要问题
* [Issue #1285](https://github.com/manticoresoftware/manticoresearch/issues/1285) 支持 Filebeat 版本 8.10 - 8.11。
* [Issue #1771](https://github.com/manticoresoftware/manticoresearch/issues/1771) [ALTER TABLE ... type='distributed'](Updating_table_schema_and_settings.md#Changing-a-distributed-table).
* [Issue #1788](https://github.com/manticoresoftware/manticoresearch/issues/1788) 添加了通过 [CREATE TABLE ... LIKE ... WITH DATA](Creating_a_table/Local_tables/Real-time_table.md#CREATE-TABLE-LIKE:) SQL 语句复制表的功能。
* [Issue #2072](https://github.com/manticoresoftware/manticoresearch/issues/2072) 优化了[表压缩算法](Securing_and_compacting_a_table/Compacting_a_table.md#Compacting-a-Table)：此前，手动 [OPTIMIZE](Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE) 和自动 [auto_optimize](Server_settings/Searchd.md#auto_optimize) 过程会首先合并块以确保块数不超过限制，然后从所有包含已删除文档的其他块中清除已删除的文档。此方法有时过于资源密集，现已禁用。现在，块合并仅根据 [progressive_merge](Server_settings/Common.md#progressive_merge) 设置进行。但是，具有大量已删除文档的块更有可能被合并。
* [Commit ce6c](https://github.com/manticoresoftware/columnar/commit/ce6c5d3381edcdc411a00a5923d13d56dd9b3010) 添加了防止加载较新版本次级索引的保护措施。
* [Issue #1417](https://github.com/manticoresoftware/manticoresearch/issues/1417) 通过 [REPLACE INTO ... SET](Data_creation_and_modification/Updating_documents/REPLACE.md#SQL-REPLACE) 实现部分替换。
* [Commit 7c16](https://github.com/manticoresoftware/manticoresearch/commit/7c167b934ce416d3dfbd658744a231e638b14d9a) 更新了默认合并缓冲区大小：`.spa`（标量属性）：256KB -> 8MB；`.spb`（blob 属性）：256KB -> 8MB；`.spc`（列属性）：1MB，无变化；`.spds`（docstore）：256KB -> 8MB；`.spidx`（次级索引）：256KB 缓冲区 -> 128MB 内存限制；`.spi`（字典）：256KB -> 16MB；`.spd`（文档列表）：8MB，无变化；`.spp`（命中列表）：8MB，无变化；`.spe`（跳表）：256KB -> 8MB。
* [Issue #1859](https://github.com/manticoresoftware/manticoresearch/issues/1859) 通过 JSON 添加了[复合聚合](Searching/Grouping.md#GROUP-BY-multiple-fields-at-once)。
* [Commit 216b](https://github.com/manticoresoftware/manticoresearch-buddy/commit/216b824) 因一些正则表达式模式的问题且无明显时间收益，禁用了 PCRE.JIT。
* [Commit 55cd](https://github.com/manticoresoftware/manticoresearch/commit/55cdce32) 添加了对原生 Galera v.3（api v25）（来自 MySQL 5.x 的 `libgalera_smm.so`）的支持。
* [Commit 86f9](https://github.com/manticoresoftware/manticoresearch-buddy/commit/86f98ab) 将指标后缀从 `_rate` 更改为 `_rps`。
* [Commit c0c1](https://github.com/manticoresoftware/manticoresearch-helm/commit/c0c1cff780031412ef57cefab987dfd16613368e) 改进了有关负载均衡 HA 支持的文档。
* [Commit d1d2](https://github.com/manticoresoftware/manticoresearch/commit/d1d2ca18857c68106e89743756260e73285be739) 在错误消息中将 `index` 更改为 `table`；修复了 bison 解析器错误消息。
* [Commit fd26](https://github.com/manticoresoftware/manticoresearch/commit/fd26671b) 支持将 `manticore.tbl` 作为表名。
* [Issue #1105](https://github.com/manticoresoftware/manticoresearch/issues/1105) 支持通过 systemd 运行indexer（[文档](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Schedule-indexer-via-systemd)）。❤️ 感谢 [@subnix](https://github.com/subnix) 的 PR。
* [Issue #1294](https://github.com/manticoresoftware/manticoresearch/issues/1294) 在 GEODIST() 中支持次级索引。
* [Issue #1394](https://github.com/manticoresoftware/manticoresearch/issues/1394) 简化了 [SHOW THREADS](Node_info_and_management/SHOW_THREADS.md#SHOW-THREADS)。
* [Issue #1424](https://github.com/manticoresoftware/manticoresearch/issues/1424) 在 `create distributed table` 语句中添加了对默认值（`agent_connect_timeout` 和 `agent_query_timeout`）的支持。
* [Issue #1442](https://github.com/manticoresoftware/manticoresearch/issues/1442) 添加了 [expansion_limit](Searching/Options.md#expansion_limit) 搜索查询选项，可覆盖 `searchd.expansion_limit`。
* [Issue #1448](https://github.com/manticoresoftware/manticoresearch/issues/1448) 实现了 int->bigint 转换的 [ALTER TABLE](Updating_table_schema_and_settings.md#Updating-table-schema-in-RT-mode)。
* [Issue #146](https://github.com/manticoresoftware/manticoresearch/issues/1456) MySQL 响应中的元信息。
* [Issue #1494](https://github.com/manticoresoftware/manticoresearch/issues/1494) [SHOW VERSION](Node_info_and_management/SHOW_VERSION.md#SHOW-VERSION).
* [Issue #1582](https://github.com/manticoresoftware/manticoresearch/issues/1582) 通过 JSON 支持[按 ID 数组删除文档](Data_creation_and_modification/Deleting_documents.md?client=JSON#Deleting-documents)。
* [Issue #1589](https://github.com/manticoresoftware/manticoresearch/issues/1589) 改进错误提示“unsupported value type”。
* [Issue #1634](https://github.com/manticoresoftware/manticoresearch/issues/1634) 在 `SHOW STATUS` 中添加了 Buddy 版本。
* [Issue #1641](https://github.com/manticoresoftware/manticoresearch/issues/1641) 在关键字没有文档的情况下优化匹配请求。
* [Issue #1712](https://github.com/manticoresoftware/manticoresearch/issues/1712) 添加了将字符串值 "true" 和 "false" 转换为 bool 属性的功能。
* [Issue #1713](https://github.com/manticoresoftware/manticoresearch/issues/1713) 添加了 [access_dict](Server_settings/Searchd.md#access_dict) 表和 searchd 选项。
* [Issue #1767](https://github.com/manticoresoftware/manticoresearch/issues/1767) 在 searchd 配置部分中添加了新选项：[expansion_merge_threshold_docs](Server_settings/Searchd.md#expansion_merge_threshold_docs) 和 [expansion_merge_threshold_hits](Server_settings/Searchd.md#expansion_merge_threshold_hits)，使扩展术语合并的阈值可配置。
* [Issue #1768](https://github.com/manticoresoftware/manticoresearch/issues/1768) 在 `@@system.sessions` 中添加了上次命令时间的显示。
* [Issue #1806](https://github.com/manticoresoftware/manticoresearch/issues/1806) 升级了 Buddy 协议到版本 2。
* [Issue #1810](https://github.com/manticoresoftware/manticoresearch/issues/1810) 为 `/sql` 端点实现了额外的请求格式，以简化与库的集成。
* [Issue #1825](https://github.com/manticoresoftware/manticoresearch/pull/1825) 在 SHOW BUDDY PLUGINS 中添加了信息部分。
* [Issue #1837](https://github.com/manticoresoftware/manticoresearch/issues/1837) 在 `CALL PQ` 中改进了大数据包的内存消耗。
* [Issue #1853](https://github.com/manticoresoftware/manticoresearch/pull/1853) 编译器从 Clang 15 切换到 Clang 16。
* [Issue #1857](https://github.com/manticoresoftware/manticoresearch/issues/1857) 添加了字符串比较。❤️ 感谢 [@etcd](https://github.com/etcd) 的 PR。
* [Issue #1915](https://github.com/manticoresoftware/manticoresearch/issues/1915) 添加了对已存储字段的支持。
* [Issue #1937](https://github.com/manticoresoftware/manticoresearch/issues/1937) 在查询日志中记录客户端的 host。
* [Issue #1981](https://github.com/manticoresoftware/manticoresearch/issues/1981) 修复了错误提示。
* [Issue #1983](https://github.com/manticoresoftware/manticoresearch/issues/1983) 引入了 JSON [查询计划](Node_info_and_management/Profiling/Query_plan.md?client=JSON#Query-plan) 的详细级别支持。
* [Issue #2010](https://github.com/manticoresoftware/manticoresearch/issues/2010) 禁用了 Buddy 的查询日志，除非设置了 `log_level=debug`。
* [Issue #2035](https://github.com/manticoresoftware/manticoresearch/issues/2035) 支持 Linux Mint 21.3。
* [Issue #2056](https://github.com/manticoresoftware/manticoresearch/issues/2056) Manticore 无法与 MySQL 8.3+ 一起构建。
* [Issue #2112](https://github.com/manticoresoftware/manticoresearch/issues/2112) `DEBUG DEDUP` 命令，用于包含重复条目的实时表块。
* [Issue #212](https://github.com/manticoresoftware/manticoresearch-buddy/issues/212) 在 [SHOW QUERIES](Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES) 中添加时间。
* [Issue #218](https://github.com/manticoresoftware/manticoresearch-buddy/issues/218) 处理 `@timestamp` 列作为时间戳。
* [Issue #252](https://github.com/manticoresoftware/manticoresearch-buddy/issues/250) 实现了启用/禁用 buddy 插件的逻辑。
* [Issue #254](https://github.com/manticoresoftware/manticoresearch-buddy/issues/254) 更新 composer 至更新版本，以修复最近的 CVE。
* [Issue #340](https://github.com/manticoresoftware/manticoresearch/issues/340) 在与 `RuntimeDirectory` 相关的 Manticore systemd 单元中进行了小优化。
* [Issue #51](https://github.com/manticoresoftware/executor/issues/51) 添加了 rdkafka 支持并更新至 PHP 8.3.3。
* [Issue #527](https://github.com/manticoresoftware/manticoresearch/issues/527) 支持[附加](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Attaching_one_table_to_another.md#Attaching-one-table-to-another) RT 表。新命令 [ALTER TABLE ... RENAME](Updating_table_schema_and_settings.md#Renaming-a-real-time-table)。

### 重大变更和弃用
* ⚠️[Issue #1436](https://github.com/manticoresoftware/manticoresearch/issues/1436) 修复了 IDF 计算问题。`local_df` 现在是默认值。改进了主代理搜索协议（版本已更新）。如果您在多实例分布式环境中运行 Manticore Search，请确保先升级代理节点，然后升级主节点。
* ⚠️[Issue #1572](https://github.com/manticoresoftware/manticoresearch/issues/1572) 添加了分布式表的复制功能并更新了复制协议。如果您在运行复制集群，需要：
  - 首先，干净地停止所有节点
  - 然后，使用 Linux 工具 `manticore_new_cluster` 并使用 `--new-cluster` 选项启动最后一个停止的节点。
  - 阅读有关[重启集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster)的详细信息。
* ⚠️[Issue #1763](https://github.com/manticoresoftware/manticoresearch/pull/1763) HTTP API 端点别名 `/json/*` 已被弃用。
* ⚠️[Issue #1982](https://github.com/manticoresoftware/manticoresearch/issues/1982) 在 JSON 中将 [profile](Node_info_and_management/Profiling/Query_profile.md#Query-profile) 更改为 [plan](Node_info_and_management/Profiling/Query_plan.md#Query-plan)，并为 JSON 添加了查询分析功能。
* ⚠️[Commit e235](https://github.com/manticoresoftware/manticoresearch-backup/commit/e23585a) manticore-backup 不再备份 `plugin_dir`。
* ⚠️[Issue #171](https://github.com/manticoresoftware/manticoresearch-buddy/issues/155) 将 Buddy 迁移到 Swoole 以提升性能和稳定性。切换到新版本时，请确保所有 Manticore 包已更新。
* ⚠️[Issue #196](https://github.com/manticoresoftware/manticoresearch-buddy/pull/205) 将所有核心插件合并到 Buddy 并更改了核心逻辑。
* ⚠️[Issue #2107](https://github.com/manticoresoftware/manticoresearch/issues/2107) 在 `/search` 响应中将文档 ID 视为数字。
* ⚠️[Issue #38](https://github.com/manticoresoftware/executor/issues/38) 添加了 Swoole，禁用了 ZTS，并移除了并行扩展。

### 复制相关的更改
* [Commit 3376](https://github.com/manticoresoftware/manticoresearch/commit/3376351f73db3ff0cb6cb708a304a7bd83e69cfb) 修复了大文件在 SST 复制中的错误。
* [Commit 6d36](https://github.com/manticoresoftware/manticoresearch/commit/6d36c68fb7ce3cb274b467aac435788905084952) 添加了复制命令的重试机制；修复了在高负载网络丢包情况下的复制连接失败。
* [Commit 842e](https://github.com/manticoresoftware/manticoresearch/commit/842e27e71790833ab4125d49103d2933d5df60c3) 将复制中的 FATAL 消息更改为 WARNING 消息。
* [Commit 8c32](https://github.com/manticoresoftware/manticoresearch/commit/8c32bce7db39d8ed3b1a06aaa1d2c3ec372084f0) 修复了针对无表或空表的复制集群的 `gcache.page_size` 计算；修复了 Galera 选项的保存和加载。
* [Commit a2af](https://github.com/manticoresoftware/manticoresearch/commit/a2af06ca36abcbf893e43c8803853130b397c54b) 增加了在加入集群的节点上跳过更新节点复制命令的功能。
* [Commit c054](https://github.com/manticoresoftware/manticoresearch/commit/c0541e6892f5116b1f957b40060ba8ecd01a5afb) 修复了在复制过程中更新 blob 属性与替换文档时产生的死锁。
* [Commit e80d](https://github.com/manticoresoftware/manticoresearch/commit/e80d505b963999016033613c40c1cf3151d648e7) 添加了 [replication_connect_timeout](Server_settings/Searchd.md#replication_connect_timeout)、[replication_query_timeout](Server_settings/Searchd.md#replication_query_timeout)、[replication_retry_delay](Server_settings/Searchd.md#replication_retry_delay)、[replication_retry_count](Server_settings/Searchd.md#replication_retry_count) 等 `searchd` 配置选项，用于控制复制期间的网络设置，类似于 `searchd.agent_*`，但具有不同的默认值。
* [Issue #1356](https://github.com/manticoresoftware/manticoresearch/issues/1356) 修复了在某些节点丢失且名称解析失败的情况下的复制节点重试。
* [Issue #1445](https://github.com/manticoresoftware/manticoresearch/issues/1445) 修复了 `show variables` 中的复制日志详细级别问题。
* [Issue #1482](https://github.com/manticoresoftware/manticoresearch/issues/1482) 修复了在 Kubernetes 中重启的 pod 上连接到集群的加入节点的复制问题。
* [Issue #1962](https://github.com/manticoresoftware/manticoresearch/issues/1962) 修复了由于节点名称无效导致的空集群中复制更改的长时间等待。

### Bug 修复
* [Commit 8a48](https://github.com/manticoresoftware/manticoresearch/commit/8a486c7ba1a293879cdecc41fb52eb7ab0cbf832) 修复了 `count distinct` 中未使用的匹配项清理问题，防止可能导致的崩溃。
* [Issue #1569](https://github.com/manticoresoftware/manticoresearch/issues/1569) 二进制日志现在以事务粒度写入。
* [Issue #2089](https://github.com/manticoresoftware/manticoresearch/issues/2089) 修复了与 64 位 ID 相关的错误，该错误可能导致通过 MySQL 插入数据时出现“Malformed packet”错误，进而导致[表损坏和 ID 重复](https://github.com/manticoresoftware/manticoresearch/issues/2090)。
* [Issue #2160](https://github.com/manticoresoftware/manticoresearch/issues/2160) 修复了插入日期时被当作 UTC 而非本地时区的问题。
* [Issue #2177](https://github.com/manticoresoftware/manticoresearch/issues/2177) 修复了在具有非空 `index_token_filter` 的实时表中执行搜索时发生的崩溃。
* [Issue #2209](https://github.com/manticoresoftware/manticoresearch/issues/2209) 更改了 RT 列式存储中的重复过滤逻辑，以修复崩溃和错误的查询结果。
* [Commit 001d](https://github.com/manticoresoftware/manticoresearch/commit/001d82904e4b887c49387308291aeefb07c0fbc4) 修复了处理联接字段后导致内存损坏的 html 剥离问题。
* [Commit 00eb](https://github.com/manticoresoftware/manticoresearch/commit/00eb2ebf) 避免了刷新后重绕流，以防止 mysqldump 通信问题。
* [Commit 0553](https://github.com/manticoresoftware/manticoresearch/commit/05534114acf08c70ffcc41a23d558dec09a34373) 如果预读尚未开始，不再等待其完成。
* [Commit 055a](https://github.com/manticoresoftware/manticoresearch/commit/055aac09f877570cd29b6594cb283182eba7f73b) 修复了将大型 Buddy 输出字符串拆分到 searchd 日志的多行问题。
* [Commit 0a88](https://github.com/manticoresoftware/manticoresearch/commit/0a88056307f82c50fcdc458ad6dbc307a00e3f92) 将 MySQL 接口中关于标题失败的警告移动到 `debugv` 详细级别。
* [Commit 150a](https://github.com/manticoresoftware/manticoresearch/commit/150a20286b7d98d24b844ae577d7fda4a444798d) 修复了多个集群管理操作的竞争条件； 禁止创建相同名称或路径的多个集群。
* [Commit 2e40](https://github.com/manticoresoftware/manticoresearch/commit/2e405c1c9ebac6f46206e3df17ac1c19129c9aa3) 修复了全文查询中的隐式截断，分割了 MatchExtended 到模板部分 D。
* [Commit 75f5](https://github.com/manticoresoftware/manticoresearch/commit/75f51421542a2a0658f9e2f9f31048c8c5224479) 修复了 `index_exact_words` 在索引和加载表到守护进程之间的差异。
* [Commit 7643](https://github.com/manticoresoftware/manticoresearch/commit/7643976bea142e08535d701000eff0d09a99fda2) 修复了无效集群删除时遗漏的错误消息。
* [Commit 7a03](https://github.com/manticoresoftware/manticoresearch/commit/7a034ac990d5ec9344320c05a6a2315583b9b858) 修复了 CBO 对队列联合和 RT 伪分片的支持。
* [Commit 7b4e](https://github.com/manticoresoftware/manticoresearch/commit/7b4ea95e) 在未加载二级索引 (SI) 库和配置参数的情况下启动时，已删除误导性的“警告：设置了 secondary_indexes 但未能初始化二级库”消息。
* [Commit 8496](https://github.com/manticoresoftware/manticoresearch/commit/8496317f099fade8d2db463fb88483e7fcdfe85b) 修复了在 quorum 中命中排序的问题。
* [Commit 8973](https://github.com/manticoresoftware/manticoresearch-buddy/commit/8973ad3) 修复了 ModifyTable 插件中大写选项的问题。
* [Commit 9935](https://github.com/manticoresoftware/manticoresearch-buddy/commit/99350d4c184379b19f3efa9751d9b5be5e6edfb5) 修复了从包含空 JSON 值（表示为 NULL）的转储恢复时的问题。
* [Commit a28f](https://github.com/manticoresoftware/manticoresearch/commit/a28f6e087b3d957e48816070be17d0823904936a) 使用 pcon 修复了在接收 SST 时连接节点的 SST 超时问题。
* [Commit b5a5](https://github.com/manticoresoftware/manticoresearch/commit/b5a55cc26debcddcda5d9caef8366a707e0dd763) 修复了选择别名字符串属性时的崩溃问题。
* [Commit c556](https://github.com/manticoresoftware/manticoresearch/commit/c556e7ed8b6da8eac8ef094650df5f0a8977916b) 在 `morphology_skip_fields` 字段中将术语转换为 `=term` 的全文查询。
* [Commit cdc3](https://github.com/manticoresoftware/manticoresearch/commit/cdc38085145cd1025b96fcac2a46188a0425d9e1) 增加了缺失的配置键 (skiplist_cache_size)。
* [Commit cf6e](https://github.com/manticoresoftware/manticoresearch/commit/cf6ec3bfdc3f0990fb31ff53822b2331a6667727) 修复了在使用复杂查询时表达式排序器崩溃的问题。
* [Commit e513](https://github.com/manticoresoftware/manticoresearch/commit/e5139ba75eef7223df375ae3cc3f138fa7d925f1) 修复了全文查询优化器 (CBO) 与无效索引提示的冲突。
* [Commit eb05](https://github.com/manticoresoftware/manticoresearch/commit/eb0514b3a6f57f278252b0724eebd112ec116e0f) 在关闭期间中断预读以加快关闭速度。
* [Commit f945](https://github.com/manticoresoftware/manticoresearch/commit/f94555a293cfa51004ff81135b8bd8500b955115) 修改了全文查询的堆栈计算，以避免复杂查询时的崩溃。
* [Issue #1262](https://github.com/manticoresoftware/manticoresearch/issues/1262) 修复了在索引包含相同列名的 SQL 源时索引器崩溃的问题。
* [Issue #1273](https://github.com/manticoresoftware/manticoresearch/issues/1273) 对于不存在的系统变量，返回 0 而非 <empty>。
* [Issue #1289](https://github.com/manticoresoftware/manticoresearch/issues/1289) 修复了检查实时表的外部文件时的 indextool 错误。
* [Issue #1335](https://github.com/manticoresoftware/manticoresearch/issues/1335) 修复了由于短语中的多词形式导致的查询解析错误。
* [Issue #1364](https://github.com/manticoresoftware/manticoresearch/issues/1364) 增加了使用旧版 binlog 版本回放空 binlog 文件的功能。
* [Issue #1365](https://github.com/manticoresoftware/manticoresearch/issues/1365) 修复了删除最后一个空 binlog 文件的问题。
* [Issue #1372](https://github.com/manticoresoftware/manticoresearch/issues/1372) 修复了在 `data_dir` 更改影响守护进程启动目录后，路径从相对路径转换为绝对路径的问题。
* [Issue #1393](https://github.com/manticoresoftware/manticoresearch/issues/1393) 修复了 hn_small 中的启动时间延迟问题：在守护进程启动时获取/缓存 CPU 信息。
* [Issue #1395](https://github.com/manticoresoftware/manticoresearch/issues/1395) 修复了加载索引时关于缺少外部文件的警告。
* [Issue #1402](https://github.com/manticoresoftware/manticoresearch/issues/1402) 修复了在全局分组器上释放数据指针属性时的崩溃问题。
* [Issue #1403](https://github.com/manticoresoftware/manticoresearch/issues/1403) _ADDITIONAL_SEARCHD_PARAMS 不起作用。
* [Issue #1427](https://github.com/manticoresoftware/manticoresearch/issues/1427) 修复了每个表的 `agent_query_timeout` 被默认查询选项 `agent_query_timeout` 替换的问题。
* [Issue #1444](https://github.com/manticoresoftware/manticoresearch/issues/1444) 修复了在使用 `packedfactors()` 时分组和排名器崩溃的问题，当每次匹配有多个值时。
* [Issue #1458](https://github.com/manticoresoftware/manticoresearch/issues/1458) 频繁索引更新时 Manticore 崩溃。
* [Issue #1481](https://github.com/manticoresoftware/manticoresearch/issues/1481) 修复了解析错误后清理解析的查询时的崩溃。
* [Issue #1484](https://github.com/manticoresoftware/manticoresearch/issues/1484) 修复了 HTTP JSON 请求未正确路由到 buddy 的问题。
* [Issue #1499](https://github.com/manticoresoftware/manticoresearch/issues/1499) JSON 属性根值不能是数组。已修复。
* [Issue #1507](https://github.com/manticoresoftware/manticoresearch/issues/1507) 修复了在事务中重新创建表时的崩溃问题。
* [Issue #1515](https://github.com/manticoresoftware/manticoresearch/issues/1515) 修复了 RU 词干的短形式扩展。
* [Issue #1579](https://github.com/manticoresoftware/manticoresearch/issues/1579) 修复了在 [date_format](Functions/Date_and_time_functions.md#DATE_FORMAT()) 表达式中使用 JSON 和 STRING 属性的问题。
* [Issue #1580](https://github.com/manticoresoftware/manticoresearch/issues/1580) 修复了 JSON 字段的多个别名分组器。
* [Issue #1594](https://github.com/manticoresoftware/manticoresearch/issues/1594) 开发版中的错误 `total_related`：修复了隐式截断与限制；在 JSON 查询中增加了更好的全扫描检测。
* [Issue #1603](https://github.com/manticoresoftware/manticoresearch/issues/1603) 修复了在所有日期表达式中使用 JSON 和 STRING 属性的问题。
* [Issue #1609](https://github.com/manticoresoftware/manticoresearch/issues/1609) 使用 LEVENSHTEIN() 时崩溃。
* [Issue #1612](https://github.com/manticoresoftware/manticoresearch/issues/1612) 修复了带高亮的搜索查询解析错误后内存损坏的问题。
* [Issue #1614](https://github.com/manticoresoftware/manticoresearch/issues/1614) 禁用了短于 `min_prefix_len` / `min_infix_len` 的词条的通配符扩展。
* [Issue #1617](https://github.com/manticoresoftware/manticoresearch/issues/1617) 修改了行为，以便如果 Buddy 成功处理请求，则不记录错误。
* [Issue #1635](https://github.com/manticoresoftware/manticoresearch/issues/1635) 修复了在查询中设置限制时查询元信息中的总数。
* [Issue #1640](https://github.com/manticoresoftware/manticoresearch/issues/1640) 在纯模式下通过 JSON 使用大写表名变得不可能。
* [Issue #1643](https://github.com/manticoresoftware/manticoresearch/issues/1643) 提供了`SPH_EXTNODE_STACK_SIZE` 默认值。
* [Issue #1646](https://github.com/manticoresoftware/manticoresearch/issues/1646) 修复了在 MVA 属性上使用 ALL/ANY 时的 SphinxQL 负过滤日志。
* [Issue #1660](https://github.com/manticoresoftware/manticoresearch/issues/1660) 修复了来自其他索引的 docid killlists 应用。❤️ 感谢 [@raxoft](https://github.com/raxoft) 的 PR。
* [Issue #1668](https://github.com/manticoresoftware/manticoresearch/issues/1668) 修复了在原始索引全扫描（没有任何索引迭代器）时由于提前退出导致的遗漏匹配；从纯行迭代器中移除了截断。
* [Issue #1671](https://github.com/manticoresoftware/manticoresearch/issues/1671) 修复了在查询包含代理和本地表的分布式表时的 `FACET` 错误。
* [Issue #1690](https://github.com/manticoresoftware/manticoresearch/issues/1690) 修复了对大值进行直方图估算时的崩溃。
* [Issue #1692](https://github.com/manticoresoftware/manticoresearch/issues/1692) 在使用 `alter table tbl add column col uint` 时崩溃。
* [Issue #1710](https://github.com/manticoresoftware/manticoresearch/issues/1710) 在 `WHERE json.array IN (<value>)` 条件下结果为空。
* [Issue #172](https://github.com/manticoresoftware/manticoresearch-buddy/issues/172) 修复了发送请求到 `/cli` 时 TableFormatter 的问题。
* [Issue #1742](https://github.com/manticoresoftware/manticoresearch/issues/1742) 在缺少词形文件的情况下 `CREATE TABLE` 未失败。
* [Issue #1762](https://github.com/manticoresoftware/manticoresearch/issues/1762) 现在实时表中的属性顺序遵循配置顺序。
* [Issue #1765](https://github.com/manticoresoftware/manticoresearch/issues/1765) 带有 'should' 条件的 HTTP bool 查询返回不正确的结果。
* [Issue #1769](https://github.com/manticoresoftware/manticoresearch/issues/1769) 使用 `SPH_SORT_ATTR_DESC` 和 `SPH_SORT_ATTR_ASC` 排序字符串属性不起作用。
* [Issue #177](https://github.com/manticoresoftware/manticoresearch-buddy/issues/177) 为 curl 请求到 Buddy 禁用了 `Expect: 100-continue` HTTP 标头。
* [Issue #1791](https://github.com/manticoresoftware/manticoresearch/issues/1791) 由 GROUP BY 别名引起的崩溃。
* [Issue #1792](https://github.com/manticoresoftware/manticoresearch/issues/1792) Windows 上的 SQL 元信息摘要显示错误的时间。
* [Issue #1794](https://github.com/manticoresoftware/manticoresearch/issues/1794) 修复了 JSON 查询中单词条性能下降的问题。
* [Issue #1798](https://github.com/manticoresoftware/manticoresearch/issues/1798) 不兼容的过滤器未在 `/search` 上引发错误。
* [Issue #1802](https://github.com/manticoresoftware/manticoresearch/issues/1802) 修复了 `ALTER CLUSTER ADD` 和 `JOIN CLUSTER` 操作的等待，防止了在 `ALTER` 将表添加到集群时，捐赠节点向连接节点发送表时的竞争条件。
* [Issue #1811](https://github.com/manticoresoftware/manticoresearch/issues/1811) 错误处理 `/pq/{table}/*` 请求。
* [Issue #1816](https://github.com/manticoresoftware/manticoresearch/issues/1816) `UNFREEZE` 在某些情况下无法正常工作。
* [Issue #183](https://github.com/manticoresoftware/manticoresearch-buddy/pull/183) 修复了在某些情况下 MVA 恢复的问题。
* [Issue #1849](https://github.com/manticoresoftware/manticoresearch/issues/1849) 修复了 indextool 在与 MCL 一起使用时关闭崩溃的问题。
* [Issue #1866](https://github.com/manticoresoftware/manticoresearch/issues/1866) 修复了 `/cli_json` 请求的不必要的 URL 解码。
* [Issue #1872](https://github.com/manticoresoftware/manticoresearch/issues/1872) 更改了在守护进程启动时的 plugin_dir 设置逻辑。
* [Issue #1874](https://github.com/manticoresoftware/manticoresearch/issues/1874) alter table ... exceptions 失败。
* [Issue #1891](https://github.com/manticoresoftware/manticoresearch/issues/1891#issuecomment-2051941200) 在插入数据时 Manticore 崩溃并显示 `signal 11`。
* [Issue #1920](https://github.com/manticoresoftware/manticoresearch/issues/1920) 减少了对 [low_priority](Searching/Options.md#low_priority) 的节流。
* [Issue #1924](https://github.com/manticoresoftware/manticoresearch/issues/1924) Mysqldump + mysql 恢复错误。
* [Issue #1929](https://github.com/manticoresoftware/manticoresearch/issues/1929) 在某些情况下 `charset_table` 中的重写不起作用。
* [Issue #1951](https://github.com/manticoresoftware/manticoresearch/issues/1951) 修复了在本地表缺失或代理描述不正确的情况下分布式表的创建；现在会返回错误消息。
* [Issue #1972](https://github.com/manticoresoftware/manticoresearch/issues/1972) 实现了 `FREEZE` 计数器以避免冻结/解冻问题。
* [Issue #1980](https://github.com/manticoresoftware/manticoresearch/issues/1980) 遵守 OR 节点的查询超时。之前 `max_query_time` 在某些情况下可能不起作用。
* [Issue #1986](https://github.com/manticoresoftware/manticoresearch/issues/1986) 将新版本重命名为当前版本 [manticore.json] 失败。
* [Issue #1988](https://github.com/manticoresoftware/manticoresearch/issues/1988) 全文查询可能会忽略 `SecondaryIndex` CBO 提示。
* [Issue #1990](https://github.com/manticoresoftware/manticoresearch/issues/1990) 修复了 `expansion_limit`，以对多个磁盘块或 RAM 块中的关键词调用结果集进行切片。
* [Issue #1994](https://github.com/manticoresoftware/manticoresearch/issues/1994) 外部文件错误。
* [Issue #2021](https://github.com/manticoresoftware/manticoresearch/issues/2021) 在停止 Manticore 后，可能会有一些 manticore-executor 进程仍然运行。
* [Issue #2029](https://github.com/manticoresoftware/manticoresearch/issues/2029) 使用 Levenshtein 距离时崩溃。
* [Issue #2037](https://github.com/manticoresoftware/manticoresearch/issues/2037) 在空索引上多次运行最大操作符后出错。
* [Issue #2052](https://github.com/manticoresoftware/manticoresearch/issues/2052) 在带有 JSON.field 的多组时崩溃。
* [Issue #2067](https://github.com/manticoresoftware/manticoresearch/issues/2067) Manticore 在处理错误的 _update 请求时崩溃。
* [Issue #2069](https://github.com/manticoresoftware/manticoresearch/issues/2069) 修复了 JSON 接口中字符串过滤器比较器的问题，用于闭合范围。
* [Issue #2082](https://github.com/manticoresoftware/manticoresearch/issues/2082) 当 data_dir 路径位于符号链接上时，`alter` 失败。
* [Issue #2102](https://github.com/manticoresoftware/manticoresearch/pull/2102) 改进了 mysqldump 中 SELECT 查询的特殊处理，以确保生成的 INSERT 语句与 Manticore 兼容。
* [Issue #2103](https://github.com/manticoresoftware/manticoresearch/issues/2103) 泰文字符在错误的字符集编码中。
* [Issue #2124](https://github.com/manticoresoftware/manticoresearch/issues/2124) 使用保留字的 SQL 时崩溃。
* [Issue #2154](https://github.com/manticoresoftware/manticoresearch/issues/2154) 具有词形的表无法导入。
* [Issue #2176](https://github.com/manticoresoftware/manticoresearch/issues/2176) 修复了当引擎参数设置为 'columnar' 并通过 JSON 添加重复 ID 时的崩溃。
* [Issue #223](https://github.com/manticoresoftware/manticoresearch-buddy/issues/223) 尝试在没有架构和列名的情况下插入文档时给出适当的错误。
* [Issue #239](https://github.com/manticoresoftware/manticoresearch-buddy/issues/239) 自动模式多行插入可能会失败。
* [Issue #399如果在数据源中声明了 id 属性，则在索引时添加错误消息。
* [Issue #59](https://github.com/manticoresoftware/manticoresearch-helm/issues/59) Manticore 集群崩溃。
* [Issue #68](https://github.com/manticoresoftware/manticoresearch-helm/issues/68) 如果存在渗透表，optimize.php 会崩溃。
* [Issue #77](https://github.com/manticoresoftware/manticoresearch-helm/issues/77) 修复了在 Kubernetes 上部署时的错误。
* [Issue #274](https://github.com/manticoresoftware/manticoresearch-buddy/issues/274) 修复了对 Buddy 的并发请求处理错误。

### manticore-backup 相关
* [Issue #97](https://github.com/manticoresoftware/manticoresearch-backup/pull/97) 当可用时，将 VIP HTTP 端口设置为默认。各项改进：改进了版本检查和 ZSTD 流解压缩；在恢复期间添加了版本不匹配的用户提示；修复了不同版本恢复时不正确的提示行为；增强了解压逻辑，以直接从流中读取，而不是写入工作内存；添加了 `--force` 标志。
* [Commit 3b35](https://github.com/manticoresoftware/manticoresearch-backup/commit/3b357ea) 添加了在 Manticore 搜索启动后显示备份版本的功能，以便在此阶段识别问题。
* [Commit ad2e](https://github.com/manticoresoftware/manticoresearch-backup/commit/ad2e5da) 更新了连接守护进程失败的错误消息。
* [Commit ce5e](https://github.com/manticoresoftware/manticoresearch-backup/commit/ce5ea85) 修复了将绝对根备份路径转换为相对路径的问题，并移除了恢复时的可写性检查，以便能够从不同路径恢复。
* [Commit db7e](https://github.com/manticoresoftware/manticoresearch-backup/commit/db7e2b9) 添加了文件迭代器的排序，以确保在各种情况下的一致性。
* [Issue #106](https://github.com/manticoresoftware/manticoresearch-backup/issues/105) 支持多配置的备份和恢复。
* [Issue #91](https://github.com/manticoresoftware/manticoresearch-backup/pull/91) 添加了 defattr，以防止在 RHEL 安装后文件出现异常的用户权限。
* [Issue #91](https://github.com/manticoresoftware/manticoresearch-backup/pull/91) 添加了额外的 chown，以确保在 Ubuntu 中文件默认归属 root 用户。

### MCL (columnar, secondary, knn libs) 相关
* [Commit f104](https://github.com/manticoresoftware/columnar/commit/f104a4709bbb144140d09a09bc0fc1db1895e331) 支持向量搜索。
* [Commit 2169](https://github.com/manticoresoftware/columnar/commit/216953beef6676f7e34c9f6f1a30c5980d746710) 修复了在中断的二级索引构建设置期间清理临时文件的问题。解决了守护进程在创建 `tmp.spidx` 文件时超过打开文件限制的问题。
* [Commit 709b](https://github.com/manticoresoftware/columnar/commit/709b9acaaac97d9a1ca8796892f9ad432021c785) 为 columnar 和 SI 使用独立的 streamvbyte 库。
* [Commit 1c26](https://github.com/manticoresoftware/manticoresearch/commit/1c2623763ced14bb94ad624dde9614a938408d89) 添加了关于 columnar 存储不支持 JSON 属性的警告。
* [Commit 3acd](https://github.com/manticoresoftware/columnar/commit/3acd6714ffce103447d7931dfc75bb1a7449f340) 修复了 SI 中的数据解包问题。
* [Commit 574c](https://github.com/manticoresoftware/manticoresearch/commit/574c023152564fa72092c18ebd1f594abfd4b6d7) 修复了在保存包含行存储和 columnar 存储的磁盘块时发生的崩溃。
* [Commit e87f](https://github.com/manticoresoftware/columnar/commit/e87f088b458d4c5b1b6159fc72ed0d57b786891a) 修复了 SI 迭代器在已处理的块上被提示的问题。
* [Issue #1474](https://github.com/manticoresoftware/manticoresearch/issues/1474) 使用 columnar 引擎的行存 MVA 列更新出错。
* [Issue #1510](https://github.com/manticoresoftware/manticoresearch/issues/1510) 修复了在 `HAVING` 中对 columnar 属性聚合时的崩溃。
* [Issue #1519](https://github.com/manticoresoftware/manticoresearch/issues/1519) 修复了使用 columnar 属性时在 `expr` 排序器中发生的崩溃。

### Docker 相关
* ❗[Issue #42](https://github.com/manticoresoftware/docker/issues/42) 通过环境变量支持[普通索引](https://github.com/manticoresoftware/docker#building-plain-tables)。
* ❗[Issue #47](https://github.com/manticoresoftware/docker/issues/47) 通过环境变量提高配置的灵活性。
* [Issue #54](https://github.com/manticoresoftware/docker/pull/54) 改进了 Docker 的[备份和恢复](https://github.com/manticoresoftware/docker#backup-and-restore)过程。
* [Issue #77](https://github.com/manticoresoftware/docker/issues/76) 改进了入口点，仅在首次启动时处理备份恢复。
* [Commit a27c](https://github.com/manticoresoftware/docker/commit/a27c048dd9e73d8d6bea8d9a4830cafb486b82c5) 修复了查询日志记录到 stdout 的问题。
* [Issue #38](https://github.com/manticoresoftware/docker/issues/38) 如果未设置 EXTRA，则静音 BUDDY 警告。
* [Issue #71](https://github.com/manticoresoftware/docker/pull/71) 修复了 `manticore.conf.sh` 中的主机名。

# Version 6.2.12
发布：2023年8月23日

版本 6.2.12 继续了 6.2 系列，解决了 6.2.0 发布后发现的问题。

### Bug 修复
* ❗[Issue #1351](https://github.com/manticoresoftware/manticoresearch/issues/1351) Manticore 6.2.0 在 Centos 7 上无法通过 systemctl 启动"：将 `TimeoutStartSec` 从 `infinity` 修改为 `0`，以提高对 Centos 7 的兼容性。
* ❗[Issue #1364](https://github.com/manticoresoftware/manticoresearch/issues/1364) "从 6.0.4 升级到 6.2.0 后崩溃"：添加了对旧版本 binlog 文件的空 binlog 文件的回放功能。
* [PR #1334](https://github.com/manticoresoftware/manticoresearch/pull/1334) "修复 searchdreplication.cpp 中的拼写错误"：修正了 `searchdreplication.cpp` 中的拼写错误：beggining -> beginning。
* [Issue #1337](https://github.com/manticoresoftware/manticoresearch/issues/1337) "Manticore 6.2.0 警告：conn (local)(12), sock=8088：因 MySQL 头文件失败而退出，AsyncNetInputBuffer_c::AppendData：错误 11（资源暂时不可用）返回 -1"：将 MySQL 接口关于头文件的警告的详细程度降低到 logdebugv。
* [Issue #1355](https://github.com/manticoresoftware/manticoresearch/issues/1355) "当 node_address 无法解析时，集群加入挂起"：改进了当某些节点不可达且其名称解析失败时的复制重试。这应该可以解决与 Kubernetes 和 Docker 节点相关的复制问题。增强了复制启动失败的错误消息，并更新了测试模型 376。此外，为名称解析失败提供了明确的错误消息。
* [Issue #1361](https://github.com/manticoresoftware/manticoresearch/issues/1361) "字符集 non_cjk 中没有 'Ø' 的小写映射"：调整了 'Ø' 字符的映射。
* [Issue #1365](https://github.com/manticoresoftware/manticoresearch/issues/1365) "searchd 在干净停止后留下 binlog.meta 和 binlog.001"：确保正确移除最后一个空的 binlog 文件。
* [Commit 0871](https://github.com/manticoresoftware/manticoresearch/commit/0871070845e93ad702ae0dfb89b35e5f24cd0851): 修复了 Windows 上与原子复制限制相关的 `Thd_t` 构建问题。
* [Commit 1cc0](https://github.com/manticoresoftware/manticoresearch/commit/1cc04bedc174737118fd5a3663e35f4e95bd4f8c): 解决了 FT CBO 与 `ColumnarScan` 的问题。
* [Commit c6bf](https://github.com/manticoresoftware/manticoresearch/commit/c6bfb30d53e3f4b67f17769667733677a661c95b): 对测试 376 进行了修正，并在测试中添加了 `AF_INET` 错误的替换。
* [Commit cbc3](https://github.com/manticoresoftware/manticoresearch/commit/cbc38705e0b4dcc9665e3e8cf64826290ebd11cf): 解决了在更新 blob 属性与替换文档时复制过程中出现的死锁问题。此外，在提交期间移除了索引的只读锁，因为在更基本的级别上已经锁定了。

### 次要问题
* [Commit 4f91](https://github.com/manticoresoftware/manticoresearch/commit/4f913a36e84e01f950b2797051222b0b1094d6fe) 更新了手册中 `/bulk` 端点的信息。

### MCL
* 支持 [Manticore Columnar Library](https://github.com/manticoresoftware/columnar) v2.2.4

# Version 6.2.0
发布：2023年8月4日

### 主要变更
* 查询优化器已增强以支持全文查询，显著提高了搜索效率和性能。
* 集成：
  - [mysqldump](https://dev.mysql.com/doc/refman/8.0/en/mysqldump.html) -使用 `mysqldump` 进行[逻辑备份](../Securing_and_compacting_a_table/Backup_and_restore.md#Backup-and-restore-with-mysqldump)。
  - [Apache Superset](https://manticoresearch.com/blog/manticoresearch-apache-superset-integration/) 和 [Grafana](https://manticoresearch.com/blog/manticoresearch-grafana-integration/) 用于可视化 Manticore 中存储的数据
  - [HeidiSQL](https://www.heidisql.com/) 和 [DBForge](https://www.devart.com/dbforge/) 以简化与 Manticore 的开发工作
* 我们已开始使用 [GitHub workflows](https://github.com/manticoresoftware/manticoresearch/actions)，这使贡献者更容易使用与核心团队在准备包时应用的相同的持续集成 (CI) 过程。所有任务都可以在 GitHub 托管的运行器上运行，从而实现无缝测试 Manticore Search 分支中的更改。
* 开始使用 [CLT](https://github.com/manticoresoftware/clt) 测试复杂场景。例如，我们现在能够确保在提交后构建的包可以在所有支持的 Linux 操作系统上正确安装。命令行测试器 (CLT) 提供了在交互模式下记录测试并轻松重放它们的用户友好方式。
* 在 count distinct 操作中，通过结合使用哈希表和 HyperLogLog，实现了显著的性能提升。
* 启用了包含二级索引的查询的多线程执行，线程数限制为物理 CPU 核心数。这应大大提高查询执行速度。
* `pseudo_sharding` 已调整为限制在空闲线程数内。此更新显著提升了吞吐性能。
* 用户现在可以通过配置设置指定[默认属性存储引擎](../Server_settings/Searchd.md#engine)，从而提供更好的定制性以匹配特定的工作负载需求。
* 支持 [Manticore Columnar Library 2.2.0](https://github.com/manticoresoftware/columnar/)，并在[二级索引](Server_settings/Searchd.md#secondary_indexes)中进行了大量 Bug 修复和改进。

### 次要问题
* [Buddy #153](https://github.com/manticoresoftware/manticoresearch-buddy/issues/153):  [/pq](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md?client=JSON#Adding-rules-to-a-percolate-table) HTTP 端点现在作为 `/json/pq` HTTP 端点的别名。
* [Commit 0bf1](https://github.com/manticoresoftware/manticoresearch/commit/0bf17d9e): 确保 `upper()` 和 `lower()` 的多字节兼容性。
* [Commit 2bb9](https://github.com/manticoresoftware/manticoresearch/commit/2bb92765): 现在对于 `count(*)` 查询，返回预计算的值，而不是扫描索引。
* [Commit 3c84](https://github.com/manticoresoftware/manticoresearch/commit/3c84091f): 现在可以使用 `SELECT` 进行任意计算并显示 `@@sysvars`。不同于之前，不再仅限于一个计算。因此，类似 `select user(), database(), @@version_comment, version(), 1+1 as a limit 10` 的查询将返回所有列。请注意，可选的 'limit' 将始终被忽略。
* [Commit 6aca](https://github.com/manticoresoftware/manticoresearch/commit/6aca32f0): 实现了 `CREATE DATABASE` 的占位查询。
* [Commit 9dc1](https://github.com/manticoresoftware/manticoresearch/commit/9dc12334): 执行 `ALTER TABLE table REBUILD SECONDARY` 时，即使属性未更新，二级索引也将始终重新构建。
* [Commit 46ed](https://github.com/manticoresoftware/manticoresearch/commit/46edb089): 在使用 CBO 前，现在会识别使用预计算数据的排序器，以避免不必要的 CBO 计算。
* [Commit 102a](https://github.com/manticoresoftware/manticoresearch/commit/102ac604): 实现了全文表达式堆栈的模拟与使用，以防止守护进程崩溃。
* [Commit 979f](https://github.com/manticoresoftware/manticoresearch/commit/979fa27c): 为不使用字符串/mvas/json 属性的匹配添加了快速的代码路径，用于匹配克隆代码。
* [Commit a073](https://github.com/manticoresoftware/manticoresearch/commit/a0735ffe): 添加了对 `SELECT DATABASE()` 命令的支持，但始终返回 `Manticore`。此功能对于与各种 MySQL 工具的集成至关重要。
* [Commit bc04](https://github.com/manticoresoftware/manticoresearch/commit/bc04908): 修改了 [/cli](../Connecting_to_the_server/HTTP.md#/cli) 端点的响应格式，并添加了 `/cli_json` 端点以用作之前的 `/cli`。
* [Commit d70b](https://github.com/manticoresoftware/manticoresearch/commit/d70b0d58): 现在可以在运行时使用 `SET` 语句更改 `thread_stack`。提供会话本地和守护进程范围的变体。当前值可在 `show variables` 输出中访问。
* [Commit d96e](https://github.com/manticoresoftware/manticoresearch/commit/d96ec6b9): 将代码集成到 CBO 中，以更准确地估计在字符串属性上执行过滤的复杂性。
* [Commit e77d](https://github.com/manticoresoftware/manticoresearch/commit/e77dd72f5a04531c352fad0d7afcd2a1cbae2510): 改进了 DocidIndex 成本计算，提高了整体性能。
* [Commit f3ae](https://github.com/manticoresoftware/manticoresearch/commit/f3ae8bea): `SHOW STATUS` 命令中现在可以看到类似于 Linux 上的 'uptime' 的负载指标。
* [Commit f3cc](https://github.com/manticoresoftware/manticoresearch/commit/f3cc0971): `DESC` 和 `SHOW CREATE TABLE` 的字段和属性顺序现在与 `SELECT * FROM` 的顺序一致。
* [Commit f3d2](https://github.com/manticoresoftware/manticoresearch/commit/f3d248a6): 不同的内部解析器在各种错误期间现在提供其内部助记码（例如 `P01`）。此增强功能有助于识别导致错误的解析器，并隐藏非必要的内部细节。
* [Issue #271](https://github.com/manticoresoftware/manticoresearch/issues/271) “有时 CALL SUGGEST 不建议纠正单字母拼写错误”：改进了短词的 [SUGGEST/QSUGGEST](../Searching/Spell_correction.md#CALL-QSUGGEST,-CALL-SUGGEST) 行为：添加了 `sentence` 选项以显示完整句子。
* [Issue #696](https://github.com/manticoresoftware/manticoresearch/issues/696) “在启用词干化时，通过精确短语查询进行渗透索引搜索时无法正常工作”：修改了渗透查询以处理精确术语修饰符，改进了搜索功能。
* [Issue #829](https://github.com/manticoresoftware/manticoresearch/issues/829) “日期格式化方法”：添加了 [date_format()](../Functions/Date_and_time_functions.md#DATE_FORMAT()) 选择列表表达式，公开了 `strftime()` 函数。
* [Issue #961](https://github.com/manticoresoftware/manticoresearch/issues/961) “通过 HTTP JSON API 对桶进行排序”：在 HTTP 接口的每个聚合桶中引入了可选的 [sort 属性](../Searching/Faceted_search.md#HTTP-JSON)。
* [Issue #1062](https://github.com/manticoresoftware/manticoresearch/issues/1062) “改进 JSON 插入 API 失败的错误日志记录 - '不支持的值类型'”：在错误情况下，`/bulk` 端点报告已处理和未处理的字符串（文档）数量的信息。
* [Issue #1070](https://github.com/manticoresoftware/manticoresearch/issues/1070) “CBO 提示不支持多个属性”：启用索引提示以处理多个属性。
* [Issue #1106](https://github.com/manticoresoftware/manticoresearch/issues/1106) “在 HTTP 搜索查询中添加标签”：在 [HTTP PQ 响应](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md?client=JSON#Adding-rules-to-a-percolate-table) 中添加了标签。
* [Issue #1301](https://github.com/manticoresoftware/manticoresearch/issues/1301) “buddy 不应并行创建表”：解决了由于并行 CREATE TABLE 操作导致的失败问题。现在，只能同时运行一个 `CREATE TABLE` 操作。
* [Issue #1303](https://github.com/manticoresoftware/manticoresearch/issues/1303) “支持在列名中使用 @”。
* [Issue #1316](https://github.com/manticoresoftware/manticoresearch/issues/1316) “在出租车数据集上的查询速度缓慢，ps=1”：优化了 CBO 逻辑，默认直方图分辨率设置为 8k，以提高随机分布值的属性精度。
* [Issue #1317](https://github.com/manticoresoftware/manticoresearch/issues/1317) “修复 CBO 与 hn 数据集上的全文”：增强了逻辑，用于决定何时使用位图迭代器交集，何时使用优先级队列。
* [Issue #1318](https://github.com/manticoresoftware/manticoresearch/issues/1318) “columnar：将迭代器接口更改为单调用”：列式迭代器现在使用单次 `Get` 调用，取代了以前的 `AdvanceTo` + `Get` 双步调用来检索值。
* [Issue #1319](https://github.com/manticoresoftware/manticoresearch/issues/1319) “聚合计算加速（删除 CheckReplaceEntry？）”：从组排序器中删除了 `CheckReplaceEntry` 调用，以加快聚合函数的计算。
* [Issue #1320](https://github.com/manticoresoftware/manticoresearch/issues/1320) “创建表 read_buffer_docs/hits 不理解 k/m/g 语法”：`CREATE TABLE` 选项 `read_buffer_docs` 和 `read_buffer_hits` 现在支持 k/m/g 语法。
* [语言包](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) （英语、德语和俄语）现在可以通过在 Linux 上执行命令 `apt/yum install manticore-language-packs` 轻松安装。在 macOS 上使用命令 `brew install manticoresoftware/tap/manticore-language-packs`。
* `SHOW CREATE TABLE` 和 `DESC` 操作之间的字段和属性顺序现在一致。
* 执行 `INSERT` 查询时，如果磁盘空间不足，新 `INSERT` 查询将失败，直到有足够的磁盘空间可用。
* 添加了 [UINT64()](Functions/Type_casting_functions.md#UINT64()) 类型转换函数。
* `/bulk` 端点现在将空行视为 [commit](../Data_creation_and_modification/Transactions.md#BEGIN,-COMMIT,-and-ROLLBACK) 命令。更多信息 [见此处](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md?client=JSON#Bulk-adding-documents)。
* 针对 [无效索引提示](../Searching/Options.md#Query-optimizer-hints) 实现了警告，提高了透明度并允许错误缓解。
* 当 `count(*)` 与单个过滤器一起使用时，查询现在会利用来自二级索引的预计算数据（如果可用），显著加快查询时间。

### ⚠️ 重大变更
* ⚠️ 在版本 6.2.0 中创建或修改的表无法被旧版本读取。
* ⚠️ 文档 ID 在索引和 INSERT 操作期间现在被处理为无符号 64 位整数。
* ⚠️ 查询优化器提示的语法已更新。新格式为 `/*+ SecondaryIndex(uid) */`。请注意，不再支持旧语法。
* ⚠️ [Issue #1160](https://github.com/manticoresoftware/manticoresearch/issues/1160): 为了避免语法冲突，表名中不再允许使用 `@`。
* ⚠️ 在 `INSERT`、`DESC` 和 `ALTER` 操作期间，标记为 `indexed` 和 `attribute` 的字符串字段/属性现在被视为单一字段。
* ⚠️ [Issue #1057](https://github.com/manticoresoftware/manticoresearch/issues/1057): MCL 库将不再在不支持 SSE 4.2 的系统上加载。
* ⚠️ [Issue #1143](https://github.com/manticoresoftware/manticoresearch/issues/1143): [agent_query_timeout](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_query_timeout) 被修复，现已生效。

### Bug 修复
* [Commit 2a6e](https://github.com/manticoresoftware/manticoresearch/commit/2a6ea8f7) “DROP TABLE 时崩溃”：解决了在执行 DROP TABLE 语句时导致 RT 表上写操作（优化、磁盘块保存）延迟完成的问题。添加了一个警告，当执行 DROP TABLE 命令后表目录未清空时进行通知。
* [Commit 2ebd](https://github.com/manticoresoftware/manticoresearch/commit/2ebd424d): 增加了对用于按多个属性分组时的 columnar 属性支持。
* [Commit 3be4](https://github.com/manticoresoftware/manticoresearch/commit/3be4503f752216b37b8cde8e47fd2d77bbce63ae) 解决了可能由于磁盘空间不足引起的崩溃问题，正确处理 binlog 写入错误。
* [Commit 6adb](https://github.com/manticoresoftware/manticoresearch/commit/6adb0934): 修复了在查询中使用多个 columnar 扫描迭代器（或二级索引迭代器）时偶尔发生的崩溃。
* [Commit 6bd9](https://github.com/manticoresoftware/manticoresearch/commit/6bd9f709): 修复了使用包含预计算数据的排序器时未删除过滤器的问题。
* [Commit 6d03](https://github.com/manticoresoftware/manticoresearch/commit/6d03566a): 更新了 CBO 代码，以更好地估算使用多线程执行的按行属性过滤的查询。
* [Commit 6dd3](https://github.com/manticoresoftware/manticoresearch/commit/6dd3964), [Helm #56](https://github.com/manticoresoftware/manticoresearch-helm/issues/56) “Kubernetes 集群中的致命崩溃”：修复了 JSON 根对象的有缺陷布隆过滤器；修复了因按 JSON 字段过滤导致的守护进程崩溃。
* [Commit 6e1b](https://github.com/manticoresoftware/manticoresearch/commit/6e1b849) 修复了无效的 `manticore.json` 配置引起的守护进程崩溃。
* [Commit 6fbc](https://github.com/manticoresoftware/manticoresearch/commit/6fbc4189) 修复了 JSON 范围过滤器以支持 int64 值。
* [Commit 9c67](https://github.com/manticoresoftware/manticoresearch/commit/9c677426) 修复了 `.sph` 文件在 `ALTER` 操作时可能被损坏的问题。
* [Commit 77cc](https://github.com/manticoresoftware/manticoresearch/commit/77ccd8f): 添加了一个共享键，用于解决从多个主节点复制 replace 语句时发生的 `pre_commit` 错误。
* [Commit 2884](https://github.com/manticoresoftware/manticoresearch/commit/2884519e) 解决了针对诸如 `date_format()` 之类函数的 bigint 检查问题。
* [Commit 9513](https://github.com/manticoresoftware/manticoresearch/commit/9513aca5): 在排序器使用预计算数据时，[SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META) 不再显示迭代器。
* [Commit a2a7](https://github.com/manticoresoftware/manticoresearch/commit/a2a70555): 更新了全文节点堆栈大小以防止复杂全文查询导致的崩溃。
* [Commit a062](https://github.com/manticoresoftware/manticoresearch/commit/a0626d7e): 修复了在带有 JSON 和字符串属性的更新复制期间发生的崩溃问题。
* [Commit b3e6](https://github.com/manticoresoftware/manticoresearch/commit/b3e63e5d): 更新了字符串构建器以使用 64 位整数，以避免处理大型数据集时的崩溃。
* [Commit c472](https://github.com/manticoresoftware/manticoresearch/commit/c472e5b): 解决了跨多个索引的 count distinct 时发生的崩溃。
* [Commit d073](https://github.com/manticoresoftware/manticoresearch/commit/d0730272): 修复了 RT 索引的磁盘块上的查询在禁用 `pseudo_sharding` 时仍然可以在多个线程中执行的问题。
* [Commit d205](https://github.com/manticoresoftware/manticoresearch/commit/d205508e) 修改了 `show index status` 命令返回的值集，现在根据使用的索引类型而有所不同。
* [Commit e9bc](https://github.com/manticoresoftware/manticoresearch/commit/e9bc2f9) 修复了处理批量请求时的 HTTP 错误，以及未将错误从网络循环返回到客户端的问题。
* [Commit f77c](https://github.com/manticoresoftware/manticoresearch/commit/f77ce0e6) 在 PQ 中使用了扩展堆栈。
* [Commit fac2](https://github.com/manticoresoftware/manticoresearch/commit/fac2d9687a20582765aaa89be9597804c4f845af) 更新了导出排序器输出以与 [packedfactors()](../Functions/Searching_and_ranking_functions.md#PACKEDFACTORS()) 对齐。
* [Commit ff87](https://github.com/manticoresoftware/manticoresearch/commit/ff87cc4): 修复了 SphinxQL 查询日志中过滤器中的字符串列表问题。
* [Issue #589](https://github.com/manticoresoftware/manticoresearch/issues/589) “字符集定义似乎依赖于代码的顺序”：修复了重复字符集映射错误。
* [Issue #811](https://github.com/manticoresoftware/manticoresearch/issues/811) “词形转换中多个单词的映射在含有 CJK 标点符号的短语搜索中产生干扰”：修正了词形转换中短语查询的 ngram 令牌位置。
* [Issue #834](https://github.com/manticoresoftware/manticoresearch/issues/834) “查询中的等号破坏了请求”：确保可以转义确切符号，并通过 `expand_keywords` 选项修复了双重精确扩展。
* [Issue #864](https://github.com/manticoresoftware/manticoresearch/issues/864) "exceptions/stop词冲突"
* [Issue #910](https://github.com/manticoresoftware/manticoresearch/issues/910) 调用 SNIPPETS() 时 Manticore 崩溃，带有 libstemmer_fr 和 index_exact_words”：解决了 `SNIPPETS()` 调用时导致的内部冲突崩溃。
* [Issue #946](https://github.com/manticoresoftware/manticoresearch/issues/946) “SELECT 查询中的重复记录”：修复了在带有 `not_terms_only_allowed` 选项的 RT 索引查询中结果集中出现的重复文档问题。
* [Issue #967](https://github.com/manticoresoftware/manticoresearch/issues/967) “在 UDF 函数中使用 JSON 参数导致崩溃”：修复了启用伪分片和带 JSON 参数的 UDF 的搜索时的守护进程崩溃。
* [Issue #1050](https://github.com/manticoresoftware/manticoresearch/issues/1050) “count(*) 在 FEDERATED 中”：修复了通过带有聚合的 `FEDERATED` 引擎查询时发生的守护进程崩溃。
* [Issue #1052](https://github.com/manticoresoftware/manticoresearch/issues/1052) 修复了 `rt_attr_json` 列与 columnar 存储不兼容的问题。
* [Issue #1072](https://github.com/manticoresoftware/manticoresearch/issues/1072) “通配符在查询中被 ignore_chars 删除”：修复了通配符在查询中不受 `ignore_chars` 影响的问题。
* [Issue #1075](https://github.com/manticoresoftware/manticoresearch/issues/1075) “indextool --check 在有分布式表时失败”：indextool 现已兼容 JSON 配置中具有 'distributed' 和 'template' 索引的实例。
* [Issue #1081](https://github.com/manticoresoftware/manticoresearch/issues/1081) “特定 RT 数据集上的特定选择导致 searchd 崩溃”：解决了带有 packedfactors 和大内存缓冲区的查询导致守护进程崩溃的问题。
* [Issue #1095](https://github.com/manticoresoftware/manticoresearch/issues/1095) “带有 not_terms_only_allowed 时忽略已删除文档”
* [Issue #1099](https://github.com/manticoresoftware/manticoresearch/issues/1099) “indextool --dumpdocids 无法工作”：恢复了 `--dumpdocids` 命令的功能。
* [Issue #1100](https://github.com/manticoresoftware/manticoresearch/issues/1100) “indextool --buildidf 无法工作”：indextool 现在在完成 globalidf 后关闭文件。
* [Issue #1104](https://github.com/manticoresoftware/manticoresearch/issues/1104) “在远程表中将 count(*) 作为架构集处理”：解决了在分布式索引查询中当代理返回空结果集时守护进程发送错误消息的问题。
* [Issue #1109](https://github.com/manticoresoftware/manticoresearch/issues/1109) “FLUSH ATTRIBUTES 在 threads=1 时挂起”。
* [Issue #1126](https://github.com/manticoresoftware/manticoresearch/issues/1126) “查询期间与 MySQL 服务器的连接丢失 - manticore 6.0.5”：修复了使用多个列属性过滤器时发生的崩溃。
* [Issue #1135](https://github.com/manticoresoftware/manticoresearch/issues/1135) “JSON 字符串过滤区分大小写”：修正了 HTTP 搜索请求中过滤的正确排序。
* [Issue #1140](https://github.com/manticoresoftware/manticoresearch/issues/1140) “错误字段匹配”：修复了 `morphology_skip_fields` 相关的问题。
* [Issue #1155](https://github.com/manticoresoftware/manticoresearch/issues/1155) “通过 API 的系统远程命令应传递 g_iMaxPacketSize”：更新以绕过节点之间复制命令的 `max_packet_size` 检查。还在状态显示中添加了最新的集群错误。
* [Issue #1302](https://github.com/manticoresoftware/manticoresearch/issues/1302) “优化失败时残留临时文件”：修复了在合并或优化过程中发生错误后遗留临时文件的问题。
* [Issue #1304](https://github.com/manticoresoftware/manticoresearch/issues/1304) “为 buddy 启动超时添加环境变量”：添加环境变量 `MANTICORE_BUDDY_TIMEOUT`（默认 3 秒），用于控制守护进程在启动时等待 buddy 消息的时长。
* [Issue #1305](https://github.com/manticoresoftware/manticoresearch/issues/1305) “保存 PQ 元数据时整型溢出”：减少守护进程在保存大型 PQ 索引时的内存消耗。
* [Issue #1306](https://github.com/manticoresoftware/manticoresearch/issues/1306) “更改其外部文件后无法重新创建 RT 表”：修复了外部文件为空字符串的更改错误；修复了更改外部文件后遗留的 RT 索引外部文件。
* [Issue #1307](https://github.com/manticoresoftware/manticoresearch/issues/1307) “SELECT 语句 sum(value) as value 无法正确工作”：修复了带有别名的选择列表表达式可能隐藏索引属性的问题；修复了 int64 整数计算的 sum。
* [Issue #1308](https://github.com/manticoresoftware/manticoresearch/issues/1308) “在复制时避免绑定到 localhost”：确保在具有多个 IP 的主机名上复制不绑定到 localhost。
* [Issue #1309](https://github.com/manticoresoftware/manticoresearch/issues/1309) “数据大于 16Mb 时返回给 mysql 客户端失败”：修复了返回给客户端超过 16Mb 的 SphinxQL 包的问题。
* [Issue #1310](https://github.com/manticoresoftware/manticoresearch/issues/1310) “外部文件的路径应为绝对路径中的错误引用”：在 `SHOW CREATE TABLE` 中显示完整的外部文件路径。
* [Issue #1311](https://github.com/manticoresoftware/manticoresearch/issues/1311) “调试构建在片段中的长字符串时崩溃”：现在，`SNIPPET()` 函数中允许包含长字符串（>255 个字符）。
* [Issue #1312](https://github.com/manticoresoftware/manticoresearch/issues/1312) “在 kqueue 轮询 (master-agent) 中使用删除后崩溃”：修复了在 kqueue 驱动系统（FreeBSD、MacOS 等）上主节点无法连接到代理时发生的崩溃。
* [Issue #1313](https://github.com/manticoresoftware/manticoresearch/issues/1313) “连接超时过长”：在 MacOS/BSD 上从主节点连接到代理时，使用了统一的连接+查询超时，而不仅是连接。
* [Issue #1314](https://github.com/manticoresoftware/manticoresearch/issues/1314) “pq (json meta) 含未达到的嵌入同义词加载失败”：修复了 pq 中嵌入的同义词标志。
* [Issue #1315](https://github.com/manticoresoftware/manticoresearch/issues/1315) “允许某些函数（sint、fibonacci、second、minute、hour、day、month、year、yearmonth、yearmonthday）使用隐式提升的参数值”。
* [Issue #1321](https://github.com/manticoresoftware/manticoresearch/issues/1321) “在 fullscan 中启用多线程 SI，但限制线程”：在 CBO 中实现了代码，以更好地预测二级索引在全文查询中使用的多线程性能。
* [Issue #1322](https://github.com/manticoresoftware/manticoresearch/issues/1322) “使用预计算排序器后，count(*) 查询仍然缓慢”：在使用预计算数据的排序器时不再启动迭代器，从而避免了不良的性能影响。
* [Issue #1411](https://github.com/manticoresoftware/manticoresearch/issues/1141) “sphinxql 中的查询日志未保留 MVA 的原始查询”：现在记录了 `all()/any()`。

# Version 6.0.4
发布日期：2023年3月15日

### 新功能
* 改进了与 Logstash、Beats 等的集成，包括：
  - 支持 Logstash 版本 7.6 - 7.15，Filebeat 版本 7.7 - 7.12
  - 自动模式支持。
  - 添加了对 Elasticsearch 格式的批量请求的处理。
* [Buddy commit ce90](https://github.com/manticoresoftware/manticoresearch-buddy/commit/ce907ea) 在 Manticore 启动时记录 Buddy 版本。

### Bug 修复
* [Issue #588](https://github.com/manticoresoftware/manticoresearch/issues/588), [Issue #942](https://github.com/manticoresoftware/manticoresearch/issues/942) 修复了 bigram 索引的搜索元数据和调用关键字中的错误字符。
* [Issue #1027](https://github.com/manticoresoftware/manticoresearch/issues/1027) 拒绝小写 HTTP 头。
* ❗[Issue #1039](https://github.com/manticoresoftware/manticoresearch/issues/1039) 修复了守护进程在读取 Buddy 控制台输出时的内存泄漏。
* [Issue #1056](https://github.com/manticoresoftware/manticoresearch/issues/1056) 修复了问号的意外行为。
* [Issue #1064](https://github.com/manticoresoftware/manticoresearch/issues/1064) - 修复了分词器小写表中的竞态条件，导致崩溃。
* [Commit 59bb](https://github.com/manticoresoftware/manticoresearch/commit/59bb54c) 修复了 JSON 接口中 ID 显式设置为 null 的文档的批量写入处理。
* [Commit 7b6b](https://github.com/manticoresoftware/manticoresearch/commit/7b6b25f) 修复了 CALL KEYWORDS 中多个相同术语的术语统计
* [Commit f381](https://github.com/manticoresoftware/manticoresearch/commit/f381ad2) 默认配置现在由 Windows 安装程序创建；路径不再在运行时替换。
* [Commit 6940](https://github.com/manticoresoftware/manticoresearch/commit/6940e95), [Commit cc5a](https://github.com/manticoresoftware/manticoresearch/commit/cc5a480) 修复了多网络节点集群的复制问题。
* [Commit 4972](https://github.com/manticoresoftware/manticoresearch/commit/49722ab) 修复了 `/pq` HTTP 端点为 `/json/pq` HTTP 端点的别名。
* [Commit 3b53](https://github.com/manticoresoftware/manticoresearch/commit/3b5385a) 修复了 Buddy 重启时守护进程崩溃的问题。
* [Buddy commit fba9](https://github.com/manticoresoftware/manticoresearch-buddy/commit/fba9c8c) 在收到无效请求时显示原始错误。
* [Buddy commit db95](https://github.com/manticoresoftware/manticoresearch-buddy/commit/db9532c) 允许备份路径中包含空格，并在正则表达式中添加了一些魔法以支持单引号。

# Version 6.0.2
发布日期： 2023年2月10日

### Bug 修复
* [Issue #1024 crash 2](https://github.com/manticoresoftware/manticoresearch/issues/1024) Facet搜索在结果数量较大时导致崩溃/段错误
* ❗[Issue #1029](https://github.com/manticoresoftware/manticoresearch/issues/1029) - 警告：编译值KNOWN_CREATE_SIZE（16）小于实际测量值（208）。请考虑修正该值！
* ❗[Issue #1032](https://github.com/manticoresoftware/manticoresearch/issues/1032) - Manticore 6.0.0 普通索引崩溃
* ❗[Issue #1033](https://github.com/manticoresoftware/manticoresearch/issues/1033) - 守护进程重启时丢失多个分布式表
* ❗[Issue #1064](https://github.com/manticoresoftware/manticoresearch/issues/1064) - tokenizer 小写表中的竞争条件问题

# Version 6.0.0
发布日期：2023年2月7日

从此版本开始，Manticore Search 配备了 Manticore Buddy，一个用 PHP 编写的伴随守护进程，用于处理不需要超低延迟或高吞吐量的高级功能。Manticore Buddy 在后台运行，用户可能甚至意识不到它的存在。虽然对终端用户不可见，但使 Manticore Buddy 易于安装并与主 C++ 守护进程兼容是一个重要挑战。此重大变更将使团队能够开发各种新的高级功能，例如分片编排、访问控制和认证，以及多种集成（如 mysqldump、DBeaver、Grafana mysql 连接器）。目前，它已支持 [SHOW QUERIES](../Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES)、[BACKUP](../Securing_and_compacting_a_table/Backup_and_restore.md#BACKUP-SQL-command-reference) 和 [Auto schema](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema)。

此版本还包含了超过 130 个 Bug 修复和众多功能，其中许多可以视为重大功能。

### 主要变更
* 🔬 实验性功能：现在可以执行与 Elasticsearch 兼容的 [insert](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md?example=Elasticsearch#Adding-documents-to-a-real-time-table) 和 [replace](Data_creation_and_modification/Updating_documents/REPLACE.md?example=Elasticsearch#REPLACE) JSON 查询，使 Manticore 能够与 Logstash（版本 < 7.13）、Filebeat 及其他 Beats 系列工具配合使用。默认启用。可以通过 `SET GLOBAL ES_COMPAT=off` 禁用。
* 支持 [Manticore Columnar Library 2.0.0](https://github.com/manticoresoftware/columnar/)，包含大量修复和 [Secondary indexes](../Server_settings/Searchd.md#secondary_indexes) 的改进。 **⚠️ 重大变更**: 本版本起，Secondary indexes 默认开启。如果从 Manticore 5 升级，请确保执行 [ALTER TABLE table_name REBUILD SECONDARY](Updating_table_schema_and_settings.md#Rebuilding-a-secondary-index)。详见下文。
* [Commit c436](https://github.com/manticoresoftware/manticoresearch/commit/c436f9023536f767610451911955ae36d90aa638) 自动模式：现在可以跳过表的创建，直接插入第一个文档，Manticore 将根据字段自动创建表。详情请见[此处](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema)。可通过 [searchd.auto_schema](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema) 开启或关闭。
* 大幅改进了 [基于成本的优化器（CBO）](../Searching/Cost_based_optimizer.md)，在多种情况下减少查询响应时间。
  - [Issue #1008](https://github.com/manticoresoftware/manticoresearch/issues/1008) CBO 的并行性能估计。
  - [Issue #1014](https://github.com/manticoresoftware/manticoresearch/issues/1014) CBO 现在识别 [secondary indexes](../Server_settings/Searchd.md#secondary_indexes)，更智能地运行。
  - [Commit cef9](https://github.com/manticoresoftware/columnar/commit/cef9b54dc3a4ee9138ebc4df3a671715b996d6b3) 将列存储表/字段的编码统计信息存储在元数据中，以帮助 CBO 做出更明智的决策。
  - [Commit 2b95](https://github.com/manticoresoftware/manticoresearch/commit/2b95fb8c2ba422c9c5a66e2b61c1f7922f31c7aa) 为 CBO 增加了微调提示。
* [遥测](Telemetry.md#Telemetry)：本次发布新增遥测功能，允许收集匿名和去标识化的指标，以帮助我们提升产品性能和用户体验。请放心，所有数据均 **完全匿名，且不会与任何个人信息关联**。可在设置中[轻松关闭](../Telemetry.md#Telemetry)此功能。
* [Commit 5aaf](https://github.com/manticoresoftware/manticoresearch/commit/5aafb17d81b601e399e212a344a4738f65576cdd) [ALTER TABLE table_name REBUILD SECONDARY](../Updating_table_schema_and_settings.md#Rebuilding-a-secondary-index) 可随时重建 secondary indexes，例如：
  - 从 Manticore 5 升级到新版本时，
  - 在索引中使用 [UPDATE](../Data_creation_and_modification/Updating_documents/UPDATE.md#UPDATE)（即[原地更新，不是替换](Data_creation_and_modification/Updating_documents/REPLACE_vs_UPDATE.md)）属性时
* [Issue #821](https://github.com/manticoresoftware/manticoresearch/issues/821) 新增工具 `manticore-backup`，用于 [备份和恢复 Manticore 实例](Securing_and_compacting_a_table/Backup_and_restore.md)../Securing_and_compacting_a_table/Backup_and_restore.md)
* SQL 命令 [BACKUP](../Securing_and_compacting_a_table/Backup_and_restore.md#BACKUP-SQL-command-reference) 可从 Manticore 内部进行备份。
* SQL 命令 [SHOW QUERIES](../Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES) 以便于查看正在运行的查询，而非线程。
* [Issue #551](https://github.com/manticoresoftware/manticoresearch/issues/551) SQL 命令 `KILL` 可终止长时间运行的 `SELECT`。
* 聚合查询的动态 `max_matches` 提升了准确性并降低了响应时间。

### 次要问题
* [Issue #822](https://github.com/manticoresoftware/manticoresearch/issues/822) SQL 命令 [FREEZE/UNFREEZE](../Securing_and_compacting_a_table/Freezing_a_table.md) 准备实时/普通表进行备份。
* [Commit c470](https://github.com/manticoresoftware/manticoresearch/commit/c47052f18ad4868134de1a97e8d7da570e531dfa) 新增设置 `accurate_aggregation` 和 `max_matches_increase_threshold` 以控制聚合精度。
* [Issue #718](https://github.com/manticoresoftware/manticoresearch/issues/718) 支持带符号的负64位 ID。请注意，ID 仍不能超过 2^63，但现在可以使用从 -2^63 到 0 的范围内的 ID。
* 为减少混淆，将 `index` 重命名为 `table`，以区别次级索引、全文索引和普通/实时索引（即表）。以下 SQL/命令行命令受到影响，旧版本已弃用但仍然可用：
  - `index <table name>` => `table <table name>`,
  - `searchd -i / --index` => `searchd -t / --table`,
  - `SHOW INDEX STATUS` => `SHOW TABLE STATUS`,
  - `SHOW INDEX SETTINGS` => `SHOW TABLE SETTINGS`,
  - `FLUSH RTINDEX` => `FLUSH TABLE`,
  - `OPTIMIZE INDEX` => `OPTIMIZE TABLE`,
  - `ATTACH TABLE plain TO RTINDEX rt` => `ATTACH TABLE plain TO TABLE rt`,
  - `RELOAD INDEX` => `RELOAD TABLE`,
  - `RELOAD INDEXES` => `RELOAD TABLES`.

  我们并不打算废弃旧形式，但为确保与文档的兼容性，建议您在应用中更改名称。未来的版本中会变更的内容是将各种 SQL 和 JSON 命令输出中的“index”重命名为“table”。
* 使用有状态的 UDF 的查询现被强制在单线程中执行。
* [Issue #1011](https://github.com/manticoresoftware/manticoresearch/issues/1011) 时间调度相关的重构，作为并行块合并的前提条件。
* **⚠️ 重大变更**: 列存储格式已更改。需要重建具有列存储属性的表。
* **⚠️ 重大变更**: 次级索引文件格式已更改。如果在配置文件中使用 `searchd.secondary_indexes = 1`，请注意新版本的 Manticore **将跳过加载具有次级索引的表**。建议：
  
  - 在升级之前，将配置文件中的 `searchd.secondary_indexes` 设置为 0。
  - 启动实例，Manticore 将加载表并发出警告。
  - 运行 `ALTER TABLE <table name> REBUILD SECONDARY` 来为每个索引重建二级索引。
  
  如果您正在运行一个复制集群，则需要在所有节点上运行 `ALTER TABLE <table name> REBUILD SECONDARY`，或者遵循[此说明](Securing_and_compacting_a_table/Compacting_a_table.md#Optimizing-clustered-tables)并仅更改操作：使用 `ALTER .. REBUILD SECONDARY` 替代 `OPTIMIZE`。
* **⚠️ 重大变更**: binlog 版本已更新，因此不会重播以前版本的 binlog。在升级过程中，确保 Manticore Search 以干净的方式停止。这意味着在停止先前的实例后，`/var/lib/manticore/binlog/` 中除了 `binlog.meta` 外不应存在其他 binlog 文件。
* [Issue #849](https://github.com/manticoresoftware/manticoresearch/issues/849) `SHOW SETTINGS`：现在可以在 Manticore 内部查看配置文件中的设置。
* [Issue #1007](https://github.com/manticoresoftware/manticoresearch/issues/1007) [SET GLOBAL CPUSTATS=1/0](Server_settings/Setting_variables_online.md#SET) 开启/关闭 CPU 时间追踪；[SHOW THREADS](../Node_info_and_management/SHOW_THREADS.md) 在 CPU 时间追踪关闭时不显示 CPU 统计信息。
* [Issue #1009](https://github.com/manticoresoftware/manticoresearch/issues/1009) RT 表的 RAM 块段现在可以在 RAM 块刷新的同时合并。
* [Issue #1012](https://github.com/manticoresoftware/manticoresearch/issues/1012) 在 [indexer](../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) 输出中增加了次级索引进度。
* [Issue #1013](https://github.com/manticoresoftware/manticoresearch/issues/1013) 之前，如果 Manticore 在启动时无法提供服务，表记录可能会从索引列表中删除。新的行为是将其保留在列表中，以便在下次启动时尝试加载。
* [indextool --docextract](../Miscellaneous_tools.md#indextool) 返回请求文档中的所有词和命中。
* [Commit 2b29](https://github.com/manticoresoftware/manticoresearch/commit/2b296ee20e520b85bcbb4383e87095d31e7165dc) 环境变量 `dump_corrupt_meta` 使 searchd 无法加载索引时可以将损坏的表元数据转储到日志中。
* [Commit c7a3](https://github.com/manticoresoftware/manticoresearch/commit/c7a3136eefe0eb2f69e3d99fae9c10beb5821ea6) ``DEBUG META` 可以显示 `max_matches` 和伪分片统计信息。
* [Commit 6bca](https://github.com/manticoresoftware/manticoresearch/commit/6bca4848dc5c8396dbc1e057be4b9b2d579794e8) 改进错误信息，代替误导性的“Index header format is not json, will try it as binary...”。
* [Commit bef3](https://github.com/manticoresoftware/lemmatizer-uk/commit/bef3ff0386d3ee87ec57619782100972c1122e47) 乌克兰语词形变化路径已更改。
* [Commit 4ae7](https://github.com/manticoresoftware/manticoresearch/commit/4ae789595329a2951e194d1191ddb3121459a560) [SHOW META](../Node_info_and_management/SHOW_META.md#SHOW-META) 中增加了次级索引统计信息。
* [Commit 2e7c](https://github.com/manticoresoftware/manticoresearch/commit/2e7c585e) JSON 接口现在可以使用 Swagger Editor 轻松可视化： https://manual.manticoresearch.com/Openapi#OpenAPI-specification.
* **⚠️ 重大变更**: 复制协议已更改。如果在运行复制集群，升级到 Manticore 5 时需要：
  
  - 首先干净地停止所有节点。
  - 然后使用 `--new-cluster` 启动最后停止的节点（在 Linux 上运行工具 `manticore_new_cluster`）。
  - 详细信息请阅读[重启集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster)。

### 与 Manticore Columnar Library 相关的更改
* 重构了与 Columnar 存储的二级索引集成。
* [Commit efe2](https://github.com/manticoresoftware/columnar/commit/efe26b2a) Manticore Columnar Library 优化，通过部分预评估 min/max 来降低响应时间。
* [Commit 2757](https://github.com/manticoresoftware/columnar/commit/2757b99b) 如果磁盘块合并被中断，守护进程现在会清理与 MCL 相关的临时文件。
* [Commit e9c6](https://github.com/manticoresoftware/manticoresearch/commit/e9c672014b63e2b49e7a5a53880583af86c3ab4e) 崩溃时会将 Columnar 和二级库的版本记录到日志中。
* [Commit f5e8](https://github.com/manticoresoftware/columnar/commit/f5e84eeb8b6cb102f13e3468246702427527fef9) 为二级索引添加了快速文档列表回退的支持。
* [Commit 06df](https://github.com/manticoresoftware/manticoresearch/commit/06dfdd2e668933495dd6cb52d5ba2e0aa9b26e58) 在使用 MCL 时，像 `select attr, count(*) from plain_index`（没有过滤）这样的查询现在更快了。
* [Commit 0a76](https://github.com/manticoresoftware/manticoresearch/commit/0a76ecb9adbe886868b962dd425de5f475bbf55e) @@autocommit 在 HandleMysqlSelectSysvar 中用于兼容 MySQL 大于 8.25 的 .NET 连接器。
* **⚠️ 重大变更**: [MCL Issue #17](https://github.com/manticoresoftware/columnar/issues/17) MCL：为列存扫描添加了 SSE 代码。MCL 现在至少需要 SSE4.2。

### 与打包相关的更改
* [Commit 4d19](https://github.com/manticoresoftware/manticoresearch/commit/4d19f5cbe49a31228aa09253f061165bfe80e51b) **⚠️ 重大变更**: 已停止对 Debian Stretch 和 Ubuntu Xenial 的支持。
* 支持 RHEL 9，包括 Centos 9、Alma Linux 9 和 Oracle Linux 9。
* [Issue #924](https://github.com/manticoresoftware/manticoresearch/issues/924) 支持 Debian Bookworm。
* [Issue #636](https://github.com/manticoresoftware/manticoresearch/issues/636) 打包：为 Linux 和 MacOS 提供 arm64 构建。
* [PR #26](https://github.com/manticoresoftware/docker/pull/26) 多架构（x86_64 / arm64）Docker 镜像。
* [简化了贡献者的包构建流程](../Installation/Compiling_from_sources.md#Building-using-CI-Docker)。
* 现在可以使用 APT 安装指定版本。
* [Commit a6b8](https://github.com/manticoresoftware/manticoresearch/commit/51fddca5c2a3ebb8576fae4c18660656ba22de0f) Windows 安装程序（之前只提供了压缩包）。
* Switched to compiling using CLang 15.
* **⚠️ 重大变更**: 自定义 Homebrew 配方，包括 Manticore Columnar Library 的配方。要安装 Manticore、MCL 和其他必要组件，请使用以下命令 `brew install manticoresoftware/manticore/manticoresearch manticoresoftware/manticore/manticore-extra`。

### Bug 修复
* [Issue #479](https://github.com/manticoresoftware/manticoresearch/issues/479) 名为 `text` 的字段问题
* [Issue #501](https://github.com/manticoresoftware/manticoresearch/issues/501) id 不能为非 bigint
* [Issue #646](https://github.com/manticoresoftware/manticoresearch/issues/646) ALTER 与名为 "text" 的字段冲突
* ❗[Issue #652](https://github.com/manticoresoftware/manticoresearch/issues/652) 可能的BUG：HTTP (JSON) 中的 offset 和 limit 影响 facet 结果
* ❗[Issue #827](https://github.com/manticoresoftware/manticoresearch/issues/827) Searchd 在高负载下挂起/崩溃
* ❗[Issue #996](https://github.com/manticoresoftware/manticoresearch/issues/996) PQ 索引内存不足
* ❗[Commit 1041](https://github.com/manticoresoftware/manticoresearch/commit/10416ef7dddf06c0d759e32ccd6ebaa2468f7cbf) `binlog_flush = 1` 自 Sphinx 以来一直有问题，已修复
* [MCL Issue #14](https://github.com/manticoresoftware/columnar/issues/14) MCL：当 ft 字段过多时，选择操作崩溃
* [Issue #470](https://github.com/manticoresoftware/manticoresearch/issues/470) sql_joined_field 无法存储
* [Issue #713](https://github.com/manticoresoftware/manticoresearch/issues/713) 使用 LEVENSHTEIN() 时崩溃
* [Issue #743](https://github.com/manticoresoftware/manticoresearch/issues/743) Manticore 意外崩溃，无法正常重启
* [Issue #788](https://github.com/manticoresoftware/manticoresearch/issues/788) 通过 /sql 调用 KEYWORDS 返回控制字符，破坏了 JSON
* [Issue #789](https://github.com/manticoresoftware/manticoresearch/issues/789) mariadb 无法创建 FEDERATED 表
* [Issue #796](https://github.com/manticoresoftware/manticoresearch/issues/796) 警告：dlopen() 失败：/usr/bin/lib_manticore_columnar.so: 无法打开共享对象文件：没有这样的文件或目录
* [Issue #797](https://github.com/manticoresoftware/manticoresearch/issues/797) 通过 API 执行 ZONESPAN 搜索时 Manticore 崩溃
* [Issue #799](https://github.com/manticoresoftware/manticoresearch/issues/799) 在使用多个索引和 facet distinct 时权重错误
* [Issue #801](https://github.com/manticoresoftware/manticoresearch/issues/801) SphinxQL 组查询在 SQL 索引重新处理后挂起
* [Issue #802](https://github.com/manticoresoftware/manticoresearch/issues/802) MCL：5.0.2 版和 manticore-columnar-lib 1.15.4 中的索引器崩溃
* [Issue #813](https://github.com/manticoresoftware/manticoresearch/issues/813) Manticore 5.0.2 在 MySQL 8.0.28 上的 FEDERATED 返回空结果集
* [Issue #824](https://github.com/manticoresoftware/manticoresearch/issues/824) 在 2 个索引上执行 select COUNT DISTINCT 且结果为零时引发内部错误
* [Issue #826](https://github.com/manticoresoftware/manticoresearch/issues/826) 在删除查询时崩溃
* [Issue #843](https://github.com/manticoresoftware/manticoresearch/issues/843) MCL：长文本字段的 Bug
* [Issue #856](https://github.com/manticoresoftware/manticoresearch/issues/856) 5.0.2 rtindex：聚合搜索的限制行为不符合预期
* [Issue #863](https://github.com/manticoresoftware/manticoresearch/issues/863) 返回的 Hits 是 Nonetype 对象，即使是应该返回多个结果的搜索
* [Issue #870](https://github.com/manticoresoftware/manticoresearch/issues/870) 在 SELECT 表达式中使用 Attribute 和 Stored Field 时崩溃
* [Issue #872](https://github.com/manticoresoftware/manticoresearch/issues/872) 崩溃后表变得不可见
* [Issue #877](https://github.com/manticoresoftware/manticoresearch/issues/877) 搜索查询中包含两个负面术语时出现错误：查询不可计算
* [Issue #878](https://github.com/manticoresoftware/manticoresearch/issues/878) 通过 json query_string 执行的 `a -b -c` 不工作
* [Issue #886](https://github.com/manticoresoftware/manticoresearch/issues/886) 带有查询匹配的 pseudo_sharding
* [Issue #893](https://github.com/manticoresoftware/manticoresearch/issues/893) Manticore 5.0.2 的 min/max 函数未按预期工作
* [Issue #896](https://github.com/manticoresoftware/manticoresearch/issues/896) 字段 "weight" 未正确解析
* [Issue #897](https://github.com/manticoresoftware/manticoresearch/issues/897) Manticore 服务启动时崩溃并不断重启
* [Issue #900](https://github.com/manticoresoftware/manticoresearch/issues/900) group by j.a 出现问题
* [Issue #913](https://github.com/manticoresoftware/manticoresearch/issues/913) 在 ranker 中使用 expr 时，若查询包含两个接近度时，Searchd 崩溃
* [Issue #916](https://github.com/manticoresoftware/manticoresearch/issues/916) net_throttle_action 功能失效
* [Issue #919](https://github.com/manticoresoftware/manticoresearch/issues/919) MCL：Manticore 在执行查询和集群恢复期间崩溃
* [Issue #925](https://github.com/manticoresoftware/manticoresearch/issues/925) SHOW CREATE TABLE 输出中缺少反引号
* [Issue #930](https://github.com/manticoresoftware/manticoresearch/issues/930) 现在可以通过 JDBC 连接器从 Java 查询 Manticore
* [Issue #933](https://github.com/manticoresoftware/manticoresearch/issues/933) bm25f 排名问题
* [Issue #934](https://github.com/manticoresoftware/manticoresearch/issues/934) 无配置索引在首次加载状态下冻结于 watchdog 中
* [Issue #937](https://github.com/manticoresoftware/manticoresearch/issues/937) 排序 facet 数据时发生段错误
* [Issue #940](https://github.com/manticoresoftware/manticoresearch/issues/940) 在 CONCAT(TO_STRING) 时崩溃
* [Issue #947](https://github.com/manticoresoftware/manticoresearch/issues/947) 在某些情况下，一个简单的 select 可能导致整个实例挂起，使用户无法登录或执行其他查询，直到该 select 完成
* [Issue #948](https://github.com/manticoresoftware/manticoresearch/issues/948) 索引器崩溃
* [Issue #950](https://github.com/manticoresoftware/manticoresearch/issues/950) 从 facet distinct 获取的计数错误
* [Issue #953](https://github.com/manticoresoftware/manticoresearch/issues/953) 在内置 sph04 ranker 中 LCS 计算错误
* [Issue #955](https://github.com/manticoresoftware/manticoresearch/issues/955) 5.0.3 开发版崩溃
* [Issue #963](https://github.com/manticoresoftware/manticoresearch/issues/963) 使用 json 和 columnar 引擎的 FACET 崩溃
* [Issue #982](https://github.com/manticoresoftware/manticoresearch/issues/982) MCL：5.0.3 中二级索引导致崩溃
* [PR #984](https://github.com/manticoresoftware/manticoresearch/pull/984) 在 HandleMysqlSelectSysvar 中支持 @@autocommit
* [PR #985](https://github.com/manticoresoftware/manticoresearch/pull/985) 修复 RT 索引中的线程块分配问题
* [Issue #985](https://github.com/manticoresoftware/manticoresearch/issues/985) 修复 RT 索引中的线程块分配问题
* [Issue #986](https://github.com/manticoresoftware/manticoresearch/issues/986) 默认 max_query_time 设置错误
* [Issue #987](https://github.com/manticoresoftware/manticoresearch/issues/987) 使用正则表达式在多线程执行时导致崩溃
* [Issue #988](https://github.com/manticoresoftware/manticoresearch/issues/988) 破损的向后索引兼容性
* [Issue #989](https://github.com/manticoresoftware/manticoresearch/issues/989) indextool 检查 columnar 属性时报错
* [Issue #990](https://github.com/manticoresoftware/manticoresearch/issues/990) json grouper 克隆导致内存泄漏
* [Issue #991](https://github.com/manticoresoftware/manticoresearch/issues/991) levenshtein 函数克隆导致内存泄漏
* [Issue #992](https://github.com/manticoresoftware/manticoresearch/issues/992) 加载 meta 时丢失错误信息
* [Issue #993](https://github.com/manticoresoftware/manticoresearch/issues/993) 动态索引/子键和 sysvars 的错误传播
* [Issue #994](https://github.com/manticoresoftware/manticoresearch/issues/994) 在 columnar 存储中对 columnar 字符串执行 count distinct 时崩溃
* [Issue #995](https://github.com/manticoresoftware/manticoresearch/issues/995) MCL: 从 taxi1 查询 min(pickup_datetime) 时崩溃
* [Issue #997](https://github.com/manticoresoftware/manticoresearch/issues/997) 空的 excludes JSON 查询从 select 列表中删除列
* [Issue #998](https://github.com/manticoresoftware/manticoresearch/issues/998) 当前调度器上运行的二级任务有时会导致异常副作用
* [Issue #999](https://github.com/manticoresoftware/manticoresearch/issues/999) 使用 facet distinct 和不同模式时崩溃
* [Issue #1000](https://github.com/manticoresoftware/manticoresearch/issues/1000) MCL：在没有 columnar 库的情况下运行后 Columnar rt 索引损坏
* [Issue #1001](https://github.com/manticoresoftware/manticoresearch/issues/1001) 隐式 cutoff 在 JSON 中不起作用
* [Issue #1002](https://github.com/manticoresoftware/manticoresearch/issues/1002) Columnar grouper 问题
* [Issue #1003](https://github.com/manticoresoftware/manticoresearch/issues/1003) 无法从索引中删除最后一个字段
* [Issue #1004](https://github.com/manticoresoftware/manticoresearch/issues/1004) --new-cluster 后的行为异常
* [Issue #1005](https://github.com/manticoresoftware/manticoresearch/issues/1005) “未加载 columnar 库”，但并不需要加载
* [Issue #1006](https://github.com/manticoresoftware/manticoresearch/issues/1006) delete 查询无错误反馈
* [Issue #1010](https://github.com/manticoresoftware/manticoresearch/issues/1010) 修复 Windows 版本中的 ICU 数据文件位置
* [PR #1018](https://github.com/manticoresoftware/manticoresearch/pull/1018) 握手发送问题
* [Issue #1020](https://github.com/manticoresoftware/manticoresearch/issues/1020) 在 show create table 中显示 id
* [Issue #1024 crash 1](https://github.com/manticoresoftware/manticoresearch/issues/1024) Facet 搜索结果数量较大时崩溃/分段错误
* [Issue #1026](https://github.com/manticoresoftware/manticoresearch/issues/1026) RT 索引：在插入大量文档且 RAMchunk 满时，searchd 无限卡住
* [Commit 4739](https://github.com/manticoresoftware/manticoresearch/commit/4739dafd) 线程在节点间复制繁忙时关闭被卡住
* [Commit ab87](https://github.com/manticoresoftware/manticoresearch/commit/ab87836782e7ae43fe5f9dea739eed0d336b42c1) JSON 范围过滤中混合使用浮点数和整数可能导致 Manticore 忽略过滤器
* [Commit d001](https://github.com/manticoresoftware/manticoresearch/commit/d00101c2905f6393ce3fca23c4f6dcb2506f4bd9) JSON 中的浮点过滤器不准确
* [Commit 4092](https://github.com/manticoresoftware/manticoresearch/commit/4092c6a8) 如果索引更改，则丢弃未提交的事务（否则可能会崩溃）
* [Commit 9692](https://github.com/manticoresoftware/manticoresearch/commit/9692a840) 使用反斜杠时的查询语法错误
* [Commit 0c19](https://github.com/manticoresoftware/manticoresearch/commit/0c19ad8d) 在 SHOW STATUS 中 workers_clients 可能错误
* [Commit 1772](https://github.com/manticoresoftware/manticoresearch/commit/17720e1e2f2ec38337df565bee78f7ce9d9c744a) 修复了在合并没有文档存储的 RAM 段时崩溃的问题
* [Commit f45b](https://github.com/manticoresoftware/manticoresearch/commit/f45b337b5c265433bb7d6262ad65d02f65edbaf3) 修复了等于 JSON 过滤器中缺失的 ALL/ANY 条件
* [Commit 3e83](https://github.com/manticoresoftware/manticoresearch/commit/3e83c1f0c0e6e7a9d4297113f69a112cbabd9e64) 如果从不可写目录启动 searchd，复制可能会失败，错误信息为 `got exception while reading ist stream: mkstemp(./gmb_pF6TJi) failed: 13 (Permission denied)`
* [Commit 92e5](https://github.com/manticoresoftware/manticoresearch/commit/92e5629f7150f62524b385fca670a01ba9590f5b) 自 4.0.2 起崩溃日志仅包含偏移量，此提交修复了此问题。

# Version 5.0.2
发布日期：2022年5月30日 

### Bugfixes
* ❗[Issue #791](https://github.com/manticoresoftware/manticoresearch/issues/791) - May 30th 

# Version 5.0.0
发布日期：2022年5月18日


### 主要新功能
* 🔬 支持 [Manticore Columnar Library 1.15.2](https://github.com/manticoresoftware/columnar/)，启用 beta 版的 [二级索引](../Server_settings/Searchd.md#secondary_indexes)。默认情况下，平面和实时列式及行式索引的二级索引构建处于启用状态（如果使用 [Manticore Columnar Library](https://github.com/manticoresoftware/columnar)），但要启用搜索功能，您需要在配置文件中设置 `secondary_indexes = 1` 或使用 [SET GLOBAL](../Server_settings/Setting_variables_online.md)。此新功能支持所有操作系统，但旧版 Debian Stretch 和 Ubuntu Xenial 除外。

* [只读模式](Security/Read_only.md)：现在可以指定仅处理读取查询并丢弃所有写入的监听器。

* 新增 [/cli](Connecting_to_the_server/HTTP.md#/cli) 端点，使通过 HTTP 运行 SQL 查询变得更简单。

* 通过 JSON 进行的批量 INSERT/REPLACE/DELETE 更快：之前可以通过 HTTP JSON 协议提供多个写入命令，但它们是逐个处理的，现在以单个事务处理。

* [#720](https://github.com/manticoresoftware/manticoresearch/issues/720) JSON 协议支持 [嵌套过滤器](../Searching/Filters.md#Nested-bool-query)。之前无法在 JSON 中编写类似 `a=1 and (b=2 or c=3)` 的代码：`must`（AND）、`should`（OR）和 `must_not`（NOT）仅在最高层级上工作。现在它们可以嵌套。

* HTTP 协议支持 [分块传输编码](https://en.wikipedia.org/wiki/Chunked_transfer_encoding)。现在可以在应用程序中使用分块传输以减少资源消耗来传输大批量数据（因为计算 `Content-Length` 是不必要的）。在服务器端，Manticore 现在始终以流式方式处理传入的 HTTP 数据，而无需等待整个批量数据传输完毕，这样可以：
  - 降低峰值内存使用，降低 OOM 风险
  - 减少响应时间（我们的测试表明处理 100MB 批量数据的响应时间减少了 11%）
  - 允许您绕过 [max_packet_size](../Server_settings/Searchd.md#max_packet_size)，传输大于 `max_packet_size` 最大允许值（128MB）的批量数据，例如，每次 1GB。
  
* [#719](https://github.com/manticoresoftware/manticoresearch/issues/719) HTTP 接口支持 `100 Continue`：现在可以从 `curl` 传输大批量数据（包括各种编程语言使用的 curl 库），默认情况下会发送 `Expect: 100-continue`，并在实际发送批量数据之前等待一段时间。之前需要添加 `Expect: `头，现在不需要。

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
  {"items":[{"insert":{"_index":"user","_id":2811798918248005633,"created":true,"result":"created","status":201}},{"insert":{"_index":"user","_id":2811798918248005634,"created":true,"result":"created","status":201}},{"insert":{"_index":"user","_id":2811798918248005635,"created":true,"result":"created","status":201}},{"insert":{"_index":"user","_id":2811798918248005636,"created":true,"result":"created","status":201}}],"errors":false}
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
  {"items":[{"bulk":{"_index":"user","_id":2811798919590182916,"created":4,"deleted":0,"updated":0,"result":"created","status":201}}],"errors":false}
  real	0m0.015s
  user	0m0.005s
  sys	0m0.004s
  ```

  </details>

* **⚠️ 重大变更**: [伪分片](../Server_settings/Searchd.md#pseudo_sharding)默认启用。如果您想禁用它，请确保在Manticore配置文件的`searchd`部分中添加`pseudo_sharding = 0`。

* 在实时/普通索引中至少有一个全文字段不再是强制性的。现在即使没有与全文搜索相关的内容，您也可以使用Manticore。

  [快速提取](../Creating_a_table/Data_types.md#fast_fetch)支持由[Manticore Columnar Library](https://github.com/manticoresoftware/columnar)提供的属性：像`select * from <columnar table>`这样的查询现在比以前快得多，尤其是在架构中有许多字段时。

* **⚠️ 重大变更**: 隐式[截止](../Searching/Options.md#cutoff)。Manticore现在不会花费时间和资源处理您不需要返回的结果集数据。缺点是它影响了[SHOW META](../Node_info_and_management/SHOW_META.md#SHOW-META)中的`total_found`和JSON输出中的[hits.total](../Searching/Full_text_matching/Basic_usage.md#HTTP-JSON)。现在只有在看到`total_relation: eq`时它才准确，而`total_relation: gte`意味着实际匹配的文档数量大于您获得的`total_found`值。要保留以前的行为，您可以使用搜索选项`cutoff=0`，这将使`total_relation`始终为`eq`。

* **⚠️ 重大变更**: 所有全文字段现在默认[存储](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#stored_fields)。您需要使用`stored_fields = `（空值）将所有字段设置为非存储（即恢复到以前的行为）。

* [#715](https://github.com/manticoresoftware/manticoresearch/issues/715) HTTP JSON支持[搜索选项](../Searching/Options.md#General-syntax)。

### 次要问题
* **⚠️ 重大变更**: 索引元文件格式已更改。之前的元文件（`.meta`, `.sph`）采用二进制格式，现在改为 JSON 格式。新版本的 Manticore 会自动转换旧的索引，但：
  - 你可能会收到警告，例如 `WARNING: ... syntax error, unexpected TOK_IDENT`
  - 你将无法在之前的 Manticore 版本中运行该索引，请确保你有备份。
* **⚠️ 重大变更**: 会话状态支持借助 [HTTP keep-alive](../Connecting_to_the_server/HTTP.md#Keep-alive)。这使得 HTTP 状态化，当客户端也支持时。比如，使用新的 [/cli](../Connecting_to_the_server/HTTP.md#/cli) 端点和 HTTP keep-alive（所有浏览器默认开启），你可以在 `SELECT` 后调用 `SHOW META`，效果与通过 MySQL 的操作相同。注意，之前的 `Connection: keep-alive` HTTP 头也被支持，但它仅导致重用相同的连接。从这一版本开始，它还使会话状态化。
* 你现在可以指定 `columnar_attrs = *` 来在 [纯模式](Read_this_first.md#Real-time-mode-vs-plain-mode) 中将所有属性定义为列式存储，这在属性列表较长时非常有用。
* 更快的复制 SST。
* **⚠️ 重大变更**: 复制协议已更改。如果你运行的是复制集群，则在升级到 Manticore 5 时需要：
  - 首先干净地停止所有节点
  - 然后以 `--new-cluster` 启动最后停止的节点（在 Linux 中运行工具 `manticore_new_cluster`）。
  - 详细信息请参阅 [重启集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster)。
* 复制改进：
  - 更快的 SST
  - 噪音抵抗，这在复制节点间网络不稳定的情况下会有帮助
  - 改进的日志记录
* 安全性改进：如果配置中未指定任何 `listen`，Manticore 现在会监听 `127.0.0.1` 而不是 `0.0.0.0`。虽然随 Manticore Search 提供的默认配置中已指定 `listen` 设置，并且通常不会有完全没有 `listen` 的配置，但这种情况仍然是可能的。之前，Manticore 会监听 `0.0.0.0`，这并不安全，而现在它监听 `127.0.0.1`，通常不会暴露在互联网。
* 针对列式属性的聚合查询更快。
* 提高 `AVG()` 准确度：之前 Manticore 在聚合时内部使用 `float`，现在使用 `double`，显著提高了准确度。
* 改进对 JDBC MySQL 驱动程序的支持。
* 支持 [jemalloc](https://github.com/jemalloc/jemalloc) 的 `DEBUG malloc_stats`。
* [optimize_cutoff](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#optimize_cutoff) 现在作为每个表的设置可用，可以在创建或修改表时设置。
* **⚠️ 重大变更**: [query_log_format](../Server_settings/Searchd.md#query_log_format) 现在默认值为 **`sphinxql`**。如果你习惯于 `plain` 格式，需要在配置文件中添加 `query_log_format = plain`。
* 显著减少内存消耗：在进行长时间和高强度的插入/替换/优化工作负载时，使用存储字段时 Manticore 的 RAM 消耗显著降低。
* [shutdown_timeout](Server_settings/Searchd.md#shutdown_timeout) 默认值已从 3 秒增加到 60 秒。
* [Commit ffd0](https://github.com/manticoresoftware/manticoresearch/commit/ffd0499d329d2c383f14c8a44c4cc84338ab56e7) 持 Java mysql connector >= 6.0.3：在 [Java mysql connection 6.0.3](https://mvnrepository.com/artifact/mysql/mysql-connector-java/6.0.3) 中，他们更改了连接到 mysql 的方式，这破坏了与 Manticore 的兼容性。现在支持新的行为。
* [Commit 1da6](https://github.com/manticoresoftware/manticoresearch/commit/1da6dbec) 在加载索引时禁用保存新的磁盘块（例如，在 searchd 启动时）。
* [Issue #746](https://github.com/manticoresoftware/manticoresearch/issues/746) 支持 glibc >= 2.34。
* [Issue #784](https://github.com/manticoresoftware/manticoresearch/issues/784) 统计 "VIP" 连接与普通（非 VIP）连接分开。之前 VIP 连接会计入 `max_connections` 限制，这可能导致非 VIP 连接出现 "maxed out" 错误。现在 VIP 连接不计入该限制。当前 VIP 连接的数量也可以在 `SHOW STATUS` 和 `status` 中查看。
* [ID](../Creating_a_table/Data_types.md#Document-ID) 现在可以明确指定。
* [Issue #687](https://github.com/manticoresoftware/manticoresearch/issues/687) 支持 mysql 协议的 zstd 压缩。

### ⚠️ 其他次要破坏性更改
* ⚠️ BM25F 公式已稍作更新，以改善搜索相关性。这仅影响使用函数 [BM25F()](../Functions/Searching_and_ranking_functions.md#BM25F()) 的搜索结果，并不改变默认排名公式的行为。
* ⚠️ REST [/sql](../Connecting_to_the_server/HTTP.md#mode=raw) 端点的行为已更改：`/sql?mode=raw` 现在需要转义，并返回一个数组。
* ⚠️ `/bulk` INSERT/REPLACE/DELETE请求的响应格式已更改：
  - 之前每个子查询构成一个独立的事务，并返回一个单独的响应。
  - 现在整个批处理被视为一个事务，返回一个单一的响应。
* ⚠️ 搜索选项 `low_priority` 和 `boolean_simplify` 现在需要一个值（`0/1`）：之前你可以使用 `SELECT ... OPTION low_priority, boolean_simplify`，现在需要使用 `SELECT ... OPTION low_priority=1, boolean_simplify=1`。
* ⚠️ 如果你使用旧版 [php](https://github.com/manticoresoftware/manticoresearch/blob/master/api/sphinxapi.php)、[python](https://github.com/manticoresoftware/manticoresearch/blob/master/api/sphinxapi.py) 或 [java](https://github.com/manticoresoftware/manticoresearch/tree/master/api/java) 客户端，请根据相应链接找到更新版本。**旧版本与 Manticore 5 不完全兼容。**
* ⚠️ HTTP JSON 请求现在在 `query_log_format=sphinxql` 模式下以不同格式记录。之前只记录了全文部分，现在将其原样记录。

### 新软件包
* **⚠️ 重大变更**: 由于新结构，在升级到 Manticore 5 时，建议在安装新包之前先移除旧包：
  - 基于 RPM 的系统: `yum remove manticore*`
  - Debian 和 Ubuntu: `apt remove manticore*`
* 新的 deb/rpm 包结构。以前的版本提供了：
  - `manticore-server`包含 `searchd`（主要搜索守护进程）及其所需的一切
  - `manticore-tools` 包含 `indexer` 和 `indextool`
  - `manticore` 包括所有内容
  - `manticore-all` RPM，作为元包，引用 `manticore-server` 和 `manticore-tools`

  新的结构为：
  - `manticore` - deb/rpm 元包，作为依赖安装上述所有包
  - `manticore-server-core` - `searchd` 及其独立运行所需的一切
  - `manticore-server` -  systemd 文件和其他辅助脚本
  - `manticore-tools` - `indexer`, `indextool` 和其他工具
  - `manticore-common` - 默认配置文件、默认数据目录、默认停用词
  - `manticore-icudata`, `manticore-dev`, `manticore-converter` 并没有太大变化
  - `.tgz` 包，包含所有的包
* 支持 Ubuntu Jammy
* 通过 [YUM 仓库](../Installation/RHEL_and_Centos.md#YUM-repository) 支持 Amazon Linux 2

### Bug 修复
* [Issue #815](https://github.com/manticoresoftware/manticoresearch/issues/815) 使用 UDF 函数时随机崩溃
* [Issue #287](https://github.com/manticoresoftware/manticoresearch/issues/287) 索引实时索引时内存不足
* [Issue #604](https://github.com/manticoresoftware/manticoresearch/issues/604) 重大变更 3.6.0, 4.2.0 sphinxql-parser
* [Issue #667](https://github.com/manticoresoftware/manticoresearch/issues/667) 致命错误：内存不足（无法分配 9007199254740992 字节）
* [Issue #676](https://github.com/manticoresoftware/manticoresearch/issues/676) 字符串未正确传递给 UDF
* ❗[Issue #698](https://github.com/manticoresoftware/manticoresearch/issues/698) 在尝试向实时索引添加文本列后，searchd 崩溃
* [Issue #705](https://github.com/manticoresoftware/manticoresearch/issues/705) 索引器无法找到所有列
* ❗[Issue #709](https://github.com/manticoresoftware/manticoresearch/issues/705) 按 json.boolean 分组时出现问题
* [Issue #716](https://github.com/manticoresoftware/manticoresearch/issues/716) 与索引相关的 indextool 命令（例如 --dumpdict）失败
* ❗[Issue #724](https://github.com/manticoresoftware/manticoresearch/issues/724) 选择中的字段消失
* [Issue #727](https://github.com/manticoresoftware/manticoresearch/issues/727) 使用 `application/x-ndjson` 时 .NET HttpClient Content-Type 不兼容
* [Issue #729](https://github.com/manticoresoftware/manticoresearch/issues/729) 字段长度计算
* ❗[Issue #730](https://github.com/manticoresoftware/manticoresearch/issues/730) 创建/插入/删除列式表时存在内存泄漏
* [Issue #731](https://github.com/manticoresoftware/manticoresearch/issues/731) 在某些条件下结果中的空列
* ❗[Issue #749](https://github.com/manticoresoftware/manticoresearch/issues/749) 守护进程启动时崩溃
* ❗[Issue #750](https://github.com/manticoresoftware/manticoresearch/issues/750) 守护进程启动时挂起
* ❗[Issue #751](https://github.com/manticoresoftware/manticoresearch/issues/751) 在 SST 时崩溃
* [Issue #752](https://github.com/manticoresoftware/manticoresearch/issues/752) json 属性在引擎为 'columnar' 时标记为列式
* [Issue #753](https://github.com/manticoresoftware/manticoresearch/issues/753) 复制监听在 0
* [Issue #754](https://github.com/manticoresoftware/manticoresearch/issues/754) columnar_attrs = * 与 csvpipe 不兼容
* ❗[Issue #755](https://github.com/manticoresoftware/manticoresearch/issues/755) 在 rt 中选择浮点数时崩溃
* ❗[Issue #756](https://github.com/manticoresoftware/manticoresearch/issues/756) Indextool 在检查时更改实时索引
* [Issue #757](https://github.com/manticoresoftware/manticoresearch/issues/757) 需要检查监听端口范围交叉
* [Issue #758](https://github.com/manticoresoftware/manticoresearch/issues/758) 在 RT 索引未能保存磁盘块时记录原始错误
* [Issue #759](https://github.com/manticoresoftware/manticoresearch/issues/759) 仅报告一个 RE2 配置错误
* ❗[Issue #760](https://github.com/manticoresoftware/manticoresearch/issues/760) 提交中内存消耗变化的错误
* [Issue #761](https://github.com/manticoresoftware/manticoresearch/issues/761) 第 3 个节点在脏重启后未能成为非主集群
* [Issue #762](https://github.com/manticoresoftware/manticoresearch/issues/762) 更新计数器增加 2
* [Issue #763](https://github.com/manticoresoftware/manticoresearch/issues/763) 新版本 4.2.1 通过使用形态学创建的 4.2.0 损坏索引
* [Issue #764](https://github.com/manticoresoftware/manticoresearch/issues/764) /sql?mode=raw 中 json 键没有转义
* ❗[Issue #765](https://github.com/manticoresoftware/manticoresearch/issues/765) 使用函数时隐藏其他值
* ❗[Issue #766](https://github.com/manticoresoftware/manticoresearch/issues/766) 在 FixupAttrForNetwork 中触发内存泄漏
* ❗[Issue #767](https://github.com/manticoresoftware/manticoresearch/issues/767) 4.2.0 和 4.2.1 中与文档存储缓存相关的内存泄漏
* [Issue #768](https://github.com/manticoresoftware/manticoresearch/issues/768) 通过网络存储字段时出现奇怪的 ping-pong
* [Issue #769](https://github.com/manticoresoftware/manticoresearch/issues/769) 如果未在“common”部分中提及，则 lemmatizer_base 重置为空
* [Issue #770](https://github.com/manticoresoftware/manticoresearch/issues/770) pseudo_sharding 导致通过 id 的 SELECT 变慢
* [Issue #771](https://github.com/manticoresoftware/manticoresearch/issues/771) DEBUG malloc_stats 输出在使用 jemalloc 时为零
* [Issue #772](https://github.com/manticoresoftware/manticoresearch/issues/772) 删除/添加列使值不可见
* [Issue #773](https://github.com/manticoresoftware/manticoresearch/issues/773) 无法将列 bit(N) 添加到列式表
* [Issue #774](https://github.com/manticoresoftware/manticoresearch/issues/774) 在 manticore.json 启动时“cluster”变为空
* ❗[Commit 1da4](https://github.com/manticoresoftware/manticoresearch/commits/1da4ce891e4ff13727cfd3331d72771c64ee7949) HTTP 操作未在 SHOW STATUS 中跟踪
* [Commit 3810](https://github.com/manticoresoftware/manticoresearch/commits/381000ab4af4c3c98ec8c730699ad7f39039cec8) 为低频单关键词查询禁用 pseudo_sharding
* [Commit 8003](https://github.com/manticoresoftware/manticoresearch/commits/800325cca283a87801b28929d82420fafc76a0ee) 修复存储属性与索引合并
* [Commit cddf](https://github.com/manticoresoftware/manticoresearch/commits/cddfeed6296f2041c6aae18414e5cd9cabf08281) 泛化独特值提取器；为列式字符串添加专用的独特提取器
* [Commit fba4](https://github.com/manticoresoftware/manticoresearch/commits/fba4bb4f) 修复从文档存储提取空整数属性
* [Commit f300](https://github.com/manticoresoftware/manticoresearch/commit/f3009a9242fa16956adf871c4887e6e0303ba364) `ranker` 在查询日志中可能被指定两次

## Version 4.2.0，2021年12月23日

### 主要新功能
* **实时索引和全文查询的伪分片支持**。在之前的版本中，我们添加了有限的伪分片支持。从这个版本开始，您可以充分利用伪分片和多核处理器，只需启用 [searchd.pseudo_sharding](Server_settings/Searchd.md#pseudo_sharding)。最棒的是，您无需对索引或查询进行任何修改，只需启用它，如果有空闲的 CPU，就会被用来降低响应时间。它支持用于全文、过滤和分析查询的普通和实时索引。例如，启用伪分片后，可以使大多数查询的**平均响应时间降低约 10 倍**，在 [Hacker News 精选评论数据集](https://zenodo.org/record/45901/) 中，文档数量乘以 100（1.16 亿文档在普通索引中）。

![Pseudo sharding on vs off in 4.2.0](4.2.0_ps_on_vs_off.png)

* [**Debian Bullseye**](https://manticoresearch.com/install/) 现在支持。

<!-- example pq_transactions_4.2.0 -->
* PQ 事务现在是原子和隔离的。之前，PQ 事务的支持有限。这使得 **REPLACE into PQ** 的速度大幅提高，特别是在一次性替换大量规则时。性能细节：

<!-- intro -->
#### 4.0.2

<!-- request 4.0.2 -->

插入 **100万个 PQ 规则** 需要 **48秒**，而在 **10,000个批次中替换仅 40,000** 个规则则需要 **406秒**。

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

插入 **100万个 PQ 规则** 需要 **34秒**，而在 **10,000个批次中替换这些规则** 则需要 **23秒**。

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

### 次要问题
*  [optimize_cutoff](Server_settings/Searchd.md#optimize_cutoff) 现在可以作为配置选项在 `searchd` 部分使用。它在您希望全球限制所有索引中的实时块数量时非常有用。
* [Commit 0087](https://github.com/manticoresoftware/manticoresearch/commit/00874743823eaf43e2a09a088392fd7e0a617f47) 支持准确的 [count(distinct ...)](Searching/Grouping.md#COUNT(DISTINCT-field)) 和 [FACET ... distinct](Searching/Faceted_search.md#Faceting-without-duplicates) 针对多个本地物理索引（实时/普通）具有相同字段设置/顺序。
* [PR #598](https://github.com/manticoresoftware/manticoresearch/pull/598) 为 `YEAR()` 和其他时间戳函数提供 bigint 支持。
* [Commit 8e85](https://github.com/manticoresoftware/manticoresearch/commit/8e85d4bce596cc88f9d9615ea243a27e4afde0f7) 自适应 [rt_mem_limit](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_mem_limit)。以前，Manticore Search 会精确收集到 `rt_mem_limit` 的数据后才保存新的磁盘块，而在保存时仍然会收集多达 10% 的数据（即双缓冲），以减少可能的插入暂停。如果该限制也被耗尽，添加新文档会被阻止，直到磁盘块完全保存到磁盘。新的自适应限制基于我们现在拥有的 [auto-optimize](Server_settings/Searchd.md#auto_optimize)，因此如果磁盘块没有完全遵循 `rt_mem_limit` 而开始提前刷新，就没有太大问题。因此，现在我们收集最多 50% 的 `rt_mem_limit` 并将其保存为磁盘块。在保存时，我们查看统计数据（保存了多少，保存时到达了多少新文档）并重新计算下次将使用的初始速率。例如，如果我们保存了 9000 万个文档，而在保存期间又到达了 1000 万个文档，则速率为 90%，因此我们知道下次可以收集最多 90% 的 `rt_mem_limit` 后再开始刷新另一个磁盘块。该速率值会自动计算，范围从 33.3% 到 95%。
* [Issue #628](https://github.com/manticoresoftware/manticoresearch/issues/628) 为 PostgreSQL 源提供 [unpack_zlib](Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Database_connection.md#unpack_zlib)。感谢 [Dmitry Voronin](https://github.com/dimv36) 的 [贡献](https://github.com/manticoresoftware/manticoresearch/pull/630)。
* [Commit 6d54](https://github.com/manticoresoftware/manticoresearch/commit/6d54cf2b319b131970c29410cc21abcbdf8083b1) 支持 `indexer -v` 和 `--version`。以前您仍然可以看到索引器的版本，但不支持 `-v`/`--version`。
* [Issue #662](https://github.com/manticoresoftware/manticoresearch/issues/662) 默认情况下通过 systemd 启动时无穷的 mlock 限制。
* [Commit 63c8](https://github.com/manticoresoftware/manticoresearch/commit/63c8cd05cf399e705c2c14939411a92cb784735a) 将自旋锁转换为协作读写锁的操作队列。
* [Commit 4113](https://github.com/manticoresoftware/manticoresearch/commit/41130ce30d1734bbf8e6b8b63c4e815caf9464f2) 环境变量 `MANTICORE_TRACK_RT_ERRORS` 有助于调试实时段损坏问题。

### 重大变更
* Binlog 版本已增加，之前版本的 binlog 不会被重放，因此在升级期间请确保干净地停止 Manticore Search：停止之前的实例后，`/var/lib/manticore/binlog/` 中不应有除 `binlog.meta` 之外的 binlog 文件。
* [Commit 3f65](https://github.com/manticoresoftware/manticoresearch/commit/3f659f36e87d99ee262205a8eae4663a255be630) 在 `show threads option format=all` 中新增了 "chain" 列。该列显示了一些任务信息票据的堆栈，对性能分析非常有用，因此如果您在解析 `show threads` 输出时，请注意这个新列。
* `searchd.workers` 自 3.5.0 版本以来已被弃用，现在标记为不推荐使用。如果您的配置文件中仍然存在该项，将在启动时触发警告。Manticore Search 会启动，但会有警告。
* 如果您使用 PHP 和 PDO 访问 Manticore，需要设置 `PDO::ATTR_EMULATE_PREPARES`。

### Bug 修复
* ❗[Issue #650](https://github.com/manticoresoftware/manticoresearch/issues/650) Manticore 4.0.2 的性能慢于 Manticore 3.6.3。虽然 4.0.2 在批量插入方面比之前版本更快，但单个文档插入显著变慢。此问题已在 4.2.0 中修复。
* ❗[Commit 22f4](https://github.com/manticoresoftware/manticoresearch/commit/22f4141bed1411c83684ae6e0bae832071cce3df) 在高强度 REPLACE 负载下，RT 索引可能会损坏，或者可能会崩溃。
* [Commit 03be](https://github.com/manticoresoftware/manticoresearch/commit/03be91e4f720a3f20bee0278dd8f0d44abd3ef0e) 修复了在合并分组器和分组 N 排序器时的平均值问题；修复了聚合的合并问题。
* [Commit 2ea5](https://github.com/manticoresoftware/manticoresearch/commit/2ea575d34efe38661f14d858f2b552456f29fe42) `indextool --check` 可能崩溃。
* [Commit 7ec7](https://github.com/manticoresoftware/manticoresearch/commit/7ec76d4abf01b5fca6f10a0000fe64d5d8e5fa52) 由于 UPDATE 操作导致的 RAM 耗尽问题。
* [Commit 658a](https://github.com/manticoresoftware/manticoresearch/commit/658a727e41f3c3fbcde9754149528f34ed76ba19) 守护进程可能在 INSERT 操作时挂起。
* [Commit 46e4](https://github.com/manticoresoftware/manticoresearch/commit/46e42b9b386e31e53247ed634e158b533e28063e) 守护进程可能在关闭时挂起。
* [Commit f8d7](https://github.com/manticoresoftware/manticoresearch/commit/f8d7d5177cc83d250b493b4cfc619732b29cf072) 守护进程可能在关闭时崩溃。
* [Commit 733a](https://github.com/manticoresoftware/manticoresearch/commit/733accf165142a1d1574689525ccca8907971158) 守护进程可能在崩溃时挂起。
* [Commit f7f8](https://github.com/manticoresoftware/manticoresearch/commit/f7f8bd8c0dbb23a59a88502dcba16b0918fb2330) 守护进程可能在启动时崩溃，尝试重新加入无效节点列表的集群。
* [Commit 1401](https://github.com/manticoresoftware/manticoresearch/commit/140155613821ecaddcd1a3dfc4f7369b0df6aef7) 如果在启动时无法解析其代理之一，分布式索引可能在实时模式下完全被遗忘。
* [Issue #683](https://github.com/manticoresoftware/manticoresearch/issues/683) `attr bit(N) engine='columnar'` 失败。
* [Issue #682](https://github.com/manticoresoftware/manticoresearch/issues/682) 创建表失败，但目录被保留。
* [Issue #663](https://github.com/manticoresoftware/manticoresearch/issues/663) 配置失败，出现未知键名 'attr_update_reserve'。
* [Issue #632](https://github.com/manticoresoftware/manticoresearch/issues/632) 批量查询导致崩溃。
* [Issue #679](https://github.com/manticoresoftware/manticoresearch/issues/679) 批量查询在 v4.0.3 中再次导致崩溃。
* [Commit f7f8](https://github.com/manticoresoftware/manticoresearch/commit/f7f8bd8c0dbb23a59a88502dcba16b0918fb2330) 修复守护进程启动时因无效节点列表而崩溃的问题。
* [Issue #643](https://github.com/manticoresoftware/manticoresearch/issues/643) Manticore 4.0.2 在批量插入后不接受连接。
* [Issue #635](https://github.com/manticoresoftware/manticoresearch/issues/635) FACET 查询与 JSON 字段或字符串属性的 ORDER BY 可能崩溃。
* [Issue #634](https://github.com/manticoresoftware/manticoresearch/issues/634) 使用 packedfactors 查询时出现 SIGSEGV 崩溃。
* [Commit 4165](https://github.com/manticoresoftware/manticoresearch/commits/41657f15f8fcf1e93cfad7f835bcf705d6e8349e) `morphology_skip_fields` 不支持创建表。

## Version 4.0.2, 2021年9月21日

### 主要新功能
- **全面支持 [Manticore Columnar Library](https://github.com/manticoresoftware/columnar)**。之前，Manticore Columnar Library 仅支持普通索引。现在它在以下方面得到了支持：
  
  - 实时索引的 `INSERT`、`REPLACE`、`DELETE`、`OPTIMIZE``
  - 复制
  - `ALTER` 操作
  - `indextool --check`
- **自动索引压缩** ([Issue #478](https://github.com/manticoresoftware/manticoresearch/issues/478))。现在您无需手动调用 OPTIMIZE 或通过定时任务或其他自动化方式进行操作。Manticore 现在默认会自动进行压缩。您可以通过 [optimize_cutoff](../Server_settings/Setting_variables_online.md) 全局变量设置默认的压缩阈值。
- **块快照和锁系统的改进**。这些更改可能在初看时不易察觉，但它们显著改善了许多实时索引中的操作行为。简单来说，之前 Manticore 的大多数数据操作依赖于重锁，现在我们使用磁盘块快照来替代。
- **显著提升实时索引的批量 INSERT 性能**。例如，在 [Hetzner 的 AX101 服务器](https://www.hetzner.com/dedicated-rootserver/ax101) 上，配备 SSD、128GB RAM 和 AMD Ryzen™ 9 5950X（16*2 核心），**使用 3.6.0 时，您可以每秒插入 236K 文档**到具有架构 `name text, email string, description text, age int, active bit(1)` 的表中（默认 `rt_mem_limit`，批量大小 25000，16 个并发插入工作者，总共插入 1600 万文档）。在 4.0.2 中，相同的并发/批量/计数可以达到 **357K 文档每秒**。

  <details>

  - 读取操作（例如 SELECT 和复制）使用快照执行。
  - 仅更改内部索引结构而不修改模式或文档的操作（例如合并 RAM 段、保存磁盘块、合并磁盘块）使用只读快照执行，并在最后替换现有块。
  - UPDATE 和 DELETE 操作是在现有块上执行，但在可能发生合并的情况下，写入会被收集，然后应用于新块。
  - UPDATE 操作为每个块依次获取独占锁。合并在收集块属性时会获取共享锁。因此，同一时间只能有一个（合并或更新）操作可以访问该块的属性。
  - 当合并进入需要属性的阶段时，会设置一个特殊标志。UPDATE 完成后会检查该标志，如果已设置，则整个更新会存储在一个特殊集合中。最终，当合并完成时，会将更新应用到新生成的磁盘块上。
  - ALTER 操作通过独占锁运行。
  - 复制操作作为常规读取操作运行，但在 SST 之前会保存属性，并在 SST 期间禁止更新。

  </details>

- **[ALTER](Updating_table_schema_and_settings.md) 现在可以添加/移除全文字段**（在实时模式下）。之前只能添加/移除属性。
- 🔬 **实验性：针对全表扫描查询的伪分片** - 允许并行化任何非全文搜索查询。您现在可以通过启用新的选项 [searchd.pseudo_sharding](Server_settings/Searchd.md#pseudo_sharding) 来实现，而不需要手动准备分片，预计非全文搜索查询的响应时间会降低到接近 `CPU 核心数`。请注意，它可能会占用所有现有的 CPU 核心，因此如果您不仅关心延迟，还关心吞吐量，请谨慎使用。

### 次要问题
<!-- example -->
- Linux Mint 和 Ubuntu Hirsute Hippo 现在通过 [APT 仓库](Installation/Debian_and_Ubuntu.md#APT-repository) 支持。
- 在某些情况下（取决于 ID 的分布），在大索引中通过 HTTP 进行 ID 更新速度更快。
- [671e65a2](https://github.com/manticoresoftware/lemmatizer-uk/commit/671e65a296800537123a3f8b324eeaa3f58b9632) - 为 lemmatizer-uk 添加了缓存功能。


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
- systemd 的自定义启动标志。现在，如果需要使用特定的启动标志运行 Manticore，您无需手动启动 searchd。
- 新功能 [LEVENSHTEIN()](Functions/String_functions.md#LEVENSHTEIN())，用于计算 Levenshtein 距离。
- 新增 searchd 启动标志 `--replay-flags=ignore-trx-errors` 和 `--replay-flags=ignore-all-errors`，这样即使 binlog 损坏，仍然可以启动 searchd。
- [Issue #621](https://github.com/manticoresoftware/manticoresearch/issues/621) - 曝露来自 RE2 的错误。
- 更准确的 COUNT(DISTINCT) 适用于由本地平面索引组成的分布式索引。
- FACET DISTINCT 以消除在进行分面搜索时的重复项。
- 确切形式修饰符 现在不再需要 词法分析，并且适用于启用了 中缀/前缀 搜索的索引。

### 重大变更
- 新版本可以读取旧版本的索引，但旧版本无法读取 Manticore 4 的索引。
- 移除了隐式按 ID 排序。需要时请显式排序。
- `charset_table` 的默认值从 `0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451` 更改为 `non_cjk`。
- `OPTIMIZE` 自动进行。如果不需要，请确保在配置文件的 `searchd` 部分设置 `auto_optimize=0`。
- [Issue #616](https://github.com/manticoresoftware/manticoresearch/issues/616) `ondisk_attrs_default` 已被弃用，现在已移除。
- 对于贡献者：我们现在在 Linux 构建中使用 Clang 编译器，根据我们的测试，它可以构建出更快的 Manticore Search 和 Manticore Columnar Library。
- 如果搜索查询中未指定 [max_matches](Searching/Options.md#max_matches)，它会隐式更新为所需的最低值，以优化新列式存储的性能。这可能会影响 [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META) 中的 `total` 指标，但不会影响 `total_found`，后者是实际找到的文档数量。

### 从Manticore 3迁移
- 确保你干净地停止 Manticore 3：
  - `/var/lib/manticore/binlog/` 中不应该有 binlog 文件（目录中只应有 `binlog.meta`）。
  - 否则，Manticore 4 将无法回复 binlogs 的索引将无法运行。
- 新版本可以读取旧的索引，但旧版本无法读取 Manticore 4 的索引，因此确保你做好备份，以便能够轻松回滚到新版本。
- 如果你运行的是复制集群，请确保：
  - 首先干净地停止所有节点。
  - 然后用 `--new-cluster` 启动最后停止的节点（在 Linux 中运行工具 `manticore_new_cluster`）。
  - 阅读有关 [重启集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster) 的更多细节。

### Bug 修复
- 修复了许多复制问题：
  - [Commit 696f](https://github.com/manticoresoftware/manticoresearch/commit/696f8649535f2b5285b878c9581c0d2b1139ae09) - 修复了在有活动索引的加入者上进行 SST 时的崩溃；在写入文件块时在加入者节点添加了 sha1 验证以加快索引加载速度；在索引加载时添加了对已更改索引文件的轮换；当活动索引被捐赠节点的新索引替换时，加入者节点添加了删除索引文件的操作；在捐赠节点添加了发送文件和块的复制日志点。
  - [Commit b296](https://github.com/manticoresoftware/manticoresearch/commit/b296c55af68314a8fc66f8b104e7301d673c3b68) - 如果地址不正确，在 JOIN CLUSTER 时崩溃。
  - [Commit 418b](https://github.com/manticoresoftware/manticoresearch/commit/418bf880a5e05ffbb68f311a1c23006a9df3220e) - 在初始复制大型索引时，加入节点可能会出现 `ERROR 1064 (42000): invalid GTID, (null)` 错误，捐赠节点在另一节点加入时可能变得无响应。
  - [Commit 6fd3](https://github.com/manticoresoftware/manticoresearch/commit/6fd350d28e4905211cf8a7298cbf7cdda101c83d) - 对于大型索引，哈希计算可能出错，从而导致复制失败。
  - [Issue #615](https://github.com/manticoresoftware/manticoresearch/issues/615) - 集群重启时复制失败。
- [Issue #574](https://github.com/manticoresoftware/manticoresearch/issues/574) - `indextool --help` 不显示参数 `--rotate`。
- [Issue #578](https://github.com/manticoresoftware/manticoresearch/issues/578) - searchd 在空闲时的高 CPU 使用率，经过大约一天后出现。
- [Issue #587](https://github.com/manticoresoftware/manticoresearch/issues/587) - 立即刷新 .meta 文件。
- [Issue #617](https://github.com/manticoresoftware/manticoresearch/issues/617) - manticore.json 文件被清空。
- [Issue #618](https://github.com/manticoresoftware/manticoresearch/issues/618) - searchd --stopwait 在以 root 身份运行时失败。它还修复了 systemctl 的行为（之前它对 ExecStop 显示失败，并且没有足够的时间让 searchd 正确停止）。
- [Issue #619](https://github.com/manticoresoftware/manticoresearch/issues/619) - INSERT/REPLACE/DELETE 与 SHOW STATUS 的冲突。`command_insert`、`command_replace` 等显示错误的指标。
- [Issue #620](https://github.com/manticoresoftware/manticoresearch/issues/620) - plain 索引的 `charset_table` 默认值错误。
- [Commit 8f75](https://github.com/manticoresoftware/manticoresearch/commit/8f7536887d4b58a5bab5647fc067b2d0482c2038) - 新的磁盘块未被 mlocked。
- [Issue #607](https://github.com/manticoresoftware/manticoresearch/issues/607) - Manticore 集群节点在无法通过名称解析节点时崩溃。
- [Issue #623](https://github.com/manticoresoftware/manticoresearch/issues/623) - 更新索引的复制可能导致未定义状态。
- [Commit ca03](https://github.com/manticoresoftware/manticoresearch/commit/ca03d2280c5197e9b311f0f03f551e3b702a8130) - ndexer 在索引带有 json 属性的普通索引源时可能会挂起。
- [Commit 53c7](https://github.com/manticoresoftware/manticoresearch/commit/53c753054558ff3f82c7af365273d32537c5338b) - 修复了 PQ 索引中的不等表达式过滤器。
- [Commit ccf9](https://github.com/manticoresoftware/manticoresearch/commit/ccf94e022ba6763d6286ab5bac767b6f6a846846) - 修复了在超过 1000 个匹配时的查询窗口。之前，`SELECT * FROM pq ORDER BY id desc LIMIT 1000, 100 OPTION max_matches=1100` 无法正常工作。
- [Commit a048](https://github.com/manticoresoftware/manticoresearch/commit/a0483fe9bef3c45dffe96f5df4f4382aee29c072) - 对 Manticore 的 HTTPS 请求可能会导致警告，如 "max packet size(8388608) exceeded"。
- [Issue #648](https://github.com/manticoresoftware/manticoresearch/issues/648) - Manticore 3 在多次更新字符串属性后可能会挂起。


## Version 3.6.0, 2021年5月3日
**在Manticore 4之前的维护版本**

### 主要新功能
- 支持 [Manticore Columnar Library](https://github.com/manticoresoftware/columnar/) 的普通索引。新增设置 [columnar_attrs](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#columnar_attrs) 用于普通索引。
- 支持 [乌克兰词形还原器](https://github.com/manticoresoftware/lemmatizer-uk)。https://github.com/manticoresoftware/lemmatizer-uk)
- 完全修订的直方图。在构建索引时，Manticore 还会为每个字段构建直方图，随后用于更快的过滤。在 3.6.0 版本中，该算法经过全面修订，数据量大且过滤频繁的情况下可以获得更高的性能。

### 次要问题
- 工具 `manticore_new_cluster [--force]` 用于通过 systemd 重启复制集群。
- `indexer --merge` 的 [--drop-src](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Merging_tables.md#Merging-tables) 选项。
- [新模式](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) `blend_mode='trim_all'`
- 添加对使用反引号转义 JSON 路径的支持 [Escaping JSON Path](Searching/Full_text_matching/Escaping.md#Escaping-json-node-names-in-SQL)。
- [indextool --check](Miscellaneous_tools.md#indextool) 现在可以在 RT 模式下工作。
- 对于 SELECT/UPDATE 的 [FORCE/IGNORE INDEX(id)](Data_creation_and_modification/Updating_documents/UPDATE.md#Updates-via-SQL) 选项。
- 合并磁盘块的块 ID 现在是唯一的。
- [indextool --check-disk-chunk CHUNK_NAME](Miscellaneous_tools.md#indextool)

### 优化
- [更快的 JSON 解析](https://github.com/manticoresoftware/manticoresearch/commit/0c25fc1788e3f77f60cb7fb8f1623d8466bba606)，我们的测试显示在类似 `WHERE json.a = 1` 的查询中，延迟降低了 3-4%。
- 非文档化命令 `DEBUG SPLIT`，作为自动分片/重平衡的前提。

### Bug 修复
- [Issue #584](https://github.com/manticoresoftware/manticoresearch/issues/524) - FACET 结果不准确且不稳定。
- [Issue #506](https://github.com/manticoresoftware/manticoresearch/issues/506) - 使用 MATCH 时出现奇怪行为；受到此问题影响的用户需要重建索引，因为问题出在构建索引的阶段。
- [Issue #387](https://github.com/manticoresoftware/manticoresearch/issues/387) - 运行包含 SNIPPET() 函数的查询时发生间歇性核心转储。
- 栈优化，改善复杂查询处理：
  - [Issue #469](https://github.com/manticoresoftware/manticoresearch/issues/469) - SELECT 导致崩溃转储。
  - [e8420cc7](https://github.com/manticoresoftware/manticoresearch/tree/e8420cc789149c84731be0874be916e33b263bd6) - 过滤树的栈大小检测。
- [Issue #461](https://github.com/manticoresoftware/manticoresearch/issues/461) - 使用 IN 条件时的 UPDATE 未正确生效。
- [Issue #464](https://github.com/manticoresoftware/manticoresearch/issues/464) - 在调用 PQ 后立即使用 SHOW STATUS 返回。
- [Issue #481](https://github.com/manticoresoftware/manticoresearch/issues/481) - 修复静态二进制构建。
- [Issue #502](https://github.com/manticoresoftware/manticoresearch/issues/502) - 多查询中的错误。
- [Issue #514](https://github.com/manticoresoftware/manticoresearch/issues/514) - 使用 'create table' 时无法使用不寻常的列名。
- [Commit d1db](https://github.com/manticoresoftware/manticoresearch/commit/d1dbe771288328c963ca8512f564c6b7cf4f466c) - 更新字符串属性时，重放 binlog 导致守护进程崩溃；将 binlog 版本设置为 10。
- [Commit 775d](https://github.com/manticoresoftware/manticoresearch/commit/775d0555562f121911d3c30085947e743fadac2d) - 修复合并分组器和分组 N 排序器的平均值；修复聚合的合并。
- [Commit 4795](https://github.com/manticoresoftware/manticoresearch/commit/4795dc49194c7745761bb5006ecb266e8a9224fa) - 针对空存储查询，合并索引过滤器和标签为空。
- [Commit c3f0](https://github.com/manticoresoftware/manticoresearch/commit/c3f0bf4dca0e4eae6ea899dd566878df57eba085) - 网络延迟高且错误率高的情况下，复制 SST 流程中断（不同数据中心复制）；将复制命令版本更新为 1.03。
- [Commit ba2d](https://github.com/manticoresoftware/manticoresearch/commit/ba2d6619b8107f3fb177741bbdff3709bac248cd) - 加入集群时，写操作锁定集群。
- [Commit de4d](https://github.com/manticoresoftware/manticoresearch/commit/de4dcb9ff6c316bc2a9ef6c62eb64769f04ee103) - 精确修饰符的通配符匹配。
- [Commit 6524](https://github.com/manticoresoftware/manticoresearch/commit/6524fc6af032a0932bd42c37874a619e1fb7f075) - 文档 ID 检查点与文档存储。
- [Commit f4ab](https://github.com/manticoresoftware/manticoresearch/commit/f4ab83c291e65a5873d271c67b5491f629544f1d) - 解析无效 XML 时索引器行为不一致。
- [Commit 7b72](https://github.com/manticoresoftware/manticoresearch/commit/7b727e22fe01b1a180d4a8d3a3fea92c18eac707) - 存储的合并查询使用通配符时无限运行。
- [Commit 812d](https://github.com/manticoresoftware/manticoresearch/commit/812dab745f0c972ecde5d5b3d4a313cd6e1e143e) - 以通配符开头的短语权重错误。
- [Commit 1771](https://github.com/manticoresoftware/manticoresearch/commit/1771afc669c75670b3511bf02ab9bfc8982b94d2) - 通配符的存储查询导致匹配中的交错命中并破坏匹配。
- [Commit aa0d](https://github.com/manticoresoftware/manticoresearch/commit/aa0d8c2bcb41428d41dbfb66e42a872a79a4ee4b) - 修复并行查询情况下的 'total' 计算。
- [Commit 18d8](https://github.com/manticoresoftware/manticoresearch/commit/18d81b3cfbb930b7361dae645fe13283d4fbf6ef) - Windows 上的多个并发会话导致守护进程崩溃。
- [Commit 8443](https://github.com/manticoresoftware/manticoresearch/commit/84432f23d6fbf7a7d0b67c8c9412eb5933b03596) - 某些索引设置无法被复制。
- [Commit 9341](https://github.com/manticoresoftware/manticoresearch/commit/93411fe6af4246ca4acce758944d513a1660b92b) - 在高频率添加新事件时，netloop 有时会冻结，因为原子 'kick' 事件被处理多次，导致丢失实际操作。
- [Commit d805](https://github.com/manticoresoftware/manticoresearch/commit/d805fc125244ef08d14fb3d1826e2c356862d539) - 新刷新到磁盘的块可能在提交时丢失。
- [Commit 63cb](https://github.com/manticoresoftware/manticoresearch/commit/63cbf008d50979cdf7062664fb02a672a3bb1131) - profiler 中的 'net_read' 不准确。
- [Commit f537](https://github.com/manticoresoftware/manticoresearch/commit/f5379bb27234b25919966c50ccb6f469ead646e2) - 阿拉伯语（从右到左文本）的 percolate 问题。
- [Commit 49ee](https://github.com/manticoresoftware/manticoresearch/commit/49eeb4202d4ce4875c07c35087d7108c1e177328) - 重复列名时未正确选择 id。
- [Commit refa](https://github.com/manticoresoftware/manticoresearch/commit/4b7df262bbfec9b9e95a44e72ad326ac8a786c49) - 修复在极少数情况下网络事件导致崩溃。
- [e8420cc7](https://github.com/manticoresoftware/manticoresearch/tree/e8420cc789149c84731be0874be916e33b263bd6) - 修复 `indextool --dumpheader`。
- [Commit ff71](https://github.com/manticoresoftware/manticoresearch/commit/ff7163532ed39042c0d3c3c2b902ff41f94d7f0c) - 使用 RECONFIGURE 的 TRUNCATE 在存储字段时表现错误。

### 重大变更：
- 新的 binlog 格式：在升级之前，需要干净地停止 Manticore。
- 索引格式有所变化：新版本可以正常读取现有索引，但如果决定从 3.6.0 降级到旧版本，新的索引将无法读取。
- 复制格式更改：请不要从旧版本复制到 3.6.0，反之亦然，确保所有节点同时切换到新版本。
- `reverse_scan` 已被弃用。确保在查询中不再使用此选项，因为从 3.6.0 开始会导致查询失败。
- 从此版本开始，我们不再提供对 RHEL6、Debian Jessie 和 Ubuntu Trusty 的构建支持。如果您需要这些支持，请[联系我们](https://manticoresearch.com/contact-us/)。

### 弃用
- 不再隐式按 ID 排序。如果依赖此功能，请确保相应地更新查询。
- 搜索选项 `reverse_scan` 已被弃用。

## Version 3.5.4, 2020年12月10日

### 新功能
- 新的 Python、JavaScript 和 Java 客户端现已正式发布，并在本手册中有详细文档。
- 实时索引的磁盘块自动删除。此优化使得在对实时索引进行 [OPTIMIZE](Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE) 时，能够自动删除显然不再需要的磁盘块（所有文档都已被抑制）。之前仍需进行合并，而现在可以直接删除该块。`cutoff` 选项将被忽略，即使没有实际合并，过时的磁盘块也会被移除。这在维护索引中的保留并删除旧文档时非常有用。现在，压缩此类索引的速度将更快。
- 支持将[standalone NOT](Searching/Options.md#not_terms_only_allowed)作为 SELECT 的选项。

### 次要问题
- [Issue #453](https://github.com/manticoresoftware/manticoresearch/issues/453) 新选项 [indexer.ignore_non_plain=1](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#ignore_non_plain) 在您运行 `indexer --all` 且配置文件中不仅包含普通索引时非常有用。若未设置 `ignore_non_plain=1`，将会收到警告并返回相应的退出代码。
- [SHOW PLAN ... OPTION format=dot](Node_info_and_management/Profiling/Query_plan.md#Dot-format-for-SHOW-PLAN) 和 [EXPLAIN QUERY ... OPTION format=dot](Searching/Full_text_matching/Profiling.md#Profiling-without-running-a-query) 允许可视化全文本查询计划执行。这对理解复杂查询非常有帮助。

### 弃用
- `indexer --verbose` 被弃用，因为它从未为 indexer 输出添加任何内容。
- 用于转储 watchdog 的回溯信号，现在使用 `USR2` 代替 `USR1`。

### Bug 修复
- [Issue #423](https://github.com/manticoresoftware/manticoresearch/issues/423) 俄文字符句号在调用片段保留模式时未高亮显示。
- [Issue #435](https://github.com/manticoresoftware/manticoresearch/issues/435) RTINDEX - GROUP N BY 表达式选择导致致命崩溃。
- [Commit 2b3b](https://github.com/manticoresoftware/manticoresearch/commit/2b3b62bd0ce2b54544733d524a169e81f71b5388) 当在集群中时，searchd 状态显示分段错误。
- [Commit 9dd2](https://github.com/manticoresoftware/manticoresearch/commit/9dd25c193ed2a53750a0e0e68566fd2b833fcb0c) 'SHOW INDEX index.N SETTINGS' 不处理大于 9 的块。
- [Issue #389](https://github.com/manticoresoftware/manticoresearch/issues/389) 导致 Manticore 崩溃的错误。
- [Commit fba1](https://github.com/manticoresoftware/manticoresearch/commit/fba166178ff35c73a0c70d131cf3aca4a635e961) 转换器创建损坏的索引。
- [Commit eecd](https://github.com/manticoresoftware/manticoresearch/commit/eecd61d861c742471175be19a5e5455034b5db5f) stopword_step=0 与调用 SNIPPETS() 的冲突。
- [Commit ea68](https://github.com/manticoresoftware/manticoresearch/commit/ea6850e449601199190f38a387c04b8a7b7e90f6) 在本地索引的低 max_matches 时，count distinct 返回 0。
- [Commit 362f](https://github.com/manticoresoftware/manticoresearch/commit/362f27db3421f09a41c6cce04b5baace00fa4c4d) 使用聚合时，未在命中中返回存储的文本。


## Version 3.5.2, 2020年10月1日

### 新功能

* **OPTIMIZE** 现在会将磁盘块减少到一定数量的块（默认是 `2* CPU 核心数量`），而不是单一块。优化块的数量可以通过 [cutoff](Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks) 选项进行控制。
* **NOT** 操作符现在可以独立使用。默认情况下，它是禁用的，因为意外的单个 NOT 查询可能会很慢。可以通过将新的 searchd 指令 [not_terms_only_allowed](Server_settings/Searchd.md#not_terms_only_allowed) 设置为 `0` 来启用。
* 新设置 [max_threads_per_query](Server_settings/Searchd.md#max_threads_per_query) 设置查询可以使用的线程数量。如果未设置该指令，则查询可以使用的线程数最大为 [threads](Server_settings/Searchd.md#threads) 的值。每个 `SELECT` 查询的线程数量可以通过 [OPTION threads=N](Searching/Options.md#threads) 限制，从而覆盖全局的 `max_threads_per_query`。
* 现在可以通过 [IMPORT TABLE](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Importing_table.md) 导入渗透索引。
* HTTP API `/search` 收到了对 [faceting](Searching/Faceted_search.md#HTTP-JSON)/[grouping](Searching/Grouping.md) 的基本支持，通过新查询节点 `aggs` 实现。

### 次要问题

* 如果未声明复制监听指令，系统将尝试使用定义的 'sphinx' 端口之后的端口，最多到 200。
* 对于 SphinxSE 连接或 SphinxAPI 客户端，需要显式设置 `listen=...:sphinx`。
* **SHOW INDEX STATUS** 输出新指标：`killed_documents`、`killed_rate`、`disk_mapped_doclists`、`disk_mapped_cached_doclists`、`disk_mapped_hitlists` 和 `disk_mapped_cached_hitlists`。
* SQL 命令 `status` 现在输出 `Queue\Threads` 和 `Tasks\Threads`。

### 弃用

* `dist_threads` 现在完全弃用，如果仍然使用该指令，searchd 将记录警告。

### Docker

官方 Docker 镜像现在基于 Ubuntu 20.04 LTS。

### 软件包

除了常规的 `manticore` 包外，您还可以按组件安装 Manticore Search：

- **`manticore-server-core`** - 提供 `searchd`、手册页、日志目录、API 和 Galera 模块。它还会安装 `manticore-common` 作为依赖项。
- **`manticore-server`** - 提供核心的自动化脚本（init.d、systemd）以及 `manticore_new_cluster` 包装器。它还会安装 `manticore-server-core` 作为依赖项。
- **`manticore-common`** - 提供配置、停用词、通用文档和骨架文件夹（数据目录、模块等）。
- **`manticore-tools`** - 提供辅助工具（`indexer`、`indextool` 等）、它们的手册页和示例。它还会安装 `manticore-common` 作为依赖项。
- **`manticore-icudata`** (RPM) 或 **`manticore-icudata-65l`** (DEB) - 提供用于 ICU 词法分析的 ICU 数据文件。
- **`manticore-devel`** (RPM) 或 **`manticore-dev`** (DEB) - 提供 UDF 的开发头文件。

### Bug 修复

1. [Commit 2a47](https://github.com/manticoresoftware/manticoresearch/commit/2a474dc1a26e8b0f8aaaae95669caf2f1d4b7746) - RT 索引在不同块上的分组时导致守护进程崩溃
1. [Commit 57a1](https://github.com/manticoresoftware/manticoresearch/commit/57a19e5ad5663ef0ca7436595218fb1221d28c8e) - 针对空远程文档的快速路径
1. [Commit 07dd](https://github.com/manticoresoftware/manticoresearch/commit/07dd3f313c63fb82c22092f9907ef24e3475250e) - 表达式栈帧检测运行时
1. [Commit 08ae](https://github.com/manticoresoftware/manticoresearch/commit/08ae357cf1012bc8e2da54c20b205b592efda3d4) - 在渗透索引中匹配超过 32 个字段
1. [Commit 16b9](https://github.com/manticoresoftware/manticoresearch/commit/16b9390fd4cdb07a77ac4497adb935573a1710e5) - 复制监听端口范围
1. [Commit 5fa6](https://github.com/manticoresoftware/manticoresearch/commit/5fa671affeacb6441a59c8a88479bfd423df7c81) - 在 PQ 上显示创建表
1. [Commit 54d1](https://github.com/manticoresoftware/manticoresearch/commit/54d133b6449105a9fb0168db3f1fbb05fb5aa1f6) - HTTPS 端口行为
1. [Commit fdbb](https://github.com/manticoresoftware/manticoresearch/commit/fdbbe5245cc296cc5c1ae3ae2fb9cb08fb66a248) - 替换时混合文档存储行
1. [Commit afb5](https://github.com/manticoresoftware/manticoresearch/commit/afb53f648ef4e64fa1776c58e66f6e716ac730ad) - 将 TFO 不可用消息级别更改为 'info'
1. [Commit 59d9](https://github.com/manticoresoftware/manticoresearch/commit/59d94cefc6e88af360b8046726a16b7eaa9f0b71) - 在 strcmp 中使用无效时崩溃
1. [Commit 04af](https://github.com/manticoresoftware/manticoresearch/commit/04af0349c1e784c2230d204592d6272b440bb375) - 向集群添加索引时带有系统（停用词）文件
1. [Commit 5014](https://github.com/manticoresoftware/manticoresearch/commit/50148b4eae9f3a924082005cf5d2f9c8339af7f5) - 合并具有大字典的索引；对大磁盘块的实时优化
1. [Commit a2ad](https://github.com/manticoresoftware/manticoresearch/commit/a2adf1582843f63d4dcc1dab8f7598e0917094a2) - Indextool 可以从当前版本转储元数据
1. [Commit 69f6](https://github.com/manticoresoftware/manticoresearch/commit/69f6d5f77c64b97e3f395aea8c555b11c77fb732) - 在 GROUP N 中的分组顺序问题
1. [Commit 24d5](https://github.com/manticoresoftware/manticoresearch/commit/24d5d80f508576794b751d70f9f9b524564e3c14) - 握手后 SphinxSE 的显式刷新
1. [Commit 31c4](https://github.com/manticoresoftware/manticoresearch/commit/31c4d78ac03ee85d9a02b86075448a04ac80f9e2) - 避免不必要地复制巨大的描述
1. [Commit 2959](https://github.com/manticoresoftware/manticoresearch/commit/2959e2caa8610f98fcdf2e5a82160f58687f4a27) - 在显示线程中出现负时间
1. [Commit f0b3](https://github.com/manticoresoftware/manticoresearch/commit/f0b357102c70f3ac1d7da260c60b67322940e647) - 令牌过滤插件与零位置增量
1. [Commit a49e](https://github.com/manticoresoftware/manticoresearch/commit/a49e5bc13ea763e6f01dd5993e891858a84d1846) - 将 'FAIL' 更改为 'WARNING' 以处理多个命中

## Version 3.5.0, 2020年7月22日

### 主要新功能
* 此版本的发布耗时较长，因为我们在努力将多任务模式从线程转换为 **协程**。这使配置更简单，查询并行化更加直接：Manticore 只使用给定数量的线程（请参阅新设置 [threads](Server_settings/Searchd.md#threads)），新的模式确保以最优方式完成。

* [高亮显示](Searching/Highlighting.md#Highlighting-options)的变化：
  - 所有与多个字段一起工作的高亮显示（如 `highlight({},'field1, field2'` 或 JSON 查询中的 `highlight`）现在默认按字段应用限制。
  - 所有与纯文本一起工作的高亮显示（如 `highlight({}, string_attr)` 或 `snippet()`）现在按整个文档应用限制。
  - [每字段限制](Searching/Highlighting.md#limits_per_field)可以通过 `limits_per_field=0` 选项切换为全局限制（默认为 `1`）。
  - 对于通过 HTTP JSON 进行高亮显示，[allow_empty](Searching/Highlighting.md#allow_empty) 现在默认值为 `0`。

* 现在可以在同一端口上 [同时使用](Server_settings/Searchd.md#listen) HTTP、HTTPS 和二进制 API（以接受来自远程 Manticore 实例的连接）。仍然需要 `listen = *:mysql` 以通过 MySQL 协议进行连接。Manticore 现在自动检测尝试连接的客户端类型，MySQL 协议除外（由于协议的限制）。

* 在实时模式下，一个字段现在可以同时是 [文本和字符串属性](Creating_a_table/Data_types.md#String) - [GitHub Issue #331](https://github.com/manticoresoftware/manticoresearch/issues/331)。

  在 [纯模式](Read_this_first.md#Real-time-mode-vs-plain-mode) 中称为 `sql_field_string`。现在它也可以在 [实时模式](Read_this_first.md#Real-time-mode-vs-plain-mode) 中用于实时索引。您可以按照示例使用它：

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

### 次要问题
* 现在您可以 [高亮显示字符串属性](Searching/Highlighting.md#Highlighting-via-SQL)。
* SQL 接口支持 SSL 和压缩。
* 支持 MySQL 客户端 [`status`](Node_info_and_management/Node_status.md#STATUS) 命令。
* [复制](Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Setting-up-replication)现在可以复制外部文件（停用词、例外等）。
* 过滤操作符 [`in`](Searching/Filters.md#Set-filters) 现在通过 HTTP JSON 接口可用。
* [`expressions`](Searching/Expressions.md#expressions) 现在在 HTTP JSON 中可用。
* 现在您可以在实时模式下动态更改 `rt_mem_limit`，即可以执行 `ALTER ... rt_mem_limit=<新值>`。
* 现在您可以使用 [独立的 CJK 字符集表](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology)：`chinese`、`japanese` 和 `korean`。
* [`thread_stack`](Server_settings/Searchd.md#thread_stack) 现在限制最大线程堆栈，而不是初始堆栈。
* 改进的 `SHOW THREADS` 输出。
* 在 `SHOW THREADS` 中显示长时间 `CALL PQ` 的进度。
* cpustat、iostat、coredump 可以在运行时通过 [SET](Server_settings/Setting_variables_online.md#SET) 进行更改。
* 实现了 `SET [GLOBAL] wait_timeout=NUM`。

### 重大变更:
* **索引格式已更改。** 在 3.5.0 中构建的索引无法被低于 3.5.0 的 Manticore 版本加载，但 Manticore 3.5.0 可以理解较旧的格式。
* [`INSERT INTO PQ VALUES()`](Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md)（即不提供列列表）之前只接受 `(query, tags)` 作为值。现已更改为 `(id, query, tags, filters)`。如果您希望自动生成 id，可以将其设置为 0。
* 在通过 HTTP JSON 接口进行高亮显示时，[`allow_empty=0`](Searching/Highlighting.md#allow_empty) 是新的默认值。
* 在 `CREATE TABLE`/`ALTER TABLE` 中，外部文件（停用词、例外等）仅允许使用绝对路径。

### 弃用：
* `ram_chunks_count` 在 `SHOW INDEX STATUS` 中重命名为 `ram_chunk_segments_count`。

* `workers` 已废弃。现在只有一种工作模式。

* `dist_threads` 已废弃。所有查询现在尽可能并行（受 `threads` 和 `jobs_queue_size` 限制）。

* `max_children` 已废弃。使用 [threads](Server_settings/Searchd.md#threads) 设置 Manticore 将使用的线程数（默认设置为 CPU 核心数）。

* `queue_max_length` 已废弃。如果确实需要，可以使用 [jobs_queue_size](Server_settings/Searchd.md#jobs_queue_size) 来微调内部作业队列大小（默认无限制）。

* 所有 `/json/*` 端点现在可以在不加 `/json/` 的情况下访问，例如 `/search`、`/insert`、`/delete`、`/pq` 等。

* 在 `describe` 中，“field”意为“全文字段”，已重命名为“text”。

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

* Cyrillic 字母 `и` 在 `non_cjk` charset_table（默认值）中不映射为 `i`，因为这对俄语词干提取器和词形还原器的影响过大。

* `read_timeout` 已被弃用。请使用 [network_timeout](Server_settings/Searchd.md#network_timeout)，它同时控制读取和写入的超时。


### 软件包

* 提供 Ubuntu Focal 20.04 的官方软件包。
* deb 软件包名称已从 `manticore-bin` 更改为 `manticore`。

### Bug 修复
1. [Issue #351](https://github.com/manticoresoftware/manticoresearch/issues/351) - searchd 内存泄漏
2. [Commit ceab](https://github.com/manticoresoftware/manticoresearch/commit/ceabe44f) - snippet 中的微小越界读取
3. [Commit 1c3e](https://github.com/manticoresoftware/manticoresearch/commit/1c3e84a3) - 危险的写入本地变量导致崩溃查询
4. [Commit 26e0](https://github.com/manticoresoftware/manticoresearch/commit/26e094ab) - 测试 226 中排序器的微小内存泄漏
5. [Commit d2c7](https://github.com/manticoresoftware/manticoresearch/commit/d2c7f86a) - 测试 226 中的巨大内存泄漏
6. [Commit 0dd8](https://github.com/manticoresoftware/manticoresearch/commit/0dd80122) - 集群显示节点同步，但 `count(*)` 显示不同的数字
7. [Commit f1c1](https://github.com/manticoresoftware/manticoresearch/commit/f1c1ac3f) - 美化：日志中重复和有时丢失的警告信息
8. [Commit f1c1](https://github.com/manticoresoftware/manticoresearch/commit/f1c1ac3f) - 美化：日志中 (null) 索引名称
9. [Commit 359d](https://github.com/manticoresoftware/manticoresearch/commit/359dbd30) - 无法检索超过 70M 的结果
10. [Commit 19f3](https://github.com/manticoresoftware/manticoresearch/commit/19f328ee) - 不能使用无列语法插入 PQ 规则
11. [Commit bf68](https://github.com/manticoresoftware/manticoresearch/commit/bf685d5d) - 向集群中索引插入文档时的误导性错误消息
12. [Commit 2cf1](https://github.com/manticoresoftware/manticoresearch/commit/2cf18c83) - `/json/replace` 和 `json/update` 返回指数形式的 id
13. [Issue #324](https://github.com/manticoresoftware/manticoresearch/issues/324) - 在同一查询中更新 JSON 标量属性和 MVA
14. [Commit d384](https://github.com/manticoresoftware/manticoresearch/commit/d38409eb) - `hitless_words` 在 RT 模式下不起作用
15. [Commit 5813](https://github.com/manticoresoftware/manticoresearch/commit/5813d639) - RT 模式下应禁止使用 `ALTER RECONFIGURE`
16. [Commit 5813](https://github.com/manticoresoftware/manticoresearch/commit/5813d639) - searchd 重启后 `rt_mem_limit` 被重置为 128M
17. highlight() 有时会挂起
18. [Commit 7cd8](https://github.com/manticoresoftware/manticoresearch/commit/7cd878f4) - RT 模式下无法使用 U+ 代码
19. [Commit 2b21](https://github.com/manticoresoftware/manticoresearch/commit/2b213de4) - RT 模式下无法在词形中使用通配符
20. [Commit e9d0](https://github.com/manticoresoftware/manticoresearch/commit/e9d07e68) - 修复 `SHOW CREATE TABLE` 与多个词形文件之间的问题
21. [Commit fc90](https://github.com/manticoresoftware/manticoresearch/commit/fc90a84f) - JSON 查询没有 "query" 时导致 searchd 崩溃
22. Manticore [官方 Docker](https://hub.docker.com/r/manticoresearch/manticore) 无法从 MySQL 8 索引
23. [Commit 23e0](https://github.com/manticoresoftware/manticoresearch/commit/23e05d32) - HTTP /json/insert 需要 id
24. [Commit bd67](https://github.com/manticoresoftware/manticoresearch/commit/bd679af0) - `SHOW CREATE TABLE` 不适用于 PQ
25. [Commit bd67](https://github.com/manticoresoftware/manticoresearch/commit/bd679af0) - `CREATE TABLE LIKE` 对 PQ 的处理不正确
26. [Commit 5eac](https://github.com/manticoresoftware/manticoresearch/commit/5eacf28f) - 在 `SHOW INDEX STATUS` 的设置中出现换行
27. [Commit cb15](https://github.com/manticoresoftware/manticoresearch/commit/cb153228) - HTTP JSON 响应中的 "highlight" 空标题
28. [Issue #318](https://github.com/manticoresoftware/manticoresearch/issues/318) - `CREATE TABLE LIKE` 的中缀错误
29. [Commit 9040](https://github.com/manticoresoftware/manticoresearch/commit/9040d22c) - RT 在负载下崩溃
30. [Commit cd512c7d](https://github.com/manticoresoftware/manticoresearch/commit/cd512c7d) - 在 RT 磁盘块崩溃时丢失崩溃日志
31. [Issue #323](https://github.com/manticoresoftware/manticoresearch/issues/323) - 导入表失败并关闭连接
32. [Commit 6275](https://github.com/manticoresoftware/manticoresearch/commit/6275316a) - `ALTER reconfigure` 损坏了 PQ 索引
33. [Commit 9c1d](https://github.com/manticoresoftware/manticoresearch/commit/9c1d221e) - 更改索引类型后 searchd 重载问题
34. [Commit 71e2](https://github.com/manticoresoftware/manticoresearch/commit/71e2b5bb) - 导入表时如果缺少文件则守护进程崩溃
35. [Issue #322](https://github.com/manticoresoftware/manticoresearch/issues/322) - 使用多个索引、分组和 ranker = none 时崩溃
36. [Commit c3f5](https://github.com/manticoresoftware/manticoresearch/commit/c3f58490) - `HIGHLIGHT()` 在字符串属性中不高亮
37. [Issue #320](https://github.com/manticoresoftware/manticoresearch/issues/320) - `FACET` 在字符串属性上排序失败
38. [Commit 4f1a](https://github.com/manticoresoftware/manticoresearch/commit/4f1a1f25) - 缺少数据目录时出现错误
39. [Commit 04f4](https://github.com/manticoresoftware/manticoresearch/commit/04f4ddd4) - 在 RT 模式下不支持 access_*
40. [Commit 1c06](https://github.com/manticoresoftware/manticoresearch/commit/1c0616a2) - 字符串中的坏 JSON 对象：1. `CALL PQ` 返回 "字符串中的坏 JSON 对象：1" 当 JSON 超过某个值时。
41. [Commit 32f9](https://github.com/manticoresoftware/manticoresearch/commit/32f943d6) - RT 模式不一致。在某些情况下，我无法删除索引，因为它未知，同时也无法创建，因为目录不为空。
42. [Issue #319](https://github.com/manticoresoftware/manticoresearch/issues/319) - 查询时崩溃
43. [Commit 22a2](https://github.com/manticoresoftware/manticoresearch/commit/22a28dd7) - `max_xmlpipe2_field` = 2M 返回了 2M 字段的警告
44. [Issue #342](https://github.com/manticoresoftware/manticoresearch/issues/342) - 查询条件执行错误
45. [Commit dd8d](https://github.com/manticoresoftware/manticoresearch/commit/dd8dcab2) - 简单的 2 个术语搜索找到仅包含一个术语的文档
46. [Commit 9091](https://github.com/manticoresoftware/manticoresearch/commit/90919e62) - 在 PQ 中无法匹配具有大写字母键的 JSON
47. [Commit 56da](https://github.com/manticoresoftware/manticoresearch/commit/56da086a) - indexer 在 csv+docstore 上崩溃
48. [Issue #363](https://github.com/manticoresoftware/manticoresearch/issues/363) - 在 CentOS 7 中使用 `[null]` 的 JSON 属性导致插入数据损坏
49. 主要 [Issue #345](https://github.com/manticoresoftware/manticoresearch/issues/345) - 记录未插入，`count()` 随机，`replace into` 返回 OK
50. `max_query_time` 使 SELECT 查询变得过慢
51. [Issue #352](https://github.com/manticoresoftware/manticoresearch/issues/352) - Master-agent 通信在 Mac OS 上失败
52. [Issue #328](https://github.com/manticoresoftware/manticoresearch/issues/328) - 使用 Connector.Net/Mysql 8.0.19 连接 Manticore 时出错
53. [Commit daa7](https://github.com/manticoresoftware/manticoresearch/commit/daa760d2fe83668038318f0f384ceefc0d075e18) - 修复了 \0 的转义并优化了性能
54. [Commit 9bc5](https://github.com/manticoresoftware/manticoresearch/commit/9bc5c01a493db993b90ff6c1ad51aeae028f629c) - 修复了 `count distinct` 与 JSON 的问题
55. [Commit 4f89](https://github.com/manticoresoftware/manticoresearch/commit/4f89a965a3af0db548ae9d1763731ebaebe21f5a) - 修复了在其他节点上删除表失败的问题
56. [Commit 952a](https://github.com/manticoresoftware/manticoresearch/commit/952af5a5931fd72b87edc169588777778b365231) - 修复了在快速运行 `call pq` 时的崩溃问题


## Version 3.4.2, 2020年4月10日
### 关键bug修复
* [Commit 2ffe](https://github.com/manticoresoftware/manticoresearch/commit/2ffe2d261793f9681265bc4abe38d695e72c121e) 修复了旧版本的 RT 索引无法索引数据的问题。

## Version 3.4.0, 26 2020年3月26日
### 主要变更
* 服务器支持两种模式：实时模式（rt-mode）和普通模式（plain-mode）
   *   实时模式需要 `data_dir`，且配置中不允许定义索引
   *   在普通模式下，索引在配置中定义；不允许使用 `data_dir`
* 复制功能仅在实时模式下可用

### 次要问题
* `charset_table` 默认设置为 `non_cjk` 别名
* 在实时模式下，全文字段默认被索引和存储
* 实时模式下的全文字段名称由“field”更改为“text”
* `ALTER RTINDEX`更名为 `ALTER TABLE`
* `TRUNCATE RTINDEX` 更名为 `TRUNCATE TABLE`

### Features
* 存储仅字段
* SHOW CREATE TABLE, IMPORT TABLE

### 改进
* 无锁 PQ 性能显著提高
* `/sql` 可以在 `mode=raw` 下执行任何类型的 SQL 语句
* 为 mysql41 协议提供别名 mysql
* 默认的 `state.sql` 文件位于 `data_dir` 中

### Bugfixes
* [Commit a533](https://github.com/manticoresoftware/manticoresearch/commit/a5333644e8e0c69746a9f24da8221fc965478e4a) 修复在 `highlight()` 中错误字段语法导致的崩溃
* [Commit 7fbb](https://github.com/manticoresoftware/manticoresearch/commit/7fbb9f2e618096289cba7f8b834530b46aa94055) 修复在复制带有文档存储的实时索引时服务器崩溃
* [Commit 24a0](https://github.com/manticoresoftware/manticoresearch/commit/24a04687eb43b4f4719c7c4a5fe3c19099240467) 修复在使用中缀或前缀选项的索引中进行高亮时的崩溃，或在未启用存储字段的情况下进行高亮
* [Commit 3465](https://github.com/manticoresoftware/manticoresearch/commit/3465c1ce9c860c09f6ccc5b83a63e3c68c241d8d) 修复关于空文档存储和空索引的文档 ID 查找的错误
* [Commit a707](https://github.com/manticoresoftware/manticoresearch/commit/b8601b4144b9092be43a7caa7cd6b19cb37e14c7) 修复 #314 SQL 插入命令后带分号
* [Commit 9562](https://github.com/manticoresoftware/manticoresearch/commit/95628c9b9b8f16fe1bd18599a84e9673575adf1a) 移除关于查询单词不匹配的警告
* [Commit b860](https://github.com/manticoresoftware/manticoresearch/commit/b8601b4144b9092be43a7caa7cd6b19cb37e14c7) 修复通过 ICU 分段的片段查询
* [Commit 5275](https://github.com/manticoresoftware/manticoresearch/commit/5275516ca4631c584e6e0127fa161a18d2658fd0) 修复文档存储块缓存中的查找/添加竞争条件
* [Commit f06e](https://github.com/manticoresoftware/manticoresearch/commit/f06ef97af5c7390427e82525c150535290e42c3e) 修复文档存储中的内存泄漏
* [Commit a725](https://github.com/manticoresoftware/manticoresearch/commit/a7258ba8340e246c0d8aaeb4a7a74129eba807eb) 修复 #316 `LAST_INSERT_ID` 在插入时返回空值
* [Commit 1ebd](https://github.com/manticoresoftware/manticoresearch/commit/1ebd5bf8d74113bcfa138ce63ce11966990a4215) 修复 #317 `json/update` HTTP 端点支持 MVA 的数组和 JSON 属性的对象
* [Commit e426](https://github.com/manticoresoftware/manticoresearch/commit/e426950a536382a142e2a42ed207c341149ff685) 修复索引器在没有显式 ID 的情况下转储实时索引时崩溃

## Version 3.3.0, 2020年2月4日
### Features
* 支持并行实时索引搜索
* EXPLAIN QUERY 命令
* 无索引定义的配置文件（测试版）
* CREATE/DROP TABLE 命令（测试版）
* `indexer --print-rt` - 可以从源读取并为实时索引打印 INSERT 语句

### 改进
* 更新到 Snowball 2.0 词干提取器
* SHOW INDEX STATUS 中的 LIKE 过滤器
* 针对高 `max_matches` 的内存使用优化
* SHOW INDEX STATUS 添加了实时索引的 `ram_chunks_count`
* 无锁 PQ
* 将 `LimitNOFILE` 更改为 65536


### Bug 修复
* [Commit 9c33](https://github.com/manticoresoftware/manticoresearch/commit/9c33aab8c1fd9059c8644d23d5f58f03010043c6) 添加检查索引模式以防止重复属性 #293
* [Commit a008](https://github.com/manticoresoftware/manticoresearch/commit/a0085f9477e191da9e0e835b91583fb9cfde698e) 修复在无命中词的情况下崩溃
* [Commit 6895](https://github.com/manticoresoftware/manticoresearch/commit/68953740af8dca1833026d0f448c71416ed3044a) 修复 ATTACH 后丢失文档存储
* [Commit d6f6](https://github.com/manticoresoftware/manticoresearch/commit/d6f696edea53ea2b9a68251e4054e4816082c285) 修复分布式设置中的文档存储问题
* [Commit bce2](https://github.com/manticoresoftware/manticoresearch/commit/bce2b7ec883d208f31094ee42a5b6f01d18a998f) 在排序器中用 OpenHash 替换 FixedHash
* [Commit e0ba](https://github.com/manticoresoftware/manticoresearch/commit/e0baf7392937969e640fb5d10e8ccb312fe36446) 修复索引定义中重复名称的属性
* [Commit ca81](https://github.com/manticoresoftware/manticoresearch/commit/ca81114bf1ac7ea7adcc8509bdc14d1f30b4f461) 修复 HIGHLIGHT() 中的 html_strip
* [Commit 493a](https://github.com/manticoresoftware/manticoresearch/commit/493a5e916d70fd4a3f14d1ee487234434f4a7ce3) 修复 HIGHLIGHT() 中的 passage 宏
* [Commit a82d](https://github.com/manticoresoftware/manticoresearch/commit/a82d41c775692f837f267d554d7d4a4b499d69f0) 修复实时索引创建小或大磁盘块时的双缓冲问题
* [Commit a404](https://github.com/manticoresoftware/manticoresearch/commit/a404c85de0d41d9d1c75b7540479d8a3af9e61aa) 修复 kqueue 中的事件删除
* [Commit 8bea](https://github.com/manticoresoftware/manticoresearch/commit/8bea0f6fd1666b817995c4dd6e67f6f1c9161638) 修复实时索引的 `rt_mem_limit` 大值下磁盘块保存
* [Commit 8707](https://github.com/manticoresoftware/manticoresearch/commit/8707f0395b7ace52dc0cba9c8ed92a2bc4392932) 修复索引时的浮点溢出
* [Commit a564](https://github.com/manticoresoftware/manticoresearch/commit/a56434ce71bb7714b4d8e75278aaa075b27ece80) 修复向实时索引插入负 ID 的文档失败
* [Commit bbeb](https://github.com/manticoresoftware/manticoresearch/commit/bbebfd7551a68641154761cac962053b54806759) 修复在 ranker 字段掩码上服务器崩溃
* [Commit 3809](https://github.com/manticoresoftware/manticoresearch/commit/3809cc1b0863e3a5a200dc423be3f7c38be56113) 修复使用查询缓存时崩溃
* [Commit dc2a](https://github.com/manticoresoftware/manticoresearch/commit/dc2a585bc724678679e4297362200bbd49ce32bb) 修复使用并行插入的实时索引 RAM 段时崩溃

## Version 3.2.2, 2019年12月19日
### Features
* 实时索引的自增 ID
* 通过新的 HIGHLIGHT() 函数支持文档存储的高亮，HTTP API 中也可用
* SNIPPET() 可以使用特殊函数 QUERY()，返回当前的 MATCH 查询
* 新的字段分隔符选项用于高亮函数

### 改进与变更
* 对远程节点的存储字段进行懒加载（可以显著提高性能）
* 字符串和表达式不再破坏多查询和 FACET 优化
* RHEL/CentOS 8 构建现在使用 mariadb-connector-c-devel 的 mysql libclient
* ICU 数据文件现在随软件包提供，移除了 icu_data_dir
* systemd 服务文件包含 'Restart=on-failure' 策略
* indextool 现在可以在线检查实时索引
* 默认配置现在为 /etc/manticoresearch/manticore.conf
* RHEL/CentOS 上的服务名称从 'searchd' 更改为 'manticore'
* 移除了查询模式和精确短语片段的选项

### Bug 修复
* [Commit 6ae4](https://github.com/manticoresoftware/manticoresearch/commit/6ae474c7894a6bee222d5b18e59a44fdbf57843a) 修复通过 HTTP 接口的 SELECT 查询崩溃
* [Commit 5957](https://github.com/manticoresoftware/manticoresearch/commit/59577513a49eac5a4a3c5e2cb38394d3246b5d35) 修复实时索引保存磁盘块但未标记某些文档已删除
* [Commit e861](https://github.com/manticoresoftware/manticoresearch/commit/e861f0fca0e88924450695d4e9d6acff7a36558a) 修复多索引或多查询与 dist_threads 的搜索崩溃
* [Commit 4409](https://github.com/manticoresoftware/manticoresearch/commit/440991fc977b8479800b45cdbc862eeb1ba5d965) 修复长术语生成中间接缝时的崩溃
* [Commit 5fd5](https://github.com/manticoresoftware/manticoresearch/commit/5fd599b48bba527c023e6aa0b262dca51ffb8a1c) 修复在添加套接字到 IOCP 时的竞争条件
* [Commit cf10](https://github.com/manticoresoftware/manticoresearch/commit/cf10d7d3589f77adfaefbe52a51777bc9c67cf99) 修复布尔查询与 JSON 选择列表的问题
* [Commit 996d](https://github.com/manticoresoftware/manticoresearch/commit/996de77f4c3ec103d965e28c36098fd07dba50c4) 修复 indextool 检查报告错误的跳过列表偏移，检查 doc2row 查找
* [Commit 6e3f](https://github.com/manticoresoftware/manticoresearch/commit/6e3fc9e88941c9427410f9d0adeebb7dd09a900f) 修复索引器在大数据时产生不良索引的负跳过列表偏移
* [Commit faed](https://github.com/manticoresoftware/manticoresearch/commit/faed3220b423a96401982bf47117edf1f62c584e) 修复 JSON 只将数字转换为字符串，并在表达式中将 JSON 字符串转换为数字
* [Commit 5331](https://github.com/manticoresoftware/manticoresearch/commit/533197200249d648ebcbdaca5d605861ee566348) 修复 indextool 在命令行设置多个命令时退出并返回错误代码
* [Commit 7955](https://github.com/manticoresoftware/manticoresearch/commit/795520ac351d22e4497ebaf6c249bf25081842b6) 修复 #275 在没有剩余空间时 binlog 状态无效
* [Commit 2284](https://github.com/manticoresoftware/manticoresearch/commit/2284da5e3ff77bf7be4038a4d950c0580e4159c8) 修复 #279 在 JSON 属性中对 IN 过滤器的崩溃
* [Commit ce2e](https://github.com/manticoresoftware/manticoresearch/commit/ce2e4b4765111a7d331ffc256911f12770a8942d) 修复 #281 错误的管道关闭调用
* [Commit 5355](https://github.com/manticoresoftware/manticoresearch/commit/535589ba8a37baf5c6056afb5ff969fd70476feb) 修复在处理递归 JSON 属性编码为字符串时服务器挂起
* [Commit a5fc](https://github.com/manticoresoftware/manticoresearch/commit/a5fc8a36e1644ee051632408ab8b53e79aeb9ff4) 修复在 multiand 节点中超出文档列表末尾的问题
* [Commit a362](https://github.com/manticoresoftware/manticoresearch/commit/a3628617ef4ee89a811d4ba2c4a04cbbc298c1b5) 修复线程公共信息的检索
* [Commit f8d2](https://github.com/manticoresoftware/manticoresearch/commit/f8d2d7bbbb90af3d2ee97424f97b05927f09d42b) 修复文档存储缓存锁问题

## Version 3.2.0, 2019年10月17日
### Features
* 文档存储
* 新指令：stored_fields、docstore_cache_size、docstore_block_size、docstore_compression、docstore_compression_level

### 改进与变更
* 改进了 SSL 支持
* 更新了 non_cjk 内置字符集
* 禁用在查询日志中记录 SELECT 的 UPDATE/DELETE 语句
* 提供 RHEL/CentOS 8 软件包

### Bug 修复
* [Commit 301a](https://github.com/manticoresoftware/manticoresearch/commit/301a806b14725e842300ee225521407464374c41) 修复在 RT 索引的磁盘块中替换文档时崩溃
* [Commit 46c1](https://github.com/manticoresoftware/manticoresearch/commit/46c1cad8fa11c1fc09cc34b20d978fdfcd4e6d27) 修复 #269 中的 LIMIT N OFFSET M
* [Commit 92a4](https://github.com/manticoresoftware/manticoresearch/commit/92a46edaa196ef33f3ac2225fbdba2c2b88ee45d) 修复显式设置 id 或提供 id 列表以跳过搜索的 DELETE 语句
* [Commit 8ca7](https://github.com/manticoresoftware/manticoresearch/commit/8ca78c138d837caab30dc6e2343a3c4d1687bb87) 修复在 windows poller 的 netloop 中移除事件后索引错误
* [Commit 6036](https://github.com/manticoresoftware/manticoresearch/commit/603631e2bff5cb0acf25f68b85621742490eccd9) 修复通过 HTTP 进行 JSON 的浮点数四舍五入
* [Commit 62f6](https://github.com/manticoresoftware/manticoresearch/commit/62f64cb9e75ec28fc3c29f8414ce9656f5580254) 修复远程片段，先检查空路径；修复 Windows 测试
* [Commit aba2](https://github.com/manticoresoftware/manticoresearch/commit/aba274c2c5abc1029894ba9f95b786bda93e8f22) 修复配置重新加载，使其在 Windows 上的行为与 Linux 一致
* [Commit 6b8c](https://github.com/manticoresoftware/manticoresearch/commit/6b8c4242ef77c3fa4d0ccb7d76d81714b6728f0b) 修复 #194 中的 PQ，使其可以与词法分析和词干提取器一起使用
* [Commit 174d](https://github.com/manticoresoftware/manticoresearch/commit/174d312905285aa5746f4f404c53e6dc669ef42f) 修复实时已退休段的管理

## Version 3.1.2, 2019年8月22日
### 功能与改进
* 实验性的 HTTP API SSL 支持
* CALL KEYWORDS 的字段过滤
* /json/search 的 max_matches
* 默认 Galera gcache.size 的自动调整
* 改进了对 FreeBSD 的支持

### Bug 修复
* [Commit 0a1a](https://github.com/manticoresoftware/manticoresearch/commit/0a1a2c81038794983b4e30c8aefc7cc20e89b03f) 修复将 RT 索引复制到同一路径下存在相同 RT 索引的节点时的问题
* [Commit 4adc](https://github.com/manticoresoftware/manticoresearch/commit/4adc075294ac823289f745e2cc419f18c7dcf2e2) 修复对没有活动的索引进行刷新重新调度的问题
* [Commit d6c0](https://github.com/manticoresoftware/manticoresearch/commit/d6c00a6f750c04021747e9c99e0f77707a4cbfc3) 改进 RT/PQ 索引的刷新重新调度
* [Commit d0a7](https://github.com/manticoresoftware/manticoresearch/commit/d0a7c95906b9b5ba33c98267d54501cfe27508d6) 修复 #250 中的 index_field_lengths 索引选项，对于 TSV 和 CSV 管道源的支持
* [Commit 1266](https://github.com/manticoresoftware/manticoresearch/commit/1266d54800242709fbe2c34e72eb69b6595b23bd) 修复 indextool 对空索引的块索引检查错误报告
* [Commit 553c](https://github.com/manticoresoftware/manticoresearch/commit/553ca73ca111f28e0888d59cd48b0d6df181ecd6) 修复 Manticore SQL 查询日志中的空选择列表
* [Commit 56c8](https://github.com/manticoresoftware/manticoresearch/commit/56c8584456293665ddd4ce4c94307d74473a78b9) 修复 indexer -h/--help 响应

## Version 3.1.0, 2019年7月16日
### 功能与改进
* 实时索引的复制
* 中文的 ICU 分词器
* 新的形态选项 icu_chinese
* 新的指令 icu_data_dir
* 复制的多语句事务
* LAST_INSERT_ID()` 和 `@session.last_insert_id
* SHOW VARIABLES` 支持 `LIKE 'pattern'
* 对渗透索引的多文档插入
* 配置的时间解析器
* 内部任务管理器
* 对文档和命中列表组件的内存锁定（mlock）
* 隔离片段路径

### 删除项
* RLP 库支持已被移除，取而代之的是 ICU；所有 `rlp*` 指令已被删除
* 禁用通过UPDATE更新文档 ID

### Bug 修复
* [Commit f047](https://github.com/manticoresoftware/manticoresearch/commit/f047222339fd0e62a07e9ca8b2a829297d5db861) 修复了 `concat` 和 `group_concat` 中的缺陷
* [Commit b081](https://github.com/manticoresoftware/manticoresearch/commit/b08147ee24712b5eaa51f261626b5d16c6242fdd) 修复了渗透索引中查询 UID 的 BIGINT 属性类型
* [Commit 4cd8](https://github.com/manticoresoftware/manticoresearch/commit/4cd85afaf76eef0ce9fa1cd073f3fed160279890) 在无法预分配新磁盘块时不崩溃
* [Commit 1a55](https://github.com/manticoresoftware/manticoresearch/commit/1a551227886474054111088412f2b2396f485652) 将缺少的时间戳数据类型添加到 `ALTER`
* [Commit f3a8](https://github.com/manticoresoftware/manticoresearch/commit/f3a8e096fd496a6b001d398aadbb781b2dbad929) 修复了错误的 mmap 读取崩溃
* [Commit 4475](https://github.com/manticoresoftware/manticoresearch/commit/447577115192db7ac7bfea70b5ba58ee2c61229f) 修复了复制中集群锁的哈希
* [Commit ff47](https://github.com/manticoresoftware/manticoresearch/commit/ff476df955f057bd38e8b6ad6056325e9d6fbb9c) 修复了复制中的提供者泄漏
* [Commit 58dc](https://github.com/manticoresoftware/manticoresearch/commit/58dcbb779c4c30ca1560460c78e35ba114b83716) 修复了 #246 中索引器中未定义的 sigmask
* [Commit 3dd8](https://github.com/manticoresoftware/manticoresearch/commit/3dd8278e747d534f69fe9c3d360eef188ab7b6d5) 修复了 netloop 报告中的竞争条件
* [Commit a02a](https://github.com/manticoresoftware/manticoresearch/commit/a02aae05a3fa3d1caf2821f43c7e237b6de76336) 为 HA 策略的重新平衡器提供零间隙

## Version 3.0.2, 2019年5月31日
### 改进
* 为文档和命中列表添加了 mmap 读取器
* `/sql` HTTP 端点的响应现在与 `/json/search` 响应相同
* 新指令 `access_plain_attrs`、`access_blob_attrs`、`access_doclists`、`access_hitlists`
* 新指令 `server_id` 用于复制设置

### 删除项
* 移除了 HTTP `/search` 端点

### 弃用
* `read_buffer`、`ondisk_attrs`、`ondisk_attrs_default`、`mlock` 被 `access_*` 指令替代

### Bug 修复
* [Commit 849c](https://github.com/manticoresoftware/manticoresearch/commit/849c16e1040f382795ba0635cb48686f9db4f2d8) 允许以数字开头的属性名称在选择列表中使用
* [Commit 48e6](https://github.com/manticoresoftware/manticoresearch/commit/48e6c302ca37bec0bc49c79619c60bf787ed0708) 修复了 UDF 中的 MVAs，修复了 MVA 别名问题
* [Commit 0555](https://github.com/manticoresoftware/manticoresearch/commit/055586a9a2ebfa8874f68099c5bc798a145fd4ef) 修复了 #187 在使用包含 SENTENCE 的查询时崩溃的问题
* [Commit 93bf](https://github.com/manticoresoftware/manticoresearch/commit/93bf52f23f1c53c8a93a603387ca5463e1fc2ae8) 修复了 #143 支持 MATCH() 周围的括号
* [Commit 599e](https://github.com/manticoresoftware/manticoresearch/commit/599ee79ccd38cce0023a55cbce8f50c3dca62e38) 修复了在 ALTER cluster 语句中保存集群状态的问题
* [Commit 230c](https://github.com/manticoresoftware/manticoresearch/commit/230c321e277514b191629538eb3f02d219113d95) 修复了在 ALTER index 时服务器崩溃，涉及 blob 属性
* [Commit 5802](https://github.com/manticoresoftware/manticoresearch/commit/5802b85aa93a81caea77073a3381ef912fdd677c) 修复了 #196 按 ID 过滤的问题
* [Commit 25d2](https://github.com/manticoresoftware/manticoresearch/commit/25d2dabda4a299a29bd375405862126da9b4bfac) 丢弃在模板索引上进行搜索
* [Commit 2a30](https://github.com/manticoresoftware/manticoresearch/commit/2a30d5b49d76faab031d195f533974938769b8e6) 修复了 SQL 回复中 ID 列的常规 bigint 类型问题


## Version 3.0.0, 2019年5月6日
### 功能和改进
* 新的索引存储。非标量属性不再限制每个索引4GB的大小
* attr_update_reserve 指令
* 可以使用 UPDATE 更新字符串、JSON 和多值属性
* 杀死列表在索引加载时应用
* killlist_target 指令
* 多个 AND 查询加速
* 更好的平均性能和内存使用
* 用于升级 2.x 版本索引的转换工具
* CONCAT() 函数
* JOIN CLUSTER cluster AT 'nodeaddress'
* ALTER CLUSTER 帖子 更新节点
* node_address 指令
* 在 SHOW STATUS 中打印节点列表

### 行为变化
* 在具有杀死列表的索引中，服务器不再按配置文件中定义的顺序旋转索引，而是遵循杀死列表目标链
* 搜索中索引的顺序不再定义杀死列表应用的顺序
* 文档 ID 现在是带符号的大整数

### 移除的指令
* docinfo（现在始终为外部），inplace_docinfo_gap，mva_updates_pool

## Version 2.8.2 GA, 2019年4月2日
### 功能和改进
* Galera 复制用于渗透索引
* OPTION morphology

### 编译说明
CMake 的最低版本现在为 3.13。编译需要 boost 和 libssl 开发库。

### Bug 修复
* [Commit 6967](https://github.com/manticoresoftware/manticoresearch/commit/6967fedb2ef818ec1c825d482563edd05e1c9245) 修复在查询多个分布式索引时选择列表中许多星号导致的崩溃
* [Commit 36df](https://github.com/manticoresoftware/manticoresearch/commit/36df1a407dc08263690e3492518613ace82d69ca) 修复 [#177](https://github.com/manticoresoftware/manticoresearch/issues/177) 通过 Manticore SQL 接口发送大包
* [Commit 5793](https://github.com/manticoresoftware/manticoresearch/commit/57932aec734583fa93359faaf5034b2e2c9d352b) 修复 [#170](https://github.com/manticoresoftware/manticoresearch/issues/170) RT 优化时 MVA 更新导致的服务器崩溃
* [Commit edb2](https://github.com/manticoresoftware/manticoresearch/commit/edb24b870423add86eba471d361e0e5aff098b18) 修复由于 RT 索引删除后配置重载时 binlog 被删除导致的服务器崩溃
* [Commit bd3e](https://github.com/manticoresoftware/manticoresearch/commit/bd3e66e0085bc7e2e351b817dfc972fd8158fbce) 修复 MySQL 握手认证插件负载
* [Commit 6a21](https://github.com/manticoresoftware/manticoresearch/commit/6a217f6ed82fb10f752213b15617b9cedf1e8533) 修复 [#172](https://github.com/manticoresoftware/manticoresearch/issues/172) RT 索引的 phrase_boundary 设置
* [Commit 3562](https://github.com/manticoresoftware/manticoresearch/commit/3562f652753e4091fd7b5b0f65b58341c8cbcb31) 修复 [#168](https://github.com/manticoresoftware/manticoresearch/issues/168) 将索引附加到自身时发生的死锁
* [Commit 250b](https://github.com/manticoresoftware/manticoresearch/commit/250b3f0e74ad18dda34ba080aa13ca87798a9ac9) 修复服务器崩溃后 binlog 保存空的 meta
* [Commit 4aa6](https://github.com/manticoresoftware/manticoresearch/commit/4aa6c69ad3a181cd7c5dafb990fb528121f68e59) 修复由于来自 RT 索引的磁盘块字符串导致的服务器崩溃

## Version 2.8.1 GA, 2019年3月6日
### Features and improvements
* SUBSTRING_INDEX()
* 支持句子和段落的渗透查询
* Debian/Ubuntu 的 systemd 生成器；还添加了 LimitCORE 以允许核心转储

### Bug 修复
* [Commit 84fe](https://github.com/manticoresoftware/manticoresearch/commit/84fe7405d17b59f4dcde15a6c8e8503923503def) 修复了在匹配模式为 all 且空的全文查询时服务器崩溃的问题
* [Commit daa8](https://github.com/manticoresoftware/manticoresearch/commit/daa88b579fa373d5d2e869e43a2e178363b0fef1) 修复了删除静态字符串时的崩溃
* [Commit 2207](https://github.com/manticoresoftware/manticoresearch/commit/22078537dddbd9ce81a182dcc879f235b6e87004) 修复了 indextool 失败时退出代码的问题
* [Commit 0721](https://github.com/manticoresoftware/manticoresearch/commit/0721696d6780c200c65d596624a8187983fb7fcb) 修复了 [#109](https://github.com/manticoresoftware/manticoresearch/issues/109) 由于错误的精确形式检查导致的前缀无匹配问题
* [Commit 8af8](https://github.com/manticoresoftware/manticoresearch/commit/8af810111b8e2f87bc9f378172eff1ab9725c7e7) 修复了 [#161](https://github.com/manticoresoftware/manticoresearch/issues/161) RT 索引配置重新加载的问题
* [Commit e2d5](https://github.com/manticoresoftware/manticoresearch/commit/e2d592773f4fd702ace90070a793a842a0fec6ab) 修复了访问大 JSON 字符串时服务器崩溃的问题
* [Commit 75cd](https://github.com/manticoresoftware/manticoresearch/commit/75cd1342d05cdc352e3dd145cdbc6f79394a165b) 修复了 JSON 文档中由索引剥离器导致的 PQ 字段错误匹配问题
* [Commit e2f7](https://github.com/manticoresoftware/manticoresearch/commit/e2f775437df922674d772ed4417780492502e65a) 修复了 RHEL7 构建解析 JSON 时服务器崩溃的问题
* [Commit 3a25](https://github.com/manticoresoftware/manticoresearch/commit/3a25a5808feb3f8b80866991b436d6c1241618c2) 修复了斜杠位于边缘时 JSON 反转义的崩溃问题
* [Commit be9f](https://github.com/manticoresoftware/manticoresearch/commit/be9f497872bea62dbdccd64fc28294fd7776c289) 修复了选项 'skip_empty' 以跳过空文档而不警告它们不是有效的 JSON
* [Commit 266e](https://github.com/manticoresoftware/manticoresearch/commit/266e0e7b088549722a805fc837bf101ff681a5e8) 修复了 [#140](https://github.com/manticoresoftware/manticoresearch/issues/161) 当六位数不够精确时在浮点数上输出八位数字
* [Commit 3f6d](https://github.com/manticoresoftware/manticoresearch/commit/3f6d23891064ee8e94030c4231497cdd7da33a6a) 修复了空 JSON 对象的创建
* [Commit f3c7](https://github.com/manticoresoftware/manticoresearch/commit/f3c7848a59b7eedc67c3403d6d4b37ce6fa94dc8) 修复了 [#160](https://github.com/manticoresoftware/manticoresearch/issues/161) 空的 MVA 输出 NULL 而不是空字符串
* [Commit 0afa](https://github.com/manticoresoftware/manticoresearch/commit/0afa2ed058e5759470b1d7354c722faab34f98bb) 修复了在没有 pthread_getname_np 时无法构建的问题
* [Commit 9405](https://github.com/manticoresoftware/manticoresearch/commit/9405fccdeb27f0302a8d9a848981d30080216777) 修复了在使用线程池工作者时服务器关闭的崩溃问题

## Version 2.8.0 GA, 2019年1月28日
### Improvements
* 为渗透索引提供分布式索引支持
* CALL PQ 新选项和更改：
    *   skip_bad_json
    *   mode (sparsed/sharded)
    *   JSON 文档可以作为 JSON 数组传递
    *   shift
    *   列名 'UID'、'Documents'、'Query'、'Tags'、'Filters' 已重命名为 'id'、'documents'、'query'、'tags'、'filters'
* DESCRIBE pq TABLE
* SELECT FROM pq WHERE UID is not possible any more, use 'id' instead
* SELECT FROM pq WHERE UID 现在不可用了，请使用 'id'
* 对 pq 索引的 SELECT 查询与常规索引相当（例如，您可以通过 REGEX() 过滤规则）
* PQ 标签可以使用 ANY/ALL
* 表达式对 JSON 字段具有自动转换，不需要显式类型转换
* 内置的 'non_cjk' charset_table 和 'cjk' ngram_chars
* 为 50 种语言提供内置停用词集合
* 停用词声明中的多个文件也可以用逗号分隔
* CALL PQ 可以接受 JSON 文档数组

### Bug 修复
* [Commit a4e1](https://github.com/manticoresoftware/manticoresearch/commit/a4e19afee54dafdc04b0da53372dafd8c0d332d6) 修复了与 JSON 相关的内存泄漏
* [Commit 28d8](https://github.com/manticoresoftware/manticoresearch/commit/28d862774874891a03e361da1b0347ebe8066ce0) 修复了由于 JSON 中缺少值而导致的崩溃
* [Commit bf4e](https://github.com/manticoresoftware/manticoresearch/commit/bf4e9ea27c349cdc9ae4e54e960a4a17b90e38fa) 修复了 RT 索引保存空元数据的问题
* [Commit 33b4](https://github.com/manticoresoftware/manticoresearch/commit/33b4573529e7c7c7bce19bf9d54054866f30d99a) 修复了因相邻字段导致的 PQ 字段错误匹配
* [Commit 6b95](https://github.com/manticoresoftware/manticoresearch/commit/6b95d48240d3b5520afade19c249d79e020a5e88) 修复了访问大 JSON 字符串时服务器崩溃的问题
* [Commit 6214](https://github.com/manticoresoftware/manticoresearch/commit/621418b829e70af36aaa322a2f51ece3f86bc499) 修复了 SIGHUP 禁用索引时的配置重载问题
* [Commit 3f7e](https://github.com/manticoresoftware/manticoresearch/commit/3f7e35d1482966cc45d5abbcb2de5de3508d66b0) 修复了在 JSON 中解析时崩溃
* [Commit cd9e](https://github.com/manticoresoftware/manticoresearch/commit/cd9e4f1709a48ddafc128c450c2d882bc11ba50e) 修复了删除带有字段过滤器的 RT 索引时服务器崩溃的问题
* [Commit 9376](https://github.com/manticoresoftware/manticoresearch/commit/9376470d455fcd256c6abff79c431a6919dfa7ac) 修复了在空文档上调用 PQ 到分布式索引时服务器崩溃的问题
* [Commit 8868](https://github.com/manticoresoftware/manticoresearch/commit/8868b207644f4f8cc1ab0c270adb35493fed22e1) 修复了 Manticore SQL 错误消息超过 512 个字符的问题
* [Commit 238b](https://github.com/manticoresoftware/manticoresearch/commit/238bdea59bad89f097403f1c978658ce45f16c70) 修复了在没有 binlog 的情况下保存渗透索引时的崩溃
* [Commit 2b21](https://github.com/manticoresoftware/manticoresearch/commit/2b219e1a32791e7740bb210b7d408f96abc6374f) 修复了在 OSX 上 HTTP 接口无法正常工作的情况
* [Commit e92c](https://github.com/manticoresoftware/manticoresearch/commit/e92c60240f8f9e1756bfa14fba0bbb402586a389) 修复了 indextool 检查 MVA 时错误的错误消息
* [Commit 6b8c](https://github.com/manticoresoftware/manticoresearch/commit/6b8c4242ef77c3fa4d0ccb7d76d81714b6728f0b) 修复了在 FLUSH RTINDEX 时写锁定的问题，以便在保存和定期刷新时不对整个索引进行写锁定
* [Commit c26a](https://github.com/manticoresoftware/manticoresearch/commit/c26a236bd42b7082db079a51a84172066d337d64) 修复了在 ALTER 渗透索引时卡住等待搜索加载的问题
* [Commit 9ee5](https://github.com/manticoresoftware/manticoresearch/commit/9ee5703f6c28f57cd5bff5e705966d93fe30d267) 修复了在 max_children 为 0 时使用线程池工作者的默认数量的问题
* [Commit 5138](https://github.com/manticoresoftware/manticoresearch/commit/5138fc048a1bd146b271ce6a72fe954344281f69) 修复了在使用带有停用词和 stopword_step=0 的索引数据时出现的错误
* [Commit 2add](https://github.com/manticoresoftware/manticoresearch/commit/2add3d319ac62fe450bf60e89033100853dc2ecf) 修复了在索引定义中仍使用 AOT 词形还原器时缺少 lemmatizer_base 导致的崩溃问题。

## Version 2.7.5 GA, 2018年12月4日
### Improvements
* REGEX 函数
* son API 搜索的 limit/offset
* qcache 的 profiler 点

### Bug 修复
* [Commit eb3c](https://github.com/manticoresoftware/manticoresearch/commit/eb3c7683b9740cff4cc4cadd2ab9f990b9f7d173) 修复了在 FACET 查询中使用多个属性宽度类型时服务器崩溃的问题。
* [Commit d915](https://github.com/manticoresoftware/manticoresearch/commit/d915cf6696514f4be365ffb5981bccaa15983e65) 修复了 FACET 查询主选择列表中的隐式分组问题。
* [Commit 5c25](https://github.com/manticoresoftware/manticoresearch/commit/5c25dc271a8c11a1c82ca3c88e518acbc6a8c164) 修复了使用 GROUP N BY 查询时服务器崩溃的问题。
* [Commit 85d3](https://github.com/manticoresoftware/manticoresearch/commit/85d30a232d4ffd89916b673a9b2db946d272a945) 修复了在内存操作处理中发生崩溃时的死锁问题。
* [Commit 8516](https://github.com/manticoresoftware/manticoresearch/commit/85166b5ef2be38061d62f2b20f15acaa0311258a) 修复了 indextool 在检查期间的内存消耗问题。
* [Commit 58fb](https://github.com/manticoresoftware/manticoresearch/commit/58fb03166830d8b23e7ab9ce56309db7ddac22f4) 修复了不再需要 gmock 包含的问题，因为上游已经自行解决。

## Version 2.7.4 GA, 2018年11月1日
### 改进
* 在远程分布式索引的情况下，SHOW THREADS 打印原始查询而不是 API 调用。
* SHOW THREADS 新选项 `format=sphinxql` 以 SQL 格式打印所有查询。
* SHOW PROFILE 打印额外的 `clone_attrs` 阶段。

### Bug 修复
* [Commit 4f15](https://github.com/manticoresoftware/manticoresearch/commit/4f155712a0bccc1bd01cc191647bc8cff814888e) 修复了在没有 malloc_stats 和 malloc_trim 的 libc 上构建失败的问题。
* [Commit f974](https://github.com/manticoresoftware/manticoresearch/commit/f974f20bda3214a56877c393a192be1a77150958) 修复了 CALL KEYWORDS 结果集中的单词内部特殊符号问题。
* [Commit 0920](https://github.com/manticoresoftware/manticoresearch/commit/092083282ea8ae0b2e72fb6989c57ccec81e74ac) 修复了通过 API 或远程代理对分布式索引的 CALL KEYWORDS 的调用问题。
* [Commit fd68](https://github.com/manticoresoftware/manticoresearch/commit/fd686bfe88b720ffd7642e36897ba45161cbd7d2) 修复了分布式索引代理的 agent_query_timeout 传播到代理作为 max_query_time 的问题。
* [Commit 4ffa](https://github.com/manticoresoftware/manticoresearch/commit/4ffa623e9d357a3b0b441615089e211f92f8de32) 修复了磁盘块的总文档计数在 OPTIMIZE 命令影响下的权重计算问题。
* [Commit dcaf](https://github.com/manticoresoftware/manticoresearch/commit/dcaf4e0e3c8e9b0fbec47bbf3307feddec0936a6) 修复了来自混合的 RT 索引的多个尾部命中的问题。
* [Commit eee3](https://github.com/manticoresoftware/manticoresearch/commit/eee381754e902a43a8f499e1c950198a2a3e6ee0) 修复了旋转时的死锁问题。

## Version 2.7.3 GA, 2018年9月26日
### 改进
* CALL KEYWORDS 的 sort_mode 选项
* VIP 连接上的 DEBUG 可以执行 'crash <password>' 以在服务器上故意触发 SIGEGV 行为
* DEBUG 可以执行 'malloc_stats' 以在 searchd.log 中转储 malloc 统计信息，'malloc_trim' 用于执行 malloc_trim()
* 如果系统上存在 gdb，则改进了回溯信息

### Bug 修复
* [Commit 0f3c](https://github.com/manticoresoftware/manticoresearch/commit/0f3cc333f6129451ad1ae2c97905b6c04ba5d454) 修复了 Windows 上重命名时的崩溃或失败
* [Commit 1455](https://github.com/manticoresoftware/manticoresearch/commit/1455ba2bf8079f03ce7f861a8d9662a360d705de) 修复了 32 位系统上的服务器崩溃
* [Commit ad37](https://github.com/manticoresoftware/manticoresearch/commit/ad3710d53b5cd6a28b8c60352d0f9e285b03d9fa) 修复了空 SNIPPET 表达式导致的服务器崩溃或挂起
* [Commit b36d](https://github.com/manticoresoftware/manticoresearch/commit/b36d79214364500252665860f000ae85343528b6) 修复了非渐进优化和渐进优化未创建最新磁盘块的杀死列表
* [Commit 34b0](https://github.com/manticoresoftware/manticoresearch/commit/34b032499afd42ce47a4c7247814b4031094388a) 修复了在线程池工作模式下 SQL 和 API 的 queue_max_length 错误回复
* [Commit ae4b](https://github.com/manticoresoftware/manticoresearch/commit/ae4b3202cbdb8014cabe2b90e269d5cf74f49871) 修复了在设置 regexp 或 rlp 选项时将全扫描查询添加到 PQ 索引时的崩溃
* [Commit f80f](https://github.com/manticoresoftware/manticoresearch/commit/f80f8d5d7560187078868aed9a9575f4549e98aa) 修复了连续调用一个 PQ 后的崩溃
* [Commit 9742](https://github.com/manticoresoftware/manticoresearch/commit/9742f5f0866af73f8cd8483ecd18a507ea80dd65) 重构了 AcquireAccum
* [Commit 39e5](https://github.com/manticoresoftware/manticoresearch/commit/39e5bc3751b7295222eb76407c5d72ce1dad545b) 修复了调用 pq 后的内存泄漏
* [Commit 21bc](https://github.com/manticoresoftware/manticoresearch/commit/21bcc6d17395f0f57dde79f6716ef303b7ea527d) 视觉美化重构（C++11 风格的 C-trs、默认值、nullptrs）
* [Commit 2d69](https://github.com/manticoresoftware/manticoresearch/commit/2d690398f14c736956cfdd66feb4d3091d6b3a4d) 修复了尝试将重复项插入 PQ 索引时的内存泄漏
* [Commit 5ed9](https://github.com/manticoresoftware/manticoresearch/commit/5ed92c4b66da6423fa28f11b1fd59103ed1e1fb9) 修复了在大型值上使用 JSON 字段 IN 时的崩溃
* [Commit 4a52](https://github.com/manticoresoftware/manticoresearch/commit/4a5262e2776aa261e34ba5c914ea60804f59de3f) 修复了在设置扩展限制时对 RT 索引调用 KEYWORDS 语句的服务器崩溃
* [Commit 5526](https://github.com/manticoresoftware/manticoresearch/commit/552646bb6fefde4e2525298e43e628839b421d3d) 修复了 PQ 匹配查询中的无效过滤器
* [Commit 204f](https://github.com/manticoresoftware/manticoresearch/commit/204f521408b730198a5bab2c20407a3d7df94873) 引入小对象分配器用于 ptr attrs
* [Commit 2545](https://github.com/manticoresoftware/manticoresearch/commit/25453e5387152c0575d1eda1b2beb89a49c7493a) 将 ISphFieldFilter 重构为引用计数版本
* [Commit 1366](https://github.com/manticoresoftware/manticoresearch/commit/1366ee06e828397074b69bec9265d7147170c60e) 修复了在非终止字符串上使用 strtod 时的 UB/SIGSEGV
* [Commit 94bc](https://github.com/manticoresoftware/manticoresearch/commit/94bc6fcbdae4d08bcf6da2966e3ec2966091289f) 修复了 JSON 结果集处理中的内存泄漏
* [Commit e78e](https://github.com/manticoresoftware/manticoresearch/commit/e78e9c948963416caff9e4e46296a58080107835) 修复了在应用属性添加时超过内存块末尾的读取
* [Commit fad5](https://github.com/manticoresoftware/manticoresearch/commit/fad572fb543606b33e688eb82a485b86011545fd) 将 CSphDict 重构为引用计数版本
* [Commit fd84](https://github.com/manticoresoftware/manticoresearch/commit/fd841a472b472280c93f4af3506851d19f6adc45) 修复了 AOT 内部类型的泄漏
* [Commit 5ee7](https://github.com/manticoresoftware/manticoresearch/commit/5ee7f208c7685f2ebb9e5623a8802b721e2a7f0a) 修复了标记管理中的内存泄漏
* [Commit 116c](https://github.com/manticoresoftware/manticoresearch/commit/116c5f1abebee9a0d99afe93546a1d8e4c6c6958) 修复了在分组器中的内存泄漏
* [Commit 56fd](https://github.com/manticoresoftware/manticoresearch/commit/56fdbc9ab38973a9a0ba8ccee45f71cf33c16423) 修复了匹配中的动态指针的特殊释放/复制（内存泄漏分组器）
* [Commit b1fc](https://github.com/manticoresoftware/manticoresearch/commit/31db660f378541aa0d1e7cf2e7209bb0a1f47fd8) 修复了 RT 的动态字符串内存泄漏
* [Commit 517b](https://github.com/manticoresoftware/manticoresearch/commit/517b9e8749f092e923cd2884b674b5bb84e20755) 重构了分组器
* [Commit b1fc](https://github.com/manticoresoftware/manticoresearch/commit/b1fc16140e5dc44290686330b476a254e0092107) 进行小的重构（C++11 风格的 C-trs，一些格式调整）
* [Commit 7034](https://github.com/manticoresoftware/manticoresearch/commit/7034e073f4cf2844762e0a464b7c3de05d3d122b) 将 ISphMatchComparator 重构为引用计数版本
* [Commit b1fc](https://github.com/manticoresoftware/manticoresearch/commit/b1fc16140e5dc44290686330b476a254e0092107) 私有化克隆器
* [Commit efbc](https://github.com/manticoresoftware/manticoresearch/commit/efbc0511d6809c4a57453c7283f9bf53f3fb8d97) 简化 MVA_UPSIZE、DOCINFO2ID_T、DOCINFOSETID 的本地小端实现
* [Commit 6da0](https://github.com/manticoresoftware/manticoresearch/commit/6da0df4ac9946fc59f98d229b90e12c9221dd4c0) 将 valgrind 支持添加到 ubertests
* [Commit 1d17](https://github.com/manticoresoftware/manticoresearch/commit/1d17669cb7ec54ac80b392e101f37a688eb98080) 修复了由于连接上的“成功”标志竞争导致的崩溃
* [Commit 5a09](https://github.com/manticoresoftware/manticoresearch/commit/5a09c32d7ded56b2c0bd3e2ad7968cb8d6362919) 将 epoll 切换为边缘触发模式
* [Commit 5d52](https://github.com/manticoresoftware/manticoresearch/commit/5d528682737fca03a4352a3093b1319ec27d598c) 修复了格式化为过滤器时的表达式中的 IN 语句
* [Commit bd8b](https://github.com/manticoresoftware/manticoresearch/commit/bd8b3c976ff8b4667f55dfdd101b20a920137ac5) 修复了对大 docid 的文档提交时 RT 索引的服务器崩溃
* [Commit ce65](https://github.com/manticoresoftware/manticoresearch/commit/ce656b83b747ce7c55795b53915770c13fb99cfe) 修复了 indextool 中无参数选项的问题
* [Commit 08c9](https://github.com/manticoresoftware/manticoresearch/commit/08c9507177820f441f534bf06a11dac5e54bebb4) 修复了扩展关键字的内存泄漏
* [Commit 30c7](https://github.com/manticoresoftware/manticoresearch/commit/30c75a2f525ec9bda625d903acdc9d4d2e2413dc) 修复了 JSON 分组器的内存泄漏
* [Commit 6023](https://github.com/manticoresoftware/manticoresearch/commit/6023f269b6f2080e4d380b0957605ef8107c8c9f) 修复了全局用户变量的泄漏
* [Commit 7c13](https://github.com/manticoresoftware/manticoresearch/commit/7c138f15ca23c0c8717fa12041e3db7f988fcf15) 修复了在早期拒绝匹配时动态字符串的泄漏
* [Commit 9154](https://github.com/manticoresoftware/manticoresearch/commit/9154b18eaed5302e8330cb3a95bd968959ccb312) 修复了在 length(<expression>) 中的泄漏
* [Commit 43fc](https://github.com/manticoresoftware/manticoresearch/commit/43fca3a4e26139765d0fac8de054200dc4875fc6) 修复了由于 parser 中的 strdup() 引起的内存泄漏
* [Commit 71ff](https://github.com/manticoresoftware/manticoresearch/commit/71ff77737e63d1019b7c9804dca7fa2351025dba) 修复了使用线程池工作者时服务器关闭时的崩溃

## Version 2.7.2 GA, 2018年8月27日
### Improvements
* 与 MySQL 8 客户端的兼容性
* [TRUNCATE](Emptying_a_table.md) WITH RECONFIGURE
* RT 索引的 SHOW STATUS 中新增的已退休内存计数器
* 多代理的全局缓存
* 改进 Windows 上的 IOCP
* HTTP 协议的 VIP 连接
* Manticore SQL [DEBUG](Reporting_bugs.md#DEBUG) 命令可以执行各种子命令
* [shutdown_token](Server_settings/Searchd.md#shutdown_token) - 调用 DEBUG 命令以执行 `shutdown` 所需的密码 SHA1 哈希
* SHOW AGENT STATUS 新增统计信息 (_ping, _has_perspool, _need_resolve)
* indexer 的 --verbose 选项现在接受 debugvvdebugvvdebugvv 用于打印调试消息

### Bug 修复
* [Commit 3900](https://github.com/manticoresoftware/manticoresearch/commit/390082a7be0a1f9539b30361d11d54de35c62a44) 移除了优化时的写锁
* [Commit 4c33](https://github.com/manticoresoftware/manticoresearch/commit/4c337648329be22373be33333cbc9c5664d18b80) 修复了重新加载索引设置时的写锁问题
* [Commit b5ea](https://github.com/manticoresoftware/manticoresearch/commit/b5ea8dc0b18a23dbd739e92c66dd93b0094df14b) 修复了使用 JSON 过滤器查询时的内存泄漏
* [Commit 930e](https://github.com/manticoresoftware/manticoresearch/commit/930e831a43fd3d1ce996185be968d1df49a088d0) 修复了 PQ 结果集中空文档的问题
* [Commit 53de](https://github.com/manticoresoftware/manticoresearch/commit/53deecfecb374431f84f8592a1095a77407b8aea) 修复了因移除任务导致的任务混淆
* [Commit cad9](https://github.com/manticoresoftware/manticoresearch/commit/cad9b9c617719aad97e9f5a2ae93491153d8e504) 修复了错误的远程主机计数
* [Commit 9000](https://github.com/manticoresoftware/manticoresearch/commit/90008cc15c4684d445d37a391bb4a667bbc2e365) 修复了解析代理描述符时的内存泄漏
* [Commit 978d](https://github.com/manticoresoftware/manticoresearch/commit/978d839641ad6040c22c9fc3b703b1a02685f266) 修复了搜索中的内存泄漏
* [Commit 0193](https://github.com/manticoresoftware/manticoresearch/commit/0193946105ac08913362b7d7acf0c90f9b38dc85) 修复了显式/内联 c-trs 和 override/final 使用的外观变化
* [Commit 943e](https://github.com/manticoresoftware/manticoresearch/commit/943e2997b33f9b2eda23d94a66068910f9476ebd) 修复了本地/远程模式中 JSON 的内存泄漏
* [Commit 02db](https://github.com/manticoresoftware/manticoresearch/commit/02dbdd6f1d0b72e7e458ebebdd2c67f989577e6b) 修复了本地/远程模式中 JSON 排序列表达式的内存泄漏
* [Commit c74d](https://github.com/manticoresoftware/manticoresearch/commit/c74d0b4035e4214510376845b22cb676a8da24a3) 修复了常量别名的内存泄漏
* [Commit 6e5b](https://github.com/manticoresoftware/manticoresearch/commit/6e5b57e1d3952c9695376728a143bf5434208f53) 修复了预读线程的内存泄漏
* [Commit 39c7](https://github.com/manticoresoftware/manticoresearch/commit/39c740fef5bf71da0bee60a35f5dd9f471af850f) 修复了因在 netloop 中等待而导致的退出卡住
* [Commit adaf](https://github.com/manticoresoftware/manticoresearch/commit/adaf9772f95772ad6d5297acace6c5cb92b56ad5) 修复了将 HA 代理更改为普通主机时的 'ping' 行为卡住问题
* [Commit 32c4](https://github.com/manticoresoftware/manticoresearch/commit/32c40eac9cdd40b15d79e58b2f15416f164f9701) 为仪表板存储分离的垃圾回收
* [Commit 511a](https://github.com/manticoresoftware/manticoresearch/commit/511a3cf2c1f1858f6e94fe9f8175b7216db3cbd6) 修复了引用计数指针的修复
* [Commit 32c4](https://github.com/manticoresoftware/manticoresearch/commit/af2326cd2927effbad7bec88e8dc238a566cf27e) 修复了在不存在的索引上发生的 indextool 崩溃
* [Commit 156e](https://github.com/manticoresoftware/manticoresearch/commit/156edc717dc63d3ea120f42466eba6e022da88be) 修复了在 xmlpipe 索引中输出属性/字段名称超过限制的问题
* [Commit cdac](https://github.com/manticoresoftware/manticoresearch/commit/cdac6d1f17d384bfc8bae49e91241b2f346c3830) 修复了如果配置中没有索引器部分则使用索引器的默认值
* [Commit e61e](https://github.com/manticoresoftware/manticoresearch/commit/e61ec00b6b27d1d5878247e2ee817f3b1e7fde16) 修复了服务器重启后 RT 索引的磁盘块中嵌入的停用词错误
* [Commit 5fba](https://github.com/manticoresoftware/manticoresearch/commit/5fba49d5e28c17de4c0acbd984466127f42de6e8) 修复了跳过幻影（已关闭但尚未最终删除的）连接
* [Commit f22a](https://github.com/manticoresoftware/manticoresearch/commit/f22ae34b623906b3c5a05a06198dad2e548dc541) 修复了混合（孤儿）网络任务
* [Commit 4689](https://github.com/manticoresoftware/manticoresearch/commit/46890e70eb8dcd00c1bf1b030538b8f131c601c2) 修复了写后读操作时的崩溃
* [Commit 03f9](https://github.com/manticoresoftware/manticoresearch/commit/03f9df904f611c438d5ebcaaeef911b0dc8d6c39) 修复了在 Windows 上运行测试时的 searchd 崩溃
* [Commit e925](https://github.com/manticoresoftware/manticoresearch/commit/e9255ed2704790a3d7f5273d38b85433463cb3ff) 修复了在常规连接时处理 EINPROGRESS 代码
* [Commit 248b](https://github.com/manticoresoftware/manticoresearch/commit/248b72f1544b27e9e82773284050d44eeb1714e1) 修复了使用 TFO 时的连接超时问题

## Version 2.7.1 GA, 2018年7月4日
### 改进
* 提高了在 PQ 中匹配多个文档时通配符的性能
* 支持 PQ 的全扫描查询
* 支持 PQ 的 MVA 属性
* 支持用于渗透索引的正则表达式和 RLP

### Bug 修复
* [Commit 6885](https://github.com/manticoresoftware/manticoresearch/commit/68856261b41f6950666f9e5122726839fb3c71d0) 修复了查询字符串丢失的问题
* [Commit 0f17](https://github.com/manticoresoftware/manticoresearch/commit/0f1770943cba53bfd2e0edfdf0b1495ff0dd0e89) 修复了 SHOW THREADS 语句中的空信息
* [Commit 53fa](https://github.com/manticoresoftware/manticoresearch/commit/53faa36a916bb87868e83aa79c1a1f972fb20ca0) 修复了使用 NOTNEAR 运算符时崩溃的问题
* [Commit 2602](https://github.com/manticoresoftware/manticoresearch/commit/26029a7cd77b518fb3a27588d7db6c8bffc73b0f) 修复了 PQ 删除时错误过滤器的错误信息


## Version 2.7.0 GA, 2018年6月11日
### 改进
* 减少系统调用次数，以避免 Meltdown 和 Spectre 补丁的影响
* 本地索引管理的内部重写
* 远程片段重构
* 全配置重新加载
* 所有节点连接现在是独立的
* 协议改进
* Windows 通信从 wsapoll 切换到 IO 完成端口
* TFO 可用于主节点与从节点之间的通信
* [SHOW STATUS](Node_info_and_management/Node_status.md#SHOW-STATUS) 现在输出服务器版本和 mysql_version_string
* 为在 CALL PQ 中调用的文档添加了 `docs_id` 选项
* 渗透查询过滤器现在可以包含表达式
* 分布式索引可以与 FEDERATED 一起工作
* 虚拟的 SHOW NAMES COLLATE 和 `SET wait_timeout`（以更好地兼容 ProxySQL）

### Bug 修复
* [Commit 5bcf](https://github.com/manticoresoftware/manticoresearch/commit/5bcff05d19e495f1a44f190885b25cbb6f217c4a) 修复了在 PQ 中添加不等于标签的问题
* [Commit 9ebc](https://github.com/manticoresoftware/manticoresearch/commit/9ebc58916cd515eaa88da66d0895aebf0d1f2b5f) 修复了 JSON 文档 CALL PQ 语句中添加文档 ID 字段的问题
* [Commit 8ae0](https://github.com/manticoresoftware/manticoresearch/commit/8ae0e593a286110d8505d88baa3ac9e433cb38c9) 修复了 PQ 索引的 flush 语句处理程序
* [Commit c24b](https://github.com/manticoresoftware/manticoresearch/commit/c24b152344ecd77661566cddb803487f51d3c1aa) 修复了 JSON 和字符串属性上的 PQ 过滤问题
* [Commit 1b8b](https://github.com/manticoresoftware/manticoresearch/commit/1b8bdde19aaa362785ea4c33bdc019154f8fbac4) 修复了空 JSON 字符串的解析
* [Commit 1ad8](https://github.com/manticoresoftware/manticoresearch/commit/1ad8a07dbfc1131913a8d92c261fbb48f934e8b7) 修复了多查询与 OR 过滤器时崩溃的问题
* [Commit 69b8](https://github.com/manticoresoftware/manticoresearch/commit/69b89806380a229e36287ad9daf6f0ea2b5453eb) 修复了 indextool 使用配置公共部分（lemmatizer_base 选项）进行命令（dumpheader）
* [Commit 6dbe](https://github.com/manticoresoftware/manticoresearch/commit/6dbeaf2389d2af6a9dfccb56bfc986685b9f203e) 修复了结果集和过滤器中的空字符串问题
* [Commit 39c4](https://github.com/manticoresoftware/manticoresearch/commit/39c4eb55ecc85f6dd54f6c8c6d6dfcf87fd8748e) 修复了负文档 ID 值
* [Commit 266b](https://github.com/manticoresoftware/manticoresearch/commit/266b7054f98e7d30fc2829b5c2467dc9f90def92) 修复了非常长单词的词条剪裁长度
* [Commit 4782](https://github.com/manticoresoftware/manticoresearch/commit/47823b0bd06cbd016a24345c99bb8693790b43dd) 修复了在 PQ 中匹配多个文档的通配符查询的问题


## Version 2.6.4 GA, 2018年5月3日
### 功能和改进
* 支持 MySQL FEDERATED 引擎[支持](Extensions/FEDERATED.md)
* MySQL 数据包现在返回 SERVER_STATUS_AUTOCOMMIT 标志，增加了与 ProxySQL 的兼容性
* [listen_tfo](Server_settings/Searchd.md#listen_tfo) - 为所有监听器启用 TCP Fast Open 连接
* indexer --dumpheader 现在也可以从 .meta 文件中转储 RT 头信息
* 为 Ubuntu Bionic 提供的 cmake 构建脚本

### Bug 修复
* [Commit 355b](https://github.com/manticoresoftware/manticoresearch/commit/355b11629174813abd9cd5bf2233be0783f77745) 修复了 RT 索引的无效查询缓存条目；
* [Commit 546e](https://github.com/manticoresoftware/manticoresearch/commit/546e2297a01cc2913bc0d33052d5bcefae8737eb) 修复了无缝旋转后丢失的索引设置
* [Commit 0c45](https://github.com/manticoresoftware/manticoresearch/commit/0c4509898393993bc87194d2aca2070395ff7f83) 修复了固定的中缀与前缀长度设置；增加了对不支持的中缀长度的警告
* [Commit 8054](https://github.com/manticoresoftware/manticoresearch/commit/80542fa54d727c781635918e8d4d65a20c2fbd1e) 修复了 RT 索引的自动刷新顺序
* [Commit 705d](https://github.com/manticoresoftware/manticoresearch/commit/705d8c5f407726ed1c6f9745ecffa375682c8969) 修复了多个属性索引及查询多个索引时结果集模式的问题
* [Commit b0ba](https://github.com/manticoresoftware/manticoresearch/commit/b0ba932fff4a082f642cb737381c8a70215d4cc9) 修复了批量插入时因文档重复而丢失的一些命中
* [Commit 4510](https://github.com/manticoresoftware/manticoresearch/commit/4510fa44ad54b5bbac53c591144501565130fcae) 修复了优化未能合并文档数量较大的 RT 索引的磁盘块问题

## Version 2.6.3 GA, 2018年3月28日
### 改进
* 编译时使用 jemalloc。如果系统中存在 jemalloc，可以通过 cmake 标志 `-DUSE_JEMALLOC=1` 启用。

### Bug 修复
* [Commit 85a6](https://github.com/manticoresoftware/manticoresearch/commit/85a6d7e3db75f28c491042409bb677ed39eba07b) 修复了在 Manticore SQL 查询日志中扩展关键字选项的日志记录
* [Commit caaa](https://github.com/manticoresoftware/manticoresearch/commit/caaa3842f4dbd8507fd9e20bf3371a0c8ec0c087) 修复了 HTTP 接口以正确处理大尺寸查询
* [Commit e386](https://github.com/manticoresoftware/manticoresearch/commit/e386d842b36195719e459b2db1fb20e402f9a51f) 修复了在启用 index_field_lengths 的情况下对 RT 索引进行 DELETE 时导致的服务器崩溃
* [Commit cd53](https://github.com/manticoresoftware/manticoresearch/commit/ce528f37eb5d8284661928f6f3b340a4a1985253) 修复了 cpustats searchd CLI 选项在不受支持的系统上工作的问题
* [Commit 8740](https://github.com/manticoresoftware/manticoresearch/commit/8740fd63ae842c2ab0e51a8fcd6180d787ed57ec) 修复了具有定义的最小长度的 utf8 子字符串匹配问题


## Version 2.6.2 GA, 2018年2月23日
### 改进
* 在使用 NOT 操作符和批量文档的情况下提高了 [Percolate Queries](Searching/Percolate_query.md) 的性能。
* [percolate_query_call](Searching/Percolate_query.md) 可以根据 [dist_threads](Server_settings/Searchd.md#threads) 使用多个线程。
* 新增全文匹配操作符 NOTNEAR/N。
* 对 percolate 索引的 SELECT 添加了 LIMIT 支持。
* [expand_keywords](Searching/Options.md#expand_keywords) 现在可以接受 'start' 和 'exact'（其中 'star,exact' 的效果与 '1' 相同）。
* 为 [joined fields](Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Processing_fetched_data.md#sql_joined_field) 提供了范围主查询，使用 sql_query_range 定义的范围查询。

### Bug 修复
* [Commit 72dc](https://github.com/manticoresoftware/manticoresearch/commit/72dcf669744e9b7d636dfc213d24df85ab301f6b) 修复了在搜索 ram 段时崩溃；双缓冲时保存磁盘块时的死锁；优化过程中保存磁盘块时的死锁。
* [Commit 3613](https://github.com/manticoresoftware/manticoresearch/commit/36137149a1c3c0893bdda5a28fc7e8244bf2d4ae) 修复了在带有空属性名称的 xml 嵌入模式下导致的索引器崩溃。
* [Commit 48d7](https://github.com/manticoresoftware/manticoresearch/commit/48d7e8001d2a66466ca64577f27ddc5421a67251) 修复了错误取消链接非拥有的 pid 文件。
* [Commit a556](https://github.com/manticoresoftware/manticoresearch/commit/a5563a465ddc59ef71e65f17b68bc33f9700e838) 修复了有时留在临时文件夹中的孤儿 fifo。
* [Commit 2376](https://github.com/manticoresoftware/manticoresearch/commit/2376e8fc4508944b96959bd10686c6d51f5145e8) 修复了由于错误的 NULL 行导致的空 FACET 结果集。
* [Commit 4842](https://github.com/manticoresoftware/manticoresearch/commit/4842b67acdd5da75672db6c08ac563b48577ddd7) 修复了在以 Windows 服务身份运行服务器时索引锁损坏的问题。
* [Commit be35](https://github.com/manticoresoftware/manticoresearch/commit/be35feef54ded59125750916cd211e56108eddae) 修复了 macOS 上的错误 iconv 库。
  [Commit 8374](https://github.com/manticoresoftware/manticoresearch/commit/83744a977d7837d1d87cd506b88b0e7eb199efe6) 修复了错误的 count(*)。


## Version 2.6.1 GA, 2018年1月26日
### 改进
* 在代理具有镜像的情况下，[agent_retry_count](Server_settings/Searchd.md#agent_retry_count) 返回每个镜像的重试次数，而不是每个代理的重试次数，总重试次数为 agent_retry_count * mirrors。
* [agent_retry_count](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_retry_count) 现在可以在每个索引中指定，覆盖全局值。增加了别名 [mirror_retry_count](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#mirror_retry_count)。
* 可以在代理定义中指定 retry_count，表示每个代理的重试次数。
* 现在 Percolate Queries 可以在 HTTP JSON API 的 [/json/pq](Searching/Percolate_query.md#Performing-a-percolate-query-with-CALL-PQ) 中使用。
* 向可执行文件添加了 -h 和 -v 选项（帮助和版本）。
* 支持 Real-Time 索引的 [morphology_skip_fields](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology)。

### Bug 修复
* [Commit a40b](https://github.com/manticoresoftware/manticore/commit/a40b0793feff65e40d10062568d9847c08d10f57) 修复了在 MVA 字段上使用 sql_range_step 时，ranged-main-query 正确工作的 issue。
* [Commit f2f5](https://github.com/manticoresoftware/manticore/commit/f2f53757db45bcfb1544263ce0817e856656a621) 修复了黑洞系统循环挂起的问题，并且黑洞代理似乎断开连接。
* [Commit 84e1](https://github.com/manticoresoftware/manticore/commit/84e1f54aef25e0fce98870ad2dd784db5116f1d6) 修复了查询 ID 的一致性，修复了存储查询的重复 ID。
* [Commit 1948](https://github.com/manticoresoftware/manticore/commit/19484231814fcb82b21763a3a4a9f45adc6b2d40) 修复了在各种状态下关闭时服务器崩溃的问题。
* [Commit 9a70](https://github.com/manticoresoftware/manticore/commit/9a706b499a1d61a90076065a1a703029d49db958) 以及 [Commit 3495](https://github.com/manticoresoftware/manticore/commit/3495fd70cba8846b1a50d55d6679b039414c3d2a) 修复了长查询的超时问题。
* [Commit 3359](https://github.com/manticoresoftware/manticore/commit/3359bcd89b4f79a645fe84b8cf8616ce0addff02) 重构了基于 kqueue 的系统（Mac OS X，BSD）上的主代理网络轮询。


## Version 2.6.0, 2017年12月29日
### 功能和改进
* [HTTP JSON](Connecting_to_the_server/HTTP.md)：JSON 查询现在可以对属性进行等值比较，MVA 和 JSON 属性可以在插入和更新中使用，JSON API 也可以在分布式索引上执行更新和删除。
* [Percolate Queries](Searching/Percolate_query.md)
* 从代码中移除了对 32 位文档 ID 的支持，同时也移除了所有转换/加载具有 32 位文档 ID 的遗留索引的代码。
* [仅对特定字段进行形态分析](https://github.com/manticoresoftware/manticore/issues/7)。新的索引指令 morphology_skip_fields 允许定义一个字段列表，这些字段不适用形态分析。
* [expand_keywords 现在可以作为查询运行时指令，通过 OPTION 语句设置](https://github.com/manticoresoftware/manticore/issues/8)。

### Bug 修复
* [Commit 0cfa](https://github.com/manticoresoftware/manticore/commit/0cfae4c) 修复了服务器的调试构建时崩溃（以及可能的发布版本中的未定义行为），当使用 rlp 构建时。
* [Commit 3242](https://github.com/manticoresoftware/manticore/commit/324291e) 修复了启用渐进选项时的 RT 索引优化，该选项以错误的顺序合并了 kill-list。
* [Commit ac0e](https://github.com/manticoresoftware/manticore/commit/ac0efee) 修复了 mac 上的小崩溃。
* 在彻底的静态代码分析后进行了大量小修复。
* 其他小的 bug 修复。

### 升级
在此版本中，我们更改了主节点与代理节点之间的内部协议。如果您在分布式环境中运行 Manticoresearch，并且有多个实例，请确保首先升级代理节点，然后再升级主节点。

## Version 2.5.1, 2017年11月23日
### 功能和改进
* 支持在 [HTTP API 协议](Connecting_to_the_server/HTTP.md) 上进行 JSON 查询，包括搜索、插入、更新、删除和替换操作。数据操作命令也可以批量处理，但目前存在一些限制，MVA 和 JSON 属性不能用于插入、替换或更新。
* [RELOAD INDEXES](Data_creation_and_modification/Adding_data_from_external_storages/Rotating_a_table.md#RELOAD-TABLES) 命令。
* [FLUSH LOGS](Logging/Rotating_query_and_server_logs.md) 命令。
* [SHOW THREADS](Node_info_and_management/SHOW_THREADS.md) 可以显示优化、旋转或刷新过程的进度。
* GROUP N BY 正确支持 MVA 属性。
* 黑洞代理在单独的线程中运行，以避免对主查询造成影响。
* 实现了索引的引用计数，以避免因旋转和高负载造成的停滞。
* 实现了 SHA1 哈希，尚未公开暴露。
* 修复了在 FreeBSD、macOS 和 Alpine 上编译的问题。

### Bug 修复
* [Commit 9897](https://github.com/manticoresoftware/manticore/commit/989752b) 修复了块索引的过滤回归。
* [Commit b1c3](https://github.com/manticoresoftware/manticore/commit/b1c3864) 将 PAGE_SIZE 重命名为 ARENA_PAGE_SIZE，以兼容 musl。
* [Commit f213](https://github.com/manticoresoftware/manticore/commit/f2133cc) 在 cmake < 3.1.0 时禁用 googletests。
* [Commit f30e](https://github.com/manticoresoftware/manticore/commit/0839de7) 服务器重启时无法绑定套接字。
* [Commit 0807](https://github.com/manticoresoftware/manticore/commit/0807240) 修复了服务器在关闭时崩溃的问题。
* [Commit 3e3a](https://github.com/manticoresoftware/manticore/commit/3e3acc3) 修复了系统黑洞线程的显示线程问题。
* [Commit 262c](https://github.com/manticoresoftware/manticore/commit/262c3fe) 重构了 iconv 的配置检查，修复了在 FreeBSD 和 Darwin 上的构建问题。

## Version 2.4.1 GA, 2017年10月16日
### 功能和改进
* 在 WHERE 子句中的属性过滤器之间支持 OR 运算符。
* 维护模式（SET MAINTENANCE=1）。
* [CALL KEYWORDS](Searching/Autocomplete.md#CALL-KEYWORDS) 可用于分布式索引。
* [以 UTC 分组](Server_settings/Searchd.md#grouping_in_utc)。
* [query_log_mode](Server_settings/Searchd.md#query_log_mode) 可自定义日志文件权限。
* 字段权重可以为零或负值。
* [max_query_time](Searching/Options.md#max_query_time) 现在也可以影响全扫描。
* 添加了 [net_wait_tm](Server_settings/Searchd.md#net_wait_tm)、[net_throttle_accept](Server_settings/Searchd.md#net_throttle_accept) 和 [net_throttle_action](Server_settings/Searchd.md#net_throttle_action) 以进行网络线程微调（在 workers=thread_pool 的情况下）。
* COUNT DISTINCT 在 facet 搜索中有效。
* IN 可以与 JSON 浮点数组一起使用。
* 多查询优化不再因整数/浮点表达式而被破坏。
* [SHOW META](Node_info_and_management/SHOW_META.md) 在使用多查询优化时显示 `multiplier` 行。

### 编译
Manticore Search 使用 cmake 构建，所需的最低 gcc 版本为 4.7.2。

### 文件夹和服务
* Manticore Search 以 `manticore` 用户身份运行。
* 默认数据文件夹现在为 `/var/lib/manticore/`。
* 默认日志文件夹现在为 `/var/log/manticore/`。
* 默认 pid 文件夹现在为 `/var/run/manticore/`。

### Bug 修复
* [Commit a58c](https://github.com/manticoresoftware/manticore/commit/a58c619) 修复了 BREAKS java connector 的 SHOW COLLATION 语句。
* [Commit 631c](https://github.com/manticoresoftware/manticore/commit/631cf4e) 修复了处理分布式索引时的崩溃；为分布式索引哈希添加了锁；从代理中移除了移动和复制操作符。
* [Commit 942b](https://github.com/manticoresoftware/manticore/commit/942bec0) 修复了由于并行重连导致处理分布式索引时的崩溃。
* [Commit e5c1](https://github.com/manticoresoftware/manticore/commit/e5c1ed2) 修复了在将查询存储到服务器日志时崩溃处理程序的崩溃。
* [Commit 4a4b](https://github.com/manticoresoftware/manticore/commit/4a4bda5) 修复了在多查询中使用池化属性时的崩溃。
* [Commit 3873](https://github.com/manticoresoftware/manticore/commit/3873bfb) 通过防止索引页面包含到核心文件中，修复了减少核心大小的问题。
* [Commit 11e6](https://github.com/manticoresoftware/manticore/commit/11e6254) 修复了在指定无效代理时 searchd 启动崩溃的问题。
* [Commit 4ca6](https://github.com/manticoresoftware/manticore/commit/4ca6350) 修复了索引器在 sql_query_killlist 查询中报告错误。
* [Commit 123a](https://github.com/manticoresoftware/manticore/commit/123a9f0) 修复了 fold_lemmas=1 与命中计数之间的冲突。
* [Commit cb99](https://github.com/manticoresoftware/manticore/commit/cb99164) 修复了 html_strip 的不一致行为。
* [Commit e406](https://github.com/manticoresoftware/manticore/commit/e406761) 修复了优化 rt 索引时丢失新设置的问题；修复了带有同步选项的优化锁泄漏。
* [Commit 86ae](https://github.com/manticoresoftware/manticore/commit/86aeb82) 修复了处理错误的多查询时的问题。
* [Commit 2645](https://github.com/manticoresoftware/manticore/commit/2645230) 修复了结果集依赖于多查询顺序的问题。
* [Commit 7239](https://github.com/manticoresoftware/manticore/commit/72395d9) 修复了在多查询中使用坏查询时的服务器崩溃。
* [Commit f353](https://github.com/manticoresoftware/manticore/commit/f353326) 修复了共享锁转为独占锁的问题。
* [Commit 3754](https://github.com/manticoresoftware/manticore/commit/3754785) 修复了无索引查询时的服务器崩溃。
* [Commit 29f3](https://github.com/manticoresoftware/manticore/commit/29f360e) 修复了服务器的死锁问题。

## Version 2.3.3, 2017年7月6日
* Manticore 品牌推广
