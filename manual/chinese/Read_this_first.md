# 首先阅读这个

## 关于本手册

本手册的安排反映了您使用 Manticore 的最可能方式：

* 从基本信息以及如何安装和连接开始
* 添加文档和运行搜索等基本事项
* 性能优化提示、技巧以及通过插件和自定义功能扩展 Manticore

##### 不要跳过 1️⃣ 2️⃣ 3️⃣
手册的关键部分在菜单中用 1️⃣、2️⃣、3️⃣ 等标记，以便您方便，因为它们对应的功能是最常用的。如果您是 Manticore 的新手 **我们强烈建议不要跳过它们**。

##### 快速入门指南
如果您想快速了解 Manticore 的一般工作方式，[⚡ 快速入门指南](Quick_start_guide.md) 是一个很好的起点。

##### 使用示例
每个查询示例的右上角都有一个小图标 📋：

![复制示例](copy_example.png)

您可以使用它复制示例到剪贴板。**如果查询是 HTTP 请求，它将被复制为 CURL 命令**。您可以按 ⚙️ 配置主机/端口。

##### 在本手册中搜索

我们喜爱搜索，并已尽力使在本手册中搜索变得尽可能方便。当然，它是由 Manticore Search 提供支持的。除了使用搜索框（需要先打开手册），还有一个非常简单的方法可以找到某些内容，只需打开 **mnt.cr/your-search-keyword**：

![mnt.cr 快速手册搜索](mnt.cr.gif)

## 最佳实践
您需要了解关于 Manticore Search 的一些事情，这可以帮助您遵循最佳实践来使用它。

#### 实时表与普通表
* **[实时表](Creating_a_table/Local_tables/Real-time_table.md)** 允许添加、更新和删除文档，并且更改可立即使用。
* **[普通表](Creating_a_table/Local_tables/Plain_table.md)** 是一种大部分不可变的数据结构，是实时表使用的基本元素。普通表存储一组文档、它们的公共字典和索引设置。一个实时表可以由多个普通表（块）组成，但 **此外 Manticore 提供直接访问构建普通表的功能**，使用工具 [indexer](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool)。当您的数据大部分不可变时，这很有意义，因此您不需要实时表。

#### 实时模式与普通模式
Manticore Search 有两种模式：
* **实时模式**（RT 模式）。这是默认模式，允许**命令式**地管理您的数据架构：
  * 允许使用 SQL 命令 `CREATE`/`ALTER`/`DROP TABLE` 及其在非 SQL 客户端中的等价命令在线管理您的数据架构
  * 在配置文件中，您只需定义与服务器相关的设置，包括 [data_dir](Server_settings/Searchd.md#data_dir)
* **普通模式** 允许在配置文件中定义您的数据架构，即提供**声明式**的架构管理。在三种情况下这很有意义：
  * 当您只处理普通表时
  * 或者当您的数据架构非常稳定且您不需要复制（因为只有在 RT 模式下才能使用）
  * 当您需要使您的数据架构可移植（例如，便于在新服务器上部署）

您不能将这两种模式结合在一起，必须通过在配置文件中指定 [data_dir](Server_settings/Searchd.md#data_dir) 来决定您想遵循哪一种模式（这是默认行为）。如果您不确定 **我们的建议是遵循 RT 模式**，因为即使您需要一个普通表，您可以使用单独的普通表配置来 [构建](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md) 它，并 [导入](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Importing_table.md) 到您的主 Manticore 实例。

实时表可以在 RT 和普通模式中使用。在 RT 模式下，实时表通过 `CREATE TABLE` 命令定义，而在普通模式中，它在配置文件中定义。普通（离线）表仅在普通模式下支持。普通表不能在 RT 模式下创建，但在普通模式下创建的现有普通表可以 [转换](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Attaching_one_table_to_another.md) 为实时表，并 [导入](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Importing_table.md) 到 RT 模式中。

#### SQL 与 JSON
Manticore 提供多种方式和接口来管理您的架构和数据，但主要有两种：
* **SQL**。这是 Manticore 的本地语言，它启用了所有 Manticore 的功能。**最佳实践是使用 SQL 来**：
  * 管理您的架构并执行其他 DBA 例程，因为这是最简单的方式
  * 设计您的查询，因为 SQL 与自然语言更加接近，这在您设计新事物时很重要。您可以通过任何 MySQL 客户端或 [/sql](Connecting_to_the_server/MySQL_protocol.md) 使用 Manticore SQL。
* **JSON**。大多数功能也可以通过 JSON 特定领域语言获得。这在您将 Manticore 集成到您的应用程序时特别有用，因为使用 JSON 您可以比使用 SQL 以更具程序化的方式进行。这种最佳实践是**首先探索如何通过 SQL 做某事，然后使用 JSON 将其集成到您的应用程序中。**

<!-- proofread -->
