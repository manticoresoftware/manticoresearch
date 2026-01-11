# 介绍

Manticore Search 是一个高性能、多存储数据库，专为搜索和分析设计，提供极速的全文搜索、实时索引以及向量搜索和列存储等高级功能，实现高效的数据分析。它既能处理小型数据集，也能处理大型数据集，为现代应用提供无缝扩展性和强大的洞察能力。

作为一个开源数据库（可在 [GitHub](https://github.com/manticoresoftware/manticoresearch/) 获取），Manticore Search 于2017年创建，是对 [Sphinx Search](https://sphinxsearch.com/) 引擎的延续。我们的开发团队汇集了 Sphinx 的所有优点，并显著提升了其功能，修复了数百个漏洞（详见我们的 [更新日志](https://manual.manticoresearch.com/Changelog)）。Manticore Search 是一个现代的、快速且轻量级的数据库，拥有卓越的全文搜索能力，基于对其前身近乎完全的重写构建。

## Manticore 的主要特点：
#### 强大且快速的全文搜索，适用于小型和大型数据集

  * [查询自动补全](Searching/Autocomplete.md)
  * [模糊搜索](Searching/Spell_correction.md#Fuzzy-Search)
  * 超过20种[全文搜索操作符](https://play.manticoresearch.com/fulltextintro/)<!--{target="_blank"}-->以及20多种排名因素
  * [自定义排名](Searching/Sorting_and_ranking.md#Ranking-overview)
  * [词干提取](Creating_a_table/NLP_and_tokenization/Morphology.md)
  * [词形还原](Creating_a_table/NLP_and_tokenization/Morphology.md)
  * [停用词](Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md)
  * [同义词](Creating_a_table/NLP_and_tokenization/Exceptions.md)
  * [词形变化](Creating_a_table/NLP_and_tokenization/Wordforms.md)
  * [字符级和词级的高级分词](Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md)
  * [完善的中文分词](Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md)
  * [文本高亮](Searching/Highlighting.md)

#### 向量搜索功能
Manticore Search 支持将机器学习模型生成的嵌入向量添加到每个文档中，然后对其执行[最近邻搜索](Searching/KNN.md)。这让您能够构建相似度搜索、推荐系统、语义搜索和基于 NLP 算法的相关性排名等功能，包括图像、视频和音频搜索。

#### JOIN
Manticore Search 支持通过 SQL 和 JSON 执行[JOIN](Searching/Joining.md)查询，允许您合并多个表中的数据。

#### 多线程
Manticore Search 利用智能查询并行处理来降低响应时间，并在需要时充分利用所有 CPU 核心。

#### 基于成本的查询优化器
基于成本的查询优化器利用索引数据的统计信息，评估给定查询的不同执行计划的相对成本。这样优化器可以确定检索所需结果的最有效计划，考虑到索引数据大小、查询复杂度及可用资源等因素。

#### 存储选项
Manticore 提供[行存储和列存储选项](Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)，以适应不同规模的数据集。传统且默认的行存储选项适用于所有大小的数据集——小型、中型及大型，而列存储选项则通过 Manticore Columnar Library 提供，适用于更大规模的数据集。这两种存储选项的关键区别在于，行存储要求所有属性（全文字段除外）保存在 RAM 中以获得最佳性能，而列存储则不要求，从而实现更低的内存消耗，但可能带来略微较慢的性能（如 https://db-benchmarks.com/ 上的统计所示）。

#### 自动二级索引
[Manticore Columnar Library](https://github.com/manticoresoftware/columnar/) 使用[分段几何模型索引 (PGM-index)](https://github.com/gvinciguerra/PGM-index)，利用索引键与其内存位置之间的学习映射。该映射的紧凑性及独特的递归构建算法使 PGM-index 成为空间效率远超传统索引的数据结构，同时仍提供最优的查询和更新性能。二级索引默认对所有数值和字符串字段开启，也可针对 JSON 属性启用。

#### 优先 SQL
Manticore 的原生语法是 SQL，支持通过 HTTP 和 MySQL 协议执行 SQL，允许通过任何编程语言中的流行 MySQL 客户端连接。

#### HTTP JSON
为了更程序化地管理数据和模式，Manticore 提供了类似 Elasticsearch 的[HTTP JSON](Searching/Full_text_matching/Basic_usage.md#HTTP-JSON)协议。

#### 兼容 Elasticsearch 的写入
您可以执行兼容 Elasticsearch 的[插入](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Adding-documents-to-a-real-time-table)和[替换](Data_creation_and_modification/Updating_documents/REPLACE.md#REPLACE) JSON 查询，从而使 Manticore 可与 Logstash（版本 < 7.13）、Filebeat 及 Beats 家族的其他工具一起使用。

#### 声明式和命令式的模式管理
轻松在线或通过配置文件创建、更新和删除表。

#### C++的优势和PHP的便利性
Manticore Search守护进程用C++编写，提供快速启动时间和高效的内存利用率。低级别的优化进一步提升了性能。另一个关键组件，称为[Manticore Buddy](https://github.com/manticoresoftware/manticoresearch-buddy)，用PHP编写，用于不需要闪电般的响应速度或极高处理能力的高级功能。虽然对C++代码进行贡献可能是一项挑战，但使用Manticore Buddy添加新的SQL/JSON命令应该是简单的过程。

#### 实时插入
新添加或更新的文档可以立即读取。

#### 互动课程，轻松学习
我们提供[免费互动课程](https://play.manticoresearch.com/)，使学习变得轻松。

#### 事务
虽然Manticore不完全符合ACID标准，但它支持隔离事务以实现原子更改，并支持二进制日志以确保安全写入。

#### 内置复制和负载均衡
数据可以分布在服务器和数据中心之间，任何Manticore Search节点都可以作为负载均衡器和数据节点。Manticore使用[Galera库](https://galeracluster.com/)实现近乎同步的多主[复制](https://play.manticoresearch.com/replication/)，确保所有节点的数据一致性，防止数据丢失，并提供出色的复制性能。

#### 内置备份功能
Manticore配备了外部工具[manticore-backup](Securing_and_compacting_a_table/Backup_and_restore.md)和[BACKUP](Securing_and_compacting_a_table/Backup_and_restore.md#BACKUP-SQL-command-reference) SQL命令，简化了备份和恢复数据的过程。或者，您可以使用[mysqldump](https://dev.mysql.com/doc/refman/8.0/en/mysqldump.html)进行[逻辑备份](Securing_and_compacting_a_table/Backup_and_restore.md#Backup-and-restore-with-mysqldump)。

#### 开箱即用的数据同步
`indexer`工具和Manticore的全面配置语法使其易于从MySQL、PostgreSQL、ODBC兼容的数据库、XML和CSV等源同步数据。

#### 集成选项
您可以使用[FEDERATED引擎](Extensions/FEDERATED.md)或[ProxySQL](https://manticoresearch.com/blog/using-proxysql-to-route-inserts-in-a-distributed-realtime-index/)将Manticore Search与MySQL/MariaDB服务器集成。

您可以使用[Apache Superset](https://manticoresearch.com/blog/manticoresearch-apache-superset-integration/)和[Grafana](https://manticoresearch.com/blog/manticoresearch-grafana-integration/)可视化存储在Manticore中的数据。可以使用各种MySQL工具以交互方式开发Manticore查询，例如[HeidiSQL](https://www.heidisql.com/)和[DBForge](https://www.devart.com/dbforge/)。

您还可以使用Manticore Search与[Kibana](Integration/Kibana.md)。

#### 简化流过滤
Manticore提供了一种特殊表类型，即"[percolate](Creating_a_table/Local_tables/Percolate_table.md)"表，允许您搜索查询而不是数据，使其成为高效过滤全文数据流的工具。只需将查询存储在表中，通过将每批文档发送到Manticore Search处理数据流，并接收匹配您存储查询的结果。

#### 可能的应用
Manticore Search具有广泛的应用场景，包括：

- **全文搜索**：
  - 适用于电子商务平台，实现快速准确的产品搜索，具有自动补全和模糊搜索等功能。
  - 适用于内容丰富的网站，使用户能够快速找到相关文章或文档。

- **数据分析**：
  - 使用[Beats/Logstash](https://manticoresearch.com/blog/integration-of-manticore-with-logstash-filebeat/)、[Vector.dev](https://manticoresearch.com/blog/integration-of-manticore-with-vectordev/)、[Fluentbit](https://manticoresearch.com/blog/integration-of-manticore-with-fluentbit/)将数据导入Manticore Search。
  - 使用Manticore的列式存储和OLAP功能高效分析大数据集。
  - 使用Manticore在几毫秒内对太字节的数据执行复杂查询。
  - 使用Kibana、[Grafana](https://manticoresearch.com/blog/manticoresearch-grafana-integration/)或[Apache Superset](https://manticoresearch.com/blog/manticoresearch-apache-superset-integration/)可视化数据。

- **分面搜索**：
  - 使用户能够通过价格、品牌或日期等类别过滤搜索结果，提供更精细的搜索体验。

- **地理空间搜索**：
  - 使用Manticore的地理空间功能实现基于位置的搜索，例如查找附近的餐厅或商店。

- **拼写纠正**：
  - 自动纠正用户在搜索查询中的拼写错误，提高搜索准确性和用户体验。

- **自动补全**：
  - 实时为用户输入提供建议，提高搜索的可用性和速度。

- **数据流过滤**：
  - 使用percolate表高效过滤和处理实时数据流，例如社交媒体帖子或日志数据。


## 要求

* 架构：arm64或x86_64
* 操作系统：基于Debian（例如Debian、Ubuntu、Mint）、基于RHEL（例如RHEL、CentOS、Alma、Oracle Linux、Amazon Linux）、Windows或MacOS。
* [Manticore列库](https://github.com/manticoresoftware/columnar)，提供[列式存储](Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)和[二级索引](Introduction.md#Automatic-secondary-indexes)，需要具有SSE >= 4.2的CPU。
* 对磁盘空间或RAM没有特定要求。一个空的Manticore Search实例仅使用大约40MB的RSS RAM。

<!-- proofread -->

