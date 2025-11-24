# Интеграция с Logstash

> ПРИМЕЧАНИЕ: Интеграция с Logstash требует [Manticore Buddy](../Installation/Manticore_Buddy.md). Если она не работает, убедитесь, что Buddy установлен.

[Logstash](https://www.elastic.co/logstash) — это инструмент управления журналами, который собирает данные из разных источников, преобразует их на лету и отправляет в желаемое место назначения. Часто используется как конвейер данных для Elasticsearch, открытого движка аналитики и поиска.

Теперь Manticore поддерживает использование Logstash в качестве конвейера обработки. Это позволяет передавать собранные и преобразованные данные в Manticore так же, как в Elasticsearch. В настоящее время поддерживаются версии 7.6-9.2.

Рассмотрим простой пример файла конфигурации Logstash, используемого для индексирования `dpkg.log`, стандартного журнала менеджера пакетов Debian. Сам журнал имеет простую структуру, как показано ниже:

```
2023-05-31 10:42:55 status triggers-awaited ca-certificates-java:all 20190405ubuntu1.1
2023-05-31 10:42:55 trigproc libc-bin:amd64 2.31-0ubuntu9.9 <none>
2023-05-31 10:42:55 status half-configured libc-bin:amd64 2.31-0ubuntu9.9
2023-05-31 10:42:55 status installed libc-bin:amd64 2.31-0ubuntu9.9
2023-05-31 10:42:55 trigproc systemd:amd64 245.4-4ubuntu3.21 <none>
```

## Конфигурация Logstash

Вот пример конфигурации Logstash:

```
input {
  file {
    path => ["/var/log/dpkg.log"]
    start_position => "beginning"
    sincedb_path => "/dev/null"
    mode => "read"
    exit_after_read => "true"
   file_completed_action => "log"
   file_completed_log_path => "/dev/null"
  }
}

output {
  elasticsearch {
   index => " dpkg_log"
   hosts => ["http://localhost:9308"]
   ilm_enabled => false
   manage_template => false
  }
}
```

Обратите внимание, что перед дальнейшими действиями необходимо учитывать важный нюанс: Manticore не поддерживает управление шаблонами индексов и управление жизненным циклом индекса (ILM) в Elasticsearch. Поскольку эти функции по умолчанию включены в Logstash, их необходимо явно отключить в конфигурации. Кроме того, параметр hosts в разделе output конфигурации должен соответствовать HTTP-порту прослушивания Manticore (по умолчанию localhost:9308).

## Конфигурация для разных версий

Конфигурация зависит от используемой версии Logstash.

### Конфигурация для Logstash 7.17

Для Logstash 7.17 базовая конфигурация проста и не требует дополнительных настроек ILM:

```
input {
  file {
    path => ["/var/log/dpkg.log"]
    start_position => "beginning"
    sincedb_path => "/dev/null"
    mode => "read"
    exit_after_read => "true"
    file_completed_action => "log"
    file_completed_log_path => "/dev/null"
  }
}

output {
  elasticsearch {
    index => "dpkg_log"
    hosts => ["http://localhost:9308"]
  }
}
```

Запускать так:
```bash
logstash -f logstash.conf
```

### Конфигурация для Logstash 8.0 - 9.1

Начиная с версии 8.0, ILM и управление шаблонами включены по умолчанию и должны быть явно отключены для совместимости с Manticore:

```
input {
  file {
    path => ["/var/log/dpkg.log"]
    start_position => "beginning"
    sincedb_path => "/dev/null"
    mode => "read"
    exit_after_read => "true"
    file_completed_action => "log"
    file_completed_log_path => "/dev/null"
  }
}

output {
  elasticsearch {
    index => "dpkg_log"
    hosts => ["http://localhost:9308"]
    ilm_enabled => false
    manage_template => false
  }
}
```

Для версий 9.0 и 9.1 Logstash требует запуска от имени суперпользователя. Задайте переменную окружения перед запуском:

```bash
export ALLOW_SUPERUSER=1
logstash -f logstash.conf
```

### Конфигурация для Logstash 9.2+

Начиная с версии 9.2, рекомендуется настраивать параметр суперпользователя через конфигурационный файл вместо использования переменных окружения. Это обеспечивает более постоянное и управляемое решение.

Конфигурационный файл (например, `logstash.conf`):
```
input {
  file {
    path => ["/var/log/dpkg.log"]
    start_position => "beginning"
    sincedb_path => "/dev/null"
    mode => "read"
    exit_after_read => "true"
    file_completed_action => "log"
    file_completed_log_path => "/dev/null"
  }
}

output {
  elasticsearch {
    index => "dpkg_log"
    hosts => ["http://localhost:9308"]
    ilm_enabled => false
    manage_template => false
  }
}
```

Создайте `/etc/logstash/logstash.yml`:
```yaml
allow_superuser: true
```

Запускать так:
```bash
logstash --path.settings=/etc/logstash -f logstash.conf
```

## Результаты Logstash

После внесения описанных изменений в конфигурацию вы можете запустить Logstash, и данные из журнала dpkg будут переданы в Manticore и корректно проиндексированы.

Ниже приведена итоговая схема созданной таблицы и пример вставленного документа:

```
mysql> DESCRIBE dpkg_log;
+------------------+--------+---------------------+
| Field            | Type   | Properties          |
+------------------+--------+---------------------+
| id               | bigint |                     |
| message          | text   | indexed stored      |
| @version         | text   | indexed stored      |
| @timestamp       | text   | indexed stored      |
| path             | text   | indexed stored      |
| host             | text   | indexed stored      |
+------------------+--------+---------------------+
```

```
mysql> SELECT * FROM dpkg_log LIMIT 1\G

*************************** 1. row ***************************
id: 7280000849080746110
host: logstash-db848f65f-lnlf9
message: 2023-04-12 02:03:21 status unpacked libc-bin:amd64 2.31-0ubuntu9
path: /var/log/dpkg.log
@timestamp: 2023-06-16T09:23:57.405Z
@version: 1
```

