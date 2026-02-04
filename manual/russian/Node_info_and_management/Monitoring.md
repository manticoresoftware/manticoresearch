# Мониторинг

Manticore предоставляет эндпоинт метрик Prometheus, а также официально поддерживаемые правила оповещений и дашборды Grafana для Manticore Buddy.

## Prometheus Exporter

<!-- example prometheus exporter -->

Manticore Search имеет встроенный экспортер Prometheus.
Чтобы запросить метрики, убедитесь, что HTTP-порт открыт, и просто вызовите эндпоинт /metrics.

**Примечание:** Для работы экспортера требуется, чтобы **Buddy** был включен.

<!-- intro -->
##### HTTP:

<!-- request http -->

```go
curl -s 0:9308/metrics
```
<!-- response http -->

```http
# HELP manticore_uptime_seconds Time in seconds since start
# TYPE manticore_uptime_seconds counter
manticore_uptime_seconds 25
# HELP manticore_connections_count Connections count since start
# TYPE manticore_connections_count gauge
manticore_connections_count 55
# HELP manticore_maxed_out_error_count Count of maxed_out errors since start
# TYPE manticore_maxed_out_error_count counter
manticore_maxed_out_error_count 0
# HELP manticore_version Manticore Search version
# TYPE manticore_version gauge
manticore_version {version="0.0.0 c88e811b2@25060409 (columnar 5.0.1 59c7092@25060304) (secondary 5.0.1 59c7092@25060304) (knn 5.0.1 59c7092@25060304) (embeddings 1.0.0) (buddy v3.28.6-7-g14ee10)"} 1
# HELP manticore_mysql_version Manticore Search version
# TYPE manticore_mysql_version gauge
manticore_mysql_version {version="0.0.0 c88e811b2@25060409 (columnar 5.0.1 59c7092@25060304) (secondary 5.0.1 59c7092@25060304) (knn 5.0.1 59c7092@25060304) (embeddings 1.0.0)"} 1
# HELP manticore_command_search_count Count of search queries since start
# TYPE manticore_command_search_count counter
manticore_command_search_count 1
......
```

<!-- end -->

## Правила оповещений Prometheus

Эта папка содержит официально поддерживаемые ресурсы мониторинга для Manticore Search.

