# Rotating query and server logs 

Manticore Search accepts signal USR1 for reopening server and query log files.

The official [DEB and RPM packages](https://manticoresearch.com/downloads/) install a Logrotate conf file for all files in default log folder.

A simple logrotate conf for log files looks like:

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

In addition, the `FLUSH LOGS` SQL command is available, which works same as system USR1 signal. Initiate reopen of searchd log and query log files, letting you implement log file rotation. Command is non-blocking (i.e., returns immediately).