# 在 Docker 中启动和使用 Manticore

该镜像基于 [当前发布的 Manticore 包](https://manticoresearch.com/install/)。

默认配置包含一个示例实时表，并监听以下默认端口：

  * `9306` 用于 MySQL 客户端连接
  * `9308` 用于 HTTP 连接
  * `9312` 用于二进制协议连接（例如在运行集群时使用）

镜像附带了用于轻松从 MySQL、PostgreSQL、XML 和 CSV 文件中索引数据的库。

# 如何运行 Manticore Search Docker 镜像

## 快速使用

以下是启动 Manticore 容器并通过 MySQL 客户端登录的最简单方法：

```bash
docker run -e EXTRA=1 --name manticore --rm -d manticoresearch/manticore && echo "Waiting for Manticore docker to start. Consider mapping the data_dir to make it start faster next time" && until docker logs manticore 2>&1 | grep -q "accepting connections"; do sleep 1; echo -n .; done && echo && docker exec -it manticore mysql && docker stop manticore
```

注意：退出 MySQL 客户端后，Manticore 容器将停止并被移除，因此不会保存任何数据。有关在生产环境中使用 Manticore 的信息，请参阅下文。

镜像附带了一个可以加载的示例表：

```sql
mysql> source /sandbox.sql
```

此外，MySQL 客户端中有几个示例查询，它们可以在上述表上运行，只需在客户端中使用上下键查看并运行它们。

## 生产环境使用


### 端口和挂载点

为了持久化数据，文件夹 `/var/lib/manticore/` 应挂载到本地存储或其他期望的存储引擎。

容器内的配置文件位于 `/etc/manticoresearch/manticore.conf`。对于自定义设置，此文件应挂载到您自己的配置文件中。

SQL/HTTP/二进制协议的端口分别是 9306/9308/9312，暴露这些端口以便根据您如何使用 Manticore。例如：

```bash
docker run -e EXTRA=1 --name manticore -v $(pwd)/data:/var/lib/manticore -p 127.0.0.1:9306:9306 -p 127.0.0.1:9308:9308 -d manticoresearch/manticore
```

或

```bash
docker run -e EXTRA=1 --name manticore -v $(pwd)/manticore.conf:/etc/manticoresearch/manticore.conf -v $(pwd)/data:/var/lib/manticore/ -p 127.0.0.1:9306:9306 -p 127.0.0.1:9308:9308 -d manticoresearch/manticore
```

确保在希望端口对外部主机可用时删除 `127.0.0.1:`。

### Manticore Columnar Library 和 Manticore Buddy

Manticore Search Docker 镜像不预装 [Manticore Columnar Library](https://github.com/manticoresoftware/columnar)，如需要列式存储和二级索引可以启用此库。不过，可以通过设置环境变量 `EXTRA=1` 在运行时轻松启用它。例如，`docker run -e EXTRA=1 ... manticoresearch/manticore`。这将下载并安装库到数据目录中（在生产环境中通常映射为卷），并且除非 Manticore Search 版本发生变化，否则不会重新下载。

使用 `EXTRA=1` 还会激活 [Manticore Buddy](https://github.com/manticoresoftware/manticoresearch-buddy)，用于处理某些命令。有关更多信息，请参阅 [changelog](https://manual.manticoresearch.com/Changelog#Version-6.0.0)。

如果您只需要 MCL，可以使用环境变量 `MCL=1`。

### Docker-compose

在许多情况下，您可能希望与其他镜像一起在 Docker Compose YAML 文件中使用 Manticore。以下是 docker-compose.yml 文件中 Manticore Search 的最小推荐配置：

```yaml
version: '2.2'

services:
  manticore:
    container_name: manticore
    image: manticoresearch/manticore
    environment:
      - EXTRA=1
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

除了使用暴露的 9306 和 9308 端口之外，您还可以通过运行 `docker-compose exec manticore mysql` 登录实例。

### HTTP 协议

HTTP 协议暴露在 9308 端口。您可以在本地映射该端口并通过 curl 连接：

```bash
docker run -e EXTRA=1 --name manticore -p 9308:9308 -d manticoresearch/manticore
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
-d'{"index":"testrt","id":1,"doc":{"title":"Hello","content":"world","gid":1}}'
```
<!-- end -->
<!-- example search -->
进行简单搜索：

<!-- request JSON -->
```json
POST /search -d '{"index":"testrt","query":{"match":{"*":"hello world"}}}'
```
<!-- end -->

### 日志记录

默认情况下，服务器设置为将日志发送到 `/dev/stdout`，可以通过主机查看：


```bash
docker logs manticore
```

通过传递变量 `QUERY_LOG_TO_STDOUT=true` 可以将查询日志重定向到 Docker 日志。


### 多节点集群与复制

这是一个定义两节点集群的简单 `docker-compose.yml` 文件：

```yaml
version: '2.2'

services:

  manticore-1:
    image: manticoresearch/manticore
    environment:
      - EXTRA=1
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
    environment:
      - EXTRA=1
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
* 如果您现在返回第一个实例，您将看到新记录：
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

建议覆盖 Docker 的默认 ulimit 设置以适应 Manticore 实例：

```bash
 --ulimit nofile=65536:65536
```

为了获得最佳性能，可以将表组件锁定在内存中。运行在 Docker 下时，Manticore 实例需要额外的权限来允许内存锁定。运行实例时，必须添加以下选项：

```bash
  --cap-add=IPC_LOCK --ulimit memlock=-1:-1
```

## 使用 Docker 配置 Manticore Search

如果您希望使用自定义配置文件运行 Manticore，并且配置文件中包含表定义，则需要将配置文件挂载到实例中：

```bash
docker run -e EXTRA=1 --name manticore -v $(pwd)/manticore.conf:/etc/manticoresearch/manticore.conf -v $(pwd)/data/:/var/lib/manticore -p 127.0.0.1:9306:9306 -d manticoresearch/manticore
```

请注意，容器内的 Manticore Search 是以 `manticore` 用户身份运行的。执行与表文件相关的操作（例如创建或旋转平面表）时也应以 `manticore` 用户身份进行。否则，文件将以 `root` 用户身份创建，搜索守护进程将没有权限打开它们。例如，以下是如何旋转所有表：

```bash
docker exec -it manticore gosu manticore indexer --all --rotate
```

您还可以使用 Docker 环境变量设置单个 `searchd` 和 `common` 配置。设置必须以其部分名称为前缀，例如 `mysql_version_string` 的变量必须命名为 `searchd_mysql_version_string`：


```bash
docker run -e EXTRA=1 --name manticore  -p 127.0.0.1:9306:9306  -e searchd_mysql_version_string='5.5.0' -d manticoresearch/manticore
```

对于 `listen` 指令，可以使用 Docker 变量 `searchd_listen` 声明新的监听接口，且可以与默认值一起使用。多个接口可以通过分号（“|”）分隔声明。要仅监听网络地址，可以使用 `$ip`（从 `hostname -i` 内部检索）作为地址别名。

例如，`-e searchd_listen='9316:http|9307:mysql|$ip:5443:mysql_vip'` 将在端口 9307 添加一个额外的 SQL 接口，在端口 5443 添加一个仅在实例 IP 上运行的 SQL VIP 监听器，以及一个在端口 9316 上运行的 HTTP 监听器，这些都将在默认的 9306 和 9308 端口之外运行。

```bash
$ docker run -e EXTRA=1 --rm -p 1188:9307  -e searchd_mysql_version_string='5.5.0' -e searchd_listen='9316:http|9307:mysql|$ip:5443:mysql_vip'  manticore
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

要使用自定义启动标志启动 Manticore，请在使用 `docker run` 时将它们指定为参数。确保不要包含 `searchd` 命令，并且包含 `--nodetach` 标志。以下是一个示例：

```bash
docker run -e EXTRA=1 --name manticore --rm manticoresearch/manticore:latest --replay-flags=ignore-trx-errors --nodetach
```

### 以非 root 身份运行

默认情况下，容器内部的主 Manticore 进程 `searchd` 以 `manticore` 用户身份运行，但启动容器时运行的脚本是以默认的 Docker 用户身份（通常是 `root`）运行的。如果您不希望这样，可以使用 `docker ... --user manticore` 或在 docker-compose yaml 文件中使用 `user: manticore` 来让所有进程都以 `manticore` 身份运行。请参阅下文，了解可能出现的卷权限问题及其解决方法。

### 启动时创建平面表

要构建在自定义配置文件中指定的平面表，您可以使用 `CREATE_PLAIN_TABLES=1` 环境变量。它将在 Manticore 启动之前执行 `indexer --all`。这在不使用卷且表易于重建时非常有用。

```bash
docker run -e CREATE_PLAIN_TABLES=1 --name manticore -v $(pwd)/manticore.conf:/etc/manticoresearch/manticore.conf -p 9306:9306 -p 9308:9308 -d manticoresearch/manticore
```

## 故障排除

### 挂载卷的权限问题

如果您以非 root 身份运行 Manticore Search docker（使用 `docker ... --user manticore` 或在 docker-compose yaml 中使用 `user: manticore`），可能会遇到权限问题，例如：

```bash
FATAL: directory /var/lib/manticore write error: failed to open /var/lib/manticore/tmp: Permission denied
```

或者在使用 `-e EXTRA=1` 时：

```bash
mkdir: cannot create directory ‘/var/lib/manticore/.mcl/’: Permission denied
```

这种情况的发生是因为用于在容器内运行进程的用户可能没有修改您挂载到容器的目录的权限。要解决此问题，您可以使用 `chown` 或 `chmod` 修改挂载目录的权限。如果您在以 `manticore` 用户身份运行容器，您需要执行：
```bash
chown -R 999:999 data
```

因为容器内的 `manticore` 用户的 ID 是 999。

<!-- proofread -->
