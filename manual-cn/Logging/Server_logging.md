# 服务器日志

默认情况下，Manticore 搜索守护进程会将所有运行时事件记录在启动 `searchd` 的目录中的 `searchd.log` 文件。在 Linux 系统中，默认路径为 `/var/log/manticore/searchd.log`。

可以通过在配置文件的 `searchd` 部分设置 `log` 来覆盖日志文件的路径/名称：

```ini
searchd {
...
    log = /custom/path/to/searchd.log
...
}
```

* 您还可以将文件名设置为 `syslog`，此时事件将发送到服务器的 syslog 守护进程。
* 在某些情况下，您可能希望将文件名设置为 `/dev/stdout`。在这种情况下，Manticore 会直接输出事件日志，这在 Docker/Kubernetes 环境中非常有用。


<!-- proofread -->