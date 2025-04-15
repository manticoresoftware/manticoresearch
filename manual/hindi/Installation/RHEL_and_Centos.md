# RedHat और CentOS पर Manticore पैकेज स्थापित करना

### समर्थित रिलीज़:

* CentOS 7, RHEL 7, Oracle Linux 7
* CentOS 8, RHEL 8, Oracle Linux 8, CentOS Stream 8
* Amazon Linux 2
* CentOS 9, RHEL 9, AlmaLinux 9

### YUM रिपॉजिटरी

RedHat/CentOS पर Manticore स्थापित करने का सबसे सरल तरीका हमारा YUM रिपॉजिटरी है:

रिपॉजिटरी स्थापित करें:
```bash
sudo yum install https://repo.manticoresearch.com/manticore-repo.noarch.rpm
```

फिर Manticore Search स्थापित करें:
```bash
sudo yum install manticore manticore-extra
```

यदि आप एक पुराने संस्करण से Manticore 6 में अपग्रेड कर रहे हैं, तो यह अनुशंसा की जाती है कि आप पहले अपने पुराने पैकेज हटाएँ ताकि अपडेट किए गए पैकेज संरचना द्वारा उत्पन्न संघर्षों से बचा जा सके:

```bash
sudo yum remove manticore*
```

यह आपके डेटा और कॉन्फ़िगरेशन फ़ाइल को नहीं हटाएगा।

###### विकास पैकेज
यदि आप "Nightly" (विकास) संस्करण पसंद करते हैं तो करें:

```bash
sudo yum -y install https://repo.manticoresearch.com/manticore-repo.noarch.rpm && \
sudo yum -y --enablerepo manticore-dev install manticore manticore-extra manticore-common manticore-server manticore-server-core manticore-tools manticore-executor manticore-buddy manticore-backup manticore-columnar-lib manticore-server-core-debuginfo manticore-tools-debuginfo manticore-columnar-lib-debuginfo  manticore-icudata manticore-galera manticore-galera-debuginfo manticore-language-packs manticore-load
```

### स्वतंत्र RPM पैकेज
Manticore रिपॉजिटरी से स्वतंत्र RPM फ़ाइलें डाउनलोड करने के लिए, https://manticoresearch.com/install/ पर उपलब्ध निर्देशों का पालन करें।

### अधिक पैकेज जिनकी आपको आवश्यकता हो सकती है
#### इंडेक्सर के लिए
यदि आप बाहरी स्रोतों से तालिकाएँ बनाने के लिए [indexer](../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) का उपयोग करने की योजना बना रहे हैं, तो आपको सुनिश्चित करना होगा कि आपने संबंधित क्लाइंट पुस्तकालय स्थापित किए हैं ताकि आप जिन इंडेक्सिंग स्रोतों को चाहते हैं उन्हें उपलब्ध करा सकें। नीचे की पंक्ति एक साथ सभी को स्थापित करेगी; इसे वैसे ही उपयोग करने के लिए स्वतंत्र महसूस करें, या केवल उन पुस्तकालयों को स्थापित करने के लिए इसे कम करें जिनकी आपको आवश्यकता है (केवल mysql स्रोतों के लिए- `mysql-libs` ही पर्याप्त होना चाहिए, और unixODBC आवश्यक नहीं है)।

```bash
sudo yum install mysql-libs postgresql-libs expat unixODBC
```

CentOS Stream 8 में आपको चलाना पड़ सकता है:

```
dnf install mariadb-connector-c
```

यदि आप MySQL से एक साधारण तालिका बनाने की कोशिश करते समय त्रुटि `sql_connect: MySQL source wasn't initialized. Wrong name in dlopen?` प्राप्त करते हैं।

#### यूक्रेनी लेम्माटाइज़र
लेम्माटाइज़र को Python 3.9+ की आवश्यकता है। **सुनिश्चित करें कि आपने इसे स्थापित किया है और यह `--enable-shared` के साथ कॉन्फ़िगर किया गया है।**

यहाँ Centos 7/8 में Python 3.9 और यूक्रेनी लेम्माटाइज़र स्थापित करने का तरीका है:

```bash
# YUM रिपॉजिटरी से Manticore Search और UK लेम्माटाइज़र स्थापित करें
yum -y install https://repo.manticoresearch.com/manticore-repo.noarch.rpm
yum -y install manticore manticore-lemmatizer-uk

# Python बनाने के लिए आवश्यक पैकेज स्थापित करें
yum groupinstall "Development Tools" -y
yum install openssl-devel libffi-devel bzip2-devel wget -y

# Python 3.9 डाउनलोड करें, निर्माण करें और स्थापित करें
cd ~
wget https://www.python.org/ftp/python/3.9.2/Python-3.9.2.tgz
tar xvf Python-3.9.2.tgz
cd Python-3.9*/
./configure --enable-optimizations --enable-shared
make -j8 altinstall

# लिंकर्स कैश अपडेट करें
ldconfig

# pymorphy2 और UK शब्दकोष स्थापित करें
pip3.9 install pymorphy2[fast]
pip3.9 install pymorphy2-dicts-uk
```
<!-- proofread -->
