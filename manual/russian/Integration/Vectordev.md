# Интеграция с Vector.dev

> ПРИМЕЧАНИЕ: Интеграция с Fluent Bit требует наличия [Manticore Buddy](../Installation/Manticore_Buddy.md). Если она не работает, убедитесь, что Buddy установлен.

[Vector от Datadog](https://vector.dev/) — это открытый конвейер данных для наблюдаемости, который может собирать, преобразовывать и маршрутизировать логи или метрики. Хотя Vector может агрегировать данные самостоятельно, его сочетание с Manticore предоставляет специализированный слой для хранения и поиска.

Пример ниже показывает, как перенаправлять `dpkg.log` Debian через Vector.dev и индексировать его в Manticore.

## Пример структуры лога

```
2023-05-31 10:42:55 status triggers-awaited ca-certificates-java:all 20190405ubuntu1.1
2023-05-31 10:42:55 trigproc libc-bin:amd64 2.31-0ubuntu9.9 <none>
2023-05-31 10:42:55 status half-configured libc-bin:amd64 2.31-0ubuntu9.9
2023-05-31 10:42:55 status installed libc-bin:amd64 2.31-0ubuntu9.9
2023-05-31 10:42:55 trigproc systemd:amd64 245.4-4ubuntu3.21 <none>
```

## Конфигурация Vector.dev

Создайте `vector.toml`, аналогичный следующему:

```toml
[sources.test_file]
type = "file"
include = [ "/var/log/dpkg.log" ]

[transforms.modify_test_file]
type = "remap"
inputs = [ "test_file" ]
source = """
.vec_timestamp = del(.timestamp)"""

[sinks.manticore]
type = "elasticsearch"
inputs = [ "modify_test_file" ]
endpoints = ["http://127.0.0.1:9308"]
bulk.index = "dpkg_log"
```

- `endpoints` указывает на HTTP-интерфейс Manticore (порт `9308` по умолчанию). Измените его, если ваш экземпляр слушает на другом порту.
- Преобразование `remap` перемещает поле `timestamp` по умолчанию Vector в `vec_timestamp`, потому что `timestamp` зарезервировано в Manticore.
- `bulk.index` определяет таблицу, которая будет создана автоматически, когда Vector начнёт отправлять данные.

Запустите Vector.dev с этой конфигурацией, и он будет отслеживать файл лога, преобразовывать каждое событие и перенаправлять его напрямую в Manticore.

## Запуск Vector.dev

Сохраните конфигурацию как `vector.toml`, затем запустите агент:

```
vector --config vector.toml
```

Если вы запускаете Vector.dev в Docker, подключите как файл конфигурации, так и каталог с логами, например:

```
docker run --rm -v /var/log/dpkg.log:/var/log/dpkg.log:ro \
  -v $(pwd)/vector.toml:/etc/vector/vector.toml:ro \
  timberio/vector:latest --config /etc/vector/vector.toml
```

## Результаты Vector.dev

Когда конвейер работает, Manticore автоматически создаёт таблицу `dpkg_log`. Её схема и пример документов выглядят так:

```
mysql> DESCRIBE dpkg_log;
+-----------------+---------+--------------------+
| Field           | Type    | Properties         |
+-----------------+---------+--------------------+
| id              | bigint  |                    |
| file            | text    | indexed stored     |
| host            | text    | indexed stored     |
| message         | text    | indexed stored     |
| source_type     | text    | indexed stored     |
| vec_timestamp   | text    | indexed stored     |
+-----------------+---------+--------------------+

mysql> SELECT * FROM dpkg_log LIMIT 3\G
*************************** 1. row ***************************
id: 7856533729353672195
file: /var/log/dpkg.log
host: logstash-787f68f6f-nhdd2
message: 2023-06-05 14:03:04 startup archives install
source_type: file
vec_timestamp: 2023-08-04T15:32:50.203091741Z
*************************** 2. row ***************************
id: 7856533729353672196
file: /var/log/dpkg.log
host: logstash-787f68f6f-nhdd2
message: 2023-06-05 14:03:04 install base-passwd:amd64 <none> 3.5.47
source_type: file
vec_timestamp: 2023-08-04T15:32:50.203808861Z
*************************** 3. row ***************************
id: 7856533729353672197
file: /var/log/dpkg.log
host: logstash-787f68f6f-nhdd2
message: 2023-06-05 14:03:04 status half-installed base-passwd:amd64 3.5.47
source_type: file
vec_timestamp: 2023-08-04T15:32:50.203814031Z
```

## Заключение

Использование Vector.dev с Manticore позволяет собирать логи практически из любого источника, обогащать или очищать их на лету и сохранять результаты в базе данных, готовой к поиску. Этот рабочий процесс сохраняет конвейеры наблюдаемости простыми, но при этом позволяет выполнять расширенные преобразования, когда это необходимо.
