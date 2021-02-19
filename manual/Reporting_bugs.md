# Reporting bugs

Unfortunately, Manticore is not yet 100% bug free (even though we're working hard towards that), so you might occasionally run into some issues.

Reporting as much as possible about each bug is very important - because to fix it, we need to be able to either reproduce and fix the bug, or to deduce what's causing it from the information that you provide. So here are some instructions how to do that.

### Bug-tracker
We track bugs and feature requests in [Github](https://github.com/manticoresoftware/manticore/issues). Feel free to create a new ticket and describe your bug in details so both you and developers can save their time.

### Crashes
Manticore is written in C++ - low level programming language allowing to speak to the computer with not so many intermediate layers for faster performance. The drawback of that is that in rare cases there is no way to handle a bug elegantly writing the error about it to a log and skipping processing the command which caused the problem. Instead of that the program can just crash which means it would stop completely and would have to be restarted.

In case of crashes we sometimes can get enough info to fix from the backtrace which Manticore tries to write down in [the log file](Logging/Server_logging.md). It might look like this:

```bash
./indexer(_Z12sphBacktraceib+0x2d6)[0x5d337e]
./indexer(_Z7sigsegvi+0xbc)[0x4ce26a]
/lib64/libpthread.so.0[0x3f75a0dd40]
/lib64/libc.so.6(fwrite+0x34)[0x3f74e5f564]
./indexer(_ZN27CSphCharsetDefinitionParser5ParseEPKcR10CSphVectorI14CSphRemapRange16CSphVe
ctorPolicyIS3_EE+0x5b)[0x51701b]
./indexer(_ZN13ISphTokenizer14SetCaseFoldingEPKcR10CSphString+0x62)[0x517e4c]
./indexer(_ZN17CSphTokenizerBase14SetCaseFoldingEPKcR10CSphString+0xbd)[0x518283]
./indexer(_ZN18CSphTokenizer_SBCSILb0EEC1Ev+0x3f)[0x5b312b]
./indexer(_Z22sphCreateSBCSTokenizerv+0x20)[0x51835c]
./indexer(_ZN13ISphTokenizer6CreateERK21CSphTokenizerSettingsPK17CSphEmbeddedFilesR10CSphS
tring+0x47)[0x5183d7]
./indexer(_Z7DoIndexRK17CSphConfigSectionPKcRK17SmallStringHash_TIS_EbP8_IO_FILE+0x494)[0x
4d31c8]
./indexer(main+0x1a17)[0x4d6719]
/lib64/libc.so.6(__libc_start_main+0xf4)[0x3f74e1d8a4]
./indexer(__gxx_personality_v0+0x231)[0x4cd779]
```

This was an example of a good backtrace - we can see mangled function
names here.

But sometimes backtrace may look like this:
```bash
/opt/piler/bin/indexer[0x4c4919]
/opt/piler/bin/indexer[0x405cf0]
/lib/x86_64-linux-gnu/libpthread.so.0(+0xfcb0)[0x7fc659cb6cb0]
/opt/piler/bin/indexer[0x4237fd]
/opt/piler/bin/indexer[0x491de6]
/opt/piler/bin/indexer[0x451704]
/opt/piler/bin/indexer[0x40861a]
/opt/piler/bin/indexer[0x40442c]
/lib/x86_64-linux-gnu/libc.so.6(__libc_start_main+0xed)[0x7fc6588aa76d]
/opt/piler/bin/indexer[0x405b89]
```

Developers might not get anything useful from these cryptic numbers since it doesn't show function names. To help that you need to provide symbols (function and variable names). If you've installed Manticore by building from sources, run the following command over your binary:
```bash
nm -n indexer > indexer.sym
```

Attach this file to a bug report along with backtrace. You should however ensure that the binary is not stripped. Our official binary packages should be fine. However, if you built Manticore manually from sources, do not run `strip` utility on that binary, and/or do not let your build/packaging system do that, otherwise the symbols will be lost completely.

In some cases crashes can happen, because the index files have a corruption. **It's highly recommended to use [indextool --check](../Miscellaneous_tools.md#indextool) and add the output to the bug report you will create on Github.**

### Core dumps

> A core dump is a file containing a process's address space (memory) when the process terminates unexpectedly.

Sometimes the backtrace doesn't provide enough information about the cause of a crash or the crash cannot be easily reproduced and core files are required for troubleshooting.

For searchd (Manticore search server) to record a core dump in case of a crash, the following needs to be ensured:

* core dumping needs to be enabled on the running operating system. Some operating systems do not have core dumping enabled by default
* searchd needs to be started with `--coredump` option

Please note that searchd core files can use a lot of space as they include data from the loaded indexes and each crash creates a new core file. Free space should be monitored while searchd runs with `--coredump` option enabled to avoid 100% disk usage.

### Hanging

In case Manticore is hanging for some reason and
1) the instance is under watchdog (which is on by default)
2) gdb is installed

