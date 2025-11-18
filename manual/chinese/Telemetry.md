# 遥测

在 Manticore，我们收集各种匿名指标以提升我们产品的质量，包括 Manticore Search。通过分析这些数据，我们不仅可以改进产品的整体性能，还能识别出哪些功能最值得优先开发，从而为用户提供更多价值。遥测系统在一个独立线程中以非阻塞模式运行，每隔几分钟采集快照并发送。

我们非常重视您的隐私，您可以放心，所有指标都是完全匿名的，不会传输任何敏感信息。不过，如果您仍希望禁用遥测，可以通过以下方式实现：
* 设置环境变量 `TELEMETRY=0`
* 或在配置文件的 `searchd` 部分设置 `telemetry = 0`

以下是我们收集的所有指标列表：

⏱️ 符号表示该指标是周期性收集的，而其他指标则是基于特定事件收集的。

| 指标 | 描述 |
|-|-|
| `invocation` | 当 Manticore Buddy 启动时发送 |
| `plugin_*` | 表示执行了指定名称的插件，例如 `plugin_backup` 表示执行了备份插件 |
| `command_*` | ⏱️ 所有以此前缀开头的指标均来自 Manticore 守护进程的 `show status` 查询 |
| `uptime` | ⏱️ Manticore Search 守护进程的运行时间 |
| `workers_total` | ⏱️ Manticore 使用的工作线程数 |
| `cluster_count` | ⏱️ 此节点处理的集群数量 |
| `cluster_size` | ⏱️ 所有集群中的节点总数 |
| `table_*_count` | ⏱️ 按类型统计创建的表数量：plain、percolate、rt 或 distributed |
| `*_field_*_count` | ⏱️ rt 和 percolate 类型表中各字段类型的计数 |
| `columnar` | ⏱️ 表示使用了 Columnar 库 |
| `columnar_field_count` | ⏱️ 使用 Columnar 库的字段数量 |

## 备份指标

Manticore 备份工具默认会向 Manticore 指标服务器发送匿名指标，以帮助改进产品。如果您不想发送遥测数据，可以通过添加 `--disable-metric` 参数运行工具，或设置环境变量 `TELEMETRY=0` 来禁用。

以下是所有收集的指标列表：

| 指标 | 描述 |
|-|-|
| `invocation` | 备份启动时发送 |
| `failed` | 备份失败时发送 |
| `done` | 备份/恢复成功时发送 |
| `arg_*` | 运行工具时使用的参数（不包括索引名称等） |
| `backup_store_versions_fails` | 备份中保存 Manticore 版本失败的指标 |
| `backup_table_count` | 备份的表总数 |
| `backup_no_permissions` | 由于目标目录权限不足导致备份失败 |
| `backup_total_size` | 完整备份的总大小 |
| `backup_time` | 备份耗时 |
| `restore_searchd_running` | 恢复失败，因 searchd 已在运行 |
| `restore_no_config_file` | 恢复时备份中未找到配置文件 |
| `restore_time` | 恢复耗时 |
| `fsync_time` | fsync 耗时 |
| `restore_target_exists` | 恢复目标文件夹或索引已存在 |
| `terminations` | 进程被终止的指标 |
| `signal_*` | 用于终止进程的信号 |
| `tables` | Manticore 中的表数量 |
| `config_unreachable` | 指定的配置文件不存在 |
| `config_data_dir_missing` | 无法从指定配置文件解析 `data_dir` |
| `config_data_dir_is_relative` | Manticore 实例配置文件中的 `data_dir` 路径为相对路径 |

## 标签

每个指标都带有以下标签：

| 标签 | 描述 |
|-|-|
| `collector` | `buddy`。表示该指标通过 Manticore Buddy 收集 |
| `os_name` | 操作系统名称 |
| `os_release_name` | 来自 `/etc/os-release` 的名称（如果存在），否则为 `unknown` |
| `os_release_version` | 来自 `/etc/os-release` 的版本（如果存在），否则为 `unknown` |
| `dockerized` | 是否在 Docker 环境中运行 |
| `official_docker` | 如果是 Docker，表示使用的是官方镜像 |
| `machine_id` | 服务器标识符（Linux 中 `/etc/machine-id` 的内容） |
| `arch` | 运行机器的架构 |
| `manticore_version` | Manticore 版本 |
| `columnar_version` | 如果安装了 Columnar 库，则为其版本 |
| `secondary_version` | 如果安装了 Columnar 库，则为 secondary 库版本 |
| `knn_version` | 如果安装了 Columnar 库，则为 KNN 库版本 |
| `buddy_version`| Manticore Buddy 版本 |

<!-- proofread -->

