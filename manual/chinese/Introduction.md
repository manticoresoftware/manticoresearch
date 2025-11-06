# 介绍


作为一个开源数据库（可在 [GitHub](https://github.com/manticoresoftware/manticoresearch/) 获取），Manticore Search 于 2017 年创建，作为 [Sphinx Search](https://sphinxsearch.com/) 引擎的继承项目。我们的开发团队采纳了 Sphinx 的所有最佳功能，并显著提升了其功能性，同时修复了数百个漏洞（详见我们的[更新日志](https://manual.manticoresearch.com/Changelog)）。Manticore Search 是一个现代化、快速且轻量级的数据库，具备卓越的全文搜索能力，基于对其前身几乎完全重写的代码构建。

## Manticore 的主要特点包括：
#### 强大且快速的全文搜索，适用于小型和大型数据集

  * [查询自动完成](Searching/Autocomplete.md)

  * [模糊搜索](Searching/Spell_correction.md#Fuzzy-Search)
  * 超过 20 种 [全文操作符](https://play.manticoresearch.com/fulltextintro/)<!--{target="_blank"}--> 和 20 多种排序因子
  * [自定义排序](Searching/Sorting_and_ranking.md#Ranking-overview)
  * [词干提取](Creating_a_table/NLP_and_tokenization/Morphology.md)
  * [词形还原](Creating_a_table/NLP_and_tokenization/Morphology.md)
  * [停用词](Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md)
  * [同义词](Creating_a_table/NLP_and_tokenization/Exceptions.md)
  * [词形变化](Creating_a_table/NLP_and_tokenization/Wordforms.md)
  * [基于字符和词的高级分词](Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md)
  * [精准的中文分词](Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md)
  * [文本高亮](Searching/Highlighting.md)

#### 向量搜索能力
Manticore Search 支持为每条文档添加由机器学习模型生成的嵌入向量，并执行对它们的[最近邻搜索](Searching/KNN.md)。这让您可以构建相似度搜索、推荐、语义搜索和基于 NLP 算法的相关性排序等功能，还支持图像、视频和声音搜索等。

#### 连接查询（JOIN）
Manticore Search 通过 SQL 和 JSON 支持[连接查询](Searching/Joining.md)，允许您组合来自多张表的数据。

#### 多线程
Manticore Search 利用智能查询并行化，降低响应时间，并在需要时充分利用所有 CPU 核心。

#### 基于成本的查询优化器
基于成本的查询优化器使用已索引数据的统计信息，评估不同执行计划在运行同一查询时的相对成本。这样优化器可以确定检索目标结果的最高效方案，考虑因素包括索引数据的大小、查询复杂度和可用资源。

#### 存储选项
Manticore 提供了[行存和列存存储选项](Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)，以适应不同规模的数据集。传统且默认的行存选项适用于所有大小的数据集——小型、中型和大型；而通过 Manticore 列存库提供的列式存储选项适合更大的数据集。这两种存储方式的关键区别是，行存为达到最佳性能，所有属性（不包括全文字段）都需保存在内存中，而列存则不需要，占用更少内存，但性能可能略有下降（具体见 https://db-benchmarks.com/ 的统计数据）。

#### 自动二级索引
[Manticore 列存库](https://github.com/manticoresoftware/columnar/) 使用了[分段几何模型索引（PGM-index）](https://github.com/gvinciguerra/PGM-index)，该模型利用已学习的映射关系，将索引键映射到内存位置。此映射的简洁性与一种特殊递归构建算法结合，使 PGM 索引在空间上比传统索引节省多个数量级，同时提供最佳的查询和更新性能。所有数值和字符串字段默认启用二级索引，且可对 JSON 属性启用。

#### SQL 优先
Manticore 原生语法为 SQL，支持通过 HTTP 和 MySQL 协议执行 SQL 查询，允许使用常见 mysql 客户端用任何编程语言连接。

#### 基于 HTTP 的 JSON 接口
为了更程序化地管理数据和模式，Manticore 提供了类似于 Elasticsearch 的 [HTTP JSON](Searching/Full_text_matching/Basic_usage.md#HTTP-JSON) 协议。

#### 兼容 Elasticsearch 的写入
您可以执行兼容 Elasticsearch 的 [insert](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Adding-documents-to-a-real-time-table) 和 [replace](Data_creation_and_modification/Updating_documents/REPLACE.md#REPLACE) JSON 查询，从而将 Manticore 与 Logstash（版本 < 7.13）、Filebeat 及 Beats 家族的其它工具结合使用。

#### 声明式和命令式的模式管理
可以轻松在在线或配置文件中创建、更新和删除表。

#### C++ 的性能优势与 PHP 的便利性
Manticore Search 守护进程采用 C++ 开发，提供快速启动时间和高效的内存利用率。底层优化进一步提升性能。另一个关键组件称为 [Manticore Buddy](https://github.com/manticoresoftware/manticoresearch-buddy)，采用 PHP 编写，用于提供不需要极快响应时间或极高处理能力的高级功能。尽管参与 C++ 代码贡献可能具有挑战性，但使用 Manticore Buddy 添加新的 SQL/JSON 命令应该是一个简单的过程。

#### 实时插入
新增或更新的文档可以立即读取。

#### 互动课程，轻松学习
我们提供[免费互动课程](https://play.manticoresearch.com/)，使学习变得轻松。

#### 事务
虽然 Manticore 并非完全支持 ACID，但它支持隔离事务用于原子更改，并支持二进制日志以确保安全写入。

#### 内置复制和负载均衡
数据可以分布在服务器和数据中心中，任何 Manticore Search 节点都可以同时作为负载均衡器和数据节点。Manticore 使用 [Galera 库](https://galeracluster.com/) 实现准同步多主 [复制](https://play.manticoresearch.com/replication/)，确保所有节点之间的数据一致性，防止数据丢失，并提供卓越的复制性能。

#### 内置备份功能
Manticore 配备了外部工具 [manticore-backup](Securing_and_compacting_a_table/Backup_and_restore.md) 和 [BACKUP](Securing_and_compacting_a_table/Backup_and_restore.md#BACKUP-SQL-command-reference) SQL 命令，简化数据备份和恢复过程。或者，您也可以使用 [mysqldump](https://dev.mysql.com/doc/refman/8.0/en/mysqldump.html) 进行[逻辑备份](Securing_and_compacting_a_table/Backup_and_restore.md#Backup-and-restore-with-mysqldump)。

#### 开箱即用的数据同步
`indexer` 工具和 Manticore 完整的配置语法使从 MySQL、PostgreSQL、ODBC 兼容数据库、XML 及 CSV 等源同步数据变得轻松。

#### 集成选项
您可以通过 [FEDERATED 引擎](Extensions/FEDERATED.md) 或 [ProxySQL](https://manticoresearch.com/blog/using-proxysql-to-route-inserts-in-a-distributed-realtime-index/) 将 Manticore Search 集成到 MySQL/MariaDB 服务器。

您可以使用 [Apache Superset](https://manticoresearch.com/blog/manticoresearch-apache-superset-integration/) 和 [Grafana](https://manticoresearch.com/blog/manticoresearch-grafana-integration/) 来可视化存储在 Manticore 中的数据。各种 MySQL 工具可用于交互式开发 Manticore 查询，例如 [HeidiSQL](https://www.heidisql.com/) 和 [DBForge](https://www.devart.com/dbforge/)。
您可以使用 [Apache Superset](https://manticoresearch.com/blog/manticoresearch-apache-superset-integration/) 和 [Grafana](https://manticoresearch.com/blog/manticoresearch-grafana-integration/) 来可视化存储在 Manticore 中的数据。各种 MySQL 工具可用于交互式开发 Manticore 查询，比如 [HeidiSQL](https://www.heidisql.com/) 和 [DBForge](https://www.devart.com/dbforge/)。

您也可以将 Manticore Search 与 [Kibana](Integration/Kibana.md) 一起使用。

Manticore 提供一种特殊类型的表，“[percolate](Creating_a_table/Local_tables/Percolate_table.md)”表，允许您搜索查询而非数据，使其成为过滤全文数据流的高效工具。只需将查询存储在表中，通过向 Manticore Search 发送每批文档处理数据流，并接收仅匹配存储查询的结果。

#### 可能的应用场景
Manticore Search 功能多样，可应用于各种场景，包括：

- **全文搜索**：
  - 适合电商平台，实现快速且精准的产品搜索，支持自动补全和模糊搜索等功能。
  - 适合内容丰富的网站，允许用户快速找到相关的文章或文档。

- **数据分析**：
  - 使用 [Beats/Logstash](https://manticoresearch.com/blog/integration-of-manticore-with-logstash-filebeat/)、[Vector.dev](https://manticoresearch.com/blog/integration-of-manticore-with-vectordev/) 和 [Fluentbit](https://manticoresearch.com/blog/integration-of-manticore-with-fluentbit/) 将数据导入 Manticore Search。
  - 使用 Manticore 的列式存储和 OLAP 能力高效分析大型数据集。
  - 在海量数据上以极低延迟执行复杂查询。
  - 使用 Kibana、[Grafana](https://manticoresearch.com/blog/manticoresearch-grafana-integration/) 或 [Apache Superset](https://manticoresearch.com/blog/manticoresearch-apache-superset-integration/) 进行数据可视化。

- **分面搜索**：
  - 允许用户按类别（如价格、品牌、日期）筛选搜索结果，实现更细化的搜索体验。

- **地理空间搜索**：
  - 实现基于位置的搜索，如查找附近的餐厅或商店，利用 Manticore 的地理空间能力。

- **拼写纠正**：
  - 自动纠正用户搜索查询中的拼写错误，提高搜索准确性和用户体验。

- **自动补全**：
  - 提供实时建议，增强搜索的易用性和速度。

- **数据流过滤**：
  - 使用 percolate 表过滤和处理实时数据流，如社交媒体动态或日志数据，高效准确。


## 要求

* 架构：arm64 或 x86_64
* 操作系统：基于 Debian（例如 Debian、Ubuntu、Mint）、基于 RHEL（例如 RHEL、CentOS、Alma、Oracle Linux、Amazon Linux）、Windows 或 MacOS。
* [Manticore Columnar Library](https://github.com/manticoresoftware/columnar)，提供[列式存储](Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)和[二级索引](Introduction.md#Automatic-secondary-indexes)，需要支持 SSE >= 4.2 的 CPU。
* 无特殊磁盘空间或内存需求。空的 Manticore Search 实例仅使用约 40MB RSS 内存。

<!-- proofread -->

