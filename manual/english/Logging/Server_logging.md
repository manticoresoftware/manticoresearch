# Server logging

By default, Manticore search daemon logs all runtime events in a `searchd.log` file in the directory where searchd was started. In Linux by default, you can find the log at `/var/log/manticore/searchd.log`.

The log file path/name can be overridden by setting `log` in the `searchd` section of the configuration file.

```ini
searchd {
...
    log = /custom/path/to/searchd.log
...
}
```

* You can also use `syslog` as the file name. In this case, events will be sent to your server's syslog daemon.
* In some cases, you might want to use `/dev/stdout` as the file name. In this case, on Linux, Manticore will simply output the events. This can be useful in Docker/Kubernetes environments.


<!-- proofread -->