# Telemetry

At Manticore, we gather various anonymized metrics to enhance the quality of our products, including Manticore Search. By analyzing this data, we can not only improve the overall performance of our product but also identify which features would be most beneficial to prioritize in order to provide even more value to our users. The telemetry system operates on a separate thread in a non-blocking mode, taking snapshots and sending them once every few minutes.

We take your privacy seriously, and you can rest assured that all metrics are completely anonymous and no sensitive information is transmitted. However, if you still wish to disable telemetry, you have the option to do so by:
* Setting the environment variable `TELEMETRY=0`
* Or setting `telemetry = 0` in the section `searchd` of your configuration file

Here is a list of all the metrics we collect:

The ⏱️ symbol indicates that the metric is collected periodically, as opposed to other metrics which are collected based on specific events.

| Metric | Description |
|-|-|
| `invocation` | Sent when Manticore Buddy is launched |
| `plugin_*` | Indicates that the plugin with a given name was executed, `plugin_backup` for backup execution, for example |
| `command_*` | ⏱️ All metrics with this prefix are sent from the `show status` query of the Manticore daemon   |
| `uptime` | ⏱️ The uptime of the Manticore Search daemon |
| `workers_total` | ⏱️ The number of workers used by Manticore |
| `cluster_count` | ⏱️ How many clusters this node handles |
| `cluster_size` | ⏱️ How many nodes in all clusters |
| `table_*_count` | ⏱️ The number of tables created for each type: plain, percolate, rt, or distributed |
| `*_field_*_count` | ⏱️ The count for each field type for tables with rt and percolate types |
| `columnar` | ⏱️ Indicates that the Columnar library was used |
| `columnar_field_count` | ⏱️ The number of fields that use the Columnar library |

## Backup metrics

The Manticore backup tool sends anonymized metrics to the Manticore metrics server by default in order to help improve the product. If you don't want to send telemetry, you can disable it by running the tool with the `--disable-metric` flag or by setting the environment variable `TELEMETRY=0`.

The following is a list of all collected metrics:

| Metric | Description |
|-|-|
| `invocation` | Sent when backup was initiated  |
| `failed` | Sent in case of failed backup |
| `done` | Sent when backup/restore is successful |
| `arg_*` | The arguments used to run the tool (excluding index names, etc.) |
| `backup_store_versions_fails` | Indicates failure in saving Manticore version in the backup |
| `backup_table_count` | Total number of backed up tables |
| `backup_no_permissions` | Failed backup due to insufficient permissions to destination directory |
| `backup_total_size` | Total size of the full backup |
| `backup_time` | Duration of the backup |
| `restore_searchd_running` | Failed to run restore process due to searchd already running |
| `restore_no_config_file` | No config file found in the backup during restore |
| `restore_time` | Duration of the restore |
| `fsync_time` | Duration of fsync |
| `restore_target_exists` | Occurs when a folder or index already exists in the destination folder for restore |
| `terminations` | Indicates that the process was terminated |
| `signal_*` | The signal used to terminate the process |
| `tables` | Number of tables in Manticore |
| `config_unreachable` | Specified configuration file does not exist |
| `config_data_dir_missing` | Failed to parse `data_dir` from specified configuration file |
| `config_data_dir_is_relative` | `data_dir` path in Manticore instance's configuration file is relative |

## Labels

Each metric comes with the following labels:

| Label | Description |
|-|-|
| `collector` | `buddy`. Indicates that this metric is collected through Manticore Buddy |
| `os_name` | Name of the operating system |
| `os_release_name` | Name from the `/etc/os-release` if presents or `unknown` |
| `os_release_version` | Version from the `/etc/os-release` if presents or `unknown` |
| `dockerized` | If it's run inside the Docker environment |
| `official_docker` | In case of Docker it's flag that shows we use official image |
| `machine_id` | Server identifier (the content of `/etc/machine-id` in Linux) |
| `arch` | Architecture of the machin we run on |
| `manticore_version` | Version of Manticore |
| `columnar_version` | Version of the Columnar library if it is installed |
| `secondary_version` | Version of the secondary library if the Columnar library is installed |
| `knn_version` | Version of the KNN library if the Columnar library is installed |
| `buddy_version`| Version of Manticore Buddy |

<!-- proofread -->
