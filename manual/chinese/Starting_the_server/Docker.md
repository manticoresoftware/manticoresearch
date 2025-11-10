# 在 Docker 中启动和使用 Manticore

该镜像基于 [当前版本的 Manticore 包](https://manticoresearch.com/install/)。

[comment]: # (the below should be in sync with https://github.com/manticoresoftware/docker/blob/master/README.md)

默认配置包含一个示例实时表，并监听默认端口：

  * `9306` 用于来自 MySQL 客户端的连接
  * `9308` 用于通过 HTTP 的连接
  * `9312` 用于通过二进制协议的连接（例如在运行集群时）

该镜像附带用于从 MySQL、PostgreSQL、XML 和 CSV 文件轻松索引数据的库。

## 如何运行 Manticore Search Docker 镜像

### 快速使用

以下是启动 Manticore 容器并通过 mysql 客户端登录的最简单方法：

```bash
docker run --name manticore --rm -d manticoresearch/manticore && echo "Waiting for Manticore docker to start. Consider mapping the data_dir to make it start faster next time" && until docker logs manticore 2>&1 | grep -q "accepting connections"; do sleep 1; echo -n .; done && echo && docker exec -it manticore mysql && docker stop manticore
```

注意，退出 MySQL 客户端后，Manticore 容器将被停止并移除，导致数据不会被保存。有关在生产环境中使用 Manticore 的信息，请参见下文。

该镜像附带一个示例表，可以这样加载：

```sql
mysql> source /sandbox.sql
```

此外，mysql 客户端的历史记录中有几个示例查询，您可以在上述表上运行，只需在客户端使用上下键查看并执行它们。

### 生产环境使用


#### 端口和挂载点

为了数据持久化，文件夹 `/var/lib/manticore/` 应挂载到本地存储或其他所需的存储引擎。

实例内的配置文件位于 `/etc/manticoresearch/manticore.conf`。对于自定义设置，应将此文件挂载到您自己的配置文件。

端口为 9306/9308/9312，分别用于 SQL/HTTP/二进制，根据您使用 Manticore 的方式暴露它们。例如：

```bash
docker run --name manticore -v $(pwd)/data:/var/lib/manticore -p 127.0.0.1:9306:9306 -p 127.0.0.1:9308:9308 -d manticoresearch/manticore
```

或者

```bash
docker run --name manticore -v $(pwd)/manticore.conf:/etc/manticoresearch/manticore.conf -v $(pwd)/data:/var/lib/manticore/ -p 127.0.0.1:9306:9306 -p 127.0.0.1:9308:9308 -d manticoresearch/manticore
```

如果您希望端口对外部主机可用，请确保移除 `127.0.0.1:`。

#### Manticore 列存库和 Manticore Buddy

Manticore Search Docker 镜像预装了 [Manticore Columnar Library](https://github.com/manticoresoftware/columnar) 和 [Manticore Buddy](https://github.com/manticoresoftware/manticoresearch-buddy)

#### Docker-compose

在许多情况下，您可能希望将 Manticore 与 Docker Compose YAML 文件中指定的其他镜像一起使用。以下是在 docker-compose.yml 文件中为 Manticore Search 推荐的最小配置：

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

HTTP 协议暴露在端口 9308。您可以将端口映射到本地并使用 curl 连接：

```bash
docker run --name manticore -p 9308:9308 -d manticoresearch/manticore
```

<!-- example create -->
创建表：

<!-- request JSON -->
```json
POST /cli -d 'CREATE TABLE testrt ( title text, content text, gid integer)'
```
<!-- end -->
<!-- example insert -->
插入文档：

<!-- request JSON -->
```json
POST /insert
-d'{"table":"testrt","id":1,"doc":{"title":"Hello","content":"world","gid":1}}'
```
<!-- end -->
<!-- example search -->
执行简单搜索：

<!-- request JSON -->
```json
POST /search -d '{"table":"testrt","query":{"match":{"*":"hello world"}}}'
```
<!-- end -->

### 日志记录

默认情况下，服务器设置将日志发送到 `/dev/stdout`，可以从主机查看：


```bash
docker logs manticore
```

通过传递变量 `QUERY_LOG_TO_STDOUT=true`，查询日志可以重定向到 Docker 日志。


#### 带复制的多节点集群

以下是定义两个节点集群的简单 `docker-compose.yml`：

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
* 在第二个实例加入集群
  ```sql
  $ docker-compose exec manticore-2 mysql

  mysql> JOIN CLUSTER posts AT 'manticore-1:9312';
  mysql> INSERT INTO posts:testrt(title,content,gid)  VALUES('hello','world',1);
  Query OK, 1 row affected (0.00 sec)

  MySQL [(none)]> exit
  Bye
  ```
* 如果现在回到第一个实例，您将看到新记录：
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

建议覆盖 docker 的默认 ulimits 以适应 Manticore 实例：

```bash
 --ulimit nofile=65536:65536
```

为了获得最佳性能，表组件可以被“mlock”锁定到内存中。当 Manticore 在 Docker 下运行时，实例需要额外权限以允许内存锁定。运行实例时必须添加以下选项：

```bash
  --cap-add=IPC_LOCK --ulimit memlock=-1:-1
```

### 使用 Docker 配置 Manticore Search

如果您想使用包含表定义的自定义配置运行 Manticore，则需要将配置挂载到实例：

```bash
docker run --name manticore -v $(pwd)/manticore.conf:/etc/manticoresearch/manticore.conf -v $(pwd)/data/:/var/lib/manticore -p 127.0.0.1:9306:9306 -d manticoresearch/manticore
```

请注意，容器内的 Manticore search 以用户 `manticore` 运行。对表文件的操作（如创建或轮换普通表）也应在 `manticore` 用户下进行。否则文件将由 `root` 创建，搜索守护进程将无权打开它们。例如，以下是如何轮换所有表：

```bash
docker exec -it manticore gosu manticore indexer --all --rotate
```

您还可以使用 Docker 环境变量设置单独的 `searchd` 和 `common` 配置项。

设置必须以其节名称为前缀，例如对于 `mysql_version_string`，变量名必须为 `searchd_mysql_version_string`：


```bash
docker run --name manticore  -p 127.0.0.1:9306:9306  -e searchd_mysql_version_string='5.5.0' -d manticoresearch/manticore
```

对于 `listen` 指令，使用 Docker 变量 `searchd_listen` 可以在默认监听接口之外添加新的监听接口。可以声明多个接口，用分号 ("|") 分隔。要仅监听网络地址，可以使用 `$ip`（内部通过 `hostname -i` 获取）作为地址别名。

例如，`-e searchd_listen='9316:http|9307:mysql|$ip:5443:mysql_vip'` 将在默认的 9306 和 9308 端口之外，额外添加一个 9307 端口的 SQL 接口，一个仅在实例 IP 上运行的 5443 端口的 SQL VIP 监听器，以及一个 9316 端口的 HTTP 监听器。

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

要使用自定义启动标志启动 Manticore，请在使用 docker run 时将它们作为参数指定。确保不要包含 `searchd` 命令，并包含 `--nodetach` 标志。示例如下：
```bash
docker run --name manticore --rm manticoresearch/manticore:latest --replay-flags=ignore-trx-errors --nodetach
```

#### 以非 root 用户运行
默认情况下，主 Manticore 进程 `searchd` 在容器内以用户 `manticore` 运行，但启动容器时运行的脚本是以您的默认 docker 用户身份运行的，在大多数情况下是 `root`。如果这不是您想要的，可以使用 `docker ... --user manticore` 或在 docker compose yaml 中使用 `user: manticore` 使所有内容都以 `manticore` 用户身份运行。下面会介绍您可能遇到的卷权限问题及其解决方法。

#### 启动时创建普通表
要构建自定义配置文件中指定的普通表，可以使用环境变量 `CREATE_PLAIN_TABLES=1`。它将在 Manticore 启动前执行 `indexer --all`。如果您不使用卷，并且表容易重新创建，这非常有用。
```bash
docker run -e CREATE_PLAIN_TABLES=1 --name manticore -v $(pwd)/manticore.conf:/etc/manticoresearch/manticore.conf -p 9306:9306 -p 9308:9308 -d manticoresearch/manticore
```

### 故障排除

#### 挂载卷的权限问题

如果您以非 root 用户运行 Manticore Search docker（使用 `docker ... --user manticore` 或在 docker compose yaml 中使用 `user: manticore`），可能会遇到权限问题，例如：
```bash
FATAL: directory /var/lib/manticore write error: failed to open /var/lib/manticore/tmp: Permission denied
```

这是因为用于在容器内运行进程的用户可能没有权限修改您挂载到容器的目录。为了解决此问题，您可以对挂载目录执行 `chown` 或 `chmod`。如果您以用户 `manticore` 运行容器，需要执行：
```bash
chown -R 999:999 data
```

因为容器内用户 `manticore` 的 ID 是 999。

<!-- proofread -->

