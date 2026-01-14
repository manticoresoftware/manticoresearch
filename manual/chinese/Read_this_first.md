# 请先阅读本文

## 关于本手册

本手册的编排旨在反映您最可能使用 Manticore 的方式：

* 从关于它的基本信息以及如何安装和连接开始
* 添加文档和运行搜索等基本操作
* 性能优化技巧、窍门，以及借助插件和自定义函数扩展 Manticore

##### 不要跳过 1️⃣ 2️⃣ 3️⃣
为了方便起见，手册的关键部分在菜单中用 1️⃣、2️⃣、3️⃣ 等标记，因为它们对应的功能最常用。如果您是 Manticore 的新手，**我们强烈建议不要跳过它们**。

##### 快速入门指南
如果您想快速了解 Manticore 的一般工作原理，[⚡ 快速入门指南](Quick_start_guide.md) 是一个很好的起点。

##### 使用示例
每个查询示例的右上角都有一个小图标 📋：

![复制示例](copy_example.png)

您可以使用它将示例复制到剪贴板。**如果查询是 HTTP 请求，它将被复制为 CURL 命令**。您可以按 ⚙️ 配置主机/端口。

##### 在本手册中搜索

我们热爱搜索，并已尽最大努力使在本手册中搜索尽可能方便。当然，它由 Manticore Search 提供支持。除了使用需要先打开手册的搜索栏外，还有一种非常简单的方法可以找到某些内容，只需打开 **mnt.cr/您的搜索关键词**：

![mnt.cr 快速手册搜索](mnt.cr.gif)

## 最佳实践
关于 Manticore Search，有几件事您需要了解，这有助于您遵循使用它的最佳实践。

#### 实时表与普通表
* **[实时表](Creating_a_table/Local_tables/Real-time_table.md)** 允许添加、更新和删除文档，并且更改立即可用。
* **[普通表](Creating_a_table/Local_tables/Plain_table.md)** 是一种基本不可变的数据结构，也是实时表使用的基本元素。普通表存储一组文档、它们的公共字典和索引设置。一个实时表可以由多个普通表（块）组成，但**除此之外，Manticore 提供了使用工具 [indexer](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) 直接构建普通表**的途径。当您的数据基本不可变时，这样做是有意义的，因此您不需要为此使用实时表。

#### 实时模式与普通模式
Manticore Search 在两种模式下工作：
* **实时模式**（RT 模式）。这是默认模式，允许**命令式**管理您的数据模式：
  * 允许使用 SQL 命令 `CREATE`/`ALTER`/`DROP TABLE` 及其在非 SQL 客户端中的等效命令在线管理您的数据模式
  * 在配置文件中，您只需要定义与服务器相关的设置，包括 [data_dir](Server_settings/Searchd.md#data_dir)
* **普通模式** 允许在配置文件中定义您的数据模式，即提供**声明式**的模式管理。这在三种情况下有意义：
  * 当您只处理普通表时
  * 或者当您的数据模式非常稳定且不需要复制时（因为复制仅在 RT 模式下可用）
  * 当您需要使您的数据模式可移植时（例如，为了更容易地在新服务器上部署）

您不能混合使用这两种模式，需要通过指定配置文件中的 [data_dir](Server_settings/Searchd.md#data_dir) 来决定要遵循哪种模式（这是默认行为）。如果您不确定，**我们的建议是遵循 RT 模式**，因为即使您需要一个普通表，您也可以使用单独的普通表配置[构建](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md)它，并[导入](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Importing_table.md)到您的主 Manticore 实例中。

实时表可以在 RT 模式和普通模式下使用。在 RT 模式下，实时表使用 `CREATE TABLE` 命令定义，而在普通模式下，它在配置文件中定义。普通（离线）表仅在普通模式下受支持。普通表不能在 RT 模式下创建，但在普通模式下创建的现有普通表可以[转换](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Attaching_one_table_to_another.md)为实时表并在 RT 模式下[导入](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Importing_table.md)。

#### SQL 与 JSON
Manticore 提供了多种方式和接口来管理您的模式和数据，但两个主要的是：
* **SQL**。这是 Manticore 的原生语言，支持 Manticore 的所有功能。**最佳实践是使用 SQL 来**：
  * 管理您的模式并执行其他 DBA 例程，因为这是最简单的方法
  * 设计您的查询，因为 SQL 比 JSON DSL 更接近自然语言，这在您设计新内容时很重要。您可以通过任何 MySQL 客户端或 [/sql](Connecting_to_the_server/MySQL_protocol.md) 使用 Manticore SQL。
* **JSON**。大多数功能也可以通过 JSON 领域特定语言使用。当您将 Manticore 集成到您的应用程序中时，这尤其有用，因为使用 JSON 可以比使用 SQL 更以编程方式实现。最佳实践是**首先探索如何通过 SQL 完成某些操作，然后使用 JSON 将其集成到您的应用程序中。**

<!-- proofread -->