Then:

* either connect to the instance via mysql (vip or regular port) and issue `debug procdump`  
* or manually send USR2 signal to **watchdog** of the hanging instance (not to the instance process itself)
* or manually run `gdb attach <PID_of_hanged>` and then these commands one by one:
  1. `info threads`
  2. `thread apply all bt`
  3. `bt`
  4. `info locals`
  5. `detach`

In the first 2 cases trace will be in the server's log. In the last (manual gdb) case it has to be copied from console output. These traces need to be attached, it will be very helpful for investigation.

### Uploading your data

To fix your bug developers often need to reproduce it locally. To do it they need your configuration file, index files, binlog (if present), sometimes data to index (like data from external storages or XML/CSV files) and queries.

Attach your data when you [create a ticket on Github](https://github.com/manticoresoftware/manticoresearch/issues/new). In case it's too big or the data is sensitive feel free to upload it to our write-only FTP server:

* ftp: `dev.manticoresearch.com`
* user: `manticorebugs`
* pass: `shithappens`
* directory: create directory `github-issue-N` so we understand what data is related with what issue on Github.

### DEBUG

```sql
DEBUG [ subcommand ]
```

`DEBUG` statement is designed to call different internal or vip commands for dev/testing purposes. It is not intended for production automation, since the syntax of `subcommand` part may be freely changed in any build.

Call `DEBUG` without params to show list of useful commands (in general) and subcommands (of `DEBUG` statement) available at the moment.

However you can invoke `DEBUG` without params to know which subcommands of the statement are available in any particular case:

```sql
mysql> debug;
+----------------+---------------------+
| command        | meaning             |
+----------------+---------------------+
| flush logs     | emulate USR1 signal |
| reload indexes | emulate HUP signal  |
+----------------+---------------------+
2 rows in set (0,00 sec)
```

(these commands are already documented, but such short help just remind about them).

If you connect via 'VIP' connection (see [listen](Server_settings/Searchd.md#listen) for details) the output might be a bit different:

```sql
mysql> debug;
+---------------------------+------------------------------+
| command                   | meaning                      |
+---------------------------+------------------------------+
| debug shutdown <password> | emulate TERM signal          |
| debug token <password>    | calculate token for password |
| flush logs                | emulate USR1 signal          |
| reload indexes            | emulate HUP signal           |
+---------------------------+------------------------------+
4 rows in set (0,00 sec)
```

Here you can see additional commands available only in the current context (namely, if you connected on a VIP port). Two additional subcommands available right now are `token` and `shutdown`. The first one just calculates a hash (SHA1) of the <password> (which, in turn, may be empty, or a word, or num/phrase enclosed in '-quotes) like:

```sql
mysql> debug token hello;
+-------------+------------------------------------------+
| command     | result                                   |
+-------------+------------------------------------------+
| debug token | aaf4c61ddcc5e8a2dabede0f3b482cd9aea9434d |
+-------------+------------------------------------------+
1 row in set (0,00 sec)
```

Another debug subcommand `shutdown` will send a TERM signal to the server and so will make it shut down. Since it is quite dangerous (nobody wants accidentally stop a production service), it:
1. needs a VIP connection, and
2. needs the password

For the chosen password you need to generate a token with `debug token` subcommand, and put it into [shutdown_token](Server_settings/Searchd.md#shutdown_token) param of searchd section of the config file. If no such section exists, or if a hash of the provided password  does not match with the token stored in the config, the subcommand will do nothing. Otherwise it will cause 'clean' shutdown of the server.
