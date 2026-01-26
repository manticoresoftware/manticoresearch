# Интеграция с Fluent Bit

> ПРИМЕЧАНИЕ: Интеграция с Fluent Bit требует [Manticore Buddy](../Installation/Manticore_Buddy.md). Если она не работает, убедитесь, что Buddy установлен.

[Fluent Bit](https://fluentbit.io/) — это кроссплатформенный процессор логов с открытым исходным кодом, который может агрегировать данные из множества источников и отправлять их в различные места назначения. Вы можете направлять вывод Fluent Bit напрямую в Manticore, чтобы собранные данные стали доступны для поиска в реальном времени.

Следующее пошаговое руководство индексирует `dpkg.log` Debian с помощью Fluent Bit и Manticore.

## Пример структуры лога

```
2023-05-31 10:42:55 status triggers-awaited ca-certificates-java:all 20190405ubuntu1.1
2023-05-31 10:42:55 trigproc libc-bin:amd64 2.31-0ubuntu9.9 <none>
2023-05-31 10:42:55 status half-configured libc-bin:amd64 2.31-0ubuntu9.9
2023-05-31 10:42:55 status installed libc-bin:amd64 2.31-0ubuntu9.9
2023-05-31 10:42:55 trigproc systemd:amd64 245.4-4ubuntu3.21 <none>
```

## Конфигурация Fluent Bit

Создайте файл конфигурации, например `fluent-bit.conf`:

```
[SERVICE]
    flush        1
    daemon       On
    log_level    info

[INPUT]
    name tail
    path /var/log/dpkg.log
    inotify_watcher false
    read_from_head true

[OUTPUT]
    name es
    match *
    host 127.0.0.1
    port 9308
    index dpkg_log
```

- Блок `[SERVICE]` запускает Fluent Bit в режиме демона, что удобно для Docker-сред. Отключите флаг демона, если хотите запустить его в интерактивном режиме.
- `inotify_watcher` отключен, чтобы избежать ограничений на уведомления о файлах внутри контейнеров.
- Плагин вывода (`name es`) может общаться с HTTP-эндпоинтом Manticore на порту `9308`.
- `index` определяет имя автоматически создаваемой таблицы в Manticore при поступлении первой партии данных.

Запустите Fluent Bit с этой конфигурацией, и он будет отслеживать `dpkg.log`, а затем пересылать каждую строку в Manticore.

## Запуск Fluent Bit

Сохраните конфигурацию как `fluent-bit.conf`, затем запустите Fluent Bit:

```
fluent-bit -c fluent-bit.conf
```

Чтобы запустить его в Docker, подключите файл лога (только для чтения) и конфигурацию:

```
docker run --rm -v /var/log/dpkg.log:/var/log/dpkg.log:ro \
  -v $(pwd)/fluent-bit.conf:/fluent-bit/etc/fluent-bit.conf:ro \
  fluent/fluent-bit:latest -c /fluent-bit/etc/fluent-bit.conf
```

## Результаты работы Fluent Bit

После начала приема данных Manticore автоматически создает таблицу `dpkg_log`. Вот ее определение и пример данных:

```
mysql> DESCRIBE dpkg_log;
+-------------+--------+----------------+
| Field       | Type   | Properties     |
+-------------+--------+----------------+
| id          | bigint |                |
| @timestamp  | text   | indexed stored |
| log         | text   | indexed stored |
+-------------+--------+----------------+

mysql> SELECT * FROM dpkg_log LIMIT 3\G
*************************** 1. row ***************************
id: 7856533729353662465
@timestamp: 2023-08-04T15:09:21.191Z
log: 2023-06-05 14:03:04 startup archives install
*************************** 2. row ***************************
id: 7856533729353662466
@timestamp: 2023-08-04T15:09:21.191Z
log: 2023-06-05 14:03:04 install base-passwd:amd64 <none> 3.5.47
*************************** 3. row ***************************
id: 7856533729353662467
@timestamp: 2023-08-04T15:09:21.191Z
log: 2023-06-05 14:03:04 status half-installed base-passwd:amd64 3.5.47
```

## Заключение

С помощью этого легковесного конвейера Fluent Bit обрабатывает сбор и доставку логов, а Manticore индексирует события для быстрого поиска и аналитики. Этот подход одинаково хорошо работает и с другими источниками логов — просто добавьте больше входных источников и повторно используйте тот же совместимый с Elasticsearch вывод, направленный на ваш кластер Manticore.
