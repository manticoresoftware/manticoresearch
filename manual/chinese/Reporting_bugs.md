# 报告错误

不幸的是，Manticore 还没有达到 100% 无错误，尽管开发团队正在为此目标努力工作。您可能会不时遇到一些问题。
报告每个错误时尽可能多地提供信息对于有效修复错误至关重要。
要修复错误，必须重现并修复它，或者根据您提供的信息推断其原因。为此，请遵循以下说明。

### 错误跟踪器
错误和功能请求在 [Github](https://github.com/manticoresoftware/manticore/issues) 上跟踪。欢迎您创建新工单，详细描述您的错误，以节省您和开发人员的时间。

### 文档更新
文档的更新（您现在正在阅读的内容）也在 [Github](https://github.com/manticoresoftware/manticoresearch/tree/master/manual) 上进行。

### 崩溃

Manticore Search 是用 C++ 编写的，这是一种低级编程语言，允许与计算机直接通信以获得更快的性能。然而，这种方法有一个缺点，在极少数情况下，可能无法通过写错误日志并跳过导致问题的命令来优雅地处理错误。相反，程序可能会崩溃，导致完全停止并需要重新启动。

当 Manticore Search 崩溃时，重要的是通过在 GitHub 上[提交错误报告](https://github.com/manticoresoftware/manticoresearch/issues/new?assignees=&labels=&template=bug_report.md&title=)或通过您私有帮助台中的 Manticore 专业服务让 Manticore 团队知道。Manticore 团队需要以下信息：

1. searchd 日志
2. coredump
3. 查询日志

此外，如果您能做以下操作将会很有帮助：
1. 运行 gdb 检查 coredump：
```
gdb /usr/bin/searchd </path/to/coredump>
```
2. 在 coredump 文件名中找到崩溃线程 ID（确保您在 /proc/sys/kernel/core_pattern 中有 `%p`），例如 `core.work_6.29050.server_name.1637586599` 表示 thread_id=29050
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

### 当 Manticore Search 卡住时我该怎么办？

如果 Manticore Search 卡住，您需要收集一些可能有助于理解原因的信息。方法如下：

1. 通过 [VIP 端口](https://manual.manticoresearch.com/Connecting_to_the_server/HTTP#VIP-connection) 运行 `show threads option format=all`
2. 收集 lsof 输出，因为卡住可能是由过多连接或打开的文件描述符引起的。
```
lsof -p `cat /run/manticore/searchd.pid`
```
3. 转储 core：
```
gcore `cat /run/manticore/searchd.pid`
```
（它会将转储保存到当前目录。）

4. 安装并运行 gdb：
```
gdb /usr/bin/searchd `cat /run/manticore/searchd.pid`
```
注意，这将暂停您正在运行的 searchd，但如果它已经卡住，这不应成为问题。
5. 在 gdb 中运行：
```
set pagination off
info threads
thread apply all bt
quit
```
6. 收集所有输出和文件，并在错误报告中提供。

对于专家：在[此提交](https://github.com/manticoresoftware/manticoresearch/commit/e317f7aa30aad51cb19d34595458bb7c8811be21)中添加的宏在调试时可能会有帮助。

### 如何启用崩溃时保存 coredump？

* 确保您使用 `--coredump` 运行 searchd。为了避免修改脚本，您可以使用 https://manual.manticoresearch.com/Starting_the_server/Linux#Custom-startup-flags-using-systemd 方法。例如：

```
[root@srv lib]# systemctl set-environment _ADDITIONAL_SEARCHD_PARAMS='--coredump'
[root@srv lib]# systemctl restart manticore
[root@srv lib]# ps aux|grep searchd
mantico+  1955  0.0  0.0  61964  1580 ?        S    11:02   0:00 /usr/bin/searchd --config /etc/manticoresearch/manticore.conf --coredump
mantico+  1956  0.6  0.0 392744  2664 ?        Sl   11:02   0:00 /usr/bin/searchd --config /etc/manticoresearch/manticore.conf --coredump
```

* 确保您的操作系统允许保存 core dump，检查 `/proc/sys/kernel/core_pattern` 不为空。这是保存 core dump 的位置。要将 core dump 保存到如 `core.searchd.1773.centos-4gb-hel1-1.1636454937` 的文件，请运行以下命令：
```
echo "/cores/core.%e.%p.%h.%t" > /proc/sys/kernel/core_pattern
```

* searchd 应该以 `ulimit -c unlimited` 启动。如果您使用 systemctl 启动 Manticore，它会自动将限制设置为无限，如 manticore.service 文件中的以下行所示：
```
[root@srv lib]# grep CORE /lib/systemd/system/manticore.service
LimitCORE=infinity
```

### 如何安装调试符号？

Manticore Search 和 Manticore Columnar Library 是用 C++ 编写的，生成的二进制文件在您的操作系统上以最佳方式执行。然而，运行二进制文件时，系统无法完全访问变量、函数、方法和类的名称。这些信息包含在单独的“调试信息”或“符号包”中。

调试符号对于故障排除和调试至关重要，因为它们允许您在崩溃时可视化系统状态，包括函数名称。Manticore Search 在 searchd 日志中提供回溯，并在使用 --coredump 标志运行时生成 coredump。没有符号，您只能看到内部偏移量，难以或无法解码崩溃原因。如果您需要提交崩溃的错误报告，Manticore 团队通常需要调试符号来协助您。

要安装 Manticore Search/Manticore Columnar Library 的调试符号，您需要为 CentOS 安装 `*debuginfo*` 包，为 Ubuntu 和 Debian 安装 `*dbgsym*` 包，或为 Windows 和 macOS 安装 `*dbgsymbols*` 包。这些包应与已安装的 Manticore 版本相同。例如，如果您在 Centos 8 中从包 https://repo.manticoresearch.com/repository/manticoresearch/release/centos/8/x86_64/manticore-4.0.2_210921.af497f245-1.el8.x86_64.rpm 安装了 Manticore Search，则对应的符号包为 https://repo.manticoresearch.com/repository/manticoresearch/release/centos/8/x86_64/manticore-debuginfo-4.0.2_210921.af497f245-1.el8.x86_64.rpm

请注意，这两个包具有相同的提交 ID `af497f245`，对应于此版本构建所基于的提交。

如果您是从 Manticore APT/YUM 仓库安装的 Manticore，可以使用以下工具之一：
* CentOS 7 中的 `debuginfo-install`
* CentOS 8 中的 `dnf debuginfo-install`
* Debian 和 Ubuntu 中的 `find-dbgsym-packages`

来查找适合您的调试符号包。

### 如何检查调试符号是否存在？

1. 在 `file /usr/bin/searchd` 的输出中查找构建 ID：

```
[root@srv lib]# file /usr/bin/searchd
/usr/bin/searchd: ELF 64-bit LSB executable, x86-64, version 1 (GNU/Linux), dynamically linked (uses shared libs), for GNU/Linux 2.6.32, BuildID[sha1]=2c582e9f564ea1fbeb0c68406c271ba27034a6d3, stripped
```

在此示例中，构建 ID 是 2c582e9f564ea1fbeb0c68406c271ba27034a6d3。

2. 在 `/usr/lib/debug/.build-id` 中找到如下符号：

```
[root@srv ~]# ls -la /usr/lib/debug/.build-id/2c/582e9f564ea1fbeb0c68406c271ba27034a6d3*
lrwxrwxrwx. 1 root root 23 Nov  9 10:42 /usr/lib/debug/.build-id/2c/582e9f564ea1fbeb0c68406c271ba27034a6d3 -> ../../../../bin/searchd
lrwxrwxrwx. 1 root root 27 Nov  9 10:42 /usr/lib/debug/.build-id/2c/582e9f564ea1fbeb0c68406c271ba27034a6d3.debug -> ../../usr/bin/searchd.debug
```

### 上传您的数据

<!-- example s3 -->
为了修复您的错误，开发人员通常需要在本地重现它。为此，他们需要您的配置文件、表文件、binlog（如果存在），有时还需要源数据（例如来自外部存储或 XML/CSV 文件的数据）和查询。

当您[在 Github 上创建工单](https://github.com/manticoresoftware/manticoresearch/issues/new)时，请附上您的数据。如果数据过大或敏感，您可以将其上传到我们的只写 S3 存储，地址为 `s3://s3.manticoresearch.com/write-only/`。

为了方便操作，我们提供了一个使用 Docker 镜像的上传机制。该镜像基于我们的开源仓库 [github.com/manticoresoftware/s3-upload](https://github.com/manticoresoftware/s3-upload) 构建，帮助您轻松上传数据到 Manticore 的只写 S3 存储。操作步骤如下：
1. 进入包含您要上传文件的目录，运行：
   ```bash
   docker run -it --rm -v $(pwd):/upload manticoresearch/upload
   ```
2. 该命令将会：
   - 询问您输入相关问题的 URL/编号
   - 上传**当前目录下的所有文件**到我们的只写 S3 存储
   - 最后，您将看到一个上传路径。请将此路径分享给开发人员。

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

另外，您也可以使用 S3 的 [Minio 客户端](https://min.io/docs/minio/linux/reference/minio-mc.html) 或 Amazon 的 [s3cmd](https://s3tools.org/s3cmd) 工具实现同样功能，例如：

1. 安装客户端 https://min.io/docs/minio/linux/reference/minio-mc.html#install-mc
例如在 64 位 Linux 上：
   ```
   curl https://dl.min.io/client/mc/release/linux-amd64/mc \
   --create-dirs \
   -o $HOME/minio-binaries/mc
   chmod +x $HOME/minio-binaries/mc
   export PATH=$PATH:$HOME/minio-binaries/
   ```
2. 添加我们的 s3 主机（使用可执行文件的完整路径或切换到其目录）：`cd $HOME/minio-binaries`，然后执行 `./mc config host add manticore http://s3.manticoresearch.com:9000 manticore manticore`
3. 复制您的文件（使用可执行文件的完整路径或切换到其目录）：`cd $HOME/minio-binaries`，然后执行 `./mc cp -r issue-1234/ manticore/write-only/issue-1234`。确保文件夹名称唯一，最好与您在 GitHub 上描述该错误的问题编号对应。

### DEBUG

```sql
DEBUG [ subcommand ]
```

`DEBUG` 语句设计供开发人员和测试人员调用各种内部或 VIP 命令。然而，它不适合生产环境使用，因为 `subcommand` 组件的语法可能在任何构建版本中自由更改。

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

VIP 连接中同样适用：
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

所有 `debug XXX` 命令都应视为不稳定，随时可能修改，因此如果它们发生变化，请不要感到惊讶。此示例输出可能不反映实际可用命令，建议您在自己的系统上尝试以查看实例中可用的命令。此外，除了这简短的“含义”列外，没有提供详细文档。

作为快速说明，下面描述了两个仅对 VIP 客户端可用的命令——shutdown 和 crash。两者都需要一个令牌，该令牌可以通过 debug token 子命令生成，并添加到配置文件 searchd 部分的 [shutdown_token](Server_settings/Searchd.md#shutdown_token) 参数中。如果不存在该部分，或者提供的密码哈希与配置中存储的令牌不匹配，则子命令不会执行任何操作。

```sql
mysql> debug token hello;
+-------------+------------------------------------------+
| command     | result                                   |
+-------------+------------------------------------------+
| debug token | aaf4c61ddcc5e8a2dabede0f3b482cd9aea9434d |
+-------------+------------------------------------------+
1 row in set (0,00 sec)
```

子命令 `shutdown` 会向服务器发送 TERM 信号，导致其关闭。这可能很危险，因为没人希望意外停止生产服务。因此，它需要 VIP 连接和密码才能使用。

子命令 `crash` 会导致服务器崩溃。它可用于测试目的，例如测试系统管理器如何维护服务的存活性，或测试跟踪核心转储的可行性。

如果某些命令在更广泛的上下文中被发现有用，它们可能会从调试子命令移至更稳定和通用的位置（如表中示例的 `debug tasks` 和 `debug sched`）。

<!-- proofread -->

