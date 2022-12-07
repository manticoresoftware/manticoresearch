# Reporting bugs

Unfortunately, Manticore is not yet 100% bug free (even though we're working hard towards that), so you might occasionally run into some issues.

Reporting as much as possible about each bug is very important - because to fix it, we need to be able to either reproduce and fix the bug, or to deduce what's causing it from the information that you provide. So here are some instructions how to do that.

### Bug-tracker
We track bugs and feature requests in [Github](https://github.com/manticoresoftware/manticore/issues). Feel free to create a new ticket and describe your bug in details so both you and developers can save their time.

### Documentation updates
Updates to the documentation (what you are reading now) is also done on [Github](https://github.com/manticoresoftware/manticoresearch/tree/master/manual).

### Crashes
Manticore is written in C++ - low level programming language allowing to speak to the computer with not so many intermediate layers for faster performance. The drawback of that is that in rare cases there is no way to handle a bug elegantly writing the error about it to a log and skipping processing the command which caused the problem. Instead of that the program can just crash which means it would stop completely and would have to be restarted.

When Manticore Search crashes you need to let Manticore team know about that by [making a bug report](https://github.com/manticoresoftware/manticoresearch/issues/new?assignees=&labels=&template=bug_report.md&title=) on github or if you use Manticore's professional services in your private helpdesk. Manticore team needs the following:

1. searchd log
2. coredump
3. query log

It will be great if you additionally do the following:
1. run gdb to inspect the coredump:
```
gdb /usr/bin/searchd </path/to/coredump>
```
2. Find crashed thread id in the coredump file name (make sure you have `%p` in /proc/sys/kernel/core_pattern), e.g. `core.work_6.29050.server_name.1637586599` means thread_id=29050
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

You need to run gdb manually and collect some info that may be useful to understand why it's hanging.

1. `show threads option format=all` run trough a [VIP port](https://manual.manticoresearch.com/Connecting_to_the_server/HTTP#VIP-connection)
2. collect lsof output since hanging can be caused by too many connections or open file descriptors
```
lsof -p `cat /var/run/manticore/searchd.pid`
```
3. dump core
```
gcore `cat /var/run/manticore/searchd.pid`
```
(it will save the dump to the current dir)

4. Install and run gdb:
```
gdb /usr/bin/searchd `cat /var/run/manticore/searchd.pid`
```
Note it will halt your running searchd, but if it's alredy hanging it shouldn't be a problem.
5. In gdb run:
```
set pagination off
info threads
thread apply all bt
quit
```
6. Collect all the outputs and files and provide them in a bug report.

For experts: the macros added in [this commit](https://github.com/manticoresoftware/manticoresearch/commit/e317f7aa30aad51cb19d34595458bb7c8811be21) can be helpful to debug.

### How to enable saving coredumps on crash?

* make sure you run searchd with `--coredump`. To avoid hacking the scripts you can use this https://manual.manticoresearch.com/Starting_the_server/Linux#Custom-startup-flags-using-systemd , e.g.:

```
[root@srv lib]# systemctl set-environment _ADDITIONAL_SEARCHD_PARAMS='--coredump'
[root@srv lib]# systemctl restart manticore
[root@srv lib]# ps aux|grep searchd
mantico+  1955  0.0  0.0  61964  1580 ?        S    11:02   0:00 /usr/bin/searchd --config /etc/manticoresearch/manticore.conf --coredump
mantico+  1956  0.6  0.0 392744  2664 ?        Sl   11:02   0:00 /usr/bin/searchd --config /etc/manticoresearch/manticore.conf --coredump
```

* make sure that your OS allows you to save coredumps: `/proc/sys/kernel/core_pattern` should be non-empty - it is where it will save them. If you do:
```
echo "/cores/core.%e.%p.%h.%t" > /proc/sys/kernel/core_pattern
```
it will instruct your kernel to save them to file like `core.searchd.1773.centos-4gb-hel1-1.1636454937`
* searchd should be started with `ulimit -c unlimited`, but if you start Manticore via systemctl it does it for yourself since it does:
```
[root@srv lib]# grep CORE /lib/systemd/system/manticore.service
LimitCORE=infinity
```

### How do I install debug symbols?

Manticore Search and Manticore Columnar Library are written in C++, which means that what you get is a compiled compact binary file which executes in your OS optimal way. When you run a binary your system doesn't have full access to the names of variables, functions, methods, classes etc that are implemented. All that is provided separately in so called "debuginfo" packages or "symbol packages".
Debug symbols are useful for troubleshooting and other debugging purposes, since when you have symbols and your binary crashes there's a way to visualize the state it crashed at including function names. Manticore Search provides such backtrace in searchd log and also generates coredump if it was run with `--coredump`. Without symbols all you get is just internal offsets that is difficult/impossible to decode. So if you make a bug report about a crash in most cases Manticore team will need debug symbols to be able to help you.

To install Manticore Search / Manticore Columnar Library debug symbols just install package `*debuginfo*` (centos), `*dbgsym*` (ubuntu, debian), `*dbgsymbols*` (windows, macos) of exactly the same version you are running. For example if you've installed Manticore Search in Centos 8 from package https://repo.manticoresearch.com/repository/manticoresearch/release/centos/8/x86_64/manticore-4.0.2_210921.af497f245-1.el8.x86_64.rpm the corresponding package with symbols is https://repo.manticoresearch.com/repository/manticoresearch/release/centos/8/x86_64/manticore-debuginfo-4.0.2_210921.af497f245-1.el8.x86_64.rpm

Note they have the same commit id `af497f245` which corresponds to the commit this version was built from.

If you installed Manticore from a Manticore APT/YUM repo you can one of the following tools:
* `debuginfo-install` in centos 7
* `dnf debuginfo-install` centos 8
* `find-dbgsym-packages` in Debian and Ubuntu

to find a debug symbols package for you.

### How to check the debug symbols exist?

1. Find build id in `file /usr/bin/searchd` output:

```
[root@srv lib]# file /usr/bin/searchd
/usr/bin/searchd: ELF 64-bit LSB executable, x86-64, version 1 (GNU/Linux), dynamically linked (uses shared libs), for GNU/Linux 2.6.32, BuildID[sha1]=2c582e9f564ea1fbeb0c68406c271ba27034a6d3, stripped
```

In this case it's 2c582e9f564ea1fbeb0c68406c271ba27034a6d3.

2. Find symbols in `/usr/lib/debug/.build-id` like this:

```
[root@srv ~]# ls -la /usr/lib/debug/.build-id/2c/582e9f564ea1fbeb0c68406c271ba27034a6d3*
lrwxrwxrwx. 1 root root 23 Nov  9 10:42 /usr/lib/debug/.build-id/2c/582e9f564ea1fbeb0c68406c271ba27034a6d3 -> ../../../../bin/searchd
lrwxrwxrwx. 1 root root 27 Nov  9 10:42 /usr/lib/debug/.build-id/2c/582e9f564ea1fbeb0c68406c271ba27034a6d3.debug -> ../../usr/bin/searchd.debug
```

### Uploading your data

To fix your bug developers often need to reproduce it locally. To do it they need your configuration file, table files, binlog (if present), sometimes source data (like data from external storages or XML/CSV files) and queries.

Attach your data when you [create a ticket on Github](https://github.com/manticoresoftware/manticoresearch/issues/new). In case it's too big or the data is sensitive feel free to upload it to our write-only FTP server:

* ftp: `dev.manticoresearch.com`
* user: `manticorebugs`
* pass: `shithappens`
* mode: only active mode is supported
* directory: create directory `github-issue-N` so we understand what data is related with what issue on [Github](https://github.com/manticoresoftware/manticoresearch/issues).

It's convenient to mirror your directory with our FTP using tool [lftp](https://lftp.yar.ru/) which is available for Linux, Mac and Windows. For example, if you want to sync your current directory `ftp` to directory `github-ussue-123`, here's what you should do and what you will get:

```bash
➜  ~ lftp -e "set ftp:passive-mode off; mkdir github-issue-123; mirror -LR ftp/ github-issue-123/" -u manticorebugs,shithappens dev.manticoresearch.com
mkdir ok, `github-issue-123' created
Total: 2 directories, 1 file, 0 symlinks
New: 1 file, 0 symlinks
lftp manticorebugs@dev.manticoresearch.com:/> quit
```

### DEBUG

```sql
DEBUG [ subcommand ]
```

`DEBUG` statement is designed to call different internal or vip commands for dev/testing purposes. It is not intended for production automation, since the syntax of `subcommand` part may be freely changed in any build.

Call `DEBUG` without params to show list of useful commands (in general) and subcommands (of `DEBUG` statement) available at current context.

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

All `debug XXX` commands should be regarded as non-stable, and they're matter of freely modification at any moment, don't be surprised. This example output here also not necessary reflect actual available commands, try it on your system to see what is available on your instance. Also, no detailed documentation implied apart this short 'meaning' column.

Just as quick illustration, two commands available only to VIP clients described below - `shutdown` and `crash`. Both requires a token, which can be generated with `debug token`
subcommand, and put into [shutdown_token](Server_settings/Searchd.md#shutdown_token) param of searchd section of the
config file. If no such section exists, or if a hash of the provided password does not match with the token stored in
the config, the subcommands will do nothing.

```sql
mysql> debug token hello;
+-------------+------------------------------------------+
| command     | result                                   |
+-------------+------------------------------------------+
| debug token | aaf4c61ddcc5e8a2dabede0f3b482cd9aea9434d |
+-------------+------------------------------------------+
1 row in set (0,00 sec)
```

Subcommand `shutdown` will send a TERM signal to the server and so will make it shut down. Since it is quite dangerous (nobody wants accidentally stop a production service), it:
1. needs a VIP connection, and
2. needs the password

Subcommand `crash` is literally causes crash. It may be used when setting up different things, like 'how system manager keeps livenes of the service', or 'how feasible is coredump tracking'.

If some commands is found useful in generic, we'll move them from `debug` subcommands to more generic and stable
place (notice meaning of `debug tasks` and `debug sched` in the table as such examples).
