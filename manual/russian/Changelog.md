# Журнал изменений

## Версия 13.6.7
**Выпущено**: 8 Августа 2025

Рекомендуемая версия [MCL](https://github.com/manticoresoftware/columnar): 8.0.1  
Рекомендуемая версия [Buddy](Installation/Manticore_Buddy.md#Manticore-Buddy): 3.34.2  

Если вы следуете [официальному руководству по установке](https://manticoresearch.com/install/), вам не о чем беспокоиться.

### Новые возможности и улучшения
* 🆕 [v13.6.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.0) [ Issue #2226](https://github.com/manticoresoftware/manticoresearch/issues/2226) Поддержка явного '|' (ИЛИ) в операторах PHRASE, PROXIMITY и QUORUM.
* 🆕 [v13.5.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.5.0) [ PR #3591](https://github.com/manticoresoftware/manticoresearch/pull/3591) Автоматическая генерация эмбеддингов в запросах (В процессе разработки, пока не готово для продакшена).
* 🆕 [v13.4.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.4.0) [ PR #3585](https://github.com/manticoresoftware/manticoresearch/pull/3585) Исправлена логика предпочтения количества потоков Buddy из конфигурации buddy_path, если установлено, вместо использования значения демона.
* 🆕 [v13.3.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.3.0) [ PR #3577](https://github.com/manticoresoftware/manticoresearch/pull/3577) Поддержка джойнов с локальными распределёнными таблицами.
* 🆕 [#3647](https://github.com/manticoresoftware/manticoresearch/issues/3647) Добавлена поддержка Debian 13 "Trixie"

### Исправления ошибок
* 🪲 [v13.6.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.7) [ Issue #3524](https://github.com/manticoresoftware/manticoresearch/issues/3524) Исправлена ошибка сохранения сгенерированных эмбеддингов в построчном хранении.
* 🪲 [v13.6.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.6) [ Issue #3563](https://github.com/manticoresoftware/manticoresearch/issues/3563) Исправлены проблемы с Sequel Ace и другими интеграциями, вызванные ошибками "unknown sysvar".
* 🪲 [v13.6.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.5) [ Issue #3467](https://github.com/manticoresoftware/manticoresearch/issues/3467) Исправлены проблемы с DBeaver и другими интеграциями, вызванные ошибками "unknown sysvar".
* 🪲 [v13.6.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.4) [ Issue #3524](https://github.com/manticoresoftware/manticoresearch/issues/3524) Исправлена ошибка объединения эмбеддингов из нескольких полей; также улучшена генерация эмбеддингов из запросов.
* 🪲 [v13.6.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.3) [ Issue #3641](https://github.com/manticoresoftware/manticoresearch/issues/3641) Исправлена ошибка в версии 13.6.0, из-за которой фраза теряла все ключевые слова в скобках, кроме первого.
* 🪲 [v13.6.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.2) Исправлена утечка памяти в transform_phrase.
* 🪲 [v13.6.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.1) Исправлена утечка памяти в версии 13.6.0.
* 🪲 [v13.5.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.5.2) [ Issue #3651](https://github.com/manticoresoftware/manticoresearch/issues/3651) Исправлены дополнительные проблемы, связанные с fuzz-тестированием полнотекстового поиска.
* 🪲 [v13.5.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.5.1) [ Issue #3560](https://github.com/manticoresoftware/manticoresearch/issues/3560) Исправлен случай, когда OPTIMIZE TABLE мог зациклиться при работе с данными KNN.
* 🪲 [v13.4.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.4.2) [ Issue #2544](https://github.com/manticoresoftware/manticoresearch/issues/2544) Исправлена ошибка, из-за которой добавление столбца float_vector могло повредить индексы.
* 🪲 [v13.4.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.4.1) [ Issue #3651](https://github.com/manticoresoftware/manticoresearch/issues/3651) Добавлено fuzz-тестирование синтаксического разбора полнотекста и исправлены несколько выявленных ошибок.
* 🪲 [v13.3.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.3.1) [ Issue #3583](https://github.com/manticoresoftware/manticoresearch/issues/3583) Исправлен сбой при использовании сложных булевых фильтров с подсветкой.
* 🪲 [v13.2.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.7) [ Issue #3481](https://github.com/manticoresoftware/manticoresearch/issues/3481) Исправлен сбой при одновременном использовании HTTP update, распределённой таблицы и неправильного кластера репликации.
* 🪲 [v13.2.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.6) [ PR #3567](https://github.com/manticoresoftware/manticoresearch/pull/3567) Обновлена зависимость manticore-backup до версии 1.9.6.
* 🪲 [v13.2.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.5) [ PR #3565](https://github.com/manticoresoftware/manticoresearch/pull/3565) Исправлена настройка CI для улучшения совместимости Docker образов.
* 🪲 [v13.2.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.4) Исправлена обработка длинных токенов. Некоторые специальные токены (например, паттерны регулярных выражений) могли создавать слишком длинные слова, теперь они сокращаются перед использованием.

## Версия 13.2.3
**Выпущено**: 8 Июля 2025

### Ломающие изменения
* ⚠️ [v13.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.0.0)  Обновлено API библиотеки KNN для поддержки пустых значений [float_vector](https://manual.manticoresearch.com/Creating_a_table/Data_types#Float-vector). Это обновление не меняет формат данных, но повышает версию API Manticore Search / MCL.
* ⚠️ [v12.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/12.0.0) [ PR #3516](https://github.com/manticoresoftware/manticoresearch/pull/3516) Исправлена ошибка с некорректным source и destination row ID во время обучения и построения индекса KNN. Это обновление не меняет формат данных, но повышает версию API Manticore Search / MCL.
* ⚠️ [v11.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.0.0)  Добавлена поддержка новых функций KNN векторного поиска, таких как квантизация, пересчет результатов и сверхвыборка. В этой версии изменен формат данных KNN и [синтаксис SQL KNN_DIST()](https://manual.manticoresearch.com/Searching/KNN?client=SQL#KNN-vector-search). Новая версия может читать старые данные, но старые версии не смогут читать новый формат.

### Новые возможности и улучшения
* 🆕 [v13.2.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.0) [ PR #3549](https://github.com/manticoresoftware/manticoresearch/issues/3525) Исправлена проблема с `@@collation_database`, вызывавшая несовместимость с некоторыми версиями mysqldump
* 🆕 [v13.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.1.0) [ Issue #3489](https://github.com/manticoresoftware/manticoresearch/issues/3489) Исправлена ошибка в Fuzzy Search, которая препятствовала разбору некоторых SQL-запросов
* 🆕 [v12.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/12.1.0) [ Issue #3426](https://github.com/manticoresoftware/manticoresearch/issues/3426) Добавлена поддержка операционных систем RHEL 10
* 🆕 [v11.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.1.0)  Добавлена поддержка пустых векторов с плавающей точкой в [KNN поиске](https://manual.manticoresearch.com/Searching/KNN#KNN-vector-search)
* 🆕 [v10.2.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.0) [ Issue #3252](https://github.com/manticoresoftware/manticoresearch/issues/3252) Переменная [log_level](https://manual.manticoresearch.com/Server_settings/Setting_variables_online#SET) теперь также регулирует уровень детализации логирования Buddy

### Исправления ошибок
* 🪲 [v13.2.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.3) [ PR #3556](https://github.com/manticoresoftware/manticoresearch/pull/3556) Исправлен разбор опции "oversampling" в JSON-запросах
* 🪲 [v13.2.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.2)  Исправлена ошибка логирования сбоев на Linux и FreeBSD путём удаления использования Boost stacktrace
* 🪲 [v13.2.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.1) [ Issue #3298](https://github.com/manticoresoftware/manticoresearch/issues/3298) Исправлено логирование сбоев при запуске внутри контейнера
* 🪲 [v12.0.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/12.0.2)  Повышена точность статистики по таблицам за счет учета времени в микросекундах
* 🪲 [v12.0.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/12.0.1) [ PR #3522](https://github.com/manticoresoftware/manticoresearch/pull/3522) Исправлен сбой при фасетировании по MVA в объединённом запросе
* 🪲 [v11.0.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.0.3) [ Issue #3502](https://github.com/manticoresoftware/manticoresearch/issues/3502) Исправлен сбой, связанный с Квантизацией Векторного Поиска
* 🪲 [v11.0.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.0.2) [ Issue #3493](https://github.com/manticoresoftware/manticoresearch/issues/3493) Изменён вывод команды `SHOW THREADS` для отображения загрузки CPU в виде целого числа
* 🪲 [v11.0.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.0.1)  Исправлены пути для колонных и вторичных библиотек
* 🪲 [v10.2.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.7)  Добавлена поддержка MCL 5.0.5, включая исправление имени файла библиотеки эмбеддингов
* 🪲 [v10.2.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.6) [ Issue #3469](https://github.com/manticoresoftware/manticoresearch/issues/3469) Внесено дополнительное исправление, связанное с issue #3469
* 🪲 [v10.2.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.4) [ Issue #3469](https://github.com/manticoresoftware/manticoresearch/issues/3469) Исправлена проблема при HTTP-запросах с булевыми запросами, возвращавшими пустые результаты
* 🪲 [v10.2.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.5)  Изменено имя файла по умолчанию для библиотеки эмбеддингов и добавлены проверки поля 'from' в KNN-векторах
* 🪲 [v10.2.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.3) [ PR #3464](https://github.com/manticoresoftware/manticoresearch/pull/3464) Обновлен Buddy до версии 3.30.2, включая [PR #565 о использовании памяти и логировании ошибок](https://github.com/manticoresoftware/manticoresearch-buddy/pull/565)
* 🪲 [v10.2.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.2) [ Issue #3470](https://github.com/manticoresoftware/manticoresearch/pull/3470) Исправлены фильтры строк JSON, фильтры NULL и проблемы с сортировкой в JOIN-запросах
* 🪲 [v10.2.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.1)  Исправлена ошибка в `dist/test_kit_docker_build.sh`, из-за которой отсутствовал коммит Buddy
* 🪲 [v10.1.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.4)  Исправлен сбой при группировке по MVA в объединённом запросе
* 🪲 [v10.1.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.3) [ Issue #3434](https://github.com/manticoresoftware/manticoresearch/issues/3434) Исправлена ошибка с фильтрацией пустых строк
* 🪲 [v10.1.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.2) [ PR #3452](https://github.com/manticoresoftware/manticoresearch/pull/3452) Обновлен Buddy до версии 3.29.7, которая решает [Buddy #507 — ошибка при нескольких запросах с fuzzy search](https://github.com/manticoresoftware/manticoresearch-buddy/issues/507) и [Buddy #561 — исправление для метрик](https://github.com/manticoresoftware/manticoresearch-buddy/pull/561), требуется для [Helm Release 10.1.0](https://github.com/manticoresoftware/manticoresearch-helm/releases/tag/manticoresearch-10.1.0)
* 🪲 [v10.1.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.1) Обновлен Buddy до версии 3.29.4, решена проблема [#3388 - "Undefined array key 'Field'"](https://github.com/manticoresoftware/manticoresearch/issues/3388) и [Buddy #547 - layouts='ru' может не работать](https://github.com/manticoresoftware/manticoresearch-buddy/issues/547)

## Версия 10.1.0
**Выпущено**: 9 июня 2025 года

Эта версия представляет собой обновление с новыми функциями, одним изменением, нарушающим совместимость, и множеством улучшений стабильности и исправлений ошибок. Изменения сосредоточены на расширении возможностей мониторинга, улучшении функциональности поиска и устранении различных критических проблем, влияющих на стабильность и производительность системы.

**Начиная с версии 10.1.0, поддержка CentOS 7 прекращена. Пользователям рекомендуется перейти на поддерживаемую операционную систему.**

### Изменения, нарушающие совместимость
* ⚠️ [v10.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.0.0) [ Issue #540](https://github.com/manticoresoftware/manticoresearch-buddy/issues/540) ИЗМЕНЕНИЕ, НАРУШАЮЩЕЕ СОВМЕСТИМОСТЬ: значение по умолчанию для [fuzzy search](Searching/Spell_correction.md#Fuzzy-Search) установлено в `layouts=''`

### Новые функции и улучшения
* 🆕 [v10.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.0) [ Issue #537](https://github.com/manticoresoftware/manticoresearch-buddy/issues/537) Добавлен встроенный [Prometheus exporter](Node_info_and_management/Node_status.md#Prometheus-Exporter)
* 🆕 [v9.8.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.8.0) [ Issue #3409](https://github.com/manticoresoftware/manticoresearch/issues/3409) Добавлена команда [ALTER TABLE tbl REBUILD KNN](Updating_table_schema_and_settings.md#Rebuilding-a-KNN-index)
* 🆕 [v9.7.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.7.0) [ Issue #1778](https://github.com/manticoresoftware/manticoresearch/issues/1778) Добавлена автоматическая генерация эмбеддингов (еще не объявлена официально, поскольку код находится в основной ветке и требует дополнительного тестирования)
* 🆕 [v9.6.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.6.0) Повышена версия API KNN для поддержки автоматических эмбеддингов
* 🆕 [v9.5.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.0) [ Issue #1894](https://github.com/manticoresoftware/manticoresearch/issues/1894) Улучшено восстановление кластера: периодическая сохранение `seqno` для более быстрого перезапуска ноды после сбоя
* 🆕 [v9.4.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.4.0) [ Issue #2400](https://github.com/manticoresoftware/manticoresearch/issues/2400) Добавлена поддержка последних версий [Logstash](Integration/Logstash.md#Integration-with-Logstash) и Beats

### Исправления ошибок
* 🪲 [v10.0.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.0.1) Исправлена обработка форм слов: пользовательские формы теперь переопределяют автоматически сгенерированные; добавлены тестовые случаи к тесту 22
* 🪲 [v9.8.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.8.2) Исправление: обновлен deps.txt для включения исправлений упаковки в MCL, связанных с библиотекой эмбеддингов
* 🪲 [v9.8.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.8.1) Исправление: обновлен deps.txt с исправлениями упаковки для MCL и библиотеки эмбеддингов
* 🪲 [v9.7.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.7.3) [ Issue #3306](https://github.com/manticoresoftware/manticoresearch/issues/3306) Исправлен сбой с сигналом 11 при индексировании
* 🪲 [v9.7.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.7.2) [ Issue #3109](https://github.com/manticoresoftware/manticoresearch/issues/3109) Исправлена проблема, когда несуществующие `@@variables` всегда возвращали 0
* 🪲 [v9.7.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.7.1) [ Issue #3377](https://github.com/manticoresoftware/manticoresearch/issues/3377) Исправлен сбой, связанный с функцией [remove_repeats()](Functions/Searching_and_ranking_functions.md#REMOVE_REPEATS%28%29)
* 🪲 [v9.6.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.6.3) [ PR #3411](https://github.com/manticoresoftware/manticoresearch/pull/3411) Исправление: использование динамического определения версий для метрик телеметрии
* 🪲 [v9.6.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.6.2) Исправление: небольшая правка вывода в [SHOW VERSION](Node_info_and_management/SHOW_VERSION.md#SHOW-VERSION)
* 🪲 [v9.6.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.6.1) Исправление: сбой при создании таблицы с KNN-атрибутом без модели
* 🪲 [v9.5.16](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.16) [ Issue #3342](https://github.com/manticoresoftware/manticoresearch/issues/3342) Исправлена проблема, когда `SELECT ... FUZZY=0` не всегда отключал fuzzy поиск
* 🪲 [v9.5.15](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.15) [ PR #3397](https://github.com/manticoresoftware/manticoresearch/pull/3397) Добавлена поддержка [MCL](https://github.com/manticoresoftware/columnar) 4.2.2; исправлены ошибки со старыми форматами хранения
* 🪲 [v9.5.14](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.14) [ Issue #3392](https://github.com/manticoresoftware/manticoresearch/issues/3392) Исправлена некорректная обработка строк в HTTP JSON ответах
* 🪲 [v9.5.13](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.13) [ Issue #3356](https://github.com/manticoresoftware/manticoresearch/issues/3356) Исправлен сбой в сложном случае полнотекстового запроса (common-sub-term)
* 🪲 [v9.5.12](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.12) Исправлена опечатка в сообщении об ошибке автоматической очистки дискового чанка
* 🪲 [v9.5.11](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.11) [ Issue #3195](https://github.com/manticoresoftware/manticoresearch/issues/3195) Улучшена [автоматическая очистка дискового чанка](Securing_and_compacting_a_table/Flushing_RAM_chunk_to_disk.md#Flushing-RAM-chunk-to-disk): пропуск сохранения при запущенной оптимизации
* 🪲 [v9.5.10](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.10) [ Issue #3313](https://github.com/manticoresoftware/manticoresearch/issues/3313) Исправлена проверка дублирующихся ID для всех дисковых чанков в RT таблице с использованием [indextool](Miscellaneous_tools.md#indextool)
* 🪲 [v9.5.9](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.9) [ Issue #3132](https://github.com/manticoresoftware/manticoresearch/issues/3132) Добавлена поддержка сортировки `_random` в JSON API
* 🪲 [v9.5.8](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.8) [ Issue #3382](https://github.com/manticoresoftware/manticoresearch/issues/3382) Исправлена ошибка: нельзя было использовать uint64 ID документа через JSON HTTP API
* 🪲 [v9.5.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.7) [ Issue #3385](https://github.com/manticoresoftware/manticoresearch/issues/3385) Исправлены некорректные результаты при фильтрации по `id != value`
* 🪲 [v9.5.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.6) [ PR #538](https://github.com/manticoresoftware/manticoresearch-buddy/pull/538) Исправлена критическая ошибка в нечётком сопоставлении в некоторых случаях
* 🪲 [v9.5.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.5) [ Issue #3199](https://github.com/manticoresoftware/manticoresearch/issues/3199) Исправлено декодирование пробелов в HTTP параметрах запроса Buddy (например, `%20` и `+`)
* 🪲 [v9.5.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.4) [ Issue #3133](https://github.com/manticoresoftware/manticoresearch/issues/3133) Исправлена некорректная сортировка по `json.field` в фасетном поиске
* 🪲 [v9.5.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.3) [ Issue #3091](https://github.com/manticoresoftware/manticoresearch/issues/3091) Исправлены несоответствующие результаты поиска для разделителей в SQL и JSON API
* 🪲 [v9.5.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.2) [ Issue #2819](https://github.com/manticoresoftware/manticoresearch/issues/2819) Повышена производительность: заменён `DELETE FROM` на `TRUNCATE` для распределённых таблиц
* 🪲 [v9.5.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.1) [ Issue #3080](https://github.com/manticoresoftware/manticoresearch/issues/3080) Исправлен сбой при фильтрации алиасом `geodist()` с JSON атрибутами

## Версия 9.3.2
Выпущено: 2 мая 2025 года

В этом релизе исправлено множество ошибок и улучшена стабильность, добавлено лучшее отслеживание использования таблиц, а также усовершенствования управления памятью и ресурсами.

❤️ Особая благодарность [@cho-m](https://github.com/cho-m) за исправление совместимости сборки с Boost 1.88.0 и [@benwills](https://github.com/benwills) за улучшение документации по `stored_only_fields`.

* 🪲 [v9.3.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.3.2)  Исправлена проблема, когда столбец "Show Threads" показывал активность CPU как число с плавающей точкой вместо строки; также исправлена ошибка парсинга результата PyMySQL из-за неправильного типа данных.
* 🪲 [v9.3.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.3.1) [ Issue #3343](https://github.com/manticoresoftware/manticoresearch/issues/3343) Исправлены оставшиеся файлы `tmp.spidx`, возникавшие при прерывании процесса оптимизации.
* 🆕 [v9.3.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.3.0) [ PR #3337](https://github.com/manticoresoftware/manticoresearch/pull/3337) Добавлен счётчик команд на таблицу и детальная статистика использования таблиц.
* 🪲 [v9.2.39](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.39) [ Issue #3236](https://github.com/manticoresoftware/manticoresearch/issues/3236) Исправление: предотвращение повреждения таблиц удалением сложных обновлений чанков. Использование функций ожидания в serial worker ломало последовательную обработку, что могло повредить таблицы.
Переосмыслен autoflush. Удалена внешняя очередь опроса, чтобы избежать ненужных блокировок таблиц. Добавлено условие "малой таблицы": если количество документов ниже 'small table limit' (8192) и Не используется Secondary Index (SI), сброс пропускается.

* 🪲 [v9.2.38](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.38)  Исправление: пропущено создание Secondary Index (SI) для фильтров с использованием `ALL`/`ANY` в строковых списках, без влияния на JSON атрибуты.
* 🪲 [v9.2.37](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.37) [ Issue #2898](https://github.com/manticoresoftware/manticoresearch/issues/2898) Добавлена поддержка обратных апострофов для системных таблиц.
* 🪲 [v9.2.36](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.36)  Исправление: использование заполнителя для кластерных операций в наследуемом коде. В парсере теперь ясно разделяются поля для имён таблиц и кластеров.
* 🪲 [v9.2.35](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.35)  Исправление: сбой при удалении кавычек с одиночной `'`.
* 🪲 [v9.2.34](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.34) [ Issue #3090](https://github.com/manticoresoftware/manticoresearch/issues/3090) Исправление: обработка больших ID документов (ранее могло не находить).
* 🪲 [v9.2.33](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.33)  Исправление: использование беззнаковых целых чисел для размеров битовых векторов.
* 🪲 [v9.2.32](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.32)  Исправление: снижение пикового использования памяти при слиянии. Поиск docid-to-rowid теперь использует 12 байт на документ вместо 16 байт. Пример: 24 ГБ ОЗУ на 2 миллиарда документов вместо 36 ГБ.
* 🪲 [v9.2.31](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.31) [ Issue #3238](https://github.com/manticoresoftware/manticoresearch/issues/3238) Исправление: неправильное значение `COUNT(*)` в больших таблицах реального времени.
* 🪲 [v9.2.30](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.30)  Исправление: неопределённое поведение при обнулении строковых атрибутов.
* 🪲 [v9.2.29](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.29)  Небольшое исправление: улучшен текст предупреждения.
* 🪲 [v9.2.28](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.28) [ Issue #3290](https://github.com/manticoresoftware/manticoresearch/issues/3290) Улучшение: расширена поддержка `indextool --buildidf`
* 🪲 [v9.2.27](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.27) [ Issue #3032](https://github.com/manticoresoftware/manticoresearch/issues/3032) С интеграцией Kafka теперь можно создавать источник для конкретного партиционирования Kafka.
* 🪲 [v9.2.26](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.26) [ Issue #3301](https://github.com/manticoresoftware/manticoresearch/issues/3301) Исправление: `ORDER BY` и `WHERE` по `id` могли вызывать ошибки OOM (Out Of Memory).
* 🪲 [v9.2.25](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.25) [ Issue #3171](https://github.com/manticoresoftware/manticoresearch/issues/3171) Исправление: сбой из-за ошибки сегментации при использовании grouper с несколькими атрибутами JSON в RT таблице с несколькими дисковыми чанками
* 🪲 [v9.2.24](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.24) [ Issue #3246](https://github.com/manticoresoftware/manticoresearch/issues/3246) Исправление: запросы `WHERE string ANY(...)` не выполнялись после сброса RAM чанка.
* 🪲 [v9.2.23](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.23) [ PR #518](https://github.com/manticoresoftware/manticoresearch-buddy/pull/518) Небольшие улучшения синтаксиса авто-шардирования.
* 🪲 [v9.2.22](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.22) [ Issue #2763](https://github.com/manticoresoftware/manticoresearch/issues/2763) Исправление: глобальный файл idf не загружался при использовании `ALTER TABLE`.
* 🪲 [v9.2.21](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.21)  Исправление: глобальные файлы idf могут быть большими. Теперь мы освобождаем таблицы быстрее, чтобы избежать удержания ненужных ресурсов.
* 🪲 [v9.2.20](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.20) [ PR #3277](https://github.com/manticoresoftware/manticoresearch/pull/3277) Улучшение: лучшая проверка параметров шардинга.

* 🪲 [v9.2.19](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.19) [ PR #3275](https://github.com/manticoresoftware/manticoresearch/pull/3275) Исправление: совместимость сборки с Boost 1.88.0.
* 🪲 [v9.2.18](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.18) [ Issue #3228](https://github.com/manticoresoftware/manticoresearch/issues/3228) Исправление: сбой при создании распределенной таблицы (ошибка недопустимого указателя).

* 🪲 [v9.2.17](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.17) [ PR #3272](https://github.com/manticoresoftware/manticoresearch/pull/3272) Исправление: проблема с многострочным нечетким `FACET`.
* 🪲 [v9.2.16](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.16) [ Issue #3063](https://github.com/manticoresoftware/manticoresearch/issues/3063) Исправление: ошибка в вычислении расстояния при использовании функции `GEODIST`.
* 🪲 [v9.2.15](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.15) [ Issue #3027](https://github.com/manticoresoftware/manticoresearch/issues/3027) Небольшое улучшение: поддержка формата фильтра `query_string` Elastic.

## Version 9.2.14
Released: March 28th 2025

### Minor changes
* [Commit ](https://github.com/manticoresoftware/manticoresearch/commit/fe9473dc3f4fcfa0eaae748b538fe853f4ce078b) Реализован флаг `--mockstack` для вычисления требований к стеку рекурсивных операций. Новый режим `--mockstack` анализирует и сообщает необходимые размеры стека для рекурсивной оценки выражений, операций сопоставления паттернов, обработки фильтров. Рассчитанные требования к стеку выводятся в консоль для целей отладки и оптимизации.
* [Issue #3058](https://github.com/manticoresoftware/manticoresearch/pull/3058) Включен по умолчанию [boolean_simplify](Searching/Options.md#boolean_simplify).
* [Issue #3172](https://github.com/manticoresoftware/manticoresearch/issues/3172) Добавлена новая опция конфигурации: `searchd.kibana_version_string`, которая может быть полезна при использовании Manticore с конкретными версиями Kibana или OpenSearch Dashboards, ожидающими определённую версию Elasticsearch.
* [Issue #3211](https://github.com/manticoresoftware/manticoresearch/issues/3211) Исправлена работа [CALL SUGGEST](Searching/Spell_correction.md#CALL-QSUGGEST,-CALL-SUGGEST) с двухсимвольными словами.
* [Issue #490](https://github.com/manticoresoftware/manticoresearch-buddy/issues/490) Улучшен [нечеткий поиск](Searching/Spell_correction.md#Fuzzy-Search): ранее он иногда не мог найти "defghi", когда искали "def ghi", если существовал другой подходящий документ.
* ⚠️ BREAKING [Issue #3165](https://github.com/manticoresoftware/manticoresearch/issues/3165) Изменено `_id` на `id` в некоторых HTTP JSON ответах для согласованности. Убедитесь, что обновили своё приложение соответственно.
* ⚠️ BREAKING [Issue #3186](https://github.com/manticoresoftware/manticoresearch/issues/3186) Добавлена проверка `server_id` при присоединении к кластеру, чтобы гарантировать уникальность ID каждого узла. Операция `JOIN CLUSTER` теперь может завершиться ошибкой с сообщением о дубликате [server_id](Server_settings/Searchd.md#server_id), если присоединяющийся узел имеет такой же `server_id`, как уже существующий в кластере. Чтобы решить эту проблему, убедитесь, что каждый узел в репликационном кластере имеет уникальный [server_id](Server_settings/Searchd.md#server_id). Вы можете изменить стандартный [server_id](Server_settings/Searchd.md#server_id) в разделе "searchd" вашего конфигурационного файла на уникальное значение перед попыткой присоединения к кластеру. Это изменение обновляет протокол репликации. Если вы запускаете репликационный кластер, вам нужно:
  - Сначала корректно остановить все ваши узлы
  - Затем запустить узел, который был остановлен последним, с `--new-cluster`, используя инструмент `manticore_new_cluster` в Linux.
  - Прочитайте подробнее о [перезапуске кластера](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster).

### Bugfixes
* [Commit 6fda](https://github.com/manticoresoftware/manticoresearch/commit/6fdad3923dd6953f4b781943eed5ec5c28b7f808) Исправлен сбой, вызванный потерей планировщика после ожидания; теперь конкретные планировщики, такие как `serializer`, правильно восстанавливаются.
* [Commit c333](https://github.com/manticoresoftware/manticoresearch/commit/c333b277d4a504de5fe298e9ab570a9f9ea31e0a) Исправлена ошибка, при которой веса из правой присоединённой таблицы не могли использоваться в выражении `ORDER BY`.
* [Issue #2644](https://github.com/manticoresoftware/manticoresearch/issues/2644) gcc 14.2.0: исправлена ошибка вызова `lower_bound` с `const knn::DocDist_t*&`. ❤️ Спасибо [@Azq2](https://github.com/Azq2) за PR.
* [Issue #3018](https://github.com/manticoresoftware/manticoresearch/issues/3018) Исправлена проблема с обработкой имён таблиц в верхнем регистре при добавлении через auto-schema.
* [Issue #3119](https://github.com/manticoresoftware/manticoresearch/issues/3119) Исправлен сбой при декодировании недопустимого base64 ввода.
* [Issue #3121](https://github.com/manticoresoftware/manticoresearch/issues/3121) Исправлены две связанные ошибки с KNN индексом при `ALTER`: векторы с плавающей точкой сохраняют свои первоначальные размеры, и KNN индексы теперь корректно создаются.
* [Issue #3123](https://github.com/manticoresoftware/manticoresearch/issues/3123) Исправлен сбой при создании вторичного индекса на пустом JSON столбце.
* [Issue #3138](https://github.com/manticoresoftware/manticoresearch/issues/3138) Исправлен сбой, вызванный дублирующими записями.
* [Issue #3151](https://github.com/manticoresoftware/manticoresearch/issues/3151) Исправлено: опция `fuzzy=1` не могла использоваться вместе с `ranker` или `field_weights`.
* [Issue #3163](https://github.com/manticoresoftware/manticoresearch/issues/3163) Исправлена ошибка, из-за которой `SET GLOBAL timezone` не работал.
* [Issue #3181](https://github.com/manticoresoftware/manticoresearch/issues/3181) Исправлена проблема с потерей значений текстовых полей при использовании ID больше 2^63.
* [Issue #3189](https://github.com/manticoresoftware/manticoresearch/issues/3189) Исправлено: `UPDATE` теперь корректно соблюдает настройку `query_log_min_msec`.
* [Issue #3247](https://github.com/manticoresoftware/manticoresearch/issues/3247) Исправлена гонка при сохранении дисковых чанков в режиме реального времени, которая могла приводить к сбою `JOIN CLUSTER`.


## Version 7.4.6
Released: February 28th 2025

### Major changes
* [Issue #832](https://github.com/manticoresoftware/manticoresearch/issues/832) Интеграция с [Kibana](Integration/Kibana.md) для более простой и эффективной визуализации данных.

### Minor changes
* [Issue #1727](https://github.com/manticoresoftware/manticoresearch/issues/1727) Исправлены различия в точности с плавающей точкой между arm64 и x86_64.
* [Issue #2995](https://github.com/manticoresoftware/manticoresearch/issues/2995) Реализованы оптимизации производительности пакетов соединений.
* [Issue #3039](https://github.com/manticoresoftware/manticoresearch/issues/3039) Реализованы оптимизации производительности для EstimateValues в гистограммах.
* [Issue #3099](https://github.com/manticoresoftware/manticoresearch/issues/3099) Добавлена поддержка Boost 1.87.0. ❤️ Спасибо, [@cho-m](https://github.com/cho-m) за PR.
* [Issue #77](https://github.com/manticoresoftware/columnar/issues/77) Оптимизировано повторное использование блоков данных при создании фильтров с несколькими значениями; добавлены min/max к метаданным атрибутов; реализована предварительная фильтрация значений фильтра на основе min/max.

### Bugfixes
* [Commit 73ac](https://github.com/manticoresoftware/manticoresearch/commit/73ac22f358a7a0e734b332c3943c86e6294c06c3) Исправлена обработка выражений в запросах с объединением, когда используются атрибуты из обеих таблиц; исправлена опция index_weights для правой таблицы.
* [Issue #2915](https://github.com/manticoresoftware/manticoresearch/issues/2915) Использование `avg()` в запросе `SELECT ... JOIN` могло приводить к некорректным результатам; теперь исправлено.
* [Issue #2996](https://github.com/manticoresoftware/manticoresearch/issues/2996) Исправлен неправильный набор результатов, вызванный неявным отсечением при включенном пакетировании соединений.
* [Issue #3031](https://github.com/manticoresoftware/manticoresearch/issues/3031) Исправлен сбой демона при завершении работы, если была активна операция слияния чанков.
* [Issue #3037](http://github.com/manticoresoftware/manticoresearch/issues/3037) Исправлена проблема, когда `IN(...)` мог давать неверные результаты.
* [Issue #3038](https://github.com/manticoresoftware/manticoresearch/issues/3038) Установка `max_iops` / `max_iosize` в версии 7.0.0 могла ухудшать производительность индексирования; исправлено.
* [Issue #3042](https://github.com/manticoresoftware/manticoresearch/issues/3042) Исправлена утечка памяти в кэше запросов с объединением.
* [Issue #3052](https://github.com/manticoresoftware/manticoresearch/issues/3052) Исправлена обработка опций запросов в объединённых JSON запросах.
* [Issue #3054](https://github.com/manticoresoftware/manticoresearch/issues/3054) Исправлена проблема с командой ATTACH TABLE.
* [Issue #3079](https://github.com/manticoresoftware/manticoresearch/issues/3079) Исправлены несоответствия в сообщениях об ошибках.
* [Issue #3087](https://github.com/manticoresoftware/manticoresearch/issues/3087) Установка `diskchunk_flush_write_timeout=-1` для каждой таблицы не отключала сброс индекса; исправлено.
* [Issue #3088](https://github.com/manticoresoftware/manticoresearch/issues/3088) Исправлено появление дублирующихся записей после массовой замены больших ID.
* [Issue #3126](https://github.com/manticoresoftware/manticoresearch/issues/3126) Исправлен сбой демона, вызванный полнотекстовым запросом с одиночным оператором `NOT` и Ранкером выражений.
* [Issue #3128](https://github.com/manticoresoftware/manticoresearch/pull/3128) Исправлена потенциальная уязвимость в библиотеке CJSON. ❤️ Спасибо, [@tabudz](https://github.com/tabudz) за PR.

## Version 7.0.0
Released: January 30th 2025

### Major changes
* [Issue #1497](https://github.com/manticoresoftware/manticoresearch/issues/1497) Добавлены новые возможности [Fuzzy Search](Searching/Spell_correction.md#Fuzzy-Search) и [Autocomplete](Searching/Autocomplete.md#CALL-AUTOCOMPLETE) для облегчения поиска.
* [Issue #1500](https://github.com/manticoresoftware/manticoresearch/issues/1500) [Интеграция с Kafka](Integration/Kafka.md#Syncing-from-Kafka).
* [Issue #1928](https://github.com/manticoresoftware/manticoresearch/issues/1928) Введены [вторичные индексы для JSON](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#json_secondary_indexes).
* [Issue #2361](https://github.com/manticoresoftware/manticoresearch/issues/2361) Обновления и поиски во время обновлений больше не блокируются слиянием чанков.
* [Issue #2787](https://github.com/manticoresoftware/manticoresearch/issues/2787) Автоматический [сброс дискового чанка](Server_settings/Searchd.md#diskchunk_flush_write_timeout) для RT таблиц для улучшения производительности; теперь мы автоматически сбрасываем RAM-чанк в дисковый чанк, предотвращая проблемы с производительностью, вызванные отсутствием оптимизаций в RAM-чанках, что иногда могло приводить к нестабильности в зависимости от размера чанка.
* [Issue #2811](https://github.com/manticoresoftware/manticoresearch/issues/2811) Опция [Scroll](Searching/Pagination.md#Scroll-Search-Option) для более удобной пагинации.
* [Issue #931](https://github.com/manticoresoftware/manticoresearch/issues/931) Интеграция с [Jieba](https://github.com/fxsjy/jieba) для лучшей [токенизации китайского языка](Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md).

### Небольшие изменения
* ⚠️ НЕСОВМЕСТИМОЕ ИЗМЕНЕНИЕ [Issue #1111](https://github.com/manticoresoftware/manticoresearch/issues/1111) Исправлена поддержка `global_idf` в RT таблицах. Требуется пересоздание таблицы.
* ⚠️ НЕСОВМЕСТИМОЕ ИЗМЕНЕНИЕ [Issue #2103](https://github.com/manticoresoftware/manticoresearch/issues/2103) Удалены тайские символы из внутреннего набора символов `cjk`. Обновите определения наборов символов соответственно: если у вас есть `cjk,non_cjk` и тайские символы важны для вас, замените на `cjk,thai,non_cjk` или `cont,non_cjk`, где `cont` — новое обозначение для всех языков с непрерывным письмом (т.е. `cjk` + `thai`). Измените существующие таблицы с помощью [ALTER TABLE](Updating_table_schema_and_settings.md#Updating-table-FT-settings-in-RT-mode).
* ⚠️ НЕСОВМЕСТИМОЕ ИЗМЕНЕНИЕ [Issue #2468](https://github.com/manticoresoftware/manticoresearch/issues/2468) [CALL SUGGEST / QSUGGEST](Searching/Spell_correction.md#CALL-QSUGGEST,-CALL-SUGGEST) теперь совместимы с распределёнными таблицами. Это увеличивает версию протокола master/agent. Если вы используете Manticore Search в распределённой среде с несколькими инстансами, убедитесь, что сначала обновили агенты, затем мастера.
* ⚠️ НЕСОВМЕСТИМОЕ ИЗМЕНЕНИЕ [Issue #2889](https://github.com/manticoresoftware/manticoresearch/issues/2889) Изменено имя столбца с `Name` на `Variable name` для PQ [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META-for-PQ-tables).
* ⚠️ НЕСОВМЕСТИМОЕ ИЗМЕНЕНИЕ [Issue #879](https://github.com/manticoresoftware/manticoresearch/issues/879) Введено [перетабличное бинарное логирование](Logging/Binary_logging.md#Per-table-binary-logging-configuration) с новыми опциями: [binlog_common](Logging/Binary_logging.md#Binary-logging-strategies), [binlog](Logging/Binary_logging.md#Per-table-binary-logging-configuration) для `create table` / `alter table`. Перед обновлением на новую версию требуется корректное завершение работы экземпляра Manticore.
* ⚠️ НЕСОВМЕСТИМОЕ ИЗМЕНЕНИЕ [Issue #1789](https://github.com/manticoresoftware/manticoresearch/issues/1789) Исправлено некорректное сообщение об ошибке при подключении ноды к кластеру с неверной версией протокола репликации. Это изменение обновляет протокол репликации. Если вы используете кластер репликации, вам нужно:
  - Сначала корректно остановить все ноды
  - Затем запустить последнюю остановленную ноду с `--new-cluster`, используя утилиту `manticore_new_cluster` в Linux.
  - Подробнее о [перезапуске кластера](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster).
* ⚠️ НЕСОВМЕСТИМОЕ ИЗМЕНЕНИЕ [Issue #2308](https://github.com/manticoresoftware/manticoresearch/issues/2308) Добавлена поддержка множества таблиц в [`ALTER CLUSTER ADD` и `DROP`](Creating_a_cluster/Setting_up_replication/Adding_and_removing_a_table_from_a_replication_cluster.md#Adding-and-removing-a-table-from-a-replication-cluster). Это изменение также затрагивает протокол репликации. Обратитесь к предыдущему разделу для инструкций по обновлению.
* [Issue #2997](https://github.com/manticoresoftware/manticoresearch/issues/2997) Исправлена проблема с dlopen на Macos.
* [Commit 4954](https://github.com/manticoresoftware/manticoresearch/commit/4954b5de7341a29902a9b8fbb9a040f7942c77c4) Изменён стандартный порог для OPTIMIZE TABLE на таблицах с индексами KNN для улучшения производительности поиска.
* [Commit cfc8](https://github.com/manticoresoftware/manticoresearch/commit/cfc87ecb6e33a8163c2235243b6b40e699dbf526) Добавлена поддержка `COUNT(DISTINCT)` для `ORDER BY` в `FACET` и `GROUP BY`.
* [Issue #1103](https://github.com/manticoresoftware/manticoresearch/issues/1103) Улучшена ясность логирования [слияния чанков](Logging/Server_logging.md#Server-logging).
* [Issue #1130](https://github.com/manticoresoftware/manticoresearch/issues/1130) Добавлена поддержка [DBeaver](Integration/DBeaver.md).
* [Issue #1546](https://github.com/manticoresoftware/manticoresearch/issues/1546) Реализованы вторичные индексы для функций [POLY2D()](Functions/Geo_spatial_functions.md#POLY2D%28%29)/[GEOPOLY2D()](Functions/Geo_spatial_functions.md#GEOPOLY2D%28%29).
* [Issue #1630](https://github.com/manticoresoftware/manticoresearch/issues/1630) HTTP-запросы теперь поддерживают `Content-Encoding: gzip`.
* [Issue #1831](https://github.com/manticoresoftware/manticoresearch/issues/1831) Добавлена команда `SHOW LOCKS`.
* [Issue #2187](https://github.com/manticoresoftware/manticoresearch/issues/2197) Запрос Buddy к демону теперь может обходить ограничение [searchd.max_connections](Server_settings/Searchd.md#max_connections).
* [Issue #2208](https://github.com/manticoresoftware/manticoresearch/issues/2208) Добавлена поддержка объединения таблиц через JSON HTTP интерфейс.
* [Issue #2235](https://github.com/manticoresoftware/manticoresearch/issues/2235) Логируются успешно обработанные запросы через Buddy в их исходном виде.
* [Issue #2249](https://github.com/manticoresoftware/manticoresearch/issues/2249) Добавлен специальный режим запуска `mysqldump` для реплицируемых таблиц.
* [Issue #2268](https://github.com/manticoresoftware/manticoresearch/issues/2268) Улучшено переименование внешних файлов при копировании для операторов `CREATE TABLE` и `ALTER TABLE`.
* [Issue #2402](https://github.com/manticoresoftware/manticoresearch/issues/2402) Обновлено значение по умолчанию [searchd.max_packet_size](Server_settings/Searchd.md#max_packet_size) до 128MB.
* [Issue #2419](https://github.com/manticoresoftware/manticoresearch/issues/2419) Добавлена поддержка [IDF boost modifier](Searching/Full_text_matching/Operators.md#IDF-boost-modifier) в JSON ["match"](Searching/Full_text_matching/Basic_usage.md#match).
* [Issue #2430](https://github.com/manticoresoftware/manticoresearch/issues/2430) Улучшена синхронизация записи [binlog](Logging/Binary_logging.md#Binary-logging) для предотвращения ошибок.
* [Issue #2458](https://github.com/manticoresoftware/manticoresearch/issues/2458) Включена поддержка zlib в пакетах для Windows.
* [Issue #2479](https://github.com/manticoresoftware/manticoresearch/issues/2479) Добавлена поддержка команды SHOW TABLE INDEXES.
* [Issue #2485](https://github.com/manticoresoftware/manticoresearch/issues/2485) Установлены метаданные сессии для ответов Buddy.
* [Issue #2490](https://github.com/manticoresoftware/manticoresearch/issues/2490) Разрешение в миллисекундах для агрегатов на совместимых эндпоинтах.
* [Issue #2500](https://github.com/manticoresoftware/manticoresearch/issues/2500) Изменены сообщения об ошибках для операций кластера при сбое запуска репликации.
* [Issue #2584](https://github.com/manticoresoftware/manticoresearch/pull/2584) Новые [метрики производительности в SHOW STATUS](Node_info_and_management/Node_status.md#Query-Time-Statistics): min/max/avg/95-й/99-й процентиль по типам запросов за последние 1, 5 и 15 минут.
* [Issue #2639](https://github.com/manticoresoftware/manticoresearch/pull/2639) Заменены все вхождения `index` на `table` в запросах и ответах.
* [Issue #2643](https://github.com/manticoresoftware/manticoresearch/issues/2643) Добавлен столбец `distinct` в результаты агрегации HTTP `/sql` эндпоинта.
* [Issue #268](https://github.com/manticoresoftware/manticoresearch-buddy/issues/268) Реализовано автодетектирование типов данных, импортируемых из Elasticsearch.
* [Issue #2744](https://github.com/manticoresoftware/manticoresearch/issues/2744) Добавлена поддержка сопоставления (collation) для выражений сравнения строковых JSON-полей.
* [Issue #2752](https://github.com/manticoresoftware/manticoresearch/issues/2752) Добавлена поддержка выражения `uuid_short` в списке select.
* [Issue #2783](https://github.com/manticoresoftware/manticoresearch/issues/2783) Manticore Search теперь запускает Buddy напрямую без обертки `manticore-buddy`.
* [Issue #2785](https://github.com/manticoresoftware/manticoresearch/issues/2785) Дифференцированные сообщения об ошибках для отсутствующих таблиц и таблиц без поддержки операций вставки.
* [Issue #2789](https://github.com/manticoresoftware/manticoresearch/issues/2789) OpenSSL 3 теперь статически встроен в `searchd`.
* [Issue #2790](https://github.com/manticoresoftware/manticoresearch/issues/2790) Добавлено выражение `CALL uuid_short` для генерации последовательностей с несколькими значениями `uuid_short`.
* [Issue #2803](https://github.com/manticoresoftware/manticoresearch/issues/2803) Добавлены отдельные опции для правой таблицы в операции JOIN.
* [Issue #2810](https://github.com/manticoresoftware/manticoresearch/issues/2810) Улучшена производительность агрегаций HTTP JSON до уровня `GROUP BY` в SphinxQL.
* [Issue #2854](https://github.com/manticoresoftware/manticoresearch/issues/2854) Добавлена поддержка `fixed_interval` в запросах Kibana, связанных с датами.
* [Issue #2909](https://github.com/manticoresoftware/manticoresearch/issues/2909) Реализована пакетная обработка JOIN-запросов, что улучшает производительность некоторых JOIN-запросов на сотни и даже тысячи раз.
* [Issue #2937](https://github.com/manticoresoftware/manticoresearch/issues/2937) Включена возможность использования веса объединённой таблицы в fullscan-запросах.
* [Issue #2953](https://github.com/manticoresoftware/manticoresearch/issues/2953) Исправлено логирование для join-запросов.
* [Issue #337](https://github.com/manticoresoftware/manticoresearch-buddy/issues/337) Скрыты исключения Buddy из лога `searchd` в неотладочном режиме.
* [Issue #2931](https://github.com/manticoresoftware/manticoresearch/issues/2931) Завершение работы демона с сообщением об ошибке при неверных портах репликационного слушателя, заданных пользователем.

### Исправления ошибок
* [Commit 0c6b](https://github.com/manticoresoftware/manticoresearch/commit/0c6bdaf41ea3684e05952e1a8273893535b923f5) Исправлено: некорректные результаты в JOIN-запросах с более чем 32 столбцами.
* [Issue #2335](https://github.com/manticoresoftware/manticoresearch/issues/2335) Решена проблема с неудачным объединением таблиц при использовании двух json-атрибутов в условии.
* [Issue #2338](https://github.com/manticoresoftware/manticoresearch/issues/2338) Исправлен некорректный total_relation в мультизапросах с [cutoff](Searching/Options.md#cutoff).
* [Issue #2366](https://github.com/manticoresoftware/manticoresearch/issues/2366) Исправлена фильтрация по `json.string` в правой таблице при [table join](Searching/Joining.md).
* [Issue #2406](https://github.com/manticoresoftware/manticoresearch/issues/2406) Разрешено использовать `null` для всех значений во всех POST HTTP JSON эндпоинтах (insert/replace/bulk). В этом случае используется значение по умолчанию.
* [Issue #2418](https://github.com/manticoresoftware/manticoresearch/issues/2418) Оптимизирован расход памяти за счет корректировки выделения [max_packet_size](Server_settings/Searchd.md#max_packet_size) сетевого буфера для начальной проверки сокета.
* [Issue #2420](https://github.com/manticoresoftware/manticoresearch/issues/2420) Исправлена вставка unsigned int в атрибут bigint через JSON-интерфейс.
* [Issue #2422](https://github.com/manticoresoftware/manticoresearch/issues/2422) Исправлена работа вторичных индексов с exclude-фильтрами при включенном pseudo_sharding.
* [Issue #2423](https://github.com/manticoresoftware/manticoresearch/issues/2423) Исправлена ошибка в [manticore_new_cluster](Starting_the_server/Manually.md#searchd-command-line-options).
* [Issue #2448](https://github.com/manticoresoftware/manticoresearch/issues/2448) Исправлен сбой демона при некорректном запросе `_update`.
* [Issue #2452](https://github.com/manticoresoftware/manticoresearch/issues/2452) Исправлена неспособность гистограмм обрабатывать фильтры значений с исключениями.
* [Issue #55](https://github.com/manticoresoftware/columnar/issues/55) Исправлены запросы [knn](Searching/KNN.md#KNN-vector-search) к распределённым таблицам.
* [Issue #68](https://github.com/manticoresoftware/columnar/issues/68) Улучшена обработка фильтров исключений при кодировании таблиц в columnar accessor.
* [Commit 0eb1](https://github.com/manticoresoftware/manticoresearch/commit/0eb1579462013cb134bcaf74e06f6eeb0c2ecc4d) Исправлен парсер выражений, который не учитывал переопределённый `thread_stack`.
* [Commit c304](https://github.com/manticoresoftware/manticoresearch/commit/c3041fd1d3e14773f3c5aa635343915a71f5cb95) Исправлен сбой при клонировании columnar IN выражения.
* [Commit edad](https://github.com/manticoresoftware/columnar/commit/edadc694c68d6022bdd13134263667430a42cc1d) Исправлена проблема инверсии в итераторе bitmap, которая вызывала сбой.
* [Commit fc30](https://github.com/manticoresoftware/executor/commit/fc302ff1117b0b835a6f5a3c07057baf5fba14d9) Исправлена проблема, из-за которой некоторые пакеты Manticore автоматически удалялись `unattended-upgrades`.
* [Issue #1019](https://github.com/manticoresoftware/manticoresearch/issues/1019) Улучшена обработка запросов из инструмента DbForge MySQL.
* [Issue #1107](https://github.com/manticoresoftware/manticoresearch/issues/1107) Исправлено экранирование специальных символов в `CREATE TABLE` и `ALTER TABLE`.  ❤️ Спасибо, [@subnix](https://github.com/subnix) за PR.
* [Issue #116](https://github.com/manticoresoftware/manticoresearch-backup/issues/116#issuecomment-2216597206) Исправлен взаимоблок на обновлении blob атрибута в замороженном индексе. Взаимоблок возникал из-за конфликтов блокировок при попытке разморозить индекс. Это также могло вызвать сбои в manticore-backup.
* [Issue #1818](https://github.com/manticoresoftware/manticoresearch/issues/1818) `OPTIMIZE` теперь выдаёт ошибку при замороженной таблице.
* [Issue #2001](https://github.com/manticoresoftware/manticoresearch/issues/2001) Разрешено использование имён функций в качестве имён колонок.
* [Issue #2153](https://github.com/manticoresoftware/manticoresearch/issues/2153) Исправлен сбой демона при запросе настроек таблицы с неизвестным дисковым чанком.
* [Issue #2184](https://github.com/manticoresoftware/manticoresearch/issues/2184) Исправлена проблема, из-за которой `searchd` зависал при остановке после `FREEZE` и `FLUSH RAMCHUNK`.
* [Issue #2228](https://github.com/manticoresoftware/manticoresearch/issues/2228) Удалены связанные с датой/временем токены (и regex) из списка зарезервированных слов.
* [Issue #2255](https://github.com/manticoresoftware/manticoresearch/issues/2255) Исправлен сбой при `FACET` с более чем 5 полями сортировки.
* [Issue #2265](https://github.com/manticoresoftware/manticoresearch/issues/2265) Исправлен сбой восстановления `mysqldump` с включённым `index_field_lengths`.
* [Issue #2291](https://github.com/manticoresoftware/manticoresearch/issues/2291) Исправлен сбой при выполнении команды `ALTER TABLE`.
* [Issue #2333](https://github.com/manticoresoftware/manticoresearch/issues/2333) Исправлена Windows DLL MySQL в пакете для корректной работы индексатора.
* [Issue #2393](https://github.com/manticoresoftware/manticoresearch/issues/2393) Исправлена ошибка компиляции GCC. ❤️ Спасибо, [@animetosho](https://github.com/animetosho) за PR.
* [Issue #2447](https://github.com/manticoresoftware/manticoresearch/issues/2447) Исправлена проблема с экранированием в [_update](Data_creation_and_modification/Updating_documents/REPLACE.md?client=Elasticsearch-like+partial).
* [Issue #2460](https://github.com/manticoresoftware/manticoresearch/issues/2460) Исправлен сбой индексатора при объявлении нескольких атрибутов или полей с одинаковым именем.
* [Issue #2467](https://github.com/manticoresoftware/manticoresearch/issues/2467) Исправлен сбой демона при неправильном преобразовании вложенных булевых запросов для связанных с "compat" поисковых endpoint-ов.
* [Issue #2493](https://github.com/manticoresoftware/manticoresearch/issues/2493) Исправлено расширение в фразах с модификаторами.
* [Issue #2535](https://github.com/manticoresoftware/manticoresearch/issues/2535) Исправлен сбой демона при использовании [ZONE](Searching/Full_text_matching/Operators.md#ZONE-limit-operator) или [ZONESPAN](Searching/Full_text_matching/Operators.md#ZONESPAN-limit-operator) полнотекстовых операторов.
* [Issue #2552](https://github.com/manticoresoftware/manticoresearch/issues/2552) Исправлена генерация инфиксов для обычных и RT таблиц с ключевым словарём.
* [Issue #2571](https://github.com/manticoresoftware/manticoresearch/issues/2571) Исправлен ответ с ошибкой в запросе `FACET`; установлен порядок сортировки по умолчанию `DESC` для `FACET` с `COUNT(*)`.
* [Issue #2580](https://github.com/manticoresoftware/manticoresearch/issues/2580) Исправлен сбой демона на Windows при запуске.
* [Issue #2603](https://github.com/manticoresoftware/manticoresearch/issues/2603) Исправлено усечение запросов для HTTP endpoint-ов `/sql` и `/sql?mode=raw`; обеспечена согласованность запросов без необходимости заголовка `query=`.
* [Issue #2623](https://github.com/manticoresoftware/manticoresearch/issues/2623) Исправлена проблема, при которой auto-schema создавал таблицу, но одновременно завершался с ошибкой.
* [Issue #2627](https://github.com/manticoresoftware/manticoresearch/issues/2627) Исправлена библиотека HNSW для поддержки загрузки нескольких KNN индексов.
* [Issue #2630](https://github.com/manticoresoftware/manticoresearch/issues/2630) Исправлена проблема заморозки при одновременном выполнении нескольких условий.
* [Issue #2645](https://github.com/manticoresoftware/manticoresearch/issues/2645) Исправлен сбой с фатальной ошибкой при использовании `OR` с KNN поиском.
* [Issue #2647](https://github.com/manticoresoftware/manticoresearch/issues/2647) Исправлена ошибка, при которой `indextool --mergeidf *.idf --out global.idf` удалял выходной файл после создания.
* [Issue #2658](https://github.com/manticoresoftware/manticoresearch/issues/2658) Исправлен сбой демона при подзапросе с `ORDER BY` строкой во внешнем запросе.
* [Issue #2686](https://github.com/manticoresoftware/manticoresearch/issues/2686) Исправлен сбой при обновлении атрибута float вместе с атрибутом string.
* [Issue #2704](https://github.com/manticoresoftware/manticoresearch/issues/2704) Исправлена ошибка, при которой несколько стоп-слов из токенизаторов `lemmatize_xxx_all` увеличивали `hitpos` последующих токенов.
* [Issue #2708](https://github.com/manticoresoftware/manticoresearch/issues/2708) Исправлен сбой при `ALTER ... ADD COLUMN ... TEXT`.
* [Issue #2737](https://github.com/manticoresoftware/manticoresearch/issues/2737) Исправлена ошибка, при которой обновление blob-атрибута в замороженной таблице с хотя бы одним RAM-чандом вызывало ожидание последующих `SELECT` запросов до разморозки таблицы.
* [Issue #2742](https://github.com/manticoresoftware/manticoresearch/issues/2742) Исправлен пропуск кеша запросов для запросов с запакованными факторами.
* [Issue #2775](https://github.com/manticoresoftware/manticoresearch/issues/2775) Manticore теперь сообщает об ошибке при неизвестном действии вместо сбоя при запросах `_bulk`.
* [Issue #2791](https://github.com/manticoresoftware/manticoresearch/issues/2791) Исправлен возврат ID вставленного документа для HTTP эндпоинта `_bulk`.
* [Issue #2797](https://github.com/manticoresoftware/manticoresearch/issues/2797) Исправлен сбой в grouper при обработке нескольких таблиц, одна из которых пуста, а другая содержит различное число совпадающих записей.
* [Issue #2835](https://github.com/manticoresoftware/manticoresearch/issues/2835) Исправлен сбой в сложных `SELECT` запросах.
* [Issue #2872](https://github.com/manticoresoftware/manticoresearch/issues/2872) Добавлено сообщение об ошибке, если аргумент `ALL` или `ANY` в выражении `IN` не является JSON-атрибутом.
* [Issue #2882](https://github.com/manticoresoftware/manticoresearch/issues/2882) Исправлен сбой демона при обновлении MVA в больших таблицах.
* [Issue #2888](https://github.com/manticoresoftware/manticoresearch/issues/2888) Исправлен сбой при ошибке токенизации с `libstemmer`. ❤️ Спасибо, [@subnix](https://github.com/subnix) за PR.
* [Issue #2919](https://github.com/manticoresoftware/manticoresearch/issues/2919) Исправлена ошибка, из-за которой вес соединённой правой таблицы неправильно учитывался в выражениях.
* [Issue #2919](https://github.com/manticoresoftware/manticoresearch/issues/2919) Исправлена ошибка, при которой вес правой соединённой таблицы не работал в выражениях.
* [Issue #325](https://github.com/manticoresoftware/manticoresearch-buddy/issues/325) Исправлен сбой `CREATE TABLE IF NOT EXISTS ... WITH DATA`, если таблица уже существует.
* [Issue #351](https://github.com/manticoresoftware/manticoresearch-buddy/issues/351) Исправлена ошибка неопределённого ключа массива "id" при подсчёте по KNN с ID документа.
* [Issue #359](https://github.com/manticoresoftware/manticoresearch-buddy/issues/359) Исправлена работа `REPLACE INTO cluster_name:table_name`.
* [Issue #67](https://github.com/manticoresoftware/docker/issues/67) Исправлена критическая ошибка при запуске Docker-контейнера Manticore с опцией `--network=host`.

## Version 6.3.8
Released: November 22nd 2024

Версия 6.3.8 продолжает серию 6.3 и включает только исправления ошибок.

### Bug fixes

* [PR #2777](https://github.com/manticoresoftware/manticoresearch/pull/2777) Исправлен расчёт доступных потоков при ограничении параллелизма запросов настройками `threads` или `max_threads_per_query`.

## manticore-extra v1.1.20

Released: Oct 7 2024

### Bug fixes

* [Issue #64](https://github.com/manticoresoftware/executor/issues/64) Решена проблема, когда утилита `unattended-upgrades`, автоматически устанавливающая обновления пакетов в системах на базе Debian, ошибочно помечала несколько пакетов Manticore, включая `manticore-galera`, `manticore-executor` и `manticore-columnar-lib`, для удаления. Это происходило из-за того, что `dpkg` ошибочно считал виртуальный пакет `manticore-extra` избыточным. Внесены изменения, чтобы `unattended-upgrades` больше не пыталась удалять ключевые компоненты Manticore.

## Version 6.3.6
Released: August 2nd 2024

Версия 6.3.6 продолжает серию 6.3 и включает только исправления ошибок.

### Bug fixes

* [Issue #2477](https://github.com/manticoresoftware/manticoresearch/issues/2477) Исправлен сбой, введённый в версии 6.3.4, который мог возникать при работе с выражениями и распределёнными или несколькими таблицами.
* [Issue #2352](https://github.com/manticoresoftware/manticoresearch/issues/2352) Исправлен сбой демона или внутренняя ошибка при преждевременном выходе, вызванная `max_query_time` при запросах по нескольким индексам.

## Version 6.3.4
Released: July 31st 2024

Версия 6.3.4 продолжает серию 6.3 и включает только незначительные улучшения и исправления ошибок.

### Minor changes
* [Issue #1130](https://github.com/manticoresoftware/manticoresearch/issues/1130) Добавлена поддержка [DBeaver](Integration/DBeaver.md#Integration-with-DBeaver).
* [Issue #2146](https://github.com/manticoresoftware/manticoresearch/issues/2146) Улучшено экранирование разделителей в формах слов и исключениях.
* [Issue #2315](https://github.com/manticoresoftware/manticoresearch/issues/2315) Добавлены операторы сравнения строк в выражения SELECT списка.
* [Issue #2363](https://github.com/manticoresoftware/manticoresearch/issues/2363) Добавлена поддержка null-значений в bulk-запросах формата Elastic.
* [Issue #2374](https://github.com/manticoresoftware/manticoresearch/issues/2374) Добавлена поддержка mysqldump версии 9.
* [Issue #2375](https://github.com/manticoresoftware/manticoresearch/issues/2375) Улучшена обработка ошибок в HTTP JSON запросах с JSON-путём к узлу, где возникает ошибка.

### Bug fixes
* [Issue #2280](https://github.com/manticoresoftware/manticoresearch/issues/2280) Исправлено ухудшение производительности в wildcard запросах с большим числом совпадений при disk_chunks > 1.
* [Issue #2332](https://github.com/manticoresoftware/manticoresearch/issues/2332) Исправлен сбой в выражениях списка SELECT MIN или MAX для пустых массивов MVA.
* [Issue #2339](https://github.com/manticoresoftware/manticoresearch/issues/2339) Исправлена некорректная обработка запроса с бесконечным диапазоном в Kibana.
* [Issue #2342](https://github.com/manticoresoftware/manticoresearch/issues/2342) Исправлен фильтр соединения на колоннарных атрибутах из правой таблицы, когда атрибут отсутствует в списке SELECT.
* [Issue #2343](https://github.com/manticoresoftware/manticoresearch/issues/2343) Исправлено дублирование спецификатора 'static' в Manticore 6.3.2.
* [Issue #2344](https://github.com/manticoresoftware/manticoresearch/issues/2344) Исправлен LEFT JOIN, возвращающий несовпадающие записи при использовании MATCH() по правой таблице.
* [Issue #2350](https://github.com/manticoresoftware/manticoresearch/issues/2350) Исправлено сохранение дискового чанка в RT индексе с `hitless_words`.
* [Issue #2364](https://github.com/manticoresoftware/manticoresearch/issues/2364) Свойство 'aggs_node_sort' теперь можно добавлять в любом порядке среди прочих свойств.
* [Issue #2368](https://github.com/manticoresoftware/manticoresearch/issues/2368) Исправлена ошибка порядка full-text и фильтра в JSON запросе.
* [Issue #2376](https://github.com/manticoresoftware/manticoresearch/issues/2376) Исправлена ошибка, связанная с некорректным JSON ответом для длинных UTF-8 запросов.
* [Issue #2684](https://github.com/manticoresoftware/dev/issues/2684) Исправлен расчет выражений presort/prefilter, зависящих от присоединенных атрибутов.
* [Issue #301](https://github.com/manticoresoftware/manticoresearch-buddy/issues/301) Изменён метод подсчёта размера данных метрик: теперь данные читаются из файла `manticore.json` вместо проверки полного размера каталога данных.
* [Issue #302](https://github.com/manticoresoftware/manticoresearch-buddy/issues/302) Добавлена обработка запросов валидации от Vector.dev.

## Версия 6.3.2
Выпущена: 26 июня 2024 г.

Версия 6.3.2 продолжает серию 6.3 и включает в себя несколько исправлений ошибок, некоторые из которых были обнаружены после выпуска 6.3.0.

### Ломающие изменения
* ⚠️[Issue #2305](https://github.com/manticoresoftware/manticoresearch/issues/2305) Значения aggs.range обновлены и теперь должны быть числовыми.

### Исправления ошибок
* [Commit c51c](https://github.com/manticoresoftware/manticoresearch/commit/c51c7fa4bd0ec4ad8d53188b888461bff29f1417) Исправлена группировка по сохранённой проверке против слияния rset.
* [Commit 0e85](https://github.com/manticoresoftware/manticoresearch/commit/0e85b9d5682f51681750d798aef6a11cfb93d69e) Исправлен сбой демона при запросе с символами подстановки в RT индексе с использованием CRC-словаря и включенным `local_df`.
* [Issue #2200](https://github.com/manticoresoftware/manticoresearch/issues/2200) Исправлен сбой при JOIN по `count(*)` без GROUP BY.
* [Issue #2201](https://github.com/manticoresoftware/manticoresearch/issues/2201) Исправлено отсутствие предупреждения при попытке группировки по полнотекстовому полю в JOIN.
* [Issue #2230](https://github.com/manticoresoftware/manticoresearch/issues/2230) Решена проблема с добавлением атрибута через `ALTER TABLE`, не учитывавшего параметры KNN.
* [Issue #2231](https://github.com/manticoresoftware/manticoresearch/issues/2231) Исправлен сбой при удалении пакета `manticore-tools` Redhat в версии 6.3.0.
* [Issue #2242](https://github.com/manticoresoftware/manticoresearch/issues/2242) Исправлены ошибки с JOIN и несколькими операторами FACET, возвращавшими некорректные результаты.
* [Issue #2250](https://github.com/manticoresoftware/manticoresearch/issues/2250) Исправлена ошибка ALTER TABLE, возникавшая если таблица находится в кластере.
* [Issue #2252](https://github.com/manticoresoftware/manticoresearch/issues/2252) Исправлено передача исходного запроса в buddy из интерфейса SphinxQL.
* [Issue #2267](https://github.com/manticoresoftware/manticoresearch/issues/2267) Улучшено расширение подстановок wildcard в `CALL KEYWORDS` для RT индекса с дисковыми чанками.
* [Issue #271](https://github.com/manticoresoftware/manticoresearch-buddy/issues/271) Исправлено зависание неправильных запросов `/cli`.
* [Issue #274](https://github.com/manticoresoftware/manticoresearch-buddy/issues/274) Решены проблемы, когда параллельные запросы к Manticore могли зависать.
* [Issue #275](https://github.com/manticoresoftware/manticoresearch-buddy/issues/275) Исправлено зависание `drop table if exists t; create table t` через `/cli`.

### Связано с репликацией
* [Issue #2270](https://github.com/manticoresoftware/manticoresearch/issues/2270) Добавлена поддержка формата `cluster:name` в HTTP эндпоинте `/_bulk`.

## Версия 6.3.0
Выпущена: 23 мая 2024 г.

### Основные изменения
* [Issue #839](https://github.com/manticoresoftware/manticoresearch/issues/839) Реализован тип данных [float_vector](Creating_a_table/Data_types.md#Float-vector); реализован [векторный поиск](Searching/KNN.md#KNN-vector-search).
* [Issue #1673](https://github.com/manticoresoftware/manticoresearch/issues/1673) [INNER/LEFT JOIN](Searching/Joining.md) (**стадия бета**).
* [Issue #1744](https://github.com/manticoresoftware/manticoresearch/issues/1744) Реализовано автодетектирование форматов даты для полей [timestamp](Creating_a_table/Data_types.md#Timestamps).
* [Issue #1720](https://github.com/manticoresoftware/manticoresearch/pull/1720) Лицензия Manticore Search изменена с GPLv2-or-later на GPLv3-or-later.
* [Commit 7a55](https://github.com/manticoresoftware/manticoresearch/commit/7a5508424) Запуск Manticore в Windows теперь требует Docker для работы Buddy.
* [Issue #1541](https://github.com/manticoresoftware/manticoresearch/issues/1541) Добавлен полнотекстовый оператор [REGEX](Searching/Full_text_matching/Operators.md#REGEX-operator).
* [Issue #2091](https://github.com/manticoresoftware/manticoresearch/issues/2091) Поддержка Ubuntu Noble 24.04.
* [Commit 514d](https://github.com/manticoresoftware/manticoresearch/commit/514d35b497f4bdb20b6473ff963752a978a4bb8d) Переработка операций с временем для улучшения производительности и добавления новых функций даты/времени:
  - [CURDATE()](Functions/Date_and_time_functions.md#CURDATE%28%29) - Возвращает текущую дату в локальном часовом поясе
  - [QUARTER()](Functions/Date_and_time_functions.md#QUARTER%28%29) - Возвращает целочисленный квартал года из аргумента временной метки
  - [DAYNAME()](Functions/Date_and_time_functions.md#DAYNAME%28%29) - Возвращает название дня недели для данного аргумента временной метки
  - [MONTHNAME()](Functions/Date_and_time_functions.md#MONTHNAME%28%29) - Возвращает название месяца для данного аргумента временной метки
  - [DAYOFWEEK()](Functions/Date_and_time_functions.md#DAYOFWEEK%28%29) - Возвращает целочисленный индекс дня недели для данного аргумента временной метки
  - [DAYOFYEAR()](Functions/Date_and_time_functions.md#DAYOFYEAR%28%29) - Возвращает целочисленный день года для данного аргумента временной метки
  - [YEARWEEK()](Functions/Date_and_time_functions.md#YEARWEEK%28%29) - Возвращает целочисленный год и код дня первого дня текущей недели для данного аргумента временной метки
  - [DATEDIFF()](Functions/Date_and_time_functions.md#DATEDIFF%28%29) - Возвращает количество дней между двумя заданными временными метками
  - [DATE()](Functions/Date_and_time_functions.md#DATE%28%29) - Форматирует часть даты из аргумента временной метки
  - [TIME()](Functions/Date_and_time_functions.md#TIME%28%29) - Форматирует часть времени из аргумента временной метки
  - [timezone](Server_settings/Searchd.md#timezone) - Часовой пояс, используемый функциями, связанными с датой/временем.
* [Commit 30e7](https://github.com/manticoresoftware/manticoresearch/commit/30e789d9657f9b3093c280c6e5b0e46a021f6dc7) Добавлены агрегаты [range](Searching/Faceted_search.md#Facet-over-set-of-ranges), [histogram](Searching/Faceted_search.md#Facet-over-histogram-values), [date_range](Searching/Faceted_search.md#Facet-over-set-of-date-ranges) и [date_histogram](Searching/Faceted_search.md#Facet-over-histogram-date-values) в HTTP-интерфейс и аналогичные выражения в SQL.

### Minor changes
* [Issue #1285](https://github.com/manticoresoftware/manticoresearch/issues/1285) Поддержка Filebeat версий 8.10 - 8.11.
* [Issue #1771](https://github.com/manticoresoftware/manticoresearch/issues/1771) [ALTER TABLE ... type='distributed'](Updating_table_schema_and_settings.md#Changing-a-distributed-table).
* [Issue #1788](https://github.com/manticoresoftware/manticoresearch/issues/1788) Добавлена возможность копирования таблиц с помощью SQL оператора [CREATE TABLE ... LIKE ... WITH DATA](Creating_a_table/Local_tables/Real-time_table.md#CREATE-TABLE-LIKE:).
* [Issue #2072](https://github.com/manticoresoftware/manticoresearch/issues/2072) Оптимизирован [алгоритм уплотнения таблиц](Securing_and_compacting_a_table/Compacting_a_table.md#Compacting-a-Table): Ранее и ручной процесс [OPTIMIZE](Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE), и автоматический [auto_optimize](Server_settings/Searchd.md#auto_optimize) сначала сливал куски, чтобы количество не превышало лимит, а затем очищал удалённые документы из всех остальных кусков, содержащих удалённые документы. Этот подход иногда был слишком ресурсоёмким и был отключён. Сейчас слияние кусков происходит только согласно настройке [progressive_merge](Server_settings/Common.md#progressive_merge). Однако куски с большим количеством удалённых документов имеют более высокую вероятность слияния.
* [Commit ce6c](https://github.com/manticoresoftware/columnar/commit/ce6c5d3381edcdc411a00a5923d13d56dd9b3010) Добавлена защита от загрузки вторичного индекса более новой версии.
* [Issue #1417](https://github.com/manticoresoftware/manticoresearch/issues/1417) Частичная замена с помощью [REPLACE INTO ... SET](Data_creation_and_modification/Updating_documents/REPLACE.md#SQL-REPLACE).
* [Commit 7c16](https://github.com/manticoresoftware/manticoresearch/commit/7c167b934ce416d3dfbd658744a231e638b14d9a) Обновлены размеры буферов слияния по умолчанию: `.spa` (скалярные атрибуты): 256KB -> 8MB; `.spb` (blob атрибуты): 256KB -> 8MB; `.spc` (колоночные атрибуты): 1MB, без изменений; `.spds` (docstore): 256KB -> 8MB; `.spidx` (вторичные индексы): буфер 256KB -> лимит памяти 128MB; `.spi` (словарь): 256KB -> 16MB; `.spd` (doclists): 8MB, без изменений; `.spp` (hitlists): 8MB, без изменений; `.spe` (skiplists): 256KB -> 8MB.
* [Issue #1859](https://github.com/manticoresoftware/manticoresearch/issues/1859) Добавлена [композитная агрегация](Searching/Grouping.md#GROUP-BY-multiple-fields-at-once) через JSON.
* [Commit 216b](https://github.com/manticoresoftware/manticoresearch-buddy/commit/216b824) Отключён PCRE.JIT из-за проблем с некоторыми регулярными выражениями и отсутствием значительной выгоды по времени.
* [Commit 55cd](https://github.com/manticoresoftware/manticoresearch/commit/55cdce32) Добавлена поддержка vanilla Galera v.3 (api v25) (`libgalera_smm.so` из MySQL 5.x).
* [Commit 86f9](https://github.com/manticoresoftware/manticoresearch-buddy/commit/86f98ab) Изменён суффикс метрики с `_rate` на `_rps`.
* [Commit c0c1](https://github.com/manticoresoftware/manticoresearch-helm/commit/c0c1cff780031412ef57cefab987dfd16613368e) Улучшена документация о поддержке HA балансировщика.
* [Commit d1d2](https://github.com/manticoresoftware/manticoresearch/commit/d1d2ca18857c68106e89743756260e73285be739) В сообщениях об ошибках изменено `index` на `table`; исправлена корректировка сообщений об ошибках парсера bison.
* [Commit fd26](https://github.com/manticoresoftware/manticoresearch/commit/fd26671b) Поддержка `manticore.tbl` как имени таблицы.
* [Issue #1105](https://github.com/manticoresoftware/manticoresearch/issues/1105) Поддержка запуска индексатора через systemd ([документация](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Schedule-indexer-via-systemd)). ❤️ Спасибо, [@subnix](https://github.com/subnix), за PR.
* [Issue #1294](https://github.com/manticoresoftware/manticoresearch/issues/1294) Поддержка вторичных индексов в GEODIST().
* [Issue #1394](https://github.com/manticoresoftware/manticoresearch/issues/1394) Упрощён [SHOW THREADS](Node_info_and_management/SHOW_THREADS.md#SHOW-THREADS).
* [Issue #1424](https://github.com/manticoresoftware/manticoresearch/issues/1424) Добавлена поддержка значений по умолчанию (`agent_connect_timeout` и `agent_query_timeout`) для оператора `create distributed table`.
* [Issue #1442](https://github.com/manticoresoftware/manticoresearch/issues/1442) Добавлена опция запроса поиска [expansion_limit](Searching/Options.md#expansion_limit), которая переопределяет `searchd.expansion_limit`.
* [Issue #1448](https://github.com/manticoresoftware/manticoresearch/issues/1448) Реализована команда [ALTER TABLE](Updating_table_schema_and_settings.md#Updating-table-schema-in-RT-mode) для конвертации int->bigint.
* [Issue #1456](https://github.com/manticoresoftware/manticoresearch/issues/1456) Метаданные в ответе MySQL.
* [Issue #1494](https://github.com/manticoresoftware/manticoresearch/issues/1494) [SHOW VERSION](Node_info_and_management/SHOW_VERSION.md#SHOW-VERSION).
* [Issue #1582](https://github.com/manticoresoftware/manticoresearch/issues/1582) Поддержка [удаления документов по массиву id](Data_creation_and_modification/Deleting_documents.md?client=JSON#Deleting-documents) через JSON.
* [Issue #1589](https://github.com/manticoresoftware/manticoresearch/issues/1589) Улучшена ошибка "unsupported value type".
* [Issue #1634](https://github.com/manticoresoftware/manticoresearch/issues/1634) Добавлена версия Buddy в `SHOW STATUS`.
* [Issue #1641](https://github.com/manticoresoftware/manticoresearch/issues/1641) Оптимизация запросов поиска в случае отсутствия документов по ключевому слову.
* [Issue #1712](https://github.com/manticoresoftware/manticoresearch/issues/1712) Добавлено преобразование в булевый атрибут из строкового значения "true" и "false" при отправке данных.
* [Issue #1713](https://github.com/manticoresoftware/manticoresearch/issues/1713) Добавлена таблица и опция searchd [access_dict](Server_settings/Searchd.md#access_dict).
* [Issue #1767](https://github.com/manticoresoftware/manticoresearch/issues/1767) Добавлены новые опции: [expansion_merge_threshold_docs](Server_settings/Searchd.md#expansion_merge_threshold_docs) и [expansion_merge_threshold_hits](Server_settings/Searchd.md#expansion_merge_threshold_hits) в разделе конфигурации searchd; сделан настраиваемым порог слияния маленьких терминов расширенных терминов.
* [Issue #1768](https://github.com/manticoresoftware/manticoresearch/issues/1768) Добавлен показ времени последней команды в `@@system.sessions`.
* [Issue #1806](https://github.com/manticoresoftware/manticoresearch/issues/1806) Обновлен протокол Buddy до версии 2.
* [Issue #1810](https://github.com/manticoresoftware/manticoresearch/issues/1810) Реализованы дополнительные форматы запросов для эндпоинта `/sql` для облегчения интеграции с библиотеками.
* [Issue #1825](https://github.com/manticoresoftware/manticoresearch/pull/1825) Добавлен раздел Информация в SHOW BUDDY PLUGINS.
* [Issue #1837](https://github.com/manticoresoftware/manticoresearch/issues/1837) Улучшено потребление памяти в `CALL PQ` при больших пакетах.
* [Issue #1853](https://github.com/manticoresoftware/manticoresearch/pull/1853) Переход на компилятор Clang 16 вместо Clang 15.
* [Issue #1857](https://github.com/manticoresoftware/manticoresearch/issues/1857) Добавлено строковое сравнение. ❤️ Спасибо, [@etcd](https://github.com/etcd) за PR.
* [Issue #1915](https://github.com/manticoresoftware/manticoresearch/issues/1915) Добавлена поддержка объединённых сохранённых полей.
* [Issue #1937](https://github.com/manticoresoftware/manticoresearch/issues/1937) В лог запросов добавлен хост и порт клиента.
* [Issue #1981](https://github.com/manticoresoftware/manticoresearch/issues/1981) Исправлена неверная ошибка.
* [Issue #1983](https://github.com/manticoresoftware/manticoresearch/issues/1983) Введена поддержка уровней подробности для [плана запроса через JSON](Node_info_and_management/Profiling/Query_plan.md?client=JSON#Query-plan).
* [Issue #2010](https://github.com/manticoresoftware/manticoresearch/issues/2010) Отключено логирование запросов от Buddy, если не установлен `log_level=debug`.
* [Issue #2035](https://github.com/manticoresoftware/manticoresearch/issues/2035) Поддержка Linux Mint 21.3.
* [Issue #2056](https://github.com/manticoresoftware/manticoresearch/issues/2056) Manticore не мог быть собран с Mysql 8.3+.
* [Issue #2112](https://github.com/manticoresoftware/manticoresearch/issues/2112) Команда `DEBUG DEDUP` для чанков таблиц реального времени, которые могут содержать дубликаты после присоединения простой таблицы с дубликатами.
* [Issue #212](https://github.com/manticoresoftware/manticoresearch-buddy/issues/212) Добавлено время в [SHOW QUERIES](Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES).
* [Issue #218](https://github.com/manticoresoftware/manticoresearch-buddy/issues/218) Обработка колонки `@timestamp` как временной метки.
* [Issue #252](https://github.com/manticoresoftware/manticoresearch-buddy/issues/250) Реализована логика включения/отключения плагинов buddy.
* [Issue #254](https://github.com/manticoresoftware/manticoresearch-buddy/issues/254) Обновлен composer до более свежей версии с исправленными последними CVE.
* [Issue #340](https://github.com/manticoresoftware/manticoresearch/issues/340) Небольшая оптимизация systemd юнита Manticore, связанная с `RuntimeDirectory`.
* [Issue #51](https://github.com/manticoresoftware/executor/issues/51) Добавлена поддержка rdkafka и обновление до PHP 8.3.3.
* [Issue #527](https://github.com/manticoresoftware/manticoresearch/issues/527) Поддержка [присоединения](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Attaching_one_table_to_another.md#Attaching-one-table-to-another) таблицы RT. Новая команда [ALTER TABLE ... RENAME](Updating_table_schema_and_settings.md#Renaming-a-real-time-table).

### Серьёзные изменения и устаревание
* ⚠️[Issue #1436](https://github.com/manticoresoftware/manticoresearch/issues/1436) Исправлена проблема расчёта IDF. `local_df` теперь используется по умолчанию. Улучшен мастер-агент протокол поиска (версия обновлена). Если вы запускаете Manticore Search в распределённой среде с несколькими инстансами, сначала обновите агенты, затем мастеров.
* ⚠️[Issue #1572](https://github.com/manticoresoftware/manticoresearch/issues/1572) Добавлена репликация распределённых таблиц и обновлён протокол репликации. Если вы запускаете кластер с репликацией, вам нужно:
  - Сначала аккуратно остановите все ваши узлы
  - Затем запустите узел, который был остановлен последним, с параметром `--new-cluster`, используя инструмент `manticore_new_cluster` в Linux.
  - Подробнее о [перезапуске кластера](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster) читайте для получения дополнительной информации.
* ⚠️[Issue #1763](https://github.com/manticoresoftware/manticoresearch/pull/1763) Псевдонимы HTTP API endpoint `/json/*` устарели.
* ⚠️[Issue #1982](https://github.com/manticoresoftware/manticoresearch/issues/1982) Изменён [profile](Node_info_and_management/Profiling/Query_profile.md#Query-profile) на [plan](Node_info_and_management/Profiling/Query_plan.md#Query-plan) в JSON, добавлено профилирование запросов для JSON.
* ⚠️[Commit e235](https://github.com/manticoresoftware/manticoresearch-backup/commit/e23585a) manticore-backup больше не создаёт резервные копии `plugin_dir`.
* ⚠️[Issue #171](https://github.com/manticoresoftware/manticoresearch-buddy/issues/155) Buddy мигрировал на Swoole для улучшения производительности и стабильности. При переходе на новую версию убедитесь, что все пакеты Manticore обновлены.
* ⚠️[Issue #196](https://github.com/manticoresoftware/manticoresearch-buddy/pull/205) Объединили все основные плагины в Buddy и изменили основную логику.
* ⚠️[Issue #2107](https://github.com/manticoresoftware/manticoresearch/issues/2107) Обработка ID документов как чисел в ответах `/search`.
* ⚠️[Issue #38](https://github.com/manticoresoftware/executor/issues/38) Добавлен Swoole, отключён ZTS и удалено расширение parallel.
* ⚠️[Issue #1929](https://github.com/manticoresoftware/manticoresearch/issues/1929) Переопределение в `charset_table` не работало в некоторых случаях.

### Изменения, связанные с репликацией
* [Commit 3376](https://github.com/manticoresoftware/manticoresearch/commit/3376351f73db3ff0cb6cb708a304a7bd83e69cfb) Исправлена ошибка репликации при SST больших файлов.
* [Commit 6d36](https://github.com/manticoresoftware/manticoresearch/commit/6d36c68fb7ce3cb274b467aac435788905084952) Добавлен механизм повторных попыток для команд репликации; исправлен сбой присоединения репликации в условиях загруженной сети с потерей пакетов.
* [Commit 842e](https://github.com/manticoresoftware/manticoresearch/commit/842e27e71790833ab4125d49103d2933d5df60c3) Сообщение FATAL в репликации изменено на сообщение WARNING.
* [Commit 8c32](https://github.com/manticoresoftware/manticoresearch/commit/8c32bce7db39d8ed3b1a06aaa1d2c3ec372084f0) Исправлен расчёт `gcache.page_size` для репликационных кластеров без таблиц или с пустыми таблицами; также исправлено сохранение и загрузка опций Galera.
* [Commit a2af](https://github.com/manticoresoftware/manticoresearch/commit/a2af06ca36abcbf893e43c8803853130b397c54b) Добавлена возможность пропуска команды обновления узлов репликации на узле, который присоединяется к кластеру.
* [Commit c054](https://github.com/manticoresoftware/manticoresearch/commit/c0541e6892f5116b1f957b40060ba8ecd01a5afb) Исправлен дедлок при репликации во время обновления blob-атрибутов и замены документов.
* [Commit e80d](https://github.com/manticoresoftware/manticoresearch/commit/e80d505b963999016033613c40c1cf3151d648e7) Добавлены параметры конфигурации searchd [replication_connect_timeout](Server_settings/Searchd.md#replication_connect_timeout), [replication_query_timeout](Server_settings/Searchd.md#replication_query_timeout), [replication_retry_delay](Server_settings/Searchd.md#replication_retry_delay), [replication_retry_count](Server_settings/Searchd.md#replication_retry_count) для контроля сети во время репликации, аналогично `searchd.agent_*`, но с другими значениями по умолчанию.
* [Issue #1356](https://github.com/manticoresoftware/manticoresearch/issues/1356) Исправлен повторный запрос узлов репликации после пропуска некоторых узлов и сбоя разрешения имен этих узлов.
* [Issue #1445](https://github.com/manticoresoftware/manticoresearch/issues/1445) Исправлен уровень подробности логов репликации при `show variables`.
* [Issue #1482](https://github.com/manticoresoftware/manticoresearch/issues/1482) Исправлена проблема репликации для узла-присоединителя, подключающегося к кластеру на поде, перезапущенном в Kubernetes.
* [Issue #1962](https://github.com/manticoresoftware/manticoresearch/issues/1962) Исправлено долгое ожидание репликации для ALTER в пустом кластере с неверным именем узла.

### Исправления ошибок
* [Commit 8a48](https://github.com/manticoresoftware/manticoresearch/commit/8a486c7ba1a293879cdecc41fb52eb7ab0cbf832) Исправлена очистка неиспользуемых совпадений в `count distinct`, которая могла вызывать аварийное завершение.
* [Issue #1569](https://github.com/manticoresoftware/manticoresearch/issues/1569) Бинарный лог теперь записывается с гранулярностью транзакций.
* [Issue #2089](https://github.com/manticoresoftware/manticoresearch/issues/2089) Исправлена ошибка, связанная с 64-битными ID, которая могла приводить к ошибке "Malformed packet" при вставке через MySQL, вызывая [повреждение таблиц и дублирование ID](https://github.com/manticoresoftware/manticoresearch/issues/2090).
* [Issue #2160](https://github.com/manticoresoftware/manticoresearch/issues/2160) Исправлена вставка дат, которые воспринимались как в UTC, а не в локальном часовом поясе.
* [Issue #2177](https://github.com/manticoresoftware/manticoresearch/issues/2177) Исправлен сбой при выполнении поиска в таблице real-time с непустым `index_token_filter`.
* [Issue #2209](https://github.com/manticoresoftware/manticoresearch/issues/2209) Изменена фильтрация дубликатов в RT columnar storage для устранения сбоев и неправильных результатов запросов.
* [Commit 001d](https://github.com/manticoresoftware/manticoresearch/commit/001d82904e4b887c49387308291aeefb07c0fbc4) Исправлен html stripper, вызывавший повреждение памяти после обработки объединённого поля.
* [Commit 00eb](https://github.com/manticoresoftware/manticoresearch/commit/00eb2ebf) Избежали перемотки потока после flush, чтобы предотвратить проблемы с передачей данных с mysqldump.
* [Commit 0553](https://github.com/manticoresoftware/manticoresearch/commit/05534114acf08c70ffcc41a23d558dec09a34373) Не ждать завершения preread, если он не был запущен.
* [Commit 055a](https://github.com/manticoresoftware/manticoresearch/commit/055aac09f877570cd29b6594cb283182eba7f73b) Исправлена большая строка вывода Buddy, разбитая на несколько строк в логе searchd.
* [Commit 0a88](https://github.com/manticoresoftware/manticoresearch/commit/0a88056307f82c50fcdc458ad6dbc307a00e3f92) Перемещено предупреждение интерфейса MySQL о сбое заголовка `debugv` уровня подробности.
* [Commit 150a](https://github.com/manticoresoftware/manticoresearch/commit/150a20286b7d98d24b844ae577d7fda4a444798d) Исправлено условие гонки при операциях управления несколькими кластерами; запрещено создавать несколько кластеров с одинаковым именем или путем.
* [Commit 2e40](https://github.com/manticoresoftware/manticoresearch/commit/2e405c1c9ebac6f46206e3df17ac1c19129c9aa3) Исправлен неявный cutoff в полнотекстовых запросах; разделен MatchExtended на шаблонную часть D.
* [Commit 75f5](https://github.com/manticoresoftware/manticoresearch/commit/75f51421542a2a0658f9e2f9f31048c8c5224479) Исправлено несоответствие `index_exact_words` между индексированием и загрузкой таблицы в демон.
* [Commit 7643](https://github.com/manticoresoftware/manticoresearch/commit/7643976bea142e08535d701000eff0d09a99fda2) Исправлено пропущенное сообщение об ошибке при удалении некорректного кластера.
* [Commit 7a03](https://github.com/manticoresoftware/manticoresearch/commit/7a034ac990d5ec9344320c05a6a2315583b9b858) Исправлено объединение CBO с очередью; исправлен CBO и псевдо-шардинг RT.
* [Commit 7b4e](https://github.com/manticoresoftware/manticoresearch/commit/7b4ea95e) При запуске без библиотеки вторичного индекса (SI) и параметров в конфигурации выдавалося вводящее в заблуждение предупреждение 'WARNING: secondary_indexes set but failed to initialize secondary library'.
* [Commit 8496](https://github.com/manticoresoftware/manticoresearch/commit/8496317f099fade8d2db463fb88483e7fcdfe85b) исправлена сортировка попаданий в кворуме.
* [Commit 8973](https://github.com/manticoresoftware/manticoresearch-buddy/commit/8973ad3) Исправлена проблема с опциями в верхнем регистре в плагине ModifyTable.
* [Commit 9935](https://github.com/manticoresoftware/manticoresearch-buddy/commit/99350d4c184379b19f3efa9751d9b5be5e6edfb5) Исправлено восстановление из дампа с пустыми json значениями (представленными как NULL).
* [Commit a28f](https://github.com/manticoresoftware/manticoresearch/commit/a28f6e087b3d957e48816070be17d0823904936a) Исправлено превышение времени ожидания SST на ноде joiner при получении SST с использованием pcon.
* [Commit b5a5](https://github.com/manticoresoftware/manticoresearch/commit/b5a55cc26debcddcda5d9caef8366a707e0dd763) Исправлен сбой при выборе атрибута строкового псевдонима.
* [Commit c556](https://github.com/manticoresoftware/manticoresearch/commit/c556e7ed8b6da8eac8ef094650df5f0a8977916b) Добавлено преобразование запроса терма в `=term` полнотекстового запроса с полем `morphology_skip_fields`.
* [Commit cdc3](https://github.com/manticoresoftware/manticoresearch/commit/cdc38085145cd1025b96fcac2a46188a0425d9e1) Добавлен отсутствующий ключ конфигурации (skiplist_cache_size).
* [Commit cf6e](https://github.com/manticoresoftware/manticoresearch/commit/cf6ec3bfdc3f0990fb31ff53822b2331a6667727) Исправлен сбой ранжировщика выражений при большом сложном запросе.
* [Commit e513](https://github.com/manticoresoftware/manticoresearch/commit/e5139ba75eef7223df375ae3cc3f138fa7d925f1) Исправлен полнотекстовый CBO против недействительных подсказок индекса.
* [Commit eb05](https://github.com/manticoresoftware/manticoresearch/commit/eb0514b3a6f57f278252b0724eebd112ec116e0f) Прервано предварительное чтение при остановке для ускорения завершения работы.
* [Commit f945](https://github.com/manticoresoftware/manticoresearch/commit/f94555a293cfa51004ff81135b8bd8500b955115) Изменен расчет стека для полнотекстовых запросов, чтобы избежать сбоя при сложном запросе.
* [Issue #1262](https://github.com/manticoresoftware/manticoresearch/issues/1262) Исправлен сбой индексатора при индексации SQL-источника с несколькими столбцами с одинаковым именем.
* [Issue #1273](https://github.com/manticoresoftware/manticoresearch/issues/1273) Возвращать 0 вместо <empty> для несуществующих системных переменных.
* [Issue #1289](https://github.com/manticoresoftware/manticoresearch/issues/1289) Исправлена ошибка indextool при проверке внешних файлов таблицы RT.
* [Issue #1335](https://github.com/manticoresoftware/manticoresearch/issues/1335) Исправлена ошибка разбора запроса из-за многословной формы внутри фразы.
* [Issue #1364](https://github.com/manticoresoftware/manticoresearch/issues/1364) Добавлено воспроизведение пустых файлов binlog со старыми версиями binlog.
* [Issue #1365](https://github.com/manticoresoftware/manticoresearch/issues/1365) Исправлено удаление последнего пустого файла binlog.
* [Issue #1372](https://github.com/manticoresoftware/manticoresearch/issues/1372) Исправлены некорректные относительные пути (преобразованные в абсолютные от каталога запуска демона) после изменений в `data_dir`, влияющих на текущий рабочий каталог при запуске демона.
* [Issue #1393](https://github.com/manticoresoftware/manticoresearch/issues/1393) Наибольшее замедление времени в hn_small: получение/кэширование информации о CPU при запуске демона.
* [Issue #1395](https://github.com/manticoresoftware/manticoresearch/issues/1395) Исправлено предупреждение о отсутствующем внешнем файле при загрузке индекса.
* [Issue #1402](https://github.com/manticoresoftware/manticoresearch/issues/1402) Исправлен сбой в глобальных группировщиках при освобождении атрибутов указателей данных.
* [Issue #1403](https://github.com/manticoresoftware/manticoresearch/issues/1403) _ADDITIONAL_SEARCHD_PARAMS не работает.
* [Issue #1427](https://github.com/manticoresoftware/manticoresearch/issues/1427) Исправлено, что параметр `agent_query_timeout` для каждой таблицы заменялся на значение по умолчанию `agent_query_timeout`.
* [Issue #1444](https://github.com/manticoresoftware/manticoresearch/issues/1444) Исправлен сбой в группировщике и ранжировщике при использовании `packedfactors()` с несколькими значениями на совпадение.
* [Issue #1458](https://github.com/manticoresoftware/manticoresearch/issues/1458) Manticore аварийно завершает работу при частых обновлениях индекса.
* [Issue #1481](https://github.com/manticoresoftware/manticoresearch/issues/1481) Исправлен сбой при очистке разобранного запроса после ошибки разбора.
* [Issue #1484](https://github.com/manticoresoftware/manticoresearch/issues/1484) Исправлена маршрутизация HTTP JSON запросов к buddy.
* [Issue #1499](https://github.com/manticoresoftware/manticoresearch/issues/1499) Значение корня JSON атрибута не могло быть массивом. Исправлено.
* [Issue #1507](https://github.com/manticoresoftware/manticoresearch/issues/1507) Исправлен сбой при пересоздании таблицы внутри транзакции.
* [Issue #1515](https://github.com/manticoresoftware/manticoresearch/issues/1515) Исправлено расширение коротких форм русских лемм.
* [Issue #1579](https://github.com/manticoresoftware/manticoresearch/issues/1579) Исправлено использование JSON и STRING атрибутов в выражении [date_format](Functions/Date_and_time_functions.md#DATE_FORMAT()).
* [Issue #1580](https://github.com/manticoresoftware/manticoresearch/issues/1580) Исправлен группировщик для множественных псевдонимов JSON полей.
* [Issue #1594](https://github.com/manticoresoftware/manticoresearch/issues/1594) Неправильный total_related в dev: исправлена неявная отсечка по сравнению с лимитом; добавлено улучшенное обнаружение полного сканирования в JSON-запросах.
* [Issue #1603](https://github.com/manticoresoftware/manticoresearch/issues/1603) Исправлено использование JSON и STRING атрибутов во всех выражениях даты.
* [Issue #1609](https://github.com/manticoresoftware/manticoresearch/issues/1609) Сбой при использовании LEVENSHTEIN().
* [Issue #1612](https://github.com/manticoresoftware/manticoresearch/issues/1612) Исправлена порча памяти после ошибки парсинга поискового запроса с подсветкой.
* [Issue #1614](https://github.com/manticoresoftware/manticoresearch/issues/1614) Отключено расширение подстановочных знаков для терминов короче `min_prefix_len` / `min_infix_len`.
* [Issue #1617](https://github.com/manticoresoftware/manticoresearch/issues/1617) Изменено поведение: не логировать ошибку, если запрос успешно обработан Buddy.
* [Issue #1635](https://github.com/manticoresoftware/manticoresearch/issues/1635) Исправлен total в метаинформации поискового запроса для запросов с установленным лимитом.
* [Issue #1640](https://github.com/manticoresoftware/manticoresearch/issues/1640) Невозможно использовать таблицу с заглавными буквами через JSON в plain режиме.
* [Issue #1643](https://github.com/manticoresoftware/manticoresearch/issues/1643) Установлено значение по умолчанию для `SPH_EXTNODE_STACK_SIZE`.
* [Issue #1646](https://github.com/manticoresoftware/manticoresearch/issues/1646) Исправлен лог SphinxQL для отрицательного фильтра с ALL/ANY на MVA атрибуте.
* [Issue #1660](https://github.com/manticoresoftware/manticoresearch/issues/1660) Исправлено применение списков удаления docid из других индексов. ❤️ Спасибо, [@raxoft](https://github.com/raxoft) за PR.
* [Issue #1668](https://github.com/manticoresoftware/manticoresearch/issues/1668) Исправлены пропущенные совпадения из-за преждевременного выхода при полном сканировании сырого индекса (без итераторов индексов); удалена отсечка из plain row итератора.
* [Issue #1671](https://github.com/manticoresoftware/manticoresearch/issues/1671) Исправлена ошибка `FACET` при запросе распределённой таблицы с агентом и локальными таблицами.
* [Issue #1690](https://github.com/manticoresoftware/manticoresearch/issues/1690) Исправлен сбой при оценке гистограммы для больших значений.
* [Issue #1692](https://github.com/manticoresoftware/manticoresearch/issues/1692) Сбой при alter table tbl add column col uint.
* [Issue #1710](https://github.com/manticoresoftware/manticoresearch/issues/1710) Пустой результат для условия `WHERE json.array IN (<value>)`.
* [Issue #172](https://github.com/manticoresoftware/manticoresearch-buddy/issues/172) Исправлена проблема с TableFormatter при отправке запроса к `/cli`.
* [Issue #1742](https://github.com/manticoresoftware/manticoresearch/issues/1742) `CREATE TABLE` не выдавал ошибку при отсутствии файла wordforms.
* [Issue #1762](https://github.com/manticoresoftware/manticoresearch/issues/1762) Порядок атрибутов в RT таблицах теперь соответствует порядку в конфигурации.
* [Issue #1765](https://github.com/manticoresoftware/manticoresearch/issues/1765) HTTP bool запрос с условием 'should' возвращал некорректные результаты.
* [Issue #1769](https://github.com/manticoresoftware/manticoresearch/issues/1769) Сортировка по строковым атрибутам не работает с `SPH_SORT_ATTR_DESC` и `SPH_SORT_ATTR_ASC`.
* [Issue #177](https://github.com/manticoresoftware/manticoresearch-buddy/issues/177) Отключён HTTP заголовок `Expect: 100-continue` для curl запросов к Buddy.
* [Issue #1791](https://github.com/manticoresoftware/manticoresearch/issues/1791) Сбой из-за алиаса в GROUP BY.
* [Issue #1792](https://github.com/manticoresoftware/manticoresearch/issues/1792) SQL мета-сводка показывает неправильное время в Windows.
* [Issue #1794](https://github.com/manticoresoftware/manticoresearch/issues/1794) Исправлен спад производительности с однотерминовыми JSON запросами.
* [Issue #1798](https://github.com/manticoresoftware/manticoresearch/issues/1798) Несовместимые фильтры не вызывали ошибку на `/search`.
* [Issue #1802](https://github.com/manticoresoftware/manticoresearch/issues/1802) Исправлены операции `ALTER CLUSTER ADD` и `JOIN CLUSTER` для ожидания друг друга, предотвращая гонку, когда `ALTER` добавляет таблицу в кластер, а донор посылает таблицы узлу присоединения.
* [Issue #1811](https://github.com/manticoresoftware/manticoresearch/issues/1811) Некорректная обработка запросов `/pq/{table}/*`.
* [Issue #1816](https://github.com/manticoresoftware/manticoresearch/issues/1816) `UNFREEZE` не работал в некоторых случаях.
* [Issue #183](https://github.com/manticoresoftware/manticoresearch-buddy/pull/183) Исправлена проблема с восстановлением MVA в некоторых случаях.
* [Issue #1849](https://github.com/manticoresoftware/manticoresearch/issues/1849) Исправлен сбой indextool при завершении работы если используется с MCL.
* [Issue #1866](https://github.com/manticoresoftware/manticoresearch/issues/1866) Исправлено ненужное декодирование URL для запросов `/cli_json`.
* [Issue #1872](https://github.com/manticoresoftware/manticoresearch/issues/1872) Изменена логика установки plugin_dir при запуске демона.
* [Issue #1874](https://github.com/manticoresoftware/manticoresearch/issues/1874) Ошибки при alter table ... exceptions.
* [Issue #1891](https://github.com/manticoresoftware/manticoresearch/issues/1891#issuecomment-2051941200) Manticore аварийно завершает работу с `signal 11` при вставке данных.
* [Issue #1920](https://github.com/manticoresoftware/manticoresearch/issues/1920) Снижено ограничение пропускной способности для [low_priority](Searching/Options.md#low_priority).
* [Issue #1924](https://github.com/manticoresoftware/manticoresearch/issues/1924) Ошибка при Mysqldump + восстановлении mysql.
* [Issue #1951](https://github.com/manticoresoftware/manticoresearch/issues/1951) Исправлено некорректное создание распределённой таблицы в случае отсутствия локальной таблицы или некорректного описания агента; теперь возвращается сообщение об ошибке.
* [Issue #1972](https://github.com/manticoresoftware/manticoresearch/issues/1972) Реализован счётчик `FREEZE` для избежания проблем с freeze/unfreeze.
* [Issue #1980](https://github.com/manticoresoftware/manticoresearch/issues/1980) Учёт тайм-аута запроса в OR узлах. Ранее `max_query_time` мог не работать в некоторых случаях.
* [Issue #1986](https://github.com/manticoresoftware/manticoresearch/issues/1986) Не удалось переименовать новый в текущий [manticore.json].
* [Issue #1988](https://github.com/manticoresoftware/manticoresearch/issues/1988) Полнотекстовый запрос мог игнорировать подсказку `SecondaryIndex` CBO.
* [Issue #1990](https://github.com/manticoresoftware/manticoresearch/issues/1990) Исправлен `expansion_limit` для среза итогового набора результатов при вызове ключевых слов из нескольких дисковых или RAM-чанков.
* [Issue #1994](https://github.com/manticoresoftware/manticoresearch/issues/1994) Неправильные внешние файлы.
* [Issue #2021](https://github.com/manticoresoftware/manticoresearch/issues/2021) Несколько процессов manticore-executor могли остаться работающими после остановки Manticore.
* [Issue #2029](https://github.com/manticoresoftware/manticoresearch/issues/2029) Сбой при использовании Levenshtein Distance.
* [Issue #2037](https://github.com/manticoresoftware/manticoresearch/issues/2037) Ошибка после нескольких операций max operator на пустом индексе.
* [Issue #2052](https://github.com/manticoresoftware/manticoresearch/issues/2052) Сбой при multi-group с JSON.field.
* [Issue #2067](https://github.com/manticoresoftware/manticoresearch/issues/2067) Manticore падал при некорректном запросе к _update.
* [Issue #2069](https://github.com/manticoresoftware/manticoresearch/issues/2069) Исправлена проблема со сравнениями строковых фильтров для закрытых диапазонов в JSON интерфейсе.
* [Issue #2082](https://github.com/manticoresoftware/manticoresearch/issues/2082) `alter` не срабатывал, если путь data_dir находился на символической ссылке.
* [Issue #2102](https://github.com/manticoresoftware/manticoresearch/pull/2102) Улучшена специальная обработка SELECT-запросов в mysqldump для обеспечения совместимости итоговых INSERT запросов с Manticore.
* [Issue #2103](https://github.com/manticoresoftware/manticoresearch/issues/2103) Тайские символы были в неправильных наборах символов.
* [Issue #2124](https://github.com/manticoresoftware/manticoresearch/issues/2124) Сбой при использовании SQL с зарезервированным словом.
* [Issue #2154](https://github.com/manticoresoftware/manticoresearch/issues/2154) Таблицы с wordforms не могли быть импортированы.
* [Issue #2176](https://github.com/manticoresoftware/manticoresearch/issues/2176) Исправлен сбой, возникавший при установке параметра engine в 'columnar' и добавлении дублирующихся ID через JSON.
* [Issue #223](https://github.com/manticoresoftware/manticoresearch-buddy/issues/223) Правильное сообщение об ошибке при попытке вставить документ без схемы и без имён колонок.
* [Issue #239](https://github.com/manticoresoftware/manticoresearch-buddy/issues/239) Авто-схема для многострочной вставки могла не работать.
* [Issue #399](https://github.com/manticoresoftware/manticoresearch/issues/399) Добавлено сообщение об ошибке при индексировании, если атрибут id объявлен на источнике данных.
* [Issue #59](https://github.com/manticoresoftware/manticoresearch-helm/issues/59) Сбой кластера Manticore.
* [Issue #68](https://github.com/manticoresoftware/manticoresearch-helm/issues/68) optimize.php падал, если была таблица percolate.
* [Issue #77](https://github.com/manticoresoftware/manticoresearch-helm/issues/77) Исправлены ошибки при развертывании в Kubernetes.
* [Issue #274](https://github.com/manticoresoftware/manticoresearch-buddy/issues/274) Исправлена некорректная обработка параллельных запросов к Buddy

### Связано с manticore-backup
* [Issue #97](https://github.com/manticoresoftware/manticoresearch-backup/pull/97) Установлен VIP HTTP порт по умолчанию, если доступен.
Различные улучшения: улучшена проверка версий и потоковая декомпрессия ZSTD; добавлены запросы пользователю при несоответствии версий при восстановлении; исправлено некорректное поведение запросов при восстановлении разных версий; улучшена логика декомпрессии для чтения напрямую из потока вместо рабочей памяти; добавлен параметр `--force`
* [Commit 3b35](https://github.com/manticoresoftware/manticoresearch-backup/commit/3b357ea) Добавлено отображение версии бэкапа после старта Manticore search для выявления проблем на данном этапе.
* [Commit ad2e](https://github.com/manticoresoftware/manticoresearch-backup/commit/ad2e5da) Обновлено сообщение об ошибке при неудачном подключении к демону.
* [Commit ce5e](https://github.com/manticoresoftware/manticoresearch-backup/commit/ce5ea85) Исправлена проблема с преобразованием абсолютных корневых путей бэкапа в относительные и удалена проверка на возможность записи при восстановлении для разрешения восстановления из разных путей.
* [Commit db7e](https://github.com/manticoresoftware/manticoresearch-backup/commit/db7e2b9) Добавлена сортировка для итератора файлов для обеспечения согласованности в различных ситуациях.
* [Issue #106](https://github.com/manticoresoftware/manticoresearch-backup/issues/105) Резервное копирование и восстановление нескольких конфигураций.
* [Issue #91](https://github.com/manticoresoftware/manticoresearch-backup/pull/91) Добавлен defattr для предотвращения необычных прав пользователя на файлы после установки на RHEL.
* [Issue #91](https://github.com/manticoresoftware/manticoresearch-backup/pull/91) Добавлен дополнительный chown для обеспечения установки файлов по умолчанию владельцем root в Ubuntu.

### Связано с MCL (columnar, secondary, knn libs)
* [Commit f104](https://github.com/manticoresoftware/columnar/commit/f104a4709bbb144140d09a09bc0fc1db1895e331) Поддержка поиска по векторам.
* [Commit 2169](https://github.com/manticoresoftware/columnar/commit/216953beef6676f7e34c9f6f1a30c5980d746710) Исправлена очистка временных файлов при прерванной сборке вторичного индекса. Это решает проблему с превышением лимита открытых файлов демоном при создании файлов `tmp.spidx`.
* [Commit 709b](https://github.com/manticoresoftware/columnar/commit/709b9acaaac97d9a1ca8796892f9ad432021c785) Использование отдельной библиотеки streamvbyte для columnar и SI.
* [Commit 1c26](https://github.com/manticoresoftware/manticoresearch/commit/1c2623763ced14bb94ad624dde9614a938408d89) Добавлено предупреждение, что столбцовое хранение не поддерживает json атрибуты.
* [Commit 3acd](https://github.com/manticoresoftware/columnar/commit/3acd6714ffce103447d7931dfc75bb1a7449f340) Исправлена распаковка данных в SI.
* [Commit 574c](https://github.com/manticoresoftware/manticoresearch/commit/574c023152564fa72092c18ebd1f594abfd4b6d7) Исправлен крах при сохранении дискового чанка с смешанным построчным и столбцовым хранением.
* [Commit e87f](https://github.com/manticoresoftware/columnar/commit/e87f088b458d4c5b1b6159fc72ed0d57b786891a) Исправлен итератор SI, который был намечен на блок, который уже обработан.
* [Issue #1474](https://github.com/manticoresoftware/manticoresearch/issues/1474) Обновление не работает для построчного MVA столбца с столбцовым движком.
* [Issue #1510](https://github.com/manticoresoftware/manticoresearch/issues/1510) Исправлен крах при агрегировании столбцового атрибута, используемого в `HAVING`.
* [Issue #1519](https://github.com/manticoresoftware/manticoresearch/issues/1519) Исправлен крах в ranker `expr` при использовании столбцового атрибута.

### Связано с Docker
* ❗[Issue #42](https://github.com/manticoresoftware/docker/issues/42) Поддержка [plain indexation](https://github.com/manticoresoftware/docker#building-plain-tables) через переменные окружения.
* ❗[Issue #47](https://github.com/manticoresoftware/docker/issues/47) Улучшена гибкость конфигурации через переменные окружения.
* [Issue #54](https://github.com/manticoresoftware/docker/pull/54) Улучшены процессы [backup and restore](https://github.com/manticoresoftware/docker#backup-and-restore) для Docker.
* [Issue #77](https://github.com/manticoresoftware/docker/issues/76) Улучшен entrypoint для обработки восстановления из резервной копии только при первом запуске.
* [Commit a27c](https://github.com/manticoresoftware/docker/commit/a27c048dd9e73d8d6bea8d9a4830cafb486b82c5) Исправлен лог запросов на stdout.
* [Issue #38](https://github.com/manticoresoftware/docker/issues/38) Отключены предупреждения BUDDY, если EXTRA не установлено.
* [Issue #71](https://github.com/manticoresoftware/docker/pull/71) Исправлено имя хоста в `manticore.conf.sh`.

## Версия 6.2.12
Выпущена: 23 августа 2023 г.

Версия 6.2.12 продолжает серию 6.2 и исправляет проблемы, обнаруженные после выпуска 6.2.0.

### Исправления ошибок
* ❗[Issue #1351](https://github.com/manticoresoftware/manticoresearch/issues/1351) "Manticore 6.2.0 не запускается через systemctl на Centos 7": Изменено `TimeoutStartSec` с `infinity` на `0` для лучшей совместимости с Centos 7.
* ❗[Issue #1364](https://github.com/manticoresoftware/manticoresearch/issues/1364) "Крах после обновления с 6.0.4 до 6.2.0": Добавлена функция воспроизведения пустых binlog файлов из более старых версий binlog.
* [PR #1334](https://github.com/manticoresoftware/manticoresearch/pull/1334) "исправление опечатки в searchdreplication.cpp": Исправлена опечатка в `searchdreplication.cpp`: beggining -> beginning.
* [Issue #1337](https://github.com/manticoresoftware/manticoresearch/issues/1337) "Manticore 6.2.0 WARNING: conn (local)(12), sock=8088: bailing on failed MySQL header, AsyncNetInputBuffer_c::AppendData: error 11 (Resource temporarily unavailable) return -1": Понижен уровень подробности предупреждения интерфейса MySQL о заголовке до logdebugv.
* [Issue #1355](https://github.com/manticoresoftware/manticoresearch/issues/1355) "join cluster зависает, если node_address не разрешается": Улучшена повторная попытка репликации при недоступности определенных узлов и сбоях разрешения их имен. Это решает проблемы с репликацией в Kubernetes и Docker. Улучшено сообщение об ошибке при старте репликации и внесены изменения в тестовую модель 376. Также добавлено ясное сообщение об ошибке при сбоях разрешения имён.
* [Issue #1361](https://github.com/manticoresoftware/manticoresearch/issues/1361) "Нет отображения в нижний регистр для 'Ø' в charset non_cjk": Скорректировано отображение символа 'Ø'.
* [Issue #1365](https://github.com/manticoresoftware/manticoresearch/issues/1365) "searchd оставляет binlog.meta и binlog.001 после чистой остановки": Обеспечено корректное удаление последнего пустого binlog файла.
* [Commit 0871](https://github.com/manticoresoftware/manticoresearch/commit/0871070845e93ad702ae0dfb89b35e5f24cd0851): Исправлена проблема сборки `Thd_t` на Windows, связанная с ограничениями атомарного копирования.
* [Commit 1cc0](https://github.com/manticoresoftware/manticoresearch/commit/1cc04bedc174737118fd5a3663e35f4e95bd4f8c): Исправлена проблема с FT CBO и `ColumnarScan`.
* [Commit c6bf](https://github.com/manticoresoftware/manticoresearch/commit/c6bfb30d53e3f4b67f17769667733677a661c95b): Внесены исправления в тест 376 и добавлена замена для ошибки `AF_INET` в тесте.
* [Commit cbc3](https://github.com/manticoresoftware/manticoresearch/commit/cbc38705e0b4dcc9665e3e8cf64826290ebd11cf): Решена проблема взаимоблокировки при репликации при обновлении blob-атрибутов и замене документов. Также удалена rlock индекса при коммите, так как он уже заблокирован на более базовом уровне.

### Мелкие изменения
* [Commit 4f91](https://github.com/manticoresoftware/manticoresearch/commit/4f913a36e84e01f950b2797051222b0b1094d6fe) Обновлена информация по `/bulk` эндпоинтам в руководстве.

### MCL
* Поддержка [Manticore Columnar Library](https://github.com/manticoresoftware/columnar) v2.2.4

## Версия 6.2.0
Выпущена: 4 августа 2023 г.

### Основные изменения
* Оптимизатор запросов был улучшен для поддержки полнотекстовых запросов, что значительно повысило эффективность и производительность поиска.
* Интеграции с:
  - [mysqldump](https://dev.mysql.com/doc/refman/8.0/en/mysqldump.html) - для [создания логических резервных копий](Securing_and_compacting_a_table/Backup_and_restore.md#Backup-and-restore-with-mysqldump) с помощью `mysqldump`
  - [Apache Superset](https://manticoresearch.com/blog/manticoresearch-apache-superset-integration/) и [Grafana](https://manticoresearch.com/blog/manticoresearch-grafana-integration/) для визуализации данных, хранящихся в Manticore
  - [HeidiSQL](https://www.heidisql.com/) и [DBForge](https://www.devart.com/dbforge/) для облегчения разработки с Manticore
* Мы начали использовать [GitHub workflows](https://github.com/manticoresoftware/manticoresearch/actions), что упрощает для участников использование того же процесса Continuous Integration (CI), который основная команда применяет при подготовке пакетов. Все задания можно запускать на GitHub-hosted раннерах, что облегчает беспрепятственное тестирование изменений в вашем форке Manticore Search.
* Мы начали использовать [CLT](https://github.com/manticoresoftware/clt) для тестирования сложных сценариев. Например, теперь мы можем гарантировать, что пакет, собранный после коммита, можно корректно установить во всех поддерживаемых операционных системах Linux. Command Line Tester (CLT) предоставляет удобный способ записи тестов в интерактивном режиме и легкого их воспроизведения.
* Существенное улучшение производительности операции count distinct за счет использования комбинации хеш-таблиц и HyperLogLog.
* Включено многопоточное выполнение запросов, содержащих вторичные индексы, с ограничением числа потоков числом физических ядер ЦП. Это должно значительно улучшить скорость выполнения запросов.
* `pseudo_sharding` был настроен на ограничение числом свободных потоков. Это обновление значительно повышает пропускную способность.
* Пользователи теперь могут указать [движок хранения атрибутов по умолчанию](Server_settings/Searchd.md#engine) через настройки конфигурации, что обеспечивает лучшую настройку под конкретные требования нагрузки.
* Поддержка [Manticore Columnar Library 2.2.0](https://github.com/manticoresoftware/columnar/) с многочисленными исправлениями ошибок и улучшениями в [вторичных индексах](Server_settings/Searchd.md#secondary_indexes).

### Незначительные изменения
* [Buddy #153](https://github.com/manticoresoftware/manticoresearch-buddy/issues/153): HTTP-эндпоинт [/pq](Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md?client=JSON#Adding-rules-to-a-percolate-table) теперь является псевдонимом для HTTP-эндпоинта `/json/pq`.
* [Коммит 0bf1](https://github.com/manticoresoftware/manticoresearch/commit/0bf17d9e): Обеспечена многобайтовая совместимость для функций `upper()` и `lower()`.
* [Коммит 2bb9](https://github.com/manticoresoftware/manticoresearch/commit/2bb92765): Вместо сканирования индекса для запросов `count(*)` теперь возвращается предварительно вычисленное значение.
* [Коммит 3c84](https://github.com/manticoresoftware/manticoresearch/commit/3c84091f): Теперь возможно использовать `SELECT` для произвольных вычислений и отображения `@@sysvars`. В отличие от предыдущего поведения, теперь можно выполнять более одного вычисления. Поэтому запросы вроде `select user(), database(), @@version_comment, version(), 1+1 as a limit 10` вернут все столбцы. Обратите внимание, что необязательный параметр 'limit' всегда будет игнорироваться.
* [Коммит 6aca](https://github.com/manticoresoftware/manticoresearch/commit/6aca32f0): Реализован заглушка для запроса `CREATE DATABASE`.
* [Коммит 9dc1](https://github.com/manticoresoftware/manticoresearch/commit/9dc12334): При выполнении `ALTER TABLE table REBUILD SECONDARY` вторичные индексы теперь всегда перестраиваются, даже если атрибуты не обновлялись.
* [Коммит 46ed](https://github.com/manticoresoftware/manticoresearch/commit/46edb089): Сортировщики, использующие предварительно вычисленные данные, теперь определяются до применения CBO для избежания ненужных вычислений CBO.
* [Коммит 102a](https://github.com/manticoresoftware/manticoresearch/commit/102ac604): Реализована имитация и использование стека полнотекстовых выражений для предотвращения сбоев демона.
* [Коммит 979f](https://github.com/manticoresoftware/manticoresearch/commit/979fa27c): Добавлен быстрый путь выполнения кода клонирования совпадений для матчей, которые не используют атрибуты string/mvas/json.
* [Коммит a073](https://github.com/manticoresoftware/manticoresearch/commit/a0735ffe): Добавлена поддержка команды `SELECT DATABASE()`. Однако она всегда будет возвращать `Manticore`. Это дополнение важно для интеграций с различными инструментами MySQL.
* [Коммит bc04](https://github.com/manticoresoftware/manticoresearch/commit/bc04908): Изменён формат ответа эндпоинта [/cli](Connecting_to_the_server/HTTP.md#/cli), а также добавлен эндпоинт `/cli_json`, функционирующий как прежний `/cli`.
* [Коммит d70b](https://github.com/manticoresoftware/manticoresearch/commit/d70b0d58): Значение `thread_stack` теперь можно изменить во время выполнения с помощью оператора `SET`. Поддерживаются как сессионный, так и общесистемный варианты. Текущие значения доступны в выводе команды `show variables`.
* [Коммит d96e](https://github.com/manticoresoftware/manticoresearch/commit/d96ec6b9): В код CBO интегрировано улучшение для более точной оценки сложности выполнения фильтров по строковым атрибутам.
* [Коммит e77d](https://github.com/manticoresoftware/manticoresearch/commit/e77dd72f5a04531c352fad0d7afcd2a1cbae2510): Улучшен расчет стоимости DocidIndex, что повысило общую производительность.
* [Коммит f3ae](https://github.com/manticoresoftware/manticoresearch/commit/f3ae8bea): Метрики нагрузки, аналогичные 'uptime' в Linux, теперь отображаются в команде `SHOW STATUS`.
* [Коммит f3cc](https://github.com/manticoresoftware/manticoresearch/commit/f3cc0971): Порядок полей и атрибутов для `DESC` и `SHOW CREATE TABLE` теперь совпадает с порядком в `SELECT * FROM`.
* [Коммит f3d2](https://github.com/manticoresoftware/manticoresearch/commit/f3d248a6): Разные внутренние парсеры теперь предоставляют свой внутренний мнемонический код (например, `P01`) при различных ошибках. Это улучшение помогает определить, какой парсер вызвал ошибку, а также скрывает несущественные внутренние детали.
* [Задача #271](https://github.com/manticoresoftware/manticoresearch/issues/271) "Иногда CALL SUGGEST не предлагает исправление опечатки из одной буквы": Улучшено поведение [SUGGEST/QSUGGEST](Searching/Spell_correction.md#CALL-QSUGGEST,-CALL-SUGGEST) для коротких слов: добавлена опция `sentence` для отображения всего предложения
* [Задача #696](https://github.com/manticoresoftware/manticoresearch/issues/696) "Percolate index не ищет правильно по точной фразе при включенном стемминге": Запрос percolate был изменен для обработки модификатора точного терма, улучшая функциональность поиска.
* [Задача #829](https://github.com/manticoresoftware/manticoresearch/issues/829) "МЕТОДЫ ФОРМАТИРОВАНИЯ ДАТЫ": добавлено выражение в списке выборки [date_format()](Functions/Date_and_time_functions.md#DATE_FORMAT()), которое раскрывает функцию `strftime()`.
* [Задача #961](https://github.com/manticoresoftware/manticoresearch/issues/961) "Сортировка бакетов через HTTP JSON API": введено необязательное [свойство sort](Searching/Faceted_search.md#HTTP-JSON) для каждого бакета агрегатов в HTTP-интерфейсе.
* [Задача #1062](https://github.com/manticoresoftware/manticoresearch/issues/1062) "Улучшение логирования ошибок failure API вставки JSON - "unsupported value type"": Точка доступа `/bulk` теперь сообщает информацию о количестве обработанных и необработанных строк (документов) в случае ошибки.
* [Задача #1070](https://github.com/manticoresoftware/manticoresearch/issues/1070) "CBO hints не поддерживают несколько атрибутов": Включена поддержка множественных атрибутов в подсказках индекса.
* [Задача #1106](https://github.com/manticoresoftware/manticoresearch/issues/1106) "Добавить теги к HTTP поисковому запросу": Теги были добавлены в [HTTP PQ ответы](Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md?client=JSON#Adding-rules-to-a-percolate-table).
* [Задача #1301](https://github.com/manticoresoftware/manticoresearch/issues/1301) "buddy не должен создавать таблицу параллельно": Исправлена проблема, вызывавшая сбои при параллельных операциях CREATE TABLE. Теперь одновременно может выполняться только одна операция `CREATE TABLE`.
* [Задача #1303](https://github.com/manticoresoftware/manticoresearch/issues/1303) "Добавлена поддержка @ в названиях колонок".
* [Задача #1316](https://github.com/manticoresoftware/manticoresearch/issues/1316) "Запросы на наборе данных taxi медленные с ps=1": Логика CBO уточнена, установлен режим разрешения гистограммы по умолчанию 8k для лучшей точности по атрибутам с случайно распределёнными значениями.
* [Задача #1317](https://github.com/manticoresoftware/manticoresearch/issues/1317) "Исправить CBO vs полнотекстовый поиск на hn dataset": Реализована усовершенствованная логика для определения, когда использовать пересечение итераторов bitmap и когда использовать очередь с приоритетом.
* [Задача #1318](https://github.com/manticoresoftware/manticoresearch/issues/1318) "columnar: изменение интерфейса итератора на одновызовной": Колонарные итераторы теперь используют один вызов `Get`, заменяя прежние двухшаговые вызовы `AdvanceTo` + `Get` для получения значения.
* [Задача #1319](https://github.com/manticoresoftware/manticoresearch/issues/1319) "Ускорение вычисления агрегатов (удалить CheckReplaceEntry?)": Вызов `CheckReplaceEntry` был удалён из группировщика сортировки для ускорения вычисления агрегатных функций.
* [Задача #1320](https://github.com/manticoresoftware/manticoresearch/issues/1320) "create table read_buffer_docs/hits не понимает синтаксис k/m/g": Опции `read_buffer_docs` и `read_buffer_hits` в `CREATE TABLE` теперь поддерживают синтаксис k/m/g.
* [Языковые пакеты](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) для английского, немецкого и русского теперь можно легко устанавливать на Linux с помощью команды `apt/yum install manticore-language-packs`. На macOS используйте команду `brew install manticoresoftware/tap/manticore-language-packs`.
* Порядок полей и атрибутов теперь совпадает между операциями `SHOW CREATE TABLE` и `DESC`.
* Если при выполнении запросов `INSERT` недостаточно места на диске, новые запросы `INSERT` будут завершаться неудачей до появления достаточного пространства.
* Добавлена функция преобразования типа [UINT64()](Functions/Type_casting_functions.md#UINT64%28%29).
* Точка доступа `/bulk` теперь обрабатывает пустые строки как команду [commit](Data_creation_and_modification/Transactions.md#BEGIN,-COMMIT,-and-ROLLBACK). Подробнее [здесь](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md?client=JSON#Bulk-adding-documents).
* Реализованы предупреждения для [недопустимых подсказок индекса](Searching/Options.md#Query-optimizer-hints), обеспечивающие большую прозрачность и позволяющие избежать ошибок.
* При использовании `count(*)` с одним фильтром запросы теперь используют предварительно вычисленные данные из вторичных индексов, если они доступны, что существенно ускоряет выполнение запросов.

### ⚠️ Изменения, нарушающие совместимость
* ⚠️ Таблицы, созданные или изменённые в версии 6.2.0, не могут быть прочитаны более старыми версиями.
* ⚠️ Идентификаторы документов теперь обрабатываются как беззнаковые 64-битные целые числа при индексации и операциях INSERT.
* ⚠️ Обновлен синтаксис подсказок оптимизатора запроса. Новый формат: `/*+ SecondaryIndex(uid) */`. Обратите внимание, что старый синтаксис больше не поддерживается.
* ⚠️ [Задача #1160](https://github.com/manticoresoftware/manticoresearch/issues/1160): Использование `@` в названиях таблиц запрещено во избежание конфликтов синтаксиса.
* ⚠️ Строковые поля/атрибуты, отмеченные как `indexed` и `attribute`, теперь рассматриваются как одно поле при операциях `INSERT`, `DESC` и `ALTER`.
* ⚠️ [Задача #1057](https://github.com/manticoresoftware/manticoresearch/issues/1057): Библиотеки MCL больше не будут загружаться на системах, не поддерживающих SSE 4.2.
* ⚠️ [Issue #1143](https://github.com/manticoresoftware/manticoresearch/issues/1143): [agent_query_timeout](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_query_timeout) был сломан. Исправлен и теперь работает корректно.

### Исправления ошибок
* [Commit 2a6e](https://github.com/manticoresoftware/manticoresearch/commit/2a6ea8f7) "Сбой при DROP TABLE": решена проблема, вызывавшая длительное ожидание завершения операций записи (оптимизация, сохранение дискового чанка) в RT-таблице при выполнении команды DROP TABLE. Добавлено предупреждение при наличии файлов в каталоге таблицы после выполнения DROP TABLE.
* [Commit 2ebd](https://github.com/manticoresoftware/manticoresearch/commit/2ebd424d): Добавлена поддержка колоннарных атрибутов, которой ранее не хватало в коде группировки по нескольким атрибутам.
* [Commit 3be4](https://github.com/manticoresoftware/manticoresearch/commit/3be4503f752216b37b8cde8e47fd2d77bbce63ae) Решена проблема сбоя, возможно вызванного нехваткой места на диске, за счет правильной обработки ошибок записи в binlog.
* [Commit 6adb](https://github.com/manticoresoftware/manticoresearch/commit/6adb0934): Исправлен сбой, который иногда происходил при использовании нескольких колоннарных итераторов сканирования (или итераторов вторичных индексов) в одном запросе.
* [Commit 6bd9](https://github.com/manticoresoftware/manticoresearch/commit/6bd9f709): Фильтры не удалялись при использовании сортировщиков с предвычисленными данными. Исправлено.
* [Commit 6d03](https://github.com/manticoresoftware/manticoresearch/commit/6d03566a): Обновлен код CBO для улучшенной оценки запросов с фильтрами по строчным атрибутам, выполняемых в нескольких потоках.
* [Commit 6dd3](https://github.com/manticoresoftware/manticoresearch/commit/6dd3964), [Helm #56](https://github.com/manticoresoftware/manticoresearch-helm/issues/56) "фатальный дамп сбоя в Kubernetes-кластере": Исправлен дефектный фильтр Блума для корня JSON-объекта; исправлен сбой демона при фильтрации по JSON-полю.
* [Commit 6e1b](https://github.com/manticoresoftware/manticoresearch/commit/6e1b849) Исправлен сбой демона, вызванный неверной конфигурацией `manticore.json`.
* [Commit 6fbc](https://github.com/manticoresoftware/manticoresearch/commit/6fbc4189) Исправлен json range фильтр для поддержки int64 значений.
* [Commit 9c67](https://github.com/manticoresoftware/manticoresearch/commit/9c677426) `.sph` файлы могли повреждаться при `ALTER`. Исправлено.
* [Commit 77cc](https://github.com/manticoresoftware/manticoresearch/commit/77ccd8f): Добавлен общий ключ для репликации оператора replace для решения ошибки `pre_commit`, возникающей при репликации replace с нескольких мастер-узлов.
* [Commit 2884](https://github.com/manticoresoftware/manticoresearch/commit/2884519e) решены проблемы с проверками bigint в функциях типа `date_format()`.
* [Commit 9513](https://github.com/manticoresoftware/manticoresearch/commit/9513aca5): Итераторы больше не отображаются в [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META), когда сортировщики используют предвычисленные данные.
* [Commit a2a7](https://github.com/manticoresoftware/manticoresearch/commit/a2a70555): Обновлен размер стека fulltext-узла для предотвращения сбоев при сложных полнотекстовых запросах.
* [Commit a062](https://github.com/manticoresoftware/manticoresearch/commit/a0626d7e): Исправлена ошибка, вызывавшая сбой при репликации обновлений с JSON и строковыми атрибутами.
* [Commit b3e6](https://github.com/manticoresoftware/manticoresearch/commit/b3e63e5d): Обновлен сборщик строк для использования 64-битных целых чисел, чтобы избежать сбоев при работе с большими объемами данных.
* [Commit c472](https://github.com/manticoresoftware/manticoresearch/commit/c472e5b): Исправлен сбой, возникавший при использовании count distinct по нескольким индексам.
* [Commit d073](https://github.com/manticoresoftware/manticoresearch/commit/d0730272): Исправлена проблема, когда запросы по дисковым чанкам RT-индексов могли выполняться в нескольких потоках даже при отключенном `pseudo_sharding`.
* [Commit d205](https://github.com/manticoresoftware/manticoresearch/commit/d205508e) Набор значений, возвращаемых командой `show index status`, изменен и теперь зависит от типа используемого индекса.
* [Commit e9bc](https://github.com/manticoresoftware/manticoresearch/commit/e9bc2f9) Исправлена ошибка HTTP при обработке пакетных запросов и проблема с отсутствием возврата ошибки клиенту из сетевого цикла.
* [Commit f77c](https://github.com/manticoresoftware/manticoresearch/commit/f77ce0e6) использование расширенного стека для PQ.
* [Commit fac2](https://github.com/manticoresoftware/manticoresearch/commit/fac2d9687a20582765aaa89be9597804c4f845af) Обновлен вывод ранжировщика экспорта в соответствии с [packedfactors()](Functions/Searching_and_ranking_functions.md#PACKEDFACTORS%28%29).
* [Commit ff87](https://github.com/manticoresoftware/manticoresearch/commit/ff87cc4): Исправлена проблема со строковым списком в фильтре лога запросов SphinxQL.
* [Issue #589](https://github.com/manticoresoftware/manticoresearch/issues/589) "Определение charset зависит от порядка кодов": Исправлена неправильная маппировка charset для дубликатов.
* [Issue #811](https://github.com/manticoresoftware/manticoresearch/issues/811) "Маппинг нескольких слов в word forms мешает поиску фраз с CJK-знаками препинания между ключевыми словами": Исправлена позиция нграм токенов в фразовом запросе с wordforms.
* [Issue #834](https://github.com/manticoresoftware/manticoresearch/issues/834) "Знак равенства в поисковом запросе ломает запрос": Обеспечена возможность экранирования точного символа и исправлено двойное точное расширение опцией `expand_keywords`.
* [Issue #864](https://github.com/manticoresoftware/manticoresearch/issues/864) "конфликт exceptions/stopwords"
* [Issue #910](https://github.com/manticoresoftware/manticoresearch/issues/910) "Сбой Manticore при вызове call snippets() с libstemmer_fr и index_exact_words": Решены внутренние конфликты, вызывавшие сбои при вызове `SNIPPETS()`.
* [Issue #946](https://github.com/manticoresoftware/manticoresearch/issues/946) "Дублирование записей при SELECT": Исправлена проблема с дублированием документов в наборе результатов для запроса с опцией `not_terms_only_allowed` к RT индексу с удалёнными документами.
* [Issue #967](https://github.com/manticoresoftware/manticoresearch/issues/967) "Использование JSON аргументов в UDF функциях приводит к сбою": Исправлен сбой демона при обработке поиска с включённым псевдо-шардированием и UDF с JSON аргументом.
* [Issue #1050](https://github.com/manticoresoftware/manticoresearch/issues/1050) "count(*) в FEDERATED": Исправлен сбой демона, возникавший при запросе через движок `FEDERATED` с агрегатом.
* [Issue #1052](https://github.com/manticoresoftware/manticoresearch/issues/1052) Исправлена проблема несовместимости столбца `rt_attr_json` с колонным хранилищем.
* [Issue #1072](https://github.com/manticoresoftware/manticoresearch/issues/1072) "* удаляется из поискового запроса из-за ignore_chars": Исправлено, чтобы подстановочные символы в запросе не затрагивались `ignore_chars`.
* [Issue #1075](https://github.com/manticoresoftware/manticoresearch/issues/1075) "indextool --check не работает, если есть распределённая таблица": indextool теперь совместим с инстансами, имеющими индексы 'distributed' и 'template' в json конфиге.
* [Issue #1081](https://github.com/manticoresoftware/manticoresearch/issues/1081) "конкретный select на конкретном RT наборе данных приводит к сбою searchd": Исправлен сбой демона при запросе с packedfactors и большим внутренним буфером.
* [Issue #1095](https://github.com/manticoresoftware/manticoresearch/issues/1095) "С удалёнными документами игнорируются с not_terms_only_allowed"
* [Issue #1099](https://github.com/manticoresoftware/manticoresearch/issues/1099) "indextool --dumpdocids не работает": Восстановлена работа команды `--dumpdocids`.
* [Issue #1100](https://github.com/manticoresoftware/manticoresearch/issues/1100) "indextool --buildidf не работает": indextool теперь закрывает файл после завершения globalidf.
* [Issue #1104](https://github.com/manticoresoftware/manticoresearch/issues/1104) "Count(*) пытается обрабатываться как schema set в удалённых таблицах": Исправлена ошибка, при которой демон отправлял сообщение об ошибке для запросов в распределённый индекс, когда агент возвращал пустой набор результатов.
* [Issue #1109](https://github.com/manticoresoftware/manticoresearch/issues/1109) "FLUSH ATTRIBUTES зависает при threads=1".
* [Issue #1126](https://github.com/manticoresoftware/manticoresearch/issues/1126) "Потеря соединения с MySQL сервером во время запроса - manticore 6.0.5": Исправлены сбои, возникавшие при использовании множества фильтров по колонным атрибутам.
* [Issue #1135](https://github.com/manticoresoftware/manticoresearch/issues/1135) "Чувствительность к регистру при фильтрации JSON строк": Исправлена сортировка для правильной работы фильтров, используемых в HTTP поисковых запросах.
* [Issue #1140](https://github.com/manticoresoftware/manticoresearch/issues/1140) "Поиск не в том поле": Исправлен ущерб, связанный с `morphology_skip_fields`.
* [Issue #1155](https://github.com/manticoresoftware/manticoresearch/issues/1155) "Системные удалённые команды через API должны передавать g_iMaxPacketSize": Внесены изменения для обхода проверки `max_packet_size` для команд репликации между узлами. Также добавлена последняя ошибка кластера в отображение статуса.
* [Issue #1302](https://github.com/manticoresoftware/manticoresearch/issues/1302) "Временные файлы остаются после ошибки оптимизации": Исправлена проблема с оставлением временных файлов после ошибки во время слияния или оптимизации.
* [Issue #1304](https://github.com/manticoresoftware/manticoresearch/issues/1304) "Добавлена переменная окружения для таймаута запуска buddy": Добавлена переменная окружения `MANTICORE_BUDDY_TIMEOUT` (по умолчанию 3 секунды) для управления временем ожидания сообщения buddy при запуске демона.
* [Issue #1305](https://github.com/manticoresoftware/manticoresearch/issues/1305) "Переполнение int при сохранении PQ meta": Снижено чрезмерное потребление памяти демоном при сохранении большого PQ индекса.
* [Issue #1306](https://github.com/manticoresoftware/manticoresearch/issues/1306) "Невозможно воссоздать RT таблицу после изменения её внешнего файла": Исправлена ошибка alter с пустой строкой для внешних файлов; исправлены оставшиеся внешние файлы RT индекса после изменения внешних файлов.
* [Issue #1307](https://github.com/manticoresoftware/manticoresearch/issues/1307) "SELECT выражение sum(value) as value работает неправильно": Исправлена проблема, когда выражение в списке select с алиасом могло скрывать атрибут индекса; также исправлено sum на count для int64 с целочисленным типом.
* [Issue #1308](https://github.com/manticoresoftware/manticoresearch/issues/1308) "Избегать привязки к localhost в репликации": Обеспечено, что репликация не привязывается к localhost для хостов с несколькими IP.
* [Issue #1309](https://github.com/manticoresoftware/manticoresearch/issues/1309) "Ответ mysql клиенту не работает для данных больше 16Mb": Исправлена проблема возврата пакета SphinxQL клиенту размером более 16 Мб.
* [Issue #1310](https://github.com/manticoresoftware/manticoresearch/issues/1310) "Неправильная ссылка в "пути к внешним файлам должны быть абсолютными": Исправлено отображение полного пути к внешним файлам в `SHOW CREATE TABLE`.
* [Issue #1311](https://github.com/manticoresoftware/manticoresearch/issues/1311) "Отладочная сборка падает на длинных строках в сниппетах": Теперь длинные строки (>255 символов) разрешены в тексте, на который нацеливается функция `SNIPPET()`.
* [Issue #1312](https://github.com/manticoresoftware/manticoresearch/issues/1312) "Ложный сбой при use-after-delete в kqueue polling (master-agent)": Исправлены сбои, когда мастер не может подключиться к агенту на системах с kqueue (FreeBSD, MacOS и др.).
* [Issue #1313](https://github.com/manticoresoftware/manticoresearch/issues/1313) "Слишком долгие подключения к самому себе": При подключении от мастера к агентам на MacOS/BSD теперь используется объединённый таймаут connect+query вместо только connect.
* [Issue #1314](https://github.com/manticoresoftware/manticoresearch/issues/1314) "pq (json meta) с недостигнутыми встроенными синонимами не загружается": Исправлен флаг встроенных синонимов в pq.
* [Issue #1315](https://github.com/manticoresoftware/manticoresearch/issues/1315) "Разрешить некоторым функциям (sint, fibonacci, second, minute, hour, day, month, year, yearmonth, yearmonthday) использовать неявно расширенные значения аргументов".
* [Issue #1321](https://github.com/manticoresoftware/manticoresearch/issues/1321) "Включить многопоточный SI в fullscan, но ограничить количество потоков": В CBO реализован код для лучшего прогнозирования многопоточной производительности вторичных индексов при их использовании в полнотекстовом запросе.
* [Issue #1322](https://github.com/manticoresoftware/manticoresearch/issues/1322) "count(*) запросы все еще медленные после использования precalc сортировщиков": Итераторы больше не инициализируются при использовании сортировщиков с предвычисленными данными, что устраняет негативное влияние на производительность.
* [Issue #1411](https://github.com/manticoresoftware/manticoresearch/issues/1141) "журнал запросов в sphinxql не сохраняет оригинальные запросы для MVA": Теперь логируются `all()/any()`.

## Версия 6.0.4
Выпущено: 15 марта 2023

### Новые возможности
* Улучшена интеграция с Logstash, Beats и т.п., включая:
  - Поддержка версий Logstash 7.6 - 7.15, Filebeat 7.7 - 7.12
  - Автоматическая поддержка схем.
  - Добавлена обработка массовых запросов в формате, похожем на Elasticsearch.
* [Коммит Buddy ce90](https://github.com/manticoresoftware/manticoresearch-buddy/commit/ce907ea) Логирование версии Buddy при запуске Manticore.

### Исправления ошибок
* [Issue #588](https://github.com/manticoresoftware/manticoresearch/issues/588), [Issue #942](https://github.com/manticoresoftware/manticoresearch/issues/942) Исправлен неправильный символ в метаданных поиска и CALL KEYWORDS для индекса биграмм.
* [Issue #1027](https://github.com/manticoresoftware/manticoresearch/issues/1027) Отклонены HTTP-заголовки с маленькими буквами.
* ❗[Issue #1039](https://github.com/manticoresoftware/manticoresearch/issues/1039) Исправлена утечка памяти в демоне при чтении вывода консоли Buddy.
* [Issue #1056](https://github.com/manticoresoftware/manticoresearch/issues/1056) Исправлено неожиданное поведение вопросительного знака.
* [Issue #1064](https://github.com/manticoresoftware/manticoresearch/issues/1064) Исправлено состояние гонки в таблицах приведения к нижнему регистру токенизатора, вызывавшее сбой.
* [Коммит 59bb](https://github.com/manticoresoftware/manticoresearch/commit/59bb54c) Исправлена обработка массовых записей в JSON-интерфейсе для документов с явно установленным id в null.
* [Коммит 7b6b](https://github.com/manticoresoftware/manticoresearch/commit/7b6b25f) Исправлена статистика терминов в CALL KEYWORDS для нескольких одинаковых терминов.
* [Коммит f381](https://github.com/manticoresoftware/manticoresearch/commit/f381ad2) Конфигурация по умолчанию теперь создается установщиком под Windows; пути больше не заменяются во время выполнения.
* [Коммит 6940](https://github.com/manticoresoftware/manticoresearch/commit/6940e95), [Коммит cc5a](https://github.com/manticoresoftware/manticoresearch/commit/cc5a480) Исправлены проблемы репликации для кластера с узлами в разных сетях.
* [Коммит 4972](https://github.com/manticoresoftware/manticoresearch/commit/49722ab) Исправлена точка входа HTTP `/pq` как псевдоним для `/json/pq`.
* [Коммит 3b53](https://github.com/manticoresoftware/manticoresearch/commit/3b5385a) Исправлен сбой демона при перезапуске Buddy.
* [Коммит Buddy fba9](https://github.com/manticoresoftware/manticoresearch-buddy/commit/fba9c8c) Отображение оригинальной ошибки при получении некорректного запроса.
* [Коммит Buddy db95](https://github.com/manticoresoftware/manticoresearch-buddy/commit/db9532c) Разрешены пробелы в пути резервной копии и добавлена поддержка одинарных кавычек в регэкспах.

## Версия 6.0.2
Выпущено: 10 февраля 2023

### Исправления ошибок
* [Issue #1024 crash 2](https://github.com/manticoresoftware/manticoresearch/issues/1024) Сбой / ошибка сегментации при поиске фасетов с большим числом результатов
* ❗[Issue #1029](https://github.com/manticoresoftware/manticoresearch/issues/1029) - ВНИМАНИЕ: Встроенное значение KNOWN_CREATE_SIZE (16) меньше измеренного (208). Рекомендуется исправить значение!
* ❗[Issue #1032](https://github.com/manticoresoftware/manticoresearch/issues/1032) - Сбой plain индекса в Manticore 6.0.0
* ❗[Issue #1033](https://github.com/manticoresoftware/manticoresearch/issues/1033) - потеря нескольких распределённых индексов при перезапуске демона
* ❗[Issue #1064](https://github.com/manticoresoftware/manticoresearch/issues/1064) - состояние гонки в таблицах приведения к нижнему регистру токенизатора

## Версия 6.0.0
Выпущено: 7 февраля 2023

Начиная с этого выпуска, Manticore Search поставляется с Manticore Buddy — сопутствующим демоном, написанным на PHP, который занимается высокоуровневой функциональностью, не требующей сверхнизкой задержки или высокой производительности. Manticore Buddy работает в фоновом режиме, и вы можете даже не подозревать о его запуске. Хотя для конечного пользователя он невидим, создание Manticore Buddy, легко устанавливаемого и совместимого с основным демоном на C++, было большой задачей. Это крупное изменение позволит команде разрабатывать широкий спектр новых высокоуровневых функций, таких как оркестровка шардов, контроль доступа и аутентификация, а также различные интеграции, например mysqldump, DBeaver, Grafana mysql connector. Уже сейчас он обрабатывает [SHOW QUERIES](Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES), [BACKUP](Securing_and_compacting_a_table/Backup_and_restore.md#BACKUP-SQL-command-reference) и [Авто-схему](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema).

В этот релиз также вошло более 130 исправлений ошибок и множество новых возможностей, многие из которых можно считать значительными.

### Основные изменения
* 🔬 Экспериментально: теперь вы можете выполнять совместимые с Elasticsearch [insert](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md?example=Elasticsearch#Adding-documents-to-a-real-time-table) и [replace](Data_creation_and_modification/Updating_documents/REPLACE.md?example=Elasticsearch#REPLACE) JSON-запросы, что позволяет использовать Manticore с такими инструментами, как Logstash (версия < 7.13), Filebeat и другие инструменты из семейства Beats. Включено по умолчанию. Вы можете отключить это с помощью `SET GLOBAL ES_COMPAT=off`.
* Поддержка [Manticore Columnar Library 2.0.0](https://github.com/manticoresoftware/columnar/) с многочисленными исправлениями и улучшениями в [вторичных индексах](Server_settings/Searchd.md#secondary_indexes). **⚠️ КАРДИНАЛЬНОЕ ИЗМЕНЕНИЕ**: вторичные индексы включены по умолчанию начиная с этого релиза. Обязательно выполните [ALTER TABLE table_name REBUILD SECONDARY](Updating_table_schema_and_settings.md#Rebuilding-a-secondary-index), если вы обновляетесь с Manticore 5. Подробнее см. ниже.
* [Коммит c436](https://github.com/manticoresoftware/manticoresearch/commit/c436f9023536f767610451911955ae36d90aa638) Auto-schema: теперь вы можете пропустить создание таблицы, просто вставьте первый документ, и Manticore создаст таблицу автоматически на основе его полей. Подробнее об этом читайте [здесь](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema). Включается/выключается с помощью [searchd.auto_schema](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema).
* Полный переработка [оптимизатора на основе стоимости](Searching/Cost_based_optimizer.md), который во многих случаях снижает время отклика запросов.
  - [Issue #1008](https://github.com/manticoresoftware/manticoresearch/issues/1008) Оценка производительности параллелизации в CBO.
  - [Issue #1014](https://github.com/manticoresoftware/manticoresearch/issues/1014) CBO теперь учитывает [вторичные индексы](Server_settings/Searchd.md#secondary_indexes) и может действовать умнее.
  - [Коммит cef9](https://github.com/manticoresoftware/columnar/commit/cef9b54dc3a4ee9138ebc4df3a671715b996d6b3) Статистика кодирования столбцовых таблиц/полей теперь хранится в метаданных для помощи CBO принимать более умные решения.
  - [Коммит 2b95](https://github.com/manticoresoftware/manticoresearch/commit/2b95fb8c2ba422c9c5a66e2b61c1f7922f31c7aa) Добавлены подсказки CBO для точной настройки его поведения.
* [Телеметрия](Telemetry.md#Telemetry): мы рады сообщить о добавлении телеметрии в этом релизе. Эта функция позволяет собирать анонимные и деперсонализированные метрики, которые помогут нам улучшить производительность и удобство использования продукта. Будьте уверены, все собранные данные **абсолютно анонимны и не будут связаны с какой-либо личной информацией**. Эту функцию можно [легко отключить](Telemetry.md#Telemetry) в настройках при желании.
* [Коммит 5aaf](https://github.com/manticoresoftware/manticoresearch/commit/5aafb17d81b601e399e212a344a4738f65576cdd) [ALTER TABLE table_name REBUILD SECONDARY](Updating_table_schema_and_settings.md#Rebuilding-a-secondary-index) для перестройки вторичных индексов в любое время, например:
  - при миграции с Manticore 5 на новую версию,
  - при выполнении [UPDATE](Data_creation_and_modification/Updating_documents/UPDATE.md#UPDATE) (т.е. [обновления на месте, а не замены](Data_creation_and_modification/Updating_documents/REPLACE_vs_UPDATE.md)) атрибута в индексе
* [Issue #821](https://github.com/manticoresoftware/manticoresearch/issues/821) Новый инструмент `manticore-backup` для [резервного копирования и восстановления экземпляра Manticore](Securing_and_compacting_a_table/Backup_and_restore.md)
* SQL-команда [BACKUP](Securing_and_compacting_a_table/Backup_and_restore.md#BACKUP-SQL-command-reference) для создания резервных копий изнутри Manticore.
* SQL-команда [SHOW QUERIES](Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES) как простой способ увидеть выполняющиеся запросы вместо потоков.
* [Issue #551](https://github.com/manticoresoftware/manticoresearch/issues/551) SQL-команда `KILL` для остановки долгих `SELECT`.
* Динамический `max_matches` для агрегационных запросов для повышения точности и уменьшения времени отклика.

### Небольшие изменения
* [Issue #822](https://github.com/manticoresoftware/manticoresearch/issues/822) SQL-команды [FREEZE/UNFREEZE](Securing_and_compacting_a_table/Freezing_a_table.md) для подготовки таблицы real-time/plain к резервному копированию.
* [Коммит c470](https://github.com/manticoresoftware/manticoresearch/commit/c47052f18ad4868134de1a97e8d7da570e531dfa) Новые настройки `accurate_aggregation` и `max_matches_increase_threshold` для управления точностью агрегации.
* [Issue #718](https://github.com/manticoresoftware/manticoresearch/issues/718) Поддержка знаковых отрицательных 64-битных ID. Обратите внимание, IDs > 2^63 по-прежнему использовать нельзя, но теперь можно использовать IDs в диапазоне от -2^63 до 0.
* Поскольку мы недавно добавили поддержку вторичных индексов, возникла путаница, так как «индекс» мог означать вторичный индекс, полнотекстовый индекс или plain/real-time `index`. Чтобы уменьшить путаницу, мы переименовываем последний в «таблица». Следующие SQL/командные строки затронуты этим изменением. Их старые версии устарели, но всё ещё работают:
  - `index <table name>` => `table <table name>`,
  - `searchd -i / --index` => `searchd -t / --table`,
  - `SHOW INDEX STATUS` => `SHOW TABLE STATUS`,
  - `SHOW INDEX SETTINGS` => `SHOW TABLE SETTINGS`,
  - `FLUSH RTINDEX` => `FLUSH TABLE`,
  - `OPTIMIZE INDEX` => `OPTIMIZE TABLE`,
  - `ATTACH TABLE plain TO RTINDEX rt` => `ATTACH TABLE plain TO TABLE rt`,
  - `RELOAD INDEX` => `RELOAD TABLE`,
  - `RELOAD INDEXES` => `RELOAD TABLES`.

  Мы не планируем делать устаревшими старые формы, но чтобы обеспечить совместимость с документацией, рекомендуем изменить имена в вашем приложении. В будущих релизах будет изменено только отображение «index» на «table» в выводе различных SQL и JSON команд.
* Запросы с stateful UDF теперь принудительно выполняются в одном потоке.
* [Issue #1011](https://github.com/manticoresoftware/manticoresearch/issues/1011) Рефакторинг всего, что связано с планированием времени, как предпосылка для параллельного слияния чанков.
* **⚠️ ЛОМАННОЕ ИЗМЕНЕНИЕ**: Формат колоночного хранения был изменён. Необходимо перестроить те таблицы, которые имеют колоночные атрибуты.
* **⚠️ ЛОМАННОЕ ИЗМЕНЕНИЕ**: Формат файла вторичных индексов изменён, поэтому если вы используете вторичные индексы для поиска и у вас в конфигурационном файле `searchd.secondary_indexes = 1`, учтите, что новая версия Manticore **пропустит загрузку таблиц с вторичными индексами**. Рекомендуется:
  - Перед обновлением изменить `searchd.secondary_indexes` на 0 в конфигурационном файле.
  - Запустить инстанс. Manticore загрузит таблицы с предупреждением.
  - Выполнить `ALTER TABLE <table name> REBUILD SECONDARY` для каждого индекса, чтобы перестроить вторичные индексы.

  Если у вас настроен репликационный кластер, нужно выполнить `ALTER TABLE <table name> REBUILD SECONDARY` на всех узлах или следовать [этой инструкции](Securing_and_compacting_a_table/Compacting_a_table.md#Optimizing-clustered-tables), заменяя `OPTIMIZE` на команду `ALTER .. REBUILD SECONDARY`.
* **⚠️ ЛОМАННОЕ ИЗМЕНЕНИЕ**: Версия binlog обновлена, поэтому binlog файлы предыдущих версий не будут проигрываться. Важно, чтобы Manticore Search останавливался корректно во время процесса обновления. Это значит, что в директории `/var/lib/manticore/binlog/` после остановки предыдущего инстанса не должно остаться binlog файлов, кроме `binlog.meta`.
* [Issue #849](https://github.com/manticoresoftware/manticoresearch/issues/849) `SHOW SETTINGS`: теперь можно просматривать настройки из конфигурационного файла изнутри Manticore.
* [Issue #1007](https://github.com/manticoresoftware/manticoresearch/issues/1007) [SET GLOBAL CPUSTATS=1/0](Server_settings/Setting_variables_online.md#SET) включает/отключает отслеживание процессорного времени; [SHOW THREADS](Node_info_and_management/SHOW_THREADS.md) теперь не показывает статистику CPU, когда отслеживание процессорного времени выключено.
* [Issue #1009](https://github.com/manticoresoftware/manticoresearch/issues/1009) Сегменты RAM чанков в RT таблицах теперь могут сливаются во время сброса RAM чанка.
* [Issue #1012](https://github.com/manticoresoftware/manticoresearch/issues/1012) Добавлен прогресс создания вторичных индексов в вывод [indexer](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool).
* [Issue #1013](https://github.com/manticoresoftware/manticoresearch/issues/1013) Ранее запись таблицы могла быть удалена Manticore из списка индексов, если она не могла начать обслуживать её при старте. Новое поведение — оставлять ее в списке, чтобы попытаться загрузить при следующем запуске.
* [indextool --docextract](Miscellaneous_tools.md#indextool) возвращает все слова и совпадения, относящиеся к запрашиваемому документу.
* [Commit 2b29](https://github.com/manticoresoftware/manticoresearch/commit/2b296ee20e520b85bcbb4383e87095d31e7165dc) Переменная окружения `dump_corrupt_meta` позволяет сбрасывать метаданные повреждённой таблицы в лог, если searchd не может загрузить индекс.
* [Commit c7a3](https://github.com/manticoresoftware/manticoresearch/commit/c7a3136eefe0eb2f69e3d99fae9c10beb5821ea6) `DEBUG META` теперь может показывать `max_matches` и статистику псевдо шардирования.
* [Commit 6bca](https://github.com/manticoresoftware/manticoresearch/commit/6bca4848dc5c8396dbc1e057be4b9b2d579794e8) Более информативная ошибка вместо запутывающего "Index header format is not json, will try it as binary...".
* [Commit bef3](https://github.com/manticoresoftware/lemmatizer-uk/commit/bef3ff0386d3ee87ec57619782100972c1122e47) Путь к украинскому лемматизатору изменён.
* [Commit 4ae7](https://github.com/manticoresoftware/manticoresearch/commit/4ae789595329a2951e194d1191ddb3121459a560) Статистика вторичных индексов добавлена в [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META).
* [Commit 2e7c](https://github.com/manticoresoftware/manticoresearch/commit/2e7c585e) JSON-интерфейс теперь легко визуализировать с помощью Swagger Editor https://manual.manticoresearch.com/Openapi#OpenAPI-specification.
* **⚠️ ЛОМАННОЕ ИЗМЕНЕНИЕ**: Протокол репликации изменён. Если у вас репликационный кластер, то при обновлении до Manticore 5 нужно:
  - корректно остановить все узлы
  - затем запустить последний остановленный узел с `--new-cluster` (на Linux использовать инструмент `manticore_new_cluster`).
  - Подробнее о [перезапуске кластера](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster).

### Изменения, связанные с Manticore Columnar Library
* Рефакторинг интеграции вторичных индексов с колоночным хранением.
* [Commit efe2](https://github.com/manticoresoftware/columnar/commit/efe26b2a) Оптимизация Manticore Columnar Library, позволяющая снизить время отклика за счёт частичной предварительной оценки min/max.
* [Commit 2757](https://github.com/manticoresoftware/columnar/commit/2757b99b) Если слияние дискового чанка прервано, демон теперь очищает временные файлы, связанные с MCL.
* [Commit e9c6](https://github.com/manticoresoftware/manticoresearch/commit/e9c672014b63e2b49e7a5a53880583af86c3ab4e) Версии библиотек Columnar и secondary теперь записываются в лог при сбое.
* [Commit f5e8](https://github.com/manticoresoftware/columnar/commit/f5e84eeb8b6cb102f13e3468246702427527fef9) Добавлена поддержка быстрого перемотки doclist во вторичных индексах.
* [Commit 06df](https://github.com/manticoresoftware/manticoresearch/commit/06dfdd2e668933495dd6cb52d5ba2e0aa9b26e58) Запросы вида `select attr, count(*) from plain_index` (без фильтрации) теперь работают быстрее при использовании MCL.
* [Commit 0a76](https://github.com/manticoresoftware/manticoresearch/commit/0a76ecb9adbe886868b962dd425de5f475bbf55e) @@autocommit в HandleMysqlSelectSysvar для совместимости с .net конектором для mysql выше версии 8.25
* **⚠️ ЛОМАННОЕ ИЗМЕНЕНИЕ**: [Проблема MCL #17](https://github.com/manticoresoftware/columnar/issues/17) MCL: добавлен SSE код для колоночного сканирования. Теперь MCL требует минимум SSE4.2.

### Изменения, связанные с пакетированием
* [Коммит 4d19](https://github.com/manticoresoftware/manticoresearch/commit/4d19f5cbe49a31228aa09253f061165bfe80e51b) **⚠️ КРИТИЧЕСКОЕ ИЗМЕНЕНИЕ**: Поддержка Debian Stretch и Ubuntu Xenial прекращена.
* Поддержка RHEL 9, включая Centos 9, Alma Linux 9 и Oracle Linux 9.
* [Задача #924](https://github.com/manticoresoftware/manticoresearch/issues/924) Поддержка Debian Bookworm.
* [Задача #636](https://github.com/manticoresoftware/manticoresearch/issues/636) Пакетирование: сборки arm64 для Linux и MacOS.
* [PR #26](https://github.com/manticoresoftware/docker/pull/26) Мультиархитектурный (x86_64 / arm64) docker-образ.
* [Упрощённая сборка пакетов для контрибьюторов](Installation/Compiling_from_sources.md#Building-using-CI-Docker).
* Теперь возможно установить конкретную версию с помощью APT.
* [Коммит a6b8](https://github.com/manticoresoftware/manticoresearch/commit/51fddca5c2a3ebb8576fae4c18660656ba22de0f) Инсталлятор для Windows (ранее предоставлялся только архив).
* Переключились на компиляцию с использованием CLang 15.
* **⚠️ КРИТИЧЕСКОЕ ИЗМЕНЕНИЕ**: Пользовательские формулы Homebrew, включая формулу для Manticore Columnar Library. Для установки Manticore, MCL и любых других необходимых компонентов используйте следующую команду `brew install manticoresoftware/manticore/manticoresearch manticoresoftware/manticore/manticore-extra`.

### Исправления ошибок
* [Задача #479](https://github.com/manticoresoftware/manticoresearch/issues/479) Поле с именем `text`
* [Задача #501](https://github.com/manticoresoftware/manticoresearch/issues/501) id не может быть non bigint
* [Задача #646](https://github.com/manticoresoftware/manticoresearch/issues/646) ALTER против поля с именем "text"
* ❗[Задача #652](https://github.com/manticoresoftware/manticoresearch/issues/652) Возможный БАГ: HTTP (JSON) offset и limit влияют на результаты фасетного поиска
* ❗[Задача #827](https://github.com/manticoresoftware/manticoresearch/issues/827) Searchd зависает/крашится при интенсивной нагрузке
* ❗[Задача #996](https://github.com/manticoresoftware/manticoresearch/issues/996) PQ индекс - недостаток памяти
* ❗[Коммит 1041](https://github.com/manticoresoftware/manticoresearch/commit/10416ef7dddf06c0d759e32ccd6ebaa2468f7cbf) `binlog_flush = 1` был сломан всё время с Sphinx. Исправлено.
* [MCL Задача #14](https://github.com/manticoresoftware/columnar/issues/14) MCL: краш при select при слишком большом количестве ft полей
* [Задача #470](https://github.com/manticoresoftware/manticoresearch/issues/470) sql_joined_field не может быть сохранён
* [Задача #713](https://github.com/manticoresoftware/manticoresearch/issues/713) Краш при использовании LEVENSHTEIN()
* [Задача #743](https://github.com/manticoresoftware/manticoresearch/issues/743) Manticore неожиданно крашится и не может нормально перезапуститься
* [Задача #788](https://github.com/manticoresoftware/manticoresearch/issues/788) CALL KEYWORDS через /sql возвращает управляющий символ, нарушающий json
* [Задача #789](https://github.com/manticoresoftware/manticoresearch/issues/789) mariadb не может создать таблицу FEDERATED
* [Задача #796](https://github.com/manticoresoftware/manticoresearch/issues/796) WARNING: dlopen() failed: /usr/bin/lib_manticore_columnar.so: не удаётся открыть файл общего объекта: Нет такого файла или каталога
* [Задача #797](https://github.com/manticoresoftware/manticoresearch/issues/797) Manticore крашится при поиске с ZONESPAN через api
* [Задача #799](https://github.com/manticoresoftware/manticoresearch/issues/799) неверный вес при использовании нескольких индексов и distinct фасета
* [Задача #801](https://github.com/manticoresoftware/manticoresearch/issues/801) SphinxQL group запрос зависает после переработки SQL индекса
* [Задача #802](https://github.com/manticoresoftware/manticoresearch/issues/802) MCL: Индексатор крашится в 5.0.2 и manticore-columnar-lib 1.15.4
* [Задача #813](https://github.com/manticoresoftware/manticoresearch/issues/813) Manticore 5.0.2 FEDERATED возвращает пустой набор (MySQL 8.0.28)
* [Задача #824](https://github.com/manticoresoftware/manticoresearch/issues/824) select COUNT DISTINCT по 2 индексам при результате ноль вызывает внутреннюю ошибку
* [Задача #826](https://github.com/manticoresoftware/manticoresearch/issues/826) КРАШ при запросе DELETE
* [Задача #843](https://github.com/manticoresoftware/manticoresearch/issues/843) MCL: баг с длинным текстовым полем
* [Задача #856](https://github.com/manticoresoftware/manticoresearch/issues/856) 5.0.2 rtindex: Поведение ограничения агрегированного поиска не соответствует ожиданиям
* [Задача #863](https://github.com/manticoresoftware/manticoresearch/issues/863) Возвращается объект Nonetype Hits даже для запросов, которые должны возвращать несколько результатов
* [Задача #870](https://github.com/manticoresoftware/manticoresearch/issues/870) Краш при использовании Attribute и Stored Field в выражении SELECT
* [Задача #872](https://github.com/manticoresoftware/manticoresearch/issues/872) таблица становится невидимой после краша
* [Задача #877](https://github.com/manticoresoftware/manticoresearch/issues/877) Два отрицательных терма в поисковом запросе вызывают ошибку: query is non-computable
* [Задача #878](https://github.com/manticoresoftware/manticoresearch/issues/878) a -b -c не работает через json query_string
* [Задача #886](https://github.com/manticoresoftware/manticoresearch/issues/886) pseudo_sharding с query match
* [Задача #893](https://github.com/manticoresoftware/manticoresearch/issues/893) Manticore 5.0.2 функция min/max не работает как ожидалось...
* [Задача #896](https://github.com/manticoresoftware/manticoresearch/issues/896) Поле "weight" разбирается некорректно
* [Задача #897](https://github.com/manticoresoftware/manticoresearch/issues/897) Сервис Manticore падает при запуске и постоянно перезапускается
* [Задача #900](https://github.com/manticoresoftware/manticoresearch/issues/900) group by j.a, smth работает неправильно
* [Задача #913](https://github.com/manticoresoftware/manticoresearch/issues/913) Краш Searchd при использовании expr в ranker, но только для запросов с двумя proximity
* [Задача #916](https://github.com/manticoresoftware/manticoresearch/issues/916) net_throttle_action сломан
* [Issue #919](https://github.com/manticoresoftware/manticoresearch/issues/919) MCL: Manticore аварийно завершается при выполнении запроса и другие сбои во время восстановления кластера.
* [Issue #925](https://github.com/manticoresoftware/manticoresearch/issues/925) SHOW CREATE TABLE выводит без обратных кавычек
* [Issue #930](https://github.com/manticoresoftware/manticoresearch/issues/930) Теперь возможно выполнять запросы к Manticore из Java через JDBC-коннектор
* [Issue #933](https://github.com/manticoresoftware/manticoresearch/issues/933) Проблемы ранжирования bm25f
* [Issue #934](https://github.com/manticoresoftware/manticoresearch/issues/934) Индексы без конфигурации застывают в watchdog в состоянии первого запуска
* [Issue #937](https://github.com/manticoresoftware/manticoresearch/issues/937) Segfault при сортировке данных фасетов
* [Issue #940](https://github.com/manticoresoftware/manticoresearch/issues/940) сбой при CONCAT(TO_STRING)
* [Issue #947](https://github.com/manticoresoftware/manticoresearch/issues/947) В некоторых случаях один простой SELECT мог вызвать зависание всей инстанции, из-за чего нельзя было войти или выполнить другой запрос, пока выполнялся этот SELECT.
* [Issue #948](https://github.com/manticoresoftware/manticoresearch/issues/948) Сбой индексации
* [Issue #950](https://github.com/manticoresoftware/manticoresearch/issues/950) неправильный подсчет из facet distinct
* [Issue #953](https://github.com/manticoresoftware/manticoresearch/issues/953) LCS неправильно вычисляется в встроенном ранжировщике sph04
* [Issue #955](https://github.com/manticoresoftware/manticoresearch/issues/955) 5.0.3 dev сбой
* [Issue #963](https://github.com/manticoresoftware/manticoresearch/issues/963) FACET с json на движке columnar вызывает сбой
* [Issue #982](https://github.com/manticoresoftware/manticoresearch/issues/982) MCL: 5.0.3 сбой из-за вторичного индекса
* [PR #984](https://github.com/manticoresoftware/manticoresearch/pull/984) @@autocommit в HandleMysqlSelectSysvar
* [PR #985](https://github.com/manticoresoftware/manticoresearch/pull/985) Исправление распределения потоков и чанков в RT индексах
* [Issue #985](https://github.com/manticoresoftware/manticoresearch/issues/985) Исправление распределения потоков и чанков в RT индексах
* [Issue #986](https://github.com/manticoresoftware/manticoresearch/issues/986) неправильный дефолтный max_query_time
* [Issue #987](https://github.com/manticoresoftware/manticoresearch/issues/987) Сбой при использовании регулярного выражения в многопоточном выполнении
* [Issue #988](https://github.com/manticoresoftware/manticoresearch/issues/988) Нарушена обратная совместимость индекса обратного поиска
* [Issue #989](https://github.com/manticoresoftware/manticoresearch/issues/989) indextool выдает ошибку при проверке атрибутов columnar
* [Issue #990](https://github.com/manticoresoftware/manticoresearch/issues/990) Утечка памяти json grouper клонов
* [Issue #991](https://github.com/manticoresoftware/manticoresearch/issues/991) Утечка памяти при клонировании функции levenshtein
* [Issue #992](https://github.com/manticoresoftware/manticoresearch/issues/992) Сообщение об ошибке теряется при загрузке meta
* [Issue #993](https://github.com/manticoresoftware/manticoresearch/issues/993) Пропаганда ошибок из динамических индексов/subkeys и sysvars
* [Issue #994](https://github.com/manticoresoftware/manticoresearch/issues/994) Сбой при count distinct по столбцу строк в columnar хранилище
* [Issue #995](https://github.com/manticoresoftware/manticoresearch/issues/995) MCL: min(pickup_datetime) из taxi1 вызывает сбой
* [Issue #997](https://github.com/manticoresoftware/manticoresearch/issues/997) Пустой excludes JSON запрос удаляет колонки из списка select
* [Issue #998](https://github.com/manticoresoftware/manticoresearch/issues/998) Вторичные задачи, запущенные в текущем планировщике, иногда вызывают аномальные побочные эффекты
* [Issue #999](https://github.com/manticoresoftware/manticoresearch/issues/999) сбой с facet distinct и разными схемами
* [Issue #1000](https://github.com/manticoresoftware/manticoresearch/issues/1000) MCL: Columnar rt индекс повреждён после запуска без columnar библиотеки
* [Issue #1001](https://github.com/manticoresoftware/manticoresearch/issues/1001) не работает неявное cutoff в json
* [Issue #1002](https://github.com/manticoresoftware/manticoresearch/issues/1002) Проблема с columnar grouper
* [Issue #1003](https://github.com/manticoresoftware/manticoresearch/issues/1003) Невозможно удалить последнее поле из индекса
* [Issue #1004](https://github.com/manticoresoftware/manticoresearch/issues/1004) неправильное поведение после --new-cluster
* [Issue #1005](https://github.com/manticoresoftware/manticoresearch/issues/1005) "columnar library not loaded", но она не нужна
* [Issue #1006](https://github.com/manticoresoftware/manticoresearch/issues/1006) нет ошибки для delete запроса
* [Issue #1010](https://github.com/manticoresoftware/manticoresearch/issues/1010) Исправлено расположение файла данных ICU в сборках Windows
* [PR #1018](https://github.com/manticoresoftware/manticoresearch/pull/1018) Проблема при отправке Handshake
* [Issue #1020](https://github.com/manticoresoftware/manticoresearch/issues/1020) Отобразить id в show create table
* [Issue #1024 crash 1](https://github.com/manticoresoftware/manticoresearch/issues/1024) Сбой / Segmentation Fault при фасетном поиске с большим количеством результатов.
* [Issue #1026](https://github.com/manticoresoftware/manticoresearch/issues/1026) RT индекс: searchd "зависает" навсегда при вставке большого количества документов и заполнении RAMchunk
* [Commit 4739](https://github.com/manticoresoftware/manticoresearch/commit/4739dafd) Поток зависает при завершении, если репликация активна между узлами
* [Commit ab87](https://github.com/manticoresoftware/manticoresearch/commit/ab87836782e7ae43fe5f9dea739eed0d336b42c1) Смешивание float и int в JSON range фильтре могло заставить Manticore игнорировать фильтр
* [Commit d001](https://github.com/manticoresoftware/manticoresearch/commit/d00101c2905f6393ce3fca23c4f6dcb2506f4bd9) Фильтры с плавающей точкой в JSON были неточными
* [Commit 4092](https://github.com/manticoresoftware/manticoresearch/commit/4092c6a8) Отбрасывать незакоммиченные txn, если индекс изменён (иначе может быть сбой)
* [Commit 9692](https://github.com/manticoresoftware/manticoresearch/commit/9692a840) Ошибка синтаксиса запроса при использовании обратного слэша
* [Commit 0c19](https://github.com/manticoresoftware/manticoresearch/commit/0c19ad8d) workers_clients мог быть неверным в SHOW STATUS
* [Commit 1772](https://github.com/manticoresoftware/manticoresearch/commit/17720e1e2f2ec38337df565bee78f7ce9d9c744a) исправлен сбой при слиянии ram сегментов без docstores
* [Commit f45b](https://github.com/manticoresoftware/manticoresearch/commit/f45b337b5c265433bb7d6262ad65d02f65edbaf3) Исправлено пропущенное условие ALL/ANY для равенства в JSON фильтре
* [Commit 3e83](https://github.com/manticoresoftware/manticoresearch/commit/3e83c1f0c0e6e7a9d4297113f69a112cbabd9e64) Репликация могла завершиться с ошибкой `got exception while reading ist stream: mkstemp(./gmb_pF6TJi) failed: 13 (Permission denied)`, если searchd был запущен из директории, в которую нельзя записывать.
* [Commit 92e5](https://github.com/manticoresoftware/manticoresearch/commit/92e5629f7150f62524b385fca670a01ba9590f5b) Начиная с версии 4.0.2 журнал сбоев содержал только смещения. Этот коммит исправляет это.

## Версия 5.0.2
Выпущена: 30 мая 2022

### Исправления ошибок
* ❗[Issue #791](https://github.com/manticoresoftware/manticoresearch/issues/791) - неверный размер стека мог вызывать сбой.

## Версия 5.0.0
Выпущена: 18 мая 2022


### Основные новые возможности
* 🔬 Поддержка [Manticore Columnar Library 1.15.2](https://github.com/manticoresoftware/columnar/), которая включает бета-версию [Вторичных индексов](Server_settings/Searchd.md#secondary_indexes). Построение вторичных индексов по умолчанию включено для плоских и реалтаймных как колоннарных, так и строковых индексов (если используется [Manticore Columnar Library](https://github.com/manticoresoftware/columnar)), но для включения поиска по ним требуется установить `secondary_indexes = 1` либо в файле конфигурации, либо через [SET GLOBAL](Server_settings/Setting_variables_online.md). Новая функциональность поддерживается во всех операционных системах, кроме старых Debian Stretch и Ubuntu Xenial.
* [Режим только для чтения](Security/Read_only.md): теперь можно указывать слушатели, которые обрабатывают только запросы на чтение, игнорируя любые записи.
* Новый эндпоинт [/cli](Connecting_to_the_server/HTTP.md#/cli) для ещё более простого выполнения SQL запросов через HTTP.
* Более быстрая пакетная вставка/замена/удаление через JSON по HTTP: ранее можно было передавать несколько команд записи через HTTP JSON протокол, но они обрабатывались последовательно, теперь они обрабатываются как одна транзакция.
* [#720](https://github.com/manticoresoftware/manticoresearch/issues/720) Поддержка [Вложенных фильтров](Searching/Filters.md#Nested-bool-query) в JSON протоколе. Ранее вы не могли описать такие выражения как `a=1 and (b=2 or c=3)` в JSON: `must` (И), `should` (ИЛИ) и `must_not` (НЕ) работали только на верхнем уровне. Теперь их можно вкладывать.
* Поддержка [Chunked transfer encoding](https://en.wikipedia.org/wiki/Chunked_transfer_encoding) в HTTP протоколе. Теперь можно использовать поэтапную передачу в вашем приложении для отправки больших партий с меньшими затратами ресурсов (т.к. не нужно вычислять `Content-Length`). На стороне сервера Manticore теперь всегда обрабатывает входящие HTTP данные потоково, не дожидаясь передачи всей партии, как раньше, что:
  - снижает пиковое использование ОЗУ, уменьшая риск OOM
  - уменьшает время ответа (по нашим тестам, на 11% для обработки партии размером 100 Мб)
  - позволяет обойти ограничение [max_packet_size](Server_settings/Searchd.md#max_packet_size) и передавать партии гораздо больше максимального значения `max_packet_size` (128 Мб), например, по 1 Гб за раз.
* [#719](https://github.com/manticoresoftware/manticoresearch/issues/719) Поддержка в HTTP интерфейсе `100 Continue`: теперь вы можете передавать большие партии из `curl` (включая curl библиотеки для разных языков программирования), которые по умолчанию делают `Expect: 100-continue` и некоторое время ждут перед отправкой партии. Раньше нужно было добавлять заголовок `Expect: `, теперь это не требуется.

  <details>

  Раньше (обратите внимание на время ответа):

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
  {"insert": {"index": "user", "doc":  {"name":"Mr. Johann Smith","email":"stiedemann.tristin@ziemann.com","description":"Temporibus amet magnam consequatur omnis consequatur illo fugit. Debitis natus doloremque est tempore deserunt vero. Harum eos corrupti nemo ut.","age":89,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"Hector Pouros","email":"hickle.mafalda@hotmail.com","description":" as voluptatem inventore sit. Aliquam fugit perferendis est id aut odio et sapiente.","age":64,"active":1}}}'
  *   Пытаемся 127.0.0.1...
  * Подключено к localhost (127.0.0.1) порт 9318 (#0)
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
  * Полная загрузка завершена успешно
  < HTTP/1.1 200 OK
  < Server: 4.2.1 63e5749@220405 dev
  < Content-Type: application/json; charset=UTF-8
  < Content-Length: 147
  <
  * Подключение #0 к хосту localhost оставлено открытым
  {"items":[{"bulk":{"table":"user","_id":2811798919590182916,"created":4,"deleted":0,"updated":0,"result":"created","status":201}}],"errors":false}
  real

0m0.015s

  user
0m0.005s
  sys
0m0.004s
  ```
  </details>

* **⚠️ КРИТИЧЕСКИЕ ИЗМЕНЕНИЯ**: [Псевдо-шардинг](Server_settings/Searchd.md#pseudo_sharding) включен по умолчанию. Если вы хотите отключить его, убедитесь, что добавили `pseudo_sharding = 0` в секцию `searchd` вашего конфигурационного файла Manticore.
* Наличие хотя бы одного полнотекстового поля в индексе реального времени/простом режиме больше не является обязательным. Теперь вы можете использовать Manticore даже в случаях, не связанных с полнотекстовым поиском.
* [Быстрая выборка](Creating_a_table/Data_types.md#fast_fetch) для атрибутов, поддерживаемых [Manticore Columnar Library](https://github.com/manticoresoftware/columnar): запросы типа `select * from <columnar table>` теперь выполняются значительно быстрее, особенно если в схеме много полей.
* **⚠️ КРИТИЧЕСКИЕ ИЗМЕНЕНИЯ**: Неявный [cutoff](Searching/Options.md#cutoff). Теперь Manticore не тратит время и ресурсы на обработку данных, которые не нужны в возвращаемом наборе результатов. Недостаток в том, что это влияет на `total_found` в [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META) и [hits.total](Searching/Full_text_matching/Basic_usage.md#HTTP-JSON) в JSON-выводе. Теперь он точен только если вы видите `total_relation: eq`, в то время как `total_relation: gte` означает, что фактическое количество подходящих документов больше, чем полученное вами значение `total_found`. Чтобы сохранить прежнее поведение, вы можете использовать опцию поиска `cutoff=0`, при которой `total_relation` всегда будет `eq`.
* **⚠️ КРИТИЧЕСКИЕ ИЗМЕНЕНИЯ**: Все полнотекстовые поля теперь по умолчанию [хранятся](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#stored_fields). Чтобы сделать все поля не хранимыми (т.е. вернуть прежнее поведение), нужно использовать `stored_fields = ` (пустое значение).
* [#715](https://github.com/manticoresoftware/manticoresearch/issues/715) HTTP JSON теперь поддерживает [настройки поиска](Searching/Options.md#General-syntax).
### Небольшие изменения
* **⚠️ КРИТИЧЕСКИЕ ИЗМЕНЕНИЯ**: Изменение формата файла метаданных индекса. Ранее мета файлы (`.meta`, `.sph`) были в бинарном формате, теперь это просто JSON. Новая версия Manticore автоматически конвертирует старые индексы, но:
  - вы можете получить предупреждение типа `WARNING: ... syntax error, unexpected TOK_IDENT`
  - вы не сможете запускать индекс в предыдущих версиях Manticore, убедитесь, что у вас есть резервная копия
* **⚠️ КРИТИЧЕСКИЕ ИЗМЕНЕНИЯ**: Поддержка состояния сессии с помощью [HTTP keep-alive](Connecting_to_the_server/HTTP.md#Keep-alive). Это делает HTTP-соединение состоянием, если клиент также его поддерживает. Например, используя новый эндпоинт [/cli](Connecting_to_the_server/HTTP.md#/cli) и HTTP keep-alive (включён по умолчанию во всех браузерах), вы можете вызвать `SHOW META` после `SELECT`, и это будет работать так же, как через mysql. Ранее заголовок HTTP `Connection: keep-alive` тоже поддерживался, но он только вызывал повторное использование того же соединения. С этой версии он также делает сессию состоявшейся.
* Теперь вы можете указать `columnar_attrs = *`, чтобы определить все ваши атрибуты как колоннарные в [простом режиме](Read_this_first.md#Real-time-mode-vs-plain-mode), что полезно, если список длинный.
* Более быстрая репликация SST
* **⚠️ КРИТИЧЕСКИЕ ИЗМЕНЕНИЯ**: Протокол репликации изменён. Если вы используете кластер репликации, при обновлении до Manticore 5 нужно:
  - сначала корректно остановить все узлы
  - затем запустить последний остановленный узел с параметром `--new-cluster` (запустите утилиту `manticore_new_cluster` в Linux).
  - более подробную информацию смотрите в разделе [перезапуск кластера](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster).
* Улучшения репликации:
  - Более быстрая SST
  - Сопротивление шуму, что может помочь в случае нестабильной сети между узлами репликации
  - Улучшенный логгинг
* Улучшение безопасности: Manticore теперь слушает на `127.0.0.1` вместо `0.0.0.0`, если в конфигурации вообще не указан параметр `listen`. Несмотря на то, что в конфигурации по умолчанию, поставляемой с Manticore Search, параметр `listen` указан и типично иметь конфигурацию с явным `listen`, возможность отсутствия этого параметра все еще существует. Ранее Manticore слушал `0.0.0.0`, что небезопасно, теперь он слушает `127.0.0.1`, который обычно не доступен из Интернета.
* Более быстрая агрегация по колоннарным атрибутам.
* Повышенная точность `AVG()`: ранее Manticore использовал `float` для агрегаций, теперь используется `double`, что значительно увеличивает точность.
* Улучшена поддержка JDBC драйвера MySQL.
* Поддержка `DEBUG malloc_stats` для [jemalloc](https://github.com/jemalloc/jemalloc).
* [optimize_cutoff](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#optimize_cutoff) теперь доступен как настройка на уровне таблицы, которую можно задавать при создании (CREATE) или изменении (ALTER) таблицы.
* **⚠️ СУЩЕСТВЕННОЕ ИЗМЕНЕНИЕ:** [query_log_format](Server_settings/Searchd.md#query_log_format) теперь по умолчанию **`sphinxql`**. Если вы привыкли к формату `plain`, вам нужно добавить `query_log_format = plain` в файл конфигурации.
* Значительные улучшения в потреблении памяти: Manticore теперь значительно меньше потребляет оперативной памяти при длительной и интенсивной нагрузке вставки/замены/оптимизации в случае использования сохраненных полей.
* Значение по умолчанию параметра [shutdown_timeout](Server_settings/Searchd.md#shutdown_timeout) увеличено с 3 секунд до 60 секунд.

* [Commit ffd0](https://github.com/manticoresoftware/manticoresearch/commit/ffd0499d329d2c383f14c8a44c4cc84338ab56e7) Поддержка Java mysql connector >= 6.0.3: в [Java mysql connection 6.0.3](https://mvnrepository.com/artifact/mysql/mysql-connector-java/6.0.3) изменили способ подключения к MySQL, что сломало совместимость с Manticore. Теперь новое поведение поддерживается.
* [Commit 1da6](https://github.com/manticoresoftware/manticoresearch/commit/1da6dbec) отключено сохранение нового дискового чанка при загрузке индекса (например, при запуске searchd).
* [Issue #746](https://github.com/manticoresoftware/manticoresearch/issues/746) Поддержка glibc >= 2.34.
* [Issue #784](https://github.com/manticoresoftware/manticoresearch/issues/784) подсчет 'VIP' соединений отдельно от обычных (не-VIP). Ранее VIP-соединения учитывались в лимите `max_connections`, что могло вызывать ошибку "maxed out" для не-VIP соединений. Теперь VIP-соединения не учитываются в лимите. Текущее количество VIP-соединений также отображается в `SHOW STATUS` и `status`.
* [ID](Creating_a_table/Data_types.md#Document-ID) теперь можно задавать явно.
* [Issue #687](https://github.com/manticoresoftware/manticoresearch/issues/687) поддержка сжатия zstd для mysql протокола
### ⚠️ Другие мелкие ломающие изменения
* ⚠️ Формула BM25F была незначительно обновлена для улучшения релевантности поиска. Это влияет только на результаты поиска при использовании функции [BM25F()](Functions/Searching_and_ranking_functions.md#BM25F%28%29), не изменяя поведение формулы ранжирования по умолчанию.
* ⚠️ Изменено поведение REST эндпоинта [/sql](Connecting_to_the_server/HTTP.md#mode=raw): теперь `/sql?mode=raw` требует экранирования и возвращает массив.

* ⚠️ Изменен формат ответа на запросы `/bulk` INSERT/REPLACE/DELETE:
  - ранее каждый подзапрос представлял отдельную транзакцию и возвращал отдельный ответ
  - теперь вся партия рассматривается как одна транзакция и возвращается единый ответ
* ⚠️ Опции поиска `low_priority` и `boolean_simplify` теперь требуют значения (`0/1`): ранее можно было использовать `SELECT ... OPTION low_priority, boolean_simplify`, теперь нужно писать `SELECT ... OPTION low_priority=1, boolean_simplify=1`.
* ⚠️ Если вы используете старые клиенты [php](https://github.com/manticoresoftware/manticoresearch/blob/master/api/sphinxapi.php), [python](https://github.com/manticoresoftware/manticoresearch/blob/master/api/sphinxapi.py) или [java](https://github.com/manticoresoftware/manticoresearch/tree/master/api/java), пожалуйста, перейдите по соответствующей ссылке и найдите обновленную версию. **Старые версии не полностью совместимы с Manticore 5.**
* ⚠️ HTTP JSON запросы теперь логируются в другом формате при `query_log_format=sphinxql`. Ранее логировалась только полнотекстовая часть, теперь логируется вся информация.
### Новые пакеты
* **⚠️ СУЩЕСТВЕННОЕ ИЗМЕНЕНИЕ:** из-за новой структуры при обновлении на Manticore 5 рекомендуется удалить старые пакеты перед установкой новых:
  - для систем на базе RPM: `yum remove manticore*`

  - для Debian и Ubuntu: `apt remove manticore*`
* Новая структура deb/rpm пакетов. Ранее версии предоставляли:
  - `manticore-server` с `searchd` (основным демоном поиска) и всем необходимым для него
  - `manticore-tools` с `indexer` и `indextool`
  - `manticore`, включающий всё
  - `manticore-all` RPM как мета-пакет, ссылающийся на `manticore-server` и `manticore-tools`
  Новая структура:
  - `manticore` — deb/rpm мета-пакет, который устанавливает все выше перечисленное в качестве зависимостей
  - `manticore-server-core` — `searchd` и всё, что нужно для его запуска отдельно
  - `manticore-server` — systemd файлы и другие вспомогательные скрипты

  - `manticore-tools` — `indexer`, `indextool` и другие инструменты
  - `manticore-common` — файл конфигурации по умолчанию, директория данных по умолчанию, стоп-слова по умолчанию
  - `manticore-icudata`, `manticore-dev`, `manticore-converter` мало изменились
  - `.tgz` архив, включающий все пакеты
* Поддержка Ubuntu Jammy
* Поддержка Amazon Linux 2 через [YUM репозиторий](Installation/RHEL_and_Centos.md#YUM-repository)
### Исправления ошибок
* [Issue #815](https://github.com/manticoresoftware/manticoresearch/issues/815) Случайный сбой при использовании UDF функции
* [Issue #287](https://github.com/manticoresoftware/manticoresearch/issues/287) нехватка памяти при индексировании RT индекса
* [Issue #604](https://github.com/manticoresoftware/manticoresearch/issues/604) Критические изменения 3.6.0, 4.2.0 sphinxql-parser
* [Issue #667](https://github.com/manticoresoftware/manticoresearch/issues/667) FATAL: недостаточно памяти (невозможно выделить 9007199254740992 байт)
* [Issue #676](https://github.com/manticoresoftware/manticoresearch/issues/676) Строки некорректно передаются в UDF
* ❗[Issue #698](https://github.com/manticoresoftware/manticoresearch/issues/698) Searchd падает после попытки добавить текстовый столбец в rt индекс
* [Issue #705](https://github.com/manticoresoftware/manticoresearch/issues/705) Индексатор не смог найти все столбцы
* ❗[Issue #709](https://github.com/manticoresoftware/manticoresearch/issues/705) Группировка по json.boolean работает неправильно
* [Issue #716](https://github.com/manticoresoftware/manticoresearch/issues/716) Сбои команд indextool, связанных с индексом (например, --dumpdict)
* ❗[Issue #724](https://github.com/manticoresoftware/manticoresearch/issues/724) Поля пропадают из выборки
* [Issue #727](https://github.com/manticoresoftware/manticoresearch/issues/727) Несовместимость Content-Type с .NET HttpClient при использовании `application/x-ndjson`
* [Issue #729](https://github.com/manticoresoftware/manticoresearch/issues/729) Вычисление длины поля
* ❗[Issue #730](https://github.com/manticoresoftware/manticoresearch/issues/730) create/insert into/drop columnar table вызывает утечку памяти
* [Issue #731](https://github.com/manticoresoftware/manticoresearch/issues/731) Пустой столбец в результатах при определённых условиях
* ❗[Issue #749](https://github.com/manticoresoftware/manticoresearch/issues/749) Краш демона при запуске
* ❗[Issue #750](https://github.com/manticoresoftware/manticoresearch/issues/750) Демон зависает при запуске
* ❗[Issue #751](https://github.com/manticoresoftware/manticoresearch/issues/751) Краш при SST
* [Issue #752](https://github.com/manticoresoftware/manticoresearch/issues/752) Json-атрибут отмечается как columnar, когда engine='columnar'
* [Issue #753](https://github.com/manticoresoftware/manticoresearch/issues/753) Репликация слушает на 0
* [Issue #754](https://github.com/manticoresoftware/manticoresearch/issues/754) columnar_attrs = * не работает с csvpipe
* ❗[Issue #755](https://github.com/manticoresoftware/manticoresearch/issues/755) Краш при выборе float в columnar в rt
* ❗[Issue #756](https://github.com/manticoresoftware/manticoresearch/issues/756) Indextool меняет rt индекс во время проверки
* [Issue #757](https://github.com/manticoresoftware/manticoresearch/issues/757) Требуется проверка пересечений диапазонов портов слушателей
* [Issue #758](https://github.com/manticoresoftware/manticoresearch/issues/758) Логировать оригинальную ошибку в случае сбоя сохранения блока диска RT индекса
* [Issue #759](https://github.com/manticoresoftware/manticoresearch/issues/759) Сообщается только одна ошибка для конфигурации RE2
* ❗[Issue #760](https://github.com/manticoresoftware/manticoresearch/issues/760) Изменения потребления ОЗУ в коммите 5463778558586d2508697fa82e71d657ac36510f
* [Issue #761](https://github.com/manticoresoftware/manticoresearch/issues/761) Третий узел не становится непервичным кластером после грязного рестарта
* [Issue #762](https://github.com/manticoresoftware/manticoresearch/issues/762) Счётчик обновлений увеличивается на 2
* [Issue #763](https://github.com/manticoresoftware/manticoresearch/issues/763) Новая версия 4.2.1 повреждает индекс, созданный с 4.2.0 с использованием морфологии
* [Issue #764](https://github.com/manticoresoftware/manticoresearch/issues/764) Отсутствует экранирование в ключах json /sql?mode=raw
* ❗[Issue #765](https://github.com/manticoresoftware/manticoresearch/issues/765) Использование функции скрывает другие значения
* ❗[Issue #766](https://github.com/manticoresoftware/manticoresearch/issues/766) Утечка памяти, вызванная строкой в FixupAttrForNetwork
* ❗[Issue #767](https://github.com/manticoresoftware/manticoresearch/issues/767) Утечка памяти в 4.2.0 и 4.2.1, связанная с кэшем docstore
* [Issue #768](https://github.com/manticoresoftware/manticoresearch/issues/768) Странный пинг-понг с сохранёнными полями по сети
* [Issue #769](https://github.com/manticoresoftware/manticoresearch/issues/769) lemmatizer_base сбрасывается в пустое значение, если не указан в секции 'common'
* [Issue #770](https://github.com/manticoresoftware/manticoresearch/issues/770) pseudo_sharding замедляет SELECT по id
* [Issue #771](https://github.com/manticoresoftware/manticoresearch/issues/771) DEBUG malloc_stats выводит нули при использовании jemalloc
* [Issue #772](https://github.com/manticoresoftware/manticoresearch/issues/772) Drop/add column делает значение невидимым
* [Issue #773](https://github.com/manticoresoftware/manticoresearch/issues/773) Невозможно добавить столбец bit(N) в columnar таблицу
* [Issue #774](https://github.com/manticoresoftware/manticoresearch/issues/774) "cluster" становится пустым при запуске в manticore.json

* ❗[Commit 1da4](https://github.com/manticoresoftware/manticoresearch/commits/1da4ce891e4ff13727cfd3331d72771c64ee7949) HTTP действия не отслеживаются в SHOW STATUS

* [Commit 3810](https://github.com/manticoresoftware/manticoresearch/commits/381000ab4af4c3c98ec8c730699ad7f39039cec8) отключение pseudo_sharding для низкочастотных запросов с одним ключевым словом
* [Commit 8003](https://github.com/manticoresoftware/manticoresearch/commits/800325cca283a87801b28929d82420fafc76a0ee) исправлена обработка сохранённых атрибутов против слияния индексов

* [Commit cddf](https://github.com/manticoresoftware/manticoresearch/commits/cddfeed6296f2041c6aae18414e5cd9cabf08281) обобщены выборщики уникальных значений; добавлены специализированные выборщики для уникальных значений columnar строк

* [Commit fba4](https://github.com/manticoresoftware/manticoresearch/commits/fba4bb4f) исправлена выборка null целочисленных атрибутов из docstore

* [Commit f300](https://github.com/manticoresoftware/manticoresearch/commit/f3009a9242fa16956adf871c4887e6e0303ba364) `ranker` мог быть указан дважды в журнале запросов
## Версия 4.2.0, 23 декабря 2021

### Основные новые функции
* **Поддержка псевдо-шардинга для индексом реального времени и полнотекстовых запросов**. В предыдущем релизе мы добавили ограниченную поддержку псевдо-шардинга. Начиная с этой версии, вы можете получить все преимущества псевдо-шардинга и вашего многоядерного процессора, просто включив [searchd.pseudo_sharding](Server_settings/Searchd.md#pseudo_sharding). Самое крутое, что вам ничего не нужно делать с вашими индексами или запросами, просто включите его, и если у вас есть свободный ЦП, он будет использоваться для снижения времени отклика. Поддерживаются обычные и индексированные в режиме реального времени индексы для полнотекстовых, фильтрационных и аналитических запросов. Например, вот как включение псевдо-шардинга может сделать среднее время отклика большинства запросов примерно в 10 раз меньше на [Hacker news curated comments dataset](https://zenodo.org/record/45901/), умноженном в 100 раз (116 миллионов документов в обычном индексе).

![Pseudo sharding on vs off in 4.2.0](4.2.0_ps_on_vs_off.png)

* [**Debian Bullseye**](https://manticoresearch.com/install/) теперь поддерживается.

<!-- example pq_transactions_4.2.0 -->
* PQ транзакции теперь атомарны и изолированы. Ранее поддержка PQ транзакций была ограничена. Это обеспечивает значительно **быстрее REPLACE в PQ**, особенно когда необходимо заменить много правил одновременно. Детали производительности:

<!-- intro -->
#### 4.0.2
<!-- request 4.0.2 -->
Вставка 1 миллиона правил PQ занимает **48 секунд**, а операция **REPLACE всего 40 тысяч** выполняется за **406 секунд** в партиях по 10 тысяч.
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
Вставка 1 миллиона правил PQ занимает **34 секунды**, а их **REPLACE — 23 секунды** в партиях по 10 тысяч.
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
*  [optimize_cutoff](Server_settings/Searchd.md#optimize_cutoff) теперь доступен как параметр конфигурации в секции `searchd`. Это полезно, если вы хотите ограничить количество RT-чанков во всех ваших индексах глобально до определенного числа.
* [Коммит 0087](https://github.com/manticoresoftware/manticoresearch/commit/00874743823eaf43e2a09a088392fd7e0a617f47) точный подсчет [count(distinct ...)](Searching/Grouping.md#COUNT%28DISTINCT-field%29) и [FACET ... distinct](Searching/Faceted_search.md#Faceting-without-duplicates) по нескольким локальным физическим индексам (реального времени/обычные) с идентичным набором/порядком полей.
* [PR #598](https://github.com/manticoresoftware/manticoresearch/pull/598) поддержка bigint для `YEAR()` и других функций временных меток.

* [Коммит 8e85](https://github.com/manticoresoftware/manticoresearch/commit/8e85d4bce596cc88f9d9615ea243a27e4afde0f7) Адаптивный [rt_mem_limit](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_mem_limit). Ранее Manticore Search собирал ровно до `rt_mem_limit` данных перед сохранением нового дискового чанка на диск, и во время сохранения собирал еще до 10% больше (т.н. двойной буфер), чтобы минимизировать возможную приостановку вставки. Если это ограничение тоже исчерпывалось, добавление новых документов блокировалось до полного сохранения дискового чанка. Новый адаптивный лимит основан на том, что теперь у нас есть [auto-optimize](Server_settings/Searchd.md#auto_optimize), поэтому немного преждевременное сбрасывание чанка в файл — не проблема. Теперь мы собираем до 50% от `rt_mem_limit` и сохраняем это как дисковый чанк. При сохранении смотрим на статистику (сколько мы сохранили, сколько новых документов поступило во время сохранения) и пересчитываем исходный коэффициент, который будет использоваться в следующий раз. Например, если мы сохранили 90 миллионов документов, и еще 10 миллионов прибыло во время сохранения, коэффициент 90%, значит в следующий раз мы можем собрать до 90% от `rt_mem_limit` перед началом сброса нового дискового чанка. Значение коэффициента рассчитывается автоматически в диапазоне от 33.3% до 95%.
* [Issue #628](https://github.com/manticoresoftware/manticoresearch/issues/628) [unpack_zlib](Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Database_connection.md#unpack_zlib) для PostgreSQL источника. Спасибо, [Dmitry Voronin](https://github.com/dimv36) за [вклад](https://github.com/manticoresoftware/manticoresearch/pull/630).
* [Коммит 6d54](https://github.com/manticoresoftware/manticoresearch/commit/6d54cf2b319b131970c29410cc21abcbdf8083b1) `indexer -v` и `--version`. Ранее вы могли видеть версию индексатора, но `-v`/`--version` не поддерживались.
* [Issue #662](https://github.com/manticoresoftware/manticoresearch/issues/662) неограниченный mlock по умолчанию при запуске Manticore через systemd.
* [Коммит 63c8](https://github.com/manticoresoftware/manticoresearch/commit/63c8cd05cf399e705c2c14939411a92cb784735a) spinlock заменен на op queue для coro rwlock.

* [Коммит 4113](https://github.com/manticoresoftware/manticoresearch/commit/41130ce30d1734bbf8e6b8b63c4e815caf9464f2) переменная окружения `MANTICORE_TRACK_RT_ERRORS`, полезная для отладки повреждений сегментов RT.
### Важные изменения
* Версия binlog была увеличена, binlog из предыдущих версий не будет воспроизводиться, поэтому убедитесь, что вы корректно остановили Manticore Search перед обновлением: в каталоге `/var/lib/manticore/binlog/` не должно остаться binlog-файлов, кроме `binlog.meta` после остановки предыдущего экземпляра.
* [Коммит 3f65](https://github.com/manticoresoftware/manticoresearch/commit/3f659f36e87d99ee262205a8eae4663a255be630) добавлен новый столбец "chain" в `show threads option format=all`. Он показывает стек некоторой информации о задачах, наиболее полезный для профилирования, так что если вы парсите вывод `show threads`, учтите появление этого столбца.
* `searchd.workers` устарел с версии 3.5.0, теперь он помечен как deprecated. Если он все еще есть в вашем конфигурационном файле, при старте будет выдано предупреждение. Manticore Search запустится, но с предупреждением.
* Если вы используете PHP и PDO для доступа к Manticore, необходимо установить `PDO::ATTR_EMULATE_PREPARES`
### Исправления ошибок
* ❗[Issue #650](https://github.com/manticoresoftware/manticoresearch/issues/650) Manticore 4.0.2 медленнее, чем Manticore 3.6.3. Версия 4.0.2 была быстрее предыдущих по массовым вставкам, но значительно медленнее при вставках одиночных документов. Это исправлено в версии 4.2.0.
* ❗[Коммит 22f4](https://github.com/manticoresoftware/manticoresearch/commit/22f4141bed1411c83684ae6e0bae832071cce3df) RT индекс мог повредиться при интенсивной нагрузке REPLACE или мог произойти сбой
* [Коммит 03be](https://github.com/manticoresoftware/manticoresearch/commit/03be91e4f720a3f20bee0278dd8f0d44abd3ef0e) исправлена усреднённая агрегация при объединении группировщиков и групп N сортировщика; исправлено слияние агрегатов
* [Коммит 2ea5](https://github.com/manticoresoftware/manticoresearch/commit/2ea575d34efe38661f14d858f2b552456f29fe42) `indextool --check` мог аварийно завершиться
* [Коммит 7ec7](https://github.com/manticoresoftware/manticoresearch/commit/7ec76d4abf01b5fca6f10a0000fe64d5d8e5fa52) проблема нехватки RAM из-за UPDATE
* [Коммит 658a](https://github.com/manticoresoftware/manticoresearch/commit/658a727e41f3c3fbcde9754149528f34ed76ba19) демон мог зависать при INSERT
* [Коммит 46e4](https://github.com/manticoresoftware/manticoresearch/commit/46e42b9b386e31e53247ed634e158b533e28063e) демон мог зависать при завершении работы
* [Коммит f8d7](https://github.com/manticoresoftware/manticoresearch/commit/f8d7d5177cc83d250b493b4cfc619732b29cf072) демон мог аварийно завершиться при завершении работы
* [Коммит 733a](https://github.com/manticoresoftware/manticoresearch/commit/733accf165142a1d1574689525ccca8907971158) демон мог зависать при сбое
* [Коммит f7f8](https://github.com/manticoresoftware/manticoresearch/commit/f7f8bd8c0dbb23a59a88502dcba16b0918fb2330) демон мог аварийно завершиться при запуске, пытаясь повторно присоединиться к кластеру с недействительным списком узлов
* [Коммит 1401](https://github.com/manticoresoftware/manticoresearch/commit/140155613821ecaddcd1a3dfc4f7369b0df6aef7) распределённый индекс в RT режиме мог быть полностью забыт, если не удавалось разрешить одного из его агентов при запуске
* [Issue #683](https://github.com/manticoresoftware/manticoresearch/issues/683) attr bit(N) engine='columnar' выдаёт ошибку
* [Issue #682](https://github.com/manticoresoftware/manticoresearch/issues/682) create table завершается с ошибкой, но оставляет каталог
* [Issue #663](https://github.com/manticoresoftware/manticoresearch/issues/663) Конфигурация завершается с ошибкой: неизвестное имя ключа 'attr_update_reserve'
* [Issue #632](https://github.com/manticoresoftware/manticoresearch/issues/632) Сбой Manticore при пакетных запросах

* [Issue #679](https://github.com/manticoresoftware/manticoresearch/issues/679) Пакетные запросы снова вызывают сбои в версии v4.0.3

* [Коммит f7f8](https://github.com/manticoresoftware/manticoresearch/commit/f7f8bd8c0dbb23a59a88502dcba16b0918fb2330) исправлен сбой демона при запуске, пытающегося повторно присоединиться к кластеру с недействительным списком узлов
* [Issue #643](https://github.com/manticoresoftware/manticoresearch/issues/643) Manticore 4.0.2 не принимает подключения после пакета вставок
* [Issue #635](https://github.com/manticoresoftware/manticoresearch/issues/635) FACET запрос с ORDER BY JSON.field или строковым атрибутом мог вызвать сбой
* [Issue #634](https://github.com/manticoresoftware/manticoresearch/issues/634) Сбой SIGSEGV на запросе с packedfactors
* [Коммит 4165](https://github.com/manticoresoftware/manticoresearch/commits/41657f15f8fcf1e93cfad7f835bcf705d6e8349e) morphology_skip_fields не поддерживался в create table
## Версия 4.0.2, 21 сентября 2021
### Основные новые возможности
- **Полная поддержка [Manticore Columnar Library](https://github.com/manticoresoftware/columnar)**. Ранее Manticore Columnar Library поддерживалась только для plain индексов. Теперь она поддерживается:
  - в real-time индексах для `INSERT`, `REPLACE`, `DELETE`, `OPTIMIZE`

  - в репликации

  - в `ALTER`
  - в `indextool --check`
- **Автоматическая компакция индексов** ([Issue #478](https://github.com/manticoresoftware/manticoresearch/issues/478)). Наконец-то вам не нужно вызывать OPTIMIZE вручную или через cron-задачи или другую автоматизацию. Manticore теперь делает это за вас автоматически и по умолчанию. Вы можете задать порог компакции по умолчанию через глобальную переменную [optimize_cutoff](Server_settings/Setting_variables_online.md).
- **Переработка системы снимков чанков и блокировок**. Эти изменения могут быть незаметны с первого взгляда, но они значительно улучшают поведение многих операций в real-time индексах. Короче говоря, ранее большинство операций с данными Manticore сильно зависели от блокировок, теперь мы используем снимки чанков на диске.
- **Значительно более быстрая массовая вставка в real-time индекс**. Например, на [сервере Hetzner AX101](https://www.hetzner.com/dedicated-rootserver/ax101) с SSD, 128 ГБ ОЗУ и процессором AMD Ryzen™ 9 5950X (16*2 ядра) **с версией 3.6.0 вы могли получить 236 тыс. документов в секунду** при вставке в таблицу со схемой `name text, email string, description text, age int, active bit(1)` (по умолчанию `rt_mem_limit`, размер батча 25000, 16 одновременных вставок, всего вставлено 16 миллионов документов). В версии 4.0.2 при тех же настройках/конкуренции/количестве достигается **357 тыс. документов в секунду**.
  <details>
  - операции чтения (например SELECT, репликация) выполняются со снимками

  - операции, которые изменяют только внутреннюю структуру индекса без изменения схемы/документов (например слияние RAM сегментов, сохранение дисковых чанков, слияние дисковых чанков), выполняются с использованием только для чтения снимков и в конце заменяют существующие чанки

  - UPDATE и DELETE выполняются по существующим чанкам, но для слияния, которое может происходить, записи собираются и затем применяются к новым чанкам
  - UPDATE получает эксклюзивную блокировку последовательно для каждого чанка. Слияния получают разделяемую блокировку при начале этапа сбора атрибутов из чанка. Таким образом в любой момент к атрибутам чанка может иметь доступ только одна операция (слияние или обновление).

  - когда слияние доходит до фазы, где требуются атрибуты, оно ставит специальный флаг. Когда UPDATE заканчивается, он проверяет этот флаг, и если он установлен, то всё обновление сохраняется в специальной коллекции. В конце, когда слияние завершено, оно применяет набор обновлений к вновь созданному дисковому чанку.
  - ALTER выполняется с эксклюзивной блокировкой
  - репликация выполняется как обычная операция чтения, но дополнительно сохраняет атрибуты до SST и запрещает обновления во время SST
  </details>
- **[ALTER](Updating_table_schema_and_settings.md) может добавлять/удалять полнотекстовое поле** (в режиме RT). Раньше он мог только добавлять/удалять атрибут.


- 🔬 **Экспериментально: псевдо-шардинг для полносканирующих запросов** — позволяет параллелить любые нефулл-текстовые поисковые запросы. Вместо ручной подготовки шардов теперь можно просто включить новый параметр [searchd.pseudo_sharding](Server_settings/Searchd.md#pseudo_sharding) и ожидать снижение времени отклика для нефулл-текстовых поисковых запросов до `CPU cores`. Обратите внимание, что он может полностью задействовать все ядра CPU, поэтому если вам важны не только задержки, но и пропускная способность — используйте с осторожностью.
### Незначительные изменения

<!-- example -->
- Linux Mint и Ubuntu Hirsute Hippo поддерживаются через [APT репозиторий](Installation/Debian_and_Ubuntu.md#APT-repository)
- более быстрое обновление по id через HTTP для больших индексов в некоторых случаях (зависит от распределения ids)

- [671e65a2](https://github.com/manticoresoftware/lemmatizer-uk/commit/671e65a296800537123a3f8b324eeaa3f58b9632) — добавлено кэширование в lemmatizer-uk
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
- [пользовательские флаги запуска для systemd](Starting_the_server/Linux.md#Custom-startup-flags-using-systemd). Теперь не обязательно запускать searchd вручную, если нужно запустить Manticore с каким-либо конкретным флагом запуска

- новая функция [LEVENSHTEIN()](Functions/String_functions.md#LEVENSHTEIN%28%29), вычисляющая расстояние Левенштейна
- добавлены новые [флаги запуска searchd](Starting_the_server/Manually.md#searchd-command-line-options) `--replay-flags=ignore-trx-errors` и `--replay-flags=ignore-all-errors`, чтобы можно было запускать searchd, если binlog повреждён
- [Issue #621](https://github.com/manticoresoftware/manticoresearch/issues/621) — показ ошибок от RE2
- более точный [COUNT(DISTINCT)](Searching/Grouping.md#COUNT%28DISTINCT-field%29) для распределённых индексов, состоящих из локальных обычных индексов
- [FACET DISTINCT](Searching/Faceted_search.md#Faceting-without-duplicates) для удаления дубликатов при фасетном поиске
- [exact form modifier](Searching/Full_text_matching/Operators.md#Exact-form-modifier) теперь не требует [морфологии](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) и работает для индексов с включённым [поиском по инфиксу/префиксу](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology)
### Обратные несовместимости
- новая версия может читать старые индексы, но старые версии не могут читать индексы Manticore 4

- удалена неявная сортировка по id. Сортируйте явно, если требуется
- значение по умолчанию `charset_table` изменилось с `0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451` на `non_cjk`
- `OPTIMIZE` теперь происходит автоматически. Если это не нужно, установите `auto_optimize=0` в секции `searchd` конфигурационного файла
- [Issue #616](https://github.com/manticoresoftware/manticoresearch/issues/616) — `ondisk_attrs_default` устарели и теперь удалены
- для контрибьюторов: теперь для сборки под Linux используется компилятор Clang, так как по результатам тестов он может собрать более быструю версию Manticore Search и Manticore Columnar Library
- если в поисковом запросе не указано [max_matches](Searching/Options.md#max_matches), оно обновляется неявно до минимального необходимого значения для оптимальной производительности нового колоночного хранилища. Это может повлиять на метрику `total` в [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META), но не на `total_found`, что отражает фактическое количество найденных документов.
### Миграция с Manticore 3
- убедитесь, что вы корректно остановили Manticore 3:
  - в `/var/lib/manticore/binlog/` не должно быть binlog файлов (только `binlog.meta` должен присутствовать)

  - иначе индексы, для которых Manticore 4 не сможет воспроизвести binlog, не будут запущены
- новая версия может читать старые индексы, но старые версии не могут читать индексы Manticore 4, поэтому сделайте резервную копию, если хотите иметь возможность быстро откатиться
- если у вас кластер репликации, убедитесь, что:
  - сначала вы корректно остановили все узлы
  - затем последний остановленный узел запускаете с флагом `--new-cluster` (на Linux используйте инструмент `manticore_new_cluster`)
  - подробнее о [перезапуске кластера](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster)
### Исправления ошибок
- Исправлено множество проблем с репликацией:
  - [Коммит 696f](https://github.com/manticoresoftware/manticoresearch/commit/696f8649535f2b5285b878c9581c0d2b1139ae09) — исправлен краш во время SST на узле joiner с активным индексом; добавлена проверка sha1 на joiner при записи чанков файлов для ускорения загрузки индекса; добавлена ротация изменённых файлов индекса на joiner при загрузке; добавлено удаление файлов индекса на joiner при замене активного индекса новым с узла донора; добавлены точки журнала репликации на узле донора для отправки файлов и чанков
  - [Коммит b296](https://github.com/manticoresoftware/manticoresearch/commit/b296c55af68314a8fc66f8b104e7301d673c3b68) — краш при JOIN CLUSTER, если адрес неправильный
  - [Коммит 418b](https://github.com/manticoresoftware/manticoresearch/commit/418bf880a5e05ffbb68f311a1c23006a9df3220e) — во время начальной репликации большого индекса узел присоединения мог падать с `ERROR 1064 (42000): invalid GTID, (null)`, донор мог переставать отвечать, пока кластера присоединялся другой узел
  - [Коммит 6fd3](https://github.com/manticoresoftware/manticoresearch/commit/6fd350d28e4905211cf8a7298cbf7cdda101c83d) — хэш мог неправильно считаться для большого индекса, что могло приводить к сбою репликации
  - [Issue #615](https://github.com/manticoresoftware/manticoresearch/issues/615) — сбой репликации при перезапуске кластера
- [Issue #574](https://github.com/manticoresoftware/manticoresearch/issues/574) — `indextool --help` не показывает параметр `--rotate`
- [Issue #578](https://github.com/manticoresoftware/manticoresearch/issues/578) — высокая загрузка CPU у searchd в простое примерно через сутки
- [Issue #587](https://github.com/manticoresoftware/manticoresearch/issues/587) — немедленный сброс .meta
- [Issue #617](https://github.com/manticoresoftware/manticoresearch/issues/617) - manticore.json опустошается
- [Issue #618](https://github.com/manticoresoftware/manticoresearch/issues/618) - searchd --stopwait не работает под root. Также исправлено поведение systemctl (ранее он показывал ошибку для ExecStop и не ждал достаточно долго, чтобы searchd завершился корректно)
- [Issue #619](https://github.com/manticoresoftware/manticoresearch/issues/619) - INSERT/REPLACE/DELETE в сравнении с SHOW STATUS. `command_insert`, `command_replace` и другие показывали неверные метрики
- [Issue #620](https://github.com/manticoresoftware/manticoresearch/issues/620) - `charset_table` для plain index имел неправильное значение по умолчанию
- [Commit 8f75](https://github.com/manticoresoftware/manticoresearch/commit/8f7536887d4b58a5bab5647fc067b2d0482c2038) - новые disk chunks не блокируются в памяти через mlock
- [Issue #607](https://github.com/manticoresoftware/manticoresearch/issues/607) - узел Manticore cluster падает при невозможности разрешить узел по имени


- [Issue #623](https://github.com/manticoresoftware/manticoresearch/issues/623) - репликация обновлённого индекса может привести к неопределённому состоянию
- [Commit ca03](https://github.com/manticoresoftware/manticoresearch/commit/ca03d2280c5197e9b311f0f03f551e3b702a8130) - indexer мог зависать при индексации plain index source с json атрибутом

- [Commit 53c7](https://github.com/manticoresoftware/manticoresearch/commit/53c753054558ff3f82c7af365273d32537c5338b) - исправлен фильтр выражения неравенства в PQ index
- [Commit ccf9](https://github.com/manticoresoftware/manticoresearch/commit/ccf94e022ba6763d6286ab5bac767b6f6a846846) - исправлен выбор окон при list запросах с более чем 1000 совпадениями. `SELECT * FROM pq ORDER BY id desc LIMIT 1000 , 100 OPTION max_matches=1100` ранее не работал
- [Commit a048](https://github.com/manticoresoftware/manticoresearch/commit/a0483fe9bef3c45dffe96f5df4f4382aee29c072) - HTTPS запрос к Manticore мог вызывать предупреждение типа "max packet size(8388608) exceeded"
- [Issue #648](https://github.com/manticoresoftware/manticoresearch/issues/648) - Manticore 3 мог зависать после нескольких обновлений строковых атрибутов

## Version 3.6.0, May 3rd 2021
**Релиз с исправлениями перед Manticore 4**
### Основные новые возможности
- Поддержка [Manticore Columnar Library](https://github.com/manticoresoftware/columnar/) для plain индексов. Новая настройка [columnar_attrs](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#columnar_attrs) для plain индексов
- Поддержка [украинского лемматизатора](https://github.com/manticoresoftware/lemmatizer-uk)
- Полностью переработанные гистограммы. При построении индекса Manticore также строит гистограммы для каждого поля, которые затем используются для более быстрого фильтрования. В версии 3.6.0 алгоритм был полностью переработан, что позволяет получать более высокую производительность при большом объёме данных и большом количестве фильтров.
### Небольшие изменения
- утилита `manticore_new_cluster [--force]`, полезная для перезапуска кластера репликации через systemd
- [--drop-src](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Merging_tables.md#Merging-tables) для `indexer --merge`

- [новый режим](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) `blend_mode='trim_all'`
- добавлена [поддержка экранирования JSON path](Searching/Full_text_matching/Escaping.md#Escaping-json-node-names-in-SQL) с помощью обратных кавычек
- [indextool --check](Miscellaneous_tools.md#indextool) теперь может работать в RT режиме

- [FORCE/IGNORE INDEX(id)](Data_creation_and_modification/Updating_documents/UPDATE.md#Updates-via-SQL) для SELECT/UPDATE
- id чанка для слитого disk chunk теперь уникален
- [indextool --check-disk-chunk CHUNK_NAME](Miscellaneous_tools.md#indextool)
### Оптимизации
- [ускоренный парсинг JSON](https://github.com/manticoresoftware/manticoresearch/commit/0c25fc1788e3f77f60cb7fb8f1623d8466bba606), наши тесты показывают снижение задержки на 3-4% для запросов типа `WHERE json.a = 1`
- не документированная команда `DEBUG SPLIT` как предпосылка для автоматического шардирования/ребалансировки
### Исправления ошибок
- [Issue #584](https://github.com/manticoresoftware/manticoresearch/issues/524) - неточные и нестабильные результаты FACET
- [Issue #506](https://github.com/manticoresoftware/manticoresearch/issues/506) - Странное поведение при использовании MATCH: пострадавшим от этой проблемы нужно пересобрать индекс, так как проблема возникала на этапе построения индекса
- [Issue #387](https://github.com/manticoresoftware/manticoresearch/issues/387) - периодический сбой с core dump при выполнении запроса с функцией SNIPPET()
- Оптимизации стека, полезные для обработки сложных запросов:
  - [Issue #469](https://github.com/manticoresoftware/manticoresearch/issues/469) - SELECT вызывал CRASH DUMP
  - [e8420cc7](https://github.com/manticoresoftware/manticoresearch/tree/e8420cc789149c84731be0874be916e33b263bd6) - определение размера стека для деревьев фильтров
- [Issue #461](https://github.com/manticoresoftware/manticoresearch/issues/461) - Обновление с использованием условия IN не применялось корректно
- [Issue #464](https://github.com/manticoresoftware/manticoresearch/issues/464) - SHOW STATUS сразу после CALL PQ возвращает - [Issue #481](https://github.com/manticoresoftware/manticoresearch/issues/481) - Исправлена статическая сборка бинарника
- [Issue #502](https://github.com/manticoresoftware/manticoresearch/issues/502) - ошибка в multi-запросах
- [Issue #514](https://github.com/manticoresoftware/manticoresearch/issues/514) - Невозможно использовать необычные имена для колонок при создании таблицы с 'create table'
- [Commit d1db](https://github.com/manticoresoftware/manticoresearch/commit/d1dbe771288328c963ca8512f564c6b7cf4f466c) - сбой демона при воспроизведении binlog с обновлением строкового атрибута; версия binlog установлена в 10
- [Commit 775d](https://github.com/manticoresoftware/manticoresearch/commit/775d0555562f121911d3c30085947e743fadac2d) - исправлено время выполнения определения кадра стека выражений (тест 207)
- [Commit 4795](https://github.com/manticoresoftware/manticoresearch/commit/4795dc49194c7745761bb5006ecb266e8a9224fa) - перколятор индексного фильтра и теги были пусты для пустого сохранённого запроса (тест 369)
- [Commit c3f0](https://github.com/manticoresoftware/manticoresearch/commit/c3f0bf4dca0e4eae6ea899dd566878df57eba085)  - нарушения потока репликации SST в сети с большой задержкой и высоким уровнем ошибок (репликация между различными дата-центрами); обновлена версия команды репликации до 1.03
- [Commit ba2d](https://github.com/manticoresoftware/manticoresearch/commit/ba2d6619b8107f3fb177741bbdff3709bac248cd)  - блокировка joiner кластера при операциях записи после присоединения к кластеру (тест 385)
- [Commit de4d](https://github.com/manticoresoftware/manticoresearch/commit/de4dcb9ff6c316bc2a9ef6c62eb64769f04ee103)  - совпадение с подстановочными знаками с точным модификатором (тест 321)
- [Commit 6524](https://github.com/manticoresoftware/manticoresearch/commit/6524fc6af032a0932bd42c37874a619e1fb7f075)  - контрольные точки docid vs docstore
- [Commit f4ab](https://github.com/manticoresoftware/manticoresearch/commit/f4ab83c291e65a5873d271c67b5491f629544f1d)  - Несогласованное поведение индексатора при разборе некорректного xml
- [Commit 7b72](https://github.com/manticoresoftware/manticoresearch/commit/7b727e22fe01b1a180d4a8d3a3fea92c18eac707)  - Хранящийся перколятор запрос с NOTNEAR выполняется бесконечно (тест 349)
- [Commit 812d](https://github.com/manticoresoftware/manticoresearch/commit/812dab745f0c972ecde5d5b3d4a313cd6e1e143e)  - неправильный вес для фразы, начинающейся с подстановочного знака
- [Commit 1771](https://github.com/manticoresoftware/manticoresearch/commit/1771afc669c75670b3511bf02ab9bfc8982b94d2)  - перколятор запрос с подстановочными знаками генерирует термины без payload при совпадении, что вызывает перемешанные попадания и нарушает совпадение (тест 417)
- [Commit aa0d](https://github.com/manticoresoftware/manticoresearch/commit/aa0d8c2bcb41428d41dbfb66e42a872a79a4ee4b)  - исправлен расчет 'total' в случае параллельного выполнения запроса
- [Commit 18d8](https://github.com/manticoresoftware/manticoresearch/commit/18d81b3cfbb930b7361dae645fe13283d4fbf6ef)  - сбой в Windows при множественных одновременных сессиях в демоне
- [Commit 8443](https://github.com/manticoresoftware/manticoresearch/commit/84432f23d6fbf7a7d0b67c8c9412eb5933b03596)  - некоторые настройки индекса не могли реплицироваться
- [Commit 9341](https://github.com/manticoresoftware/manticoresearch/commit/93411fe6af4246ca4acce758944d513a1660b92b)  - При высокой скорости добавления новых событий netloop иногда зависает из-за того, что атомарное событие 'kick' обрабатывается один раз для нескольких событий одновременно и теряет реальные действия из них
статус запроса, а не статус сервера
- [Commit d805](https://github.com/manticoresoftware/manticoresearch/commit/d805fc125244ef08d14fb3d1826e2c356862d539)  - Новый сброшенный фрагмент диска может быть утерян при фиксации

- [Commit 63cb](https://github.com/manticoresoftware/manticoresearch/commit/63cbf008d50979cdf7062664fb02a672a3bb1131)  - неточная 'net_read' в профилировщике
- [Commit f537](https://github.com/manticoresoftware/manticoresearch/commit/f5379bb27234b25919966c50ccb6f469ead646e2)  - Проблема с перколятором для арабского (тексты справа налево)
- [Commit 49ee](https://github.com/manticoresoftware/manticoresearch/commit/49eeb4202d4ce4875c07c35087d7108c1e177328)  - id неправильно выбирается при дублировании имени колонки
- [Commit refa](https://github.com/manticoresoftware/manticoresearch/commit/4b7df262bbfec9b9e95a44e72ad326ac8a786c49)  сетевые события для исправления сбоя в редких случаях
- [e8420cc7](https://github.com/manticoresoftware/manticoresearch/tree/e8420cc789149c84731be0874be916e33b263bd6) исправление в `indextool --dumpheader`
- [Commit ff71](https://github.com/manticoresoftware/manticoresearch/commit/ff7163532ed39042c0d3c3c2b902ff41f94d7f0c) - TRUNCATE WITH RECONFIGURE работает неправильно с хранимыми полями

### Ломающие изменения:
- Новый формат binlog: необходимо корректно остановить Manticore перед обновлением
- Формат индекса слегка изменён: новая версия без проблем читает существующие индексы, но если вы решите откатиться с 3.6.0 на более старую версию, новые индексы станут нечитаемы

- Изменение формата репликации: не реплицируйте с более старой версии в 3.6.0 и обратно, переключайтесь на новую версию одновременно на всех узлах

- `reverse_scan` устарел. Убедитесь, что не используете эту опцию в своих запросах с версии 3.6.0, иначе они будут ошибочными
- С этого релиза мы больше не предоставляем сборки для RHEL6, Debian Jessie и Ubuntu Trusty. Если их поддержка критически важна для вас, [свяжитесь с нами](https://manticoresearch.com/contact-us/)
### Устаревшие возможности
- Больше нет неявной сортировки по id. Если вы на неё полагались, убедитесь, что обновили свои запросы соответственно

- Опция поиска `reverse_scan` устарела
## Версия 3.5.4, 10 декабря 2020
### Новые возможности

- Новые клиенты для Python, Javascript и Java теперь доступны в общем доступе и хорошо документированы в этом руководстве.
- автоматическое удаление дискового фрагмента real-time индекса. Эта оптимизация позволяет автоматически удалять дисковый фрагмент при [OPTIMIZING](Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE) real-time индекса, если фрагмент явно больше не нужен (все документы удалены). Ранее необходимо было сделать слияние, теперь фрагмент можно просто удалить мгновенно. Опция [cutoff](Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks) игнорируется, то есть даже если ничего не объединяется, устаревший дисковый фрагмент удаляется. Это полезно, если вы поддерживаете хранение в индексе и удаляете старые документы. Теперь компактификация таких индексов будет быстрее.
- [standalone NOT](Searching/Options.md#not_terms_only_allowed) как опция для SELECT

### Незначительные изменения
- [Issue #453](https://github.com/manticoresoftware/manticoresearch/issues/453) Новая опция [indexer.ignore_non_plain=1](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#ignore_non_plain) полезна, если вы запускаете `indexer --all` и у вас есть не только plain индексы в файле конфигурации. Без `ignore_non_plain=1` будет предупреждение и соответствующий код выхода.
- [SHOW PLAN ... OPTION format=dot](Node_info_and_management/Profiling/Query_plan.md#Dot-format-for-SHOW-PLAN) и [EXPLAIN QUERY ... OPTION format=dot](Searching/Full_text_matching/Profiling.md#Profiling-without-running-a-query) позволяют визуализировать выполнение плана запроса полнотекстового поиска. Полезно для понимания сложных запросов.
### Устаревания
- `indexer --verbose` устарел, так как никогда не добавлял ничего к выводу индексатора
- Для дампа стека сигнал watchdog теперь необходимо использовать `USR2` вместо `USR1`
### Исправления ошибок
- [Issue #423](https://github.com/manticoresoftware/manticoresearch/issues/423) кириллический символ точка в вызовах snippets в режиме retain не подсвечивается
- [Issue #435](https://github.com/manticoresoftware/manticoresearch/issues/435) RTINDEX - GROUP N BY выражение select = критический сбой
- [Commit 2b3b](https://github.com/manticoresoftware/manticoresearch/commit/2b3b62bd0ce2b54544733d524a169e81f71b5388) статус searchd показывает Segmentation fault в кластере


- [Commit 9dd2](https://github.com/manticoresoftware/manticoresearch/commit/9dd25c193ed2a53750a0e0e68566fd2b833fcb0c) 'SHOW INDEX index.N SETTINGS' не обрабатывает чанки >9

- [Issue #389](https://github.com/manticoresoftware/manticoresearch/issues/389) Ошибка, приводящая к сбою Manticore

- [Commit fba1](https://github.com/manticoresoftware/manticoresearch/commit/fba166178ff35c73a0c70d131cf3aca4a635e961) Конвертер создает сломанные индексы
- [Commit eecd](https://github.com/manticoresoftware/manticoresearch/commit/eecd61d861c742471175be19a5e5455034b5db5f) stopword_step=0 против CALL SNIPPETS()
- [Commit ea68](https://github.com/manticoresoftware/manticoresearch/commit/ea6850e449601199190f38a387c04b8a7b7e90f6) count distinct возвращает 0 при низком max_matches на локальном индексе
- [Commit 362f](https://github.com/manticoresoftware/manticoresearch/commit/362f27db3421f09a41c6cce04b5baace00fa4c4d) При использовании агрегации сохранённые тексты не возвращаются в результатах
## Версия 3.5.2, 1 окт 2020
### Новые возможности

* OPTIMIZE уменьшает количество дисковых чанков до определенного числа (по умолчанию `2* число ядер`), а не до одного. Оптимальное число чанков можно контролировать опцией [cutoff](Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks).

* Оператор NOT теперь можно использовать самостоятельно. По умолчанию он отключён, так как случайные одиночные запросы NOT могут быть медленными. Его можно включить, установив новую директиву searchd [not_terms_only_allowed](Server_settings/Searchd.md#not_terms_only_allowed) в значение `0`.
* Новая настройка [max_threads_per_query](Server_settings/Searchd.md#max_threads_per_query) задаёт, сколько потоков может использовать запрос. Если директива не установлена, запрос может использовать потоки до значения [threads](Server_settings/Searchd.md#threads).
Для каждого запроса SELECT количество потоков можно ограничить опцией [OPTION threads=N](Searching/Options.md#threads), переопределяя глобальный max_threads_per_query.
* Индексы percolate теперь можно импортировать через [IMPORT TABLE](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Importing_table.md).

* HTTP API `/search` получает базовую поддержку [фасетинга](Searching/Faceted_search.md#HTTP-JSON)/[группировки](Searching/Grouping.md) с помощью нового узла запроса `aggs`.

### Мелкие изменения

* Если директива прослушивания репликации не задана, движок будет пытаться использовать порты после заданного порта 'sphinx', вплоть до 200.

* `listen=...:sphinx` должен быть явно установлен для соединений SphinxSE или клиентов SphinxAPI.

* [SHOW INDEX STATUS](Node_info_and_management/Table_settings_and_status/SHOW_TABLE_STATUS.md) выводит новые метрики: `killed_documents`, `killed_rate`, `disk_mapped_doclists`, `disk_mapped_cached_doclists`, `disk_mapped_hitlists` и `disk_mapped_cached_hitlists`.

* SQL команда `status` теперь выводит `Queue\Threads` и `Tasks\Threads`.

### Устаревания:
* `dist_threads` полностью устарел, searchd будет выдавать предупреждение при его использовании.
### Docker
Официальный Docker-образ теперь основан на Ubuntu 20.04 LTS
### Распределение
Помимо обычного пакета `manticore`, вы можете также устанавливать Manticore Search по компонентам:

- `manticore-server-core` - обеспечивает `searchd`, man-страницу, каталог логов, API и модуль galera. Также установит `manticore-common` как зависимость.

- `manticore-server` - предоставляет скрипты автоматизации для ядра (init.d, systemd) и обёртку `manticore_new_cluster`. Также установит `manticore-server-core` как зависимость.
- `manticore-common` - содержит конфигурацию, стоп-слова, общие документы и скелетные папки (datadir, modules и т.д.)
- `manticore-tools` - предоставляет вспомогательные инструменты ( `indexer`, `indextool` и т.д.), их man-страницы и примеры. Также установит `manticore-common` как зависимость.
- `manticore-icudata` (RPM) или `manticore-icudata-65l` (DEB) - содержит файл данных ICU для использования морфологии icu.
- `manticore-devel` (RPM) или `manticore-dev` (DEB) - содержит заголовочные файлы для UDF.
### Исправления ошибок
1. [Commit 2a47](https://github.com/manticoresoftware/manticoresearch/commit/2a474dc1a26e8b0f8aaaae95669caf2f1d4b7746) Сбой демона при grouper в RT индексе с разными чанками
2. [Commit 57a1](https://github.com/manticoresoftware/manticoresearch/commit/57a19e5ad5663ef0ca7436595218fb1221d28c8e) Быстрый путь для пустых удалённых документов
3. [Commit 07dd](https://github.com/manticoresoftware/manticoresearch/commit/07dd3f313c63fb82c22092f9907ef24e3475250e) Определение стека выражений во время выполнения
4. [Commit 08ae](https://github.com/manticoresoftware/manticoresearch/commit/08ae357cf1012bc8e2da54c20b205b592efda3d4) Сопоставление более 32 полей в percolate индексах
5. [Commit 16b9](https://github.com/manticoresoftware/manticoresearch/commit/16b9390fd4cdb07a77ac4497adb935573a1710e5) Диапазон портов для прослушивания репликации
6. [Commit 5fa6](https://github.com/manticoresoftware/manticoresearch/commit/5fa671affeacb6441a59c8a88479bfd423df7c81) Показывать create table на pq
7. [Commit 54d1](https://github.com/manticoresoftware/manticoresearch/commit/54d133b6449105a9fb0168db3f1fbb05fb5aa1f6) Поведение HTTPS-порта
8. [Коммит fdbb](https://github.com/manticoresoftware/manticoresearch/commit/fdbbe5245cc296cc5c1ae3ae2fb9cb08fb66a248) Перемешивание строк docstore при замене
9. [Коммит afb5](https://github.com/manticoresoftware/manticoresearch/commit/afb53f648ef4e64fa1776c58e66f6e716ac730ad) Смена уровня сообщения TFO при недоступности на 'info'
10. [Коммит 59d9](https://github.com/manticoresoftware/manticoresearch/commit/59d94cefc6e88af360b8046726a16b7eaa9f0b71) Сбой при неправильном использовании strcmp
11. [Коммит 04af](https://github.com/manticoresoftware/manticoresearch/commit/04af0349c1e784c2230d204592d6272b440bb375) Добавление индекса в кластер с системными файлами (стоп-слова)
12. [Коммит 5014](https://github.com/manticoresoftware/manticoresearch/commit/50148b4eae9f3a924082005cf5d2f9c8339af7f5) Слияние индексов с большими словарями; оптимизация RT больших дисковых чанков
13. [Коммит a2ad](https://github.com/manticoresoftware/manticoresearch/commit/a2adf1582843f63d4dcc1dab8f7598e0917094a2) Indextool может выгружать метаданные из текущей версии

14. [Коммит 69f6](https://github.com/manticoresoftware/manticoresearch/commit/69f6d5f77c64b97e3f395aea8c555b11c77fb732) Проблема с порядком группировки в GROUP N

15. [Коммит 24d5](https://github.com/manticoresoftware/manticoresearch/commit/24d5d80f508576794b751d70f9f9b524564e3c14) Явная очистка для SphinxSE после рукопожатия
16. [Коммит 31c4](https://github.com/manticoresoftware/manticoresearch/commit/31c4d78ac03ee85d9a02b86075448a04ac80f9e2) Избегание копирования больших описаний, когда это не нужно
17. [Коммит 2959](https://github.com/manticoresoftware/manticoresearch/commit/2959e2caa8610f98fcdf2e5a82160f58687f4a27) Отрицательное время в show threads
18. [Коммит f0b3](https://github.com/manticoresoftware/manticoresearch/commit/f0b357102c70f3ac1d7da260c60b67322940e647) Плагин фильтра токенов против нулевых дельт позиций
19. [Коммит a49e](https://github.com/manticoresoftware/manticoresearch/commit/a49e5bc13ea763e6f01dd5993e891858a84d1846) Изменение 'FAIL' на 'WARNING' при множественных совпадениях
## Версия 3.5.0, 22 июля 2020
### Основные новые возможности:

* Этот релиз занял много времени, потому что мы усиленно работали над изменением режима многозадачности с потоков на **кооперативные корутины**. Это упрощает конфигурацию и делает параллелизацию запросов гораздо более прямолинейной: Manticore просто использует заданное количество потоков (см. новую настройку [threads](Server_settings/Searchd.md#threads)), а новый режим обеспечивает выполнение этого самым оптимальным образом.

* Изменения в [выделении подсветки](Searching/Highlighting.md#Highlighting-options):

  - любая подсветка, работающая с несколькими полями (`highlight({},'field1, field2'`) или `highlight` в JSON-запросах), теперь по умолчанию применяет лимиты по полям.

  - любая подсветка, работающая с обычным текстом (`highlight({}, string_attr)` или `snippet()`), теперь применяет лимиты ко всему документу.
  - [лимиты по полям](Searching/Highlighting.md#limits_per_field) можно переключить на глобальные лимиты опцией `limits_per_field=0` (по умолчанию `1`).
  - [allow_empty](Searching/Highlighting.md#allow_empty) теперь по умолчанию равно `0` для подсветки через HTTP JSON.
* Теперь тот же порт [можно использовать](Server_settings/Searchd.md#listen) для http, https и бинарного API (чтобы принимать соединения от удалённого экземпляра Manticore). Для соединений по протоколу mysql все ещё требуется `listen = *:mysql`. Manticore теперь автоматически определяет тип клиента, пытающегося подключиться, за исключением MySQL (из-за ограничений протокола).
* В режиме RT поле теперь может быть одновременно [текстовым и строковым атрибутом](Creating_a_table/Data_types.md#String) — [задача на GitHub #331](https://github.com/manticoresoftware/manticoresearch/issues/331).
  В [простом режиме](Read_this_first.md#Real-time-mode-vs-plain-mode) это называется `sql_field_string`. Теперь доступно и в [RT режиме](Read_this_first.md#Real-time-mode-vs-plain-mode) для индексов реального времени. Вы можете использовать это, как показано в примере:
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
* Теперь вы можете [подсвечивать строковые атрибуты](Searching/Highlighting.md#Highlighting-via-SQL).
* Поддержка SSL и сжатия для SQL интерфейса
* Поддержка команды mysql клиента [`status`](Node_info_and_management/Node_status.md#STATUS).
* [Репликация](Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Setting-up-replication) теперь может реплицировать внешние файлы (стоп-слова, исключения и прочее).
* Оператор фильтра [`in`](Searching/Filters.md#Set-filters) теперь доступен через HTTP JSON интерфейс.
* [`expressions`](Searching/Expressions.md#expressions) в HTTP JSON.
* Теперь можно менять [rt_mem_limit на лету](https://github.com/manticoresoftware/manticoresearch/issues/344) в режиме RT, т.е. делать `ALTER ... rt_mem_limit=<новое значение>`.

* Можно использовать [отдельные таблицы символов для CJK](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology): `chinese`, `japanese` и `korean`.
* [thread_stack](Server_settings/Searchd.md#thread_stack) теперь ограничивает максимальный стек потока, а не стартовый.
* Улучшен вывод `SHOW THREADS`.
* Показ прогресса долгого `CALL PQ` в `SHOW THREADS`.
* cpustat, iostat, coredump можно менять во время работы через [SET](Server_settings/Setting_variables_online.md#SET).

* Реализован `SET [GLOBAL] wait_timeout=NUM`,
### Изменения, нарушающие обратную совместимость:
* **Формат индекса изменён.** Индексы, построенные в 3.5.0, не могут быть загружены версиями Manticore ниже 3.5.0, но Manticore 3.5.0 понимает более старые форматы.
* [`INSERT INTO PQ VALUES()`](Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md) (т.е. без указания списка столбцов) ранее ожидал именно `(query, tags)` в качестве значений. Теперь это изменено на `(id,query,tags,filters)`. Id можно задать 0, если хотите, чтобы он генерировался автоматически.
* [`allow_empty=0`](Searching/Highlighting.md#allow_empty) — новое значение по умолчанию для подсветки через HTTP JSON интерфейс.
* Для внешних файлов (стоп-слов, исключений и т.п.) в `CREATE TABLE`/`ALTER TABLE` разрешены только абсолютные пути.
### Устаревшие возможности:
* `ram_chunks_count` переименован в `ram_chunk_segments_count` в `SHOW INDEX STATUS`.
* `workers` устарел. Теперь режим работы только один.
* `dist_threads` устарел. Все запросы выполняются максимально параллельно (ограничено параметрами `threads` и `jobs_queue_size`).
* `max_children` устарел. Используйте [threads](Server_settings/Searchd.md#threads) для установки количества потоков, которые будет использовать Manticore (по умолчанию число ядер процессора).
* `queue_max_length` устарел. Вместо него используйте [jobs_queue_size](Server_settings/Searchd.md#jobs_queue_size) для точной настройки внутренней очереди заданий (по умолчанию без ограничений).
* Все эндпоинты `/json/*` теперь доступны без `/json/`, например `/search`, `/insert`, `/delete`, `/pq` и т.д.
* Значение `field` в смысле «полнотекстовое поле» переименовано в «text» в describe.
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
* Кириллическая `и` не маппится в `i` в `non_cjk` charset_table (который используется по умолчанию), так как это сильно влияло на русские стеммеры и лемматизаторы.
* `read_timeout`. Используйте вместо него [network_timeout](Server_settings/Searchd.md#network_timeout), который контролирует и чтение, и запись.
### Пакеты
* Официальный пакет для Ubuntu Focal 20.04
* Имя deb-пакета изменилось с `manticore-bin` на `manticore`
### Исправления ошибок:
1. [Issue #351](https://github.com/manticoresoftware/manticoresearch/issues/351) утечка памяти в searchd
2. [Commit ceab](https://github.com/manticoresoftware/manticoresearch/commit/ceabe44f) Небольшое чтение за пределами буфера в сниппетах
3. [Commit 1c3e](https://github.com/manticoresoftware/manticoresearch/commit/1c3e84a3) Опасная запись в локальную переменную для аварийных запросов
4. [Commit 26e0](https://github.com/manticoresoftware/manticoresearch/commit/26e094ab) Небольшая утечка памяти сортировщика в тесте 226
5. [Commit d2c7](https://github.com/manticoresoftware/manticoresearch/commit/d2c7f86a) Огромная утечка памяти в тесте 226
6. [Commit 0dd8](https://github.com/manticoresoftware/manticoresearch/commit/0dd80122) Кластер показывает синхронизацию узлов, но `count(*)` показывает разные числа
7. [Commit f1c1](https://github.com/manticoresoftware/manticoresearch/commit/f1c1ac3f) Косметические: Дублирующиеся и иногда потерянные предупреждения в логе
8. [Commit f1c1](https://github.com/manticoresoftware/manticoresearch/commit/f1c1ac3f) Косметические: (null) имя индекса в логе
9. [Commit 359d](https://github.com/manticoresoftware/manticoresearch/commit/359dbd30) Не получается получить больше 70 миллионов результатов
10. [Commit 19f3](https://github.com/manticoresoftware/manticoresearch/commit/19f328ee) Не вставляются PQ правила с синтаксисом без столбцов
11. [Commit bf68](https://github.com/manticoresoftware/manticoresearch/commit/bf685d5d) Ошибочное сообщение при вставке документа в индекс в кластере
12. [Commit 2cf1](https://github.com/manticoresoftware/manticoresearch/commit/2cf18c83) `/json/replace` и `json/update` возвращают id в экспоненциальном виде
13. [Issue #324](https://github.com/manticoresoftware/manticoresearch/issues/324) Обновление json scalar properties и mva в одном запросе
14. [Commit d384](https://github.com/manticoresoftware/manticoresearch/commit/d38409eb) `hitless_words` не работает в режиме RT
15. [Commit 5813](https://github.com/manticoresoftware/manticoresearch/commit/5813d639) В режиме rt должно быть запрещено `ALTER RECONFIGURE`
16. [Commit 5813](https://github.com/manticoresoftware/manticoresearch/commit/5813d639) После перезапуска searchd `rt_mem_limit` сбрасывается в 128M
17. highlight() иногда зависает
18. [Commit 7cd8](https://github.com/manticoresoftware/manticoresearch/commit/7cd878f4) Не удалось использовать U+code в режиме RT
19. [Commit 2b21](https://github.com/manticoresoftware/manticoresearch/commit/2b213de4) Не удалось использовать wildcard в wordforms в режиме RT
20. [Commit e9d0](https://github.com/manticoresoftware/manticoresearch/commit/e9d07e68) Исправлено `SHOW CREATE TABLE` при нескольких файлах wordform
21. [Commit fc90](https://github.com/manticoresoftware/manticoresearch/commit/fc90a84f) JSON запрос без "query" приводит к крашу searchd
22. Официальный docker Manticore [https://hub.docker.com/r/manticoresearch/manticore](https://hub.docker.com/r/manticoresearch/manticore) не мог индексировать из mysql 8
23. [Commit 23e0](https://github.com/manticoresoftware/manticoresearch/commit/23e05d32) HTTP /json/insert требует id
24. [Commit bd67](https://github.com/manticoresoftware/manticoresearch/commit/bd679af0) `SHOW CREATE TABLE` не работает для PQ
25. [Commit bd67](https://github.com/manticoresoftware/manticoresearch/commit/bd679af0) `CREATE TABLE LIKE` неправильно работает для PQ
26. [Commit 5eac](https://github.com/manticoresoftware/manticoresearch/commit/5eacf28f) Конец строки в настройках в show index status
27. [Commit cb15](https://github.com/manticoresoftware/manticoresearch/commit/cb153228) Пустой title в "highlight" в ответе HTTP JSON
28. [Issue #318](https://github.com/manticoresoftware/manticoresearch/issues/318) Ошибка инфиксного выражения в `CREATE TABLE LIKE`
29. [Коммит 9040](https://github.com/manticoresoftware/manticoresearch/commit/9040d22c) Сбой RT под нагрузкой
30. [cd512c7d ](https://github.com/manticoresoftware/manticoresearch/commit/cd512c7d ) Потерян лог сбоя при краше RT disk chunk
31. [Issue #323](https://github.com/manticoresoftware/manticoresearch/issues/323) Неудачный импорт таблицы и закрытие соединения
32. [Коммит 6275](https://github.com/manticoresoftware/manticoresearch/commit/6275316a) `ALTER reconfigure` повреждает индекс PQ
33. [Коммит 9c1d](https://github.com/manticoresoftware/manticoresearch/commit/9c1d221e) Проблемы с перезагрузкой searchd после изменения типа индекса
34. [Коммит 71e2](https://github.com/manticoresoftware/manticoresearch/commit/71e2b5bb) Сбой демона при импорте таблицы с отсутствующими файлами
35. [Issue #322](https://github.com/manticoresoftware/manticoresearch/issues/322) Сбой при select с использованием нескольких индексов, group by и ranker = none
36. [Коммит c3f5](https://github.com/manticoresoftware/manticoresearch/commit/c3f58490) `HIGHLIGHT()` не выделяет в строковых атрибутах
37. [Issue #320](https://github.com/manticoresoftware/manticoresearch/issues/320) `FACET` не сортирует по строковому атрибуту
38. [Коммит 4f1a](https://github.com/manticoresoftware/manticoresearch/commit/4f1a1f25) Ошибка при отсутствии каталога данных
39. [Коммит 04f4](https://github.com/manticoresoftware/manticoresearch/commit/04f4ddd4) access_* не поддерживаются в режиме RT
40. [Коммит 1c06](https://github.com/manticoresoftware/manticoresearch/commit/1c0616a2) Плохие JSON объекты в строках: 1. `CALL PQ` возвращает "Bad JSON objects in strings: 1", когда json превышает некоторое значение.
41. [Коммит 32f9](https://github.com/manticoresoftware/manticoresearch/commit/32f943d6) Несогласованность в режиме RT. В некоторых случаях невозможно удалить индекс, так как он неизвестен, и создать его, так как каталог не пуст.
42. [Issue #319](https://github.com/manticoresoftware/manticoresearch/issues/319) Сбой при select
43. [Коммит 22a2](https://github.com/manticoresoftware/manticoresearch/commit/22a28dd7) `max_xmlpipe2_field` = 2M выдавал предупреждение на поле 2M
44. [Issue #342](https://github.com/manticoresoftware/manticoresearch/issues/342) Ошибка выполнения условий запроса
45. [Коммит dd8d](https://github.com/manticoresoftware/manticoresearch/commit/dd8dcab2) Простой поиск из 2 терминов находит документ, содержащий только один термин
46. [Коммит 9091](https://github.com/manticoresoftware/manticoresearch/commit/90919e62) В PQ было невозможно сопоставить json с заглавными буквами в ключах
47. [Коммит 56da](https://github.com/manticoresoftware/manticoresearch/commit/56da086a) Индексатор падает при csv+docstore
48. [Issue #363](https://github.com/manticoresoftware/manticoresearch/issues/363) Использование `[null]` в json атрибуте на centos 7 вызывает повреждение вставленных данных
49. Крупный [Issue #345](https://github.com/manticoresoftware/manticoresearch/issues/345) Записи не вставляются, count() случайное, "replace into" возвращает OK
50. max_query_time слишком сильно замедляет SELECT-запросы


51. [Issue #352](https://github.com/manticoresoftware/manticoresearch/issues/352) Сбой связи мастер-агент на Mac OS
52. [Issue #328](https://github.com/manticoresoftware/manticoresearch/issues/328) Ошибка соединения с Manticore через Connector.Net/Mysql 8.0.19
53. [Коммит daa7](https://github.com/manticoresoftware/manticoresearch/commit/daa760d2fe83668038318f0f384ceefc0d075e18) Исправлена экранизация \0 и оптимизирована производительность

54. [Коммит 9bc5](https://github.com/manticoresoftware/manticoresearch/commit/9bc5c01a493db993b90ff6c1ad51aeae028f629c) Исправлен подсчёт distinct для JSON
55. [Коммит 4f89](https://github.com/manticoresoftware/manticoresearch/commit/4f89a965a3af0db548ae9d1763731ebaebe21f5a) Исправлена ошибка с удалением таблицы на другом узле
56. [Коммит 952a](https://github.com/manticoresoftware/manticoresearch/commit/952af5a5931fd72b87edc169588777778b365231) Исправлены сбои при частом вызове call pq
## Версия 3.4.2, 10 апреля 2020
### Критические исправления ошибок
* [Коммит 2ffe](https://github.com/manticoresoftware/manticoresearch/commit/2ffe2d261793f9681265bc4abe38d695e72c121e) исправлено: индекс RT из старой версии не индексирует данные

## Версия 3.4.0, 26 марта 2020
### Крупные изменения
* сервер работает в 2 режимах: rt-mode и plain-mode
   *   rt-mode требует data_dir и не допускает определение индексов в конфиге
   *   в plain-mode индексы определяются в конфиге; data_dir не разрешен
* репликация доступна только в rt-mode

### Незначительные изменения
* charset_table по умолчанию non_cjk alias
* в rt-mode полнотекстовые поля по умолчанию индексируются и сохраняются

* полнотекстовые поля в rt-mode переименованы с 'field' на 'text'
* ALTER RTINDEX переименован в ALTER TABLE
* TRUNCATE RTINDEX переименован в TRUNCATE TABLE
### Возможности
* поля только для хранения

* SHOW CREATE TABLE, IMPORT TABLE
### Улучшения
* намного более быстрый lockless PQ
* /sql может выполнять любые типы SQL-запросов в режиме mode=raw
* алиас mysql для протокола mysql41
* файл state.sql по умолчанию в data_dir
### Исправления ошибок
* [Коммит a533](https://github.com/manticoresoftware/manticoresearch/commit/a5333644e8e0c69746a9f24da8221fc965478e4a) исправлен сбой при неправильном синтаксисе поля в highlight()
* [Коммит 7fbb](https://github.com/manticoresoftware/manticoresearch/commit/7fbb9f2e618096289cba7f8b834530b46aa94055) исправлен сбой сервера при репликации RT индекса с docstore
* [Коммит 24a0](https://github.com/manticoresoftware/manticoresearch/commit/24a04687eb43b4f4719c7c4a5fe3c19099240467) исправлен сбой при highlight для индекса с опцией infix или prefix и без включённых хранимых полей
* [Коммит 3465](https://github.com/manticoresoftware/manticoresearch/commit/3465c1ce9c860c09f6ccc5b83a63e3c68c241d8d) исправлена ложная ошибка про пустой docstore и lookup по doc-id для пустого индекса
* [Коммит a707](https://github.com/manticoresoftware/manticoresearch/commit/b8601b4144b9092be43a7caa7cd6b19cb37e14c7) исправлен #314 SQL insert команда с завершающей точкой с запятой
* [Коммит 9562](https://github.com/manticoresoftware/manticoresearch/commit/95628c9b9b8f16fe1bd18599a84e9673575adf1a) убрано предупреждение о несовпадении слов запроса

* [Commit b860](https://github.com/manticoresoftware/manticoresearch/commit/b8601b4144b9092be43a7caa7cd6b19cb37e14c7) исправлены запросы в сниппетах, сегментированных через ICU
* [Commit 5275](https://github.com/manticoresoftware/manticoresearch/commit/5275516ca4631c584e6e0127fa161a18d2658fd0) исправлено состояние гонки при поиске/добавлении кэш-блока docstore
* [Commit f06e](https://github.com/manticoresoftware/manticoresearch/commit/f06ef97af5c7390427e82525c150535290e42c3e) исправлена утечка памяти в docstore
* [Commit a725](https://github.com/manticoresoftware/manticoresearch/commit/a7258ba8340e246c0d8aaeb4a7a74129eba807eb) исправлено #316 LAST_INSERT_ID возвращает пустое значение при INSERT
* [Commit 1ebd](https://github.com/manticoresoftware/manticoresearch/commit/1ebd5bf8d74113bcfa138ce63ce11966990a4215) исправлено #317 HTTP endpoint json/update для поддержки массива для MVA и объекта для JSON атрибута
* [Commit e426](https://github.com/manticoresoftware/manticoresearch/commit/e426950a536382a142e2a42ed207c341149ff685) исправлено частое сбрасывание индексатором rt без явного id
## Версия 3.3.0, 4 февраля 2020

### Особенности
* Параллельный поиск по индексу Real-Time
* Команда EXPLAIN QUERY
* конфигурационный файл без определений индексов (альфа-версия)
* команды CREATE/DROP TABLE (альфа-версия)
* indexer --print-rt — может читать из источника и выводить INSERTы для индекса Real-Time
### Улучшения


* Обновлено до Snowball 2.0 стеммеров
* фильтр LIKE для SHOW INDEX STATUS
* улучшено использование памяти при высоком max_matches
* SHOW INDEX STATUS добавляет ram_chunks_count для индексов RT
* безблочная очередь (lockless PQ)
* изменён LimitNOFILE до 65536
### Исправления ошибок
* [Commit 9c33](https://github.com/manticoresoftware/manticoresearch/commit/9c33aab8c1fd9059c8644d23d5f58f03010043c6) добавлена проверка схемы индекса на дублирование атрибутов #293
* [Commit a008](https://github.com/manticoresoftware/manticoresearch/commit/a0085f9477e191da9e0e835b91583fb9cfde698e) исправлен сбой при hitless terms
* [Commit 6895](https://github.com/manticoresoftware/manticoresearch/commit/68953740af8dca1833026d0f448c71416ed3044a) исправлен loose docstore после ATTACH
* [Commit d6f6](https://github.com/manticoresoftware/manticoresearch/commit/d6f696edea53ea2b9a68251e4054e4816082c285) исправлена проблема docstore в распределённой настройке
* [Commit bce2](https://github.com/manticoresoftware/manticoresearch/commit/bce2b7ec883d208f31094ee42a5b6f01d18a998f) заменён FixedHash на OpenHash в сортировщике
* [Commit e0ba](https://github.com/manticoresoftware/manticoresearch/commit/e0baf7392937969e640fb5d10e8ccb312fe36446) исправлены атрибуты с дублированными именами в определении индекса
* [Commit ca81](https://github.com/manticoresoftware/manticoresearch/commit/ca81114bf1ac7ea7adcc8509bdc14d1f30b4f461) исправлен html_strip в HIGHLIGHT()
* [Commit 493a](https://github.com/manticoresoftware/manticoresearch/commit/493a5e916d70fd4a3f14d1ee487234434f4a7ce3) исправлен макрос passage в HIGHLIGHT()
* [Commit a82d](https://github.com/manticoresoftware/manticoresearch/commit/a82d41c775692f837f267d554d7d4a4b499d69f0) исправлены проблемы с двойным буфером при создании RT индекса с маленьким или большим дисковым чанком
* [Commit a404](https://github.com/manticoresoftware/manticoresearch/commit/a404c85de0d41d9d1c75b7540479d8a3af9e61aa) исправлено удаление событий для kqueue

* [Commit 8bea](https://github.com/manticoresoftware/manticoresearch/commit/8bea0f6fd1666b817995c4dd6e67f6f1c9161638) исправлено сохранение дискового чанка для большого значения rt_mem_limit RT индекса
* [Commit 8707](https://github.com/manticoresoftware/manticoresearch/commit/8707f0395b7ace52dc0cba9c8ed92a2bc4392932) исправлено переполнение float при индексировании
* [Commit a564](https://github.com/manticoresoftware/manticoresearch/commit/a56434ce71bb7714b4d8e75278aaa075b27ece80) исправлена ошибка при вставке документа с отрицательным ID в RT индекс
* [Commit bbeb](https://github.com/manticoresoftware/manticoresearch/commit/bbebfd7551a68641154761cac962053b54806759) исправлен сбой сервера при использовании ranker fieldmask
* [Commit 3809](https://github.com/manticoresoftware/manticoresearch/commit/3809cc1b0863e3a5a200dc423be3f7c38be56113) исправлен сбой при использовании кэша запросов
* [Commit dc2a](https://github.com/manticoresoftware/manticoresearch/commit/dc2a585bc724678679e4297362200bbd49ce32bb) исправлен сбой при использовании RAM сегментов RT индекса с параллельными вставками

## Версия 3.2.2, 19 декабря 2019
### Особенности
* Автоинкремент ID для RT индексов
* Поддержка подсветки для docstore через новую функцию HIGHLIGHT(), доступную также в HTTP API
* SNIPPET() может использовать специальную функцию QUERY(), которая возвращает текущий MATCH запрос
* новая опция field_separator для функций подсветки
### Улучшения и изменения
* ленивое извлечение сохранённых полей для удалённых узлов (существенно увеличивает производительность)
* строки и выражения больше не ломают оптимизации multi-query и FACET
* сборка RHEL/CentOS 8 теперь использует mysql libclient из mariadb-connector-c-devel

* ICU data файл теперь включён в пакеты, icu_data_dir удалён
* файлы сервиса systemd содержат политику 'Restart=on-failure'
* indextool теперь может проверять индексы real-time онлайн
* конфигурация по умолчанию теперь /etc/manticoresearch/manticore.conf
* сервис на RHEL/CentOS переименован в 'manticore' вместо 'searchd'
* удалены опции query_mode и exact_phrase для сниппетов
### Исправления ошибок
* [Commit 6ae4](https://github.com/manticoresoftware/manticoresearch/commit/6ae474c7894a6bee222d5b18e59a44fdbf57843a) исправлен сбой при SELECT запросе через HTTP интерфейс
* [Commit 5957](https://github.com/manticoresoftware/manticoresearch/commit/59577513a49eac5a4a3c5e2cb38394d3246b5d35) исправлено сохранение диск чанков RT индекса без пометки некоторых документов как удалённых
* [Commit e861](https://github.com/manticoresoftware/manticoresearch/commit/e861f0fca0e88924450695d4e9d6acff7a36558a) исправлен сбой при поиске по мульти индексу или мульти запросам с dist_threads
* [Commit 4409](https://github.com/manticoresoftware/manticoresearch/commit/440991fc977b8479800b45cdbc862eeb1ba5d965) исправлен сбой при генерации инфиксов для длинных терминов с широкими utf8 кодовыми точками
* [Commit 5fd5](https://github.com/manticoresoftware/manticoresearch/commit/5fd599b48bba527c023e6aa0b262dca51ffb8a1c) исправлено состояние гонки при добавлении сокета в IOCP
* [Commit cf10](https://github.com/manticoresoftware/manticoresearch/commit/cf10d7d3589f77adfaefbe52a51777bc9c67cf99) исправлена проблема с булевыми запросами и списком выбора JSON
* [Commit 996d](https://github.com/manticoresoftware/manticoresearch/commit/996de77f4c3ec103d965e28c36098fd07dba50c4) исправлена проверка indextool для отчетов о неправильном смещении skiplist, проверка поиска doc2row
* [Commit 6e3f](https://github.com/manticoresoftware/manticoresearch/commit/6e3fc9e88941c9427410f9d0adeebb7dd09a900f) исправлен индексатор, производящий плохой индекс с негативным смещением skiplist на больших данных
* [Commit faed](https://github.com/manticoresoftware/manticoresearch/commit/faed3220b423a96401982bf47117edf1f62c584e) исправлена конвертация JSON: теперь конвертируется только числовое в строку и строка JSON в числовое в выражениях
* [Commit 5331](https://github.com/manticoresoftware/manticoresearch/commit/533197200249d648ebcbdaca5d605861ee566348) исправлен indextool для выхода с ошибкой при установке нескольких команд в командной строке
* [Commit 7955](https://github.com/manticoresoftware/manticoresearch/commit/795520ac351d22e4497ebaf6c249bf25081842b6) исправлен #275 некорректное состояние binlog при ошибке "нет места на диске"

* [Commit 2284](https://github.com/manticoresoftware/manticoresearch/commit/2284da5e3ff77bf7be4038a4d950c0580e4159c8) исправлен #279 сбой при использовании фильтра IN для JSON-атрибута
* [Commit ce2e](https://github.com/manticoresoftware/manticoresearch/commit/ce2e4b4765111a7d331ffc256911f12770a8942d) исправлен #281 неправильный вызов закрытия pipe
* [Commit 5355](https://github.com/manticoresoftware/manticoresearch/commit/535589ba8a37baf5c6056afb5ff969fd70476feb) исправлена зависшая работа сервера при CALL PQ с рекурсивным JSON-атрибутом, кодированным как строка
* [Commit a5fc](https://github.com/manticoresoftware/manticoresearch/commit/a5fc8a36e1644ee051632408ab8b53e79aeb9ff4) исправлен выход за пределы конца doclist в узле multiand

* [Commit a362](https://github.com/manticoresoftware/manticoresearch/commit/a3628617ef4ee89a811d4ba2c4a04cbbc298c1b5) исправлено получение публичной информации о потоке
* [Commit f8d2](https://github.com/manticoresoftware/manticoresearch/commit/f8d2d7bbbb90af3d2ee97424f97b05927f09d42b) исправлены блокировки кэша docstore
## Версия 3.2.0, 17 октября 2019
### Функции
* Хранение документов

* новые директивы stored_fields, docstore_cache_size, docstore_block_size, docstore_compression, docstore_compression_level
### Улучшения и изменения
* улучшена поддержка SSL
* обновлена встроенная кодировка non_cjk
* отключен лог запросов SELECT для операторов UPDATE/DELETE в журнале запросов
* пакеты для RHEL/CentOS 8
### Исправления ошибок
* [Commit 301a](https://github.com/manticoresoftware/manticoresearch/commit/301a806b14725e842300ee225521407464374c41) исправлен сбой при замене документа в дисковом чанке RT индекса
* [Commit 46c1](https://github.com/manticoresoftware/manticoresearch/commit/46c1cad8fa11c1fc09cc34b20d978fdfcd4e6d27) исправлен #269 LIMIT N OFFSET M
* [Commit 92a4](https://github.com/manticoresoftware/manticoresearch/commit/92a46edaa196ef33f3ac2225fbdba2c2b88ee45d) исправлены операторы DELETE с явно установленным id или предоставленным списком id для пропуска поиска

* [Commit 8ca7](https://github.com/manticoresoftware/manticoresearch/commit/8ca78c138d837caab30dc6e2343a3c4d1687bb87) исправлен неправильный индекс после удаления события в netloop у poller windowspoll
* [Commit 6036](https://github.com/manticoresoftware/manticoresearch/commit/603631e2bff5cb0acf25f68b85621742490eccd9) исправлено округление float при JSON через HTTP
* [Commit 62f6](https://github.com/manticoresoftware/manticoresearch/commit/62f64cb9e75ec28fc3c29f8414ce9656f5580254) исправлены удаленные сниппеты для проверки пустого пути; исправлены тесты для Windows
* [Commit aba2](https://github.com/manticoresoftware/manticoresearch/commit/aba274c2c5abc1029894ba9f95b786bda93e8f22) исправлен перезапуск конфигурации для работы на Windows так же, как на Linux
* [Commit 6b8c](https://github.com/manticoresoftware/manticoresearch/commit/6b8c4242ef77c3fa4d0ccb7d76d81714b6728f0b) исправлен #194 PQ для работы с морфологией и стеммерами
* [Commit 174d](https://github.com/manticoresoftware/manticoresearch/commit/174d312905285aa5746f4f404c53e6dc669ef42f) исправлено управление устаревшими сегментами RT
## Версия 3.1.2, 22 августа 2019

### Функции и улучшения
* Экспериментальная поддержка SSL для HTTP API
* фильтр по полям для CALL KEYWORDS
* max_matches для /json/search
* автоматическое определение размера по умолчанию для Galera gcache.size
* улучшена поддержка FreeBSD
### Исправления ошибок
* [Commit 0a1a](https://github.com/manticoresoftware/manticoresearch/commit/0a1a2c81038794983b4e30c8aefc7cc20e89b03f) исправлена репликация RT индекса в узел, где существует такой же RT индекс с другим путем

* [Commit 4adc](https://github.com/manticoresoftware/manticoresearch/commit/4adc075294ac823289f745e2cc419f18c7dcf2e2) исправлено переназначение flush для индексов без активности
* [Commit d6c0](https://github.com/manticoresoftware/manticoresearch/commit/d6c00a6f750c04021747e9c99e0f77707a4cbfc3) улучшено переназначение сброса RT/PQ индексов
* [Commit d0a7](https://github.com/manticoresoftware/manticoresearch/commit/d0a7c95906b9b5ba33c98267d54501cfe27508d6) исправлен #250 параметр index_field_lengths для источников TSV и CSV через pipe
* [Commit 1266](https://github.com/manticoresoftware/manticoresearch/commit/1266d54800242709fbe2c34e72eb69b6595b23bd) исправлен неверный отчет indextool для проверки блочного индекса на пустом индексе
* [Commit 553c](https://github.com/manticoresoftware/manticoresearch/commit/553ca73ca111f28e0888d59cd48b0d6df181ecd6) исправлен пустой список выбора в журнале запросов Manticore SQL
* [Commit 56c8](https://github.com/manticoresoftware/manticoresearch/commit/56c8584456293665ddd4ce4c94307d74473a78b9) исправлен ответ индексатора на -h/--help
## Версия 3.1.0, 16 июля 2019
### Функции и улучшения
* репликация для индексов RealTime
* ICU токенизатор для китайского языка
* новый параметр морфологии icu_chinese
* новая директива icu_data_dir
* транзакции с несколькими операторами для репликации
* LAST_INSERT_ID() и @session.last_insert_id

* LIKE 'pattern' для SHOW VARIABLES
* множественное вставление документов для перколяторных индексов
* добавлены парсеры времени для конфигурации

* внутренний менеджер задач
* mlock для компонентов doc и hit listes
* ограничение пути сниппетов
### Удаления
* Поддержка библиотеки RLP прекращена в пользу ICU; все директивы rlp\* удалены
* обновление ID документа с помощью UPDATE отключено
### Исправления ошибок
* [Коммит f047](https://github.com/manticoresoftware/manticoresearch/commit/f047222339fd0e62a07e9ca8b2a829297d5db861) исправление дефектов в concat и group_concat
* [Коммит b081](https://github.com/manticoresoftware/manticoresearch/commit/b08147ee24712b5eaa51f261626b5d16c6242fdd) исправлен тип атрибута query uid для percolate индекса на BIGINT
* [Коммит 4cd8](https://github.com/manticoresoftware/manticoresearch/commit/4cd85afaf76eef0ce9fa1cd073f3fed160279890) предотвращен сбой при неудаче предвыделения нового дискового чанка
* [Коммит 1a55](https://github.com/manticoresoftware/manticoresearch/commit/1a551227886474054111088412f2b2396f485652) добавлен отсутствующий тип данных timestamp в ALTER

* [Коммит f3a8](https://github.com/manticoresoftware/manticoresearch/commit/f3a8e096fd496a6b001d398aadbb781b2dbad929) исправлен сбой при неправильном mmap-считывании
* [Коммит 4475](https://github.com/manticoresoftware/manticoresearch/commit/447577115192db7ac7bfea70b5ba58ee2c61229f) исправлен хэш блокировки кластеров в репликации
* [Коммит ff47](https://github.com/manticoresoftware/manticoresearch/commit/ff476df955f057bd38e8b6ad6056325e9d6fbb9c) исправлена утечка провайдеров в репликации
* [Коммит 58dc](https://github.com/manticoresoftware/manticoresearch/commit/58dcbb779c4c30ca1560460c78e35ba114b83716) исправлен \#246 undefined sigmask в indexer
* [Коммит 3dd8](https://github.com/manticoresoftware/manticoresearch/commit/3dd8278e747d534f69fe9c3d360eef188ab7b6d5) исправлена гонка в отчётах netloop
* [Коммит a02a](https://github.com/manticoresoftware/manticoresearch/commit/a02aae05a3fa3d1caf2821f43c7e237b6de76336) нулевой разрыв для балансировщика стратегий HA

## Версия 3.0.2, 31 мая 2019
### Улучшения

* добавлены mmap-ридеры для документов и списков попаданий
* ответ HTTP эндпоинта `/sql` теперь совпадает с ответом `/json/search`

* новые директивы `access_plain_attrs`, `access_blob_attrs`, `access_doclists`, `access_hitlists`
* новая директива `server_id` для настроек репликации
### Удаления
* удалён HTTP эндпоинт `/search`
### Устаревшие функции
* `read_buffer`, `ondisk_attrs`, `ondisk_attrs_default`, `mlock` заменены директивами `access_*`
### Исправления ошибок
* [Коммит 849c](https://github.com/manticoresoftware/manticoresearch/commit/849c16e1040f382795ba0635cb48686f9db4f2d8) разрешены имена атрибутов, начинающиеся с цифр в списке select
* [Коммит 48e6](https://github.com/manticoresoftware/manticoresearch/commit/48e6c302ca37bec0bc49c79619c60bf787ed0708) исправлены MVAs в UDF, исправлено алиасинг MVA
* [Коммит 0555](https://github.com/manticoresoftware/manticoresearch/commit/055586a9a2ebfa8874f68099c5bc798a145fd4ef) исправлен \#187 сбой при использовании запроса с SENTENCE


* [Коммит 93bf](https://github.com/manticoresoftware/manticoresearch/commit/93bf52f23f1c53c8a93a603387ca5463e1fc2ae8) исправлен \#143 поддержка () вокруг MATCH()
* [Коммит 599e](https://github.com/manticoresoftware/manticoresearch/commit/599ee79ccd38cce0023a55cbce8f50c3dca62e38) исправлено сохранение состояния кластера при ALTER cluster
* [Коммит 230c](https://github.com/manticoresoftware/manticoresearch/commit/230c321e277514b191629538eb3f02d219113d95) исправлен сбой сервера при ALTER index с blob-атрибутами
* [Коммит 5802](https://github.com/manticoresoftware/manticoresearch/commit/5802b85aa93a81caea77073a3381ef912fdd677c) исправлен \#196 фильтр по id
* [Коммит 25d2](https://github.com/manticoresoftware/manticoresearch/commit/25d2dabda4a299a29bd375405862126da9b4bfac) отключён поиск по индексам шаблона
* [Коммит 2a30](https://github.com/manticoresoftware/manticoresearch/commit/2a30d5b49d76faab031d195f533974938769b8e6) исправлен тип столбца id на обычный bigint в SQL ответе
## Версия 3.0.0, 6 мая 2019
### Функции и улучшения
* Новое хранилище индексов. Некваровые атрибуты больше не ограничены размером 4 ГБ на индекс
* директива attr_update_reserve
* Строки, JSON и MVA можно обновлять с помощью UPDATE
* killlists применяются при загрузке индекса
* директива killlist_target
* ускорение множественных AND-поисков
* лучшее среднее быстродействие и использование ОЗУ

* конвертер для обновления индексов, созданных в 2.x
* функция CONCAT()
* JOIN CLUSTER cluster AT 'nodeaddress:port'
* ALTER CLUSTER posts UPDATE nodes

* директива node_address
* список узлов выводится в SHOW STATUS

### Изменения в поведении
* при наличии killlist сервер не выполняет ротацию индексов в порядке, определённом в конфиге, а следует цепочке целей killlist
* порядок индексов в поиске больше не определяет порядок применения killlist
* ID документов теперь подписанные целые числа большого размера

### Удалённые директивы
* docinfo (всегда extern теперь), inplace_docinfo_gap, mva_updates_pool
## Версия 2.8.2 GA, 2 апреля 2019

### Функции и улучшения
* репликация Galera для percolate индексов
* OPTION morphology
### Примечания к компиляции
Минимальная версия Cmake теперь 3.13. Для компиляции требуются boost и libssl
разработческие библиотеки.
### Исправления ошибок
* [Коммит 6967](https://github.com/manticoresoftware/manticoresearch/commit/6967fedb2ef818ec1c825d482563edd05e1c9245) исправлен сбой при множественных звездочках в списке select для запроса по многим распределённым индексам
* [Коммит 36df](https://github.com/manticoresoftware/manticoresearch/commit/36df1a407dc08263690e3492518613ace82d69ca) исправлен [\#177](https://github.com/manticoresoftware/manticoresearch/issues/177) большой пакет через интерфейс Manticore SQL
* [Коммит 5793](https://github.com/manticoresoftware/manticoresearch/commit/57932aec734583fa93359faaf5034b2e2c9d352b) исправлен [\#170](https://github.com/manticoresoftware/manticoresearch/issues/170) сбой сервера при оптимизации RT с обновлённым MVA

* [Коммит edb2](https://github.com/manticoresoftware/manticoresearch/commit/edb24b870423add86eba471d361e0e5aff098b18) исправлен сбой сервера при удалении binlog из-за удаления RT индекса после перезагрузки конфигурации по SIGHUP
* [Коммит bd3e](https://github.com/manticoresoftware/manticoresearch/commit/bd3e66e0085bc7e2e351b817dfc972fd8158fbce) исправлены загрузки плагинов аутентификации mysql handshake
* [Commit 6a21](https://github.com/manticoresoftware/manticoresearch/commit/6a217f6ed82fb10f752213b15617b9cedf1e8533) исправлены настройки phrase_boundary в RT индексе [\#172](https://github.com/manticoresoftware/manticoresearch/issues/172)
* [Commit 3562](https://github.com/manticoresoftware/manticoresearch/commit/3562f652753e4091fd7b5b0f65b58341c8cbcb31) исправлена дедлок при присоединении индекса к самому себе [\#168](https://github.com/manticoresoftware/manticoresearch/issues/168)
* [Commit 250b](https://github.com/manticoresoftware/manticoresearch/commit/250b3f0e74ad18dda34ba080aa13ca87798a9ac9) исправлено сохранение binlog пустого meta после сбоя сервера

* [Commit 4aa6](https://github.com/manticoresoftware/manticoresearch/commit/4aa6c69ad3a181cd7c5dafb990fb528121f68e59) исправлен сбой сервера из-за строки в сортировщике из RT индекса с disk chunks
## Версия 2.8.1 GA, 6 марта 2019
### Новый функционал и улучшения
* SUBSTRING_INDEX()
* поддержка SENTENCE и PARAGRAPH для перколятных запросов
* systemd генератор для Debian/Ubuntu; также добавлен LimitCORE для разрешения создания core-файлов
### Исправления ошибок
* [Commit 84fe](https://github.com/manticoresoftware/manticoresearch/commit/84fe7405d17b59f4dcde15a6c8e8503923503def) исправлен сбой сервера при режиме match all и пустом полном текстовом запросе
* [Commit daa8](https://github.com/manticoresoftware/manticoresearch/commit/daa88b579fa373d5d2e869e43a2e178363b0fef1) исправлен сбой при удалении статической строки
* [Commit 2207](https://github.com/manticoresoftware/manticoresearch/commit/22078537dddbd9ce81a182dcc879f235b6e87004) исправлен код выхода при ошибке indextool с FATAL
* [Commit 0721](https://github.com/manticoresoftware/manticoresearch/commit/0721696d6780c200c65d596624a8187983fb7fcb) исправлено [\#109](https://github.com/manticoresoftware/manticoresearch/issues/109) отсутствие совпадений для префиксов из-за неправильной проверки exact form
* [Commit 8af8](https://github.com/manticoresoftware/manticoresearch/commit/8af810111b8e2f87bc9f378172eff1ab9725c7e7) исправлено [\#161](https://github.com/manticoresoftware/manticoresearch/issues/161) обновление конфигурационных настроек для RT индексов
* [Commit e2d5](https://github.com/manticoresoftware/manticoresearch/commit/e2d592773f4fd702ace90070a793a842a0fec6ab) исправлен сбой сервера при обращении к большой JSON-строке
* [Commit 75cd](https://github.com/manticoresoftware/manticoresearch/commit/75cd1342d05cdc352e3dd145cdbc6f79394a165b) исправлено некорректное совпадение из-за изменения поля PQ в JSON-документе с помощью индексного стриппера
* [Commit e2f7](https://github.com/manticoresoftware/manticoresearch/commit/e2f775437df922674d772ed4417780492502e65a) исправлен сбой сервера при разборе JSON на сборках RHEL7
* [Commit 3a25](https://github.com/manticoresoftware/manticoresearch/commit/3a25a5808feb3f8b80866991b436d6c1241618c2) исправлен сбой при json unescaping, когда слэш на краю

* [Commit be9f](https://github.com/manticoresoftware/manticoresearch/commit/be9f497872bea62dbdccd64fc28294fd7776c289) исправлена опция 'skip_empty' для пропуска пустых документов без предупреждения, что они не валидный json
* [Commit 266e](https://github.com/manticoresoftware/manticoresearch/commit/266e0e7b088549722a805fc837bf101ff681a5e8) исправлено [\#140](https://github.com/manticoresoftware/manticoresearch/issues/161) вывод 8 знаков у float, когда 6 недостаточно для точности
* [Commit 3f6d](https://github.com/manticoresoftware/manticoresearch/commit/3f6d23891064ee8e94030c4231497cdd7da33a6a) исправлено создание пустого jsonobj
* [Commit f3c7](https://github.com/manticoresoftware/manticoresearch/commit/f3c7848a59b7eedc67c3403d6d4b37ce6fa94dc8) исправлено [\#160](https://github.com/manticoresoftware/manticoresearch/issues/161) вывод пустого mva как NULL вместо пустой строки
* [Commit 0afa](https://github.com/manticoresoftware/manticoresearch/commit/0afa2ed058e5759470b1d7354c722faab34f98bb) исправлен сбой сборки без pthread_getname_np
* [Commit 9405](https://github.com/manticoresoftware/manticoresearch/commit/9405fccdeb27f0302a8d9a848981d30080216777) исправлен сбой при завершении сервера с рабочими thread_pool
## Версия 2.8.0 GA, 28 января 2019
### Улучшения
* Распределённые индексы для перколятных индексов
* CALL PQ новые опции и изменения:
    *   skip_bad_json
    *   mode (sparsed/sharded)
    *   JSON-документы теперь можно передавать как JSON-массив
    *   shift
    *   Имена колонок 'UID', 'Documents', 'Query', 'Tags', 'Filters' переименованы в 'id', 'documents', 'query', 'tags', 'filters'
* DESCRIBE pq TABLE
* SELECT FROM pq WHERE UID теперь невозможен, используйте 'id' вместо этого
* SELECT по pq индексам теперь наравне с обычными индексами (например, можно фильтровать правила через REGEX())

* ANY/ALL можно использовать по PQ тэгам
* выражения имеют автоматическое приведение типов для JSON полей, не требующее явного кастинга
* встроенная charset_table 'non_cjk' и ngram_chars 'cjk'
* встроенные коллекции стоп-слов для 50 языков
* в определении стоп-слов можно указать несколько файлов через запятую
* CALL PQ теперь может принимать JSON-массив документов
### Исправления ошибок
* [Commit a4e1](https://github.com/manticoresoftware/manticoresearch/commit/a4e19afee54dafdc04b0da53372dafd8c0d332d6) исправлен утечка, связанная с csjon
* [Commit 28d8](https://github.com/manticoresoftware/manticoresearch/commit/28d862774874891a03e361da1b0347ebe8066ce0) исправлен сбой из-за отсутствующего значения в json
* [Commit bf4e](https://github.com/manticoresoftware/manticoresearch/commit/bf4e9ea27c349cdc9ae4e54e960a4a17b90e38fa) исправлено сохранение пустого meta для RT индекса
* [Commit 33b4](https://github.com/manticoresoftware/manticoresearch/commit/33b4573529e7c7c7bce19bf9d54054866f30d99a) исправлено потерянное значение флага form (exact) для последовательности лемматайзера
* [Commit 6b95](https://github.com/manticoresoftware/manticoresearch/commit/6b95d48240d3b5520afade19c249d79e020a5e88) исправлено использование saturate вместо overflow для строковых атрибутов превышающих 4М
* [Commit 6214](https://github.com/manticoresoftware/manticoresearch/commit/621418b829e70af36aaa322a2f51ece3f86bc499) исправлен сбой сервера при SIGHUP с отключённым индексом
* [Commit 3f7e](https://github.com/manticoresoftware/manticoresearch/commit/3f7e35d1482966cc45d5abbcb2de5de3508d66b0) исправлен сбой сервера при одновременном вызове API сессии статуса
* [Commit cd9e](https://github.com/manticoresoftware/manticoresearch/commit/cd9e4f1709a48ddafc128c450c2d882bc11ba50e) исправлен сбой сервера при удалении запроса к RT индексу с фильтрами по полям
* [Commit 9376](https://github.com/manticoresoftware/manticoresearch/commit/9376470d455fcd256c6abff79c431a6919dfa7ac) исправлен сбой сервера при вызове CALL PQ к распределённому индексу с пустым документом
* [Commit 8868](https://github.com/manticoresoftware/manticoresearch/commit/8868b207644f4f8cc1ab0c270adb35493fed22e1) исправлено обрезание сообщения об ошибке Manticore SQL длиной более 512 символов
* [Commit de9d](https://github.com/manticoresoftware/manticoresearch/commit/de9deda9c142823d6dbf529423417b5c670fae94) исправлен сбой при сохранении percolate индекса без binlog
* [Commit 2b21](https://github.com/manticoresoftware/manticoresearch/commit/2b219e1a32791e7740bb210b7d408f96abc6374f) исправлен неработающий http интерфейс в OSX

* [Commit e92c](https://github.com/manticoresoftware/manticoresearch/commit/e92c60240f8f9e1756bfa14fba0bbb402586a389) исправлено ложное сообщение об ошибке indextool при проверке MVA
* [Commit 238b](https://github.com/manticoresoftware/manticoresearch/commit/238bdea59bad89f097403f1c978658ce45f16c70) исправлена блокировка записи при FLUSH RTINDEX, чтобы не блокировать весь индекс во время сохранения и при регулярном flush из rt_flush_period
* [Commit c26a](https://github.com/manticoresoftware/manticoresearch/commit/c26a236bd42b7082db079a51a84172066d337d64) исправлено зависание ALTER percolate индекса при ожидании загрузки поиска
* [Commit 9ee5](https://github.com/manticoresoftware/manticoresearch/commit/9ee5703f6c28f57cd5bff5e705966d93fe30d267) исправлено использование max_children по умолчанию количества работников thread_pool при значении 0
* [Commit 5138](https://github.com/manticoresoftware/manticoresearch/commit/5138fc048a1bd146b271ce6a72fe954344281f69) исправлена ошибка при индексации данных в индекс с плагином index_token_filter вместе с stopwords и stopword_step=0

* [Commit 2add](https://github.com/manticoresoftware/manticoresearch/commit/2add3d319ac62fe450bf60e89033100853dc2ecf) исправлен сбой при отсутствии lemmatizer_base при продолжении использования aot лемматизаторов в определениях индекса
## Версия 2.7.5 GA, 4 декабря 2018
### Улучшения
* функция REGEX
* limit/offset для json API поиска
* profiler точки для qcache
### Исправления ошибок

* [Commit eb3c](https://github.com/manticoresoftware/manticoresearch/commit/eb3c7683b9740cff4cc4cadd2ab9f990b9f7d173) исправлен сбой сервера на FACET с несколькими атрибутами широких типов
* [Commit d915](https://github.com/manticoresoftware/manticoresearch/commit/d915cf6696514f4be365ffb5981bccaa15983e65) исправлен неявный group by в основном списке выбора для запроса FACET
* [Commit 5c25](https://github.com/manticoresoftware/manticoresearch/commit/5c25dc271a8c11a1c82ca3c88e518acbc6a8c164) исправлен сбой на запросе с GROUP N BY
* [Commit 85d3](https://github.com/manticoresoftware/manticoresearch/commit/85d30a232d4ffd89916b673a9b2db946d272a945) исправлена взаимоблокировка при обработке сбоя в операциях с памятью
* [Commit 8516](https://github.com/manticoresoftware/manticoresearch/commit/85166b5ef2be38061d62f2b20f15acaa0311258a) исправлено потребление памяти indextool во время проверки

* [Commit 58fb](https://github.com/manticoresoftware/manticoresearch/commit/58fb03166830d8b23e7ab9ce56309db7ddac22f4) удалён ненужный включаемый файл gmock, теперь upstream разрешает это самостоятельно
## Версия 2.7.4 GA, 1 ноября 2018
### Улучшения
* SHOW THREADS для удалённых распределённых индексов выводит оригинальный запрос вместо вызова API
* SHOW THREADS новая опция `format=sphinxql` выводит все запросы в формате SQL
* SHOW PROFILE выводит дополнительный этап `clone_attrs`
### Исправления ошибок
* [Commit 4f15](https://github.com/manticoresoftware/manticoresearch/commit/4f155712a0bccc1bd01cc191647bc8cff814888e) исправлена ошибка сборки с libc без malloc_stats, malloc_trim

* [Commit f974](https://github.com/manticoresoftware/manticoresearch/commit/f974f20bda3214a56877c393a192be1a77150958) исправлены специальные символы внутри слов для результата CALL KEYWORDS
* [Commit 0920](https://github.com/manticoresoftware/manticoresearch/commit/092083282ea8ae0b2e72fb6989c57ccec81e74ac) исправлен сломанный CALL KEYWORDS к распределённому индексу через API или удалённому агенту
* [Commit fd68](https://github.com/manticoresoftware/manticoresearch/commit/fd686bfe88b720ffd7642e36897ba45161cbd7d2) исправлено распространение agent_query_timeout распределённого индекса к агентам как max_query_time
* [Commit 4ffa](https://github.com/manticoresoftware/manticoresearch/commit/4ffa623e9d357a3b0b441615089e211f92f8de32) исправлен счётчик общего количества документов на дисковом чанке, который влиялся командой OPTIMIZE и ломал расчёт весов
* [Commit dcaf](https://github.com/manticoresoftware/manticoresearch/commit/dcaf4e0e3c8e9b0fbec47bbf3307feddec0936a6) исправлены множественные совпадения tail на RT индексе из blended
* [Commit eee3](https://github.com/manticoresoftware/manticoresearch/commit/eee381754e902a43a8f499e1c950198a2a3e6ee0) исправлена взаимоблокировка при ротации

## Версия 2.7.3 GA, 26 сентября 2018
### Улучшения
* опция sort_mode для CALL KEYWORDS
* DEBUG на VIP соединении может выполнить 'crash <password>' для намеренного вызова SIGEGV на сервере
* DEBUG может выполнить 'malloc_stats' для дампа статистики malloc в searchd.log и 'malloc_trim' для вызова malloc_trim()
* улучшенный бэктрейс при наличии gdb в системе
### Исправления ошибок
* [Commit 0f3c](https://github.com/manticoresoftware/manticoresearch/commit/0f3cc333f6129451ad1ae2c97905b6c04ba5d454) исправлен сбой или ошибка rename в Windows
* [Commit 1455](https://github.com/manticoresoftware/manticoresearch/commit/1455ba2bf8079f03ce7f861a8d9662a360d705de) исправлены сбои сервера на 32-битных системах
* [Commit ad37](https://github.com/manticoresoftware/manticoresearch/commit/ad3710d53b5cd6a28b8c60352d0f9e285b03d9fa) исправлен сбой или зависание сервера на пустом выражении SNIPPET
* [Commit b36d](https://github.com/manticoresoftware/manticoresearch/commit/b36d79214364500252665860f000ae85343528b6) исправлен сломанный непрогрессивный optimize и исправлен прогрессивный optimize чтобы не создавать kill-list для самого старого дискового чанка
* [Commit 34b0](https://github.com/manticoresoftware/manticoresearch/commit/34b032499afd42ce47a4c7247814b4031094388a) исправлен неправильный ответ queue_max_length для SQL и API в режиме потокового пула
* [Commit ae4b](https://github.com/manticoresoftware/manticoresearch/commit/ae4b3202cbdb8014cabe2b90e269d5cf74f49871) исправлен сбой при добавлении полносканирующего запроса в PQ индекс с установленными опциями regexp или rlp
* [Commit f80f](https://github.com/manticoresoftware/manticoresearch/commit/f80f8d5d7560187078868aed9a9575f4549e98aa) исправлен сбой при последовательном вызове PQ
* [Commit 9742](https://github.com/manticoresoftware/manticoresearch/commit/9742f5f0866af73f8cd8483ecd18a507ea80dd65) рефакторинг AcquireAccum
* [Commit 39e5](https://github.com/manticoresoftware/manticoresearch/commit/39e5bc3751b7295222eb76407c5d72ce1dad545b) исправлена утечка памяти после вызова pq
* [Commit 21bc](https://github.com/manticoresoftware/manticoresearch/commit/21bcc6d17395f0f57dde79f6716ef303b7ea527d) косметический рефакторинг (стиль c++11 c-trs, значения по умолчанию, nullptr)
* [Commit 2d69](https://github.com/manticoresoftware/manticoresearch/commit/2d690398f14c736956cfdd66feb4d3091d6b3a4d) исправлена утечка памяти при попытке вставить дубликат в PQ индекс
* [Commit 5ed9](https://github.com/manticoresoftware/manticoresearch/commit/5ed92c4b66da6423fa28f11b1fd59103ed1e1fb9) исправлен сбой при обработке JSON-поля IN с большими значениями
* [Commit 4a52](https://github.com/manticoresoftware/manticoresearch/commit/4a5262e2776aa261e34ba5c914ea60804f59de3f) исправлен сбой сервера при вызове CALL KEYWORDS для RT индекса с установленным лимитом расширения
* [Commit 5526](https://github.com/manticoresoftware/manticoresearch/commit/552646bb6fefde4e2525298e43e628839b421d3d) исправлен неверный фильтр в запросе PQ matches;
* [Commit 204f](https://github.com/manticoresoftware/manticoresearch/commit/204f521408b730198a5bab2c20407a3d7df94873) введён небольшой аллокатор объектов для ptr attrs
* [Commit 2545](https://github.com/manticoresoftware/manticoresearch/commit/25453e5387152c0575d1eda1b2beb89a49c7493a) рефакторинг ISphFieldFilter в версию с подсчётом ссылок
* [Commit 1366](https://github.com/manticoresoftware/manticoresearch/commit/1366ee06e828397074b69bec9265d7147170c60e) исправлены неопределённое поведение и ошибку сегментации при использовании strtod на строках без терминатора
* [Commit 94bc](https://github.com/manticoresoftware/manticoresearch/commit/94bc6fcbdae4d08bcf6da2966e3ec2966091289f) исправлена утечка памяти при обработке результатов JSON
* [Commit e78e](https://github.com/manticoresoftware/manticoresearch/commit/e78e9c948963416caff9e4e46296a58080107835) исправлен выход за пределы блока памяти при применении операции add к атрибуту
* [Commit fad5](https://github.com/manticoresoftware/manticoresearch/commit/fad572fb543606b33e688eb82a485b86011545fd) рефакторинг CSphDict для варианта с подсчётом ссылок
* [Commit fd84](https://github.com/manticoresoftware/manticoresearch/commit/fd841a472b472280c93f4af3506851d19f6adc45) исправлена утечка внутреннего типа AOT наружу
* [Commit 5ee7](https://github.com/manticoresoftware/manticoresearch/commit/5ee7f208c7685f2ebb9e5623a8802b721e2a7f0a) исправлена утечка памяти при управлении токенизатором
* [Commit 116c](https://github.com/manticoresoftware/manticoresearch/commit/116c5f1abebee9a0d99afe93546a1d8e4c6c6958) исправлена утечка памяти в grouper
* [Commit 56fd](https://github.com/manticoresoftware/manticoresearch/commit/56fdbc9ab38973a9a0ba8ccee45f71cf33c16423) специальное освобождение/копирование для динамических указателей в matches (утечка памяти в grouper)
* [Commit b1fc](https://github.com/manticoresoftware/manticoresearch/commit/31db660f378541aa0d1e7cf2e7209bb0a1f47fd8) исправлена утечка памяти динамических строк для RT
* [Commit 517b](https://github.com/manticoresoftware/manticoresearch/commit/517b9e8749f092e923cd2884b674b5bb84e20755) рефакторинг grouper
* [Commit b1fc](https://github.com/manticoresoftware/manticoresearch/commit/b1fc16140e5dc44290686330b476a254e0092107) небольшой рефакторинг (c++11 c-trs, некоторые переработки форматирования)
* [Commit 7034](https://github.com/manticoresoftware/manticoresearch/commit/7034e073f4cf2844762e0a464b7c3de05d3d122b) рефакторинг ISphMatchComparator в версию с подсчётом ссылок
* [Commit b1fc](https://github.com/manticoresoftware/manticoresearch/commit/b1fc16140e5dc44290686330b476a254e0092107) приватизация клонировщика
* [Commit efbc](https://github.com/manticoresoftware/manticoresearch/commit/efbc0511d6809c4a57453c7283f9bf53f3fb8d97) упрощение нативного little-endian для MVA_UPSIZE, DOCINFO2ID_T, DOCINFOSETID
* [Commit 6da0](https://github.com/manticoresoftware/manticoresearch/commit/6da0df4ac9946fc59f98d229b90e12c9221dd4c0) добавлена поддержка valgrind в ubertests
* [Commit 1d17](https://github.com/manticoresoftware/manticoresearch/commit/1d17669cb7ec54ac80b392e101f37a688eb98080) исправлен сбой из-за гонки флага 'success' на соединении
* [Commit 5a09](https://github.com/manticoresoftware/manticoresearch/commit/5a09c32d7ded56b2c0bd3e2ad7968cb8d6362919) переключение epoll на edge-triggered режим
* [Commit 5d52](https://github.com/manticoresoftware/manticoresearch/commit/5d528682737fca03a4352a3093b1319ec27d598c) исправлен оператор IN в выражении с форматированием, как у фильтра
* [Commit bd8b](https://github.com/manticoresoftware/manticoresearch/commit/bd8b3c976ff8b4667f55dfdd101b20a920137ac5) исправлен сбой RT индекса при коммите документа с большим docid
* [Commit ce65](https://github.com/manticoresoftware/manticoresearch/commit/ce656b83b747ce7c55795b53915770c13fb99cfe) исправлены опции без аргументов в indextool
* [Commit 08c9](https://github.com/manticoresoftware/manticoresearch/commit/08c9507177820f441f534bf06a11dac5e54bebb4) исправлена утечка памяти при расширенном ключевом слове

* [Commit 30c7](https://github.com/manticoresoftware/manticoresearch/commit/30c75a2f525ec9bda625d903acdc9d4d2e2413dc) исправлена утечка памяти json grouper
* [Commit 6023](https://github.com/manticoresoftware/manticoresearch/commit/6023f269b6f2080e4d380b0957605ef8107c8c9f) исправлена утечка глобальных пользовательских переменных
* [Commit 7c13](https://github.com/manticoresoftware/manticoresearch/commit/7c138f15ca23c0c8717fa12041e3db7f988fcf15) исправлена утечка динамических строк при раннем отвержении matches
* [Commit 9154](https://github.com/manticoresoftware/manticoresearch/commit/9154b18eaed5302e8330cb3a95bd968959ccb312) исправлен утечка памяти в length(<expression>)
* [Commit 43fc](https://github.com/manticoresoftware/manticoresearch/commit/43fca3a4e26139765d0fac8de054200dc4875fc6) исправлена утечка памяти из-за strdup() в парсере
* [Commit 71ff](https://github.com/manticoresoftware/manticoresearch/commit/71ff77737e63d1019b7c9804dca7fa2351025dba) рефакторинг парсера выражений для точного отслеживания счётчиков ссылок
## Версия 2.7.2 GA, 27 августа 2018
### Улучшения
* совместимость с клиентами MySQL 8
* [TRUNCATE](Emptying_a_table.md) С ПЕРЕИНИЦИАЛИЗАЦИЕЙ
* удалён счётчик памяти в SHOW STATUS для RT индексов
* глобальный кэш мультиагентов

* улучшена IOCP на Windows
* VIP-подключения для HTTP протокола
* Manticore SQL команда [DEBUG](Reporting_bugs.md#DEBUG), которая может выполнять различные подкоманды
* [shutdown_token](Server_settings/Searchd.md#shutdown_token) - SHA1 хэш пароля, необходимый для вызова `shutdown` через DEBUG команду
* новые статистики в SHOW AGENT STATUS (_ping, _has_perspool, _need_resolve)
* опция --verbose у indexer теперь принимает \[debugvv\] для вывода отладочных сообщений
### Исправления ошибок
* [Commit 3900](https://github.com/manticoresoftware/manticoresearch/commit/390082a7be0a1f9539b30361d11d54de35c62a44) удалён wlock при оптимизации
* [Commit 4c33](https://github.com/manticoresoftware/manticoresearch/commit/4c337648329be22373be33333cbc9c5664d18b80) исправлен wlock при перезагрузке настроек индекса
* [Commit b5ea](https://github.com/manticoresoftware/manticoresearch/commit/b5ea8dc0b18a23dbd739e92c66dd93b0094df14b) исправлена утечка памяти при запросе с JSON фильтром
* [Commit 930e](https://github.com/manticoresoftware/manticoresearch/commit/930e831a43fd3d1ce996185be968d1df49a088d0) исправлены пустые документы в наборе результатов PQ
* [Commit 53de](https://github.com/manticoresoftware/manticoresearch/commit/53deecfecb374431f84f8592a1095a77407b8aea) исправлена путаница задач из-за удаления одной из них
* [Commit cad9](https://github.com/manticoresoftware/manticoresearch/commit/cad9b9c617719aad97e9f5a2ae93491153d8e504) исправлен неправильный подсчёт удалённых хостов
* [Commit 9000](https://github.com/manticoresoftware/manticoresearch/commit/90008cc15c4684d445d37a391bb4a667bbc2e365) исправлена утечка памяти при разборе дескрипторов агентов
* [Commit 978d](https://github.com/manticoresoftware/manticoresearch/commit/978d839641ad6040c22c9fc3b703b1a02685f266) исправлена утечка при поиске
* [Commit 0193](https://github.com/manticoresoftware/manticoresearch/commit/0193946105ac08913362b7d7acf0c90f9b38dc85) косметические изменения в использовании explicit/inline c-trs, override/final
* [Commit 943e](https://github.com/manticoresoftware/manticoresearch/commit/943e2997b33f9b2eda23d94a66068910f9476ebd) исправлена утечка json в локальной/удалённой схеме
* [Commit 02db](https://github.com/manticoresoftware/manticoresearch/commit/02dbdd6f1d0b72e7e458ebebdd2c67f989577e6b) исправлена утечка json выражений сортировки столбца в локальной/удалённой схеме
* [Commit c74d](https://github.com/manticoresoftware/manticoresearch/commit/c74d0b4035e4214510376845b22cb676a8da24a3) исправлена утечка const alias
* [Commit 6e5b](https://github.com/manticoresoftware/manticoresearch/commit/6e5b57e1d3952c9695376728a143bf5434208f53) исправлена утечка потока preread
* [Commit 39c7](https://github.com/manticoresoftware/manticoresearch/commit/39c740fef5bf71da0bee60a35f5dd9f471af850f) исправлена блокировка при выходе из-за зависания ожидания в netloop
* [Commit adaf](https://github.com/manticoresoftware/manticoresearch/commit/adaf9772f95772ad6d5297acace6c5cb92b56ad5) исправлено зависание поведения 'ping' при смене HA-агента на обычный хост
* [Commit 32c4](https://github.com/manticoresoftware/manticoresearch/commit/32c40eac9cdd40b15d79e58b2f15416f164f9701) выделена отдельная сборка мусора для dashboard storage
* [Commit 511a](https://github.com/manticoresoftware/manticoresearch/commit/511a3cf2c1f1858f6e94fe9f8175b7216db3cbd6) исправлена проблема с ref-counted указателем
* [Commit 32c4](https://github.com/manticoresoftware/manticoresearch/commit/af2326cd2927effbad7bec88e8dc238a566cf27e) исправлен сбой indextool при отсутствии индекса
* [Commit 156e](https://github.com/manticoresoftware/manticoresearch/commit/156edc717dc63d3ea120f42466eba6e022da88be) исправлено имя выходного файла для превышающих предел атрибутов/полей в xmlpipe индексации
* [Commit cdac](https://github.com/manticoresoftware/manticoresearch/commit/cdac6d1f17d384bfc8bae49e91241b2f346c3830) исправлено значение индексера по умолчанию, если в конфиге нет секции индексера
* [Commit e61e](https://github.com/manticoresoftware/manticoresearch/commit/e61ec00b6b27d1d5878247e2ee817f3b1e7fde16) исправлены неправильные встроенные стоп-слова в дисковом чанке у RT индекса после перезапуска сервера

* [Commit 5fba](https://github.com/manticoresoftware/manticoresearch/commit/5fba49d5e28c17de4c0acbd984466127f42de6e8) пропущены фантомные (уже закрытые, но ещё не окончательно удалённые из poller) соединения
* [Commit f22a](https://github.com/manticoresoftware/manticoresearch/commit/f22ae34b623906b3c5a05a06198dad2e548dc541) исправлены смешанные (осиротевшие) сетевые задачи
* [Commit 4689](https://github.com/manticoresoftware/manticoresearch/commit/46890e70eb8dcd00c1bf1b030538b8f131c601c2) исправлен сбой при чтении после записи
* [Commit 03f9](https://github.com/manticoresoftware/manticoresearch/commit/03f9df904f611c438d5ebcaaeef911b0dc8d6c39) исправлены сбои searchd при запуске тестов на Windows
* [Commit e925](https://github.com/manticoresoftware/manticoresearch/commit/e9255ed2704790a3d7f5273d38b85433463cb3ff) исправлена обработка кода EINPROGRESS при обычном connect()
* [Commit 248b](https://github.com/manticoresoftware/manticoresearch/commit/248b72f1544b27e9e82773284050d44eeb1714e1) исправлены тайм-ауты подключения при работе с TFO

## Версия 2.7.1 GA, 4 июля 2018
### Улучшения
* улучшена производительность шаблонов при сопоставлении нескольких документов в PQ
* поддержка fullscan-запросов в PQ
* поддержка MVA атрибутов в PQ


* поддержка regexp и RLP для percolate индексов
### Исправления ошибок
* [Commit 6885](https://github.com/manticoresoftware/manticoresearch/commit/68856261b41f6950666f9e5122726839fb3c71d0) исправлена потеря строки запроса
* [Commit 0f17](https://github.com/manticoresoftware/manticoresearch/commit/0f1770943cba53bfd2e0edfdf0b1495ff0dd0e89) исправлена пустая информация в операторе SHOW THREADS
* [Commit 53fa](https://github.com/manticoresoftware/manticoresearch/commit/53faa36a916bb87868e83aa79c1a1f972fb20ca0) исправлен сбой при поиске с оператором NOTNEAR
* [Commit 2602](https://github.com/manticoresoftware/manticoresearch/commit/26029a7cd77b518fb3a27588d7db6c8bffc73b0f) исправлено сообщение об ошибке при неверном фильтре для PQ delete
## Версия 2.7.0 GA, 11 июня 2018
### Улучшения
* уменьшено количество системных вызовов для снижения влияния патчей Meltdown и Spectre
* внутренний переписанный механизм управления локальными индексами
* рефакторинг удалённых сниппетов
* полная перезагрузка конфигурации
* все соединения с узлами теперь независимы
* улучшения в протоколе
* в Windows коммуникация переведена с wsapoll на IO completion ports

* можно использовать TFO для связи между мастером и узлами
* [SHOW STATUS](Node_info_and_management/Node_status.md#SHOW-STATUS) теперь выводит server version и mysql_version_string
* добавлен параметр `docs_id` для документов, вызываемых в CALL PQ.
* фильтр запросов percolate теперь может содержать выражения
* распределённые индексы могут работать с FEDERATED
* заглушки для SHOW NAMES COLLATE и `SET wait_timeout` (для лучшей совместимости с ProxySQL)
### Исправления ошибок
* [Commit 5bcf](https://github.com/manticoresoftware/manticoresearch/commit/5bcff05d19e495f1a44f190885b25cbb6f217c4a) исправлено добавление операторов неравенства в теги PQ
* [Commit 9ebc](https://github.com/manticoresoftware/manticoresearch/commit/9ebc58916cd515eaa88da66d0895aebf0d1f2b5f) добавлено поле id документа в JSON документе CALL PQ
* [Commit 8ae0](https://github.com/manticoresoftware/manticoresearch/commit/8ae0e593a286110d8505d88baa3ac9e433cb38c9) исправлены обработчики оператора flush для индекса PQ
* [Commit c24b](https://github.com/manticoresoftware/manticoresearch/commit/c24b152344ecd77661566cddb803487f51d3c1aa) исправлена фильтрация PQ по JSON и строковым атрибутам
* [Commit 1b8b](https://github.com/manticoresoftware/manticoresearch/commit/1b8bdde19aaa362785ea4c33bdc019154f8fbac4) исправлен разбор пустой JSON строки


* [Commit 1ad8](https://github.com/manticoresoftware/manticoresearch/commit/1ad8a07dbfc1131913a8d92c261fbb48f934e8b7) исправлен сбой при выполнении мультизапроса с фильтрами OR
* [Commit 69b8](https://github.com/manticoresoftware/manticoresearch/commit/69b89806380a229e36287ad9daf6f0ea2b5453eb) исправлен indextool для использования общей секции конфигурации (опция lemmatizer_base) для команд (dumpheader)
* [Commit 6dbe](https://github.com/manticoresoftware/manticoresearch/commit/6dbeaf2389d2af6a9dfccb56bfc986685b9f203e) исправлена пустая строка в результирующем наборе и фильтре
* [Commit 39c4](https://github.com/manticoresoftware/manticoresearch/commit/39c4eb55ecc85f6dd54f6c8c6d6dfcf87fd8748e) исправлены отрицательные значения id документов
* [Commit 266b](https://github.com/manticoresoftware/manticoresearch/commit/266b7054f98e7d30fc2829b5c2467dc9f90def92) исправлена длина обрезки слова для очень длинных слов в индексе
* [Commit 4782](https://github.com/manticoresoftware/manticoresearch/commit/47823b0bd06cbd016a24345c99bb8693790b43dd) исправлено сопоставление нескольких документов запросов с подстановочными знаками в PQ
## Версия 2.6.4 GA, 3 мая 2018

### Новые возможности и улучшения
* поддержка механизма MySQL FEDERATED [support](Extensions/FEDERATED.md)
* пакеты MySQL теперь возвращают флаг SERVER_STATUS_AUTOCOMMIT, повышая совместимость с ProxySQL
* [listen_tfo](Server_settings/Searchd.md#listen_tfo) - включение TCP Fast Open для всех слушателей
* indexer --dumpheader теперь может выгружать RT заголовок из файла .meta
* скрипт сборки cmake для Ubuntu Bionic
### Исправления ошибок
* [Commit 355b](https://github.com/manticoresoftware/manticoresearch/commit/355b11629174813abd9cd5bf2233be0783f77745) исправлены неверные записи кэша запросов для RT индекса;

* [Commit 546e](https://github.com/manticoresoftware/manticoresearch/commit/546e2297a01cc2913bc0d33052d5bcefae8737eb) исправлено пропадание настроек индекса после сквозной ротации
* [Commit 0c45](https://github.com/manticoresoftware/manticoresearch/commit/0c4509898393993bc87194d2aca2070395ff7f83) исправлены настройки длины инфиксов и префиксов; добавлено предупреждение о неподдерживаемой длине инфикса
* [Commit 8054](https://github.com/manticoresoftware/manticoresearch/commit/80542fa54d727c781635918e8d4d65a20c2fbd1e) исправлен порядок автофлаширования RT индексов

* [Commit 705d](https://github.com/manticoresoftware/manticoresearch/commit/705d8c5f407726ed1c6f9745ecffa375682c8969) исправлены ошибки схемы результирующего набора для индексов с множеством атрибутов и запросов к нескольким индексам
* [Commit b0ba](https://github.com/manticoresoftware/manticoresearch/commit/b0ba932fff4a082f642cb737381c8a70215d4cc9) исправлено потеря некоторых результатов при пакетной вставке с дубликатами документов
* [Commit 4510](https://github.com/manticoresoftware/manticoresearch/commit/4510fa44ad54b5bbac53c591144501565130fcae) исправлен сбой optimize при невозможности объединить дисковые куски RT индекса с большим числом документов
## Версия 2.6.3 GA, 28 марта 2018
### Улучшения
* jemalloc при компиляции. Если jemalloc доступен в системе, его можно активировать флагом cmake `-DUSE_JEMALLOC=1`


### Исправления ошибок
* [Commit 85a6](https://github.com/manticoresoftware/manticoresearch/commit/85a6d7e3db75f28c491042409bb677ed39eba07b) исправлена опция log expand_keywords в логе запросов Manticore SQL
* [Commit caaa](https://github.com/manticoresoftware/manticoresearch/commit/caaa3842f4dbd8507fd9e20bf3371a0c8ec0c087) исправлен HTTP интерфейс для корректной обработки запросов большого размера
* [Commit e386](https://github.com/manticoresoftware/manticoresearch/commit/e386d842b36195719e459b2db1fb20e402f9a51f) исправлен сбой сервера при DELETE в RT индекс с включённым index_field_lengths
* [Commit cd53](https://github.com/manticoresoftware/manticoresearch/commit/ce528f37eb5d8284661928f6f3b340a4a1985253) исправлена опция cpustats для searchd cli для работы на неподдерживаемых системах
* [Commit 8740](https://github.com/manticoresoftware/manticoresearch/commit/8740fd63ae842c2ab0e51a8fcd6180d787ed57ec) исправлено сопоставление подстрок utf8 с определёнными минимальными длинами
## Версия 2.6.2 GA, 23 февраля 2018
### Улучшения

* улучшена производительность [Percolate Queries](Searching/Percolate_query.md) при использовании оператора NOT и для пакетных документов.
* [percolate_query_call](Searching/Percolate_query.md) теперь может использовать несколько потоков в зависимости от [dist_threads](Server_settings/Searchd.md#threads)
* новый оператор полного текстового сопоставления NOTNEAR/N
* LIMIT для SELECT на перколяторных индексах
* [expand_keywords](Searching/Options.md#expand_keywords) теперь принимает 'start','exact' (где 'star,exact' имеет такой же эффект, как '1')
* ranged-main-query для [joined fields](Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Processing_fetched_data.md#sql_joined_field), который использует ranged query, определенный sql_query_range
### Исправления ошибок
* [Commit 72dc](https://github.com/manticoresoftware/manticoresearch/commit/72dcf669744e9b7d636dfc213d24df85ab301f6b) исправлен сбой при поиске в ram-сегментах; устранен дедлок при сохранении чанка на диск с двойным буфером; устранен дедлок при сохранении чанка на диск во время оптимизации
* [Commit 3613](https://github.com/manticoresoftware/manticoresearch/commit/36137149a1c3c0893bdda5a28fc7e8244bf2d4ae) исправлен сбой индексатора на xml-схеме с пустым именем атрибута


* [Commit 48d7](https://github.com/manticoresoftware/manticoresearch/commit/48d7e8001d2a66466ca64577f27ddc5421a67251) исправлено ошибочное удаление pid-файла, не принадлежащего процессу
* [Commit a556](https://github.com/manticoresoftware/manticoresearch/commit/a5563a465ddc59ef71e65f17b68bc33f9700e838) исправлено оставление сиротских fifo иногда в временной папке
* [Commit 2376](https://github.com/manticoresoftware/manticoresearch/commit/2376e8fc4508944b96959bd10686c6d51f5145e8) исправлено пустое множество результатов FACET с неверной строкой NULL
* [Commit 4842](https://github.com/manticoresoftware/manticoresearch/commit/4842b67acdd5da75672db6c08ac563b48577ddd7) исправлен сломанный блокировщик индекса при запуске сервера как службы Windows
* [Commit be35](https://github.com/manticoresoftware/manticoresearch/commit/be35feef54ded59125750916cd211e56108eddae) исправлены некорректные библиотеки iconv на Mac OS
* [Commit 8374](https://github.com/manticoresoftware/manticoresearch/commit/83744a977d7837d1d87cd506b88b0e7eb199efe6) исправлен неверный count(\*)
## Версия 2.6.1 GA, 26 января 2018
### Улучшения

* [agent_retry_count](Server_settings/Searchd.md#agent_retry_count) для агентов с зеркалами теперь указывает количество повторов на зеркало, а не на агента, общее количество повторов на агента равно agent_retry_count\*mirrors.
* [agent_retry_count](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_retry_count) теперь можно задать для каждого индекса, переопределяя глобальное значение. Добавлен алиас [mirror_retry_count](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#mirror_retry_count).
* retry_count теперь можно указать в определении агента, и значение означает количество попыток на агента
* Перколяторные запросы теперь доступны через HTTP JSON API по адресу [/json/pq](Searching/Percolate_query.md#Performing-a-percolate-query-with-CALL-PQ).
* Добавлены опции -h и -v (справка и версия) для исполняемых файлов
* [morphology_skip_fields](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) добавлена поддержка для Real-Time индексов
### Исправления ошибок


* [Commit a40b](https://github.com/manticoresoftware/manticore/commit/a40b0793feff65e40d10062568d9847c08d10f57) исправлен ranged-main-query для корректной работы с sql_range_step при использовании с полем MVA
* [Commit f2f5](https://github.com/manticoresoftware/manticore/commit/f2f53757db45bcfb1544263ce0817e856656a621) исправлена проблема зависания system loop в blackhole и отключения агентов blackhole
* [Commit 84e1](https://github.com/manticoresoftware/manticore/commit/84e1f54aef25e0fce98870ad2dd784db5116f1d6) исправлен идентификатор запроса для согласованности, устранено дублирование id для сохраненных запросов
* [Commit 1948](https://github.com/manticoresoftware/manticore/commit/19484231814fcb82b21763a3a4a9f45adc6b2d40) исправлен сбой сервера при завершении работы из различных состояний
* [Commit 9a70](https://github.com/manticoresoftware/manticore/commit/9a706b499a1d61a90076065a1a703029d49db958)  [Commit 3495](https://github.com/manticoresoftware/manticore/commit/3495fd70cba8846b1a50d55d6679b039414c3d2a) исправлены тайм-ауты на длительных запросах
* [Commit 3359](https://github.com/manticoresoftware/manticore/commit/3359bcd89b4f79a645fe84b8cf8616ce0addff02) переработан опрос сети master-agent на системах с kqueue (Mac OS X, BSD).
## Версия 2.6.0, 29 декабря 2017

### Новые возможности и улучшения
* [HTTP JSON](Connecting_to_the_server/HTTP.md): JSON-запросы теперь могут сравнивать атрибуты на равенство, MVA и JSON-атрибуты могут использоваться при вставках и обновлениях, обновления и удаления через JSON API возможны для распределенных индексов
* [Percolate Queries](Searching/Percolate_query.md)
* Удалена поддержка 32-битных docid из кода. Также удален весь код, который переводил/загружал устаревшие индексы с 32-битными docid.
* [Морфология только для определенных полей](https://github.com/manticoresoftware/manticore/issues/7) . Новая директива индекса morphology_skip_fields позволяет определить список полей, для которых морфология не применяется.
* [expand_keywords теперь может быть директивой выполнения запроса и устанавливаться с помощью оператора OPTION](https://github.com/manticoresoftware/manticore/issues/8)

### Исправления ошибок
* [Commit 0cfa](https://github.com/manticoresoftware/manticore/commit/0cfae4c) исправлен сбой при отладочной сборке сервера (и, возможно, UB в релизе) при сборке с rlp

* [Commit 3242](https://github.com/manticoresoftware/manticore/commit/324291e) исправлена оптимизация RT индекса с включенной опцией progressive, которая сливает kill-листы в неверном порядке
* [Commit ac0e](https://github.com/manticoresoftware/manticore/commit/ac0efee) незначительный сбой на Mac
* множество мелких исправлений после тщательного статического анализа кода
* другие мелкие исправления ошибок
### Обновление
В этом релизе протокол внутреннего общения master и agents изменен. Если вы используете Manticoresearch в распределенной среде с несколькими инстансами, обязательно сначала обновите агенты, затем мастера.
## Версия 2.5.1, 23 ноября 2017
### Новые возможности и улучшения
* JSON-запросы по [HTTP API протоколу](Connecting_to_the_server/HTTP.md). Поддерживаются операции поиска, вставки, обновления, удаления, замены. Команды обработки данных также могут выполняться пакетно, однако в настоящее время существуют некоторые ограничения: MVA и JSON-атрибуты не могут использоваться для вставок, замен или обновлений.
* Команда [RELOAD INDEXES](Data_creation_and_modification/Adding_data_from_external_storages/Rotating_a_table.md#RELOAD-TABLES)
* Команда [FLUSH LOGS](Logging/Rotating_query_and_server_logs.md)

* Команда [SHOW THREADS](Node_info_and_management/SHOW_THREADS.md) может показывать прогресс оптимизации, ротации или сбросов.
* GROUP N BY корректно работает с MVA-атрибутами
* агенты blackhole запускаются в отдельном потоке, чтобы больше не влиять на запросы мастера
* реализован подсчет ссылок на индексы для избежания задержек, вызванных ротацией и высокой нагрузкой
* реализован SHA1-хэшинг, пока еще не доступен внешне
* исправления для компиляции на FreeBSD, macOS и Alpine
### Исправления ошибок
* [Коммит 9897](https://github.com/manticoresoftware/manticore/commit/989752b) регрессия фильтра с блочным индексом

* [Коммит b1c3](https://github.com/manticoresoftware/manticore/commit/b1c3864) переименование PAGE_SIZE -> ARENA_PAGE_SIZE для совместимости с musl
* [Коммит f213](https://github.com/manticoresoftware/manticore/commit/f2133cc) отключение googletests для cmake < 3.1.0
* [Коммит f30e](https://github.com/manticoresoftware/manticore/commit/0839de7) ошибка при привязке сокета при перезапуске сервера
* [Коммит 0807](https://github.com/manticoresoftware/manticore/commit/0807240) фикс краха сервера при завершении работы
* [Коммит 3e3a](https://github.com/manticoresoftware/manticore/commit/3e3acc3) исправлен show threads для системного потока blackhole
* [Коммит 262c](https://github.com/manticoresoftware/manticore/commit/262c3fe) рефакторинг проверки config iconv, исправления сборки на FreeBSD и Darwin
## Версия 2.4.1 GA, 16 октября 2017
### Новые возможности и улучшения
* Оператор OR в предложении WHERE между фильтрами атрибутов
* Режим обслуживания ( SET MAINTENANCE=1)
* [CALL KEYWORDS](Searching/Autocomplete.md#CALL-KEYWORDS) доступны для распределенных индексов
* [Группировка в UTC](Server_settings/Searchd.md#grouping_in_utc)
* [query_log_mode](Server_settings/Searchd.md#query_log_mode) для пользовательских разрешений файлов журналов
* Весовые коэффициенты полей могут быть нулевыми или отрицательными

* [max_query_time](Searching/Options.md#max_query_time) теперь может влиять на полные сканирования
* добавлены [net_wait_tm](Server_settings/Searchd.md#net_wait_tm), [net_throttle_accept](Server_settings/Searchd.md#net_throttle_accept) и [net_throttle_action](Server_settings/Searchd.md#net_throttle_action) для тонкой настройки сетевого потока (в случае workers=thread_pool)

* COUNT DISTINCT работает с поиском по фасетам
* IN может использоваться с JSON-массивами чисел с плавающей точкой
* оптимизация мультизапросов больше не ломается из-за целочисленных или числовых выражений
* [SHOW META](Node_info_and_management/SHOW_META.md) показывает строку `multiplier` при использовании оптимизации мультизапроса
### Компиляция

Manticore Search собирается с помощью cmake, минимальная требуемая версия gcc для компиляции — 4.7.2.
### Каталоги и сервис
* Manticore Search запускается под пользователем `manticore`.
* Папка данных по умолчанию теперь `/var/lib/manticore/`.
* Папка журналов по умолчанию теперь `/var/log/manticore/`.
* Папка для pid-файлов по умолчанию теперь `/var/run/manticore/`.
### Исправления ошибок
* [Коммит a58c](https://github.com/manticoresoftware/manticore/commit/a58c619) исправлена команда SHOW COLLATION, нарушающая работу java-коннектора
* [Коммит 631c](https://github.com/manticoresoftware/manticore/commit/631cf4e) исправлены сбои при обработке распределённых индексов; добавлены блокировки в хэш распределённого индекса; удалены операторы перемещения и копирования из агента
* [Коммит 942b](https://github.com/manticoresoftware/manticore/commit/942bec0) исправлены сбои при обработке распределённых индексов из-за параллельных переподключений
* [Коммит e5c1](https://github.com/manticoresoftware/manticore/commit/e5c1ed2) исправлен сбой оброботчика сбоев при записи запроса в журнал сервера
* [Коммит 4a4b](https://github.com/manticoresoftware/manticore/commit/4a4bda5) исправлен сбой с pooled-атрибутами в мультизапросах
* [Коммит 3873](https://github.com/manticoresoftware/manticore/commit/3873bfb) уменьшен размер core-файла за счет предотвращения включения страниц индекса
* [Коммит 11e6](https://github.com/manticoresoftware/manticore/commit/11e6254) исправлены сбои searchd при старте при указании недопустимых агентов
* [Коммит 4ca6](https://github.com/manticoresoftware/manticore/commit/4ca6350) исправлена ошибка индекса при ошибке в sql_query_killlist
* [Коммит 123a](https://github.com/manticoresoftware/manticore/commit/123a9f0) исправлена работа fold_lemmas=1 относительно количества совпадений
* [Коммит cb99](https://github.com/manticoresoftware/manticore/commit/cb99164) исправлено неконсистентное поведение html_strip
* [Коммит e406](https://github.com/manticoresoftware/manticore/commit/e406761) исправлена потеря новых настроек в optimize rt index; исправлены утечки блокировок при optimize с опцией sync;

* [Коммит 86ae](https://github.com/manticoresoftware/manticore/commit/86aeb82) исправлена обработка ошибочных мультизапросов
* [Коммит 2645](https://github.com/manticoresoftware/manticore/commit/2645230) исправлена зависимость результата от порядка мультизапросов

