# 手动启动 Manticore

您还可以通过直接调用 `searchd`（Manticore Search 服务器二进制文件）来启动 Manticore Search：

```shell
searchd [OPTIONS]
```

请注意，如果未指定配置文件的路径，`searchd` 会尝试根据操作系统在多个位置查找该文件。


## searchd 命令行选项


在所有操作系统中，`searchd` 可用的选项如下：

* `--help` (`-h` 是简写) 列出您特定版本的 `searchd` 中可以使用的所有参数。
* `--version` (`-v` 是简写) 显示 Manticore Search 版本信息。
* `--config <file>` (`-c <file>` 是简写) 告诉 `searchd` 使用指定的文件作为其配置。
* `--stop` 用于异步停止 `searchd`，使用 Manticore 配置文件中指定的 PID 文件的详细信息。因此，您可能还需要通过 `--config` 选项确认要使用的配置文件。示例：

  ```bash
  $ searchd --config /etc/manticoresearch/manticore.conf --stop
  ```

* `--stopwait` 用于同步停止 `searchd`。`--stop` 本质上是告诉正在运行的实例退出（通过发送 SIGTERM），然后立即返回。`--stopwait` 还将尝试等待正在运行的 `searchd` 实例实际完成关机（例如，保存所有待处理的属性更改）并退出。示例：

  ```bash
  $ searchd --config /etc/manticoresearch/manticore.conf --stopwait
  ```
  可能的退出代码如下：
    * 0 表示成功
    * 1 表示连接到正在运行的 searchd 服务器失败
    * 2 表示服务器在关机期间报告错误
    * 3 表示服务器在关机期间崩溃

