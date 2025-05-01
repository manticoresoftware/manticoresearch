# Rotating query and server logs 

Manticore Search accepts the USR1 signal for reopening server and query log files.

The official [DEB and RPM packages](https://manticoresearch.com/install/) install a Logrotate configuration file for all files in the default log folder.

A simple logrotate configuration for log files looks like:

```ini
/var/log/manticore/*.log {
       weekly
       rotate 10
       copytruncate
       delaycompress
       compress
       notifempty
       missingok
}
```

## FLUSH LOGS

```sql
mysql> FLUSH LOGS;
Query OK, 0 rows affected (0.01 sec)
```

Additionally, the `FLUSH LOGS` SQL command is available, which works the same way as the USR1 system signal. It initiates the reopening of searchd log and query log files, allowing you to implement log file rotation. The command is non-blocking (i.e., it returns immediately).
<!-- proofread -->