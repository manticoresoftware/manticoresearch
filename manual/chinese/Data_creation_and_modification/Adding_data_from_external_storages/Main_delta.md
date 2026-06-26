# 主增量模式

<!-- example maindelta -->
在许多情况下，整个数据集太大，无法频繁从头重建，而新记录的数量相对较小。例如，一个论坛可能有1,000,000条存档帖子，但每天只收到1,000条新帖子。

在这种情况下，可以使用“主增量”方案来实现“实时”（近乎实时）的表更新。

该方案涉及设置两个数据源和两个表，一个“主”表用于存储很少变化（如果有的话）的数据，一个“增量”表用于存储新文档。在示例中，1,000,000条存档帖子将存储在主表中，而每天的1,000条新帖子将放在增量表中。增量表可以频繁重建，使文档在几秒或几分钟内可供搜索。确定哪些文档属于哪个表以及重建主表可以完全自动化。一种方法是创建一个计数器表来跟踪用于拆分文档的ID，并在重建主表时更新它。

使用时间戳列作为拆分变量比使用ID更有效，因为时间戳不仅可以跟踪新文档，还可以跟踪修改过的文档。

对于可能包含修改或删除文档的数据集，增量表应提供受影响文档的列表，确保它们被抑制并从搜索查询中排除。这通过使用名为Kill Lists的功能来实现。要被杀死的文档ID可以在由[sql_query_killlist](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md#Table-kill-list)定义的辅助查询中指定。增量表必须使用[killlist_target](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md#killlist_target)指令来指示将应用Kill Lists的目标表。Kill Lists对目标表的影响是永久性的，这意味着即使在没有增量表的情况下执行搜索，被抑制的文档也不会出现在搜索结果中。

注意我们如何在增量源中覆盖`sql_query_pre`。我们必须显式地包含这个覆盖。如果不这样做，`REPLACE`查询将在增量源的构建过程中执行，从而使它变得毫无用处。

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

