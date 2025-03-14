# 首先阅读

## 关于本手册

本手册的编排旨在反映您使用 Manticore 的最可能路径：

* 从有关 Manticore 的基本信息以及如何安装和连接开始
* 处理诸如添加文档和运行搜索等基本操作
* 提供性能优化提示、技巧以及通过插件和自定义函数扩展 Manticore 的方法

##### 不要跳过  1️⃣ 2️⃣ 3️⃣
手册中的关键部分在菜单中用 1️⃣, 2️⃣, 3️⃣ 等标记，以便您查阅这些最常用的功能。如果您是 Manticore 新手，**我们强烈建议不要跳过它们**。

##### 快速入门指南
如果您想快速了解 Manticore 的工作原理，[⚡ 快速入门指南](Quick_start_guide.md) 是一个很好的起点。

##### 使用示例
每个查询示例的右上角都有一个小图标 📋：

![Copy example](copy_example.png)⛔

您可以使用它将示例复制到剪贴板。**如果查询是 HTTP 请求，它将作为 CURL 命令被复制**。您可以按 ⚙️ 来配置主机/端口。

##### 在本手册中搜索

我们热爱搜索，并尽最大努力使在本手册中搜索尽可能方便。当然，这背后使用的是 Manticore Search。除了使用需要先打开手册的搜索栏之外，还有一种非常简单的方法，只需打开 **mnt.cr/your-search-keyword** 即可找到您需要的内容：

![mnt.cr quick manual search](mnt.cr.gif)⛔

## 最佳实践
了解以下一些关于 Manticore Search 的要点可以帮助您遵循最佳实践并更好地使用它。

#### 实时表 vs 普通表
* [**实时表**](Creating_a_table/Local_tables/Real-time_table.md) 允许添加、更新和删除文档，并能立即查看更改。
* [**普通表**](Creating_a_table/Local_tables/Plain_table.md) 是一种几乎不可变的数据结构，也是实时表的基础组成部分。普通表存储一组文档及其公共词典和索引设置。一个实时表可以由多个普通表（块）组成，但**Manticore 还提供直接使用工具** [**indexer**](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-工具) **构建普通表的访问**。当数据大多是不可变时，使用普通表更加合理，因此不需要实时表。

#### 实时模式 vs 普通模式
Manticore Search 有两种运行模式：
* **实时模式**（RT 模式）。这是默认模式，允许通过 SQL 命令 **命令式地** 管理数据架构：
  * 允许在线使用 SQL 命令 CREATE/ALTER/DROP TABLE 以及非 SQL 客户端的等效命令来管理数据架构
  * 在配置文件中，只需定义与服务器相关的设置，包括 [data_dir](Server_settings/Searchd.md#data_dir)
* **普通模式** 允许在配置文件中定义数据架构，即提供**声明式**的架构管理方式。它在以下三种情况下是有意义的：
  * 当仅处理普通表时
  * 或者当数据架构非常稳定且不需要复制（因为复制仅在实时模式下可用）
  * 当需要使数据架构可移植（例如，便于在新服务器上部署）

这两种模式不能混合使用，您需要通过在配置文件中指定 [data_dir](Server_settings/Searchd.md#data_dir) 决定使用哪一种模式（这是默认行为）。如果不确定，**我们建议使用实时模式**，即使需要普通表，您也可以使用单独的普通表配置文件来[构建](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md)它，然后[导入](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Importing_table.md)到您的主 Manticore 实例。

实时表可以在实时模式和普通模式下使用。在实时模式下，实时表通过 CREATE TABLE 命令定义，而在普通模式下，它在配置文件中定义。普通（离线）表仅支持普通模式。普通表不能在实时模式下创建，但可以将普通模式中创建的普通表[转换](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Attaching_one_table_to_another.md)为实时表，并在实时模式中[导入](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Importing_table.md)。

#### SQL vs JSON
Manticore 提供多种管理架构和数据的方式，主要有两种接口：
* **SQL**。这是 Manticore 的原生语言，能够使用所有 Manticore 功能。**最佳实践是使用 SQL 来**：
  * 管理架构并进行其他数据库管理任务，因为这是最简单的方式
  * 设计查询，因为 SQL 比 JSON 的领域专用语言（DSL）更接近自然语言，这在设计新功能时尤为重要。您可以通过任何 MySQL 客户端或 [/sql](Connecting_to_the_server/MySQL_protocol.md) 使用 Manticore SQL。
* **JSON**。大多数功能也可以通过 JSON 领域专用语言实现。JSON 特别适合在将 Manticore 与应用程序集成时使用，因为通过 JSON 可以更具程序化地进行集成操作。**最佳实践是先通过 SQL 探索如何操作，然后使用 JSON 将其集成到应用程序中。**

<!-- proofread -->
