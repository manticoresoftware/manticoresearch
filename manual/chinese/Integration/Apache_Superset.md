# 与 Apache Superset 的集成

> 注意：与 Apache Superset 的集成需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果无法正常工作，请确保已安装 Buddy。

[Apache Superset](https://superset.apache.org/) 是一个现代的数据探索和可视化平台。Manticore Search 可以与 Apache Superset 集成，允许用户充分利用 Manticore Search 的强大功能进行数据分析和可视化。

**注意：此集成仅适用于 Apache Superset 3.x 分支，尚不支持版本 4 或更高版本。**

## 前提条件

在将 Manticore Search 与 Apache Superset 集成之前，请确保：

1. Manticore Search（版本 6.2.0 或更高）已正确安装并配置在您的服务器上。请参阅[官方 Manticore Search 安装指南](https://manual.manticoresearch.com/Installation/Installation)获取帮助。
2. Apache Superset 已在您的系统上设置。请按照[官方 Apache Superset 安装指南](https://superset.apache.org/docs/quickstart)进行安装。

## 将 Manticore Search 连接到 Apache Superset

要将 Manticore Search 连接到 Apache Superset：

1. 登录到您的 Apache Superset 仪表板。
2. 点击顶部导航栏中的“+”，选择“数据库连接”。
3. 点击“+ DATABASE”添加新数据库。
4. 选择“MySQL”作为数据库类型。
5. 在配置页面中，提供以下详细信息：
   - 数据库名称：数据库的名称（例如，“Manticore”）
   - 主机：您的 Manticore Search 服务器主机
   - 端口：Manticore Search 运行的端口
   - 用户名：您的 Manticore Search 用户名
   - 显示名称：连接的显示名称
6. 点击“Connect”验证连接并保存。

## 创建图表和仪表板

将 Manticore Search 连接到 Apache Superset 后，您可以使用 Manticore 数据创建图表和仪表板：

1. 在 Apache Superset 仪表板中，点击顶部导航栏的“Dashboards”，选择“New dashboard”。
2. 打开仪表板，点击“+”图标添加新图表。
3. 选择连接到 Manticore Search 的数据集。
4. 选择您想创建的图表类型（例如，条形图、折线图、饼图）。
5. 使用 Apache Superset 的查询构建器或编写 SQL 查询，从您的 Manticore Search 数据库中获取数据。
6. 根据需要自定义图表的外观、标签和其他设置。
7. 点击“UPDATE CHART”更新图表。
8. 为图表添加名称，点击“Save”将其永久保存到仪表板中。

## 支持的功能

通过 Apache Superset 使用 Manticore Search 时，您可以：

- 查看和查询 Manticore Search 表中的数据
- 基于 Manticore 数据创建各种类型的可视化
- 使用这些可视化构建和自定义仪表板

## 数据类型处理

在通过 Apache Superset 使用时，Manticore Search 的数据类型会映射到 MySQL 数据类型。有关支持的数据类型及其映射的更多详细信息，请参阅 Manticore Search 文档。

## 限制

- 通过 Apache Superset 使用 Manticore Search 时，某些高级 MySQL 功能可能不可用。
- 与直接使用 MySQL 数据库相比，某些数据操作可能受到限制。

通过遵循这些步骤和指南，您可以有效地将 Manticore Search 与 Apache Superset 集成，实现强大的数据探索和可视化功能。

## 参考资料
有关 Apache Superset 和 Manticore Search 的更多信息，请参阅以下资源：
- [Apache Superset 集成](https://manticoresearch.com/blog/manticoresearch-apache-superset-integration/)

