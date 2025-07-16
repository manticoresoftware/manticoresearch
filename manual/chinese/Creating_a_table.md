# 创建表格
<!-- proofread -->

Manticore Search 提供了多种创建表格的方法，每种方法都针对不同的用例和需求而设计。本节介绍主要方法及其具体配置。

## 表格类型

Manticore 支持不同类型的表格：

- **实时表格** - 用于实时更新和即时搜索
- **普通表格** - 用于不需要频繁更新的静态数据
- **渗透表格** - 用于存储和匹配搜索查询
- **模板表格** - 用于创建表格模板
- **分布式表格** - 用于跨多个节点扩展

## 关键组件

创建表格时，您需要考虑：

1. **数据类型** - 了解可用的数据类型及其存储选项
2. **自然语言处理和分词** - 配置文本数据的处理和索引方式
3. **表格结构** - 为您的特定用例定义架构和设置

## 入门指南

要创建表格，您需要：

1. 根据您的用例选择合适的表格类型
2. 定义包含所需字段和属性的表格架构
3. 根据您的特定需求配置必要的设置

有关详细信息，请参阅有关每个方面的信息，请参阅以下章节：

- [数据类型](Creating_a_table/Data_types.md) - 了解支持的数据类型和存储选项
- [创建本地表](Creating_a_table/Local_tables.md) - 创建不同类型本地表的详细指南
- [自然语言处理 (NLP) 和分词 (Tokenization)](Creating_a_table/NLP_and_tokenization/Data_tokenization.md) - 配置文本处理和索引
- [创建分布式表](Creating_a_table/Creating_a_distributed_table/Creating_a_distributed_table.md) - 设置分布式表以进行扩展

以下每个章节都提供了有关在 Manticore Search 中创建表的具体方面的深入信息。