# 旋转查询和服务器日志

Manticore Search 接受 USR1 信号以重新打开服务器和查询日志文件。

官方的 [DEB 和 RPM 包](https://manticoresearch.com/install/) 为默认日志文件夹中的所有文件安装了 Logrotate 配置文件。

日志文件的简单 logrotate 配置如下所示：

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

## 刷新日志

```sql
mysql> FLUSH LOGS;
Query OK, 0 rows affected (0.01 sec)
```

此外，`FLUSH LOGS` SQL 命令也可用，其作用与 USR1 系统信号相同。它启动搜索日志和查询日志文件的重新打开，允许您实现日志文件旋转。该命令是非阻塞的（即，它会立即返回）。
<!-- proofread -->
