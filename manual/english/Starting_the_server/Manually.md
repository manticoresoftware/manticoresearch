# Starting Manticore manually

You can also start Manticore Search by calling `searchd` (Manticore Search server binary) directly:

```shell
searchd [OPTIONS]
```

Note that without specifying a path to the configuration file, `searchd` will try to find it in several locations depending on the operating system.


## searchd command line options


The options available to `searchd` in all operating systems are:

* `--help` (`-h` for short) lists all of the parameters that can be used in your particular build of `searchd`.
* `--version` (`-v` for short) shows Manticore Search version information.
* `--config <file>` (`-c <file>` for short) tells `searchd` to use the specified file as its configuration.
* `--stop` is used to asynchronously stop `searchd`, using the details of the PID file as specified in the Manticore configuration file. Therefore, you may also need to confirm to `searchd` which configuration file to use with the `--config` option. Example:

  ```bash
  $ searchd --config /etc/manticoresearch/manticore.conf --stop
  ```

* `--stopwait` is used to synchronously stop `searchd`. `--stop` essentially tells the running instance to exit (by sending it a SIGTERM) and then immediately returns. `--stopwait` will also attempt to wait until the running `searchd` instance actually finishes the shutdown (eg. saves all the pending attribute changes) and exits. Example:

  ```bash
  $ searchd --config /etc/manticoresearch/manticore.conf --stopwait
  ```
  Possible exit codes are as follows:
    * 0 on success
    * 1 if connection to running searchd server failed
    * 2 if server reported an error during shutdown
    * 3 if server crashed during shutdown

