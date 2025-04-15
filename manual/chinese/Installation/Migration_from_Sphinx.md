# 从Sphinx Search迁移

## Sphinx 2.x -> Manticore 2.x
Manticore Search 2.x 兼容 Sphinxsearch 2.x，并且可以加载已由 Sphinxsearch 创建的现有表。 在大多数情况下，升级仅需替换二进制文件。

Manticore 默认使用的配置文件是 `/etc/manticoresearch/manticore.conf`，而不是 sphinx.conf（在 Linux 中通常位于 `/etc/sphinxsearch/sphinx.conf`）。 它还在不同的用户下运行，并使用不同的文件夹。

Systemd 服务名称已从 `sphinx/sphinxsearch` 更改为 `manticore`，服务以用户 `manticore` 运行（Sphinx 使用的是 `sphinx` 或 `sphinxsearch`）。 它还使用了一个不同的文件夹来存放 PID 文件。

默认使用的文件夹是 `/var/lib/manticore`、`/var/log/manticore`、`/var/run/manticore`。 您仍可以使用现有的 Sphinx 配置，但您需要手动更改 `/var/lib/sphinxsearch` 和 `/var/log/sphinxsearch` 文件夹的权限。 或者，只需在系统文件中将所有 'sphinx' 全局更改为 'manticore'。 如果您使用其他文件夹（用于数据、词形文件等），则所有权也必须切换到用户 `manticore`。 `pid_file` 的位置应更改为匹配 manticore.service 的 `/var/run/manticore/searchd.pid`。

如果您想使用 Manticore 文件夹，则需要将表文件移动到新数据文件夹（`/var/lib/manticore`），并且必须将权限更改为用户 `manticore`。

## Sphinx 2.x / Manticore 2.x -> Manticore 3.x
从 Sphinx / Manticore 2.x 升级到 3.x 并不简单，因为表存储引擎经历了重大升级，新的 searchd 无法加载旧表并在运行时将其升级到新格式。

