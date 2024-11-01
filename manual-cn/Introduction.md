# 介绍

Manticore Search 是一个专门为搜索设计的多存储数据库，具有强大的全文搜索能力。

作为一个开源数据库（在 [GitHub](https://github.com/manticoresoftware/manticoresearch/) 上可用），Manticore Search 于 2017 年创建，延续了[Sphinx 搜索引擎](https://sphinxsearch.com/)。我们的开发团队整合了 Sphinx 的所有优点，并显著改善了其功能，修复了数百个错误（详见我们的[更新日志](https://manual.manticoresearch.com/Changelog)）。通过几乎完全的代码重写，Manticore Search 现在是一个现代化、快速且轻量级的数据库，具有完整的功能和卓越的全文搜索能力。

## Manticore 的关键特性包括：
#### 强大且快速的全文搜索，适用于小型和大型数据集

  * 超过 20 种[全文操作符](https://play.manticoresearch.com/fulltextintro/)<!--{target="_blank"}--> 和 20 种排名因子
  * 自定义排名
  * [词干提取](Creating_a_table/NLP_and_tokenization/Morphology.md)
  * [词形还原](Creating_a_table/NLP_and_tokenization/Morphology.md)
  * [停用词](Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md)
  * [同义词](Creating_a_table/NLP_and_tokenization/Exceptions.md)
  * [词形](Creating_a_table/NLP_and_tokenization/Wordforms.md)
  * [字符和单词级的高级分词](Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md)
  * [适当的中文分词](Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md)
  * [文本高亮](Searching/Highlighting.md)

#### 向量搜索能力

Manticore Search 支持将机器学习模型生成的嵌入向量添加到每个文档中，并基于这些嵌入进行[最近邻搜索](Searching/KNN.md)。这使您能够构建诸如相似性搜索、推荐、语义搜索以及基于 NLP 算法的相关性排名等功能，甚至可以应用于图像、视频和音频搜索等场景。

#### 多线程
Manticore Search 采用智能查询并行化，以降低响应时间并在需要时充分利用所有 CPU 核心。

#### 基于成本的查询优化器
基于成本的查询优化器使用关于索引数据的统计数据来评估给定查询不同执行计划的相对成本。这使优化器能够确定检索所需结果的最有效计划，考虑到索引数据的大小、查询的复杂性以及可用资源等因素。

#### 存储选项
Manticore 提供[行式和列式存储选项](Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) ，以适应各种大小的数据集。传统的默认行式存储选项适用于所有大小的小型、中型和大型数据集，而列式存储选项通过 Manticore 列库提供，适用于更大的数据集。这些存储选项之间的主要区别在于，行式存储要求所有属性（排除全文字段）保存在 RAM 中以实现最佳性能，而列式存储则不需要，从而提供较低的 RAM 消耗，但可能在性能上稍慢（如在 https://db-benchmarks.com/ 上的统计数据所示）。

#### 自动二级索引
[Manticore Columnar Library](https://github.com/manticoresoftware/columnar/) 使用 [Piecewise Geometric Model index](https://github.com/gvinciguerra/PGM-index)，该索引利用了索引键与其在内存中位置之间的学习映射。该映射的简洁性，再加上独特的递归构建算法，使得 PGM 索引在空间占用方面远远优于传统索引，同时仍能提供最佳的查询和更新时间性能。所有数值字段的二级索引默认是开启的。

#### SQL 优先
Manticore 的原生语法是 SQL，支持通过 HTTP 和 MySQL 协议进行 SQL 查询，允许通过任何编程语言的流行 MySQL 客户端进行连接。

#### 通过 HTTP 的 JSON
对于更具程序化的数据和模式管理，Manticore 提供 HTTP JSON 协议，类似于 Elasticsearch。

#### 与 Elasticsearch 兼容的写入
你可以执行与 Elasticsearch 兼容的[插入](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Adding-documents-to-a-real-time-table)和[替换](Data_creation_and_modification/Updating_documents/REPLACE.md#REPLACE) JSON 查询，这使得 Manticore 可以与 Logstash（版本 < 7.13）、Filebeat 及其他 Beats 工具配合使用。

### 声明式和命令式架构管理

轻松在线或通过配置文件创建、更新和删除表。

### C++ 的优势与 PHP 的便利

Manticore Search 守护进程用 C++ 开发，提供快速的启动时间和高效的内存利用。低级优化的利用进一步提高了性能。另一个关键组件 [Manticore Buddy](https://github.com/manticoresoftware/manticoresearch-buddy) 用 PHP 编写，适用于不需要超快响应时间或极高处理能力的高级功能。尽管对 C++ 代码的贡献可能具有挑战性，但使用 Manticore Buddy 添加新的 SQL/JSON 命令应该是一个简单的过程。

### 实时插入

新添加或更新的文档可以立即被读取。

### 互动课程，轻松学习

我们提供[免费的互动课程](https://play.manticoresearch.com/)，让学习变得轻松。

### 事务

虽然 Manticore 不是完全符合 ACID 的，但它支持隔离事务以实现原子性变更，并提供二进制日志以确保安全写入。

#### 内置复制与负载均衡
数据可以分布在服务器和数据中心，任何 Manticore Search 节点都可以充当负载均衡器和数据节点。Manticore 使用 [Galera 库](https://galeracluster.com/)实现几乎同步的多主[复制](https://play.manticoresearch.com/replication/)，确保所有节点之间的数据一致性，防止数据丢失，并提供卓越的复制性能。

#### 内置备份功能
Manticore 配备了外部工具[manticore-backup](Securing_and_compacting_a_table/Backup_and_restore.md)和 [BACKUP](Securing_and_compacting_a_table/Backup_and_restore.md#BACKUP-SQL-command-reference)  SQL 命令，简化数据备份和恢复的过程。你也可以使用 [mysqldump](https://dev.mysql.com/doc/refman/8.0/en/mysqldump.html)  进行[make logical backups](../Securing_and_compacting_a_table/Backup_and_restore.md#Backup-and-restore-with-mysqldump)。

#### 开箱即用的数据同步
Manticore 的索引工具和全面的配置语法使得从 MySQL、PostgreSQL、ODBC 兼容数据库、XML 和 CSV 等来源同步数据变得简单。

#### 集成选项
你可以通过 [FEDERATED 引擎](Extensions/FEDERATED.md)或 [ProxySQL](https://manticoresearch.com/2018/06/18/using-proxysql-to-route-inserts-in-a-distributed-realtime-index/) 将 Manticore Search 与 MySQL/MariaDB 服务器集成。

你可以使用 [Apache Superset](https://manticoresearch.com/blog/manticoresearch-apache-superset-integration/) 和  [Grafana](https://manticoresearch.com/blog/manticoresearch-grafana-integration/) 来可视化存储在 Manticore 中的数据。各种 MySQL 工具（如 [HeidiSQL](https://www.heidisql.com/) 和 [DBForge](https://www.devart.com/dbforge/)）可以用来交互式地开发 Manticore 查询。

#### 简化流过滤
Manticore 提供了一种特殊的表类型，称为“过筛”表，允许你搜索查询而不是数据，使其成为过滤全文数据流的有效工具。只需将查询存储在表中，处理数据流时将每批文档发送到 Manticore Search，然后仅接收与你存储的查询匹配的结果。

# 可能的应用场景：
Manticore 具有多种用例，包括：

  * [全文搜索](https://play.manticoresearch.com/fulltextintro/)
    * 对于小数据量，享受强大全文搜索语法和低内存消耗（低至 7-8 MB）。
    * 对于大数据，受益于 Manticore 的高可用性和处理大表的能力。
  * OLAP：使用 Manticore Search 和  [Manticore Columnar Library](https://github.com/manticoresoftware/columnar)分析单台或多台服务器上的 TB 级数据。
  * [分面搜索](https://play.manticoresearch.com/faceting/)
  * [地理空间搜索](https://play.manticoresearch.com/geosearch/)
  * [拼写校正](https://play.manticoresearch.com/didyoumean/)
  * [自动补全](https://play.manticoresearch.com/simpleautocomplete/)
  * [数据流过滤](https://play.manticoresearch.com/pq/)

# 需求

* 架构： arm64 或 x86_64
* 操作系统： 基于 Debian（如 Debian、Ubuntu、Mint）、基于 RHEL（如 RHEL、CentOS、Alma、Oracle Linux、Amazon Linux）、Windows 或 MacOS。
* [Manticore 列库](https://github.com/manticoresoftware/columnar)提供列式存储和[二级索引](introduction.md#Automatic-secondary-indexes)，要求 CPU 支持 SSE >= 4.2。
* 没有特定的磁盘空间或 RAM 要求。一个空的 Manticore Search 实例仅使用约 40MB 的 RSS RAM。

<!-- proofread -->
