# 与 Apache Superset 集成

> 注意：与 Apache Superset 的集成需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果无法使用，请确保已安装 Buddy。

[Apache Superset](https://superset.apache.org/) 是一个现代的数据探索和可视化平台。Manticore Search 可以与 Apache Superset 集成，使用户能够充分利用 Manticore Search 的全部潜力进行数据分析和可视化。

**注意：此集成仅适用于 Apache Superset 3.x 分支，尚不支持版本 4 或更高。**

## 前提条件

在将 Manticore Search 与 Apache Superset 集成之前，请确保：

1. 已正确安装并配置 Manticore Search（版本 6.2.0 或更高）在您的服务器上。请参考 [官方 Manticore Search 安装指南](https://manual.manticoresearch.com/Installation/Installation)。
2. 已在您的系统上设置 Apache Superset。请按照 [官方 Apache Superset 安装指南](https://superset.apache.org/docs/quickstart) 进行安装。

## 连接 Manticore Search 到 Apache Superset

连接 Manticore Search 到 Apache Superset 的步骤：

1. 登录到您的 Apache Superset 仪表盘。
2. 点击顶部导航栏中的“+”，选择“数据库连接”。
3. 点击“+ DATABASE”添加新的数据库。
4. 选择“MySQL”作为数据库类型。
5. 在配置页面填写以下信息：
   - 数据库名称：数据库的名称（例如，“Manticore”）
   - 主机：您的 Manticore Search 服务器主机
   - 端口：Manticore Search 运行的端口
   - 用户名：您的 Manticore Search 用户名
   - 显示名称：连接的显示名称
6. 点击“Connect”验证连接并保存。

## 创建图表和仪表盘

连接 Manticore Search 到 Apache Superset 后，您可以使用 Manticore 数据创建图表和仪表盘：

1. 在 Apache Superset 仪表盘中，点击顶部导航栏的“仪表盘”，选择“新建仪表盘”。
2. 打开仪表盘，点击“+”图标添加新图表。
3. 选择连接到 Manticore Search 的数据集。
4. 选择希望创建的图表类型（如条形图、折线图、饼图）。
5. 使用 Apache Superset 的查询构建器或编写 SQL 查询以从 Manticore Search 数据库提取数据。
6. 根据需要自定义图表的外观、标签及其他设置。
7. 点击“更新图表”更新图表。
8. 为图表命名，点击“保存”将其永久保存在仪表盘中。

## 支持的功能

通过 Apache Superset 使用 Manticore Search 时，您可以：

- 查看并查询来自 Manticore Search 表的数据
- 基于 Manticore 数据创建各种类型的可视化
- 使用这些可视化构建和定制仪表盘

## 数据类型处理

在 Apache Superset 中，Manticore Search 的数据类型映射为 MySQL 数据类型。有关支持的数据类型及其映射的详细信息，请参阅 Manticore Search 文档。

## 限制

- 在通过 Apache Superset 使用 Manticore Search 时，某些高级 MySQL 功能可能不可用。
- 与直接使用 MySQL 数据库相比，某些数据操作可能受限。

通过遵循这些步骤和指南，您可以有效地将 Manticore Search 与 Apache Superset 集成，实现强大的数据探索和可视化能力。

## 参考资料
有关 Apache Superset 和 Manticore Search 的更多信息，请参阅以下资源：
- [Apache Superset 集成](https://manticoresearch.com/blog/manticoresearch-apache-superset-integration/)

