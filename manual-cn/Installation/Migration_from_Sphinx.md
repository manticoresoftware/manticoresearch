# 从Sphinx迁移

## Sphinx 2.x -> Manticore 2.x
Manticore Search 2.x 保持了与 Sphinxsearch 2.x 的兼容性，可以加载由 Sphinxsearch 创建的现有表。在大多数情况下，升级仅仅是替换二进制文件。

Manticore 默认使用 `/etc/manticoresearch/manticore.conf`，而不是 sphinx.conf（在 Linux 中通常位于 `/etc/sphinxsearch/sphinx.conf`）。它还在不同的用户下运行，并使用不同的文件夹。

Systemd 服务名称已从 `sphinx/sphinxsearch` 更改为 `manticore`，并且服务在用户 `manticore` 下运行（Sphinx 使用 `sphinx` 或 `sphinxsearch`）。它还使用了一个不同的文件夹来存放 PID 文件。

默认使用的文件夹是 `/var/lib/manticore`、`/var/log/manticore` 和 `/var/run/manticore`。您仍然可以使用现有的 Sphinx 配置，但您需要手动更改 `/var/lib/sphinxsearch` 和 `/var/log/sphinxsearch` 文件夹的权限。或者，您可以在系统文件中全局将 'sphinx' 重命名为 'manticore'。如果您使用其他文件夹（用于数据、词形文件等），其所有权也必须切换到 `manticore` 用户。`pid_file` 位置应更改为与 manticore.service 相匹配，位于 `/var/run/manticore/searchd.pid`。

如果您希望使用 Manticore 文件夹，表文件需要移动到新的数据文件夹（`/var/lib/manticore`），并且权限必须更改为用户 `manticore`。

## Sphinx 2.x / Manticore 2.x -> Manticore 3.x
从 Sphinx / Manticore 2.x 升级到 3.x 并不简单，因为表存储引擎进行了重大升级，新的 searchd 无法加载旧表并将其即时升级到新格式。

