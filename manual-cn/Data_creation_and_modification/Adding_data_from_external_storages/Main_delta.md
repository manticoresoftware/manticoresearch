# 主+增量（Main+Delta）方案

<!-- example maindelta -->

在许多情况下，数据集的总量太大，无法频繁地从头开始重建，而新增记录的数量却相对较少。例如，一个论坛可能拥有 1,000,000 条已归档的帖子，但每天仅收到 1,000 条新帖子。

在这种情况下，可以使用“主+增量”方案来实现“实时”（几乎是实时）的表更新。

该概念包括设置两个源和两张表，一个“主”表用于存储很少（如果有的话）变化的数据，另一张“增量”表则用于存储新文档。在示例中，1,000,000 条已归档的帖子将存储在主表中，而每天新增的 1,000 条帖子将放置在增量表中。增量表可以频繁地重建，使文档能够在几秒或几分钟内可供搜索。确定哪些文档属于哪个表，以及何时重建主表，可以完全自动化。一种方法是创建一个计数器表，用于跟踪用于拆分文档的 ID，并在重建主表时进行更新。

使用时间戳列作为拆分变量比使用 ID 更为有效，因为时间戳不仅可以跟踪新文档，还可以跟踪已修改的文档。

对于可能包含已修改或已删除文档的数据集，增量表应提供受影响文档的列表，确保这些文档被抑制并从搜索查询中排除。这是通过称为“删除列表”（Kill Lists）的功能来实现的。要删除的文档 ID 可以在由 [sql_query_killlist](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md#Table-kill-list) 定义的辅助查询中指定。增量表必须使用 [killlist_target](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md#killlist_target) 指令来指示删除列表将应用的目标表。删除列表对目标表的影响是永久性的，这意味着即使在不使用增量表进行搜索的情况下，被抑制的文档也不会出现在搜索结果中。

请注意，我们在增量源中重写了 `sql_query_pre`。我们必须显式包含此重写。如果不这样做，`REPLACE` 查询将在增量源的构建过程中执行，从而使其无效。

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
