# 介绍

测试更新！

Manticore Search 是一个高性能、多存储数据库，专为搜索和分析而设计，提供闪电般快速的全文搜索、实时索引以及向量搜索和列存储等高级功能，以实现高效的数据分析。它既适用于小型数据集，也能处理大型数据集，为现代应用提供无缝扩展和强大洞察。

作为一个开源数据库（可在 [GitHub](https://github.com/manticoresoftware/manticoresearch/) 获取），Manticore Search 于 2017 年创建，是 [Sphinx Search](https://sphinxsearch.com/) 引擎的延续。我们的开发团队将 Sphinx 的所有最佳功能整合并大幅提升其功能性，同时修复了数百个漏洞（详见我们的 [更新日志](https://manual.manticoresearch.com/Changelog)）。Manticore Search 是一个现代、快速且轻量级的数据库，拥有卓越的全文搜索能力，几乎是其前身的全面重写版本。

## Manticore 的主要特点：
#### 功能强大且快速的全文搜索，适用于小型和大型数据集

  * [查询自动补全](Searching/Autocomplete.md)
  * [模糊搜索](Searching/Spell_correction.md#Fuzzy-Search)
  * 超过 20 种 [全文操作符](https://play.manticoresearch.com/fulltextintro/)<!--{target="_blank"}--> 和 20 多种排序因子
  * [自定义排序](Searching/Sorting_and_ranking.md#Ranking-overview)
  * [词干提取](Creating_a_table/NLP_and_tokenization/Morphology.md)
  * [词形还原](Creating_a_table/NLP_and_tokenization/Morphology.md)
  * [停用词](Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md)
  * [同义词](Creating_a_table/NLP_and_tokenization/Exceptions.md)
  * [词形变化](Creating_a_table/NLP_and_tokenization/Wordforms.md)
  * [字符和单词级别的高级分词](Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md)
  * [正确的中文分词](Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md)
  * [文本高亮](Searching/Highlighting.md)

#### 向量搜索能力
Manticore Search 支持将由机器学习模型生成的嵌入向量添加到每条文档中，然后执行 [最近邻搜索](Searching/KNN.md)。这让您能构建相似度搜索、推荐系统、语义搜索以及基于 NLP 算法的相关性排序等功能，还包括图像、视频和声音搜索。

#### JOIN
Manticore Search 支持通过 SQL 和 JSON 执行 [JOIN](Searching/Joining.md) 查询，可将多个表中的数据组合起来。

#### 多线程
Manticore Search 利用智能查询并行化，降低响应时间，并在需要时充分利用所有 CPU 核心。

#### 基于成本的查询优化器
成本基查询优化器利用关于索引数据的统计信息，评估不同执行计划的相对成本，从而确定检索所需结果的最高效方案，考虑索引数据大小、查询复杂度及可用资源等因素。

#### 存储选项
Manticore 提供了 [行式和列式存储选项](Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)，以适应不同规模的数据集。传统且默认的行式存储适用于小、中、大型数据集，而列存储选项通过 Manticore 列存库为更大规模数据集提供支持。这两种存储的主要区别在于，行式存储要求所有属性（不包括全文字段）保存在内存中以获得最佳性能，而列式存储则不需要，因此内存占用更低，但性能可能略微降低（参见 https://db-benchmarks.com/ 上的统计数据）。

#### 自动二级索引
[Manticore 列存库](https://github.com/manticoresoftware/columnar/) 使用了 [分段几何模型索引](https://github.com/gvinciguerra/PGM-index)，该索引利用了索引键与内存位置之间的学习映射。映射的简洁性以及其特有的递归构造算法，使得 PGM 索引在空间效率上远超传统索引，同时保持最佳的查询和更新性能。二级索引对所有数值和字符串字段默认开启，并且可以为 JSON 属性启用。

#### SQL 优先
Manticore 的原生命令语法是 SQL，并支持基于 HTTP 的 SQL 以及 MySQL 协议，允许通过任何编程语言的流行 MySQL 客户端进行连接。

#### HTTP JSON
为了更程序化地管理数据和模式，Manticore 提供了类似于 Elasticsearch 的 [HTTP JSON](Searching/Full_text_matching/Basic_usage.md#HTTP-JSON) 协议。

#### 兼容 Elasticsearch 的写入
您可以执行兼容 Elasticsearch 的 [插入](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Adding-documents-to-a-real-time-table) 和 [替换](Data_creation_and_modification/Updating_documents/REPLACE.md#REPLACE) JSON 查询，从而使 Manticore 与 Logstash（版本 < 7.13）、Filebeat 及 Beats 家族的其他工具兼容。

#### 声明式和命令式的模式管理
轻松在线或通过配置文件创建、更新和删除表。

#### C++ 的性能优势与 PHP 的便捷性结合
Manticore Search 守护进程采用 C++ 开发，提供快速启动时间和高效的内存利用。低级优化进一步提升了性能。另一个关键组件称为 [Manticore Buddy](https://github.com/manticoresoftware/manticoresearch-buddy)，使用 PHP 编写，用于不需要极快响应速度或极高处理能力的高级功能。虽然参与 C++ 代码贡献可能具有挑战性，但使用 Manticore Buddy 添加新的 SQL/JSON 命令应该是一个简单的过程。

#### 实时插入
新添加或更新的文档可以立即被读取。

#### 交互式课程，轻松学习
我们提供 [免费交互式课程](https://play.manticoresearch.com/)，让学习变得轻而易举。

#### 事务
虽然 Manticore 并非完全遵守 ACID 标准，但支持隔离事务以实现原子性更改，并支持二进制日志以确保写入安全。

#### 内置复制与负载均衡
数据可以跨服务器和数据中心分布，任何 Manticore Search 节点既可作为负载均衡器又可作为数据节点。Manticore 使用 [Galera 库](https://galeracluster.com/) 实现几乎同步的多主 [复制](https://play.manticoresearch.com/replication/)，确保所有节点的数据一致性，防止数据丢失，并提供卓越的复制性能。

#### 内置备份功能
Manticore 配备了外部工具 [manticore-backup](Securing_and_compacting_a_table/Backup_and_restore.md) 和 [BACKUP](Securing_and_compacting_a_table/Backup_and_restore.md#BACKUP-SQL-command-reference) SQL 命令，简化备份和恢复数据的过程。或者，您可以使用 [mysqldump](https://dev.mysql.com/doc/refman/8.0/en/mysqldump.html) 来 [进行逻辑备份](Securing_and_compacting_a_table/Backup_and_restore.md#Backup-and-restore-with-mysqldump)。

#### 开箱即用的数据同步
`indexer` 工具和 Manticore 详尽的配置语法使其容易从 MySQL、PostgreSQL、ODBC 兼容数据库、XML 和 CSV 等源同步数据。

#### 集成选项
您可以使用 [FEDERATED 引擎](Extensions/FEDERATED.md) 或通过 [ProxySQL](https://manticoresearch.com/blog/using-proxysql-to-route-inserts-in-a-distributed-realtime-index/) 将 Manticore Search 与 MySQL/MariaDB 服务器集成。

您可以使用 [Apache Superset](https://manticoresearch.com/blog/manticoresearch-apache-superset-integration/)、Kibana 和 [Grafana](https://manticoresearch.com/blog/manticoresearch-grafana-integration/) 来可视化存储在 Manticore 中的数据。多种 MySQL 工具可用于交互式开发 Manticore 查询，如 [HeidiSQL](https://www.heidisql.com/) 和 [DBForge](https://www.devart.com/dbforge/)。

您还可以将 Manticore Search 用于 [Kibana](Integration/Kibana.md) 集成。

#### 流过滤简便实现
Manticore 提供一种特殊的表类型，即“[percolate](Creating_a_table/Local_tables/Percolate_table.md)”表，允许搜索查询而非数据，使其成为过滤全文数据流的高效工具。只需将您的查询存储在该表中，通过向 Manticore Search 发送每批文档处理数据流，便可仅接收与您存储的查询匹配的结果。

#### 可能的应用场景
Manticore Search 功能多样，可应用于多种场景，包括：

- **全文搜索**：
  - 适用于电商平台，支持快速精准的产品搜索，具备自动补全和模糊搜索功能。
  - 非常适合内容丰富的网站，让用户快速找到相关文章或文档。

- **数据分析**：
  - 通过 [Beats/Logstash](https://manticoresearch.com/blog/integration-of-manticore-with-logstash-filebeat/)、[Vector.dev](https://manticoresearch.com/blog/integration-of-manticore-with-vectordev/)、[Fluentbit](https://manticoresearch.com/blog/integration-of-manticore-with-fluentbit/) 将数据导入 Manticore Search。
  - 利用 Manticore 的列存储和 OLAP 功能高效分析大数据集。
  - 在处理数 TB 数据时执行复杂查询且延迟极低。
  - 使用 Kibana、[Grafana](https://manticoresearch.com/blog/manticoresearch-grafana-integration/) 或 [Apache Superset](https://manticoresearch.com/blog/manticoresearch-apache-superset-integration/) 可视化数据。

- **分面搜索**：
  - 允许用户按类别筛选搜索结果，如价格、品牌或日期，以实现更精准的搜索体验。

- **地理空间搜索**：
  - 实现基于位置的搜索，例如寻找附近的餐馆或商店，利用 Manticore 的地理空间功能。

- **拼写纠正**：
  - 自动纠正用户搜索查询中的拼写错误，提高搜索准确性和用户体验。

- **自动补全**：
  - 在用户输入时提供实时建议，增强搜索的易用性和速度。

- **数据流过滤**：
  - 使用 percolate 表过滤和处理实时数据流，如社交媒体信息流或日志数据，高效便捷。


## 要求

* 架构：arm64 或 x86_64
* 操作系统：基于 Debian 的系统（如 Debian、Ubuntu、Mint），基于 RHEL 的系统（如 RHEL、CentOS、Alma、Oracle Linux、Amazon Linux）、Windows 或 MacOS。
* [Manticore Columnar Library](https://github.com/manticoresoftware/columnar) 提供 [列式存储](Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) 和 [二级索引](Introduction.md#Automatic-secondary-indexes)，需要支持 SSE >= 4.2 的 CPU。
* 无具体磁盘空间或内存要求。空的 Manticore Search 实例仅占用约 40MB 的 RSS 内存。

<!-- proofread -->

