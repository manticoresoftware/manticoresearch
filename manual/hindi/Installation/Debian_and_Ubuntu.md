# Debian या Ubuntu में Manticore स्थापित करना

### समर्थित संस्करण:

* Debian
  * 10.0 (Buster)
  * 11.0 (Bullseye)
  * 12.0 (Bookworm)

* Ubuntu
  * 18.04 (Bionic)
  * 20.04 (Focal)
  * 21.04 (Hirsute Hippo)
  * 22.04 (Jammy)
  * 24.04 (Noble)

* Mint
  * 19
  * 20
  * 21

### APT रिपोजिटोरी
Ubuntu/Debian/Mint में Manticore स्थापित करने का सबसे आसान तरीका हमारे APT रिपोजिटोरी का उपयोग करना है.

रिपोजिटोरी स्थापित करें:
```bash
wget https://repo.manticoresearch.com/manticore-repo.noarch.deb
sudo dpkg -i manticore-repo.noarch.deb
sudo apt update
```
(यदि यह स्थापित नहीं है तो `wget` स्थापित करें; यदि `apt-key` विफल हो तो `gnupg2` स्थापित करें).

फिर Manticore खोज स्थापित करें:
```
sudo apt install manticore manticore-extra
```

यदि आप पुराने संस्करण से Manticore 6 में अपग्रेड कर रहे हैं, तो पहले अपने पुराने पैकेज हटाने की सिफारिश की जाती है ताकि अपडेट की गई पैकेज संरचना के कारण संघर्ष से बचा जा सके:

```bash
sudo apt remove manticore*
```

यह आपके डेटा और कॉन्फ़िगरेशन फ़ाइल को नहीं हटाएगा.

###### विकास पैकेज
यदि आप "Nightly" (विकास) संस्करण पसंद करते हैं तो करें:
```bash
wget https://repo.manticoresearch.com/manticore-dev-repo.noarch.deb && \
sudo dpkg -i manticore-dev-repo.noarch.deb && \
sudo apt -y update && \
sudo apt -y install manticore manticore-extra manticore-common manticore-server manticore-server-core manticore-tools manticore-executor manticore-buddy manticore-backup manticore-columnar-lib manticore-server-core-dbgsym manticore-tools-dbgsym manticore-columnar-lib-dbgsym manticore-icudata-65l manticore-galera manticore-galera-dbgsym manticore-language-packs manticore-load
```

### स्टैंडअलोन DEB पैकेज
Manticore रिपोजिटोरी से स्टैंडअलोन DEB फ़ाइलें डाउनलोड करने के लिए, https://manticoresearch.com/install/ पर उपलब्ध निर्देशों का पालन करें.

### अधिक पैकेज जिन्हें आपको आवश्यकता हो सकती है
#### इंडेक्सर के लिए
Manticore पैकेज zlib और ssl पुस्तकालयों पर निर्भर करता है, कुछ और आवश्यक नहीं है। हालाँकि, यदि आप [इंडेक्सर](../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) का उपयोग करके बाहरी भंडारण से तालिकाएँ बनाना चाहते हैं, तो आपको उपयुक्त क्लाइंट पुस्तकालय स्थापित करने की आवश्यकता होगी। यह जानने के लिए कि `इंडेक्सर` को कौन सी विशिष्ट पुस्तकालयों की आवश्यकता है, इसे चलाएँ और इसके आउटपुट के शीर्ष पर देखें:

```bash
$ sudo -u manticore indexer
Manticore 3.5.4 13f8d08d@201211 रिलीज़
कॉपीराइट (c) 2001-2016, एंड्रयू एक्शनऑफ
कॉपीराइट (c) 2008-2016, Sphinx Technologies Inc (http://sphinxsearch.com)
कॉपीराइट (c) 2017-2020, Manticore Software LTD (https://manticoresearch.com)

gcc/clang v 5.4.0 द्वारा बनाया गया है,

Linux runner-0277ea0f-project-3858465-concurrent-0 4.19.78-coreos पर बनाया गया #1 SMP Mon Oct 14 22:56:39 -00 2019 x86_64 x86_64 x86_64 GNU/Linux

CMake द्वारा निम्नलिखित परिभाषाओं के साथ कॉन्फ़िगर किया गया: -DCMAKE_BUILD_TYPE=RelWithDebInfo -DDISTR_BUILD=xenial -DUSE_SSL=ON -DDL_UNIXODBC=1 -DUNIXODBC_LIB=libodbc.so.2 -DDL_EXPAT=1 -DEXPAT_LIB=libexpat.so.1 -DUSE_LIBICONV=1 -DDL_MYSQL=1 -DMYSQL_LIB=libmysqlclient.so.20 -DDL_PGSQL=1 -DPGSQL_LIB=libpq.so.5 -DLOCALDATADIR=/var/data -DFULL_SHARE_DIR=/usr/share/manticore -DUSE_ICU=1 -DUSE_BISON=ON -DUSE_FLEX=ON -DUSE_SYSLOG=1 -DWITH_EXPAT=1 -DWITH_ICONV=ON -DWITH_MYSQL=1 -DWITH_ODBC=ON -DWITH_POSTGRESQL=1 -DWITH_RE2=1 -DWITH_STEMMER=1 -DWITH_ZLIB=ON -DGALERA_SOVERSION=31 -DSYSCONFDIR=/etc/manticoresearch
```

