# 简介

Manticore Search 是一款面向搜索和分析场景打造的高性能多存储数据库，提供极速全文检索、实时索引，以及向量搜索和列式存储等高级功能，便于高效进行数据分析。它既能处理小规模数据集，也能应对大规模数据集，为现代应用提供无缝扩展能力和强大的洞察力。

作为一款开源数据库（可在 [GitHub](https://github.com/manticoresoftware/manticoresearch/) 获取），Manticore Search 于 2017 年创建，作为 [Sphinx Search](https://sphinxsearch.com/) 引擎的延续。我们的开发团队继承了 Sphinx 的所有最佳特性，并在此基础上大幅增强了功能，同时修复了数百个 bug（详见我们的 [Changelog](https://manual.manticoresearch.com/Changelog)）。Manticore Search 是一款现代、快速、轻量的数据库，拥有出色的全文检索能力，并且几乎是对其前身的彻底重写。

## Manticore 的核心特性包括：
#### 强大而快速的全文检索，适用于小型和大型数据集

  * [查询自动补全](Searching/Autocomplete.md)
  * [模糊搜索](Searching/Spell_correction.md#Fuzzy-Search)
  * 超过 20 种 [全文运算符](https://play.manticoresearch.com/fulltextintro/)<!--{target="_blank"}-->，以及超过 20 个排序因子
  * [自定义排序](Searching/Sorting_and_ranking.md#Ranking-overview)
  * [词干提取](Creating_a_table/NLP_and_tokenization/Morphology.md)
  * [词形还原](Creating_a_table/NLP_and_tokenization/Morphology.md)
  * [停用词](Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md)
  * [同义词](Creating_a_table/NLP_and_tokenization/Exceptions.md)
  * [词形映射](Creating_a_table/NLP_and_tokenization/Wordforms.md)
  * [字符级和词级高级分词](Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md)
  * [完善的中文分词](Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md)
  * [文本高亮](Searching/Highlighting.md)

#### 向量搜索能力
Manticore Search 支持将机器学习模型生成的 embedding 添加到每个文档中，然后对其执行 [最近邻搜索](Searching/KNN.md)。这使你可以构建相似度搜索、推荐、语义搜索，以及基于 NLP 算法的相关性排序等功能，还包括图像、视频和声音搜索等场景。

#### JOIN
Manticore Search 支持通过 SQL 和 JSON 执行 [JOIN](Searching/Joining.md) 查询，让你可以把多个表中的数据组合起来。

#### 多线程
Manticore Search 采用智能查询并行化，以降低响应时间，并在需要时充分利用所有 CPU 核心。

#### 基于代价的查询优化器
基于代价的查询优化器会使用关于已索引数据的统计信息，评估给定查询的不同执行计划的相对成本。这样优化器就能在综合考虑已索引数据规模、查询复杂度和可用资源等因素后，确定检索目标结果时最有效的执行方案。

#### 存储选项
Manticore 提供 [行式和列式存储选项](Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)，以适配不同规模的数据集。传统且默认的行式存储适用于各种规模的数据集 - 小型、中型和大型；而列式存储则通过 Manticore Columnar Library 提供，面向更大的数据集。这些存储方式的关键区别在于，行式存储为了获得最佳性能，需要将所有属性（全文字段除外）保留在 RAM 中，而列式存储则不需要，因此 RAM 占用更低，但性能可能会略慢一些（https://db-benchmarks.com/ 上的统计数据也证明了这一点）。

#### 自动二级索引
[Manticore Columnar Library](https://github.com/manticoresoftware/columnar/) 使用 [Piecewise Geometric Model index](https://github.com/gvinciguerra/PGM-index)，它利用索引键与其在内存中位置之间的学习映射。这种映射十分紧凑，再加上独特的递归构建算法，使 PGM-index 在空间占用上远超传统索引，同时仍能提供一流的查询和更新性能。所有数值字段和字符串字段默认都开启二级索引，json 属性也可以启用。

#### SQL 优先
Manticore 的原生语法是 SQL，并支持通过 HTTP 和 MySQL 协议使用 SQL，因此可以在任何编程语言中通过常见的 mysql 客户端进行连接。

#### 通过 HTTP 传输 JSON
对于更偏编程化的数据和 schema 管理方式，Manticore 提供 [HTTP JSON](Searching/Full_text_matching/Basic_usage.md#HTTP-JSON) 协议，类似 Elasticsearch 的方式。

#### 分片表
Manticore 支持 [`type='sharding'` 表](Creating_a_table/Creating_a_sharded_table/Creating_a_sharded_table.md)，可以在单节点或复制集群中透明地将读写分发到多个物理分片，从而提升写入扩展性，同时简化路由、故障切换和运维管理。

#### 身份验证与授权
Manticore 为 MySQL、HTTP/HTTPS、分布式远程代理以及复制相关操作提供内置的 [身份验证和授权](Security/Authentication_and_authorization.md)，支持用户、Bearer token 和细粒度权限控制。

#### 兼容 Elasticsearch 的写入
你可以执行与 Elasticsearch 兼容的 [insert](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Adding-documents-to-a-real-time-table) 和 [replace](Data_creation_and_modification/Updating_documents/REPLACE.md#REPLACE) JSON 查询，这样就能将 Manticore 与 Logstash（版本 < 7.13）、Filebeat 以及 Beats 家族中的其他工具一起使用。

#### 声明式与命令式 schema 管理
可在线或通过配置文件轻松创建、更新和删除表。

#### C++ 的优势与 PHP 的便捷性
Manticore Search 守护进程使用 C++ 开发，启动速度快，内存利用率高。底层优化进一步增强了性能。另一个关键组件 [Manticore Buddy](https://github.com/manticoresoftware/manticoresearch-buddy) 使用 PHP 编写，用于那些不需要闪电般响应速度或极高处理能力的高层功能。尽管为 C++ 代码做贡献可能更具挑战，但使用 Manticore Buddy 添加新的 SQL/JSON 命令应该是一个相对直接的过程。

#### 实时插入
新添加或更新的文档可以立即读取。

#### 轻松学习的交互式课程
我们提供 [免费的交互式课程](https://play.manticoresearch.com/)，让学习变得轻松无负担。

#### 事务
虽然 Manticore 并不完全符合 ACID，但它支持用于原子性变更的隔离事务，以及用于安全写入的二进制日志。

#### 内置复制与负载均衡
数据可以分布到各个服务器和数据中心，任意一个 Manticore Search 节点都可以同时作为负载均衡器和数据节点。Manticore 使用 [Galera library](https://play.manticoresearch.com/replication/) 实现几乎同步的多主 [复制](https://galeracluster.com/)，从而确保所有节点的数据一致性，防止数据丢失，并提供出色的复制性能。

#### 内置备份能力
Manticore 配备了外部工具 [manticore-backup](Securing_and_compacting_a_table/Backup_and_restore.md)，以及 [BACKUP](Securing_and_compacting_a_table/Backup_and_restore.md#BACKUP-SQL-command-reference) SQL 命令，以简化数据备份和恢复流程。或者，你也可以使用 [mysqldump](https://dev.mysql.com/doc/refman/8.0/en/mysqldump.html) 来 [制作逻辑备份](Securing_and_compacting_a_table/Backup_and_restore.md#Backup-and-restore-with-mysqldump)。

#### 开箱即用的数据同步
Manticore 的 `indexer` 工具和完整的配置语法，可以轻松同步来自 MySQL、PostgreSQL、ODBC 兼容数据库、XML 和 CSV 等来源的数据。

#### 集成选项
你可以通过 [FEDERATED engine](Extensions/FEDERATED.md) 或 [ProxySQL](https://manticoresearch.com/blog/using-proxysql-to-route-inserts-in-a-distributed-realtime-index/) 将 Manticore Search 与 MySQL/MariaDB 服务器集成。

你可以使用 [Apache Superset](https://manticoresearch.com/blog/manticoresearch-apache-superset-integration/) 和 [Grafana](https://manticoresearch.com/blog/manticoresearch-grafana-integration/) 可视化存储在 Manticore 中的数据。还可以使用多种 MySQL 工具交互式开发 Manticore 查询，例如 [HeidiSQL](https://www.heidisql.com/) 和 [DBForge](https://www.devart.com/dbforge/)。

你也可以将 Manticore Search 与 [Kibana](Integration/Kibana.md) 一起使用。

#### 轻松进行流过滤
Manticore 提供一种特殊的表类型，即“[percolate](Creating_a_table/Local_tables/Percolate_table.md)”表，它允许你搜索查询而不是数据，因此非常适合用于过滤全文数据流。只需把查询存入表中，将每批文档发送给 Manticore Search 处理数据流，就能只接收与已存查询匹配的结果。

#### 可能的应用场景
Manticore Search 用途广泛，可应用于多种场景，包括：

- **全文检索**：
  - 非常适合电商平台，借助自动补全和模糊搜索等功能，提供快速而准确的商品搜索。
  - 非常适合内容密集型网站，让用户能够快速找到相关的文章或文档。

- **数据分析**：
  - 使用 [Beats/Logstash](https://manticoresearch.com/blog/integration-of-manticore-with-logstash-filebeat/)、[Vector.dev](https://manticoresearch.com/blog/integration-of-manticore-with-vectordev/)、[Fluentbit](https://manticoresearch.com/blog/integration-of-manticore-with-fluentbit/) 将数据导入 Manticore Search。
  - 借助 Manticore 的列式存储和 OLAP 能力，高效分析大规模数据集。
  - 以极低延迟对 TB 级数据执行复杂查询。
  - 使用 Kibana、[Grafana](https://manticoresearch.com/blog/manticoresearch-grafana-integration/) 或 [Apache Superset](https://manticoresearch.com/blog/manticoresearch-apache-superset-integration/) 可视化数据。

- **分面搜索**：
  - 允许用户按价格、品牌或日期等类别筛选搜索结果，获得更精细的搜索体验。

- **地理空间搜索**：
  - 利用 Manticore 的地理空间能力实现基于位置的搜索，例如查找附近的餐厅或商店。

- **拼写纠错**：
  - 自动纠正用户在搜索查询中的拼写错误，以提升搜索准确性和用户体验。

- **自动补全**：
  - 在用户输入时实时提供建议，提升搜索可用性和速度。

- **数据流过滤**：
  - 使用 percolate 表高效过滤和处理实时数据流，例如社交媒体流或日志数据。


## 要求

* 架构：arm64 或 x86_64
* 操作系统：基于 Debian 的系统（例如 Debian、Ubuntu、Mint）、基于 RHEL 的系统（例如 RHEL、CentOS、Alma、Oracle Linux、Amazon Linux）、Windows 或 MacOS。
* [Manticore Columnar Library](https://github.com/manticoresoftware/columnar)，它提供 [列式存储](Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) 和 [二级索引](Introduction.md#Automatic-secondary-indexes)，要求 CPU 支持 SSE >= 4.2。
* 不需要特定的磁盘空间或 RAM 要求。一个空的 Manticore Search 实例只会占用大约 40MB 的 RSS 内存。

<!-- proofread -->

