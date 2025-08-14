# Changelog

## Dev version

* 🪲 [v13.6.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.5) [ Issue #3467](https://github.com/manticoresoftware/manticoresearch/issues/3467) Исправлены проблемы с DBeaver и другими интеграциями, вызывавшими ошибки "unknown sysvar".
* 🪲 [v13.6.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.4) [ Issue #3524](https://github.com/manticoresoftware/manticoresearch/issues/3524) Исправлена ошибка с конкатенацией embedding нескольких полей; также исправлена генерация embedding из запросов.
* 🪲 [v13.6.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.3) [ Issue #3641](https://github.com/manticoresoftware/manticoresearch/issues/3641) Исправлена ошибка в версии 13.6.0, при которой фраза теряла все ключевые слова в скобках, кроме первого.
* 🪲 [v13.6.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.2) Исправлена утечка памяти в transform_phrase.
* 🪲 [v13.6.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.1) Исправлена утечка памяти в версии 13.6.0.
* 🆕 [v13.6.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.0) [ Issue #2226](https://github.com/manticoresoftware/manticoresearch/issues/2226) Добавлена поддержка явного оператора '|' (ИЛИ) в PHRASE, PROXIMITY и QUORUM.
* 🪲 [v13.5.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.5.2) [ Issue #3651](https://github.com/manticoresoftware/manticoresearch/issues/3651) Исправлены дополнительные проблемы, связанные с фуззи-тестированием полнотекстового поиска.
* 🪲 [v13.5.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.5.1) [ Issue #3560](https://github.com/manticoresoftware/manticoresearch/issues/3560) Исправлен случай, когда OPTIMIZE TABLE мог зависать бесконечно при работе с данными KNN.
* 🆕 [v13.5.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.5.0) [ PR #3591](https://github.com/manticoresoftware/manticoresearch/pull/3591) Автоматическая генерация embedding в запросах (WIP).
* 🪲 [v13.4.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.4.2) [ Issue #2544](https://github.com/manticoresoftware/manticoresearch/issues/2544) Исправлена ошибка, при которой добавление float_vector колонки могло портить индексы.
* 🪲 [v13.4.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.4.1) Добавлено фуззи-тестирование парсера полнотекстового поиска и исправлены несколько найденных ошибок.
* 🆕 [v13.4.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.4.0) [ PR #3585](https://github.com/manticoresoftware/manticoresearch/pull/3585) Исправлена логика выбора количества потоков для buddy из buddy_path config, если задано, вместо использования значения демона.
* 🪲 [v13.3.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.3.1) [ Issue #3583](https://github.com/manticoresoftware/manticoresearch/issues/3583) Исправлен сбой при использовании сложных булевых фильтров с подсветкой.
* 🆕 [v13.3.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.3.0) [ PR #3577](https://github.com/manticoresoftware/manticoresearch/pull/3577) Поддержка JOIN с локальными распределёнными таблицами.
* 🪲 [v13.2.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.7) [ Issue #3481](https://github.com/manticoresoftware/manticoresearch/issues/3481) Исправлен сбой при одновременном использовании HTTP обновления, распределённой таблицы и неправильного кластера репликации.
* 🪲 [v13.2.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.6) [ PR #3567](https://github.com/manticoresoftware/manticoresearch/pull/3567) Обновлена зависимость manticore-backup до версии 1.9.6.
* 🪲 [v13.2.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.5) [ PR #3565](https://github.com/manticoresoftware/manticoresearch/pull/3565) Исправлена настройка CI для улучшения совместимости Docker образов.
* 🪲 [v13.2.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.4) Исправлена обработка длинных токенов. Некоторые специальные токены (например, шаблоны regex) могли создавать слова слишком большой длины, теперь они сокращаются перед использованием.

## Version 13.2.3
**Released**: July 8th 2025

### Breaking Changes
* ⚠️ [v13.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.0.0)  Обновлен API библиотеки KNN для поддержки пустых значений [float_vector](https://manual.manticoresearch.com/Creating_a_table/Data_types#Float-vector). Это обновление не меняет формат данных, но увеличивает версию API Manticore Search / MCL.
* ⚠️ [v12.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/12.0.0) [ PR #3516](https://github.com/manticoresoftware/manticoresearch/pull/3516) Исправлена ошибка с некорректными ID строк источника и назначения при обучении и построении KNN индекса. Это обновление не меняет формат данных, но увеличивает версию API Manticore Search / MCL.
* ⚠️ [v11.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.0.0)  Добавлена поддержка новых функций поиска KNN векторов, таких как квантизация, пересчёт и сверхвыборка. Эта версия меняет формат данных KNN и синтаксис SQL [KNN_DIST()](https://manual.manticoresearch.com/Searching/KNN?client=SQL#KNN-vector-search). Новая версия может читать старые данные, но старые версии не смогут читать новый формат.

### New Features and Improvements
* 🆕 [v13.2.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.0) [ PR #3549](https://github.com/manticoresoftware/manticoresearch/issues/3525) Исправлена проблема с `@@collation_database`, вызывавшая несовместимость с некоторыми версиями mysqldump
* 🆕 [v13.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.1.0) [ Issue #3489](https://github.com/manticoresoftware/manticoresearch/issues/3489) Исправлена ошибка в Fuzzy Search, при которой не парсились определённые SQL запросы
* 🆕 [v12.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/12.1.0) [ Issue #3426](https://github.com/manticoresoftware/manticoresearch/issues/3426) Добавлена поддержка операционных систем RHEL 10
* 🆕 [v11.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.1.0)  Добавлена поддержка пустых векторов с плавающей точкой в [KNN search](https://manual.manticoresearch.com/Searching/KNN#KNN-vector-search)
* 🆕 [v10.2.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.0) [ Issue #3252](https://github.com/manticoresoftware/manticoresearch/issues/3252) [log_level](https://manual.manticoresearch.com/Server_settings/Setting_variables_online#SET) теперь также регулирует уровень детализации логирования Buddy

### Исправления ошибок
* 🪲 [v13.2.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.3) [ PR #3556](https://github.com/manticoresoftware/manticoresearch/pull/3556) Исправлен разбор опции "oversampling" в JSON-запросах
* 🪲 [v13.2.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.2)  Исправлена ошибка записи логов при сбоях на Linux и FreeBSD путём удаления использования Boost stacktrace
* 🪲 [v13.2.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.1) [ Issue #3298](https://github.com/manticoresoftware/manticoresearch/issues/3298) Исправлена ошибка записи логов при сбоях при запуске внутри контейнера
* 🪲 [v12.0.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/12.0.2)  Улучшена точность статистики по таблицам за счёт подсчёта в микросекундах
* 🪲 [v12.0.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/12.0.1) [ PR #3522](https://github.com/manticoresoftware/manticoresearch/pull/3522) Исправлена ошибка сбоя при фасетировании по MVA в объединённом запросе
* 🪲 [v11.0.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.0.3) [ Issue #3502](https://github.com/manticoresoftware/manticoresearch/issues/3502) Исправлена ошибка сбоя, связанная с квантизацией векторного поиска
* 🪲 [v11.0.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.0.2) [ Issue #3493](https://github.com/manticoresoftware/manticoresearch/issues/3493) Изменено отображение команды `SHOW THREADS` для вывода использования CPU в целочисленном формате
* 🪲 [v11.0.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.0.1)  Исправлены пути для столбцовых и вторичных библиотек
* 🪲 [v10.2.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.7)  Добавлена поддержка MCL 5.0.5, включая исправление имени файла библиотеки эмбеддингов
* 🪲 [v10.2.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.6) [ Issue #3469](https://github.com/manticoresoftware/manticoresearch/issues/3469) Внесено ещё одно исправление, связанное с issue #3469
* 🪲 [v10.2.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.4) [ Issue #3469](https://github.com/manticoresoftware/manticoresearch/issues/3469) Исправлена проблема, при которой HTTP-запросы с булевыми условиями возвращали пустые результаты
* 🪲 [v10.2.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.5)  Изменено имя файла библиотеки эмбеддингов по умолчанию и добавлены проверки поля 'from' в KNN-векторах
* 🪲 [v10.2.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.3) [ PR #3464](https://github.com/manticoresoftware/manticoresearch/pull/3464) Обновлен Buddy до версии 3.30.2, включающей [PR #565 по использованию памяти и логированию ошибок](https://github.com/manticoresoftware/manticoresearch-buddy/pull/565)
* 🪲 [v10.2.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.2) [ Issue #3470](https://github.com/manticoresoftware/manticoresearch/pull/3470) Исправлены фильтры строк JSON, фильтры null и проблемы с сортировкой в JOIN-запросах
* 🪲 [v10.2.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.1)  Исправлена ошибка в `dist/test_kit_docker_build.sh`, из-за которой отсутствовал коммит Buddy
* 🪲 [v10.1.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.4)  Исправлена ошибка сбоя при группировке по MVA в объединённом запросе
* 🪲 [v10.1.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.3) [ Issue #3434](https://github.com/manticoresoftware/manticoresearch/issues/3434) Исправлена ошибка фильтрации пустых строк
* 🪲 [v10.1.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.2) [ PR #3452](https://github.com/manticoresoftware/manticoresearch/pull/3452) Обновлен Buddy до версии 3.29.7, что решает [Buddy #507 - ошибка при мульти-запросах с нечетким поиском](https://github.com/manticoresoftware/manticoresearch-buddy/issues/507) и [Buddy #561 - исправление для metrics rate](https://github.com/manticoresoftware/manticoresearch-buddy/pull/561), требуется для [Helm Release 10.1.0](https://github.com/manticoresoftware/manticoresearch-helm/releases/tag/manticoresearch-10.1.0)
* 🪲 [v10.1.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.1)  Обновлен Buddy до версии 3.29.4, решающей [#3388 - "Неопределённый ключ массива 'Field'"](https://github.com/manticoresoftware/manticoresearch/issues/3388) и [Buddy #547 - layouts='ru' может не работать](https://github.com/manticoresoftware/manticoresearch-buddy/issues/547)

## Версия 10.1.0
**Выпущено**: 9 июня 2025 года

Эта версия представляет собой обновление с новыми функциями, одним критическим изменением и многочисленными улучшениями стабильности и исправлениями ошибок. Изменения сосредоточены на расширении возможностей мониторинга, улучшении функционала поиска и устранении различных критических проблем, влияющих на стабильность и производительность системы.

**Начиная с версии 10.1.0, поддержка CentOS 7 больше не предоставляется. Пользователям рекомендуется перейти на поддерживаемую операционную систему.**

### Критические изменения
* ⚠️ [v10.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.0.0) [ Issue #540](https://github.com/manticoresoftware/manticoresearch-buddy/issues/540) КРИТИЧЕСКОЕ ИЗМЕНЕНИЕ: установлено значение `layouts=''` по умолчанию для [нечеткого поиска](Searching/Spell_correction.md#Fuzzy-Search)

### Новые функции и улучшения
* 🆕 [v10.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.0) [ Issue #537](https://github.com/manticoresoftware/manticoresearch-buddy/issues/537) Добавлен встроенный [Prometheus exporter](Node_info_and_management/Node_status.md#Prometheus-Exporter)
* 🆕 [v9.8.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.8.0) [ Issue #3409](https://github.com/manticoresoftware/manticoresearch/issues/3409) Добавлена команда [ALTER TABLE tbl REBUILD KNN](Updating_table_schema_and_settings.md#Rebuilding-a-KNN-index)
* 🆕 [v9.7.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.7.0) [ Issue #1778](https://github.com/manticoresoftware/manticoresearch/issues/1778) Добавлено автоматическое создание embeddings (пока не объявляем официально, так как код в основной ветке, но требует дополнительного тестирования)
* 🆕 [v9.6.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.6.0) Повышена версия API KNN для поддержки авто-embeddings
* 🆕 [v9.5.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.0) [ Issue #1894](https://github.com/manticoresoftware/manticoresearch/issues/1894) Улучшено восстановление кластера: периодическое сохранение `seqno` для более быстрого перезапуска узла после сбоя
* 🆕 [v9.4.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.4.0) [ Issue #2400](https://github.com/manticoresoftware/manticoresearch/issues/2400) Добавлена поддержка последних версий [Logstash](Integration/Logstash.md#Integration-with-Logstash) и Beats

### Исправления ошибок
* 🪲 [v10.0.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.0.1) Исправлена обработка форм слов: пользовательские формы теперь имеют приоритет над авто-сгенерированными; добавлены тесты к тесту 22
* 🪲 [v9.8.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.8.2) Исправление: снова обновлен файл deps.txt для включения исправлений упаковки в MCL, связанных с библиотекой embeddings
* 🪲 [v9.8.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.8.1) Исправление: обновлен deps.txt с исправлениями упаковки для MCL и библиотеки embeddings
* 🪲 [v9.7.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.7.3) [ Issue #3306](https://github.com/manticoresoftware/manticoresearch/issues/3306) Исправлен сбой с сигналом 11 во время индексирования
* 🪲 [v9.7.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.7.2) [ Issue #3109](https://github.com/manticoresoftware/manticoresearch/issues/3109) Исправлена ошибка, из-за которой несуществующие `@@variables` всегда возвращали 0
* 🪲 [v9.7.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.7.1) [ Issue #3377](https://github.com/manticoresoftware/manticoresearch/issues/3377) Исправлен сбой, связанный с [remove_repeats()](Functions/Searching_and_ranking_functions.md#REMOVE_REPEATS%28%29)
* 🪲 [v9.6.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.6.3) [ PR #3411](https://github.com/manticoresoftware/manticoresearch/pull/3411) Исправление: использование динамического определения версий для телеметрических метрик
* 🪲 [v9.6.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.6.2) Исправление: небольшая поправка в выводе [SHOW VERSION](Node_info_and_management/SHOW_VERSION.md#SHOW-VERSION)
* 🪲 [v9.6.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.6.1) Исправление: сбой при создании таблицы с атрибутом KNN без модели
* 🪲 [v9.5.16](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.16) [ Issue #3342](https://github.com/manticoresoftware/manticoresearch/issues/3342) Исправлена ошибка, из-за которой `SELECT ... FUZZY=0` не всегда отключал fuzzy поиск
* 🪲 [v9.5.15](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.15) [ PR #3397](https://github.com/manticoresoftware/manticoresearch/pull/3397) Добавлена поддержка [MCL](https://github.com/manticoresoftware/columnar) 4.2.2; исправлены ошибки с более старыми форматами хранения
* 🪲 [v9.5.14](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.14) [ Issue #3392](https://github.com/manticoresoftware/manticoresearch/issues/3392) Исправлена неправильная обработка строк в HTTP JSON ответах
* 🪲 [v9.5.13](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.13) [ Issue #3356](https://github.com/manticoresoftware/manticoresearch/issues/3356) Исправлен сбой в сложном случае полнотекстового запроса (common-sub-term)
* 🪲 [v9.5.12](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.12) Исправлена опечатка в сообщении об ошибке авто-сброса дискового чанка
* 🪲 [v9.5.11](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.11) [ Issue #3195](https://github.com/manticoresoftware/manticoresearch/issues/3195) Улучшен [автоматический сброс дискового чанка](Securing_and_compacting_a_table/Flushing_RAM_chunk_to_disk.md#Flushing-RAM-chunk-to-disk): пропуск сохранения при запущенной оптимизации
* 🪲 [v9.5.10](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.10) [ Issue #3313](https://github.com/manticoresoftware/manticoresearch/issues/3313) Исправлена проверка дубликатов ID для всех дисковых чанков в RT таблице с использованием [indextool](Miscellaneous_tools.md#indextool)
* 🪲 [v9.5.9](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.9) [ Issue #3132](https://github.com/manticoresoftware/manticoresearch/issues/3132) Добавлена поддержка сортировки `_random` в JSON API
* 🪲 [v9.5.8](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.8) [ Issue #3382](https://github.com/manticoresoftware/manticoresearch/issues/3382) Исправлена ошибка: невозможно было использовать uint64 ID документа через JSON HTTP API
* 🪲 [v9.5.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.7) [ Issue #3385](https://github.com/manticoresoftware/manticoresearch/issues/3385) Исправлены некорректные результаты при фильтрации по `id != value`
* 🪲 [v9.5.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.6) [ PR #538](https://github.com/manticoresoftware/manticoresearch-buddy/pull/538) Исправлена критическая ошибка с fuzzy совпадением в некоторых случаях
* 🪲 [v9.5.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.5) [ Issue #3199](https://github.com/manticoresoftware/manticoresearch/issues/3199) Исправлено декодирование пробелов в параметрах HTTP-запроса Buddy (например, `%20` и `+`)
* 🪲 [v9.5.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.4) [ Issue #3133](https://github.com/manticoresoftware/manticoresearch/issues/3133) Исправлена некорректная сортировка по `json.field` в поиске с фасетами
* 🪲 [v9.5.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.3) [ Issue #3091](https://github.com/manticoresoftware/manticoresearch/issues/3091) Исправлены непоследовательные результаты поиска для разделителей в SQL и JSON API
* 🪲 [v9.5.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.2) [ Issue #2819](https://github.com/manticoresoftware/manticoresearch/issues/2819) Улучшена производительность: заменён `DELETE FROM` на `TRUNCATE` для распределённых таблиц
* 🪲 [v9.5.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.1) [ Issue #3080](https://github.com/manticoresoftware/manticoresearch/issues/3080) Исправлен сбой при фильтрации алиасного `geodist()` с JSON-атрибутами

## Версия 9.3.2
Выпущена: 2 мая 2025 года

В этом выпуске включено множество исправлений ошибок и улучшений стабильности, улучшено отслеживание использования таблиц, а также усовершенствования управления памятью и ресурсами.

❤️ Особая благодарность [@cho-m](https://github.com/cho-m) за исправление совместимости сборки с Boost 1.88.0 и [@benwills](https://github.com/benwills) за улучшение документации по `stored_only_fields`.

* 🪲 [v9.3.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.3.2)  Исправлена проблема, при которой колонка "Show Threads" показывала активность CPU в виде числа с плавающей точкой вместо строки; также исправлена ошибка разбора набора результатов PyMySQL из-за неправильного типа данных.
* 🪲 [v9.3.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.3.1) [ Issue #3343](https://github.com/manticoresoftware/manticoresearch/issues/3343) Исправлено оставление файлов `tmp.spidx` при прерывании процесса оптимизации.
* 🆕 [v9.3.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.3.0) [ PR #3337](https://github.com/manticoresoftware/manticoresearch/pull/3337) Добавлен счётчик команд на каждую таблицу и подробная статистика использования таблиц.
* 🪲 [v9.2.39](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.39) [ Issue #3236](https://github.com/manticoresoftware/manticoresearch/issues/3236) Исправление: предотвращён урон таблице удалением сложных обновлений чанков. Использование функций ожидания внутри последовательного рабочего процесса ломало последовательную обработку, что могло повредить таблицы.
Переосмыслен автосброс. Удалена внешняя очередь опроса для избежания ненужных блокировок таблиц. Добавлено условие "малой таблицы": если количество документов ниже 'предела малой таблицы' (8192) и не используется вторичный индекс (SI), сброс пропускается.

* 🪲 [v9.2.38](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.38)  Исправление: пропустить создание Вторичного Индекса (SI) для фильтров с использованием `ALL`/`ANY` для списков строк, без влияния на JSON-атрибуты.
* 🪲 [v9.2.37](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.37) [ Issue #2898](https://github.com/manticoresoftware/manticoresearch/issues/2898) Добавлена поддержка обратных кавычек для системных таблиц.
* 🪲 [v9.2.36](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.36)  Исправление: использование плейсхолдера для операций кластера в старом коде. В парсере теперь чётко разделяются поля для имён таблиц и кластеров.
* 🪲 [v9.2.35](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.35)  Исправление: сбой при снятии кавычек с одиночной `'`.
* 🪲 [v9.2.34](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.34) [ Issue #3090](https://github.com/manticoresoftware/manticoresearch/issues/3090) Исправление: обработка больших идентификаторов документов (раньше могла не находить их).
* 🪲 [v9.2.33](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.33)  Исправление: использование беззнаковых целых для размеров битовых векторов.
* 🪲 [v9.2.32](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.32)  Исправление: снижение пикового использования памяти во время слияния. Поиск соответствий docid-to-rowid теперь использует 12 байт на документ вместо 16. Пример: 24 ГБ ОЗУ для 2 миллиардов документов вместо 36 ГБ.
* 🪲 [v9.2.31](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.31) [ Issue #3238](https://github.com/manticoresoftware/manticoresearch/issues/3238) Исправление: некорректное значение `COUNT(*)` в больших таблицах реального времени.
* 🪲 [v9.2.30](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.30)  Исправление: неопределённое поведение при занулении строковых атрибутов.
* 🪲 [v9.2.29](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.29)  Незначительное исправление: улучшено предупреждающее сообщение.
* 🪲 [v9.2.28](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.28) [ Issue #3290](https://github.com/manticoresoftware/manticoresearch/issues/3290) Улучшение: улучшена команда `indextool --buildidf`
* 🪲 [v9.2.27](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.27) [ Issue #3032](https://github.com/manticoresoftware/manticoresearch/issues/3032) С интеграцией Kafka теперь можно создавать источник для конкретного раздела Kafka.
* 🪲 [v9.2.26](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.26) [ Issue #3301](https://github.com/manticoresoftware/manticoresearch/issues/3301) Исправление: `ORDER BY` и `WHERE` на `id` могли вызывать ошибки Out Of Memory.
* 🪲 [v9.2.25](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.25) [ Issue #3171](https://github.com/manticoresoftware/manticoresearch/issues/3171) Исправление: сбой из-за ошибки сегментации при использовании grouper с несколькими JSON-атрибутами в RT-таблице с несколькими дисковыми чанками
* 🪲 [v9.2.24](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.24) [ Issue #3246](https://github.com/manticoresoftware/manticoresearch/issues/3246) Исправление: запросы `WHERE string ANY(...)` не работали после сброса RAM-чанка.
* 🪲 [v9.2.23](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.23) [ PR #518](https://github.com/manticoresoftware/manticoresearch-buddy/pull/518) Незначительные улучшения синтаксиса авто-шардинга.
* 🪲 [v9.2.22](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.22) [ Issue #2763](https://github.com/manticoresoftware/manticoresearch/issues/2763) Исправлено: глобальный idf-файл не загружался при использовании `ALTER TABLE`.
* 🪲 [v9.2.21](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.21)  Исправлено: глобальные idf-файлы могут быть большими. Теперь мы освобождаем таблицы раньше, чтобы избежать удержания ненужных ресурсов.
* 🪲 [v9.2.20](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.20) [ PR #3277](https://github.com/manticoresoftware/manticoresearch/pull/3277) Улучшение: более корректная проверка опций шардинга.

* 🪲 [v9.2.19](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.19) [ PR #3275](https://github.com/manticoresoftware/manticoresearch/pull/3275) Исправлено: совместимость сборки с Boost 1.88.0.
* 🪲 [v9.2.18](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.18) [ Issue #3228](https://github.com/manticoresoftware/manticoresearch/issues/3228) Исправлено: сбой при создании распределённой таблицы (проблема с неверным указателем).

* 🪲 [v9.2.17](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.17) [ PR #3272](https://github.com/manticoresoftware/manticoresearch/pull/3272) Исправлено: проблема с многострочным fuzzy `FACET`.
* 🪲 [v9.2.16](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.16) [ Issue #3063](https://github.com/manticoresoftware/manticoresearch/issues/3063) Исправлено: ошибка при вычислении расстояния с использованием функции `GEODIST`.
* 🪲 [v9.2.15](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.15) [ Issue #3027](https://github.com/manticoresoftware/manticoresearch/issues/3027) Небольшое улучшение: поддержка формата фильтра Elastic `query_string`.

## Version 9.2.14
Выпущено: 28 марта 2025

### Небольшие изменения
* [Commit ](https://github.com/manticoresoftware/manticoresearch/commit/fe9473dc3f4fcfa0eaae748b538fe853f4ce078b) Реализован флаг `--mockstack` для расчёта требований к стеку рекурсивных операций. Новый режим `--mockstack` анализирует и выводит необходимые размеры стеков для рекурсивной оценки выражений, операций сопоставления шаблонов, обработки фильтров. Рассчитанные требования к стеку выводятся в консоль для отладки и оптимизации.
* [Issue #3058](https://github.com/manticoresoftware/manticoresearch/pull/3058) Включена опция [boolean_simplify](Searching/Options.md#boolean_simplify) по умолчанию.
* [Issue #3172](https://github.com/manticoresoftware/manticoresearch/issues/3172) Добавлена новая опция конфигурации: `searchd.kibana_version_string`, которая может быть полезна при использовании Manticore с определёнными версиями Kibana или OpenSearch Dashboards, ожидающими конкретную версию Elasticsearch.
* [Issue #3211](https://github.com/manticoresoftware/manticoresearch/issues/3211) Исправлена работа [CALL SUGGEST](Searching/Spell_correction.md#CALL-QSUGGEST,-CALL-SUGGEST) с двухсимвольными словами.
* [Issue #490](https://github.com/manticoresoftware/manticoresearch-buddy/issues/490) Улучшен [fuzzy search](Searching/Spell_correction.md#Fuzzy-Search): ранее иногда не находились "defghi" при поиске "def ghi", если существовал другой подходящий документ.
* ⚠️ BREAKING [Issue #3165](https://github.com/manticoresoftware/manticoresearch/issues/3165) Изменено `_id` на `id` в некоторых HTTP JSON ответах для согласованности. Обязательно обновите ваше приложение.
* ⚠️ BREAKING [Issue #3186](https://github.com/manticoresoftware/manticoresearch/issues/3186) Добавлена проверка `server_id` при присоединении к кластеру, чтобы обеспечивать уникальность ID узлов. Операция `JOIN CLUSTER` теперь может завершиться ошибкой с сообщением о дублирующемся [server_id](Server_settings/Searchd.md#server_id), если узел с таким же `server_id` уже существует в кластере. Для решения проблемы убедитесь, что у каждого узла в кластере репликации уникальный [server_id](Server_settings/Searchd.md#server_id). Вы можете изменить дефолтный [server_id](Server_settings/Searchd.md#server_id) в разделе "searchd" вашего конфигурационного файла на уникальное значение перед попыткой присоединения к кластеру. Это изменение обновляет протокол репликации. Если вы используете кластер репликации, вам необходимо:
  - Сначала корректно остановить все ваши узлы
  - Затем запустить последний остановленный узел с параметром `--new-cluster`, используя инструмент `manticore_new_cluster` в Linux.
  - Подробнее о [перезапуске кластера](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster).

### Исправления ошибок
* [Commit 6fda](https://github.com/manticoresoftware/manticoresearch/commit/6fdad3923dd6953f4b781943eed5ec5c28b7f808) Исправлен сбой из-за потери планировщика после ожидания; теперь специфичные планировщики, такие как `serializer`, корректно восстанавливаются.
* [Commit c333](https://github.com/manticoresoftware/manticoresearch/commit/c333b277d4a504de5fe298e9ab570a9f9ea31e0a) Исправлена ошибка, когда веса из правой присоединённой таблицы не могли использоваться в выражении `ORDER BY`.
* [Issue #2644](https://github.com/manticoresoftware/manticoresearch/issues/2644) gcc 14.2.0: исправлена ошибка вызова `lower_bound` с `const knn::DocDist_t*&`. ❤️ Спасибо [@Azq2](https://github.com/Azq2) за PR.
* [Issue #3018](https://github.com/manticoresoftware/manticoresearch/issues/3018) Исправлена проблема обработки имён таблиц с заглавными буквами при вставках с авто-схемой.
* [Issue #3119](https://github.com/manticoresoftware/manticoresearch/issues/3119) Исправлен сбой при декодировании некорректного base64 входа.
* [Issue #3121](https://github.com/manticoresoftware/manticoresearch/issues/3121) Исправлены две связанные проблемы с KNN индексами при `ALTER`: вещественные векторы теперь сохраняют исходные размеры, и KNN индексы корректно генерируются.
* [Issue #3123](https://github.com/manticoresoftware/manticoresearch/issues/3123) Исправлен сбой при построении вторичного индекса на пустом JSON-столбце.
* [Issue #3138](https://github.com/manticoresoftware/manticoresearch/issues/3138) Исправлен сбой, вызванный дублирующимися записями.
* [Issue #3151](https://github.com/manticoresoftware/manticoresearch/issues/3151) Исправлено: опция `fuzzy=1` не могла использоваться совместно с `ranker` или `field_weights`.
* [Issue #3163](https://github.com/manticoresoftware/manticoresearch/issues/3163) Исправлена ошибка, из-за которой `SET GLOBAL timezone` не имел эффекта.
* [Issue #3181](https://github.com/manticoresoftware/manticoresearch/issues/3181) Исправлена проблема, из-за которой значения текстовых полей могли теряться при использовании ID больше 2^63.
* [Issue #3189](https://github.com/manticoresoftware/manticoresearch/issues/3189) Исправлено: операторы `UPDATE` теперь корректно учитывают настройку `query_log_min_msec`.
* [Issue #3247](https://github.com/manticoresoftware/manticoresearch/issues/3247) Исправлено состояние гонки при сохранении real-time дисковых чанков, которое могло вызывать сбой `JOIN CLUSTER`.


## Version 7.4.6
Выпущена: 28 февраля 2025 г.

### Основные изменения
* [Issue #832](https://github.com/manticoresoftware/manticoresearch/issues/832) Интеграция с [Kibana](Integration/Kibana.md) для более удобной и эффективной визуализации данных.

### Небольшие изменения
* [Issue #1727](https://github.com/manticoresoftware/manticoresearch/issues/1727) Исправлены различия в точности чисел с плавающей запятой между arm64 и x86_64.
* [Issue #2995](https://github.com/manticoresoftware/manticoresearch/issues/2995) Реализованы оптимизации производительности для обработки join партиями.
* [Issue #3039](https://github.com/manticoresoftware/manticoresearch/issues/3039) Реализованы оптимизации производительности для EstimateValues в гистограммах.
* [Issue #3099](https://github.com/manticoresoftware/manticoresearch/issues/3099) Добавлена поддержка Boost 1.87.0. ❤️ Спасибо, [@cho-m](https://github.com/cho-m) за PR.
* [Issue #77](https://github.com/manticoresoftware/columnar/issues/77) Оптимизировано повторное использование блоков данных при создании фильтров с несколькими значениями; добавлены min/max к метаданным атрибутов; реализована предварительная фильтрация значений фильтра на основе min/max.

### Исправления ошибок
* [Commit 73ac](https://github.com/manticoresoftware/manticoresearch/commit/73ac22f358a7a0e734b332c3943c86e6294c06c3) Исправлена обработка выражений в joined-запросах при использовании атрибутов из обеих таблиц; исправлена опция index_weights для правой таблицы.
* [Issue #2915](https://github.com/manticoresoftware/manticoresearch/issues/2915) Использование `avg()` в запросе `SELECT ... JOIN` могло приводить к некорректным результатам; теперь исправлено.
* [Issue #2996](https://github.com/manticoresoftware/manticoresearch/issues/2996) Исправлен неверный набор результатов, вызванный неявным cutoff при включенном join batching.
* [Issue #3031](https://github.com/manticoresoftware/manticoresearch/issues/3031) Исправлен сбой демона при завершении работы, если в процессе был активный merge чанков.
* [Issue #3037](http://github.com/manticoresoftware/manticoresearch/issues/3037) Исправлена ошибка, из-за которой `IN(...)` мог выдавать некорректные результаты.
* [Issue #3038](https://github.com/manticoresoftware/manticoresearch/issues/3038) Установка `max_iops` / `max_iosize` в версии 7.0.0 могла ухудшать производительность индексирования; теперь исправлено.
* [Issue #3042](https://github.com/manticoresoftware/manticoresearch/issues/3042) Исправлена утечка памяти в кэше join-запросов.
* [Issue #3052](https://github.com/manticoresoftware/manticoresearch/issues/3052) Исправлена обработка параметров запроса в joined JSON-запросах.
* [Issue #3054](https://github.com/manticoresoftware/manticoresearch/issues/3054) Исправлена проблема с командой ATTACH TABLE.
* [Issue #3079](https://github.com/manticoresoftware/manticoresearch/issues/3079) Исправлены несоответствия в сообщениях об ошибках.
* [Issue #3087](https://github.com/manticoresoftware/manticoresearch/issues/3087) Установка `diskchunk_flush_write_timeout=-1` для таблицы не отключала сброс индекса; теперь исправлено.
* [Issue #3088](https://github.com/manticoresoftware/manticoresearch/issues/3088) Устранены дубликаты после массовой замены больших ID.
* [Issue #3126](https://github.com/manticoresoftware/manticoresearch/issues/3126) Исправлен сбой демона, вызванный полнотекстовым запросом с одним оператором `NOT` и ранжировщиком выражений.
* [Issue #3128](https://github.com/manticoresoftware/manticoresearch/pull/3128) Исправлена потенциальная уязвимость в библиотеке CJSON. ❤️ Спасибо, [@tabudz](https://github.com/tabudz) за PR.

## Version 7.0.0
Выпущена: 30 января 2025 г.

### Основные изменения
* [Issue #1497](https://github.com/manticoresoftware/manticoresearch/issues/1497) Добавлены новые функции [Нечеткого поиска](Searching/Spell_correction.md#Fuzzy-Search) и [Автозаполнения](Searching/Autocomplete.md#CALL-AUTOCOMPLETE) для более удобного поиска.
* [Issue #1500](https://github.com/manticoresoftware/manticoresearch/issues/1500) [Интеграция с Kafka](Integration/Kafka.md#Syncing-from-Kafka).
* [Issue #1928](https://github.com/manticoresoftware/manticoresearch/issues/1928) Введены [вторичные индексы для JSON](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#json_secondary_indexes).
* [Issue #2361](https://github.com/manticoresoftware/manticoresearch/issues/2361) Обновления и поиски во время обновлений больше не блокируются merge чанков.
* [Issue #2787](https://github.com/manticoresoftware/manticoresearch/issues/2787) Автоматический [сброс дисковых чанков](Server_settings/Searchd.md#diskchunk_flush_write_timeout) для RT таблиц для повышения производительности; теперь мы автоматически сбрасываем RAM-чанк в дисковый чанк, предотвращая проблемы с производительностью из-за отсутствия оптимизаций в RAM чанках, которые иногда могли приводить к нестабильности в зависимости от размера чанка.
* [Issue #2811](https://github.com/manticoresoftware/manticoresearch/issues/2811) Опция [Scroll](Searching/Pagination.md#Scroll-Search-Option) для удобной постраничной навигации.
* [Issue #931](https://github.com/manticoresoftware/manticoresearch/issues/931) Интеграция с [Jieba](https://github.com/fxsjy/jieba) для улучшенной [токенизации китайского языка](Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md).

### Небольшие изменения
* ⚠️ BREAKING [Issue #1111](https://github.com/manticoresoftware/manticoresearch/issues/1111) Исправлена поддержка `global_idf` в RT таблицах. Требуется пересоздание таблицы.
* ⚠️ BREAKING [Issue #2103](https://github.com/manticoresoftware/manticoresearch/issues/2103) Удалены тайские символы из внутреннего набора символов `cjk`. Обновите определения своих наборов символов: если у вас есть `cjk,non_cjk` и для вас важны тайские символы, измените его на `cjk,thai,non_cjk`, или `cont,non_cjk`, где `cont` — это новое обозначение для всех языков с непрерывным письмом (т.е. `cjk` + `thai`). Измените существующие таблицы, используя [ALTER TABLE](Updating_table_schema_and_settings.md#Updating-table-FT-settings-in-RT-mode).
* ⚠️ BREAKING [Issue #2468](https://github.com/manticoresoftware/manticoresearch/issues/2468) [CALL SUGGEST / QSUGGEST](Searching/Spell_correction.md#CALL-QSUGGEST,-CALL-SUGGEST) теперь совместим с распределёнными таблицами. Это увеличивает версию протокола мастер/агент. Если вы запускаете Manticore Search в распределённой среде с несколькими инстансами, обязательно сначала обновите агенты, затем мастера.
* ⚠️ BREAKING [Issue #2889](https://github.com/manticoresoftware/manticoresearch/issues/2889) Изменено имя колонки с `Name` на `Variable name` для PQ [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META-for-PQ-tables).
* ⚠️ BREAKING [Issue #879](https://github.com/manticoresoftware/manticoresearch/issues/879) Введена [поколоночная бинарная логика](Logging/Binary_logging.md#Per-table-binary-logging-configuration) с новыми опциями: [binlog_common](Logging/Binary_logging.md#Binary-logging-strategies), [binlog](Logging/Binary_logging.md#Per-table-binary-logging-configuration) для `create table` / `alter table`. Перед обновлением до новой версии необходимо корректно завершить работу инстанса Manticore.
* ⚠️ BREAKING [Issue #1789](https://github.com/manticoresoftware/manticoresearch/issues/1789) Исправлено некорректное сообщение об ошибке, когда узел присоединяется к кластеру с неправильной версией протокола репликации. Это изменение обновляет протокол репликации. Если вы используете кластер репликации, необходимо:
  - Сначала корректно остановить все узлы
  - Затем запустить последний остановленный узел с `--new-cluster`, используя утилиту `manticore_new_cluster` в Linux.
  - Подробнее о [перезапуске кластера](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster).
* ⚠️ BREAKING [Issue #2308](https://github.com/manticoresoftware/manticoresearch/issues/2308) Добавлена поддержка нескольких таблиц в [`ALTER CLUSTER ADD` и `DROP`](Creating_a_cluster/Setting_up_replication/Adding_and_removing_a_table_from_a_replication_cluster.md#Adding-and-removing-a-table-from-a-replication-cluster). Это изменение также влияет на протокол репликации. Обратитесь к предыдущему разделу для рекомендаций по обновлению.
* [Issue #2997](https://github.com/manticoresoftware/manticoresearch/issues/2997) Исправлена проблема с dlopen на Macos.
* [Commit 4954](https://github.com/manticoresoftware/manticoresearch/commit/4954b5de7341a29902a9b8fbb9a040f7942c77c4) Изменено значение по умолчанию для cutoff в OPTIMIZE TABLE на таблицах с KNN индексами для улучшения производительности поиска.
* [Commit cfc8](https://github.com/manticoresoftware/manticoresearch/commit/cfc87ecb6e33a8163c2235243b6b40e699dbf526) Добавлена поддержка `COUNT(DISTINCT)` для `ORDER BY` в `FACET` и `GROUP BY`.
* [Issue #1103](https://github.com/manticoresoftware/manticoresearch/issues/1103) Улучшена ясность в [логировании](Logging/Server_logging.md#Server-logging) слияния чанков.
* [Issue #1130](https://github.com/manticoresoftware/manticoresearch/issues/1130) Добавлена поддержка для [DBeaver](Integration/DBeaver.md).
* [Issue #1546](https://github.com/manticoresoftware/manticoresearch/issues/1546) Реализованы вторичные индексы для функций [POLY2D()](Functions/Geo_spatial_functions.md#POLY2D%28%29)/[GEOPOLY2D()](Functions/Geo_spatial_functions.md#GEOPOLY2D%28%29).
* [Issue #1630](https://github.com/manticoresoftware/manticoresearch/issues/1630) HTTP-запросы теперь поддерживают `Content-Encoding: gzip`.
* [Issue #1831](https://github.com/manticoresoftware/manticoresearch/issues/1831) Добавлена команда `SHOW LOCKS`.
* [Issue #2187](https://github.com/manticoresoftware/manticoresearch/issues/2197) Разрешен обход ограничения [searchd.max_connections](Server_settings/Searchd.md#max_connections) для запросов Buddy к демону.
* [Issue #2208](https://github.com/manticoresoftware/manticoresearch/issues/2208) Добавлена поддержка объединения таблиц через JSON HTTP интерфейс.
* [Issue #2235](https://github.com/manticoresoftware/manticoresearch/issues/2235) Логирование успешно обработанных запросов через Buddy в их исходной форме.
* [Issue #2249](https://github.com/manticoresoftware/manticoresearch/issues/2249) Добавлен специальный режим запуска `mysqldump` для реплицированных таблиц.
* [Issue #2268](https://github.com/manticoresoftware/manticoresearch/issues/2268) Улучшено переименование внешних файлов при копировании в операциях `CREATE TABLE` и `ALTER TABLE`.
* [Issue #2402](https://github.com/manticoresoftware/manticoresearch/issues/2402) Значение по умолчанию для [searchd.max_packet_size](Server_settings/Searchd.md#max_packet_size) обновлено на 128MB.
* [Issue #2419](https://github.com/manticoresoftware/manticoresearch/issues/2419) Добавлена поддержка [IDF boost modifier](Searching/Full_text_matching/Operators.md#IDF-boost-modifier) в JSON ["match"](Searching/Full_text_matching/Basic_usage.md#match).
* [Issue #2430](https://github.com/manticoresoftware/manticoresearch/issues/2430) Усилена синхронизация записи [binlog](Logging/Binary_logging.md#Binary-logging) для предотвращения ошибок.
* [Issue #2458](https://github.com/manticoresoftware/manticoresearch/issues/2458) Добавлена поддержка zlib в Windows-пакетах.
* [Issue #2479](https://github.com/manticoresoftware/manticoresearch/issues/2479) Добавлена поддержка команды SHOW TABLE INDEXES.
* [Issue #2485](https://github.com/manticoresoftware/manticoresearch/issues/2485) Установлены метаданные сессии для ответов Buddy.
* [Issue #2490](https://github.com/manticoresoftware/manticoresearch/issues/2490) Разрешение в миллисекундах для агрегаций на совместимых конечных точках.
* [Issue #2500](https://github.com/manticoresoftware/manticoresearch/issues/2500) Изменены сообщения об ошибках для операций кластера при сбое запуска репликации.
* [Issue #2584](https://github.com/manticoresoftware/manticoresearch/pull/2584) Новые [метрики производительности в SHOW STATUS](Node_info_and_management/Node_status.md#Query-Time-Statistics): min/max/avg/95-й/99-й перцентиль по типу запроса за последние 1, 5 и 15 минут.
* [Issue #2639](https://github.com/manticoresoftware/manticoresearch/pull/2639) Заменены все вхождения `index` на `table` в запросах и ответах.
* [Issue #2643](https://github.com/manticoresoftware/manticoresearch/issues/2643) Добавлена колонка `distinct` в результаты агрегаций HTTP `/sql` эндпоинта.
* [Issue #268](https://github.com/manticoresoftware/manticoresearch-buddy/issues/268) Реализовано автодетектирование типов данных, импортируемых из Elasticsearch.
* [Issue #2744](https://github.com/manticoresoftware/manticoresearch/issues/2744) Добавлена поддержка сортировки (collation) для выражений сравнения строковых JSON-полей.
* [Issue #2752](https://github.com/manticoresoftware/manticoresearch/issues/2752) Добавлена поддержка выражения `uuid_short` в списке select.
* [Issue #2783](https://github.com/manticoresoftware/manticoresearch/issues/2783) Manticore Search теперь запускает Buddy напрямую без оболочки `manticore-buddy`.
* [Issue #2785](https://github.com/manticoresoftware/manticoresearch/issues/2785) Различные сообщения об ошибках для отсутствующих таблиц и таблиц, не поддерживающих операции вставки.
* [Issue #2789](https://github.com/manticoresoftware/manticoresearch/issues/2789) OpenSSL 3 теперь статически встроен в `searchd`.
* [Issue #2790](https://github.com/manticoresoftware/manticoresearch/issues/2790) Добавлена команда `CALL uuid_short` для генерации последовательностей с несколькими значениями `uuid_short`.
* [Issue #2803](https://github.com/manticoresoftware/manticoresearch/issues/2803) Добавлены отдельные опции для правой таблицы в операции JOIN.
* [Issue #2810](https://github.com/manticoresoftware/manticoresearch/issues/2810) Улучшена производительность HTTP JSON агрегаций, сопоставимая с `GROUP BY` в SphinxQL.
* [Issue #2854](https://github.com/manticoresoftware/manticoresearch/issues/2854) Добавлена поддержка `fixed_interval` в запросах Kibana, связанных с датами.
* [Issue #2909](https://github.com/manticoresoftware/manticoresearch/issues/2909) Реализован пакетный режим для JOIN-запросов, который улучшает производительность некоторых JOIN-запросов в сотни или даже тысячи раз.
* [Issue #2937](https://github.com/manticoresoftware/manticoresearch/issues/2937) Включена возможность использования веса присоединенной таблицы в fullscan-запросах.
* [Issue #2953](https://github.com/manticoresoftware/manticoresearch/issues/2953) Исправлен логгинг для join-запросов.
* [Issue #337](https://github.com/manticoresoftware/manticoresearch-buddy/issues/337) Исключения Buddy скрыты из лога `searchd` в режиме без отладки.
* [Issue #2931](https://github.com/manticoresoftware/manticoresearch/issues/2931) Завершение работы демона с сообщением об ошибке, если пользователь указывает неправильные порты для слушателя репликации.

### Исправления ошибок
* [Commit 0c6b](https://github.com/manticoresoftware/manticoresearch/commit/0c6bdaf41ea3684e05952e1a8273893535b923f5) Исправлено: Неправильные результаты при выполнении JOIN-запросов с более чем 32 колонками.
* [Issue #2335](https://github.com/manticoresoftware/manticoresearch/issues/2335) Решена проблема с ошибкой соединения таблиц, когда в условии использовались два json-атрибута.
* [Issue #2338](https://github.com/manticoresoftware/manticoresearch/issues/2338) Исправлено некорректное значение total_relation в мультизапросах с [cutoff](Searching/Options.md#cutoff).
* [Issue #2366](https://github.com/manticoresoftware/manticoresearch/issues/2366) Исправлена фильтрация по `json.string` в правой таблице при [соединении таблиц](Searching/Joining.md).
* [Issue #2406](https://github.com/manticoresoftware/manticoresearch/issues/2406) Разрешено использовать `null` для всех значений в любых POST HTTP JSON эндпоинтах (insert/replace/bulk). В этом случае используется значение по умолчанию.
* [Issue #2418](https://github.com/manticoresoftware/manticoresearch/issues/2418) Оптимизировано потребление памяти за счет корректировки выделения сетевого буфера [max_packet_size](Server_settings/Searchd.md#max_packet_size) для начального тестирования сокета.
* [Issue #2420](https://github.com/manticoresoftware/manticoresearch/issues/2420) Исправлена вставка unsigned int в bigint атрибут через JSON-интерфейс.
* [Issue #2422](https://github.com/manticoresoftware/manticoresearch/issues/2422) Исправлена работа вторичных индексов с фильтрами исключения и включенным pseudo_sharding.
* [Issue #2423](https://github.com/manticoresoftware/manticoresearch/issues/2423) Исправлена ошибка в [manticore_new_cluster](Starting_the_server/Manually.md#searchd-command-line-options).
* [Issue #2448](https://github.com/manticoresoftware/manticoresearch/issues/2448) Исправлен сбой демона при ошибочном запросе `_update`.
* [Issue #2452](https://github.com/manticoresoftware/manticoresearch/issues/2452) Исправлено некорректное поведение гистограмм при фильтрах значений с исключениями.
* [Issue #55](https://github.com/manticoresoftware/columnar/issues/55) Исправлены [knn](Searching/KNN.md#KNN-vector-search) запросы к распределённым таблицам.
* [Issue #68](https://github.com/manticoresoftware/columnar/issues/68) Улучшена обработка фильтров исключения на кодировании таблиц в columnar accessor.
* [Commit 0eb1](https://github.com/manticoresoftware/manticoresearch/commit/0eb1579462013cb134bcaf74e06f6eeb0c2ecc4d) Исправлен разбор выражений, который не учитывал переопределенный `thread_stack`.
* [Commit c304](https://github.com/manticoresoftware/manticoresearch/commit/c3041fd1d3e14773f3c5aa635343915a71f5cb95) Исправлен сбой при клонировании columnar IN выражения.
* [Commit edad](https://github.com/manticoresoftware/columnar/commit/edadc694c68d6022bdd13134263667430a42cc1d) Исправлена проблема с инверсией в итераторе bitmap, которая приводила к сбою.
* [Коммит fc30](https://github.com/manticoresoftware/executor/commit/fc302ff1117b0b835a6f5a3c07057baf5fba14d9) Исправлена проблема, когда некоторые пакеты Manticore автоматически удалялись с помощью `unattended-upgrades`.
* [Задача #1019](https://github.com/manticoresoftware/manticoresearch/issues/1019) Улучшена обработка запросов из инструмента DbForge MySQL.
* [Задача #1107](https://github.com/manticoresoftware/manticoresearch/issues/1107) Исправлено экранирование специальных символов в `CREATE TABLE` и `ALTER TABLE`. ❤️ Спасибо, [@subnix](https://github.com/subnix), за PR.
* [Задача #116](https://github.com/manticoresoftware/manticoresearch-backup/issues/116#issuecomment-2216597206) Исправлена взаимная блокировка при обновлении blob-атрибута в замороженном индексе. Взаимная блокировка возникала из-за конфликтующих блокировок при попытке разморозить индекс. Это могло также вызывать сбой в manticore-backup.
* [Задача #1818](https://github.com/manticoresoftware/manticoresearch/issues/1818) `OPTIMIZE` теперь выдает ошибку, если таблица заморожена.
* [Задача #2001](https://github.com/manticoresoftware/manticoresearch/issues/2001) Разрешено использовать имена функций как имена столбцов.
* [Задача #2153](https://github.com/manticoresoftware/manticoresearch/issues/2153) Исправлен сбой демона при запросах настроек таблицы с неизвестным chunk диска.
* [Задача #2184](https://github.com/manticoresoftware/manticoresearch/issues/2184) Исправлена проблема, из-за которой `searchd` зависал при остановке после команд `FREEZE` и `FLUSH RAMCHUNK`.
* [Задача #2228](https://github.com/manticoresoftware/manticoresearch/issues/2228) Удалены токены, связанные с датой/временем (и regex), из зарезервированных слов.
* [Задача #2255](https://github.com/manticoresoftware/manticoresearch/issues/2255) Исправлен сбой при использовании `FACET` с более чем 5 полями сортировки.
* [Задача #2265](https://github.com/manticoresoftware/manticoresearch/issues/2265) Исправлен сбой восстановления `mysqldump` при включенной опции `index_field_lengths`.
* [Задача #2291](https://github.com/manticoresoftware/manticoresearch/issues/2291) Исправлен сбой при выполнении команды `ALTER TABLE`.
* [Задача #2333](https://github.com/manticoresoftware/manticoresearch/issues/2333) Исправлена DLL MySQL в пакете для Windows, чтобы индексатор работал корректно.
* [Задача #2393](https://github.com/manticoresoftware/manticoresearch/issues/2393) Исправлена ошибка компиляции GCC. ❤️ Спасибо, [@animetosho](https://github.com/animetosho), за PR.
* [Задача #2447](https://github.com/manticoresoftware/manticoresearch/issues/2447) Исправлена проблема экранирования в [_update](Data_creation_and_modification/Updating_documents/REPLACE.md?client=Elasticsearch-like+partial).
* [Задача #2460](https://github.com/manticoresoftware/manticoresearch/issues/2460) Исправлен сбой индексатора при объявлении нескольких атрибутов или полей с одним и тем же именем.
* [Задача #2467](https://github.com/manticoresoftware/manticoresearch/issues/2467) Исправлен сбой демона при некорректном преобразовании вложенных bool-запросов для связанных с поиском endpoint-ов "compat".
* [Задача #2493](https://github.com/manticoresoftware/manticoresearch/issues/2493) Исправлено расширение в фразах с модификаторами.
* [Задача #2535](https://github.com/manticoresoftware/manticoresearch/issues/2535) Исправлен сбой демона при использовании полнотекстовых операторов [ZONE](Searching/Full_text_matching/Operators.md#ZONE-limit-operator) или [ZONESPAN](Searching/Full_text_matching/Operators.md#ZONESPAN-limit-operator).
* [Задача #2552](https://github.com/manticoresoftware/manticoresearch/issues/2552) Исправлена генерация инфикса для plain и RT таблиц с словарём ключевых слов.
* [Задача #2571](https://github.com/manticoresoftware/manticoresearch/issues/2571) Исправлен ответ с ошибкой в запросе `FACET`; установлен порядок сортировки по умолчанию `DESC` для `FACET` с `COUNT(*)`.
* [Задача #2580](https://github.com/manticoresoftware/manticoresearch/issues/2580) Исправлен сбой демона при запуске на Windows.
* [Задача #2603](https://github.com/manticoresoftware/manticoresearch/issues/2603) Исправлено усечение запроса для HTTP endpoint-ов `/sql` и `/sql?mode=raw`; сделаны запросы с этих endpoint-ов согласованными без необходимости заголовка `query=`.
* [Задача #2623](https://github.com/manticoresoftware/manticoresearch/issues/2623) Исправлена проблема, при которой авто-схема создаёт таблицу, но при этом сразу же завершается с ошибкой.
* [Задача #2627](https://github.com/manticoresoftware/manticoresearch/issues/2627) Исправлена библиотека HNSW для поддержки загрузки нескольких KNN индексов.
* [Задача #2630](https://github.com/manticoresoftware/manticoresearch/issues/2630) Исправлена проблема заморозки при одновременном возникновении нескольких условий.
* [Задача #2645](https://github.com/manticoresoftware/manticoresearch/issues/2645) Исправлен сбой с фатальной ошибкой при использовании `OR` с поиском KNN.
* [Задача #2647](https://github.com/manticoresoftware/manticoresearch/issues/2647) Исправлена проблема, когда `indextool --mergeidf *.idf --out global.idf` удалял выходной файл после создания.
* [Задача #2658](https://github.com/manticoresoftware/manticoresearch/issues/2658) Исправлен сбой демона при подзапросе с `ORDER BY` строкой во внешнем запросе.
* [Задача #2686](https://github.com/manticoresoftware/manticoresearch/issues/2686) Исправлен сбой при обновлении атрибута float вместе с атрибутом string.
* [Задача #2704](https://github.com/manticoresoftware/manticoresearch/issues/2704) Исправлена проблема, из-за которой несколько стоп-слов от токенизаторов `lemmatize_xxx_all` увеличивали `hitpos` следующих токенов.
* [Задача #2708](https://github.com/manticoresoftware/manticoresearch/issues/2708) Исправлен сбой при `ALTER ... ADD COLUMN ... TEXT`.
* [Задача #2737](https://github.com/manticoresoftware/manticoresearch/issues/2737) Исправлена проблема, когда обновление blob-атрибута в замороженной таблице с хотя бы одним RAM chunk вызывает ожидание последующих запросов `SELECT` до разморозки таблицы.
* [Задача #2742](https://github.com/manticoresoftware/manticoresearch/issues/2742) Исправлен пропуск кэша запросов для запросов с упакованными факторами.
* [Задача #2775](https://github.com/manticoresoftware/manticoresearch/issues/2775) Теперь Manticore сообщает об ошибке при неизвестном действии вместо сбоя на `_bulk` запросах.
* [Проблема #2791](https://github.com/manticoresoftware/manticoresearch/issues/2791) Исправлен возврат идентификатора вставленного документа для HTTP `_bulk` endpoint.
* [Проблема #2797](https://github.com/manticoresoftware/manticoresearch/issues/2797) Исправлен сбой в grouper при обработке нескольких таблиц, одна из которых пуста, а другая содержит разное количество совпадающих записей.
* [Проблема #2835](https://github.com/manticoresoftware/manticoresearch/issues/2835) Исправлен сбой в сложных запросах `SELECT`.
* [Проблема #2872](https://github.com/manticoresoftware/manticoresearch/issues/2872) Добавлено сообщение об ошибке, если аргумент `ALL` или `ANY` в выражении `IN` не является атрибутом JSON.
* [Проблема #2882](https://github.com/manticoresoftware/manticoresearch/issues/2882) Исправлен сбой демона при обновлении MVA в больших таблицах.
* [Проблема #2888](https://github.com/manticoresoftware/manticoresearch/issues/2888) Исправлен сбой при ошибке токенизации с `libstemmer`. ❤️ Спасибо, [@subnix](https://github.com/subnix) за PR.
* [Проблема #2919](https://github.com/manticoresoftware/manticoresearch/issues/2919) Исправлена проблема, когда присоединённый вес из правой таблицы неправильно работал в выражениях.
* [Проблема #2919](https://github.com/manticoresoftware/manticoresearch/issues/2919) Исправлена проблема, при которой вес правой присоединённой таблицы не работает в выражениях.
* [Проблема #325](https://github.com/manticoresoftware/manticoresearch-buddy/issues/325) Исправлен сбой `CREATE TABLE IF NOT EXISTS ... WITH DATA`, когда таблица уже существует.
* [Проблема #351](https://github.com/manticoresoftware/manticoresearch-buddy/issues/351) Исправлена ошибка неопределённого ключа массива "id" при подсчёте по KNN с идентификатором документа.
* [Проблема #359](https://github.com/manticoresoftware/manticoresearch-buddy/issues/359) Исправлена функциональность `REPLACE INTO cluster_name:table_name`.
* [Проблема #67](https://github.com/manticoresoftware/docker/issues/67) Исправлена критическая ошибка при запуске контейнера Manticore Docker с `--network=host`.

## Версия 6.3.8
Выпущено: 22 ноября 2024

Версия 6.3.8 продолжает серию 6.3 и включает только исправления ошибок.

### Исправления ошибок

* [PR #2777](https://github.com/manticoresoftware/manticoresearch/pull/2777) Исправлен расчёт доступных потоков, когда параллельность запросов ограничена настройками `threads` или `max_threads_per_query`.

## manticore-extra v1.1.20

Выпущено: 7 октября 2024

### Исправления ошибок

* [Проблема #64](https://github.com/manticoresoftware/executor/issues/64) Решена проблема, когда утилита `unattended-upgrades`, автоматически устанавливающая обновления пакетов в системах на базе Debian, ошибочно помечала несколько пакетов Manticore, включая `manticore-galera`, `manticore-executor` и `manticore-columnar-lib` для удаления. Это происходило из-за того, что `dpkg` ошибочно считал виртуальный пакет `manticore-extra` избыточным. Были внесены изменения, чтобы `unattended-upgrades` больше не пыталась удалять важные компоненты Manticore.

## Версия 6.3.6
Выпущено: 2 августа 2024

Версия 6.3.6 продолжает серию 6.3 и включает только исправления ошибок.

### Исправления ошибок

* [Проблема #2477](https://github.com/manticoresoftware/manticoresearch/issues/2477) Исправлен сбой, возникший в версии 6.3.4, который мог случаться при работе с выражениями и распределёнными или множественными таблицами.
* [Проблема #2352](https://github.com/manticoresoftware/manticoresearch/issues/2352) Исправлен сбой демона или внутренняя ошибка при преждевременном выходе из-за `max_query_time` при запросах к нескольким индексам.

## Версия 6.3.4
Выпущено: 31 июля 2024

Версия 6.3.4 продолжает серию 6.3 и включает только незначительные улучшения и исправления ошибок.

### Незначительные изменения
* [Проблема #1130](https://github.com/manticoresoftware/manticoresearch/issues/1130) Добавлена поддержка [DBeaver](Integration/DBeaver.md#Integration-with-DBeaver).
* [Проблема #2146](https://github.com/manticoresoftware/manticoresearch/issues/2146) Улучшено экранирование разделителей в формах слов и исключениях.
* [Проблема #2315](https://github.com/manticoresoftware/manticoresearch/issues/2315) Добавлены операторы сравнения строк в выражения списка SELECT.
* [Проблема #2363](https://github.com/manticoresoftware/manticoresearch/issues/2363) Добавлена поддержка null-значений в bulk-запросах, похожих на Elastic.
* [Проблема #2374](https://github.com/manticoresoftware/manticoresearch/issues/2374) Добавлена поддержка mysqldump версии 9.
* [Проблема #2375](https://github.com/manticoresoftware/manticoresearch/issues/2375) Улучшена обработка ошибок в HTTP JSON-запросах с JSON-путём к узлу, где возникает ошибка.

### Исправления ошибок
* [Проблема #2280](https://github.com/manticoresoftware/manticoresearch/issues/2280) Исправлено снижение производительности в запросах с подстановочным знаком при большом количестве совпадений, когда disk_chunks > 1.
* [Проблема #2332](https://github.com/manticoresoftware/manticoresearch/issues/2332) Исправлен сбой в выражениях списка SELECT MVA MIN или MAX для пустых массивов MVA.
* [Проблема #2339](https://github.com/manticoresoftware/manticoresearch/issues/2339) Исправлена некорректная обработка бесконечного диапазона в запросах Kibana.
* [Проблема #2342](https://github.com/manticoresoftware/manticoresearch/issues/2342) Исправлен фильтр соединения по колонным атрибутам из правой таблицы, когда атрибут отсутствует в списке SELECT.
* [Проблема #2343](https://github.com/manticoresoftware/manticoresearch/issues/2343) Исправлен дублирующийся спецификатор 'static' в Manticore 6.3.2.
* [Проблема #2344](https://github.com/manticoresoftware/manticoresearch/issues/2344) Исправлен LEFT JOIN, возвращающий неподходящие записи при использовании MATCH() по правой таблице.
* [Проблема #2350](https://github.com/manticoresoftware/manticoresearch/issues/2350) Исправлено сохранение chunk диска в RT индексе с `hitless_words`.
* [Проблема #2364](https://github.com/manticoresoftware/manticoresearch/issues/2364) Свойство 'aggs_node_sort' теперь можно добавлять в любом порядке среди других свойств.
* [Проблема #2368](https://github.com/manticoresoftware/manticoresearch/issues/2368) Исправлена ошибка порядка фильтра по полному тексту и фильтру в JSON-запросе.
* [Проблема #2376](https://github.com/manticoresoftware/manticoresearch/issues/2376) Исправлена ошибка, связанная с некорректным JSON-ответом для длинных UTF-8 запросов.
* [Issue #2684](https://github.com/manticoresoftware/dev/issues/2684) Исправлен расчет выражений presort/prefilter, зависящих от соединенных атрибутов.
* [Issue #301](https://github.com/manticoresoftware/manticoresearch-buddy/issues/301) Изменен способ вычисления размера данных для метрик: теперь используется чтение из файла `manticore.json` вместо проверки всего размера каталога данных.
* [Issue #302](https://github.com/manticoresoftware/manticoresearch-buddy/issues/302) Добавлена обработка запросов валидации от Vector.dev.

## Version 6.3.2
Released: June 26th 2024

Версия 6.3.2 продолжает серию 6.3 и включает несколько исправлений багов, часть из которых обнаружена после выпуска 6.3.0.

### Breaking changes
* ⚠️[Issue #2305](https://github.com/manticoresoftware/manticoresearch/issues/2305) Обновлены значения aggs.range — теперь они числовые.

### Bug fixes
* [Commit c51c](https://github.com/manticoresoftware/manticoresearch/commit/c51c7fa4bd0ec4ad8d53188b888461bff29f1417) Исправлена группировка по проверке сохранённого vs rset merge.
* [Commit 0e85](https://github.com/manticoresoftware/manticoresearch/commit/0e85b9d5682f51681750d798aef6a11cfb93d69e) Исправлен сбой демона при запросах с использованием символов подстановки в RT-индексе с CRC-словарём и включённым `local_df`.
* [Issue #2200](https://github.com/manticoresoftware/manticoresearch/issues/2200) Исправлен сбой в JOIN при использовании `count(*)` без GROUP BY.
* [Issue #2201](https://github.com/manticoresoftware/manticoresearch/issues/2201) Исправлено отсутствие предупреждения в JOIN при попытке группировки по полю полнотекстового поиска.
* [Issue #2230](https://github.com/manticoresoftware/manticoresearch/issues/2230) Исправлена ситуация, когда добавление атрибута через `ALTER TABLE` не учитывало опции KNN.
* [Issue #2231](https://github.com/manticoresoftware/manticoresearch/issues/2231) Исправлен сбой при удалении пакета `manticore-tools` для Redhat в версии 6.3.0.
* [Issue #2242](https://github.com/manticoresoftware/manticoresearch/issues/2242) Исправлены ошибки при JOIN и множественных операторах FACET, приводившие к неверным результатам.
* [Issue #2250](https://github.com/manticoresoftware/manticoresearch/issues/2250) Исправлена ошибка ALTER TABLE при работе с таблицей в кластере.
* [Issue #2252](https://github.com/manticoresoftware/manticoresearch/issues/2252) Исправлена передача исходного запроса в buddy из интерфейса SphinxQL.
* [Issue #2267](https://github.com/manticoresoftware/manticoresearch/issues/2267) Улучшено расширение подстановочных знаков в `CALL KEYWORDS` для RT-индекса с дисковыми чанками.
* [Issue #271](https://github.com/manticoresoftware/manticoresearch-buddy/issues/271) Исправлено зависание при некорректных запросах `/cli`.
* [Issue #274](https://github.com/manticoresoftware/manticoresearch-buddy/issues/274) Решены проблемы, из-за которых параллельные запросы к Manticore могли застревать.
* [Issue #275](https://github.com/manticoresoftware/manticoresearch-buddy/issues/275) Исправлено зависание при выполнении `drop table if exists t; create table t` через `/cli`.

### Replication-related
* [Issue #2270](https://github.com/manticoresoftware/manticoresearch/issues/2270) Добавлена поддержка формата `cluster:name` в HTTP-эндпоинте `/_bulk`.

## Version 6.3.0
Released: May 23rd 2024

### Major changes
* [Issue #839](https://github.com/manticoresoftware/manticoresearch/issues/839) Реализован тип данных [float_vector](Creating_a_table/Data_types.md#Float-vector); реализован [векторный поиск](Searching/KNN.md#KNN-vector-search).
* [Issue #1673](https://github.com/manticoresoftware/manticoresearch/issues/1673) Добавлены [INNER/LEFT JOIN](Searching/Joining.md) (**бета-версия**).
* [Issue #1744](https://github.com/manticoresoftware/manticoresearch/issues/1744) Реализовано автоматическое определение форматов даты для полей [timestamp](Creating_a_table/Data_types.md#Timestamps).
* [Issue #1720](https://github.com/manticoresoftware/manticoresearch/pull/1720) Лицензия Manticore Search изменена с GPLv2-or-later на GPLv3-or-later.
* [Commit 7a55](https://github.com/manticoresoftware/manticoresearch/commit/7a5508424) Для запуска Manticore в Windows теперь требуется Docker для запуска Buddy.
* [Issue #1541](https://github.com/manticoresoftware/manticoresearch/issues/1541) Добавлен полнотекстовый оператор [REGEX](Searching/Full_text_matching/Operators.md#REGEX-operator).
* [Issue #2091](https://github.com/manticoresoftware/manticoresearch/issues/2091) Добавлена поддержка Ubuntu Noble 24.04.
* [Commit 514d](https://github.com/manticoresoftware/manticoresearch/commit/514d35b497f4bdb20b6473ff963752a978a4bb8d) Переработка операций с временем для улучшения производительности и новых функций даты/времени:
  - [CURDATE()](Functions/Date_and_time_functions.md#CURDATE%28%29) — возвращает текущую дату в локальном часовом поясе
  - [QUARTER()](Functions/Date_and_time_functions.md#QUARTER%28%29) — возвращает порядковый номер квартала года из аргумента timestamp
  - [DAYNAME()](Functions/Date_and_time_functions.md#DAYNAME%28%29) — возвращает название дня недели для заданного timestamp
  - [MONTHNAME()](Functions/Date_and_time_functions.md#MONTHNAME%28%29) — возвращает название месяца для заданного timestamp
  - [DAYOFWEEK()](Functions/Date_and_time_functions.md#DAYOFWEEK%28%29) — возвращает порядковый номер дня недели для заданного timestamp
  - [DAYOFYEAR()](Functions/Date_and_time_functions.md#DAYOFYEAR%28%29) — возвращает порядковый номер дня в году для заданного timestamp
  - [YEARWEEK()](Functions/Date_and_time_functions.md#YEARWEEK%28%29) — возвращает год и код первого дня текущей недели для заданного timestamp
  - [DATEDIFF()](Functions/Date_and_time_functions.md#DATEDIFF%28%29) — возвращает количество дней между двумя timestamp
  - [DATE()](Functions/Date_and_time_functions.md#DATE%28%29) — формирует дату из timestamp
  - [TIME()](Functions/Date_and_time_functions.md#TIME%28%29) — формирует время из timestamp
  - [timezone](Server_settings/Searchd.md#timezone) — часовой пояс, используемый функциями, связанными с датой и временем.
* [Коммит 30e7](https://github.com/manticoresoftware/manticoresearch/commit/30e789d9657f9b3093c280c6e5b0e46a021f6dc7) Добавлены агрегаты [range](Searching/Faceted_search.md#Facet-over-set-of-ranges), [histogram](Searching/Faceted_search.md#Facet-over-histogram-values), [date_range](Searching/Faceted_search.md#Facet-over-set-of-date-ranges) и [date_histogram](Searching/Faceted_search.md#Facet-over-histogram-date-values) в HTTP интерфейс и аналогичные выражения в SQL.

### Незначительные изменения
* [Issue #1285](https://github.com/manticoresoftware/manticoresearch/issues/1285) Поддержка версий Filebeat 8.10 - 8.11.
* [Issue #1771](https://github.com/manticoresoftware/manticoresearch/issues/1771) [ALTER TABLE ... type='distributed'](Updating_table_schema_and_settings.md#Changing-a-distributed-table).
* [Issue #1788](https://github.com/manticoresoftware/manticoresearch/issues/1788) Добавлена возможность копирования таблиц с помощью SQL-запроса [CREATE TABLE ... LIKE ... WITH DATA](Creating_a_table/Local_tables/Real-time_table.md#CREATE-TABLE-LIKE:).
* [Issue #2072](https://github.com/manticoresoftware/manticoresearch/issues/2072) Оптимизирован [алгоритм уплотнения таблиц](Securing_and_compacting_a_table/Compacting_a_table.md#Compacting-a-Table): Ранее как ручной процесс [OPTIMIZE](Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE), так и автоматический [auto_optimize](Server_settings/Searchd.md#auto_optimize) сначала слияли чанки, чтобы не превышать лимит, а затем удаляли удалённые документы из всех других чанков с удалёнными документами. Этот подход был иногда слишком ресурсоёмким и был отключён. Теперь слияние чанков происходит только по настройке [progressive_merge](Server_settings/Common.md#progressive_merge). Однако чанки с большим количеством удалённых документов с большей вероятностью будут слиты.
* [Коммит ce6c](https://github.com/manticoresoftware/columnar/commit/ce6c5d3381edcdc411a00a5923d13d56dd9b3010) Добавлена защита от загрузки вторичного индекса новой версии.
* [Issue #1417](https://github.com/manticoresoftware/manticoresearch/issues/1417) Частичная замена через [REPLACE INTO ... SET](Data_creation_and_modification/Updating_documents/REPLACE.md#SQL-REPLACE).
* [Коммит 7c16](https://github.com/manticoresoftware/manticoresearch/commit/7c167b934ce416d3dfbd658744a231e638b14d9a) Обновлены размеры буферов слияния по умолчанию: `.spa` (скалярные атрибуты): 256KB -> 8MB; `.spb` (blob-атрибуты): 256KB -> 8MB; `.spc` (колоночные атрибуты): 1MB, без изменений; `.spds` (docstore): 256KB -> 8MB; `.spidx` (вторичные индексы): буфер 256KB -> лимит памяти 128MB; `.spi` (словарь): 256KB -> 16MB; `.spd` (списки документов): 8MB, без изменений; `.spp` (списки попаданий): 8MB, без изменений; `.spe` (списки пропусков): 256KB -> 8MB.
* [Issue #1859](https://github.com/manticoresoftware/manticoresearch/issues/1859) Добавлена [композитная агрегация](Searching/Grouping.md#GROUP-BY-multiple-fields-at-once) через JSON.
* [Коммит 216b](https://github.com/manticoresoftware/manticoresearch-buddy/commit/216b824) Отключен PCRE.JIT из-за проблем с некоторыми регулярными выражениями и отсутствия значительного выигрыша во времени.
* [Коммит 55cd](https://github.com/manticoresoftware/manticoresearch/commit/55cdce32) Добавлена поддержка vanilla Galera v.3 (api v25) (`libgalera_smm.so` из MySQL 5.x).
* [Коммит 86f9](https://github.com/manticoresoftware/manticoresearch-buddy/commit/86f98ab) Суффикс метрик изменён с `_rate` на `_rps`.
* [Коммит c0c1](https://github.com/manticoresoftware/manticoresearch-helm/commit/c0c1cff780031412ef57cefab987dfd16613368e) Улучшена документация о поддержке HA балансировщика.
* [Коммит d1d2](https://github.com/manticoresoftware/manticoresearch/commit/d1d2ca18857c68106e89743756260e73285be739) В сообщениях об ошибках `index` изменено на `table`; исправлена ошибка сообщения парсера bison.
* [Коммит fd26](https://github.com/manticoresoftware/manticoresearch/commit/fd26671b) Добавлена поддержка имени таблицы `manticore.tbl`.
* [Issue #1105](https://github.com/manticoresoftware/manticoresearch/issues/1105) Поддержка запуска индексатора через systemd ([документация](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Schedule-indexer-via-systemd)). ❤️ Спасибо, [@subnix](https://github.com/subnix) за PR.
* [Issue #1294](https://github.com/manticoresoftware/manticoresearch/issues/1294) Поддержка вторичных индексов в GEODIST().
* [Issue #1394](https://github.com/manticoresoftware/manticoresearch/issues/1394) Упрощён [SHOW THREADS](Node_info_and_management/SHOW_THREADS.md#SHOW-THREADS).
* [Issue #1424](https://github.com/manticoresoftware/manticoresearch/issues/1424) Добавлена поддержка значений по умолчанию (`agent_connect_timeout` и `agent_query_timeout`) для оператора `create distributed table`.
* [Issue #1442](https://github.com/manticoresoftware/manticoresearch/issues/1442) Добавлена опция поискового запроса [expansion_limit](Searching/Options.md#expansion_limit), которая переопределяет `searchd.expansion_limit`.
* [Issue #1448](https://github.com/manticoresoftware/manticoresearch/issues/1448) Реализован [ALTER TABLE](Updating_table_schema_and_settings.md#Updating-table-schema-in-RT-mode) для преобразования int->bigint.
* [Issue #1456](https://github.com/manticoresoftware/manticoresearch/issues/1456) Метаинформация в ответе MySQL.
* [Issue #1494](https://github.com/manticoresoftware/manticoresearch/issues/1494) [SHOW VERSION](Node_info_and_management/SHOW_VERSION.md#SHOW-VERSION).
* [Issue #1582](https://github.com/manticoresoftware/manticoresearch/issues/1582) Поддержка удаления документов по массиву id через JSON ([удаление документов](Data_creation_and_modification/Deleting_documents.md?client=JSON#Deleting-documents)).
* [Issue #1589](https://github.com/manticoresoftware/manticoresearch/issues/1589) Улучшено сообщение об ошибке "unsupported value type".
* [Issue #1634](https://github.com/manticoresoftware/manticoresearch/issues/1634) Добавлена версия Buddy в `SHOW STATUS`.
* [Issue #1641](https://github.com/manticoresoftware/manticoresearch/issues/1641) Оптимизация запросов Match в случае нулевого количества документов по ключевому слову.
* [Issue #1712](https://github.com/manticoresoftware/manticoresearch/issues/1712) Добавлена конвертация в булево значение из строкового значения "true" и "false" при отправке данных.
* [Issue #1713](https://github.com/manticoresoftware/manticoresearch/issues/1713) Добавлена таблица и опция searchd [access_dict](Server_settings/Searchd.md#access_dict).
* [Issue #1767](https://github.com/manticoresoftware/manticoresearch/issues/1767) Добавлены новые опции: [expansion_merge_threshold_docs](Server_settings/Searchd.md#expansion_merge_threshold_docs) и [expansion_merge_threshold_hits](Server_settings/Searchd.md#expansion_merge_threshold_hits) в секцию searchd конфига; сделан порог слияния мелких терминов расширенных терминов настраиваемым.
* [Issue #1768](https://github.com/manticoresoftware/manticoresearch/issues/1768) Добавлено отображение времени последней команды в `@@system.sessions`.
* [Issue #1806](https://github.com/manticoresoftware/manticoresearch/issues/1806) Обновлен протокол Buddy до версии 2.
* [Issue #1810](https://github.com/manticoresoftware/manticoresearch/issues/1810) Реализованы дополнительные форматы запросов к эндпоинту `/sql` для упрощения интеграции с библиотеками.
* [Issue #1825](https://github.com/manticoresoftware/manticoresearch/pull/1825) Добавлен раздел Информация к SHOW BUDDY PLUGINS.
* [Issue #1837](https://github.com/manticoresoftware/manticoresearch/issues/1837) Улучшено потребление памяти в `CALL PQ` с большими пакетами.
* [Issue #1853](https://github.com/manticoresoftware/manticoresearch/pull/1853) Компилятор переключён с Clang 15 на Clang 16.
* [Issue #1857](https://github.com/manticoresoftware/manticoresearch/issues/1857) Добавлено сравнение строк. ❤️ Спасибо, [@etcd](https://github.com/etcd) за PR.
* [Issue #1915](https://github.com/manticoresoftware/manticoresearch/issues/1915) Добавлена поддержка объединённых сохранённых полей.
* [Issue #1937](https://github.com/manticoresoftware/manticoresearch/issues/1937) Добавлен лог хоста:порта клиента в query-log.
* [Issue #1981](https://github.com/manticoresoftware/manticoresearch/issues/1981) Исправлена неправильная ошибка.
* [Issue #1983](https://github.com/manticoresoftware/manticoresearch/issues/1983) Введена поддержка уровней подробности для [плана запроса в формате JSON](Node_info_and_management/Profiling/Query_plan.md?client=JSON#Query-plan).
* [Issue #2010](https://github.com/manticoresoftware/manticoresearch/issues/2010) Отключено логирование запросов от Buddy, если не установлен `log_level=debug`.
* [Issue #2035](https://github.com/manticoresoftware/manticoresearch/issues/2035) Поддержка Linux Mint 21.3.
* [Issue #2056](https://github.com/manticoresoftware/manticoresearch/issues/2056) Manticore не собирался с Mysql 8.3+.
* [Issue #2112](https://github.com/manticoresoftware/manticoresearch/issues/2112) Команда `DEBUG DEDUP` для чанков таблиц реального времени, которые могут иметь дубли после присоединения plain таблицы с дубликатами.
* [Issue #212](https://github.com/manticoresoftware/manticoresearch-buddy/issues/212) Добавлено время в [SHOW QUERIES](Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES).
* [Issue #218](https://github.com/manticoresoftware/manticoresearch-buddy/issues/218) Обработка колонки `@timestamp` как timestamp.
* [Issue #252](https://github.com/manticoresoftware/manticoresearch-buddy/issues/250) Реализована логика включения/отключения плагинов buddy.
* [Issue #254](https://github.com/manticoresoftware/manticoresearch-buddy/issues/254) Обновлен composer на более свежую версию с исправленными последними CVE.
* [Issue #340](https://github.com/manticoresoftware/manticoresearch/issues/340) Небольшая оптимизация systemd юнита Manticore, связанная с `RuntimeDirectory`.
* [Issue #51](https://github.com/manticoresoftware/executor/issues/51) Добавлена поддержка rdkafka и обновлен PHP до версии 8.3.3.
* [Issue #527](https://github.com/manticoresoftware/manticoresearch/issues/527) Поддержка [присоединения](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Attaching_one_table_to_another.md#Attaching-one-table-to-another) таблицы RT. Новая команда [ALTER TABLE ... RENAME](Updating_table_schema_and_settings.md#Renaming-a-real-time-table).

### Существенные изменения и устаревшие возможности
* ⚠️[Issue #1436](https://github.com/manticoresoftware/manticoresearch/issues/1436) Исправлена проблема с расчетом IDF. `local_df` теперь по умолчанию. Улучшен протокол поиска master-agent (версия обновлена). Если вы используете Manticore Search в распределенной среде с несколькими инстансами, обязательно сначала обновите агенты, затем мастеров.
* ⚠️[Issue #1572](https://github.com/manticoresoftware/manticoresearch/issues/1572) Добавлена репликация распределенных таблиц и обновлен протокол репликации. Если вы используете кластер репликации, необходимо:
  - Сначала корректно остановить все ваши узлы
  - Затем запустить последний остановленный узел с `--new-cluster`, используя инструмент `manticore_new_cluster` в Linux.
  - Подробности см. в [перезапуске кластера](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster).
* ⚠️[Issue #1763](https://github.com/manticoresoftware/manticoresearch/pull/1763) Алиасы HTTP API эндпоинтов `/json/*` объявлены устаревшими.
* ⚠️[Issue #1982](https://github.com/manticoresoftware/manticoresearch/issues/1982) Изменено [profile](Node_info_and_management/Profiling/Query_profile.md#Query-profile) на [plan](Node_info_and_management/Profiling/Query_plan.md#Query-plan) в JSON, добавлен профилинг запросов для JSON.
* ⚠️[Commit e235](https://github.com/manticoresoftware/manticoresearch-backup/commit/e23585a) manticore-backup больше не бэкапит `plugin_dir`.
* ⚠️[Issue #171](https://github.com/manticoresoftware/manticoresearch-buddy/issues/155) Миграция Buddy на Swoole для улучшения производительности и стабильности. При переходе на новую версию убедитесь, что все пакеты Manticore обновлены.
* ⚠️[Issue #196](https://github.com/manticoresoftware/manticoresearch-buddy/pull/205) Все основные плагины объединены в Buddy и изменена основная логика.
* ⚠️[Задача #2107](https://github.com/manticoresoftware/manticoresearch/issues/2107) Обработка идентификаторов документов как чисел в ответах `/search`.
* ⚠️[Задача #38](https://github.com/manticoresoftware/executor/issues/38) Добавлен Swoole, отключен ZTS и удалено расширение parallel.
* ⚠️[Задача #1929](https://github.com/manticoresoftware/manticoresearch/issues/1929) Переопределение в `charset_table` не работало в некоторых случаях.

### Изменения, связанные с репликацией
* [Коммит 3376](https://github.com/manticoresoftware/manticoresearch/commit/3376351f73db3ff0cb6cb708a304a7bd83e69cfb) Исправлена ошибка репликации при SST больших файлов.
* [Коммит 6d36](https://github.com/manticoresoftware/manticoresearch/commit/6d36c68fb7ce3cb274b467aac435788905084952) Добавлен механизм повторных попыток для команд репликации; исправлен сбой присоединения к репликации на загруженной сети с потерей пакетов.
* [Коммит 842e](https://github.com/manticoresoftware/manticoresearch/commit/842e27e71790833ab4125d49103d2933d5df60c3) Сообщение FATAL в репликации изменено на предупреждение WARNING.
* [Коммит 8c32](https://github.com/manticoresoftware/manticoresearch/commit/8c32bce7db39d8ed3b1a06aaa1d2c3ec372084f0) Исправлен расчет `gcache.page_size` для репликационных кластеров без таблиц или с пустыми таблицами; также исправлено сохранение и загрузка опций Galera.
* [Коммит a2af](https://github.com/manticoresoftware/manticoresearch/commit/a2af06ca36abcbf893e43c8803853130b397c54b) Добавлена возможность пропускать команду репликации update nodes на ноде, которая присоединяется к кластеру.
* [Коммит c054](https://github.com/manticoresoftware/manticoresearch/commit/c0541e6892f5116b1f957b40060ba8ecd01a5afb) Исправлена взаимоблокировка при репликации при обновлении blob-атрибутов и замене документов.
* [Коммит e80d](https://github.com/manticoresoftware/manticoresearch/commit/e80d505b963999016033613c40c1cf3151d648e7) Добавлены параметры конфигурации searchd [replication_connect_timeout](Server_settings/Searchd.md#replication_connect_timeout), [replication_query_timeout](Server_settings/Searchd.md#replication_query_timeout), [replication_retry_delay](Server_settings/Searchd.md#replication_retry_delay), [replication_retry_count](Server_settings/Searchd.md#replication_retry_count) для управления сетью при репликации, аналогично `searchd.agent_*`, но с иными значениями по умолчанию.
* [Задача #1356](https://github.com/manticoresoftware/manticoresearch/issues/1356) Исправлена повторная попытка подключения к узлам репликации после пропуска некоторых нод и отказа в разрешении их имен.
* [Задача #1445](https://github.com/manticoresoftware/manticoresearch/issues/1445) Исправлен уровень детализации логов репликации в переменной `show variables`.
* [Задача #1482](https://github.com/manticoresoftware/manticoresearch/issues/1482) Исправлена проблема репликации для узла-присоединяющегося при подключении к кластеру после перезапуска пода в Kubernetes.
* [Задача #1962](https://github.com/manticoresoftware/manticoresearch/issues/1962) Исправлено длительное ожидание репликации изменения на пустом кластере с неверным именем ноды.

### Исправления ошибок
* [Коммит 8a48](https://github.com/manticoresoftware/manticoresearch/commit/8a486c7ba1a293879cdecc41fb52eb7ab0cbf832) Исправлена очистка неиспользуемых совпадений в `count distinct`, которая могла вызвать сбой.
* [Задача #1569](https://github.com/manticoresoftware/manticoresearch/issues/1569) Двоичный журнал теперь записывается с транзакционной гранулярностью.
* [Задача #2089](https://github.com/manticoresoftware/manticoresearch/issues/2089) Исправлена ошибка с 64-битными идентификаторами, которая могла привести к ошибке "Malformed packet" при вставке через MySQL и в итоге к [повреждению таблиц и дубликатам ID](https://github.com/manticoresoftware/manticoresearch/issues/2090).
* [Задача #2160](https://github.com/manticoresoftware/manticoresearch/issues/2160) Исправлена вставка дат как если бы они были в UTC, а не в локальном часовом поясе.
* [Задача #2177](https://github.com/manticoresoftware/manticoresearch/issues/2177) Исправлен сбой при выполнении поиска в таблице реального времени с непустым `index_token_filter`.
* [Задача #2209](https://github.com/manticoresoftware/manticoresearch/issues/2209) Изменена фильтрация дубликатов в RT колоннарном хранилище для исправления сбоев и неверных результатов запросов.
* [Коммит 001d](https://github.com/manticoresoftware/manticoresearch/commit/001d82904e4b887c49387308291aeefb07c0fbc4) Исправлен html stripper, портящий память после обработки соединенного поля.
* [Коммит 00eb](https://github.com/manticoresoftware/manticoresearch/commit/00eb2ebf) Избежан перемотки потока после flush для предотвращения проблем коммуникации с mysqldump.
* [Коммит 0553](https://github.com/manticoresoftware/manticoresearch/commit/05534114acf08c70ffcc41a23d558dec09a34373) Не ждать окончания preread, если он не запущен.
* [Коммит 055a](https://github.com/manticoresoftware/manticoresearch/commit/055aac09f877570cd29b6594cb283182eba7f73b) Исправлена слишком длинная строка вывода Buddy для разбиения на несколько строк в логе searchd.
* [Коммит 0a88](https://github.com/manticoresoftware/manticoresearch/commit/0a88056307f82c50fcdc458ad6dbc307a00e3f92) Перемещено предупреждение интерфейса MySQL о невозможности задать уровень детализации заголовка `debugv`.
* [Коммит 150a](https://github.com/manticoresoftware/manticoresearch/commit/150a20286b7d98d24b844ae577d7fda4a444798d) Исправлено состояние гонки при работе с несколькими кластерными операциями; запрещено создание нескольких кластеров с одинаковым именем или путем.
* [Коммит 2e40](https://github.com/manticoresoftware/manticoresearch/commit/2e405c1c9ebac6f46206e3df17ac1c19129c9aa3) Исправлено неявное усечение в полнотекстовых запросах; разделен MatchExtended на шаблонную частьD.
* [Коммит 75f5](https://github.com/manticoresoftware/manticoresearch/commit/75f51421542a2a0658f9e2f9f31048c8c5224479) Исправлено расхождение `index_exact_words` между индексированием и загрузкой таблицы в демоне.
* [Коммит 7643](https://github.com/manticoresoftware/manticoresearch/commit/7643976bea142e08535d701000eff0d09a99fda2) Исправлено отсутствие сообщения об ошибке при удалении неверного кластера.
* [Commit 7a03](https://github.com/manticoresoftware/manticoresearch/commit/7a034ac990d5ec9344320c05a6a2315583b9b858) Исправлена ошибка CBO с объединением очередей; исправлена ошибка CBO с псевдо-шардингом RT.
* [Commit 7b4e](https://github.com/manticoresoftware/manticoresearch/commit/7b4ea95e) При запуске без библиотеки вторичных индексов (SI) и параметров в конфигурации выводилось вводящее в заблуждение предупреждающее сообщение 'WARNING: secondary_indexes set but failed to initialize secondary library'.
* [Commit 8496](https://github.com/manticoresoftware/manticoresearch/commit/8496317f099fade8d2db463fb88483e7fcdfe85b) исправлена сортировка попаданий при кворуме.
* [Commit 8973](https://github.com/manticoresoftware/manticoresearch-buddy/commit/8973ad3) Исправлена ошибка с опциями в верхнем регистре в плагине ModifyTable.
* [Commit 9935](https://github.com/manticoresoftware/manticoresearch-buddy/commit/99350d4c184379b19f3efa9751d9b5be5e6edfb5) Исправлено восстановление из дампа с пустыми json значениями (представленными как NULL).
* [Commit a28f](https://github.com/manticoresoftware/manticoresearch/commit/a28f6e087b3d957e48816070be17d0823904936a) Исправлен таймаут SST на узле joiner при приеме SST с использованием pcon.
* [Commit b5a5](https://github.com/manticoresoftware/manticoresearch/commit/b5a55cc26debcddcda5d9caef8366a707e0dd763) Исправлен сбой при выборе строкового атрибута с псевдонимом.
* [Commit c556](https://github.com/manticoresoftware/manticoresearch/commit/c556e7ed8b6da8eac8ef094650df5f0a8977916b) Добавлено преобразование запроса термина в `=term` полного текстового запроса с полем `morphology_skip_fields`.
* [Commit cdc3](https://github.com/manticoresoftware/manticoresearch/commit/cdc38085145cd1025b96fcac2a46188a0425d9e1) Добавлен отсутствующий ключ конфигурации (skiplist_cache_size).
* [Commit cf6e](https://github.com/manticoresoftware/manticoresearch/commit/cf6ec3bfdc3f0990fb31ff53822b2331a6667727) Исправлен сбой ранжировщика выражений с большим сложным запросом.
* [Commit e513](https://github.com/manticoresoftware/manticoresearch/commit/e5139ba75eef7223df375ae3cc3f138fa7d925f1) Исправлена ошибка fulltext CBO с недопустимыми подсказками индекса.
* [Commit eb05](https://github.com/manticoresoftware/manticoresearch/commit/eb0514b3a6f57f278252b0724eebd112ec116e0f) Прерывание preread при отключении для более быстрого завершения.
* [Commit f945](https://github.com/manticoresoftware/manticoresearch/commit/f94555a293cfa51004ff81135b8bd8500b955115) Изменен расчет стека для полнотекстовых запросов, чтобы избежать сбоя при сложном запросе.
* [Issue #1262](https://github.com/manticoresoftware/manticoresearch/issues/1262) Исправлен сбой индекса при индексировании SQL-источника с несколькими столбцами с одинаковым именем.
* [Issue #1273](https://github.com/manticoresoftware/manticoresearch/issues/1273) Возвращать 0 вместо <empty> для несуществующих sysvars.
* [Issue #1289](https://github.com/manticoresoftware/manticoresearch/issues/1289) Исправлена ошибка indextool при проверке внешних файлов таблицы RT.
* [Issue #1335](https://github.com/manticoresoftware/manticoresearch/issues/1335) Исправлена ошибка парсинга запроса из-за многословной формы внутри фразы.
* [Issue #1364](https://github.com/manticoresoftware/manticoresearch/issues/1364) Добавлен повтор воспроизведения пустых binlog файлов со старыми версиями binlog.
* [Issue #1365](https://github.com/manticoresoftware/manticoresearch/issues/1365) Исправлено удаление последнего пустого binlog файла.
* [Issue #1372](https://github.com/manticoresoftware/manticoresearch/issues/1372) Исправлены неправильные относительные пути (преобразованные в абсолютные из каталога запуска демона) после изменения `data_dir`, влияющие на текущий рабочий каталог при запуске демона.
* [Issue #1393](https://github.com/manticoresoftware/manticoresearch/issues/1393) Самое медленное ухудшение времени в hn_small: получение/кэширование информации о процессоре при запуске демона.
* [Issue #1395](https://github.com/manticoresoftware/manticoresearch/issues/1395) Исправлено предупреждение о отсутствующем внешнем файле при загрузке индекса.
* [Issue #1402](https://github.com/manticoresoftware/manticoresearch/issues/1402) Исправлен сбой в глобальных группировщиках при освобождении атрибутов указателя на данные.
* [Issue #1403](https://github.com/manticoresoftware/manticoresearch/issues/1403) _ADDITIONAL_SEARCHD_PARAMS не работает.
* [Issue #1427](https://github.com/manticoresoftware/manticoresearch/issues/1427) Исправлено замещение per table `agent_query_timeout` опцией запроса по умолчанию `agent_query_timeout`.
* [Issue #1444](https://github.com/manticoresoftware/manticoresearch/issues/1444) Исправлен сбой в группировщике и ранжировщике при использовании `packedfactors()` с несколькими значениями на совпадение.
* [Issue #1458](https://github.com/manticoresoftware/manticoresearch/issues/1458) Manticore падает при частом обновлении индекса.
* [Issue #1481](https://github.com/manticoresoftware/manticoresearch/issues/1481) Исправлен сбой при очистке разобранного запроса после ошибки парсинга.
* [Issue #1484](https://github.com/manticoresoftware/manticoresearch/issues/1484) Исправлена маршрутизация HTTP JSON запросов к buddy.
* [Issue #1499](https://github.com/manticoresoftware/manticoresearch/issues/1499) Корневое значение JSON атрибута не могло быть массивом. Исправлено.
* [Issue #1507](https://github.com/manticoresoftware/manticoresearch/issues/1507) Исправлен сбой при воссоздании таблицы в транзакции.
* [Issue #1515](https://github.com/manticoresoftware/manticoresearch/issues/1515) Исправлено расширение коротких форм RU лемм.
* [Issue #1579](https://github.com/manticoresoftware/manticoresearch/issues/1579) Исправлено использование JSON и STRING атрибутов в выражении [date_format](Functions/Date_and_time_functions.md#DATE_FORMAT()).
* [Issue #1580](https://github.com/manticoresoftware/manticoresearch/issues/1580) Исправлен группировщик для нескольких псевдонимов JSON полей.
* [Issue #1594](https://github.com/manticoresoftware/manticoresearch/issues/1594) Некорректное значение total_related в dev: исправлен неявный cutoff против limit; добавлено лучшее определение полного сканирования в json запросах.
* [Issue #1603](https://github.com/manticoresoftware/manticoresearch/issues/1603) Исправлено использование JSON и STRING атрибутов во всех датах выражениях.
* [Issue #1609](https://github.com/manticoresoftware/manticoresearch/issues/1609) сбой при использовании LEVENSHTEIN().
* [Issue #1612](https://github.com/manticoresoftware/manticoresearch/issues/1612) Исправлена ошибка повреждения памяти после ошибки разбора поискового запроса с подсветкой.
* [Issue #1614](https://github.com/manticoresoftware/manticoresearch/issues/1614) Отключено расширение wildcard для терминов короче `min_prefix_len` / `min_infix_len`.
* [Issue #1617](https://github.com/manticoresoftware/manticoresearch/issues/1617) Изменено поведение: теперь ошибка не регистрируется, если Buddy успешно обрабатывает запрос.
* [Issue #1635](https://github.com/manticoresoftware/manticoresearch/issues/1635) Исправлено значение total в метаданных поискового запроса для запросов с установленным ограничением limit.
* [Issue #1640](https://github.com/manticoresoftware/manticoresearch/issues/1640) Невозможно использовать таблицу с заглавными буквами через JSON в plain режиме.
* [Issue #1643](https://github.com/manticoresoftware/manticoresearch/issues/1643) Добавлено значение по умолчанию для `SPH_EXTNODE_STACK_SIZE`.
* [Issue #1646](https://github.com/manticoresoftware/manticoresearch/issues/1646) Исправлен лог SphinxQL для отрицательного фильтра с ALL/ANY по MVA атрибуту.
* [Issue #1660](https://github.com/manticoresoftware/manticoresearch/issues/1660) Исправлено применение списков исключений docid из других индексов. ❤️ Спасибо, [@raxoft](https://github.com/raxoft), за PR.
* [Issue #1668](https://github.com/manticoresoftware/manticoresearch/issues/1668) Исправлены пропущенные совпадения из-за преждевременного выхода при полном обходе сырого индекса (без итераторов индексов); удалено ограничение cutoff из plain row итератора.
* [Issue #1671](https://github.com/manticoresoftware/manticoresearch/issues/1671) Исправлена ошибка `FACET` при запросе распределённой таблицы с агентом и локальными таблицами.
* [Issue #1690](https://github.com/manticoresoftware/manticoresearch/issues/1690) Исправлен сбой при оценке гистограммы для больших значений.
* [Issue #1692](https://github.com/manticoresoftware/manticoresearch/issues/1692) сбой при alter table tbl add column col uint.
* [Issue #1710](https://github.com/manticoresoftware/manticoresearch/issues/1710) Пустой результат для условия `WHERE json.array IN (<value>)`.
* [Issue #172](https://github.com/manticoresoftware/manticoresearch-buddy/issues/172) Исправлена проблема с TableFormatter при отправке запроса на `/cli`.
* [Issue #1742](https://github.com/manticoresoftware/manticoresearch/issues/1742) `CREATE TABLE` не вызывал ошибку при отсутствии файла wordforms.
* [Issue #1762](https://github.com/manticoresoftware/manticoresearch/issues/1762) Порядок атрибутов в RT таблицах теперь соответствует порядку в конфигурации.
* [Issue #1765](https://github.com/manticoresoftware/manticoresearch/issues/1765) HTTP bool запрос с условием 'should' возвращает неправильные результаты.
* [Issue #1769](https://github.com/manticoresoftware/manticoresearch/issues/1769) Сортировка по строковым атрибутам не работает с `SPH_SORT_ATTR_DESC` и `SPH_SORT_ATTR_ASC`.
* [Issue #177](https://github.com/manticoresoftware/manticoresearch-buddy/issues/177) Отключён заголовок HTTP `Expect: 100-continue` для curl-запросов к Buddy.
* [Issue #1791](https://github.com/manticoresoftware/manticoresearch/issues/1791) сбой из-за псевдонима в GROUP BY.
* [Issue #1792](https://github.com/manticoresoftware/manticoresearch/issues/1792) SQL meta summary показывает неправильное время в Windows.
* [Issue #1794](https://github.com/manticoresoftware/manticoresearch/issues/1794) Исправлено падение производительности при поиске по одному термину в JSON запросах.
* [Issue #1798](https://github.com/manticoresoftware/manticoresearch/issues/1798) Несовместимые фильтры не вызывали ошибку на `/search`.
* [Issue #1802](https://github.com/manticoresoftware/manticoresearch/issues/1802) Исправлена блокировка операций `ALTER CLUSTER ADD` и `JOIN CLUSTER`, чтобы они дожидались друг друга, предотвращая состояние гонки, когда `ALTER` добавляет таблицу в кластер, а узел донор отправляет таблицы узлу присоединения.
* [Issue #1811](https://github.com/manticoresoftware/manticoresearch/issues/1811) Некорректная обработка запросов `/pq/{table}/*`.
* [Issue #1816](https://github.com/manticoresoftware/manticoresearch/issues/1816) `UNFREEZE` не работал в некоторых случаях.
* [Issue #183](https://github.com/manticoresoftware/manticoresearch-buddy/pull/183) Исправлена проблема с восстановлением MVA в некоторых случаях.
* [Issue #1849](https://github.com/manticoresoftware/manticoresearch/issues/1849) Исправлен сбой indextool при завершении работы при использовании с MCL.
* [Issue #1866](https://github.com/manticoresoftware/manticoresearch/issues/1866) Исправлено ненужное декодирование url для запросов `/cli_json`.
* [Issue #1872](https://github.com/manticoresoftware/manticoresearch/issues/1872) изменена логика установки plugin_dir при запуске демона.
* [Issue #1874](https://github.com/manticoresoftware/manticoresearch/issues/1874) сбоят исключения при alter table ...
* [Issue #1891](https://github.com/manticoresoftware/manticoresearch/issues/1891#issuecomment-2051941200) Manticore падает с `signal 11` при вставке данных.
* [Issue #1920](https://github.com/manticoresoftware/manticoresearch/issues/1920) Уменьшено ограничение скорости для [low_priority](Searching/Options.md#low_priority).
* [Issue #1924](https://github.com/manticoresoftware/manticoresearch/issues/1924) Ошибка mysqldump + восстановление mysql.
* [Issue #1951](https://github.com/manticoresoftware/manticoresearch/issues/1951) Исправлено некорректное создание распределённой таблицы в случае отсутствия локальной таблицы или неверного описания агента; теперь возвращается сообщение об ошибке.
* [Issue #1972](https://github.com/manticoresoftware/manticoresearch/issues/1972) Реализован счётчик `FREEZE` для избежания проблем с freeze/unfreeze.
* [Issue #1980](https://github.com/manticoresoftware/manticoresearch/issues/1980) Соблюдение таймаута запроса в OR узлах. Ранее `max_query_time` мог не работать в некоторых случаях.
* [Issue #1986](https://github.com/manticoresoftware/manticoresearch/issues/1986) Не удалось переименовать new в current [manticore.json].
* [Issue #1988](https://github.com/manticoresoftware/manticoresearch/issues/1988) Полнотекстовый запрос мог игнорировать подсказку CBO `SecondaryIndex`.
* [Issue #1990](https://github.com/manticoresoftware/manticoresearch/issues/1990) Исправлено `expansion_limit` для нарезки итогового набора результатов для ключевых слов вызова из нескольких дисковых или ОЗУ чанков.
* [Issue #1994](https://github.com/manticoresoftware/manticoresearch/issues/1994) неправильные внешние файлы.
* [Issue #2021](https://github.com/manticoresoftware/manticoresearch/issues/2021) Несколько процессов manticore-executor могли оставаться запущенными после остановки Manticore.
* [Issue #2029](https://github.com/manticoresoftware/manticoresearch/issues/2029) Сбой при использовании расстояния Левенштейна.
* [Issue #2037](https://github.com/manticoresoftware/manticoresearch/issues/2037) Ошибка после многократного выполнения max оператора на пустом индексе.
* [Issue #2052](https://github.com/manticoresoftware/manticoresearch/issues/2052) сбой на multi-group с JSON.field.
* [Issue #2067](https://github.com/manticoresoftware/manticoresearch/issues/2067) Manticore аварийно завершался при некорректном запросе к _update.
* [Issue #2069](https://github.com/manticoresoftware/manticoresearch/issues/2069) Исправлена проблема с компараторами строковых фильтров для закрытых диапазонов в JSON интерфейсе.
* [Issue #2082](https://github.com/manticoresoftware/manticoresearch/issues/2082) `alter` не срабатывал, если путь data_dir располагался на символической ссылке.
* [Issue #2102](https://github.com/manticoresoftware/manticoresearch/pull/2102) Улучшена специальная обработка SELECT запросов в mysqldump для обеспечения совместимости итоговых INSERT операторов с Manticore.
* [Issue #2103](https://github.com/manticoresoftware/manticoresearch/issues/2103) Тайские символы были в неправильных кодировках.
* [Issue #2124](https://github.com/manticoresoftware/manticoresearch/issues/2124) Сбой при использовании SQL с зарезервированным словом.
* [Issue #2154](https://github.com/manticoresoftware/manticoresearch/issues/2154) Таблицы с wordforms не могли быть импортированы.
* [Issue #2176](https://github.com/manticoresoftware/manticoresearch/issues/2176) Исправлен сбой, возникавший при установке параметра engine в 'columnar' и добавлении дубликатных ID через JSON.
* [Issue #223](https://github.com/manticoresoftware/manticoresearch-buddy/issues/223) Правильная ошибка при попытке вставить документ без схемы и без названий столбцов.
* [Issue #239](https://github.com/manticoresoftware/manticoresearch-buddy/issues/239) Автоматический multi-line insert по схеме мог не сработать.
* [Issue #399](https://github.com/manticoresoftware/manticoresearch/issues/399) Добавлено сообщение об ошибке при индексации, если атрибут id объявлен в источнике данных.
* [Issue #59](https://github.com/manticoresoftware/manticoresearch-helm/issues/59) Сбой кластера Manticore.
* [Issue #68](https://github.com/manticoresoftware/manticoresearch-helm/issues/68) optimize.php аварийно завершался, если была таблица percolate.
* [Issue #77](https://github.com/manticoresoftware/manticoresearch-helm/issues/77) Исправлены ошибки при деплое на Kubernetes.
* [Issue #274](https://github.com/manticoresoftware/manticoresearch-buddy/issues/274) Исправлена некорректная обработка одновременных запросов к Buddy.

### Связано с manticore-backup
* [Issue #97](https://github.com/manticoresoftware/manticoresearch-backup/pull/97) Установлен VIP HTTP порт по умолчанию, если доступен.
Различные улучшения: улучшена проверка версий и потоковое декомпрессирование ZSTD; добавлены запросы пользователя при несовпадении версий при восстановлении; исправлено некорректное поведение запроса при восстановлении из разных версий; улучшена логика декомпрессирования для чтения напрямую из потока, а не в оперативную память; добавлен флаг `--force`
* [Commit 3b35](https://github.com/manticoresoftware/manticoresearch-backup/commit/3b357ea) Добавлено отображение версии бэкапа после запуска Manticore search для выявления проблем на этом этапе.
* [Commit ad2e](https://github.com/manticoresoftware/manticoresearch-backup/commit/ad2e5da) Обновлено сообщение об ошибке при неудачных подключениях к демону.
* [Commit ce5e](https://github.com/manticoresoftware/manticoresearch-backup/commit/ce5ea85) Исправлена проблема с преобразованием абсолютных корневых путей бэкапа в относительные и удалена проверка права записи при восстановлении для возможности восстановления из разных путей.
* [Commit db7e](https://github.com/manticoresoftware/manticoresearch-backup/commit/db7e2b9) Добавлена сортировка итератора файлов для обеспечения консистентности в различных ситуациях.
* [Issue #106](https://github.com/manticoresoftware/manticoresearch-backup/issues/105) Резервное копирование и восстановление нескольких конфигураций.
* [Issue #91](https://github.com/manticoresoftware/manticoresearch-backup/pull/91) Добавлен defattr для предотвращения необычных прав пользователей в файлах после установки на RHEL.
* [Issue #91](https://github.com/manticoresoftware/manticoresearch-backup/pull/91) Добавлен дополнительный chown для установки файлов по умолчанию пользователю root в Ubuntu.

### Связано с MCL (columnar, secondary, knn libs)
* [Commit f104](https://github.com/manticoresoftware/columnar/commit/f104a4709bbb144140d09a09bc0fc1db1895e331) Поддержка векторного поиска.
* [Commit 2169](https://github.com/manticoresoftware/columnar/commit/216953beef6676f7e34c9f6f1a30c5980d746710) Исправлена очистка временных файлов при прерванной настройке сборки вторичного индекса. Это решает проблему, когда демон превышал лимит открытых файлов при создании файлов `tmp.spidx`.
* [Commit 709b](https://github.com/manticoresoftware/columnar/commit/709b9acaaac97d9a1ca8796892f9ad432021c785) Использование отдельной библиотеки streamvbyte для columnar и SI.
* [Commit 1c26](https://github.com/manticoresoftware/manticoresearch/commit/1c2623763ced14bb94ad624dde9614a938408d89) Добавлено предупреждение, что columnar storage не поддерживает json атрибуты.
* [Commit 3acd](https://github.com/manticoresoftware/columnar/commit/3acd6714ffce103447d7931dfc75bb1a7449f340) Исправлена распаковка данных в SI.
* [Commit 574c](https://github.com/manticoresoftware/manticoresearch/commit/574c023152564fa72092c18ebd1f594abfd4b6d7) Исправлен сбой при сохранении дискового чанка с смешанным хранением rowwise и columnar.
* [Commit e87f](https://github.com/manticoresoftware/columnar/commit/e87f088b458d4c5b1b6159fc72ed0d57b786891a) Исправлен итератор SI, на который указывали предыдущие обработанные блоки.
* [Issue #1474](https://github.com/manticoresoftware/manticoresearch/issues/1474) Обновление не работает для построчного MVA столбца с колоночным движком.
* [Issue #1510](https://github.com/manticoresoftware/manticoresearch/issues/1510) Исправлен сбой при агрегировании к колоночному атрибуту, используемому в `HAVING`.
* [Issue #1519](https://github.com/manticoresoftware/manticoresearch/issues/1519) Исправлен сбой в ранкере `expr` при использовании колоночного атрибута.

### Связано с Docker
* ❗[Issue #42](https://github.com/manticoresoftware/docker/issues/42) Поддержка [plain indexation](https://github.com/manticoresoftware/docker#building-plain-tables) через переменные окружения.
* ❗[Issue #47](https://github.com/manticoresoftware/docker/issues/47) Повышена гибкость конфигурации с помощью переменных окружения.
* [Issue #54](https://github.com/manticoresoftware/docker/pull/54) Улучшены процессы [резервного копирования и восстановления](https://github.com/manticoresoftware/docker#backup-and-restore) для Docker.
* [Issue #77](https://github.com/manticoresoftware/docker/issues/76) Улучшена точка входа для обработки восстановления резервных копий только при первом запуске.
* [Commit a27c](https://github.com/manticoresoftware/docker/commit/a27c048dd9e73d8d6bea8d9a4830cafb486b82c5) Исправлен лог запросов в stdout.
* [Issue #38](https://github.com/manticoresoftware/docker/issues/38) Заглушены предупреждения BUDDY, если EXTRA не установлена.
* [Issue #71](https://github.com/manticoresoftware/docker/pull/71) Исправлено имя хоста в `manticore.conf.sh`.

## Версия 6.2.12
Выпущена: 23 августа 2023

Версия 6.2.12 продолжает серию 6.2 и устраняет проблемы, обнаруженные после выхода 6.2.0.

### Исправления ошибок
* ❗[Issue #1351](https://github.com/manticoresoftware/manticoresearch/issues/1351) "Manticore 6.2.0 не запускается через systemctl на Centos 7": Изменено `TimeoutStartSec` с `infinity` на `0` для лучшей совместимости с Centos 7.
* ❗[Issue #1364](https://github.com/manticoresoftware/manticoresearch/issues/1364) "Сбой после обновления с 6.0.4 на 6.2.0": Добавлена возможность воспроизведения пустых binlog файлов из старых версий binlog.
* [PR #1334](https://github.com/manticoresoftware/manticoresearch/pull/1334) "исправление опечатки в searchdreplication.cpp": Исправлена опечатка в `searchdreplication.cpp`: beggining -> beginning.
* [Issue #1337](https://github.com/manticoresoftware/manticoresearch/issues/1337) "Manticore 6.2.0 WARNING: conn (local)(12), sock=8088: bailing on failed MySQL header, AsyncNetInputBuffer_c::AppendData: error 11 (Resource temporarily unavailable) return -1": Снижена уровень подробности предупреждения MySQL интерфейса о заголовке до logdebugv.
* [Issue #1355](https://github.com/manticoresoftware/manticoresearch/issues/1355) "join cluster зависает, когда не удается разрешить node_address": Улучшен повторный запрос на репликацию при недоступности некоторых узлов и ошибках разрешения имен. Это должно решить проблемы в узлах Kubernetes и Docker, связанные с репликацией. Улучшено сообщение об ошибке при неудаче запуска репликации и обновлена тестовая модель 376. Также добавлено четкое сообщение об ошибке при неудаче разрешения имени.
* [Issue #1361](https://github.com/manticoresoftware/manticoresearch/issues/1361) "Нет нижнего регистра для 'Ø' в charset non_cjk": Скорректировано отображение символа 'Ø'.
* [Issue #1365](https://github.com/manticoresoftware/manticoresearch/issues/1365) "searchd оставляет binlog.meta и binlog.001 после чистой остановки": Обеспечено правильное удаление последнего пустого binlog файла.
* [Commit 0871](https://github.com/manticoresoftware/manticoresearch/commit/0871070845e93ad702ae0dfb89b35e5f24cd0851): Исправлена проблема сборки `Thd_t` на Windows, связанная с ограничениями атомарного копирования.
* [Commit 1cc0](https://github.com/manticoresoftware/manticoresearch/commit/1cc04bedc174737118fd5a3663e35f4e95bd4f8c): Исправлена проблема с FT CBO и `ColumnarScan`.
* [Commit c6bf](https://github.com/manticoresoftware/manticoresearch/commit/c6bfb30d53e3f4b67f17769667733677a661c95b): Исправлены ошибки в тесте 376 и добавлена подстановка для ошибки `AF_INET` в тесте.
* [Commit cbc3](https://github.com/manticoresoftware/manticoresearch/commit/cbc38705e0b4dcc9665e3e8cf64826290ebd11cf): Решена проблема дедлока при репликации при обновлении blob-атрибутов против замены документов. Также убран rlock индекса при коммите, так как он уже заблокирован на более низком уровне.

### Незначительные изменения
* [Commit 4f91](https://github.com/manticoresoftware/manticoresearch/commit/4f913a36e84e01f950b2797051222b0b1094d6fe) Обновлена информация о `/bulk` эндпоинтах в документации.

### MCL
* Поддержка [Manticore Columnar Library](https://github.com/manticoresoftware/columnar) версии 2.2.4

## Версия 6.2.0
Выпущена: 4 августа 2023

### Основные изменения
* Оптимизатор запросов был усовершенствован для поддержки полнотекстовых запросов, значительно улучшая эффективность и производительность поиска.
* Интеграции с:
  - [mysqldump](https://dev.mysql.com/doc/refman/8.0/en/mysqldump.html) - для [создания логических резервных копий](Securing_and_compacting_a_table/Backup_and_restore.md#Backup-and-restore-with-mysqldump) с использованием `mysqldump`
  - [Apache Superset](https://manticoresearch.com/blog/manticoresearch-apache-superset-integration/) и [Grafana](https://manticoresearch.com/blog/manticoresearch-grafana-integration/) для визуализации данных, хранящихся в Manticore
  - [HeidiSQL](https://www.heidisql.com/) и [DBForge](https://www.devart.com/dbforge/) для облегчения разработки с Manticore
* Мы начали использовать [GitHub workflows](https://github.com/manticoresoftware/manticoresearch/actions), что упрощает для участников процесс использования той же системы непрерывной интеграции (CI), которую применяет основная команда при подготовке пакетов. Все задания могут выполняться на GitHub-хостах, что облегчает бесшовное тестирование изменений в вашей форк-версии Manticore Search.
* Мы начали использовать [CLT](https://github.com/manticoresoftware/clt) для тестирования сложных сценариев. Например, теперь мы можем гарантировать, что пакет, собранный после коммита, можно правильно установить на всех поддерживаемых Linux-операционных системах. Command Line Tester (CLT) предоставляет удобный способ записи тестов в интерактивном режиме и их безупречного воспроизведения.
* Значительное улучшение производительности операции count distinct за счет использования комбинации хеш-таблиц и HyperLogLog.
* Включено многопоточное выполнение запросов, содержащих вторичные индексы, при этом число потоков ограничено количеством физических ядер процессора. Это должно значительно повысить скорость выполнения запросов.
* `pseudo_sharding` был адаптирован так, чтобы ограничиваться количеством свободных потоков. Это обновление значительно улучшает пропускную способность.
* Теперь пользователи имеют возможность задавать [движок хранения атрибутов по умолчанию](Server_settings/Searchd.md#engine) через конфигурационные настройки, что обеспечивает лучшую настройку под конкретные требования к нагрузке.
* Поддержка [Manticore Columnar Library 2.2.0](https://github.com/manticoresoftware/columnar/) с многочисленными исправлениями ошибок и улучшениями в [вторичных индексах](Server_settings/Searchd.md#secondary_indexes).

### Незначительные изменения
* [Buddy #153](https://github.com/manticoresoftware/manticoresearch-buddy/issues/153): HTTP-эндпоинт [/pq](Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md?client=JSON#Adding-rules-to-a-percolate-table) теперь служит псевдонимом для HTTP-эндпоинта `/json/pq`.
* [Commit 0bf1](https://github.com/manticoresoftware/manticoresearch/commit/0bf17d9e): Обеспечена многобайтовая совместимость для функций `upper()` и `lower()`.
* [Commit 2bb9](https://github.com/manticoresoftware/manticoresearch/commit/2bb92765): Вместо сканирования индекса для запросов `count(*)` теперь возвращается заранее вычисленное значение.
* [Commit 3c84](https://github.com/manticoresoftware/manticoresearch/commit/3c84091f): Теперь можно использовать `SELECT` для выполнения произвольных вычислений и отображения `@@sysvars`. В отличие от прежнего поведения, теперь можно выполнять несколько вычислений. Таким образом, запросы вида `select user(), database(), @@version_comment, version(), 1+1 as a limit 10` вернут все столбцы. Обратите внимание, что необязательный параметр 'limit' всегда будет игнорироваться.
* [Commit 6aca](https://github.com/manticoresoftware/manticoresearch/commit/6aca32f0): Реализован заглушечный запрос `CREATE DATABASE`.
* [Commit 9dc1](https://github.com/manticoresoftware/manticoresearch/commit/9dc12334): При выполнении `ALTER TABLE table REBUILD SECONDARY` вторичные индексы теперь всегда пересоздаются, даже если атрибуты не обновлялись.
* [Commit 46ed](https://github.com/manticoresoftware/manticoresearch/commit/46edb089): Сортировщики, использующие заранее вычисленные данные, теперь определяются до использования CBO, чтобы избежать ненужных вычислений CBO.
* [Commit 102a](https://github.com/manticoresoftware/manticoresearch/commit/102ac604): Реализовано моделирование и использование стека полнотекстовых выражений для предотвращения сбоев демона.
* [Commit 979f](https://github.com/manticoresoftware/manticoresearch/commit/979fa27c): Добавлен быстрый путь для клонирования совпадений в случаях, когда совпадения не используют строковые/многозначные/json атрибуты.
* [Commit a073](https://github.com/manticoresoftware/manticoresearch/commit/a0735ffe): Добавлена поддержка команды `SELECT DATABASE()`. Однако она всегда будет возвращать `Manticore`. Это дополнение важно для интеграций с различными MySQL-инструментами.
* [Commit bc04](https://github.com/manticoresoftware/manticoresearch/commit/bc04908): Изменён формат ответа эндпоинта [/cli](Connecting_to_the_server/HTTP.md#/cli), добавлен эндпоинт `/cli_json`, выполняющий функции предыдущего `/cli`.
* [Commit d70b](https://github.com/manticoresoftware/manticoresearch/commit/d70b0d58): `thread_stack` теперь можно изменять во время выполнения с помощью оператора `SET`. Доступны как сессионные, так и демоночные варианты. Текущие значения доступны в выводе `show variables`.
* [Commit d96e](https://github.com/manticoresoftware/manticoresearch/commit/d96ec6b9): Код интегрирован в CBO для более точной оценки сложности выполнения фильтров по строковым атрибутам.
* [Commit e77d](https://github.com/manticoresoftware/manticoresearch/commit/e77dd72f5a04531c352fad0d7afcd2a1cbae2510): Улучшен расчет стоимости DocidIndex, что повышает общую производительность.
* [Commit f3ae](https://github.com/manticoresoftware/manticoresearch/commit/f3ae8bea): Метрики нагрузки, аналогичные 'uptime' в Linux, теперь отображаются в команде `SHOW STATUS`.
* [Commit f3cc](https://github.com/manticoresoftware/manticoresearch/commit/f3cc0971): Порядок полей и атрибутов для `DESC` и `SHOW CREATE TABLE` теперь совпадает с порядком `SELECT * FROM`.
* [Commit f3d2](https://github.com/manticoresoftware/manticoresearch/commit/f3d248a6): Разные внутренние парсеры теперь при различных ошибках сообщают свой внутренний мнемонический код (например, `P01`). Это улучшение помогает определить, какой парсер вызвал ошибку, и скрывает несущественные внутренние детали.
* [Issue #271](https://github.com/manticoresoftware/manticoresearch/issues/271) "Иногда CALL SUGGEST не предлагает исправление опечатки из одного символа": улучшено поведение [SUGGEST/QSUGGEST](Searching/Spell_correction.md#CALL-QSUGGEST,-CALL-SUGGEST) для коротких слов: добавлена опция `sentence` для показа всего предложения
* [Issue #696](https://github.com/manticoresoftware/manticoresearch/issues/696) "Percolate индекс не ищет корректно точные фразовые запросы при включенном стемминге": изменен запрос percolate для обработки модификатора точного термина, что улучшает функцию поиска.
* [Issue #829](https://github.com/manticoresoftware/manticoresearch/issues/829) "Методы форматирования даты": добавлено выражение выборочного списка [date_format()](Functions/Date_and_time_functions.md#DATE_FORMAT()), раскрывающее функцию `strftime()`.
* [Issue #961](https://github.com/manticoresoftware/manticoresearch/issues/961) "Сортировка бакетов через HTTP JSON API": добавлено необязательное [свойство sort](Searching/Faceted_search.md#HTTP-JSON) для каждого бакета агрегатов в HTTP-интерфейсе.
* [Issue #1062](https://github.com/manticoresoftware/manticoresearch/issues/1062) "Улучшение логирования ошибок при сбое JSON insert api - "unsupported value type"": эндпоинт `/bulk` теперь сообщает информацию о количестве обработанных и необработанных строк (документов) в случае ошибки.
* [Issue #1070](https://github.com/manticoresoftware/manticoresearch/issues/1070) "CBO hints не поддерживают несколько атрибутов": включена поддержка подсказок индекса для работы с несколькими атрибутами.
* [Issue #1106](https://github.com/manticoresoftware/manticoresearch/issues/1106) "Добавить теги к http поисковому запросу": теги были добавлены в [HTTP PQ ответы](Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md?client=JSON#Adding-rules-to-a-percolate-table).
* [Issue #1301](https://github.com/manticoresoftware/manticoresearch/issues/1301) "buddy не должен создавать таблицу параллельно": исправлена проблема, вызывающая сбои при параллельных операциях CREATE TABLE. Теперь одновременно может выполняться только одна операция `CREATE TABLE`.
* [Issue #1303](https://github.com/manticoresoftware/manticoresearch/issues/1303) "добавлена поддержка @ в именах колонок".
* [Issue #1316](https://github.com/manticoresoftware/manticoresearch/issues/1316) "Запросы к датасету такси медленные с ps=1": логика CBO была усовершенствована, и разрешение гистограммы по умолчанию установлено в 8k для большей точности на атрибутах с случайно распределёнными значениями.
* [Issue #1317](https://github.com/manticoresoftware/manticoresearch/issues/1317) "Исправление CBO против полнотекстового поиска на hn датасете": улучшена логика определения момента использования пересечения итераторов bitmap и момента использования приоритетной очереди.
* [Issue #1318](https://github.com/manticoresoftware/manticoresearch/issues/1318) "columnar: изменение интерфейса итератора на одно-вызовный": колоннарные итераторы теперь используют один вызов `Get`, заменив прежние два шага `AdvanceTo` + `Get` для получения значения.
* [Issue #1319](https://github.com/manticoresoftware/manticoresearch/issues/1319) "Ускорение расчёта агрегатов (удаление CheckReplaceEntry?)": вызов `CheckReplaceEntry` был удалён из сортировщика групп для ускорения вычисления агрегатных функций.
* [Issue #1320](https://github.com/manticoresoftware/manticoresearch/issues/1320) "create table read_buffer_docs/hits не понимает синтаксис k/m/g": параметры `CREATE TABLE` `read_buffer_docs` и `read_buffer_hits` теперь поддерживают синтаксис k/m/g.
* [Language packs](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) для английского, немецкого и русского языков теперь можно легко установить на Linux с помощью команды `apt/yum install manticore-language-packs`. На macOS используйте команду `brew install manticoresoftware/tap/manticore-language-packs`.
* Порядок полей и атрибутов теперь согласован между операциями `SHOW CREATE TABLE` и `DESC`.
* Если недостаточно места на диске при выполнении запросов `INSERT`, новые запросы `INSERT` будут завершаться с ошибкой до освобождения достаточного объёма места.
* Добавлена функция преобразования типов [UINT64()](Functions/Type_casting_functions.md#UINT64%28%29).
* Эндпоинт `/bulk` теперь обрабатывает пустые строки как команду [commit](Data_creation_and_modification/Transactions.md#BEGIN,-COMMIT,-and-ROLLBACK). Подробнее [здесь](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md?client=JSON#Bulk-adding-documents).
* Реализованы предупреждения для [неверных подсказок индекса](Searching/Options.md#Query-optimizer-hints), повышающие прозрачность и позволяющие уменьшить количество ошибок.
* При использовании `count(*)` с одним фильтром запросы теперь используют предварительно рассчитанные данные из вторичных индексов, когда они доступны, что значительно ускоряет выполнение запросов.

### ⚠️ Изменения, нарушающие обратную совместимость
* ⚠️ Таблицы, созданные или изменённые в версии 6.2.0, не могут быть прочитаны более старыми версиями.
* ⚠️ Идентификаторы документов теперь обрабатываются как беззнаковые 64-битные числа при индексировании и операциях INSERT.
* ⚠️ Синтаксис подсказок оптимизатора запросов был обновлён. Новый формат: `/*+ SecondaryIndex(uid) */`. Обратите внимание, что старый синтаксис больше не поддерживается.
* ⚠️ [Issue #1160](https://github.com/manticoresoftware/manticoresearch/issues/1160): использование символа `@` в именах таблиц запрещено для предотвращения конфликтов синтаксиса.
* ⚠️ Строковые поля/атрибуты, помеченные как `indexed` и `attribute`, теперь рассматриваются как единое поле при операциях `INSERT`, `DESC` и `ALTER`.
* ⚠️ [Issue #1057](https://github.com/manticoresoftware/manticoresearch/issues/1057): библиотеки MCL больше не будут загружаться на системах без поддержки SSE 4.2.
* ⚠️ [Issue #1143](https://github.com/manticoresoftware/manticoresearch/issues/1143): параметр [agent_query_timeout](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_query_timeout) был сломан. Исправлен и теперь работает.

### Исправления ошибок
* [Commit 2a6e](https://github.com/manticoresoftware/manticoresearch/commit/2a6ea8f7) "Сбой при DROP TABLE": решена проблема, вызывавшая длительное ожидание завершения операций записи (оптимизация, сохранение чанка на диске) в таблице RT при выполнении DROP TABLE. Добавлено предупреждение при ненулевой директории таблицы после DROP TABLE.
* [Commit 2ebd](https://github.com/manticoresoftware/manticoresearch/commit/2ebd424d): добавлена поддержка колоннарных атрибутов, отсутствовавшая в коде для группировки по нескольким атрибутам.
* [Commit 3be4](https://github.com/manticoresoftware/manticoresearch/commit/3be4503f752216b37b8cde8e47fd2d77bbce63ae) Исправлен сбой, потенциально вызванный исчерпанием дискового пространства, за счёт корректной обработки ошибок записи в binlog.
* [Commit 6adb](https://github.com/manticoresoftware/manticoresearch/commit/6adb0934): Исправлена ошибка, вызывавшая периодические сбои при использовании нескольких итераторов колонного сканирования (или итераторов вторичного индекса) в запросе.
* [Commit 6bd9](https://github.com/manticoresoftware/manticoresearch/commit/6bd9f709): Фильтры не удалялись при использовании сортировщиков, которые используют предварительно рассчитанные данные. Эта проблема была исправлена.
* [Commit 6d03](https://github.com/manticoresoftware/manticoresearch/commit/6d03566a): Код CBO был обновлен для предоставления лучших оценок для запросов, использующих фильтры поверх построчных атрибутов, выполняемых в нескольких потоках.
* [Commit 6dd3](https://github.com/manticoresoftware/manticoresearch/commit/6dd3964), [Helm #56](https://github.com/manticoresoftware/manticoresearch-helm/issues/56) "фатальный дамп аварийного завершения в кластере Kubernetes": Исправлен дефектный фильтр Блума для корневого JSON-объекта; исправлен сбой демона из-за фильтрации по полю JSON.
* [Commit 6e1b](https://github.com/manticoresoftware/manticoresearch/commit/6e1b849) Исправлен сбой демона, вызванный неверной конфигурацией `manticore.json`.
* [Commit 6fbc](https://github.com/manticoresoftware/manticoresearch/commit/6fbc4189) Исправлен фильтр диапазона JSON для поддержки значений int64.
* [Commit 9c67](https://github.com/manticoresoftware/manticoresearch/commit/9c677426) Файлы `.sph` могли быть повреждены при выполнении `ALTER`. Исправлено.
* [Commit 77cc](https://github.com/manticoresoftware/manticoresearch/commit/77ccd8f): Добавлен общий ключ для репликации оператора replace для устранения ошибки `pre_commit`, возникающей при репликации replace с нескольких мастер-узлов.
* [Commit 2884](https://github.com/manticoresoftware/manticoresearch/commit/2884519e) решены проблемы с проверками bigint в функциях, таких как 'date_format()'.
* [Commit 9513](https://github.com/manticoresoftware/manticoresearch/commit/9513aca5): Итераторы больше не отображаются в [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META), когда сортировщики используют предварительно рассчитанные данные.
* [Commit a2a7](https://github.com/manticoresoftware/manticoresearch/commit/a2a70555): Обновлен размер стека узла полнотекстового поиска для предотвращения сбоев при выполнении сложных полнотекстовых запросов.
* [Commit a062](https://github.com/manticoresoftware/manticoresearch/commit/a0626d7e): Исправлена ошибка, вызывавшая сбой при репликации обновлений с атрибутами JSON и строк.
* [Commit b3e6](https://github.com/manticoresoftware/manticoresearch/commit/b3e63e5d): Обновлен конструктор строк для использования 64-битных целых, чтобы избежать сбоев при работе с большими объемами данных.
* [Commit c472](https://github.com/manticoresoftware/manticoresearch/commit/c472e5b): Исправлен сбой, возникавший при подсчете уникальных значений across нескольких индексах.
* [Commit d073](https://github.com/manticoresoftware/manticoresearch/commit/d0730272): Исправлена проблема, при которой запросы по дисковым чанкам RT-индексов могли выполняться в нескольких потоках, даже если `pseudo_sharding` был отключен.
* [Commit d205](https://github.com/manticoresoftware/manticoresearch/commit/d205508e) Изменен набор значений, возвращаемых командой `show index status`, теперь он варьируется в зависимости от типа используемого индекса.
* [Commit e9bc](https://github.com/manticoresoftware/manticoresearch/commit/e9bc2f9) Исправлена ошибка HTTP при обработке массовых запросов и проблема, при которой ошибка не возвращалась клиенту из сетевого цикла.
* [Commit f77c](https://github.com/manticoresoftware/manticoresearch/commit/f77ce0e6) Использование расширенного стека для PQ.
* [Commit fac2](https://github.com/manticoresoftware/manticoresearch/commit/fac2d9687a20582765aaa89be9597804c4f845af) Обновлен вывод ранжировщика экспорта для соответствия [packedfactors()](Functions/Searching_and_ranking_functions.md#PACKEDFACTORS%28%29).
* [Commit ff87](https://github.com/manticoresoftware/manticoresearch/commit/ff87cc4): Исправлена проблема со строковым списком в фильтре журнала запросов SphinxQL.
* [Issue #589](https://github.com/manticoresoftware/manticoresearch/issues/589) "Определение charset, кажется, зависит от порядка кодов": Исправлено некорректное сопоставление наборов символов для дубликатов.
* [Issue #811](https://github.com/manticoresoftware/manticoresearch/issues/811) "Отображение нескольких слов в формах слов мешает поиску фраз с пунктуацией CJK между ключевыми словами": Исправлена позиция токенов ngram в запросе фразы с формами слов.
* [Issue #834](https://github.com/manticoresoftware/manticoresearch/issues/834) "Знак равно в поисковом запросе ломает запрос": Обеспечена возможность экранировать точный символ и исправлено двойное точное расширение опцией `expand_keywords`.
* [Issue #864](https://github.com/manticoresoftware/manticoresearch/issues/864) "Конфликт exceptions/stopwords"
* [Issue #910](https://github.com/manticoresoftware/manticoresearch/issues/910) "Сбой Manticore при вызове call snippets() с libstemmer_fr и index_exact_words": Решены внутренние конфликты, вызывавшие сбои при вызове `SNIPPETS()`.
* [Issue #946](https://github.com/manticoresoftware/manticoresearch/issues/946) "Дублирующиеся записи при SELECT": Исправлена проблема дублирования документов в результатах запроса с опцией `not_terms_only_allowed` для RT-индекса с удаленными документами.
* [Issue #967](https://github.com/manticoresoftware/manticoresearch/issues/967) "Использование JSON аргументов в UDF функциях приводит к сбою": Исправлен сбой демона при обработке поиска с включенным псевдо-шардингом и UDF с аргументом JSON.
* [Issue #1050](https://github.com/manticoresoftware/manticoresearch/issues/1050) "count(*) в FEDERATED": Исправлен сбой демона, возникающий при запросе через движок `FEDERATED` с агрегатом.
* [Issue #1052](https://github.com/manticoresoftware/manticoresearch/issues/1052) Исправлена проблема, при которой колонка `rt_attr_json` была несовместима с колонным хранением.
* [Issue #1072](https://github.com/manticoresoftware/manticoresearch/issues/1072) "* удаляется из поискового запроса опцией ignore_chars": Исправлена проблема, чтобы wildcard в запросе не влияли на `ignore_chars`.
* [Issue #1075](https://github.com/manticoresoftware/manticoresearch/issues/1075) "indextool --check fails if there's a distributed table": indextool теперь совместим с инстансами, имеющими индексы 'distributed' и 'template' в json конфигурации.
* [Issue #1081](https://github.com/manticoresoftware/manticoresearch/issues/1081) "particular select on particular RT dataset leads to crash of searchd": Исправлен сбой демона при запросе с packedfactors и большим внутренним буфером.
* [Issue #1095](https://github.com/manticoresoftware/manticoresearch/issues/1095) "With not_terms_only_allowed deleted documents are ignored"
* [Issue #1099](https://github.com/manticoresoftware/manticoresearch/issues/1099) "indextool --dumpdocids is not working": Восстановлена функциональность команды `--dumpdocids`.
* [Issue #1100](https://github.com/manticoresoftware/manticoresearch/issues/1100) "indextool --buildidf is not working": indextool теперь закрывает файл после завершения globalidf.
* [Issue #1104](https://github.com/manticoresoftware/manticoresearch/issues/1104) "Count(*) is trying to be treated as schema set in remote tables": Исправлена ошибка, при которой демон отправлял сообщение об ошибке при запросах к распределённому индексу, если агент возвращал пустой набор результатов.
* [Issue #1109](https://github.com/manticoresoftware/manticoresearch/issues/1109) "FLUSH ATTRIBUTES hangs with threads=1".
* [Issue #1126](https://github.com/manticoresoftware/manticoresearch/issues/1126) "Lost connection to MySQL server during query - manticore 6.0.5": Устранены сбои, происходившие при использовании нескольких фильтров на колоннарных атрибутах.
* [Issue #1135](https://github.com/manticoresoftware/manticoresearch/issues/1135) "JSON string filtering case sensitivity": Исправлена сортировка для правильной работы фильтров в HTTP поисковых запросах.
* [Issue #1140](https://github.com/manticoresoftware/manticoresearch/issues/1140) "Match in a wrong field": Исправлены повреждения, связанные с `morphology_skip_fields`.
* [Issue #1155](https://github.com/manticoresoftware/manticoresearch/issues/1155) "system remote commands via API should pass g_iMaxPacketSize": Внесены изменения, чтобы обходить проверку `max_packet_size` для команд репликации между узлами. Кроме того, последнее кластерное сообщение об ошибке добавлено в отображение статуса.
* [Issue #1302](https://github.com/manticoresoftware/manticoresearch/issues/1302) "tmp files left on failed optimize": Исправлена ошибка, из-за которой временные файлы оставались после ошибки во время процесса слияния или оптимизации.
* [Issue #1304](https://github.com/manticoresoftware/manticoresearch/issues/1304) "add env var for buddy start timeout": Добавлена переменная окружения `MANTICORE_BUDDY_TIMEOUT` (по умолчанию 3 секунды) для контроля времени ожидания сообщения buddy при запуске демона.
* [Issue #1305](https://github.com/manticoresoftware/manticoresearch/issues/1305) "Int overflow when saving PQ meta": Снижено чрезмерное потребление памяти демоном при сохранении большого PQ индекса.
* [Issue #1306](https://github.com/manticoresoftware/manticoresearch/issues/1306) "Can't recreate RT table after altering its external file": Исправлена ошибка alter с пустой строкой в поле внешних файлов; исправлены внешние файлы индекса RT, оставшиеся после изменения внешних файлов.
* [Issue #1307](https://github.com/manticoresoftware/manticoresearch/issues/1307) "SELECT statement sum(value) as value doesn't work properly": Исправлена ошибка, когда выражение списка select с псевдонимом могло скрывать атрибут индекса; также исправлен sum для подсчёта в int64 для целых чисел.
* [Issue #1308](https://github.com/manticoresoftware/manticoresearch/issues/1308) "Avoid binding to localhost in replication": Обеспечена недопустимость привязки репликации к localhost для имен хостов с несколькими IP.
* [Issue #1309](https://github.com/manticoresoftware/manticoresearch/issues/1309) "reply to mysql client failed for data larger 16Mb": Исправлена проблема с возвращением пакета SphinxQL клиенту, если размер превышал 16 Мб.
* [Issue #1310](https://github.com/manticoresoftware/manticoresearch/issues/1310) "wrong reference in "paths to external files should be absolute": Исправлено отображение полного пути к внешним файлам в `SHOW CREATE TABLE`.
* [Issue #1311](https://github.com/manticoresoftware/manticoresearch/issues/1311) "debug build crashes on long strings in snippets": Теперь в функции `SNIPPET()` разрешены длинные строки (>255 символов).
* [Issue #1312](https://github.com/manticoresoftware/manticoresearch/issues/1312) "spurious crash on use-after-delete in kqueue polling (master-agent)": Исправлены сбои, возникавшие при невозможности мастера подключиться к агенту на системах с kqueue (FreeBSD, MacOS и др.).
* [Issue #1313](https://github.com/manticoresoftware/manticoresearch/issues/1313) "too long connect to itself": При подключении мастера к агентам на MacOS/BSD теперь применяется единый тайм-аут на соединение и запрос вместо только соединения.
* [Issue #1314](https://github.com/manticoresoftware/manticoresearch/issues/1314) "pq (json meta) with unreached embedded synonyms fails to load": Исправлен флаг встроенных синонимов в pq.
* [Issue #1315](https://github.com/manticoresoftware/manticoresearch/issues/1315) "Allow some functions (sint, fibonacci, second, minute, hour, day, month, year, yearmonth, yearmonthday) to use implicitly promoted argument values".
* [Issue #1321](https://github.com/manticoresoftware/manticoresearch/issues/1321) "Enable multithreaded SI in fullscan, but limit threads": В CBO реализован код для лучшего прогнозирования многопоточной производительности вторичных индексов при использовании в полнотекстовом запросе.
* [Issue #1322](https://github.com/manticoresoftware/manticoresearch/issues/1322) "count(*) queries still slow after using precalc sorters": Итераторы теперь не инициализируются при использовании сортировщиков с предварительно вычисленными данными, что предотвращает ухудшение производительности.
* [Issue #1411](https://github.com/manticoresoftware/manticoresearch/issues/1141) "query log in sphinxql does not preserve original queries for MVA's": Теперь логируются `all()/any()`.

## Версия 6.0.4
Выпущена: 15 марта 2023

### Новые возможности
* Улучшена интеграция с Logstash, Beats и т.п., включая:
  - Поддержка версий Logstash 7.6 - 7.15, версий Filebeat 7.7 - 7.12
  - Поддержка авто-схемы.
  - Добавлена обработка массовых запросов в формате, похожем на Elasticsearch.
* [Buddy commit ce90](https://github.com/manticoresoftware/manticoresearch-buddy/commit/ce907ea) Версия Log Buddy при запуске Manticore.

### Исправления ошибок
* [Issue #588](https://github.com/manticoresoftware/manticoresearch/issues/588), [Issue #942](https://github.com/manticoresoftware/manticoresearch/issues/942) Исправлен неверный символ в мета-данных поиска и вызов ключевых слов для биграммного индекса.
* [Issue #1027](https://github.com/manticoresoftware/manticoresearch/issues/1027) Отклонены HTTP заголовки с маленькими буквами.
* ❗[Issue #1039](https://github.com/manticoresoftware/manticoresearch/issues/1039) Исправлена утечка памяти в демоне при чтении вывода консоли Buddy.
* [Issue #1056](https://github.com/manticoresoftware/manticoresearch/issues/1056) Исправлено неожиданное поведение вопросительного знака.
* [Issue #1064](https://github.com/manticoresoftware/manticoresearch/issues/1064) - Исправлено состояние гонки в таблицах нижнего регистра токенизатора, вызывающее сбой.
* [Commit 59bb](https://github.com/manticoresoftware/manticoresearch/commit/59bb54c) Исправлена обработка массовых записей в JSON-интерфейсе для документов с id явно установленным в null.
* [Commit 7b6b](https://github.com/manticoresoftware/manticoresearch/commit/7b6b25f) Исправлена статистика терминов в CALL KEYWORDS для нескольких одинаковых терминов.
* [Commit f381](https://github.com/manticoresoftware/manticoresearch/commit/f381ad2) Конфигурация по умолчанию теперь создаётся установщиком Windows; пути больше не подставляются во время выполнения.
* [Commit 6940](https://github.com/manticoresoftware/manticoresearch/commit/6940e95), [Commit cc5a](https://github.com/manticoresoftware/manticoresearch/commit/cc5a480) Исправлены проблемы репликации для кластера с узлами в нескольких сетях.
* [Commit 4972](https://github.com/manticoresoftware/manticoresearch/commit/49722ab) Исправлен HTTP эндпоинт `/pq` для работы как alias эндпоинта `/json/pq`.
* [Commit 3b53](https://github.com/manticoresoftware/manticoresearch/commit/3b5385a) Исправлен сбой демона при перезапуске Buddy.
* [Buddy commit fba9](https://github.com/manticoresoftware/manticoresearch-buddy/commit/fba9c8c) Отображение исходной ошибки при получении некорректного запроса.
* [Buddy commit db95](https://github.com/manticoresoftware/manticoresearch-buddy/commit/db9532c) Разрешены пробелы в пути для резервного копирования и добавлена поддержка одинарных кавычек с помощью магии в регулярном выражении.

## Версия 6.0.2
Выпущена: 10 февраля 2023

### Исправления ошибок
* [Issue #1024 crash 2](https://github.com/manticoresoftware/manticoresearch/issues/1024) Сбой / ошибка сегментации при поиске с фасетами с большим количеством результатов
* ❗[Issue #1029](https://github.com/manticoresoftware/manticoresearch/issues/1029) - ВНИМАНИЕ: Значение KNOWN_CREATE_SIZE, зафиксированное на этапе компиляции (16), меньше измеренного (208). Рассмотрите возможность исправления значения!
* ❗[Issue #1032](https://github.com/manticoresoftware/manticoresearch/issues/1032) - Сбой plain индекса Manticore 6.0.0
* ❗[Issue #1033](https://github.com/manticoresoftware/manticoresearch/issues/1033) - множественные потерянные распределённые индексы при перезапуске демона
* ❗[Issue #1064](https://github.com/manticoresoftware/manticoresearch/issues/1064) - состояние гонки в таблицах токенизатора с нижним регистром

## Версия 6.0.0
Выпущена: 7 февраля 2023

Начиная с этого выпуска, Manticore Search поставляется вместе с Manticore Buddy — сайдкар-демоном, написанным на PHP, который обрабатывает функциональность высокого уровня, не требующую супернизкой задержки или высокой пропускной способности. Manticore Buddy работает в фоновом режиме, и вы можете даже не заметить его запуск. Несмотря на то, что он невидим для конечного пользователя, создание легко устанавливаемого и совместимого с основным C++-демоном Manticore Buddy было значительной задачей. Это крупное изменение позволит команде разрабатывать широкий спектр новых функций высокого уровня, таких как оркестрация шардов, контроль доступа и аутентификация, а также различные интеграции, например mysqldump, DBeaver, mysql-коннектор для Grafana. Уже сейчас Buddy обрабатывает [SHOW QUERIES](Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES), [BACKUP](Securing_and_compacting_a_table/Backup_and_restore.md#BACKUP-SQL-command-reference) и [Auto schema](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema).

В этот релиз также включено более 130 исправлений ошибок и множество новых возможностей, многие из которых можно считать значительными.

### Основные изменения
* 🔬 Экспериментально: теперь можно выполнять совместимые с Elasticsearch [insert](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md?example=Elasticsearch#Adding-documents-to-a-real-time-table) и [replace](Data_creation_and_modification/Updating_documents/REPLACE.md?example=Elasticsearch#REPLACE) JSON-запросы, что позволяет использовать Manticore с такими инструментами, как Logstash (версия < 7.13), Filebeat и другими инструментами из семьи Beats. Включено по умолчанию. Можно отключить с помощью `SET GLOBAL ES_COMPAT=off`.
* Поддержка [Manticore Columnar Library 2.0.0](https://github.com/manticoresoftware/columnar/) с множеством исправлений и улучшений в [Вторичных индексах](Server_settings/Searchd.md#secondary_indexes). **⚠️ ЛОМАЮЩЕЕ ИЗМЕНЕНИЕ**: Вторичные индексы по умолчанию ВКЛЮЧЕНЫ с этого выпуска. Обязательно выполните [ALTER TABLE table_name REBUILD SECONDARY](Updating_table_schema_and_settings.md#Rebuilding-a-secondary-index), если вы обновляетесь с Manticore 5. Подробнее см. ниже.
* [Коммит c436](https://github.com/manticoresoftware/manticoresearch/commit/c436f9023536f767610451911955ae36d90aa638) Авто-схема: теперь можно пропустить создание таблицы, просто вставьте первый документ, и Manticore автоматически создаст таблицу на основе его полей. Подробнее об этом можно прочитать [здесь](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema). Ее можно включать/выключать с помощью [searchd.auto_schema](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema).
* Значительное обновление [оптимизатора на основе затрат (cost-based optimizer)](Searching/Cost_based_optimizer.md), которое во многих случаях сокращает время отклика запросов.
  - [Issue #1008](https://github.com/manticoresoftware/manticoresearch/issues/1008) Оценка производительности параллелизации в CBO.
  - [Issue #1014](https://github.com/manticoresoftware/manticoresearch/issues/1014) CBO теперь учитывает [вторичные индексы](Server_settings/Searchd.md#secondary_indexes) и может действовать умнее.
  - [Коммит cef9](https://github.com/manticoresoftware/columnar/commit/cef9b54dc3a4ee9138ebc4df3a671715b996d6b3) Статистика кодирования столбцовых таблиц/полей теперь хранится в метаданных, чтобы помочь CBO принимать более обоснованные решения.
  - [Коммит 2b95](https://github.com/manticoresoftware/manticoresearch/commit/2b95fb8c2ba422c9c5a66e2b61c1f7922f31c7aa) Добавлены подсказки для CBO для тонкой настройки его поведения.
* [Телеметрия](Telemetry.md#Telemetry): с радостью сообщаем о добавлении телеметрии в этом релизе. Эта функция позволяет собирать анонимные и деперсонализированные метрики, которые помогут улучшить производительность и удобство использования нашего продукта. Будьте уверены, что все собираемые данные **абсолютно анонимны и не будут связаны с личной информацией**. Эту функцию можно [легко отключить](Telemetry.md#Telemetry) в настройках при необходимости.
* [Коммит 5aaf](https://github.com/manticoresoftware/manticoresearch/commit/5aafb17d81b601e399e212a344a4738f65576cdd) [ALTER TABLE table_name REBUILD SECONDARY](Updating_table_schema_and_settings.md#Rebuilding-a-secondary-index) для перестройки вторичных индексов в любое время, например:
  - при миграции с Manticore 5 на новую версию,
  - если вы сделали [UPDATE](Data_creation_and_modification/Updating_documents/UPDATE.md#UPDATE) (т.е. [обновление на месте, а не замену](Data_creation_and_modification/Updating_documents/REPLACE_vs_UPDATE.md)) атрибута в индексе
* [Issue #821](https://github.com/manticoresoftware/manticoresearch/issues/821) Новый инструмент `manticore-backup` для [резервного копирования и восстановления экземпляра Manticore](Securing_and_compacting_a_table/Backup_and_restore.md)
* SQL-команда [BACKUP](Securing_and_compacting_a_table/Backup_and_restore.md#BACKUP-SQL-command-reference) для создания резервных копий изнутри Manticore.
* SQL-команда [SHOW QUERIES](Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES) как простой способ просмотра выполняющихся запросов вместо потоков.
* [Issue #551](https://github.com/manticoresoftware/manticoresearch/issues/551) SQL-команда `KILL` для прерывания долгих `SELECT`.
* Динамическое значение `max_matches` для агрегатных запросов для повышения точности и снижения времени отклика.

### Мелкие изменения
* [Issue #822](https://github.com/manticoresoftware/manticoresearch/issues/822) SQL-команды [FREEZE/UNFREEZE](Securing_and_compacting_a_table/Freezing_a_table.md) для подготовки real-time/plain таблицы к резервному копированию.
* [Коммит c470](https://github.com/manticoresoftware/manticoresearch/commit/c47052f18ad4868134de1a97e8d7da570e531dfa) Новые параметры `accurate_aggregation` и `max_matches_increase_threshold` для контролируемой точности агрегации.
* [Issue #718](https://github.com/manticoresoftware/manticoresearch/issues/718) Поддержка 64-битных знаковых отрицательных ID. Обратите внимание, что вы все еще не можете использовать ID больше 2^63, но теперь доступны ID в диапазоне от -2^63 до 0.
* Поскольку недавно была добавлена поддержка вторичных индексов, возникло путаница, поскольку "index" мог означать вторичный индекс, полнотекстовый индекс или простой/real-time `index`. Чтобы уменьшить путаницу, мы переименовываем последний в "table". На следующие SQL/команды командной строки повлияло это изменение. Старые версии устарели, но продолжают работать:
  - `index <имя таблицы>` => `table <имя таблицы>`,
  - `searchd -i / --index` => `searchd -t / --table`,
  - `SHOW INDEX STATUS` => `SHOW TABLE STATUS`,
  - `SHOW INDEX SETTINGS` => `SHOW TABLE SETTINGS`,
  - `FLUSH RTINDEX` => `FLUSH TABLE`,
  - `OPTIMIZE INDEX` => `OPTIMIZE TABLE`,
  - `ATTACH TABLE plain TO RTINDEX rt` => `ATTACH TABLE plain TO TABLE rt`,
  - `RELOAD INDEX` => `RELOAD TABLE`,
  - `RELOAD INDEXES` => `RELOAD TABLES`.

  Мы не планируем делать старые формы недействительными, но для совместимости с документацией рекомендуем изменить имена в вашем приложении. Что будет изменено в будущих релизах — это переименование "index" в "table" в выводе различных SQL и JSON-команд.
* Запросы с состоянием UDF теперь выполняются принудительно в одном потоке.
* [Issue #1011](https://github.com/manticoresoftware/manticoresearch/issues/1011) Рефакторинг всего, что связано с планированием по времени, как предпосылка для параллельного слияния чанков.
* **⚠️ ПРЕРЫВАЮЩЕЕ ИЗМЕНЕНИЕ**: Формат хранения столбцовых данных изменен. Нужно перестроить таблицы, у которых есть столбцовые атрибуты.
* **⚠️ ПРЕРЫВАЮЩЕЕ ИЗМЕНЕНИЕ**: Формат файла вторичных индексов изменен, поэтому если вы используете вторичные индексы для поиска и у вас в конфигурационном файле стоит `searchd.secondary_indexes = 1`, имейте в виду, что новая версия Manticore **пропустит загрузку таблиц с вторичными индексами**. Рекомендуется:
  - Перед обновлением изменить `searchd.secondary_indexes` на 0 в конфигурационном файле.
  - Запустить экземпляр. Manticore загрузит таблицы с предупреждением.
  - Выполнить `ALTER TABLE <table name> REBUILD SECONDARY` для каждого индекса, чтобы перестроить вторичные индексы.

  Если вы запускаете кластер репликации, вам нужно выполнить `ALTER TABLE <table name> REBUILD SECONDARY` на всех узлах или следовать [этой инструкции](Securing_and_compacting_a_table/Compacting_a_table.md#Optimizing-clustered-tables) с единственным изменением: выполнить `ALTER .. REBUILD SECONDARY` вместо `OPTIMIZE`.
* **⚠️ КРУПНОЕ ИЗМЕНЕНИЕ**: Версия binlog обновлена, поэтому binlog из предыдущих версий не будет воспроизводиться. Важно убедиться, что Manticore Search корректно остановлен во время процесса обновления. Это означает, что в `/var/lib/manticore/binlog/` после остановки предыдущего экземпляра не должно быть binlog файлов, кроме `binlog.meta`.
* [Issue #849](https://github.com/manticoresoftware/manticoresearch/issues/849) `SHOW SETTINGS`: теперь можно увидеть настройки из конфигурационного файла внутри Manticore.
* [Issue #1007](https://github.com/manticoresoftware/manticoresearch/issues/1007) [SET GLOBAL CPUSTATS=1/0](Server_settings/Setting_variables_online.md#SET) включает/выключает отслеживание времени ЦП; [SHOW THREADS](Node_info_and_management/SHOW_THREADS.md) теперь не показывает статистику CPU, когда отслеживание времени ЦП выключено.
* [Issue #1009](https://github.com/manticoresoftware/manticoresearch/issues/1009) Сегменты RAM чанков в RT таблицах теперь могут объединяться, пока RAM чанк сбрасывается.
* [Issue #1012](https://github.com/manticoresoftware/manticoresearch/issues/1012) В выходные данные [indexer](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) добавлен прогресс вторичных индексов.
* [Issue #1013](https://github.com/manticoresoftware/manticoresearch/issues/1013) Ранее запись таблицы могла быть удалена из списка индексов Manticore, если не удавалось начать её обслуживание при запуске. Новое поведение — сохранять её в списке и пытаться загрузить при следующем запуске.
* [indextool --docextract](Miscellaneous_tools.md#indextool) возвращает все слова и совпадения для запрошенного документа.
* [Commit 2b29](https://github.com/manticoresoftware/manticoresearch/commit/2b296ee20e520b85bcbb4383e87095d31e7165dc) Переменная окружения `dump_corrupt_meta` включает запись поврежденных метаданных таблицы в лог, если searchd не может загрузить индекс.
* [Commit c7a3](https://github.com/manticoresoftware/manticoresearch/commit/c7a3136eefe0eb2f69e3d99fae9c10beb5821ea6) `DEBUG META` может показать `max_matches` и псевдо статистику шардинга.
* [Commit 6bca](https://github.com/manticoresoftware/manticoresearch/commit/6bca4848dc5c8396dbc1e057be4b9b2d579794e8) Более понятная ошибка вместо сбивающего с толку сообщения "Index header format is not json, will try it as binary...".
* [Commit bef3](https://github.com/manticoresoftware/lemmatizer-uk/commit/bef3ff0386d3ee87ec57619782100972c1122e47) Путь к украинскому лемматизатору изменён.
* [Commit 4ae7](https://github.com/manticoresoftware/manticoresearch/commit/4ae789595329a2951e194d1191ddb3121459a560) Статистика вторичных индексов добавлена в [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META).
* [Commit 2e7c](https://github.com/manticoresoftware/manticoresearch/commit/2e7c585e) JSON-интерфейс теперь можно легко визуализировать с помощью Swagger Editor https://manual.manticoresearch.com/Openapi#OpenAPI-specification.
* **⚠️ КРУПНОЕ ИЗМЕНЕНИЕ**: Протокол репликации изменён. Если вы запускаете кластер репликации, то при обновлении до Manticore 5 необходимо:
  - сначала корректно остановить все узлы
  - затем запустить последний остановленный узел с параметром `--new-cluster` (запустить инструмент `manticore_new_cluster` в Linux).
  - подробнее читайте про [перезапуск кластера](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster).

### Изменения, связанные с библиотекой Manticore Columnar
* Рефакторинг интеграции вторичных индексов с Columnar хранилищем.
* [Commit efe2](https://github.com/manticoresoftware/columnar/commit/efe26b2a) Оптимизация библиотеки Manticore Columnar, снижающая время отклика за счет частичной предварительной оценки min/max.
* [Commit 2757](https://github.com/manticoresoftware/columnar/commit/2757b99b) Если слияние дисковых чанков прерывается, демон теперь очищает временные файлы, связанные с MCL.
* [Commit e9c6](https://github.com/manticoresoftware/manticoresearch/commit/e9c672014b63e2b49e7a5a53880583af86c3ab4e) Версии Columnar и вторичных библиотек выводятся в лог при сбое.
* [Commit f5e8](https://github.com/manticoresoftware/columnar/commit/f5e84eeb8b6cb102f13e3468246702427527fef9) Добавлена поддержка быстрой перемотки списка документов во вторичных индексах.
* [Commit 06df](https://github.com/manticoresoftware/manticoresearch/commit/06dfdd2e668933495dd6cb52d5ba2e0aa9b26e58) Запросы вида `select attr, count(*) from plain_index` (без фильтрации) теперь работают быстрее при использовании MCL.
* [Commit 0a76](https://github.com/manticoresoftware/manticoresearch/commit/0a76ecb9adbe886868b962dd425de5f475bbf55e) @@autocommit в HandleMysqlSelectSysvar для совместимости с .net коннектором mysql выше 8.25
* **⚠️ КРУПНОЕ ИЗМЕНЕНИЕ**: [Проблема MCL #17](https://github.com/manticoresoftware/columnar/issues/17) MCL: добавлен SSE код для сканирования Columnar. MCL теперь требует минимум SSE4.2.

### Изменения, связанные с пакетированием
* [Commit 4d19](https://github.com/manticoresoftware/manticoresearch/commit/4d19f5cbe49a31228aa09253f061165bfe80e51b) **⚠️ КРУПНОЕ ИЗМЕНЕНИЕ**: Прекращена поддержка Debian Stretch и Ubuntu Xenial.
* Поддержка RHEL 9, включая Centos 9, Alma Linux 9 и Oracle Linux 9.
* [Issue #924](https://github.com/manticoresoftware/manticoresearch/issues/924) Поддержка Debian Bookworm.
* [Issue #636](https://github.com/manticoresoftware/manticoresearch/issues/636) Пакетирование: сборки arm64 для Linux и MacOS.
* [PR #26](https://github.com/manticoresoftware/docker/pull/26) Docker-образ с мультиархитектурной поддержкой (x86_64 / arm64).
* [Упрощённая сборка пакетов для контрибуторов](Installation/Compiling_from_sources.md#Building-using-CI-Docker).
* Теперь можно установить конкретную версию с помощью APT.
* [Коммит a6b8](https://github.com/manticoresoftware/manticoresearch/commit/51fddca5c2a3ebb8576fae4c18660656ba22de0f) Инсталлятор для Windows (ранее предоставляли только архив).
* Перешли на компиляцию с использованием CLang 15.
* **⚠️ Критическое изменение**: Пользовательские формулы Homebrew, включая формулу для Manticore Columnar Library. Для установки Manticore, MCL и любых других необходимых компонентов используйте следующую команду `brew install manticoresoftware/manticore/manticoresearch manticoresoftware/manticore/manticore-extra`.

### Исправления ошибок
* [Issue #479](https://github.com/manticoresoftware/manticoresearch/issues/479) Поле с именем `text`
* [Issue #501](https://github.com/manticoresoftware/manticoresearch/issues/501) id не может быть не bigint
* [Issue #646](https://github.com/manticoresoftware/manticoresearch/issues/646) ALTER vs поле с именем "text"
* ❗[Issue #652](https://github.com/manticoresoftware/manticoresearch/issues/652) Возможная ошибка: HTTP (JSON) offset и limit влияют на результаты фасетов
* ❗[Issue #827](https://github.com/manticoresoftware/manticoresearch/issues/827) Searchd зависает/крашится при интенсивной загрузке
* ❗[Issue #996](https://github.com/manticoresoftware/manticoresearch/issues/996) PQ индекс вышел из памяти
* ❗[Коммит 1041](https://github.com/manticoresoftware/manticoresearch/commit/10416ef7dddf06c0d759e32ccd6ebaa2468f7cbf) `binlog_flush = 1` был сломан всё время с Sphinx. Исправлено.
* [MCL Issue #14](https://github.com/manticoresoftware/columnar/issues/14) MCL: краш при select, когда слишком много ft полей
* [Issue #470](https://github.com/manticoresoftware/manticoresearch/issues/470) sql_joined_field не может быть сохранён
* [Issue #713](https://github.com/manticoresoftware/manticoresearch/issues/713) Краш при использовании LEVENSHTEIN()
* [Issue #743](https://github.com/manticoresoftware/manticoresearch/issues/743) Manticore неожиданно крашится и не может нормально перезапуститься
* [Issue #788](https://github.com/manticoresoftware/manticoresearch/issues/788) CALL KEYWORDS через /sql возвращает управляющий символ, который ломает json
* [Issue #789](https://github.com/manticoresoftware/manticoresearch/issues/789) mariadb не может создать таблицу FEDERATED
* [Issue #796](https://github.com/manticoresoftware/manticoresearch/issues/796) WARNING: dlopen() failed: /usr/bin/lib_manticore_columnar.so: не может открыть разделяемый объект: Нет такого файла или директории
* [Issue #797](https://github.com/manticoresoftware/manticoresearch/issues/797) Manticore крашится при поиске с использованием ZONESPAN через api
* [Issue #799](https://github.com/manticoresoftware/manticoresearch/issues/799) неправильный вес при использовании нескольких индексов и facet distinct
* [Issue #801](https://github.com/manticoresoftware/manticoresearch/issues/801) SphinxQL групповой запрос виснет после повторной обработки SQL индекса
* [Issue #802](https://github.com/manticoresoftware/manticoresearch/issues/802) MCL: индексатор крашится в версиях 5.0.2 и manticore-columnar-lib 1.15.4
* [Issue #813](https://github.com/manticoresoftware/manticoresearch/issues/813) Manticore 5.0.2 FEDERATED возвращает пустой набор (MySQL 8.0.28)
* [Issue #824](https://github.com/manticoresoftware/manticoresearch/issues/824) select COUNT DISTINCT по 2 индексам при нулевом результате выдает внутреннюю ошибку
* [Issue #826](https://github.com/manticoresoftware/manticoresearch/issues/826) Краш на delete запросе
* [Issue #843](https://github.com/manticoresoftware/manticoresearch/issues/843) MCL: ошибка с длинным текстовым полем
* [Issue #856](https://github.com/manticoresoftware/manticoresearch/issues/856) 5.0.2 rtindex: Поведение ограничения агрегированного поиска не соответствует ожиданиям
* [Issue #863](https://github.com/manticoresoftware/manticoresearch/issues/863) Возвращается объект NoneType даже для запросов, которые должны возвращать несколько результатов
* [Issue #870](https://github.com/manticoresoftware/manticoresearch/issues/870) Краш при использовании Attribute и Stored Field в SELECT выражении
* [Issue #872](https://github.com/manticoresoftware/manticoresearch/issues/872) таблица становится невидимой после краша
* [Issue #877](https://github.com/manticoresoftware/manticoresearch/issues/877) Два отрицательных термина в поисковом запросе вызывают ошибку: запрос не вычисляем
* [Issue #878](https://github.com/manticoresoftware/manticoresearch/issues/878) a -b -c не работает через json query_string
* [Issue #886](https://github.com/manticoresoftware/manticoresearch/issues/886) pseudo_sharding с query match
* [Issue #893](https://github.com/manticoresoftware/manticoresearch/issues/893) Manticore 5.0.2 функции min/max работают не так, как ожидалось...
* [Issue #896](https://github.com/manticoresoftware/manticoresearch/issues/896) Поле "weight" неправильно парсится
* [Issue #897](https://github.com/manticoresoftware/manticoresearch/issues/897) Сервис Manticore крашится при старте и постоянно перезапускается
* [Issue #900](https://github.com/manticoresoftware/manticoresearch/issues/900) group by j.a, smth работает неправильно
* [Issue #913](https://github.com/manticoresoftware/manticoresearch/issues/913) Краш Searchd при использовании expr в ranker, но только для запросов с двумя proximity
* [Issue #916](https://github.com/manticoresoftware/manticoresearch/issues/916) net_throttle_action сломан
* [Issue #919](https://github.com/manticoresoftware/manticoresearch/issues/919) MCL: Manticore крашится при выполнении запроса и другие сбои при восстановлении кластера
* [Issue #925](https://github.com/manticoresoftware/manticoresearch/issues/925) SHOW CREATE TABLE выводит без обратных кавычек
* [Issue #930](https://github.com/manticoresoftware/manticoresearch/issues/930) Теперь возможно выполнять запросы к Manticore из Java через JDBC коннектор
* [Issue #933](https://github.com/manticoresoftware/manticoresearch/issues/933) проблемы с ранжированием bm25f
* [Issue #934](https://github.com/manticoresoftware/manticoresearch/issues/934) бесконечно замороженные configless индексы в watchdog при первом запуске
* [Issue #937](https://github.com/manticoresoftware/manticoresearch/issues/937) Segfault при сортировке данных фасета
* [Issue #940](https://github.com/manticoresoftware/manticoresearch/issues/940) краш на CONCAT(TO_STRING)
* [Issue #947](https://github.com/manticoresoftware/manticoresearch/issues/947) В некоторых случаях одиночный простой select мог привести к полной остановке инстанса, поэтому нельзя было войти в него или выполнить какой-либо другой запрос, пока не завершался выполняющийся select.
* [Issue #948](https://github.com/manticoresoftware/manticoresearch/issues/948) Сбой индексатора
* [Issue #950](https://github.com/manticoresoftware/manticoresearch/issues/950) Неправильный подсчет из facet distinct
* [Issue #953](https://github.com/manticoresoftware/manticoresearch/issues/953) LCS рассчитывается неправильно в встроенном ранжировщике sph04
* [Issue #955](https://github.com/manticoresoftware/manticoresearch/issues/955) Крушение dev версии 5.0.3
* [Issue #963](https://github.com/manticoresoftware/manticoresearch/issues/963) FACET с json на columnar engine вызывает сбой
* [Issue #982](https://github.com/manticoresoftware/manticoresearch/issues/982) MCL: 5.0.3 аварийное завершение из-за вторичного индекса
* [PR #984](https://github.com/manticoresoftware/manticoresearch/pull/984) @@autocommit в HandleMysqlSelectSysvar
* [PR #985](https://github.com/manticoresoftware/manticoresearch/pull/985) Исправлена распределение thread-chunk в RT индексах
* [Issue #985](https://github.com/manticoresoftware/manticoresearch/issues/985) Исправлено распределение thread-chunk в RT индексах
* [Issue #986](https://github.com/manticoresoftware/manticoresearch/issues/986) Неправильное значение по умолчанию max_query_time
* [Issue #987](https://github.com/manticoresoftware/manticoresearch/issues/987) Сбой при использовании регулярного выражения в многопоточном выполнении
* [Issue #988](https://github.com/manticoresoftware/manticoresearch/issues/988) Нарушена совместимость обратного индекса
* [Issue #989](https://github.com/manticoresoftware/manticoresearch/issues/989) indextool сообщает об ошибке проверки columnar атрибутов
* [Issue #990](https://github.com/manticoresoftware/manticoresearch/issues/990) Утечка памяти у json grouper клонов
* [Issue #991](https://github.com/manticoresoftware/manticoresearch/issues/991) Утечка памяти у клонирования функции levenshtein
* [Issue #992](https://github.com/manticoresoftware/manticoresearch/issues/992) Потеря сообщения об ошибке при загрузке meta
* [Issue #993](https://github.com/manticoresoftware/manticoresearch/issues/993) Пропаганда ошибок из динамических индексов/subkeys и sysvars
* [Issue #994](https://github.com/manticoresoftware/manticoresearch/issues/994) Сбой при count distinct по строковому столбцу в columnar хранилище
* [Issue #995](https://github.com/manticoresoftware/manticoresearch/issues/995) MCL: min(pickup_datetime) из taxi1 вызывает сбой
* [Issue #997](https://github.com/manticoresoftware/manticoresearch/issues/997) Пустой excludes JSON запрос удаляет столбцы из списка select
* [Issue #998](https://github.com/manticoresoftware/manticoresearch/issues/998) Вторичные задачи, запускаемые на текущем планировщике, иногда вызывают ненормальные побочные эффекты
* [Issue #999](https://github.com/manticoresoftware/manticoresearch/issues/999) Сбой с facet distinct и разными схемами
* [Issue #1000](https://github.com/manticoresoftware/manticoresearch/issues/1000) MCL: Columnar rt индекс повреждается после запуска без columnar библиотеки
* [Issue #1001](https://github.com/manticoresoftware/manticoresearch/issues/1001) Не работает неявный cutoff в json
* [Issue #1002](https://github.com/manticoresoftware/manticoresearch/issues/1002) Проблема с columnar grouper
* [Issue #1003](https://github.com/manticoresoftware/manticoresearch/issues/1003) Невозможно удалить последнее поле из индекса
* [Issue #1004](https://github.com/manticoresoftware/manticoresearch/issues/1004) Некорректное поведение после --new-cluster
* [Issue #1005](https://github.com/manticoresoftware/manticoresearch/issues/1005) "columnar library not loaded", хотя она не требуется
* [Issue #1006](https://github.com/manticoresoftware/manticoresearch/issues/1006) Отсутствие ошибки для delete запроса
* [Issue #1010](https://github.com/manticoresoftware/manticoresearch/issues/1010) Исправлено расположение ICU data файла в Windows сборках
* [PR #1018](https://github.com/manticoresoftware/manticoresearch/pull/1018) Проблема с отправкой Handshake
* [Issue #1020](https://github.com/manticoresoftware/manticoresearch/issues/1020) Отображение id в show create table
* [Issue #1024 crash 1](https://github.com/manticoresoftware/manticoresearch/issues/1024) Сбой / Segmentation Fault при Facet поиске с большим количеством результатов.
* [Issue #1026](https://github.com/manticoresoftware/manticoresearch/issues/1026) RT индекс: searchd "зависает" навсегда при вставке большого количества документов и заполнении RAMchunk
* [Commit 4739](https://github.com/manticoresoftware/manticoresearch/commit/4739dafd) Поток зависает при отключении, пока репликация активна между узлами
* [Commit ab87](https://github.com/manticoresoftware/manticoresearch/commit/ab87836782e7ae43fe5f9dea739eed0d336b42c1) Смешивание float и int в JSON range фильтре могло привести к игнорированию фильтра в Manticore
* [Commit d001](https://github.com/manticoresoftware/manticoresearch/commit/d00101c2905f6393ce3fca23c4f6dcb2506f4bd9) Float фильтры в JSON были неточными
* [Commit 4092](https://github.com/manticoresoftware/manticoresearch/commit/4092c6a8) Прервать незакоммиченные транзакции при изменении индекса (иначе может случиться сбой)
* [Commit 9692](https://github.com/manticoresoftware/manticoresearch/commit/9692a840) Синтаксическая ошибка запроса при использовании обратного слэша
* [Commit 0c19](https://github.com/manticoresoftware/manticoresearch/commit/0c19ad8d) workers_clients может быть неверным в SHOW STATUS
* [Commit 1772](https://github.com/manticoresoftware/manticoresearch/commit/17720e1e2f2ec38337df565bee78f7ce9d9c744a) Исправлен сбой при слиянии ram сегментов без docstores
* [Commit f45b](https://github.com/manticoresoftware/manticoresearch/commit/f45b337b5c265433bb7d6262ad65d02f65edbaf3) Исправлено пропущенное условие ALL/ANY для equals JSON фильтра
* [Commit 3e83](https://github.com/manticoresoftware/manticoresearch/commit/3e83c1f0c0e6e7a9d4297113f69a112cbabd9e64) Репликация могла завершиться с ошибкой `got exception while reading ist stream: mkstemp(./gmb_pF6TJi) failed: 13 (Permission denied)`, если searchd был запущен из каталога без прав записи.
* [Commit 92e5](https://github.com/manticoresoftware/manticoresearch/commit/92e5629f7150f62524b385fca670a01ba9590f5b) С версии 4.0.2 журнал сбоев включал только оффсеты. Этот коммит исправляет это.

## Version 5.0.2
Released: May 30th 2022

### Bugfixes
* ❗[Issue #791](https://github.com/manticoresoftware/manticoresearch/issues/791) - неправильный размер стека мог вызвать сбой.

## Version 5.0.0
Released: May 18th 2022


### Major new features
* 🔬 Поддержка [Manticore Columnar Library 1.15.2](https://github.com/manticoresoftware/columnar/), которая включает бета-версию [Вторичных индексов](Server_settings/Searchd.md#secondary_indexes). Построение вторичных индексов включено по умолчанию для обычных и реального времени колоночных и строчных индексов (если используется [Manticore Columnar Library](https://github.com/manticoresoftware/columnar)), но для включения их в поиске необходимо задать `secondary_indexes = 1` либо в конфигурационном файле, либо с помощью [SET GLOBAL](Server_settings/Setting_variables_online.md). Новая функциональность поддерживается во всех операционных системах, кроме старого Debian Stretch и Ubuntu Xenial.
* [Режим только для чтения](Security/Read_only.md): теперь можно указать слушатели, которые обрабатывают только запросы на чтение, отклоняя любые записи.
* Новый эндпоинт [/cli](Connecting_to_the_server/HTTP.md#/cli) для еще более простого запуска SQL-запросов по HTTP.
* Более быстрая массовая вставка/замена/удаление через JSON по HTTP: ранее можно было отправлять несколько команд записи через HTTP JSON протокол, но они обрабатывались по одной, теперь они обрабатываются как одна транзакция.
* [#720](https://github.com/manticoresoftware/manticoresearch/issues/720) Поддержка [Вложенных фильтров](Searching/Filters.md#Nested-bool-query) в JSON протоколе. Ранее в JSON нельзя было кодировать конструкции типа `a=1 and (b=2 or c=3)`: `must` (AND), `should` (OR) и `must_not` (NOT) работали только на верхнем уровне. Теперь их можно вкладывать.
* Поддержка [Chunked transfer encoding](https://en.wikipedia.org/wiki/Chunked_transfer_encoding) в HTTP-протоколе. Теперь вы можете использовать передачу чанками в вашем приложении для отправки больших партий с меньшим потреблением ресурсов (так как вычислять `Content-Length` не нужно). На стороне сервера Manticore теперь всегда обрабатывает входящие HTTP-данные потоково, не ожидая полной передачи партии как раньше, что:
  - снижает пиковое использование ОЗУ, уменьшая риск OOM
  - уменьшает время отклика (по нашим тестам, уменьшение на 11% при обработке партии 100МБ)
  - позволяет обойти [max_packet_size](Server_settings/Searchd.md#max_packet_size) и передавать партии гораздо большего размера, чем максимальное значение `max_packet_size` (128МБ), например, 1ГБ за раз.
* [#719](https://github.com/manticoresoftware/manticoresearch/issues/719) Поддержка HTTP-интерфейсом `100 Continue`: теперь можно передавать большие партии с помощью `curl` (включая curl библиотеки, используемые в различных языках программирования), который по умолчанию отправляет `Expect: 100-continue` и ждет некоторое время перед отправкой партии. Ранее приходилось добавлять пустой заголовок `Expect: `, теперь это не нужно.

  <details>

  Ранее (обратите внимание на время отклика):

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
  real
0m1.022s
  user
0m0.001s

  sys
0m0.010s
  ```
  Теперь:
  ```bash
  $ time curl -v -sX POST http://localhost:9318/bulk -H "Content-Type: application/x-ndjson" --data '{"insert": {"index": "user", "doc":  {"name":"Prof. Matt Heaney IV","email":"ibergnaum@yahoo.com","description":"Tempora ullam eaque consequatur. Vero aut minima ut et ut omnis officiis vel. Molestiae quis voluptatum sint numquam.","age":15,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"Prof. Boyd McKenzie","email":"carlotta11@hotmail.com","description":"Blanditiis maiores odio corporis eaque illum. Aut et rerum iste. Neque et ullam quisquam officia dignissimos quo cumque.","age":84,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"Г-н Иоганн Смит","email":"stiedemann.tristin@ziemann.com","description":"Temporibus amet magnam consequatur omnis consequatur illo fugit. Debitis natus doloremque est tempore deserunt vero. Harum eos corrupti nemo ut.","age":89,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"Гектор Пурос","email":"hickle.mafalda@hotmail.com","description":" as voluptatem inventore sit. Aliquam fugit perferendis est id aut odio et sapiente.","age":64,"active":1}}}'
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

* **⚠️ ЛОМАННОЕ ИЗМЕНЕНИЕ**: [Псевдо-шардинг](Server_settings/Searchd.md#pseudo_sharding) теперь включён по умолчанию. Если хотите отключить, добавьте `pseudo_sharding = 0` в секцию `searchd` вашего конфигурационного файла Manticore.
* Минимум одно полнотекстовое поле в индексе реального времени/простом теперь не обязательно. Теперь Manticore можно использовать и в случаях, не связанных с полнотекстовым поиском.
* [Быстрое извлечение](Creating_a_table/Data_types.md#fast_fetch) для атрибутов с поддержкой [Manticore Columnar Library](https://github.com/manticoresoftware/columnar): запросы вроде `select * from <columnar table>` теперь выполняются значительно быстрее, особенно если в схеме много полей.
* **⚠️ ЛОМАННОЕ ИЗМЕНЕНИЕ**: Неявный [cutoff](Searching/Options.md#cutoff). Manticore теперь не тратит время и ресурсы на обработку данных, не нужных в результирующем наборе. Недостаток — это влияет на `total_found` в [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META) и [hits.total](Searching/Full_text_matching/Basic_usage.md#HTTP-JSON) в JSON-выводе. Точное значение теперь только в случае `total_relation: eq`, а `total_relation: gte` означает, что фактическое число совпадающих документов больше значения `total_found`. Для возврата прежнего поведения используйте опцию поиска `cutoff=0`, что заставит `total_relation` всегда быть `eq`.
* **⚠️ ЛОМАННОЕ ИЗМЕНЕНИЕ**: Все полнотекстовые поля теперь по умолчанию [хранятся](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#stored_fields). Для отключения этой функции используйте `stored_fields = ` (пустое значение), чтобы сделать все поля не хранимыми (т.е. вернуть предыдущее поведение).
* [#715](https://github.com/manticoresoftware/manticoresearch/issues/715) HTTP JSON теперь поддерживает [опции поиска](Searching/Options.md#General-syntax).
### Незначительные изменения
* **⚠️ ЛОМАННОЕ ИЗМЕНЕНИЕ**: Изменён формат файла метаданных индекса. Раньше файлы метаданных (`.meta`, `.sph`) были в бинарном формате, теперь это просто json. Новая версия Manticore автоматически конвертирует старые индексы, но:
  - возможно предупреждение `WARNING: ... syntax error, unexpected TOK_IDENT`
  - вы не сможете запускать индекс с предыдущими версиями Manticore, убедитесь в наличии резервной копии
* **⚠️ ЛОМАННОЕ ИЗМЕНЕНИЕ**: Поддержка состояния сессии с помощью [HTTP keep-alive](Connecting_to_the_server/HTTP.md#Keep-alive). Это делает HTTP состоянием, если клиент тоже поддерживает это. Например, с новым эндпоинтом [/cli](Connecting_to_the_server/HTTP.md#/cli) и HTTP keep-alive (включён по умолчанию во всех браузерах) вы можете вызвать `SHOW META` после `SELECT`, и это будет работать как в mysql. Ранее HTTP-заголовок `Connection: keep-alive` поддерживался, но лишь для повторного использования соединения. Теперь это ещё и делает сессию состоянием.
* Теперь можно задавать `columnar_attrs = *`, чтобы определить все атрибуты как columnar в [простом режиме](Read_this_first.md#Real-time-mode-vs-plain-mode) — удобно, если список длинный.
* Быстрее репликация SST
* **⚠️ ЛОМАННОЕ ИЗМЕНЕНИЕ**: Протокол репликации изменён. При работе с кластером репликации и обновлении до Manticore 5 необходимо:
  - сначала корректно остановить все узлы
  - затем запустить последний остановленный узел с `--new-cluster` (используйте утилиту `manticore_new_cluster` в Linux).
  - подробнее об [перезапуске кластера](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster)
* Улучшения репликации:
  - Быстрее SST
  - Устойчивость к помехам, что помогает при нестабильной сети между узлами
  - Улучшенное логирование
* Улучшение безопасности: теперь Manticore слушает `127.0.0.1`, а не `0.0.0.0`, если в конфиге нет параметра `listen`. Хотя в поставляемом по умолчанию конфиге `listen` указан, конфигурация без него возможна. Раньше Manticore слушал `0.0.0.0`, что небезопасно, теперь — `127.0.0.1`, который обычно не доступен из Интернета.
* Быстрее агрегация по columnar-атрибутам.
* Повышена точность `AVG()`: раньше для агрегаций использовался `float`, теперь — `double`, что значительно повышает точность.
* Улучшена поддержка драйвера JDBC MySQL.
* Поддержка `DEBUG malloc_stats` для [jemalloc](https://github.com/jemalloc/jemalloc).
* [optimize_cutoff](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#optimize_cutoff) теперь доступен как настройка для каждой таблицы при создании или изменении таблицы.
* **⚠️ КРИТИЧЕСКОЕ ИЗМЕНЕНИЕ**: [query_log_format](Server_settings/Searchd.md#query_log_format) теперь по умолчанию **`sphinxql`**. Если вы привыкли к формату `plain`, необходимо добавить `query_log_format = plain` в файл конфигурации.
* Значительные улучшения по потреблению памяти: Manticore теперь существенно меньше использует оперативную память при длительной и интенсивной нагрузке insert/replace/optimize, если используются хранимые поля.
* Значение по умолчанию для [shutdown_timeout](Server_settings/Searchd.md#shutdown_timeout) увеличено с 3 секунд до 60 секунд.

* [Коммит ffd0](https://github.com/manticoresoftware/manticoresearch/commit/ffd0499d329d2c383f14c8a44c4cc84338ab56e7) Поддержка Java mysql connector >= 6.0.3: в [Java mysql connection 6.0.3](https://mvnrepository.com/artifact/mysql/mysql-connector-java/6.0.3) изменён способ подключения к mysql, что нарушило совместимость с Manticore. Теперь поддерживается новое поведение.
* [Коммит 1da6](https://github.com/manticoresoftware/manticoresearch/commit/1da6dbec) отключено сохранение нового дискового фрагмента при загрузке индекса (например, при запуске searchd).
* [Issue #746](https://github.com/manticoresoftware/manticoresearch/issues/746) Поддержка glibc >= 2.34.
* [Issue #784](https://github.com/manticoresoftware/manticoresearch/issues/784) VIP-подключения считаются отдельно от обычных (не VIP). Ранее VIP-подключения учитывались в лимите `max_connections`, что могло вызывать ошибку "maxed out" на обычные подключения. Теперь VIP-подключения не учитываются в лимит. Текущее число VIP-подключений можно увидеть в `SHOW STATUS` и `status`.
* [ID](Creating_a_table/Data_types.md#Document-ID) теперь можно указывать явно.
* [Issue #687](https://github.com/manticoresoftware/manticoresearch/issues/687) поддержка сжатия zstd для mysql протокола
### ⚠️ Другие незначительные критические изменения
* ⚠️ Формула BM25F слегка обновлена для улучшения релевантности поиска. Это влияет только на поиск при использовании функции [BM25F()](Functions/Searching_and_ranking_functions.md#BM25F%28%29), поведение формулы ранжирования по умолчанию не изменилось.
* ⚠️ Изменено поведение REST эндпоинта [/sql](Connecting_to_the_server/HTTP.md#mode=raw): `/sql?mode=raw` теперь требует экранирования и возвращает массив.

* ⚠️ Изменён формат ответа на `/bulk` запросы INSERT/REPLACE/DELETE:
  - ранее каждый подзапрос был отдельной транзакцией и возвращал отдельный ответ
  - теперь вся партия считается одной транзакцией и возвращает единый ответ
* ⚠️ Опции поиска `low_priority` и `boolean_simplify` теперь требуют указания значения (`0/1`): раньше можно было `SELECT ... OPTION low_priority, boolean_simplify`, теперь необходимо `SELECT ... OPTION low_priority=1, boolean_simplify=1`.
* ⚠️ Если вы используете старые клиенты [php](https://github.com/manticoresoftware/manticoresearch/blob/master/api/sphinxapi.php), [python](https://github.com/manticoresoftware/manticoresearch/blob/master/api/sphinxapi.py) или [java](https://github.com/manticoresoftware/manticoresearch/tree/master/api/java), пожалуйста, перейдите по ссылкам и найдите обновлённые версии. **Старые версии не полностью совместимы с Manticore 5.**
* ⚠️ HTTP JSON запросы теперь логируются в другом формате при режиме `query_log_format=sphinxql`. Ранее логировалась только полнотекстовая часть, теперь логируется полностью.
### Новые пакеты
* **⚠️ КРИТИЧЕСКОЕ ИЗМЕНЕНИЕ**: из-за новой структуры при обновлении до Manticore 5 рекомендуется удалить старые пакеты перед установкой новых:
  - для RPM-систем: `yum remove manticore*`

  - для Debian и Ubuntu: `apt remove manticore*`
* Новая структура deb/rpm пакетов. Ранее версии включали:
  - `manticore-server` с `searchd` (главный демон поиска) и всем необходимым для него
  - `manticore-tools` с `indexer` и `indextool`
  - `manticore` включающий всё
  - мета-пакет RPM `manticore-all`, ссылавшийся на `manticore-server` и `manticore-tools`
  Новая структура:
  - `manticore` - мета пакет deb/rpm, устанавливающий все перечисленные выше как зависимости
  - `manticore-server-core` - `searchd` и всё необходимое для его работы отдельно
  - `manticore-server` - systemd файлы и другие дополнительные скрипты

  - `manticore-tools` - `indexer`, `indextool` и прочие инструменты
  - `manticore-common` - файл конфигурации по умолчанию, директория данных по умолчанию, стоп-слова по умолчанию
  - `manticore-icudata`, `manticore-dev`, `manticore-converter` практически не изменились
  - `.tgz` архив, включающий все пакеты
* Поддержка Ubuntu Jammy
* Поддержка Amazon Linux 2 через [YUM репозиторий](Installation/RHEL_and_Centos.md#YUM-repository)
### Исправления ошибок
* [Issue #815](https://github.com/manticoresoftware/manticoresearch/issues/815) Случайные сбои при использовании UDF-функции
* [Issue #287](https://github.com/manticoresoftware/manticoresearch/issues/287) Ошибка "out of memory" при индексации RT индекса
* [Issue #604](https://github.com/manticoresoftware/manticoresearch/issues/604) Критическое изменение в парсере sphinxql версии 3.6.0, 4.2.0
* [Issue #667](https://github.com/manticoresoftware/manticoresearch/issues/667) Критическая ошибка: недостаточно памяти (не удалось выделить 9007199254740992 байт)
* [Issue #676](https://github.com/manticoresoftware/manticoresearch/issues/676) Строки неправильно передаются в UDF
* ❗[Issue #698](https://github.com/manticoresoftware/manticoresearch/issues/698) Сбой searchd при попытке добавить текстовый столбец в rt индекс
* [Issue #705](https://github.com/manticoresoftware/manticoresearch/issues/705) Индексатор не мог найти все колонки
* ❗[Issue #709](https://github.com/manticoresoftware/manticoresearch/issues/705) Неправильная работа группировки по json.boolean
* [Issue #716](https://github.com/manticoresoftware/manticoresearch/issues/716) Ошибки в командах indextool, связанных с индексом (например, --dumpdict)
* ❗[Issue #724](https://github.com/manticoresoftware/manticoresearch/issues/724) Поля исчезают из выборки
* [Issue #727](https://github.com/manticoresoftware/manticoresearch/issues/727) Несовместимость Content-Type .NET HttpClient при использовании `application/x-ndjson`
* [Issue #729](https://github.com/manticoresoftware/manticoresearch/issues/729) Расчет длины поля
* ❗[Issue #730](https://github.com/manticoresoftware/manticoresearch/issues/730) create/insert into/drop columnar таблицы вызывает утечку памяти
* [Issue #731](https://github.com/manticoresoftware/manticoresearch/issues/731) Пустой столбец в результатах при определённых условиях
* ❗[Issue #749](https://github.com/manticoresoftware/manticoresearch/issues/749) Сбой демона при запуске
* ❗[Issue #750](https://github.com/manticoresoftware/manticoresearch/issues/750) Демон зависает при запуске
* ❗[Issue #751](https://github.com/manticoresoftware/manticoresearch/issues/751) Сбой при SST
* [Issue #752](https://github.com/manticoresoftware/manticoresearch/issues/752) Json-атрибут помечается как columnar, когда engine='columnar'
* [Issue #753](https://github.com/manticoresoftware/manticoresearch/issues/753) Репликация слушает на 0
* [Issue #754](https://github.com/manticoresoftware/manticoresearch/issues/754) columnar_attrs = * не работает с csvpipe
* ❗[Issue #755](https://github.com/manticoresoftware/manticoresearch/issues/755) Сбой при выборке float в columnar в rt
* ❗[Issue #756](https://github.com/manticoresoftware/manticoresearch/issues/756) Indextool изменяет rt индекс во время проверки
* [Issue #757](https://github.com/manticoresoftware/manticoresearch/issues/757) Необходима проверка пересечений диапазонов портов слушателей
* [Issue #758](https://github.com/manticoresoftware/manticoresearch/issues/758) Лог оригинальной ошибки при сбое сохранения RT индекса на диск
* [Issue #759](https://github.com/manticoresoftware/manticoresearch/issues/759) Сообщается только одна ошибка для конфигурации RE2
* ❗[Issue #760](https://github.com/manticoresoftware/manticoresearch/issues/760) Изменения в потреблении RAM в коммите 5463778558586d2508697fa82e71d657ac36510f
* [Issue #761](https://github.com/manticoresoftware/manticoresearch/issues/761) 3-й узел не формирует неосновной кластер после грязной перезагрузки
* [Issue #762](https://github.com/manticoresoftware/manticoresearch/issues/762) Счетчик обновлений увеличивается на 2
* [Issue #763](https://github.com/manticoresoftware/manticoresearch/issues/763) Новая версия 4.2.1 портит индекс, созданный в 4.2.0 с использованием морфологии
* [Issue #764](https://github.com/manticoresoftware/manticoresearch/issues/764) Нет экранирования в ключах json /sql?mode=raw
* ❗[Issue #765](https://github.com/manticoresoftware/manticoresearch/issues/765) Использование функции скрывает другие значения
* ❗[Issue #766](https://github.com/manticoresoftware/manticoresearch/issues/766) Утечка памяти, вызванная строкой в FixupAttrForNetwork
* ❗[Issue #767](https://github.com/manticoresoftware/manticoresearch/issues/767) Утечка памяти в версиях 4.2.0 и 4.2.1 связанная с кешем docstore
* [Issue #768](https://github.com/manticoresoftware/manticoresearch/issues/768) Странный пинг-понг со сохранёнными полями по сети
* [Issue #769](https://github.com/manticoresoftware/manticoresearch/issues/769) lemmatizer_base сбрасывается в пустое, если не указан в разделе 'common'
* [Issue #770](https://github.com/manticoresoftware/manticoresearch/issues/770) pseudo_sharding замедляет SELECT по id
* [Issue #771](https://github.com/manticoresoftware/manticoresearch/issues/771) DEBUG malloc_stats выводит нули при использовании jemalloc
* [Issue #772](https://github.com/manticoresoftware/manticoresearch/issues/772) Drop/add column делает значение невидимым
* [Issue #773](https://github.com/manticoresoftware/manticoresearch/issues/773) Невозможно добавить столбец bit(N) в columnar таблицу
* [Issue #774](https://github.com/manticoresoftware/manticoresearch/issues/774) "cluster" становится пустым при запуске в manticore.json

* ❗[Commit 1da4](https://github.com/manticoresoftware/manticoresearch/commits/1da4ce891e4ff13727cfd3331d72771c64ee7949) HTTP действия не отслеживаются в SHOW STATUS

* [Commit 3810](https://github.com/manticoresoftware/manticoresearch/commits/381000ab4af4c3c98ec8c730699ad7f39039cec8) отключение pseudo_sharding для низкочастотных одиночных запросов по ключу
* [Commit 8003](https://github.com/manticoresoftware/manticoresearch/commits/800325cca283a87801b28929d82420fafc76a0ee) исправлена синхронизация сохранённых атрибутов и слияния индексов

* [Commit cddf](https://github.com/manticoresoftware/manticoresearch/commits/cddfeed6296f2041c6aae18414e5cd9cabf08281) обобщены выборщики distinct значений; добавлены специализированные выборщики distinct для строк columnar

* [Commit fba4](https://github.com/manticoresoftware/manticoresearch/commits/fba4bb4f) исправлен выбор null integer атрибутов из docstore

* [Commit f300](https://github.com/manticoresoftware/manticoresearch/commit/f3009a9242fa16956adf871c4887e6e0303ba364) `ranker` мог быть указан дважды в журнале запросов
## Версия 4.2.0, 23 декабря 2021

### Основные новые возможности
* **Поддержка псевдо-шардинга для индексов реального времени и полнотекстовых запросов**. В предыдущем релизе была добавлена ограниченная поддержка псевдошардинга. Начиная с этой версии вы можете получить все преимущества псевдошардинга и многоядерного процессора просто включив [searchd.pseudo_sharding](Server_settings/Searchd.md#pseudo_sharding). Самое классное, что вам ничего не нужно менять в своих индексах или запросах, просто включите его и если у вас есть свободный CPU, он будет использован для уменьшения времени ответа. Поддерживаются простые и индексы реального времени для полнотекстовых, фильтрационных и аналитических запросов. Например, вот как включение псевдо-шардинга может сделать среднее **время ответа большинства запросов примерно в 10 раз меньше** на [Hacker news curated comments dataset](https://zenodo.org/record/45901/) умноженном в 100 раз (116 миллионов документов в простом индексе).

![Псевдо-шардинг включен и выключен в 4.2.0](4.2.0_ps_on_vs_off.png)

* Теперь поддерживается [**Debian Bullseye**](https://manticoresearch.com/install/).

<!-- example pq_transactions_4.2.0 -->
* Транзакции PQ теперь атомарны и изолированы. Ранее поддержка транзакций PQ была ограничена. Это обеспечивает гораздо более **быстрый REPLACE в PQ**, особенно когда нужно заменить много правил одновременно. Подробности производительности:

<!-- intro -->
#### 4.0.2
<!-- request 4.0.2 -->
Требуется **48 секунд, чтобы вставить 1 млн правил PQ** и **406 секунд, чтобы ЗАМЕНИТЬ всего 40 тыс.** партиями по 10 тыс.
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
Требуется **34 секунды, чтобы вставить 1 млн правил PQ** и **23 секунды, чтобы ЗАМЕНИТЬ их** партиями по 10 тыс.
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
### Незначительные изменения
*  [optimize_cutoff](Server_settings/Searchd.md#optimize_cutoff) теперь доступен как опция конфигурации в секции `searchd`. Полезно, когда вы хотите ограничить количество RT чанков во всех ваших индексах до определённого числа глобально.
* [Commit 0087](https://github.com/manticoresoftware/manticoresearch/commit/00874743823eaf43e2a09a088392fd7e0a617f47) точный подсчёт [count(distinct ...)](Searching/Grouping.md#COUNT%28DISTINCT-field%29) и [FACET ... distinct](Searching/Faceted_search.md#Faceting-without-duplicates) по нескольким локальным физическим индексам (real-time/plain) с одинаковым набором/порядком полей.
* [PR #598](https://github.com/manticoresoftware/manticoresearch/pull/598) поддержка bigint для `YEAR()` и других функций временной метки.

* [Commit 8e85](https://github.com/manticoresoftware/manticoresearch/commit/8e85d4bce596cc88f9d9615ea243a27e4afde0f7) Адаптивный [rt_mem_limit](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_mem_limit). Ранее Manticore Search собирал ровно до `rt_mem_limit` данных перед сохранением нового дискового чанка, и при сохранении продолжал собирать до 10% больше (так называемый двойной буфер) чтобы минимизировать возможные приостановки вставки. Если этот лимит тоже исчерпывался, добавление новых документов блокировалось до полного сохранения дискового чанка. Новый адаптивный лимит основан на том, что теперь у нас есть [auto-optimize](Server_settings/Searchd.md#auto_optimize), поэтому не страшно, если дисковые чанки не будут строго соответствовать `rt_mem_limit` и начнут сброс раньше. Итак, сейчас мы собираем до 50% `rt_mem_limit` и сохраняем это как дисковый чанк. При сохранении смотрим статистику (сколько мы сохранили, сколько новых документов пришло за время сохранения) и пересчитываем исходный коэффициент, который будет использоваться в следующий раз. Например, если мы сохранили 90 млн документов, а за время сохранения пришло ещё 10 млн, коэффициент 90%, значит в следующий раз мы можем собирать до 90% `rt_mem_limit` перед началом сброса следующего дискового чанка. Значение коэффициента рассчитывается автоматически от 33.3% до 95%.
* [Issue #628](https://github.com/manticoresoftware/manticoresearch/issues/628) [unpack_zlib](Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Database_connection.md#unpack_zlib) для источника PostgreSQL. Спасибо, [Dmitry Voronin](https://github.com/dimv36) за [вклад](https://github.com/manticoresoftware/manticoresearch/pull/630).
* [Commit 6d54](https://github.com/manticoresoftware/manticoresearch/commit/6d54cf2b319b131970c29410cc21abcbdf8083b1) `indexer -v` и `--version`. Ранее можно было увидеть версию индексатора, но `-v`/`--version` не поддерживались.
* [Issue #662](https://github.com/manticoresoftware/manticoresearch/issues/662) бесконечный лимит mlock по умолчанию, когда Manticore запускается через systemd.
* [Commit 63c8](https://github.com/manticoresoftware/manticoresearch/commit/63c8cd05cf399e705c2c14939411a92cb784735a) spinlock -> op queue для coro rwlock.

* [Commit 4113](https://github.com/manticoresoftware/manticoresearch/commit/41130ce30d1734bbf8e6b8b63c4e815caf9464f2) переменная окружения `MANTICORE_TRACK_RT_ERRORS`, полезная для отладки повреждений RT сегментов.
### Серьёзные изменения
* Версия binlog была увеличена, binlog из предыдущей версии не будет воспроизведён, поэтому при обновлении убедитесь, что вы корректно остановили Manticore Search: после остановки предыдущего экземпляра не должно быть файлов binlog в `/var/lib/manticore/binlog/`, кроме `binlog.meta`.
* [Commit 3f65](https://github.com/manticoresoftware/manticoresearch/commit/3f659f36e87d99ee262205a8eae4663a255be630) новый столбец "chain" в `show threads option format=all`. Он показывает стек некоторых тикетов задачи, наиболее полезен для целей профилирования, поэтому при разборе вывода `show threads` учитывайте новый столбец.
* `searchd.workers` устарел с версии 3.5.0, теперь помечен как deprecated, если он ещё есть в вашем конфигурационном файле — при старте будет предупреждение. Manticore Search запустится, но с предупреждением.
* Если вы используете PHP и PDO для доступа к Manticore, необходимо включить `PDO::ATTR_EMULATE_PREPARES`.
### Исправления ошибок
* ❗[Issue #650](https://github.com/manticoresoftware/manticoresearch/issues/650) Manticore 4.0.2 медленнее, чем Manticore 3.6.3. В 4.0.2 было быстрее bulk вставок, но заметно медленнее вставок одиночных документов. Исправлено в 4.2.0.
* ❗[Commit 22f4](https://github.com/manticoresoftware/manticoresearch/commit/22f4141bed1411c83684ae6e0bae832071cce3df) RT индекс мог повредиться под интенсивной нагрузкой REPLACE, или мог аварийно завершиться.
* [Commit 03be](https://github.com/manticoresoftware/manticoresearch/commit/03be91e4f720a3f20bee0278dd8f0d44abd3ef0e) исправлен average при слиянии группировщиков и сортировщика group N; исправлено слияние агрегатов.
* [Commit 2ea5](https://github.com/manticoresoftware/manticoresearch/commit/2ea575d34efe38661f14d858f2b552456f29fe42) `indextool --check` мог аварийно завершаться.
* [Commit 7ec7](https://github.com/manticoresoftware/manticoresearch/commit/7ec76d4abf01b5fca6f10a0000fe64d5d8e5fa52) проблема истощения RAM, вызванная UPDATE.
* [Commit 658a](https://github.com/manticoresoftware/manticoresearch/commit/658a727e41f3c3fbcde9754149528f34ed76ba19) демон мог зависать на INSERT.
* [Commit 46e4](https://github.com/manticoresoftware/manticoresearch/commit/46e42b9b386e31e53247ed634e158b533e28063e) демон мог зависать при выключении.
* [Commit f8d7](https://github.com/manticoresoftware/manticoresearch/commit/f8d7d5177cc83d250b493b4cfc619732b29cf072) демон мог аварийно завершаться при выключении.
* [Commit 733a](https://github.com/manticoresoftware/manticoresearch/commit/733accf165142a1d1574689525ccca8907971158) демон мог зависать при сбое.
* [Commit f7f8](https://github.com/manticoresoftware/manticoresearch/commit/f7f8bd8c0dbb23a59a88502dcba16b0918fb2330) демон мог аварийно завершаться при запуске, пытаясь повторно войти в кластер с некорректным списком узлов
* [Commit 1401](https://github.com/manticoresoftware/manticoresearch/commit/140155613821ecaddcd1a3dfc4f7369b0df6aef7) распределенный индекс мог полностью забываться в режиме RT, если не мог разрешить одного из своих агентов при запуске
* [Issue #683](https://github.com/manticoresoftware/manticoresearch/issues/683) атрибут bit(N) engine='columnar' выдаёт ошибку
* [Issue #682](https://github.com/manticoresoftware/manticoresearch/issues/682) создание таблицы завершается неудачей, но директория остаётся
* [Issue #663](https://github.com/manticoresoftware/manticoresearch/issues/663) Конфигурация завершается ошибкой: неизвестное имя ключа 'attr_update_reserve'
* [Issue #632](https://github.com/manticoresoftware/manticoresearch/issues/632) Аварийное завершение Manticore при пакетных запросах

* [Issue #679](https://github.com/manticoresoftware/manticoresearch/issues/679) Пакетные запросы снова вызывают сбои в версии v4.0.3

* [Commit f7f8](https://github.com/manticoresoftware/manticoresearch/commit/f7f8bd8c0dbb23a59a88502dcba16b0918fb2330) исправлен сбой демона при запуске при попытке повторного подключения к кластеру с неверным списком узлов
* [Issue #643](https://github.com/manticoresoftware/manticoresearch/issues/643) Manticore 4.0.2 не принимает подключения после серии вставок
* [Issue #635](https://github.com/manticoresoftware/manticoresearch/issues/635) FACET-запрос с ORDER BY JSON.field или строковым атрибутом может вызывать сбой
* [Issue #634](https://github.com/manticoresoftware/manticoresearch/issues/634) Сбой SIGSEGV при запросе с packedfactors
* [Commit 4165](https://github.com/manticoresoftware/manticoresearch/commits/41657f15f8fcf1e93cfad7f835bcf705d6e8349e) morphology_skip_fields не поддерживался при создании таблицы
## Версия 4.0.2, 21 сентября 2021
### Основные новые возможности
- **Полная поддержка [Manticore Columnar Library](https://github.com/manticoresoftware/columnar)**. Ранее библиотека Manticore Columnar поддерживалась только для простых индексов. Теперь поддержка есть:
  - в индексах реального времени для `INSERT`, `REPLACE`, `DELETE`, `OPTIMIZE`

  - в репликации

  - в `ALTER`
  - в `indextool --check`
- **Автоматическая компактификация индексов** ([Issue #478](https://github.com/manticoresoftware/manticoresearch/issues/478)). Наконец-то не нужно вызывать OPTIMIZE вручную или через крон-задание или другой вид автоматизации. Manticore теперь делает это автоматически и по умолчанию. Порог компактификации по умолчанию можно задать через глобальную переменную [optimize_cutoff](Server_settings/Setting_variables_online.md).
- **Полная переработка системы снимков и блокировок чанков**. Эти изменения могут быть сначала незаметны внешне, но значительно улучшают поведение многих операций в индексах реального времени. Вкратце, ранее большинство операций по изменению данных в Manticore сильно полагались на блокировки, теперь используется снятие снимков с дисковых чанков.
- **Значительно более быстрая пакетная вставка в индекс реального времени**. Например, на сервере [Hetzner AX101](https://www.hetzner.com/dedicated-rootserver/ax101) с SSD, 128 ГБ ОЗУ и AMD Ryzen™ 9 5950X (16*2 ядра) **в версии 3.6.0 можно было вставлять 236К документов в секунду** в таблицу со схемой `name text, email string, description text, age int, active bit(1)` (по умолчанию `rt_mem_limit`, размер пакета 25000, 16 параллельных вставок, всего вставлено 16 миллионов документов). В версии 4.0.2 при той же конкуренции/пакете/количестве даётся уже **357К документов в секунду**.
  <details>
  - операции чтения (например SELECT, репликация) выполняются со снимками

  - операции, просто изменяющие внутреннюю структуру индекса без изменения схемы/документов (например, слияние RAM сегментов, сохранение дисковых чанков, слияние дисковых чанков) выполняются с использованием только для чтения снимков и в конце заменяют существующие чанки

  - UPDATE и DELETE выполняются против существующих чанков, но если в процессе слияния, то записи собираются и применяются затем к новым чанкам
  - UPDATE по очереди захватывает эксклюзивную блокировку для каждого чанка. Слияния захватывают разделяемую блокировку при начале этапа сбора атрибутов с чанка. Таким образом, одновременно только одна операция (слияние или обновление) имеет доступ к атрибутам чанка

  - когда слияние достигает стадии, где нужны атрибуты, оно устанавливает специальный флаг. Когда UPDATE заканчивается, оно проверяет этот флаг, и если он установлен, то всё обновление хранится в специальной коллекции. В конце, когда слияние завершается, оно применяет обновления к появившемуся дисковому чанку
  - ALTER выполняется через эксклюзивную блокировку
  - репликация выполняется как обычная операция чтения, но дополнительно сохраняет атрибуты перед SST и запрещает обновления во время SST
  </details>
- **[ALTER](Updating_table_schema_and_settings.md) может добавлять/удалять полнотекстовое поле** (в режиме RT). Ранее можно было только добавлять/удалять атрибут.


- 🔬 **Экспериментально: псевдо-шардинг для полносканирующих запросов** - позволяет параллелить любой запрос, не связанный с полнотекстовым поиском. Вместо подготовки шард вручную теперь можно просто включить новый параметр [searchd.pseudo_sharding](Server_settings/Searchd.md#pseudo_sharding) и ожидать ускорение ответа для таких запросов до количества CPU-ядер. Но это легко может загрузить все доступные CPU ядра, поэтому если вам важна не только задержка, но и пропускная способность, используйте с осторожностью.
### Мелкие изменения

<!-- example -->
- Linux Mint и Ubuntu Hirsute Hippo поддерживаются через [APT репозиторий](Installation/Debian_and_Ubuntu.md#APT-repository)
- ускоренное обновление по id через HTTP в больших индексах в некоторых случаях (зависит от распределения id)

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
- [специальные флаги запуска для systemd](Starting_the_server/Linux.md#Custom-startup-flags-using-systemd). Теперь нет необходимости запускать searchd вручную, если нужно запустить Manticore с каким-либо специфическим флагом запуска

- новая функция [LEVENSHTEIN()](Functions/String_functions.md#LEVENSHTEIN%28%29), которая вычисляет расстояние Левенштейна
- добавлены новые [флаги запуска searchd](Starting_the_server/Manually.md#searchd-command-line-options) `--replay-flags=ignore-trx-errors` и `--replay-flags=ignore-all-errors`, чтобы можно было всё ещё запускать searchd, если binlog повреждён
- [Issue #621](https://github.com/manticoresoftware/manticoresearch/issues/621) - вывод ошибок из RE2
- более точный [COUNT(DISTINCT)](Searching/Grouping.md#COUNT%28DISTINCT-field%29) для распределённых индексов, состоящих из локальных plain-индексов
- [FACET DISTINCT](Searching/Faceted_search.md#Faceting-without-duplicates) для удаления дубликатов при использовании фасетного поиска
- [модификатор exact form](Searching/Full_text_matching/Operators.md#Exact-form-modifier) теперь не требует [морфологии](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) и работает для индексов с включённым [infix/prefix](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) поиском
### Ломающие изменения
- новая версия может читать старые индексы, но старые версии не смогут читать индексы Manticore 4

- удалена неявная сортировка по id. Сортируйте явно, если нужно
- значение по умолчанию `charset_table` изменилось с `0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451` на `non_cjk`
- `OPTIMIZE` теперь происходит автоматически. Если это не нужно, установите `auto_optimize=0` в секции `searchd` в конфигурационном файле
- [Issue #616](https://github.com/manticoresoftware/manticoresearch/issues/616) `ondisk_attrs_default` устарели и теперь удалены
- для контрибьюторов: теперь мы используем компилятор Clang для сборок под Linux, так как по нашим тестам он может собрать более быструю Manticore Search и Manticore Columnar Library
- если [max_matches](Searching/Options.md#max_matches) не указано в поисковом запросе, оно неявно обновляется до минимально необходимого для производительности нового колоночного хранилища значения. Это может повлиять на метрику `total` в [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META), но не на `total_found`, который показывает фактическое количество найденных документов.
### Миграция с Manticore 3
- убедитесь, что вы корректно остановили Manticore 3:
  - в папке `/var/lib/manticore/binlog/` не должно быть binlog файлов (там должен быть только `binlog.meta`)

  - иначе индексы, для которых Manticore 4 не сможет проиграть binlog, не будут запущены
- новая версия может читать старые индексы, но старые версии не могут читать индексы Manticore 4, поэтому сделайте резервную копию, если хотите иметь возможность откатиться легко
- если у вас кластер репликации, убедитесь, что:
  - сначала корректно остановили все узлы
  - затем запустили последний остановленный узел с `--new-cluster` (используйте инструмент `manticore_new_cluster` в Linux)
  - прочитайте про [рестарт кластера](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster) для подробностей
### Исправления багов
- Исправлено множество проблем с репликацией:
  - [Commit 696f](https://github.com/manticoresoftware/manticoresearch/commit/696f8649535f2b5285b878c9581c0d2b1139ae09) - исправлен крах во время SST на узле-джойнере с активным индексом; добавлена проверка sha1 при записи чанков файлов на узле-джойнере для ускорения загрузки индекса; добавлена ротация изменённых файлов индекса на узле-джойнере при загрузке индекса; добавлено удаление файлов индекса на узле-джойнере при замене активного индекса новым из узла-донор; добавлены точки лога репликации на узле-доноре для отправки файлов и чанков
  - [Commit b296](https://github.com/manticoresoftware/manticoresearch/commit/b296c55af68314a8fc66f8b104e7301d673c3b68) - крах при JOIN CLUSTER, если адрес указан неверно
  - [Commit 418b](https://github.com/manticoresoftware/manticoresearch/commit/418bf880a5e05ffbb68f311a1c23006a9df3220e) - при начальной репликации большого индекса узел-джойнер мог упасть с `ERROR 1064 (42000): invalid GTID, (null)`, донор мог перестать отвечать, когда другой узел присоединялся
  - [Commit 6fd3](https://github.com/manticoresoftware/manticoresearch/commit/6fd350d28e4905211cf8a7298cbf7cdda101c83d) - хэш мог вычисляться неправильно для большого индекса, что могло привести к сбою репликации
  - [Issue #615](https://github.com/manticoresoftware/manticoresearch/issues/615) - сбой репликации при рестарте кластера
- [Issue #574](https://github.com/manticoresoftware/manticoresearch/issues/574) - `indextool --help` не отображал параметр `--rotate`
- [Issue #578](https://github.com/manticoresoftware/manticoresearch/issues/578) - высокая загрузка CPU searchd в простое примерно через день
- [Issue #587](https://github.com/manticoresoftware/manticoresearch/issues/587) - немедленный сброс .meta
- [Issue #617](https://github.com/manticoresoftware/manticoresearch/issues/617) - manticore.json очищается
- [Issue #618](https://github.com/manticoresoftware/manticoresearch/issues/618) - searchd --stopwait не срабатывал под root. Также исправлено поведение systemctl (ранее показывало ошибку ExecStop и не ожидало достаточно долго, чтобы searchd корректно завершился)
- [Issue #619](https://github.com/manticoresoftware/manticoresearch/issues/619) - INSERT/REPLACE/DELETE против SHOW STATUS. `command_insert`, `command_replace` и другие показывали неверные метрики
- [Issue #620](https://github.com/manticoresoftware/manticoresearch/issues/620) - `charset_table` для plain-индекса имел неверное значение по умолчанию
- [Commit 8f75](https://github.com/manticoresoftware/manticoresearch/commit/8f7536887d4b58a5bab5647fc067b2d0482c2038) - новые дисковые чанки не попадали в mlock
- [Issue #607](https://github.com/manticoresoftware/manticoresearch/issues/607) - узел кластера Manticore падал, если не удавалось разрешить узел по имени


- [Issue #623](https://github.com/manticoresoftware/manticoresearch/issues/623) - репликация обновлённого индекса могла привести к неопределённому состоянию
- [Commit ca03](https://github.com/manticoresoftware/manticoresearch/commit/ca03d2280c5197e9b311f0f03f551e3b702a8130) - индексатор мог зависать при индексации обычного источника индекса с атрибутом json

- [Commit 53c7](https://github.com/manticoresoftware/manticoresearch/commit/53c753054558ff3f82c7af365273d32537c5338b) - исправлен фильтр выражения "не равно" в индексе PQ
- [Commit ccf9](https://github.com/manticoresoftware/manticoresearch/commit/ccf94e022ba6763d6286ab5bac767b6f6a846846) - исправлен выбор окон в списковых запросах свыше 1000 совпадений. `SELECT * FROM pq ORDER BY id desc LIMIT 1000 , 100 OPTION max_matches=1100` раньше не работал
- [Commit a048](https://github.com/manticoresoftware/manticoresearch/commit/a0483fe9bef3c45dffe96f5df4f4382aee29c072) - HTTPS-запрос к Manticore мог вызывать предупреждение вроде "max packet size(8388608) exceeded"
- [Issue #648](https://github.com/manticoresoftware/manticoresearch/issues/648) - Manticore 3 мог зависать после нескольких обновлений строковых атрибутов

## Version 3.6.0, May 3rd 2021
**Технический релиз перед Manticore 4**
### Основные новшества
- Поддержка [Manticore Columnar Library](https://github.com/manticoresoftware/columnar/) для plain индексов. Новая настройка [columnar_attrs](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#columnar_attrs) для plain индексов
- Поддержка [Украинского лемматизатора](https://github.com/manticoresoftware/lemmatizer-uk)
- Полностью пересмотренные гистограммы. При построении индекса Manticore также строит гистограммы для каждого поля, которые затем используются для более быстрого фильтрования. В версии 3.6.0 алгоритм полностью пересмотрен, вы можете получить более высокую производительность при большом объёме данных и интенсивном фильтровании.
### Небольшие изменения
- утилита `manticore_new_cluster [--force]` полезна для перезапуска репликационного кластера через systemd
- [--drop-src](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Merging_tables.md#Merging-tables) для `indexer --merge`

- [новый режим](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) `blend_mode='trim_all'`
- добавлена [поддержка экранирования JSON пути](Searching/Full_text_matching/Escaping.md#Escaping-json-node-names-in-SQL) обратными кавычками
- [indextool --check](Miscellaneous_tools.md#indextool) может работать в RT-режиме

- [FORCE/IGNORE INDEX(id)](Data_creation_and_modification/Updating_documents/UPDATE.md#Updates-via-SQL) для SELECT/UPDATE
- id чанка для объединённого дискового чанка теперь уникален
- [indextool --check-disk-chunk CHUNK_NAME](Miscellaneous_tools.md#indextool)
### Оптимизации
- [ускоренный парсинг JSON](https://github.com/manticoresoftware/manticoresearch/commit/0c25fc1788e3f77f60cb7fb8f1623d8466bba606), наши тесты показывают снижение задержки на 3-4% для запросов типа `WHERE json.a = 1`
- недокументированная команда `DEBUG SPLIT` как предпосылка для автоматического шардирования/ребалансировки
### Исправления ошибок
- [Issue #584](https://github.com/manticoresoftware/manticoresearch/issues/524) - неточные и нестабильные результаты FACET
- [Issue #506](https://github.com/manticoresoftware/manticoresearch/issues/506) - Странное поведение при использовании MATCH: пострадавшим от этой проблемы нужно перестроить индекс, так как проблема была на этапе построения индекса
- [Issue #387](https://github.com/manticoresoftware/manticoresearch/issues/387) - периодические сбои с дампом при выполнении запроса с функцией SNIPPET()
- Оптимизации стека полезны для обработки сложных запросов:
  - [Issue #469](https://github.com/manticoresoftware/manticoresearch/issues/469) - SELECT вызывает CRASH DUMP
  - [e8420cc7](https://github.com/manticoresoftware/manticoresearch/tree/e8420cc789149c84731be0874be916e33b263bd6) - определение размера стека для деревьев фильтров
- [Issue #461](https://github.com/manticoresoftware/manticoresearch/issues/461) - Обновление с использованием условия IN не работает корректно
- [Issue #464](https://github.com/manticoresoftware/manticoresearch/issues/464) - SHOW STATUS сразу после CALL PQ возвращает - [Issue #481](https://github.com/manticoresoftware/manticoresearch/issues/481) - Исправлена статическая сборка бинарников
- [Issue #502](https://github.com/manticoresoftware/manticoresearch/issues/502) - ошибка в мультизапросах
- [Issue #514](https://github.com/manticoresoftware/manticoresearch/issues/514) - Невозможно использовать необычные имена для столбцов при использовании 'create table'
- [Commit d1db](https://github.com/manticoresoftware/manticoresearch/commit/d1dbe771288328c963ca8512f564c6b7cf4f466c) - аварийный сбой демона при повторе binlog с обновлением строкового атрибута; установлен бинлог версии 10
- [Commit 775d](https://github.com/manticoresoftware/manticoresearch/commit/775d0555562f121911d3c30085947e743fadac2d)  - исправлено определение времени выполнения стекового кадра выражения (тест 207)
- [Commit 4795](https://github.com/manticoresoftware/manticoresearch/commit/4795dc49194c7745761bb5006ecb266e8a9224fa)  - фильтр индексатора percolate и теги были пустыми для пустого сохраненного запроса (тест 369)
- [Commit c3f0](https://github.com/manticoresoftware/manticoresearch/commit/c3f0bf4dca0e4eae6ea899dd566878df57eba085)  - прерывания потока репликации SST при сетях с большой задержкой и высоким уровнем ошибок (репликация между разными дата-центрами); обновлена версия команды репликации до 1.03
- [Commit ba2d](https://github.com/manticoresoftware/manticoresearch/commit/ba2d6619b8107f3fb177741bbdff3709bac248cd)  - блокировка кластера joiner на операции записи после присоединения к кластеру (тест 385)
- [Commit de4d](https://github.com/manticoresoftware/manticoresearch/commit/de4dcb9ff6c316bc2a9ef6c62eb64769f04ee103)  - сопоставление шаблонов с модификатором exact (тест 321)
- [Commit 6524](https://github.com/manticoresoftware/manticoresearch/commit/6524fc6af032a0932bd42c37874a619e1fb7f075)  - контрольные точки docid против docstore
- [Commit f4ab](https://github.com/manticoresoftware/manticoresearch/commit/f4ab83c291e65a5873d271c67b5491f629544f1d)  - непоследовательное поведение индексатора при парсинге некорректного xml
- [Commit 7b72](https://github.com/manticoresoftware/manticoresearch/commit/7b727e22fe01b1a180d4a8d3a3fea92c18eac707)  - сохранённый percolate запрос с NOTNEAR выполняется вечно (тест 349)
- [Commit 812d](https://github.com/manticoresoftware/manticoresearch/commit/812dab745f0c972ecde5d5b3d4a313cd6e1e143e)  - неправильный вес для фразы, начинающейся с wildcard
- [Commit 1771](https://github.com/manticoresoftware/manticoresearch/commit/1771afc669c75670b3511bf02ab9bfc8982b94d2)  - percolate запрос с wildcard генерирует термы без payload при совпадении, что вызывает смешанные результаты и нарушает сопоставление (тест 417)
- [Commit aa0d](https://github.com/manticoresoftware/manticoresearch/commit/aa0d8c2bcb41428d41dbfb66e42a872a79a4ee4b)  - исправлен расчёт 'total' в случае параллельного запроса
- [Commit 18d8](https://github.com/manticoresoftware/manticoresearch/commit/18d81b3cfbb930b7361dae645fe13283d4fbf6ef)  - сбой в Windows при множественных одновременных сессиях на демоне
- [Commit 8443](https://github.com/manticoresoftware/manticoresearch/commit/84432f23d6fbf7a7d0b67c8c9412eb5933b03596)  - некоторые настройки индекса не могли реплицироваться
- [Commit 9341](https://github.com/manticoresoftware/manticoresearch/commit/93411fe6af4246ca4acce758944d513a1660b92b)  - при высокой скорости добавления новых событий netloop иногда зависает из-за атомарного события 'kick', обрабатываемого одновременно для нескольких событий и теряющего реальные действия из них
статус запроса, а не статус сервера
- [Commit d805](https://github.com/manticoresoftware/manticoresearch/commit/d805fc125244ef08d14fb3d1826e2c356862d539)  - новый сброшенный диск-чанк может быть потерян при коммите

- [Commit 63cb](https://github.com/manticoresoftware/manticoresearch/commit/63cbf008d50979cdf7062664fb02a672a3bb1131)  - неточный 'net_read' в профилировщике
- [Commit f537](https://github.com/manticoresoftware/manticoresearch/commit/f5379bb27234b25919966c50ccb6f469ead646e2)  - проблема Percolate с арабским (тексты справа налево)
- [Commit 49ee](https://github.com/manticoresoftware/manticoresearch/commit/49eeb4202d4ce4875c07c35087d7108c1e177328)  - id неверно выбирался при дублирующемся имени колонки
- [Commit refa](https://github.com/manticoresoftware/manticoresearch/commit/4b7df262bbfec9b9e95a44e72ad326ac8a786c49) исправление событий сети для устранения краха в редких случаях
- [e8420cc7](https://github.com/manticoresoftware/manticoresearch/tree/e8420cc789149c84731be0874be916e33b263bd6) исправление в `indextool --dumpheader`
- [Commit ff71](https://github.com/manticoresoftware/manticoresearch/commit/ff7163532ed39042c0d3c3c2b902ff41f94d7f0c) - TRUNCATE WITH RECONFIGURE работал неправильно с сохранёнными полями

### Критические изменения:
- Новый формат binlog: перед обновлением необходимо корректно остановить Manticore
- Формат индекса немного изменился: новая версия без проблем читает существующие индексы, но при понижении версии с 3.6.0 более новые индексы будут нечитаемы

- Изменение формата репликации: не реплицируйте с более старой версии на 3.6.0 и обратно, обновляйте все узлы одновременно

- `reverse_scan` устарел. Убедитесь, что не используете эту опцию в запросах с версии 3.6.0, иначе они будут выдавать ошибку
- С этого релиза мы не предоставляем сборки для RHEL6, Debian Jessie и Ubuntu Trusty. Если для вас критична их поддержка, [свяжитесь с нами](https://manticoresearch.com/contact-us/)
### Устаревшие функции
- Больше нет неявной сортировки по id. Если вы на неё полагаетесь, обновите запросы соответственно

- Опция поиска `reverse_scan` была устаревшей
## Версия 3.5.4, 10 декабря 2020
### Новые возможности

- Новые клиенты Python, Javascript и Java теперь общедоступны и хорошо документированы в этом руководстве.
- автоматическое удаление диск-чанка из real-time индекса. Эта оптимизация позволяет автоматически удалять диск-чанк при [OPTIMIZING](Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE) real-time индекса, когда чанки явно больше не нужны (все документы удалены). Ранее требовалось слияние, теперь чанки можно просто моментально удалить. Опция [cutoff](Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks) игнорируется, т.е. даже без фактического слияния устаревший диск-чанк удаляется. Это полезно, если вы поддерживаете ретеншн в индексе и удаляете старые документы. Теперь компактизация таких индексов будет быстрее.
- [standalone NOT](Searching/Options.md#not_terms_only_allowed) в качестве опции для SELECT

### Незначительные изменения
- [Issue #453](https://github.com/manticoresoftware/manticoresearch/issues/453) Новая опция [indexer.ignore_non_plain=1](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#ignore_non_plain) полезна, если вы запускаете `indexer --all` и у вас есть не только plain индексы в конфигурации. Без `ignore_non_plain=1` будет предупреждение и соответствующий код выхода.
- [SHOW PLAN ... OPTION format=dot](Node_info_and_management/Profiling/Query_plan.md#Dot-format-for-SHOW-PLAN) и [EXPLAIN QUERY ... OPTION format=dot](Searching/Full_text_matching/Profiling.md#Profiling-without-running-a-query) позволяют визуализировать план выполнения полнотекстового запроса. Полезно для понимания сложных запросов.
### Устаревшие функции
- `indexer --verbose` устарел, так как не добавлял ничего к выводу индексатора
- Для дампа бэктрейса watchdog теперь используется сигнал `USR2` вместо `USR1`
### Исправления ошибок
- [Issue #423](https://github.com/manticoresoftware/manticoresearch/issues/423) кириллическая точка в режиме вызова сниппетов не выделялась
- [Issue #435](https://github.com/manticoresoftware/manticoresearch/issues/435) RTINDEX - GROUP N BY выражение select = критический сбой
- [Commit 2b3b](https://github.com/manticoresoftware/manticoresearch/commit/2b3b62bd0ce2b54544733d524a169e81f71b5388) статус searchd показывает Segmentation fault в кластере


- [Commit 9dd2](https://github.com/manticoresoftware/manticoresearch/commit/9dd25c193ed2a53750a0e0e68566fd2b833fcb0c) 'SHOW INDEX index.N SETTINGS' не отображал чанки >9

- [Issue #389](https://github.com/manticoresoftware/manticoresearch/issues/389) ошибка, приводящая к сбою Manticore

- [Commit fba1](https://github.com/manticoresoftware/manticoresearch/commit/fba166178ff35c73a0c70d131cf3aca4a635e961) Конвертер создает поврежденные индексы
- [Commit eecd](https://github.com/manticoresoftware/manticoresearch/commit/eecd61d861c742471175be19a5e5455034b5db5f) stopword_step=0 против CALL SNIPPETS()
- [Commit ea68](https://github.com/manticoresoftware/manticoresearch/commit/ea6850e449601199190f38a387c04b8a7b7e90f6) count distinct возвращает 0 при низком max_matches на локальном индексе
- [Commit 362f](https://github.com/manticoresoftware/manticoresearch/commit/362f27db3421f09a41c6cce04b5baace00fa4c4d) При использовании агрегирования хранимые тексты не возвращаются в результатах
## Version 3.5.2, Oct 1 2020
### Новые возможности

* OPTIMIZE уменьшает количество дисковых чанков до числа чанков (по умолчанию `2 * Количество ядер`), а не до одного. Оптимальное количество чанков можно контролировать с помощью опции [cutoff](Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks).

* Оператор NOT теперь можно использовать самостоятельно. По умолчанию он отключен, так как случайные одиночные запросы NOT могут работать медленно. Его можно включить, установив новый директив searchd [not_terms_only_allowed](Server_settings/Searchd.md#not_terms_only_allowed) в значение `0`.
* Новая настройка [max_threads_per_query](Server_settings/Searchd.md#max_threads_per_query) определяет, сколько потоков может использовать запрос. Если директива не установлена, запрос может использовать потоки вплоть до значения [threads](Server_settings/Searchd.md#threads).
Для каждого `SELECT` запроса количество потоков можно ограничить с помощью [OPTION threads=N](Searching/Options.md#threads), переопределяющего глобальный `max_threads_per_query`.
* Теперь можно импортировать перколяторные индексы с помощью [IMPORT TABLE](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Importing_table.md).

* HTTP API `/search` получает базовую поддержку [фасетирования](Searching/Faceted_search.md#HTTP-JSON)/[группировки](Searching/Grouping.md) через новый узел запроса `aggs`.

### Незначительные изменения

* Если директива replication listen не объявлена, движок будет пытаться использовать порты после заданного порта 'sphinx', до 200.

* `listen=...:sphinx` необходимо явно указывать для соединений SphinxSE или клиентов SphinxAPI.

* [SHOW INDEX STATUS](Node_info_and_management/Table_settings_and_status/SHOW_TABLE_STATUS.md) выводит новые метрики: `killed_documents`, `killed_rate`, `disk_mapped_doclists`, `disk_mapped_cached_doclists`, `disk_mapped_hitlists` и `disk_mapped_cached_hitlists`.

* SQL-команда `status` теперь выводит `Queue\Threads` и `Tasks\Threads`.

### Устаревшее:
* `dist_threads` полностью устарел, searchd будет выводить предупреждение, если директива все еще используется.
### Docker
Официальный образ Docker теперь основан на Ubuntu 20.04 LTS
### Упаковка
Помимо обычного пакета `manticore` теперь можно устанавливать Manticore Search по компонентам:

- `manticore-server-core` - обеспечивает `searchd`, manpage, каталог логов, API и модуль galera. Также установит `manticore-common` как зависимость.

- `manticore-server` - обеспечивает скрипты автоматизации для ядра (init.d, systemd) и оболочку `manticore_new_cluster`. Также установит `manticore-server-core` как зависимость.
- `manticore-common` - обеспечивает конфиги, стоп-слова, общие документы и скелетные папки (datadir, modules и т.д.)
- `manticore-tools` - обеспечивает вспомогательные инструменты ( `indexer`, `indextool` и пр.), их man-страницы и примеры. Также установит `manticore-common` как зависимость.
- `manticore-icudata` (RPM) или `manticore-icudata-65l` (DEB) - обеспечивает ICU data файл для использования морфологии icu.
- `manticore-devel` (RPM) или `manticore-dev` (DEB) - обеспечивает dev-заголовки для UDF.
### Исправления ошибок
1. [Commit 2a47](https://github.com/manticoresoftware/manticoresearch/commit/2a474dc1a26e8b0f8aaaae95669caf2f1d4b7746) Сбой демона при группировщике для RT индекса с разными чанками
2. [Commit 57a1](https://github.com/manticoresoftware/manticoresearch/commit/57a19e5ad5663ef0ca7436595218fb1221d28c8e) Быстрый путь для пустых удаленных документов
3. [Commit 07dd](https://github.com/manticoresoftware/manticoresearch/commit/07dd3f313c63fb82c22092f9907ef24e3475250e) Время выполнения обнаружения фрейма стека выражения
4. [Commit 08ae](https://github.com/manticoresoftware/manticoresearch/commit/08ae357cf1012bc8e2da54c20b205b592efda3d4) Масчтабное совпадение более чем по 32 полям для перколяторных индексов
5. [Commit 16b9](https://github.com/manticoresoftware/manticoresearch/commit/16b9390fd4cdb07a77ac4497adb935573a1710e5) Диапазон портов для replication listen
6. [Commit 5fa6](https://github.com/manticoresoftware/manticoresearch/commit/5fa671affeacb6441a59c8a88479bfd423df7c81) Show create table на pq
7. [Commit 54d1](https://github.com/manticoresoftware/manticoresearch/commit/54d133b6449105a9fb0168db3f1fbb05fb5aa1f6) Поведение HTTPS порта
8. [Commit fdbb](https://github.com/manticoresoftware/manticoresearch/commit/fdbbe5245cc296cc5c1ae3ae2fb9cb08fb66a248) Перемешивание строк docstore при замене
9. [Commit afb5](https://github.com/manticoresoftware/manticoresearch/commit/afb53f648ef4e64fa1776c58e66f6e716ac730ad) Уровень сообщения недоступности TFO переключен на 'info'
10. [Commit 59d9](https://github.com/manticoresoftware/manticoresearch/commit/59d94cefc6e88af360b8046726a16b7eaa9f0b71) Сбой при неверном использовании strcmp
11. [Commit 04af](https://github.com/manticoresoftware/manticoresearch/commit/04af0349c1e784c2230d204592d6272b440bb375) Добавление индекса в кластер с системными (stopwords) файлами
12. [Commit 5014](https://github.com/manticoresoftware/manticoresearch/commit/50148b4eae9f3a924082005cf5d2f9c8339af7f5) Слияние индексов с большими словарями; RT оптимизация больших дисковых чанков
13. [Commit a2ad](https://github.com/manticoresoftware/manticoresearch/commit/a2adf1582843f63d4dcc1dab8f7598e0917094a2) Indextool теперь может выгружать метаинформацию из текущей версии

14. [Commit 69f6](https://github.com/manticoresoftware/manticoresearch/commit/69f6d5f77c64b97e3f395aea8c555b11c77fb732) Проблема с порядком групп при GROUP N

15. [Commit 24d5](https://github.com/manticoresoftware/manticoresearch/commit/24d5d80f508576794b751d70f9f9b524564e3c14) Явный flush для SphinxSE после рукопожатия
16. [Коммит 31c4](https://github.com/manticoresoftware/manticoresearch/commit/31c4d78ac03ee85d9a02b86075448a04ac80f9e2) Избегать копирования огромных описаний, когда это не нужно
17. [Коммит 2959](https://github.com/manticoresoftware/manticoresearch/commit/2959e2caa8610f98fcdf2e5a82160f58687f4a27) Отрицательное время в show threads
18. [Коммит f0b3](https://github.com/manticoresoftware/manticoresearch/commit/f0b357102c70f3ac1d7da260c60b67322940e647) Плагин фильтра токенов против нулевых дельт позиций
19. [Коммит a49e](https://github.com/manticoresoftware/manticoresearch/commit/a49e5bc13ea763e6f01dd5993e891858a84d1846) Изменить 'FAIL' на 'WARNING' при множественных совпадениях
## Версия 3.5.0, 22 июля 2020
### Основные новые возможности:

* Этот релиз занял так много времени, потому что мы усердно работали над изменением режима многозадачности с потоков на **корутины**. Это упрощает конфигурацию и делает распараллеливание запросов гораздо более прямолинейным: Manticore просто использует заданное количество потоков (см. новую настройку [threads](Server_settings/Searchd.md#threads)) и новый режим гарантирует, что это сделано максимально оптимально.

* Изменения в [выделении](Searching/Highlighting.md#Highlighting-options):

  - любое выделение, которое работает с несколькими полями (`highlight({},'field1, field2'`) или `highlight` в json-запросах) теперь по умолчанию применяет ограничения по полю.

  - любое выделение, которое работает с простым текстом (`highlight({}, string_attr)` или `snippet()`) теперь применяет ограничения ко всему документу.
  - [ограничения по полю](Searching/Highlighting.md#limits_per_field) могут быть переключены на глобальные ограничения опцией `limits_per_field=0` (по умолчанию `1`).
  - [allow_empty](Searching/Highlighting.md#allow_empty) теперь по умолчанию `0` для выделения через HTTP JSON.
* Теперь один и тот же порт [можно использовать](Server_settings/Searchd.md#listen) для http, https и бинарного API (для приёма соединений с удалённого экземпляра Manticore). `listen = *:mysql` всё ещё требуется для соединений по mysql-протоколу. Manticore теперь автоматически определяет тип клиента, пытающегося подключиться, за исключением MySQL (из-за ограничений протокола).
* В режиме RT поле теперь может быть одновременно [текстовым и строковым атрибутом](Creating_a_table/Data_types.md#String) - [GitHub issue #331](https://github.com/manticoresoftware/manticoresearch/issues/331).
  В [plain режиме](Read_this_first.md#Real-time-mode-vs-plain-mode) это называется `sql_field_string`. Теперь оно доступно и в [RT режиме](Read_this_first.md#Real-time-mode-vs-plain-mode) для индексов реального времени. Вы можете использовать это, как показано в примере:
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
### Мелкие изменения
* Теперь вы можете [выделять строковые атрибуты](Searching/Highlighting.md#Highlighting-via-SQL).
* Поддержка SSL и сжатия для SQL интерфейса.
* Поддержка команды `status` клиента mysql [`status`](Node_info_and_management/Node_status.md#STATUS).
* [Репликация](Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Setting-up-replication) теперь может реплицировать внешние файлы (стоп-слова, исключения и прочее).
* Оператор фильтра [`in`](Searching/Filters.md#Set-filters) теперь доступен через HTTP JSON интерфейс.
* [`expressions`](Searching/Expressions.md#expressions) в HTTP JSON.
* Теперь вы можете [изменять `rt_mem_limit` на лету](https://github.com/manticoresoftware/manticoresearch/issues/344) в RT режиме, то есть выполняя `ALTER ... rt_mem_limit=<новое значение>`.

* Теперь вы можете использовать [отдельные таблицы кодировок CJK](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology): `chinese`, `japanese` и `korean`.
* [thread_stack](Server_settings/Searchd.md#thread_stack) теперь ограничивает максимальный стек потока, а не начальный.
* Улучшен вывод `SHOW THREADS`.
* Отображение прогресса длительного `CALL PQ` в `SHOW THREADS`.
* cpustat, iostat, coredump теперь можно менять во время выполнения с помощью [SET](Server_settings/Setting_variables_online.md#SET).

* Реализована команда `SET [GLOBAL] wait_timeout=NUM`,
### Изменения, нарушающие обратную совместимость:
* **Формат индекса был изменён.** Индексы, созданные в 3.5.0, не могут быть загружены версиями Manticore ниже 3.5.0, но Manticore 3.5.0 поддерживает старые форматы.
* [`INSERT INTO PQ VALUES()`](Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md) (то есть без указания списка столбцов) раньше ожидал точно `(query, tags)` как значения. Теперь изменён на `(id,query,tags,filters)`. ID можно установить в 0, если хотите, чтобы он задавался автоматически.
* [`allow_empty=0`](Searching/Highlighting.md#allow_empty) теперь новый дефолт для выделения через HTTP JSON интерфейс.
* Для внешних файлов (стоп-слова, исключения и прочее) в `CREATE TABLE`/`ALTER TABLE` допускаются только абсолютные пути.
### Устаревшее:
* `ram_chunks_count` был переименован в `ram_chunk_segments_count` в `SHOW INDEX STATUS`.
* `workers` устарел. Сейчас есть только один режим workers.
* `dist_threads` устарел. Все запросы выполняются максимально параллельно (ограничены `threads` и `jobs_queue_size`).
* `max_children` устарел. Используйте [threads](Server_settings/Searchd.md#threads) для установки количества потоков, которые Manticore будет использовать (по умолчанию равно количеству ядер процессора).
* `queue_max_length` устарел. Вместо него при необходимости используйте [jobs_queue_size](Server_settings/Searchd.md#jobs_queue_size) для точной настройки внутреннего размера очереди заданий (по умолчанию неограниченно).
* Все эндпоинты `/json/*` теперь доступны без `/json/`, например `/search`, `/insert`, `/delete`, `/pq` и т. д.
* Значение `field` как «поле полнотекстового поиска» было переименовано в «text» в `describe`.
  <!-- more -->
  **3.4.2:**
  ```sql
  mysql> describe t;
  +-------+--------+----------------+
  | Поле | Тип   | Свойства     |
  +-------+--------+----------------+
  | id    | bigint |                |
  | f     | field  | indexed stored |
  +-------+--------+----------------+
  ```
  **3.5.0**:
  ```sql
  mysql> describe t;
  +-------+--------+----------------+
  | Поле  | Тип    | Свойства      |
  +-------+--------+----------------+
  | id    | bigint |                |


  | f     | text   | indexed stored |

  +-------+--------+----------------+
  ```

  <!-- \more -->
* Кириллическая `и` не отображается как `i` в `non_cjk` charset_table (что является значением по умолчанию), так как это слишком сильно влияло на русские стеммеры и лемматизаторы.
* `read_timeout`. Используйте вместо этого [network_timeout](Server_settings/Searchd.md#network_timeout), который управляет и чтением, и записью.
### Пакеты
* Официальный пакет Ubuntu Focal 20.04
* Имя deb-пакета изменено с `manticore-bin` на `manticore`
### Исправления ошибок:
1. [Issue #351](https://github.com/manticoresoftware/manticoresearch/issues/351) утечка памяти в searchd
2. [Commit ceab](https://github.com/manticoresoftware/manticoresearch/commit/ceabe44f) Незначительный выход за границы при чтении в сниппетах
3. [Commit 1c3e](https://github.com/manticoresoftware/manticoresearch/commit/1c3e84a3) Опасная запись в локальную переменную при запросах с ошибками
4. [Commit 26e0](https://github.com/manticoresoftware/manticoresearch/commit/26e094ab) Незначительная утечка памяти сортировщика в тесте 226
5. [Commit d2c7](https://github.com/manticoresoftware/manticoresearch/commit/d2c7f86a) Большая утечка памяти в тесте 226
6. [Commit 0dd8](https://github.com/manticoresoftware/manticoresearch/commit/0dd80122) Кластер показывает, что узлы синхронизированы, но `count(*)` показывает разные значения
7. [Commit f1c1](https://github.com/manticoresoftware/manticoresearch/commit/f1c1ac3f) Косметические: дублирование и иногда потеря предупреждающих сообщений в логе
8. [Commit f1c1](https://github.com/manticoresoftware/manticoresearch/commit/f1c1ac3f) Косметические: (null) имя индекса в логе
9. [Commit 359d](https://github.com/manticoresoftware/manticoresearch/commit/359dbd30) Невозможно получить более 70 миллионов результатов
10. [Commit 19f3](https://github.com/manticoresoftware/manticoresearch/commit/19f328ee) Нельзя вставлять правила PQ без синтаксиса колонок
11. [Commit bf68](https://github.com/manticoresoftware/manticoresearch/commit/bf685d5d) Вводит в заблуждение сообщение об ошибке при вставке документа в кластерный индекс
12. [Commit 2cf1](https://github.com/manticoresoftware/manticoresearch/commit/2cf18c83) `/json/replace` и `json/update` возвращают id в экспоненциальной форме
13. [Issue #324](https://github.com/manticoresoftware/manticoresearch/issues/324) Обновление скалярных свойств json и mva в одном запросе
14. [Commit d384](https://github.com/manticoresoftware/manticoresearch/commit/d38409eb) `hitless_words` не работает в режиме RT
15. [Commit 5813](https://github.com/manticoresoftware/manticoresearch/commit/5813d639) В режиме rt использование `ALTER RECONFIGURE` должно быть запрещено
16. [Commit 5813](https://github.com/manticoresoftware/manticoresearch/commit/5813d639) `rt_mem_limit` сбрасывается до 128M после перезапуска searchd
17. highlight() иногда зависает
18. [Commit 7cd8](https://github.com/manticoresoftware/manticoresearch/commit/7cd878f4) Не удалось использовать U+код в режиме RT
19. [Commit 2b21](https://github.com/manticoresoftware/manticoresearch/commit/2b213de4) Не удалось использовать подстановочный знак в wordforms в режиме RT
20. [Commit e9d0](https://github.com/manticoresoftware/manticoresearch/commit/e9d07e68) Исправлен `SHOW CREATE TABLE` для нескольких файлов wordform
21. [Commit fc90](https://github.com/manticoresoftware/manticoresearch/commit/fc90a84f) JSON-запрос без "query" вызывает крах searchd
22. Официальный [docker Manticore](https://hub.docker.com/r/manticoresearch/manticore) не мог индексировать из mysql 8
23. [Commit 23e0](https://github.com/manticoresoftware/manticoresearch/commit/23e05d32) HTTP /json/insert требует наличие id
24. [Commit bd67](https://github.com/manticoresoftware/manticoresearch/commit/bd679af0) `SHOW CREATE TABLE` не работает для PQ
25. [Commit bd67](https://github.com/manticoresoftware/manticoresearch/commit/bd679af0) `CREATE TABLE LIKE` неправильно работает с PQ
26. [Commit 5eac](https://github.com/manticoresoftware/manticoresearch/commit/5eacf28f) Конец строки в настройках в show index status
27. [Commit cb15](https://github.com/manticoresoftware/manticoresearch/commit/cb153228) Пустой заголовок в "highlight" в HTTP JSON ответе
28. [Issue #318](https://github.com/manticoresoftware/manticoresearch/issues/318) Ошибка `CREATE TABLE LIKE` infix
29. [Commit 9040](https://github.com/manticoresoftware/manticoresearch/commit/9040d22c) RT крашится под нагрузкой
30. [cd512c7d ](https://github.com/manticoresoftware/manticoresearch/commit/cd512c7d ) Потерян лог краша при сбое в RT disk chunk
31. [Issue #323](https://github.com/manticoresoftware/manticoresearch/issues/323) Импорт таблицы неудачен и закрывает соединение
32. [Commit 6275](https://github.com/manticoresoftware/manticoresearch/commit/6275316a) `ALTER reconfigure` повреждает индекс PQ
33. [Commit 9c1d](https://github.com/manticoresoftware/manticoresearch/commit/9c1d221e) Проблемы с перезагрузкой searchd после изменения типа индекса
34. [Commit 71e2](https://github.com/manticoresoftware/manticoresearch/commit/71e2b5bb) Демон падает при импортировании таблицы с отсутствующими файлами
35. [Issue #322](https://github.com/manticoresoftware/manticoresearch/issues/322) Краш при select с использованием нескольких индексов, group by и ranker = none
36. [Commit c3f5](https://github.com/manticoresoftware/manticoresearch/commit/c3f58490) `HIGHLIGHT()` не подсвечивает строковые атрибуты
37. [Issue #320](https://github.com/manticoresoftware/manticoresearch/issues/320) `FACET` не сортирует по строковому атрибуту
38. [Commit 4f1a](https://github.com/manticoresoftware/manticoresearch/commit/4f1a1f25) Ошибка при отсутствии директории данных
39. [Commit 04f4](https://github.com/manticoresoftware/manticoresearch/commit/04f4ddd4) access_* не поддерживаются в режиме RT
40. [Коммит 1c06](https://github.com/manticoresoftware/manticoresearch/commit/1c0616a2) Плохие объекты JSON в строках: 1. `CALL PQ` возвращает "Bad JSON objects in strings: 1", когда json больше некоторого значения.
41. [Коммит 32f9](https://github.com/manticoresoftware/manticoresearch/commit/32f943d6) Несоответствие в режиме RT. В некоторых случаях не могу удалить индекс, так как он неизвестен, и не могу создать его, так как директория не пуста.
42. [Проблема #319](https://github.com/manticoresoftware/manticoresearch/issues/319) Сбой при select
43. [Коммит 22a2](https://github.com/manticoresoftware/manticoresearch/commit/22a28dd7) `max_xmlpipe2_field` = 2M возвращал предупреждение при поле 2M
44. [Проблема #342](https://github.com/manticoresoftware/manticoresearch/issues/342) Ошибка выполнения условий запроса
45. [Коммит dd8d](https://github.com/manticoresoftware/manticoresearch/commit/dd8dcab2) Простой поиск из 2 терминов находит документ, содержащий только один термин
46. [Коммит 9091](https://github.com/manticoresoftware/manticoresearch/commit/90919e62) В PQ было невозможно сопоставить json с заглавными буквами в ключах
47. [Коммит 56da](https://github.com/manticoresoftware/manticoresearch/commit/56da086a) Индексатор падает при csv+docstore
48. [Проблема #363](https://github.com/manticoresoftware/manticoresearch/issues/363) использование `[null]` в json атрибуте в centos 7 вызывает повреждение вставленных данных
49. Критическая [Проблема #345](https://github.com/manticoresoftware/manticoresearch/issues/345) Записи не вставляются, count() выдаёт случайные значения, "replace into" возвращает OK
50. max_query_time слишком сильно замедляет SELECT


51. [Проблема #352](https://github.com/manticoresoftware/manticoresearch/issues/352) Связь мастер-агент не работает на Mac OS
52. [Проблема #328](https://github.com/manticoresoftware/manticoresearch/issues/328) Ошибка при подключении к Manticore с Connector.Net/Mysql 8.0.19
53. [Коммит daa7](https://github.com/manticoresoftware/manticoresearch/commit/daa760d2fe83668038318f0f384ceefc0d075e18) Исправлено экранирование \0 и оптимизирована производительность

54. [Коммит 9bc5](https://github.com/manticoresoftware/manticoresearch/commit/9bc5c01a493db993b90ff6c1ad51aeae028f629c) Исправлен count distinct для json
55. [Коммит 4f89](https://github.com/manticoresoftware/manticoresearch/commit/4f89a965a3af0db548ae9d1763731ebaebe21f5a) Исправлена ошибка при удалении таблицы на другом узле
56. [Коммит 952a](https://github.com/manticoresoftware/manticoresearch/commit/952af5a5931fd72b87edc169588777778b365231) Исправлены сбои при частом вызове call pq
## Версия 3.4.2, 10 апреля 2020
### Критические исправления ошибок
* [Коммит 2ffe](https://github.com/manticoresoftware/manticoresearch/commit/2ffe2d261793f9681265bc4abe38d695e72c121e) исправлена ошибка: RT индекс из старой версии не индексирует данные

## Версия 3.4.0, 26 марта 2020
### Основные изменения
* сервер работает в 2 режимах: rt-режим и plain-режим
   *   rt-режим требует data_dir и отсутствия определения индекса в конфигурации
   *   в plain-режиме индексы определены в конфигурации; data_dir не разрешён
* репликация доступна только в rt-режиме

### Незначительные изменения
* charset_table по умолчанию на алиасе non_cjk
* в rt-режиме полнотекстовые поля индексируются и хранятся по умолчанию

* полнотекстовые поля в rt-режиме переименованы с 'field' в 'text'
* ALTER RTINDEX переименован в ALTER TABLE
* TRUNCATE RTINDEX переименован в TRUNCATE TABLE
### Функции
* поля только для хранения

* SHOW CREATE TABLE, IMPORT TABLE
### Улучшения
* гораздо более быстрый lockless PQ
* /sql может выполнять любые SQL-запросы в режиме mode=raw
* псевдоним mysql для протокола mysql41
* файл состояния state.sql по умолчанию в data_dir
### Исправления ошибок
* [Коммит a533](https://github.com/manticoresoftware/manticoresearch/commit/a5333644e8e0c69746a9f24da8221fc965478e4a) исправлен сбой при неправильном синтаксисе поля в highlight()
* [Коммит 7fbb](https://github.com/manticoresoftware/manticoresearch/commit/7fbb9f2e618096289cba7f8b834530b46aa94055) исправлен сбой сервера при репликации RT индекса с docstore
* [Коммит 24a0](https://github.com/manticoresoftware/manticoresearch/commit/24a04687eb43b4f4719c7c4a5fe3c19099240467) исправлен сбой при highlight к индексу с опциями infix или prefix и к индексу без включённых хранимых полей
* [Коммит 3465](https://github.com/manticoresoftware/manticoresearch/commit/3465c1ce9c860c09f6ccc5b83a63e3c68c241d8d) исправлена ложная ошибка о пустом docstore и проверке dock-id для пустого индекса
* [Коммит a707](https://github.com/manticoresoftware/manticoresearch/commit/b8601b4144b9092be43a7caa7cd6b19cb37e14c7) исправлена проблема #314: SQL insert с завершающей точкой с запятой
* [Коммит 9562](https://github.com/manticoresoftware/manticoresearch/commit/95628c9b9b8f16fe1bd18599a84e9673575adf1a) удалено предупреждение при несовпадении слов запроса

* [Коммит b860](https://github.com/manticoresoftware/manticoresearch/commit/b8601b4144b9092be43a7caa7cd6b19cb37e14c7) исправлены запросы в сниппетах с сегментацией через ICU
* [Коммит 5275](https://github.com/manticoresoftware/manticoresearch/commit/5275516ca4631c584e6e0127fa161a18d2658fd0) исправлена ошибка гонки при поиске/добавлении в кэш блоков docstore
* [Коммит f06e](https://github.com/manticoresoftware/manticoresearch/commit/f06ef97af5c7390427e82525c150535290e42c3e) исправлена утечка памяти в docstore
* [Коммит a725](https://github.com/manticoresoftware/manticoresearch/commit/a7258ba8340e246c0d8aaeb4a7a74129eba807eb) исправлена проблема #316: LAST_INSERT_ID возвращает пустое значение при INSERT
* [Коммит 1ebd](https://github.com/manticoresoftware/manticoresearch/commit/1ebd5bf8d74113bcfa138ce63ce11966990a4215) исправлена проблема #317: конечная точка json/update HTTP теперь поддерживает массив для MVA и объект для JSON атрибута
* [Коммит e426](https://github.com/manticoresoftware/manticoresearch/commit/e426950a536382a142e2a42ed207c341149ff685) исправлена ошибка индексатора при дампе rt без явного id
## Версия 3.3.0, 4 февраля 2020

### Функции
* Параллельный поиск в Real-Time индексах
* Команда EXPLAIN QUERY
* конфигурационный файл без определений индексов (альфа-версия)
* команды CREATE/DROP TABLE (альфа-версия)
* indexer --print-rt - может читать из источника и печатать INSERT для Real-Time индекса
### Улучшения


* Обновлены стеммеры до Snowball 2.0
* LIKE фильтр для SHOW INDEX STATUS
* улучшено использование памяти при большом max_matches
* SHOW INDEX STATUS добавляет ram_chunks_count для RT индексов
* lockless PQ
* изменено LimitNOFILE на 65536
### Исправления ошибок
* [Коммит 9c33](https://github.com/manticoresoftware/manticoresearch/commit/9c33aab8c1fd9059c8644d23d5f58f03010043c6) добавлена проверка схемы индекса на дублирующиеся атрибуты #293
* [Коммит a008](https://github.com/manticoresoftware/manticoresearch/commit/a0085f9477e191da9e0e835b91583fb9cfde698e) исправлена ошибка при отсутствии попаданий терминов
* [Коммит 6895](https://github.com/manticoresoftware/manticoresearch/commit/68953740af8dca1833026d0f448c71416ed3044a) исправлен разрыв docstore после ATTACH
* [Коммит d6f6](https://github.com/manticoresoftware/manticoresearch/commit/d6f696edea53ea2b9a68251e4054e4816082c285) исправлена проблема docstore в распределённой конфигурации
* [Коммит bce2](https://github.com/manticoresoftware/manticoresearch/commit/bce2b7ec883d208f31094ee42a5b6f01d18a998f) заменён FixedHash на OpenHash в сортировщике
* [Коммит e0ba](https://github.com/manticoresoftware/manticoresearch/commit/e0baf7392937969e640fb5d10e8ccb312fe36446) исправлены атрибуты с дублирующимися именами при определении индекса
* [Коммит ca81](https://github.com/manticoresoftware/manticoresearch/commit/ca81114bf1ac7ea7adcc8509bdc14d1f30b4f461) исправлен html_strip в HIGHLIGHT()
* [Коммит 493a](https://github.com/manticoresoftware/manticoresearch/commit/493a5e916d70fd4a3f14d1ee487234434f4a7ce3) исправлен макрос passage в HIGHLIGHT()
* [Коммит a82d](https://github.com/manticoresoftware/manticoresearch/commit/a82d41c775692f837f267d554d7d4a4b499d69f0) исправлены проблемы двойного буфера при создании RT индекса маленьких или больших дисковых чанков
* [Коммит a404](https://github.com/manticoresoftware/manticoresearch/commit/a404c85de0d41d9d1c75b7540479d8a3af9e61aa) исправлено удаление событий для kqueue

* [Коммит 8bea](https://github.com/manticoresoftware/manticoresearch/commit/8bea0f6fd1666b817995c4dd6e67f6f1c9161638) исправлено сохранение дискового чанка при большом значении rt_mem_limit для RT индекса
* [Коммит 8707](https://github.com/manticoresoftware/manticoresearch/commit/8707f0395b7ace52dc0cba9c8ed92a2bc4392932) исправлен переполнение float при индексации
* [Коммит a564](https://github.com/manticoresoftware/manticoresearch/commit/a56434ce71bb7714b4d8e75278aaa075b27ece80) теперь невозможна вставка документа с отрицательным ID в RT индекс, выдается ошибка
* [Коммит bbeb](https://github.com/manticoresoftware/manticoresearch/commit/bbebfd7551a68641154761cac962053b54806759) исправлен краш сервера при ranker fieldmask
* [Коммит 3809](https://github.com/manticoresoftware/manticoresearch/commit/3809cc1b0863e3a5a200dc423be3f7c38be56113) исправлен краш при использовании кэша запросов
* [Коммит dc2a](https://github.com/manticoresoftware/manticoresearch/commit/dc2a585bc724678679e4297362200bbd49ce32bb) исправлен краш при использовании RAM сегментов RT индексов с параллельными вставками

## Версия 3.2.2, 19 декабря 2019
### Особенности
* Автоматическое увеличение ID для RT индексов
* Поддержка подсветки для docstore через новую функцию HIGHLIGHT(), также доступную в HTTP API
* SNIPPET() может использовать специальную функцию QUERY(), возвращающую текущий MATCH запрос
* новая опция field_separator для функций подсветки
### Улучшения и изменения
* ленивое получение сохранённых полей для удалённых узлов (может значительно повысить производительность)
* строки и выражения больше не нарушают оптимизации multi-query и FACET
* сборка для RHEL/CentOS 8 теперь использует mysql libclient из mariadb-connector-c-devel

* файл данных ICU теперь включён в пакеты, опция icu_data_dir удалена
* файлы systemd сервиса включают политику 'Restart=on-failure'
* indextool теперь может проверять realtime индексы онлайн
* конфигурационный файл по умолчанию теперь /etc/manticoresearch/manticore.conf
* сервис в RHEL/CentOS переименован с 'searchd' на 'manticore'
* удалены опции query_mode и exact_phrase для сниппетов
### Исправления ошибок
* [Коммит 6ae4](https://github.com/manticoresoftware/manticoresearch/commit/6ae474c7894a6bee222d5b18e59a44fdbf57843a) исправлен краш при SELECT запросе по HTTP интерфейсу
* [Коммит 5957](https://github.com/manticoresoftware/manticoresearch/commit/59577513a49eac5a4a3c5e2cb38394d3246b5d35) исправлено сохранение дисковых чанков RT индекса, некоторые документы не помечались как удалённые
* [Коммит e861](https://github.com/manticoresoftware/manticoresearch/commit/e861f0fca0e88924450695d4e9d6acff7a36558a) исправлен краш при поиске по мультииндексу или мультизапросам с dist_threads
* [Коммит 4409](https://github.com/manticoresoftware/manticoresearch/commit/440991fc977b8479800b45cdbc862eeb1ba5d965) исправлен краш при генерации infix для длинных терминов с широкими utf8 кодпоинтами
* [Коммит 5fd5](https://github.com/manticoresoftware/manticoresearch/commit/5fd599b48bba527c023e6aa0b262dca51ffb8a1c) исправлена гонка при добавлении сокета в IOCP
* [Коммит cf10](https://github.com/manticoresoftware/manticoresearch/commit/cf10d7d3589f77adfaefbe52a51777bc9c67cf99) исправлена проблема в булевых запросах против JSON select списка
* [Коммит 996d](https://github.com/manticoresoftware/manticoresearch/commit/996de77f4c3ec103d965e28c36098fd07dba50c4) исправлена проверка indextool, чтобы сообщать о неправильном смещении skiplist, проверка doc2row lookup
* [Коммит 6e3f](https://github.com/manticoresoftware/manticoresearch/commit/6e3fc9e88941c9427410f9d0adeebb7dd09a900f) исправлено создание некорректного индекса индексатором с отрицательным смещением skiplist на больших данных
* [Коммит faed](https://github.com/manticoresoftware/manticoresearch/commit/faed3220b423a96401982bf47117edf1f62c584e) исправлено преобразование JSON: только числовые в строку и строка JSON в числовое в выражениях
* [Коммит 5331](https://github.com/manticoresoftware/manticoresearch/commit/533197200249d648ebcbdaca5d605861ee566348) исправлен выход indextool с ошибкой при нескольких командах в командной строке
* [Коммит 7955](https://github.com/manticoresoftware/manticoresearch/commit/795520ac351d22e4497ebaf6c249bf25081842b6) исправлен #275 binlog в некорректном состоянии при ошибке "нет места на диске"

* [Коммит 2284](https://github.com/manticoresoftware/manticoresearch/commit/2284da5e3ff77bf7be4038a4d950c0580e4159c8) исправлен #279 краш при фильтре IN к JSON атрибуту
* [Коммит ce2e](https://github.com/manticoresoftware/manticoresearch/commit/ce2e4b4765111a7d331ffc256911f12770a8942d) исправлен #281 неправильный вызов закрытия пайпа
* [Commit 5355](https://github.com/manticoresoftware/manticoresearch/commit/535589ba8a37baf5c6056afb5ff969fd70476feb) исправлено зависание сервера при CALL PQ с рекурсивным JSON атрибутом, закодированным как строка
* [Commit a5fc](https://github.com/manticoresoftware/manticoresearch/commit/a5fc8a36e1644ee051632408ab8b53e79aeb9ff4) исправлено продвижение за пределы конца doclist в multiand узле

* [Commit a362](https://github.com/manticoresoftware/manticoresearch/commit/a3628617ef4ee89a811d4ba2c4a04cbbc298c1b5) исправлен возврат публичной информации о потоке
* [Commit f8d2](https://github.com/manticoresoftware/manticoresearch/commit/f8d2d7bbbb90af3d2ee97424f97b05927f09d42b) исправлены блокировки кеша docstore
## Версия 3.2.0, 17 октября 2019
### Возможности
* Хранение документов

* новые директивы stored_fields, docstore_cache_size, docstore_block_size, docstore_compression, docstore_compression_level
### Улучшения и изменения
* улучшена поддержка SSL
* обновлена встроенная кодировка non_cjk
* отключено логирование UPDATE/DELETE запросов как SELECT в журнале запросов
* пакеты для RHEL/CentOS 8
### Исправления ошибок
* [Commit 301a](https://github.com/manticoresoftware/manticoresearch/commit/301a806b14725e842300ee225521407464374c41) исправлен сбой при замене документа в дисковом чанке RT индекса
* [Commit 46c1](https://github.com/manticoresoftware/manticoresearch/commit/46c1cad8fa11c1fc09cc34b20d978fdfcd4e6d27) исправлено \#269 LIMIT N OFFSET M
* [Commit 92a4](https://github.com/manticoresoftware/manticoresearch/commit/92a46edaa196ef33f3ac2225fbdba2c2b88ee45d) исправлены DELETE-запросы с явно установленным id или списком id для пропуска поиска

* [Commit 8ca7](https://github.com/manticoresoftware/manticoresearch/commit/8ca78c138d837caab30dc6e2343a3c4d1687bb87) исправлен неправильный индекс после удаления события в netloop у poller windowspoll
* [Commit 6036](https://github.com/manticoresoftware/manticoresearch/commit/603631e2bff5cb0acf25f68b85621742490eccd9) исправлено округление float в JSON через HTTP
* [Commit 62f6](https://github.com/manticoresoftware/manticoresearch/commit/62f64cb9e75ec28fc3c29f8414ce9656f5580254) исправлены удалённые сниппеты для проверки пустого пути в первую очередь; исправлены тесты для windows
* [Commit aba2](https://github.com/manticoresoftware/manticoresearch/commit/aba274c2c5abc1029894ba9f95b786bda93e8f22) исправлена перезагрузка конфигурации для работы на Windows так же как на Linux
* [Commit 6b8c](https://github.com/manticoresoftware/manticoresearch/commit/6b8c4242ef77c3fa4d0ccb7d76d81714b6728f0b) исправлено \#194 PQ для работы с морфологией и стеммерами
* [Commit 174d](https://github.com/manticoresoftware/manticoresearch/commit/174d312905285aa5746f4f404c53e6dc669ef42f) исправлено управление устаревшими сегментами RT
## Версия 3.1.2, 22 августа 2019

### Возможности и улучшения
* Экспериментальная поддержка SSL для HTTP API
* фильтр по полю для CALL KEYWORDS
* max_matches для /json/search
* автоматический размер по умолчанию для Galera gcache.size
* улучшена поддержка FreeBSD
### Исправления ошибок
* [Commit 0a1a](https://github.com/manticoresoftware/manticoresearch/commit/0a1a2c81038794983b4e30c8aefc7cc20e89b03f) исправлена репликация RT индекса на узел, где такой же RT индекс существует и имеет другой путь

* [Commit 4adc](https://github.com/manticoresoftware/manticoresearch/commit/4adc075294ac823289f745e2cc419f18c7dcf2e2) исправлена повторная постановка на выполнение flush для индексов без активности
* [Commit d6c0](https://github.com/manticoresoftware/manticoresearch/commit/d6c00a6f750c04021747e9c99e0f77707a4cbfc3) улучшена повторная постановка на выполнение очистки записей RT/PQ индексов
* [Commit d0a7](https://github.com/manticoresoftware/manticoresearch/commit/d0a7c95906b9b5ba33c98267d54501cfe27508d6) исправлено \#250 опция индексации index_field_lengths для TSV и CSV piped источников
* [Commit 1266](https://github.com/manticoresoftware/manticoresearch/commit/1266d54800242709fbe2c34e72eb69b6595b23bd) исправлен неверный отчёт indextool при проверке блочного индекса на пустом индексе
* [Commit 553c](https://github.com/manticoresoftware/manticoresearch/commit/553ca73ca111f28e0888d59cd48b0d6df181ecd6) исправлен пустой список select в журнале запросов Manticore SQL
* [Commit 56c8](https://github.com/manticoresoftware/manticoresearch/commit/56c8584456293665ddd4ce4c94307d74473a78b9) исправлен ответ indexer -h/--help
## Версия 3.1.0, 16 июля 2019
### Возможности и улучшения
* репликация для RealTime индексов
* ICU токенизатор для китайского
* новая морфологическая опция icu_chinese
* новая директива icu_data_dir
* транзакции с несколькими командами для репликации
* LAST_INSERT_ID() и @session.last_insert_id

* LIKE 'pattern' для SHOW VARIABLES
* Вставка нескольких документов для percolate индексов
* Добавлены парсеры времени для конфигурации

* внутренний менеджер задач
* mlock для компонентов doc и hit lists
* ограничение пути для сниппетов
### Удаления
* Поддержка библиотеки RLP снята в пользу ICU; все директивы rlp\* удалены
* обновление ID документа через UPDATE отключено
### Исправления ошибок
* [Commit f047](https://github.com/manticoresoftware/manticoresearch/commit/f047222339fd0e62a07e9ca8b2a829297d5db861) исправлены дефекты в concat и group_concat
* [Commit b081](https://github.com/manticoresoftware/manticoresearch/commit/b08147ee24712b5eaa51f261626b5d16c6242fdd) исправлен uid запроса в percolate индексе на тип BIGINT атрибута
* [Commit 4cd8](https://github.com/manticoresoftware/manticoresearch/commit/4cd85afaf76eef0ce9fa1cd073f3fed160279890) не происходит сбой, если не удалось заранее выделить новый дисковый чанк
* [Commit 1a55](https://github.com/manticoresoftware/manticoresearch/commit/1a551227886474054111088412f2b2396f485652) добавлен отсутствующий тип данных timestamp в ALTER

* [Commit f3a8](https://github.com/manticoresoftware/manticoresearch/commit/f3a8e096fd496a6b001d398aadbb781b2dbad929) исправлен сбой из-за неправильного mmap чтения
* [Commit 4475](https://github.com/manticoresoftware/manticoresearch/commit/447577115192db7ac7bfea70b5ba58ee2c61229f) исправлен хеш блокировки кластеров в репликации
* [Commit ff47](https://github.com/manticoresoftware/manticoresearch/commit/ff476df955f057bd38e8b6ad6056325e9d6fbb9c) исправлена утечка провайдеров при репликации
* [Commit 58dc](https://github.com/manticoresoftware/manticoresearch/commit/58dcbb779c4c30ca1560460c78e35ba114b83716) исправлено \#246 неопределённое sigmask в indexer
* [Commit 3dd8](https://github.com/manticoresoftware/manticoresearch/commit/3dd8278e747d534f69fe9c3d360eef188ab7b6d5) исправлена гонка при отчетах netloop
* [Commit a02a](https://github.com/manticoresoftware/manticoresearch/commit/a02aae05a3fa3d1caf2821f43c7e237b6de76336) нулевой разрыв для ре-балансировщика стратегий HA

## Версия 3.0.2, 31 мая 2019
### Улучшения

* добавлены mmap читатели для документов и списков попаданий
* ответ HTTP эндпоинта `/sql` теперь совпадает с ответом `/json/search`

* новые директивы `access_plain_attrs`, `access_blob_attrs`, `access_doclists`, `access_hitlists`
* новая директива `server_id` для настроек репликации
### Удаления
* удалён HTTP эндпоинт `/search`
### Устаревшие возможности
* `read_buffer`, `ondisk_attrs`, `ondisk_attrs_default`, `mlock` заменены директивами `access_*`
### Исправления ошибок
* [Commit 849c](https://github.com/manticoresoftware/manticoresearch/commit/849c16e1040f382795ba0635cb48686f9db4f2d8) разрешены имена атрибутов, начинающиеся с цифр, в списке select
* [Commit 48e6](https://github.com/manticoresoftware/manticoresearch/commit/48e6c302ca37bec0bc49c79619c60bf787ed0708) исправлены MVAs в UDF и исправлено использование псевдонимов для MVA
* [Commit 0555](https://github.com/manticoresoftware/manticoresearch/commit/055586a9a2ebfa8874f68099c5bc798a145fd4ef) исправлен \#187 краш при использовании запроса с SENTENCE


* [Commit 93bf](https://github.com/manticoresoftware/manticoresearch/commit/93bf52f23f1c53c8a93a603387ca5463e1fc2ae8) исправлен \#143 поддержка () вокруг MATCH()
* [Commit 599e](https://github.com/manticoresoftware/manticoresearch/commit/599ee79ccd38cce0023a55cbce8f50c3dca62e38) исправлено сохранение состояния кластера при выполнении ALTER cluster
* [Commit 230c](https://github.com/manticoresoftware/manticoresearch/commit/230c321e277514b191629538eb3f02d219113d95) исправлен краш сервера при ALTER индекса с блоб атрибутами
* [Commit 5802](https://github.com/manticoresoftware/manticoresearch/commit/5802b85aa93a81caea77073a3381ef912fdd677c) исправлен \#196 фильтр по id
* [Commit 25d2](https://github.com/manticoresoftware/manticoresearch/commit/25d2dabda4a299a29bd375405862126da9b4bfac) отключён поиск на шаблонных индексах
* [Commit 2a30](https://github.com/manticoresoftware/manticoresearch/commit/2a30d5b49d76faab031d195f533974938769b8e6) исправлен столбец id для передачи в SQL с обычным типом bigint
## Версия 3.0.0, 6 мая 2019
### Функции и улучшения
* Новое хранилище индекса. Невекторные атрибуты больше не ограничены размером 4 ГБ на индекс
* Директива attr_update_reserve
* Строки, JSON и MVAs можно обновлять с помощью UPDATE
* killlists применяются во время загрузки индекса
* Директива killlist_target
* Ускорение множественных AND поисков
* Улучшена средняя производительность и использование ОЗУ

* Инструмент convert для обновления индексов, созданных в версии 2.x
* Функция CONCAT()
* JOIN CLUSTER cluster AT 'nodeaddress:port'
* ALTER CLUSTER posts UPDATE nodes

* Директива node_address
* Список узлов отображается в SHOW STATUS

### Изменения в поведении
* В случае индексов с killlist сервер не переключает индексы в порядке конфигурации, а следует цепочке целей killlist
* Порядок индексов в поиске больше не определяет порядок применения killlist
* Идентификаторы документов теперь знаковые большие целые

### Удалённые директивы
* docinfo (теперь всегда extern), inplace_docinfo_gap, mva_updates_pool
## Версия 2.8.2 GA, 2 апреля 2019

### Функции и улучшения
* Galera репликация для percolate индексов
* Опция OPTION morphology
### Примечания по компиляции
Минимальная версия cmake теперь 3.13. Для компиляции требуются boost и libssl
разработческие библиотеки.
### Исправления ошибок
* [Commit 6967](https://github.com/manticoresoftware/manticoresearch/commit/6967fedb2ef818ec1c825d482563edd05e1c9245) исправлен краш при множестве звёздочек в списке select для запроса по многим распределённым индексам
* [Commit 36df](https://github.com/manticoresoftware/manticoresearch/commit/36df1a407dc08263690e3492518613ace82d69ca) исправлена [\#177](https://github.com/manticoresoftware/manticoresearch/issues/177) большая упаковка по интерфейсу Manticore SQL
* [Commit 5793](https://github.com/manticoresoftware/manticoresearch/commit/57932aec734583fa93359faaf5034b2e2c9d352b) исправлен [\#170](https://github.com/manticoresoftware/manticoresearch/issues/170) краш сервера при оптимизации RT с обновлёнными MVA

* [Commit edb2](https://github.com/manticoresoftware/manticoresearch/commit/edb24b870423add86eba471d361e0e5aff098b18) исправлен краш сервера при удалении бинарного лога из-за удаления RT индекса после перезагрузки конфигурации по SIGHUP
* [Commit bd3e](https://github.com/manticoresoftware/manticoresearch/commit/bd3e66e0085bc7e2e351b817dfc972fd8158fbce) исправлены payloads плагина аутентификации mysql handshake
* [Commit 6a21](https://github.com/manticoresoftware/manticoresearch/commit/6a217f6ed82fb10f752213b15617b9cedf1e8533) исправлена [\#172](https://github.com/manticoresoftware/manticoresearch/issues/172) настройка phrase_boundary в RT индексе
* [Commit 3562](https://github.com/manticoresoftware/manticoresearch/commit/3562f652753e4091fd7b5b0f65b58341c8cbcb31) исправлен [\#168](https://github.com/manticoresoftware/manticoresearch/issues/168) дедлок при ATTACH индекса к самому себе
* [Commit 250b](https://github.com/manticoresoftware/manticoresearch/commit/250b3f0e74ad18dda34ba080aa13ca87798a9ac9) исправлено сохранение пустых метаданных в binlog после краша сервера

* [Commit 4aa6](https://github.com/manticoresoftware/manticoresearch/commit/4aa6c69ad3a181cd7c5dafb990fb528121f68e59) исправлен краш сервера из-за строки в сортировщике с RT индексом с дисковыми чанками
## Версия 2.8.1 GA, 6 марта 2019
### Функции и улучшения
* SUBSTRING_INDEX()
* Поддержка SENTENCE и PARAGRAPH для percolate запросов
* systemd генератор для Debian/Ubuntu; добавлен LimitCORE для разрешения создания core файлов
### Исправления ошибок
* [Commit 84fe](https://github.com/manticoresoftware/manticoresearch/commit/84fe7405d17b59f4dcde15a6c8e8503923503def) исправлен краш сервера при режиме match all и пустом полном текстовом запросе
* [Commit daa8](https://github.com/manticoresoftware/manticoresearch/commit/daa88b579fa373d5d2e869e43a2e178363b0fef1) исправлен краш при удалении статической строки
* [Commit 2207](https://github.com/manticoresoftware/manticoresearch/commit/22078537dddbd9ce81a182dcc879f235b6e87004) исправлен код выхода при сбое indextool с FATAL
* [Commit 0721](https://github.com/manticoresoftware/manticoresearch/commit/0721696d6780c200c65d596624a8187983fb7fcb) исправлена [\#109](https://github.com/manticoresoftware/manticoresearch/issues/109) отсутствие совпадений для префиксов из-за неправильной проверки точной формы
* [Commit 8af8](https://github.com/manticoresoftware/manticoresearch/commit/8af810111b8e2f87bc9f378172eff1ab9725c7e7) исправлен [\#161](https://github.com/manticoresoftware/manticoresearch/issues/161) перезагрузки настроек конфигурации для RT индексов
* [Commit e2d5](https://github.com/manticoresoftware/manticoresearch/commit/e2d592773f4fd702ace90070a793a842a0fec6ab) исправлен сбой сервера при доступе к большой JSON строке
* [Commit 75cd](https://github.com/manticoresoftware/manticoresearch/commit/75cd1342d05cdc352e3dd145cdbc6f79394a165b) исправлено поле PQ в JSON документе, изменяемое индексовым stripper, вызывающее неправильное совпадение по смежному полю
* [Commit e2f7](https://github.com/manticoresoftware/manticoresearch/commit/e2f775437df922674d772ed4417780492502e65a) исправлен сбой сервера при разборе JSON в сборках RHEL7
* [Commit 3a25](https://github.com/manticoresoftware/manticoresearch/commit/3a25a5808feb3f8b80866991b436d6c1241618c2) исправлен сбой при отмене экранирования json, когда слэш находится на краю

* [Commit be9f](https://github.com/manticoresoftware/manticoresearch/commit/be9f497872bea62dbdccd64fc28294fd7776c289) исправлена опция 'skip_empty' для пропуска пустых документов без предупреждения о невалидном json
* [Commit 266e](https://github.com/manticoresoftware/manticoresearch/commit/266e0e7b088549722a805fc837bf101ff681a5e8) исправлен [\#140](https://github.com/manticoresoftware/manticoresearch/issues/161) вывод 8 знаков после запятой для float, когда 6 недостаточно для точности
* [Commit 3f6d](https://github.com/manticoresoftware/manticoresearch/commit/3f6d23891064ee8e94030c4231497cdd7da33a6a) исправлено создание пустого jsonobj
* [Commit f3c7](https://github.com/manticoresoftware/manticoresearch/commit/f3c7848a59b7eedc67c3403d6d4b37ce6fa94dc8) исправлен [\#160](https://github.com/manticoresoftware/manticoresearch/issues/161) вывод пустого mva как NULL вместо пустой строки
* [Commit 0afa](https://github.com/manticoresoftware/manticoresearch/commit/0afa2ed058e5759470b1d7354c722faab34f98bb) исправлен сбой сборки без pthread_getname_np
* [Commit 9405](https://github.com/manticoresoftware/manticoresearch/commit/9405fccdeb27f0302a8d9a848981d30080216777) исправлен сбой при остановке сервера с потоками thread_pool workers
## Версия 2.8.0 GA, 28 января 2019
### Улучшения
* Распределённые индексы для percolate индексов
* Вызов CALL PQ: новые опции и изменения:
    *   skip_bad_json
    *   mode (sparsed/sharded)
    *   JSON документы могут передаваться как JSON массив
    *   shift
    *   Имена колонок 'UID', 'Documents', 'Query', 'Tags', 'Filters' переименованы в 'id', 'documents', 'query', 'tags', 'filters'
* DESCRIBE pq TABLE
* SELECT FROM pq WHERE UID больше не поддерживается, вместо этого используйте 'id'
* SELECT по pq индексам равнозначен обычным индексам (например, вы можете фильтровать правила используя REGEX())

* ANY/ALL можно использовать для тегов PQ
* выражения автоматически конвертируются для JSON полей без необходимости явного кастинга
* встроенные charset_table 'non_cjk' и ngram_chars 'cjk'
* встроенные коллекции стоп-слов для 50 языков
* несколько файлов в декларации стоп-слов могут быть разделены запятой
* CALL PQ может принимать JSON-массив документов
### Исправления багов
* [Commit a4e1](https://github.com/manticoresoftware/manticoresearch/commit/a4e19afee54dafdc04b0da53372dafd8c0d332d6) исправлена утечка, связанная с csjon
* [Commit 28d8](https://github.com/manticoresoftware/manticoresearch/commit/28d862774874891a03e361da1b0347ebe8066ce0) исправлен сбой из-за отсутствующего значения в json
* [Commit bf4e](https://github.com/manticoresoftware/manticoresearch/commit/bf4e9ea27c349cdc9ae4e54e960a4a17b90e38fa) исправлено сохранение пустого meta для RT индекса
* [Commit 33b4](https://github.com/manticoresoftware/manticoresearch/commit/33b4573529e7c7c7bce19bf9d54054866f30d99a) исправлен потерянный флаг формы (exact) для последовательности лемматизатора
* [Commit 6b95](https://github.com/manticoresoftware/manticoresearch/commit/6b95d48240d3b5520afade19c249d79e020a5e88) исправлено поведение при использовании строковых атрибутов больше 4M: теперь используется насыщение вместо переполнения
* [Commit 6214](https://github.com/manticoresoftware/manticoresearch/commit/621418b829e70af36aaa322a2f51ece3f86bc499) исправлен сбой сервера на SIGHUP с отключённым индексом
* [Commit 3f7e](https://github.com/manticoresoftware/manticoresearch/commit/3f7e35d1482966cc45d5abbcb2de5de3508d66b0) исправлен сбой сервера при одновременных API session status командах
* [Commit cd9e](https://github.com/manticoresoftware/manticoresearch/commit/cd9e4f1709a48ddafc128c450c2d882bc11ba50e) исправлен сбой сервера при удалении запроса из RT индекса с фильтрами по полям
* [Commit 9376](https://github.com/manticoresoftware/manticoresearch/commit/9376470d455fcd256c6abff79c431a6919dfa7ac) исправлен сбой сервера при CALL PQ к распределённому индексу с пустым документом
* [Commit 8868](https://github.com/manticoresoftware/manticoresearch/commit/8868b207644f4f8cc1ab0c270adb35493fed22e1) исправлено усечение сообщения об ошибке Manticore SQL длиной более 512 символов
* [Commit de9d](https://github.com/manticoresoftware/manticoresearch/commit/de9deda9c142823d6dbf529423417b5c670fae94) исправлен сбой при сохранении percolate индекса без binlog
* [Commit 2b21](https://github.com/manticoresoftware/manticoresearch/commit/2b219e1a32791e7740bb210b7d408f96abc6374f) исправлена неработающая http интерфейс в OSX

* [Commit e92c](https://github.com/manticoresoftware/manticoresearch/commit/e92c60240f8f9e1756bfa14fba0bbb402586a389) исправлено ложное сообщение об ошибке indextool при проверке MVA
* [Commit 238b](https://github.com/manticoresoftware/manticoresearch/commit/238bdea59bad89f097403f1c978658ce45f16c70) исправлена блокировка записи при FLUSH RTINDEX, чтобы не блокировать всю запись индекса во время сохранения и при обычной очистке из rt_flush_period
* [Коммит c26a](https://github.com/manticoresoftware/manticoresearch/commit/c26a236bd42b7082db079a51a84172066d337d64) исправлен застревание ALTER percolate index при ожидании загрузки поиска
* [Коммит 9ee5](https://github.com/manticoresoftware/manticoresearch/commit/9ee5703f6c28f57cd5bff5e705966d93fe30d267) исправлено max_children для использования значения по умолчанию количества работников thread_pool при значении 0
* [Коммит 5138](https://github.com/manticoresoftware/manticoresearch/commit/5138fc048a1bd146b271ce6a72fe954344281f69) исправлена ошибка при индексации данных в индекс с плагином index_token_filter вместе со стоп-словами и stopword_step=0

* [Коммит 2add](https://github.com/manticoresoftware/manticoresearch/commit/2add3d319ac62fe450bf60e89033100853dc2ecf) исправлен сбой при отсутствии lemmatizer_base при использовании aot лемматизаторов в определениях индекса
## Версия 2.7.5 GA, 4 декабря 2018
### Улучшения
* Функция REGEX
* limit/offset для json API поиска
* точки профилировщика для qcache
### Исправления ошибок

* [Коммит eb3c](https://github.com/manticoresoftware/manticoresearch/commit/eb3c7683b9740cff4cc4cadd2ab9f990b9f7d173) исправлен сбой сервера при FACET с несколькими атрибутами широких типов
* [Коммит d915](https://github.com/manticoresoftware/manticoresearch/commit/d915cf6696514f4be365ffb5981bccaa15983e65) исправлено неявное group by в основном списке select FACET-запроса
* [Коммит 5c25](https://github.com/manticoresoftware/manticoresearch/commit/5c25dc271a8c11a1c82ca3c88e518acbc6a8c164) исправлен сбой при запросе с GROUP N BY
* [Коммит 85d3](https://github.com/manticoresoftware/manticoresearch/commit/85d30a232d4ffd89916b673a9b2db946d272a945) исправлен дедлок при обработке сбоя операций в памяти
* [Коммит 8516](https://github.com/manticoresoftware/manticoresearch/commit/85166b5ef2be38061d62f2b20f15acaa0311258a) исправлено потребление памяти indextool во время проверки

* [Коммит 58fb](https://github.com/manticoresoftware/manticoresearch/commit/58fb03166830d8b23e7ab9ce56309db7ddac22f4) исправлен лишний include gmock, теперь не нужен, так как upstream решил этот вопрос
## Версия 2.7.4 GA, 1 ноября 2018
### Улучшения
* SHOW THREADS в случае удалённых распределённых индексов выводит оригинальный запрос, вместо вызова API
* Новая опция SHOW THREADS `format=sphinxql` выводит все запросы в SQL-формате
* SHOW PROFILE выводит дополнительный этап `clone_attrs`
### Исправления ошибок
* [Коммит 4f15](https://github.com/manticoresoftware/manticoresearch/commit/4f155712a0bccc1bd01cc191647bc8cff814888e) исправлена ошибка сборки с libc без malloc_stats, malloc_trim

* [Коммит f974](https://github.com/manticoresoftware/manticoresearch/commit/f974f20bda3214a56877c393a192be1a77150958) исправлены специальные символы внутри слов в наборе результатов CALL KEYWORDS
* [Коммит 0920](https://github.com/manticoresoftware/manticoresearch/commit/092083282ea8ae0b2e72fb6989c57ccec81e74ac) исправлен сломанный CALL KEYWORDS для распределённого индекса через API или удалённого агента
* [Коммит fd68](https://github.com/manticoresoftware/manticoresearch/commit/fd686bfe88b720ffd7642e36897ba45161cbd7d2) исправлена передача distributed index agent_query_timeout агентам как max_query_time
* [Коммит 4ffa](https://github.com/manticoresoftware/manticoresearch/commit/4ffa623e9d357a3b0b441615089e211f92f8de32) исправлен общий счётчик документов на дисковом чанке, который повреждался командой OPTIMIZE и ломал вычисление веса
* [Коммит dcaf](https://github.com/manticoresoftware/manticoresearch/commit/dcaf4e0e3c8e9b0fbec47bbf3307feddec0936a6) исправлены множественные хвостовые попадания в RT индекс из blended
* [Коммит eee3](https://github.com/manticoresoftware/manticoresearch/commit/eee381754e902a43a8f499e1c950198a2a3e6ee0) исправлен дедлок при ротации

## Версия 2.7.3 GA, 26 сентября 2018
### Улучшения
* опция sort_mode для CALL KEYWORDS
* DEBUG на VIP соединении может выполнить 'crash <password>' для намеренного действия SIGEGV на сервере
* DEBUG может выполнить 'malloc_stats' для дампа статистики malloc в searchd.log, 'malloc_trim' для выполнения malloc_trim()
* улучшен backtrace, если на системе присутствует gdb
### Исправления ошибок
* [Коммит 0f3c](https://github.com/manticoresoftware/manticoresearch/commit/0f3cc333f6129451ad1ae2c97905b6c04ba5d454) исправлен сбой или ошибка переименования на Windows
* [Коммит 1455](https://github.com/manticoresoftware/manticoresearch/commit/1455ba2bf8079f03ce7f861a8d9662a360d705de) исправлены сбои сервера на 32-битных системах
* [Коммит ad37](https://github.com/manticoresoftware/manticoresearch/commit/ad3710d53b5cd6a28b8c60352d0f9e285b03d9fa) исправлен сбой или зависание сервера при пустом выражении SNIPPET
* [Коммит b36d](https://github.com/manticoresoftware/manticoresearch/commit/b36d79214364500252665860f000ae85343528b6) исправлен сломанный непрогрессивный optimize и исправлен прогрессивный optimize, чтобы не создавать kill-list для старейшего дискового чанка
* [Коммит 34b0](https://github.com/manticoresoftware/manticoresearch/commit/34b032499afd42ce47a4c7247814b4031094388a) исправлен неправильный ответ queue_max_length для SQL и API в режиме потокового пула воркеров
* [Коммит ae4b](https://github.com/manticoresoftware/manticoresearch/commit/ae4b3202cbdb8014cabe2b90e269d5cf74f49871) исправлен сбой при добавлении полного сканирования запроса в PQ индекс с установленными опциями regexp или rlp
* [Коммит f80f](https://github.com/manticoresoftware/manticoresearch/commit/f80f8d5d7560187078868aed9a9575f4549e98aa) исправлен сбой при вызове одного PQ после другого
* [Коммит 9742](https://github.com/manticoresoftware/manticoresearch/commit/9742f5f0866af73f8cd8483ecd18a507ea80dd65) рефакторинг AcquireAccum
* [Коммит 39e5](https://github.com/manticoresoftware/manticoresearch/commit/39e5bc3751b7295222eb76407c5d72ce1dad545b) исправлен утечка памяти после вызова pq
* [Коммит 21bc](https://github.com/manticoresoftware/manticoresearch/commit/21bcc6d17395f0f57dde79f6716ef303b7ea527d) косметический рефакторинг (стиль c++11 c-тр, значения по умолчанию, nullptr)
* [Коммит 2d69](https://github.com/manticoresoftware/manticoresearch/commit/2d690398f14c736956cfdd66feb4d3091d6b3a4d) исправлена утечка памяти при попытке вставить дубликат в PQ индекс
* [Коммит 5ed9](https://github.com/manticoresoftware/manticoresearch/commit/5ed92c4b66da6423fa28f11b1fd59103ed1e1fb9) исправлен сбой при использовании JSON поля IN с большими значениями
* [Commit 4a52](https://github.com/manticoresoftware/manticoresearch/commit/4a5262e2776aa261e34ba5c914ea60804f59de3f) исправлен сбой сервера при выполнении оператора CALL KEYWORDS для RT индекса с установленным ограничением расширения
* [Commit 5526](https://github.com/manticoresoftware/manticoresearch/commit/552646bb6fefde4e2525298e43e628839b421d3d) исправлен недопустимый фильтр в запросе PQ matches;
* [Commit 204f](https://github.com/manticoresoftware/manticoresearch/commit/204f521408b730198a5bab2c20407a3d7df94873) введён небольшой аллокатор объектов для ptr атрибутов
* [Commit 2545](https://github.com/manticoresoftware/manticoresearch/commit/25453e5387152c0575d1eda1b2beb89a49c7493a) переработан ISphFieldFilter на счётчик ссылок
* [Commit 1366](https://github.com/manticoresoftware/manticoresearch/commit/1366ee06e828397074b69bec9265d7147170c60e) исправлены неопределённое поведение/сегфолты при использовании strtod для неterminated строк
* [Commit 94bc](https://github.com/manticoresoftware/manticoresearch/commit/94bc6fcbdae4d08bcf6da2966e3ec2966091289f) исправлена утечка памяти при обработке json resultset
* [Commit e78e](https://github.com/manticoresoftware/manticoresearch/commit/e78e9c948963416caff9e4e46296a58080107835) исправлено чтение за пределами блока памяти при применении добавления атрибута
* [Commit fad5](https://github.com/manticoresoftware/manticoresearch/commit/fad572fb543606b33e688eb82a485b86011545fd) переработан CSphDict на счётчик ссылок
* [Commit fd84](https://github.com/manticoresoftware/manticoresearch/commit/fd841a472b472280c93f4af3506851d19f6adc45) исправлена утечка внутреннего типа AOT наружу
* [Commit 5ee7](https://github.com/manticoresoftware/manticoresearch/commit/5ee7f208c7685f2ebb9e5623a8802b721e2a7f0a) исправлена утечка памяти при управлении токенизатором
* [Commit 116c](https://github.com/manticoresoftware/manticoresearch/commit/116c5f1abebee9a0d99afe93546a1d8e4c6c6958) исправлена утечка памяти в группировщике
* [Commit 56fd](https://github.com/manticoresoftware/manticoresearch/commit/56fdbc9ab38973a9a0ba8ccee45f71cf33c16423) специальное освобождение/копирование для динамических указателей в matches (утечка памяти группировщика)
* [Commit b1fc](https://github.com/manticoresoftware/manticoresearch/commit/31db660f378541aa0d1e7cf2e7209bb0a1f47fd8) исправлена утечка динамических строк для RT
* [Commit 517b](https://github.com/manticoresoftware/manticoresearch/commit/517b9e8749f092e923cd2884b674b5bb84e20755) переработан группировщик
* [Commit b1fc](https://github.com/manticoresoftware/manticoresearch/commit/b1fc16140e5dc44290686330b476a254e0092107) мелкий рефакторинг (c++11 c-trs, некоторые изменения форматирования)
* [Commit 7034](https://github.com/manticoresoftware/manticoresearch/commit/7034e073f4cf2844762e0a464b7c3de05d3d122b) переработан ISphMatchComparator на счётчик ссылок
* [Commit b1fc](https://github.com/manticoresoftware/manticoresearch/commit/b1fc16140e5dc44290686330b476a254e0092107) приватизация клонирования
* [Commit efbc](https://github.com/manticoresoftware/manticoresearch/commit/efbc0511d6809c4a57453c7283f9bf53f3fb8d97) упрощён нативный little-endian для MVA_UPSIZE, DOCINFO2ID_T, DOCINFOSETID
* [Commit 6da0](https://github.com/manticoresoftware/manticoresearch/commit/6da0df4ac9946fc59f98d229b90e12c9221dd4c0) добавлена поддержка valgrind в ubertests
* [Commit 1d17](https://github.com/manticoresoftware/manticoresearch/commit/1d17669cb7ec54ac80b392e101f37a688eb98080) исправлен сбой из-за гонки флага 'success' в соединении
* [Commit 5a09](https://github.com/manticoresoftware/manticoresearch/commit/5a09c32d7ded56b2c0bd3e2ad7968cb8d6362919) переключение epoll на режим с edge-triggered
* [Commit 5d52](https://github.com/manticoresoftware/manticoresearch/commit/5d528682737fca03a4352a3093b1319ec27d598c) исправлен оператор IN в выражении с форматированием как у фильтра
* [Commit bd8b](https://github.com/manticoresoftware/manticoresearch/commit/bd8b3c976ff8b4667f55dfdd101b20a920137ac5) исправлен сбой в RT индексе при коммите документа с большим docid
* [Commit ce65](https://github.com/manticoresoftware/manticoresearch/commit/ce656b83b747ce7c55795b53915770c13fb99cfe) исправлены опции без аргументов в indextool
* [Commit 08c9](https://github.com/manticoresoftware/manticoresearch/commit/08c9507177820f441f534bf06a11dac5e54bebb4) исправлена утечка памяти расширенного ключевого слова

* [Commit 30c7](https://github.com/manticoresoftware/manticoresearch/commit/30c75a2f525ec9bda625d903acdc9d4d2e2413dc) исправлена утечка памяти json группировщика
* [Commit 6023](https://github.com/manticoresoftware/manticoresearch/commit/6023f269b6f2080e4d380b0957605ef8107c8c9f) исправлена утечка глобальных пользовательских переменных
* [Commit 7c13](https://github.com/manticoresoftware/manticoresearch/commit/7c138f15ca23c0c8717fa12041e3db7f988fcf15) исправлена утечка динамических строк на ранне отклонённых совпадениях
* [Commit 9154](https://github.com/manticoresoftware/manticoresearch/commit/9154b18eaed5302e8330cb3a95bd968959ccb312) исправлена утечка на length(<expression>)
* [Commit 43fc](https://github.com/manticoresoftware/manticoresearch/commit/43fca3a4e26139765d0fac8de054200dc4875fc6) исправлена утечка памяти из-за strdup() в парсере
* [Commit 71ff](https://github.com/manticoresoftware/manticoresearch/commit/71ff77737e63d1019b7c9804dca7fa2351025dba) переработан парсер выражений для точного отслеживания счётчиков ссылок
## Версия 2.7.2 GA, 27 августа 2018
### Улучшения
* совместимость с клиентами MySQL 8
* [TRUNCATE](Emptying_a_table.md) С ПЕРЕГРУЗКОЙ
* удалён счётчик памяти в SHOW STATUS для RT индексов
* глобальный кэш мультиагентов

* улучшен IOCP на Windows
* VIP соединения для HTTP протокола
* команда Manticore SQL [DEBUG](Reporting_bugs.md#DEBUG), которая может выполнять различные подкоманды
* [shutdown_token](Server_settings/Searchd.md#shutdown_token) - SHA1 хеш пароля, необходимый для вызова `shutdown` командой DEBUG
* новые статистики для SHOW AGENT STATUS (_ping, _has_perspool, _need_resolve)
* опция --verbose индексатора теперь принимает \[debugvv\] для вывода отладочных сообщений
### Исправления ошибок
* [Commit 3900](https://github.com/manticoresoftware/manticoresearch/commit/390082a7be0a1f9539b30361d11d54de35c62a44) удалён wlock при оптимизации
* [Commit 4c33](https://github.com/manticoresoftware/manticoresearch/commit/4c337648329be22373be33333cbc9c5664d18b80) исправлен wlock при перезагрузке настроек индекса
* [Коммит b5ea](https://github.com/manticoresoftware/manticoresearch/commit/b5ea8dc0b18a23dbd739e92c66dd93b0094df14b) исправлена утечка памяти при запросе с JSON фильтром
* [Коммит 930e](https://github.com/manticoresoftware/manticoresearch/commit/930e831a43fd3d1ce996185be968d1df49a088d0) исправлены пустые документы в результате запроса PQ
* [Коммит 53de](https://github.com/manticoresoftware/manticoresearch/commit/53deecfecb374431f84f8592a1095a77407b8aea) исправлена путаница задач из-за удаления одной из них
* [Коммит cad9](https://github.com/manticoresoftware/manticoresearch/commit/cad9b9c617719aad97e9f5a2ae93491153d8e504) исправлен неправильный подсчет удаленных хостов
* [Коммит 9000](https://github.com/manticoresoftware/manticoresearch/commit/90008cc15c4684d445d37a391bb4a667bbc2e365) исправлена утечка памяти парсенных дескрипторов агентов
* [Коммит 978d](https://github.com/manticoresoftware/manticoresearch/commit/978d839641ad6040c22c9fc3b703b1a02685f266) исправлена утечка в поиске
* [Коммит 0193](https://github.com/manticoresoftware/manticoresearch/commit/0193946105ac08913362b7d7acf0c90f9b38dc85) косметические изменения в explicit/inline c-trs, использовании override/final
* [Коммит 943e](https://github.com/manticoresoftware/manticoresearch/commit/943e2997b33f9b2eda23d94a66068910f9476ebd) исправлена утечка json в локальной/удаленной схеме
* [Коммит 02db](https://github.com/manticoresoftware/manticoresearch/commit/02dbdd6f1d0b72e7e458ebebdd2c67f989577e6b) исправлена утечка json выражения сортировки колонок в локальной/удаленной схеме
* [Коммит c74d](https://github.com/manticoresoftware/manticoresearch/commit/c74d0b4035e4214510376845b22cb676a8da24a3) исправлена утечка const alias
* [Коммит 6e5b](https://github.com/manticoresoftware/manticoresearch/commit/6e5b57e1d3952c9695376728a143bf5434208f53) исправлена утечка потока предварительного чтения
* [Коммит 39c7](https://github.com/manticoresoftware/manticoresearch/commit/39c740fef5bf71da0bee60a35f5dd9f471af850f) исправлена зависание при выходе из-за зависшего ожидания в netloop
* [Коммит adaf](https://github.com/manticoresoftware/manticoresearch/commit/adaf9772f95772ad6d5297acace6c5cb92b56ad5) исправлено зависание поведения 'ping' при смене агента HA на обычный хост
* [Коммит 32c4](https://github.com/manticoresoftware/manticoresearch/commit/32c40eac9cdd40b15d79e58b2f15416f164f9701) отдельный GC для dashboard storage
* [Коммит 511a](https://github.com/manticoresoftware/manticoresearch/commit/511a3cf2c1f1858f6e94fe9f8175b7216db3cbd6) исправлена ошибка ref-counted ptr
* [Коммит 32c4](https://github.com/manticoresoftware/manticoresearch/commit/af2326cd2927effbad7bec88e8dc238a566cf27e) исправлен сбой indextool при отсутствии индекса
* [Коммит 156e](https://github.com/manticoresoftware/manticoresearch/commit/156edc717dc63d3ea120f42466eba6e022da88be) исправлено имя выходного атрибута/поля превышающего лимит при индексировании xmlpipe
* [Коммит cdac](https://github.com/manticoresoftware/manticoresearch/commit/cdac6d1f17d384bfc8bae49e91241b2f346c3830) исправлено значение индексера по умолчанию при отсутствии раздела indexer в конфиге
* [Коммит e61e](https://github.com/manticoresoftware/manticoresearch/commit/e61ec00b6b27d1d5878247e2ee817f3b1e7fde16) исправлены неправильные встроенные стоп-слова в disk chunk для RT индекса после перезапуска сервера

* [Коммит 5fba](https://github.com/manticoresoftware/manticoresearch/commit/5fba49d5e28c17de4c0acbd984466127f42de6e8) исправлено пропускание фантомных (уже закрытых, но не окончательно удаленных из poller) соединений
* [Коммит f22a](https://github.com/manticoresoftware/manticoresearch/commit/f22ae34b623906b3c5a05a06198dad2e548dc541) исправлены смешанные (оставшиеся без хозяина) сетевые задачи
* [Коммит 4689](https://github.com/manticoresoftware/manticoresearch/commit/46890e70eb8dcd00c1bf1b030538b8f131c601c2) исправлен сбой при операции чтения после записи
* [Коммит 03f9](https://github.com/manticoresoftware/manticoresearch/commit/03f9df904f611c438d5ebcaaeef911b0dc8d6c39) исправлены сбои searchd при запуске тестов на Windows
* [Коммит e925](https://github.com/manticoresoftware/manticoresearch/commit/e9255ed2704790a3d7f5273d38b85433463cb3ff) исправлена обработка кода EINPROGRESS для обычного connect()
* [Коммит 248b](https://github.com/manticoresoftware/manticoresearch/commit/248b72f1544b27e9e82773284050d44eeb1714e1) исправлены таймауты соединений при работе с TFO

## Версия 2.7.1 GA, 4 июля 2018
### Улучшения
* улучшена производительность подстановочных знаков при совпадении нескольких документов в PQ
* поддержка fullscan запросов в PQ
* поддержка MVA атрибутов в PQ


* поддержка regexp и RLP для перколяторных индексов
### Исправления ошибок
* [Коммит 6885](https://github.com/manticoresoftware/manticoresearch/commit/68856261b41f6950666f9e5122726839fb3c71d0) исправлена потеря строки запроса
* [Коммит 0f17](https://github.com/manticoresoftware/manticoresearch/commit/0f1770943cba53bfd2e0edfdf0b1495ff0dd0e89) исправлена пустая информация при SHOW THREADS
* [Коммит 53fa](https://github.com/manticoresoftware/manticoresearch/commit/53faa36a916bb87868e83aa79c1a1f972fb20ca0) исправлен сбой при матчинге с оператором NOTNEAR
* [Коммит 2602](https://github.com/manticoresoftware/manticoresearch/commit/26029a7cd77b518fb3a27588d7db6c8bffc73b0f) исправлено сообщение об ошибке при неверном фильтре в PQ delete
## Версия 2.7.0 GA, 11 июня 2018
### Улучшения
* уменьшено количество системных вызовов для избежания влияния патчей Meltdown и Spectre
* внутренний переписанный менеджмент локальных индексов
* рефакторинг удалённых сниппетов
* полная перезагрузка конфигурации
* все подключения узлов теперь независимы
* улучшения в протоколе
* для Windows переключение с wsapoll на IO completion ports

* для связи между мастером и узлами можно использовать TFO
* [SHOW STATUS](Node_info_and_management/Node_status.md#SHOW-STATUS) теперь выводит версию сервера и mysql_version_string
* добавлена опция `docs_id` для документов вызыванных в CALL PQ.
* фильтр перколяторных запросов теперь может содержать выражения
* распределённые индексы могут работать с FEDERATED
* фиктивные SHOW NAMES COLLATE и `SET wait_timeout` (для лучшей совместимости с ProxySQL)
### Исправления ошибок
* [Коммит 5bcf](https://github.com/manticoresoftware/manticoresearch/commit/5bcff05d19e495f1a44f190885b25cbb6f217c4a) исправлены добавленные теги not equal в PQ
* [Commit 9ebc](https://github.com/manticoresoftware/manticoresearch/commit/9ebc58916cd515eaa88da66d0895aebf0d1f2b5f) исправлено добавление поля идентификатора документа в JSON документ при вызове CALL PQ
* [Commit 8ae0](https://github.com/manticoresoftware/manticoresearch/commit/8ae0e593a286110d8505d88baa3ac9e433cb38c9) исправлена обработка flush statement для PQ индекса
* [Commit c24b](https://github.com/manticoresoftware/manticoresearch/commit/c24b152344ecd77661566cddb803487f51d3c1aa) исправлена фильтрация PQ для JSON и строковых атрибутов
* [Commit 1b8b](https://github.com/manticoresoftware/manticoresearch/commit/1b8bdde19aaa362785ea4c33bdc019154f8fbac4) исправлен парсинг пустой JSON строки


* [Commit 1ad8](https://github.com/manticoresoftware/manticoresearch/commit/1ad8a07dbfc1131913a8d92c261fbb48f934e8b7) исправлен сбой при мультизапросе с OR фильтрами
* [Commit 69b8](https://github.com/manticoresoftware/manticoresearch/commit/69b89806380a229e36287ad9daf6f0ea2b5453eb) исправлен indextool для использования секции config common (опция lemmatizer_base) для команд (dumpheader)
* [Commit 6dbe](https://github.com/manticoresoftware/manticoresearch/commit/6dbeaf2389d2af6a9dfccb56bfc986685b9f203e) исправлена обработка пустой строки в наборе результатов и фильтре
* [Commit 39c4](https://github.com/manticoresoftware/manticoresearch/commit/39c4eb55ecc85f6dd54f6c8c6d6dfcf87fd8748e) исправлены отрицательные значения идентификатора документа
* [Commit 266b](https://github.com/manticoresoftware/manticoresearch/commit/266b7054f98e7d30fc2829b5c2467dc9f90def92) исправлена длина обрезки слова для очень длинных индексированных слов
* [Commit 4782](https://github.com/manticoresoftware/manticoresearch/commit/47823b0bd06cbd016a24345c99bb8693790b43dd) исправлено сопоставление нескольких документов с wildcard-запросами в PQ
## Version 2.6.4 GA, 3 May 2018

### Features and improvements
* Поддержка движка MySQL FEDERATED [support](Extensions/FEDERATED.md)
* Пакеты MySQL теперь возвращают флаг SERVER_STATUS_AUTOCOMMIT, добавлена совместимость с ProxySQL
* [listen_tfo](Server_settings/Searchd.md#listen_tfo) - включение TCP Fast Open для всех слушателей
* indexer --dumpheader теперь может также выгружать заголовок RT из файла .meta
* скрипт сборки cmake для Ubuntu Bionic
### Bugfixes
* [Commit 355b](https://github.com/manticoresoftware/manticoresearch/commit/355b11629174813abd9cd5bf2233be0783f77745) исправлены некорректные записи кэша запросов для RT индекса;

* [Commit 546e](https://github.com/manticoresoftware/manticoresearch/commit/546e2297a01cc2913bc0d33052d5bcefae8737eb) исправлена потеря настроек индекса после бесшовного ротационного обновления
* [Commit 0c45](https://github.com/manticoresoftware/manticoresearch/commit/0c4509898393993bc87194d2aca2070395ff7f83) исправлены установки длины infix и prefix; добавлено предупреждение о неподдерживаемой длине infix
* [Commit 8054](https://github.com/manticoresoftware/manticoresearch/commit/80542fa54d727c781635918e8d4d65a20c2fbd1e) исправлен порядок авто-флаш RT индексов

* [Commit 705d](https://github.com/manticoresoftware/manticoresearch/commit/705d8c5f407726ed1c6f9745ecffa375682c8969) исправлены проблемы структуры набора результатов для индекса с несколькими атрибутами и запросов к нескольким индексам
* [Commit b0ba](https://github.com/manticoresoftware/manticoresearch/commit/b0ba932fff4a082f642cb737381c8a70215d4cc9) исправлена потеря некоторых совпадений при пакетной вставке с дублирующимися документами
* [Commit 4510](https://github.com/manticoresoftware/manticoresearch/commit/4510fa44ad54b5bbac53c591144501565130fcae) исправлена ошибка optimize при слиянии дисковых сегментов RT индекса с большим количеством документов
## Version 2.6.3 GA, 28 March 2018
### Improvements
* jemalloc при компиляции. Если jemalloc присутствует в системе, его можно включить с помощью флага cmake `-DUSE_JEMALLOC=1`


### Bugfixes
* [Commit 85a6](https://github.com/manticoresoftware/manticoresearch/commit/85a6d7e3db75f28c491042409bb677ed39eba07b) исправлена опция expand_keywords в логе запросов Manticore SQL
* [Commit caaa](https://github.com/manticoresoftware/manticoresearch/commit/caaa3842f4dbd8507fd9e20bf3371a0c8ec0c087) исправлен HTTP интерфейс для корректной обработки больших запросов
* [Commit e386](https://github.com/manticoresoftware/manticoresearch/commit/e386d842b36195719e459b2db1fb20e402f9a51f) исправлен сбой сервера при DELETE в RT индекс с включённой опцией index_field_lengths
* [Commit cd53](https://github.com/manticoresoftware/manticoresearch/commit/ce528f37eb5d8284661928f6f3b340a4a1985253) исправлена опция cpustats в searchd cli для работы на неподдерживаемых системах
* [Commit 8740](https://github.com/manticoresoftware/manticoresearch/commit/8740fd63ae842c2ab0e51a8fcd6180d787ed57ec) исправлено сопоставление подстрок UTF8 с заданными минимальными длинами
## Version 2.6.2 GA, 23 February 2018
### Improvements

* улучшена производительность [Percolate Queries](Searching/Percolate_query.md) при использовании оператора NOT и для батчевых документов.
* [percolate_query_call](Searching/Percolate_query.md) может использовать несколько потоков в зависимости от [dist_threads](Server_settings/Searchd.md#threads)
* новый оператор полнотекстового сопоставления NOTNEAR/N
* ограничение LIMIT для SELECT на перколатных индексах
* [expand_keywords](Searching/Options.md#expand_keywords) теперь может принимать значения 'start','exact' (где 'start,exact' эквивалентно '1')
* ranged-main-query для [joined fields](Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Processing_fetched_data.md#sql_joined_field), который использует ranged-запрос, определённый sql_query_range
### Bugfixes
* [Commit 72dc](https://github.com/manticoresoftware/manticoresearch/commit/72dcf669744e9b7d636dfc213d24df85ab301f6b) исправлен сбой при поиске в RAM сегментах; исправлены взаимоблокировки при сохранении дисковых сегментов с двойным буфером; исправлены взаимоблокировки при сохранении дисковых сегментов во время optimize
* [Commit 3613](https://github.com/manticoresoftware/manticoresearch/commit/36137149a1c3c0893bdda5a28fc7e8244bf2d4ae) исправлен сбой индексатора при xml встроенной схеме с пустым именем атрибута


* [Commit 48d7](https://github.com/manticoresoftware/manticoresearch/commit/48d7e8001d2a66466ca64577f27ddc5421a67251) исправлено ошибочное удаление pid-файла, не принадлежащего процессу
* [Commit a556](https://github.com/manticoresoftware/manticoresearch/commit/a5563a465ddc59ef71e65f17b68bc33f9700e838) исправлено, что иногда в temp папке оставались сиротские fifo-файлы
* [Коммит 2376](https://github.com/manticoresoftware/manticoresearch/commit/2376e8fc4508944b96959bd10686c6d51f5145e8) исправлен пустой результат FACET с неправильной строкой NULL
* [Коммит 4842](https://github.com/manticoresoftware/manticoresearch/commit/4842b67acdd5da75672db6c08ac563b48577ddd7) исправлена сломанная блокировка индекса при запуске сервера как службы Windows
* [Коммит be35](https://github.com/manticoresoftware/manticoresearch/commit/be35feef54ded59125750916cd211e56108eddae) исправлены неправильные библиотеки iconv на Mac OS
* [Коммит 8374](https://github.com/manticoresoftware/manticoresearch/commit/83744a977d7837d1d87cd506b88b0e7eb199efe6) исправлен неправильный count(\*)
## Версия 2.6.1 GA, 26 января 2018
### Улучшения

* [agent_retry_count](Server_settings/Searchd.md#agent_retry_count) в случае агентов с зеркалами возвращает значение повторных попыток на зеркало вместо на агента, общее число повторов на агента равно agent_retry_count\*mirrors.
* [agent_retry_count](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_retry_count) теперь может указываться для каждого индекса отдельно, переопределяя глобальное значение. Добавлен псевдоним [mirror_retry_count](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#mirror_retry_count).
* retry_count может быть указан в определении агента, значение означает количество попыток на агента
* Перколятор-запросы теперь доступны в HTTP JSON API по адресу [/json/pq](Searching/Percolate_query.md#Performing-a-percolate-query-with-CALL-PQ).
* Добавлены опции -h и -v (помощь и версия) к исполняемым файлам
* [morphology_skip_fields](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) поддерживается для индексов Real-Time
### Исправления ошибок


* [Коммит a40b](https://github.com/manticoresoftware/manticore/commit/a40b0793feff65e40d10062568d9847c08d10f57) исправлена работа ranged-main-query с sql_range_step при использовании с полем MVA
* [Коммит f2f5](https://github.com/manticoresoftware/manticore/commit/f2f53757db45bcfb1544263ce0817e856656a621) исправлена проблема с зависанием цикла системы blackhole и отключением агентов blackhole
* [Коммит 84e1](https://github.com/manticoresoftware/manticore/commit/84e1f54aef25e0fce98870ad2dd784db5116f1d6) исправлен идентификатор запроса для согласованности, исправлен дублирующийся id для сохранённых запросов
* [Коммит 1948](https://github.com/manticoresoftware/manticore/commit/19484231814fcb82b21763a3a4a9f45adc6b2d40) исправлен сбой сервера при завершении из различных состояний
* [Коммит 9a70](https://github.com/manticoresoftware/manticore/commit/9a706b499a1d61a90076065a1a703029d49db958)  [Коммит 3495](https://github.com/manticoresoftware/manticore/commit/3495fd70cba8846b1a50d55d6679b039414c3d2a) исправлены тайм-ауты при длительных запросах
* [Коммит 3359](https://github.com/manticoresoftware/manticore/commit/3359bcd89b4f79a645fe84b8cf8616ce0addff02) переработан опрос сети master-agent на системах с kqueue (Mac OS X, BSD).
## Версия 2.6.0, 29 декабря 2017

### Функции и улучшения
* [HTTP JSON](Connecting_to_the_server/HTTP.md): JSON-запросы теперь могут выполнять сравнение на равенство по атрибутам, MVA и JSON-атрибуты могут использоваться при вставке и обновлении, а также обновление и удаление через JSON API возможно на распределённых индексах
* [Перколятор-запросы](Searching/Percolate_query.md)
* Удалена поддержка 32-битных docid из кода. Также удалён весь код для конвертации/загрузки устаревших индексов с 32-битными docid.
* [Морфология только для определённых полей](https://github.com/manticoresoftware/manticore/issues/7). Новая директива индекса morphology_skip_fields позволяет указать список полей, для которых морфология не применяется.
* [expand_keywords теперь может быть директивой времени выполнения запроса, задаваемой через оператор OPTION](https://github.com/manticoresoftware/manticore/issues/8)

### Исправления ошибок
* [Коммит 0cfa](https://github.com/manticoresoftware/manticore/commit/0cfae4c) исправлен сбой при сборке сервера с отладкой (и, возможно, неопределённое поведение в релизе) при сборке с rlp

* [Коммит 3242](https://github.com/manticoresoftware/manticore/commit/324291e) исправлена оптимизация RT индекса с включённой опцией progressive, которая объединяла kill-листы в неправильном порядке
* [Коммит ac0e](https://github.com/manticoresoftware/manticore/commit/ac0efee)  незначительный сбой на Mac
* множество мелких исправлений после тщательного статического анализа кода
* другие незначительные исправления ошибок
### Обновление
В этом релизе изменён внутренний протокол, используемый между мастерами и агентами для общения. Если вы запускаете Manticoresearch в распределённой среде с несколькими инстансами, убедитесь, что обновляете сначала агентов, затем мастеров.
## Версия 2.5.1, 23 ноября 2017
### Функции и улучшения
* JSON-запросы через [HTTP API протокол](Connecting_to_the_server/HTTP.md). Поддерживается поиск, вставка, обновление, удаление, замена. Команды манипуляции данными могут выполняться пакетно, хотя есть некоторые ограничения — атрибуты MVA и JSON не могут использоваться для вставок, замен или обновлений.
* Команда [RELOAD INDEXES](Data_creation_and_modification/Adding_data_from_external_storages/Rotating_a_table.md#RELOAD-TABLES)
* Команда [FLUSH LOGS](Logging/Rotating_query_and_server_logs.md)

* Команда [SHOW THREADS](Node_info_and_management/SHOW_THREADS.md) может показывать прогресс оптимизации, ротации или выгрузки.
* GROUP N BY корректно работает с MVA-атрибутами
* Агенты blackhole теперь запускаются в отдельном потоке и не влияют на запросы мастера
* Реализован подсчёт ссылок на индексы, чтобы избегать зависаний из-за ротаций при высокой нагрузке
* Реализован SHA1 хеш, пока не доступен внешне
* исправлены проблемы компиляции на FreeBSD, macOS и Alpine
### Исправления ошибок
* [Коммит 9897](https://github.com/manticoresoftware/manticore/commit/989752b) регрессия фильтра с блочным индексом

* [Коммит b1c3](https://github.com/manticoresoftware/manticore/commit/b1c3864) переименование PAGE_SIZE -> ARENA_PAGE_SIZE для совместимости с musl
* [Коммит f213](https://github.com/manticoresoftware/manticore/commit/f2133cc) отключение googletests для cmake < 3.1.0
* [Коммит f30e](https://github.com/manticoresoftware/manticore/commit/0839de7) ошибка привязки сокета при перезапуске сервера
* [Коммит 0807](https://github.com/manticoresoftware/manticore/commit/0807240) исправлен сбой сервера при завершении работы
* [Commit 3e3a](https://github.com/manticoresoftware/manticore/commit/3e3acc3) исправлен показ потоков для системного потока blackhole
* [Commit 262c](https://github.com/manticoresoftware/manticore/commit/262c3fe) переработана проверка конфигурации iconv, исправлена сборка на FreeBSD и Darwin
## Версия 2.4.1 GA, 16 октября 2017
### Функции и улучшения
* Оператор OR в WHERE между фильтрами атрибутов
* Режим обслуживания ( SET MAINTENANCE=1)
* [CALL KEYWORDS](Searching/Autocomplete.md#CALL-KEYWORDS) доступны для распределённых индексов
* [Группировка в UTC](Server_settings/Searchd.md#grouping_in_utc)
* [query_log_mode](Server_settings/Searchd.md#query_log_mode) для пользовательских разрешений логов
* Веса полей могут быть нулевыми или отрицательными

* [max_query_time](Searching/Options.md#max_query_time) теперь может влиять на полные сканирования
* добавлены [net_wait_tm](Server_settings/Searchd.md#net_wait_tm), [net_throttle_accept](Server_settings/Searchd.md#net_throttle_accept) и [net_throttle_action](Server_settings/Searchd.md#net_throttle_action) для тонкой настройки сетевых потоков (при workers=thread_pool)

* COUNT DISTINCT работает с поиском по фасетам
* IN можно использовать с JSON-массивами float
* оптимизация мультизапросов больше не ломается из-за целочисленных/вещественных выражений
* [SHOW META](Node_info_and_management/SHOW_META.md) показывает строку `multiplier` при использовании оптимизации мультизапросов
### Компиляция

Manticore Search собирается с помощью cmake, минимальная версия gcc для сборки — 4.7.2.
### Папки и сервис
* Manticore Search запускается под пользователем `manticore`.
* Папка данных по умолчанию теперь `/var/lib/manticore/`.
* Папка логов по умолчанию теперь `/var/log/manticore/`.
* Папка pid по умолчанию теперь `/var/run/manticore/`.
### Исправления ошибок
* [Commit a58c](https://github.com/manticoresoftware/manticore/commit/a58c619) исправлено выражение SHOW COLLATION, ломавшее java-коннектор
* [Commit 631c](https://github.com/manticoresoftware/manticore/commit/631cf4e) исправлены сбои при обработке распределённых индексов; добавлены блокировки хэша распределённого индекса; удалены операторы перемещения и копирования из агента
* [Commit 942b](https://github.com/manticoresoftware/manticore/commit/942bec0) исправлены сбои при обработке распределённых индексов из-за параллельных переподключений
* [Commit e5c1](https://github.com/manticoresoftware/manticore/commit/e5c1ed2) исправлен сбой в обработчике сбоев при сохранении запроса в лог сервера
* [Commit 4a4b](https://github.com/manticoresoftware/manticore/commit/4a4bda5) исправлен сбой с пулом атрибутов в мультизапросах
* [Commit 3873](https://github.com/manticoresoftware/manticore/commit/3873bfb) уменьшен размер core файла за счёт предотвращения включения страниц индекса
* [Commit 11e6](https://github.com/manticoresoftware/manticore/commit/11e6254) исправлены сбои searchd при стартe с неправильными агентами
* [Commit 4ca6](https://github.com/manticoresoftware/manticore/commit/4ca6350) исправлена ошибка indexer в sql_query_killlist
* [Commit 123a](https://github.com/manticoresoftware/manticore/commit/123a9f0) исправлена проблема fold_lemmas=1 и подсчёта попаданий
* [Commit cb99](https://github.com/manticoresoftware/manticore/commit/cb99164) исправлено непоследовательное поведение html_strip
* [Commit e406](https://github.com/manticoresoftware/manticore/commit/e406761) исправлена потеря настроек optimize rt index; исправлены утечки блокировок при optimize с опцией sync;

* [Commit 86ae](https://github.com/manticoresoftware/manticore/commit/86aeb82) исправлена обработка ошибочных мультизапросов
* [Commit 2645](https://github.com/manticoresoftware/manticore/commit/2645230) исправлена зависимость результата от порядка мультизапросов

