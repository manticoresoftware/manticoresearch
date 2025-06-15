# 手动启动 Manticore

你也可以直接通过调用 `searchd`（Manticore Search 服务器二进制文件）来启动 Manticore Search：

```shell
searchd [OPTIONS]
```

注意，如果不指定配置文件的路径，`searchd` 会根据操作系统在几个位置尝试查找配置文件。


## searchd 命令行选项


适用于所有操作系统的 `searchd` 可用选项有：

* `--help`（简写为 `-h`）列出你特定构建版本的 `searchd` 可使用的所有参数。
* `--version`（简写为 `-v`）显示 Manticore Search 的版本信息。
* `--config <file>`（简写为 `-c <file>`）告诉 `searchd` 使用指定的文件作为配置文件。
* `--stop` 用于异步停止 `searchd`，利用 Manticore 配置文件中指定的 PID 文件详情。因此，你可能还需要用 `--config` 选项确认 `searchd` 使用哪个配置文件。例如：

  ```bash
  $ searchd --config /etc/manticoresearch/manticore.conf --stop
  ```

* `--stopwait` 用于同步停止 `searchd`。`--stop` 实际上是告诉正在运行的实例退出（通过发送 SIGTERM），并且立即返回。`--stopwait` 还会尝试等待正在运行的 `searchd` 实例真正完成关闭（例如保存所有待处理的属性更改）并退出。例如：

  ```bash
  $ searchd --config /etc/manticoresearch/manticore.conf --stopwait
  ```
  可能的退出代码如下：
    * 0 表示成功
    * 1 连接运行中的 searchd 服务器失败
    * 2 服务器在关闭时报告错误
    * 3 服务器关闭时崩溃

