# 创建表格
<!-- proofread -->

Manticore Search 提供了多种创建表格的方法，每种方法针对不同的用例和需求设计。本节涵盖主要的方法及其具体配置。

## 表格类型

Manticore 支持不同类型的表格：

- **实时表** - 用于实时更新和即时搜索
- **普通表** - 用于不需要频繁更新的静态数据
- **Percolate 表** - 用于存储和匹配搜索查询
- **模板表** - 用于创建表格模板
- **分布式表** - 用于跨多个节点扩展

## 关键组件

创建表格时，您需要考虑：

1. **数据类型** - 了解可用的数据类型及其存储选项
2. **NLP 和分词** - 配置文本数据的处理和索引方式
3. **表结构** - 定义特定用例的模式和设置

## 入门

要创建表格，您需要：

1. 为您的用例选择合适的表格类型
2. 定义包含所需字段和属性的表格模式
3. 配置满足特定需求的必要设置

有关每个方面的详细信息，请参阅以下章节：

- [数据类型](Creating_a_table/Data_types.md) - 了解支持的数据类型和存储选项
- [创建本地表](Creating_a_table/Local_tables.md) - 创建不同类型本地表的详细指南
- [NLP 和分词](Creating_a_table/NLP_and_tokenization/Data_tokenization.md) - 配置文本处理和索引
- [创建分布式表](Creating_a_table/Creating_a_distributed_table/Creating_a_distributed_table.md) - 设置分布式表以实现扩展

这些章节分别提供了关于 Manticore Search 中表格创建各个具体方面的深入信息。
