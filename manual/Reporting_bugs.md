# Reporting bugs

Unfortunately, Manticore is not yet 100% bug-free, although the development team is working hard towards that goal. You may encounter some issues from time to time. 
It is crucial to report as much information as possible about each bug to fix it effectively.
To fix a bug, either it needs to be reproduced and fixed or its cause needs to be deduced based on the information you provide. To help with this, please follow the instructions below.

### Bug-tracker
Bugs and feature requests are tracked on [Github](https://github.com/manticoresoftware/manticore/issues). You are welcome to create a new ticket and describe your bug in detail to save time for both you and the developers.

### Documentation updates
Updates to the documentation (what you are reading now) are also done on [Github](https://github.com/manticoresoftware/manticoresearch/tree/master/manual).

### Crashes

Manticore Search is written in C++, which is a low-level programming language that allows for direct communication with the computer for faster performance. However, there is a drawback to this approach as in rare cases, it may not be possible to elegantly handle a bug by writing an error to a log and skipping the processing of the command that caused the problem. Instead, the program may crash, resulting in it stopping completely and needing to be restarted.

When Manticore Search crashes, it is important to let the Manticore team know by [submitting a bug report](https://github.com/manticoresoftware/manticoresearch/issues/new?assignees=&labels=&template=bug_report.md&title=) on GitHub or through Manticore's professional services in your private helpdesk. The Manticore team requires the following information:

1. The searchd log
2. The coredump
3. The query log

Additionally, it would be helpful if you could do the following:
1. Run gdb to inspect the coredump:
```
gdb /usr/bin/searchd </path/to/coredump>
```
2. Find the crashed thread ID in the coredump file name (make sure you have `%p` in /proc/sys/kernel/core_pattern), e.g. `core.work_6.29050.server_name.1637586599` means thread_id=29050
3. In gdb run:
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
4. Provide the outputs

### What do I do when Manticore Search hangs?

If Manticore Search hangs, you need to collect some information that may be useful in understanding the cause. Here's how you can do it:

1. Run `show threads option format=all` trough a [VIP port](https://manual.manticoresearch.com/Connecting_to_the_server/HTTP#VIP-connection)
2. collect the lsof output output, as hanging can be caused by too many connections or open file descriptors.
```
lsof -p `cat /var/run/manticore/searchd.pid`
```
3. Dump the core:
```
gcore `cat /var/run/manticore/searchd.pid`
```
(It will save the dump to the current directory.)

4. Install and run gdb:
```
gdb /usr/bin/searchd `cat /var/run/manticore/searchd.pid`
```
Note that this will halt your running searchd, but if it's already hanging, it shouldn't be a problem.
5. In gdb run:
```
set pagination off
info threads
thread apply all bt
quit
```
6. Collect all the outputs and files and provide them in a bug report.

For experts: the macros added in [this commit](https://github.com/manticoresoftware/manticoresearch/commit/e317f7aa30aad51cb19d34595458bb7c8811be21) can be helpful in debugging.

### How to enable saving coredumps on crash?

* make sure you run searchd with `--coredump`. To avoid modifying scripts, you can use the  https://manual.manticoresearch.com/Starting_the_server/Linux#Custom-startup-flags-using-systemd , method. For example::

```
[root@srv lib]# systemctl set-environment _ADDITIONAL_SEARCHD_PARAMS='--coredump'
[root@srv lib]# systemctl restart manticore
[root@srv lib]# ps aux|grep searchd
mantico+  1955  0.0  0.0  61964  1580 ?        S    11:02   0:00 /usr/bin/searchd --config /etc/manticoresearch/manticore.conf --coredump
mantico+  1956  0.6  0.0 392744  2664 ?        Sl   11:02   0:00 /usr/bin/searchd --config /etc/manticoresearch/manticore.conf --coredump
```

* Ensure that your operating system allows you to save core dumps by checking that: `/proc/sys/kernel/core_pattern` is not empty. This is the location where the core dumps will be saved. To save core dumps to a file such as `core.searchd.1773.centos-4gb-hel1-1.1636454937`, run the following command:
```
echo "/cores/core.%e.%p.%h.%t" > /proc/sys/kernel/core_pattern
```

* searchd should be started with `ulimit -c unlimited`. If you start Manticore using systemctl, it will automatically set the limit to infinity as indicated by the following line in the manticore.service file:
```
[root@srv lib]# grep CORE /lib/systemd/system/manticore.service
LimitCORE=infinity
```

### How do I install debug symbols?

Manticore Search and Manticore Columnar Library are written in C++, which results in compiled binary files that execute optimally on your operating system. However, when running a binary, your system does not have full access to the names of variables, functions, methods, and classes. This information is provided in separate "debuginfo" or "symbol packages."

Debug symbols are essential for troubleshooting and debugging, as they allow you to visualize the system state when it crashed, including the names of functions. Manticore Search provides a backtrace in the searchd log and generates a coredump if run with the --coredump flag. Without symbols, all you will see is internal offsets, making it difficult or impossible to decode the cause of the crash. If you need to make a bug report about a crash, the Manticore team will often require debug symbols to assist you.

To install Manticore Search/Manticore Columnar Library debug symbols, you will need to install the `*debuginfo*` package for CentOS, the `*dbgsym*` package for Ubuntu and Debian, or the `*dbgsymbols*` package for Windows and macOS. These packages should be the same version as the installed Manticore. For example, if you installed Manticore Search in Centos 8 from the package https://repo.manticoresearch.com/repository/manticoresearch/release/centos/8/x86_64/manticore-4.0.2_210921.af497f245-1.el8.x86_64.rpm , the corresponding package with symbols would be https://repo.manticoresearch.com/repository/manticoresearch/release/centos/8/x86_64/manticore-debuginfo-4.0.2_210921.af497f245-1.el8.x86_64.rpm

Note that both packages have the same commit id `af497f245`, which corresponds to the commit that this version was built from.

If you have installed Manticore from a Manticore APT/YUM repository, you can use one of the following tools:
* `debuginfo-install` in CentOS 7
* `dnf debuginfo-install` CentOS 8
* `find-dbgsym-packages` in Debian and Ubuntu

to find a debug symbols package for you.

### How to check the debug symbols exist?

1. Find the build ID in the output of `file /usr/bin/searchd`:

```
[root@srv lib]# file /usr/bin/searchd
/usr/bin/searchd: ELF 64-bit LSB executable, x86-64, version 1 (GNU/Linux), dynamically linked (uses shared libs), for GNU/Linux 2.6.32, BuildID[sha1]=2c582e9f564ea1fbeb0c68406c271ba27034a6d3, stripped
```

In this case, the build ID is 2c582e9f564ea1fbeb0c68406c271ba27034a6d3.

2. Find symbols in `/usr/lib/debug/.build-id` like this:

```
[root@srv ~]# ls -la /usr/lib/debug/.build-id/2c/582e9f564ea1fbeb0c68406c271ba27034a6d3*
lrwxrwxrwx. 1 root root 23 Nov  9 10:42 /usr/lib/debug/.build-id/2c/582e9f564ea1fbeb0c68406c271ba27034a6d3 -> ../../../../bin/searchd
lrwxrwxrwx. 1 root root 27 Nov  9 10:42 /usr/lib/debug/.build-id/2c/582e9f564ea1fbeb0c68406c271ba27034a6d3.debug -> ../../usr/bin/searchd.debug
```

### Uploading your data

To fix your bug, developers often need to reproduce it locally. To do this, they need your configuration file, table files, binlog (if present), and sometimes source data (such as data from external storages or XML/CSV files) and queries.

Attach your data when you [create a ticket on Github](https://github.com/manticoresoftware/manticoresearch/issues/new). If the data is too large or sensitive, you can upload it to our write-only S3 storage at  `s3://s3.manticoresearch.com/write-only/`. Here's how you can do it using the Minio client:
1. Install the client https://min.io/docs/minio/linux/reference/minio-mc.html#install-mc
2. Add our s3 host: `mc config host add manticore http://s3.manticoresearch.com:9000 manticore manticore`
3. Copy your files: `mc cp -r issue-1234/ manticore/write-only/issue-1234` . Make sure the folder name is unique and best if it corresponds to the issue on GitHub where you described the bug.

### DEBUG

```sql
DEBUG [ subcommand ]
```

The `DEBUG` statement is designed for developers and testers to call various internal or VIP commands. However, it is not intended for production use as the syntax of the  `subcommand` component may change freely in any build.

To view a list of useful commands and `DEBUG` statement subcommands available in the current context, simply call `DEBUG` without any parameters.

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

All `debug XXX` commands should be regarded as non-stable and subject to modification at any time, so don't be surprised if they change. This example output may not reflect the actual available commands, so try it on your system to see what is available on your instance. Additionally, there is no detailed documentation provided aside from this short 'meaning' column.

As a quick illustration, two commands available only to VIP clients are described below - shutdown and crash. Both require a token, which can be generated with the debug token subcommand, and added to the [shutdown_token](Server_settings/Searchd.md#shutdown_token) param in the searchd section of the config file. If no such section exists, or if the provided password hash does not match the token stored in the config, the subcommands will do nothing.

```sql
mysql> debug token hello;
+-------------+------------------------------------------+
| command     | result                                   |
+-------------+------------------------------------------+
| debug token | aaf4c61ddcc5e8a2dabede0f3b482cd9aea9434d |
+-------------+------------------------------------------+
1 row in set (0,00 sec)
```

The subcommand `shutdown` will send a TERM signal to the server, causing it to shut down. This can be dangerous, as nobody wants to accidentally stop a production service. Therefore, it requires a VIP connection and the password to be used.

The subcommand `crash` literally causes a crash. It may be used for testing purposes, such as to test how the system manager maintains the service's liveness or to test the feasibility of tracking coredumps.

If some commands are found to be useful in a more general context, they may be moved from the debug subcommands to a more stable and generic location (as exemplified by the `debug tasks` and `debug sched` in the table).

<!-- proofread -->