# Main+delta schema

<!-- example maindelta -->
在许多情况下，整个数据集过大，无法频繁从头重建，而新记录的数量保持相对较小。例如，一个论坛可能有1,000,000条归档帖子，但每天只收到1,000条新帖子。

在这种情况下，可以通过使用“主+增量”方案实现“实时”(几乎实时)表更新。

该概念涉及设置两个源和两个表，一个“主”表用于数据（很少变化，甚至不变化），一个“增量”表用于新文档。在这个例子中，1,000,000条归档帖子将存储在主表中，而每天的1,000条新帖子将放在增量表中。增量表可以频繁重建，使文档在几秒钟或几分钟内可供搜索。确定哪些文档属于哪个表并重建主表可以完全自动化。一种方法是创建一个计数器表来跟踪用于拆分文档的ID，并在每次重建主表时更新它。

使用时间戳列作为拆分变量比使用ID更有效，因为时间戳不仅可以跟踪新文档，还可以跟踪已修改的文档。

对于可能包含已修改或已删除文档的数据集，增量表应提供受影响文档的列表，确保它们在搜索查询中被抑制和排除。这是通过称为“杀死列表”的功能来完成的。要被杀死的文档ID可以在由 [sql_query_killlist](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md#Table-kill-list) 定义的辅助查询中指定。增量表必须指示杀死列表将应用于的目标表，使用 [killlist_target](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md#killlist_target) 指令。杀死列表对目标表的影响是永久性的，这意味着即使在没有增量表的情况下进行搜索，被抑制的文档也不会出现在搜索结果中。

请注意我们在增量源中覆盖 `sql_query_pre`。我们必须明确包含此覆盖。如果不这样做，`REPLACE` 查询将在增量源的构建过程中执行，从而有效地使其无效。

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
