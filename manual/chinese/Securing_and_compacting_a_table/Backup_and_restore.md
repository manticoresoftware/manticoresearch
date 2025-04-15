# 备份和恢复

定期备份你的表对于在系统崩溃、硬件故障或数据损坏/丢失的情况下进行恢复至关重要。在升级到新版本的 Manticore Search 或运行 [ALTER TABLE](../Updating_table_schema_and_settings.md#Updating-table-schema-in-RT-mode) 之前，强烈建议进行备份。

数据库系统的备份可以通过两种独特的方式进行：逻辑备份和物理备份。这两种方法各有优缺点，可能会根据特定的数据库环境和需求而有所不同。在这里，我们将深入探讨这两种备份之间的区别。

### 逻辑备份

逻辑备份涉及将数据库架构和数据导出为 SQL 语句或特定于数据库的数据格式。这种备份形式通常是人可读的，可以用于在不同的系统或数据库引擎上恢复数据库。

逻辑备份的优缺点：
- ➕ **可移植性:** 逻辑备份通常比物理备份更具可移植性，因为它们可以用于在不同的硬件或操作系统上恢复数据库。
- ➕ **灵活性:** 逻辑备份允许您选择性地恢复特定的表、索引或其他数据库对象。
- ➕ **兼容性:** 逻辑备份可用于在不同的数据库管理系统或版本之间迁移数据，前提是目标系统支持导出的格式或 SQL 语句。
- ➖ **备份和恢复速度较慢:** 逻辑备份的速度可能比物理备份慢，因为它们需要数据库引擎将数据转换为 SQL 语句或其他导出格式。
- ➖ **系统负载增加:** 创建逻辑备份可能会导致系统负载增加，因为该过程需要更多的 CPU 和内存资源来处理和导出数据。

Manticore Search 支持 [mysqldump](../Securing_and_compacting_a_table/Backup_and_restore.md#Backup-and-restore-with-mysqldump) 进行逻辑备份。

### 物理备份

物理备份涉及复制构成数据库的原始数据文件和系统文件。这种类型的备份基本上在某一时间点创建了数据库物理状态的快照。

物理备份的优缺点：
- ➕ **速度:** 物理备份通常比逻辑备份更快，因为它们涉及直接从磁盘复制原始数据文件。
- ➕ **一致性:** 物理备份确保整个数据库的一致备份，因为所有相关文件一起复制。
- ➕ **较低的系统负载:** 创建物理备份通常对系统的负载较小，因为该过程不涉及额外的数据处理。
- ➖ **可移植性:** 物理备份通常比逻辑备份的可移植性差，因为它们可能依赖于特定的硬件、操作系统或数据库引擎配置。
- ➖ **灵活性:** 物理备份不允许选择性恢复特定的数据库对象，因为备份包含整个数据库的原始文件。
- ➖ **兼容性:** 物理备份不能用于在不同的数据库管理系统或版本之间迁移数据，因为原始数据文件可能在不同的平台或软件之间不兼容。

Manticore Search 有 [manticore-backup](../Securing_and_compacting_a_table/Backup_and_restore.md#Using-manticore-backup-command-line-tool) 命令行工具进行物理备份。

总之，逻辑备份提供了更多的灵活性、可移植性和兼容性，但可能更慢并且资源密集，而物理备份则更快、更一致且资源占用较低，但在可移植性和灵活性方面可能受限。选择这两种备份方法中的一种将取决于您的特定数据库环境、硬件和需求。

## 使用 manticore-backup 命令行工具

`manticore-backup` 工具包含在官方 Manticore Search [软件包](https://manticoresearch.com/install) 中，自动化了备份在 [RT 模式](../Read_this_first.md#Real-time-mode-vs-plain-mode) 下运行的实例的表的过程。

### 安装

**如果您遵循了 [官方安装说明](https://manticoresearch.com/install/)，您应该已经安装了所有内容，无需担心。** 否则，[`manticore-backup`](https://github.com/manticoresoftware/manticoresearch-backup) 需要 PHP 8.1.10 和 [特定模块](https://github.com/manticoresoftware/executor/blob/main/build-linux) 或 [`manticore-executor`](https://github.com/manticoresoftware/executor)，它是 `manticore-extra` 包的一部分，您需要确保其中一个可用。

请注意，`manticore-backup` 目前尚未在 Windows 上可用。

### 如何使用

首先，确保在与您即将备份的 Manticore 实例运行的同一服务器上运行 `manticore-backup`。

第二，我们建议在 `root` 用户下运行该工具，以便该工具可以转移您备份的文件的所有权。否则，也会进行备份，但不会进行所有权转移。在任何情况下，您都应该确保 `manticore-backup` 访问 Manticore 实例的数据目录。

<!-- example backup1 -->

`manticore-backup` 唯一必需的参数是 `--backup-dir`，此参数指定了备份的目标位置。如果您不提供任何额外的参数，`manticore-backup` 将：
- 定位一个使用默认配置运行的 Manticore 实例
- 在 `--backup-dir` 目录中创建一个带有时间戳名称的子目录
- 备份实例中找到的所有表

<!-- request Example -->
```bash
manticore-backup --config=path/to/manticore.conf --backup-dir=backupdir
```

<!-- response Example -->
```bash
Copyright (c) 2023-2024, Manticore Software LTD (https://manticoresearch.com)

Manticore 配置文件: /etc/manticoresearch/manticore.conf
Tables to backup: all tables
Target dir: /mnt/backup/

Manticore config
  endpoint =  127.0.0.1:9308

Manticore versions:
  manticore: 5.0.2
  columnar: 1.15.4
  secondary: 1.15.4
2022-10-04 17:18:39 [Info] 开始备份...
2022-10-04 17:18:39 [Info] 备份配置文件...
2022-10-04 17:18:39 [Info]   配置文件 - OK
2022-10-04 17:18:39 [Info] 备份表...
2022-10-04 17:18:39 [Info]   pq (percolate) [425B]...
2022-10-04 17:18:39 [Info]    OK
2022-10-04 17:18:39 [Info]   products (rt) [512B]...
2022-10-04 17:18:39 [Info]    OK
2022-10-04 17:18:39 [Info] 正在运行同步
2022-10-04 17:18:42 [Info]  OK
2022-10-04 17:18:42 [Info] 您可以在这里找到备份: /mnt/backup/backup-20221004171839
2022-10-04 17:18:42 [Info] 消耗时间: 2.76s
2022-10-04 17:18:42 [Info] 完成
```
<!-- end -->

<!-- example backup2 -->
To back up specific tables only, use the `--tables` flag followed by a comma-separated list of tables, for example `--tables=tbl1,tbl2`. This will only backup the specified tables and ignore the rest.

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
2022-10-04 17:25:02 [Info] 开始备份...
2022-10-04 17:25:02 [Info] 备份配置文件...
2022-10-04 17:25:02 [Info]   配置文件 - OK
2022-10-04 17:25:02 [Info] 备份表...
2022-10-04 17:25:02 [Info]   products (rt) [512B]...
2022-10-04 17:25:02 [Info]    OK
2022-10-04 17:25:02 [Info] 正在运行同步
2022-10-04 17:25:06 [Info]  OK
2022-10-04 17:25:06 [Info] 您可以在这里找到备份: /mnt/backup/backup-20221004172502
2022-10-04 17:25:06 [Info] 消耗时间: 4.82s
2022-10-04 17:25:06 [Info] 完成
```
<!-- end -->

### Arguments

| Argument | Description |
|-|-|
| `--backup-dir=path` | 这是备份目录的路径，备份将存储在此目录中。该目录必须已经存在。此参数是必需的，且没有默认值。在每次备份运行时，manticore-backup将在提供的目录中创建一个带有时间戳的子目录（`backup-[datetime]`），并将所有所需的表复制到其中。因此，`--backup-dir`是您所有备份的容器，可以安全多次运行此脚本。|
| `--restore[=backup]` | 从`--backup-dir`恢复。仅仅使用 --restore 列出可用备份。`--restore=backup` 将从 `<--backup-dir>/backup` 恢复。|
| `--force` | 跳过还原时的版本检查，并优雅地恢复备份。|
| `--disable-telemetry` | 如果您想禁用发送匿名指标到Manticore，请传递此标志。您还可以使用环境变量 TELEMETRY=0 |
| `--config=/path/to/manticore.conf` | Manticore 配置的路径。可选。如果没有提供，将使用针对您操作系统的默认配置。用于确定与Manticore守护进程的通信的主机和端口。`manticore-backup` 工具支持 [动态配置](../Server_settings/Scripted_configuration.md) 文件。如果您的配置分散在多个文件中，可以多次指定`--config`选项。|
| `--tables=tbl1,tbl2, ...` | 要备份的表的以分号分隔的列表。要备份所有表，请省略此参数。所有提供的表必须存在于您正在备份的Manticore实例中，否则备份将失败。|
| `--compress` | 备份文件是否应进行压缩。默认情况下未启用。 | optional |
| `--unlock` | 在某些罕见情况下，当出现问题时，表可能会保持锁定状态。使用此参数解锁它们。|
| `--version` | 显示当前版本。|
| `--help` | 显示此帮助信息。|

## BACKUP SQL command reference

You can also back up your data through SQL by running the simple command `BACKUP TO /path/to/backup`.

> NOTE: `BACKUP` is not supported in Windows. Consider using [mysqldump](../Securing_and_compacting_a_table/Backup_and_restore.md#Backup-and-restore-with-mysqldump) instead.

> NOTE: `BACKUP` requires [Manticore Buddy](../Installation/Manticore_Buddy.md). If it doesn't work, make sure Buddy is installed.

### General syntax of BACKUP

```sql
BACKUP
  [{TABLE | TABLES} a[, b]]
  [{OPTION | OPTIONS}
    async = {on | off | 1 | 0 | true | false | yes | no}
    [, compress = {on | off | 1 | 0 | true | false | yes | no}]
  ]
  TO path_to_backup
```

For instance, to back up tables `a` and `b` to the `/backup` directory, run the following command:

```sql
BACKUP TABLES a, b TO /backup
```

There are options available to control and adjust the backup process, such as:

* `async`: makes the backup non-blocking, allowing you to receive a response with the query ID immediately and run other queries while the backup is ongoing. The default value is `0`.
* `compress`: enables file compression using zstd. The default value is `0`.
For example, to run a backup of all tables in async mode with compression enabled to the `/tmp` directory:

```sql
BACKUP OPTION async = yes, compress = yes TO /tmp
```

### Important considerations

1. The path should not contain special symbols or spaces, as they are not supported.
2. Ensure that Manticore Buddy is launched (it is by default).

### How backup maintains consistency of tables
要确保备份期间表的一致性，Manticore Search 的备份工具使用创新的 [FREEZE 和 UNFREEZE](../Securing_and_compacting_a_table/Freezing_a_table.md) 命令。与传统的 MySQL 等锁定和解锁表功能不同，`FREEZE` 会停止将数据刷新到磁盘，同时仍然允许写入（在一定程度上）和选择更新的表数据。

然而，如果在涉及大量插入的长时间备份操作中，RAM 块大小超过 `rt_mem_limit` 阈值，数据可能会被刷新到磁盘，并且写入操作将被阻塞，直到刷新完成。尽管如此，该工具在表锁定、数据一致性和数据库写入可用性之间保持平衡，同时表处于冻结状态。

当你使用 `manticore-backup` 或 SQL `BACKUP` 命令时，`FREEZE` 命令会被执行一次，并同时冻结所有正在备份的表。备份过程随后逐个备份每个表，并在成功备份每个表后释放冻结。

如果备份失败或被中断，该工具会尝试解冻所有表。

## 使用 manticore-backup 工具还原

<!-- example restore_list -->
要从备份还原 Manticore 实例，请使用带有 `--backup-dir` 和 `--restore` 参数的 `manticore-backup` 命令。例如：`manticore-backup --backup-dir=/path/to/backups --restore`。如果不为 `--restore` 提供任何参数，它将简单地列出 `--backup-dir` 中的所有备份。

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

要开始还原作业，请使用 `--restore=备份名称` 标志运行 `manticore-backup`，其中 `备份名称` 是 `--backup-dir` 中的备份目录名称。请注意：
1. 在要还原的主机和端口上不能运行任何 Manticore 实例。
2. 旧的 `manticore.json` 文件不能存在。
3. 旧的配置文件不能存在。
4. 旧的数据目录必须存在且为空。

如果满足所有条件，还原将继续进行。该工具将提供提示，因此你不必记住它们。为避免覆盖现有文件，请确保在还原之前删除仍然存在的文件。因此有了所有这些条件。

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

## 使用 mysqldump 进行备份和还原

<!-- example mysqldump_backup -->

> 注意：某些版本的 `mysqldump` / `mariadb-dump` 需要 [Manticore Buddy](../../Installation/Manticore_Buddy.md)。如果转储不起作用，请确保已安装 Buddy。

要创建 Manticore Search 数据库的备份，你可以使用 `mysqldump` 命令。我们将在示例中使用默认端口和主机。

请注意，`mysqldump` 仅支持实时表。

<!-- request Basic -->
```bash
mysqldump -h0 -P9306 manticore > manticore_backup.sql
mariadb-dump -h0 -P9306 manticore > manticore_backup.sql
```

执行此命令将生成一个名为 `manticore_backup.sql` 的备份文件。该文件将包含所有数据和表架构。

<!-- request Replace mode -->
```bash
mysqldump -h0 -P9306 --replace --net-buffer-length=16m -etc manticore tbl > tbl.sql
```

这将生成一个 `tbl.sql` 备份文件，其中包含 `replace` 命令而不是 `insert`，每批中保留列名。文档将被批处理到最大 16 兆字节。不会有 `drop`/`create table` 命令。这对于在更改分词设置后进行全文重新索引很有用。

<!-- request Replication mode -->
```bash
mysqldump -etc --replace -h0 -P9306 -ucluster manticore cluster:tbl | mysql -P9306 -h0
mariadb-dump -etc --replace -h0 -P9306 -ucluster manticore cluster:tbl | mysql -P9306 -h0
```

在这种情况下，`mysqldump` 将生成类似 `REPLACE INTO cluster:table ...` 的命令，这些命令将直接发送到 Manticore 实例，从而重新插入文档。
使用 `cluster` 用户和 `-t` 标志以启用复制模式。请参阅下面的注释中的详细信息。

<!-- end -->

<!-- example mysqldump_restore -->
### 还原

如果你想从备份文件还原 Manticore Search 数据库，mysql 客户端是你的首选工具。

请注意，如果你在 [Plain 模式](../Read_this_first.md#Real-time-mode-vs-plain-mode)下还原，则无法直接删除和重新创建表。因此，你应该：
- 使用带有 `-t` 选项的 `mysqldump` 来排除备份中的 `CREATE TABLE` 语句。
- 在继续还原之前手动 [TRUNCATE](../Emptying_a_table.md) 表。

<!-- request SQL -->
```bash
mysql -h0 -P9306 < manticore_backup.sql
mariadb -h0 -P9306 < manticore_backup.sql
```

此命令允许您从 `manticore_backup.sql` 文件中恢复所有内容。
<!-- end -->

### 其他选项

这里有一些可以与 mysqldump 一起使用以定制您的备份的更多设置：

- `-t` 跳过 `drop` / `create` 表命令。对于在更改标记设置后对表进行全文重建是有用的。
- `--no-data`：此设置省略表数据，导致备份文件仅包含表模式。
- `--ignore-table=[database_name].[table_name]`：此选项允许您在备份操作中跳过特定表。请注意，数据库名称必须是 `manticore`。
- `--replace` 执行 `replace` 而不是 `insert`。对于在更改标记设置后对表进行全文重建是有用的。
- `--net-buffer-length=16M` 使批处理大小达到 16 兆字节，以加快恢复速度。
- `-e` 用于批量处理文档。对于更快的恢复是有用的。
- `-c` 保留列名。对于在更改其模式（例如，改变字段顺序）后对表进行重建是有用的。

要获取设置和其详细描述的综合列表，请参考官方 [MySQL 文档](https://dev.mysql.com/doc/refman/8.0/en/mysqldump.html) 或 [MariaDB 文档](https://mariadb.com/kb/en/mariadb-dump/)。

### 注意事项

* 要以复制模式创建转储（转储包含 `INSERT/REPLACE INTO <cluster_name>:<table_name>`）：
  - 使用 `cluster` 用户。例如：`mysqldump -u cluster ...` 或 `mariadb-dump -u cluster ...`。您可以通过运行 `SET GLOBAL cluster_user = new_name` 更改启用 `mysqldump` 复制模式的用户名。
  - 使用 `-t` 标志。
  - 在复制模式中指定表时，您需要遵循 `cluster_name:table_name` 语法。例如：`mysqldump -P9306 -h0 -t -ucluster manticore cluster:tbl`。
* 建议在计划备份所有数据库时明确指定 `manticore` 数据库，而不是使用 `--all-databases` 选项。
* 请注意，`mysqldump` 不支持备份分布式表，且无法备份包含非存储字段的表。对于这种情况，请考虑使用 `manticore-backup` 或 `BACKUP` SQL 命令。如果您有分布式表，建议始终指定要转储的表。

<!-- proofread -->

