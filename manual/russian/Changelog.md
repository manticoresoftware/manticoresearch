# Changelog

## Version 13.11.1
**Released**: September 13th 2025

### Bug Fixes
* 🪲 [v13.11.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.11.1) [ Issue #3751](https://github.com/manticoresoftware/manticoresearch/issues/3751) Исправлен сбой при удалении документа с включенными встраиваниями.

## Version 13.11.0
**Released**: September 13th 2025

Главной особенностью этого релиза является [Auto Embeddings](Searching/KNN.md#Auto-Embeddings-%28Recommended%29) — новая функция, которая делает семантический поиск таким же простым, как SQL.  
Не нужны внешние сервисы или сложные конвейеры: просто вставьте текст и ищите с помощью естественного языка.

### Что предлагают Auto Embeddings

- **Автоматическое создание встраиваний** непосредственно из вашего текста  
- **Запросы на естественном языке**, которые понимают смысл, а не только ключевые слова  
- **Поддержка нескольких моделей** (OpenAI, Hugging Face, Voyage, Jina)  
- **Бесшовная интеграция** с SQL и JSON API  

### Рекомендуемые библиотеки
- Рекомендуемая версия [MCL](https://github.com/manticoresoftware/columnar): 8.0.3
- Рекомендуемая версия [Buddy](Installation/Manticore_Buddy.md#Manticore-Buddy): 3.35.1

Если вы следуете [официальному руководству по установке](https://manticoresearch.com/install/), вам не нужно об этом беспокоиться.

### Новые функции и улучшения
* 🆕 [v13.11.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.11.0) [ PR #3746](https://github.com/manticoresoftware/manticoresearch/pull/3746) Добавлена поддержка "query" в JSON-запросах для генерации встраиваний.
* 🆕 [v13.10.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.10.0) [ Issue #3709](https://github.com/manticoresoftware/manticoresearch/issues/3709) RPM-пакет manticore-server больше не владеет `/run`.
* 🆕 [v13.9.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.9.0) [ PR #3716](https://github.com/manticoresoftware/manticoresearch/pull/3716) Добавлена поддержка `boolean_simplify` в конфигурации.
* 🆕 [v13.8.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.8.0) [ Issue #3253](https://github.com/manticoresoftware/manticoresearch/issues/3253) Установлен конфиг sysctl для увеличения vm.max_map_count для больших наборов данных.
* 🆕 [v13.7.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.7.0) [ PR #3681](https://github.com/manticoresoftware/manticoresearch/pull/3681) Добавлена поддержка `alter table <table> modify column <column> api_key=<key>`.

### Исправления ошибок
* 🪲 [v13.10.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.10.5) [ PR #3737](https://github.com/manticoresoftware/manticoresearch/pull/3737) Опция scroll теперь корректно возвращает все документы с большими 64-битными ID.
* 🪲 [v13.10.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.10.4) [ PR #3736](https://github.com/manticoresoftware/manticoresearch/pull/3736) Исправлен сбой при использовании KNN с фильтром-деревом.
* 🪲 [v13.10.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.10.3) [ Issue #3520](https://github.com/manticoresoftware/manticoresearch/issues/3520) Эндпоинт `/sql` больше не разрешает команду SHOW VERSION.
* 🪲 [v13.10.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.10.2) [ PR #3637](https://github.com/manticoresoftware/manticoresearch/pull/3637) Обновлен скрипт установщика для Windows.
* 🪲 [v13.10.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.10.1) Исправлено определение локального часового пояса в Linux.
* 🪲 [v13.9.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.9.2) [ PR #3726](https://github.com/manticoresoftware/manticoresearch/pull/3726) В режиме columnar дублирующиеся ID теперь корректно возвращают ошибку.
* 🪲 [v13.9.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.9.1) [ PR #3333](https://github.com/manticoresoftware/manticoresearch/pull/3333) Руководство теперь автоматически переводится.
* 🪲 [v13.8.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.8.6) [ PR #3715](https://github.com/manticoresoftware/manticoresearch/pull/3715) Исправлена проблема генерации встраиваний, когда все документы в пакете пропускались.
* 🪲 [v13.8.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.8.5) [ PR #3711](https://github.com/manticoresoftware/manticoresearch/pull/3711) Добавлены модели встраиваний Jina и Voyage, а также другие изменения, связанные с автоматическими встраиваниями.
* 🪲 [v13.8.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.8.4) [ PR #3710](https://github.com/manticoresoftware/manticoresearch/pull/3710) Исправлен сбой при объединенных запросах с несколькими фасетами.
* 🪲 [v13.8.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.8.3) Исправлена проблема, когда коммиты удаления/обновления в транзакции с несколькими выражениями на эндпоинте _bulk не учитывались как ошибки.
* 🪲 [v13.8.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.8.2) [ PR #3705](https://github.com/manticoresoftware/manticoresearch/pull/3705) Исправлен сбой при объединении по не columnar строковым атрибутам и улучшено сообщение об ошибках.
* 🪲 [v13.8.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.8.1) [ PR #3704](https://github.com/manticoresoftware/manticoresearch/pull/3704) Исправлен сбой в query embeddings, когда модель не была указана; добавлена строка встраиваний для коммуникации master-agent; добавлены тесты.
* 🪲 [v13.7.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.7.2) [ PR #Buddy#589](https://github.com/manticoresoftware/manticoresearch-buddy/pull/589) Удалена хак-правка IDF по умолчанию для нечеткого поиска.
* 🪲 [v13.7.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.7.1) [ Issue #3454](https://github.com/manticoresoftware/manticoresearch/issues/3454) Исправлено некорректное декодирование scroll с большими 64-битными ID.
* 🪲 [v13.6.9](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.9) [ Issue #3674](https://github.com/manticoresoftware/manticoresearch/issues/3674) Исправлена проблема с драйвером/пулом подключений JDBC+MySQL при настройке transaction_read_only.
* 🪲 [v13.6.8](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.8) [ PR #3676](https://github.com/manticoresoftware/manticoresearch/pull/3676) Исправлен сбой при получении пустого набора результатов от модели embedding.

## Version 13.6.7
**Выпущено**: 8 августа 2025

Рекомендуемая версия [MCL](https://github.com/manticoresoftware/columnar): 8.0.1  
Рекомендуемая версия [Buddy](Installation/Manticore_Buddy.md#Manticore-Buddy): 3.34.2  

Если вы следуете [официальному руководству по установке](https://manticoresearch.com/install/), то не нужно об этом беспокоиться.

### Новые возможности и улучшения
* 🆕 [v13.6.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.0) [ Issue #2226](https://github.com/manticoresoftware/manticoresearch/issues/2226) Поддержка явного оператора '|' (ИЛИ) в PHRASE, PROXIMITY и QUORUM операторах.
* 🆕 [v13.5.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.5.0) [ PR #3591](https://github.com/manticoresoftware/manticoresearch/pull/3591) Автоматическая генерация embedding в запросах (в разработке, еще не готово для продакшена).
* 🆕 [v13.4.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.4.0) [ PR #3585](https://github.com/manticoresoftware/manticoresearch/pull/3585) Исправлена логика выбора количества потоков Buddy из конфигурации buddy_path, если указано, вместо использования значения демона.
* 🆕 [v13.3.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.3.0) [ PR #3577](https://github.com/manticoresoftware/manticoresearch/pull/3577) Поддержка join с локальными распределенными таблицами.
* 🆕 [#3647](https://github.com/manticoresoftware/manticoresearch/issues/3647) Добавлена поддержка Debian 13 "Trixie"

### Исправления ошибок
* 🪲 [v13.6.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.7) [ Issue #3524](https://github.com/manticoresoftware/manticoresearch/issues/3524) Исправлена ошибка сохранения сгенерированных embedding в построчном хранении.
* 🪲 [v13.6.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.6) [ Issue #3563](https://github.com/manticoresoftware/manticoresearch/issues/3563) Исправлены проблемы с Sequel Ace и другими интеграциями, вызывавшими ошибки "unknown sysvar".
* 🪲 [v13.6.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.5) [ Issue #3467](https://github.com/manticoresoftware/manticoresearch/issues/3467) Исправлены проблемы с DBeaver и другими интеграциями, вызывавшими ошибки "unknown sysvar".
* 🪲 [v13.6.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.4) [ Issue #3524](https://github.com/manticoresoftware/manticoresearch/issues/3524) Исправлена ошибка конкатенации multi-field embedding; также исправлена генерация embedding из запросов.
* 🪲 [v13.6.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.3) [ Issue #3641](https://github.com/manticoresoftware/manticoresearch/issues/3641) Исправлена ошибка в версии 13.6.0, когда фраза теряла все ключевые слова в скобках, кроме первого.
* 🪲 [v13.6.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.2) Исправлена утечка памяти в transform_phrase.
* 🪲 [v13.6.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.1) Исправлена утечка памяти в версии 13.6.0.
* 🪲 [v13.5.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.5.2) [ Issue #3651](https://github.com/manticoresoftware/manticoresearch/issues/3651) Исправлены дополнительные проблемы, связанные с фузз-тестированием полнотекстового поиска.
* 🪲 [v13.5.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.5.1) [ Issue #3560](https://github.com/manticoresoftware/manticoresearch/issues/3560) Исправлен случай, когда OPTIMIZE TABLE могла зависать бесконечно при использовании данных KNN.
* 🪲 [v13.4.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.4.2) [ Issue #2544](https://github.com/manticoresoftware/manticoresearch/issues/2544) Исправлена проблема, при которой добавление столбца float_vector могло повреждать индексы.
* 🪲 [v13.4.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.4.1) [ Issue #3651](https://github.com/manticoresoftware/manticoresearch/issues/3651) Добавлено фузз-тестирование для разбора полнотекстового поиска и исправлено несколько проблем, выявленных в ходе тестирования.
* 🪲 [v13.3.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.3.1) [ Issue #3583](https://github.com/manticoresoftware/manticoresearch/issues/3583) Исправлен сбой при использовании сложных булевых фильтров с подсветкой.
* 🪲 [v13.2.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.7) [ Issue #3481](https://github.com/manticoresoftware/manticoresearch/issues/3481) Исправлен сбой при использовании HTTP-обновления, распределенной таблицы и неправильного кластера репликации вместе.
* 🪲 [v13.2.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.6) [ PR #3567](https://github.com/manticoresoftware/manticoresearch/pull/3567) Обновлена зависимость manticore-backup до версии 1.9.6.
* 🪲 [v13.2.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.5) [ PR #3565](https://github.com/manticoresoftware/manticoresearch/pull/3565) Исправлена настройка CI для улучшения совместимости Docker-образов.
* 🪲 [v13.2.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.4) Исправлена обработка длинных токенов. Некоторые специальные токены (например, паттерны regex) могли создавать слова слишком большой длины, теперь они укорачиваются перед использованием.

## Version 13.2.3
**Выпущено**: 8 июля 2025

### Несовместимые изменения
* ⚠️ [v13.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.0.0)  Обновлен API библиотеки KNN для поддержки пустых значений [float_vector](https://manual.manticoresearch.com/Creating_a_table/Data_types#Float-vector). Это обновление не изменяет формат данных, но увеличивает версию API Manticore Search / MCL.
* ⚠️ [v12.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/12.0.0) [ PR #3516](https://github.com/manticoresoftware/manticoresearch/pull/3516) Исправлена ошибка с некорректными исходными и целевыми идентификаторами строк при обучении и построении KNN индекса. Это обновление не меняет формат данных, но увеличивает версию API Manticore Search / MCL.
* ⚠️ [v11.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.0.0) Добавлена поддержка новых функций поиска KNN векторов, таких как квантование, пересчет баллов и сверхвыборка. Эта версия меняет формат данных KNN и [синтаксис SQL KNN_DIST()](https://manual.manticoresearch.com/Searching/KNN?client=SQL#KNN-vector-search). Новая версия может читать старые данные, но старые версии не смогут прочитать новый формат.

### Новые функции и улучшения
* 🆕 [v13.2.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.0) [ PR #3549](https://github.com/manticoresoftware/manticoresearch/issues/3525) Исправлена проблема с `@@collation_database`, вызывавшая несовместимость с некоторыми версиями mysqldump
* 🆕 [v13.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.1.0) [ Issue #3489](https://github.com/manticoresoftware/manticoresearch/issues/3489) Исправлена ошибка в Fuzzy Search, препятствовавшая разбору некоторых SQL-запросов
* 🆕 [v12.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/12.1.0) [ Issue #3426](https://github.com/manticoresoftware/manticoresearch/issues/3426) Добавлена поддержка операционных систем RHEL 10
* 🆕 [v11.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.1.0) Добавлена поддержка пустых векторов с плавающей точкой в [поиске KNN](https://manual.manticoresearch.com/Searching/KNN#KNN-vector-search)
* 🆕 [v10.2.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.0) [ Issue #3252](https://github.com/manticoresoftware/manticoresearch/issues/3252) [log_level](https://manual.manticoresearch.com/Server_settings/Setting_variables_online#SET) теперь также регулирует уровень детализации логирования Buddy

### Исправления ошибок
* 🪲 [v13.2.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.3) [ PR #3556](https://github.com/manticoresoftware/manticoresearch/pull/3556) Исправлен разбор опции "oversampling" в JSON-запросах
* 🪲 [v13.2.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.2) Исправлена ошибка логирования аварийных завершений в Linux и FreeBSD путем удаления использования Boost stacktrace
* 🪲 [v13.2.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.1) [ Issue #3298](https://github.com/manticoresoftware/manticoresearch/issues/3298) Исправлено логирование аварийных завершений при запуске в контейнере
* 🪲 [v12.0.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/12.0.2) Повышена точность статистики по таблицам за счет учета микросекунд
* 🪲 [v12.0.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/12.0.1) [ PR #3522](https://github.com/manticoresoftware/manticoresearch/pull/3522) Исправлен сбой при фасетировании по MVA в объединенном запросе
* 🪲 [v11.0.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.0.3) [ Issue #3502](https://github.com/manticoresoftware/manticoresearch/issues/3502) Исправлен сбой, связанный с квантованием поиска по векторам
* 🪲 [v11.0.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.0.2) [ Issue #3493](https://github.com/manticoresoftware/manticoresearch/issues/3493) Изменен вывод CPU использования в `SHOW THREADS` на целое число
* 🪲 [v11.0.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.0.1) Исправлены пути для колоннарных и вторичных библиотек
* 🪲 [v10.2.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.7) Добавлена поддержка MCL 5.0.5, включая исправление имени файла библиотеки embeddings
* 🪲 [v10.2.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.6) [ Issue #3469](https://github.com/manticoresoftware/manticoresearch/issues/3469) Применено очередное исправление, связанное с issue #3469
* 🪲 [v10.2.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.4) [ Issue #3469](https://github.com/manticoresoftware/manticoresearch/issues/3469) Исправлена проблема с HTTP-запросами с bool-выражениями, возвращавших пустые результаты
* 🪲 [v10.2.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.5) Изменено имя файла библиотеки embeddings по умолчанию и добавлены проверки поля 'from' в KNN-векторах
* 🪲 [v10.2.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.3) [ PR #3464](https://github.com/manticoresoftware/manticoresearch/pull/3464) Обновлен Buddy до версии 3.30.2, которая включает [PR #565 по использованию памяти и логированию ошибок](https://github.com/manticoresoftware/manticoresearch-buddy/pull/565)
* 🪲 [v10.2.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.2) [ Issue #3470](https://github.com/manticoresoftware/manticoresearch/pull/3470) Исправлены фильтры строк JSON, фильтры null и проблемы с сортировкой в JOIN-запросах
* 🪲 [v10.2.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.1) Исправлена ошибка в `dist/test_kit_docker_build.sh`, приводившая к отсутствию коммита Buddy
* 🪲 [v10.1.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.4) Исправлен сбой при группировке по MVA в объединенном запросе
* 🪲 [v10.1.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.3) [ Issue #3434](https://github.com/manticoresoftware/manticoresearch/issues/3434) Исправлена ошибка при фильтрации пустых строк
* 🪲 [v10.1.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.2) [ PR #3452](https://github.com/manticoresoftware/manticoresearch/pull/3452) Обновлен Buddy до версии 3.29.7, что устраняет [Buddy #507 - ошибка при многозапросах с нечёткостью поиска](https://github.com/manticoresoftware/manticoresearch-buddy/issues/507) и [Buddy #561 - Исправление для метрик rate](https://github.com/manticoresoftware/manticoresearch-buddy/pull/561), необходимо для [Helm Release 10.1.0](https://github.com/manticoresoftware/manticoresearch-helm/releases/tag/manticoresearch-10.1.0)
* 🪲 [v10.1.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.1)  Обновлен Buddy до версии 3.29.4, исправляющий [#3388 - "Неопределенный ключ массива 'Field'"](https://github.com/manticoresoftware/manticoresearch/issues/3388) и [Buddy #547 - layouts='ru' может не работать](https://github.com/manticoresoftware/manticoresearch-buddy/issues/547)

## Версия 10.1.0
**Выпущено**: 9 июня 2025 г.

Эта версия представляет собой обновление с новыми функциями, одним критическим изменением и многочисленными улучшениями стабильности и исправлениями ошибок. Изменения направлены на расширение возможностей мониторинга, улучшение функциональности поиска и исправление различных критических проблем, влияющих на стабильность и производительность системы.

**Начиная с версии 10.1.0, CentOS 7 больше не поддерживается. Пользователям рекомендуется перейти на поддерживаемую операционную систему.**

### Критические изменения
* ⚠️ [v10.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.0.0) [ Issue #540](https://github.com/manticoresoftware/manticoresearch-buddy/issues/540) КРИТИЧЕСКОЕ ИЗМЕНЕНИЕ: установите `layouts=''` по умолчанию для [нечёткого поиска](Searching/Spell_correction.md#Fuzzy-Search)

### Новые функции и улучшения
* 🆕 [v10.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.0) [ Issue #537](https://github.com/manticoresoftware/manticoresearch-buddy/issues/537) Добавлен встроенный [Prometheus экспортер](Node_info_and_management/Node_status.md#Prometheus-Exporter)
* 🆕 [v9.8.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.8.0) [ Issue #3409](https://github.com/manticoresoftware/manticoresearch/issues/3409) Добавлена команда [ALTER TABLE tbl REBUILD KNN](Updating_table_schema_and_settings.md#Rebuilding-a-KNN-index)
* 🆕 [v9.7.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.7.0) [ Issue #1778](https://github.com/manticoresoftware/manticoresearch/issues/1778) Добавлено автоматическое создание embedding-ов (пока не объявляем официально, так как код в основной ветке, но требует дополнительного тестирования)
* 🆕 [v9.6.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.6.0)  Обновлена версия KNN API для поддержки авто-embedding-ов
* 🆕 [v9.5.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.0) [ Issue #1894](https://github.com/manticoresoftware/manticoresearch/issues/1894) Улучшено восстановление кластера: периодически сохраняется `seqno` для более быстрого перезапуска узла после сбоя
* 🆕 [v9.4.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.4.0) [ Issue #2400](https://github.com/manticoresoftware/manticoresearch/issues/2400) Добавлена поддержка последних версий [Logstash](Integration/Logstash.md#Integration-with-Logstash) и Beats

### Исправления ошибок
* 🪲 [v10.0.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.0.1)  Исправлена обработка форм слов: пользовательские формы теперь перекрывают автоматически сгенерированные; добавлены тестовые кейсы к тесту 22
* 🪲 [v9.8.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.8.2)  Исправление: обновлен deps.txt для включения исправлений упаковки в MCL, связанных с библиотекой embedding-ов
* 🪲 [v9.8.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.8.1)  Исправление: обновлен deps.txt с исправлениями упаковки для MCL и библиотеки embedding-ов
* 🪲 [v9.7.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.7.3) [ Issue #3306](https://github.com/manticoresoftware/manticoresearch/issues/3306) Исправлен краш с сигналом 11 во время индексации
* 🪲 [v9.7.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.7.2) [ Issue #3109](https://github.com/manticoresoftware/manticoresearch/issues/3109) Исправлена проблема, когда несуществующие `@@variables` всегда возвращали 0
* 🪲 [v9.7.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.7.1) [ Issue #3377](https://github.com/manticoresoftware/manticoresearch/issues/3377) Исправлен краш, связанный с [remove_repeats()](Functions/Searching_and_ranking_functions.md#REMOVE_REPEATS%28%29)
* 🪲 [v9.6.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.6.3) [ PR #3411](https://github.com/manticoresoftware/manticoresearch/pull/3411) Исправление: использование динамического обнаружения версий для телеметрических метрик
* 🪲 [v9.6.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.6.2)  Исправление: небольшое исправление вывода [SHOW VERSION](Node_info_and_management/SHOW_VERSION.md#SHOW-VERSION)
* 🪲 [v9.6.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.6.1)  Исправление: краш при создании таблицы с атрибутом KNN без модели
* 🪲 [v9.5.16](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.16) [ Issue #3342](https://github.com/manticoresoftware/manticoresearch/issues/3342) Исправлена проблема, при которой `SELECT ... FUZZY=0` не всегда отключал нечёткий поиск
* 🪲 [v9.5.15](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.15) [ PR #3397](https://github.com/manticoresoftware/manticoresearch/pull/3397) Добавлена поддержка [MCL](https://github.com/manticoresoftware/columnar) 4.2.2; исправлены ошибки с более старыми форматами хранения
* 🪲 [v9.5.14](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.14) [ Issue #3392](https://github.com/manticoresoftware/manticoresearch/issues/3392) Исправлена неправильная обработка строк в HTTP JSON ответах
* 🪲 [v9.5.13](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.13) [ Issue #3356](https://github.com/manticoresoftware/manticoresearch/issues/3356) Исправлен краш в сложном случае полнотекстового запроса (common-sub-term)
* 🪲 [v9.5.12](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.12)  Исправлена опечатка в сообщении об ошибке автоматической очистки дискового чанка
* 🪲 [v9.5.11](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.11) [ Issue #3195](https://github.com/manticoresoftware/manticoresearch/issues/3195) Улучшена [автоматическая очистка дискового чанка](Securing_and_compacting_a_table/Flushing_RAM_chunk_to_disk.md#Flushing-RAM-chunk-to-disk): пропуск сохранения если идет оптимизация
* 🪲 [v9.5.10](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.10) [ Issue #3313](https://github.com/manticoresoftware/manticoresearch/issues/3313) Исправлена проверка дублирующихся ID для всех дисковых чанков в RT таблице с использованием [indextool](Miscellaneous_tools.md#indextool)
* 🪲 [v9.5.9](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.9) [ Issue #3132](https://github.com/manticoresoftware/manticoresearch/issues/3132) Добавлена поддержка сортировки `_random` в JSON API
* 🪲 [v9.5.8](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.8) [ Issue #3382](https://github.com/manticoresoftware/manticoresearch/issues/3382) Исправлена проблема: нельзя было использовать uint64 ID документа через JSON HTTP API
* 🪲 [v9.5.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.7) [ Issue #3385](https://github.com/manticoresoftware/manticoresearch/issues/3385) Исправлены некорректные результаты при фильтрации по `id != value`
* 🪲 [v9.5.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.6) [ PR #538](https://github.com/manticoresoftware/manticoresearch-buddy/pull/538) Исправлена критическая ошибка при нечетком сопоставлении в некоторых случаях
* 🪲 [v9.5.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.5) [ Issue #3199](https://github.com/manticoresoftware/manticoresearch/issues/3199) Исправлено декодирование пробела в параметрах HTTP-запросов Buddy (например, `%20` и `+`)
* 🪲 [v9.5.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.4) [ Issue #3133](https://github.com/manticoresoftware/manticoresearch/issues/3133) Исправлена некорректная сортировка по `json.field` в фасетном поиске
* 🪲 [v9.5.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.3) [ Issue #3091](https://github.com/manticoresoftware/manticoresearch/issues/3091) Исправлено непоследовательное поведение поисковых результатов для разделителей в SQL и JSON API
* 🪲 [v9.5.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.2) [ Issue #2819](https://github.com/manticoresoftware/manticoresearch/issues/2819) Повышена производительность: заменено `DELETE FROM` на `TRUNCATE` для распределённых таблиц
* 🪲 [v9.5.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.1) [ Issue #3080](https://github.com/manticoresoftware/manticoresearch/issues/3080) Исправлен сбой при фильтрации с алиасами `geodist()` с JSON атрибутами

## Версия 9.3.2
Выпущено: 2 мая 2025

В этот релиз вошло множество исправлений багов и улучшений стабильности, улучшенный учёт использования таблиц, а также улучшения управления памятью и ресурсами.

❤️ Особая благодарность [@cho-m](https://github.com/cho-m) за исправление совместимости сборки с Boost 1.88.0 и [@benwills](https://github.com/benwills) за улучшение документации по `stored_only_fields`.

* 🪲 [v9.3.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.3.2)  Исправлена проблема, когда в колонке "Show Threads" показывалась активность CPU в виде числа с плавающей точкой вместо строки; также исправлена ошибка парсинга набора результатов PyMySQL из-за неправильного типа данных.
* 🪲 [v9.3.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.3.1) [ Issue #3343](https://github.com/manticoresoftware/manticoresearch/issues/3343) Исправлены оставшиеся файлы `tmp.spidx` после прерывания процесса оптимизации.
* 🆕 [v9.3.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.3.0) [ PR #3337](https://github.com/manticoresoftware/manticoresearch/pull/3337) Добавлен счетчик команд на таблицу и детальная статистика использования таблицы.
* 🪲 [v9.2.39](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.39) [ Issue #3236](https://github.com/manticoresoftware/manticoresearch/issues/3236) Исправление: предотвращена порча таблиц удалением сложных обновлений чанков. Использование функций ожидания внутри последовательного рабочего потока ломало последовательную обработку, что могло повредить таблицы.
	Переосмыслена автозапись. Удалена внешняя очередь опроса для избежания ненужных блокировок таблиц. Добавлено условие "малой таблицы": если количество документов ниже порога 'малой таблицы' (8192) и не используется Вторичный Индекс (SI), то запись пропускается.

* 🪲 [v9.2.38](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.38)  Исправление: пропуск создания Вторичного Индекса (SI) для фильтров с использованием `ALL`/`ANY` на списках строк без влияния на JSON атрибуты.
* 🪲 [v9.2.37](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.37) [ Issue #2898](https://github.com/manticoresoftware/manticoresearch/issues/2898) Добавлена поддержка обратных кавычек для системных таблиц.
* 🪲 [v9.2.36](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.36)  Исправление: использование заполнителя для операций с кластером в legacy-коде. В парсере теперь явно разделяются поля имени таблицы и кластера.
* 🪲 [v9.2.35](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.35)  Исправление: сбой при удалении кавычек у одиночной `'`.
* 🪲 [v9.2.34](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.34) [ Issue #3090](https://github.com/manticoresoftware/manticoresearch/issues/3090) Исправление: обработка больших ID документов (ранее была вероятность не найти их).
* 🪲 [v9.2.33](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.33)  Исправление: использование беззнаковых целых чисел для размеров битовых векторов.
* 🪲 [v9.2.32](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.32)  Исправление: сокращение пиковой нагрузки на память во время слияния. Поиск docid-to-rowid теперь занимает 12 байт на документ вместо 16 байт. Пример: 24 ГБ ОЗУ для 2 миллиардов документов вместо 36 ГБ.
* 🪲 [v9.2.31](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.31) [ Issue #3238](https://github.com/manticoresoftware/manticoresearch/issues/3238) Исправление: неправильное значение `COUNT(*)` в больших таблицах реального времени.
* 🪲 [v9.2.30](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.30)  Исправление: неопределенное поведение при обнулении строковых атрибутов.
* 🪲 [v9.2.29](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.29)  Небольшое исправление: улучшен текст предупреждения.
* 🪲 [v9.2.28](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.28) [ Issue #3290](https://github.com/manticoresoftware/manticoresearch/issues/3290) Улучшение: усовершенствован `indextool --buildidf`
* 🪲 [v9.2.27](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.27) [ Issue #3032](https://github.com/manticoresoftware/manticoresearch/issues/3032) С интеграцией Kafka теперь можно создавать источник для конкретного партиции Kafka.
* 🪲 [v9.2.26](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.26) [ Issue #3301](https://github.com/manticoresoftware/manticoresearch/issues/3301) Исправление: `ORDER BY` и `WHERE` по `id` могли вызывать ошибки OOM (Out Of Memory).
* 🪲 [v9.2.25](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.25) [ Issue #3171](https://github.com/manticoresoftware/manticoresearch/issues/3171) Исправление: сбой, вызванный ошибкой сегментации при использовании grouper с несколькими JSON-атрибутами в RT-таблице с несколькими дисковыми чанками.
* 🪲 [v9.2.24](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.24) [ Issue #3246](https://github.com/manticoresoftware/manticoresearch/issues/3246) Исправление: запросы `WHERE string ANY(...)` не выполнялись после сброса RAM-чанка.
* 🪲 [v9.2.23](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.23) [ PR #518](https://github.com/manticoresoftware/manticoresearch-buddy/pull/518) Небольшие улучшения синтаксиса автоматического шардирования.
* 🪲 [v9.2.22](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.22) [ Issue #2763](https://github.com/manticoresoftware/manticoresearch/issues/2763) Исправление: глобальный idf-файл не загружался при использовании `ALTER TABLE`.
* 🪲 [v9.2.21](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.21)  Исправление: глобальные idf-файлы могут быть большими. Теперь мы освобождаем таблицы раньше, чтобы избежать удержания ненужных ресурсов.
* 🪲 [v9.2.20](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.20) [ PR #3277](https://github.com/manticoresoftware/manticoresearch/pull/3277) Улучшение: лучшая валидация опций шардирования.

* 🪲 [v9.2.19](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.19) [ PR #3275](https://github.com/manticoresoftware/manticoresearch/pull/3275) Исправление: совместимость сборки с Boost 1.88.0.
* 🪲 [v9.2.18](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.18) [ Issue #3228](https://github.com/manticoresoftware/manticoresearch/issues/3228) Исправление: сбой при создании распределенной таблицы (проблема с некорректным указателем).

* 🪲 [v9.2.17](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.17) [ PR #3272](https://github.com/manticoresoftware/manticoresearch/pull/3272) Исправление: проблема с многострочным нечетким `FACET`.
* 🪲 [v9.2.16](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.16) [ Issue #3063](https://github.com/manticoresoftware/manticoresearch/issues/3063) Исправление: ошибка в расчёте расстояния при использовании функции `GEODIST`.
* 🪲 [v9.2.15](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.15) [ Issue #3027](https://github.com/manticoresoftware/manticoresearch/issues/3027) Небольшое улучшение: поддержка формата фильтра Elastic `query_string`.

## Version 9.2.14
Released: March 28th 2025

### Minor changes
* [Commit ](https://github.com/manticoresoftware/manticoresearch/commit/fe9473dc3f4fcfa0eaae748b538fe853f4ce078b) Реализован флаг `--mockstack` для расчёта требований к стеку рекурсивных операций. Новый режим `--mockstack` анализирует и сообщает необходимые размеры стека для рекурсивной оценки выражений, операций сопоставления с образцом, обработки фильтров. Рассчитанные требования к стеку выводятся в консоль для отладки и оптимизации.
* [Issue #3058](https://github.com/manticoresoftware/manticoresearch/pull/3058) Включена по умолчанию опция [boolean_simplify](Searching/Options.md#boolean_simplify).
* [Issue #3172](https://github.com/manticoresoftware/manticoresearch/issues/3172) Добавлена новая конфигурационная опция: `searchd.kibana_version_string`, которая может быть полезна при использовании Manticore с конкретными версиями Kibana или OpenSearch Dashboards, ожидающими определённую версию Elasticsearch.
* [Issue #3211](https://github.com/manticoresoftware/manticoresearch/issues/3211) Исправлена работа [CALL SUGGEST](Searching/Spell_correction.md#CALL-QSUGGEST,-CALL-SUGGEST) с двухсимвольными словами.
* [Issue #490](https://github.com/manticoresoftware/manticoresearch-buddy/issues/490) Улучшен [нечеткий поиск](Searching/Spell_correction.md#Fuzzy-Search): ранее он иногда не мог найти "defghi" при поиске "def ghi", если существовал другой совпадающий документ.
* ⚠️ BREAKING [Issue #3165](https://github.com/manticoresoftware/manticoresearch/issues/3165) В некоторых HTTP JSON ответах поле `_id` изменено на `id` для согласованности. Обязательно обновите ваше приложение соответственно.
* ⚠️ BREAKING [Issue #3186](https://github.com/manticoresoftware/manticoresearch/issues/3186) Добавлена проверка `server_id` при добавлении в кластер для обеспечения уникальности ID каждого узла. Операция `JOIN CLUSTER` теперь может завершаться ошибкой с сообщением о дублирующемся [server_id](Server_settings/Searchd.md#server_id), если присоединяемый узел имеет такой же `server_id`, как и существующий узел в кластере. Чтобы устранить эту проблему, убедитесь, что каждый узел в репликационном кластере имеет уникальный [server_id](Server_settings/Searchd.md#server_id). Вы можете изменить стандартный [server_id](Server_settings/Searchd.md#server_id) в разделе "searchd" вашего конфигурационного файла на уникальное значение перед попыткой присоединения к кластеру. Это изменение обновляет протокол репликации. Если вы управляете репликационным кластером, вам нужно:
  - Сначала корректно остановить все узлы
  - Затем запустить последний остановленный узел с `--new-cluster`, используя утилиту `manticore_new_cluster` в Linux.
  - Подробнее читайте о [перезапуске кластера](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster).

### Исправления багов
* [Commit 6fda](https://github.com/manticoresoftware/manticoresearch/commit/6fdad3923dd6953f4b781943eed5ec5c28b7f808) Исправлен сбой, вызванный потерей планировщика после ожидания; теперь специфичные планировщики, такие как `serializer`, корректно восстанавливаются.
* [Commit c333](https://github.com/manticoresoftware/manticoresearch/commit/c333b277d4a504de5fe298e9ab570a9f9ea31e0a) Исправлена ошибка, из-за которой веса из правой присоединённой таблицы не могли использоваться в выражении `ORDER BY`.
* [Issue #2644](https://github.com/manticoresoftware/manticoresearch/issues/2644) gcc 14.2.0: исправлена ошибка вызова `lower_bound` с `const knn::DocDist_t*&`. ❤️ Спасибо [@Azq2](https://github.com/Azq2) за PR.
* [Issue #3018](https://github.com/manticoresoftware/manticoresearch/issues/3018) Исправлена проблема с обработкой заглавных имён таблиц при авто-схеме вставок.
* [Issue #3119](https://github.com/manticoresoftware/manticoresearch/issues/3119) Исправлен сбой при декодировании некорректного base64 ввода.
* [Issue #3121](https://github.com/manticoresoftware/manticoresearch/issues/3121) Исправлены две связанные проблемы с KNN индексами на `ALTER`: теперь плавающие векторы сохраняют исходные размеры, и KNN индексы корректно создаются.
* [Issue #3123](https://github.com/manticoresoftware/manticoresearch/issues/3123) Исправлен сбой при построении вторичного индекса на пустом JSON столбце.
* [Issue #3138](https://github.com/manticoresoftware/manticoresearch/issues/3138) Исправлен сбой, вызванный дублирующимися записями.
* [Issue #3151](https://github.com/manticoresoftware/manticoresearch/issues/3151) Исправлено: опция `fuzzy=1` не могла использоваться с `ranker` или `field_weights`.
* [Issue #3163](https://github.com/manticoresoftware/manticoresearch/issues/3163) Исправлена ошибка, когда `SET GLOBAL timezone` не оказывал эффекта.
* [Issue #3181](https://github.com/manticoresoftware/manticoresearch/issues/3181) Исправлена проблема, при которой значения текстового поля могли теряться при использовании ID больше 2^63.
* [Issue #3189](https://github.com/manticoresoftware/manticoresearch/issues/3189) Исправлено: выражения `UPDATE` теперь корректно учитывают настройку `query_log_min_msec`.
* [Issue #3247](https://github.com/manticoresoftware/manticoresearch/issues/3247) Исправлена гонка при сохранении real-time дисковых чанков, из-за которой могло не удаваться `JOIN CLUSTER`.


## Версия 7.4.6
Релиз: 28 февраля 2025

### Основные изменения
* [Issue #832](https://github.com/manticoresoftware/manticoresearch/issues/832) Интеграция с [Kibana](Integration/Kibana.md) для удобства и улучшенной визуализации данных.

### Мелкие изменения
* [Issue #1727](https://github.com/manticoresoftware/manticoresearch/issues/1727) Исправлены различия точности с плавающей запятой между arm64 и x86_64.
* [Issue #2995](https://github.com/manticoresoftware/manticoresearch/issues/2995) Внедрены оптимизации производительности для пакетной обработки join.
* [Issue #3039](https://github.com/manticoresoftware/manticoresearch/issues/3039) Внедрены оптимизации производительности для EstimateValues в гистограммах.
* [Issue #3099](https://github.com/manticoresoftware/manticoresearch/issues/3099) Добавлена поддержка Boost 1.87.0. ❤️ Спасибо [@cho-m](https://github.com/cho-m) за PR.
* [Issue #77](https://github.com/manticoresoftware/columnar/issues/77) Оптимизирован повторный использование блоков данных при создании фильтров с множеством значений; добавлены min/max к метаданным атрибутов; реализована предварительная фильтрация значений фильтров на основе min/max.

### Исправления багов
* [Commit 73ac](https://github.com/manticoresoftware/manticoresearch/commit/73ac22f358a7a0e734b332c3943c86e6294c06c3) Исправлена обработка выражений в join-запросах, когда используются атрибуты из обеих левой и правой таблиц; исправлена опция index_weights для правой таблицы.
* [Issue #2915](https://github.com/manticoresoftware/manticoresearch/issues/2915) Использование `avg()` в `SELECT ... JOIN` могло приводить к некорректным результатам; теперь исправлено.
* [Issue #2996](https://github.com/manticoresoftware/manticoresearch/issues/2996) Исправлен некорректный набор результатов, вызванный неявным ограничением при включённом пакетировании join.
* [Issue #3031](https://github.com/manticoresoftware/manticoresearch/issues/3031) Исправлен сбой демона при завершении работы, когда шла активная слияние чанка.
* [Issue #3037](http://github.com/manticoresoftware/manticoresearch/issues/3037) Исправлена проблема, когда `IN(...)` мог возвращать некорректные результаты.
* [Issue #3038](https://github.com/manticoresoftware/manticoresearch/issues/3038) Установка `max_iops` / `max_iosize` в версии 7.0.0 могла ухудшать производительность индексации; теперь исправлено.
* [Issue #3042](https://github.com/manticoresoftware/manticoresearch/issues/3042) Исправлена утечка памяти в кэше join-запросов.
* [Issue #3052](https://github.com/manticoresoftware/manticoresearch/issues/3052) Исправлена обработка опций запросов в join JSON-запросах.
* [Issue #3054](https://github.com/manticoresoftware/manticoresearch/issues/3054) Исправлена проблема с командой ATTACH TABLE.
* [Issue #3079](https://github.com/manticoresoftware/manticoresearch/issues/3079) Исправлены несоответствия в сообщениях об ошибках.
* [Issue #3087](https://github.com/manticoresoftware/manticoresearch/issues/3087) Установка `diskchunk_flush_write_timeout=-1` для каждой таблицы не отключала сброс индекса; теперь это исправлено.
* [Issue #3088](https://github.com/manticoresoftware/manticoresearch/issues/3088) Исправлены дублирующиеся записи после массовой замены больших ID.
* [Issue #3126](https://github.com/manticoresoftware/manticoresearch/issues/3126) Исправлен сбой демона, вызванный полнотекстовым запросом с одиночным оператором `NOT` и ранжировщиком выражений.
* [Issue #3128](https://github.com/manticoresoftware/manticoresearch/pull/3128) Исправлена потенциальная уязвимость в библиотеке CJSON. ❤️ Спасибо, [@tabudz](https://github.com/tabudz) за PR.

## Version 7.0.0
Released: January 30th 2025

### Major changes
* [Issue #1497](https://github.com/manticoresoftware/manticoresearch/issues/1497) Добавлены новые возможности [Fuzzy Search](Searching/Spell_correction.md#Fuzzy-Search) и [Autocomplete](Searching/Autocomplete.md#CALL-AUTOCOMPLETE) для упрощения поиска.
* [Issue #1500](https://github.com/manticoresoftware/manticoresearch/issues/1500) [Интеграция с Kafka](Integration/Kafka.md#Syncing-from-Kafka).
* [Issue #1928](https://github.com/manticoresoftware/manticoresearch/issues/1928) Введены [вторичные индексы для JSON](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#json_secondary_indexes).
* [Issue #2361](https://github.com/manticoresoftware/manticoresearch/issues/2361) Обновления и поиски во время обновлений больше не блокируются слиянием чанков.
* [Issue #2787](https://github.com/manticoresoftware/manticoresearch/issues/2787) Автоматический [сброс дискового чанка](Server_settings/Searchd.md#diskchunk_flush_write_timeout) для RT таблиц для улучшения производительности; теперь мы автоматически сбрасываем RAM-чанк на дисковый чанк, предотвращая проблемы с производительностью из-за отсутствия оптимизаций в RAM чанках, что иногда могло приводить к нестабильности в зависимости от размера чанка.
* [Issue #2811](https://github.com/manticoresoftware/manticoresearch/issues/2811) Опция [Scroll](Searching/Pagination.md#Scroll-Search-Option) для упрощенной пагинации.
* [Issue #931](https://github.com/manticoresoftware/manticoresearch/issues/931) Интеграция с [Jieba](https://github.com/fxsjy/jieba) для улучшенной [токенизации китайского языка](Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md).

### Minor changes
* ⚠️ BREAKING [Issue #1111](https://github.com/manticoresoftware/manticoresearch/issues/1111) Исправлена поддержка `global_idf` в RT таблицах. Требуется пересоздание таблиц.
* ⚠️ BREAKING [Issue #2103](https://github.com/manticoresoftware/manticoresearch/issues/2103) Исключены тайские символы из внутреннего набора символов `cjk`. Обновите определения наборов символов: если у вас есть `cjk,non_cjk` и тайские символы важны, замените на `cjk,thai,non_cjk` или `cont,non_cjk`, где `cont` — новое обозначение для всех языков с непрерывным письмом (т.е., `cjk` + `thai`). Измените существующие таблицы с помощью [ALTER TABLE](Updating_table_schema_and_settings.md#Updating-table-FT-settings-in-RT-mode).
* ⚠️ BREAKING [Issue #2468](https://github.com/manticoresoftware/manticoresearch/issues/2468) [CALL SUGGEST / QSUGGEST](Searching/Spell_correction.md#CALL-QSUGGEST,-CALL-SUGGEST) теперь совместим с распределёнными таблицами. Это увеличивает версию протокола мастер/агент. Если вы используете Manticore Search в распределённой среде с несколькими инстансами, сначала обновите агенты, затем мастера.
* ⚠️ BREAKING [Issue #2889](https://github.com/manticoresoftware/manticoresearch/issues/2889) Изменено имя столбца с `Name` на `Variable name` для PQ [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META-for-PQ-tables).
* ⚠️ BREAKING [Issue #879](https://github.com/manticoresoftware/manticoresearch/issues/879) Введён [binlog на таблицу](Logging/Binary_logging.md#Per-table-binary-logging-configuration) с новыми опциями: [binlog_common](Logging/Binary_logging.md#Binary-logging-strategies), [binlog](Logging/Binary_logging.md#Per-table-binary-logging-configuration) для `create table` / `alter table`. Перед обновлением до новой версии необходимо корректно завершить работу инстанса Manticore.
* ⚠️ BREAKING [Issue #1789](https://github.com/manticoresoftware/manticoresearch/issues/1789) Исправлено неправильное сообщение об ошибке при подключении узла к кластеру с неверной версией протокола репликации. Внесено обновление в протокол репликации. Если вы используете кластер с репликацией, нужно:
  - Сначала корректно остановить все узлы
  - Затем запустить последний остановленный узел с `--new-cluster` с помощью инструментария `manticore_new_cluster` в Linux.
  - Подробнее о [перезапуске кластера](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster).
* ⚠️ BREAKING [Issue #2308](https://github.com/manticoresoftware/manticoresearch/issues/2308) Добавлена поддержка нескольких таблиц в [`ALTER CLUSTER ADD` и `DROP`](Creating_a_cluster/Setting_up_replication/Adding_and_removing_a_table_from_a_replication_cluster.md#Adding-and-removing-a-table-from-a-replication-cluster). Это изменение также затрагивает протокол репликации. См. предыдущее описание для рекомендаций по обновлению.
* [Issue #2997](https://github.com/manticoresoftware/manticoresearch/issues/2997) Исправлена проблема dlopen на Macos.
* [Commit 4954](https://github.com/manticoresoftware/manticoresearch/commit/4954b5de7341a29902a9b8fbb9a040f7942c77c4) Изменён стандартный порог для OPTIMIZE TABLE на таблицах с индексами KNN для улучшения производительности поиска.
* [Commit cfc8](https://github.com/manticoresoftware/manticoresearch/commit/cfc87ecb6e33a8163c2235243b6b40e699dbf526) Добавлена поддержка `COUNT(DISTINCT)` для `ORDER BY` в `FACET` и `GROUP BY`.
* [Issue #1103](https://github.com/manticoresoftware/manticoresearch/issues/1103) Улучшена ясность в [логировании](Logging/Server_logging.md#Server-logging) слияния чанков.
* [Issue #1130](https://github.com/manticoresoftware/manticoresearch/issues/1130) Добавлена поддержка для [DBeaver](Integration/DBeaver.md).
* [Issue #1546](https://github.com/manticoresoftware/manticoresearch/issues/1546) Реализованы вторичные индексы для функций [POLY2D()](Functions/Geo_spatial_functions.md#POLY2D%28%29)/[GEOPOLY2D()](Functions/Geo_spatial_functions.md#GEOPOLY2D%28%29).
* [Issue #1630](https://github.com/manticoresoftware/manticoresearch/issues/1630) HTTP-запросы теперь поддерживают `Content-Encoding: gzip`.
* [Issue #1831](https://github.com/manticoresoftware/manticoresearch/issues/1831) Добавлена команда `SHOW LOCKS`.
* [Issue #2187](https://github.com/manticoresoftware/manticoresearch/issues/2197) Разрешено обходить ограничение [searchd.max_connections](Server_settings/Searchd.md#max_connections) для запросов Buddy к демону.
* [Issue #2208](https://github.com/manticoresoftware/manticoresearch/issues/2208) Добавлена поддержка объединения таблиц через интерфейс JSON HTTP.
* [Issue #2235](https://github.com/manticoresoftware/manticoresearch/issues/2235) Логирование успешно обработанных запросов через Buddy в их оригинальном виде.
* [Issue #2249](https://github.com/manticoresoftware/manticoresearch/issues/2249) Добавлен специальный режим запуска `mysqldump` для реплицируемых таблиц.
* [Issue #2268](https://github.com/manticoresoftware/manticoresearch/issues/2268) Улучшено переименование внешних файлов при копировании для операторов `CREATE TABLE` и `ALTER TABLE`.
* [Issue #2402](https://github.com/manticoresoftware/manticoresearch/issues/2402) Обновлено значение по умолчанию для [searchd.max_packet_size](Server_settings/Searchd.md#max_packet_size) до 128MB.
* [Issue #2419](https://github.com/manticoresoftware/manticoresearch/issues/2419) Добавлена поддержка [IDF boost modifier](Searching/Full_text_matching/Operators.md#IDF-boost-modifier) в JSON ["match"](Searching/Full_text_matching/Basic_usage.md#match).
* [Issue #2430](https://github.com/manticoresoftware/manticoresearch/issues/2430) Усилена синхронизация при записи [binlog](Logging/Binary_logging.md#Binary-logging) для предотвращения ошибок.
* [Issue #2458](https://github.com/manticoresoftware/manticoresearch/issues/2458) Включена поддержка zlib в пакетах для Windows.
* [Issue #2479](https://github.com/manticoresoftware/manticoresearch/issues/2479) Добавлена поддержка команды SHOW TABLE INDEXES.
* [Issue #2485](https://github.com/manticoresoftware/manticoresearch/issues/2485) Установлены метаданные сессии для ответов Buddy.
* [Issue #2490](https://github.com/manticoresoftware/manticoresearch/issues/2490) Разрешение миллисекунд для агрегатов на точках совместимости.
* [Issue #2500](https://github.com/manticoresoftware/manticoresearch/issues/2500) Изменены сообщения об ошибках для операций с кластером при сбое запуска репликации.
* [Issue #2584](https://github.com/manticoresoftware/manticoresearch/pull/2584) Новые [метрики производительности в SHOW STATUS](Node_info_and_management/Node_status.md#Query-Time-Statistics): мин/макс/среднее/95-й/99-й процентиль по типу запроса за последние 1, 5 и 15 минут.
* [Issue #2639](https://github.com/manticoresoftware/manticoresearch/pull/2639) Все случаи `index` заменены на `table` в запросах и ответах.
* [Issue #2643](https://github.com/manticoresoftware/manticoresearch/issues/2643) Добавлен столбец `distinct` в результаты агрегаций HTTP `/sql` эндпоинта.
* [Issue #268](https://github.com/manticoresoftware/manticoresearch-buddy/issues/268) Реализовано автоматическое определение типов данных, импортированных из Elasticsearch.
* [Issue #2744](https://github.com/manticoresoftware/manticoresearch/issues/2744) Добавлена поддержка сопоставления (collation) для выражений сравнения строковых JSON-полей.
* [Issue #2752](https://github.com/manticoresoftware/manticoresearch/issues/2752) Добавлена поддержка выражения `uuid_short` в списке select.
* [Issue #2783](https://github.com/manticoresoftware/manticoresearch/issues/2783) Manticore Search теперь запускает Buddy напрямую без обертки `manticore-buddy`.
* [Issue #2785](https://github.com/manticoresoftware/manticoresearch/issues/2785) Разграничены сообщения об ошибках для отсутствующих таблиц и таблиц, не поддерживающих операции вставки.
* [Issue #2789](https://github.com/manticoresoftware/manticoresearch/issues/2789) OpenSSL 3 теперь статически встроен в `searchd`.
* [Issue #2790](https://github.com/manticoresoftware/manticoresearch/issues/2790) Добавлена инструкция `CALL uuid_short` для генерации последовательностей с несколькими значениями `uuid_short`.
* [Issue #2803](https://github.com/manticoresoftware/manticoresearch/issues/2803) Добавлены отдельные опции для правой таблицы в операции JOIN.
* [Issue #2810](https://github.com/manticoresoftware/manticoresearch/issues/2810) Повышена производительность агрегаций HTTP JSON до уровня `GROUP BY` в SphinxQL.
* [Issue #2854](https://github.com/manticoresoftware/manticoresearch/issues/2854) Добавлена поддержка `fixed_interval` в запросах, связанных с датами в Kibana.
* [Issue #2909](https://github.com/manticoresoftware/manticoresearch/issues/2909) Реализована пакетная обработка для JOIN-запросов, что улучшает производительность некоторых JOIN-запросов в сотни и даже тысячи раз.
* [Issue #2937](https://github.com/manticoresoftware/manticoresearch/issues/2937) Включена возможность использовать вес соединённой таблицы в fullscan-запросах.
* [Issue #2953](https://github.com/manticoresoftware/manticoresearch/issues/2953) Исправлена логировка для join-запросов.
* [Issue #337](https://github.com/manticoresoftware/manticoresearch-buddy/issues/337) Исключения Buddy скрыты из лога `searchd` в режиме без отладки.
* [Issue #2931](https://github.com/manticoresoftware/manticoresearch/issues/2931) Завершение работы демона с сообщением об ошибке, если пользователь указал неверные порты для слушателя репликации.

### Исправления ошибок
* [Commit 0c6b](https://github.com/manticoresoftware/manticoresearch/commit/0c6bdaf41ea3684e05952e1a8273893535b923f5) Исправлено: Некорректные результаты в JOIN-запросах с более чем 32 колонками.
* [Issue #2335](https://github.com/manticoresoftware/manticoresearch/issues/2335) Исправлена ошибка, когда соединение таблиц не выполнялось при использовании двух json-атрибутов в условии.
* [Issue #2338](https://github.com/manticoresoftware/manticoresearch/issues/2338) Исправлено неверное значение total_relation в мультизапросах с использованием [cutoff](Searching/Options.md#cutoff).
* [Issue #2366](https://github.com/manticoresoftware/manticoresearch/issues/2366) Исправлена фильтрация по `json.string` в правой таблице при [объединении таблиц](Searching/Joining.md).
* [Issue #2406](https://github.com/manticoresoftware/manticoresearch/issues/2406) Включена возможность использования `null` для всех значений в любых POST HTTP JSON эндпоинтах (insert/replace/bulk). В этом случае используется значение по умолчанию.
* [Issue #2418](https://github.com/manticoresoftware/manticoresearch/issues/2418) Оптимизировано потребление памяти путем настройки выделения [буфера сети max_packet_size](Server_settings/Searchd.md#max_packet_size) для начального пробного соединения сокета.
* [Issue #2420](https://github.com/manticoresoftware/manticoresearch/issues/2420) Исправлена вставка беззнакового int в атрибут bigint через JSON интерфейс.
* [Issue #2422](https://github.com/manticoresoftware/manticoresearch/issues/2422) Исправлена работа вторичных индексов с фильтрами исключения и включенным pseudo_sharding.
* [Issue #2423](https://github.com/manticoresoftware/manticoresearch/issues/2423) Исправлена ошибка в [manticore_new_cluster](Starting_the_server/Manually.md#searchd-command-line-options).
* [Issue #2448](https://github.com/manticoresoftware/manticoresearch/issues/2448) Исправлен сбой демона при обработке некорректного запроса `_update`.
* [Issue #2452](https://github.com/manticoresoftware/manticoresearch/issues/2452) Исправлена неспособность гистограмм обрабатывать фильтры по значениям с исключениями.
* [Issue #55](https://github.com/manticoresoftware/columnar/issues/55) Исправлены запросы [knn](Searching/KNN.md#KNN-vector-search) к распределённым таблицам.
* [Issue #68](https://github.com/manticoresoftware/columnar/issues/68) Улучшена обработка фильтров исключения при кодировании таблиц в columnar accessor.
* [Commit 0eb1](https://github.com/manticoresoftware/manticoresearch/commit/0eb1579462013cb134bcaf74e06f6eeb0c2ecc4d) Исправлен парсер выражений, теперь учитывает переопределённый `thread_stack`.
* [Commit c304](https://github.com/manticoresoftware/manticoresearch/commit/c3041fd1d3e14773f3c5aa635343915a71f5cb95) Исправлен сбой при клонировании columnar IN выражения.
* [Commit edad](https://github.com/manticoresoftware/columnar/commit/edadc694c68d6022bdd13134263667430a42cc1d) Исправлена ошибка инверсии в итераторе битовой карты, приводившая к сбою.
* [Commit fc30](https://github.com/manticoresoftware/executor/commit/fc302ff1117b0b835a6f5a3c07057baf5fba14d9) Исправлена проблема, из-за которой некоторые пакеты Manticore автоматически удалялись `unattended-upgrades`.
* [Issue #1019](https://github.com/manticoresoftware/manticoresearch/issues/1019) Улучшена обработка запросов из инструмента DbForge MySQL.
* [Issue #1107](https://github.com/manticoresoftware/manticoresearch/issues/1107) Исправлено экранирование специальных символов в `CREATE TABLE` и `ALTER TABLE`.  ❤️ Спасибо, [@subnix](https://github.com/subnix) за PR.
* [Issue #116](https://github.com/manticoresoftware/manticoresearch-backup/issues/116#issuecomment-2216597206) Исправлена взаимная блокировка при обновлении blob-атрибута в замороженном индексе. Взаимная блокировка происходила из-за конфликтующих блокировок при попытке разморозить индекс. Это могло вызвать сбой в manticore-backup.
* [Issue #1818](https://github.com/manticoresoftware/manticoresearch/issues/1818) `OPTIMIZE` теперь выдаёт ошибку при попытке использовать на замороженной таблице.
* [Issue #2001](https://github.com/manticoresoftware/manticoresearch/issues/2001) Разрешено использовать имена функций как имена столбцов.
* [Issue #2153](https://github.com/manticoresoftware/manticoresearch/issues/2153) Исправлен сбой демона при запросе настроек таблицы с неизвестным дисковым чанком.
* [Issue #2184](https://github.com/manticoresoftware/manticoresearch/issues/2184) Исправлена проблема, из-за которой `searchd` зависал при остановке после `FREEZE` и `FLUSH RAMCHUNK`.
* [Issue #2228](https://github.com/manticoresoftware/manticoresearch/issues/2228) Удалены токены, связанные с датой/временем (и regex) из списка зарезервированных слов.
* [Issue #2255](https://github.com/manticoresoftware/manticoresearch/issues/2255) Исправлен сбой на `FACET` с более чем 5 полями сортировки.
* [Issue #2265](https://github.com/manticoresoftware/manticoresearch/issues/2265) Исправлен сбой при восстановлении `mysqldump` с включённым `index_field_lengths`.
* [Issue #2291](https://github.com/manticoresoftware/manticoresearch/issues/2291) Исправлен сбой при выполнении команды `ALTER TABLE`.
* [Issue #2333](https://github.com/manticoresoftware/manticoresearch/issues/2333) Исправлена MySQL DLL в пакете для Windows, чтобы индексатор работал корректно.
* [Issue #2393](https://github.com/manticoresoftware/manticoresearch/issues/2393) Исправлена ошибка компиляции GCC. ❤️ Спасибо, [@animetosho](https://github.com/animetosho) за PR.
* [Issue #2447](https://github.com/manticoresoftware/manticoresearch/issues/2447) Исправлена ошибка экранирования в [_update](Data_creation_and_modification/Updating_documents/REPLACE.md?client=Elasticsearch-like+partial).
* [Issue #2460](https://github.com/manticoresoftware/manticoresearch/issues/2460) Исправлен сбой индексатора при объявлении нескольких атрибутов или полей с одним именем.
* [Issue #2467](https://github.com/manticoresoftware/manticoresearch/issues/2467) Исправлен сбой демона при неправильной трансформации для вложенных булевых запросов для поисковых эндпоинтов "compat".
* [Issue #2493](https://github.com/manticoresoftware/manticoresearch/issues/2493) Исправлено расширение в фразах с модификаторами.
* [Issue #2535](https://github.com/manticoresoftware/manticoresearch/issues/2535) Исправлен сбой демона при использовании полнотекстового оператора [ZONE](Searching/Full_text_matching/Operators.md#ZONE-limit-operator) или [ZONESPAN](Searching/Full_text_matching/Operators.md#ZONESPAN-limit-operator).
* [Issue #2552](https://github.com/manticoresoftware/manticoresearch/issues/2552) Исправлена генерация инфиксов для plain и RT таблиц с словарём ключевых слов.
* [Issue #2571](https://github.com/manticoresoftware/manticoresearch/issues/2571) Исправлен ответ с ошибкой в запросе `FACET`; установлен порядок сортировки по умолчанию `DESC` для `FACET` с `COUNT(*)`.
* [Issue #2580](https://github.com/manticoresoftware/manticoresearch/issues/2580) Исправлен сбой демона на Windows при запуске.
* [Issue #2603](https://github.com/manticoresoftware/manticoresearch/issues/2603) Исправлено усечение запроса для HTTP эндпоинтов `/sql` и `/sql?mode=raw`; сделаны запросы с этих эндпоинтов консистентными без необходимости заголовка `query=`.
* [Issue #2623](https://github.com/manticoresoftware/manticoresearch/issues/2623) Исправлена проблема, при которой auto-schema создает таблицу, но одновременно падает.
* [Issue #2627](https://github.com/manticoresoftware/manticoresearch/issues/2627) Исправлена библиотека HNSW для поддержки загрузки нескольких индексов KNN.
* [Issue #2630](https://github.com/manticoresoftware/manticoresearch/issues/2630) Исправлена проблема зависания при одновременном возникновении нескольких условий.
* [Issue #2645](https://github.com/manticoresoftware/manticoresearch/issues/2645) Исправлен сбой с критической ошибкой при использовании `OR` с поиском KNN.
* [Issue #2647](https://github.com/manticoresoftware/manticoresearch/issues/2647) Исправлена проблема, когда `indextool --mergeidf *.idf --out global.idf` удалял выходной файл после создания.
* [Issue #2658](https://github.com/manticoresoftware/manticoresearch/issues/2658) Исправлен сбой демона при подзапросе с `ORDER BY` строкой во внешнем запросе.
* [Issue #2686](https://github.com/manticoresoftware/manticoresearch/issues/2686) Исправлен сбой при обновлении float-атрибута вместе с текстовым атрибутом.
* [Issue #2704](https://github.com/manticoresoftware/manticoresearch/issues/2704) Исправлена проблема, при которой несколько стоп-слов от токенизаторов `lemmatize_xxx_all` увеличивали `hitpos` у последующих токенов.
* [Issue #2708](https://github.com/manticoresoftware/manticoresearch/issues/2708) Исправлен сбой при `ALTER ... ADD COLUMN ... TEXT`.
* [Issue #2737](https://github.com/manticoresoftware/manticoresearch/issues/2737) Исправлена проблема, при которой обновление blob-атрибута в замороженной таблице с как минимум одним RAM-чанком заставляло последующие запросы `SELECT` ожидать разморозки таблицы.
* [Issue #2742](https://github.com/manticoresoftware/manticoresearch/issues/2742) Исправлен пропуск кэша запросов для запросов с упакованными факторами.
* [Issue #2775](https://github.com/manticoresoftware/manticoresearch/issues/2775) Manticore теперь выдает ошибку при неизвестном действии вместо сбоя на запросах `_bulk`.
* [Issue #2791](https://github.com/manticoresoftware/manticoresearch/issues/2791) Исправлен возврат ID вставленного документа для HTTP эндпоинта `_bulk`.
* [Issue #2797](https://github.com/manticoresoftware/manticoresearch/issues/2797) Исправлен сбой в grouper при работе с несколькими таблицами, одна из которых пустая, а другая содержит разное количество совпадений.
* [Issue #2835](https://github.com/manticoresoftware/manticoresearch/issues/2835) Исправлен сбой в сложных `SELECT` запросах.
* [Issue #2872](https://github.com/manticoresoftware/manticoresearch/issues/2872) Добавлено сообщение об ошибке, если аргумент `ALL` или `ANY` в выражении `IN` не является JSON-атрибутом.
* [Issue #2882](https://github.com/manticoresoftware/manticoresearch/issues/2882) Исправлен сбой демона при обновлении MVA в больших таблицах.
* [Issue #2888](https://github.com/manticoresoftware/manticoresearch/issues/2888) Исправлен сбой при ошибке токенизации с `libstemmer`.  ❤️ Спасибо, [@subnix](https://github.com/subnix), за PR.
* [Issue #2919](https://github.com/manticoresoftware/manticoresearch/issues/2919) Исправлена проблема, при которой объединённый вес из правой таблицы некорректно работал в выражениях.
* [Issue #2919](https://github.com/manticoresoftware/manticoresearch/issues/2919) Исправлена проблема, когда вес правой присоединённой таблицы не работает в выражениях.
* [Issue #325](https://github.com/manticoresoftware/manticoresearch-buddy/issues/325) Исправлен сбой `CREATE TABLE IF NOT EXISTS ... WITH DATA`, если таблица уже существует.
* [Issue #351](https://github.com/manticoresoftware/manticoresearch-buddy/issues/351) Исправлена ошибка undefined array key "id" при подсчёте по KNN с ID документа.
* [Issue #359](https://github.com/manticoresoftware/manticoresearch-buddy/issues/359) Исправлена функциональность `REPLACE INTO cluster_name:table_name`.
* [Issue #67](https://github.com/manticoresoftware/docker/issues/67) Исправлена критическая ошибка при запуске контейнера Manticore Docker с `--network=host`.

## Version 6.3.8
Выпуск: 22 ноября 2024

Версия 6.3.8 продолжает серию 6.3 и включает только исправления ошибок.

### Исправления ошибок

* [PR #2777](https://github.com/manticoresoftware/manticoresearch/pull/2777) Исправлен расчет доступных потоков при ограничении параллелизма запросов настройками `threads` или `max_threads_per_query`.

## manticore-extra v1.1.20

Выпуск: 7 октября 2024

### Исправления ошибок

* [Issue #64](https://github.com/manticoresoftware/executor/issues/64) Решена проблема, когда утилита `unattended-upgrades`, автоматически устанавливающая обновления на Debian-системах, неправильно помечала несколько пакетов Manticore, включая `manticore-galera`, `manticore-executor` и `manticore-columnar-lib`, как подлежащие удалению. Это происходило из-за того, что `dpkg` ошибочно считал виртуальный пакет `manticore-extra` избыточным. Внесены изменения, чтобы `unattended-upgrades` больше не пыталась удалять важные компоненты Manticore.

## Version 6.3.6
Выпуск: 2 августа 2024

Версия 6.3.6 продолжает серию 6.3 и включает только исправления ошибок.

### Исправления ошибок

* [Issue #2477](https://github.com/manticoresoftware/manticoresearch/issues/2477) Исправлен сбой, возникший в версии 6.3.4, который мог происходить при работе с выражениями и распределёнными или множественными таблицами.
* [Issue #2352](https://github.com/manticoresoftware/manticoresearch/issues/2352) Исправлен сбой демона или внутренняя ошибка при раннем выходе, вызванная `max_query_time` при запросе нескольких индексов.

## Version 6.3.4
Выпуск: 31 июля 2024

Версия 6.3.4 продолжает серию 6.3 и включает только незначительные улучшения и исправления ошибок.

### Незначительные изменения
* [Issue #1130](https://github.com/manticoresoftware/manticoresearch/issues/1130) Добавлена поддержка [DBeaver](Integration/DBeaver.md#Integration-with-DBeaver).
* [Issue #2146](https://github.com/manticoresoftware/manticoresearch/issues/2146) Улучшено экранирование разделителей в формах слов и исключениях.
* [Issue #2315](https://github.com/manticoresoftware/manticoresearch/issues/2315) Добавлены операторы сравнения строк в выражения списка SELECT.
* [Issue #2363](https://github.com/manticoresoftware/manticoresearch/issues/2363) Добавлена поддержка null значений в пакетных запросах в стиле Elastic.
* [Issue #2374](https://github.com/manticoresoftware/manticoresearch/issues/2374) Добавлена поддержка mysqldump версии 9.
* [Issue #2375](https://github.com/manticoresoftware/manticoresearch/issues/2375) Улучшена обработка ошибок в HTTP JSON запросах с JSON-путём к узлу, где возникает ошибка.

### Исправления ошибок
* [Issue #2280](https://github.com/manticoresoftware/manticoresearch/issues/2280) Исправлено ухудшение производительности при запросах с подстановочными знаками с множеством совпадений, когда disk_chunks > 1.
* [Issue #2332](https://github.com/manticoresoftware/manticoresearch/issues/2332) Исправлен сбой в выражениях списка SELECT с MVA MIN или MAX для пустых массивов MVA.
* [Issue #2339](https://github.com/manticoresoftware/manticoresearch/issues/2339) Исправлена некорректная обработка бесконечного диапазонного запроса Kibana.
* [Issue #2342](https://github.com/manticoresoftware/manticoresearch/issues/2342) Исправлен фильтр соединения по колоночным атрибутам из правой таблицы, когда атрибут отсутствует в списке SELECT.
* [Issue #2343](https://github.com/manticoresoftware/manticoresearch/issues/2343) Исправлено дублирование спецификатора 'static' в версии Manticore 6.3.2.
* [Issue #2344](https://github.com/manticoresoftware/manticoresearch/issues/2344) Исправлен LEFT JOIN, возвращающий несовпадающие записи при использовании MATCH() по правой таблице.
* [Issue #2350](https://github.com/manticoresoftware/manticoresearch/issues/2350) Исправлено сохранение disk chunk в RT индексе с `hitless_words`.
* [Issue #2364](https://github.com/manticoresoftware/manticoresearch/issues/2364) Свойство 'aggs_node_sort' теперь можно добавлять в любом порядке среди других свойств.
* [Issue #2368](https://github.com/manticoresoftware/manticoresearch/issues/2368) Исправлена ошибка порядка full-text и фильтров в JSON-запросе.
* [Issue #2376](https://github.com/manticoresoftware/manticoresearch/issues/2376) Исправлена ошибка, связанная с некорректным JSON-ответом на длинные UTF-8 запросы.
* [Issue #2684](https://github.com/manticoresoftware/dev/issues/2684) Исправлен расчёт выражений presort/prefilter, зависящих от присоединённых атрибутов.
* [Issue #301](https://github.com/manticoresoftware/manticoresearch-buddy/issues/301) Изменён способ вычисления размера данных для метрик — теперь считывается из файла `manticore.json`, вместо проверки всего размера каталога данных.
* [Issue #302](https://github.com/manticoresoftware/manticoresearch-buddy/issues/302) Добавлена обработка запросов валидации от Vector.dev.

## Версия 6.3.2
Выпущена: 26 июня 2024

Версия 6.3.2 продолжает серию 6.3 и включает исправления нескольких ошибок, некоторые из которых были обнаружены после выпуска 6.3.0.

### Ломающие изменения
* ⚠️[Issue #2305](https://github.com/manticoresoftware/manticoresearch/issues/2305) Значения aggs.range обновлены до числового типа.

### Исправления ошибок
* [Commit c51c](https://github.com/manticoresoftware/manticoresearch/commit/c51c7fa4bd0ec4ad8d53188b888461bff29f1417) Исправлена группировка по сохранённой проверке против слияния rset.
* [Commit 0e85](https://github.com/manticoresoftware/manticoresearch/commit/0e85b9d5682f51681750d798aef6a11cfb93d69e) Исправлен сбой демона при запросах с подстановочными знаками в RT индексе с использованием CRC словаря и включённым `local_df`.
* [Issue #2200](https://github.com/manticoresoftware/manticoresearch/issues/2200) Исправлен сбой в JOIN при использовании `count(*)` без GROUP BY.
* [Issue #2201](https://github.com/manticoresoftware/manticoresearch/issues/2201) Исправлено отсутствие предупреждения при попытке группировки по полнотекстовому полю в JOIN.
* [Issue #2230](https://github.com/manticoresoftware/manticoresearch/issues/2230) Исправлена проблема, когда добавление атрибута через `ALTER TABLE` не учитывало опции KNN.
* [Issue #2231](https://github.com/manticoresoftware/manticoresearch/issues/2231) Исправлен сбой при удалении пакета `manticore-tools` Redhat в версии 6.3.0.
* [Issue #2242](https://github.com/manticoresoftware/manticoresearch/issues/2242) Исправлены проблемы с JOIN и несколькими операторами FACET, возвращающими некорректные результаты.
* [Issue #2250](https://github.com/manticoresoftware/manticoresearch/issues/2250) Исправлена ошибка ALTER TABLE, вызывавшая ошибку, если таблица находится в кластере.
* [Issue #2252](https://github.com/manticoresoftware/manticoresearch/issues/2252) Исправлена передача исходного запроса в buddy из интерфейса SphinxQL.
* [Issue #2267](https://github.com/manticoresoftware/manticoresearch/issues/2267) Улучшено расширение подстановочных знаков в `CALL KEYWORDS` для RT индекса с дисковыми чанками.
* [Issue #271](https://github.com/manticoresoftware/manticoresearch-buddy/issues/271) Исправлено зависание при некорректных запросах `/cli`.
* [Issue #274](https://github.com/manticoresoftware/manticoresearch-buddy/issues/274) Решены проблемы с зависанием при одновременных запросах к Manticore.
* [Issue #275](https://github.com/manticoresoftware/manticoresearch-buddy/issues/275) Исправлено зависание при выполнении `drop table if exists t; create table t` через `/cli`.

### Связанное с репликацией
* [Issue #2270](https://github.com/manticoresoftware/manticoresearch/issues/2270) Добавлена поддержка формата `cluster:name` в HTTP-эндоинте `/_bulk`.

## Версия 6.3.0
Выпущена: 23 мая 2024

### Основные изменения
* [Issue #839](https://github.com/manticoresoftware/manticoresearch/issues/839) Реализован тип данных [float_vector](Creating_a_table/Data_types.md#Float-vector); реализован [поиск по векторам](Searching/KNN.md#KNN-vector-search).
* [Issue #1673](https://github.com/manticoresoftware/manticoresearch/issues/1673) Добавлен [INNER/LEFT JOIN](Searching/Joining.md) (**бета-версия**).
* [Issue #1744](https://github.com/manticoresoftware/manticoresearch/issues/1744) Реализовано автоматическое определение форматов даты для полей [timestamp](Creating_a_table/Data_types.md#Timestamps).
* [Issue #1720](https://github.com/manticoresoftware/manticoresearch/pull/1720) Лицензия Manticore Search изменена с GPLv2-or-later на GPLv3-or-later.
* [Commit 7a55](https://github.com/manticoresoftware/manticoresearch/commit/7a5508424) Запуск Manticore в Windows теперь требует Docker для запуска Buddy.
* [Issue #1541](https://github.com/manticoresoftware/manticoresearch/issues/1541) Добавлен оператор полнотекстового поиска [REGEX](Searching/Full_text_matching/Operators.md#REGEX-operator).
* [Issue #2091](https://github.com/manticoresoftware/manticoresearch/issues/2091) Поддержка Ubuntu Noble 24.04.
* [Commit 514d](https://github.com/manticoresoftware/manticoresearch/commit/514d35b497f4bdb20b6473ff963752a978a4bb8d) Переработка операций с временем для улучшения производительности и новых функций даты/времени:
  - [CURDATE()](Functions/Date_and_time_functions.md#CURDATE%28%29) - Возвращает текущую дату в локальном часовом поясе
  - [QUARTER()](Functions/Date_and_time_functions.md#QUARTER%28%29) - Возвращает целочисленный квартал года из аргумента timestamp
  - [DAYNAME()](Functions/Date_and_time_functions.md#DAYNAME%28%29) - Возвращает название дня недели для заданного аргумента timestamp
  - [MONTHNAME()](Functions/Date_and_time_functions.md#MONTHNAME%28%29) - Возвращает название месяца для заданного аргумента timestamp
  - [DAYOFWEEK()](Functions/Date_and_time_functions.md#DAYOFWEEK%28%29) - Возвращает целочисленный индекс дня недели для заданного аргумента timestamp
  - [DAYOFYEAR()](Functions/Date_and_time_functions.md#DAYOFYEAR%28%29) - Возвращает целочисленный номер дня в году для заданного аргумента timestamp
  - [YEARWEEK()](Functions/Date_and_time_functions.md#YEARWEEK%28%29) - Возвращает целочисленный год и код дня первого дня текущей недели для заданного аргумента timestamp
  - [DATEDIFF()](Functions/Date_and_time_functions.md#DATEDIFF%28%29) - Возвращает количество дней между двумя заданными timestamp
  - [DATE()](Functions/Date_and_time_functions.md#DATE%28%29) - Форматирует часть даты из аргумента timestamp
  - [TIME()](Functions/Date_and_time_functions.md#TIME%28%29) - Форматирует часть времени из аргумента timestamp
  - [timezone](Server_settings/Searchd.md#timezone) - Часовой пояс, используемый функциями, связанными с датой/временем.
* [Commit 30e7](https://github.com/manticoresoftware/manticoresearch/commit/30e789d9657f9b3093c280c6e5b0e46a021f6dc7) Добавлены агрегаты [range](Searching/Faceted_search.md#Facet-over-set-of-ranges), [histogram](Searching/Faceted_search.md#Facet-over-histogram-values), [date_range](Searching/Faceted_search.md#Facet-over-set-of-date-ranges) и [date_histogram](Searching/Faceted_search.md#Facet-over-histogram-date-values) в HTTP-интерфейс и аналогичные выражения в SQL.

### Minor changes
* [Issue #1285](https://github.com/manticoresoftware/manticoresearch/issues/1285) Поддержка версий Filebeat 8.10 - 8.11.
* [Issue #1771](https://github.com/manticoresoftware/manticoresearch/issues/1771) [ALTER TABLE ... type='distributed'](Updating_table_schema_and_settings.md#Changing-a-distributed-table).
* [Issue #1788](https://github.com/manticoresoftware/manticoresearch/issues/1788) Добавлена возможность копирования таблиц с помощью оператора SQL [CREATE TABLE ... LIKE ... WITH DATA](Creating_a_table/Local_tables/Real-time_table.md#CREATE-TABLE-LIKE:).
* [Issue #2072](https://github.com/manticoresoftware/manticoresearch/issues/2072) Оптимизирован [алгоритм компактификации таблиц](Securing_and_compacting_a_table/Compacting_a_table.md#Compacting-a-Table): Ранее как ручной процесс [OPTIMIZE](Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE), так и автоматический [auto_optimize](Server_settings/Searchd.md#auto_optimize) сначала сливались чанки, чтобы количество не превышало лимит, затем удалялись удалённые документы из всех других чанков, содержащих удалённые документы. Этот подход иногда был слишком ресурсозатратным и теперь отключён. Сейчас слияние чанков происходит только согласно настройке [progressive_merge](Server_settings/Common.md#progressive_merge). Однако чанки с большим количеством удалённых документов с большей вероятностью будут слиты.
* [Commit ce6c](https://github.com/manticoresoftware/columnar/commit/ce6c5d3381edcdc411a00a5923d13d56dd9b3010) Добавлена защита от загрузки вторичного индекса более новой версии.
* [Issue #1417](https://github.com/manticoresoftware/manticoresearch/issues/1417) Частичная замена через [REPLACE INTO ... SET](Data_creation_and_modification/Updating_documents/REPLACE.md#SQL-REPLACE).
* [Commit 7c16](https://github.com/manticoresoftware/manticoresearch/commit/7c167b934ce416d3dfbd658744a231e638b14d9a) Обновлены размеры буфера слияния по умолчанию: `.spa` (скалярные атрибуты): 256КБ -> 8МБ; `.spb` (blob-атрибуты): 256КБ -> 8МБ; `.spc` (колоночные атрибуты): 1МБ, без изменений; `.spds` (docstore): 256КБ -> 8МБ; `.spidx` (вторичные индексы): буфер 256КБ -> лимит памяти 128МБ; `.spi` (словарь): 256КБ -> 16МБ; `.spd` (doclists): 8МБ, без изменений; `.spp` (hitlists): 8МБ, без изменений; `.spe` (skiplists): 256КБ -> 8МБ.
* [Issue #1859](https://github.com/manticoresoftware/manticoresearch/issues/1859) Добавлена [композитная агрегация](Searching/Grouping.md#GROUP-BY-multiple-fields-at-once) через JSON.
* [Commit 216b](https://github.com/manticoresoftware/manticoresearch-buddy/commit/216b824) Отключён PCRE.JIT из-за проблем с некоторыми регулярными выражениями и отсутствием значительного прироста по времени.
* [Commit 55cd](https://github.com/manticoresoftware/manticoresearch/commit/55cdce32) Добавлена поддержка vanilla Galera v.3 (api v25) (`libgalera_smm.so` из MySQL 5.x).
* [Commit 86f9](https://github.com/manticoresoftware/manticoresearch-buddy/commit/86f98ab) Суффикс метрики изменён с `_rate` на `_rps`.
* [Commit c0c1](https://github.com/manticoresoftware/manticoresearch-helm/commit/c0c1cff780031412ef57cefab987dfd16613368e) Улучшена документация по поддержке HA балансировщика.
* [Commit d1d2](https://github.com/manticoresoftware/manticoresearch/commit/d1d2ca18857c68106e89743756260e73285be739) Изменено `index` на `table` в сообщениях об ошибках; исправлено сообщение об ошибке парсера bison.
* [Commit fd26](https://github.com/manticoresoftware/manticoresearch/commit/fd26671b) Поддержка `manticore.tbl` как имени таблицы.
* [Issue #1105](https://github.com/manticoresoftware/manticoresearch/issues/1105) Поддержка запуска индексера через systemd ([документация](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Schedule-indexer-via-systemd)). ❤️ Спасибо, [@subnix](https://github.com/subnix) за PR.
* [Issue #1294](https://github.com/manticoresoftware/manticoresearch/issues/1294) Поддержка вторичных индексов в GEODIST().
* [Issue #1394](https://github.com/manticoresoftware/manticoresearch/issues/1394) Упрощён [SHOW THREADS](Node_info_and_management/SHOW_THREADS.md#SHOW-THREADS).
* [Issue #1424](https://github.com/manticoresoftware/manticoresearch/issues/1424) Добавлена поддержка значений по умолчанию (`agent_connect_timeout` и `agent_query_timeout`) для оператора `create distributed table`.
* [Issue #1442](https://github.com/manticoresoftware/manticoresearch/issues/1442) Добавлена опция поискового запроса [expansion_limit](Searching/Options.md#expansion_limit), которая переопределяет `searchd.expansion_limit`.
* [Issue #1448](https://github.com/manticoresoftware/manticoresearch/issues/1448) Реализована команда [ALTER TABLE](Updating_table_schema_and_settings.md#Updating-table-schema-in-RT-mode) для конвертации int в bigint.
* [Issue #1456](https://github.com/manticoresoftware/manticoresearch/issues/1456) Метаинформация в ответе MySQL.
* [Issue #1494](https://github.com/manticoresoftware/manticoresearch/issues/1494) [SHOW VERSION](Node_info_and_management/SHOW_VERSION.md#SHOW-VERSION).
* [Issue #1582](https://github.com/manticoresoftware/manticoresearch/issues/1582) Поддержка [удаления документов по массиву id](Data_creation_and_modification/Deleting_documents.md?client=JSON#Deleting-documents) через JSON.
* [Issue #1589](https://github.com/manticoresoftware/manticoresearch/issues/1589) Улучшено сообщение об ошибке "unsupported value type".
* [Issue #1634](https://github.com/manticoresoftware/manticoresearch/issues/1634) Добавлена версия Buddy в `SHOW STATUS`.
* [Issue #1641](https://github.com/manticoresoftware/manticoresearch/issues/1641) Оптимизация запросов на совпадение в случае нуля документов по ключевому слову.
* [Issue #1712](https://github.com/manticoresoftware/manticoresearch/issues/1712) Добавлено преобразование к булевому атрибуту из строкового значения "true" и "false" при отправке данных.
* [Issue #1713](https://github.com/manticoresoftware/manticoresearch/issues/1713) Добавлена опция таблицы и searchd [access_dict](Server_settings/Searchd.md#access_dict).
* [Issue #1767](https://github.com/manticoresoftware/manticoresearch/issues/1767) Добавлены новые опции: [expansion_merge_threshold_docs](Server_settings/Searchd.md#expansion_merge_threshold_docs) и [expansion_merge_threshold_hits](Server_settings/Searchd.md#expansion_merge_threshold_hits) в разделе searchd конфига; сделан порог для объединения мелких терминов расширенных терминов настраиваемым.
* [Issue #1768](https://github.com/manticoresoftware/manticoresearch/issues/1768) Добавлено отображение времени последней команды в `@@system.sessions`.
* [Issue #1806](https://github.com/manticoresoftware/manticoresearch/issues/1806) Обновлен протокол Buddy до версии 2.
* [Issue #1810](https://github.com/manticoresoftware/manticoresearch/issues/1810) Реализованы дополнительные форматы запросов к эндпоинту `/sql` для упрощения интеграции с библиотеками.
* [Issue #1825](https://github.com/manticoresoftware/manticoresearch/pull/1825) Добавлен раздел Информация к SHOW BUDDY PLUGINS.
* [Issue #1837](https://github.com/manticoresoftware/manticoresearch/issues/1837) Улучшено потребление памяти в `CALL PQ` с большими пакетами.
* [Issue #1853](https://github.com/manticoresoftware/manticoresearch/pull/1853) Переключение компилятора с Clang 15 на Clang 16.
* [Issue #1857](https://github.com/manticoresoftware/manticoresearch/issues/1857) Добавлено строковое сравнение. ❤️ Спасибо, [@etcd](https://github.com/etcd) за PR.
* [Issue #1915](https://github.com/manticoresoftware/manticoresearch/issues/1915) Добавлена поддержка объединённых хранимых полей.
* [Issue #1937](https://github.com/manticoresoftware/manticoresearch/issues/1937) В лог запросов добавлены host:port клиента.
* [Issue #1981](https://github.com/manticoresoftware/manticoresearch/issues/1981) Исправлена некорректная ошибка.
* [Issue #1983](https://github.com/manticoresoftware/manticoresearch/issues/1983) Введена поддержка уровней детализации для [плана запроса через JSON](Node_info_and_management/Profiling/Query_plan.md?client=JSON#Query-plan).
* [Issue #2010](https://github.com/manticoresoftware/manticoresearch/issues/2010) Отключено логирование запросов от Buddy, если не задан `log_level=debug`.
* [Issue #2035](https://github.com/manticoresoftware/manticoresearch/issues/2035) Поддержка Linux Mint 21.3.
* [Issue #2056](https://github.com/manticoresoftware/manticoresearch/issues/2056) Manticore не мог быть собран с Mysql 8.3+.
* [Issue #2112](https://github.com/manticoresoftware/manticoresearch/issues/2112) Команда `DEBUG DEDUP` для чанков таблиц реального времени, которые могут содержать дубликаты после присоединения plain таблицы с дубликатами.
* [Issue #212](https://github.com/manticoresoftware/manticoresearch-buddy/issues/212) Добавлено время к [SHOW QUERIES](Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES).
* [Issue #218](https://github.com/manticoresoftware/manticoresearch-buddy/issues/218) Обработка колонки `@timestamp` как timestamp.
* [Issue #252](https://github.com/manticoresoftware/manticoresearch-buddy/issues/250) Реализована логика включения/выключения плагинов buddy.
* [Issue #254](https://github.com/manticoresoftware/manticoresearch-buddy/issues/254) Обновлен composer до более свежей версии, в которой исправлены последние CVE.
* [Issue #340](https://github.com/manticoresoftware/manticoresearch/issues/340) Небольшая оптимизация в unit-файле systemd для Manticore, связанная с `RuntimeDirectory`.
* [Issue #51](https://github.com/manticoresoftware/executor/issues/51) Добавлена поддержка rdkafka и обновлено до PHP 8.3.3.
* [Issue #527](https://github.com/manticoresoftware/manticoresearch/issues/527) Поддержка [присоединения](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Attaching_one_table_to_another.md#Attaching-one-table-to-another) RT-таблицы. Новая команда [ALTER TABLE ... RENAME](Updating_table_schema_and_settings.md#Renaming-a-real-time-table).

### Изменения с нарушением обратной совместимости и удаление функций
* ⚠️[Issue #1436](https://github.com/manticoresoftware/manticoresearch/issues/1436) Исправлена ошибка расчёта IDF. `local_df` теперь используется по умолчанию. Улучшен протокол поиска master-agent (версия обновлена). Если вы используете Manticore Search в распределённой среде с несколькими инстансами, убедитесь, что сначала обновили агенты, затем мастера.
* ⚠️[Issue #1572](https://github.com/manticoresoftware/manticoresearch/issues/1572) Добавлена репликация распределённых таблиц и обновлён протокол репликации. Если у вас кластер с репликацией, необходимо:
  - Сначала корректно остановить все узлы
  - Затем запустить последний остановленный узел с параметром `--new-cluster`, используя инструмент `manticore_new_cluster` в Linux.
  - Подробности смотрите в разделе [перезапуск кластера](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster).
* ⚠️[Issue #1763](https://github.com/manticoresoftware/manticoresearch/pull/1763) HTTP API: псевдонимы конечных точек `/json/*` объявлены устаревшими.
* ⚠️[Issue #1982](https://github.com/manticoresoftware/manticoresearch/issues/1982) В JSON изменено [profile](Node_info_and_management/Profiling/Query_profile.md#Query-profile) на [plan](Node_info_and_management/Profiling/Query_plan.md#Query-plan), добавлено профилирование запросов для JSON.
* ⚠️[Commit e235](https://github.com/manticoresoftware/manticoresearch-backup/commit/e23585a) manticore-backup больше не создаёт резервные копии `plugin_dir`.
* ⚠️[Issue #171](https://github.com/manticoresoftware/manticoresearch-buddy/issues/155) Buddy мигрировал на Swoole для повышения производительности и стабильности. При переходе на новую версию убедитесь, что все пакеты Manticore обновлены.
* ⚠️[Issue #196](https://github.com/manticoresoftware/manticoresearch-buddy/pull/205) Объединены все основные плагины в Buddy, изменена базовая логика.
* ⚠️[Issue #2107](https://github.com/manticoresoftware/manticoresearch/issues/2107) В ответах `/search` идентификаторы документов обрабатываются как числа.
* ⚠️[Issue #38](https://github.com/manticoresoftware/executor/issues/38) Добавлен Swoole, отключён ZTS и удалено расширение parallel.
* ⚠️[Issue #1929](https://github.com/manticoresoftware/manticoresearch/issues/1929) Переопределение в `charset_table` не работало в некоторых случаях.

### Изменения, связанные с репликацией
* [Commit 3376](https://github.com/manticoresoftware/manticoresearch/commit/3376351f73db3ff0cb6cb708a304a7bd83e69cfb) Исправлена ошибка репликации при SST больших файлов.
* [Commit 6d36](https://github.com/manticoresoftware/manticoresearch/commit/6d36c68fb7ce3cb274b467aac435788905084952) Добавлен механизм повторных попыток для команд репликации; исправлен сбой репликационного подключения при занятой сети с потерей пакетов.
* [Commit 842e](https://github.com/manticoresoftware/manticoresearch/commit/842e27e71790833ab4125d49103d2933d5df60c3) Сообщение FATAL в репликации изменено на WARNING.
* [Commit 8c32](https://github.com/manticoresoftware/manticoresearch/commit/8c32bce7db39d8ed3b1a06aaa1d2c3ec372084f0) Исправлен расчёт `gcache.page_size` для репликационных кластеров без таблиц или с пустыми таблицами; также исправлено сохранение и загрузка опций Galera.
* [Commit a2af](https://github.com/manticoresoftware/manticoresearch/commit/a2af06ca36abcbf893e43c8803853130b397c54b) Добавлена возможность пропускать команду репликации обновления узлов на узле, присоединяющемся к кластеру.
* [Commit c054](https://github.com/manticoresoftware/manticoresearch/commit/c0541e6892f5116b1f957b40060ba8ecd01a5afb) Исправлен дедлок при репликации при обновлении blob-атрибутов и замене документов.
* [Commit e80d](https://github.com/manticoresoftware/manticoresearch/commit/e80d505b963999016033613c40c1cf3151d648e7) Добавлены опции конфигурации searchd [replication_connect_timeout](Server_settings/Searchd.md#replication_connect_timeout), [replication_query_timeout](Server_settings/Searchd.md#replication_query_timeout), [replication_retry_delay](Server_settings/Searchd.md#replication_retry_delay), [replication_retry_count](Server_settings/Searchd.md#replication_retry_count) для управления сетью во время репликации, аналогично `searchd.agent_*`, но с другими значениями по умолчанию.
* [Issue #1356](https://github.com/manticoresoftware/manticoresearch/issues/1356) Исправлен повторный запрос узлов репликации после пропуска некоторых узлов и сбоя разрешения их имён.
* [Issue #1445](https://github.com/manticoresoftware/manticoresearch/issues/1445) Исправлен уровень детализации логов репликации в `show variables`.
* [Issue #1482](https://github.com/manticoresoftware/manticoresearch/issues/1482) Исправлена проблема репликации узла, присоединяющегося к кластеру после перезапуска пода в Kubernetes.
* [Issue #1962](https://github.com/manticoresoftware/manticoresearch/issues/1962) Исправлено долгое ожидание репликации ALTER на пустом кластере с некорректным именем узла.

### Исправления ошибок
* [Commit 8a48](https://github.com/manticoresoftware/manticoresearch/commit/8a486c7ba1a293879cdecc41fb52eb7ab0cbf832) Исправлена очистка неиспользуемых совпадений в `count distinct`, которая могла вызывать сбой.
* [Issue #1569](https://github.com/manticoresoftware/manticoresearch/issues/1569) Двоичный журнал теперь записывается с атомарностью транзакций.
* [Issue #2089](https://github.com/manticoresoftware/manticoresearch/issues/2089) Исправлена ошибка, связанная с 64-битными идентификаторами, которая могла привести к ошибке «Malformed packet» при вставке через MySQL, что вызывало [повреждение таблиц и дублирование идентификаторов](https://github.com/manticoresoftware/manticoresearch/issues/2090).
* [Issue #2160](https://github.com/manticoresoftware/manticoresearch/issues/2160) Исправлена вставка дат, которые воспринимались как находящиеся в UTC вместо часового пояса по умолчанию.
* [Issue #2177](https://github.com/manticoresoftware/manticoresearch/issues/2177) Исправлен сбой, возникавший при выполнении поиска в таблице реального времени с непустым `index_token_filter`.
* [Issue #2209](https://github.com/manticoresoftware/manticoresearch/issues/2209) Изменена фильтрация дубликатов в RT колоннарном хранении для исправления сбоев и неверных результатов запросов.
* [Commit 001d](https://github.com/manticoresoftware/manticoresearch/commit/001d82904e4b887c49387308291aeefb07c0fbc4) Исправлен html-стриппер, который повреждал память после обработки объединённого поля.
* [Commit 00eb](https://github.com/manticoresoftware/manticoresearch/commit/00eb2ebf) Избежано перемотки потока после flush для предотвращения проблем с передачей данных с mysqldump.
* [Commit 0553](https://github.com/manticoresoftware/manticoresearch/commit/05534114acf08c70ffcc41a23d558dec09a34373) Не ожидается завершение preread, если он не был запущен.
* [Commit 055a](https://github.com/manticoresoftware/manticoresearch/commit/055aac09f877570cd29b6594cb283182eba7f73b) Исправлена слишком длинная строка вывода Buddy, которая теперь разделяется на несколько строк в логе searchd.
* [Commit 0a88](https://github.com/manticoresoftware/manticoresearch/commit/0a88056307f82c50fcdc458ad6dbc307a00e3f92) Перемещено предупреждение интерфейса MySQL о неудачном заголовке `debugv` уровня подробности.
* [Commit 150a](https://github.com/manticoresoftware/manticoresearch/commit/150a20286b7d98d24b844ae577d7fda4a444798d) Исправлена гонка при выполнении нескольких операций управления кластерами; запрещено создавать несколько кластеров с одинаковым именем или путем.
* [Commit 2e40](https://github.com/manticoresoftware/manticoresearch/commit/2e405c1c9ebac6f46206e3df17ac1c19129c9aa3) Исправлено неявное усечение в полнотекстовых запросах; разбит MatchExtended на шаблонную часть D.
* [Commit 75f5](https://github.com/manticoresoftware/manticoresearch/commit/75f51421542a2a0658f9e2f9f31048c8c5224479) Исправлено несоответствие `index_exact_words` между индексированием и загрузкой таблицы в демона.
* [Commit 7643](https://github.com/manticoresoftware/manticoresearch/commit/7643976bea142e08535d701000eff0d09a99fda2) Исправлено пропущенное сообщение об ошибке при удалении некорректного кластера.
* [Commit 7a03](https://github.com/manticoresoftware/manticoresearch/commit/7a034ac990d5ec9344320c05a6a2315583b9b858) Исправлено объединение CBO и очереди; исправлено псевдо-шардирование CBO и RT.
* [Commit 7b4e](https://github.com/manticoresoftware/manticoresearch/commit/7b4ea95e) При запуске без библиотеки вторичного индекса (SI) и параметров в конфигурации больше не выводится вводящее в заблуждение предупреждение 'WARNING: secondary_indexes set but failed to initialize secondary library'.
* [Commit 8496](https://github.com/manticoresoftware/manticoresearch/commit/8496317f099fade8d2db463fb88483e7fcdfe85b) Исправлена сортировка хитов в режиме кворума.
* [Commit 8973](https://github.com/manticoresoftware/manticoresearch-buddy/commit/8973ad3) Исправлена проблема с опциями в верхнем регистре в плагине ModifyTable.
* [Commit 9935](https://github.com/manticoresoftware/manticoresearch-buddy/commit/99350d4c184379b19f3efa9751d9b5be5e6edfb5) Исправлено восстановление из дампа с пустыми значениями json (представляемыми как NULL).
* [Commit a28f](https://github.com/manticoresoftware/manticoresearch/commit/a28f6e087b3d957e48816070be17d0823904936a) Исправлено превышение таймаута SST на ноде объединения при приеме SST с помощью pcon.
* [Commit b5a5](https://github.com/manticoresoftware/manticoresearch/commit/b5a55cc26debcddcda5d9caef8366a707e0dd763) Исправлен сбой при выборе псевдонимного строкового атрибута.
* [Commit c556](https://github.com/manticoresoftware/manticoresearch/commit/c556e7ed8b6da8eac8ef094650df5f0a8977916b) Добавлено преобразование запроса терма в `=term` для полнотекстового запроса с использованием поля `morphology_skip_fields`.
* [Commit cdc3](https://github.com/manticoresoftware/manticoresearch/commit/cdc38085145cd1025b96fcac2a46188a0425d9e1) Добавлен отсутствующий ключ конфигурации (skiplist_cache_size).
* [Commit cf6e](https://github.com/manticoresoftware/manticoresearch/commit/cf6ec3bfdc3f0990fb31ff53822b2331a6667727) Исправлен сбой ранжировщика выражений при большом сложном запросе.
* [Commit e513](https://github.com/manticoresoftware/manticoresearch/commit/e5139ba75eef7223df375ae3cc3f138fa7d925f1) Исправлен полнотекстовый CBO относительно некорректных подсказок индексирования.
* [Commit eb05](https://github.com/manticoresoftware/manticoresearch/commit/eb0514b3a6f57f278252b0724eebd112ec116e0f) Прерван preread при завершении для ускорения закрытия.
* [Commit f945](https://github.com/manticoresoftware/manticoresearch/commit/f94555a293cfa51004ff81135b8bd8500b955115) Изменён расчёт стека для полнотекстовых запросов для предотвращения сбоя при сложных запросах.
* [Issue #1262](https://github.com/manticoresoftware/manticoresearch/issues/1262) Исправлен сбой индексации, произошедший при индексировании SQL-источника с несколькими столбцами с одинаковыми именами.
* [Issue #1273](https://github.com/manticoresoftware/manticoresearch/issues/1273) Возвращается 0 вместо <empty> для несуществующих системных переменных.
* [Issue #1289](https://github.com/manticoresoftware/manticoresearch/issues/1289) Исправлена ошибка indextool при проверке внешних файлов таблицы RT.
* [Issue #1335](https://github.com/manticoresoftware/manticoresearch/issues/1335) Исправлена ошибка разбора запроса из-за многословной формы внутри фразы.
* [Issue #1364](https://github.com/manticoresoftware/manticoresearch/issues/1364) Добавлен повтор воспроизведения пустых binlog файлов со старыми версиями binlog.
* [Issue #1365](https://github.com/manticoresoftware/manticoresearch/issues/1365) Исправлено удаление последнего пустого файла binlog.
* [Issue #1372](https://github.com/manticoresoftware/manticoresearch/issues/1372) Исправлены неправильные относительные пути (преобразованные в абсолютные из каталога запуска демона) после изменений в `data_dir`, влияющие на текущий рабочий каталог при запуске демона.
* [Issue #1393](https://github.com/manticoresoftware/manticoresearch/issues/1393) Наихудшее ухудшение времени в hn_small: получение/кэширование информации о ЦП при запуске демона.
* [Issue #1395](https://github.com/manticoresoftware/manticoresearch/issues/1395) Исправлено предупреждение о пропущенном внешнем файле при загрузке индекса.
* [Issue #1402](https://github.com/manticoresoftware/manticoresearch/issues/1402) Исправлен сбой в глобальных группировщиках при освобождении указателей данных.
* [Issue #1403](https://github.com/manticoresoftware/manticoresearch/issues/1403) _ADDITIONAL_SEARCHD_PARAMS не работает.
* [Issue #1427](https://github.com/manticoresoftware/manticoresearch/issues/1427) Исправлена замена на значение по умолчанию параметра `agent_query_timeout` для каждой таблицы.
* [Issue #1444](https://github.com/manticoresoftware/manticoresearch/issues/1444) Исправлен сбой в группировщике и ранжировщике при использовании `packedfactors()` с несколькими значениями на совпадение.
* [Issue #1458](https://github.com/manticoresoftware/manticoresearch/issues/1458) Manticore аварийно завершает работу при частых обновлениях индекса.
* [Issue #1481](https://github.com/manticoresoftware/manticoresearch/issues/1481) Исправлен сбой при очистке разобранного запроса после ошибки парсинга.
* [Issue #1484](https://github.com/manticoresoftware/manticoresearch/issues/1484) Исправлена маршрутизация HTTP JSON запросов к buddy.
* [Issue #1499](https://github.com/manticoresoftware/manticoresearch/issues/1499) Корень JSON атрибута не мог быть массивом. Исправлено.
* [Issue #1507](https://github.com/manticoresoftware/manticoresearch/issues/1507) Исправлен сбой при воссоздании таблицы внутри транзакции.
* [Issue #1515](https://github.com/manticoresoftware/manticoresearch/issues/1515) Исправлено расширение сокращённых форм русских лемм.
* [Issue #1579](https://github.com/manticoresoftware/manticoresearch/issues/1579) Исправлено использование JSON и STRING атрибутов в выражении [date_format](Functions/Date_and_time_functions.md#DATE_FORMAT()).
* [Issue #1580](https://github.com/manticoresoftware/manticoresearch/issues/1580) Исправлен группировщик для нескольких псевдонимов JSON полей.
* [Issue #1594](https://github.com/manticoresoftware/manticoresearch/issues/1594) Неверный total_related в dev: исправлен неявный cutoff против limit; добавлено лучшее обнаружение fullscan в JSON запросах.
* [Issue #1603](https://github.com/manticoresoftware/manticoresearch/issues/1603) Исправлено использование JSON и STRING атрибутов во всех выражениях даты.
* [Issue #1609](https://github.com/manticoresoftware/manticoresearch/issues/1609) сбой при использовании LEVENSHTEIN().
* [Issue #1612](https://github.com/manticoresoftware/manticoresearch/issues/1612) Исправлено повреждение памяти после ошибки парсинга поискового запроса с подсветкой.
* [Issue #1614](https://github.com/manticoresoftware/manticoresearch/issues/1614) Отключено расширение шаблонов для терминов короче `min_prefix_len` / `min_infix_len`.
* [Issue #1617](https://github.com/manticoresoftware/manticoresearch/issues/1617) Изменено поведение: не логировать ошибку, если Buddy успешно обрабатывает запрос.
* [Issue #1635](https://github.com/manticoresoftware/manticoresearch/issues/1635) Исправлено значение total в метаданных поискового запроса для запросов с установленным limit.
* [Issue #1640](https://github.com/manticoresoftware/manticoresearch/issues/1640) Невозможно использовать таблицу с заглавной буквой через JSON в plain режиме.
* [Issue #1643](https://github.com/manticoresoftware/manticoresearch/issues/1643) Установлено значение по умолчанию для `SPH_EXTNODE_STACK_SIZE`.
* [Issue #1646](https://github.com/manticoresoftware/manticoresearch/issues/1646) Исправлен лог SphinxQL при отрицательном фильтре с ALL/ANY на атрибуте MVA.
* [Issue #1660](https://github.com/manticoresoftware/manticoresearch/issues/1660) Исправлено применение списков killlists docid из других индексов. ❤️ Спасибо, [@raxoft](https://github.com/raxoft) за PR.
* [Issue #1668](https://github.com/manticoresoftware/manticoresearch/issues/1668) Исправлены пропущенные совпадения из-за преждевременного выхода при полном сканировании raw индекса (без итераторов индексов); удалён cutoff из plain row итератора.
* [Issue #1671](https://github.com/manticoresoftware/manticoresearch/issues/1671) Исправлена ошибка `FACET` при запросе распределённой таблицы с агентом и локальными таблицами.
* [Issue #1690](https://github.com/manticoresoftware/manticoresearch/issues/1690) Исправлен сбой при оценке гистограммы для больших значений.
* [Issue #1692](https://github.com/manticoresoftware/manticoresearch/issues/1692) сбой при alter table tbl add column col uint.
* [Issue #1710](https://github.com/manticoresoftware/manticoresearch/issues/1710) Пустой результат для условия `WHERE json.array IN (<value>)`.
* [Issue #172](https://github.com/manticoresoftware/manticoresearch-buddy/issues/172) Исправлена проблема с TableFormatter при отправке запроса на `/cli`.
* [Issue #1742](https://github.com/manticoresoftware/manticoresearch/issues/1742) `CREATE TABLE` не выдавал ошибку при отсутствии файла wordforms.
* [Issue #1762](https://github.com/manticoresoftware/manticoresearch/issues/1762) Порядок атрибутов в RT таблицах теперь следует порядку конфигурации.
* [Issue #1765](https://github.com/manticoresoftware/manticoresearch/issues/1765) HTTP bool запрос с условием 'should' возвращает неверные результаты.
* [Issue #1769](https://github.com/manticoresoftware/manticoresearch/issues/1769) Сортировка по строковым атрибутам не работает с `SPH_SORT_ATTR_DESC` и `SPH_SORT_ATTR_ASC`.
* [Issue #177](https://github.com/manticoresoftware/manticoresearch-buddy/issues/177) Отключён HTTP заголовок `Expect: 100-continue` для curl запросов к Buddy.
* [Issue #1791](https://github.com/manticoresoftware/manticoresearch/issues/1791) сбой, вызванный alias в GROUP BY.
* [Issue #1792](https://github.com/manticoresoftware/manticoresearch/issues/1792) SQL meta summary показывает неправильное время в windows.
* [Issue #1794](https://github.com/manticoresoftware/manticoresearch/issues/1794) Исправлено падение производительности для однотерминных запросов в JSON.
* [Issue #1798](https://github.com/manticoresoftware/manticoresearch/issues/1798) Несовместимые фильтры не вызывали ошибку при выполнении `/search`.
* [Issue #1802](https://github.com/manticoresoftware/manticoresearch/issues/1802) Исправлены операции `ALTER CLUSTER ADD` и `JOIN CLUSTER` для ожидания друг друга, предотвращая состояние гонки, когда `ALTER` добавляет таблицу в кластер, а донор отправляет таблицы на узел-присоединитель.
* [Issue #1811](https://github.com/manticoresoftware/manticoresearch/issues/1811) Некорректная обработка запросов `/pq/{table}/*`.
* [Issue #1816](https://github.com/manticoresoftware/manticoresearch/issues/1816) `UNFREEZE` не работал в некоторых случаях.
* [Issue #183](https://github.com/manticoresoftware/manticoresearch-buddy/pull/183) Исправлена проблема с восстановлением MVA в некоторых случаях.
* [Issue #1849](https://github.com/manticoresoftware/manticoresearch/issues/1849) Исправлен сбой indextool при завершении работы, если используется с MCL.
* [Issue #1866](https://github.com/manticoresoftware/manticoresearch/issues/1866) Исправлено ненужное декодирование URL для запросов `/cli_json`.
* [Issue #1872](https://github.com/manticoresoftware/manticoresearch/issues/1872) Изменена логика установки plugin_dir при запуске демона.
* [Issue #1874](https://github.com/manticoresoftware/manticoresearch/issues/1874) Ошибка при исключениях alter table ...
* [Issue #1891](https://github.com/manticoresoftware/manticoresearch/issues/1891#issuecomment-2051941200) Manticore аварийно завершался с `signal 11` при вставке данных.
* [Issue #1920](https://github.com/manticoresoftware/manticoresearch/issues/1920) Снижено ограничение для [low_priority](Searching/Options.md#low_priority).
* [Issue #1924](https://github.com/manticoresoftware/manticoresearch/issues/1924) Ошибка при mysqldump + mysql restore.
* [Issue #1951](https://github.com/manticoresoftware/manticoresearch/issues/1951) Исправлено некорректное создание распределённой таблицы в случае отсутствия локальной таблицы или некорректного описания агента; теперь возвращается сообщение об ошибке.
* [Issue #1972](https://github.com/manticoresoftware/manticoresearch/issues/1972) Реализован счётчик `FREEZE` для избежания проблем со freeze/unfreeze.
* [Issue #1980](https://github.com/manticoresoftware/manticoresearch/issues/1980) Соблюдение таймаута запроса в узлах OR. Ранее `max_query_time` мог не работать в некоторых случаях.
* [Issue #1986](https://github.com/manticoresoftware/manticoresearch/issues/1986) Не удалось переименовать new в current [manticore.json].
* [Issue #1988](https://github.com/manticoresoftware/manticoresearch/issues/1988) Запрос полного текста мог игнорировать подсказку `SecondaryIndex` CBO.
* [Issue #1990](https://github.com/manticoresoftware/manticoresearch/issues/1990) Исправлен `expansion_limit` для среза итогового набора результатов при вызове ключевых слов из нескольких дисковых или оперативных чанков.
* [Issue #1994](https://github.com/manticoresoftware/manticoresearch/issues/1994) неправильные внешние файлы.
* [Issue #2021](https://github.com/manticoresoftware/manticoresearch/issues/2021) Несколько процессов manticore-executor могли оставаться запущенными после остановки Manticore.
* [Issue #2029](https://github.com/manticoresoftware/manticoresearch/issues/2029) Сбой при использовании расстояния Левенштейна.
* [Issue #2037](https://github.com/manticoresoftware/manticoresearch/issues/2037) Ошибка после многократного запуска оператора max на пустом индексе.
* [Issue #2052](https://github.com/manticoresoftware/manticoresearch/issues/2052) сбой на multi-group с JSON.field.
* [Issue #2067](https://github.com/manticoresoftware/manticoresearch/issues/2067) Manticore аварийно завершался при некорректном запросе к _update.
* [Issue #2069](https://github.com/manticoresoftware/manticoresearch/issues/2069) Исправлена проблема с компараторами строковых фильтров для закрытых диапазонов в JSON-интерфейсе.
* [Issue #2082](https://github.com/manticoresoftware/manticoresearch/issues/2082) `alter` не работал, если путь data_dir находился на символической ссылке.
* [Issue #2102](https://github.com/manticoresoftware/manticoresearch/pull/2102) Улучшена специальная обработка SELECT-запросов в mysqldump для обеспечения совместимости результирующих операторов INSERT с Manticore.
* [Issue #2103](https://github.com/manticoresoftware/manticoresearch/issues/2103) Тайские символы были в неправильных наборах символов.
* [Issue #2124](https://github.com/manticoresoftware/manticoresearch/issues/2124) Сбой при использовании SQL с зарезервированным словом.
* [Issue #2154](https://github.com/manticoresoftware/manticoresearch/issues/2154) Таблицы с wordforms нельзя было импортировать.
* [Issue #2176](https://github.com/manticoresoftware/manticoresearch/issues/2176) Исправлен сбой, который возникал, когда параметр engine был установлен в 'columnar' и дублирующиеся ID добавлялись через JSON.
* [Issue #223](https://github.com/manticoresoftware/manticoresearch-buddy/issues/223) Корректная ошибка при попытке вставить документ без схемы и без имён столбцов.
* [Issue #239](https://github.com/manticoresoftware/manticoresearch-buddy/issues/239) Автоматическая схема множественной вставки могла сбойнуть.
* [Issue #399](https://github.com/manticoresoftware/manticoresearch/issues/399) Добавлено сообщение об ошибке при индексировании, если в источнике данных объявлен атрибут id.
* [Issue #59](https://github.com/manticoresoftware/manticoresearch-helm/issues/59) Сбой кластера Manticore.
* [Issue #68](https://github.com/manticoresoftware/manticoresearch-helm/issues/68) optimize.php падал, если присутствовала таблица percolate.
* [Issue #77](https://github.com/manticoresoftware/manticoresearch-helm/issues/77) Исправлены ошибки при развертывании на Kubernetes.
* [Issue #274](https://github.com/manticoresoftware/manticoresearch-buddy/issues/274) Исправлена некорректная обработка параллельных запросов к Buddy

### Связано с manticore-backup
* [Issue #97](https://github.com/manticoresoftware/manticoresearch-backup/pull/97) Установлен VIP HTTP порт по умолчанию, если доступен.
Различные улучшения: улучшена проверка версий и потоковая декомпрессия ZSTD; добавлены запросы к пользователю при несовпадении версий во время восстановления; исправлено некорректное поведение запросов для разных версий при восстановлении; улучшена логика декомпрессии для чтения непосредственно из потока, а не в рабочую память; добавлен флаг `--force`
* [Commit 3b35](https://github.com/manticoresoftware/manticoresearch-backup/commit/3b357ea) Добавлено отображение версии бэкапа после запуска Manticore search для выявления проблем на этом этапе.
* [Commit ad2e](https://github.com/manticoresoftware/manticoresearch-backup/commit/ad2e5da) Обновлено сообщение об ошибке при неудачном подключении к демону.
* [Commit ce5e](https://github.com/manticoresoftware/manticoresearch-backup/commit/ce5ea85) Исправлена проблема с преобразованием абсолютных корневых путей бэкапа в относительные и удалена проверка на возможность записи при восстановлении для возможности восстановления из других путей.
* [Commit db7e](https://github.com/manticoresoftware/manticoresearch-backup/commit/db7e2b9) Добавлена сортировка к итератору файлов для обеспечения последовательности в различных ситуациях.
* [Issue #106](https://github.com/manticoresoftware/manticoresearch-backup/issues/105) Бэкап и восстановление нескольких конфигураций.
* [Issue #91](https://github.com/manticoresoftware/manticoresearch-backup/pull/91) Добавлен defattr для предотвращения необычных пользовательских прав на файлы после установки на RHEL.
* [Issue #91](https://github.com/manticoresoftware/manticoresearch-backup/pull/91) Добавлен дополнительный chown для гарантии, что файлы по умолчанию принадлежат пользователю root в Ubuntu.

### Связано с MCL (columnar, secondary, knn libs)
* [Commit f104](https://github.com/manticoresoftware/columnar/commit/f104a4709bbb144140d09a09bc0fc1db1895e331) Поддержка векторного поиска.
* [Commit 2169](https://github.com/manticoresoftware/columnar/commit/216953beef6676f7e34c9f6f1a30c5980d746710) Исправлена очистка временных файлов при прерванной настройке создания вторичного индекса. Это решает проблему, когда демон превышал лимит открытых файлов при создании файлов `tmp.spidx`.
* [Commit 709b](https://github.com/manticoresoftware/columnar/commit/709b9acaaac97d9a1ca8796892f9ad432021c785) Использование отдельной библиотеки streamvbyte для columnar и SI.
* [Commit 1c26](https://github.com/manticoresoftware/manticoresearch/commit/1c2623763ced14bb94ad624dde9614a938408d89) Добавлено предупреждение о том, что столбцовое хранение не поддерживает JSON атрибуты.
* [Commit 3acd](https://github.com/manticoresoftware/columnar/commit/3acd6714ffce103447d7931dfc75bb1a7449f340) Исправлена распаковка данных в SI.
* [Commit 574c](https://github.com/manticoresoftware/manticoresearch/commit/574c023152564fa72092c18ebd1f594abfd4b6d7) Исправлен сбой при сохранении дискового блока с комбинированным строковым и столбцовым хранением.
* [Commit e87f](https://github.com/manticoresoftware/columnar/commit/e87f088b458d4c5b1b6159fc72ed0d57b786891a) Исправлен итератор SI, который указывал на уже обработанный блок.
* [Issue #1474](https://github.com/manticoresoftware/manticoresearch/issues/1474) Обновление сломано для строковой колонки MVA с использованием столбцового движка.
* [Issue #1510](https://github.com/manticoresoftware/manticoresearch/issues/1510) Исправлен сбой при агрегации по столбцовому атрибуту, используемому в `HAVING`.
* [Issue #1519](https://github.com/manticoresoftware/manticoresearch/issues/1519) Исправлен сбой в ранжировщике `expr` при использовании столбцового атрибута.

### Связано с Docker
* ❗[Issue #42](https://github.com/manticoresoftware/docker/issues/42) Поддержка [plain indexation](https://github.com/manticoresoftware/docker#building-plain-tables) через переменные окружения.
* ❗[Issue #47](https://github.com/manticoresoftware/docker/issues/47) Улучшена гибкость настройки конфигурации через переменные окружения.
* [Issue #54](https://github.com/manticoresoftware/docker/pull/54) Улучшены процессы [бэкапа и восстановления](https://github.com/manticoresoftware/docker#backup-and-restore) для Docker.
* [Issue #77](https://github.com/manticoresoftware/docker/issues/76) Улучшен entrypoint для обработки восстановления бэкапа только при первом старте.
* [Commit a27c](https://github.com/manticoresoftware/docker/commit/a27c048dd9e73d8d6bea8d9a4830cafb486b82c5) Исправлен лог запросов в stdout.
* [Issue #38](https://github.com/manticoresoftware/docker/issues/38) Приглушены предупреждения BUDDY, если EXTRA не задан.
* [Issue #71](https://github.com/manticoresoftware/docker/pull/71) Исправлено имя хоста в `manticore.conf.sh`.

## Версия 6.2.12
Выпущена: 23 августа 2023

Версия 6.2.12 продолжает серию 6.2 и устраняет проблемы, обнаруженные после выпуска 6.2.0.

### Исправления ошибок
* ❗[Issue #1351](https://github.com/manticoresoftware/manticoresearch/issues/1351) «Manticore 6.2.0 не запускается через systemctl на Centos 7»: Изменён `TimeoutStartSec` с `infinity` на `0` для лучшей совместимости с Centos 7.
* ❗[Issue #1364](https://github.com/manticoresoftware/manticoresearch/issues/1364) «Сбой после обновления с 6.0.4 до 6.2.0»: Добавлена функция воспроизведения для пустых binlog файлов из старых версий binlog.
* [PR #1334](https://github.com/manticoresoftware/manticoresearch/pull/1334) «исправление опечатки в searchdreplication.cpp»: Исправлена опечатка в `searchdreplication.cpp`: beggining -> beginning.
* [Issue #1337](https://github.com/manticoresoftware/manticoresearch/issues/1337) «Manticore 6.2.0 WARNING: conn (local)(12), sock=8088: bailing on failed MySQL header, AsyncNetInputBuffer_c::AppendData: error 11 (Resource temporarily unavailable) return -1»: Уровень подробности предупреждения интерфейса MySQL о заголовке понижен до logdebugv.
* [Issue #1355](https://github.com/manticoresoftware/manticoresearch/issues/1355) «join cluster зависает, когда node_address не удаётся разрешить»: Улучшен повтор попыток репликации, когда некоторые узлы недоступны и их разрешение имён не выполняется. Это решает проблемы, возникающие в Kubernetes и Docker узлах, связанные с репликацией. Улучшено сообщение об ошибке при неудаче запуска репликации и внесены обновления в тестовую модель 376. Также добавлено ясное сообщение об ошибке при сбое разрешения имён.
* [Issue #1361](https://github.com/manticoresoftware/manticoresearch/issues/1361) "No lower case mapping for "Ø" in charset non_cjk": Скорректирована сопоставление для символа 'Ø'.
* [Issue #1365](https://github.com/manticoresoftware/manticoresearch/issues/1365) "searchd leaves binlog.meta and binlog.001 after clean stop": Обеспечено правильное удаление последнего пустого файла binlog.
* [Commit 0871](https://github.com/manticoresoftware/manticoresearch/commit/0871070845e93ad702ae0dfb89b35e5f24cd0851): Исправлена проблема сборки `Thd_t` на Windows, связанная с ограничениями атомарного копирования.
* [Commit 1cc0](https://github.com/manticoresoftware/manticoresearch/commit/1cc04bedc174737118fd5a3663e35f4e95bd4f8c): Решена проблема с FT CBO и `ColumnarScan`.
* [Commit c6bf](https://github.com/manticoresoftware/manticoresearch/commit/c6bfb30d53e3f4b67f17769667733677a661c95b): Внесены исправления в тест 376 и добавлена замена для ошибки `AF_INET` в тесте.
* [Commit cbc3](https://github.com/manticoresoftware/manticoresearch/commit/cbc38705e0b4dcc9665e3e8cf64826290ebd11cf): Решена проблема взаимной блокировки при репликации при обновлении атрибутов blob против замены документов. Также удалена блокировка rlock индекса во время коммита, так как она уже заблокирована на более базовом уровне.

### Небольшие изменения
* [Commit 4f91](https://github.com/manticoresoftware/manticoresearch/commit/4f913a36e84e01f950b2797051222b0b1094d6fe) Обновлена информация о эндпоинтах `/bulk` в руководстве.

### MCL
* Поддержка [Manticore Columnar Library](https://github.com/manticoresoftware/columnar) версии 2.2.4

## Версия 6.2.0
Выпущена: 4 августа 2023

### Ключевые изменения
* Оптимизатор запросов был улучшен для поддержки полнотекстовых запросов, что значительно повысило эффективность и производительность поиска.
* Интеграции с:
  - [mysqldump](https://dev.mysql.com/doc/refman/8.0/en/mysqldump.html) — для [создания логических бэкапов](Securing_and_compacting_a_table/Backup_and_restore.md#Backup-and-restore-with-mysqldump) с помощью `mysqldump`
  - [Apache Superset](https://manticoresearch.com/blog/manticoresearch-apache-superset-integration/) и [Grafana](https://manticoresearch.com/blog/manticoresearch-grafana-integration/) для визуализации данных, хранящихся в Manticore
  - [HeidiSQL](https://www.heidisql.com/) и [DBForge](https://www.devart.com/dbforge/) для облегчения разработки с Manticore
* Мы начали использовать [GitHub workflows](https://github.com/manticoresoftware/manticoresearch/actions), что упрощает для контрибьюторов применение того же процесса непрерывной интеграции (CI), что и у основной команды при подготовке пакетов. Все задачи могут запускаться на раннерах GitHub, что облегчает бесшовное тестирование изменений в вашем форке Manticore Search.
* Мы начали использовать [CLT](https://github.com/manticoresoftware/clt) для тестирования сложных сценариев. Например, теперь мы можем гарантировать, что пакет, собранный после коммита, может быть корректно установлен на всех поддерживаемых дистрибутивах Linux. Command Line Tester (CLT) предоставляет удобный способ записывать тесты в интерактивном режиме и легко воспроизводить их.
* Значительное улучшение производительности операции подсчёта уникальных значений (count distinct) за счёт использования комбинации хеш-таблиц и HyperLogLog.
* Включено многопоточное выполнение запросов, содержащих вторичные индексы, с числом потоков, ограниченным количеством физических ядер ЦП. Это значительно улучшит скорость выполнения запросов.
* `pseudo_sharding` был настроен так, чтобы ограничиваться количеством свободных потоков. Это обновление значительно повышает производительность пропускной способности.
* Пользователи теперь могут указать [движок хранения атрибутов по умолчанию](Server_settings/Searchd.md#engine) через настройки конфигурации, обеспечивая лучшую настройку под конкретные требования нагрузки.
* Поддержка [Manticore Columnar Library 2.2.0](https://github.com/manticoresoftware/columnar/) с множеством исправлений ошибок и улучшений в [вторичных индексах](Server_settings/Searchd.md#secondary_indexes).

### Небольшие изменения
* [Buddy #153](https://github.com/manticoresoftware/manticoresearch-buddy/issues/153): HTTP эндпоинт [/pq](Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md?client=JSON#Adding-rules-to-a-percolate-table) теперь служит псевдонимом для HTTP эндпоинта `/json/pq`.
* [Commit 0bf1](https://github.com/manticoresoftware/manticoresearch/commit/0bf17d9e): Обеспечена многобайтовая совместимость для функций `upper()` и `lower()`.
* [Commit 2bb9](https://github.com/manticoresoftware/manticoresearch/commit/2bb92765): Вместо сканирования индекса для запросов `count(*)` теперь возвращается предварительно рассчитанное значение.
* [Commit 3c84](https://github.com/manticoresoftware/manticoresearch/commit/3c84091f): Теперь возможно использовать `SELECT` для выполнения произвольных вычислений и отображения `@@sysvars`. В отличие от предыдущего состояния, вы больше не ограничены одним вычислением. Таким образом, запросы вроде `select user(), database(), @@version_comment, version(), 1+1 as a limit 10` будут возвращать все колонки. Обратите внимание, что опциональный 'limit' всегда игнорируется.
* [Commit 6aca](https://github.com/manticoresoftware/manticoresearch/commit/6aca32f0): Реализован заглушечный запрос `CREATE DATABASE`.
* [Commit 9dc1](https://github.com/manticoresoftware/manticoresearch/commit/9dc12334): При выполнении `ALTER TABLE table REBUILD SECONDARY` вторичные индексы теперь всегда перестраиваются, даже если атрибуты не обновлялись.
* [Commit 46ed](https://github.com/manticoresoftware/manticoresearch/commit/46edb089): Сортировщики, использующие предварительно рассчитанные данные, теперь выявляются до применения CBO, чтобы избежать ненужных вычислений CBO.
* [Commit 102a](https://github.com/manticoresoftware/manticoresearch/commit/102ac604): Реализован и применён макет стеков полнотекстовых выражений для предотвращения сбоев демона.
* [Commit 979f](https://github.com/manticoresoftware/manticoresearch/commit/979fa27c): Добавлен быстрый путь выполнения кода клонирования совпадений для совпадений, которые не используют атрибуты string/mvas/json.
* [Коммит a073](https://github.com/manticoresoftware/manticoresearch/commit/a0735ffe): Добавлена поддержка команды `SELECT DATABASE()`. Однако она всегда будет возвращать `Manticore`. Это дополнение важно для интеграции с различными MySQL-инструментами.
* [Коммит bc04](https://github.com/manticoresoftware/manticoresearch/commit/bc04908): Изменён формат ответа конечной точки [/cli](Connecting_to_the_server/HTTP.md#/cli), добавлена конечная точка `/cli_json`, которая функционирует как предыдущая `/cli`.
* [Коммит d70b](https://github.com/manticoresoftware/manticoresearch/commit/d70b0d58): `thread_stack` теперь можно изменять во время выполнения с помощью оператора `SET`. Доступны локальные для сессии и глобальные для демона варианты. Текущие значения доступны в выводе `show variables`.
* [Коммит d96e](https://github.com/manticoresoftware/manticoresearch/commit/d96ec6b9): Код был интегрирован в CBO для более точной оценки сложности выполнения фильтров по строковым атрибутам.
* [Коммит e77d](https://github.com/manticoresoftware/manticoresearch/commit/e77dd72f5a04531c352fad0d7afcd2a1cbae2510): Улучшен расчёт стоимости DocidIndex, что повысило общую производительность.
* [Коммит f3ae](https://github.com/manticoresoftware/manticoresearch/commit/f3ae8bea): Метрики загрузки, аналогичные 'uptime' в Linux, теперь видны в команде `SHOW STATUS`.
* [Коммит f3cc](https://github.com/manticoresoftware/manticoresearch/commit/f3cc0971): Порядок полей и атрибутов для `DESC` и `SHOW CREATE TABLE` теперь совпадает с `SELECT * FROM`.
* [Коммит f3d2](https://github.com/manticoresoftware/manticoresearch/commit/f3d248a6): Различные внутренние парсеры теперь при различных ошибках предоставляют свой внутренний мнемонический код (например, `P01`). Это облегчает определение парсера, вызвавшего ошибку, и скрывает несущественные внутренние детали.
* [Issue #271](https://github.com/manticoresoftware/manticoresearch/issues/271) "Иногда CALL SUGGEST не предлагает исправление опечатки из одной буквы": Улучшено поведение [SUGGEST/QSUGGEST](Searching/Spell_correction.md#CALL-QSUGGEST,-CALL-SUGGEST) для коротких слов: добавлена опция `sentence` для отображения всего предложения.
* [Issue #696](https://github.com/manticoresoftware/manticoresearch/issues/696) "Индекс Percolate некорректно ищет по точной фразе при включённом стемминге": Модифицирован запрос percolate для обработки модификатора точного термина, что улучшило функциональность поиска.
* [Issue #829](https://github.com/manticoresoftware/manticoresearch/issues/829) "Методы форматирования дат": добавлено выражение списка выбора [date_format()](Functions/Date_and_time_functions.md#DATE_FORMAT()), которое раскрывает функцию `strftime()`.
* [Issue #961](https://github.com/manticoresoftware/manticoresearch/issues/961) "Сортировка ведер через HTTP JSON API": введено необязательное [свойство sort](Searching/Faceted_search.md#HTTP-JSON) для каждого ведра агрегатов в HTTP-интерфейсе.
* [Issue #1062](https://github.com/manticoresoftware/manticoresearch/issues/1062) "Улучшение логирования ошибок JSON insert api - "unsupported value type"": Конечная точка `/bulk` сообщает информацию о количестве обработанных и необработанных строк (документов) в случае ошибки.
* [Issue #1070](https://github.com/manticoresoftware/manticoresearch/issues/1070) "CBO hints не поддерживают несколько атрибутов": Включена поддержка подсказок индекса для нескольких атрибутов.
* [Issue #1106](https://github.com/manticoresoftware/manticoresearch/issues/1106) "Добавить теги в http search query": Теги добавлены в [HTTP PQ ответы](Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md?client=JSON#Adding-rules-to-a-percolate-table).
* [Issue #1301](https://github.com/manticoresoftware/manticoresearch/issues/1301) "buddy не должен создавать таблицу параллельно": Исправлена проблема, вызывавшая сбои при параллельных операциях CREATE TABLE. Теперь может выполняться только одна операция `CREATE TABLE` одновременно.
* [Issue #1303](https://github.com/manticoresoftware/manticoresearch/issues/1303) "Добавить поддержку @ в имена столбцов".
* [Issue #1316](https://github.com/manticoresoftware/manticoresearch/issues/1316) "Запросы к набору данных такси медленные с ps=1": Логика CBO была уточнена, а разрешение гистограммы по умолчанию установлено на 8k для повышения точности по атрибутам со случайным распределением значений.
* [Issue #1317](https://github.com/manticoresoftware/manticoresearch/issues/1317) "Исправить CBO против полнотекстового поиска на hn dataset": Внедрена улучшенная логика определения, когда использовать пересечение итераторов битовой карты, а когда — очередь с приоритетом.
* [Issue #1318](https://github.com/manticoresoftware/manticoresearch/issues/1318) "columnar: изменить интерфейс итератора на однократный вызов": Итераторы columnar теперь используют один вызов `Get`, заменяя прежние двухшаговые вызовы `AdvanceTo` + `Get` для получения значения.
* [Issue #1319](https://github.com/manticoresoftware/manticoresearch/issues/1319) "Ускорение вычисления агрегатов (удалить CheckReplaceEntry?)": Вызов `CheckReplaceEntry` был убран из сортировщика групп для ускорения вычисления агрегатных функций.
* [Issue #1320](https://github.com/manticoresoftware/manticoresearch/issues/1320) "create table read_buffer_docs/hits не понимает синтаксис k/m/g": Опции `CREATE TABLE` `read_buffer_docs` и `read_buffer_hits` теперь поддерживают синтаксис k/m/g.
* Пакеты языков [Language packs](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) для английского, немецкого и русского теперь можно легко установить на Linux командой `apt/yum install manticore-language-packs`. На macOS используйте команду `brew install manticoresoftware/tap/manticore-language-packs`.
* Порядок полей и атрибутов теперь согласован между операциями `SHOW CREATE TABLE` и `DESC`.
* Если при выполнении запросов `INSERT` на диске недостаточно места, новые запросы `INSERT` будут завершаться с ошибкой до освобождения достаточного объёма дискового пространства.
* Добавлена функция приведения типа [UINT64()](Functions/Type_casting_functions.md#UINT64%28%29).
* Точка доступа `/bulk` теперь обрабатывает пустые строки как команду [commit](Data_creation_and_modification/Transactions.md#BEGIN,-COMMIT,-and-ROLLBACK). Подробнее [здесь](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md?client=JSON#Bulk-adding-documents).
* Реализованы предупреждения для [недопустимых подсказок индекса](Searching/Options.md#Query-optimizer-hints), обеспечивающие большую прозрачность и позволяющие избежать ошибок.
* При использовании `count(*)` с одним фильтром запросы теперь используют предварительно вычисленные данные из вторичных индексов, если они доступны, существенно ускоряя время выполнения запросов.

### ⚠️ Критические изменения
* ⚠️ Таблицы, созданные или изменённые в версии 6.2.0, не могут быть прочитаны более старыми версиями.
* ⚠️ Идентификаторы документов теперь обрабатываются как беззнаковые 64-битные целые числа при индексации и операциях INSERT.
* ⚠️ Синтаксис подсказок оптимизатора запросов обновлён. Новый формат — `/*+ SecondaryIndex(uid) */`. Старый синтаксис больше не поддерживается.
* ⚠️ [Проблема #1160](https://github.com/manticoresoftware/manticoresearch/issues/1160): использование символа `@` в названиях таблиц запрещено для предотвращения конфликтов синтаксиса.
* ⚠️ Строковые поля/атрибуты, помеченные как `indexed` и `attribute`, теперь рассматриваются как единое поле при операциях `INSERT`, `DESC` и `ALTER`.
* ⚠️ [Проблема #1057](https://github.com/manticoresoftware/manticoresearch/issues/1057): библиотеки MCL больше не загружаются на системах, не поддерживающих SSE 4.2.
* ⚠️ [Проблема #1143](https://github.com/manticoresoftware/manticoresearch/issues/1143): параметр [agent_query_timeout](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_query_timeout) был неисправен. Исправлен и теперь работает.

### Исправления ошибок
* [Коммит 2a6e](https://github.com/manticoresoftware/manticoresearch/commit/2a6ea8f7) "Сбой при DROP TABLE": решена проблема, вызывавшая долгие ожидания завершения операций записи (optimize, сохранение дискового чанка) в RT-таблице при выполнении команды DROP TABLE. Добавлено предупреждение при наличии остающихся файлов в директории таблицы после DROP TABLE.
* [Коммит 2ebd](https://github.com/manticoresoftware/manticoresearch/commit/2ebd424d): добавлена поддержка колоннарных атрибутов в код, используемый для группировки по нескольким атрибутам.
* [Коммит 3be4](https://github.com/manticoresoftware/manticoresearch/commit/3be4503f752216b37b8cde8e47fd2d77bbce63ae) Исправлен сбой, возможно вызванный нехваткой места на диске, за счёт корректной обработки ошибок записи в binlog.
* [Коммит 6adb](https://github.com/manticoresoftware/manticoresearch/commit/6adb0934): исправлен сбой, который иногда происходил при использовании нескольких колоннарных скан-итераторов (или итераторов вторичного индекса) в запросе.
* [Коммит 6bd9](https://github.com/manticoresoftware/manticoresearch/commit/6bd9f709): исправлена ошибка, при которой фильтры не удалялись при использовании сортировщиков с предварительно вычисленными данными.
* [Коммит 6d03](https://github.com/manticoresoftware/manticoresearch/commit/6d03566a): обновлён код CBO для более точной оценки запросов с фильтрами по посрочным атрибутам, выполняемых в нескольких потоках.
* [Коммит 6dd3](https://github.com/manticoresoftware/manticoresearch/commit/6dd3964), [Helm #56](https://github.com/manticoresoftware/manticoresearch-helm/issues/56) "фатальный сбой в кластере Kubernetes": исправлен дефектный фильтр Блума для корневого объекта JSON; исправлен сбой демона при фильтрации по полю JSON.
* [Коммит 6e1b](https://github.com/manticoresoftware/manticoresearch/commit/6e1b849) Исправлен крах демона, вызванный некорректным конфигом `manticore.json`.
* [Коммит 6fbc](https://github.com/manticoresoftware/manticoresearch/commit/6fbc4189) Исправлен json фильтр диапазона для поддержки значений int64.
* [Коммит 9c67](https://github.com/manticoresoftware/manticoresearch/commit/9c677426) `.sph` файлы могли повреждаться при `ALTER`. Исправлено.
* [Коммит 77cc](https://github.com/manticoresoftware/manticoresearch/commit/77ccd8f): добавлен общий ключ для репликации оператора replace для устранения ошибки `pre_commit`, возникающей при репликации replace с нескольких master-узлов.
* [Коммит 2884](https://github.com/manticoresoftware/manticoresearch/commit/2884519e) устранены проблемы с проверкой bigint для функций типа 'date_format()'.
* [Коммит 9513](https://github.com/manticoresoftware/manticoresearch/commit/9513aca5): итераторы больше не отображаются в [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META) при использовании сортировщиков с предварительно вычисленными данными.
* [Коммит a2a7](https://github.com/manticoresoftware/manticoresearch/commit/a2a70555): обновлен размер стека узла полнотекстового поиска для предотвращения сбоев при сложных полнотекстовых запросах.
* [Коммит a062](https://github.com/manticoresoftware/manticoresearch/commit/a0626d7e): устранен баг, вызывавший сбой при репликации обновлений с JSON и строковыми атрибутами.
* [Коммит b3e6](https://github.com/manticoresoftware/manticoresearch/commit/b3e63e5d): обновлен строителя строк для использования 64-битных целых чисел, чтобы избежать сбоев при обработке больших наборов данных.
* [Коммит c472](https://github.com/manticoresoftware/manticoresearch/commit/c472e5b): устранен сбой при использовании count distinct по нескольким индексам.
* [Коммит d073](https://github.com/manticoresoftware/manticoresearch/commit/d0730272): исправлена ошибка, при которой запросы по дисковым чанкам RT-индексов могли выполняться в несколько потоков, даже если `pseudo_sharding` отключён.
* [Коммит d205](https://github.com/manticoresoftware/manticoresearch/commit/d205508e) Множество значений, возвращаемых командой `show index status`, изменено и теперь зависит от типа используемого индекса.
* [Коммит e9bc](https://github.com/manticoresoftware/manticoresearch/commit/e9bc2f9) Исправлена ошибка HTTP при обработке массовых запросов и устранена проблема с не передачей ошибки клиенту из сетевого цикла.
* [Коммит f77c](https://github.com/manticoresoftware/manticoresearch/commit/f77ce0e6) использование расширенного стека для PQ.
* [Коммит fac2](https://github.com/manticoresoftware/manticoresearch/commit/fac2d9687a20582765aaa89be9597804c4f845af) Обновлен вывод ранжера экспорта для согласования с [packedfactors()](Functions/Searching_and_ranking_functions.md#PACKEDFACTORS%28%29).
* [Коммит ff87](https://github.com/manticoresoftware/manticoresearch/commit/ff87cc4): Исправлена проблема со списком строк в фильтре журнала запросов SphinxQL.
* [Issue #589](https://github.com/manticoresoftware/manticoresearch/issues/589) "Определение набора символов, похоже, зависит от упорядочения кодов": Исправлено некорректное отображение набора символов для дубликатов.
* [Issue #811](https://github.com/manticoresoftware/manticoresearch/issues/811) "Отображение нескольких слов в формах слов мешает поиску фраз с пунктуацией CJK между ключевыми словами": Исправлено положение токенов ngram в запросе фразы с формами слов.
* [Issue #834](https://github.com/manticoresoftware/manticoresearch/issues/834) "Знак равенства в поисковом запросе ломает запрос": Обеспечена возможность экранирования точного символа и исправлено двойное точное расширение опцией `expand_keywords`.
* [Issue #864](https://github.com/manticoresoftware/manticoresearch/issues/864) "конфликт exceptions/stopwords"
* [Issue #910](https://github.com/manticoresoftware/manticoresearch/issues/910) "Сбой Manticore при вызове call snippets() с libstemmer_fr и index_exact_words": Устранены внутренние конфликты, вызывавшие сбои при вызове `SNIPPETS()`.
* [Issue #946](https://github.com/manticoresoftware/manticoresearch/issues/946) "Дублирование записей при SELECT": Исправлена проблема с дублированием документов в результате для запроса с опцией `not_terms_only_allowed` к RT-индексу с удаленными документами.
* [Issue #967](https://github.com/manticoresoftware/manticoresearch/issues/967) "Использование аргументов JSON в функциях UDF приводит к сбою": Исправлен сбой демона при обработке поиска с включенным псевдо-шардингом и UDF с JSON-аргументом.
* [Issue #1050](https://github.com/manticoresoftware/manticoresearch/issues/1050) "count(*) в FEDERATED": Исправлен сбой демона при запросе через движок `FEDERATED` с агрегатами.
* [Issue #1052](https://github.com/manticoresoftware/manticoresearch/issues/1052) Исправлена проблема несовместимости колонки `rt_attr_json` с колоночным хранением.
* [Issue #1072](https://github.com/manticoresoftware/manticoresearch/issues/1072) "* удаляется из поискового запроса по ignore_chars": Исправлено так, что подстановочные знаки в запросе не затрагиваются `ignore_chars`.
* [Issue #1075](https://github.com/manticoresoftware/manticoresearch/issues/1075) "indextool --check не работает, если есть распределённая таблица": indextool теперь совместим с инстансами, имеющими индексы 'distributed' и 'template' в json-конфиге.
* [Issue #1081](https://github.com/manticoresoftware/manticoresearch/issues/1081) "Определённый select на определённом RT датасете приводит к сбою searchd": Исправлен сбой демона при запросе с packedfactors и большим внутренним буфером.
* [Issue #1095](https://github.com/manticoresoftware/manticoresearch/issues/1095) "С удалёнными документами с not_terms_only_allowed они игнорируются"
* [Issue #1099](https://github.com/manticoresoftware/manticoresearch/issues/1099) "indextool --dumpdocids не работает": Восстановлена функциональность команды `--dumpdocids`.
* [Issue #1100](https://github.com/manticoresoftware/manticoresearch/issues/1100) "indextool --buildidf не работает": indextool теперь закрывает файл после завершения globalidf.
* [Issue #1104](https://github.com/manticoresoftware/manticoresearch/issues/1104) "Count(*) пытается обрабатываться как schema set в удалённых таблицах": Исправлена проблема с отправкой демоном сообщения об ошибке для запросов в распределённый индекс при пустом наборе результатов агента.
* [Issue #1109](https://github.com/manticoresoftware/manticoresearch/issues/1109) "FLUSH ATTRIBUTES зависает с threads=1".
* [Issue #1126](https://github.com/manticoresoftware/manticoresearch/issues/1126) "Потеряно соединение с MySQL сервером во время запроса - manticore 6.0.5": Исправлены сбои при использовании нескольких фильтров по колонковым атрибутам.
* [Issue #1135](https://github.com/manticoresoftware/manticoresearch/issues/1135) "Регистрозависимость фильтрации JSON строк": Исправлена сортировка для правильной работы фильтров, используемых в HTTP поисковых запросах.
* [Issue #1140](https://github.com/manticoresoftware/manticoresearch/issues/1140) "Матч в неправильном поле": Исправлен ущерб, связанный с `morphology_skip_fields`.
* [Issue #1155](https://github.com/manticoresoftware/manticoresearch/issues/1155) "Системные удалённые команды через API должны использовать g_iMaxPacketSize": Внесены изменения для обхода проверки `max_packet_size` для команд репликации между узлами. Кроме того, последняя ошибка кластера добавлена в отображение состояния.
* [Issue #1302](https://github.com/manticoresoftware/manticoresearch/issues/1302) "Оставшиеся временные файлы после неудачного optimize": Исправлена проблема с оставлением временных файлов после ошибки во время слияния или оптимизации.
* [Issue #1304](https://github.com/manticoresoftware/manticoresearch/issues/1304) "Добавлена переменная окружения для таймаута запуска buddy": Добавлена переменная окружения `MANTICORE_BUDDY_TIMEOUT` (по умолчанию 3 секунды) для контроля времени ожидания сообщения buddy при запуске демона.
* [Issue #1305](https://github.com/manticoresoftware/manticoresearch/issues/1305) "Переполнение int при сохранении PQ meta": Смягчен чрезмерный расход памяти демоном при сохранении большого PQ индекса.
* [Issue #1306](https://github.com/manticoresoftware/manticoresearch/issues/1306) "Невозможно воссоздать RT таблицу после изменения внешнего файла": Исправлена ошибка alter с пустой строкой для внешних файлов; исправлены внешние файлы RT индекса, остававшиеся после изменения внешних файлов.
* [Issue #1307](https://github.com/manticoresoftware/manticoresearch/issues/1307) "Запрос SELECT sum(value) as value работает неправильно": Исправлена проблема, когда выражение списка select с псевдонимом могло скрывать атрибут индекса; также исправлено преобразование sum в count для int64 для целочисленных значений.
* [Issue #1308](https://github.com/manticoresoftware/manticoresearch/issues/1308) "Avoid binding to localhost in replication": Обеспечена невозможность привязки репликации к localhost для имен хостов с несколькими IP-адресами.
* [Issue #1309](https://github.com/manticoresoftware/manticoresearch/issues/1309) "reply to mysql client failed for data larger 16Mb": Исправлена ошибка возврата пакета SphinxQL клиенту размером более 16 Мб.
* [Issue #1310](https://github.com/manticoresoftware/manticoresearch/issues/1310) "wrong reference in "paths to external files should be absolute": Исправлено отображение полного пути к внешним файлам в `SHOW CREATE TABLE`.
* [Issue #1311](https://github.com/manticoresoftware/manticoresearch/issues/1311) "debug build crashes on long strings in snippets": Теперь в тексте, обрабатываемом функцией `SNIPPET()`, разрешены длинные строки (>255 символов).
* [Issue #1312](https://github.com/manticoresoftware/manticoresearch/issues/1312) "spurious crash on use-after-delete in kqueue polling (master-agent)": Исправлены сбои при невозможности подключения мастера к агенту на системах с поддержкой kqueue (FreeBSD, MacOS и др.).
* [Issue #1313](https://github.com/manticoresoftware/manticoresearch/issues/1313) "too long connect to itself": При подключении мастера к агентам на MacOS/BSD используется объединённый таймаут connect+query вместо только connect.
* [Issue #1314](https://github.com/manticoresoftware/manticoresearch/issues/1314) "pq (json meta) with unreached embedded synonyms fails to load": Исправлен флаг встроенных синонимов в pq.
* [Issue #1315](https://github.com/manticoresoftware/manticoresearch/issues/1315) "Allow some functions (sint, fibonacci, second, minute, hour, day, month, year, yearmonth, yearmonthday) to use implicitly promoted argument values".
* [Issue #1321](https://github.com/manticoresoftware/manticoresearch/issues/1321) "Enable multithreaded SI in fullscan, but limit threads": В CBO реализован код для лучшего прогнозирования производительности многопоточных вторичных индексов при использовании в полнотекстовом запросе.
* [Issue #1322](https://github.com/manticoresoftware/manticoresearch/issues/1322) "count(*) queries still slow after using precalc sorters": Итераторы теперь не инициализируются при использовании сортировщиков на основе предрассчитанных данных, что устраняет негативное влияние на производительность.
* [Issue #1411](https://github.com/manticoresoftware/manticoresearch/issues/1141) "query log in sphinxql does not preserve original queries for MVA's": Теперь логируются `all()/any()`.

## Version 6.0.4
Released: March 15 2023

### New features
* Улучшена интеграция с Logstash, Beats и др., включая:
  - Поддержку версий Logstash 7.6 - 7.15, Filebeat 7.7 - 7.12
  - Поддержку авто-схем.
  - Добавлена обработка bulk-запросов в формате, похожем на Elasticsearch.
* [Buddy commit ce90](https://github.com/manticoresoftware/manticoresearch-buddy/commit/ce907ea) Логирование версии Buddy при запуске Manticore.

### Bugfixes
* [Issue #588](https://github.com/manticoresoftware/manticoresearch/issues/588), [Issue #942](https://github.com/manticoresoftware/manticoresearch/issues/942) Исправлен неправильный символ в метаинформации поиска и вызов ключевых слов для bigram индекса.
* [Issue #1027](https://github.com/manticoresoftware/manticoresearch/issues/1027) Отклонение HTTP заголовков с маленькими буквами.
* ❗[Issue #1039](https://github.com/manticoresoftware/manticoresearch/issues/1039) Исправлена утечка памяти в демоне при чтении вывода консоли Buddy.
* [Issue #1056](https://github.com/manticoresoftware/manticoresearch/issues/1056) Исправлено неожиданное поведение знака вопроса.
* [Issue #1064](https://github.com/manticoresoftware/manticoresearch/issues/1064) - Исправлено состояние гонки в таблицах преобразования к нижнему регистру токенизатора, вызывавшее сбой.
* [Commit 59bb](https://github.com/manticoresoftware/manticoresearch/commit/59bb54c) Исправлена обработка bulk-записей в JSON интерфейсе для документов с id явно равным null.
* [Commit 7b6b](https://github.com/manticoresoftware/manticoresearch/commit/7b6b25f) Исправлена статистика терминов в CALL KEYWORDS для множества одинаковых терминов.
* [Commit f381](https://github.com/manticoresoftware/manticoresearch/commit/f381ad2) Файл конфигурации по умолчанию теперь создаётся установщиком Windows; пути больше не заменяются во время выполнения.
* [Commit 6940](https://github.com/manticoresoftware/manticoresearch/commit/6940e95), [Commit cc5a](https://github.com/manticoresoftware/manticoresearch/commit/cc5a480) Исправлены проблемы репликации для кластера с узлами в нескольких сетях.
* [Commit 4972](https://github.com/manticoresoftware/manticoresearch/commit/49722ab) Исправлена точка доступа `/pq` HTTP, теперь является псевдонимом для `/json/pq`.
* [Commit 3b53](https://github.com/manticoresoftware/manticoresearch/commit/3b5385a) Исправлен сбой демона при перезапуске Buddy.
* [Buddy commit fba9](https://github.com/manticoresoftware/manticoresearch-buddy/commit/fba9c8c) Отображение оригинальной ошибки при получении неверного запроса.
* [Buddy commit db95](https://github.com/manticoresoftware/manticoresearch-buddy/commit/db9532c) Разрешены пробелы в пути для бэкапов и добавлена поддержка одиночных кавычек с помощью регэкспа.

## Version 6.0.2
Released: Feb 10 2023

### Bugfixes
* [Issue #1024 crash 2](https://github.com/manticoresoftware/manticoresearch/issues/1024) Сбой / ошибка сегментации при поиске Facet с большим числом результатов
* ❗[Issue #1029](https://github.com/manticoresoftware/manticoresearch/issues/1029) - ВНИМАНИЕ: Встроенное значение KNOWN_CREATE_SIZE (16) меньше измеренного (208). Рекомендуется исправить значение!
* ❗[Issue #1032](https://github.com/manticoresoftware/manticoresearch/issues/1032) - Краш plain индекса в Manticore 6.0.0
* ❗[Issue #1033](https://github.com/manticoresoftware/manticoresearch/issues/1033) - Потеря нескольких distributed при перезапуске демона
* ❗[Issue #1064](https://github.com/manticoresoftware/manticoresearch/issues/1064) - состояние гонки в таблицах токенизатора с преобразованием к нижнему регистру

## Version 6.0.0
Released: Feb 7 2023

Начиная с этого релиза, Manticore Search поставляется с Manticore Buddy, дополнительным демоном на PHP, который обрабатывает функциональность высокого уровня, не требующую очень низкой задержки или высокой пропускной способности. Manticore Buddy работает "за кулисами", и вы можете даже не заметить, что он запущен. Хотя он невидим для конечного пользователя, создание Manticore Buddy, который легко устанавливается и совместим с основным демоном на C++, стало значительным вызовом. Это крупное изменение позволит команде разрабатывать широкий спектр новых функций высокого уровня, таких как оркестрация шард, контроль доступа и аутентификация, а также различные интеграции, например mysqldump, DBeaver, Grafana mysql connector. На данный момент он уже обрабатывает [SHOW QUERIES](Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES), [BACKUP](Securing_and_compacting_a_table/Backup_and_restore.md#BACKUP-SQL-command-reference) и [Auto schema](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema).

Этот релиз также включает более 130 исправлений ошибок и множество новых функций, многие из которых можно считать крупными.

### Крупные изменения
* 🔬 Экспериментально: теперь вы можете выполнять Elasticsearch-совместимые [insert](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md?example=Elasticsearch#Adding-documents-to-a-real-time-table) и [replace](Data_creation_and_modification/Updating_documents/REPLACE.md?example=Elasticsearch#REPLACE) JSON-запросы, что позволяет использовать Manticore с такими инструментами, как Logstash (версия < 7.13), Filebeat и другими инструментами из семейства Beats. Включено по умолчанию. Вы можете отключить это с помощью `SET GLOBAL ES_COMPAT=off`.
* Поддержка [Manticore Columnar Library 2.0.0](https://github.com/manticoresoftware/columnar/) с многочисленными исправлениями и улучшениями в [вторичных индексах](Server_settings/Searchd.md#secondary_indexes). **⚠️ ЛОМАНИЕ СОВМЕСТИМОСТИ**: Вторичные индексы включены по умолчанию в этом релизе. Убедитесь, что вы выполняете [ALTER TABLE table_name REBUILD SECONDARY](Updating_table_schema_and_settings.md#Rebuilding-a-secondary-index), если обновляетесь с Manticore 5. Подробнее см. ниже.
* [Коммит c436](https://github.com/manticoresoftware/manticoresearch/commit/c436f9023536f767610451911955ae36d90aa638) Auto-schema: теперь можно пропустить создание таблицы, просто вставьте первый документ, и Manticore автоматически создаст таблицу на основе его полей. Подробнее об этом читайте [здесь](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema). Вы можете включать/выключать его с помощью [searchd.auto_schema](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema).
* Значительное обновление [оптимизатора на основе стоимости](Searching/Cost_based_optimizer.md), что уменьшает время отклика запросов во многих случаях.
  - [Issue #1008](https://github.com/manticoresoftware/manticoresearch/issues/1008) Оценка производительности параллелизации в CBO.
  - [Issue #1014](https://github.com/manticoresoftware/manticoresearch/issues/1014) CBO теперь учитывает [вторичные индексы](Server_settings/Searchd.md#secondary_indexes) и может действовать умнее.
  - [Коммит cef9](https://github.com/manticoresoftware/columnar/commit/cef9b54dc3a4ee9138ebc4df3a671715b996d6b3) Статистика кодировки колоннарных таблиц/полей теперь хранится в метаданных, что помогает CBO принимать более умные решения.
  - [Коммит 2b95](https://github.com/manticoresoftware/manticoresearch/commit/2b95fb8c2ba422c9c5a66e2b61c1f7922f31c7aa) Добавлены подсказки CBO для тонкой настройки его поведения.
* [Телеметрия](Telemetry.md#Telemetry): рады сообщить о добавлении телеметрии в этот релиз. Эта функция позволяет собирать анонимные и деперсонализированные метрики, которые помогут улучшить производительность и удобство использования продукта. Будьте уверены, все данные **полностью анонимны и не будут связаны с какой-либо персональной информацией**. Эта функция может быть [легко отключена](Telemetry.md#Telemetry) в настройках при желании.
* [Коммит 5aaf](https://github.com/manticoresoftware/manticoresearch/commit/5aafb17d81b601e399e212a344a4738f65576cdd) [ALTER TABLE table_name REBUILD SECONDARY](Updating_table_schema_and_settings.md#Rebuilding-a-secondary-index), чтобы перекомпилировать вторичные индексы в любое удобное время, например:
  - при миграции с Manticore 5 на новую версию,
  - когда вы выполняли [UPDATE](Data_creation_and_modification/Updating_documents/UPDATE.md#UPDATE) (т.е. [обновление на месте, не замена](Data_creation_and_modification/Updating_documents/REPLACE_vs_UPDATE.md)) атрибута в индексе.
* [Issue #821](https://github.com/manticoresoftware/manticoresearch/issues/821) Новый инструмент `manticore-backup` для [резервного копирования и восстановления экземпляра Manticore](Securing_and_compacting_a_table/Backup_and_restore.md).
* SQL-команда [BACKUP](Securing_and_compacting_a_table/Backup_and_restore.md#BACKUP-SQL-command-reference) для создания резервных копий изнутри Manticore.
* SQL-команда [SHOW QUERIES](Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES) для простого просмотра выполняющихся запросов вместо потоков.
* [Issue #551](https://github.com/manticoresoftware/manticoresearch/issues/551) SQL-команда `KILL` для остановки длительно выполняющегося `SELECT`.
* Динамический `max_matches` для агрегатных запросов для повышения точности и уменьшения времени отклика.

### Небольшие изменения
* [Issue #822](https://github.com/manticoresoftware/manticoresearch/issues/822) SQL-команды [FREEZE/UNFREEZE](Securing_and_compacting_a_table/Freezing_a_table.md) для подготовки таблицы реального времени/обычной таблицы к резервному копированию.
* [Коммит c470](https://github.com/manticoresoftware/manticoresearch/commit/c47052f18ad4868134de1a97e8d7da570e531dfa) Новые настройки `accurate_aggregation` и `max_matches_increase_threshold` для контролируемой точности агрегации.
* [Issue #718](https://github.com/manticoresoftware/manticoresearch/issues/718) Поддержка подписанных отрицательных 64-битных ID. Обратите внимание, вы по-прежнему не можете использовать ID > 2^63, но теперь можно использовать ID в диапазоне от -2^63 до 0.
* Так как мы недавно добавили поддержку вторичных индексов, возникла путаница, поскольку "индекс" мог означать вторичный индекс, полнотекстовый индекс или простой/реального времени `index`. Чтобы уменьшить путаницу, мы переименовываем последний в "таблица". Следующие SQL/командные строки изменены вследствие этого. Их старые версии устарели, но все еще работают:
  - `index <table name>` => `table <table name>`,
  - `searchd -i / --index` => `searchd -t / --table`,
  - `SHOW INDEX STATUS` => `SHOW TABLE STATUS`,
  - `SHOW INDEX SETTINGS` => `SHOW TABLE SETTINGS`,
  - `FLUSH RTINDEX` => `FLUSH TABLE`,
  - `OPTIMIZE INDEX` => `OPTIMIZE TABLE`,
  - `ATTACH TABLE plain TO RTINDEX rt` => `ATTACH TABLE plain TO TABLE rt`,
  - `RELOAD INDEX` => `RELOAD TABLE`,
  - `RELOAD INDEXES` => `RELOAD TABLES`.

  Мы не планируем делать старые формы устаревшими, но для совместимости с документацией рекомендуем изменить имена в вашем приложении. В будущем релизе будет изменено в выводе различных SQL и JSON команд: переименование "index" в "table".
* Запросы с состоянием UDF теперь принудительно выполняются в одном потоке.
* [Issue #1011](https://github.com/manticoresoftware/manticoresearch/issues/1011) Рефакторинг всего, что связано с планированием времени, как предварительное условие для параллельного слияния чанков.
* **⚠️ ИЗМЕНЕНИЕ С ЛОМКОЙ СОВМЕСТИМОСТИ**: Формат колоночного хранилища изменен. Вам необходимо перестроить те таблицы, которые имеют колоночные атрибуты.
* **⚠️ ИЗМЕНЕНИЕ С ЛОМКОЙ СОВМЕСТИМОСТИ**: Формат файлов вторичных индексов изменен, поэтому если вы используете вторичные индексы для поиска и у вас в конфигурационном файле стоит `searchd.secondary_indexes = 1`, имейте в виду, что новая версия Manticore **пропустит загрузку таблиц с вторичными индексами**. Рекомендуется:
  - Перед обновлением измените `searchd.secondary_indexes` на 0 в конфигурационном файле.
  - Запустите инстанс. Manticore загрузит таблицы с предупреждением.
  - Выполните `ALTER TABLE <table name> REBUILD SECONDARY` для каждого индекса чтобы перестроить вторичные индексы.

  Если у вас кластер с репликацией, вам нужно будет выполнить `ALTER TABLE <table name> REBUILD SECONDARY` на всех узлах или следовать [инструкции](Securing_and_compacting_a_table/Compacting_a_table.md#Optimizing-clustered-tables) с изменением: вместо `OPTIMIZE` запускать `ALTER .. REBUILD SECONDARY`.
* **⚠️ ИЗМЕНЕНИЕ С ЛОМКОЙ СОВМЕСТИМОСТИ**: Версия binlog обновлена, поэтому любые binlog файлы предыдущих версий не будут воспроизведены. Важно, чтобы Manticore Search корректно завершал работу во время обновления. Это значит, что после остановки предыдущего инстанса не должно быть binlog файлов в `/var/lib/manticore/binlog/` кроме `binlog.meta`.
* [Issue #849](https://github.com/manticoresoftware/manticoresearch/issues/849) `SHOW SETTINGS`: теперь можно видеть настройки из конфигурационного файла внутри Manticore.
* [Issue #1007](https://github.com/manticoresoftware/manticoresearch/issues/1007) [SET GLOBAL CPUSTATS=1/0](Server_settings/Setting_variables_online.md#SET) включает/выключает отслеживание CPU времени; [SHOW THREADS](Node_info_and_management/SHOW_THREADS.md) теперь не показывает статистику CPU, когда отслеживание отключено.
* [Issue #1009](https://github.com/manticoresoftware/manticoresearch/issues/1009) Сегменты RAM чанка таблицы RT теперь могут сливаться во время сброса RAM чанка.
* [Issue #1012](https://github.com/manticoresoftware/manticoresearch/issues/1012) Добавлен прогресс вторичных индексов в вывод [indexer](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool).
* [Issue #1013](https://github.com/manticoresoftware/manticoresearch/issues/1013) Ранее запись таблицы могла быть удалена Manticore из списка индексов, если не удавалось начать её обслуживание при старте. Новое поведение — сохранять её в списке и пытаться загрузить при следующем запуске.
* [indextool --docextract](Miscellaneous_tools.md#indextool) возвращает все слова и хиты, относящиеся к запрошенному документу.
* [Commit 2b29](https://github.com/manticoresoftware/manticoresearch/commit/2b296ee20e520b85bcbb4383e87095d31e7165dc) Переменная окружения `dump_corrupt_meta` позволяет выгружать повреждённые метаданные таблицы в лог, если searchd не может загрузить индекс.
* [Commit c7a3](https://github.com/manticoresoftware/manticoresearch/commit/c7a3136eefe0eb2f69e3d99fae9c10beb5821ea6) `DEBUG META` теперь может показывать `max_matches` и псевдо-статистику шардинга.
* [Commit 6bca](https://github.com/manticoresoftware/manticoresearch/commit/6bca4848dc5c8396dbc1e057be4b9b2d579794e8) Более информативная ошибка вместо запутывающего "Index header format is not json, will try it as binary...".
* [Commit bef3](https://github.com/manticoresoftware/lemmatizer-uk/commit/bef3ff0386d3ee87ec57619782100972c1122e47) Путь к украинскому лемматизатору изменён.
* [Commit 4ae7](https://github.com/manticoresoftware/manticoresearch/commit/4ae789595329a2951e194d1191ddb3121459a560) Статистика вторичных индексов добавлена в [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META).
* [Commit 2e7c](https://github.com/manticoresoftware/manticoresearch/commit/2e7c585e) JSON-интерфейс теперь легко визуализировать с помощью Swagger Editor https://manual.manticoresearch.com/Openapi#OpenAPI-specification.
* **⚠️ ИЗМЕНЕНИЕ С ЛОМКОЙ СОВМЕСТИМОСТИ**: Протокол репликации изменён. Если вы используете кластер с репликацией, при обновлении до Manticore 5 нужно:
  - сначала корректно остановить все узлы
  - затем запустить узел, который был остановлен последним, с флагом `--new-cluster` (или запустить инструмент `manticore_new_cluster` в Linux).
  - прочитать подробнее о [перезапуске кластера](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster).

### Изменения, связанные с Manticore Columnar Library
* Рефакторинг интеграции вторичных индексов с колоночным хранилищем.
* [Commit efe2](https://github.com/manticoresoftware/columnar/commit/efe26b2a) Оптимизация Manticore Columnar Library, которая может сократить время отклика за счет частичной предварительной оценки min/max.
* [Commit 2757](https://github.com/manticoresoftware/columnar/commit/2757b99b) Если слияние дискового чанка прерывается, демон теперь очищает временные файлы, связанные с MCL.
* [Commit e9c6](https://github.com/manticoresoftware/manticoresearch/commit/e9c672014b63e2b49e7a5a53880583af86c3ab4e) Версии библиотек Columnar и secondary выводятся в лог при сбое.
* [Commit f5e8](https://github.com/manticoresoftware/columnar/commit/f5e84eeb8b6cb102f13e3468246702427527fef9) Добавлена поддержка быстрого перематывания doclist для вторичных индексов.
* [Commit 06df](https://github.com/manticoresoftware/manticoresearch/commit/06dfdd2e668933495dd6cb52d5ba2e0aa9b26e58) Запросы типа `select attr, count(*) from plain_index` (без фильтрации) теперь выполняются быстрее при использовании MCL.
* [Commit 0a76](https://github.com/manticoresoftware/manticoresearch/commit/0a76ecb9adbe886868b962dd425de5f475bbf55e) @@autocommit в HandleMysqlSelectSysvar для совместимости с .net коннектором для mysql версии выше 8.25
* **⚠️ BREAKING CHANGE**: [MCL Issue #17](https://github.com/manticoresoftware/columnar/issues/17) MCL: добавлен SSE код для columnar сканирования. Теперь MCL требует минимум SSE4.2.

### Изменения, связанные с упаковкой
* [Commit 4d19](https://github.com/manticoresoftware/manticoresearch/commit/4d19f5cbe49a31228aa09253f061165bfe80e51b) **⚠️ BREAKING CHANGE**: Поддержка Debian Stretch и Ubuntu Xenial прекращена.
* Поддержка RHEL 9, включая Centos 9, Alma Linux 9 и Oracle Linux 9.
* [Issue #924](https://github.com/manticoresoftware/manticoresearch/issues/924) Поддержка Debian Bookworm.
* [Issue #636](https://github.com/manticoresoftware/manticoresearch/issues/636) Упаковка: сборки arm64 для Linux и MacOS.
* [PR #26](https://github.com/manticoresoftware/docker/pull/26) Мультиархитектурный (x86_64 / arm64) docker образ.
* [Упрощена сборка пакетов для участников](Installation/Compiling_from_sources.md#Building-using-CI-Docker).
* Теперь возможно установить конкретную версию через APT.
* [Commit a6b8](https://github.com/manticoresoftware/manticoresearch/commit/51fddca5c2a3ebb8576fae4c18660656ba22de0f) Установщик для Windows (ранее предоставлялся только архив).
* Переход на компиляцию с использованием CLang 15.
* **⚠️ BREAKING CHANGE**: Пользовательские формулы Homebrew, включая формулу для Manticore Columnar Library. Для установки Manticore, MCL и всех необходимых компонентов используйте команду `brew install manticoresoftware/manticore/manticoresearch manticoresoftware/manticore/manticore-extra`.

### Исправления ошибок
* [Issue #479](https://github.com/manticoresoftware/manticoresearch/issues/479) Поле с именем `text`
* [Issue #501](https://github.com/manticoresoftware/manticoresearch/issues/501) id не может быть не bigint
* [Issue #646](https://github.com/manticoresoftware/manticoresearch/issues/646) ALTER против поля с именем "text"
* ❗[Issue #652](https://github.com/manticoresoftware/manticoresearch/issues/652) Возможная ОШИБКА: HTTP (JSON) offset и limit влияют на результаты фасета
* ❗[Issue #827](https://github.com/manticoresoftware/manticoresearch/issues/827) Searchd зависает/сбои при интенсивной нагрузке
* ❗[Issue #996](https://github.com/manticoresoftware/manticoresearch/issues/996) Индекс PQ выходит из памяти
* ❗[Commit 1041](https://github.com/manticoresoftware/manticoresearch/commit/10416ef7dddf06c0d759e32ccd6ebaa2468f7cbf) `binlog_flush = 1` был сломан с времен Sphinx. Исправлено.
* [MCL Issue #14](https://github.com/manticoresoftware/columnar/issues/14) MCL: сбой при select с слишком большим количеством ft полей
* [Issue #470](https://github.com/manticoresoftware/manticoresearch/issues/470) sql_joined_field нельзя сохранить
* [Issue #713](https://github.com/manticoresoftware/manticoresearch/issues/713) Сбой при использовании LEVENSHTEIN()
* [Issue #743](https://github.com/manticoresoftware/manticoresearch/issues/743) Непредвиденные сбои Manticore и невозможность нормального перезапуска
* [Issue #788](https://github.com/manticoresoftware/manticoresearch/issues/788) Вызов KEYWORDS через /sql возвращает управляющий символ, который ломает json
* [Issue #789](https://github.com/manticoresoftware/manticoresearch/issues/789) mariadb не может создать таблицу FEDERATED
* [Issue #796](https://github.com/manticoresoftware/manticoresearch/issues/796) WARNING: dlopen() failed: /usr/bin/lib_manticore_columnar.so: невозможно открыть разделяемый объект: Нет такого файла или каталога
* [Issue #797](https://github.com/manticoresoftware/manticoresearch/issues/797) Manticore сбой при поиске с ZONESPAN через api
* [Issue #799](https://github.com/manticoresoftware/manticoresearch/issues/799) Неправильный вес при использовании нескольких индексов и distinct в фасете
* [Issue #801](https://github.com/manticoresoftware/manticoresearch/issues/801) Запрос SphinxQL group зависает после перепроцессинга SQL индекса
* [Issue #802](https://github.com/manticoresoftware/manticoresearch/issues/802) MCL: Индексатор падает в 5.0.2 и manticore-columnar-lib 1.15.4
* [Issue #813](https://github.com/manticoresoftware/manticoresearch/issues/813) Manticore 5.0.2 FEDERATED возвращает пустой набор (MySQL 8.0.28)
* [Issue #824](https://github.com/manticoresoftware/manticoresearch/issues/824) select COUNT DISTINCT по 2 индексам при результате 0 вызывает внутреннюю ошибку
* [Issue #826](https://github.com/manticoresoftware/manticoresearch/issues/826) СБРОС при запросе DELETE
* [Issue #843](https://github.com/manticoresoftware/manticoresearch/issues/843) MCL: ошибка с длинным текстовым полем
* [Issue #856](https://github.com/manticoresoftware/manticoresearch/issues/856) 5.0.2 rtindex: Поведение ограничения агрегированного поиска не соответствует ожиданиям
* [Issue #863](https://github.com/manticoresoftware/manticoresearch/issues/863) Hits возвращается как объект Nonetype даже для поиска с несколькими результатами
* [Issue #870](https://github.com/manticoresoftware/manticoresearch/issues/870) Сбой при использовании Attribute и Stored Field в выражении SELECT
* [Issue #872](https://github.com/manticoresoftware/manticoresearch/issues/872) таблица становится невидимой после сбоя
* [Issue #877](https://github.com/manticoresoftware/manticoresearch/issues/877) Два отрицательных термина в поисковом запросе вызывают ошибку: запрос невычислим
* [Issue #878](https://github.com/manticoresoftware/manticoresearch/issues/878) a -b -c не работает через json query_string
* [Issue #886](https://github.com/manticoresoftware/manticoresearch/issues/886) pseudo_sharding с запросом match
* [Issue #893](https://github.com/manticoresoftware/manticoresearch/issues/893) Manticore 5.0.2 функции min/max работают не так, как ожидалось...
* [Issue #896](https://github.com/manticoresoftware/manticoresearch/issues/896) Поле "weight" неправильно парсится
* [Issue #897](https://github.com/manticoresoftware/manticoresearch/issues/897) Сервис Manticore падает при запуске и постоянно перезапускается
* [Issue #900](https://github.com/manticoresoftware/manticoresearch/issues/900) группировка по j.a, smth работает неправильно
* [Issue #913](https://github.com/manticoresoftware/manticoresearch/issues/913) Searchd падает при использовании expr в ранжировщике, но только для запросов с двумя proximity
* [Issue #916](https://github.com/manticoresoftware/manticoresearch/issues/916) net_throttle_action не работает
* [Issue #919](https://github.com/manticoresoftware/manticoresearch/issues/919) MCL: Manticore падает при выполнении запроса и другие сбои во время восстановления кластера.
* [Issue #925](https://github.com/manticoresoftware/manticoresearch/issues/925) SHOW CREATE TABLE выводит без обратных кавычек
* [Issue #930](https://github.com/manticoresoftware/manticoresearch/issues/930) Теперь можно выполнять запросы к Manticore из Java через JDBC коннектор
* [Issue #933](https://github.com/manticoresoftware/manticoresearch/issues/933) Проблемы с ранжированием bm25f
* [Issue #934](https://github.com/manticoresoftware/manticoresearch/issues/934) configless индексы зависают в watchdog в состоянии first-load
* [Issue #937](https://github.com/manticoresoftware/manticoresearch/issues/937) Segfault при сортировке данных фасетов
* [Issue #940](https://github.com/manticoresoftware/manticoresearch/issues/940) сбой при CONCAT(TO_STRING)
* [Issue #947](https://github.com/manticoresoftware/manticoresearch/issues/947) В некоторых случаях один простой select мог вызвать зависание всего экземпляра, так что нельзя было войти или выполнить любой другой запрос до завершения текущего select.
* [Issue #948](https://github.com/manticoresoftware/manticoresearch/issues/948) Сбой индексатора
* [Issue #950](https://github.com/manticoresoftware/manticoresearch/issues/950) неправильный подсчет из facet distinct
* [Issue #953](https://github.com/manticoresoftware/manticoresearch/issues/953) LCS неправильно вычисляется в встроенном ранжировщике sph04
* [Issue #955](https://github.com/manticoresoftware/manticoresearch/issues/955) 5.0.3 dev падает
* [Issue #963](https://github.com/manticoresoftware/manticoresearch/issues/963) FACET с json на columnar engine вызывает сбой
* [Issue #982](https://github.com/manticoresoftware/manticoresearch/issues/982) MCL: 5.0.3 сбой с вторичным индексом
* [PR #984](https://github.com/manticoresoftware/manticoresearch/pull/984) @@autocommit в HandleMysqlSelectSysvar
* [PR #985](https://github.com/manticoresoftware/manticoresearch/pull/985) Исправлена распределение thread-chunk в RT индексах
* [Issue #985](https://github.com/manticoresoftware/manticoresearch/issues/985) Исправлена распределение thread-chunk в RT индексах
* [Issue #986](https://github.com/manticoresoftware/manticoresearch/issues/986) неправильное значение max_query_time по умолчанию
* [Issue #987](https://github.com/manticoresoftware/manticoresearch/issues/987) Сбой при использовании регулярного выражения в многопоточном выполнении
* [Issue #988](https://github.com/manticoresoftware/manticoresearch/issues/988) Нарушена совместимость с обратным индексом
* [Issue #989](https://github.com/manticoresoftware/manticoresearch/issues/989) indextool сообщает об ошибке при проверке атрибутов columnar
* [Issue #990](https://github.com/manticoresoftware/manticoresearch/issues/990) утечка памяти json grouper clones
* [Issue #991](https://github.com/manticoresoftware/manticoresearch/issues/991) Утечка памяти при клонировании функции levenshtein
* [Issue #992](https://github.com/manticoresoftware/manticoresearch/issues/992) Сообщение об ошибке теряется при загрузке meta
* [Issue #993](https://github.com/manticoresoftware/manticoresearch/issues/993) Пропагирование ошибок из динамических индексов/subkeys и sysvars
* [Issue #994](https://github.com/manticoresoftware/manticoresearch/issues/994) Сбой при count distinct по строковому полю columnar в columnar хранилище
* [Issue #995](https://github.com/manticoresoftware/manticoresearch/issues/995) MCL: min(pickup_datetime) из taxi1 вызывает сбой
* [Issue #997](https://github.com/manticoresoftware/manticoresearch/issues/997) пустой excludes JSON запрос удаляет колонки из списка select
* [Issue #998](https://github.com/manticoresoftware/manticoresearch/issues/998) Вторичные задачи, выполняемые в текущем планировщике, иногда вызывают аномальные побочные эффекты
* [Issue #999](https://github.com/manticoresoftware/manticoresearch/issues/999) сбой с facet distinct и разными схемами
* [Issue #1000](https://github.com/manticoresoftware/manticoresearch/issues/1000) MCL: Columnar rt индекс поврежден после запуска без библиотеки columnar
* [Issue #1001](https://github.com/manticoresoftware/manticoresearch/issues/1001) неявный cutoff не работает в json
* [Issue #1002](https://github.com/manticoresoftware/manticoresearch/issues/1002) Проблема с Columnar grouper
* [Issue #1003](https://github.com/manticoresoftware/manticoresearch/issues/1003) Невозможно удалить последнее поле из индекса
* [Issue #1004](https://github.com/manticoresoftware/manticoresearch/issues/1004) неправильное поведение после --new-cluster
* [Issue #1005](https://github.com/manticoresoftware/manticoresearch/issues/1005) "columnar library not loaded", но она не требуется
* [Issue #1006](https://github.com/manticoresoftware/manticoresearch/issues/1006) отсутствие ошибки для delete запроса
* [Issue #1010](https://github.com/manticoresoftware/manticoresearch/issues/1010) Исправлено расположение файла ICU data в сборках для Windows
* [PR #1018](https://github.com/manticoresoftware/manticoresearch/pull/1018) Проблема отправки рукопожатия
* [Issue #1020](https://github.com/manticoresoftware/manticoresearch/issues/1020) Отображение id в show create table
* [Issue #1024 crash 1](https://github.com/manticoresoftware/manticoresearch/issues/1024) Сбой / ошибочный доступ к памяти при поиске фасетов с большим количеством результатов.
* [Issue #1026](https://github.com/manticoresoftware/manticoresearch/issues/1026) RT индекс: searchd «зависает» навсегда при одновременной вставке многих документов и заполнении RAMchunk
* [Commit 4739](https://github.com/manticoresoftware/manticoresearch/commit/4739dafd) Поток зависает при завершении работы, пока между узлами ведется репликация
* [Commit ab87](https://github.com/manticoresoftware/manticoresearch/commit/ab87836782e7ae43fe5f9dea739eed0d336b42c1) Смешивание float и int в JSON фильтре диапазона могло привести к игнорированию фильтра Manticore
* [Commit d001](https://github.com/manticoresoftware/manticoresearch/commit/d00101c2905f6393ce3fca23c4f6dcb2506f4bd9) Float фильтры в JSON были неточными
* [Commit 4092](https://github.com/manticoresoftware/manticoresearch/commit/4092c6a8) Отбрасывание незафиксированных транзакций при изменении индекса (иначе возможен сбой)
* [Commit 9692](https://github.com/manticoresoftware/manticoresearch/commit/9692a840) Ошибка синтаксиса запроса при использовании обратного слеша
* [Commit 0c19](https://github.com/manticoresoftware/manticoresearch/commit/0c19ad8d) workers_clients мог быть неверным в SHOW STATUS
* [Commit 1772](https://github.com/manticoresoftware/manticoresearch/commit/17720e1e2f2ec38337df565bee78f7ce9d9c744a) Исправлен сбой при слиянии RAM сегментов без docstores
* [Commit f45b](https://github.com/manticoresoftware/manticoresearch/commit/f45b337b5c265433bb7d6262ad65d02f65edbaf3) Исправлено пропущенное условие ALL/ANY для JSON фильтра на равенство
* [Commit 3e83](https://github.com/manticoresoftware/manticoresearch/commit/3e83c1f0c0e6e7a9d4297113f69a112cbabd9e64) Репликация могла не работать с ошибкой `got exception while reading ist stream: mkstemp(./gmb_pF6TJi) failed: 13 (Permission denied)`, если searchd был запущен из каталога без прав на запись.
* [Commit 92e5](https://github.com/manticoresoftware/manticoresearch/commit/92e5629f7150f62524b385fca670a01ba9590f5b) С версии 4.0.2 лог сбоев содержал только смещения. Этот коммит исправляет это.

## Version 5.0.2
Выпущено: 30 мая 2022

### Исправления ошибок
* ❗[Issue #791](https://github.com/manticoresoftware/manticoresearch/issues/791) - неправильный размер стека мог вызвать сбой.

## Version 5.0.0
Выпущено: 18 мая 2022


### Основные новые возможности
* 🔬 Поддержка [Manticore Columnar Library 1.15.2](https://github.com/manticoresoftware/columnar/), которая включает в себя бета-версию [Вторичных индексов](Server_settings/Searchd.md#secondary_indexes). Построение вторичных индексов включено по умолчанию для обычных и реального времени columnar и row-wise индексов (если используется [Manticore Columnar Library](https://github.com/manticoresoftware/columnar)), но для включения поиска по ним необходимо установить `secondary_indexes = 1` либо в конфигурационном файле, либо используя [SET GLOBAL](Server_settings/Setting_variables_online.md). Новая функциональность поддерживается во всех операционных системах, кроме старых Debian Stretch и Ubuntu Xenial.
* [Режим только для чтения](Security/Read_only.md): теперь можно задавать слушателей, обрабатывающих только запросы на чтение, игнорируя любые записи.
* Новый endpoint [/cli](Connecting_to_the_server/HTTP.md#/cli) для ещё более простого выполнения SQL запросов по HTTP.
* Более быстрая пакетная вставка/замена/удаление через JSON по HTTP: ранее можно было передавать несколько команд записи через HTTP JSON протокол, но они обрабатывались по одной, теперь же они обрабатываются как одна транзакция.
* [#720](https://github.com/manticoresoftware/manticoresearch/issues/720) Поддержка [Вложенных фильтров](Searching/Filters.md#Nested-bool-query) в JSON протоколе. Ранее нельзя было реализовать конструкции типа `a=1 and (b=2 or c=3)` в JSON: `must` (AND), `should` (OR) и `must_not` (NOT) работали только на верхнем уровне. Теперь их можно вкладывать.
* Поддержка [Chunked transfer encoding](https://en.wikipedia.org/wiki/Chunked_transfer_encoding) в HTTP протоколе. Теперь в вашем приложении можно использовать chunked transfer для передачи больших партий с меньшим расходом ресурсов (так как вычисление `Content-Length` не требуется). На серверной стороне Manticore всегда обрабатывает входящие HTTP данные потоком, без ожидания передачи всей партии, как было ранее, что:
  - снижает пиковое потребление памяти, уменьшая риск OOM
  - сокращает время отклика (наши тесты показали сокращение примерно на 11% при обработке партии в 100MB)
  - позволяет обходить [max_packet_size](Server_settings/Searchd.md#max_packet_size) и передавать партии намного больше максимального разрешённого значения `max_packet_size` (128MB), например, по 1GB за раз.
* [#719](https://github.com/manticoresoftware/manticoresearch/issues/719) Поддержка HTTP интерфейса `100 Continue`: теперь можно передавать большие партии из `curl` (включая curl библиотеки, используемые в разных языках программирования), которые по умолчанию используют `Expect: 100-continue` и некоторое время ждут перед началом отправки. Ранее нужно было добавлять заголовок `Expect: `, теперь это не требуется.

  <details>

  Раньше (обратите внимание на время отклика):

  ```bash
  $ time curl -v -sX POST http://localhost:9318/bulk -H "Content-Type: application/x-ndjson" --data '{"insert": {"index": "user", "doc":  {"name":"Prof. Matt Heaney IV","email":"ibergnaum@yahoo.com","description":"Tempora ullam eaque consequatur. Vero aut minima ut et ut omnis officiis vel. Molestiae quis voluptatum sint numquam.","age":15,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"Prof. Boyd McKenzie","email":"carlotta11@hotmail.com","description":"Blanditiis maiores odio corporis eaque illum. Aut et rerum iste. Neque et ullam quisquam officia dignissimos quo cumque.","age":84,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"Мистер Йоханн Смит","email":"stiedemann.tristin@ziemann.com","description":"Temporibus amet magnam consequatur omnis consequatur illo fugit. Debitis natus doloremque est tempore deserunt vero. Harum eos corrupti nemo ut.","age":89,"active":1}}}
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
  real	0m1.022s
  user	0m0.001s
  sys	0m0.010s
  ```

  Now:
  ```bash
  $ time curl -v -sX POST http://localhost:9318/bulk -H "Content-Type: application/x-ndjson" --data '{"insert": {"index": "user", "doc":  {"name":"Проф. Мэтт Хини IV","email":"ibergnaum@yahoo.com","description":"Tempora ullam eaque consequatur. Vero aut minima ut et ut omnis officiis vel. Molestiae quis voluptatum sint numquam.","age":15,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"Проф. Бойд Маккензи","email":"carlotta11@hotmail.com","description":"Blanditiis maiores odio corporis eaque illum. Aut et rerum iste. Neque et ullam quisquam officia dignissimos quo cumque.","age":84,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"Мистер Йоханн Смит","email":"stiedemann.tristin@ziemann.com","description":"Temporibus amet magnam consequatur omnis consequatur illo fugit. Debitis natus doloremque est tempore deserunt vero. Harum eos corrupti nemo ut.","age":89,"active":1}}}
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
  real	0m0.015s
  user	0m0.005s
  sys	0m0.004s
  ```

  </details>

* **⚠️ КРИТИЧЕСКИЕ ИЗМЕНЕНИЯ**: [Псевдо-шардинг](Server_settings/Searchd.md#pseudo_sharding) включен по умолчанию. Если вы хотите его отключить, убедитесь, что добавили `pseudo_sharding = 0` в секцию `searchd` вашего конфигурационного файла Manticore.
* Наличие хотя бы одного полнотекстового поля в реальном/простом индексе теперь не является обязательным. Вы теперь можете использовать Manticore даже в случаях, не связанных с полнотекстовым поиском.
* [Быстрый доступ к атрибутам](Creating_a_table/Data_types.md#fast_fetch), поддерживаемым [Manticore Columnar Library](https://github.com/manticoresoftware/columnar): запросы типа `select * from <columnar table>` теперь выполняются гораздо быстрее, особенно если в схеме много полей.
* **⚠️ КРИТИЧЕСКИЕ ИЗМЕНЕНИЯ**: Неявный [cutoff](Searching/Options.md#cutoff). Теперь Manticore не тратит время и ресурсы на обработку данных, не нужных в результирующем наборе. Недостаток в том, что это влияет на `total_found` в [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META) и [hits.total](Searching/Full_text_matching/Basic_usage.md#HTTP-JSON) в JSON-выводе. Это значение теперь точно лишь в случае, если вы видите `total_relation: eq`, тогда как `total_relation: gte` означает, что фактическое количество соответствующих документов больше, чем полученное значение `total_found`. Чтобы сохранить предыдущие поведение, используйте опцию поиска `cutoff=0`, что заставит `total_relation` всегда быть `eq`.
* **⚠️ КРИТИЧЕСКИЕ ИЗМЕНЕНИЯ**: Все полнотекстовые поля теперь по умолчанию [хранятся](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#stored_fields). Чтобы сделать все поля не хранимыми (то есть вернуться к предыдущему поведению), используйте `stored_fields = ` (пустое значение).
* [#715](https://github.com/manticoresoftware/manticoresearch/issues/715) HTTP JSON поддерживает [опции поиска](Searching/Options.md#General-syntax).

### Мелкие изменения
* **⚠️ КРИТИЧЕСКИЕ ИЗМЕНЕНИЯ**: Изменение формата файла мета-информации индекса. Раньше метафайлы (`.meta`, `.sph`) были в бинарном формате, теперь это просто json. Новая версия Manticore автоматически конвертирует старые индексы, но:
  - вы можете получить предупреждение вроде `WARNING: ... syntax error, unexpected TOK_IDENT`
  - вы не сможете запускать индекс на предыдущих версиях Manticore, убедитесь, что у вас есть резервная копия
* **⚠️ КРИТИЧЕСКОЕ ИЗМЕНЕНИЕ**: Поддержка состояния сессии с помощью [HTTP keep-alive](Connecting_to_the_server/HTTP.md#Keep-alive). Это делает HTTP состоящим, если клиент также это поддерживает. Например, используя новый эндпоинт [/cli](Connecting_to_the_server/HTTP.md#/cli) и HTTP keep-alive (который включён по умолчанию во всех браузерах), вы можете вызвать `SHOW META` после `SELECT`, и это будет работать так же, как через mysql. Обратите внимание, что ранее HTTP-заголовок `Connection: keep-alive` тоже поддерживался, но он лишь приводил к повторному использованию того же соединения. С этой версии он также делает состояние сессии состоящим.
* Теперь вы можете указать `columnar_attrs = *`, чтобы определить все ваши атрибуты как колоночные в [plain режиме](Read_this_first.md#Real-time-mode-vs-plain-mode), что полезно, если список длинный.
* Быстрее SST репликация
* **⚠️ КРИТИЧЕСКОЕ ИЗМЕНЕНИЕ**: Протокол репликации был изменён. Если у вас работает кластер репликации, то при обновлении до Manticore 5 необходимо:
  - сначала корректно остановить все ваши узлы
  - затем запустить узел, который был остановлен последним, с `--new-cluster` (в Linux запустите инструмент `manticore_new_cluster`).
  - подробности смотрите в разделе [перезапуск кластера](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster).
* Улучшения репликации:
  - Быстрее SST
  - Устойчивость к помехам, что может помочь при нестабильной сети между узлами репликации
  - Улучшенное логирование
* Улучшения безопасности: теперь Manticore слушает `127.0.0.1` вместо `0.0.0.0`, если в конфигурации вообще не указан параметр `listen`. Хотя в стандартной конфигурации, поставляемой с Manticore Search, параметр `listen` указан, и конфигурация без него нетипична, такая возможность существует. Ранее Manticore слушал на `0.0.0.0`, что небезопасно, теперь он слушает на `127.0.0.1`, который обычно не доступен из Интернета.
* Быстрее агрегация по колоночным атрибутам.
* Повышена точность `AVG()`: ранее Manticore использовал внутренне `float` для агрегатов, теперь используется `double`, что значительно повышает точность.
* Улучшена поддержка JDBC драйвера MySQL.
* Поддержка `DEBUG malloc_stats` для [jemalloc](https://github.com/jemalloc/jemalloc).
* [optimize_cutoff](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#optimize_cutoff) теперь доступен как настройка на уровне таблицы, которую можно задать при CREATE или ALTER таблицы.
* **⚠️ КРИТИЧЕСКОЕ ИЗМЕНЕНИЕ**: [query_log_format](Server_settings/Searchd.md#query_log_format) теперь по умолчанию **`sphinxql`**. Если вы привыкли к формату `plain`, нужно добавить `query_log_format = plain` в ваш конфигурационный файл.
* Значительное снижение потребления памяти: Manticore теперь значительно меньше потребляет ОЗУ при длительной и интенсивной нагрузке вставки/замены/оптимизации, если используются хранимые поля.
* Значение по умолчанию для [shutdown_timeout](Server_settings/Searchd.md#shutdown_timeout) увеличено с 3 до 60 секунд.
* [Коммит ffd0](https://github.com/manticoresoftware/manticoresearch/commit/ffd0499d329d2c383f14c8a44c4cc84338ab56e7) Поддержка Java mysql connector >= 6.0.3: в [Java mysql connection 6.0.3](https://mvnrepository.com/artifact/mysql/mysql-connector-java/6.0.3) изменён способ подключения к mysql, что нарушило совместимость с Manticore. Новое поведение теперь поддерживается.
* [Коммит 1da6](https://github.com/manticoresoftware/manticoresearch/commit/1da6dbec) отключено сохранение нового дискового чанка при загрузке индекса (например, при старте searchd).
* [Issue #746](https://github.com/manticoresoftware/manticoresearch/issues/746) Поддержка glibc >= 2.34.
* [Issue #784](https://github.com/manticoresoftware/manticoresearch/issues/784) подсчёт 'VIP' соединений отдельно от обычных (не-VIP). Ранее VIP-соединения учитывались при лимите `max_connections`, что могло вызвать ошибку «maxed out» для обычных соединений. Теперь VIP-соединения не учитываются в лимите. Текущее число VIP-соединений также доступно через `SHOW STATUS` и `status`.
* [ID](Creating_a_table/Data_types.md#Document-ID) теперь можно задавать явно.
* [Issue #687](https://github.com/manticoresoftware/manticoresearch/issues/687) поддержка сжатия zstd для mysql протокола

### ⚠️ Другие незначительные критические изменения
* ⚠️ Формула BM25F слегка изменена для улучшения релевантности поиска. Это влияет только на результаты поиска при использовании функции [BM25F()](Functions/Searching_and_ranking_functions.md#BM25F%28%29), поведение формулы ранжирования по умолчанию не изменено.
* ⚠️ Изменено поведение REST эндпоинта [/sql](Connecting_to_the_server/HTTP.md#mode=raw): `/sql?mode=raw` теперь требует экранирования и возвращает массив.
* ⚠️ Изменён формат ответа на `/bulk` INSERT/REPLACE/DELETE запросы:
  - ранее каждый подзапрос был отдельной транзакцией и возвращал отдельный ответ
  - теперь весь батч считается одной транзакцией, которая возвращает единый ответ
* ⚠️ Опции поиска `low_priority` и `boolean_simplify` теперь требуют указания значения (`0/1`): раньше можно было писать `SELECT ... OPTION low_priority, boolean_simplify`, теперь нужно писать `SELECT ... OPTION low_priority=1, boolean_simplify=1`.
* ⚠️ Если вы используете старые клиенты [php](https://github.com/manticoresoftware/manticoresearch/blob/master/api/sphinxapi.php), [python](https://github.com/manticoresoftware/manticoresearch/blob/master/api/sphinxapi.py) или [java](https://github.com/manticoresoftware/manticoresearch/tree/master/api/java), пожалуйста, перейдите по соответствующей ссылке и найдите обновлённую версию. **Старые версии не полностью совместимы с Manticore 5.**
* ⚠️ HTTP JSON запросы теперь логируются в другом формате в режиме `query_log_format=sphinxql`. Ранее логировалась только часть полнотекстового запроса, теперь логируется как есть.

### Новые пакеты
* **⚠️ КРИТИЧЕСКОЕ ИЗМЕНЕНИЕ**: из-за новой структуры при обновлении до Manticore 5 рекомендуется удалить старые пакеты перед установкой новых:
  - Для RPM-based систем: `yum remove manticore*`
  - Для Debian и Ubuntu: `apt remove manticore*`
* Новая структура пакетов deb/rpm. Предыдущие версии включали:
  - `manticore-server` с `searchd` (главным демон-поиском) и всем необходимым для него
  - `manticore-tools` с `indexer` и `indextool`
  - `manticore`, включающий всё
  - `manticore-all` RPM как мета-пакет, ссылающийся на `manticore-server` и `manticore-tools`

  Новая структура:
  - `manticore` - deb/rpm мета-пакет, который устанавливает всё вышеперечисленное как зависимости
  - `manticore-server-core` - `searchd` и всё, чтобы запустить его отдельно
  - `manticore-server` - системные файлы и другие вспомогательные скрипты
  - `manticore-tools` - `indexer`, `indextool` и другие инструменты
  - `manticore-common` - файл конфигурации по умолчанию, стандартный каталог данных, стандартные стоп-слова
  - `manticore-icudata`, `manticore-dev`, `manticore-converter` практически не изменились
  - `.tgz` пакет, включающий все пакеты
* Поддержка Ubuntu Jammy
* Поддержка Amazon Linux 2 через [YUM репозиторий](Installation/RHEL_and_Centos.md#YUM-repository)

### Исправления ошибок
* [Issue #815](https://github.com/manticoresoftware/manticoresearch/issues/815) Случайный сбой при использовании функции UDF
* [Issue #287](https://github.com/manticoresoftware/manticoresearch/issues/287) исчерпание памяти при индексировании RT индекса
* [Issue #604](https://github.com/manticoresoftware/manticoresearch/issues/604) Ломающие изменения в sphinxql-parser в версиях 3.6.0, 4.2.0
* [Issue #667](https://github.com/manticoresoftware/manticoresearch/issues/667) FATAL: недостаточно памяти (не удалось выделить 9007199254740992 байт)
* [Issue #676](https://github.com/manticoresoftware/manticoresearch/issues/676) Строки неправильно передаются в UDF
* ❗[Issue #698](https://github.com/manticoresoftware/manticoresearch/issues/698) Сбой searchd после попытки добавить текстовый столбец в rt индекс
* [Issue #705](https://github.com/manticoresoftware/manticoresearch/issues/705) Indexer не мог найти все столбцы
* ❗[Issue #709](https://github.com/manticoresoftware/manticoresearch/issues/705) Группировка по json.boolean работает неправильно
* [Issue #716](https://github.com/manticoresoftware/manticoresearch/issues/716) Ошибки команд indextool, связанных с индексом (например --dumpdict)
* ❗[Issue #724](https://github.com/manticoresoftware/manticoresearch/issues/724) Поля исчезают из выборки
* [Issue #727](https://github.com/manticoresoftware/manticoresearch/issues/727) Несовместимость Content-Type в .NET HttpClient при использовании `application/x-ndjson`
* [Issue #729](https://github.com/manticoresoftware/manticoresearch/issues/729) Расчёт длины поля
* ❗[Issue #730](https://github.com/manticoresoftware/manticoresearch/issues/730) create/insert into/drop columnar table вызывает утечку памяти
* [Issue #731](https://github.com/manticoresoftware/manticoresearch/issues/731) Пустой столбец в результатах при определённых условиях
* ❗[Issue #749](https://github.com/manticoresoftware/manticoresearch/issues/749) Сбой демона при запуске
* ❗[Issue #750](https://github.com/manticoresoftware/manticoresearch/issues/750) Демон зависает при запуске
* ❗[Issue #751](https://github.com/manticoresoftware/manticoresearch/issues/751) Сбой при SST
* [Issue #752](https://github.com/manticoresoftware/manticoresearch/issues/752) Json атрибут отмечается как columnar, если engine='columnar'
* [Issue #753](https://github.com/manticoresoftware/manticoresearch/issues/753) Репликация слушает на 0
* [Issue #754](https://github.com/manticoresoftware/manticoresearch/issues/754) columnar_attrs = * не работает с csvpipe
* ❗[Issue #755](https://github.com/manticoresoftware/manticoresearch/issues/755) Сбой при выборе float в columnar в rt
* ❗[Issue #756](https://github.com/manticoresoftware/manticoresearch/issues/756) Indextool меняет rt индекс во время проверки
* [Issue #757](https://github.com/manticoresoftware/manticoresearch/issues/757) Необходима проверка пересечений портов слушателей
* [Issue #758](https://github.com/manticoresoftware/manticoresearch/issues/758) Логирование оригинальной ошибки, если RT индекс не удалось сохранить на диск
* [Issue #759](https://github.com/manticoresoftware/manticoresearch/issues/759) Сообщается только одна ошибка для конфигурации RE2
* ❗[Issue #760](https://github.com/manticoresoftware/manticoresearch/issues/760) Изменения в потреблении ОЗУ в коммите 5463778558586d2508697fa82e71d657ac36510f
* [Issue #761](https://github.com/manticoresoftware/manticoresearch/issues/761) 3-й узел не формирует непервичный кластер после грязного рестарта
* [Issue #762](https://github.com/manticoresoftware/manticoresearch/issues/762) Счётчик обновлений увеличивается на 2
* [Issue #763](https://github.com/manticoresoftware/manticoresearch/issues/763) Новая версия 4.2.1 повреждает индекс, созданный с 4.2.0 с использованием морфологии
* [Issue #764](https://github.com/manticoresoftware/manticoresearch/issues/764) Отсутствие экранирования в ключах json /sql?mode=raw
* ❗[Issue #765](https://github.com/manticoresoftware/manticoresearch/issues/765) Использование функции скрывает другие значения
* ❗[Issue #766](https://github.com/manticoresoftware/manticoresearch/issues/766) Утечка памяти вызванная строкой в FixupAttrForNetwork
* ❗[Issue #767](https://github.com/manticoresoftware/manticoresearch/issues/767) Утечка памяти в версиях 4.2.0 и 4.2.1, связанная с кэшем docstore
* [Issue #768](https://github.com/manticoresoftware/manticoresearch/issues/768) Странный пинг-понг со хранимыми полями по сети
* [Issue #769](https://github.com/manticoresoftware/manticoresearch/issues/769) lemmatizer_base сбрасывается в пустое, если не упоминается в секции 'common'
* [Issue #770](https://github.com/manticoresoftware/manticoresearch/issues/770) pseudo_sharding замедляет SELECT по id
* [Issue #771](https://github.com/manticoresoftware/manticoresearch/issues/771) DEBUG malloc_stats выводит нули при использовании jemalloc
* [Issue #772](https://github.com/manticoresoftware/manticoresearch/issues/772) Удаление/добавление столбца делает значение невидимым
* [Issue #773](https://github.com/manticoresoftware/manticoresearch/issues/773) Невозможно добавить столбец bit(N) в columnar таблицу
* [Issue #774](https://github.com/manticoresoftware/manticoresearch/issues/774) "cluster" становится пустым при запуске в manticore.json
* ❗[Commit 1da4](https://github.com/manticoresoftware/manticoresearch/commits/1da4ce891e4ff13727cfd3331d72771c64ee7949) HTTP actions are not tracked in SHOW STATUS
* [Commit 3810](https://github.com/manticoresoftware/manticoresearch/commits/381000ab4af4c3c98ec8c730699ad7f39039cec8) disable pseudo_sharding for low frequency single keyword queries
* [Commit 8003](https://github.com/manticoresoftware/manticoresearch/commits/800325cca283a87801b28929d82420fafc76a0ee) fixed stored attributes vs index merge
* [Commit cddf](https://github.com/manticoresoftware/manticoresearch/commits/cddfeed6296f2041c6aae18414e5cd9cabf08281) generalized distinct value fetchers; added specialized distinct fetchers for columnar strings
* [Commit fba4](https://github.com/manticoresoftware/manticoresearch/commits/fba4bb4f) fixed fetching null integer attributes from docstore
* [Commit f300](https://github.com/manticoresoftware/manticoresearch/commit/f3009a9242fa16956adf871c4887e6e0303ba364) `ranker` could be specified twice in query log

## Version 4.2.0, Dec 23 2021

### Major new features
* **Поддержка псевдо-шардинга для индексов в реальном времени и полнотекстовых запросов**. В предыдущем релизе была добавлена ограниченная поддержка псевдо-шардинга. Начиная с этой версии, вы можете получить все преимущества псевдо-шардинга и вашего многоядерного процессора просто включив [searchd.pseudo_sharding](Server_settings/Searchd.md#pseudo_sharding). Самое классное, что для этого не нужно ничего делать с вашими индексами или запросами, просто включите эту опцию, и если у вас есть свободный CPU, он будет использован для снижения времени отклика. Поддерживаются обычные и индексы реального времени для полнотекстовых, фильтровых и аналитических запросов. Например, вот как включение псевдо-шардинга может сделать **время отклика большинства запросов примерно в 10 раз меньше** на [наборе данных комментариев Hacker news](https://zenodo.org/record/45901/), умноженном в 100 раз (116 миллионов документов в обычном индексе).

![Pseudo sharding on vs off in 4.2.0](4.2.0_ps_on_vs_off.png)

* [**Debian Bullseye**](https://manticoresearch.com/install/) теперь поддерживается.

<!-- example pq_transactions_4.2.0 -->
* Транзакции PQ теперь атомарны и изолированы. Ранее поддержка транзакций PQ была ограничена. Это обеспечивает гораздо **быстрее выполнение REPLACE в PQ**, особенно когда необходимо заменить сразу много правил. Подробности о производительности:

<!-- intro -->
#### 4.0.2

<!-- request 4.0.2 -->

Вставка 1 млн правил PQ занимает **48 секунд**, а замена всего 40 тыс правил в пакетах по 10 тыс — **406 секунд**.

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

Вставка 1 млн правил PQ занимает **34 секунды**, а замена их — **23 секунды** в пакетах по 10 тыс.

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

### Minor changes
*  [optimize_cutoff](Server_settings/Searchd.md#optimize_cutoff) теперь доступен как опция конфигурации в секции `searchd`. Это удобно, когда вы хотите ограничить число RT чанков во всех ваших индексах до определённого значения глобально.
* [Commit 0087](https://github.com/manticoresoftware/manticoresearch/commit/00874743823eaf43e2a09a088392fd7e0a617f47) точный подсчёт [count(distinct ...)](Searching/Grouping.md#COUNT%28DISTINCT-field%29) и [FACET ... distinct](Searching/Faceted_search.md#Faceting-without-duplicates) по нескольким локальным физическим индексам (реального времени/обычным) с одинаковым набором/порядком полей.
* [PR #598](https://github.com/manticoresoftware/manticoresearch/pull/598) поддержка bigint для `YEAR()` и других функций временной метки.
* [Commit 8e85](https://github.com/manticoresoftware/manticoresearch/commit/8e85d4bce596cc88f9d9615ea243a27e4afde0f7) Адаптивный [rt_mem_limit](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_mem_limit). Раньше Manticore Search собирал ровно до `rt_mem_limit` данных перед сохранением нового дискового чанка, и во время сохранения продолжал собирать ещё до 10% сверх лимита (так называемый двойной буфер) чтобы минимизировать возможную паузу вставки. Если этот лимит исчерпывался, добавление новых документов блокировалось до полного сохранения дискового чанка. Новый адаптивный лимит основан на том факте, что теперь есть [auto-optimize](Server_settings/Searchd.md#auto_optimize), так что не критично если дисковые чанки не полностью соблюдают `rt_mem_limit` и начинают сброс чанка раньше. Теперь собирается до 50% от `rt_mem_limit` и сохраняется как дисковый чанк. При сохранении анализируются статистика (сколько сохранено, сколько пришло новых документов во время сохранения) и пересчитывается начальный коэффициент, который будет использоваться в следующий раз. Например, если было сохранено 90 миллионов документов, а во время сохранения пришло еще 10 миллионов, коэффициент 90%, значит в следующий раз можно собирать до 90% `rt_mem_limit` перед началом сброса очередного дискового чанка. Значение коэффициента вычисляется автоматически от 33.3% до 95%.
* [Issue #628](https://github.com/manticoresoftware/manticoresearch/issues/628) [unpack_zlib](Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Database_connection.md#unpack_zlib) для источника PostgreSQL. Спасибо, [Dmitry Voronin](https://github.com/dimv36) за [вклад](https://github.com/manticoresoftware/manticoresearch/pull/630).
* [Commit 6d54](https://github.com/manticoresoftware/manticoresearch/commit/6d54cf2b319b131970c29410cc21abcbdf8083b1) `indexer -v` и `--version`. Ранее можно было видеть версию индексатора, но `-v`/`--version` не поддерживались.
* [Issue #662](https://github.com/manticoresoftware/manticoresearch/issues/662) бесконечный лимит mlock по умолчанию при запуске Manticore через systemd.
* [Commit 63c8](https://github.com/manticoresoftware/manticoresearch/commit/63c8cd05cf399e705c2c14939411a92cb784735a) spinlock -> очередь операций для coro rwlock.
* [Commit 4113](https://github.com/manticoresoftware/manticoresearch/commit/41130ce30d1734bbf8e6b8b63c4e815caf9464f2) переменная окружения `MANTICORE_TRACK_RT_ERRORS`, полезная для отладки повреждений сегментов RT.

### Breaking changes
* Версия binlog была увеличена, binlog из предыдущей версии не будет воспроизводиться, поэтому убедитесь, что вы корректно останавливаете Manticore Search во время обновления: после остановки предыдущего экземпляра в `/var/lib/manticore/binlog/` не должно быть никаких файлов binlog, кроме `binlog.meta`.
* [Коммит 3f65](https://github.com/manticoresoftware/manticoresearch/commit/3f659f36e87d99ee262205a8eae4663a255be630) новый столбец "chain" в `show threads option format=all`. Он показывает стек некоторых тикетов информации о задачах, что очень полезно для профилирования, поэтому если вы парсите вывод `show threads`, обратите внимание на новый столбец.
* `searchd.workers` устарел с версии 3.5.0, сейчас это deprecated, если он ещё есть в вашем конфигурационном файле, при запуске будет выдано предупреждение. Manticore Search запустится, но с предупреждением.
* Если вы используете PHP и PDO для доступа к Manticore, необходимо задать `PDO::ATTR_EMULATE_PREPARES`

### Исправления ошибок
* ❗[Проблема #650](https://github.com/manticoresoftware/manticoresearch/issues/650) Manticore 4.0.2 медленнее, чем Manticore 3.6.3. В 4.0.2bulk-инсерты были быстрее, чем в предыдущих версиях, но одиночные вставки документов значительно медленнее. Исправлено в версии 4.2.0.
* ❗[Коммит 22f4](https://github.com/manticoresoftware/manticoresearch/commit/22f4141bed1411c83684ae6e0bae832071cce3df) RT индекс мог повредиться при интенсивной нагрузке REPLACE, или мог крашиться
* [Коммит 03be](https://github.com/manticoresoftware/manticoresearch/commit/03be91e4f720a3f20bee0278dd8f0d44abd3ef0e) исправлена средняя при слиянии группировщиков и группового сортировщика N; исправлено слияние агрегатов
* [Коммит 2ea5](https://github.com/manticoresoftware/manticoresearch/commit/2ea575d34efe38661f14d858f2b552456f29fe42) `indextool --check` мог крашиться
* [Коммит 7ec7](https://github.com/manticoresoftware/manticoresearch/commit/7ec76d4abf01b5fca6f10a0000fe64d5d8e5fa52) проблема исчерпания RAM вызванная UPDATE-операциями
* [Коммит 658a](https://github.com/manticoresoftware/manticoresearch/commit/658a727e41f3c3fbcde9754149528f34ed76ba19) демон мог зависать при INSERT
* [Коммит 46e4](https://github.com/manticoresoftware/manticoresearch/commit/46e42b9b386e31e53247ed634e158b533e28063e) демон мог зависать при завершении работы
* [Коммит f8d7](https://github.com/manticoresoftware/manticoresearch/commit/f8d7d5177cc83d250b493b4cfc619732b29cf072) демон мог аварийно завершаться при завершении работы
* [Коммит 733a](https://github.com/manticoresoftware/manticoresearch/commit/733accf165142a1d1574689525ccca8907971158) демон мог зависать при аварийном завершении
* [Коммит f7f8](https://github.com/manticoresoftware/manticoresearch/commit/f7f8bd8c0dbb23a59a88502dcba16b0918fb2330) демон мог аварийно завершаться при запуске при попытке повторного присоединения к кластеру с некорректным списком узлов
* [Коммит 1401](https://github.com/manticoresoftware/manticoresearch/commit/140155613821ecaddcd1a3dfc4f7369b0df6aef7) распределённый индекс мог полностью забываться в режиме RT, если не удавалось разрешить одного из его агентов при старте
* [Проблема #683](https://github.com/manticoresoftware/manticoresearch/issues/683) attr bit(N) engine='columnar' выдает ошибку
* [Проблема #682](https://github.com/manticoresoftware/manticoresearch/issues/682) create table не может выполниться, но оставляет директорию
* [Проблема #663](https://github.com/manticoresoftware/manticoresearch/issues/663) Конфигурация выдает ошибку: неизвестное имя ключа 'attr_update_reserve'
* [Проблема #632](https://github.com/manticoresoftware/manticoresearch/issues/632) Краш Manticore при пакетных запросах
* [Проблема #679](https://github.com/manticoresoftware/manticoresearch/issues/679) Пакетные запросы снова вызывают краш в версии 4.0.3
* [Коммит f7f8](https://github.com/manticoresoftware/manticoresearch/commit/f7f8bd8c0dbb23a59a88502dcba16b0918fb2330) исправлен краш демона при старте, пытающемся повторно присоединиться к кластеру с неверным списком узлов
* [Проблема #643](https://github.com/manticoresoftware/manticoresearch/issues/643) Manticore 4.0.2 не принимает подключения после серии вставок
* [Проблема #635](https://github.com/manticoresoftware/manticoresearch/issues/635) FACET-запрос с ORDER BY JSON.field или строковым атрибутом мог приводить к крашу
* [Проблема #634](https://github.com/manticoresoftware/manticoresearch/issues/634) Краш SIGSEGV на запросе с packedfactors
* [Коммит 4165](https://github.com/manticoresoftware/manticoresearch/commits/41657f15f8fcf1e93cfad7f835bcf705d6e8349e) morphology_skip_fields не поддерживался оператором create table

## Версия 4.0.2, 21 сентября 2021

### Основные новые возможности
- **Полная поддержка [Manticore Columnar Library](https://github.com/manticoresoftware/columnar)**. Ранее Manticore Columnar Library поддерживалась только для плоских индексов. Теперь поддерживается:
  - в индексах реального времени для операций `INSERT`, `REPLACE`, `DELETE`, `OPTIMIZE`
  - при репликации
  - в `ALTER`
  - в `indextool --check`
- **Автоматическая компактация индексов** ([Проблема #478](https://github.com/manticoresoftware/manticoresearch/issues/478)). Наконец-то вам не нужно вручную вызывать OPTIMIZE или настраивать задачу на cron или другой вид автоматизации. Теперь Manticore делает это автоматически и по умолчанию. Вы можете задать порог компактации по умолчанию через глобальную переменную [optimize_cutoff](Server_settings/Setting_variables_online.md).
- **Переработка системы снимков чанков и блокировок**. Эти изменения на первый взгляд могут быть не видны, но значительно улучшают поведение многих операций в индексах реального времени. Вкратце, раньше большинство операций с данными Manticore сильно зависели от блокировок, теперь мы используем снимки чанков на диске.
- **Значительно ускоренная производительность массовых вставок (bulk INSERT) в индекс реального времени**. Например, на сервере [Hetzner AX101](https://www.hetzner.com/dedicated-rootserver/ax101) с SSD, 128 ГБ ОЗУ и AMD Ryzen™ 9 5950X (16*2 ядра) **в версии 3.6.0 можно было вставлять 236K документов в секунду** в таблицу со схемой `name text, email string, description text, age int, active bit(1)` (стандартный `rt_mem_limit`, размер пакета 25000, 16 параллельных workers на вставку, всего вставлено 16 миллионов документов). В версии 4.0.2 при тех же условиях достигается **357K документов в секунду**.

  <details>

  - операции чтения (например, SELECT, репликация) выполняются с использованием снимков
  - операции, которые просто изменяют внутреннюю структуру индекса без изменения схемы/документов (например, слияние сегментов в RAM, сохранение частей на диске, слияние частей на диске), выполняются с использованием снимков только для чтения и в конце заменяют существующие части
  - UPDATE и DELETE выполняются по существующим частям, но в случае слияния текущие записи собираются и затем применяются к новым частям
  - UPDATE последовательно захватывает эксклюзивную блокировку для каждой части. Слияния захватывают разделяемую блокировку при переходе к этапу сбора атрибутов из части. Таким образом, в одно и то же время к атрибутам части имеет доступ только одна операция (слияние или обновление).
  - когда слияние достигает фазы, когда ему нужны атрибуты, оно устанавливает специальный флаг. Когда UPDATE заканчивается, он проверяет этот флаг, и если он установлен, то всё обновление сохраняется в специальной коллекции. Наконец, когда слияние заканчивается, оно применяет набор обновлений к новорождённой части диска.
  - ALTER выполняется с эксклюзивной блокировкой
  - репликация выполняется как обычная операция чтения, но дополнительно сохраняет атрибуты перед SST и запрещает обновления во время SST

  </details>

- **[ALTER](Updating_table_schema_and_settings.md) может добавить/удалить полнотекстовое поле** (в режиме RT). Ранее могла только добавить/удалить атрибут.
- 🔬 **Экспериментально: псевдо-шардинг для полносканирующих запросов** - позволяет параллелить любой запрос без полнотекстового поиска. Вместо ручного создания шардов теперь можно просто включить новую опцию [searchd.pseudo_sharding](Server_settings/Searchd.md#pseudo_sharding) и ожидать сокращения времени отклика до `CPU cores` раз для не полнотекстовых запросов. Учтите, что он может легко задействовать все CPU ядра, поэтому если вам важна не только задержка, но и пропускная способность — используйте с осторожностью.

### Незначительные изменения
<!-- example -->
- Linux Mint и Ubuntu Hirsute Hippo поддерживаются через [APT-репозиторий](Installation/Debian_and_Ubuntu.md#APT-repository)
- более быстрая обновление по id через HTTP в больших индексах в некоторых случаях (зависит от распределения id)
- [671e65a2](https://github.com/manticoresoftware/lemmatizer-uk/commit/671e65a296800537123a3f8b324eeaa3f58b9632) - добавлен кеш к lemmatizer-uk


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
- [параметры запуска systemd](Starting_the_server/Linux.md#Custom-startup-flags-using-systemd). Теперь не нужно запускать searchd вручную, если требуется запустить Manticore с каким-то специфическим параметром запуска
- новая функция [LEVENSHTEIN()](Functions/String_functions.md#LEVENSHTEIN%28%29), которая вычисляет расстояние Левенштейна
- добавлены новые [флаги запуска searchd](Starting_the_server/Manually.md#searchd-command-line-options) `--replay-flags=ignore-trx-errors` и `--replay-flags=ignore-all-errors`, чтобы можно было запустить searchd, даже если binlog повреждён
- [Issue #621](https://github.com/manticoresoftware/manticoresearch/issues/621) - отображение ошибок из RE2
- более точный [COUNT(DISTINCT)](Searching/Grouping.md#COUNT%28DISTINCT-field%29) для распределённых индексов, состоящих из локальных простых индексов
- [FACET DISTINCT](Searching/Faceted_search.md#Faceting-without-duplicates) для удаления дубликатов при выполнении фасетного поиска
- [модификатор точной формы](Searching/Full_text_matching/Operators.md#Exact-form-modifier) теперь не требует [морфологии](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) и работает для индексов с включённым [infix/prefix](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) поиском

### Ломающее изменения
- новая версия может читать старые индексы, но старые версии не могут читать индексы Manticore 4
- убрана неявная сортировка по id. Если нужна, сортируйте явно
- значение по умолчанию параметра `charset_table` изменено с `0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451` на `non_cjk`
- `OPTIMIZE` теперь происходит автоматически. Если не нужно, то установите `auto_optimize=0` в секции `searchd` конфигурационного файла
- [Issue #616](https://github.com/manticoresoftware/manticoresearch/issues/616) `ondisk_attrs_default` устарел и теперь удалён
- для контрибьюторов: теперь мы используем компилятор Clang для сборок под Linux, согласно нашим тестам он собирает более быстрый Manticore Search и Manticore Columnar Library
- если в поисковом запросе параметр [max_matches](Searching/Options.md#max_matches) не указан, он неявно обновляется до минимально необходимого значения для производительности нового колоночного хранилища. Это может повлиять на метрику `total` в [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META), но не на `total_found`, который отражает реальное количество найденных документов.

### Миграция с Manticore 3
- убедитесь, что вы корректно остановили Manticore 3:
  - в `/var/lib/manticore/binlog/` не должно быть binlog файлов (в каталоге должен остаться только `binlog.meta`)
  - иначе Manticore 4 не сможет воспроизвести binlog для некоторых индексов и они не будут запущены
- новая версия может читать старые индексы, но старые версии не могут читать индексы Manticore 4, поэтому обязательно сделайте резервную копию, если хотите иметь возможность легко откатиться
- если у вас кластер репликации, убедитесь, что вы:
  - сначала корректно остановили все узлы
  - затем запустили последний остановленный узел с параметром `--new-cluster` (запустите инструмент `manticore_new_cluster` в Linux).
  - подробнее про [перезапуск кластера](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster)

### Исправления ошибок
- Исправлено множество проблем с репликацией:
  - [Commit 696f](https://github.com/manticoresoftware/manticoresearch/commit/696f8649535f2b5285b878c9581c0d2b1139ae09) - исправлен сбой во время SST на присоединяющемся узле с активным индексом; добавлена проверка sha1 на узле joiner при записи файловых частей для ускорения загрузки индекса; добавлена ротация изменённых файлов индекса на узле joiner при загрузке индекса; добавлено удаление файлов индекса на узле joiner, когда активный индекс заменяется новым индексом с donor узла; добавлены точки журнала репликации на donor узле для отправки файлов и частей
  - [Commit b296](https://github.com/manticoresoftware/manticoresearch/commit/b296c55af68314a8fc66f8b104e7301d673c3b68) - сбой при JOIN CLUSTER в случае неправильного адреса
  - [Commit 418b](https://github.com/manticoresoftware/manticoresearch/commit/418bf880a5e05ffbb68f311a1c23006a9df3220e) - при первоначальной репликации большого индекса узел, присоединяющийся к кластеру, мог аварийно завершить работу с ошибкой `ERROR 1064 (42000): invalid GTID, (null)`, донор мог перестать отвечать во время присоединения другого узла
  - [Commit 6fd3](https://github.com/manticoresoftware/manticoresearch/commit/6fd350d28e4905211cf8a7298cbf7cdda101c83d) - для большого индекса могло неверно вычисляться хэш, что могло привести к сбою репликации
  - [Issue #615](https://github.com/manticoresoftware/manticoresearch/issues/615) - сбой репликации при перезапуске кластера
- [Issue #574](https://github.com/manticoresoftware/manticoresearch/issues/574) - `indextool --help` не отображает параметр `--rotate`
- [Issue #578](https://github.com/manticoresoftware/manticoresearch/issues/578) - поиск high CPU при простое после примерно дня работы
- [Issue #587](https://github.com/manticoresoftware/manticoresearch/issues/587) - немедленный сброс .meta
- [Issue #617](https://github.com/manticoresoftware/manticoresearch/issues/617) - manticore.json очищается
- [Issue #618](https://github.com/manticoresoftware/manticoresearch/issues/618) - searchd --stopwait не работает под root. Также исправлено поведение systemctl (раньше показывал сбой в ExecStop и не ждал достаточно долго остановки searchd)
- [Issue #619](https://github.com/manticoresoftware/manticoresearch/issues/619) - INSERT/REPLACE/DELETE против SHOW STATUS. `command_insert`, `command_replace` и другие показывали неверные метрики
- [Issue #620](https://github.com/manticoresoftware/manticoresearch/issues/620) - `charset_table` для plain индекса имел неверное значение по умолчанию
- [Commit 8f75](https://github.com/manticoresoftware/manticoresearch/commit/8f7536887d4b58a5bab5647fc067b2d0482c2038) - новые дисковые чанки не блокировались в памяти (mlocked)
- [Issue #607](https://github.com/manticoresoftware/manticoresearch/issues/607) - узел кластера Manticore аварийно завершал работу при невозможности разрешить узел по имени
- [Issue #623](https://github.com/manticoresoftware/manticoresearch/issues/623) - репликация обновлённого индекса может привести к неопределённому состоянию
- [Commit ca03](https://github.com/manticoresoftware/manticoresearch/commit/ca03d2280c5197e9b311f0f03f551e3b702a8130) - indexer мог зависать при индексировании plain индекса с атрибутом json
- [Commit 53c7](https://github.com/manticoresoftware/manticoresearch/commit/53c753054558ff3f82c7af365273d32537c5338b) - исправлен фильтр с выражением неравенства в PQ индексе
- [Commit ccf9](https://github.com/manticoresoftware/manticoresearch/commit/ccf94e022ba6763d6286ab5bac767b6f6a846846) - исправлены выборки по окну при запросах списка с более чем 1000 совпадений. Ранее `SELECT * FROM pq ORDER BY id desc LIMIT 1000 , 100 OPTION max_matches=1100` не работал
- [Commit a048](https://github.com/manticoresoftware/manticoresearch/commit/a0483fe9bef3c45dffe96f5df4f4382aee29c072) - HTTPS-запрос к Manticore мог вызывать предупреждение вроде "max packet size(8388608) exceeded"
- [Issue #648](https://github.com/manticoresoftware/manticoresearch/issues/648) - Manticore 3 мог зависать после нескольких обновлений строковых атрибутов


## Version 3.6.0, 3 мая 2021
**Релиз технического обслуживания перед Manticore 4**

### Основные нововведения
- Поддержка [Manticore Columnar Library](https://github.com/manticoresoftware/columnar/) для plain индексов. Новая настройка [columnar_attrs](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#columnar_attrs) для plain индексов
- Поддержка [украинского лемматизатора](https://github.com/manticoresoftware/lemmatizer-uk)
- Полностью переработаны гистограммы. При построении индекса Manticore также строит гистограммы для каждого поля, которые затем используются для ускорения фильтрации. В версии 3.6.0 алгоритм был полностью пересмотрен, что даёт более высокую производительность при большом объёме данных и активной фильтрации.

### Мелкие изменения
- утилита `manticore_new_cluster [--force]` полезна для перезапуска кластера репликации через systemd
- [--drop-src](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Merging_tables.md#Merging-tables) для `indexer --merge`
- [новый режим](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) `blend_mode='trim_all'`
- добавлена [поддержка экранирования путей JSON](Searching/Full_text_matching/Escaping.md#Escaping-json-node-names-in-SQL) при помощи обратных кавычек
- [indextool --check](Miscellaneous_tools.md#indextool) теперь может работать в RT режиме
- [FORCE/IGNORE INDEX(id)](Data_creation_and_modification/Updating_documents/UPDATE.md#Updates-via-SQL) для SELECT/UPDATE
- идентификатор чанка для объединённого дискового чанка теперь уникален
- [indextool --check-disk-chunk CHUNK_NAME](Miscellaneous_tools.md#indextool)

### Оптимизации
- [ускоренный парсинг JSON](https://github.com/manticoresoftware/manticoresearch/commit/0c25fc1788e3f77f60cb7fb8f1623d8466bba606), наши тесты показывают снижение задержки на 3-4% в запросах типа `WHERE json.a = 1`
- не документированная команда `DEBUG SPLIT` как предпосылка для автоматического шардинга/ребалансировки

### Исправления ошибок
- [Issue #584](https://github.com/manticoresoftware/manticoresearch/issues/524) - неточные и нестабильные результаты FACET
- [Issue #506](https://github.com/manticoresoftware/manticoresearch/issues/506) - странное поведение при использовании MATCH: у пострадавших от этой проблемы необходимо перестроить индекс, так как проблема была на этапе построения индекса
- [Issue #387](https://github.com/manticoresoftware/manticoresearch/issues/387) - периодический дамп памяти при выполнении запроса с функцией SNIPPET()
- Оптимизации стека, полезные для обработки сложных запросов:
  - [Issue #469](https://github.com/manticoresoftware/manticoresearch/issues/469) - SELECT приводил к аварийному сбою с дампом
  - [e8420cc7](https://github.com/manticoresoftware/manticoresearch/tree/e8420cc789149c84731be0874be916e33b263bd6) - определение размера стека для деревьев фильтров
- [Issue #461](https://github.com/manticoresoftware/manticoresearch/issues/461) - Обновление с использованием условия IN не применяется корректно
- [Issue #464](https://github.com/manticoresoftware/manticoresearch/issues/464) - SHOW STATUS сразу после CALL PQ возвращает - [Issue #481](https://github.com/manticoresoftware/manticoresearch/issues/481) - Исправлена сборка статического бинарника
- [Issue #502](https://github.com/manticoresoftware/manticoresearch/issues/502) - ошибка в мульти-запросах
- [Issue #514](https://github.com/manticoresoftware/manticoresearch/issues/514) - Невозможно использовать необычные имена столбцов при использовании 'create table'
- [Commit d1db](https://github.com/manticoresoftware/manticoresearch/commit/d1dbe771288328c963ca8512f564c6b7cf4f466c) - краш демона при повторе binlog с обновлением строкового атрибута; установлен номер версии binlog на 10
- [Commit 775d](https://github.com/manticoresoftware/manticoresearch/commit/775d0555562f121911d3c30085947e743fadac2d)  - исправлено время выполнения обнаружения фрейма стека выражения (тест 207)
- [Commit 4795](https://github.com/manticoresoftware/manticoresearch/commit/4795dc49194c7745761bb5006ecb266e8a9224fa)  - перколяционный индексный фильтр и теги были пусты для пустого сохранённого запроса (тест 369)
- [Commit c3f0](https://github.com/manticoresoftware/manticoresearch/commit/c3f0bf4dca0e4eae6ea899dd566878df57eba085)  - сбои репликационного SST потока в сетях с длинной задержкой и высоким уровнем ошибок (репликация между разными дата-центрами); обновлена версия команды репликации до 1.03
- [Commit ba2d](https://github.com/manticoresoftware/manticoresearch/commit/ba2d6619b8107f3fb177741bbdff3709bac248cd)  - блокировка объединителя кластера при операциях записи после объединения в кластер (тест 385)
- [Commit de4d](https://github.com/manticoresoftware/manticoresearch/commit/de4dcb9ff6c316bc2a9ef6c62eb64769f04ee103)  - сопоставление с шаблонами с точным модификатором (тест 321)
- [Commit 6524](https://github.com/manticoresoftware/manticoresearch/commit/6524fc6af032a0932bd42c37874a619e1fb7f075)  - контрольные точки docid против docstore
- [Commit f4ab](https://github.com/manticoresoftware/manticoresearch/commit/f4ab83c291e65a5873d271c67b5491f629544f1d)  - Несогласованное поведение индексатора при разборе некорректного xml
- [Commit 7b72](https://github.com/manticoresoftware/manticoresearch/commit/7b727e22fe01b1a180d4a8d3a3fea92c18eac707)  - Сохранённый перколяционный запрос с NOTNEAR выполняется вечно (тест 349)
- [Commit 812d](https://github.com/manticoresoftware/manticoresearch/commit/812dab745f0c972ecde5d5b3d4a313cd6e1e143e)  - неправильный вес для фразы, начинающейся с шаблона
- [Commit 1771](https://github.com/manticoresoftware/manticoresearch/commit/1771afc669c75670b3511bf02ab9bfc8982b94d2)  - перколяционный запрос с шаблонами генерирует термы без полезной нагрузки при сопоставлении, что вызывает перемешанные попадания и сбои сопоставления (тест 417)
- [Commit aa0d](https://github.com/manticoresoftware/manticoresearch/commit/aa0d8c2bcb41428d41dbfb66e42a872a79a4ee4b)  - исправлен подсчёт 'total' в случае параллельного запроса
- [Commit 18d8](https://github.com/manticoresoftware/manticoresearch/commit/18d81b3cfbb930b7361dae645fe13283d4fbf6ef)  - краш в Windows при множественных одновременных сессиях в демоне
- [Commit 8443](https://github.com/manticoresoftware/manticoresearch/commit/84432f23d6fbf7a7d0b67c8c9412eb5933b03596)  - некоторые настройки индекса не могли быть реплицированы
- [Commit 9341](https://github.com/manticoresoftware/manticoresearch/commit/93411fe6af4246ca4acce758944d513a1660b92b)  - При высокой скорости добавления новых событий netloop иногда зависал из-за того, что атомарное событие 'kick' обрабатывалось один раз на несколько событий, и при этом терялись реальные действия от них
статус запроса, а не статус сервера
- [Commit d805](https://github.com/manticoresoftware/manticoresearch/commit/d805fc125244ef08d14fb3d1826e2c356862d539)  - Новый сброшенный на диск чанк может быть утерян при коммите
- [Commit 63cb](https://github.com/manticoresoftware/manticoresearch/commit/63cbf008d50979cdf7062664fb02a672a3bb1131)  - неточная 'net_read' в профилировщике
- [Commit f537](https://github.com/manticoresoftware/manticoresearch/commit/f5379bb27234b25919966c50ccb6f469ead646e2)  - Проблема перколяции с арабским (тексты справа налево)
- [Commit 49ee](https://github.com/manticoresoftware/manticoresearch/commit/49eeb4202d4ce4875c07c35087d7108c1e177328)  - id не выбирался корректно при дублировании имени столбца
- [Commit refa](https://github.com/manticoresoftware/manticoresearch/commit/4b7df262bbfec9b9e95a44e72ad326ac8a786c49) исправление сети событий для устранения краша в редких случаях
- [e8420cc7](https://github.com/manticoresoftware/manticoresearch/tree/e8420cc789149c84731be0874be916e33b263bd6) исправление в `indextool --dumpheader`
- [Commit ff71](https://github.com/manticoresoftware/manticoresearch/commit/ff7163532ed39042c0d3c3c2b902ff41f94d7f0c) - TRUNCATE WITH RECONFIGURE неправильно работал с сохранёнными полями

### Ломающее изменения:
- Новый формат binlog: необходимо корректно остановить Manticore перед обновлением
- Формат индекса слегка изменился: новая версия может корректно читать ваши существующие индексы, но если вы решите откатиться с версии 3.6.0 на более старую, новые индексы станут нечитаемыми
- Изменение формата репликации: не реплицируйте с более старой версии на 3.6.0 и наоборот, переключайтесь на новую версию сразу на всех узлах
- `reverse_scan` устарел. Убедитесь, что вы не используете эту опцию в запросах начиная с 3.6.0, иначе запросы будут давать ошибку
- С этого выпуска мы больше не предоставляем сборки для RHEL6, Debian Jessie и Ubuntu Trusty. Если для вас критически важно их иметь, [свяжитесь с нами](https://manticoresearch.com/contact-us/)

### Устаревшие возможности
- Больше нет неявной сортировки по id. Если вы пользуетесь этим, обязательно обновите запросы
- Опция поиска `reverse_scan` признана устаревшей

## Версия 3.5.4, 10 декабря 2020

### Новые возможности
- Новые клиенты для Python, Javascript и Java теперь доступны в полном объёме и хорошо документированы в этом руководстве.
- автоматическое удаление дискового чанка реального времени индекса. Эта оптимизация позволяет автоматически удалять дисковый чанк при [OPTIMIZing](Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE) индекса реального времени, когда чанк явно больше не нужен (все документы подавлены). Раньше это все равно требовало слияния, теперь чанк можно просто мгновенно удалить. Опция [cutoff](Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks) игнорируется, т.е. даже если ничего фактически не сливается, устаревший дисковый чанк удаляется. Это полезно в случае, если вы поддерживаете ретеншен в индексе и удаляете более старые документы. Теперь сжатие таких индексов будет быстрее.
- [standalone NOT](Searching/Options.md#not_terms_only_allowed) как опция для SELECT

### Незначительные изменения
- [Issue #453](https://github.com/manticoresoftware/manticoresearch/issues/453) Новая опция [indexer.ignore_non_plain=1](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#ignore_non_plain) полезна в случае, если вы запускаете `indexer --all` и в конфигурационном файле есть не только plain индексы. Без `ignore_non_plain=1` вы получите предупреждение и соответствующий код выхода.
- [SHOW PLAN ... OPTION format=dot](Node_info_and_management/Profiling/Query_plan.md#Dot-format-for-SHOW-PLAN) и [EXPLAIN QUERY ... OPTION format=dot](Searching/Full_text_matching/Profiling.md#Profiling-without-running-a-query) позволяют визуализировать план исполнения полнотекстового запроса. Полезно для понимания сложных запросов.

### Устаревание
- `indexer --verbose` устарел, так как никогда не добавлял ничего к выводу indexer
- Для дампа backtrace watchdog теперь следует использовать сигнал `USR2` вместо `USR1`

### Исправления ошибок
- [Issue #423](https://github.com/manticoresoftware/manticoresearch/issues/423) кириллические символы в режиме вызова сниппетов не подсвечиваются
- [Issue #435](https://github.com/manticoresoftware/manticoresearch/issues/435) RTINDEX - GROUP N BY expression select = фатальный сбой
- [Commit 2b3b](https://github.com/manticoresoftware/manticoresearch/commit/2b3b62bd0ce2b54544733d524a169e81f71b5388) статус searchd показывает Segmentation fault при работе в кластере
- [Commit 9dd2](https://github.com/manticoresoftware/manticoresearch/commit/9dd25c193ed2a53750a0e0e68566fd2b833fcb0c) 'SHOW INDEX index.N SETTINGS' не обрабатывает чанки >9
- [Issue #389](https://github.com/manticoresoftware/manticoresearch/issues/389) Ошибка, приводящая к сбою Manticore
- [Commit fba1](https://github.com/manticoresoftware/manticoresearch/commit/fba166178ff35c73a0c70d131cf3aca4a635e961) Конвертер создает сломанные индексы
- [Commit eecd](https://github.com/manticoresoftware/manticoresearch/commit/eecd61d861c742471175be19a5e5455034b5db5f) stopword_step=0 vs CALL SNIPPETS()
- [Commit ea68](https://github.com/manticoresoftware/manticoresearch/commit/ea6850e449601199190f38a387c04b8a7b7e90f6) count distinct возвращает 0 при низком max_matches на локальном индексе
- [Commit 362f](https://github.com/manticoresoftware/manticoresearch/commit/362f27db3421f09a41c6cce04b5baace00fa4c4d) При использовании агрегации хранящиеся тексты не возвращаются в результатах


## Версия 3.5.2, 1 октябрь 2020

### Новые возможности

* OPTIMIZE теперь сокращает количество дисковых чанков до некоторого числа (по умолчанию `2* Количество ядер`), а не до одного. Оптимальное количество чанков можно контролировать с помощью опции [cutoff](Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks).
* Оператор NOT теперь может использоваться самостоятельно. По умолчанию он отключен, так как случайные одиночные запросы с NOT могут быть медленными. Его можно включить, установив новый директив searchd [not_terms_only_allowed](Server_settings/Searchd.md#not_terms_only_allowed) в значение `0`.
* Новая настройка [max_threads_per_query](Server_settings/Searchd.md#max_threads_per_query) задает, сколько потоков может использовать запрос. Если директива не задана, запрос может использовать потоки до значения [threads](Server_settings/Searchd.md#threads).
Для каждого `SELECT` запроса количество потоков можно ограничить опцией [OPTION threads=N](Searching/Options.md#threads) переопределяющей глобальное `max_threads_per_query`.
* Перколяторные индексы теперь можно импортировать с помощью [IMPORT TABLE](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Importing_table.md).
* HTTP API `/search` получает базовую поддержку для [фасетного поиска](Searching/Faceted_search.md#HTTP-JSON)/[группировки](Searching/Grouping.md) через новый узел запроса `aggs`.

### Незначительные изменения

* Если не объявлена директива прослушивания для репликации, движок будет пытаться использовать порты после заданного порта 'sphinx', до 200.
* `listen=...:sphinx` теперь требуется явно указывать для подключений SphinxSE или клиентов SphinxAPI.
* [SHOW INDEX STATUS](Node_info_and_management/Table_settings_and_status/SHOW_TABLE_STATUS.md) выводит новые метрики: `killed_documents`, `killed_rate`, `disk_mapped_doclists`, `disk_mapped_cached_doclists`, `disk_mapped_hitlists` и `disk_mapped_cached_hitlists`.
* SQL команда `status` теперь выводит `Queue\Threads` и `Tasks\Threads`.

### Устаревание:

* `dist_threads` полностью устарел, searchd будет логировать предупреждение, если эта директива по-прежнему используется.

### Docker

Официальный Docker образ теперь основан на Ubuntu 20.04 LTS

### Пакетирование

Помимо обычного пакета `manticore`, Manticore Search теперь можно установить по компонентам:

- `manticore-server-core` - предоставляет `searchd`, manpage, каталог логов, API и модуль galera. Также установит `manticore-common` как зависимость.
- `manticore-server` - предоставляет скрипты автоматизации для ядра (init.d, systemd) и обертку `manticore_new_cluster`. Также установит `manticore-server-core` как зависимость.
- `manticore-common` - предоставляет конфигурации, стоп-слова, общие документы и skeleton папки (datadir, modules и т.д.)
- `manticore-tools` - предоставляет вспомогательные инструменты (`indexer`, `indextool` и т.д.), их manpages и примеры. Также установит `manticore-common` как зависимость.
- `manticore-icudata` (RPM) или `manticore-icudata-65l` (DEB) - предоставляет ICU data файл для использования морфологии ICU.
- `manticore-devel` (RPM) или `manticore-dev` (DEB) - предоставляет заголовочные файлы для UDF.

### Исправления ошибок

1. [Коммит 2a47](https://github.com/manticoresoftware/manticoresearch/commit/2a474dc1a26e8b0f8aaaae95669caf2f1d4b7746) Сбой демона в grouper на RT индексе с разными чанками
2. [Коммит 57a1](https://github.com/manticoresoftware/manticoresearch/commit/57a19e5ad5663ef0ca7436595218fb1221d28c8e) Быстрый путь для пустых удалённых документов
3. [Коммит 07dd](https://github.com/manticoresoftware/manticoresearch/commit/07dd3f313c63fb82c22092f9907ef24e3475250e) Определение фрейма стека выражений во время выполнения
4. [Коммит 08ae](https://github.com/manticoresoftware/manticoresearch/commit/08ae357cf1012bc8e2da54c20b205b592efda3d4) Соответствие более чем 32 полей на перколяторных индексах
5. [Коммит 16b9](https://github.com/manticoresoftware/manticoresearch/commit/16b9390fd4cdb07a77ac4497adb935573a1710e5) Диапазон портов для прослушивания репликации
6. [Коммит 5fa6](https://github.com/manticoresoftware/manticoresearch/commit/5fa671affeacb6441a59c8a88479bfd423df7c81) Показ create table в pq
7. [Коммит 54d1](https://github.com/manticoresoftware/manticoresearch/commit/54d133b6449105a9fb0168db3f1fbb05fb5aa1f6) Поведение HTTPS порта
8. [Коммит fdbb](https://github.com/manticoresoftware/manticoresearch/commit/fdbbe5245cc296cc5c1ae3ae2fb9cb08fb66a248) Перемешивание строк docstore при замене
9. [Коммит afb5](https://github.com/manticoresoftware/manticoresearch/commit/afb53f648ef4e64fa1776c58e66f6e716ac730ad) Переключение уровня сообщения об отсутствии TFO на 'info'
10. [Коммит 59d9](https://github.com/manticoresoftware/manticoresearch/commit/59d94cefc6e88af360b8046726a16b7eaa9f0b71) Сбой при неправильном использовании strcmp
11. [Коммит 04af](https://github.com/manticoresoftware/manticoresearch/commit/04af0349c1e784c2230d204592d6272b440bb375) Добавление индекса в кластер с системными (стоп-словами) файлами
12. [Коммит 5014](https://github.com/manticoresoftware/manticoresearch/commit/50148b4eae9f3a924082005cf5d2f9c8339af7f5) Слияние индексов с большими словарями; оптимизация RT крупных дисковых чанков
13. [Коммит a2ad](https://github.com/manticoresoftware/manticoresearch/commit/a2adf1582843f63d4dcc1dab8f7598e0917094a2) Indextool теперь может сбрасывать метаданные из текущей версии
14. [Коммит 69f6](https://github.com/manticoresoftware/manticoresearch/commit/69f6d5f77c64b97e3f395aea8c555b11c77fb732) Проблема в порядке группировки в GROUP N
15. [Коммит 24d5](https://github.com/manticoresoftware/manticoresearch/commit/24d5d80f508576794b751d70f9f9b524564e3c14) Явная принудительная очистка для SphinxSE после рукопожатия
16. [Коммит 31c4](https://github.com/manticoresoftware/manticoresearch/commit/31c4d78ac03ee85d9a02b86075448a04ac80f9e2) Избегать копирования больших описаний, когда это не нужно
17. [Коммит 2959](https://github.com/manticoresoftware/manticoresearch/commit/2959e2caa8610f98fcdf2e5a82160f58687f4a27) Отрицательное время в show threads
18. [Коммит f0b3](https://github.com/manticoresoftware/manticoresearch/commit/f0b357102c70f3ac1d7da260c60b67322940e647) Плагин фильтра токенов против нулевых дельт позиций
19. [Коммит a49e](https://github.com/manticoresoftware/manticoresearch/commit/a49e5bc13ea763e6f01dd5993e891858a84d1846) Изменение 'FAIL' на 'WARNING' при множественных попаданиях

## Версия 3.5.0, 22 июля 2020

### Основные новые возможности:
* Выпуск занял так много времени, потому что мы усердно работали над изменением режима многозадачности с потоков на **корутины**. Это упрощает конфигурацию и делает параллелизацию запросов гораздо более прямолинейной: Manticore просто использует заданное количество потоков (см. новую настройку [threads](Server_settings/Searchd.md#threads)), а новый режим обеспечивает выполнение этого наиболее оптимальным способом.
* Изменения в [подсветке](Searching/Highlighting.md#Highlighting-options):
  - любая подсветка, работающая с несколькими полями (`highlight({},'field1, field2'`) или `highlight` в json-запросах) теперь по умолчанию применяет ограничения на уровне каждого поля.
  - любая подсветка, работающая с обычным текстом (`highlight({}, string_attr)` или `snippet()` теперь применяет ограничения к целому документу.
  - [ограничения на поле](Searching/Highlighting.md#limits_per_field) можно переключить на глобальные ограничения с помощью опции `limits_per_field=0` (по умолчанию `1`).
  - [allow_empty](Searching/Highlighting.md#allow_empty) теперь по умолчанию равно `0` для подсветки через HTTP JSON.

* Тот же порт [теперь может использоваться](Server_settings/Searchd.md#listen) для http, https и бинарного API (для принятия соединений от удалённого экземпляра Manticore). Всё ещё требуется `listen = *:mysql` для подключений по протоколу mysql. Manticore теперь автоматически определяет тип клиента, пытающегося подключиться, за исключением MySQL (из-за ограничений протокола).

* В режиме RT поле теперь может одновременно быть [текстовым и строковым атрибутом](Creating_a_table/Data_types.md#String) - [задача на GitHub #331](https://github.com/manticoresoftware/manticoresearch/issues/331).

  В [обычном режиме](Read_this_first.md#Real-time-mode-vs-plain-mode) это называется `sql_field_string`. Теперь это доступно в [RT режиме](Read_this_first.md#Real-time-mode-vs-plain-mode) также для real-time индексов. Вы можете использовать это, как показано в примере:

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
* [Репликация](Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Setting-up-replication) теперь может реплицировать внешние файлы (стоп-слова, исключения и т. п.).
* Оператор фильтра [`in`](Searching/Filters.md#Set-filters) теперь доступен через HTTP JSON интерфейс.
* [`expressions`](Searching/Expressions.md#expressions) в HTTP JSON.
* [Теперь можно менять `rt_mem_limit` на лету](https://github.com/manticoresoftware/manticoresearch/issues/344) в RT режиме, т.е. можно делать `ALTER ... rt_mem_limit=<new value>`.
* Теперь можно использовать [отдельные таблицы символов CJK](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology): `chinese`, `japanese` и `korean`.
* [thread_stack](Server_settings/Searchd.md#thread_stack) теперь ограничивает максимальный размер стека потока, а не начальный.
* Улучшен вывод `SHOW THREADS`.
* Отображение прогресса длительного `CALL PQ` в `SHOW THREADS`.
* cpustat, iostat, coredump можно менять во время работы с помощью [SET](Server_settings/Setting_variables_online.md#SET).
* Реализован `SET [GLOBAL] wait_timeout=NUM`,

### Совместимость нарушена:
* **Формат индекса изменён.** Индексы, созданные в версии 3.5.0, не могут быть загружены версиями Manticore ниже 3.5.0, но Manticore 3.5.0 понимает старые форматы.
* [`INSERT INTO PQ VALUES()`](Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md) (т.е. без указания списка столбцов) раньше ожидал именно `(query, tags)` в качестве значений. Теперь это изменено на `(id,query,tags,filters)`. id можно установить в 0, если хотите, чтобы он генерировался автоматически.
* [`allow_empty=0`](Searching/Highlighting.md#allow_empty) — новое значение по умолчанию для подсветки через HTTP JSON интерфейс.
* Для внешних файлов (stopwords, exceptions и т.д.) в `CREATE TABLE`/`ALTER TABLE` допускаются только абсолютные пути.

### Устаревшие возможности:
* `ram_chunks_count` был переименован в `ram_chunk_segments_count` в `SHOW INDEX STATUS`.
* `workers` устарел. Теперь существует только один режим работы с воркерами.
* `dist_threads` устарел. Все запросы теперь максимально параллельны (ограничиваются `threads` и `jobs_queue_size`).
* `max_children` устарел. Используйте [threads](Server_settings/Searchd.md#threads), чтобы задать число потоков для Manticore (по умолчанию равен количеству ядер CPU).
* `queue_max_length` устарел. При необходимости тонкой настройки очереди заданий используйте [jobs_queue_size](Server_settings/Searchd.md#jobs_queue_size) (по умолчанию без ограничений).
* Все эндпоинты `/json/*` теперь доступны без `/json/`, например `/search`, `/insert`, `/delete`, `/pq` и т.д.
* Значение `field` как "полнотекстовое поле" в `describe` переименовано в "text".
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
* Кириллическая `и` не отображается в `i` в таблице символов `non_cjk` (которая используется по умолчанию), так как это слишком сильно влияло на русский стеммер и лемматизатор.
* `read_timeout`. Используйте вместо него [network_timeout](Server_settings/Searchd.md#network_timeout), который контролирует и чтение, и запись.


### Пакеты

* Официальный пакет для Ubuntu Focal 20.04
* Имя пакета deb изменено с `manticore-bin` на `manticore`

### Исправления ошибок:
1. [Issue #351](https://github.com/manticoresoftware/manticoresearch/issues/351) утечка памяти searchd
2. [Commit ceab](https://github.com/manticoresoftware/manticoresearch/commit/ceabe44f) Незначительный выход за границы при чтении в сниппетах
3. [Commit 1c3e](https://github.com/manticoresoftware/manticoresearch/commit/1c3e84a3) Опасная запись в локальную переменную при аварийных запросах
4. [Commit 26e0](https://github.com/manticoresoftware/manticoresearch/commit/26e094ab) Незначительная утечка памяти сортировщика в тесте 226
5. [Commit d2c7](https://github.com/manticoresoftware/manticoresearch/commit/d2c7f86a) Существенная утечка памяти в тесте 226
6. [Commit 0dd8](https://github.com/manticoresoftware/manticoresearch/commit/0dd80122) Кластер показывает синхронность узлов, но `count(*)` возвращает разные значения
7. [Commit f1c1](https://github.com/manticoresoftware/manticoresearch/commit/f1c1ac3f) Косметика: дублирование и иногда потеря предупреждений в логе
8. [Commit f1c1](https://github.com/manticoresoftware/manticoresearch/commit/f1c1ac3f) Косметика: (null) имя индекса в логе
9. [Commit 359d](https://github.com/manticoresoftware/manticoresearch/commit/359dbd30) Невозможно получить более 70 млн результатов
10. [Commit 19f3](https://github.com/manticoresoftware/manticoresearch/commit/19f328ee) Невозможно вставить правила PQ с синтаксисом без колонок
11. [Commit bf68](https://github.com/manticoresoftware/manticoresearch/commit/bf685d5d) Ввод документа в индекс кластера даёт вводящее в заблуждение сообщение об ошибке
12. [Commit 2cf1](https://github.com/manticoresoftware/manticoresearch/commit/2cf18c83) `/json/replace` и `json/update` возвращают id в экспоненциальной форме
13. [Issue #324](https://github.com/manticoresoftware/manticoresearch/issues/324) Обновление json скалярных свойств и mva в одном запросе
14. [Commit d384](https://github.com/manticoresoftware/manticoresearch/commit/d38409eb) Опция `hitless_words` не работает в RT режиме
15. [Commit 5813](https://github.com/manticoresoftware/manticoresearch/commit/5813d639) В RT режиме должно быть запрещено `ALTER RECONFIGURE`
16. [Commit 5813](https://github.com/manticoresoftware/manticoresearch/commit/5813d639) После перезапуска searchd `rt_mem_limit` сбрасывается в 128M
17. highlight() иногда зависает
18. [Commit 7cd8](https://github.com/manticoresoftware/manticoresearch/commit/7cd878f4) Неудачная попытка использования Unicode U+code в RT режиме
19. [Commit 2b21](https://github.com/manticoresoftware/manticoresearch/commit/2b213de4) Неудачная попытка использования wildcard в wordforms в RT режиме
20. [Commit e9d0](https://github.com/manticoresoftware/manticoresearch/commit/e9d07e68) Исправлен `SHOW CREATE TABLE` в случае нескольких файлов wordform
21. [Коммит fc90](https://github.com/manticoresoftware/manticoresearch/commit/fc90a84f) JSON-запрос без "query" вызывает сбой searchd
22. Официальный [docker Manticore](https://hub.docker.com/r/manticoresearch/manticore) не мог индексировать из mysql 8
23. [Коммит 23e0](https://github.com/manticoresoftware/manticoresearch/commit/23e05d32) HTTP /json/insert требует id
24. [Коммит bd67](https://github.com/manticoresoftware/manticoresearch/commit/bd679af0) `SHOW CREATE TABLE` не работает для PQ
25. [Коммит bd67](https://github.com/manticoresoftware/manticoresearch/commit/bd679af0) `CREATE TABLE LIKE` неправильно работает для PQ
26. [Коммит 5eac](https://github.com/manticoresoftware/manticoresearch/commit/5eacf28f) Конец строки в настройках в show index status
27. [Коммит cb15](https://github.com/manticoresoftware/manticoresearch/commit/cb153228) Пустой заголовок в "highlight" в HTTP JSON ответе
28. [Issue #318](https://github.com/manticoresoftware/manticoresearch/issues/318) Ошибка инфикса в `CREATE TABLE LIKE`
29. [Коммит 9040](https://github.com/manticoresoftware/manticoresearch/commit/9040d22c) RT падает под нагрузкой
30. [cd512c7d ](https://github.com/manticoresoftware/manticoresearch/commit/cd512c7d ) Потерян лог сбоя при падении в RT disk chunk
31. [Issue #323](https://github.com/manticoresoftware/manticoresearch/issues/323) Импорт таблицы завершился сбоем и закрыл соединение
32. [Коммит 6275](https://github.com/manticoresoftware/manticoresearch/commit/6275316a) `ALTER reconfigure` повреждает индекс PQ
33. [Коммит 9c1d](https://github.com/manticoresoftware/manticoresearch/commit/9c1d221e) Проблемы с перезагрузкой searchd после изменения типа индекса
34. [Коммит 71e2](https://github.com/manticoresoftware/manticoresearch/commit/71e2b5bb) Демон падает при импорте таблицы с отсутствующими файлами
35. [Issue #322](https://github.com/manticoresoftware/manticoresearch/issues/322) Сбой при select с использованием нескольких индексов, group by и ranker = none
36. [Коммит c3f5](https://github.com/manticoresoftware/manticoresearch/commit/c3f58490) `HIGHLIGHT()` не выделяет в строковых атрибутах
37. [Issue #320](https://github.com/manticoresoftware/manticoresearch/issues/320) `FACET` не сортирует по строковому атрибуту
38. [Коммит 4f1a](https://github.com/manticoresoftware/manticoresearch/commit/4f1a1f25) Ошибка при отсутствии каталога данных
39. [Коммит 04f4](https://github.com/manticoresoftware/manticoresearch/commit/04f4ddd4) access_* не поддерживаются в RT режиме
40. [Коммит 1c06](https://github.com/manticoresoftware/manticoresearch/commit/1c0616a2) Плохие JSON объекты в строках: 1. `CALL PQ` возвращает "Bad JSON objects in strings: 1", когда json больше определённого значения.
41. [Коммит 32f9](https://github.com/manticoresoftware/manticoresearch/commit/32f943d6) Несогласованность в RT-режиме. В некоторых случаях невозможно удалить индекс, так как он неизвестен, и нельзя создать, так как каталог не пуст.
42. [Issue #319](https://github.com/manticoresoftware/manticoresearch/issues/319) Сбой при select
43. [Коммит 22a2](https://github.com/manticoresoftware/manticoresearch/commit/22a28dd7) `max_xmlpipe2_field` = 2M возвращал предупреждение на поле 2M
44. [Issue #342](https://github.com/manticoresoftware/manticoresearch/issues/342) Ошибка выполнения условий запроса
45. [Коммит dd8d](https://github.com/manticoresoftware/manticoresearch/commit/dd8dcab2) Простой поиск по 2 терминам находит документ, содержащий только один термин
46. [Коммит 9091](https://github.com/manticoresoftware/manticoresearch/commit/90919e62) В PQ было невозможно сопоставить json с заглавными буквами в ключах
47. [Коммит 56da](https://github.com/manticoresoftware/manticoresearch/commit/56da086a) Индексатор падает на csv+docstore
48. [Issue #363](https://github.com/manticoresoftware/manticoresearch/issues/363) Использование `[null]` в json атрибуте в centos 7 вызывает повреждённые вставленные данные
49. Основной [Issue #345](https://github.com/manticoresoftware/manticoresearch/issues/345) Записи не вставляются, count() рандомный, "replace into" возвращает OK
50. max_query_time слишком сильно замедляет SELECTы
51. [Issue #352](https://github.com/manticoresoftware/manticoresearch/issues/352) Ошибка в коммуникации master-agent на Mac OS
52. [Issue #328](https://github.com/manticoresoftware/manticoresearch/issues/328) Ошибка при соединении с Manticore через Connector.Net/Mysql 8.0.19
53. [Коммит daa7](https://github.com/manticoresoftware/manticoresearch/commit/daa760d2fe83668038318f0f384ceefc0d075e18) Исправлено экранирование \0 и оптимизирована производительность
54. [Коммит 9bc5](https://github.com/manticoresoftware/manticoresearch/commit/9bc5c01a493db993b90ff6c1ad51aeae028f629c) Исправлен count distinct для json
55. [Коммит 4f89](https://github.com/manticoresoftware/manticoresearch/commit/4f89a965a3af0db548ae9d1763731ebaebe21f5a) Исправлена ошибка при удалении таблицы на другом узле
56. [Коммит 952a](https://github.com/manticoresoftware/manticoresearch/commit/952af5a5931fd72b87edc169588777778b365231) Исправлены сбои при частом вызове call pq


## Версия 3.4.2, 10 апреля 2020
### Критические исправления ошибок
* [Коммит 2ffe](https://github.com/manticoresoftware/manticoresearch/commit/2ffe2d261793f9681265bc4abe38d695e72c121e) исправлена ошибка: RT индекс из старой версии не индексировал данные

## Версия 3.4.0, 26 марта 2020
### Главные изменения
* сервер работает в 2 режимах: rt-mode и plain-mode
   *   rt-mode требует data_dir и отсутствия определения индекса в конфиге
   *   в plain-mode индексы определяются в конфиге; data_dir не разрешён
* репликация доступна только в rt-mode

### Незначительные изменения
* charset_table по умолчанию – псевдоним non_cjk
* в rt-mode полнотекстовые поля индексируются и сохраняются по умолчанию
* полнотекстовые поля в rt-mode переименованы с 'field' в 'text'
* ALTER RTINDEX переименован в ALTER TABLE
* TRUNCATE RTINDEX переименован в TRUNCATE TABLE

### Возможности
* поля только для хранения
* SHOW CREATE TABLE, IMPORT TABLE

### Улучшения
* значительно более быстрый lockless PQ
* /sql может выполнять любые SQL-запросы в режиме mode=raw
* псевдоним mysql для протокола mysql41
* state.sql по умолчанию в data_dir

### Исправления ошибок
* [Коммит a533](https://github.com/manticoresoftware/manticoresearch/commit/a5333644e8e0c69746a9f24da8221fc965478e4a) исправлена ошибка при неправильном синтаксисе поля в highlight()
* [Коммит 7fbb](https://github.com/manticoresoftware/manticoresearch/commit/7fbb9f2e618096289cba7f8b834530b46aa94055) исправлена ошибка сервера при репликации RT индекса с docstore
* [Коммит 24a0](https://github.com/manticoresoftware/manticoresearch/commit/24a04687eb43b4f4719c7c4a5fe3c19099240467) исправлена ошибка в highlight для индекса с опцией infix или prefix и для индекса без включённых сохранённых полей
* [Коммит 3465](https://github.com/manticoresoftware/manticoresearch/commit/3465c1ce9c860c09f6ccc5b83a63e3c68c241d8d) исправлена ложная ошибка о пустом docstore и поиске doc-id для пустого индекса
* [Коммит a707](https://github.com/manticoresoftware/manticoresearch/commit/b8601b4144b9092be43a7caa7cd6b19cb37e14c7) исправлена #314 SQL команда insert с завершающей точкой с запятой
* [Коммит 9562](https://github.com/manticoresoftware/manticoresearch/commit/95628c9b9b8f16fe1bd18599a84e9673575adf1a) удалено предупреждение о несовпадении слова(слов) запроса
* [Коммит b860](https://github.com/manticoresoftware/manticoresearch/commit/b8601b4144b9092be43a7caa7cd6b19cb37e14c7) исправлены запросы в сниппетах сегментированных с помощью ICU
* [Коммит 5275](https://github.com/manticoresoftware/manticoresearch/commit/5275516ca4631c584e6e0127fa161a18d2658fd0) исправлена ситуация гонки при поиске/добавлении в кеш блоков docstore
* [Коммит f06e](https://github.com/manticoresoftware/manticoresearch/commit/f06ef97af5c7390427e82525c150535290e42c3e) исправлена утечка памяти в docstore
* [Коммит a725](https://github.com/manticoresoftware/manticoresearch/commit/a7258ba8340e246c0d8aaeb4a7a74129eba807eb) исправлена #316 LAST_INSERT_ID возвращает пустое значение при INSERT
* [Коммит 1ebd](https://github.com/manticoresoftware/manticoresearch/commit/1ebd5bf8d74113bcfa138ce63ce11966990a4215) исправлена #317 json/update HTTP endpoint для поддержки массива для MVA и объекта для JSON атрибута
* [Коммит e426](https://github.com/manticoresoftware/manticoresearch/commit/e426950a536382a142e2a42ed207c341149ff685) исправлена ошибка, при которой индексер сбрасывал rt без явного id

## Версия 3.3.0, 4 февраля 2020
### Нововведения
* Параллельный поиск по Real-Time индексам
* Команда EXPLAIN QUERY
* файл конфигурации без описаний индексов (альфа-версия)
* команды CREATE/DROP TABLE (альфа-версия)
* indexer --print-rt - может читать из источника и выводить INSERT-команды для Real-Time индекса

### Улучшения
* Обновлены стеммеры Snowball до версии 2.0
* Фильтр LIKE для SHOW INDEX STATUS
* улучшено использование памяти при высоком max_matches
* SHOW INDEX STATUS добавляет ram_chunks_count для RT индексов
* без блокировок Priority Queue (PQ)
* изменён LimitNOFILE на 65536


### Исправления ошибок
* [Коммит 9c33](https://github.com/manticoresoftware/manticoresearch/commit/9c33aab8c1fd9059c8644d23d5f58f03010043c6) добавлена проверка схемы индекса на дубликаты атрибутов #293
* [Коммит a008](https://github.com/manticoresoftware/manticoresearch/commit/a0085f9477e191da9e0e835b91583fb9cfde698e) исправлен сбой при hitless terms
* [Коммит 6895](https://github.com/manticoresoftware/manticoresearch/commit/68953740af8dca1833026d0f448c71416ed3044a) исправлен "свободный" docstore после ATTACH
* [Коммит d6f6](https://github.com/manticoresoftware/manticoresearch/commit/d6f696edea53ea2b9a68251e4054e4816082c285) исправлена проблема с docstore в распределённой конфигурации
* [Коммит bce2](https://github.com/manticoresoftware/manticoresearch/commit/bce2b7ec883d208f31094ee42a5b6f01d18a998f) заменён FixedHash на OpenHash в сортировщике
* [Коммит e0ba](https://github.com/manticoresoftware/manticoresearch/commit/e0baf7392937969e640fb5d10e8ccb312fe36446) исправлены атрибуты с дублирующимися именами в описании индекса
* [Коммит ca81](https://github.com/manticoresoftware/manticoresearch/commit/ca81114bf1ac7ea7adcc8509bdc14d1f30b4f461) исправлен html_strip в HIGHLIGHT()
* [Коммит 493a](https://github.com/manticoresoftware/manticoresearch/commit/493a5e916d70fd4a3f14d1ee487234434f4a7ce3) исправлен макрос passage в HIGHLIGHT()
* [Коммит a82d](https://github.com/manticoresoftware/manticoresearch/commit/a82d41c775692f837f267d554d7d4a4b499d69f0) исправлены проблемы двойного буфера при создании RT индекса с маленькими или большими дисковыми чанками
* [Коммит a404](https://github.com/manticoresoftware/manticoresearch/commit/a404c85de0d41d9d1c75b7540479d8a3af9e61aa) исправлено удаление событий для kqueue
* [Коммит 8bea](https://github.com/manticoresoftware/manticoresearch/commit/8bea0f6fd1666b817995c4dd6e67f6f1c9161638) исправлено сохранение дискового чанка при большом значении rt_mem_limit для RT индекса
* [Коммит 8707](https://github.com/manticoresoftware/manticoresearch/commit/8707f0395b7ace52dc0cba9c8ed92a2bc4392932) исправлен переполнение float при индексации
* [Коммит a564](https://github.com/manticoresoftware/manticoresearch/commit/a56434ce71bb7714b4d8e75278aaa075b27ece80) теперь вставка документа с отрицательным ID в RT индекс завершается ошибкой
* [Коммит bbeb](https://github.com/manticoresoftware/manticoresearch/commit/bbebfd7551a68641154761cac962053b54806759) исправлен сбой сервера при использовании ranker fieldmask
* [Коммит 3809](https://github.com/manticoresoftware/manticoresearch/commit/3809cc1b0863e3a5a200dc423be3f7c38be56113) исправлен сбой при использовании кэша запросов
* [Коммит dc2a](https://github.com/manticoresoftware/manticoresearch/commit/dc2a585bc724678679e4297362200bbd49ce32bb) исправлен сбой при использовании RAM сегментов RT индекса с параллельными вставками

## Версия 3.2.2, 19 декабря 2019
### Нововведения
* Автоинкремент ID для RT индексов
* Поддержка подсветки через docstore с помощью новой функции HIGHLIGHT(), доступной также в HTTP API
* SNIPPET() может использовать специальную функцию QUERY(), возвращающую текущий MATCH запрос
* новая опция field_separator для функций подсветки.

### Улучшения и изменения
* ленивое получение сохранённых полей для удалённых узлов (может значительно повысить производительность)
* строки и выражения больше не нарушают оптимизации multi-query и FACET
* сборка для RHEL/CentOS 8 теперь использует mysql libclient из mariadb-connector-c-devel
* файл данных ICU теперь поставляется вместе с пакетами, icu_data_dir удалён
* systemd сервисы содержат политику 'Restart=on-failure'
* indextool теперь может проверять real-time индексы онлайн
* конфигурация по умолчанию теперь /etc/manticoresearch/manticore.conf
* сервис в RHEL/CentOS переименован в 'manticore' вместо 'searchd'
* удалены опции query_mode и exact_phrase для снитпетов

### Исправления ошибок
* [Коммит 6ae4](https://github.com/manticoresoftware/manticoresearch/commit/6ae474c7894a6bee222d5b18e59a44fdbf57843a) исправлено аварийное завершение при SELECT запросе через HTTP интерфейс
* [Коммит 5957](https://github.com/manticoresoftware/manticoresearch/commit/59577513a49eac5a4a3c5e2cb38394d3246b5d35) исправлено сохранение RT индекса дисковых чанков без отметки некоторых документов как удаленных
* [Коммит e861](https://github.com/manticoresoftware/manticoresearch/commit/e861f0fca0e88924450695d4e9d6acff7a36558a) исправлено аварийное завершение при поиске по множественным индексам или множественным запросам с dist_threads
* [Коммит 4409](https://github.com/manticoresoftware/manticoresearch/commit/440991fc977b8479800b45cdbc862eeb1ba5d965) исправлено аварийное завершение при генерации инфикса для длинных терминов с широкими utf8 кодовыми точками
* [Коммит 5fd5](https://github.com/manticoresoftware/manticoresearch/commit/5fd599b48bba527c023e6aa0b262dca51ffb8a1c) исправлена гонка при добавлении сокета к IOCP
* [Коммит cf10](https://github.com/manticoresoftware/manticoresearch/commit/cf10d7d3589f77adfaefbe52a51777bc9c67cf99) исправлена проблема с bool запросами против json select списка
* [Коммит 996d](https://github.com/manticoresoftware/manticoresearch/commit/996de77f4c3ec103d965e28c36098fd07dba50c4) исправлена проверка indextool для сообщения о неверном смещении skiplist, проверка doc2row lookup
* [Коммит 6e3f](https://github.com/manticoresoftware/manticoresearch/commit/6e3fc9e88941c9427410f9d0adeebb7dd09a900f) исправлен индексатор, который создавал плохой индекс с отрицательным смещением skiplist на больших данных
* [Коммит faed](https://github.com/manticoresoftware/manticoresearch/commit/faed3220b423a96401982bf47117edf1f62c584e) исправлено преобразование JSON: теперь конвертируются только числа в строки и строки JSON в числа при выражениях
* [Коммит 5331](https://github.com/manticoresoftware/manticoresearch/commit/533197200249d648ebcbdaca5d605861ee566348) исправлен выход indextool с ошибкой при установке нескольких команд в командной строке
* [Коммит 7955](https://github.com/manticoresoftware/manticoresearch/commit/795520ac351d22e4497ebaf6c249bf25081842b6) исправлено #275 binlog в некорректном состоянии при ошибке "нет места на диске"
* [Коммит 2284](https://github.com/manticoresoftware/manticoresearch/commit/2284da5e3ff77bf7be4038a4d950c0580e4159c8) исправлено #279 аварийное завершение при фильтре IN к JSON атрибуту
* [Коммит ce2e](https://github.com/manticoresoftware/manticoresearch/commit/ce2e4b4765111a7d331ffc256911f12770a8942d) исправлено #281 неправильный вызов закрытия пайпа
* [Коммит 5355](https://github.com/manticoresoftware/manticoresearch/commit/535589ba8a37baf5c6056afb5ff969fd70476feb) исправлено зависание сервера при вызове CALL PQ с рекурсивным JSON атрибутом, закодированным как строка
* [Коммит a5fc](https://github.com/manticoresoftware/manticoresearch/commit/a5fc8a36e1644ee051632408ab8b53e79aeb9ff4) исправлено продвижение за пределы конца doclist в узле multiand
* [Коммит a362](https://github.com/manticoresoftware/manticoresearch/commit/a3628617ef4ee89a811d4ba2c4a04cbbc298c1b5) исправлено получение публичной информации о потоках
* [Коммит f8d2](https://github.com/manticoresoftware/manticoresearch/commit/f8d2d7bbbb90af3d2ee97424f97b05927f09d42b) исправлены блокировки кэша docstore

## Версия 3.2.0, 17 октября 2019
### Новые возможности
* Хранение документов
* новые директивы stored_fields, docstore_cache_size, docstore_block_size, docstore_compression, docstore_compression_level

### Улучшения и изменения
* улучшена поддержка SSL
* обновлена встроенная кодировка non_cjk
* отключён лог запросов UPDATE/DELETE, записывающий SELECT в журнал
* пакеты для RHEL/CentOS 8

### Исправления ошибок
* [Коммит 301a](https://github.com/manticoresoftware/manticoresearch/commit/301a806b14725e842300ee225521407464374c41) исправлено аварийное завершение при замене документа в дисковом чанке RT индекса
* [Коммит 46c1](https://github.com/manticoresoftware/manticoresearch/commit/46c1cad8fa11c1fc09cc34b20d978fdfcd4e6d27) исправлено #269 LIMIT N OFFSET M
* [Коммит 92a4](https://github.com/manticoresoftware/manticoresearch/commit/92a46edaa196ef33f3ac2225fbdba2c2b88ee45d) исправлены DELETE запросы с явно заданным id или списком id для пропуска поиска
* [Коммит 8ca7](https://github.com/manticoresoftware/manticoresearch/commit/8ca78c138d837caab30dc6e2343a3c4d1687bb87) исправлен неправильный индекс после удаления события на netloop с poller windowspoll
* [Коммит 6036](https://github.com/manticoresoftware/manticoresearch/commit/603631e2bff5cb0acf25f68b85621742490eccd9) исправлен округление float в JSON через HTTP
* [Коммит 62f6](https://github.com/manticoresoftware/manticoresearch/commit/62f64cb9e75ec28fc3c29f8414ce9656f5580254) исправлены удалённые сниппеты с первой проверкой пустого пути; исправлены тесты для windows
* [Коммит aba2](https://github.com/manticoresoftware/manticoresearch/commit/aba274c2c5abc1029894ba9f95b786bda93e8f22) исправлена перезагрузка конфигурации для работы на windows так же, как на linux
* [Коммит 6b8c](https://github.com/manticoresoftware/manticoresearch/commit/6b8c4242ef77c3fa4d0ccb7d76d81714b6728f0b) исправлено #194 PQ для работы с морфологией и стеммерами
* [Коммит 174d](https://github.com/manticoresoftware/manticoresearch/commit/174d312905285aa5746f4f404c53e6dc669ef42f) исправлено управление устаревшими сегментами RT

## Версия 3.1.2, 22 августа 2019
### Нововведения и улучшения
* Экспериментальная поддержка SSL для HTTP API
* фильтр по полю для CALL KEYWORDS
* max_matches для /json/search
* автоматический подбор размера по умолчанию Galera gcache.size
* улучшена поддержка FreeBSD

### Исправления ошибок
* [Коммит 0a1a](https://github.com/manticoresoftware/manticoresearch/commit/0a1a2c81038794983b4e30c8aefc7cc20e89b03f) исправлена репликация RT индекса в ноду, где тот же RT индекс существует по другому пути
* [Коммит 4adc](https://github.com/manticoresoftware/manticoresearch/commit/4adc075294ac823289f745e2cc419f18c7dcf2e2) исправлено переназначение flush для индексов без активности
* [Коммит d6c0](https://github.com/manticoresoftware/manticoresearch/commit/d6c00a6f750c04021747e9c99e0f77707a4cbfc3) улучшено переназначение flush для RT/PQ индексов
* [Коммит d0a7](https://github.com/manticoresoftware/manticoresearch/commit/d0a7c95906b9b5ba33c98267d54501cfe27508d6) исправлено #250 опция index_field_lengths для TSV и CSV из конвейера источников
* [Commit 1266](https://github.com/manticoresoftware/manticoresearch/commit/1266d54800242709fbe2c34e72eb69b6595b23bd) исправление некорректного отчета indextool при проверке блочного индекса на пустом индексе
* [Commit 553c](https://github.com/manticoresoftware/manticoresearch/commit/553ca73ca111f28e0888d59cd48b0d6df181ecd6) исправление пустого списка селектов в журнале запросов Manticore SQL
* [Commit 56c8](https://github.com/manticoresoftware/manticoresearch/commit/56c8584456293665ddd4ce4c94307d74473a78b9) исправление ответа indexer на -h/--help

## Версия 3.1.0, 16 июля 2019
### Новые возможности и улучшения
* репликация для RealTime индексов
* ICU токенизатор для китайского языка
* новый вариант морфологии icu_chinese
* новая директива icu_data_dir
* транзакции с многими операторами для репликации
* LAST_INSERT_ID() и @session.last_insert_id
* LIKE 'pattern' для SHOW VARIABLES
* Несколько документов INSERT для перколяционных индексов
* Добавлены парсеры времени для конфигурации
* внутренний диспетчер задач
* mlock для компонентов списков документов и хитов
* путь для jail snippets

### Удаления
* Поддержка библиотеки RLP удалена в пользу ICU; все директивы rlp\* удалены
* обновление ID документа с помощью UPDATE отключено

### Исправления ошибок
* [Commit f047](https://github.com/manticoresoftware/manticoresearch/commit/f047222339fd0e62a07e9ca8b2a829297d5db861) исправление дефектов в concat и group_concat
* [Commit b081](https://github.com/manticoresoftware/manticoresearch/commit/b08147ee24712b5eaa51f261626b5d16c6242fdd) исправление uid запроса в перколяционном индексе на тип атрибута BIGINT
* [Commit 4cd8](https://github.com/manticoresoftware/manticoresearch/commit/4cd85afaf76eef0ce9fa1cd073f3fed160279890) не завершаться при сбое предварительного распределения нового дискового блока
* [Commit 1a55](https://github.com/manticoresoftware/manticoresearch/commit/1a551227886474054111088412f2b2396f485652) добавлен недостающий тип данных timestamp для ALTER
* [Commit f3a8](https://github.com/manticoresoftware/manticoresearch/commit/f3a8e096fd496a6b001d398aadbb781b2dbad929) исправление сбоя неправильного чтения mmap
* [Commit 4475](https://github.com/manticoresoftware/manticoresearch/commit/447577115192db7ac7bfea70b5ba58ee2c61229f) исправление блокировки хеша кластеров в репликации
* [Commit ff47](https://github.com/manticoresoftware/manticoresearch/commit/ff476df955f057bd38e8b6ad6056325e9d6fbb9c) исправление утечки провайдеров в репликации
* [Commit 58dc](https://github.com/manticoresoftware/manticoresearch/commit/58dcbb779c4c30ca1560460c78e35ba114b83716) исправление \#246 неопределенной sigmask в indexer
* [Commit 3dd8](https://github.com/manticoresoftware/manticoresearch/commit/3dd8278e747d534f69fe9c3d360eef188ab7b6d5) исправление гонки в отчете netloop
* [Commit a02a](https://github.com/manticoresoftware/manticoresearch/commit/a02aae05a3fa3d1caf2821f43c7e237b6de76336) нулевой зазор для ребалансировщика стратегий HA

## Версия 3.0.2, 31 мая 2019
### Улучшения
* добавлены mmap ридеры для документов и списков хитов
* Ответ HTTP конечной точки `/sql` теперь такой же, как ответ `/json/search`
* новые директивы `access_plain_attrs`, `access_blob_attrs`, `access_doclists`, `access_hitlists`
* новая директива `server_id` для настроек репликации

### Удаления
* удалена HTTP конечная точка `/search`

### Устаревания
* `read_buffer`, `ondisk_attrs`, `ondisk_attrs_default`, `mlock` заменены на директивы `access_*`

### Исправления ошибок
* [Commit 849c](https://github.com/manticoresoftware/manticoresearch/commit/849c16e1040f382795ba0635cb48686f9db4f2d8) разрешить имена атрибутов, начинающиеся с чисел, в списке селектов
* [Commit 48e6](https://github.com/manticoresoftware/manticoresearch/commit/48e6c302ca37bec0bc49c79619c60bf787ed0708) исправлены MVA в UDF, исправлено алиасирование MVA
* [Commit 0555](https://github.com/manticoresoftware/manticoresearch/commit/055586a9a2ebfa8874f68099c5bc798a145fd4ef) исправление \#187 краха при использовании запроса с SENTENCE
* [Commit 93bf](https://github.com/manticoresoftware/manticoresearch/commit/93bf52f23f1c53c8a93a603387ca5463e1fc2ae8) исправление \#143 поддержка () вокруг MATCH()
* [Commit 599e](https://github.com/manticoresoftware/manticoresearch/commit/599ee79ccd38cce0023a55cbce8f50c3dca62e38) исправлено сохранение состояния кластера в операторе ALTER cluster
* [Commit 230c](https://github.com/manticoresoftware/manticoresearch/commit/230c321e277514b191629538eb3f02d219113d95) исправление краха сервера на ALTER index с blob атрибутами
* [Commit 5802](https://github.com/manticoresoftware/manticoresearch/commit/5802b85aa93a81caea77073a3381ef912fdd677c) исправление \#196 фильтрации по id
* [Commit 25d2](https://github.com/manticoresoftware/manticoresearch/commit/25d2dabda4a299a29bd375405862126da9b4bfac) прекращение поиска в шаблонных индексах
* [Commit 2a30](https://github.com/manticoresoftware/manticoresearch/commit/2a30d5b49d76faab031d195f533974938769b8e6) исправление столбца ID на стандартный тип данных bigint в ответе SQL


## Версия 3.0.0, 6 мая 2019
### Новые возможности и улучшения
* Новое хранилище индексов. Нескалярные атрибуты больше не ограничены размером 4GB на индекс
* директива attr_update_reserve
* Строки, JSON и MVA можно обновлять с помощью UPDATE
* killlists применяются при загрузке индекса
* директива killlist_target
* ускорение поиска multi AND
* улучшенная средняя производительность и использование памяти RAM
* инструмент для преобразования для обновления индексов, созданных в версии 2.x
* функция CONCAT()
* JOIN CLUSTER cluster AT 'nodeaddress:port'
* ALTER CLUSTER posts UPDATE nodes
* директива node_address
* список узлов отображается в SHOW STATUS

### Изменения поведения
* в случае индексов с killlists сервер не вращает индексы в порядке, определенном в конфигурации, а следует цепочке целевых списков удаления
* порядок индексов в поиске больше не определяет порядок, в котором применяются killlists
* ID документов теперь являются подписанными большими целыми числами

### Удаленные директивы
* docinfo (всегда extern теперь), inplace_docinfo_gap, mva_updates_pool

## Версия 2.8.2 GA, 2 апреля 2019
### Новые возможности и улучшения
* Репликация Galera для перколяционных индексов
* OPTION морфология

### Примечания по компиляции
Минимальная версия Cmake теперь 3.13. Компиляция требует наличие boost и libssl
библиотек для разработки.

### Исправления ошибок
* [Коммит 6967](https://github.com/manticoresoftware/manticoresearch/commit/6967fedb2ef818ec1c825d482563edd05e1c9245) исправлена авария при множественных звездочках в списке select для запроса в множество распределенных индексов
* [Коммит 36df](https://github.com/manticoresoftware/manticoresearch/commit/36df1a407dc08263690e3492518613ace82d69ca) исправлен [\#177](https://github.com/manticoresoftware/manticoresearch/issues/177) большой пакет через интерфейс Manticore SQL
* [Коммит 5793](https://github.com/manticoresoftware/manticoresearch/commit/57932aec734583fa93359faaf5034b2e2c9d352b) исправлена [\#170](https://github.com/manticoresoftware/manticoresearch/issues/170) авария сервера при оптимизации RT с обновлённым MVA
* [Коммит edb2](https://github.com/manticoresoftware/manticoresearch/commit/edb24b870423add86eba471d361e0e5aff098b18) исправлена авария сервера при удалении binlog из-за удаления RT индекса после перезагрузки конфигурации на SIGHUP
* [Коммит bd3e](https://github.com/manticoresoftware/manticoresearch/commit/bd3e66e0085bc7e2e351b817dfc972fd8158fbce) исправлены полезные данные плагина аутентификации mysql handshake
* [Коммит 6a21](https://github.com/manticoresoftware/manticoresearch/commit/6a217f6ed82fb10f752213b15617b9cedf1e8533) исправлена [\#172](https://github.com/manticoresoftware/manticoresearch/issues/172) настройка phrase_boundary в RT индексе
* [Коммит 3562](https://github.com/manticoresoftware/manticoresearch/commit/3562f652753e4091fd7b5b0f65b58341c8cbcb31) исправлена [\#168](https://github.com/manticoresoftware/manticoresearch/issues/168) взаимная блокировка при ATTACH индексе к самому себе
* [Коммит 250b](https://github.com/manticoresoftware/manticoresearch/commit/250b3f0e74ad18dda34ba080aa13ca87798a9ac9) исправлен binlog, сохраняющий пустые мета-данные после аварии сервера
* [Коммит 4aa6](https://github.com/manticoresoftware/manticoresearch/commit/4aa6c69ad3a181cd7c5dafb990fb528121f68e59) исправлена авария сервера вызванная строкой в сортировщике из RT индекса с дисковыми чанками

## Версия 2.8.1 GA, 6 марта 2019
### Нововведения и улучшения
* SUBSTRING_INDEX()
* Поддержка SENTENCE и PARAGRAPH для перколирующих запросов
* systemd генератор для Debian/Ubuntu; также добавлен LimitCORE для разрешения дампа core

### Исправления ошибок
* [Коммит 84fe](https://github.com/manticoresoftware/manticoresearch/commit/84fe7405d17b59f4dcde15a6c8e8503923503def) исправлена авария сервера в режиме match all с пустым полнотекстовым запросом
* [Коммит daa8](https://github.com/manticoresoftware/manticoresearch/commit/daa88b579fa373d5d2e869e43a2e178363b0fef1) исправлена авария при удалении статической строки
* [Коммит 2207](https://github.com/manticoresoftware/manticoresearch/commit/22078537dddbd9ce81a182dcc879f235b6e87004) исправлен код выхода при сбое indextool с FATAL
* [Коммит 0721](https://github.com/manticoresoftware/manticoresearch/commit/0721696d6780c200c65d596624a8187983fb7fcb) исправлен [\#109](https://github.com/manticoresoftware/manticoresearch/issues/109) отсутствие совпадений для префиксов из-за неверной проверки exact form
* [Коммит 8af8](https://github.com/manticoresoftware/manticoresearch/commit/8af810111b8e2f87bc9f378172eff1ab9725c7e7) исправлен [\#161](https://github.com/manticoresoftware/manticoresearch/issues/161) перезагрузки конфигурации для RT индексов
* [Коммит e2d5](https://github.com/manticoresoftware/manticoresearch/commit/e2d592773f4fd702ace90070a793a842a0fec6ab) исправлена авария сервера при работе с большой JSON строкой
* [Коммит 75cd](https://github.com/manticoresoftware/manticoresearch/commit/75cd1342d05cdc352e3dd145cdbc6f79394a165b) исправлено поле PQ в JSON документе, изменённое стриппером индекса, вызывающее неправильное совпадение с соседним полем
* [Коммит e2f7](https://github.com/manticoresoftware/manticoresearch/commit/e2f775437df922674d772ed4417780492502e65a) исправлена авария сервера при разборе JSON на сборках RHEL7
* [Коммит 3a25](https://github.com/manticoresoftware/manticoresearch/commit/3a25a5808feb3f8b80866991b436d6c1241618c2) исправлена авария при unescaping json, когда слэш находится на грани
* [Коммит be9f](https://github.com/manticoresoftware/manticoresearch/commit/be9f497872bea62dbdccd64fc28294fd7776c289) исправлена опция 'skip_empty' для пропуска пустых документов без предупреждения о невалидном json
* [Коммит 266e](https://github.com/manticoresoftware/manticoresearch/commit/266e0e7b088549722a805fc837bf101ff681a5e8) исправлен [\#140](https://github.com/manticoresoftware/manticoresearch/issues/161) вывод 8 знаков в float, когда 6 недостаточно для точности
* [Коммит 3f6d](https://github.com/manticoresoftware/manticoresearch/commit/3f6d23891064ee8e94030c4231497cdd7da33a6a) исправлено создание пустого jsonobj
* [Коммит f3c7](https://github.com/manticoresoftware/manticoresearch/commit/f3c7848a59b7eedc67c3403d6d4b37ce6fa94dc8) исправлен [\#160](https://github.com/manticoresoftware/manticoresearch/issues/161) вывод NULL вместо пустой строки для пустого mva
* [Коммит 0afa](https://github.com/manticoresoftware/manticoresearch/commit/0afa2ed058e5759470b1d7354c722faab34f98bb) исправлен сбой сборки без pthread_getname_np
* [Коммит 9405](https://github.com/manticoresoftware/manticoresearch/commit/9405fccdeb27f0302a8d9a848981d30080216777) исправлена авария при завершении сервера с worker-ами thread_pool

## Версия 2.8.0 GA, 28 января 2019
### Улучшения
* Распределённые индексы для percolate индексов
* Новые опции и изменения CALL PQ:
    *   skip_bad_json
    *   mode (sparsed/sharded)
    *   json документы могут передаваться в виде json массива
    *   shift
    *   Имена колонок 'UID', 'Documents', 'Query', 'Tags', 'Filters' переименованы в 'id', 'documents', 'query', 'tags', 'filters'
* DESCRIBE pq TABLE
* SELECT FROM pq WHERE UID больше не возможен, используйте 'id' вместо этого
* SELECT по pq индексам теперь наравне с обычными индексами (например, можно фильтровать правила через REGEX())
* ANY/ALL можно использовать для PQ тегов
* выражения имеют автоматическое преобразование для JSON полей, явное приведение не требуется
* встроенная 'non_cjk' charset_table и 'cjk' ngram_chars
* встроенные коллекции стоп-слов для 50 языков
* несколько файлов в объявлении стоп-слов можно также разделять запятой
* CALL PQ теперь может принимать JSON массив документов

### Исправления ошибок
* [Commit a4e1](https://github.com/manticoresoftware/manticoresearch/commit/a4e19afee54dafdc04b0da53372dafd8c0d332d6) исправлена утечка, связанная с csjon
* [Commit 28d8](https://github.com/manticoresoftware/manticoresearch/commit/28d862774874891a03e361da1b0347ebe8066ce0) исправлен сбой из-за отсутствующего значения в json
* [Commit bf4e](https://github.com/manticoresoftware/manticoresearch/commit/bf4e9ea27c349cdc9ae4e54e960a4a17b90e38fa) исправлено сохранение пустой метаинформации для RT индекса
* [Commit 33b4](https://github.com/manticoresoftware/manticoresearch/commit/33b4573529e7c7c7bce19bf9d54054866f30d99a) исправлен пропавший флаг формы (exact) для последовательности лемматизатора
* [Commit 6b95](https://github.com/manticoresoftware/manticoresearch/commit/6b95d48240d3b5520afade19c249d79e020a5e88) исправлен предел использования строковых атрибутов > 4M - теперь используется saturate вместо переполнения
* [Commit 6214](https://github.com/manticoresoftware/manticoresearch/commit/621418b829e70af36aaa322a2f51ece3f86bc499) исправлен сбой сервера при SIGHUP с отключённым индексом
* [Commit 3f7e](https://github.com/manticoresoftware/manticoresearch/commit/3f7e35d1482966cc45d5abbcb2de5de3508d66b0) исправлен сбой сервера при одновременных командах статуса сессии API
* [Commit cd9e](https://github.com/manticoresoftware/manticoresearch/commit/cd9e4f1709a48ddafc128c450c2d882bc11ba50e) исправлен сбой сервера при удалении запроса к RT индексу с фильтрами полей
* [Commit 9376](https://github.com/manticoresoftware/manticoresearch/commit/9376470d455fcd256c6abff79c431a6919dfa7ac) исправлен сбой сервера при вызове CALL PQ к распределённому индексу с пустым документом
* [Commit 8868](https://github.com/manticoresoftware/manticoresearch/commit/8868b207644f4f8cc1ab0c270adb35493fed22e1) исправлено обрезание сообщения об ошибке Manticore SQL больше 512 символов
* [Commit de9d](https://github.com/manticoresoftware/manticoresearch/commit/de9deda9c142823d6dbf529423417b5c670fae94) исправлен сбой при сохранении перколационного индекса без binlog
* [Commit 2b21](https://github.com/manticoresoftware/manticoresearch/commit/2b219e1a32791e7740bb210b7d408f96abc6374f) исправлена неработоспособность http-интерфейса в OSX
* [Commit e92c](https://github.com/manticoresoftware/manticoresearch/commit/e92c60240f8f9e1756bfa14fba0bbb402586a389) исправлено ложное сообщение об ошибке indextool при проверке MVA
* [Commit 238b](https://github.com/manticoresoftware/manticoresearch/commit/238bdea59bad89f097403f1c978658ce45f16c70) исправлена блокировка записи при FLUSH RTINDEX — теперь не блокируется весь индекс во время сохранения и при регулярной очистке rt_flush_period
* [Commit c26a](https://github.com/manticoresoftware/manticoresearch/commit/c26a236bd42b7082db079a51a84172066d337d64) исправлено зависание ALTER перколационного индекса в ожидании загрузки поиска
* [Commit 9ee5](https://github.com/manticoresoftware/manticoresearch/commit/9ee5703f6c28f57cd5bff5e705966d93fe30d267) исправлено max_children для использования значения по умолчанию количества потоков в thread_pool при значении 0
* [Commit 5138](https://github.com/manticoresoftware/manticoresearch/commit/5138fc048a1bd146b271ce6a72fe954344281f69) исправлена ошибка индексации данных в индекс с плагином index_token_filter вместе с stopwords и stopword_step=0
* [Commit 2add](https://github.com/manticoresoftware/manticoresearch/commit/2add3d319ac62fe450bf60e89033100853dc2ecf) исправлен сбой при отсутствии lemmatizer_base при использовании aot лемматизаторов в определениях индекса

## Версия 2.7.5 GA, 4 декабря 2018
### Улучшения
* Функция REGEX
* limit/offset для поиска через json API
* точки профилировщика для qcache

### Исправления ошибок
* [Commit eb3c](https://github.com/manticoresoftware/manticoresearch/commit/eb3c7683b9740cff4cc4cadd2ab9f990b9f7d173) исправлен сбой сервера при FACET с множественными атрибутами широких типов
* [Commit d915](https://github.com/manticoresoftware/manticoresearch/commit/d915cf6696514f4be365ffb5981bccaa15983e65) исправлено неявное группирование в главном списке select запроса FACET
* [Commit 5c25](https://github.com/manticoresoftware/manticoresearch/commit/5c25dc271a8c11a1c82ca3c88e518acbc6a8c164) исправлен сбой при запросе с GROUP N BY
* [Commit 85d3](https://github.com/manticoresoftware/manticoresearch/commit/85d30a232d4ffd89916b673a9b2db946d272a945) исправлена взаимная блокировка при обработке сбоев в операциях с памятью
* [Commit 8516](https://github.com/manticoresoftware/manticoresearch/commit/85166b5ef2be38061d62f2b20f15acaa0311258a) исправлено потребление памяти indextool во время проверки
* [Commit 58fb](https://github.com/manticoresoftware/manticoresearch/commit/58fb03166830d8b23e7ab9ce56309db7ddac22f4) удалён не нужный больше инклюд gmock, так как upstream сам это решил

## Версия 2.7.4 GA, 1 ноября 2018
### Улучшения
* SHOW THREADS для удалённых распределённых индексов показывает исходный запрос вместо вызова API
* SHOW THREADS новая опция `format=sphinxql` выводит все запросы в формате SQL
* SHOW PROFILE выводит дополнительную стадию `clone_attrs`

### Исправления ошибок
* [Commit 4f15](https://github.com/manticoresoftware/manticoresearch/commit/4f155712a0bccc1bd01cc191647bc8cff814888e) исправлен сбой сборки с libc без malloc_stats, malloc_trim
* [Commit f974](https://github.com/manticoresoftware/manticoresearch/commit/f974f20bda3214a56877c393a192be1a77150958) исправлены специальные символы внутри слов для результата CALL KEYWORDS
* [Commit 0920](https://github.com/manticoresoftware/manticoresearch/commit/092083282ea8ae0b2e72fb6989c57ccec81e74ac) исправлен сломанный CALL KEYWORDS к распределённому индексу через API или к удалённому агенту
* [Commit fd68](https://github.com/manticoresoftware/manticoresearch/commit/fd686bfe88b720ffd7642e36897ba45161cbd7d2) исправлено распространение agent_query_timeout в распределённом индексе к агентам как max_query_time
* [Commit 4ffa](https://github.com/manticoresoftware/manticoresearch/commit/4ffa623e9d357a3b0b441615089e211f92f8de32) исправлен счётчик общих документов в дисковом чанке, который сбивался командой OPTIMIZE и ломал вычисление веса
* [Commit dcaf](https://github.com/manticoresoftware/manticoresearch/commit/dcaf4e0e3c8e9b0fbec47bbf3307feddec0936a6) исправлены множественные хвостовые попадания в RT индекс из blended
* [Commit eee3](https://github.com/manticoresoftware/manticoresearch/commit/eee381754e902a43a8f499e1c950198a2a3e6ee0) исправлена взаимная блокировка при ротации

## Версия 2.7.3 GA, 26 сентября 2018
### Улучшения
* опция sort_mode для CALL KEYWORDS
* DEBUG на VIP соединении может выполнять 'crash <password>' для намеренного сигнала SIGEGV на сервере
* DEBUG может выполнять 'malloc_stats' для дампа статистики malloc в searchd.log, 'malloc_trim' для вызова malloc_trim()
* улучшена трассировка стека, если gdb установлен в системе

### Исправления ошибок
* [Commit 0f3c](https://github.com/manticoresoftware/manticoresearch/commit/0f3cc333f6129451ad1ae2c97905b6c04ba5d454) исправлен сбой или ошибка rename на Windows
* [Commit 1455](https://github.com/manticoresoftware/manticoresearch/commit/1455ba2bf8079f03ce7f861a8d9662a360d705de) исправлены сбои сервера на 32-битных системах
* [Commit ad37](https://github.com/manticoresoftware/manticoresearch/commit/ad3710d53b5cd6a28b8c60352d0f9e285b03d9fa) исправлен сбой или зависание сервера при пустом выражении SNIPPET
* [Commit b36d](https://github.com/manticoresoftware/manticoresearch/commit/b36d79214364500252665860f000ae85343528b6) исправлена сломанная непостепенная оптимизация и исправлена постепенная оптимизация, чтобы не создавать kill-list для старейшего дискового чанка
* [Commit 34b0](https://github.com/manticoresoftware/manticoresearch/commit/34b032499afd42ce47a4c7247814b4031094388a) исправлен неправильный ответ queue_max_length для SQL и API в режиме потокового пула
* [Commit ae4b](https://github.com/manticoresoftware/manticoresearch/commit/ae4b3202cbdb8014cabe2b90e269d5cf74f49871) исправлен сбой при добавлении запроса полного сканирования в PQ индекс с установленными параметрами regexp или rlp
* [Commit f80f](https://github.com/manticoresoftware/manticoresearch/commit/f80f8d5d7560187078868aed9a9575f4549e98aa) исправлен сбой при последовательном вызове PQ
* [Commit 9742](https://github.com/manticoresoftware/manticoresearch/commit/9742f5f0866af73f8cd8483ecd18a507ea80dd65) рефакторинг AcquireAccum
* [Commit 39e5](https://github.com/manticoresoftware/manticoresearch/commit/39e5bc3751b7295222eb76407c5d72ce1dad545b) исправлена утечка памяти после вызова pq
* [Commit 21bc](https://github.com/manticoresoftware/manticoresearch/commit/21bcc6d17395f0f57dde79f6716ef303b7ea527d) косметический рефакторинг (стиль c++11 c-trs, defaults, nullptrs)
* [Commit 2d69](https://github.com/manticoresoftware/manticoresearch/commit/2d690398f14c736956cfdd66feb4d3091d6b3a4d) исправлена утечка памяти при попытке вставки дубликата в PQ индекс
* [Commit 5ed9](https://github.com/manticoresoftware/manticoresearch/commit/5ed92c4b66da6423fa28f11b1fd59103ed1e1fb9) исправлен сбой при использовании JSON поля IN с большими значениями
* [Commit 4a52](https://github.com/manticoresoftware/manticoresearch/commit/4a5262e2776aa261e34ba5c914ea60804f59de3f) исправлен сбой сервера на CALL KEYWORDS операторе к RT индексу с установленным пределом расширения
* [Commit 5526](https://github.com/manticoresoftware/manticoresearch/commit/552646bb6fefde4e2525298e43e628839b421d3d) исправлен недопустимый фильтр в PQ matches query;
* [Commit 204f](https://github.com/manticoresoftware/manticoresearch/commit/204f521408b730198a5bab2c20407a3d7df94873) введён малый аллокатор для ptr атрибутов
* [Commit 2545](https://github.com/manticoresoftware/manticoresearch/commit/25453e5387152c0575d1eda1b2beb89a49c7493a) рефакторинг ISphFieldFilter на версию с подсчётом ссылок
* [Commit 1366](https://github.com/manticoresoftware/manticoresearch/commit/1366ee06e828397074b69bec9265d7147170c60e) исправлены неопределённое поведение/сигнал SIGSEGV при использовании strtod на не terminated строках
* [Commit 94bc](https://github.com/manticoresoftware/manticoresearch/commit/94bc6fcbdae4d08bcf6da2966e3ec2966091289f) исправлена утечка памяти при обработке результатов JSON
* [Commit e78e](https://github.com/manticoresoftware/manticoresearch/commit/e78e9c948963416caff9e4e46296a58080107835) исправлено чтение за пределами блока памяти при применении добавления атрибута
* [Commit fad5](https://github.com/manticoresoftware/manticoresearch/commit/fad572fb543606b33e688eb82a485b86011545fd) рефакторинг CSphDict для версии с подсчётом ссылок
* [Commit fd84](https://github.com/manticoresoftware/manticoresearch/commit/fd841a472b472280c93f4af3506851d19f6adc45) исправлена утечка внутреннего типа AOT вне его
* [Commit 5ee7](https://github.com/manticoresoftware/manticoresearch/commit/5ee7f208c7685f2ebb9e5623a8802b721e2a7f0a) исправлена утечка управления токенизатором
* [Commit 116c](https://github.com/manticoresoftware/manticoresearch/commit/116c5f1abebee9a0d99afe93546a1d8e4c6c6958) исправлена утечка памяти в grouper
* [Commit 56fd](https://github.com/manticoresoftware/manticoresearch/commit/56fdbc9ab38973a9a0ba8ccee45f71cf33c16423) специальное освобождение/копирование для динамических ptrs в matches (утечка памяти в grouper)
* [Commit b1fc](https://github.com/manticoresoftware/manticoresearch/commit/31db660f378541aa0d1e7cf2e7209bb0a1f47fd8) исправлена утечка динамических строк для RT
* [Commit 517b](https://github.com/manticoresoftware/manticoresearch/commit/517b9e8749f092e923cd2884b674b5bb84e20755) рефакторинг grouper
* [Commit b1fc](https://github.com/manticoresoftware/manticoresearch/commit/b1fc16140e5dc44290686330b476a254e0092107) небольшой рефакторинг (c++11 c-trs, некоторые изменения форматирования)
* [Commit 7034](https://github.com/manticoresoftware/manticoresearch/commit/7034e073f4cf2844762e0a464b7c3de05d3d122b) рефакторинг ISphMatchComparator на версию с подсчётом ссылок
* [Commit b1fc](https://github.com/manticoresoftware/manticoresearch/commit/b1fc16140e5dc44290686330b476a254e0092107) приватизация cloner
* [Commit efbc](https://github.com/manticoresoftware/manticoresearch/commit/efbc0511d6809c4a57453c7283f9bf53f3fb8d97) упрощена обработка native little-endian для MVA_UPSIZE, DOCINFO2ID_T, DOCINFOSETID
* [Commit 6da0](https://github.com/manticoresoftware/manticoresearch/commit/6da0df4ac9946fc59f98d229b90e12c9221dd4c0) добавлена поддержка valgrind для ubertests
* [Commit 1d17](https://github.com/manticoresoftware/manticoresearch/commit/1d17669cb7ec54ac80b392e101f37a688eb98080) исправлен сбой из-за гонки флага 'success' на соединении
* [Commit 5a09](https://github.com/manticoresoftware/manticoresearch/commit/5a09c32d7ded56b2c0bd3e2ad7968cb8d6362919) переключение epoll на edge-triggered режим
* [Commit 5d52](https://github.com/manticoresoftware/manticoresearch/commit/5d528682737fca03a4352a3093b1319ec27d598c) исправлен оператор IN в выражении с форматированием как в фильтре
* [Commit bd8b](https://github.com/manticoresoftware/manticoresearch/commit/bd8b3c976ff8b4667f55dfdd101b20a920137ac5) исправлен сбой в RT индексе при коммите документа с большим docid
* [Commit ce65](https://github.com/manticoresoftware/manticoresearch/commit/ce656b83b747ce7c55795b53915770c13fb99cfe) исправлены опции без аргументов в indextool
* [Commit 08c9](https://github.com/manticoresoftware/manticoresearch/commit/08c9507177820f441f534bf06a11dac5e54bebb4) исправлена утечка памяти расширенного ключевого слова
* [Commit 30c7](https://github.com/manticoresoftware/manticoresearch/commit/30c75a2f525ec9bda625d903acdc9d4d2e2413dc) исправлена утечка памяти json grouper
* [Commit 6023](https://github.com/manticoresoftware/manticoresearch/commit/6023f269b6f2080e4d380b0957605ef8107c8c9f) исправлена утечка глобальных пользовательских переменных
* [Commit 7c13](https://github.com/manticoresoftware/manticoresearch/commit/7c138f15ca23c0c8717fa12041e3db7f988fcf15) исправлена утечка динамических строк при раннем отклонении совпадений
* [Commit 9154](https://github.com/manticoresoftware/manticoresearch/commit/9154b18eaed5302e8330cb3a95bd968959ccb312) исправлена утечка при length(<expression>)
* [Commit 43fc](https://github.com/manticoresoftware/manticoresearch/commit/43fca3a4e26139765d0fac8de054200dc4875fc6) исправлена утечка памяти из-за strdup() в парсере
* [Commit 71ff](https://github.com/manticoresoftware/manticoresearch/commit/71ff77737e63d1019b7c9804dca7fa2351025dba) переработан парсер выражений для точного отслеживания счетчиков ссылок

## Версия 2.7.2 GA, 27 августа 2018
### Улучшения
* совместимость с клиентами MySQL 8
* [TRUNCATE](Emptying_a_table.md) С RECONFIGURE
* удалён счётчик памяти в SHOW STATUS для RT индексов
* глобальный кэш мультиагентов
* улучшенный IOCP на Windows
* VIP соединения для протокола HTTP
* Manticore SQL команда [DEBUG](Reporting_bugs.md#DEBUG), которая может выполнять различные подкоманды
* [shutdown_token](Server_settings/Searchd.md#shutdown_token) - SHA1-хеш пароля для вызова `shutdown` через команду DEBUG
* новые статистики для SHOW AGENT STATUS (_ping, _has_perspool, _need_resolve)
* опция --verbose у indexer теперь принимает \[debugvv\] для вывода отладочных сообщений

### Исправления ошибок
* [Commit 3900](https://github.com/manticoresoftware/manticoresearch/commit/390082a7be0a1f9539b30361d11d54de35c62a44) удалена блокировка записи при оптимизации
* [Commit 4c33](https://github.com/manticoresoftware/manticoresearch/commit/4c337648329be22373be33333cbc9c5664d18b80) исправлена блокировка записи при перезагрузке настроек индекса
* [Commit b5ea](https://github.com/manticoresoftware/manticoresearch/commit/b5ea8dc0b18a23dbd739e92c66dd93b0094df14b) исправлена утечка памяти при запросе с JSON фильтром
* [Commit 930e](https://github.com/manticoresoftware/manticoresearch/commit/930e831a43fd3d1ce996185be968d1df49a088d0) исправлены пустые документы в наборе результатов PQ
* [Commit 53de](https://github.com/manticoresoftware/manticoresearch/commit/53deecfecb374431f84f8592a1095a77407b8aea) исправлено путаница задач из-за удалённой задачи
* [Commit cad9](https://github.com/manticoresoftware/manticoresearch/commit/cad9b9c617719aad97e9f5a2ae93491153d8e504) исправлен неправильный подсчет удалённого хоста
* [Commit 9000](https://github.com/manticoresoftware/manticoresearch/commit/90008cc15c4684d445d37a391bb4a667bbc2e365) исправлена утечка памяти разобранных дескрипторов агентств
* [Commit 978d](https://github.com/manticoresoftware/manticoresearch/commit/978d839641ad6040c22c9fc3b703b1a02685f266) исправлена утечка в поиске
* [Commit 0193](https://github.com/manticoresoftware/manticoresearch/commit/0193946105ac08913362b7d7acf0c90f9b38dc85) косметические изменения для явных/встроенных c-trs, использования override/final
* [Commit 943e](https://github.com/manticoresoftware/manticoresearch/commit/943e2997b33f9b2eda23d94a66068910f9476ebd) исправлена утечка json в локальной/удалённой схеме
* [Commit 02db](https://github.com/manticoresoftware/manticoresearch/commit/02dbdd6f1d0b72e7e458ebebdd2c67f989577e6b) исправлена утечка json сортировочного выражения колонки в локальной/удалённой схеме
* [Commit c74d](https://github.com/manticoresoftware/manticoresearch/commit/c74d0b4035e4214510376845b22cb676a8da24a3) исправлена утечка константного алиаса
* [Commit 6e5b](https://github.com/manticoresoftware/manticoresearch/commit/6e5b57e1d3952c9695376728a143bf5434208f53) исправлена утечка потока предварительного чтения
* [Commit 39c7](https://github.com/manticoresoftware/manticoresearch/commit/39c740fef5bf71da0bee60a35f5dd9f471af850f) исправлена зависание при выходе из-за зависшего ожидания в netloop
* [Commit adaf](https://github.com/manticoresoftware/manticoresearch/commit/adaf9772f95772ad6d5297acace6c5cb92b56ad5) исправлено застревание поведения 'ping' при смене HA агента на обычный хост
* [Commit 32c4](https://github.com/manticoresoftware/manticoresearch/commit/32c40eac9cdd40b15d79e58b2f15416f164f9701) отдельный сборщик мусора для хранилища панели мониторинга
* [Commit 511a](https://github.com/manticoresoftware/manticoresearch/commit/511a3cf2c1f1858f6e94fe9f8175b7216db3cbd6) исправлена фиксация ref-counted ptr
* [Commit 32c4](https://github.com/manticoresoftware/manticoresearch/commit/af2326cd2927effbad7bec88e8dc238a566cf27e) исправлен сбой indextool при отсутствии индекса
* [Commit 156e](https://github.com/manticoresoftware/manticoresearch/commit/156edc717dc63d3ea120f42466eba6e022da88be) исправлено имя выходного файла при превышении атрибута/поля в xmlpipe индексации
* [Commit cdac](https://github.com/manticoresoftware/manticoresearch/commit/cdac6d1f17d384bfc8bae49e91241b2f346c3830) исправлено значение индексера по умолчанию при отсутствии секции indexer в конфиге
* [Commit e61e](https://github.com/manticoresoftware/manticoresearch/commit/e61ec00b6b27d1d5878247e2ee817f3b1e7fde16) исправлены неверные встроенные стоп-слова в блочном куске диска у RT индекса после перезапуска сервера
* [Commit 5fba](https://github.com/manticoresoftware/manticoresearch/commit/5fba49d5e28c17de4c0acbd984466127f42de6e8) исправлен пропуск фантомных (уже закрытых, но не окончательно удалённых из poller) соединений
* [Commit f22a](https://github.com/manticoresoftware/manticoresearch/commit/f22ae34b623906b3c5a05a06198dad2e548dc541) исправлены смешанные (осиротевшие) сетевые задачи
* [Commit 4689](https://github.com/manticoresoftware/manticoresearch/commit/46890e70eb8dcd00c1bf1b030538b8f131c601c2) исправлен сбой при чтении после записи
* [Commit 03f9](https://github.com/manticoresoftware/manticoresearch/commit/03f9df904f611c438d5ebcaaeef911b0dc8d6c39) исправлены сбои searchd при запуске тестов на Windows
* [Commit e925](https://github.com/manticoresoftware/manticoresearch/commit/e9255ed2704790a3d7f5273d38b85433463cb3ff) исправлена обработка кода EINPROGRESS при обычном connect()
* [Commit 248b](https://github.com/manticoresoftware/manticoresearch/commit/248b72f1544b27e9e82773284050d44eeb1714e1) исправлены тайм-ауты соединения при работе с TFO

## Версия 2.7.1 GA, 4 июля 2018
### Улучшения
* улучшена производительность подстановочных знаков при сопоставлении нескольких документов на PQ
* поддержка fullscan запросов на PQ
* поддержка MVA атрибутов на PQ
* поддержка regexp и RLP для percolate индексов

### Исправления ошибок
* [Commit 6885](https://github.com/manticoresoftware/manticoresearch/commit/68856261b41f6950666f9e5122726839fb3c71d0) исправлена потеря строк запроса
* [Commit 0f17](https://github.com/manticoresoftware/manticoresearch/commit/0f1770943cba53bfd2e0edfdf0b1495ff0dd0e89) исправлено пустое info в операторе SHOW THREADS
* [Commit 53fa](https://github.com/manticoresoftware/manticoresearch/commit/53faa36a916bb87868e83aa79c1a1f972fb20ca0) исправлен сбой при сопоставлении с оператором NOTNEAR
* [Commit 2602](https://github.com/manticoresoftware/manticoresearch/commit/26029a7cd77b518fb3a27588d7db6c8bffc73b0f) исправлено сообщение об ошибке при неправильном фильтре для PQ delete


## Версия 2.7.0 GA, 11 июня 2018
### Улучшения
* уменьшено количество системных вызовов для предотвращения влияния патчей Meltdown и Spectre
* внутренний переписанный менеджмент локальных индексов
* рефакторинг удалённых сниппетов
* полная перезагрузка конфигурации
* все соединения с нодами теперь независимы
* улучшения протокола
* коммуникация в Windows переключена с wsapoll на IO completion ports
* TFO может использоваться для связи между мастером и нодами
* [SHOW STATUS](Node_info_and_management/Node_status.md#SHOW-STATUS) теперь выводит server version и mysql_version_string
* добавлена опция `docs_id` для документов, вызываемых в CALL PQ
* фильтр запроса percolate теперь может содержать выражения
* распределённые индексы могут работать с FEDERATED
* фиктивный SHOW NAMES COLLATE и `SET wait_timeout` (для лучшей совместимости с ProxySQL)

### Исправления ошибок
* [Commit 5bcf](https://github.com/manticoresoftware/manticoresearch/commit/5bcff05d19e495f1a44f190885b25cbb6f217c4a) исправлено добавление тегов "not equal" в PQ
* [Commit 9ebc](https://github.com/manticoresoftware/manticoresearch/commit/9ebc58916cd515eaa88da66d0895aebf0d1f2b5f) исправлено добавление поля id документа в JSON документ CALL PQ
* [Commit 8ae0](https://github.com/manticoresoftware/manticoresearch/commit/8ae0e593a286110d8505d88baa3ac9e433cb38c9) исправлены обработчики оператора flush для PQ индекса
* [Commit c24b](https://github.com/manticoresoftware/manticoresearch/commit/c24b152344ecd77661566cddb803487f51d3c1aa) исправлена фильтрация PQ по JSON и строковым атрибутам
* [Commit 1b8b](https://github.com/manticoresoftware/manticoresearch/commit/1b8bdde19aaa362785ea4c33bdc019154f8fbac4) исправлен разбор пустой JSON строки
* [Commit 1ad8](https://github.com/manticoresoftware/manticoresearch/commit/1ad8a07dbfc1131913a8d92c261fbb48f934e8b7) исправлен сбой при multi-query с OR фильтрами
* [Commit 69b8](https://github.com/manticoresoftware/manticoresearch/commit/69b89806380a229e36287ad9daf6f0ea2b5453eb) исправлен indextool для использования общей секции конфига (опция lemmatizer_base) для команд (dumpheader)
* [Commit 6dbe](https://github.com/manticoresoftware/manticoresearch/commit/6dbeaf2389d2af6a9dfccb56bfc986685b9f203e) исправлена пустая строка в результирующем наборе и фильтре
* [Commit 39c4](https://github.com/manticoresoftware/manticoresearch/commit/39c4eb55ecc85f6dd54f6c8c6d6dfcf87fd8748e) исправлены отрицательные значения id документов
* [Commit 266b](https://github.com/manticoresoftware/manticoresearch/commit/266b7054f98e7d30fc2829b5c2467dc9f90def92) исправлена длина обрезки слова для очень длинных проиндексированных слов
* [Commit 4782](https://github.com/manticoresoftware/manticoresearch/commit/47823b0bd06cbd016a24345c99bb8693790b43dd) исправлено сопоставление нескольких документов wildcard запросов на PQ


## Версия 2.6.4 GA, 3 мая 2018
### Нововведения и улучшения
* поддержка MySQL FEDERATED engine [support](Extensions/FEDERATED.md)
* пакеты MySQL теперь возвращают флаг SERVER_STATUS_AUTOCOMMIT, добавлена совместимость с ProxySQL
* [listen_tfo](Server_settings/Searchd.md#listen_tfo) — включение TCP Fast Open соединений для всех слушателей
* indexer --dumpheader теперь может также выгружать заголовок RT из .meta файла
* cmake скрипт сборки для Ubuntu Bionic

### Исправления ошибок
* [Commit 355b](https://github.com/manticoresoftware/manticoresearch/commit/355b11629174813abd9cd5bf2233be0783f77745) исправлены некорректные записи кеша запросов для RT индекса;
* [Commit 546e](https://github.com/manticoresoftware/manticoresearch/commit/546e2297a01cc2913bc0d33052d5bcefae8737eb) исправлена потеря настроек индекса после бесшовного ротации
* [Commit 0c45](https://github.com/manticoresoftware/manticoresearch/commit/0c4509898393993bc87194d2aca2070395ff7f83) исправлены настройки infix и prefix длины; добавлено предупреждение о неподдерживаемой infix длине
* [Commit 8054](https://github.com/manticoresoftware/manticoresearch/commit/80542fa54d727c781635918e8d4d65a20c2fbd1e) исправлен порядок автофлашинга RT индексов
* [Commit 705d](https://github.com/manticoresoftware/manticoresearch/commit/705d8c5f407726ed1c6f9745ecffa375682c8969) исправлены проблемы со схемой результата набора для индекса с множеством атрибутов и запросами к нескольким индексам
* [Commit b0ba](https://github.com/manticoresoftware/manticoresearch/commit/b0ba932fff4a082f642cb737381c8a70215d4cc9) исправлена потеря некоторых хитов при пакетной вставке с дубликатами документов
* [Commit 4510](https://github.com/manticoresoftware/manticoresearch/commit/4510fa44ad54b5bbac53c591144501565130fcae) исправлен сбой optimize при слиянии дисковых чанков RT индекса с большим количеством документов

## Версия 2.6.3 GA, 28 марта 2018
### Улучшения
* jemalloc при компиляции. Если jemalloc присутствует в системе, его можно включить с помощью флага cmake `-DUSE_JEMALLOC=1`

### Исправления ошибок
* [Commit 85a6](https://github.com/manticoresoftware/manticoresearch/commit/85a6d7e3db75f28c491042409bb677ed39eba07b) исправлена опция log expand_keywords в журнале запросов Manticore SQL
* [Commit caaa](https://github.com/manticoresoftware/manticoresearch/commit/caaa3842f4dbd8507fd9e20bf3371a0c8ec0c087) исправлен HTTP-интерфейс для корректной обработки запросов большого размера
* [Commit e386](https://github.com/manticoresoftware/manticoresearch/commit/e386d842b36195719e459b2db1fb20e402f9a51f) исправлен сбой сервера при DELETE в RT индексе с включённым index_field_lengths
* [Commit cd53](https://github.com/manticoresoftware/manticoresearch/commit/ce528f37eb5d8284661928f6f3b340a4a1985253) исправлена опция cpustats командной строки searchd для работы с неподдерживаемыми системами
* [Commit 8740](https://github.com/manticoresoftware/manticoresearch/commit/8740fd63ae842c2ab0e51a8fcd6180d787ed57ec) исправлено сопоставление подстрок UTF8 с определёнными минимальными длинами


## Версия 2.6.2 GA, 23 февраля 2018
### Улучшения
* улучшена производительность [Percolate Queries](Searching/Percolate_query.md) при использовании оператора NOT и для пакетных документов.
* [percolate_query_call](Searching/Percolate_query.md) теперь может использовать несколько потоков в зависимости от [dist_threads](Server_settings/Searchd.md#threads)
* новый оператор полнотекстового сопоставления NOTNEAR/N
* добавлена поддержка LIMIT для SELECT в percolate индексах
* [expand_keywords](Searching/Options.md#expand_keywords) теперь может принимать значения 'start','exact' (где 'start,exact' имеет тот же эффект, что и '1')
* ranged-main-query для [joined fields](Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Processing_fetched_data.md#sql_joined_field), который использует ограниченный запрос, определённый через sql_query_range

### Исправления ошибок
* [Commit 72dc](https://github.com/manticoresoftware/manticoresearch/commit/72dcf669744e9b7d636dfc213d24df85ab301f6b) исправлен сбой при поиске в RAM сегментах; устранена взаимная блокировка при сохранении диск-чанка с двойным буфером; устранена взаимная блокировка при сохранении диск-чанка во время оптимизации
* [Commit 3613](https://github.com/manticoresoftware/manticoresearch/commit/36137149a1c3c0893bdda5a28fc7e8244bf2d4ae) исправлен сбой индексатора на XML-схеме с пустым именем атрибута
* [Commit 48d7](https://github.com/manticoresoftware/manticoresearch/commit/48d7e8001d2a66466ca64577f27ddc5421a67251) исправлено ошибочное удаление не принадлежащего файла pid
* [Commit a556](https://github.com/manticoresoftware/manticoresearch/commit/a5563a465ddc59ef71e65f17b68bc33f9700e838) исправлены оставшиеся "осиротевшие" fifo-файлы в папке temp
* [Commit 2376](https://github.com/manticoresoftware/manticoresearch/commit/2376e8fc4508944b96959bd10686c6d51f5145e8) исправлен пустой результат FACET с некорректной NULL-строкой
* [Commit 4842](https://github.com/manticoresoftware/manticoresearch/commit/4842b67acdd5da75672db6c08ac563b48577ddd7) исправлена сломанная блокировка индекса при запуске сервера как службы Windows
* [Commit be35](https://github.com/manticoresoftware/manticoresearch/commit/be35feef54ded59125750916cd211e56108eddae) исправлены неправильные библиотеки iconv на Mac OS
* [Commit 8374](https://github.com/manticoresoftware/manticoresearch/commit/83744a977d7837d1d87cd506b88b0e7eb199efe6) исправлен неправильный подсчёт count(\*)


## Версия 2.6.1 GA, 26 января 2018
### Улучшения
* [agent_retry_count](Server_settings/Searchd.md#agent_retry_count) для агентов с зеркалами теперь задаёт число попыток повторов на зеркало, а не на агента, таким образом общее число попыток на агента равно agent_retry_count\*mirrors.
* [agent_retry_count](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_retry_count) теперь можно задавать для каждого индекса отдельно, переопределяя глобальное значение. Добавлен синоним [mirror_retry_count](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#mirror_retry_count).
* retry_count может быть указан в определении агента и обозначает число попыток повторов на агента
* Percolate Queries теперь доступны в HTTP JSON API по адресу [/json/pq](Searching/Percolate_query.md#Performing-a-percolate-query-with-CALL-PQ).
* Добавлены опции -h и -v (помощь и версия) для исполняемых файлов
* [morphology_skip_fields](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) поддержка для индексов Real-Time

### Исправления ошибок
* [Commit a40b](https://github.com/manticoresoftware/manticore/commit/a40b0793feff65e40d10062568d9847c08d10f57) исправлен ranged-main-query для корректной работы с sql_range_step при использовании поля MVA
* [Commit f2f5](https://github.com/manticoresoftware/manticore/commit/f2f53757db45bcfb1544263ce0817e856656a621) исправлена проблема с зависанием цикла blackhole system и с отключением blackhole агентов
* [Commit 84e1](https://github.com/manticoresoftware/manticore/commit/84e1f54aef25e0fce98870ad2dd784db5116f1d6) исправлено согласование id запросов, исправлены дублированные id для сохранённых запросов
* [Commit 1948](https://github.com/manticoresoftware/manticore/commit/19484231814fcb82b21763a3a4a9f45adc6b2d40) исправлен сбой сервера при выключении из различных состояний
* [Commit 9a70](https://github.com/manticoresoftware/manticore/commit/9a706b499a1d61a90076065a1a703029d49db958)  [Commit 3495](https://github.com/manticoresoftware/manticore/commit/3495fd70cba8846b1a50d55d6679b039414c3d2a) тайм-ауты при длительных запросах
* [Commit 3359](https://github.com/manticoresoftware/manticore/commit/3359bcd89b4f79a645fe84b8cf8616ce0addff02) переработан сетевой опрос master-agent для систем с kqueue (Mac OS X, BSD).


## Версия 2.6.0, 29 декабря 2017
### Новые возможности и улучшения
* [HTTP JSON](Connecting_to_the_server/HTTP.md): JSON-запросы теперь могут использовать сравнение атрибутов на равенство, MVA и JSON поля можно использовать при вставках и обновлениях, обновления и удаления через JSON API теперь возможны для распределённых индексов
* [Percolate Queries](Searching/Percolate_query.md)
* Удалена поддержка 32-битных docid из кода. Также удалён весь код, который конвертировал или загружал устаревшие индексы с 32-битными docid.
* [Морфология только для определённых полей](https://github.com/manticoresoftware/manticore/issues/7). Новый директив индекса morphology_skip_fields позволяет задавать список полей, для которых морфология не применяется.
* [expand_keywords теперь может быть директивой выполнения запроса, установленной с помощью оператора OPTION](https://github.com/manticoresoftware/manticore/issues/8)

### Исправления ошибок
* [Commit 0cfa](https://github.com/manticoresoftware/manticore/commit/0cfae4c) исправлен сбой в debug-сборке сервера (и возможно UB в релизной) при сборке с rlp
* [Commit 3242](https://github.com/manticoresoftware/manticore/commit/324291e) исправлена оптимизация RT индекса с включенной опцией progressive, которая объединяла kill-листы в неправильном порядке
* [Commit ac0e](https://github.com/manticoresoftware/manticore/commit/ac0efee) небольшой сбой на mac
* множество мелких исправлений после тщательного статического анализа кода
* другие мелкие исправления ошибок

### Обновление
В этом релизе мы изменили внутренний протокол, используемый мастерами и агентами для общения друг с другом. Если вы запускаете Manticoresearch в распределённой среде с несколькими инстансами, убедитесь, что сначала обновили агенты, а затем мастеров.

## Версия 2.5.1, 23 ноября 2017
### Новые возможности и улучшения
* JSON-запросы через [протокол HTTP API](Connecting_to_the_server/HTTP.md). Поддерживается поиск, вставка, обновление, удаление, замена. Команды манипуляции данными также могут быть пакетными, хотя есть некоторые ограничения, так как MVA и JSON атрибуты не могут использоваться для вставок, замен или обновлений.
* Команда [RELOAD INDEXES](Data_creation_and_modification/Adding_data_from_external_storages/Rotating_a_table.md#RELOAD-TABLES)
* Команда [FLUSH LOGS](Logging/Rotating_query_and_server_logs.md)
* [SHOW THREADS](Node_info_and_management/SHOW_THREADS.md) может показывать прогресс выполнения optimize, rotation или flushes.
* GROUP N BY корректно работает с атрибутами MVA
* blackhole агенты теперь запускаются в отдельном потоке, чтобы больше не влиять на выполнение запросов мастера
* реализован подсчет ссылок на индексы для избежания зависаний, вызванных ротациями и высокой нагрузкой
* реализован SHA1 хешинг, пока что не доступен извне
* исправления для компиляции на FreeBSD, macOS и Alpine

### Исправления ошибок
* [Commit 9897](https://github.com/manticoresoftware/manticore/commit/989752b) регрессия фильтра с блоковым индексом
* [Commit b1c3](https://github.com/manticoresoftware/manticore/commit/b1c3864) переименование PAGE_SIZE -> ARENA_PAGE_SIZE для совместимости с musl
* [Commit f213](https://github.com/manticoresoftware/manticore/commit/f2133cc) отключение googletests для cmake < 3.1.0
* [Commit f30e](https://github.com/manticoresoftware/manticore/commit/0839de7) ошибка связывания сокета при рестарте сервера
* [Commit 0807](https://github.com/manticoresoftware/manticore/commit/0807240) исправлен сбой сервера при завершении работы
* [Commit 3e3a](https://github.com/manticoresoftware/manticore/commit/3e3acc3) исправлен show threads для системного blackhole потока
* [Commit 262c](https://github.com/manticoresoftware/manticore/commit/262c3fe) переработана проверка конфигурации iconv, исправления сборки на FreeBSD и Darwin

## Версия 2.4.1 GA, 16 октября 2017
### Новые возможности и улучшения
* Оператор OR в WHERE между фильтрами атрибутов
* Режим обслуживания ( SET MAINTENANCE=1)
* [CALL KEYWORDS](Searching/Autocomplete.md#CALL-KEYWORDS) доступны для распределённых индексов
* [Группировка по UTC](Server_settings/Searchd.md#grouping_in_utc)
* [query_log_mode](Server_settings/Searchd.md#query_log_mode) для управления правами доступа к файлам журнала
* Веса полей могут быть нулевыми или отрицательными
* [max_query_time](Searching/Options.md#max_query_time) теперь может влиять на полное сканирование
* добавлены [net_wait_tm](Server_settings/Searchd.md#net_wait_tm), [net_throttle_accept](Server_settings/Searchd.md#net_throttle_accept) и [net_throttle_action](Server_settings/Searchd.md#net_throttle_action) для тонкой настройки сетевого потока (при workers=thread_pool)
* COUNT DISTINCT работает с фасетными поисками
* IN можно использовать с JSON массивами float
* оптимизация мультизапросов больше не ломается при использовании выражений integer/float
* [SHOW META](Node_info_and_management/SHOW_META.md) показывает строку `multiplier` при использовании оптимизации мультизапросов

### Компиляция
Manticore Search собирается с помощью cmake, минимальная версия gcc для компиляции — 4.7.2.

### Папки и сервис
* Manticore Search запускается от пользователя `manticore`.
* Папка данных по умолчанию теперь `/var/lib/manticore/`.
* Папка логов по умолчанию теперь `/var/log/manticore/`.
* Папка с pid по умолчанию теперь `/var/run/manticore/`.

### Исправления ошибок
* [Commit a58c](https://github.com/manticoresoftware/manticore/commit/a58c619) исправлен оператор SHOW COLLATION, ломавший java-коннектор
* [Commit 631c](https://github.com/manticoresoftware/manticore/commit/631cf4e) исправлены сбои при обработке распределённых индексов; добавлены блокировки для хеша распределённого индекса; удалены операторы перемещения и копирования из агента
* [Commit 942b](https://github.com/manticoresoftware/manticore/commit/942bec0) исправлены сбои при обработке распределённых индексов из-за параллельных переподключений
* [Commit e5c1](https://github.com/manticoresoftware/manticore/commit/e5c1ed2) исправлен сбой обработчика сбоев при записи запроса в журнал сервера
* [Commit 4a4b](https://github.com/manticoresoftware/manticore/commit/4a4bda5) исправлен сбой с pooled атрибутами в мультизапросах
* [Commit 3873](https://github.com/manticoresoftware/manticore/commit/3873bfb) уменьшен размер core-файла за счёт предотвращения включения страниц индекса
* [Commit 11e6](https://github.com/manticoresoftware/manticore/commit/11e6254) исправлены сбои searchd при запуске при указании неверных агентов
* [Commit 4ca6](https://github.com/manticoresoftware/manticore/commit/4ca6350) исправлена ошибка в запросе sql_query_killlist репортера индексатора
* [Commit 123a](https://github.com/manticoresoftware/manticore/commit/123a9f0) исправлено fold_lemmas=1 в отношении подсчёта хитов
* [Commit cb99](https://github.com/manticoresoftware/manticore/commit/cb99164) исправлено непоследовательное поведение html_strip
* [Commit e406](https://github.com/manticoresoftware/manticore/commit/e406761) исправлена потеря новых настроек optimize rt index; исправлены утечки блокировок при optimize с опцией sync;
* [Commit 86ae](https://github.com/manticoresoftware/manticore/commit/86aeb82) исправлена обработка ошибочных мультизапросов
* [Commit 2645](https://github.com/manticoresoftware/manticore/commit/2645230) исправлена зависимость набора результатов от порядка мультизапросов
* [Commit 7239](https://github.com/manticoresoftware/manticore/commit/72395d9) исправлен сбой сервера при мультизапросе с некорректным запросом
* [Commit f353](https://github.com/manticoresoftware/manticore/commit/f353326) исправлен переход с разделяемой блокировки на эксклюзивную
* [Commit 3754](https://github.com/manticoresoftware/manticore/commit/3754785) исправлен сбой сервера при запросе без индексов
* [Commit 29f3](https://github.com/manticoresoftware/manticore/commit/29f360e) исправлена взаимная блокировка сервера

## Версия 2.3.3, 06 июля 2017
* Брендинг Manticore

