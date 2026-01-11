# 备份与恢复

定期备份您的表对于在系统崩溃、硬件故障或数据损坏/丢失时进行恢复至关重要。强烈建议在升级到新的 Manticore Search 版本或运行 [ALTER TABLE](../Updating_table_schema_and_settings.md#Updating-table-schema-in-RT-mode) 前进行备份。

数据库系统备份有两种独特的方法：逻辑备份和物理备份。这两种方法各有优缺点，具体取决于特定的数据库环境和需求。这里，我们将深入探讨这两种备份类型的区别。

### 逻辑备份

逻辑备份涉及将数据库模式和数据导出为 SQL 语句或数据库特定的数据格式。这种备份形式通常对人类可读，并且可用于在不同系统或数据库引擎上恢复数据库。

逻辑备份的优缺点：
- ➕ **可移植性：** 逻辑备份通常比物理备份更具可移植性，因为它们可以用于在不同硬件或操作系统上恢复数据库。
- ➕ **灵活性：** 逻辑备份允许您选择性地恢复特定的表、索引或其他数据库对象。
- ➕ **兼容性：** 逻辑备份可用于在不同数据库管理系统或版本之间迁移数据，只要目标系统支持导出格式或 SQL 语句。
- ➖ **备份与恢复速度较慢：** 逻辑备份通常比物理备份慢，因为需要数据库引擎将数据转换为 SQL 语句或其他导出格式。
- ➖ **系统负载增加：** 创建逻辑备份可能导致系统负载较高，因为该过程需要更多的 CPU 和内存资源来处理和导出数据。

Manticore Search 支持使用 [mysqldump](../Securing_and_compacting_a_table/Backup_and_restore.md#Backup-and-restore-with-mysqldump) 进行逻辑备份。

### 物理备份

物理备份涉及复制构成数据库的原始数据文件和系统文件。这种备份类型本质上是创建数据库在某一时间点的物理状态快照。

物理备份的优缺点：
- ➕ **速度：** 物理备份通常比逻辑备份快，因为它们直接从磁盘复制原始数据文件。
- ➕ **一致性：** 物理备份确保整个数据库的一致性备份，因为所有相关文件都会被一起复制。
- ➕ **系统负载较低：** 与逻辑备份相比，创建物理备份通常对系统负载较小，因为过程不涉及额外的数据处理。
- ➖ **可移植性：** 物理备份通常比逻辑备份的可移植性差，因为它们可能依赖于特定的硬件、操作系统或数据库引擎配置。
- ➖ **灵活性：** 物理备份不允许选择性恢复特定数据库对象，因为备份包含整个数据库的原始文件。
- ➖ **兼容性：** 物理备份不能用于在不同数据库管理系统或版本之间迁移数据，因为原始数据文件在不同平台或软件间可能不兼容。

Manticore Search 提供了用于物理备份的命令行工具 [manticore-backup](../Securing_and_compacting_a_table/Backup_and_restore.md#Using-manticore-backup-command-line-tool)。

总之，逻辑备份提供了更多的灵活性、可移植性和兼容性，但可能较慢且资源消耗较高；物理备份更快、更一致且资源消耗较低，但在可移植性和灵活性方面可能受限。您可以根据具体的数据库环境、硬件和需求选择合适的备份方法。

## 使用 manticore-backup 命令行工具

`manticore-backup` 工具包含在官方 Manticore Search [安装包](https://manticoresearch.com/install)中，可自动备份以 [RT 模式](../Read_this_first.md#Real-time-mode-vs-plain-mode) 运行的实例中的表。

### 安装

**如果您按照了[官方安装说明](https://manticoresearch.com/install/)，应该已经安装完成，无需额外操作。** 否则， [`manticore-backup`](https://github.com/manticoresoftware/manticoresearch-backup) 需要 PHP 8.1.10 和 [特定模块](https://github.com/manticoresoftware/executor/blob/main/build-linux) 或 [`manticore-executor`](https://github.com/manticoresoftware/executor)，后者是 `manticore-extra` 包的一部分，您需要确保其中之一可用。

注意，目前 `manticore-backup` 尚不支持 Windows。

### 使用方法

首先，确保您在运行要备份的 Manticore 实例的同一服务器上运行 `manticore-backup`。

其次，建议以 `root` 用户运行此工具，以便工具能转移您备份文件的所有权。否则，将生成备份但不进行所有权转移。无论哪种情况，都应确保 `manticore-backup` 有权访问该 Manticore 实例的数据目录。

<!-- example backup1 -->

`manticore-backup` 唯一必需的参数是 `--backup-dir`，用于指定备份的目标位置。如果不提供其他参数，`manticore-backup` 将：
- 定位一个使用默认配置运行的 Manticore 实例
- 在 `--backup-dir` 目录下创建一个带时间戳的子目录
- 备份该实例中的所有表

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
要仅备份特定表，请使用 `--tables` 标志，后跟逗号分隔的表名列表，例如 `--tables=tbl1,tbl2`。这只备份指定的表，忽略其余表。

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
| `--backup-dir=path` | 这是备份目录的路径，备份将存储在该目录中。目录必须已存在。此参数为必需且无默认值。每次备份运行时，manticore-backup 会在指定目录中创建一个带时间戳的子目录（`backup-[datetime]`），并将所有必需的表复制到该目录。因此，`--backup-dir` 是所有备份的容器，安全地多次运行该脚本。|
| `--restore[=backup]` | 从`--backup-dir`恢复。仅使用--restore会列出可用备份。`--restore=backup`将从`<--backup-dir>/backup`恢复。|
| `--force` | 在恢复时跳过版本检查，并优雅地恢复备份。|
| `--disable-telemetry` | 如果您想禁用向 Manticore 发送匿名指标，请使用此标志。您也可以使用环境变量 TELEMETRY=0 |
| `--config=/path/to/manticore.conf` | Manticore 配置文件路径。可选。如果未提供，则使用操作系统的默认配置。用于确定与 Manticore 守护进程通信的主机和端口。`manticore-backup` 工具支持[动态配置](../Server_settings/Scripted_configuration.md)文件。如果配置分散在多个文件中，可多次指定`--config`选项。|
| `--tables=tbl1,tbl2, ...` | 你想备份的表的分号分隔列表。要备份所有表，可省略此参数。所有提供的表必须存在于你要备份的 Manticore 实例中，否则备份会失败。|
| `--compress` | 是否压缩备份文件。默认未启用。| 可选 |
| `--unlock` | 在极少数情况下，当出现异常时，表可能被锁定。使用此参数解锁它们。|
| `--version` | 显示当前版本。|
| `--help` | 显示此帮助信息。|

## BACKUP SQL 命令参考

你也可以通过运行简单命令 `BACKUP TO /path/to/backup` 来备份数据。

> 注意： Windows 不支持 `BACKUP`。建议改用 [mysqldump](../Securing_and_compacting_a_table/Backup_and_restore.md#Backup-and-restore-with-mysqldump)。

> 注意： `BACKUP` 需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果不起作用，请确认 Buddy 已安装。

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

例如，要备份表 `a` 和 `b` 到 `/backup` 目录，运行以下命令：

```sql
BACKUP TABLES a, b TO /backup
```

备份过程可以通过以下选项进行控制和调整：

* `async`：使备份非阻塞，立即返回备份路径响应，备份进行时可以执行其他查询。默认值为 `0`。
* `compress`：启用使用 zstd 的文件压缩。默认值为 `0`。

例如，要以异步模式启用压缩将所有表备份到 `/tmp` 目录：

```sql
BACKUP TO /tmp OPTION async = yes, compress = yes
```

### 异步备份行为

使用 `async = 1`（或 `yes`、`on`、`true`）时，备份操作在后台任务中运行：

* 命令立即返回备份路径
* 备份进行时可继续执行其他查询
* 备份任务由 Manticore Buddy 管理的独立线程运行
* 任务运行期间会出现在 `SHOW QUERIES` 输出中，完成后自动移除

**异步备份示例：**

```sql
BACKUP TO /tmp/mybackup OPTION async = 1
```

将立即返回类似下列输出：
```
+----------------------------------+
| Path                             |
+----------------------------------+
| /tmp/mybackup/backup-20221004... |
+----------------------------------+
```

你可以使用 `SHOW QUERIES` 检查备份是否仍在运行。备份完成后任务将从查询列表消失，所有备份文件将存在指定目录。

### 重要注意事项

1. 如果备份路径包含空格，请用单引号括起来，例如 `BACKUP TO '/path/with spaces'`
2. 不含空格的路径无需引号：`BACKUP TO /tmp/backup`
3. 支持 Windows 路径：`BACKUP TO 'C:\path'` 或 `BACKUP TO C:\windows\backup`
4. 确保 Manticore Buddy 已启动（默认启动）
5. 备份目录必须存在且 Manticore 进程有写权限

### 备份如何保持表的一致性

为确保备份期间表的一致性，Manticore Search 的备份工具采用创新的 [FREEZE 和 UNFREEZE](../Securing_and_compacting_a_table/Freezing_and_locking_a_table.md) 命令。不同于 MySQL 等传统的锁表功能，`FREEZE` 停止向磁盘刷新数据，但仍允许对表进行写入（在某种程度上）和选择更新数据。

但如果在涉及大量插入的长时间备份操作中，内存块大小超出 `rt_mem_limit` 阈值，数据可能被刷新到磁盘，此时写操作将被阻塞直到刷新完成。尽管如此，该工具在表冻结期间在表锁、数据一致性和数据库写入可用性之间保持平衡。

使用 `manticore-backup` 或 SQL `BACKUP` 命令时，会执行一次 `FREEZE` 来冻结所有要备份的表。接着备份过程逐个备份表，成功备份后释放相应表的冻结状态。

如果备份失败或中断，工具会尝试解冻所有表。

## 使用 manticore-backup 工具恢复

<!-- example restore_list -->
要从备份恢复 Manticore 实例，使用带有 `--backup-dir` 和 `--restore` 参数的 `manticore-backup` 命令。例如：`manticore-backup --backup-dir=/path/to/backups --restore`。如果 `--restore` 未指定参数，则只会列出 `--backup-dir` 中的所有备份。

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

要启动恢复任务，请运行带有 `--restore=backup name` 标志的 `manticore-backup`，其中 `backup name` 是 `--backup-dir` 目录下的备份文件夹名称。请注意：
1. 在被恢复的同一主机和端口上，不能有任何 Manticore 实例正在运行。
2. 旧的 `manticore.json` 文件不得存在。
3. 旧的配置文件不得存在。
4. 旧的数据目录必须存在且为空。

如果所有条件都满足，恢复将继续。该工具会提供提示，所以您不必记住所有细节。避免覆盖已有文件非常重要，因此如果这些文件仍然存在，请先将其删除。因此有以上所有条件要求。

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

## 使用 mysqldump 备份和恢复

Manticore 支持 MySQL 中的 `mysqldump` 工具到 9.5 版本以及 MariaDB 的 `mariadb-dump` 工具到 12.1 版本。

<!-- example mysqldump_backup -->

> 注意：某些版本的 `mysqldump` / `mariadb-dump` 需要安装 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果转储操作不成功，请确保已安装 Buddy。

要创建 Manticore Search 数据库的备份，可以使用 `mysqldump` 命令。以下示例中将使用默认端口和主机。

请注意，`mysqldump` 仅支持实时表。

<!-- request Basic -->
```bash
mysqldump -h0 -P9306 manticore > manticore_backup.sql
mariadb-dump -h0 -P9306 manticore > manticore_backup.sql
```

执行此命令将生成名为 `manticore_backup.sql` 的备份文件。该文件包含所有数据和表结构。

<!-- request Replace mode -->
```bash
mysqldump -h0 -P9306 --replace --net-buffer-length=16m -etc manticore tbl > tbl.sql
```

这将生成一个名为 `tbl.sql` 的备份文件，其中使用 `replace` 命令代替 `insert`，并在每个批次中保留列名。文档将批处理为最大 16 兆字节大小。不会有 `drop`/`create table` 命令。这对于更改分词设置后进行全文重建索引非常有用。

<!-- request Replication mode -->
```bash
mysqldump -etc --replace -h0 -P9306 -ucluster manticore --skip-lock-tables cluster:tbl | mysql -P9306 -h0
mariadb-dump -etc --replace -h0 -P9306 -ucluster manticore --skip-lock-tables cluster:tbl | mysql -P9306 -h0
```

在这种情况下，`mysqldump` 会生成诸如 `REPLACE INTO cluster:table ...` 的命令，这些命令会直接发送到 Manticore 实例，从而重新插入文档。
使用 `cluster` 用户和 `-t` 标志以启用复制模式。详细信息见下方注释。

<!-- end -->

<!-- example mysqldump_restore -->
### 恢复

如果您想要从备份文件恢复 Manticore Search 数据库，mysql 客户端是您的首选工具。

请注意，如果您在[纯模式](../Read_this_first.md#Real-time-mode-vs-plain-mode)下恢复，则不能直接删除并重建表。因此，您应当：
- 使用带 `-t` 选项的 `mysqldump` 以从备份中排除 `CREATE TABLE` 语句。
- 在继续恢复之前，手动[TRUNCATE](../Emptying_a_table.md)表。

<!-- request SQL -->
```bash
mysql -h0 -P9306 < manticore_backup.sql
mariadb -h0 -P9306 < manticore_backup.sql
```

此命令使您能够从 `manticore_backup.sql` 文件中恢复所有内容。
<!-- end -->

### 额外选项

以下是一些可以与 mysqldump 一起使用以定制备份的更多设置：

- `-t` 跳过 `drop`/`create` 表命令。对更改分词设置后进行全文重建索引非常有用。
- `--no-data`：此设置将忽略表数据，只备份表结构。
- `--ignore-table=[database_name].[table_name]`：此选项允许在备份时跳过特定表。注意数据库名称必须为 `manticore`。
- `--replace` 使用 `replace` 替代 `insert`。对更改分词设置后全文重建索引非常有用。
- `--net-buffer-length=16M` 将批处理大小调整为最多 16 兆字节，以加快恢复速度。
- `-e` 用于批处理文档。加速恢复。
- `-c` 保留列名称。在更改表架构（例如字段顺序改变）后重建索引时很有用。

欲了解设置的完整列表及详细说明，请参阅官方的 [MySQL 文档](https://dev.mysql.com/doc/refman/8.0/en/mysqldump.html) 或 [MariaDB 文档](https://mariadb.com/kb/en/mariadb-dump/)。

### 注释

* 创建复制模式的转储（转储包括 `INSERT/REPLACE INTO <cluster_name>:<table_name>`）时：
  - 确保在转储过程中表未被更改。
  - 使用 `cluster` 用户。例如：`mysqldump -u cluster ...` 或 `mariadb-dump -u cluster ...`。您可以通过执行 `SET GLOBAL cluster_user = new_name` 来更改启用复制模式的 `mysqldump` 用户名。
  - 使用 `-t` 标志。
  - 使用 `--skip-lock-tables` 标志。
  - 在复制模式下指定表时，需使用 `cluster_name:table_name` 语法。例如：`mysqldump -P9306 -h0 -t -ucluster manticore cluster:tbl`。
* 建议在计划备份所有数据库时明确指定 `manticore` 数据库，而不是使用 `--all-databases` 选项。
* 注意，`mysqldump` 不支持备份分布式表，也不能备份包含非存储字段的表。对于这些情况，建议使用 `manticore-backup` 或者 `BACKUP` SQL 命令。如果您有分布式表，推荐始终指定要转储的表。

<!-- proofread -->