Файл: [manticore-alerts.yml](https://github.com/manticoresoftware/manticoresearch-buddy/blob/main/monitoring/prometheus/manticore-alerts.yml)

Пример конфигурации сбора метрик (scrape config):

```yaml
scrape_configs:
  - job_name: manticoresearch
    metrics_path: /metrics
    static_configs:
      - targets:
          - 127.0.0.1:9308
```

#### Руководство по оповещениям:
* **ManticoreBuddyTargetDown**: Prometheus не может собрать метрики с этой цели. Означает, что сервис не работает, недоступен или конфигурация сбора неверна.
* **ManticoreBuddyRecentlyRestarted**: Время работы (uptime) остается менее 5 минут в течение 5 минут. Означает, что процесс перезапускается или нестабилен.
* **ManticoreBuddyMaxedOutErrors**: Счетчик ошибок `maxed_out` увеличивается. Означает, что Manticore отклоняет задачи из-за лимитов.
* **ManticoreBuddySearchLatencyP95High**: 95-й процентиль задержки поиска остается выше 500 мс в течение 10 минут. Большинство пользователей ощущают медленный поиск.
* **ManticoreBuddySearchLatencyP99High**: 99-й процентиль задержки поиска остается выше 1000 мс в течение 10 минут. Самые медленные запросы очень медленные.
* **ManticoreBuddyWorkQueueBacklog**: Длина очереди задач остается выше 100 в течение 5 минут. Запросы накапливаются.
* **ManticoreBuddyWorkersSaturated**: Более 90% рабочих потоков активны в течение 10 минут. Потоки загружены по максимуму, поэтому задержка будет расти.
* **ManticoreBuddyQueryCacheNearLimit**: Использование кэша запросов остается выше 90% в течение 10 минут. Вероятно, происходит вытеснение из кэша, ожидайте более медленных запросов.
* **ManticoreBuddyDiskMappedCacheLow**: Коэффициент отображения диска в кэш остается ниже 50% в течение 15 минут. Больше операций ввода-вывода на диск, более медленные запросы.
* **ManticoreBuddyDiskMappedCacheVeryLow**: Коэффициент отображения диска в кэш остается ниже 20% в течение 15 минут. Серьезная нагрузка на диск, высокий риск большой задержки.

#### Зачем нужны эти оповещения:
* Доступность и перезапуски помогают рано обнаружить серьезные сбои и нестабильность.
* Задержка и очередь обнаруживают перегрузку до появления ошибок.
* Кэш и коэффициент отображения диска являются ранними предупреждениями о замедлениях, связанных с вводом-выводом.
* Насыщение рабочих потоков и ошибки maxed_out указывают на достижение пределов производительности.

#### Если вы видите оповещение:
* **TargetDown**: проверьте состояние сервиса, сеть, конфигурацию сбора метрик и брандмауэр.
* **RecentlyRestarted**: проверьте логи, сбои, нехватку памяти (OOM) или перезапуски от оркестратора.
* **MaxedOutErrors**: проверьте лимиты ресурсов, количество рабочих потоков и параллелизм запросов.
* **Latency/Queue/Saturation**: уменьшите нагрузку, масштабируйте ресурсы или оптимизируйте запросы/индексы.
* **Cache alerts**: увеличьте размер кэша или уменьшите количество запросов, вызывающих вытеснение из кэша.
* **Disk mapped cache ratio alerts**: добавьте оперативной памяти, уменьшите рабочий набор данных или оптимизируйте таблицы.

## Дашборд Grafana

Файл: [manticore-buddy-dashboard.json](https://github.com/manticoresoftware/manticoresearch-buddy/blob/main/monitoring/grafana/manticore-dashboard.json)

#### Руководство по панелям дашборда:
* **Uptime**: Время с последнего перезапуска. Если этот показатель постоянно падает, сервис перезапускается или падает.
* **Version**: Точная сборка Manticore, которая работает. Если поведение изменилось после развертывания, проверьте это в первую очередь.
* **Current Connections**: Сколько клиентских подключений открыто прямо сейчас. Внезапный скачок = всплеск трафика; внезапное падение = сбой или проблемы на стороне клиента.
* **Active Workers**: Сколько рабочих потоков занято. Если этот показатель долгое время близок к общему количеству, сервер перегружен.
* **Load (All Queues)**: Средняя нагрузка очереди за 1/5/15 минут. Если все три линии растут, система не справляется.
* **Work Queue Length**: Сколько задач ожидают выполнения. Если этот показатель продолжает расти, запросы накапливаются, и задержка будет увеличиваться.
* **Commands per Second**: Сколько операций поиска/вставки/обновления и т.д. в секунду. Показывает, насколько высока нагрузка и какого она типа.
* **Search Latency (p95/p99)**: 95%/99% поисковых запросов выполняются быстрее этого значения. Если эти значения растут, пользователи ощущают медленный поиск, даже если средние показатели выглядят нормально.
* **Query Cache Usage**: Сколько кэша используется по сравнению с максимумом. Если использование постоянно растет до предела, кэш будет вытесняться, и запросы замедлятся.
* **Query Cache Hit Rate**: Сколько попаданий в кэш в секунду. Если этот показатель падает при сохранении высокой нагрузки, кэш не помогает.
* **Disk and RAM Bytes**: Размер проиндексированных данных на диске и в оперативной памяти. Быстрый рост означает большее давление на хранилище и обычно более медленные запросы.
* **Disk Mapped Cache Ratio by Table**: Какая часть каждой таблицы уже закэширована в памяти. Низкие коэффициенты означают больше операций чтения с диска и более медленные запросы.
* **Slowest Thread**: Сколько времени занимает самый медленный выполняемый запрос. Если этот показатель резко возрастает, возможно, есть зависшие или очень тяжелые запросы.
* **Connections by Type**: Разделяет подключения на текущие, buddy и vip. Полезно для понимания, какой протокол создает нагрузку.

#### Как импортировать:
1. Откройте Grafana и перейдите в Dashboards -> Import.
2. Загрузите JSON-файл.
3. При запросе выберите ваш источник данных Prometheus.
