# 从 Sphinx Search 迁移

## Sphinx 2.x -> Manticore 2.x
Manticore Search 2.x 与 Sphinxsearch 2.x 兼容，可以加载由 Sphinxsearch 创建的现有表。在大多数情况下，升级只需替换二进制文件。

Manticore 默认使用 `/etc/manticoresearch/manticore.conf`，而不是 sphinx.conf （在 Linux 中通常位于 `/etc/sphinxsearch/sphinx.conf`）。它也以不同的用户身份运行，并使用不同的文件夹。

Systemd 服务名称已由 `sphinx/sphinxsearch` 改为 `manticore`，服务以 `manticore` 用户身份运行（Sphinx 使用 `sphinx` 或 `sphinxsearch`）。它还使用了不同的 PID 文件夹。

默认使用的文件夹是 `/var/lib/manticore`、`/var/log/manticore`、`/var/run/manticore`。您仍然可以使用现有的 Sphinx 配置，但需要手动更改 `/var/lib/sphinxsearch` 和 `/var/log/sphinxsearch` 文件夹的权限。或者，直接在系统文件中全局将 'sphinx' 重命名为 'manticore'。如果您使用其他文件夹（用于数据、词形文件等），所有权也必须切换为 `manticore` 用户。`pid_file` 位置应更改为与 manticore.service 匹配，即 `/var/run/manticore/searchd.pid`。

如果您想使用 Manticore 的文件夹，表文件需要移动到新的数据文件夹（`/var/lib/manticore`），并且权限必须改为 `manticore` 用户。

## Sphinx 2.x / Manticore 2.x -> Manticore 3.x
从 Sphinx / Manticore 2.x 升级到 3.x 并非简单，因为表存储引擎经过了重大升级，新的 searchd 无法加载旧表并即时升级到新格式。

