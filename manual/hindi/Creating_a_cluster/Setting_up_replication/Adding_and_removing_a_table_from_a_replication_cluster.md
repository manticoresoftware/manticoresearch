# प्रतिकृति क्लस्टर से तालिका जोड़ना और हटाना

<!-- example adding and removing a table from a replication cluster 1 -->
`ALTER CLUSTER <cluster_name> ADD <table_name>[, <table_name>]` एक या अधिक मौजूदा स्थानीय तालिकाओं को क्लस्टर में जोड़ता है। जो नोड ALTER क्वेरी प्राप्त करता है वह तालिका(ओं) को क्लस्टर के अन्य नोड्स पर भेज देता है। क्लस्टर के अन्य नोड्स पर एक ही नाम वाली सभी स्थानीय तालिकाएँ नए तालिका(ओं) से प्रतिस्थापित हो जाती हैं।

जब तालिकाएँ प्रतिकृत हो जाती हैं, तो किसी भी नोड पर लिखने के कथन किए जा सकते हैं, लेकिन तालिका के नाम को क्लस्टर के नाम के साथ पहले लिखा जाना चाहिए, जैसे `INSERT INTO <clusterName>:<table_name>`।


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
ALTER CLUSTER click_query ADD clicks_daily_index
```

<!-- request JSON -->

```json
POST /cli -d "
ALTER CLUSTER click_query ADD clicks_daily_index
"
```

<!-- request PHP -->

```php
$params = [
  'cluster' => 'click_query',
  'body' => [
     'operation' => 'add',
     'table' => 'clicks_daily_index'
      
  ]
];
$response = $client->cluster()->alter($params);        
```


<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('ALTER CLUSTER click_query ADD clicks_daily_index')
```

<!-- response Python -->
```python
{u'error': u'', u'total': 0, u'warning': u''}
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('ALTER CLUSTER click_query ADD clicks_daily_index');
```

<!-- response javascript -->
```javascript
{"total":0,"error":"","warning":""}
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("ALTER CLUSTER click_query ADD clicks_daily_index");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("ALTER CLUSTER click_query ADD clicks_daily_index");
```

<!-- end -->

<!-- example adding and removing a table from a replication cluster 2 -->
`ALTER CLUSTER <cluster_name> DROP <table_name>[, <table_name>]` एक या अधिक मौजूदा तालिका(ओं) को भुला देता है, जिसका अर्थ है कि यह नोड्स पर तालिका(ओं) की फ़ाइलें नहीं हटाता, बल्कि केवल उन्हें निष्क्रिय, गैर-प्रतिकृत तालिका(ओं) बना देता है।

जब एक तालिका को क्लस्टर से हटाया जाता है, तो यह एक `local` तालिका बन जाती है, और लिखने के कथन को केवल तालिका के नाम का उपयोग करना होता है, जैसे `INSERT INTO <table_name>`, बिना क्लस्टर प्रीफिक्स के।


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
ALTER CLUSTER posts DROP weekly_index
```

<!-- request JSON -->

```json
POST /cli -d "
ALTER CLUSTER posts DROP weekly_index
"
```

<!-- request PHP -->

```php
$params = [
  'cluster' => 'posts',
  'body' => [
     'operation' => 'drop',
     'table' => 'weekly_index'
      
  ]
];
$response = $client->cluster->alter($params);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('ALTER CLUSTER posts DROP weekly_index')
```

<!-- response Python -->
```python
{u'error': u'', u'total': 0, u'warning': u''}
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('ALTER CLUSTER posts DROP weekly_index');
```

<!-- response javascript -->
```javascript
{"total":0,"error":"","warning":""}
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("ALTER CLUSTER posts DROP weekly_index");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("ALTER CLUSTER posts DROP weekly_index");
```

<!-- end -->
<!-- proofread -->
