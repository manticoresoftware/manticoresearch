# 首先阅读

## 关于本手册

本手册的编排反映了您最可能使用 Manticore 的方式：

* 从关于它的基本信息以及如何安装和连接开始
* 重要内容如添加文档和运行搜索
* 性能优化技巧、窍门以及通过插件和自定义函数扩展 Manticore

##### 不要跳过 1️⃣ 2️⃣ 3️⃣
手册的关键部分在菜单中用 1️⃣、2️⃣、3️⃣ 等标记，方便您查阅，因为对应的功能是最常用的。如果您是 Manticore 新手，**强烈建议不要跳过这些部分**。

##### 快速入门指南
如果您想快速了解 Manticore 的工作原理，[⚡ 快速入门指南](Quick_start_guide.md) 是一个很好的起点。

##### 使用示例
每个查询示例右上角都有一个小图标 📋：

![复制示例](copy_example.png)

您可以用它将示例复制到剪贴板。**如果查询是 HTTP 请求，它将被复制为 CURL 命令**。按下 ⚙️ 可以配置主机/端口。

##### 在本手册中搜索

我们热爱搜索，并尽力使本手册中的搜索尽可能方便。当然，它是由 Manticore Search 支持的。除了使用需要先打开手册的搜索栏外，还有一种非常简单的方式，只需打开 **mnt.cr/your-search-keyword** 即可查找内容：

![mnt.cr 快速手册搜索](mnt.cr.gif)

## 最佳实践
您需要了解关于 Manticore Search 的一些内容，这有助于您遵循最佳使用实践。

#### 实时表与普通表
* **[实时表](Creating_a_table/Local_tables/Real-time_table.md)** 允许添加、更新和删除文档，且更改立即生效。
* **[普通表](Creating_a_table/Local_tables/Plain_table.md)** 是一种大部分不可变的数据结构，是实时表的基本元素。普通表存储一组文档、它们的公共字典和索引设置。一个实时表可以由多个普通表（块）组成，但**除此之外，Manticore 还提供了使用工具 [indexer](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) 直接构建普通表的功能**。当您的数据大部分不可变时，这很有意义，因此您不需要实时表。

#### 实时模式与普通模式
Manticore Search 有两种工作模式：
* **实时模式**（RT 模式）。这是默认模式，允许您**命令式地**管理数据模式：
  * 允许使用 SQL 命令 `CREATE`/`ALTER`/`DROP TABLE` 及其非 SQL 客户端等效命令在线管理数据模式
  * 在配置文件中只需定义与服务器相关的设置，包括 [data_dir](Server_settings/Searchd.md#data_dir)
* **普通模式** 允许您在配置文件中定义数据模式，即提供**声明式**的模式管理。这在以下三种情况下有意义：
  * 当您只处理普通表时
  * 或者当您的数据模式非常稳定且不需要复制（复制仅在 RT 模式下可用）
  * 当您需要使数据模式可移植（例如，便于在新服务器上部署）

您不能同时使用两种模式，需要通过在配置文件中指定 [data_dir](Server_settings/Searchd.md#data_dir) 来决定使用哪种模式（这是默认行为）。如果不确定，**我们的建议是使用 RT 模式**，因为即使您需要普通表，也可以通过单独的普通表配置[构建](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md)它，并[导入](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Importing_table.md)到您的主 Manticore 实例。

实时表可以在 RT 和普通模式下使用。在 RT 模式下，实时表通过 `CREATE TABLE` 命令定义，而在普通模式下则在配置文件中定义。普通（离线）表仅支持普通模式。普通表不能在 RT 模式下创建，但在普通模式下创建的现有普通表可以[转换](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Attaching_one_table_to_another.md)为实时表，并在 RT 模式下[导入](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Importing_table.md)。

#### SQL 与 JSON
Manticore 提供多种方式和接口来管理您的模式和数据，但主要有两种：
* **SQL**。这是 Manticore 的原生语言，支持所有 Manticore 功能。**最佳实践是使用 SQL 来**：
  * 管理您的模式和执行其他数据库管理员操作，因为这是最简单的方式
  * 设计查询，因为 SQL 比 JSON DSL 更接近自然语言，这在设计新内容时很重要。您可以通过任何 MySQL 客户端或 [/sql](Connecting_to_the_server/MySQL_protocol.md) 使用 Manticore SQL。
* **JSON**。大部分功能也可通过 JSON 领域特定语言实现。这在将 Manticore 集成到您的应用程序时尤其有用，因为使用 JSON 可以比 SQL 更程序化地实现。最佳实践是**先了解如何通过 SQL 实现某功能，然后用 JSON 将其集成到您的应用程序中。**

<!-- proofread -->