* `--status` command is used to query running `searchd` instance status using the connection details from the (optionally) provided configuration file. It will try to connect to running instance using the first found UNIX socket or TCP port from the configuration file. On success it will query for a number of status and performance counter values and print them. You can also use [SHOW STATUS](../Node_info_and_management/Node_status.md#SHOW-STATUS) command to access the very same counters via SQL protocol. Examples:

  ```bash
  $ searchd --status
  $ searchd --config /etc/manticoresearch/manticore.conf --status
  ```

* `--pidfile` is used to explicitly force using a PID file (where the `searchd` process identification number is stored) despite any other debugging options that say otherwise (for instance, `--console`). This is a debugging option.

  ```bash
  $ searchd --console --pidfile
  ```

* `--console` is used to force `searchd` into console mode. Typically, Manticore runs as a conventional server application and logs information into log files (as specified in the configuration file). However, when debugging issues in the configuration or the server itself, or trying to diagnose hard-to-track-down problems, it may be easier to force it to dump information directly to the console/command line from which it is being called. Running in console mode also means that the process will not be forked (so searches are done in sequence) and logs will not be written to. (It should be noted that console mode is not the intended method for running `searchd`.)  You can invoke it as:

  ```bash
  $ searchd --config /etc/manticoresearch/manticore.conf --console
  ```

* `--logdebug`, `--logreplication`, `--logdebugv`, and `--logdebugvv` options enable additional debug output in the server log. They differ by the logging verboseness level. These are debugging options and should not be normally enabled, as they can pollute the log a lot. They can be used temporarily on request to assist with complicated debugging sessions.

* `--iostats` is used in conjunction with the logging options (the `query_log` must have been activated in  `manticore.conf`) to provide more detailed information on a per-query basis about the input/output operations carried out in the course of that query, with a slight performance hit and slightly bigger logs. The IO statistics don't include information about IO operations for attributes, as these are loaded with mmap. To enable it, you can start `searchd` as follows:

  ```bash
  $ searchd --config /etc/manticoresearch/manticore.conf --iostats
  ```

* `--cpustats` is used to provide actual CPU time report (in addition to wall time) in both query log file (for every given query) and status report (aggregated). It depends on `clock_gettime()` Linux system call or falls back to less precise call on certain systems. You might start `searchd` thus:

  ```bash
  $ searchd --config /etc/manticoresearch/manticore.conf --cpustats
  ```

*  `--port portnumber` (`-p` for short) is used to specify the port that Manticore should listen on to accept binary protocol requests, usually for debugging purposes. This will usually default to 9312, but sometimes you need to run it on a different port. Specifying it on the command line will override anything specified in the configuration file. The valid range is 0 to 65535, but ports numbered 1024 and below usually require a privileged account in order to run.

  An example of usage:

  ```bash
  $ searchd --port 9313
  ```

* `--listen ( address ":" port | port | path ) [ ":" protocol ]` (or `-l` for short) Works as `--port`, but allows you to specify not only the port, but the full path, IP address and port, or Unix-domain socket path that `searchd` will listen on. In other words, you can specify either an IP address (or hostname) and port number, just a port number, or a Unix socket path. If you specify a port number but not the address, searchd will listen on all network interfaces. A Unix path is identified by a leading slash. As the last parameter, you can also specify a protocol handler (listener) to be used for connections on this socket. Supported protocol values are 'sphinx' and 'mysql' (MySQL protocol used since 4.1).

* `--force-preread` forbids the server from serving any incoming connection until prereading of table files completes. By default, at startup, the server accepts connections while table files are lazy-loaded into memory. This extends the behavior and makes it wait until the files are loaded.

* `--index (--table) <table>` (or `-i (-t) <table>` for short) forces this instance of `searchd`  to only serve the specified table. Like `--port`, above, this is usually for debugging purposes; more long-term changes would generally be applied to the configuration file itself.

* `--strip-path` strips the path names from all the file names referenced from the table (stopwords, wordforms, exceptions, etc). This is useful for picking up tables built on another machine with possibly different path layouts.

* `--replay-flags=<OPTIONS>` switch can be used to specify a list of extra binary log replay options. The supported options are:
    * `accept-desc-timestamp`, ignore descending transaction timestamps and replay such transactions anyway (the default behavior is to exit with an error).
    * `ignore-open-errors`, ignore missing binlog files (the default behavior is to exit with an error).
    * `ignore-trx-errors`, ignore any transaction errors and skip current binlog file (the default behavior is to exit with an error).
    * `ignore-all-errors`, ignore any errors described above (the default behavior is to exit with an error).

    Example:

    ```bash
    $ searchd --replay-flags=accept-desc-timestamp
    ```

* `--coredump` is used to enable saving a core file or a minidump of the server on crash. Disabled by default to speed up of server restart on crash. This is useful for debugging purposes.

  ```bash
  $ searchd --config /etc/manticoresearch/manticore.conf --coredump
  ```

* `--new-cluster` bootstraps a replication cluster and makes the server a reference node with [cluster restart](../Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md) protection. On Linux you can also run `manticore_new_cluster`. It will start Manticore in `--new-cluster` mode via systemd.

* `--new-cluster-force` bootstraps a replication cluster and makes the server a reference node bypassing [cluster restart](../Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md) protection. On Linux you can also run `manticore_new_cluster --force`. It will start Manticore in `--new-cluster-force` mode via systemd.

* `--mockstack` analyzes and reports the necessary stack sizes for recursive expression evaluation, pattern matching operations, and filter processing. This debugging option outputs calculated stack requirements to the console for optimization purposes. The output provides environment variables that can be used to configure stack requirements for different operations.

  Example:
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

### Windows options

There are some options for `searchd` that are specific to Windows platforms, concerning handling as a service, and are only available in Windows binaries.

Note that in Windows searchd will default to `--console` mode, unless you install it as a service.

* `--install` installs `searchd` as a service into the Microsoft Management Console (Control Panel / Administrative Tools / Services). Any other parameters specified on the command line, where `--install` is specified will also become part of the command line on future starts of the service. For example, as a part of calling `searchd`, you will likely also need to specify the configuration file with `--config`, and you would do that as well as specifying `--install`. Once called, the usual start/stop facilities will become available via the management console, so any methods you could use for starting, stopping and restarting services would also apply to `searchd`. Example:

  ```bat
  C:\WINDOWS\system32> C:\Manticore\bin\searchd.exe --install
     --config C:\Manticore\manticore.conf
  ```

  If you want to have the I/O stats every time you start `searchd`, you need to specify the option on the same line as the `--install` command thus:

  ```bat
  C:\WINDOWS\system32> C:\Manticore\bin\searchd.exe --install
     --config C:\Manticore\manticore.conf --iostats
  ```
* `--delete` removes the service from the Microsoft Management Console and other places where services are registered, after previously being installed with `--install`. Note that this does not uninstall the software or delete the tables. It means the service will not be called from the services system, and will not be started on the machine's next start. If currently running as a service, the current instance will not be terminated (until the next reboot or until `--stop`). If the service was installed with a custom name (with `--servicename`), the same name will need to be specified with `--servicename` when calling to uninstall. Example:

  ```bat
  C:\WINDOWS\system32> C:\Manticore\bin\searchd.exe --delete
  ```

* `--servicename <name>` applies the given name to `searchd` when installing or deleting the service, as it would appear in the Management Console; this will default to searchd, but if being deployed on servers where multiple administrators may log in to the system, or a system with multiple `searchd` instances, a more descriptive name may be applicable. Note that unless combined with `--install` or `--delete`, this option does not do anything.  Example:
  ```bat
  C:\WINDOWS\system32> C:\Manticore\bin\searchd.exe --install
     --config C:\Manticore\manticore.conf --servicename ManticoreSearch
  ```

* `--ntservice` is an option that is passed by the Microsoft Management Console to `searchd` to invoke it as a service on Windows platforms. It would not normally be necessary to call this directly; this would normally be called by Windows when the service is started, although if you wanted to call this as a regular service from the command-line (as the complement to `--console`) you could do so in theory.

* `--safetrace` forces `searchd` to only use the system's backtrace() call in crash reports. In certain (rare) scenarios, this might be a "safer" way to get that report. This is a debugging option.

* `--nodetach` switch (Linux only) tells `searchd` not to detach into the background. This will also cause log entries to be printed out to the console. Query processing operates as usual. This is a debugging option and might also be useful when you run Manticore in a Docker container to capture its output.

## Plugin dir

Manticore utilizes the [plugin_dir](../Server_settings/Common.md#plugin_dir) for storing and using Manticore Buddy plugins. By default, this value is accessible to the "manticore" user in a standard installation. However, if you start the searchd daemon manually with a different user, the daemon might not have access to the `plugin_dir`. To address this problem, ensure you specify a `plugin_dir` in the common section that the user running the searchd daemon can write to.

## Signals

`searchd` supports a number of signals:

* `SIGTERM` - Initiates a clean shutdown. New queries will not be handled, but queries that are already started will not be forcibly interrupted.
* `SIGHUP` - Initiates tables rotation. Depending on the value of [seamless_rotate](../Server_settings/Searchd.md#seamless_rotate) setting, new queries might be shortly stalled; clients will receive temporary errors.
* `SIGUSR1` - Forces reopen of searchd log and query log files, allowing for log file rotation.

## Environment variables

* `MANTICORE_TRACK_DAEMON_SHUTDOWN=1` enables detailed logging while searchd is shutting down. It's useful in case of some shutdown problems, such as when Manticore takes too long to shut down or freezes during the shutdown process.
<!-- proofread -->
