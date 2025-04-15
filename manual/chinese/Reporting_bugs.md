# 报告错误

不幸的是，Manticore 还不是 100% 无错误的，尽管开发团队正在努力追求这个目标。您可能会不时遇到一些问题。 
报告每个错误时提供尽可能多的信息以有效修复是至关重要的。
要修复一个错误，要么需要重现并修复它，要么需要根据您提供的信息推测其原因。为此，请遵循以下说明。

### 错误追踪器
错误和功能请求在 [Github](https://github.com/manticoresoftware/manticore/issues) 上被跟踪。欢迎您创建一个新工单，并详细描述您的错误，以便为您和开发人员节省时间。

### 文档更新
文档（您现在正在阅读的内容）的更新也在 [Github](https://github.com/manticoresoftware/manticoresearch/tree/master/manual) 上进行。

### 崩溃

Manticore Search 是用 C++ 编写的，这是一种低级编程语言，允许与计算机进行直接通信以实现更快的性能。但是，这种方法也有一个缺点，因为在极少数情况下，可能无法通过将错误写入日志并跳过导致问题的命令的处理来优雅地处理错误。相反，程序可能会崩溃，导致它完全停止并需要重新启动。

当 Manticore Search 崩溃时，重要的是通过 [提交错误报告](https://github.com/manticoresoftware/manticoresearch/issues/new?assignees=&labels=&template=bug_report.md&title=) 在 GitHub 或通过 Manticore 的专业服务在您的私人帮助台通知 Manticore 团队。Manticore 团队需要以下信息：

1. searchd 日志
2. core dump
3. 查询日志

此外，如果您能做以下事情，将会很有帮助：
1. 运行 gdb 以检查 core dump：
```
gdb /usr/bin/searchd </path/to/coredump>
```
2. 在 core dump 文件名中找到崩溃线程 ID（确保您在 /proc/sys/kernel/core_pattern 中有 `%p`），例如 `core.work_6.29050.server_name.1637586599` 意味着 thread_id=29050
3. 在 gdb 中运行：
```
set pagination off
info threads
# 根据线程 id 寻找线程编号（例如，对于 `LWP 29050`，它将是线程编号 8
thread apply all bt
thread <线程编号>
bt full
info locals
quit
```
4. 提供输出

### 当 Manticore Search 停顿时我该怎么办？

如果 Manticore Search 停顿，您需要收集一些可能有助于理解原因的信息。以下是您可以执行的操作：

1. 通过 [VIP 端口](https://manual.manticoresearch.com/Connecting_to_the_server/HTTP#VIP-connection) 运行 `show threads option format=all`
2. 收集 lsof 输出，因为停顿可能是由于太多连接或打开的文件描述符导致的。
```
lsof -p `cat /var/run/manticore/searchd.pid`
```
3. 转储 core：
```
gcore `cat /var/run/manticore/searchd.pid`
```
（它会将转储保存到当前目录。）

4. 安装并运行 gdb：
```
gdb /usr/bin/searchd `cat /var/run/manticore/searchd.pid`
```
请注意，这将会暂停您正在运行的 searchd，但如果它已经停顿，它不应该是个问题。
5. 在 gdb 中运行：
```
set pagination off
info threads
thread apply all bt
quit
```
6. 收集所有输出和文件，并提供它们以便进行错误报告。

对于专家：在 [此提交](https://github.com/manticoresoftware/manticoresearch/commit/e317f7aa30aad51cb19d34595458bb7c8811be21) 中添加的宏可以在调试时很有帮助。

### 如何在崩溃时启用保存 core dumps？

* 确保您以 `--coredump` 运行 searchd。为了避免修改脚本，您可以使用 https://manual.manticoresearch.com/Starting_the_server/Linux#Custom-startup-flags-using-systemd 方法。例如：

```
[root@srv lib]# systemctl set-environment _ADDITIONAL_SEARCHD_PARAMS='--coredump'
[root@srv lib]# systemctl restart manticore
[root@srv lib]# ps aux|grep searchd
mantico+  1955  0.0  0.0  61964  1580 ?        S    11:02   0:00 /usr/bin/searchd --config /etc/manticoresearch/manticore.conf --coredump
mantico+  1956  0.6  0.0 392744  2664 ?        Sl   11:02   0:00 /usr/bin/searchd --config /etc/manticoresearch/manticore.conf --coredump
```

* 确保您的操作系统允许您通过检查 `/proc/sys/kernel/core_pattern` 不为空来保存 core dumps。这是将保存 core dumps 的位置。要将 core dumps 保存到文件，例如 `core.searchd.1773.centos-4gb-hel1-1.1636454937`，请运行以下命令：
```
echo "/cores/core.%e.%p.%h.%t" > /proc/sys/kernel/core_pattern
```

* searchd 应该以 `ulimit -c unlimited` 启动。如果您使用 systemctl 启动 Manticore，它会自动将限制设置为无限，如 manticore.service 文件中的以下行所示：
```
[root@srv lib]# grep CORE /lib/systemd/system/manticore.service
LimitCORE=infinity
```

### 如何安装调试符号？

Manticore Search 和 Manticore Columnar Library 是用 C++ 编写的，生成的编译二进制文件能够在您的操作系统上最佳执行。然而，在运行二进制文件时，您的系统并不能完全访问变量、函数、方法和类的名称。这些信息提供在单独的“调试信息”或“符号包”中。

调试符号对于故障排除和调试是必不可少的，因为它们允许您在崩溃时可视化系统状态，包括函数名称。Manticore Search 在 searchd 日志中提供回溯，并在使用 --coredump 标志运行时生成 core dump。没有符号，您看到的仅仅是内部偏移量，这使得解码崩溃原因变得困难或不可能。如果您需要对崩溃进行错误报告，Manticore 团队通常会要求调试符号以协助您。
要安装 Manticore Search/Manticore Columnar Library 调试符号,你需要安装对应的包:CentOS 系统需要安装 `*debuginfo*` 包,Ubuntu 和 Debian 需要安装 `*dbgsym*` 包,Windows 和 macOS 需要安装 `*dbgsymbols*` 包。这些包的版本应该与已安装的 Manticore 版本一致。例如,如果你在 CentOS 8 中从以下包安装了 Manticore Search: https://repo.manticoresearch.com/repository/manticoresearch/release/centos/8/x86_64/manticore-4.0.2_210921.af497f245-1.el8.x86_64.rpm ,对应的调试符号包应该是 https://repo.manticoresearch.com/repository/manticoresearch/release/centos/8/x86_64/manticore-debuginfo-4.0.2_210921.af497f245-1.el8.x86_64.rpm
请注意两个包都有相同的提交 ID `af497f245`,这对应于构建此版本的提交。
如果你从 Manticore APT/YUM 仓库安装了 Manticore,你可以使用以下工具之一:
* CentOS 7 中的 `debuginfo-install` 
* CentOS 8 中的 `dnf debuginfo-install`
* Debian 和 Ubuntu 中的 `find-dbgsym-packages`
来为你找到调试符号包。
### 如何检查调试符号是否存在?
1. 在 `file /usr/bin/searchd` 的输出中找到构建 ID:
```
[root@srv lib]# file /usr/bin/searchd
/usr/bin/searchd: ELF 64-bit LSB executable, x86-64, version 1 (GNU/Linux), dynamically linked (uses shared libs), for GNU/Linux 2.6.32, BuildID[sha1]=2c582e9f564ea1fbeb0c68406c271ba27034a6d3, stripped
```
在这个例子中,构建 ID 是 2c582e9f564ea1fbeb0c68406c271ba27034a6d3。
2. 在 `/usr/lib/debug/.build-id` 中查找符号,如下所示:
```
[root@srv ~]# ls -la /usr/lib/debug/.build-id/2c/582e9f564ea1fbeb0c68406c271ba27034a6d3*
lrwxrwxrwx. 1 root root 23 Nov  9 10:42 /usr/lib/debug/.build-id/2c/582e9f564ea1fbeb0c68406c271ba27034a6d3 -> ../../../../bin/searchd
lrwxrwxrwx. 1 root root 27 Nov  9 10:42 /usr/lib/debug/.build-id/2c/582e9f564ea1fbeb0c68406c271ba27034a6d3.debug -> ../../usr/bin/searchd.debug
```
### 上传你的数据
<!-- example s3 -->
为了修复你的 bug,开发人员通常需要在本地复现它。为此,他们需要你的配置文件、表文件、binlog(如果存在),有时还需要源数据(如来自外部存储或 XML/CSV 文件的数据)和查询。
在[Github 上创建工单](https://github.com/manticoresoftware/manticoresearch/issues/new)时附上你的数据。如果数据太大或敏感,你可以将其上传到我们的只写 S3 存储 `s3://s3.manticoresearch.com/write-only/`。
为了方便上传,我们提供了一个使用 Docker 镜像的上传机制。这个镜像基于我们在 [github.com/manticoresoftware/s3-upload](https://github.com/manticoresoftware/s3-upload) 的开源仓库构建,可以帮助你轻松地将数据上传到 Manticore 的只写 S3 存储。以下是操作方法:
1. 导航到包含要上传文件的目录并运行:
 ```bash
 docker run -it --rm -v $(pwd):/upload manticoresearch/upload
 ```
2. 这将:
 - 要求你输入相关 issue URL/编号
 - 将**当前目录中的所有文件**上传到我们的只写 S3 存储
 - 最后,你会看到一个上传路径。请将此路径分享给开发人员。
<!-- intro -->
示例:
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
0 of 40   100% in    2s    15.03 B/s  done
upload: './tt/tt.0.spd' -> 's3://write-only/issue-20250219-123/tt/tt.0.spd'  [2 of 23]
 of 1   100% in    0s     1.99 B/s  done
upload: './tt/tt.0.spe' -> 's3://write-only/issue-20250219-123/tt/tt.0.spe'  [3 of 23]
 of 1   100% in    0s     2.02 B/s  done
upload: './tt/tt.0.sph' -> 's3://write-only/issue-20250219-123/tt/tt.0.sph'  [4 of 23]
20 of 420   100% in    0s   895.32 B/s  done
upload: './tt/tt.0.sphi' -> 's3://write-only/issue-20250219-123/tt/tt.0.sphi'  [5 of 23]
6 of 66   100% in    0s   142.67 B/s  done
upload: './tt/tt.0.spi' -> 's3://write-only/issue-20250219-123/tt/tt.0.spi'  [6 of 23]
8 of 18   100% in    0s    39.13 B/s  done
upload: './tt/tt.0.spidx' -> 's3://write-only/issue-20250219-123/tt/tt.0.spidx'  [7 of 23]
45 of 145   100% in    0s   313.38 B/s  done
upload: './tt/tt.0.spm' -> 's3://write-only/issue-20250219-123/tt/tt.0.spm'  [8 of 23]
 of 4   100% in    0s     8.36 B/s  done
upload: './tt/tt.0.spp' -> 's3://write-only/issue-20250219-123/tt/tt.0.spp'  [9 of 23]
 of 1   100% in    0s     2.15 B/s  done
upload: './tt/tt.0.spt' -> 's3://write-only/issue-20250219-123/tt/tt.0.spt'  [10 of 23]
6 of 36   100% in    0s    78.35 B/s  done
upload: './tt/tt.1.spa' -> 's3://write-only/issue-20250219-123/tt/tt.1.spa'  [11 of 23]
8 of 48   100% in    0s    81.35 B/s  done
upload: './tt/tt.1.spd' -> 's3://write-only/issue-20250219-123/tt/tt.1.spd'  [12 of 23]
 of 1   100% in    0s     1.65 B/s  done
upload: './tt/tt.1.spe' -> 's3://write-only/issue-20250219-123/tt/tt.1.spe'  [13 of 23]
 of 1   100% in    0s     1.95 B/s  done
upload: './tt/tt.1.sph' -> 's3://write-only/issue-20250219-123/tt/tt.1.sph'  [14 of 23]
20 of 420   100% in    0s   891.58 B/s  done
upload: './tt/tt.1.sphi' -> 's3://write-only/issue-20250219-123/tt/tt.1.sphi'  [15 of 23]
2 of 82   100% in    0s   166.42 B/s  done
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

✅ 上传完成！
📋 请将此路径分享给开发人员：
issue-20250219-123

💡 提示：与Manticore团队沟通时，请确保包含此路径
```
<!-- end -->

或者，您可以使用S3 [Minio客户端](https://min.io/docs/minio/linux/reference/minio-mc.html)或Amazon [s3cmd](https://s3tools.org/s3cmd)工具做同样的事情，例如：

1. 安装客户端 https://min.io/docs/minio/linux/reference/minio-mc.html#install-mc
例如在64位Linux上：
   ```
   curl https://dl.min.io/client/mc/release/linux-amd64/mc \
   --create-dirs \
   -o $HOME/minio-binaries/mc
   chmod +x $HOME/minio-binaries/mc
   export PATH=$PATH:$HOME/minio-binaries/
   ```
2. 添加我们的s3主机（使用可执行文件的完整路径或切换到其目录）：`cd $HOME/minio-binaries` 然后 `./mc config host add manticore http://s3.manticoresearch.com:9000 manticore manticore`
3. 复制您的文件（使用可执行文件的完整路径或切换到其目录）：`cd $HOME/minio-binaries` 然后 `./mc cp -r issue-1234/ manticore/write-only/issue-1234`。确保文件夹名称是唯一的，最好与您在GitHub上描述bug的issue相对应。

### DEBUG

```sql
DEBUG [ subcommand ]
```

`DEBUG`语句是为开发人员和测试人员设计的，用于调用各种内部或VIP命令。但是，它不适合生产使用，因为`subcommand`组件的语法可能会在任何版本中自由变化。

要查看在当前上下文中可用的有用命令和`DEBUG`语句子命令的列表，只需调用没有任何参数的`DEBUG`。

```sql
mysql> debug;
+-------------------------------------------------------------------------+----------------------------------------------------------------------------------------+
| command                                                                 | meaning                                                                                |
+-------------------------------------------------------------------------+----------------------------------------------------------------------------------------+
| flush logs                                                              | 模拟USR1信号                                                                         |
| reload indexes                                                          | 模拟HUP信号                                                                          |
| debug token <password>                                                  | 计算密码的令牌                                                                       |
| debug malloc_stats                                                      | 执行'malloc_stats'，结果在searchd.log中                                           |
| debug malloc_trim                                                       | 执行'malloc_trim'调用                                                                  |
| debug sleep <N>                                                         | 睡眠<N>秒                                                                             |
| debug tasks                                                             | 显示全局任务状态（使用 select from @@system.tasks 代替）                          |
| debug sched                                                             | 显示任务管理器调度（使用 select from @@system.sched 代替）                       |
| debug merge <TBL> [chunk] <X> [into] [chunk] <Y> [option sync=1,byid=0] | 对于RT表<TBL>将磁盘块X合并到磁盘块Y                                               |
| debug drop [chunk] <X> [from] <TBL> [option sync=1]                     | 对于RT表<TBL>删除磁盘块X                                                          |
| debug files <TBL> [option format=all|external]                          | 列出属于<TBL>的文件。'all' - 包括外部文件（词形，停用词等）                     |
| debug close                                                             | 请求服务器从其侧关闭连接                                                         |
| debug compress <TBL> [chunk] <X> [option sync=1]                        | 压缩RT表<TBL>的磁盘块X（删除已删除的文档）                                         |
| debug split <TBL> [chunk] <X> on @<uservar> [option sync=1]             | 使用来自@uservar的DocIDs集拆分RT表<TBL>的磁盘块X                                  |
| debug wait <cluster> [like 'xx'] [option timeout=3]                     | 等待<cluster>准备就绪，但不超过3秒。                                               |
| debug wait <cluster> status <N> [like 'xx'] [option timeout=13]         | 等待<cluster>提交达到<N>，但不超过13秒                                            |
| debug meta                                                              | 显示 max_matches/pseudo_shards。需要设置 profiling=1                                  |
| debug trace OFF|'path/to/file' [<N>]                                    | 将流量跟踪到文件，直到写入 N 字节，或 'trace OFF'                               |
| debug curl <URL>                                                        | 通过 libcurl 请求给定的 URL                                                          |
+-------------------------------------------------------------------------+----------------------------------------------------------------------------------------+
19 rows in set (0.00 sec)
```

Same from VIP connection:
```sql
mysql> debug;
+-------------------------------------------------------------------------+----------------------------------------------------------------------------------------+
| command                                                                 | meaning                                                                                |
+-------------------------------------------------------------------------+----------------------------------------------------------------------------------------+
| flush logs                                                              | 模拟 USR1 信号                                                                    |
| reload indexes                                                          | 模拟 HUP 信号                                                                     |
| debug shutdown <password>                                               | 模拟 TERM 信号                                                                    |
| debug crash <password>                                                  | 崩溃守护进程（执行 SIGSEGV 操作）                                                     |
| debug token <password>                                                  | 为密码计算令牌                                                           |
| debug malloc_stats                                                      | 执行 'malloc_stats'，结果在 searchd.log 中                                          |
| debug malloc_trim                                                       | 执行 'malloc_trim' 调用                                                             |
| debug procdump                                                          | 请求监视程序转储我们                                                                |
| debug setgdb on|off                                                     | 启用或禁用与 gdb 相关的潜在危险崩溃转储                         |
| debug setgdb status                                                     | 显示当前 gdb 转储模式                                                       |
| debug sleep <N>                                                         | 睡眠 <N> 秒                                                                  |
| debug tasks                                                             | 显示全局任务统计（而是使用从 @@system.tasks 选择）                     |
| debug sched                                                             | 显示任务管理器调度（而是使用从 @@system.sched 选择）                 |
| debug merge <TBL> [chunk] <X> [into] [chunk] <Y> [option sync=1,byid=0] | 对于 RT 表 <TBL> 将磁盘块 X 合并到磁盘块 Y                                |
| debug drop [chunk] <X> [from] <TBL> [option sync=1]                     | 对于 RT 表 <TBL> 删除磁盘块 X                                                   |
| debug files <TBL> [option format=all|external]                          | 列出属于 <TBL> 的文件。 'all' - 包括外部（词形、停用词等） |
| debug close                                                             | 请求服务器从其端关闭连接                                          |
| debug compress <TBL> [chunk] <X> [option sync=1]                        | 压缩 RT 表 <TBL> 的磁盘块 X（清除已删除的文档）                   |
| debug split <TBL> [chunk] <X> on @<uservar> [option sync=1]             | 使用来自 @uservar 的 DocIDs 集划分 RT 表 <TBL> 的磁盘块 X                 |
| debug wait <cluster> [like 'xx'] [option timeout=3]                     | 等待 <cluster> 准备，但不超过 3 秒。                                         |
| debug wait <cluster> status <N> [like 'xx'] [option timeout=13]         | 等待 <cluster> 提交达到 <N>，但不超过 13 秒                            |
| debug meta                                                              | 显示 max_matches/pseudo_shards。需要设置 profiling=1                                  |
| debug trace OFF|'path/to/file' [<N>]                                    | 将流量跟踪到文件，直到写入 N 字节，或 'trace OFF'                               |
| debug curl <URL>                                                        | 通过 libcurl 请求给定的 URL                                                          |
+-------------------------------------------------------------------------+----------------------------------------------------------------------------------------+
24 rows in set (0.00 sec)
```

All `debug XXX` commands should be regarded as non-stable and subject to modification at any time, so don't be surprised if they change. This example output may not reflect the actual available commands, so try it on your system to see what is available on your instance. Additionally, there is no detailed documentation provided aside from this short 'meaning' column.

作为一个快速的说明，下面描述的有两个仅对 VIP 客户端可用的命令 - shutdown 和 crash。两者都需要一个令牌，该令牌可以通过 debug token 子命令生成，并添加到配置文件 searchd 部分的 [shutdown_token](Server_settings/Searchd.md#shutdown_token) 参数中。如果没有这样的部分，或者提供的密码哈希与存储在配置中的令牌不匹配，则子命令将不会执行任何操作。

```sql
mysql> debug token hello;
+-------------+------------------------------------------+
| command     | result                                   |
+-------------+------------------------------------------+
| debug token | aaf4c61ddcc5e8a2dabede0f3b482cd9aea9434d |
+-------------+------------------------------------------+
1 row in set (0,00 sec)
```

子命令 `shutdown` 将向服务器发送一个 TERM 信号，导致其关闭。这可能是危险的，因为没有人想意外停止生产服务。因此，它需要 VIP 连接和要使用的密码。

子命令 `crash` 字面上会导致崩溃。它可以用于测试目的，例如测试系统管理器如何维护服务的存活性或测试跟踪核心转储的可行性。

如果发现某些命令在更一般的上下文中是有用的，它们可能会从 debug 子命令移动到一个更稳定和通用的位置（如表中示例的 `debug tasks` 和 `debug sched`）。

<!-- proofread -->




















