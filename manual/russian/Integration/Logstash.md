# Интеграция с Logstash

> ПРИМЕЧАНИЕ: Интеграция с Logstash требует [Manticore Buddy](../Installation/Manticore_Buddy.md). Если это не работает, убедитесь, что Buddy установлен.

[Logstash](https://www.elastic.co/logstash) — это инструмент управления журналами, который собирает данные из различных источников, преобразует их на лету и отправляет в нужное место назначения. Он часто используется как конвейер данных для Elasticsearch, открытого аналитического и поискового движка.

Теперь Manticore поддерживает использование Logstash в качестве конвейера обработки. Это позволяет отправлять собранные и преобразованные данные в Manticore так же, как и в Elasticsearch. В настоящее время поддерживаются версии с 7.6 по 9.2.

Рассмотрим простой пример файла конфигурации Logstash, используемого для индексирования `dpkg.log`, стандартного файла журнала менеджера пакетов Debian. Сам журнал имеет простую структуру, как показано ниже:

```
2023-05-31 10:42:55 status triggers-awaited ca-certificates-java:all 20190405ubuntu1.1
2023-05-31 10:42:55 trigproc libc-bin:amd64 2.31-0ubuntu9.9 <none>
2023-05-31 10:42:55 status half-configured libc-bin:amd64 2.31-0ubuntu9.9
2023-05-31 10:42:55 status installed libc-bin:amd64 2.31-0ubuntu9.9
2023-05-31 10:42:55 trigproc systemd:amd64 245.4-4ubuntu3.21 <none>
```

## Конфигурация Logstash

Пример конфигурации Logstash:

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

Обратите внимание, прежде чем двигаться дальше, необходимо учесть важное предупреждение: Manticore не поддерживает управление шаблонами журналов (Log Template Management) и управление жизненным циклом индексов (Index Lifecycle Management) в Elasticsearch. Поскольку эти функции включены по умолчанию в Logstash, их необходимо явно отключить в конфигурации. Кроме того, опция hosts в секции output конфигурации должна соответствовать HTTP-порту прослушивания Manticore (по умолчанию localhost:9308).

## Конфигурация в зависимости от версии

Конфигурация варьируется в зависимости от версии Logstash, которую вы используете.

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

Запуск:
```bash
logstash -f logstash.conf
```

### Конфигурация для Logstash 8.0 - 9.1

Начиная с версии 8.0, ILM (управление жизненным циклом индекса) и управление шаблонами включены по умолчанию и должны быть явно отключены для совместимости с Manticore:

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

Для версий 9.0 и 9.1 Logstash должен запускаться с правами суперпользователя. Задайте переменную окружения перед запуском:

```bash
export ALLOW_SUPERUSER=1
logstash -f logstash.conf
```

### Конфигурация для Logstash 9.2+

Начиная с версии 9.2, рекомендуемый подход — настраивать параметр суперпользователя через файл конфигурации вместо использования переменных окружения. Это обеспечивает более постоянное и управляемое решение.

Файл конфигурации (например, `logstash.conf`):
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

Запуск:
```bash
logstash --path.settings=/etc/logstash -f logstash.conf
```

## Результаты Logstash

После настройки конфигурации, как описано выше, вы можете запустить Logstash, и данные из журнала dpkg будут передаваться в Manticore и правильно индексироваться.

Вот итоговая схема созданной таблицы и пример вставленного документа:

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

