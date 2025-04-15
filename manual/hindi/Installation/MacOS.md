# MacOS पर Manticore स्थापित करना

## Homebrew पैकेज प्रबंधक के द्वारा

```bash
brew install manticoresoftware/tap/manticoresearch manticoresoftware/tap/manticore-extra
```

Manticore को एक brew सेवा के रूप में शुरू करें:

```bash
brew services start manticoresearch
```

Manticore के लिए डिफ़ॉल्ट कॉन्फ़िगरेशन फ़ाइल `/usr/local/etc/manticoresearch/manticore.conf` या `/opt/homebrew/etc/manticoresearch/manticore.conf` में स्थित है।

यदि आप [indexer](../Creating_a_table/Local_tables/Plain_table.md) का उपयोग करके MySQL, PostgreSQL, या ODBC का उपयोग करके किसी अन्य डेटाबेस से डेटा लाने की योजना बनाते हैं, तो आपको अतिरिक्त पुस्तकालयों, जैसे कि `mysql@5.7`, `libpq`, और `unixodbc` की आवश्यकता हो सकती है।

#### विकास पैकेज
यदि आप "Nightly" (विकास) संस्करण पसंद करते हैं, तो करें:
```bash
brew tap manticoresoftware/tap-dev
brew install manticoresoftware/tap-dev/manticoresearch-dev manticoresoftware/tap-dev/manticore-extra-dev manticoresoftware/tap-dev/manticore-language-packs
brew services start manticoresearch-dev
```

<!--
## बाइनरी के साथ टैरेबॉल से

इसे [वेबसाइट से](https://manticoresearch.com/install/) डाउनलोड करें और एक फ़ोल्डर में अनपैक करें:

```bash
mkdir manticore

cd manticore

wget https://repo.manticoresearch.com/repository/manticoresearch_macos/release/manticore-5.0.2-220530-348514c86-main.tar.gz

tar -xf manticore-5.0.2-220530-348514c86-main.tar.gz

wget https://repo.manticoresearch.com/repository/manticoresearch_macos/release/manticore-columnar-lib-1.15.4-220522-2fef34e-osx10.14.4-x86_64.tar.gz

tar -xf manticore-columnar-lib-1.15.4-220522-2fef34e-osx10.14.4-x86_64.tar.gz

# Manticore शुरू करें
FULL_SHARE_DIR=./share/manticore ./bin/searchd -c ./etc/manticoresearch/manticore.conf

# इंडेक्सर चलाएँ
FULL_SHARE_DIR=./share/manticore ./bin/indexer -c ./etc/manticoresearch/manticore.conf
```

Manticore कॉन्फ़िगरेशन फ़ाइल `./etc/manticoresearch/manticore.conf` है जब आप आर्काइव को अनपैक करते हैं।

-->

<!-- प्रूफरीड करें -->
