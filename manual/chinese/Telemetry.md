# 远程监控

在 Manticore，我们收集各种匿名指标，以提高我们产品的质量，包括 Manticore Search。通过分析这些数据，我们不仅可以改善产品的整体性能，还可以确定哪些功能最值得优先考虑，以便为用户提供更多价值。遥测系统在非阻塞模式下在单独的线程上运行，每几分钟拍摄一次快照并发送。

我们非常重视您的隐私，您可以放心，所有指标都是完全匿名的，并且没有传输任何敏感信息。但是，如果您仍然希望禁用遥测，您可以通过以下方式进行设置：
* 设置环境变量 `TELEMETRY=0`
* 或在您的配置文件的 `searchd` 部分设置 `telemetry = 0`

以下是我们收集的所有指标列表：

⏱️ 符号表示该指标是定期收集的，与基于特定事件收集的其他指标不同。

| 指标 | 描述 |
|-|-|
| `invocation` | 当 Manticore Buddy 启动时发送 |
| `plugin_*` | 表示已执行给定名称的插件，例如 `plugin_backup` 表示备份执行 |
| `command_*` | ⏱️ 所有带有此前缀的指标来自 Manticore 守护进程的 `show status` 查询 |
| `uptime` | ⏱️ Manticore Search 守护进程的正常运行时间 |
| `workers_total` | ⏱️ Manticore 使用的工作线程数量 |
| `cluster_count` | ⏱️ 此节点处理的集群数量 |
| `cluster_size` | ⏱️ 所有集群中的节点总数 |
| `table_*_count` | ⏱️ 为每种类型创建的表的数量：普通、检索、实时或分布式 |
| `*_field_*_count` | ⏱️ 对于具有实时和检索类型的表，每种字段类型的计数 |
| `columnar` | ⏱️ 指示使用了 Columnar 库 |
| `columnar_field_count` | ⏱️ 使用 Columnar 库的字段数量 |

## 备份指标

Manticore 备份工具默认情况下会向 Manticore 指标服务器发送匿名指标，以帮助改进产品。如果您不想发送遥测，可以通过使用 `--disable-metric` 标志运行工具或设置环境变量 `TELEMETRY=0` 来禁用它。

以下是所有收集的指标列表：

| 指标 | 描述 |
|-|-|
| `invocation` | 当备份被启动时发送 |
| `failed` | 在备份失败的情况下发送 |
| `done` | 当备份/恢复成功时发送 |
| `arg_*` | 用于运行工具的参数（不包括索引名称等） |
| `backup_store_versions_fails` | 指示在备份中保存 Manticore 版本失败 |
| `backup_table_count` | 备份的表的总数量 |
| `backup_no_permissions` | 由于目标目录权限不足，备份失败 |
| `backup_total_size` | 完整备份的总大小 |
| `backup_time` | 备份的持续时间 |
| `restore_searchd_running` | 由于 searchd 已在运行，恢复过程失败 |
| `restore_no_config_file` | 在恢复过程中未找到备份的配置文件 |
| `restore_time` | 恢复的持续时间 |
| `fsync_time` | fsync 的持续时间 |
| `restore_target_exists` | 当目标文件夹或索引在恢复的目标文件夹中已经存在时发生 |
| `terminations` | 表示该进程被终止 |
| `signal_*` | 用于终止进程的信号 |
| `tables` | Manticore 中的表数量 |
| `config_unreachable` | 指定的配置文件不存在 |
| `config_data_dir_missing` | 无法从指定配置文件解析 `data_dir` |
| `config_data_dir_is_relative` | Manticore 实例配置文件中的 `data_dir` 路径是相对的 |

## 标签

每个指标都带有以下标签：

| 标签 | 描述 |
|-|-|
| `collector` | `buddy`。表示此指标是通过 Manticore Buddy 收集的 |
| `os_name` | 操作系统名称 |
| `os_release_name` | 来自 `/etc/os-release` 的名称（如果存在）或 `unknown` |
| `os_release_version` | 来自 `/etc/os-release` 的版本（如果存在）或 `unknown` |
| `dockerized` | 如果它在 Docker 环境中运行 |
| `official_docker` | 如果是 Docker，则表示我们使用的是官方映像的标志 |
| `machine_id` | 服务器标识符（Linux 中的 `/etc/machine-id` 内容） |
| `arch` | 我们运行的机器的架构 |
| `manticore_version` | Manticore 的版本 |
| `columnar_version` | 如果已安装 Columnar 库，则为其版本 |
| `secondary_version` | 如果安装了 Columnar 库，则为次级库的版本 |
| `knn_version` | 如果安装了 Columnar 库，则为 KNN 库的版本 |
| `buddy_version`| Manticore Buddy 的版本 |

<!-- proofread -->