Manticore Search 3 重新设计了表存储。使用 Manticore/Sphinx 2.x 创建的表在未经过[转换](../Installation/Migration_from_Sphinx.md#index_converter)的情况下，无法被 Manticore Search 3 加载。由于 4GB 限制，2.x 中的实时表在优化操作后可能仍有多个磁盘块。升级到 3.x 后，这些表现在可以通过常规 [OPTIMIZE](../Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE) 命令优化为一个磁盘块。索引文件也发生了变化。唯一未发生结构变化的组件是 `.spp` 文件（命中列表）。`.sps`（字符串/JSON）和 `.spm`（MVA）现在由 `.spb`（可变长属性）持有。新格式包含 `.spm` 文件，但它用于行映射（以前专用于 MVA 属性）。新增了 `.spt`（docid 查找）、`.sphi`（二级索引直方图）、`.spds`（文档存储）等新扩展名。如果您使用脚本操作表文件，它们应适配新的文件扩展名。

升级过程可能因您的设置而异（集群中的服务器数量、是否有高可用性等），但通常涉及创建 3.x 表的新版本，替换现有的表，以及用新二进制文件替换旧的 2.x 二进制文件。

需要注意的两个特殊要求：

- 需要使用 [FLUSH RAMCHUNK](../Securing_and_compacting_a_table/Flushing_RAM_chunk_to_a_new_disk_chunk.md#FLUSH-RAMCHUNK) 刷新实时表。
- 使用删除列表的普通表需要在表配置中添加一个新指令（参见 [killlist_target](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#killlist_target)）。

Manticore Search 3 包含一个新工具 - [index_converter](../Installation/Migration_from_Sphinx.md#index_converter) - 可将 Sphinx 2.x / Manticore 2.x 表转换为 3.x 格式。`index_converter` 是一个独立的软件包，必须先安装。使用转换工具创建 3.x 版本的表。`index_converter` 可以将新文件写入现有数据文件夹，并备份旧文件，或者将新文件写入选定文件夹。

## 基本升级指南

如果您有单台服务器：

- 安装 manticore-converter 软件包。
- 使用 index_converter 在不同于现有数据文件夹的位置创建表的新版本（使用 `--output-dir` 选项）。
- 停止现有的 Manticore/Sphinx，升级到 3.0，将新表移动到数据文件夹，然后启动 Manticore。

为了将停机时间降到最低，您可以将 2.x 表、配置文件（您需要在这里编辑表、日志和端口的路径）和二进制文件复制到单独的位置，并在单独的端口上启动此副本。指向您的应用程序。在升级到 3.0 并启动新服务器后，您可以将应用程序指回正常端口。如果一切正常，停止 2.x 副本并删除文件以释放空间。

如果您有备用机器（如测试或登台服务器），您可以先在那升级表，甚至安装 Manticore 3 以进行一些测试。如果一切正常，将新表文件复制到生产服务器。如果您有多台服务器可以从生产环境中移除，请逐台进行升级。在分布式设置中，2.x searchd 可以作为 3.x 节点的主节点，因此您可以先在数据节点上进行升级，然后再升级主节点。

客户端连接引擎的方式未发生变化，查询模式或查询行为也没有变化。

## Sphinx / Manticore 2.x 与 Manticore 3.x 中的 kill-lists
在 Manticore Search 3 中，[kill-lists](../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md) 进行了重新设计。在以前的版本中，kill-lists 在查询时应用于每个已搜索表的结果集。

因此，在 2.x 中，查询时表的顺序是有影响的。例如，如果增量表有 kill-list，为了将其应用于主表，顺序必须是主表在前，增量表在后（无论是在分布式表中还是在 FROM 子句中）。

在 Manticore 3 中，kill-lists 会在 searchd 启动时或表被轮换时应用于表。表配置中的新指令 `killlist_target` 指定目标表，并定义来自源表的哪些文档 ID 应用于抑制。这些可以是定义的 kill-list 中的 ID、表的实际文档 ID，或两者兼有。

来自 kill-lists 的文档会从目标表中删除，即使查询中不包含提供 kill-lists 的表，它们也不会出现在结果中。因此，查询时表的顺序不再重要。现在，`delta, main` 和 `main, delta` 将提供相同的结果。

在以前的版本中，表的轮换顺序是按照配置文件中的顺序进行的。而在 Manticore 3 中，表的轮换顺序更智能，遵循 killlist 目标。在开始轮换表之前，服务器会根据 `killlist_target` 定义查找表链。然后，它会先轮换未被任何地方引用为 kill-list 目标的表。接着，它会轮换由已轮换表作为目标的表，依此类推。例如，如果我们执行 `indexer --all`，并且我们有 3 个表：main、delta_big（以 main 为目标）和 delta_small（以 delta_big 为目标），首先会轮换 delta_small，然后是 delta_big，最后是 main。这样可以确保依赖表在轮换时能够从其他表获得最新的 kill-list。

## 在 Manticore 3.x 中移除的配置键
* `docinfo` - 现在一切都外部化了。
* `inplace_docinfo_gap` - 不再需要。
* `mva_updates_pool` - MVA 不再有专用的更新池，因为它们现在可以直接在 blob 中更新（见下文）。

## 在 Manticore 3.x 中更新可变长属性

在 Manticore 3.x 中，可以使用 `UPDATE` 语句更新字符串、JSON 和 MVA 属性。

在 2.x 中，字符串属性需要 `REPLACE`，对于 JSON 只能更新标量属性（因为它们是定宽的），MVA 可以使用 MVA 池进行更新。现在更新直接在 blob 组件上执行。可能需要调整的设置之一是 [attr_update_reserve](../Data_creation_and_modification/Updating_documents/UPDATE.md#attr_update_reserve)，它允许更改 blob 末尾的分配额外空间，以避免新值大于 blob 中现有值时频繁调整大小。

## Manticore 3.x 中的文档 ID
文档 ID 过去是无符号 64 位整数。现在它们是正的有符号 64 位整数。

## Manticore 3.x 中的 RT 模式
关于 [RT 模式](../Read_this_first.md#Real-time-mode-vs-plain-mode)，请点击此处。

## Manticore 3.x 引入的特殊后缀
Manticore 3.x 支持并解析特殊后缀，使得使用具有特殊意义的数值更加简便。常见格式为整数数字 + 字母，如 10k 或 100d，但不支持 40.3s（因为 40.3 不是整数），或 2d 4h（因为有两个值，而非一个）。字母不区分大小写，因此 10W 等同于 10w。目前支持两类后缀：

- 尺寸后缀：可用于定义某些参数（如内存缓冲区、磁盘文件、RAM 限制等）的大小，以字节为单位。未带后缀的数字表示字节数（八位字节）。尺寸值可以使用后缀 `k` 表示千字节（1k=1024 字节），`m` 表示兆字节（1m=1024k），`g` 表示千兆字节（1g=1024m），以及 `t` 表示太字节（1t=1024g）。
- 时间后缀：可用于定义时间间隔参数（如延迟、超时时间等）。未带后缀的值通常有记录的时间单位，必须知道这些数值所代表的时间单位是“秒”还是“毫秒”。使用带后缀的值可以明确指定单位。时间值的后缀包括 `us` 表示微秒，`ms` 表示毫秒，`s` 表示秒，`m` 表示分钟，`h` 表示小时，`d` 表示天，`w` 表示周。

## index_converter

`index_converter` 是一种工具，用于将 Sphinx/Manticore Search 2.x 创建的表转换为 Manticore Search 3.x 的表格式。该工具可以通过多种方式使用：

#### Convert one table at a time

```ini
$ index_converter --config /home/myuser/manticore.conf --index tablename
```

#### Convert all tables

```ini
$ index_converter --config /home/myuser/manticore.conf --all
```

#### Convert tables found in a folder

```ini
$ index_converter  --path /var/lib/manticoresearch/data --all
```

新版本的表默认写入相同的文件夹，之前版本的文件将保存在名称带 `.old` 扩展名的文件中。例外的是 `.spp`（命中列表）文件，它是唯一没有任何结构变化的表组件。

您可以使用 `--output-dir` 选项将新表版本保存到不同的文件夹：

```ini
$ index_converter --config /home/myuser/manticore.conf --all --output-dir /new/path
```

#### 转换 kill 列表

对于包含 kill-lists 的表，这是一个特殊情况。由于 kill-lists 的工作方式已更改（请参阅 [killlist_target](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#killlist_target)），增量表需要知道应用 kill-lists 的目标表。转换表后，有 3 种方法可以设置 kill-lists 的目标表：

* 在转换表时使用 `--killlist-target`
  ```ini
  $ index_converter --config /home/myuser/manticore.conf --index deltaindex --killlist-target mainindex:kl
  ```
* 在执行转换前，将 killlist_target 添加到配置中
* 转换后使用 [ALTER ... KILLIST_TARGET](../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md#killlist_target) 命令

#### index_converter 选项列表

以下是 `index_converter` 选项的完整列表：

- `--config <file>`（简写为 `-c <file>`）指定要使用的配置文件。通常，它会在安装目录中查找 manticore.conf（例如，如果安装在 `/usr/local/sphinx`，则查找 `/usr/local/manticore/etc/manticore.conf`），其次是调用 `index_converter` 时所在的当前目录。
- `--index` 指定要转换的表
- `--path` - 使用包含表的路径，而不是配置文件
- `--strip-path` - 从表中引用的文件名（如停用词、例外词和词形）中去除路径
- `--large-docid` - 允许转换 ID 大于 2^63 的文档并显示警告，否则在遇到大 ID 时将以错误退出。添加此选项是因为在 Manticore 3.x 中，文档 ID 是有符号 bigint，而之前是无符号的
- `--output-dir <dir>` - 将新文件写入选定文件夹，而不是与现有表文件相同的位置。设置此选项后，现有表文件将保持不变。
- `--all` - 转换配置中的所有表
- `--killlist-target <targets>` 设置应用 kill-lists 的目标表。此选项应仅与 `--index` 选项结合使用

<!-- proofread -->
