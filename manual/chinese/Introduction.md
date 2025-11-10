# 介绍

Manticore Search 是一个高性能、多存储的数据库，专为搜索和分析而设计，提供极速的全文搜索、实时索引以及向量搜索和列存储等高级功能，以实现高效的数据分析。它既能处理小型数据集，也能应对大型数据集，提供无缝的可扩展性和强大的洞察力，适用于现代应用。

作为一个开源数据库（可在 [GitHub](https://github.com/manticoresoftware/manticoresearch/) 获取），Manticore Search 于 2017 年创建，是 [Sphinx Search](https://sphinxsearch.com/) 引擎的延续。我们的开发团队吸取了 Sphinx 的所有优点，并显著提升了其功能，修复了数百个漏洞（详见我们的 [更新日志](https://manual.manticoresearch.com/Changelog)）。Manticore Search 是一个现代、快速且轻量级的数据库，具备卓越的全文搜索能力，基于对其前身的几乎完全重写构建。

## Manticore 的主要特点包括：
#### 强大且快速的全文搜索，适用于小型和大型数据集

  * [查询自动补全](Searching/Autocomplete.md)
  * [模糊搜索](Searching/Spell_correction.md#Fuzzy-Search)
  * 超过 20 种 [全文操作符](https://play.manticoresearch.com/fulltextintro/)<!--{target="_blank"}--> 和超过 20 种排名因素
  * [自定义排名](Searching/Sorting_and_ranking.md#Ranking-overview)
  * [词干提取](Creating_a_table/NLP_and_tokenization/Morphology.md)
  * [词形还原](Creating_a_table/NLP_and_tokenization/Morphology.md)
  * [停用词](Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md)
  * [同义词](Creating_a_table/NLP_and_tokenization/Exceptions.md)
  * [词形变化](Creating_a_table/NLP_and_tokenization/Wordforms.md)
  * [字符和词级别的高级分词](Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md)
  * [准确的中文分词](Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md)
  * [文本高亮](Searching/Highlighting.md)

#### 向量搜索功能
Manticore Search 支持将机器学习模型生成的嵌入向量添加到每个文档中，然后对其进行[最近邻搜索](Searching/KNN.md)。这使您能够构建相似度搜索、推荐、语义搜索和基于自然语言处理算法的相关性排名等功能，还包括图像、视频和声音搜索。

#### JOIN
Manticore Search 支持通过 SQL 和 JSON 进行的 [JOIN](Searching/Joining.md) 查询，允许您合并多个表的数据。

#### 多线程
Manticore Search 利用智能查询并行化技术，降低响应时间，并在需要时充分利用所有 CPU 核心。

#### 基于成本的查询优化器
基于成本的查询优化器使用关于索引数据的统计信息，评估给定查询不同执行计划的相对成本。这样，优化器能够确定检索所需结果的最高效计划，考虑因素包括索引数据的大小、查询的复杂性和可用资源。

#### 存储选项
Manticore 提供[行存储和列存储选项](Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)，以适应不同规模的数据集。传统且默认的行存储适用于所有规模的数据集——小型、中型和大型，而列存储选项通过 Manticore Columnar Library 提供，适合更大规模的数据集。这两种存储选项的关键区别在于，行存储要求所有属性（全文字段除外）保存在内存中以获得最佳性能，而列存储则不需要，因此内存消耗更低，但性能可能略有下降（详见 https://db-benchmarks.com/ 上的统计数据）。

#### 自动二级索引
[Manticore Columnar Library](https://github.com/manticoresoftware/columnar/) 使用 [Piecewise Geometric Model 索引](https://github.com/gvinciguerra/PGM-index)，该索引利用索引键与其内存位置之间的学习映射。该映射的简洁性及其独特的递归构造算法，使 PGM 索引在空间上远超传统索引，同时仍提供最佳的查询和更新性能。二级索引默认对所有数值和字符串字段开启，并可对 json 属性启用。

#### SQL 优先
Manticore 的原生语法是 SQL，支持通过 HTTP 和 MySQL 协议的 SQL，允许通过任何编程语言中的流行 mysql 客户端连接。

#### HTTP JSON
为了更程序化地管理数据和模式，Manticore 提供类似 Elasticsearch 的 [HTTP JSON](Searching/Full_text_matching/Basic_usage.md#HTTP-JSON) 协议。

#### 兼容 Elasticsearch 的写入
您可以执行兼容 Elasticsearch 的 [插入](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Adding-documents-to-a-real-time-table) 和 [替换](Data_creation_and_modification/Updating_documents/REPLACE.md#REPLACE) JSON 查询，从而使 Manticore 能与 Logstash（版本 < 7.13）、Filebeat 及 Beats 家族的其他工具配合使用。

#### 声明式和命令式的模式管理
轻松在线或通过配置文件创建、更新和删除表。

#### C++ 的优势与 PHP 的便利性
Manticore Search 守护进程采用 C++ 开发，提供快速启动时间和高效的内存利用率。底层优化进一步提升了性能。另一个关键组件称为 [Manticore Buddy](https://github.com/manticoresoftware/manticoresearch-buddy)，用 PHP 编写，用于不需要极快响应时间或极高处理能力的高级功能。虽然贡献 C++ 代码可能具有挑战性，但使用 Manticore Buddy 添加新的 SQL/JSON 命令应该是一个简单的过程。

#### 实时插入
新添加或更新的文档可以立即读取。

#### 交互式课程，轻松学习
我们提供[免费交互式课程](https://play.manticoresearch.com/)，让学习变得轻松。

#### 事务
虽然 Manticore 并非完全符合 ACID 标准，但支持隔离事务以实现原子更改，并支持二进制日志以保证写入安全。

#### 内置复制和负载均衡
数据可以分布在服务器和数据中心中，任何 Manticore Search 节点都可以同时作为负载均衡器和数据节点。Manticore 使用 [Galera 库](https://galeracluster.com/) 实现几乎同步的多主[复制](https://play.manticoresearch.com/replication/)，确保所有节点数据一致，防止数据丢失，并提供卓越的复制性能。

#### 内置备份功能
Manticore 配备了外部工具 [manticore-backup](Securing_and_compacting_a_table/Backup_and_restore.md) 和 [BACKUP](Securing_and_compacting_a_table/Backup_and_restore.md#BACKUP-SQL-command-reference) SQL 命令，简化数据备份和恢复过程。或者，您也可以使用 [mysqldump](https://dev.mysql.com/doc/refman/8.0/en/mysqldump.html) 来[制作逻辑备份](Securing_and_compacting_a_table/Backup_and_restore.md#Backup-and-restore-with-mysqldump)。

#### 开箱即用的数据同步
`indexer` 工具和 Manticore 的全面配置语法使得从 MySQL、PostgreSQL、兼容 ODBC 的数据库、XML 和 CSV 等源同步数据变得轻松。

#### 集成选项
您可以使用 [FEDERATED 引擎](Extensions/FEDERATED.md) 或通过 [ProxySQL](https://manticoresearch.com/blog/using-proxysql-to-route-inserts-in-a-distributed-realtime-index/) 将 Manticore Search 与 MySQL/MariaDB 服务器集成。

您可以使用 [Apache Superset](https://manticoresearch.com/blog/manticoresearch-apache-superset-integration/) 和 [Grafana](https://manticoresearch.com/blog/manticoresearch-grafana-integration/) 来可视化存储在 Manticore 中的数据。各种 MySQL 工具可用于交互式开发 Manticore 查询，如 [HeidiSQL](https://www.heidisql.com/) 和 [DBForge](https://www.devart.com/dbforge/)。

您还可以将 Manticore Search 与 [Kibana](Integration/Kibana.md) 一起使用。

#### 流过滤轻松实现
Manticore 提供了一种特殊的表类型，“[percolate](Creating_a_table/Local_tables/Percolate_table.md)”表，允许您搜索查询而非数据，使其成为过滤全文数据流的高效工具。只需将查询存储在表中，通过将每批文档发送到 Manticore Search 来处理数据流，并仅接收与存储查询匹配的结果。

#### 可能的应用
Manticore Search 多功能，可应用于多种场景，包括：

- **全文搜索**：
  - 适用于电子商务平台，实现快速准确的产品搜索，具备自动补全和模糊搜索等功能。
  - 适合内容丰富的网站，允许用户快速找到相关的文章或文档。

- **数据分析**：
  - 使用 [Beats/Logstash](https://manticoresearch.com/blog/integration-of-manticore-with-logstash-filebeat/)、[Vector.dev](https://manticoresearch.com/blog/integration-of-manticore-with-vectordev/)、[Fluentbit](https://manticoresearch.com/blog/integration-of-manticore-with-fluentbit/) 将数据导入 Manticore Search。
  - 利用 Manticore 的列存储和 OLAP 功能高效分析大数据集。
  - 对数 TB 数据执行复杂查询，延迟极低。
  - 使用 Kibana、[Grafana](https://manticoresearch.com/blog/manticoresearch-grafana-integration/) 或 [Apache Superset](https://manticoresearch.com/blog/manticoresearch-apache-superset-integration/) 进行数据可视化。

- **分面搜索**：
  - 允许用户按类别（如价格、品牌或日期）过滤搜索结果，提供更精细的搜索体验。

- **地理空间搜索**：
  - 利用 Manticore 的地理空间功能实现基于位置的搜索，如查找附近的餐厅或商店。

- **拼写纠正**：
  - 自动纠正用户搜索查询中的拼写错误，提高搜索准确性和用户体验。

- **自动补全**：
  - 在用户输入时提供实时建议，提升搜索的易用性和速度。

- **数据流过滤**：
  - 使用 percolate 表高效过滤和处理实时数据流，如社交媒体信息流或日志数据。

## 要求

* 架构：arm64 或 x86_64
* 操作系统：基于 Debian（如 Debian、Ubuntu、Mint）、基于 RHEL（如 RHEL、CentOS、Alma、Oracle Linux、Amazon Linux）、Windows 或 MacOS。
* [Manticore Columnar Library](https://github.com/manticoresoftware/columnar)，提供[列存储](Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)和[二级索引](Introduction.md#Automatic-secondary-indexes)，需要支持 SSE >= 4.2 的 CPU。
* 无特定磁盘空间或内存要求。空的 Manticore Search 实例仅使用约 40MB 的 RSS 内存。

<!-- proofread -->

