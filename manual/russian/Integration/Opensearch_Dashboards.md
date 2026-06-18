# Интеграция Manticore с OpenSearch Dashboards

[OpenSearch Dashboards](https://opensearch.org/platform/opensearch-dashboards/) — это интерфейс аналитики и визуализации с открытым исходным кодом для OpenSearch. Он вырос из той же кодовой базы эпохи Kibana (примерно Elasticsearch 7.10) и общается с кластером через HTTP API, совместимый с Elasticsearch. Manticore предоставляет API такого стиля на своем HTTP-листенере, поэтому вы можете направить OpenSearch Dashboards на Manticore и использовать **Discover**, **Visualize** и **Dashboards** аналогично [Kibana](Kibana.md).

Большая часть повседневного поведения, ограничений на типы полей и агрегации, а также пути приема данных повторяют то, что уже задокументировано для Kibana. Рассматривайте [руководство по интеграции Kibana](Kibana.md) как подробный справочник по поддерживаемым и неподдерживаемым функциям; эта страница посвящена тому, что специфично для OpenSearch Dashboards.

## Предварительные требования

1. **Установите OpenSearch Dashboards** с [страницы загрузок OpenSearch](https://opensearch.org/downloads.html) или из пакетов вашего дистрибутива. Выберите версию, ожидания клиента по умолчанию которой близки к линейке Elasticsearch 7.x; работа по совместимости Manticore описана в терминах этого семейства (на странице [Kibana](Kibana.md) указана **Kibana 7.6.0** как протестированная). Более новые выпуски OpenSearch Dashboards могут потребовать настройки (см. [Строка версии](#version-string) ниже).
2. **Убедитесь, что Manticore** запущен и HTTP API доступен (по умолчанию `http://localhost:9308`, если вы следуете приведенному ниже примеру).

## Конфигурация

1. Отредактируйте конфигурационный файл OpenSearch Dashboards (имя и путь зависят от установки; распространенные расположения — `config/opensearch_dashboards.yml` в раскладке tar-архива или `/etc/opensearch-dashboards/opensearch_dashboards.yml` в некоторых пакетах). См. официальное руководство: [Configuring OpenSearch Dashboards](https://docs.opensearch.org/latest/install-and-configure/configuring-dashboards/).
2. Установите URL бэкенда на ваш HTTP-эндпоинт Manticore, например:

   ```yaml
   opensearch.hosts: ["http://localhost:9308"]
   ```

   Используйте хост и порт, на которых `searchd` слушает HTTP. Если Manticore использует HTTPS, используйте `https://` и настройте параметры `opensearch.ssl.*`, как описано в документации OpenSearch.
3. Запустите OpenSearch Dashboards и откройте интерфейс (URL по умолчанию обычно `http://localhost:5601`). Замените `localhost` на имя хоста или IP-адрес вашего сервера, если это необходимо.

> **Примечания**
>
> - Manticore не требует аутентификации для этой интеграции, если вы сами ее не добавите; оставьте настройки аутентификации OpenSearch Dashboards не заданными, если только ваша среда в них не нуждается.
> - Manticore должен работать в [режиме реального времени](../Read_this_first.md#Real-time-mode-vs-plain-mode) для этого рабочего процесса.

### Пример конфигурации Manticore

```ini
searchd {
    listen = 127.0.0.1:9308:http
    pid_file = /var/run/manticore/searchd.pid
    data_dir = /var/lib/manticore
}
```

### Строка версии

Для данной версии Dashboards проверьте примечания по совместимости ниже, затем установите [`kibana_version_string`](../Server_settings/Searchd.md#kibana_version_string) в `searchd` в соответствии с ожидаемой версией сервера — а не произвольным числом. Если этого не сделать, вы можете увидеть предупреждения о неподдерживаемой версии или сбои в интерфейсе. Тот же параметр используется для Kibana и OpenSearch Dashboards.

## Использование интерфейса

### Discover

Используйте **Discover** для поиска и фильтрации документов. Поведение строки запроса зависит от вашей версии OpenSearch Dashboards. Неподдерживаемые функции запросов будут давать сбой так же, как и с Kibana против Manticore.

### Визуализации и дашборды

Создайте **index patterns** (или эквивалентные подключения данных в более новых выпусках), соответствующие вашим таблицам Manticore, затем стройте визуализации и собирайте **Dashboards**, как если бы вы работали с OpenSearch или Elasticsearch. Неподдерживаемые типы агрегаций и маппинги полей относятся к тому же классу ограничений, что и в руководстве по [Kibana](../Integration/Kibana.md).

### Управление

Используйте раздел **Management** в OpenSearch Dashboards для настроек стека (index patterns, сохраненные объекты, расширенные опции). Точные названия пунктов меню зависят от версии; следуйте документации upstream для вашей установки.

## Ограничения

- Неподдерживаемые типы полей Elasticsearch, ограничения агрегаций и UI-приложения, описанные для [Kibana](Kibana.md), в целом применимы и к OpenSearch Dashboards: Manticore реализует подмножество HTTP API, а не полную поверхность Elastic или OpenSearch Server.

- В настоящее время протестирована и рекомендуется OpenSearch Dashboards **3.4.0**. Другие версии Dashboards могут работать, но могут вызывать проблемы. Непротестированные мажорные версии Dashboards не поддерживаются; если вы все же попробуете одну из них, вы должны согласовать [`kibana_version_string`](../Server_settings/Searchd.md#kibana_version_string) с тем, что ожидает этот выпуск. ([Строка версии](../Integration/Opensearch_Dashboards.md#version-string)).

- Функции, требующие реального кластера OpenSearch (потоки плагина безопасности, оповещения на основе API OpenSearch, которые вы не отобразили в Manticore, и т.д.), выходят за рамки того, что предоставляет Manticore.

## Прием и исследование данных

Принимайте данные с помощью [Logstash](../Integration/Logstash.md), [Filebeat](../Integration/Filebeat.md), [Fluent Bit](../Integration/Fluentbit.md), [Vector.dev](../Integration/Vectordev.md) или других совместимых с Elasticsearch приемников, а затем исследуйте данные в OpenSearch Dashboards.
