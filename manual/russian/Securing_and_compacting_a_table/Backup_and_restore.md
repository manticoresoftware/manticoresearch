# Backup and Restore

Backing up your tables on a regular basis is essential for recovery in the event of system crashes, hardware failure, or data corruption/loss. It's also highly recommended to make backups before upgrading to a new Manticore Search version or running [ALTER TABLE](../Updating_table_schema_and_settings.md#Updating-table-schema-in-RT-mode).

Backing up database systems can be done in two unique ways: logical and physical backups. Each of these methods has its pros and cons, which may vary based on the specific database environment and needs. Here, we'll delve into the distinction between these two types of backups.

### Logical Backups

Logical backups entail exporting the database schema and data as SQL statements or as data formats specific to the database. This backup form is typically readable by humans and can be employed to restore the database on various systems or database engines.

Pros and cons of logical backups:
- ➕ **Portability:** Logical backups are generally more portable than physical backups, as they can be used to restore the database on different hardware or operating systems.
- ➕ **Flexibility:** Logical backups allow you to selectively restore specific tables, indexes, or other database objects.
- ➕ **Compatibility:** Logical backups can be used to migrate data between different database management systems or versions, provided the target system supports the exported format or SQL statements.
- ➖ **Slower Backup and Restore:** Logical backups can be slower than physical backups, as they require the database engine to convert the data into SQL statements or another export format.
- ➖ **Increased System Load:** Creating logical backups can cause higher system load, as the process requires more CPU and memory resources to process and export the data.

