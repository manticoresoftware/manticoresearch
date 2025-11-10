# Plain table

**Plain table** 是用于非 [percolate](../../Creating_a_table/Local_tables/Percolate_table.md) 搜索的基本元素。它只能在配置文件中使用 [Plain mode](../../Creating_a_table/Local_tables.md#Defining-table-schema-in-config-%28Plain mode%29) 定义，不支持在 [RT mode](../../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29) 中使用。它通常与 [source](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#source) 结合使用，以处理 [来自外部存储](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md) 的数据，之后可以 [附加](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Attaching_one_table_to_another.md) 到 **实时表**。

#### 创建 plain table

要创建 plain table，您需要在配置文件中定义它。`CREATE TABLE` 命令不支持创建。

<!-- example plain -->

下面是一个 plain table 配置示例，以及用于从 MySQL 数据库获取数据的 source：

<!-- intro -->
#### 如何创建 plain table

<!-- request Plain table example -->

```ini
source source {
  type             = mysql
  sql_host         = localhost
  sql_user         = myuser
  sql_pass         = mypass
  sql_db           = mydb
  sql_query        = SELECT id, title, description, category_id  from mytable
  sql_attr_uint    = category_id
  sql_field_string = title
 }

table tbl {
  type   = plain
  source = source
  path   = /path/to/table
 }
```
<!-- end -->

### 👍 使用 plain table 可以做的事情：
  * 使用 [source](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#source) 和 [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) 从外部存储构建表
  * 对 [整数、浮点数、字符串和 MVA 属性](../../Creating_a_table/Data_types.md) 进行原地更新
  * [更新](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md#killlist_target) 它的 `killlist_target`

### ⛔ 使用 plain table 不能做的事情：
  * 在表构建完成后插入额外数据
  * 从表中删除数据
  * 在线创建、删除或修改表结构
  * 使用 [UUID](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-ID) 自动生成 ID（来自外部存储的数据必须包含唯一标识符）

数值属性，包括 [MVA](../../Creating_a_table/Data_types.md#Multi-value-integer-%28MVA%29)，是 plain table 中唯一可以更新的元素。表中的其他数据都是不可变的。如果需要更新或新增记录，必须重建表。在重建过程中，现有表仍可用于服务请求，当新版本准备好时，会执行称为 [rotation](../../Data_creation_and_modification/Adding_data_from_external_storages/Rotating_a_table.md) 的过程，将其上线并丢弃旧版本。

#### plain table 构建性能
plain table 的索引速度取决于多个因素，包括：
* 数据源检索速度
* [分词设置](../../Creating_a_table/NLP_and_tokenization/Data_tokenization.md)
* 硬件规格（如 CPU、内存和磁盘性能）

#### plain table 构建场景
##### 需要时完全重建
对于小型数据集，最简单的方案是拥有一个单一的 plain table，根据需要完全重建。当满足以下条件时，这种方法是可接受的：
* 表中的数据不如数据源中的数据新鲜
* 随着数据集增长，构建表所需时间增加

##### 主表+增量表方案
对于较大数据集，可以使用 plain table 替代 [实时表](../../Creating_a_table/Local_tables/Real-time_table.md)。主表+增量表方案包括：
* 创建一个较小的表用于增量索引
* 使用 [分布式表](../../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md) 组合两个表

这种方法允许较大表不频繁重建，而更频繁地处理来自数据源的更新。较小的表可以更频繁地重建（例如每分钟甚至每几秒）。

然而，随着时间推移，较小表的索引时间会变得过长，需要重建较大表并清空较小表。

主表+增量表方案在 [此交互式课程](https://play.manticoresearch.com/maindelta/) 中有详细说明。

**kill list** 机制和 [killlist_target](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#killlist_target) 指令用于确保当前表中的文档优先于另一个表中的文档。

更多信息请参见 [这里](../../Data_creation_and_modification/Adding_data_from_external_storages/Main_delta.md)。

#### plain table 文件结构
下表列出了 plain table 中使用的各种文件扩展名及其描述：

| Extension | Description |
| - | - |
|`.spa` | 以 [行存模式](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) 存储文档属性 |
|`.spb` | 以 [行存模式](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) 存储 blob 属性：字符串、MVA、json |
|`.spc` | 以 [列存模式](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) 存储文档属性 |
|`.spd` | 存储每个词 ID 的匹配文档 ID 列表 |
|`.sph` | 存储表头信息 |
|`.sphi` | 存储属性值的直方图 |
|`.spi` | 存储词列表（词 ID 和指向 `.spd` 文件的指针） |
|`.spidx` | 存储二级索引数据 |
|`.spjidx` | 存储为 JSON 属性生成的二级索引数据 |
|`.spk` | 存储 kill-list |
|`.spl` | 锁文件 |
|`.spm` | 存储已删除文档的位图 |
|`.spp` | 存储每个词 ID 的命中（又称 posting，词出现）列表 |
|`.spt` | 存储加速按文档 ID 查找的附加数据结构 |
|`.spe` | 存储跳跃列表以加速文档列表过滤 |
|`.spds` | 存储文档文本 |
|`.tmp*` | 在 index_settings_and_status 期间的临时文件 |
|`.new.sp*` | 旋转前的纯表新版本 |
|`.old.sp*` | 旋转后的纯表旧版本 |

<!-- proofread -->

