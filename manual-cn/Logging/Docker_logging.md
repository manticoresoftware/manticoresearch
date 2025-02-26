# Docker 日志

当您使用[官方的 Manticore Docker 镜像](https://hub.docker.com/r/manticoresearch/manticore/)时，服务器日志会被发送到 `/dev/stdout`，可以通过以下命令在主机上查看日志：

```bash
docker logs manticore
```
查询日志可以通过设置环境变量 `QUERY_LOG_TO_STDOUT=true` 来转到 Docker 日志。

日志文件夹与 Linux 包的情况相同，默认路径为 `/var/log/manticore`。如果需要，可以将其挂载到本地路径以便查看或处理日志。

<!-- proofread -->