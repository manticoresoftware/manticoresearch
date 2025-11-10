# 手动启动 Manticore

您也可以通过直接调用 `searchd`（Manticore Search 服务器二进制文件）来启动 Manticore Search：

```shell
searchd [OPTIONS]
```

请注意，如果不指定配置文件路径，`searchd` 会根据操作系统在多个位置尝试查找配置文件。


## searchd 命令行选项


`searchd` 在所有操作系统中可用的选项有：

* `--help`（简写为 `-h`）列出您当前构建的 `searchd` 中可用的所有参数。
* `--version`（简写为 `-v`）显示 Manticore Search 版本信息。
* `--config <file>`（简写为 `-c <file>`）告诉 `searchd` 使用指定的文件作为配置文件。
* `--stop` 用于异步停止 `searchd`，使用 Manticore 配置文件中指定的 PID 文件详情。因此，您可能还需要通过 `--config` 选项确认 `searchd` 使用哪个配置文件。示例：

  ```bash
  $ searchd --config /etc/manticoresearch/manticore.conf --stop
  ```

* `--stopwait` 用于同步停止 `searchd`。`--stop` 本质上是告诉正在运行的实例退出（通过发送 SIGTERM 信号），然后立即返回。`--stopwait` 还会尝试等待正在运行的 `searchd` 实例实际完成关闭（例如保存所有待处理的属性更改）并退出。示例：

  ```bash
  $ searchd --config /etc/manticoresearch/manticore.conf --stopwait
  ```
  可能的退出代码如下：
    * 成功时返回 0
    * 连接到正在运行的 searchd 服务器失败时返回 1
    * 服务器在关闭期间报告错误时返回 2
    * 服务器在关闭期间崩溃时返回 3

