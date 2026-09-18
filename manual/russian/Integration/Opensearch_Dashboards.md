# Интеграция Manticore с OpenSearch Dashboards

[OpenSearch Dashboards](https://opensearch.org/platform/opensearch-dashboards/) — это визуальный интерфейс, который позволяет исследовать, визуализировать и создавать дашборды для ваших лог-данных. Он подключается к backend через HTTP API. Manticore предоставляет этот API на своем HTTP-листенере, поэтому вы можете направить OpenSearch Dashboards на Manticore и использовать **Discover**, **Visualize** и **Dashboards** похожим образом, как в [Kibana](../Integration/Kibana.md). С этой интеграцией вы можете строить графики и дашборды, выполнять произвольные поиски в **Discover** и продолжать работать с привычными инструментами загрузки данных, такими как [Logstash](../Integration/Logstash.md) и [Filebeat](../Integration/Filebeat.md), чтобы загружать лог-данные и события для анализа.

## Требования
1. **Скачайте OpenSearch Dashboards**: убедитесь, что вы загружаете версию OpenSearch Dashboards, совместимую с Manticore. На данный момент протестирована и рекомендована версия **3.4.0**. Другие версии могут работать, но способны вызвать проблемы. Установите [`kibana_version_string`](../Server_settings/Searchd.md#kibana_version_string) в Manticore так, чтобы она соответствовала вашей версии OpenSearch Dashboards (см. ниже [Конфигурация](../Integration/Opensearch_Dashboards.md#Configuration)).
2. **Проверьте Manticore**: убедитесь, что ваш экземпляр Manticore запущен и его HTTP API доступен (по умолчанию: `http://localhost:9308`).
3. **Manticore Buddy**: убедитесь, что [Manticore Buddy](../Installation/Manticore_Buddy.md) установлен и запущен. Интеграция OpenSearch Dashboards реализована плагином **EmulateElastic** в Buddy, который эмулирует совместимый с Elasticsearch HTTP API, ожидаемый Dashboards (тот же путь выполнения, что и для [Kibana](../Integration/Kibana.md)). Buddy запускается автоматически вместе с `searchd`, если вы не отключили его через [`buddy_path`](../Server_settings/Searchd.md#buddy_path).

## Конфигурация
1. Откройте [файл конфигурации OpenSearch Dashboards](https://docs.opensearch.org/3.4/install-and-configure/configuring-dashboards/) (`opensearch_dashboards.yml`; типичные пути: `config/opensearch_dashboards.yml` в tarball-раскладке или `/etc/opensearch-dashboards/opensearch_dashboards.yml` в некоторых пакетах).
2. Укажите URL вашего экземпляра Manticore:
   ```yaml
   opensearch.hosts: ["http://localhost:9308"]
   ```
   При запуске OpenSearch Dashboards в Docker то же значение можно задать через переменную окружения `OPENSEARCH_HOSTS`.

   Поскольку Manticore не предоставляет плагин OpenSearch Security, вам также нужно отключить плагин security dashboards в OpenSearch Dashboards:
   
   - **Docker**: установите `DISABLE_SECURITY_DASHBOARDS_PLUGIN=true` в окружении контейнера.
   - **Установка из tarball**: остановите OpenSearch Dashboards, затем выполните: `./bin/opensearch-dashboards-plugin remove securityDashboards`. После этого снова запустите OpenSearch Dashboards.
   
   Подробности см. в разделе [Отключение и включение плагина Security](https://docs.opensearch.org/3.4/security/configuration/disable-enable-security/).
3. Запустите OpenSearch Dashboards и откройте его в браузере по адресу `http://localhost:5601`. При необходимости замените `localhost` на IP-адрес или имя хоста вашего сервера.

> Примечание: чтобы интеграция с OpenSearch Dashboards работала, Manticore должен работать в [режиме real-time](../Read_this_first.md#Real-time-mode-vs-plain-mode).

### Пример конфигурации Manticore
Установите `kibana_version_string` в ту же версию, что и ваша установка OpenSearch Dashboards. OpenSearch Dashboards проверяет версию backend, сообщаемую Manticore, и может показать предупреждения или не запуститься, если версии не совпадают.

```ini
searchd {
    listen = 127.0.0.1:9308:http
    pid_file = /var/run/manticore/searchd.pid
    data_dir = /var/lib/manticore
    kibana_version_string = 3.4.0
}
```

## Поддерживаемые возможности

### Discover
- Используйте вкладку **Discover**, чтобы интерактивно искать и фильтровать документы в таблицах Manticore.

### Визуализации
- Перейдите в **Visualizations**, чтобы создавать собственные визуализации:
  - Создайте **index pattern**, который соответствует имени таблицы Manticore.
  - Выберите тип визуализации (например, столбчатую диаграмму, линейный график или круговую диаграмму), основанный на поддерживаемых агрегациях: `terms`, `histogram`, `date_histogram`, `range`, `date_range` и метриках `max`, `min`, `sum`, `avg`.
  - Настройте визуализацию, выполните ее и изучите данные.
  - Сохраните визуализации для дальнейшего использования.

### Дашборды
- Откройте **Dashboards**, чтобы создавать или просматривать интерактивные дашборды:
  - Добавляйте сохраненные визуализации, фильтры или элементы управления для более удобной работы.
  - Взаимодействуйте с данными прямо с дашборда.
  - Сохраняйте дашборды для дальнейшего использования.

### Управление
- Используйте **Management > Dashboards Management** для index patterns и сохраненных объектов (визуализаций, дашбордов).
- Запрашиваемые на уровне стека данные, необходимые для запуска Dashboards, эмулируются (версия node, настройки кластера, объекты конфигурации, список индексов). Полное администрирование кластера OpenSearch в Manticore недоступно.

## Ограничения

- На данный момент протестирована и рекомендована версия OpenSearch Dashboards **3.4.0**. Другие версии могут работать, но способны вызвать проблемы. Вам нужно установить в Manticore [`kibana_version_string`](../Server_settings/Searchd.md#kibana_version_string), чтобы она совпадала с используемой вами версией OpenSearch Dashboards.
- Следующие [типы полей OpenSearch](https://docs.opensearch.org/3.4/mappings/supported-field-types/index/) не поддерживаются:
  - [Географические типы полей](https://docs.opensearch.org/3.4/mappings/supported-field-types/geographic/) (`geo_point`, `geo_shape`)
  - [Декартовы типы полей](https://docs.opensearch.org/3.4/mappings/supported-field-types/xy/) (`xy_point`, `xy_shape`)
  - [Типы полей диапазонов](https://docs.opensearch.org/3.4/mappings/supported-field-types/range/) (`integer_range`, `long_range`, `double_range`, `float_range`, `ip_range`, `date_range`)
  - [Специализированные типы поисковых полей](https://docs.opensearch.org/3.4/mappings/supported-field-types/specialized-search/) (`semantic`, `rank_feature`, `rank_features`, `percolator`, `star_tree`, `derived`)
  - [Векторные типы полей](https://docs.opensearch.org/3.4/mappings/supported-field-types/knn-vector/) (`knn_vector`, `sparse_vector`)
  - [Типы полей автодополнения](https://docs.opensearch.org/3.4/mappings/supported-field-types/autocomplete/) и [расширенные строковые типы](https://docs.opensearch.org/3.4/mappings/supported-field-types/string/) ([`completion`](https://docs.opensearch.org/3.4/mappings/supported-field-types/completion/), [`search_as_you_type`](https://docs.opensearch.org/3.4/mappings/supported-field-types/search-as-you-type/), [`match_only_text`](https://docs.opensearch.org/3.4/mappings/supported-field-types/match-only-text/) и похожие типы; обычные `text` и `keyword` поддерживаются)
  - [Реляционные типы полей](https://docs.opensearch.org/3.4/mappings/supported-field-types/index/#object-field-types) ([`nested`](https://docs.opensearch.org/3.4/mappings/supported-field-types/nested/), [`join`](https://docs.opensearch.org/3.4/mappings/supported-field-types/join/))
- Функции агрегации метрик ограничены [теми, которые поддерживает Manticore](../Searching/Grouping.md#Aggregation-functions).
- Вложенные агрегации (`aggs` внутри `aggs`) не поддерживаются.
- Расширенные возможности [Dashboards Query Language (DQL)](https://docs.opensearch.org/3.4/dashboards/dql/) (поиск по вложенным полям, регулярные выражения, нечеткий поиск, поиск по близости, усиление терминов и подобные) могут не работать с Manticore.
- Следующие инструменты и плагины OpenSearch Dashboards не поддерживаются:
  - [Геопространственные возможности](https://docs.opensearch.org/3.4/install-and-configure/plugins/) (карты, зависящие от географических типов полей)
  - [Observability](https://docs.opensearch.org/3.4/observing-your-data/) – приложения для мониторинга метрик инфраструктуры, изучения логов и анализа трассировок
  - [Alerting](https://docs.opensearch.org/3.4/observing-your-data/alerting/index/) – мониторы, триггеры и уведомления через OpenSearch API
  - [Anomaly Detection](https://docs.opensearch.org/3.4/observing-your-data/ad/index/) – детекторы выбросов во временных рядах
  - [Security Analytics](https://docs.opensearch.org/3.4/security-analytics/) – SIEM-процессы для обнаружения угроз и расследований
  - Мониторинг:
    - [Index State Management](https://docs.opensearch.org/3.4/im-plugin/ism/index/) – автоматические политики жизненного цикла индексов
    - [Performance Analyzer](https://docs.opensearch.org/3.4/monitoring-your-cluster/pa/index/) – метрики производительности кластера и анализ первопричин
  - [Index Management](https://docs.opensearch.org/3.4/dashboards/im-dashboards/index/) – интерфейс для управления индексами OpenSearch, шаблонами, алиасами и настройками жизненного цикла
  - Потоки [плагина Security](https://docs.opensearch.org/3.4/api-reference/security/index/) — недоступны в Manticore

## Загрузка и исследование данных
Интегрируйте Manticore с такими инструментами, как [Logstash](../Integration/Logstash.md), [Filebeat](../Integration/Filebeat.md), [Fluentbit](https://manticoresearch.com/blog/integration-of-manticore-with-fluentbit/) или [Vector.dev](https://manticoresearch.com/blog/integration-of-manticore-with-vectordev/), чтобы загружать данные из источников вроде веб-логов. После того как данные загружены в Manticore, вы можете исследовать и визуализировать их в OpenSearch Dashboards.
