# 手动启动 Manticore

您也可以通过直接调用 `searchd`（Manticore Search 服务器二进制文件）来启动 Manticore Search：

```shell
searchd [OPTIONS]
```

请注意，如果没有指定配置文件的路径，`searchd` 将会根据操作系统在几个位置尝试查找它。


## searchd 命令行选项


在所有操作系统中，`searchd` 可用的选项包括：

* `--help`（简写为 `-h`）列出在您当前构建的 `searchd` 中可以使用的所有参数。
* `--version`（简写为 `-v`）显示 Manticore Search 的版本信息。
* `--quiet`（简写为 `-q`）抑制启动输出，仅显示错误（横幅和预加载消息）。
* `--config <file>`（简写为 `-c <file>`）告诉 `searchd` 使用指定的文件作为其配置。
* `--stop` 用于异步停止 `searchd`，使用 Manticore 配置文件中指定的 PID 文件详细信息。因此，您可能还需要通过 `--config` 选项向 `searchd` 确认要使用的配置文件。示例：

  ```bash
  $ searchd --config /etc/manticoresearch/manticore.conf --stop
  ```

* `--stopwait` 用于同步停止 `searchd`。`--stop` 本质上是告诉正在运行的实例退出（通过发送 SIGTERM），然后立即返回。`--stopwait` 还会尝试等待直到正在运行的 `searchd` 实例实际完成关闭（例如，保存所有待处理的属性更改）并退出。示例：

  ```bash
  $ searchd --config /etc/manticoresearch/manticore.conf --stopwait
  ```
  可能的退出代码如下：
    * 成功时为 0
    * 如果连接到正在运行的 searchd 服务器失败时为 1
    * 如果服务器在关闭期间报告错误时为 2
    * 如果服务器在关闭期间崩溃时为 3

