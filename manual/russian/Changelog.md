# Журнал изменений

## Девелоперская версия

...

## Версия 13.2.3
**Выпущено**: 8 июля 2025

### Критические изменения
* ⚠️ [v13.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.0.0)  Обновлено API библиотеки KNN для поддержки пустых значений [float_vector](https://manual.manticoresearch.com/Creating_a_table/Data_types#Float-vector). Это обновление не изменяет формат данных, но повышает версию API Manticore Search / MCL.
* ⚠️ [v12.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/12.0.0) [ PR #3516](https://github.com/manticoresoftware/manticoresearch/pull/3516) Исправлена ошибка с некорректными ID исходных и целевых строк при обучении и построении индекса KNN. Это обновление не изменяет формат данных, но повышает версию API Manticore Search / MCL.
* ⚠️ [v11.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.0.0)  Добавлена поддержка новых функций поиска KNN, таких как квантизация, переоценка и сверхвыборка. Эта версия изменяет формат данных KNN и SQL-синтаксис [KNN_DIST()](https://manual.manticoresearch.com/Searching/KNN?client=SQL#KNN-vector-search). Новая версия может читать старые данные, но старые версии не смогут прочитать новый формат.

### Новые функции и улучшения
* 🆕 [v13.2.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.0) [ PR #3549](https://github.com/manticoresoftware/manticoresearch/issues/3525) Исправлена проблема с `@@collation_database`, вызывавшая несовместимость с некоторыми версиями mysqldump
* 🆕 [v13.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.1.0) [ Issue #3489](https://github.com/manticoresoftware/manticoresearch/issues/3489) Исправлена ошибка Fuzzy Search, препятствовавшая разбору некоторых SQL-запросов
* 🆕 [v12.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/12.1.0) [ Issue #3426](https://github.com/manticoresoftware/manticoresearch/issues/3426) Добавлена поддержка операционных систем RHEL 10
* 🆕 [v11.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.1.0)  Добавлена поддержка пустых float-векторов в [поиске KNN](https://manual.manticoresearch.com/Searching/KNN#KNN-vector-search)
* 🆕 [v10.2.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.0) [ Issue #3252](https://github.com/manticoresoftware/manticoresearch/issues/3252) [log_level](https://manual.manticoresearch.com/Server_settings/Setting_variables_online#SET) теперь также регулирует уровень подробности логирования Buddy

### Исправления ошибок
* 🪲 [v13.2.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.3) [ PR #3556](https://github.com/manticoresoftware/manticoresearch/pull/3556) Исправлен разбор опции "oversampling" в JSON-запросах
* 🪲 [v13.2.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.2)  Исправлена ошибка логирования сбоев на Linux и FreeBSD удалением использования Boost stacktrace
* 🪲 [v13.2.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.1) [ Issue #3298](https://github.com/manticoresoftware/manticoresearch/issues/3298) Исправлено логирование сбоев при работе внутри контейнера
* 🪲 [v12.0.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/12.0.2)  Улучшена точность статистики по таблицам с учетом микросекунд
* 🪲 [v12.0.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/12.0.1) [ PR #3522](https://github.com/manticoresoftware/manticoresearch/pull/3522) Исправлен сбой при фасетировании по MVA в объединённом запросе
* 🪲 [v11.0.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.0.3) [ Issue #3502](https://github.com/manticoresoftware/manticoresearch/issues/3502) Исправлен сбой, связанный с квантизацией векторного поиска
* 🪲 [v11.0.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.0.2) [ Issue #3493](https://github.com/manticoresoftware/manticoresearch/issues/3493) Изменён вывод CPU-загрузки в `SHOW THREADS` на целочисленный формат
* 🪲 [v11.0.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.0.1)  Исправлены пути для колоннарных и вторичных библиотек
* 🪲 [v10.2.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.7)  Добавлена поддержка MCL 5.0.5, включая исправление имени файла библиотеки эмбеддингов
* 🪲 [v10.2.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.6) [ Issue #3469](https://github.com/manticoresoftware/manticoresearch/issues/3469) Применено дополнительное исправление по вопросу #3469
* 🪲 [v10.2.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.4) [ Issue #3469](https://github.com/manticoresoftware/manticoresearch/issues/3469) Исправлена проблема, при которой HTTP-запросы с bool-запросами возвращали пустые результаты
* 🪲 [v10.2.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.5)  Изменено имя файла библиотеки эмбеддингов по умолчанию и добавлены проверки поля 'from' в KNN-векторах
* 🪲 [v10.2.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.3) [ PR #3464](https://github.com/manticoresoftware/manticoresearch/pull/3464) Обновлен Buddy до версии 3.30.2, включая [PR #565 по использованию памяти и логированию ошибок](https://github.com/manticoresoftware/manticoresearch-buddy/pull/565)
* 🪲 [v10.2.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.2) [ Issue #3470](https://github.com/manticoresoftware/manticoresearch/pull/3470) Исправлены фильтры строк JSON, null-фильтры и проблемы с сортировкой в JOIN-запросах
* 🪲 [v10.2.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.1)  Исправлена ошибка в `dist/test_kit_docker_build.sh`, из-за которой пропадал коммит Buddy
* 🪲 [v10.1.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.4)  Исправлен сбой при группировке по MVA в объединённом запросе
* 🪲 [v10.1.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.3) [ Issue #3434](https://github.com/manticoresoftware/manticoresearch/issues/3434) Исправлена ошибка с фильтрацией пустых строк
* 🪲 [v10.1.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.2) [ PR #3452](https://github.com/manticoresoftware/manticoresearch/pull/3452) Обновлен Buddy до версии 3.29.7, которая решает [Buddy #507 - ошибка при мультизапросах с нечетким поиском](https://github.com/manticoresoftware/manticoresearch-buddy/issues/507) и [Buddy #561 - исправление для метрик rate](https://github.com/manticoresoftware/manticoresearch-buddy/pull/561), необходимо для [Helm Release 10.1.0](https://github.com/manticoresoftware/manticoresearch-helm/releases/tag/manticoresearch-10.1.0)
* 🪲 [v10.1.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.1)  Обновлен Buddy до версии 3.29.4, исправляющей [#3388 - "Неопределенный ключ массива 'Field'"](https://github.com/manticoresoftware/manticoresearch/issues/3388) и [Buddy #547 - layouts='ru' может не работать](https://github.com/manticoresoftware/manticoresearch-buddy/issues/547)

## Version 10.1.0
**Released**: June 9th 2025

Эта версия представляет собой обновление с новыми функциями, одним несовместимым изменением и многочисленными улучшениями стабильности и исправлениями ошибок. Изменения направлены на повышение возможностей мониторинга, улучшение функционала поиска и устранение различных критических проблем, влияющих на стабильность и производительность системы.

**Начиная с версии 10.1.0, CentOS 7 больше не поддерживается. Пользователям рекомендуется перейти на поддерживаемую операционную систему.**

### Breaking Changes
* ⚠️ [v10.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.0.0) [ Issue #540](https://github.com/manticoresoftware/manticoresearch-buddy/issues/540) BREAKING CHANGE: установлен параметр по умолчанию `layouts=''` для [нечеткого поиска](Searching/Spell_correction.md#Fuzzy-Search)

### New Features and Improvements
* 🆕 [v10.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.0) [ Issue #537](https://github.com/manticoresoftware/manticoresearch-buddy/issues/537) Добавлен встроенный [экспортер Prometheus](Node_info_and_management/Node_status.md#Prometheus-Exporter)
* 🆕 [v9.8.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.8.0) [ Issue #3409](https://github.com/manticoresoftware/manticoresearch/issues/3409) Добавлена команда [ALTER TABLE tbl REBUILD KNN](Updating_table_schema_and_settings.md#Rebuilding-a-KNN-index)
* 🆕 [v9.7.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.7.0) [ Issue #1778](https://github.com/manticoresoftware/manticoresearch/issues/1778) Добавлена автоматическая генерация эмбеддингов (пока не анонсируется официально, так как код находится в основной ветке и требует дополнительного тестирования)
* 🆕 [v9.6.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.6.0)  Обновлена версия API KNN для поддержки автоэмбеддингов
* 🆕 [v9.5.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.0) [ Issue #1894](https://github.com/manticoresoftware/manticoresearch/issues/1894) Улучшено восстановление кластера: периодическое сохранение `seqno` для более быстрого перезапуска узла после сбоя
* 🆕 [v9.4.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.4.0) [ Issue #2400](https://github.com/manticoresoftware/manticoresearch/issues/2400) Добавлена поддержка последних версий [Logstash](Integration/Logstash.md#Integration-with-Logstash) и Beats

### Bug Fixes
* 🪲 [v10.0.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.0.1)  Исправлена обработка форм слов: определенные пользователем формы теперь переопределяют автоматически сгенерированные; добавлены тесты для проверки 22
* 🪲 [v9.8.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.8.2)  Исправление: обновлен deps.txt для включения исправлений упаковки в MCL, связанных с библиотекой эмбеддингов
* 🪲 [v9.8.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.8.1)  Исправление: обновлен deps.txt с исправлениями упаковки для MCL и библиотеки эмбеддингов
* 🪲 [v9.7.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.7.3) [ Issue #3306](https://github.com/manticoresoftware/manticoresearch/issues/3306) Исправлен сбой с сигналом 11 во время индексирования
* 🪲 [v9.7.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.7.2) [ Issue #3109](https://github.com/manticoresoftware/manticoresearch/issues/3109) Исправлена ошибка, когда несуществующие `@@variables` всегда возвращали 0
* 🪲 [v9.7.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.7.1) [ Issue #3377](https://github.com/manticoresoftware/manticoresearch/issues/3377) Исправлен сбой, связанный с [remove_repeats()](Functions/Searching_and_ranking_functions.md#REMOVE_REPEATS%28%29)
* 🪲 [v9.6.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.6.3) [ PR #3411](https://github.com/manticoresoftware/manticoresearch/pull/3411) Исправление: использование динамического определения версий для метрик телеметрии
* 🪲 [v9.6.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.6.2)  Исправление: небольшое исправление вывода [SHOW VERSION](Node_info_and_management/SHOW_VERSION.md#SHOW-VERSION)
* 🪲 [v9.6.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.6.1)  Исправление: сбой при создании таблицы с атрибутом KNN без модели
* 🪲 [v9.5.16](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.16) [ Issue #3342](https://github.com/manticoresoftware/manticoresearch/issues/3342) Исправлена ошибка, при которой `SELECT ... FUZZY=0` не всегда отключал нечеткий поиск
* 🪲 [v9.5.15](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.15) [ PR #3397](https://github.com/manticoresoftware/manticoresearch/pull/3397) Добавлена поддержка [MCL](https://github.com/manticoresoftware/columnar) 4.2.2; исправлены ошибки со старыми форматами хранения
* 🪲 [v9.5.14](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.14) [ Issue #3392](https://github.com/manticoresoftware/manticoresearch/issues/3392) Исправлена некорректная обработка строк в HTTP JSON ответах
* 🪲 [v9.5.13](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.13) [ Issue #3356](https://github.com/manticoresoftware/manticoresearch/issues/3356) Исправлен сбой в сложном случае полнотекстового запроса (общий подтермин)
* 🪲 [v9.5.12](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.12)  Исправлена опечатка в сообщении об ошибке автосброса на диск чанка
* 🪲 [v9.5.11](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.11) [ Issue #3195](https://github.com/manticoresoftware/manticoresearch/issues/3195) Улучшен [автосброс чанка на диск](Securing_and_compacting_a_table/Flushing_RAM_chunk_to_disk.md#Flushing-RAM-chunk-to-disk): пропуск сохранения, если идет оптимизация
* 🪲 [v9.5.10](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.10) [ Issue #3313](https://github.com/manticoresoftware/manticoresearch/issues/3313) Исправлена проверка дубликатов ID для всех дисковых чанков в RT таблице с помощью [indextool](Miscellaneous_tools.md#indextool)
* 🪲 [v9.5.9](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.9) [ Issue #3132](https://github.com/manticoresoftware/manticoresearch/issues/3132) Добавлена поддержка сортировки `_random` в JSON API
* 🪲 [v9.5.8](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.8) [ Issue #3382](https://github.com/manticoresoftware/manticoresearch/issues/3382) Исправлена проблема: нельзя было использовать uint64 ID документа через JSON HTTP API
* 🪲 [v9.5.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.7) [ Issue #3385](https://github.com/manticoresoftware/manticoresearch/issues/3385) Исправлены некорректные результаты при фильтрации по `id != value`
* 🪲 [v9.5.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.6) [ PR #538](https://github.com/manticoresoftware/manticoresearch-buddy/pull/538) Исправлена критическая ошибка в нечёткости совпадения в некоторых случаях
* 🪲 [v9.5.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.5) [ Issue #3199](https://github.com/manticoresoftware/manticoresearch/issues/3199) Исправлено декодирование пробела в параметрах HTTP-запросов Buddy (например, `%20` и `+`)
* 🪲 [v9.5.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.4) [ Issue #3133](https://github.com/manticoresoftware/manticoresearch/issues/3133) Исправлена некорректная сортировка по `json.field` в фасетном поиске
* 🪲 [v9.5.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.3) [ Issue #3091](https://github.com/manticoresoftware/manticoresearch/issues/3091) Исправлены несогласованные результаты поиска для разделителей в SQL и JSON API
* 🪲 [v9.5.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.2) [ Issue #2819](https://github.com/manticoresoftware/manticoresearch/issues/2819) Улучшена производительность: заменён `DELETE FROM` на `TRUNCATE` для распределённых таблиц
* 🪲 [v9.5.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.1) [ Issue #3080](https://github.com/manticoresoftware/manticoresearch/issues/3080) Исправен сбой при фильтрации с алиасом `geodist()` с JSON-атрибутами

## Version 9.3.2
Релиз: 2 мая 2025

В этом релизе исправлено множество багов и улучшена стабильность, улучшен трекинг использования таблиц, а также улучшено управление памятью и ресурсами.

❤️ Особая благодарность [@cho-m](https://github.com/cho-m) за исправление совместимости сборки с Boost 1.88.0 и [@benwills](https://github.com/benwills) за улучшение документации по `stored_only_fields`.

* 🪲 [v9.3.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.3.2)  Исправлена проблема, когда в колонке "Show Threads" активность CPU отображалась как число с плавающей точкой вместо строки; также исправлена ошибка парсинга результата PyMySQL из-за неправильного типа данных.
* 🪲 [v9.3.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.3.1) [ Issue #3343](https://github.com/manticoresoftware/manticoresearch/issues/3343) Исправлены оставшиеся файлы `tmp.spidx` при прерывании процесса оптимизации.
* 🆕 [v9.3.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.3.0) [ PR #3337](https://github.com/manticoresoftware/manticoresearch/pull/3337) Добавлен счётчик команд на таблицу и детальная статистика использования таблиц.
* 🪲 [v9.2.39](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.39) [ Issue #3236](https://github.com/manticoresoftware/manticoresearch/issues/3236) Исправлено: предотвращено повреждение таблиц удалением сложных обновлений чанков. Использование функций ожидания внутри сериализованного воркера ломало последовательную обработку, что могло повредить таблицы.
Автосброс реализован заново. Удалена внешняя очередь опроса для избегания лишних блокировок таблиц. Добавлено условие для "маленькой таблицы": если число документов ниже лимита "маленькой таблицы" (8192) и нет Secondary Index (SI), сброс пропускается.

* 🪲 [v9.2.38](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.38)  Исправлено: пропуск создания Secondary Index (SI) для фильтров с использованием `ALL`/`ANY` в списках строк, не влияя на JSON-атрибуты.
* 🪲 [v9.2.37](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.37) [ Issue #2898](https://github.com/manticoresoftware/manticoresearch/issues/2898) Добавлена поддержка обратных кавычек для системных таблиц.
* 🪲 [v9.2.36](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.36)  Исправлено: использование плейсхолдера для операций с кластером в наследуемом коде. В парсере теперь явно разделяются поля для имени таблицы и кластера.
* 🪲 [v9.2.35](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.35)  Исправлен сбой при убирании кавычек с одиночной `'`.
* 🪲 [v9.2.34](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.34) [ Issue #3090](https://github.com/manticoresoftware/manticoresearch/issues/3090) Исправлено: обработка больших ID документов (ранее могла не находить их).
* 🪲 [v9.2.33](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.33)  Исправлено: использование беззнаковых целых чисел для размеров битовых векторов.
* 🪲 [v9.2.32](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.32)  Исправлено: снижение пикового потребления памяти при слиянии. Поиски docid-в-rowid теперь используют 12 байт на документ вместо 16. Пример: 24 ГБ RAM на 2 миллиарда документов вместо 36 ГБ.
* 🪲 [v9.2.31](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.31) [ Issue #3238](https://github.com/manticoresoftware/manticoresearch/issues/3238) Исправление: некорректное значение `COUNT(*)` в больших таблицах реального времени.
* 🪲 [v9.2.30](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.30)  Исправление: неопределенное поведение при обнулении строковых атрибутов.
* 🪲 [v9.2.29](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.29)  Незначительное исправление: улучшен текст предупреждения.
* 🪲 [v9.2.28](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.28) [ Issue #3290](https://github.com/manticoresoftware/manticoresearch/issues/3290) Улучшение: улучшен `indextool --buildidf`
* 🪲 [v9.2.27](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.27) [ Issue #3032](https://github.com/manticoresoftware/manticoresearch/issues/3032) Теперь с интеграцией Kafka можно создавать источник для конкретного раздела Kafka.
* 🪲 [v9.2.26](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.26) [ Issue #3301](https://github.com/manticoresoftware/manticoresearch/issues/3301) Исправление: `ORDER BY` и `WHERE` по `id` могли вызывать ошибки OOM (Out Of Memory).
* 🪲 [v9.2.25](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.25) [ Issue #3171](https://github.com/manticoresoftware/manticoresearch/issues/3171) Исправление: сбой из-за ошибки сегментации при использовании grouper с несколькими JSON-атрибутами в таблице RT с несколькими дисковыми чанками
* 🪲 [v9.2.24](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.24) [ Issue #3246](https://github.com/manticoresoftware/manticoresearch/issues/3246) Исправление: запросы `WHERE string ANY(...)` не работали после сброса RAM чанка.
* 🪲 [v9.2.23](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.23) [ PR #518](https://github.com/manticoresoftware/manticoresearch-buddy/pull/518) Незначительные улучшения синтаксиса авто-шардинга.
* 🪲 [v9.2.22](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.22) [ Issue #2763](https://github.com/manticoresoftware/manticoresearch/issues/2763) Исправление: глобальный файл idf не загружался при использовании `ALTER TABLE`.
* 🪲 [v9.2.21](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.21)  Исправление: глобальные файлы idf могут быть большими. Теперь таблицы освобождаются раньше, чтобы избежать удержания ненужных ресурсов.
* 🪲 [v9.2.20](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.20) [ PR #3277](https://github.com/manticoresoftware/manticoresearch/pull/3277) Улучшение: лучшая проверка параметров шардинга.

* 🪲 [v9.2.19](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.19) [ PR #3275](https://github.com/manticoresoftware/manticoresearch/pull/3275) Исправление: совместимость сборки с Boost 1.88.0.
* 🪲 [v9.2.18](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.18) [ Issue #3228](https://github.com/manticoresoftware/manticoresearch/issues/3228) Исправление: сбой при создании распределенной таблицы (проблема с недопустимым указателем).

* 🪲 [v9.2.17](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.17) [ PR #3272](https://github.com/manticoresoftware/manticoresearch/pull/3272) Исправление: проблема с многострочным нечетким `FACET`.
* 🪲 [v9.2.16](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.16) [ Issue #3063](https://github.com/manticoresoftware/manticoresearch/issues/3063) Исправление: ошибка в вычислении расстояния при использовании функции `GEODIST`.
* 🪲 [v9.2.15](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.15) [ Issue #3027](https://github.com/manticoresoftware/manticoresearch/issues/3027) Незначительное улучшение: поддержка формата фильтра `query_string` Elastic.

## Версия 9.2.14
Выпущено: 28 марта 2025

### Незначительные изменения
* [Commit ](https://github.com/manticoresoftware/manticoresearch/commit/fe9473dc3f4fcfa0eaae748b538fe853f4ce078b) Реализован флаг `--mockstack` для расчета требований к стэку рекурсивных операций. Новый режим `--mockstack` анализирует и сообщает необходимые размеры стэка для оценки рекурсивных выражений, операций сопоставления паттернов, обработки фильтров. Вычисленные требования к стеку выводятся в консоль для отладки и оптимизации.
* [Issue #3058](https://github.com/manticoresoftware/manticoresearch/pull/3058) По умолчанию включён [boolean_simplify](Searching/Options.md#boolean_simplify).
* [Issue #3172](https://github.com/manticoresoftware/manticoresearch/issues/3172) Добавлена новая опция конфигурации: `searchd.kibana_version_string`, которая может быть полезна при использовании Manticore с определёнными версиями Kibana или OpenSearch Dashboards, ожидающими конкретную версию Elasticsearch.
* [Issue #3211](https://github.com/manticoresoftware/manticoresearch/issues/3211) Исправлена работа [CALL SUGGEST](Searching/Spell_correction.md#CALL-QSUGGEST,-CALL-SUGGEST) с двухсимвольными словами.
* [Issue #490](https://github.com/manticoresoftware/manticoresearch-buddy/issues/490) Улучшен [нечеткий поиск](Searching/Spell_correction.md#Fuzzy-Search): ранее иногда он не мог найти "defghi" при поиске "def ghi", если существовал другой совпадающий документ.
* ⚠️ СЛОМАНО [Issue #3165](https://github.com/manticoresoftware/manticoresearch/issues/3165) Изменено `_id` на `id` в некоторых HTTP JSON ответах для единообразия. Обязательно обновите ваше приложение соответственно.
* ⚠️ ВАЖНОЕ ИЗМЕНЕНИЕ [Issue #3186](https://github.com/manticoresoftware/manticoresearch/issues/3186) Добавлена проверка `server_id` при объединении в кластер, чтобы обеспечить уникальность ID каждого узла. Операция `JOIN CLUSTER` теперь может завершиться ошибкой с сообщением о дублировании [server_id](Server_settings/Searchd.md#server_id), если присоединяемый узел имеет тот же `server_id`, что и уже существующий в кластере. Чтобы решить эту проблему, убедитесь, что у каждого узла в кластере репликации свой уникальный [server_id](Server_settings/Searchd.md#server_id). Вы можете изменить стандартный [server_id](Server_settings/Searchd.md#server_id) в разделе "searchd" вашего конфигурационного файла на уникальное значение перед попыткой присоединения к кластеру. Это изменение обновляет протокол репликации. Если у вас работает кластер репликации, необходимо:
  - Сначала аккуратно остановить все ваши узлы
  - Затем запустить последний остановленный узел с параметром `--new-cluster`, используя инструмент `manticore_new_cluster` в Linux.
  - Подробнее читайте о [перезапуске кластера](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster).

### Исправления багов
* [Коммит 6fda](https://github.com/manticoresoftware/manticoresearch/commit/6fdad3923dd6953f4b781943eed5ec5c28b7f808) Исправлен сбой, вызванный потерей планировщика после ожидания; теперь конкретные планировщики, такие как `serializer`, правильно восстанавливаются.
* [Коммит c333](https://github.com/manticoresoftware/manticoresearch/commit/c333b277d4a504de5fe298e9ab570a9f9ea31e0a) Исправлена ошибка, при которой веса из правой присоединённой таблицы не могли использоваться в разделе `ORDER BY`.
* [Issue #2644](https://github.com/manticoresoftware/manticoresearch/issues/2644) gcc 14.2.0: исправлена ошибка вызова `lower_bound` с `const knn::DocDist_t*&`. ❤️ Спасибо [@Azq2](https://github.com/Azq2) за PR.
* [Issue #3018](https://github.com/manticoresoftware/manticoresearch/issues/3018) Исправлена проблема обработки имен таблиц в верхнем регистре при вставках по авто-схеме.
* [Issue #3119](https://github.com/manticoresoftware/manticoresearch/issues/3119) Исправлен сбой при декодировании некорректного base64 ввода.
* [Issue #3121](https://github.com/manticoresoftware/manticoresearch/issues/3121) Исправлены две связанные проблемы с индексом KNN на `ALTER`: теперь вещественные векторы сохраняют исходные размеры, а KNN индексы корректно генерируются.
* [Issue #3123](https://github.com/manticoresoftware/manticoresearch/issues/3123) Исправлен сбой при создании вторичного индекса на пустом JSON столбце.
* [Issue #3138](https://github.com/manticoresoftware/manticoresearch/issues/3138) Исправлен сбой из-за дублирующихся записей.
* [Issue #3151](https://github.com/manticoresoftware/manticoresearch/issues/3151) Исправлено: опция `fuzzy=1` не могла использоваться с `ranker` или `field_weights`.
* [Issue #3163](https://github.com/manticoresoftware/manticoresearch/issues/3163) Исправлена ошибка, из-за которой `SET GLOBAL timezone` не оказывал эффекта.
* [Issue #3181](https://github.com/manticoresoftware/manticoresearch/issues/3181) Исправлена проблема, из-за которой значения текстовых полей могли теряться при использовании ID больше 2^63.
* [Issue #3189](https://github.com/manticoresoftware/manticoresearch/issues/3189) Исправлено: операторы `UPDATE` теперь корректно учитывают настройку `query_log_min_msec`.
* [Issue #3247](https://github.com/manticoresoftware/manticoresearch/issues/3247) Исправлена гонка при сохранении реального времени дисковых чанков, из-за которой могла неудачно проходить команда `JOIN CLUSTER`.


## Версия 7.4.6
Выпущена: 28 февраля 2025

### Основные изменения
* [Issue #832](https://github.com/manticoresoftware/manticoresearch/issues/832) Интеграция с [Kibana](Integration/Kibana.md) для более лёгкой и эффективной визуализации данных.

### Незначительные изменения
* [Issue #1727](https://github.com/manticoresoftware/manticoresearch/issues/1727) Исправлены различия в точности с плавающей точкой между arm64 и x86_64.
* [Issue #2995](https://github.com/manticoresoftware/manticoresearch/issues/2995) Реализованы оптимизации производительности для пакетной обработки join.
* [Issue #3039](https://github.com/manticoresoftware/manticoresearch/issues/3039) Реализованы оптимизации производительности для EstimateValues в гистограммах.
* [Issue #3099](https://github.com/manticoresoftware/manticoresearch/issues/3099) Добавлена поддержка Boost 1.87.0. ❤️ Спасибо, [@cho-m](https://github.com/cho-m) за PR.
* [Issue #77](https://github.com/manticoresoftware/columnar/issues/77) Оптимизировано повторное использование блоков данных при создании фильтров с несколькими значениями; добавлены min/max в метаданные атрибутов; реализована предварительная фильтрация значений фильтра на основе min/max.

### Исправления багов
* [Коммит 73ac](https://github.com/manticoresoftware/manticoresearch/commit/73ac22f358a7a0e734b332c3943c86e6294c06c3) Исправлена обработка выражений в присоединённых запросах при использовании атрибутов из обеих таблиц; исправлена опция index_weights для правой таблицы.
* [Issue #2915](https://github.com/manticoresoftware/manticoresearch/issues/2915) Использование `avg()` в запросах `SELECT ... JOIN` могло приводить к некорректным результатам; теперь исправлено.
* [Issue #2996](https://github.com/manticoresoftware/manticoresearch/issues/2996) Исправлен неправильный набор результатов, вызванный неявным ограничением при включенной пакетной обработке join.
* [Issue #3031](https://github.com/manticoresoftware/manticoresearch/issues/3031) Исправлен сбой демона при завершении работы во время активного слияния чанков.
* [Issue #3037](http://github.com/manticoresoftware/manticoresearch/issues/3037) Исправлена проблема, когда `IN(...)` мог выдавать некорректные результаты.
* [Issue #3038](https://github.com/manticoresoftware/manticoresearch/issues/3038) Установка `max_iops` / `max_iosize` в версии 7.0.0 могла ухудшать производительность индексирования; теперь исправлено.
* [Issue #3042](https://github.com/manticoresoftware/manticoresearch/issues/3042) Исправлена утечка памяти в кэше запросов join.
* [Issue #3052](https://github.com/manticoresoftware/manticoresearch/issues/3052) Исправлена обработка опций запросов в присоединённых JSON-запросах.
* [Issue #3054](https://github.com/manticoresoftware/manticoresearch/issues/3054) Исправлена проблема с командой ATTACH TABLE.
* [Issue #3079](https://github.com/manticoresoftware/manticoresearch/issues/3079) Исправлены несоответствия в сообщениях об ошибках.
* [Issue #3087](https://github.com/manticoresoftware/manticoresearch/issues/3087) Установка `diskchunk_flush_write_timeout=-1` для каждой таблицы не отключала сброс индекса; теперь это исправлено.
* [Issue #3088](https://github.com/manticoresoftware/manticoresearch/issues/3088) Решена проблема с дублированием записей после массовой замены больших идентификаторов.
* [Issue #3126](https://github.com/manticoresoftware/manticoresearch/issues/3126) Исправлен сбой демона, вызванный полнотекстовым запросом с одним оператором `NOT` и ранжировщиком выражений.
* [Issue #3128](https://github.com/manticoresoftware/manticoresearch/pull/3128) Исправлена потенциальная уязвимость в библиотеке CJSON. ❤️ Спасибо, [@tabudz](https://github.com/tabudz) за PR.

## Версия 7.0.0
Выпущена: 30 января 2025

### Основные изменения
* [Issue #1497](https://github.com/manticoresoftware/manticoresearch/issues/1497) Добавлена новая функциональность [Fuzzy Search](Searching/Spell_correction.md#Fuzzy-Search) и [Autocomplete](Searching/Autocomplete.md#CALL-AUTOCOMPLETE) для облегчения поиска.
* [Issue #1500](https://github.com/manticoresoftware/manticoresearch/issues/1500) [Интеграция с Kafka](Integration/Kafka.md#Syncing-from-Kafka).
* [Issue #1928](https://github.com/manticoresoftware/manticoresearch/issues/1928) Введены [вторичные индексы для JSON](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#json_secondary_indexes).
* [Issue #2361](https://github.com/manticoresoftware/manticoresearch/issues/2361) Обновления и поиски во время обновлений больше не блокируются слиянием чанков.
* [Issue #2787](https://github.com/manticoresoftware/manticoresearch/issues/2787) Автоматический [сброс дисковых чанков](Server_settings/Searchd.md#diskchunk_flush_write_timeout) для RT таблиц для улучшения производительности; теперь мы автоматически сбрасываем RAM-чанки на дисковые чанки, предотвращая проблемы с производительностью, вызванные отсутствием оптимизаций в RAM-чанках, что иногда приводило к нестабильности в зависимости от размера чанка.
* [Issue #2811](https://github.com/manticoresoftware/manticoresearch/issues/2811) Опция [Scroll](Searching/Pagination.md#Scroll-Search-Option) для облегчения постраничного вывода.
* [Issue #931](https://github.com/manticoresoftware/manticoresearch/issues/931) Интеграция с [Jieba](https://github.com/fxsjy/jieba) для лучшей [токенизации китайского языка](Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md).

### Незначительные изменения
* ⚠️ НЕСОВМЕСТИМО [Issue #1111](https://github.com/manticoresoftware/manticoresearch/issues/1111) Исправлена поддержка `global_idf` в RT таблицах. Требуется пересоздание таблиц.
* ⚠️ НЕСОВМЕСТИМО [Issue #2103](https://github.com/manticoresoftware/manticoresearch/issues/2103) Удалены тайские символы из внутреннего набора символов `cjk`. Обновите определения наборов символов соответственно: если у вас есть `cjk,non_cjk` и тайские символы важны для вас, замените на `cjk,thai,non_cjk` или `cont,non_cjk`, где `cont` — новое обозначение для всех языков с непрерывными скриптами (то есть `cjk` + `thai`). Измените существующие таблицы с помощью [ALTER TABLE](Updating_table_schema_and_settings.md#Updating-table-FT-settings-in-RT-mode).
* ⚠️ НЕСОВМЕСТИМО [Issue #2468](https://github.com/manticoresoftware/manticoresearch/issues/2468) [CALL SUGGEST / QSUGGEST](Searching/Spell_correction.md#CALL-QSUGGEST,-CALL-SUGGEST) теперь совместимы с распределенными таблицами. Это увеличивает версию протокола мастер/агент. Если вы используете Manticore Search в распределенной среде с несколькими инстансами, сначала обновите агенты, затем мастеров.
* ⚠️ НЕСОВМЕСТИМО [Issue #2889](https://github.com/manticoresoftware/manticoresearch/issues/2889) Изменено имя столбца с `Name` на `Variable name` для PQ [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META-for-PQ-tables).
* ⚠️ НЕСОВМЕСТИМО [Issue #879](https://github.com/manticoresoftware/manticoresearch/issues/879) Введён [binlog на таблицу](Logging/Binary_logging.md#Per-table-binary-logging-configuration) с новыми опциями: [binlog_common](Logging/Binary_logging.md#Binary-logging-strategies), [binlog](Logging/Binary_logging.md#Per-table-binary-logging-configuration) для `create table` / `alter table`. Перед обновлением на новую версию необходимо корректно завершить работу экземпляра Manticore.
* ⚠️ НЕСОВМЕСТИМО [Issue #1789](https://github.com/manticoresoftware/manticoresearch/issues/1789) Исправлено некорректное сообщение об ошибке при присоединении ноды к кластеру с неправильной версией протокола репликации. Это изменение обновляет протокол репликации. Если у вас репликационный кластер, нужно:
  - Сначала корректно остановить все ноды
  - Затем запустить последнюю остановленную ноду с `--new-cluster`, используя инструмент `manticore_new_cluster` в Linux.
  - Подробнее о [перезапуске кластера](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster).
* ⚠️ НЕСОВМЕСТИМО [Issue #2308](https://github.com/manticoresoftware/manticoresearch/issues/2308) Добавлена поддержка нескольких таблиц в [`ALTER CLUSTER ADD` и `DROP`](Creating_a_cluster/Setting_up_replication/Adding_and_removing_a_table_from_a_replication_cluster.md#Adding-and-removing-a-table-from-a-replication-cluster). Это изменение также влияет на протокол репликации. См. предыдущий раздел для рекомендаций по обновлению.
* [Issue #2997](https://github.com/manticoresoftware/manticoresearch/issues/2997) Исправлена проблема с dlopen на MacOS.
* [Commit 4954](https://github.com/manticoresoftware/manticoresearch/commit/4954b5de7341a29902a9b8fbb9a040f7942c77c4) Изменено значение по умолчанию cutoff для OPTIMIZE TABLE в таблицах с KNN индексами для улучшения производительности поиска.
* [Commit cfc8](https://github.com/manticoresoftware/manticoresearch/commit/cfc87ecb6e33a8163c2235243b6b40e699dbf526) Добавлена поддержка `COUNT(DISTINCT)` для `ORDER BY` в `FACET` и `GROUP BY`.
* [Issue #1103](https://github.com/manticoresoftware/manticoresearch/issues/1103) Улучшена ясность сообщений в [логировании](Logging/Server_logging.md#Server-logging) слияния чанков.
* [Issue #1130](https://github.com/manticoresoftware/manticoresearch/issues/1130) Добавлена поддержка для [DBeaver](Integration/DBeaver.md).
* [Issue #1546](https://github.com/manticoresoftware/manticoresearch/issues/1546) Реализованы вторичные индексы для функций [POLY2D()](Functions/Geo_spatial_functions.md#POLY2D%28%29)/[GEOPOLY2D()](Functions/Geo_spatial_functions.md#GEOPOLY2D%28%29).
* [Issue #1630](https://github.com/manticoresoftware/manticoresearch/issues/1630) HTTP-запросы теперь поддерживают `Content-Encoding: gzip`.
* [Issue #1831](https://github.com/manticoresoftware/manticoresearch/issues/1831) Добавлена команда `SHOW LOCKS`.
* [Issue #2187](https://github.com/manticoresoftware/manticoresearch/issues/2197) Разрешен запрос Buddy к демону обходить ограничение [searchd.max_connections](Server_settings/Searchd.md#max_connections).
* [Issue #2208](https://github.com/manticoresoftware/manticoresearch/issues/2208) Добавлена поддержка объединения таблиц через JSON HTTP интерфейс.
* [Issue #2235](https://github.com/manticoresoftware/manticoresearch/issues/2235) Логирование успешно обработанных запросов через Buddy в их оригинальной форме.
* [Issue #2249](https://github.com/manticoresoftware/manticoresearch/issues/2249) Добавлен специальный режим для запуска `mysqldump` для реплицируемых таблиц.
* [Issue #2268](https://github.com/manticoresoftware/manticoresearch/issues/2268) Улучшено переименование внешних файлов при копировании для операторов `CREATE TABLE` и `ALTER TABLE`.
* [Issue #2402](https://github.com/manticoresoftware/manticoresearch/issues/2402) Обновлено значение по умолчанию для [searchd.max_packet_size](Server_settings/Searchd.md#max_packet_size) до 128MB.
* [Issue #2419](https://github.com/manticoresoftware/manticoresearch/issues/2419) Добавлена поддержка [IDF boost modifier](Searching/Full_text_matching/Operators.md#IDF-boost-modifier) в JSON ["match"](Searching/Full_text_matching/Basic_usage.md#match).
* [Issue #2430](https://github.com/manticoresoftware/manticoresearch/issues/2430) Усилена синхронизация записи [binlog](Logging/Binary_logging.md#Binary-logging) для предотвращения ошибок.
* [Issue #2458](https://github.com/manticoresoftware/manticoresearch/issues/2458) Включена поддержка zlib в пакетах для Windows.
* [Issue #2479](https://github.com/manticoresoftware/manticoresearch/issues/2479) Добавлена поддержка команды SHOW TABLE INDEXES.
* [Issue #2485](https://github.com/manticoresoftware/manticoresearch/issues/2485) Установлены метаданные сессии для ответов Buddy.
* [Issue #2490](https://github.com/manticoresoftware/manticoresearch/issues/2490) Разрешение в миллисекундах для агрегатов на точках совместимости.
* [Issue #2500](https://github.com/manticoresoftware/manticoresearch/issues/2500) Изменены сообщения об ошибках для операций с кластером при сбое запуска репликации.
* [Issue #2584](https://github.com/manticoresoftware/manticoresearch/pull/2584) Новые [метрики производительности в SHOW STATUS](Node_info_and_management/Node_status.md#Query-Time-Statistics): минимума/максимума/среднего/95-го/99-го перцентиля по типу запроса за последние 1, 5 и 15 минут.
* [Issue #2639](https://github.com/manticoresoftware/manticoresearch/pull/2639) Во всех запросах и ответах заменено `index` на `table`.
* [Issue #2643](https://github.com/manticoresoftware/manticoresearch/issues/2643) Добавлен столбец `distinct` в результаты агрегатов HTTP `/sql` эндпоинта.
* [Issue #268](https://github.com/manticoresoftware/manticoresearch-buddy/issues/268) Реализовано автодетектирование типов данных при импорте из Elasticsearch.
* [Issue #2744](https://github.com/manticoresoftware/manticoresearch/issues/2744) Добавлена поддержка_COLLATION для выражений сравнения строковых JSON-полей.
* [Issue #2752](https://github.com/manticoresoftware/manticoresearch/issues/2752) Добавлена поддержка выражения `uuid_short` в списке select.
* [Issue #2783](https://github.com/manticoresoftware/manticoresearch/issues/2783) Manticore Search теперь запускает Buddy напрямую без обертки `manticore-buddy`.
* [Issue #2785](https://github.com/manticoresoftware/manticoresearch/issues/2785) Различаются сообщения об ошибках для отсутствующих таблиц и таблиц, не поддерживающих операции вставки.
* [Issue #2789](https://github.com/manticoresoftware/manticoresearch/issues/2789) OpenSSL 3 теперь статически встроен в `searchd`.
* [Issue #2790](https://github.com/manticoresoftware/manticoresearch/issues/2790) Добавлена инструкция `CALL uuid_short` для генерации последовательностей с несколькими значениями `uuid_short`.
* [Issue #2803](https://github.com/manticoresoftware/manticoresearch/issues/2803) Добавлены отдельные опции для правой таблицы в операции JOIN.
* [Issue #2810](https://github.com/manticoresoftware/manticoresearch/issues/2810) Повышена производительность HTTP JSON агрегатов до уровня `GROUP BY` в SphinxQL.
* [Issue #2854](https://github.com/manticoresoftware/manticoresearch/issues/2854) Добавлена поддержка `fixed_interval` в запросах по датам Kibana.
* [Issue #2909](https://github.com/manticoresoftware/manticoresearch/issues/2909) Реализована пакетная обработка для JOIN-запросов, что улучшает производительность некоторых JOIN-запросов в сотни или даже тысячи раз.
* [Issue #2937](https://github.com/manticoresoftware/manticoresearch/issues/2937) Включено использование веса присоединенной таблицы в fullscan-запросах.
* [Issue #2953](https://github.com/manticoresoftware/manticoresearch/issues/2953) Исправлено логирование для join-запросов.
* [Issue #337](https://github.com/manticoresoftware/manticoresearch-buddy/issues/337) Исключения Buddy скрыты из лога `searchd` в режиме без отладки.
* [Issue #2931](https://github.com/manticoresoftware/manticoresearch/issues/2931) Завершение работы демона с сообщением об ошибке, если пользователь устанавливает неправильные порты для слушателя репликации.

### Исправления ошибок
* [Commit 0c6b](https://github.com/manticoresoftware/manticoresearch/commit/0c6bdaf41ea3684e05952e1a8273893535b923f5) Исправлено: Неверные результаты в JOIN-запросах с более чем 32 столбцами.
* [Issue #2335](https://github.com/manticoresoftware/manticoresearch/issues/2335) Исправлена ошибка, при которой объединение таблиц не работало при использовании двух json-атрибутов в условии.
* [Issue #2338](https://github.com/manticoresoftware/manticoresearch/issues/2338) Исправлено некорректное total_relation в мультизапросах с [cutoff](Searching/Options.md#cutoff).
* [Issue #2366](https://github.com/manticoresoftware/manticoresearch/issues/2366) Исправлена фильтрация по `json.string` в правой таблице при [объединении таблиц](Searching/Joining.md).
* [Issue #2406](https://github.com/manticoresoftware/manticoresearch/issues/2406) Добавлена возможность использовать `null` для всех значений в любом POST HTTP JSON эндпоинте (insert/replace/bulk). В этом случае используется значение по умолчанию.
* [Issue #2418](https://github.com/manticoresoftware/manticoresearch/issues/2418) Оптимизировано потребление памяти за счёт настройки выделения [max_packet_size](Server_settings/Searchd.md#max_packet_size) сетевого буфера для первоначального опроса сокета.
* [Issue #2420](https://github.com/manticoresoftware/manticoresearch/issues/2420) Исправлена вставка unsigned int в bigint атрибут через JSON интерфейс.
* [Issue #2422](https://github.com/manticoresoftware/manticoresearch/issues/2422) Исправлена работа вторичных индексов с exclude фильтрами и включённым pseudo_sharding.
* [Issue #2423](https://github.com/manticoresoftware/manticoresearch/issues/2423) Исправлена ошибка в [manticore_new_cluster](Starting_the_server/Manually.md#searchd-command-line-options).
* [Issue #2448](https://github.com/manticoresoftware/manticoresearch/issues/2448) Исправлен крах демона при некорректном запросе `_update`.
* [Issue #2452](https://github.com/manticoresoftware/manticoresearch/issues/2452) Исправлена невозможность обработки гистограммами value фильтров с исключениями.
* [Issue #55](https://github.com/manticoresoftware/columnar/issues/55) Исправлены [knn](Searching/KNN.md#KNN-vector-search) запросы к распределённым таблицам.
* [Issue #68](https://github.com/manticoresoftware/columnar/issues/68) Улучшена обработка exclude фильтров при кодировании таблиц в columnar accessor.
* [Commit 0eb1](https://github.com/manticoresoftware/manticoresearch/commit/0eb1579462013cb134bcaf74e06f6eeb0c2ecc4d) Исправлен парсер выражений, не учитывающий переопределённый `thread_stack`.
* [Commit c304](https://github.com/manticoresoftware/manticoresearch/commit/c3041fd1d3e14773f3c5aa635343915a71f5cb95) Исправлен крах при клонировании columnar IN выражения.
* [Commit edad](https://github.com/manticoresoftware/columnar/commit/edadc694c68d6022bdd13134263667430a42cc1d) Исправлена ошибка инверсии в итераторе битовой карты, которая вызывала крах.
* [Commit fc30](https://github.com/manticoresoftware/executor/commit/fc302ff1117b0b835a6f5a3c07057baf5fba14d9) Исправлена проблема, когда некоторые пакеты Manticore автоматически удалялись `unattended-upgrades`.
* [Issue #1019](https://github.com/manticoresoftware/manticoresearch/issues/1019) Улучшена обработка запросов из инструмента DbForge MySQL.
* [Issue #1107](https://github.com/manticoresoftware/manticoresearch/issues/1107) Исправлено экранирование специальных символов в `CREATE TABLE` и `ALTER TABLE`. ❤️ Спасибо, [@subnix](https://github.com/subnix) за PR.
* [Issue #116](https://github.com/manticoresoftware/manticoresearch-backup/issues/116#issuecomment-2216597206) Исправлена взаимоблокировка при обновлении blob атрибута в замороженном индексе. Взаимоблокировка возникала из-за конфликтующих блокировок при попытке разморозить индекс. Это также могло привести к сбою в manticore-backup.
* [Issue #1818](https://github.com/manticoresoftware/manticoresearch/issues/1818) `OPTIMIZE` теперь выдаёт ошибку, если таблица заморожена.
* [Issue #2001](https://github.com/manticoresoftware/manticoresearch/issues/2001) Разрешено использовать имена функций в качестве имён столбцов.
* [Issue #2153](https://github.com/manticoresoftware/manticoresearch/issues/2153) Исправлен крах демона при запросе настроек таблицы с неизвестным дисковым чанком.
* [Issue #2184](https://github.com/manticoresoftware/manticoresearch/issues/2184) Исправлена проблема, из-за которой `searchd` зависал при остановке после `FREEZE` и `FLUSH RAMCHUNK`.
* [Issue #2228](https://github.com/manticoresoftware/manticoresearch/issues/2228) Удалены токены даты/времени (и regex) из списка зарезервированных слов.
* [Issue #2255](https://github.com/manticoresoftware/manticoresearch/issues/2255) Исправлен крах при `FACET` с более чем 5 полями сортировки.
* [Issue #2265](https://github.com/manticoresoftware/manticoresearch/issues/2265) Исправлен сбой восстановления `mysqldump` с включённым `index_field_lengths`.
* [Issue #2291](https://github.com/manticoresoftware/manticoresearch/issues/2291) Исправлен крах при выполнении команды `ALTER TABLE`.
* [Issue #2333](https://github.com/manticoresoftware/manticoresearch/issues/2333) Исправлена DLL MySQL в Windows пакете для корректной работы индексатора.
* [Issue #2393](https://github.com/manticoresoftware/manticoresearch/issues/2393) Исправлена ошибка компиляции GCC. ❤️ Спасибо, [@animetosho](https://github.com/animetosho) за PR.
* [Issue #2447](https://github.com/manticoresoftware/manticoresearch/issues/2447) Исправлена ошибка экранирования в [_update](Data_creation_and_modification/Updating_documents/REPLACE.md?client=Elasticsearch-like+partial).
* [Issue #2460](https://github.com/manticoresoftware/manticoresearch/issues/2460) Исправлен крах индексатора при объявлении нескольких атрибутов или полей с одинаковым именем.
* [Issue #2467](https://github.com/manticoresoftware/manticoresearch/issues/2467) Исправлен крах демона при некорректной трансформации вложенных bool запросов для совместимых search endpoint'ов.
* [Issue #2493](https://github.com/manticoresoftware/manticoresearch/issues/2493) Исправлено расширение в фразах с модификаторами.
* [Issue #2535](https://github.com/manticoresoftware/manticoresearch/issues/2535) Исправлен крах демона при использовании полнотекстового оператора [ZONE](Searching/Full_text_matching/Operators.md#ZONE-limit-operator) или [ZONESPAN](Searching/Full_text_matching/Operators.md#ZONESPAN-limit-operator).
* [Issue #2552](https://github.com/manticoresoftware/manticoresearch/issues/2552) Исправлена генерация инфикса для plain и RT таблиц с использованием словарей ключевых слов.
* [Issue #2571](https://github.com/manticoresoftware/manticoresearch/issues/2571) Исправлен ответ с ошибкой в запросе `FACET`; установлен порядок сортировки по умолчанию `DESC` для `FACET` с `COUNT(*)`.
* [Issue #2580](https://github.com/manticoresoftware/manticoresearch/issues/2580) Исправлен сбой демона на Windows при запуске.
* [Issue #2603](https://github.com/manticoresoftware/manticoresearch/issues/2603) Исправлено усечение запроса для HTTP endpoint'ов `/sql` и `/sql?mode=raw`; сделаны запросы с этих endpoint'ов консистентными без необходимости использования заголовка `query=`.
* [Issue #2623](https://github.com/manticoresoftware/manticoresearch/issues/2623) Исправлена проблема, при которой auto-schema создаёт таблицу, но одновременно происходит сбой.
* [Issue #2627](https://github.com/manticoresoftware/manticoresearch/issues/2627) Исправлена библиотека HNSW для поддержки загрузки нескольких индексов KNN.
* [Issue #2630](https://github.com/manticoresoftware/manticoresearch/issues/2630) Исправлена проблема зависания при одновременном возникновении нескольких условий.
* [Issue #2645](https://github.com/manticoresoftware/manticoresearch/issues/2645) Исправлен сбой с фатальной ошибкой при использовании `OR` с поиском KNN.
* [Issue #2647](https://github.com/manticoresoftware/manticoresearch/issues/2647) Исправлена ошибка, при которой `indextool --mergeidf *.idf --out global.idf` удалял выходной файл после его создания.
* [Issue #2658](https://github.com/manticoresoftware/manticoresearch/issues/2658) Исправлен сбой демона при подзапросе с `ORDER BY` строкой во внешнем select.
* [Issue #2686](https://github.com/manticoresoftware/manticoresearch/issues/2686) Исправлен сбой при обновлении float-атрибута вместе со строковым атрибутом.
* [Issue #2704](https://github.com/manticoresoftware/manticoresearch/issues/2704) Исправлена проблема, при которой несколько стоп-слов из токенизаторов `lemmatize_xxx_all` увеличивают `hitpos` у последующих токенов.
* [Issue #2708](https://github.com/manticoresoftware/manticoresearch/issues/2708) Исправлен сбой при `ALTER ... ADD COLUMN ... TEXT`.
* [Issue #2737](https://github.com/manticoresoftware/manticoresearch/issues/2737) Исправлена проблема, при которой обновление blob-атрибута в замороженной таблице с хотя бы одним RAM-чанком заставляет последующие `SELECT`-запросы ждать пока таблица не разморозится.
* [Issue #2742](https://github.com/manticoresoftware/manticoresearch/issues/2742) Исправлен пропуск кэша запросов для запросов с упакованными факторами.
* [Issue #2775](https://github.com/manticoresoftware/manticoresearch/issues/2775) Manticore теперь сообщает об ошибке при неизвестном действии вместо сбоя на запросах `_bulk`.
* [Issue #2791](https://github.com/manticoresoftware/manticoresearch/issues/2791) Исправлен возврат ID вставленного документа для HTTP `_bulk` endpoint.
* [Issue #2797](https://github.com/manticoresoftware/manticoresearch/issues/2797) Исправлен сбой в grouper при обработке нескольких таблиц, одна из которых пустая, а другая содержит разное количество совпадающих записей.
* [Issue #2835](https://github.com/manticoresoftware/manticoresearch/issues/2835) Исправлен сбой в сложных запросах `SELECT`.
* [Issue #2872](https://github.com/manticoresoftware/manticoresearch/issues/2872) Добавлено сообщение об ошибке, если аргумент `ALL` или `ANY` в выражении `IN` не является JSON-атрибутом.
* [Issue #2882](https://github.com/manticoresoftware/manticoresearch/issues/2882) Исправлен сбой демона при обновлении MVA в больших таблицах.
* [Issue #2888](https://github.com/manticoresoftware/manticoresearch/issues/2888) Исправлен сбой при ошибке токенизации с `libstemmer`.  ❤️ Спасибо, [@subnix](https://github.com/subnix) за PR.
* [Issue #2919](https://github.com/manticoresoftware/manticoresearch/issues/2919) Исправлена проблема, при которой вес из правой таблицы в join не корректно работал в выражениях.
* [Issue #2919](https://github.com/manticoresoftware/manticoresearch/issues/2919) Исправлена проблема, при которой вес в правой join-таблице не работает в выражениях.
* [Issue #325](https://github.com/manticoresoftware/manticoresearch-buddy/issues/325) Исправлен сбой `CREATE TABLE IF NOT EXISTS ... WITH DATA`, если таблица уже существует.
* [Issue #351](https://github.com/manticoresoftware/manticoresearch-buddy/issues/351) Исправлена ошибка "undefined array key 'id'" при подсчёте по KNN с указанием ID документа.
* [Issue #359](https://github.com/manticoresoftware/manticoresearch-buddy/issues/359) Исправлена работа `REPLACE INTO cluster_name:table_name`.
* [Issue #67](https://github.com/manticoresoftware/docker/issues/67) Исправлена фатальная ошибка при запуске Docker-контейнера Manticore с `--network=host`.

## Version 6.3.8
Выпуск: 22 ноября 2024

Версия 6.3.8 продолжает серию 6.3 и включает только исправления ошибок.

### Исправления ошибок

* [PR #2777](https://github.com/manticoresoftware/manticoresearch/pull/2777) Исправлен расчёт доступных потоков, когда ограничена параллельность запросов настройками `threads` или `max_threads_per_query`.

## manticore-extra v1.1.20

Выпуск: 7 октябрь 2024

### Исправления ошибок

* [Issue #64](https://github.com/manticoresoftware/executor/issues/64) Исправлена проблема, когда утилита `unattended-upgrades`, которая автоматически устанавливает обновления пакетов на системах на базе Debian, неправильно помечала несколько пакетов Manticore, включая `manticore-galera`, `manticore-executor` и `manticore-columnar-lib` для удаления. Это происходило из-за того, что `dpkg` ошибочно считал виртуальный пакет `manticore-extra` избыточным. Были внесены изменения, чтобы утилита `unattended-upgrades` больше не пыталась удалить важные компоненты Manticore.

## Version 6.3.6
Выпуск: 2 августа 2024

Версия 6.3.6 продолжает серию 6.3 и включает только исправления ошибок.

### Исправления ошибок

* [Issue #2477](https://github.com/manticoresoftware/manticoresearch/issues/2477) Исправлен сбой, возникший в версии 6.3.4, который мог происходить при работе с выражениями и распределёнными или множественными таблицами.
* [Issue #2352](https://github.com/manticoresoftware/manticoresearch/issues/2352) Исправлен сбой демона или внутренняя ошибка при преждевременном выходе, вызванном `max_query_time`, при запросах к нескольким индексам.

## Version 6.3.4
Выпуск: 31 июля 2024

Версия 6.3.4 продолжает серию 6.3 и включает только незначительные улучшения и исправления ошибок.

### Незначительные изменения
* [Issue #1130](https://github.com/manticoresoftware/manticoresearch/issues/1130) Добавлена поддержка для [DBeaver](Integration/DBeaver.md#Integration-with-DBeaver).
* [Issue #2146](https://github.com/manticoresoftware/manticoresearch/issues/2146) Улучшено экранирование разделителей в формах слов и исключениях.
* [Issue #2315](https://github.com/manticoresoftware/manticoresearch/issues/2315) Добавлены операторы сравнения строк в выражения списка SELECT.
* [Issue #2363](https://github.com/manticoresoftware/manticoresearch/issues/2363) Добавлена поддержка null значений в пакетных запросах в стиле Elastic.
* [Issue #2374](https://github.com/manticoresoftware/manticoresearch/issues/2374) Добавлена поддержка mysqldump версии 9.
* [Issue #2375](https://github.com/manticoresoftware/manticoresearch/issues/2375) Улучшена обработка ошибок в HTTP JSON запросах с JSON путем к узлу, в котором происходит ошибка.

### Исправления ошибок
* [Issue #2280](https://github.com/manticoresoftware/manticoresearch/issues/2280) Исправлено снижение производительности при запросах с шаблонами (wildcard) с большим количеством совпадений, когда disk_chunks > 1.
* [Issue #2332](https://github.com/manticoresoftware/manticoresearch/issues/2332) Исправлен сбой при использовании MIN или MAX в SELECT списках по пустым MVA массивам.
* [Issue #2339](https://github.com/manticoresoftware/manticoresearch/issues/2339) Исправлена неправильная обработка запроса с бесконечным диапазоном Kibana.
* [Issue #2342](https://github.com/manticoresoftware/manticoresearch/issues/2342) Исправлен фильтр соединения по столбцовым атрибутам из правой таблицы, если атрибут не включён в список SELECT.
* [Issue #2343](https://github.com/manticoresoftware/manticoresearch/issues/2343) Исправлено дублирование спецификатора 'static' в Manticore 6.3.2.
* [Issue #2344](https://github.com/manticoresoftware/manticoresearch/issues/2344) Исправлен LEFT JOIN, возвращающий несовпадающие записи при использовании MATCH() по правой таблице.
* [Issue #2350](https://github.com/manticoresoftware/manticoresearch/issues/2350) Исправлено сохранение disk chunk в RT индексе с параметром `hitless_words`.
* [Issue #2364](https://github.com/manticoresoftware/manticoresearch/issues/2364) Свойство 'aggs_node_sort' теперь можно добавлять в любом порядке среди прочих свойств.
* [Issue #2368](https://github.com/manticoresoftware/manticoresearch/issues/2368) Исправлена ошибка порядка фильтрации full-text по сравнению с фильтром в JSON запросе.
* [Issue #2376](https://github.com/manticoresoftware/manticoresearch/issues/2376) Исправлена ошибка, связанная с некорректным JSON ответом для длинных UTF-8 запросов.
* [Issue #2684](https://github.com/manticoresoftware/dev/issues/2684) Исправлен расчёт выражений presort/prefilter, зависящих от присоединённых атрибутов.
* [Issue #301](https://github.com/manticoresoftware/manticoresearch-buddy/issues/301) Изменён метод вычисления размера данных для метрик: теперь читается из файла `manticore.json`, а не проверяется общий размер каталога данных.
* [Issue #302](https://github.com/manticoresoftware/manticoresearch-buddy/issues/302) Добавлена обработка запросов валидации от Vector.dev.

## Версия 6.3.2
Выпущена: 26 июня 2024

Версия 6.3.2 продолжает серию 6.3 и включает несколько исправлений ошибок, часть из которых обнаружена после выпуска 6.3.0.

### Ломающие изменения
* ⚠️[Issue #2305](https://github.com/manticoresoftware/manticoresearch/issues/2305) Обновлены значения aggs.range на числовые.

### Исправления ошибок
* [Commit c51c](https://github.com/manticoresoftware/manticoresearch/commit/c51c7fa4bd0ec4ad8d53188b888461bff29f1417) Исправлена группировка по проверке stored check vs rset merge.
* [Commit 0e85](https://github.com/manticoresoftware/manticoresearch/commit/0e85b9d5682f51681750d798aef6a11cfb93d69e) Исправлен сбой демона при запросах с шаблонами в RT индексе с использованием CRC словаря и включённым `local_df`.
* [Issue #2200](https://github.com/manticoresoftware/manticoresearch/issues/2200) Исправлен сбой при JOIN с `count(*)` без GROUP BY.
* [Issue #2201](https://github.com/manticoresoftware/manticoresearch/issues/2201) Исправлено отсутствие предупреждения при попытке группировки по полнотекстовому полю в JOIN.
* [Issue #2230](https://github.com/manticoresoftware/manticoresearch/issues/2230) Исправлено, что добавление атрибута через `ALTER TABLE` не учитывало опции KNN.
* [Issue #2231](https://github.com/manticoresoftware/manticoresearch/issues/2231) Исправлена ошибка удаления пакета `manticore-tools` для Redhat в версии 6.3.0.
* [Issue #2242](https://github.com/manticoresoftware/manticoresearch/issues/2242) Исправлены проблемы с JOIN и множественными выражениями FACET, возвращающими некорректные результаты.
* [Issue #2250](https://github.com/manticoresoftware/manticoresearch/issues/2250) Исправлена ошибка ALTER TABLE, возникающая если таблица находится в кластере.
* [Issue #2252](https://github.com/manticoresoftware/manticoresearch/issues/2252) Исправлена передача исходного запроса в buddy из интерфейса SphinxQL.
* [Issue #2267](https://github.com/manticoresoftware/manticoresearch/issues/2267) Улучшено расширение шаблонов в `CALL KEYWORDS` для RT индекса с disk chunk.
* [Issue #271](https://github.com/manticoresoftware/manticoresearch-buddy/issues/271) Исправлено зависание на некорректных `/cli` запросах.
* [Issue #274](https://github.com/manticoresoftware/manticoresearch-buddy/issues/274) Решены проблемы, при которых одновременные запросы к Manticore могли зависать.
* [Issue #275](https://github.com/manticoresoftware/manticoresearch-buddy/issues/275) Исправлено зависание при выполнении `drop table if exists t; create table t` через `/cli`.

### Связано с репликацией
* [Issue #2270](https://github.com/manticoresoftware/manticoresearch/issues/2270) Добавлена поддержка формата `cluster:name` в HTTP эндпоинте `/_bulk`.

## Версия 6.3.0
Выпущена: 23 мая 2024

### Крупные изменения
* [Issue #839](https://github.com/manticoresoftware/manticoresearch/issues/839) Реализован тип данных [float_vector](Creating_a_table/Data_types.md#Float-vector); реализован [поиск по векторам](Searching/KNN.md#KNN-vector-search).
* [Issue #1673](https://github.com/manticoresoftware/manticoresearch/issues/1673) [INNER/LEFT JOIN](Searching/Joining.md) (**бета-версия**).
* [Issue #1744](https://github.com/manticoresoftware/manticoresearch/issues/1744) Реализовано автоматическое определение форматов даты для полей [timestamp](Creating_a_table/Data_types.md#Timestamps).
* [Issue #1720](https://github.com/manticoresoftware/manticoresearch/pull/1720) Лицензия Manticore Search изменена с GPLv2-or-later на GPLv3-or-later.
* [Commit 7a55](https://github.com/manticoresoftware/manticoresearch/commit/7a5508424) Запуск Manticore в Windows теперь требует Docker для работы Buddy.
* [Issue #1541](https://github.com/manticoresoftware/manticoresearch/issues/1541) Добавлен полнотекстовый оператор [REGEX](Searching/Full_text_matching/Operators.md#REGEX-operator).
* [Issue #2091](https://github.com/manticoresoftware/manticoresearch/issues/2091) Поддержка Ubuntu Noble 24.04.
* [Commit 514d](https://github.com/manticoresoftware/manticoresearch/commit/514d35b497f4bdb20b6473ff963752a978a4bb8d) Переработка операций со временем для повышения производительности и новые функции даты/времени:
  - [CURDATE()](Functions/Date_and_time_functions.md#CURDATE%28%29) - Возвращает текущую дату в локальной временной зоне
  - [QUARTER()](Functions/Date_and_time_functions.md#QUARTER%28%29) - Возвращает целочисленный квартал года из аргумента timestamp
  - [DAYNAME()](Functions/Date_and_time_functions.md#DAYNAME%28%29) - Возвращает название дня недели для заданного аргумента timestamp
  - [MONTHNAME()](Functions/Date_and_time_functions.md#MONTHNAME%28%29) - Возвращает название месяца для заданного аргумента timestamp
  - [DAYOFWEEK()](Functions/Date_and_time_functions.md#DAYOFWEEK%28%29) - Возвращает целочисленный индекс дня недели для заданного аргумента timestamp
  - [DAYOFYEAR()](Functions/Date_and_time_functions.md#DAYOFYEAR%28%29) - Возвращает целочисленный номер дня в году для заданного аргумента timestamp
  - [YEARWEEK()](Functions/Date_and_time_functions.md#YEARWEEK%28%29) - Возвращает целочисленный год и код дня первого дня текущей недели для заданного аргумента timestamp
  - [DATEDIFF()](Functions/Date_and_time_functions.md#DATEDIFF%28%29) - Возвращает количество дней между двумя заданными timestamp
  - [DATE()](Functions/Date_and_time_functions.md#DATE%28%29) - Форматирует часть даты из аргумента timestamp
  - [TIME()](Functions/Date_and_time_functions.md#TIME%28%29) - Форматирует часть времени из аргумента timestamp
  - [timezone](Server_settings/Searchd.md#timezone) - Временная зона, используемая функциями, связанными с датой/временем.
* [Commit 30e7](https://github.com/manticoresoftware/manticoresearch/commit/30e789d9657f9b3093c280c6e5b0e46a021f6dc7) Добавлены агрегаты [range](Searching/Faceted_search.md#Facet-over-set-of-ranges), [histogram](Searching/Faceted_search.md#Facet-over-histogram-values), [date_range](Searching/Faceted_search.md#Facet-over-set-of-date-ranges) и [date_histogram](Searching/Faceted_search.md#Facet-over-histogram-date-values) в HTTP интерфейс и аналогичные выражения в SQL.

### Minor changes
* [Issue #1285](https://github.com/manticoresoftware/manticoresearch/issues/1285) Поддержка версий Filebeat 8.10 - 8.11.
* [Issue #1771](https://github.com/manticoresoftware/manticoresearch/issues/1771) [ALTER TABLE ... type='distributed'](Updating_table_schema_and_settings.md#Changing-a-distributed-table).
* [Issue #1788](https://github.com/manticoresoftware/manticoresearch/issues/1788) Добавлена возможность копирования таблиц с помощью SQL-запроса [CREATE TABLE ... LIKE ... WITH DATA](Creating_a_table/Local_tables/Real-time_table.md#CREATE-TABLE-LIKE:).
* [Issue #2072](https://github.com/manticoresoftware/manticoresearch/issues/2072) Оптимизация [алгоритма компактирования таблиц](Securing_and_compacting_a_table/Compacting_a_table.md#Compacting-a-Table): Ранее и ручной процесс [OPTIMIZE](Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE), и автоматический [auto_optimize](Server_settings/Searchd.md#auto_optimize) сначала объединяли чанки, чтобы не превышать лимит, а затем очищали удалённые документы из всех других чанков с удалёнными документами. Этот подход иногда был слишком ресурсоёмким и был отключён. Теперь объединение чанков происходит только согласно настройке [progressive_merge](Server_settings/Common.md#progressive_merge). Однако чанки с большим числом удалённых документов с большей вероятностью будут объединены.
* [Commit ce6c](https://github.com/manticoresoftware/columnar/commit/ce6c5d3381edcdc411a00a5923d13d56dd9b3010) Добавлена защита от загрузки вторичного индекса более новой версии.
* [Issue #1417](https://github.com/manticoresoftware/manticoresearch/issues/1417) Частичная замена через [REPLACE INTO ... SET](Data_creation_and_modification/Updating_documents/REPLACE.md#SQL-REPLACE).
* [Commit 7c16](https://github.com/manticoresoftware/manticoresearch/commit/7c167b934ce416d3dfbd658744a231e638b14d9a) Обновлены размеры буферов слияния по умолчанию: `.spa` (скалярные атрибуты): 256КБ -> 8МБ; `.spb` (blob атрибуты): 256КБ -> 8МБ; `.spc` (колончатые атрибуты): 1МБ, без изменений; `.spds` (docstore): 256КБ -> 8МБ; `.spidx` (вторичные индексы): буфер 256КБ -> ограничение памяти 128МБ; `.spi` (словарь): 256КБ -> 16МБ; `.spd` (doclists): 8МБ, без изменений; `.spp` (hitlists): 8МБ, без изменений; `.spe` (skiplists): 256КБ -> 8МБ.
* [Issue #1859](https://github.com/manticoresoftware/manticoresearch/issues/1859) Добавлена [композитная агрегация](Searching/Grouping.md#GROUP-BY-multiple-fields-at-once) через JSON.
* [Commit 216b](https://github.com/manticoresoftware/manticoresearch-buddy/commit/216b824) Отключён PCRE.JIT из-за проблем с некоторыми регулярными выражениями и отсутствия существенного прироста скорости.
* [Commit 55cd](https://github.com/manticoresoftware/manticoresearch/commit/55cdce32) Добавлена поддержка vanilla Galera v.3 (api v25) (`libgalera_smm.so` из MySQL 5.x).
* [Commit 86f9](https://github.com/manticoresoftware/manticoresearch-buddy/commit/86f98ab) Суффикс метрик изменён с `_rate` на `_rps`.
* [Commit c0c1](https://github.com/manticoresoftware/manticoresearch-helm/commit/c0c1cff780031412ef57cefab987dfd16613368e) Улучшена документация по поддержке HA для балансировщика.
* [Коммит d1d2](https://github.com/manticoresoftware/manticoresearch/commit/d1d2ca18857c68106e89743756260e73285be739) Изменено `index` на `table` в сообщениях об ошибках; исправлено сообщение об ошибке парсера bison.
* [Коммит fd26](https://github.com/manticoresoftware/manticoresearch/commit/fd26671b) Добавлена поддержка `manticore.tbl` в качестве имени таблицы.
* [Задача #1105](https://github.com/manticoresoftware/manticoresearch/issues/1105) Поддержка запуска индексатора через systemd ([документация](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Schedule-indexer-via-systemd)). ❤️ Спасибо, [@subnix](https://github.com/subnix) за PR.
* [Задача #1294](https://github.com/manticoresoftware/manticoresearch/issues/1294) Поддержка вторичных индексов в GEODIST().
* [Задача #1394](https://github.com/manticoresoftware/manticoresearch/issues/1394) Упрощён [SHOW THREADS](Node_info_and_management/SHOW_THREADS.md#SHOW-THREADS).
* [Задача #1424](https://github.com/manticoresoftware/manticoresearch/issues/1424) Добавлена поддержка значений по умолчанию (`agent_connect_timeout` и `agent_query_timeout`) для оператора `create distributed table`.
* [Задача #1442](https://github.com/manticoresoftware/manticoresearch/issues/1442) Добавлена опция поискового запроса [expansion_limit](Searching/Options.md#expansion_limit), которая переопределяет `searchd.expansion_limit`.
* [Задача #1448](https://github.com/manticoresoftware/manticoresearch/issues/1448) Реализован [ALTER TABLE](Updating_table_schema_and_settings.md#Updating-table-schema-in-RT-mode) для преобразования int->bigint.
* [Задача #1456](https://github.com/manticoresoftware/manticoresearch/issues/1456) Метаданные в ответе MySQL.
* [Задача #1494](https://github.com/manticoresoftware/manticoresearch/issues/1494) [SHOW VERSION](Node_info_and_management/SHOW_VERSION.md#SHOW-VERSION).
* [Задача #1582](https://github.com/manticoresoftware/manticoresearch/issues/1582) Поддержка [удаления документов по массиву id](Data_creation_and_modification/Deleting_documents.md?client=JSON#Deleting-documents) через JSON.
* [Задача #1589](https://github.com/manticoresoftware/manticoresearch/issues/1589) Улучшено сообщение об ошибке "unsupported value type".
* [Задача #1634](https://github.com/manticoresoftware/manticoresearch/issues/1634) В `SHOW STATUS` добавлена версия Buddy.
* [Задача #1641](https://github.com/manticoresoftware/manticoresearch/issues/1641) Оптимизация запросов сопоставления в случае нуля документов для ключевого слова.
* [Задача #1712](https://github.com/manticoresoftware/manticoresearch/issues/1712) Добавлено преобразование в булев атрибут из строк "true" и "false" при отправке данных.
* [Задача #1713](https://github.com/manticoresoftware/manticoresearch/issues/1713) Добавлена таблица и опция searchd [access_dict](Server_settings/Searchd.md#access_dict).
* [Задача #1767](https://github.com/manticoresoftware/manticoresearch/issues/1767) Добавлены новые опции: [expansion_merge_threshold_docs](Server_settings/Searchd.md#expansion_merge_threshold_docs) и [expansion_merge_threshold_hits](Server_settings/Searchd.md#expansion_merge_threshold_hits) в секцию searchd в конфиге; сделан параметр порога для слияния маленьких расширенных терминов настраиваемым.
* [Задача #1768](https://github.com/manticoresoftware/manticoresearch/issues/1768) Добавлен показ времени последней команды в `@@system.sessions`.
* [Задача #1806](https://github.com/manticoresoftware/manticoresearch/issues/1806) Обновлен протокол Buddy до версии 2.
* [Задача #1810](https://github.com/manticoresoftware/manticoresearch/issues/1810) Реализованы дополнительные форматы запросов к эндпоинту `/sql` для упрощения интеграции с библиотеками.
* [Задача #1825](https://github.com/manticoresoftware/manticoresearch/pull/1825) В SHOW BUDDY PLUGINS добавлен раздел Info.
* [Задача #1837](https://github.com/manticoresoftware/manticoresearch/issues/1837) Улучшено потребление памяти в `CALL PQ` с большими пакетами.
* [Задача #1853](https://github.com/manticoresoftware/manticoresearch/pull/1853) Компилятор заменён с Clang 15 на Clang 16.
* [Задача #1857](https://github.com/manticoresoftware/manticoresearch/issues/1857) Добавлено сравнение строк. ❤️ Спасибо, [@etcd](https://github.com/etcd) за PR.
* [Задача #1915](https://github.com/manticoresoftware/manticoresearch/issues/1915) Добавлена поддержка объединённых хранимых полей.
* [Задача #1937](https://github.com/manticoresoftware/manticoresearch/issues/1937) В журнал запросов добавлен хост:порт клиента.
* [Задача #1981](https://github.com/manticoresoftware/manticoresearch/issues/1981) Исправлена неправильная ошибка.
* [Задача #1983](https://github.com/manticoresoftware/manticoresearch/issues/1983) Введена поддержка уровней подробности для [плана запроса в формате JSON](Node_info_and_management/Profiling/Query_plan.md?client=JSON#Query-plan).
* [Задача #2010](https://github.com/manticoresoftware/manticoresearch/issues/2010) Отключено логирование запросов от Buddy, если не установлен `log_level=debug`.
* [Задача #2035](https://github.com/manticoresoftware/manticoresearch/issues/2035) Поддержка Linux Mint 21.3.
* [Задача #2056](https://github.com/manticoresoftware/manticoresearch/issues/2056) Manticore не собирался с Mysql 8.3+.
* [Задача #2112](https://github.com/manticoresoftware/manticoresearch/issues/2112) Команда `DEBUG DEDUP` для чанков таблиц реального времени, которые могут содержать дубликаты после присоединения plain table с дубликатами.
* [Задача #212](https://github.com/manticoresoftware/manticoresearch-buddy/issues/212) Добавлено время в [SHOW QUERIES](Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES).
* [Задача #218](https://github.com/manticoresoftware/manticoresearch-buddy/issues/218) Обработка столбца `@timestamp` как временной метки.
* [Задача #252](https://github.com/manticoresoftware/manticoresearch-buddy/issues/250) Реализована логика включения/отключения плагинов buddy.
* [Задача #254](https://github.com/manticoresoftware/manticoresearch-buddy/issues/254) Обновлен composer до более свежей версии, где исправлены последние CVE.
* [Issue #340](https://github.com/manticoresoftware/manticoresearch/issues/340) Незначительная оптимизация в системном юните Manticore для systemd, связанная с `RuntimeDirectory`.
* [Issue #51](https://github.com/manticoresoftware/executor/issues/51) Добавлена поддержка rdkafka и обновление до PHP 8.3.3.
* [Issue #527](https://github.com/manticoresoftware/manticoresearch/issues/527) Поддержка [присоединения](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Attaching_one_table_to_another.md#Attaching-one-table-to-another) RT-таблицы. Новая команда [ALTER TABLE ... RENAME](Updating_table_schema_and_settings.md#Renaming-a-real-time-table).

### Критические изменения и устаревания
* ⚠️[Issue #1436](https://github.com/manticoresoftware/manticoresearch/issues/1436) Исправлена ошибка вычисления IDF. `local_df` теперь значение по умолчанию. Улучшен протокол поиска master-agent (обновлена версия). Если вы используете Manticore Search в распределенной среде с несколькими инстансами, сначала обновите агенты, затем мастера.
* ⚠️[Issue #1572](https://github.com/manticoresoftware/manticoresearch/issues/1572) Добавлена репликация распределённых таблиц и обновлён протокол репликации. Если вы запускаете кластер репликации, необходимо:
  - Сначала корректно остановить все узлы
  - Затем запустить последний остановленный узел с `--new-cluster`, используя инструмент `manticore_new_cluster` в Linux.
  - Подробнее о [перезапуске кластера](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster).
* ⚠️[Issue #1763](https://github.com/manticoresoftware/manticoresearch/pull/1763) Псевдонимы HTTP API эндпоинтов `/json/*` устарели.
* ⚠️[Issue #1982](https://github.com/manticoresoftware/manticoresearch/issues/1982) Изменено [profile](Node_info_and_management/Profiling/Query_profile.md#Query-profile) на [plan](Node_info_and_management/Profiling/Query_plan.md#Query-plan) в JSON, добавлено профилирование запросов для JSON.
* ⚠️[Commit e235](https://github.com/manticoresoftware/manticoresearch-backup/commit/e23585a) manticore-backup больше не сохраняет `plugin_dir`.
* ⚠️[Issue #171](https://github.com/manticoresoftware/manticoresearch-buddy/issues/155) Миграция Buddy на Swoole для улучшения производительности и стабильности. При переходе на новую версию убедитесь, что все пакеты Manticore обновлены.
* ⚠️[Issue #196](https://github.com/manticoresoftware/manticoresearch-buddy/pull/205) Объединены все основные плагины в Buddy и изменена основная логика.
* ⚠️[Issue #2107](https://github.com/manticoresoftware/manticoresearch/issues/2107) Идентификаторы документов в ответах `/search` теперь обрабатываются как числа.
* ⚠️[Issue #38](https://github.com/manticoresoftware/executor/issues/38) Добавлен Swoole, отключён ZTS, удалено расширение parallel.
* ⚠️[Issue #1929](https://github.com/manticoresoftware/manticoresearch/issues/1929) Переопределение в `charset_table` не работало в некоторых случаях.

### Изменения, связанные с репликацией
* [Commit 3376](https://github.com/manticoresoftware/manticoresearch/commit/3376351f73db3ff0cb6cb708a304a7bd83e69cfb) Исправлена ошибка репликации при SST больших файлов.
* [Commit 6d36](https://github.com/manticoresoftware/manticoresearch/commit/6d36c68fb7ce3cb274b467aac435788905084952) Добавлен механизм повторных попыток для команд репликации; исправлен сбой присоединения репликации в условиях загруженной сети с потерями пакетов.
* [Commit 842e](https://github.com/manticoresoftware/manticoresearch/commit/842e27e71790833ab4125d49103d2933d5df60c3) Сообщение FATAL в репликации изменено на WARNING.
* [Commit 8c32](https://github.com/manticoresoftware/manticoresearch/commit/8c32bce7db39d8ed3b1a06aaa1d2c3ec372084f0) Исправлен расчёт `gcache.page_size` для репликационных кластеров без таблиц или с пустыми таблицами; также исправлено сохранение и загрузка опций Galera.
* [Commit a2af](https://github.com/manticoresoftware/manticoresearch/commit/a2af06ca36abcbf893e43c8803853130b397c54b) Добавлена функция пропуска команды обновления узлов репликации на узле, присоединяющемся к кластеру.
* [Commit c054](https://github.com/manticoresoftware/manticoresearch/commit/c0541e6892f5116b1f957b40060ba8ecd01a5afb) Исправлена блокировка во время репликации при обновлении BLOB-атрибутов и замене документов.
* [Commit e80d](https://github.com/manticoresoftware/manticoresearch/commit/e80d505b963999016033613c40c1cf3151d648e7) Добавлены опции конфигурации searchd [replication_connect_timeout](Server_settings/Searchd.md#replication_connect_timeout), [replication_query_timeout](Server_settings/Searchd.md#replication_query_timeout), [replication_retry_delay](Server_settings/Searchd.md#replication_retry_delay), [replication_retry_count](Server_settings/Searchd.md#replication_retry_count) для контроля сети при репликации аналогично `searchd.agent_*`, но с разными значениями по умолчанию.
* [Issue #1356](https://github.com/manticoresoftware/manticoresearch/issues/1356) Исправлен повтор подключения узлов репликации при пропуске некоторых узлов и ошибках разрешения их имён.
* [Issue #1445](https://github.com/manticoresoftware/manticoresearch/issues/1445) Исправлен уровень подробности логов репликации для команды `show variables`.
* [Issue #1482](https://github.com/manticoresoftware/manticoresearch/issues/1482) Исправлена ошибка репликации узла-присоединителя при подключении к кластеру после перезапуска в Kubernetes.
* [Issue #1962](https://github.com/manticoresoftware/manticoresearch/issues/1962) Исправлено длительное ожидание изменения репликации в пустом кластере с некорректным именем узла.

### Исправления ошибок
* [Commit 8a48](https://github.com/manticoresoftware/manticoresearch/commit/8a486c7ba1a293879cdecc41fb52eb7ab0cbf832) Исправлена очистка неиспользуемых совпадений в `count distinct`, вызывавшая возможный сбой.
* [Issue #1569](https://github.com/manticoresoftware/manticoresearch/issues/1569) Бинарный лог теперь записывается с гранулярностью транзакций.
* [Issue #2089](https://github.com/manticoresoftware/manticoresearch/issues/2089) Исправлена ошибка, связанная с 64-битными идентификаторами, которая могла привести к ошибке "Malformed packet" при вставке через MySQL, вызывая [повреждение таблиц и дублирование идентификаторов](https://github.com/manticoresoftware/manticoresearch/issues/2090).
* [Issue #2160](https://github.com/manticoresoftware/manticoresearch/issues/2160) Исправлена вставка дат как если бы они были в UTC, а не в местном часовом поясе.
* [Issue #2177](https://github.com/manticoresoftware/manticoresearch/issues/2177) Исправлен сбой, который происходил при выполнении поиска в таблице реального времени с непустым `index_token_filter`.
* [Issue #2209](https://github.com/manticoresoftware/manticoresearch/issues/2209) Изменена фильтрация дубликатов в колонковом хранилище RT для устранения сбоев и неправильных результатов запросов.
* [Commit 001d](https://github.com/manticoresoftware/manticoresearch/commit/001d82904e4b887c49387308291aeefb07c0fbc4) Исправлен html stipper, искажающий память после обработки объединённого поля.
* [Commit 00eb](https://github.com/manticoresoftware/manticoresearch/commit/00eb2ebf) Избежано перемотки потока после flush, чтобы предотвратить проблемы с коммуникацией с mysqldump.
* [Commit 0553](https://github.com/manticoresoftware/manticoresearch/commit/05534114acf08c70ffcc41a23d558dec09a34373) Не ждём окончания preread, если он не был запущен.
* [Commit 055a](https://github.com/manticoresoftware/manticoresearch/commit/055aac09f877570cd29b6594cb283182eba7f73b) Исправлена большая выходная строка Buddy для разбиения на несколько строк в логе searchd.
* [Commit 0a88](https://github.com/manticoresoftware/manticoresearch/commit/0a88056307f82c50fcdc458ad6dbc307a00e3f92) Перемещено предупреждение интерфейса MySQL об ошибочном уровне детализации заголовка `debugv`.
* [Commit 150a](https://github.com/manticoresoftware/manticoresearch/commit/150a20286b7d98d24b844ae577d7fda4a444798d) Исправлена гонка при управлении несколькими кластерами; запрещено создавать несколько кластеров с одинаковым именем или путём.
* [Commit 2e40](https://github.com/manticoresoftware/manticoresearch/commit/2e405c1c9ebac6f46206e3df17ac1c19129c9aa3) Исправлен неявный cutoff в полнотекстовых запросах; разделён MatchExtended на шаблон partD.
* [Commit 75f5](https://github.com/manticoresoftware/manticoresearch/commit/75f51421542a2a0658f9e2f9f31048c8c5224479) Исправлено несоответствие `index_exact_words` между индексацией и загрузкой таблицы в демон.
* [Commit 7643](https://github.com/manticoresoftware/manticoresearch/commit/7643976bea142e08535d701000eff0d09a99fda2) Исправлено пропущенное сообщение об ошибке при удалении неверного кластера.
* [Commit 7a03](https://github.com/manticoresoftware/manticoresearch/commit/7a034ac990d5ec9344320c05a6a2315583b9b858) Исправлен CBO против объединения очередей; исправлен CBO против псевдо-шардинга RT.
* [Commit 7b4e](https://github.com/manticoresoftware/manticoresearch/commit/7b4ea95e) При запуске без библиотеки вторичного индекса (SI) и параметров в конфигурации выдавалось вводящее в заблуждение предупреждение 'WARNING: secondary_indexes set but failed to initialize secondary library'.
* [Commit 8496](https://github.com/manticoresoftware/manticoresearch/commit/8496317f099fade8d2db463fb88483e7fcdfe85b) Исправлена сортировка совпадений в кворуме.
* [Commit 8973](https://github.com/manticoresoftware/manticoresearch-buddy/commit/8973ad3) Исправлена проблема с опциями в верхнем регистре в плагине ModifyTable.
* [Commit 9935](https://github.com/manticoresoftware/manticoresearch-buddy/commit/99350d4c184379b19f3efa9751d9b5be5e6edfb5) Исправлено восстановление из дампа с пустыми json значениями (представленными как NULL).
* [Commit a28f](https://github.com/manticoresoftware/manticoresearch/commit/a28f6e087b3d957e48816070be17d0823904936a) Исправлен тайм-аут SST на узле соединения при получении SST с использованием pcon.
* [Commit b5a5](https://github.com/manticoresoftware/manticoresearch/commit/b5a55cc26debcddcda5d9caef8366a707e0dd763) Исправлен сбой при выборе атрибута строка с алиасом.
* [Commit c556](https://github.com/manticoresoftware/manticoresearch/commit/c556e7ed8b6da8eac8ef094650df5f0a8977916b) Добавлено преобразование запроса терма в `=term` полнотекстового запроса с полем `morphology_skip_fields`.
* [Commit cdc3](https://github.com/manticoresoftware/manticoresearch/commit/cdc38085145cd1025b96fcac2a46188a0425d9e1) Добавлен отсутствующий ключ конфигурации (skiplist_cache_size).
* [Commit cf6e](https://github.com/manticoresoftware/manticoresearch/commit/cf6ec3bfdc3f0990fb31ff53822b2331a6667727) Исправлен сбой в ранжировщике выражений при большом сложном запросе.
* [Commit e513](https://github.com/manticoresoftware/manticoresearch/commit/e5139ba75eef7223df375ae3cc3f138fa7d925f1) Исправлен полнотекстовый CBO при некорректных подсказках индекса.
* [Commit eb05](https://github.com/manticoresoftware/manticoresearch/commit/eb0514b3a6f57f278252b0724eebd112ec116e0f) Прерван preread при завершении работы для ускорения остановки.
* [Commit f945](https://github.com/manticoresoftware/manticoresearch/commit/f94555a293cfa51004ff81135b8bd8500b955115) Изменён расчёт стека для полнотекстовых запросов, чтобы избежать сбоя при сложном запросе.
* [Issue #1262](https://github.com/manticoresoftware/manticoresearch/issues/1262) Исправлен сбой индексатора при индексации SQL-источника с несколькими столбцами с одинаковым именем.
* [Issue #1273](https://github.com/manticoresoftware/manticoresearch/issues/1273) Возвращается 0 вместо <empty> для несуществующих системных переменных.
* [Issue #1289](https://github.com/manticoresoftware/manticoresearch/issues/1289) Исправлена ошибка indextool при проверке внешних файлов таблицы RT.
* [Issue #1335](https://github.com/manticoresoftware/manticoresearch/issues/1335) Исправлена ошибка парсинга запроса из-за многословных форм внутри фразы.
* [Issue #1364](https://github.com/manticoresoftware/manticoresearch/issues/1364) Добавлено воспроизведение пустых binlog-файлов со старыми версиями binlog.
* [Issue #1365](https://github.com/manticoresoftware/manticoresearch/issues/1365) Исправлено удаление последнего пустого binlog-файла.
* [Issue #1372](https://github.com/manticoresoftware/manticoresearch/issues/1372) Исправлены неверные относительные пути (преобразованы в абсолютные из каталога запуска демона) после изменений в `data_dir`, влияющих на текущий рабочий каталог при запуске демона.
* [Issue #1393](https://github.com/manticoresoftware/manticoresearch/issues/1393) Самое медленное ухудшение времени в hn_small: получение/кэширование информации о CPU при запуске демона.
* [Issue #1395](https://github.com/manticoresoftware/manticoresearch/issues/1395) Исправлено предупреждение о пропущенном внешнем файле при загрузке индекса.
* [Issue #1402](https://github.com/manticoresoftware/manticoresearch/issues/1402) Исправлен сбой в глобальных группировщиках при освобождении указателей данных атрибутов.
* [Issue #1403](https://github.com/manticoresoftware/manticoresearch/issues/1403) _ADDITIONAL_SEARCHD_PARAMS не работает.
* [Issue #1427](https://github.com/manticoresoftware/manticoresearch/issues/1427) Исправлено замещение параметра `agent_query_timeout` для каждой таблицы на значение по умолчанию.
* [Issue #1444](https://github.com/manticoresoftware/manticoresearch/issues/1444) Исправлен сбой в группировщике и ранжировщике при использовании `packedfactors()` с несколькими значениями на совпадение.
* [Issue #1458](https://github.com/manticoresoftware/manticoresearch/issues/1458) Manticore аварийно завершает работу при частых обновлениях индекса.
* [Issue #1481](https://github.com/manticoresoftware/manticoresearch/issues/1481) Исправлен сбой при очистке разобранного запроса после ошибки синтаксического анализа.
* [Issue #1484](https://github.com/manticoresoftware/manticoresearch/issues/1484) Исправлена проблема с маршрутизацией HTTP JSON запросов к buddy.
* [Issue #1499](https://github.com/manticoresoftware/manticoresearch/issues/1499) Корневая JSON-атрибут не могла быть массивом. Исправлено.
* [Issue #1507](https://github.com/manticoresoftware/manticoresearch/issues/1507) Исправлен сбой при воссоздании таблицы внутри транзакции.
* [Issue #1515](https://github.com/manticoresoftware/manticoresearch/issues/1515) Исправлено расширение коротких форм русских лемм.
* [Issue #1579](https://github.com/manticoresoftware/manticoresearch/issues/1579) Исправлено использование JSON и STRING атрибутов в выражении [date_format](Functions/Date_and_time_functions.md#DATE_FORMAT()).
* [Issue #1580](https://github.com/manticoresoftware/manticoresearch/issues/1580) Исправлен группировщик для нескольких псевдонимов к JSON полям.
* [Issue #1594](https://github.com/manticoresoftware/manticoresearch/issues/1594) Неверное total_related в dev: исправлено неявное ограничение vs limit; улучшено обнаружение fullscan в JSON запросах.
* [Issue #1603](https://github.com/manticoresoftware/manticoresearch/issues/1603) Исправлено использование JSON и STRING атрибутов во всех выражениях с датами.
* [Issue #1609](https://github.com/manticoresoftware/manticoresearch/issues/1609) Сбой при использовании LEVENSHTEIN().
* [Issue #1612](https://github.com/manticoresoftware/manticoresearch/issues/1612) Исправлено повреждение памяти после ошибки синтаксического анализа поискового запроса с подсветкой.
* [Issue #1614](https://github.com/manticoresoftware/manticoresearch/issues/1614) Отключено расширение wildcard для терминов короче, чем `min_prefix_len` / `min_infix_len`.
* [Issue #1617](https://github.com/manticoresoftware/manticoresearch/issues/1617) Изменено поведение: не логировать ошибку, если Buddy успешно обрабатывает запрос.
* [Issue #1635](https://github.com/manticoresoftware/manticoresearch/issues/1635) Исправлено значение total в метаданных поискового запроса для запросов с установкой limit.
* [Issue #1640](https://github.com/manticoresoftware/manticoresearch/issues/1640) Невозможно использовать таблицу с заглавными буквами через JSON в plain режиме.
* [Issue #1643](https://github.com/manticoresoftware/manticoresearch/issues/1643) Добавлено значение по умолчанию для `SPH_EXTNODE_STACK_SIZE`.
* [Issue #1646](https://github.com/manticoresoftware/manticoresearch/issues/1646) Исправлен журнал SphinxQL для отрицательного фильтра с ALL/ANY на атрибуте MVA.
* [Issue #1660](https://github.com/manticoresoftware/manticoresearch/issues/1660) Исправлено применение списков вычеркивания docid из других индексов. ❤️ Спасибо, [@raxoft](https://github.com/raxoft) за PR.
* [Issue #1668](https://github.com/manticoresoftware/manticoresearch/issues/1668) Исправлены пропущенные совпадения из-за преждевременного выхода при полном сканировании сырого индекса (без итераторов индексов); удалён cutoff из plain row итератора.
* [Issue #1671](https://github.com/manticoresoftware/manticoresearch/issues/1671) Исправлена ошибка `FACET` при запросе распределённой таблицы с агентскими и локальными таблицами.
* [Issue #1690](https://github.com/manticoresoftware/manticoresearch/issues/1690) Исправлен сбой при оценке гистограммы для больших значений.
* [Issue #1692](https://github.com/manticoresoftware/manticoresearch/issues/1692) Сбой при alter table tbl add column col uint.
* [Issue #1710](https://github.com/manticoresoftware/manticoresearch/issues/1710) Пустой результат для условия `WHERE json.array IN (<value>)`.
* [Issue #172](https://github.com/manticoresoftware/manticoresearch-buddy/issues/172) Исправлена проблема с TableFormatter при отправке запроса к `/cli`.
* [Issue #1742](https://github.com/manticoresoftware/manticoresearch/issues/1742) `CREATE TABLE` не выдавал ошибку при отсутствии файла wordforms.
* [Issue #1762](https://github.com/manticoresoftware/manticoresearch/issues/1762) Порядок атрибутов в RT таблицах теперь следует порядку конфигурации.
* [Issue #1765](https://github.com/manticoresoftware/manticoresearch/issues/1765) HTTP bool запрос с условием 'should' возвращает некорректные результаты.
* [Issue #1769](https://github.com/manticoresoftware/manticoresearch/issues/1769) Сортировка по строковым атрибутам не работает с `SPH_SORT_ATTR_DESC` и `SPH_SORT_ATTR_ASC`.
* [Issue #177](https://github.com/manticoresoftware/manticoresearch-buddy/issues/177) Заголовок HTTP запроса `Expect: 100-continue` отключен для curl запросов к Buddy.
* [Issue #1791](https://github.com/manticoresoftware/manticoresearch/issues/1791) Сбой, вызванный псевдонимом GROUP BY.
* [Issue #1792](https://github.com/manticoresoftware/manticoresearch/issues/1792) SQL мета сводка показывает неверное время на Windows.
* [Issue #1794](https://github.com/manticoresoftware/manticoresearch/issues/1794) Исправлено падение производительности при одном термине в JSON-запросах.
* [Issue #1798](https://github.com/manticoresoftware/manticoresearch/issues/1798) Несовместимые фильтры не вызывали ошибку на `/search`.
* [Issue #1802](https://github.com/manticoresoftware/manticoresearch/issues/1802) Исправлена работа операций `ALTER CLUSTER ADD` и `JOIN CLUSTER` для взаимного ожидания друг друга, предотвращая состояние гонки, при котором `ALTER` добавляет таблицу в кластер, пока донор отправляет таблицы на узел присоединения.
* [Issue #1811](https://github.com/manticoresoftware/manticoresearch/issues/1811) Неправильная обработка запросов `/pq/{table}/*`.
* [Issue #1816](https://github.com/manticoresoftware/manticoresearch/issues/1816) `UNFREEZE` не срабатывал в некоторых случаях.
* [Issue #183](https://github.com/manticoresoftware/manticoresearch-buddy/pull/183) Исправлена проблема с восстановлением MVA в некоторых случаях.
* [Issue #1849](https://github.com/manticoresoftware/manticoresearch/issues/1849) Исправлен сбой indextool при завершении работы, если используется с MCL.
* [Issue #1866](https://github.com/manticoresoftware/manticoresearch/issues/1866) Исправлена ненужная декодировка URL для запросов `/cli_json`.
* [Issue #1872](https://github.com/manticoresoftware/manticoresearch/issues/1872) Изменена логика установки plugin_dir при запуске демона.
* [Issue #1874](https://github.com/manticoresoftware/manticoresearch/issues/1874) Ошибка при исключениях alter table ...
* [Issue #1891](https://github.com/manticoresoftware/manticoresearch/issues/1891#issuecomment-2051941200) Manticore падает с `signal 11` при вставке данных.
* [Issue #1920](https://github.com/manticoresoftware/manticoresearch/issues/1920) Уменьшено ограничение по скорости для [low_priority](Searching/Options.md#low_priority).
* [Issue #1924](https://github.com/manticoresoftware/manticoresearch/issues/1924) Ошибка при Mysqldump + восстановлении mysql.
* [Issue #1951](https://github.com/manticoresoftware/manticoresearch/issues/1951) Исправлено некорректное создание распределённой таблицы в случае отсутствия локальной таблицы или неправильного описания агента; теперь возвращается сообщение об ошибке.
* [Issue #1972](https://github.com/manticoresoftware/manticoresearch/issues/1972) Реализован счётчик `FREEZE`, чтобы избежать проблем с freeze/unfreeze.
* [Issue #1980](https://github.com/manticoresoftware/manticoresearch/issues/1980) Учтён таймаут запроса в OR-узлах. Ранее `max_query_time` мог не работать в некоторых случаях.
* [Issue #1986](https://github.com/manticoresoftware/manticoresearch/issues/1986) Не удалось переименовать new в current [manticore.json].
* [Issue #1988](https://github.com/manticoresoftware/manticoresearch/issues/1988) Полнотекстовый запрос мог игнорировать подсказку `SecondaryIndex` CBO.
* [Issue #1990](https://github.com/manticoresoftware/manticoresearch/issues/1990) Исправлен `expansion_limit` для среза итогового набора результатов по ключевым словам вызова из нескольких дисковых или RAM-чанков.
* [Issue #1994](https://github.com/manticoresoftware/manticoresearch/issues/1994) неправильные внешние файлы.
* [Issue #2021](https://github.com/manticoresoftware/manticoresearch/issues/2021) Несколько процессов manticore-executor могли остаться запущенными после остановки Manticore.
* [Issue #2029](https://github.com/manticoresoftware/manticoresearch/issues/2029) Сбой при использовании расстояния Левенштейна.
* [Issue #2037](https://github.com/manticoresoftware/manticoresearch/issues/2037) Ошибка после нескольких вызовов max на пустом индексе.
* [Issue #2052](https://github.com/manticoresoftware/manticoresearch/issues/2052) Сбой на multi-group с JSON.field.
* [Issue #2067](https://github.com/manticoresoftware/manticoresearch/issues/2067) Manticore падал на некорректном запросе к _update.
* [Issue #2069](https://github.com/manticoresoftware/manticoresearch/issues/2069) Исправлена проблема с операторами сравнения строк для замкнутых диапазонов в JSON-интерфейсе.
* [Issue #2082](https://github.com/manticoresoftware/manticoresearch/issues/2082) `alter` не срабатывал, если путь data_dir находился на симлинке.
* [Issue #2102](https://github.com/manticoresoftware/manticoresearch/pull/2102) Улучшена специальная обработка запросов SELECT в mysqldump для обеспечения совместимости итоговых вставок с Manticore.
* [Issue #2103](https://github.com/manticoresoftware/manticoresearch/issues/2103) Тайские символы были в неправильных кодировках.
* [Issue #2124](https://github.com/manticoresoftware/manticoresearch/issues/2124) Сбой при использовании SQL с зарезервированным словом.
* [Issue #2154](https://github.com/manticoresoftware/manticoresearch/issues/2154) Таблицы с формами слов не могли быть импортированы.
* [Issue #2176](https://github.com/manticoresoftware/manticoresearch/issues/2176) Исправлен сбой, который происходил при установке параметра engine в 'columnar' и добавлении дублирующихся ID через JSON.
* [Issue #223](https://github.com/manticoresoftware/manticoresearch-buddy/issues/223) Корректная ошибка при попытке вставить документ без схемы и без названий колонок.
* [Issue #239](https://github.com/manticoresoftware/manticoresearch-buddy/issues/239) Автоматическая схема для многострочной вставки могла завершиться неудачей.
* [Issue #399](https://github.com/manticoresoftware/manticoresearch/issues/399) Добавлено сообщение об ошибке при индексации, если атрибут id объявлен в источнике данных.
* [Issue #59](https://github.com/manticoresoftware/manticoresearch-helm/issues/59) Поломка кластера Manticore.
* [Issue #68](https://github.com/manticoresoftware/manticoresearch-helm/issues/68) optimize.php падал, если присутствовала таблица percolate.
* [Issue #77](https://github.com/manticoresoftware/manticoresearch-helm/issues/77) Исправлены ошибки при развертывании в Kubernetes.
* [Issue #274](https://github.com/manticoresoftware/manticoresearch-buddy/issues/274) Исправлена некорректная обработка одновременных запросов к Buddy

### Related with manticore-backup
* [Issue #97](https://github.com/manticoresoftware/manticoresearch-backup/pull/97) Установлен VIP HTTP порт по умолчанию, если доступен.
Различные улучшения: улучшена проверка версий и потоковая декомпрессия ZSTD; добавлены запросы пользователю при несоответствии версий в процессе восстановления; исправлено неправильное поведение запросов для разных версий при восстановлении; улучшена логика декомпрессии для чтения прямо из потока вместо рабочей памяти; добавлен флаг `--force`
* [Commit 3b35](https://github.com/manticoresoftware/manticoresearch-backup/commit/3b357ea) Добавлено отображение версии резервной копии после запуска Manticore search для идентификации проблем на этом этапе.
* [Commit ad2e](https://github.com/manticoresoftware/manticoresearch-backup/commit/ad2e5da) Обновлено сообщение об ошибке при неудачных попытках подключения к демону.
* [Commit ce5e](https://github.com/manticoresoftware/manticoresearch-backup/commit/ce5ea85) Исправлена проблема с конвертацией абсолютных корневых путей резервной копии в относительные и удалена проверка доступности записи при восстановлении для возможности восстановления из разных путей.
* [Commit db7e](https://github.com/manticoresoftware/manticoresearch-backup/commit/db7e2b9) Добавлена сортировка итератора файлов для обеспечения согласованности в различных ситуациях.
* [Issue #106](https://github.com/manticoresoftware/manticoresearch-backup/issues/105) Резервное копирование и восстановление нескольких конфигураций.
* [Issue #91](https://github.com/manticoresoftware/manticoresearch-backup/pull/91) Добавлен defattr для предотвращения необычных прав пользователей для файлов после установки на RHEL.
* [Issue #91](https://github.com/manticoresoftware/manticoresearch-backup/pull/91) Добавлено дополнительное выполнение chown, чтобы файлы по умолчанию принадлежали пользователю root в Ubuntu.

### Связано с MCL (columnar, secondary, knn libs)
* [Commit f104](https://github.com/manticoresoftware/columnar/commit/f104a4709bbb144140d09a09bc0fc1db1895e331) Поддержка векторного поиска.
* [Commit 2169](https://github.com/manticoresoftware/columnar/commit/216953beef6676f7e34c9f6f1a30c5980d746710) Исправлена очистка временных файлов при прерванной настройке сборки вторичного индекса. Это решает проблему, когда демон превышал лимит открытых файлов при создании файлов `tmp.spidx`.
* [Commit 709b](https://github.com/manticoresoftware/columnar/commit/709b9acaaac97d9a1ca8796892f9ad432021c785) Использование отдельной библиотеки streamvbyte для columnar и SI.
* [Commit 1c26](https://github.com/manticoresoftware/manticoresearch/commit/1c2623763ced14bb94ad624dde9614a938408d89) Добавлено предупреждение, что columnar хранилище не поддерживает json атрибуты.
* [Commit 3acd](https://github.com/manticoresoftware/columnar/commit/3acd6714ffce103447d7931dfc75bb1a7449f340) Исправлена распаковка данных в SI.
* [Commit 574c](https://github.com/manticoresoftware/manticoresearch/commit/574c023152564fa72092c18ebd1f594abfd4b6d7) Исправлен сбой при сохранении дискового чанка с смешанным хранением rowwise и columnar.
* [Commit e87f](https://github.com/manticoresoftware/columnar/commit/e87f088b458d4c5b1b6159fc72ed0d57b786891a) Исправлен итератор SI, которому указывался на уже обработанный блок.
* [Issue #1474](https://github.com/manticoresoftware/manticoresearch/issues/1474) Обновление не работает для rowwise MVA столбца с движком columnar.
* [Issue #1510](https://github.com/manticoresoftware/manticoresearch/issues/1510) Исправлен сбой при агрегации по columnar атрибуту, используемому в `HAVING`.
* [Issue #1519](https://github.com/manticoresoftware/manticoresearch/issues/1519) Исправлен сбой в ранжировщике `expr` при использовании columnar атрибута.

### Связано с Docker
* ❗[Issue #42](https://github.com/manticoresoftware/docker/issues/42) Поддержка [plain индексирования](https://github.com/manticoresoftware/docker#building-plain-tables) через переменные окружения.
* ❗[Issue #47](https://github.com/manticoresoftware/docker/issues/47) Улучшена гибкость конфигурации через переменные окружения.
* [Issue #54](https://github.com/manticoresoftware/docker/pull/54) Улучшены процессы [резервного копирования и восстановления](https://github.com/manticoresoftware/docker#backup-and-restore) для Docker.
* [Issue #77](https://github.com/manticoresoftware/docker/issues/76) Улучшен entrypoint для обработки восстановления резервной копии только при первом запуске.
* [Commit a27c](https://github.com/manticoresoftware/docker/commit/a27c048dd9e73d8d6bea8d9a4830cafb486b82c5) Исправлен лог запросов в stdout.
* [Issue #38](https://github.com/manticoresoftware/docker/issues/38) Подавление предупреждений BUDDY, если EXTRA не установлен.
* [Issue #71](https://github.com/manticoresoftware/docker/pull/71) Исправлено имя хоста в `manticore.conf.sh`.

## Версия 6.2.12
Выпуск: 23 августа 2023

Версия 6.2.12 продолжает серию 6.2 и устраняет проблемы, обнаруженные после релиза 6.2.0.

### Исправления ошибок
* ❗[Issue #1351](https://github.com/manticoresoftware/manticoresearch/issues/1351) «Manticore 6.2.0 не запускается через systemctl на Centos 7»: Изменено значение `TimeoutStartSec` с `infinity` на `0` для лучшей совместимости с Centos 7.
* ❗[Issue #1364](https://github.com/manticoresoftware/manticoresearch/issues/1364) «Сбой после обновления с 6.0.4 до 6.2.0»: Добавлена функция воспроизведения для пустых binlog-файлов из старых версий binlog.
* [PR #1334](https://github.com/manticoresoftware/manticoresearch/pull/1334) «исправление опечатки в searchdreplication.cpp»: Исправлена опечатка в `searchdreplication.cpp`: beggining -> beginning.
* [Issue #1337](https://github.com/manticoresoftware/manticoresearch/issues/1337) «Manticore 6.2.0 WARNING: conn (local)(12), sock=8088: bailing on failed MySQL header, AsyncNetInputBuffer_c::AppendData: error 11 (Resource temporarily unavailable) return -1»: Понижен уровень детализации предупреждения интерфейса MySQL о заголовке до logdebugv.
* [Issue #1355](https://github.com/manticoresoftware/manticoresearch/issues/1355) «join cluster зависает, когда нельзя разрешить node_address»: Улучшен повтор попытки репликации при недоступности некоторых узлов и их неудачном разрешении имени. Это должно решить проблемы репликации в узлах Kubernetes и Docker. Улучшено сообщение об ошибке при ошибках запуска репликации, обновлена тестовая модель 376. Кроме того, добавлено чёткое сообщение об ошибке при сбоях разрешения имен.
* [Issue #1361](https://github.com/manticoresoftware/manticoresearch/issues/1361) "Нет маппинга в нижнем регистре для "Ø" в кодировке non_cjk": Отрегулировали сопоставление для символа 'Ø'.
* [Issue #1365](https://github.com/manticoresoftware/manticoresearch/issues/1365) "searchd оставляет binlog.meta и binlog.001 после чистой остановки": Обеспечено правильное удаление последнего пустого файла binlog.
* [Commit 0871](https://github.com/manticoresoftware/manticoresearch/commit/0871070845e93ad702ae0dfb89b35e5f24cd0851): Исправлена проблема сборки `Thd_t` на Windows, связанная с ограничениями атомарного копирования.
* [Commit 1cc0](https://github.com/manticoresoftware/manticoresearch/commit/1cc04bedc174737118fd5a3663e35f4e95bd4f8c): Исправлена проблема с FT CBO и `ColumnarScan`.
* [Commit c6bf](https://github.com/manticoresoftware/manticoresearch/commit/c6bfb30d53e3f4b67f17769667733677a661c95b): Внесены исправления в тест 376 и добавлена замена для ошибки `AF_INET` в тесте.
* [Commit cbc3](https://github.com/manticoresoftware/manticoresearch/commit/cbc38705e0b4dcc9665e3e8cf64826290ebd11cf): Решена проблема взаимоблокировки при репликации при обновлении blob-атрибутов и замене документов. Также удалена rlock индекса во время коммита, так как он уже заблокирован на более низком уровне.

### Незначительные изменения
* [Commit 4f91](https://github.com/manticoresoftware/manticoresearch/commit/4f913a36e84e01f950b2797051222b0b1094d6fe) Обновлена информация по эндпоинтам `/bulk` в руководстве.

### MCL
* Поддержка [Manticore Columnar Library](https://github.com/manticoresoftware/columnar) версии 2.2.4

## Версия 6.2.0
Выпущено: 4 августа 2023

### Крупные изменения
* Оптимизатор запросов улучшен для поддержки полнотекстовых запросов, что значительно повышает эффективность и производительность поиска.
* Интеграции с:
  - [mysqldump](https://dev.mysql.com/doc/refman/8.0/en/mysqldump.html) — для [создания логических резервных копий](Securing_and_compacting_a_table/Backup_and_restore.md#Backup-and-restore-with-mysqldump) с помощью `mysqldump`
  - [Apache Superset](https://manticoresearch.com/blog/manticoresearch-apache-superset-integration/) и [Grafana](https://manticoresearch.com/blog/manticoresearch-grafana-integration/) для визуализации данных, хранящихся в Manticore
  - [HeidiSQL](https://www.heidisql.com/) и [DBForge](https://www.devart.com/dbforge/) для удобства разработки с Manticore
* Начали использовать [GitHub workflows](https://github.com/manticoresoftware/manticoresearch/actions), что упрощает участникам использовать тот же процесс непрерывной интеграции (CI), который применяет основная команда при подготовке пакетов. Все задания могут выполняться на хостах GitHub, что облегчает бесперебойное тестирование изменений в вашей форк-версии Manticore Search.
* Начали использовать [CLT](https://github.com/manticoresoftware/clt) для тестирования сложных сценариев. Например, теперь мы можем гарантировать, что пакет, собранный после коммита, корректно устанавливается во всех поддерживаемых дистрибутивах Linux. Command Line Tester (CLT) предоставляет удобный способ записывать тесты в интерактивном режиме и без труда воспроизводить их.
* Значительное улучшение производительности операции count distinct за счёт использования комбинации хэш-таблиц и HyperLogLog.
* Включено многопоточное выполнение запросов, содержащих вторичные индексы, при этом количество потоков ограничено числом физических ядер процессора. Это должно существенно ускорить выполнение запросов.
* `pseudo_sharding` теперь ограничен числом свободных потоков. Это обновление значительно повышает сквозную производительность.
* Пользователи теперь могут указывать [движок хранения атрибутов по умолчанию](Server_settings/Searchd.md#engine) через настройки конфигурации, обеспечивая лучшую кастомизацию под конкретные нагрузки.
* Поддержка [Manticore Columnar Library 2.2.0](https://github.com/manticoresoftware/columnar/) с множеством исправлений ошибок и улучшений в [вторичных индексах](Server_settings/Searchd.md#secondary_indexes).

### Незначительные изменения
* [Buddy #153](https://github.com/manticoresoftware/manticoresearch-buddy/issues/153): HTTP-эндпоинт [/pq](Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md?client=JSON#Adding-rules-to-a-percolate-table) теперь служит псевдонимом для HTTP-эндпоинта `/json/pq`.
* [Commit 0bf1](https://github.com/manticoresoftware/manticoresearch/commit/0bf17d9e): Обеспечена многобайтовая совместимость для функций `upper()` и `lower()`.
* [Commit 2bb9](https://github.com/manticoresoftware/manticoresearch/commit/2bb92765): Вместо сканирования индекса для запросов `count(*)` теперь возвращается предварительно рассчитанное значение.
* [Commit 3c84](https://github.com/manticoresoftware/manticoresearch/commit/3c84091f): Теперь можно использовать `SELECT` для произвольных вычислений и отображения `@@sysvars`. В отличие от предыдущих версий, теперь вы не ограничены одним вычислением. Таким образом, запросы вида `select user(), database(), @@version_comment, version(), 1+1 as a limit 10` вернут все столбцы. Обратите внимание, что опциональный 'limit' всегда будет игнорироваться.
* [Commit 6aca](https://github.com/manticoresoftware/manticoresearch/commit/6aca32f0): Реализован заглушечный запрос `CREATE DATABASE`.
* [Commit 9dc1](https://github.com/manticoresoftware/manticoresearch/commit/9dc12334): При выполнении `ALTER TABLE table REBUILD SECONDARY` вторичные индексы всегда перестраиваются, даже если атрибуты не обновлялись.
* [Commit 46ed](https://github.com/manticoresoftware/manticoresearch/commit/46edb089): Сортировщики с использованием предварительно рассчитанных данных теперь определяются до применения CBO, чтобы избежать лишних вычислений CBO.
* [Commit 102a](https://github.com/manticoresoftware/manticoresearch/commit/102ac604): Реализовано имитирование и использование стека полнотекстовых выражений для предотвращения сбоев демона.
* [Commit 979f](https://github.com/manticoresoftware/manticoresearch/commit/979fa27c): Добавлен ускоренный кодовый путь для клонирования совпадений для матчей, которые не используют атрибуты строк/массивов/json.
* [Коммит a073](https://github.com/manticoresoftware/manticoresearch/commit/a0735ffe): Добавлена поддержка команды `SELECT DATABASE()`. Однако она всегда будет возвращать `Manticore`. Это дополнение важно для интеграции с различными инструментами MySQL.
* [Коммит bc04](https://github.com/manticoresoftware/manticoresearch/commit/bc04908): Изменён формат ответа эндпоинта [/cli](Connecting_to_the_server/HTTP.md#/cli), и добавлен эндпоинт `/cli_json`, который работает как прежний `/cli`.
* [Коммит d70b](https://github.com/manticoresoftware/manticoresearch/commit/d70b0d58): Параметр `thread_stack` теперь можно изменять во время работы с помощью оператора `SET`. Поддерживаются как сессионные, так и глобальные варианты. Текущие значения можно посмотреть в выводе `show variables`.
* [Коммит d96e](https://github.com/manticoresoftware/manticoresearch/commit/d96ec6b9): Код был интегрирован в CBO для более точной оценки сложности выполнения фильтров по строковым атрибутам.
* [Коммит e77d](https://github.com/manticoresoftware/manticoresearch/commit/e77dd72f5a04531c352fad0d7afcd2a1cbae2510): Улучшен расчёт стоимости DocidIndex, что повысило общую производительность.
* [Коммит f3ae](https://github.com/manticoresoftware/manticoresearch/commit/f3ae8bea): Метрики нагрузки, аналогичные 'uptime' в Linux, теперь отображаются в команде `SHOW STATUS`.
* [Коммит f3cc](https://github.com/manticoresoftware/manticoresearch/commit/f3cc0971): Порядок полей и атрибутов для команд `DESC` и `SHOW CREATE TABLE` теперь совпадает с порядком `SELECT * FROM`.
* [Коммит f3d2](https://github.com/manticoresoftware/manticoresearch/commit/f3d248a6): Различные внутренние парсеры теперь при различных ошибках предоставляют свой внутренний мнемонический код (например, `P01`). Это помогает определить, какой парсер вызвал ошибку, а также скрывает несущественные внутренние детали.
* [Issue #271](https://github.com/manticoresoftware/manticoresearch/issues/271) "Иногда CALL SUGGEST не предлагает исправление ошибки в одной букве": Улучшено поведение [SUGGEST/QSUGGEST](Searching/Spell_correction.md#CALL-QSUGGEST,-CALL-SUGGEST) для коротких слов: добавлен параметр `sentence` для показа всего предложения.
* [Issue #696](https://github.com/manticoresoftware/manticoresearch/issues/696) "Percolate индекс не выполняет поиск точными фразами, когда включён стемминг": Запрос percolate был изменён для поддержки точного модификатора термина, что улучшило функциональность поиска.
* [Issue #829](https://github.com/manticoresoftware/manticoresearch/issues/829) "Методы форматирования дат": добавлено выражение списка выбора [date_format()](Functions/Date_and_time_functions.md#DATE_FORMAT()), которое использует функцию `strftime()`.
* [Issue #961](https://github.com/manticoresoftware/manticoresearch/issues/961) "Сортировка бакетов через HTTP JSON API": введено опциональное свойство [sort property](Searching/Faceted_search.md#HTTP-JSON) для каждого бакета агрегатов в HTTP интерфейсе.
* [Issue #1062](https://github.com/manticoresoftware/manticoresearch/issues/1062) "Улучшение логирования ошибок при сбое JSON insert api - "unsupported value type"": Эндпоинт `/bulk` теперь сообщает информацию о количестве обработанных и необработанных строк (документов) при ошибке.
* [Issue #1070](https://github.com/manticoresoftware/manticoresearch/issues/1070) "CBO hints не поддерживают несколько атрибутов": Поддержка подсказок индексов расширена для работы с несколькими атрибутами.
* [Issue #1106](https://github.com/manticoresoftware/manticoresearch/issues/1106) "Добавить теги в HTTP поисковый запрос": В [HTTP PQ ответах](Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md?client=JSON#Adding-rules-to-a-percolate-table) добавлены теги.
* [Issue #1301](https://github.com/manticoresoftware/manticoresearch/issues/1301) "buddy не должен создавать таблицу параллельно": Исправлена ошибка, вызывавшая сбои при параллельном выполнении CREATE TABLE. Теперь выполняется только одна операция `CREATE TABLE` одновременно.
* [Issue #1303](https://github.com/manticoresoftware/manticoresearch/issues/1303) "добавить поддержку @ в именах колонок".
* [Issue #1316](https://github.com/manticoresoftware/manticoresearch/issues/1316) "Запросы к набору данных такси медленные с ps=1": Логика CBO улучшена, а разрешение гистограммы по умолчанию установлено в 8k для большей точности работы с атрибутами со случайным распределением значений.
* [Issue #1317](https://github.com/manticoresoftware/manticoresearch/issues/1317) "Исправление CBO против полнотекстового индекса в наборе данных hn": Внедрена усовершенствованная логика определения, когда использовать пересечение итераторов bitmap и когда применять приоритетную очередь.
* [Issue #1318](https://github.com/manticoresoftware/manticoresearch/issues/1318) "columnar: изменить интерфейс итератора на однократный вызов": Итераторы columnar теперь используют один вызов `Get`, заменяющий предыдущие двухшаговые вызовы `AdvanceTo` + `Get` для получения значения.
* [Issue #1319](https://github.com/manticoresoftware/manticoresearch/issues/1319) "Ускорение вычисления агрегатов (убрать CheckReplaceEntry?)": Вызов `CheckReplaceEntry` удалён из сортировщика групп, что ускоряет вычисление агрегатных функций.
* [Issue #1320](https://github.com/manticoresoftware/manticoresearch/issues/1320) "create table read_buffer_docs/hits не понимает синтаксис k/m/g": Опции `read_buffer_docs` и `read_buffer_hits` в `CREATE TABLE` теперь поддерживают синтаксис k/m/g.
* [Языковые пакеты](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) для английского, немецкого и русского теперь можно легко установить на Linux командой `apt/yum install manticore-language-packs`. На macOS используйте команду `brew install manticoresoftware/tap/manticore-language-packs`.
* Порядок полей и атрибутов теперь согласован между командами `SHOW CREATE TABLE` и `DESC`.
* Если на диске недостаточно места при выполнении `INSERT` запросов, новые `INSERT` запросы будут завершаться с ошибкой, пока не освободится достаточно места.
* Добавлена функция преобразования типов [UINT64()](Functions/Type_casting_functions.md#UINT64%28%29).
* Конечная точка `/bulk` теперь обрабатывает пустые строки как команду [commit](Data_creation_and_modification/Transactions.md#BEGIN,-COMMIT,-and-ROLLBACK). Подробнее [здесь](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md?client=JSON#Bulk-adding-documents).
* Для [недействительных подсказок индекса](Searching/Options.md#Query-optimizer-hints) реализованы предупреждения, обеспечивающие большую прозрачность и позволяющие снизить вероятность ошибок.
* При использовании `count(*)` с одним фильтром запросы теперь используют заранее рассчитанные данные из вторичных индексов, если они доступны, что существенно ускоряет время выполнения запроса.

### ⚠️ Ломающие изменения
* ⚠️ Таблицы, созданные или изменённые в версии 6.2.0, не могут быть прочитаны более старыми версиями.
* ⚠️ Идентификаторы документов теперь обрабатываются как беззнаковые 64-битные целые числа при индексировании и операциях INSERT.
* ⚠️ Синтаксис подсказок оптимизатора запросов обновлён. Новый формат — `/*+ SecondaryIndex(uid) */`. Учтите, что старый синтаксис больше не поддерживается.
* ⚠️ [Issue #1160](https://github.com/manticoresoftware/manticoresearch/issues/1160): Использование символа `@` в именах таблиц запрещено для предотвращения конфликтов синтаксиса.
* ⚠️ Строковые поля/атрибуты, помеченные как `indexed` и `attribute`, теперь считаются единым полем при операциях `INSERT`, `DESC` и `ALTER`.
* ⚠️ [Issue #1057](https://github.com/manticoresoftware/manticoresearch/issues/1057): Библиотеки MCL больше не будут загружаться на системах, не поддерживающих SSE 4.2.
* ⚠️ [Issue #1143](https://github.com/manticoresoftware/manticoresearch/issues/1143): [agent_query_timeout](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_query_timeout) был неисправен. Исправлено и теперь работает корректно.

### Исправления ошибок
* [Commit 2a6e](https://github.com/manticoresoftware/manticoresearch/commit/2a6ea8f7) "Сбой при DROP TABLE": решена проблема, вызывавшая длительное ожидание завершения операций записи (оптимизация, сохранение на диск) в RT-таблице при выполнении DROP TABLE. Добавлено предупреждение при непустой директории таблицы после команды DROP TABLE.
* [Commit 2ebd](https://github.com/manticoresoftware/manticoresearch/commit/2ebd424d): Добавлена поддержка колоннарных атрибутов, отсутствовавшая в коде для группировки по нескольким атрибутам.
* [Commit 3be4](https://github.com/manticoresoftware/manticoresearch/commit/3be4503f752216b37b8cde8e47fd2d77bbce63ae) Исправлен сбой, который мог возникнуть из-за нехватки места на диске, путём правильной обработки ошибок записи в binlog.
* [Commit 6adb](https://github.com/manticoresoftware/manticoresearch/commit/6adb0934): Исправлен случайный сбой при использовании нескольких колоннарных итераторов сканирования (или итераторов по вторичным индексам) в запросе.
* [Commit 6bd9](https://github.com/manticoresoftware/manticoresearch/commit/6bd9f709): Фильтры не удалялись при использовании сортировщиков, оперирующих ранее рассчитанными данными. Исправлено.
* [Commit 6d03](https://github.com/manticoresoftware/manticoresearch/commit/6d03566a): Код CBO обновлён для лучших оценок запросов с фильтрами по строковым атрибутам при выполнении в нескольких потоках.
* [Commit 6dd3](https://github.com/manticoresoftware/manticoresearch/commit/6dd3964), [Helm #56](https://github.com/manticoresoftware/manticoresearch-helm/issues/56) "фатальный сбой в Kubernetes кластере": исправлен дефектный блум-фильтр для JSON корневого объекта; исправлен сбой демона при фильтрации по JSON полю.
* [Commit 6e1b](https://github.com/manticoresoftware/manticoresearch/commit/6e1b849) Устранён сбой демона, вызванный неверной конфигурацией `manticore.json`.
* [Commit 6fbc](https://github.com/manticoresoftware/manticoresearch/commit/6fbc4189) Исправлен json фильтр диапазона для поддержки значений int64.
* [Commit 9c67](https://github.com/manticoresoftware/manticoresearch/commit/9c677426) `.sph` файлы могли быть повреждены после `ALTER`. Исправлено.
* [Commit 77cc](https://github.com/manticoresoftware/manticoresearch/commit/77ccd8f): Добавлен общий ключ для репликации операции replace, чтобы решить ошибку `pre_commit`, возникавшую при репликации replace с нескольких мастер-узлов.
* [Commit 2884](https://github.com/manticoresoftware/manticoresearch/commit/2884519e) устранены проблемы с проверками bigint для функций типа 'date_format()'.
* [Commit 9513](https://github.com/manticoresoftware/manticoresearch/commit/9513aca5): Итераторы больше не отображаются в [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META), когда сортировщики используют заранее рассчитанные данные.
* [Commit a2a7](https://github.com/manticoresoftware/manticoresearch/commit/a2a70555): Обновлён размер стека fulltext узла для предотвращения сбоев при сложных полнотекстовых запросах.
* [Commit a062](https://github.com/manticoresoftware/manticoresearch/commit/a0626d7e): Исправлена ошибка, вызывавшая сбой при репликации обновлений с JSON и строковыми атрибутами.
* [Commit b3e6](https://github.com/manticoresoftware/manticoresearch/commit/b3e63e5d): Строитель строк обновлён для использования 64-битных целочисленных значений, чтобы избежать сбоев при работе с большими объёмами данных.
* [Commit c472](https://github.com/manticoresoftware/manticoresearch/commit/c472e5b): Исправлен сбой, возникавший при подсчёте distinct по нескольким индексам.
* [Commit d073](https://github.com/manticoresoftware/manticoresearch/commit/d0730272): Исправлена проблема, когда запросы по дисковым чанкам RT индексов могли выполняться в нескольких потоках, даже если `pseudo_sharding` был отключён.
* [Commit d205](https://github.com/manticoresoftware/manticoresearch/commit/d205508e) Набор значений команды `show index status` изменён и теперь варьируется в зависимости от типа используемого индекса.
* [Commit e9bc](https://github.com/manticoresoftware/manticoresearch/commit/e9bc2f9) Исправлена HTTP ошибка при обработке пакетных запросов и проблема, когда ошибка не возвращалась клиенту из сетевого цикла.
* [Commit f77c](https://github.com/manticoresoftware/manticoresearch/commit/f77ce0e6) использование расширенного стека для PQ.
* [Коммит fac2](https://github.com/manticoresoftware/manticoresearch/commit/fac2d9687a20582765aaa89be9597804c4f845af) Обновлен вывод ранжировщика экспорта для соответствия [packedfactors()](Functions/Searching_and_ranking_functions.md#PACKEDFACTORS%28%29).
* [Коммит ff87](https://github.com/manticoresoftware/manticoresearch/commit/ff87cc4): Исправлена ошибка со строковым списком в фильтре журнала запросов SphinxQL.
* [Проблема #589](https://github.com/manticoresoftware/manticoresearch/issues/589) "Определение charset, кажется, зависит от порядка кодов": Исправлено неправильное отображение charset для дубликатов.
* [Проблема #811](https://github.com/manticoresoftware/manticoresearch/issues/811) "Отображение нескольких слов в формах слов мешает поиску фраз с CJK пунктуацией между ключевыми словами": Исправлена позиция токенов ngram в пределах фразового запроса с формами слов.
* [Проблема #834](https://github.com/manticoresoftware/manticoresearch/issues/834) "Знак равенства в поисковом запросе ломает запрос": Обеспечена возможность экранирования точного символа и исправлено двойное точное расширение опцией `expand_keywords`.
* [Проблема #864](https://github.com/manticoresoftware/manticoresearch/issues/864) "Конфликт exceptions/stopwords"
* [Проблема #910](https://github.com/manticoresoftware/manticoresearch/issues/910) "Сбой Manticore при вызове call snippets() с libstemmer_fr и index_exact_words": Устранены внутренние конфликты, вызывавшие сбои при вызове `SNIPPETS()`.
* [Проблема #946](https://github.com/manticoresoftware/manticoresearch/issues/946) "Дублирование записей в SELECT": Исправлена проблема дублирующихся документов в результатах запроса с опцией `not_terms_only_allowed` для RT индекса с удалёнными документами.
* [Проблема #967](https://github.com/manticoresoftware/manticoresearch/issues/967) "Использование JSON аргументов в UDF функциях приводит к сбою": Исправлен сбой демона при обработке поиска с включённым псевдо-шардингом и UDF с JSON аргументом.
* [Проблема #1050](https://github.com/manticoresoftware/manticoresearch/issues/1050) "count(*) в FEDERATED": Исправлен сбой демона при выполнении запроса через движок `FEDERATED` с агрегатами.
* [Проблема #1052](https://github.com/manticoresoftware/manticoresearch/issues/1052) Исправлена несовместимость колонки `rt_attr_json` с колоночным хранилищем.
* [Проблема #1072](https://github.com/manticoresoftware/manticoresearch/issues/1072) "* удаляется из поискового запроса в ignore_chars": Исправлено, чтобы подстановочные знаки в запросе не затрагивались опцией `ignore_chars`.
* [Проблема #1075](https://github.com/manticoresoftware/manticoresearch/issues/1075) "indextool --check не работает если есть распределённая таблица": indextool теперь совместим с инстансами, имеющими 'distributed' и 'template' индексы в json-конфиге.
* [Проблема #1081](https://github.com/manticoresoftware/manticoresearch/issues/1081) "Конкретный select на конкретном RT наборе данных приводит к сбою searchd": Исправлен сбой демона на запросе с packedfactors и большим внутренним буфером.
* [Проблема #1095](https://github.com/manticoresoftware/manticoresearch/issues/1095) "При not_terms_only_allowed удалённые документы игнорируются"
* [Проблема #1099](https://github.com/manticoresoftware/manticoresearch/issues/1099) "indextool --dumpdocids не работает": Восстановлена работа команды `--dumpdocids`.
* [Проблема #1100](https://github.com/manticoresoftware/manticoresearch/issues/1100) "indextool --buildidf не работает": indextool теперь закрывает файл после завершения работы с globalidf.
* [Проблема #1104](https://github.com/manticoresoftware/manticoresearch/issues/1104) "Count(*) пытается восприниматься как схема в удалённых таблицах": Исправлена проблема, из-за которой демон отправлял сообщение об ошибке для запросов к распределённому индексу, когда агент возвращал пустой набор результатов.
* [Проблема #1109](https://github.com/manticoresoftware/manticoresearch/issues/1109) "FLUSH ATTRIBUTES зависает при threads=1".
* [Проблема #1126](https://github.com/manticoresoftware/manticoresearch/issues/1126) "Потеря соединения с сервером MySQL во время запроса - manticore 6.0.5": Исправлены сбои при использовании нескольких фильтров по колоночным атрибутам.
* [Проблема #1135](https://github.com/manticoresoftware/manticoresearch/issues/1135) "Чувствительность к регистру при фильтрации JSON строк": Скорректирована сопоставимость для корректной работы фильтров, используемых в HTTP поисковых запросах.
* [Проблема #1140](https://github.com/manticoresoftware/manticoresearch/issues/1140) "Матч в неправильном поле": Исправлены повреждения, связанные с `morphology_skip_fields`.
* [Проблема #1155](https://github.com/manticoresoftware/manticoresearch/issues/1155) "Удалённые системные команды через API должны принимать g_iMaxPacketSize": Внесены изменения для обхода проверки `max_packet_size` для команд репликации между узлами. Также добавлена последняя ошибка кластера в отображение статуса.
* [Проблема #1302](https://github.com/manticoresoftware/manticoresearch/issues/1302) "Временные файлы остаются после неудачного optimize": Исправлена проблема с оставшимися временными файлами после ошибки в процессе слияния или оптимизации.
* [Проблема #1304](https://github.com/manticoresoftware/manticoresearch/issues/1304) "Добавлена переменная окружения для таймаута запуска buddy": Добавлена переменная окружения `MANTICORE_BUDDY_TIMEOUT` (по умолчанию 3 секунды) для управления временем ожидания сообщения buddy при запуске демона.
* [Проблема #1305](https://github.com/manticoresoftware/manticoresearch/issues/1305) "Переполнение Int при сохранении мета PQ": Смягчено чрезмерное потребление памяти демоном при сохранении большого PQ индекса.
* [Проблема #1306](https://github.com/manticoresoftware/manticoresearch/issues/1306) "Невозможно воссоздать RT таблицу после изменения внешнего файла": Исправлена ошибка alter с пустой строкой для внешних файлов; исправлены оставшиеся внешние файлы RT индекса после изменения внешних файлов.
* [Проблема #1307](https://github.com/manticoresoftware/manticoresearch/issues/1307) "SELECT sum(value) as value не работает правильно": Исправлена проблема, когда выражение в списке select с псевдонимом могло скрывать атрибут индекса; также исправлен sum для подсчёта в int64 для целочисленных значений.
* [Issue #1308](https://github.com/manticoresoftware/manticoresearch/issues/1308) "Избегать привязки к localhost в репликации": Гарантирована неприоритетная привязка репликации к localhost для хостов с несколькими IP.
* [Issue #1309](https://github.com/manticoresoftware/manticoresearch/issues/1309) "ответ mysql клиенту не удался для данных больше 16Mb": Исправлена проблема возврата пакета SphinxQL размером более 16Mb клиенту.
* [Issue #1310](https://github.com/manticoresoftware/manticoresearch/issues/1310) "неверная ссылка в "пути к внешним файлам должны быть абсолютными": Исправлено отображение полного пути к внешним файлам в `SHOW CREATE TABLE`.
* [Issue #1311](https://github.com/manticoresoftware/manticoresearch/issues/1311) "сборка для отладки падает на длинных строках в сниппетах": Теперь в тексте, обрабатываемом функцией `SNIPPET()`, допускаются длинные строки (>255 символов).
* [Issue #1312](https://github.com/manticoresoftware/manticoresearch/issues/1312) "ложный сбой при использовании-after-delete в kqueue polling (master-agent)": Исправлены сбои при невозможности мастера подключиться к агенту на системах с kqueue (FreeBSD, MacOS и др.).
* [Issue #1313](https://github.com/manticoresoftware/manticoresearch/issues/1313) "слишком долгое подключение к себе": При подключении от мастера к агентам на MacOS/BSD теперь используется объединенный таймаут на подключение и запрос, вместо только подключения.
* [Issue #1314](https://github.com/manticoresoftware/manticoresearch/issues/1314) "pq (json meta) с недостижимыми встроенными синонимами не загружается": Исправлен флаг встроенных синонимов в pq.
* [Issue #1315](https://github.com/manticoresoftware/manticoresearch/issues/1315) "Разрешить некоторым функциям (sint, fibonacci, second, minute, hour, day, month, year, yearmonth, yearmonthday) использовать неявно повышенные значения аргументов".
* [Issue #1321](https://github.com/manticoresoftware/manticoresearch/issues/1321) "Включить многопоточный SI в fullscan, но ограничить количество потоков": В CBO реализован код для лучшего прогнозирования производительности многопоточных вторичных индексов при использовании в полнотекстовом запросе.
* [Issue #1322](https://github.com/manticoresoftware/manticoresearch/issues/1322) "count(*) запросы все еще медленные после использования precalc sorters": Итераторы больше не инициализируются при использовании сортировщиков с предвычисленными данными, избегая негативного влияния на производительность.
* [Issue #1411](https://github.com/manticoresoftware/manticoresearch/issues/1141) "лог запросов в sphinxql не сохраняет оригинальные запросы для MVA": Теперь логируются `all()/any()`.

## Version 6.0.4
Released: March 15 2023

### New features
* Улучшена интеграция с Logstash, Beats и т.д., включая:
  - Поддержку Logstash версий 7.6 - 7.15, Filebeat версий 7.7 - 7.12
  - Поддержку авто-схемы.
  - Добавлена обработка bulk-запросов в формате, похожем на Elasticsearch.
* [Buddy commit ce90](https://github.com/manticoresoftware/manticoresearch-buddy/commit/ce907ea) Версия Log Buddy при старте Manticore.

### Bugfixes
* [Issue #588](https://github.com/manticoresoftware/manticoresearch/issues/588), [Issue #942](https://github.com/manticoresoftware/manticoresearch/issues/942) Исправлен неправильный символ в поисковой метаинформации и вызове keywords для биграммного индекса.
* [Issue #1027](https://github.com/manticoresoftware/manticoresearch/issues/1027) Заголовки HTTP в нижнем регистре отклоняются.
* ❗[Issue #1039](https://github.com/manticoresoftware/manticoresearch/issues/1039) Исправлена утечка памяти в демоне при чтении вывода консоли Buddy.
* [Issue #1056](https://github.com/manticoresoftware/manticoresearch/issues/1056) Исправлено непредвиденное поведение вопросительного знака.
* [Issue #1064](https://github.com/manticoresoftware/manticoresearch/issues/1064) - Исправлена гонка при доступе к таблицам токенизатора с нижним регистром, приводящая к сбою.
* [Commit 59bb](https://github.com/manticoresoftware/manticoresearch/commit/59bb54c) Исправлена обработка bulk-записей в JSON интерфейсе для документов с явно установленным id в null.
* [Commit 7b6b](https://github.com/manticoresoftware/manticoresearch/commit/7b6b25f) Исправлена статистика терминов в CALL KEYWORDS для множественных одинаковых терминов.
* [Commit f381](https://github.com/manticoresoftware/manticoresearch/commit/f381ad2) Конфигурация по умолчанию теперь создается установщиком для Windows; пути больше не заменяются во время выполнения.
* [Commit 6940](https://github.com/manticoresoftware/manticoresearch/commit/6940e95), [Commit cc5a](https://github.com/manticoresoftware/manticoresearch/commit/cc5a480) Исправлены проблемы репликации для кластера с узлами в разных сетях.
* [Commit 4972](https://github.com/manticoresoftware/manticoresearch/commit/49722ab) Исправлен HTTP endpoint `/pq` как псевдоним для `/json/pq`.
* [Commit 3b53](https://github.com/manticoresoftware/manticoresearch/commit/3b5385a) Исправлен сбой демона при перезапуске Buddy.
* [Buddy commit fba9](https://github.com/manticoresoftware/manticoresearch-buddy/commit/fba9c8c) Отображение оригинальной ошибки при получении некорректного запроса.
* [Buddy commit db95](https://github.com/manticoresoftware/manticoresearch-buddy/commit/db9532c) Разрешены пробелы в пути резервного копирования и добавлены специальные обработки в регулярные выражения для поддержки одиночных кавычек.

## Version 6.0.2
Released: Feb 10 2023

### Bugfixes
* [Issue #1024 crash 2](https://github.com/manticoresoftware/manticoresearch/issues/1024) Сбой / Segmentation Fault при поиске по атрибуту Facet с большим числом результатов
* ❗[Issue #1029](https://github.com/manticoresoftware/manticoresearch/issues/1029) - ВНИМАНИЕ: Встроенное значение KNOWN_CREATE_SIZE (16) меньше измеренного (208). Рассмотрите возможность исправления значения!
* ❗[Issue #1032](https://github.com/manticoresoftware/manticoresearch/issues/1032) - Падение plain индекса Manticore 6.0.0
* ❗[Issue #1033](https://github.com/manticoresoftware/manticoresearch/issues/1033) - Потеря нескольких распределенных запросов при перезапуске демона
* ❗[Issue #1064](https://github.com/manticoresoftware/manticoresearch/issues/1064) - гонка в таблицах токенизатора нижнего регистра

## Version 6.0.0
Released: Feb 7 2023

Начиная с этого релиза, Manticore Search поставляется с Manticore Buddy, сайдкар-демоном, написанным на PHP, который обрабатывает высокоуровневую функциональность, не требующую сверхнизкой задержки или высокой пропускной способности. Manticore Buddy работает за кулисами, и вы можете даже не заметить, что он запущен. Хотя он невидим для конечного пользователя, создание Manticore Buddy, легко устанавливаемого и совместимого с основным демоном на C++, было значительной задачей. Это крупное изменение позволит команде разрабатывать широкий спектр новых высокоуровневых функций, таких как оркестрация шардов, контроль доступа и аутентификация, а также различные интеграции, например mysqldump, DBeaver, графический коннектор Grafana mysql. Уже сейчас он обрабатывает [SHOW QUERIES](Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES), [BACKUP](Securing_and_compacting_a_table/Backup_and_restore.md#BACKUP-SQL-command-reference) и [Auto schema](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema).

Этот релиз также включает более 130 исправлений ошибок и множество новых функций, многие из которых можно считать крупными.

### Основные изменения
* 🔬 Экспериментально: теперь можно выполнять совместимые с Elasticsearch [insert](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md?example=Elasticsearch#Adding-documents-to-a-real-time-table) и [replace](Data_creation_and_modification/Updating_documents/REPLACE.md?example=Elasticsearch#REPLACE) JSON-запросы, что позволяет использовать Manticore с такими инструментами, как Logstash (версия < 7.13), Filebeat и другими инструментами из семейства Beats. Включено по умолчанию. Вы можете отключить это с помощью `SET GLOBAL ES_COMPAT=off`.
* Поддержка [Manticore Columnar Library 2.0.0](https://github.com/manticoresoftware/columnar/) с множеством исправлений и улучшений в [вторичных индексах](Server_settings/Searchd.md#secondary_indexes). **⚠️ КРИТИЧЕСКОЕ ИЗМЕНЕНИЕ**: вторичные индексы по умолчанию ВКЛЮЧЕНЫ с этого релиза. Убедитесь, что вы выполнили [ALTER TABLE table_name REBUILD SECONDARY](Updating_table_schema_and_settings.md#Rebuilding-a-secondary-index), если обновляетесь с Manticore 5. Подробнее см. ниже.
* [Коммит c436](https://github.com/manticoresoftware/manticoresearch/commit/c436f9023536f767610451911955ae36d90aa638) Авто-схема: теперь можно пропустить создание таблицы, просто вставьте первый документ, и Manticore автоматически создаст таблицу на основе его полей. Подробно об этом можно прочитать [здесь](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema). Вы можете включать/выключать это с помощью [searchd.auto_schema](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema).
* Значительное обновление [cost-based optimizer](Searching/Cost_based_optimizer.md), который в ряде случаев снижает время отклика запросов.
  - [Issue #1008](https://github.com/manticoresoftware/manticoresearch/issues/1008) Оценка производительности параллелизации в CBO.
  - [Issue #1014](https://github.com/manticoresoftware/manticoresearch/issues/1014) CBO теперь учитывает [вторичные индексы](Server_settings/Searchd.md#secondary_indexes) и может действовать более разумно.
  - [Коммит cef9](https://github.com/manticoresoftware/columnar/commit/cef9b54dc3a4ee9138ebc4df3a671715b996d6b3) Статистика кодировок колоннарных таблиц/полей теперь сохраняется в метаданных для помощи CBO в принятии более грамотных решений.
  - [Коммит 2b95](https://github.com/manticoresoftware/manticoresearch/commit/2b95fb8c2ba422c9c5a66e2b61c1f7922f31c7aa) Добавлены подсказки CBO для тонкой настройки его поведения.
* [Телеметрия](Telemetry.md#Telemetry): мы рады объявить о добавлении телеметрии в этот релиз. Эта функция позволяет собирать анонимные и деперсонализированные метрики, которые помогут нам улучшить производительность и опыт использования нашего продукта. Будьте уверены, все собираемые данные **полностью анонимны и не будут связаны с какой-либо личной информацией**. Эта функция может быть [легко отключена](Telemetry.md#Telemetry) в настройках при желании.
* [Коммит 5aaf](https://github.com/manticoresoftware/manticoresearch/commit/5aafb17d81b601e399e212a344a4738f65576cdd) [ALTER TABLE table_name REBUILD SECONDARY](Updating_table_schema_and_settings.md#Rebuilding-a-secondary-index) для перестроения вторичных индексов в любое время, например:
  - при миграции с Manticore 5 на новую версию,
  - при выполнении [UPDATE](Data_creation_and_modification/Updating_documents/UPDATE.md#UPDATE) (то есть [обновления на месте, а не замены](Data_creation_and_modification/Updating_documents/REPLACE_vs_UPDATE.md)) атрибута в индексе
* [Issue #821](https://github.com/manticoresoftware/manticoresearch/issues/821) Новый инструмент `manticore-backup` для [резервного копирования и восстановления экземпляра Manticore](Securing_and_compacting_a_table/Backup_and_restore.md)
* SQL-команда [BACKUP](Securing_and_compacting_a_table/Backup_and_restore.md#BACKUP-SQL-command-reference) для выполнения резервного копирования изнутри Manticore.
* SQL-команда [SHOW QUERIES](Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES) как простой способ посмотреть выполняющиеся запросы, а не только потоки.
* [Issue #551](https://github.com/manticoresoftware/manticoresearch/issues/551) SQL-команда `KILL` для прерывания долгих `SELECT` запросов.
* Динамический `max_matches` для агрегационных запросов для увеличения точности и снижения времени отклика.

### Мелкие изменения
* [Issue #822](https://github.com/manticoresoftware/manticoresearch/issues/822) SQL-команды [FREEZE/UNFREEZE](Securing_and_compacting_a_table/Freezing_a_table.md) для подготовки таблицы реального времени/обычной таблицы к резервному копированию.
* [Коммит c470](https://github.com/manticoresoftware/manticoresearch/commit/c47052f18ad4868134de1a97e8d7da570e531dfa) Новые настройки `accurate_aggregation` и `max_matches_increase_threshold` для контролируемой точности агрегации.
* [Issue #718](https://github.com/manticoresoftware/manticoresearch/issues/718) Поддержка знаковых отрицательных 64-битных ID. Обратите внимание, вы все еще не можете использовать ID > 2^63, но теперь можно использовать id в диапазоне от -2^63 до 0.
* Так как мы недавно добавили поддержку вторичных индексов, ситуация стала запутанной, так как "индекс" мог означать вторичный индекс, полнотекстовый индекс или простой/реального времени `index`. Чтобы уменьшить путаницу, мы переименовываем последний в "table". Следующие SQL/команды командной строки затрагиваются этим изменением. Их старые версии устарели, но все еще работают:
  - `index <table name>` => `table <table name>`,
  - `searchd -i / --index` => `searchd -t / --table`,
  - `SHOW INDEX STATUS` => `SHOW TABLE STATUS`,
  - `SHOW INDEX SETTINGS` => `SHOW TABLE SETTINGS`,
  - `FLUSH RTINDEX` => `FLUSH TABLE`,
  - `OPTIMIZE INDEX` => `OPTIMIZE TABLE`,
  - `ATTACH TABLE plain TO RTINDEX rt` => `ATTACH TABLE plain TO TABLE rt`,
  - `RELOAD INDEX` => `RELOAD TABLE`,
  - `RELOAD INDEXES` => `RELOAD TABLES`.

  Мы не планируем делать старые формы устаревшими, но для обеспечения совместимости с документацией рекомендуем изменить имена в вашем приложении. В будущем релизе будет изменено переименование "index" в "table" в выводе различных SQL и JSON команд.
* Запросы с состоянием в UDF теперь вынуждены выполняться в одном потоке.
* [Issue #1011](https://github.com/manticoresoftware/manticoresearch/issues/1011) Рефакторинг всего, что связано с планированием времени, как предварительное условие для параллельного слияния чанков.
* **⚠️ КРИТИЧЕСКОЕ ИЗМЕНЕНИЕ**: Формат колоночного хранения был изменен. Вам нужно перестроить те таблицы, у которых есть колоночные атрибуты.
* **⚠️ КРИТИЧЕСКОЕ ИЗМЕНЕНИЕ**: Формат файлов вторичных индексов был изменен, поэтому если вы используете вторичные индексы для поиска и у вас в конфигурационном файле указано `searchd.secondary_indexes = 1`, имейте в виду, что новая версия Manticore **пропустит загрузку таблиц с вторичными индексами**. Рекомендуется:
  - Перед обновлением изменить `searchd.secondary_indexes` на 0 в конфигурационном файле.
  - Запустить инстанс. Manticore загрузит таблицы с предупреждением.
  - Выполнить `ALTER TABLE <table name> REBUILD SECONDARY` для каждого индекса, чтобы перестроить вторичные индексы.

  Если вы используете кластер репликации, потребуется выполнить `ALTER TABLE <table name> REBUILD SECONDARY` на всех узлах или следовать [этой инструкции](Securing_and_compacting_a_table/Compacting_a_table.md#Optimizing-clustered-tables) с заменой: выполнить `ALTER .. REBUILD SECONDARY` вместо `OPTIMIZE`.
* **⚠️ КРИТИЧЕСКОЕ ИЗМЕНЕНИЕ**: Версия binlog была обновлена, поэтому любые binlog файлы из предыдущих версий не будут воспроизведены. Важно обеспечить корректную остановку Manticore Search при обновлении. Это означает, что после остановки предыдущего инстанса в `/var/lib/manticore/binlog/` не должно оставаться файлов binlog, кроме `binlog.meta`.
* [Issue #849](https://github.com/manticoresoftware/manticoresearch/issues/849) `SHOW SETTINGS`: теперь можно видеть настройки из конфигурационного файла изнутри Manticore.
* [Issue #1007](https://github.com/manticoresoftware/manticoresearch/issues/1007) [SET GLOBAL CPUSTATS=1/0](Server_settings/Setting_variables_online.md#SET) включает/выключает отслеживание процессорного времени; [SHOW THREADS](Node_info_and_management/SHOW_THREADS.md) теперь не показывает статистику CPU, если отслеживание времени CPU выключено.
* [Issue #1009](https://github.com/manticoresoftware/manticoresearch/issues/1009) Сегменты RAM чанка таблицы RT теперь могут слипаться во время сброса RAM чанка.
* [Issue #1012](https://github.com/manticoresoftware/manticoresearch/issues/1012) Добавлен прогресс вторичных индексов в вывод [indexer](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool).
* [Issue #1013](https://github.com/manticoresoftware/manticoresearch/issues/1013) Ранее запись таблицы могла быть удалена из списка индексов Manticore, если он не мог начать обслуживать её при запуске. Новое поведение — оставлять её в списке, чтобы попытаться загрузить при следующем запуске.
* [indextool --docextract](Miscellaneous_tools.md#indextool) возвращает все слова и вхождения для запрошенного документа.
* [Commit 2b29](https://github.com/manticoresoftware/manticoresearch/commit/2b296ee20e520b85bcbb4383e87095d31e7165dc) Переменная окружения `dump_corrupt_meta` позволяет сбрасывать поврежденные метаданные таблицы в лог, если searchd не может загрузить индекс.
* [Commit c7a3](https://github.com/manticoresoftware/manticoresearch/commit/c7a3136eefe0eb2f69e3d99fae9c10beb5821ea6) `DEBUG META` может показывать `max_matches` и псевдо-статистику шардинга.
* [Commit 6bca](https://github.com/manticoresoftware/manticoresearch/commit/6bca4848dc5c8396dbc1e057be4b9b2d579794e8) Более информативная ошибка вместо запутывающей "Index header format is not json, will try it as binary...".
* [Commit bef3](https://github.com/manticoresoftware/lemmatizer-uk/commit/bef3ff0386d3ee87ec57619782100972c1122e47) Путь лемматизатора украинского языка был изменен.
* [Commit 4ae7](https://github.com/manticoresoftware/manticoresearch/commit/4ae789595329a2951e194d1191ddb3121459a560) В статистику вторичных индексов добавлена команда [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META).
* [Commit 2e7c](https://github.com/manticoresoftware/manticoresearch/commit/2e7c585e) JSON интерфейс теперь легко визуализируется с помощью Swagger Editor https://manual.manticoresearch.com/Openapi#OpenAPI-specification.
* **⚠️ КРИТИЧЕСКОЕ ИЗМЕНЕНИЕ**: Протокол репликации изменён. Если вы используете кластер репликации, при обновлении до Manticore 5 необходимо:
  - сначала корректно остановить все узлы
  - затем запустить последний остановленный узел с `--new-cluster` (запустить инструмент `manticore_new_cluster` в Linux).
  - подробнее читайте в разделе [перезапуск кластера](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster).

### Изменения, связанные с Manticore Columnar Library
* Рефакторинг интеграции вторичных индексов с колоночным хранением.
* [Commit efe2](https://github.com/manticoresoftware/columnar/commit/efe26b2a) Оптимизация Manticore Columnar Library, которая может снизить время отклика за счет частичной предварительной оценки min/max.
* [Commit 2757](https://github.com/manticoresoftware/columnar/commit/2757b99b) Если слияние дискового чанка прервано, демон теперь очищает временные файлы, связанные с MCL.
* [Commit e9c6](https://github.com/manticoresoftware/manticoresearch/commit/e9c672014b63e2b49e7a5a53880583af86c3ab4e) Версии библиотек Columnar и secondary выгружаются в лог при сбое.
* [Commit f5e8](https://github.com/manticoresoftware/columnar/commit/f5e84eeb8b6cb102f13e3468246702427527fef9) Добавлена поддержка быстрого перемотки doclist для вторичных индексов.
* [Commit 06df](https://github.com/manticoresoftware/manticoresearch/commit/06dfdd2e668933495dd6cb52d5ba2e0aa9b26e58) Запросы типа `select attr, count(*) from plain_index` (без фильтрации) теперь работают быстрее при использовании MCL.
* [Commit 0a76](https://github.com/manticoresoftware/manticoresearch/commit/0a76ecb9adbe886868b962dd425de5f475bbf55e) @@autocommit в HandleMysqlSelectSysvar для совместимости с .net коннектором для mysql версии выше 8.25
* **⚠️ ЛОМКА ИНТЕРФЕЙСА**: [MCL Issue #17](https://github.com/manticoresoftware/columnar/issues/17) MCL: добавлен SSE код для сканирования столбцов. Теперь MCL требует минимум SSE4.2.

### Изменения, связанные с упаковкой
* [Commit 4d19](https://github.com/manticoresoftware/manticoresearch/commit/4d19f5cbe49a31228aa09253f061165bfe80e51b) **⚠️ ЛОМКА ИНТЕРФЕЙСА**: Прекращена поддержка Debian Stretch и Ubuntu Xenial.
* Поддержка RHEL 9, включая Centos 9, Alma Linux 9 и Oracle Linux 9.
* [Issue #924](https://github.com/manticoresoftware/manticoresearch/issues/924) Поддержка Debian Bookworm.
* [Issue #636](https://github.com/manticoresoftware/manticoresearch/issues/636) Упаковка: сборки для arm64 для Linux и MacOS.
* [PR #26](https://github.com/manticoresoftware/docker/pull/26) Мультиядерное (x86_64 / arm64) docker-изображение.
* [Упрощена сборка пакетов для контрибьюторов](Installation/Compiling_from_sources.md#Building-using-CI-Docker).
* Теперь можно установить конкретную версию через APT.
* [Commit a6b8](https://github.com/manticoresoftware/manticoresearch/commit/51fddca5c2a3ebb8576fae4c18660656ba22de0f) Инсталлятор для Windows (ранее мы предоставляли только архив).
* Переход на компиляцию с помощью CLang 15.
* **⚠️ ЛОМКА ИНТЕРФЕЙСА**: Пользовательские формулы Homebrew, включая формулу для Manticore Columnar Library. Чтобы установить Manticore, MCL и другие необходимые компоненты, используйте команду `brew install manticoresoftware/manticore/manticoresearch manticoresoftware/manticore/manticore-extra`.

### Исправления ошибок
* [Issue #479](https://github.com/manticoresoftware/manticoresearch/issues/479) Поле с именем `text`
* [Issue #501](https://github.com/manticoresoftware/manticoresearch/issues/501) id не может быть не bigint
* [Issue #646](https://github.com/manticoresoftware/manticoresearch/issues/646) ALTER с полем с именем "text"
* ❗[Issue #652](https://github.com/manticoresoftware/manticoresearch/issues/652) Возможная ошибка: HTTP (JSON) offset и limit влияют на результаты фасета
* ❗[Issue #827](https://github.com/manticoresoftware/manticoresearch/issues/827) Searchd зависает/вылетает при интенсивной нагрузке
* ❗[Issue #996](https://github.com/manticoresoftware/manticoresearch/issues/996) PQ индекс вышел из памяти
* ❗[Commit 1041](https://github.com/manticoresoftware/manticoresearch/commit/10416ef7dddf06c0d759e32ccd6ebaa2468f7cbf) `binlog_flush = 1` был сломан с самого начала после Sphinx. Исправлено.
* [MCL Issue #14](https://github.com/manticoresoftware/columnar/issues/14) MCL: сбой при выборе, когда слишком много ft полей
* [Issue #470](https://github.com/manticoresoftware/manticoresearch/issues/470) sql_joined_field нельзя сохранить
* [Issue #713](https://github.com/manticoresoftware/manticoresearch/issues/713) Сбой при использовании LEVENSHTEIN()
* [Issue #743](https://github.com/manticoresoftware/manticoresearch/issues/743) Неожиданный сбой Manticore и невозможность нормального перезапуска
* [Issue #788](https://github.com/manticoresoftware/manticoresearch/issues/788) CALL KEYWORDS через /sql возвращает управляющий символ, который ломает JSON
* [Issue #789](https://github.com/manticoresoftware/manticoresearch/issues/789) mariadb не может создать таблицу FEDERATED
* [Issue #796](https://github.com/manticoresoftware/manticoresearch/issues/796) WARNING: dlopen() failed: /usr/bin/lib_manticore_columnar.so: cannot open shared object file: No such file or directory
* [Issue #797](https://github.com/manticoresoftware/manticoresearch/issues/797) Manticore вылетает при поиске с ZONESPAN через api
* [Issue #799](https://github.com/manticoresoftware/manticoresearch/issues/799) Неверный вес при использовании нескольких индексов и facet distinct
* [Issue #801](https://github.com/manticoresoftware/manticoresearch/issues/801) SphinxQL групповой запрос зависает после повторной обработки SQL индекса
* [Issue #802](https://github.com/manticoresoftware/manticoresearch/issues/802) MCL: Индексатор вылетает в версии 5.0.2 и manticore-columnar-lib 1.15.4
* [Issue #813](https://github.com/manticoresoftware/manticoresearch/issues/813) Manticore 5.0.2 FEDERATED возвращает пустой набор (MySQL 8.0.28)
* [Issue #824](https://github.com/manticoresoftware/manticoresearch/issues/824) select COUNT DISTINCT на 2 индексах при результате равном нулю вызывает внутреннюю ошибку
* [Issue #826](https://github.com/manticoresoftware/manticoresearch/issues/826) Сбой при выполнении запроса удаления
* [Issue #843](https://github.com/manticoresoftware/manticoresearch/issues/843) MCL: Ошибка с длинным текстовым полем
* [Issue #856](https://github.com/manticoresoftware/manticoresearch/issues/856) 5.0.2 rtindex: Поведение ограничения агрегированного поиска не соответствует ожиданиям
* [Issue #863](https://github.com/manticoresoftware/manticoresearch/issues/863) Возвращается объект типа NoneType, хотя поиск должен возвращать множество результатов
* [Issue #870](https://github.com/manticoresoftware/manticoresearch/issues/870) Сбой при использовании Attribute и Stored Field в выражении SELECT
* [Issue #872](https://github.com/manticoresoftware/manticoresearch/issues/872) таблица становится невидимой после сбоя
* [Issue #877](https://github.com/manticoresoftware/manticoresearch/issues/877) Два отрицательных термина в поисковом запросе вызывают ошибку: запрос невычислим
* [Issue #878](https://github.com/manticoresoftware/manticoresearch/issues/878) a -b -c не работает через json query_string
* [Issue #886](https://github.com/manticoresoftware/manticoresearch/issues/886) pseudo_sharding с query match
* [Issue #893](https://github.com/manticoresoftware/manticoresearch/issues/893) Функции min/max в Manticore 5.0.2 работают не так, как ожидалось...
* [Issue #896](https://github.com/manticoresoftware/manticoresearch/issues/896) Поле "weight" парсится некорректно
* [Issue #897](https://github.com/manticoresoftware/manticoresearch/issues/897) Сервис Manticore падает при запуске и постоянно перезапускается
* [Issue #900](https://github.com/manticoresoftware/manticoresearch/issues/900) group by j.a, smth работает неверно
* [Issue #913](https://github.com/manticoresoftware/manticoresearch/issues/913) Searchd падает при использовании expr в ranker, но только для запросов с двумя proximity
* [Issue #916](https://github.com/manticoresoftware/manticoresearch/issues/916) net_throttle_action не работает
* [Issue #919](https://github.com/manticoresoftware/manticoresearch/issues/919) MCL: Manticore падает при выполнении запроса и другие сбои при восстановлении кластера.
* [Issue #925](https://github.com/manticoresoftware/manticoresearch/issues/925) SHOW CREATE TABLE выводит без обратных кавычек
* [Issue #930](https://github.com/manticoresoftware/manticoresearch/issues/930) Теперь возможно выполнять запросы к Manticore из Java через JDBC коннектор
* [Issue #933](https://github.com/manticoresoftware/manticoresearch/issues/933) Проблемы ранжирования bm25f
* [Issue #934](https://github.com/manticoresoftware/manticoresearch/issues/934) Индексы без конфигурации застревают в watchdog в состоянии первого запуска
* [Issue #937](https://github.com/manticoresoftware/manticoresearch/issues/937) Segfault при сортировке данных фасетов
* [Issue #940](https://github.com/manticoresoftware/manticoresearch/issues/940) сбой при CONCAT(TO_STRING)
* [Issue #947](https://github.com/manticoresoftware/manticoresearch/issues/947) В некоторых случаях один простой select мог вызвать зависание всего инстанса, в результате чего нельзя было войти или выполнить другие запросы пока select не завершится.
* [Issue #948](https://github.com/manticoresoftware/manticoresearch/issues/948) Сбой индексации
* [Issue #950](https://github.com/manticoresoftware/manticoresearch/issues/950) Неправильный подсчет из facet distinct
* [Issue #953](https://github.com/manticoresoftware/manticoresearch/issues/953) LCS рассчитывается неправильно во встроенном ranker sph04
* [Issue #955](https://github.com/manticoresoftware/manticoresearch/issues/955) 5.0.3 dev падает
* [Issue #963](https://github.com/manticoresoftware/manticoresearch/issues/963) FACET с json на engine columnar приводит к сбою
* [Issue #982](https://github.com/manticoresoftware/manticoresearch/issues/982) MCL: 5.0.3 сбой из-за вторичного индекса
* [PR #984](https://github.com/manticoresoftware/manticoresearch/pull/984) @@autocommit в HandleMysqlSelectSysvar
* [PR #985](https://github.com/manticoresoftware/manticoresearch/pull/985) Исправлена распределение потоков по чанкам в RT индексах
* [Issue #985](https://github.com/manticoresoftware/manticoresearch/issues/985) Исправлена распределение потоков по чанкам в RT индексах
* [Issue #986](https://github.com/manticoresoftware/manticoresearch/issues/986) Неправильное значение по умолчанию max_query_time
* [Issue #987](https://github.com/manticoresoftware/manticoresearch/issues/987) Сбой при использовании regex выражения в многопоточном выполнении
* [Issue #988](https://github.com/manticoresoftware/manticoresearch/issues/988) Нарушена обратная совместимость индексами
* [Issue #989](https://github.com/manticoresoftware/manticoresearch/issues/989) indextool сообщает ошибку при проверке атрибутов columnar
* [Issue #990](https://github.com/manticoresoftware/manticoresearch/issues/990) Утечка памяти при клонировании json grouper
* [Issue #991](https://github.com/manticoresoftware/manticoresearch/issues/991) Утечка памяти при клонировании функции levenshtein
* [Issue #992](https://github.com/manticoresoftware/manticoresearch/issues/992) Ошибка теряется при загрузке meta
* [Issue #993](https://github.com/manticoresoftware/manticoresearch/issues/993) Проталкивание ошибок из динамических индексов/subkeys и sysvars
* [Issue #994](https://github.com/manticoresoftware/manticoresearch/issues/994) Сбой при count distinct по columnar строке в columnar storage
* [Issue #995](https://github.com/manticoresoftware/manticoresearch/issues/995) MCL: min(pickup_datetime) из taxi1 вызывает сбой
* [Issue #997](https://github.com/manticoresoftware/manticoresearch/issues/997) Пустой excludes JSON запрос удаляет колонки из списка select
* [Issue #998](https://github.com/manticoresoftware/manticoresearch/issues/998) Вторичные задачи, выполняемые на текущем планировщике, иногда вызывают аномальные побочные эффекты
* [Issue #999](https://github.com/manticoresoftware/manticoresearch/issues/999) сбой с facet distinct и разными схемами
* [Issue #1000](https://github.com/manticoresoftware/manticoresearch/issues/1000) MCL: Колончатый rt индекс повреждается после запуска без колончатой библиотеки
* [Issue #1001](https://github.com/manticoresoftware/manticoresearch/issues/1001) не работает неявный cutoff в json
* [Issue #1002](https://github.com/manticoresoftware/manticoresearch/issues/1002) Проблема с columnar grouper
* [Issue #1003](https://github.com/manticoresoftware/manticoresearch/issues/1003) Невозможно удалить последнее поле из индекса
* [Issue #1004](https://github.com/manticoresoftware/manticoresearch/issues/1004) Неправильное поведение после --new-cluster
* [Issue #1005](https://github.com/manticoresoftware/manticoresearch/issues/1005) "columnar library not loaded", хотя она не требуется
* [Issue #1006](https://github.com/manticoresoftware/manticoresearch/issues/1006) Нет ошибки для запроса delete
* [Issue #1010](https://github.com/manticoresoftware/manticoresearch/issues/1010) Исправлено расположение файла данных ICU в сборках для Windows
* [PR #1018](https://github.com/manticoresoftware/manticoresearch/pull/1018) Проблема отправки рукопожатия
* [Issue #1020](https://github.com/manticoresoftware/manticoresearch/issues/1020) Отображение id в show create table
* [Issue #1024 crash 1](https://github.com/manticoresoftware/manticoresearch/issues/1024) Сбой / Ошибка сегментации при поиске по фасетам с большим количеством результатов.
* [Issue #1026](https://github.com/manticoresoftware/manticoresearch/issues/1026) RT индекс: searchd "зависает" навсегда при вставке большого количества документов и переполнении RAMchunk
* [Commit 4739](https://github.com/manticoresoftware/manticoresearch/commit/4739dafd) Поток зависает при завершении, пока репликация занята между узлами
* [Commit ab87](https://github.com/manticoresoftware/manticoresearch/commit/ab87836782e7ae43fe5f9dea739eed0d336b42c1) Смешивание float и int в JSON range фильтре могло привести к игнорированию фильтра в Manticore
* [Commit d001](https://github.com/manticoresoftware/manticoresearch/commit/d00101c2905f6393ce3fca23c4f6dcb2506f4bd9) Float фильтры в JSON были неточными
* [Commit 4092](https://github.com/manticoresoftware/manticoresearch/commit/4092c6a8) Отбрасывать неподтвержденные транзакции, если индекс изменен (иначе может произойти сбой)
* [Commit 9692](https://github.com/manticoresoftware/manticoresearch/commit/9692a840) Ошибка синтаксиса запроса при использовании обратного слэша
* [Commit 0c19](https://github.com/manticoresoftware/manticoresearch/commit/0c19ad8d) workers_clients могли быть неверными в SHOW STATUS
* [Commit 1772](https://github.com/manticoresoftware/manticoresearch/commit/17720e1e2f2ec38337df565bee78f7ce9d9c744a) исправлен сбой при объединении ram сегментов без docstores
* [Commit f45b](https://github.com/manticoresoftware/manticoresearch/commit/f45b337b5c265433bb7d6262ad65d02f65edbaf3) Исправлено пропущенное условие ALL/ANY для equals JSON фильтра
* [Commit 3e83](https://github.com/manticoresoftware/manticoresearch/commit/3e83c1f0c0e6e7a9d4297113f69a112cbabd9e64) Репликация могла завершаться с ошибкой `got exception while reading ist stream: mkstemp(./gmb_pF6TJi) failed: 13 (Permission denied)`, если searchd был запущен из каталога, в который нельзя записывать.
* [Commit 92e5](https://github.com/manticoresoftware/manticoresearch/commit/92e5629f7150f62524b385fca670a01ba9590f5b) Начиная с версии 4.0.2 логи сбоев содержали только оффсеты. Этот коммит это исправляет.

## Версия 5.0.2
Выпущена: 30 мая 2022

### Исправления ошибок
* ❗[Issue #791](https://github.com/manticoresoftware/manticoresearch/issues/791) - неправильный размер стека мог вызвать сбой.

## Версия 5.0.0
Выпущена: 18 мая 2022


### Основные новые функции
* 🔬 Поддержка [Manticore Columnar Library 1.15.2](https://github.com/manticoresoftware/columnar/), которая включает бета-версию [Вторичных индексов](Server_settings/Searchd.md#secondary_indexes). Построение вторичных индексов включено по умолчанию для простых и реального времени колоннарных и построчных индексов (если используется [Manticore Columnar Library](https://github.com/manticoresoftware/columnar)), но для включения поиска по ним необходимо установить значение `secondary_indexes = 1` либо в вашем конфигурационном файле, либо с помощью команды [SET GLOBAL](Server_settings/Setting_variables_online.md). Новая функциональность поддерживается во всех операционных системах, кроме старых Debian Stretch и Ubuntu Xenial.
* [Режим только для чтения](Security/Read_only.md): теперь вы можете указать слушателей, которые обрабатывают только запросы на чтение, игнорируя любые записи.
* Новый [/cli](Connecting_to_the_server/HTTP.md#/cli) эндпоинт для еще более удобного выполнения SQL запросов по HTTP.
* Более быстрая массовая вставка INSERT/REPLACE/DELETE через JSON по HTTP: ранее можно было отправлять несколько команд записи через HTTP JSON протокол по одной, теперь они обрабатываются как одна транзакция.
* [#720](https://github.com/manticoresoftware/manticoresearch/issues/720) Поддержка [вложенных фильтров](Searching/Filters.md#Nested-bool-query) в JSON протоколе. Ранее нельзя было кодировать выражения вроде `a=1 and (b=2 or c=3)` в JSON: `must` (AND), `should` (OR) и `must_not` (NOT) работали только на верхнем уровне. Теперь их можно вкладывать.
* Поддержка [Chunked transfer encoding](https://en.wikipedia.org/wiki/Chunked_transfer_encoding) в HTTP протоколе. Теперь вы можете использовать chunked transfer в вашем приложении для передачи больших пакетов с меньшим расходом ресурсов (так как не нужно вычислять `Content-Length`). На серверной стороне Manticore теперь всегда обрабатывает входящие HTTP данные в потоковом режиме, без ожидания полной передачи пакета, что:
  - снижает пиковое использование RAM, уменьшая риск ошибки Out Of Memory
  - уменьшает время отклика (наши тесты показали снижение на 11% при обработке 100 МБ пакета)
  - позволяет обойти ограничение [max_packet_size](Server_settings/Searchd.md#max_packet_size) и передавать пакеты гораздо больше максимального разрешенного размера `max_packet_size` (128МБ), например, по 1ГБ за раз.
* [#719](https://github.com/manticoresoftware/manticoresearch/issues/719) HTTP интерфейс поддерживает `100 Continue`: теперь вы можете передавать большие пакеты через `curl` (включая curl библиотеки, используемые в различных языках программирования), который по умолчанию использует `Expect: 100-continue` и ждет некоторое время перед отправкой пакета. Ранее нужно было добавлять заголовок `Expect: `, теперь это не требуется.

  <details>

  Ранее (обратите внимание на время ответа):

  ```bash
  $ time curl -v -sX POST http://localhost:9318/bulk -H "Content-Type: application/x-ndjson" --data '{"insert": {"index": "user", "doc":  {"name":"Prof. Matt Heaney IV","email":"ibergnaum@yahoo.com","description":"Tempora ullam eaque consequatur. Vero aut minima ut et ut omnis officiis vel. Molestiae quis voluptatum sint numquam.","age":15,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"Prof. Boyd McKenzie","email":"carlotta11@hotmail.com","description":"Blanditiis maiores odio corporis eaque illum. Aut et rerum iste. Neque et ullam quisquam officia dignissimos quo cumque.","age":84,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"Г-н Иоганн Смит","email":"stiedemann.tristin@ziemann.com","description":"Temporibus amet magnam consequatur omnis consequatur illo fugit. Debitis natus doloremque est tempore deserunt vero. Harum eos corrupti nemo ut.","age":89,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"Гектор Поурос","email":"hickle.mafalda@hotmail.com","description":" as voluptatem inventore sit. Aliquam fugit perferendis est id aut odio et sapiente.","age":64,"active":1}}}'
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
  real
0m1.022s
  user
0m0.001s

  sys
0m0.010s
  ```
  Now:
  ```bash
  $ time curl -v -sX POST http://localhost:9318/bulk -H "Content-Type: application/x-ndjson" --data '{"insert": {"index": "user", "doc":  {"name":"Проф. Мэтт Хини IV","email":"ibergnaum@yahoo.com","description":"Tempora ullam eaque consequatur. Vero aut minima ut et ut omnis officiis vel. Molestiae quis voluptatum sint numquam.","age":15,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"Проф. Бойд Маккензи","email":"carlotta11@hotmail.com","description":"Blanditiis maiores odio corporis eaque illum. Aut et rerum iste. Neque et ullam quisquam officia dignissimos quo cumque.","age":84,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"Г-н Иоганн Смит","email":"stiedemann.tristin@ziemann.com","description":"Temporibus amet magnam consequatur omnis consequatur illo fugit. Debitis natus doloremque est tempore deserunt vero. Harum eos corrupti nemo ut.","age":89,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"Гектор Поурос","email":"hickle.mafalda@hotmail.com","description":" as voluptatem inventore sit. Aliquam fugit perferendis est id aut odio et sapiente.","age":64,"active":1}}}'
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
  real

0m0.015s

  user
0m0.005s
  sys
0m0.004s
  ```
  </details>

* **⚠️ КРУПНОЕ ИЗМЕНЕНИЕ**: [Псевдо-шардинг](Server_settings/Searchd.md#pseudo_sharding) включен по умолчанию. Если вы хотите его отключить, убедитесь, что вы добавили `pseudo_sharding = 0` в секцию `searchd` вашего конфигурационного файла Manticore.
* Наличие хотя бы одного полнотекстового поля в индексе реального времени/простой индексации теперь не обязательно. Вы теперь можете использовать Manticore даже в случаях, не связанных с полнотекстовым поиском.
* [Быстрое получение](Creating_a_table/Data_types.md#fast_fetch) для атрибутов, поддерживаемых [Manticore Columnar Library](https://github.com/manticoresoftware/columnar): запросы вида `select * from <columnar table>` теперь работают намного быстрее, чем раньше, особенно при большом количестве полей в схеме.
* **⚠️ КРУПНОЕ ИЗМЕНЕНИЕ**: Неявное [cutoff](Searching/Options.md#cutoff). Manticore теперь не тратит время и ресурсы на обработку данных, которые вам не нужны в возвращаемом наборе результатов. Минус в том, что это влияет на `total_found` в [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META) и [hits.total](Searching/Full_text_matching/Basic_usage.md#HTTP-JSON) в JSON-выводе. Оно теперь точно только в случае, если вы видите `total_relation: eq`, тогда как `total_relation: gte` означает, что фактическое число совпадающих документов больше значения `total_found`, которое вы получили. Чтобы сохранить прежнее поведение, вы можете использовать опцию поиска `cutoff=0`, что сделает `total_relation` всегда равным `eq`.
* **⚠️ КРУПНОЕ ИЗМЕНЕНИЕ**: Все полнотекстовые поля теперь по умолчанию являются [stored](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#stored_fields). Чтобы сделать все поля не хранимыми (т.е. вернуть прежнее поведение), используйте `stored_fields = ` (пустое значение).
* [#715](https://github.com/manticoresoftware/manticoresearch/issues/715) HTTP JSON поддерживает [опции поиска](Searching/Options.md#General-syntax).
### Незначительные изменения
* **⚠️ КРУПНОЕ ИЗМЕНЕНИЕ**: Изменен формат файла метаданных индекса. Раньше мета-файлы (`.meta`, `.sph`) были в бинарном формате, теперь это просто json. Новая версия Manticore автоматически конвертирует старые индексы, но:
  - вы можете получить предупреждение типа `WARNING: ... syntax error, unexpected TOK_IDENT`
  - вы не сможете запускать индекс в предыдущих версиях Manticore, убедитесь, что у вас есть резервная копия
* **⚠️ ВАЖНОЕ ИЗМЕНЕНИЕ**: Поддержка состояния сессии с помощью [HTTP keep-alive](Connecting_to_the_server/HTTP.md#Keep-alive). Это делает HTTP состояние, если клиент также его поддерживает. Например, используя новый эндпоинт [/cli](Connecting_to_the_server/HTTP.md#/cli) и HTTP keep-alive (включён по умолчанию во всех браузерах), вы можете вызвать `SHOW META` после `SELECT`, и это будет работать так же, как и через mysql. Ранее HTTP-заголовок `Connection: keep-alive` тоже поддерживался, но он только вызывал повторное использование того же соединения. С этой версии он также делает сессию состоянием.
* Теперь вы можете указывать `columnar_attrs = *`, чтобы определить все ваши атрибуты как колоночные в [plain mode](Read_this_first.md#Real-time-mode-vs-plain-mode), что полезно, если список длинный.
* Быстрее репликация SST
* **⚠️ ВАЖНОЕ ИЗМЕНЕНИЕ**: Протокол репликации был изменён. Если у вас запущен кластер репликации, то при обновлении до Manticore 5 нужно:
  - сначала корректно остановить все ваши узлы
  - затем запустить последний остановленный узел с параметром `--new-cluster` (в Linux используйте инструмент `manticore_new_cluster`).
  - подробнее читайте про [перезапуск кластера](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster).
* Улучшения в репликации:
  - Быстрее SST
  - Защита от шумов, что помогает при нестабильном соединении между узлами репликации
  - Улучшенное логирование
* Улучшения безопасности: теперь Manticore слушает на `127.0.0.1` вместо `0.0.0.0`, если в конфиге не указано `listen`. Хотя в стандартной конфигурации, поставляемой с Manticore Search, `listen` указан, а конфигурация без `listen` нечасто встречается, такая возможность существует. Ранее Manticore слушал на `0.0.0.0`, что небезопасно, теперь же слушает на `127.0.0.1`, что обычно не доступно из интернета.
* Быстрее агрегация по колоночным атрибутам.
* Повышена точность `AVG()`: раньше Manticore использовал `float` для агрегаций, теперь используется `double`, что значительно повышает точность.
* Улучшена поддержка JDBC MySQL драйвера.
* Поддержка `DEBUG malloc_stats` для [jemalloc](https://github.com/jemalloc/jemalloc).
* [optimize_cutoff](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#optimize_cutoff) теперь доступна как настройка на таблицу, которую можно установить при создании или изменении таблицы.
* **⚠️ ВАЖНОЕ ИЗМЕНЕНИЕ**: [query_log_format](Server_settings/Searchd.md#query_log_format) теперь **по умолчанию `sphinxql`**. Если вы привыкли к формату `plain`, добавьте в конфиг `query_log_format = plain`.
* Значительно снижено потребление памяти: Manticore теперь потребляет гораздо меньше ОЗУ при длинных и интенсивных вставках/заменах/оптимизациях, если используются сохранённые поля.
* Значение по умолчанию [shutdown_timeout](Server_settings/Searchd.md#shutdown_timeout) увеличено с 3 до 60 секунд.

* [Коммит ffd0](https://github.com/manticoresoftware/manticoresearch/commit/ffd0499d329d2c383f14c8a44c4cc84338ab56e7) Поддержка Java mysql connector >= 6.0.3: в [Java mysql connection 6.0.3](https://mvnrepository.com/artifact/mysql/mysql-connector-java/6.0.3) изменён способ подключения к mysql, что нарушило совместимость с Manticore. Новый способ теперь поддерживается.
* [Коммит 1da6](https://github.com/manticoresoftware/manticoresearch/commit/1da6dbec) отключено сохранение нового дискового куска при загрузке индекса (например, при старте searchd).
* [Issue #746](https://github.com/manticoresoftware/manticoresearch/issues/746) Поддержка glibc >= 2.34.
* [Issue #784](https://github.com/manticoresoftware/manticoresearch/issues/784) подсчёт 'VIP' подключений отдельно от обычных (не-VIP). Ранее VIP подключения учитывались в лимит `max_connections`, что могло вызывать ошибку «maxed out» для обычных подключений. Теперь VIP подключения не учитываются в лимит. Текущее число VIP подключений также видно в `SHOW STATUS` и `status`.
* Теперь можно явно указывать [ID](Creating_a_table/Data_types.md#Document-ID).
* [Issue #687](https://github.com/manticoresoftware/manticoresearch/issues/687) поддержка сжатия zstd для mysql proto.
### ⚠️ Другие незначительные изменения с нарушением обратной совместимости
* ⚠️ Формула BM25F была слегка обновлена для улучшения релевантности поиска. Это влияет только на результаты поиска при использовании функции [BM25F()](Functions/Searching_and_ranking_functions.md#BM25F%28%29), поведение формулы ранжирования по умолчанию не изменилось.
* ⚠️ Изменено поведение REST эндпоинта [/sql](Connecting_to_the_server/HTTP.md#mode=raw): `/sql?mode=raw` теперь требует экранирования и возвращает массив.

* ⚠️ Изменён формат ответа для `/bulk` запросов INSERT/REPLACE/DELETE:
  - ранее каждый подзапрос был отдельной транзакцией и имел отдельный ответ
  - теперь весь пакет считается одной транзакцией и возвращает единый ответ
* ⚠️ Опции поиска `low_priority` и `boolean_simplify` теперь требуют значения (`0/1`): раньше можно было писать `SELECT ... OPTION low_priority, boolean_simplify`, теперь нужно `SELECT ... OPTION low_priority=1, boolean_simplify=1`.
* ⚠️ Если вы используете старые [php](https://github.com/manticoresoftware/manticoresearch/blob/master/api/sphinxapi.php), [python](https://github.com/manticoresoftware/manticoresearch/blob/master/api/sphinxapi.py) или [java](https://github.com/manticoresoftware/manticoresearch/tree/master/api/java) клиенты, пожалуйста, перейдите по соответствующей ссылке и найдите обновлённую версию. **Старые версии не полностью совместимы с Manticore 5.**
* ⚠️ HTTP JSON-запросы теперь логируются в другом формате при режиме `query_log_format=sphinxql`. Ранее логировалась только полнотекстовая часть, теперь логируется полностью.
### Новые пакеты
* **⚠️ ВАЖНОЕ ИЗМЕНЕНИЕ**: из-за новой структуры при обновлении до Manticore 5 рекомендуется удалить старые пакеты перед установкой новых:
  - В RPM-системах: `yum remove manticore*`

  - В Debian и Ubuntu: `apt remove manticore*`
* Новая структура пакетов deb/rpm. Предыдущие версии предоставляли:
  - `manticore-server` с `searchd` (основной демон поиска) и всем необходимым для него
  - `manticore-tools` с `indexer` и `indextool`
  - `manticore`, включающий всё
  - `manticore-all` RPM как мета-пакет, ссылающийся на `manticore-server` и `manticore-tools`
  Новая структура:
  - `manticore` — мета-пакет deb/rpm, который устанавливает всё вышеуказанное как зависимости
  - `manticore-server-core` — `searchd` и всё для его отдельного запуска
  - `manticore-server` — systemd-файлы и другие вспомогательные скрипты

  - `manticore-tools` — `indexer`, `indextool` и другие инструменты
  - `manticore-common` — конфигурационный файл по умолчанию, каталог данных по умолчанию, стоп-слова по умолчанию
  - `manticore-icudata`, `manticore-dev`, `manticore-converter` практически не изменились
  - архив `.tgz`, включающий все пакеты
* Поддержка Ubuntu Jammy
* Поддержка Amazon Linux 2 через [YUM repo](Installation/RHEL_and_Centos.md#YUM-repository)
### Исправления ошибок
* [Issue #815](https://github.com/manticoresoftware/manticoresearch/issues/815) Случайный сбой при использовании UDF-функции
* [Issue #287](https://github.com/manticoresoftware/manticoresearch/issues/287) нехватка памяти при индексировании RT индекса
* [Issue #604](https://github.com/manticoresoftware/manticoresearch/issues/604) Ломающее изменение sphinxql-parser в версиях 3.6.0, 4.2.0
* [Issue #667](https://github.com/manticoresoftware/manticoresearch/issues/667) FATAL: недостаточно памяти (невозможно выделить 9007199254740992 байт)
* [Issue #676](https://github.com/manticoresoftware/manticoresearch/issues/676) Строки неправильно передаются в UDF
* ❗[Issue #698](https://github.com/manticoresoftware/manticoresearch/issues/698) Сбой searchd после попытки добавить текстовый столбец в rt индекс
* [Issue #705](https://github.com/manticoresoftware/manticoresearch/issues/705) Indexer не мог найти все столбцы
* ❗[Issue #709](https://github.com/manticoresoftware/manticoresearch/issues/705) Группировка по json.boolean работает неправильно
* [Issue #716](https://github.com/manticoresoftware/manticoresearch/issues/716) Ошибки команд indextool связанных с индексом (например, --dumpdict)
* ❗[Issue #724](https://github.com/manticoresoftware/manticoresearch/issues/724) Поля исчезают из выборки
* [Issue #727](https://github.com/manticoresoftware/manticoresearch/issues/727) Совместимость Content-Type .NET HttpClient при использовании `application/x-ndjson`
* [Issue #729](https://github.com/manticoresoftware/manticoresearch/issues/729) Вычисление длины поля
* ❗[Issue #730](https://github.com/manticoresoftware/manticoresearch/issues/730) create/insert into/drop columnar table вызывает утечку памяти
* [Issue #731](https://github.com/manticoresoftware/manticoresearch/issues/731) Пустой столбец в результатах при определённых условиях
* ❗[Issue #749](https://github.com/manticoresoftware/manticoresearch/issues/749) Сбой демона при запуске
* ❗[Issue #750](https://github.com/manticoresoftware/manticoresearch/issues/750) Зависание демона при запуске
* ❗[Issue #751](https://github.com/manticoresoftware/manticoresearch/issues/751) Сбой при SST
* [Issue #752](https://github.com/manticoresoftware/manticoresearch/issues/752) Json-атрибут помечается как columnar, когда engine='columnar'
* [Issue #753](https://github.com/manticoresoftware/manticoresearch/issues/753) Репликация слушает 0
* [Issue #754](https://github.com/manticoresoftware/manticoresearch/issues/754) columnar_attrs = * не работает с csvpipe
* ❗[Issue #755](https://github.com/manticoresoftware/manticoresearch/issues/755) Сбой при выборе float в columnar в rt
* ❗[Issue #756](https://github.com/manticoresoftware/manticoresearch/issues/756) Indextool изменяет rt индекс во время проверки
* [Issue #757](https://github.com/manticoresoftware/manticoresearch/issues/757) Необходима проверка пересечения диапазонов портов слушателей
* [Issue #758](https://github.com/manticoresoftware/manticoresearch/issues/758) Логирование исходной ошибки, если RT индекс не смог сохранить часть на диск
* [Issue #759](https://github.com/manticoresoftware/manticoresearch/issues/759) Сообщается только одна ошибка для конфигурации RE2
* ❗[Issue #760](https://github.com/manticoresoftware/manticoresearch/issues/760) Изменения потребления оперативной памяти в коммите 5463778558586d2508697fa82e71d657ac36510f
* [Issue #761](https://github.com/manticoresoftware/manticoresearch/issues/761) Третий узел не создаёт некластерный кластер после грязной перезагрузки
* [Issue #762](https://github.com/manticoresoftware/manticoresearch/issues/762) Счётчик обновлений увеличивается на 2
* [Issue #763](https://github.com/manticoresoftware/manticoresearch/issues/763) Новая версия 4.2.1 повреждает индекс, созданный с 4.2.0 с использованием морфологии
* [Issue #764](https://github.com/manticoresoftware/manticoresearch/issues/764) Отсутствует экранирование в ключах json /sql?mode=raw
* ❗[Issue #765](https://github.com/manticoresoftware/manticoresearch/issues/765) Использование функции скрывает другие значения
* ❗[Issue #766](https://github.com/manticoresoftware/manticoresearch/issues/766) Утечка памяти вызванная строкой в FixupAttrForNetwork
* ❗[Issue #767](https://github.com/manticoresoftware/manticoresearch/issues/767) Утечка памяти в версиях 4.2.0 и 4.2.1, связанная с кешем docstore
* [Issue #768](https://github.com/manticoresoftware/manticoresearch/issues/768) Странный пин-понг с сохранёнными полями по сети
* [Issue #769](https://github.com/manticoresoftware/manticoresearch/issues/769) lemmatizer_base сбрасывается в пустое состояние, если не указан в разделе 'common'
* [Issue #770](https://github.com/manticoresoftware/manticoresearch/issues/770) pseudo_sharding замедляет SELECT по id
* [Issue #771](https://github.com/manticoresoftware/manticoresearch/issues/771) DEBUG malloc_stats выводит нули при использовании jemalloc
* [Issue #772](https://github.com/manticoresoftware/manticoresearch/issues/772) Drop/add столбец делает значение невидимым
* [Issue #773](https://github.com/manticoresoftware/manticoresearch/issues/773) Невозможно добавить столбец bit(N) в columnar таблицу
* [Issue #774](https://github.com/manticoresoftware/manticoresearch/issues/774) "cluster" становится пустым при запуске в manticore.json

* ❗[Commit 1da4](https://github.com/manticoresoftware/manticoresearch/commits/1da4ce891e4ff13727cfd3331d72771c64ee7949) HTTP действия не отслеживаются в SHOW STATUS

* [Commit 3810](https://github.com/manticoresoftware/manticoresearch/commits/381000ab4af4c3c98ec8c730699ad7f39039cec8) отключение pseudo_sharding для редко встречающихся запросов с одним ключевым словом
* [Commit 8003](https://github.com/manticoresoftware/manticoresearch/commits/800325cca283a87801b28929d82420fafc76a0ee) исправлена обработка сохраненных атрибутов при слиянии индексов

* [Commit cddf](https://github.com/manticoresoftware/manticoresearch/commits/cddfeed6296f2041c6aae18414e5cd9cabf08281) обобщены механизмы получения уникальных значений; добавлены специализированные методы для столбцовых строк

* [Commit fba4](https://github.com/manticoresoftware/manticoresearch/commits/fba4bb4f) исправлено получение null целочисленных атрибутов из docstore

* [Commit f300](https://github.com/manticoresoftware/manticoresearch/commit/f3009a9242fa16956adf871c4887e6e0303ba364) `ranker` мог быть указан дважды в логах запросов
## Версия 4.2.0, 23 декабря 2021

### Основные новые функции
* **Поддержка Псевдо-шардинга для RT индексах и полнотекстовых запросах**. В предыдущем релизе была добавлена ограниченная поддержка псевдошардинга. Начиная с этой версии, вы можете получить все преимущества псевдошардинга и вашего многоядерного процессора, просто включив [searchd.pseudo_sharding](Server_settings/Searchd.md#pseudo_sharding). Самое крутое — вам не нужно ничего менять в индексах или запросах, просто включите эту опцию, и если у вас есть свободный CPU, он будет использован для уменьшения времени отклика. Поддерживаются плоские и RT индексы для полнотекстовых, фильтрационных и аналитических запросов. Например, вот как включение псевдошардинга может сделать среднее **время отклика большинства запросов примерно в 10 раз меньше** на [наборе данных Hacker news с курированными комментариями](https://zenodo.org/record/45901/) умноженном в 100 раз (116 миллионов документов в плоском индексе).

![Псевдошардинг включен и выключен в 4.2.0](4.2.0_ps_on_vs_off.png)

* [**Debian Bullseye**](https://manticoresearch.com/install/) теперь поддерживается.

<!-- example pq_transactions_4.2.0 -->
* Транзакции PQ теперь атомарны и изолированы. Ранее поддержка транзакций PQ была ограничена. Это обеспечивает намного **быстрее REPLACE в PQ**, особенно когда нужно сразу заменить много правил. Детали производительности:

<!-- intro -->
#### 4.0.2
<!-- request 4.0.2 -->
Для вставки 1M правил PQ требуется **48 секунд**, а чтобы заменить всего 40K за 10K партий — **406 секунд**.
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
Для вставки 1M правил PQ требуется **34 секунды**, а чтобы заменить их — **23 секунды** за 10K партий.
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
### Мелкие изменения
*  [optimize_cutoff](Server_settings/Searchd.md#optimize_cutoff) теперь доступен как опция конфигурации в разделе `searchd`. Полезно, когда нужно ограничить число RT чанков во всех ваших индексах до определенного числа глобально.
* [Commit 0087](https://github.com/manticoresoftware/manticoresearch/commit/00874743823eaf43e2a09a088392fd7e0a617f47) точный подсчет [count(distinct ...)](Searching/Grouping.md#COUNT%28DISTINCT-field%29) и [FACET ... distinct](Searching/Faceted_search.md#Faceting-without-duplicates) по нескольким локальным физическим индексам (real-time/plain) с одинаковым набором/порядком полей.
* [PR #598](https://github.com/manticoresoftware/manticoresearch/pull/598) поддержка bigint для `YEAR()` и других функций для временных меток.

* [Commit 8e85](https://github.com/manticoresoftware/manticoresearch/commit/8e85d4bce596cc88f9d9615ea243a27e4afde0f7) Адаптивный [rt_mem_limit](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_mem_limit). Ранее Manticore Search собирал ровно до `rt_mem_limit` данных перед сохранением нового дискового чанка, а во время сохранения собирал еще до 10% сверх (так называемый двойной буфер), чтобы минимизировать возможную приостановку вставки. Если этот лимит тоже исчерпывался, добавление новых документов блокировалось до полного сохранения дискового чанка. Новый адаптивный лимит основан на том, что теперь у нас есть [auto-optimize](Server_settings/Searchd.md#auto_optimize), поэтому не страшно, если дисковые чанки не полностью соответствуют `rt_mem_limit` и начинают сбрасываться раньше. Теперь мы собираем до 50% `rt_mem_limit` и сохраняем это как дискчанг. При сохранении смотрим на статистику (сколько сохранили, сколько новых документов пришло во время сохранения) и пересчитываем исходную скорость, которая будет использована в следующий раз. Например, если сохранили 90 миллионов документов, а во время сохранения пришло еще 10 миллионов, скорость — 90%, значит в следующий раз можно собирать до 90% от `rt_mem_limit` перед началом сброса очередного дискового чанка. Значение скорости рассчитывается автоматически от 33,3% до 95%.
* [Issue #628](https://github.com/manticoresoftware/manticoresearch/issues/628) [unpack_zlib](Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Database_connection.md#unpack_zlib) для источника PostgreSQL. Спасибо, [Dmitry Voronin](https://github.com/dimv36) за [вклад](https://github.com/manticoresoftware/manticoresearch/pull/630).
* [Commit 6d54](https://github.com/manticoresoftware/manticoresearch/commit/6d54cf2b319b131970c29410cc21abcbdf8083b1) `indexer -v` и `--version`. Ранее вы могли видеть версию индексатора, но `-v`/`--version` не поддерживались.
* [Issue #662](https://github.com/manticoresoftware/manticoresearch/issues/662) бесконечный лимит mlock по умолчанию при запуске Manticore через systemd.
* [Commit 63c8](https://github.com/manticoresoftware/manticoresearch/commit/63c8cd05cf399e705c2c14939411a92cb784735a) spinlock -> очередь операций для coro rwlock.

* [Commit 4113](https://github.com/manticoresoftware/manticoresearch/commit/41130ce30d1734bbf8e6b8b63c4e815caf9464f2) переменная окружения `MANTICORE_TRACK_RT_ERRORS` полезна для отладки повреждений сегментов RT.
### Нарушающие совместимость изменения
* Версия Binlog была повышена, binlog из предыдущей версии не будет воспроизведен, поэтому убедитесь, что вы корректно остановили Manticore Search во время обновления: после остановки предыдущего экземпляра в `/var/lib/manticore/binlog/` не должно оставаться binlog файлов, кроме `binlog.meta`.
* [Коммит 3f65](https://github.com/manticoresoftware/manticoresearch/commit/3f659f36e87d99ee262205a8eae4663a255be630) новый столбец "chain" в `show threads option format=all`. Он показывает стек некоторых задач информационных тикетов, наиболее полезный для целей профилирования, так что если вы разбираете вывод `show threads`, имейте в виду о новом столбце.
* Настройка `searchd.workers` устарела с версии 3.5.0, теперь она объявлена deprecated, если она все еще присутствует в вашем конфигурационном файле, при старте будет выдано предупреждение. Manticore Search запустится, но с предупреждением.
* Если вы используете PHP и PDO для доступа к Manticore, вам нужно установить `PDO::ATTR_EMULATE_PREPARES`
### Исправление ошибок
* ❗[Проблема #650](https://github.com/manticoresoftware/manticoresearch/issues/650) Manticore 4.0.2 медленнее, чем Manticore 3.6.3. 4.0.2 была быстрее предыдущих версий по массовым вставкам, но значительно медленнее для одиночных вставок документов. Это исправлено в версии 4.2.0.
* ❗[Коммит 22f4](https://github.com/manticoresoftware/manticoresearch/commit/22f4141bed1411c83684ae6e0bae832071cce3df) RT индекс мог быть поврежден под интенсивной нагрузкой REPLACE или мог крашиться
* [Коммит 03be](https://github.com/manticoresoftware/manticoresearch/commit/03be91e4f720a3f20bee0278dd8f0d44abd3ef0e) исправлена средняя при слиянии группировщиков и сортировщика group N; исправлено слияние агрегатов
* [Коммит 2ea5](https://github.com/manticoresoftware/manticoresearch/commit/2ea575d34efe38661f14d858f2b552456f29fe42) `indextool --check` мог крашиться
* [Коммит 7ec7](https://github.com/manticoresoftware/manticoresearch/commit/7ec76d4abf01b5fca6f10a0000fe64d5d8e5fa52) проблема исчерпания ОЗУ вызванная UPDATE
* [Коммит 658a](https://github.com/manticoresoftware/manticoresearch/commit/658a727e41f3c3fbcde9754149528f34ed76ba19) демон мог зависать при INSERT
* [Коммит 46e4](https://github.com/manticoresoftware/manticoresearch/commit/46e42b9b386e31e53247ed634e158b533e28063e) демон мог зависать при завершении работы
* [Коммит f8d7](https://github.com/manticoresoftware/manticoresearch/commit/f8d7d5177cc83d250b493b4cfc619732b29cf072) демон мог упасть при завершении работы
* [Коммит 733a](https://github.com/manticoresoftware/manticoresearch/commit/733accf165142a1d1574689525ccca8907971158) демон мог зависать при краше
* [Коммит f7f8](https://github.com/manticoresoftware/manticoresearch/commit/f7f8bd8c0dbb23a59a88502dcba16b0918fb2330) демон мог упасть при старте пытаясь повторно войти в кластер с некорректным списком узлов
* [Коммит 1401](https://github.com/manticoresoftware/manticoresearch/commit/140155613821ecaddcd1a3dfc4f7369b0df6aef7) распределённый индекс мог быть полностью забыт в RT режиме, если не удавалось разрешить одного из его агентов при старте
* [Проблема #683](https://github.com/manticoresoftware/manticoresearch/issues/683) attr bit(N) engine='columnar' не работает
* [Проблема #682](https://github.com/manticoresoftware/manticoresearch/issues/682) create table завершается неудачей, но оставляет директорию
* [Проблема #663](https://github.com/manticoresoftware/manticoresearch/issues/663) Конфигурация не запускается с ошибкой: неизвестное имя ключа 'attr_update_reserve'
* [Проблема #632](https://github.com/manticoresoftware/manticoresearch/issues/632) Краш Manticore на пакетных запросах

* [Проблема #679](https://github.com/manticoresoftware/manticoresearch/issues/679) Пакетные запросы продолжают вызывать краши с версией 4.0.3

* [Коммит f7f8](https://github.com/manticoresoftware/manticoresearch/commit/f7f8bd8c0dbb23a59a88502dcba16b0918fb2330) исправлен краш демона при старте пытаясь повторно войти в кластер с некорректным списком узлов
* [Проблема #643](https://github.com/manticoresoftware/manticoresearch/issues/643) Manticore 4.0.2 не принимает подключения после серии вставок
* [Проблема #635](https://github.com/manticoresoftware/manticoresearch/issues/635) FACET запрос с ORDER BY JSON.field или строковым атрибутом может вызвать краш
* [Проблема #634](https://github.com/manticoresoftware/manticoresearch/issues/634) Краш SIGSEGV на запросе с packedfactors
* [Коммит 4165](https://github.com/manticoresoftware/manticoresearch/commits/41657f15f8fcf1e93cfad7f835bcf705d6e8349e) morphology_skip_fields не поддерживался при create table
## Версия 4.0.2, 21 сентября 2021
### Основные новые возможности
- **Полная поддержка [Manticore Columnar Library](https://github.com/manticoresoftware/columnar)**. Ранее Manticore Columnar Library поддерживалась только для простых индексов. Теперь она поддерживается:
  - в real-time индексах для `INSERT`, `REPLACE`, `DELETE`, `OPTIMIZE`

  - в репликации

  - в `ALTER`
  - в `indextool --check`
- **Автоматическая компактизация индексов** ([Проблема #478](https://github.com/manticoresoftware/manticoresearch/issues/478)). Наконец, вам не нужно вызывать OPTIMIZE вручную, через crontab или другую автоматизацию. Manticore теперь делает это автоматически и по умолчанию. Вы можете задать порог компактизации по умолчанию через глобальную переменную [optimize_cutoff](Server_settings/Setting_variables_online.md).
- **Переработка системы снимков чанков и блокировок**. Эти изменения на первый взгляд могут быть незаметны, но существенно улучшают поведение многих операций в real-time индексах. Вкратце — ранее большинство операций манипуляций с данными Manticore сильно зависели от блокировок, теперь мы используем снимки дисковых чанков.
- **Значительно более быстрая массовая вставка (bulk INSERT) в real-time индекс**. Например, на сервере [Hetzner AX101](https://www.hetzner.com/dedicated-rootserver/ax101) с SSD, 128 ГБ ОЗУ и AMD Ryzen™ 9 5950X (16*2 ядра) **в версии 3.6.0 можно было получить 236K документов в секунду**, вставляя документы в таблицу со схемой `name text, email string, description text, age int, active bit(1)` (значение по умолчанию `rt_mem_limit`, размер батча 25000, 16 параллельных вставляющих воркеров, всего вставлено 16 миллионов документов). В версии 4.0.2 при той же параллельности/размере батча/количестве достигается **357K документов в секунду**.
  <details>
  - операции чтения (например SELECTы, репликация) выполняются с использованием снимков

  - операции, которые изменяют только внутреннюю структуру индекса без изменения схемы/документов (например, слияние RAM-сегментов, сохранение дисковых чанков, слияние дисковых чанков) выполняются с использованием снимков только для чтения и в конце заменяют существующие чанки

  - UPDATE и DELETE выполняются над существующими чанкми, но в случае слияния запись собирается и затем применяется к новым чанкам
  - UPDATE последовательно захватывает эксклюзивный лок на каждый чанк. Слияния захватывают общий лок при переходе к этапу сбора атрибутов из чанка. Таким образом, в одно и то же время только одна операция (слияние или обновление) имеет доступ к атрибутам чанка.

  - когда слияние переходит к фазе, когда нужны атрибуты, оно устанавливает специальный флаг. Когда UPDATE заканчивается, он проверяет флаг, и если он установлен, всё обновление сохраняется в специальной коллекции. В конце, когда слияние завершено, обновления применяются к новообразованному дисковому чанку.
  - ALTER выполняется с эксклюзивным локом
  - репликация работает как обычная операция чтения, но дополнительно сохраняет атрибуты перед SST и запрещает обновления во время SST
  </details>
- **[ALTER](Updating_table_schema_and_settings.md) может добавить/удалить полнотекстовое поле** (в RT-режиме). Ранее он мог только добавить/удалить атрибут.


- 🔬 **Экспериментально: псевдо-шардинг для полноскановых запросов** — позволяет распараллеливать любой поиск, не связанный с полнотекстовым поиском. Вместо ручной подготовки шардов теперь можно просто включить новую опцию [searchd.pseudo_sharding](Server_settings/Searchd.md#pseudo_sharding) и ждать до `CPU cores` раз меньше времени отклика для запросов без полнотекстового поиска. Обратите внимание, что это может полностью занять все доступные CPU ядра, поэтому если вам важна не только задержка, но и пропускная способность — используйте с осторожностью.
### Мелкие изменения

<!-- example -->
- Linux Mint и Ubuntu Hirsute Hippo поддерживаются через [APT репозиторий](Installation/Debian_and_Ubuntu.md#APT-repository)
- более быстрое обновление по id через HTTP в больших индексах в некоторых случаях (зависит от распределения id)

- [671e65a2](https://github.com/manticoresoftware/lemmatizer-uk/commit/671e65a296800537123a3f8b324eeaa3f58b9632) — добавлено кэширование для lemmatizer-uk
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
- [кастомные флаги запуска для systemd](Starting_the_server/Linux.md#Custom-startup-flags-using-systemd). Теперь не нужно запускать searchd вручную, если нужно запустить Manticore с определённым флагом запуска

- новая функция [LEVENSHTEIN()](Functions/String_functions.md#LEVENSHTEIN%28%29), которая вычисляет расстояние Левенштейна
- добавлены новые [флаги запуска searchd](Starting_the_server/Manually.md#searchd-command-line-options) `--replay-flags=ignore-trx-errors` и `--replay-flags=ignore-all-errors`, чтобы можно было запустить searchd даже если binlog повреждён
- [Issue #621](https://github.com/manticoresoftware/manticoresearch/issues/621) — отображение ошибок из RE2
- более точный [COUNT(DISTINCT)](Searching/Grouping.md#COUNT%28DISTINCT-field%29) для распределённых индексов, состоящих из локальных обычных индексов
- [FACET DISTINCT](Searching/Faceted_search.md#Faceting-without-duplicates) для удаления дубликатов при фасетном поиске
- [модификатор точной формы](Searching/Full_text_matching/Operators.md#Exact-form-modifier) теперь не требует [морфологии](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) и работает для индексов с включённым [infix/prefix](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) поиском
### Ломающие изменения
- новая версия может читать старые индексы, но старые версии не могут читать индексы Manticore 4

- убрана неявная сортировка по id. Сортируйте явно, если это необходимо
- значение по умолчанию `charset_table` изменяется с `0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451` на `non_cjk`
- `OPTIMIZE` происходит автоматически. Если не нужно, установите `auto_optimize=0` в секции `searchd` конфигурационного файла
- [Issue #616](https://github.com/manticoresoftware/manticoresearch/issues/616) `ondisk_attrs_default` устаревшие, теперь удалены
- для разработчиков: теперь для сборок Linux используется компилятор Clang, так как по результатам тестов он строит более быструю версию Manticore Search и Manticore Columnar Library
- если в поисковом запросе не указан [max_matches](Searching/Options.md#max_matches), он неявно обновляется до минимально необходимого для производительности нового колоночного хранилища значения. Это может повлиять на метрику `total` в [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META), но не `total_found`, который соответствует реальному числу найденных документов.
### Миграция с Manticore 3
- убедитесь, что вы корректно остановили Manticore 3:
  - в папке `/var/lib/manticore/binlog/` не должно быть файлов binlog (только `binlog.meta` должен быть в директории)

  - иначе индексы, для которых Manticore 4 не может проиграть бинлоги, не будут запущены
- новая версия может читать старые индексы, но старые версии не могут читать индексы Manticore 4, поэтому обязательно сделайте резервную копию, если хотите иметь возможность откатиться
- если вы используете кластер репликации, убедитесь, что:
  - сначала корректно остановлены все узлы
  - затем запущен последний остановленный узел с опцией `--new-cluster` (запустите утилиту `manticore_new_cluster` в Linux)
  - прочитайте подробнее о [перезапуске кластера](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster)
### Исправления ошибок
- Исправлено множество проблем с репликацией:
  - [Commit 696f](https://github.com/manticoresoftware/manticoresearch/commit/696f8649535f2b5285b878c9581c0d2b1139ae09) — исправлен краш во время SST на присоединяющемся узле с активным индексом; добавлена sha1 проверка на узле присоединения при записи чанков файлов для ускорения загрузки индекса; добавлена ротация изменённых файлов индекса при загрузке индекса на узле присоединения; добавлено удаление файлов индекса на узле присоединения, когда активный индекс заменяется новым индексом с узла донора; добавлены точки журнала репликации на узле донора для отправки файлов и чанков
  - [Коммит b296](https://github.com/manticoresoftware/manticoresearch/commit/b296c55af68314a8fc66f8b104e7301d673c3b68) - сбой при JOIN CLUSTER в случае неверного адреса
  - [Коммит 418b](https://github.com/manticoresoftware/manticoresearch/commit/418bf880a5e05ffbb68f311a1c23006a9df3220e) - при начальной репликации большого индекса присоединяющийся узел мог выдать ошибку `ERROR 1064 (42000): invalid GTID, (null)`, донор мог стать неотзывчивым во время присоединения другого узла
  - [Коммит 6fd3](https://github.com/manticoresoftware/manticoresearch/commit/6fd350d28e4905211cf8a7298cbf7cdda101c83d) - хэш мог вычисляться неправильно для большого индекса, что могло привести к сбою репликации
  - [Задача #615](https://github.com/manticoresoftware/manticoresearch/issues/615) - сбой репликации при перезапуске кластера
- [Задача #574](https://github.com/manticoresoftware/manticoresearch/issues/574) - `indextool --help` не отображает параметр `--rotate`
- [Задача #578](https://github.com/manticoresoftware/manticoresearch/issues/578) - высокий загрузка CPU searchd при простое после примерно суток работы
- [Задача #587](https://github.com/manticoresoftware/manticoresearch/issues/587) - немедленная запись flush .meta
- [Задача #617](https://github.com/manticoresoftware/manticoresearch/issues/617) - manticore.json опустошается
- [Задача #618](https://github.com/manticoresoftware/manticoresearch/issues/618) - searchd --stopwait не работает под root. Также исправлено поведение systemctl (ранее он показывал ошибку для ExecStop и недостаточно долго ждал корректной остановки searchd)
- [Задача #619](https://github.com/manticoresoftware/manticoresearch/issues/619) - INSERT/REPLACE/DELETE и SHOW STATUS. `command_insert`, `command_replace` и другие показывали неверные метрики
- [Задача #620](https://github.com/manticoresoftware/manticoresearch/issues/620) - неверное значение по умолчанию для `charset_table` в plain-индексе
- [Коммит 8f75](https://github.com/manticoresoftware/manticoresearch/commit/8f7536887d4b58a5bab5647fc067b2d0482c2038) - новые дисковые чанки не блокируются в памяти (mlocked)
- [Задача #607](https://github.com/manticoresoftware/manticoresearch/issues/607) - сбой узла кластера Manticore при невозможности разрешить имя узла


- [Задача #623](https://github.com/manticoresoftware/manticoresearch/issues/623) - репликация обновлённого индекса может привести к неопределённому состоянию
- [Коммит ca03](https://github.com/manticoresoftware/manticoresearch/commit/ca03d2280c5197e9b311f0f03f551e3b702a8130) - indexer мог зависнуть при индексации plain-источника с JSON-атрибутом

- [Коммит 53c7](https://github.com/manticoresoftware/manticoresearch/commit/53c753054558ff3f82c7af365273d32537c5338b) - исправлен фильтр выражения "не равно" в PQ индексе
- [Коммит ccf9](https://github.com/manticoresoftware/manticoresearch/commit/ccf94e022ba6763d6286ab5bac767b6f6a846846) - исправлена выборка окон в лист-запросах с более чем 1000 совпадениями. `SELECT * FROM pq ORDER BY id desc LIMIT 1000 , 100 OPTION max_matches=1100` ранее не работал
- [Коммит a048](https://github.com/manticoresoftware/manticoresearch/commit/a0483fe9bef3c45dffe96f5df4f4382aee29c072) - HTTPS-запрос к Manticore мог вызвать предупреждение "max packet size(8388608) exceeded"
- [Задача #648](https://github.com/manticoresoftware/manticoresearch/issues/648) - Manticore 3 мог зависать после нескольких обновлений строковых атрибутов

## Версия 3.6.0, 3 мая 2021
**Технический релиз перед Manticore 4**
### Основные новые возможности
- Поддержка [Manticore Columnar Library](https://github.com/manticoresoftware/columnar/) для plain-индексов. Новая настройка [columnar_attrs](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#columnar_attrs) для plain-индексов
- Поддержка [Украинского лемматизатора](https://github.com/manticoresoftware/lemmatizer-uk)
- Полностью переработанные гистограммы. При построении индекса Manticore также строит гистограммы для каждого поля, которые затем используются для ускорения фильтрации. В версии 3.6.0 алгоритм полностью переработан и вы можете получить более высокую производительность при большом объёме данных и интенсивной фильтрации.
### Небольшие изменения
- утилита `manticore_new_cluster [--force]` полезна для перезапуска репликационного кластера через systemd
- [--drop-src](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Merging_tables.md#Merging-tables) для `indexer --merge`

- [новый режим](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) `blend_mode='trim_all'`
- добавлена [поддержка экранирования пути JSON](Searching/Full_text_matching/Escaping.md#Escaping-json-node-names-in-SQL) с помощью обратных кавычек
- [indextool --check](Miscellaneous_tools.md#indextool) может работать в режиме RT

- [FORCE/IGNORE INDEX(id)](Data_creation_and_modification/Updating_documents/UPDATE.md#Updates-via-SQL) для SELECT/UPDATE
- идентификатор чанка для объединённого дискового чанка теперь уникален
- [indextool --check-disk-chunk CHUNK_NAME](Miscellaneous_tools.md#indextool)
### Оптимизации
- [ускоренный парсинг JSON](https://github.com/manticoresoftware/manticoresearch/commit/0c25fc1788e3f77f60cb7fb8f1623d8466bba606), наши тесты показывают на 3-4% меньшую задержку по запросам типа `WHERE json.a = 1`
- недокументированная команда `DEBUG SPLIT` как предпосылка для автоматического шардинга/ребалансировки
### Исправления ошибок
- [Задача #584](https://github.com/manticoresoftware/manticoresearch/issues/524) - неточные и нестабильные результаты FACET
- [Задача #506](https://github.com/manticoresoftware/manticoresearch/issues/506) - странное поведение при использовании MATCH: пострадавшим от этой проблемы нужно перестроить индекс, так как проблема была на этапе построения
- [Задача #387](https://github.com/manticoresoftware/manticoresearch/issues/387) - периодический дамп ядра при выполнении запроса с функцией SNIPPET()
- Оптимизации стека, полезные для обработки сложных запросов:
  - [Задача #469](https://github.com/manticoresoftware/manticoresearch/issues/469) - CRASH DUMP при SELECT результатах
  - [e8420cc7](https://github.com/manticoresoftware/manticoresearch/tree/e8420cc789149c84731be0874be916e33b263bd6) - определение размера стека для фильтров деревьев
- [Issue #461](https://github.com/manticoresoftware/manticoresearch/issues/461) - Обновление с использованием условия IN не применяется корректно
- [Issue #464](https://github.com/manticoresoftware/manticoresearch/issues/464) - SHOW STATUS сразу после CALL PQ возвращает - [Issue #481](https://github.com/manticoresoftware/manticoresearch/issues/481) - Исправлена статическая сборка бинарника
- [Issue #502](https://github.com/manticoresoftware/manticoresearch/issues/502) - ошибка в мульти-запросах
- [Issue #514](https://github.com/manticoresoftware/manticoresearch/issues/514) - Невозможно использовать нестандартные имена колонок при использовании 'create table'
- [Commit d1db](https://github.com/manticoresoftware/manticoresearch/commit/d1dbe771288328c963ca8512f564c6b7cf4f466c) - сбой демона при повторе binlog с обновлением строкового атрибута; установлен binlog версия 10
- [Commit 775d](https://github.com/manticoresoftware/manticoresearch/commit/775d0555562f121911d3c30085947e743fadac2d)  - исправлено определение времени выполнения выражения в стеке (тест 207)
- [Commit 4795](https://github.com/manticoresoftware/manticoresearch/commit/4795dc49194c7745761bb5006ecb266e8a9224fa)  - фильтр индекса percolate и теги были пусты для пустого сохранённого запроса (тест 369)
- [Commit c3f0](https://github.com/manticoresoftware/manticoresearch/commit/c3f0bf4dca0e4eae6ea899dd566878df57eba085)  - прерывание потока репликации SST в сети с большой задержкой и высоким уровнем ошибок (репликация между разными дата-центрами); обновлена версия команды репликации до 1.03
- [Commit ba2d](https://github.com/manticoresoftware/manticoresearch/commit/ba2d6619b8107f3fb177741bbdff3709bac248cd)  - блокировка joiner кластера при операциях записи после подключения к кластеру (тест 385)
- [Commit de4d](https://github.com/manticoresoftware/manticoresearch/commit/de4dcb9ff6c316bc2a9ef6c62eb64769f04ee103)  - сопоставление с подстановочными знаками с модификатором exact (тест 321)
- [Commit 6524](https://github.com/manticoresoftware/manticoresearch/commit/6524fc6af032a0932bd42c37874a619e1fb7f075)  - контрольные точки docid vs docstore
- [Commit f4ab](https://github.com/manticoresoftware/manticoresearch/commit/f4ab83c291e65a5873d271c67b5491f629544f1d)  - Неконсистентное поведение индексатора при разборе некорректного xml
- [Commit 7b72](https://github.com/manticoresoftware/manticoresearch/commit/7b727e22fe01b1a180d4a8d3a3fea92c18eac707)  - Сохранённый percolate запрос с NOTNEAR выполняется бесконечно (тест 349)
- [Commit 812d](https://github.com/manticoresoftware/manticoresearch/commit/812dab745f0c972ecde5d5b3d4a313cd6e1e143e)  - неверный вес для фразы, начинающейся с подстановочного знака
- [Commit 1771](https://github.com/manticoresoftware/manticoresearch/commit/1771afc669c75670b3511bf02ab9bfc8982b94d2)  - percolate запрос с подстановками генерирует термы без полезной нагрузки при совпадении, что вызывает перемежающиеся совпадения и нарушает сопоставление (тест 417)
- [Commit aa0d](https://github.com/manticoresoftware/manticoresearch/commit/aa0d8c2bcb41428d41dbfb66e42a872a79a4ee4b)  - исправлен расчет 'total' в случае параллельного запроса
- [Commit 18d8](https://github.com/manticoresoftware/manticoresearch/commit/18d81b3cfbb930b7361dae645fe13283d4fbf6ef)  - сбой в Windows при множественных параллельных сессиях на демоне
- [Commit 8443](https://github.com/manticoresoftware/manticoresearch/commit/84432f23d6fbf7a7d0b67c8c9412eb5933b03596)  - некоторые настройки индекса не реплицировались
- [Commit 9341](https://github.com/manticoresoftware/manticoresearch/commit/93411fe6af4246ca4acce758944d513a1660b92b)  - При высокой скорости добавления новых событий netloop иногда замерзал из-за атомарного события 'kick', которое обрабатывалось один раз для нескольких событий и терялись их актуальные действия
статус запроса, а не статус сервера
- [Commit d805](https://github.com/manticoresoftware/manticoresearch/commit/d805fc125244ef08d14fb3d1826e2c356862d539)  - Новый сброшенный на диск чанк мог быть потерян при коммите

- [Commit 63cb](https://github.com/manticoresoftware/manticoresearch/commit/63cbf008d50979cdf7062664fb02a672a3bb1131)  - неточная метка 'net_read' в профайлере
- [Commit f537](https://github.com/manticoresoftware/manticoresearch/commit/f5379bb27234b25919966c50ccb6f469ead646e2)  - Проблема Percolate с арабским (текст справа налево)
- [Commit 49ee](https://github.com/manticoresoftware/manticoresearch/commit/49eeb4202d4ce4875c07c35087d7108c1e177328)  - id неправильно выбирался при дублировании имени колонки
- [Commit refa](https://github.com/manticoresoftware/manticoresearch/commit/4b7df262bbfec9b9e95a44e72ad326ac8a786c49) коррекция сетевых событий для устранения сбоя в редких случаях
- [e8420cc7](https://github.com/manticoresoftware/manticoresearch/tree/e8420cc789149c84731be0874be916e33b263bd6) исправление в `indextool --dumpheader`
- [Commit ff71](https://github.com/manticoresoftware/manticoresearch/commit/ff7163532ed39042c0d3c3c2b902ff41f94d7f0c) - TRUNCATE WITH RECONFIGURE неправильно работал с сохранёнными полями

### Критические изменения:
- Новый формат binlog: необходимо корректно остановить Manticore перед обновлением
- Немного изменён формат индекса: новая версия нормально читает ваши существующие индексы, но если вы решите понизить версию с 3.6.0 на старую, новые индексы будут нечитаемыми

- Изменение формата репликации: не реплицируйте с более старой версии на 3.6.0 и наоборот, переключайтесь на новую версию сразу на всех узлах

- `reverse_scan` устарел. Убедитесь, что не используете эту опцию в запросах с 3.6.0, иначе они завершатся ошибкой
- Начиная с этого релиза, мы больше не предоставляем сборки для RHEL6, Debian Jessie и Ubuntu Trusty. Если это критично для вас, [свяжитесь с нами](https://manticoresearch.com/contact-us/)
### Устаревшие функции
- Больше нет неявной сортировки по id. Если вы на неё опирались, обновите свои запросы

- Опция поиска `reverse_scan` устарела
## Версия 3.5.4, 10 декабря 2020
### Новые возможности

- Новые клиенты для Python, Javascript и Java теперь доступны в широком доступе и хорошо документированы в этом руководстве.
- автоматическое удаление дискового чанка индексa реального времени. Эта оптимизация позволяет автоматически удалять дисковый чанк при [OPTIMIZING](Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE) индексе реального времени, когда чанк очевидно больше не нужен (все документы подавлены). Ранее для этого требовалось слияние, теперь чанк можно просто мгновенно удалить. Опция [cutoff](Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks) игнорируется, т.е. даже если ничего фактически не объединяется, устаревший дисковый чанк удаляется. Это полезно, если вы поддерживаете политику хранения в индексе и удаляете старые документы. Теперь уплотнение таких индексов будет происходить быстрее.
- [отдельный NOT](Searching/Options.md#not_terms_only_allowed) как опция для SELECT

### Незначительные изменения
- [Issue #453](https://github.com/manticoresoftware/manticoresearch/issues/453) Новая опция [indexer.ignore_non_plain=1](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#ignore_non_plain) полезна, если вы запускаете `indexer --all` и у вас в конфигурационном файле есть не только plain индексы. Без `ignore_non_plain=1` вы получите предупреждение и соответствующий код выхода.
- [SHOW PLAN ... OPTION format=dot](Node_info_and_management/Profiling/Query_plan.md#Dot-format-for-SHOW-PLAN) и [EXPLAIN QUERY ... OPTION format=dot](Searching/Full_text_matching/Profiling.md#Profiling-without-running-a-query) позволяют визуализировать выполнение плана полнотекстового запроса. Полезно для понимания сложных запросов.
### Устаревания
- `indexer --verbose` устарел, так как он ничего не добавлял к выводу indexer
- Для дампа стека watchdog теперь следует использовать сигнал `USR2` вместо `USR1`
### Исправления ошибок
- [Issue #423](https://github.com/manticoresoftware/manticoresearch/issues/423) Кириллический символ точка в режиме вызова сниппетов не подсвечивается
- [Issue #435](https://github.com/manticoresoftware/manticoresearch/issues/435) RTINDEX - GROUP N BY expression select = фатальный сбой
- [Commit 2b3b](https://github.com/manticoresoftware/manticoresearch/commit/2b3b62bd0ce2b54544733d524a169e81f71b5388) статус searchd показывает Segmentation fault при работе в кластере


- [Commit 9dd2](https://github.com/manticoresoftware/manticoresearch/commit/9dd25c193ed2a53750a0e0e68566fd2b833fcb0c) 'SHOW INDEX index.N SETTINGS' не обрабатывает чанки >9

- [Issue #389](https://github.com/manticoresoftware/manticoresearch/issues/389) Ошибка, вызывающая сбой Manticore

- [Commit fba1](https://github.com/manticoresoftware/manticoresearch/commit/fba166178ff35c73a0c70d131cf3aca4a635e961) Конвертер создает поврежденные индексы
- [Commit eecd](https://github.com/manticoresoftware/manticoresearch/commit/eecd61d861c742471175be19a5e5455034b5db5f) stopword_step=0 против CALL SNIPPETS()
- [Commit ea68](https://github.com/manticoresoftware/manticoresearch/commit/ea6850e449601199190f38a387c04b8a7b7e90f6) count distinct возвращает 0 при низком max_matches в локальном индексе
- [Commit 362f](https://github.com/manticoresoftware/manticoresearch/commit/362f27db3421f09a41c6cce04b5baace00fa4c4d) При использовании агрегации сохранённые тексты не возвращаются в совпадениях
## Версия 3.5.2, 1 октября 2020
### Новые возможности

* OPTIMIZE уменьшает количество дисковых чанков до заданного числа чанков (по умолчанию `2* количество ядер`), а не до одного. Оптимальное количество чанков можно контролировать опцией [cutoff](Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks).

* Оператор NOT теперь может использоваться самостоятельно. По умолчанию он отключен, так как случайные одиночные NOT-запросы могут быть медленными. Его можно включить, задав новый директив searchd [not_terms_only_allowed](Server_settings/Searchd.md#not_terms_only_allowed) в `0`.
* Новая настройка [max_threads_per_query](Server_settings/Searchd.md#max_threads_per_query) устанавливает, сколько потоков может использовать запрос. Если директива не задана, запрос может использовать потоки вплоть до значения [threads](Server_settings/Searchd.md#threads).
Для каждого запроса SELECT число потоков можно ограничить с помощью [OPTION threads=N](Searching/Options.md#threads), переопределяющей глобальный `max_threads_per_query`.
* Перколяторные индексы теперь можно импортировать с помощью [IMPORT TABLE](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Importing_table.md).

* HTTP API `/search` получает базовую поддержку для [фасетирования](Searching/Faceted_search.md#HTTP-JSON)/[группировки](Searching/Grouping.md) через новый узел запроса `aggs`.

### Незначительные изменения

* Если директива replication listen не объявлена, движок будет пытаться использовать порты после заданного порта 'sphinx' до 200.

* `listen=...:sphinx` теперь нужно явно задавать для подключений SphinxSE или клиентов SphinxAPI.

* [SHOW INDEX STATUS](Node_info_and_management/Table_settings_and_status/SHOW_TABLE_STATUS.md) выводит новые метрики: `killed_documents`, `killed_rate`, `disk_mapped_doclists`, `disk_mapped_cached_doclists`, `disk_mapped_hitlists` и `disk_mapped_cached_hitlists`.

* SQL-команда `status` теперь выводит `Queue\Threads` и `Tasks\Threads`.

### Устаревания:
* `dist_threads` полностью устарел, searchd будет выводить предупреждение, если директива ещё используется.
### Docker
Официальный Docker-образ теперь основан на Ubuntu 20.04 LTS
### Упаковка
Помимо обычного пакета `manticore`, Manticore Search можно установить компонентами:

- `manticore-server-core` - предоставляет `searchd`, man-страницу, каталог логов, API и модуль galera. Также установит `manticore-common` как зависимость.

- `manticore-server` - предоставляет скрипты автоматизации для core (init.d, systemd) и обертку `manticore_new_cluster`. Также установит `manticore-server-core` как зависимость.
- `manticore-common` - содержит конфигурацию, стоп-слова, общие документы и скелетные папки (datadir, modules и др.)
- `manticore-tools` - содержит вспомогательные инструменты (`indexer`, `indextool` и т.д.), их man-страницы и примеры. Также установит `manticore-common` как зависимость.
- `manticore-icudata` (RPM) или `manticore-icudata-65l` (DEB) - предоставляет файл данных ICU для использования icu-морфологии.
- `manticore-devel` (RPM) или `manticore-dev` (DEB) - предоставляет заголовочные файлы для UDF.
### Исправления ошибок
1. [Коммит 2a47](https://github.com/manticoresoftware/manticoresearch/commit/2a474dc1a26e8b0f8aaaae95669caf2f1d4b7746) Краш демона в grouper на RT индексе с разными чанками
2. [Коммит 57a1](https://github.com/manticoresoftware/manticoresearch/commit/57a19e5ad5663ef0ca7436595218fb1221d28c8e) Быстрый путь для пустых удалённых документов
3. [Коммит 07dd](https://github.com/manticoresoftware/manticoresearch/commit/07dd3f313c63fb82c22092f9907ef24e3475250e) Выполнение обнаружения кадра стека выражений в рантайме
4. [Коммит 08ae](https://github.com/manticoresoftware/manticoresearch/commit/08ae357cf1012bc8e2da54c20b205b592efda3d4) Соответствие более чем 32 полям в percolate индексах
5. [Коммит 16b9](https://github.com/manticoresoftware/manticoresearch/commit/16b9390fd4cdb07a77ac4497adb935573a1710e5) Диапазон портов прослушивания репликации
6. [Коммит 5fa6](https://github.com/manticoresoftware/manticoresearch/commit/5fa671affeacb6441a59c8a88479bfd423df7c81) Показать create table в pq
7. [Коммит 54d1](https://github.com/manticoresoftware/manticoresearch/commit/54d133b6449105a9fb0168db3f1fbb05fb5aa1f6) Поведение порта HTTPS
8. [Коммит fdbb](https://github.com/manticoresoftware/manticoresearch/commit/fdbbe5245cc296cc5c1ae3ae2fb9cb08fb66a248) Смешивание строк docstore при замене
9. [Коммит afb5](https://github.com/manticoresoftware/manticoresearch/commit/afb53f648ef4e64fa1776c58e66f6e716ac730ad) Переключение уровня сообщения TFO unavailable на 'info'
10. [Коммит 59d9](https://github.com/manticoresoftware/manticoresearch/commit/59d94cefc6e88af360b8046726a16b7eaa9f0b71) Краш при неправильном использовании strcmp
11. [Коммит 04af](https://github.com/manticoresoftware/manticoresearch/commit/04af0349c1e784c2230d204592d6272b440bb375) Добавление индекса в кластер с системными файлами (стоп-слова)
12. [Коммит 5014](https://github.com/manticoresoftware/manticoresearch/commit/50148b4eae9f3a924082005cf5d2f9c8339af7f5) Слияние индексов с большими словарями; RT оптимизация больших дисковых чанков
13. [Коммит a2ad](https://github.com/manticoresoftware/manticoresearch/commit/a2adf1582843f63d4dcc1dab8f7598e0917094a2) Indextool может выгружать метаданные из текущей версии

14. [Коммит 69f6](https://github.com/manticoresoftware/manticoresearch/commit/69f6d5f77c64b97e3f395aea8c555b11c77fb732) Проблема с порядком группировки в GROUP N

15. [Коммит 24d5](https://github.com/manticoresoftware/manticoresearch/commit/24d5d80f508576794b751d70f9f9b524564e3c14) Явная очистка для SphinxSE после рукопожатия
16. [Коммит 31c4](https://github.com/manticoresoftware/manticoresearch/commit/31c4d78ac03ee85d9a02b86075448a04ac80f9e2) Избегать копирования огромных описаний, если это не нужно
17. [Коммит 2959](https://github.com/manticoresoftware/manticoresearch/commit/2959e2caa8610f98fcdf2e5a82160f58687f4a27) Отрицательное время в show threads
18. [Коммит f0b3](https://github.com/manticoresoftware/manticoresearch/commit/f0b357102c70f3ac1d7da260c60b67322940e647) Плагин фильтра токенов и нулевые дельты по позициям
19. [Коммит a49e](https://github.com/manticoresoftware/manticoresearch/commit/a49e5bc13ea763e6f01dd5993e891858a84d1846) Изменить 'FAIL' на 'WARNING' при множественных попаданиях
## Версия 3.5.0, 22 июля 2020
### Основные новые возможности:

* Этот релиз занял так много времени, потому что мы усердно работали над изменением режима многозадачности с потоков на **корутины**. Это упрощает конфигурацию и значительно упрощает параллелизацию запросов: Manticore просто использует заданное количество потоков (см. новую настройку [threads](Server_settings/Searchd.md#threads)), а новый режим гарантирует, что это делается максимально оптимально.

* Изменения в [подсветке](Searching/Highlighting.md#Highlighting-options):

  - любая подсветка, работающая с несколькими полями (`highlight({},'field1, field2'`) или `highlight` в JSON-запросах), теперь по умолчанию применяет лимиты на каждое поле.

  - любая подсветка, работающая с простым текстом (`highlight({}, string_attr)` или `snippet()`), теперь применяет лимиты ко всему документу.
  - [лимиты на поле](Searching/Highlighting.md#limits_per_field) можно переключить в глобальные лимиты параметром `limits_per_field=0` (по умолчанию `1`).
  - [allow_empty](Searching/Highlighting.md#allow_empty) теперь равно `0` по умолчанию для подсветки через HTTP JSON.
* Теперь один и тот же порт [может использоваться](Server_settings/Searchd.md#listen) для http, https и бинарного API (чтобы принимать подключения с удалённого экземпляра Manticore). Для соединений через протокол mysql всё ещё требуется `listen = *:mysql`. Manticore теперь автоматически определяет тип клиента, пытающегося подключиться, за исключением MySQL (из-за ограничений протокола).
* В RT-режиме поле теперь может быть одновременно [текстовым и строковым атрибутом](Creating_a_table/Data_types.md#String) - [GitHub issue #331](https://github.com/manticoresoftware/manticoresearch/issues/331).
  В [plain режиме](Read_this_first.md#Real-time-mode-vs-plain-mode) это называется `sql_field_string`. Теперь оно доступно и в [RT режиме](Read_this_first.md#Real-time-mode-vs-plain-mode) для индексов реального времени. Вы можете использовать его, как показано в примере:
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
* Теперь вы можете [подсвечивать строковые атрибуты](Searching/Highlighting.md#Highlighting-via-SQL).
* Поддержка SSL и сжатия для SQL интерфейса
* Поддержка команды клиента mysql [`status`](Node_info_and_management/Node_status.md#STATUS).
* [Репликация](Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Setting-up-replication) теперь может реплицировать внешние файлы (стоп-слова, исключения и т.д.).
* Оператор фильтра [`in`](Searching/Filters.md#Set-filters) теперь доступен через HTTP JSON интерфейс.
* [`expressions`](Searching/Expressions.md#expressions) в HTTP JSON.
* [Теперь можно изменить `rt_mem_limit` на лету](https://github.com/manticoresoftware/manticoresearch/issues/344) в RT режиме, т.е. можно выполнить `ALTER ... rt_mem_limit=<new value>`.

* Теперь можно использовать [отдельные таблицы кодировок для CJK](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology): `chinese`, `japanese` и `korean`.
* [thread_stack](Server_settings/Searchd.md#thread_stack) теперь ограничивает максимальный размер стека потока, а не начальный.
* Улучшен вывод `SHOW THREADS`.
* Отображение прогресса длительного `CALL PQ` в `SHOW THREADS`.
* cpustat, iostat, coredump можно менять во время работы через [SET](Server_settings/Setting_variables_online.md#SET).

* Реализован `SET [GLOBAL] wait_timeout=NUM`,
### Ломающие изменения:
* **Формат индекса изменён.** Индексы, созданные в версии 3.5.0, не могут быть загружены в Manticore ниже версии 3.5.0, но Manticore 3.5.0 понимает старые форматы.
* [`INSERT INTO PQ VALUES()`](Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md) (т.е. без указания списка столбцов) ранее ожидал ровно `(query, tags)` в значениях. Это изменено на `(id,query,tags,filters)`. Идентификатор можно задать 0, если хотите, чтобы он генерировался автоматически.
* [`allow_empty=0`](Searching/Highlighting.md#allow_empty) — новое значение по умолчанию для подсветки в HTTP JSON интерфейсе.
* В `CREATE TABLE`/`ALTER TABLE` теперь разрешены только абсолютные пути для внешних файлов (стоп-слов, исключений и т.п.).
### Устаревшие возможности:
* `ram_chunks_count` переименован в `ram_chunk_segments_count` в `SHOW INDEX STATUS`.
* `workers` устарел. Сейчас режим только один.
* `dist_threads` устарел. Все запросы максимально параллельны (ограничено `threads` и `jobs_queue_size`).
* `max_children` устарел. Используйте [threads](Server_settings/Searchd.md#threads) для установки количества потоков Manticore (по умолчанию — число ядер CPU).
* `queue_max_length` устарел. В случае необходимости используйте [jobs_queue_size](Server_settings/Searchd.md#jobs_queue_size) для тонкой настройки внутренней очереди заданий (по умолчанию неограничена).
* Все эндпоинты `/json/*` теперь доступны без префикса `/json/`, например `/search`, `/insert`, `/delete`, `/pq` и т.п.
* Значение `field` как "полнотекстовое поле" переименовано в "text" в `describe`.
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
* Кириллическая `и` не маппится в `i` в кодовой таблице `non_cjk` (которая используется по умолчанию), так как это сильно влияло на русский стеммер и лемматизаторы.
* `read_timeout`. Используйте вместо него [network_timeout](Server_settings/Searchd.md#network_timeout), который контролирует и чтение, и запись.
### Пакеты
* Официальный пакет для Ubuntu Focal 20.04
* Название deb пакета изменено с `manticore-bin` на `manticore`
### Исправления багов:
1. [Issue #351](https://github.com/manticoresoftware/manticoresearch/issues/351) утечка памяти searchd
2. [Commit ceab](https://github.com/manticoresoftware/manticoresearch/commit/ceabe44f) Небольшое чтение за границами в сниппетах
3. [Commit 1c3e](https://github.com/manticoresoftware/manticoresearch/commit/1c3e84a3) Опасная запись в локальную переменную для аварийных запросов
4. [Commit 26e0](https://github.com/manticoresoftware/manticoresearch/commit/26e094ab) Небольшая утечка памяти сортировщика в тесте 226
5. [Commit d2c7](https://github.com/manticoresoftware/manticoresearch/commit/d2c7f86a) Большая утечка памяти в тесте 226
6. [Commit 0dd8](https://github.com/manticoresoftware/manticoresearch/commit/0dd80122) Кластер показывает, что узлы синхронизированы, но `count(*)` показывает разные числа
7. [Commit f1c1](https://github.com/manticoresoftware/manticoresearch/commit/f1c1ac3f) Косметический: Дублирование и иногда потеря предупреждений в логе
8. [Commit f1c1](https://github.com/manticoresoftware/manticoresearch/commit/f1c1ac3f) Косметический: (null) имя индекса в логе
9. [Commit 359d](https://github.com/manticoresoftware/manticoresearch/commit/359dbd30) Невозможно получить более 70M результатов
10. [Commit 19f3](https://github.com/manticoresoftware/manticoresearch/commit/19f328ee) Невозможно добавить правила PQ без указания столбцов
11. [Commit bf68](https://github.com/manticoresoftware/manticoresearch/commit/bf685d5d) Ввод документа в индекс кластера вызывает вводящее в заблуждение сообщение об ошибке
12. [Commit 2cf1](https://github.com/manticoresoftware/manticoresearch/commit/2cf18c83) `/json/replace` и `json/update` возвращают id в экспоненциальной форме
13. [Issue #324](https://github.com/manticoresoftware/manticoresearch/issues/324) Обновление скалярных json свойств и mva в одном запросе
14. [Commit d384](https://github.com/manticoresoftware/manticoresearch/commit/d38409eb) `hitless_words` не работает в RT режиме
15. [Commit 5813](https://github.com/manticoresoftware/manticoresearch/commit/5813d639) `ALTER RECONFIGURE` в rt режиме должен быть запрещён
16. [Commit 5813](https://github.com/manticoresoftware/manticoresearch/commit/5813d639) `rt_mem_limit` сбрасывается в 128M после перезапуска searchd
17. highlight() иногда зависает
18. [Commit 7cd8](https://github.com/manticoresoftware/manticoresearch/commit/7cd878f4) Не удалось использовать U+код в RT режиме
19. [Commit 2b21](https://github.com/manticoresoftware/manticoresearch/commit/2b213de4) Не удалось использовать подстановочный знак в wordforms в RT режиме
20. [Commit e9d0](https://github.com/manticoresoftware/manticoresearch/commit/e9d07e68) Исправлен `SHOW CREATE TABLE` при нескольких файлах wordform
21. [Коммит fc90](https://github.com/manticoresoftware/manticoresearch/commit/fc90a84f) JSON-запрос без "query" приводит к сбою searchd
22. Официальный [docker Manticore](https://hub.docker.com/r/manticoresearch/manticore) не может индексировать из mysql 8
23. [Коммит 23e0](https://github.com/manticoresoftware/manticoresearch/commit/23e05d32) HTTP /json/insert требует id
24. [Коммит bd67](https://github.com/manticoresoftware/manticoresearch/commit/bd679af0) `SHOW CREATE TABLE` не работает для PQ
25. [Коммит bd67](https://github.com/manticoresoftware/manticoresearch/commit/bd679af0) `CREATE TABLE LIKE` работает некорректно для PQ
26. [Коммит 5eac](https://github.com/manticoresoftware/manticoresearch/commit/5eacf28f) Конец строки в настройках в show index status
27. [Коммит cb15](https://github.com/manticoresoftware/manticoresearch/commit/cb153228) Пустой заголовок в "highlight" в HTTP JSON-ответе
28. [Задача #318](https://github.com/manticoresoftware/manticoresearch/issues/318) Ошибка infix в `CREATE TABLE LIKE`
29. [Коммит 9040](https://github.com/manticoresoftware/manticoresearch/commit/9040d22c) Сбой RT под нагрузкой
30. [cd512c7d ](https://github.com/manticoresoftware/manticoresearch/commit/cd512c7d ) Потерян лог сбоя при сбое RT disk chunk
31. [Задача #323](https://github.com/manticoresoftware/manticoresearch/issues/323) Импорт таблицы завершается ошибкой и закрывает соединение
32. [Коммит 6275](https://github.com/manticoresoftware/manticoresearch/commit/6275316a) `ALTER reconfigure` повреждает индекс PQ
33. [Коммит 9c1d](https://github.com/manticoresoftware/manticoresearch/commit/9c1d221e) Проблемы с перезагрузкой searchd после изменения типа индекса
34. [Коммит 71e2](https://github.com/manticoresoftware/manticoresearch/commit/71e2b5bb) Сбой демона при импорте таблицы с отсутствующими файлами
35. [Задача #322](https://github.com/manticoresoftware/manticoresearch/issues/322) Сбой при select с использованием нескольких индексов, group by и ранжировщика = none
36. [Коммит c3f5](https://github.com/manticoresoftware/manticoresearch/commit/c3f58490) `HIGHLIGHT()` не выделяет в строковых атрибутах
37. [Задача #320](https://github.com/manticoresoftware/manticoresearch/issues/320) `FACET` не сортирует по строковому атрибуту
38. [Коммит 4f1a](https://github.com/manticoresoftware/manticoresearch/commit/4f1a1f25) Ошибка при отсутствии каталога данных
39. [Коммит 04f4](https://github.com/manticoresoftware/manticoresearch/commit/04f4ddd4) access_* не поддерживаются в RT-режиме
40. [Коммит 1c06](https://github.com/manticoresoftware/manticoresearch/commit/1c0616a2) Плохие JSON-объекты в строках: 1. `CALL PQ` возвращает "Bad JSON objects in strings: 1" когда json превышает некоторый размер.
41. [Коммит 32f9](https://github.com/manticoresoftware/manticoresearch/commit/32f943d6) Несогласованность RT-режима. В некоторых случаях нельзя удалить индекс, так как он неизвестен, и нельзя создать, так как директория не пуста.
42. [Задача #319](https://github.com/manticoresoftware/manticoresearch/issues/319) Сбой при select
43. [Коммит 22a2](https://github.com/manticoresoftware/manticoresearch/commit/22a28dd7) `max_xmlpipe2_field` = 2M возвращал предупреждение на поле 2M
44. [Задача #342](https://github.com/manticoresoftware/manticoresearch/issues/342) Ошибка выполнения условий запроса
45. [Коммит dd8d](https://github.com/manticoresoftware/manticoresearch/commit/dd8dcab2) Простой поиск по 2 терминам находит документ, содержащий только один термин
46. [Коммит 9091](https://github.com/manticoresoftware/manticoresearch/commit/90919e62) В PQ было невозможно сопоставить json с заглавными буквами в ключах
47. [Коммит 56da](https://github.com/manticoresoftware/manticoresearch/commit/56da086a) Индексатор падает при csv+docstore
48. [Задача #363](https://github.com/manticoresoftware/manticoresearch/issues/363) использование `[null]` в json атрибуте в centos 7 вызывает поврежденные вставляемые данные
49. Крупная [Задача #345](https://github.com/manticoresoftware/manticoresearch/issues/345) Записи не вставляются, count() случайный, "replace into" возвращает OK
50. max_query_time слишком сильно замедляет SELECT-запросы


51. [Задача #352](https://github.com/manticoresoftware/manticoresearch/issues/352) Сбой связи master-agent на Mac OS
52. [Задача #328](https://github.com/manticoresoftware/manticoresearch/issues/328) Ошибка при подключении к Manticore с Connector.Net/Mysql 8.0.19
53. [Коммит daa7](https://github.com/manticoresoftware/manticoresearch/commit/daa760d2fe83668038318f0f384ceefc0d075e18) Исправлено экранирование \0 и оптимизирована производительность

54. [Коммит 9bc5](https://github.com/manticoresoftware/manticoresearch/commit/9bc5c01a493db993b90ff6c1ad51aeae028f629c) Исправлен count distinct vs json
55. [Коммит 4f89](https://github.com/manticoresoftware/manticoresearch/commit/4f89a965a3af0db548ae9d1763731ebaebe21f5a) Исправлена ошибка при сбое drop table на другом узле
56. [Коммит 952a](https://github.com/manticoresoftware/manticoresearch/commit/952af5a5931fd72b87edc169588777778b365231) Исправлены сбои при интенсивном вызове call pq
## Версия 3.4.2, 10 апреля 2020
### Критические исправления ошибок
* [Коммит 2ffe](https://github.com/manticoresoftware/manticoresearch/commit/2ffe2d261793f9681265bc4abe38d695e72c121e) исправлена ошибка: RT индекс из старой версии не индексирует данные

## Версия 3.4.0, 26 марта 2020
### Основные изменения
* сервер работает в 2 режимах: rt-mode и plain-mode
   *   rt-mode требует data_dir и не допускает определение индексов в конфиге
   *   в plain-mode индексы определяются в конфиге; data_dir не разрешён
* репликация доступна только в rt-mode

### Незначительные изменения
* charset_table по умолчанию alias non_cjk
* в rt-mode полнотекстовые поля индексируются и сохраняются по умолчанию

* полнотекстовые поля в rt-mode переименованы с 'field' в 'text'
* ALTER RTINDEX переименован в ALTER TABLE
* TRUNCATE RTINDEX переименован в TRUNCATE TABLE
### Возможности
* поля только для хранения

* SHOW CREATE TABLE, IMPORT TABLE
### Улучшения
* значительно быстрее lockless PQ
* /sql может выполнять любые типы SQL-запросов в режиме mode=raw
* алиас mysql для протокола mysql41
* состояние state.sql по умолчанию в data_dir
### Исправления ошибок
* [Коммит a533](https://github.com/manticoresoftware/manticoresearch/commit/a5333644e8e0c69746a9f24da8221fc965478e4a) исправлена ошибка сбоя при неправильном синтаксисе поля в highlight()
* [Коммит 7fbb](https://github.com/manticoresoftware/manticoresearch/commit/7fbb9f2e618096289cba7f8b834530b46aa94055) исправлен сбой сервера при репликации RT индекса с docstore
* [Коммит 24a0](https://github.com/manticoresoftware/manticoresearch/commit/24a04687eb43b4f4719c7c4a5fe3c19099240467) исправлен сбой при highlight к индексу с опцией infix или prefix и к индексу без включенных сохранённых полей
* [Коммит 3465](https://github.com/manticoresoftware/manticoresearch/commit/3465c1ce9c860c09f6ccc5b83a63e3c68c241d8d) исправлена ложная ошибка о пустом docstore и док-айди при поиске для пустого индекса
* [Коммит a707](https://github.com/manticoresoftware/manticoresearch/commit/b8601b4144b9092be43a7caa7cd6b19cb37e14c7) исправлена #314 команда SQL insert с завершающей точкой с запятой
* [Коммит 9562](https://github.com/manticoresoftware/manticoresearch/commit/95628c9b9b8f16fe1bd18599a84e9673575adf1a) удалено предупреждение о несоответствии слов запроса

* [Коммит b860](https://github.com/manticoresoftware/manticoresearch/commit/b8601b4144b9092be43a7caa7cd6b19cb37e14c7) исправлены запросы в сниппетах сегментированных через ICU
* [Коммит 5275](https://github.com/manticoresoftware/manticoresearch/commit/5275516ca4631c584e6e0127fa161a18d2658fd0) исправлено состояние гонки при поиске/добавлении кеша блока docstore
* [Коммит f06e](https://github.com/manticoresoftware/manticoresearch/commit/f06ef97af5c7390427e82525c150535290e42c3e) исправлена утечка памяти в docstore
* [Коммит a725](https://github.com/manticoresoftware/manticoresearch/commit/a7258ba8340e246c0d8aaeb4a7a74129eba807eb) исправлена #316 LAST_INSERT_ID возвращает пустое значение при INSERT
* [Коммит 1ebd](https://github.com/manticoresoftware/manticoresearch/commit/1ebd5bf8d74113bcfa138ce63ce11966990a4215) исправлена #317 точка http обновления json/update с поддержкой массива для MVA и объекта для JSON атрибута
* [Коммит e426](https://github.com/manticoresoftware/manticoresearch/commit/e426950a536382a142e2a42ed207c341149ff685) исправлен сбой индексации rt без явного id
## Версия 3.3.0, 4 февраля 2020

### Функции
* Параллельный поиск в индексах Real-Time
* Команда EXPLAIN QUERY
* конфигурационный файл без определений индексов (альфа-версия)
* команды CREATE/DROP TABLE (альфа-версия)
* indexer --print-rt — может читать из источника и выводить INSERT для Real-Time индекса
### Улучшения


* Обновлены стеммеры Snowball до версии 2.0
* Фильтр LIKE для SHOW INDEX STATUS
* улучшено использование памяти при высоком max_matches
* SHOW INDEX STATUS добавляет ram_chunks_count для RT индексов
* безблоковая очередь приоритета (lockless PQ)
* изменён LimitNOFILE до 65536
### Исправления ошибок
* [Коммит 9c33](https://github.com/manticoresoftware/manticoresearch/commit/9c33aab8c1fd9059c8644d23d5f58f03010043c6) добавлена проверка схемы индекса на дублирующиеся атрибуты #293
* [Коммит a008](https://github.com/manticoresoftware/manticoresearch/commit/a0085f9477e191da9e0e835b91583fb9cfde698e) исправлен сбой при hitless terms
* [Коммит 6895](https://github.com/manticoresoftware/manticoresearch/commit/68953740af8dca1833026d0f448c71416ed3044a) исправлен повреждённый docstore после ATTACH
* [Коммит d6f6](https://github.com/manticoresoftware/manticoresearch/commit/d6f696edea53ea2b9a68251e4054e4816082c285) исправлена проблема docstore в распределённой настройке
* [Коммит bce2](https://github.com/manticoresoftware/manticoresearch/commit/bce2b7ec883d208f31094ee42a5b6f01d18a998f) заменён FixedHash на OpenHash в сортировщике
* [Коммит e0ba](https://github.com/manticoresoftware/manticoresearch/commit/e0baf7392937969e640fb5d10e8ccb312fe36446) исправлены атрибуты с дублирующимися именами при определении индекса
* [Коммит ca81](https://github.com/manticoresoftware/manticoresearch/commit/ca81114bf1ac7ea7adcc8509bdc14d1f30b4f461) исправлен html_strip в HIGHLIGHT()
* [Коммит 493a](https://github.com/manticoresoftware/manticoresearch/commit/493a5e916d70fd4a3f14d1ee487234434f4a7ce3) исправлен макрос passage в HIGHLIGHT()
* [Коммит a82d](https://github.com/manticoresoftware/manticoresearch/commit/a82d41c775692f837f267d554d7d4a4b499d69f0) исправлены проблемы двойного буфера при создании RT индексом маленьких или больших дисковых чанков
* [Коммит a404](https://github.com/manticoresoftware/manticoresearch/commit/a404c85de0d41d9d1c75b7540479d8a3af9e61aa) исправлено удаление события для kqueue

* [Коммит 8bea](https://github.com/manticoresoftware/manticoresearch/commit/8bea0f6fd1666b817995c4dd6e67f6f1c9161638) исправлено сохранение дискового чанка при большом значении rt_mem_limit RT индекса
* [Коммит 8707](https://github.com/manticoresoftware/manticoresearch/commit/8707f0395b7ace52dc0cba9c8ed92a2bc4392932) исправлено переполнение float при индексировании
* [Коммит a564](https://github.com/manticoresoftware/manticoresearch/commit/a56434ce71bb7714b4d8e75278aaa075b27ece80) исправлена ошибка, когда вставка документа с отрицательным ID в RT индекс теперь завершается ошибкой
* [Коммит bbeb](https://github.com/manticoresoftware/manticoresearch/commit/bbebfd7551a68641154761cac962053b54806759) исправлен сбой сервера при использовании ranker fieldmask
* [Коммит 3809](https://github.com/manticoresoftware/manticoresearch/commit/3809cc1b0863e3a5a200dc423be3f7c38be56113) исправлен сбой при использовании кеша запросов
* [Коммит dc2a](https://github.com/manticoresoftware/manticoresearch/commit/dc2a585bc724678679e4297362200bbd49ce32bb) исправлен сбой при использовании RAM сегментов RT индекса с параллельными вставками

## Версия 3.2.2, 19 декабря 2019
### Функции
* Автоинкремент ID для RT индексов
* Поддержка подсветки для docstore через новую функцию HIGHLIGHT(), доступную также в HTTP API
* SNIPPET() теперь может использовать специальную функцию QUERY(), которая возвращает текущий запрос MATCH
* новый параметр field_separator для функций подсветки.
### Улучшения и изменения
* ленивое извлечение сохранённых полей для удалённых узлов (может значительно повысить производительность)
* строки и выражения больше не ломают оптимизации multi-query и FACET
* сборка для RHEL/CentOS 8 теперь использует mysql libclient из mariadb-connector-c-devel

* ICU data файл теперь поставляется с пакетами, параметр icu_data_dir удалён
* systemd файлы служб теперь включают политику 'Restart=on-failure'
* indextool теперь может проверять real-time индексы онлайн
* конфигурационный файл по умолчанию теперь /etc/manticoresearch/manticore.conf
* сервис в RHEL/CentOS переименован в 'manticore' вместо 'searchd'
* удалены опции query_mode и exact_phrase для сниппетов
### Исправления ошибок
* [Коммит 6ae4](https://github.com/manticoresoftware/manticoresearch/commit/6ae474c7894a6bee222d5b18e59a44fdbf57843a) исправлен сбой при SELECT-запросе через HTTP-интерфейс
* [Коммит 5957](https://github.com/manticoresoftware/manticoresearch/commit/59577513a49eac5a4a3c5e2cb38394d3246b5d35) исправлена проблема с сохранением RT индекса в дисковые чанки, не отмечались некоторые документы как удалённые
* [Коммит e861](https://github.com/manticoresoftware/manticoresearch/commit/e861f0fca0e88924450695d4e9d6acff7a36558a) исправлен сбой при поиске по нескольким индексам или многозапросам с dist_threads
* [Коммит 4409](https://github.com/manticoresoftware/manticoresearch/commit/440991fc977b8479800b45cdbc862eeb1ba5d965) исправлен сбой при генерации инфиксов для длинных терминов с широкими utf8 кодовыми точками
* [Коммит 5fd5](https://github.com/manticoresoftware/manticoresearch/commit/5fd599b48bba527c023e6aa0b262dca51ffb8a1c) исправлена гонка при добавлении сокета в IOCP
* [Коммит cf10](https://github.com/manticoresoftware/manticoresearch/commit/cf10d7d3589f77adfaefbe52a51777bc9c67cf99) исправлена проблема булевых запросов с JSON-списком выбора
* [Коммит 996d](https://github.com/manticoresoftware/manticoresearch/commit/996de77f4c3ec103d965e28c36098fd07dba50c4) исправлена проверка indextool для отчёта о неправильном смещении skiplist, проверка doc2row lookup
* [Коммит 6e3f](https://github.com/manticoresoftware/manticoresearch/commit/6e3fc9e88941c9427410f9d0adeebb7dd09a900f) исправлена ситуация, когда indexer создавал плохой индекс с отрицательным смещением skiplist на больших данных
* [Коммит faed](https://github.com/manticoresoftware/manticoresearch/commit/faed3220b423a96401982bf47117edf1f62c584e) исправлено преобразование JSON: числовой в строковый и строковый в числовой в выражениях
* [Коммит 5331](https://github.com/manticoresoftware/manticoresearch/commit/533197200249d648ebcbdaca5d605861ee566348) исправлен выход indextool с ошибкой при указании нескольких команд в командной строке
* [Коммит 7955](https://github.com/manticoresoftware/manticoresearch/commit/795520ac351d22e4497ebaf6c249bf25081842b6) исправлена #275 невалидное состояние binlog при ошибке "на диске нет места"

* [Коммит 2284](https://github.com/manticoresoftware/manticoresearch/commit/2284da5e3ff77bf7be4038a4d950c0580e4159c8) исправлена #279 ошибка при фильтре IN в JSON-атрибуте
* [Коммит ce2e](https://github.com/manticoresoftware/manticoresearch/commit/ce2e4b4765111a7d331ffc256911f12770a8942d) исправлена #281 неправильная команда закрытия pipe
* [Коммит 5355](https://github.com/manticoresoftware/manticoresearch/commit/535589ba8a37baf5c6056afb5ff969fd70476feb) исправлено зависание сервера при вызове CALL PQ с рекурсивным JSON-атрибутом, закодированным как строка
* [Коммит a5fc](https://github.com/manticoresoftware/manticoresearch/commit/a5fc8a36e1644ee051632408ab8b53e79aeb9ff4) исправлен выход за пределы списка документов в multiand node

* [Коммит a362](https://github.com/manticoresoftware/manticoresearch/commit/a3628617ef4ee89a811d4ba2c4a04cbbc298c1b5) исправлен сбор публичной информации о потоках
* [Коммит f8d2](https://github.com/manticoresoftware/manticoresearch/commit/f8d2d7bbbb90af3d2ee97424f97b05927f09d42b) исправлены блокировки кэша docstore
## Версия 3.2.0, 17 октября 2019
### Особенности
* Хранение документов

* новые директивы stored_fields, docstore_cache_size, docstore_block_size, docstore_compression, docstore_compression_level
### Улучшения и изменения
* улучшена поддержка SSL
* обновлена встроенная кодировка non_cjk
* отключено логирование UPDATE/DELETE запросов в журнал запросов SELECT
* пакеты для RHEL/CentOS 8
### Исправления ошибок
* [Коммит 301a](https://github.com/manticoresoftware/manticoresearch/commit/301a806b14725e842300ee225521407464374c41) исправлен сбой при замене документа в дисковом чанке RT индекса
* [Коммит 46c1](https://github.com/manticoresoftware/manticoresearch/commit/46c1cad8fa11c1fc09cc34b20d978fdfcd4e6d27) исправлен #269 LIMIT N OFFSET M
* [Коммит 92a4](https://github.com/manticoresoftware/manticoresearch/commit/92a46edaa196ef33f3ac2225fbdba2c2b88ee45d) исправлены DELETE запросы с явно заданным id или списком id, пропускающие поиск

* [Коммит 8ca7](https://github.com/manticoresoftware/manticoresearch/commit/8ca78c138d837caab30dc6e2343a3c4d1687bb87) исправлен неправильный индекс после удаления события в netloop при poller windowspoll
* [Коммит 6036](https://github.com/manticoresoftware/manticoresearch/commit/603631e2bff5cb0acf25f68b85621742490eccd9) исправлено округление float в JSON через HTTP
* [Коммит 62f6](https://github.com/manticoresoftware/manticoresearch/commit/62f64cb9e75ec28fc3c29f8414ce9656f5580254) исправлены удалённые сниппеты для проверки пустого пути в первую очередь; исправлены тесты на Windows
* [Коммит aba2](https://github.com/manticoresoftware/manticoresearch/commit/aba274c2c5abc1029894ba9f95b786bda93e8f22) исправлена перезагрузка конфига для работы на Windows так же, как на Linux
* [Коммит 6b8c](https://github.com/manticoresoftware/manticoresearch/commit/6b8c4242ef77c3fa4d0ccb7d76d81714b6728f0b) исправлен #194 PQ для работы с морфологией и стеммерами
* [Коммит 174d](https://github.com/manticoresoftware/manticoresearch/commit/174d312905285aa5746f4f404c53e6dc669ef42f) исправлено управление устаревшими сегментами RT
## Версия 3.1.2, 22 августа 2019

### Особенности и улучшения
* Экспериментальная поддержка SSL для HTTP API
* фильтр полей для CALL KEYWORDS
* max_matches для /json/search
* автоматическое задание размера по умолчанию для Galera gcache.size
* улучшена поддержка FreeBSD
### Исправления ошибок
* [Коммит 0a1a](https://github.com/manticoresoftware/manticoresearch/commit/0a1a2c81038794983b4e30c8aefc7cc20e89b03f) исправлена репликация RT индекса в узел, где существует такой же RT индекс с другим путем

* [Коммит 4adc](https://github.com/manticoresoftware/manticoresearch/commit/4adc075294ac823289f745e2cc419f18c7dcf2e2) исправлен перенос flush для индексов без активности
* [Коммит d6c0](https://github.com/manticoresoftware/manticoresearch/commit/d6c00a6f750c04021747e9c99e0f77707a4cbfc3) улучшено переназначение флешинга RT/PQ индексов
* [Коммит d0a7](https://github.com/manticoresoftware/manticoresearch/commit/d0a7c95906b9b5ba33c98267d54501cfe27508d6) исправлен #250 параметр index_field_lengths для источников TSV и CSV через pipe
* [Commit 1266](https://github.com/manticoresoftware/manticoresearch/commit/1266d54800242709fbe2c34e72eb69b6595b23bd) исправлена ошибка indextool с некорректным отчетом о проверке блочного индекса на пустом индексе
* [Commit 553c](https://github.com/manticoresoftware/manticoresearch/commit/553ca73ca111f28e0888d59cd48b0d6df181ecd6) исправлен пустой список выбора в журнале запросов Manticore SQL
* [Commit 56c8](https://github.com/manticoresoftware/manticoresearch/commit/56c8584456293665ddd4ce4c94307d74473a78b9) исправлен ответ indexer -h/--help
## Версия 3.1.0, 16 июля 2019
### Новые возможности и улучшения
* репликация для RealTime индексов
* ICU токенизатор для китайского
* новая опция морфологии icu_chinese
* новая директива icu_data_dir
* транзакции с несколькими операторами для репликации
* LAST_INSERT_ID() и @session.last_insert_id

* LIKE 'pattern' для SHOW VARIABLES
* Множественные вставки документов для percolate индексов
* Добавлены парсеры времени для конфигурации

* внутренний менеджер задач
* mlock для компонентов списков документов и результатов
* путь для jail сниппетов
### Удаления
* поддержка библиотеки RLP отменена в пользу ICU; все директивы rlp* удалены
* обновление ID документа с помощью UPDATE отключено
### Исправления ошибок
* [Commit f047](https://github.com/manticoresoftware/manticoresearch/commit/f047222339fd0e62a07e9ca8b2a829297d5db861) исправлены дефекты в concat и group_concat
* [Commit b081](https://github.com/manticoresoftware/manticoresearch/commit/b08147ee24712b5eaa51f261626b5d16c6242fdd) исправлен тип атрибута uid запроса в percolate индексе на BIGINT
* [Commit 4cd8](https://github.com/manticoresoftware/manticoresearch/commit/4cd85afaf76eef0ce9fa1cd073f3fed160279890) предотвращен сбой при неудаче предварительного выделения нового дискового чанка
* [Commit 1a55](https://github.com/manticoresoftware/manticoresearch/commit/1a551227886474054111088412f2b2396f485652) добавлен отсутствующий тип данных timestamp в ALTER

* [Commit f3a8](https://github.com/manticoresoftware/manticoresearch/commit/f3a8e096fd496a6b001d398aadbb781b2dbad929) исправлен сбой из-за неправильного mmap чтения
* [Commit 4475](https://github.com/manticoresoftware/manticoresearch/commit/447577115192db7ac7bfea70b5ba58ee2c61229f) исправлена блокировка хэша кластеров в репликации
* [Commit ff47](https://github.com/manticoresoftware/manticoresearch/commit/ff476df955f057bd38e8b6ad6056325e9d6fbb9c) исправлена утечка провайдеров в репликации
* [Commit 58dc](https://github.com/manticoresoftware/manticoresearch/commit/58dcbb779c4c30ca1560460c78e35ba114b83716) исправлена ошибка #246 неопреленный sigmask в indexer
* [Commit 3dd8](https://github.com/manticoresoftware/manticoresearch/commit/3dd8278e747d534f69fe9c3d360eef188ab7b6d5) исправлена гонка условий в отчёте netloop
* [Commit a02a](https://github.com/manticoresoftware/manticoresearch/commit/a02aae05a3fa3d1caf2821f43c7e237b6de76336) нулевой промежуток для перетасовщика стратегий HA

## Версия 3.0.2, 31 мая 2019
### Улучшения

* добавлены mmap читатели для списков документов и результатов
* ответ HTTP эндпоинта `/sql` теперь совпадает с ответом `/json/search`

* новые директивы `access_plain_attrs`, `access_blob_attrs`, `access_doclists`, `access_hitlists`
* новая директива `server_id` для настроек репликации
### Удаления
* удалён HTTP эндпоинт `/search`
### Устаревшие возможности
* `read_buffer`, `ondisk_attrs`, `ondisk_attrs_default`, `mlock` заменены директивами `access_*`
### Исправления ошибок
* [Commit 849c](https://github.com/manticoresoftware/manticoresearch/commit/849c16e1040f382795ba0635cb48686f9db4f2d8) разрешены имена атрибутов начинающиеся с цифр в списке выбора
* [Commit 48e6](https://github.com/manticoresoftware/manticoresearch/commit/48e6c302ca37bec0bc49c79619c60bf787ed0708) исправлены MVAs в UDF и исправлено присваивание псевдонимов для MVA
* [Commit 0555](https://github.com/manticoresoftware/manticoresearch/commit/055586a9a2ebfa8874f68099c5bc798a145fd4ef) исправлена ошибка #187 сбоя при использовании запроса с SENTENCE


* [Commit 93bf](https://github.com/manticoresoftware/manticoresearch/commit/93bf52f23f1c53c8a93a603387ca5463e1fc2ae8) исправлена ошибка #143 поддержка () вокруг MATCH()
* [Commit 599e](https://github.com/manticoresoftware/manticoresearch/commit/599ee79ccd38cce0023a55cbce8f50c3dca62e38) исправлен сохранение состояния кластера при ALTER cluster
* [Commit 230c](https://github.com/manticoresoftware/manticoresearch/commit/230c321e277514b191629538eb3f02d219113d95) исправлен сбой сервера при ALTER index с blob атрибутами
* [Commit 5802](https://github.com/manticoresoftware/manticoresearch/commit/5802b85aa93a81caea77073a3381ef912fdd677c) исправлена ошибка #196 фильтрации по id
* [Commit 25d2](https://github.com/manticoresoftware/manticoresearch/commit/25d2dabda4a299a29bd375405862126da9b4bfac) отменён поиск по шаблонным индексам
* [Commit 2a30](https://github.com/manticoresoftware/manticoresearch/commit/2a30d5b49d76faab031d195f533974938769b8e6) исправлен столбец id для типа bigint в SQL ответе
## Версия 3.0.0, 6 мая 2019
### Новые возможности и улучшения
* Новое хранение индекса. Нечисловые атрибуты больше не ограничены размером 4GB на индекс
* директива attr_update_reserve
* строки, JSON и MVAs могут обновляться с помощью UPDATE
* killlists применяются при загрузке индекса
* директива killlist_target
* ускорение много AND-поисков
* улучшение средней производительности и использования RAM

* инструмент конвертации для обновления индексов, созданных в 2.x
* функция CONCAT()
* JOIN CLUSTER cluster AT 'nodeaddress:port'
* ALTER CLUSTER posts UPDATE nodes

* директива node_address
* список узлов выводится в SHOW STATUS

### Изменения в поведении
* при наличии killist в индексах сервер не меняет порядок индексов согласно конфигурации, а следует цепочке таргетов killlist
* порядок индексов в поиске больше не определяет порядок применения killlist
* Document ID теперь является знаковым bigint

### Удалённые директивы
* docinfo (теперь всегда extern), inplace_docinfo_gap, mva_updates_pool
## Версия 2.8.2 GA, 2 апреля 2019

### Новые возможности и улучшения
* репликация Galera для percolate индексов
* опция OPTION morphology
### Заметки по сборке
Минимальная версия Cmake теперь 3.13. Для сборки необходимы boost и libssl
разработческие библиотеки.
### Исправления ошибок
* [Commit 6967](https://github.com/manticoresoftware/manticoresearch/commit/6967fedb2ef818ec1c825d482563edd05e1c9245) исправлена ошибка сбоя при множественных звездочках в списке select для запроса в многие распределённые индексы
* [Commit 36df](https://github.com/manticoresoftware/manticoresearch/commit/36df1a407dc08263690e3492518613ace82d69ca) исправлена [\#177](https://github.com/manticoresoftware/manticoresearch/issues/177) большая пакетная передача через интерфейс Manticore SQL
* [Commit 5793](https://github.com/manticoresoftware/manticoresearch/commit/57932aec734583fa93359faaf5034b2e2c9d352b) исправлена [\#170](https://github.com/manticoresoftware/manticoresearch/issues/170) авария сервера при оптимизации RT с обновлённым MVA

* [Commit edb2](https://github.com/manticoresoftware/manticoresearch/commit/edb24b870423add86eba471d361e0e5aff098b18) исправлен сбой сервера при удалении binlog из-за удаления RT индекса после перезагрузки конфига по SIGHUP
* [Commit bd3e](https://github.com/manticoresoftware/manticoresearch/commit/bd3e66e0085bc7e2e351b817dfc972fd8158fbce) исправлены полезные нагрузки аутентификационного плагина handshake MySQL
* [Commit 6a21](https://github.com/manticoresoftware/manticoresearch/commit/6a217f6ed82fb10f752213b15617b9cedf1e8533) исправлена [\#172](https://github.com/manticoresoftware/manticoresearch/issues/172) установка phrase_boundary в RT индексе
* [Commit 3562](https://github.com/manticoresoftware/manticoresearch/commit/3562f652753e4091fd7b5b0f65b58341c8cbcb31) исправлена [\#168](https://github.com/manticoresoftware/manticoresearch/issues/168) взаимная блокировка при ATTACH индекса самого к себе
* [Commit 250b](https://github.com/manticoresoftware/manticoresearch/commit/250b3f0e74ad18dda34ba080aa13ca87798a9ac9) исправлено сохранение пустой метаинформации binlog после сбоя сервера

* [Commit 4aa6](https://github.com/manticoresoftware/manticoresearch/commit/4aa6c69ad3a181cd7c5dafb990fb528121f68e59) исправлен сбой сервера из-за строки в сортировщике из RT индекса с дисковыми чанками
## Версия 2.8.1 GA, 6 марта 2019
### Новые возможности и улучшения
* SUBSTRING_INDEX()
* Поддержка SENTENCE и PARAGRAPH для percolate-запросов
* systemd-генератор для Debian/Ubuntu; добавлен LimitCORE для разрешения дампов ядра
### Исправления ошибок
* [Commit 84fe](https://github.com/manticoresoftware/manticoresearch/commit/84fe7405d17b59f4dcde15a6c8e8503923503def) исправлен сбой сервера в режиме match all и пустом полнотекстовом запросе
* [Commit daa8](https://github.com/manticoresoftware/manticoresearch/commit/daa88b579fa373d5d2e869e43a2e178363b0fef1) исправлен сбой при удалении статической строки
* [Commit 2207](https://github.com/manticoresoftware/manticoresearch/commit/22078537dddbd9ce81a182dcc879f235b6e87004) исправлен код возврата при ошибке FATAL в indextool
* [Commit 0721](https://github.com/manticoresoftware/manticoresearch/commit/0721696d6780c200c65d596624a8187983fb7fcb) исправлена [\#109](https://github.com/manticoresoftware/manticoresearch/issues/109) ошибка отсутствия совпадений для префиксов из-за неправильной проверки точной формы
* [Commit 8af8](https://github.com/manticoresoftware/manticoresearch/commit/8af810111b8e2f87bc9f378172eff1ab9725c7e7) исправлена [\#161](https://github.com/manticoresoftware/manticoresearch/issues/161) перезагрузка настроек конфигурации для RT индексов
* [Commit e2d5](https://github.com/manticoresoftware/manticoresearch/commit/e2d592773f4fd702ace90070a793a842a0fec6ab) исправлен сбой сервера при доступе к большой JSON-строке
* [Commit 75cd](https://github.com/manticoresoftware/manticoresearch/commit/75cd1342d05cdc352e3dd145cdbc6f79394a165b) исправлена ошибка, когда поле PQ в JSON документе, изменённое индексным стриппером, вызывает неверное совпадение по смежному полю
* [Commit e2f7](https://github.com/manticoresoftware/manticoresearch/commit/e2f775437df922674d772ed4417780492502e65a) исправлен сбой сервера при разборе JSON на сборках RHEL7
* [Commit 3a25](https://github.com/manticoresoftware/manticoresearch/commit/3a25a5808feb3f8b80866991b436d6c1241618c2) исправлен сбой при unescaping JSON, когда слэш находится на границе

* [Commit be9f](https://github.com/manticoresoftware/manticoresearch/commit/be9f497872bea62dbdccd64fc28294fd7776c289) исправлена опция 'skip_empty' для пропуска пустых документов без предупреждения, что они невалидный JSON
* [Commit 266e](https://github.com/manticoresoftware/manticoresearch/commit/266e0e7b088549722a805fc837bf101ff681a5e8) исправлена [\#140](https://github.com/manticoresoftware/manticoresearch/issues/161) выдача 8 знаков после запятой для float, когда 6 недостаточно для точности
* [Commit 3f6d](https://github.com/manticoresoftware/manticoresearch/commit/3f6d23891064ee8e94030c4231497cdd7da33a6a) исправлено создание пустого jsonobj
* [Commit f3c7](https://github.com/manticoresoftware/manticoresearch/commit/f3c7848a59b7eedc67c3403d6d4b37ce6fa94dc8) исправлена [\#160](https://github.com/manticoresoftware/manticoresearch/issues/161) пустая mva выводит NULL вместо пустой строки
* [Commit 0afa](https://github.com/manticoresoftware/manticoresearch/commit/0afa2ed058e5759470b1d7354c722faab34f98bb) исправлен сбой сборки без pthread_getname_np
* [Commit 9405](https://github.com/manticoresoftware/manticoresearch/commit/9405fccdeb27f0302a8d9a848981d30080216777) исправлен сбой сервера при завершении с thread_pool воркерами
## Версия 2.8.0 GA, 28 января 2019
### Улучшения
* Распределённые индексы для percolate индексов
* Новые опции и изменения CALL PQ:
    *   skip_bad_json
    *   mode (sparsed/sharded)
    *   JSON документы можно передавать как json-массив
    *   shift
    *   Имена колонок 'UID', 'Documents', 'Query', 'Tags', 'Filters' переименованы в 'id', 'documents', 'query', 'tags', 'filters'
* DESCRIBE pq TABLE
* SELECT FROM pq WHERE UID теперь невозможен, используйте 'id' вместо
* SELECT по pq индексам наравне с обычными индексами (например, можно фильтровать правила через REGEX())

* ANY/ALL можно использовать на PQ тегах
* Выражения имеют автоконвертацию для JSON полей, без необходимости явного приведения типов
* Встроенная таблица символов 'non_cjk' и символы ngram 'cjk'
* Встроенные коллекции стоп-слов для 50 языков
* В декларации стоп-слов можно перечислять несколько файлов через запятую
* CALL PQ может принимать JSON массив документов
### Исправления ошибок
* [Commit a4e1](https://github.com/manticoresoftware/manticoresearch/commit/a4e19afee54dafdc04b0da53372dafd8c0d332d6) исправлена утечка, связанная с csjon
* [Commit 28d8](https://github.com/manticoresoftware/manticoresearch/commit/28d862774874891a03e361da1b0347ebe8066ce0) исправлен сбой из-за отсутствующего значения в json
* [Commit bf4e](https://github.com/manticoresoftware/manticoresearch/commit/bf4e9ea27c349cdc9ae4e54e960a4a17b90e38fa) исправлено сохранение пустых метаданных для RT-индекса
* [Commit 33b4](https://github.com/manticoresoftware/manticoresearch/commit/33b4573529e7c7c7bce19bf9d54054866f30d99a) исправлен пропавший флаг формы (exact) для последовательности лемматизатора
* [Commit 6b95](https://github.com/manticoresoftware/manticoresearch/commit/6b95d48240d3b5520afade19c249d79e020a5e88) исправлено использование saturate вместо переполнения для строковых атрибутов > 4M
* [Commit 6214](https://github.com/manticoresoftware/manticoresearch/commit/621418b829e70af36aaa322a2f51ece3f86bc499) исправлен сбой сервера при SIGHUP с отключённым индексом
* [Commit 3f7e](https://github.com/manticoresoftware/manticoresearch/commit/3f7e35d1482966cc45d5abbcb2de5de3508d66b0) исправлен сбой сервера при одновременных командах статуса сессии API
* [Commit cd9e](https://github.com/manticoresoftware/manticoresearch/commit/cd9e4f1709a48ddafc128c450c2d882bc11ba50e) исправлен сбой сервера при удалении запроса к RT-индексу с фильтрами по полям
* [Commit 9376](https://github.com/manticoresoftware/manticoresearch/commit/9376470d455fcd256c6abff79c431a6919dfa7ac) исправлен сбой сервера при CALL PQ к распределённому индексу с пустым документом
* [Commit 8868](https://github.com/manticoresoftware/manticoresearch/commit/8868b207644f4f8cc1ab0c270adb35493fed22e1) исправлено обрезание сообщения ошибки Manticore SQL, превышающего 512 символов
* [Commit de9d](https://github.com/manticoresoftware/manticoresearch/commit/de9deda9c142823d6dbf529423417b5c670fae94) исправлен сбой при сохранении percolate-индекса без binlog
* [Commit 2b21](https://github.com/manticoresoftware/manticoresearch/commit/2b219e1a32791e7740bb210b7d408f96abc6374f) исправлен неработающий http-интерфейс в OSX

* [Commit e92c](https://github.com/manticoresoftware/manticoresearch/commit/e92c60240f8f9e1756bfa14fba0bbb402586a389) исправлено ложное сообщение об ошибке indextool при проверке MVA
* [Commit 238b](https://github.com/manticoresoftware/manticoresearch/commit/238bdea59bad89f097403f1c978658ce45f16c70) исправлена блокировка записи при FLUSH RTINDEX, чтобы не блокировать весь индекс во время сохранения и регулярного сброса из rt_flush_period
* [Commit c26a](https://github.com/manticoresoftware/manticoresearch/commit/c26a236bd42b7082db079a51a84172066d337d64) исправлено застревание ALTER percolate-индекса во время ожидания загрузки поиска
* [Commit 9ee5](https://github.com/manticoresoftware/manticoresearch/commit/9ee5703f6c28f57cd5bff5e705966d93fe30d267) исправлено max_children, чтобы при значении 0 использовалось стандартное количество потоков thread_pool
* [Commit 5138](https://github.com/manticoresoftware/manticoresearch/commit/5138fc048a1bd146b271ce6a72fe954344281f69) исправлена ошибка при индексировании данных в индекс с плагином index_token_filter вместе со стоп-словами и stopword_step=0

* [Commit 2add](https://github.com/manticoresoftware/manticoresearch/commit/2add3d319ac62fe450bf60e89033100853dc2ecf) исправлен сбой при отсутствии lemmatizer_base, если в определениях индекса всё ещё используются aot-лемматизаторы
## Версия 2.7.5 GA, 4 декабря 2018
### Улучшения
* Функция REGEX
* limit/offset для поиска в json API
* точки профилирования для qcache
### Исправления ошибок

* [Commit eb3c](https://github.com/manticoresoftware/manticoresearch/commit/eb3c7683b9740cff4cc4cadd2ab9f990b9f7d173) исправлен сбой сервера на FACET с несколькими атрибутами широкой типизации
* [Commit d915](https://github.com/manticoresoftware/manticoresearch/commit/d915cf6696514f4be365ffb5981bccaa15983e65) исправлено неявное групповое объединение при основном списке выборки в запросе FACET
* [Commit 5c25](https://github.com/manticoresoftware/manticoresearch/commit/5c25dc271a8c11a1c82ca3c88e518acbc6a8c164) исправлен сбой при запросе с GROUP N BY
* [Commit 85d3](https://github.com/manticoresoftware/manticoresearch/commit/85d30a232d4ffd89916b673a9b2db946d272a945) исправлена взаимная блокировка при обработке сбоев в операциях с памятью
* [Commit 8516](https://github.com/manticoresoftware/manticoresearch/commit/85166b5ef2be38061d62f2b20f15acaa0311258a) исправлено потребление памяти indextool во время проверки

* [Commit 58fb](https://github.com/manticoresoftware/manticoresearch/commit/58fb03166830d8b23e7ab9ce56309db7ddac22f4) удалён ненужный больше include gmock, так как upstream разрешил этот вопрос самостоятельно
## Версия 2.7.4 GA, 1 ноября 2018
### Улучшения
* SHOW THREADS для удалённых распределённых индексов выводит оригинальный запрос вместо вызова API
* SHOW THREADS новая опция `format=sphinxql` выводит все запросы в формате SQL
* SHOW PROFILE выводит дополнительный этап `clone_attrs`
### Исправления ошибок
* [Commit 4f15](https://github.com/manticoresoftware/manticoresearch/commit/4f155712a0bccc1bd01cc191647bc8cff814888e) исправлена ошибка сборки с libc без malloc_stats, malloc_trim

* [Commit f974](https://github.com/manticoresoftware/manticoresearch/commit/f974f20bda3214a56877c393a192be1a77150958) исправлены специальные символы внутри слов для набора результатов CALL KEYWORDS
* [Commit 0920](https://github.com/manticoresoftware/manticoresearch/commit/092083282ea8ae0b2e72fb6989c57ccec81e74ac) исправлен сломанный CALL KEYWORDS для распределённого индекса через API или удалённого агента
* [Commit fd68](https://github.com/manticoresoftware/manticoresearch/commit/fd686bfe88b720ffd7642e36897ba45161cbd7d2) исправлено распространение agent_query_timeout для распределённого индекса к агентам как max_query_time
* [Commit 4ffa](https://github.com/manticoresoftware/manticoresearch/commit/4ffa623e9d357a3b0b441615089e211f92f8de32) исправлен счётчик общих документов в дисковом чанке, который был затронут командой OPTIMIZE и ломал вычисление веса
* [Commit dcaf](https://github.com/manticoresoftware/manticoresearch/commit/dcaf4e0e3c8e9b0fbec47bbf3307feddec0936a6) исправлены множественные хвостовые попадания в RT-индексе из blended
* [Commit eee3](https://github.com/manticoresoftware/manticoresearch/commit/eee381754e902a43a8f499e1c950198a2a3e6ee0) исправлена взаимная блокировка при ротации

## Версия 2.7.3 GA, 26 сентября 2018
### Улучшения
* option sort_mode для CALL KEYWORDS
* DEBUG по соединению VIP может выполнять 'crash <password>' для намеренного вызова SIGEGV на сервере
* DEBUG может выполнять 'malloc_stats' для дампа статистики malloc в searchd.log, 'malloc_trim' для вызова malloc_trim()
* улучшенный backtrace, если на системе присутствует gdb
### Исправления ошибок
* [Коммит 0f3c](https://github.com/manticoresoftware/manticoresearch/commit/0f3cc333f6129451ad1ae2c97905b6c04ba5d454) исправлен сбой или ошибка rename в Windows
* [Коммит 1455](https://github.com/manticoresoftware/manticoresearch/commit/1455ba2bf8079f03ce7f861a8d9662a360d705de) исправлены сбои сервера на 32-битных системах
* [Коммит ad37](https://github.com/manticoresoftware/manticoresearch/commit/ad3710d53b5cd6a28b8c60352d0f9e285b03d9fa) исправлен сбой или зависание сервера при пустом выражении SNIPPET
* [Коммит b36d](https://github.com/manticoresoftware/manticoresearch/commit/b36d79214364500252665860f000ae85343528b6) исправлен некорректный непрогрессивный optimize и исправлен progressive optimize, чтобы не создавать kill-list для самого старого дискового чанка
* [Коммит 34b0](https://github.com/manticoresoftware/manticoresearch/commit/34b032499afd42ce47a4c7247814b4031094388a) исправлен неправильный ответ queue_max_length для SQL и API в режиме worker пула потоков
* [Коммит ae4b](https://github.com/manticoresoftware/manticoresearch/commit/ae4b3202cbdb8014cabe2b90e269d5cf74f49871) исправлен сбой при добавлении full-scan запроса в PQ индекс с включенными опциями regexp или rlp
* [Коммит f80f](https://github.com/manticoresoftware/manticoresearch/commit/f80f8d5d7560187078868aed9a9575f4549e98aa) исправлен сбой при вызове одного PQ после другого
* [Коммит 9742](https://github.com/manticoresoftware/manticoresearch/commit/9742f5f0866af73f8cd8483ecd18a507ea80dd65) рефакторинг AcquireAccum
* [Коммит 39e5](https://github.com/manticoresoftware/manticoresearch/commit/39e5bc3751b7295222eb76407c5d72ce1dad545b) исправлен утечка памяти после вызова pq
* [Коммит 21bc](https://github.com/manticoresoftware/manticoresearch/commit/21bcc6d17395f0f57dde79f6716ef303b7ea527d) косметический рефакторинг (стиль c++11 c-trs, значения по умолчанию, nullptr)
* [Коммит 2d69](https://github.com/manticoresoftware/manticoresearch/commit/2d690398f14c736956cfdd66feb4d3091d6b3a4d) исправлена утечка памяти при попытке вставить дубликат в PQ индекс
* [Коммит 5ed9](https://github.com/manticoresoftware/manticoresearch/commit/5ed92c4b66da6423fa28f11b1fd59103ed1e1fb9) исправлен сбой с JSON полем IN при больших значениях
* [Коммит 4a52](https://github.com/manticoresoftware/manticoresearch/commit/4a5262e2776aa261e34ba5c914ea60804f59de3f) исправлен сбой сервера при вызове CALL KEYWORDS к RT индексу с установленным ограничением расширения
* [Коммит 5526](https://github.com/manticoresoftware/manticoresearch/commit/552646bb6fefde4e2525298e43e628839b421d3d) исправлен недействительный фильтр в запросе PQ matches
* [Коммит 204f](https://github.com/manticoresoftware/manticoresearch/commit/204f521408b730198a5bab2c20407a3d7df94873) введен малый аллокатор объектов для ptr attrs
* [Коммит 2545](https://github.com/manticoresoftware/manticoresearch/commit/25453e5387152c0575d1eda1b2beb89a49c7493a) рефакторинг ISphFieldFilter до варианта с подсчетом ссылок
* [Коммит 1366](https://github.com/manticoresoftware/manticoresearch/commit/1366ee06e828397074b69bec9265d7147170c60e) исправлена неопределенность/SIGSEGV при использовании strtod на не терминальных строках
* [Коммит 94bc](https://github.com/manticoresoftware/manticoresearch/commit/94bc6fcbdae4d08bcf6da2966e3ec2966091289f) исправлена утечка памяти при обработке набора результатов json
* [Коммит e78e](https://github.com/manticoresoftware/manticoresearch/commit/e78e9c948963416caff9e4e46296a58080107835) исправлено чтение за пределами блока памяти при применении добавления атрибута
* [Коммит fad5](https://github.com/manticoresoftware/manticoresearch/commit/fad572fb543606b33e688eb82a485b86011545fd) рефакторинг CSphDict до варианта с подсчетом ссылок
* [Коммит fd84](https://github.com/manticoresoftware/manticoresearch/commit/fd841a472b472280c93f4af3506851d19f6adc45) исправлена утечка внутреннего типа AOT наружу
* [Коммит 5ee7](https://github.com/manticoresoftware/manticoresearch/commit/5ee7f208c7685f2ebb9e5623a8802b721e2a7f0a) исправлена утечка памяти в управлении токенизатором
* [Коммит 116c](https://github.com/manticoresoftware/manticoresearch/commit/116c5f1abebee9a0d99afe93546a1d8e4c6c6958) исправлена утечка памяти в grouper
* [Коммит 56fd](https://github.com/manticoresoftware/manticoresearch/commit/56fdbc9ab38973a9a0ba8ccee45f71cf33c16423) специальное освобождение/копирование для динамических ptr в matches (утечка памяти в grouper)
* [Коммит b1fc](https://github.com/manticoresoftware/manticoresearch/commit/31db660f378541aa0d1e7cf2e7209bb0a1f47fd8) исправлена утечка памяти динамических строк для RT
* [Коммит 517b](https://github.com/manticoresoftware/manticoresearch/commit/517b9e8749f092e923cd2884b674b5bb84e20755) рефакторинг grouper
* [Коммит b1fc](https://github.com/manticoresoftware/manticoresearch/commit/b1fc16140e5dc44290686330b476a254e0092107) незначительный рефакторинг (c++11 c-trs, некоторые переработки формата)
* [Коммит 7034](https://github.com/manticoresoftware/manticoresearch/commit/7034e073f4cf2844762e0a464b7c3de05d3d122b) рефакторинг ISphMatchComparator до варианта с подсчетом ссылок
* [Коммит b1fc](https://github.com/manticoresoftware/manticoresearch/commit/b1fc16140e5dc44290686330b476a254e0092107) приватизация клонировщика
* [Коммит efbc](https://github.com/manticoresoftware/manticoresearch/commit/efbc0511d6809c4a57453c7283f9bf53f3fb8d97) упрощение нативного little-endian для MVA_UPSIZE, DOCINFO2ID_T, DOCINFOSETID
* [Коммит 6da0](https://github.com/manticoresoftware/manticoresearch/commit/6da0df4ac9946fc59f98d229b90e12c9221dd4c0) добавлена поддержка valgrind для ubertests
* [Коммит 1d17](https://github.com/manticoresoftware/manticoresearch/commit/1d17669cb7ec54ac80b392e101f37a688eb98080) исправлен сбой из-за гонки флага 'success' на соединении
* [Коммит 5a09](https://github.com/manticoresoftware/manticoresearch/commit/5a09c32d7ded56b2c0bd3e2ad7968cb8d6362919) переключение epoll на режим edge-triggered
* [Коммит 5d52](https://github.com/manticoresoftware/manticoresearch/commit/5d528682737fca03a4352a3093b1319ec27d598c) исправлен оператор IN в выражении с форматированием, похожим на at filter
* [Коммит bd8b](https://github.com/manticoresoftware/manticoresearch/commit/bd8b3c976ff8b4667f55dfdd101b20a920137ac5) исправлен сбой в RT индексе при коммите документа с большим docid
* [Коммит ce65](https://github.com/manticoresoftware/manticoresearch/commit/ce656b83b747ce7c55795b53915770c13fb99cfe) исправлены опции без аргументов в indextool
* [Коммит 08c9](https://github.com/manticoresoftware/manticoresearch/commit/08c9507177820f441f534bf06a11dac5e54bebb4) исправлена утечка памяти расширенного ключевого слова

* [Коммит 30c7](https://github.com/manticoresoftware/manticoresearch/commit/30c75a2f525ec9bda625d903acdc9d4d2e2413dc) исправлена утечка памяти json grouper
* [Коммит 6023](https://github.com/manticoresoftware/manticoresearch/commit/6023f269b6f2080e4d380b0957605ef8107c8c9f) исправлена утечка глобальных пользовательских переменных
* [Коммит 7c13](https://github.com/manticoresoftware/manticoresearch/commit/7c138f15ca23c0c8717fa12041e3db7f988fcf15) исправлена утечка динамических строк при раннем отклонении совпадений
* [Коммит 9154](https://github.com/manticoresoftware/manticoresearch/commit/9154b18eaed5302e8330cb3a95bd968959ccb312) исправлена утечка памяти в length(<expression>)
* [Коммит 43fc](https://github.com/manticoresoftware/manticoresearch/commit/43fca3a4e26139765d0fac8de054200dc4875fc6) исправлена утечка памяти из-за strdup() в парсере
* [Коммит 71ff](https://github.com/manticoresoftware/manticoresearch/commit/71ff77737e63d1019b7c9804dca7fa2351025dba) рефакторинг парсера выражений для точного отслеживания счетчиков ссылок
## Версия 2.7.2 GA, 27 августа 2018
### Улучшения
* совместимость с клиентами MySQL 8
* [TRUNCATE](Emptying_a_table.md) С RECONFIGURE
* удалён счётчик памяти в SHOW STATUS для RT индексов
* глобальный кэш мультиагентов

* улучшенный IOCP на Windows
* VIP соединения для HTTP протокола
* команда Manticore SQL [DEBUG](Reporting_bugs.md#DEBUG), которая может запускать различные подкоманды
* [shutdown_token](Server_settings/Searchd.md#shutdown_token) - SHA1 хеш пароля, необходимый для вызова `shutdown` через команду DEBUG
* новые статистики в SHOW AGENT STATUS (_ping, _has_perspool, _need_resolve)
* опция --verbose у indexer теперь принимает \[debugvv\] для вывода отладочных сообщений
### Исправления ошибок
* [Коммит 3900](https://github.com/manticoresoftware/manticoresearch/commit/390082a7be0a1f9539b30361d11d54de35c62a44) удалён wlock при оптимизации
* [Коммит 4c33](https://github.com/manticoresoftware/manticoresearch/commit/4c337648329be22373be33333cbc9c5664d18b80) исправлен wlock при перезагрузке настроек индекса
* [Коммит b5ea](https://github.com/manticoresoftware/manticoresearch/commit/b5ea8dc0b18a23dbd739e92c66dd93b0094df14b) исправлена утечка памяти при запросе с фильтром JSON
* [Коммит 930e](https://github.com/manticoresoftware/manticoresearch/commit/930e831a43fd3d1ce996185be968d1df49a088d0) исправлены пустые документы в PQ наборе результатов
* [Коммит 53de](https://github.com/manticoresoftware/manticoresearch/commit/53deecfecb374431f84f8592a1095a77407b8aea) исправлено смешение задач из-за удалённой задачи
* [Коммит cad9](https://github.com/manticoresoftware/manticoresearch/commit/cad9b9c617719aad97e9f5a2ae93491153d8e504) исправлен неправильный подсчёт удалённого хоста
* [Коммит 9000](https://github.com/manticoresoftware/manticoresearch/commit/90008cc15c4684d445d37a391bb4a667bbc2e365) исправлена утечка памяти распарсенных описателей агентов
* [Коммит 978d](https://github.com/manticoresoftware/manticoresearch/commit/978d839641ad6040c22c9fc3b703b1a02685f266) исправлена утечка в поиске
* [Коммит 0193](https://github.com/manticoresoftware/manticoresearch/commit/0193946105ac08913362b7d7acf0c90f9b38dc85) косметические изменения по explicit/inline c-trs, использования override/final
* [Коммит 943e](https://github.com/manticoresoftware/manticoresearch/commit/943e2997b33f9b2eda23d94a66068910f9476ebd) исправлена утечка json в локальной/удалённой схеме
* [Коммит 02db](https://github.com/manticoresoftware/manticoresearch/commit/02dbdd6f1d0b72e7e458ebebdd2c67f989577e6b) исправлена утечка json сортирующего столбца выражения в локальной/удалённой схеме
* [Коммит c74d](https://github.com/manticoresoftware/manticoresearch/commit/c74d0b4035e4214510376845b22cb676a8da24a3) исправлена утечка константного алиаса
* [Коммит 6e5b](https://github.com/manticoresoftware/manticoresearch/commit/6e5b57e1d3952c9695376728a143bf5434208f53) исправлена утечка потока preread
* [Коммит 39c7](https://github.com/manticoresoftware/manticoresearch/commit/39c740fef5bf71da0bee60a35f5dd9f471af850f) исправлено зависание при выходе из-за зависания ожидания в netloop
* [Коммит adaf](https://github.com/manticoresoftware/manticoresearch/commit/adaf9772f95772ad6d5297acace6c5cb92b56ad5) исправлено зависание поведения 'ping' при смене HA агента на обычный хост
* [Коммит 32c4](https://github.com/manticoresoftware/manticoresearch/commit/32c40eac9cdd40b15d79e58b2f15416f164f9701) отдельный gc для dashboard storage
* [Коммит 511a](https://github.com/manticoresoftware/manticoresearch/commit/511a3cf2c1f1858f6e94fe9f8175b7216db3cbd6) исправлена фиксация ref-counted ptr
* [Коммит 32c4](https://github.com/manticoresoftware/manticoresearch/commit/af2326cd2927effbad7bec88e8dc238a566cf27e) исправлен сбой indextool при несуществующем индексе
* [Коммит 156e](https://github.com/manticoresoftware/manticoresearch/commit/156edc717dc63d3ea120f42466eba6e022da88be) исправлено имя выходного файла атрибута/поля, превышающего лимит в xmlpipe индексировании
* [Коммит cdac](https://github.com/manticoresoftware/manticoresearch/commit/cdac6d1f17d384bfc8bae49e91241b2f346c3830) исправлено значение indexer по умолчанию, если в конфиге отсутствует секция indexer
* [Коммит e61e](https://github.com/manticoresoftware/manticoresearch/commit/e61ec00b6b27d1d5878247e2ee817f3b1e7fde16) исправлены неправильные встроенные стоп-слова в дисковом куске RT индекса после перезапуска сервера

* [Коммит 5fba](https://github.com/manticoresoftware/manticoresearch/commit/5fba49d5e28c17de4c0acbd984466127f42de6e8) пропущены фантомные (уже закрытые, но не окончательно удалённые из poller) соединения
* [Коммит f22a](https://github.com/manticoresoftware/manticoresearch/commit/f22ae34b623906b3c5a05a06198dad2e548dc541) исправлены смешанные (осиротевшие) сетевые задачи
* [Коммит 4689](https://github.com/manticoresoftware/manticoresearch/commit/46890e70eb8dcd00c1bf1b030538b8f131c601c2) исправлен сбой при read action после записи
* [Commit 03f9](https://github.com/manticoresoftware/manticoresearch/commit/03f9df904f611c438d5ebcaaeef911b0dc8d6c39) исправлены ошибки завершения работы searchd при запуске тестов на Windows
* [Commit e925](https://github.com/manticoresoftware/manticoresearch/commit/e9255ed2704790a3d7f5273d38b85433463cb3ff) исправлена обработка кода EINPROGRESS при обычном connect()
* [Commit 248b](https://github.com/manticoresoftware/manticoresearch/commit/248b72f1544b27e9e82773284050d44eeb1714e1) исправлены тайм-ауты соединения при работе с TFO

## Version 2.7.1 GA, 4 July 2018
### Improvements
* улучшена производительность подстановочных знаков при сопоставлении нескольких документов в PQ
* поддержка запросов fullscan в PQ
* поддержка атрибутов MVA в PQ


* поддержка regexp и RLP для percolate индексов
### Bugfixes
* [Commit 6885](https://github.com/manticoresoftware/manticoresearch/commit/68856261b41f6950666f9e5122726839fb3c71d0) исправлена потеря строки запроса
* [Commit 0f17](https://github.com/manticoresoftware/manticoresearch/commit/0f1770943cba53bfd2e0edfdf0b1495ff0dd0e89) исправлена пустая информация в SHOW THREADS
* [Commit 53fa](https://github.com/manticoresoftware/manticoresearch/commit/53faa36a916bb87868e83aa79c1a1f972fb20ca0) исправлен сбой при сопоставлении с оператором NOTNEAR
* [Commit 2602](https://github.com/manticoresoftware/manticoresearch/commit/26029a7cd77b518fb3a27588d7db6c8bffc73b0f) исправлено сообщение об ошибке при неверном фильтре для удаления PQ
## Version 2.7.0 GA, 11 June 2018
### Improvements
* сокращено число системных вызовов для уменьшения влияния патчей Meltdown и Spectre
* внутренний переписан менеджмент локальных индексов
* рефакторинг удалённых сниппетов
* полная перезагрузка конфигурации
* все соединения с узлами теперь независимы
* улучшения протокола
* на Windows связь переключена с wsapoll на IO completion ports

* TFO может использоваться для связи между мастером и узлами
* [SHOW STATUS](Node_info_and_management/Node_status.md#SHOW-STATUS) теперь выводит server version и mysql_version_string
* добавлена опция `docs_id` для документов в вызове CALL PQ.
* фильтр percolate запросов теперь может содержать выражения
* распределённые индексы могут работать с FEDERATED
* заглушка SHOW NAMES COLLATE и `SET wait_timeout` (для лучшей совместимости с ProxySQL)
### Bugfixes
* [Commit 5bcf](https://github.com/manticoresoftware/manticoresearch/commit/5bcff05d19e495f1a44f190885b25cbb6f217c4a) исправлены добавления "не равно" в теги PQ
* [Commit 9ebc](https://github.com/manticoresoftware/manticoresearch/commit/9ebc58916cd515eaa88da66d0895aebf0d1f2b5f) исправлено добавление поля ID документа в JSON документ CALL PQ
* [Commit 8ae0](https://github.com/manticoresoftware/manticoresearch/commit/8ae0e593a286110d8505d88baa3ac9e433cb38c9) исправлены обработчики flush для индекса PQ
* [Commit c24b](https://github.com/manticoresoftware/manticoresearch/commit/c24b152344ecd77661566cddb803487f51d3c1aa) исправлена фильтрация PQ по JSON и строковым атрибутам
* [Commit 1b8b](https://github.com/manticoresoftware/manticoresearch/commit/1b8bdde19aaa362785ea4c33bdc019154f8fbac4) исправлен парсинг пустой JSON строки


* [Commit 1ad8](https://github.com/manticoresoftware/manticoresearch/commit/1ad8a07dbfc1131913a8d92c261fbb48f934e8b7) исправлен сбой при мультизапросе с OR фильтрами
* [Commit 69b8](https://github.com/manticoresoftware/manticoresearch/commit/69b89806380a229e36287ad9daf6f0ea2b5453eb) исправлен indextool для использования общей секции конфига (опция lemmatizer_base) для команд (dumpheader)
* [Commit 6dbe](https://github.com/manticoresoftware/manticoresearch/commit/6dbeaf2389d2af6a9dfccb56bfc986685b9f203e) исправлена пустая строка в результирующем наборе и фильтре
* [Commit 39c4](https://github.com/manticoresoftware/manticoresearch/commit/39c4eb55ecc85f6dd54f6c8c6d6dfcf87fd8748e) исправлены отрицательные значения ID документов
* [Commit 266b](https://github.com/manticoresoftware/manticoresearch/commit/266b7054f98e7d30fc2829b5c2467dc9f90def92) исправлена длина усечения слова для очень длинных проиндексированных слов
* [Commit 4782](https://github.com/manticoresoftware/manticoresearch/commit/47823b0bd06cbd016a24345c99bb8693790b43dd) исправлено сопоставление нескольких документов с помощью wildcard запросов в PQ
## Version 2.6.4 GA, 3 May 2018

### Features and improvements
* поддержка MySQL FEDERATED движка [support](Extensions/FEDERATED.md)
* MySQL пакеты теперь возвращают флаг SERVER_STATUS_AUTOCOMMIT, добавляя совместимость с ProxySQL
* [listen_tfo](Server_settings/Searchd.md#listen_tfo) - включение TCP Fast Open для всех слушателей
* indexer --dumpheader теперь может выгружать также RT заголовок из .meta файла
* скрипт сборки cmake для Ubuntu Bionic
### Bugfixes
* [Commit 355b](https://github.com/manticoresoftware/manticoresearch/commit/355b11629174813abd9cd5bf2233be0783f77745) исправлены неверные записи кэша запросов для RT индекса;

* [Commit 546e](https://github.com/manticoresoftware/manticoresearch/commit/546e2297a01cc2913bc0d33052d5bcefae8737eb) исправлено потеря настроек индекса после seamless rotation
* [Commit 0c45](https://github.com/manticoresoftware/manticoresearch/commit/0c4509898393993bc87194d2aca2070395ff7f83) исправлена установка длин infix и prefix; добавлено предупреждение о неподдерживаемой длине infix
* [Commit 8054](https://github.com/manticoresoftware/manticoresearch/commit/80542fa54d727c781635918e8d4d65a20c2fbd1e) исправлен порядок автофлаш RT индексов

* [Commit 705d](https://github.com/manticoresoftware/manticoresearch/commit/705d8c5f407726ed1c6f9745ecffa375682c8969) исправлены проблемы со схемой результирующего набора для индексов с множественными атрибутами и запросами к нескольким индексам
* [Commit b0ba](https://github.com/manticoresoftware/manticoresearch/commit/b0ba932fff4a082f642cb737381c8a70215d4cc9) исправлено потеря некоторых хитов при пакетной вставке с дубликатами документов
* [Commit 4510](https://github.com/manticoresoftware/manticoresearch/commit/4510fa44ad54b5bbac53c591144501565130fcae) исправлено сбой optimize при слиянии дисковых чанков RT индекса с большим количеством документов
## Version 2.6.3 GA, 28 March 2018
### Improvements
* jemalloc при компиляции. Если jemalloc присутствует в системе, его можно включить флагом cmake `-DUSE_JEMALLOC=1`


### Bugfixes
* [Commit 85a6](https://github.com/manticoresoftware/manticoresearch/commit/85a6d7e3db75f28c491042409bb677ed39eba07b) исправлена опция log expand_keywords в журнале запросов Manticore SQL
* [Commit caaa](https://github.com/manticoresoftware/manticoresearch/commit/caaa3842f4dbd8507fd9e20bf3371a0c8ec0c087) исправлен HTTP интерфейс для корректной обработки запросов большого размера
* [Commit e386](https://github.com/manticoresoftware/manticoresearch/commit/e386d842b36195719e459b2db1fb20e402f9a51f) исправлен сбой сервера при DELETE в RT индекс с включенным index_field_lengths
* [Commit cd53](https://github.com/manticoresoftware/manticoresearch/commit/ce528f37eb5d8284661928f6f3b340a4a1985253) исправлена опция cpustats searchd cli для работы с неподдерживаемыми системами
* [Commit 8740](https://github.com/manticoresoftware/manticoresearch/commit/8740fd63ae842c2ab0e51a8fcd6180d787ed57ec) исправлено сопоставление utf8 подстрок с заданными минимальными длинами
## Version 2.6.2 GA, 23 February 2018
### Improvements

* улучшена производительность [Percolate Queries](Searching/Percolate_query.md) в случае использования оператора NOT и для пакетных документов.
* [percolate_query_call](Searching/Percolate_query.md) теперь может использовать несколько потоков в зависимости от [dist_threads](Server_settings/Searchd.md#threads)
* новый оператор полнотекстового сопоставления NOTNEAR/N
* LIMIT для SELECT на перколяторных индексах
* [expand_keywords](Searching/Options.md#expand_keywords) теперь может принимать 'start','exact' (где 'start,exact' имеет тот же эффект, что и '1')
* ranged-main-query для [joined fields](Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Processing_fetched_data.md#sql_joined_field), который использует диапазонный запрос, заданный sql_query_range
### Bugfixes
* [Commit 72dc](https://github.com/manticoresoftware/manticoresearch/commit/72dcf669744e9b7d636dfc213d24df85ab301f6b) исправлен сбой при поиске по сегментам ram; устранена взаимная блокировка при сохранении дискового чанка с двойным буфером; устранена взаимная блокировка при сохранении дискового чанка во время оптимизации
* [Commit 3613](https://github.com/manticoresoftware/manticoresearch/commit/36137149a1c3c0893bdda5a28fc7e8244bf2d4ae) исправлен сбой индексатора при xml-схеме с вложенным пустым именем атрибута


* [Commit 48d7](https://github.com/manticoresoftware/manticoresearch/commit/48d7e8001d2a66466ca64577f27ddc5421a67251) исправлено ошибочное удаление не принадлежащего pid-файла
* [Commit a556](https://github.com/manticoresoftware/manticoresearch/commit/a5563a465ddc59ef71e65f17b68bc33f9700e838) исправлены сиротские fifo, иногда оставшиеся во временной папке
* [Commit 2376](https://github.com/manticoresoftware/manticoresearch/commit/2376e8fc4508944b96959bd10686c6d51f5145e8) исправлен пустой результат FACET с неправильной NULL строкой
* [Commit 4842](https://github.com/manticoresoftware/manticoresearch/commit/4842b67acdd5da75672db6c08ac563b48577ddd7) исправлена сломанная блокировка индекса при запуске сервера как службы Windows
* [Commit be35](https://github.com/manticoresoftware/manticoresearch/commit/be35feef54ded59125750916cd211e56108eddae) исправлены неправильные библиотеки iconv на mac os
* [Commit 8374](https://github.com/manticoresoftware/manticoresearch/commit/83744a977d7837d1d87cd506b88b0e7eb199efe6) исправлен неправильный count(\*)
## Version 2.6.1 GA, 26 January 2018
### Improvements

* [agent_retry_count](Server_settings/Searchd.md#agent_retry_count) для агентов с зеркалами теперь задаёт количество повторных попыток на зеркало, а не на агента в целом; общее количество повторов на агента равно agent_retry_count\*mirrors.
* [agent_retry_count](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_retry_count) теперь можно указывать для каждого индекса отдельно, переопределяя глобальное значение. Добавлен псевдоним [mirror_retry_count](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#mirror_retry_count).
* retry_count теперь можно указать в определении агента, и значение представляет количество попыток для агента
* Percolate Queries теперь доступны в HTTP JSON API по адресу [/json/pq](Searching/Percolate_query.md#Performing-a-percolate-query-with-CALL-PQ).
* Добавлены опции -h и -v (help и version) для исполняемых файлов
* поддержка [morphology_skip_fields](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) для индексов Real-Time
### Bugfixes


* [Commit a40b](https://github.com/manticoresoftware/manticore/commit/a40b0793feff65e40d10062568d9847c08d10f57) исправлен ranged-main-query для корректной работы с sql_range_step при применении к MVA полю
* [Commit f2f5](https://github.com/manticoresoftware/manticore/commit/f2f53757db45bcfb1544263ce0817e856656a621) исправлена проблема с зависанием цикла системы blackhole и видимостью отключенных агентов blackhole
* [Commit 84e1](https://github.com/manticoresoftware/manticore/commit/84e1f54aef25e0fce98870ad2dd784db5116f1d6) исправлен идентификатор запроса для обеспечения согласованности, устранено дублирование id для сохранённых запросов
* [Commit 1948](https://github.com/manticoresoftware/manticore/commit/19484231814fcb82b21763a3a4a9f45adc6b2d40) исправлен сбой сервера при завершении работы из различных состояний
* [Commit 9a70](https://github.com/manticoresoftware/manticore/commit/9a706b499a1d61a90076065a1a703029d49db958)  [Commit 3495](https://github.com/manticoresoftware/manticore/commit/3495fd70cba8846b1a50d55d6679b039414c3d2a) исправлены таймауты на длительных запросах
* [Commit 3359](https://github.com/manticoresoftware/manticore/commit/3359bcd89b4f79a645fe84b8cf8616ce0addff02) проведена рефакторинг сетевого опроса master-agent на системах с kqueue (Mac OS X, BSD).
## Version 2.6.0, 29 December 2017

### Features and improvements
* [HTTP JSON](Connecting_to_the_server/HTTP.md): теперь JSON запросы могут сравнивать атрибуты на равенство, многозначные атрибуты (MVA) и JSON атрибуты можно использовать в вставках и обновлениях, обновления и удаления через JSON API возможны для распределённых индексов
* [Percolate Queries](Searching/Percolate_query.md)
* Удалена поддержка 32-битных docid из кода. Также удалён весь код, который конвертирует/загружает устаревшие индексы с 32-битными docid.
* [Морфология только для определённых полей](https://github.com/manticoresoftware/manticore/issues/7). Новая директива индекса morphology_skip_fields позволяет указать список полей, для которых морфология не применяется.
* [expand_keywords теперь может быть директивой выполнения запроса, устанавливаемой с помощью оператора OPTION](https://github.com/manticoresoftware/manticore/issues/8)

### Исправления ошибок
* [Коммит 0cfa](https://github.com/manticoresoftware/manticore/commit/0cfae4c) исправил сбой на debug-сборке сервера (и возможно UB на релизе) при сборке с rlp

* [Коммит 3242](https://github.com/manticoresoftware/manticore/commit/324291e) исправлена оптимизация RT-индекса с включённой прогрессивной опцией, которая некорректно сливает списки удаления (kill-lists)
* [Коммит ac0e](https://github.com/manticoresoftware/manticore/commit/ac0efee) мелкий сбой на mac
* много мелких исправлений после тщательного статического анализа кода
* другие мелкие исправления ошибок
### Обновление
В этом релизе мы изменили внутренний протокол, используемый мастерами и агентами для общения друг с другом. Если вы запускаете Manticoresearch в распределенной среде с несколькими инстансами, убедитесь, что вы сначала обновили агенты, а затем мастера.
## Версия 2.5.1, 23 ноября 2017
### Функции и улучшения
* JSON-запросы по [HTTP API протоколу](Connecting_to_the_server/HTTP.md). Поддерживаются операции поиска, вставки, обновления, удаления, замены. Команды манипуляции данными можно выполнять пакетно, но есть некоторые ограничения, так как MVA и JSON-атрибуты нельзя использовать для вставок, замен или обновлений.
* Команда [RELOAD INDEXES](Data_creation_and_modification/Adding_data_from_external_storages/Rotating_a_table.md#RELOAD-TABLES)
* Команда [FLUSH LOGS](Logging/Rotating_query_and_server_logs.md)

* [SHOW THREADS](Node_info_and_management/SHOW_THREADS.md) может показывать прогресс оптимизации, ротации или сбросов.
* GROUP N BY корректно работает с MVA-атрибутами
* blackhole-агенты теперь запускаются в отдельном потоке, чтобы не влиять на запросы мастера
* реализован подсчет ссылок на индексы, чтобы избежать зависаний при ротациях и высокой нагрузке
* реализован SHA1-хэш, пока не доступный внешне
* исправления для компиляции на FreeBSD, macOS и Alpine
### Исправления ошибок
* [Коммит 9897](https://github.com/manticoresoftware/manticore/commit/989752b) регрессия фильтра с блоковым индексом

* [Коммит b1c3](https://github.com/manticoresoftware/manticore/commit/b1c3864) переименование PAGE_SIZE -> ARENA_PAGE_SIZE для совместимости с musl
* [Коммит f213](https://github.com/manticoresoftware/manticore/commit/f2133cc) отключение googletests для cmake < 3.1.0
* [Коммит f30e](https://github.com/manticoresoftware/manticore/commit/0839de7) сбой привязки сокета при перезапуске сервера
* [Коммит 0807](https://github.com/manticoresoftware/manticore/commit/0807240) исправлен сбой сервера при завершении работы
* [Коммит 3e3a](https://github.com/manticoresoftware/manticore/commit/3e3acc3) исправлен показ потоков для системного blackhole-потока
* [Коммит 262c](https://github.com/manticoresoftware/manticore/commit/262c3fe) рефакторинг проверки конфигурации iconv, исправления сборки для FreeBSD и Darwin
## Версия 2.4.1 GA, 16 октября 2017
### Функции и улучшения
* Оператор OR в WHERE между фильтрами атрибутов
* Режим обслуживания (SET MAINTENANCE=1)
* [CALL KEYWORDS](Searching/Autocomplete.md#CALL-KEYWORDS) доступны для распределённых индексов
* [Группировка в UTC](Server_settings/Searchd.md#grouping_in_utc)
* [query_log_mode](Server_settings/Searchd.md#query_log_mode) для пользовательских прав доступа к логам
* Вес полей может быть нулевым или отрицательным

* [max_query_time](Searching/Options.md#max_query_time) теперь может влиять на полносканирование
* добавлены [net_wait_tm](Server_settings/Searchd.md#net_wait_tm), [net_throttle_accept](Server_settings/Searchd.md#net_throttle_accept) и [net_throttle_action](Server_settings/Searchd.md#net_throttle_action) для тонкой настройки сетевого потока (в случае workers=thread_pool)

* COUNT DISTINCT работает с фасетными запросами
* IN можно использовать с JSON-массивами чисел с плавающей точкой
* оптимизация мультизапросов больше не ломается из-за целочисленных или числовых выражений
* [SHOW META](Node_info_and_management/SHOW_META.md) показывает строку `multiplier` при использовании оптимизации мультизапросов
### Компиляция

Manticore Search собирается с помощью cmake, минимальная версия gcc для компиляции - 4.7.2.
### Папки и сервис
* Manticore Search работает под пользователем `manticore`.
* Папка по умолчанию для данных теперь `/var/lib/manticore/`.
* Папка по умолчанию для логов теперь `/var/log/manticore/`.
* Папка по умолчанию для pid теперь `/var/run/manticore/`.
### Исправления ошибок
* [Коммит a58c](https://github.com/manticoresoftware/manticore/commit/a58c619) исправлена команда SHOW COLLATION, которая ломала java-коннектор
* [Коммит 631c](https://github.com/manticoresoftware/manticore/commit/631cf4e) исправлены сбои при обработке распределённых индексов; добавлены блокировки для хеша распределённого индекса; удалены операторы перемещения и копирования из агента
* [Коммит 942b](https://github.com/manticoresoftware/manticore/commit/942bec0) исправлены сбои при обработке распределённых индексов из-за параллельных переподключений
* [Коммит e5c1](https://github.com/manticoresoftware/manticore/commit/e5c1ed2) исправлен сбой обработчика аварий при сохранении запроса в лог сервера
* [Коммит 4a4b](https://github.com/manticoresoftware/manticore/commit/4a4bda5) исправлен сбой с атрибутами пула в мультизапросах
* [Коммит 3873](https://github.com/manticoresoftware/manticore/commit/3873bfb) уменьшен размер core-файла, предотвращая включение страниц индекса в core
* [Коммит 11e6](https://github.com/manticoresoftware/manticore/commit/11e6254) исправлены сбои searchd при старте при указании неправильных агентов
* [Коммит 4ca6](https://github.com/manticoresoftware/manticore/commit/4ca6350) исправлена ошибка индекса в sql_query_killlist-запросе
* [Коммит 123a](https://github.com/manticoresoftware/manticore/commit/123a9f0) исправлена ошибка fold_lemmas=1 в подсчёте совпадений
* [Коммит cb99](https://github.com/manticoresoftware/manticore/commit/cb99164) исправлено неконсистентное поведение html_strip
* [Коммит e406](https://github.com/manticoresoftware/manticore/commit/e406761) исправлена потеря новых настроек optimize rt index; исправлены утечки блокировок при sync optimize;

* [Коммит 86ae](https://github.com/manticoresoftware/manticore/commit/86aeb82) исправлена обработка ошибочных мультизапросов
* [Commit 2645](https://github.com/manticoresoftware/manticore/commit/2645230) исправлена зависимость результата от порядка выполнения нескольких запросов

## Версия 2.3.3, 06 July 2017
* Брендинг Мантикоры