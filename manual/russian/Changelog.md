# Журнал изменений

## Версия 25.0.0
**Выпуск**: 30 марта 2026 года

Этот выпуск приносит более широкий пересмотр упаковки, новые опции `API_URL` и `API_TIMEOUT` для авто-встраивания моделей, гибридный поиск, поддержку резервного копирования и восстановления, совместимую с S3, более быстрое обслуживание RT-таблиц с N-сторонними слияниями и параллельным `OPTIMIZE`, префильтрацию KNN и подготовленные выражения, совместимые с MySQL.

## Критические изменения
* ⚠️ [v25.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/25.0.0) [ PR #123](https://github.com/manticoresoftware/columnar/pull/123) **Критическое изменение**: обновлён **MCL** до версии 13.0.0, добавлены [`API_URL`](Searching/KNN.md#Creating-a-table-with-auto-embeddings) и [`API_TIMEOUT`](Searching/KNN.md#Creating-a-table-with-auto-embeddings) для [**авто-встраивающих моделей**](Searching/KNN.md#Auto-Embeddings-%28Recommended%29). Это затрагивает вас, если вы используете [**авто-встраивающие модели**](Searching/KNN.md#Auto-Embeddings-%28Recommended%29) или управляете **MCL** отдельно от демона, потому что Manticore 25.0.0 требует **MCL 13.0.0** и не совместим со старыми версиями MCL; обновите демон и MCL вместе, и откат безопасен только если вы также восстановите соответствующую старую версию MCL.
* ⚠️ [v24.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/24.0.0) [ Issue #4343](https://github.com/manticoresoftware/manticoresearch/issues/4343) **Критическое изменение**: внутренняя версия **протокола репликации** увеличена, и добавлено отслеживание эпохи кластера, чтобы восстанавливающиеся узлы могли обнаруживать, когда членство таблиц в кластере изменилось, пока они были отключены, и использовать SST вместо сбоя во время восстановления IST. Это затрагивает вас только если вы используете **кластеры репликации**: кластеры репликации со смешанными версиями не совместимы через это изменение, поэтому обновите узлы кластера вместе, и откат безопасен только до того, как новый протокол репликации будет использован в кластере.
* ⚠️ [v23.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/23.0.0) [ Issue #4364](https://github.com/manticoresoftware/manticoresearch/issues/4364) **Критическое изменение**: введена **версия формата индекса 69** для поддержки новых опций токенизации биграмм, включая режимы [`bigram_delimiter`](Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#bigram_delimiter) и цифро-зависимые режимы [`bigram_index`](Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#bigram_index). Это затрагивает вас, если вы используете эти опции биграмм или если вы перестраиваете индексы и позже вам нужно открыть перезаписанные индексы в старой версии Manticore: существующие старые индексы остаются читаемыми, но индексы, перестроенные или вновь записанные в формате 69, не совместимы со старыми версиями Manticore, поэтому откат безопасен только до того, как такие индексы будут перезаписаны.
* ⚠️ [v22.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/22.0.0) [ Issue #4301](https://github.com/manticoresoftware/manticoresearch/issues/4301) **Критическое изменение**: **генерация встраиваний для RT-таблиц** вынесена из фазы коммита и реплицируется как векторные данные. Это затрагивает вас, если вы используете **кластеры репликации** с RT-таблицами, у которых есть `model_name`: это сохраняет векторы, предоставленные пользователем, предотвращает регенерацию разных встраиваний на репликах и улучшает производительность **авто-встраиваний** при использовании с репликацией, но кластеры репликации со смешанными версиями не совместимы через это изменение. Обновите узлы кластера вместе; откат безопасен только до того, как новый протокол репликации будет использован в кластере.
* ⚠️ [v21.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/21.0.0) [ PR #138](https://github.com/manticoresoftware/columnar/pull/138) **Критическое изменение**: обновлён **MCL** до версии 12.0.0 и увеличено требование к **KNN API** для поддержки адаптивного [**раннего завершения KNN**](Searching/KNN.md#Early-termination) во время обхода HNSW. Это затрагивает вас, если вы управляете **MCL** отдельно от демона или выполняете **KNN-запросы**, которые полагаются на предыдущее поведение исследования кандидатов или выбор пограничных результатов: Manticore 21.0.0 требует **MCL 12.0.0** с новым интерфейсом KNN, и с этой библиотекой поиск KNN может остановиться раньше, как только будет найдено достаточно кандидатов, что может изменить, какие кандидаты исследуются и возвращаются. Существующие данные остаются совместимыми, и специальная миграция не требуется, но обновите демон и MCL вместе; откат безопасен только если вы также восстановите соответствующую старую версию MCL.
* ⚠️ [v19.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/19.0.0) [ Issue #4103](https://github.com/manticoresoftware/manticoresearch/issues/4103) **Критическое изменение**: добавлена [**префильтрация KNN**](Searching/KNN.md#Filtering-strategies:-prefilter-vs.-postfilter). Это затрагивает вас, если вы выполняете **KNN-запросы** вместе с фильтрами атрибутов, потому что фильтры теперь могут применяться во время поиска KNN вместо только после выбора кандидатов. Это изменяет поведение выбора результатов для отфильтрованных KNN-запросов, отдавая приоритет отфильтрованным ближайшим соседям; специальная миграция не требуется, но запросы, полагающиеся на предыдущее поведение результатов после фильтрации, могут возвращать другие результаты после обновления. Откат возможен, так как эта функция не вносит изменений в формат индекса или протокола.
* ⚠️ [v18.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/18.0.0) [ Issue #4261](https://github.com/manticoresoftware/manticoresearch/issues/4261) **Критическое изменение**: введена **версия формата индекса 68** для исправления обработки макета словаря [`hitless_words`](Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#hitless_words). Это затрагивает вас, если вы перестраиваете или вновь записываете индексы и позже вам нужно открыть их в старой версии Manticore: существующие старые индексы остаются читаемыми, но любые индексы, перестроенные или вновь записанные в новом формате, не совместимы со старыми версиями Manticore, поэтому откат безопасен только до того, как такие индексы будут перезаписаны.

### Изменения в пакетах
* 🆕 [v25.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/25.0.0) [ PR #4357](https://github.com/manticoresoftware/manticoresearch/pull/4357) Изменение упаковки: `manticore` теперь является пакетом-сборкой для deb и rpm. Он включает демон, инструменты, конвертер, заголовки для разработки, данные ICU, пакеты зависимостей (`manticore-columnar-lib`, `manticore-backup`, `manticore-buddy`, `manticore-executor`, `manticore-load`, `manticore-galera` и `manticore-tzdata`, где применимо), а также встроенные языковые пакеты для немецкого, английского и русского языков вместе с поддержкой Jieba. Украинский лемматизатор не включен; установите его отдельно, следуя инструкциям для [Debian/Ubuntu](Installation/Debian_and_Ubuntu.md#Ukrainian-lemmatizer) или [RHEL/CentOS](Installation/RHEL_and_Centos.md#Ukrainian-lemmatizer).
  При обновлении с предыдущей структуры пакетов обычно можно просто установить пакет `manticore`. Если старые разделенные пакеты вызывают конфликты, удалите их с помощью `apt remove 'manticore*'` или `yum remove 'manticore*'`, а затем установите `manticore`. Эта очистка не удаляет ваши существующие данные или конфигурацию.

## Новые возможности и улучшения
* 🆕 [v24.4.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/24.4.0) [ PR #4091](https://github.com/manticoresoftware/manticoresearch/pull/4091) Улучшена компактизация RT-таблиц путем добавления N-way объединений дисковых чанков и параллельных задач `OPTIMIZE`, сокращая время, необходимое для объединения многих дисковых чанков, и вводя новые параметры [`merge_chunks_per_job`](Server_settings/Searchd.md#merge_chunks_per_job) и [`parallel_chunk_merges`](Server_settings/Searchd.md#parallel_chunk_merges).
* 🆕 [v24.3.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/24.3.0) [ PR #133](https://github.com/manticoresoftware/manticoresearch-backup/pull/133) Обновлён Manticore Backup до версии 1.10.0, добавлена поддержка резервного копирования и восстановления в S3-совместимые хранилища, включая AWS S3, MinIO, Wasabi и Cloudflare R2.
* 🆕 [v24.2.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/24.2.0) [ Issue #2079](https://github.com/manticoresoftware/manticoresearch/issues/2079) Добавлен [гибридный поиск](Searching/Hybrid_search.md) с Reciprocal Rank Fusion (`fusion_method='rrf'`), позволяющий SQL и JSON запросам объединять полнотекстовые и KNN результаты, ранжировать их с помощью `hybrid_score()` и поддерживать несколько KNN подзапросов в одном объединенном наборе результатов.
* 🆕 [v24.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/24.1.0) [ PR #141](https://github.com/manticoresoftware/columnar/pull/141) Обновлён MCL до версии 12.4.1, добавлена поддержка GGUF-квантованных локальных моделей эмбедингов, моделей кодировщика T5 и управляемых моделей Hugging Face через скачивание с токенной авторизацией.
* 🆕 [v23.2.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/23.2.0) [ PR #4372](https://github.com/manticoresoftware/manticoresearch-buddy/pull/648) Обновлён Buddy до версии 3.44.0, добавлена поддержка нечеткого поиска для запросов, затрагивающих несколько таблиц.
* 🆕 [v23.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/23.1.0) [ PR #4357](https://github.com/manticoresoftware/manticoresearch/pull/4357) Добавлен новый пакет-сборка Manticore для deb и rpm, заменяющий предыдущий метапакет для упрощения установки и распространения входящих компонентов.
* 🆕 [v21.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/21.1.0) [ PR #4352](https://github.com/manticoresoftware/columnar/pull/140) Обновлён MCL до версии 12.1.0, улучшена производительность раннего завершения KNN и добавлено более эффективное кодирование 64–битных значений.
* 🆕 [v19.2.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/19.2.0) [ PR #4306](https://github.com/manticoresoftware/manticoresearch/pull/4306) Обновлён MCL до версии 11.1.0, добавлена поддержка кеша кодеков.
* 🆕 [v19.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/19.1.0) [ PR #4271](https://github.com/manticoresoftware/manticoresearch-buddy/pull/644) Обновлён Buddy до версии 3.43.0, включая улучшенную обработку метрик.
* 🆕 [v19.0.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/19.0.3) [ Issue #4303](https://github.com/manticoresoftware/manticoresearch/issues/4303) Добавлена отдельная обработка [`interactive_timeout`](Server_settings/Setting_variables_online.md#SET) и [`wait_timeout`](Server_settings/Setting_variables_online.md#SET) для SQL соединений, чтобы интерактивные клиенты и неинтерактивные клиенты могли использовать различные интервалы ожидания в простое, как в MySQL.
* 🆕 [v17.6.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/17.6.0) [ Issue #1124](https://github.com/manticoresoftware/manticoresearch/issues/1124) Добавлена поддержка подготовленных выражений, совместимых с MySQL, включая обработку prepare/execute через бинарный протокол и проверку привязки параметров.

## Исправления ошибок
* 🪲 [v24.2.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/24.2.3) [ Issue #4375](https://github.com/manticoresoftware/manticoresearch/issues/4375) Исправлены ложные полнотекстовые совпадения, когда `max_query_time` прерывает запросы с использованием операторов, таких как `NOTNEAR`, `MAYBE` или отрицание, поэтому прерванные поиски больше не возвращают строки, которые фактически не удовлетворяют запросу.
* 🪲 [v24.2.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/24.2.2) [ Issue #4398](https://github.com/manticoresoftware/manticoresearch/issues/4398) Исправлена статистика времени запросов, чтобы журнал запросов, глобальные счётчики и тайминги `SHOW STATUS` для каждой таблицы оставались согласованными, а внутренние запросы Buddy больше не искажают статистику демона, видимую пользователю.
* 🪲 [v24.2.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/24.2.1) [ PR #653](https://github.com/manticoresoftware/manticoresearch-buddy/pull/653) Обновлён Buddy до версии 3.44.1, исправлена некорректная обработка пустых полезных нагрузок запросов по умолчанию.
* 🪲 [v24.1.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/24.1.3) [ PR #4396](https://github.com/manticoresoftware/manticoresearch/pull/4396) Исправлен ручной запуск `searchd` на macOS, чтобы SQL-команды с поддержкой Buddy больше не завершались ошибкой из-за загрузки старой системной `libcurl`; теперь при запуске автоматически предпочитается curl от Homebrew, а `MANTICORE_CURL_LIB` по-прежнему доступна для явного переопределения.
* 🪲 [v24.1.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/24.1.2) [ PR #4394](https://github.com/manticoresoftware/manticoresearch/pull/4394) Исправлены сборки пакетов для macOS, чтобы SQL-команды с поддержкой Buddy больше не завершались ошибкой из-за того, что `searchd` линкуется с несовместимой `libcurl`; пакет теперь предпочитает curl от Homebrew и поддерживает переопределение `MANTICORE_CURL_LIB` во время выполнения.
* 🪲 [v24.1.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/24.1.1) [ Issue #4388](https://github.com/manticoresoftware/manticoresearch/issues/4388) Исправлена репликация в кластерах, когда транзакция содержит дублирующиеся идентификаторы документов, чтобы реплики больше не теряли строки, в то время как донор корректно удаляет дубликаты.
* 🪲 [v24.0.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/24.0.1) [ Issue #4354](https://github.com/manticoresoftware/manticoresearch/issues/4354) Исправлена обработка `UPDATE`, когда изменённый атрибут также должен отключить свой вторичный индекс, предотвращая некорректные предупреждения и несогласованное состояние вторичного индекса.
* 🪲 [v23.0.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/23.0.2) [ PR #4370](https://github.com/manticoresoftware/columnar/issues/125) Обновлён MCL до версии 12.1.1, исправлены сбои при генерации авто-эмбеддингов, когда текст содержит некорректный UTF-8, включая вставки в таблицы с `html_strip=1`.
* 🪲 [v23.0.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/23.0.1) [ PR #4371](https://github.com/manticoresoftware/manticoresearch/pull/4371) Исправлено отсутствие блокировки сегментов RT во время валидации эмбеддингов, предотвращая сбои и состояния гонки в операциях, связанных с эмбеддингами.
* 🪲 [v21.0.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/21.0.3) [ Issue #4315](https://github.com/manticoresoftware/manticoresearch/issues/4315) Исправлены вставки в таблицы с колонками авто-эмбеддингов, чтобы значения MVA сохранялись, а не хранились пустыми.
* 🪲 [v21.0.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/21.0.2) [ PR #4277](https://github.com/manticoresoftware/manticoresearch-buddy/pull/649) Обновлён Buddy до версии 3.43.1, исправлена обработка автодополнения и нечёткого поиска для терминов с числовыми префиксами и подстановочными звёздочками.
* 🪲 [v21.0.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/21.0.1) [ PR #4349](https://github.com/manticoresoftware/manticoresearch/pull/4349) Исправлен сбой при генерации эмбеддингов для таблиц, использующих колоночное хранилище.
* 🪲 [v19.2.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/19.2.5) [ PR #4333](https://github.com/manticoresoftware/executor/pull/81) Обновлён Executor до версии 1.4.1, переход на загрузку исходников PHP с GitHub вместо php.net для избежания ссылок, защищённых CAPTCHA, и обновление встроенного PHP до 8.4.18.
* 🪲 [v19.2.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/19.2.4) [ Issue #4314](https://github.com/manticoresoftware/manticoresearch/issues/4314) Исправлены колонки `float_vector` на основе моделей для поддержки явного пустого `FROM=''`, что теперь корректно означает «использовать все текстовые/строковые поля».
* 🪲 [v19.2.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/19.2.3) [ Issue #4315](https://github.com/manticoresoftware/manticoresearch/issues/4315) Исправлены вставки в таблицы с колонками авто-эмбеддингов `float_vector`, чтобы значения MVA сохранялись, а не хранились пустыми.
* 🪲 [v19.2.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/19.2.2) [ Issue #4297](https://github.com/manticoresoftware/manticoresearch/issues/4297) Исправлена команда `IMPORT TABLE` для корректного копирования внешних файлов, включая `hitless_words`, чтобы импортированные таблицы больше не ссылались на отсутствующие внешние файлы после импорта.
* 🪲 [v19.2.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/19.2.1) [ Issue #4229](https://github.com/manticoresoftware/manticoresearch/issues/4229) Исправлены результаты `LEFT JOIN` для выдачи корректных значений MySQL `NULL` вместо строки `NULL`, улучшая совместимость с нативными клиентами и драйверами MySQL.
* 🪲 [v19.0.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/19.0.4) [ Issue #4308](https://github.com/manticoresoftware/manticoresearch/issues/4308) Исправлены сбои демона, вызванные фильтрацией по сохранённым/полнотекстовым полям, таким как `WHERE title='test'`; эти запросы теперь возвращают ошибку вместо сбоя.
* 🪲 [v19.0.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/19.0.2) [ PR #4311](https://github.com/manticoresoftware/manticoresearch/issues/4307) Исправлены сбои в HTTP-запросах `/sql?mode=raw` с несколькими инструкциями, использующих `.@files`, за которыми следует `SHOW META`.
* 🪲 [v19.0.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/19.0.1) [ Issue #4103](https://github.com/manticoresoftware/manticoresearch/issues/4103) Обновлён MCL до версии 11.0.0 для поддержки предфильтрации KNN.
* 🪲 [v18.0.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/18.0.1) [ Issue #4293](https://github.com/manticoresoftware/manticoresearch/issues/4293) Исправлена команда `ALTER TABLE ... ADD COLUMN` для колонок авто-эмбеддингов `float_vector`, чтобы `model_name` и `FROM` сохранялись корректно, а не заменялись на `knn_dims='0'`.
* 🪲 [v17.6.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/17.6.6) [ Issue #4264](https://github.com/manticoresoftware/manticoresearch/issues/4264) Исправлена обработка EOF kqueue на macOS для предотвращения ложных предупреждений о получении HTTP, таких как `Resource temporarily unavailable`, во время обработки запросов.
* 🪲 [v17.6.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/17.6.5) [ PR #4296](https://github.com/manticoresoftware/manticoresearch/pull/4296) Исправлены оценка количества документов и стоимости для полнотекстовых узлов `AND`, улучшая решения планировщика запросов для пересекающихся полнотекстовых условий.
* 🪲 [v17.6.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/17.6.4) [ Issue #4274](https://github.com/manticoresoftware/manticoresearch/issues/4274) Исправлена ошибка `indextool --check`, не открывающего файлы `hitless_words` дисковых чанков, путём разрешения их путей относительно директории индекса.
* 🪲 [v17.6.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/17.6.3) [ Issue #4284](https://github.com/manticoresoftware/manticoresearch/issues/4284) Исправлены сбои на узлах-репликах при вставке в кластеризованные таблицы с авто-эмбеддингами путём сохранения исходного текста, необходимого для генерации эмбеддингов во время реплицируемых коммитов.
* 🪲 [v17.6.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/17.6.2) [ Issue #4257](https://github.com/manticoresoftware/manticoresearch/issues/4257) Исправлено недопустимое чтение, обнаруженное Valgrind при выполнении подготовленных выражений, путём обеспечения корректного завершения буфера ввода парсера.
* 🪲 [v17.6.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/17.6.1) [ Issue #4207](https://github.com/manticoresoftware/manticoresearch/issues/4207) Добавлено исправление регрессии и покрытие тестами для гонки сохранения дискового чанка RT, которая могла приводить к потере удалений документов и появлению дублирующихся строк после слияний.
* 🪲 [v17.5.10](https://github.com/manticoresoftware/manticoresearch/releases/tag/17.5.10) [ Issue #4207](https://github.com/manticoresoftware/columnar/issues/133) Обновлён MCL до версии 10.2.2, исправлены сбои при выполнении поисков во время вставки данных в таблицы с локальными моделями эмбеддингов путём добавления защиты от параллелизма при использовании локальных моделей.
* 🪲 [v17.5.9](https://github.com/manticoresoftware/manticoresearch/releases/tag/17.5.9) [ PR #4247](https://github.com/manticoresoftware/manticoresearch/pull/4247) Исправлена гонка во время сохранения дискового чанка RT, которая могла приводить к потере удалений документов и появлению дублирующихся строк после слияний или сохранений.
* 🪲 [v17.5.8](https://github.com/manticoresoftware/manticoresearch/releases/tag/17.5.8) [ PR #4241](https://github.com/manticoresoftware/manticoresearch/pull/4247) Исправлена некорректная оценка атрибутов после лимита в присоединённых таблицах, включая запросы `LEFT JOIN` после сброса RAM-чанков на диск.
* 🪲 [v17.5.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/17.5.7) [ PR #4239](https://github.com/manticoresoftware/manticoresearch/pull/4239) Уточнены определения времени запроса и реального времени в документации по логированию и статусу.
* 🪲 [v17.5.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/17.5.6) [ PR #4245](https://github.com/manticoresoftware/manticoresearch/pull/4245) Исправлены глобальные метрики `search_stats_ms_*`, которые завышали время поиска из-за двойного учёта локального времени выполнения; теперь эти статистики соответствуют фактическому времени выполнения запроса.
* 🪲 [v17.5.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/17.5.5) [ Issue #858](https://github.com/manticoresoftware/manticoresearch/issues/858) Исправлены функция `HIGHLIGHT()` и генерация сниппетов для текста CJK, добавляющие ненужные пробелы между словами; выделенные отрывки теперь сохраняют исходное расстояние между словами вместо токенизированного.
* 🪲 [v17.5.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/17.5.4) [ Issue #1166](https://github.com/manticoresoftware/manticoresearch/issues/1166) Исправлен сбой при INSERT после `TRUNCATE TABLE ... WITH RECONFIGURE` для RT-таблиц с `index_field_lengths=1`, особенно когда поля и строковые атрибуты смешаны; атрибуты длины поля теперь корректно настраиваются во время переконфигурации.
* 🪲 [v17.5.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/17.5.3) Исправлено повреждённое содержимое документации Wordforms в руководстве.
* 🪲 [v17.5.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/17.5.2) [ Issue #3213](https://github.com/manticoresoftware/manticoresearch/issues/3213) Исправлены JOIN-запросы с фильтрацией по выражениям, построенным из псевдонимов колонок присоединённой таблицы, например `SELECT t2.i al, (al*0.1) pct ... WHERE pct > 0`, которые могли завершаться ошибкой `incoming-schema expression missing evaluator`; теперь эти фильтры корректно вычисляются после соединения.

## Версия 17.5.1
**Выпущена**: 7 февраля 2026 г.

### Рекомендуемые библиотеки
- Рекомендуемая версия [MCL](https://github.com/manticoresoftware/columnar): 10.2.0
- Рекомендуемая версия [Buddy](Installation/Manticore_Buddy.md#Manticore-Buddy): 3.41.0

Если вы следуете [официальному руководству по установке](https://manticoresearch.com/install/), вам не нужно об этом беспокоиться.

❤️ Мы хотели бы поблагодарить [@pakud](https://github.com/pakud) за их работу над [PR #4075](https://github.com/manticoresoftware/manticoresearch/pull/4075).

## Критические изменения
* ⚠️ [v17.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/17.0.0) [ Issue #4120](https://github.com/manticoresoftware/manticoresearch/issues/4120) MCL 10.0.0: Добавлена поддержка `DROP CACHE`. Это обновляет интерфейс между демоном и MCL. Более старые версии Manticore Search не поддерживают новую MCL.
* ⚠️ [v16.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/16.0.0) [ Issue #4019](https://github.com/manticoresoftware/manticoresearch/issues/4019) JSON-ответы на перколяционные запросы теперь возвращают `_id` и `_score` совпадений как числа вместо строк, соответствуя обычному поиску; это критическое изменение для клиентов, которые полагались на строковый тип для этих полей.

## Новые возможности и улучшения
* 🆕 [v17.5.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/17.5.0) [ PR #130](https://github.com/manticoresoftware/columnar/pull/130) MCL обновлена до версии 10.2.0: Исправлена поддержка модели QWEN и добавлена поддержка дополнительных моделей.
* 🆕 [v17.3.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/17.3.0) [ PR #4186](https://github.com/manticoresoftware/manticoresearch/pull/4186) Executor обновлен до версии 1.4.0, включая обновленную версию PHP и расширение llm-php-ext.
* 🆕 [v17.2.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/17.2.0) [ PR #4195](https://github.com/manticoresoftware/manticoresearch/pull/4195) Обновлена MCL до версии 10.1.0; Добавлена поддержка локальных моделей эмбеддингов Qwen.
* 🆕 [v17.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/17.1.0) [ Issue #3826](https://github.com/manticoresoftware/manticoresearch/issues/3826) Экземпляры морфологии Jieba теперь используются совместно между таблицами с одинаковой конфигурацией (режим, флаг HMM, путь к пользовательскому словарю), что значительно снижает использование памяти, когда многие таблицы используют Jieba (например, многие пустые таблицы больше не вызывают использование ~20 ГБ).
* 🆕 [v17.0.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/17.0.7) [ Issue #2046](https://github.com/manticoresoftware/manticoresearch/issues/2046) В режиме RT стоп-слова, словоформы, исключения и hitless_words теперь можно задавать непосредственно в `CREATE TABLE` (значения, разделенные точкой с запятой; словоформы/исключения используют `>` или `=>` для пар, с экранированием через `\`), что позволяет создавать таблицы без внешних файлов; `SHOW CREATE TABLE` возвращает эти встроенные значения.
* 🆕 [v16.3.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/16.3.0) Поиск KNN теперь по умолчанию использует oversampling=3 и rescore=1, а также поддерживает опускание k, так что лимит запроса используется в качестве эффективного k; это уменьшает ненужное передискретизирование и улучшает поведение при использовании `SELECT *` с KNN на колоночных таблицах.
* 🆕 [v16.2.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/16.2.0) [ PR #4088](https://github.com/manticoresoftware/manticoresearch/pull/4088) Добавлен флаг `--quiet` (`-q`) для searchd, чтобы подавлять вывод при запуске (баннер и сообщения о предварительном кэшировании), выводя только ошибки; полезно при запуске и остановке searchd в цикле или из скриптов.
* 🆕 [v16.0.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/16.0.1) [ Issue #3336](https://github.com/manticoresoftware/manticoresearch/issues/3336) HTTP-соединения теперь по умолчанию являются постоянными при использовании HTTP/1.1: клиентам больше не нужно явно отправлять заголовок `Keep-Alive`, что снижает случайные сбои соединений в API-клиентах (например, PHP, Go). Чтобы закрыть соединение, клиент отправляет `Connection: close`. HTTP/1.0 по-прежнему требует `Connection: keep-alive` для постоянства соединения.

## Исправления ошибок
* 🪲 [v17.5.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/17.5.1) [ Issue #3498](https://github.com/manticoresoftware/manticoresearch/issues/3498) Исправлены результаты JOIN, возвращающие пустые или дублированные значения, когда столбец был одновременно строковым атрибутом и хранимым полем; теперь значение атрибута возвращается корректно.
* 🪲 [v17.4.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/17.4.2) [ Issue #2559](https://github.com/manticoresoftware/manticoresearch/issues/2559) Исправлены JOIN по строковым атрибутам JSON (например, `j.s`), которые не возвращали совпадений; теперь они работают как JOIN по обычным строковым атрибутам.
* 🪲 [v17.4.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/17.4.1) Исправлена невычисляемость хранимых атрибутов в финальной стадии при установленном cutoff.
* 🪲 [v17.2.10](https://github.com/manticoresoftware/manticoresearch/releases/tag/17.2.10) [ Issue #425](https://github.com/manticoresoftware/manticoresearch-buddy/issues/425) Автоматическое создание таблиц (автосхема) теперь работает для `REPLACE INTO`, а также для `INSERT INTO`, поэтому таблицы создаются по требованию при их отсутствии.
* 🪲 [v17.2.9](https://github.com/manticoresoftware/manticoresearch/releases/tag/17.2.9) [ Issue #3226](https://github.com/manticoresoftware/manticoresearch/issues/3226) Исправлены некорректные результаты `GROUP BY` для колоночных MVA с несколькими столбцами `GROUP BY`, отклоняя такие запросы с той же ошибкой, что и для построчных ("MVA values can't be used in multiple group-by").
* 🪲 [v17.2.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/17.2.7) [ Issue #1737](https://github.com/manticoresoftware/manticoresearch/issues/1737) Исправлена функция `highlight()` с `html_strip_mode=strip`, которая портила содержимое, декодируя сущности и изменяя теги; исходная форма сущностей теперь сохраняется в выделенном выводе.
* 🪲 [v17.2.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/17.2.6) [ Issue #3203](https://github.com/manticoresoftware/manticoresearch/issues/3203) Исправлен сбой `ALTER TABLE REBUILD SECONDARY` с ошибкой `failed to rename … .tmp.spjidx`, когда таблица имела несколько дисковых чанков.
* 🪲 [v17.2.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/17.2.5) [ Issue #3226](https://github.com/manticoresoftware/manticoresearch/issues/3226) Исправлены некорректные результаты GROUP BY для колоночных MVA с несколькими столбцами GROUP BY, отклоняя такие запросы с той же ошибкой, что и для построчных ("MVA values can't be used in multiple group-by").
* 🪲 [v17.2.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/17.2.4) [ Issue #4148](https://github.com/manticoresoftware/manticoresearch/issues/4148) Исправлены распределенные запросы, возвращающие хранимые поля из неправильного локального индекса, когда таблицы агентов содержат дублирующиеся идентификаторы документов; хранимые поля теперь следуют порядку индексов, и для дубликатов используется первый совпадающий индекс.
* 🪲 [v17.2.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/17.2.3) [ Issue #4176](https://github.com/manticoresoftware/manticoresearch/issues/4176) Исправлено нарушение работы таблиц, использующих внешние стоп-слова, словоформы или исключения, при переименовании таблицы: `ATTACH TABLE` теперь мигрирует эти файлы в новый чанковый формат и обновляет заголовки дисковых чанков, поэтому после `ALTER TABLE RENAME` демон больше не сообщает об отсутствующих внешних файлах при перезапуске.
* 🪲 [v17.2.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/17.2.2) [ Issue #1065](https://github.com/manticoresoftware/manticoresearch/issues/1065) Добавлена опция поиска expand_blended, чтобы токенизация запроса применяла правила смешения таблицы и расширяла смешанные варианты (например, "well-being" → "well-being" | "wellbeing" | "well" "being"), позволяя одному запросу находить документы, проиндексированные в любой из этих форм.
* 🪲 [v17.1.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/17.1.3) [ Issue #1618](https://github.com/manticoresoftware/manticoresearch/issues/1618) Обновлен Buddy до версии 3.40.7. Поддержка использования Manticore с HikariCP и JPA/MyBatis (Spring Boot) предоставляется через Manticore Buddy, который реализует необходимое поведение протокола MySQL.
* 🪲 [v17.1.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/17.1.2) [ Issue #4128](https://github.com/manticoresoftware/manticoresearch/issues/4128) Исправлен MATCH с OR для одной и той же фразы в разных полях (например, `(@name ="^New York$") | (@ascii_name ="^New York$")`), который возвращал совпадения из других полей; булево упрощение больше не снимает ограничения полей в этом случае.
* 🪲 [v17.1.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/17.1.1) [ Issue #4131](https://github.com/manticoresoftware/manticoresearch/issues/4131) Исправлен сбой `ALTER TABLE` с настройками уровня таблицы (например, `html_strip='1'`) на таблицах с авто-эмбеддингами с ошибкой "knn_dims can't be used together with model_name"; сериализация теперь опускает knn_dims, когда установлен model_name.
* 🪲 [v17.0.12](https://github.com/manticoresoftware/manticoresearch/releases/tag/17.0.12) [ PR #4188](https://github.com/manticoresoftware/manticoresearch/pull/4188) Исправлена некорректная оценка фильтра и дерева фильтров в объединенных запросах.
* 🪲 [v17.0.11](https://github.com/manticoresoftware/manticoresearch/releases/tag/17.0.11) [ Issue #3661](https://github.com/manticoresoftware/manticoresearch/issues/3661) Исправлены периодические сбои при использовании колоночных атрибутов MVA64 (например, во время слияния/оптимизации или при выборе хранимых/колоночных атрибутов), путем исправления обработки данных колоночных MVA64 в docstore и blob-pool.
* 🪲 [v17.0.10](https://github.com/manticoresoftware/manticoresearch/releases/tag/17.0.10) [ Issue #3944](https://github.com/manticoresoftware/manticoresearch/issues/3944) `HIGHLIGHT` и генерация сниппетов теперь поддерживают `REGEX` в строке запроса, поэтому совпадения `REGEX` корректно выделяются, когда тот же запрос `REGEX` используется в `MATCH` и `HIGHLIGHT`.
* 🪲 [v17.0.8](https://github.com/manticoresoftware/manticoresearch/releases/tag/17.0.8) [ Issue #4159](https://github.com/manticoresoftware/manticoresearch/issues/4159) Исправлена функция `HISTOGRAM()`, возвращавшая некорректные значения (например, ноль), когда первый аргумент является выражением с плавающей точкой, таким как `price*100`; гистограмма теперь использует бакетирование с плавающей точкой для аргументов типа float, поэтому результаты выражений группируются правильно.
* 🪲 [v17.0.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/17.0.5) [ Issue #4148](https://github.com/manticoresoftware/manticoresearch/issues/4148) Исправлены распределенные запросы, возвращавшие неправильное хранимое поле, когда агент имеет несколько локальных индексов с дублирующимися идентификаторами документов; хранимое поле теперь берется из совпадающего индекса в соответствии с порядком индексов (первое совпадение побеждает), и поведение задокументировано.
* 🪲 [v17.0.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/17.0.3) [ Issue #4115](https://github.com/manticoresoftware/manticoresearch/issues/4115) `HIGHLIGHT()` и генерация сниппетов теперь поддерживают явный оператор OR (`|`) внутри выражений в кавычках, используя то же преобразование запроса, что и поиск.
* 🪲 [v17.0.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/17.0.1) [ Issue #4118](https://github.com/manticoresoftware/manticoresearch/issues/4118) В простом режиме KNN-атрибуты с model_name (авто-эмбеддинги) больше не требуют явного свойства dims; таблица обслуживается с использованием размерностей модели.
* 🪲 [v16.3.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/16.3.4) [ PR #4121](https://github.com/manticoresoftware/manticoresearch/pull/4121) Улучшена производительность для запросов, использующих хранимые колоночные атрибуты (ранний выход и кэшированные проверки зависимостей атрибутов, снижены накладные расходы читателя docstore), и добавлена команда `DROP CACHE` для очистки кэшей запросов, docstore, skip и вторичных индексов.
* 🪲 [v16.3.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/16.3.3) [ Issue #3928](https://github.com/manticoresoftware/manticoresearch/issues/3928) Исправлена постраничная навигация scroll, когда `ORDER BY` включал строковый столбец: токен scroll теперь применяется корректно и возвращает следующую страницу вместо повторения первой.
* 🪲 [v16.3.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/16.3.2) [ Issue #4040](https://github.com/manticoresoftware/manticoresearch/issues/4040) Исправлена ошибка сегментации во время запуска, когда измерение стека не удавалось в некоторых средах (например, AlmaLinux 10, Docker/VM): мокирование стека теперь сравнивает дельту кадра с оставшимся размером стека вместо использования суммы, которая могла переполниться или быть неверной, поэтому измерение больше не переходит в недопустимое состояние, и searchd больше не падает после предупреждения "Something wrong measuring stack".
* 🪲 [v16.3.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/16.3.1) [ Issue #4062](https://github.com/manticoresoftware/manticoresearch/issues/4062) JOIN-запросы теперь последовательно сообщают об ошибке, когда атрибуты левой таблицы имеют префикс в фильтрах `WHERE`; ранее в некоторых случаях (например, когда правая таблица не имела полнотекстового поля) запрос выполнялся без ошибки и возвращал пустой набор результатов.
* 🪲 [v16.2.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/16.2.6) [ Issue #1827](https://github.com/manticoresoftware/manticoresearch/issues/1827) Зарезервированные ключевые слова в обратных кавычках (например, order, year, facet) теперь принимаются в качестве имен атрибутов и столбцов в `CREATE TABLE` и в выражениях, поэтому таблицы и запросы могут использовать зарезервированные слова в качестве идентификаторов при экранировании обратными кавычками.
* 🪲 [v16.2.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/16.2.5) [ Issue #4107](https://github.com/manticoresoftware/manticoresearch/issues/4107) Исправлен парсер SphinxQL, чтобы выражения, заканчивающиеся именем столбца в обратных кавычках, разбирались корректно вместо вызова синтаксической ошибки.
* 🪲 [v16.2.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/16.2.4) [ Issue #4106](https://github.com/manticoresoftware/manticoresearch/issues/4106) `SHOW CREATE TABLE` теперь выводит имена столбцов, которые являются зарезервированными или специальными (например, knn), в обратных кавычках, чтобы `CREATE TABLE ... LIKE` и повторное выполнение показанного DDL работали, когда исходная таблица имеет такие столбцы.
* 🪲 [v16.2.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/16.2.3) [ Issue #3661](https://github.com/manticoresoftware/manticoresearch/issues/3661) Исправлено количество хранимых атрибутов в docstore во время `ALTER`, чтобы переименование несуществующего `.spds` больше не происходило, и сохранены существующие данные blob в пуле blob при добавлении колоночных атрибутов, чтобы ошибки "Blob offset out of bounds" и сбои запросов/слияний больше не возникали.
* 🪲 [v16.2.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/16.2.1) [ PR #4083](https://github.com/manticoresoftware/manticoresearch/pull/4083) Исправлены некорректные отображения, связанные с диакритическими знаками, в таблицах символов CJK, японского и корейского языков, чтобы базовые и озвученные/диакритические формы символов нормализовались корректно для индексации и поиска.
* 🪲 [v16.1.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/16.1.4) [ PR #4084](https://github.com/manticoresoftware/manticoresearch/pull/4084) Исправлена утечка памяти состояния приемника репликации на узлах-присоединителях, когда не нужно было отправлять файлы; добавлена команда API кластера для очистки состояния recv на присоединителях, чтобы оно освобождалось, когда донор обнаруживает, что все узлы уже синхронизированы.
* 🪲 [v16.1.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/16.1.3) [ Issue #615](https://github.com/manticoresoftware/manticoresearch-buddy/issues/615) Обновлен Buddy до версии 3.40.5. Исправлены ошибки недопустимого JSON в плагине KNN, когда строки результатов содержали поля bigint: строковые значения сериализовались без кавычек (например, строки, похожие на числа, такие как `0000000000`), создавая недопустимый JSON; строковые атрибуты теперь всегда сериализуются как строки JSON в кавычках.
* 🪲 [v16.1.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/16.1.2) [ PR #4077](https://github.com/manticoresoftware/manticoresearch/pull/4077) Исправлен вывод и разбор JSON для больших значений double: вывод набора результатов теперь использует резервный формат, когда значение превышает буфер по умолчанию, а целочисленные литералы, переполняющие int64, разбираются как double вместо некорректных целых чисел.
* 🪲 [v16.1.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/16.1.1) [ Issue #2628](https://github.com/manticoresoftware/manticoresearch/issues/2628) Улучшено восстановление после ошибок KNN-индекса: когда RT-индекс не может загрузить KNN-индекс, частично загруженные данные HNSW теперь очищаются, поэтому демон больше не падает при удалении дискового чанка.
* 🪲 [v16.0.13](https://github.com/manticoresoftware/manticoresearch/releases/tag/16.0.13) [ PR #4076](https://github.com/manticoresoftware/manticoresearch/pull/4076) Добавлена опция `searchd.attr_autoconv_strict` (по умолчанию 0) для управления преобразованием строк в числа при `INSERT`/`REPLACE` в RT-таблицы: при включении недопустимые значения (пустая строка, нечисловые, завершающие символы, переполнение) возвращают ошибки вместо тихого преобразования в 0.
* 🪲 [v16.0.12](https://github.com/manticoresoftware/manticoresearch/releases/tag/16.0.12) [ Issue #1751](https://github.com/manticoresoftware/manticoresearch/issues/1751) Исправлен `must_not` в JSON-поиске, чтобы он вел себя как логическое НЕ (возвращая все документы, которые не соответствуют внутреннему запросу); также исправлен `WHERE NOT ...` в SphinxQL для выражений-фильтров.
* 🪲 [v16.0.11](https://github.com/manticoresoftware/manticoresearch/releases/tag/16.0.11) [ PR #2990](https://github.com/manticoresoftware/manticoresearch/issues/2990) Обновлен Libstemmer до версии, включающей исправление ошибки греческого стеммера (`libstemmer_el`) [#204](https://github.com/snowballstem/snowball/issues/204), предотвращая сбой сервера, который мог происходить с определенным греческим текстом при использовании `morphology='libstemmer_el'` (см. issue [#2888](https://github.com/manticoresoftware/manticoresearch/issues/2888)).
* 🪲 [v16.0.10](https://github.com/manticoresoftware/manticoresearch/releases/tag/16.0.10) Вычисление хранимых атрибутов перемещено на стадию postlimit, где это возможно, что улучшает производительность.
* 🪲 [v16.0.9](https://github.com/manticoresoftware/manticoresearch/releases/tag/16.0.9) [ Issue #3905](https://github.com/manticoresoftware/manticoresearch/issues/3905) Исправлен периодический сбой, когда ротация индекса совпадала с SIGHUP (например, перезагрузка конфигурации) на главном узле: при динамической конфигурации fork() дублировал сокеты, и epoll мог позже сообщить о fd, чьи связанные данные уже были освобождены. Сокеты теперь удаляются из списка интересов epoll с помощью EPOLL_CTL_DEL перед закрытием, предотвращая использование после освобождения в `LazyNetEvents_c::EventTick()`.
* 🪲 [v16.0.8](https://github.com/manticoresoftware/manticoresearch/releases/tag/16.0.8) [ Issue #3418](https://github.com/manticoresoftware/manticoresearch/issues/3418) Обновлено требование к Buddy до версии 3.40.4, которая включает улучшенный формат журналов ошибок обработки: когда запрос завершается неудачей (например, "unknown local table(s) 'index'"), Buddy теперь записывает причину сбоя от демона вместо только "Failed to handle query" без контекста.
* 🪲 [v16.0.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/16.0.7) [ Issue #2281](https://github.com/manticoresoftware/manticoresearch/issues/2281) Устранены несоответствия в именах типов для многозначных атрибутов в перколяционных таблицах: mva и mva64 теперь принимаются как синонимы для multi и multi64 в `CREATE TABLE`, поэтому любое именование может использоваться при определении таблиц (например, `CREATE TABLE t (id bigint, tags mva64, ...) type='pq')`. Вывод схемы (например, `DESC table`) продолжает использовать существующие имена типов для совместимости.
* 🪲 [v16.0.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/16.0.4) [ PR #4047](https://github.com/manticoresoftware/manticoresearch/pull/4047) Обновлена версия MCL до 9.0.1. Исправлена обработка векторов с плавающей точкой KNN, когда block_size равен 1, путем отключения сжатия таблицы в этом случае (сжатие не используется для размера блока 1).
* 🪲 [v16.0.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/16.0.3) [ Issue #4042](https://github.com/manticoresoftware/manticoresearch/issues/4042) Исправлен сбой повторно присоединившегося узла, когда `ALTER CLUSTER ... ADD TABLE` выполнялся на другом узле после того, как присоединитель перезапустился и повторно присоединился.
* 🪲 [v15.1.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/15.1.5) [ Issue #4009](https://github.com/manticoresoftware/manticoresearch/issues/4009) Исправлено использование кэша запросов, когда вторичный индекс был принудительно включен через подсказки; кэш теперь отключается, чтобы принудительный SI всегда учитывался.
* 🪲 [v15.1.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/15.1.4) [ Issue #2591](https://github.com/manticoresoftware/manticoresearch/issues/2591) Исправлен полнотекстовый оператор `NOTNEAR`, чтобы он корректно исключал совпадения, когда правый термин появляется в пределах указанного расстояния либо до, либо после левого термина (симметричное/обратное сопоставление). Ранее NOTNEAR рассматривал правый термин только тогда, когда он появлялся после левого термина, поэтому, например, `d NOTNEAR/3 a` мог некорректно соответствовать документу, содержащему "a b c d". NEAR теперь также возвращает ошибку, когда расстояние равно 0.
* 🪲 [v15.1.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/15.1.3) [ Issue #507](https://github.com/manticoresoftware/manticoresearch-buddy/issues/507) Обновлен Buddy до версии 3.40.3. Исправлены многозапросные запросы, использующие нечеткий поиск (`OPTION fuzzy=1`), за которыми следует show meta на `/cli` и `/sql?mode=raw`.
* 🪲 [v15.1.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/15.1.1) [ Issue #4009](https://github.com/manticoresoftware/manticoresearch/issues/4009) Исправлены некорректные результаты с подсказкой SecondaryIndex и кэшем запросов, сделав SI и кэш запросов взаимоисключающими для каждого запроса и добавив предупреждение, когда принудительный SI игнорируется из-за попадания в кэш.

## Версия 15.1.0
**Выпущено**: 7 декабря 2025

### Рекомендуемые библиотеки
- Рекомендуемая версия [MCL](https://github.com/manticoresoftware/columnar): 9.0.0
- Рекомендуемая версия [Buddy](Installation/Manticore_Buddy.md#Manticore-Buddy): 3.40.2

Если вы следуете [официальному руководству по установке](https://manticoresearch.com/install/), вам не о чем беспокоиться.

## Несовместимые изменения
* ⚠️ [v15.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/15.0.0) [ PR #4003](https://github.com/manticoresoftware/manticoresearch/pull/4003) Обновлено требование к MCL до версии 9.0.0, добавлено хранение с плавающей точкой без сжатия для векторов, изменение размера блока для KNN-векторов и небуферизованное чтение. Это обновление изменяет формат данных. Старые версии MCL не смогут читать новые данные, но новая версия может без проблем читать ваши существующие колоннарные таблицы.

### Новые функции и улучшения
* 🆕 [v15.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/15.1.0) [ PR #3990](https://github.com/manticoresoftware/manticoresearch/pull/3990) Улучшены записи логов сброса чанков на диск с разбивкой общего времени на более ясные части.
* 🆕 [v14.7.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/14.7.0) [ Issue #3860](https://github.com/manticoresoftware/manticoresearch/issues/3860) Обновлено требование к Buddy до версии 3.40.1, включающее улучшение автодополнения: нормализованы символы-разделители биграмм в пробелы и отфильтрованы дублирующиеся или некорректные сочетания предложений для повышения их качества. Также исправлены ошибки invalid JSON в представлениях Kafka и исправлено автодополнение — сортировка сочетаний больше не вызывает ошибок при отсутствии некоторых ключей в score map.
* 🆕 [v14.6.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/14.6.0) [ Issue #615](https://github.com/manticoresoftware/manticoresearch-buddy/issues/615) Обновлено требование к Manticore Buddy до версии 3.39.1, исправляющей ошибки invalid JSON в плагине KNN и позволяющей обработчикам Buddy переопределять HTTP `Content-Type`, так что `/metrics` теперь возвращает текстовый формат Prometheus (`text/plain; version=0.0.4`) вместо JSON, устраняя ошибки при сборе метрик.
* 🆕 [v14.4.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/14.4.0) [ PR #3942](https://github.com/manticoresoftware/manticoresearch/pull/3942) Обновлено требование к Manticore Buddy до 3.38.0, отфильтрованы предложения с нулевым количеством документов, улучшена обработка строковых ключей с Ds\Map, изменён формат отчёта об использовании памяти в Buddy с килобайтов на байты для большей точности, а также повышена производительность, стабильность и поддерживаемость.
* 🆕 [v14.5.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/14.5.0) [ Issue #3329](https://github.com/manticoresoftware/manticoresearch/issues/3329) Удаление лишних пробелов и символов новой строки в JSON-пейлоадах при логировании запросов — пропуск начальных и конечных пробелов, чтобы избежать записи некорректного JSON.
* 🆕 [v14.3.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/14.3.0) [ PR #3932](https://github.com/manticoresoftware/manticoresearch/pull/3932) Улучшена обработка `LOCK TABLES` / `UNLOCK TABLES`: блокировки на запись теперь возвращают предупреждения вместо ошибок, блокировки на чтение корректно отображаются в `SHOW LOCKS`, логика блокировок стала более последовательной.
* 🆕 [v14.2.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/14.2.0) [ Issue #3891](https://github.com/manticoresoftware/manticoresearch/issues/3891) Добавлена поддержка произвольных выражений фильтрации в `JOIN ON` (не только проверок на равенство), позволяющая делать запросы вида `... ON t1.id = t2.t1_id AND t2.value = 5`.

## Исправления ошибок
* 🪲 [v15.0.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/15.0.6) [ Issue #3601](https://github.com/manticoresoftware/manticoresearch/issues/3601) Исправлена регрессия, при которой нативная служба Windows не запускалась при установке с пользовательским путем конфигурации.
* 🪲 [v15.0.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/15.0.5) [ Issue #3864](https://github.com/manticoresoftware/manticoresearch/issues/3864) Исправлена обработка "объединённых полей" в источниках на основе SQL так, чтобы "маркер конца" корректно устанавливался при переборе объединённых результатов.
* 🪲 [v15.0.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/15.0.4) [ Issue #4004](https://github.com/manticoresoftware/manticoresearch/issues/4004) Исправлена регрессия, при которой HTTP-ответы `/sql` от демона некорректно использовали заголовок `Content-Type: text/html` вместо `application/json`.
* 🪲 [v15.0.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/15.0.3) [ Issue #2727](https://github.com/manticoresoftware/manticoresearch/issues/2727) Исправлена проблема, когда группировка через `GROUP BY` / `FACET` по атрибутам, созданным с помощью JSON-to-attribute mapping, не работала.
* 🪲 [v15.0.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/15.0.2) [ Issue #3962](https://github.com/manticoresoftware/manticoresearch/issues/3962) Обновлено требование Buddy до версии 3.40.2, которая добавляет поддержку нечеткого поиска для `/sql` и включает другие исправления для нечеткого поиска.
* 🪲 [v15.0.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/15.0.1) [ PR #3922](https://github.com/manticoresoftware/manticoresearch/pull/3922) Обновлена документация и тесты, связанные с поддержкой Logstash 9.2.
* 🪲 [v14.7.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/14.7.6) [ PR #4002](https://github.com/manticoresoftware/manticoresearch/pull/4002) Исправлено поведение KNN при оверсэмплинге: больше не вычисляется повторно оценённое расстояние KNN, если переоценка не запрашивается, и передаются подсказки по доступу к float-векторам в слой колонного хранения.
* 🪲 [v14.7.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/14.7.5) [ PR #3999](https://github.com/manticoresoftware/manticoresearch/pull/3999) Исправлена модель “test_298” для устранения ошибки в тесте, связанной с KNN.
* 🪲 [v14.7.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/14.7.4) [ Issue #3977](https://github.com/manticoresoftware/manticoresearch/issues/3977) Тесты на Windows иногда создавали дампы с расширением .mdmp — исправлено, теперь "ubertests" не оставляют minidumps после завершения.
* 🪲 [v14.7.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/14.7.3) [ Issue #3832](https://github.com/manticoresoftware/manticoresearch/issues/3832) Исправлена ошибка, при которой мульти-запросы через `/cli_json`, содержащие точку с запятой (например, объединение SQL-запросов), не выполнялись — теперь точки с запятой не заменяются на нулевые символы перед обработкой.
* 🪲 [v14.7.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/14.7.2) [ Issue #1613](https://github.com/manticoresoftware/manticoresearch/issues/1613) Документирована внутренняя 32-битная маска, используемая при подсчёте ранжирующих факторов для операторов Phrase/Proximity/NEAR, и объяснено, как она может недосчитывать термины после 31-го ключевого слова.
* 🪲 [v14.7.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/14.7.1) [ PR #3992](https://github.com/manticoresoftware/manticoresearch/pull/3992) Исправлены отсутствующие сообщения об ошибках для HTTP-запросов UPDATE и DELETE по распределённым таблицам, обрабатываемым агентами, теперь клиенты корректно получают ошибки в случае неудачи операций.
* 🪲 [v14.6.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/14.6.4) [ Issue #3478](https://github.com/manticoresoftware/manticoresearch/issues/3478) Улучшена проверка обновлений: теперь проверяется, что обновляемые атрибуты не конфликтуют с полями полнотекстового поиска; обновления отклоняются при попытке модифицировать полнотекстовые поля.
* 🪲 [v14.6.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/14.6.3) [ Issue #2352](https://github.com/manticoresoftware/manticoresearch/issues/2352) Исправлена внутренняя ошибка при использовании распределённых таблиц с `persistent_connections_limit`.
* 🪲 [v14.6.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/14.6.2) [ Issue #3757](https://github.com/manticoresoftware/manticoresearch/issues/3757) Исправлена ошибка, при которой счётчики состояния таблиц с суффиксом "_sec" (например, `query_time_1min`) фактически показывали миллисекунды вместо секунд.
* 🪲 [v14.6.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/14.6.1) [ Issue #3979](https://github.com/manticoresoftware/manticoresearch/issues/3979) Исправлена ошибка, при которой статистика времени поиска `SHOW INDEX <name> STATUS` (search_stats_ms_*) не совпадала со значениями из логов запросов; теперь отображаются реальные зарегистрированные времена.
* 🪲 [v14.5.8](https://github.com/manticoresoftware/manticoresearch/releases/tag/14.5.8) [ Issue #3517](https://github.com/manticoresoftware/manticoresearch/issues/3517) Обновлена обработка HTTP-заголовков, чтобы интеграции Buddy могли определять или переопределять заголовки, вместо того чтобы всегда навязывать `application/json`.
* 🪲 [v14.5.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/14.5.7) Исправлен тест 226 для обеспечения стабильного порядка результатов, а также исправлена модель GTest для обработки смешанных JSON-массивов.
* 🪲 [v14.5.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/14.5.6) Исправлена сборка Windows путём коррекции несоответствия типов в `binlog.cpp`, теперь `DoSaveMeta()` корректно компилируется для Windows.
* 🪲 [v14.5.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/14.5.5) [Issue #805](https://github.com/manticoresoftware/manticoresearch/issues/805) [Issue #807](https://github.com/manticoresoftware/manticoresearch/issues/807) [Issue #3924](https://github.com/manticoresoftware/manticoresearch/issues/3924) Исправлено неконсистентное поведение фасетирования по JSON-атрибутам: использование псевдонимов JSON-массива в FACET теперь ведёт себя так же, как фасетирование по самому массиву.
* 🪲 [v14.5.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/14.5.4) [ Issue #3927](https://github.com/manticoresoftware/manticoresearch/issues/3927) Исправлен сбой в поиске KNN векторов путём пропуска поиска по пустым индексам HNSW.
* 🪲 [v14.5.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/14.5.2) [ Issue #3669](https://github.com/manticoresoftware/manticoresearch/issues/3669) Исправлен разбор `MATCH()`, теперь пустые группы, например `()`, не вызывают ошибку (например, запросы вида `camera()` теперь работают корректно).
* 🪲 [v14.5.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/14.5.1) [ PR #3961](https://github.com/manticoresoftware/manticoresearch/pull/3961) Обновлена документация по резервному копированию в режиме репликации mysqldump: объяснено использование `--skip-lock-tables` при дампе реплицируемых таблиц и исправлены несколько повреждённых ссылок в руководстве.
* 🪲 [v14.3.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/14.3.2) [ Issue #2772](https://github.com/manticoresoftware/manticoresearch/issues/2772) Исправлена ошибка, когда некоторые команды, выполненные через MySQL клиент 9, вызывали ошибку "unexpected $undefined near '$$'" в логе запросов.
* 🪲 [v14.3.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/14.3.1) [ PR #3934](https://github.com/manticoresoftware/manticoresearch/pull/3934) Обновлено требование Manticore Buddy с 3.37.0 до 3.37.2, убраны избыточные проверки `LOCK/UNLOCK TABLES` (теперь это обрабатывает демон), а также исправлен разбор автодополнения, чтобы запросы с экранированными кавычками (например, `\"` или `\'`) обрабатывались корректно, без ошибок.
* 🪲 [v14.2.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/14.2.1) [ Issue #3602](https://github.com/manticoresoftware/manticoresearch/issues/3602) Исправлен сбой при использовании `knn_dist()` в пользовательском ранжировщике с запросами KNN+MATCH; теперь такие запросы возвращают понятную ошибку вместо сбоя.
* 🪲 [v14.1.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/14.1.1) docs: исправлены мелкие проблемы с переводом.

## Версия 14.1.0

**Выпущено**: 7 ноября 2025 г.

❤️ Мы хотим искренне поблагодарить [@ricardopintottrdata](https://github.com/ricardopintottrdata) за их работу над [PR #3792](https://github.com/manticoresoftware/manticoresearch/pull/3792) и [PR #3828](https://github.com/manticoresoftware/manticoresearch/pull/3828) — решение проблем с подсчетами `HAVING` total и ошибкой `filter with empty name` — а также [@jdelStrother](https://github.com/jdelStrother) за их вклад с [PR #3819](https://github.com/manticoresoftware/manticoresearch/pull/3819), который улучшает обработку `ParseCJKSegmentation`, когда поддержка Jieba недоступна.

Ваши усилия помогают сделать проект сильнее — большое спасибо!

### Рекомендуемые библиотеки
- Рекомендуемая версия [MCL](https://github.com/manticoresoftware/columnar): 8.1.0
- Рекомендуемая версия [Buddy](Installation/Manticore_Buddy.md#Manticore-Buddy): 3.37.0

Если вы следуете [официальному руководству по установке](https://manticoresearch.com/install/), вам не о чем беспокоиться.

### ⚠️ Важно
Версия [v14.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/14.0.0) обновляет протокол репликации. Если вы работаете с кластером репликации, вам нужно:
  - Сначала корректно остановить все ваши узлы
  - Затем запустить последний остановленный узел с параметром `--new-cluster`, используя инструмент `manticore_new_cluster` в Linux.
  - Подробнее о [перезапуске кластера](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster).

### Новые возможности и улучшения
* 🆕 [v14.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/14.1.0) [ Issue #3047](https://github.com/manticoresoftware/manticoresearch/issues/3047) Добавлена поддержка операторов `LOCK TABLES`, генерируемых mysqldump, что улучшает безопасность логических бэкапов.
* ⚠️ [v14.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/14.0.0) [ PR #3896](https://github.com/manticoresoftware/manticoresearch/pull/3896) Добавлен [индикатор прогресса](../Creating_a_cluster/Setting_up_replication/Replication_cluster_status.md#SST-Progress-Metrics) для донорских и присоединяющихся узлов в репликации SST, отображаемый в SHOW STATUS.
* 🆕 [v13.16.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.16.0) [ PR #3894](https://github.com/manticoresoftware/manticoresearch/pull/3894) Обновлен buddy с версии 3.36.1 до 3.37.0, добавляющей опцию "quorum".
* 🆕 [v13.15.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.15.0) [ PR #3842](https://github.com/manticoresoftware/manticoresearch/pull/3842) Опция [force_bigrams](../Searching/Spell_correction.md#Using-force_bigrams-for-better-transposition-handling) для плагинов fuzzy и autocomplete.

### Исправления ошибок
* 🪲 [Issue #4299](https://github.com/manticoresoftware/manticoresearch/issues/4299) Добавлено тестирование для Grafana версии 12.4.
* 🪲 [ Issue #3994](https://github.com/manticoresoftware/manticoresearch/issues/3994) Добавлено тестирование версии Grafana 12.3.
* 🪲 [v14.0.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/14.0.1) [ Issue #3844](https://github.com/manticoresoftware/manticoresearch/issues/3844) Исправлен сбой, вызванный использованием `max(ft field)`.
* 🪲 [v13.15.13](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.15.13) [ PR #3828](https://github.com/manticoresoftware/manticoresearch/pull/3828) Исправлена ошибка при использовании пустого имени фильтра.
* 🪲 [v13.15.12](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.15.12) [ PR #3873](https://github.com/manticoresoftware/manticoresearch/pull/3873) Обновлен buddy с 3.36.0 до 3.36.1 с проверкой режима RT в плагине EmulateElastic.
* 🪲 [v13.15.11](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.15.11) [ PR #3857](https://github.com/manticoresoftware/manticoresearch/pull/3857) Добавлено тестирование версии Filebeat 9.2.
* 🪲 [v13.15.10](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.15.10) [ PR #3880](https://github.com/manticoresoftware/manticoresearch/pull/3880) Протестирован автоматический перевод документации после исправлений.
* 🪲 [v13.15.9](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.15.9) [ Issue #3783](https://github.com/manticoresoftware/manticoresearch/issues/3783) Исправлена проблема, препятствующая нативной компиляции FreeBSD.
* 🪲 [v13.15.8](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.15.8) Исправлены переводы документации.
* 🪲 [v13.15.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.15.7) [ PR #3868](https://github.com/manticoresoftware/manticoresearch/pull/3868) Обновлен executor с 1.3.5 до 1.3.6, добавлена поддержка расширения iconv.
* 🪲 [v13.15.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.15.6) Исправлена проблема сборки фуззера, связанная с [issue 3817](https://github.com/manticoresoftware/manticoresearch/issues/3817).
* 🪲 [v13.15.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.15.5) [ Issue #3644](https://github.com/manticoresoftware/manticoresearch/issues/3644) Исправлен сбой, вызванный определёнными полнотекстовыми запросами.
* 🪲 [v13.15.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.15.4) [ Issue #3686](https://github.com/manticoresoftware/manticoresearch/issues/3686) Исправлена проблема, из-за которой полнотекстовый запрос `"(abc|def)"` не работал как ожидалось.
* 🪲 [v13.15.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.15.3) [ Issue #3428](https://github.com/manticoresoftware/manticoresearch/issues/3428) Добавлена возможность получить общее количество результатов для запросов с использованием `HAVING`.
* 🪲 [v13.15.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.15.2) [ Issue #3817](https://github.com/manticoresoftware/manticoresearch/issues/3817) Добавлена опция [searchd.expansion_phrase_warning](../Server_settings/Searchd.md#expansion_phrase_warning).
* 🪲 [v13.15.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.15.1) [ PR #3848](https://github.com/manticoresoftware/manticoresearch/pull/3848) Исправлена генерация ключей транзакций репликации и обработка конфликтующих транзакций.
* 🪲 [v13.14.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.14.0) [ Issue #3806](https://github.com/manticoresoftware/manticoresearch/issues/3806) Исправлен `CALL SUGGEST`, не совпадавший с триграммами.
* 🪲 [v13.13.8](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.13.8) [ PR #3839](https://github.com/manticoresoftware/manticoresearch/pull/3839) Обновлен buddy с 3.35.4 до 3.35.5 для корректировки регулярного выражения для сопоставления соединений в SQL-запросах.
* 🪲 [v13.13.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.13.7) [ Issue #3815](https://github.com/manticoresoftware/manticoresearch/issues/3815) Обновлен buddy с 3.35.3 до 3.35.4 для исправления проблемы с отрицательными ID в REPLACE.
* 🪲 [v13.13.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.13.6) [ PR #3830](https://github.com/manticoresoftware/manticoresearch/pull/3830) Обновлен buddy с 3.35.1 до 3.35.3.
* 🪲 [v13.13.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.13.5) [ PR #3823](https://github.com/manticoresoftware/manticoresearch/pull/3823) Добавлен интеграционный тест для Grafana.
* 🪲 [v13.13.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.13.4) [ PR #3819](https://github.com/manticoresoftware/manticoresearch/pull/3819) Исправлен `ParseCJKSegmentation` при отсутствии поддержки Jieba.
* 🪲 [v13.13.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.13.3) [ PR #3808](https://github.com/manticoresoftware/manticoresearch/pull/3808) Исправлена обработка ошибок при использовании фильтров в JSON-запросах с правым соединением.
* 🪲 [v13.13.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.13.2) [ PR #3789](https://github.com/manticoresoftware/manticoresearch/pull/3789) Проверены параметры KNN.
* 🪲 [v13.13.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.13.1) [ Issue #3800](https://github.com/manticoresoftware/manticoresearch/issues/3800) Исправлены проблемы сборки при компиляции без поддержки cjk/jieba.

## Версия 13.13.0
**Выпущено**: 7 октября 2025

### Рекомендуемые библиотеки
- Рекомендуемая версия [MCL](https://github.com/manticoresoftware/columnar): 8.1.0
- Рекомендуемая версия [Buddy](Installation/Manticore_Buddy.md#Manticore-Buddy): 3.35.1

Если вы следуете [официальному руководству по установке](https://manticoresearch.com/install/), вам не о чем беспокоиться.

### ⚠️ Важно

Поскольку конфигурационный файл был обновлен, **во время обновления на Linux вы можете увидеть предупреждение с вопросом, сохранить ли вашу версию или использовать новую из пакета**. Если у вас есть пользовательская (не по умолчанию) конфигурация, рекомендуется сохранить вашу версию и обновить путь `pid_file` на `/run/manticore/searchd.pid`. Однако, даже если вы не измените путь, все должно работать нормально.

### Новые функции и улучшения
* 🆕 [v13.13.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.13.0) Добавлена поддержка MCL 8.1.0 с SI блок-кешем.
* 🆕 [v13.12.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.12.0) Реализована опция [secondary_index_block_cache](../Server_settings/Searchd.md#secondary_index_block_cache), обновлено API вторичного индекса и встроены методы доступа к сортировке.

### Исправления ошибок
* 🪲 [v13.11.8](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.11.8) [Проблема #3791](https://github.com/manticoresoftware/manticoresearch/issues/3791) Исправлено состояние гонки между проверкой и вызовом сработавшего таймера.
* 🪲 [v13.11.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.11.7) [Проблема #1045](https://github.com/manticoresoftware/manticoresearch/issues/1045) Исправлено предупреждение systemctl на RHEL 8 при обновлении systemd путем замены устаревшего пути `/var/run/manticore` на правильный `/run/manticore` в конфигурации. Поскольку конфигурационный файл был обновлен, во время обновления вы можете увидеть предупреждение с вопросом, сохранить ли вашу версию или использовать новую из пакета. Если у вас есть пользовательская (не по умолчанию) конфигурация, рекомендуется сохранить вашу версию и обновить путь `pid_file` на `/run/manticore/searchd.pid`.
* 🪲 [v13.11.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.11.6) [PR #3766](https://github.com/manticoresoftware/manticoresearch/pull/3766) Добавлена поддержка версии MCL 8.0.6.
* 🪲 [v13.11.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.11.5) [PR #3767](https://github.com/manticoresoftware/manticoresearch/pull/3767) Улучшены переводы документации на китайский язык и обновлены подмодули.
* 🪲 [v13.11.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.11.4) [PR #3765](https://github.com/manticoresoftware/manticoresearch/pull/3765) Исправлена обработка присвоенных псевдонимов атрибутов соединения.
* 🪲 [v13.11.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.11.3) [PR #3763](https://github.com/manticoresoftware/manticoresearch/pull/3763) Исправлен сбой, который мог возникать при пакетных соединениях по строковым атрибутам, а также устранена проблема, при которой фильтры иногда не работали с LEFT JOIN.
* 🪲 [v13.11.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.11.2) [Проблема #3065](https://github.com/manticoresoftware/manticoresearch/issues/3065) Исправлен сбой при вставке данных в колонковую таблицу с включенным параметром index_field_lengths.
* 🪲 [v13.11.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.11.1) [Проблема #3751](https://github.com/manticoresoftware/manticoresearch/issues/3751) Исправлен сбой, возникающий при удалении документа с включёнными встраиваниями.

## Версия 13.11.1
**Выпущена**: 13 сентября 2025

### Исправления ошибок
* 🪲 [v13.11.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.11.1) [Проблема #3751](https://github.com/manticoresoftware/manticoresearch/issues/3751) Исправлен сбой, возникающий при удалении документа с включёнными встраиваниями.

## Версия 13.11.0
**Выпущена**: 13 сентября 2025

Главной особенностью этого релиза является [Auto Embeddings](Searching/KNN.md#Auto-Embeddings-%28Recommended%29) — новая функция, которая делает семантический поиск таким же простым, как SQL.
Не требуются внешние сервисы или сложные пайплайны: просто вставьте текст и производите поиск на естественном языке.

### Что предлагают Auto Embeddings

- **Автоматическое создание эмбеддингов** напрямую из вашего текста
- **Запросы на естественном языке**, которые понимают смысл, а не только ключевые слова
- **Поддержка нескольких моделей** (OpenAI, Hugging Face, Voyage, Jina)
- **Бесшовная интеграция** с SQL и JSON API

### Рекомендуемые библиотеки
- Рекомендуемая версия [MCL](https://github.com/manticoresoftware/columnar): 8.0.3
- Рекомендуемая версия [Buddy](Installation/Manticore_Buddy.md#Manticore-Buddy): 3.35.1

Если вы следуете [официальному руководству по установке](https://manticoresearch.com/install/), вам не о чем беспокоиться.

Рекомендуемая версия [MCL](https://github.com/manticoresoftware/columnar): 8.0.1
Рекомендуемая версия [Buddy](Installation/Manticore_Buddy.md#Manticore-Buddy): 3.34.2

### Новые функции и улучшения
* 🆕 [v13.11.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.11.0) [PR #3746](https://github.com/manticoresoftware/manticoresearch/pull/3746) Добавлена поддержка "query" в JSON-запросах для генерации эмбеддингов.
* 🆕 [v13.10.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.10.0) [Проблема #3709](https://github.com/manticoresoftware/manticoresearch/issues/3709) RPM-пакет manticore-server больше не владеет `/run`.
* 🆕 [v13.9.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.9.0) [PR #3716](https://github.com/manticoresoftware/manticoresearch/pull/3716) Добавлена поддержка `boolean_simplify` в конфигурации.
* 🆕 [v13.8.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.8.0) [Проблема #3253](https://github.com/manticoresoftware/manticoresearch/issues/3253) Установлена конфигурация sysctl для повышения vm.max_map_count для больших наборов данных.
* 🆕 [v13.7.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.7.0) [PR #3681](https://github.com/manticoresoftware/manticoresearch/pull/3681) Добавлена поддержка `alter table <table> modify column <column> api_key=<key>`.

### Исправление ошибок
* 🪲 [v13.10.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.10.5) [ PR #3737](https://github.com/manticoresoftware/manticoresearch/pull/3737) Опция scroll теперь корректно возвращает все документы с большими 64-битными ID.
* 🪲 [v13.10.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.10.4) [ PR #3736](https://github.com/manticoresoftware/manticoresearch/pull/3736) Исправлен сбой при использовании KNN с фильтр-деревом.
* 🪲 [v13.10.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.10.3) [ Issue #3520](https://github.com/manticoresoftware/manticoresearch/issues/3520) В эндпоинте `/sql` больше нельзя использовать команду SHOW VERSION.
* 🪲 [v13.10.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.10.2) [ PR #3637](https://github.com/manticoresoftware/manticoresearch/pull/3637) Обновлен скрипт установщика для Windows.
* 🪲 [v13.10.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.10.1) Исправлено определение локального часового пояса на Linux.
* 🪲 [v13.9.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.9.2) [ PR #3726](https://github.com/manticoresoftware/manticoresearch/pull/3726) Дублирующиеся ID в колонковом режиме теперь корректно вызывают ошибку.
* 🪲 [v13.9.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.9.1) [ PR #3333](https://github.com/manticoresoftware/manticoresearch/pull/3333) Руководство теперь автоматически переводится.
* 🪲 [v13.8.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.8.6) [ PR #3715](https://github.com/manticoresoftware/manticoresearch/pull/3715) Исправлена ошибка генерации эмбеддингов, когда все документы в пакете были пропущены.
* 🪲 [v13.8.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.8.5) [ PR #3711](https://github.com/manticoresoftware/manticoresearch/pull/3711) Добавлены модели эмбеддингов Jina и Voyage, а также другие автоматические изменения, связанные с эмбеддингами.
* 🪲 [v13.8.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.8.4) [ PR #3710](https://github.com/manticoresoftware/manticoresearch/pull/3710) Исправлен сбой в объединённых запросах с несколькими фасетами.
* 🪲 [v13.8.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.8.3) Исправлена проблема, когда коммиты удаления/обновления в транзакции с несколькими операторами в _bulk эндпоинте не учитывались как ошибки.
* 🪲 [v13.8.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.8.2) [ PR #3705](https://github.com/manticoresoftware/manticoresearch/pull/3705) Исправлен сбой при объединении по неколонковым строковым атрибутам, улучшено отображение ошибок.
* 🪲 [v13.8.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.8.1) [ PR #3704](https://github.com/manticoresoftware/manticoresearch/pull/3704) Исправлен сбой в эмбеддингах запросов при отсутствии указанной модели; добавлена строка эмбеддингов в коммуникацию master-agent; добавлены тесты.
* 🪲 [v13.7.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.7.2) [ PR #Buddy#589](https://github.com/manticoresoftware/manticoresearch-buddy/pull/589) Убрана стандартная уловка с IDF для нечеткого поиска.
* 🪲 [v13.7.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.7.1) [ Issue #3454](https://github.com/manticoresoftware/manticoresearch/issues/3454) Исправлено некорректное декодирование scroll с большими 64-битными ID.
* 🪲 [v13.6.9](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.9) [ Issue #3674](https://github.com/manticoresoftware/manticoresearch/issues/3674) Исправлена проблема с драйвером JDBC+MySQL и пулом соединений при установке transaction_read_only.
* 🪲 [v13.6.8](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.8) [ PR #3676](https://github.com/manticoresoftware/manticoresearch/pull/3676) Исправлен сбой на пустом наборе результатов, возвращаемом моделью эмбеддингов.

## Версия 13.6.7
**Выпущена**: 8 августа 2025 года

Рекомендуемая версия [MCL](https://github.com/manticoresoftware/columnar): 8.0.1  
Рекомендуемая версия [Buddy](Installation/Manticore_Buddy.md#Manticore-Buddy): 3.34.2

Если вы следуете [официальному руководству по установке](https://manticoresearch.com/install/), беспокоиться не о чем.

### Новые функции и улучшения
* 🆕 [v13.6.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.0) [ Issue #2226](https://github.com/manticoresoftware/manticoresearch/issues/2226) Поддержка явного оператора '|' (ИЛИ) в PHRASE, PROXIMITY и QUORUM.
* 🆕 [v13.5.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.5.0) [ PR #3591](https://github.com/manticoresoftware/manticoresearch/pull/3591) Автоматическая генерация эмбеддингов в запросах (Работа в процессе, пока не готово для продакшена).
* 🆕 [v13.4.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.4.0) [ PR #3585](https://github.com/manticoresoftware/manticoresearch/pull/3585) Исправлена логика предпочтения количества потоков buddy из конфигурации buddy_path, если указано, вместо использования значения демона.
* 🆕 [v13.3.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.3.0) [ PR #3577](https://github.com/manticoresoftware/manticoresearch/pull/3577) Поддержка join с локальными распределёнными таблицами.
* 🆕 [#3647](https://github.com/manticoresoftware/manticoresearch/issues/3647) Добавлена поддержка Debian 13 "Trixie"

### Исправления ошибок
* 🪲 [v13.6.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.7) [ Issue #3524](https://github.com/manticoresoftware/manticoresearch/issues/3524) Исправлена проблема с сохранением сгенерированных эмбеддингов в построчном хранении.
* 🪲 [v13.6.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.6) [ Issue #3563](https://github.com/manticoresoftware/manticoresearch/issues/3563) Исправлены ошибки в Sequel Ace и других интеграциях, возникавшие из-за ошибок "unknown sysvar".
* 🪲 [v13.6.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.5) [ Issue #3467](https://github.com/manticoresoftware/manticoresearch/issues/3467) Исправлены ошибки в DBeaver и других интеграциях, возникавшие из-за ошибок "unknown sysvar".
* 🪲 [v13.6.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.4) [ Issue #3524](https://github.com/manticoresoftware/manticoresearch/issues/3524) Исправлена проблема с конкатенацией эмбеддингов из нескольких полей; также исправлена генерация эмбеддингов из запросов.
* 🪲 [v13.6.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.3) [ Issue #3641](https://github.com/manticoresoftware/manticoresearch/issues/3641) Исправлен баг в версии 13.6.0, при котором фраза теряла все ключевые слова в скобках, кроме первого.
* 🪲 [v13.6.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.2) Исправлена утечка памяти в transform_phrase.
* 🪲 [v13.6.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.1) Исправлена утечка памяти в версии 13.6.0.
* 🪲 [v13.5.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.5.2) [ Issue #3651](https://github.com/manticoresoftware/manticoresearch/issues/3651) Исправлены дополнительные проблемы, связанные с фузз-тестированием полнотекстового поиска.
* 🪲 [v13.5.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.5.1) [ Issue #3560](https://github.com/manticoresoftware/manticoresearch/issues/3560) Исправлена ситуация, когда OPTIMIZE TABLE мог зависать бесконечно при работе с данными KNN.
* 🪲 [v13.4.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.4.2) [ Issue #2544](https://github.com/manticoresoftware/manticoresearch/issues/2544) Исправлена проблема, при которой добавление столбца float_vector могло привести к повреждению индексов.
* 🪲 [v13.4.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.4.1) [ Issue #3651](https://github.com/manticoresoftware/manticoresearch/issues/3651) Добавлено фузз-тестирование полнотекстового парсинга и исправлены несколько обнаруженных в ходе тестирования проблем.
* 🪲 [v13.3.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.3.1) [ Issue #3583](https://github.com/manticoresoftware/manticoresearch/issues/3583) Исправлен сбой при использовании сложных булевых фильтров с подсветкой.
* 🪲 [v13.2.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.7) [ Issue #3481](https://github.com/manticoresoftware/manticoresearch/issues/3481) Исправлен сбой при использовании HTTP-обновления, распределённой таблицы и неправильного кластера репликации одновременно.
* 🪲 [v13.2.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.6) [ PR #3567](https://github.com/manticoresoftware/manticoresearch/pull/3567) Обновлена зависимость manticore-backup до версии 1.9.6.
* 🪲 [v13.2.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.5) [ PR #3565](https://github.com/manticoresoftware/manticoresearch/pull/3565) Исправлена настройка CI для улучшения совместимости с Docker-образами.
* 🪲 [v13.2.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.4) Исправлена обработка длинных токенов. Некоторые специальные токены (например, шаблоны регулярных выражений) могли создавать слишком длинные слова, теперь они укорачиваются перед использованием.

## Версия 13.2.3
**Выпущено**: 8 июля 2025

### Критические изменения
* ⚠️ [PR #3586](https://github.com/manticoresoftware/manticoresearch/pull/3586) Снята поддержка Debian 10 (Buster). Debian 10 достиг конца срока поддержки 30 июня 2024 года. Пользователям рекомендуется обновиться до Debian 11 (Bullseye) или Debian 12 (Bookworm).
* ⚠️ [v13.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.0.0) Обновлено API библиотеки KNN для поддержки пустых значений [float_vector](https://manual.manticoresearch.com/Creating_a_table/Data_types#Float-vector). Это обновление не меняет формат данных, но повышает версию API Manticore Search / MCL.
* ⚠️ [v12.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/12.0.0) [ PR #3516](https://github.com/manticoresoftware/manticoresearch/pull/3516) Исправлен баг с неправильными ID исходных и целевых строк при обучении и построении KNN индекса. Это обновление не меняет формат данных, но повышает версию API Manticore Search / MCL.
* ⚠️ [v11.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.0.0) Добавлена поддержка новых функций поиска по векторам KNN, таких как квантование, рескоринг и оверсэмплинг. Эта версия изменяет формат данных KNN и синтаксис SQL [KNN_DIST()](https://manual.manticoresearch.com/Searching/KNN?client=SQL#KNN-vector-search). Новая версия может читать старые данные, но старые версии не смогут читать новый формат.

### Новые возможности и улучшения
* 🆕 [v13.2.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.0) [ PR #3549](https://github.com/manticoresoftware/manticoresearch/issues/3525) Исправлена проблема с `@@collation_database`, вызывавшая несовместимость с некоторыми версиями mysqldump
* 🆕 [v13.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.1.0) [ Issue #3489](https://github.com/manticoresoftware/manticoresearch/issues/3489) Исправлена ошибка в Fuzzy Search, препятствовавшая парсингу некоторых SQL-запросов
* 🆕 [v12.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/12.1.0) [ Issue #3426](https://github.com/manticoresoftware/manticoresearch/issues/3426) Добавлена поддержка операционных систем RHEL 10
* 🆕 [v11.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.1.0) Добавлена поддержка пустых векторов с плавающей точкой в [KNN поиске](https://manual.manticoresearch.com/Searching/KNN#KNN-vector-search)
* 🆕 [v10.2.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.0) [ Issue #3252](https://github.com/manticoresoftware/manticoresearch/issues/3252) [log_level](https://manual.manticoresearch.com/Server_settings/Setting_variables_online#SET) теперь также регулирует уровень логирования Buddy

### Исправленные ошибки
* 🪲 [v13.2.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.3) [ PR #3556](https://github.com/manticoresoftware/manticoresearch/pull/3556) Исправлен парсинг опции "oversampling" в JSON-запросах
* 🪲 [v13.2.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.2) Исправлено логирование крашей в Linux и FreeBSD путём удаления использования Boost stacktrace
* 🪲 [v13.2.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.1) [ Issue #3298](https://github.com/manticoresoftware/manticoresearch/issues/3298) Исправлено логирование крашей при запуске внутри контейнера
* 🪲 [v12.0.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/12.0.2) Повышена точность статистики по таблицам за счёт учёта микросекунд
* 🪲 [v12.0.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/12.0.1) [ PR #3522](https://github.com/manticoresoftware/manticoresearch/pull/3522) Исправлен краш при фасетировании по MVA в объединённом запросе
* 🪲 [v11.0.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.0.3) [ Issue #3502](https://github.com/manticoresoftware/manticoresearch/issues/3502) Исправлен краш, связанный с Квантованием Векторного Поиска
* 🪲 [v11.0.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.0.2) [ Issue #3493](https://github.com/manticoresoftware/manticoresearch/issues/3493) Изменён `SHOW THREADS` для отображения загрузки CPU в виде целого числа
* 🪲 [v11.0.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.0.1) Исправлены пути для колонковых и вторичных библиотек
* 🪲 [v10.2.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.7) Добавлена поддержка MCL 5.0.5, включая исправление имени файла библиотеки эмбеддингов
* 🪲 [v10.2.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.6) [ Issue #3469](https://github.com/manticoresoftware/manticoresearch/issues/3469) Применено дополнительное исправление, связанное с issue #3469
* 🪲 [v10.2.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.4) [ Issue #3469](https://github.com/manticoresoftware/manticoresearch/issues/3469) Исправлена проблема, при которой HTTP-запросы с Bool-запросами возвращали пустые результаты
* 🪲 [v10.2.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.5) Изменено имя файла библиотеки эмбеддингов по умолчанию и добавлены проверки поля 'from' в KNN векторах
* 🪲 [v10.2.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.3) [ PR #3464](https://github.com/manticoresoftware/manticoresearch/pull/3464) Обновлён Buddy до версии 3.30.2, включающей [PR #565 о памяти и логировании ошибок](https://github.com/manticoresoftware/manticoresearch-buddy/pull/565)
* 🪲 [v10.2.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.2) [ Issue #3470](https://github.com/manticoresoftware/manticoresearch/pull/3470) Исправлены фильтры JSON-строк, Null-фильтры и проблемы сортировки в JOIN запросах
* 🪲 [v10.2.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.1) Исправлена ошибка в `dist/test_kit_docker_build.sh`, из-за которой отсутствовал коммит Buddy
* 🪲 [v10.1.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.4) Исправлен краш при группировке по MVA в объединённом запросе
* 🪲 [v10.1.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.3) [ Issue #3434](https://github.com/manticoresoftware/manticoresearch/issues/3434) Исправлена ошибка с фильтрацией пустых строк
* 🪲 [v10.1.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.2) [ PR #3452](https://github.com/manticoresoftware/manticoresearch/pull/3452) Обновлён Buddy до версии 3.29.7, в которой устранены [Buddy #507 - ошибка с мультизапросами при fuzzy search](https://github.com/manticoresoftware/manticoresearch-buddy/issues/507) и [Buddy #561 - исправление метрики rate](https://github.com/manticoresoftware/manticoresearch-buddy/pull/561), необходимо для [Helm Release 10.1.0](https://github.com/manticoresoftware/manticoresearch-helm/releases/tag/manticoresearch-10.1.0)
* 🪲 [v10.1.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.1) Обновлён Buddy до версии 3.29.4, решающий [#3388 - "Неопределённый ключ массива 'Field'"](https://github.com/manticoresoftware/manticoresearch/issues/3388) и [Buddy #547 - layouts='ru' может не работать](https://github.com/manticoresoftware/manticoresearch-buddy/issues/547)

## Версия 10.1.0
**Выпущена**: 9 июня 2025

Эта версия представляет обновление с новыми возможностями, одним изменением, нарушающим обратную совместимость, и многочисленными улучшениями стабильности и исправлениями ошибок. Изменения направлены на расширение возможностей мониторинга, улучшение функций поиска и исправление различных критических проблем, влияющих на стабильность и производительность системы.

**Начиная с версии 10.1.0, поддержка CentOS 7 больше не осуществляется. Пользователям рекомендуется перейти на поддерживаемую операционную систему.**

### Критические изменения
* ⚠️ [v10.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.0.0) [ Issue #540](https://github.com/manticoresoftware/manticoresearch-buddy/issues/540) КРИТИЧЕСКОЕ ИЗМЕНЕНИЕ: установлено значение `layouts=''` по умолчанию для [нечеткого поиска](Searching/Spell_correction.md#Fuzzy-Search)

### Новые функции и улучшения
* 🆕 [v10.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.0) [ Issue #537](https://github.com/manticoresoftware/manticoresearch-buddy/issues/537) Добавлен встроенный [экспортер Prometheus](Node_info_and_management/Node_status.md#Prometheus-Exporter)
* 🆕 [v9.8.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.8.0) [ Issue #3409](https://github.com/manticoresoftware/manticoresearch/issues/3409) Добавлена команда [ALTER TABLE tbl REBUILD KNN](Updating_table_schema_and_settings.md#Rebuilding-a-KNN-index)
* 🆕 [v9.7.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.7.0) [ Issue #1778](https://github.com/manticoresoftware/manticoresearch/issues/1778) Добавлена автоматическая генерация эмбеддингов (пока не объявлено официально, так как код в основной ветке, но требует дополнительного тестирования)
* 🆕 [v9.6.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.6.0) Обновлена версия API KNN для поддержки авто-эмбеддингов
* 🆕 [v9.5.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.0) [ Issue #1894](https://github.com/manticoresoftware/manticoresearch/issues/1894) Улучшено восстановление кластера: периодическое сохранение `seqno` для более быстрого перезапуска узла после сбоя
* 🆕 [v9.4.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.4.0) [ Issue #2400](https://github.com/manticoresoftware/manticoresearch/issues/2400) Добавлена поддержка последних версий [Logstash](Integration/Logstash.md#Integration-with-Logstash) и Beats

### Исправления ошибок
* 🪲 [v10.0.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.0.1)  Исправлена обработка форм слов: пользовательские формы теперь переопределяют автоматически сгенерированные; добавлены тестовые случаи в тест 22
* 🪲 [v9.8.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.8.2)  Исправление: снова обновлён deps.txt для включения исправлений упаковки в MCL, связанных с библиотекой embeddings
* 🪲 [v9.8.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.8.1)  Исправление: обновлён deps.txt с исправлениями упаковки для MCL и библиотеки embeddings
* 🪲 [v9.7.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.7.3) [ Issue #3306](https://github.com/manticoresoftware/manticoresearch/issues/3306) Исправлен сбой с сигналом 11 при индексации
* 🪲 [v9.7.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.7.2) [ Issue #3109](https://github.com/manticoresoftware/manticoresearch/issues/3109) Исправлена проблема, когда несуществующие `@@variables` всегда возвращали 0
* 🪲 [v9.7.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.7.1) [ Issue #3377](https://github.com/manticoresoftware/manticoresearch/issues/3377) Исправлен сбой, связанный с [remove_repeats()](Functions/Searching_and_ranking_functions.md#REMOVE_REPEATS%28%29)
* 🪲 [v9.6.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.6.3) [ PR #3411](https://github.com/manticoresoftware/manticoresearch/pull/3411) Исправление: использование динамического определения версий для телеметрических метрик
* 🪲 [v9.6.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.6.2)  Исправление: небольшая корректировка вывода в [SHOW VERSION](Node_info_and_management/SHOW_VERSION.md#SHOW-VERSION)
* 🪲 [v9.6.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.6.1)  Исправление: сбой при создании таблицы с KNN-атрибутом без модели
* 🪲 [v9.5.16](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.16) [ Issue #3342](https://github.com/manticoresoftware/manticoresearch/issues/3342) Исправлена проблема, когда `SELECT ... FUZZY=0` не всегда отключал нечёткий поиск
* 🪲 [v9.5.15](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.15) [ PR #3397](https://github.com/manticoresoftware/manticoresearch/pull/3397) Добавлена поддержка [MCL](https://github.com/manticoresoftware/columnar) 4.2.2; исправлены ошибки с более старыми форматами хранения
* 🪲 [v9.5.14](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.14) [ Issue #3392](https://github.com/manticoresoftware/manticoresearch/issues/3392) Исправлена некорректная обработка строк в HTTP JSON ответах
* 🪲 [v9.5.13](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.13) [ Issue #3356](https://github.com/manticoresoftware/manticoresearch/issues/3356) Исправлен сбой в сложных случаях полнотекстового запроса (common-sub-term)
* 🪲 [v9.5.12](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.12)  Исправлена опечатка в сообщении об ошибке автосброса дискового чанка
* 🪲 [v9.5.11](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.11) [ Issue #3195](https://github.com/manticoresoftware/manticoresearch/issues/3195) Улучшен [автосброс дисковых чанков](Securing_and_compacting_a_table/Flushing_RAM_chunk_to_disk.md#Flushing-RAM-chunk-to-disk): пропуск сохранения, если идёт оптимизация
* 🪲 [v9.5.10](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.10) [ Issue #3313](https://github.com/manticoresoftware/manticoresearch/issues/3313) Исправлена проверка на дубликат ID для всех дисковых чанков в RT таблице с использованием [indextool](Miscellaneous_tools.md#indextool)
* 🪲 [v9.5.9](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.9) [ Issue #3132](https://github.com/manticoresoftware/manticoresearch/issues/3132) Добавлена поддержка сортировки `_random` в JSON API
* 🪲 [v9.5.8](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.8) [ Issue #3382](https://github.com/manticoresoftware/manticoresearch/issues/3382) Исправлена проблема: нельзя было использовать uint64 ID документа через JSON HTTP API
* 🪲 [v9.5.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.7) [ Issue #3385](https://github.com/manticoresoftware/manticoresearch/issues/3385) Исправлены некорректные результаты при фильтрации по `id != value`
* 🪲 [v9.5.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.6) [ PR #538](https://github.com/manticoresoftware/manticoresearch-buddy/pull/538) Исправлена критическая ошибка с нечётким сопоставлением в некоторых случаях
* 🪲 [v9.5.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.5) [ Issue #3199](https://github.com/manticoresoftware/manticoresearch/issues/3199) Исправлено декодирование пробелов в параметрах HTTP-запроса Buddy (например, `%20` и `+`)
* 🪲 [v9.5.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.4) [ Issue #3133](https://github.com/manticoresoftware/manticoresearch/issues/3133) Исправлена некорректная сортировка по `json.field` в фасетном поиске
* 🪲 [v9.5.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.3) [ Issue #3091](https://github.com/manticoresoftware/manticoresearch/issues/3091) Исправлены несоответствующие результаты поиска для разделителей в SQL и JSON API
* 🪲 [v9.5.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.2) [ Issue #2819](https://github.com/manticoresoftware/manticoresearch/issues/2819) Улучшена производительность: заменён `DELETE FROM` на `TRUNCATE` для распределённых таблиц
* 🪲 [v9.5.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.1) [ Issue #3080](https://github.com/manticoresoftware/manticoresearch/issues/3080) Исправлен сбой при фильтрации алиасного `geodist()` с JSON-атрибутами

## Версия 9.3.2  
Выпущена: 2 мая 2025 года

Этот релиз включает множество исправлений ошибок и улучшений стабильности, лучшее отслеживание использования таблиц и улучшения управления памятью и ресурсами.

❤️ Особая благодарность [@cho-m](https://github.com/cho-m) за исправление совместимости сборки с Boost 1.88.0 и [@benwills](https://github.com/benwills) за улучшение документации по `stored_only_fields`.

* 🪲 [v9.3.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.3.2) Исправлена проблема, при которой в столбце "Show Threads" активность ЦП отображалась в виде числа с плавающей точкой вместо строки; также исправлена ошибка разбора набора результатов PyMySQL, вызванная неправильным типом данных.
* 🪲 [v9.3.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.3.1) [ Issue #3343](https://github.com/manticoresoftware/manticoresearch/issues/3343) Исправлено оставшиеся файлы `tmp.spidx`, когда процесс оптимизации был прерван.
* 🆕 [v9.3.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.3.0) [ PR #3337](https://github.com/manticoresoftware/manticoresearch/pull/3337) Добавлен счетчик команд по каждой таблице и подробная статистика использования таблиц.
* 🪲 [v9.2.39](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.39) [ Issue #3236](https://github.com/manticoresoftware/manticoresearch/issues/3236) Исправление: предотвращение повреждения таблиц путем удаления сложных обновлений чанков. Использование wait-функций внутри последовательного воркера нарушало последовательную обработку, что могло повредить таблицы.
	Реализован заново автосброс. Удалена внешняя очередь опроса для избежания ненужных блокировок таблиц. Добавлено условие "малой таблицы": если количество документов ниже 'малого лимита таблицы' (8192) и не используется Вторичный индекс (SI), сброс пропускается.

* 🪲 [v9.2.38](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.38) Исправление: пропуск создания Вторичного индекса (SI) для фильтров, использующих `ALL`/`ANY` на списках строк, без влияния на атрибуты JSON.
* 🪲 [v9.2.37](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.37) [ Issue #2898](https://github.com/manticoresoftware/manticoresearch/issues/2898) Добавлена поддержка обратных кавычек для системных таблиц.
* 🪲 [v9.2.36](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.36) Исправление: использование заполнителя для операций с кластерами в устаревшем коде. В парсере теперь явно разделяются поля для имен таблиц и кластеров.
* 🪲 [v9.2.35](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.35) Исправление: сбой при удалении кавычек с одиночной `'`.
* 🪲 [v9.2.34](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.34) [ Issue #3090](https://github.com/manticoresoftware/manticoresearch/issues/3090) Исправление: обработка больших идентификаторов документов (ранее могла не находить их).
* 🪲 [v9.2.33](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.33) Исправление: использование беззнаковых целых чисел для размеров битовых векторов.
* 🪲 [v9.2.32](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.32) Исправление: снижение пикового использования памяти при слиянии. Поиск соответствия docid-коду строки теперь использует 12 байт на документ вместо 16 байт. Например: 24 ГБ ОЗУ для 2 миллиардов документов вместо 36 ГБ.
* 🪲 [v9.2.31](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.31) [ Issue #3238](https://github.com/manticoresoftware/manticoresearch/issues/3238) Исправление: неправильное значение `COUNT(*)` в больших таблицах реального времени.
* 🪲 [v9.2.30](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.30) Исправление: неопределенное поведение при обнулении строковых атрибутов.
* 🪲 [v9.2.29](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.29) Небольшое исправление: улучшен текст предупреждения.
* 🪲 [v9.2.28](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.28) [ Issue #3290](https://github.com/manticoresoftware/manticoresearch/issues/3290) Улучшение: усовершенствован `indextool --buildidf`
* 🪲 [v9.2.27](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.27) [ Issue #3032](https://github.com/manticoresoftware/manticoresearch/issues/3032) С интеграцией Kafka теперь можно создавать источник для конкретного раздела Kafka.
* 🪲 [v9.2.26](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.26) [ Issue #3301](https://github.com/manticoresoftware/manticoresearch/issues/3301) Исправление: `ORDER BY` и `WHERE` по `id` могли вызвать ошибки OOM (Out Of Memory).
* 🪲 [v9.2.25](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.25) [ Issue #3171](https://github.com/manticoresoftware/manticoresearch/issues/3171) Исправление: сбой из-за ошибки сегментации при использовании grouper с несколькими JSON-атрибутами в RT таблице с несколькими дисковыми чанками
* 🪲 [v9.2.24](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.24) [ Issue #3246](https://github.com/manticoresoftware/manticoresearch/issues/3246) Исправление: запросы `WHERE string ANY(...)` не срабатывали после сброса RAM-чанка.
* 🪲 [v9.2.23](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.23) [ PR #518](https://github.com/manticoresoftware/manticoresearch-buddy/pull/518) Небольшие улучшения синтаксиса авто-шардинга.
* 🪲 [v9.2.22](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.22) [ Issue #2763](https://github.com/manticoresoftware/manticoresearch/issues/2763) Исправление: глобальный idf-файл не загружался при использовании `ALTER TABLE`.
* 🪲 [v9.2.21](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.21) Исправление: глобальные idf-файлы могут быть большими. Теперь мы раньше освобождаем таблицы, чтобы избежать удержания ненужных ресурсов.
* 🪲 [v9.2.20](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.20) [ PR #3277](https://github.com/manticoresoftware/manticoresearch/pull/3277) Улучшение: лучшая проверка опций шардинга.

* 🪲 [v9.2.19](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.19) [ PR #3275](https://github.com/manticoresoftware/manticoresearch/pull/3275) Исправление: совместимость сборки с Boost 1.88.0.
* 🪲 [v9.2.18](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.18) [ Issue #3228](https://github.com/manticoresoftware/manticoresearch/issues/3228) Исправление: сбой при создании распределенной таблицы (ошибка с недопустимым указателем).

* 🪲 [v9.2.17](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.17) [ PR #3272](https://github.com/manticoresoftware/manticoresearch/pull/3272) Исправление: проблема с многострочным нечетким `FACET`.
* 🪲 [v9.2.16](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.16) [ Issue #3063](https://github.com/manticoresoftware/manticoresearch/issues/3063) Исправление: ошибка в расчете расстояния при использовании функции `GEODIST`.
* 🪲 [v9.2.15](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.15) [ Issue #3027](https://github.com/manticoresoftware/manticoresearch/issues/3027) Незначительное улучшение: поддержка формата фильтра `query_string` от Elastic.

## Версия 9.2.14
Выпущена: 28 марта 2025

### Незначительные изменения
* [Коммит ](https://github.com/manticoresoftware/manticoresearch/commit/fe9473dc3f4fcfa0eaae748b538fe853f4ce078b) Реализован флаг `--mockstack` для расчета требований к стеку рекурсивных операций. Новый режим `--mockstack` анализирует и сообщает о необходимых размерах стека для рекурсивной оценки выражений, операций сопоставления с образцом, обработки фильтров. Рассчитанные требования к стеку выводятся в консоль для целей отладки и оптимизации.
* [Issue #3058](https://github.com/manticoresoftware/manticoresearch/pull/3058) Параметр [boolean_simplify](Searching/Options.md#boolean_simplify) включен по умолчанию.
* [Issue #3172](https://github.com/manticoresoftware/manticoresearch/issues/3172) Добавлена новая опция конфигурации: `searchd.kibana_version_string`, которая может быть полезна при использовании Manticore с определенными версиями Kibana или OpenSearch Dashboards, которые ожидают конкретную версию Elasticsearch.
* [Issue #3211](https://github.com/manticoresoftware/manticoresearch/issues/3211) Исправлена работа [CALL SUGGEST](Searching/Spell_correction.md#CALL-QSUGGEST,-CALL-SUGGEST) со словами из 2 символов.
* [Issue #490](https://github.com/manticoresoftware/manticoresearch-buddy/issues/490) Улучшен [нечеткий поиск](Searching/Spell_correction.md#Fuzzy-Search): ранее он иногда не мог найти "defghi" при поиске "def ghi", если существовал другой подходящий документ.
* ⚠️ КРИТИЧЕСКОЕ ИЗМЕНЕНИЕ [Issue #3165](https://github.com/manticoresoftware/manticoresearch/issues/3165) Изменено `_id` на `id` в некоторых HTTP JSON ответах для единообразия. Убедитесь, что обновили ваше приложение соответствующим образом.
* ⚠️ КРИТИЧЕСКОЕ ИЗМЕНЕНИЕ [Issue #3186](https://github.com/manticoresoftware/manticoresearch/issues/3186) Добавлена проверка `server_id` при присоединении к кластеру, чтобы гарантировать уникальность ID каждого узла. Операция `JOIN CLUSTER` теперь может завершиться ошибкой с сообщением о дублировании [server_id](Server_settings/Searchd.md#server_id), когда присоединяющийся узел имеет тот же `server_id`, что и существующий узел в кластере. Чтобы решить эту проблему, убедитесь, что каждый узел в репликационном кластере имеет уникальный [server_id](Server_settings/Searchd.md#server_id). Вы можете изменить значение по умолчанию для [server_id](Server_settings/Searchd.md#server_id) в разделе "searchd" вашего конфигурационного файла на уникальное значение перед попыткой присоединения к кластеру. Это изменение обновляет протокол репликации. Если вы используете репликационный кластер, вам необходимо:
  - Сначала корректно остановить все ваши узлы
  - Затем запустить узел, который был остановлен последним, с опцией `--new-cluster`, используя инструмент `manticore_new_cluster` в Linux.
  - Прочтите о [перезапуске кластера](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster) для получения более подробной информации.

### Исправления ошибок
* [Коммит 6fda](https://github.com/manticoresoftware/manticoresearch/commit/6fdad3923dd6953f4b781943eed5ec5c28b7f808) Исправлен сбой, вызванный потерей планировщика после ожидания; теперь специфические планировщики, такие как `serializer`, корректно восстанавливаются.
* [Коммит c333](https://github.com/manticoresoftware/manticoresearch/commit/c333b277d4a504de5fe298e9ab570a9f9ea31e0a) Исправлена ошибка, при которой веса из правой присоединенной таблицы не могли использоваться в предложении `ORDER BY`.
* [Issue #2644](https://github.com/manticoresoftware/manticoresearch/issues/2644) gcc 14.2.0: исправлена ошибка вызова `lower_bound` с `const knn::DocDist_t*&`. ❤️ Спасибо [@Azq2](https://github.com/Azq2) за PR.
* [Issue #3018](https://github.com/manticoresoftware/manticoresearch/issues/3018) Исправлена проблема с обработкой имен таблиц в верхнем регистре при автоматических вставках схемы.
* [Issue #3119](https://github.com/manticoresoftware/manticoresearch/issues/3119) Исправлен сбой при декодировании некорректного ввода base64.
* [Issue #3121](https://github.com/manticoresoftware/manticoresearch/issues/3121) Исправлены две связанные проблемы с KNN индексом при `ALTER`: векторы с плавающей точкой теперь сохраняют свои исходные размерности, и KNN индексы теперь корректно генерируются.
* [Issue #3123](https://github.com/manticoresoftware/manticoresearch/issues/3123) Исправлен сбой при построении вторичного индекса на пустой JSON колонке.
* [Issue #3138](https://github.com/manticoresoftware/manticoresearch/issues/3138) Исправлен сбой, вызванный дублирующимися записями.
* [Issue #3151](https://github.com/manticoresoftware/manticoresearch/issues/3151) Исправлено: опция `fuzzy=1` не могла использоваться с `ranker` или `field_weights`.
* [Issue #3163](https://github.com/manticoresoftware/manticoresearch/issues/3163) Исправлена ошибка, при которой `SET GLOBAL timezone` не имел эффекта.
* [Issue #3181](https://github.com/manticoresoftware/manticoresearch/issues/3181) Исправлена проблема, при которой значения текстовых полей могли теряться при использовании ID больше 2^63.
* [Issue #3189](https://github.com/manticoresoftware/manticoresearch/issues/3189) Исправлено: операторы `UPDATE` теперь корректно учитывают настройку `query_log_min_msec`.
* [Issue #3247](https://github.com/manticoresoftware/manticoresearch/issues/3247) Исправлено состояние гонки при сохранении дисковых чанков реального времени, которое могло приводить к сбою `JOIN CLUSTER`.


## Версия 7.4.6
Выпущена: 28 февраля 2025

### Основные изменения
* [Issue #832](https://github.com/manticoresoftware/manticoresearch/issues/832) Интеграция с [Kibana](Integration/Kibana.md) для более простой и эффективной визуализации данных.

### Незначительные изменения
* [Issue #1727](https://github.com/manticoresoftware/manticoresearch/issues/1727) Исправлены различия в точности чисел с плавающей запятой между arm64 и x86_64.
* [Issue #2995](https://github.com/manticoresoftware/manticoresearch/issues/2995) Реализованы оптимизации производительности для пакетной обработки join.
* [Issue #3039](https://github.com/manticoresoftware/manticoresearch/issues/3039) Реализованы оптимизации производительности для EstimateValues в гистограммах.
* [Issue #3099](https://github.com/manticoresoftware/manticoresearch/issues/3099) Добавлена поддержка Boost 1.87.0. ❤️ Спасибо, [@cho-m](https://github.com/cho-m) за PR.
* [Issue #77](https://github.com/manticoresoftware/columnar/issues/77) Оптимизировано повторное использование блоков данных при создании фильтров с несколькими значениями; добавлены min/max в метаданные атрибутов; реализована предварительная фильтрация значений фильтра на основе min/max.

### Исправления ошибок
* [Commit 73ac](https://github.com/manticoresoftware/manticoresearch/commit/73ac22f358a7a0e734b332c3943c86e6294c06c3) Исправлена обработка выражений в объединённых запросах, когда используются атрибуты из обеих таблиц (левой и правой); исправлена опция index_weights для правой таблицы.
* [Issue #2915](https://github.com/manticoresoftware/manticoresearch/issues/2915) Использование `avg()` в запросе `SELECT ... JOIN` могло приводить к некорректным результатам; теперь исправлено.
* [Issue #2996](https://github.com/manticoresoftware/manticoresearch/issues/2996) Исправлен некорректный набор результатов из-за неявного ограничения при включенной пакетной обработке join.
* [Issue #3031](https://github.com/manticoresoftware/manticoresearch/issues/3031) Исправлен сбой демона при завершении работы, если была активна слияние чанков.
* [Issue #3037](http://github.com/manticoresoftware/manticoresearch/issues/3037) Исправлена проблема, когда `IN(...)` мог возвращать некорректные результаты.
* [Issue #3038](https://github.com/manticoresoftware/manticoresearch/issues/3038) Установка `max_iops` / `max_iosize` в версии 7.0.0 могла ухудшать производительность индексирования; теперь исправлено.
* [Issue #3042](https://github.com/manticoresoftware/manticoresearch/issues/3042) Исправлена утечка памяти в кеше запросов join.
* [Issue #3052](https://github.com/manticoresoftware/manticoresearch/issues/3052) Исправлена обработка опций запросов в объединённых JSON-запросах.
* [Issue #3054](https://github.com/manticoresoftware/manticoresearch/issues/3054) Исправлена проблема с командой ATTACH TABLE.
* [Issue #3079](https://github.com/manticoresoftware/manticoresearch/issues/3079) Исправлены несоответствия в сообщениях об ошибках.
* [Issue #3087](https://github.com/manticoresoftware/manticoresearch/issues/3087) Установка `diskchunk_flush_write_timeout=-1` для каждой таблицы не отключала сброс индекса; теперь исправлено.
* [Issue #3088](https://github.com/manticoresoftware/manticoresearch/issues/3088) Устранены дублирующие записи после массовой замены больших ID.
* [Issue #3126](https://github.com/manticoresoftware/manticoresearch/issues/3126) Исправлен сбой демона, вызванный полнотекстовым запросом с одним оператором `NOT` и ранжировщиком выражений.
* [Issue #3128](https://github.com/manticoresoftware/manticoresearch/pull/3128) Исправлена потенциальная уязвимость в библиотеке CJSON. ❤️ Спасибо, [@tabudz](https://github.com/tabudz) за PR.

## Версия 7.0.0
Выпущена: 30 января 2025 г.

### Основные изменения
* [Issue #1497](https://github.com/manticoresoftware/manticoresearch/issues/1497) Добавлен новый функционал [Нечёткого поиска](Searching/Spell_correction.md#Fuzzy-Search) и [Автозаполнения](Searching/Autocomplete.md#CALL-AUTOCOMPLETE) для упрощения поиска.
* [Issue #1500](https://github.com/manticoresoftware/manticoresearch/issues/1500) [Интеграция с Kafka](Integration/Kafka.md#Syncing-from-Kafka).
* [Issue #1928](https://github.com/manticoresoftware/manticoresearch/issues/1928) Введены [вторичные индексы для JSON](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#json_secondary_indexes).
* [Issue #2361](https://github.com/manticoresoftware/manticoresearch/issues/2361) Обновления и поиск во время обновлений больше не блокируются слиянием чанков.
* [Issue #2787](https://github.com/manticoresoftware/manticoresearch/issues/2787) Автоматический [сброс дисковых чанков](Server_settings/Searchd.md#diskchunk_flush_write_timeout) для RT-таблиц для улучшения производительности; теперь мы автоматически сбрасываем RAM-чанки в дисковые чанки, предотвращая проблемы с производительностью из-за отсутствия оптимизаций в RAM-чанках, что иногда могло приводить к нестабильности в зависимости от размера чанка.
* [Issue #2811](https://github.com/manticoresoftware/manticoresearch/issues/2811) Опция [Scroll](Searching/Pagination.md#Scroll-Search-Option) для упрощения постраничного вывода.
* [Issue #931](https://github.com/manticoresoftware/manticoresearch/issues/931) Интеграция с [Jieba](https://github.com/fxsjy/jieba) для улучшенной [токенизации китайского языка](Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md).

### Небольшие изменения
* ⚠️ BREAKING [Issue #1111](https://github.com/manticoresoftware/manticoresearch/issues/1111) Исправлена поддержка `global_idf` в RT таблицах. Требуется пересоздание таблицы.
* ⚠️ BREAKING [Issue #2103](https://github.com/manticoresoftware/manticoresearch/issues/2103) Удалены тайские символы из внутреннего `cjk` набора символов. Обновите определения вашего набора символов соответственно: если у вас `cjk,non_cjk` и тайские символы важны для вас, измените его на `cjk,thai,non_cjk`, или `cont,non_cjk`, где `cont` — это новое обозначение для всех языков с непрерывным письмом (т.е. `cjk` + `thai`). Измените существующие таблицы с помощью [ALTER TABLE](Updating_table_schema_and_settings.md#Updating-table-FT-settings-in-RT-mode).
* ⚠️ BREAKING [Issue #2468](https://github.com/manticoresoftware/manticoresearch/issues/2468) [CALL SUGGEST / QSUGGEST](Searching/Spell_correction.md#CALL-QSUGGEST,-CALL-SUGGEST) теперь совместимы с распределенными таблицами. Это увеличивает версию протокола master/agent. Если вы запускаете Manticore Search в распределенной среде с несколькими экземплярами, сначала обновите агенты, затем мастеров.
* ⚠️ BREAKING [Issue #2889](https://github.com/manticoresoftware/manticoresearch/issues/2889) Изменено имя столбца с `Name` на `Variable name` для PQ [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META-for-PQ-tables).
* ⚠️ BREAKING [Issue #879](https://github.com/manticoresoftware/manticoresearch/issues/879) Введен [переменный binlog для таблиц](Logging/Binary_logging.md#Per-table-binary-logging-configuration) с новыми опциями: [binlog_common](Logging/Binary_logging.md#Binary-logging-strategies), [binlog](Logging/Binary_logging.md#Per-table-binary-logging-configuration) для `create table` / `alter table`. Вам необходимо выполнить чистое завершение работы экземпляра Manticore перед обновлением до новой версии.
* ⚠️ BREAKING [Issue #1789](https://github.com/manticoresoftware/manticoresearch/issues/1789) Исправлено неправильное сообщение об ошибке, когда узел присоединяется к кластеру с неправильной версией протокола репликации. Это изменение обновляет протокол репликации. Если вы запускаете кластер репликации, вам необходимо:
  - Сначала аккуратно остановить все ваши узлы
  - Затем запустить узел, который был остановлен последним, с `--new-cluster`, используя инструмент `manticore_new_cluster` в Linux.
  - Ознакомьтесь с [перезапуском кластера](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster) для получения дополнительных сведений.
* ⚠️ BREAKING [Issue #2308](https://github.com/manticoresoftware/manticoresearch/issues/2308) Добавлена поддержка нескольких таблиц в [`ALTER CLUSTER ADD` и `DROP`](Creating_a_cluster/Setting_up_replication/Adding_and_removing_a_table_from_a_replication_cluster.md#Adding-and-removing-a-table-from-a-replication-cluster). Это изменение также затрагивает протокол репликации. Обратитесь к предыдущему разделу за рекомендациями по обработке этого обновления.
* [Issue #2997](https://github.com/manticoresoftware/manticoresearch/issues/2997) Исправлена проблема с dlopen на Macos.
* [Commit 4954](https://github.com/manticoresoftware/manticoresearch/commit/4954b5de7341a29902a9b8fbb9a040f7942c77c4) Изменен стандартный порог для OPTIMIZE TABLE на таблицах с индексами KNN для улучшения производительности поиска.
* [Commit cfc8](https://github.com/manticoresoftware/manticoresearch/commit/cfc87ecb6e33a8163c2235243b6b40e699dbf526) Добавлена поддержка `COUNT(DISTINCT)` для `ORDER BY` в `FACET` и `GROUP BY`.
* [Issue #1103](https://github.com/manticoresoftware/manticoresearch/issues/1103) Улучшена ясность в [логировании](Logging/Server_logging.md#Server-logging) слияния чанков.
* [Issue #1130](https://github.com/manticoresoftware/manticoresearch/issues/1130) Добавлена поддержка [DBeaver](Integration/DBeaver.md).
* [Issue #1546](https://github.com/manticoresoftware/manticoresearch/issues/1546) Реализованы вторичные индексы для функций [POLY2D()](Functions/Geo_spatial_functions.md#POLY2D%28%29)/[GEOPOLY2D()](Functions/Geo_spatial_functions.md#GEOPOLY2D%28%29).
* [Issue #1630](https://github.com/manticoresoftware/manticoresearch/issues/1630) HTTP-запросы теперь поддерживают `Content-Encoding: gzip`.
* [Issue #1831](https://github.com/manticoresoftware/manticoresearch/issues/1831) Добавлена команда `SHOW LOCKS`.
* [Issue #2187](https://github.com/manticoresoftware/manticoresearch/issues/2197) Разрешен запрос Buddy к демону для обхода ограничения [searchd.max_connections](Server_settings/Searchd.md#max_connections).
* [Issue #2208](https://github.com/manticoresoftware/manticoresearch/issues/2208) Добавлена поддержка объединения таблиц через JSON HTTP интерфейс.
* [Issue #2235](https://github.com/manticoresoftware/manticoresearch/issues/2235) Логирование успешно обработанных запросов через Buddy в их оригинальной форме.
* [Issue #2249](https://github.com/manticoresoftware/manticoresearch/issues/2249) Добавлен специальный режим для выполнения `mysqldump` для реплицированных таблиц.
* [Issue #2268](https://github.com/manticoresoftware/manticoresearch/issues/2268) Улучшено переименование внешних файлов при копировании для операторов `CREATE TABLE` и `ALTER TABLE`.
* [Issue #2402](https://github.com/manticoresoftware/manticoresearch/issues/2402) Обновлено значение по умолчанию для [searchd.max_packet_size](Server_settings/Searchd.md#max_packet_size) до 128 МБ.
* [Issue #2419](https://github.com/manticoresoftware/manticoresearch/issues/2419) Добавлена поддержка [модификатора IDF boost](Searching/Full_text_matching/Operators.md#IDF-boost-modifier) в JSON ["match"](Searching/Full_text_matching/Basic_usage.md#match).
* [Issue #2430](https://github.com/manticoresoftware/manticoresearch/issues/2430) Улучшена синхронизация записи [binlog](Logging/Binary_logging.md#Binary-logging) для предотвращения ошибок.
* [Issue #2458](https://github.com/manticoresoftware/manticoresearch/issues/2458) Включена поддержка zlib в пакетах для Windows.
* [Issue #2479](https://github.com/manticoresoftware/manticoresearch/issues/2479) Добавлена поддержка команды SHOW TABLE INDEXES.
* [Issue #2485](https://github.com/manticoresoftware/manticoresearch/issues/2485) Установлены метаданные сессии для ответов Buddy.
* [Issue #2490](https://github.com/manticoresoftware/manticoresearch/issues/2490) Миллисекундная точность для агрегаций на совместимых конечных точках.
* [Issue #2500](https://github.com/manticoresoftware/manticoresearch/issues/2500) Изменены сообщения об ошибках для операций с кластерами, когда репликация не удается запуститься.
* [Issue #2584](https://github.com/manticoresoftware/manticoresearch/pull/2584) Новые [показатели производительности в SHOW STATUS](Node_info_and_management/Node_status.md#Query-Time-Statistics): min/max/avg/95-й/99-й процентиль по типу запроса за последние 1, 5 и 15 минут.
* [Issue #2639](https://github.com/manticoresoftware/manticoresearch/pull/2639) Заменены все вхождения `index` на `table` в запросах и ответах.
* [Issue #2643](https://github.com/manticoresoftware/manticoresearch/issues/2643) Добавлен столбец `distinct` в результаты агрегации конечной точки HTTP `/sql`.
* [Issue #268](https://github.com/manticoresoftware/manticoresearch-buddy/issues/268) Реализовано автоматическое определение типов данных, импортируемых из Elasticsearch.
* [Issue #2744](https://github.com/manticoresoftware/manticoresearch/issues/2744) Добавлена поддержка сопоставления строк для выражений сравнения JSON полей.
* [Issue #2752](https://github.com/manticoresoftware/manticoresearch/issues/2752) Добавлена поддержка выражения `uuid_short` в списке выбора.
* [Issue #2783](https://github.com/manticoresoftware/manticoresearch/issues/2783) Manticore Search теперь запускает Buddy напрямую без обертки `manticore-buddy`.
* [Issue #2785](https://github.com/manticoresoftware/manticoresearch/issues/2785) Различные сообщения об ошибках для отсутствующих таблиц и таблиц, которые не поддерживают операции вставки.
* [Issue #2789](https://github.com/manticoresoftware/manticoresearch/issues/2789) OpenSSL 3 теперь статически встроен в `searchd`.
* [Issue #2790](https://github.com/manticoresoftware/manticoresearch/issues/2790) Добавлено выражение `CALL uuid_short` для генерации последовательностей с несколькими значениями `uuid_short`.
* [Issue #2803](https://github.com/manticoresoftware/manticoresearch/issues/2803) Добавлены отдельные параметры для правой таблицы в операции JOIN.
* [Issue #2810](https://github.com/manticoresoftware/manticoresearch/issues/2810) Улучшена производительность агрегации HTTP JSON для соответствия `GROUP BY` в SphinxQL.
* [Issue #2854](https://github.com/manticoresoftware/manticoresearch/issues/2854) Добавлена поддержка `fixed_interval` в запросах Kibana, связанных с датами.
* [Issue #2909](https://github.com/manticoresoftware/manticoresearch/issues/2909) Реализовано пакетирование для JOIN запросов, что улучшает производительность определенных JOIN запросов в сотни или даже тысячи раз.
* [Issue #2937](https://github.com/manticoresoftware/manticoresearch/issues/2937) Включено использование веса объединенной таблицы в запросах полного сканирования.
* [Issue #2953](https://github.com/manticoresoftware/manticoresearch/issues/2953) Исправлено логирование для запросов объединения.
* [Issue #337](https://github.com/manticoresoftware/manticoresearch-buddy/issues/337) Скрыты исключения Buddy из лога `searchd` в не-отладочном режиме.
* [Issue #2931](https://github.com/manticoresoftware/manticoresearch/issues/2931) Демон завершает работу с сообщением об ошибке, если пользователь устанавливает неправильные порты для слушателя репликации.

### Исправления ошибок
* [Commit 0c6b](https://github.com/manticoresoftware/manticoresearch/commit/0c6bdaf41ea3684e05952e1a8273893535b923f5) Исправлено: Неправильные результаты, возвращаемые в JOIN-запросах с более чем 32 столбцами.
* [Issue #2335](https://github.com/manticoresoftware/manticoresearch/issues/2335) Решена проблема, когда соединение таблиц не выполнялось, когда в условии использовались два json-атрибута.
* [Issue #2338](https://github.com/manticoresoftware/manticoresearch/issues/2338) Исправлен неправильный total_relation в многозапросах с [cutoff](Searching/Options.md#cutoff).
* [Issue #2366](https://github.com/manticoresoftware/manticoresearch/issues/2366) Исправлено фильтрация по `json.string` в правой таблице на [соединении таблиц](Searching/Joining.md).
* [Issue #2406](https://github.com/manticoresoftware/manticoresearch/issues/2406) Включено использование `null` для всех значений в любых POST HTTP JSON конечных точках (вставка/замена/пакет). В этом случае используется значение по умолчанию.
* [Issue #2418](https://github.com/manticoresoftware/manticoresearch/issues/2418) Оптимизировано потребление памяти путем корректировки выделения [max_packet_size](Server_settings/Searchd.md#max_packet_size) сетевого буфера для первоначального зондирования сокета.
* [Issue #2420](https://github.com/manticoresoftware/manticoresearch/issues/2420) Исправлена вставка беззнакового целого числа в атрибут bigint через JSON интерфейс.
* [Issue #2422](https://github.com/manticoresoftware/manticoresearch/issues/2422) Исправлены вторичные индексы для корректной работы с фильтрами исключения и включенным псевдо-шардингом.
* [Issue #2423](https://github.com/manticoresoftware/manticoresearch/issues/2423) Устранен баг в [manticore_new_cluster](Starting_the_server/Manually.md#searchd-command-line-options).
* [Issue #2448](https://github.com/manticoresoftware/manticoresearch/issues/2448) Решена проблема с аварийным завершением демона при неправильно сформированном запросе `_update`.
* [Issue #2452](https://github.com/manticoresoftware/manticoresearch/issues/2452) Исправлена неспособность гистограмм обрабатывать фильтры значений с исключениями.
* [Issue #55](https://github.com/manticoresoftware/columnar/issues/55) Исправлены запросы [knn](Searching/KNN.md#KNN-vector-search) к распределенным таблицам.
* [Issue #68](https://github.com/manticoresoftware/columnar/issues/68) Улучшена обработка фильтров исключения при кодировании таблиц в колонном доступе.
* [Commit 0eb1](https://github.com/manticoresoftware/manticoresearch/commit/0eb1579462013cb134bcaf74e06f6eeb0c2ecc4d) Исправлен парсер выражений, который не подчинялся переопределенному `thread_stack`.
* [Commit c304](https://github.com/manticoresoftware/manticoresearch/commit/c3041fd1d3e14773f3c5aa635343915a71f5cb95) Исправлен сбой при клонировании выражения IN для колонн.
* [Commit edad](https://github.com/manticoresoftware/columnar/commit/edadc694c68d6022bdd13134263667430a42cc1d) Исправлена проблема инверсии в итераторе битов, которая вызывала сбой.
* [Commit fc30](https://github.com/manticoresoftware/executor/commit/fc302ff1117b0b835a6f5a3c07057baf5fba14d9) Исправлена проблема, когда некоторые пакеты Manticore автоматически удалялись с помощью `unattended-upgrades`.
* [Issue #1019](https://github.com/manticoresoftware/manticoresearch/issues/1019) Улучшена обработка запросов из инструмента DbForge MySQL.
* [Issue #1107](https://github.com/manticoresoftware/manticoresearch/issues/1107) Исправлено экранирование специальных символов в `CREATE TABLE` и `ALTER TABLE`. ❤️ Спасибо, [@subnix](https://github.com/subnix) за PR.
* [Issue #116](https://github.com/manticoresoftware/manticoresearch-backup/issues/116#issuecomment-2216597206) Исправлен дедлок при обновлении атрибута blob в замороженном индексе. Дедлок возник из-за конфликтующих блокировок при попытке разморозить индекс. Это также могло вызвать сбой в manticore-backup.
* [Issue #1818](https://github.com/manticoresoftware/manticoresearch/issues/1818) `OPTIMIZE` теперь выдает ошибку, когда таблица заморожена.
* [Issue #2001](https://github.com/manticoresoftware/manticoresearch/issues/2001) Разрешены имена функций для использования в качестве имен столбцов.
* [Issue #2153](https://github.com/manticoresoftware/manticoresearch/issues/2153) Исправлено аварийное завершение демона при запросе настроек таблицы с неизвестным дисковым чанком.
* [Issue #2184](https://github.com/manticoresoftware/manticoresearch/issues/2184) Исправлена проблема, когда `searchd` зависал при остановке после `FREEZE` и `FLUSH RAMCHUNK`.
* [Issue #2228](https://github.com/manticoresoftware/manticoresearch/issues/2228) Удалены токены, связанные с датой/временем (и регулярные выражения) из зарезервированных слов.
* [Issue #2255](https://github.com/manticoresoftware/manticoresearch/issues/2255) Исправлен сбой на `FACET` с более чем 5 полями сортировки.
* [Issue #2265](https://github.com/manticoresoftware/manticoresearch/issues/2265) Исправлен сбой восстановления `mysqldump` с включенным `index_field_lengths`.
* [Issue #2291](https://github.com/manticoresoftware/manticoresearch/issues/2291) Исправлен сбой при выполнении команды `ALTER TABLE`.
* [Issue #2333](https://github.com/manticoresoftware/manticoresearch/issues/2333) Исправлен MySQL DLL в пакете Windows для корректной работы индексатора.
* [Issue #2393](https://github.com/manticoresoftware/manticoresearch/issues/2393) Исправлена ошибка компиляции GCC. ❤️ Спасибо, [@animetosho](https://github.com/animetosho) за PR.
* [Issue #2447](https://github.com/manticoresoftware/manticoresearch/issues/2447) Исправлена проблема экранирования в [_update](Data_creation_and_modification/Updating_documents/REPLACE.md?client=Elasticsearch-like+partial).
* [Issue #2460](https://github.com/manticoresoftware/manticoresearch/issues/2460) Исправлен сбой индексатора при объявлении нескольких атрибутов или полей с одинаковым именем.
* [Issue #2467](https://github.com/manticoresoftware/manticoresearch/issues/2467) Решена проблема аварийного завершения демона при неправильной трансформации для вложенных булевых запросов для конечных точек, связанных с "совместимостью".
* [Issue #2493](https://github.com/manticoresoftware/manticoresearch/issues/2493) Исправлено расширение в фразах с модификаторами.
* [Issue #2535](https://github.com/manticoresoftware/manticoresearch/issues/2535) Решен сбой демона при использовании [ZONE](Searching/Full_text_matching/Operators.md#ZONE-limit-operator) или [ZONESPAN](Searching/Full_text_matching/Operators.md#ZONESPAN-limit-operator) полнотекстового оператора.
* [Issue #2552](https://github.com/manticoresoftware/manticoresearch/issues/2552) Исправлено создание инфиксов для обычных и RT таблиц с словарем ключевых слов.
* [Issue #2571](https://github.com/manticoresoftware/manticoresearch/issues/2571) Исправлен ответ с ошибкой в запросе `FACET`; установлен порядок сортировки по умолчанию на `DESC` для `FACET` с `COUNT(*)`.
* [Issue #2580](https://github.com/manticoresoftware/manticoresearch/issues/2580) Исправлено аварийное завершение демона на Windows во время запуска.
* [Issue #2603](https://github.com/manticoresoftware/manticoresearch/issues/2603) Исправлено усечение запросов для HTTP конечных точек `/sql` и `/sql?mode=raw`; сделаны запросы из этих конечных точек последовательными без необходимости заголовка `query=`.
* [Issue #2623](https://github.com/manticoresoftware/manticoresearch/issues/2623) Исправлена проблема, когда авто-схема создает таблицу, но одновременно не удается.
* [Issue #2627](https://github.com/manticoresoftware/manticoresearch/issues/2627) Исправлена библиотека HNSW для поддержки загрузки нескольких KNN индексов.
* [Issue #2630](https://github.com/manticoresoftware/manticoresearch/issues/2630) Исправлена проблема заморозки, когда несколько условий происходят одновременно.
* [Issue #2645](https://github.com/manticoresoftware/manticoresearch/issues/2645) Исправлен сбой с фатальной ошибкой при использовании `OR` с KNN поиском.
* [Issue #2647](https://github.com/manticoresoftware/manticoresearch/issues/2647) Исправлена проблема, когда `indextool --mergeidf *.idf --out global.idf` удаляет выходной файл после создания.
* [Issue #2658](https://github.com/manticoresoftware/manticoresearch/issues/2658) Исправлено аварийное завершение демона при подвыборе с `ORDER BY` строкой во внешнем выборе.
* [Issue #2686](https://github.com/manticoresoftware/manticoresearch/issues/2686) Исправлен сбой при обновлении атрибута float вместе с атрибутом string.
* [Issue #2704](https://github.com/manticoresoftware/manticoresearch/issues/2704) Исправлена проблема, когда несколько стоп-слов из токенизаторов `lemmatize_xxx_all` увеличивают `hitpos` последующих токенов.
* [Issue #2708](https://github.com/manticoresoftware/manticoresearch/issues/2708) Исправлен сбой при `ALTER ... ADD COLUMN ... TEXT`.
* [Issue #2737](https://github.com/manticoresoftware/manticoresearch/issues/2737) Исправлена проблема, когда обновление атрибута blob в замороженной таблице с хотя бы одним RAM чанком вызывает ожидание последующих запросов `SELECT`, пока таблица не будет разморожена.
* [Issue #2742](https://github.com/manticoresoftware/manticoresearch/issues/2742) Исправлен пропуск кэша запросов для запросов с упакованными факторами.
* [Issue #2775](https://github.com/manticoresoftware/manticoresearch/issues/2775) Manticore теперь сообщает об ошибке при неизвестном действии вместо сбоя на запросах `_bulk`.
* [Issue #2791](https://github.com/manticoresoftware/manticoresearch/issues/2791) Исправлен возврат ID вставленного документа для HTTP конечной точки `_bulk`.
* [Issue #2797](https://github.com/manticoresoftware/manticoresearch/issues/2797) Исправлен сбой в группировщике при обработке нескольких таблиц, одна из которых пуста, а другая имеет разное количество совпадающих записей.
* [Issue #2835](https://github.com/manticoresoftware/manticoresearch/issues/2835) Исправлен сбой в сложных запросах `SELECT`.
* [Issue #2872](https://github.com/manticoresoftware/manticoresearch/issues/2872) Добавлено сообщение об ошибке, если аргумент `ALL` или `ANY` в выражении `IN` не является JSON-атрибутом.
* [Issue #2882](https://github.com/manticoresoftware/manticoresearch/issues/2882) Исправлено аварийное завершение демона при обновлении MVA в больших таблицах.
* [Issue #2888](https://github.com/manticoresoftware/manticoresearch/issues/2888) Исправлен сбой при ошибке токенизации с `libstemmer`. ❤️ Спасибо, [@subnix](https://github.com/subnix) за PR.
* [Issue #2919](https://github.com/manticoresoftware/manticoresearch/issues/2919) Исправлена проблема, когда присоединенный вес из правой таблицы не работал корректно в выражениях.
* [Issue #2919](https://github.com/manticoresoftware/manticoresearch/issues/2919) Исправлена проблема, когда вес правой присоединенной таблицы не работает в выражениях.
* [Issue #325](https://github.com/manticoresoftware/manticoresearch-buddy/issues/325) Исправлен сбой `CREATE TABLE IF NOT EXISTS ... WITH DATA`, когда таблица уже существует.
* [Issue #351](https://github.com/manticoresoftware/manticoresearch-buddy/issues/351) Исправлена ошибка неопределенного ключа массива "id" при подсчете по KNN с ID документа.
* [Issue #359](https://github.com/manticoresoftware/manticoresearch-buddy/issues/359) Исправлена функциональность `REPLACE INTO cluster_name:table_name`.
* [Issue #67](https://github.com/manticoresoftware/docker/issues/67) Исправлена фатальная ошибка при запуске контейнера Manticore Docker с `--network=host`.

## Версия 6.3.8
Выпущена: 22 ноября 2024

Версия 6.3.8 продолжает серию 6.3 и включает только исправления ошибок.

### Исправления ошибок

* [PR #2777](https://github.com/manticoresoftware/manticoresearch/pull/2777) Исправлено вычисление доступных потоков, когда параллелизм запросов ограничен настройками `threads` или `max_threads_per_query`.

## manticore-extra v1.1.20

Выпущен: 7 октября 2024

### Исправления ошибок

* [Issue #64](https://github.com/manticoresoftware/executor/issues/64) Решена проблема, когда утилита `unattended-upgrades`, которая автоматически устанавливает обновления пакетов на системах на базе Debian, неверно помечала несколько пакетов Manticore, включая `manticore-galera`, `manticore-executor` и `manticore-columnar-lib`, для удаления. Это произошло из-за того, что `dpkg` ошибочно считал виртуальный пакет `manticore-extra` избыточным. Были внесены изменения, чтобы гарантировать, что `unattended-upgrades` больше не пытается удалить важные компоненты Manticore.

## Версия 6.3.6
Выпущена: 2 августа 2024

Версия 6.3.6 продолжает серию 6.3 и включает только исправления ошибок.

### Исправления ошибок

* [Issue #2477](https://github.com/manticoresoftware/manticoresearch/issues/2477) Исправлен сбой, введенный в версии 6.3.4, который мог произойти при работе с выражениями и распределенными или несколькими таблицами.
* [Issue #2352](https://github.com/manticoresoftware/manticoresearch/issues/2352) Исправлен сбой демона или внутренняя ошибка при раннем выходе, вызванная `max_query_time`, при запросе нескольких индексов.

## Версия 6.3.4
Выпущена: 31 июля 2024

Версия 6.3.4 продолжает серию 6.3 и включает только незначительные улучшения и исправления ошибок.

### Небольшие изменения
* [Issue #1130](https://github.com/manticoresoftware/manticoresearch/issues/1130) Добавлена поддержка [DBeaver](Integration/DBeaver.md#Integration-with-DBeaver).
* [Issue #2146](https://github.com/manticoresoftware/manticoresearch/issues/2146) Улучшено экранирование разделителей в формах слов и исключениях.
* [Issue #2315](https://github.com/manticoresoftware/manticoresearch/issues/2315) Добавлены операторы сравнения строк в выражениях списка SELECT.
* [Issue #2363](https://github.com/manticoresoftware/manticoresearch/issues/2363) Добавлена поддержка нулевых значений в запросах Elastic-like bulk.
* [Issue #2374](https://github.com/manticoresoftware/manticoresearch/issues/2374) Добавлена поддержка mysqldump версии 9.
* [Issue #2375](https://github.com/manticoresoftware/manticoresearch/issues/2375) Улучшена обработка ошибок в HTTP JSON запросах с JSON путем к узлу, где возникает ошибка.

### Исправления ошибок
* [Issue #2280](https://github.com/manticoresoftware/manticoresearch/issues/2280) Исправлено ухудшение производительности в запросах с подстановочными знаками с множеством совпадений, когда disk_chunks > 1.
* [Issue #2332](https://github.com/manticoresoftware/manticoresearch/issues/2332) Исправлен сбой в выражениях списка SELECT MVA MIN или MAX для пустых массивов MVA.
* [Issue #2339](https://github.com/manticoresoftware/manticoresearch/issues/2339) Исправлена некорректная обработка запроса бесконечного диапазона Kibana.
* [Issue #2342](https://github.com/manticoresoftware/manticoresearch/issues/2342) Исправлен фильтр соединения по колоннарным атрибутам из правой таблицы, когда атрибут отсутствует в списке SELECT.
* [Issue #2343](https://github.com/manticoresoftware/manticoresearch/issues/2343) Исправлен дублирующийся спецификатор 'static' в Manticore 6.3.2.
* [Issue #2344](https://github.com/manticoresoftware/manticoresearch/issues/2344) Исправлено LEFT JOIN, возвращающий несовпадающие записи, когда используется MATCH() по правой таблице.
* [Issue #2350](https://github.com/manticoresoftware/manticoresearch/issues/2350) Исправлено сохранение дискового чанка в RT индексе с `hitless_words`.
* [Issue #2364](https://github.com/manticoresoftware/manticoresearch/issues/2364) Свойство 'aggs_node_sort' теперь можно добавлять в любом порядке среди других свойств.
* [Issue #2368](https://github.com/manticoresoftware/manticoresearch/issues/2368) Исправлена ошибка в порядке full-text и filter в JSON запросе.
* [Issue #2376](https://github.com/manticoresoftware/manticoresearch/issues/2376) Исправлен баг, связанный с некорректным JSON ответом для длинных UTF-8 запросов.
* [Issue #2684](https://github.com/manticoresoftware/dev/issues/2684) Исправлено вычисление выражений presort/prefilter, которые зависят от объединенных атрибутов.
* [Issue #301](https://github.com/manticoresoftware/manticoresearch-buddy/issues/301) Изменен метод вычисления размера данных для метрик, чтобы читать из файла `manticore.json`, а не проверять весь размер каталога данных.
* [Issue #302](https://github.com/manticoresoftware/manticoresearch-buddy/issues/302) Добавлена обработка запросов валидации от Vector.dev.

## Версия 6.3.2
Выпущена: 26 июня 2024

Версия 6.3.2 продолжает серию 6.3 и включает несколько исправлений ошибок, некоторые из которых были обнаружены после выпуска 6.3.0.

### Ломающее изменения
* ⚠️[Issue #2305](https://github.com/manticoresoftware/manticoresearch/issues/2305) Обновлены значения aggs.range до числовых.

### Исправления ошибок
* [Коммит c51c](https://github.com/manticoresoftware/manticoresearch/commit/c51c7fa4bd0ec4ad8d53188b888461bff29f1417) Исправлено группирование по сохраненной проверке против объединения rset.
* [Коммит 0e85](https://github.com/manticoresoftware/manticoresearch/commit/0e85b9d5682f51681750d798aef6a11cfb93d69e) Исправлен сбой в демоне при запросе с использованием символов подстановки в RT индексе с включенным словарем CRC и `local_df`.
* [Проблема #2200](https://github.com/manticoresoftware/manticoresearch/issues/2200) Исправлен сбой в JOIN на `count(*)` без GROUP BY.
* [Проблема #2201](https://github.com/manticoresoftware/manticoresearch/issues/2201) Исправлен JOIN, который не возвращал предупреждение при попытке группировки по полю полного текста.
* [Проблема #2230](https://github.com/manticoresoftware/manticoresearch/issues/2230) Устранена проблема, когда добавление атрибута через `ALTER TABLE` не учитывало параметры KNN.
* [Проблема #2231](https://github.com/manticoresoftware/manticoresearch/issues/2231) Исправлен сбой при удалении пакета `manticore-tools` Redhat версии 6.3.0.
* [Проблема #2242](https://github.com/manticoresoftware/manticoresearch/issues/2242) Исправлены проблемы с JOIN и несколькими операторами FACET, возвращающими некорректные результаты.
* [Проблема #2250](https://github.com/manticoresoftware/manticoresearch/issues/2250) Исправлено сообщение об ошибке при выполнении ALTER TABLE, если таблица находится в кластере.
* [Проблема #2252](https://github.com/manticoresoftware/manticoresearch/issues/2252) Исправлен оригинальный запрос, передаваемый в buddy из интерфейса SphinxQL.
* [Проблема #2267](https://github.com/manticoresoftware/manticoresearch/issues/2267) Улучшено расширение символов подстановки в `CALL KEYWORDS` для RT индекса с дисковыми чанками.
* [Проблема #271](https://github.com/manticoresoftware/manticoresearch-buddy/issues/271) Исправлено зависание некорректных запросов `/cli`.
* [Проблема #274](https://github.com/manticoresoftware/manticoresearch-buddy/issues/274) Устранены проблемы, когда параллельные запросы к Manticore могли зависать.
* [Проблема #275](https://github.com/manticoresoftware/manticoresearch-buddy/issues/275) Исправлено зависание `drop table if exists t; create table t` через `/cli`.

### Связанные с репликацией
* [Проблема #2270](https://github.com/manticoresoftware/manticoresearch/issues/2270) Добавлена поддержка формата `cluster:name` в HTTP-эндпоинте `/_bulk`.

## Версия 6.3.0
Выпущено: 23 мая 2024 года

### Основные изменения
* [Проблема #839](https://github.com/manticoresoftware/manticoresearch/issues/839) Реализован тип данных [float_vector](Creating_a_table/Data_types.md#Float-vector); реализован [поиск векторов](Searching/KNN.md#KNN-vector-search).
* [Проблема #1673](https://github.com/manticoresoftware/manticoresearch/issues/1673) [INNER/LEFT JOIN](Searching/Joining.md) (**бета-версия**).
* [Проблема #1744](https://github.com/manticoresoftware/manticoresearch/issues/1744) Реализовано автоматическое определение форматов даты для полей [timestamp](Creating_a_table/Data_types.md#Timestamps).
* [Проблема #1720](https://github.com/manticoresoftware/manticoresearch/pull/1720) Изменена лицензия Manticore Search с GPLv2-or-later на GPLv3-or-later.
* [Коммит 7a55](https://github.com/manticoresoftware/manticoresearch/commit/7a5508424) Запуск Manticore в Windows теперь требует Docker для работы Buddy.
* [Проблема #1541](https://github.com/manticoresoftware/manticoresearch/issues/1541) Добавлен оператор полного текста [REGEX](Searching/Full_text_matching/Operators.md#REGEX-operator).
* [Проблема #2091](https://github.com/manticoresoftware/manticoresearch/issues/2091) Поддержка Ubuntu Noble 24.04.
* [Коммит 514d](https://github.com/manticoresoftware/manticoresearch/commit/514d35b497f4bdb20b6473ff963752a978a4bb8d) Переработка операций с временем для повышения производительности и новые функции даты/времени:
  - [CURDATE()](Functions/Date_and_time_functions.md#CURDATE%28%29) - Возвращает текущую дату в местном часовом поясе
  - [QUARTER()](Functions/Date_and_time_functions.md#QUARTER%28%29) - Возвращает целый квартал года из аргумента timestamp
  - [DAYNAME()](Functions/Date_and_time_functions.md#DAYNAME%28%29) - Возвращает название дня недели для данного аргумента timestamp
  - [MONTHNAME()](Functions/Date_and_time_functions.md#MONTHNAME%28%29) - Возвращает название месяца для данного аргумента timestamp
  - [DAYOFWEEK()](Functions/Date_and_time_functions.md#DAYOFWEEK%28%29) - Возвращает целый индекс дня недели для данного аргумента timestamp
  - [DAYOFYEAR()](Functions/Date_and_time_functions.md#DAYOFYEAR%28%29) - Возвращает целый день года для данного аргумента timestamp
  - [YEARWEEK()](Functions/Date_and_time_functions.md#YEARWEEK%28%29) - Возвращает целый год и код дня первой недели для данного аргумента timestamp
  - [DATEDIFF()](Functions/Date_and_time_functions.md#DATEDIFF%28%29) - Возвращает количество дней между двумя заданными временными метками
  - [DATE()](Functions/Date_and_time_functions.md#DATE%28%29) - Форматирует часть даты из аргумента timestamp
  - [TIME()](Functions/Date_and_time_functions.md#TIME%28%29) - Форматирует часть времени из аргумента timestamp
  - [timezone](Server_settings/Searchd.md#timezone) - Часовой пояс, используемый функциями, связанными с датой/временем.
* [Коммит 30e7](https://github.com/manticoresoftware/manticoresearch/commit/30e789d9657f9b3093c280c6e5b0e46a021f6dc7) Добавлены агрегаты [range](Searching/Faceted_search.md#Facet-over-set-of-ranges), [histogram](Searching/Faceted_search.md#Facet-over-histogram-values), [date_range](Searching/Faceted_search.md#Facet-over-set-of-date-ranges) и [date_histogram](Searching/Faceted_search.md#Facet-over-histogram-date-values) в HTTP-интерфейс и аналогичные выражения в SQL.

### Небольшие изменения
* [Issue #1285](https://github.com/manticoresoftware/manticoresearch/issues/1285) Поддержка версий Filebeat 8.10 - 8.11.
* [Issue #1771](https://github.com/manticoresoftware/manticoresearch/issues/1771) [ALTER TABLE ... type='distributed'](Updating_table_schema_and_settings.md#Changing-a-distributed-table).
* [Issue #1788](https://github.com/manticoresoftware/manticoresearch/issues/1788) Добавлена возможность копирования таблиц с помощью SQL-запроса [CREATE TABLE ... LIKE ... WITH DATA](Creating_a_table/Local_tables/Real-time_table.md#CREATE-TABLE-LIKE:).
* [Issue #2072](https://github.com/manticoresoftware/manticoresearch/issues/2072) Оптимизирован [алгоритм компактирования таблиц](Securing_and_compacting_a_table/Compacting_a_table.md#Compacting-a-Table): Ранее как ручные [OPTIMIZE](Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE), так и автоматические [auto_optimize](Server_settings/Searchd.md#auto_optimize) процессы сначала объединяли чанки, чтобы убедиться, что их количество не превышает лимит, а затем удаляли документы из всех других чанков, содержащих удаленные документы. Этот подход иногда был слишком ресурсоемким и был отключен. Теперь объединение чанков происходит исключительно в соответствии с настройкой [progressive_merge](Server_settings/Common.md#progressive_merge). Однако чанки с большим количеством удаленных документов с большей вероятностью будут объединены.
* [Commit ce6c](https://github.com/manticoresoftware/columnar/commit/ce6c5d3381edcdc411a00a5923d13d56dd9b3010) Добавлена защита от загрузки вторичного индекса более новой версии.
* [Issue #1417](https://github.com/manticoresoftware/manticoresearch/issues/1417) Частичная замена через [REPLACE INTO ... SET](Data_creation_and_modification/Updating_documents/REPLACE.md#SQL-REPLACE).
* [Commit 7c16](https://github.com/manticoresoftware/manticoresearch/commit/7c167b934ce416d3dfbd658744a231e638b14d9a) Обновлены размеры буферов слияния по умолчанию: `.spa` (скалярные атрибуты): 256KB -> 8MB; `.spb` (блоб-атрибуты): 256KB -> 8MB; `.spc` (колоночные атрибуты): 1MB, без изменений; `.spds` (docstore): 256KB -> 8MB; `.spidx` (вторичные индексы): 256KB буфер -> 128MB лимит памяти; `.spi` (словарь): 256KB -> 16MB; `.spd` (списки документов): 8MB, без изменений; `.spp` (списки попаданий): 8MB, без изменений; `.spe` (списки пропусков): 256KB -> 8MB.
* [Issue #1859](https://github.com/manticoresoftware/manticoresearch/issues/1859) Добавлена [композитная агрегация](Searching/Grouping.md#GROUP-BY-multiple-fields-at-once) через JSON.
* [Commit 216b](https://github.com/manticoresoftware/manticoresearch-buddy/commit/216b824) Отключен PCRE.JIT из-за проблем с некоторыми шаблонами regex и отсутствия значительной выгоды по времени.
* [Commit 55cd](https://github.com/manticoresoftware/manticoresearch/commit/55cdce32) Добавлена поддержка ванильной Galera v.3 (api v25) (`libgalera_smm.so` из MySQL 5.x).
* [Commit 86f9](https://github.com/manticoresoftware/manticoresearch-buddy/commit/86f98ab) Изменен суффикс метрики с `_rate` на `_rps`.
* [Commit c0c1](https://github.com/manticoresoftware/manticoresearch-helm/commit/c0c1cff780031412ef57cefab987dfd16613368e) Улучшена документация о поддержке HA балансировщика.
* [Commit d1d2](https://github.com/manticoresoftware/manticoresearch/commit/d1d2ca18857c68106e89743756260e73285be739) Изменено `index` на `table` в сообщениях об ошибках; исправлено сообщение об ошибке парсера bison.
* [Commit fd26](https://github.com/manticoresoftware/manticoresearch/commit/fd26671b) Поддержка `manticore.tbl` в качестве имени таблицы.
* [Issue #1105](https://github.com/manticoresoftware/manticoresearch/issues/1105) Поддержка запуска индексатора через systemd ([docs](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Schedule-indexer-via-systemd)). ❤️ Спасибо, [@subnix](https://github.com/subnix) за PR.
* [Issue #1294](https://github.com/manticoresoftware/manticoresearch/issues/1294) Поддержка вторичных индексов в GEODIST().
* [Issue #1394](https://github.com/manticoresoftware/manticoresearch/issues/1394) Упрощен [SHOW THREADS](Node_info_and_management/SHOW_THREADS.md#SHOW-THREADS).
* [Issue #1424](https://github.com/manticoresoftware/manticoresearch/issues/1424) Добавлена поддержка значений по умолчанию (`agent_connect_timeout` и `agent_query_timeout`) для оператора `create distributed table`.
* [Issue #1442](https://github.com/manticoresoftware/manticoresearch/issues/1442) Добавлен параметр запроса поиска [expansion_limit](Searching/Options.md#expansion_limit), который переопределяет `searchd.expansion_limit`.
* [Issue #1448](https://github.com/manticoresoftware/manticoresearch/issues/1448) Реализован [ALTER TABLE](Updating_table_schema_and_settings.md#Updating-table-schema-in-RT-mode) для преобразования int->bigint.
* [Issue #1456](https://github.com/manticoresoftware/manticoresearch/issues/1456) Метаинформация в ответе MySQL.
* [Issue #1494](https://github.com/manticoresoftware/manticoresearch/issues/1494) [SHOW VERSION](Node_info_and_management/SHOW_VERSION.md#SHOW-VERSION).
* [Issue #1582](https://github.com/manticoresoftware/manticoresearch/issues/1582) Поддержка [удаления документов по массиву id](Data_creation_and_modification/Deleting_documents.md?client=JSON#Deleting-documents) через JSON.
* [Issue #1589](https://github.com/manticoresoftware/manticoresearch/issues/1589) Улучшено сообщение об ошибке "unsupported value type".
* [Issue #1634](https://github.com/manticoresoftware/manticoresearch/issues/1634) Добавлена версия Buddy в `SHOW STATUS`.
* [Issue #1641](https://github.com/manticoresoftware/manticoresearch/issues/1641) Оптимизация запросов на совпадение в случае нуля документов для ключевого слова.
* [Issue #1712](https://github.com/manticoresoftware/manticoresearch/issues/1712) Добавлено преобразование в булевый атрибут из строкового значения "true" и "false" при отправке данных.
* [Issue #1713](https://github.com/manticoresoftware/manticoresearch/issues/1713) Добавлена таблица [access_dict](Server_settings/Searchd.md#access_dict) и опция searchd.
* [Issue #1767](https://github.com/manticoresoftware/manticoresearch/issues/1767) Добавлены новые параметры: [expansion_merge_threshold_docs](Server_settings/Searchd.md#expansion_merge_threshold_docs) и [expansion_merge_threshold_hits](Server_settings/Searchd.md#expansion_merge_threshold_hits) в секцию searchd конфигурации; сделан порог для объединения крошечных терминов расширенных терминов настраиваемым.
* [Issue #1768](https://github.com/manticoresoftware/manticoresearch/issues/1768) Добавлено отображение времени последней команды в `@@system.sessions`.
* [Issue #1806](https://github.com/manticoresoftware/manticoresearch/issues/1806) Обновлен протокол Buddy до версии 2.
* [Issue #1810](https://github.com/manticoresoftware/manticoresearch/issues/1810) Реализованы дополнительные форматы запросов к конечной точке `/sql` для упрощения интеграции с библиотеками.
* [Issue #1825](https://github.com/manticoresoftware/manticoresearch/pull/1825) Добавлен раздел Информация в SHOW BUDDY PLUGINS.
* [Issue #1837](https://github.com/manticoresoftware/manticoresearch/issues/1837) Улучшено потребление памяти в `CALL PQ` с большими пакетами.
* [Issue #1853](https://github.com/manticoresoftware/manticoresearch/pull/1853) Переключен компилятор с Clang 15 на Clang 16.
* [Issue #1857](https://github.com/manticoresoftware/manticoresearch/issues/1857) Добавлено сравнение строк. ❤️ Спасибо, [@etcd](https://github.com/etcd) за PR.
* [Issue #1915](https://github.com/manticoresoftware/manticoresearch/issues/1915) Добавлена поддержка объединенных хранимых полей.
* [Issue #1937](https://github.com/manticoresoftware/manticoresearch/issues/1937) Логирование хоста:порта клиента в журнале запросов.
* [Issue #1981](https://github.com/manticoresoftware/manticoresearch/issues/1981) Исправлена ошибка.
* [Issue #1983](https://github.com/manticoresoftware/manticoresearch/issues/1983) Введена поддержка уровней подробности для [плана запроса через JSON](Node_info_and_management/Profiling/Query_plan.md?client=JSON#Query-plan).
* [Issue #2010](https://github.com/manticoresoftware/manticoresearch/issues/2010) Отключено логирование запросов от Buddy, если не установлен `log_level=debug`.
* [Issue #2035](https://github.com/manticoresoftware/manticoresearch/issues/2035) Поддержка Linux Mint 21.3.
* [Issue #2056](https://github.com/manticoresoftware/manticoresearch/issues/2056) Manticore не удалось собрать с Mysql 8.3+.
* [Issue #2112](https://github.com/manticoresoftware/manticoresearch/issues/2112) Команда `DEBUG DEDUP` для чанков таблицы реального времени, которые могут содержать дублирующиеся записи после присоединения обычной таблицы, содержащей дубликаты.
* [Issue #212](https://github.com/manticoresoftware/manticoresearch-buddy/issues/212) Добавлено время к [SHOW QUERIES](Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES).
* [Issue #218](https://github.com/manticoresoftware/manticoresearch-buddy/issues/218) Обработка столбца `@timestamp` как временной метки.
* [Issue #252](https://github.com/manticoresoftware/manticoresearch-buddy/issues/250) Реализована логика для включения/выключения плагинов Buddy.
* [Issue #254](https://github.com/manticoresoftware/manticoresearch-buddy/issues/254) Обновлен composer до более свежей версии, в которой исправлены недавние уязвимости CVE.
* [Issue #340](https://github.com/manticoresoftware/manticoresearch/issues/340) Небольшая оптимизация в юните systemd Manticore, связанная с `RuntimeDirectory`.
* [Issue #51](https://github.com/manticoresoftware/executor/issues/51) Добавлена поддержка rdkafka и обновлено до PHP 8.3.3.
* [Issue #527](https://github.com/manticoresoftware/manticoresearch/issues/527) Поддержка [присоединения](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Attaching_one_table_to_another.md#Attaching-one-table-to-another) таблицы реального времени. Новая команда [ALTER TABLE ... RENAME](Updating_table_schema_and_settings.md#Renaming-a-real-time-table).

### Ломающие изменения и устаревания
* ⚠️[Issue #1436](https://github.com/manticoresoftware/manticoresearch/issues/1436) Исправлена проблема с расчетом IDF. `local_df` теперь по умолчанию. Улучшен протокол поиска мастер-агент (версия обновлена). Если вы запускаете Manticore Search в распределенной среде с несколькими экземплярами, сначала обновите агентов, затем мастеров.
* ⚠️[Issue #1572](https://github.com/manticoresoftware/manticoresearch/issues/1572) Добавлено реплицирование распределенных таблиц и обновлен протокол репликации. Если вы запускаете кластер репликации, вам нужно:
  - Сначала аккуратно остановить все ваши узлы
  - Затем запустить узел, который был остановлен последним, с `--new-cluster`, используя инструмент `manticore_new_cluster` в Linux.
  - Ознакомьтесь с [перезапуском кластера](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster) для получения дополнительных деталей.
* ⚠️[Issue #1763](https://github.com/manticoresoftware/manticoresearch/pull/1763) Псевдонимы конечных точек HTTP API `/json/*` были устаревшими.
* ⚠️[Issue #1982](https://github.com/manticoresoftware/manticoresearch/issues/1982) Изменено [профилирование](Node_info_and_management/Profiling/Query_profile.md#Query-profile) на [план](Node_info_and_management/Profiling/Query_plan.md#Query-plan) в JSON, добавлено профилирование запросов для JSON.
* ⚠️[Commit e235](https://github.com/manticoresoftware/manticoresearch-backup/commit/e23585a) manticore-backup больше не создает резервные копии `plugin_dir`.
* ⚠️[Issue #171](https://github.com/manticoresoftware/manticoresearch-buddy/issues/155) Перенесен Buddy на Swoole для улучшения производительности и стабильности. При переходе на новую версию убедитесь, что все пакеты Manticore обновлены.
* ⚠️[Issue #196](https://github.com/manticoresoftware/manticoresearch-buddy/pull/205) Объединены все основные плагины в Buddy и изменена основная логика.
* ⚠️[Issue #2107](https://github.com/manticoresoftware/manticoresearch/issues/2107) Обработка идентификаторов документов как чисел в ответах `/search`.
* ⚠️[Issue #38](https://github.com/manticoresoftware/executor/issues/38) Добавлен Swoole, отключен ZTS и удалено параллельное расширение.
* ⚠️[Issue #1929](https://github.com/manticoresoftware/manticoresearch/issues/1929) Переопределение в `charset_table` не работало в некоторых случаях.

### Изменения, связанные с репликацией
* [Commit 3376](https://github.com/manticoresoftware/manticoresearch/commit/3376351f73db3ff0cb6cb708a304a7bd83e69cfb) Исправлена ошибка репликации на SST больших файлов.
* [Commit 6d36](https://github.com/manticoresoftware/manticoresearch/commit/6d36c68fb7ce3cb274b467aac435788905084952) Добавлен механизм повторной попытки для команд репликации; исправлена ошибка соединения репликации в загруженной сети с потерей пакетов.
* [Commit 842e](https://github.com/manticoresoftware/manticoresearch/commit/842e27e71790833ab4125d49103d2933d5df60c3) Изменено сообщение FATAL в репликации на сообщение WARNING.
* [Commit 8c32](https://github.com/manticoresoftware/manticoresearch/commit/8c32bce7db39d8ed3b1a06aaa1d2c3ec372084f0) Исправлен расчет `gcache.page_size` для кластеров репликации без таблиц или с пустыми таблицами; также исправлено сохранение и загрузка параметров Galera.
* [Commit a2af](https://github.com/manticoresoftware/manticoresearch/commit/a2af06ca36abcbf893e43c8803853130b397c54b) Добавлена функциональность для пропуска команды репликации обновления узлов на узле, который присоединяется к кластеру.
* [Commit c054](https://github.com/manticoresoftware/manticoresearch/commit/c0541e6892f5116b1f957b40060ba8ecd01a5afb) Исправлен взаимоблокировка во время репликации при обновлении атрибутов blob по сравнению с заменой документов.
* [Commit e80d](https://github.com/manticoresoftware/manticoresearch/commit/e80d505b963999016033613c40c1cf3151d648e7) Добавлены параметры конфигурации searchd [replication_connect_timeout](Server_settings/Searchd.md#replication_connect_timeout), [replication_query_timeout](Server_settings/Searchd.md#replication_query_timeout), [replication_retry_delay](Server_settings/Searchd.md#replication_retry_delay), [replication_retry_count](Server_settings/Searchd.md#replication_retry_count) для управления сетью во время репликации, аналогично `searchd.agent_*`, но с другими значениями по умолчанию.
* [Issue #1356](https://github.com/manticoresoftware/manticoresearch/issues/1356) Исправлена повторная попытка узлов репликации после пропуска некоторых узлов и неудачи разрешения имен этих узлов.
* [Issue #1445](https://github.com/manticoresoftware/manticoresearch/issues/1445) Исправлен уровень подробности журнала репликации в `show variables`.
* [Issue #1482](https://github.com/manticoresoftware/manticoresearch/issues/1482) Исправлена проблема репликации для узла-присоединителя, подключающегося к кластеру на поде, перезапущенном в Kubernetes.
* [Issue #1962](https://github.com/manticoresoftware/manticoresearch/issues/1962) Исправлено долгое ожидание репликации для изменения на пустом кластере с недопустимым именем узла.

### Исправления ошибок
* [Commit 8a48](https://github.com/manticoresoftware/manticoresearch/commit/8a486c7ba1a293879cdecc41fb52eb7ab0cbf832) Исправлена очистка неиспользуемых совпадений в `count distinct`, которая могла приводить к сбою.
* [Issue #1569](https://github.com/manticoresoftware/manticoresearch/issues/1569) Бинарный лог теперь записывается с транзакционной гранулярностью.
* [Issue #2089](https://github.com/manticoresoftware/manticoresearch/issues/2089) Исправлена ошибка, связанная с 64-битными идентификаторами, которая могла приводить к ошибке "Malformed packet" при вставке через MySQL, что в свою очередь приводило к [повреждению таблиц и дублированию идентификаторов](https://github.com/manticoresoftware/manticoresearch/issues/2090).
* [Issue #2160](https://github.com/manticoresoftware/manticoresearch/issues/2160) Исправлена вставка дат как UTC вместо локального часового пояса.
* [Issue #2177](https://github.com/manticoresoftware/manticoresearch/issues/2177) Исправлен сбой, возникавший при выполнении поиска в таблице real-time с непустым `index_token_filter`.
* [Issue #2209](https://github.com/manticoresoftware/manticoresearch/issues/2209) Изменена фильтрация дубликатов в колоннарном хранилище RT для исправления сбоев и некорректных результатов запросов.
* [Commit 001d](https://github.com/manticoresoftware/manticoresearch/commit/001d82904e4b887c49387308291aeefb07c0fbc4) Исправлен html stipper, повреждающий память после обработки объединённого поля.
* [Commit 00eb](https://github.com/manticoresoftware/manticoresearch/commit/00eb2ebf) Предотвращена перемотка потока после flush для избежания проблем взаимодействия с mysqldump.
* [Commit 0553](https://github.com/manticoresoftware/manticoresearch/commit/05534114acf08c70ffcc41a23d558dec09a34373) Не ждать завершения preread, если он не был запущен.
* [Commit 055a](https://github.com/manticoresoftware/manticoresearch/commit/055aac09f877570cd29b6594cb283182eba7f73b) Исправлен вывод длинной строки Buddy: теперь она разделяется на несколько строк в логе searchd.
* [Commit 0a88](https://github.com/manticoresoftware/manticoresearch/commit/0a88056307f82c50fcdc458ad6dbc307a00e3f92) Перемещено предупреждение интерфейса MySQL о неудачной установке уровня подробности заголовка `debugv`.
* [Commit 150a](https://github.com/manticoresoftware/manticoresearch/commit/150a20286b7d98d24b844ae577d7fda4a444798d) Исправлено состояние гонки при операциях управления несколькими кластерами; запрещено создание нескольких кластеров с одинаковым именем или путём.
* [Commit 2e40](https://github.com/manticoresoftware/manticoresearch/commit/2e405c1c9ebac6f46206e3df17ac1c19129c9aa3) Исправлен неявный cutoff в полнотекстовых запросах; MatchExtended разделён на шаблонную часть partD.
* [Commit 75f5](https://github.com/manticoresoftware/manticoresearch/commit/75f51421542a2a0658f9e2f9f31048c8c5224479) Исправлено расхождение параметра `index_exact_words` между индексированием и загрузкой таблицы в демон.
* [Commit 7643](https://github.com/manticoresoftware/manticoresearch/commit/7643976bea142e08535d701000eff0d09a99fda2) Исправлено отсутствие сообщения об ошибке при удалении некорректного кластера.
* [Commit 7a03](https://github.com/manticoresoftware/manticoresearch/commit/7a034ac990d5ec9344320c05a6a2315583b9b858) Исправлены ошибки CBO при объединении через очередь; исправлено псевдо-шардирование CBO для RT.
* [Commit 7b4e](https://github.com/manticoresoftware/manticoresearch/commit/7b4ea95e) При запуске без библиотеки secondary index (SI) и соответствующих параметров в конфигурации выводилось вводящее в заблуждение сообщение 'WARNING: secondary_indexes set but failed to initialize secondary library'.
* [Commit 8496](https://github.com/manticoresoftware/manticoresearch/commit/8496317f099fade8d2db463fb88483e7fcdfe85b) Исправлена сортировка хитов в quorum.
* [Commit 8973](https://github.com/manticoresoftware/manticoresearch-buddy/commit/8973ad3) Исправлена проблема с опциями в верхнем регистре в плагине ModifyTable.
* [Commit 9935](https://github.com/manticoresoftware/manticoresearch-buddy/commit/99350d4c184379b19f3efa9751d9b5be5e6edfb5) Исправлено восстановление из дампа с пустыми значениями JSON (представленными как NULL).
* [Commit a28f](https://github.com/manticoresoftware/manticoresearch/commit/a28f6e087b3d957e48816070be17d0823904936a) Исправлен таймаут SST на узле присоединения при получении SST с использованием pcon.
* [Commit b5a5](https://github.com/manticoresoftware/manticoresearch/commit/b5a55cc26debcddcda5d9caef8366a707e0dd763) Исправлен сбой при выборе строкового атрибута с псевдонимом.
* [Commit c556](https://github.com/manticoresoftware/manticoresearch/commit/c556e7ed8b6da8eac8ef094650df5f0a8977916b) Добавлено преобразование запроса терма в `=term` в полнотекстовом запросе с использованием поля `morphology_skip_fields`.
* [Commit cdc3](https://github.com/manticoresoftware/manticoresearch/commit/cdc38085145cd1025b96fcac2a46188a0425d9e1) Добавлен отсутствующий ключ конфигурации (skiplist_cache_size).
* [Commit cf6e](https://github.com/manticoresoftware/manticoresearch/commit/cf6ec3bfdc3f0990fb31ff53822b2331a6667727) Исправлен сбой в ранжировщике выражений при большом сложном запросе.
* [Commit e513](https://github.com/manticoresoftware/manticoresearch/commit/e5139ba75eef7223df375ae3cc3f138fa7d925f1) Исправлен конфликт CBO полнотекстового поиска с некорректными подсказками индексов.
* [Commit eb05](https://github.com/manticoresoftware/manticoresearch/commit/eb0514b3a6f57f278252b0724eebd112ec116e0f) Прерывание preread при выключении для ускорения завершения работы.
* [Commit f945](https://github.com/manticoresoftware/manticoresearch/commit/f94555a293cfa51004ff81135b8bd8500b955115) Изменён расчёт стека для полнотекстовых запросов, чтобы избежать сбоя при сложном запросе.
* [Issue #1262](https://github.com/manticoresoftware/manticoresearch/issues/1262) Исправлен сбой индексатора при индексировании SQL-источника с несколькими столбцами с одинаковым именем.
* [Issue #1273](https://github.com/manticoresoftware/manticoresearch/issues/1273) Возвращать 0 вместо <empty> для несуществующих системных переменных.
* [Issue #1289](https://github.com/manticoresoftware/manticoresearch/issues/1289) Исправлена ошибка indextool при проверке внешних файлов таблицы RT.
* [Issue #1335](https://github.com/manticoresoftware/manticoresearch/issues/1335) Исправлена ошибка разбора запроса из-за нескольких форм слова внутри фразы.
* [Issue #1364](https://github.com/manticoresoftware/manticoresearch/issues/1364) Добавлен повтор воспроизведения пустых файлов binlog для старых версий binlog.
* [Issue #1365](https://github.com/manticoresoftware/manticoresearch/issues/1365) Исправлено удаление последнего пустого файла binlog.
* [Issue #1372](https://github.com/manticoresoftware/manticoresearch/issues/1372) Исправлены некорректные относительные пути (преобразуемые в абсолютные относительно директории запуска демона) после изменений в `data_dir`, влияющих на текущий рабочий каталог при старте демона.
* [Issue #1393](https://github.com/manticoresoftware/manticoresearch/issues/1393) Существенное замедление в hn_small: получение/кеширование информации о CPU при запуске демона.
* [Issue #1395](https://github.com/manticoresoftware/manticoresearch/issues/1395) Исправлено предупреждение об отсутствии внешнего файла при загрузке индекса.
* [Issue #1402](https://github.com/manticoresoftware/manticoresearch/issues/1402) Исправлен сбой в глобальных группировщиках при освобождении атрибутов data ptr.
* [Issue #1403](https://github.com/manticoresoftware/manticoresearch/issues/1403) _ADDITIONAL_SEARCHD_PARAMS не работает.
* [Issue #1427](https://github.com/manticoresoftware/manticoresearch/issues/1427) Исправлено замещение параметра `agent_query_timeout` для таблицы на значение параметра по умолчанию `agent_query_timeout`.
* [Issue #1444](https://github.com/manticoresoftware/manticoresearch/issues/1444) Исправлен сбой в группировщике и ранжировщике при использовании `packedfactors()` с несколькими значениями на совпадение.
* [Issue #1458](https://github.com/manticoresoftware/manticoresearch/issues/1458) Manticore падает при частых обновлениях индекса.
* [Issue #1481](https://github.com/manticoresoftware/manticoresearch/issues/1481) Исправлен сбой при очистке разобранного запроса после ошибки разбора.
* [Issue #1484](https://github.com/manticoresoftware/manticoresearch/issues/1484) Исправлено ненаправление HTTP JSON-запросов к buddy.
* [Issue #1499](https://github.com/manticoresoftware/manticoresearch/issues/1499) Корневое значение JSON-атрибута не могло быть массивом. Исправлено.
* [Issue #1507](https://github.com/manticoresoftware/manticoresearch/issues/1507) Исправлен сбой при пересоздании таблицы в рамках транзакции.
* [Issue #1515](https://github.com/manticoresoftware/manticoresearch/issues/1515) Исправлено расширение сокращённых форм русских лемм.
* [Issue #1579](https://github.com/manticoresoftware/manticoresearch/issues/1579) Исправлено использование атрибутов JSON и STRING в выражении [date_format](Functions/Date_and_time_functions.md#DATE_FORMAT()).
* [Issue #1580](https://github.com/manticoresoftware/manticoresearch/issues/1580) Исправлен группировщик для нескольких псевдонимов JSON-полей.
* [Issue #1594](https://github.com/manticoresoftware/manticoresearch/issues/1594) Некорректное total_related в dev: исправлен неявный cutoff vs limit; добавлено улучшенное обнаружение полного сканирования в JSON-запросах.
* [Issue #1603](https://github.com/manticoresoftware/manticoresearch/issues/1603) Исправлено использование атрибутов JSON и STRING во всех выражениях дат.
* [Issue #1609](https://github.com/manticoresoftware/manticoresearch/issues/1609) Сбой при использовании LEVENSHTEIN().
* [Issue #1612](https://github.com/manticoresoftware/manticoresearch/issues/1612) Исправлено повреждение памяти после ошибки разбора поискового запроса с подсветкой.
* [Issue #1614](https://github.com/manticoresoftware/manticoresearch/issues/1614) Отключено расширение подстановочных символов для терминов короче `min_prefix_len` / `min_infix_len`.
* [Issue #1617](https://github.com/manticoresoftware/manticoresearch/issues/1617) Изменено поведение: не логировать ошибку, если Buddy успешно обрабатывает запрос.
* [Issue #1635](https://github.com/manticoresoftware/manticoresearch/issues/1635) Исправлено значение total в метаинформации поискового запроса для запросов с установленным limit.
* [Issue #1640](https://github.com/manticoresoftware/manticoresearch/issues/1640) Невозможно использовать таблицу с буквами верхнего регистра через JSON в режиме plain.
* [Issue #1643](https://github.com/manticoresoftware/manticoresearch/issues/1643) Добавлено значение по умолчанию для `SPH_EXTNODE_STACK_SIZE`.
* [Issue #1646](https://github.com/manticoresoftware/manticoresearch/issues/1646) Исправлен лог SphinxQL отрицательного фильтра с ALL/ANY для атрибута MVA.
* [Issue #1660](https://github.com/manticoresoftware/manticoresearch/issues/1660) Исправлено применение списков удаления docid из других индексов. ❤️ Спасибо, [@raxoft](https://github.com/raxoft), за PR.
* [Issue #1668](https://github.com/manticoresoftware/manticoresearch/issues/1668) Исправлены пропущенные совпадения из-за преждевременного выхода при полном сканировании raw индекса (без итераторов индексов); удалён cutoff из plain row iterator.
* [Issue #1671](https://github.com/manticoresoftware/manticoresearch/issues/1671) Исправлена ошибка `FACET` при запросе к распределённой таблице с агентными и локальными таблицами.
* [Issue #1690](https://github.com/manticoresoftware/manticoresearch/issues/1690) Исправлен сбой при оценке гистограммы для больших значений.
* [Issue #1692](https://github.com/manticoresoftware/manticoresearch/issues/1692) Сбой при выполнении alter table tbl add column col uint.
* [Issue #1710](https://github.com/manticoresoftware/manticoresearch/issues/1710) Пустой результат для условия `WHERE json.array IN (<value>)`.
* [Issue #172](https://github.com/manticoresoftware/manticoresearch-buddy/issues/172) Исправлена проблема с TableFormatter при отправке запроса на `/cli`.
* [Issue #1742](https://github.com/manticoresoftware/manticoresearch/issues/1742) `CREATE TABLE` не выдавал ошибку в случае отсутствия файла wordforms.
* [Issue #1762](https://github.com/manticoresoftware/manticoresearch/issues/1762) Порядок атрибутов в таблицах RT теперь соответствует порядку в конфигурации.
* [Issue #1765](https://github.com/manticoresoftware/manticoresearch/issues/1765) HTTP-булев запрос с условием 'should' возвращает некорректные результаты.
* [Issue #1769](https://github.com/manticoresoftware/manticoresearch/issues/1769) Сортировка по строковым атрибутам не работает с `SPH_SORT_ATTR_DESC` и `SPH_SORT_ATTR_ASC`.
* [Issue #177](https://github.com/manticoresoftware/manticoresearch-buddy/issues/177) Отключён HTTP-заголовок `Expect: 100-continue` для curl-запросов к Buddy.
* [Issue #1791](https://github.com/manticoresoftware/manticoresearch/issues/1791) Сбой, вызванный псевдонимом в GROUP BY.
* [Issue #1792](https://github.com/manticoresoftware/manticoresearch/issues/1792) SQL meta summary показывает неправильное время в Windows.
* [Issue #1794](https://github.com/manticoresoftware/manticoresearch/issues/1794) Исправлено падение производительности для однотерминных JSON-запросов.
* [Issue #1798](https://github.com/manticoresoftware/manticoresearch/issues/1798) Несовместимые фильтры не вызывали ошибку на `/search`.
* [Issue #1802](https://github.com/manticoresoftware/manticoresearch/issues/1802) Исправлены операции `ALTER CLUSTER ADD` и `JOIN CLUSTER` для ожидания друг друга, что предотвращает состояние гонки, когда `ALTER` добавляет таблицу в кластер, пока узел-донора отправляет таблицы узлу-присоединителю.
* [Issue #1811](https://github.com/manticoresoftware/manticoresearch/issues/1811) Некорректная обработка запросов `/pq/{table}/*`.
* [Issue #1816](https://github.com/manticoresoftware/manticoresearch/issues/1816) `UNFREEZE` не работал в некоторых случаях.
* [Issue #183](https://github.com/manticoresoftware/manticoresearch-buddy/pull/183) Исправлена проблема с восстановлением MVA в некоторых случаях.
* [Issue #1849](https://github.com/manticoresoftware/manticoresearch/issues/1849) Исправлен сбой indextool при завершении работы в связке с MCL.
* [Issue #1866](https://github.com/manticoresoftware/manticoresearch/issues/1866) Исправлено ненужное декодирование URL для запросов `/cli_json`.
* [Issue #1872](https://github.com/manticoresoftware/manticoresearch/issues/1872) Изменена логика задания plugin_dir при старте демона.
* [Issue #1874](https://github.com/manticoresoftware/manticoresearch/issues/1874) Сбой при alter table ... exceptions.
* [Issue #1891](https://github.com/manticoresoftware/manticoresearch/issues/1891#issuecomment-2051941200) Manticore падает с `signal 11` при вставке данных.
* [Issue #1920](https://github.com/manticoresoftware/manticoresearch/issues/1920) Уменьшено ограничение пропускной способности для [low_priority](Searching/Options.md#low_priority).
* [Issue #1924](https://github.com/manticoresoftware/manticoresearch/issues/1924) Ошибка при работе mysqldump и восстановлении mysql.
* [Issue #1951](https://github.com/manticoresoftware/manticoresearch/issues/1951) Исправлено некорректное создание распределённой таблицы в случае отсутствия локальной таблицы или неверного описания агента; теперь возвращается сообщение об ошибке.
* [Issue #1972](https://github.com/manticoresoftware/manticoresearch/issues/1972) Реализован счётчик `FREEZE` для избежания проблем с freeze/unfreeze.
* [Issue #1980](https://github.com/manticoresoftware/manticoresearch/issues/1980) Учтён тайм-аут запроса в OR-узлах. Ранее `max_query_time` в некоторых случаях не работал.
* [Issue #1986](https://github.com/manticoresoftware/manticoresearch/issues/1986) Не удалось переименовать новый файл в текущий [manticore.json].
* [Issue #1988](https://github.com/manticoresoftware/manticoresearch/issues/1988) Полнотекстовый запрос мог игнорировать подсказку CBO `SecondaryIndex`.
* [Issue #1990](https://github.com/manticoresoftware/manticoresearch/issues/1990) Исправлен `expansion_limit` для разбиения конечного набора результатов ключевых слов вызова из нескольких дисковых или RAM чанков.
* [Issue #1994](https://github.com/manticoresoftware/manticoresearch/issues/1994) Неверные внешние файлы.
* [Issue #2021](https://github.com/manticoresoftware/manticoresearch/issues/2021) Некоторые процессы manticore-executor могли оставаться запущенными после остановки Manticore.
* [Issue #2029](https://github.com/manticoresoftware/manticoresearch/issues/2029) Сбой при использовании расстояния Левенштейна.
* [Issue #2037](https://github.com/manticoresoftware/manticoresearch/issues/2037) Ошибка после нескольких запусков оператора max на пустом индексе.
* [Issue #2052](https://github.com/manticoresoftware/manticoresearch/issues/2052) Сбой при многогруппировке с JSON.field.
* [Issue #2067](https://github.com/manticoresoftware/manticoresearch/issues/2067) Manticore падал при некорректном запросе к _update.
* [Issue #2069](https://github.com/manticoresoftware/manticoresearch/issues/2069) Исправлена проблема с компараторами строковых фильтров для замкнутых диапазонов в JSON-интерфейсе.
* [Issue #2082](https://github.com/manticoresoftware/manticoresearch/issues/2082) `alter` не выполнялся, когда путь data_dir находился на символической ссылке.
* [Issue #2102](https://github.com/manticoresoftware/manticoresearch/pull/2102) Улучшена специальная обработка SELECT-запросов в mysqldump для обеспечения совместимости получаемых INSERT-операторов с Manticore.
* [Issue #2103](https://github.com/manticoresoftware/manticoresearch/issues/2103) Тайские символы были в неправильных наборах символов.
* [Issue #2124](https://github.com/manticoresoftware/manticoresearch/issues/2124) Сбой при использовании SQL-запроса с зарезервированным словом.
* [Issue #2154](https://github.com/manticoresoftware/manticoresearch/issues/2154) Таблицы с wordforms не могли быть импортированы.
* [Issue #2176](https://github.com/manticoresoftware/manticoresearch/issues/2176) Исправлен сбой, возникавший при установке параметра engine в 'columnar' и добавлении дублированных идентификаторов через JSON.
* [Issue #223](https://github.com/manticoresoftware/manticoresearch-buddy/issues/223) Корректная ошибка при попытке вставить документ без схемы и без имён столбцов.
* [Issue #239](https://github.com/manticoresoftware/manticoresearch-buddy/issues/239) Многострочная вставка Auto-schema могла завершаться неудачей.
* [Issue #399](https://github.com/manticoresoftware/manticoresearch/issues/399) Добавлено сообщение об ошибке при индексации, если атрибут id объявлен в источнике данных.
* [Issue #59](https://github.com/manticoresoftware/manticoresearch-helm/issues/59) Сбой кластера Manticore.
* [Issue #68](https://github.com/manticoresoftware/manticoresearch-helm/issues/68) optimize.php падал, если таблица percolate присутствовала.
* [Issue #77](https://github.com/manticoresoftware/manticoresearch-helm/issues/77) Исправлены ошибки при развертывании на Kubernetes.
* [Issue #274](https://github.com/manticoresoftware/manticoresearch-buddy/issues/274) Исправлена некорректная обработка параллельных запросов к Buddy.

### Связано с manticore-backup
* [Issue #97](https://github.com/manticoresoftware/manticoresearch-backup/pull/97) Установить VIP HTTP порт по умолчанию, если он доступен.  
Различные улучшения: улучшена проверка версий и потоковая декомпрессия ZSTD; добавлены пользовательские подсказки для несоответствий версий во время восстановления; исправлено неправильное поведение подсказок для разных версий при восстановлении; улучшена логика декомпрессии для чтения непосредственно из потока, а не в рабочую память; добавлен флаг `--force`  
* [Commit 3b35](https://github.com/manticoresoftware/manticoresearch-backup/commit/3b357ea) Добавлено отображение версии резервной копии после запуска поиска Manticore для выявления проблем на этом этапе.  
* [Commit ad2e](https://github.com/manticoresoftware/manticoresearch-backup/commit/ad2e5da) Обновлено сообщение об ошибке для неудачных подключений к демону.  
* [Commit ce5e](https://github.com/manticoresoftware/manticoresearch-backup/commit/ce5ea85) Исправлена проблема с преобразованием абсолютных путей резервных копий в относительные и удалена проверка на возможность записи при восстановлении, чтобы разрешить восстановление из разных путей.  
* [Commit db7e](https://github.com/manticoresoftware/manticoresearch-backup/commit/db7e2b9) Добавлена сортировка для итератора файлов, чтобы обеспечить согласованность в различных ситуациях.  
* [Issue #106](https://github.com/manticoresoftware/manticoresearch-backup/issues/105) Резервное копирование и восстановление нескольких конфигураций.  
* [Issue #91](https://github.com/manticoresoftware/manticoresearch-backup/pull/91) Добавлен defattr для предотвращения необычных пользовательских прав на файлы после установки на RHEL.  
* [Issue #91](https://github.com/manticoresoftware/manticoresearch-backup/pull/91) Добавлен дополнительный chown, чтобы гарантировать, что файлы по умолчанию принадлежат пользователю root в Ubuntu.  

### Связано с MCL (колоночные, вторичные, knn библиотеки)
* [Commit f104](https://github.com/manticoresoftware/columnar/commit/f104a4709bbb144140d09a09bc0fc1db1895e331) Поддержка векторного поиска.  
* [Commit 2169](https://github.com/manticoresoftware/columnar/commit/216953beef6676f7e34c9f6f1a30c5980d746710) Исправлена очистка временных файлов во время прерванной настройки сборки вторичного индекса. Это решает проблему, когда демон превышал лимит открытых файлов при создании файлов `tmp.spidx`.  
* [Commit 709b](https://github.com/manticoresoftware/columnar/commit/709b9acaaac97d9a1ca8796892f9ad432021c785) Использовать отдельную библиотеку streamvbyte для колоночного и SI.  
* [Commit 1c26](https://github.com/manticoresoftware/manticoresearch/commit/1c2623763ced14bb94ad624dde9614a938408d89) Добавлено предупреждение о том, что колоночное хранилище не поддерживает json атрибуты.  
* [Commit 3acd](https://github.com/manticoresoftware/columnar/commit/3acd6714ffce103447d7931dfc75bb1a7449f340) Исправлена распаковка данных в SI.  
* [Commit 574c](https://github.com/manticoresoftware/manticoresearch/commit/574c023152564fa72092c18ebd1f594abfd4b6d7) Исправлен сбой при сохранении дискового чанка с смешанным построчным и колоночным хранилищем.  
* [Commit e87f](https://github.com/manticoresoftware/columnar/commit/e87f088b458d4c5b1b6159fc72ed0d57b786891a) Исправлен итератор SI, который намекал на уже обработанный блок.  
* [Issue #1474](https://github.com/manticoresoftware/manticoresearch/issues/1474) Обновление не работает для построчной MVA колонки с колоночным движком.  
* [Issue #1510](https://github.com/manticoresoftware/manticoresearch/issues/1510) Исправлен сбой при агрегации к колоночному атрибуту, использованному в `HAVING`.  
* [Issue #1519](https://github.com/manticoresoftware/manticoresearch/issues/1519) Исправлен сбой в ранжировщике `expr` при использовании колоночного атрибута.  

### Связано с Docker
* ❗[Issue #42](https://github.com/manticoresoftware/docker/issues/42) Поддержка [обычной индексации](https://github.com/manticoresoftware/docker#building-plain-tables) через переменные окружения.  
* ❗[Issue #47](https://github.com/manticoresoftware/docker/issues/47) Улучшена гибкость конфигурации через переменные окружения.  
* [Issue #54](https://github.com/manticoresoftware/docker/pull/54) Улучшены процессы [резервного копирования и восстановления](https://github.com/manticoresoftware/docker#backup-and-restore) для Docker.  
* [Issue #77](https://github.com/manticoresoftware/docker/issues/76) Улучшен entrypoint для обработки восстановления резервной копии только при первом запуске.  
* [Commit a27c](https://github.com/manticoresoftware/docker/commit/a27c048dd9e73d8d6bea8d9a4830cafb486b82c5) Исправлен лог запросов в stdout.  
* [Issue #38](https://github.com/manticoresoftware/docker/issues/38) Заглушить предупреждения BUDDY, если EXTRA не установлен.  
* [Issue #71](https://github.com/manticoresoftware/docker/pull/71) Исправлено имя хоста в `manticore.conf.sh`.  

## Версия 6.2.12
Выпущена: 23 августа 2023 года  

Версия 6.2.12 продолжает серию 6.2 и устраняет проблемы, обнаруженные после выпуска 6.2.0.  

### Исправления ошибок
* ❗[Проблема #1351](https://github.com/manticoresoftware/manticoresearch/issues/1351) "Manticore 6.2.0 не запускается через systemctl на Centos 7": Изменен `TimeoutStartSec` с `infinity` на `0` для лучшей совместимости с Centos 7.
* ❗[Проблема #1364](https://github.com/manticoresoftware/manticoresearch/issues/1364) "Сбой после обновления с 6.0.4 до 6.2.0": Добавлена функция воспроизведения для пустых файлов binlog из более старых версий binlog.
* [PR #1334](https://github.com/manticoresoftware/manticoresearch/pull/1334) "исправлена опечатка в searchdreplication.cpp": Исправлена опечатка в `searchdreplication.cpp`: beggining -> beginning.
* [Проблема #1337](https://github.com/manticoresoftware/manticoresearch/issues/1337) "Manticore 6.2.0 WARNING: conn (local)(12), sock=8088: выход на неудачном заголовке MySQL, AsyncNetInputBuffer_c::AppendData: ошибка 11 (Ресурс временно недоступен) возвращает -1": Понижен уровень подробности предупреждения интерфейса MySQL о заголовке до logdebugv.
* [Проблема #1355](https://github.com/manticoresoftware/manticoresearch/issues/1355) "присоединение к кластеру зависает, когда node_address не может быть разрешен": Улучшен повторный запрос репликации, когда определенные узлы недоступны, и их разрешение имен не удается. Это должно решить проблемы в Kubernetes и Docker узлах, связанные с репликацией. Улучшено сообщение об ошибке для неудач старта репликации и внесены обновления в тестовую модель 376. Кроме того, предоставлено четкое сообщение об ошибке для неудач разрешения имен.
* [Проблема #1361](https://github.com/manticoresoftware/manticoresearch/issues/1361) "Нет сопоставления нижнего регистра для "Ø" в наборе символов non_cjk": Скорректировано сопоставление для символа 'Ø'.
* [Проблема #1365](https://github.com/manticoresoftware/manticoresearch/issues/1365) "searchd оставляет binlog.meta и binlog.001 после чистой остановки": Обеспечено правильное удаление последнего пустого файла binlog.
* [Коммит 0871](https://github.com/manticoresoftware/manticoresearch/commit/0871070845e93ad702ae0dfb89b35e5f24cd0851): Исправлена проблема сборки `Thd_t` на Windows, связанная с ограничениями атомарного копирования.
* [Коммит 1cc0](https://github.com/manticoresoftware/manticoresearch/commit/1cc04bedc174737118fd5a3663e35f4e95bd4f8c): Решена проблема с FT CBO против `ColumnarScan`.
* [Коммит c6bf](https://github.com/manticoresoftware/manticoresearch/commit/c6bfb30d53e3f4b67f17769667733677a661c95b): Внесены исправления в тест 376 и добавлена замена для ошибки `AF_INET` в тесте.
* [Коммит cbc3](https://github.com/manticoresoftware/manticoresearch/commit/cbc38705e0b4dcc9665e3e8cf64826290ebd11cf): Решена проблема взаимной блокировки во время репликации при обновлении атрибутов blob по сравнению с заменой документов. Также удален rlock индекса во время коммита, так как он уже заблокирован на более базовом уровне.

### Небольшие изменения
* [Коммит 4f91](https://github.com/manticoresoftware/manticoresearch/commit/4f913a36e84e01f950b2797051222b0b1094d6fe) Обновлена информация о конечных точках `/bulk` в руководстве.

### MCL
* Поддержка [Manticore Columnar Library](https://github.com/manticoresoftware/columnar) v2.2.4

## Версия 6.2.0
Выпущено: 4 августа 2023 года

### Основные изменения
* Оптимизатор запросов был улучшен для поддержки полнотекстовых запросов, что значительно повысило эффективность поиска и производительность.
* Интеграции с:
  - [mysqldump](https://dev.mysql.com/doc/refman/8.0/en/mysqldump.html) - для [создания логических резервных копий](Securing_and_compacting_a_table/Backup_and_restore.md#Backup-and-restore-with-mysqldump) с использованием `mysqldump`
  - [Apache Superset](https://manticoresearch.com/blog/manticoresearch-apache-superset-integration/) и [Grafana](https://manticoresearch.com/blog/manticoresearch-grafana-integration/) для визуализации данных, хранящихся в Manticore
  - [HeidiSQL](https://www.heidisql.com/) и [DBForge](https://www.devart.com/dbforge/) для упрощения разработки с Manticore
* Мы начали использовать [GitHub workflows](https://github.com/manticoresoftware/manticoresearch/actions), что упрощает участникам использование того же процесса Непрерывной Интеграции (CI), который применяет основная команда при подготовке пакетов. Все задания могут выполняться на хостах GitHub, что облегчает тестирование изменений в вашем форке Manticore Search.
* Мы начали использовать [CLT](https://github.com/manticoresoftware/clt) для тестирования сложных сценариев. Например, теперь мы можем гарантировать, что пакет, собранный после коммита, может быть правильно установлен на всех поддерживаемых операционных системах Linux. Командный тестер (CLT) предоставляет удобный способ записи тестов в интерактивном режиме и их легкого воспроизведения.
* Значительное улучшение производительности операции подсчета уникальных значений за счет использования комбинации хеш-таблиц и HyperLogLog.
* Включено многопоточное выполнение запросов, содержащих вторичные индексы, с количеством потоков, ограниченным количеством физических ядер ЦП. Это должно значительно улучшить скорость выполнения запросов.
* `pseudo_sharding` был скорректирован, чтобы ограничиваться количеством свободных потоков. Это обновление значительно повышает производительность пропускной способности.
* Пользователи теперь могут указывать [движок хранения атрибутов по умолчанию](Server_settings/Searchd.md#engine) через настройки конфигурации, что обеспечивает лучшую настройку в соответствии с конкретными требованиями нагрузки.
* Поддержка [Manticore Columnar Library 2.2.0](https://github.com/manticoresoftware/columnar/) с многочисленными исправлениями ошибок и улучшениями в [вторичных индексах](Server_settings/Searchd.md#secondary_indexes).

### Небольшие изменения
* [Buddy #153](https://github.com/manticoresoftware/manticoresearch-buddy/issues/153): HTTP-эндпоинт [/pq](Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md?client=JSON#Adding-rules-to-a-percolate-table) теперь служит псевдонимом для HTTP-эндпоинта `/json/pq`.
* [Commit 0bf1](https://github.com/manticoresoftware/manticoresearch/commit/0bf17d9e): Мы обеспечили совместимость с многобайтовыми символами для `upper()` и `lower()`.
* [Commit 2bb9](https://github.com/manticoresoftware/manticoresearch/commit/2bb92765): Вместо сканирования индекса для запросов `count(*)` теперь возвращается предрассчитанное значение.
* [Commit 3c84](https://github.com/manticoresoftware/manticoresearch/commit/3c84091f): Теперь возможно использовать `SELECT` для выполнения произвольных вычислений и отображения `@@sysvars`. В отличие от предыдущего, вы больше не ограничены только одним вычислением. Поэтому запросы, такие как `select user(), database(), @@version_comment, version(), 1+1 as a limit 10`, вернут все столбцы. Обратите внимание, что необязательный 'limit' всегда будет игнорироваться.
* [Commit 6aca](https://github.com/manticoresoftware/manticoresearch/commit/6aca32f0): Реализован заглушечный запрос `CREATE DATABASE`.
* [Commit 9dc1](https://github.com/manticoresoftware/manticoresearch/commit/9dc12334): При выполнении `ALTER TABLE table REBUILD SECONDARY` вторичные индексы теперь всегда перестраиваются, даже если атрибуты не были обновлены.
* [Commit 46ed](https://github.com/manticoresoftware/manticoresearch/commit/46edb089): Сортировщики, использующие предрассчитанные данные, теперь идентифицируются перед использованием CBO, чтобы избежать ненужных расчетов CBO.
* [Commit 102a](https://github.com/manticoresoftware/manticoresearch/commit/102ac604): Реализация имитации и использование стека выражений полнотекстового поиска для предотвращения сбоев демона.
* [Commit 979f](https://github.com/manticoresoftware/manticoresearch/commit/979fa27c): Быстрый код был добавлен для клонирования совпадений для совпадений, которые не используют строковые/массивные/json атрибуты.
* [Commit a073](https://github.com/manticoresoftware/manticoresearch/commit/a0735ffe): Добавлена поддержка команды `SELECT DATABASE()`. Однако она всегда будет возвращать `Manticore`. Это дополнение имеет решающее значение для интеграций с различными инструментами MySQL.
* [Commit bc04](https://github.com/manticoresoftware/manticoresearch/commit/bc04908): Изменен формат ответа эндпоинта [/cli](Connecting_to_the_server/HTTP.md#/cli) и добавлен эндпоинт `/cli_json`, который будет функционировать как предыдущий `/cli`.
* [Commit d70b](https://github.com/manticoresoftware/manticoresearch/commit/d70b0d58): `thread_stack` теперь можно изменять во время выполнения с помощью оператора `SET`. Доступны как локальные для сессии, так и глобальные для демона варианты. Текущие значения можно получить в выводе `show variables`.
* [Commit d96e](https://github.com/manticoresoftware/manticoresearch/commit/d96ec6b9): Код был интегрирован в CBO для более точной оценки сложности выполнения фильтров по строковым атрибутам.
* [Commit e77d](https://github.com/manticoresoftware/manticoresearch/commit/e77dd72f5a04531c352fad0d7afcd2a1cbae2510): Расчет стоимости DocidIndex был улучшен, что повысило общую производительность.
* [Commit f3ae](https://github.com/manticoresoftware/manticoresearch/commit/f3ae8bea): Метрики загрузки, аналогичные 'uptime' в Linux, теперь видны в команде `SHOW STATUS`.
* [Commit f3cc](https://github.com/manticoresoftware/manticoresearch/commit/f3cc0971): Порядок полей и атрибутов для `DESC` и `SHOW CREATE TABLE` теперь соответствует порядку `SELECT * FROM`.
* [Commit f3d2](https://github.com/manticoresoftware/manticoresearch/commit/f3d248a6): Разные внутренние парсеры теперь предоставляют свой внутренний мнемонический код (например, `P01`) при различных ошибках. Это улучшение помогает определить, какой парсер вызвал ошибку, и также скрывает несущественные внутренние детали.
* [Issue #271](https://github.com/manticoresoftware/manticoresearch/issues/271) "Иногда CALL SUGGEST не предлагает исправление для опечатки в одну букву": Улучшено поведение [SUGGEST/QSUGGEST](Searching/Spell_correction.md#CALL-QSUGGEST,-CALL-SUGGEST) для коротких слов: добавлена опция `sentence`, чтобы показать целое предложение.
* [Issue #696](https://github.com/manticoresoftware/manticoresearch/issues/696) "Индекс перколяции не ищет правильно по запросу точной фразы, когда включено стеммирование": Запрос перколяции был изменен для обработки модификатора точного термина, улучшая функциональность поиска.
* [Issue #829](https://github.com/manticoresoftware/manticoresearch/issues/829) "Методы форматирования даты": добавлено выражение списка выбора [date_format()](Functions/Date_and_time_functions.md#DATE_FORMAT()), которое открывает функцию `strftime()`.
* [Issue #961](https://github.com/manticoresoftware/manticoresearch/issues/961) "Сортировка ведер через HTTP JSON API": введено необязательное [свойство сортировки](Searching/Faceted_search.md#HTTP-JSON) для каждого ведра агрегатов в HTTP-интерфейсе.
* [Issue #1062](https://github.com/manticoresoftware/manticoresearch/issues/1062) "Improve error logging of JSON insert api failure - "unsupported value type": Эндпоинт `/bulk` сообщает информацию о количестве обработанных и необработанных строк (документов) в случае ошибки.
* [Issue #1070](https://github.com/manticoresoftware/manticoresearch/issues/1070) "Подсказки CBO не поддерживают несколько атрибутов": Включены подсказки индекса для обработки нескольких атрибутов.
* [Issue #1106](https://github.com/manticoresoftware/manticoresearch/issues/1106) "Добавить теги к http поисковому запросу": Теги были добавлены к [HTTP PQ ответам](Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md?client=JSON#Adding-rules-to-a-percolate-table).
* [Issue #1301](https://github.com/manticoresoftware/manticoresearch/issues/1301) "buddy не должен создавать таблицу параллельно": Решена проблема, из-за которой происходили сбои при параллельных операциях CREATE TABLE. Теперь только одна операция `CREATE TABLE` может выполняться одновременно.
* [Issue #1303](https://github.com/manticoresoftware/manticoresearch/issues/1303) "добавить поддержку @ в названиях столбцов".
* [Issue #1316](https://github.com/manticoresoftware/manticoresearch/issues/1316) "Запросы к набору данных такси медленные с ps=1": Логика CBO была уточнена, и разрешение по умолчанию для гистограммы было установлено на 8k для лучшей точности по атрибутам с случайно распределенными значениями.
* [Issue #1317](https://github.com/manticoresoftware/manticoresearch/issues/1317) "Исправить CBO против полнотекстового поиска на наборе данных hn": Улучшенная логика была реализована для определения, когда использовать пересечение итераторов битовых карт и когда использовать приоритетную очередь.
* [Issue #1318](https://github.com/manticoresoftware/manticoresearch/issues/1318) "колоночный: изменить интерфейс итератора на однократный вызов": Колонковые итераторы теперь используют один вызов `Get`, заменяя предыдущие двухшаговые вызовы `AdvanceTo` + `Get` для получения значения.
* [Issue #1319](https://github.com/manticoresoftware/manticoresearch/issues/1319) "Ускорение расчета агрегатов (удалить CheckReplaceEntry?)": Вызов `CheckReplaceEntry` был удален из группового сортировщика для ускорения расчета агрегатных функций.
* [Issue #1320](https://github.com/manticoresoftware/manticoresearch/issues/1320) "создать таблицу read_buffer_docs/hits не понимает синтаксис k/m/g": Опции `CREATE TABLE` `read_buffer_docs` и `read_buffer_hits` теперь поддерживают синтаксис k/m/g.
* [Языковые пакеты](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) для английского, немецкого и русского языков теперь можно легко установить на Linux, выполнив команду `apt/yum install manticore-language-packs`. На macOS используйте команду `brew install manticoresoftware/tap/manticore-language-packs`.
* Порядок полей и атрибутов теперь согласован между операциями `SHOW CREATE TABLE` и `DESC`.
* Если на диске недостаточно места при выполнении запросов `INSERT`, новые запросы `INSERT` будут завершаться неудачей, пока не станет доступно достаточно места на диске.
* Функция преобразования типа [UINT64()](Functions/Type_casting_functions.md#UINT64%28%29) была добавлена.
* Эндпоинт `/bulk` теперь обрабатывает пустые строки как команду [commit](Data_creation_and_modification/Transactions.md#BEGIN,-COMMIT,-and-ROLLBACK). Дополнительная информация [здесь](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md?client=JSON#Bulk-adding-documents).
* Предупреждения были реализованы для [недействительных подсказок индекса](Searching/Options.md#Query-optimizer-hints), обеспечивая большую прозрачность и позволяя смягчить ошибки.
* Когда `count(*)` используется с одним фильтром, запросы теперь используют предрассчитанные данные из вторичных индексов, когда это возможно, значительно ускоряя время выполнения запросов.

### ⚠️ Ломающие изменения
* ⚠️ Таблицы, созданные или измененные в версии 6.2.0, не могут быть прочитаны более старыми версиями.
* ⚠️ Идентификаторы документов теперь обрабатываются как беззнаковые 64-битные целые числа во время индексации и операций INSERT.
* ⚠️ Синтаксис для подсказок оптимизатора запросов был обновлен. Новый формат: `/*+ SecondaryIndex(uid) */`. Обратите внимание, что старый синтаксис больше не поддерживается.
* ⚠️ [Проблема #1160](https://github.com/manticoresoftware/manticoresearch/issues/1160): Использование `@` в именах таблиц было запрещено для предотвращения конфликтов синтаксиса.
* ⚠️ Строковые поля/атрибуты, помеченные как `indexed` и `attribute`, теперь рассматриваются как одно поле во время операций `INSERT`, `DESC` и `ALTER`.
* ⚠️ [Проблема #1057](https://github.com/manticoresoftware/manticoresearch/issues/1057): Библиотеки MCL больше не будут загружаться на системах, которые не поддерживают SSE 4.2.
* ⚠️ [Проблема #1143](https://github.com/manticoresoftware/manticoresearch/issues/1143): [agent_query_timeout](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_query_timeout) был сломан. Исправлено и теперь эффективно.

### Исправления ошибок
* [Коммит 2a6e](https://github.com/manticoresoftware/manticoresearch/commit/2a6ea8f7) "Сбой при DROP TABLE": решена проблема, вызывавшая длительное ожидание завершения операций записи (оптимизация, сохранение дискового чанка) на RT таблице при выполнении оператора DROP TABLE. Добавлено предупреждение, чтобы уведомить, когда директория таблицы не пуста после выполнения команды DROP TABLE.
* [Коммит 2ebd](https://github.com/manticoresoftware/manticoresearch/commit/2ebd424d): Добавлена поддержка колоннарных атрибутов, которые отсутствовали в коде, используемом для группировки по нескольким атрибутам.
* [Коммит 3be4](https://github.com/manticoresoftware/manticoresearch/commit/3be4503f752216b37b8cde8e47fd2d77bbce63ae) Исправлена проблема сбоя, потенциально вызванная нехваткой дискового пространства, путем правильной обработки ошибок записи в binlog.
* [Коммит 6adb](https://github.com/manticoresoftware/manticoresearch/commit/6adb0934): Исправлен сбой, который иногда происходил при использовании нескольких колоннарных итераторов сканирования (или итераторов вторичного индекса) в запросе.
* [Коммит 6bd9](https://github.com/manticoresoftware/manticoresearch/commit/6bd9f709): Фильтры не удалялись при использовании сортировщиков, которые используют предрассчитанные данные. Эта проблема была исправлена.
* [Коммит 6d03](https://github.com/manticoresoftware/manticoresearch/commit/6d03566a): Код CBO был обновлен для предоставления лучших оценок для запросов, использующих фильтры по строковым атрибутам, выполняемым в нескольких потоках.
* [Коммит 6dd3](https://github.com/manticoresoftware/manticoresearch/commit/6dd3964), [Helm #56](https://github.com/manticoresoftware/manticoresearch-helm/issues/56) "фатальный дамп сбоя в кластере Kubernetes": Исправлен дефектный bloom-фильтр для корневого объекта JSON; исправлен сбой демона из-за фильтрации по полю JSON.
* [Коммит 6e1b](https://github.com/manticoresoftware/manticoresearch/commit/6e1b849) Исправлен сбой демона, вызванный недействительным конфигом `manticore.json`.
* [Коммит 6fbc](https://github.com/manticoresoftware/manticoresearch/commit/6fbc4189) Исправлен фильтр диапазона json для поддержки значений int64.
* [Коммит 9c67](https://github.com/manticoresoftware/manticoresearch/commit/9c677426) Файлы `.sph` могли быть повреждены `ALTER`. Исправлено.
* [Коммит 77cc](https://github.com/manticoresoftware/manticoresearch/commit/77ccd8f): Добавлен общий ключ для репликации оператора замены, чтобы решить ошибку `pre_commit`, возникающую при репликации замены с нескольких мастер-узлов.
* [Коммит 2884](https://github.com/manticoresoftware/manticoresearch/commit/2884519e) решены проблемы с проверками bigint по функциям, таким как 'date_format()'.
* [Коммит 9513](https://github.com/manticoresoftware/manticoresearch/commit/9513aca5): Итераторы больше не отображаются в [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META), когда сортировщики используют предрассчитанные данные.
* [Коммит a2a7](https://github.com/manticoresoftware/manticoresearch/commit/a2a70555): Размер стека узла полнотекстового поиска был обновлен, чтобы предотвратить сбои при сложных полнотекстовых запросах.
* [Коммит a062](https://github.com/manticoresoftware/manticoresearch/commit/a0626d7e): Исправлена ошибка, вызывавшая сбой во время репликации обновлений с атрибутами JSON и строками.
* [Коммит b3e6](https://github.com/manticoresoftware/manticoresearch/commit/b3e63e5d): Строковый сборщик был обновлен для использования 64-битных целых чисел, чтобы избежать сбоев при работе с большими наборами данных.
* [Коммит c472](https://github.com/manticoresoftware/manticoresearch/commit/c472e5b): Устранен сбой, который происходил при подсчете уникальных значений по нескольким индексам.
* [Коммит d073](https://github.com/manticoresoftware/manticoresearch/commit/d0730272): Исправлена проблема, когда запросы по дисковым чанкам RT индексов могли выполняться в нескольких потоках, даже если `pseudo_sharding` был отключен.
* [Коммит d205](https://github.com/manticoresoftware/manticoresearch/commit/d205508e) Набор значений, возвращаемых командой `show index status`, был изменен и теперь варьируется в зависимости от типа используемого индекса.
* [Коммит e9bc](https://github.com/manticoresoftware/manticoresearch/commit/e9bc2f9) Исправлена ошибка HTTP при обработке массовых запросов и проблема, когда ошибка не возвращалась клиенту из сетевого цикла.
* [Коммит f77c](https://github.com/manticoresoftware/manticoresearch/commit/f77ce0e6) использование расширенного стека для PQ.
* [Коммит fac2](https://github.com/manticoresoftware/manticoresearch/commit/fac2d9687a20582765aaa89be9597804c4f845af) Обновлен вывод ранжировщика экспорта для соответствия [packedfactors()](Functions/Searching_and_ranking_functions.md#PACKEDFACTORS%28%29).
* [Коммит ff87](https://github.com/manticoresoftware/manticoresearch/commit/ff87cc4): Исправлена проблема со строковым списком в фильтре журнала запросов SphinxQL.
* [Проблема #589](https://github.com/manticoresoftware/manticoresearch/issues/589) "Определение кодировки, похоже, зависит от порядка кодов": Исправлено неправильное сопоставление кодировки для дубликатов.
* [Проблема #811](https://github.com/manticoresoftware/manticoresearch/issues/811) "Сопоставление нескольких слов в формах слов мешает фразовому поиску с CJK знаками препинания между ключевыми словами": Исправлена позиция токена ngram в фразовом запросе с формами слов.
* [Проблема #834](https://github.com/manticoresoftware/manticoresearch/issues/834) "Знак равенства в поисковом запросе нарушает запрос": Обеспечено экранирование точного символа и исправлено двойное точное расширение с помощью опции `expand_keywords`.
* [Проблема #864](https://github.com/manticoresoftware/manticoresearch/issues/864) "конфликт исключений/стоп-слов"
* [Проблема #910](https://github.com/manticoresoftware/manticoresearch/issues/910) "Сбой Manticore при вызове call snippets() с libstemmer_fr и index_exact_words": Устранены внутренние конфликты, вызывающие сбои при вызове `SNIPPETS()`.
* [Проблема #946](https://github.com/manticoresoftware/manticoresearch/issues/946) "Дублирующиеся записи во время SELECT": Исправлена проблема дублирующихся документов в наборе результатов для запроса с опцией `not_terms_only_allowed` к RT индексу с убитыми документами.
* [Проблема #967](https://github.com/manticoresoftware/manticoresearch/issues/967) "Использование JSON аргументов в UDF функциях приводит к сбою": Исправлен сбой демона при обработке поиска с включенным псевдо-шардингом и UDF с аргументом JSON.
* [Проблема #1050](https://github.com/manticoresoftware/manticoresearch/issues/1050) "count(*) в FEDERATED": Исправлен сбой демона, возникающий при запросе через движок `FEDERATED` с агрегатом.
* [Проблема #1052](https://github.com/manticoresoftware/manticoresearch/issues/1052) Исправлена проблема, когда столбец `rt_attr_json` был несовместим с колоннарным хранилищем.
* [Проблема #1072](https://github.com/manticoresoftware/manticoresearch/issues/1072) "* удаляется из поискового запроса с ignore_chars": Исправлена эта проблема, чтобы подстановочные знаки в запросе не затрагивались `ignore_chars`.
* [Проблема #1075](https://github.com/manticoresoftware/manticoresearch/issues/1075) "indextool --check не работает, если есть распределенная таблица": indextool теперь совместим с экземплярами, имеющими 'distributed' и 'template' индексы в json конфиге.
* [Проблема #1081](https://github.com/manticoresoftware/manticoresearch/issues/1081) "особый select на особом RT наборе данных приводит к сбою searchd": Устранен сбой демона при запросе с packedfactors и большим внутренним буфером.
* [Проблема #1095](https://github.com/manticoresoftware/manticoresearch/issues/1095) "С удаленными документами игнорируются not_terms_only_allowed"
* [Проблема #1099](https://github.com/manticoresoftware/manticoresearch/issues/1099) "indextool --dumpdocids не работает": Восстановлена функциональность команды `--dumpdocids`.
* [Проблема #1100](https://github.com/manticoresoftware/manticoresearch/issues/1100) "indextool --buildidf не работает": indextool теперь закрывает файл после завершения globalidf.
* [Проблема #1104](https://github.com/manticoresoftware/manticoresearch/issues/1104) "Count(*) пытается обрабатываться как схема, установленная в удаленных таблицах": Исправлена проблема, когда сообщение об ошибке отправлялось демоном для запросов в распределенный индекс, когда агент возвращал пустой набор результатов.
* [Проблема #1109](https://github.com/manticoresoftware/manticoresearch/issues/1109) "FLUSH ATTRIBUTES зависает с threads=1".
* [Проблема #1126](https://github.com/manticoresoftware/manticoresearch/issues/1126) "Потеряно соединение с сервером MySQL во время запроса - manticore 6.0.5": Устранены сбои, которые происходили при использовании нескольких фильтров по колоннарным атрибутам.
* [Проблема #1135](https://github.com/manticoresoftware/manticoresearch/issues/1135) "Чувствительность к регистру фильтрации строк JSON": Исправлена сортировка для правильной работы фильтров, используемых в HTTP поисковых запросах.
* [Проблема #1140](https://github.com/manticoresoftware/manticoresearch/issues/1140) "Совпадение в неправильном поле": Исправлены повреждения, связанные с `morphology_skip_fields`.
* [Проблема #1155](https://github.com/manticoresoftware/manticoresearch/issues/1155) "системные удаленные команды через API должны передавать g_iMaxPacketSize": Внесены обновления для обхода проверки `max_packet_size` для команд репликации между узлами. Кроме того, последняя ошибка кластера была добавлена в отображение статуса.
* [Проблема #1302](https://github.com/manticoresoftware/manticoresearch/issues/1302) "временные файлы остаются после неудачной оптимизации": Исправлена проблема, когда временные файлы оставались после ошибки, произошедшей во время процесса слияния или оптимизации.
* [Проблема #1304](https://github.com/manticoresoftware/manticoresearch/issues/1304) "добавить переменную окружения для времени ожидания запуска buddy": Добавлена переменная окружения `MANTICORE_BUDDY_TIMEOUT` (по умолчанию 3 секунды) для управления временем ожидания демона для сообщения buddy при запуске.
* [Проблема #1305](https://github.com/manticoresoftware/manticoresearch/issues/1305) "Переполнение целого числа при сохранении метаданных PQ": Уменьшено чрезмерное потребление памяти демоном при сохранении большого индекса PQ.
* [Проблема #1306](https://github.com/manticoresoftware/manticoresearch/issues/1306) "Невозможно воссоздать RT таблицу после изменения ее внешнего файла": Исправлена ошибка изменения с пустой строкой для внешних файлов; исправлены внешние файлы RT индекса, оставшиеся после изменения внешних файлов.
* [Проблема #1307](https://github.com/manticoresoftware/manticoresearch/issues/1307) "Запрос SELECT sum(value) as value не работает должным образом": Исправлена проблема, когда выражение списка выбора с псевдонимом могло скрыть атрибут индекса; также исправлен суммирование для подсчета в int64 для целого числа.
* [Проблема #1308](https://github.com/manticoresoftware/manticoresearch/issues/1308) "Избегать привязки к localhost в репликации": Обеспечено, чтобы репликация не привязывалась к localhost для имен хостов с несколькими IP.
* [Проблема #1309](https://github.com/manticoresoftware/manticoresearch/issues/1309) "ответ клиенту mysql не удался для данных больше 16Mb": Исправлена проблема возврата пакета SphinxQL размером более 16Mb клиенту.
* [Проблема #1310](https://github.com/manticoresoftware/manticoresearch/issues/1310) "неправильная ссылка в "пути к внешним файлам должны быть абсолютными": Исправлено отображение полного пути к внешним файлам в `SHOW CREATE TABLE`.
* [Проблема #1311](https://github.com/manticoresoftware/manticoresearch/issues/1311) "сборка отладки сбоит на длинных строках в фрагментах": Теперь длинные строки (>255 символов) разрешены в тексте, на который нацелена функция `SNIPPET()`.
* [Проблема #1312](https://github.com/manticoresoftware/manticoresearch/issues/1312) "случайный сбой при использовании после удаления в опросе kqueue (master-agent)": Исправлены сбои, когда мастер не может подключиться к агенту на системах с управлением kqueue (FreeBSD, MacOS и т.д.).
* [Проблема #1313](https://github.com/manticoresoftware/manticoresearch/issues/1313) "слишком долгое соединение с самим собой": При подключении от мастера к агентам на MacOS/BSD теперь используется единый тайм-аут соединения+запроса вместо простого соединения.
* [Проблема #1314](https://github.com/manticoresoftware/manticoresearch/issues/1314) "pq (json meta) с недостижимыми встроенными синонимами не загружается": Исправлен флаг встроенных синонимов в pq.
* [Проблема #1315](https://github.com/manticoresoftware/manticoresearch/issues/1315) "Разрешить некоторым функциям (sint, fibonacci, second, minute, hour, day, month, year, yearmonth, yearmonthday) использовать неявно продвигаемые значения аргументов".
* [Проблема #1321](https://github.com/manticoresoftware/manticoresearch/issues/1321) "Включить многопоточную SI в полном сканировании, но ограничить потоки": Код был реализован в CBO для лучшего прогнозирования многопоточной производительности вторичных индексов, когда они используются в полнотекстовом запросе.
* [Проблема #1322](https://github.com/manticoresoftware/manticoresearch/issues/1322) "запросы count(*) все еще медленные после использования предрассчитанных сортировщиков": Итераторы больше не инициализируются при использовании сортировщиков, которые используют предрассчитанные данные, что позволяет избежать негативных эффектов производительности.
* [Проблема #1411](https://github.com/manticoresoftware/manticoresearch/issues/1141) "журнал запросов в sphinxql не сохраняет оригинальные запросы для MVA": Теперь `all()/any()` записывается в журнал.

## Версия 6.0.4
Выпущено: 15 марта 2023

### Новые функции
* Улучшенная интеграция с Logstash, Beats и т.д., включая:
  - Поддержка версий Logstash 7.6 - 7.15, версий Filebeat 7.7 - 7.12
  - Поддержка авто-схемы.
  - Добавлена обработка массовых запросов в формате, похожем на Elasticsearch.
* [Коммит Buddy ce90](https://github.com/manticoresoftware/manticoresearch-buddy/commit/ce907ea) Версия Log Buddy при запуске Manticore.

### Исправления ошибок
* [Проблема #588](https://github.com/manticoresoftware/manticoresearch/issues/588), [Проблема #942](https://github.com/manticoresoftware/manticoresearch/issues/942) Исправлен неправильный символ в метаданных поиска и ключевых словах для биграммного индекса.
* [Проблема #1027](https://github.com/manticoresoftware/manticoresearch/issues/1027) Заголовки HTTP в нижнем регистре отклоняются.
* ❗[Проблема #1039](https://github.com/manticoresoftware/manticoresearch/issues/1039) Исправлена утечка памяти в демоне при чтении вывода консоли Buddy.
* [Проблема #1056](https://github.com/manticoresoftware/manticoresearch/issues/1056) Исправлено неожиданное поведение вопросительного знака.
* [Проблема #1064](https://github.com/manticoresoftware/manticoresearch/issues/1064) - Исправлено состояние гонки в таблицах токенизатора в нижнем регистре, вызывающее сбой.
* [Коммит 59bb](https://github.com/manticoresoftware/manticoresearch/commit/59bb54c) Исправлена обработка массовых записей в JSON-интерфейсе для документов с явно установленным id в null.
* [Коммит 7b6b](https://github.com/manticoresoftware/manticoresearch/commit/7b6b25f) Исправлена статистика терминов в CALL KEYWORDS для нескольких одинаковых терминов.
* [Коммит f381](https://github.com/manticoresoftware/manticoresearch/commit/f381ad2) Конфигурация по умолчанию теперь создается установщиком Windows; пути больше не заменяются во время выполнения.
* [Коммит 6940](https://github.com/manticoresoftware/manticoresearch/commit/6940e95), [Коммит cc5a](https://github.com/manticoresoftware/manticoresearch/commit/cc5a480) Исправлены проблемы с репликацией для кластера с узлами в нескольких сетях.
* [Коммит 4972](https://github.com/manticoresoftware/manticoresearch/commit/49722ab) Исправлен HTTP-эндпоинт `/pq`, чтобы он был псевдонимом для HTTP-эндпоинта `/json/pq`.
* [Коммит 3b53](https://github.com/manticoresoftware/manticoresearch/commit/3b5385a) Исправлен сбой демона при перезапуске Buddy.
* [Коммит Buddy fba9](https://github.com/manticoresoftware/manticoresearch-buddy/commit/fba9c8c) Отображение оригинальной ошибки при получении недействительного запроса.
* [Коммит Buddy db95](https://github.com/manticoresoftware/manticoresearch-buddy/commit/db9532c) Разрешены пробелы в пути резервного копирования и добавлена магия к regexp для поддержки одинарных кавычек.

## Версия 6.0.2
Выпущено: 10 февраля 2023

### Исправления ошибок
* [Проблема #1024 сбой 2](https://github.com/manticoresoftware/manticoresearch/issues/1024) Сбой / Ошибка сегментации при поиске по фасетам с большим количеством результатов
* ❗[Проблема #1029](https://github.com/manticoresoftware/manticoresearch/issues/1029) - ПРЕДУПРЕЖДЕНИЕ: Скомпилированное значение KNOWN_CREATE_SIZE (16) меньше измеренного (208). Рассмотрите возможность исправления значения!
* ❗[Проблема #1032](https://github.com/manticoresoftware/manticoresearch/issues/1032) - Простой индекс Manticore 6.0.0 вызывает сбой
* ❗[Проблема #1033](https://github.com/manticoresoftware/manticoresearch/issues/1033) - множественные распределенные теряются при перезапуске демона
* ❗[Проблема #1064](https://github.com/manticoresoftware/manticoresearch/issues/1064) - состояние гонки в таблицах токенизатора в нижнем регистре

## Версия 6.0.0
Выпущено: 7 февраля 2023

Начиная с этого выпуска, Manticore Search поставляется с Manticore Buddy, демоном-сайдкаром, написанным на PHP, который обрабатывает высокоуровневую функциональность, не требующую супернизкой задержки или высокой пропускной способности. Manticore Buddy работает за кулисами, и вы можете даже не осознавать, что он работает. Хотя он невидим для конечного пользователя, это была значительная задача сделать Manticore Buddy легко устанавливаемым и совместимым с основным демоном на C++. Это главное изменение позволит команде разрабатывать широкий спектр новых высокоуровневых функций, таких как оркестрация шардов, контроль доступа и аутентификация, а также различные интеграции, такие как mysqldump, DBeaver, Grafana mysql connector. На данный момент он уже обрабатывает [SHOW QUERIES](Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES), [BACKUP](Securing_and_compacting_a_table/Backup_and_restore.md#BACKUP-SQL-command-reference) и [Авто-схему](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema).

Этот выпуск также включает более 130 исправлений ошибок и множество функций, многие из которых можно считать основными.

### Основные изменения
* 🔬 Экспериментально: теперь вы можете выполнять совместимые с Elasticsearch [вставки](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md?example=Elasticsearch#Adding-documents-to-a-real-time-table) и [замены](Data_creation_and_modification/Updating_documents/REPLACE.md?example=Elasticsearch#REPLACE) JSON-запросов, что позволяет использовать Manticore с такими инструментами, как Logstash (версия < 7.13), Filebeat и другими инструментами из семейства Beats. Включено по умолчанию. Вы можете отключить это, используя `SET GLOBAL ES_COMPAT=off`.
* Поддержка [Manticore Columnar Library 2.0.0](https://github.com/manticoresoftware/columnar/) с многочисленными исправлениями и улучшениями в [вторичных индексах](Server_settings/Searchd.md#secondary_indexes). **⚠️ СУЩЕСТВЕННОЕ ИЗМЕНЕНИЕ**: Вторичные индексы включены по умолчанию с этой версии. Убедитесь, что вы выполняете [ALTER TABLE table_name REBUILD SECONDARY](Updating_table_schema_and_settings.md#Rebuilding-a-secondary-index), если вы обновляетесь с Manticore 5. См. ниже для получения дополнительных сведений.
* [Коммит c436](https://github.com/manticoresoftware/manticoresearch/commit/c436f9023536f767610451911955ae36d90aa638) Авто-схема: теперь вы можете пропустить создание таблицы, просто вставив первый документ, и Manticore автоматически создаст таблицу на основе его полей. Читайте об этом более подробно [здесь](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema). Вы можете включить или отключить это, используя [searchd.auto_schema](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema).
* Огромная переработка [оптимизатора на основе стоимости](Searching/Cost_based_optimizer.md), который снижает время отклика запросов во многих случаях.
  - [Проблема #1008](https://github.com/manticoresoftware/manticoresearch/issues/1008) Оценка производительности параллелизации в CBO.
  - [Проблема #1014](https://github.com/manticoresoftware/manticoresearch/issues/1014) CBO теперь учитывает [вторичные индексы](Server_settings/Searchd.md#secondary_indexes) и может действовать более разумно.
  - [Коммит cef9](https://github.com/manticoresoftware/columnar/commit/cef9b54dc3a4ee9138ebc4df3a671715b996d6b3) Статистика кодирования колоннарных таблиц/полей теперь хранится в метаданных, чтобы помочь CBO принимать более разумные решения.
  - [Коммит 2b95](https://github.com/manticoresoftware/manticoresearch/commit/2b95fb8c2ba422c9c5a66e2b61c1f7922f31c7aa) Добавлены подсказки CBO для тонкой настройки его поведения.
* [Телеметрия](Telemetry.md#Telemetry): мы рады объявить о добавлении телеметрии в этом релизе. Эта функция позволяет нам собирать анонимные и деперсонализированные метрики, которые помогут нам улучшить производительность и пользовательский опыт нашего продукта. Будьте уверены, все собранные данные **абсолютно анонимны и не будут связаны с какой-либо личной информацией**. Эта функция может быть [легко отключена](Telemetry.md#Telemetry) в настройках, если это необходимо.
* [Коммит 5aaf](https://github.com/manticoresoftware/manticoresearch/commit/5aafb17d81b601e399e212a344a4738f65576cdd) [ALTER TABLE table_name REBUILD SECONDARY](Updating_table_schema_and_settings.md#Rebuilding-a-secondary-index) для перестройки вторичных индексов, когда вам это нужно, например:
  - когда вы мигрируете с Manticore 5 на более новую версию,
  - когда вы выполнили [UPDATE](Data_creation_and_modification/Updating_documents/UPDATE.md#UPDATE) (т.е. [обновление на месте, а не замена](Data_creation_and_modification/Updating_documents/REPLACE_vs_UPDATE.md)) атрибута в индексе
* [Проблема #821](https://github.com/manticoresoftware/manticoresearch/issues/821) Новый инструмент `manticore-backup` для [резервного копирования и восстановления экземпляра Manticore](Securing_and_compacting_a_table/Backup_and_restore.md)
* SQL команда [BACKUP](Securing_and_compacting_a_table/Backup_and_restore.md#BACKUP-SQL-command-reference) для выполнения резервного копирования изнутри Manticore.
* SQL команда [SHOW QUERIES](Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES) как простой способ увидеть выполняющиеся запросы, а не потоки.
* [Проблема #551](https://github.com/manticoresoftware/manticoresearch/issues/551) SQL команда `KILL` для завершения длительного `SELECT`.
* Динамический `max_matches` для агрегирующих запросов для повышения точности и снижения времени отклика.

### Небольшие изменения
* [Проблема #822](https://github.com/manticoresoftware/manticoresearch/issues/822) SQL команды [FREEZE/UNFREEZE](Securing_and_compacting_a_table/Freezing_and_locking_a_table.md) для подготовки реального/обычного таблицы к резервному копированию.
* [Коммит c470](https://github.com/manticoresoftware/manticoresearch/commit/c47052f18ad4868134de1a97e8d7da570e531dfa) Новые настройки `accurate_aggregation` и `max_matches_increase_threshold` для контролируемой точности агрегации.
* [Проблема #718](https://github.com/manticoresoftware/manticoresearch/issues/718) Поддержка подписанных отрицательных 64-битных идентификаторов. Обратите внимание, что вы все еще не можете использовать идентификаторы > 2^63, но теперь вы можете использовать идентификаторы в диапазоне от -2^63 до 0.
* Поскольку мы недавно добавили поддержку вторичных индексов, ситуация стала запутанной, так как "индекс" может относиться к вторичному индексу, полнотекстовому индексу или обычному/реальному `индексу`. Чтобы уменьшить путаницу, мы переименовываем последний в "таблицу". Следующие SQL/командные строки подвержены этому изменению. Их старые версии устарели, но все еще функциональны:
  - `index <table name>` => `table <table name>`,
  - `searchd -i / --index` => `searchd -t / --table`,
  - `SHOW INDEX STATUS` => `SHOW TABLE STATUS`,
  - `SHOW INDEX SETTINGS` => `SHOW TABLE SETTINGS`,
  - `FLUSH RTINDEX` => `FLUSH TABLE`,
  - `OPTIMIZE INDEX` => `OPTIMIZE TABLE`,
  - `ATTACH TABLE plain TO RTINDEX rt` => `ATTACH TABLE plain TO TABLE rt`,
  - `RELOAD INDEX` => `RELOAD TABLE`,
  - `RELOAD INDEXES` => `RELOAD TABLES`.

  Мы не планируем делать старые формы устаревшими, но для обеспечения совместимости с документацией мы рекомендуем изменить названия в вашем приложении. Что будет изменено в будущем релизе, так это переименование "index" в "table" в выводе различных SQL и JSON команд.
* Запросы с состоянием UDF теперь вынуждены выполняться в одном потоке.
* [Проблема #1011](https://github.com/manticoresoftware/manticoresearch/issues/1011) Рефакторинг всего, что связано с планированием времени, как предварительное условие для параллельного объединения чанков.
* **⚠️ СЛОМАННОЕ ИЗМЕНЕНИЕ**: Формат хранения столбцов был изменен. Вам нужно перестроить те таблицы, которые имеют столбцовые атрибуты.
* **⚠️ СЛОМАННОЕ ИЗМЕНЕНИЕ**: Формат файла вторичных индексов был изменен, поэтому если вы используете вторичные индексы для поиска и у вас в конфигурационном файле указано `searchd.secondary_indexes = 1`, имейте в виду, что новая версия Manticore **пропустит загрузку таблиц, которые имеют вторичные индексы**. Рекомендуется:
  - Перед обновлением измените `searchd.secondary_indexes` на 0 в конфигурационном файле.
  - Запустите экземпляр. Manticore загрузит таблицы с предупреждением.
  - Выполните `ALTER TABLE <имя таблицы> REBUILD SECONDARY` для каждого индекса, чтобы перестроить вторичные индексы.

  Если вы запускаете кластер репликации, вам нужно будет выполнить `ALTER TABLE <имя таблицы> REBUILD SECONDARY` на всех узлах или следовать [этой инструкции](Securing_and_compacting_a_table/Compacting_a_table.md#Optimizing-clustered-tables) с единственным изменением: выполните `ALTER .. REBUILD SECONDARY` вместо `OPTIMIZE`.
* **⚠️ СЛОМАННОЕ ИЗМЕНЕНИЕ**: Версия binlog была обновлена, поэтому любые binlog из предыдущих версий не будут воспроизведены. Важно убедиться, что Manticore Search остановлен корректно в процессе обновления. Это означает, что в `/var/lib/manticore/binlog/` не должно быть файлов binlog, кроме `binlog.meta`, после остановки предыдущего экземпляра.
* [Проблема #849](https://github.com/manticoresoftware/manticoresearch/issues/849) `SHOW SETTINGS`: теперь вы можете видеть настройки из конфигурационного файла изнутри Manticore.
* [Проблема #1007](https://github.com/manticoresoftware/manticoresearch/issues/1007) [SET GLOBAL CPUSTATS=1/0](Server_settings/Setting_variables_online.md#SET) включает/выключает отслеживание времени процессора; [SHOW THREADS](Node_info_and_management/SHOW_THREADS.md) теперь не показывает статистику CPU, когда отслеживание времени процессора выключено.
* [Проблема #1009](https://github.com/manticoresoftware/manticoresearch/issues/1009) Сегменты RAM чанков RT таблицы теперь могут быть объединены во время сброса RAM чанка.
* [Проблема #1012](https://github.com/manticoresoftware/manticoresearch/issues/1012) Добавлен прогресс вторичного индекса в вывод [indexer](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool).
* [Проблема #1013](https://github.com/manticoresoftware/manticoresearch/issues/1013) Ранее запись таблицы могла быть удалена Manticore из списка индексов, если она не могла начать обслуживать ее при запуске. Новое поведение заключается в том, чтобы оставить ее в списке, чтобы попытаться загрузить ее при следующем запуске.
* [indextool --docextract](Miscellaneous_tools.md#indextool) возвращает все слова и попадания, принадлежащие запрашиваемому документу.
* [Коммит 2b29](https://github.com/manticoresoftware/manticoresearch/commit/2b296ee20e520b85bcbb4383e87095d31e7165dc) Переменная окружения `dump_corrupt_meta` включает дамп поврежденных метаданных таблицы в журнал в случае, если searchd не может загрузить индекс.
* [Коммит c7a3](https://github.com/manticoresoftware/manticoresearch/commit/c7a3136eefe0eb2f69e3d99fae9c10beb5821ea6) `DEBUG META` может показать `max_matches` и статистику псевдо-шардинга.
* [Коммит 6bca](https://github.com/manticoresoftware/manticoresearch/commit/6bca4848dc5c8396dbc1e057be4b9b2d579794e8) Лучше ошибка вместо запутанного "Формат заголовка индекса не json, попробую как бинарный...".
* [Коммит bef3](https://github.com/manticoresoftware/lemmatizer-uk/commit/bef3ff0386d3ee87ec57619782100972c1122e47) Путь украинского лемматизатора был изменен.
* [Коммит 4ae7](https://github.com/manticoresoftware/manticoresearch/commit/4ae789595329a2951e194d1191ddb3121459a560) Статистика вторичных индексов была добавлена в [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META).
* [Коммит 2e7c](https://github.com/manticoresoftware/manticoresearch/commit/2e7c585e) JSON интерфейс теперь можно легко визуализировать с помощью Swagger Editor https://manual.manticoresearch.com/Openapi#OpenAPI-specification.
* **⚠️ СЛОМАННОЕ ИЗМЕНЕНИЕ**: Протокол репликации был изменен. Если вы запускаете кластер репликации, то при обновлении до Manticore 5 вам нужно:
  - сначала корректно остановить все ваши узлы
  - а затем запустить узел, который был остановлен последним с `--new-cluster` (запустите инструмент `manticore_new_cluster` в Linux).
  - прочитайте о [перезапуске кластера](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster) для получения дополнительных деталей.

### Изменения, связанные с библиотекой Manticore Columnar
* Рефакторинг интеграции вторичных индексов с колонковым хранилищем.
* [Коммит efe2](https://github.com/manticoresoftware/columnar/commit/efe26b2a) Оптимизация библиотеки Manticore Columnar, которая может снизить время отклика за счет частической предварительной оценки min/max.
* [Коммит 2757](https://github.com/manticoresoftware/columnar/commit/2757b99b) Если слияние дисковых чанков прерывается, демон теперь очищает временные файлы, связанные с MCL.
* [Коммит e9c6](https://github.com/manticoresoftware/manticoresearch/commit/e9c672014b63e2b49e7a5a53880583af86c3ab4e) Версии колонковых и вторичных библиотек записываются в журнал при сбое.
* [Коммит f5e8](https://github.com/manticoresoftware/columnar/commit/f5e84eeb8b6cb102f13e3468246702427527fef9) Добавлена поддержка быстрого перематывания списка документов для вторичных индексов.
* [Коммит 06df](https://github.com/manticoresoftware/manticoresearch/commit/06dfdd2e668933495dd6cb52d5ba2e0aa9b26e58) Запросы, такие как `select attr, count(*) from plain_index` (без фильтрации), теперь быстрее, если вы используете MCL.
* [Коммит 0a76](https://github.com/manticoresoftware/manticoresearch/commit/0a76ecb9adbe886868b962dd425de5f475bbf55e) @@autocommit в HandleMysqlSelectSysvar для совместимости с .net коннектором для mysql версии выше 8.25
* **⚠️ СЛОМАННОЕ ИЗМЕНЕНИЕ**: [MCL Проблема #17](https://github.com/manticoresoftware/columnar/issues/17) MCL: добавлен код SSE для колонкового сканирования. MCL теперь требует как минимум SSE4.2.

### Изменения, связанные с упаковкой
* [Коммит 4d19](https://github.com/manticoresoftware/manticoresearch/commit/4d19f5cbe49a31228aa09253f061165bfe80e51b) **⚠️ СЛОМАННОЕ ИЗМЕНЕНИЕ**: Поддержка Debian Stretch и Ubuntu Xenial прекращена.
* Поддержка RHEL 9, включая Centos 9, Alma Linux 9 и Oracle Linux 9.
* [Проблема #924](https://github.com/manticoresoftware/manticoresearch/issues/924) Поддержка Debian Bookworm.
* [Проблема #636](https://github.com/manticoresoftware/manticoresearch/issues/636) Упаковка: сборки arm64 для Linux и MacOS.
* [PR #26](https://github.com/manticoresoftware/docker/pull/26) Многоархитектурный (x86_64 / arm64) образ docker.
* [Упрощенная сборка пакетов для участников](Installation/Compiling_from_sources.md#Building-using-CI-Docker).
* Теперь возможно установить конкретную версию с помощью APT.
* [Коммит a6b8](https://github.com/manticoresoftware/manticoresearch/commit/51fddca5c2a3ebb8576fae4c18660656ba22de0f) Установщик для Windows (ранее мы предоставляли только архив).
* Переключились на компиляцию с использованием CLang 15.
* **⚠️ СЛОМАННОЕ ИЗМЕНЕНИЕ**: Пользовательские формулы Homebrew, включая формулу для библиотеки Manticore Columnar. Чтобы установить Manticore, MCL и любые другие необходимые компоненты, используйте следующую команду `brew install manticoresoftware/manticore/manticoresearch manticoresoftware/manticore/manticore-extra`.

### Исправления ошибок
* [Проблема #479](https://github.com/manticoresoftware/manticoresearch/issues/479) Поле с именем `text`
* [Проблема #501](https://github.com/manticoresoftware/manticoresearch/issues/501) id не может быть не bigint
* [Проблема #646](https://github.com/manticoresoftware/manticoresearch/issues/646) ALTER против поля с именем "text"
* ❗[Проблема #652](https://github.com/manticoresoftware/manticoresearch/issues/652) Возможная ОШИБКА: HTTP (JSON) смещение и лимит влияют на результаты фасетов
* ❗[Проблема #827](https://github.com/manticoresoftware/manticoresearch/issues/827) Searchd зависает/выходит из строя при интенсивной загрузке
* ❗[Проблема #996](https://github.com/manticoresoftware/manticoresearch/issues/996) PQ индекс исчерпал память
* ❗[Коммит 1041](https://github.com/manticoresoftware/manticoresearch/commit/10416ef7dddf06c0d759e32ccd6ebaa2468f7cbf) `binlog_flush = 1` был сломан все время с Sphinx. Исправлено.
* [MCL Проблема #14](https://github.com/manticoresoftware/columnar/issues/14) MCL: сбой при выборе, когда слишком много полей ft
* [Проблема #470](https://github.com/manticoresoftware/manticoresearch/issues/470) sql_joined_field не может быть сохранен
* [Проблема #713](https://github.com/manticoresoftware/manticoresearch/issues/713) Сбой при использовании LEVENSHTEIN()
* [Проблема #743](https://github.com/manticoresoftware/manticoresearch/issues/743) Manticore неожиданно вылетает и не может нормально перезапуститься
* [Проблема #788](https://github.com/manticoresoftware/manticoresearch/issues/788) CALL KEYWORDS через /sql возвращает управляющий символ, который ломает json
* [Проблема #789](https://github.com/manticoresoftware/manticoresearch/issues/789) mariadb не может создать таблицу FEDERATED
* [Проблема #796](https://github.com/manticoresoftware/manticoresearch/issues/796) ПРЕДУПРЕЖДЕНИЕ: dlopen() не удалось: /usr/bin/lib_manticore_columnar.so: не удается открыть общий объект: Нет такого файла или каталога
* [Проблема #797](https://github.com/manticoresoftware/manticoresearch/issues/797) Manticore вылетает, когда поиск с ZONESPAN выполняется через api
* [Проблема #799](https://github.com/manticoresoftware/manticoresearch/issues/799) неправильный вес при использовании нескольких индексов и уникальных фасетов
* [Проблема #801](https://github.com/manticoresoftware/manticoresearch/issues/801) Запрос группы SphinxQL зависает после повторной обработки SQL индекса
* [Проблема #802](https://github.com/manticoresoftware/manticoresearch/issues/802) MCL: Индексатор вылетает в 5.0.2 и manticore-columnar-lib 1.15.4
* [Проблема #813](https://github.com/manticoresoftware/manticoresearch/issues/813) Manticore 5.0.2 FEDERATED возвращает пустое множество (MySQL 8.0.28)
* [Проблема #824](https://github.com/manticoresoftware/manticoresearch/issues/824) выбор COUNT DISTINCT на 2 индексах, когда результат равен нулю, вызывает внутреннюю ошибку
* [Проблема #826](https://github.com/manticoresoftware/manticoresearch/issues/826) СБОЙ на запросе удаления
* [Проблема #843](https://github.com/manticoresoftware/manticoresearch/issues/843) MCL: Ошибка с длинным текстовым полем
* [Проблема #856](https://github.com/manticoresoftware/manticoresearch/issues/856) 5.0.2 rtindex: Поведение лимита агрегатного поиска не соответствует ожиданиям
* [Проблема #863](https://github.com/manticoresoftware/manticoresearch/issues/863) Возвращаемые результаты имеют тип Nonetype, даже для запросов, которые должны возвращать несколько результатов
* [Проблема #870](https://github.com/manticoresoftware/manticoresearch/issues/870) Сбой при использовании атрибута и сохраненного поля в выражении SELECT
* [Проблема #872](https://github.com/manticoresoftware/manticoresearch/issues/872) таблица становится невидимой после сбоя
* [Проблема #877](https://github.com/manticoresoftware/manticoresearch/issues/877) Два отрицательных термина в запросе поиска дают ошибку: запрос не вычисляемый
* [Проблема #878](https://github.com/manticoresoftware/manticoresearch/issues/878) a -b -c не работает через json query_string
* [Проблема #886](https://github.com/manticoresoftware/manticoresearch/issues/886) псевдо_шардинг с совпадением запроса
* [Проблема #893](https://github.com/manticoresoftware/manticoresearch/issues/893) Manticore 5.0.2 функция min/max не работает как ожидалось ...
* [Проблема #896](https://github.com/manticoresoftware/manticoresearch/issues/896) Поле "weight" не разбирается правильно
* [Проблема #897](https://github.com/manticoresoftware/manticoresearch/issues/897) Сервис Manticore вылетает при запуске и продолжает перезапускаться
* [Проблема #900](https://github.com/manticoresoftware/manticoresearch/issues/900) группировка по j.a, что-то работает неправильно
* [Проблема #913](https://github.com/manticoresoftware/manticoresearch/issues/913) Сбой Searchd, когда expr используется в ранжировании, но только для запросов с двумя близостями
* [Проблема #916](https://github.com/manticoresoftware/manticoresearch/issues/916) net_throttle_action сломан
* [Проблема #919](https://github.com/manticoresoftware/manticoresearch/issues/919) MCL: Manticore вылетает при выполнении запроса, и другие вылетают во время восстановления кластера.
* [Проблема #925](https://github.com/manticoresoftware/manticoresearch/issues/925) SHOW CREATE TABLE выводит без обратных кавычек
* [Проблема #930](https://github.com/manticoresoftware/manticoresearch/issues/930) Теперь можно запрашивать Manticore из Java через JDBC коннектор
* [Проблема #933](https://github.com/manticoresoftware/manticoresearch/issues/933) проблемы с ранжированием bm25f
* [Проблема #934](https://github.com/manticoresoftware/manticoresearch/issues/934) индексы без конфигурации зависают в watchdog в состоянии первой загрузки
* [Проблема #937](https://github.com/manticoresoftware/manticoresearch/issues/937) Segfault при сортировке данных фасета
* [Проблема #940](https://github.com/manticoresoftware/manticoresearch/issues/940) сбой на CONCAT(TO_STRING)
* [Проблема #947](https://github.com/manticoresoftware/manticoresearch/issues/947) В некоторых случаях один простой выбор может вызвать зависание всего экземпляра, так что вы не могли войти в него или выполнить любой другой запрос, пока выполняется текущий выбор.
* [Проблема #948](https://github.com/manticoresoftware/manticoresearch/issues/948) Сбой индексатора
* [Проблема #950](https://github.com/manticoresoftware/manticoresearch/issues/950) неправильный подсчет из уникального фасета
* [Проблема #953](https://github.com/manticoresoftware/manticoresearch/issues/953) LCS рассчитывается неправильно в встроенном ранжировщике sph04
* [Проблема #955](https://github.com/manticoresoftware/manticoresearch/issues/955) 5.0.3 dev вылетает
* [Проблема #963](https://github.com/manticoresoftware/manticoresearch/issues/963) FACET с json на движке columnar вызывает сбой
* [Проблема #982](https://github.com/manticoresoftware/manticoresearch/issues/982) MCL: 5.0.3 сбой из-за вторичного индекса
* [PR #984](https://github.com/manticoresoftware/manticoresearch/pull/984) @@autocommit в HandleMysqlSelectSysvar
* [PR #985](https://github.com/manticoresoftware/manticoresearch/pull/985) Исправить распределение потоков в RT индексах
* [Проблема #985](https://github.com/manticoresoftware/manticoresearch/issues/985) Исправить распределение потоков в RT индексах
* [Проблема #986](https://github.com/manticoresoftware/manticoresearch/issues/986) неправильное значение по умолчанию max_query_time
* [Проблема #987](https://github.com/manticoresoftware/manticoresearch/issues/987) Сбой при использовании регулярного выражения в многопоточном выполнении
* [Проблема #988](https://github.com/manticoresoftware/manticoresearch/issues/988) Нарушена обратная совместимость индекса
* [Проблема #989](https://github.com/manticoresoftware/manticoresearch/issues/989) indextool сообщает об ошибке при проверке атрибутов columnar
* [Проблема #990](https://github.com/manticoresoftware/manticoresearch/issues/990) утечка памяти клонов json grouper
* [Проблема #991](https://github.com/manticoresoftware/manticoresearch/issues/991) Утечка памяти клонирования функции levenshtein
* [Проблема #992](https://github.com/manticoresoftware/manticoresearch/issues/992) Сообщение об ошибке потеряно при загрузке метаданных
* [Проблема #993](https://github.com/manticoresoftware/manticoresearch/issues/993) Распространение ошибок от динамических индексов/подключей и системных переменных
* [Проблема #994](https://github.com/manticoresoftware/manticoresearch/issues/994) Сбой при подсчете уникальных значений по строке в columnar хранилище
* [Проблема #995](https://github.com/manticoresoftware/manticoresearch/issues/995) MCL: min(pickup_datetime) из taxi1 вызывает сбой
* [Проблема #997](https://github.com/manticoresoftware/manticoresearch/issues/997) пустые исключения JSON запрос удаляет столбцы из списка выбора
* [Проблема #998](https://github.com/manticoresoftware/manticoresearch/issues/998) Вторичные задачи, выполняемые на текущем планировщике, иногда вызывают аномальные побочные эффекты
* [Проблема #999](https://github.com/manticoresoftware/manticoresearch/issues/999) сбой с уникальными фасетами и разными схемами
* [Проблема #1000](https://github.com/manticoresoftware/manticoresearch/issues/1000) MCL: Колонный rt индекс стал поврежденным после запуска без колонной библиотеки
* [Проблема #1001](https://github.com/manticoresoftware/manticoresearch/issues/1001) неявный обрезка не работает в json
* [Проблема #1002](https://github.com/manticoresoftware/manticoresearch/issues/1002) Проблема с колонным группировщиком
* [Проблема #1003](https://github.com/manticoresoftware/manticoresearch/issues/1003) Невозможно удалить последнее поле из индекса
* [Проблема #1004](https://github.com/manticoresoftware/manticoresearch/issues/1004) неправильное поведение после --new-cluster
* [Проблема #1005](https://github.com/manticoresoftware/manticoresearch/issues/1005) "колонная библиотека не загружена", но она не требуется
* [Проблема #1006](https://github.com/manticoresoftware/manticoresearch/issues/1006) нет ошибки для запроса удаления
* [Проблема #1010](https://github.com/manticoresoftware/manticoresearch/issues/1010) Исправлено местоположение файла данных ICU в сборках Windows
* [PR #1018](https://github.com/manticoresoftware/manticoresearch/pull/1018) Проблема с отправкой рукопожатия
* [Проблема #1020](https://github.com/manticoresoftware/manticoresearch/issues/1020) Отображение id в show create table
* [Проблема #1024 crash 1](https://github.com/manticoresoftware/manticoresearch/issues/1024) Сбой / Сегментация при поиске фасета с большим количеством результатов.
* [Проблема #1026](https://github.com/manticoresoftware/manticoresearch/issues/1026) RT индекс: searchd "зависает" навсегда, когда вставляется много документов и RAMchunk заполняется
* [Коммит 4739](https://github.com/manticoresoftware/manticoresearch/commit/4739dafd) Поток зависает при завершении работы, пока репликация занята между узлами
* [Коммит ab87](https://github.com/manticoresoftware/manticoresearch/commit/ab87836782e7ae43fe5f9dea739eed0d336b42c1) Смешивание чисел с плавающей запятой и целых в фильтре диапазона JSON может заставить Manticore игнорировать фильтр
* [Коммит d001](https://github.com/manticoresoftware/manticoresearch/commit/d00101c2905f6393ce3fca23c4f6dcb2506f4bd9) Фильтры с плавающей запятой в JSON были неточными
* [Коммит 4092](https://github.com/manticoresoftware/manticoresearch/commit/4092c6a8) Отменить неподтвержденные транзакции, если индекс изменен (иначе может произойти сбой)
* [Коммит 9692](https://github.com/manticoresoftware/manticoresearch/commit/9692a840) Ошибка синтаксиса запроса при использовании обратной косой черты
* [Коммит 0c19](https://github.com/manticoresoftware/manticoresearch/commit/0c19ad8d) workers_clients могут быть неверными в SHOW STATUS
* [Коммит 1772](https://github.com/manticoresoftware/manticoresearch/commit/17720e1e2f2ec38337df565bee78f7ce9d9c744a) исправлен сбой при слиянии сегментов ram без docstores
* [Коммит f45b](https://github.com/manticoresoftware/manticoresearch/commit/f45b337b5c265433bb7d6262ad65d02f65edbaf3) Исправлено пропущенное условие ALL/ANY для фильтра JSON равенства
* [Коммит 3e83](https://github.com/manticoresoftware/manticoresearch/commit/3e83c1f0c0e6e7a9d4297113f69a112cbabd9e64) Репликация может завершиться неудачей с `got exception while reading ist stream: mkstemp(./gmb_pF6TJi) failed: 13 (Permission denied)`, если searchd был запущен из каталога, в который он не может записывать.
* [Коммит 92e5](https://github.com/manticoresoftware/manticoresearch/commit/92e5629f7150f62524b385fca670a01ba9590f5b) С 4.0.2 журнал сбоев включал только смещения. Этот коммит это исправляет.

## Версия 5.0.2
Выпущена: 30 мая 2022

### Исправления ошибок
* ❗[Issue #791](https://github.com/manticoresoftware/manticoresearch/issues/791) - неправильный размер стека мог привести к сбою.

## Версия 5.0.0
Выпущена: 18 мая 2022


### Основные новые возможности
* 🔬 Поддержка [Manticore Columnar Library 1.15.2](https://github.com/manticoresoftware/columnar/), которая включает бета-версию [вторичных индексов](Server_settings/Searchd.md#secondary_indexes). Построение вторичных индексов включено по умолчанию для колоночных и построчных индексов (как обычных, так и реального времени), если используется [Manticore Columnar Library](https://github.com/manticoresoftware/columnar), но для их использования при поиске необходимо установить `secondary_indexes = 1` либо в конфигурационном файле, либо с помощью [SET GLOBAL](Server_settings/Setting_variables_online.md). Новая функциональность поддерживается во всех операционных системах, кроме старых Debian Stretch и Ubuntu Xenial.
* [Режим только для чтения](Security/Read_only.md): теперь можно указать слушателей, которые обрабатывают только запросы на чтение, игнорируя любые операции записи.
* Новый эндпоинт [/cli](Connecting_to_the_server/HTTP.md#/cli) для ещё более простого выполнения SQL-запросов через HTTP.
* Более быстрые массовые INSERT/REPLACE/DELETE через JSON по HTTP: ранее можно было передавать несколько команд записи через протокол HTTP JSON, но они обрабатывались по одной, теперь они обрабатываются как единая транзакция.
* [#720](https://github.com/manticoresoftware/manticoresearch/issues/720) Поддержка [вложенных фильтров](Searching/Filters.md#Nested-bool-query) в протоколе JSON. Ранее нельзя было закодировать такие конструкции, как `a=1 and (b=2 or c=3)` в JSON: `must` (И), `should` (ИЛИ) и `must_not` (НЕ) работали только на верхнем уровне. Теперь их можно вкладывать.
* Поддержка [Chunked transfer encoding](https://en.wikipedia.org/wiki/Chunked_transfer_encoding) в протоколе HTTP. Теперь вы можете использовать chunked transfer в своём приложении для передачи больших пакетов с уменьшенным потреблением ресурсов (поскольку вычисление `Content-Length` не требуется). На стороне сервера Manticore теперь всегда обрабатывает входящие HTTP-данные в потоковом режиме, не дожидаясь передачи всего пакета, как раньше, что:
  - снижает пиковое использование оперативной памяти, уменьшая риск OOM
  - уменьшает время отклика (наши тесты показали снижение на 11% при обработке пакета размером 100 МБ)
  - позволяет обойти ограничение [max_packet_size](Server_settings/Searchd.md#max_packet_size) и передавать пакеты, значительно превышающие максимально допустимое значение `max_packet_size` (128 МБ), например, 1 ГБ за раз.
* [#719](https://github.com/manticoresoftware/manticoresearch/issues/719) Поддержка `100 Continue` в HTTP-интерфейсе: теперь можно передавать большие пакеты из `curl` (включая библиотеки curl, используемые различными языками программирования), который по умолчанию отправляет `Expect: 100-continue` и ждёт некоторое время перед фактической отправкой пакета. Ранее приходилось добавлять заголовок `Expect: `, теперь в этом нет необходимости.

  <details>

  Раньше (обратите внимание на время отклика):

  ```bash
  $ time curl -v -sX POST http://localhost:9318/bulk -H "Content-Type: application/x-ndjson" --data '{"insert": {"index": "user", "doc":  {"name":"Prof. Matt Heaney IV","email":"ibergnaum@yahoo.com","description":"Tempora ullam eaque consequatur. Vero aut minima ut et ut omnis officiis vel. Molestiae quis voluptatum sint numquam.","age":15,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"Prof. Boyd McKenzie","email":"carlotta11@hotmail.com","description":"Blanditiis maiores odio corporis eaque illum. Aut et rerum iste. Neque et ullam quisquam officia dignissimos quo cumque.","age":84,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"Mr. Johann Smith","email":"stiedemann.tristin@ziemann.com","description":"Temporibus amet magnam consequatur omnis consequatur illo fugit. Debitis natus doloremque est tempore deserunt vero. Harum eos corrupti nemo ut.","age":89,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"Hector Pouros","email":"hickle.mafalda@hotmail.com","description":" as voluptatem inventore sit. Aliquam fugit perferendis est id aut odio et sapiente.","age":64,"active":1}}}'
  *   Trying 127.0.0.1...
  * Connected to localhost (127.0.0.1) port 9318 (#0)
  > POST /bulk HTTP/1.1
  > Host: localhost:9318
  > User-Agent: curl/7.47.0
  > Accept: */*
  > Content-Type: application/x-ndjson
  > Content-Length: 1025
  > Expect: 100-continue
  >
  * Done waiting for 100-continue
  * We are completely uploaded and fine
  < HTTP/1.1 200 OK
  < Server: 4.2.0 15e927b@211223 release (columnar 1.11.4 327b3d4@211223)
  < Content-Type: application/json; charset=UTF-8
  < Content-Length: 434
  <
  * Connection #0 to host localhost left intact
  {"items":[{"insert":{"table":"user","_id":2811798918248005633,"created":true,"result":"created","status":201}},{"insert":{"table":"user","_id":2811798918248005634,"created":true,"result":"created","status":201}},{"insert":{"table":"user","_id":2811798918248005635,"created":true,"result":"created","status":201}},{"insert":{"table":"user","_id":2811798918248005636,"created":true,"result":"created","status":201}}],"errors":false}
  real	0m1.022s
  user	0m0.001s
  sys	0m0.010s
  ```

  Теперь:
  ```bash
  $ time curl -v -sX POST http://localhost:9318/bulk -H "Content-Type: application/x-ndjson" --data '{"insert": {"index": "user", "doc":  {"name":"Prof. Matt Heaney IV","email":"ibergnaum@yahoo.com","description":"Tempora ullam eaque consequatur. Vero aut minima ut et ut omnis officiis vel. Molestiae quis voluptatum sint numquam.","age":15,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"Prof. Boyd McKenzie","email":"carlotta11@hotmail.com","description":"Blanditiis maiores odio corporis eaque illum. Aut et rerum iste. Neque et ullam quisquam officia dignissimos quo cumque.","age":84,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"Mr. Johann Smith","email":"stiedemann.tristin@ziemann.com","description":"Temporibus amet magnam consequatur omnis consequatur illo fugit. Debitis natus doloremque est tempore deserunt vero. Harum eos corrupti nemo ut.","age":89,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"Hector Pouros","email":"hickle.mafalda@hotmail.com","description":" as voluptatem inventore sit. Aliquam fugit perferendis est id aut odio et sapiente.","age":64,"active":1}}}'
  *   Trying 127.0.0.1...
  * Connected to localhost (127.0.0.1) port 9318 (#0)
  > POST /bulk HTTP/1.1
  > Host: localhost:9318
  > User-Agent: curl/7.47.0
  > Accept: */*
  > Content-Type: application/x-ndjson
  > Content-Length: 1025
  > Expect: 100-continue
  >
  < HTTP/1.1 100 Continue
  < Server: 4.2.1 63e5749@220405 dev
  < Content-Type: application/json; charset=UTF-8
  < Content-Length: 0
  * We are completely uploaded and fine
  < HTTP/1.1 200 OK
  < Server: 4.2.1 63e5749@220405 dev
  < Content-Type: application/json; charset=UTF-8
  < Content-Length: 147
  <
  * Connection #0 to host localhost left intact
  {"items":[{"bulk":{"table":"user","_id":2811798919590182916,"created":4,"deleted":0,"updated":0,"result":"created","status":201}}],"errors":false}
  real	0m0.015s
  user	0m0.005s
  sys	0m0.004s
  ```

  </details>

* **⚠️ КРИТИЧЕСКОЕ ИЗМЕНЕНИЕ**: [Псевдошардирование](Server_settings/Searchd.md#pseudo_sharding) включено по умолчанию. Если вы хотите отключить его, убедитесь, что добавили `pseudo_sharding = 0` в раздел `searchd` вашего конфигурационного файла Manticore.
* Наличие хотя бы одного полнотекстового поля в индексе реального времени/обычном индексе больше не является обязательным. Теперь вы можете использовать Manticore даже в случаях, не связанных с полнотекстовым поиском.
* [Быстрая выборка](Creating_a_table/Data_types.md#fast_fetch) для атрибутов, использующих [Manticore Columnar Library](https://github.com/manticoresoftware/columnar): запросы вида `select * from <columnar table>` теперь выполняются намного быстрее, чем раньше, особенно если в схеме много полей.
* **⚠️ КРИТИЧЕСКОЕ ИЗМЕНЕНИЕ**: Неявный [cutoff](Searching/Options.md#cutoff). Теперь Manticore не тратит время и ресурсы на обработку данных, которые вам не нужны в возвращаемом наборе результатов. Обратной стороной является то, что это влияет на `total_found` в [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META) и на `hits.total` в JSON-выводе. Теперь это значение точно только в случае, если вы видите `total_relation: eq`, в то время как `total_relation: gte` означает, что фактическое количество соответствующих документов больше, чем полученное значение `total_found`. Чтобы сохранить прежнее поведение, можно использовать опцию поиска `cutoff=0`, которая делает `total_relation` всегда равным `eq`.
* **⚠️ КРИТИЧЕСКОЕ ИЗМЕНЕНИЕ**: Все полнотекстовые поля теперь по умолчанию [хранятся](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#stored_fields). Чтобы сделать все поля нехранимыми (т.е. вернуться к прежнему поведению), необходимо использовать `stored_fields = ` (пустое значение).
* [#715](https://github.com/manticoresoftware/manticoresearch/issues/715) HTTP JSON поддерживает [опции поиска](Searching/Options.md#General-syntax).

### Незначительные изменения
* **⚠️ КРИТИЧЕСКОЕ ИЗМЕНЕНИЕ**: Изменение формата метафайла индекса. Ранее метафайлы (`.meta`, `.sph`) были в бинарном формате, теперь это просто json. Новая версия Manticore автоматически преобразует старые индексы, но:
  - вы можете получить предупреждение типа `WARNING: ... syntax error, unexpected TOK_IDENT`
  - вы не сможете запустить индекс с предыдущими версиями Manticore, убедитесь, что у вас есть резервная копия
* **⚠️ КРИТИЧЕСКОЕ ИЗМЕНЕНИЕ**: Поддержка состояния сессии с помощью [HTTP keep-alive](Connecting_to_the_server/HTTP.md#HTTP-state). Это делает HTTP сессионным, когда клиент тоже его поддерживает. Например, используя новый эндпоинт [/cli](Connecting_to_the_server/HTTP.md#/cli) и HTTP keep-alive (который включен по умолчанию во всех браузерах), вы можете вызвать `SHOW META` после `SELECT`, и это будет работать так же, как и через mysql. Обратите внимание, что ранее заголовок HTTP `Connection: keep-alive` тоже поддерживался, но он только приводил к повторному использованию того же соединения. Начиная с этой версии он также делает сессию сессионной.
* Теперь вы можете указать `columnar_attrs = *`, чтобы определить все ваши атрибуты как колоночные в [plain mode](Read_this_first.md#Real-time-mode-vs-plain-mode), что полезно, если список длинный.
* Более быстрая репликация SST
* **⚠️ КРИТИЧЕСКОЕ ИЗМЕНЕНИЕ**: Протокол репликации был изменен. Если вы запускаете кластер репликации, то при обновлении до Manticore 5 вам необходимо:
  - сначала корректно остановить все ваши узлы
  - а затем запустить узел, который был остановлен последним, с параметром `--new-cluster` (запустите инструмент `manticore_new_cluster` в Linux).
  - прочитайте о [перезапуске кластера](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster) для получения более подробной информации.
* Улучшения репликации:
  - Более быстрая SST
  - Устойчивость к шуму, что может помочь в случае нестабильной сети между узлами репликации
  - Улучшенное логирование
* Улучшение безопасности: Manticore теперь прослушивает `127.0.0.1` вместо `0.0.0.0` в случае, если в конфигурации вообще не указан `listen`. Хотя в конфигурации по умолчанию, которая поставляется с Manticore Search, параметр `listen` указан, и не типично иметь конфигурацию вообще без `listen`, это все же возможно. Ранее Manticore прослушивал бы `0.0.0.0`, что небезопасно, теперь он прослушивает `127.0.0.1`, который обычно не доступен из Интернета.
* Более быстрая агрегация по колоночным атрибутам.
* Повышена точность `AVG()`: ранее Manticore использовал `float` внутренне для агрегаций, теперь он использует `double`, что значительно повышает точность.
* Улучшена поддержка драйвера JDBC MySQL.
* Поддержка `DEBUG malloc_stats` для [jemalloc](https://github.com/jemalloc/jemalloc).
* [optimize_cutoff](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#optimize_cutoff) теперь доступен как настройка для каждой таблицы, которую можно установить при CREATE или ALTER таблицы.
* **⚠️ КРИТИЧЕСКОЕ ИЗМЕНЕНИЕ**: [query_log_format](Server_settings/Searchd.md#query_log_format) теперь по умолчанию **`sphinxql`**. Если вы привыкли к формату `plain`, вам нужно добавить `query_log_format = plain` в ваш конфигурационный файл.
* Значительные улучшения потребления памяти: Manticore теперь потребляет значительно меньше оперативной памяти в случае длительной и интенсивной нагрузки на вставку/замену/оптимизацию, если используются хранимые поля.
* Значение по умолчанию для [shutdown_timeout](Server_settings/Searchd.md#shutdown_timeout) было увеличено с 3 секунд до 60 секунд.
* [Коммит ffd0](https://github.com/manticoresoftware/manticoresearch/commit/ffd0499d329d2c383f14c8a44c4cc84338ab56e7) Поддержка Java mysql connector >= 6.0.3: в [Java mysql connection 6.0.3](https://mvnrepository.com/artifact/mysql/mysql-connector-java/6.0.3) они изменили способ подключения к mysql, что нарушило совместимость с Manticore. Новое поведение теперь поддерживается.
* [Коммит 1da6](https://github.com/manticoresoftware/manticoresearch/commit/1da6dbec) отключил сохранение нового дискового чанка при загрузке индекса (например, при запуске searchd).
* [Issue #746](https://github.com/manticoresoftware/manticoresearch/issues/746) Поддержка glibc >= 2.34.
* [Issue #784](https://github.com/manticoresoftware/manticoresearch/issues/784) подсчет 'VIP' соединений отдельно от обычных (не-VIP). Ранее VIP-соединения учитывались в лимите `max_connections`, что могло вызывать ошибку "maxed out" для не-VIP соединений. Теперь VIP-соединения не учитываются в лимите. Текущее количество VIP-соединений также можно увидеть в `SHOW STATUS` и `status`.
* [ID](Creating_a_table/Data_types.md#Document-ID) теперь можно указывать явно.
* [Issue #687](https://github.com/manticoresoftware/manticoresearch/issues/687) поддержка сжатия zstd для mysql proto

### ⚠️ Другие незначительные изменения
* ⚠️ Формула BM25F была немного обновлена для улучшения релевантности поиска. Это влияет только на результаты поиска в случае, если вы используете функцию [BM25F()](Functions/Searching_and_ranking_functions.md#BM25F%28%29), это не изменяет поведение формулы ранжирования по умолчанию.
* ⚠️ Изменено поведение REST [/sql](Connecting_to_the_server/HTTP.md#mode=raw) конечной точки: `/sql?mode=raw` теперь требует экранирования и возвращает массив.
* ⚠️ Изменение формата ответа на запросы `/bulk` INSERT/REPLACE/DELETE:
  - ранее каждый подзапрос составлял отдельную транзакцию и приводил к отдельному ответу
  - теперь весь пакет считается одной транзакцией, которая возвращает один ответ
* ⚠️ Параметры поиска `low_priority` и `boolean_simplify` теперь требуют значения (`0/1`): ранее вы могли делать `SELECT ... OPTION low_priority, boolean_simplify`, теперь вам нужно делать `SELECT ... OPTION low_priority=1, boolean_simplify=1`.
* ⚠️ Если вы используете старые клиенты [php](https://github.com/manticoresoftware/manticoresearch/blob/master/api/sphinxapi.php), [python](https://github.com/manticoresoftware/manticoresearch/blob/master/api/sphinxapi.py) или [java](https://github.com/manticoresoftware/manticoresearch/tree/master/api/java), пожалуйста, перейдите по соответствующей ссылке и найдите обновленную версию. **Старые версии не полностью совместимы с Manticore 5.**
* ⚠️ HTTP JSON запросы теперь записываются в другом формате в режиме `query_log_format=sphinxql`. Ранее записывалась только часть с полным текстом, теперь записывается как есть.

### Новые пакеты
* **⚠️ СУЩЕСТВЕННОЕ ИЗМЕНЕНИЕ**: из-за новой структуры при обновлении до Manticore 5 рекомендуется удалить старые пакеты перед установкой новых:
  - Для RPM: `yum remove manticore*`
  - Для Debian и Ubuntu: `apt remove manticore*`
* Новая структура deb/rpm пакетов. Предыдущие версии предоставляли:
  - `manticore-server` с `searchd` (основной демон поиска) и всем необходимым для него
  - `manticore-tools` с `indexer` и `indextool`
  - `manticore`, включающий все
  - `manticore-all` RPM как мета-пакет, ссылающийся на `manticore-server` и `manticore-tools`

  Новая структура:
  - `manticore` - мета-пакет deb/rpm, который устанавливает все вышеперечисленное как зависимости
  - `manticore-server-core` - `searchd` и все, чтобы запустить его отдельно
  - `manticore-server` - файлы systemd и другие вспомогательные скрипты
  - `manticore-tools` - `indexer`, `indextool` и другие инструменты
  - `manticore-common` - файл конфигурации по умолчанию, каталог данных по умолчанию, стоп-слова по умолчанию
  - `manticore-icudata`, `manticore-dev`, `manticore-converter` не изменились сильно
  - `.tgz` пакет, который включает все пакеты
* Поддержка Ubuntu Jammy
* Поддержка Amazon Linux 2 через [YUM репозиторий](Installation/RHEL_and_Centos.md#YUM-repository)

### Исправления ошибок
* [Проблема #815](https://github.com/manticoresoftware/manticoresearch/issues/815) Случайный сбой при использовании функции UDF
* [Проблема #287](https://github.com/manticoresoftware/manticoresearch/issues/287) недостаточно памяти при индексации RT индекса
* [Проблема #604](https://github.com/manticoresoftware/manticoresearch/issues/604) Ломающее изменение 3.6.0, 4.2.0 sphinxql-parser
* [Проблема #667](https://github.com/manticoresoftware/manticoresearch/issues/667) ФАТАЛЬНО: недостаточно памяти (невозможно выделить 9007199254740992 байт)
* [Проблема #676](https://github.com/manticoresoftware/manticoresearch/issues/676) Строки не передаются правильно в UDF
* ❗[Проблема #698](https://github.com/manticoresoftware/manticoresearch/issues/698) Searchd выдает сбой после попытки добавить текстовый столбец в rt индекс
* [Проблема #705](https://github.com/manticoresoftware/manticoresearch/issues/705) Индексатор не смог найти все столбцы
* ❗[Проблема #709](https://github.com/manticoresoftware/manticoresearch/issues/705) Группировка по json.boolean работает неправильно
* [Проблема #716](https://github.com/manticoresoftware/manticoresearch/issues/716) Команды indextool, связанные с индексом (например, --dumpdict) не работают
* ❗[Проблема #724](https://github.com/manticoresoftware/manticoresearch/issues/724) Поля исчезают из выбора
* [Проблема #727](https://github.com/manticoresoftware/manticoresearch/issues/727) Несоответствие Content-Type HttpClient .NET при использовании `application/x-ndjson`
* [Проблема #729](https://github.com/manticoresoftware/manticoresearch/issues/729) Расчет длины поля
* ❗[Проблема #730](https://github.com/manticoresoftware/manticoresearch/issues/730) создание/вставка/удаление колонной таблицы вызывает утечку памяти
* [Проблема #731](https://github.com/manticoresoftware/manticoresearch/issues/731) Пустой столбец в результатах при определенных условиях
* ❗[Проблема #749](https://github.com/manticoresoftware/manticoresearch/issues/749) Сбой демона при запуске
* ❗[Проблема #750](https://github.com/manticoresoftware/manticoresearch/issues/750) Демон зависает при запуске
* ❗[Проблема #751](https://github.com/manticoresoftware/manticoresearch/issues/751) Сбой на SST
* [Проблема #752](https://github.com/manticoresoftware/manticoresearch/issues/752) Атрибут Json помечен как колонный, когда engine='columnar'
* [Проблема #753](https://github.com/manticoresoftware/manticoresearch/issues/753) Репликация слушает на 0
* [Проблема #754](https://github.com/manticoresoftware/manticoresearch/issues/754) columnar_attrs = * не работает с csvpipe
* ❗[Проблема #755](https://github.com/manticoresoftware/manticoresearch/issues/755) Сбой при выборе float в колонной в rt
* ❗[Проблема #756](https://github.com/manticoresoftware/manticoresearch/issues/756) Indextool изменяет rt индекс во время проверки
* [Проблема #757](https://github.com/manticoresoftware/manticoresearch/issues/757) Необходима проверка на пересечение диапазонов портов слушателей
* [Проблема #758](https://github.com/manticoresoftware/manticoresearch/issues/758) Записать оригинальную ошибку в случае, если RT индекс не удалось сохранить на диск
* [Проблема #759](https://github.com/manticoresoftware/manticoresearch/issues/759) Сообщается только одна ошибка для конфигурации RE2
* ❗[Проблема #760](https://github.com/manticoresoftware/manticoresearch/issues/760) Изменения потребления ОЗУ в коммите 5463778558586d2508697fa82e71d657ac36510f
* [Проблема #761](https://github.com/manticoresoftware/manticoresearch/issues/761) 3-й узел не создает непервичный кластер после грязного перезапуска
* [Проблема #762](https://github.com/manticoresoftware/manticoresearch/issues/762) Счетчик обновлений увеличивается на 2
* [Проблема #763](https://github.com/manticoresoftware/manticoresearch/issues/763) Новая версия 4.2.1 повреждает индекс, созданный с 4.2.0 с использованием морфологии
* [Проблема #764](https://github.com/manticoresoftware/manticoresearch/issues/764) Нет экранирования в ключах json /sql?mode=raw
* ❗[Проблема #765](https://github.com/manticoresoftware/manticoresearch/issues/765) Использование функции скрывает другие значения
* ❗[Проблема #766](https://github.com/manticoresoftware/manticoresearch/issues/766) Утечка памяти, вызванная строкой в FixupAttrForNetwork
* ❗[Проблема #767](https://github.com/manticoresoftware/manticoresearch/issues/767) Утечка памяти в 4.2.0 и 4.2.1, связанная с кэшем docstore
* [Проблема #768](https://github.com/manticoresoftware/manticoresearch/issues/768) Странный пинг-понг с сохраненными полями по сети
* [Проблема #769](https://github.com/manticoresoftware/manticoresearch/issues/769) lemmatizer_base сбрасывается в пустое значение, если не упомянуто в разделе 'common'
* [Проблема #770](https://github.com/manticoresoftware/manticoresearch/issues/770) pseudo_sharding делает SELECT по id медленнее
* [Проблема #771](https://github.com/manticoresoftware/manticoresearch/issues/771) DEBUG malloc_stats выводит нули при использовании jemalloc
* [Проблема #772](https://github.com/manticoresoftware/manticoresearch/issues/772) Удаление/добавление столбца делает значение невидимым
* [Проблема #773](https://github.com/manticoresoftware/manticoresearch/issues/773) Невозможно добавить столбец bit(N) в колонную таблицу
* [Проблема #774](https://github.com/manticoresoftware/manticoresearch/issues/774) "cluster" становится пустым при запуске в manticore.json
* ❗[Коммит 1da4](https://github.com/manticoresoftware/manticoresearch/commits/1da4ce891e4ff13727cfd3331d72771c64ee7949) HTTP действия не отслеживаются в SHOW STATUS
* [Коммит 3810](https://github.com/manticoresoftware/manticoresearch/commits/381000ab4af4c3c98ec8c730699ad7f39039cec8) отключить pseudo_sharding для запросов с низкой частотой по одному ключевому слову
* [Коммит 8003](https://github.com/manticoresoftware/manticoresearch/commits/800325cca283a87801b28929d82420fafc76a0ee) исправлено слияние сохраненных атрибутов и индекса
* [Коммит cddf](https://github.com/manticoresoftware/manticoresearch/commits/cddfeed6296f2041c6aae18414e5cd9cabf08281) обобщены выборщики уникальных значений; добавлены специализированные выборщики уникальных значений для колонных строк
* [Коммит fba4](https://github.com/manticoresoftware/manticoresearch/commits/fba4bb4f) исправлено извлечение нулевых целочисленных атрибутов из docstore
* [Коммит f300](https://github.com/manticoresoftware/manticoresearch/commit/f3009a9242fa16956adf871c4887e6e0303ba364) `ranker` может быть указан дважды в журнале запросов

## Версия 4.2.0, 23 дек 2021

### Основные новые функции
* **Поддержка псевдо-шардирования для индексов в реальном времени и полнотекстовых запросов**. В предыдущем релизе мы добавили ограниченную поддержку псевдо-шардирования. Начиная с этой версии, вы можете получить все преимущества псевдо-шардирования и вашего многоядерного процессора, просто включив [searchd.pseudo_sharding](Server_settings/Searchd.md#pseudo_sharding). Самое крутое в этом то, что вам не нужно ничего делать с вашими индексами или запросами для этого, просто включите это, и если у вас есть свободный ЦП, он будет использован для снижения времени ответа. Это поддерживает обычные и индексы в реальном времени для полнотекстовых, фильтрационных и аналитических запросов. Например, вот как включение псевдо-шардирования может сделать **время ответа большинства запросов в среднем примерно в 10 раз ниже** на [наборе данных курируемых комментариев Hacker news](https://zenodo.org/record/45901/), умноженном на 100 (116 миллионов документов в обычном индексе).

![Псевдо-шардирование включено и выключено в 4.2.0](4.2.0_ps_on_vs_off.png)

* [**Debian Bullseye**](https://manticoresearch.com/install/) теперь поддерживается.

<!-- example pq_transactions_4.2.0 -->
* PQ транзакции теперь атомарны и изолированы. Ранее поддержка PQ транзакций была ограничена. Это позволяет значительно **ускорить REPLACE в PQ**, особенно когда вам нужно заменить много правил сразу. Подробности о производительности:

<!-- intro -->
#### 4.0.2

<!-- request 4.0.2 -->

Вставка **1M PQ правил занимает 48 секунд** и **REPLACE всего 40K** в 10K пакетах занимает 406 секунд.

```sql
root@perf3 ~ # mysql -P9306 -h0 -e "drop table if exists pq; create table pq (f text, f2 text, j json, s string) type='percolate';"; date; for m in `seq 1 1000`; do (echo -n "insert into pq (id,query,filters,tags) values "; for n in `seq 1 1000`; do echo -n "(0,'@f (cat | ( angry dog ) | (cute mouse)) @f2 def', 'j.json.language=\"en\"', '{\"tag1\":\"tag1\",\"tag2\":\"tag2\"}')"; [ $n != 1000 ] && echo -n ","; done; echo ";")|mysql -P9306 -h0; done; date; mysql -P9306 -h0 -e "select count(*) from pq"

Wed Dec 22 10:24:30 AM CET 2021
Wed Dec 22 10:25:18 AM CET 2021
+----------+
| count(*) |
+----------+
|  1000000 |
+----------+

root@perf3 ~ # date; (echo "begin;"; for offset in `seq 0 10000 30000`; do n=0; echo "replace into pq (id,query,filters,tags) values "; for id in `mysql -P9306 -h0 -NB -e "select id from pq limit $offset, 10000 option max_matches=1000000"`; do echo "($id,'@f (tiger | ( angry bear ) | (cute panda)) @f2 def', 'j.json.language=\"de\"', '{\"tag1\":\"tag1\",\"tag2\":\"tag2\"}')"; n=$((n+1)); [ $n != 10000 ] && echo -n ","; done; echo ";"; done; echo "commit;") > /tmp/replace.sql; date
Wed Dec 22 10:26:23 AM CET 2021
Wed Dec 22 10:26:27 AM CET 2021
root@perf3 ~ # time mysql -P9306 -h0 < /tmp/replace.sql

real	6m46.195s
user	0m0.035s
sys	0m0.008s
```

<!-- intro -->

#### 4.2.0

<!-- request 4.2.0 -->

Вставка **1M PQ правил занимает 34 секунды** и **REPLACE их** в 10K пакетах занимает 23 секунды.

```sql
root@perf3 ~ # mysql -P9306 -h0 -e "drop table if exists pq; create table pq (f text, f2 text, j json, s string) type='percolate';"; date; for m in `seq 1 1000`; do (echo -n "insert into pq (id,query,filters,tags) values "; for n in `seq 1 1000`; do echo -n "(0,'@f (cat | ( angry dog ) | (cute mouse)) @f2 def', 'j.json.language=\"en\"', '{\"tag1\":\"tag1\",\"tag2\":\"tag2\"}')"; [ $n != 1000 ] && echo -n ","; done; echo ";")|mysql -P9306 -h0; done; date; mysql -P9306 -h0 -e "select count(*) from pq"

Wed Dec 22 10:06:38 AM CET 2021
Wed Dec 22 10:07:12 AM CET 2021
+----------+
| count(*) |
+----------+
|  1000000 |
+----------+

root@perf3 ~ # date; (echo "begin;"; for offset in `seq 0 10000 990000`; do n=0; echo "replace into pq (id,query,filters,tags) values "; for id in `mysql -P9306 -h0 -NB -e "select id from pq limit $offset, 10000 option max_matches=1000000"`; do echo "($id,'@f (tiger | ( angry bear ) | (cute panda)) @f2 def', 'j.json.language=\"de\"', '{\"tag1\":\"tag1\",\"tag2\":\"tag2\"}')"; n=$((n+1)); [ $n != 10000 ] && echo -n ","; done; echo ";"; done; echo "commit;") > /tmp/replace.sql; date
Wed Dec 22 10:12:31 AM CET 2021
Wed Dec 22 10:14:00 AM CET 2021
root@perf3 ~ # time mysql -P9306 -h0 < /tmp/replace.sql

real	0m23.248s
user	0m0.891s
sys	0m0.047s
```

<!-- end -->

### Небольшие изменения
*  [optimize_cutoff](Server_settings/Searchd.md#optimize_cutoff) теперь доступен как параметр конфигурации в разделе `searchd`. Это полезно, когда вы хотите ограничить количество RT чанков во всех ваших индексах до определенного числа глобально.
* [Commit 0087](https://github.com/manticoresoftware/manticoresearch/commit/00874743823eaf43e2a09a088392fd7e0a617f47) точный [count(distinct ...)](Searching/Grouping.md#COUNT%28DISTINCT-field%29) и [FACET ... distinct](Searching/Faceted_search.md#Faceting-without-duplicates) по нескольким локальным физическим индексам (реального времени/обычным) с идентичными полями и порядком.
* [PR #598](https://github.com/manticoresoftware/manticoresearch/pull/598) поддержка bigint для `YEAR()` и других функций временных меток.
* [Commit 8e85](https://github.com/manticoresoftware/manticoresearch/commit/8e85d4bce596cc88f9d9615ea243a27e4afde0f7) Адаптивный [rt_mem_limit](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_mem_limit). Ранее Manticore Search собирал ровно до `rt_mem_limit` данных перед сохранением нового дискового чанка на диск, и во время сохранения все еще собирал до 10% больше (так называемый двойной буфер), чтобы минимизировать возможные задержки вставки. Если этот лимит также исчерпывался, добавление новых документов блокировалось до полного сохранения дискового чанка на диск. Новый адаптивный лимит основан на том факте, что у нас теперь есть [auto-optimize](Server_settings/Searchd.md#auto_optimize), так что это не большая проблема, если дисковые чанки не полностью соблюдают `rt_mem_limit` и начинают сбрасывать дисковый чанк раньше. Теперь мы собираем до 50% `rt_mem_limit` и сохраняем это как дисковый чанк. При сохранении мы смотрим на статистику (сколько мы сохранили, сколько новых документов поступило во время сохранения) и пересчитываем начальную скорость, которая будет использоваться в следующий раз. Например, если мы сохранили 90 миллионов документов, и еще 10 миллионов документов поступило во время сохранения, скорость составляет 90%, так что мы знаем, что в следующий раз мы можем собрать до 90% `rt_mem_limit` перед началом сброса другого дискового чанка. Значение скорости рассчитывается автоматически от 33.3% до 95%.
* [Issue #628](https://github.com/manticoresoftware/manticoresearch/issues/628) [unpack_zlib](Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Database_connection.md#unpack_zlib) для источника PostgreSQL. Спасибо, [Дмитрий Воронин](https://github.com/dimv36) за [вклад](https://github.com/manticoresoftware/manticoresearch/pull/630).
* [Commit 6d54](https://github.com/manticoresoftware/manticoresearch/commit/6d54cf2b319b131970c29410cc21abcbdf8083b1) `indexer -v` и `--version`. Ранее вы все еще могли видеть версию индексатора, но `-v`/`--version` не поддерживались.
* [Issue #662](https://github.com/manticoresoftware/manticoresearch/issues/662) бесконечный лимит mlock по умолчанию, когда Manticore запускается через systemd.
* [Commit 63c8](https://github.com/manticoresoftware/manticoresearch/commit/63c8cd05cf399e705c2c14939411a92cb784735a) spinlock -> op queue для coro rwlock.
* [Commit 4113](https://github.com/manticoresoftware/manticoresearch/commit/41130ce30d1734bbf8e6b8b63c4e815caf9464f2) переменная окружения `MANTICORE_TRACK_RT_ERRORS`, полезная для отладки повреждений сегментов RT.

### Ломающее изменения
* Версия Binlog была увеличена, binlog из предыдущей версии не будет воспроизведен, поэтому убедитесь, что вы останавливаете Manticore Search корректно во время обновления: никаких файлов binlog не должно быть в `/var/lib/manticore/binlog/`, кроме `binlog.meta` после остановки предыдущего экземпляра.
* [Commit 3f65](https://github.com/manticoresoftware/manticoresearch/commit/3f659f36e87d99ee262205a8eae4663a255be630) новый столбец "chain" в `show threads option format=all`. Он показывает стек некоторых информационных билетов задач, наиболее полезен для профилирования, так что если вы парсите вывод `show threads`, будьте внимательны к новому столбцу.
* `searchd.workers` был устаревшим с версии 3.5.0, теперь он считается устаревшим, если вы все еще имеете его в вашем конфигурационном файле, это вызовет предупреждение при запуске. Manticore Search запустится, но с предупреждением.
* Если вы используете PHP и PDO для доступа к Manticore, вам нужно установить `PDO::ATTR_EMULATE_PREPARES`

### Исправления ошибок
* ❗[Проблема #650](https://github.com/manticoresoftware/manticoresearch/issues/650) Manticore 4.0.2 медленнее, чем Manticore 3.6.3. 4.0.2 был быстрее, чем предыдущие версии по скорости массовых вставок, но значительно медленнее для вставок одного документа. Это было исправлено в 4.2.0.
* ❗[Коммит 22f4](https://github.com/manticoresoftware/manticoresearch/commit/22f4141bed1411c83684ae6e0bae832071cce3df) Индекс RT мог быть поврежден под интенсивной нагрузкой REPLACE, или он мог аварийно завершиться
* [Коммит 03be](https://github.com/manticoresoftware/manticoresearch/commit/03be91e4f720a3f20bee0278dd8f0d44abd3ef0e) исправлено среднее значение при объединении группировщиков и сортировщика группы N; исправлено объединение агрегатов
* [Коммит 2ea5](https://github.com/manticoresoftware/manticoresearch/commit/2ea575d34efe38661f14d858f2b552456f29fe42) `indextool --check` мог аварийно завершиться
* [Коммит 7ec7](https://github.com/manticoresoftware/manticoresearch/commit/7ec76d4abf01b5fca6f10a0000fe64d5d8e5fa52) Проблема исчерпания ОЗУ, вызванная UPDATE
* [Коммит 658a](https://github.com/manticoresoftware/manticoresearch/commit/658a727e41f3c3fbcde9754149528f34ed76ba19) демон мог зависнуть при INSERT
* [Коммит 46e4](https://github.com/manticoresoftware/manticoresearch/commit/46e42b9b386e31e53247ed634e158b533e28063e) демон мог зависнуть при завершении работы
* [Коммит f8d7](https://github.com/manticoresoftware/manticoresearch/commit/f8d7d5177cc83d250b493b4cfc619732b29cf072) демон мог аварийно завершиться при завершении работы
* [Коммит 733a](https://github.com/manticoresoftware/manticoresearch/commit/733accf165142a1d1574689525ccca8907971158) демон мог зависнуть при аварийном завершении
* [Коммит f7f8](https://github.com/manticoresoftware/manticoresearch/commit/f7f8bd8c0dbb23a59a88502dcba16b0918fb2330) демон мог аварийно завершиться при запуске, пытаясь повторно присоединиться к кластеру с недействительным списком узлов
* [Коммит 1401](https://github.com/manticoresoftware/manticoresearch/commit/140155613821ecaddcd1a3dfc4f7369b0df6aef7) распределенный индекс мог быть полностью забыт в режиме RT, если он не мог разрешить одного из своих агентов при запуске
* [Проблема #683](https://github.com/manticoresoftware/manticoresearch/issues/683) атрибут bit(N) engine='columnar' не работает
* [Проблема #682](https://github.com/manticoresoftware/manticoresearch/issues/682) создание таблицы не удалось, но оставляет директорию
* [Проблема #663](https://github.com/manticoresoftware/manticoresearch/issues/663) Конфигурация не работает с: неизвестное имя ключа 'attr_update_reserve'
* [Проблема #632](https://github.com/manticoresoftware/manticoresearch/issues/632) Manticore аварийно завершился при пакетных запросах
* [Проблема #679](https://github.com/manticoresoftware/manticoresearch/issues/679) Пакетные запросы снова вызывают аварии с v4.0.3
* [Коммит f7f8](https://github.com/manticoresoftware/manticoresearch/commit/f7f8bd8c0dbb23a59a88502dcba16b0918fb2330) исправлено аварийное завершение демона при запуске, пытаясь повторно присоединиться к кластеру с недействительным списком узлов
* [Проблема #643](https://github.com/manticoresoftware/manticoresearch/issues/643) Manticore 4.0.2 не принимает соединения после пакета вставок
* [Проблема #635](https://github.com/manticoresoftware/manticoresearch/issues/635) Запрос FACET с ORDER BY JSON.field или строковым атрибутом мог аварийно завершиться
* [Проблема #634](https://github.com/manticoresoftware/manticoresearch/issues/634) Авария SIGSEGV при запросе с packedfactors
* [Коммит 4165](https://github.com/manticoresoftware/manticoresearch/commits/41657f15f8fcf1e93cfad7f835bcf705d6e8349e) morphology_skip_fields не поддерживался при создании таблицы

## Версия 4.0.2, 21 сентября 2021 года

### Основные новые функции
- **Полная поддержка [Manticore Columnar Library](https://github.com/manticoresoftware/columnar)**. Ранее Manticore Columnar Library поддерживалась только для обычных индексов. Теперь она поддерживается:
  - в индексах реального времени для `INSERT`, `REPLACE`, `DELETE`, `OPTIMIZE`
  - в репликации
  - в `ALTER`
  - в `indextool --check`
- **Автоматическая компакция индексов** ([Проблема #478](https://github.com/manticoresoftware/manticoresearch/issues/478)). Наконец, вам не нужно вручную вызывать OPTIMIZE или через крон-задачу или другой вид автоматизации. Manticore теперь делает это автоматически и по умолчанию. Вы можете установить порог компакции по умолчанию через [optimize_cutoff](Server_settings/Setting_variables_online.md) глобальную переменную.
- **Переработка системы снимков и блокировок чанков**. Эти изменения могут быть невидимы снаружи на первый взгляд, но они значительно улучшают поведение многих вещей, происходящих в индексах реального времени. В двух словах, ранее большинство операций манипуляции данными Manticore сильно полагались на блокировки, теперь мы используем снимки дисковых чанков вместо этого.
- **Значительно более высокая производительность массовых вставок в индекс реального времени**. Например, на [сервере AX101 от Hetzner](https://www.hetzner.com/dedicated-rootserver/ax101) с SSD, 128 ГБ ОЗУ и AMD Ryzen™ 9 5950X (16*2 ядра) **с 3.6.0 вы могли вставить 236K документов в секунду** в таблицу со схемой `name text, email string, description text, age int, active bit(1)` (по умолчанию `rt_mem_limit`, размер пакета 25000, 16 параллельных рабочих процессов вставки, всего вставлено 16 миллионов документов). В 4.0.2 та же степень параллелизма/размер пакета/количество дает **357K документов в секунду**.

  <details>

  - операции чтения (например, SELECT, репликация) выполняются со снимками (snapshots)
  - операции, которые изменяют только внутреннюю структуру индекса без модификации схемы/документов (например, слияние RAM-сегментов, сохранение дисковых чанков, слияние дисковых чанков) выполняются с read-only снимками и в конце заменяют существующие чанки
  - UPDATE и DELETE выполняются над существующими чанками, но на случай, если происходит слияние, записи собираются и затем применяются к новым чанкам
  - UPDATE получают эксклюзивную блокировку последовательно для каждого чанка. Слияния получают shared блокировку при входе в этап сбора атрибутов из чанка. Таким образом, в любой момент времени только одна операция (слияние или обновление) имеет доступ к атрибутам чанка.
  - когда слияние достигает фазы, в которой ему нужны атрибуты, оно устанавливает специальный флаг. Когда UPDATE завершается, он проверяет этот флаг, и если он установлен, всё обновление сохраняется в специальной коллекции. Наконец, когда слияние завершается, оно применяет набор обновлений к новому дисковому чанку.
  - ALTER выполняется с эксклюзивной блокировкой
  - репликация выполняется как обычная операция чтения, но дополнительно сохраняет атрибуты перед SST и запрещает обновления во время SST

  </details>

- **[ALTER](Updating_table_schema_and_settings.md) может добавлять/удалять полнотекстовое поле** (в RT-режиме). Ранее можно было только добавлять/удалять атрибут.
- 🔬 **Экспериментально: псевдошардирование для запросов с полным сканированием** - позволяет распараллелить любой запрос, не являющийся полнотекстовым поиском. Вместо ручной подготовки шардов теперь можно просто включить новую опцию [searchd.pseudo_sharding](Server_settings/Searchd.md#pseudo_sharding) и ожидать снижения времени отклика до `CPU cores` раз для запросов, не являющихся полнотекстовым поиском. Обратите внимание, что это может легко занять все имеющиеся ядра CPU, поэтому если важна не только задержка, но и пропускная способность - используйте с осторожностью.

### Незначительные изменения
<!-- example -->
- Linux Mint и Ubuntu Hirsute Hippo поддерживаются через [APT-репозиторий](Installation/Debian_and_Ubuntu.md#APT-repository)
- более быстрое обновление по id через HTTP в больших индексах в некоторых случаях (зависит от распределения id)
- [671e65a2](https://github.com/manticoresoftware/lemmatizer-uk/commit/671e65a296800537123a3f8b324eeaa3f58b9632) - добавлено кэширование в lemmatizer-uk


<!-- intro -->
#### 3.6.0

<!-- request 3.6.0 -->
```
time curl -X POST -d '{"update":{"index":"idx","id":4611686018427387905,"doc":{"mode":0}}}' -H "Content-Type: application/x-ndjson" http://127.0.0.1:6358/json/bulk

real    0m43.783s
user    0m0.008s
sys     0m0.007s
```

<!-- intro -->
#### 4.0.2

<!-- request 4.0.2 -->
```
time curl -X POST -d '{"update":{"index":"idx","id":4611686018427387905,"doc":{"mode":0}}}' -H "Content-Type: application/x-ndjson" http://127.0.0.1:6358/json/bulk

real    0m0.006s
user    0m0.004s
sys     0m0.001s
```
<!-- end -->
- [пользовательские флаги запуска для systemd](Starting_the_server/Linux.md#Custom-startup-flags-using-systemd). Теперь не нужно запускать searchd вручную, если требуется запустить Manticore с каким-либо специфическим флагом запуска
- новая функция [LEVENSHTEIN()](Functions/String_functions.md#LEVENSHTEIN%28%29), которая вычисляет расстояние Левенштейна
- добавлены новые [флаги запуска searchd](Starting_the_server/Manually.md#searchd-command-line-options) `--replay-flags=ignore-trx-errors` и `--replay-flags=ignore-all-errors`, чтобы можно было запустить searchd, даже если binlog повреждён
- [Issue #621](https://github.com/manticoresoftware/manticoresearch/issues/621) - пробрасывание ошибок из RE2
- более точный [COUNT(DISTINCT)](Searching/Grouping.md#COUNT%28DISTINCT-field%29) для распределённых индексов, состоящих из локальных обычных индексов
- [FACET DISTINCT](Searching/Faceted_search.md#Faceting-without-duplicates) для удаления дубликатов при фасетном поиске
- [модификатор точной формы](Searching/Full_text_matching/Operators.md#Exact-form-modifier) теперь не требует [морфологии](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) и работает для индексов с включённым поиском по [инфиксам/префиксам](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology)

### Критические изменения
- новая версия может читать старые индексы, но старые версии не могут читать индексы Manticore 4
- удалена неявная сортировка по id. Сортируйте явно, если требуется
- значение по умолчанию для `charset_table` изменено с `0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451` на `non_cjk`
- `OPTIMIZE` выполняется автоматически. Если он не нужен, убедитесь, что установили `auto_optimize=0` в секции `searchd` конфигурационного файла
- [Issue #616](https://github.com/manticoresoftware/manticoresearch/issues/616) `ondisk_attrs_default` были устаревшими, теперь они удалены
- для контрибьюторов: теперь для сборок под Linux используется компилятор Clang, так как согласно нашим тестам он может собрать более быстрый Manticore Search и Manticore Columnar Library
- если [max_matches](Searching/Options.md#max_matches) не указан в поисковом запросе, он неявно обновляется минимально необходимым значением для повышения производительности нового колоночного хранилища. Это может повлиять на метрику `total` в [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META), но не на `total_found`, который является фактическим количеством найденных документов.

### Миграция с Manticore 3
- убедитесь, что вы остановили Manticore 3 корректно:
  - в `/var/lib/manticore/binlog/` не должно быть файлов binlog (в директории должен быть только `binlog.meta`)
  - иначе индексы, для которых Manticore 4 не сможет воспроизвести binlog, не будут запущены
- новая версия может читать старые индексы, но старые версии не могут читать индексы Manticore 4, поэтому убедитесь, что сделали резервную копию, если хотите иметь возможность легко откатиться на предыдущую версию
- если вы запускаете кластер репликации, убедитесь, что вы:
  - сначала корректно остановили все ваши ноды
  - а затем запустили ноду, которая была остановлена последней, с `--new-cluster` (запустите инструмент `manticore_new_cluster` в Linux).
  - прочитайте о [перезапуске кластера](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster) для получения более подробной информации

### Исправления ошибок
- Исправлено множество проблем с репликацией:
  - [Коммит 696f](https://github.com/manticoresoftware/manticoresearch/commit/696f8649535f2b5285b878c9581c0d2b1139ae09) - исправлен краш во время SST на узле-присоединяющемся при активном индексе; добавлена проверка sha1 на узле-присоединяющемся при записи файловых чанков для ускорения загрузки индекса; добавлена ротация измененных файлов индекса на узле-присоединяющемся при загрузке индекса; добавлено удаление файлов индекса на узле-присоединяющемся, когда активный индекс заменяется новым индексом от узла-донора; добавлены точки лога репликации на узле-доноре для отправки файлов и чанков
  - [Коммит b296](https://github.com/manticoresoftware/manticoresearch/commit/b296c55af68314a8fc66f8b104e7301d673c3b68) - краш при JOIN CLUSTER в случае некорректного адреса
  - [Коммит 418b](https://github.com/manticoresoftware/manticoresearch/commit/418bf880a5e05ffbb68f311a1c23006a9df3220e) - во время начальной репликации большого индекса присоединяющийся узел мог завершиться с ошибкой `ERROR 1064 (42000): invalid GTID, (null)`, донор мог перестать отвечать, пока другой узел присоединялся
  - [Коммит 6fd3](https://github.com/manticoresoftware/manticoresearch/commit/6fd350d28e4905211cf8a7298cbf7cdda101c83d) - хэш мог вычисляться неправильно для большого индекса, что могло привести к сбою репликации
  - [Issue #615](https://github.com/manticoresoftware/manticoresearch/issues/615) - репликация завершалась сбоем при перезапуске кластера
- [Issue #574](https://github.com/manticoresoftware/manticoresearch/issues/574) - `indextool --help` не отображает параметр `--rotate`
- [Issue #578](https://github.com/manticoresoftware/manticoresearch/issues/578) - высокое использование CPU процессом searchd в простое после примерно дня работы
- [Issue #587](https://github.com/manticoresoftware/manticoresearch/issues/587) - немедленная запись .meta файла
- [Issue #617](https://github.com/manticoresoftware/manticoresearch/issues/617) - файл manticore.json очищается
- [Issue #618](https://github.com/manticoresoftware/manticoresearch/issues/618) - `searchd --stopwait` завершается с ошибкой под root. Также исправлено поведение systemctl (ранее он показывал ошибку для ExecStop и не ждал достаточно долго для корректной остановки searchd)
- [Issue #619](https://github.com/manticoresoftware/manticoresearch/issues/619) - INSERT/REPLACE/DELETE vs SHOW STATUS. `command_insert`, `command_replace` и другие показывали неверные метрики
- [Issue #620](https://github.com/manticoresoftware/manticoresearch/issues/620) - `charset_table` для обычного индекса имела неверное значение по умолчанию
- [Коммит 8f75](https://github.com/manticoresoftware/manticoresearch/commit/8f7536887d4b58a5bab5647fc067b2d0482c2038) - новые дисковые чанки не блокируются в памяти (mlock)
- [Issue #607](https://github.com/manticoresoftware/manticoresearch/issues/607) - Узел кластера Manticore падает, когда не может разрешить имя узла
- [Issue #623](https://github.com/manticoresoftware/manticoresearch/issues/623) - репликация обновленного индекса может привести к неопределенному состоянию
- [Коммит ca03](https://github.com/manticoresoftware/manticoresearch/commit/ca03d2280c5197e9b311f0f03f551e3b702a8130) - indexer мог зависнуть при индексировании источника обычного индекса с json-атрибутом
- [Коммит 53c7](https://github.com/manticoresoftware/manticoresearch/commit/53c753054558ff3f82c7af365273d32537c5338b) - исправлен фильтр с выражением "не равно" для PQ индекса
- [Коммит ccf9](https://github.com/manticoresoftware/manticoresearch/commit/ccf94e022ba6763d6286ab5bac767b6f6a846846) - исправлен выбор окон для списковых запросов свыше 1000 совпадений. `SELECT * FROM pq ORDER BY id desc LIMIT 1000 , 100 OPTION max_matches=1100` ранее не работал
- [Коммит a048](https://github.com/manticoresoftware/manticoresearch/commit/a0483fe9bef3c45dffe96f5df4f4382aee29c072) - HTTPS-запрос к Manticore мог вызывать предупреждение вида "max packet size(8388608) exceeded"
- [Issue #648](https://github.com/manticoresoftware/manticoresearch/issues/648) - Manticore 3 мог зависнуть после нескольких обновлений строковых атрибутов


## Версия 3.6.0, 3 мая 2021
**Поддерживающий релиз перед Manticore 4**

### Основные новые возможности
- Поддержка [Manticore Columnar Library](https://github.com/manticoresoftware/columnar/) для обычных индексов. Новая настройка [columnar_attrs](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#columnar_attrs) для обычных индексов
- Поддержка [Украинского лемматизатора](https://github.com/manticoresoftware/lemmatizer-uk)
- Полностью переработанные гистограммы. При построении индекса Manticore также строит гистограммы для каждого поля в нем, которые затем использует для более быстрой фильтрации. В версии 3.6.0 алгоритм был полностью переработан, и вы можете получить более высокую производительность, если у вас много данных и выполняется много фильтраций.

### Незначительные изменения
- Инструмент `manticore_new_cluster [--force]`, полезный для перезапуска кластера репликации через systemd
- [--drop-src](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Merging_tables.md#Merging-tables) для `indexer --merge`
- [Новый режим](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) `blend_mode='trim_all'`
- Добавлена [поддержка экранирования JSON пути](Searching/Full_text_matching/Escaping.md#Escaping-json-node-names-in-SQL) с помощью обратных кавычек
- [indextool --check](Miscellaneous_tools.md#indextool) может работать в режиме RT
- [FORCE/IGNORE INDEX(id)](Data_creation_and_modification/Updating_documents/UPDATE.md#Updates-via-SQL) для SELECT/UPDATE
- ID чанка для объединенного дискового чанка теперь уникален
- [indextool --check-disk-chunk CHUNK_NAME](Miscellaneous_tools.md#indextool)

### Оптимизации
- [Более быстрый разбор JSON](https://github.com/manticoresoftware/manticoresearch/commit/0c25fc1788e3f77f60cb7fb8f1623d8466bba606), наши тесты показывают снижение задержки на 3-4% для запросов вида `WHERE json.a = 1`
- Не документированная команда `DEBUG SPLIT` как предварительное условие для автоматического шардирования/перебалансировки

### Исправления ошибок
- [Проблема #584](https://github.com/manticoresoftware/manticoresearch/issues/524) - неточные и нестабильные результаты FACET
- [Проблема #506](https://github.com/manticoresoftware/manticoresearch/issues/506) - Странное поведение при использовании MATCH: тем, кто страдает от этой проблемы, необходимо перестроить индекс, так как проблема была на этапе построения индекса
- [Проблема #387](https://github.com/manticoresoftware/manticoresearch/issues/387) - периодический дамп ядра при выполнении запроса с функцией SNIPPET()
- Оптимизации стека, полезные для обработки сложных запросов:
  - [Проблема #469](https://github.com/manticoresoftware/manticoresearch/issues/469) - Результаты SELECT приводят к CRASH DUMP
  - [e8420cc7](https://github.com/manticoresoftware/manticoresearch/tree/e8420cc789149c84731be0874be916e33b263bd6) - определение размера стека для деревьев фильтров
- [Проблема #461](https://github.com/manticoresoftware/manticoresearch/issues/461) - Обновление с использованием условия IN не применяется корректно
- [Проблема #464](https://github.com/manticoresoftware/manticoresearch/issues/464) - SHOW STATUS сразу после CALL PQ возвращает - [Проблема #481](https://github.com/manticoresoftware/manticoresearch/issues/481) - Исправлена сборка статического бинарника
- [Проблема #502](https://github.com/manticoresoftware/manticoresearch/issues/502) - ошибка в многозапросах
- [Проблема #514](https://github.com/manticoresoftware/manticoresearch/issues/514) - Невозможно использовать необычные имена для столбцов при использовании 'create table'
- [Коммит d1db](https://github.com/manticoresoftware/manticoresearch/commit/d1dbe771288328c963ca8512f564c6b7cf4f466c) - сбой демона при воспроизведении binlog с обновлением строкового атрибута; установить версию binlog на 10
- [Коммит 775d](https://github.com/manticoresoftware/manticoresearch/commit/775d0555562f121911d3c30085947e743fadac2d)  - исправлено определение фрейма стека выражений во время выполнения (тест 207)
- [Коммит 4795](https://github.com/manticoresoftware/manticoresearch/commit/4795dc49194c7745761bb5006ecb266e8a9224fa)  - фильтр индекса перколяторов и теги были пустыми для пустого сохраненного запроса (тест 369)
- [Коммит c3f0](https://github.com/manticoresoftware/manticoresearch/commit/c3f0bf4dca0e4eae6ea899dd566878df57eba085)  - сбои репликации потока SST в сети с высокой задержкой и высоким уровнем ошибок (репликация в разных центрах обработки данных); обновлена версия команды репликации на 1.03
- [Коммит ba2d](https://github.com/manticoresoftware/manticoresearch/commit/ba2d6619b8107f3fb177741bbdff3709bac248cd)  - блокировка кластера joiner при операциях записи после присоединения к кластеру (тест 385)
- [Коммит de4d](https://github.com/manticoresoftware/manticoresearch/commit/de4dcb9ff6c316bc2a9ef6c62eb64769f04ee103)  - соответствие подстановочным знакам с точным модификатором (тест 321)
- [Коммит 6524](https://github.com/manticoresoftware/manticoresearch/commit/6524fc6af032a0932bd42c37874a619e1fb7f075)  - контрольные точки docid против docstore
- [Коммит f4ab](https://github.com/manticoresoftware/manticoresearch/commit/f4ab83c291e65a5873d271c67b5491f629544f1d)  - Непоследовательное поведение индексатора при разборе недопустимого xml
- [Коммит 7b72](https://github.com/manticoresoftware/manticoresearch/commit/7b727e22fe01b1a180d4a8d3a3fea92c18eac707)  - Сохраненный запрос перколяторов с NOTNEAR выполняется бесконечно (тест 349)
- [Коммит 812d](https://github.com/manticoresoftware/manticoresearch/commit/812dab745f0c972ecde5d5b3d4a313cd6e1e143e)  - неправильный вес для фразы, начинающейся с подстановочного знака
- [Коммит 1771](https://github.com/manticoresoftware/manticoresearch/commit/1771afc669c75670b3511bf02ab9bfc8982b94d2)  - запрос перколяторов с подстановочными знаками генерирует термины без полезной нагрузки при совпадении, что вызывает пересекающиеся попадания и нарушает совпадение (тест 417)
- [Коммит aa0d](https://github.com/manticoresoftware/manticoresearch/commit/aa0d8c2bcb41428d41dbfb66e42a872a79a4ee4b)  - исправлено вычисление 'total' в случае параллелизованного запроса
- [Коммит 18d8](https://github.com/manticoresoftware/manticoresearch/commit/18d81b3cfbb930b7361dae645fe13283d4fbf6ef)  - сбой в Windows с несколькими параллельными сессиями на демоне
- [Коммит 8443](https://github.com/manticoresoftware/manticoresearch/commit/84432f23d6fbf7a7d0b67c8c9412eb5933b03596)  - некоторые настройки индекса не могли быть реплицированы
- [Коммит 9341](https://github.com/manticoresoftware/manticoresearch/commit/93411fe6af4246ca4acce758944d513a1660b92b)  - При высокой скорости добавления новых событий netloop иногда зависает из-за атомарного события 'kick', обрабатываемого один раз для нескольких событий за раз и теряющего актуальные действия из них
статус запроса, а не статус сервера
- [Коммит d805](https://github.com/manticoresoftware/manticoresearch/commit/d805fc125244ef08d14fb3d1826e2c356862d539)  - Новый сброшенный диск может быть потерян при коммите
- [Коммит 63cb](https://github.com/manticoresoftware/manticoresearch/commit/63cbf008d50979cdf7062664fb02a672a3bb1131)  - неточная 'net_read' в профайлере
- [Коммит f537](https://github.com/manticoresoftware/manticoresearch/commit/f5379bb27234b25919966c50ccb6f469ead646e2)  - Проблема с перколятором на арабском (тексты справа налево)
- [Коммит 49ee](https://github.com/manticoresoftware/manticoresearch/commit/49eeb4202d4ce4875c07c35087d7108c1e177328)  - id не выбирается корректно при дублирующемся имени столбца
- [Коммит refa](https://github.com/manticoresoftware/manticoresearch/commit/4b7df262bbfec9b9e95a44e72ad326ac8a786c49) сетевых событий для исправления сбоя в редких случаях
- [e8420cc7](https://github.com/manticoresoftware/manticoresearch/tree/e8420cc789149c84731be0874be916e33b263bd6) исправление в `indextool --dumpheader`
- [Коммит ff71](https://github.com/manticoresoftware/manticoresearch/commit/ff7163532ed39042c0d3c3c2b902ff41f94d7f0c) - TRUNCATE WITH RECONFIGURE работал неправильно с сохраненными полями

### Критические изменения:
- Новый формат бинарного лога: необходимо выполнить полную остановку Manticore перед обновлением
- Формат индекса немного изменяется: новая версия может читать ваши существующие индексы, но если вы решите откатиться с версии 3.6.0 на более старую, новые индексы будут недоступны для чтения
- Изменение формата репликации: не реплицируйте со старой версии на 3.6.0 и наоборот, переключайтесь на новую версию на всех ваших узлах одновременно
- `reverse_scan` устарел. Убедитесь, что вы не используете эту опцию в своих запросах, начиная с версии 3.6.0, иначе они завершатся ошибкой
- Начиная с этого релиза мы больше не предоставляем сборки для RHEL6, Debian Jessie и Ubuntu Trusty. Если для вас критически важно, чтобы они поддерживались, [свяжитесь с нами](https://manticoresearch.com/contact-us/)

### Устаревания
- Больше не используется неявная сортировка по id. Если вы полагаетесь на нее, обязательно обновите свои запросы соответствующим образом
- Опция поиска `reverse_scan` устарела

## Версия 3.5.4, 10 декабря 2020

### Новые возможности
- Новые клиенты для Python, Javascript и Java теперь общедоступны и хорошо задокументированы в этом руководстве.
- Автоматическое удаление дискового чанка реального индекса. Эта оптимизация позволяет автоматически удалять дисковый чанк при [OPTIMIZировании](Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE) реального индекса, когда чанк явно больше не нужен (все документы подавлены). Ранее это все равно требовало слияния, теперь чанк можно просто мгновенно удалить. Опция [cutoff](Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks) игнорируется, т.е. даже если фактически ничего не сливается, устаревший дисковый чанк удаляется. Это полезно, если вы поддерживаете хранение данных в своем индексе и удаляете старые документы. Теперь уплотнение таких индексов будет быстрее.
- [Автономный NOT](Searching/Options.md#not_terms_only_allowed) как опция для SELECT

### Незначительные изменения
- [Issue #453](https://github.com/manticoresoftware/manticoresearch/issues/453) Новая опция [indexer.ignore_non_plain=1](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#ignore_non_plain) полезна, если вы запускаете `indexer --all` и в файле конфигурации есть не только простые индексы. Без `ignore_non_plain=1` вы получите предупреждение и соответствующий код завершения.
- [SHOW PLAN ... OPTION format=dot](Node_info_and_management/Profiling/Query_plan.md#Dot-format-for-SHOW-PLAN) и [EXPLAIN QUERY ... OPTION format=dot](Searching/Full_text_matching/Profiling.md#Profiling-without-running-a-query) позволяют визуализировать выполнение плана полнотекстового запроса. Полезно для понимания сложных запросов.

### Устаревания
- `indexer --verbose` устарел, так как он никогда ничего не добавлял к выводу indexer
- Для дампа трассировки сторожевого процесса теперь следует использовать сигнал `USR2` вместо `USR1`

### Исправления ошибок
- [Issue #423](https://github.com/manticoresoftware/manticoresearch/issues/423) кириллический символ точка вызов сниппетов режим retain не подсвечивает
- [Issue #435](https://github.com/manticoresoftware/manticoresearch/issues/435) RTINDEX - GROUP N BY выражение select = фатальный сбой
- [Commit 2b3b](https://github.com/manticoresoftware/manticoresearch/commit/2b3b62bd0ce2b54544733d524a169e81f71b5388) статус searchd показывает Segmentation fault при работе в кластере
- [Commit 9dd2](https://github.com/manticoresoftware/manticoresearch/commit/9dd25c193ed2a53750a0e0e68566fd2b833fcb0c) 'SHOW INDEX index.N SETTINGS' не обращается к чанкам >9
- [Issue #389](https://github.com/manticoresoftware/manticoresearch/issues/389) Ошибка, вызывающая сбой Manticore
- [Commit fba1](https://github.com/manticoresoftware/manticoresearch/commit/fba166178ff35c73a0c70d131cf3aca4a635e961) Конвертер создает поврежденные индексы
- [Commit eecd](https://github.com/manticoresoftware/manticoresearch/commit/eecd61d861c742471175be19a5e5455034b5db5f) stopword_step=0 vs CALL SNIPPETS()
- [Commit ea68](https://github.com/manticoresoftware/manticoresearch/commit/ea6850e449601199190f38a387c04b8a7b7e90f6) count distinct возвращает 0 при низком max_matches на локальном индексе
- [Commit 362f](https://github.com/manticoresoftware/manticoresearch/commit/362f27db3421f09a41c6cce04b5baace00fa4c4d) При использовании агрегации сохраненные тексты не возвращаются в результатах


## Версия 3.5.2, 1 октября 2020

### Новые возможности

* OPTIMIZE сокращает количество дисковых чанков до числа чанков (по умолчанию `2* Количество ядер`) вместо одного. Оптимальное количество чанков можно контролировать с помощью опции [cutoff](Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks).
* Оператор NOT теперь можно использовать автономно. По умолчанию он отключен, поскольку случайные одиночные запросы NOT могут быть медленными. Его можно включить, установив новую директиву searchd [not_terms_only_allowed](Server_settings/Searchd.md#not_terms_only_allowed) в `0`.
* Новая настройка [max_threads_per_query](Server_settings/Searchd.md#max_threads_per_query) устанавливает, сколько потоков может использовать запрос. Если директива не установлена, запрос может использовать потоки до значения [threads](Server_settings/Searchd.md#threads).
Для каждого запроса `SELECT` количество потоков можно ограничить с помощью [OPTION threads=N](Searching/Options.md#threads), переопределяя глобальный `max_threads_per_query`.
* Перколяционные индексы теперь можно импортировать с помощью [IMPORT TABLE](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Importing_table.md).
* HTTP API `/search` получает базовую поддержку [фасетного поиска](Searching/Faceted_search.md#HTTP-JSON)/[группировки](Searching/Grouping.md) с помощью нового узла запроса `aggs`.

### Незначительные изменения

* Если не объявлена директива прослушивания репликации, движок попытается использовать порты после определенного порта 'sphinx', до 200.
* `listen=...:sphinx` необходимо явно установить для подключений SphinxSE или клиентов SphinxAPI.
* [SHOW INDEX STATUS](Node_info_and_management/Table_settings_and_status/SHOW_TABLE_STATUS.md) выводит новые метрики: `killed_documents`, `killed_rate`, `disk_mapped_doclists`, `disk_mapped_cached_doclists`, `disk_mapped_hitlists` и `disk_mapped_cached_hitlists`.
* SQL команда `status` теперь выводит `Queue\Threads` и `Tasks\Threads`.

### Устаревания:

* `dist_threads` теперь полностью устарел, searchd будет записывать предупреждение, если директива все еще используется.

### Docker

Официальный образ Docker теперь основан на Ubuntu 20.04 LTS

### Упаковка

Помимо обычного пакета `manticore`, вы также можете установить Manticore Search по компонентам:

- `manticore-server-core` - предоставляет `searchd`, manpage, каталог логов, API и модуль galera. Он также установит `manticore-common` как зависимость.
- `manticore-server` - предоставляет автоматизационные скрипты для ядра (init.d, systemd) и обертку `manticore_new_cluster`. Он также установит `manticore-server-core` как зависимость.
- `manticore-common` - предоставляет конфигурацию, стоп-слова, общие документы и скелетные папки (datadir, модули и т.д.)
- `manticore-tools` - предоставляет вспомогательные инструменты (`indexer`, `indextool` и т.д.), их manpages и примеры. Он также установит `manticore-common` как зависимость.
- `manticore-icudata` (RPM) или `manticore-icudata-65l` (DEB) - предоставляет файл данных ICU для использования морфологии icu.
- `manticore-devel` (RPM) или `manticore-dev` (DEB) - предоставляет заголовки для разработки UDF.

### Исправления ошибок

1. [Коммит 2a47](https://github.com/manticoresoftware/manticoresearch/commit/2a474dc1a26e8b0f8aaaae95669caf2f1d4b7746) Сбой демона в группировщике на RT индексе с разными чанками
2. [Коммит 57a1](https://github.com/manticoresoftware/manticoresearch/commit/57a19e5ad5663ef0ca7436595218fb1221d28c8e) Быстрый путь для пустых удаленных документов
3. [Коммит 07dd](https://github.com/manticoresoftware/manticoresearch/commit/07dd3f313c63fb82c22092f9907ef24e3475250e) Обнаружение фрейма стека выражений во время выполнения
4. [Коммит 08ae](https://github.com/manticoresoftware/manticoresearch/commit/08ae357cf1012bc8e2da54c20b205b592efda3d4) Сопоставление более 32 полей в индексах перколяции
5. [Коммит 16b9](https://github.com/manticoresoftware/manticoresearch/commit/16b9390fd4cdb07a77ac4497adb935573a1710e5) Диапазон портов для прослушивания репликации
6. [Коммит 5fa6](https://github.com/manticoresoftware/manticoresearch/commit/5fa671affeacb6441a59c8a88479bfd423df7c81) Показать создание таблицы на pq
7. [Коммит 54d1](https://github.com/manticoresoftware/manticoresearch/commit/54d133b6449105a9fb0168db3f1fbb05fb5aa1f6) Поведение порта HTTPS
8. [Коммит fdbb](https://github.com/manticoresoftware/manticoresearch/commit/fdbbe5245cc296cc5c1ae3ae2fb9cb08fb66a248) Смешивание строк docstore при замене
9. [Коммит afb5](https://github.com/manticoresoftware/manticoresearch/commit/afb53f648ef4e64fa1776c58e66f6e716ac730ad) Переключение уровня сообщения о недоступности TFO на 'info'
10. [Коммит 59d9](https://github.com/manticoresoftware/manticoresearch/commit/59d94cefc6e88af360b8046726a16b7eaa9f0b71) Сбой при неверном использовании strcmp
11. [Коммит 04af](https://github.com/manticoresoftware/manticoresearch/commit/04af0349c1e784c2230d204592d6272b440bb375) Добавление индекса в кластер с системными (стоп-словами) файлами
12. [Коммит 5014](https://github.com/manticoresoftware/manticoresearch/commit/50148b4eae9f3a924082005cf5d2f9c8339af7f5) Объединение индексов с большими словарями; оптимизация RT больших дисковых чанков
13. [Коммит a2ad](https://github.com/manticoresoftware/manticoresearch/commit/a2adf1582843f63d4dcc1dab8f7598e0917094a2) Indextool может выгружать метаданные из текущей версии
14. [Коммит 69f6](https://github.com/manticoresoftware/manticoresearch/commit/69f6d5f77c64b97e3f395aea8c555b11c77fb732) Проблема в порядке группировки в GROUP N
15. [Коммит 24d5](https://github.com/manticoresoftware/manticoresearch/commit/24d5d80f508576794b751d70f9f9b524564e3c14) Явная очистка для SphinxSE после рукопожатия
16. [Коммит 31c4](https://github.com/manticoresoftware/manticoresearch/commit/31c4d78ac03ee85d9a02b86075448a04ac80f9e2) Избегать копирования огромных описаний, когда это не нужно
17. [Коммит 2959](https://github.com/manticoresoftware/manticoresearch/commit/2959e2caa8610f98fcdf2e5a82160f58687f4a27) Отрицательное время в показе потоков
18. [Коммит f0b3](https://github.com/manticoresoftware/manticoresearch/commit/f0b357102c70f3ac1d7da260c60b67322940e647) Плагин фильтра токенов против нулевых дельт позиции
19. [Коммит a49e](https://github.com/manticoresoftware/manticoresearch/commit/a49e5bc13ea763e6f01dd5993e891858a84d1846) Изменить 'FAIL' на 'WARNING' при нескольких попаданиях

## Версия 3.5.0, 22 июля 2020

### Основные новые функции:
* Этот релиз занял так много времени, потому что мы усердно работали над изменением многозадачного режима с потоков на **корутины**. Это упрощает конфигурацию и делает параллелизацию запросов гораздо более понятной: Manticore просто использует заданное количество потоков (см. новую настройку [threads](Server_settings/Searchd.md#threads)), и новый режим гарантирует, что это сделано наиболее оптимальным образом.
* Изменения в [подсветке](Searching/Highlighting.md#Highlighting-options):
  - любая подсветка, которая работает с несколькими полями (`highlight({},'field1, field2'`) или `highlight` в json запросах) теперь по умолчанию применяет ограничения на поле.
  - любая подсветка, которая работает с обычным текстом (`highlight({}, string_attr)` или `snippet()` теперь применяет ограничения ко всему документу.
  - [ограничения на поле](Searching/Highlighting.md#limits_per_field) могут быть переключены на глобальные ограничения с помощью опции `limits_per_field=0` (`1` по умолчанию).
  - [allow_empty](Searching/Highlighting.md#allow_empty) теперь `0` по умолчанию для подсветки через HTTP JSON.

* Один и тот же порт [теперь может использоваться](Server_settings/Searchd.md#listen) для http, https и бинарного API (для принятия соединений от удаленного экземпляра Manticore). `listen = *:mysql` по-прежнему требуется для соединений через mysql протокол. Manticore теперь автоматически определяет тип клиента, пытающегося подключиться к нему, за исключением MySQL (из-за ограничений протокола).

* В режиме RT поле теперь может быть [текстовым и строковым атрибутом](Creating_a_table/Data_types.md#String) одновременно - [проблема GitHub #331](https://github.com/manticoresoftware/manticoresearch/issues/331).

  В [обычном режиме](Read_this_first.md#Real-time-mode-vs-plain-mode) это называется `sql_field_string`. Теперь это доступно в [RT режиме](Read_this_first.md#Real-time-mode-vs-plain-mode) для индексов реального времени тоже. Вы можете использовать это, как показано в примере:

  <!-- more -->
  ```sql
  create table t(f string attribute indexed);
  insert into t values(0,'abc','abc');
  select * from t where match('abc');
  +---------------------+------+
  | id                  | f    |
  +---------------------+------+
  | 2810845392541843463 | abc  |
  +---------------------+------+
  1 row in set (0.01 sec)

  mysql> select * from t where f='abc';
  +---------------------+------+
  | id                  | f    |
  +---------------------+------+
  | 2810845392541843463 | abc  |
  +---------------------+------+
  1 row in set (0.00 sec)
  ```
  <!-- \more -->

### Незначительные изменения
* Теперь можно [подсвечивать строковые атрибуты](Searching/Highlighting.md#Highlighting-via-SQL).
* Поддержка SSL и сжатия для SQL-интерфейса
* Поддержка команды [`status`](Node_info_and_management/Node_status.md#STATUS) клиента mysql.
* [Репликация](Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Setting-up-replication) теперь может реплицировать внешние файлы (стоп-слова, исключения и т.д.).
* Оператор фильтрации [`in`](Searching/Filters.md#Set-filters) теперь доступен через HTTP JSON интерфейс.
* [`expressions`](Searching/Expressions.md#expressions) в HTTP JSON.
* [Теперь можно изменять `rt_mem_limit` на лету](https://github.com/manticoresoftware/manticoresearch/issues/344) в RT-режиме, т.е. можно выполнять `ALTER ... rt_mem_limit=<новое значение>`.
* Теперь можно использовать [отдельные таблицы символов для CJK](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology): `chinese`, `japanese` и `korean`.
* [thread_stack](Server_settings/Searchd.md#thread_stack) теперь ограничивает максимальный стек потока, а не начальный.
* Улучшен вывод `SHOW THREADS`.
* Отображение прогресса длительного `CALL PQ` в `SHOW THREADS`.
* cpustat, iostat, coredump можно изменять во время выполнения с помощью [SET](Server_settings/Setting_variables_online.md#SET).
* Реализовано `SET [GLOBAL] wait_timeout=NUM` ,

### Критические изменения:
* **Формат индекса был изменен.** Индексы, построенные в версии 3.5.0, не могут быть загружены Manticore версии < 3.5.0, но Manticore 3.5.0 понимает старые форматы.
* [`INSERT INTO PQ VALUES()`](Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md) (т.е. без указания списка столбцов) ранее ожидал строго значения `(query, tags)`. Это изменено на `(id,query,tags,filters)`. ID можно установить в 0, если требуется его авто-генерация.
* [`allow_empty=0`](Searching/Highlighting.md#allow_empty) теперь значение по умолчанию при подсветке через HTTP JSON интерфейс.
* В `CREATE TABLE`/`ALTER TABLE` для внешних файлов (стоп-слов, исключений и т.д.) разрешены только абсолютные пути.

### Устаревшие возможности:
* `ram_chunks_count` переименована в `ram_chunk_segments_count` в `SHOW INDEX STATUS`.
* `workers` устарел. Теперь существует только один режим workers.
* `dist_threads` устарел. Все запросы теперь максимально параллельны (ограничены `threads` и `jobs_queue_size`).
* `max_children` устарел. Используйте [threads](Server_settings/Searchd.md#threads) для установки количества потоков, которые будет использовать Manticore (по умолчанию равно количеству ядер ЦП).
* `queue_max_length` устарел. Вместо этого, если это действительно необходимо, используйте [jobs_queue_size](Server_settings/Searchd.md#jobs_queue_size) для тонкой настройки размера внутренней очереди заданий (по умолчанию не ограничен).
* Все конечные точки `/json/*` теперь доступны без `/json/`, например, `/search`, `/insert`, `/delete`, `/pq` и т.д.
* Значение `field`, означающее "полнотекстовое поле", переименовано в "text" в `describe`.
  <!-- more -->
  **3.4.2:**
  ```sql
  mysql> describe t;
  +-------+--------+----------------+
  | Field | Type   | Properties     |
  +-------+--------+----------------+
  | id    | bigint |                |
  | f     | field  | indexed stored |
  +-------+--------+----------------+
  ```
  **3.5.0**:
  ```sql
  mysql> describe t;
  +-------+--------+----------------+
  | Field | Type   | Properties     |
  +-------+--------+----------------+
  | id    | bigint |                |
  | f     | text   | indexed stored |
  +-------+--------+----------------+
  ```
  <!-- \more -->
* Кириллическая `и` больше не отображается в `i` в `non_cjk` charset_table (которая используется по умолчанию), так как это слишком сильно влияло на русские стеммеры и лемматизаторы.
* `read_timeout`. Используйте вместо этого [network_timeout](Server_settings/Searchd.md#network_timeout), который контролирует как чтение, так и запись.


### Пакеты

* Официальный пакет для Ubuntu Focal 20.04
* Имя deb-пакета изменено с `manticore-bin` на `manticore`

### Исправления ошибок:
1. [Проблема #351](https://github.com/manticoresoftware/manticoresearch/issues/351) утечка памяти searchd
2. [Коммит ceab](https://github.com/manticoresoftware/manticoresearch/commit/ceabe44f) Небольшое чтение за пределами в фрагментах
3. [Коммит 1c3e](https://github.com/manticoresoftware/manticoresearch/commit/1c3e84a3) Опасная запись в локальную переменную для запросов на сбой
4. [Коммит 26e0](https://github.com/manticoresoftware/manticoresearch/commit/26e094ab) Небольшая утечка памяти сортировщика в тесте 226
5. [Коммит d2c7](https://github.com/manticoresoftware/manticoresearch/commit/d2c7f86a) Огромная утечка памяти в тесте 226
6. [Коммит 0dd8](https://github.com/manticoresoftware/manticoresearch/commit/0dd80122) Кластер показывает, что узлы синхронизированы, но `count(*)` показывает разные числа
7. [Коммит f1c1](https://github.com/manticoresoftware/manticoresearch/commit/f1c1ac3f) Косметика: Дублирующиеся и иногда потерянные предупреждающие сообщения в журнале
8. [Коммит f1c1](https://github.com/manticoresoftware/manticoresearch/commit/f1c1ac3f) Косметика: (null) имя индекса в журнале
9. [Коммит 359d](https://github.com/manticoresoftware/manticoresearch/commit/359dbd30) Невозможно получить более 70M результатов
10. [Коммит 19f3](https://github.com/manticoresoftware/manticoresearch/commit/19f328ee) Невозможно вставить правила PQ с синтаксисом без колонок
11. [Коммит bf68](https://github.com/manticoresoftware/manticoresearch/commit/bf685d5d) Вводя документ в индекс в кластере, появляется вводящий в заблуждение текст ошибки
12. [Коммит 2cf1](https://github.com/manticoresoftware/manticoresearch/commit/2cf18c83) `/json/replace` и `json/update` возвращают id в экспоненциальной форме
13. [Проблема #324](https://github.com/manticoresoftware/manticoresearch/issues/324) Обновление скалярных свойств json и mva в одном запросе
14. [Коммит d384](https://github.com/manticoresoftware/manticoresearch/commit/d38409eb) `hitless_words` не работает в режиме RT
15. [Коммит 5813](https://github.com/manticoresoftware/manticoresearch/commit/5813d639) `ALTER RECONFIGURE` в режиме rt должен быть запрещен
16. [Коммит 5813](https://github.com/manticoresoftware/manticoresearch/commit/5813d639) `rt_mem_limit` сбрасывается на 128M после перезапуска searchd
17. highlight() иногда зависает
18. [Коммит 7cd8](https://github.com/manticoresoftware/manticoresearch/commit/7cd878f4) Не удалось использовать U+код в режиме RT
19. [Коммит 2b21](https://github.com/manticoresoftware/manticoresearch/commit/2b213de4) Не удалось использовать подстановочный знак в словоформах в режиме RT
20. [Коммит e9d0](https://github.com/manticoresoftware/manticoresearch/commit/e9d07e68) Исправлено `SHOW CREATE TABLE` против нескольких файлов словоформ
21. [Коммит fc90](https://github.com/manticoresoftware/manticoresearch/commit/fc90a84f) JSON-запрос без "query" вызывает сбой searchd
22. Официальный [docker Manticore](https://hub.docker.com/r/manticoresearch/manticore) не мог индексировать из mysql 8
23. [Коммит 23e0](https://github.com/manticoresoftware/manticoresearch/commit/23e05d32) HTTP /json/insert требует id
24. [Коммит bd67](https://github.com/manticoresoftware/manticoresearch/commit/bd679af0) `SHOW CREATE TABLE` не работает для PQ
25. [Коммит bd67](https://github.com/manticoresoftware/manticoresearch/commit/bd679af0) `CREATE TABLE LIKE` не работает должным образом для PQ
26. [Коммит 5eac](https://github.com/manticoresoftware/manticoresearch/commit/5eacf28f) Конец строки в настройках в показе статуса индекса
27. [Коммит cb15](https://github.com/manticoresoftware/manticoresearch/commit/cb153228) Пустой заголовок в "highlight" в HTTP JSON-ответе
28. [Проблема #318](https://github.com/manticoresoftware/manticoresearch/issues/318) Ошибка инфикса `CREATE TABLE LIKE`
29. [Коммит 9040](https://github.com/manticoresoftware/manticoresearch/commit/9040d22c) RT вылетает под нагрузкой
30. [cd512c7d ](https://github.com/manticoresoftware/manticoresearch/commit/cd512c7d) Потерян журнал сбоев при сбое на дисковом куске RT
31. [Проблема #323](https://github.com/manticoresoftware/manticoresearch/issues/323) Ошибка импорта таблицы и закрытие соединения
32. [Коммит 6275](https://github.com/manticoresoftware/manticoresearch/commit/6275316a) `ALTER reconfigure` повреждает индекс PQ
33. [Коммит 9c1d](https://github.com/manticoresoftware/manticoresearch/commit/9c1d221e) Проблемы с перезагрузкой searchd после изменения типа индекса
34. [Коммит 71e2](https://github.com/manticoresoftware/manticoresearch/commit/71e2b5bb) Демон вылетает при импорте таблицы с пропущенными файлами
35. [Проблема #322](https://github.com/manticoresoftware/manticoresearch/issues/322) Сбой при выборе с использованием нескольких индексов, группировки и ranker = none
36. [Коммит c3f5](https://github.com/manticoresoftware/manticoresearch/commit/c3f58490) `HIGHLIGHT()` не выделяет в строковых атрибутах
37. [Проблема #320](https://github.com/manticoresoftware/manticoresearch/issues/320) `FACET` не удается отсортировать по строковому атрибуту
38. [Коммит 4f1a](https://github.com/manticoresoftware/manticoresearch/commit/4f1a1f25) Ошибка в случае отсутствия каталога данных
39. [Коммит 04f4](https://github.com/manticoresoftware/manticoresearch/commit/04f4ddd4) access_* не поддерживаются в режиме RT
40. [Коммит 1c06](https://github.com/manticoresoftware/manticoresearch/commit/1c0616a2) Плохие JSON-объекты в строках: 1. `CALL PQ` возвращает "Плохие JSON-объекты в строках: 1", когда json превышает некоторое значение.
41. [Коммит 32f9](https://github.com/manticoresoftware/manticoresearch/commit/32f943d6) Непоследовательность в режиме RT. В некоторых случаях я не могу удалить индекс, так как он неизвестен, и не могу создать его, так как каталог не пуст.
42. [Проблема #319](https://github.com/manticoresoftware/manticoresearch/issues/319) Сбой при выборе
43. [Коммит 22a2](https://github.com/manticoresoftware/manticoresearch/commit/22a28dd7) `max_xmlpipe2_field` = 2M возвращает предупреждение о поле 2M
44. [Проблема #342](https://github.com/manticoresoftware/manticoresearch/issues/342) Ошибка выполнения условий запроса
45. [Коммит dd8d](https://github.com/manticoresoftware/manticoresearch/commit/dd8dcab2) Простой поиск по 2 терминам находит документ, содержащий только один термин
46. [Коммит 9091](https://github.com/manticoresoftware/manticoresearch/commit/90919e62) В PQ было невозможно сопоставить json с заглавными буквами в ключах
47. [Коммит 56da](https://github.com/manticoresoftware/manticoresearch/commit/56da086a) Индексатор вылетает на csv+docstore
48. [Проблема #363](https://github.com/manticoresoftware/manticoresearch/issues/363) использование `[null]` в json attr в centos 7 вызывает поврежденные вставленные данные
49. Основная [Проблема #345](https://github.com/manticoresoftware/manticoresearch/issues/345) Записи не вставляются, count() случайный, "replace into" возвращает OK
50. max_query_time слишком сильно замедляет SELECT
51. [Проблема #352](https://github.com/manticoresoftware/manticoresearch/issues/352) Связь мастер-агент не работает на Mac OS
52. [Проблема #328](https://github.com/manticoresoftware/manticoresearch/issues/328) Ошибка при подключении к Manticore с Connector.Net/Mysql 8.0.19
53. [Коммит daa7](https://github.com/manticoresoftware/manticoresearch/commit/daa760d2fe83668038318f0f384ceefc0d075e18) Исправлено экранирование \0 и оптимизирована производительность
54. [Коммит 9bc5](https://github.com/manticoresoftware/manticoresearch/commit/9bc5c01a493db993b90ff6c1ad51aeae028f629c) Исправлено count distinct против json
55. [Коммит 4f89](https://github.com/manticoresoftware/manticoresearch/commit/4f89a965a3af0db548ae9d1763731ebaebe21f5a) Исправлено удаление таблицы на другом узле
56. [Коммит 952a](https://github.com/manticoresoftware/manticoresearch/commit/952af5a5931fd72b87edc169588777778b365231) Исправить сбои при плотном выполнении вызова pq


## Версия 3.4.2, 10 апреля 2020
### Критические исправления ошибок
* [Коммит 2ffe](https://github.com/manticoresoftware/manticoresearch/commit/2ffe2d261793f9681265bc4abe38d695e72c121e) исправляет сбой индекса RT из старой версии при индексировании данных

## Версия 3.4.0, 26 марта 2020
### Основные изменения
* сервер работает в 2 режимах: rt-mode и plain-mode
   *   rt-mode требует data_dir и отсутствует определение индекса в конфиге
   *   в plain-mode индексы определяются в конфиге; data_dir не разрешён
* репликация доступна только в rt-mode

### Незначительные изменения
* charset_table по умолчанию соответствует псевдониму non_cjk
* в rt-mode полнотекстовые поля по умолчанию индексируются и сохраняются
* полнотекстовые поля в rt-mode переименованы с 'field' в 'text'
* ALTER RTINDEX переименован в ALTER TABLE
* TRUNCATE RTINDEX переименован в TRUNCATE TABLE

### Функции
* поля только для хранения
* SHOW CREATE TABLE, IMPORT TABLE

### Улучшения
* значительно более быстрая очередь без блокировок (lockless PQ)
* /sql может выполнять любые SQL-запросы в режиме mode=raw
* псевдоним mysql для протокола mysql41
* состояние по умолчанию state.sql в data_dir

### Исправления ошибок
* [Коммит a533](https://github.com/manticoresoftware/manticoresearch/commit/a5333644e8e0c69746a9f24da8221fc965478e4a) исправляет сбой при неверном синтаксисе поля в highlight()
* [Коммит 7fbb](https://github.com/manticoresoftware/manticoresearch/commit/7fbb9f2e618096289cba7f8b834530b46aa94055) исправляет сбой сервера при репликации RT индекса с docstore
* [Коммит 24a0](https://github.com/manticoresoftware/manticoresearch/commit/24a04687eb43b4f4719c7c4a5fe3c19099240467) исправляет сбой при highlight для индекса с опцией infix или prefix и без включённых сохранённых полей
* [Коммит 3465](https://github.com/manticoresoftware/manticoresearch/commit/3465c1ce9c860c09f6ccc5b83a63e3c68c241d8d) исправляет ложную ошибку о пустом docstore и lookup dock-id для пустого индекса
* [Коммит a707](https://github.com/manticoresoftware/manticoresearch/commit/b8601b4144b9092be43a7caa7cd6b19cb37e14c7) исправляет #314 ошибку INSERT с завершающей точкой с запятой
* [Коммит 9562](https://github.com/manticoresoftware/manticoresearch/commit/95628c9b9b8f16fe1bd18599a84e9673575adf1a) удалено предупреждение о несовпадении слов запроса
* [Коммит b860](https://github.com/manticoresoftware/manticoresearch/commit/b8601b4144b9092be43a7caa7cd6b19cb37e14c7) исправлены запросы в сниппетах, сегментированных через ICU
* [Коммит 5275](https://github.com/manticoresoftware/manticoresearch/commit/5275516ca4631c584e6e0127fa161a18d2658fd0) исправлена гонка при поиске/добавлении в кэш блоков docstore
* [Коммит f06e](https://github.com/manticoresoftware/manticoresearch/commit/f06ef97af5c7390427e82525c150535290e42c3e) исправлена утечка памяти в docstore
* [Коммит a725](https://github.com/manticoresoftware/manticoresearch/commit/a7258ba8340e246c0d8aaeb4a7a74129eba807eb) исправляет #316 LAST_INSERT_ID возвращает пустое значение при INSERT
* [Коммит 1ebd](https://github.com/manticoresoftware/manticoresearch/commit/1ebd5bf8d74113bcfa138ce63ce11966990a4215) исправляет #317 HTTP endpoint json/update для поддержки массива для MVA и объекта для JSON атрибута
* [Коммит e426](https://github.com/manticoresoftware/manticoresearch/commit/e426950a536382a142e2a42ed207c341149ff685) исправлена проблема с индексатором, сбрасывающим rt без явного id

## Версия 3.3.0, 4 февраля 2020
### Функции
* Параллельный поиск по Real-Time индексам
* Команда EXPLAIN QUERY
* конфигурационный файл без определений индексов (альфа-версия)
* команды CREATE/DROP TABLE (альфа-версия)
* indexer --print-rt - может читать из источника и выводить INSERT для Real-Time индекса

### Улучшения
* Обновлены стеммеры Snowball до версии 2.0
* LIKE фильтр для SHOW INDEX STATUS
* улучшено использование памяти при больших max_matches
* SHOW INDEX STATUS добавляет ram_chunks_count для RT индексов
* очередь без блокировок (lockless PQ)
* изменено LimitNOFILE на 65536


### Исправления ошибок
* [Коммит 9c33](https://github.com/manticoresoftware/manticoresearch/commit/9c33aab8c1fd9059c8644d23d5f58f03010043c6) добавлена проверка схемы индекса на наличие дублирующихся атрибутов #293
* [Коммит a008](https://github.com/manticoresoftware/manticoresearch/commit/a0085f9477e191da9e0e835b91583fb9cfde698e) исправлен сбой в hitless terms
* [Коммит 6895](https://github.com/manticoresoftware/manticoresearch/commit/68953740af8dca1833026d0f448c71416ed3044a) исправлена потеря docstore после ATTACH
* [Коммит d6f6](https://github.com/manticoresoftware/manticoresearch/commit/d6f696edea53ea2b9a68251e4054e4816082c285) исправлена проблема с docstore в распределенной конфигурации
* [Коммит bce2](https://github.com/manticoresoftware/manticoresearch/commit/bce2b7ec883d208f31094ee42a5b6f01d18a998f) заменен FixedHash на OpenHash в сортировщике
* [Коммит e0ba](https://github.com/manticoresoftware/manticoresearch/commit/e0baf7392937969e640fb5d10e8ccb312fe36446) исправлены атрибуты с дублирующимися именами при определении индекса
* [Коммит ca81](https://github.com/manticoresoftware/manticoresearch/commit/ca81114bf1ac7ea7adcc8509bdc14d1f30b4f461) исправлен html_strip в HIGHLIGHT()
* [Коммит 493a](https://github.com/manticoresoftware/manticoresearch/commit/493a5e916d70fd4a3f14d1ee487234434f4a7ce3) исправлен макрос passage в HIGHLIGHT()
* [Коммит a82d](https://github.com/manticoresoftware/manticoresearch/commit/a82d41c775692f837f267d554d7d4a4b499d69f0) исправлены проблемы с двойным буфером при создании RT-индексом малого или большого дискового чанка
* [Коммит a404](https://github.com/manticoresoftware/manticoresearch/commit/a404c85de0d41d9d1c75b7540479d8a3af9e61aa) исправлено удаление событий для kqueue
* [Коммит 8bea](https://github.com/manticoresoftware/manticoresearch/commit/8bea0f6fd1666b817995c4dd6e67f6f1c9161638) исправлено сохранение дискового чанка для большого значения rt_mem_limit у RT-индекса
* [Коммит 8707](https://github.com/manticoresoftware/manticoresearch/commit/8707f0395b7ace52dc0cba9c8ed92a2bc4392932) исправлено переполнение float при индексации
* [Коммит a564](https://github.com/manticoresoftware/manticoresearch/commit/a56434ce71bb7714b4d8e75278aaa075b27ece80) исправлено: вставка документа с отрицательным ID в RT-индекс теперь завершается ошибкой
* [Коммит bbeb](https://github.com/manticoresoftware/manticoresearch/commit/bbebfd7551a68641154761cac962053b54806759) исправлен сбой сервера на ranker fieldmask
* [Коммит 3809](https://github.com/manticoresoftware/manticoresearch/commit/3809cc1b0863e3a5a200dc423be3f7c38be56113) исправлен сбой при использовании кэша запросов
* [Коммит dc2a](https://github.com/manticoresoftware/manticoresearch/commit/dc2a585bc724678679e4297362200bbd49ce32bb) исправлен сбой при использовании RAM-сегментов RT-индекса с параллельными вставками

## Версия 3.2.2, 19 декабря 2019
### Возможности
* Автоинкрементный ID для RT-индексов
* Поддержка выделения для docstore через новую функцию HIGHLIGHT(), также доступную в HTTP API
* SNIPPET() может использовать специальную функцию QUERY(), которая возвращает текущий запрос MATCH
* Новая опция field_separator для функций выделения.

### Улучшения и изменения
* Ленивая загрузка хранимых полей для удаленных узлов (может значительно повысить производительность)
* Строки и выражения больше не нарушают оптимизации multi-query и FACET
* Сборка для RHEL/CentOS 8 теперь использует библиотеку mysql libclient из mariadb-connector-c-devel
* Файл данных ICU теперь поставляется с пакетами, параметр icu_data_dir удален
* Файлы службы systemd включают политику 'Restart=on-failure'
* Инструмент indextool теперь может проверять индексы реального времени в режиме онлайн
* Конфигурация по умолчанию теперь /etc/manticoresearch/manticore.conf
* Служба на RHEL/CentOS переименована в 'manticore' с 'searchd'
* Удалены опции сниппета query_mode и exact_phrase

### Исправления ошибок
* [Commit 6ae4](https://github.com/manticoresoftware/manticoresearch/commit/6ae474c7894a6bee222d5b18e59a44fdbf57843a) исправление сбоя при выполнении SELECT запроса через HTTP интерфейс
* [Commit 5957](https://github.com/manticoresoftware/manticoresearch/commit/59577513a49eac5a4a3c5e2cb38394d3246b5d35) исправление RT индекса, который сохраняет дисковые чанки, но не помечает некоторые документы как удаленные
* [Commit e861](https://github.com/manticoresoftware/manticoresearch/commit/e861f0fca0e88924450695d4e9d6acff7a36558a) исправление сбоя при поиске по многим индексам или многим запросам с dist_threads
* [Commit 4409](https://github.com/manticoresoftware/manticoresearch/commit/440991fc977b8479800b45cdbc862eeb1ba5d965) исправление сбоя при генерации инфиксов для длинных терминов с широкими utf8 кодовыми точками
* [Commit 5fd5](https://github.com/manticoresoftware/manticoresearch/commit/5fd599b48bba527c023e6aa0b262dca51ffb8a1c) исправление гонки при добавлении сокета в IOCP
* [Commit cf10](https://github.com/manticoresoftware/manticoresearch/commit/cf10d7d3589f77adfaefbe52a51777bc9c67cf99) исправление проблемы с булевыми запросами против json списка выбора
* [Commit 996d](https://github.com/manticoresoftware/manticoresearch/commit/996de77f4c3ec103d965e28c36098fd07dba50c4) исправление проверки indextool для сообщения о неправильном смещении skiplist, проверка doc2row поиска
* [Commit 6e3f](https://github.com/manticoresoftware/manticoresearch/commit/6e3fc9e88941c9427410f9d0adeebb7dd09a900f) исправление индексации, которая создает плохой индекс с отрицательным смещением skiplist на больших данных
* [Commit faed](https://github.com/manticoresoftware/manticoresearch/commit/faed3220b423a96401982bf47117edf1f62c584e) исправление JSON, который конвертирует только числовые значения в строку и конвертацию JSON строки в числовое значение в выражениях
* [Commit 5331](https://github.com/manticoresoftware/manticoresearch/commit/533197200249d648ebcbdaca5d605861ee566348) исправление выхода indextool с кодом ошибки в случае установки нескольких команд в командной строке
* [Commit 7955](https://github.com/manticoresoftware/manticoresearch/commit/795520ac351d22e4497ebaf6c249bf25081842b6) исправление #275 binlog недопустимое состояние при ошибке недостаточно места на диске
* [Commit 2284](https://github.com/manticoresoftware/manticoresearch/commit/2284da5e3ff77bf7be4038a4d950c0580e4159c8) исправление #279 сбоя при фильтре IN для JSON атрибута
* [Commit ce2e](https://github.com/manticoresoftware/manticoresearch/commit/ce2e4b4765111a7d331ffc256911f12770a8942d) исправление #281 неправильного вызова закрытия канала
* [Commit 5355](https://github.com/manticoresoftware/manticoresearch/commit/535589ba8a37baf5c6056afb5ff969fd70476feb) исправление зависания сервера при вызове CALL PQ с рекурсивным JSON атрибутом, закодированным как строка
* [Commit a5fc](https://github.com/manticoresoftware/manticoresearch/commit/a5fc8a36e1644ee051632408ab8b53e79aeb9ff4) исправление продвижения за пределы конца списка документов в узле multiand
* [Commit a362](https://github.com/manticoresoftware/manticoresearch/commit/a3628617ef4ee89a811d4ba2c4a04cbbc298c1b5) исправление получения публичной информации о потоке
* [Commit f8d2](https://github.com/manticoresoftware/manticoresearch/commit/f8d2d7bbbb90af3d2ee97424f97b05927f09d42b) исправление блокировок кэша docstore

## Версия 3.2.0, 17 октября 2019
### Возможности
* Хранение документов
* новые директивы stored_fields, docstore_cache_size, docstore_block_size, docstore_compression, docstore_compression_level

### Улучшения и изменения
* улучшенная поддержка SSL
* обновленный встроенный набор символов non_cjk
* отключено ведение журнала операторов UPDATE/DELETE при выполнении SELECT в журнале запросов
* пакеты RHEL/CentOS 8

### Исправления ошибок
* [Commit 301a](https://github.com/manticoresoftware/manticoresearch/commit/301a806b14725e842300ee225521407464374c41) исправление сбоя при замене документа в дисковом чанке RT индекса
* [Commit 46c1](https://github.com/manticoresoftware/manticoresearch/commit/46c1cad8fa11c1fc09cc34b20d978fdfcd4e6d27) исправление \#269 LIMIT N OFFSET M
* [Commit 92a4](https://github.com/manticoresoftware/manticoresearch/commit/92a46edaa196ef33f3ac2225fbdba2c2b88ee45d) исправление операторов DELETE с явно установленным id или списком id, предоставленным для пропуска поиска
* [Commit 8ca7](https://github.com/manticoresoftware/manticoresearch/commit/8ca78c138d837caab30dc6e2343a3c4d1687bb87) исправление неправильного индекса после удаления события в netloop в poller windowspoll
* [Commit 6036](https://github.com/manticoresoftware/manticoresearch/commit/603631e2bff5cb0acf25f68b85621742490eccd9) исправление округления float в JSON через HTTP
* [Commit 62f6](https://github.com/manticoresoftware/manticoresearch/commit/62f64cb9e75ec28fc3c29f8414ce9656f5580254) исправление удаленных фрагментов для проверки пустого пути в первую очередь; исправление тестов для windows
* [Commit aba2](https://github.com/manticoresoftware/manticoresearch/commit/aba274c2c5abc1029894ba9f95b786bda93e8f22) исправление перезагрузки конфигурации для работы в windows так же, как и в linux
* [Commit 6b8c](https://github.com/manticoresoftware/manticoresearch/commit/6b8c4242ef77c3fa4d0ccb7d76d81714b6728f0b) исправление \#194 PQ для работы с морфологией и стеммерами
* [Commit 174d](https://github.com/manticoresoftware/manticoresearch/commit/174d312905285aa5746f4f404c53e6dc669ef42f) исправление управления списками сегментов RT

## Версия 3.1.2, 22 августа 2019
### Возможности и улучшения
* Экспериментальная поддержка SSL для HTTP API
* фильтр полей для CALL KEYWORDS
* max_matches для /json/search
* автоматическое определение размера по умолчанию для Galera gcache.size
* улучшенная поддержка FreeBSD

### Исправления ошибок
* [Коммит 0a1a](https://github.com/manticoresoftware/manticoresearch/commit/0a1a2c81038794983b4e30c8aefc7cc20e89b03f) исправлена репликация RT-индекса на узел, где такой же RT-индекс уже существует и имеет другой путь
* [Коммит 4adc](https://github.com/manticoresoftware/manticoresearch/commit/4adc075294ac823289f745e2cc419f18c7dcf2e2) исправлено перепланирование сброса на диск для индексов без активности
* [Коммит d6c0](https://github.com/manticoresoftware/manticoresearch/commit/d6c00a6f750c04021747e9c99e0f77707a4cbfc3) улучшено перепланирование сброса на диск для RT/PQ индексов
* [Коммит d0a7](https://github.com/manticoresoftware/manticoresearch/commit/d0a7c95906b9b5ba33c98267d54501cfe27508d6) исправлена опция индекса index_field_lengths для TSV и CSV piped источников (#250)
* [Коммит 1266](https://github.com/manticoresoftware/manticoresearch/commit/1266d54800242709fbe2c34e72eb69b6595b23bd) исправлен некорректный отчет indextool при проверке блочного индекса на пустом индексе
* [Коммит 553c](https://github.com/manticoresoftware/manticoresearch/commit/553ca73ca111f28e0888d59cd48b0d6df181ecd6) исправлен пустой список выбора в логе SQL-запросов Manticore
* [Коммит 56c8](https://github.com/manticoresoftware/manticoresearch/commit/56c8584456293665ddd4ce4c94307d74473a78b9) исправлен ответ indexer на -h/--help

## Версия 3.1.0, 16 июля 2019
### Возможности и улучшения
* репликация для RealTime-индексов
* ICU-токенизатор для китайского языка
* новая опция морфологии icu_chinese
* новая директива icu_data_dir
* транзакции с несколькими операторами для репликации
* LAST_INSERT_ID() и @session.last_insert_id
* LIKE 'шаблон' для SHOW VARIABLES
* Вставка нескольких документов для перколяционных индексов
* Добавлены парсеры времени для конфигурации
* внутренний менеджер задач
* mlock для компонентов списков документов и хитов
* jail snippets path

### Удаления
* Поддержка библиотеки RLP удалена в пользу ICU; все директивы rlp* удалены
* Обновление ID документа с помощью UPDATE отключено

### Исправления ошибок
* [Коммит f047](https://github.com/manticoresoftware/manticoresearch/commit/f047222339fd0e62a07e9ca8b2a829297d5db861) исправлены дефекты в concat и group_concat
* [Коммит b081](https://github.com/manticoresoftware/manticoresearch/commit/b08147ee24712b5eaa51f261626b5d16c6242fdd) исправлен тип атрибута query uid в перколяционном индексе на BIGINT
* [Коммит 4cd8](https://github.com/manticoresoftware/manticoresearch/commit/4cd85afaf76eef0ce9fa1cd073f3fed160279890) не падать, если не удалось предварительно выделить новый дисковой чанк
* [Коммит 1a55](https://github.com/manticoresoftware/manticoresearch/commit/1a551227886474054111088412f2b2396f485652) добавлен отсутствующий тип данных timestamp в ALTER
* [Коммит f3a8](https://github.com/manticoresoftware/manticoresearch/commit/f3a8e096fd496a6b001d398aadbb781b2dbad929) исправлен краш из-за неправильного чтения mmap
* [Коммит 4475](https://github.com/manticoresoftware/manticoresearch/commit/447577115192db7ac7bfea70b5ba58ee2c61229f) исправлен хэш блокировки кластеров в репликации
* [Коммит ff47](https://github.com/manticoresoftware/manticoresearch/commit/ff476df955f057bd38e8b6ad6056325e9d6fbb9c) исправлена утечка провайдеров в репликации
* [Коммит 58dc](https://github.com/manticoresoftware/manticoresearch/commit/58dcbb779c4c30ca1560460c78e35ba114b83716) исправлена #246 неопределенная sigmask в indexer
* [Коммит 3dd8](https://github.com/manticoresoftware/manticoresearch/commit/3dd8278e747d534f69fe9c3d360eef188ab7b6d5) исправлена гонка в отчетности netloop
* [Коммит a02a](https://github.com/manticoresoftware/manticoresearch/commit/a02aae05a3fa3d1caf2821f43c7e237b6de76336) нулевой зазор для балансировщика стратегий HA

## Версия 3.0.2, 31 мая 2019
### Улучшения
* добавлены mmap-ридеры для документов и списков хитов
* ответ конечной точки HTTP `/sql` теперь такой же, как ответ `/json/search`
* новые директивы `access_plain_attrs`, `access_blob_attrs`, `access_doclists`, `access_hitlists`
* новая директива `server_id` для настроек репликации

### Удаления
* удалена конечная точка HTTP `/search`

### Устаревания
* `read_buffer`, `ondisk_attrs`, `ondisk_attrs_default`, `mlock` заменены директивами `access_*`

### Исправления ошибок
* [Коммит 849c](https://github.com/manticoresoftware/manticoresearch/commit/849c16e1040f382795ba0635cb48686f9db4f2d8) разрешены имена атрибутов, начинающиеся с цифр, в списке выбора
* [Коммит 48e6](https://github.com/manticoresoftware/manticoresearch/commit/48e6c302ca37bec0bc49c79619c60bf787ed0708) исправлены MVA в UDF, исправлен алиасинг MVA
* [Коммит 0555](https://github.com/manticoresoftware/manticoresearch/commit/055586a9a2ebfa8874f68099c5bc798a145fd4ef) исправлен #187 краш при использовании запроса с SENTENCE
* [Коммит 93bf](https://github.com/manticoresoftware/manticoresearch/commit/93bf52f23f1c53c8a93a603387ca5463e1fc2ae8) исправлена #143 поддержка () вокруг MATCH()
* [Коммит 599e](https://github.com/manticoresoftware/manticoresearch/commit/599ee79ccd38cce0023a55cbce8f50c3dca62e38) исправлено сохранение состояния кластера при выполнении ALTER cluster
* [Коммит 230c](https://github.com/manticoresoftware/manticoresearch/commit/230c321e277514b191629538eb3f02d219113d95) исправлен краш сервера при ALTER index с blob-атрибутами
* [Коммит 5802](https://github.com/manticoresoftware/manticoresearch/commit/5802b85aa93a81caea77073a3381ef912fdd677c) исправлена #196 фильтрация по id
* [Коммит 25d2](https://github.com/manticoresoftware/manticoresearch/commit/25d2dabda4a299a29bd375405862126da9b4bfac) отменен поиск по шаблонным индексам
* [Коммит 2a30](https://github.com/manticoresoftware/manticoresearch/commit/2a30d5b49d76faab031d195f533974938769b8e6) исправлен столбец id для соответствия обычному типу bigint в SQL-ответе


## Версия 3.0.0, 6 мая 2019
### Возможности и улучшения
* Новое хранилище индексов. Немасштабируемые атрибуты больше не ограничены размером 4 ГБ на индекс
* Директива attr_update_reserve
* Строковые, JSON и MVA атрибуты могут быть обновлены с помощью UPDATE
* Списки удаления применяются во время загрузки индекса
* Директива killlist_target
* Ускорение многократных AND-поисков
* Улучшена средняя производительность и использование оперативной памяти
* Инструмент convert для обновления индексов, созданных в версии 2.x
* Функция CONCAT()
* JOIN CLUSTER cluster AT 'nodeaddress:port'
* ALTER CLUSTER posts UPDATE nodes
* Директива node_address
* Список узлов выводится в SHOW STATUS

### Изменения в поведении
* в случае индексов с killlists сервер не вращает индексы в порядке, определённом в конфиге, а следует цепочке целей killlist
* порядок индексов в поиске больше не определяет порядок применения killlists
* Идентификаторы документов теперь подписанные большие целые числа

### Удалённые директивы
* docinfo (теперь всегда extern), inplace_docinfo_gap, mva_updates_pool

## Версия 2.8.2 GA, 2 апреля 2019
### Новые возможности и улучшения
* Galera-репликация для percolate-индексов
* OPTION morphology

### Примечания по компиляции
Минимальная версия Cmake теперь 3.13. Для компиляции требуются библиотеки boost и libssl
для разработки.

### Исправления ошибок
* [Коммит 6967](https://github.com/manticoresoftware/manticoresearch/commit/6967fedb2ef818ec1c825d482563edd05e1c9245) исправлен сбой при множественных звёздочках в списке select для запроса к множеству распределённых индексов
* [Коммит 36df](https://github.com/manticoresoftware/manticoresearch/commit/36df1a407dc08263690e3492518613ace82d69ca) исправлен [\#177](https://github.com/manticoresoftware/manticoresearch/issues/177) — большой пакет через Manticore SQL интерфейс
* [Коммит 5793](https://github.com/manticoresoftware/manticoresearch/commit/57932aec734583fa93359faaf5034b2e2c9d352b) исправлен [\#170](https://github.com/manticoresoftware/manticoresearch/issues/170) сбой сервера при RT optimize с обновлённым MVA
* [Коммит edb2](https://github.com/manticoresoftware/manticoresearch/commit/edb24b870423add86eba471d361e0e5aff098b18) исправлен сбой сервера при удалении binlog вследствие удаления RT индекса после перезагрузки конфига по SIGHUP
* [Коммит bd3e](https://github.com/manticoresoftware/manticoresearch/commit/bd3e66e0085bc7e2e351b817dfc972fd8158fbce) исправлены payloads аутентификационного плагина handshake MySQL
* [Коммит 6a21](https://github.com/manticoresoftware/manticoresearch/commit/6a217f6ed82fb10f752213b15617b9cedf1e8533) исправлен [\#172](https://github.com/manticoresoftware/manticoresearch/issues/172) — настройки phrase_boundary для RT индекса
* [Коммит 3562](https://github.com/manticoresoftware/manticoresearch/commit/3562f652753e4091fd7b5b0f65b58341c8cbcb31) исправлен [\#168](https://github.com/manticoresoftware/manticoresearch/issues/168) — дедлок при ATTACH индекса к самому себе
* [Коммит 250b](https://github.com/manticoresoftware/manticoresearch/commit/250b3f0e74ad18dda34ba080aa13ca87798a9ac9) исправлено: binlog сохраняет пустую метаинформацию после сбоя сервера
* [Коммит 4aa6](https://github.com/manticoresoftware/manticoresearch/commit/4aa6c69ad3a181cd7c5dafb990fb528121f68e59) исправлен сбой сервера из-за строки на сортировщике из RT индекса с дисковыми чанками

## Версия 2.8.1 GA, 6 марта 2019
### Новые возможности и улучшения
* SUBSTRING_INDEX()
* Поддержка SENTENCE и PARAGRAPH для percolate-запросов
* systemd-генератор для Debian/Ubuntu; также добавлен LimitCORE для разрешения core dump

### Исправления ошибок
* [Коммит 84fe](https://github.com/manticoresoftware/manticoresearch/commit/84fe7405d17b59f4dcde15a6c8e8503923503def) исправлен сбой сервера в режиме совпадения all и пустом полномтекстовом запросе
* [Коммит daa8](https://github.com/manticoresoftware/manticoresearch/commit/daa88b579fa373d5d2e869e43a2e178363b0fef1) исправлен сбой при удалении статической строки
* [Коммит 2207](https://github.com/manticoresoftware/manticoresearch/commit/22078537dddbd9ce81a182dcc879f235b6e87004) исправлен код выхода при неудаче indextool с FATAL
* [Коммит 0721](https://github.com/manticoresoftware/manticoresearch/commit/0721696d6780c200c65d596624a8187983fb7fcb) исправлен [\#109](https://github.com/manticoresoftware/manticoresearch/issues/109) — отсутствие совпадений для префиксов из-за некорректной проверки exact form
* [Коммит 8af8](https://github.com/manticoresoftware/manticoresearch/commit/8af810111b8e2f87bc9f378172eff1ab9725c7e7) исправлен [\#161](https://github.com/manticoresoftware/manticoresearch/issues/161) перезагрузка настроек конфига для RT индексов
* [Коммит e2d5](https://github.com/manticoresoftware/manticoresearch/commit/e2d592773f4fd702ace90070a793a842a0fec6ab) исправлен сбой сервера при доступе к большой JSON строке
* [Коммит 75cd](https://github.com/manticoresoftware/manticoresearch/commit/75cd1342d05cdc352e3dd145cdbc6f79394a165b) исправлено: поле PQ в JSON-документе, изменённом stripper’ом индекса, вызывает ошибочное совпадение с соседним полем
* [Коммит e2f7](https://github.com/manticoresoftware/manticoresearch/commit/e2f775437df922674d772ed4417780492502e65a) исправлен сбой сервера при разборе JSON на сборках RHEL7
* [Коммит 3a25](https://github.com/manticoresoftware/manticoresearch/commit/3a25a5808feb3f8b80866991b436d6c1241618c2) исправлен сбой функции unescaping json при символе слэша на границе
* [Коммит be9f](https://github.com/manticoresoftware/manticoresearch/commit/be9f497872bea62dbdccd64fc28294fd7776c289) исправлена опция 'skip_empty', чтобы пропускать пустые документы и не предупреждать, что они невалидный json
* [Коммит 266e](https://github.com/manticoresoftware/manticoresearch/commit/266e0e7b088549722a805fc837bf101ff681a5e8) исправлен [\#140](https://github.com/manticoresoftware/manticoresearch/issues/161) — вывод 8 знаков на float, если 6 недостаточно для точности
* [Коммит 3f6d](https://github.com/manticoresoftware/manticoresearch/commit/3f6d23891064ee8e94030c4231497cdd7da33a6a) исправлено создание пустых jsonobj
* [Коммит f3c7](https://github.com/manticoresoftware/manticoresearch/commit/f3c7848a59b7eedc67c3403d6d4b37ce6fa94dc8) исправлен [\#160](https://github.com/manticoresoftware/manticoresearch/issues/161) — пустой mva возвращает NULL вместо пустой строки
* [Коммит 0afa](https://github.com/manticoresoftware/manticoresearch/commit/0afa2ed058e5759470b1d7354c722faab34f98bb) исправлена ошибка сборки без pthread_getname_np
* [Коммит 9405](https://github.com/manticoresoftware/manticoresearch/commit/9405fccdeb27f0302a8d9a848981d30080216777) исправлен сбой при завершении работы сервера с thread_pool workers

## Версия 2.8.0 GA, 28 января 2019
### Улучшения
* Распределенные индексы для перколяторных индексов
* CALL PQ новые опции и изменения:
    *   skip_bad_json
    *   mode (sparsed/sharded)
    *   json документы могут передаваться как json массив
    *   shift
    *   Имена столбцов 'UID', 'Documents', 'Query', 'Tags', 'Filters' были переименованы в 'id', 'documents', 'query', 'tags', 'filters'
* DESCRIBE pq TABLE
* SELECT FROM pq, где UID больше не возможен, используйте 'id' вместо
* SELECT по pq индексам на уровне с обычными индексами (например, вы можете фильтровать правила через REGEX())
* ANY/ALL могут использоваться на тегах PQ
* выражения имеют автоматическое преобразование для JSON полей, не требуя явного приведения
* встроенная 'non_cjk' charset_table и 'cjk' ngram_chars
* встроенные коллекции стоп-слов для 50 языков
* несколько файлов в объявлении стоп-слов также могут быть разделены запятой
* CALL PQ может принимать JSON массив документов

### Исправления ошибок
* [Коммит a4e1](https://github.com/manticoresoftware/manticoresearch/commit/a4e19afee54dafdc04b0da53372dafd8c0d332d6) исправил утечку, связанную с csjon
* [Коммит 28d8](https://github.com/manticoresoftware/manticoresearch/commit/28d862774874891a03e361da1b0347ebe8066ce0) исправил сбой из-за отсутствующего значения в json
* [Коммит bf4e](https://github.com/manticoresoftware/manticoresearch/commit/bf4e9ea27c349cdc9ae4e54e960a4a17b90e38fa) исправил сохранение пустой мета для RT индекса
* [Коммит 33b4](https://github.com/manticoresoftware/manticoresearch/commit/33b4573529e7c7c7bce19bf9d54054866f30d99a) исправил потерянный флаг формы (точный) для последовательности лемматизатора
* [Коммит 6b95](https://github.com/manticoresoftware/manticoresearch/commit/6b95d48240d3b5520afade19c249d79e020a5e88) исправил строковые атрибуты > 4M использовать насыщение вместо переполнения
* [Коммит 6214](https://github.com/manticoresoftware/manticoresearch/commit/621418b829e70af36aaa322a2f51ece3f86bc499) исправил сбой сервера на SIGHUP с отключенным индексом
* [Коммит 3f7e](https://github.com/manticoresoftware/manticoresearch/commit/3f7e35d1482966cc45d5abbcb2de5de3508d66b0) исправил сбой сервера на одновременных командах статуса API сессии
* [Коммит cd9e](https://github.com/manticoresoftware/manticoresearch/commit/cd9e4f1709a48ddafc128c450c2d882bc11ba50e) исправил сбой сервера при удалении запроса к RT индексу с фильтрами полей
* [Коммит 9376](https://github.com/manticoresoftware/manticoresearch/commit/9376470d455fcd256c6abff79c431a6919dfa7ac) исправил сбой сервера при CALL PQ к распределенному индексу с пустым документом
* [Коммит 8868](https://github.com/manticoresoftware/manticoresearch/commit/8868b207644f4f8cc1ab0c270adb35493fed22e1) исправил обрезку сообщения об ошибке Manticore SQL больше 512 символов
* [Коммит de9d](https://github.com/manticoresoftware/manticoresearch/commit/de9deda9c142823d6dbf529423417b5c670fae94) исправил сбой при сохранении перколяторного индекса без binlog
* [Коммит 2b21](https://github.com/manticoresoftware/manticoresearch/commit/2b219e1a32791e7740bb210b7d408f96abc6374f) исправил работу http интерфейса в OSX
* [Коммит e92c](https://github.com/manticoresoftware/manticoresearch/commit/e92c60240f8f9e1756bfa14fba0bbb402586a389) исправил ложное сообщение об ошибке indextool при проверке MVA
* [Коммит 238b](https://github.com/manticoresoftware/manticoresearch/commit/238bdea59bad89f097403f1c978658ce45f16c70) исправил блокировку записи при FLUSH RTINDEX, чтобы не блокировать весь индекс во время сохранения и при обычном сбросе из rt_flush_period
* [Коммит c26a](https://github.com/manticoresoftware/manticoresearch/commit/c26a236bd42b7082db079a51a84172066d337d64) исправил зависание ALTER перколяторного индекса в ожидании загрузки поиска
* [Коммит 9ee5](https://github.com/manticoresoftware/manticoresearch/commit/9ee5703f6c28f57cd5bff5e705966d93fe30d267) исправил max_children, чтобы использовать стандартное количество потоков thread_pool для значения 0
* [Коммит 5138](https://github.com/manticoresoftware/manticoresearch/commit/5138fc048a1bd146b271ce6a72fe954344281f69) исправил ошибку при индексации данных в индекс с плагином index_token_filter вместе со стоп-словами и stopword_step=0
* [Коммит 2add](https://github.com/manticoresoftware/manticoresearch/commit/2add3d319ac62fe450bf60e89033100853dc2ecf) исправил сбой при отсутствии lemmatizer_base при использовании aot лемматизаторов в определениях индекса

## Версия 2.7.5 GA, 4 декабря 2018
### Улучшения
* Функция REGEX
* limit/offset для json API поиска
* точки профилирования для qcache

### Исправления ошибок
* [Коммит eb3c](https://github.com/manticoresoftware/manticoresearch/commit/eb3c7683b9740cff4cc4cadd2ab9f990b9f7d173) исправил сбой сервера на FACET с несколькими атрибутами широких типов
* [Коммит d915](https://github.com/manticoresoftware/manticoresearch/commit/d915cf6696514f4be365ffb5981bccaa15983e65) исправил неявную группировку по основному списку выбора FACET запроса
* [Коммит 5c25](https://github.com/manticoresoftware/manticoresearch/commit/5c25dc271a8c11a1c82ca3c88e518acbc6a8c164) исправил сбой на запросе с GROUP N BY
* [Коммит 85d3](https://github.com/manticoresoftware/manticoresearch/commit/85d30a232d4ffd89916b673a9b2db946d272a945) исправил взаимную блокировку при обработке сбоя в операциях с памятью
* [Коммит 8516](https://github.com/manticoresoftware/manticoresearch/commit/85166b5ef2be38061d62f2b20f15acaa0311258a) исправил потребление памяти indextool во время проверки
* [Коммит 58fb](https://github.com/manticoresoftware/manticoresearch/commit/58fb03166830d8b23e7ab9ce56309db7ddac22f4) исправил включение gmock, которое больше не нужно, так как upstream решает это самостоятельно

## Версия 2.7.4 GA, 1 ноября 2018
### Улучшения
* SHOW THREADS в случае удаленных распределенных индексов выводит оригинальный запрос вместо вызова API
* SHOW THREADS новая опция `format=sphinxql` выводит все запросы в SQL формате
* SHOW PROFILE выводит дополнительный этап `clone_attrs`

### Исправления ошибок
* [Коммит 4f15](https://github.com/manticoresoftware/manticoresearch/commit/4f155712a0bccc1bd01cc191647bc8cff814888e) исправлена ошибка сборки с libc без malloc_stats, malloc_trim
* [Коммит f974](https://github.com/manticoresoftware/manticoresearch/commit/f974f20bda3214a56877c393a192be1a77150958) исправлена обработка специальных символов внутри слов для результата CALL KEYWORDS
* [Коммит 0920](https://github.com/manticoresoftware/manticoresearch/commit/092083282ea8ae0b2e72fb6989c57ccec81e74ac) исправлена поломка CALL KEYWORDS для распределённого индекса через API или удалённого агента
* [Коммит fd68](https://github.com/manticoresoftware/manticoresearch/commit/fd686bfe88b720ffd7642e36897ba45161cbd7d2) исправлена передача agent_query_timeout распределённого индекса агентам как max_query_time
* [Коммит 4ffa](https://github.com/manticoresoftware/manticoresearch/commit/4ffa623e9d357a3b0b441615089e211f92f8de32) исправлен счётчик общего числа документов в дисковом чанке, который отключался командой OPTIMIZE и ломал расчёт веса
* [Коммит dcaf](https://github.com/manticoresoftware/manticoresearch/commit/dcaf4e0e3c8e9b0fbec47bbf3307feddec0936a6) исправлены множественные tail hits в RT индексе из blended
* [Коммит eee3](https://github.com/manticoresoftware/manticoresearch/commit/eee381754e902a43a8f499e1c950198a2a3e6ee0) исправлен deadlock при ротации

## Версия 2.7.3 GA, 26 сентября 2018
### Улучшения
* опция sort_mode для CALL KEYWORDS
* DEBUG на VIP подключении может выполнять 'crash <password>' для намеренного вызова SIGEGV на сервере
* DEBUG может выполнять 'malloc_stats' для дампа статистики malloc в searchd.log и 'malloc_trim' для вызова malloc_trim()
* улучшен backtrace при наличии gdb в системе

### Исправления ошибок
* [Commit 0f3c](https://github.com/manticoresoftware/manticoresearch/commit/0f3cc333f6129451ad1ae2c97905b6c04ba5d454) исправлен сбой или ошибка rename в Windows
* [Commit 1455](https://github.com/manticoresoftware/manticoresearch/commit/1455ba2bf8079f03ce7f861a8d9662a360d705de) исправлены сбои сервера на 32-битных системах
* [Commit ad37](https://github.com/manticoresoftware/manticoresearch/commit/ad3710d53b5cd6a28b8c60352d0f9e285b03d9fa) исправлен сбой или зависание сервера при пустом выражении SNIPPET
* [Commit b36d](https://github.com/manticoresoftware/manticoresearch/commit/b36d79214364500252665860f000ae85343528b6) исправлена сломанная непрогрессивная оптимизация и исправлена прогрессивная оптимизация, чтобы не создавать kill-list для самого старого chunk на диске
* [Commit 34b0](https://github.com/manticoresoftware/manticoresearch/commit/34b032499afd42ce47a4c7247814b4031094388a) исправлен неправильный ответ queue_max_length для SQL и API в режиме рабочего потока пула потоков
* [Commit ae4b](https://github.com/manticoresoftware/manticoresearch/commit/ae4b3202cbdb8014cabe2b90e269d5cf74f49871) исправлен сбой при добавлении запроса полного сканирования в индекс PQ с установленными опциями regexp или rlp
* [Commit f80f](https://github.com/manticoresoftware/manticoresearch/commit/f80f8d5d7560187078868aed9a9575f4549e98aa) исправлен сбой при вызове одного PQ за другим
* [Commit 9742](https://github.com/manticoresoftware/manticoresearch/commit/9742f5f0866af73f8cd8483ecd18a507ea80dd65) рефакторинг AcquireAccum
* [Commit 39e5](https://github.com/manticoresoftware/manticoresearch/commit/39e5bc3751b7295222eb76407c5d72ce1dad545b) исправлена утечка памяти после вызова pq
* [Commit 21bc](https://github.com/manticoresoftware/manticoresearch/commit/21bcc6d17395f0f57dde79f6716ef303b7ea527d) косметический рефакторинг (c++11 стиль c-трушек, значения по умолчанию, nullptr)
* [Commit 2d69](https://github.com/manticoresoftware/manticoresearch/commit/2d690398f14c736956cfdd66feb4d3091d6b3a4d) исправлена утечка памяти при попытке вставить дубликат в индекс PQ
* [Commit 5ed9](https://github.com/manticoresoftware/manticoresearch/commit/5ed92c4b66da6423fa28f11b1fd59103ed1e1fb9) исправлен сбой при использовании JSON поля IN с большими значениями
* [Commit 4a52](https://github.com/manticoresoftware/manticoresearch/commit/4a5262e2776aa261e34ba5c914ea60804f59de3f) исправлен сбой сервера при выполнении оператора CALL KEYWORDS для RT индекса с установленным ограничением расширения
* [Commit 5526](https://github.com/manticoresoftware/manticoresearch/commit/552646bb6fefde4e2525298e43e628839b421d3d) исправлен неверный фильтр в запросах PQ matches;
* [Commit 204f](https://github.com/manticoresoftware/manticoresearch/commit/204f521408b730198a5bab2c20407a3d7df94873) введен небольшой аллокатор объектов для ptr атрибутов
* [Commit 2545](https://github.com/manticoresoftware/manticoresearch/commit/25453e5387152c0575d1eda1b2beb89a49c7493a) проведен рефакторинг ISphFieldFilter с добавлением подсчёта ссылок
* [Commit 1366](https://github.com/manticoresoftware/manticoresearch/commit/1366ee06e828397074b69bec9265d7147170c60e) исправлены неопределённое поведение/segfault при использовании strtod на нетерминированных строках
* [Commit 94bc](https://github.com/manticoresoftware/manticoresearch/commit/94bc6fcbdae4d08bcf6da2966e3ec2966091289f) исправлена утечка памяти при обработке json наборов результатов
* [Commit e78e](https://github.com/manticoresoftware/manticoresearch/commit/e78e9c948963416caff9e4e46296a58080107835) исправлен выход за пределы блока памяти при применении атрибута add
* [Commit fad5](https://github.com/manticoresoftware/manticoresearch/commit/fad572fb543606b33e688eb82a485b86011545fd) рефакторинг CSphDict для версии с подсчётом ссылок
* [Commit fd84](https://github.com/manticoresoftware/manticoresearch/commit/fd841a472b472280c93f4af3506851d19f6adc45) исправлена утечка внутреннего типа AOT за пределами
* [Commit 5ee7](https://github.com/manticoresoftware/manticoresearch/commit/5ee7f208c7685f2ebb9e5623a8802b721e2a7f0a) исправлена утечка памяти в управлении токенизатором
* [Commit 116c](https://github.com/manticoresoftware/manticoresearch/commit/116c5f1abebee9a0d99afe93546a1d8e4c6c6958) исправлена утечка памяти в grouper
* [Commit 56fd](https://github.com/manticoresoftware/manticoresearch/commit/56fdbc9ab38973a9a0ba8ccee45f71cf33c16423) специальное освобождение/копирование для динамических указателей в matches (утечка памяти в grouper)
* [Commit b1fc](https://github.com/manticoresoftware/manticoresearch/commit/31db660f378541aa0d1e7cf2e7209bb0a1f47fd8) исправлена утечка памяти динамических строк для RT
* [Commit 517b](https://github.com/manticoresoftware/manticoresearch/commit/517b9e8749f092e923cd2884b674b5bb84e20755) рефакторинг grouper
* [Commit b1fc](https://github.com/manticoresoftware/manticoresearch/commit/b1fc16140e5dc44290686330b476a254e0092107) небольшой рефакторинг (c++11 c-трушки, некоторые переразметки)
* [Commit 7034](https://github.com/manticoresoftware/manticoresearch/commit/7034e073f4cf2844762e0a464b7c3de05d3d122b) рефакторинг ISphMatchComparator для версии с подсчётом ссылок
* [Commit b1fc](https://github.com/manticoresoftware/manticoresearch/commit/b1fc16140e5dc44290686330b476a254e0092107) приватизация клонировщика
* [Commit efbc](https://github.com/manticoresoftware/manticoresearch/commit/efbc0511d6809c4a57453c7283f9bf53f3fb8d97) упрощение native little-endian для MVA_UPSIZE, DOCINFO2ID_T, DOCINFOSETID
* [Commit 6da0](https://github.com/manticoresoftware/manticoresearch/commit/6da0df4ac9946fc59f98d229b90e12c9221dd4c0) добавлена поддержка valgrind в ubertests
* [Commit 1d17](https://github.com/manticoresoftware/manticoresearch/commit/1d17669cb7ec54ac80b392e101f37a688eb98080) исправлен сбой из-за гонки флага 'success' при подключении
* [Commit 5a09](https://github.com/manticoresoftware/manticoresearch/commit/5a09c32d7ded56b2c0bd3e2ad7968cb8d6362919) переключен epoll на режим edge-triggered
* [Commit 5d52](https://github.com/manticoresoftware/manticoresearch/commit/5d528682737fca03a4352a3093b1319ec27d598c) исправлен оператор IN в выражении с форматированием как у фильтра
* [Commit bd8b](https://github.com/manticoresoftware/manticoresearch/commit/bd8b3c976ff8b4667f55dfdd101b20a920137ac5) исправлен сбой в RT индексе при коммите документа с большим docid
* [Commit ce65](https://github.com/manticoresoftware/manticoresearch/commit/ce656b83b747ce7c55795b53915770c13fb99cfe) исправлены опции без аргументов в indextool
* [Commit 08c9](https://github.com/manticoresoftware/manticoresearch/commit/08c9507177820f441f534bf06a11dac5e54bebb4) исправлена утечка памяти расширенного ключевого слова
* [Commit 30c7](https://github.com/manticoresoftware/manticoresearch/commit/30c75a2f525ec9bda625d903acdc9d4d2e2413dc) исправлена утечка памяти json grouper
* [Commit 6023](https://github.com/manticoresoftware/manticoresearch/commit/6023f269b6f2080e4d380b0957605ef8107c8c9f) исправлена утечка глобальных пользовательских переменных
* [Commit 7c13](https://github.com/manticoresoftware/manticoresearch/commit/7c138f15ca23c0c8717fa12041e3db7f988fcf15) исправлена утечка динамических строк при раннем отклонении matches
* [Commit 9154](https://github.com/manticoresoftware/manticoresearch/commit/9154b18eaed5302e8330cb3a95bd968959ccb312) исправлена утечка при length(<expression>)
* [Commit 43fc](https://github.com/manticoresoftware/manticoresearch/commit/43fca3a4e26139765d0fac8de054200dc4875fc6) исправлена утечка памяти из-за strdup() в парсере
* [Commit 71ff](https://github.com/manticoresoftware/manticoresearch/commit/71ff77737e63d1019b7c9804dca7fa2351025dba) исправлен рефакторинг парсера выражений для точного соблюдения подсчёта ссылок

## Версия 2.7.2 GA, 27 августа 2018
### Улучшения
* совместимость с клиентами MySQL 8
* [TRUNCATE](Emptying_a_table.md) С ПЕРЕОПРЕДЕЛЕНИЕМ
* снят счетчик памяти на SHOW STATUS для RT индексов
* глобальный кэш для нескольких агентов
* улучшен IOCP на Windows
* VIP соединения для HTTP протокола
* Manticore SQL [DEBUG](Reporting_bugs.md#DEBUG) команда, которая может выполнять различные подкоманды
* [shutdown_token](Server_settings/Searchd.md#shutdown_token) - SHA1 хэш пароля, необходимый для вызова `shutdown` с использованием команды DEBUG
* новые статистики для SHOW AGENT STATUS (_ping, _has_perspool, _need_resolve)
* опция --verbose индексатора теперь принимает \[debugvv\] для печати отладочных сообщений

### Исправления ошибок
* [Commit 3900](https://github.com/manticoresoftware/manticoresearch/commit/390082a7be0a1f9539b30361d11d54de35c62a44) убрал wlock при оптимизации
* [Commit 4c33](https://github.com/manticoresoftware/manticoresearch/commit/4c337648329be22373be33333cbc9c5664d18b80) исправил wlock при перезагрузке настроек индекса
* [Commit b5ea](https://github.com/manticoresoftware/manticoresearch/commit/b5ea8dc0b18a23dbd739e92c66dd93b0094df14b) исправил утечку памяти при запросе с фильтром JSON
* [Commit 930e](https://github.com/manticoresoftware/manticoresearch/commit/930e831a43fd3d1ce996185be968d1df49a088d0) исправил пустые документы в наборе результатов PQ
* [Commit 53de](https://github.com/manticoresoftware/manticoresearch/commit/53deecfecb374431f84f8592a1095a77407b8aea) исправил путаницу задач из-за удаленной
* [Commit cad9](https://github.com/manticoresoftware/manticoresearch/commit/cad9b9c617719aad97e9f5a2ae93491153d8e504) исправил неправильный подсчет удаленных хостов
* [Commit 9000](https://github.com/manticoresoftware/manticoresearch/commit/90008cc15c4684d445d37a391bb4a667bbc2e365) исправил утечку памяти разобранных дескрипторов агентов
* [Commit 978d](https://github.com/manticoresoftware/manticoresearch/commit/978d839641ad6040c22c9fc3b703b1a02685f266) исправил утечку в поиске
* [Commit 0193](https://github.com/manticoresoftware/manticoresearch/commit/0193946105ac08913362b7d7acf0c90f9b38dc85) косметические изменения в явных/встраиваемых c-trs, использование override/final
* [Commit 943e](https://github.com/manticoresoftware/manticoresearch/commit/943e2997b33f9b2eda23d94a66068910f9476ebd) исправил утечку json в локальной/удаленной схеме
* [Commit 02db](https://github.com/manticoresoftware/manticoresearch/commit/02dbdd6f1d0b72e7e458ebebdd2c67f989577e6b) исправил утечку json сортировки колонки expr в локальной/удаленной схеме
* [Commit c74d](https://github.com/manticoresoftware/manticoresearch/commit/c74d0b4035e4214510376845b22cb676a8da24a3) исправил утечку константного псевдонима
* [Commit 6e5b](https://github.com/manticoresoftware/manticoresearch/commit/6e5b57e1d3952c9695376728a143bf5434208f53) исправил утечку потока предварительного чтения
* [Commit 39c7](https://github.com/manticoresoftware/manticoresearch/commit/39c740fef5bf71da0bee60a35f5dd9f471af850f) исправил зависание при выходе из-за зависшего ожидания в netloop
* [Commit adaf](https://github.com/manticoresoftware/manticoresearch/commit/adaf9772f95772ad6d5297acace6c5cb92b56ad5) исправил зависание поведения 'ping' при изменении HA агента на обычный хост
* [Commit 32c4](https://github.com/manticoresoftware/manticoresearch/commit/32c40eac9cdd40b15d79e58b2f15416f164f9701) отдельный gc для хранилища панели управления
* [Commit 511a](https://github.com/manticoresoftware/manticoresearch/commit/511a3cf2c1f1858f6e94fe9f8175b7216db3cbd6) исправил исправление указателя с подсчетом ссылок
* [Commit 32c4](https://github.com/manticoresoftware/manticoresearch/commit/af2326cd2927effbad7bec88e8dc238a566cf27e) исправил сбой indextool на несуществующем индексе
* [Commit 156e](https://github.com/manticoresoftware/manticoresearch/commit/156edc717dc63d3ea120f42466eba6e022da88be) исправил имя вывода превышающего атрибута/поля в индексации xmlpipe
* [Commit cdac](https://github.com/manticoresoftware/manticoresearch/commit/cdac6d1f17d384bfc8bae49e91241b2f346c3830) исправил значение индексатора по умолчанию, если в конфигурации нет секции индексатора
* [Commit e61e](https://github.com/manticoresoftware/manticoresearch/commit/e61ec00b6b27d1d5878247e2ee817f3b1e7fde16) исправил неправильные встроенные стоп-слова в дисковом чанке по RT индексу после перезапуска сервера
* [Commit 5fba](https://github.com/manticoresoftware/manticoresearch/commit/5fba49d5e28c17de4c0acbd984466127f42de6e8) исправил пропуск фантомных (уже закрытых, но не окончательно удаленных из поллера) соединений
* [Commit f22a](https://github.com/manticoresoftware/manticoresearch/commit/f22ae34b623906b3c5a05a06198dad2e548dc541) исправил смешанные (сиротские) сетевые задачи
* [Commit 4689](https://github.com/manticoresoftware/manticoresearch/commit/46890e70eb8dcd00c1bf1b030538b8f131c601c2) исправил сбой при чтении действия после записи
* [Commit 03f9](https://github.com/manticoresoftware/manticoresearch/commit/03f9df904f611c438d5ebcaaeef911b0dc8d6c39) исправил сбои searchd при запуске тестов на Windows
* [Commit e925](https://github.com/manticoresoftware/manticoresearch/commit/e9255ed2704790a3d7f5273d38b85433463cb3ff) исправил обработку кода EINPROGRESS при обычном connect()
* [Commit 248b](https://github.com/manticoresoftware/manticoresearch/commit/248b72f1544b27e9e82773284050d44eeb1714e1) исправил тайм-ауты соединения при работе с TFO

## Версия 2.7.1 GA, 4 июля 2018
### Улучшения
* улучшена производительность подстановочных знаков при сопоставлении нескольких документов в PQ
* поддержка полных запросов в PQ
* поддержка MVA атрибутов в PQ
* поддержка regexp и RLP для индексов перколяторов

### Исправления ошибок
* [Commit 6885](https://github.com/manticoresoftware/manticoresearch/commit/68856261b41f6950666f9e5122726839fb3c71d0) исправил потерю строки запроса
* [Commit 0f17](https://github.com/manticoresoftware/manticoresearch/commit/0f1770943cba53bfd2e0edfdf0b1495ff0dd0e89) исправил пустую информацию в операторе SHOW THREADS
* [Commit 53fa](https://github.com/manticoresoftware/manticoresearch/commit/53faa36a916bb87868e83aa79c1a1f972fb20ca0) исправил сбой при сопоставлении с оператором NOTNEAR
* [Commit 2602](https://github.com/manticoresoftware/manticoresearch/commit/26029a7cd77b518fb3a27588d7db6c8bffc73b0f) исправил сообщение об ошибке при плохом фильтре для удаления PQ


## Версия 2.7.0 GA, 11 июня 2018
### Улучшения
* уменьшено количество системных вызовов для снижения влияния патчей Meltdown и Spectre
* внутренняя переработка управления локальными индексами
* рефакторинг удаленных сниппетов
* полная перезагрузка конфигурации
* все соединения между узлами теперь независимы
* улучшения протокола
* коммуникация в Windows переключена с wsapoll на порты завершения ввода-вывода (IO completion ports)
* TFO можно использовать для связи между мастером и узлами
* [SHOW STATUS](Node_info_and_management/Node_status.md#SHOW-STATUS) теперь выводит версию сервера и mysql_version_string
* добавлена опция `docs_id` для документов, вызываемых в CALL PQ.
* фильтр перколяционных запросов теперь может содержать выражения
* распределенные индексы могут работать с FEDERATED
* фиктивные SHOW NAMES COLLATE и `SET wait_timeout` (для лучшей совместимости с ProxySQL)

### Исправления ошибок
* [Коммит 5bcf](https://github.com/manticoresoftware/manticoresearch/commit/5bcff05d19e495f1a44f190885b25cbb6f217c4a) исправлено добавление неравенства к тегам PQ
* [Коммит 9ebc](https://github.com/manticoresoftware/manticoresearch/commit/9ebc58916cd515eaa88da66d0895aebf0d1f2b5f) исправлено добавление поля id документа в JSON-документ оператора CALL PQ
* [Коммит 8ae0](https://github.com/manticoresoftware/manticoresearch/commit/8ae0e593a286110d8505d88baa3ac9e433cb38c9) исправлена очистка обработчиков операторов для индекса PQ
* [Коммит c24b](https://github.com/manticoresoftware/manticoresearch/commit/c24b152344ecd77661566cddb803487f51d3c1aa) исправлена фильтрация PQ по JSON и строковым атрибутам
* [Коммит 1b8b](https://github.com/manticoresoftware/manticoresearch/commit/1b8bdde19aaa362785ea4c33bdc019154f8fbac4) исправлен разбор пустой JSON-строки
* [Коммит 1ad8](https://github.com/manticoresoftware/manticoresearch/commit/1ad8a07dbfc1131913a8d92c261fbb48f934e8b7) исправлен сбой при мультизапросе с фильтрами OR
* [Коммит 69b8](https://github.com/manticoresoftware/manticoresearch/commit/69b89806380a229e36287ad9daf6f0ea2b5453eb) исправлена работа indextool для использования общего раздела конфигурации (опция lemmatizer_base) для команд (dumpheader)
* [Коммит 6dbe](https://github.com/manticoresoftware/manticoresearch/commit/6dbeaf2389d2af6a9dfccb56bfc986685b9f203e) исправлена пустая строка в наборе результатов и фильтре
* [Коммит 39c4](https://github.com/manticoresoftware/manticoresearch/commit/39c4eb55ecc85f6dd54f6c8c6d6dfcf87fd8748e) исправлены отрицательные значения id документа
* [Коммит 266b](https://github.com/manticoresoftware/manticoresearch/commit/266b7054f98e7d30fc2829b5c2467dc9f90def92) исправлена длина обрезки слова для очень длинных индексируемых слов
* [Коммит 4782](https://github.com/manticoresoftware/manticoresearch/commit/47823b0bd06cbd016a24345c99bb8693790b43dd) исправлено сопоставление нескольких документов для запросов с подстановочными знаками в PQ


## Версия 2.6.4 GA, 3 мая 2018
### Функции и улучшения
* Поддержка [движка MySQL FEDERATED](Extensions/FEDERATED.md)
* Пакеты MySQL теперь возвращают флаг SERVER_STATUS_AUTOCOMMIT, что повышает совместимость с ProxySQL
* [listen_tfo](Server_settings/Searchd.md#listen_tfo) - включение TCP Fast Open соединений для всех слушателей
* indexer --dumpheader может также выгружать заголовок RT из файла .meta
* скрипт сборки cmake для Ubuntu Bionic

### Исправления ошибок
* [Коммит 355b](https://github.com/manticoresoftware/manticoresearch/commit/355b11629174813abd9cd5bf2233be0783f77745) исправлены неверные записи кэша запросов для RT-индекса;
* [Коммит 546e](https://github.com/manticoresoftware/manticoresearch/commit/546e2297a01cc2913bc0d33052d5bcefae8737eb) исправлена потеря настроек индекса после бесшовной ротации
* [Коммит 0c45](https://github.com/manticoresoftware/manticoresearch/commit/0c4509898393993bc87194d2aca2070395ff7f83) исправлена установка длины инфикса против префикса; добавлено предупреждение о неподдерживаемой длине инфикса
* [Коммит 8054](https://github.com/manticoresoftware/manticoresearch/commit/80542fa54d727c781635918e8d4d65a20c2fbd1e) исправлен порядок автоматической очистки RT-индексов
* [Коммит 705d](https://github.com/manticoresoftware/manticoresearch/commit/705d8c5f407726ed1c6f9745ecffa375682c8969) исправлены проблемы со схемой набора результатов для индекса с несколькими атрибутами и запросов к нескольким индексам
* [Коммит b0ba](https://github.com/manticoresoftware/manticoresearch/commit/b0ba932fff4a082f642cb737381c8a70215d4cc9) исправлена потеря некоторых совпадений при пакетной вставке с дубликатами документов
* [Коммит 4510](https://github.com/manticoresoftware/manticoresearch/commit/4510fa44ad54b5bbac53c591144501565130fcae) исправлена ошибка, когда оптимизация не могла объединить дисковые чанки RT-индекса с большим количеством документов

## Версия 2.6.3 GA, 28 марта 2018
### Улучшения
* jemalloc при компиляции. Если jemalloc присутствует в системе, его можно включить с помощью флага cmake `-DUSE_JEMALLOC=1`

### Исправления ошибок
* [Коммит 85a6](https://github.com/manticoresoftware/manticoresearch/commit/85a6d7e3db75f28c491042409bb677ed39eba07b) исправлено логирование опции expand_keywords в журнал SQL-запросов Manticore
* [Коммит caaa](https://github.com/manticoresoftware/manticoresearch/commit/caaa3842f4dbd8507fd9e20bf3371a0c8ec0c087) исправлен интерфейс HTTP для корректной обработки запросов большого размера
* [Коммит e386](https://github.com/manticoresoftware/manticoresearch/commit/e386d842b36195719e459b2db1fb20e402f9a51f) исправлен сбой сервера при DELETE в RT-индекс с включенной опцией index_field_lengths
* [Коммит cd53](https://github.com/manticoresoftware/manticoresearch/commit/ce528f37eb5d8284661928f6f3b340a4a1985253) исправлена работа опции командной строки searchd cpustats в неподдерживаемых системах
* [Коммит 8740](https://github.com/manticoresoftware/manticoresearch/commit/8740fd63ae842c2ab0e51a8fcd6180d787ed57ec) исправлено сопоставление подстрок utf8 с определенными минимальными длинами


## Версия 2.6.2 GA, 23 февраля 2018
### Улучшения
* улучшена производительность [Percolate Queries](Searching/Percolate_query.md) при использовании оператора NOT и для пакетных документов.
* [percolate_query_call](Searching/Percolate_query.md) может использовать несколько потоков в зависимости от [dist_threads](Server_settings/Searchd.md#threads)
* новый оператор полнотекстового соответствия NOTNEAR/N
* LIMIT для SELECT на перколяционных индексах
* [expand_keywords](Searching/Options.md#expand_keywords) может принимать значения 'start','exact' (где 'star,exact' имеет тот же эффект, что и '1')
* ranged-main-query для [joined fields](Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Processing_fetched_data.md#sql_joined_field), который использует диапазонный запрос, определенный sql_query_range

### Исправления ошибок
* [Коммит 72dc](https://github.com/manticoresoftware/manticoresearch/commit/72dcf669744e9b7d636dfc213d24df85ab301f6b) исправлен сбой при поиске по сегментам в оперативной памяти; взаимоблокировка при сохранении дискового чанка с двойным буфером; взаимоблокировка при сохранении дискового чанка во время оптимизации
* [Коммит 3613](https://github.com/manticoresoftware/manticoresearch/commit/36137149a1c3c0893bdda5a28fc7e8244bf2d4ae) исправлен сбой индексатора на встроенной xml-схеме с пустым именем атрибута
* [Коммит 48d7](https://github.com/manticoresoftware/manticoresearch/commit/48d7e8001d2a66466ca64577f27ddc5421a67251) исправлено ошибочное удаление pid-файла, не принадлежащего процессу
* [Коммит a556](https://github.com/manticoresoftware/manticoresearch/commit/a5563a465ddc59ef71e65f17b68bc33f9700e838) исправлены оставшиеся иногда во временной папке "осиротевшие" fifo-файлы
* [Коммит 2376](https://github.com/manticoresoftware/manticoresearch/commit/2376e8fc4508944b96959bd10686c6d51f5145e8) исправлен пустой набор результатов FACET с неправильной строкой NULL
* [Коммит 4842](https://github.com/manticoresoftware/manticoresearch/commit/4842b67acdd5da75672db6c08ac563b48577ddd7) исправлена проблема с блокировкой индекса при запуске сервера как службы Windows
* [Коммит be35](https://github.com/manticoresoftware/manticoresearch/commit/be35feef54ded59125750916cd211e56108eddae) исправлены неправильные библиотеки iconv на Mac OS
* [Коммит 8374](https://github.com/manticoresoftware/manticoresearch/commit/83744a977d7837d1d87cd506b88b0e7eb199efe6) исправлен неправильный count(\*)


## Версия 2.6.1 GA, 26 января 2018
### Улучшения
* [agent_retry_count](Server_settings/Searchd.md#agent_retry_count) в случае агентов с зеркалами дает значение количества повторных попыток на зеркало, а не на агента; общее количество повторных попыток на агента равно agent_retry_count\*зеркала.
* [agent_retry_count](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_retry_count) теперь можно указывать для каждого индекса, переопределяя глобальное значение. Добавлен псевдоним [mirror_retry_count](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#mirror_retry_count).
* retry_count можно указать в определении агента, и значение представляет количество повторных попыток на агента
* Percolate Queries теперь доступны в HTTP JSON API по адресу [/json/pq](Searching/Percolate_query.md#Performing-a-percolate-query-with-CALL-PQ).
* Добавлены опции -h и -v (справка и версия) для исполняемых файлов
* Поддержка [morphology_skip_fields](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) для Real-Time индексов

### Исправления ошибок
* [Коммит a40b](https://github.com/manticoresoftware/manticore/commit/a40b0793feff65e40d10062568d9847c08d10f57) исправлена работа ranged-main-query с sql_range_step при использовании на поле MVA
* [Коммит f2f5](https://github.com/manticoresoftware/manticore/commit/f2f53757db45bcfb1544263ce0817e856656a621) исправлена проблема с зависанием системного цикла blackhole и кажущимся отключением агентов blackhole
* [Коммит 84e1](https://github.com/manticoresoftware/manticore/commit/84e1f54aef25e0fce98870ad2dd784db5116f1d6) исправлен идентификатор запроса для обеспечения согласованности, исправлен дублирующийся идентификатор для сохраненных запросов
* [Коммит 1948](https://github.com/manticoresoftware/manticore/commit/19484231814fcb82b21763a3a4a9f45adc6b2d40) исправлен сбой сервера при завершении работы из различных состояний
* [Коммит 9a70](https://github.com/manticoresoftware/manticore/commit/9a706b499a1d61a90076065a1a703029d49db958)  [Коммит 3495](https://github.com/manticoresoftware/manticore/commit/3495fd70cba8846b1a50d55d6679b039414c3d2a) таймауты на длинные запросы
* [Коммит 3359](https://github.com/manticoresoftware/manticore/commit/3359bcd89b4f79a645fe84b8cf8616ce0addff02) рефакторинг сетевого опроса мастер-агента на системах, основанных на kqueue (Mac OS X, BSD).


## Версия 2.6.0, 29 декабря 2017
### Функции и улучшения
* [HTTP JSON](Connecting_to_the_server/HTTP.md): JSON-запросы теперь могут выполнять проверку равенства по атрибутам, MVA и JSON-атрибуты можно использовать при вставках и обновлениях, обновления и удаления через JSON API можно выполнять на распределенных индексах
* [Percolate Queries](Searching/Percolate_query.md)
* Удалена поддержка 32-битных docid из кода. Также удален весь код, который преобразует/загружает устаревшие индексы с 32-битными docid.
* [Морфология только для определенных полей](https://github.com/manticoresoftware/manticore/issues/7). Новая директива индекса morphology_skip_fields позволяет определить список полей, для которых морфология не применяется.
* [expand_keywords теперь может быть директивой времени выполнения запроса, устанавливаемой с помощью оператора OPTION](https://github.com/manticoresoftware/manticore/issues/8)

### Исправления ошибок
* [Коммит 0cfa](https://github.com/manticoresoftware/manticore/commit/0cfae4c) исправлен сбой в отладочной сборке сервера (и возможно неопределенное поведение в релизной) при сборке с rlp
* [Коммит 3242](https://github.com/manticoresoftware/manticore/commit/324291e) исправлена оптимизация RT-индекса с включенной опцией progressive, которая объединяет kill-листы в неправильном порядке
* [Коммит ac0e](https://github.com/manticoresoftware/manticore/commit/ac0efee)  незначительный сбой на Mac
* множество мелких исправлений после тщательного статического анализа кода
* другие мелкие исправления ошибок

### Обновление
В этом выпуске мы изменили внутренний протокол, используемый мастерами и агентами для взаимодействия друг с другом. В случае, если вы запускаете Manticoresearch в распределенной среде с несколькими экземплярами, убедитесь, что сначала обновляете агентов, а затем мастеров.

## Version 2.5.1, 23 November 2017
### Features and improvements
* JSON queries on [HTTP API protocol](Connecting_to_the_server/HTTP.md). Supported search, insert, update, delete, replace operations. Data manipulation commands can be also bulked, also there are some limitations currently as MVA and JSON attributes can't be used for inserts, replaces or updates.
* [RELOAD INDEXES](Data_creation_and_modification/Adding_data_from_external_storages/Rotating_a_table.md#RELOAD-TABLES) command
* [FLUSH LOGS](Logging/Rotating_query_and_server_logs.md) command
* [SHOW THREADS](Node_info_and_management/SHOW_THREADS.md) can show progress of optimize, rotation or flushes.
* GROUP N BY work correctly with MVA attributes
* blackhole agents are run on separate thread to not affect master query anymore
* implemented reference count on indexes, to avoid stalls caused by rotations and high load
* SHA1 hashing implemented, not exposed yet externally
* fixes for compiling on FreeBSD, macOS and Alpine

### Bugfixes
* [Commit 9897](https://github.com/manticoresoftware/manticore/commit/989752b) filter regression with block index
* [Commit b1c3](https://github.com/manticoresoftware/manticore/commit/b1c3864) rename PAGE_SIZE -> ARENA_PAGE_SIZE for compatibility with musl
* [Commit f213](https://github.com/manticoresoftware/manticore/commit/f2133cc) disable googletests for cmake < 3.1.0
* [Commit f30e](https://github.com/manticoresoftware/manticore/commit/0839de7) failed to bind socket on server restart
* [Commit 0807](https://github.com/manticoresoftware/manticore/commit/0807240) fixed crash of server on shutdown
* [Commit 3e3a](https://github.com/manticoresoftware/manticore/commit/3e3acc3) fixed show threads for system blackhole thread
* [Commit 262c](https://github.com/manticoresoftware/manticore/commit/262c3fe) Refactored config check of iconv, fixes building on FreeBSD and Darwin

## Version 2.4.1 GA, 16 October 2017
### Features and improvements
* OR operator in WHERE clause between attribute filters
* Maintenance mode ( SET MAINTENANCE=1)
* [CALL KEYWORDS](Searching/Autocomplete.md#CALL-KEYWORDS) available on distributed indexes
* [Grouping in UTC](Server_settings/Searchd.md#grouping_in_utc)
* [query_log_mode](Server_settings/Searchd.md#query_log_mode) for custom log files permissions
* Field weights can be zero or negative
* [max_query_time](Searching/Options.md#max_query_time) can now affect full-scans
* added [net_wait_tm](Server_settings/Searchd.md#net_wait_tm), [net_throttle_accept](Server_settings/Searchd.md#net_throttle_accept) and [net_throttle_action](Server_settings/Searchd.md#net_throttle_action) for network thread fine tuning (in case of workers=thread_pool)
* COUNT DISTINCT works with facet searches
* IN can be used with JSON float arrays
* multi-query optimization is not broken anymore by integer/float expressions
* [SHOW META](Node_info_and_management/SHOW_META.md) shows a `multiplier` row when multi-query optimization is used

### Compiling
Manticore Search is built using cmake and the minimum gcc version required for compiling is 4.7.2.

### Folders and service
* Manticore Search runs under `manticore` user.
* Default data folder is now `/var/lib/manticore/`.
* Default log folder is now `/var/log/manticore/`.
* Default pid folder is now `/var/run/manticore/`.

### Bugfixes
* [Commit a58c](https://github.com/manticoresoftware/manticore/commit/a58c619) fixed SHOW COLLATION statement that breaks java connector
* [Commit 631c](https://github.com/manticoresoftware/manticore/commit/631cf4e) fixed crashes on processing distributed indexes; added locks to distributed index hash; removed move and copy operators from agent
* [Commit 942b](https://github.com/manticoresoftware/manticore/commit/942bec0) fixed crashes on processing distributed indexes due to parallel reconnects
* [Commit e5c1](https://github.com/manticoresoftware/manticore/commit/e5c1ed2) fixed crash at crash handler on store query to server log
* [Commit 4a4b](https://github.com/manticoresoftware/manticore/commit/4a4bda5) fixed a crash with pooled attributes in multiqueries
* [Commit 3873](https://github.com/manticoresoftware/manticore/commit/3873bfb) fixed reduced core size by prevent index pages got included into core file
* [Commit 11e6](https://github.com/manticoresoftware/manticore/commit/11e6254) fixed searchd crashes on startup when invalid agents are specified
* [Commit 4ca6](https://github.com/manticoresoftware/manticore/commit/4ca6350) fixed indexer reports error in sql_query_killlist query
* [Commit 123a](https://github.com/manticoresoftware/manticore/commit/123a9f0) fixed fold_lemmas=1 vs hit count
* [Commit cb99](https://github.com/manticoresoftware/manticore/commit/cb99164) fixed inconsistent behavior of html_strip
* [Commit e406](https://github.com/manticoresoftware/manticore/commit/e406761) fixed optimize rt index loose new settings; fixed optimize with sync option lock leaks;
* [Commit 86ae](https://github.com/manticoresoftware/manticore/commit/86aeb82) fixed processing erroneous multiqueries
* [Commit 2645](https://github.com/manticoresoftware/manticore/commit/2645230) fixed result set depends on multi-query order
* [Commit 7239](https://github.com/manticoresoftware/manticore/commit/72395d9) fixed server crash on multi-query with bad query
* [Commit f353](https://github.com/manticoresoftware/manticore/commit/f353326) fixed shared to exclusive lock
* [Commit 3754](https://github.com/manticoresoftware/manticore/commit/3754785) fixed server crash for query without indexes
* [Commit 29f3](https://github.com/manticoresoftware/manticore/commit/29f360e) fixed dead lock of server

## Version 2.3.3, 06 July 2017
* Manticore branding