यहाँ आप **libodbc.so.2**, **libexpat.so.1**, **libmysqlclient.so.20**, और **libpq.so.5** का उल्लेख देख सकते हैं.

विभिन्न Debian/Ubuntu संस्करणों के लिए सभी क्लाइंट पुस्तकालयों की सूची के साथ नीचे एक संदर्भ तालिका है:

| Distr | MySQL | PostgreSQL | XMLpipe | UnixODBC |
| - | - | - | - | - |
| Ubuntu Trusty | libmysqlclient.so.18 | libpq.so.5 | libexpat.so.1 | libodbc.so.1 |
| Ubuntu Bionic | libmysqlclient.so.20 | libpq.so.5 | libexpat.so.1 | libodbc.so.2 |
| Ubuntu Focal | libmysqlclient.so.21 | libpq.so.5 | libexpat.so.1 | libodbc.so.2 |
| Ubuntu Hirsute | libmysqlclient.so.21 | libpq.so.5 | libexpat.so.1 | libodbc.so.2 |
| Ubuntu Jammy | libmysqlclient.so.21 | libpq.so.5 | libexpat.so.1 | libodbc.so.2 |
| Debian Jessie | libmysqlclient.so.18 | libpq.so.5 | libexpat.so.1 | libodbc.so.2 |
| Debian Buster | libmariadb.so.3 | libpq.so.5 | libexpat.so.1 | libodbc.so.2 |
| Debian Bullseye | libmariadb.so.3 | libpq.so.5 | libexpat.so.1 | libodbc.so.2 |
| Debian Bookworm | libmariadb.so.3 | libpq.so.5 | libexpat.so.1 | libodbc.so.2 |

पुस्तकालयों को प्रदान करने वाले पैकेज खोजने के लिए, आप उदाहरण के लिए `apt-file` का उपयोग कर सकते हैं:

```bash
apt-file find libmysqlclient.so.20
libmysqlclient20: /usr/lib/x86_64-linux-gnu/libmysqlclient.so.20
libmysqlclient20: /usr/lib/x86_64-linux-gnu/libmysqlclient.so.20.2.0
libmysqlclient20: /usr/lib/x86_64-linux-gnu/libmysqlclient.so.20.3.6
```

ध्यान दें कि आपको केवल उन प्रकार के भंडारण के लिए पुस्तकालयों की आवश्यकता है जिन्हें आप उपयोग करने जा रहे हैं। इसलिए यदि आप केवल MySQL से तालिकाएँ बनाने की योजना बना रहे हैं, तो आप केवल MySQL पुस्तकालय (उपरोक्त मामले में `libmysqlclient20`) स्थापित करने की आवश्यकता हो सकती है।

अंत में, आवश्यक पैकेज स्थापित करें:

```bash
sudo apt-get install libmysqlclient20 libodbc1 libpq5 libexpat1
```

यदि आप बिल्कुल `इंडेक्सर` उपकरण का उपयोग नहीं करने जा रहे हैं, तो आपको किसी भी पुस्तकालयों की खोज और स्थापना करने की आवश्यकता नहीं है।

CJK टोकनाइजेशन समर्थन को सक्षम करने के लिए, आधिकारिक पैकेज बाइनरी संगठित ICU पुस्तकालय के साथ हैं और ICU डेटा फ़ाइल शामिल करते हैं। वे आपके सिस्टम पर उपलब्ध किसी भी ICU रनटाइम पुस्तकालय से स्वतंत्र हैं, और इन्हें अपडेट नहीं किया जा सकता है।

#### यूक्रेनी लेमेटाइज़र
लेमेटाइज़र को Python 3.9+ की आवश्यकता होती है। **सुनिश्चित करें कि आपने इसे स्थापित किया है और यह `--enable-shared` के साथ कॉन्फ़िगर किया गया है।**

यहाँ Debian और Ubuntu पर Python 3.9 और यूक्रेनी लेमेटाइज़र को स्थापित करने का तरीका है:

```bash
# APT रिपोजिटोरी से Manticore Search और UK लेमेटाइज़र स्थापित करें
cd ~
wget https://repo.manticoresearch.com/manticore-repo.noarch.deb
sudo dpkg -i manticore-repo.noarch.deb
sudo apt -y update
sudo apt -y install manticore manticore-lemmatizer-uk

# Python बनाने के लिए आवश्यक पैकेज इंस्टॉल करें
sudo apt -y update
sudo apt -y install wget build-essential libreadline-dev libncursesw5-dev libssl-dev libsqlite3-dev tk-dev libgdbm-dev libc6-dev libbz2-dev libffi-dev zlib1g-dev

# Python 3.9 डाउनलोड करें, बनाएं और इंस्टॉल करें
cd ~
wget https://www.python.org/ftp/python/3.9.4/Python-3.9.4.tgz
tar xzf Python-3.9.4.tgz
cd Python-3.9.4
./configure --enable-optimizations --enable-shared
sudo make -j8 altinstall

# लिंकर्स कैश अपडेट करें
sudo ldconfig

# pymorphy2 और यूके शब्दकोष इंस्टॉल करें
sudo LD_LIBRARY_PATH=~/Python-3.9.4 pip3.9 install pymorphy2[fast]
sudo LD_LIBRARY_PATH=~/Python-3.9.4 pip3.9 install pymorphy2-dicts-uk
```
<!-- proofread -->
