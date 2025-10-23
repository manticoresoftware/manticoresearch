# Раздел "Searchd" в конфигурации

Ниже приведены настройки, которые используются в разделе `searchd` файла конфигурации Manticore Search для управления поведением сервера. Ниже приводится краткое описание каждой настройки:

### access_plain_attrs

Эта настройка устанавливает глобальные значения по умолчанию для [access_plain_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files). Она является необязательной, значение по умолчанию — `mmap_preread`.

Директива `access_plain_attrs` позволяет определить значение по умолчанию для [access_plain_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) для всех таблиц, управляемых этим экземпляром searchd. Директивы для отдельных таблиц имеют более высокий приоритет и переопределят это глобальное значение по умолчанию, обеспечивая более тонкий контроль.

### access_blob_attrs

Эта настройка устанавливает глобальные значения по умолчанию для [access_blob_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files). Она является необязательной, значение по умолчанию — `mmap_preread`.

Директива `access_blob_attrs` позволяет определить значение по умолчанию для [access_blob_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) для всех таблиц, управляемых этим экземпляром searchd. Директивы для отдельных таблиц имеют более высокий приоритет и переопределят это глобальное значение по умолчанию, обеспечивая более тонкий контроль.

### access_doclists

Эта настройка устанавливает глобальные значения по умолчанию для [access_doclists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files). Она является необязательной, значение по умолчанию — `file`.

Директива `access_doclists` позволяет определить значение по умолчанию для [access_doclists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) для всех таблиц, управляемых этим экземпляром searchd. Директивы для отдельных таблиц имеют более высокий приоритет и переопределят это глобальное значение по умолчанию, обеспечивая более тонкий контроль.

### access_hitlists

Эта настройка устанавливает глобальные значения по умолчанию для [access_hitlists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files). Она является необязательной, значение по умолчанию — `file`.

Директива `access_hitlists` позволяет определить значение по умолчанию для [access_hitlists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) для всех таблиц, управляемых этим экземпляром searchd. Директивы для отдельных таблиц имеют более высокий приоритет и переопределят это глобальное значение по умолчанию, обеспечивая более тонкий контроль.

### access_dict

Эта настройка устанавливает глобальные значения по умолчанию для [access_dict](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files). Она является необязательной, значение по умолчанию — `mmap_preread`.

Директива `access_dict` позволяет определить значение по умолчанию для [access_dict](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) для всех таблиц, управляемых этим экземпляром searchd. Директивы для отдельных таблиц имеют более высокий приоритет и переопределят это глобальное значение по умолчанию, обеспечивая более тонкий контроль.

### agent_connect_timeout

Эта настройка устанавливает глобальные значения по умолчанию для параметра [agent_connect_timeout](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_connect_timeout).


### agent_query_timeout

Эта настройка устанавливает глобальные значения по умолчанию для параметра [agent_query_timeout](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_query_timeout). Она может быть переопределена для каждого запроса с помощью условия `OPTION agent_query_timeout=XXX`.


### agent_retry_count

Эта настройка является целочисленным значением, определяющим, сколько раз Manticore попытается подключиться и выполнить запрос к удалённым агентам через распределённую таблицу, прежде чем отобразить фатальную ошибку запроса. Значение по умолчанию — 0 (т.е. без повторных попыток). Вы также можете задать это значение для каждого запроса с помощью условия `OPTION retry_count=XXX`. Если задана опция для конкретного запроса, она переопределит значение, указанное в конфигурации.

