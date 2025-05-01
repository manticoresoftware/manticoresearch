# डॉकर में मैनटिकोर शुरू करना और इसका उपयोग करना

यह छवि [मैनटिकोर पैकेज के वर्तमान संस्करण](https://manticoresearch.com/install/) पर आधारित है।

[comment]: # (the below should be in sync with https://github.com/manticoresoftware/docker/blob/master/README.md)

डिफ़ॉल्ट कॉन्फ़िगरेशन में एक नमूना रियल-टाइम तालिका शामिल है और डिफ़ॉल्ट पोर्ट पर सुनता है:

  * `9306` MySQL क्लाइंट से कनेक्शनों के लिए
  * `9308` HTTP के माध्यम से कनेक्शनों के लिए
  * `9312` बाइनरी प्रोटोकॉल के माध्यम से कनेक्शनों के लिए (उदाहरण के लिए, यदि आप एक क्लस्टर चला रहे हैं)

यह छवि MySQL, PostgreSQL, XML और CSV फ़ाइलों से डेटा सूचकांक बनाने के लिए सुविधाजनक पुस्तकालयों के साथ आती है।

## मैनटिकोर सर्च डॉकर छवि को कैसे चलाएँ

### त्वरित उपयोग

नीचे दिया गया सबसे सरल तरीका है ताकि आप मैनटिकोर को एक कंटेनर में शुरू कर सकें और MySQL क्लाइंट के माध्यम से इसमें लॉगिन कर सकें:

```bash
docker run --name manticore --rm -d manticoresearch/manticore && echo "Waiting for Manticore docker to start. Consider mapping the data_dir to make it start faster next time" && until docker logs manticore 2>&1 | grep -q "accepting connections"; do sleep 1; echo -n .; done && echo && docker exec -it manticore mysql && docker stop manticore
```

ध्यान दें कि MySQL क्लाइंट से बाहर निकलने पर मैनटिकोर कंटेनर बंद और हट जाएगा, जिससे कोई डेटा सुरक्षित नहीं रहेगा। उत्पादन वातावरण में मैनटिकोर का उपयोग करने के लिए जानकारी के लिए, कृपया नीचे देखें।

यह छवि एक नमूना टेबल के साथ आती है जिसे इस तरह लोड किया जा सकता है:

```sql
mysql> source /sandbox.sql
```

इसके अलावा, MySQL क्लाइंट के पास इसके इतिहास में कई नमूना प्रश्न हैं जिन्हें आप उपरोक्त तालिका पर चला सकते हैं, बस क्लाइंट में उप/डाउन कुंजियों का उपयोग करके उन्हें देखें और चलाएं।

### उत्पादन उपयोग


#### पोर्ट और माउंटिंग पॉइंट

डेटा स्थिरता के लिए `/var/lib/manticore/` फ़ोल्डर को स्थानीय स्टोरेज या किसी अन्य इच्छित स्टोरेज इंजन पर माउंट किया जाना चाहिए।

इंस्टेंस के अंदर कॉन्फ़िगरेशन फ़ाइल `/etc/manticoresearch/manticore.conf` पर स्थित है। कस्टम सेटिंग्स के लिए, इस फ़ाइल को आपके अपने कॉन्फ़िगरेशन फ़ाइल पर माउंट किया जाना चाहिए।

पोर्ट SQL/HTTP/Binary के लिए 9306/9308/9312 हैं, इन्हें इस बात के आधार पर उजागर करें कि आप मैनटिकोर का उपयोग कैसे करने वाले हैं। उदाहरण के लिए:

```bash
docker run --name manticore -v $(pwd)/data:/var/lib/manticore -p 127.0.0.1:9306:9306 -p 127.0.0.1:9308:9308 -d manticoresearch/manticore
```

या

```bash
docker run --name manticore -v $(pwd)/manticore.conf:/etc/manticoresearch/manticore.conf -v $(pwd)/data:/var/lib/manticore/ -p 127.0.0.1:9306:9306 -p 127.0.0.1:9308:9308 -d manticoresearch/manticore
```

सुनिश्चित करें कि यदि आप पोर्ट को बाहरी मेज़बानों के लिए उपलब्ध रखना चाहते हैं तो `127.0.0.1:` हटा दें।

#### मैनटिकोर कॉलमनार पुस्तकालय और मैनटिकोर बडी

मैनटिकोर सर्च डॉकर छवि पहले से स्थापित [मैनटिकोर कॉलमनार पुस्तकालय](https://github.com/manticoresoftware/columnar) और [मैनटिकोर बडी](https://github.com/manticoresoftware/manticoresearch-buddy) के साथ आती है।

#### डॉकर-कंपोज़

कई मामलों में, आप मैनटिकोर का उपयोग अन्य छवियों के साथ मिलाकर करना चाह सकते हैं जिन्हें डॉकर कंपोज़ YAML फ़ाइल में निर्दिष्ट किया गया है। नीचे मैनटिकोर सर्च के लिए डॉकर-कंपोज़.yml फ़ाइल में न्यूनतम अनुशंसित कॉन्फ़िगरेशन है:

```yaml
version: '2.2'

services:
  manticore:
    container_name: manticore
    image: manticoresearch/manticore
    restart: always
    ports:
      - 127.0.0.1:9306:9306
      - 127.0.0.1:9308:9308
    ulimits:
      nproc: 65535
      nofile:
         soft: 65535
         hard: 65535
      memlock:
        soft: -1
        hard: -1
    volumes:
      - ./data:/var/lib/manticore
#      - ./manticore.conf:/etc/manticoresearch/manticore.conf # uncomment if you use a custom config
```

9306 और 9308 के एक्सपोज़ किए गए पोर्ट का उपयोग करने के अलावा, आप `docker-compose exec manticore mysql` चलाकर इंस्टेंस में लॉगिन कर सकते हैं।

#### HTTP प्रोटोकॉल

HTTP प्रोटोकॉल पोर्ट 9308 पर उजागर होता है। आप पोर्ट को स्थानीय रूप से मैप कर सकते हैं और curl का उपयोग करके कनेक्ट कर सकते हैं:

```bash
docker run --name manticore -p 9308:9308 -d manticoresearch/manticore
```

<!-- example create -->
एक तालिका बनाएं:

<!-- request JSON -->
```json
POST /cli -d 'CREATE TABLE testrt ( title text, content text, gid integer)'
```
<!-- end -->
<!-- example insert -->
एक दस्तावेज़ डालें:

<!-- request JSON -->
```json
POST /insert
-d'{"table":"testrt","id":1,"doc":{"title":"Hello","content":"world","gid":1}}'
```
<!-- end -->
<!-- example search -->
एक सरल खोज करें:

<!-- request JSON -->
```json
POST /search -d '{"table":"testrt","query":{"match":{"*":"hello world"}}}'
```
<!-- end -->

### लॉगिंग

डिफ़ॉल्ट रूप से, सर्वर को लॉगिंग `/dev/stdout` पर भेजने के लिए सेट किया गया है, जिसे होस्ट से देखा जा सकता है:


```bash
docker logs manticore
```

क्वेरी लॉग को `QUERY_LOG_TO_STDOUT=true` वेरिएबल पास करके डॉकर लॉग में मोड़ दिया जा सकता है।


#### मल्टी-नोड क्लस्टर विद प्रतिकृति

दो नोड क्लस्टर को परिभाषित करने के लिए यहां एक सरल `docker-compose.yml` है:

```yaml
version: '2.2'

services:

  manticore-1:
    image: manticoresearch/manticore
    restart: always
    ulimits:
      nproc: 65535
      nofile:
         soft: 65535
         hard: 65535
      memlock:
        soft: -1
        hard: -1
    networks:
      - manticore
  manticore-2:
    image: manticoresearch/manticore
    restart: always
    ulimits:
      nproc: 65535
      nofile:
        soft: 65535
        hard: 65535
      memlock:
        soft: -1
        hard: -1
    networks:
      - manticore
networks:
  manticore:
    driver: bridge
```
* इसे शुरू करें: `docker-compose up`
* एक क्लस्टर बनाएँ:
  ```sql
  $ docker-compose exec manticore-1 mysql

  mysql> CREATE TABLE testrt ( title text, content text, gid integer);

  mysql> CREATE CLUSTER posts;
  Query OK, 0 rows affected (0.24 sec)
  mysql> ALTER CLUSTER posts ADD testrt;
  Query OK, 0 rows affected (0.07 sec)

  MySQL [(none)]> exit
  Bye
  ```
* 2nd उदाहरण पर क्लस्टर से जुड़ें
  ```sql
  $ docker-compose exec manticore-2 mysql

  mysql> JOIN CLUSTER posts AT 'manticore-1:9312';
  mysql> INSERT INTO posts:testrt(title,content,gid)  VALUES('hello','world',1);
  Query OK, 1 row affected (0.00 sec)

  MySQL [(none)]> exit
  Bye
  ```
* यदि आप अब पहले उदाहरण पर वापस जाते हैं, तो आप नया रिकॉर्ड देखेंगे:
  ```sql
  $ docker-compose exec manticore-1 mysql

  MySQL [(none)]> select * from testrt;
  +---------------------+------+-------+---------+
  | id                  | gid  | title | content |
  +---------------------+------+-------+---------+
  | 3891565839006040065 |    1 | hello | world   |
  +---------------------+------+-------+---------+
  1 row in set (0.00 sec)

  MySQL [(none)]> exit
  Bye
  ```

### मेमोरी लॉकिंग और सीमाएँ

Manticore उदाहरण के लिए डॉकर के डिफ़ॉल्ट ulimits को ओवरराइट करने की सिफारिश की जाती है:

```bash
 --ulimit nofile=65536:65536
```

सर्वोत्तम प्रदर्शन के लिए, तालिका के घटकों को मेमोरी में "mlocked" किया जा सकता है। जब Manticore Docker के तहत चलाया जाता है, तो उदाहरण को मेमोरी लॉकिंग की अनुमति देने के लिए अतिरिक्त विशेषाधिकार की आवश्यकता होती है। उदाहरण चलाते समय निम्नलिखित विकल्प जोड़े जाने चाहिए:

```bash
  --cap-add=IPC_LOCK --ulimit memlock=-1:-1
```

### डॉकर के साथ Manticore खोज को कॉन्फ़िगर करना

यदि आप तालिका परिभाषाओं के साथ एक कस्टम कॉन्फ़िगरेशन के साथ Manticore चलाना चाहते हैं, तो आपको उदाहरण में कॉन्फ़िगरेशन को माउंट करने की आवश्यकता होगी:

```bash
docker run --name manticore -v $(pwd)/manticore.conf:/etc/manticoresearch/manticore.conf -v $(pwd)/data/:/var/lib/manticore -p 127.0.0.1:9306:9306 -d manticoresearch/manticore
```

ध्यान में रखें कि कंटेनर के अंदर Manticore खोज उपयोगकर्ता `manticore` के तहत चलती है। तालिका फ़ाइलों के साथ संचालित करते समय (जैसे साधारण तालिकाओं को बनाना या घुमाना) को भी `manticore` के तहत किया जाना चाहिए। अन्यथा फ़ाइलें `root` के तहत बनाई जाएँगी और खोज डेमन के पास उन्हें खोलने के अधिकार नहीं होंगे। उदाहरण के लिए, यहाँ बताया गया है कि आप सभी तालिकाओं को कैसे घुमा सकते हैं:

```bash
docker exec -it manticore gosu manticore indexer --all --rotate
```

आप डॉकर पर्यावरण चर का उपयोग करके व्यक्तिगत `searchd` और `common` कॉन्फ़िगरेशन सेटिंग्स भी सेट कर सकते हैं।

सेटिंग्स को उनके अनुभाग नाम से पूर्व स्थापित किया जाना चाहिए, उदाहरण के लिए `mysql_version_string` के मामले में, चर को `searchd_mysql_version_string` नाम दिया जाना चाहिए:


```bash
docker run --name manticore  -p 127.0.0.1:9306:9306  -e searchd_mysql_version_string='5.5.0' -d manticoresearch/manticore
```

`listen` निदेशिका के मामले में, नए सुनने वाले इंटरफेस डीफॉल्ट के अलावा Docker चर `searchd_listen` का उपयोग करते हैं। कई इंटरफेस घोषित किए जा सकते हैं, जिसे एक सेमी-कोलन ("|") से अलग किया जाता है। केवल एक नेटवर्क पते पर सुनने के लिए, `$ip` (जो `hostname -i` से आंतरिक रूप से प्राप्त होता है) को पते के उपनाम के रूप में उपयोग किया जा सकता है।

उदाहरण के लिए `-e searchd_listen='9316:http|9307:mysql|$ip:5443:mysql_vip'` एक अतिरिक्त SQL इंटरफेस को पोर्ट 9307 पर, पोर्ट 5443 पर केवल उदाहरण के आईपी पर चलने वाला एक SQL VIP श्रोता, और पोर्ट 9316 पर एक HTTP श्रोता जोड़ देगा, जो क्रमशः 9306 और 9308 पर डीफॉल्ट के अलावा है।

```bash
$ docker run --rm -p 1188:9307  -e searchd_mysql_version_string='5.5.0' -e searchd_listen='9316:http|9307:mysql|$ip:5443:mysql_vip'  manticore
[Mon Aug 17 07:31:58.719 2020] [1] using config file '/etc/manticoresearch/manticore.conf' (9130 chars)...
listening on all interfaces for http, port=9316
listening on all interfaces for mysql, port=9307
listening on 172.17.0.17:5443 for VIP mysql
listening on all interfaces for mysql, port=9306
listening on UNIX socket /var/run/mysqld/mysqld.sock
listening on 172.17.0.17:9312 for sphinx
listening on all interfaces for http, port=9308
prereading 0 indexes
prereaded 0 indexes in 0.000 sec
accepting connections
```

#### स्टार्टअप फ्लैग्स

कस्टम स्टार्टअप फ्लैग्स के साथ Manticore को प्रारंभ करने के लिए, डॉकर रन का उपयोग करते समय उन्हें तर्कों के रूप में निर्दिष्ट करें। सुनिश्चित करें कि आप `searchd` कमांड को शामिल नहीं करते हैं और `--nodetach` फ्लैग शामिल करते हैं। यहाँ एक उदाहरण है:
```bash
docker run --name manticore --rm manticoresearch/manticore:latest --replay-flags=ignore-trx-errors --nodetach
```

#### गैर-रूट के तहत चलाना
डिफ़ॉल्ट रूप से, मुख्य Manticore प्रक्रिया `searchd` कंटेनर के अंदर उपयोगकर्ता `manticore` के तहत चल रही है, लेकिन स्क्रिप्ट जो कंटेनर शुरू करते समय चलती है, आपके डिफ़ॉल्ट डॉकर उपयोगकर्ता के तहत चलती है जो अधिकांश मामलों में `root` होता है। यदि आप ऐसा नहीं चाहते हैं, तो आप `docker ... --user manticore` या डॉकर कम्पोज़ yaml में `user: manticore` का उपयोग कर सकते हैं ताकि सब कुछ `manticore` के तहत चल सके। नीचे पढ़ें कि आप संभावित वॉल्यूम अनुमतियों की समस्या प्राप्त कर सकते हैं और इसे कैसे हल कर सकते हैं।

#### स्टार्टअप पर साधारण तालिकाएँ बनाना
अपने कस्टम कॉन्फ़िगरेशन फ़ाइल में निर्दिष्ट साधारण तालिकाओं का निर्माण करने के लिए, आप `CREATE_PLAIN_TABLES=1` पर्यावरण चर का उपयोग कर सकते हैं। यह Manticore शुरू होने से पहले `indexer --all` को निष्पादित करेगा। यह उपयोगी है यदि आप वॉल्यूम का उपयोग नहीं करते हैं, और आपकी तालिकाएँ फिर से बनाने के लिए सरल होती हैं।
```bash
docker run -e CREATE_PLAIN_TABLES=1 --name manticore -v $(pwd)/manticore.conf:/etc/manticoresearch/manticore.conf -p 9306:9306 -p 9308:9308 -d manticoresearch/manticore
```

### समस्या समाधान

#### माउंट किए गए वॉल्यूम के साथ अनुमतियों की समस्या

यदि आप गैर-रूट (उदाहरण के लिए `docker ... --user manticore` का उपयोग करके या डॉकर कम्पोज़ yaml में `user: manticore`) के तहत Manticore Search डॉकर चला रहे हैं, तो आपको अनुमतियों की समस्या का सामना करना पड़ सकता है, उदाहरण के लिए:
```bash
FATAL: directory /var/lib/manticore write error: failed to open /var/lib/manticore/tmp: Permission denied
```
यह तब हो सकता है क्योंकि उपयोगकर्ता जिसे कंटेनर के अंदर प्रक्रियाओं को चलाने के लिए उपयोग किया जाता है, आपके द्वारा कंटेनर पर माउंट किए गए निर्देशिका को संशोधित करने की अनुमति नहीं हो सकती है। इसे ठीक करने के लिए आप माउंट किए गए निर्देशिका का `chown` या `chmod` कर सकते हैं। यदि आप उपयोगकर्ता `manticore` के तहत कंटेनर चलाते हैं तो आपको यह करना होगा:
```bash
chown -R 999:999 data
```

क्योंकि उपयोगकर्ता `manticore` का ID कंटेनर के अंदर 999 है।

<!-- proofread -->


# Docker में Manticore शुरू करना और उपयोग करना

यह छवि [Manticore पैकेज के वर्तमान रिलीज़](https://manticoresearch.com/install/) पर आधारित है।

[comment]: # (the below should be in sync with https://github.com/manticoresoftware/docker/blob/master/README.md)

डिफ़ॉल्ट कॉन्फ़िगरेशन में एक नमूना रीयल-टाइम तालिका शामिल है और यह डिफ़ॉल्ट पोर्ट पर सुनता है:

  * `9306` MySQL क्लाइंट से कनेक्शन के लिए
  * `9308` HTTP के माध्यम से कनेक्शन के लिए
  * `9312` बाइनरी प्रोटोकॉल (जैसे कि यदि आप एक क्लस्टर चलाते हैं) के माध्यम से कनेक्शन के लिए

यह छवि MySQL, PostgreSQL, XML और CSV फ़ाइलों से डेटा को आसान ढंग से अनुक्रमित करने के लिए पुस्तकालयों के साथ आती है।

# Manticore सर्च Docker छवि कैसे चलाएं

## त्वरित उपयोग

नीचे Manticore को एक कंटेनर में शुरू करने और इसे mysql क्लाइंट के माध्यम से लॉग इन करने का सबसे सरल तरीका है:

```bash
docker run --name manticore --rm -d manticoresearch/manticore && echo "Manticore docker शुरू होने की प्रतीक्षा कर रहे हैं। अगली बार इसे तेज़ी से शुरू करने के लिए data_dir मैप करने पर विचार करें" && until docker logs manticore 2>&1 | grep -q "accepting connections"; do sleep 1; echo -n .; done && echo && docker exec -it manticore mysql && docker stop manticore
```

ध्यान दें कि MySQL क्लाइंट से बाहर निकलने पर, Manticore कंटेनर को रोका और हटा दिया जाएगा, जिसके परिणामस्वरूप कोई सहेजा गया डेटा नहीं होगा। उत्पादन वातावरण में Manticore का उपयोग करने के लिए जानकारी के लिए कृपया नीचे देखें।

यह छवि एक नमूना तालिका के साथ आती है जिसे इस प्रकार लोड किया जा सकता है:

```sql
mysql> source /sandbox.sql
```

इसके अलावा, mysql क्लाइंट के इतिहास में कई नमूना क्वेरी हैं जिन्हें आप ऊपर दी गई तालिका पर चला सकते हैं, बस क्लाइंट में Up/Down कुंजी का उपयोग करें उन्हें देखने और चलाने के लिए।

## उत्पादन उपयोग


### पोर्ट और माउंटिंग पॉइंट्स

डेटा निरंतरता के लिए फ़ोल्डर `/var/lib/manticore/` को स्थानीय संग्रहण या अन्य इच्छित संग्रहण इंजन पर माउंट किया जाना चाहिए।

उदाहरण में कॉन्फ़िगरेशन फ़ाइल `/etc/manticoresearch/manticore.conf` पर स्थित है। कस्टम सेटिंग्स के लिए, इस फ़ाइल को आपके अपने कॉन्फ़िगरेशन फ़ाइल पर माउंट किया जाना चाहिए।

पोर्ट 9306/9308/9312 SQL/HTTP/Binary के लिए हैं, उन्हें उस तरह से एक्सपोज़ करें जिस तरह आप Manticore का उपयोग करने जा रहे हैं। उदाहरण के लिए:

```bash
docker run --name manticore -v $(pwd)/data:/var/lib/manticore -p 127.0.0.1:9306:9306 -p 127.0.0.1:9308:9308 -d manticoresearch/manticore
```

या

```bash
docker run --name manticore -v $(pwd)/manticore.conf:/etc/manticoresearch/manticore.conf -v $(pwd)/data:/var/lib/manticore/ -p 127.0.0.1:9306:9306 -p 127.0.0.1:9308:9308 -d manticoresearch/manticore
```

सुनिश्चित करें कि आप `127.0.0.1:` हटा दें यदि आप चाहते हैं कि पोर्ट बाहरी होस्ट के लिए उपलब्ध हों।

### Manticore कॉलम नार पुस्तकालय और Manticorebuddy

Manticore सर्च Docker छवि में पहले से स्थापित [Manticore कॉलम नार पुस्तकालय](https://github.com/manticoresoftware/columnar) और [Manticore बडी](https://github.com/manticoresoftware/manticoresearch-buddy) शामिल हैं।

### Docker-compose

कई मामलों में, आप Docker Compose YAML फ़ाइल में निर्दिष्ट अन्य छवियों के साथ Manticore का उपयोग करना चाहेंगे। नीचे एक docker-compose.yml फ़ाइल में Manticore सर्च के लिए न्यूनतम अनुशंसित कॉन्फ़िगरेशन है:

```yaml
version: '2.2'

services:
  manticore:
    container_name: manticore
    image: manticoresearch/manticore
    restart: always
    ports:
      - 127.0.0.1:9306:9306
      - 127.0.0.1:9308:9308
    ulimits:
      nproc: 65535
      nofile:
         soft: 65535
         hard: 65535
      memlock:
        soft: -1
        hard: -1
    volumes:
      - ./data:/var/lib/manticore
#      - ./manticore.conf:/etc/manticoresearch/manticore.conf # uncomment if you use a custom config
```

एक्सपोज़ किए गए पोर्ट 9306 और 9308 का उपयोग करने के अलावा, आप `docker-compose exec manticore mysql` चलाकर उदाहरण में लॉग इन कर सकते हैं।

### HTTP प्रोटोकॉल

HTTP प्रोटोकॉल पोर्ट 9308 पर एक्सपोज़ किया गया है। आप पोर्ट को स्थानीय रूप से मैप कर सकते हैं और curl का उपयोग करके कनेक्ट कर सकते हैं।:

```bash
docker run --name manticore -p 9308:9308 -d manticoresearch/manticore
```

<!-- example create -->
एक तालिका बनाएँ:

<!-- request JSON -->
```json
POST /cli -d 'CREATE TABLE testrt ( title text, content text, gid integer)'
```
<!-- end -->
<!-- example insert -->
एक दस्तावेज़ जोड़ें:

<!-- request JSON -->
```json
POST /insert
-d'{"table":"testrt","id":1,"doc":{"title":"Hello","content":"world","gid":1}}'
```
<!-- end -->
<!-- example search -->
एक साधारण खोज करें:

<!-- request JSON -->
```json
POST /search -d '{"table":"testrt","query":{"match":{"*":"hello world"}}}'
```
<!-- end -->

### लॉगिंग

डिफ़ॉल्ट रूप से, सर्वर अपने लॉगिंग को `/dev/stdout` पर सेट करता है, जिसे होस्ट से देखा जा सकता है:


```bash
docker logs manticore
```

क्वेरी लॉग को Docker लॉग पर भेजा जा सकता है कैशिंग के लिए `QUERY_LOG_TO_STDOUT=true` पास करके।


### मल्टी-नोड क्लस्टर रीप्लिकेशन के साथ

यहाँ दो नोड क्लस्टर को परिभाषित करने के लिए एक सरल `docker-compose.yml` है:

```yaml
version: '2.2'

services:

  manticore-1:
    image: manticoresearch/manticore
    restart: always
    ulimits:
      nproc: 65535
      nofile:
         soft: 65535
         hard: 65535
      memlock:
        soft: -1
        hard: -1
    networks:
      - manticore
  manticore-2:
    image: manticoresearch/manticore
    restart: always
    ulimits:
      nproc: 65535
      nofile:
        soft: 65535
        hard: 65535
      memlock:
        soft: -1
        hard: -1
    networks:
      - manticore
networks:
  manticore:
    driver: bridge
```
* इसे शुरू करें: `docker-compose up`
* एक क्लस्टर बनाएँ:
  ```sql
  $ docker-compose exec manticore-1 mysql

  mysql> CREATE TABLE testrt ( title text, content text, gid integer);

  mysql> CREATE CLUSTER posts;
  Query OK, 0 rows affected (0.24 sec)
  mysql> ALTER CLUSTER posts ADD testrt;
  Query OK, 0 rows affected (0.07 sec)

  MySQL [(none)]> exit
  Bye
  ```
* Join to the the cluster on the 2nd instance
  ```sql
  $ docker-compose exec manticore-2 mysql

  mysql> JOIN CLUSTER posts AT 'manticore-1:9312';
  mysql> INSERT INTO posts:testrt(title,content,gid)  VALUES('hello','world',1);
  Query OK, 1 row affected (0.00 sec)

  MySQL [(none)]> exit
  Bye
  ```
* If you now go back to the first instance you'll see the new record:
  ```sql
  $ docker-compose exec manticore-1 mysql

  MySQL [(none)]> select * from testrt;
  +---------------------+------+-------+---------+
  | id                  | gid  | title | content |
  +---------------------+------+-------+---------+
  | 3891565839006040065 |    1 | hello | world   |
  +---------------------+------+-------+---------+
  1 row in set (0.00 sec)

  MySQL [(none)]> exit
  Bye
  ```

## Memory locking and limits

It's recommended to overwrite the default ulimits of docker for the Manticore instance:

```bash
 --ulimit nofile=65536:65536
```

For best performance, table components can be "mlocked" into memory. When Manticore is run under Docker, the instance requires additional privileges to allow memory locking. The following options must be added when running the instance:

```bash
  --cap-add=IPC_LOCK --ulimit memlock=-1:-1
```

## Configuring Manticore Search with Docker

If you want to run Manticore with a custom configuration that includes table definitions, you will need to mount the configuration to the instance:

```bash
docker run --name manticore -v $(pwd)/manticore.conf:/etc/manticoresearch/manticore.conf -v $(pwd)/data/:/var/lib/manticore -p 127.0.0.1:9306:9306 -d manticoresearch/manticore
```

Take into account that Manticore search inside the container is run under user `manticore`. Performing operations with table files (like creating or rotating plain tables) should be also done under `manticore`. Otherwise the files will be created under `root` and the search daemon won't have rights to open them. For example here is how you can rotate all tables:

```bash
docker exec -it manticore gosu manticore indexer --all --rotate
```

You can also set individual `searchd` and `common` configuration settings using Docker environment variables.

The settings must be prefixed with their section name, example for in case of `mysql_version_string` the variable must be named `searchd_mysql_version_string`:


```bash
docker run --name manticore  -p 127.0.0.1:9306:9306  -e searchd_mysql_version_string='5.5.0' -d manticoresearch/manticore
```

In case of the `listen` directive, new listening interfaces using the Docker variable `searchd_listen`  in addition to the default ones. Multiple interfaces can be declared, separated by a semi-colon ("|"). To listen only on a network address, the `$ip` (retrieved internally from `hostname -i`) can be used as address alias.

For example `-e searchd_listen='9316:http|9307:mysql|$ip:5443:mysql_vip'` will add an additional SQL interface on port 9307, an SQL VIP listener on port 5443 running only on the instance's IP, and an HTTP listener on port 9316, in addition to the defaults on 9306 and 9308, respectively.

```bash
$ docker run --rm -p 1188:9307  -e searchd_mysql_version_string='5.5.0' -e searchd_listen='9316:http|9307:mysql|$ip:5443:mysql_vip'  manticore
[Mon Aug 17 07:31:58.719 2020] [1] using config file '/etc/manticoresearch/manticore.conf' (9130 chars)...
listening on all interfaces for http, port=9316
listening on all interfaces for mysql, port=9307
listening on 172.17.0.17:5443 for VIP mysql
listening on all interfaces for mysql, port=9306
listening on UNIX socket /var/run/mysqld/mysqld.sock
listening on 172.17.0.17:9312 for sphinx
listening on all interfaces for http, port=9308
prereading 0 indexes
prereaded 0 indexes in 0.000 sec
accepting connections
```

### Startup flags

To start Manticore with custom startup flags, specify them as arguments when using docker run. Ensure you do not include the `searchd` command and include the `--nodetach` flag. Here's an example:
```bash
docker run --name manticore --rm manticoresearch/manticore:latest --replay-flags=ignore-trx-errors --nodetach
```

### Running under non-root
By default, the main Manticore process `searchd` is running under user `manticore` inside the container, but the script which runs on starting the container is run under your default docker user which in most cases is `root`. If that's not what you want you can use `docker ... --user manticore` or `user: manticore` in docker compose yaml to make everything run under `manticore`. Read below about possible volume permissions issue you can get and how to solve it.

### Creating plain tables on startup
To build plain tables specified in your custom configuration file, you can use the `CREATE_PLAIN_TABLES=1` environment variable. It will execute `indexer --all` before Manticore starts. This is useful if you don't use volumes, and your tables are easy to recreate.
```bash
docker run -e CREATE_PLAIN_TABLES=1 --name manticore -v $(pwd)/manticore.conf:/etc/manticoresearch/manticore.conf -p 9306:9306 -p 9308:9308 -d manticoresearch/manticore
```

## Troubleshooting

### Permissions issue with a mounted volume

In case you are running Manticore Search docker under non-root (using `docker ... --user manticore` or `user: manticore` in docker compose yaml), you can face a permissions issue, for example:
```bash
FATAL: directory /var/lib/manticore write error: failed to open /var/lib/manticore/tmp: Permission denied
```
यह इस कारण हो सकता है क्योंकि उपयोगकर्ता जो कंटेनर के अंदर प्रक्रियाएँ चलाने के लिए उपयोग किया जाता है, आपके द्वारा कंटेनर में माउंट की गई निर्देशिका को संशोधित करने की कोई अनुमति नहीं हो सकती है। इसे ठीक करने के लिए आप माउंट की गई निर्देशिका का `chown` या `chmod` कर सकते हैं। यदि आप कंटेनर को उपयोगकर्ता `manticore` के तहत चलाते हैं, तो आपको निम्नलिखित करना होगा:
```bash
chown -R 999:999 data
```

क्योंकि उपयोगकर्ता `manticore` का ID कंटेनर के अंदर 999 है।

<!-- proofread -->