* `--status` 命令用于查询运行中的 `searchd` 实例状态，使用（可选提供的）配置文件中的连接详情。它会尝试使用配置文件中找到的第一个 UNIX 套接字或 TCP 端口连接运行实例。成功时，它会查询多个状态和性能计数器值并打印。你也可以使用 [SHOW STATUS](../Node_info_and_management/Node_status.md#SHOW-STATUS) 命令通过 SQL 协议访问相同的计数器。示例：

  ```bash
  $ searchd --status
  $ searchd --config /etc/manticoresearch/manticore.conf --status
  ```

* `--pidfile` 用于明确强制使用 PID 文件（存储 `searchd` 进程标识号），无视其他调试选项（例如 `--console`）。这是一个调试选项。

  ```bash
  $ searchd --console --pidfile
  ```

* `--console` 用于强制 `searchd` 进入控制台模式。通常，Manticore 作为传统服务器应用程序运行，日志信息写入日志文件（如配置文件中指定）。但是当调试配置或服务器本身问题，或试图诊断难以追踪的问题时，强制直接向调用的控制台/命令行输出信息可能更方便。控制台模式运行意味着进程不会进行 fork（所以搜索是顺序执行的），且不会写日志文件。（需要注意的是，控制台模式非 `searchd` 的预期运行方式。）调用示例：

  ```bash
  $ searchd --config /etc/manticoresearch/manticore.conf --console
  ```

* `--logdebug`、`--logreplication`、`--logdebugv` 和 `--logdebugvv` 选项启用服务器日志中额外的调试输出。它们各自日志详细程度不同。这些是调试选项，一般不应启用，因为会极大污染日志。可根据需要临时使用以协助复杂调试。

* `--iostats` 配合日志选项使用（必须在 `manticore.conf` 中启用了 `query_log`），以提供每个查询中执行的输入/输出操作的更详细信息，会稍微影响性能并增加日志大小。IO 统计不包括属性的 IO 操作，因为属性是通过 mmap 加载的。启用示例：

  ```bash
  $ searchd --config /etc/manticoresearch/manticore.conf --iostats
  ```

* `--cpustats` 用于在查询日志文件（针对每个查询）和状态报告（聚合）中提供实际 CPU 时间报告（除了墙钟时间之外）。依赖 Linux 系统调用 `clock_gettime()`，在某些系统上会回退到不太精确的调用。可用以下方式启动：

  ```bash
  $ searchd --config /etc/manticoresearch/manticore.conf --cpustats
  ```

* `--port portnumber`（简写为 `-p`）用于指定 Manticore 应监听以接受二进制协议请求的端口，通常用于调试。默认通常为 9312，但有时需要运行在不同端口。命令行指定的优先级高于配置文件。有效范围是 0 到 65535，但端口号在 1024 及以下通常需要有特权账户才能运行。

  使用示例：

  ```bash
  $ searchd --port 9313
  ```

* `--listen ( address ":" port | port | path ) [ ":" protocol ]`（简写为 `-l`）功能与 `--port` 类似，但允许你指定不仅是端口，还可以是完整路径、IP 地址和端口，或 Unix 域套接字路径。换句话说，你可以指定 IP 地址（或主机名）和端口号，只指定端口号，或者 Unix 套接字路径。如果只指定端口号而不指定地址，searchd 将监听所有网络接口。Unix 路径以斜杠开头。作为最后一个参数，你还可以指定用于该套接字连接的协议处理程序（监听器）。支持的协议值有 ‘sphinx’ 和 ‘mysql’（MySQL 协议，自 4.1 版本起使用）。

* `--force-preread` 禁止服务器在表文件预读取完成之前接受任何传入连接。默认情况下，启动时服务器在表文件懒加载到内存时接受连接。此选项扩展了该行为，使其等待文件加载完成后再继续。

* `--index (--table) <table>`（简写为 `-i (-t) <table>`）强制此实例的 `searchd` 仅服务指定的表。与上面的 `--port` 类似，这通常用于调试；更长期的更改一般会应用于配置文件本身。

* `--strip-path` 移除表中引用的所有文件名的路径（如停用词、词形变化、例外等）。这对于拾取在另一台可能路径布局不同的机器上构建的表非常有用。

* `--replay-flags=<OPTIONS>` 开关可用于指定一系列额外的二进制日志重放选项。支持的选项包括：
    * `accept-desc-timestamp`，忽略降序事务时间戳，仍然重放这些事务（默认行为是遇错退出）。
    * `ignore-open-errors`，忽略缺失的二进制日志文件（默认行为是遇错退出）。
    * `ignore-trx-errors`，忽略任何事务错误并跳过当前二进制日志文件（默认行为是遇错退出）。
    * `ignore-all-errors`，忽略上述所有错误（默认行为是遇错退出）。

    示例：

    ```bash
    $ searchd --replay-flags=accept-desc-timestamp
    ```

* `--coredump` 用于启用服务器崩溃时保存 core 文件或 minidump。默认禁用以加快服务器崩溃后的重启速度。此选项对调试目的非常有用。

  ```bash
  $ searchd --config /etc/manticoresearch/manticore.conf --coredump
  ```

* `--new-cluster` 引导复制集群并使服务器成为带有[集群重启](../Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md)保护的参考节点。在 Linux 上你也可以运行 `manticore_new_cluster`。它将通过 systemd 以 `--new-cluster` 模式启动 Manticore。

* `--new-cluster-force` 引导复制集群并使服务器成为绕过[集群重启](../Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md)保护的参考节点。在 Linux 上你也可以运行 `manticore_new_cluster --force`。它将通过 systemd 以 `--new-cluster-force` 模式启动 Manticore。

* `--mockstack` 分析并报告递归表达式评估、模式匹配操作和过滤处理所需的栈大小。此调试选项将计算的栈需求输出到控制台以供优化使用。输出提供可用于配置不同操作栈需求的环境变量。

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

对于 Windows 平台，有一些仅对 `searchd` 作为服务处理相关的选项，这些选项仅在 Windows 二进制文件中可用。

注意，在 Windows 下，除非将其安装为服务，否则 searchd 将默认使用 `--console` 模式。

* `--install` 将 `searchd` 安装为 Microsoft 管理控制台中的服务（控制面板 / 管理工具 / 服务）。在指定 `--install` 时命令行中指定的其他参数也会成为该服务未来启动时的命令行参数。例如，作为调用 `searchd` 的一部分，通常还需要指定配置文件参数 `--config` ，这也需要与 `--install` 一起指定。调用后，管理控制台将可以进行常规启动/停止操作，因此任何可用于启动、停止和重启服务的方法同样适用于 `searchd`。示例：

  ```bat
  C:\WINDOWS\system32> C:\Manticore\bin\searchd.exe --install
     --config C:\Manticore\manticore.conf
  ```

  如果每次启动 `searchd` 时都想查看 I/O 统计数据，需要在与 `--install` 命令同一行指定该选项，如下：

  ```bat
  C:\WINDOWS\system32> C:\Manticore\bin\searchd.exe --install
     --config C:\Manticore\manticore.conf --iostats
  ```
* `--delete` 从 Microsoft 管理控制台及其他注册服务的位置移除该服务（之前须用 `--install` 安装）。注意，这不会卸载软件或删除表。它意味着服务将不会被服务系统调用，也不会在机器下次启动时启动。如果当前作为服务运行，当前实例不会被终止（直到下一次重启或使用 `--stop`）。如果服务是使用自定义名称安装（使用 `--servicename`），卸载时也需要使用相同的名称。示例：

  ```bat
  C:\WINDOWS\system32> C:\Manticore\bin\searchd.exe --delete
  ```

* `--servicename <name>` 在安装或删除服务时将给定的名称应用于 `searchd`，该名称将在管理控制台中显示；默认值为 searchd，但如果部署在多个管理员可能登录系统的服务器上，或者系统中有多个 `searchd` 实例，则可能需要一个更具描述性的名称。请注意，除非与 `--install` 或 `--delete` 结合使用，否则此选项不会产生任何效果。示例：
  ```bat
  C:\WINDOWS\system32> C:\Manticore\bin\searchd.exe --install
     --config C:\Manticore\manticore.conf --servicename ManticoreSearch
  ```

* `--ntservice` 是由 Microsoft 管理控制台传递给 `searchd` 的一个选项，用于在 Windows 平台上将其作为服务调用。通常不需要直接调用此选项；Windows 在启动服务时通常会调用它，不过理论上如果你想在命令行中（作为 `--console` 的补充）将其作为常规服务调用，也是可以的。

* `--safetrace` 强制 `searchd` 在崩溃报告中仅使用系统的 backtrace() 调用。在某些（罕见的）场景中，这可能是获取报告的“更安全”的方式。这是一个调试选项。

* `--nodetach` 开关（仅限 Linux）告诉 `searchd` 不要分离到后台。这也将导致日志条目打印到控制台上。查询处理照常进行。这是一个调试选项，在你在 Docker 容器中运行 Manticore 时捕获其输出可能也很有用。

## 插件目录

Manticore 使用 [plugin_dir](../Server_settings/Common.md#plugin_dir) 来存储和使用 Manticore Buddy 插件。默认情况下，在标准安装中此目录对“manticore”用户是可访问的。但是，如果你用不同的用户手动启动 searchd 守护进程，守护进程可能无法访问 `plugin_dir`。为了解决此问题，请确保在 common 部分指定一个运行 searchd 守护进程的用户具有写权限的 `plugin_dir`。

## 信号

`searchd` 支持多种信号：

* `SIGTERM` - 启动干净的关闭过程。新查询将不被处理，但已开始的查询不会被强制中断。
* `SIGHUP` - 启动表轮转。根据 [seamless_rotate](../Server_settings/Searchd.md#seamless_rotate) 设置的值，新查询可能会短暂延迟；客户端将收到临时错误。
* `SIGUSR1` - 强制重新打开 searchd 的日志和查询日志文件，以便进行日志文件轮转。

## 环境变量

* `MANTICORE_TRACK_DAEMON_SHUTDOWN=1` 使 searchd 在关闭时进行详细日志记录。这在关闭过程中出现问题时非常有用，比如 Manticore 关闭时间过长或关闭过程中卡死时。
<!-- proofread -->

