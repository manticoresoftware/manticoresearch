# Docker 日志

当您使用 [官方 Manticore docker 镜像](https://hub.docker.com/r/manticoresearch/manticore/) 时，服务器日志会发送到 `/dev/stdout`，可以在主机上通过以下方式查看：

```bash
docker logs manticore
```
查询日志可以通过传递变量 `QUERY_LOG_TO_STDOUT=true` 来重定向到 Docker 日志。

日志文件夹与 Linux 包的情况相同，设置为 `/var/log/manticore`。如果需要，可以将其挂载到本地路径以查看或处理日志。
<!-- proofread -->

