# Docker logging

When you use the [official Manticore docker image](https://hub.docker.com/r/manticoresearch/manticore/), the server log is sent to `/dev/stdout` which can be viewed from host with:

```bash
docker logs manticore
```
The query log can be diverted to the Docker log by passing the variable `QUERY_LOG_TO_STDOUT=true`.

The log folder is the same as in the case of the Linux package, set to `/var/log/manticore`. If desired, it can be mounted to a local path to view or process the logs.
<!-- proofread -->