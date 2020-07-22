# Server logging

By default, searchd will try to log all runtime events in a `searchd.log` file in current working directory.

The path to a specific log file can be set with `log` directive in `searchd` section of the configuration file.

```ini
searchd {
...
    log = /var/log/searchd.log
...
}
```

Also you can use the 'syslog' as the file name. In this case the events will be sent to syslog daemon. To use the syslog option Manticore must be configured `--with-syslog` on building (official packages come with syslog support).
