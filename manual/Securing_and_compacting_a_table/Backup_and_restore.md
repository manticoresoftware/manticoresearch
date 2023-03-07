# Backup and restore

It's crucial to regularly back up your tables to recover them in case of system crashes, hardware failure, or data corruption/loss. Backups are also necessary before upgrading to a new version of Manticore Search that changes the table format, and for transferring data to another system when migrating to a new server.

The `manticore-backup` tool, included in the official Manticore Search [packages](https://manticoresearch.com/install), automates the process of backing up tables for an instance running in [RT mode](../Read_this_first.md#Real-time-mode-vs-plain-mode).

### Installation

**If you followed [the official installation instructions](https://manticoresearch.com/install/), you should already have everything installed and don't need to worry.** Otherwise, [`manticore-backup`](https://github.com/manticoresoftware/manticoresearch-backup) requires PHP 8.1.10 and [specific modules](https://github.com/manticoresoftware/executor/blob/main/build-linux) or [`manticore-executor`](https://github.com/manticoresoftware/executor), which is a part of the `manticore-extra` package, and you need to ensure that one of these is available.

Note that `manticore-backup` is not available for Windows yet.

### How to use

First, make sure you're running `manticore-backup` on the same server where the Manticore instance you are about to back up is running.

Second, we recommend running the tool under the `root` user so the tool can transfer ownership of the files you are backing up. Otherwise, a backup will be also made but with no ownership transfer. In either case, you should make sure that `manticore-backup` has access to the data dir of the Manticore instance.

<!-- example backup1 -->

The only required argument for `manticore-backup` is `--backup-dir`, which specifies the destination for the backup. If you don't provide any additional arguments, `manticore-backup` will:
- locate a Manticore instance running with the default configuration
- create a subdirectory in the `--backup-dir` directory with a timestamped name
- backup all tables found in the instance

<!-- request Example -->
```bash
manticore-backup --config=path/to/manticore.conf --backup-dir=backupdir
```

<!-- response Example -->
```bash
Copyright (c) 2023, Manticore Software LTD (https://manticoresearch.com)

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
To back up specific tables only, use the `--tables` flag followed by a comma-separated list of tables, for example `--tables=tbl1,tbl2`. This will only backup the specified tables and ignore the rest.

<!-- request Example -->
```bash
manticore-backup --backup-dir=/mnt/backup/ --tables=products
```

<!-- response Example -->
```bash
Copyright (c) 2023, Manticore Software LTD (https://manticoresearch.com)

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

## Arguments

| Argument | Description |
|-|-|
| `--backup-dir=path` | This is the path to the backup directory where the backup will be stored. The directory must already exist. This argument is required and has no default value. On each backup run, manticore-backup will create a subdirectory in the provided directory with a timestamp in the name (`backup-[datetime]`), and will copy all required tables to it. So the `--backup-dir` is a container for all your backups, and it's safe to run the script multiple times.|
| `--restore[=backup]` | Restore from `--backup-dir`. Just --restore lists available backups. `--restore=backup` will restore from `<--backup-dir>/backup`. |
| `--config=/path/to/manticore.conf` | Path to Manticore config. This is optional. If it's not passed, a default one for your operating system will be used. It's used to get the host and port to communicate with the Manticore daemon. |
| `--tables=tbl1,tbl2, ...` | Semicolon-separated list of tables that you want to back up. To back up all tables, omit this argument. All the provided tables must exist in the Manticore instance you are backing up from, or the backup will fail. |
| `--compress` | Whether the backed up files should be compressed. Not enabled by default. | optional |
| `--unlock` | In rare cases when something goes wrong, tables can be left in a locked state. Use this argument to unlock them. |
| `--version` | Show the current version. |
| `--help` | Show this help. |

## BACKUP SQL command reference

You can also back up your data through SQL by running the simple command `BACKUP TO /path/to/backup`.

Note, this command is not supported in Windows yet.

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

To ensure consistency of tables during backup, Manticore Search's backup tools use the innovative [FREEZE and UNFREEZE](../Securing_and_compacting_a_table/Freezing_a_table.md) commands. Unlike the traditional lock and unlock tables feature of e.g. MySQL, `FREEZE` stops flushing data to disk while still permitting writing (to some extent) and selecting updated data from the table.

However, if your RAM chunk size grows beyond the `rt_mem_limit` threshold during lengthy backup operations involving many inserts, data may be flushed to disk, and write operations will be blocked until flushing is complete. Despite this, the tool maintains a balance between table locking, data consistency, and database write availability while the table is frozen.

When you use `manticore-backup` or the SQL `BACKUP` command, the `FREEZE` command is executed once and freezes all tables you are backing up simultaneously. The backup process subsequently backs up each table one by one, releasing the freeze after successfully backing up each table.

If backup fails or gets interrupted, the tool tries to unfreeze all the tables.

## Restore

<!-- example restore_list -->
To restore a Manticore instance from a backup, use the `manticore-backup` command with the `--backup-dir` and `--restore` arguments. For example: `manticore-backup --backup-dir=/path/to/backups --restore`. If you don't provide any argument for `--restore`, it will simply list all the backups in the `--backup-dir`.

<!-- request Example -->
```bash
manticore-backup --backup-dir=/mnt/backup/ --restore
```

<!-- response Example -->

```bash
Copyright (c) 2023, Manticore Software LTD (https://manticoresearch.com)

Manticore config file:
Backup dir: /tmp/

Available backups: 3
  backup-20221006144635 (Oct 06 2022 14:46:35)
  backup-20221006145233 (Oct 06 2022 14:52:33)
  backup-20221007104044 (Oct 07 2022 10:40:44)
```

<!-- end -->

<!-- example restore -->

To start a restore job, run `manticore-backup` with the flag `--restore=backup name`, where `backup name` is the name of the backup directory within the `--backup-dir`. Note that:
1. There can't be any Manticore instance running on the same host and port as the one being restored.
2. The old `manticore.json` file must not exist.
3. The old configuration file must not exist.
4. The old data directory must exist and be empty.

If all conditions are met, the restore will proceed. The tool will provide hints, so you don't have to memorize them. It's crucial to avoid overwriting existing files, so make sure to remove them prior to the restore if they still exist. Hence all the conditions.

<!-- request Example -->
```bash
manticore-backup --backup-dir=/mnt/backup/ --restore=backup-20221007104044
```

<!-- response Example -->

```bash
Copyright (c) 2023, Manticore Software LTD (https://manticoresearch.com)

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

<!-- proofread -->
