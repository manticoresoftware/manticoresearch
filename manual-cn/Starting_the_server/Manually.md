# 手动启动 Manticore

你也可以通过直接调用 `searchd`（Manticore Search 服务器二进制文件）来启动 Manticore Search：

```shell
searchd [OPTIONS]
```

请注意，如果未指定配置文件的路径，`searchd` 将尝试在多个位置查找它，具体取决于操作系统。


## searchd 命令行选项

`searchd` 在所有操作系统中可用的选项包括：

- `--help`（简写为 `-h`）：列出可以在 `searchd` 特定构建中使用的所有参数。
- `--version`（简写为 `-v`）：显示 Manticore Search 的版本信息。
- `--config <file>`（简写为 `-c <file>`）：告诉 `searchd` 使用指定的文件作为其配置文件。
- `--stop`：用于异步停止 `searchd`，使用配置文件中指定的 PID 文件的详细信息。因此，你可能还需要使用 `--config` 选项确认 `searchd` 应使用哪个配置文件。例如：

```bash
$ searchd --config /etc/manticoresearch/manticore.conf --stop
```

* `--stopwait`：用于同步停止 `searchd`。`--stop` 基本上是告诉正在运行的实例退出（通过向其发送 SIGTERM），然后立即返回。`--stopwait` 还将尝试等待正在运行的 `searchd` 实例实际完成关闭（例如保存所有挂起的属性更改）并退出。例如：

  ```bash
  $ searchd --config /etc/manticoresearch/manticore.conf --stopwait
  ```

返回的退出码如下：

