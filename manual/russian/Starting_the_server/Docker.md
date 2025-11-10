# Запуск и использование Manticore в Docker

Образ основан на [текущем релизе пакета Manticore](https://manticoresearch.com/install/).

[comment]: # (the below should be in sync with https://github.com/manticoresoftware/docker/blob/master/README.md)

Конфигурация по умолчанию включает пример таблицы Real-Time и слушает на стандартных портах:

  * `9306` для подключений от MySQL клиента
  * `9308` для подключений через HTTP
  * `9312` для подключений по бинарному протоколу (например, если вы запускаете кластер)

Образ поставляется с библиотеками для удобного индексирования данных из MySQL, PostgreSQL, XML и CSV файлов.

## Как запустить Docker-образ Manticore Search

### Быстрый запуск

Ниже приведён самый простой способ запустить Manticore в контейнере и войти в него через mysql клиент:

```bash
docker run --name manticore --rm -d manticoresearch/manticore && echo "Waiting for Manticore docker to start. Consider mapping the data_dir to make it start faster next time" && until docker logs manticore 2>&1 | grep -q "accepting connections"; do sleep 1; echo -n .; done && echo && docker exec -it manticore mysql && docker stop manticore
```

Обратите внимание, что при выходе из MySQL клиента контейнер Manticore будет остановлен и удалён, в результате данные не сохранятся. Для информации по использованию Manticore в продакшене смотрите ниже.

Образ содержит пример таблицы, которую можно загрузить так:

```sql
mysql> source /sandbox.sql
```

Также mysql клиент имеет несколько примеров запросов в своей истории, которые вы можете выполнить на вышеуказанной таблице, просто используйте клавиши Вверх/Вниз в клиенте для просмотра и запуска.

### Использование в продакшене


#### Порты и точки монтирования

Для сохранения данных папка `/var/lib/manticore/` должна быть смонтирована в локальное хранилище или другой желаемый движок хранения.

Файл конфигурации внутри инстанса находится по пути `/etc/manticoresearch/manticore.conf`. Для пользовательских настроек этот файл должен быть смонтирован в ваш собственный конфигурационный файл.

Порты 9306/9308/9312 предназначены для SQL/HTTP/Бинарного протокола, откройте их в зависимости от того, как вы собираетесь использовать Manticore. Например:

```bash
docker run --name manticore -v $(pwd)/data:/var/lib/manticore -p 127.0.0.1:9306:9306 -p 127.0.0.1:9308:9308 -d manticoresearch/manticore
```

или

```bash
docker run --name manticore -v $(pwd)/manticore.conf:/etc/manticoresearch/manticore.conf -v $(pwd)/data:/var/lib/manticore/ -p 127.0.0.1:9306:9306 -p 127.0.0.1:9308:9308 -d manticoresearch/manticore
```

Убедитесь, что удалили `127.0.0.1:`, если хотите, чтобы порты были доступны для внешних хостов.

#### Библиотека Manticore Columnar и Manticore Buddy

Docker-образ Manticore Search поставляется с предустановленными [Manticore Columnar Library](https://github.com/manticoresoftware/columnar) и [Manticore Buddy](https://github.com/manticoresoftware/manticoresearch-buddy)

#### Docker-compose

Во многих случаях вы можете захотеть использовать Manticore вместе с другими образами, указанными в YAML-файле Docker Compose. Ниже минимальная рекомендуемая конфигурация для Manticore Search в файле docker-compose.yml:

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

Помимо использования открытых портов 9306 и 9308, вы можете войти в инстанс, выполнив `docker-compose exec manticore mysql`.

#### HTTP протокол

HTTP протокол доступен на порту 9308. Вы можете пробросить порт локально и подключиться с помощью curl:

```bash
docker run --name manticore -p 9308:9308 -d manticoresearch/manticore
```

<!-- example create -->
Создайте таблицу:

<!-- request JSON -->
```json
POST /cli -d 'CREATE TABLE testrt ( title text, content text, gid integer)'
```
<!-- end -->
<!-- example insert -->
Вставьте документ:

<!-- request JSON -->
```json
POST /insert
-d'{"table":"testrt","id":1,"doc":{"title":"Hello","content":"world","gid":1}}'
```
<!-- end -->
<!-- example search -->
Выполните простой поиск:

<!-- request JSON -->
```json
POST /search -d '{"table":"testrt","query":{"match":{"*":"hello world"}}}'
```
<!-- end -->

### Логирование

По умолчанию сервер настроен отправлять логи в `/dev/stdout`, которые можно просмотреть с хоста с помощью:


```bash
docker logs manticore
```

Лог запросов можно направить в Docker лог, передав переменную `QUERY_LOG_TO_STDOUT=true`.


#### Многозвенный кластер с репликацией

Вот простой `docker-compose.yml` для определения кластера из двух узлов:

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
* Запустите его: `docker-compose up`
* Создайте кластер:
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
* Присоединитесь к кластеру на втором инстансе
  ```sql
  $ docker-compose exec manticore-2 mysql

  mysql> JOIN CLUSTER posts AT 'manticore-1:9312';
  mysql> INSERT INTO posts:testrt(title,content,gid)  VALUES('hello','world',1);
  Query OK, 1 row affected (0.00 sec)

  MySQL [(none)]> exit
  Bye
  ```
* Если теперь вернуться к первому инстансу, вы увидите новую запись:
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

### Блокировка памяти и лимиты

Рекомендуется переопределить стандартные ulimits docker для инстанса Manticore:

```bash
 --ulimit nofile=65536:65536
```

Для лучшей производительности компоненты таблиц могут быть "mlocked" в память. При запуске Manticore под Docker инстансу требуются дополнительные привилегии для разрешения блокировки памяти. Следующие опции должны быть добавлены при запуске инстанса:

```bash
  --cap-add=IPC_LOCK --ulimit memlock=-1:-1
```

### Настройка Manticore Search с Docker

Если вы хотите запустить Manticore с пользовательской конфигурацией, включающей определения таблиц, вам нужно смонтировать конфигурацию в инстанс:

```bash
docker run --name manticore -v $(pwd)/manticore.conf:/etc/manticoresearch/manticore.conf -v $(pwd)/data/:/var/lib/manticore -p 127.0.0.1:9306:9306 -d manticoresearch/manticore
```

Учтите, что Manticore search внутри контейнера запускается под пользователем `manticore`. Операции с файлами таблиц (например, создание или ротация plain таблиц) также должны выполняться под `manticore`. Иначе файлы будут созданы под `root`, и демон поиска не сможет их открыть. Например, вот как можно выполнить ротацию всех таблиц:

```bash
docker exec -it manticore gosu manticore indexer --all --rotate
```

Вы также можете задать отдельные настройки `searchd` и `common` через переменные окружения Docker.

Настройки должны иметь префикс с именем секции, пример для `mysql_version_string` — переменная должна называться `searchd_mysql_version_string`:


```bash
docker run --name manticore  -p 127.0.0.1:9306:9306  -e searchd_mysql_version_string='5.5.0' -d manticoresearch/manticore
```

В случае директивы `listen` новые интерфейсы прослушивания добавляются с помощью Docker переменной `searchd_listen` в дополнение к стандартным. Можно объявить несколько интерфейсов, разделённых точкой с запятой ("|"). Для прослушивания только на сетевом адресе можно использовать `$ip` (получаемый внутренне из `hostname -i`) как псевдоним адреса.

For example `-e searchd_listen='9316:http|9307:mysql|$ip:5443:mysql_vip'` will add an additional SQL interface on port 9307, an SQL VIP listener on port 5443 running only on the instance's IP, and an HTTP listener on port 9316, in addition to the defaults on 9306 and 9308, respectively.

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

#### Startup flags

To start Manticore with custom startup flags, specify them as arguments when using docker run. Ensure you do not include the `searchd` command and include the `--nodetach` flag. Here's an example:
```bash
docker run --name manticore --rm manticoresearch/manticore:latest --replay-flags=ignore-trx-errors --nodetach
```

#### Running under non-root
By default, the main Manticore process `searchd` is running under user `manticore` inside the container, but the script which runs on starting the container is run under your default docker user which in most cases is `root`. If that's not what you want you can use `docker ... --user manticore` or `user: manticore` in docker compose yaml to make everything run under `manticore`. Read below about possible volume permissions issue you can get and how to solve it.

#### Creating plain tables on startup
To build plain tables specified in your custom configuration file, you can use the `CREATE_PLAIN_TABLES=1` environment variable. It will execute `indexer --all` before Manticore starts. This is useful if you don't use volumes, and your tables are easy to recreate.
```bash
docker run -e CREATE_PLAIN_TABLES=1 --name manticore -v $(pwd)/manticore.conf:/etc/manticoresearch/manticore.conf -p 9306:9306 -p 9308:9308 -d manticoresearch/manticore
```

### Troubleshooting

#### Permissions issue with a mounted volume

In case you are running Manticore Search docker under non-root (using `docker ... --user manticore` or `user: manticore` in docker compose yaml), you can face a permissions issue, for example:
```bash
FATAL: directory /var/lib/manticore write error: failed to open /var/lib/manticore/tmp: Permission denied
```

This can happen because the user which is used to run processes inside the container may have no permissions to modify the directory you have mounted to the container. To fix it you can `chown` or `chmod` the mounted directory. If you run the container under user `manticore` you need to do:
```bash
chown -R 999:999 data
```

since user `manticore` has ID 999 inside the container.

<!-- proofread -->

