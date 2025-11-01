# Интеграция с Filebeat

> ПРИМЕЧАНИЕ: Интеграция с Filebeat требует [Manticore Buddy](../Installation/Manticore_Buddy.md). Если она не работает, убедитесь, что Buddy установлен.

[Filebeat](https://www.elastic.co/beats/filebeat) — это легковесный агент для пересылки и централизации логов. После установки в качестве агента он отслеживает указанные вами файлы логов или расположения, собирает события логов и пересылает их для индексирования, обычно в Elasticsearch или Logstash.

Теперь Manticore также поддерживает использование Filebeat в качестве конвейеров обработки. Это позволяет отправлять собранные и преобразованные данные в Manticore так же, как в Elasticsearch. В настоящее время полностью поддерживаются все версии до 9.0.

## Конфигурация Filebeat

Конфигурация зависит от используемой версии Filebeat.

### Конфигурация для Filebeat 7.17, 8.0, 8.1

> **Важно**: В версиях Filebeat 7.17.0, 8.0.0 и 8.1.0 есть известная проблема с glibc 2.35+ (используется в Ubuntu 22.04 и более новых дистрибутивах). Эти версии могут завершиться сбоем с сообщением "Критическая ошибка glibc: регистрация rseq не удалась". Чтобы это исправить, добавьте конфигурацию `seccomp`, как показано ниже.

```yaml
```
filebeat.inputs:
- type: log
  enabled: true
  paths:
    - /var/log/dpkg.log
  close_eof: true
  scan_frequency: 1s

output.elasticsearch:
  hosts: ["http://localhost:9308"]
  index: "dpkg_log"
  allow_older_versions: true  # Требуется для 8.1

# Исправление для совместимости с glibc 2.35+ (Ubuntu 22.04+)
seccomp:
  default_action: allow
  syscalls:
    - action: allow
      names:
        - rseq
  compression_level: 0

setup.ilm.enabled: false
setup.template.enabled: false
setup.template.name: "dpkg_log"
setup.template.pattern: "dpkg_log"
```
**Ссылки**: [Проблема #30576](https://github.com/elastic/beats/issues/30576), [PR #30620](https://github.com/elastic/beats/pull/30620)



### Конфигурация для Filebeat 8.1 - 8.10

Для версий с 8.1 по 8.10 необходимо добавить опцию `allow_older_versions`:

```
filebeat.inputs:
- type: log
  enabled: true
  paths:
    - /var/log/dpkg.log
  close_eof: true
  scan_frequency: 1s

output.elasticsearch:
  hosts: ["http://localhost:9308"]
  index: "dpkg_log"
  compression_level: 0
  allow_older_versions: true

setup.ilm.enabled: false
setup.template.enabled: false
setup.template.name: "dpkg_log"
setup.template.pattern: "dpkg_log"
```

### Конфигурация для Filebeat 8.11 - 8.19

Начиная с версии 8.11, сжатие вывода включено по умолчанию, поэтому для совместимости с Manticore нужно явно указать `compression_level: 0`:

```
filebeat.inputs:
- type: log
  enabled: true
  paths:
    - /var/log/dpkg.log
  close_eof: true
  scan_frequency: 1s

output.elasticsearch:
  hosts: ["http://localhost:9308"]
  index: "dpkg_log"
  compression_level: 0
  allow_older_versions: true

setup.ilm.enabled: false
setup.template.enabled: false
setup.template.name: "dpkg_log"
setup.template.pattern: "dpkg_log"
```

### Конфигурация для Filebeat 9.0+

Filebeat 9.0 вводит значительные изменения в архитектуре, заменяя тип входных данных `log` на `filestream`. Начиная с версии 9.0, также изменился метод идентификации файлов по умолчанию на fingerprint, который требует, чтобы файлы были не менее 1024 байт ([см. issue #44780](https://github.com/elastic/beats/issues/44780)). Для совместимости Manticore с файлами любого размера необходимо отключить fingerprint.

Вот необходимая конфигурация для Filebeat 9.0 и всех последующих версий:

```
filebeat.inputs:
- type: filestream
  id: dpkg-log-input
  enabled: true
  paths:
    - /var/log/dpkg.log
  prospector.scanner.check_interval: 1s
  prospector.scanner.fingerprint.enabled: false

output.elasticsearch:
  hosts: ["http://localhost:9308"]
  index: "dpkg_log"
  compression_level: 0
  allow_older_versions: true

setup.ilm.enabled: false
setup.template.enabled: false
setup.template.name: "dpkg_log"
setup.template.pattern: "dpkg_log"
```

**Важные замечания для Filebeat 9.0+:**
- Вход `type: filestream` заменяет старый `type: log`
- Настройка `prospector.scanner.fingerprint.enabled: false` **обязательна** для отключения идентификации файлов по fingerprint, что обеспечивает корректную обработку файлов размером менее 1024 байт
- Поле `id` обязательно для входов filestream и должно быть уникальным

## Результаты Filebeat

После запуска Filebeat с этой конфигурацией данные логов будут отправлены в Manticore и корректно проиндексированы. Ниже приведена итоговая схема таблицы, созданной Manticore, и пример вставленного документа:

```
mysql> DESCRIBE dpkg_log;
+------------------+--------+--------------------+
| Field            | Type   | Properties         |
+------------------+--------+--------------------+
| id               | bigint |                    |
| @timestamp       | text   | indexed stored     |
| message          | text   | indexed stored     |
| log              | json   |                    |
| input            | json   |                    |
| ecs              | json   |                    |
| host             | json   |                    |
| agent            | json   |                    |
+------------------+--------+--------------------+
```

```
mysql> SELECT * FROM dpkg_log LIMIT 1\G
*************************** 1. row ***************************
id: 7280000849080753116
@timestamp: 2023-06-16T09:27:38.792Z
message: 2023-04-12 02:06:08 status half-installed libhogweed5:amd64 3.5.1+really3.5.1-2
input: {"type":"filestream"}
ecs: {"version":"1.6.0"}
host: {"name":"logstash-db848f65f-lnlf9"}
agent: {"ephemeral_id":"587c2ebc-e7e2-4e27-b772-19c611115996","id":"2e3d985b-3610-4b8b-aa3b-2e45804edd2c","name":"logstash-db848f65f-lnlf9","type":"filebeat","version":"7.10.0","hostname":"logstash-db848f65f-lnlf9"}
log: {"offset":80,"file":{"path":"/var/log/dpkg.log"}}
```