Обратите внимание, что если вы используете [зеркала агентов](../Creating_a_cluster/Remote_nodes/Mirroring.md#Agent-mirrors) в определении вашей распределённой таблицы, сервер будет выбирать другое зеркало для каждой попытки подключения в соответствии с выбранной стратегией [ha_strategy](../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_strategy). В этом случае значение `agent_retry_count` будет агрегироваться для всех зеркал в наборе.

Например, если у вас 10 зеркал и задано `agent_retry_count=5`, сервер попытается выполнить до 50 попыток, предполагая в среднем 5 попыток на каждое из 10 зеркал (с опцией `ha_strategy = roundrobin` это будет так).

Однако значение, заданное опцией `retry_count` для [агента](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent), служит абсолютным ограничением. Другими словами, опция `[retry_count=2]` в определении агента всегда означает максимум 2 попытки, независимо от того, указано ли у вас 1 или 10 зеркал для этого агента.

### agent_retry_delay

Эта настройка является целочисленным значением в миллисекундах (или с использованием [special_suffixes](../Server_settings/Special_suffixes.md)), определяющим задержку перед повторной попыткой запроса к удалённому агенту в случае отказа. Это значение актуально только при указанном ненулевом [agent_retry_count](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md) или ненулевом параметре `retry_count` для конкретного запроса. Значение по умолчанию — 500. Вы также можете задать это значение для каждого запроса с помощью условия `OPTION retry_delay=XXX`. Если задана опция для конкретного запроса, она переопределит значение, указанное в конфигурации.


### attr_flush_period

<!-- example conf attr_flush_period -->
При использовании [Update](../Data_creation_and_modification/Updating_documents/UPDATE.md) для изменения атрибутов документа в реальном времени, изменения сначала записываются во внутреннюю копию атрибутов в памяти. Эти обновления происходят в памяти, отображённой в файл, что означает, что ОС решает, когда записывать изменения на диск. При нормальном завершении работы `searchd` (инициируемом сигналом `SIGTERM`) все изменения принудительно записываются на диск.

Вы также можете указать `searchd` периодически записывать эти изменения на диск, чтобы предотвратить потерю данных. Интервал между этими сбросами определяется параметром `attr_flush_period`, указанным в секундах (или с помощью [special_suffixes](../Server_settings/Special_suffixes.md)).

По умолчанию значение равно 0, что отключает периодический сброс. Однако, сброс все равно происходит при нормальном завершении работы.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
attr_flush_period = 900 # persist updates to disk every 15 minutes
```
<!-- end -->

### auto_optimize

<!-- example conf auto_optimize -->
Этот параметр управляет автоматическим процессом [OPTIMIZE](../Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE) для сжатия таблиц.

По умолчанию сжатие таблиц происходит автоматически. Вы можете изменить это поведение с помощью параметра `auto_optimize`:
* 0 для отключения автоматического сжатия таблиц (вы всё ещё можете вызвать `OPTIMIZE` вручную)
* 1 для явного включения
* для включения с умножением порога оптимизации на 2.

По умолчанию OPTIMIZE выполняется, пока количество дисковых чанков меньше или равно количеству логических ядер процессора, умноженному на 2.

Однако, если в таблице есть атрибуты с индексами KNN, этот порог отличается. В этом случае он устанавливается как количество физических ядер процессора, делённое на 2, для улучшения производительности поиска KNN.

Обратите внимание, что переключение `auto_optimize` в положение вкл или выкл не мешает запуску [OPTIMIZE TABLE](../Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE) вручную.

<!-- intro -->
##### Пример:

<!-- request Disable -->
```ini
auto_optimize = 0 # disable automatic OPTIMIZE
```

<!-- request Throttle -->
```ini
auto_optimize = 2 # OPTIMIZE starts at 16 chunks (on 4 cpu cores server)
```

<!-- end -->

### auto_schema

<!-- example conf auto_schema -->
Manticore поддерживает автоматическое создание таблиц, которые ещё не существуют, но указаны в операторах INSERT. Эта функция включена по умолчанию. Чтобы отключить её, явно задайте `auto_schema = 0` в вашей конфигурации. Чтобы снова включить, задайте `auto_schema = 1` или удалите параметр `auto_schema` из конфигурации.

Имейте в виду, что HTTP-эндпоинт `/bulk` не поддерживает автоматическое создание таблиц.

> ПРИМЕЧАНИЕ: [функция авто-схемы](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema) требует [Manticore Buddy](../Installation/Manticore_Buddy.md). Если она не работает, убедитесь, что Buddy установлен.

<!-- request Disable -->
```ini
auto_schema = 0 # disable automatic table creation
```

<!-- request Enable -->
```ini
auto_schema = 1 # enable automatic table creation
```

<!-- end -->

### binlog_flush

<!-- example conf binlog_flush -->
Этот параметр управляет режимом сброса/синхронизации транзакций двоичного лога. Он необязательный, с значением по умолчанию 2 (сброс каждый транзакция, синхронизация каждую секунду).

Директива определяет, как часто двоичный лог будет сбрасываться в ОС и синхронизироваться с диском. Поддерживаются три режима:

*  0, сброс и синхронизация каждую секунду. Это обеспечивает лучшую производительность, но в случае сбоя сервера или сбоя ОС/оборудования можно потерять до одной секунды зафиксированных транзакций.
*  1, сброс и синхронизация каждой транзакции. Этот режим даёт наихудшую производительность, но гарантирует сохранность данных каждой зафиксированной транзакции.
*  2, сброс каждой транзакции, синхронизация каждую секунду. Этот режим обеспечивает хорошую производительность и гарантирует, что каждая зафиксированная транзакция сохранена при сбое сервера. Однако в случае сбоя ОС/оборудования можно потерять до одной секунды зафиксированных транзакций.

Для тех, кто знаком с MySQL и InnoDB, эта директива похожа на `innodb_flush_log_at_trx_commit`. В большинстве случаев режим 2 — это хороший баланс между скоростью и безопасностью, с полной защитой данных таблиц RT от сбоев сервера и частичной защитой от аппаратных сбоев.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
binlog_flush = 1 # ultimate safety, low speed
```
<!-- end -->

### binlog_common

<!-- example conf binlog_common -->
Этот параметр управляет способом управления файлами двоичного лога. Он необязательный, со значением по умолчанию 0 (отдельный файл для каждой таблицы).

Вы можете выбрать один из двух способов управления файлами двоичного лога:

* Отдельный файл для каждой таблицы (по умолчанию, `0`): Каждая таблица сохраняет свои изменения в собственном лог-файле. Эта настройка хороша, если у вас много таблиц, обновляемых в разное время. Она позволяет обновлять таблицы без ожидания других. Также, при проблемах с лог-файлом одной таблицы, это не влияет на остальные.
* Один файл для всех таблиц (`1`): Все таблицы используют один и тот же файл двоичного лога. Этот метод упрощает управление файлами, поскольку их меньше. Однако это может удерживать файлы дольше, чем необходимо, если одной таблице ещё нужно сохранить обновления. Эта настройка также может замедлить работу, если много таблиц нуждаются в обновлении одновременно, так как все изменения должны ждать записи в один файл.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
binlog_common = 1 # use a single binary log file for all tables
```
<!-- end -->

### binlog_max_log_size

<!-- example conf binlog_max_log_size -->
Этот параметр контролирует максимальный размер файла двоичного лога. Он необязательный, со значением по умолчанию 256 МБ.

Новый binlog файл будет принудительно открыт, как только текущий binlog достигнет этого размера. Это даёт более тонкую гранулярность логов и может повысить эффективность использования диска binlog при некоторых приграничных нагрузках. Значение 0 означает, что файл binlog не должен переоткрываться по размеру.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
binlog_max_log_size = 16M
```
<!-- end -->


### binlog_path

<!-- example conf binlog_path -->
Этот параметр определяет путь к файлам двоичного лога (также известного как журнал транзакций). Он необязательный, со значением по умолчанию каталога данных, заданного во время сборки (например, `/var/lib/manticore/data/binlog.*` в Linux).

Двоичные логи используются для восстановления данных таблицы RT после сбоев и для обновлений атрибутов простых дисковых индексов, которые в противном случае хранились бы только в ОЗУ до сброса. Когда ведение логов включено, каждая транзакция, подтверждённая в таблице RT, записывается в файл журнала. Логи затем автоматически проигрываются при запуске после некорректного завершения работы, восстанавливая зафиксированные изменения.

Директива `binlog_path` указывает расположение файлов двоичных логов. Она должна содержать только путь; `searchd` создаст и удалит несколько файлов `binlog.*` в каталоге по мере необходимости (включая данные binlog, метаданные и файлы блокировки и т.д.).

Пустое значение отключает двоичное логирование, что улучшает производительность, но ставит данные таблицы RT под угрозу.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
binlog_path = # disable logging
binlog_path = /var/lib/manticore/data # /var/lib/manticore/data/binlog.001 etc will be created
```
<!-- end -->

### boolean_simplify

<!-- example conf boolean_simplify -->
Этот параметр контролирует значение по умолчанию для опции поиска [boolean_simplify](../Searching/Options.md#boolean_simplify). Он необязателен, значение по умолчанию — 1 (включено).

Если задано 1, сервер автоматически применит [оптимизацию булевых запросов](../Searching/Full_text_matching/Boolean_optimization.md) для улучшения производительности запроса. Если 0 — запросы будут выполняться без оптимизации по умолчанию. Это значение по умолчанию можно переопределить для каждого запроса с помощью соответствующей опции поиска `boolean_simplify`.

<!-- request Example -->
```ini
searchd {
    boolean_simplify = 0  # disable boolean optimization by default
}
```
<!-- end -->

### buddy_path

<!-- example conf buddy_path -->
Этот параметр определяет путь к бинарному файлу Manticore Buddy. Он необязателен, по умолчанию содержит путь, установленный при сборке, который различается в разных операционных системах. Обычно менять эту настройку не нужно. Однако она может быть полезна, если вы хотите запустить Manticore Buddy в режиме отладки, внести изменения в Manticore Buddy или реализовать новый плагин. В последнем случае вы можете выполнить `git clone` Buddy с https://github.com/manticoresoftware/manticoresearch-buddy, добавить новый плагин в каталог `./plugins/` и запустить `composer install --prefer-source` для удобства разработки после перехода в каталог с исходниками Buddy.

Чтобы иметь возможность запускать `composer`, на вашей машине должна быть установлена PHP версии 8.2 или выше с следующими расширениями:

```
--enable-dom
--with-libxml
--enable-tokenizer
--enable-xml
--enable-xmlwriter
--enable-xmlreader
--enable-simplexml
--enable-phar
--enable-bcmath
--with-gmp
--enable-debug
--with-mysqli
--enable-mysqlnd
```

Вы также можете выбрать специальную версию `manticore-executor-dev` для Linux amd64, доступную в релизах, например: https://github.com/manticoresoftware/executor/releases/tag/v1.0.13

Если вы пойдёте этим путём, не забудьте связать dev-версию manticore executor с `/usr/bin/php`.

Чтобы отключить Manticore Buddy, установите значение пустым, как показано в примере.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
buddy_path = manticore-executor -n /usr/share/manticore/modules/manticore-buddy/src/main.php # use the default Manticore Buddy in Linux
buddy_path = manticore-executor -n /usr/share/manticore/modules/manticore-buddy/src/main.php --threads=1 # runs Buddy with a single worker
buddy_path = manticore-executor -n /opt/homebrew/share/manticore/modules/manticore-buddy/bin/manticore-buddy/src/main.php # use the default Manticore Buddy in MacOS arm64
buddy_path = manticore-executor -n /Users/username/manticoresearch-buddy/src/main.php # use Manticore Buddy from a non-default location
buddy_path = # disables Manticore Buddy
buddy_path = manticore-executor -n /Users/username/manticoresearch-buddy/src/main.php --skip=manticoresoftware/buddy-plugin-replace # --skip - skips plugins
buddy_path = manticore-executor -n /usr/share/manticore/modules/manticore-buddy/src/main.php --enable-plugin=manticoresoftware/buddy-plugin-show # runs Buddy with only the SHOW plugin
```
<!-- end -->

### client_timeout

<!-- example conf client_timeout -->
Этот параметр определяет максимальное время ожидания между запросами (в секундах или с использованием [special_suffixes](../Server_settings/Special_suffixes.md)) при использовании постоянных соединений. Он необязателен, значение по умолчанию — пять минут.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
client_timeout = 1h
```
<!-- end -->


### collation_libc_locale

<!-- example conf collation_libc_locale -->
Локаль libc сервера. Необязательна, по умолчанию — C.

Задает локаль libc, влияющую на сравнения на основе libc. Подробности смотрите в разделе [collations](../Searching/Collations.md).


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
collation_libc_locale = fr_FR
```
<!-- end -->


### collation_server

<!-- example conf collation_server -->
Основная локаль сравнения сервера. Необязательна, значение по умолчанию — libc_ci.

Определяет локаль сравнения по умолчанию для входящих запросов. Локаль сравнения можно переопределять для каждого запроса. Список доступных локалей и прочие детали см. в разделе [collations](../Searching/Collations.md).


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
collation_server = utf8_ci
```
<!-- end -->


### data_dir

<!-- example conf data_dir -->
При указании этот параметр включает [режим реального времени](../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29), который представляет собой императивный способ управления схемой данных. Значение должно быть путём к каталогу, где вы хотите хранить все ваши таблицы, двоичные логи и всё остальное, необходимое для корректной работы Manticore Search в этом режиме.
Индексация [простых таблиц](../Creating_a_table/Local_tables/Plain_table.md) не разрешена при указании `data_dir`. Подробнее о различиях между режимом RT и простым режимом читайте в [этой секции](../Read_this_first.md#Real-time-table-vs-plain-table).

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
data_dir = /var/lib/manticore
```
<!-- end -->

### diskchunk_flush_search_timeout

<!-- example conf diskchunk_flush_search_timeout -->
Таймаут для предотвращения автоматической выгрузки RAM-чунка, если в таблице нет поисков. Необязателен, по умолчанию 30 секунд.

Время для проверки поисков перед принятием решения об автоматической выгрузке.
Автоматическая выгрузка произойдёт только если в таблице было хотя бы одно поисковое обращение в течение последних `diskchunk_flush_search_timeout` секунд. Работает совместно с [diskchunk_flush_write_timeout](../Server_settings/Searchd.md#diskchunk_flush_write_timeout). Соответствующая [настройка на уровне таблицы](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#diskchunk_flush_search_timeout) имеет более высокий приоритет и переопределит это значение по умолчанию, обеспечивая более тонкий контроль.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
diskchunk_flush_search_timeout = 120s
```
<!-- end -->

### diskchunk_flush_write_timeout

<!-- example conf diskchunk_flush_write_timeout -->
Время в секундах ожидания без записи перед автоматической выгрузкой RAM-чунка на диск. Необязателен, по умолчанию 1 секунда.

If no write occurs in the RAM chunk within `diskchunk_flush_write_timeout` seconds, the chunk will be flushed to disk. Works in conjunction with [diskchunk_flush_search_timeout](../Server_settings/Searchd.md#diskchunk_flush_search_timeout). To disable auto-flush, set `diskchunk_flush_write_timeout = -1` explicitly in your configuration. The corresponding [per-table setting](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#diskchunk_flush_write_timeout) has a higher priority and will override this instance-wide default, providing more fine-grained control.

<!-- intro -->
##### Example:

<!-- request Example -->

```ini
diskchunk_flush_write_timeout = 60s
```
<!-- end -->

### docstore_cache_size

<!-- example conf docstore_cache_size -->
This setting specifies the maximum size of document blocks from document storage that are held in memory. It is optional, with a default value of 16m (16 megabytes).

When `stored_fields` is used, document blocks are read from disk and uncompressed. Since every block typically holds several documents, it may be reused when processing the next document. For this purpose, the block is held in a server-wide cache. The cache holds uncompressed blocks.


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
docstore_cache_size = 8m
```
<!-- end -->

### engine

<!-- example conf engine -->
Default attribute storage engine used when creating tables in RT mode. Can be `rowwise` (default) or `columnar`.

<!-- intro -->
##### Example:

<!-- request Example -->

```ini
engine = columnar
```
<!-- end -->


### expansion_limit

<!-- example conf expansion_limit -->
This setting determines the maximum number of expanded keywords for a single wildcard. It is optional, with a default value of 0 (no limit).

When performing substring searches against tables built with `dict = keywords` enabled, a single wildcard may potentially result in thousands or even millions of matched keywords (think of matching `a*` against the entire Oxford dictionary). This directive allows you to limit the impact of such expansions. Setting `expansion_limit = N` restricts expansions to no more than N of the most frequent matching keywords (per each wildcard in the query).

<!-- intro -->
##### Example:

<!-- request Example -->

```ini
expansion_limit = 16
```
<!-- end -->

### expansion_merge_threshold_docs

<!-- example conf expansion_merge_threshold_docs -->
This setting determines the maximum number of documents in the expanded keyword that allows merging all such keywords together. It is optional, with a default value of 32.

When performing substring searches against tables built with `dict = keywords` enabled, a single wildcard may potentially result in thousands or even millions of matched keywords. This directive allows you to increase the limit of how many keywords will merge together to speed up matching but uses more memory in the search.

<!-- intro -->
##### Example:

<!-- request Example -->

```ini
expansion_merge_threshold_docs = 1024
```
<!-- end -->

### expansion_merge_threshold_hits

<!-- example conf expansion_merge_threshold_hits -->
This setting determines the maximum number of hits in the expanded keyword that allows merging all such keywords together. It is optional, with a default value of 256.

When performing substring searches against tables built with `dict = keywords` enabled, a single wildcard may potentially result in thousands or even millions of matched keywords. This directive allows you to increase the limit of how many keywords will merge together to speed up matching but uses more memory in the search.

<!-- intro -->
##### Example:

<!-- request Example -->

```ini
expansion_merge_threshold_hits = 512
```
<!-- end -->

### expansion_phrase_limit

<!-- example conf expansion_phrase_limit -->
This setting controls the maximum number of alternative phrase variants generated due to `OR` operators inside `PHRASE`, `PROXIMITY`, and `QUORUM` operators. It is optional, with a default value of 1024.

При использовании оператора `|` (ИЛИ) внутри фраземного оператора общее количество развернутых комбинаций может расти экспоненциально в зависимости от количества указанных альтернатив. Этот параметр помогает предотвратить чрезмерное расширение запроса, ограничивая количество перестановок, учитываемых при обработке запроса.

Если количество сгенерированных вариантов превышает этот предел, запрос может:

- завершиться с ошибкой (поведение по умолчанию)
- вернуть частичные результаты с предупреждением, если включен `expansion_phrase_warning`

<!-- intro -->
##### Example:

<!-- request Example -->

```ini
expansion_phrase_limit = 4096
```
<!-- end -->

### expansion_phrase_warning

<!-- example conf expansion_phrase_warning -->
Этот параметр управляет поведением, когда предел расширения запроса, определенный `expansion_phrase_limit`, превышен.

По умолчанию запрос завершается с сообщением об ошибке. Когда для `expansion_phrase_warning` установлено значение 1, поиск продолжается с использованием частичного преобразования фразы (до установленного предела), и сервер возвращает пользователю предупреждающее сообщение вместе с набором результатов. Это позволяет запросам, которые слишком сложны для полного расширения, все же возвращать частичные результаты без полного прекращения.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
expansion_phrase_warning = 1
```
<!-- end -->

### grouping_in_utc

This setting specifies whether timed grouping in API and SQL will be calculated in the local timezone or in UTC. It is optional, with a default value of 0 (meaning 'local timezone').

By default, all 'group by time' expressions (like group by day, week, month, and year in API, also group by day, month, year, yearmonth, yearmonthday in SQL) are done using local time. For example, if you have documents with attributes timed `13:00 utc` and `15:00 utc`, in the case of grouping, they both will fall into facility groups according to your local timezone setting. If you live in `utc`, it will be one day, but if you live in `utc+10`, then these documents will be matched into different `group by day` facility groups (since 13:00 utc in UTC+10 timezone is 23:00 local time, but 15:00 is 01:00 of the next day). Sometimes such behavior is unacceptable, and it is desirable to make time grouping not dependent on timezone. You can run the server with a defined global TZ environment variable, but it will affect not only grouping but also timestamping in the logs, which may be undesirable as well. Switching 'on' this option (either in config or using [SET global](../Server_settings/Setting_variables_online.md#SET) statement in SQL) will cause all time grouping expressions to be calculated in UTC, leaving the rest of time-depentend functions (i.e. logging of the server) in local TZ.


### timezone

This setting specifies the timezone to be used by date/time-related functions. By default, the local timezone is used, but you can specify a different timezone in IANA format (e.g., `Europe/Amsterdam`).

Note that this setting has no impact on logging, which always operates in the local timezone.

Также обратите внимание, что если используется `grouping_in_utc`, функция 'group by time' по-прежнему будет использовать UTC, тогда как другие функции, связанные с датой/временем, будут использовать указанную временную зону. В целом не рекомендуется смешивать `grouping_in_utc` и `timezone`.

Эту опцию можно настроить либо в конфигурации, либо с помощью оператора [SET global](../Server_settings/Setting_variables_online.md#SET) в SQL.


### ha_period_karma

<!-- example conf ha_period_karma -->
Этот параметр задаёт размер окна статистики зеркал агента в секундах (или [special_suffixes](../Server_settings/Special_suffixes.md)). Опционально, значение по умолчанию — 60 секунд.

Для распределённой таблицы с зеркалами агента (подробнее в [agent](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md)) мастер отслеживает несколько разных счётчиков по каждому зеркалу. Эти счётчики затем используются для переключения на резервный узел и балансировки (мастер выбирает лучшее зеркало на основе счётчиков). Счётчики накапливаются блоками продолжительностью `ha_period_karma` секунд.

После начала нового блока мастер может использовать накопленные значения из предыдущего блока до тех пор, пока новый блок не будет заполнен на половину. В результате, влияние предыдущей истории на выбор зеркала прекращается не более чем через 1,5 раза ha_period_karma в секундах.

Хотя для выбора зеркала используются максимум два блока, для инструментирования хранится до 15 последних блоков. Эти блоки можно просмотреть с помощью оператора [SHOW AGENT STATUS](../Node_info_and_management/Node_status.md#SHOW-AGENT-STATUS).


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
ha_period_karma = 2m
```
<!-- end -->


### ha_ping_interval

<!-- example conf ha_ping_interval -->
Этот параметр задаёт интервал между ping-запросами агента к зеркалам в миллисекундах (или [special_suffixes](../Server_settings/Special_suffixes.md)). Опционально, значение по умолчанию — 1000 миллисекунд.

Для распределённой таблицы с зеркалами агента (подробнее в [agent](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md)) мастер отправляет всем зеркалам ping-команду в периоды простоя. Это необходимо для отслеживания текущего статуса агента (включён или отключён, время сетевого отклика и т. п.). Интервал между такими ping определяется этой директивой. Чтобы отключить пинги, установите ha_ping_interval в 0.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
ha_ping_interval = 3s
```
<!-- end -->


### hostname_lookup

Опция `hostname_lookup` определяет стратегию обновления имён хостов. По умолчанию IP-адреса хостов агентов кэшируются при запуске сервера, чтобы избежать чрезмерных обращений к DNS. Однако в некоторых случаях IP может динамически изменяться (например, в облачном хостинге), и может потребоваться не кэшировать IP-адреса. Установка опции в `request` отключает кэширование и выполняет запрос DNS для каждого запроса. IP-адреса также можно обновлять вручную с помощью команды `FLUSH HOSTNAMES`.

### jobs_queue_size

Параметр jobs_queue_size задаёт, сколько "заданий" может находиться в очереди одновременно. По умолчанию ограничение отсутствует.

В большинстве случаев "задание" означает один запрос к одной локальной таблице (обычной таблице или дисковому чанку таблицы реального времени). Например, если у вас распределённая таблица, состоящая из 2 локальных таблиц, или таблица реального времени с 2 дисковыми чанками, поисковый запрос к любой из них создаст обычно 2 задания в очереди. Затем эти задания обрабатывает пул потоков (размер которого задаётся параметром [threads](../Server_settings/Searchd.md#threads)). Однако в некоторых случаях, если запрос слишком сложный, может быть создано больше заданий. Изменение этого параметра рекомендуется, когда значения [max_connections](../Server_settings/Searchd.md#max_connections) и [threads](../Server_settings/Searchd.md#threads) недостаточны для достижения баланса между производительностью и нагрузкой.

### join_batch_size

Объединения таблиц работают путём накопления пакетного набора совпадений — результатов запроса, выполненного по левой таблице. Этот пакет затем обрабатывается как единый запрос по правой таблице.

Этот параметр позволяет настраивать размер пакета. Значение по умолчанию — `1000`; установка в `0` отключает пакетную обработку.

Больший размер пакета может улучшить производительность, однако для некоторых запросов это может привести к избыточному потреблению памяти.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
join_batch_size = 2000
```
<!-- end -->

### join_cache_size

Каждый запрос к правой таблице определяется конкретными условиями JOIN ON, которые определяют набор результатов, извлекаемых из правой таблицы.

Если уникальных условий JOIN ON немного, повторное использование результатов может быть эффективнее, чем многократное выполнение запросов к правой таблице. Для этого результаты сохраняются в кэше.

Этот параметр позволяет настроить размер этого кэша. Значение по умолчанию — `20 MB`; установка в 0 отключает кэширование.

Учтите, что каждый поток поддерживает собственный кэш, поэтому при оценке общего объёма потребляемой памяти следует учитывать количество потоков, выполняющих запросы.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
join_cache_size = 10M
```
<!-- end -->

### listen_backlog

<!-- example conf listen_backlog -->
Параметр listen_backlog определяет длину очереди TCP для входящих соединений. Это особенно актуально для сборок под Windows, которые обрабатывают запросы по одному. Когда очередь соединений достигает предела, новые входящие подключения будут отклоняться.
Для сборок под другие ОС значение по умолчанию обычно подходит, и настройка этого параметра, как правило, не требуется.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
listen_backlog = 20
```
<!-- end -->

### kibana_version_string

<!-- example conf kibana_version_string -->
Строка версии сервера, возвращаемая Kibana или OpenSearch Dashboards. Опционально — по умолчанию установлено `7.6.0`.

Некоторые версии Kibana и OpenSearch Dashboards ожидают, что сервер будет сообщать определённый номер версии и могут в зависимости от него вести себя по-разному. Чтобы обойти такие проблемы, можно использовать этот параметр, который заставляет Manticore сообщать Kibana или OpenSearch Dashboards кастомную версию.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
kibana_version_string = 1.2.3
```
<!-- end -->

### listen

<!-- example conf listen -->
Эта настройка позволяет указать IP-адрес и порт, либо путь к Unix-доменному сокету, на которых Manticore будет принимать подключения.

Общая синтаксическая форма для `listen`:

```ini
listen = ( address ":" port | port | path | address ":" port start - port end ) [ ":" protocol [ "_vip" ] [ "_readonly" ] ]
```

Вы можете указать:
* либо IP-адрес (или имя хоста) и номер порта
* либо только номер порта
* либо путь к Unix-сокету (не поддерживается в Windows)
* либо IP-адрес и диапазон портов

Если вы указываете номер порта, но не адрес, `searchd` будет слушать на всех сетевых интерфейсах. Unix путь определяется ведущим слэшем. Диапазон портов можно указать только для протокола репликации.

Вы также можете указать обработчик протокола (listener), который будет использоваться для подключений на этом сокете. Слушатели:

* **Не указан** - Manticore будет принимать подключения на этом порту от:
  - других агентов Manticore (например, удаленной распределенной таблицы)
  - клиентов через HTTP и HTTPS
  - [Manticore Buddy](https://manticoresearch.com/blog/manticoresearch-buddy-intro/). **Убедитесь, что у вас есть слушатель такого типа (или `http` слушатель, как указано ниже), чтобы избежать ограничений в функциональности Manticore.**
* `mysql` протокол MySQL для подключений от MySQL клиентов. Примечания:
  - Поддерживается также сжатый протокол.
  - Если включен [SSL](../Security/SSL.md#SSL), можно установить зашифрованное соединение.
* `replication` - протокол репликации, используемый для связи узлов. Подробнее см. в разделе [репликация](../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md). Вы можете указать несколько слушателей репликации, но они должны все слушать на одном IP; различаться могут только порты. Если вы определяете слушатель репликации с диапазоном портов (например, `listen = 192.168.0.1:9320-9328:replication`), Manticore не сразу начинает слушать на этих портах. Вместо этого он выберет случайные свободные порты из указанного диапазона только при использовании репликации. Для корректной работы репликации требуется минимум 2 порта в диапазоне.
* `http` - то же, что **Не указано**. Manticore будет принимать подключения на этом порту от удаленных агентов и клиентов через HTTP и HTTPS.
* `https` - протокол HTTPS. Manticore будет принимать **только** HTTPS подключения на этом порту. Подробнее см. раздел [SSL](../Security/SSL.md).
* `sphinx` - устаревший бинарный протокол. Используется для обслуживания подключений от удаленных клиентов [SphinxSE](../Extensions/SphinxSE.md). Некоторые реализации клиентов Sphinx API (например, Java) требуют явного объявления слушателя.

Добавление суффикса `_vip` к протоколам клиентов (то есть ко всем кроме `replication`, например `mysql_vip` или `http_vip` или просто `_vip`) заставляет создать отдельный поток для подключения, чтобы обойти различные ограничения. Это полезно для обслуживания узлов в случае сильной нагрузки, когда сервер иначе мог бы зависнуть или не позволить подключиться через обычный порт.

Суффикс `_readonly` устанавливает [режим только для чтения](../Security/Read_only.md) для слушателя и ограничивает его прием только запросами на чтение.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
listen = localhost
listen = localhost:5000 # listen for remote agents (binary API) and http/https requests on port 5000 at localhost
listen = 192.168.0.1:5000 # listen for remote agents (binary API) and http/https requests on port 5000 at 192.168.0.1
listen = /var/run/manticore/manticore.s # listen for binary API requests on unix socket
listen = /var/run/manticore/manticore.s:mysql # listen for mysql requests on unix socket
listen = 9312 # listen for remote agents (binary API) and http/https requests on port 9312 on any interface
listen = localhost:9306:mysql # listen for mysql requests on port 9306 at localhost
listen = localhost:9307:mysql_readonly # listen for mysql requests on port 9307 at localhost and accept only read queries
listen = 127.0.0.1:9308:http # listen for http requests as well as connections from remote agents (and binary API) on port 9308 at localhost
listen = 192.168.0.1:9320-9328:replication # listen for replication connections on ports 9320-9328 at 192.168.0.1
listen = 127.0.0.1:9443:https # listen for https requests (not http) on port 9443 at 127.0.0.1
listen = 127.0.0.1:9312:sphinx # listen for legacy Sphinx requests (e.g. from SphinxSE) on port 9312 at 127.0.0.1
```
<!-- end -->

Можно использовать несколько директив `listen`. `searchd` будет слушать клиентские подключения на всех указанных портах и сокетах. Стандартная конфигурация, поставляемая с пакетами Manticore, определяет прослушивание на портах:
* `9308` и `9312` — для подключений от удаленных агентов и клиентов, не основанных на MySQL
* и на порте `9306` для MySQL подключений.

Если в конфигурации не указано ни одного `listen`, Manticore будет ожидать подключения на:
* `127.0.0.1:9306` для клиентов MySQL
* `127.0.0.1:9312` для HTTP/HTTPS и подключений от других узлов Manticore и клиентов, использующих бинарный API Manticore.

#### Прослушивание привилегированных портов

По умолчанию в Linux нельзя заставить Manticore слушать порт ниже 1024 (например, `listen = 127.0.0.1:80:http` или `listen = 127.0.0.1:443:https`), если вы не запускаете searchd под root. Если вы хотите, чтобы Manticore запускался от не-root пользователя и слушал порты < 1024, рассмотрите один из следующих вариантов (любой из них должен работать):
* Выполните команду `setcap CAP_NET_BIND_SERVICE=+eip /usr/bin/searchd`
* Добавьте `AmbientCapabilities=CAP_NET_BIND_SERVICE` в systemd-юнит Manticore и перезагрузите демон (`systemctl daemon-reload`).

#### Технические подробности о протоколе Sphinx API и TFO
<details>
Устаревший протокол Sphinx состоит из 2 фаз: обмена рукопожатием и передачи данных. Рукопожатие состоит из 4-байтового пакета от клиента и 4-байтового пакета от демона, служащего единственной цели — клиент устанавливает, что удаленный процесс — настоящий демон Sphinx, демон устанавливает, что удаленный процесс — настоящий клиент Sphinx. Основной поток данных достаточно прост: обе стороны объявляют свои рукопожатия и проверяют их друг у друга. Такой обмен короткими пакетами предполагает использование специального флага `TCP_NODELAY`, который отключает алгоритм Нэйгла TCP и объявляет, что TCP-соединение будет проходить как диалог из маленьких пакетов.
Однако строго не определено, кто начинает переговоры первым. Исторически все клиенты, использующие бинарный API, говорят первыми: отправляют рукопожатие, затем читают 4 байта от демона, затем отправляют запрос и читают ответ от демона.
При улучшении совместимости протокола Sphinx мы учли следующее:

1. Как правило, связь мастер-агент устанавливается от известного клиента к известному хосту на известном порту. Поэтому маловероятно, что конечная точка даст неправильное рукопожатие. Следовательно, можно неявно считать, что обе стороны валидны и действительно говорят на протоколе Sphinx.
2. При таком предположении мы можем "склеить" рукопожатие и настоящий запрос и отправить их одним пакетом. Если бекенд — устаревший демон Sphinx, он просто прочитает этот склеенный пакет как 4 байта рукопожатия, затем тело запроса. Поскольку они пришли в одном пакете, у сокета бекенда нет задержки RTT, а буфер фронтенда работает, как обычно.
3. Продолжая предположение: поскольку пакет 'query' довольно мал, а handshake ещё меньше, давайте отправим оба в начальном TCP пакете 'SYN' с использованием современной техники TFO (tcp-fast-open). То есть: мы подключаемся к удалённой ноде с «склеенным» пакетом handshake + тело. Демон принимает соединение и сразу же имеет в буфере сокета как handshake, так и тело, так как они пришли в самом первом TCP пакете 'SYN'. Это исключает еще один RTT.
4. Наконец, научим демон принимать это улучшение. На самом деле, с точки зрения приложения, это подразумевает НЕ использовать `TCP_NODELAY`. А с точки зрения системы, это означает обеспечить, чтобы на стороне демона было включено принятие TFO, а на стороне клиента - отправка TFO. По умолчанию в современных системах клиентский TFO уже активирован, поэтому вам нужно только настроить TFO на сервере, чтобы всё заработало.

Все эти улучшения без фактического изменения протокола позволили нам исключить 1,5 RTT протокола TCP из соединения. Если запрос и ответ могут быть помещены в один TCP-пакет, это сокращает всю сессию бинарного API с 3,5 RTT до 2 RTT - что делает сетевой обмен примерно в 2 раза быстрее.

Итак, все наши улучшения строятся вокруг исходно неустановленного утверждения: «кто говорит первым». Если первым говорит клиент, мы можем применить все эти оптимизации и эффективно обработать connect + handshake + query в одном TFO-пакете. Более того, можно заглянуть в начало полученного пакета и определить реальный протокол. Вот почему можно подключаться к одному и тому же порту через API/http/https. Если первым должен говорить демон, все эти оптимизации невозможны, и мультипротокол тоже невозможен. Поэтому у нас есть выделенный порт для MySQL, и мы не объединили его со всеми другими протоколами в один порт. Внезапно, среди всех клиентов, один был написан с предположением, что демон должен отправлять handshake первым. Это - отсутствие возможности всех описанных улучшений. Это плагин SphinxSE для mysql/mariadb. Поэтому специально для этого единственного клиента мы выделили протокол `sphinx`, работающий наиболее наследуемым способом. А именно: обе стороны активируют `TCP_NODELAY` и обмениваются маленькими пакетами. Демон шлёт свой handshake при подключении, затем клиент отправляет свой, и затем всё работает обычным способом. Это не очень оптимально, но просто работает. Если вы используете SphinxSE для подключения к Manticore - вам нужно выделить listener с явно заданным протоколом `sphinx`. Для других клиентов - не используйте этот listener, так как он медленнее. Если вы используете другие устаревшие клиенты Sphinx API - сначала проверьте, могут ли они работать с неодназначенным мультипротокольным портом. Для связи master-agent использование неодназначенного (мультипротокольного) порта с включенным клиентским и серверным TFO работает хорошо и определённо ускорит работу сетевого бэкенда, особенно если у вас очень лёгкие и быстрые запросы.
</details>

### listen_tfo

Эта настройка позволяет использовать флаг TCP_FASTOPEN для всех слушателей. По умолчанию он управляется системой, но может быть явно отключен установкой в '0'.

Для общего понимания расширения TCP Fast Open обратитесь к [Wikipedia](https://en.wikipedia.org/wiki/TCP_Fast_Open). Вкратце, оно позволяет исключить один TCP круг (RTT) при установлении соединения.

На практике использование TFO во многих случаях может оптимизировать сетевую эффективность клиент-агент, как если бы использовались [постоянные агенты](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md), но без удержания активных соединений и без ограничения максимального количества соединений.

В современных ОС поддержка TFO обычно включена на системном уровне, но это лишь «возможность», а не правило. Linux (самая прогрессивная) поддерживает его с 2011 года, на ядрах начиная с 3.7 (для серверной стороны). Windows поддерживает с некоторых сборок Windows 10. Другие операционные системы (FreeBSD, MacOS) тоже включены в игру.

В Linux system сервер проверяет переменную `/proc/sys/net/ipv4/tcp_fastopen` и ведет себя согласно ей. Бит 0 управляет клиентской стороной, бит 1 - слушателями. По умолчанию параметр установлен в 1, то есть клиенты включены, слушатели выключены.

### log

<!-- example conf log -->
Параметр log указывает имя файла журнала, в который будут записываться все события `searchd` во время работы. Если не указан, используется имя по умолчанию 'searchd.log'.

В качестве альтернативы можно использовать в качестве имени файла 'syslog'. В этом случае события будут отправляться демону syslog. Для использования опции syslog нужно скомпилировать Manticore с опцией `-–with-syslog`.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
log = /var/log/searchd.log
```
<!-- end -->


### max_batch_queries

<!-- example conf max_batch_queries -->
Ограничивает количество запросов в партии. Опционально, по умолчанию 32.

Заставляет searchd выполнять проверку разумности количества запросов, отправленных в одной партии при использовании [мультизапросов](../Searching/Multi-queries.md). Установите в 0, чтобы пропустить проверку.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
max_batch_queries = 256
```
<!-- end -->

### max_connections

<!-- example max_connections -->
Максимальное количество одновременных клиентских соединений. По умолчанию неограниченно. Обычно заметно только при использовании каких-либо постоянных соединений, таких как CLI-сессии mysql или постоянные удалённые соединения от удалённых распределённых таблиц. При превышении лимита вы всё равно можете подключиться к серверу, используя [VIP connection](../Connecting_to_the_server/MySQL_protocol.md#VIP-connection). VIP-соединения не учитываются в лимит.

<!-- request Example -->
```ini
max_connections = 10
```

<!-- end -->

### max_threads_per_query

<!-- example max_threads_per_query -->
Глобальное ограничение количества потоков, которые может использовать одна операция. По умолчанию соответствующие операции могут занимать все ядра CPU, не оставляя места для других операций. Например, `call pq` к достаточно большой перколятной таблице может использовать все потоки в течение десятков секунд. Установка `max_threads_per_query` в, скажем, половину от [threads](../Server_settings/Searchd.md#threads) обеспечит возможность выполнения нескольких таких операций `call pq` параллельно.

Вы также можете установить этот параметр как переменную сессии или глобальную переменную во время выполнения.

Кроме того, вы можете управлять поведением для каждой отдельной запроса с помощью [threads OPTION](../Searching/Options.md#threads).

<!-- intro -->
##### Пример:
<!-- request Example -->

```ini
max_threads_per_query = 4
```

<!-- end -->

### max_filters

<!-- example conf max_filters -->
Максимально допустимое количество фильтров на запрос. Этот параметр используется только для внутренних проверок целостности и напрямую не влияет на использование ОЗУ или производительность. Опционально, по умолчанию 256.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
max_filters = 1024
```
<!-- end -->


### max_filter_values

<!-- example conf max_filter_values -->
Максимально допустимое количество значений для одного фильтра. Этот параметр используется только для внутренних проверок целостности и напрямую не влияет на использование ОЗУ или производительность. Опционально, по умолчанию 4096.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
max_filter_values = 16384
```
<!-- end -->


### max_open_files

<!-- example conf max_open_files -->
Максимальное количество файлов, которые серверу разрешено открыть, называется "мягким лимитом". Обратите внимание, что обслуживание больших фрагментированных таблиц реального времени может потребовать установки этого лимита на высоком уровне, так как каждый дисковый кусок может занимать десятки и более файлов. Например, таблица реального времени с 1000 кусков может требовать открытия одновременно тысяч файлов. Если в логах появляется ошибка 'Too many open files', попробуйте отрегулировать этот параметр — это может помочь решить проблему.

Существует также "жесткий лимит", который нельзя превысить с помощью этой опции. Этот лимит определяется системой и может быть изменён в файле `/etc/security/limits.conf` на Linux. На других операционных системах подходы могут отличаться, поэтому обратитесь к своим руководствам для дополнительной информации.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
max_open_files = 10000
```
<!-- end -->

<!-- example conf max_open_files max -->
Помимо непосредственных числовых значений, вы можете использовать магическое слово 'max', чтобы установить лимит равным текущему доступному жёсткому лимиту.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
max_open_files = max
```
<!-- end -->


### max_packet_size

<!-- example conf max_packet_size -->
Максимально допустимый размер сетевого пакета. Этот параметр ограничивает как пакеты запросов от клиентов, так и пакеты ответов от удалённых агентов в распределённой среде. Используется только для внутренних проверок целостности, напрямую не влияет на использование ОЗУ или производительность. Опционально, по умолчанию 128M.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
max_packet_size = 32M
```
<!-- end -->


### mysql_version_string

<!-- example conf mysql_version_string -->
Строка версии сервера, возвращаемая через протокол MySQL. Опционально, по умолчанию пусто (возвращается версия Manticore).

Некоторые придирчивые клиентские библиотеки MySQL зависят от определённого формата номера версии, используемого MySQL, и более того, иногда выбирают разный путь выполнения в зависимости от указанного номера версии (а не от флагов возможностей). Например, Python MySQLdb 1.2.2 вызывает исключение, если номер версии не соответствует формату X.Y.ZZ; MySQL .NET connector 6.3.x внутренне падает на номерах версий 1.x с определённой комбинацией флагов и т. д. Чтобы обойти это, вы можете использовать директиву `mysql_version_string` и заставить `searchd` сообщать клиентам, подключающимся по протоколу MySQL, другую версию. (По умолчанию он сообщает свою собственную версию.)


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
mysql_version_string = 5.0.37
```
<!-- end -->


### net_workers

Количество сетевых потоков, по умолчанию 1.

Этот параметр полезен при экстремально высоких скоростях запросов, когда одного потока недостаточно для управления всеми входящими запросами.


### net_wait_tm

Управляет интервалом цикла занятости сетевого потока. По умолчанию -1, может быть установлен в -1, 0 или положительное целое число.

В случаях, когда сервер настроен как чистый мастер и просто маршрутизирует запросы на агентов, важно обрабатывать запросы без задержек и не допускать сна сетевого потока. Для этого используется цикл занятости. После входящего запроса сетевой поток использует CPU poll в течение `10 * net_wait_tm` миллисекунд, если `net_wait_tm` положительное число, или опрашивает только CPU, если `net_wait_tm` равен `0`. Также цикл занятости можно отключить значением `net_wait_tm = -1` — в этом случае poller устанавливает таймаут исходя из текущих таймаутов агента в системном вызове опроса.

> **ВНИМАНИЕ:** Цикл занятости CPU действительно загружает ядро CPU, поэтому установка этого значения отличным от значения по умолчанию вызовет заметное использование CPU даже при простаивающем сервере.


### net_throttle_accept

Определяет, сколько клиентов принимается на каждой итерации сетевого цикла. По умолчанию 0 (без ограничений), что подходит для большинства пользователей. Это опция тонкой настройки для управления пропускной способностью сетевого цикла в сценариях высокой нагрузки.


### net_throttle_action

Определяет, сколько запросов обрабатывается на каждой итерации сетевого цикла. По умолчанию 0 (без ограничений), что подходит для большинства пользователей. Это опция тонкой настройки для управления пропускной способностью сетевого цикла в сценариях высокой нагрузки.

### network_timeout

<!-- example conf network_timeout -->
Таймаут чтения/записи сетевого клиентского запроса в секундах (или [special_suffixes](../Server_settings/Special_suffixes.md)). Опционально, по умолчанию 5 секунд. `searchd` принудительно закроет соединение с клиентом, который не успел отправить запрос или прочитать результат в течение этого таймаута.

Обратите также внимание на параметр [reset_network_timeout_on_packet](../Server_settings/Searchd.md#reset_network_timeout_on_packet). Этот параметр изменяет поведение `network_timeout` с применения ко всему `query` или `result` на применение к отдельным пакетам. Обычно запрос/результат помещается в один или два пакета. Однако в случаях, когда требуется большое количество данных, этот параметр может быть бесценным для поддержания активных операций.

<!-- request Example -->

```ini
network_timeout = 10s
```
<!-- end -->

### node_address

<!-- example conf node_address -->
Этот параметр позволяет указать сетевой адрес узла. По умолчанию он установлен на адрес репликации [listen](../Server_settings/Searchd.md#listen). В большинстве случаев это корректно; однако бывают ситуации, когда необходимо указать его вручную:

* Узел за брандмауэром
* Включён транслятор сетевых адресов (NAT)
* Развёртывания с использованием контейнеров, например Docker или облачные развёртывания
* Кластеры с узлами в более чем одном регионе


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
node_address = 10.101.0.10
```
<!-- end -->

### not_terms_only_allowed

<!-- example conf not_terms_only_allowed -->
Этот параметр определяет, разрешать ли запросы, состоящие только из оператора полнотекстового поиска с [отрицанием](../Searching/Full_text_matching/Operators.md#Negation-operator). Необязательный, значение по умолчанию — 0 (запросы с одним оператором NOT отклоняются).


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
not_terms_only_allowed = 1
```
<!-- end -->

### optimize_cutoff

<!-- example conf optimize_cutoff -->
Устанавливает порог сжатия таблицы по умолчанию. Подробнее здесь — [Number of optimized disk chunks](../Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks). Этот параметр можно переопределить опцией запроса [cutoff](../Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks). Также возможна динамическая смена через [SET GLOBAL](../Server_settings/Setting_variables_online.md#SET).

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
optimize_cutoff = 4
```
<!-- end -->

### persistent_connections_limit

<!-- example conf persistent_connections_limit -->
Этот параметр определяет максимальное количество одновременных постоянных соединений с удалёнными [persistent agents](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md). Каждый раз при подключении агента, определённого под `agent_persistent`, стараемся использовать существующее соединение (если оно есть), либо подключаемся и сохраняем соединение для будущего использования. Однако в некоторых случаях имеет смысл ограничить количество таких постоянных соединений. Эта директива задаёт предел. Он влияет на количество соединений к каждому хосту агента по всем распределённым таблицам.

Рекомендуется установить значение равным или меньшим параметра [max_connections](../Server_settings/Searchd.md#max_connections) в конфигурации агента.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
persistent_connections_limit = 29 # assume that each host of agents has max_connections = 30 (or 29).
```
<!-- end -->


### pid_file

<!-- example conf pid_file -->
pid_file — обязательный параметр конфигурации Manticore search, задающий путь к файлу, в котором хранится идентификатор процесса сервера `searchd`.

Файл идентификатора процесса searchd пересоздаётся и блокируется при запуске, и содержит идентификатор главного процесса сервера, пока сервер работает. Файл удаляется при остановке сервера.
Назначение этого файла — позволить Manticore выполнять различные внутренние задачи, такие как проверка наличия уже запущенного экземпляра `searchd`, остановка `searchd` и уведомление о необходимости ротировать таблицы. Файл также может использоваться внешними скриптами автоматизации.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
pid_file = /run/manticore/searchd.pid
```
<!-- end -->


### predicted_time_costs

<!-- example conf predicted_time_costs -->
Затраты модели предсказания времени выполнения запроса, в наносекундах. Необязательный, значение по умолчанию — `doc=64, hit=48, skip=2048, match=64`.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
predicted_time_costs = doc=128, hit=96, skip=4096, match=128
```
<!-- end -->

<!-- example conf predicted_time_costs 1 -->
Прерывание запросов до их завершения на основе времени выполнения (с помощью настройки максимального времени запроса) — удобная страховка, но она сопровождается присущим ей недостатком: неопределёнными (нестабильными) результатами. То есть, если вы повторите один и тот же (сложный) поисковый запрос с ограничением по времени несколько раз, лимит времени будет достигаться на разных этапах, и вы получите *разные* наборы результатов.

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SELECT … OPTION max_query_time
```
<!-- request API -->

```api
SetMaxQueryTime()
```
<!-- end -->

Существует опция [SELECT … OPTION max_predicted_time](../Searching/Options.md#max_predicted_time), которая позволяет ограничить время запроса *и* получить стабильные, повторяемые результаты. Вместо регулярной проверки фактического текущего времени при оценке запроса, что является неопределённым процессом, она прогнозирует текущее время выполнения с помощью простой линейной модели:

```ini
predicted_time =
    doc_cost * processed_documents +
    hit_cost * processed_hits +
    skip_cost * skiplist_jumps +
    match_cost * found_matches
```

Запрос прерывается заранее, когда `predicted_time` достигает заданного предела.

Конечно, это не жёсткое ограничение на фактическое затраченное время (впрочем, оно является жёстким ограничением на количество *обработанной* работы), и простая линейная модель ни в коем случае не является идеально точной. Поэтому реальное время на часах *может* быть как меньше, так и больше заданного лимита. Однако погрешности вполне приемлемы: например, в наших экспериментах с целевым лимитом в 100 мс большинство тестовых запросов укладывались в диапазон от 95 до 105 мс, и *все* запросы — в диапазон от 80 до 120 мс. Также, в качестве приятного побочного эффекта, использование моделируемого времени запроса вместо измерения реального времени выполнения приводит к небольшому уменьшению количества вызовов gettimeofday().

Ни два сервера одинаковой марки и модели не бывают идентичны, поэтому директива `predicted_time_costs` позволяет настроить затраты для приведённой выше модели. Для удобства они выражены целыми числами и считаются в наносекундах. (Ограничение в max_predicted_time считается в миллисекундах, и указание значений затрат как 0.000128 мс вместо 128 нс несколько более подвержено ошибкам.) Не обязательно указывать все четыре значения затрат одновременно, пропущенные будут иметь значения по умолчанию. Однако мы настоятельно рекомендуем указать все для удобочитаемости.


### preopen_tables

<!-- example conf preopen_tables -->
Директива конфигурации preopen_tables задаёт, нужно ли принудительно предварительно открывать все таблицы при запуске. Значение по умолчанию — 1, что означает, что все таблицы будут предварительно открыты независимо от настройки [preopen](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Other-performance-related-settings) для каждой таблицы. При значении 0 будут учитываться настройки для каждой таблицы, которые по умолчанию равны 0.

Предварительное открытие таблиц может предотвратить гонки между поисковыми запросами и ротациями, которые иногда могут приводить к сбоям запросов. Однако это также увеличивает количество используемых дескрипторов файлов. В большинстве сценариев рекомендуется предварительно открывать таблицы.

Вот пример конфигурации:

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
preopen_tables = 1
```
<!-- end -->

### pseudo_sharding

<!-- example conf pseudo_sharding -->
Опция конфигурации pseudo_sharding позволяет параллелить поисковые запросы к локальным простым и реальном времени таблицам, независимо от того, запрашиваются ли они напрямую или через распределённую таблицу. Эта функция автоматически распараллеливает запросы до числа потоков, заданных в `searchd.threads` # потоков.

Обратите внимание, что если ваши рабочие потоки уже заняты, потому что у вас:
* высокая конкуренция запросов
* физическое шардирование любого рода:
  - распределённая таблица из нескольких простых/реальным времени таблиц
  - реальное времени таблица, состоящая из слишком большого числа дисковых чанков

тогда включение pseudo_sharding может не дать никаких преимуществ и даже привести к небольшому снижению пропускной способности. Если для вас важна большая пропускная способность, а не меньшая задержка, рекомендуется отключить эту опцию.

Включено по умолчанию.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
pseudo_sharding = 0
```
<!-- end -->


### replication_connect_timeout

Директива `replication_connect_timeout` задаёт тайм-аут подключения к удалённому узлу. По умолчанию значение считается в миллисекундах, но может иметь [другой суффикс](../Server_settings/Special_suffixes.md). Значение по умолчанию — 1000 (1 секунда).

При подключении к удалённому узлу Manticore будет ждать не более указанного времени для успешного установления соединения. Если время истекает, но соединение не установлено, и включены `retries`, будет инициирована повторная попытка.


### replication_query_timeout

Директива `replication_query_timeout` задает время ожидания, в течение которого searchd будет ждать завершения запроса от удалённого узла. Значение по умолчанию — 3000 миллисекунд (3 секунды), но может быть указано с `суффиксом` для другого единицы времени.

После установления соединения Manticore будет ждать максимальное время `replication_query_timeout` для завершения удалённым узлом. Обратите внимание, что этот тайм-аут отличается от `replication_connect_timeout`, и возможная общая задержка из-за удалённого узла будет суммой обоих значений.


### replication_retry_count

Эта настройка — целое число, задающее, сколько раз Manticore попытается подключиться и выполнить запрос к удалённому узлу во время репликации, прежде чем сообщить о критической ошибке запроса. Значение по умолчанию — 3.


### replication_retry_delay

Эта настройка — целое число в миллисекундах (или [special_suffixes](../Server_settings/Special_suffixes.md)) задаёт задержку перед повторной попыткой запроса к удалённому узлу при сбое во время репликации. Это значение актуально только при ненулевом значении. Значение по умолчанию — 500.

### qcache_max_bytes

<!-- example conf qcache_max_bytes -->
Эта настройка конфигурации задаёт максимальный объём оперативной памяти, выделенной для кэшированных наборов результатов, в байтах. Значение по умолчанию — 16777216, что эквивалентно 16 мегабайтам. Если значение выставить в 0, кэш запросов будет отключен. Для дополнительной информации о кэше запросов, пожалуйста, обратитесь к разделу [query cache](../Searching/Query_cache.md).


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
qcache_max_bytes = 16777216
```
<!-- end -->


### qcache_thresh_msec

Целое число, в миллисекундах. Минимальный порог по времени выполнения запроса, при котором результат будет кэшироваться. Значение по умолчанию — 3000, или 3 секунды. 0 означает кэшировать всё. Подробности смотрите в разделе [query cache](../Searching/Query_cache.md). Это значение также может быть выражено с помощью временных [special_suffixes](../Server_settings/Special_suffixes.md), но используйте это с осторожностью, чтобы не запутаться из-за названия переменной с суффиксом '_msec'.


### qcache_ttl_sec

Целое число, в секундах. Период действия кэшированного результата. Значение по умолчанию — 60, или 1 минута. Минимально возможное значение — 1 секунда. Подробности смотрите в разделе [query cache](../Searching/Query_cache.md). Это значение также может быть выражено с помощью временных [special_suffixes](../Server_settings/Special_suffixes.md), но используйте это с осторожностью, чтобы не запутаться из-за названия переменной с суффиксом '_sec'.


### query_log_format

<!-- example conf query_log_format -->
Формат журнала запросов. Допустимые значения: `plain` и `sphinxql`, значение по умолчанию — `sphinxql`.

Режим `sphinxql` логирует валидные SQL-запросы. Режим `plain` логирует запросы в текстовом формате (в основном подходит для чисто полнотекстовых сценариев). Эта директива позволяет переключаться между двумя форматами при запуске сервера поиска. Формат журнала также можно изменить на лету с помощью синтаксиса `SET GLOBAL query_log_format=sphinxql`. Более подробную информацию см. в разделе [Query logging](../Logging/Query_logging.md).


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
query_log_format = sphinxql
```
<!-- end -->

### query_log_min_msec

Лимит (в миллисекундах), который предотвращает запись запроса в журнал запросов. Необязательно, по умолчанию 0 (все запросы записываются в журнал запросов). Эта директива указывает, что будут логироваться только запросы, время выполнения которых превышает заданный лимит (это значение также может быть выражено с помощью [специальных_суффиксов](../Server_settings/Special_suffixes.md), но используйте это с осторожностью и не путайте с именем самого значения, содержащим `_msec`).

### query_log

<!-- example conf query_log -->
Имя файла журнала запросов. Необязательно, по умолчанию пусто (не логировать запросы). Все поисковые запросы (такие как SELECT ... но не INSERT/REPLACE/UPDATE) будут записаны в этот файл. Формат описан в [Журналирование запросов](../Logging/Query_logging.md). В случае формата 'plain', вы можете использовать 'syslog' в качестве пути к журналу. В этом случае все поисковые запросы будут отправлены демону syslog с приоритетом `LOG_INFO`, с префиксом '[query]' вместо временной метки. Для использования опции syslog, Manticore должен быть сконфигурирован с `-–with-syslog` при сборке.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
query_log = /var/log/query.log
```
<!-- end -->


### query_log_mode

<!-- example conf query_log_mode -->
Директива query_log_mode позволяет установить различные права доступа для файлов журналов searchd и query. По умолчанию эти журналы создаются с правами 600, что означает, что только пользователь, под которым запущен сервер, и пользователи root могут читать эти файлы.
Эта директива полезна, если вы хотите разрешить другим пользователям читать файлы журналов, например, решениям для мониторинга, работающим с непривилегированными пользователями.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
query_log_mode  = 666
```
<!-- end -->

### read_buffer_docs

<!-- example conf read_buffer_docs -->
Директива read_buffer_docs управляет размером буфера чтения на ключевое слово для списков документов. Для каждого вхождения ключевого слова в каждом поисковом запросе есть два связанных буфера чтения: один для списка документов и один для списка попаданий. Эта настройка позволяет управлять размером буфера списка документов.

Больший размер буфера может увеличить использование ОЗУ на запрос, но может уменьшить время ввода-вывода. Имеет смысл устанавливать большие значения для медленного хранилища, но для хранилищ с высокой производительностью ввода-вывода экспериментируйте с малыми значениями.

Значение по умолчанию — 256K, минимальное — 8K. Вы также можете задать [read_buffer_docs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#read_buffer_docs) на уровне конкретной таблицы, что переопределит настройки на уровне сервера.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
read_buffer_docs = 128K
```
<!-- end -->


### read_buffer_hits

<!-- example conf read_buffer_hits -->
Директива read_buffer_hits задаёт размер буфера чтения на ключевое слово для списков попаданий в поисковых запросах. По умолчанию размер 256K, минимальное значение — 8K. Для каждого вхождения ключевого слова в запросе связаны два буфера — для списка документов и для списка попаданий. Увеличение буфера может увеличить использование ОЗУ на запрос, но снизить время ввода-вывода. Для медленного хранилища имеет смысл использовать большие размеры буфера, для хранилищ с высокой производительностью ввода-вывода рекомендуется экспериментировать с малыми значениями.

Эту настройку можно также задать на уровне конкретной таблицы с помощью параметра read_buffer_hits в [read_buffer_hits](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#read_buffer_hits), что переопределит значение на уровне сервера.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
read_buffer_hits = 128K
```
<!-- end -->

### read_unhinted

<!-- example conf read_unhinted -->
Размер чтения без подсказки. Необязательно, по умолчанию 32K, минимально 1K

При выполнении запросов некоторые операции чтения заранее знают, сколько данных предстоит считать, а некоторые — нет. Наиболее заметно, что размер списка попаданий заранее неизвестен. Эта настройка позволяет контролировать объём данных, который читается в таких случаях. Она влияет на время ввода-вывода для списков попаданий, уменьшая его для списков, больших, чем размер unhinted read, и увеличивая для меньших. На использование ОЗУ не влияет, так как буфер уже выделен. Следовательно, этот параметр не должен быть больше, чем read_buffer.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
read_unhinted = 32K
```
<!-- end -->

### reset_network_timeout_on_packet

<!-- example conf reset_network_timeout_on_packet -->
Уточняет поведение сетевых таймаутов (таких как `network_timeout` и `agent_query_timeout`).

Если установить в 0, таймауты ограничивают максимальное время отправки всего запроса/запроса.
Если установить в 1 (по умолчанию), таймауты ограничивают максимальное время между сетевой активностью.

При репликации узел может отправлять большой файл (например, 100ГБ) другому узлу. Предположим, сеть передаёт данные со скоростью 1ГБ/с отдельными пакетами размером 4-5МБ. Передача всего файла займет 100 секунд. Таймаут по умолчанию в 5 секунд позволит передать только 5ГБ до разрыва соединения. Увеличение таймаута — временное решение, но оно не масштабируемо (следующий файл может быть 150ГБ, что опять вызовет сбой). Однако при установленном по умолчанию значении `reset_network_timeout_on_packet` равном 1 таймаут применяется не ко всей передаче, а к отдельным пакетам. Пока передача продолжается (и данные действительно принимаются в течение периода таймаута), соединение поддерживается. Если передача застрянет, и таймаут произойдет между пакетами, соединение будет разорвано.

Заметьте, если у вас настроена распределённая таблица, каждый узел — как мастер, так и агенты — должен быть настроен. Для мастера актуален `agent_query_timeout`, для агентов — `network_timeout`.

<!-- intro -->

##### Пример:

<!-- request Example -->

```ini
reset_network_timeout_on_packet = 0
```

<!-- end -->


### rt_flush_period

<!-- example conf rt_flush_period -->
Период проверки сброса RAM-чунков RT таблиц, в секундах (или с [специальными_суффиксами](../Server_settings/Special_suffixes.md)). Необязательно, по умолчанию 10 часов.

Активно обновляемые RT-таблицы, полностью помещающиеся в RAM-чанк, могут всё же приводить к постоянно растущим binlog, что влияет на использование диска и время восстановления после сбоя. С помощью этой директивы сервер поиска выполняет периодические проверки на возможность очистки, и пригодные RAM-чанки могут быть сохранены, что позволяет выполнить последующую очистку binlog. Подробнее см. в разделе [Бинарное логирование](../Logging/Binary_logging.md).

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
rt_flush_period = 3600 # 1 hour
```
<!-- end -->


### rt_merge_iops

<!-- example conf rt_merge_iops -->
Максимальное количество операций ввода-вывода (в секунду), которые потоку объединения RT-чанков разрешено запускать. Необязательно, по умолчанию значение 0 (без ограничения).

Данная директива позволяет ограничить влияние на I/O, вызываемое операторами `OPTIMIZE`. Гарантируется, что все операции оптимизации RT не будут генерировать больше операций дискового ввода-вывода (IOPS) в секунду, чем указанный лимит. Ограничение rt_merge_iops может снизить просадку производительности поиска, вызванную операциями объединения.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
rt_merge_iops = 40
```
<!-- end -->

### rt_merge_maxiosize

<!-- example conf rt_merge_maxiosize -->
Максимальный размер операции ввода-вывода, которую потоку объединения RT-чанков разрешено запускать. Необязательно, по умолчанию значение 0 (без ограничения).

Данная директива позволяет ограничить влияние на I/O, вызываемое операторами `OPTIMIZE`. Операции ввода-вывода, превышающие данный лимит, будут разбиты на две и более операций, которые затем будут учитываться как отдельные операции с точки зрения лимита [rt_merge_iops](../Server_settings/Searchd.md#rt_merge_iops). Таким образом, гарантируется, что все операции оптимизации не будут генерировать больше, чем (rt_merge_iops * rt_merge_maxiosize) байт дискового ввода-вывода в секунду.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
rt_merge_maxiosize = 1M
```
<!-- end -->


### seamless_rotate

<!-- example conf seamless_rotate -->
Предотвращает "зависание" `searchd` при ротации таблиц с большим объёмом данных для предварительной загрузки в память. Необязательно, по умолчанию 1 (бесшовная ротация включена). На системах Windows бесшовная ротация по умолчанию отключена.

Таблицы могут содержать данные, которые необходимо предварительно загрузить в ОЗУ. В настоящий момент файлы `.spa`, `.spb`, `.spi` и `.spm` полностью предварительно загружаются в память (они содержат данные атрибутов, blob-атрибутов, таблицу ключевых слов и карту "убитых" строк соответственно). Без бесшовной ротации при ротации таблицы используется минимум RAM и процесс выглядит так:

1. Новые запросы временно отклоняются (с ошибкой "retry");
2. `searchd` ожидает завершения всех текущих запросов;
3. Старая таблица освобождается, её файлы переименовываются;
4. Файлы новой таблицы переименовываются, выделяется необходимая память;
5. Данные атрибутов и словарей новой таблицы подгружаются в память;
6. `searchd` возобновляет обработку запросов из новой таблицы.

Однако, если объём атрибутов или словарей значителен, то этап подгрузки может занять заметное время — до нескольких минут при подгрузке файлов в 1-5+ ГБ.

При включённой бесшовной ротации процесс выглядит так:

1. Выделяется память под новую таблицу;
2. Данные атрибутов и словарей новой таблицы асинхронно подгружаются в память;
3. В случае успеха освобождается старая таблица и оба набора файлов переименовываются;
4. В случае неудачи новая таблица освобождается;
5. В любой момент запросы обрабатываются либо из старой, либо из новой копии таблицы.

Бесшовная ротация требует большего пикового объёма памяти во время ротации (так как одновременно в памяти находятся старые и новые данные `.spa/.spb/.spi/.spm`). Среднее потребление памяти остаётся прежним.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
seamless_rotate = 1
```
<!-- end -->

### secondary_index_block_cache
<!-- example conf secondary_index_block_cache -->

Данная опция задаёт размер кэша блоков, используемого вторичными индексами. Необязательная, по умолчанию 8 МБ. Когда вторичные индексы работают с фильтрами, содержащими множество значений (например, фильтры IN()), они читают и обрабатывают блоки метаданных для этих значений.
В объединённых запросах этот процесс повторяется для каждого пакета строк из левой таблицы, и каждый пакет может повторно читать одни и те же метаданные в рамках одного запроса с объединением. Это может серьёзно повлиять на производительность. Кэш блоков метаданных хранит эти блоки в памяти, чтобы
их могли использовать последующие пакеты данных.

Кэш используется только в объединённых запросах и не влияет на не объединённые запросы. Обратите внимание, что ограничение размера кэша действует для каждого атрибута и для каждого вторичного индекса. Каждый атрибут в каждом дисковом чанке работает в рамках этого лимита. В худшем случае общий объём
использования памяти можно оценить, умножив лимит на количество дисковых чанков и количество атрибутов, используемых в объединённых запросах.

Значение `secondary_index_block_cache = 0` выключает кэш.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
secondary_index_block_cache = 16M
```

<!-- end -->

### secondary_indexes
<!-- example conf secondary_indexes -->

Эта опция включает/отключает использование вторичных индексов в поисковых запросах. Необязательная, по умолчанию стоит 1 (включено). Обратите внимание, для индексации включать не нужно — она всегда включена если установлена [Manticore Columnar Library](https://github.com/manticoresoftware/columnar). Эта библиотека также необходима для использования индексов при поиске. Доступно три режима:

* `0`: Отключить использование вторичных индексов при поиске. Можно включить индивидуально для запроса с помощью [подсказок анализатора](../Searching/Options.md#Query-optimizer-hints)
* `1`: Включить использование вторичных индексов при поиске. Можно отключать индивидуально для запросов с помощью [подсказок анализатора](../Searching/Options.md#Query-optimizer-hints)
* `force`: То же, что и включить, но любые ошибки при загрузке вторичных индексов будут зафиксированы, и индекс полностью не загрузится в демон.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
secondary_indexes = 1
```

<!-- end -->

### server_id

<!-- example conf server_id -->
Целое число, служащее идентификатором сервера и используемое в качестве начального значения для генерации уникального короткого UUID для узлов, входящих в состав кластера репликации. server_id должен быть уникальным среди узлов кластера и находиться в диапазоне от 0 до 127. Если server_id не установлен, он вычисляется как хеш MAC-адреса и пути к PID-файлу или в качестве начального значения для короткого UUID будет использоваться случайное число.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
server_id = 1
```
<!-- end -->


### shutdown_timeout

<!-- example conf shutdown_timeout -->
Время ожидания для команды `searchd --stopwait` в секундах (или [special_suffixes](../Server_settings/Special_suffixes.md)). Опционально, по умолчанию 60 секунд.

Когда вы запускаете `searchd --stopwait`, вашему серверу нужно выполнить некоторые действия перед остановкой, такие как завершение запросов, сброс RT RAM чанков, сброс атрибутов и обновление binlog. Эти задачи требуют определенного времени. `searchd --stopwait` будет ждать до `shutdown_time` секунд, пока сервер завершает свои задачи. Подходящее время зависит от размера вашей таблицы и нагрузки.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
shutdown_timeout = 3m # wait for up to 3 minutes
```
<!-- end -->


### shutdown_token

SHA1-хеш пароля, необходимого для вызова команды 'shutdown' из VIP Manticore SQL соединения. Без него,[debug](../Reporting_bugs.md#DEBUG) подкоманда 'shutdown' никогда не вызовет остановку сервера. Обратите внимание, что подобное простое хеширование не стоит считать надежной защитой, так как мы не используем соленый хеш или какую-либо современную хеш-функцию. Это предназначено как надежная мера для фоновых демонов в локальной сети.

### snippets_file_prefix

<!-- example conf snippets_file_prefix -->
Префикс, добавляемый к локальным именам файлов при генерации сниппетов. Опционально, по умолчанию - текущая рабочая папка.

Этот префикс можно использовать при распределённой генерации сниппетов вместе с опциями `load_files` или `load_files_scattered`.

Обратите внимание, что это префикс, а **не** путь! Это значит, что если префикс установлен в "server1", и запрос ссылается на "file23", `searchd` попытается открыть "server1file23" (всё это без кавычек). Таким образом, если вам нужен именно путь, необходимо включить завершающий слеш.

После формирования окончательного пути к файлу сервер разворачивает все относительные директории и сравнивает окончательный результат со значением `snippet_file_prefix`. Если результат не начинается с префикса, такой файл будет отклонён с сообщением об ошибке.

Например, если вы установите `/mnt/data` и кто-то попробует сгенерировать сниппет из файла `../../../etc/passwd`, он получит сообщение об ошибке:

`File '/mnt/data/../../../etc/passwd' escapes '/mnt/data/' scope`

вместо содержимого файла.

Также при неустановленном параметре и чтении `/etc/passwd`, на самом деле будет читаться /daemon/working/folder/etc/passwd, поскольку параметр по умолчанию – рабочая папка сервера.

Учтите, что это локальная настройка; она никак не влияет на агенты. Вы можете безопасно настроить префикс на мастере. Запросы, маршрутизируемые к агентам, не будут зависеть от настроек мастера. Однако они будут зависеть от собственных настроек агента.

Это может быть полезно, например, когда места хранения документов (локальное хранилище или точки монтирования NAS) различаются между серверами.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
snippets_file_prefix = /mnt/common/server1/
```
<!-- end -->

> **ВНИМАНИЕ:** Если вы всё же хотите получить доступ к файлам из корня файловой системы, вы должны явно установить `snippets_file_prefix` в пустое значение (строкой `snippets_file_prefix=`) или в корень (`snippets_file_prefix=/`).


### sphinxql_state

<!-- example conf sphinxql_state -->
Путь к файлу, в который сериализуется текущее состояние SQL.

При запуске сервера этот файл воспроизводится. При соответствующих изменениях состояния (например, SET GLOBAL) этот файл перезаписывается автоматически. Это помогает предотвратить трудно диагностируемые проблемы: если вы загружаете UDF-функции, но Manticore аварийно завершается, при автоматическом перезапуске ваши UDF и глобальные переменные будут недоступны. Использование постоянного состояния помогает обеспечить плавное восстановление без подобных сюрпризов.

`sphinxql_state` не может использоваться для выполнения произвольных команд, таких как `CREATE TABLE`.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
sphinxql_state = uservars.sql
```
<!-- end -->


### sphinxql_timeout

<!-- example conf sphinxql_timeout -->
Максимальное время ожидания между запросами (в секундах или [special_suffixes](../Server_settings/Special_suffixes.md)) при использовании SQL-интерфейса. Опционально, по умолчанию 15 минут.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
sphinxql_timeout = 15m
```
<!-- end -->


### ssl_ca

<!-- example conf ssl_ca -->
Путь к файлу сертификата центра сертификации (CA) SSL (также известен как корневой сертификат). Опционально, по умолчанию пусто. Если не пуст, сертификат в `ssl_cert` должен быть подписан этим корневым сертификатом.

Сервер использует CA файл для проверки подписи сертификата. Файл должен быть в формате PEM.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
ssl_ca = keys/ca-cert.pem
```
<!-- end -->


### ssl_cert

<!-- example conf ssl_cert -->
Путь к SSL-сертификату сервера. Опционально, по умолчанию пусто.

Сервер использует этот сертификат в качестве самоподписанного публичного ключа для шифрования HTTP-трафика через SSL. Файл должен быть в формате PEM.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
ssl_cert = keys/server-cert.pem
```
<!-- end -->


### ssl_key

<!-- example conf ssl_key -->
Путь к ключу SSL-сертификата. Опционально, по умолчанию пусто.

Сервер использует этот приватный ключ для шифрования HTTP-трафика через SSL. Файл должен быть в формате PEM.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
ssl_key = keys/server-key.pem
```
<!-- end -->


### subtree_docs_cache

<!-- example conf subtree_docs_cache -->
Максимальный размер кеша документов общего поддерева на запрос. Опционально, по умолчанию 0 (отключено).

Эта настройка ограничивает использование ОЗУ оптимизатором общих поддеревьев (см. [multi-queries](../Searching/Multi-queries.md)). Максимально выделяется столько памяти для кеширования записей документов для каждого запроса. Установка значения 0 отключает оптимизатор.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
subtree_docs_cache = 8M
```
<!-- end -->


### subtree_hits_cache

<!-- example conf subtree_hits_cache -->
Максимальный размер кэша попаданий общих поддеревьев, на запрос. Необязательно, по умолчанию 0 (отключено).

Этот параметр ограничивает использование ОЗУ оптимизатором общих поддеревьев (см. [мультизапросы](../Searching/Multi-queries.md)). Максимально столько ОЗУ будет потрачено на кэширование вхождений ключевых слов (попаданий) для каждого запроса. Установка лимита в 0 отключает оптимизатор.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
subtree_hits_cache = 16M
```
<!-- end -->

### threads

<!-- example threads -->
Количество рабочих потоков (или размер пула потоков) для демона Manticore. Manticore создаёт такое количество потоков ОС при запуске, и они выполняют все задачи внутри демона, такие как выполнение запросов, создание сниппетов и т.д. Некоторые операции могут быть разбиты на подзадачи и выполняться параллельно, например:

* Поиск в таблице в реальном времени
* Поиск в распределённой таблице, состоящей из локальных таблиц
* Вызов перколяции запроса
* и другие

По умолчанию установлено количество потоков равное числу ядер CPU на сервере. Manticore создаёт потоки при запуске и сохраняет их до остановки. Каждая подзадача может использовать один из потоков при необходимости. По завершении подзадачи поток освобождается, чтобы использоваться другой подзадачей.

В случае интенсивной I/O нагрузки может иметь смысл установить значение выше, чем число ядер CPU.

<!-- request Example -->
```ini
threads = 10
```

<!-- end -->

### thread_stack

<!-- example conf thread_stack -->
Максимальный размер стека для задачи (корутина, один поисковый запрос может вызывать несколько задач/корутин). Необязательно, по умолчанию 128K.

Каждая задача имеет собственный стек размером 128K. Когда запускается запрос, проверяется, сколько стека он требует. Если стандартных 128K хватает — запрос обрабатывается напрямую. Если требуется больше, запускается другая задача с увеличенным стеком, которая продолжает обработку. Максимальный размер такого увеличенного стека ограничивается этим параметром.

Установка значения в разумно высоком размере поможет обрабатывать очень глубокие запросы без значительного увеличения общего потребления ОЗУ. Например, установка значения 1G не означает, что каждая новая задача будет занимать 1G ОЗУ, а лишь то, что если задача требует, скажем, 100M стека, то мы выделяем именно 100M. Другие задачи при этом будут работать с дефолтным стеком в 128K. Таким образом, можно запускать ещё более сложные запросы с потребностью в 500M стека. И только если задача потребует более 1G — мы увидим ошибку и сообщим о слишком низком значении thread_stack.

Однако на практике даже запрос, нуждающийся в стеке 16M, часто слишком сложен для обработки парсером и требует много времени и ресурсов. Поэтому демон обработает его, но ограничение таких запросов через `thread_stack` вполне разумно.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
thread_stack = 8M
```
<!-- end -->


### unlink_old

<!-- example conf unlink_old -->
Определяет, следует ли удалять копии таблиц с расширением `.old` при успешной ротации. Необязательно, по умолчанию 1 (удалять).


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
unlink_old = 0
```
<!-- end -->


### watchdog

<!-- example conf watchdog -->
Многопоточный сторожевой процесс сервера. Необязательно, по умолчанию 1 (сторож включён).

Когда запрос Manticore завершается с ошибкой, это может привести к падению всего сервера. При включённом сторожевом процессе у `searchd` есть отдельный лёгкий процесс, который следит за основным серверным процессом и автоматически перезапускает его в случае аномального завершения. Сторожевой процесс включён по умолчанию.

<!-- request Example -->

```ini
watchdog = 0 # disable watchdog
```
<!-- end -->
<!-- proofread -->

