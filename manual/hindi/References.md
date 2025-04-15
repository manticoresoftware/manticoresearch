# संदर्भ


### SQL आदेश

##### स्कीमा प्रबंधन

* [CREATE TABLE](Creating_a_table/Local_tables/Real-time_table.md#CREATE-TABLE-command:) - नए तालिका का निर्माण करता है

* [CREATE TABLE LIKE](Creating_a_table/Local_tables/Real-time_table.md#CREATE-TABLE-LIKE:) - एक अन्य तालिका को टेम्पलेट के रूप में उपयोग करके तालिका का निर्माण करता है

* [CREATE TABLE LIKE ... WITH DATA](Creating_a_table/Local_tables/Real-time_table.md#CREATE-TABLE-LIKE:) - एक तालिका की नकल करता है

* [CREATE SOURCE](Integration/Kafka.md#Source) - Kafka उपभोक्ता स्रोत बनाता है

* [CREATE MATERIALIZED VIEW](Integration/Kafka.md#Materialized-view) - Kafka संदेशों से डेटा रूपांतरण

* [CREATE MV](Integration/Kafka.md#Materialized-view) - पहले जैसा ही

* [DESCRIBE](Listing_tables.md#DESCRIBE) - तालिका के फ़ील्ड सूची और उनके प्रकारों को प्रिंट करता है

* [ALTER TABLE](Updating_table_schema_and_settings.md) - तालिका स्कीमा / सेटिंग्स में परिवर्तन करता है

* [ALTER TABLE REBUILD SECONDARY](Updating_table_schema_and_settings.md#Rebuilding-a-secondary-index) - माध्यमिक इंडेक्स को अपडेट/पुनः प्राप्त करता है

* [ALTER TABLE type='distributed'](Updating_table_schema_and_settings.md#Changing-a-distributed-table) - माध्यमिक इंडेक्स को अपडेट/पुनः प्राप्त करता है

* [ALTER TABLE RENAME](Updating_table_schema_and_settings.md#Renaming-a-real-time-table)

* [ALTER MATERIALIZED VIEW {name} suspended=1](Integration/Kafka.md#Altering-materialized-views) - Kafka स्रोत से उपभोग को निलंबित या फिर से शुरू करें

* [DROP TABLE IF EXISTS](Deleting_a_table.md#Deleting-a-table) - तालिका को हटाता है (यदि यह मौजूदा है)

* [SHOW TABLES](Listing_tables.md#DESCRIBE) - तालिकाओं की सूची दिखाता है

* [SHOW SOURCES](Integration/Kafka.md#Listing) - Kafka स्रोतों की सूची दिखाता है

* [SHOW MATERIALIZED VIEWS](Integration/Kafka.md#Listing) - बनाए गए दृश्य की सूची दिखाता है

* [SHOW MVS](Integration/Kafka.md#Listing) - पिछले आदेश का उपनाम

* [SHOW CREATE TABLE](Listing_tables.md#DESCRIBE) - तालिका बनाने के लिए SQL आदेश दिखाता है

* [SHOW TABLE INDEXES](Node_info_and_management/Table_settings_and_status/SHOW_TABLE_INDEXES.md) - तालिका के लिए उपलब्ध माध्यमिक इंडेक्स के बारे में जानकारी प्रदर्शित करता है

* [SHOW TABLE STATUS](Node_info_and_management/Table_settings_and_status/SHOW_TABLE_STATUS.md) - वर्तमान तालिका स्थिति के बारे में जानकारी दिखाता है

* [SHOW TABLE SETTINGS](Node_info_and_management/Table_settings_and_status/SHOW_TABLE_SETTINGS.md) - तालिका सेटिंग्स दिखाता है

* [SHOW LOCKS](Securing_and_compacting_a_table/Freezing_a_table.md#SHOW-LOCKS) - जमा तालिकाओं के बारे में जानकारी दिखाता है


##### डेटा प्रबंधन

* [INSERT](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md) - नए दस्तावेज़ जोड़ता है

* [REPLACE](Data_creation_and_modification/Updating_documents/REPLACE.md) - मौजूदा दस्तावेज़ों को नए से प्रतिस्थापित करता है

* [REPLACE .. SET](Data_creation_and_modification/Updating_documents/REPLACE.md?client=REPLACE+SET) - तालिका में एक या एकाधिक क्षेत्रों को प्रतिस्थापित करता है

* [UPDATE](Data_creation_and_modification/Updating_documents/UPDATE.md) - दस्तावेज़ों में इन-प्लेस अपडेट करता है

* [DELETE](Data_creation_and_modification/Deleting_documents.md) - दस्तावेज़ों को हटाता है

* [TRUNCATE TABLE](Emptying_a_table.md) - तालिका से सभी दस्तावेज़ों को हटाता है


##### बैकअप

* [BACKUP](Securing_and_compacting_a_table/Backup_and_restore.md#BACKUP-SQL-command-reference) - आपकी तालिकाओं का बैकअप लेता है


##### SELECT

* [SELECT](Searching/Full_text_matching/Basic_usage.md#SQL) - खोजता है

  * [WHERE](Searching/Filters.md#Filters) - फ़िल्टर करता है

  * [GROUP BY](Searching/Grouping.md) - खोज परिणामों को समूहबद्ध करता है

  * [GROUP BY ORDER](Searching/Grouping.md) - समूहों को क्रमबद्ध करता है

  * [GROUP BY HAVING](Searching/Grouping.md) - समूहों को फ़िल्टर करता है

  * [OPTION](Searching/Options.md#OPTION) - क्वेरी विकल्प

  * [FACET](Searching/Faceted_search.md) - डेटा समुच्चय के लिए

  * [SUB-SELECTS](Searching/Sub-selects.md) - SELECT उप-वैश्य के उपयोग के बारे में

  * [JOIN](Searching/Joining.md) - SELECT में तालिकाओं को जोड़ता है

* [EXPLAIN QUERY](Searching/Full_text_matching/Profiling.md#Profiling-without-running-a-query) - क्वेरी निष्पादन योजना को दिखाता है बिना क्वेरी चलाए

* [SHOW META](Node_info_and_management/SHOW_META.md) - निष्पादित क्वेरी के बारे में विस्तृत जानकारी दिखाता है

* [SHOW PROFILE](Node_info_and_management/Profiling/Query_profile.md) - निष्पादित क्वेरी के बारे में प्रोफाइलिंग जानकारी दिखाता है

* [SHOW PLAN](Searching/Full_text_matching/Profiling.md#Profiling-the-query-tree-in-SQL) - क्वेरी निष्पादन योजना को दिखाता है जब क्वेरी का निष्पादन किया गया हो

* [SHOW WARNINGS](Node_info_and_management/SHOW_WARNINGS.md) - नवीनतम क्वेरी से चेतावनियाँ दिखाता है


##### फ्लशिंग विभिन्न चीजें

* [FLUSH ATTRIBUTES](Securing_and_compacting_a_table/Flushing_attributes.md) - अद्यतन विशेषताओं को डिस्क में फ्लश करने के लिए मजबूर करता है

* [FLUSH HOSTNAMES](Securing_and_compacting_a_table/Flushing_hostnames.md) - एजेंट होस्ट नामों से संबद्ध आईपी को नवीनीकरण करता है

* [FLUSH LOGS](Logging/Rotating_query_and_server_logs.md) - खोजद लॉग और क्वेरी लॉग फ़ाइलों को फिर से खोलने की प्रक्रिया शुरू करता है (USR1 के समान)


##### रीयल-टाइम तालिका ऑप्टिमाइजेशन

* [FLUSH RAMCHUNK](Securing_and_compacting_a_table/Flushing_RAM_chunk_to_a_new_disk_chunk.md#FLUSH-RAMCHUNK) - नए डिस्क चंक का निर्माण करने के लिए मजबूर करता है

* [FLUSH TABLE](Securing_and_compacting_a_table/Flushing_RAM_chunk_to_disk.md#FLUSH-TABLE) - रीयल-टाइम तालिका के RAM चंक को डिस्क में फ्लश करता है

* [OPTIMIZE TABLE](Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE) - रीयल-टाइम तालिका के अनुकूलन के लिए कतार लगाता है


##### रीयल-टाइम तालिका में आयात करना

* [ATTACH TABLE](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Attaching_one_table_to_another.md) - एक साधारण तालिका से रीयल-टाइम तालिका में डेटा स्थानांतरित करता है

* [IMPORT TABLE](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Importing_table.md) - पहले से बनाए गए RT या PQ तालिका को RT मोड में चल रहे सर्वर में आयात करता है


##### पुनरावृत्ति

* [JOIN CLUSTER](Creating_a_cluster/Setting_up_replication/Joining_a_replication_cluster.md) - एक पुनरावृत्ति क्लस्टर में शामिल होता है
* [ALTER CLUSTER](Creating_a_cluster/Setting_up_replication/Managing_replication_nodes.md) - एक प्रतिकृति क्लस्टर में एक तालिका जोड़ता/हटाता है
* [SET CLUSTER](Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Cluster-parameters) - प्रतिकृति क्लस्टर सेटिंग्स बदलता है
* [DELETE CLUSTER](Creating_a_cluster/Setting_up_replication/Deleting_a_replication_cluster.md) - एक प्रतिकृति क्लस्टर को हटाता है

##### Plain table rotate
* [RELOAD TABLE](Data_creation_and_modification/Adding_data_from_external_storages/Rotating_a_table.md#RELOAD-TABLE) - एक साधारण तालिका को घुमाता है
* [RELOAD TABLES](Data_creation_and_modification/Adding_data_from_external_storages/Rotating_a_table.md#RELOAD-TABLE) - सभी साधारण तालिकाओं को घुमाता है

##### Transactions
* [BEGIN](Data_creation_and_modification/Transactions.md#BEGIN,-COMMIT,-and-ROLLBACK) - एक लेनदेन शुरू करता है
* [COMMIT](Data_creation_and_modification/Transactions.md#BEGIN,-COMMIT,-and-ROLLBACK) - एक लेनदेन समाप्त करता है
* [ROLLBACK](Data_creation_and_modification/Transactions.md#BEGIN,-COMMIT,-and-ROLLBACK) - एक लेनदेन को वापस ले जाता है

##### CALL
* [CALL SUGGEST, CALL QSUGGEST](Searching/Spell_correction.md#CALL-QSUGGEST,-CALL-SUGGEST) - सही वर्तनी वाले शब्दों का सुझाव देता है
* [CALL SNIPPETS](Searching/Highlighting.md) - प्रदान किए गए डेटा और प्रश्न से एक हाइलाइटेड परिणाम स्निपेट बनाता है
* [CALL PQ](Searching/Percolate_query.md) - एक पर्कोलेट क्वेरी चलाता है
* [CALL KEYWORDS](Searching/Autocomplete.md#CALL-KEYWORDS) - यह चेक करने के लिए उपयोग किया जाता है कि कीवर्ड को कैसे टोकन किया गया है। यह प्रदान किए गए कीवर्ड के टोकनयुक्त रूपों को पुनर्प्राप्त करने की अनुमति भी देता है
* [CALL AUTOCOMPLETE](Searching/Autocomplete.md#CALL-AUTOCOMPLETE) - आपके खोज प्रश्न को स्वतः पूर्ण करता है

##### Plugins
* [CREATE FUNCTION](Extensions/UDFs_and_Plugins/UDF/Creating_a_function.md) - एक उपयोगकर्ता-परिभाषित फ़ंक्शन (UDF) स्थापित करता है
* [DROP FUNCTION](Extensions/UDFs_and_Plugins/UDF/Deleting_a_function.md) - एक उपयोगकर्ता-परिभाषित फ़ंक्शन (UDF) को हटाता है
* [CREATE PLUGIN](Extensions/UDFs_and_Plugins/Plugins/Creating_a_plugin.md) - एक प्लगइन स्थापित करता है
* [CREATE BUDDY PLUGIN](Extensions/UDFs_and_Plugins/Plugins/Creating_a_plugin.md#CREATE-BUDDY-PLUGIN) - एक बडी प्लगइन स्थापित करता है
* [DROP PLUGIN](Extensions/UDFs_and_Plugins/Plugins/Deleting_a_plugin.md#DELETE-PLUGIN) - एक प्लगइन को हटाता है
* [DROP BUDDY PLUGIN](Extensions/UDFs_and_Plugins/Plugins/Deleting_a_plugin.md#DELETE-BUDDY-PLUGIN) - एक बडी प्लगइन को हटा देता है
* [RELOAD PLUGINS](Extensions/UDFs_and_Plugins/Plugins/Reloading_plugins.md) - दिए गए पुस्तकालय से सभी प्लगइन्स को फिर से लोड करता है
* [ENABLE BUDDY PLUGIN](Extensions/UDFs_and_Plugins/Plugins/Enabling_and_disabling_buddy_plugins.md#ENABLE-BUDDY-PLUGIN) - पहले निष्क्रिय किए गए बडी प्लगइन को पुनः सक्रिय करता है
* [DISABLE BUDDY PLUGIN](Extensions/UDFs_and_Plugins/Plugins/Enabling_and_disabling_buddy_plugins.md#DISABLE-BUDDY-PLUGIN) - एक सक्रिय बडी प्लगइन को निष्क्रिय करता है

##### Server status
* [SHOW STATUS](Node_info_and_management/Node_status.md#SHOW-STATUS) - कई उपयोगी प्रदर्शन काउंटर प्रदर्शित करता है
* [SHOW THREADS](Node_info_and_management/SHOW_THREADS.md) - सभी वर्तमान सक्रिय ग्राहक थ्रेड की सूची करता है
* [SHOW VARIABLES](Node_info_and_management/SHOW_VARIABLES.md) - सर्वर-व्यापी चर और उनके मानों की लिस्ट बनाता है
* [SHOW VERSION](Node_info_and_management/SHOW_VERSION.md#SHOW-VERSION) - इंसटेंस के विभिन्न घटकों की विस्तृत संस्करण जानकारी प्रदान करता है.

### HTTP endpoints
* [/sql](Connecting_to_the_server/HTTP.md#SQL-over-HTTP) - HTTP JSON के माध्यम से एक SQL बयान निष्पादित करें
* [/cli](Connecting_to_the_server/HTTP.md#/cli) - एक HTTP कमांड लाइन इंटरफ़ेस प्रदान करता है
* [/insert](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md) - एक वास्तविक समय की तालिका में एक दस्तावेज़ सम्मिलित करता है
* [/pq/tbl_name/doc](Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md#Adding-rules-to-a-percolate-table) - एक पर्कोलेट तालिका में एक PQ नियम जोड़ता है
* [/update](Data_creation_and_modification/Updating_documents/UPDATE.md#Updates-via-HTTP-JSON) - एक वास्तविक समय की तालिका में एक दस्तावेज़ को अपडेट करता है
* [/replace](Data_creation_and_modification/Updating_documents/REPLACE.md) - एक मौजूदा दस्तावेज़ को एक वास्तविक समय की तालिका में बदलता है या अगर यह मौजूद नहीं है तो इसे सम्मिलित करता है
* [/pq/tbl_name/doc/N?refresh=1](Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md#Adding-rules-to-a-percolate-table) - एक पर्कोलेट तालिका में एक PQ नियम को बदलता है
* [/delete](Data_creation_and_modification/Deleting_documents.md) - एक तालिका से एक दस्तावेज़ को हटा देता है
* [/bulk](Data_creation_and_modification/Updating_documents/UPDATE.md#Bulk-updates) - एक ही कॉल में कई सम्मिलित, अपडेट, या हटाने का संचालन करता है। बल्क सम्मिलन के बारे में [यहाँ](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real_time_table.md) और जानें।
* [/search](Searching/Full_text_matching/Basic_usage.md#HTTP-JSON) - एक खोज करता है
* [/search -> knn](Searching/KNN.md) - एक KNN वेक्टर खोज करता है
* [/pq/tbl_name/search](Searching/Percolate_query.md) - एक पर्कोलेट तालिका में एक विपरीत खोज करता है
* [/tbl_name/_mapping](Creating_a_table/Local_tables/Real-time_table.md#_mapping-API:) - Elasticsearch शैली में एक तालिका स्कीमा बनाता है

### Common things
* [field name syntax](Creating_a_table/Data_types.md#Field-name-syntax)
* [data types](Creating_a_table/Data_types.md)
* [engine](Creating_a_table/Data_types.md)
* [plain mode](Read_this_first.md#Real-time-mode-vs-plain-mode)
* [real-time mode](Read_this_first.md#Real-time-mode-vs-plain-mode)

##### Common table settings
* [access_plain_attrs](Server_settings/Searchd.md#access_plain_attrs)
* [access_blob_attrs](Server_settings/Searchd.md#access_blob_attrs)
* [access_doclists](Server_settings/Searchd.md#access_doclists)
* [access_hitlists](Server_settings/Searchd.md#access_hitlists)
* [access_dict](Server_settings/Searchd.md#access_dict)
* [attr_update_reserve](Data_creation_and_modification/Updating_documents/UPDATE.md#attr_update_reserve)
* [bigram_freq_words](Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#bigram_freq_words)
* [bigram_index](Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#bigram_index)
* [blend_chars](Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#blend_chars)
* [blend_mode](Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#blend_mode)
* [charset_table](Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table)
* [dict](Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)
* [docstore_block_size](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [docstore_compression](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [docstore_compression_level](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [embedded_limit](Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#embedded_limit)
* [exceptions](Creating_a_table/NLP_and_tokenization/Exceptions.md#exceptions)
* [expand_keywords](Searching/Options.md#expand_keywords)
* [global_idf](Searching/Options.md#global_idf)
* [hitless_words](Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#hitless_words)
* [html_index_attrs](Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#html_index_attrs)
* [html_remove_elements](Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#html_remove_elements)
* [html_strip](Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#html_strip)
* [ignore_chars](Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ignore_chars)
* [index_exact_words](Creating_a_table/NLP_and_tokenization/Morphology.md#index_exact_words)
* [index_field_lengths](Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#index_field_lengths)
* [index_sp](Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#index_sp)
* [index_token_filter](Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#index_token_filter)
* [index_zones](Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#index_zones)
* [infix_fields](Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#infix_fields)
* [inplace_enable](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [inplace_hit_gap](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [inplace_reloc_factor](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [inplace_write_factor](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [jieba_hmm](Creating_a_table/NLP_and_tokenization/Morphology.md#jieba_hmm)
* [jieba_mode](Creating_a_table/NLP_and_tokenization/Morphology.md#jieba_mode)
* [jieba_user_dict_path](Creating_a_table/NLP_and_tokenization/Morphology.md#jieba_user_dict_path)
* [killlist_target](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [max_substring_len](Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#max_substring_len)
* [min_infix_len](Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len)
* [min_prefix_len](Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_prefix_len)
* [min_stemming_len](Creating_a_table/NLP_and_tokenization/Morphology.md#min_stemming_len)
* [min_word_len](Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#min_word_len)
* [morphology](Searching/Options.md#morphology)
* [morphology_skip_fields](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology_skip_fields)
* [ngram_chars](Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_chars)
* [ngram_len](Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_len)
* [overshort_step](Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#overshort_step)
* [path](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [phrase_boundary](Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#phrase_boundary)
* [phrase_boundary_step](Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#phrase_boundary_step)
* [prefix_fields](Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#prefix_fields)
* [preopen](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [read_buffer_docs](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [read_buffer_hits](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [regexp_filter](Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#regexp_filter)
* [stopwords](Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopwords)
* [stopword_step](Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopword_step)
* [stopwords_unstemmed](Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopwords_unstemmed)
* [type](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [wordforms](Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms)

##### Plain table settings
* [json_secondary_indexes](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#json_secondary_indexes)
* [source](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [stored_fields](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [stored_only_fields](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [columnar_attrs](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)

##### वितरित तालिका सेटिंग्स
* [local](Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md)
* [agent](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent)
* [agent_connect_timeout](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent)
* [agent_blackhole](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent)
* [agent_persistent](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent)
* [agent_query_timeout](Searching/Options.md#agent_query_timeout)
* [agent_retry_count](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent)
* [ha_strategy](Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_strategy)
* [mirror_retry_count](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent)

##### RT तालिका सेटिंग्स
* [rt_attr_bigint](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [rt_attr_bool](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [rt_attr_float](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [rt_attr_float_vector](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [rt_attr_json](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [rt_attr_multi_64](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [rt_attr_multi](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [rt_attr_string](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [rt_attr_timestamp](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [rt_attr_uint](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [rt_field](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [rt_mem_limit](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [diskchunk_flush_write_timeout](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)
* [diskchunk_flush_search_timeout](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#General-syntax-of-CREATE-TABLE)


## पूर्ण-टेक्स्ट खोज ऑपरेटर
* [OR](Searching/Full_text_matching/Operators.md#OR-operator)
* [MAYBE](Searching/Full_text_matching/Operators.md#MAYBE-operator)
* [NOT](Searching/Full_text_matching/Operators.md#Negation-operator) - NOT ऑपरेटर
* [@field](Searching/Full_text_matching/Operators.md#Field-search-operator) - फ़ील्ड खोज ऑपरेटर
* [@field%5BN%5D](Searching/Full_text_matching/Operators.md#Field-search-operator) - फ़ील्ड स्थिति सीमा संशोधक
* [@(field1,field2,...)](Searching/Full_text_matching/Operators.md#Field-search-operator) - मल्टी-फील्ड खोज ऑपरेटर
* [@!field](Searching/Full_text_matching/Operators.md#Field-search-operator) - अनदेखा फ़ील्ड खोज ऑपरेटर
* [@!(field1,field2,...)](Searching/Full_text_matching/Operators.md#Field-search-operator) - अनदेखा मल्टी-फील्ड खोज ऑपरेटर
* [@*](Searching/Full_text_matching/Operators.md#Field-search-operator) - सभी-फ़ील्ड खोज ऑपरेटर
* ["word1 word2 ... "](Searching/Full_text_matching/Operators.md#Phrase-search-operator) - वाक्यांश खोज ऑपरेटर
* ["word1 word2 ... "~N](Searching/Full_text_matching/Operators.md#Proximity-search-operator) - समीपता खोज ऑपरेटर
* ["word1 word2 ... "/N](Searching/Full_text_matching/Operators.md#Quorum-matching-operator) - कुल मिलाकर मिलान ऑपरेटर
* [word1 << word2 << word3](Searching/Full_text_matching/Operators.md#Strict-order-operator) - सख्त क्रम ऑपरेटर
* [=word1](Searching/Full_text_matching/Operators.md#Exact-form-modifier) - सही रूप संशोधक
* [^word1](Searching/Full_text_matching/Operators.md#Field-start-and-field-end-modifier) - फ़ील्ड-शुरुआत संशोधक
* [word2$](Searching/Full_text_matching/Operators.md#Field-start-and-field-end-modifier) - फ़ील्ड-समापन संशोधक
* [word^N](Searching/Full_text_matching/Operators.md#IDF-boost-modifier) - कीवर्ड IDF बूस्ट संशोधक
* [word1 NEAR/N word2](Searching/Full_text_matching/Operators.md#NEAR-operator) - NEAR, सामान्यीकृत समीपता ऑपरेटर
* [word1 NOTNEAR/N word2](Searching/Full_text_matching/Operators.md#NOTNEAR-operator) - NOTNEAR, नकारात्मक अभिव्यक्ति ऑपरेटर
* [word1 PARAGRAPH word2 PARAGRAPH "word3 word4"](Searching/Full_text_matching/Operators.md#SENTENCE-and-PARAGRAPH-operators) - PARAGRAPH ऑपरेटर
* [word1 SENTENCE word2 SENTENCE "word3 word4"](Searching/Full_text_matching/Operators.md#SENTENCE-and-PARAGRAPH-operators) - SENTENCE ऑपरेटर
* [ZONE:(h3,h4)](Searching/Full_text_matching/Operators.md#ZONE-limit-operator) - ZONE सीमा ऑपरेटर
* [ZONESPAN:(h2)](Searching/Full_text_matching/Operators.md#ZONESPAN-limit-operator) - ZONESPAN सीमा ऑपरेटर
* [@@relaxed](Searching/Full_text_matching/Operators.md#Field-search-operator) - अनुपस्थित फ़ील्ड के बारे में त्रुटियों को दबाने के लिए
* [t?st](Searching/Full_text_matching/Operators.md#Wildcard-operators) - वाइल्डकार्ड ऑपरेटर
* [REGEX(/pattern/)](Searching/Full_text_matching/Operators.md#REGEX-operator) - REGEX ऑपरेटर

## कार्य
##### गणितीय
* [ABS()](Functions/Mathematical_functions.md#ABS%28%29) - निरपेक्ष मान लौटाता है
* [ATAN2()](Functions/Mathematical_functions.md#ATAN2%28%29) - दो तर्कों का आर्कटांगेंट फ़ंक्शन लौटाता है
* [BITDOT()](Functions/Mathematical_functions.md#BITDOT%28%29) - प्रत्येक बिट के उत्पादों का योग लौटाता है जो एक मास्क के साथ इसके वजन से गुणा किया गया है
* [CEIL()](Functions/Mathematical_functions.md#CEIL%28%29) - तर्क से अधिक या उसके बराबर सबसे छोटा पूर्णांक मूल्य लौटाता है
* [COS()](Functions/Mathematical_functions.md#COS%28%29) - तर्क का कोसाइन लौटाता है
* [CRC32()](Functions/Mathematical_functions.md#CRC32%28%29) - तर्क का CRC32 मान लौटाता है
* [EXP()](Functions/Mathematical_functions.md#EXP%28%29) - तर्क का गुणांक लौटाता है
* [FIBONACCI()](Functions/Mathematical_functions.md#FIBONACCI%28%29) - N-वे फिबोनाच्ची संख्या लौटाता है, जहाँ N पूर्णांक तर्क है
* [FLOOR()](Functions/Mathematical_functions.md#FLOOR%28%29) - तर्क से कम या उसके बराबर सबसे बड़ा पूर्णांक मूल्य लौटाता है
* [GREATEST()](Functions/Mathematical_functions.md#GREATEST%28%29) - JSON/MVA array को तर्क के रूप में लेता है और उस array में सबसे बड़ा मान लौटाता है
* [IDIV()](Functions/Mathematical_functions.md#IDIV%28%29) - पहले तर्क का दूसरे तर्क द्वारा पूर्णांक विभाजन का परिणाम लौटाता है
* [LEAST()](Functions/Mathematical_functions.md#LEAST%28%29) - JSON/MVA array को तर्क के रूप में लेता है, और उस array में सबसे छोटा मान लौटाता है
* [LN()](Functions/Mathematical_functions.md#LN%28%29) - तर्क का प्राकृतिक लघुगणक लौटाता है
* [LOG10()](Functions/Mathematical_functions.md#LOG10%28%29) - तर्क का सामान्य लघुगणक लौटाता है
* [LOG2()](Functions/Mathematical_functions.md#LOG2%28%29) - तर्क का द्विआधारी लघुगणक लौटाता है
* [MAX()](Functions/Mathematical_functions.md#MAX%28%29) - दो तर्कों में से बड़े को लौटाता है
* [MIN()](Functions/Mathematical_functions.md#MIN%28%29) - दो तर्कों में से छोटे को लौटाता है
* [POW()](Functions/Mathematical_functions.md#POW%28%29) - पहले तर्क को दूसरे तर्क के शक्ति में उठाता है
* [RAND()](Functions/Mathematical_functions.md#RAND%28%29) - 0 और 1 के बीच यादृच्छिक फ्लोट लौटाता है
* [SIN()](Functions/Mathematical_functions.md#SIN%28%29) - तर्क का साइन लौटाता है
* [SQRT()](Functions/Mathematical_functions.md#SQRT%28%29) - तर्क की वर्गमूल लौटाता है


##### खोज और रैंकिंग
* [BM25F()](Functions/Searching_and_ranking_functions.md#BM25F%28%29) - सटीक BM25F सूत्र मान लौटाता है
* [EXIST()](Functions/Searching_and_ranking_functions.md#EXIST%28%29) - गैर-उपस्थित कॉलम को डिफ़ॉल्ट मानों के साथ प्रतिस्थापित करता है
* [GROUP_CONCAT()](Searching/Grouping.md#GROUP_CONCAT%28field%29) - समूह में सभी दस्तावेज़ों के गुणों के मानों की एक अल्पविराम से अलग सूची बनाता है
* [HIGHLIGHT()](Searching/Highlighting.md) - खोज परिणामों को हाईलाइट करता है
* [MIN_TOP_SORTVAL()](Functions/Searching_and_ranking_functions.md#MIN_TOP_SORTVAL%28%29) - वर्तमान शीर्ष-N मेलों में सबसे खराब पाए गए तत्व का क्रम कुंजी मान लौटाता है
* [MIN_TOP_WEIGHT()](Functions/Searching_and_ranking_functions.md#MIN_TOP_WEIGHT%28%29) - वर्तमान शीर्ष-N मेलों में सबसे खराब पाए गए तत्व का वजन लौटाता है
* [PACKEDFACTORS()](Functions/Searching_and_ranking_functions.md#PACKEDFACTORS%28%29) - वजन कारकों का आउटपुट
* [REMOVE_REPEATS()](Functions/Searching_and_ranking_functions.md#REMOVE_REPEATS%28%29) - एक ही 'कॉलम' मान के साथ दोहराए गए समायोजित पंक्तियों को हटा देता है
* [WEIGHT()](Functions/Searching_and_ranking_functions.md#WEIGHT%28%29) - पूर्णपाठ मिलान स्कोर लौटाता है
* [ZONESPANLIST()](Functions/Searching_and_ranking_functions.md#ZONESPANLIST%28%29) - मेल खाने वाले ज़ोन स्पैन के जोड़ लौटाता है
* [QUERY()](Functions/Searching_and_ranking_functions.md#QUERY%28%29) - वर्तमान पूर्ण-पाठ खोज लौटाता है

##### प्रकार Casting
* [BIGINT()](Functions/Type_casting_functions.md#BIGINT%28%29) - पूर्णांक तर्क को 64-बिट प्रकार में बलात् बढ़ाता है
* [DOUBLE()](Functions/Type_casting_functions.md#DOUBLE%28%29) - दिए गए तर्क को फ़्लोटिंग पॉइंट प्रकार में बलात् बढ़ाता है
* [INTEGER()](Functions/Type_casting_functions.md#INTEGER%28%29) - दिए गए तर्क को 64-बिट साइनड प्रकार में बलात् बढ़ाता है
* [TO_STRING()](Functions/Type_casting_functions.md#TO_STRING%28%29) - तर्क को स्ट्रिंग प्रकार में बलात् बढ़ाता है
* [UINT()](Functions/Type_casting_functions.md#UINT%28%29) - दिए गए तर्क को 32-बिट बिना साइन वाला पूर्णांक प्रकार में परिवर्तित करता है
* [UINT64()](Functions/Type_casting_functions.md#UINT64%28%29) - दिए गए तर्क को 64-बिट बिना साइन वाला पूर्णांक प्रकार में परिवर्तित करता है
* [SINT()](Functions/Type_casting_functions.md#SINT%28%29) - 32-बिट बिना साइन पूर्णांक को साइन किया हुआ 64-बिट पूर्णांक के रूप में व्याख्या करता है

##### Arrays और शर्तें
* [ALL()](Functions/Arrays_and_conditions_functions.md#ALL%28%29) - 1 लौटाता है यदि शर्त array के सभी तत्वों के लिए सत्य है
* [ANY()](Functions/Arrays_and_conditions_functions.md#ANY%28%29) - 1 लौटाता है यदि शर्त array के किसी भी तत्व के लिए सत्य है
* [CONTAINS()](Functions/Arrays_and_conditions_functions.md#CONTAINS%28%29) - जाँच करता है कि (x,y) बिंदु दिए गए बहुभुज के भीतर है या नहीं
* [IF()](Functions/Arrays_and_conditions_functions.md#IF%28%29) - जाँच करता है कि 1स्ट तर्क 0.0 के बराबर है, यदि यह शून्य नहीं है तो 2ंड तर्क लौटाता है या जब यह हो तो 3रा लौटाता है
* [IN()](Functions/Arrays_and_conditions_functions.md#IN%28%29) - 1 लौटाता है यदि पहला तर्क किसी अन्य तर्क के बराबर है, अन्यथा 0 लौटाता है
* [INDEXOF()](Functions/Arrays_and_conditions_functions.md#INDEXOF%28%29) - array में सभी तत्वों के माध्यम से इटरेट करता है और पहले मेल खाने वाले तत्व का अनुक्रमांक लौटाता है
* [INTERVAL()](Functions/Arrays_and_conditions_functions.md#INTERVAL%28%29) - पहले तर्क से कम तर्क का अनुक्रमांक लौटाता है
* [LENGTH()](Functions/Arrays_and_conditions_functions.md#LENGTH%28%29) - MVA में तत्वों की संख्या लौटाता है
* [REMAP()](Functions/Arrays_and_conditions_functions.md#REMAP%28%29) - शर्त मानों के आधार पर अभिव्यक्ति मानों में कुछ अपवाद बनाने की अनुमति देता है

##### दिनांक और समय
* [NOW()](Functions/Date_and_time_functions.md#NOW%28%29) - वर्तमान स्टाम्प को INTEGER के रूप में लौटाता है
* [CURTIME()](Functions/Date_and_time_functions.md#CURTIME%28%29) - स्थानीय समय क्षेत्र में वर्तमान समय लौटाता है
* [CURDATE()](Functions/Date_and_time_functions.md#CURDATE%28%29) - स्थानीय समय क्षेत्र में वर्तमान तिथि लौटाता है
* [UTC_TIME()](Functions/Date_and_time_functions.md#UTC_TIME%28%29) - UTC समय क्षेत्र में वर्तमान समय लौटाता है
* [UTC_TIMESTAMP()](Functions/Date_and_time_functions.md#UTC_TIMESTAMP%28%29) - UTC समय क्षेत्र में वर्तमान तिथि/समय लौटाता है
* [SECOND()](Functions/Date_and_time_functions.md#SECOND%28%29) - टाइमस्टैम्प तर्क से पूर्णांक सेकंड लौटाता है
* [MINUTE()](Functions/Date_and_time_functions.md#MINUTE%28%29) - टाइमस्टैम्प तर्क से पूर्णांक मिनट लौटाता है
* [HOUR()](Functions/Date_and_time_functions.md#HOUR%28%29) - टाइमस्टैम्प तर्क से पूर्णांक घंटा लौटाता है
* [DAY()](Functions/Date_and_time_functions.md#DAY%28%29) - टाइमस्टैम्प तर्क से पूर्णांक दिन लौटाता है
* [MONTH()](Functions/Date_and_time_functions.md#MONTH%28%29) - टाइमस्टैम्प तर्क से पूर्णांक महीना लौटाता है
* [QUARTER()](Functions/Date_and_time_functions.md#QUARTER%28%29) - टाइमस्टैम्प तर्क से वर्ष का पूर्णांक तिमाही लौटाता है
* [YEAR()](Functions/Date_and_time_functions.md#YEAR%28%29) - टाइमस्टैम्प तर्क से पूर्णांक वर्ष लौटाता है
* [DAYNAME()](Functions/Date_and_time_functions.md#DAYNAME%28%29) - दिए गए टाइमस्टैम्प तर्क के लिए सप्ताह का दिन नाम लौटाता है
* [MONTHNAME()](Functions/Date_and_time_functions.md#MONTHNAME%28%29) - दिए गए टाइमस्टैम्प तर्क के लिए महीने का नाम लौटाता है
* [DAYOFWEEK()](Functions/Date_and_time_functions.md#DAYOFWEEK%28%29) - दिए गए टाइमस्टैम्प तर्क के लिए पूर्णांक सप्ताह का दिन सूचकांक लौटाता है
* [DAYOFYEAR()](Functions/Date_and_time_functions.md#DAYOFYEAR%28%29) - दिए गए टाइमस्टैम्प तर्क के लिए वर्ष का पूर्णांक दिन लौटाता है
* [YEARWEEK()](Functions/Date_and_time_functions.md#YEARWEEK%28%29) - दिए गए टाइमस्टैम्प तर्क के लिए वर्तमान सप्ताह के पहले दिन का पूर्णांक वर्ष और दिन कोड लौटाता है
* [YEARMONTH()](Functions/Date_and_time_functions.md#YEARMONTH%28%29) - टाइमस्टैम्प तर्क से पूर्णांक वर्ष और महीने का कोड लौटाता है
* [YEARMONTHDAY()](Functions/Date_and_time_functions.md#YEARMONTHDAY%28%29) - टाइमस्टैम्प तर्क से पूर्णांक वर्ष, महीने और दिन का कोड लौटाता है
* [TIMEDIFF()](Functions/Date_and_time_functions.md#TIMEDIFF%28%29) - टाइमस्टैम्प के बीच का अंतर लौटाता है
* [DATEDIFF()](Functions/Date_and_time_functions.md#DATEDIFF%28%29) - दो दिए गए टाइमस्टैम्प के बीच दिनों की संख्या लौटाता है
* [DATE()](Functions/Date_and_time_functions.md#DATE%28%29) - टाइमस्टैम्प तर्क से दिनांक भाग को स्वरूपित करता है
* [TIME()](Functions/Date_and_time_functions.md#TIME%28%29) - टाइमस्टैम्प तर्क से समय भाग को स्वरूपित करता है
* [DATE_FORMAT()](Functions/Date_and_time_functions.md#DATE_FORMAT%28%29) - दिए गए दिनांक और स्वरूप तर्कों के आधार पर स्वरूपित स्ट्रिंग लौटाता है


##### भू-स्थानिक
* [GEODIST()](Functions/Geo_spatial_functions.md#GEODIST%28%29) - दो दिए गए बिंदुओं के बीच भूस्पष्ट दूरी की गणना करता है
* [GEOPOLY2D()](Functions/Geo_spatial_functions.md#GEOPOLY2D%28%29) - एक बहुभुज बनाता है जो पृथ्वी की वक्रता को ध्यान में रखता है
* [POLY2D()](Functions/Geo_spatial_functions.md#POLY2D%28%29) - सामान्य स्थान में एक साधारण बहुभुज बनाता है

##### स्ट्रिंग
* [CONCAT()](Functions/String_functions.md#CONCAT%28%29) - दो या दो से अधिक स्ट्रिंग्स को जोड़ता है
* [REGEX()](Functions/String_functions.md#REGEX%28%29) - यदि नियमित अभिव्यक्ति विशेषता की स्ट्रिंग से मेल खाती है तो 1 लौटाता है, अन्यथा 0
* [SNIPPET()](Functions/String_functions.md#SNIPPET%28%29) - खोज परिणामों को हाइलाइट करता है
* [SUBSTRING_INDEX()](Functions/String_functions.md#SUBSTRING_INDEX%28%29) - निर्दिष्ट संख्या के डिलीमीटर होने से पहले की स्ट्रिंग का उप-स्ट्रिंग लौटाता है

##### अन्य
* [CONNECTION_ID()](Functions/Other_functions.md#CONNECTION_ID%28%29) - वर्तमान कनेक्शन आईडी लौटाता है
* [KNN_DIST()](Functions/Other_functions.md#KNN_DIST%28%29) - KNN वेक्टर खोज की दूरी लौटाता है
* [LAST_INSERT_ID()](Functions/Other_functions.md#LAST_INSERT_ID%28%29) - वर्तमान सत्र में अंतिम वक्तव्य द्वारा डाले गए या प्रतिस्थापित दस्तावेजों के आईडी लौटाता है
* [UUID_SHORT()](Functions/Other_functions.md#UUID_SHORT%28%29) - "छोटा" सार्वभौमिक पहचानकर्ता लौटाता है जो ऑटो-आईडी जनरेशन के लिए समान एल्गोरिदम का पालन करता है.

## कॉन्फ़िगरेशन फ़ाइल में सामान्य सेटिंग्स
अनुभाग `common {}` में डालने के लिए:
* [lemmatizer_base](Server_settings/Common.md#lemmatizer_base) - लेम्माटाइज़र शब्दकोशों का आधार पथ
* [progressive_merge](Server_settings/Common.md#progressive_merge) - वास्तविक समय तालिका में डिस्क चंक्स को मिलाने का आदेश परिभाषित करता है
* [json_autoconv_keynames](Server_settings/Common.md#json_autoconv_keynames) - JSON विशेषताओं के भीतर कुंजी नामों को ऑटो-रूपांतरित करने के लिए क्या और कैसे
* [json_autoconv_numbers](Server_settings/Common.md#json_autoconv_numbers) - संभावित JSON स्ट्रिंग्स जो संख्याओं का प्रतिनिधित्व करती हैं उन्हें स्वचालित रूप से पहचानता है और संख्यात्मक विशेषताओं में परिवर्तित करता है
* [on_json_attr_error](Server_settings/Common.md#on_json_attr_error) - यदि JSON प्रारूप त्रुटियाँ पाई जाती हैं तो क्या करें
* [plugin_dir](Server_settings/Common.md#plugin_dir) - गतिशील पुस्तकालयों और UDFs के लिए स्थान

## [Indexer](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments)
`indexer` एक उपकरण है जो [सादा तालिकाएँ](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) बनाने के लिए है

##### कॉन्फ़िगरेशन फ़ाइल में इंडेक्सर सेटिंग्स
अनुभाग `indexer {}` में डालने के लिए:
* [lemmatizer_cache](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - लेम्माटाइज़र कैश आकार
* [max_file_field_buffer](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - अधिकतम फ़ाइल फ़ील्ड अनुकूली बफर आकार
* [max_iops](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - अधिकतम अनुक्रमण I/O संचालन प्रति सेकंड
* [max_iosize](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - अधिकतम अनुमत I/O संचालन आकार
* [max_xmlpipe2_field](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - XMLpipe2 स्रोत प्रकार के लिए अधिकतम अनुमत क्षेत्र आकार
* [mem_limit](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - अनुक्रमण RAM उपयोग सीमा
* [on_file_field_error](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - फ़ाइल क्षेत्रों में IO त्रुटियों को कैसे संभालना है
* [write_buffer](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - लिखने के बफ़र का आकार
* [ignore_non_plain](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - गैर-प्लेन तालिकाओं के बारे में चेतावनियों की अनदेखी करने के लिए

##### अनुक्रमण प्रारंभ पैरामीटर
```bash
indexer [OPTIONS] [indexname1 [indexname2 [...]]]
```
* [--all](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - कॉन्फ़िगरेशन से सभी तालिकाएँ फिर से बनाएँ
* [--buildstops](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - डेटा अनुक्रमण के रूप में तालिका स्रोत का विश्लेषण करता है, अनुक्रमित शर्तों की एक सूची उत्पन्न करता है
* [--buildfreqs](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - --buildstops के लिए तालिका में आवृत्ति गिनती जोड़ता है
* [--config, -c](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - कॉन्फ़िगरेशन फ़ाइल के पथ को निर्दिष्ट करता है
* [--dump-rows](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - निर्दिष्ट फ़ाइल में SQL स्रोत द्वारा पुनः प्राप्त पंक्तियों को निकालता है
* [--help](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - सभी उपलब्ध पैरामीटर प्रदर्शित करता है
* [--keep-attrs](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - पुनः अनुक्रमण के दौरान मौजूदा विशेषताओं के पुन: उपयोग की अनुमति देता है
* [--keep-attrs-names](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - मौजूदा तालिका से पुन: उपयोग करने के लिए किन विशेषताओं को निर्दिष्ट करता है
* [--merge-dst-range](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - मर्ज करते समय दिए गए फ़िल्टर रेंज को लागू करता है
* [--merge-killlists](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - तालिकाओं को मर्ज करते समय किल लिस्ट प्रसंस्करण को बदलता है
* [--merge](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - दो प्लेन तालिकाओं को एक में मिलाता है
* [--nohup](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - इस विकल्प के सक्षम होने पर अनुक्रमक को SIGHUP भेजने से रोकता है
* [--noprogress](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - प्रगति विवरणों को छुपाता है
* [--print-queries](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - अनुक्रमक द्वारा डेटाबेस में भेजे गए SQL प्रश्नों को आउटपुट करता है
* [--print-rt](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - SQL स्रोत से लाए गए डेटा को INSERTs के रूप में वास्तविक समय तालिका में प्रदर्शित करता है
* [--quiet](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - सभी आउटपुट को दबाता है
* [--rotate](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - सभी तालिकाएँ बनाए जाने के बाद तालिका को घुमाने की प्रक्रिया शुरू करता है
* [--sighup-each](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - प्रत्येक तालिका के बनने के बाद उसकी घुमाव के लिए ट्रिगर करता है
* [-v](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) - अनुक्रमक संस्करण प्रदर्शित करता है

## तालिका कनवर्टर Manticore v2 / Sphinx v2 के लिए
`index_converter` एक उपकरण है जिसे Sphinx/Manticore Search 2.x के साथ बनाई गई तालिकाओं को Manticore Search 3.x तालिका प्रारूप में परिवर्तित करने के लिए डिज़ाइन किया गया है।
```bash
index_converter {--config /path/to/config|--path}
```
##### तालिका कनवर्टर प्रारंभ पैरामीटर
* [--config, -c](Installation/Migration_from_Sphinx.md#index_converter) - तालिका कॉन्फ़िगरेशन फ़ाइल के लिए पथ
* [--index](Installation/Migration_from_Sphinx.md#index_converter) - निर्दिष्ट करता है कि कौन सी तालिका को परिवर्तित करना है
* [--path](Installation/Migration_from_Sphinx.md#index_converter) - कॉन्फ़िगरेशन फ़ाइल के बजाय तालिका(s) रखने के लिए पथ सेट करता है
* [--strip-path](Installation/Migration_from_Sphinx.md#index_converter) - तालिका द्वारा संदर्भित फ़ाइल नामों से पथ को हटाता है
* [--large-docid](Installation/Migration_from_Sphinx.md#index_converter) - 2^63 से बड़े आईडी वाले दस्तावेजों के परिवर्तित करने की अनुमति देता है
* [--output-dir](Installation/Migration_from_Sphinx.md#index_converter) - निर्दिष्ट फ़ोल्डर में नए फ़ाइलें लिखता है
* [--all](Installation/Migration_from_Sphinx.md#index_converter) - कॉन्फ़िगरेशन फ़ाइल / पथ से सभी तालिकाओं को परिवर्तित करता है
* [--killlist-target](Installation/Migration_from_Sphinx.md#index_converter) - किल-लिस्ट लागू करने के लिए लक्षित तालिका सेट करता है

## [Searchd](Starting_the_server/Manually.md)
`searchd` अंटिकॉर सर्वर है.

##### कॉन्फ़िगरेशन फ़ाइल में Searchd सेटिंग्स
इसे कॉन्फ़िगरेशन फ़ाइल के `searchd {}` अनुभाग में रखा जाना चाहिए:
  * [access_blob_attrs](Server_settings/Searchd.md#access_blob_attrs) - परिभाषित करता है कि तालिका की ब्लॉब विशेषताओं की फ़ाइल कैसे एक्सेस की जाती है
  * [access_doclists](Server_settings/Searchd.md#access_doclists) - परिभाषित करता है कि तालिका की doclists फ़ाइल कैसे एक्सेस की जाती है
  * [access_hitlists](Server_settings/Searchd.md#access_hitlists) - परिभाषित करता है कि तालिका की hitlists फ़ाइल कैसे एक्सेस की जाती है
  * [access_plain_attrs](Server_settings/Searchd.md#access_plain_attrs) - परिभाषित करता है कि खोज सर्वर तालिका की सामान्य विशेषताओं को कैसे एक्सेस करता है
  * [access_dict](Server_settings/Searchd.md#access_dict) - परिभाषित करता है कि तालिका की शब्दकोष फ़ाइल कैसे एक्सेस की जाती है
  * [agent_connect_timeout](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_connect_timeout) - रिमोट एजेंट कनेक्शन टाइमआउट
  * [agent_query_timeout](Searching/Options.md#agent_query_timeout) - रिमोट एजेंट क्वेरी टाइमआउट
  * [agent_retry_count](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_connect_timeout) - निर्दिष्ट करता है कि अंटिकॉर कितनी बार रिमोट एजेंट से कनेक्ट होने और क्वेरी करने का प्रयास करता है
  * [agent_retry_delay](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) - विफलता की स्थिति में रिमोट एजेंट से क्वेरी फिर से प्रयास करने से पहले की देरी निर्दिष्ट करता है
  * [attr_flush_period](Data_creation_and_modification/Updating_documents/UPDATE.md#attr_flush_period) - डिस्क पर अपडेट की गई विशेषताओं को फ्लश करने के बीच का समय अवधि सेट करता है
  * [binlog_flush](Server_settings/Searchd.md#binlog_flush) - बाइनरी लॉग लेनदेन फ्लश/सिंक मोड
  * [binlog_max_log_size](Server_settings/Searchd.md#binlog_max_log_size) - अधिकतम बाइनरी लॉग फ़ाइल आकार
  * [binlog_common](Logging/Binary_logging.md#Binary-logging-strategies) - सभी तालिकाओं के लिए सामान्य बाइनरी लॉग फ़ाइल
  * [binlog_filename_digits](Logging/Binary_logging.md#Log-files) - बाइनरी लॉग फ़ाइल नाम में अंकों की संख्या
  * [binlog_flush](Logging/Binary_logging.md#Binary-flushing-strategies) - बाइनरी लॉग फ्लशिंग रणनीति
  * [binlog_path](Server_settings/Searchd.md#binlog_path) - बाइनरी लॉग फ़ाइलों का पथ
  * [client_timeout](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) - लगातार कनेक्शन का उपयोग करते समय अनुरोधों के बीच अधिकतम समय
  * [collation_libc_locale](Server_settings/Searchd.md#collation_libc_locale) - सर्वर libc स्थानीयता
  * [collation_server](Server_settings/Searchd.md#collation_server) - डिफ़ॉल्ट सर्वर संकलन
  * [data_dir](Server_settings/Searchd.md#data_dir) - डेटा निर्देशिका का पथ जहाँ अंटिकॉर सब कुछ संग्रहीत करता है ([RT मोड](Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29))
  * [diskchunk_flush_write_timeout](Server_settings/Searchd.md#diskchunk_flush_write_timeout) - यदि इसमें लेखन नहीं है तो RAM चंक को स्वचालित रूप से फ्लश करने का टाइमआउट
  * [diskchunk_flush_search_timeout](Server_settings/Searchd.md#diskchunk_flush_search_timeout) - यदि तालिका में खोजें नहीं हैं तो RAM चंक को स्वचालित रूप से फ्लश करने से रोकने का टाइमआउट
  * [docstore_cache_size](Server_settings/Searchd.md#docstore_cache_size) - मेमोरी में रखे गए दस्तावेज़ संग्रह से दस्तावेज़ ब्लॉकों का अधिकतम आकार
  * [expansion_limit](Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#expansion_limit) - एकल वाइल्डकार्ड के लिए अधिकतम विस्तारित कीवर्ड की संख्या
  * [grouping_in_utc](Server_settings/Searchd.md#grouping_in_utc) - समय फ़ील्ड को समूहबद्ध करने के लिए UTC समय क्षेत्र का उपयोग करने की अनुमति देता है
  * [ha_period_karma](Server_settings/Searchd.md#ha_period_karma) - एजेंट मिरर सांख्यिकी विंडो का आकार
  * [ha_ping_interval](Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_ping_interval) - एजेंट मिरर पिंग के बीच अंतराल
  * [hostname_lookup](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) - होस्टनेम नवीनीकरण रणनीति
  * [jobs_queue_size](Server_settings/Searchd.md#jobs_queue_size) - परिभाषित करता है कि कतार में एक साथ कितने "जॉब" की अनुमति है
  * [join_batch_size](Searching/Joining.md#Join-batching) - प्रदर्शन और मेमोरी उपयोग को संतुलित करने के लिए तालिका के जॉइन के लिए बैच का आकार परिभाषित करता है
  * [join_cache_size](Searching/Joining.md#Join-caching) - JOIN क्वेरी परिणामों को पुन: उपयोग करने के लिए कैश का आकार परिभाषित करता है
  * [kibana_version_string](Server_settings/Searchd.md#kibana_version_string) – वह सर्वर संस्करण स्ट्रिंग जो Kibana अनुरोधों के जवाब में भेजी जाती है
  * [listen](Server_settings/Searchd.md#listen) - searchd के लिए सुनने के लिए IP पते और पोर्ट या यूनिक्स-डोमेन सॉकेट पथ निर्दिष्ट करता है
  * [listen_backlog](Server_settings/Searchd.md#listen_backlog) - TCP सुनने की बैकलॉग
  * [listen_tfo](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) - सभी श्रोताओं के लिए TCP_FASTOPEN ध्वज को सक्षम करता है
  * [log](Server_settings/Searchd.md#log) - अंटिकॉर सर्वर लॉग फ़ाइल का पथ
  * [max_batch_queries](Server_settings/Searchd.md#max_batch_queries) - प्रति बैच क्वेरी की संख्या को सीमित करता है
  * [max_connections](Server_settings/Searchd.md#max_connections) - सक्रिय कनेक्शनों की अधिकतम संख्या
  * [max_filters](Server_settings/Searchd.md#max_filters) - प्रति-क्वेरी फ़िल्टर गणना की अधिकतम अनुमति
  * [max_filter_values](Server_settings/Searchd.md#max_filter_values) - प्रति-फ़िल्टर मानों की अधिकतम अनुमति
  * [max_open_files](Server_settings/Searchd.md#max_open_files) - सर्वर द्वारा खोले गए फ़ाइलों की अधिकतम संख्या
  * [max_packet_size](Server_settings/Searchd.md#max_packet_size) - अधिकतम अनुमति नेटवर्क पैकेट आकार
  * [mysql_version_string](Server_settings/Searchd.md#mysql_version_string) - MySQL प्रोटोकॉल के माध्यम से लौटाई गई सर्वर संस्करण स्ट्रिंग
  * [net_throttle_accept](Server_settings/Searchd.md#net_throttle_accept) - प्रत्येक नेटवर्क लूप के पुनरावृत्ति पर कितने क्लाइंट स्वीकार किए जाते हैं, इसे परिभाषित करता है
  * [net_throttle_action](Server_settings/Searchd.md#net_throttle_action)  - प्रत्येक नेटवर्क लूप के पुनरावृत्ति पर कितने अनुरोधों को संसाधित किया जाता है, इसे परिभाषित करता है
  * [net_wait_tm](Server_settings/Searchd.md#net_wait_tm) - एक नेटवर्क थ्रेड के व्यस्त लूप अंतराल को नियंत्रित करता है
  * [net_workers](Server_settings/Searchd.md#net_workers) - नेटवर्क थ्रेड्स की संख्या
  * [network_timeout](Server_settings/Searchd.md#network_timeout) - क्लाइंट अनुरोधों के लिए नेटवर्क समय सीमा
  * [node_address](Server_settings/Searchd.md#node_address) - नोड का नेटवर्क पता निर्दिष्ट करता है
  * [persistent_connections_limit](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) - दूरस्थ स्थायी एजेंटों के लिए अधिकतम संख्या के समानांतर स्थायी कनेक्शन
  * [pid_file](Server_settings/Searchd.md#pid_file) - मैन्टिकोर सर्वर पीआईडी फ़ाइल का पथ
  * [predicted_time_costs](Server_settings/Searchd.md#predicted_time_costs) - अनुरोध समय अनुमान मॉडल के लिए लागत
  * [preopen_tables](Server_settings/Searchd.md#preopen_tables) - यह निर्धारित करता है कि क्या स्टार्टअप पर सभी तालिकाओं को बलपूर्वक पूर्व-खोलना है
  * [pseudo_sharding](Server_settings/Searchd.md#pseudo_sharding) - साधारण और वास्तविक समय की तालिकाओं के लिए खोज अनुरोधों के लिए नकली-शार्डिंग सक्षम करता है
  * [qcache_max_bytes](Server_settings/Searchd.md#qcache_max_bytes) - कैश किए गए परिणाम सेटों के लिए अधिकतम रैम
  * [qcache_thresh_msec](Server_settings/Searchd.md#qcache_thresh_msec) - एक अनुरोध परिणाम को कैश करने के लिए न्यूनतम दीवार समय थ्रेशोल्ड
  * [qcache_ttl_sec](Server_settings/Searchd.md#qcache_ttl_sec) - कैश किए गए परिणाम सेट के लिए समाप्ति अवधि
  * [query_log](Server_settings/Searchd.md#query_log) - क्वेरी लॉग फ़ाइल का पथ
  * [query_log_format](Server_settings/Searchd.md#query_log_format) - क्वेरी लॉग फ़ॉर्मेट
  * [query_log_min_msec](Server_settings/Searchd.md#query_log_min_msec) - बहुत तेज़ अनुरोधों की लॉगिंग को रोकता है
  * [query_log_mode](Server_settings/Searchd.md#query_log_mode) - क्वेरी लॉग फ़ाइल अनुमति मोड
  * [read_buffer_docs](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#read_buffer_docs) - दस्तावेज़ सूचियों के लिए प्रति-किवर्ड पढ़ने का बफर आकार
  * [read_buffer_hits](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#read_buffer_docs) - हिट सूचियों के लिए प्रति-किवर्ड पढ़ने का बफर आकार
  * [read_unhinted](Server_settings/Searchd.md#read_unhinted) - अनहिन्टेड पढ़ने का आकार
  * [rt_flush_period](Server_settings/Searchd.md#rt_flush_period) - मैन्टिकोर कितनी बार वास्तविक समय की तालिकाओं के आरएएम_chunks को डिस्क पर फेंकता है
  * [rt_merge_iops](Server_settings/Searchd.md#rt_merge_iops) - अधिकतम संख्या में I/O संचालन (प्रति सेकंड) जो वास्तविक समय के खंडों को मर्ज करने वाले थ्रेड को करने की अनुमति है
  * [rt_merge_maxiosize](Server_settings/Searchd.md#rt_merge_maxiosize) - अधिकतम आकार का I/O संचालन जो वास्तविक समय के खंडों को मर्ज करने वाले थ्रेड को करने की अनुमति है
  * [seamless_rotate](Server_settings/Searchd.md#seamless_rotate) - बड़े डेटा के साथ तालिकाओं को पूर्व-कैश करने के लिए घुमाते समय सर्च्ड ठहराव को रोकता है
  * [secondary_indexes](Server_settings/Searchd.md#secondary_indexes) - खोज अनुरोधों के लिए माध्यमिक सूचकांकों का उपयोग सक्षम करता है
  * [server_id](Server_settings/Searchd.md#server_id) - सर्वर पहचानकर्ता जिसका उपयोग एक अद्वितीय दस्तावेज़ आईडी बनाने के लिए बीज के रूप में किया जाता है
  * [shutdown_timeout](Server_settings/Searchd.md#shutdown_timeout) - सर्च्ड `--stopwait` समय सीमा
  * [shutdown_token](Server_settings/Searchd.md#shutdown_token) - वीआईपी एसक्यूएल कनेक्शन से `shutdown` आदेश को लागू करने के लिए आवश्यक पासवर्ड का SHA1 हैश
  * [snippets_file_prefix](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) - `load_files` मोड में स्निपेट्स生成 करते समय स्थानीय फ़ाइल नामों में जोड़ने के लिए उपसर्ग
  * [sphinxql_state](Server_settings/Searchd.md#sphinxql_state) - फ़ाइल का पथ जहां वर्तमान SQL स्थिति में सीरियलाइज किया जाएगा
  * [sphinxql_timeout](Server_settings/Searchd.md#sphinxql_timeout) - MySQL क्लाइंट से अनुरोधों के बीच का अधिकतम समय
  * [ssl_ca](Server_settings/Searchd.md#ssl_ca) - एसएसएल सर्टिफिकेट अथॉरिटी सर्टिफिकेट फ़ाइल का पथ
  * [ssl_cert](Server_settings/Searchd.md#ssl_cert) - सर्वर के एसएसएल प्रमाणपत्र का पथ
  * [ssl_key](Server_settings/Searchd.md#ssl_key) - सर्वर के एसएसएल प्रमाणपत्र कुंजी का पथ
  * [subtree_docs_cache](Server_settings/Searchd.md#subtree_docs_cache) - अधिकतम सामान्य उपवृक्ष दस्तावेज़ कैश आकार
  * [subtree_hits_cache](Server_settings/Searchd.md#subtree_hits_cache) - प्रति-प्रश्न अधिकतम सामान्य उपवृक्ष हिट कैश आकार
  * [timezone](Server_settings/Searchd.md#timezone) - तिथि/समय-संबंधित कार्यों द्वारा उपयोग की जाने वाली टाइमज़ोन
  * [thread_stack](Server_settings/Searchd.md#thread_stack) - एक कार्य के लिए अधिकतम स्टैक आकार
  * [unlink_old](Server_settings/Searchd.md#unlink_old) - सफल रोटेशन पर .old तालिका प्रतियों को अनलिंक करना है या नहीं
  * [watchdog](Server_settings/Searchd.md#watchdog) - मैन्टिकोर सर्वर वॉचडॉग को सक्षम या अक्षम करना है

##### सर्च्ड प्रारंभ पैरामीटर
```bash
searchd [OPTIONS]
```
* [--config, -c](Starting_the_server/Manually.md#searchd-command-line-options) - कॉन्फ़िगरेशन फ़ाइल का पथ निर्दिष्ट करता है
* [--console](Starting_the_server/Manually.md#searchd-command-line-options) - सर्वर को कंसोल मोड में चलाने के लिए मजबूर करता है
* [--coredump](Starting_the_server/Manually.md#searchd-command-line-options) - क्रैश होने पर कोर डंप सहेजने को सक्षम बनाता है
* [--cpustats](Starting_the_server/Manually.md#searchd-command-line-options) - CPU समय रिपोर्टिंग को सक्षम बनाता है
* [--delete](Starting_the_server/Manually.md#searchd-command-line-options) - मैन्टिकोर सेवा को माइक्रोसॉफ्ट प्रबंधन कंसोल और अन्य स्थानों से हटा देता है जहां सेवाएं पंजीकृत होती हैं
* [--force-preread](Starting_the_server/Manually.md#searchd-command-line-options) - टैब फ़ाइलों के पूर्व-पढ़े जाने तक सर्वर को आने वाले कनेक्शनों को सेवा देने से रोकता है
* [--help, -h](Starting_the_server/Manually.md#searchd-command-line-options) - सभी उपलब्ध पैरामीटर प्रदर्शित करता है
* [--table (--index)](Starting_the_server/Manually.md#searchd-command-line-options) - सर्वर को केवल निर्दिष्ट तालिका को सेवा देने के लिए सीमित करता है
* [--install](Starting_the_server/Manually.md#searchd-command-line-options) - Microsoft प्रबंधन कंसोल में searchd को एक सेवा के रूप में स्थापित करता है
* [--iostats](Starting_the_server/Manually.md#searchd-command-line-options) - इनपुट/आउटपुट रिपोर्टिंग सक्षम करता है
* [--listen, -l](Starting_the_server/Manually.md#searchd-command-line-options) - कॉन्फ़िगरेशन फ़ाइल से [listen](Server_settings/Searchd.md#listen) को ओवरराइड करता है
* [--logdebug, --logdebugv, --logdebugvv](Starting_the_server/Manually.md#searchd-command-line-options) - सर्वर लॉग में अतिरिक्त डिबग आउटपुट सक्षम करता है
* [--logreplication](Starting_the_server/Manually.md#searchd-command-line-options) - सर्वर लॉग में अतिरिक्त पुनरुत्पादन डिबग आउटपुट सक्षम करता है
* [--new-cluster](Starting_the_server/Manually.md#searchd-command-line-options) - एक पुनरुत्पादन क्लस्टर आरंभ करता है और सर्वर को [क्लस्टर पुनः आरंभ](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md) सुरक्षा के साथ संदर्भ नोड के रूप में सेट करता है
* [--new-cluster-force](Starting_the_server/Manually.md#searchd-command-line-options) - एक पुनरुत्पादन क्लस्टर आरंभ करता है और सर्वर को संदर्भ नोड के रूप में सेट करता है, [क्लस्टर पुनः आरंभ](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md) सुरक्षा को बायपास करता है
* [--nodetach](Starting_the_server/Manually.md#searchd-command-line-options) - searchd को अग्रभूमि में चलते रहने देता है
* [--ntservice](Starting_the_server/Manually.md#searchd-command-line-options) - Microsoft प्रबंधन कंसोल द्वारा Windows प्लेटफॉर्म पर searchd को एक सेवा के रूप में लॉन्च करने के लिए उपयोग किया जाता है
* [--pidfile](Starting_the_server/Manually.md#searchd-command-line-options) - कॉन्फ़िगरेशन फ़ाइल में [pid_file](Server_settings/Searchd.md#pid_file) को ओवरराइड करता है
* [--port, p](Starting_the_server/Manually.md#searchd-command-line-options) - निर्दिष्ट करता है कि searchd को किस पोर्ट पर सुनना चाहिए, कॉन्फ़िगरेशन फ़ाइल में निर्दिष्ट पोर्ट को अनदेखा करते हुए
* [--replay-flags](Starting_the_server/Manually.md#searchd-command-line-options) - अतिरिक्त बाइनरी लॉग पुनरावृत्ति विकल्प सेट करता है
* [--servicename](Starting_the_server/Manually.md#searchd-command-line-options) - सेवा को इंस्टॉल या हटाते समय searchd को दिए गए नाम को सौंपता है, जैसा कि Microsoft प्रबंधन कंसोल में प्रदर्शित होता है
* [--status](Starting_the_server/Manually.md#searchd-command-line-options) - चल रहे खोज सेवा से उसके स्थिति को लौटाने के लिए क्वेरी करता है
* [--stop](Starting_the_server/Manually.md#searchd-command-line-options) - Manticore सर्वर को रोकता है
* [--stopwait](Starting_the_server/Manually.md#searchd-command-line-options) - Manticore सर्वर को शांति से रोकता है
* [--strip-path](Starting_the_server/Manually.md#searchd-command-line-options) - तालिका में संदर्भित सभी फ़ाइल नामों से पथ नाम हटा देता है
* [-v](Starting_the_server/Manually.md#searchd-command-line-options) - संस्करण जानकारी प्रदर्शित करता है

##### Searchd परिवेश चर
* [MANTICORE_TRACK_DAEMON_SHUTDOWN](Starting_the_server/Manually.md#Environment-variables) - searchd शटडाउन के दौरान विस्तृत लॉगिंग सक्षम करता है

## [Indextool](Miscellaneous_tools.md#indextool)
समस्या निवारण के लिए सहायक विविध तालिका रखरखाव विशेषताएँ।
```bash
indextool <command> [options]
```
##### Indextool प्रारंभ पैरामीटर
भौतिक तालिका से संबंधित विभिन्न डिबग जानकारी को डंप करने के लिए उपयोग किया जाता है।
```bash
indextool <command> [options]
```
* [--config, -c](Miscellaneous_tools.md#indextool) - कॉन्फ़िगरेशन फ़ाइल के लिए पथ निर्दिष्ट करता है
* [--quiet, -q](Miscellaneous_tools.md#indextool) - indextool को चुप रखता है; कोई बैनर आउटपुट नहीं, आदि।
* [--help, -h](Miscellaneous_tools.md#indextool) - सभी उपलब्ध पैरामीटर सूचीबद्ध करता है
* [-v](Miscellaneous_tools.md#indextool) - संस्करण जानकारी प्रदर्शित करता है
* [Indextool](Miscellaneous_tools.md#indextool) - कॉन्फ़िगरेशन फ़ाइल की पुष्टि करता है
* [--buildidf](Miscellaneous_tools.md#indextool) - एक या एक से अधिक शब्दकोश डम्प से एक IDF फ़ाइल बनाता है
* [--build-infixes](Miscellaneous_tools.md#indextool) - मौजूदा dict=keywords तालिका के लिए इन्फिक्स बनाता है
* [--dumpheader](Miscellaneous_tools.md#indextool) - प्रदान किए गए तालिका हैडर फ़ाइल को तुरंत डंप करता है
* [--dumpconfig](Miscellaneous_tools.md#indextool) - दिए गए तालिका हैडर फ़ाइल से तालिका परिभाषा को लगभग अनुरूप manticore.conf प्रारूप में डंप करता है
* [--dumpheader](Miscellaneous_tools.md#indextool) - तालिका नाम द्वारा तालिका हैडर को डंप करता है जबकि कॉन्फ़िगरेशन फ़ाइल में हैडर पथ को खोजता है
* [--dumpdict](Miscellaneous_tools.md#indextool) - तालिका शब्दकोश को डंप करता है
* [--dumpdocids](Miscellaneous_tools.md#indextool) - तालिका नाम द्वारा दस्तावेज़ आईडी को डंप करता है
* [--dumphitlist](Miscellaneous_tools.md#indextool) - निर्दिष्ट तालिका में दिए गए कीवर्ड/आईडी की सभीOccurrences को डंप करता है
* [--docextract](Miscellaneous_tools.md#indextool) - पूरे शब्दकोश/दस्तावेज़/हिट पर तालिका चेक पास करता है और अनुरोधित दस्तावेज़ से संबंधित सभी शब्दों और हिट एकत्र करता है
* [--fold](Miscellaneous_tools.md#indextool) - तालिका सेटिंग्स के आधार पर टोकनाइजेशन का परीक्षण करता है
* [--htmlstrip](Miscellaneous_tools.md#indextool) - निर्दिष्ट तालिका के लिए HTML स्ट्रिपर सेटिंग्स का उपयोग करके STDIN को फ़िल्टर करता है
* [--mergeidf](Miscellaneous_tools.md#indextool) - कई .idf फ़ाइलों को एकल फ़ाइल में मर्ज करता है
* [--morph](Miscellaneous_tools.md#indextool) - प्रदान की गई STDIN पर रूपिकी लागू करता है और परिणाम को stdout पर आउटपुट करता है
* [--check](Miscellaneous_tools.md#indextool) - तालिका डेटा फ़ाइलों की संगति की जांच करता है
* [--check-id-dups](Miscellaneous_tools.md#indextool) - डुप्लिकेट आईडी की जांच करता है
* [--check-disk-chunk](Miscellaneous_tools.md#indextool) - RT तालिका का एकल डिस्क खंड जांचता है
* [--strip-path](Miscellaneous_tools.md#indextool) - तालिका में संदर्भित सभी फ़ाइल नामों से पथ नाम हटा देता है
* [--rotate](Miscellaneous_tools.md#indextool) - `--check` का उपयोग करते समय रोटेशन की प्रतीक्षा कर रहे तालिका की जाँच करने के लिए निर्धारित करता है
* [--apply-killlists](Miscellaneous_tools.md#indextool) - सभी तालिकाओं के लिए किल-लिस्ट लागू करता है जो कॉन्फ़िगरेशन फ़ाइल में सूचीबद्ध हैं

## [Wordbreaker](Miscellaneous_tools.md#wordbreaker)
यौगिक शब्दों को उनके घटकों में विभाजित करता है।
```bash
wordbreaker [-dict path/to/dictionary_file] {split|test|bench}
```

##### Wordbreaker प्रारंभ पैरामीटर्स
* [STDIN](Miscellaneous_tools.md#wordbreaker) - एक स्ट्रिंग स्वीकार करता है जिसे भागों में तोड़ा जाना है
* [-dict](Miscellaneous_tools.md#wordbreaker) - उपयोग के लिए शब्दकोश फ़ाइल निर्दिष्ट करता है
* [split|test|bench](Miscellaneous_tools.md#wordbreaker) - कमांड निर्दिष्ट करता है

## [Spelldump](Miscellaneous_tools.md#spelldump)
ispell या MySpell फ़ॉर्मेट का उपयोग करके शब्दकोश फ़ाइल की सामग्री निकालता है

```bash
spelldump [options] <dictionary> <affix> [result] [locale-name]
