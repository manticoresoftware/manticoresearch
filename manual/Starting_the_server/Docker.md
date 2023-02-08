# Starting and using Manticore in Docker

The image is based on [current release of Manticore package](https://manticoresearch.com/install/).

[comment]: # (the below should be in sync with https://github.com/manticoresoftware/docker/blob/master/README.md)

The default configuration includes a sample Real-Time table and listens on the default ports:

  * `9306` for connections from a MySQL client
  * `9308` for connections via HTTP
  * `9312` for connections via a binary protocol (e.g. in case you run a cluster)

The image comes with libraries for easy indexing data from MySQL, PostgreSQL XML and CSV files.

# How to run Manticore Search Docker image

## Quick usage

The below is the simplest way to start Manticore in a container and log in to it via mysql client:

```bash
docker run -e EXTRA=1 --name manticore --rm -d manticoresearch/manticore && echo "Waiting for Manticore docker to start. Consider mapping the data_dir to make it start faster next time" && until docker logs manticore 2>&1 | grep -q "accepting connections"; do sleep 1; echo -n .; done && echo && docker exec -it manticore mysql && docker stop manticore
```

Note that upon exiting the MySQL client, the Manticore container will be stopped and removed, resulting in no saved data. For information on using Manticore in a production environment, please see below.

The image comes with a sample table which can be loaded like this:

```sql
mysql> source /sandbox.sql
```

Also the mysql client has in history several sample queries that you can run on the above table, just use Up/Down keys in the client to see and run them.

## Production use


### Ports and mounting points

For data persistence the folder `/var/lib/manticore/` should be mounted to local storage or other desired storage engine.

Configuration file inside the instance is located at  `/etc/manticoresearch/manticore.conf`. For custom settings, this file should be mounted to own configuration file.

The ports are 9306/9308/9312 for SQL/HTTP/Binary, expose them depending on how you are going to use Manticore. For example:

```bash
docker run -e EXTRA=1 --name manticore -v $(pwd)/data:/var/lib/manticore -p 127.0.0.1:9306:9306 -p 127.0.0.1:9308:9308 -d manticoresearch/manticore
```

or

```bash
docker run -e EXTRA=1 --name manticore -v $(pwd)/manticore.conf:/etc/manticoresearch/manticore.conf -v $(pwd)/data:/var/lib/manticore/ -p 127.0.0.1:9306:9306 -p 127.0.0.1:9308:9308 -d manticoresearch/manticore
```

Make sure to remove `127.0.0.1:` if you want the ports to be available for external hosts.

### Docker-compose

In many cases you might want to use Manticore together with other images specified in a docker-compose YAML file. Here is the minimal recommended specification for Manticore Search in docker-compose.yml:

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

Besides using the exposed ports 9306 and 9308 you can log into the instance by running `docker-compose exec manticore mysql`.

### HTTP protocol


HTTP protocol is exposed on port 9308. You can map the port locally and connect with curl:

```bash
docker run -e EXTRA=1 --name manticore -p 9308:9308 -d manticoresearch/manticore
```

<!-- example create -->
Create a table:

<!-- request JSON -->
```json
POST /cli -d 'CREATE TABLE testrt ( title text, content text, gid integer)'
```
<!-- end -->
<!-- example insert -->
Insert a document:

<!-- request JSON -->
```json
POST /insert
-d'{"index":"testrt","id":1,"doc":{"title":"Hello","content":"world","gid":1}}'
```
<!-- end -->
<!-- example search -->
Perform a simple search:

<!-- request JSON -->
```json
POST /search -d '{"index":"testrt","query":{"match":{"*":"hello world"}}}'
```
<!-- end -->

### Logging

By default, the server is set to send it's logging to `/dev/stdout`, which can be viewed from the host with:


```bash
docker logs manticore
```

The query log can be diverted to Docker log by passing variable `QUERY_LOG_TO_STDOUT=true`.


### Multi-node cluster with replication

Here is a simple `docker-compose.yml` for defining a two node cluster:

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
* Start it: `docker-compose up`
* Create a cluster:
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
* Join to the the cluster on the 2nd instance
  ```sql
  $ docker-compose exec manticore-2 mysql

  mysql> JOIN CLUSTER posts AT 'manticore-1:9312';
  mysql> INSERT INTO posts:testrt(title,content,gid)  VALUES('hello','world',1);
  Query OK, 1 row affected (0.00 sec)

  MySQL [(none)]> exit
  Bye
  ```
* If you now go back to the first instance you'll see the new record:
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

## Memory locking and limits

It's recommended to overwrite the default ulimits of docker for the Manticore instance:

```bash
 --ulimit nofile=65536:65536
```

For best performance, table components can be "mlocked" into memory. When Manticore is run under Docker, the instance requires additional privileges to allow memory locking. The following options must be added when running the instance:

```bash
  --cap-add=IPC_LOCK --ulimit memlock=-1:-1
```

## Configuring Manticore Search with Docker

If you want to run Manticore with your custom config containing tables definition you will need to mount the configuration to the instance:

```bash
docker run -e EXTRA=1 --name manticore -v $(pwd)/manticore.conf:/etc/manticoresearch/manticore.conf -v $(pwd)/data/:/var/lib/manticore -p 127.0.0.1:9306:9306 -d manticoresearch/manticore
```

Take into account that Manticore search inside the container is run under user `manticore`. Performing operations with table files (like creating or rotating plain tables) should be also done under `manticore`. Otherwise the files will be created under `root` and the search daemon won't have rights to open them. For example here is how you can rotate all tables:

```bash
docker exec -it manticore gosu manticore indexer --all --rotate
```

You can also set individual `searchd` and `common` configuration settings using Docker environment variables.  

The settings must be prefixed with their section name, example for in case of `mysql_version_string` the variable must be named `searchd_mysql_version_string`:


```bash
docker run -e EXTRA=1 --name manticore  -p 127.0.0.1:9306:9306  -e searchd_mysql_version_string='5.5.0' -d manticoresearch/manticore
```

In case of `listen` directive, you can pass using Docker variable `searchd_listen`  new listening interfaces in addition to the default ones. Multiple interfaces can be declared separated by semi-colon ("|").
For listening only on  network address, the `$ip` (retrieved internally from `hostname -i`) can be used as address alias.

For example `-e searchd_listen='9316:http|9307:mysql|$ip:5443:mysql_vip'` will add an additional SQL interface on port 9307, an SQL VIP listener on port 5443 running only on the instance's IP and an HTTP listener on port 9316, beside the defaults on 9306 and 9308, respectively.

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
