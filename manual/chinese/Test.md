# 介绍

Manticore Search 是一个高性能、多存储数据库，专为搜索和分析而构建，提供闪电般快速的全文搜索、实时索引，以及向量搜索和列存储等先进功能，助力高效数据分析。它设计用于处理小型和大型数据集，实现无缝扩展并为现代应用提供强大洞察力。

作为一个开源数据库（可在[GitHub](https://github.com/manticoresoftware/manticoresearch/)获取），Manticore Search 创建于2017年，是[Sphinx Search](https://sphinxsearch.com/)引擎的延续。我们的开发团队吸收了 Sphinx 的所有最佳功能，并显著改进了其功能，在此过程中修复了数百个漏洞（详见我们的[更新日志](https://manual.manticoresearch.com/Changelog)）。Manticore Search 是一个现代、快速且轻量级的数据库，具有卓越的全文搜索能力，基于其前身的几乎完全重写版本构建。

## Manticore 的主要特性：
#### 强大快速的全文搜索，适用于小型和大型数据集

  * [查询自动补全](Searching/Autocomplete.md)
  * [模糊搜索](Searching/Spell_correction.md#Fuzzy-Search)
  * 超过20种[全文操作符](https://play.manticoresearch.com/fulltextintro/)<!--{target="_blank"}-->及20多种排名因子
  * [自定义排序](Searching/Sorting_and_ranking.md#Ranking-overview)
  * [词干提取](Creating_a_table/NLP_and_tokenization/Morphology.md)
  * [词形还原](Creating_a_table/NLP_and_tokenization/Morphology.md)
  * [停用词](Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md)
  * [同义词](Creating_a_table/NLP_and_tokenization/Exceptions.md)
  * [词形变化](Creating_a_table/NLP_and_tokenization/Wordforms.md)
  * [字符及词级别的高级分词](Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md)
  * [准确的中文分词](Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md)
  * [文本高亮](Searching/Highlighting.md)

#### 向量搜索能力
Manticore Search 支持将您的机器学习模型生成的嵌入（embeddings）添加到每个文档中，然后对其进行[最近邻搜索](Searching/KNN.md)。这使您可以构建相似性搜索、推荐、语义搜索以及基于自然语言处理算法的相关排序等功能，还包括图像、视频和音频搜索。

#### JOIN
Manticore Search 通过 SQL 和 JSON 支持[JOIN](Searching/Joining.md)查询，允许您合并来自多个表的数据。

#### 多线程
Manticore Search 利用智能查询并行化，降低响应时间，并在需要时充分利用所有 CPU 核心。

#### 基于成本的查询优化器
基于成本的查询优化器利用关于索引数据的统计信息，评估给定查询不同执行计划的相对成本。这样，优化器可以确定最有效的执行计划以获取所需结果，同时考虑索引数据的大小、查询复杂性和可用资源等因素。

#### 存储选项
Manticore 提供[行存储和列存储选项](Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)，适应各种规模的数据集。传统且默认的行存储适用于各种规模数据集——小型、中型和大型，而通过 Manticore Columnar Library 提供的列存储选项适用于更大型的数据集。两种存储的关键区别在于，行存储要求所有属性（不包括全文字段）保存在内存中以获得最佳性能，列存储则不需要，从而提供更低的内存消耗，但性能可能略有降低（可参考 https://db-benchmarks.com/ 上的统计数据）。

#### 自动二级索引
[Manticore Columnar Library](https://github.com/manticoresoftware/columnar/) 使用[分段几何模型索引](https://github.com/gvinciguerra/PGM-index)，利用索引键与其内存位置之间的学习映射。这种映射的简洁性，配合特殊的递归构造算法，使 PGM 索引在空间占用上远超传统索引，并且仍然提供最佳的查询和更新性能。二级索引默认开启，适用于所有数值型和字符串字段，也可为 JSON 属性启用。

#### SQL 优先
Manticore 的原生语法是 SQL，支持基于 HTTP 的 SQL 以及 MySQL 协议，允许通过任意编程语言的常用 MySQL 客户端连接。

#### 基于 HTTP 的 JSON
为更程序化地管理数据和模式，Manticore 提供了类似 Elasticsearch 的[HTTP JSON](Searching/Full_text_matching/Basic_usage.md#HTTP-JSON)协议。

#### 兼容 Elasticsearch 的写入
您可以执行兼容 Elasticsearch 的[插入](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Adding-documents-to-a-real-time-table)和[替换](Data_creation_and_modification/Updating_documents/REPLACE.md#REPLACE) JSON 查询，使 Manticore 能与 Logstash（版本 < 7.13）、Filebeat 以及 Beats 系列其他工具协同使用。

#### 声明式和命令式的模式管理
轻松地在线或通过配置文件创建、更新和删除表。

#### C++ 的优势与 PHP 的便利性
Manticore Search守护进程采用C++开发，提供快速启动时间和高效的内存利用。低级优化进一步提升性能。另一个关键组件叫做 [Manticore Buddy](https://github.com/manticoresoftware/manticoresearch-buddy)，用PHP编写，用于不需要极快响应时间或极高处理能力的高级功能。虽然参与C++代码的开发可能具有挑战性，但使用Manticore Buddy添加新的SQL/JSON命令应该是一个简单的过程。

#### 实时插入
新添加或更新的文档可以立即读取。

#### 交互式课程，轻松学习
我们提供[免费交互式课程](https://play.manticoresearch.com/)，让学习变得轻松。

#### 事务
虽然Manticore并非完全符合ACID，但支持隔离事务以实现原子更改，并支持二进制日志以确保写入安全。

#### 内置复制与负载均衡
数据可以分布在多个服务器和数据中心，任何Manticore Search节点都可以同时作为负载均衡器和数据节点。Manticore采用 [Galera库](https://galeracluster.com/) 实现几乎同步的多主[复制](https://play.manticoresearch.com/replication/)，确保所有节点数据一致，防止数据丢失，并提供卓越的复制性能。

#### 内置备份功能
Manticore配备了外部工具 [manticore-backup](Securing_and_compacting_a_table/Backup_and_restore.md) 和 [BACKUP](Securing_and_compacting_a_table/Backup_and_restore.md#BACKUP-SQL-command-reference) SQL命令，简化数据备份与恢复流程。或者，您也可以使用 [mysqldump](https://dev.mysql.com/doc/refman/8.0/en/mysqldump.html) 来[制作逻辑备份](Securing_and_compacting_a_table/Backup_and_restore.md#Backup-and-restore-with-mysqldump)。

#### 开箱即用的数据同步
`indexer` 工具及Manticore的全面配置语法，可轻松将数据从MySQL、PostgreSQL、兼容ODBC的数据库、XML和CSV等源同步。

#### 集成选项
您可以使用[FEDERATED引擎](Extensions/FEDERATED.md)或通过[ProxySQL](https://manticoresearch.com/blog/using-proxysql-to-route-inserts-in-a-distributed-realtime-index/)与MySQL/MariaDB服务器集成Manticore Search。

您可以使用[Apache Superset](https://manticoresearch.com/blog/manticoresearch-apache-superset-integration/)、Kibana及[Grafana](https://manticoresearch.com/blog/manticoresearch-grafana-integration/)来可视化存储在Manticore中的数据。多种MySQL工具可用于交互式开发Manticore查询，如[HeidiSQL](https://www.heidisql.com/)和[DBForge](https://www.devart.com/dbforge/)。

您可以将Manticore Search与[Kibana](Integration/Kibana.md)一起使用。

#### 流过滤变得简单
Manticore提供一种特殊的表类型——“[percolate](Creating_a_table/Local_tables/Percolate_table.md)”表，它允许搜索查询而非数据，是过滤全文数据流的高效工具。只需将查询存储在表中，通过向Manticore Search发送数据流中的每批文档处理数据流，同时只接收匹配存储查询的结果。

#### 可能的应用场景
Manticore Search多功能，可应用于多种场景，包括：

- **全文检索**：
  - 适用于电子商务平台，实现快速精准的产品搜索，并支持自动完成及模糊搜索等功能。
  - 适合内容丰富的网站，允许用户快速找到相关的文章或文档。

- **数据分析**：
  - 使用[Beats/Logstash](https://manticoresearch.com/blog/integration-of-manticore-with-logstash-filebeat/)、[Vector.dev](https://manticoresearch.com/blog/integration-of-manticore-with-vectordev/)、[Fluentbit](https://manticoresearch.com/blog/integration-of-manticore-with-fluentbit/)将数据导入Manticore Search。
  - 利用Manticore的列存储和OLAP能力，高效分析大型数据集。
  - 对数TB数据执行复杂查询，延迟极低。
  - 使用Kibana、[Grafana](https://manticoresearch.com/blog/manticoresearch-grafana-integration/)或[Apache Superset](https://manticoresearch.com/blog/manticoresearch-apache-superset-integration/)进行数据可视化。

- **分面搜索**：
  - 允许用户按类别（如价格、品牌或日期）过滤搜索结果，提供更精细的搜索体验。

- **地理空间搜索**：
  - 利用Manticore的地理空间功能，实现基于位置的搜索，如查找附近的餐馆或商店。

- **拼写纠错**：
  - 自动纠正用户搜索查询中的拼写错误，提升搜索准确度和用户体验。

- **自动补全**：
  - 用户输入时实时提供建议，增强搜索的易用性和速度。

- **数据流过滤**：
  - 使用percolate表高效过滤和处理实时数据流，如社交媒体信息流或日志数据。


## 需求

* 架构：arm64或x86_64
* 操作系统：基于Debian（如Debian、Ubuntu、Mint）、基于RHEL（如RHEL、CentOS、Alma、Oracle Linux、Amazon Linux）、Windows或MacOS。
* [Manticore Columnar Library](https://github.com/manticoresoftware/columnar)，提供[列存储](Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)和[二级索引](Introduction.md#Automatic-secondary-indexes)，需要支持SSE >= 4.2的CPU。
* 无具体磁盘空间或内存要求。空白的Manticore Search实例仅使用约40MB的RSS内存。

<!-- proofread -->

