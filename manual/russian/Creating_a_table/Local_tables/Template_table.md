# Шаблонная таблица

<!-- example template -->
Шаблонная таблица — это особый тип таблицы в Manticore, который не сохраняет никаких данных и не создаёт файлы на вашем диске. Несмотря на это, она может иметь такие же [настройки обработки естественного языка (NLP)](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Natural-language-processing-specific-settings), как простая или таблица в реальном времени. Шаблонные таблицы могут использоваться для следующих целей:

* В качестве шаблона для унаследования настроек в [простом режиме](../../Creating_a_table/Local_tables.md#Defining-table-schema-in-config-%28Plain mode%29), что упрощает ваш конфигурационный файл Manticore.
* Генерация ключевых слов с помощью команды [CALL KEYWORDS](../../Searching/Autocomplete.md#CALL-KEYWORDS).
* Подсветка произвольной строки с помощью команды [CALL SNIPPETS](../../Searching/Highlighting.md#CALL-SNIPPETS).


<!-- intro -->
##### Создание шаблонной таблицы через конфигурационный файл:

<!-- request CONFIG -->

```ini
table template {
  type = template
  morphology = stem_en
  wordforms = wordforms.txt
  exceptions = exceptions.txt
  stopwords = stopwords.txt
}
```
<!-- end -->
<!-- proofread -->