* `--status` 命令用于使用（可选）提供的配置文件中的连接详细信息查询正在运行的 `searchd` 实例状态。它将尝试使用从配置文件中找到的第一个 UNIX 套接字或 TCP 端口连接到正在运行的实例。如果成功，它将查询状态和性能计数值，并打印它们。您还可以使用 [SHOW STATUS](../Node_info_and_management/Node_status.md#SHOW-STATUS) 命令通过 SQL 协议访问相同的计数器。示例：

  ```bash
  $ searchd --status
  $ searchd --config /etc/manticoresearch/manticore.conf --status
  ```

* `--pidfile` 用于强制明确使用 PID 文件（存储 `searchd` 进程标识号的地方），尽管其他调试选项指示相反（例如，`--console`）。这是一种调试选项。

  ```bash
  $ searchd --console --pidfile
  ```

* `--console` 用于强制 `searchd` 进入控制台模式。通常，Manticore 作为常规服务器应用程序运行，并将信息记录到日志文件中（如配置文件中指定）。然而，在调试配置或服务器本身中的问题，或试图诊断难以追踪的问题时，强制它直接将信息输出到调用它的控制台/命令行可能更容易。以控制台模式运行还意味着进程不会被分叉（所以搜索是顺序进行的），日志不会被写入。（应该注意，控制台模式并不是运行 `searchd` 的预期方法。）您可以这样调用它：

  ```bash
  $ searchd --config /etc/manticoresearch/manticore.conf --console
  ```

* `--logdebug`、`--logreplication`、`--logdebugv` 和 `--logdebugvv` 选项启用服务器日志中的额外调试输出。它们通过日志详细程度级别而不同。这些是调试选项，通常不应启用，因为它们可能会大量污染日志。可以根据请求临时使用这些选项以协助复杂的调试会话。

* `--iostats` 与日志选项一起使用（`query_log` 必须在 `manticore.conf` 中激活），以提供更详细的每个查询基础上有关在该查询过程中进行的输入/输出操作的信息，伴随轻微的性能损失和稍大的日志。IO 统计信息不包括关于属性的 IO 操作，因为这些是通过 mmap 加载的。要启用它，您可以如下启动 `searchd`：

  ```bash
  $ searchd --config /etc/manticoresearch/manticore.conf --iostats
  ```

* `--cpustats` 用于在查询日志文件（每个查询）和状态报告（汇总）中提供实际 CPU 时间报告（除墙时间外）。它依赖于 `clock_gettime()` Linux 系统调用，或者在某些系统上回退到精度较低的调用。您可以这样启动 `searchd`：

  ```bash
  $ searchd --config /etc/manticoresearch/manticore.conf --cpustats
  ```

*  `--port portnumber` (`-p` 是简写) 用于指定 Manticore 应该监听的端口，以接受二进制协议请求，通常用于调试目的。这通常默认为 9312，但有时您需要在其他端口上运行它。在命令行中指定将覆盖配置文件中指定的任何内容。有效范围为 0 到 65535，但编号为 1024 及以下的端口通常需要特权账户才能运行。

  使用示例：

  ```bash
  $ searchd --port 9313
  ```
* `--listen ( address ":" port | port | path ) [ ":" protocol ]` (或 `-l` 作为简写) 的功能与 `--port` 相同，但允许您指定不仅仅是端口，而是完整的路径、IP地址和端口，或者 `searchd` 将监听的 Unix 域套接字路径。换句话说，您可以指定一个 IP 地址（或主机名）和端口号，仅指定端口号，或一个 Unix 套接字路径。如果您指定一个端口号但不指定地址，`searchd` 将在所有网络接口上进行监听。Unix 路径以斜杠开头。作为最后一个参数，您还可以指定一个用于此套接字连接的协议处理程序（监听器）。支持的协议值为 'sphinx' 和 'mysql'（自 4.1 起使用的 MySQL 协议）。

* `--force-preread` 禁止服务器在表文件的预读完成之前处理任何传入连接。默认情况下，在启动时，服务器在表文件懒加载到内存时接受连接。这扩展了该行为，并使其在文件加载完成之前保持等待。

* `--index (--table) <table>` (或 `-i (-t) <table>` 作为简写) 强制此实例的 `searchd` 仅提供指定的表服务。就像上面的 `--port`，这通常用于调试目的；更长期的更改通常会应用于配置文件本身。

* `--strip-path` 从表中引用的所有文件名中去掉路径名（停用词、词形、例外等）。这对于获取在可能有不同路径布局的另一台机器上构建的表非常有用。

* `--replay-flags=<OPTIONS>` 开关可用于指定额外的二进制日志重放选项列表。支持的选项包括：
    * `accept-desc-timestamp`，忽略递减的事务时间戳并重放此类事务（默认行为是以错误退出）。
    * `ignore-open-errors`，忽略缺失的 binlog 文件（默认行为是以错误退出）。
    * `ignore-trx-errors`，忽略任何事务错误并跳过当前 binlog 文件（默认行为是以错误退出）。
    * `ignore-all-errors`，忽略上述任何错误（默认行为是以错误退出）。

    示例：

    ```bash
    $ searchd --replay-flags=accept-desc-timestamp
    ```

* `--coredump` 用于在服务器崩溃时启用保存核心文件或小型转储。默认情况下禁用以加快服务器崩溃后的重启速度。这对于调试目的非常有用。

  ```bash
  $ searchd --config /etc/manticoresearch/manticore.conf --coredump
  ```

* `--new-cluster` 启动一个复制集群，并使服务器成为具有 [集群重启](../Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md) 保护的参考节点。在 Linux 上，您还可以运行 `manticore_new_cluster`。它将通过 systemd 启动 Manticore 的 `--new-cluster` 模式。

* `--new-cluster-force` 启动一个复制集群，并使服务器成为绕过 [集群重启](../Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md) 保护的参考节点。在 Linux 上，您还可以运行 `manticore_new_cluster --force`。它将通过 systemd 启动 Manticore 的 `--new-cluster-force` 模式。

* `--mockstack` 分析并报告递归表达式评估、模式匹配操作和过滤处理所需的堆栈大小。此调试选项将计算的堆栈需求输出到控制台以便进行优化。输出提供可以用于配置不同操作的堆栈需求的环境变量。

  示例：
  ```bash
  $ searchd --mockstack
  Manticore 7.4.7 e90b5afbb@25032706 dev (columnar 4.1.2 15bbcc7@25031206) (secondary 4.1.2 15bbcc7@25031206) (knn 4.1.2 15bbcc7@25031206)
  版权所有 (c) 2001-2016, Andrew Aksyonoff
  版权所有 (c) 2008-2016, Sphinx Technologies Inc (http://sphinxsearch.com)
  版权所有 (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)

  export MANTICORE_KNOWN_CREATE_SIZE=200
  export MANTICORE_START_KNOWN_CREATE_SIZE=4504
  export MANTICORE_KNOWN_EXPR_SIZE=16
  export MANTICORE_START_KNOWN_EXPR_SIZE=200
  export MANTICORE_NONE=32
  export MANTICORE_START_NONE=104
  export MANTICORE_KNOWN_FILTER_SIZE=224
  export MANTICORE_START_KNOWN_FILTER_SIZE=11192
  export MANTICORE_KNOWN_MATCH_SIZE=320
  export MANTICORE_START_KNOWN_MATCH_SIZE=14552
  export NO_STACK_CALCULATION=1
  ```

### Windows 选项

有一些特定于 Windows 平台的 `searchd` 选项，涉及作为服务的处理，并且仅在 Windows 二进制文件中可用。

请注意，在 Windows 中，除非您将其作为服务安装，否则 `searchd` 将默认处于 `--console` 模式。

* `--install` 将 `searchd` 安装为 Microsoft 管理控制台（控制面板 / 管理工具 / 服务）中的服务。命令行中指定的任何其他参数，当指定 `--install` 时，也将成为服务未来启动的命令行的一部分。例如，作为调用 `searchd` 的一部分，您可能还需要通过 `--config` 指定配置文件，并且您需要在指定 `--install` 时这样做。一旦调用，通常的启动/停止功能将通过管理控制台变得可用，因此您可以用于启动、停止和重新启动服务的任何方法也适用于 `searchd`。示例：

  ```bat
  C:\WINDOWS\system32> C:\Manticore\bin\searchd.exe --install
     --config C:\Manticore\manticore.conf
  ```

  如果您希望每次启动 `searchd` 时都有 I/O 统计信息，您需要在与 `--install` 命令相同的行上指定该选项，如下所示：

  ```bat
  C:\WINDOWS\system32> C:\Manticore\bin\searchd.exe --install
     --config C:\Manticore\manticore.conf --iostats
  ```
* `--delete` 从 Microsoft Management Console 和其他服务注册位置移除服务，此前已使用 `--install` 安装。请注意，这并不会卸载软件或删除表。它意味着该服务不会从服务系统中调用，并且在下次机器启动时不会启动。如果目前作为服务运行，当前实例将不会被终止（直到下次重启或 `--stop`）。如果服务是使用自定义名称（`--servicename`）安装的，则在调用卸载时需要使用 `--servicename` 指定相同的名称。示例：

  ```bat
  C:\WINDOWS\system32> C:\Manticore\bin\searchd.exe --delete
  ```

* `--servicename <name>` 在安装或删除服务时将给定名称应用于 `searchd`，如在管理控制台中所示；默认情况下为 searchd，但如果在多个管理员可能登录到系统的服务器上，或在具有多个 `searchd` 实例的系统上，可能需要使用更具描述性的名称。请注意，除非与 `--install` 或 `--delete` 组合使用，否则此选项无效。示例：
  ```bat
  C:\WINDOWS\system32> C:\Manticore\bin\searchd.exe --install
     --config C:\Manticore\manticore.conf --servicename ManticoreSearch
  ```

* `--ntservice` 是 Microsoft Management Console 传递给 `searchd` 的一个选项，用于在 Windows 平台上将其作为服务调用。通常不需要直接调用此选项；通常是在服务启动时由 Windows 调用，尽管如果您想将其作为常规服务从命令行调用（作为 `--console` 的补充），在理论上您可以这样做。

* `--safetrace` 强制 `searchd` 在崩溃报告中仅使用系统的 backtrace() 调用。在某些（罕见）场景下，这可能是获取该报告的“更安全”方法。这是一个调试选项。

* `--nodetach` 开关（仅限 Linux）告诉 `searchd` 不要脱离到后台。这也将导致日志条目打印到控制台。查询处理照常进行。这是一个调试选项，当您在 Docker 容器中运行 Manticore 时，捕获其输出时可能也会很有用。

## Plugin dir

Manticore 使用 [plugin_dir](../Server_settings/Common.md#plugin_dir) 来存储和使用 Manticore Buddy 插件。默认情况下，在标准安装中，该值对“manticore”用户可用。但是，如果您用不同的用户手动启动 searchd 守护进程，则守护进程可能没有访问 `plugin_dir` 的权限。为了解决此问题，请确保在通用部分中指定一个用户可以写入的 `plugin_dir`。

## Signals

`searchd` 支持多种信号：

* `SIGTERM` - 启动干净的关闭。新查询将不会被处理，但已启动的查询不会被强制中断。
* `SIGHUP` - 启动表轮换。根据 [seamless_rotate](../Server_settings/Searchd.md#seamless_rotate) 设置的值，新查询可能会短暂停滞；客户端将收到临时错误。
* `SIGUSR1` - 强制重新打开 searchd 日志和查询日志文件，允许日志文件轮换。

## Environment variables

* `MANTICORE_TRACK_DAEMON_SHUTDOWN=1` 在 searchd 关闭时启用详细日志记录。这在某些关闭问题的情况下非常有用，例如当 Manticore 关闭所需时间过长或在关闭过程中被冻结时。
<!-- proofread -->

