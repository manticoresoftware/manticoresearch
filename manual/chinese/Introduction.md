# 介绍

Manticore Search 是一个高性能、多存储数据库，专为搜索和分析而构建，提供闪电般快速的全文搜索、实时索引以及包括向量搜索和列存储在内的高级功能，以实现高效的数据分析。它设计用于处理小型和大型数据集，为现代应用提供无缝的可扩展性和强大的洞察力。

作为一个开源数据库（可在 [GitHub](https://github.com/manticoresoftware/manticoresearch/) 获取），Manticore Search 于 2017 年诞生，是对 [Sphinx Search](https://sphinxsearch.com/) 引擎的延续。我们的开发团队继承了 Sphinx 的所有优良特性，并显著提升了其功能，同时修复了数百个漏洞（详见我们的[更新日志](https://manual.manticoresearch.com/Changelog)）。Manticore Search 是一个现代、快速且轻量的数据库，具备卓越的全文搜索功能，基于对其前身近乎完全的重写构建而成。

## Manticore 的主要特点：
#### 强大且快速的全文搜索，适用于小型和大型数据集

  * [查询自动补全](Searching/Autocomplete.md)
  * [模糊搜索](Searching/Spell_correction.md#Fuzzy-Search)
  * 超过 20 种[全文操作符](https://play.manticoresearch.com/fulltextintro/)<!--{target="_blank"}-->和超过 20 种排名因素
  * [自定义排名](Searching/Sorting_and_ranking.md#Ranking-overview)
  * [词干提取](Creating_a_table/NLP_and_tokenization/Morphology.md)
  * [词形还原](Creating_a_table/NLP_and_tokenization/Morphology.md)
  * [停用词](Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md)
  * [同义词](Creating_a_table/NLP_and_tokenization/Exceptions.md)
  * [词形变体](Creating_a_table/NLP_and_tokenization/Wordforms.md)
  * [字符和单词级别的高级分词](Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md)
  * [准确的中文分词](Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md)
  * [文本高亮](Searching/Highlighting.md)

#### 向量搜索能力
Manticore Search 支持为每个文档添加由您的机器学习模型生成的嵌入向量，然后对它们进行[近邻搜索](Searching/KNN.md)。这让您能够构建类似度搜索、推荐系统、语义搜索以及基于自然语言处理算法的相关性排名等功能，还包括图像、视频和声音搜索等。

#### JOIN
Manticore Search 支持通过 SQL 和 JSON 的[JOIN](Searching/Joining.md) 查询，允许您合并来自多个表的数据。

#### 多线程
Manticore Search 利用智能查询并行化来降低响应时间，并在需要时充分利用所有 CPU 核心。

#### 基于成本的查询优化器
基于成本的查询优化器利用关于已索引数据的统计数据，评估给定查询不同执行计划的相对成本。这使得优化器能够确定检索所需结果的最有效执行计划，考虑的因素包括已索引数据的大小、查询的复杂度以及可用资源。

#### 存储选项
Manticore 提供[行式存储和列式存储选项](Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)，以适应各种规模的数据集。传统且默认的行式存储适用于小型、中型和大型数据集，而列式存储选项通过 Manticore 列存储库为更大规模的数据集提供支持。这两种存储选项的关键区别在于，行式存储为获得最佳性能，要求所有属性（全文字段除外）保持在内存中，而列式存储则不要求，因此内存消耗较低，但性能可能略微下降（具体可见 https://db-benchmarks.com/ 的统计数据）。

#### 自动辅助索引
[Manticore 列存储库](https://github.com/manticoresoftware/columnar/) 采用[分段几何模型索引](https://github.com/gvinciguerra/PGM-index)，利用已学习的映射关系将索引键与内存位置关联。这种映射的简洁性及独特的递归构建算法，使得 PGM 索引在空间效率方面远超传统索引，同时提供了极佳的查询和更新性能。所有数值和字符串字段的辅助索引默认开启，也可以为 JSON 属性启用。

#### 以 SQL 为先
Manticore 原生语法为 SQL，并支持通过 HTTP 和 MySQL 协议的 SQL，允许通过任何编程语言的流行 MySQL 客户端连接。

#### HTTP JSON
为了更程序化地管理数据和模式，Manticore 提供了类似 Elasticsearch 的 [HTTP JSON](Searching/Full_text_matching/Basic_usage.md#HTTP-JSON) 协议。

#### 兼容 Elasticsearch 的写入
您可以执行兼容 Elasticsearch 的 [insert](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Adding-documents-to-a-real-time-table) 和 [replace](Data_creation_and_modification/Updating_documents/REPLACE.md#REPLACE) JSON 查询，使得 Manticore 可以与 Logstash（版本小于 7.13）、Filebeat 及 Beats 家族其他工具结合使用。

#### 声明式和命令式模式管理
轻松在线或通过配置文件创建、更新和删除表。

#### C++ 的性能优势与 PHP 的便利性
Manticore Search守护进程采用C++开发，提供快速启动时间和高效的内存利用。底层优化进一步提升了性能。另一个关键组件称为[Manticore Buddy](https://github.com/manticoresoftware/manticoresearch-buddy)，使用PHP编写，主要用于不需要极快响应时间或极高处理能力的高级功能。尽管贡献C++代码可能具有挑战性，但通过Manticore Buddy添加新的SQL/JSON命令应该是一个简单的过程。

#### 实时插入
新添加或更新的文档可立即读取。

#### 互动课程轻松学习
我们提供[免费互动课程](https://play.manticoresearch.com/)，使学习变得轻松。

#### 事务
虽然Manticore未完全实现ACID，但支持原子更改的隔离事务和安全写入的二进制日志。

#### 内置复制和负载均衡
数据可以分布在服务器和数据中心，任何Manticore Search节点均可同时作为负载均衡器和数据节点。Manticore使用[Galera库](https://galeracluster.com/)实现几乎同步的多主[复制](https://play.manticoresearch.com/replication/)，确保所有节点数据一致，防止数据丢失并提供出色的复制性能。

#### 内置备份功能
Manticore配备了外部工具[manticore-backup](Securing_and_compacting_a_table/Backup_and_restore.md)和[BACKUP](Securing_and_compacting_a_table/Backup_and_restore.md#BACKUP-SQL-command-reference) SQL命令，简化备份和恢复过程。或者，您也可以使用[mysqldump](https://dev.mysql.com/doc/refman/8.0/en/mysqldump.html)来[制作逻辑备份](Securing_and_compacting_a_table/Backup_and_restore.md#Backup-and-restore-with-mysqldump)。

#### 开箱即用的数据同步
`indexer`工具和Manticore全面的配置语法，使得从MySQL、PostgreSQL、兼容ODBC的数据库、XML和CSV等源同步数据变得简单。

#### 集成选项
您可以使用[FEDERATED引擎](Extensions/FEDERATED.md)或通过[ProxySQL](https://manticoresearch.com/blog/using-proxysql-to-route-inserts-in-a-distributed-realtime-index/)将Manticore Search与MySQL/MariaDB服务器集成。

您可以使用[Apache Superset](https://manticoresearch.com/blog/manticoresearch-apache-superset-integration/)、Kibana和[Grafana](https://manticoresearch.com/blog/manticoresearch-grafana-integration/)来可视化存储在Manticore中的数据。各类MySQL工具如[HeidiSQL](https://www.heidisql.com/)和[DBForge](https://www.devart.com/dbforge/)可用来交互式开发Manticore查询。

您可以将Manticore Search与[Kibana](Integration/Kibana.md)一起使用。

#### 流过滤轻松实现
Manticore提供了一种特殊的表类型“[percolate](Creating_a_table/Local_tables/Percolate_table.md)”表，该表允许您搜索查询而非数据，是过滤全文数据流的高效工具。只需将查询存储在表中，处理数据流时将每批文档发送到Manticore Search，即可仅获取匹配存储查询的结果。

#### 可能的应用场景
Manticore Search具有多功能性，可应用于各种场景，包括：

- **全文搜索**：
  - 适合电商平台，实现快速准确的产品搜索，支持自动补全和模糊搜索等功能。
  - 适合内容丰富的网站，帮助用户快速查找相关文章或文档。

- **数据分析**：
  - 使用[Beats/Logstash](https://manticoresearch.com/blog/integration-of-manticore-with-logstash-filebeat/)、[Vector.dev](https://manticoresearch.com/blog/integration-of-manticore-with-vectordev/)、[Fluentbit](https://manticoresearch.com/blog/integration-of-manticore-with-fluentbit/)将数据导入Manticore Search。
  - 利用Manticore列式存储和OLAP能力高效分析大型数据集。
  - 在数TB数据上执行复杂查询，延迟极低。
  - 使用Kibana、[Grafana](https://manticoresearch.com/blog/manticoresearch-grafana-integration/)或[Apache Superset](https://manticoresearch.com/blog/manticoresearch-apache-superset-integration/)进行数据可视化。

- **分面搜索**：
  - 允许用户按照价格、品牌或日期等类别筛选搜索结果，实现更精确的搜索体验。

- **地理空间搜索**：
  - 利用Manticore的地理空间功能，实现基于位置的搜索，例如查找附近餐馆或商店。

- **拼写纠正**：
  - 自动纠正用户搜索查询中的错别字，提高搜索准确率和用户体验。

- **自动补全**：
  - 输入时提供实时建议，提升搜索的易用性和速度。

- **数据流过滤**：
  - 使用percolate表过滤和处理实时数据流，如社交媒体流或日志数据，效率高。


## 需求

* 架构：arm64或x86_64
* 操作系统：基于Debian（如Debian、Ubuntu、Mint）、基于RHEL（如RHEL、CentOS、Alma、Oracle Linux、Amazon Linux）、Windows或MacOS。
* [Manticore列式库](https://github.com/manticoresoftware/columnar)，提供[列式存储](Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)和[二级索引](Introduction.md#Automatic-secondary-indexes)，需要CPU支持SSE >= 4.2。
* 无特定磁盘空间或内存要求。一个空的Manticore Search实例仅使用约40MB的RSS内存。

<!-- proofread -->

