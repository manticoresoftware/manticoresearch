# 轮换查询和服务器日志

Manticore Search 支持通过发送 USR1 信号重新打开服务器和查询日志文件。

官方的 [DEB 和 RPM 包](https://manticoresearch.com/install/) 会为默认日志文件夹中的所有日志文件安装一个 Logrotate 配置文件。

一个简单的 Logrotate 配置文件示例如下：

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

此外，还可以使用 `FLUSH LOGS` SQL 命令，其作用与 USR1 系统信号相同。该命令会重新打开 `searchd` 日志和查询日志文件，允许您实现日志文件轮换。此命令是非阻塞的（即，它会立即返回）。
<!-- proofread -->