Manticore Search 3 采用了重新设计的表存储结构。用 Manticore/Sphinx 2.x 创建的表无法被 Manticore Search 3 加载，必须进行[转换](../Installation/Migration_from_Sphinx.md#index_converter)。由于 4GB 限制，2.x 中的实时表在优化操作后仍可能存在多个磁盘块。升级到 3.x 后，这些表可以通过常用的 [OPTIMIZE](../Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE) 命令优化为单个磁盘块。索引文件也发生了变化。唯一没有结构变化的是 `.spp` 文件（hitlists）。`.sps`（字符串/json）和 `.spm`（MVA）现在放在 `.spb`（变长属性）中。新的格式中 `.spm` 文件仍然存在，但用于行映射（之前专用于 MVA 属性）。新增的扩展名有 `.spt`（docid 查找）、`.sphi`（二级索引直方图）、`.spds`（文档存储）。如果您使用脚本操作表文件，应适配新文件扩展名。

升级步骤可能因设置不同（集群中服务器数量，是否有高可用等）而异，但总体上，涉及创建新的 3.x 版本表，替换现有表，以及替换旧的 2.x 二进制文件为新版本。

有两个特别需求需注意：

* 需要使用 [FLUSH RAMCHUNK](../Securing_and_compacting_a_table/Flushing_RAM_chunk_to_a_new_disk_chunk.md#FLUSH-RAMCHUNK) 刷新实时表
* 带有 kill-lists 的普通表需要在表配置中添加新指令（见 [killlist_target](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#killlist_target)）

Manticore Search 3 包含了新工具 - [index_converter](../Installation/Migration_from_Sphinx.md#index_converter)，用于将 Sphinx 2.x / Manticore 2.x 表转换为 3.x 格式。`index_converter` 是单独的包，需要先安装。使用转换工具创建 3.x 版本的表。`index_converter` 可以将新文件写入现有数据文件夹并备份旧文件，或者写入选定文件夹。

## 基础升级说明

如果只有单台服务器：

* 安装 manticore-converter 包
* 使用 index_converter 在不同于现有数据文件夹的文件夹中创建新版本表（使用 `--output-dir` 选项）
* 停止现有 Manticore/Sphinx，升级到 3.0，将新表移动到数据文件夹，启动 Manticore

为了减少停机时间，您可以复制 2.x 表、配置（需要在这里编辑表路径、日志路径和端口等）和二进制文件到一个单独位置，并在不同端口启动。将应用指向该端口。升级到 3.0 并启动新服务器后，您可以将应用指向正常端口。如果一切正常，停止 2.x 副本并删除相关文件以释放空间。

如果有备用服务器（如测试或预发布服务器），可先在那里升级表并安装 Manticore 3 进行测试。若无误，将新表复制到生产服务器。如果有多台服务器可从生产环境下线，一台台地升级。对于分布式环境，2.x 的 searchd 可以作为主节点配合 3.x 节点使用，因此可以先升级数据节点，再升级主节点。

客户端连接引擎的方式，查询模式及查询行为均未更改。

## Sphinx / Manticore 2.x 与 Manticore 3.x 中的 kill-lists 对比
Manticore Search 3 中的[Kill-lists](../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md) 经过重新设计。在之前版本中，kill-lists 在查询时应用于每个已查询表返回的结果集合。

因此，在 2.x 中，查询时表的顺序很重要。例如，如果增量表有 kill-list，为使其作用于主表，查询顺序必须是主表、增量表（无论是在分布式表中还是 FROM 子句中）。

在 Manticore 3 中，kill-lists 会在 searchd 启动加载表或表旋转时应用。表配置中新指令 `killlist_target` 指定目标表，并定义应从源表中使用哪些文档 ID 进行屏蔽。这些可以是定义的 kill-list 中的 ID，表的实际文档 ID，或者两者兼有。

来自 kill-lists 的文档会从目标表中删除，即使搜索不包含提供 kill-lists 的表，也不会返回这些文档。因此，搜索表的顺序不再重要。现在，`delta, main` 和 `main, delta` 会提供相同的结果。

在之前的版本中，表的旋转顺序遵循配置文件中的顺序。在 Manticore 3 中，表的旋转顺序更加智能，并且与 killlist 目标保持一致。服务器在开始旋转表之前，会根据 `killlist_target` 定义寻找表链。然后，它会先旋转未被任何地方作为 kill-list 目标引用的表。接下来旋转已被已旋转表作为目标的表，依此类推。例如，如果执行 `indexer --all` 并且有三个表：main、delta_big（目标为 main）和 delta_small（目标为 delta_big），则首先旋转 delta_small，接着是 delta_big，最后是 main。这样可以确保当依赖表被旋转时，它获得来自其他表的最新 kill-list。

## Manticore 3.x 中移除的配置键
* `docinfo` - 现在一切均为 extern
* `inplace_docinfo_gap` - 不再需要
* `mva_updates_pool` - MVA 不再拥有专用的更新池，因为现在它们可以直接在 blob 中更新（见下文）。

## Manticore 3.x 中可变长度属性的更新
字符串、JSON 和 MVA 属性可以在 Manticore 3.x 中使用 `UPDATE` 语句更新。

在 2.x 中，字符串属性需要使用 `REPLACE`，JSON 只能更新标量属性（因为它们是固定宽度的），MVA 可以使用 MVA 池更新。现在更新直接在 blob 组件上执行。一个可能需要调整的设置是 [attr_update_reserve](../Data_creation_and_modification/Updating_documents/UPDATE.md#attr_update_reserve)，它允许更改在 blob 末尾分配的额外空间，用来避免在新值比现有值更大时频繁调整大小。

## Manticore 3.x 中的文档 ID
文档 ID 以前是无符号 64 位整数。现在它们是正有符号 64 位整数。

## Manticore 3.x 中的 RT 模式
请在此处阅读有关 [RT 模式](../Read_this_first.md#Real-time-mode-vs-plain-mode) 的内容

## Manticore 3.x 起的特殊后缀
Manticore 3.x 识别并解析特殊后缀，这使得具有特殊含义的数字值使用更加方便。它们的常见形式是整数 + 字面量，比如 10k 或 100d，但不包括 40.3s（因为 40.3 不是整数），也不包括 2d 4h（因为有两个值，而非一个）。字面量不区分大小写，因此 10W 与 10w 相同。目前支持两种此类后缀：

* 大小后缀 - 用于定义大小（内存缓冲区、磁盘文件、RAM 限制等）参数中的字节数。在这些位置裸数字表示字节数（八位组）。大小值的后缀为 `k` 表示千字节（1k=1024），`m` 表示兆字节（1m=1024k），`g` 表示千兆字节（1g=1024m），`t` 表示太字节（1t=1024g）。
* 时间后缀 - 用于定义时间间隔参数如延迟、超时等。一般裸数值在文档中有说明其计量单位，你必须知道数字 100 代表“100秒”还是“100毫秒”。但你可以直接写带后缀的数值，后缀决定了单位。时间值后缀为 `us`（微秒）、`ms`（毫秒）、`s`（秒）、`m`（分钟）、`h`（小时）、`d`（天）、`w`（周）。

## index_converter

`index_converter` 是一个工具，用于将使用 Sphinx/Manticore Search 2.x 创建的表转换为 Manticore Search 3.x 的表格式。该工具可以通过多种方式使用：

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

新版本的表默认写入同一文件夹。之前版本的文件以 `.old` 扩展名备份。例外的是 `.spp`（hitlists） 文件，它是唯一在新格式中没有变化的表组件。

你可以使用 `--output-dir` 选项将新表版本保存到不同的文件夹

```ini
$ index_converter --config /home/myuser/manticore.conf --all --output-dir /new/path
```

#### 转换 kill lists

kill-lists 表是个特殊情况。由于 kill-lists 的工作方式发生变化（见 [killlist_target](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#killlist_target)），delta 表应知道哪些是应用 kill-lists 的目标表。有三种方式使转换后的表准备好设置目标表来应用 kill-lists：

* 转换表时使用 `--killlist-target`
  ```ini
  $ index_converter --config /home/myuser/manticore.conf --index deltaindex --killlist-target mainindex:kl
  ```
* 在转换前在配置中添加 killlist_target
* 转换后使用 [ALTER ... KILLIST_TARGET](../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md#killlist_target) 命令

#### index_converter 选项完整清单

以下是 `index_converter` 的完整选项列表：

* `--config <file>`（缩写为 `-c <file>`）指示 index_converter 使用指定配置文件。通常，它会优先查找安装目录中的 manticore.conf 文件（例如，安装到 `/usr/local/sphinx` 时为 `/usr/local/manticore/etc/manticore.conf`），其次是你从 shell 调用 index_converter 时的当前目录。
* `--index` 指定哪个表将被转换
* `--path` - 不使用配置文件，而是使用包含表的路径
* `--strip-path` - 从表引用的文件名中去除路径：停用词、例外和词形变化文件
* `--large-docid` - 允许转换 ID 大于 2^63 的文档并显示警告，否则遇到大 ID 时会错误退出。添加此选项是因为在 Manticore 3.x 中，文档 ID 是有符号 bigint，而之前是无符号的
* `--output-dir <dir>` - 将新文件写入所选文件夹，而不是和现有表文件相同的位置。设置此选项时，现有表文件将在其原位置保持不变。
* `--all` - 转换配置中的所有表
* `--killlist-target <targets>` 设置应用 kill-list 的目标表。此选项应仅与 `--index` 选项一起使用。

<!-- proofread -->

