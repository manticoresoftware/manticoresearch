# 服务器日志

默认情况下，Manticore 搜索守护进程会将所有运行时事件记录在启动 searchd 的目录中的 `searchd.log` 文件中。在 Linux 中，默认情况下，您可以在 `/var/log/manticore/searchd.log` 找到该日志。

日志文件路径/名称可以通过在配置文件的 `searchd` 部分设置 `log` 来覆盖。

```ini
searchd {
...
    log = /custom/path/to/searchd.log
...
}
```

* 您也可以使用 `syslog` 作为文件名。在这种情况下，事件将发送到服务器的 syslog 守护进程。
* 在某些情况下，您可能希望使用 `/dev/stdout` 作为文件名。在这种情况下，在 Linux 上，Manticore 将简单地输出事件。这在 Docker/Kubernetes 环境中非常有用。


<!-- proofread -->

