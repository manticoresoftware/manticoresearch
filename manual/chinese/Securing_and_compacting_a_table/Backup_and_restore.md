# 备份与恢复

定期备份您的表对于在系统崩溃、硬件故障或数据损坏/丢失时进行恢复至关重要。强烈建议在升级到新的 Manticore Search 版本或运行 [ALTER TABLE](../Updating_table_schema_and_settings.md#Updating-table-schema-in-RT-mode) 之前进行备份。

数据库系统的备份可以通过两种独特的方式完成：逻辑备份和物理备份。这两种方法各有优缺点，具体取决于特定的数据库环境和需求。这里，我们将深入探讨这两种备份类型的区别。

### 逻辑备份

逻辑备份包括将数据库模式和数据导出为 SQL 语句或数据库特定的数据格式。这种备份形式通常是人类可读的，并且可以用于在不同系统或数据库引擎上恢复数据库。

逻辑备份的优缺点：
- ➕ **可移植性：** 逻辑备份通常比物理备份更具可移植性，因为它们可以用于在不同硬件或操作系统上恢复数据库。
- ➕ **灵活性：** 逻辑备份允许您选择性地恢复特定的表、索引或其他数据库对象。
- ➕ **兼容性：** 逻辑备份可用于在不同的数据库管理系统或版本之间迁移数据，前提是目标系统支持导出的格式或 SQL 语句。
- ➖ **备份和恢复速度较慢：** 逻辑备份可能比物理备份慢，因为它们需要数据库引擎将数据转换为 SQL 语句或其他导出格式。
- ➖ **系统负载增加：** 创建逻辑备份可能导致更高的系统负载，因为该过程需要更多的 CPU 和内存资源来处理和导出数据。

Manticore Search 支持使用 [mysqldump](../Securing_and_compacting_a_table/Backup_and_restore.md#Backup-and-restore-with-mysqldump) 进行逻辑备份。

### 物理备份

物理备份涉及复制构成数据库的原始数据文件和系统文件。这种备份类型本质上是在某一时间点创建数据库物理状态的快照。

物理备份的优缺点：
- ➕ **速度：** 物理备份通常比逻辑备份更快，因为它们直接从磁盘复制原始数据文件。
- ➕ **一致性：** 物理备份确保整个数据库的一致备份，因为所有相关文件都会一起复制。
- ➕ **系统负载较低：** 与逻辑备份相比，创建物理备份通常对系统的负载较小，因为该过程不涉及额外的数据处理。
- ➖ **可移植性：** 物理备份通常比逻辑备份的可移植性差，因为它们可能依赖于特定的硬件、操作系统或数据库引擎配置。
- ➖ **灵活性：** 物理备份不允许选择性地恢复特定的数据库对象，因为备份包含整个数据库的原始文件。
- ➖ **兼容性：** 物理备份不能用于在不同的数据库管理系统或版本之间迁移数据，因为原始数据文件可能在不同平台或软件之间不兼容。

Manticore Search 提供了用于物理备份的 [manticore-backup](../Securing_and_compacting_a_table/Backup_and_restore.md#Using-manticore-backup-command-line-tool) 命令行工具。

总之，逻辑备份提供了更多的灵活性、可移植性和兼容性，但可能较慢且资源消耗较大；而物理备份速度更快、更一致且资源消耗较少，但在可移植性和灵活性方面可能有限。选择哪种备份方法取决于您的具体数据库环境、硬件和需求。

## 使用 manticore-backup 命令行工具

`manticore-backup` 工具包含在官方 Manticore Search [软件包](https://manticoresearch.com/install)中，自动化了在 [RT 模式](../Read_this_first.md#Real-time-mode-vs-plain-mode) 下运行的实例的表备份过程。

### 安装

**如果您遵循了[官方安装说明](https://manticoresearch.com/install/)，您应该已经安装好所有内容，无需担心。** 否则，[`manticore-backup`](https://github.com/manticoresoftware/manticoresearch-backup) 需要 PHP 8.1.10 和 [特定模块](https://github.com/manticoresoftware/executor/blob/main/build-linux) 或 [`manticore-executor`](https://github.com/manticoresoftware/executor)，后者是 `manticore-extra` 软件包的一部分，您需要确保其中之一可用。

请注意，`manticore-backup` 目前尚不支持 Windows。

### 使用方法

首先，确保您在运行要备份的 Manticore 实例的同一服务器上运行 `manticore-backup`。

其次，建议以 `root` 用户身份运行该工具，以便工具可以转移您备份文件的所有权。否则，备份仍会完成，但不会转移所有权。无论哪种情况，您都应确保 `manticore-backup` 有权访问 Manticore 实例的数据目录。

<!-- example backup1 -->

`manticore-backup` 唯一必需的参数是 `--backup-dir`，它指定备份的目标位置。如果您不提供任何其他参数，`manticore-backup` 将：
- 定位使用默认配置运行的 Manticore 实例
- 在 `--backup-dir` 目录下创建一个带时间戳的子目录
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
要仅备份特定表，请使用 `--tables` 标志，后跟以逗号分隔的表名列表，例如 `--tables=tbl1,tbl2`。这将只备份指定的表，忽略其余表。

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

| 参数 | 描述 |
|-|-|
| `--backup-dir=path` | 这是备份目录的路径，备份将存储在该目录中。该目录必须已存在。此参数是必需的且没有默认值。每次备份运行时，manticore-backup 会在提供的目录中创建一个带有时间戳的子目录（`backup-[datetime]`），并将所有所需的表复制到该子目录中。因此，`--backup-dir` 是所有备份的容器，运行脚本多次是安全的。|
| `--restore[=backup]` | 从 `--backup-dir` 恢复。仅使用 --restore 会列出可用的备份。`--restore=backup` 将从 `<--backup-dir>/backup` 恢复。 |
| `--force` | 在恢复时跳过版本检查并优雅地恢复备份。 |
| `--disable-telemetry` | 如果您想禁用向 Manticore 发送匿名指标，请传递此标志。您也可以使用环境变量 TELEMETRY=0 |
| `--config=/path/to/manticore.conf` | Manticore 配置的路径。可选。如果未提供，将使用适合您操作系统的默认配置。用于确定与 Manticore 守护进程通信的主机和端口。`manticore-backup` 工具支持[动态配置](../Server_settings/Scripted_configuration.md)文件。如果您的配置分布在多个文件中，可以多次指定 `--config` 选项。 |
| `--tables=tbl1,tbl2, ...` | 您想备份的表的分号分隔列表。要备份所有表，请省略此参数。所有提供的表必须存在于您备份的 Manticore 实例中，否则备份将失败。 |
| `--compress` | 是否对备份的文件进行压缩。默认未启用。| 可选 |
| `--unlock` | 在极少数情况下，当出现问题时，表可能会处于锁定状态。使用此参数解锁它们。 |
| `--version` | 显示当前版本。 |
| `--help` | 显示此帮助信息。 |

## BACKUP SQL 命令参考

您也可以通过运行简单命令 `BACKUP TO /path/to/backup` 来备份数据。

> 注意：`BACKUP` 不支持 Windows。请考虑使用 [mysqldump](../Securing_and_compacting_a_table/Backup_and_restore.md#Backup-and-restore-with-mysqldump) 替代。

> 注意：`BACKUP` 需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果无法使用，请确保 Buddy 已安装。

### BACKUP 的通用语法

```sql
BACKUP
  [{TABLE | TABLES} a[, b]]
  TO path_to_backup
  [{OPTION | OPTIONS}
    async = {on | off | 1 | 0 | true | false | yes | no}
    [, compress = {on | off | 1 | 0 | true | false | yes | no}]
  ]
```

例如，要将表 `a` 和 `b` 备份到 `/backup` 目录，运行以下命令：

```sql
BACKUP TABLES a, b TO /backup
```

有一些选项可用于控制和调整备份过程，例如：

* `async`：使备份非阻塞，允许您立即收到带有备份路径的响应，并在备份进行时运行其他查询。默认值为 `0`。
* `compress`：启用使用 zstd 的文件压缩。默认值为 `0`。

例如，要以异步模式运行所有表的备份，并启用压缩到 `/tmp` 目录：

```sql
BACKUP TO /tmp OPTION async = yes, compress = yes
```

### 异步备份行为

使用 `async = 1`（或 `yes`、`on`、`true`）时，备份操作在后台任务中运行：

* 命令立即返回备份路径
* 您可以在备份进行时继续运行其他查询
* 备份任务在由 Manticore Buddy 管理的单独线程中运行
* 运行时，备份任务会出现在 `SHOW QUERIES` 输出中，完成后会自动移除

**异步备份示例：**

```sql
BACKUP TO /tmp/mybackup OPTION async = 1
```

这将立即返回类似如下的输出：
```
+----------------------------------+
| Path                             |
+----------------------------------+
| /tmp/mybackup/backup-20221004... |
+----------------------------------+
```

您可以使用 `SHOW QUERIES` 检查备份是否仍在运行。完成后，任务将从查询列表中消失，所有备份文件将存在于指定目录中。

### 重要注意事项

1. 如果路径中包含空格，可以用单引号括起来，例如 `BACKUP TO '/path/with spaces'`
2. 不含空格的路径无需引号：`BACKUP TO /tmp/backup`
3. 支持 Windows 路径：`BACKUP TO 'C:\path'` 或 `BACKUP TO C:\windows\backup`
4. 确保已启动 Manticore Buddy（默认启动）
5. 备份目录必须存在且 Manticore 进程有写权限

### 备份如何保持表的一致性

为了确保备份期间表的一致性，Manticore Search 的备份工具使用了创新的 [FREEZE 和 UNFREEZE](../Securing_and_compacting_a_table/Freezing_a_table.md) 命令。与例如 MySQL 的传统锁定和解锁表功能不同，`FREEZE` 会停止将数据刷新到磁盘，同时仍允许一定程度的写入和从表中选择更新的数据。

然而，如果在涉及大量插入的长时间备份操作中，您的 RAM 块大小超过了 `rt_mem_limit` 阈值，数据可能会被刷新到磁盘，写操作将被阻塞直到刷新完成。尽管如此，该工具在表冻结期间保持了表锁定、数据一致性和数据库写入可用性之间的平衡。

当您使用 `manticore-backup` 或 SQL `BACKUP` 命令时，`FREEZE` 命令会执行一次，并同时冻结所有您正在备份的表。备份过程随后逐个备份每个表，成功备份后释放冻结。

如果备份失败或中断，工具会尝试解冻所有表。

## 使用 manticore-backup 工具恢复

<!-- example restore_list -->
要从备份恢复 Manticore 实例，请使用带有 `--backup-dir` 和 `--restore` 参数的 `manticore-backup` 命令。例如：`manticore-backup --backup-dir=/path/to/backups --restore`。如果您未为 `--restore` 提供任何参数，它将仅列出 `--backup-dir` 中的所有备份。

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

要开始恢复任务，运行带有 `--restore=backup name` 标志的 `manticore-backup`，其中 `backup name` 是 `--backup-dir` 中备份目录的名称。请注意：
1. 恢复时，不能有任何 Manticore 实例在相同的主机和端口上运行。
2. 旧的 `manticore.json` 文件必须不存在。
3. 旧的配置文件必须不存在。
4. 旧的数据目录必须存在且为空。

如果满足所有条件，恢复将继续。该工具会提供提示，因此您无需记住它们。避免覆盖现有文件至关重要，因此如果它们仍然存在，请确保在恢复之前将其删除。因此需要满足所有条件。

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

Manticore 支持来自 MySQL 9.5 及以下版本的 `mysqldump` 工具和来自 MariaDB 12.0 及以下版本的 `mariadb-dump` 工具。

<!-- example mysqldump_backup -->

> 注意：某些版本的 `mysqldump` / `mariadb-dump` 需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果转储无法工作，请确保已安装 Buddy。

要创建 Manticore Search 数据库的备份，可以使用 `mysqldump` 命令。示例中将使用默认端口和主机。

注意，`mysqldump` 仅支持实时表。

<!-- request Basic -->
```bash
mysqldump -h0 -P9306 manticore > manticore_backup.sql
mariadb-dump -h0 -P9306 manticore > manticore_backup.sql
```

执行此命令将生成名为 `manticore_backup.sql` 的备份文件。该文件将包含所有数据和表结构。

<!-- request Replace mode -->
```bash
mysqldump -h0 -P9306 --replace --net-buffer-length=16m -etc manticore tbl > tbl.sql
```

这将生成一个备份文件 `tbl.sql`，其中使用 `replace` 命令替代 `insert`，每批次保留列名。文档将批量处理，最大大小为16兆字节。不会有 `drop`/`create table` 命令。这对于更改分词设置后进行全文重新索引非常有用。

<!-- request Replication mode -->
```bash
mysqldump -etc --replace -h0 -P9306 -ucluster manticore cluster:tbl | mysql -P9306 -h0
mariadb-dump -etc --replace -h0 -P9306 -ucluster manticore cluster:tbl | mysql -P9306 -h0
```

在这种情况下，`mysqldump` 将生成类似 `REPLACE INTO cluster:table ...` 的命令，这些命令将直接发送到 Manticore 实例，从而重新插入文档。
使用 `cluster` 用户和 `-t` 标志启用复制模式。详情见下方说明。

<!-- end -->

<!-- example mysqldump_restore -->
### 恢复

如果您想从备份文件恢复 Manticore Search 数据库，mysql 客户端是您的首选工具。

注意，如果您在[纯模式](../Read_this_first.md#Real-time-mode-vs-plain-mode)下恢复，不能直接删除并重建表。因此，您应当：
- 使用带有 `-t` 选项的 `mysqldump`，以排除备份中的 `CREATE TABLE` 语句。
- 在继续恢复之前，手动[清空](../Emptying_a_table.md)表。

<!-- request SQL -->
```bash
mysql -h0 -P9306 < manticore_backup.sql
mariadb -h0 -P9306 < manticore_backup.sql
```

此命令允许您从 `manticore_backup.sql` 文件恢复所有内容。
<!-- end -->

### 其他选项

以下是一些可与 mysqldump 一起使用以定制备份的设置：

- `-t` 跳过 `drop`/`create` 表命令。适用于更改分词设置后对表进行全文重新索引。
- `--no-data`：此设置省略表数据，仅备份表结构。
- `--ignore-table=[database_name].[table_name]`：此选项允许您在备份操作中跳过特定表。注意数据库名必须是 `manticore`。
- `--replace` 使用 `replace` 替代 `insert`。适用于更改分词设置后对表进行全文重新索引。
- `--net-buffer-length=16M` 使批处理最大为16兆字节，以加快恢复速度。
- `-e` 批量处理文档。适用于更快恢复。
- `-c` 保留列名。适用于更改表结构（例如更改字段顺序）后重新索引表。

有关设置的完整列表及详细说明，请参阅官方 [MySQL 文档](https://dev.mysql.com/doc/refman/8.0/en/mysqldump.html) 或 [MariaDB 文档](https://mariadb.com/kb/en/mariadb-dump/)。

### 说明

* 要创建复制模式的转储（转储包含 `INSERT/REPLACE INTO <cluster_name>:<table_name>`）：
  - 使用 `cluster` 用户。例如：`mysqldump -u cluster ...` 或 `mariadb-dump -u cluster ...`。您可以通过运行 `SET GLOBAL cluster_user = new_name` 来更改启用 mysqldump 复制模式的用户名。
  - 使用 `-t` 标志。
  - 在复制模式下指定表时，需要遵循 `cluster_name:table_name` 语法。例如：`mysqldump -P9306 -h0 -t -ucluster manticore cluster:tbl`。
* 建议在计划备份所有数据库时明确指定 `manticore` 数据库，而不是使用 `--all-databases` 选项。
* 注意，`mysqldump` 不支持备份分布式表，也无法备份包含非存储字段的表。对于这种情况，请考虑使用 `manticore-backup` 或 `BACKUP` SQL 命令。如果您有分布式表，建议始终指定要转储的表。

<!-- proofread -->
