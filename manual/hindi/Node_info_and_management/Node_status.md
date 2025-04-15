# नोड स्थिति

## स्थिति

<!-- उदाहरण स्थिति -->
आपके मैन्टिकोरे नोड के बारे में उच्च-स्तरीय जानकारी देखने का सबसे आसान तरीका MySQL क्लाइंट में `status` चलाना है। यह विभिन्न पहलुओं के बारे में जानकारी प्रदर्शित करेगा, जैसे:
* वर्तमान संस्करण
* क्या SSL प्रभावी है या नहीं
* वर्तमान TCP पोर्ट/यूनिक्स सॉकेट
* अपटाइम
* [थ्रेड्स](../Server_settings/Searchd.md#threads) की संख्या
* [क्यू में नौकरियों की संख्या](../Server_settings/Searchd.md#jobs_queue_size)
* कनेक्शनों की संख्या (`clients`)
* वर्तमान में प्रोसेस हो रहे कार्यों की संख्या
* प्रारंभ से निष्पादित क्वेरी की संख्या
* क्यू में नौकरियों की संख्या और कार्यों की संख्या, थ्रेड्स की संख्या द्वारा मानकीकृत

<!-- SQL अनुरोध -->
```sql
mysql> status
```

<!-- SQL प्रतिक्रिया -->
```sql
--------------
mysql  Ver 14.14 Distrib 5.7.30, for Linux (x86_64) using  EditLine wrapper

कनेक्शन आईडी:		378
वर्तमान डेटाबेस:	Manticore
वर्तमान उपयोगकर्ता:	Usual
SSL:			उपयोग में नहीं
वर्तमान पृष्ठ:		stdout
आउटफाइल का उपयोग:		''
विभाजक का उपयोग:	;
सर्वर संस्करण:		3.4.3 a48c61d6@200702 coroutines git branch coroutines_work_junk...origin/coroutines_work_junk
प्रोटोकॉल संस्करण:	10
कनेक्शन:		0 TCP/IP के माध्यम से
सर्वर वर्ण सेट:
Db     वर्ण सेट:
क्लाइंट वर्ण सेट:	utf8
कनेक्शन वर्ण सेट:	utf8
TCP पोर्ट:		8306
अपटाइम:			23 घंटे 6 सेकंड

थ्रेड्स: 12  क्यू: 3  क्लाइंट: 1  वीआईपी क्लाइंट: 0  कार्य: 5  क्वेरी: 318967  दीवार: 7h  CPU: 0us
क्यू/थ: 0.2  कार्य/थ: 0.4
--------------
```


<!-- अंत -->

## स्थिति दिखाएँ

```sql
SHOW STATUS [ LIKE pattern ]
```

<!-- उदाहरण स्थिति दिखाएँ -->

`SHOW STATUS` एक SQL वक्तव्य है जो विभिन्न सहायक प्रदर्शन कounters की जानकारी प्रस्तुत करता है। IO और CPU कounters केवल तब उपलब्ध होंगे जब `searchd` को `--iostats` और `--cpustats` स्विच के साथ प्रारंभ किया गया हो (या यदि उन्हें `SET GLOBAL iostats/cpustats=1` के माध्यम से सक्षम किया गया हो)।

<!-- परिचय -->
##### SQL:
<!-- SQL अनुरोध -->

```sql
SHOW STATUS;
```

<!-- SQL प्रतिक्रिया -->
```sql
+-------------------------------+------------------------------------------------------------------------------------------------------------------------------------------------+
| काउंटर                       | मान                                                                                                                                           |
+-------------------------------+------------------------------------------------------------------------------------------------------------------------------------------------+
| अपटाइम                        | 182                                                                                                                                            |
| कनेक्शन                     | 368                                                                                                                                            |
| अधिकतम सीमा                     | 0                                                                                                                                              |
| संस्करण                       | 6.3.7 b10359434@24100213 dev (columnar 2.3.1 bd59d08@24093015) (secondary 2.3.1 bd59d08@24093015) (knn 2.3.1 bd59d08@24093015) (buddy v2.3.13) |
| mysql_version                 | 6.3.7 b10359434@24100213 dev (columnar 2.3.1 bd59d08@24093015) (secondary 2.3.1 bd59d08@24093015) (knn 2.3.1 bd59d08@24093015)                 |
| command_search                | 4                                                                                                                                              |
| command_excerpt               | 0                                                                                                                                              |
| command_update                | 2                                                                                                                                              |
| command_keywords              | 0                                                                                                                                              |
| command_persist               | 0                                                                                                                                              |
| command_status                | 8                                                                                                                                              |
| command_flushattrs            | 0                                                                                                                                              |
| command_sphinxql              | 0                                                                                                                                              |
| command_ping                  | 0                                                                                                                                              |
| command_delete                | 3                                                                                                                                              |
| command_set                   | 0                                                                                                                                              |
| command_insert                | 3                                                                                                                                              |
| command_replace               | 3                                                                                                                                              |
| command_commit                | 0                                                                                                                                              |
| command_suggest               | 0                                                                                                                                              |
| command_json                  | 0                                                                                                                                              |
| command_callpq                | 0                                                                                                                                              |
| command_cluster               | 0                                                                                                                                              |
| command_getfield              | 0                                                                                                                                              |
| insert_replace_stats_ms_avg   | 42.625 35.895 35.895                                                                                                                           |
| insert_replace_stats_ms_min   | 0.096 0.096 0.096                                                                                                                              |
| insert_replace_stats_ms_max   | 85.154 85.154 85.154                                                                                                                           |
| insert_replace_stats_ms_pct95 | 42.625 42.625 42.625                                                                                                                           |
| insert_replace_stats_ms_pct99 | 42.625 42.625 42.625                                                                                                                           |
| search_stats_ms_avg           | 0.209 0.188 0.188                                                                                                                              |
| search_stats_ms_min           | 0.205 0.058 0.058                                                                                                                              |
| search_stats_ms_max           | 0.214 0.278 0.278                                                                                                                              |
| search_stats_ms_pct95         | 0.214 0.278 0.278                                                                                                                              |
| search_stats_ms_pct99         | 0.214 0.278 0.278                                                                                                                              |
| update_stats_ms_avg           | 0.024 0.024 0.024                                                                                                                              |
| update_stats_ms_min           | 0.007 0.007 0.007                                                                                                                              |
| update_stats_ms_max           | 0.042 0.042 0.042                                                                                                                              |
| update_stats_ms_pct95         | 0.042 0.042 0.042                                                                                                                              |
| update_stats_ms_pct99         | 0.042 0.042 0.042                                                                                                                              |
| agent_connect                 | 0                                                                                                                                              |
| agent_tfo                     | 0                                                                                                                                              |
| agent_retry                   | 0                                                                                                                                              |
| queries                       | 6                                                                                                                                              |
| dist_queries                  | 0                                                                                                                                              |
| workers_total                 | 32                                                                                                                                             |
| workers_active                | 3                                                                                                                                              |
| workers_clients               | 1                                                                                                                                              |
| workers_clients_vip           | 0                                                                                                                                              |
| workers_clients_buddy         | 1                                                                                                                                              |
| work_queue_length             | 6                                                                                                                                              |
| load                          | 0.03 0.03 0.03                                                                                                                                 |
| load_primary                  | 0.00 0.00 0.00                                                                                                                                 |
| load_secondary                | 0.00 0.00 0.00                                                                                                                                 |
| query_wall                    | 0.000                                                                                                                                          |
| query_cpu                     | OFF                                                                                                                                            |
| dist_wall                     | 0.000                                                                                                                                          |
| dist_local                    | 0.000                                                                                                                                          |
| dist_wait                     | 0.000                                                                                                                                          |
| query_reads                   | OFF                                                                                                                                            |
| query_readkb                  | OFF                                                                                                                                            |
| query_readtime                | OFF                                                                                                                                            |
| avg_query_wall                | 0.000                                                                                                                                          |
| avg_query_cpu                 | OFF                                                                                                                                            |
| avg_dist_wall                 | 0.000                                                                                                                                          |
| avg_dist_local                | 0.000                                                                                                                                          |
| avg_dist_wait                 | 0.000                                                                                                                                          |
| avg_query_reads               | OFF                                                                                                                                            |
| avg_query_readkb              | OFF                                                                                                                                            |
| avg_query_readtime            | OFF                                                                                                                                            |
| qcache_max_bytes              | 16777216                                                                                                                                       |
| qcache_thresh_msec            | 3000                                                                                                                                           |
| qcache_ttl_sec                | 60                                                                                                                                             |
| qcache_cached_queries         | 0                                                                                                                                              |
| qcache_used_bytes             | 0                                                                                                                                              |
| qcache_hits                   | 0                                                                                                                                              |
+-------------------------------+------------------------------------------------------------------------------------------------------------------------------------------------+
```

<!-- end -->

<!-- example show status like -->

एक वैकल्पिक `LIKE` क्लॉज समर्थित है, जो आपको केवल उन वेरिएबल्स को चुनने की अनुमति देता है जो एक विशिष्ट पैटर्न से मेल खाते हैं। पैटर्न सिंटैक्स मानक SQL वाइल्डकार्ड का पालन करता है, जहां `%` किसी भी संख्या के किसी भी वर्ण का प्रतिनिधित्व करता है, और `_` एकल वर्ण का प्रतिनिधित्व करता है।

<!-- request qcache -->

```sql
SHOW STATUS LIKE 'qcache%';
```

<!-- response qcache -->
```sql
+-----------------------+-------+
| काउंटर                | मान   |
+-----------------------+-------+
| qcache_max_bytes      | 0     |
| qcache_thresh_msec    | 3000  |
| qcache_ttl_sec        | 60    |
| qcache_cached_queries | 0     |
| qcache_used_bytes     | 0     |
| qcache_hits           | 0     |
+-----------------------+-------+
```

<!-- request perf_stats -->

```sql
SHOW STATUS LIKE '%stats_ms%';
```

<!-- response perf_stats -->
```sql
+-------------------------------+-------------------+
| काउंटर                       | मान               |
+-------------------------------+-------------------+
| insert_replace_stats_ms_avg   | N/A 35.895 35.895 |
| insert_replace_stats_ms_min   | N/A 0.096 0.096   |
| insert_replace_stats_ms_max   | N/A 85.154 85.154 |
| insert_replace_stats_ms_pct95 | N/A 42.625 42.625 |
| insert_replace_stats_ms_pct99 | N/A 42.625 42.625 |
| search_stats_ms_avg           | N/A 0.188 0.188   |
| search_stats_ms_min           | N/A 0.058 0.058   |
| search_stats_ms_max           | N/A 0.278 0.278   |
| search_stats_ms_pct95         | N/A 0.278 0.278   |
| search_stats_ms_pct99         | N/A 0.278 0.278   |
| update_stats_ms_avg           | N/A 0.024 0.024   |
| update_stats_ms_min           | N/A 0.007 0.007   |
| update_stats_ms_max           | N/A 0.042 0.042   |
| update_stats_ms_pct95         | N/A 0.042 0.042   |
| update_stats_ms_pct99         | N/A 0.042 0.042   |
+-------------------------------+-------------------+
```

<!-- end -->

### क्वेरी समय सांख्यिकी

<!-- example show status like stats_ms -->

`SHOW STATUS` कमांड विभिन्न प्रदर्शन मैट्रिक्स पर विस्तृत रिपोर्ट प्रदान करती है Manticore में, जिसमें INSERT/REPLACE, खोज और अपडेट क्वेरी के लिए क्वेरी समय सांख्यिकी शामिल हैं। ये सांख्यिकी 1, 5 और 15 मिनट की स्लाइडिंग विंडो के माध्यम से कैलकुलेट की जाती हैं, जो औसत, न्यूनतम, अधिकतम, और 95 वें/99 वें प्रतिशत मूल्यों को क्वेरी समय के लिए दिखाती हैं।

ये मैट्रिक्स विशिष्ट समय इंटरवल में प्रदर्शन को ट्रैक करने में मदद करते हैं, जिससे क्वेरी प्रतिक्रिया समय में रुझानों को पहचानना और संभावित बाधाओं को ढूंढना आसान हो जाता है।

निम्नलिखित मैट्रिक्स `SHOW STATUS` आउटपुट का हिस्सा हैं:
- `*_avg`: पिछले 1, 5, और 15 मिनट में प्रत्येक प्रकार की क्वेरी के लिए औसत क्वेरी समय।
- `*_min`: प्रत्येक क्वेरी प्रकार के लिए दर्ज किए गए सबसे छोटे क्वेरी समय।
- `*_max`: प्रत्येक क्वेरी प्रकार के लिए दर्ज किए गए सबसे लंबे क्वेरी समय।
- `*_pct95`: वह समय जिसके तहत 95% क्वेरियों का समापन किया गया है।
- `*_pct99`: वह समय जिसके तहत 99% क्वेरियों का समापन किया गया है।

ये सांख्यिकी INSERT/REPLACE (`insert_replace_stats_*`), खोज (`search_stats_*`), और अपडेट (`update_stats_*`) क्वेरियों के लिए अलग से प्रदान की जाती हैं, विभिन्न संचालन के प्रदर्शन पर विस्तृत अंतर्दृष्टि प्रदान करती हैं।

यदि निगरानी किए गए अंतराल के दौरान कोई क्वेरी निष्पादित नहीं की गई है, तो सिस्टम `N/A` प्रदर्शित करेगा।

<!-- request perf_stats -->

```sql
SHOW STATUS LIKE '%stats_ms%';
```

<!-- response perf_stats -->
```sql
+-------------------------------+-------------------+
| Counter                       | Value             |
+-------------------------------+-------------------+
| insert_replace_stats_ms_avg   | N/A 35.895 35.895 |
| insert_replace_stats_ms_min   | N/A 0.096 0.096   |
| insert_replace_stats_ms_max   | N/A 85.154 85.154 |
| insert_replace_stats_ms_pct95 | N/A 42.625 42.625 |
| insert_replace_stats_ms_pct99 | N/A 42.625 42.625 |
| search_stats_ms_avg           | N/A 0.188 0.188   |
| search_stats_ms_min           | N/A 0.058 0.058   |
| search_stats_ms_max           | N/A 0.278 0.278   |
| search_stats_ms_pct95         | N/A 0.278 0.278   |
| search_stats_ms_pct99         | N/A 0.278 0.278   |
| update_stats_ms_avg           | N/A 0.024 0.024   |
| update_stats_ms_min           | N/A 0.007 0.007   |
| update_stats_ms_max           | N/A 0.042 0.042   |
| update_stats_ms_pct95         | N/A 0.042 0.042   |
| update_stats_ms_pct99         | N/A 0.042 0.042   |
+-------------------------------+-------------------+
```

<!-- end -->

## SHOW SETTINGS

<!-- example show settings -->

`SHOW SETTINGS` एक SQL बयान है जो आपके कॉन्फ़िगरेशन फ़ाइल से वर्तमान सेटिंग्स दिखाता है। सेटिंग नामों का प्रतिनिधित्व निम्नलिखित प्रारूप में किया गया है: `'config_section_name'.'setting_name'`

परिणाम में दो अतिरिक्त मान भी शामिल होते हैं:
- `configuration_file` - कॉन्फ़िगरेशन फ़ाइल का पथ
- `worker_pid` - चल रहे `searchd` उदाहरण का प्रक्रिया ID

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
SHOW SETTINGS;
```

<!-- response SQL -->
```sql
+--------------------------+-------------------------------------+
| Setting_name             | Value                               |
+--------------------------+-------------------------------------+
| configuration_file       | /etc/manticoresearch/manticore.conf |
| worker_pid               | 658                                 |
| searchd.listen           | 0.0.0:9312                          |
| searchd.listen           | 0.0.0:9306:mysql                    |
| searchd.listen           | 0.0.0:9308:http                     |
| searchd.log              | /var/log/manticore/searchd.log      |
| searchd.query_log        | /var/log/manticore/query.log        |
| searchd.pid_file         | /var/run/manticore/searchd.pid      |
| searchd.data_dir         | /var/lib/manticore                  |
| searchd.query_log_format | sphinxql                            |
| searchd.binlog_path      | /var/lib/manticore/binlog           |
| common.plugin_dir        | /usr/local/lib/manticore            |
| common.lemmatizer_base   | /usr/share/manticore/morph/         |
+--------------------------+-------------------------------------+
13 rows in set (0.00 sec)
```

<!-- end -->

## SHOW AGENT STATUS
```sql
SHOW AGENT ['agent_or_index'] STATUS [ LIKE pattern ]
```

<!-- example SHOW AGENT STATUS -->

`SHOW AGENT STATUS` [रिमोट एजेंटों](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) या एक वितरित तालिका की सांख्यिकी प्रदर्शित करता है। इसमें पिछले अनुरोध की उम्र, अंतिम उत्तर, विभिन्न प्रकार की त्रुटियों और सफलताओं की संख्या आदि जैसे मान शामिल हैं। प्रत्येक एजेंट के लिए अंतिम 1, 5 और 15 अंतराल के लिए सांख्यिकी प्रदर्शित की जाती हैं, जिनमें से प्रत्येक [ha_period_karma](../Server_settings/Searchd.md#ha_period_karma) सेकंड में होता है।

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
SHOW AGENT STATUS;
```

<!-- response SQL -->
```sql
+------------------------------------+----------------------------+
| Variable_name                      | Value                      |
+------------------------------------+----------------------------+
| status_period_seconds              | 60                         |
| status_stored_periods              | 15                         |
| ag_0_hostname                      | 192.168.0.202:6713         |
| ag_0_references                    | 2                          |
| ag_0_lastquery                     | 0.41                       |
| ag_0_lastanswer                    | 0.19                       |
| ag_0_lastperiodmsec                | 222                        |
| ag_0_pingtripmsec                  | 10.521                     |
| ag_0_errorsarow                    | 0                          |
| ag_0_1periods_query_timeouts       | 0                          |
| ag_0_1periods_connect_timeouts     | 0                          |
| ag_0_1periods_connect_failures     | 0                          |
| ag_0_1periods_network_errors       | 0                          |
| ag_0_1periods_wrong_replies        | 0                          |
| ag_0_1periods_unexpected_closings  | 0                          |
| ag_0_1periods_warnings             | 0                          |
| ag_0_1periods_succeeded_queries    | 27                         |
| ag_0_1periods_msecsperquery        | 232.31                     |
| ag_0_5periods_query_timeouts       | 0                          |
| ag_0_5periods_connect_timeouts     | 0                          |
| ag_0_5periods_connect_failures     | 0                          |
| ag_0_5periods_network_errors       | 0                          |
| ag_0_5periods_wrong_replies        | 0                          |
| ag_0_5periods_unexpected_closings  | 0                          |
| ag_0_5periods_warnings             | 0                          |
| ag_0_5periods_succeeded_queries    | 146                        |
| ag_0_5periods_msecsperquery        | 231.83                     |
| ag_1_hostname                      | 192.168.0.202:6714         |
| ag_1_references                    | 2                          |
| ag_1_lastquery                     | 0.41                       |
| ag_1_lastanswer                    | 0.19                       |
| ag_1_lastperiodmsec                | 220                        |
| ag_1_pingtripmsec                  | 10.004                     |
| ag_1_errorsarow                    | 0                          |
| ag_1_1periods_query_timeouts       | 0                          |
| ag_1_1periods_connect_timeouts     | 0                          |
| ag_1_1periods_connect_failures     | 0                          |
| ag_1_1periods_network_errors       | 0                          |
| ag_1_1periods_wrong_replies        | 0                          |
| ag_1_1periods_unexpected_closings  | 0                          |
| ag_1_1periods_warnings             | 0                          |
| ag_1_1periods_succeeded_queries    | 27                         |
| ag_1_1periods_msecsperquery        | 231.24                     |
| ag_1_5periods_query_timeouts       | 0                          |
| ag_1_5periods_connect_timeouts     | 0                          |
| ag_1_5periods_connect_failures     | 0                          |
| ag_1_5periods_network_errors       | 0                          |
| ag_1_5periods_wrong_replies        | 0                          |
| ag_1_5periods_unexpected_closings  | 0                          |
| ag_1_5periods_warnings             | 0                          |
| ag_1_5periods_succeeded_queries    | 146                        |
| ag_1_5periods_msecsperquery        | 230.85                     |
+------------------------------------+----------------------------+
50 rows in set (0.01 sec)
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$client->nodes()->agentstatus();
```

<!-- response PHP -->

```php
Array(
	[status_period_seconds] => 60
	[status_stored_periods] => 15
	[ag_0_hostname] => 192.168.0.202:6713
	[ag_0_references] => 2
	[ag_0_lastquery] => 0.41
	[ag_0_lastanswer] => 0.19
	[ag_0_lastperiodmsec] => 222  
	[ag_0_errorsarow] => 0
	[ag_0_1periods_query_timeouts] => 0
	[ag_0_1periods_connect_timeouts] => 0
	[ag_0_1periods_connect_failures] => 0
	[ag_0_1periods_network_errors] => 0
	[ag_0_1periods_wrong_replies] => 0
	[ag_0_1periods_unexpected_closings] => 0
	[ag_0_1periods_warnings] => 0
	[ag_0_1periods_succeeded_queries] => 27
	[ag_0_1periods_msecsperquery] => 232.31
	[ag_0_5periods_query_timeouts] => 0
	[ag_0_5periods_connect_timeouts] => 0
	[ag_0_5periods_connect_failures] => 0
	[ag_0_5periods_network_errors] => 0
	[ag_0_5periods_wrong_replies] => 0
	[ag_0_5periods_unexpected_closings] => 0
	[ag_0_5periods_warnings] => 0
	[ag_0_5periods_succeeded_queries] => 146  
	[ag_0_5periods_msecsperquery] => 231.83
	[ag_1_hostname 192.168.0.202:6714
	[ag_1_references] => 2
	[ag_1_lastquery] => 0.41
	[ag_1_lastanswer] => 0.19
	[ag_1_lastperiodmsec] => 220  
	[ag_1_errorsarow] => 0
	[ag_1_1periods_query_timeouts] => 0
	[ag_1_1periods_connect_timeouts] => 0
	[ag_1_1periods_connect_failures] => 0
	[ag_1_1periods_network_errors] => 0
	[ag_1_1periods_wrong_replies] => 0
	[ag_1_1periods_unexpected_closings] => 0
	[ag_1_1periods_warnings] => 0
	[ag_1_1periods_succeeded_queries] => 27
	[ag_1_1periods_msecsperquery] => 231.24
	[ag_1_5periods_query_timeouts] => 0
	[ag_1_5periods_connect_timeouts] => 0
	[ag_1_5periods_connect_failures] => 0
	[ag_1_5periods_network_errors] => 0
	[ag_1_5periods_wrong_replies] => 0
	[ag_1_5periods_unexpected_closings
	[ag_1_5periods_warnings] => 0
	[ag_1_5periods_succeeded_queries] => 146  
	[ag_1_5periods_msecsperquery] => 230.85
)
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('SHOW AGENT STATUS')
```
<!-- response Python -->

```python
{u'columns': [{u'Key': {u'type': u'string'}},
              {u'Value': {u'type': u'string'}}],
 u'data': [
	{u'Key': u'status_period_seconds', u'Value': u'60'},
	{u'Key': u'status_stored_periods', u'Value': u'15'},
	{u'Key': u'ag_0_hostname', u'Value': u'192.168.0.202:6713'},
	{u'Key': u'ag_0_references', u'Value': u'2'},
	{u'Key': u'ag_0_lastquery', u'Value': u'0.41'},
	{u'Key': u'ag_0_lastanswer', u'Value': u'0.19'},
	{u'Key': u'ag_0_lastperiodmsec', u'Value': u'222'},
	{u'Key': u'ag_0_errorsarow', u'Value': u'0'},
	{u'Key': u'ag_0_1periods_query_timeouts', u'Value': u'0'},
	{u'Key': u'ag_0_1periods_connect_timeouts', u'Value': u'0'},
	{u'Key': u'ag_0_1periods_connect_failures', u'Value': u'0'},
	{u'Key': u'ag_0_1periods_network_errors', u'Value': u'0'},
	{u'Key': u'ag_0_1periods_wrong_replies', u'Value': u'0'},
	{u'Key': u'ag_0_1periods_unexpected_closings', u'Value': u'0'},
	{u'Key': u'ag_0_1periods_warnings', u'Value': u'0'},
	{u'Key': u'ag_0_1periods_succeeded_queries', u'Value': u'27'},
	{u'Key': u'ag_0_1periods_msecsperquery', u'Value': u'232.31'},
	{u'Key': u'ag_0_5periods_query_timeouts', u'Value': u'0'},
	{u'Key': u'ag_0_5periods_connect_timeouts', u'Value': u'0'},
	{u'Key': u'ag_0_5periods_connect_failures', u'Value': u'0'},
	{u'Key': u'ag_0_5periods_network_errors', u'Value': u'0'},
	{u'Key': u'ag_0_5periods_wrong_replies', u'Value': u'0'},
	{u'Key': u'ag_0_5periods_unexpected_closings', u'Value': u'0'},
	{u'Key': u'ag_0_5periods_warnings', u'Value': u'0'},
	{u'Key': u'ag_0_5periods_succeeded_queries', u'Value': u'146'},
	{u'Key': u'ag_0_5periods_msecsperquery', u'Value': u'231.83'},
	{u'Key': u'ag_1_hostname 192.168.0.202:6714'},
	{u'Key': u'ag_1_references', u'Value': u'2'},
	{u'Key': u'ag_1_lastquery', u'Value': u'0.41'},
	{u'Key': u'ag_1_lastanswer', u'Value': u'0.19'},
	{u'Key': u'ag_1_lastperiodmsec', u'Value': u'220'},
	{u'Key': u'ag_1_errorsarow', u'Value': u'0'},
	{u'Key': u'ag_1_1periods_query_timeouts', u'Value': u'0'},
	{u'Key': u'ag_1_1periods_connect_timeouts', u'Value': u'0'},
	{u'Key': u'ag_1_1periods_connect_failures', u'Value': u'0'},
	{u'Key': u'ag_1_1periods_network_errors', u'Value': u'0'},
	{u'Key': u'ag_1_1periods_wrong_replies', u'Value': u'0'},
	{u'Key': u'ag_1_1periods_unexpected_closings', u'Value': u'0'},
	{u'Key': u'ag_1_1periods_warnings', u'Value': u'0'},
	{u'Key': u'ag_1_1periods_succeeded_queries', u'Value': u'27'},
	{u'Key': u'ag_1_1periods_msecsperquery', u'Value': u'231.24'},
	{u'Key': u'ag_1_5periods_query_timeouts', u'Value': u'0'},
	{u'Key': u'ag_1_5periods_connect_timeouts', u'Value': u'0'},
	{u'Key': u'ag_1_5periods_connect_failures', u'Value': u'0'},
	{u'Key': u'ag_1_5periods_network_errors', u'Value': u'0'},
	{u'Key': u'ag_1_5periods_wrong_replies', u'Value': u'0'},
	{u'Key': u'ag_1_5periods_warnings', u'Value': u'0'},
	{u'Key': u'ag_1_5periods_succeeded_queries', u'Value': u'146'},
	{u'Key': u'ag_1_5periods_msecsperquery', u'Value': u'230.85'}],
 u'error': u'',
 u'total': 0,
 u'warning': u''}
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql("SHOW AGENT STATUS");
```
<!-- response javascript -->

```javascript
{"columns": [{"Key": {"type": "string"}},
              {"Value": {"type": "string"}}],
 "data": [
	{"Key": "status_period_seconds", "Value": "60"},
	{"Key": "status_stored_periods", "Value": "15"},
	{"Key": "ag_0_hostname", "Value": "192.168.0.202:6713"},
	{"Key": "ag_0_references", "Value": "2"},
	{"Key": "ag_0_lastquery", "Value": "0.41"},
	{"Key": "ag_0_lastanswer", "Value": "0.19"},
	{"Key": "ag_0_lastperiodmsec", "Value": "222"},
	{"Key": "ag_0_errorsarow", "Value": "0"},
	{"Key": "ag_0_1periods_query_timeouts", "Value": "0"},
	{"Key": "ag_0_1periods_connect_timeouts", "Value": "0"},
	{"Key": "ag_0_1periods_connect_failures", "Value": "0"},
	{"Key": "ag_0_1periods_network_errors", "Value": "0"},
	{"Key": "ag_0_1periods_wrong_replies", "Value": "0"},
	{"Key": "ag_0_1periods_unexpected_closings", "Value": "0"},
	{"Key": "ag_0_1periods_warnings", "Value": "0"},
	{"Key": "ag_0_1periods_succeeded_queries", "Value": "27"},
	{"Key": "ag_0_1periods_msecsperquery", "Value": "232.31"},
	{"Key": "ag_0_5periods_query_timeouts", "Value": "0"},
	{"Key": "ag_0_5periods_connect_timeouts", "Value": "0"},
	{"Key": "ag_0_5periods_connect_failures", "Value": "0"},
	{"Key": "ag_0_5periods_network_errors", "Value": "0"},
	{"Key": "ag_0_5periods_wrong_replies", "Value": "0"},
	{"Key": "ag_0_5periods_unexpected_closings", "Value": "0"},
	{"Key": "ag_0_5periods_warnings", "Value": "0"},
	{"Key": "ag_0_5periods_succeeded_queries", "Value": "146"},
	{"Key": "ag_0_5periods_msecsperquery", "Value": "231.83"},
	{"Key": "ag_1_hostname 192.168.0.202:6714"},
	{"Key": "ag_1_references", "Value": "2"},
	{"Key": "ag_1_lastquery", "Value": "0.41"},
	{"Key": "ag_1_lastanswer", "Value": "0.19"},
	{"Key": "ag_1_lastperiodmsec", "Value": "220"},
	{"Key": "ag_1_errorsarow", "Value": "0"},
	{"Key": "ag_1_1periods_query_timeouts", "Value": "0"},
	{u'Key': u'ag_1_1periods_connect_timeouts', u'Value': u'0'},
	{u'Key': u'ag_1_1periods_connect_failures', u'Value': u'0'},
	{u'Key': u'ag_1_1periods_network_errors', u'Value': u'0'},
	{u'Key': u'ag_1_1periods_wrong_replies', u'Value': u'0'},
	{u'Key': u'ag_1_1periods_unexpected_closings', u'Value': u'0'},
	{u'Key': u'ag_1_1periods_warnings', u'Value': u'0'},
	{u'Key': u'ag_1_1periods_succeeded_queries', u'Value': u'27'},
	{u'Key': u'ag_1_1periods_msecsperquery', u'Value': u'231.24'},
	{u'Key': u'ag_1_5periods_query_timeouts', u'Value': u'0'},
	{u'Key': u'ag_1_5periods_connect_timeouts', u'Value': u'0'},
	{u'Key': u'ag_1_5periods_connect_failures', u'Value': u'0'},
	{u'Key': u'ag_1_5periods_network_errors', u'Value': u'0'},
	{u'Key': u'ag_1_5periods_wrong_replies', u'Value': u'0'},
	{u'Key': u'ag_1_5periods_warnings', u'Value': u'0'},
	{u'Key': u'ag_1_5periods_succeeded_queries', u'Value': u'146'},
	{u'Key': u'ag_1_5periods_msecsperquery', u'Value': u'230.85'}],
 "error": "",
 "total": 0,
 "warning": ""}
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("SHOW AGENT STATUS");
```
<!-- response Java -->

```java
{columns=[{ Key : { type=string }},
              { Value : { type=string }}],
  data : [
	{ Key=status_period_seconds ,  Value=60 },
	{ Key=status_stored_periods ,  Value=15 },
	{ Key=ag_0_hostname ,  Value=192.168.0.202:6713 },
	{ Key=ag_0_references ,  Value=2 },
	{ Key=ag_0_lastquery ,  Value=0.41 },
	{ Key=ag_0_lastanswer ,  Value=0.19 },
	{ Key=ag_0_lastperiodmsec ,  Value=222 },
	{ Key=ag_0_errorsarow ,  Value=0 },
	{ Key=ag_0_1periods_query_timeouts ,  Value=0 },
	{ Key=ag_0_1periods_connect_timeouts ,  Value=0 },
	{ Key=ag_0_1periods_connect_failures ,  Value=0 },
	{ Key=ag_0_1periods_network_errors ,  Value=0 },
	{ Key=ag_0_1periods_wrong_replies ,  Value=0 },
	{ Key=ag_0_1periods_unexpected_closings ,  Value=0 },
	{ Key=ag_0_1periods_warnings ,  Value=0 },
	{ Key=ag_0_1periods_succeeded_queries ,  Value=27 },
	{ Key=ag_0_1periods_msecsperquery ,  Value=232.31 },
	{ Key=ag_0_5periods_query_timeouts ,  Value=0 },
	{ Key=ag_0_5periods_connect_timeouts ,  Value=0 },
	{ Key=ag_0_5periods_connect_failures ,  Value=0 },
	{ Key=ag_0_5periods_network_errors ,  Value=0 },
	{ Key=ag_0_5periods_wrong_replies ,  Value=0 },
	{ Key=ag_0_5periods_unexpected_closings ,  Value=0 },
	{ Key=ag_0_5periods_warnings ,  Value=0 },
	{ Key=ag_0_5periods_succeeded_queries ,  Value=146 },
	{ Key=ag_0_5periods_msecsperquery ,  Value=231.83 },
	{ Key=ag_1_hostname 192.168.0.202:6714 },
	{ Key=ag_1_references ,  Value=2 },
	{ Key=ag_1_lastquery ,  Value=0.41 },
	{ Key=ag_1_lastanswer ,  Value=0.19 },
	{ Key=ag_1_lastperiodmsec ,  Value=220 },
	{ Key=ag_1_errorsarow ,  Value=0 },
	{ Key=ag_1_1periods_query_timeouts ,  Value=0 },
	{ Key=ag_1_1periods_connect_timeouts ,  Value=0 },
	{ Key=ag_1_1periods_connect_failures ,  Value=0 },
	{ Key=ag_1_1periods_network_errors ,  Value=0 },
	{ Key=ag_1_1periods_wrong_replies ,  Value=0 },
	{ Key=ag_1_1periods_unexpected_closings ,  Value=0 },
	{ Key=ag_1_1periods_warnings ,  Value=0 },
	{ Key=ag_1_1periods_succeeded_queries ,  Value=27 },
	{ Key=ag_1_1periods_msecsperquery ,  Value=231.24 },
	{ Key=ag_1_5periods_query_timeouts ,  Value=0 },
	{ Key=ag_1_5periods_connect_timeouts ,  Value=0 },
	{ Key=ag_1_5periods_connect_failures ,  Value=0 },
	{ Key=ag_1_5periods_network_errors ,  Value=0 },
	{ Key=ag_1_5periods_wrong_replies ,  Value=0 },
	{ Key=ag_1_5periods_warnings ,  Value=0 },
	{ Key=ag_1_5periods_succeeded_queries ,  Value=146 },
	{ Key=ag_1_5periods_msecsperquery ,  Value=230.85 }],
  error= ,
  total=0,
  warning= }
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("SHOW AGENT STATUS");
```
<!-- response C# -->

```clike
{columns=[{ Key : { type=string }},
              { Value : { type=string }}],
  data : [
	{ Key=status_period_seconds ,  Value=60 },
	{ Key=status_stored_periods ,  Value=15 },
	{ Key=ag_0_hostname ,  Value=192.168.0.202:6713 },
	{ Key=ag_0_references ,  Value=2 },
	{ Key=ag_0_lastquery ,  Value=0.41 },
	{ Key=ag_0_lastanswer ,  Value=0.19 },
	{ Key=ag_0_lastperiodmsec ,  Value=222 },
	{ Key=ag_0_errorsarow ,  Value=0 },
	{ Key=ag_0_1periods_query_timeouts ,  Value=0 },
	{ Key=ag_0_1periods_connect_timeouts ,  Value=0 },
	{ Key=ag_0_1periods_connect_failures ,  Value=0 },
	{ Key=ag_0_1periods_network_errors ,  Value=0 },
	{ Key=ag_0_1periods_wrong_replies ,  Value=0 },
	{ Key=ag_0_1periods_unexpected_closings ,  Value=0 },
	{ Key=ag_0_1periods_warnings ,  Value=0 },
	{ Key=ag_0_1periods_succeeded_queries ,  Value=27 },
	{ Key=ag_0_1periods_msecsperquery ,  Value=232.31 },
	{ Key=ag_0_5periods_query_timeouts ,  Value=0 },
	{ Key=ag_0_5periods_connect_timeouts ,  Value=0 },
	{ Key=ag_0_5periods_connect_failures ,  Value=0 },
	{ Key=ag_0_5periods_network_errors ,  Value=0 },
	{ Key=ag_0_5periods_wrong_replies ,  Value=0 },
	{ Key=ag_0_5periods_unexpected_closings ,  Value=0 },
	{ Key=ag_0_5periods_warnings ,  Value=0 },
	{ Key=ag_0_5periods_succeeded_queries ,  Value=146 },
	{ Key=ag_0_5periods_msecsperquery ,  Value=231.83 },
	{ Key=ag_1_hostname 192.168.0.202:6714 },
	{ Key=ag_1_references ,  Value=2 },
	{ Key=ag_1_lastquery ,  Value=0.41 },
	{ Key=ag_1_lastanswer ,  Value=0.19 },
	{ Key=ag_1_lastperiodmsec ,  Value=220 },
	{ Key=ag_1_errorsarow ,  Value=0 },
	{ Key=ag_1_1periods_query_timeouts ,  Value=0 },
	{ Key=ag_1_1periods_connect_timeouts ,  Value=0 },
	{ Key=ag_1_1periods_connect_failures ,  Value=0 },
	{ Key=ag_1_1periods_network_errors ,  Value=0 },
	{ Key=ag_1_1periods_wrong_replies ,  Value=0 },
	{ Key=ag_1_1periods_unexpected_closings ,  Value=0 },
	{ Key=ag_1_1periods_warnings ,  Value=0 },
	{ Key=ag_1_1periods_succeeded_queries ,  Value=27 },
	{ Key=ag_1_1periods_msecsperquery ,  Value=231.24 },
	{ Key=ag_1_5periods_query_timeouts ,  Value=0 },
	{ Key=ag_1_5periods_connect_timeouts ,  Value=0 },
	{ Key=ag_1_5periods_connect_failures ,  Value=0 },
	{ Key=ag_1_5periods_network_errors ,  Value=0 },
	{ Key=ag_1_5periods_wrong_replies ,  Value=0 },
	{ Key=ag_1_5periods_warnings ,  Value=0 },
	{ Key=ag_1_5periods_succeeded_queries ,  Value=146 },
	{ Key=ag_1_5periods_msecsperquery ,  Value=230.85 }],
  error="" ,
  total=0,
  warning="" }
```

<!-- intro -->
##### TypeScript:

<!-- request TypeScript -->

```typescript
res = await utilsApi.sql("SHOW AGENT STATUS");
```
<!-- response javascript -->

```typescript
{
	"columns":
	[{
		"Key":
		{
			"type": "string"
		}
	},
    {
    	"Value":
    	{
    		"type": "string"
    	}
    }],
 	"data":
 	[
		{"Key": "status_period_seconds", "Value": "60"},
		{"Key": "status_stored_periods", "Value": "15"},
		{"Key": "ag_0_hostname", "Value": "192.168.0.202:6713"},
		{"Key": "ag_0_references", "Value": "2"},
		{"Key": "ag_0_lastquery", "Value": "0.41"},
		{"Key": "ag_0_lastanswer", "Value": "0.19"},
		{"Key": "ag_0_lastperiodmsec", "Value": "222"},
		{"Key": "ag_0_errorsarow", "Value": "0"},
		{"Key": "ag_0_1periods_query_timeouts", "Value": "0"},
		{"Key": "ag_0_1periods_connect_timeouts", "Value": "0"},
		{"Key": "ag_0_1periods_connect_failures", "Value": "0"},
		{"Key": "ag_0_1periods_network_errors", "Value": "0"},
		{"Key": "ag_0_1periods_wrong_replies", "Value": "0"},
		{"Key": "ag_0_1periods_unexpected_closings", "Value": "0"},
		{"Key": "ag_0_1periods_warnings", "Value": "0"},
		{"Key": "ag_0_1periods_succeeded_queries", "Value": "27"},
		{"Key": "ag_0_1periods_msecsperquery", "Value": "232.31"},
		{"Key": "ag_0_5periods_query_timeouts", "Value": "0"},
		{"Key": "ag_0_5periods_connect_timeouts", "Value": "0"},
		{"Key": "ag_0_5periods_connect_failures", "Value": "0"},
		{"Key": "ag_0_5periods_network_errors", "Value": "0"},
		{"Key": "ag_0_5periods_wrong_replies", "Value": "0"},
		{"Key": "ag_0_5periods_unexpected_closings", "Value": "0"},
		{"Key": "ag_0_5periods_warnings", "Value": "0"},
		{"Key": "ag_0_5periods_succeeded_queries", "Value": "146"},
		{"Key": "ag_0_5periods_msecsperquery", "Value": "231.83"},
		{"Key": "ag_1_hostname 192.168.0.202:6714"},
		{"Key": "ag_1_references", "Value": "2"},
		{"Key": "ag_1_lastquery", "Value": "0.41"},
		{"Key": "ag_1_lastanswer", "Value": "0.19"},
		{"Key": "ag_1_lastperiodmsec", "Value": "220"},
		{"Key": "ag_1_errorsarow", "Value": "0"},
		{"Key": "ag_1_1periods_query_timeouts", "Value": "0"},
		{"Key": "ag_1_1periods_connect_timeouts", "Value": "0"},
		{"Key": "ag_1_1periods_connect_failures", "Value": "0"},
		{"Key": "ag_1_1periods_network_errors", "Value": "0"},
		{"Key": "ag_1_1periods_wrong_replies", "Value": "0"},
		{"Key": "ag_1_1periods_unexpected_closings", "Value": "0"},
		{"Key": "ag_1_1periods_warnings", "Value": "0"},
		{"Key": "ag_1_1periods_succeeded_queries", "Value": "27"},
		{"Key": "ag_1_1periods_msecsperquery", "Value": "231.24"},
		{"Key": "ag_1_5periods_query_timeouts", "Value": "0"},
		{"Key": "ag_1_5periods_connect_timeouts", "Value": "0"},
		{"Key": "ag_1_5periods_connect_failures", "Value": "0"},
		{"Key": "ag_1_5periods_network_errors", "Value": "0"},
		{"Key": "ag_1_5periods_wrong_replies", "Value": "0"},
		{"Key": "ag_1_5periods_warnings", "Value": "0"},
		{"Key": "ag_1_5periods_succeeded_queries", "Value": "146"},
		{"Key": "ag_1_5periods_msecsperquery", "Value": "230.85"}
	],
 	"error": "",
 	"total": 0,
 	"warning": ""
 }
```

<!-- intro -->
##### जाओ:

<!-- request Go -->

```go
res := apiClient.UtilsAPI.Sql(context.Background()).Body("SHOW AGENT STATUS").Execute()
```
<!-- response Go -->

```go
{
	"columns":
	[{
		"Key":
		{
			"type": "string"
		}
	},
    {
    	"Value":
    	{
    		"type": "string"
    	}
    }],
 	"data":
 	[
		{"Key": "status_period_seconds", "Value": "60"},
		{"Key": "status_stored_periods", "Value": "15"},
		{"Key": "ag_0_hostname", "Value": "192.168.0.202:6713"},
		{"Key": "ag_0_references", "Value": "2"},
		{"Key": "ag_0_lastquery", "Value": "0.41"},
		{"Key": "ag_0_lastanswer", "Value": "0.19"},
		{"Key": "ag_0_lastperiodmsec", "Value": "222"},
		{"Key": "ag_0_errorsarow", "Value": "0"},
		{"Key": "ag_0_1periods_query_timeouts", "Value": "0"},
		{"Key": "ag_0_1periods_connect_timeouts", "Value": "0"},
		{"Key": "ag_0_1periods_connect_failures", "Value": "0"},
		{"Key": "ag_0_1periods_network_errors", "Value": "0"},
		{"Key": "ag_0_1periods_wrong_replies", "Value": "0"},
		{"Key": "ag_0_1periods_unexpected_closings", "Value": "0"},
		{"Key": "ag_0_1periods_warnings", "Value": "0"},
		{"Key": "ag_0_1periods_succeeded_queries", "Value": "27"},
		{"Key": "ag_0_1periods_msecsperquery", "Value": "232.31"},
		{"Key": "ag_0_5periods_query_timeouts", "Value": "0"},
		{"Key": "ag_0_5periods_connect_timeouts", "Value": "0"},
		{"Key": "ag_0_5periods_connect_failures", "Value": "0"},
		{"Key": "ag_0_5periods_network_errors", "Value": "0"},
		{"Key": "ag_0_5periods_wrong_replies", "Value": "0"},
		{"Key": "ag_0_5periods_unexpected_closings", "Value": "0"},
		{"Key": "ag_0_5periods_warnings", "Value": "0"},
		{"Key": "ag_0_5periods_succeeded_queries", "Value": "146"},
		{"Key": "ag_0_5periods_msecsperquery", "Value": "231.83"},
		{"Key": "ag_1_hostname 192.168.0.202:6714"},
		{"Key": "ag_1_references", "Value": "2"},
		{"Key": "ag_1_lastquery", "Value": "0.41"},
		{"Key": "ag_1_lastanswer", "Value": "0.19"},
		{"Key": "ag_1_lastperiodmsec", "Value": "220"},
		{"Key": "ag_1_errorsarow", "Value": "0"},
		{"Key": "ag_1_1periods_query_timeouts", "Value": "0"},
		{"Key": "ag_1_1periods_connect_timeouts", "Value": "0"},
		{"Key": "ag_1_1periods_connect_failures", "Value": "0"},
		{"Key": "ag_1_1periods_network_errors", "Value": "0"},
		{"Key": "ag_1_1periods_wrong_replies", "Value": "0"},
		{"Key": "ag_1_1periods_unexpected_closings", "Value": "0"},
		{"Key": "ag_1_1periods_warnings", "Value": "0"},
		{"Key": "ag_1_1periods_succeeded_queries", "Value": "27"},
		{"Key": "ag_1_1periods_msecsperquery", "Value": "231.24"},
		{"Key": "ag_1_5periods_query_timeouts", "Value": "0"},
		{"Key": "ag_1_5periods_connect_timeouts", "Value": "0"},
		{"Key": "ag_1_5periods_connect_failures", "Value": "0"},
		{"Key": "ag_1_5periods_network_errors", "Value": "0"},
		{"Key": "ag_1_5periods_wrong_replies", "Value": "0"},
		{"Key": "ag_1_5periods_warnings", "Value": "0"},
		{"Key": "ag_1_5periods_succeeded_queries", "Value": "146"},
		{"Key": "ag_1_5periods_msecsperquery", "Value": "230.85"}
	],
 	"error": "",
 	"total": 0,
 	"warning": ""
 }
```

<!-- end -->


<!-- example SHOW AGENT LIKE -->

एक वैकल्पिक `LIKE` क्लॉज़ समर्थित है, जिसका सSyntax `SHOW STATUS` में समान है।
<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
SHOW AGENT STATUS LIKE '%5period%msec%';
```

<!-- response SQL -->
```sql
+-----------------------------+--------+
| Key                         | Value  |
+-----------------------------+--------+
| ag_0_5periods_msecsperquery | 234.72 |
| ag_1_5periods_msecsperquery | 233.73 |
| ag_2_5periods_msecsperquery | 343.81 |
+-----------------------------+--------+
3 rows in set (0.00 sec)
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$client->nodes()->agentstatus(
    ['body'=>
        ['pattern'=>'%5period%msec%']
    ]
);
```

<!-- response PHP -->

```php
Array(
    [ag_0_5periods_msecsperquery] => 234.72
    [ag_1_5periods_msecsperquery] => 233.73
    [ag_2_5periods_msecsperquery] => 343.81
)
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('SHOW AGENT STATUS LIKE \'%5period%msec%\'')
```
<!-- response Python -->

```python
{u'columns': [{u'Key': {u'type': u'string'}},
              {u'Value': {u'type': u'string'}}],
 u'data': [
	{u'Key': u'ag_0_5periods_msecsperquery', u'Value': u'234.72'},
	{u'Key': u'ag_1_5periods_msecsperquery', u'Value': u'233.73'},
	{u'Key': u'ag_2_5periods_msecsperquery', u'Value': u'343.81'}],
 u'error': u'',
 u'total': 0,
 u'warning': u''}
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql("SHOW AGENT STATUS LIKE \"%5period%msec%\"");
```
<!-- response javascript -->

```javascript
{"columns": [{"Key": {"type": "string"}},
              {"Value": {"type": "string"}}],
 "data": [
	{"Key": "ag_0_5periods_msecsperquery", "Value": "234.72"},
	{"Key": "ag_1_5periods_msecsperquery", "Value": "233.73"},
	{"Key": "ag_2_5periods_msecsperquery", "Value": "343.81"}],
 "error": "",
 "total": 0,
 "warning": ""}
```


<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("SHOW AGENT STATUS LIKE \"%5period%msec%\"");
```
<!-- response Java -->

```java
{columns: [{Key={type=string}},
              {Value={type=string}}],
 data: [
	{Key=ag_0_5periods_msecsperquery, Value=234.72},
	{Key=ag_1_5periods_msecsperquery, Value=233.73},
	{Key=ag_2_5periods_msecsperquery, Value=343.81}],
 error: ,
 total: 0,
 warning: }
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("SHOW AGENT STATUS LIKE \"%5period%msec%\"");
```
<!-- response C# -->

```clike
{columns: [{Key={type=string}},
              {Value={type=string}}],
 data: [
	{Key=ag_0_5periods_msecsperquery, Value=234.72},
	{Key=ag_1_5periods_msecsperquery, Value=233.73},
	{Key=ag_2_5periods_msecsperquery, Value=343.81}],
 error: "",
 total: 0,
 warning: ""}
```

<!-- intro -->
##### TypeScript:

<!-- request TypeScript -->

```typescript
res = await utilsApi.sql("SHOW AGENT STATUS LIKE \"%5period%msec%\"");
```
<!-- response TypeScript -->

```typescript
{
	"columns":
	[{
		"Key": {"type": "string"}
	},
    {
    	"Value": {"type": "string"}
    }],
	"data":
	[
		{"Key": "ag_0_5periods_msecsperquery", "Value": "234.72"},
		{"Key": "ag_1_5periods_msecsperquery", "Value": "233.73"},
		{"Key": "ag_2_5periods_msecsperquery", "Value": "343.81"}
	],
 	"error": "",
 	"total": 0,
 	"warning": ""
}
```

<!-- intro -->
##### Go:

<!-- request Go -->

```go
apiClient.UtilsAPI.Sql(context.Background()).Body("SHOW AGENT STATUS LIKE \"%5period%msec%\"").Execute()
```
<!-- response Go -->

```go
{
	"columns":
	[{
		"Key": {"type": "string"}
	},
    {
    	"Value": {"type": "string"}
    }],
	"data":
	[
		{"Key": "ag_0_5periods_msecsperquery", "Value": "234.72"},
		{"Key": "ag_1_5periods_msecsperquery", "Value": "233.73"},
		{"Key": "ag_2_5periods_msecsperquery", "Value": "343.81"}
	],
 	"error": "",
 	"total": 0,
 	"warning": ""
}
```

<!-- end -->


<!-- example show specific agent -->

You can specify a particular agent by its address. In this case, only that agent's data will be displayed. Additionally, the `agent_` prefix will be used instead of `ag_N_`:

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
SHOW AGENT '192.168.0.202:6714' STATUS LIKE '%15periods%';
```

<!-- response SQL -->

```sql
+-------------------------------------+--------+
| Variable_name                       | Value  |
+-------------------------------------+--------+
| agent_15periods_query_timeouts      | 0      |
| agent_15periods_connect_timeouts    | 0      |
| agent_15periods_connect_failures    | 0      |
| agent_15periods_network_errors      | 0      |
| agent_15periods_wrong_replies       | 0      |
| agent_15periods_unexpected_closings | 0      |
| agent_15periods_warnings            | 0      |
| agent_15periods_succeeded_queries   | 439    |
| agent_15periods_msecsperquery       | 231.73 |
+-------------------------------------+--------+
9 rows in set (0.00 sec)
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$client->nodes()->agentstatus(
    ['body'=>
        ['agent'=>'192.168.0.202:6714'],
        ['pattern'=>'%5period%msec%']
    ]
);
```

<!-- response PHP -->

```php
Array(
    [agent_15periods_query_timeouts] => 0
    [agent_15periods_connect_timeouts] => 0
    [agent_15periods_connect_failures] => 0
    [agent_15periods_network_errors] => 0
    [agent_15periods_wrong_replies] => 0
    [agent_15periods_unexpected_closings] => 0
    [agent_15periods_warnings] => 0
    [agent_15periods_succeeded_queries] => 439
    [agent_15periods_msecsperquery] => 231.73
)
```


<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('SHOW AGENT \'192.168.0.202:6714\' STATUS LIKE \'%15periods%\'')
```
<!-- response Python -->

```python
{u'columns': [{u'Key': {u'type': u'string'}},
              {u'Value': {u'type': u'string'}}],
 u'data': [
	{u'Key': u'agent_15periods_query_timeouts', u'Value': u'0'},
	{u'Key': u'agent_15periods_connect_timeouts', u'Value': u'0'},
	{u'Key': u'agent_15periods_connect_failures', u'Value': u'0'},
    {u'Key': u'agent_15periods_network_errors', u'Value': u'0'},
    {u'Key': u'agent_15periods_connect_failures', u'Value': u'0'},
    {u'Key': u'agent_15periods_wrong_replies', u'Value': u'0'},
    {u'Key': u'agent_15periods_unexpected_closings', u'Value': u'0'},
    {u'Key': u'agent_15periods_warnings', u'Value': u'0'},
    {u'Key': u'agent_15periods_succeeded_queries', u'Value': u'439'},
    {u'Key': u'agent_15periods_msecsperquery', u'Value': u'233.73'},
    ],
 u'error': u'',
 u'total': 0,
 u'warning': u''}
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql("SHOW AGENT \"192.168.0.202:6714\" STATUS LIKE \"%15periods%\"");
```
<!-- response javascript -->

```javascript
{"columns": [{"Key": {"type": "string"}},
              {"Value": {"type": "string"}}],
 "data": [
	{"Key": "agent_15periods_query_timeouts", "Value": "0"},
	{"Key": "agent_15periods_connect_timeouts", "Value": "0"},
	{"Key": "agent_15periods_connect_failures", "Value": "0"},
    {"Key": "agent_15periods_network_errors", "Value": "0"},
    {"Key": "agent_15periods_connect_failures", "Value": "0"},
    {"Key": "agent_15periods_wrong_replies", "Value": "0"},
    {"Key": "agent_15periods_unexpected_closings", "Value": "0"},
    {"Key": "agent_15periods_warnings", "Value": "0"},
    {"Key": "agent_15periods_succeeded_queries", "Value": "439"},
    {"Key": "agent_15periods_msecsperquery", "Value": "233.73"},
    ],
 "error": "",
 "total": 0,
 "warning": ""}
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("SHOW AGENT \"192.168.0.202:6714\" STATUS LIKE \"%15periods%\"");
```
<!-- response Java -->

```java
{columns=[{Key={type=string}},
              {Value={type=string}}],
 data=[
	{Key=agent_15periods_query_timeouts, Value=0},
	{Key=agent_15periods_connect_timeouts, Value=0},
	{Key=agent_15periods_connect_failures, Value=0},
    {Key=agent_15periods_network_errors, Value=0},
    {Key=agent_15periods_connect_failures, Value=0},
    {Key=agent_15periods_wrong_replies, Value=0},
    {Key=agent_15periods_unexpected_closings, Value=0},
    {Key=agent_15periods_warnings, Value=0},
    {Key=agent_15periods_succeeded_queries, Value=439},
    {Key=agent_15periods_msecsperquery, Value=233.73},
    ],
 error=,
 total=0,
 warning=}
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("SHOW AGENT \"192.168.0.202:6714\" STATUS LIKE \"%15periods%\"");
```
<!-- response C# -->

```clike
{columns=[{Key={type=string}},
              {Value={type=string}}],
 data=[
	{Key=agent_15periods_query_timeouts, Value=0},
	{Key=agent_15periods_connect_timeouts, Value=0},
	{Key=agent_15periods_connect_failures, Value=0},
    {Key=agent_15periods_network_errors, Value=0},
    {Key=agent_15periods_connect_failures, Value=0},
    {Key=agent_15periods_wrong_replies, Value=0},
    {Key=agent_15periods_unexpected_closings, Value=0},
    {Key=agent_15periods_warnings, Value=0},
    {Key=agent_15periods_succeeded_queries, Value=439},
    {Key=agent_15periods_msecsperquery, Value=233.73},
    ],
 error="",
 total=0,
 warning=""}
```

<!-- intro -->
##### TypeScript:

<!-- request TypeScript -->

```typescript
res = await utilsApi.sql("SHOW AGENT \"192.168.0.202:6714\" STATUS LIKE \"%15periods%\"");
```
<!-- response TypeScript -->

```typescript
{
	"columns":
	[{
		{"Key": {"type": "string"}
	},
    	{"Value": {"type": "string"}
    }],
	"data":
	[
		{"Key": "agent_15periods_query_timeouts", "Value": "0"},
		{"Key": "agent_15periods_connect_timeouts", "Value": "0"},
		{"Key": "agent_15periods_connect_failures", "Value": "0"},
	    {"Key": "agent_15periods_network_errors", "Value": "0"},
	    {"Key": "agent_15periods_connect_failures", "Value": "0"},
	    {"Key": "agent_15periods_wrong_replies", "Value": "0"},
	    {"Key": "agent_15periods_unexpected_closings", "Value": "0"},
	    {"Key": "agent_15periods_warnings", "Value": "0"},
	    {"Key": "agent_15periods_succeeded_queries", "Value": "439"},
	    {"Key": "agent_15periods_msecsperquery", "Value": "233.73"},
    ],
	"error": "",
	"total": 0,
	"warning": ""
}
```

<!-- intro -->
##### Go:

<!-- request Go -->

```go
apiClient.UtilsAPI.Sql(context.Background()).Body("SHOW AGENT \"192.168.0.202:6714\" STATUS LIKE \"%15periods%\").Execute()
```
<!-- response Go -->

```go
{
	"columns":
	[{
		{"Key": {"type": "string"}
	},
    	{"Value": {"type": "string"}
    }],
	"data":
	[
		{"Key": "agent_15periods_query_timeouts", "Value": "0"},
		{"Key": "agent_15periods_connect_timeouts", "Value": "0"},
		{"Key": "agent_15periods_connect_failures", "Value": "0"},
	    {"Key": "agent_15periods_network_errors", "Value": "0"},
	    {"Key": "agent_15periods_connect_failures", "Value": "0"},
	    {"Key": "agent_15periods_wrong_replies", "Value": "0"},
	    {"Key": "agent_15periods_unexpected_closings", "Value": "0"},
	    {"Key": "agent_15periods_warnings", "Value": "0"},
	    {"Key": "agent_15periods_succeeded_queries", "Value": "439"},
	    {"Key": "agent_15periods_msecsperquery", "Value": "233.73"},
    ],
	"error": "",
	"total": 0,
	"warning": ""
}
```

<!-- end -->
<!-- example show agent table status -->

अंततः, आप `SHOW AGENT table_name STATUS` बयान का उपयोग करके किसी विशिष्ट वितरित तालिका में एजेंटों की स्थिति की जांच कर सकते हैं। यह बयान तालिका की HA स्थिति को प्रदर्शित करता है (यानी, क्या यह एजेंट मिरर का उपयोग करता है या नहीं) और मिरर के बारे में जानकारी प्रदान करता है, जिसमें: पता, ब्लैकहोल और स्थायी ध्वज, और चयन संभावना शामिल है, जिसका उपयोग तब होता है जब [वजनित संभावना रणनीतियों](../Creating_a_cluster/Remote_nodes/Load_balancing.md) में से एक लागू होता है।

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
SHOW AGENT dist_index STATUS;
```

<!-- response SQL -->
```sql
+--------------------------------------+--------------------------------+
| Variable_name                        | Value                          |
+--------------------------------------+--------------------------------+
| dstindex_1_is_ha                     | 1                              |
| dstindex_1mirror1_id                 | 192.168.0.202:6713:loc         |
| dstindex_1mirror1_probability_weight | 0.372864                       |
| dstindex_1mirror1_is_blackhole       | 0                              |
| dstindex_1mirror1_is_persistent      | 0                              |
| dstindex_1mirror2_id                 | 192.168.0.202:6714:loc         |
| dstindex_1mirror2_probability_weight | 0.374635                       |
| dstindex_1mirror2_is_blackhole       | 0                              |
| dstindex_1mirror2_is_persistent      | 0                              |
| dstindex_1mirror3_id                 | dev1.manticoresearch.com:6714:loc |
| dstindex_1mirror3_probability_weight | 0.252501                       |
| dstindex_1mirror3_is_blackhole       | 0                              |
| dstindex_1mirror3_is_persistent      | 0                              |
+--------------------------------------+--------------------------------+
13 rows in set (0.00 sec)
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$client->nodes()->agentstatus(
    ['body'=>
        ['agent'=>'dist_index']
    ]
);
```

<!-- response PHP -->

```php
Array(
    [dstindex_1_is_ha] => 1
    [dstindex_1mirror1_id] => 192.168.0.202:6713:loc
    [dstindex_1mirror1_probability_weight] => 0.372864
    [dstindex_1mirror1_is_blackhole] => 0
    [dstindex_1mirror1_is_persistent] => 0
    [dstindex_1mirror2_id] => 192.168.0.202:6714:loc
    [dstindex_1mirror2_probability_weight] => 0.374635
    [dstindex_1mirror2_is_blackhole] => 0
    [dstindex_1mirror2_is_persistent] => 0
    [dstindex_1mirror3_id] => dev1.manticoresearch.com:6714:loc
    [dstindex_1mirror3_probability_weight] => 0.252501
    [dstindex_1mirror3_is_blackhole] => 0
    [dstindex_1mirror3_is_persistent] => 0
)
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('SHOW AGENT \'192.168.0.202:6714\' STATUS LIKE \'%15periods%\'')
```
<!-- response Python -->

```python
{u'columns': [{u'Key': {u'type': u'string'}},
              {u'Value': {u'type': u'string'}}],
 u'data': [
	{u'Key': u'dstindex_1_is_ha', u'Value': u'1'},
	{u'Key': u'dstindex_1mirror1_id', u'Value': u'192.168.0.202:6713:loc'},
	{u'Key': u'dstindex_1mirror1_probability_weight', u'Value': u'0.372864'},
    {u'Key': u'dstindex_1mirror1_is_blackhole', u'Value': u'0'},
    {u'Key': u'dstindex_1mirror1_is_persistent', u'Value': u'0'},
    {u'Key': u'dstindex_1mirror2_id', u'Value': u'192.168.0.202:6714:loc'},
    {u'Key': u'dstindex_1mirror2_probability_weight', u'Value': u'0.374635'},
    {u'Key': u'dstindex_1mirror2_is_blackhole', u'Value': u'0'},
    {u'Key': u'dstindex_1mirror2_is_persistent', u'Value': u'439'},
    {u'Key': u'dstindex_1mirror3_id', u'Value': u'dev1.manticoresearch.com:6714:loc'},
    {u'Key': u'dstindex_1mirror3_probability_weight', u'Value': u' 0.252501'},
    {u'Key': u'dstindex_1mirror3_is_blackhole', u'Value': u'0'},
    {u'Key': u'dstindex_1mirror3_is_persistent', u'Value': u'439'}    
    ],
 u'error': u'',
 u'total': 0,
 u'warning': u''}
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql("SHOW AGENT \"192.168.0.202:6714\" STATUS LIKE \"%15periods%\"");
```
<!-- response javascript -->

```javascript
{"columns": [{"Key": {"type": "string"}},
              {"Value": {"type": "string"}}],
 "data": [
	{"Key": "dstindex_1_is_ha", "Value": "1"},
	{"Key": "dstindex_1mirror1_id", "Value": "192.168.0.202:6713:loc"},
	{"Key": "dstindex_1mirror1_probability_weight", "Value": "0.372864"},
    {"Key": "dstindex_1mirror1_is_blackhole", "Value": "0"},
    {"Key": "dstindex_1mirror1_is_persistent", "Value": "0"},
    {"Key": "dstindex_1mirror2_id", "Value": "192.168.0.202:6714:loc"},
    {"Key": "dstindex_1mirror2_probability_weight", "Value": "0.374635"},
    {"Key": "dstindex_1mirror2_is_blackhole", "Value": "0"},
    {"Key": "dstindex_1mirror2_is_persistent", "Value": "439"},
    {"Key": "dstindex_1mirror3_id", "Value": "dev1.manticoresearch.com:6714:loc"},
    {"Key": "dstindex_1mirror3_probability_weight", "Value": " 0.252501"},
    {"Key": "dstindex_1mirror3_is_blackhole", "Value": "0"},
    {"Key": "dstindex_1mirror3_is_persistent", "Value": "439"}    
    ],
 "error": "",
 "total": 0,
 "warning": ""}
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("SHOW AGENT \"192.168.0.202:6714\" STATUS LIKE \"%15periods%\"");
```
<!-- response Java -->

```java
{columns=[{Key={type=string}},
              {Value={type=string}}],
 data=[
	{Key=dstindex_1_is_ha, Value=1},
	{Key=dstindex_1mirror1_id, Value=192.168.0.202:6713:loc},
	{Key=dstindex_1mirror1_probability_weight, Value=0.372864},
    {Key=dstindex_1mirror1_is_blackhole, Value=0},
    {Key=dstindex_1mirror1_is_persistent, Value=0},
    {Key=dstindex_1mirror2_id, Value=192.168.0.202:6714:loc},
    {Key=dstindex_1mirror2_probability_weight, Value=0.374635},
    {Key=dstindex_1mirror2_is_blackhole, Value=0},
    {Key=dstindex_1mirror2_is_persistent, Value=439},
    {Key=dstindex_1mirror3_id, Value=dev1.manticoresearch.com:6714:loc},
    {Key=dstindex_1mirror3_probability_weight, Value= 0.252501},
    {Key=dstindex_1mirror3_is_blackhole, Value=0},
    {Key=dstindex_1mirror3_is_persistent, Value=439}    
    ],
 error=,
 total=0,
 warning=}
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("SHOW AGENT \"192.168.0.202:6714\" STATUS LIKE \"%15periods%\"");
```
<!-- response C# -->

```clike
{columns=[{Key={type=string}},
              {Value={type=string}}],
 data=[
	{Key=dstindex_1_is_ha, Value=1},
	{Key=dstindex_1mirror1_id, Value=192.168.0.202:6713:loc},
	{Key=dstindex_1mirror1_probability_weight, Value=0.372864},
    {Key=dstindex_1mirror1_is_blackhole, Value=0},
    {Key=dstindex_1mirror1_is_persistent, Value=0},
    {Key=dstindex_1mirror2_id, Value=192.168.0.202:6714:loc},
    {Key=dstindex_1mirror2_probability_weight, Value=0.374635},
    {Key=dstindex_1mirror2_is_blackhole, Value=0},
    {Key=dstindex_1mirror2_is_persistent, Value=439},
    {Key=dstindex_1mirror3_id, Value=dev1.manticoresearch.com:6714:loc},
    {Key=dstindex_1mirror3_probability_weight, Value= 0.252501},
    {Key=dstindex_1mirror3_is_blackhole, Value=0},
    {Key=dstindex_1mirror3_is_persistent, Value=439}    
    ],
 error="",
 total=0,
 warning=""}
```

<!-- intro -->
##### TypeScript:

<!-- request TypeScript -->

```typescript
res = await utilsApi.sql("SHOW AGENT \"192.168.0.202:6714\" STATUS LIKE \"%15periods%\"");
```
<!-- response TypeScript -->

```typescript
{
	"columns":
	[{
		"Key": {"type": "string"}},
        {"Value": {"type": "string"}
	}],
	"data":
	[
		{"Key": "dstindex_1_is_ha", "Value": "1"},
		{"Key": "dstindex_1mirror1_id", "Value": "192.168.0.202:6713:loc"},
		{"Key": "dstindex_1mirror1_probability_weight", "Value": "0.372864"},
	    {"Key": "dstindex_1mirror1_is_blackhole", "Value": "0"},
	    {"Key": "dstindex_1mirror1_is_persistent", "Value": "0"},
	    {"Key": "dstindex_1mirror2_id", "Value": "192.168.0.202:6714:loc"},
	    {"Key": "dstindex_1mirror2_probability_weight", "Value": "0.374635"},
	    {"Key": "dstindex_1mirror2_is_blackhole", "Value": "0"},
	    {"Key": "dstindex_1mirror2_is_persistent", "Value": "439"},
	    {"Key": "dstindex_1mirror3_id", "Value": "dev1.manticoresearch.com:6714:loc"},
	    {"Key": "dstindex_1mirror3_probability_weight", "Value": " 0.252501"},
	    {"Key": "dstindex_1mirror3_is_blackhole", "Value": "0"},
	    {"Key": "dstindex_1mirror3_is_persistent", "Value": "439"}    
    ],
	"error": "",
	"total": 0,
	"warning": ""
}
```

<!-- intro -->
##### Go:

<!-- request Go -->

```go
apiClient.UtilsAPI.Sql(context.Background()).Body("SHOW AGENT \"192.168.0.202:6714\" STATUS LIKE \"%15periods%\"").Execute()
```
<!-- response Go -->

```go
{
	"columns":
	[{
		"Key": {"type": "string"}},
        {"Value": {"type": "string"}
	}],
	"data":
	[
		{"Key": "dstindex_1_is_ha", "Value": "1"},
		{"Key": "dstindex_1mirror1_id", "Value": "192.168.0.202:6713:loc"},
		{"Key": "dstindex_1mirror1_probability_weight", "Value": "0.372864"},
	    {"Key": "dstindex_1mirror1_is_blackhole", "Value": "0"},
	    {"Key": "dstindex_1mirror1_is_persistent", "Value": "0"},
	    {"Key": "dstindex_1mirror2_id", "Value": "192.168.0.202:6714:loc"},
	    {"Key": "dstindex_1mirror2_probability_weight", "Value": "0.374635"},
	    {"Key": "dstindex_1mirror2_is_blackhole", "Value": "0"},
	    {"Key": "dstindex_1mirror2_is_persistent", "Value": "439"},
	    {"Key": "dstindex_1mirror3_id", "Value": "dev1.manticoresearch.com:6714:loc"},
	    {"Key": "dstindex_1mirror3_probability_weight", "Value": " 0.252501"},
	    {"Key": "dstindex_1mirror3_is_blackhole", "Value": "0"},
	    {"Key": "dstindex_1mirror3_is_persistent", "Value": "439"}    
    ],
	"error": "",
	"total": 0,
	"warning": ""
}
```

<!-- end -->

<!-- proofread -->

