# 引言

Manticore Search 是一个高性能的多存储数据库，专为搜索和分析而设计，提供闪电般快速的全文搜索、实时索引以及像向量搜索和柱状存储等高阶功能，以便于高效的数据分析。它能够处理小型和大型数据集，提供无缝的可伸缩性和强大的洞察力，适用于现代应用程序。

作为一个开源数据库（可在 [GitHub](https://github.com/manticoresoftware/manticoresearch/) 上获取），Manticore Search 于 2017 年创建，作为 [Sphinx Search](https://sphinxsearch.com/) 引擎的延续。我们的开发团队汲取了 Sphinx 的所有最佳特性，并显著改善了其功能，修复了数百个错误（详见我们的 [Changelog](https://manual.manticoresearch.com/Changelog)）。Manticore Search 是一个现代的、快速的、轻量级的数据库，具有卓越的全文搜索能力，建立在其前身的几乎完全重写之上。

## Manticore 的主要特点：
#### 强大且快速的全文搜索，适用于小型和大型数据集

  * [查询自动完成](Searching/Autocomplete.md)
  * [模糊搜索](Searching/Spell_correction.md#Fuzzy-Search)
  * 超过 20 个 [全文操作符](https://play.manticoresearch.com/fulltextintro/)<!--{target="_blank"}--> 和超过 20 个排名因素
  * [自定义排名](Searching/Sorting_and_ranking.md#Ranking-overview)
  * [词干提取](Creating_a_table/NLP_and_tokenization/Morphology.md)
  * [词形还原](Creating_a_table/NLP_and_tokenization/Morphology.md)
  * [停用词](Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md)
  * [同义词](Creating_a_table/NLP_and_tokenization/Exceptions.md)
  * [词形](Creating_a_table/NLP_and_tokenization/Wordforms.md)
  * [字符和词级的高级分词](Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md)
  * [正确的中文分词](Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md)
  * [文本高亮](Searching/Highlighting.md)

#### 向量搜索能力
Manticore Search 支持将您机器学习模型生成的嵌入添加到每个文档中，然后对其进行 [最近邻搜索](Searching/KNN.md)。这使您能够构建类似于相似性搜索、推荐、语义搜索和基于 NLP 算法的相关性排名等特性，包括图像、视频和声音搜索。

#### JOIN
Manticore Search 通过 SQL 和 JSON 支持 [JOIN](Searching/Joining.md) 查询，允许您结合来自多个表的数据。

#### 多线程
Manticore Search 利用智能查询并行化来降低响应时间，并在需要时充分利用所有 CPU 核心。

#### 基于成本的查询优化器
基于成本的查询优化器使用有关索引数据的统计信息来评估给定查询的不同执行计划的相对成本。这使优化器能够确定检索所需结果的最有效计划，考虑到索引数据的大小、查询的复杂性和可用资源等因素。

#### 存储选项
Manticore 提供 [行存储和列存储选项](Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)，以适应各种大小的数据集。传统的默认行存储选项适用于所有大小的数据集 - 小型、中型和大型，而列存储选项则通过 Manticore 列库为更大的数据集提供。此存储选项之间的主要区别在于，行存储要求将所有属性（不包括全文字段）保留在 RAM 中，以获得最佳性能，而列存储则不要求如此，因此提供更低的 RAM 消耗，但性能可能稍慢（如 https://db-benchmarks.com/ 上的统计数据所示）。

#### 自动二级索引
[Manticore 列库](https://github.com/manticoresoftware/columnar/) 使用 [分段几何模型索引](https://github.com/gvinciguerra/PGM-index)，该索引利用索引键与其在内存中位置之间的学习映射。这种映射的简洁性，以及独特的递归构造算法，使 PGM 索引在空间上比传统索引要优秀几个数量级，同时仍然提供最佳的查询和更新时间性能。所有数字和字符串字段的二级索引默认开启，且可为 json 属性启用。

#### SQL 优先
Manticore 的本机语法是 SQL，支持通过 HTTP 和 MySQL 协议执行 SQL，允许通过任何编程语言中的流行 mysql 客户端进行连接。

#### HTTP 上的 JSON
为了更程序化地管理数据和模式，Manticore 提供 [HTTP JSON](Searching/Full_text_matching/Basic_usage.md#HTTP-JSON) 协议，类似于 Elasticsearch 的协议。

#### 兼容 Elasticsearch 的写入
您可以执行兼容 Elasticsearch 的 [插入](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real_time_table.md#Adding-documents-to-a-real-time-table) 和 [替换](Data_creation_and_modification/Updating_documents/REPLACE.md#REPLACE) JSON 查询，这使得 Manticore 可以与 Logstash（版本 < 7.13）、Filebeat 和其他 Beats 家族的工具一起使用。

#### 声明式和命令式模式管理
轻松在线或通过配置文件创建、更新和删除表。

#### C++ 的优势和 PHP 的便利性
Manticore Search 守护进程是用 C++ 开发的，提供快速启动时间和高效的内存利用。低级优化进一步提升了性能。另一个关键组件，称为 [Manticore Buddy](https://github.com/manticoresoftware/manticoresearch-buddy)，是用 PHP 编写的，用于不需要极快响应时间或极高处理能力的高级功能。尽管对 C++ 代码的贡献可能具有挑战性，但使用 Manticore Buddy 添加新的 SQL/JSON 命令应该是一个直接的过程。

#### 实时插入
新添加或更新的文档可以立即读取。

#### 交互式课程，轻松学习
我们提供[免费的交互式课程](https://play.manticoresearch.com/)，让学习变得轻松。

#### 事务
虽然 Manticore 不完全符合 ACID 标准，但它支持隔离事务以进行原子更改，并通过二进制日志进行安全写入。

#### 内置复制和负载均衡
数据可以分布在服务器和数据中心上，任何 Manticore Search 节点都可以同时作为负载均衡器和数据节点。Manticore 使用 [Galera 库](https://galeracluster.com/)实现几乎同步的多主[复制](https://play.manticoresearch.com/replication/)，确保所有节点间的数据一致性，防止数据丢失，并提供出色的复制性能。

#### 内置备份功能
Manticore 配备了外部工具 [manticore-backup](Securing_and_compacting_a_table/Backup_and_restore.md)，以及 [BACKUP](Securing_and_compacting_a_table/Backup_and_restore.md#BACKUP-SQL-command-reference) SQL 命令，简化了备份和恢复数据的过程。或者，您可以使用 [mysqldump](https://dev.mysql.com/doc/refman/8.0/en/mysqldump.html) 来[进行逻辑备份](../Securing_and_compacting_a_table/Backup_and_restore.md#Backup-and-restore-with-mysqldump)。

#### 开箱即用的数据同步
`indexer` 工具和 Manticore 的全面配置语法使得从 MySQL、PostgreSQL、ODBC 兼容的数据库、XML 和 CSV 等源同步数据变得简单。

#### 集成选项
您可以使用 [FEDERATED 引擎](Extensions/FEDERATED.md)或通过 [ProxySQL](https://manticoresearch.com/blog/using-proxysql-to-route-inserts-in-a-distributed-realtime-index/) 将 Manticore Search 与 MySQL/MariaDB 服务器集成。

您可以使用 [Apache Superset](https://manticoresearch.com/blog/manticoresearch-apache-superset-integration/)、Kibana 和 [Grafana](https://manticoresearch.com/blog/manticoresearch-grafana-integration/) 来可视化存储在 Manticore 中的数据。可以使用各种 MySQL 工具以交互方式开发 Manticore 查询，如 [HeidiSQL](https://www.heidisql.com/) 和 [DBForge](https://www.devart.com/dbforge/)。

您可以将 Manticore Search 与 [Kibana](Integration/Kibana.md) 一起使用。

#### 流过滤变得简单
Manticore 提供了一种特殊的表类型，即"[percolate](Creating_a_table/Local_tables/Percolate_table.md)"表，它允许您搜索查询而不是数据，使其成为过滤全文数据流的有效工具。只需将查询存储在表中，通过将每批文档发送到 Manticore Search 来处理数据流，并仅接收与存储的查询匹配的结果。

#### 可能的应用
Manticore Search 用途广泛，可应用于各种场景，包括：

- **全文搜索**：
  - 对于电子商务平台理想，支持具有自动完成和模糊搜索等功能的快速准确的产品搜索。
  - 对于内容丰富的网站完美，使用户能快速找到相关文章或文档。

- **数据分析**：
  - 使用 [Beats/Logstash](https://manticoresearch.com/blog/integration-of-manticore-with-logstash-filebeat/)、[Vector.dev](https://manticoresearch.com/blog/integration-of-manticore-with-vectordev/)、[Fluentbit](https://manticoresearch.com/blog/integration-of-manticore-with-fluentbit/) 将数据导入 Manticore Search。
  - 使用 Manticore 的列式存储和 OLAP 功能高效分析大型数据集。
  - 以最小延迟对数 TB 数据执行复杂查询。
  - 使用 Kibana、[Grafana](https://manticoresearch.com/blog/manticoresearch-grafana-integration/) 或 [Apache Superset](https://manticoresearch.com/blog/manticoresearch-apache-superset-integration/) 可视化数据。

- **分面搜索**：
  - 使用户能够按类别（如价格、品牌或日期）过滤搜索结果，提供更精细的搜索体验。

- **地理空间搜索**：
  - 利用 Manticore 的地理空间功能，实现基于位置的搜索，如查找附近的餐馆或商店。

- **拼写纠正**：
  - 自动纠正搜索查询中的用户拼写错误，以提高搜索准确性和用户体验。

- **自动完成**：
  - 在用户输入时提供实时建议，增强搜索可用性和速度。

- **数据流过滤**：
  - 使用 percolate 表高效过滤和处理实时数据流，如社交媒体源或日志数据。


## 要求

* 架构：arm64 或 x86_64
* 操作系统：基于 Debian（如 Debian、Ubuntu、Mint），基于 RHEL（如 RHEL、CentOS、Alma、Oracle Linux、Amazon Linux），Windows 或 MacOS。
* [Manticore Columnar Library](https://github.com/manticoresoftware/columnar)，提供[列式存储](Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)和[辅助索引](Introduction.md#Automatic-secondary-indexes)，需要 CPU 支持 SSE >= 4.2。
* 不需要特定的磁盘空间或内存要求。空的 Manticore Search 实例仅使用约 40MB 的 RSS 内存。

<!-- proofread -->
# 引言

Manticore Search 是一个高性能的多存储数据库，专门针对搜索和分析而构建，提供闪电般快速的全文搜索、实时索引，以及像向量搜索和列式存储等高级功能，以便对数据进行高效分析。旨在处理小型和大型数据集，它为现代应用程序提供无缝的可扩展性和强大的洞察力。

作为一个开源数据库（可在 [GitHub](https://github.com/manticoresoftware/manticoresearch/) 上获取），Manticore Search 于2017年创建，作为[Sphinx Search](https://sphinxsearch.com/)引擎的延续。我们的开发团队汲取了 Sphinx 的所有最佳特性，并显著改善了其功能，在此过程中修复了数百个错误（详细信息请参见我们的 [Changelog](https://manual.manticoresearch.com/Changelog)）。Manticore Search 是一个现代、快速且轻量级的数据库，具有卓越的全文搜索能力，建立在其前身的几乎完全重写基础上。

## Manticore 的关键特性包括：
#### 强大且快速的全文搜索，适用于小型和大型数据集

  * [查询自动补全](Searching/Autocomplete.md)
  * [模糊搜索](Searching/Spell_correction.md#Fuzzy-Search)
  * 超过 20 个 [全文操作符](https://play.manticoresearch.com/fulltextintro/)<!--{target="_blank"}--> 和超过 20 个排名因素
  * [自定义排名](Searching/Sorting_and_ranking.md#Ranking-overview)
  * [词干提取](Creating_a_table/NLP_and_tokenization/Morphology.md)
  * [词形还原](Creating_a_table/NLP_and_tokenization/Morphology.md)
  * [停用词](Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md)
  * [同义词](Creating_a_table/NLP_and_tokenization/Exceptions.md)
  * [词形](Creating_a_table/NLP_and_tokenization/Wordforms.md)
  * [字符和单词级的高级分词](Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md)
  * [正确的中文分词](Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md)
  * [文本高亮](Searching/Highlighting.md)

#### 向量搜索能力
Manticore Search 支持为每个文档添加由您的机器学习模型生成的嵌入，并在这些嵌入上执行 [最近邻搜索](Searching/KNN.md)。这使您能够构建如相似性搜索、推荐、语义搜索以及基于 NLP 算法的相关性排名等功能，还包括图像、视频和声音搜索。

#### JOIN
Manticore Search 通过 SQL 和 JSON 支持 [JOIN](Searching/Joining.md) 查询，允许您从多个表中组合数据。

#### 多线程
Manticore Search 使用智能查询并行化来降低响应时间，并在需要时充分利用所有 CPU 核心。

#### 基于成本的查询优化器
基于成本的查询优化器使用有关索引数据的统计信息来评估给定查询的不同执行计划的相对成本。这使得优化器能够确定检索所需结果的最有效计划，考虑到索引数据的大小、查询的复杂性以及可用资源等因素。

#### 存储选项
Manticore 提供 [行式和列式存储选项](Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)以适应各种大小的数据集。传统的默认行式存储选项适用于所有大小的数据集——小型、中型和大型，而列式存储选项则通过 Manticore 列式库提供，以处理更大规模的数据集。这些存储选项之间的关键区别在于，行式存储要求所有属性（不包括全文字段）保存在 RAM 中以获得最佳性能，而列式存储则不需要，因此提供了较低的 RAM 消耗，但可能导致稍慢的性能（如 https://db-benchmarks.com/ 上的统计数据所示）。

#### 自动二级索引
[Manticore 列式库](https://github.com/manticoresoftware/columnar/) 使用 [分段几何模型索引](https://github.com/gvinciguerra/PGM-index)，该索引利用了索引键与其在内存中的位置之间的学习映射。这种映射的简洁性，加上独特的递归构造算法，使 PGM 索引在空间上极大地超过传统索引，同时仍然提供最佳的查询和更新时间性能。对于所有数字和字符串字段，二级索引默认为开启状态，并可以为 json 属性启用。

#### SQL 优先
Manticore 的原生语法是 SQL，并支持通过 HTTP 和 MySQL 协议进行 SQL，从而允许通过任何编程语言中的流行 mysql 客户端进行连接。

#### HTTP 上的 JSON
对于更程序化的数据和架构管理，Manticore 提供类似于 Elasticsearch 的 [HTTP JSON](Searching/Full_text_matching/Basic_usage.md#HTTP-JSON) 协议。

#### Elasticsearch 兼容写入
您可以执行 Elasticsearch 兼容的 [插入](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real_time_table.md#Adding-documents-to-a-real-time-table) 和 [替换](Data_creation_and_modification/Updating_documents/REPLACE.md#REPLACE) JSON 查询，这使得可以将 Manticore 与像 Logstash（版本 < 7.13）、Filebeat 及其他 Beats 家族的工具结合使用。

#### 声明式和命令式架构管理
轻松在线或通过配置文件创建、更新和删除表。

#### C++ 的优点和 PHP 的便利性
Manticore Search 守护进程使用 C++ 开发，提供快速的启动时间和高效的内存利用。低级优化进一步增强了性能。另一个重要组件，称为 [Manticore Buddy](https://github.com/manticoresoftware/manticoresearch-buddy)，使用 PHP 编写，用于不需要闪电般快速响应时间或极高处理能力的高级功能。尽管为 C++ 代码做贡献可能是一个挑战，但使用 Manticore Buddy 添加新的 SQL/JSON 命令应该是一个简单的过程。

#### 实时插入
新添加或更新的文档可以立即被读取。

#### 交互式课程以便于学习
我们提供 [免费的交互式课程](https://play.manticoresearch.com/) 以使学习变得轻松。

#### 事务
虽然 Manticore 并不完全符合 ACID 标准，但它支持用于原子更改的隔离事务和用于安全写入的二进制日志。

#### 内置复制和负载均衡
数据可以分布在服务器和数据中心之间，任何 Manticore Search 节点都可以充当负载均衡器和数据节点。Manticore 实施几乎同步的多主 [复制](https://play.manticoresearch.com/replication/) 使用 [Galera 库](https://galeracluster.com/)，确保所有节点之间的数据一致性，防止数据丢失，并提供卓越的复制性能。

#### 内置备份功能
Manticore 配备了外部工具 [manticore-backup](Securing_and_compacting_a_table/Backup_and_restore.md)，以及 [BACKUP](Securing_and_compacting_a_table/Backup_and_restore.md#BACKUP-SQL-command-reference) SQL 命令，以简化备份和恢复数据的过程。或者，您可以使用 [mysqldump](https://dev.mysql.com/doc/refman/8.0/en/mysqldump.html) 来 [进行逻辑备份](../Securing_and_compacting_a_table/Backup_and_restore.md#Backup-and-restore-with-mysqldump)。

#### 开箱即用的数据同步
`indexer` 工具和 Manticore 的全面配置语法使得从 MySQL、PostgreSQL、ODBC 兼容数据库、XML 和 CSV 等源同步数据变得简单。

#### 集成选项
您可以使用 [FEDERATED 引擎](Extensions/FEDERATED.md) 或通过 [ProxySQL](https://manticoresearch.com/blog/using-proxysql-to-route-inserts-in-a-distributed-realtime-index/) 将 Manticore Search 与 MySQL/MariaDB 服务器集成。

您可以使用 [Apache Superset](https://manticoresearch.com/blog/manticoresearch-apache-superset-integration/)、Kibana 和 [Grafana](https://manticoresearch.com/blog/manticoresearch-grafana-integration/) 来可视化存储在 Manticore 中的数据。可以使用各种 MySQL 工具交互式地开发 Manticore 查询，例如 [HeidiSQL](https://www.heidisql.com/) 和 [DBForge](https://www.devart.com/dbforge/)。

您可以与 [Kibana](Integration/Kibana.md) 一起使用 Manticore Search。

#### 轻松的流过滤
Manticore 提供了一种特殊的表类型，即 "[percolate](Creating_a_table/Local_tables/Percolate_table.md)" 表，允许您搜索查询而不是数据，使其成为过滤全文数据流的有效工具。只需将您的查询存储在表中，处理您的数据流，通过将每批文档发送到 Manticore Search 即可，接收仅与您存储的查询匹配的结果。

#### 可能的应用
Manticore Search 多才多艺，可以应用于各种场景，包括：

- **全文搜索**：
  - 适用于电子商务平台，使产品搜索快速而准确，具备自动完成和模糊搜索等功能。
  - 非常适合内容丰富的网站，使用户能够快速找到相关的文章或文档。

- **数据分析**：
  - 使用 [Beats/Logstash](https://manticoresearch.com/blog/integration-of-manticore-with-logstash-filebeat/)、[Vector.dev](https://manticoresearch.com/blog/integration-of-manticore-with-vectordev/) 和 [Fluentbit](https://manticoresearch.com/blog/integration-of-manticore-with-fluentbit/) 将数据导入 Manticore Search。
  - 利用 Manticore 的列式存储和 OLAP 功能高效分析大数据集。
  - 对于 TB 级的数据执行复杂查询，延迟极小。
  - 使用 Kibana、[Grafana](https://manticoresearch.com/blog/manticoresearch-grafana-integration/) 或 [Apache Superset](https://manticoresearch.com/blog/manticoresearch-apache-superset-integration/) 可视化数据。

- **分面搜索**：
  - 使用户能够通过类别（如价格、品牌或日期）过滤搜索结果，以获得更精细的搜索体验。

- **地理空间搜索**：
  - 实施基于位置的搜索，例如查找附近的餐馆或商店，利用 Manticore 的地理空间能力。

- **拼写纠正**：
  - 自动纠正用户在搜索查询中的输入错误，以提高搜索准确性和用户体验。

- **自动完成**：
  - 在用户输入时提供实时建议，增强搜索的可用性和速度。

- **数据流过滤**：
  - 使用 percolate 表高效地过滤和处理实时数据流，例如社交媒体信息流或日志数据。


# 要求

* 架构：arm64 或 x86_64
* 操作系统：基于 Debian 的 (例如 Debian、Ubuntu、Mint)、基于 RHEL 的 (例如 RHEL、CentOS、Alma、Oracle Linux、Amazon Linux)、Windows 或 MacOS。
* [Manticore 列式库](https://github.com/manticoresoftware/columnar)，提供 [列式存储](Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) 和 [二级索引](Introduction.md#Automatic-secondary-indexes)，需要支持 SSE >= 4.2 的 CPU。
* 不需要特定的磁盘空间或内存要求。一个空的 Manticore Search 实例仅使用大约 40MB 的 RSS 内存。

<!-- proofread -->