- 0 表示成功
- 1 表示连接到正在运行的 searchd 服务器失败
- 2 表示服务器在关闭期间报告了错误
- 3 表示服务器在关闭期间崩溃
- `--status`：用于查询正在运行的 `searchd` 实例状态，使用（可选提供的）配置文件中的连接详细信息。它将尝试使用配置文件中的第一个找到的 UNIX 套接字或 TCP 端口连接到正在运行的实例。成功后，它将查询一些状态和性能计数器值并打印出来。你也可以使用 [SHOW STATUS](../Node_info_and_management/Node_status.md#SHOW-STATUS) 命令通过 SQL 协议访问这些计数器。例如：

```bash
$ searchd --status
$ searchd --config /etc/manticoresearch/manticore.conf --status
```

* `--pidfile`：用于强制使用 PID 文件（存储 `searchd` 进程 ID），尽管其他调试选项可能说不应使用它（例如 `--console`）。这是一个调试选项。

  ```bash
  $ searchd --console --pidfile
  ```

* `--console`：用于强制 `searchd` 进入控制台模式。通常，Manticore 作为传统的服务器应用程序运行，并将信息记录到日志文件中（如配置文件中指定）。然而，当调试配置或服务器本身的问题，或尝试诊断难以追踪的问题时，将其信息直接转储到控制台/命令行可能会更容易。在控制台模式下运行还意味着该进程不会被分叉（因此查询是按顺序执行的），并且不会写入日志文件。（需要注意的是，控制台模式并不是运行 `searchd` 的预期方式。）可以这样调用它：

  ```bash
  $ searchd --config /etc/manticoresearch/manticore.conf --console
  ```

* `--logdebug`、`--logreplication`、`--logdebugv` 和 `--logdebugvv` 选项在服务器日志中启用额外的调试输出。它们的区别在于日志记录的详细级别。这些是调试选项，通常不应启用，因为它们可能会污染日志。它们可以在复杂的调试会话中临时使用。

* `--iostats`：与日志记录选项结合使用（必须在 `manticore.conf` 中启用了 `query_log`），以提供有关查询期间执行的输入/输出操作的更多详细信息，尽管性能会有所影响，日志也会稍微大一些。IO 统计信息不包括属性的 IO 操作信息，因为它们是通过 mmap 加载的。可以按以下方式启用它：

  ```bash
  $ searchd --config /etc/manticoresearch/manticore.conf --iostats
  ```

* `--cpustats`：用于提供实际的 CPU 时间报告（除了墙上时间），可以在查询日志文件中（对于每个查询）和状态报告中（聚合）。它依赖于 Linux 系统调用 `clock_gettime()` 或在某些系统上退回到不太精确的调用。你可以这样启动 `searchd`：

  ```bash
  $ searchd --config /etc/manticoresearch/manticore.conf --cpustats
  ```

* `--port portnumber`（简写为 `-p`）：用于指定 Manticore 应该监听哪个端口以接受二进制协议请求，通常用于调试目的。这个端口通常默认为 9312，但有时你需要在不同的端口上运行它。在命令行上指定该选项将覆盖配置文件中指定的任何内容。有效的范围是 0 到 65535，但编号为 1024 及以下的端口通常需要特权帐户才能运行。

  使用示例：

  ```bash
  $ searchd --port 9313
  ```

* `--listen ( address ":" port | port | path ) [ ":" protocol ]`（简写为 `-l`）：类似于 `--port`，但允许你不仅指定端口，还可以指定完整的路径、IP 地址和端口，或者 Unix 域套接字路径，供 `searchd` 监听。换句话说，你可以指定 IP 地址（或主机名）和端口号，或仅指定端口号，或者指定 Unix 套接字路径。如果你仅指定端口号但未指定地址，`searchd` 将监听所有网络接口。通过前导斜杠可以识别 Unix 路径。作为最后一个参数，你还可以指定用于此套接字连接的协议处理器。支持的协议值为 `sphinx` 和 `mysql`（自 4.1 以来使用 MySQL 协议）。

* `--force-preread`：禁止服务器在预读完表文件之前处理任何传入连接。默认情况下，服务器启动时会在懒加载表文件的同时接受连接。此选项扩展了该行为，使其在文件加载完毕之前保持等待。

* `--index (--table) <table>`（简写为 `-i (-t) <table>`）：强制此 `searchd` 实例仅提供指定的表服务。类似于上面的 `--port`，通常用于调试；更长期的更改通常会应用到配置文件本身。

* `--strip-path`：从表引用的所有文件名中剥离路径名（如停用词、词形表、例外处理等）。这在拾取在另一台机器上构建且可能有不同路径布局的表时很有用。

* `--replay-flags=<OPTIONS>`：该开关可用于指定一组额外的二进制日志重放选项。支持的选项有：

    * `accept-desc-timestamp`：忽略降序事务时间戳，并仍然重放此类事务（默认行为是返回错误并退出）。
    * `ignore-open-errors`：忽略缺失的 binlog 文件（默认行为是返回错误并退出）。
    * `ignore-trx-errors`：忽略任何事务错误并跳过当前 binlog 文件（默认行为是返回错误并退出）。
    * `ignore-all-errors`：忽略上述所有错误（默认行为是返回错误并退出）。

    示例：

    ```bash
    $ searchd --replay-flags=accept-desc-timestamp
    ```

* `--coredump`：启用在服务器崩溃时保存核心文件或最小转储。默认情况下禁用，以加快崩溃时服务器的重启速度。此选项对调试有用。

  ```bash
  $ searchd --config /etc/manticoresearch/manticore.conf --coredump
  ```

* `--new-cluster`：启动时引导一个复制集群，并使服务器成为参考节点，带有[集群重启](../Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md)保护。在 Linux 上，你还可以运行 `manticore_new_cluster`。它将通过 systemd 启动 Manticore 并处于 `--new-cluster` 模式。

* `--new-cluster-force`：启动时引导一个复制集群，并在绕过[集群重启](../Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md)保护的情况下使服务器成为参考节点。在 Linux 上，你还可以运行 `manticore_new_cluster --force`。它将通过 systemd 启动 Manticore 并处于 `--new-cluster-force` 模式。

### Windows 选项

在 Windows 平台上，`searchd` 有一些特定于 Windows 的选项，用于处理服务，并且仅在 Windows 二进制文件中可用。

请注意，在 Windows 中，除非你将其安装为服务，否则 `searchd` 默认处于 `--console` 模式。

* `--install`：将 `searchd` 作为服务安装到 Microsoft 管理控制台（控制面板/管理工具/服务）中。在命令行中指定 `--install` 时提供的任何其他参数，也会成为未来启动该服务时的命令行参数。例如，作为调用 `searchd` 的一部分，你可能还需要使用 `--config` 指定配置文件，并且你会在指定 `--install` 的同时这样做。调用后，可以通过管理控制台使用常规启动/停止设施，因此可以应用任何适用于启动、停止和重新启动服务的方法。例如：

  ```bat
  C:\WINDOWS\system32> C:\Manticore\bin\searchd.exe --install
     --config C:\Manticore\manticore.conf
  ```

  如果你希望每次启动 `searchd` 时都启用 I/O 统计信息，则需要将该选项与 `--install` 命令放在同一行上，如下所示：

  ```bat
  C:\WINDOWS\system32> C:\Manticore\bin\searchd.exe --install
     --config C:\Manticore\manticore.conf --iostats
  ```
* `--delete`：从 Microsoft 管理控制台和其他服务注册位置移除服务，前提是之前已使用 `--install` 安装了该服务。请注意，这不会卸载软件或删除表文件。这意味着该服务不会再从服务系统中调用，也不会在机器下次启动时启动。如果当前作为服务运行，则当前实例不会被终止（直到下次重启或使用 `--stop`）。如果服务是使用自定义名称安装的（使用 `--servicename`），则在调用删除服务时需要使用 `--servicename` 指定相同的名称。例如：

  ```bat
  C:\WINDOWS\system32> C:\Manticore\bin\searchd.exe --delete
  ```

* `--servicename <name>`：为 `searchd` 指定名称，在安装或删除服务时将其用作服务在管理控制台中的显示名称；默认名称为 `searchd`，但如果你在管理员较多的服务器上或多个 `searchd` 实例共存的系统上部署，可能需要更具描述性的名称。请注意，除非与 `--install` 或 `--delete` 结合使用，否则该选项无效。例如：
  
  ```bat
  C:\WINDOWS\system32> C:\Manticore\bin\searchd.exe --install
     --config C:\Manticore\manticore.conf --servicename ManticoreSearch
  ```
  
* `--ntservice`：这是由 Microsoft 管理控制台传递给 `searchd` 的选项，用于在 Windows 平台上以服务形式调用它。通常不需要直接调用此选项；这通常由 Windows 在服务启动时调用，尽管如果你想从命令行像常规服务一样调用它（作为 `--console` 的补充），理论上也可以这样做。

* `--safetrace`：强制 `searchd` 仅使用系统的 `backtrace()` 调用生成崩溃报告。在某些（罕见）情况下，这可能是生成报告的“更安全”方式。这是一个调试选项。

* `--nodetach`（仅限 Linux）：告诉 `searchd` 不要脱离后台。这也将导致日志条目打印到控制台。查询处理会照常进行。这是一个调试选项，在你运行 Manticore 容器时捕获其输出时也可能有用。


## 插件目录

Manticore 使用 [plugin_dir](../Server_settings/Common.md#plugin_dir) 存储和使用 Manticore Buddy 插件。在标准安装中，该值默认对用户 "manticore" 可访问。然而，如果你以不同用户手动启动 searchd 守护进程，守护进程可能无法访问 `plugin_dir`。为了解决这个问题，请确保在公共部分指定一个运行 searchd 守护进程的用户可写入的 `plugin_dir`。

## 信号

`searchd` 支持多个信号：

- `SIGTERM` - 发起干净的关闭操作。不处理新查询，但不会强制中断已经开始的查询。
- `SIGHUP` - 发起表旋转操作。根据 [seamless_rotate](../Server_settings/Searchd.md#seamless_rotate) 设置的值，新查询可能会短暂停滞；客户端将收到临时错误。
- `SIGUSR1` - 强制重新打开 searchd 日志和查询日志文件，允许日志文件旋转。

## 环境变量

- `MANTICORE_TRACK_DAEMON_SHUTDOWN=1` 启用详细日志记录，记录 searchd 关闭过程。它在 Manticore 花费太长时间关闭或关闭过程中冻结的情况下非常有用。

<!-- proofread -->
