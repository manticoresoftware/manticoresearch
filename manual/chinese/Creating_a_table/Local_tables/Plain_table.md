# 普通表

**普通表** 是用于非 [percolate](../../Creating_a_table/Local_tables/Percolate_table.md) 搜索的基本元素。它只能在配置文件中使用 [普通模式](../../Creating_a_table/Local_tables.md#Defining-table-schema-in-config-%28Plain mode%29) 定义，不支持在 [实时模式](../../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29) 中使用。它通常与 [源](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#source) 配合使用，从 [外部存储](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md) 处理数据，之后可以 [附加](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Attaching_one_table_to_another.md) 到 **实时表**。

#### 创建普通表

要创建普通表，您需要在配置文件中定义它。 `CREATE TABLE` 命令不支持普通表。

<!-- example plain -->

下面是一个普通表配置示例，以及用于从 MySQL 数据库获取数据的源示例：

<!-- intro -->
#### 如何创建普通表

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

### 👍 使用普通表可以做什么：
  * 使用 [源](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#source) 和 [索引器](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) 从外部存储构建表
  * 对 [整数、浮点数、字符串和 MVA 属性](../../Creating_a_table/Data_types.md) 进行就地更新
  * [更新](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md#killlist_target) 它的 `killlist_target`

### ⛔ 使用普通表不能做什么：
  * 构建完成后，不能向表中插入额外数据
  * 不能从表中删除数据
  * 不能在线创建、删除或修改表的模式
  * 不能使用 [UUID](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-ID) 进行自动 ID 生成（来自外部存储的数据必须包含唯一标识符）

数值属性，包括 [MVA](../../Creating_a_table/Data_types.md#Multi-value-integer-%28MVA%29)，是普通表中唯一可以更新的元素。表中的其他数据是不可变的。如果需要更新或添加新记录，必须重建表。在重建过程中，现有表仍然可用以响应请求，当新版本准备好后，会执行称为 [轮换](../../Data_creation_and_modification/Adding_data_from_external_storages/Rotating_a_table.md) 的过程，将新版本上线并废弃旧版本。

#### 普通表构建性能
普通表索引的速度取决于多个因素，包括：
* 数据源检索速度
* [分词设置](../../Creating_a_table/NLP_and_tokenization/Data_tokenization.md)
* 硬件规格（如 CPU、内存和磁盘性能）

#### 普通表构建方案
##### 按需完全重建
对于小型数据集，最简单的方法是拥有一个完全按需重建的普通表。当满足以下条件时，这种方法是可接受的：
* 表中数据不如源数据新鲜
* 随着数据集增长，构建表所需时间增加

##### 主表+增量表方案
对于较大数据集，可以使用普通表代替 [实时表](../../Creating_a_table/Local_tables/Real-time_table.md)。主表+增量表方案包含：
* 创建一个较小的表用于增量索引
* 使用 [分布式表](../../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md) 结合两个表

这种方法允许较大表不频繁重建，而增量表更频繁处理来自源的更新。较小表可以更频繁重建（例如每分钟或甚至每几秒）。

但随着时间推移，较小表的索引时间会变得过长，需要重建较大表并清空较小表。

主表+增量表架构在 [此交互式课程](https://play.manticoresearch.com/maindelta/) 中有详细讲解。

**kill list** 机制和 [killlist_target](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#killlist_target) 指令确保当前表中的文档优先于另一个表中的。

有关此主题的更多信息，请见 [这里](../../Data_creation_and_modification/Adding_data_from_external_storages/Main_delta.md)。

#### 普通表文件结构
下表列出了普通表中使用的各种文件扩展名及其描述：

| 扩展名 | 描述 |
| - | - |
|`.spa` | 以[行模式](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)存储文档属性 |
|`.spb` | 以[行模式](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)存储二进制大对象属性：字符串、MVA、json |
|`.spc` | 以[列模式](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)存储文档属性 |
|`.spd` | 存储每个词ID对应的匹配文档ID列表 |
|`.sph` | 存储表头信息 |
|`.sphi` | 存储属性值的直方图 |
|`.spi` | 存储词列表（词ID及指向`.spd`文件的指针） |
|`.spidx` | 存储二级索引数据 |
|`.spjidx` | 存储为JSON属性生成的二级索引数据 |
|`.spk` | 存储屏蔽列表 |
|`.spl` | 锁文件 |
|`.spm` | 存储被屏蔽文档的位图 |
|`.spp` | 存储每个词ID的命中列表（又称posting，词出现列表） |
|`.spt` | 存储额外的数据结构以加速通过文档ID的查找 |
|`.spe` | 存储跳过列表以加速文档列表过滤 |
|`.spds` | 存储文档文本 |
|`.tmp*` | 索引设置和状态期间的临时文件 |
|`.new.sp*` | 旋转前纯表的新版本 |
|`.old.sp*` | 旋转后纯表的旧版本 |

<!-- proofread -->