* `--status` 命令用于通过配置文件（可选提供）中的连接详细信息查询正在运行的 `searchd` 实例状态。它将尝试使用配置文件中找到的第一个 UNIX 套接字或 TCP 端口连接到正在运行的实例。成功时，它将查询多个状态和性能计数器值并打印出来。您也可以使用 [SHOW STATUS](../Node_info_and_management/Node_status.md#SHOW-STATUS) 命令通过 SQL 协议访问相同的计数器。示例：

  ```bash
  $ searchd --status
  $ searchd --config /etc/manticoresearch/manticore.conf --status
  ```

* `--pidfile` 用于显式强制使用 PID 文件（其中存储了 `searchd` 进程标识号），即使其他调试选项另有说明（例如，`--console`）。这是一个调试选项。

  ```bash
  $ searchd --console --pidfile
  ```

* `--console` 用于强制 `searchd` 进入控制台模式。通常，Manticore 作为常规服务器应用程序运行，并将信息记录到日志文件中（如配置文件中指定）。但是，当调试配置或服务器本身的问题，或尝试诊断难以追踪的问题时，强制它直接将信息转储到调用它的控制台/命令行可能更方便。在控制台模式下运行还意味着进程不会分叉（因此搜索是顺序进行的），并且不会写入日志。（需要注意的是，控制台模式不是运行 `searchd` 的预期方法。）您可以这样调用它：

  ```bash
  $ searchd --config /etc/manticoresearch/manticore.conf --console
  ```

* `--logdebug`、`--logreplication`、`--logdebugv` 和 `--logdebugvv` 选项用于在服务器日志中启用额外的调试输出。它们的区别在于日志详细程度级别。这些是调试选项，通常不应启用，因为它们可能会大量污染日志。它们可以临时启用以协助复杂的调试会话。

* `--iostats` 与日志选项（`manticore.conf` 中必须已激活 `query_log`）一起使用，以提供有关查询过程中执行的输入/输出操作的更详细信息，这会带来轻微的性能损失和稍大的日志。IO 统计信息不包括属性的 IO 操作信息，因为这些是通过 mmap 加载的。要启用它，您可以这样启动 `searchd`：

  ```bash
  $ searchd --config /etc/manticoresearch/manticore.conf --iostats
  ```

* `--cpustats` 用于在查询日志文件（每个查询）和状态报告（聚合）中提供实际的 CPU 时间报告（除了墙时间）。它依赖于 Linux 系统调用 `clock_gettime()`，或者在某些系统上回退到较不精确的调用。您可以这样启动 `searchd`：

  ```bash
  $ searchd --config /etc/manticoresearch/manticore.conf --cpustats
  ```

*  `--port portnumber`（简写为 `-p`）用于指定 Manticore 应监听以接受二进制协议请求的端口，通常用于调试目的。这通常默认为 9312，但有时需要在不同的端口上运行。在命令行上指定它将覆盖配置文件中指定的任何内容。有效范围是 0 到 65535，但 1024 及以下的端口通常需要特权账户才能运行。

  使用示例：

  ```bash
  $ searchd --port 9313
  ```

* `--listen ( address ":" port | port | path ) [ ":" protocol ]`（或 `-l` 简写）与 `--port` 的作用相同，但允许您指定不仅仅是端口，还可以指定完整的路径、IP 地址和端口，或 Unix 域套接字路径，`searchd` 将监听这些。换句话说，您可以指定 IP 地址（或主机名）和端口号，仅端口号，或 Unix 套接字路径。如果您指定了端口号但未指定地址，searchd 将监听所有网络接口。Unix 路径通过前导斜杠标识。作为最后一个参数，您还可以指定用于此套接字连接的协议处理程序（监听器）。支持的协议值为 'sphinx' 和 'mysql'（自 4.1 版本以来使用的 MySQL 协议）。

* `--force-preread` 禁止服务器在预读取表文件完成之前服务任何传入连接。默认情况下，在启动时，服务器在表文件懒加载到内存时接受连接。这扩展了该行为，并使其等待直到文件加载完成。

* `--index (--table) <table>`（或 `-i (-t) <table>` 简写）强制此 `searchd` 实例仅服务指定的表。与上面的 `--port` 一样，这通常用于调试目的；更长期的更改通常会直接应用于配置文件本身。

* `--strip-path` 从表中引用的所有文件名（停用词、词形变化、例外等）中剥离路径名。这对于拾取在另一台机器上构建的表（路径布局可能不同）非常有用。

* `--replay-flags=<OPTIONS>` 开关可用于指定额外的二进制日志重放选项列表。支持的选项有：
    * `accept-desc-timestamp`，忽略递减的事务时间戳并仍然重放这些事务（默认行为是出现错误时退出）。
    * `ignore-open-errors`，忽略缺失的二进制日志文件（默认行为是出现错误时退出）。
    * `ignore-trx-errors`，忽略任何事务错误并跳过当前二进制日志文件（默认行为是出现错误时退出）。
    * `ignore-all-errors`，忽略上述任何错误（默认行为是出现错误时退出）。

    例如：

    ```bash
    $ searchd --replay-flags=accept-desc-timestamp
    ```

* `--coredump` 用于启用在服务器崩溃时保存核心文件或迷你转储。默认禁用以加快服务器崩溃时的重启速度。这对于调试很有用。

  ```bash
  $ searchd --config /etc/manticoresearch/manticore.conf --coredump
  ```

* `--new-cluster` 启动一个复制集群并使服务器成为带有[集群重启](../Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md)保护的参考节点。在 Linux 上你还可以运行 `manticore_new_cluster`。它将通过 systemd 以 `--new-cluster` 模式启动 Manticore。

* `--new-cluster-force` 启动一个复制集群并使服务器成为参考节点，绕过[集群重启](../Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md)保护。在 Linux 上你还可以运行 `manticore_new_cluster --force`。它将通过 systemd 以 `--new-cluster-force` 模式启动 Manticore。

* `--mockstack` 分析并报告递归表达式求值、模式匹配操作和过滤处理所需的堆栈大小。此调试选项将计算的堆栈需求输出到控制台以进行优化。输出提供了可用于配置不同操作堆栈需求的环境变量。

  例如：
  ```bash
  $ searchd --mockstack
  Manticore 7.4.7 e90b5afbb@25032706 dev (columnar 4.1.2 15bbcc7@25031206) (secondary 4.1.2 15bbcc7@25031206) (knn 4.1.2 15bbcc7@25031206)
  Copyright (c) 2001-2016, Andrew Aksyonoff
  Copyright (c) 2008-2016, Sphinx Technologies Inc (http://sphinxsearch.com)
  Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)

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

`searchd` 在 Windows 平台上有一些特定的选项，主要是关于作为服务的处理，仅在 Windows 二进制文件上可用。

请注意，Windows 平台上的 searchd 默认使用 `--console` 模式，除非你将其安装为服务。

* `--install` 将 `searchd` 安装为 Microsoft 管理控制台（控制面板 / 管理工具 / 服务）中的服务。命令行中除 `--install` 外指定的其它参数也将成为服务未来启动时命令行的一部分。例如，作为调用 `searchd` 的一部分，你可能还需要使用 `--config` 指定配置文件，这时也应该与 `--install` 一起指定。调用后，可以通过管理控制台使用常规的启动/停止功能，因此任何启动、停止和重启服务的方法也适用于 `searchd`。示例：

  ```bat
  C:\WINDOWS\system32> C:\Manticore\bin\searchd.exe --install
     --config C:\Manticore\manticore.conf
  ```

  如果你希望每次启动 `searchd` 都显示 I/O 统计，需要将该选项与 `--install` 命令在同一行指定，如下：

  ```bat
  C:\WINDOWS\system32> C:\Manticore\bin\searchd.exe --install
     --config C:\Manticore\manticore.conf --iostats
  ```
* `--delete` 从 Microsoft 管理控制台及服务注册的其它位置移除该服务，前提是之前使用了 `--install`。注意，这不会卸载软件或删除表。它意味着服务将不会被服务系统调用，也不会在机器下一次启动时启动。如果当前作为服务运行，当前实例不会被终止（直到下一次重启或使用 `--stop`）。如果服务是用自定义名称（通过 `--servicename`）安装的，卸载时也需要使用相同的名称指定 `--servicename`。示例：

  ```bat
  C:\WINDOWS\system32> C:\Manticore\bin\searchd.exe --delete
  ```

* `--servicename <name>` 在安装或删除服务时赋予 `searchd` 一个名字，这个名字将显示在管理控制台；默认是 searchd，但如果在多管理员登录的服务器，或有多个 `searchd` 实例的系统上部署，可能需要更具描述性的名字。注意，除非与 `--install` 或 `--delete` 结合使用，否则此选项无效。示例：
  ```bat
  C:\WINDOWS\system32> C:\Manticore\bin\searchd.exe --install
     --config C:\Manticore\manticore.conf --servicename ManticoreSearch
  ```

* `--ntservice` 是由 Microsoft 管理控制台传递给 `searchd` 的选项，用于在 Windows 平台以服务形式调用它。通常不需要直接调用此选项；这是 Windows 启动服务时自动调用的，但理论上你可以作为命令行中的常规服务调用它（作为 `--console` 的补充）。

* `--safetrace` 强制 `searchd` 在崩溃报告中仅使用系统的 backtrace() 调用。在某些（罕见）场景中，这可能是获取报告的“更安全”方式。这是一个调试选项。

* `--nodetach` 开关（仅限 Linux）告诉 `searchd` 不要分离到后台。这也会导致日志条目打印到控制台。查询处理照常进行。这是一个调试选项，也可能在你运行 Manticore 容器化时捕获输出时有用。

## Plugin dir

Manticore 利用 [plugin_dir](../Server_settings/Common.md#plugin_dir) 来存储和使用 Manticore Buddy 插件。默认情况下，该值在标准安装中对 “manticore” 用户可访问。但是，如果你以不同用户手动启动 searchd 守护进程，守护进程可能无法访问 `plugin_dir`。为解决此问题，请确保在公共配置章节指定一个搜索守护进程运行用户可写的 `plugin_dir`。

## Signals

`searchd` 支持多个信号：

* `SIGTERM` - 初始清理关闭。新查询将不会被处理，但已启动的查询不会被强制中断。  
* `SIGHUP` - 启动表的旋转。根据 [seamless_rotate](../Server_settings/Searchd.md#seamless_rotate) 设置值，新查询可能会短暂卡住；客户端将收到临时错误。  
* `SIGUSR1` - 强制重开搜索d日志和查询日志文件，允许日志文件旋转。  

## 环境变量  

* `MANTICORE_TRACK_DAEMON_SHUTDOWN=1` 启用详细日志，当搜索d关闭时会记录详细信息。这在某些关闭问题中非常有用，例如当Manticore长时间关闭或在关闭过程中冻结时。  
<!-- proofread -->
