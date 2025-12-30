# 介绍

测试。
Manticore Search 是一款专为搜索和分析而设计的高性能、多存储数据库，提供闪电般的全文搜索、实时索引以及向量搜索和列式存储等高级功能，以实现高效的数据分析。它能够处理小数据集和大数据集，为现代应用程序提供无缝的可扩展性和强大的洞察力。

作为一款开源数据库（可在 [GitHub](https://github.com/manticoresoftware/manticoresearch/) 上获取），Manticore Search 于 2017 年作为 [Sphinx Search](https://sphinxsearch.com/) 引擎的延续而创建。我们的开发团队将 Sphinx 的所有最佳功能整合进来，并显著改进了其功能，同时修复了数百个错误（详见我们的 [Changelog](https://manual.manticoresearch.com/Changelog)）。Manticore Search 是一款现代、快速且轻量级的数据库，具有卓越的全文搜索能力，其前身几乎完全重写。

## Manticore 的关键功能包括：
#### 强大且快速的全文搜索，适用于小数据集和大数据集

  * [查询自动补全](Searching/Autocomplete.md)
  * [模糊搜索](Searching/Spell_correction.md#Fuzzy-Search)
  * 超过 20 个 [全文操作符](https://play.manticoresearch.com/fulltextintro/)<!--{target="_blank"}--> 和超过 20 个排名因素
  * [自定义排名](Searching/Sorting_and_ranking.md#Ranking-overview)
  * [词干提取](Creating_a_table/NLP_and_tokenization/Morphology.md)
  * [词形还原](Creating_a_table/NLP_and_tokenization/Morphology.md)
  * [停用词](Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md)
  * [同义词](Creating_a_table/NLP_and_tokenization/Exceptions.md)
  * [词形转换](Creating_a_table/NLP_and_tokenization/Wordforms.md)
  * [字符和单词级别的高级分词](Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md)
  * [正确的中文分词](Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md)
  * [文本高亮](Searching/Highlighting.md)

#### 向量搜索功能
Manticore Search 支持将您的机器学习模型生成的嵌入向量添加到每个文档中，然后对它们执行 [最近邻搜索](Searching/KNN.md)。这使您能够构建基于 NLP 算法的功能，如相似性搜索、推荐、语义搜索和相关性排序，以及其他功能，包括图像、视频和声音搜索。

#### JOIN
Manticore Search 支持通过 SQL 和 JSON 的 [JOIN](Searching/Joining.md) 查询，允许您将多个表中的数据结合起来。

#### 多线程
Manticore Search 利用智能查询并行化来降低响应时间，并在需要时充分利用所有 CPU 核心。

#### 基于成本的查询优化器
基于成本的查询优化器使用有关索引数据的统计信息来评估给定查询的不同执行计划的相对成本。这使优化器能够确定最有效的计划来检索所需结果，同时考虑索引数据的大小、查询的复杂性以及可用资源等因素。

#### 存储选项
Manticore 提供 [行式和列式存储选项](Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)，以适应各种大小的数据集。传统的默认行式存储选项适用于所有大小的数据集——小、中、大，而列式存储选项则通过 Manticore Columnar Library 提供，适用于更大的数据集。这些存储选项之间的主要区别在于，行式存储需要将所有属性（不包括全文字段）保留在 RAM 中以实现最佳性能，而列式存储则不需要，因此提供更低的 RAM 消耗，但可能会略微降低性能（如 https://db-benchmarks.com/ 上的统计数据所示）。

#### 自动次级索引
[Manticore Columnar Library](https://github.com/manticoresoftware/columnar/) 使用 [Piecewise Geometric Model 索引](https://github.com/gvinciguerra/PGM-index)，该索引利用了索引键与其在内存中位置之间的学习映射。这种映射的简洁性，加上一种特殊的递归构造算法，使 PGM 索引成为一种在空间上比传统索引高出几个数量级的数据结构，同时仍提供最佳的查询和更新时间性能。次级索引默认对所有数字和字符串字段启用，并且可以为 JSON 属性启用。

#### 以 SQL 为主
Manticore 的原生语法是 SQL，并且支持通过 HTTP 和 MySQL 协议使用 SQL，允许通过任何编程语言的流行 MySQL 客户端进行连接。

#### 通过 HTTP 的 JSON
为了以更编程化的方式管理数据和模式，Manticore 提供了 [HTTP JSON](Searching/Full_text_matching/Basic_usage.md#HTTP-JSON) 协议，类似于 Elasticsearch 的协议。

#### 与 Elasticsearch 兼容的写入
您可以执行与 Elasticsearch 兼容的 [插入](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Adding-documents-to-a-real-time-table) 和 [替换](Data_creation_and_modification/Updating_documents/REPLACE.md#REPLACE) JSON 查询，这使得可以使用 Manticore 与 Logstash（版本 < 7.13）、Filebeat 和其他 Beats 家族工具。

#### 声明式和命令式模式管理
通过在线或通过配置文件轻松创建、更新和删除表。

#### C++的优势与PHP的便捷性
Manticore Search 守护进程使用C++开发，提供快速的启动时间和高效的内存利用。低级优化进一步提升了性能。另一个关键组件，称为 [Manticore Buddy](https://github.com/manticoresoftware/manticoresearch-buddy)，使用PHP编写，用于不需要闪电般快速响应时间或极高处理能力的高级功能。虽然对C++代码的贡献可能具有挑战性，但使用Manticore Buddy添加新的SQL/JSON命令应该是一个简单的过程。

#### 实时插入
新添加或更新的文档可以立即读取。

#### 互动课程便于学习
我们提供 [免费互动课程](https://play.manticoresearch.com/)，让学习变得轻松。

#### 事务
虽然Manticore不是完全符合ACID标准，但它支持隔离事务以实现原子更改和二进制日志以实现安全写入。

#### 内置复制和负载均衡
数据可以跨服务器和数据中心分布，任何Manticore Search节点都可以同时充当负载均衡器和数据节点。Manticore使用 [Galera库](https://play.manticoresearch.com/replication/) 实现虚拟同步多主 [复制](https://galeracluster.com/)，确保所有节点的数据一致性，防止数据丢失，并提供卓越的复制性能。

#### 内置备份功能
Manticore配备了一个外部工具 [manticore-backup](Securing_and_compacting_a_table/Backup_and_restore.md)，以及 [BACKUP](Securing_and_compacting_a_table/Backup_and_restore.md#BACKUP-SQL-command-reference) SQL命令，以简化备份和恢复数据的过程。或者，您可以使用 [mysqldump](https://dev.mysql.com/doc/refman/8.0/en/mysqldump.html) 来 [进行逻辑备份](Securing_and_compacting_a_table/Backup_and_restore.md#Backup-and-restore-with-mysqldump)。

#### 开箱即用的数据同步
Manticore的`indexer`工具和全面的配置语法使得从MySQL、PostgreSQL、ODBC兼容数据库、XML和CSV等来源同步数据变得容易。

#### 集成选项
您可以使用 [FEDERATED引擎](Extensions/FEDERATED.md) 或通过 [ProxySQL](https://manticoresearch.com/blog/using-proxysql-to-route-inserts-in-a-distributed-realtime-index/) 将Manticore Search与MySQL/MariaDB服务器集成。

您可以使用 [Apache Superset](https://manticoresearch.com/blog/manticoresearch-apache-superset-integration/) 和 [Grafana](https://manticoresearch.com/blog/manticoresearch-grafana-integration/) 来可视化存储在Manticore中的数据。可以使用各种MySQL工具交互式开发Manticore查询，例如 [HeidiSQL](https://www.heidisql.com/) 和 [DBForge](https://www.devart.com/dbforge/)。

您还可以使用Manticore Search与 [Kibana](Integration/Kibana.md) 集成。

#### 简化的流过滤
Manticore提供了一种特殊的表类型，称为"[percolate](Creating_a_table/Local_tables/Percolate_table.md)"表，允许您搜索查询而不是数据，使其成为过滤全文数据流的高效工具。只需将查询存储在表中，通过将每批文档发送到Manticore Search来处理数据流，即可仅接收与存储查询匹配的结果。

#### 可能的应用
Manticore Search功能强大，可以应用于各种场景，包括：

- **全文搜索**：
  - 适用于电子商务平台，通过自动补全和模糊搜索等功能实现快速准确的产品搜索。
  - 适用于内容丰富的网站，允许用户快速找到相关文章或文档。

- **数据分析**：
  - 使用 [Beats/Logstash](https://manticoresearch.com/blog/integration-of-manticore-with-logstash-filebeat/)、[Vector.dev](https://manticoresearch.com/blog/integration-of-manticore-with-vectordev/) 或 [Fluentbit](https://manticoresearch.com/blog/integration-of-manticore-with-fluentbit/) 将数据导入Manticore Search。
  - 使用Manticore的列式存储和OLAP功能高效分析大型数据集。
  - 在最小延迟下对TB级数据执行复杂查询。
  - 使用Kibana、[Grafana](https://manticoresearch.com/blog/manticoresearch-grafana-integration/) 或 [Apache Superset](https://manticoresearch.com/blog/manticoresearch-apache-superset-integration/) 可视化数据。

- **面向属性的搜索**：
  - 允许用户通过类别（如价格、品牌或日期）过滤搜索结果，提供更精细的搜索体验。

- **地理空间搜索**：
  - 使用Manticore的地理空间功能实现基于位置的搜索，例如查找附近的餐厅或商店。

- **拼写纠正**：
  - 自动纠正用户搜索查询中的拼写错误，以提高搜索准确性和用户体验。

- **自动补全**：
  - 在用户输入时提供实时建议，增强搜索的可用性和速度。

- **数据流过滤**：
  - 使用percolate表高效过滤和处理实时数据流，例如社交媒体信息流或日志数据。


## 要求

* 架构：arm64 或 x86_64
* 操作系统：基于Debian（如Debian、Ubuntu、Mint）、基于RHEL（如RHEL、CentOS、Alma、Oracle Linux、Amazon Linux）、Windows 或 MacOS。
* [Manticore列式库](https://github.com/manticoresoftware/columnar)，提供 [列式存储](Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) 和 [二级索引](Introduction.md#Automatic-secondary-indexes)，需要配备SSE >= 4.2的CPU。
* 不需要特定的磁盘空间或RAM要求。一个空的Manticore Search实例仅使用约40MB的RSS RAM。

<!-- proofread -->

