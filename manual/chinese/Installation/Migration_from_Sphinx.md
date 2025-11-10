# 从 Sphinx Search 迁移

## Sphinx 2.x -> Manticore 2.x
Manticore Search 2.x 保持与 Sphinxsearch 2.x 的兼容性，可以加载由 Sphinxsearch 创建的现有表。在大多数情况下，升级只是替换二进制文件的问题。

Manticore 默认使用 `/etc/manticoresearch/manticore.conf`，而不是 sphinx.conf（在 Linux 中通常位于 `/etc/sphinxsearch/sphinx.conf`）。它还以不同的用户身份运行，并使用不同的文件夹。

Systemd 服务名称已从 `sphinx/sphinxsearch` 更改为 `manticore`，服务以用户 `manticore` 运行（Sphinx 使用的是 `sphinx` 或 `sphinxsearch`）。它还使用不同的文件夹存放 PID 文件。

默认使用的文件夹是 `/var/lib/manticore`、`/var/log/manticore`、`/var/run/manticore`。你仍然可以使用现有的 Sphinx 配置，但需要手动更改 `/var/lib/sphinxsearch` 和 `/var/log/sphinxsearch` 文件夹的权限。或者，直接在系统文件中全局将 'sphinx' 重命名为 'manticore'。如果你使用其他文件夹（用于数据、词形文件等），所有权也必须切换为用户 `manticore`。`pid_file` 位置应更改为与 manticore.service 匹配，即 `/run/manticore/searchd.pid`。

如果你想使用 Manticore 文件夹，则需要将表文件移动到新的数据文件夹（`/var/lib/manticore`），并将权限更改为用户 `manticore`。

## Sphinx 2.x / Manticore 2.x -> Manticore 3.x
从 Sphinx / Manticore 2.x 升级到 3.x 并不简单，因为表存储引擎经历了重大升级，新的 searchd 无法加载旧表并即时升级到新格式。

