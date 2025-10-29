# Интеграция Manticore с Kibana

[Kibana](https://www.elastic.co/kibana) — это визуальный интерфейс, который позволяет исследовать, визуализировать и создавать панели мониторинга для ваших логов. Интеграция Kibana с Manticore Search может ускорить загрузку визуализаций Kibana до 3 раз по сравнению с Elasticsearch, как показано в этом [демо](https://github.com/manticoresoftware/kibana-demo#manticore-search-kibana-demo). Эта интеграция позволяет пользователям беспрепятственно анализировать свои данные с помощью интерактивных панелей, пользовательских визуализаций и возможностей поиска в реальном времени. Она также упрощает работу с разнообразными источниками данных, поддерживая инструменты такие как Logstash и Filebeat для упрощённого сбора данных, что делает её отличным выбором для рабочих процессов анализа логов.

## Предварительные требования
1. **Скачайте Kibana**: Убедитесь, что вы скачали версию Kibana, совместимую с Manticore. В настоящее время рекомендуется и протестирована версия 7.6.0. Другие версии 7.x могут работать, но могут возникнуть проблемы. Версии 8.x не поддерживаются.
2. **Проверьте Manticore**: Убедитесь, что ваш экземпляр Manticore запущен и его HTTP API доступен (по умолчанию: `http://localhost:9308`).

## Конфигурация
1. Откройте [файл конфигурации Kibana](https://www.elastic.co/guide/en/kibana/current/settings.html) (`kibana.yml`).
2. Укажите URL вашего экземпляра Manticore:
   ```yaml
   elasticsearch.hosts: ["http://localhost:9308"]
   ```
3. Запустите Kibana и откройте её в браузере по адресу `http://localhost:5601`. При необходимости замените `localhost` на IP адрес или имя вашего сервера.

> Примечание: Для работы с Kibana настройка аутентификации в Manticore не требуется. Также обратите внимание, что Manticore должен работать в [режиме реального времени](../Read_this_first#Real-time-mode-vs-plain-mode) для интеграции с Kibana.

### Пример конфигурации Manticore
```searchd {
    listen = 127.0.0.1:9308:http
    pid_file = /var/run/manticore/searchd.pid
    data_dir = /var/lib/manticore
 }

## Поддерживаемые функции
### Discover
- Используйте вкладку **Discover** в Kibana для интерактивного поиска и фильтрации данных.
- Улучшайте свои запросы с помощью строки запроса, используя простые запросы на [языке запросов Kibana](https://www.elastic.co/guide/en/kibana/current/kuery-query.html).

### Визуализации
- Перейдите в раздел **Visualizations** для создания пользовательских визуализаций:
  - Создайте таблицу (в Kibana это называется 'index pattern'), если она ещё не существует, чтобы определить источник данных.
  - Выберите тип визуализации (например, столбчатая диаграмма, линейный график или круговая диаграмма).
  - Настройте визуализацию, выполните её и изучайте данные.
  - Сохраняйте визуализации для последующего использования.

### Панели мониторинга
- Зайдите в **Dashboards** для создания или просмотра интерактивных панелей мониторинга:
  - Добавляйте визуализации, фильтры или элементы управления для персонализации.
  - Сохраняйте панели для повторного использования.

### Управление
- Перейдите в **Management > Kibana**, чтобы настроить параметры, такие как часовой пояс по умолчанию и предпочтения визуализации.

## Ограничения
- На данный момент рекомендована и протестирована версия Kibana 7.6.0. Другие версии 7.x могут работать, но могут возникнуть проблемы. Версии 8.x не поддерживаются.
- В настоящее время тестируется и рекомендуется версия Kibana 7.6.0. Другие версии 7.x могут работать, но могут 

вызвать проблемы. Версии 8.x не поддерживаются.
- Не поддерживаются следующие специфичные для Elasticsearch типы полей:
  - [Пространственные типы данных](https://www.elastic.co/guide/en/elasticsearch/reference/current/mapping-types.html#spatial_datatypes)
  - [Структурированные типы данных](https://www.elastic.co/guide/en/elasticsearch/reference/current/mapping-types.html#structured-data-types)
  - [Типы ранжирования документов](https://www.elastic.co/guide/en/elasticsearch/reference/current/mapping-types.html#document-ranking-types)
  - [Типы текстового поиска](https://www.elastic.co/guide/en/elasticsearch/reference/current/mapping-types.html#text-search-types) (за исключением простого 'text')
  - [Отношенческие типы данных](https://www.elastic.co/guide/en/elasticsearch/reference/current/mapping-types.html#object-types)
- Функции агрегации метрик ограничены [тем, что поддерживает Manticore](../Searching/Grouping.md#Aggregation-functions).
- Не поддерживаются следующие инструменты Kibana:
  - [Canvas](https://www.elastic.co/guide/en/kibana/7.6/canvas.html) – инструмент для визуализации и презентаций с использованием цветов и изображений.
  - [Elastic Maps](https://www.elastic.co/guide/en/kibana/7.6/maps.html) – инструмент для анализа географических данных.
  - [Metrics](https://www.elastic.co/guide/en/kibana/7.6/xpack-infra.html) – приложение для мониторинга метрик инфраструктуры.
  - [Logs](https://www.elastic.co/guide/en/kibana/7.6/xpack-logs.html) – консольный интерфейс для изучения логов распространённых сервисов.
  - Мониторинг:
    - [Uptime](https://www.elastic.co/guide/en/kibana/7.6/xpack-uptime.html) – мониторинг состояния сетевых узлов через HTTP/S, TCP и ICMP.
    - [APM (Application Performance Monitoring)](https://www.elastic.co/guide/en/kibana/7.6/xpack-apm.html) – сбор подробных метрик производительности приложений.
    - [SIEM (Security Information and Event Management)](https://www.elastic.co/guide/en/kibana/7.6/xpack-siem.html) – интерактивное рабочее пространство для команд безопасности для сортировки событий и проведения начальных расследований.
    - [ILM (Управление жизненным циклом индекса)](https://www.elastic.co/guide/en/elasticsearch/reference/7.6/index-lifecycle-management.html) - автоматическое управление индексами согласно требованиям к производительности, отказоустойчивости и хранению.
    - [Stack Monitoring](https://www.elastic.co/guide/en/kibana/7.6/xpack-monitoring.html) – предоставляет визуализации данных мониторинга Elastic Stack.
  - [Управление Elasticsearch](https://www.elastic.co/guide/en/kibana/7.6/management.html) – пользовательский интерфейс для управления объектами Elastic Stack, включая ILM (Управление жизненным циклом индекса) и др.

## Загрузка и исследование данных
Интегрируйте Manticore с инструментами такими как [Logstash](../Integration/Logstash.md), [Filebeat](../Integration/Filebeat.md), [Fluentbit](https://manticoresearch.com/blog/integration-of-manticore-with-fluentbit/) или [Vector.dev](https://manticoresearch.com/blog/integration-of-manticore-with-vectordev/), чтобы загружать данные из источников, таких как веб-логи. После загрузки данных в Manticore вы можете исследовать и визуализировать их в Kibana.