* `--status` 命令用于使用（可选提供的）配置文件中的连接详情查询正在运行的 `searchd` 实例状态。它将尝试使用配置文件中找到的第一个 UNIX 套接字或 TCP 端口连接到正在运行的实例。成功后，它将查询多个状态和性能计数器值并打印出来。您也可以使用 [SHOW STATUS](../Node_info_and_management/Node_status.md#SHOW-STATUS) 命令通过 SQL 协议访问相同的计数器。示例：

  ```bash
  $ searchd --status
  $ searchd --config /etc/manticoresearch/manticore.conf --status
  ```

* `--pidfile` 用于显式强制使用 PID 文件（存储 `searchd` 进程标识号的文件），无视任何其他调试选项（例如 `--console`）。这是一个调试选项。

  ```bash
  $ searchd --console --pidfile
  ```

* `--console` 用于强制 `searchd` 进入控制台模式。通常，Manticore 作为传统服务器应用运行，并将信息记录到日志文件中（如配置文件中指定）。但是，在调试配置或服务器本身的问题，或尝试诊断难以追踪的问题时，强制其直接将信息输出到调用它的控制台/命令行可能更方便。控制台模式运行还意味着进程不会被 fork（因此搜索是顺序执行的），且不会写入日志。（需要注意的是，控制台模式并非运行 `searchd` 的推荐方式。）您可以这样调用：

  ```bash
  $ searchd --config /etc/manticoresearch/manticore.conf --console
  ```

* `--logdebug`、`--logreplication`、`--logdebugv` 和 `--logdebugvv` 选项启用服务器日志中的额外调试输出。它们的区别在于日志详细程度。这些是调试选项，通常不应启用，因为它们会大量污染日志。可根据需要临时启用以协助复杂的调试过程。

* `--iostats` 与日志选项配合使用（`manticore.conf` 中必须激活 `query_log`）以提供每个查询过程中输入/输出操作的更详细信息，代价是性能略有下降和日志稍大。IO 统计不包括属性的 IO 操作信息，因为属性是通过 mmap 加载的。启用方法如下：

  ```bash
  $ searchd --config /etc/manticoresearch/manticore.conf --iostats
  ```

* `--cpustats` 用于在查询日志文件（针对每个查询）和状态报告（汇总）中提供实际 CPU 时间报告（除了墙钟时间）。它依赖于 Linux 系统调用 `clock_gettime()`，在某些系统上会回退到精度较低的调用。您可以这样启动 `searchd`：

  ```bash
  $ searchd --config /etc/manticoresearch/manticore.conf --cpustats
  ```

* `--port portnumber`（简写为 `-p`）用于指定 Manticore 应监听的端口以接受二进制协议请求，通常用于调试。默认通常是 9312，但有时您需要在不同端口运行。命令行指定的端口会覆盖配置文件中的设置。有效范围是 0 到 65535，但编号 1024 及以下的端口通常需要特权账户运行。

  使用示例：

  ```bash
  $ searchd --port 9313
  ```

* `--listen ( address ":" port | port | path ) [ ":" protocol ]`（简写为 `-l`）功能类似于 `--port`，但允许您指定不仅是端口，还可以是完整路径、IP 地址和端口，或 `searchd` 将监听的 Unix 域套接字路径。换句话说，您可以指定 IP 地址（或主机名）和端口号，仅端口号，或 Unix 套接字路径。如果只指定端口号而不指定地址，searchd 将监听所有网络接口。Unix 路径以斜杠开头。作为最后一个参数，您还可以指定用于该套接字连接的协议处理程序（监听器）。支持的协议值为 'sphinx' 和 'mysql'（自 4.1 版本起使用的 MySQL 协议）。

* `--force-preread` 禁止服务器在表文件预读取完成之前处理任何传入连接。默认情况下，服务器启动时会在表文件懒加载到内存的同时接受连接。此选项扩展了该行为，使其等待文件加载完成后再接受连接。

* `--index (--table) <table>`（简写为 `-i (-t) <table>`）强制此 `searchd` 实例仅服务指定的表。与上面的 `--port` 类似，这通常用于调试目的；更长期的更改通常会直接应用于配置文件。

* `--strip-path` 从表引用的所有文件名中剥离路径名（停用词、词形变化、例外等）。这对于拾取在另一台机器上构建的、可能具有不同路径布局的表非常有用。

* `--replay-flags=<OPTIONS>` 开关可用于指定额外的二进制日志重放选项列表。支持的选项有：
    * `accept-desc-timestamp`，忽略递减的事务时间戳并仍然重放此类事务（默认行为是出现错误时退出）。
    * `ignore-open-errors`，忽略缺失的二进制日志文件（默认行为是出现错误时退出）。
    * `ignore-trx-errors`，忽略任何事务错误并跳过当前二进制日志文件（默认行为是出现错误时退出）。
    * `ignore-all-errors`，忽略上述任何错误（默认行为是出现错误时退出）。

    示例：

    ```bash
    $ searchd --replay-flags=accept-desc-timestamp
    ```

* `--coredump` 用于启用在服务器崩溃时保存核心文件或迷你转储。默认禁用以加快服务器崩溃后的重启速度。此选项对调试非常有用。

  ```bash
  $ searchd --config /etc/manticoresearch/manticore.conf --coredump
  ```

* `--new-cluster` 引导复制集群并使服务器成为具有[集群重启](../Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md)保护的参考节点。在 Linux 上，你也可以运行 `manticore_new_cluster`。它将通过 systemd 以 `--new-cluster` 模式启动 Manticore。

* `--new-cluster-force` 引导复制集群并使服务器成为参考节点，绕过[集群重启](../Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md)保护。在 Linux 上，你也可以运行 `manticore_new_cluster --force`。它将通过 systemd 以 `--new-cluster-force` 模式启动 Manticore。

* `--mockstack` 分析并报告递归表达式求值、模式匹配操作和过滤处理所需的堆栈大小。此调试选项将计算的堆栈需求输出到控制台以便优化。输出提供了可用于配置不同操作堆栈需求的环境变量。

  示例：
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

`searchd` 有一些特定于 Windows 平台的选项，涉及作为服务的处理，仅在 Windows 二进制文件中可用。

请注意，在 Windows 中，除非将其安装为服务，否则 searchd 默认以 `--console` 模式运行。

* `--install` 将 `searchd` 安装为 Microsoft 管理控制台（控制面板 / 管理工具 / 服务）中的服务。命令行中与 `--install` 一起指定的其他参数也将成为服务未来启动时的命令行参数。例如，作为调用 `searchd` 的一部分，你可能还需要使用 `--config` 指定配置文件，并且你会在指定 `--install` 的同时指定它。调用后，通常的启动/停止功能将通过管理控制台可用，因此任何用于启动、停止和重启服务的方法也适用于 `searchd`。示例：

  ```bat
  C:\WINDOWS\system32> C:\Manticore\bin\searchd.exe --install
     --config C:\Manticore\manticore.conf
  ```

  如果你希望每次启动 `searchd` 时都显示 I/O 统计信息，需要在与 `--install` 命令同一行指定该选项，如下：

  ```bat
  C:\WINDOWS\system32> C:\Manticore\bin\searchd.exe --install
     --config C:\Manticore\manticore.conf --iostats
  ```
* `--delete` 从 Microsoft 管理控制台及其他服务注册位置移除服务，此前需使用 `--install` 安装。请注意，这不会卸载软件或删除表。它意味着服务将不会从服务系统调用，也不会在机器下次启动时启动。如果当前作为服务运行，当前实例不会被终止（直到下次重启或使用 `--stop`）。如果服务是用自定义名称安装的（使用 `--servicename`），卸载时也需要使用相同名称。示例：

  ```bat
  C:\WINDOWS\system32> C:\Manticore\bin\searchd.exe --delete
  ```

* `--servicename <name>` 在安装或删除服务时，将给定的名称应用于 `searchd`，该名称将在管理控制台中显示；默认值为 searchd，但如果部署在多个管理员可能登录系统的服务器上，或具有多个 `searchd` 实例的系统上，可能需要更具描述性的名称。请注意，除非与 `--install` 或 `--delete` 结合使用，否则此选项无任何作用。示例：
  ```bat
  C:\WINDOWS\system32> C:\Manticore\bin\searchd.exe --install
     --config C:\Manticore\manticore.conf --servicename ManticoreSearch
  ```

* `--ntservice` 是由 Microsoft 管理控制台传递给 `searchd` 的一个选项，用于在 Windows 平台上以服务方式调用它。通常不需要直接调用此选项；当服务启动时，Windows 通常会调用它，尽管理论上如果你想从命令行以常规服务方式调用它（作为 `--console` 的补充），也是可以的。

* `--safetrace` 强制 `searchd` 在崩溃报告中仅使用系统的 backtrace() 调用。在某些（罕见的）情况下，这可能是获取该报告的“更安全”方式。这是一个调试选项。

* `--nodetach` 开关（仅限 Linux）告诉 `searchd` 不要分离到后台。这也会导致日志条目打印到控制台。查询处理照常进行。这是一个调试选项，在你在 Docker 容器中运行 Manticore 以捕获其输出时也可能有用。

## 插件目录

Manticore 利用 [plugin_dir](../Server_settings/Common.md#plugin_dir) 来存储和使用 Manticore Buddy 插件。默认情况下，该值在标准安装中对“manticore”用户可访问。但是，如果你以不同用户手动启动 searchd 守护进程，守护进程可能无法访问 `plugin_dir`。为了解决此问题，请确保在公共部分指定一个运行 searchd 守护进程的用户可以写入的 `plugin_dir`。

## 信号

`searchd` 支持多种信号：

* `SIGTERM` - 启动干净关闭。新查询将不被处理，但已开始的查询不会被强制中断。
* `SIGHUP` - 启动表轮换。根据 [seamless_rotate](../Server_settings/Searchd.md#seamless_rotate) 设置的值，新查询可能会短暂阻塞；客户端将收到临时错误。
* `SIGUSR1` - 强制重新打开 searchd 日志和查询日志文件，允许日志文件轮换。

## 环境变量

* `MANTICORE_TRACK_DAEMON_SHUTDOWN=1` 启用 searchd 关闭时的详细日志记录。当出现某些关闭问题时（例如 Manticore 关闭时间过长或关闭过程中冻结），此功能非常有用。
<!-- proofread -->

