# 轮转查询日志与服务器日志

Manticore Search 接受 USR1 信号以重新打开服务器和查询日志文件。

官方 [DEB 和 RPM 安装包](https://manticoresearch.com/install/) 为默认日志文件夹中的所有文件安装了 Logrotate 配置文件。

一个简单的日志轮转配置示例如下：

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

此外，还提供了 `FLUSH LOGS` SQL 命令，其作用方式与 USR1 系统信号相同。它会启动 searchd 日志和查询日志文件的重新打开操作，从而允许您实现日志文件的轮转。该命令是非阻塞的（即，它会立即返回）。
<!-- proofread -->

