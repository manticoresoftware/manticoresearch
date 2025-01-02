# 普通表

**普通表**是非[透析](../../Creating_a_table/Local_tables/Percolate_table.md)搜索的基本元素。它只能在配置文件中使用[普通模式](../../Creating_a_table/Local_tables.md#Defining-table-schema-in-config-%28Plain mode%29)⛔进行定义，并且不支持[实时模式](../../Creating_a_table/Local_tables.md#在线模式管理（实时模式）)。它通常与[源](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#source)结合使用，从[外部存储](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md)处理数据，并可以后来[附加](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Attaching_one_table_to_another.md)到**实时表**。

#### 创建普通表

要创建一个普通表，您需要在配置文件中定义它。`CREATE TABLE`命令不支持此功能。

<!-- example plain -->

以下是普通表配置及其从MySQL数据库获取数据的源的示例：

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

### 👍 您可以使用普通表做什么：
  * 使用[源](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#source)和[索引器](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-工具)从外部存储构建它
  * 在[整数、浮点数、字符串和MVA属性](../../Creating_a_table/Data_types.md)上执行就地更新
  * [更新](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md#killlist_target)其`killlist_target`

### ⛔ 您不能使用普通表做什么：
  * 在表构建后插入额外数据
  * 从表中删除数据
  * 在线创建、删除或更改表架构
  * 使用[UUID](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#自动-ID)进行自动ID生成（来自外部存储的数据必须包含唯一标识符）

数值属性，包括[MVAs](../../Creating_a_table/Data_types.md#Multi-value-integer-(MVA))，是普通表中唯一可以更新的元素。表中的所有其他数据都是不可变的。如果需要更新或添加新记录，则必须重建表。在重建过程中，现有表保持可用以处理请求，并在新版本准备好时执行称为[轮换](../../Data_creation_and_modification/Adding_data_from_external_storages/Rotating_a_table.md)的过程，将其上线并丢弃旧版本。

#### 普通表构建性能

普通表的索引速度取决于几个因素，包括：
* 数据源检索速度
* [分词设置](../../Creating_a_table/NLP_and_tokenization/Data_tokenization.md)
* 硬件规格（如CPU、RAM和磁盘性能）

#### 普通表构建场景
##### 根据需要完全重建
对于小型数据集，最简单的选择是拥有一个普通表，根据需要完全重建。此方法在以下情况下可接受：

- 表中的数据不如源中的数据新鲜
- 构建表所需的时间随着数据集的增长而增加

##### 主+增量场景
对于较大的数据集，可以使用普通表代替[实时表](../../Creating_a_table/Local_tables/Real-time_table.md)。主+增量场景涉及：
* 创建一个较小的表用于增量索引
* 使用[分布式表](../../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md)合并两个表

这种方法允许较大的表不频繁重建，并更频繁地处理来自源的更新。较小的表可以更频繁地重建（例如，每分钟甚至每几秒）。

但是，随着时间的推移，较小表的索引时间会变得过长，要求重建较大的表并清空较小的表。

主+增量架构在[这个互动课程](https://play.manticoresearch.com/maindelta/)中详细解释。

**杀死列表**的机制和[killlist_target](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#killlist_target)指令用于确保当前表中的文档优先于来自其他表的文档。

有关此主题的更多信息，请参见[这里](../../Data_creation_and_modification/Adding_data_from_external_storages/Main_delta.md)。

### 普通表文件结构

以下表格概述了普通表中使用的各种文件扩展名及其各自的描述：

| 扩展名     | 描述                                                         |
| ---------- | ------------------------------------------------------------ |
| `.spa`     | 以[行模式](../../Creating_a_table/Data_types.md#行存储和列存储属性)存储文档属性 |
| `.spb`     | 以[行模式](../../Creating_a_table/Data_types.md#行存储和列存储属性)存储blob属性：字符串、MVA、json |
| `.spc`     | 以[列模式](../../Creating_a_table/Data_types.md#行存储和列存储属性)存储文档属性 |
| `.spd`     | 为每个词ID存储匹配的文档ID列表                               |
| `.sph`     | 存储表头信息                                                 |
| `.sphi`    | 存储属性值的直方图                                           |
| `.spi`     | 存储词列表（词ID和指向`.spd`文件的指针）                     |
| `.spidx`   | 存储二级索引数据                                             |
| `.spjidx`  | 存储为JSON属性生成的二级索引数据                             |
| `.spk`     | 存储杀死列表                                                 |
| `.spl`     | 锁文件                                                       |
| `.spm`     | 存储已被杀死文档的位图                                       |
| `.spp`     | 为每个词ID存储命中列表（即发布、即词出现）                   |
| `.spt`     | 存储附加数据结构以加速按文档ID的查找                         |
| `.spe`     | 存储跳跃列表以加速文档列表过滤                               |
| `.spds`    | 存储文档文本                                                 |
| `.tmp*`    | 在`index_settings_and_status`期间的临时文件                  |
| `.new.sp*` | 在轮换之前的普通表的新版本                                   |
| `.old.sp*` | 在轮换之后的普通表的旧版本                                   |

<!-- proofread -->
