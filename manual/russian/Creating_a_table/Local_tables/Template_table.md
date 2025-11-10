# Таблица шаблона

<!-- example template -->
 Таблица шаблона — это особый тип таблицы в Manticore, который не хранит никаких данных и не создает файлов на вашем диске. Несмотря на это, она может иметь те же [настройки NLP](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Natural-language-processing-specific-settings), что и обычная или таблица реального времени. Таблицы шаблонов могут использоваться для следующих целей:

* В качестве шаблона для наследования настроек в [Plain режиме](../../Creating_a_table/Local_tables.md#Defining-table-schema-in-config-%28Plain mode%29), упрощая ваш конфигурационный файл Manticore.
* Генерация ключевых слов с помощью команды [CALL KEYWORDS](../../Searching/Autocomplete.md#CALL-KEYWORDS).
* Подсветка произвольной строки с помощью команды [CALL SNIPPETS](../../Searching/Highlighting.md#CALL-SNIPPETS).


<!-- intro -->
##### Создание таблицы шаблона через конфигурационный файл:

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

