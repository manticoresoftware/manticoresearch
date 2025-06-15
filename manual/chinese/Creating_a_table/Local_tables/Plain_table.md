# 普通表

**普通表** 是非[渗透](../../Creating_a_table/Local_tables/Percolate_table.md)搜索的基本元素。它只能在配置文件中使用[普通模式](../../Creating_a_table/Local_tables.md#Defining-table-schema-in-config-%28Plain mode%29)定义，不支持[RT模式](../../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29)。它通常与[source](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#source) 配合使用，从[外部存储](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md)处理数据，之后可以[附加](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Attaching_one_table_to_another.md)到**实时表**。

#### 创建普通表

要创建普通表，需要在配置文件中定义。`CREATE TABLE`命令不支持创建普通表。

<!-- example plain -->

这是一个普通表配置示例，以及用于从MySQL数据库获取数据的source：

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

### 👍 普通表的功能：
  * 使用[source](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#source) 和[indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool)从外部存储构建表
  * 对[整数、浮点数、字符串和MVA属性](../../Creating_a_table/Data_types.md)进行就地更新
  * [更新](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md#killlist_target) 其 `killlist_target`

### ⛔ 普通表不能做的事情：
  * 构建后向表中插入额外数据
  * 从表中删除数据
  * 在线创建、删除或更改表结构
  * 使用[UUID](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-ID)自动生成ID（外部存储数据必须包含唯一标识符）

数字型属性，包括[MVA](../../Creating_a_table/Data_types.md#Multi-value-integer-%28MVA%29)，是普通表中唯一可以更新的元素。表中其他数据不可变。如需更新或添加新记录，必须重建表。重建过程中，现有表仍可响应请求，当新版本准备好时，会执行称为[轮换](../../Data_creation_and_modification/Adding_data_from_external_storages/Rotating_a_table.md)的过程，使新表上线并废弃旧版本。

#### 普通表构建性能
普通表索引速度取决于多个因素：
* 数据源获取速度
* [分词设置](../../Creating_a_table/NLP_and_tokenization/Data_tokenization.md)
* 硬件规格（如CPU、内存、磁盘性能）

#### 普通表构建场景
##### 需要时完全重建
对于小型数据集，最简单方案是有一个普通表，根据需要完全重建。当满足下述情况时，该方法可行：
* 表中的数据鲜度不如数据源
* 随数据集增长，构建时间增长

##### 主表+增量表方案
对于大型数据集，普通表可以替代[实时表](../../Creating_a_table/Local_tables/Real-time_table.md)。主表+增量表方案包括：
* 创建较小表用于增量索引
* 使用[分布式表](../../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md)组合这两张表

该方法允许较大表不频繁重建，同时更频繁处理来自源的更新。较小表可以更频繁重建（例如每分钟或每几秒）。

但随着时间推移，较小表的索引时间会变长，需要重建较大表并清空较小表。

主表+增量表方案在[此互动课程](https://play.manticoresearch.com/maindelta/)中有详细讲解。

使用**kill list**机制及[killlist_target](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#killlist_target)指令确保当前表的文档优先于另一表。

更多信息见[这里](../../Data_creation_and_modification/Adding_data_from_external_storages/Main_delta.md)。

#### 普通表文件结构
下表列出了普通表使用的各种文件扩展名及其描述：

| 扩展名 | 描述 |
| - | - |
|`.spa` | 按[行模式](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)存储文档属性 |
|`.spb` | 按[行模式](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)存储blob属性：字符串、MVA、json |
|`.spc` | 按[列模式](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)存储文档属性 |
|`.spd` | 存储每个词ID的匹配文档ID列表 |
|`.sph` | 存储表头信息 |
|`.sphi` | 存储属性值的直方图 |
|`.spi` | 存储词列表（词ID和指向`.spd`文件的指针） |
|`.spidx` | 存储二级索引数据 |
|`.spjidx` | 存储为JSON属性生成的二级索引数据 |
|`.spk` | 存储kill-lists |
|`.spl` | 锁文件 |
|`.spm` | 存储已kill文档的位图 |
|`.spp` | 存储每个词ID的命中（即posting、词出现）列表 |
|`.spt` | 存储用于加速按文档ID查找的额外数据结构 |
|`.spe` | 存储跳跃列表以加快文档列表过滤 |
|`.spds` | 存储文档文本 |
|`.tmp*` | 在 index_settings_and_status 期间的临时文件 |
|`.new.sp*` | 旋转前的纯表新版本 |
|`.old.sp*` | 旋转后的纯表旧版本 |

<!-- proofread -->

