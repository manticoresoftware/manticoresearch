# 分面搜索

					分面搜索对现代搜索应用程序来说与 [自动完成](../Searching/Autocomplete.md)、[拼写校正](../Searching/Spell_correction.md) 和搜索关键词 [高亮](../Searching/Highlighting.md) 一样重要，尤其是在电子商务产品中。

					![分面搜索](faceted.png)

					分面搜索在处理大量数据和各种互联属性时非常有用，例如尺寸、颜色、制造商或其他因素。在查询大量数据时，搜索结果往往包含许多与用户期望不符的条目。分面搜索使最终用户能够明确定义他们希望搜索结果满足的标准。

					在 Manticore Search 中，有一种优化会保持原始查询的结果集，并在每个分面计算中重用该结果集。由于聚合是应用于已计算的文档子集，因此速度很快，并且总执行时间通常仅比初始查询稍长。可以将分面添加到任何查询中，并且分面可以是任何属性或表达式。分面结果包括分面值和分面计数。可以通过在查询的最后声明它们来使用 SQL `SELECT` 语句访问分面。

					## 聚合

					<!-- example Aggregations -->
					### SQL
					分面值可以源自属性、JSON 属性中的 JSON 属性，或表达式。分面值也可以被别名，但 **别名必须在所有结果集中是唯一的**（主查询结果集和其他分面结果集）。分面值来源于聚合的属性/表达式，但也可以来自另一个属性/表达式。

					CODE_BLOCK_0
					多个分面声明必须用空格分隔。
					### HTTP JSON

					分面可以在 `aggs` 节点中定义：

					CODE_BLOCK_1

					其中：

					* `group name` 是分配给聚合的别名
					* `field` 值必须包含被分面的属性或表达式的名称
					* 可选的 `size` 指定要包含在结果中的最大桶数。当未指定时，它会继承主查询的限制。更多详细信息请查看 [分面结果的大小](../Searching/Faceted_search.md#Size-of-facet-result) 部分。
					* 可选的 `sort` 使用与 [主查询中的 "sort" 参数](../Searching/Sorting_and_ranking.md#Sorting-via-JSON) 相同的语法指定属性和/或附加属性的数组。
					结果集将包含一个 `aggregations` 节点，其中返回的分面为 `key` 是聚合值，`doc_count` 是聚合计数。
					CODE_BLOCK_2
					<!-- intro -->
					##### SQL:
					<!-- request SQL -->
					CODE_BLOCK_3
					<!-- response SQL -->
					CODE_BLOCK_4
					<!-- intro -->
					##### JSON:

					<!-- request JSON -->
					CODE_BLOCK_5
					<!-- response JSON -->
					CODE_BLOCK_6
					<!-- intro -->

					##### PHP:

					<!-- request PHP -->
					CODE_BLOCK_7
					<!-- response PHP -->
					CODE_BLOCK_8
					<!-- intro -->
					##### Python:
					<!-- request Python -->
					CODE_BLOCK_9
					<!-- response Python -->
					CODE_BLOCK_10
					<!-- intro -->
					##### Javascript:
					<!-- request Javascript -->
					CODE_BLOCK_11
					<!-- response Javascript -->
					CODE_BLOCK_12
					<!-- intro -->
					##### Java:
					<!-- request Java -->
					CODE_BLOCK_13
					<!-- response Java -->
					CODE_BLOCK_14
					<!-- intro -->
					##### C#:
					<!-- request C# -->
					CODE_BLOCK_15
					<!-- response C# -->
					CODE_BLOCK_16

					<!-- request TypeScript -->
					CODE_BLOCK_17

					<!-- response TypeScript -->

					CODE_BLOCK_18
					<!-- request Go -->
					CODE_BLOCK_19

					<!-- response Go -->

					CODE_BLOCK_20
					<!-- end -->
					<!-- example Another_attribute -->
					### 通过聚合另一个属性进行分面
					数据可以通过聚合另一个属性或表达式来分面。例如，如果文档同时包含品牌 ID 和名称，我们可以在分面中返回品牌名称，但聚合品牌 ID。这可以通过使用 `FACET {expr1} BY {expr2}` 来实现。
					<!-- intro -->
					##### SQL:
					<!-- request SQL -->
					CODE_BLOCK_21
					<!-- response SQL -->
					CODE_BLOCK_22
					<!-- end -->
					<!-- example Distinct -->
					### 无重复项的分面
					如果您需要从 FACET 返回的桶中移除重复项，可以使用 `DISTINCT field_name`，其中 `field_name` 是您想要去重的字段。如果您对分布式表进行了 FACET 查询并且不确定表中是否有唯一 ID，这也可以是 `id`（这是默认值）（表应该是本地的并且拥有相同的模式）。
					如果您的查询中有多个 FACET 声明，`field_name` 应在所有声明中保持一致。
					`DISTINCT` 在 `count(*)` 列之前返回附加列 `count(distinct ...)`，允许您在不需要进行另一个查询的情况下获得两者结果。
					<!-- intro -->
					##### SQL:
					<!-- request SQL -->
					CODE_BLOCK_23
					<!-- response SQL -->
					CODE_BLOCK_24
					<!-- intro -->
					##### JSON:
					<!-- request JSON -->
					CODE_BLOCK_25
					<!-- response JSON -->
					CODE_BLOCK_26
					<!-- end -->
					<!-- example Expressions -->
					### 对表达式的分面
					分面可以对表达式进行聚合。一个经典的例子是通过特定范围划分价格：
					<!-- request SQL -->

					CODE_BLOCK_27

					<!-- response SQL -->
					CODE_BLOCK_28
					<!-- request JSON -->
					CODE_BLOCK_29
					<!-- response JSON -->
					CODE_BLOCK_30
					<!-- request PHP -->
					CODE_BLOCK_31
					<!-- response PHP -->
					CODE_BLOCK_32
					<!-- request Python -->
					CODE_BLOCK_33
					<!-- response Python -->
					CODE_BLOCK_34
					<!-- request Javascript -->
					CODE_BLOCK_35
					<!-- response Javascript -->
					CODE_BLOCK_36
					<!-- request Java -->
					CODE_BLOCK_37
					<!-- response Java -->
					CODE_BLOCK_38
					<!-- request C# -->
					CODE_BLOCK_39
					<!-- response C# -->
					CODE_BLOCK_40

					<!-- request TypeScript -->

					CODE_BLOCK_41
					<!-- response TypeScript -->
					CODE_BLOCK_42
					<!-- request Go -->
					CODE_BLOCK_43
					<!-- response Go -->
					CODE_BLOCK_44
					<!-- end -->
					<!-- example Multi-level -->
					### 对多级分组的分面
					分面可以对多级分组进行聚合，结果集与查询执行多级分组时的结果集相同：
					<!-- request SQL -->
					CODE_BLOCK_45
					<!-- response SQL -->
Here's the translation of the document, maintaining the original structure exactly:
					<!-- end -->
					<!-- example histogram -->
					### 直方图值分面
					分面可以通过构建固定大小的桶来聚合直方图值。
					关键函数是：
					CODE_BLOCK_47
					直方图参数 `interval` 必须为正数，直方图参数 `offset` 必须为正数且小于 `interval`。默认情况下，桶作为数组返回。直方图参数 `keyed` 使响应成为带有桶键的字典。
					<!-- request SQL -->
					CODE_BLOCK_48
					<!-- response SQL -->
					CODE_BLOCK_49
					<!-- request JSON -->
					CODE_BLOCK_50
					<!-- response JSON -->
					CODE_BLOCK_51
					<!-- request JSON 2 -->
					CODE_BLOCK_52
					<!-- response JSON 2 -->
					CODE_BLOCK_53
					<!-- end -->
					<!-- example histogram_date -->
					### 日期值直方图分面
					分面可以聚合日期值直方图，这与普通直方图类似。不同之处在于使用日期或时间表达式指定间隔。这种表达式需要特殊支持，因为间隔并非总是固定长度。使用以下关键函数将值舍入到最近的桶：
					CODE_BLOCK_54
					直方图参数 `calendar_interval` 理解月份有不同的天数。
					与 `calendar_interval` 不同，`fixed_interval` 参数使用固定数量的单位，无论落在日历的哪个位置都不会偏离。但是 `fixed_interval` 无法处理月等单位，因为月不是固定数量。尝试为 `fixed_interval` 指定周或月等单位将导致错误。
					接受的间隔在 [date_histogram](../Functions/Date_and_time_functions.md#DATE_HISTOGRAM%28%29) 表达式中描述。默认情况下，桶作为数组返回。直方图参数 `keyed` 使响应成为带有桶键的字典。
					<!-- request SQL -->
					CODE_BLOCK_55
					<!-- response SQL -->
					CODE_BLOCK_56
					<!-- request JSON -->
					CODE_BLOCK_57
					<!-- response JSON -->
					CODE_BLOCK_58
					<!-- end -->
					<!-- example facet range -->
					### 范围集分面
					分面可以聚合范围集。值根据桶范围进行检查，每个桶包含 `from` 值并排除范围的 `to` 值。
					将 `keyed` 属性设置为 `true` 会使响应成为带有桶键的字典，而不是数组。
					<!-- request SQL -->
					CODE_BLOCK_59
					<!-- response SQL -->
					CODE_BLOCK_60
					<!-- request JSON -->
					CODE_BLOCK_61
					<!-- response JSON -->
					CODE_BLOCK_62
					<!-- request JSON 2 -->
					CODE_BLOCK_63
					<!-- response JSON 2 -->
					CODE_BLOCK_64
					<!-- end -->
					<!-- example facet range_date -->
					### 日期范围集分面
					分面可以聚合日期范围集，这与普通范围类似。不同之处在于 `from` 和 `to` 值可以用 [日期数学](../Functions/Date_and_time_functions.md#Date-math) 表达式表示。此聚合包含每个范围的 `from` 值并排除 `to` 值。将 `keyed` 属性设置为 `true` 会使响应成为带有桶键的字典，而不是数组。
					<!-- request SQL -->
					CODE_BLOCK_65
					<!-- response SQL -->
					CODE_BLOCK_66
					<!-- request JSON -->
					CODE_BLOCK_67
					<!-- response JSON -->
					CODE_BLOCK_68
					<!-- end -->
					<!-- example Ordering -->

					### 分面结果排序
					分面支持 `ORDER BY` 子句，就像标准查询一样。每个分面可以有自己的排序，分面排序不影响主结果集的排序，后者由主查询的 `ORDER BY` 决定。排序可以基于属性名称、计数（使用 `COUNT(*)`, `COUNT(DISTINCT attribute_name)`）或特殊的 `FACET()` 函数，该函数提供聚合的数据值。默认情况下，带有 `ORDER BY COUNT(*)` 的查询将以降序排序。

					<!-- intro -->
					##### SQL:
					<!-- request SQL -->
					CODE_BLOCK_69
					<!-- response SQL -->
					CODE_BLOCK_70
					<!-- intro -->
					##### JSON:
					<!-- request JSON -->
					CODE_BLOCK_71
					<!-- response JSON -->
					CODE_BLOCK_72
					<!-- end -->
					<!-- example Size -->
					### 分面结果大小
					默认情况下，每个分面结果集限制为 20 个值。可以使用 `LIMIT` 子句单独控制每个分面的值数量，方法是提供要返回的值数量，格式为 `LIMIT count`，或者使用偏移量 `LIMIT offset, count`。
					返回的最大分面值受查询的 `max_matches` 设置限制。如果要实现动态 `max_matches`（为了更好的性能，将 `max_matches` 限制为 offset + 每页），必须考虑到太低的 `max_matches` 值可能会影响分面值的数量。在这种情况下，应使用足以覆盖分面值数量的最小 `max_matches` 值。
					<!-- intro -->
					##### SQL:
					<!-- request SQL -->
					CODE_BLOCK_73
					<!-- response SQL -->
					CODE_BLOCK_74
					<!-- request JSON -->
					CODE_BLOCK_75
					<!-- response JSON -->
					CODE_BLOCK_76
					<!-- request PHP -->
					CODE_BLOCK_77
					<!-- response PHP -->
					CODE_BLOCK_78
					<!-- request Python -->
					CODE_BLOCK_79
					<!-- response Python -->
					CODE_BLOCK_80
					<!-- request Javascript -->
					CODE_BLOCK_81
					<!-- response Javascript -->
					CODE_BLOCK_82
					<!-- request Java -->
					CODE_BLOCK_83
					<!-- response Java -->
					CODE_BLOCK_84
					<!-- request C# -->
					CODE_BLOCK_85
					<!-- response C# -->
					CODE_BLOCK_86
					<!-- request TypeScript -->
					CODE_BLOCK_87
					<!-- response TypeScript -->
					CODE_BLOCK_88
					<!-- request Go -->
					CODE_BLOCK_89
					<!-- response Go -->
					CODE_BLOCK_90
					<!-- end -->
					### 返回的结果集
					使用 SQL 时，带有分面的搜索返回多个结果集。使用的 MySQL 客户端/库/连接器**必须**支持多结果集才能访问分面结果集。
					<!-- example Performance -->
					### 性能
Internally, the `FACET` 是执行多查询的简写，其中第一个查询包含主要搜索查询，批处理中其余的查询各自具有聚类。与多查询的情况一样，通用查询优化可以在分面搜索中启动，这意味着搜索查询只执行一次，分面在搜索查询结果上操作，每个分面仅为总查询时间添加一小部分时间。
					要检查分面搜索是否以优化模式运行，可以查看 [query log](../Logging/Query_logging.md)，所有记录的查询将包含一个 `xN` 字符串，其中 `N` 是在优化组中运行的查询数量。或者，您可以检查 [SHOW META](../Node_info_and_management/SHOW_META.md) 语句的输出，该输出将显示一个 `multiplier` 指标：
					<!-- request SQL -->
					CODE_BLOCK_91
					<!-- response SQL -->
					CODE_BLOCK_92
					<!-- end -->
					<!-- proofread -->
















































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































