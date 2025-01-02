# 报告错误

不幸的是，Manticore 还不是 100% 无错误的，但开发团队正在为此努力。您可能会偶尔遇到一些问题。为了有效地修复错误，报告尽可能多的信息非常重要。要修复错误，通常需要重现并解决问题，或者根据您提供的信息推断出问题的原因。为此，请按照以下说明操作。

### Bug 追踪器
错误和功能请求都在 [Github](https://github.com/manticoresoftware/manticore/issues) 上进行跟踪。欢迎您创建一个新工单，并详细描述您的错误，这样可以节省您和开发人员的时间。

### 文档更新
本文档（您正在阅读的内容）的更新也可以在 [Github](https://github.com/manticoresoftware/manticoresearch/tree/master/manual) 上进行。

### 崩溃

Manticore Search 是用 C++ 编写的，这是一种低级编程语言，允许与计算机直接通信，以实现更快的性能。然而，这种方法也有一个缺点，在极少数情况下，可能无法通过将错误写入日志并跳过导致问题的命令来优雅地处理错误。相反，程序可能会崩溃，导致完全停止并需要重新启动。

当 Manticore Search 崩溃时，重要的是通过 [GitHub 提交错误报告](https://github.com/manticoresoftware/manticoresearch/issues/new?assignees=&labels=&template=bug_report.md&title=) 或通过 Manticore 的专业服务在您的私人帮助台报告问题。Manticore 团队需要以下信息：

1. `searchd` 日志
2. `coredump`
3. 查询日志

此外，建议您执行以下操作：

1. 使用 gdb 检查 `coredump`：

```
gdb /usr/bin/searchd </path/to/coredump>
```
2. 在 `coredump` 文件名中查找崩溃的线程 ID（确保在 `/proc/sys/kernel/core_pattern` 中有 `%p`），例如 `core.work_6.29050.server_name.1637586599` 表示 `thread_id=29050`
3. 在 gdb 中运行：
```
set pagination off
info threads
# find thread number by it's id (e.g. for `LWP 29050` it will be thread number 8
thread apply all bt
thread <thread number>
bt full
info locals
quit
```
4. 提供输出结果

### 当 Manticore Search 挂起时，我该怎么办？

如果 Manticore Search 挂起，您需要收集一些信息，以帮助了解问题的原因。您可以按以下步骤进行操作：

1. 通过 [VIP 端口](https://manual.manticoresearch.com/Connecting_to_the_server/HTTP#VIP-connection) 运行 `show threads option format=all`。
2. 收集 lsof 输出，因为挂起可能是由过多连接或打开的文件描述符引起的。

```
lsof -p `cat /var/run/manticore/searchd.pid`
```
3. 转储核心文件：
```
gcore `cat /var/run/manticore/searchd.pid`
```
（它会将转储文件保存到当前目录。）

4. 安装并运行 gdb：
```
gdb /usr/bin/searchd `cat /var/run/manticore/searchd.pid`
```
请注意，这将暂停正在运行的 `searchd`，但如果它已经挂起，则不应有任何问题。
5. 在 gdb 中运行：
```
set pagination off
info threads
thread apply all bt
quit
```
6. 收集所有输出和文件，并将它们附加到错误报告中。

对于专家用户：可以参考[这个提交](https://github.com/manticoresoftware/manticoresearch/commit/e317f7aa30aad51cb19d34595458bb7c8811be21)中添加的宏来帮助调试。

### 如何启用崩溃时保存核心转储？

* 确保以 `--coredump` 运行 `searchd`。要避免修改脚本，您可以使用 [systemd 自定义启动参数](Starting_the_server/Linux#使用 systemd 自定义启动标志)⛔。例如：

```
[root@srv lib]# systemctl set-environment _ADDITIONAL_SEARCHD_PARAMS='--coredump'
[root@srv lib]# systemctl restart manticore
[root@srv lib]# ps aux|grep searchd
mantico+  1955  0.0  0.0  61964  1580 ?        S    11:02   0:00 /usr/bin/searchd --config /etc/manticoresearch/manticore.conf --coredump
mantico+  1956  0.6  0.0 392744  2664 ?        Sl   11:02   0:00 /usr/bin/searchd --config /etc/manticoresearch/manticore.conf --coredump
```

* 确保操作系统允许保存核心转储，检查 `/proc/sys/kernel/core_pattern` 是否不为空。此路径指示核心转储的保存位置。要将核心转储保存到文件（如 `core.searchd.1773.centos-4gb-hel1-1.1636454937`），可以运行以下命令：
```
echo "/cores/core.%e.%p.%h.%t" > /proc/sys/kernel/core_pattern
```

* `searchd` 应以 `ulimit -c unlimited` 启动。如果使用 systemctl 启动 Manticore，systemctl 将自动将限制设置为无限大，如 manticore.service 文件中所示：
```
[root@srv lib]# grep CORE /lib/systemd/system/manticore.service
LimitCORE=infinity
```

### 如何安装调试符号？

Manticore Search 和 Manticore Columnar Library 使用 C++ 编写，因此会编译生成在您的操作系统上优化执行的二进制文件。然而，在运行这些二进制文件时，系统并不完全知道变量、函数、方法和类的名称。这些信息通过单独的 "debuginfo" 或 "symbol 包" 提供。

调试符号对于故障排除和调试至关重要，因为它们允许您在程序崩溃时可视化系统状态，包括函数的名称。Manticore Search 在生成 coredump 文件时提供了回溯信息，并在使用 `--coredump` 标志运行时生成 coredump 文件。没有符号时，您只能看到内部偏移量，难以或无法解码崩溃原因。如果您需要报告崩溃问题，Manticore 团队通常会要求调试符号帮助您。

要安装 Manticore Search/Manticore Columnar Library 的调试符号，您需要为 CentOS 安装 `*debuginfo*` 包，为 Ubuntu 和 Debian 安装 `*dbgsym*` 包，或者为 Windows 和 macOS 安装 `*dbgsymbols*` 包。这些包的版本应与已安装的 Manticore 版本相同。例如，如果您在 CentOS 8 中从包安装了 Manticore Search：https://repo.manticoresearch.com/repository/manticoresearch/release/centos/8/x86_64/manticore-4.0.2_210921.af497f245-1.el8.x86_64.rpm，那么与其对应的符号包将是：https://repo.manticoresearch.com/repository/manticoresearch/release/centos/8/x86_64/manticore-debuginfo-4.0.2_210921.af497f245-1.el8.x86_64.rpm。

注意，两个包的提交 ID 均为 `af497f245`，这对应于构建该版本的提交。

如果您从 Manticore APT/YUM 存储库安装了 Manticore，您可以使用以下工具之一：

* `debuginfo-install` in CentOS 7
* `dnf debuginfo-install` CentOS 8
* `find-dbgsym-packages` in Debian and Ubuntu

为您找到调试符号包。

### 如何检查调试符号是否存在？

1. 在 `file /usr/bin/searchd` 的输出中找到构建 ID：

```
[root@srv lib]# file /usr/bin/searchd
/usr/bin/searchd: ELF 64-bit LSB executable, x86-64, version 1 (GNU/Linux), dynamically linked (uses shared libs), for GNU/Linux 2.6.32, BuildID[sha1]=2c582e9f564ea1fbeb0c68406c271ba27034a6d3, stripped
```

在此示例中，构建 ID 为 `2c582e9f564ea1fbeb0c68406c271ba27034a6d3`。

2. 在 `/usr/lib/debug/.build-id` 中找到符号，如下所示：

```
[root@srv ~]# ls -la /usr/lib/debug/.build-id/2c/582e9f564ea1fbeb0c68406c271ba27034a6d3*
lrwxrwxrwx. 1 root root 23 Nov  9 10:42 /usr/lib/debug/.build-id/2c/582e9f564ea1fbeb0c68406c271ba27034a6d3 -> ../../../../bin/searchd
lrwxrwxrwx. 1 root root 27 Nov  9 10:42 /usr/lib/debug/.build-id/2c/582e9f564ea1fbeb0c68406c271ba27034a6d3.debug -> ../../usr/bin/searchd.debug
```

### 上传您的数据

为了修复您的问题，开发人员通常需要在本地重现它。为此，他们需要您的配置文件、表文件、binlog（如果有）以及有时源数据（例如外部存储中的数据或 XML/CSV 文件）和查询。

当您[在 Github 上创建工单](https://github.com/manticoresoftware/manticoresearch/issues/new)时，附上您的数据。如果数据过大或敏感，您可以将其上传到我们的只写 S3 存储：`s3://s3.manticoresearch.com/write-only/`。以下是使用 Minio 客户端上传数据的步骤：

1. 安装客户端 https://min.io/docs/minio/linux/reference/minio-mc.html#install-mc 

  例如，在 64 位 Linux 上：

```
curl https://dl.min.io/client/mc/release/linux-amd64/mc \
  --create-dirs \
  -o $HOME/minio-binaries/mc
chmod +x $HOME/minio-binaries/mc
export PATH=$PATH:$HOME/minio-binaries/
```
2. 添加我们的 s3 主机（使用可执行文件的完整路径，或切换到其目录）：`cd $HOME/minio-binaries`，然后运行：`./mc config host add manticore http://s3.manticoresearch.com:9000 manticore manticore`
3. 复制您的文件（使用可执行文件的完整路径，或切换到其目录）：`cd $HOME/minio-binaries`，然后运行：`./mc cp -r issue-1234/ manticore/write-only/issue-1234`。确保文件夹名称唯一，最好与您在 GitHub 上描述错误的工单相对应。

### DEBUG

```sql
DEBUG [ subcommand ]
```

`DEBUG` 语句是为开发者和测试人员设计的，用于调用各种内部或 VIP 命令。然而，它**不适用于生产环境**，因为 `subcommand` 组件的语法可能会在任何版本中自由更改。

要查看当前环境中可用的有用命令和 `DEBUG` 语句的子命令列表，只需在没有任何参数的情况下调用 `DEBUG`。

```sql
mysql> debug;
+-------------------------------------------------------------------------+----------------------------------------------------------------------------------------+
| command                                                                 | meaning                                                                                |
+-------------------------------------------------------------------------+----------------------------------------------------------------------------------------+
| flush logs                                                              | emulate USR1 signal                                                                    |
| reload indexes                                                          | emulate HUP signal                                                                     |
| debug token <password>                                                  | calculate token for password                                                           |
| debug malloc_stats                                                      | perform 'malloc_stats', result in searchd.log                                          |
| debug malloc_trim                                                       | pefrorm 'malloc_trim' call                                                             |
| debug sleep <N>                                                         | sleep for <N> seconds                                                                  |
| debug tasks                                                             | display global tasks stat (use select from @@system.tasks instead)                     |
| debug sched                                                             | display task manager schedule (use select from @@system.sched instead)                 |
| debug merge <TBL> [chunk] <X> [into] [chunk] <Y> [option sync=1,byid=0] | For RT table <TBL> merge disk chunk X into disk chunk Y                                |
| debug drop [chunk] <X> [from] <TBL> [option sync=1]                     | For RT table <TBL> drop disk chunk X                                                   |
| debug files <TBL> [option format=all|external]                          | list files belonging to <TBL>. 'all' - including external (wordforms, stopwords, etc.) |
| debug close                                                             | ask server to close connection from it's side                                          |
| debug compress <TBL> [chunk] <X> [option sync=1]                        | Compress disk chunk X of RT table <TBL> (wipe out deleted documents)                   |
| debug split <TBL> [chunk] <X> on @<uservar> [option sync=1]             | Split disk chunk X of RT table <TBL> using set of DocIDs from @uservar                 |
| debug wait <cluster> [like 'xx'] [option timeout=3]                     | wait <cluster> ready, but no more than 3 secs.                                         |
| debug wait <cluster> status <N> [like 'xx'] [option timeout=13]         | wait <cluster> commit achieve <N>, but no more than 13 secs                            |
| debug meta                                                              | Show max_matches/pseudo_shards. Needs set profiling=1                                  |
| debug trace OFF|'path/to/file' [<N>]                                    | trace flow to file until N bytes written, or 'trace OFF'                               |
| debug curl <URL>                                                        | request given url via libcurl                                                          |
+-------------------------------------------------------------------------+----------------------------------------------------------------------------------------+
19 rows in set (0.00 sec)
```

Same from VIP connection:
```sql
mysql> debug;
+-------------------------------------------------------------------------+----------------------------------------------------------------------------------------+
| command                                                                 | meaning                                                                                |
+-------------------------------------------------------------------------+----------------------------------------------------------------------------------------+
| flush logs                                                              | emulate USR1 signal                                                                    |
| reload indexes                                                          | emulate HUP signal                                                                     |
| debug shutdown <password>                                               | emulate TERM signal                                                                    |
| debug crash <password>                                                  | crash daemon (make SIGSEGV action)                                                     |
| debug token <password>                                                  | calculate token for password                                                           |
| debug malloc_stats                                                      | perform 'malloc_stats', result in searchd.log                                          |
| debug malloc_trim                                                       | pefrorm 'malloc_trim' call                                                             |
| debug procdump                                                          | ask watchdog to dump us                                                                |
| debug setgdb on|off                                                     | enable or disable potentially dangerous crash dumping with gdb                         |
| debug setgdb status                                                     | show current mode of gdb dumping                                                       |
| debug sleep <N>                                                         | sleep for <N> seconds                                                                  |
| debug tasks                                                             | display global tasks stat (use select from @@system.tasks instead)                     |
| debug sched                                                             | display task manager schedule (use select from @@system.sched instead)                 |
| debug merge <TBL> [chunk] <X> [into] [chunk] <Y> [option sync=1,byid=0] | For RT table <TBL> merge disk chunk X into disk chunk Y                                |
| debug drop [chunk] <X> [from] <TBL> [option sync=1]                     | For RT table <TBL> drop disk chunk X                                                   |
| debug files <TBL> [option format=all|external]                          | list files belonging to <TBL>. 'all' - including external (wordforms, stopwords, etc.) |
| debug close                                                             | ask server to close connection from it's side                                          |
| debug compress <TBL> [chunk] <X> [option sync=1]                        | Compress disk chunk X of RT table <TBL> (wipe out deleted documents)                   |
| debug split <TBL> [chunk] <X> on @<uservar> [option sync=1]             | Split disk chunk X of RT table <TBL> using set of DocIDs from @uservar                 |
| debug wait <cluster> [like 'xx'] [option timeout=3]                     | wait <cluster> ready, but no more than 3 secs.                                         |
| debug wait <cluster> status <N> [like 'xx'] [option timeout=13]         | wait <cluster> commit achieve <N>, but no more than 13 secs                            |
| debug meta                                                              | Show max_matches/pseudo_shards. Needs set profiling=1                                  |
| debug trace OFF|'path/to/file' [<N>]                                    | trace flow to file until N bytes written, or 'trace OFF'                               |
| debug curl <URL>                                                        | request given url via libcurl                                                          |
+-------------------------------------------------------------------------+----------------------------------------------------------------------------------------+
24 rows in set (0.00 sec)
```

所有 `debug XXX` 命令都应被视为不稳定，并且可能随时修改，因此如果它们发生变化，请不要感到意外。以下示例输出可能无法反映实际可用的命令，因此请在您的系统上运行以查看您的实例上有哪些可用命令。此外，除了此处提供的简短“含义”列之外，没有其他详细文档。

以下简要说明两个仅对 VIP 客户端可用的命令——`shutdown` 和 `crash`。这两个命令都需要一个 token，它可以通过 `debug token` 子命令生成，并添加到配置文件中 searchd 部分的 [shutdown_token](Server_settings/Searchd.md#shutdown_token) 参数中。如果不存在该部分，或者提供的密码哈希与配置文件中存储的 token 不匹配，则这些子命令不会执行任何操作。

```sql
mysql> debug token hello;
+-------------+------------------------------------------+
| command     | result                                   |
+-------------+------------------------------------------+
| debug token | aaf4c61ddcc5e8a2dabede0f3b482cd9aea9434d |
+-------------+------------------------------------------+
1 row in set (0,00 sec)
```

子命令 `shutdown` 将向服务器发送 TERM 信号，导致其关闭。这可能是危险的，因为没有人希望意外停止生产服务。因此，它需要 VIP 连接和密码。

子命令 `crash` 会导致服务器崩溃。它可能用于测试目的，例如测试系统管理器如何维护服务的存活性或测试跟踪核心转储的可行性。

如果某些命令在更通用的环境中被发现有用，它们可能会从 debug 子命令移至更稳定和通用的位置（如表中的 `debug tasks` 和 `debug sched` 所示）。

<!-- proofread -->