# 旋转查询和服务器日志

Manticore Search 接受 USR1 信号，用于重新打开服务器和查询日志文件。

官方 [DEB 和 RPM 包](https://manticoresearch.com/install/) 安装默认日志文件夹中所有文件的 Logrotate 配置文件。

简单的日志旋转配置如下：

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

此外，`FLUSH LOGS` SQL 命令可用，其工作方式与 USR1 系统信号相同。它会启动搜索日志和查询日志文件的重新打开，允许您实现日志文件旋转。该命令是非阻塞的（即，它会立即返回）。
<!-- proofread -->

