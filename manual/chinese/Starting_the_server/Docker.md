# 启动和使用 Manticore 在 Docker 中

该镜像基于 [当前版本的 Manticore 包](https://manticoresearch.com/install/).

[comment]: # (the below should be in sync with https://github.com/manticoresoftware/docker/blob/master/README.md)

默认配置包括一个示例实时表并监听默认端口：

  * `9306` 用于来自 MySQL 客户端的连接
  * `9308` 用于通过 HTTP 的连接
  * `9312` 用于通过二进制协议的连接（例如在您运行集群的情况下）

该镜像附带用于从 MySQL、PostgreSQL、XML 和 CSV 文件轻松索引数据的库。

# 如何运行 Manticore Search Docker 镜像

## 快速使用

以下是以容器中启动 Manticore 并通过 mysql 客户端登录的最简单方法：

```bash
docker run --name manticore --rm -d manticoresearch/manticore && echo "等待 Manticore docker 启动。考虑映射 data_dir 以使下次启动更快" && until docker logs manticore 2>&1 | grep -q "accepting connections"; do sleep 1; echo -n .; done && echo && docker exec -it manticore mysql && docker stop manticore
```

请注意，退出 MySQL 客户端后，Manticore 容器将被停止并移除，导致没有保存的数据。有关在生产环境中使用 Manticore 的信息，请参见下文。

该镜像附带一个可以像这样加载的示例表：

```sql
mysql> source /sandbox.sql
```

此外，mysql 客户端的历史记录中有几个示例查询，您可以在上述表上运行，只需使用客户端中的向上/向下键查看并运行它们。

## 生产使用


### 端口和挂载点

为了数据持久性，文件夹 `/var/lib/manticore/` 应该挂载到本地存储或其他所需的存储引擎。

实例内部的配置文件位于 `/etc/manticoresearch/manticore.conf`。对于自定义设置，该文件应挂载到您自己的配置文件。

端口为 9306/9308/9312 用于 SQL/HTTP/二进制，根据您将如何使用 Manticore 来暴露它们。例如：

```bash
docker run --name manticore -v $(pwd)/data:/var/lib/manticore -p 127.0.0.1:9306:9306 -p 127.0.0.1:9308:9308 -d manticoresearch/manticore
```

或者

```bash
docker run --name manticore -v $(pwd)/manticore.conf:/etc/manticoresearch/manticore.conf -v $(pwd)/data:/var/lib/manticore/ -p 127.0.0.1:9306:9306 -p 127.0.0.1:9308:9308 -d manticoresearch/manticore
```

如果您希望端口对外部主机可用，请确保删除 `127.0.0.1:`。

### Manticore 列库和 Manticore Buddy

Manticore Search Docker 镜像附带预安装的 [Manticore 列库](https://github.com/manticoresoftware/columnar) 和 [Manticore Buddy](https://github.com/manticoresoftware/manticoresearch-buddy)

### Docker-compose

在许多情况下，您可能希望将 Manticore 与 Docker Compose YAML 文件中指定的其他映像一起使用。以下是在 docker-compose.yml 文件中推荐的最小配置：

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

### HTTP 协议

HTTP 协议在端口 9308 上暴露。您可以在本地映射端口并使用 curl 连接：

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
执行简单搜索：

<!-- request JSON -->
```json
POST /search -d '{"table":"testrt","query":{"match":{"*":"hello world"}}}'
```
<!-- end -->

### 日志记录

默认情况下，服务器设置为将日志发送到 `/dev/stdout`，可以从主机查看：


```bash
docker logs manticore
```

查询日志可以通过传递变量 `QUERY_LOG_TO_STDOUT=true` 转发到 Docker 日志。


### 具有复制的多节点集群

这是一个简单的 `docker-compose.yml` 用于定义一个两节点集群：

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
* 启动它： `docker-compose up`
* 创建一个集群：
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
* 如果现在返回第一个实例，你将看到新记录：
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

## 内存锁定和限制

建议覆盖 Manticore 实例的 docker 默认 ulimits：

```bash
 --ulimit nofile=65536:65536
```

为了获得最佳性能，表组件可以“mlocked”（锁定到内存中）。当 Manticore 在 Docker 下运行时，实例需要额外的权限来允许内存锁定。启动实例时必须添加以下选项：

```bash
  --cap-add=IPC_LOCK --ulimit memlock=-1:-1
```

## 使用 Docker 配置 Manticore Search

如果你想使用包含表定义的自定义配置运行 Manticore，你需要将配置挂载到实例：

```bash
docker run --name manticore -v $(pwd)/manticore.conf:/etc/manticoresearch/manticore.conf -v $(pwd)/data/:/var/lib/manticore -p 127.0.0.1:9306:9306 -d manticoresearch/manticore
```

请注意，Manticore 搜索在容器内以用户 `manticore` 运行。对表文件（如创建或旋转普通表）的操作也应在 `manticore` 用户下执行。否则，文件将以 `root` 用户创建，搜索守护进程将没有权限打开它们。例如，以下是如何旋转所有表：

```bash
docker exec -it manticore gosu manticore indexer --all --rotate
```

你还可以使用 Docker 环境变量设置单独的 `searchd` 和 `common` 配置选项。

设置必须以其部分名称为前缀，例如在 `mysql_version_string` 的情况下，变量必须命名为 `searchd_mysql_version_string`：


```bash
docker run --name manticore  -p 127.0.0.1:9306:9306  -e searchd_mysql_version_string='5.5.0' -d manticoresearch/manticore
```

关于 `listen` 指令，新的监听接口使用 Docker 变量 `searchd_listen`，并且可以使用默认接口。可以声明多个接口，用分号（"|"）分隔。要仅在网络地址上监听，可以将 `$ip`（内部通过 `hostname -i` 获取）用作地址别名。

例如 `-e searchd_listen='9316:http|9307:mysql|$ip:5443:mysql_vip'` 将在端口 9307 上添加额外的 SQL 接口，在端口 5443 上仅在实例的 IP 上运行的 SQL VIP 监听器，以及在端口 9316 上的 HTTP 监听器，此外还包括默认的 9306 和 9308。

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

### 启动标志

要使用自定义启动标志启动 Manticore，请在使用 docker run 时将其作为参数指定。确保不包含 `searchd` 命令，并包含 `--nodetach` 标志。以下是示例：
```bash
docker run --name manticore --rm manticoresearch/manticore:latest --replay-flags=ignore-trx-errors --nodetach
```

### 在非 root 下运行
默认情况下，主要的 Manticore 进程 `searchd` 在容器内以用户 `manticore` 运行，但在启动容器时运行的脚本是在您默认的 docker 用户下运行的，通常是 `root`。如果这不是你想要的，可以使用 `docker ... --user manticore` 或在 docker compose yaml 中使用 `user: manticore` 使所有操作在 `manticore` 用户下运行。请阅读下面有关可能出现的卷权限问题及其解决方法的信息。

### 启动时创建普通表
要构建在自定义配置文件中指定的普通表，可以使用环境变量 `CREATE_PLAIN_TABLES=1`。这将在 Manticore 启动之前执行 `indexer --all`。如果不使用卷，并且表很容易重建，这非常有用。
```bash
docker run -e CREATE_PLAIN_TABLES=1 --name manticore -v $(pwd)/manticore.conf:/etc/manticoresearch/manticore.conf -p 9306:9306 -p 9308:9308 -d manticoresearch/manticore
```

## 故障排除

### 挂载卷的权限问题

如果你在非 root 下运行 Manticore Search docker（使用 `docker ... --user manticore` 或在 docker compose yaml 中使用 `user: manticore`），可能会遇到权限问题，例如：
```bash
FATAL: directory /var/lib/manticore write error: failed to open /var/lib/manticore/tmp: Permission denied
```
这可能发生是因为用于在容器内运行进程的用户可能没有权限修改您已挂载到容器的目录。要解决此问题，您可以对挂载的目录执行 `chown` 或 `chmod`。如果您以用户 `manticore` 运行容器，则需要执行：
```bash
chown -R 999:999 data
```

因为用户 `manticore` 在容器内的 ID 是 999。

<!-- proofread -->

