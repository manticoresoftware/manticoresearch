# Backup and restore

It is vital to back up your tables to recover them later and be up and running again in case problems occur, such as system crashes, hardware failure, or data corruption / loss by any reason. Backups are also essential as a safeguard before upgrading Manticore Search to a new version which introduces index format change and can also be used to transfer your data to another system when you want to migrate to a new server.

`manticore-backup` which is a part of Manticore Search [official packages](https://manticoresearch.com/install/) can help you automate backing up tables of a Manticore instance running in [RT mode](../Read_this_first.md#Real-time-mode-vs-plain-mode) and feel safe in case of any outages or crashes.

### Installation

**If you followed [the official installation instructions](https://manticoresearch.com/install/) you should have already everything installed and don't need to worry.** Otherwise, [manticore-backup](https://github.com/manticoresoftware/manticoresearch-backup) requires PHP 8.1.10 or [manticore-executor](https://github.com/manticoresoftware/executor) which is a part of package `manticore-extra` and you need to make sure either of them is available.

⚠️ Note, support of Windows for `manticore-backup` is still in progress. You can use the tool on any `Linux` distribution or `macOS`.


### How to use

First, make sure you're running `manticore-backup` on the same server where the Manticore instance you are about to back up is running.

Second, we recommend running the tool under the `root` user so the tool can transfer ownership of the files you are backing up. Otherwise, a backup will be also made but with no ownership transfer. In either case, you should make sure that `manticore-backup` has access to the data dir of the Manticore instance.

<!-- example backup1 -->

The only mandatory argument is `--backup-dir` - the path to put the backup in. In this case if you omit all the other arguments `manticore-backup` will:
* find Manticore instance running with a default config
* create a subdirectory in the `--backup-dir` directory with a timestamp in the name.
* back up all tables found in the instance

<!-- request Example -->
```bash
manticore-backup --config=path/to/manticore.conf --backup-dir=backupdir
```

<!-- response Example -->
```bash
Copyright (c) 2022, Manticore Software LTD (https://manticoresearch.com)

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
If you want to select tables to backup feel free to use flag `--tables=tbl1,tbl2` that will back up only required tables and skip all others.

<!-- request Example -->
```bash
manticore-backup --backup-dir=/mnt/backup/ --tables=products
```

<!-- response Example -->
```bash
Copyright (c) 2022, Manticore Software LTD (https://manticoresearch.com)

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
| `--backup-dir=path` | This is a path to the backup directory where a backup is stored.  The directory must exist. This argument is required and has no default value. On each backup run, it will create directory `backup-[datetime]` in the provided directory and will copy all required tables to it. So the backup-dir is a container of all your backups, and it's safe to run the script multiple times.|
| `--restore[=backup]` | Restore from `--backup-dir`. Just --restore lists available backups. `--restore=backup` will restore from `<--backup-dir>/backup`. |
| `--config=/path/to/manticore.conf` | Path to Manticore config. This is optional and in case it's not passed we use a default one for your operating system. It's used to get the host and port to talk with the Manticore daemon. |
| `--tables=tbl1,tbl2, ...` | Semicolon-separated list of tables that you want to backup. To back up all the tables, just skip this argument. All the provided tables are supposed to exist in the Manticore instance you are backing up from, otherwise the backup will fail. |
| `--compress` | Whether the backed up files should be compressed. Not by default. | optional |
| `--unlock` | In rare cases when something goes wrong the tables can be left in locked state. Using this argument you can unlock them. |
| `--version` | Show the current version. |
| `--help` | Show this help. |


## Restore

<!-- example restore_list -->
To restore Manticore instance from backup you need to do `manticore-backup --backup-dir=/path/to/backups --restore`. When you don't provide any argument for `--restore` it will just list all backups in the `--backup-dir`.

<!-- request Example -->
```bash
manticore-backup --backup-dir=/mnt/backup/ --restore
```

<!-- response Example -->

```bash
Copyright (c) 2022, Manticore Software LTD (https://manticoresearch.com)

Manticore config file:
Backup dir: /tmp/

Available backups: 3
  backup-20221006144635 (Oct 06 2022 14:46:35)
  backup-20221006145233 (Oct 06 2022 14:52:33)
  backup-20221007104044 (Oct 07 2022 10:40:44)
```

<!-- end -->

<!-- example restore -->
To start a restore job you need to do `--restore=backup name`, where `backup name` is the name of the backup directory inside the `--backup-dir`.

Note also, that:
1. There should be no Manticore instance running on the same host and port as the ones you are restoring
2. Old `manticore.json` should not exist
3. Old configuration file should not exist
4. Old directory with tables should exist and should be empty

Only if all the conditions are met, will the restore happen. Don't worry the tool will give you hints so you don't need to remember it. What's important is to not overwrite your existing files, that's why there's the requirement to remove them beforehand if they still exist.

<!-- request Example -->
```bash
manticore-backup --backup-dir=/mnt/backup/ --restore=backup-20221007104044
```

<!-- response Example -->

```bash
Copyright (c) 2022, Manticore Software LTD (https://manticoresearch.com)

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