Manticore Search 3 重新设计了表存储。 使用 Manticore/Sphinx 2.x 创建的表无法被 Manticore Search 3 加载，除非经过 [转换](../Installation/Migration_from_Sphinx.md#index_converter)。 由于 4GB 的限制，2.x 中的实时表在优化操作后仍可能有若干个磁盘块。 升级到 3.x 后，这些表现在可以使用常规的 [OPTIMIZE](../Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE) 命令优化为 1 磁盘块。 索引文件也有所更改。 唯一没有任何结构变化的组件是 `.spp` 文件（命中列表）。 `.sps`（字符串/json）和 `.spm`（MVA）现在由 `.spb`（可变长度属性）持有。 新格式中存在 `.spm` 文件，但它用于行映射（之前专用于 MVA 属性）。 新增加的扩展名有 `.spt`（docid 查找）、`.sphi`（二级索引直方图）、`.spds`（文档存储）。 在您使用操作表文件的脚本的情况下，应该将它们调整为新的文件扩展名。

升级过程可能因您的设置（集群中的服务器数量、是否具有高可用性等）而异，但一般来说，它涉及创建新的 3.x 表版本并替换现有的表，以及用新的二进制文件替换旧的 2.x 二进制文件。

需要注意两个特殊要求：

* 需要使用 [FLUSH RAMCHUNK](../Securing_and_compacting_a_table/Flushing_RAM_chunk_to_a_new_disk_chunk.md#FLUSH-RAMCHUNK) 刷新实时表
* 带有杀死列表的普通表需要在表配置中添加新指令（参见 [killlist_target](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#killlist_target)）

Manticore Search 3 包含一个新工具 - [index_converter](../Installation/Migration_from_Sphinx.md#index_converter) - 可以将 Sphinx 2.x / Manticore 2.x 表转换为 3.x 格式。 `index_converter` 作为单独的包提供，首先应该安装它。 使用转换工具创建 3.x 版本的表。 `index_converter` 可以在现有数据文件夹中写入新文件并备份旧文件，或者可以将新文件写入选定的文件夹。

## 基本升级说明

如果您有一台独立服务器：

* 安装 manticore-converter 包
* 使用 index_converter 在与现有数据文件夹不同的文件夹中创建表的新版本（使用 `--output-dir` 选项）
* 停止现有的 Manticore/Sphinx，升级到 3.0，将新表移动到数据文件夹，并启动 Manticore

为了最小化停机时间，您可以将 2.x 表、配置（您需要在此处编辑表、日志和不同端口的路径）和二进制文件复制到另一个位置，并在另一个端口上启动它。 将您的应用程序指向它。 升级到 3.0 并且新的服务器启动后，您可以将应用程序指回正常端口。 如果一切正常，停止 2.x 副本并删除文件以释放空间。

如果您有一台备用机器（例如测试或暂存服务器），您可以首先在上面进行表升级，甚至安装 Manticore 3 进行几次测试。 如果一切正常，将新的表文件复制到生产服务器。如果有多台服务器可以从生产中拔出，请逐一进行，并在每台上执行升级。 对于分布式设置，2.x 的 searchd 可以作为主节点与 3.x 的节点协作，因此您可以先对数据节点进行升级，然后再对主节点进行升级。

客户端连接引擎的方式、查询模式或查询行为没有任何更改。

## Sphinx / Manticore 2.x 随 Kill-lists vs Manticore 3.x
[杀死列表](../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md) 在 Manticore Search 3 中进行了重新设计。 在以前的版本中，杀死列表在查询时应用于每个先前搜索的表提供的结果集。

因此，在 2.x 中，查询时的表顺序很重要。 例如，如果增量表具有杀死列表，则为了将其应用于主表，顺序必须是主表、增量表（无论是在分布式表中还是在 FROM 子句中）。
在Manticore 3中，杀死列表在搜索启动或旋转期间加载表时应用。表配置中的新指令`killlist_target`指定目标表，并定义应该使用源表中的哪些文档ID进行抑制。这些可以是定义的杀死列表中的ID，表的实际文档ID或两者。

来自杀死列表的文档将从目标表中删除，即使搜索不包括提供杀死列表的表，它们也不会出现在结果中。因此，搜索表的顺序不再重要。现在，`delta, main` 和 `main, delta` 将提供相同的结果。

在以前的版本中，表按照配置文件中的顺序进行轮换。在Manticore 3中，表的轮换顺序更智能，并根据杀死列表目标进行工作。在开始轮换表之前，服务器会根据`killlist_target`定义查找表链。然后，它将首先轮换未在任何地方被引用为杀死列表目标的表。接下来，它将旋转由已旋转表所针对的表，依此类推。例如，如果我们执行`indexer --all`并且我们有3个表：main、delta_big（目标为main）和delta_small（目标为delta_big），首先旋转delta_small，然后是delta_big，最后是main。这是为了确保当一个依赖表被旋转时，它会从其他表获取最新的杀死列表。

## 在Manticore 3.x中删除的配置键
* `docinfo` - 一切现在都是extern
* `inplace_docinfo_gap` - 不再需要
* `mva_updates_pool` - MVAs不再有专用的更新池，现在可以直接在blob中更新（见下文）。

## 在Manticore 3.x中更新可变长度属性
字符串、JSON和MVA属性可以使用`UPDATE`语句在Manticore 3.x中更新。

在2.x中，字符串属性需要`REPLACE`，对于JSON，仅可以更新标量属性（因它们为固定宽度），MVAs可以使用MVA池进行更新。现在更新直接在blob组件上进行。一个可能需要调整的设置是[attr_update_reserve](../Data_creation_and_modification/Updating_documents/UPDATE.md#attr_update_reserve)，它允许更改用于避免频繁调整大小的blob末尾的额外空间，以防新的值大于blob中现有的值。

## 在Manticore 3.x中的文档ID
文档ID曾是无符号64位整数。现在它们是正签名64位整数。

## 在Manticore 3.x中的实时模式
点击这里查看[实时模式](../Read_this_first.md#Real-time-mode-vs-plain-mode)

## 自Manticore 3.x以来的特殊后缀
Manticore 3.x识别并解析特殊后缀，使使用具有特殊含义的数字值更容易。它们的常见形式是整数数字+字面量，如10k或100d，但不是40.3s（因为40.3不是整数），或者不是2d 4h（因为有两个，而不是一个值）。字面量不区分大小写，因此10W与10w是相同的。目前支持这两种类型的后缀：

* 大小后缀 - 可以用于定义某物（内存缓冲区、磁盘文件、RAM限制等）大小的参数，以字节为单位。那地方的“裸体”数字字面意思是字节大小（八位字）。大小值使用后缀`k`表示千字节（1k=1024），`m`表示兆字节（1m=1024k），`g`表示千兆字节（1g=1024m），`t`表示太字节（1t=1024g）。
* 时间后缀 - 可以用于定义某些时间间隔值的参数，如延迟、超时等。这些参数的“裸体”值通常有文档规范，您必须知道它们的数字，比如说100，意味着“100秒”或“100毫秒”。但是，您可以直接写带后缀的值，它将完全由其后缀决定。时间值使用后缀`us`表示微秒，`ms`表示毫秒，`s`表示秒，`m`表示分钟，`h`表示小时，`d`表示天，`w`表示周。

## index_converter

`index_converter`是一个将使用Sphinx/Manticore Search 2.x创建的表转换为Manticore Search 3.x表格式的工具。该工具可以以几种不同的方式使用：

#### 一次转换一个表

```ini
$ index_converter --config /home/myuser/manticore.conf --index tablename
```

#### 转换所有表

```ini
$ index_converter --config /home/myuser/manticore.conf --all
```

#### 转换在文件夹中的表

```ini
$ index_converter  --path /var/lib/manticoresearch/data --all
```

新版本的表默认写入同一文件夹。之前版本的文件保存在其名称中带有`.old`扩展名。一个例外是`.spp`（命中列表）文件，这是新格式中唯一没有任何更改的表组件。

您可以使用`-–output-dir`选项将新表版本保存到不同文件夹

```ini
$ index_converter --config /home/myuser/manticore.conf --all --output-dir /new/path
```

#### 转换杀死列表

针对包含杀死列表的表的特殊情况。由于杀死列表的工作行为发生了变化（见[killlist_target](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#killlist_target)），增量表应该知道哪些是应用杀死列表的目标表。有3种方法可以使转换后的表准备好设置目标表以应用杀死列表：

* 在转换表时使用`-–killlist-target`
  ```ini
  $ index_converter --config /home/myuser/manticore.conf --index deltaindex --killlist-target mainindex:kl
  ```
* 在进行转换之前在配置中添加killlist_target
* 在转换后使用[ALTER ... KILLIST_TARGET](../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md#killlist_target)命令

#### index_converter选项的完整列表

这是 `index_converter` 选项的完整列表：

* `--config <file>`（短格式为 `-c <file>`）告诉 index_converter 使用给定的文件作为其配置。通常，它将在安装目录中查找 manticore.conf（例如，如果安装在 `/usr/local/sphinx`，则为 `/usr/local/manticore/etc/manticore.conf`），随后是在从 shell 调用 index_converter 时所在的当前目录。
* `--index` 指定应转换哪个表
* `--path` - 可以使用包含表的路径，而不是使用配置文件
* `--strip-path` - 从表中引用的文件名中剥离路径：停用词、例外和词形
* `--large-docid` - 允许转换 ID 大于 2^63 的文档并显示警告，否则它将只是在大 ID 处以错误退出。此选项的添加是因为在 Manticore 3.x 中文档 ID 是有符号的 bigint，而之前是无符号的
* `--output-dir <dir>` - 将新文件写入所选文件夹，而不是与现有表文件位于同一位置。当设置此选项时，现有表文件将保持在其位置不变。
* `--all` - 从配置中转换所有表
* `--killlist-target <targets>` 设置将应用杀死列表的目标表。此选项应仅与 `--index` 选项结合使用

<!-- proofread -->

