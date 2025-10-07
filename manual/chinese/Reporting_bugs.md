# 报告错误

不幸的是，Manticore 还不是百分之百无错误的，尽管开发团队正在为此目标努力工作。您可能不时会遇到一些问题。
报告错误时尽可能多提供信息至关重要，以便有效修复。
要修复一个错误，要么需要重现并修复，要么需要根据您提供的信息推断原因。为此，请遵循以下说明。

### 错误跟踪器
错误和功能请求在 [Github](https://github.com/manticoresoftware/manticore/issues) 上跟踪。欢迎您创建新的工单，详细描述您的错误，以节省您和开发者的时间。

### 文档更新
文档的更新（您正在阅读的内容）也在 [Github](https://github.com/manticoresoftware/manticoresearch/tree/master/manual) 上进行。

### 崩溃

Manticore Search 是用 C++ 编写的，C++ 是一种低级编程语言，允许与计算机直接通信以提升性能。然而，这种方法有一个缺点，在极少数情况下，可能无法通过将错误写入日志并跳过导致问题的命令来优雅处理错误。程序可能会崩溃，导致完全停止，需要重新启动。

当 Manticore Search 崩溃时，向 Manticore 团队报告很重要，可以通过在 GitHub 上[提交错误报告](https://github.com/manticoresoftware/manticoresearch/issues/new?assignees=&labels=&template=bug_report.md&title=)或通过 Manticore 的私有帮助台中的专业服务提交。Manticore 团队需要以下信息：

1. searchd 日志
2. coredump 文件
3. 查询日志

此外，如果您能做到以下几点将更有帮助：
1. 运行 gdb 来检查 coredump：
```
gdb /usr/bin/searchd </path/to/coredump>
```
2. 在 coredump 文件名中找到崩溃线程 ID（确保 `/proc/sys/kernel/core_pattern` 中含 `%p`），例如 `core.work_6.29050.server_name.1637586599` 表示 thread_id=29050
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

如果 Manticore Search 挂起，您需要收集一些可能有助于了解原因的信息。步骤如下：

1. 通过 [VIP 端口](https://manual.manticoresearch.com/Connecting_to_the_server/HTTP#VIP-connection) 运行 `show threads option format=all`
2. 收集 lsof 输出，因为挂起可能是由于太多连接或文件描述符打开导致的。
```
lsof -p `cat /run/manticore/searchd.pid`
```
3. 导出 core：
```
gcore `cat /run/manticore/searchd.pid`
```
（这会将 dump 保存到当前目录。）

4. 安装并运行 gdb：
```
gdb /usr/bin/searchd `cat /run/manticore/searchd.pid`
```
注意，这将停止正在运行的 searchd，但如果它已经挂起，这不应成为问题。
5. 在 gdb 中运行：
```
set pagination off
info threads
thread apply all bt
quit
```
6. 收集所有输出和文件并在错误报告中提供。

专家提示：在[此提交](https://github.com/manticoresoftware/manticoresearch/commit/e317f7aa30aad51cb19d34595458bb7c8811be21)中添加的宏对于调试可能有帮助。

### 如何在崩溃时启用保存 coredumps？

* 确保您使用 `--coredump` 启动 searchd。为避免修改脚本，您可以使用 https://manual.manticoresearch.com/Starting_the_server/Linux#Custom-startup-flags-using-systemd 方法。例如::

```
[root@srv lib]# systemctl set-environment _ADDITIONAL_SEARCHD_PARAMS='--coredump'
[root@srv lib]# systemctl restart manticore
[root@srv lib]# ps aux|grep searchd
mantico+  1955  0.0  0.0  61964  1580 ?        S    11:02   0:00 /usr/bin/searchd --config /etc/manticoresearch/manticore.conf --coredump
mantico+  1956  0.6  0.0 392744  2664 ?        Sl   11:02   0:00 /usr/bin/searchd --config /etc/manticoresearch/manticore.conf --coredump
```

* 确保您的操作系统允许保存 core dumps，检查 `/proc/sys/kernel/core_pattern` 是否非空。这是保存 core dumps 的位置。若要将 core dumps 保存为 `core.searchd.1773.centos-4gb-hel1-1.1636454937` 等文件，运行以下命令：
```
echo "/cores/core.%e.%p.%h.%t" > /proc/sys/kernel/core_pattern
```

* 启动 searchd 前应运行 `ulimit -c unlimited`。如果您使用 systemctl 启动 Manticore，它会自动将限制设置为无限制，如 manticore.service 文件中的以下行所示：
```
[root@srv lib]# grep CORE /lib/systemd/system/manticore.service
LimitCORE=infinity
```

### 如何安装调试符号？

Manticore Search 和 Manticore Columnar Library 是用 C++ 编写的，生成的二进制文件在您的操作系统上以最佳方式执行。然而，运行二进制文件时，系统无法完全访问变量名、函数、方法和类的名称。这些信息包含在单独的 "debuginfo" 或 "symbol packages" 中。

调试符号对于故障排查和调试至关重要，它们允许您看到崩溃时的系统状态，包括函数名称。Manticore Search 会在 searchd 日志中提供回溯，并在使用 --coredump 标志运行时生成 coredump。没有符号，您只能看到内部偏移，不易或无法解码崩溃原因。如果您需要关于崩溃的错误报告，Manticore 团队通常需要调试符号来协助您。

安装 Manticore Search/Manticore Columnar Library 调试符号，需要安装适合对应发行版的 `*debuginfo*` 包（CentOS）、`*dbgsym*` 包（Ubuntu 和 Debian）或 `*dbgsymbols*` 包（Windows 和 macOS）。这些包应与已安装的 Manticore 版本相同。例如，如果您在 CentOS 8 中从 https://repo.manticoresearch.com/repository/manticoresearch/release/centos/8/x86_64/manticore-4.0.2_210921.af497f245-1.el8.x86_64.rpm 安装了 Manticore Search，则符号包对应为 https://repo.manticoresearch.com/repository/manticoresearch/release/centos/8/x86_64/manticore-debuginfo-4.0.2_210921.af497f245-1.el8.x86_64.rpm

注意这两个包具有相同的提交 ID `af497f245`，对应构建该版本的提交。

如果您是通过 Manticore 的 APT/YUM 仓库安装的，可以使用以下工具之一：
* CentOS 7 的 `debuginfo-install`
* CentOS 8 的 `dnf debuginfo-install`
* Debian 和 Ubuntu 的 `find-dbgsym-packages`

来查找适合的调试符号包。

### 如何检查调试符号是否存在？

1. 在输出 `file /usr/bin/searchd` 中找到构建 ID：

```
[root@srv lib]# file /usr/bin/searchd
/usr/bin/searchd: ELF 64-bit LSB executable, x86-64, version 1 (GNU/Linux), dynamically linked (uses shared libs), for GNU/Linux 2.6.32, BuildID[sha1]=2c582e9f564ea1fbeb0c68406c271ba27034a6d3, stripped
```

在此示例中，构建 ID 是 2c582e9f564ea1fbeb0c68406c271ba27034a6d3。

2. 在 `/usr/lib/debug/.build-id` 中找到类似这样的符号：

```
[root@srv ~]# ls -la /usr/lib/debug/.build-id/2c/582e9f564ea1fbeb0c68406c271ba27034a6d3*
lrwxrwxrwx. 1 root root 23 Nov  9 10:42 /usr/lib/debug/.build-id/2c/582e9f564ea1fbeb0c68406c271ba27034a6d3 -> ../../../../bin/searchd
lrwxrwxrwx. 1 root root 27 Nov  9 10:42 /usr/lib/debug/.build-id/2c/582e9f564ea1fbeb0c68406c271ba27034a6d3.debug -> ../../usr/bin/searchd.debug
```

### 上传您的数据

<!-- example s3 -->
为了修复您的错误，开发人员通常需要在本地重现它。为此，他们需要您的配置文件、表文件、binlog（如果存在），有时还需要源数据（例如来自外部存储或 XML/CSV 文件的数据）以及查询。

当您在 [Github 上创建工单](https://github.com/manticoresoftware/manticoresearch/issues/new) 时，请附上您的数据。如果数据过大或包含敏感信息，您可以上传到我们的只写 S3 存储 `s3://s3.manticoresearch.com/write-only/`。

为了方便操作，我们提供了一个使用 Docker 镜像的上传机制。该镜像从我们的开源仓库 [github.com/manticoresoftware/s3-upload](https://github.com/manticoresoftware/s3-upload) 构建，帮助您轻松上传数据到 Manticore 的只写 S3 存储。具体操作如下：
1. 进入包含您想上传的文件的目录，然后执行：
   ```bash
   docker run -it --rm -v $(pwd):/upload manticoresearch/upload
   ```
2. 该命令将会：
   - 询问您输入相关的工单 URL 或编号
   - 上传**当前目录中的所有文件**到我们的只写 S3 存储
   - 最后，您会看到一个上传路径。请将该路径分享给开发人员。

<!-- intro -->
示例：

<!-- request Example -->
```bash
docker run -it --rm -v $(pwd):/upload manticoresearch/upload
```

<!-- response Example -->
```bash
🚀 Welcome to Manticore Search Upload Tool! 🚀

📂 Files to be uploaded:
  tt (800)

🔗 Please enter the related issue URL/number
(e.g., https://github.com/manticoresoftware/manticoresearch/issues/123 or just 123):
123

📤 Starting upload process...
INFO: Cache file not found or empty, creating/populating it.
INFO: Compiling list of local files...
INFO: Running stat() and reading/calculating MD5 values on 23 files, this may take some time...
INFO: Summary: 23 local files to upload
upload: './tt/tt.0.spa' -> 's3://write-only/issue-20250219-123/tt/tt.0.spa'  [1 of 23]
 40 of 40   100% in    2s    15.03 B/s  done
upload: './tt/tt.0.spd' -> 's3://write-only/issue-20250219-123/tt/tt.0.spd'  [2 of 23]
 1 of 1   100% in    0s     1.99 B/s  done
upload: './tt/tt.0.spe' -> 's3://write-only/issue-20250219-123/tt/tt.0.spe'  [3 of 23]
 1 of 1   100% in    0s     2.02 B/s  done
upload: './tt/tt.0.sph' -> 's3://write-only/issue-20250219-123/tt/tt.0.sph'  [4 of 23]
 420 of 420   100% in    0s   895.32 B/s  done
upload: './tt/tt.0.sphi' -> 's3://write-only/issue-20250219-123/tt/tt.0.sphi'  [5 of 23]
 66 of 66   100% in    0s   142.67 B/s  done
upload: './tt/tt.0.spi' -> 's3://write-only/issue-20250219-123/tt/tt.0.spi'  [6 of 23]
 18 of 18   100% in    0s    39.13 B/s  done
upload: './tt/tt.0.spidx' -> 's3://write-only/issue-20250219-123/tt/tt.0.spidx'  [7 of 23]
 145 of 145   100% in    0s   313.38 B/s  done
upload: './tt/tt.0.spm' -> 's3://write-only/issue-20250219-123/tt/tt.0.spm'  [8 of 23]
 4 of 4   100% in    0s     8.36 B/s  done
upload: './tt/tt.0.spp' -> 's3://write-only/issue-20250219-123/tt/tt.0.spp'  [9 of 23]
 1 of 1   100% in    0s     2.15 B/s  done
upload: './tt/tt.0.spt' -> 's3://write-only/issue-20250219-123/tt/tt.0.spt'  [10 of 23]
 36 of 36   100% in    0s    78.35 B/s  done
upload: './tt/tt.1.spa' -> 's3://write-only/issue-20250219-123/tt/tt.1.spa'  [11 of 23]
 48 of 48   100% in    0s    81.35 B/s  done
upload: './tt/tt.1.spd' -> 's3://write-only/issue-20250219-123/tt/tt.1.spd'  [12 of 23]
 1 of 1   100% in    0s     1.65 B/s  done
upload: './tt/tt.1.spe' -> 's3://write-only/issue-20250219-123/tt/tt.1.spe'  [13 of 23]
 1 of 1   100% in    0s     1.95 B/s  done
upload: './tt/tt.1.sph' -> 's3://write-only/issue-20250219-123/tt/tt.1.sph'  [14 of 23]
 420 of 420   100% in    0s   891.58 B/s  done
upload: './tt/tt.1.sphi' -> 's3://write-only/issue-20250219-123/tt/tt.1.sphi'  [15 of 23]
 82 of 82   100% in    0s   166.42 B/s  done
upload: './tt/tt.1.spi' -> 's3://write-only/issue-20250219-123/tt/tt.1.spi'  [16 of 23]
 18 of 18   100% in    0s    39.46 B/s  done
upload: './tt/tt.1.spidx' -> 's3://write-only/issue-20250219-123/tt/tt.1.spidx'  [17 of 23]
 183 of 183   100% in    0s   374.04 B/s  done
upload: './tt/tt.1.spm' -> 's3://write-only/issue-20250219-123/tt/tt.1.spm'  [18 of 23]
 4 of 4   100% in    0s     8.42 B/s  done
upload: './tt/tt.1.spp' -> 's3://write-only/issue-20250219-123/tt/tt.1.spp'  [19 of 23]
 1 of 1   100% in    0s     1.28 B/s  done
upload: './tt/tt.1.spt' -> 's3://write-only/issue-20250219-123/tt/tt.1.spt'  [20 of 23]
 50 of 50   100% in    1s    34.60 B/s  done
upload: './tt/tt.lock' -> 's3://write-only/issue-20250219-123/tt/tt.lock'  [21 of 23]
 0 of 0     0% in    0s     0.00 B/s  done
upload: './tt/tt.meta' -> 's3://write-only/issue-20250219-123/tt/tt.meta'  [22 of 23]
 456 of 456   100% in    0s   923.34 B/s  done
upload: './tt/tt.settings' -> 's3://write-only/issue-20250219-123/tt/tt.settings'  [23 of 23]
 3 of 3   100% in    0s     6.41 B/s  done

✅ Upload complete!
📋 Please share this path with the developers:
issue-20250219-123

💡 Tip: Make sure to include this path when communicating with the Manticore team
```
<!-- end -->

或者，您也可以使用 S3 的 [Minio 客户端](https://min.io/docs/minio/linux/reference/minio-mc.html) 或 Amazon 的 [s3cmd](https://s3tools.org/s3cmd) 工具实现相同操作，例如：

1. 安装客户端 https://min.io/docs/minio/linux/reference/minio-mc.html#install-mc
例如在 64 位 Linux 系统上：
   ```
   curl https://dl.min.io/client/mc/release/linux-amd64/mc \
   --create-dirs \
   -o $HOME/minio-binaries/mc
   chmod +x $HOME/minio-binaries/mc
   export PATH=$PATH:$HOME/minio-binaries/
   ```
2. 添加我们的 s3 主机（使用完整的可执行文件路径或切换到其目录）：`cd $HOME/minio-binaries`，然后执行 `./mc config host add manticore http://s3.manticoresearch.com:9000 manticore manticore`
3. 复制您的文件（使用完整的可执行文件路径或切换到其目录）：`cd $HOME/minio-binaries`，然后执行 `./mc cp -r issue-1234/ manticore/write-only/issue-1234`。确保文件夹名称是唯一的，最好对应您在 GitHub 上描述该错误的工单编号。

### DEBUG

```sql
DEBUG [ subcommand ]
```

`DEBUG` 语句是为开发者和测试人员调用各种内部或 VIP 命令而设计的。但它不适用于生产环境，因为 `subcommand` 部分的语法可能在任何版本中随意更改。

要查看当前上下文中可用的有用命令和 `DEBUG` 语句子命令列表，只需调用不带任何参数的 `DEBUG`。

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

来自 VIP 连接的同样内容：
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

所有 `debug XXX` 命令都应视为不稳定且可能随时修改，所以如果它们发生了变化，不要感到惊讶。此示例输出可能不反映实际可用的命令，建议您在自己的系统上尝试，以查看您的实例可用哪些命令。此外，除了这列简短的“含义”说明外，没有提供详细文档。

作为一个简单示例，这里描述了仅对 VIP 客户端可用的两个命令 —— shutdown 和 crash。两者都需要令牌，该令牌可以通过 `debug token` 子命令生成，并添加到配置文件中 searchd 部分的 [shutdown_token](Server_settings/Searchd.md#shutdown_token) 参数中。如果该部分不存在，或者提供的密码哈希与配置中存储的令牌不匹配，则子命令不会执行任何操作。

```sql
mysql> debug token hello;
+-------------+------------------------------------------+
| command     | result                                   |
+-------------+------------------------------------------+
| debug token | aaf4c61ddcc5e8a2dabede0f3b482cd9aea9434d |
+-------------+------------------------------------------+
1 row in set (0,00 sec)
```

`shutdown` 子命令会向服务器发送 TERM 信号，导致其关闭。这可能很危险，因为没有人希望意外停止生产服务。因此它需要 VIP 连接和密码验证。

`crash` 子命令字面意思是导致崩溃。它可用于测试，例如测试系统管理器如何维持服务存活性，或测试追踪核心转储的可行性。

如果某些命令在更广泛的上下文中被发现有用，它们可能会从调试子命令迁移到更稳定且通用的位置（如表格中示例的 `debug tasks` 和 `debug sched`）。

<!-- proofread -->

