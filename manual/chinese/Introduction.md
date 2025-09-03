# 介绍

Manticore Search 是一个高性能、多存储的数据库，专为搜索和分析而设计，提供极速的全文搜索、实时索引以及高级功能，如向量搜索和列存储，以实现高效的数据分析。它设计用来处理小型和大型数据集，提供无缝的可扩展性和强大的现代应用洞察力。

作为一个开源数据库（可在[GitHub](https://github.com/manticoresoftware/manticoresearch/)获得），Manticore Search 于2017年创建，作为[Sphinx Search](https://sphinxsearch.com/)引擎的延续。我们的开发团队吸取了Sphinx的所有优点，并大幅提升其功能，同时修复了数百个BUG（详见我们的[更新日志](https://manual.manticoresearch.com/Changelog)）。Manticore Search 是一个现代、快速且轻量级的数据库，具备卓越的全文搜索能力，几乎是其前身的重写版本。

## Manticore 的主要特点有：
#### 强大且快速的全文搜索，适用于小型和大型数据集

  * [查询自动补全](Searching/Autocomplete.md)
  * [模糊搜索](Searching/Spell_correction.md#Fuzzy-Search)
  * 超过20个[全文操作符](https://play.manticoresearch.com/fulltextintro/)<!--{target="_blank"}--> 和20多个排序因子
  * [自定义排序](Searching/Sorting_and_ranking.md#Ranking-overview)
  * [词干提取](Creating_a_table/NLP_and_tokenization/Morphology.md)
  * [词形还原](Creating_a_table/NLP_and_tokenization/Morphology.md)
  * [停用词](Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md)
  * [同义词](Creating_a_table/NLP_and_tokenization/Exceptions.md)
  * [词型变换](Creating_a_table/NLP_and_tokenization/Wordforms.md)
  * [字符和词级别的高级分词](Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md)
  * [准确的中文分词](Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md)
  * [文本高亮](Searching/Highlighting.md)

#### 向量搜索功能
Manticore Search 支持将机器学习模型生成的嵌入向量添加到每个文档中，然后对它们执行[最近邻搜索](Searching/KNN.md)。这让你能够构建相似度搜索、推荐、语义搜索以及基于NLP算法的相关性排序等功能，还包括图像、视频和声音搜索。

#### JOIN
Manticore Search 支持通过 SQL 和 JSON 执行[JOIN](Searching/Joining.md)查询，允许将来自多个表的数据合并。

#### 多线程
Manticore Search 利用智能的查询并行化技术，降低响应时间，并在需要时充分利用所有CPU核心。

#### 基于成本的查询优化器
基于成本的查询优化器使用有关索引数据的统计信息，评估给定查询的不同执行计划的相对成本。这样，优化器可以确定检索期望结果的最高效计划，考虑数据索引大小、查询复杂度及可用资源等因素。

#### 存储选项
Manticore 提供[行存和列存两种存储选项](Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)，适应不同规模的数据集。传统且默认的行存选项适用于小型、中型和大型数据集，而通过 Manticore 列存库提供的列存选项适合更大的数据集。这两种存储选项的关键区别在于，行存需要所有属性（全文字段除外）保持在内存中以获得最佳性能，而列存则不需要，因而内存消耗更低，但可能导致性能略微下降（详见 https://db-benchmarks.com/ 上的统计数据）。

#### 自动辅助索引
[Manticore 列存库](https://github.com/manticoresoftware/columnar/)使用[分段几何模型索引](https://github.com/gvinciguerra/PGM-index)，它利用了被索引键和它们内存位置之间的学习映射。这种映射的简洁性，结合独特的递归构造算法，使PGM索引成为一种数据结构，在空间使用上远远优于传统索引，同时提供最佳的查询和更新性能。辅助索引默认开启，适用于所有数值和字符串字段，并可为json属性启用。

#### SQL优先
Manticore 的本地语法是 SQL，支持基于HTTP和MySQL协议的SQL查询，允许通过任何编程语言的流行mysql客户端连接。

#### 基于HTTP的JSON
为了更程序化地管理数据和模式，Manticore 提供类似Elasticsearch的[HTTP JSON](Searching/Full_text_matching/Basic_usage.md#HTTP-JSON)协议。

#### Elasticsearch兼容写入
你可以执行兼容Elasticsearch的[插入](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Adding-documents-to-a-real-time-table)和[替换](Data_creation_and_modification/Updating_documents/REPLACE.md#REPLACE) JSON查询，从而使Manticore能配合诸如Logstash（版本 < 7.13）、Filebeat及Beats家族的其他工具使用。

#### 声明式和命令式的模式管理
可轻松在线或通过配置文件创建、更新和删除表。

#### C++的效能与PHP的便利结合
Manticore Search 守护进程采用 C++ 开发，提供快速启动时间和高效的内存利用率。低级别的优化进一步提升了性能。另一个重要组件称为 [Manticore Buddy](https://github.com/manticoresoftware/manticoresearch-buddy)，使用 PHP 编写，适用于不需要极快响应时间或极高处理能力的高级功能。虽然为 C++ 代码做出贡献可能具有挑战性，但使用 Manticore Buddy 添加新的 SQL/JSON 命令应该是一个简单的过程。

#### 实时插入
新增或更新的文档可以立即读取。

#### 轻松学习的交互式课程
我们提供[免费交互课程](https://play.manticoresearch.com/)，助您轻松学习。

#### 事务
虽然 Manticore 并非完全符合 ACID 标准，但它支持隔离事务以实现原子性变更，并支持二进制日志以保证写入安全。

#### 内置复制和负载均衡
数据可以分布在多个服务器和数据中心，任意 Manticore Search 节点既可以作为负载均衡器，也可以作为数据节点。Manticore 使用 [Galera 库](https://galeracluster.com/) 实现了几乎同步的多主 [复制](https://play.manticoresearch.com/replication/)，确保所有节点间数据一致，防止数据丢失，并提供卓越的复制性能。

#### 内置备份功能
Manticore 配备了外部工具 [manticore-backup](Securing_and_compacting_a_table/Backup_and_restore.md) 和 [BACKUP](Securing_and_compacting_a_table/Backup_and_restore.md#BACKUP-SQL-command-reference) SQL 命令，简化备份和恢复过程。您也可以使用 [mysqldump](https://dev.mysql.com/doc/refman/8.0/en/mysqldump.html) 来[制作逻辑备份](Securing_and_compacting_a_table/Backup_and_restore.md#Backup-and-restore-with-mysqldump)。

#### 开箱即用的数据同步
`indexer` 工具和 Manticore 完善的配置语法使得从 MySQL、PostgreSQL、ODBC 兼容数据库、XML 及 CSV 等源同步数据变得容易。

#### 集成选项
您可以使用 [FEDERATED 引擎](Extensions/FEDERATED.md) 或通过 [ProxySQL](https://manticoresearch.com/blog/using-proxysql-to-route-inserts-in-a-distributed-realtime-index/) 将 Manticore Search 集成到 MySQL/MariaDB 服务器中。

您可以使用 [Apache Superset](https://manticoresearch.com/blog/manticoresearch-apache-superset-integration/)、Kibana 和 [Grafana](https://manticoresearch.com/blog/manticoresearch-grafana-integration/) 来可视化存储在 Manticore 中的数据。可使用各种 MySQL 工具来交互式开发 Manticore 查询，如 [HeidiSQL](https://www.heidisql.com/) 和 [DBForge](https://www.devart.com/dbforge/)。

您可以将 Manticore Search 与 [Kibana](Integration/Kibana.md) 结合使用。

#### 流过滤轻松实现
Manticore 提供一种特殊的表类型，称为“[percolate](Creating_a_table/Local_tables/Percolate_table.md)” 表，它允许您搜索查询而非数据，使其成为过滤全文数据流的高效工具。只需将查询存储到表中，处理您的数据流，将每批文档发送给 Manticore Search，并仅接收与存储查询匹配的结果。

#### 可能的应用
Manticore Search 用途广泛，可应用于多种场景，包括：

- **全文搜索**：
  - 适用于电商平台，支持快速准确的商品搜索，具有自动补全和模糊搜索等功能。
  - 适合内容丰富的网站，帮助用户快速查找相关文章或文档。

- **数据分析**：
  - 使用 [Beats/Logstash](https://manticoresearch.com/blog/integration-of-manticore-with-logstash-filebeat/)、[Vector.dev](https://manticoresearch.com/blog/integration-of-manticore-with-vectordev/)、[Fluentbit](https://manticoresearch.com/blog/integration-of-manticore-with-fluentbit/) 将数据导入 Manticore Search。
  - 利用 Manticore 的列存储和 OLAP 能力高效分析大规模数据集。
  - 对 TB 级数据执行复杂查询，延迟极低。
  - 使用 Kibana、[Grafana](https://manticoresearch.com/blog/manticoresearch-grafana-integration/) 或 [Apache Superset](https://manticoresearch.com/blog/manticoresearch-apache-superset-integration/) 可视化数据。

- **分面搜索**：
  - 允许用户通过价格、品牌或日期等类别过滤搜索结果，提供更精细的搜索体验。

- **地理空间搜索**：
  - 利用 Manticore 的地理空间功能实现基于位置的搜索，如查找附近餐馆或商店。

- **拼写纠正**：
  - 自动纠正搜索查询中的用户错别字，提高搜索准确性和用户体验。

- **自动补全**：
  - 在用户输入时提供实时建议，增强搜索的可用性和速度。

- **数据流过滤**：
  - 使用 percolate 表有效过滤和处理实时数据流，如社交媒体动态或日志数据。


## 要求

* 架构：arm64 或 x86_64
* 操作系统：基于 Debian（如 Debian、Ubuntu、Mint）、基于 RHEL（如 RHEL、CentOS、Alma、Oracle Linux、Amazon Linux）、Windows 或 MacOS。
* [Manticore Columnar Library](https://github.com/manticoresoftware/columnar)，提供[列存储](Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)和[二级索引](Introduction.md#Automatic-secondary-indexes)，需要支持 SSE >= 4.2 的 CPU。
* 无具体磁盘空间或内存要求。空的 Manticore Search 实例仅使用约 40MB 的 RSS 内存。

<!-- proofread -->

