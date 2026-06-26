# 从 Sphinx Search 迁移

## Sphinx 2.x -> Manticore 2.x  
Manticore Search 2.x 与 Sphinxsearch 2.x 保持兼容，可以加载由 Sphinxsearch 创建的现有表。在大多数情况下，升级仅仅是替换二进制文件的问题。

Manticore 默认使用 `/etc/manticoresearch/manticore.conf`，而不是 sphinx.conf（在 Linux中通常位于 `/etc/sphinxsearch/sphinx.conf`）。它还以不同的用户身份运行，并使用不同的文件夹。

Systemd 服务名称已从 `sphinx/sphinxsearch` 改为 `manticore`，服务以 `manticore` 用户身份运行（Sphinx 使用的是 `sphinx` 或 `sphinxsearch`）。此外，它还使用不同的文件夹来存放 PID 文件。

默认使用的文件夹是 `/var/lib/manticore`、`/var/log/manticore`、`/var/run/manticore`。您仍可以使用现有的 Sphinx 配置，但您需要手动更改 `/var/lib/sphinxsearch` 和 `/var/log/sphinxsearch` 文件夹的权限。或者，直接在系统文件中全局将 'sphinx' 重命名为 'manticore'。如果您使用其他文件夹（用于数据，词形文件等），所有权也必须切换为用户 `manticore`。`pid_file` 位置应更改为与 manticore.service 匹配，即 `/run/manticore/searchd.pid`。

如果您想使用 Manticore 文件夹，则需将表文件移动到新的数据文件夹（`/var/lib/manticore`），并将权限更改为用户 `manticore`。

## Sphinx 2.x / Manticore 2.x -> Manticore 3.x  
从 Sphinx / Manticore 2.x 升级到 3.x 并不简单，因为表存储引擎进行了重大升级，新的 searchd 无法加载旧表并即时升级它们到新格式。

