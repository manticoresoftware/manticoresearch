# 遥测

在 Manticore，我们收集各种匿名指标以提升我们产品的质量，包括 Manticore Search。通过分析这些数据，我们不仅可以改善产品的整体性能，还能识别出哪些功能最值得优先开发，从而为用户提供更多价值。遥测系统在一个独立的线程上以非阻塞模式运行，每隔几分钟采集快照并发送。

我们非常重视您的隐私，您可以放心，所有指标都是完全匿名的，不会传输任何敏感信息。不过，如果您仍希望禁用遥测，可以通过以下方式实现：
* 设置环境变量 `TELEMETRY=0`
* 或在配置文件的 `searchd` 部分设置 `telemetry = 0`

以下是我们收集的所有指标列表：

⏱️ 符号表示该指标为周期性收集，其他指标则是基于特定事件收集。

| 指标 | 描述 |
|-|-|
| `invocation` | Manticore Buddy 启动时发送 |
| `plugin_*` | 表示执行了指定名称的插件，例如 `plugin_backup` 表示备份插件执行 |
| `command_*` | ⏱️ 所有此前缀的指标均来自 Manticore 守护进程的 `show status` 查询 |
| `uptime` | ⏱️ Manticore Search 守护进程运行时间 |
| `workers_total` | ⏱️ Manticore 使用的工作线程数 |
| `cluster_count` | ⏱️ 该节点处理的集群数量 |
| `cluster_size` | ⏱️ 所有集群中节点的总数 |
| `table_*_count` | ⏱️ 各类型表（plain、percolate、rt、distributed）创建的数量 |
| `*_field_*_count` | ⏱️ rt 和 percolate 类型表中各字段类型的数量 |
| `columnar` | ⏱️ 表示使用了 Columnar 库 |
| `columnar_field_count` | ⏱️ 使用 Columnar 库的字段数 |

## 备份指标

Manticore 备份工具默认向 Manticore 指标服务器发送匿名指标，以助于产品改进。如果您不想发送遥测，可通过加上 `--disable-metric` 参数运行工具或设置环境变量 `TELEMETRY=0` 来禁用。

以下是所有收集的指标列表：

| 指标 | 描述 |
|-|-|
| `invocation` | 备份启动时发送 |
| `failed` | 备份失败时发送 |
| `done` | 备份/恢复成功时发送 |
| `arg_*` | 工具执行时使用的参数（不包括索引名等） |
| `backup_store_versions_fails` | 保存备份中的 Manticore 版本失败 |
| `backup_table_count` | 备份的表总数 |
| `backup_no_permissions` | 由于目标目录权限不足导致的备份失败 |
| `backup_total_size` | 完整备份的总大小 |
| `backup_time` | 备份所用时间 |
| `restore_searchd_running` | 由于 searchd 正在运行，恢复进程启动失败 |
| `restore_no_config_file` | 恢复时备份中未找到配置文件 |
| `restore_time` | 恢复所用时间 |
| `fsync_time` | fsync 操作所用时长 |
| `restore_target_exists` | 目标路径中已存在文件夹或索引，导致恢复失败 |
| `terminations` | 表示进程被终止 |
| `signal_*` | 导致进程终止的信号 |
| `tables` | Manticore 中的表数量 |
| `config_unreachable` | 指定的配置文件不存在 |
| `config_data_dir_missing` | 解析指定配置文件中的 `data_dir` 失败 |
| `config_data_dir_is_relative` | Manticore 实例配置文件中的 `data_dir` 路径是相对路径 |

## 标签

每个指标都带有以下标签：

| 标签 | 描述 |
|-|-|
| `collector` | `buddy`。表示该指标由 Manticore Buddy 收集 |
| `os_name` | 操作系统名称 |
| `os_release_name` | 取自 `/etc/os-release` 的名称（如果存在），否则为 `unknown` |
| `os_release_version` | 取自 `/etc/os-release` 的版本（如果存在），否则为 `unknown` |
| `dockerized` | 是否运行在 Docker 环境中 |
| `official_docker` | 如果是 Docker，表示是否使用官方镜像 |
| `machine_id` | 服务器标识符（Linux 下 `/etc/machine-id` 的内容） |
| `arch` | 运行主机的架构 |
| `manticore_version` | Manticore 版本 |
| `columnar_version` | 如果安装了 Columnar 库则为该库版本 |
| `secondary_version` | 如果安装了 Columnar 库则为二级库版本 |
| `knn_version` | 如果安装了 Columnar 库则为 KNN 库版本 |
| `buddy_version`| Manticore Buddy 版本 |

<!-- proofread -->

