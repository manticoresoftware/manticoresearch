# Запуск и использование Manticore в Docker

Образ основан на [текущем релизе пакета Manticore](https://manticoresearch.com/install/).

[comment]: # (нижеследующее должно быть синхронизировано с https://github.com/manticoresoftware/docker/blob/master/README.md)

Конфигурация по умолчанию включает пример таблицы Real-Time и прослушивает стандартные порты:

  * `9306` для подключений от клиента MySQL
  * `9308` для подключений через HTTP
  * `9312` для подключений через бинарный протокол (например, если вы запускаете кластер)

Образ поставляется с библиотеками для удобного индексирования данных из MySQL, PostgreSQL, XML и CSV файлов.

## Как запустить Docker-образ Manticore Search

### Быстрое использование

Ниже приведен самый простой способ запустить Manticore в контейнере и войти в него через mysql-клиент:

```bash
docker run --name manticore --rm -d manticoresearch/manticore && echo "Waiting for Manticore docker to start. Consider mapping the data_dir to make it start faster next time" && until docker logs manticore 2>&1 | grep -q "accepting connections"; do sleep 1; echo -n .; done && echo && docker exec -it manticore mysql && docker stop manticore
```

Обратите внимание, что после выхода из MySQL-клиента контейнер Manticore будет остановлен и удален, что приведет к потере всех данных. Для информации об использовании Manticore в производственной среде, пожалуйста, смотрите ниже.

Образ поставляется с примером таблицы, которую можно загрузить следующим образом:

```sql
mysql> source /sandbox.sql
```

Также, mysql-клиент имеет несколько примеров запросов в своей истории, которые вы можете выполнить на указанной выше таблице, просто используйте клавиши Вверх/Вниз в клиенте, чтобы увидеть и запустить их.

### Использование в production


#### Порты и точки монтирования

Для сохранения данных папка `/var/lib/manticore/` должна быть смонтирована на локальное хранилище или другой желаемый механизм хранения.

Файл конфигурации внутри экземпляра находится по пути `/etc/manticoresearch/manticore.conf`. Для пользовательских настроек этот файл должен быть смонтирован на ваш собственный конфигурационный файл.

Порты: 9306/9308/9312 для SQL/HTTP/Binary, пробрасывайте их в зависимости от того, как вы собираетесь использовать Manticore. Например:

```bash
docker run --name manticore -v $(pwd)/data:/var/lib/manticore -p 127.0.0.1:9306:9306 -p 127.0.0.1:9308:9308 -d manticoresearch/manticore
```

или

```bash
docker run --name manticore -v $(pwd)/manticore.conf:/etc/manticoresearch/manticore.conf -v $(pwd)/data:/var/lib/manticore/ -p 127.0.0.1:9306:9306 -p 127.0.0.1:9308:9308 -d manticoresearch/manticore
```

Убедитесь, что удалили `127.0.0.1:`, если хотите, чтобы порты были доступны для внешних хостов.

#### Manticore Columnar Library и Manticore Buddy

Docker-образ Manticore Search поставляется с предустановленными [Manticore Columnar Library](https://github.com/manticoresoftware/columnar) и [Manticore Buddy](https://github.com/manticoresoftware/manticoresearch-buddy)

#### Docker-compose

Во многих случаях вы можете захотеть использовать Manticore вместе с другими образами, указанными в YAML-файле Docker Compose. Ниже приведена минимальная рекомендуемая конфигурация для Manticore Search в файле docker-compose.yml:

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

Помимо использования проброшенных портов 9306 и 9308, вы можете войти в экземпляр, выполнив `docker-compose exec manticore mysql`.

#### HTTP протокол

HTTP протокол доступен на порту 9308. Вы можете пробросить порт локально и подключиться с помощью curl.:

```bash
docker run --name manticore -p 9308:9308 -d manticoresearch/manticore
```

<!-- example create -->
Создать таблицу:

<!-- request JSON -->
```json
POST /cli -d 'CREATE TABLE testrt ( title text, content text, gid integer)'
```
<!-- end -->
<!-- example insert -->
Вставить документ:

<!-- request JSON -->
```json
POST /insert
-d'{"table":"testrt","id":1,"doc":{"title":"Hello","content":"world","gid":1}}'
```
<!-- end -->
<!-- example search -->
Выполнить простой поиск:

<!-- request JSON -->
```json
POST /search -d '{"table":"testrt","query":{"match":{"*":"hello world"}}}'
```
<!-- end -->

### Логирование

По умолчанию сервер настроен на отправку своих логов в `/dev/stdout`, что можно просмотреть с хоста с помощью:


```bash
docker logs manticore
```

Лог запросов может быть перенаправлен в лог Docker путем передачи переменной `QUERY_LOG_TO_STDOUT=true`.


#### Кластер из нескольких узлов с репликацией

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
* Присоединитесь к кластеру на 2-м экземпляре
  ```sql
  $ docker-compose exec manticore-2 mysql

  mysql> JOIN CLUSTER posts AT 'manticore-1:9312';
  mysql> INSERT INTO posts:testrt(title,content,gid)  VALUES('hello','world',1);
  Query OK, 1 row affected (0.00 sec)

  MySQL [(none)]> exit
  Bye
  ```
* Если вы теперь вернетесь к первому экземпляру, вы увидите новую запись:
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

Рекомендуется переопределить лимиты Docker по умолчанию для экземпляра Manticore:

```bash
 --ulimit nofile=65536:65536
```

Для наилучшей производительности компоненты таблиц могут быть "заблокированы в памяти" (mlocked). Когда Manticore запущен под Docker, экземпляру требуются дополнительные привилегии для разрешения блокировки памяти. При запуске экземпляра необходимо добавить следующие опции:

```bash
  --cap-add=IPC_LOCK --ulimit memlock=-1:-1
```

### Настройка Manticore Search с Docker

Если вы хотите запустить Manticore с пользовательской конфигурацией, включающей определения таблиц, вам нужно смонтировать конфигурацию в экземпляр:

```bash
docker run --name manticore -v $(pwd)/manticore.conf:/etc/manticoresearch/manticore.conf -v $(pwd)/data/:/var/lib/manticore -p 127.0.0.1:9306:9306 -d manticoresearch/manticore
```

Учитывайте, что Manticore search внутри контейнера запускается под пользователем `manticore`. Операции с файлами таблиц (такие как создание или ротация plain-таблиц) также должны выполняться под `manticore`. В противном случае файлы будут созданы под `root`, и поисковый демон не будет иметь прав на их открытие. Например, вот как можно выполнить ротацию всех таблиц:

```bash
docker exec -it manticore gosu manticore indexer --all --rotate
```

Вы также можете задать отдельные настройки конфигурации `searchd` и `common`, используя переменные окружения Docker.

Настройки должны иметь префикс с именем их секции, например, для `mysql_version_string` переменная должна называться `searchd_mysql_version_string`:


```bash
docker run --name manticore  -p 127.0.0.1:9306:9306  -e searchd_mysql_version_string='5.5.0' -d manticoresearch/manticore
```

В случае директивы `listen`, новые интерфейсы прослушивания добавляются с помощью переменной Docker `searchd_listen` в дополнение к стандартным. Можно объявить несколько интерфейсов, разделенных точкой с запятой ("|"). Для прослушивания только на сетевом адресе можно использовать `$ip` (полученный внутри с помощью `hostname -i`) как псевдоним адреса.

Например, `-e searchd_listen='9316:http|9307:mysql|$ip:5443:mysql_vip'` добавит дополнительный SQL-интерфейс на порту 9307, VIP-слушатель SQL на порту 5443, работающий только на IP экземпляра, и HTTP-слушатель на порту 9316, в дополнение к стандартным на 9306 и 9308 соответственно.

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

#### Флаги запуска

Чтобы запустить Manticore с пользовательскими флагами запуска, укажите их в качестве аргументов при использовании docker run. Убедитесь, что вы не включаете команду `searchd` и включаете флаг `--nodetach`. Вот пример:
```bash
docker run --name manticore --rm manticoresearch/manticore:latest --replay-flags=ignore-trx-errors --nodetach
```

#### Запуск под непривилегированным пользователем
По умолчанию основной процесс Manticore `searchd` запускается под пользователем `manticore` внутри контейнера, но скрипт, который выполняется при запуске контейнера, запускается под вашим пользователем Docker по умолчанию, которым в большинстве случаев является `root`. Если это не то, что вам нужно, вы можете использовать `docker ... --user manticore` или `user: manticore` в yaml-файле docker compose, чтобы все запускалось под `manticore`. Читайте ниже о возможной проблеме с правами на том, с которой вы можете столкнуться, и как ее решить.

#### Создание обычных таблиц при запуске
Для создания обычных таблиц, указанных в вашем пользовательском конфигурационном файле, вы можете использовать переменную окружения `CREATE_PLAIN_TABLES=1`. Это выполнит `indexer --all` перед запуском Manticore. Это полезно, если вы не используете тома, и ваши таблицы легко воссоздать.
```bash
docker run -e CREATE_PLAIN_TABLES=1 --name manticore -v $(pwd)/manticore.conf:/etc/manticoresearch/manticore.conf -p 9306:9306 -p 9308:9308 -d manticoresearch/manticore
```

### Устранение неполадок

#### Проблемы с правами доступа к смонтированному тому

Если вы запускаете Manticore Search docker от имени не-root пользователя (используя `docker ... --user manticore` или `user: manticore` в docker compose yaml), вы можете столкнуться с проблемами прав доступа, например:
```bash
FATAL: directory /var/lib/manticore write error: failed to open /var/lib/manticore/tmp: Permission denied
```

Это может произойти потому, что пользователь, используемый для запуска процессов внутри контейнера, может не иметь прав на изменение директории, которую вы смонтировали в контейнер. Чтобы исправить это, вы можете применить `chown` или `chmod` к смонтированной директории. Если вы запускаете контейнер от имени пользователя `manticore`, вам нужно сделать:
```bash
chown -R 999:999 data
```

так как пользователь `manticore` имеет ID 999 внутри контейнера.

<!-- proofread -->

