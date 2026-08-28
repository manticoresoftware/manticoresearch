# Internal searchd options

## internal command line options

* `--watchdog` (no windows): Enables the internal watchdog, overriding any settings in the configuration file. Note that `--watchdog` is incompatible with foreground mode; if `--console` or `--nodetach` is used, the watchdog is implicitly disabled, and a warning is issued.

* `--no-watchdog` (no windows): Disables the internal watchdog. This overrides any settings in the configuration file and is equivalent to setting `watchdog=0` in the config.

* `--systemd` (Linux only): Forces `searchd` to operate as if it were managed by systemd. By default, `searchd` checks whether it can notify systemd of its state; if communication is possible, it assumes systemd management. In this mode, the watchdog is disabled (unless explicitly enabled) and the `pid_file` configuration is not strictly required. This option explicitly forces systemd management mode, may be used to debug behaviour under systemd management.

* `--no-systemd` (Linux only): Disables systemd management mode and prevents `searchd` from assuming any external management. Consequently, the process runs with the watchdog enabled (unless explicitly disabled) and writes its PID to the file specified by the `pid_file` option in the configuration. Can be useful, if run searchd in systemd (deprecated) fork mode. In the case searchd will recognize

* `--test` switch to test mode. Daemon makes extra checks where necessary. Also, it sets some internal limits to extremely unfriendly values - to catch possible bugs more quickly than in usual codeflow. rt-ratio is forcibly set to 0.2, which means, daemon will flush ram-chunk when it occupies just 20% of rt-mem-limit. And it might be up to 5 chunks savings simultaneously (in usual conditions it is 0.33, and it might be just 3 junks a time - one finishing, one in progress, and one just starting). Parallel processing forcibly invert order of jobs - in order to force the daemon to use most detailed and comprehensive merge procedures, which usually can be avoided). 
 
* `--force-pseudo-sharding` - always use pseudo-sharing, despite the size of the index. Used for testing purposes

* `--with-buddy` - always start buddy, despite test mode (activated by `--test`, or `--console`, or whatever). Used for testing purposes.

## Undocumented env variables

* `MANTICORE_LOG_RTSPLIT_QUERY` - verbose logging split query events, ruled by this env variable
* `MANTICORE_LOG_RTSAVEDIAG` - ops for save RAM segments as disk chunk
* `MANTICORE_LOG_SPLIT_QUERY` - verbose logging split query events, ruled by this env variable
* `MANTICORE_LOG_RPL_TNX` - verbose logging of replicating transactions, ruled by this env variable
* `MANTICORE_GUARDED_STACK` - use stack with guard page (will crash on overflow)
* `MANTICORE_LOG_SST_PROGRESS` - sst (replication) progress logging
* `MANTICORE_LOG_SST_PROGRESS_INTEVAL` - interval for progress logging, milliseconds
* `MANTICORE_LOG_HTTP_BAD_REQ` - log content of bad http requests, ruled by this env variable
* `MANTICORE_LOG_HTTP_DATA` - verbose logging of http data, ruled by this env variable
* `MANTICORE_LOG_HTTP` - verbose logging http processing events, ruled by this env variable
* `MANTICORE_TELEMETRY` - manages telemetry (default true, explicitly can te set to false)
* `MANTICORE_NO_CHANGE_CWD` - don't switch to working dir
* `MANTICORE_TRACE_LOAD` - if true, periodically log current load state
* `MANTICORE_BINLOG_COMMON` - if present, and nothing set in config, binlog will be common for all tables 
* `MANTICORE_LOG_ES_COMPAT` - verbose logging compat events, ruled by this env variable
* `MANTICORE_TRACK_RT_ERRORS` - check newborn disk chunks, and if it is damaged, report up to this N of errors.
* `MANTICORE_BUDDY_TIMEOUT` - (default 3 seconds) daemon's wait duration for a buddy message at startup