Manticore Search 3 重新设计了表存储。由 Manticore/Sphinx 2.x 创建的表无法被 Manticore Search 3 加载，除非进行[转换](../Installation/Migration_from_Sphinx.md#index_converter)。由于 4GB 的限制，2.x 中的实时表在优化后仍可能有多个磁盘块。升级到 3.x 后，这些表现在可以通过常规的 [OPTIMIZE](../Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE) 命令优化为单个磁盘块。索引文件也发生了变化。唯一没有结构变化的组件是 `.spp` 文件（命中列表）。`.sps`（字符串/JSON）和 `.spm`（MVA）现在被包含在 `.spb`（变长属性）中。新格式仍有 `.spm` 文件，但用作行映射（之前专用于 MVA 属性）。新增的扩展名有 `.spt`（docid 查找）、`.sphi`（二级索引直方图）、`.spds`（文档存储）。如果您使用脚本操作表文件，需调整以适应新文件扩展名。

升级流程可能因您的设置不同（集群中服务器数量，是否有高可用等）而异，但一般包括创建新的 3.x 表版本并替换现有表，以及用新二进制替换旧 2.x 二进制。

有两个特殊要求需要注意：

* 需要使用 [FLUSH RAMCHUNK](../Securing_and_compacting_a_table/Flushing_RAM_chunk_to_a_new_disk_chunk.md#FLUSH-RAMCHUNK) 将实时表刷新  
* 带有 kill-list 的纯表需在表配置中添加新的指令（参见 [killlist_target](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#killlist_target)）

Manticore Search 3 包含了一个新工具 - [index_converter](../Installation/Migration_from_Sphinx.md#index_converter) - 能够将 Sphinx 2.x / Manticore 2.x 表转换为 3.x 格式。`index_converter` 是一个独立的包，需要先安装。使用该转换工具创建 3.x 版本的表。`index_converter` 可以将新文件写入现有数据文件夹并备份旧文件，也可以写入指定文件夹。

## 基本升级指南

如果您只有单台服务器：

* 安装 manticore-converter 包  
* 使用 index_converter 在与现有数据文件夹不同的文件夹中创建新版本的表（使用 `--output-dir` 选项）  
* 停止现有的 Manticore/Sphinx，升级至 3.0，将新表移入数据文件夹，启动 Manticore

为了减少停机时间，您可以复制 2.x 的表、配置（需要修改表路径、日志路径及端口号）、二进制文件到一个单独位置，并在不同端口启动。指向应用程序到该地址。升级到 3.0 并启动新服务器后，再将应用程序指向正常端口。一切正常后，停止并删除 2.x 副本文件以释放空间。

如果您有备用机器（如测试或预发布服务器），可以先在那里升级表并安装 Manticore 3 进行测试。测试通过后将新表文件拷贝至生产服务器。如果您有多个可以下线的服务器，逐一升级。对于分布式设置，2.x 的 searchd 可作为主节点与 3.x 节点协作，因此可以先升级数据节点，然后再升级主节点。

客户端连接引擎的方式、查询模式或查询行为均未发生变化。

## Sphinx / Manticore 2.x 与 Manticore 3.x 中的 kill-list 差异  
[Kill-list](../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md) 在 Manticore Search 3 中进行了重新设计。以前版本中，kill-list 是在查询时应用于每个已搜索表的结果集。

因此，在 2.x 中，查询时表的顺序很重要。例如，如果一个增量表有 kill-list，为了对主表应用 kill-list，查询顺序必须是主表先，然后是增量表（无论是在分布式表中还是在 FROM 子句中）。

在 Manticore 3 中，杀死列表（kill-lists）会在 searchd 启动时加载表或进行轮换时应用到表上。表配置中的新指令 `killlist_target` 指定目标表，并定义应从源表中使用哪些文档 ID 进行抑制。这些可以是定义的杀死列表中的 ID、表的实际文档 ID，或两者。

杀死列表中的文档会从目标表中删除，即使搜索不包括提供杀死列表的表，这些文档也不会出现在结果中。因此，搜索表的顺序不再重要。现在，`delta, main` 和 `main, delta` 将提供相同的结果。

在以前版本中，表轮换遵循配置文件中的顺序。在 Manticore 3 中，表的轮换顺序更智能，并且符合 killlist 目标。启动轮换表之前，服务器会通过 `killlist_target` 定义查找表链。然后它会首先轮换未被作为杀死列表目标引用的表。接下来，它会轮换被已轮换表所针对的表，依此类推。例如，如果执行 `indexer --all` 并且有 3 个表：main，delta_big（针对 main）和 delta_small（针对 delta_big），则首先轮换 delta_small，然后轮换 delta_big，最后轮换 main。这是为了确保当依赖的表被轮换时，能从其他表获取最新的杀死列表。

## Manticore 3.x 中移除的配置键
* `docinfo` - 现在一切都是 extern
* `inplace_docinfo_gap` - 不再需要
* `mva_updates_pool` - MVAs 不再有专门的更新池，因为现在它们可以直接在 blob 中更新（见下文）。

## Manticore 3.x 中的变长属性更新
字符串、JSON 和 MVA 属性可以通过 Manticore 3.x 中的 `UPDATE` 语句更新。

在 2.x 中，字符串属性需要使用 `REPLACE`，JSON 只能更新标量属性（因为它们是定长的），MVA 可以通过 MVA 池来更新。现在更新直接在 blob 组件上执行。可能需要调整的一个设置是 [attr_update_reserve](../Data_creation_and_modification/Updating_documents/UPDATE.md#attr_update_reserve)，该设置允许更改在 blob 末尾为避免频繁调整大小而分配的额外空间大小，当新值比现有 blob 中的值更大时。

## Manticore 3.x 中的文档 ID
文档 ID 过去是无符号 64 位整数。现在它们是正有符号 64 位整数。

## Manticore 3.x 中的 RT 模式
请阅读有关 [RT 模式](../Read_this_first.md#Real-time-mode-vs-plain-mode) 的内容。

## Manticore 3.x 起的特殊后缀
Manticore 3.x 识别并解析特殊后缀，使使用带有特殊含义的数值更容易。它们的常见形式为整数数字 + 字面量，比如 10k 或 100d，但不包括 40.3s（因为 40.3 不是整数），也不包括 2d 4h（因为这有两个值，而不是一个值）。字面量不区分大小写，所以 10W 和 10w 是相同的。目前支持两种类型的此类后缀：

* 大小后缀 — 可用于定义某些大小（内存缓冲区、磁盘文件、RAM 限制等）的参数，以字节为单位。在这些地方，“裸”数字字面意思是大小的字节数（八位组）。大小值后缀包括 `k` 表示千字节（1k=1024），`m` 表示兆字节（1m=1024k），`g` 表示千兆字节（1g=1024m）及 `t` 表示太字节（1t=1024g）。
* 时间后缀 — 可用于定义某些时间间隔值的参数，如延迟、超时等。“裸”值通常有文档中说明的量度单位，你必须知道它们的数字，如 100，是表示“100秒”还是“100毫秒”。不过无需猜测，只需写带后缀的值即可，由后缀完全决定时间单位。时间后缀包括 `us` 表示微秒，`ms` 表示毫秒，`s` 表示秒，`m` 表示分钟，`h` 表示小时，`d` 表示天及 `w` 表示周。

## index_converter

`index_converter` 是一个将使用 Sphinx/Manticore Search 2.x 创建的表转换为 Manticore Search 3.x 表格式的工具。该工具可以用于多种方式：

#### 一次转换一个表

```ini
$ index_converter --config /home/myuser/manticore.conf --index tablename
```

#### 转换所有表

```ini
$ index_converter --config /home/myuser/manticore.conf --all
```

#### 转换文件夹中发现的表

```ini
$ index_converter  --path /var/lib/manticoresearch/data --all
```

新版表默认写入同一文件夹。前一版本的文件会以 `.old` 扩展名保存。例外的是 `.spp`（命中列表）文件，这是唯一在新格式中没有任何更改的表组件。

你可以使用 `-–output-dir` 选项将新表版本保存到不同文件夹。

```ini
$ index_converter --config /home/myuser/manticore.conf --all --output-dir /new/path
```

#### 转换杀死列表

杀死列表表是特殊情况。由于杀死列表行为已更改（参见 [killlist_target](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#killlist_target)），增量表应知道用于应用杀死列表的目标表。有 3 种方法可以将转换后的表准备好设置应用杀死列表的目标表：

* 转换表时使用 `-–killlist-target`
  ```ini
  $ index_converter --config /home/myuser/manticore.conf --index deltaindex --killlist-target mainindex:kl
  ```
* 在转换前在配置中添加 killlist_target
* 转换后使用 [ALTER ... KILLIST_TARGET](../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md#killlist_target) 命令

#### index_converter 选项完整列表

以下是 `index_converter` 的完整选项列表：

* `--config <file>`（简写为 `-c <file>`）告诉 index_converter 使用指定的文件作为配置文件。通常，它会在安装目录中查找 manticore.conf（例如，如果安装在 `/usr/local/sphinx`，则查找 `/usr/local/manticore/etc/manticore.conf`），然后查找调用 index_converter 时所在的当前目录。
* `--index` 指定要转换的表
* `--path` - 不使用配置文件，改为使用包含表的路径
* `--strip-path` - 从表引用的文件名中去除路径：stopwords、exceptions 和 wordforms
* `--large-docid` - 允许转换 ID 大于 2^63 的文档并显示警告，否则遇到过大的 ID 会直接报错退出。此选项是因为 Manticore 3.x 中文档 ID 是有符号 bigint，而之前是无符号的
* `--output-dir <dir>` - 将新文件写入指定文件夹，而不是放到现有表文件所在位置。设置此选项时，现有表文件将保持其原位置不变。
* `--all` - 转换配置中的所有表
* `--killlist-target <targets>` 设置要应用 kill-list 的目标表。此选项应仅与 `--index` 选项配合使用

<!-- proofread -->

