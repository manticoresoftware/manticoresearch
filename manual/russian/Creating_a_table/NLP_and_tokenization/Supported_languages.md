# Поддерживаемые языки

Manticore поддерживает широкий спектр языков, при этом базовая поддержка включена для большинства языков через `charset_table = non_cont` (что является значением по умолчанию). Также можно использовать опцию `non_cjk`, которая является синонимом для `non_cont`: `charset_table = non_cjk`.

Для многих языков Manticore предоставляет файл [стоп-слов](../../Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopwords), который можно использовать для улучшения релевантности поиска.

Кроме того, для некоторых языков доступна расширенная морфология, которая может значительно улучшить релевантность поиска, используя лемматизацию или алгоритмы стемминга на основе словарей для лучшей сегментации и нормализации.

В таблице ниже перечислены все поддерживаемые языки и указано, как включить:
* базовую поддержку (столбец "Supported")
* стоп-слова (столбец "Stopwords file name")
* расширенную морфологию (столбец "Advanced morphology")

| Язык | Supported | Stopwords file name | Advanced morphology | Примечания |
| - | - | - | - | - |
| Afrikaans | charset_table=non_cont | af | - | |
| Arabic | charset_table=non_cont | ar | morphology=stem_ar (арабский стеммер); morphology=libstemmer_ar  | |
| Armenian | charset_table=non_cont | hy | - | |
| Assamese | specify charset_table specify charset_table manually | - | - | |
| Basque | charset_table=non_cont | eu | - | |
| Bengali | charset_table=non_cont | bn | - | |
| Bishnupriya | specify charset_table manually | - | - | |
| Buhid | specify charset_table manually | - | - | |
| Bulgarian | charset_table=non_cont | bg | - | |
| Catalan | charset_table=non_cont | ca | morphology=libstemmer_ca | |
| Chinese using [ICU](https://icu.unicode.org/) | charset_table=chinese | zh | morphology=icu_chinese | Более точный, чем использование ngrams |
| Chinese using [Jieba](https://github.com/fxsjy/jieba) | charset_table=chinese | zh | morphology=jieba_chinese, требует пакет `manticore-language-packs` | Более точный, чем использование ngrams |
| Chinese using ngrams| ngram_chars=chinese | zh | ngram_chars=1 | Быстрая индексация, но производительность поиска может быть хуже |
| Croatian | charset_table=non_cont | hr | - | |
| Kurdish | charset_table=non_cont | ckb | - | |
| Czech | charset_table=non_cont | cz | morphology=stem_cz (чешский стеммер) | |
| Danish | charset_table=non_cont | da | morphology=libstemmer_da | |
| Dutch | charset_table=non_cont | nl | morphology=libstemmer_nl | |
| English | charset_table=non_cont | en | morphology=lemmatize_en (одна корневая форма); morphology=lemmatize_en_all (все корневые формы); morphology=stem_en (стеммер Портера для английского); morphology=stem_enru (стеммер Портера для английского и русского); morphology=libstemmer_en (английский из libstemmer)  | |
| Esperanto | charset_table=non_cont | eo | - | |
| Estonian | charset_table=non_cont | et | - | |
| Finnish | charset_table=non_cont | fi | morphology=libstemmer_fi | |
| French | charset_table=non_cont | fr | morphology=libstemmer_fr | |
| Galician | charset_table=non_cont | gl | - | |
| Garo | specify charset_table manually | - | - | |
| German | charset_table=non_cont | de | morphology=lemmatize_de (одна корневая форма); morphology=lemmatize_de_all (все корневые формы); morphology=libstemmer_de | |
| Greek | charset_table=non_cont | el | morphology=libstemmer_el | |
| Hebrew | charset_table=non_cont | he | - | |
| Hindi | charset_table=non_cont | hi | morphology=libstemmer_hi | |
| Hmong | specify charset_table manually | - | - | |
| Ho | specify charset_table manually | - | - | |
| Hungarian | charset_table=non_cont | hu | morphology=libstemmer_hu | |
| Indonesian | charset_table=non_cont | id | morphology=libstemmer_id | |
| Irish | charset_table=non_cont | ga | morphology=libstemmer_ga | |
| Italian | charset_table=non_cont | it | morphology=libstemmer_it | |
| Japanese | ngram_chars=japanese | - | ngram_chars=japanese ngram_len=1 | Требуется сегментация на основе ngram |
| Komi | specify charset_table manually | - | - | |
| Korean | ngram_chars=korean | - | ngram_chars=korean ngram_len=1 | Требуется сегментация на основе ngram  |
| Large Flowery Miao | specify charset_table manually | - | - | |
| Latin | charset_table=non_cont | la | - | |
| Latvian | charset_table=non_cont | lv | - | |
| Lithuanian | charset_table=non_cont | lt | morphology=libstemmer_lt | |
| Maba | specify charset_table manually | - | - | |
| Maithili | specify charset_table manually | - | - | |
| Marathi | specify charset_table manually | - | - | |
| Marathi | charset_table=non_cont | mr | - | |
| Mende | specify charset_table manually | - | - | |
| Mru | specify charset_table manually | - | - | |
| Myene | specify charset_table manually | - | - | |
| Nepali | specify charset_table manually | - | morphology=libstemmer_ne | |
| Ngambay | specify charset_table manually | - | - | |
| Norwegian | charset_table=non_cont | no | morphology=libstemmer_no | |
| Odia | specify charset_table manually | - | - | |
| Persian | charset_table=non_cont | fa | - | |
| Polish | charset_table=non_cont | pl | - | |
| Portuguese | charset_table=non_cont | pt | morphology=libstemmer_pt | |
| Romanian | charset_table=non_cont | ro | morphology=libstemmer_ro | |
| Russian | charset_table=non_cont | ru | morphology=lemmatize_ru (одна корневая форма); morphology=lemmatize_ru_all (все корневые формы); morphology=stem_ru (стеммер Портера для русского); morphology=stem_enru (стеммер Портера для английского и русского); morphology=libstemmer_ru (из libstemmer) | |
| Santali | specify charset_table manually | - | - | |
| Sindhi | specify charset_table manually | - | - | |
| Slovak | charset_table=non_cont | sk | - | |
| Slovenian | charset_table=non_cont | sl | - | |
| Somali | charset_table=non_cont | so | - | |
| Sotho | charset_table=non_cont | st | - | |
| Spanish | charset_table=non_cont | es | morphology=libstemmer_es | |
| Swahili | charset_table=non_cont | sw | - | |
| Swedish | charset_table=non_cont | sv | morphology=libstemmer_sv | |
| Sylheti | specify charset_table manually | - | - | |
| Tamil | specify charset_table manually | - | morphology=libstemmer_ta | |
| Thai | charset_table=thai | th | - | |
| Turkish | charset_table=non_cont | tr | morphology=libstemmer_tr | |
| Ukrainian | charset_table=non_cont,U+0406->U+0456,U+0456,U+0407->U+0457,U+0457,U+0490->U+0491,U+0491   | - | morphology=lemmatize_uk_all | Требует [установки](../../Installation/Debian_and_Ubuntu.md#Ukrainian-lemmatizer) украинского лемматизатора |
| Yoruba | charset_table=non_cont | yo | - | |
| Zulu | charset_table=non_cont | zu | - |  |
<!-- proofread -->

