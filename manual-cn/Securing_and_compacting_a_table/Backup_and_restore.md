# 备份与恢复

定期备份您的表对于在系统崩溃、硬件故障或数据损坏/丢失的情况下进行恢复至关重要。在升级到新版本的 Manticore Search 或运行 [ALTER TABLE](../Updating_table_schema_and_settings.md#在实时模式下更新表结构) 之前，强烈建议进行备份。

备份数据库系统可以通过两种独特的方式进行：逻辑备份和物理备份。这两种方法各有优缺点，具体可能因数据库环境和需求的不同而有所差异。下面我们将深入探讨这两种备份的区别。

### 逻辑备份

逻辑备份涉及将数据库架构和数据导出为 SQL 语句或特定于数据库的数据格式。这种备份形式通常是可读的，并且可以在不同系统或数据库引擎上恢复数据库。

逻辑备份的优缺点：

- ➕ **可移植性：** 逻辑备份通常比物理备份更具可移植性，因为它们可以用于在不同硬件或操作系统上恢复数据库。
- ➕ **灵活性：** 逻辑备份允许您选择性地恢复特定表、索引或其他数据库对象。
- ➕ **兼容性：** 逻辑备份可用于在不同的数据库管理系统或版本之间迁移数据，前提是目标系统支持导出的格式或 SQL 语句。
- ➖ **备份和恢复速度较慢：** 逻辑备份通常比物理备份速度较慢，因为它们需要数据库引擎将数据转换为 SQL 语句或其他导出格式。
- ➖ **系统负载增加：** 创建逻辑备份可能会导致系统负载增加，因为该过程需要更多的 CPU 和内存资源来处理和导出数据。

Manticore Search 支持使用 [mysqldump](../Securing_and_compacting_a_table/Backup_and_restore.md#使用-mysqldump-进行备份和恢复) 进行逻辑备份。

### 物理备份

物理备份涉及复制构成数据库的原始数据文件和系统文件。这种类型的备份基本上是在给定时间点创建数据库物理状态的快照。

物理备份的优缺点：

- ➕ **速度：** 物理备份通常比逻辑备份更快，因为它们涉及直接从磁盘复制原始数据文件。
- ➕ **一致性：** 物理备份确保整个数据库的一致性备份，因为所有相关文件都是一起复制的。
- ➕ **较低的系统负载：** 创建物理备份通常对系统的负载较小，因为该过程不涉及额外的数据处理。
- ➖ **可移植性：** 物理备份通常比逻辑备份的可移植性较低，因为它们可能依赖于特定的硬件、操作系统或数据库引擎配置。
- ➖ **灵活性：** 物理备份不允许选择性地恢复特定数据库对象，因为备份包含整个数据库的原始文件。
- ➖ **兼容性：** 物理备份不能在不同的数据库管理系统或版本之间迁移数据，因为原始数据文件可能在不同的平台或软件之间不兼容。

Manticore Search 提供了 [manticore-backup](../Securing_and_compacting_a_table/Backup_and_restore.md#使用-manticore-backup-命令行工具) 命令行工具以进行物理备份。

总之，逻辑备份提供了更多的灵活性、可移植性和兼容性，但速度较慢且资源消耗较高；而物理备份速度更快、一致性更好、资源消耗较低，但在可移植性和灵活性方面可能受到限制。选择这两种备份方法将取决于您特定的数据库环境、硬件和需求。

## 使用 manticore-backup 命令行工具

`manticore-backup` 工具包含在官方 Manticore Search [软件包](https://manticoresearch.com/install) 中，自动化备份运行在 [RT 模式](../Read_this_first.md#实时模式-vs-普通模式) 下的表的过程。

### 安装

**如果您遵循了 [官方安装说明](https://manticoresearch.com/install/)，您应该已经安装了所有内容，无需担心。** 否则，[`manticore-backup`](https://github.com/manticoresoftware/manticoresearch-backup) 需要 PHP 8.1.10 和 [特定模块](https://github.com/manticoresoftware/executor/blob/main/build-linux)，或 [`manticore-executor`](https://github.com/manticoresoftware/executor)，这是 `manticore-extra` 软件包的一部分，您需要确保其中之一可用。

请注意，`manticore-backup` 目前不支持 Windows。

### 如何使用

首先，确保在与您即将备份的 Manticore 实例运行的同一服务器上运行 `manticore-backup`。

其次，建议以 `root` 用户身份运行该工具，以便工具可以转移您正在备份的文件的所有权。否则，也会进行备份，但不进行所有权转移。在任何情况下，您都应确保 `manticore-backup` 可以访问 Manticore 实例的数据目录。

`manticore-backup` 唯一必需的参数是 `--backup-dir`，它指定备份的目标位置。如果您没有提供任何其他参数，`manticore-backup` 将：

- 定位到使用默认配置运行的 Manticore 实例
- 在 `--backup-dir` 目录中创建一个带有时间戳名称的子目录
- 备份实例中找到的所有表

<!-- request Example -->
```bash
manticore-backup --config=path/to/manticore.conf --backup-dir=backupdir
```

<!-- response Example -->
```bash
Copyright (c) 2023-2024, Manticore Software LTD (https://manticoresearch.com)

Manticore config file: /etc/manticoresearch/manticore.conf
Tables to backup: all tables
Target dir: /mnt/backup/

Manticore config
  endpoint =  127.0.0.1:9308

Manticore versions:
  manticore: 5.0.2
  columnar: 1.15.4
  secondary: 1.15.4
2022-10-04 17:18:39 [Info] Starting the backup...
2022-10-04 17:18:39 [Info] Backing up config files...
2022-10-04 17:18:39 [Info]   config files - OK
2022-10-04 17:18:39 [Info] Backing up tables...
2022-10-04 17:18:39 [Info]   pq (percolate) [425B]...
2022-10-04 17:18:39 [Info]    OK
2022-10-04 17:18:39 [Info]   products (rt) [512B]...
2022-10-04 17:18:39 [Info]    OK
2022-10-04 17:18:39 [Info] Running sync
2022-10-04 17:18:42 [Info]  OK
2022-10-04 17:18:42 [Info] You can find backup here: /mnt/backup/backup-20221004171839
2022-10-04 17:18:42 [Info] Elapsed time: 2.76s
2022-10-04 17:18:42 [Info] Done
```
<!-- end -->

<!-- example backup2 -->
要仅备份特定表，可以使用 `--tables` 标志，后跟以逗号分隔的表名，例如 `--tables=tbl1,tbl2`。这将只备份指定的表，忽略其他表。

<!-- request Example -->
```bash
manticore-backup --backup-dir=/mnt/backup/ --tables=products
```

<!-- response Example -->
```bash
Copyright (c) 2023-2024, Manticore Software LTD (https://manticoresearch.com)

Manticore config file: /etc/manticoresearch/manticore.conf
Tables to backup: products
Target dir: /mnt/backup/

Manticore config
  endpoint =  127.0.0.1:9308

Manticore versions:
  manticore: 5.0.3
  columnar: 1.16.1
  secondary: 0.0.0
2022-10-04 17:25:02 [Info] Starting the backup...
2022-10-04 17:25:02 [Info] Backing up config files...
2022-10-04 17:25:02 [Info]   config files - OK
2022-10-04 17:25:02 [Info] Backing up tables...
2022-10-04 17:25:02 [Info]   products (rt) [512B]...
2022-10-04 17:25:02 [Info]    OK
2022-10-04 17:25:02 [Info] Running sync
2022-10-04 17:25:06 [Info]  OK
2022-10-04 17:25:06 [Info] You can find backup here: /mnt/backup/backup-20221004172502
2022-10-04 17:25:06 [Info] Elapsed time: 4.82s
2022-10-04 17:25:06 [Info] Done
```
<!-- end -->

### 参数

| 参数                               | 描述                                                         |
| ---------------------------------- | ------------------------------------------------------------ |
| `--backup-dir=path`                | 备份目录的路径，备份将存储在此目录中。该目录必须已经存在。此参数是必需的，没有默认值。每次备份运行时，manticore-backup 将在提供的目录中创建一个带有时间戳名称的子目录（`backup-[datetime]`），并将所有所需的表复制到其中。因此，`--backup-dir` 是您所有备份的容器，可以安全地多次运行该脚本。 |
| `--restore[=backup]`               | 从 `--backup-dir` 恢复。仅使用 --restore 列出可用的备份。`--restore=backup` 将从 `<--backup-dir>/backup` 恢复。 |
| `--force`                          | 跳过恢复时的版本检查，优雅地恢复备份。                       |
| `--disable-telemetry`              | 如果您希望禁用向 Manticore 发送匿名指标，请传递此标志。您也可以使用环境变量 TELEMETRY=0 |
| `--config=/path/to/manticore.conf` | Manticore 配置的路径。可选。如果未提供，将使用适合您操作系统的默认配置。用于确定与 Manticore 守护进程的通信的主机和端口。`manticore-backup` 工具支持 [动态配置](../Server_settings/Scripted_configuration.md) 文件。如果您的配置分布在多个文件中，可以多次指定 `--config` 选项。 |
| `--tables=tbl1,tbl2,...`           | 要备份的表的以逗号分隔的列表。要备份所有表，请省略此参数。所有提供的表必须存在于您要备份的 Manticore 实例中，否则备份将失败。 |
| `--compress`                       | 是否压缩备份的文件。默认情况下未启用。                       |
| `--unlock`                         | 在某些情况下，当出现问题时，表可能会处于锁定状态。使用此参数解锁它们。 |
| `--version`                        | 显示当前版本。                                               |
| `--help`                           | 显示此帮助信息。                                             |

## 备份 SQL 命令参考

您还可以通过运行简单命令 `BACKUP TO /path/to/backup` 进行数据备份。

> 注意：`BACKUP` 在 Windows 上不受支持。请考虑使用 [mysqldump](../Securing_and_compacting_a_table/Backup_and_restore.md#使用-mysqldump-进行备份和恢复) 替代。

> 注意：`BACKUP` 需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果它无法工作，请确保 Buddy 已安装。

### BACKUP 的通用语法

```sql
BACKUP
  [{TABLE | TABLES} a[, b]]
  [{OPTION | OPTIONS}
    async = {on | off | 1 | 0 | true | false | yes | no}
    [, compress = {on | off | 1 | 0 | true | false | yes | no}]
  ]
  TO path_to_backup
```

例如，要将表 `a` 和 `b` 备份到 `/backup` 目录，请运行以下命令：

```sql
BACKUP TABLES a, b TO /backup
```

有一些选项可用于控制和调整备份过程，例如：

* `async`：使备份非阻塞，允许您立即接收带有查询 ID 的响应，并在备份进行时运行其他查询。默认值为 `0`。
* `compress`：启用使用 zstd 进行文件压缩。默认值为 `0`。

```sql
BACKUP OPTION async = yes, compress = yes TO /tmp
```

### 重要注意事项

1. 路径中不应包含特殊符号或空格，因为不支持它们。
2. 确保 Manticore Buddy 已启动（默认情况下是）。

### 备份如何维护表的一致性

为了确保备份期间表的一致性，Manticore Search 的备份工具使用创新的 [FREEZE 和 UNFREEZE](../Securing_and_compacting_a_table/Freezing_a_table.md) 命令。与例如 MySQL 的传统锁定和解锁表的功能不同，`FREEZE` 在停止将数据刷新到磁盘的同时仍允许写入（在某种程度上）和从表中选择更新的数据。

然而，如果在涉及大量插入的漫长备份操作期间，您的 RAM 块大小超过 `rt_mem_limit` 阈值，则数据可能会刷新到磁盘，写入操作将在刷新完成之前被阻止。尽管如此，该工具在冻结表期间保持表锁定、数据一致性和数据库写入可用性之间的平衡。

当您使用 `manticore-backup` 或 SQL `BACKUP` 命令时，`FREEZE` 命令会被执行一次，并同时冻结您要备份的所有表。备份过程随后逐个备份每个表，成功备份每个表后释放冻结状态。

如果备份失败或中断，工具会尝试解冻所有表。

## 使用 manticore-backup 工具恢复

<!-- example restore_list -->
要从备份中恢复 Manticore 实例，请使用 `manticore-backup` 命令，并带上 `--backup-dir` 和 `--restore` 参数。例如：`manticore-backup --backup-dir=/path/to/backups --restore`。如果不提供 `--restore` 的任何参数，它将简单列出 `--backup-dir` 中的所有备份。

<!-- request Example -->
```bash
manticore-backup --backup-dir=/mnt/backup/ --restore
```

<!-- response Example -->

```bash
Copyright (c) 2023-2024, Manticore Software LTD (https://manticoresearch.com)

Manticore config file:
Backup dir: /tmp/

Available backups: 3
  backup-20221006144635 (Oct 06 2022 14:46:35)
  backup-20221006145233 (Oct 06 2022 14:52:33)
  backup-20221007104044 (Oct 07 2022 10:40:44)
```

<!-- end -->

<!-- example restore -->

要启动恢复作业，运行 `manticore-backup`，并带上 `--restore=备份名称` 参数，其中 `备份名称` 是 `--backup-dir` 中备份目录的名称。请注意：

1. 不能有任何 Manticore 实例在同一主机和端口上运行。
2. 旧的 `manticore.json` 文件不得存在。
3. 旧的配置文件不得存在。
4. 旧的数据目录必须存在且为空。

如果所有条件都满足，恢复将继续进行。该工具将提供提示，因此您无需记住这些条件。避免覆盖现有文件非常重要，因此请确保在恢复之前删除它们（如果它们仍然存在）。

<!-- request Example -->
```bash
manticore-backup --backup-dir=/mnt/backup/ --restore=backup-20221007104044
```

<!-- response Example -->

```bash
Copyright (c) 2023-2024, Manticore Software LTD (https://manticoresearch.com)

Manticore config file:
Backup dir: /tmp/
2022-10-07 11:17:25 [Info] Starting to restore...

Manticore config
  endpoint =  127.0.0.1:9308
2022-10-07 11:17:25 [Info] Restoring config files...
2022-10-07 11:17:25 [Info]   config files - OK
2022-10-07 11:17:25 [Info] Restoring state files...
2022-10-07 11:17:25 [Info]   config files - OK
2022-10-07 11:17:25 [Info] Restoring data files...
2022-10-07 11:17:25 [Info]   config files - OK
2022-10-07 11:17:25 [Info] The backup '/tmp/backup-20221007104044' was successfully restored.
2022-10-07 11:17:25 [Info] Elapsed time: 0.02s
2022-10-07 11:17:25 [Info] Done
```

<!-- end -->

## 使用 mysqldump 进行备份和恢复

<!-- example mysqldump_backup -->

要创建 Manticore Search 数据库的备份，可以使用 `mysqldump` 命令。我们将在示例中使用默认的端口和主机。

请注意，`mysqldump` 仅支持实时表。

<!-- request Example -->
```bash
mysqldump -h0 -P9306 manticore > manticore_backup.sql
mariadb-dump -h0 -P9306 manticore > manticore_backup.sql
```

执行此命令将生成一个名为 `manticore_backup.sql` 的备份文件。该文件将保存所有数据和表模式。

<!-- request Example 2 -->
```bash
mysqldump -h0 -P9306 --replace --net-buffer-length=16m -etc manticore tbl > tbl.sql
```

这将生成一个备份文件 `tbl.sql`，其中使用 `replace` 命令而不是 `insert`，并在每个批次中保留列名。文档将批量处理，大小上限为 16 兆字节。不会有 `drop`/`create table` 命令。这在更改标记化设置后进行全文重建时非常有用。
<!-- end -->

<!-- example mysqldump_restore -->
### 恢复

如果您希望从备份文件恢复 Manticore Search 数据库，mysql 客户端是您首选的工具。

请注意，如果您在 [Plain mode](../Read_this_first.md#实时模式-vs-普通模式) 下恢复，则不能直接删除和重新创建表。因此，您应该：

- 使用 `mysqldump` 并加上 `-t` 选项，排除备份中的 `CREATE TABLE` 语句。
- 在继续恢复之前手动 [TRUNCATE](../Emptying_a_table.md) 表。

<!-- request SQL -->
```bash
mysql -h0 -P9306 < manticore_backup.sql
mariadb -h0 -P9306 < manticore_backup.sql
```

此命令使您能够从 `manticore_backup.sql` 文件中恢复所有内容。
<!-- end -->

### 附加选项

以下是一些可以与 mysqldump 一起使用的设置，以调整您的备份：

- `-t` 跳过 `drop`/`create` 表命令。对在更改标记化设置后进行表的全文重建很有用。
- `--no-data`：此设置从备份中省略表数据，生成的备份文件仅包含表模式。
- `--ignore-table=[database_name].[table_name]`：此选项允许您在备份操作中跳过特定表。请注意，数据库名称必须是 `manticore`。
- `--replace` 执行 `replace` 而不是 `insert`。在更改标记化设置后，对表进行全文重建时非常有用。
- `--net-buffer-length=16M` 使批次最大为 16 兆字节，以加快恢复速度。
- `-e` 批量处理文档。对加快恢复速度有帮助。
- `-c` 保留列名。在更改表模式（例如，更改字段顺序）后，进行表的重建时非常有用。

有关设置的全面列表及其详细描述，请参阅 [官方 MySQL 文档](https://dev.mysql.com/doc/refman/8.0/en/mysqldump.html)。

### 注意事项

我们建议在计划备份所有数据库时明确指定 `manticore` 数据库，而不是使用 `--all-databases` 选项。

请注意，`mysqldump` 不支持备份分布式表。此外，它无法备份包含非存储字段的表（可以考虑使用 `manticore-backup` 或 `BACKUP` SQL 命令）。

<!-- proofread -->