Manticore Search 3 重新设计了表存储。使用 Manticore/Sphinx 2.x 创建的表无法被 Manticore Search 3 加载，除非进行[转换](../Installation/Migration_from_Sphinx.md#index_converter)。由于 4GB 限制，2.x 中的实时表在优化操作后仍可能有多个磁盘块。升级到 3.x 后，这些表可以通过常规的 [OPTIMIZE](../Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE) 命令优化为单磁盘块。索引文件也发生了变化。唯一未发生结构变化的组件是 `.spp` 文件（命中列表）。`.sps`（字符串/json）和 `.spm`（MVA）现在由 `.spb`（变长属性）持有。新格式中存在 `.spm` 文件，但它用于行映射（之前专用于 MVA 属性）。新增的扩展名有 `.spt`（docid 查找）、`.sphi`（二级索引直方图）、`.spds`（文档存储）。如果你使用脚本操作表文件，应适配新的文件扩展名。

升级过程可能因你的设置（集群中服务器数量、是否有高可用等）而异，但通常涉及创建新的 3.x 表版本并替换现有表，以及用新二进制文件替换旧的 2.x 文件。

有两个特殊要求需要注意：

* 实时表需要使用 [FLUSH RAMCHUNK](../Securing_and_compacting_a_table/Flushing_RAM_chunk_to_a_new_disk_chunk.md#FLUSH-RAMCHUNK) 刷新
* 带有 kill-list 的普通表需要在表配置中添加新指令（参见 [killlist_target](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#killlist_target)）

Manticore Search 3 包含一个新工具 - [index_converter](../Installation/Migration_from_Sphinx.md#index_converter) - 可以将 Sphinx 2.x / Manticore 2.x 表转换为 3.x 格式。`index_converter` 是一个单独的软件包，需要先安装。使用转换工具创建表的 3.x 版本。`index_converter` 可以将新文件写入现有数据文件夹并备份旧文件，或者写入指定文件夹。

## 基本升级说明

如果你只有一台服务器：

* 安装 manticore-converter 软件包
* 使用 index_converter 在不同于现有数据文件夹的文件夹中创建表的新版本（使用 `--output-dir` 选项）
* 停止现有的 Manticore/Sphinx，升级到 3.0，将新表移动到数据文件夹，然后启动 Manticore

为了最小化停机时间，你可以复制 2.x 表、配置（需要编辑表、日志路径和不同端口）、二进制文件到另一个位置，并在不同端口启动。将应用程序指向它。升级到 3.0 并启动新服务器后，可以将应用程序指回正常端口。如果一切正常，停止 2.x 副本并删除文件以释放空间。

如果你有备用机器（如测试或预发布服务器），可以先在那里进行表升级，甚至安装 Manticore 3 进行多次测试。如果一切正常，将新表文件复制到生产服务器。如果你有多台服务器可以从生产环境中抽出，逐台进行升级。对于分布式设置，2.x searchd 可以作为主节点与 3.x 节点共存，因此可以先升级数据节点，再升级主节点。

客户端连接引擎的方式、查询模式或查询行为没有任何变化。

## Sphinx / Manticore 2.x 与 Manticore 3.x 中的 kill-list
[Kill-list](../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md) 在 Manticore Search 3 中进行了重新设计。在之前的版本中，kill-list 在查询时应用于每个先前搜索表提供的结果集。

因此，在 2.x 中，查询时表的顺序很重要。例如，如果增量表有 kill-list，为了对主表应用它，顺序必须是主表、增量表（无论是在分布式表中还是在 FROM 子句中）。

在 Manticore 3 中，kill-list 会在 searchd 启动时加载表或表被轮换时应用。表配置中的新指令 `killlist_target` 指定目标表，并定义应从源表中用于抑制的文档 ID。这些可以是定义的 kill-list 中的 ID、表的实际文档 ID 或两者。

来自 kill-list 的文档会从目标表中删除，即使搜索不包含提供 kill-list 的表，也不会在结果中返回。因此，搜索表的顺序不再重要。现在，`delta, main` 和 `main, delta` 将提供相同的结果。

在以前的版本中，表的轮换遵循配置文件中的顺序。在 Manticore 3 中，表的轮换顺序更智能，并且与 killlist 目标一致。在开始轮换表之前，服务器会根据 `killlist_target` 定义查找表链。然后，它会先轮换未被任何地方作为 kill-list 目标引用的表。接着，轮换已被已轮换表作为目标的表，依此类推。例如，如果我们执行 `indexer --all` 并且有 3 个表：main、delta_big（目标为 main）和 delta_small（目标为 delta_big），则首先轮换 delta_small，然后是 delta_big，最后是 main。这样可以确保当依赖表被轮换时，它能获得来自其他表的最新 kill-list。

## Manticore 3.x 中移除的配置键
* `docinfo` - 现在全部为 extern
* `inplace_docinfo_gap` - 不再需要
* `mva_updates_pool` - MVA 不再有专用的更新池，因为现在它们可以直接在 blob 中更新（见下文）。

## Manticore 3.x 中更新变长属性
字符串、JSON 和 MVA 属性可以在 Manticore 3.x 中使用 `UPDATE` 语句更新。

在 2.x 中，字符串属性需要 `REPLACE`，JSON 只能更新标量属性（因为它们是定长的），MVA 可以使用 MVA 池更新。现在更新直接在 blob 组件上执行。可能需要调整的一个设置是 [attr_update_reserve](../Data_creation_and_modification/Updating_documents/UPDATE.md#attr_update_reserve)，它允许更改在 blob 末尾分配的额外空间，用于避免当新值比现有值大时频繁调整大小。

## Manticore 3.x 中的文档 ID
文档 ID 以前是无符号 64 位整数。现在是正有符号 64 位整数。

## Manticore 3.x 中的 RT 模式
请阅读关于 [RT 模式](../Read_this_first.md#Real-time-mode-vs-plain-mode) 的内容。

## Manticore 3.x 以来的特殊后缀
Manticore 3.x 识别并解析特殊后缀，使得使用带有特殊含义的数值更方便。它们的常见形式是整数 + 字面量，如 10k 或 100d，但不包括 40.3s（因为 40.3 不是整数），也不包括 2d 4h（因为有两个值，而非一个）。字面量不区分大小写，因此 10W 与 10w 相同。目前支持两种类型的后缀：

* 大小后缀 - 可用于定义某些大小（内存缓冲区、磁盘文件、RAM 限制等）参数的字节数。在这些位置的“裸”数字字面意思是字节数（八位字节）。大小值使用后缀 `k` 表示千字节（1k=1024），`m` 表示兆字节（1m=1024k），`g` 表示千兆字节（1g=1024m），`t` 表示太字节（1t=1024g）。
* 时间后缀 - 可用于定义某些时间间隔值，如延迟、超时等。这些参数的“裸”值通常有文档说明的单位，你必须知道数字 100 是表示“100 秒”还是“100 毫秒”。不过，你可以直接写带后缀的值，单位由后缀完全确定。时间值使用后缀 `us` 表示微秒，`ms` 表示毫秒，`s` 表示秒，`m` 表示分钟，`h` 表示小时，`d` 表示天，`w` 表示周。

## index_converter

`index_converter` 是一个用于将 Sphinx/Manticore Search 2.x 创建的表转换为 Manticore Search 3.x 表格式的工具。该工具可以通过多种方式使用：

#### 一次转换一个表

```ini
$ index_converter --config /home/myuser/manticore.conf --index tablename
```

#### 转换所有表

```ini
$ index_converter --config /home/myuser/manticore.conf --all
```

#### 转换文件夹中找到的表

```ini
$ index_converter  --path /var/lib/manticoresearch/data --all
```

新版本的表默认写入同一文件夹。旧版本的文件会以 `.old` 扩展名保存。例外的是 `.spp`（hitlists）文件，这是唯一在新格式中没有任何更改的表组件。

你可以使用 `-–output-dir` 选项将新表版本保存到不同的文件夹

```ini
$ index_converter --config /home/myuser/manticore.conf --all --output-dir /new/path
```

#### 转换 kill list

包含 kill-list 的表是特殊情况。由于 kill-list 的工作方式发生了变化（参见 [killlist_target](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#killlist_target)），delta 表应知道应用 kill-list 的目标表。转换表后准备设置应用 kill-list 的目标表有三种方法：

* 转换表时使用 `-–killlist-target`
  ```ini
  $ index_converter --config /home/myuser/manticore.conf --index deltaindex --killlist-target mainindex:kl
  ```
* 在转换前在配置中添加 killlist_target
* 转换后使用 [ALTER ... KILLIST_TARGET](../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md#killlist_target) 命令

#### index_converter 选项完整列表

以下是 `index_converter` 选项的完整列表：

* `--config <file>`（简写为 `-c <file>`）告诉 index_converter 使用指定的配置文件。通常，它会在安装目录（例如，如果安装在 `/usr/local/sphinx`，则为 `/usr/local/manticore/etc/manticore.conf`）中查找 manticore.conf，然后是你从 shell 调用 index_converter 时所在的当前目录。
* `--index` 指定要转换的表
* `--path` - 不使用配置文件，而是使用包含表的路径
* `--strip-path` - 从表引用的文件名中去除路径：停用词、例外和词形变化
* `--large-docid` - 允许转换文档ID大于2^63的文档并显示警告，否则遇到大ID时会直接报错退出。添加此选项是因为在Manticore 3.x中，文档ID是有符号的bigint，而之前是无符号的
* `--output-dir <dir>` - 将新文件写入指定文件夹，而不是与现有表文件相同的位置。设置此选项时，现有表文件将保持原位置不变。
* `--all` - 转换配置中的所有表
* `--killlist-target <targets>` 设置将应用kill-list的目标表。此选项应仅与`--index`选项一起使用

<!-- proofread -->

