# 主+增量 模式

<!-- example maindelta -->
在许多情况下，整个数据集太大，无法频繁地从头重建，而新记录的数量相对较少。例如，一个论坛可能有1,000,000条存档帖子，但每天只接收1,000条新帖子。

在这种情况下，可以使用“主+增量”模式来实现“实时”（近实时）表更新。

该概念涉及设置两个数据源和两张表，一张“主”表用于存储很少更改（如果有的话）的数据，另一张“增量”表用于存放新文档。在示例中，1,000,000条存档帖子将存储在主表中，而每天新增的1,000条帖子则放入增量表。增量表可以频繁重建，使文档在几秒或几分钟内可供搜索。确定哪些文档属于哪张表以及重建主表可以完全自动化。一种方法是创建一个计数表，用于跟踪用来划分文档的ID，并在重建主表时更新该表。

使用时间戳列作为划分变量比使用ID更有效，因为时间戳不仅可以追踪新文档，还可以追踪被修改的文档。

对于可能包含被修改或删除文档的数据集，增量表应提供受影响文档的列表，确保这些文档被屏蔽并从搜索查询中排除。这通过一个称为“Kill List”（杀死列表）的功能实现。要被杀死的文档ID可以在辅助查询中指定，该查询由[sql_query_killlist](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md#Table-kill-list)定义。增量表必须使用[killlist_target](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md#killlist_target)指令指明将应用杀死列表的目标表。杀死列表对目标表的影响是永久的，这意味着即使在没有增量表的搜索中，被屏蔽的文档也不会出现在搜索结果中。

注意我们在增量数据源中重写了`sql_query_pre`。我们必须显式包含此重写。如果不这么做，`REPLACE`查询也会在增量数据源构建期间执行，实际上使其无效。

<!-- request Example -->
```ini
# in MySQL
CREATE TABLE deltabreaker (
  index_name VARCHAR(50) NOT NULL,
  created_at TIMESTAMP NOT NULL  DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (index_name)
);

# in manticore.conf
source main {
  ...
  sql_query_pre = REPLACE INTO deltabreaker SET index_name = 'main', created_at = NOW()
  sql_query =  SELECT id, title, UNIX_TIMESTAMP(updated_at) AS updated FROM documents WHERE deleted=0 AND  updated_at  >=FROM_UNIXTIME($start) AND updated_at  <=FROM_UNIXTIME($end)
  sql_query_range  = SELECT ( SELECT UNIX_TIMESTAMP(MIN(updated_at)) FROM documents) min, ( SELECT UNIX_TIMESTAMP(created_at)-1 FROM deltabreaker WHERE index_name='main') max
  sql_query_post_index = REPLACE INTO deltabreaker set index_name = 'delta', created_at = (SELECT created_at FROM deltabreaker t WHERE index_name='main')
  ...
  sql_attr_timestamp = updated
}

source delta : main {
  sql_query_pre =
  sql_query_range = SELECT ( SELECT UNIX_TIMESTAMP(created_at) FROM deltabreaker WHERE index_name='delta') min, UNIX_TIMESTAMP() max
  sql_query_killlist = SELECT id FROM documents WHERE updated_at >=  (SELECT created_at FROM deltabreaker WHERE index_name='delta')
}

table main {
  path = /var/lib/manticore/main
  source = main
}

table delta {
  path = /var/lib/manticore/delta
  source = delta
  killlist_target = main:kl
}
```

<!-- end -->


<!-- proofread -->