Manticore Search supports [mysqldump](../Securing_and_compacting_a_table/Backup_and_restore.md#Backup-and-restore-with-mysqldump) for logical backups.

### Physical Backups

Physical backups involve copying the raw data files and system files that comprise the database. This type of backup essentially creates a snapshot of the database's physical state at a given point in time.

Pros and cons of physical backups:
- ➕ **Speed:** Physical backups are usually faster than logical backups, as they involve copying raw data files directly from disk.
- ➕ **Consistency:** Physical backups ensure a consistent backup of the entire database, as all related files are copied together.
- ➕ **Lower System Load:** Creating physical backups generally places less load on the system compared to logical backups, as the process does not involve additional data processing.
- ➖ **Portability:** Physical backups are typically less portable than logical backups, as they may be dependent on the specific hardware, operating system, or database engine configuration.
- ➖ **Flexibility:** Physical backups do not allow for the selective restoration of specific database objects, as the backup contains the entire database's raw files.
- ➖ **Compatibility:** Physical backups cannot be used to migrate data between different database management systems or versions, as the raw data files may not be compatible across different platforms or software.

Manticore Search has [manticore-backup](../Securing_and_compacting_a_table/Backup_and_restore.md#Using-manticore-backup-command-line-tool) command line tool for physical backups.

In summary, logical backups provide more flexibility, portability, and compatibility but can be slower and more resource-intensive, while physical backups are faster, more consistent, and less resource-intensive but may be limited in terms of portability and flexibility. The choice between these two backup methods will depend on your specific database environment, hardware, and requirements.

## Using manticore-backup command line tool

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

### Arguments

| Argument | Description |
|-|-|
| `--backup-dir=path` | Это путь к директории резервного копирования, где будет храниться резервная копия. Директория должна уже существовать. Этот аргумент обязателен и не имеет значения по умолчанию. При каждом запуске резервного копирования manticore-backup создаст поддиректорию в указанной директории с отметкой времени в имени (`backup-[datetime]`) и скопирует в неё все необходимые таблицы. Таким образом, `--backup-dir` является контейнером для всех ваших резервных копий, и безопасно запускать скрипт несколько раз. |
| `--restore[=backup]` | Восстановление из `--backup-dir`. Просто --restore выводит список доступных резервных копий. `--restore=backup` восстановит из `<--backup-dir>/backup`. |
| `--force` | Пропустить проверку версий при восстановлении и выполнить восстановление резервной копии без ошибок. |
| `--disable-telemetry` | Используйте этот флаг, если хотите отключить отправку анонимизированных метрик в Manticore. Также можно использовать переменную окружения TELEMETRY=0 |
| `--config=/path/to/manticore.conf` | Путь к конфигурации Manticore. Необязательно. Если не указан, будет использована конфигурация по умолчанию для вашей операционной системы. Используется для определения хоста и порта для взаимодействия с демоном Manticore. Инструмент `manticore-backup` поддерживает [динамические конфигурационные файлы](../Server_settings/Scripted_configuration.md). Вы можете указывать опцию `--config` несколько раз, если ваша конфигурация разбита на несколько файлов. |
| `--tables=tbl1,tbl2, ...` | Список таблиц, которые необходимо сохранить, разделенный точкой с запятой. Чтобы сохранить все таблицы, пропустите этот аргумент. Все указанные таблицы должны существовать в экземпляре Manticore, с которого выполняется резервное копирование, иначе резервное копирование завершится ошибкой. |
| `--compress` | Следует ли сжимать резервные файлы. По умолчанию не включено. | необязательно |
| `--unlock` | В редких случаях, если что-то пошло не так, таблицы могут остаться заблокированными. Используйте этот аргумент, чтобы разблокировать их. |
| `--version` | Показать текущую версию. |
| `--help` | Показать эту справку. |

## Справочник по команде BACKUP SQL

Вы также можете создавать резервные копии через SQL, выполнив простую команду `BACKUP TO /path/to/backup`.

> ПРИМЕЧАНИЕ: `BACKUP` не поддерживается в Windows. Рассмотрите возможность использования [mysqldump](../Securing_and_compacting_a_table/Backup_and_restore.md#Backup-and-restore-with-mysqldump) вместо него.

> ПРИМЕЧАНИЕ: `BACKUP` требует [Manticore Buddy](../Installation/Manticore_Buddy.md). Если не работает, убедитесь, что Buddy установлен.

### Общий синтаксис BACKUP

```sql
BACKUP
  [{TABLE | TABLES} a[, b]]
  [{OPTION | OPTIONS}
    async = {on | off | 1 | 0 | true | false | yes | no}
    [, compress = {on | off | 1 | 0 | true | false | yes | no}]
  ]
  TO path_to_backup
```

Например, чтобы создать резервные копии таблиц `a` и `b` в директорию `/backup`, выполните следующую команду:

```sql
BACKUP TABLES a, b TO /backup
```

Доступны опции для управления процессом резервного копирования, такие как:

* `async`: делает резервное копирование неблокирующим, позволяя сразу получить ответ с ID запроса и выполнять другие запросы во время выполнения резервного копирования. Значение по умолчанию `0`.
* `compress`: включает сжатие файлов с использованием zstd. Значение по умолчанию `0`.
Например, чтобы выполнить резервное копирование всех таблиц в асинхронном режиме с включенным сжатием в директорию `/tmp`:

```sql
BACKUP OPTION async = yes, compress = yes TO /tmp
```

### Важные замечания

1. Путь не должен содержать специальные символы или пробелы, так как они не поддерживаются.
2. Убедитесь, что Manticore Buddy запущен (по умолчанию он запущен).

### Как резервное копирование обеспечивает согласованность таблиц

Для обеспечения согласованности таблиц во время резервного копирования инструменты резервного копирования Manticore Search используют инновационные команды [FREEZE и UNFREEZE](../Securing_and_compacting_a_table/Freezing_a_table.md). В отличие от традиционной блокировки и разблокировки таблиц, например, в MySQL, команда `FREEZE` приостанавливает запись данных на диск, но при этом продолжает позволять запись (в определённой степени) и выборку обновленных данных из таблицы.

Однако, если размер chunks в RAM превысит предел `rt_mem_limit` во время длительных операций резервного копирования с большим количеством вставок, данные могут быть сброшены на диск, и операции записи будут заблокированы до завершения сброса. Несмотря на это, инструмент поддерживает баланс между блокировкой таблиц, согласованностью данных и доступностью записи в базу данных во время заморозки таблицы.

Когда вы используете `manticore-backup` или SQL-команду `BACKUP`, команда `FREEZE` выполняется один раз и одновременно замораживает все таблицы, которые вы резервируете. Процесс резервного копирования затем копирует каждую таблицу по очереди, снимая заморозку после успешного резервного копирования каждой таблицы.

Если резервное копирование не удаётся или прерывается, инструмент пытается разморозить все таблицы.

## Восстановление с помощью инструмента manticore-backup

<!-- example restore_list -->
Для восстановления экземпляра Manticore из резервной копии используйте команду `manticore-backup` с аргументами `--backup-dir` и `--restore`. Например: `manticore-backup --backup-dir=/path/to/backups --restore`. Если вы не укажете аргумент для `--restore`, он просто выведет список всех резервных копий в `--backup-dir`.

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

Чтобы запустить задание на восстановление, выполните `manticore-backup` с флагом `--restore=backup name`, где `backup name` — имя каталога резервной копии в `--backup-dir`. Обратите внимание:
1. Нельзя запускать экземпляр Manticore на том же хосте и порту, что и восстанавливаемый.
2. Старый файл `manticore.json` не должен существовать.
3. Старый конфигурационный файл не должен существовать.
4. Директория с данными должна существовать и быть пустой.

Если все условия выполнены, восстановление начнётся. Инструмент предоставит подсказки, чтобы вам не пришлось их запоминать. Очень важно не перезаписывать существующие файлы, поэтому убедитесь, что вы их удалили до восстановления, если они всё ещё существуют. Именно поэтому все эти условия.

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

## Резервное копирование и восстановление с помощью mysqldump

<!-- example mysqldump_backup -->

> ПРИМЕЧАНИЕ: некоторые версии `mysqldump` / `mariadb-dump` требуют [Manticore Buddy](../Installation/Manticore_Buddy.md). Если дамп не работает, убедитесь, что Buddy установлен.

Чтобы создать резервную копию вашей базы данных Manticore Search, вы можете использовать команду `mysqldump`. В примерах будет использоваться порт и хост по умолчанию.

Обратите внимание, что `mysqldump` поддерживается только для таблиц реального времени.

<!-- request Basic -->
```bash
mysqldump -h0 -P9306 manticore > manticore_backup.sql
mariadb-dump -h0 -P9306 manticore > manticore_backup.sql
```

Выполнение этой команды создаст файл резервной копии с именем `manticore_backup.sql`. В этом файле будут содержаться все данные и схемы таблиц.

<!-- request Replace mode -->
```bash
mysqldump -h0 -P9306 --replace --net-buffer-length=16m -etc manticore tbl > tbl.sql
```

Это создаст файл резервной копии `tbl.sql` с командами `replace` вместо `insert`, при этом имена столбцов сохраняются в каждом пакете. Документы будут объединены в пакеты размером до 16 мегабайт. Команды `drop`/`create table` отсутствуют. Это полезно для полнотекстовой переиндексации после изменения настроек токенизации.

<!-- request Replication mode -->
```bash
mysqldump -etc --replace -h0 -P9306 -ucluster manticore cluster:tbl | mysql -P9306 -h0
mariadb-dump -etc --replace -h0 -P9306 -ucluster manticore cluster:tbl | mysql -P9306 -h0
```

В этом случае `mysqldump` будет генерировать команды вида `REPLACE INTO cluster:table ...`, которые будут напрямую отправляться в экземпляр Manticore, что приведет к повторной вставке документов.
Используйте пользователя `cluster` и флаг `-t` для включения режима репликации. Подробнее об этом смотрите в примечаниях ниже.

<!-- end -->

<!-- example mysqldump_restore -->
### Восстановление

Если вы хотите восстановить базу данных Manticore Search из файла резервной копии, инструментом выбора является клиент mysql.

Обратите внимание, что при восстановлении в [Plain mode](../Read_this_first.md#Real-time-mode-vs-plain-mode) вы не можете напрямую удалять и создавать таблицы. Поэтому необходимо:
- Использовать `mysqldump` с опцией `-t`, чтобы исключить из резервной копии команды `CREATE TABLE`.
- Вручную [TRUNCATE](../Emptying_a_table.md) таблицы перед продолжением восстановления.

<!-- request SQL -->
```bash
mysql -h0 -P9306 < manticore_backup.sql
mariadb -h0 -P9306 < manticore_backup.sql
```

Эта команда позволяет восстановить все из файла `manticore_backup.sql`.
<!-- end -->

### Дополнительные опции

Вот еще несколько настроек, которые можно использовать с mysqldump для настройки резервного копирования:

- `-t` пропускает команды `drop`/`create` таблиц. Полезно для полнотекстовой переиндексации таблицы после изменения настроек токенизации.
- `--no-data`: Эта опция исключает данные таблиц из резервной копии, создавая файл, содержащий только схемы таблиц.
- `--ignore-table=[database_name].[table_name]`: Эта опция позволяет пропустить конкретную таблицу при резервном копировании. Имя базы данных должно быть `manticore`.
- `--replace` для использования `replace` вместо `insert`. Полезно для полнотекстовой переиндексации таблицы после изменения настройки токенизации.
- `--net-buffer-length=16M` для создания пакетов размером до 16 мегабайт для более быстрой реставрации.
- `-e` для объединения документов в пакеты. Ускоряет реставрацию.
- `-c` для сохранения имен столбцов. Полезно для переиндексации таблицы после изменения ее схемы (например, изменения порядка полей).

Для полного списка настроек и их подробного описания обратитесь к официальной [документации MySQL](https://dev.mysql.com/doc/refman/8.0/en/mysqldump.html) или [документации MariaDB](https://mariadb.com/kb/en/mariadb-dump/).

### Примечания

* Для создания дампа в режиме репликации (когда дамп включает `INSERT/REPLACE INTO <cluster_name>:<table_name>`):
  - Используйте пользователя `cluster`. Например: `mysqldump -u cluster ...` или `mariadb-dump -u cluster ...`. Вы можете изменить имя пользователя, включающего режим репликации для `mysqldump`, выполнив `SET GLOBAL cluster_user = new_name`.
  - Используйте флаг `-t`.
  - При указании таблицы в режиме репликации следует использовать синтаксис `cluster_name:table_name`. Например: `mysqldump -P9306 -h0 -t -ucluster manticore cluster:tbl`.
* Рекомендуется явно указывать базу данных `manticore`, если вы планируете бэкапить все базы данных, вместо использования опции `--all-databases`.
* Обратите внимание, что `mysqldump` не поддерживает резервное копирование распределенных таблиц и не может делать бэкап таблиц, содержащих нестатические поля. Для таких случаев рассмотрите использование `manticore-backup` или SQL-команды `BACKUP`. Если у вас есть распределенные таблицы, рекомендуется всегда специально указывать таблицы для дампа.

<!-- proofread -->

