# 介绍

<<<<<<< HEAD
测试 1

=======
>>>>>>> 4c284d8fb (chore(docs): remove test placeholders from introduction files)
Manticore Search 是一个高性能、多存储的数据库，专为搜索和分析而设计，提供闪电般快速的全文搜索、实时索引以及矢量搜索和列存储等高级功能，以实现高效的数据分析。它旨在处理小型和大型数据集，提供无缝的可扩展性和强大的洞察力，满足现代应用的需求。

作为一个开源数据库（可在 [GitHub](https://github.com/manticoresoftware/manticoresearch/) 上获取），Manticore Search 创建于2017年，作为 [Sphinx Search](https://sphinxsearch.com/) 引擎的延续。我们的开发团队吸收了 Sphinx 的所有优良特性，并显著提升了其功能，同时修复了数百个漏洞（详见我们的 [更新日志](https://manual.manticoresearch.com/Changelog)）。Manticore Search 是一个现代、快速且轻量的数据库，具备卓越的全文搜索能力，基于对其前身的几乎重写构建而成。

## Manticore 的主要特点包括：
#### 强大且快速的全文搜索，适用于小型和大型数据集

  * [查询自动补全](Searching/Autocomplete.md)
  * [模糊搜索](Searching/Spell_correction.md#Fuzzy-Search)
  * 超过20种 [全文操作符](https://play.manticoresearch.com/fulltextintro/)<!--{target="_blank"}--> 和超过20种排名因子
  * [自定义排名](Searching/Sorting_and_ranking.md#Ranking-overview)
  * [词干提取](Creating_a_table/NLP_and_tokenization/Morphology.md)
  * [词形还原](Creating_a_table/NLP_and_tokenization/Morphology.md)
  * [停用词](Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md)
  * [同义词](Creating_a_table/NLP_and_tokenization/Exceptions.md)
  * [词形变化](Creating_a_table/NLP_and_tokenization/Wordforms.md)
  * [字符级和词级的高级分词](Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md)
  * [准确的中文分词](Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md)
  * [文本高亮](Searching/Highlighting.md)

#### 矢量搜索能力
Manticore Search 支持将由您的机器学习模型生成的嵌入向量添加到每个文档中，然后对它们进行[最近邻搜索](Searching/KNN.md)。这使您可以构建诸如相似度搜索、推荐、语义搜索及基于NLP算法的相关性排名等功能，包括图像、视频和声音搜索。

#### JOIN
Manticore Search 通过 SQL 和 JSON 支持 [JOIN](Searching/Joining.md) 查询，允许您组合多个表的数据。

#### 多线程
Manticore Search 利用智能查询并行化以降低响应时间，并在需要时充分利用所有 CPU 核心。

#### 基于成本的查询优化器
基于成本的查询优化器使用有关索引数据的统计数据来评估不同执行计划的相对成本，从而确定检索所需结果的最有效方案，考虑索引数据大小、查询复杂性和可用资源等因素。

#### 存储选项
Manticore 提供[行存储和列存储选项](Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)，以适应各种规模的数据集。传统且默认的行存储选项适用于所有大小的数据集，而通过 Manticore Columnar Library 提供的列存储选项则适用于更大的数据集。这两种存储方式的主要区别在于，行存储要求除全文字段外的所有属性保持在内存中以实现最佳性能，而列存储则不需要，从而降低了内存消耗，但性能可能略有下降（详见 https://db-benchmarks.com/ 的统计数据）。

#### 自动二级索引
[Manticore Columnar Library](https://github.com/manticoresoftware/columnar/) 使用 [分段几何模型索引](https://github.com/gvinciguerra/PGM-index)，这种索引利用了索引键与内存位置之间的学习映射。该映射的紧凑性，加上独特的递归构建算法，使得 PGM 索引在空间效率上远胜传统索引，同时仍提供最佳的查询和更新性能。所有数值和字符串字段默认启用二级索引，且可以用于 json 属性。

#### 以 SQL 为主
Manticore 的原生语法为 SQL，支持 SQL over HTTP 和 MySQL 协议，允许通过各种编程语言中的流行 MySQL 客户端连接。

#### HTTP JSON
为了更程序化地管理数据和模式，Manticore 提供了类似 Elasticsearch 的 [HTTP JSON](Searching/Full_text_matching/Basic_usage.md#HTTP-JSON) 协议。

#### 兼容 Elasticsearch 的写入操作
您可以执行兼容 Elasticsearch 的 [插入](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Adding-documents-to-a-real-time-table) 和 [替换](Data_creation_and_modification/Updating_documents/REPLACE.md#REPLACE) JSON 查询，从而使 Manticore 能与 Logstash（版本低于7.13）、Filebeat 及其他 Beats 系列工具协同使用。

#### 声明式与命令式的模式管理
轻松在线或通过配置文件创建、更新和删除表。

#### C++ 的优势与 PHP 的便利
Manticore Search 守护进程是用 C++ 开发的，提供快速启动时间和高效的内存利用率。低级优化进一步提升了性能。另一个关键组件称为 [Manticore Buddy](https://github.com/manticoresoftware/manticoresearch-buddy)，它是用 PHP 编写的，用于实现不需要极快响应时间或极高处理能力的高级功能。虽然参与 C++ 代码的贡献可能具有挑战性，但使用 Manticore Buddy 添加新的 SQL/JSON 命令应该是一个简单的过程。

#### 实时插入
新添加或更新的文档可以立即被读取。

#### 便于学习的互动课程
我们提供[免费互动课程](https://play.manticoresearch.com/)，让学习变得轻松。

#### 事务
虽然 Manticore 并非完全 ACID 兼容，但它支持隔离事务以实现原子更改和二进制日志以保证写入安全。

#### 内置复制和负载均衡
数据可以分布在服务器和数据中心上，任何 Manticore Search 节点既可以作为负载均衡器也可以作为数据节点。Manticore 使用 [Galera 库](https://galeracluster.com/) 实现几乎同步的多主机 [复制](https://play.manticoresearch.com/replication/)，确保所有节点的数据一致，防止数据丢失，并提供卓越的复制性能。

#### 内置备份功能
Manticore 配备了外部工具 [manticore-backup](Securing_and_compacting_a_table/Backup_and_restore.md) 和 [BACKUP](Securing_and_compacting_a_table/Backup_and_restore.md#BACKUP-SQL-command-reference) SQL 命令，简化了备份和还原数据的过程。或者，您也可以使用 [mysqldump](https://dev.mysql.com/doc/refman/8.0/en/mysqldump.html) 来[进行逻辑备份](Securing_and_compacting_a_table/Backup_and_restore.md#Backup-and-restore-with-mysqldump)。

#### 开箱即用的数据同步
`indexer` 工具和 Manticore 完善的配置语法使得从 MySQL、PostgreSQL、ODBC 兼容数据库、XML 和 CSV 等来源同步数据变得轻松。

#### 集成选项
您可以使用 [FEDERATED 引擎](Extensions/FEDERATED.md) 或通过 [ProxySQL](https://manticoresearch.com/blog/using-proxysql-to-route-inserts-in-a-distributed-realtime-index/) 将 Manticore Search 与 MySQL/MariaDB 服务器集成。

您可以使用 [Apache Superset](https://manticoresearch.com/blog/manticoresearch-apache-superset-integration/)、Kibana 和 [Grafana](https://manticoresearch.com/blog/manticoresearch-grafana-integration/) 可视化存储在 Manticore 中的数据。各种 MySQL 工具可用于交互式开发 Manticore 查询，例如 [HeidiSQL](https://www.heidisql.com/) 和 [DBForge](https://www.devart.com/dbforge/)。

您可以将 Manticore Search 与 [Kibana](Integration/Kibana.md) 一起使用。

#### 流过滤变得简单
Manticore 提供了一种特殊的表类型，“[percolate](Creating_a_table/Local_tables/Percolate_table.md)” 表，允许搜索查询而非数据，成为过滤全文数据流的高效工具。只需将您的查询存储在表中，处理您的数据流时将每批文档发送给 Manticore Search，您将只收到与存储查询匹配的结果。

#### 可能的应用
Manticore Search 多功能，可应用于多种场景，包括：

- **全文搜索**：
  - 适合电子商务平台，实现快速且精准的产品搜索，包含自动完成和模糊搜索等功能。
  - 适合内容丰富的网站，使用户能快速找到相关的文章或文档。

- **数据分析**：
  - 使用 [Beats/Logstash](https://manticoresearch.com/blog/integration-of-manticore-with-logstash-filebeat/)、[Vector.dev](https://manticoresearch.com/blog/integration-of-manticore-with-vectordev/) 和 [Fluentbit](https://manticoresearch.com/blog/integration-of-manticore-with-fluentbit/) 将数据导入 Manticore Search。
  - 利用 Manticore 的列存储和 OLAP 能力高效分析大数据集。
  - 对数 TB 数据执行复杂查询，延迟极低。
  - 使用 Kibana、[Grafana](https://manticoresearch.com/blog/manticoresearch-grafana-integration/) 或 [Apache Superset](https://manticoresearch.com/blog/manticoresearch-apache-superset-integration/) 可视化数据。

- **分面搜索**：
  - 允许用户按类别（如价格、品牌或日期）过滤搜索结果，实现更精细的搜索体验。

- **地理空间搜索**：
  - 利用 Manticore 的地理空间能力实现基于位置的搜索，如查找附近的餐厅或商店。

- **拼写纠正**：
  - 自动纠正用户搜索查询中的拼写错误，提升搜索准确率和用户体验。

- **自动完成**：
  - 在用户输入时提供实时建议，提高搜索的可用性和速度。

- **数据流过滤**：
  - 使用 percolate 表过滤和处理实时数据流，如社交媒体信息流或日志数据，高效便捷。


## 系统要求

* 架构：arm64 或 x86_64
* 操作系统：基于 Debian（如 Debian、Ubuntu、Mint）、基于 RHEL（如 RHEL、CentOS、Alma、Oracle Linux、Amazon Linux）、Windows 或 MacOS。
* [Manticore Columnar Library](https://github.com/manticoresoftware/columnar)，提供[列存储](Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)和[二级索引](Introduction.md#Automatic-secondary-indexes)，要求 CPU 支持 SSE >= 4.2。
* 无特定的磁盘空间或内存要求。空的 Manticore Search 实例仅使用大约 40MB 的 RSS 内存。

<!-- proofread -->

