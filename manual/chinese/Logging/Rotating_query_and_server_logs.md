# 旋转查询和服务器日志

Manticore Search 接受 USR1 信号以重新打开服务器和查询日志文件。

官方的 [DEB 和 RPM 软件包](https://manticoresearch.com/install/) 会为默认日志文件夹中的所有文件安装一个 Logrotate 配置文件。

一个简单的日志轮换配置文件示例如下：

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

此外，还提供了 `FLUSH LOGS` SQL 命令，其工作方式与 USR1 系统信号相同。它会触发重新打开 searchd 日志和查询日志文件，从而允许您实现日志文件轮换。该命令是非阻塞的（即立即返回）。
<!-- proofread -->

