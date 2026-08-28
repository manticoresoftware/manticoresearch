# 使用 Docker 启动和使用 Manticore

该镜像是基于 [Manticore 包的当前版本](https://manticoresearch.com/install/)。

[注释]: # (以下内容应与 https://github.com/manticoresoftware/docker/blob/master/README.md 中的内容保持一致)

默认配置包括一个示例实时表，并监听默认端口：

  * `9306` 用于 MySQL 客户端连接
  * `9308` 用于 HTTP 连接
  * `9312` 用于二进制协议连接（例如，如果您运行集群）

该镜像包含用于从 MySQL、PostgreSQL、XML 和 CSV 文件中轻松索引数据的库。

## 如何运行 Manticore Search Docker 镜像

### 快速使用

以下是最简单的方法来启动 Manticore 容器并在其中使用 mysql 客户端登录：

```bash
docker run --name manticore --rm -d manticoresearch/manticore && echo "Waiting for Manticore docker to start. Consider mapping the data_dir to make it start faster next time" && until docker logs manticore 2>&1 | grep -q "accepting connections"; do sleep 1; echo -n .; done && echo && docker exec -it manticore mysql && docker stop manticore
```

请注意，退出 mysql 客户端后，Manticore 容器将停止并删除，导致没有保存的数据。有关在生产环境中使用 Manticore 的信息，请参阅以下内容。

该镜像包含一个可以像这样加载的示例表：

```sql
mysql> source /sandbox.sql
```

此外，mysql 客户端中有一些示例查询，您可以在上面的表上运行这些查询，只需在客户端中使用上/下箭头键查看和运行它们即可。

### 生产使用


#### 端口和挂载点

为了数据持久化，`/var/lib/manticore/` 文件夹应挂载到本地存储或其他所需的存储引擎。

实例中的配置文件位于 `/etc/manticoresearch/manticore.conf`。对于自定义设置，应将该文件挂载到您自己的配置文件。

端口为 9306/9308/9312 用于 SQL/HTTP/二进制，根据您如何使用 Manticore 来暴露它们。例如：

```bash
docker run --name manticore -v $(pwd)/data:/var/lib/manticore -p 127.0.0.1:9306:9306 -p 127.0.0.1:9308:9308 -d manticoresearch/manticore
```

或

```bash
docker run --name manticore -v $(pwd)/manticore.conf:/etc/manticoresearch/manticore.conf -v $(pwd)/data:/var/lib/manticore/ -p 127.0.0.1:9306:9306 -p 127.0.0.1:9308:9308 -d manticoresearch/manticore
```

请确保删除 `127.0.0.1:`，以便端口对外部主机可用。

#### Manticore 列表库和 Manticore Buddy

Manticore Search Docker 镜像预装了 [Manticore 列表库](https://github.com/manticoresoftware/columnar) 和 [Manticore Buddy](https://github.com/manticoresoftware/manticoresearch-buddy)。

#### Docker-compose

在许多情况下，您可能希望使用 Manticore 与其他指定在 Docker Compose YAML 文件中的镜像一起使用。以下是在 docker-compose.yml 文件中推荐的 Manticore Search 的最小配置：

```yaml
version: '2.2'

services:
  manticore:
    container_name: manticore
    image: manticoresearch/manticore
    restart: always
    ports:
      - 127.0.0.1:9306:9306
      - 127.0.0.1:9308:9308
    ulimits:
      nproc: 65535
      nofile:
         soft: 65535
         hard: 65535
      memlock:
        soft: -1
        hard: -1
    volumes:
      - ./data:/var/lib/manticore
#      - ./manticore.conf:/etc/manticoresearch/manticore.conf # uncomment if you use a custom config
```

除了使用暴露的端口 9306 和 9308，您还可以通过运行 `docker-compose exec manticore mysql` 登录实例。

#### HTTP 协议

HTTP 协议在端口 9308 上暴露。您可以将端口映射到本地并使用 curl 连接：

```bash
docker run --name manticore -p 9308:9308 -d manticoresearch/manticore
```

<!-- example create -->
创建一个表：

<!-- request JSON -->
```json
POST /cli -d 'CREATE TABLE testrt ( title text, content text, gid integer)'
```
<!-- end -->
<!-- example insert -->
插入一个文档：

<!-- request JSON -->
```json
POST /insert
-d'{"table":"testrt","id":1,"doc":{"title":"Hello","content":"world","gid":1}}'
```
<!-- end -->
<!-- example search -->
执行一个简单的搜索：

<!-- request JSON -->
```json
POST /search -d '{"table":"testrt","query":{"match":{"*":"hello world"}}}'
```
<!-- end -->

### 日志

默认情况下，服务器将日志发送到 `/dev/stdout`，可以从主机查看：


```bash
docker logs manticore
```

可以通过传递变量 `QUERY_LOG_TO_STDOUT=true` 将查询日志重定向到 Docker 日志。


#### 多节点集群与复制

以下是一个简单的 `docker-compose.yml` 文件，用于定义一个两节点集群：

```yaml
version: '2.2'

services:

  manticore-1:
    image: manticoresearch/manticore
    restart: always
    ulimits:
      nproc: 65535
      nofile:
         soft: 65535
         hard: 65535
      memlock:
        soft: -1
        hard: -1
    networks:
      - manticore
  manticore-2:
    image: manticoresearch/manticore
    restart: always
    ulimits:
      nproc: 65535
      nofile:
        soft: 65535
        hard: 65535
      memlock:
        soft: -1
        hard: -1
    networks:
      - manticore
networks:
  manticore:
    driver: bridge
```
* 启动它：`docker-compose up`
* 创建集群：
  ```sql
  $ docker-compose exec manticore-1 mysql

  mysql> CREATE TABLE testrt ( title text, content text, gid integer);

  mysql> CREATE CLUSTER posts;
  Query OK, 0 rows affected (0.24 sec)

  mysql> ALTER CLUSTER posts ADD testrt;
  Query OK, 0 rows affected (0.07 sec)

  MySQL [(none)]> exit
  Bye
  ```
* 在第二个实例上加入集群
  ```sql
  $ docker-compose exec manticore-2 mysql

  mysql> JOIN CLUSTER posts AT 'manticore-1:9312';
  mysql> INSERT INTO posts:testrt(title,content,gid)  VALUES('hello','world',1);
  Query OK, 1 row affected (0.00 sec)

  MySQL [(none)]> exit
  Bye
  ```
* 如果您现在回到第一个实例，您将看到新的记录：
  ```sql
  $ docker-compose exec manticore-1 mysql

  MySQL [(none)]> select * from testrt;
  +---------------------+------+-------+---------+
  | id                  | gid  | title | content |
  +---------------------+------+-------+---------+
  | 3891565839006040065 |    1 | hello | world   |
  +---------------------+------+-------+---------+
  1 row in set (0.00 sec)

  MySQL [(none)]> exit
  Bye
  ```

### 内存锁定和限制

建议为 Manticore 实例覆盖 Docker 的默认 ulimits：

```bash
 --ulimit nofile=65536:65536
```

为了获得最佳性能，可以将表组件“锁定”到内存中。当 Manticore 在 Docker 下运行时，实例需要额外的权限以允许内存锁定。运行实例时必须添加以下选项：

```bash
  --cap-add=IPC_LOCK --ulimit memlock=-1:-1
```

### 使用 Docker 配置 Manticore Search

如果您想使用自定义配置运行 Manticore，该配置包括表定义，您需要将配置挂载到实例中：

```bash
docker run --name manticore -v $(pwd)/manticore.conf:/etc/manticoresearch/manticore.conf -v $(pwd)/data/:/var/lib/manticore -p 127.0.0.1:9306:9306 -d manticoresearch/manticore
```

请注意，Manticore 搜索在容器内以用户 `manticore` 运行。对表文件（如创建或旋转纯表）的操作也应在 `manticore` 用户下进行。否则，文件将由 `root` 创建，搜索守护进程将没有权限打开它们。例如，这里是如何旋转所有表的：

```bash
docker exec -it manticore gosu manticore indexer --all --rotate
```

您还可以使用 Docker 环境变量设置单独的 `searchd` 和 `common` 配置设置。

设置必须带有它们的节名前缀，例如对于 `mysql_version_string`，变量必须命名为 `searchd_mysql_version_string`：


```bash
docker run --name manticore  -p 127.0.0.1:9306:9306  -e searchd_mysql_version_string='5.5.0' -d manticoresearch/manticore
```

对于 `listen` 指令，可以使用 Docker 变量 `searchd_listen` 附加新的监听接口，除了默认的接口。可以声明多个接口，用分号（`;`）分隔。要仅监听网络地址，可以使用 `$ip`（从 `hostname -i` 内部检索）作为地址别名。

例如 `-e searchd_listen='9316:http|9307:mysql|$ip:5443:mysql_vip'` 将添加一个附加的 SQL 接口在端口 9307，一个 SQL VIP 监听器在端口 5443 仅运行在实例的 IP 上，以及一个 HTTP 接口在端口 9316，此外还有默认的 9306 和 9308。

```bash
$ docker run --rm -p 1188:9307  -e searchd_mysql_version_string='5.5.0' -e searchd_listen='9316:http|9307:mysql|$ip:5443:mysql_vip'  manticore
[Mon Aug 17 07:31:58.719 2020] [1] using config file '/etc/manticoresearch/manticore.conf' (9130 chars)...
listening on all interfaces for http, port=9316
listening on all interfaces for mysql, port=9307
listening on 172.17.0.17:5443 for VIP mysql
listening on all interfaces for mysql, port=9306
listening on UNIX socket /var/run/mysqld/mysqld.sock
listening on 172.17.0.17:9312 for sphinx
listening on all interfaces for http, port=9308
prereading 0 indexes
prereaded 0 indexes in 0.000 sec
accepting connections
```

#### 启动标志

要以自定义启动标志启动 Manticore，请在使用 `docker run` 时指定它们。确保不要包含 `searchd` 命令，并包含 `--nodetach` 标志。以下是一个示例：
```bash
docker run --name manticore --rm manticoresearch/manticore:latest --replay-flags=ignore-trx-errors --nodetach
```

#### 以非 root 用户运行
默认情况下，主要的 Manticore 进程 `searchd` 在容器内以用户 `manticore` 运行，但启动容器的脚本以默认的 Docker 用户运行，大多数情况下是 `root`。如果您不希望这样，可以使用 `docker ... --user manticore` 或在 docker compose yaml 中使用 `user: manticore` 使一切都在 `manticore` 用户下运行。请参阅以下内容以了解可能遇到的卷权限问题及其解决方案。

#### 在启动时创建普通表格
要构建自定义配置文件中指定的普通表格，可以使用 `CREATE_PLAIN_TABLES=1` 环境变量。它将在 Manticore 启动前执行 `indexer --all`。如果您不使用数据卷，并且表格容易重建，这非常有用。
```bash
docker run -e CREATE_PLAIN_TABLES=1 --name manticore -v $(pwd)/manticore.conf:/etc/manticoresearch/manticore.conf -p 9306:9306 -p 9308:9308 -d manticoresearch/manticore
```

### 故障排除

#### 挂载卷的权限问题

如果您以非 root 用户运行 Manticore Search 容器（使用 `docker ... --user manticore` 或在 docker compose yaml 中使用 `user: manticore`），可能会遇到权限问题，例如：
```bash
FATAL: directory /var/lib/manticore write error: failed to open /var/lib/manticore/tmp: Permission denied
```

这可能是因为用于在容器内部运行进程的用户没有权限修改您挂载到容器的目录。要解决此问题，您可以 `chown` 或 `chmod` 挂载目录。如果容器是以 `manticore` 用户运行，您需要执行：
```bash
chown -R 999:999 data
```

因为容器内部 `manticore` 用户的 ID 是 999。

<!-- proofread -->

