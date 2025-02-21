# 远程监控

在 Manticore，我们收集各种匿名指标，以提高 Manticore Search 等产品的质量。通过分析这些数据，我们不仅可以提升产品的整体性能，还可以确定优先开发哪些功能，从而为用户提供更多价值。遥测系统在一个独立的线程上运行，以非阻塞的方式定期拍摄快照，并每隔几分钟发送一次。

我们非常重视您的隐私，所有收集的指标都是完全匿名的，且不会传输任何敏感信息。然而，如果您仍希望禁用遥测功能，您可以通过以下方式实现：

- 设置环境变量 `TELEMETRY=0`
- 或者在配置文件的 `searchd` 部分中设置 `telemetry = 0`

以下是我们收集的所有指标列表：

⏱️ 符号表示此指标是定期收集的，而其他指标基于特定事件进行收集。

| 指标                   | 描述                                                         |
| ---------------------- | ------------------------------------------------------------ |
| `invocation`           | Manticore Buddy 启动时发送                                   |
| `plugin_*`             | 表示运行了给定名称的插件，例如备份执行时的 `plugin_backup`   |
| `command_*`            | ⏱️ 所有带有此前缀的指标都来自 Manticore 守护进程的 `show status` 查询 |
| `uptime`               | ⏱️ Manticore Search 守护进程的正常运行时间                    |
| `workers_total`        | ⏱️ Manticore 使用的工作线程数量                               |
| `cluster_count`        | ⏱️ 此节点管理的集群数量                                       |
| `cluster_size`         | ⏱️ 所有集群中的节点数量                                       |
| `table_*_count`        | ⏱️ 每种类型创建的表的数量：普通表、渗透表、实时表或分布式表   |
| `*_field_*_count`      | ⏱️ 实时表和渗透表中每种字段类型的数量                         |
| `columnar`             | ⏱️ 表示使用了列式库                                           |
| `columnar_field_count` | ⏱️ 使用列式库的字段数量                                       |

## 备份指标

Manticore 备份工具会默认发送匿名的指标到 Manticore 服务器，以帮助改进产品。如果您不希望发送遥测数据，可以通过 `--disable-metric` 标志禁用，或通过设置环境变量 `TELEMETRY=0`。

以下是收集的所有备份指标：

| 指标                          | 描述                                                 |
| ----------------------------- | ---------------------------------------------------- |
| `invocation`                  | 备份启动时发送                                       |
| `failed`                      | 备份失败时发送                                       |
| `done`                        | 备份/恢复成功时发送                                  |
| `arg_*`                       | 运行工具时使用的参数（不包括索引名称等）             |
| `backup_store_versions_fails` | 保存 Manticore 版本信息到备份时失败                  |
| `backup_table_count`          | 备份表的总数                                         |
| `backup_no_permissions`       | 备份失败，原因是没有访问目标目录的权限               |
| `backup_total_size`           | 完整备份的总大小                                     |
| `backup_time`                 | 备份持续时间                                         |
| `restore_searchd_running`     | 恢复进程失败，原因是 searchd 已在运行                |
| `restore_no_config_file`      | 恢复过程中没有找到备份中的配置文件                   |
| `restore_time`                | 恢复持续时间                                         |
| `fsync_time`                  | fsync 持续时间                                       |
| `restore_target_exists`       | 发生目标文件夹或索引已存在的情况                     |
| `terminations`                | 表示进程被终止                                       |
| `signal_*`                    | 终止进程时使用的信号                                 |
| `tables`                      | Manticore 中的表数                                   |
| `config_unreachable`          | 指定的配置文件不存在                                 |
| `config_data_dir_missing`     | 无法从指定的配置文件中解析 `data_dir`                |
| `config_data_dir_is_relative` | Manticore 实例配置文件中的 `data_dir` 路径是相对路径 |

## 标签

每个指标都附带以下标签：

| 标签                 | 描述                                                     |
| -------------------- | -------------------------------------------------------- |
| `collector`          | `buddy`。表示此指标通过 Manticore Buddy 收集             |
| `os_name`            | 操作系统名称                                             |
| `os_release_name`    | `/etc/os-release` 中的名称（如果存在）或 `unknown`       |
| `os_release_version` | `/etc/os-release` 中的版本（如果存在）或 `unknown`       |
| `dockerized`         | 是否在 Docker 环境中运行                                 |
| `official_docker`    | 如果是在 Docker 环境中运行的，标志表示是否使用了官方镜像 |
| `machine_id`         | 服务器标识符（Linux 系统中 `/etc/machine-id` 的内容）    |
| `arch`               | 运行机器的架构                                           |
| `manticore_version`  | Manticore 的版本                                         |
| `columnar_version`   | 列式库的版本（如果安装了列式库）                         |
| `secondary_version`  | 二级库的版本（如果安装了列式库）                         |
| `knn_version`        | KNN 库的版本（如果安装了列式库）                         |
| `buddy_version`      | Manticore Buddy 的版本                                   |

<!-- proofread -->
