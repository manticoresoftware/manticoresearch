# एक स्थानीय वितरण तालिका बना रहा है

<!-- example local_dist -->

Manticore Search में एक वितरित तालिका "मास्टर नोड" की तरह कार्य करती है जो मांगी गई क्वेरी को अन्य तालिकाओं को प्रॉक्सी करती है और जो प्रतिक्रियाएँ प्राप्त होती हैं, उनके merged परिणाम प्रदान करती है। तालिका अपने आप में कोई डेटा नहीं रखती है। यह स्थानीय तालिकाओं और अन्य सर्वरों पर स्थित तालिकाओं दोनों से कनेक्ट कर सकती है। यहाँ एक सरल वितरित तालिका का उदाहरण है:

<!-- intro -->
##### कॉन्फ़िगरेशन फ़ाइल:

<!-- request Configuration file -->
```ini
table index_dist {
  type  = distributed
  local = index1
  local = index2
  ...
 }
```

<!-- request RT mode -->
```sql
CREATE TABLE local_dist type='distributed' local='index1' local='index2';
```


<!-- request PHP -->

```php
$params = [
    'body' => [
        'settings' => [
            'type' => 'distributed',
            'local' => [
                'index1',
                'index2'
            ]
        ]
    ],
    'table' => 'products'
];
$index = new \Manticoresearch\Index($client);
$index->create($params);
```
<!-- intro -->
##### पायथन:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE local_dist type=\'distributed\' local=\'index1\' local=\'index2\'')
```
<!-- intro -->
##### जावास्क्रिप्ट:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE local_dist type=\'distributed\' local=\'index1\' local=\'index2\'');
```

<!-- intro -->
##### जावा:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE local_dist type='distributed' local='index1' local='index2'");
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE local_dist type='distributed' local='index1' local='index2'");
```

<!-- end -->
<!-- proofread -->
