# Server logging

By default, Manticore search daemon will log all runtime events in a `searchd.log` file in the directory the searchd was started from. Normally in Linux you can find the log in `/var/log/manticore/searchd.log`.

The log file path/name can be overriden via setting `log` in section `searchd` of the configuration file.

```ini
searchd {
...
    log = /custom/path/to/searchd.log
...
}
```

* Also you can use `syslog` as the file name. In this case the events will be sent to your server's syslog daemon.
* In some cases you might want to use `/dev/stdout` as the file name. In this case in Linux Manticore will just output the events. It can be useful in Docker/Kubernetes environments.

