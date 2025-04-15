# Filebeat के साथ एकीकरण

> नोट: Filebeat के साथ एकीकरण के लिए [Manticore Buddy](../Installation/Manticore_Buddy.md) की आवश्यकता है। यदि यह काम नहीं करता है, सुनिश्चित करें कि Buddy स्थापित है।

[Filebeat](https://www.elastic.co/beats/filebeat) लॉग डेटा को अग्रेषित और केंद्रीकृत करने के लिए एक हल्का शिपर है। एक एजेंट के रूप में स्थापित होने पर, यह उन लॉग फ़ाइलों या स्थानों की निगरानी करता है जो आप निर्दिष्ट करते हैं, लॉग घटनाओं को एकत्र करता है, और सामान्यतः Elasticsearch या Logstash के लिए अनुक्रमण के लिए अग्रेषित करता है।

अब, Manticore भी Filebeat के उपयोग का समर्थन करता है जैसा कि प्रसंस्करण पाइपलाइनों में। यह एकत्रित और परिवर्तित डेटा को Manticore में भेजने की अनुमति देता है जैसे कि Elasticsearch में। वर्तमान में, सभी संस्करण >= 7.10 समर्थित हैं।

## Filebeat कॉन्फ़िगरेशन

नीचे हमारे उदाहरण dpkg लॉग के साथ काम करने के लिए एक Filebeat कॉन्फ़िगरेशन है:

```
filebeat.inputs:
- type: filestream
  id: example
  paths:
	- /var/log/dpkg.log

output.elasticsearch:
  hosts: ["http://localhost:9308"]
  index:  "dpkg_log"
  allow_older_versions: true

setup.ilm:
  enabled: false

setup.template:
  name: "dpkg_log"
  pattern: "dpkg_log"
```

### Filebeat संस्करणों के लिए कॉन्फ़िगरेशन >= 8.11

ध्यान दें कि Filebeat के संस्करण 8.10 से अधिक में आउटपुट संकुचन सुविधा डिफ़ॉल्ट रूप से सक्षम है। यही कारण है कि Manticore के साथ संगतता प्रदान करने के लिए कॉन्फ़िगरेशन फ़ाइल में `compression_level: 0` विकल्प जोड़ा जाना चाहिए:

```
filebeat.inputs:
- type: filestream
  id: example
  paths:
	- /var/log/dpkg.log

output.elasticsearch:
  hosts: ["http://localhost:9308"]
  index:  "dpkg_log"
  allow_older_versions: true
  compression_level: 0

setup.ilm:
  enabled: false

setup.template:
  name: "dpkg_log"
  pattern: "dpkg_log"
```

## Filebeat परिणाम

जब आप इस कॉन्फ़िगरेशन के साथ Filebeat चलाते हैं, तो लॉग डेटा Manticore में भेजा जाएगा और सही तरीके से अनुक्रमित किया जाएगा। यहाँ Manticore द्वारा बनाए गए तालिका का परिणामी स्कीमा और सम्मिलित दस्तावेज़ का एक उदाहरण है:

```
mysql> DESCRIBE dpkg_log;
+------------------+--------+--------------------+
| Field            | Type   | Properties         |
+------------------+--------+--------------------+
| id               | bigint |                    |
| @timestamp       | text   | indexed stored     |
| message          | text   | indexed stored     |
| log              | json   |                    |
| input            | json   |                    |
| ecs              | json   |                    |
| host             | json   |                    |
| agent            | json   |                    |
+------------------+--------+--------------------+
```

```
mysql> SELECT * FROM dpkg_log LIMIT 1\G
*************************** 1. row ***************************
id: 7280000849080753116
@timestamp: 2023-06-16T09:27:38.792Z
message: 2023-04-12 02:06:08 status half-installed libhogweed5:amd64 3.5.1+really3.5.1-2
input: {"type":"filestream"}
ecs: {"version":"1.6.0"}
host: {"name":"logstash-db848f65f-lnlf9"}
agent: {"ephemeral_id":"587c2ebc-e7e2-4e27-b772-19c611115996","id":"2e3d985b-3610-4b8b-aa3b-2e45804edd2c","name":"logstash-db848f65f-lnlf9","type":"filebeat","version":"7.10.0","hostname":"logstash-db848f65f-lnlf9"}
log: {"offset":80,"file":{"path":"/var/log/dpkg.log"}}
```
