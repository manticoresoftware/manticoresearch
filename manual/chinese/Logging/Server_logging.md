# 服务器日志

默认情况下，Manticore 搜索守护进程会将所有运行时事件记录在启动 searchd 的目录中的 `searchd.log` 文件中。在 Linux 中，默认情况下，您可以在 `/var/log/manticore/searchd.log` 找到日志。

可以通过在配置文件的 `searchd` 部分中设置 `log` 来覆盖日志文件的路径/名称。

```ini
searchd {
...
    log = /custom/path/to/searchd.log
...
}
```

* 您还可以将 `syslog` 用作文件名。在这种情况下，事件将被发送到您服务器的 syslog 守护进程。
* 在某些情况下，您可能想要使用 `/dev/stdout` 作为文件名。在这种情况下，Manticore 将简单地输出事件。这在 Docker/Kubernetes 环境中可能很有用。


<!-- proofread -->
