# 普通表

**普通表** 是一个基本元素，用于非 [渗透搜索](../../Creating_a_table/Local_tables/Percolate_table.md)。它只能在配置文件中使用 [普通模式](../../Creating_a_table/Local_tables.md#Defining-table-schema-in-config-%28Plain mode%29) 定义，并且不支持 [实时模式](../../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29)。它通常与 [源](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#source) 结合使用，以处理 [来自外部存储的数据](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md)，并可以在以后 [附加](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Attaching_one_table_to_another.md) 到 **实时表**。

#### 创建普通表

要创建普通表，您需要在配置文件中定义它。`CREATE TABLE` 命令不支持此操作。

<!-- example plain -->

下面是一个普通表配置的示例，以及从 MySQL 数据库获取数据的源：

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

### 👍 您可以使用普通表做的事情：
  * 从外部存储构建它，使用 [源](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#source) 和 [索引器](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool)
  * 对 [整数、浮点数、字符串和 MVA 属性](../../Creating_a_table/Data_types.md) 进行就地更新
  * [更新](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md#killlist_target) 的 `killlist_target`

### ⛔ 您不能使用普通表做的事情：
  * 在表构建完成后插入额外数据
  * 从表中删除数据
  * 在线创建、删除或更改表模式
  * 使用 [UUID](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-ID) 进行自动 ID 生成（来自外部存储的数据必须包含唯一标识符）

数字属性，包括 [MVAs](../../Creating_a_table/Data_types.md#Multi-value-integer-%28MVA%29)，是普通表中唯一可以更新的元素。表中的所有其他数据都是不可变的。如果需要更新或创建新记录，必须重新构建表。在重建过程中，现有表仍然可用以处理请求，当新版本准备好时，会执行称为 [旋转](../../Data_creation_and_modification/Adding_data_from_external_storages/Rotating_a_table.md) 的过程，将其上线并丢弃旧版本。

#### 普通表构建性能
普通表的索引速度取决于多个因素，包括：
* 数据源检索速度
* [分词设置](../../Creating_a_table/NLP_and_tokenization/Data_tokenization.md)
* 硬件规格（如 CPU、RAM 和磁盘性能）

#### 普通表构建场景
##### 需要时完全重建
对于小数据集，最简单的选择是拥有一个单一的普通表，根据需要完全重建。当：
* 表中的数据不如源中的数据新鲜时
* 随着数据集的增长，构建表所需的时间增加时，  

##### 主+增量场景
对于较大的数据集，普通表可以用作 [实时表](../../Creating_a_table/Local_tables/Real-time_table.md)。主+增量场景包括：
* 创建一个更小的表以进行增量索引
* 使用 [分布式表](../../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md) 合并两个表

这种方法允许更少频繁地重建较大的表，并更频繁地处理来自源的更新。较小的表可以更常被重建（例如，每分钟或甚至每几秒）。

但是，随着时间的推移，较小表的索引时间会变得过长，要求重建较大的表，并清空较小的表。

主+增量模式在 [这个互动课程](https://play.manticoresearch.com/maindelta/) 中有详细说明。

**杀死列表** 和 [killlist_target](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#killlist_target) 指令的机制用于确保当前表中的文档优先于另一个表中的文档。

更多信息请参见 [此处](../../Data_creation_and_modification/Adding_data_from_external_storages/Main_delta.md)。

#### 普通表文件结构
以下表格概述了普通表中使用的各种文件扩展名及其各自描述：

| 扩展名 | 描述 |
| - | - |
|`.spa` | 以 [行式模式](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) 存储文档属性 |
|`.spb` | 以 [行式模式](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) 存储二进制属性：字符串、MVA、json |
|`.spc` | 以 [列式模式](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) 存储文档属性  |
|`.spd` | 存储每个词 ID 的匹配文档 ID 列表 |
|`.sph` | 存储表头信息 |
|`.sphi` | 存储属性值的直方图 |
|`.spi` | 存储词列表（词ID和指向`.spd`文件的指针） |
|`.spidx` | 存储次要索引数据 |
|`.spjidx` | 存储为JSON属性生成的次要索引数据 |
|`.spk` | 存储清除列表 |
|`.spl` | 锁定文件 |
|`.spm` | 存储已删除文档的位图 |
|`.spp` | 存储每个词ID的命中（即发布，即词出现）列表 |
|`.spt` | 存储额外的数据结构以加速按文档ID查找 |
|`.spe` | 存储跳跃列表以加速文档列表过滤 |
|`.spds` | 存储文档文本 |
|`.tmp*` | 索引设置和状态期间的临时文件 |
|`.new.sp*` | 旋转前的普通表的新版本 |
|`.old.sp*` | 旋转后的普通表的旧版本 |

<!-- proofread -->
