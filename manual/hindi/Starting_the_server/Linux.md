# लिनक्स में Manticore प्रारंभ करना

### systemd का उपयोग करके प्रारंभ और रोकना

स्थापना के बाद Manticore Search सेवा स्वचालित रूप से प्रारंभ नहीं होती है। Manticore को प्रारंभ करने के लिए, निम्नलिखित आदेश चलाएँ:

```shell
sudo systemctl start manticore
```

Manticore को रोकने के लिए, निम्नलिखित आदेश चलाएँ:

```shell
sudo systemctl stop manticore
```

Manticore सेवा बूट पर चलाने के लिए सेट की गई है। आप इसे चलाकर जाँच सकते हैं:

```shell
sudo systemctl is-enabled manticore
```

यदि आप Manticore को बूट समय पर प्रारंभ करने से अक्षम करना चाहते हैं, तो चलाएँ:

```bash
sudo systemctl disable manticore
```

Manticore को बूट पर प्रारंभ करने के लिए, चलाएँ:

```bash
sudo systemctl enable manticore
```

`searchd` प्रक्रिया प्रारंभिक जानकारी `systemd` जर्नल में लॉग करती है। यदि `systemd` लॉगिंग सक्षम है तो आप लॉग की गई जानकारी को निम्नलिखित आदेश से देख सकते हैं:

```shell
sudo journalctl -u manticore
```

### systemd का उपयोग करके कस्टम प्रारंभिक फ्लैग

`systemctl set-environment _ADDITIONAL_SEARCHD_PARAMS` आपको कस्टम प्रारंभिक फ्लैग निर्दिष्ट करने की अनुमति देता है जिनके साथ Manticore Search डेमन को प्रारंभ किया जाना चाहिए। पूर्ण सूची [यहाँ](../Starting_the_server/Manually.md#searchd-command-line-options) देखें।

उदाहरण के लिए, Manticore को डिबग लॉगिंग स्तर के साथ प्रारंभ करने के लिए, आप चला सकते हैं:
```bash
systemctl set-environment _ADDITIONAL_SEARCHD_PARAMS='--logdebug'
systemctl restart manticore
```

इसे पूर्ववत करने के लिए, चलाएँ:
```bash
systemctl set-environment _ADDITIONAL_SEARCHD_PARAMS=''
systemctl restart manticore
```

नोट, systemd पर्यावरण चर सर्वर रिबूट पर रीसेट हो जाते हैं।

### सेवा का उपयोग करके प्रारंभ और रोकना

Manticore को सेवा आदेशों का उपयोग करके प्रारंभ और रोका जा सकता है:

```shell
sudo service manticore start
sudo service manticore stop
```

रेडहैट सिस्टम पर बूट पर sysV सेवा को सक्षम करने के लिए चलाएँ:

```shell
chkconfig manticore on
```

डेबियन सिस्टम (उबונטू सहित) पर बूट पर sysV सेवा को सक्षम करने के लिए चलाएँ:

```shell
update-rc.d manticore defaults
```

कृपया ध्यान दें कि `searchd` को `manticore` उपयोगकर्ता के तहत init प्रणाली द्वारा प्रारंभ किया जाता है और सर्वर द्वारा बनाए गए सभी फ़ाइलें इस उपयोगकर्ता द्वारा स्वामित्व में होंगी। यदि `searchd` उदाहरण के लिए, रूट उपयोगकर्ता के तहत प्रारंभ किया जाता है, तो फ़ाइल अनुमतियाँ बदल जाएँगी, जिससे `searchd` को फिर से सेवा के रूप में चलाने पर समस्याएँ हो सकती हैं।

<!-- proofread -->
