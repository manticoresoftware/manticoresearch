# Запуск и использование Manticore в Docker

Образ основан на [текущем релизе пакета Manticore](https://manticoresearch.com/install/).

[comment]: # (the below should be in sync with https://github.com/manticoresoftware/docker/blob/master/README.md)

Стандартная конфигурация включает пример Real-Time таблицы и прослушивает на стандартных портах:

  * `9306` для подключений клиента MySQL
  * `9308` для подключений через HTTP
  * `9312` для подключений через бинарный протокол (например, если вы запускаете кластер)

Образ поставляется с библиотеками для лёгкой индексации данных из MySQL, PostgreSQL, XML и CSV файлов.

## Как запустить Docker образ Manticore Search

### Быстрый старт

Ниже приведён самый простой способ запустить Manticore в контейнере и войти в него через mysql клиент:

```bash
docker run --name manticore --rm -d manticoresearch/manticore && echo "Waiting for Manticore docker to start. Consider mapping the data_dir to make it start faster next time" && until docker logs manticore 2>&1 | grep -q "accepting connections"; do sleep 1; echo -n .; done && echo && docker exec -it manticore mysql && docker stop manticore
```

Обратите внимание, что после выхода из MySQL клиента контейнер Manticore будет остановлен и удалён, из-за чего данные не сохранятся. Для информации о использовании Manticore в продакшн-среде смотрите ниже.

Образ поставляется с примером таблицы, которую можно загрузить так:

```sql
mysql> source /sandbox.sql
```

Также mysql клиент имеет несколько примеров запросов в своей истории, которые вы можете выполнить на указанной таблице, просто используйте клавиши Вверх/Вниз, чтобы просмотреть и запустить их.

### Использование в продакшене


#### Порты и точки монтирования

Для сохранения данных папка `/var/lib/manticore/` должна быть смонтирована к локальному хранилищу или другому желаемому хранилищу.

Конфигурационный файл внутри инстанса располагается по пути `/etc/manticoresearch/manticore.conf`. Для пользовательских настроек этот файл должен быть смонтирован к вашему собственному конфигурационному файлу.

Порты — 9306/9308/9312 для SQL/HTTP/Бинарного, выставляйте их наружу в зависимости от того, как вы собираетесь использовать Manticore. Например:

```bash
docker run --name manticore -v $(pwd)/data:/var/lib/manticore -p 127.0.0.1:9306:9306 -p 127.0.0.1:9308:9308 -d manticoresearch/manticore
```

или

```bash
docker run --name manticore -v $(pwd)/manticore.conf:/etc/manticoresearch/manticore.conf -v $(pwd)/data:/var/lib/manticore/ -p 127.0.0.1:9306:9306 -p 127.0.0.1:9308:9308 -d manticoresearch/manticore
```

Обязательно уберите `127.0.0.1:`, если хотите, чтобы порты были доступны для внешних хостов.

#### Manticore Columnar Library and Manticore Buddy

Docker образ Manticore Search поставляется с предустановленными [Manticore Columnar Library](https://github.com/manticoresoftware/columnar) и [Manticore Buddy](https://github.com/manticoresoftware/manticoresearch-buddy)

#### Docker-compose

Во многих случаях вы можете захотеть использовать Manticore вместе с другими образами, указанными в YAML файле Docker Compose. Ниже минимальная рекомендуемая конфигурация для Manticore Search в docker-compose.yml файле:

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

HTTP протокол доступен на порту 9308. Вы можете пробросить этот порт локально и подключиться с помощью curl:

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

По умолчанию сервер настроен на отправку логов в `/dev/stdout`, которые можно просматривать с хоста с помощью:


```bash
docker logs manticore
```

Логи запросов можно направить в Docker лог, передав переменную `QUERY_LOG_TO_STDOUT=true`.


#### Кластер из нескольких узлов с репликацией

Ниже простой пример `docker-compose.yml` для определения кластера из двух узлов:

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
* Запуск: `docker-compose up`
* Создание кластера:
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
* Подключение ко 2-му узлу к кластеру
  ```sql
  $ docker-compose exec manticore-2 mysql

  mysql> JOIN CLUSTER posts AT 'manticore-1:9312';
  mysql> INSERT INTO posts:testrt(title,content,gid)  VALUES('hello','world',1);
  Query OK, 1 row affected (0.00 sec)

  MySQL [(none)]> exit
  Bye
  ```
* Если теперь вернуться к первому узлу, вы увидите новую запись:
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

Рекомендуется переопределить значения ulimits docker для инстанса Manticore:

```bash
 --ulimit nofile=65536:65536
```

Для лучшей производительности компоненты таблиц могут быть "заблокированы" (mlocked) в памяти. При запуске Manticore в Docker инстанс требует дополнительных привилегий для блокировки памяти. Следующие опции должны быть добавлены при запуске инстанса:

```bash
  --cap-add=IPC_LOCK --ulimit memlock=-1:-1
```

### Настройка Manticore Search с Docker

Если вы хотите запустить Manticore с пользовательской конфигурацией, которая включает определения таблиц, необходимо примонтировать конфигурацию к инстансу:

```bash
docker run --name manticore -v $(pwd)/manticore.conf:/etc/manticoresearch/manticore.conf -v $(pwd)/data/:/var/lib/manticore -p 127.0.0.1:9306:9306 -d manticoresearch/manticore
```

Учитывайте, что поиск Manticore внутри контейнера запускается под пользователем `manticore`. Все операции с файлами таблиц (например, создание или ротация plain таблиц) также должны выполняться под `manticore`. Иначе файлы создадутся от `root`, и поисковый демон не сможет их открыть. Например, вот как можно сделать ротацию всех таблиц:

```bash
docker exec -it manticore gosu manticore indexer --all --rotate
```

Также можно задать отдельные настройки `searchd` и `common` с помощью переменных окружения Docker.

Настройки должны иметь префикс с названием секции, пример: для `mysql_version_string` переменной должно быть имя `searchd_mysql_version_string`:


```bash
docker run --name manticore  -p 127.0.0.1:9306:9306  -e searchd_mysql_version_string='5.5.0' -d manticoresearch/manticore
```

В случае директивы `listen` новые интерфейсы прослушивания задаются с помощью Docker переменной `searchd_listen` дополнительно к стандартным. Можно объявлять несколько интерфейсов, разделённых точкой с запятой ("|"). Для прослушивания только сетевого адреса можно использовать `$ip` (который извлекается внутренне через `hostname -i`) в качестве псевдонима адреса.

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

#### Параметры запуска

Чтобы запустить Manticore с пользовательскими параметрами запуска, укажите их в качестве аргументов при использовании docker run. Убедитесь, что не включаете команду `searchd` и добавляете флаг `--nodetach`. Пример:
```bash
docker run --name manticore --rm manticoresearch/manticore:latest --replay-flags=ignore-trx-errors --nodetach
```

#### Запуск от имени не-root пользователя
По умолчанию основной процесс Manticore `searchd` запускается от имени пользователя `manticore` внутри контейнера, но скрипт, который выполняется при запуске контейнера, запускается от вашего стандартного пользователя docker, который в большинстве случаев является `root`. Если вы хотите иначе, вы можете использовать `docker ... --user manticore` или `user: manticore` в docker compose yaml, чтобы всё запускалось от имени `manticore`. Ниже описаны возможные проблемы с разрешениями для томов и способы их решения.

#### Создание обычных таблиц при запуске
Чтобы создавать обычные таблицы, указанные в вашем пользовательском конфигурационном файле, можно использовать переменную окружения `CREATE_PLAIN_TABLES=1`. Это выполнит команду `indexer --all` перед запуском Manticore. Это полезно, если вы не используете тома и ваши таблицы легко пересоздаются.
```bash
docker run -e CREATE_PLAIN_TABLES=1 --name manticore -v $(pwd)/manticore.conf:/etc/manticoresearch/manticore.conf -p 9306:9306 -p 9308:9308 -d manticoresearch/manticore
```

### Устранение неполадок

#### Проблемы с разрешениями у смонтированного тома

Если вы запускаете Manticore Search docker от имени не-root пользователя (с помощью `docker ... --user manticore` или `user: manticore` в docker compose yaml), возможно возникновение проблем с разрешениями, например:
```bash
FATAL: directory /var/lib/manticore write error: failed to open /var/lib/manticore/tmp: Permission denied
```

Такое может происходить, потому что пользователь, под которым запускаются процессы внутри контейнера, может не иметь прав на изменение директории, смонтированной в контейнер. Для решения вы можете изменить владельца или права с помощью `chown` или `chmod` на монтируемую директорию. Если вы запускаете контейнер от имени пользователя `manticore`, нужно сделать следующее:
```bash
chown -R 999:999 data
```

поскольку пользователь `manticore` имеет ID 999 внутри контейнера.

<!-- proofread -->

