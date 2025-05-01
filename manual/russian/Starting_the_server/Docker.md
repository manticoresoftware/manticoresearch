# Запуск и использование Manticore в Docker

Образ основан на [текущем релизе пакета Manticore](https://manticoresearch.com/install/).

[comment]: # (данные ниже должны соответствовать https://github.com/manticoresoftware/docker/blob/master/README.md)

Конфигурация по умолчанию включает в себя образец таблицы Real-Time и слушает на портах по умолчанию:

  * `9306` для соединений от клиента MySQL
  * `9308` для соединений через HTTP
  * `9312` для соединений через бинарный протокол (например, в случае, если вы запускаете кластер)

Образ поставляется с библиотеками для легкой индексации данных из MySQL, PostgreSQL, XML и CSV файлов.

## Как запустить образ Manticore Search в Docker

### Быстрое использование

Ниже приведен самый простой способ запустить Manticore в контейнере и войти в него через клиент mysql:

```bash
docker run --name manticore --rm -d manticoresearch/manticore && echo "Ожидание запуска Manticore docker. Рекомендуется сопоставить data_dir, чтобы ускорить запуск в следующий раз" && until docker logs manticore 2>&1 | grep -q "accepting connections"; do sleep 1; echo -n .; done && echo && docker exec -it manticore mysql && docker stop manticore
```

Обратите внимание, что после выхода из клиента MySQL контейнер Manticore будет остановлен и удален, что приведет к потере всех данных. Для получения информации об использовании Manticore в производственной среде, см. ниже.

Образ поставляется с образцом таблицы, которую можно загрузить следующим образом:

```sql
mysql> source /sandbox.sql
```

Кроме того, клиент mysql имеет несколько образцов запросов в своей истории, которые вы можете запустить на вышеуказанной таблице, просто используйте клавиши Вверх/Вниз в клиенте, чтобы увидеть и запустить их.

### Производственное использование


#### Порты и точки монтирования

Для постоянства данных папка `/var/lib/manticore/` должна быть смонтирована на локальное хранилище или другой желаемый хранилище.

Файл конфигурации внутри инстанса находится по адресу `/etc/manticoresearch/manticore.conf`. Для пользовательских настроек этот файл должен быть смонтирован на ваш собственный файл конфигурации.

Порты 9306/9308/9312 для SQL/HTTP/Бинарный, откройте их в зависимости от того, как вы собираетесь использовать Manticore. Например:

```bash
docker run --name manticore -v $(pwd)/data:/var/lib/manticore -p 127.0.0.1:9306:9306 -p 127.0.0.1:9308:9308 -d manticoresearch/manticore
```

или

```bash
docker run --name manticore -v $(pwd)/manticore.conf:/etc/manticoresearch/manticore.conf -v $(pwd)/data:/var/lib/manticore/ -p 127.0.0.1:9306:9306 -p 127.0.0.1:9308:9308 -d manticoresearch/manticore
```

Убедитесь, что вы удалили `127.0.0.1:`, если хотите, чтобы порты были доступны для внешних хостов.

#### Библиотека Manticore Columnar и Manticore Buddy

Образ Manticore Search в Docker поставляется с предустановленной [Manticore Columnar Library](https://github.com/manticoresoftware/columnar) и [Manticore Buddy](https://github.com/manticoresoftware/manticoresearch-buddy)

#### Docker-compose

Во многих случаях вам может понадобиться использовать Manticore в сочетании с другими образами, указанными в файле Docker Compose YAML. Ниже приведена минимальная рекомендуемая конфигурация для Manticore Search в файле docker-compose.yml:

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
#      - ./manticore.conf:/etc/manticoresearch/manticore.conf # раскомментируйте, если используете пользовательскую конфигурацию
```

Кроме использования открытых портов 9306 и 9308, вы можете войти в инстанс, выполнив `docker-compose exec manticore mysql`.

#### Протокол HTTP

Протокол HTTP открыт на порту 9308. Вы можете сопоставить порт локально и подключиться с помощью curl.:

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

По умолчанию сервер настроен на отправку своих логов в `/dev/stdout`, которые можно просмотреть с хоста с помощью:


```bash
docker logs manticore
```

Лог запросов можно перенаправить в лог Docker, передав переменную `QUERY_LOG_TO_STDOUT=true`.


#### Кластер с несколькими узлами с репликацией

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
* Запустите: `docker-compose up`
* Создайте кластер:
  ```sql
  $ docker-compose exec manticore-1 mysql

  mysql> CREATE TABLE testrt ( title text, content text, gid integer);

  mysql> CREATE CLUSTER posts;
  Query OK, 0 rows affected (0.24 sec)

  mysql> ALTER CLUSTER posts ADD testrt;
  Запрос выполнен, 0 строк затронуто (0.07 сек)

  MySQL [(none)]> exit
  Пока
  ```
* Присоединиться к кластеру на втором экземпляре
  ```sql
  $ docker-compose exec manticore-2 mysql

  mysql> JOIN CLUSTER posts AT 'manticore-1:9312';
  mysql> INSERT INTO posts:testrt(title,content,gid)  VALUES('hello','world',1);
  Запрос выполнен, 1 строка затронута (0.00 сек)

  MySQL [(none)]> exit
  Пока
  ```
* Если вы сейчас вернетесь к первому экземпляру, вы увидите новую запись:
  ```sql
  $ docker-compose exec manticore-1 mysql

  MySQL [(none)]> select * from testrt;
  +---------------------+------+-------+---------+
  | id                  | gid  | title | content |
  +---------------------+------+-------+---------+
  | 3891565839006040065 |    1 | hello | world   |
  +---------------------+------+-------+---------+
  1 строка в наборе (0.00 сек)

  MySQL [(none)]> exit
  Пока
  ```

### Блокировка памяти и ограничения

Рекомендуется переопределить стандартные ulimits Docker для экземпляра Manticore:

```bash
 --ulimit nofile=65536:65536
```

Для достижения наилучших показателей производительности компоненты таблицы могут быть "mlocked" в память. Когда Manticore запускается под Docker, экземпляру требуются дополнительные привилегии для разрешения блокировки памяти. Следующие параметры должны быть добавлены при запуске экземпляра:

```bash
  --cap-add=IPC_LOCK --ulimit memlock=-1:-1
```

### Настройка Manticore Search с Docker

Если вы хотите запустить Manticore с пользовательской конфигурацией, которая включает определения таблиц, вам необходимо смонтировать конфигурацию в экземпляр:

```bash
docker run --name manticore -v $(pwd)/manticore.conf:/etc/manticoresearch/manticore.conf -v $(pwd)/data/:/var/lib/manticore -p 127.0.0.1:9306:9306 -d manticoresearch/manticore
```

Имейте в виду, что Manticore search внутри контейнера выполняется от имени пользователя `manticore`. Операции с файлами таблиц (например, создание или ротация обычных таблиц) также следует выполнять от имени `manticore`. В противном случае файлы будут созданы под `root`, и демон поиска не будет иметь прав для их открытия. Например, вот как вы можете ротировать все таблицы:

```bash
docker exec -it manticore gosu manticore indexer --all --rotate
```

Вы также можете установить индивидуальные настройки конфигурации `searchd` и `common`, используя переменные окружения Docker.

Настройки должны иметь префикс с именем их раздела, например, в случае `mysql_version_string` переменная должна называться `searchd_mysql_version_string`:


```bash
docker run --name manticore  -p 127.0.0.1:9306:9306  -e searchd_mysql_version_string='5.5.0' -d manticoresearch/manticore
```

В случае директивы `listen` новые интерфейсы могут быть добавлены, используя переменную Docker `searchd_listen`, в дополнение к стандартным. Можно объявить несколько интерфейсов, разделив их точкой с запятой ("|"). Чтобы слушать только по сетевому адресу, можно использовать `$ip` (внутри получаемый из `hostname -i`) в качестве псевдонима адреса.

Например, `-e searchd_listen='9316:http|9307:mysql|$ip:5443:mysql_vip'` добавит дополнительный SQL интерфейс на порту 9307, SQL VIP слушатель на порту 5443, работающий только на IP экземпляра, и HTTP слушатель на порту 9316, в дополнение к стандартам на 9306 и 9308 соответственно.

```bash
$ docker run --rm -p 1188:9307  -e searchd_mysql_version_string='5.5.0' -e searchd_listen='9316:http|9307:mysql|$ip:5443:mysql_vip'  manticore
[Пн 17 Авг 07:31:58.719 2020] [1] используется файл конфигурации '/etc/manticoresearch/manticore.conf' (9130 символов)...
слушает на всех интерфейсах для http, порт=9316
слушает на всех интерфейсах для mysql, порт=9307
слушает на 172.17.0.17:5443 для VIP mysql
слушает на всех интерфейсах для mysql, порт=9306
слушает на UNIX сокете /var/run/mysqld/mysqld.sock
слушает на 172.17.0.17:9312 для sphinx
слушает на всех интерфейсах для http, порт=9308
предварительная загрузка 0 индексов
предварительно загружено 0 индексов за 0.000 сек
принимаем подключения
```

#### Флаги запуска

Чтобы запустить Manticore с пользовательскими флагами запуска, укажите их как аргументы при использовании docker run. Убедитесь, что вы не включаете команду `searchd` и добавляете флаг `--nodetach`. Вот пример:
```bash
docker run --name manticore --rm manticoresearch/manticore:latest --replay-flags=ignore-trx-errors --nodetach
```

#### Запуск под непривилегированным пользователем
По умолчанию основной процесс Manticore `searchd` выполняется от имени пользователя `manticore` внутри контейнера, но скрипт, который запускается при старте контейнера, выполняется от вашего пользовательского Docker, который в большинстве случаев является `root`. Если вы этого не хотите, вы можете использовать `docker ... --user manticore` или `user: manticore` в файле docker compose yaml, чтобы все выполнялось под `manticore`. Читайте ниже о возможных проблемах с правами доступа к томам, которые вы можете получить, и как их решить.

#### Создание обычных таблиц при запуске
Чтобы создать обычные таблицы, указанные в вашем файле пользовательской конфигурации, вы можете использовать переменную окружения `CREATE_PLAIN_TABLES=1`. Это выполнит `indexer --all` перед запуском Manticore. Это полезно, если вы не используете тома, и ваши таблицы легко воспроизводимы.
```bash
docker run -e CREATE_PLAIN_TABLES=1 --name manticore -v $(pwd)/manticore.conf:/etc/manticoresearch/manticore.conf -p 9306:9306 -p 9308:9308 -d manticoresearch/manticore
```

### Устранение неполадок

#### Проблема с правами доступа к смонтированному тому

В случае, если вы запускаете Manticore Search docker под непривилегированным пользователем (используя `docker ... --user manticore` или `user: manticore` в файле docker compose yaml), вы можете столкнуться с проблемой прав доступа, например:
```bash
FATAL: directory /var/lib/manticore write error: failed to open /var/lib/manticore/tmp: Permission denied
```
Это может произойти, потому что пользователь, который используется для запуска процессов внутри контейнера, может не иметь разрешений на изменение директории, которую вы смонтировали в контейнер. Чтобы это исправить, вы можете использовать `chown` или `chmod` для смонтированной директории. Если вы запускаете контейнер под пользователем `manticore`, вам нужно сделать:
```bash
chown -R 999:999 data
```

поскольку пользователь `manticore` имеет ID 999 внутри контейнера.

<!-- proofread -->

# Запуск и использование Manticore в Docker

Образ основан на [текущем релизе пакета Manticore](https://manticoresearch.com/install/).

[comment]: # (the below should be in sync with https://github.com/manticoresoftware/docker/blob/master/README.md)

Конфигурация по умолчанию включает в себя пример таблицы в режиме реального времени и слушает на портах по умолчанию:

  * `9306` для подключений от клиента MySQL
  * `9308` для подключений по HTTP
  * `9312` для подключений через бинарный протокол (например, в случае, если вы запускаете кластер)

Образ поставляется с библиотеками для простого индексирования данных из MySQL, PostgreSQL, XML и CSV файлов.

# Как запустить образ Manticore Search Docker

## Быстрое использование

Ниже представлен самый простой способ запустить Manticore в контейнере и войти в него через клиент mysql:

```bash
docker run --name manticore --rm -d manticoresearch/manticore && echo "Ожидание запуска Manticore docker. Рассмотрите возможность маппинга data_dir, чтобы ускорить запуск в следующий раз" && until docker logs manticore 2>&1 | grep -q "accepting connections"; do sleep 1; echo -n .; done && echo && docker exec -it manticore mysql && docker stop manticore
```

Обратите внимание, что после выхода из клиента MySQL контейнер Manticore будет остановлен и удалён, что приведёт к отсутствию сохранённых данных. Для получения информации о использовании Manticore в производственной среде, пожалуйста, смотрите ниже.

Образ поставляется с примером таблицы, которую можно загрузить следующим образом:

```sql
mysql> source /sandbox.sql
```

Также в клиенте mysql есть несколько примерных запросов в его истории, которые вы можете выполнить для вышеуказанной таблицы, просто используйте клавиши Вверх/Вниз в клиенте, чтобы увидеть и выполнить их.

## Производственное использование


### Порты и точки монтирования

Для сохранения данных папка `/var/lib/manticore/` должна быть смонтирована на локальное хранилище или другой желаемый хранильный механизм.

Конфигурационный файл внутри инстанса находится по адресу `/etc/manticoresearch/manticore.conf`. Для пользовательских настроек этот файл должен быть смонтирован на ваш собственный конфигурационный файл.

Порты 9306/9308/9312 для SQL/HTTP/Бинарный протокол, откройте их в зависимости от того, как вы собираетесь использовать Manticore. Например:

```bash
docker run --name manticore -v $(pwd)/data:/var/lib/manticore -p 127.0.0.1:9306:9306 -p 127.0.0.1:9308:9308 -d manticoresearch/manticore
```

или

```bash
docker run --name manticore -v $(pwd)/manticore.conf:/etc/manticoresearch/manticore.conf -v $(pwd)/data:/var/lib/manticore/ -p 127.0.0.1:9306:9306 -p 127.0.0.1:9308:9308 -d manticoresearch/manticore
```

Убедитесь, что вы удалили `127.0.0.1:`, если хотите, чтобы порты были доступны для внешних хостов.

### Библиотека Manticore Columnar и Manticore Buddy

Образ Manticore Search Docker поставляется с предустановленной [Библиотекой Manticore Columnar](https://github.com/manticoresoftware/columnar) и [Manticore Buddy](https://github.com/manticoresoftware/manticoresearch-buddy)

### Docker-compose

В большинстве случаев вы можете захотеть использовать Manticore в сочетании с другими образами, указанными в файле YAML Docker Compose. Ниже представлена минимальная рекомендуемая конфигурация для Manticore Search в файле docker-compose.yml:

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
#      - ./manticore.conf:/etc/manticoresearch/manticore.conf # раскомментируйте, если вы используете пользовательский конфиг
```

Помимо использования открытых портов 9306 и 9308, вы можете войти в экземпляр, запустив `docker-compose exec manticore mysql`.

### Протокол HTTP

Протокол HTTP доступен на порту 9308. Вы можете сопоставить порт локально и подключиться, используя curl.:

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

### Ведение журнала

По умолчанию сервер настроен на отправку своей информации в журнал в `/dev/stdout`, который можно просмотреть из хоста, выполнив:


```bash
docker logs manticore
```

Журнал запросов можно перенаправить в журнал Docker, передав переменную `QUERY_LOG_TO_STDOUT=true`.


### Многоузловой кластер с репликацией

Вот простой `docker-compose.yml` для определения двух узлового кластера:

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
* Запустите: `docker-compose up`
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
* Присоединитесь к кластеру на втором экземпляре
  ```sql
  $ docker-compose exec manticore-2 mysql

  mysql> JOIN CLUSTER posts AT 'manticore-1:9312';
  mysql> INSERT INTO posts:testrt(title,content,gid)  VALUES('hello','world',1);
  Query OK, 1 row affected (0.00 sec)

  MySQL [(none)]> exit
  Bye
  ```
* Если вы сейчас вернетесь к первому экземпляру, вы увидите новую запись:
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

## Блокировка памяти и лимиты

Рекомендуется переписать стандартные ulimits Docker для экземпляра Manticore:

```bash
 --ulimit nofile=65536:65536
```

Для оптимальной производительности компоненты таблицы могут быть "mlocked" в память. Когда Manticore запускается под Docker, экземпляру требуются дополнительные привилегии для разрешения блокировки памяти. Следующие параметры должны быть добавлены при запуске экземпляра:

```bash
  --cap-add=IPC_LOCK --ulimit memlock=-1:-1
```

## Настройка Manticore Search с Docker

Если вы хотите запустить Manticore с пользовательской конфигурацией, которая включает определения таблиц, вам нужно смонтировать конфигурацию к экземпляру:

```bash
docker run --name manticore -v $(pwd)/manticore.conf:/etc/manticoresearch/manticore.conf -v $(pwd)/data/:/var/lib/manticore -p 127.0.0.1:9306:9306 -d manticoresearch/manticore
```

Имейте в виду, что поиск Manticore внутри контейнера запускается под пользователем `manticore`. Операции с файлами таблицы (такими как создание или ротация обычных таблиц) также должны выполняться под `manticore`. В противном случае файлы будут созданы под `root`, и демон поиска не сможет их открыть. Например, вот как вы можете ротировать все таблицы:

```bash
docker exec -it manticore gosu manticore indexer --all --rotate
```

Вы также можете установить индивидуальные настройки `searchd` и `common` с помощью переменных окружения Docker.

Настройки должны быть префиксированы именем их секции, например, для переменной `mysql_version_string` переменная должна называться `searchd_mysql_version_string`:


```bash
docker run --name manticore  -p 127.0.0.1:9306:9306  -e searchd_mysql_version_string='5.5.0' -d manticoresearch/manticore
```

В случае директивы `listen` добавляются новые интерфейсы для прослушивания с использованием переменной Docker `searchd_listen`, помимо стандартных. Можно объявить несколько интерфейсов, разделённых точкой с запятой ("|"). Чтобы прослушивать только на сетевом адресе, можно использовать `$ip` (внутренне полученный с помощью `hostname -i`) в качестве псевдонима адреса.

Например, `-e searchd_listen='9316:http|9307:mysql|$ip:5443:mysql_vip'` добавит дополнительный SQL интерфейс на порт 9307, SQL VIP слушатель на порт 5443, работающий только на IP экземпляра, и HTTP слушатель на порт 9316, в дополнение к стандартным на 9306 и 9308 соответственно.

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

### Флаги запуска

Для запуска Manticore с пользовательскими флагами запуска укажите их как аргументы при использовании docker run. Убедитесь, что вы не включаете команду `searchd` и добавляете флаг `--nodetach`. Вот пример:
```bash
docker run --name manticore --rm manticoresearch/manticore:latest --replay-flags=ignore-trx-errors --nodetach
```

### Запуск под ненаделенным пользователем
По умолчанию основной процесс Manticore `searchd` запускается под пользователем `manticore` внутри контейнера, но скрипт, который запускается при старте контейнера, выполняется под вашим стандартным пользователем Docker, который в большинстве случаев является `root`. Если вы этого не хотите, вы можете использовать `docker ... --user manticore` или `user: manticore` в docker compose yaml, чтобы всё работало под `manticore`. Читайте ниже о возможных проблемах с правами на томах и о том, как их решить.

### Создание обычных таблиц при запуске
Чтобы создать обычные таблицы, заданные в вашем пользовательском файле конфигурации, вы можете использовать переменную окружения `CREATE_PLAIN_TABLES=1`. Это выполнит `indexer --all` перед запуском Manticore. Это полезно, если вы не используете тома, и ваши таблицы легко восстановить.
```bash
docker run -e CREATE_PLAIN_TABLES=1 --name manticore -v $(pwd)/manticore.conf:/etc/manticoresearch/manticore.conf -p 9306:9306 -p 9308:9308 -d manticoresearch/manticore
```

## Устранение неполадок

### Проблема с правами доступа к смонтированному тому

В случае, если вы запускаете Manticore Search Docker под ненаделенным пользователем (используя `docker ... --user manticore` или `user: manticore` в docker compose yaml), вы можете столкнуться с проблемой прав доступа, например:
```bash
FATAL: directory /var/lib/manticore write error: failed to open /var/lib/manticore/tmp: Permission denied
```
Это может произойти, потому что пользователь, под которым выполняются процессы внутри контейнера, может не иметь разрешений на изменение каталога, который вы смонтировали в контейнер. Чтобы это исправить, вы можете использовать `chown` или `chmod` для смонтированного каталога. Если вы запускаете контейнер от имени пользователя `manticore`, вам нужно сделать следующее:
```bash
chown -R 999:999 data
```

поскольку пользователь `manticore` имеет ID 999 внутри контейнера.

<!-- proofread -->


