# Telemetry

At Manticore, we gather various anonymized metrics in order to enhance the quality of our products, including Manticore Search. By analyzing this data, we can not only improve the overall performance of our product, but also identify which features would be most beneficial to prioritize in order to provide even more value to our users. The telemetry system operates on a separate thread in non-blocking mode, taking snapshots and sending them once every few minutes.

We take your privacy seriously, and you can be assured that all metrics are completely anonymous and no sensitive information is transmitted. However, if you still wish to disable telemetry, you can do so by:
* setting the environment variable `TELEMETRY=0`
* or setting `telemetry = 0` in the section `searchd` of your configuration file

Here's a list of all the metrics we collect:

| Metric | Description |
|-|-|
| collector | 🏷 `buddy`. Indicates that this metric comes through Manticore Buddy |
| os_name | 🏷️ Name of the operating system |
| machine_id | 🏷 Server identifier (the content of `/etc/machine-id` in Linux)
| manticore_version | 🏷️ Version of Manticore |
| columnar_version | 🏷️ Version of Columnar lib if it is installed |
| secondary_version | 🏷️ Version of the secondary lib if the Columnar lib is installed |
| buddy_version| 🏷️ Version of the Buddy |
| invocation | Sent when the Buddy is launched |
| show_queries | Indicates that the `show queries` command was executed |
| backup | Indicates that the `backup` query was executed |
| insert_query | Indicates that the auto schema logic was executed |
| command_* |  All metrics with this prefix are sent from the `show status` query of the manticore daemon   |
| uptime | The uptime of the manticore search daemon |
| workers_total | The number of workers that Manticore uses |
| cluster_* | Cluster-related metrics from the `show status` results |
| table_*_count | The number of tables created for each type: plain, percolate, rt, or distributed |
| *_field_*_count | The count for each field type for tables with rt and percolate types |
| columnar | Indicates that the columnar lib was used |
| columnar_field_count | The number of fields that use the columnar lib |

## Backup metrics

The Manticore backup tool sends anonymized metrics to the Manticore metrics server by default in order to help the maintainers improve the product. If you wish to disable telemetry, you can do so by running the tool with the `--disable-metric` flag or by setting the environment variable `TELEMETRY=0`.

Below is a list of all metrics that we collect:

| Metric | Description |
|-|-|
| collector | 🏷 `backup`. Means this metric comes from the backup tool |
| os_name | 🏷️ Name of the operating system |
| machine_id | 🏷 Server identifier (the content of `/etc/machine-id` in Linux)
| backup_version | 🏷️ Version of the backup tool that was used |
| manticore_version | 🏷️ Version of Manticore |
| columnar_version | 🏷️ Version of Columnar lib if it is installed |
| secondary_version | 🏷️ Version of the secondary lib if the Columnar lib is installed |
| invocation | Sent when backup was invoked  |
| failed | Sent in the event of a failed backup |
| done | Sent when the backup/restore was successful |
| arg_* | The arguments used to run the tool (excluding index names, etc.) |
| backup_store_versions_fails | Indicates a failure to save the Manticore version in the backup |
| backup_table_count | The total number of backed up tables |
| backup_no_permissions | Failed to back up due to insufficient permissions to the destination directory |
| backup_total_size | The total size of the full backup |
| backup_time | The duration of the backup |
| restore_searchd_running | Failed to run the restore process due to searchd already being running |
| restore_no_config_file | No config file found in the backup during restore |
| restore_time | The duration of the restore |
| fsync_time | The duration of the fsync |
| restore_target_exists | Occurs when there is already a folder or index in the destination folder to restore to |
| terminations | Indicates that the process was terminated |
| signal_* | The signal used to terminate the process |
| tables | The number of tables in Manticore |
| config_unreachable | The specified configuration file does not exist |
| config_data_dir_missing | Failed to parse the `data_dir` from the specified configuration file |
| config_data_dir_is_relative | The `data_dir` path in the configuration file of the Manticore instance is relative |
