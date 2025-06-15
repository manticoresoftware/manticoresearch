# 远程数据收集

在 Manticore，我们收集各种匿名指标以提升我们产品的质量，包括 Manticore Search。通过分析这些数据，我们不仅可以改进产品的整体性能，还能识别哪些功能最值得优先开发，以便为用户提供更多价值。远程数据收集系统在单独的线程上以非阻塞模式运行，每隔几分钟拍摄一次快照并发送。

我们非常重视您的隐私，您可以放心所有指标均为完全匿名，且不传输任何敏感信息。然而，如果您仍希望禁用远程数据收集，可以通过以下方式实现：
* 设置环境变量 `TELEMETRY=0`
* 或在配置文件的 `searchd` 部分设置 `telemetry = 0`

以下是我们收集的所有指标列表：

⏱️ 符号表示该指标是定期收集的，而其他指标则是基于特定事件收集的。

| 指标 | 描述 |
|-|-|
| `invocation` | 当 Manticore Buddy 启动时发送 |
| `plugin_*` | 表示执行了指定名称的插件，例如 `plugin_backup` 表示备份插件执行 |
| `command_*` | ⏱️ 所有该前缀的指标均通过 Manticore 守护进程的 `show status` 查询发送 |
| `uptime` | ⏱️ Manticore Search 守护进程的运行时间 |
| `workers_total` | ⏱️ Manticore 使用的工作线程数量 |
| `cluster_count` | ⏱️ 该节点处理的集群数量 |
| `cluster_size` | ⏱️ 所有集群中的节点总数 |
| `table_*_count` | ⏱️ 为每种类型创建的表数量：plain, percolate, rt, 或 distributed |
| `*_field_*_count` | ⏱️ rt 和 percolate 类型表中每种字段类型的数量 |
| `columnar` | ⏱️ 表示使用了 Columnar 库 |
| `columnar_field_count` | ⏱️ 使用 Columnar 库的字段数量 |

## 备份指标

Manticore 备份工具默认会向 Manticore 指标服务器发送匿名指标，以帮助改善产品。如果您不想发送远程数据，可以使用 `--disable-metric` 参数运行该工具，或设置环境变量 `TELEMETRY=0` 来禁用。

以下为所有收集的指标列表：

| 指标 | 描述 |
|-|-|
| `invocation` | 当备份启动时发送 |
| `failed` | 备份失败时发送 |
| `done` | 备份/恢复成功时发送 |
| `arg_*` | 运行工具时使用的参数（不包括索引名等） |
| `backup_store_versions_fails` | 表示备份过程中保存 Manticore 版本失败 |
| `backup_table_count` | 备份的表总数 |
| `backup_no_permissions` | 因目标目录权限不足导致的备份失败 |
| `backup_total_size` | 完整备份的总大小 |
| `backup_time` | 备份耗时 |
| `restore_searchd_running` | 恢复进程启动失败，因为 searchd 已经在运行 |
| `restore_no_config_file` | 恢复时未在备份中找到配置文件 |
| `restore_time` | 恢复耗时 |
| `fsync_time` | fsync 耗时 |
| `restore_target_exists` | 恢复目标文件夹或索引已存在时发生 |
| `terminations` | 表示进程被终止 |
| `signal_*` | 用于终止进程的信号 |
| `tables` | Manticore 中的表数量 |
| `config_unreachable` | 指定的配置文件不存在 |
| `config_data_dir_missing` | 无法从指定配置文件解析 `data_dir` |
| `config_data_dir_is_relative` | Manticore 实例配置文件中的 `data_dir` 路径为相对路径 |

## 标签

每个指标包含以下标签：

| 标签 | 描述 |
|-|-|
| `collector` | `buddy`。表示该指标是通过 Manticore Buddy 收集的 |
| `os_name` | 操作系统名称 |
| `os_release_name` | 来自 `/etc/os-release` 的名称（如果存在），否则为 `unknown` |
| `os_release_version` | 来自 `/etc/os-release` 的版本号（如果存在），否则为 `unknown` |
| `dockerized` | 是否运行在 Docker 环境中 |
| `official_docker` | 如果是 Docker，表明使用的是官方镜像 |
| `machine_id` | 服务器标识符（Linux 中 `/etc/machine-id` 的内容） |
| `arch` | 运行机器的架构 |
| `manticore_version` | Manticore 版本 |
| `columnar_version` | 安装了 Columnar 库时的版本 |
| `secondary_version` | 安装了 Columnar 库时 secondary 库的版本 |
| `knn_version` | 安装了 Columnar 库时 KNN 库的版本 |
| `buddy_version` | Manticore Buddy 版本 |

<!-- proofread -->

