# Docker 日志

当你使用 [官方 Manticore Docker 镜像](https://hub.docker.com/r/manticoresearch/manticore/) 时，服务器日志会发送到 `/dev/stdout`，可在宿主机上通过以下方式查看：

```bash
docker logs manticore
```
可以通过传入变量 `QUERY_LOG_TO_STDOUT=true` 将查询日志重定向到 Docker 日志。

日志目录与 Linux 软件包的情况相同，设置为 `/var/log/manticore`。如有需要，可以将其挂载到本地路径，以便查看或处理日志。
<!-- proofread -->

