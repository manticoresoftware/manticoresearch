# Backup and restore

It is crucial to regularly back up your tables in order to be able to recover them in the event of problems such as system crashes, hardware failure, or data corruption/loss for any reason. Backups are also essential before upgrading Manticore Search to a new version that introduces a change in table format, and can also be used to transfer your data to another system when you want to migrate to a new server.

The `manticore-backup` tool, which is included in the Manticore Search [official packages](https://manticoresearch.com/install/) can help you automate the process of backing up tables for a Manticore instance running in [RT mode](../Read_this_first.md#Real-time-mode-vs-plain-mode), giving you peace of mind in case of any outages or crashes.

### Installation

**If you followed [the official installation instructions](https://manticoresearch.com/install/) you should have already everything installed and don't need to worry.** Otherwise, [manticore-backup](https://github.com/manticoresoftware/manticoresearch-backup) requires PHP 8.1.10 and [specific modules](https://github.com/manticoresoftware/executor/blob/main/build-linux) or [manticore-executor](https://github.com/manticoresoftware/executor) which is a part of package `manticore-extra` and you need to make sure either of them is available.

`manticore-backup` is not available for Windows.

### How to use

First, make sure you're running `manticore-backup` on the same server where the Manticore instance you are about to back up is running.

Second, we recommend running the tool under the `root` user so the tool can transfer ownership of the files you are backing up. Otherwise, a backup will be also made but with no ownership transfer. In either case, you should make sure that `manticore-backup` has access to the data dir of the Manticore instance.

<!-- example backup1 -->

The only mandatory argument is `--backup-dir` - the path to put the backup in. In this case, if you omit all the other arguments `manticore-backup` will:
* find Manticore instance running with a default config
* create a subdirectory in the `--backup-dir` directory with a timestamp in the name
* back up all tables found in the instance

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
If you want to select specific tables to back up, you can use the `--tables` flag followed by a comma-separated list of tables, e.g. `--tables=tbl1,tbl2`. This will only back up the specified tables and skip all others.

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

For example, you can run a backup of tables `a` and `b` to the `/backup` directory using the following command:

```sql
BACKUP TABLES a, b TO /backup
```

Options allow you to control the backup process and tune certain aspects:

* `async` makes the backup non-blocking, allowing you to receive a response with the query id immediately and run other queries while the backup is in progress. The default value is `0`.
* `compress` enables file compression using zstd. The default value is `0`.

For example, you can run a backup of all tables in async mode with compression enabled to the `/tmp` directory:


```sql
BACKUP OPTION async = yes, compress = yes TO /tmp
```

### Important notes

1. We do not support special symbols or spaces in the path.
2. You must have Manticore Buddy launched (it is enabled by default).

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
To start a restore job, you need to run `--restore=backup name`, where `backup name` is the name of the backup directory inside the `--backup-dir`. Note that:
1. There should be no Manticore instance running on the same host and port as the ones you are restoring.
2. The old manticore.json file should not exist.
3. The old configuration file should not exist.
4. The old data directory should exist and should be empty.

Only if all the conditions are met, will the restore happen. Don't worry the tool will give you hints so you don't need to remember it. What's important is to not overwrite your existing files, that's why there's the requirement to remove them beforehand if they still exist.

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
