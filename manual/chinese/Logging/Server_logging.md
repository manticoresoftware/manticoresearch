# 服务器日志记录

默认情况下，Manticore搜索守护进程会在启动搜索守护进程的目录中将所有运行时事件记录在`searchd.log`文件中。在Linux中，默认情况下，您可以在`/var/log/manticore/searchd.log`找到日志。

日志文件的路径/名称可以通过在配置文件的`searchd`部分设置`log`来覆盖。

```ini
searchd {
...
    log = /custom/path/to/searchd.log
...
}
```

* 您也可以使用`syslog`作为文件名。在这种情况下，事件将发送到您的服务器的syslog守护进程。
* 在某些情况下，您可能希望将`/dev/stdout`用作文件名。在这种情况下，在Linux中，Manticore将简单地输出事件。这在Docker/Kubernetes环境中非常有用。


<!-- proofread -->

