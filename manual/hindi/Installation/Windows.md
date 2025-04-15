# Windows पर Manticore इंस्टॉल करना

Windows पर Manticore विभिन्न तरीकों से इंस्टॉल किया जा सकता है। हम WSL (Windows Subsystem for Linux) का उपयोग करने की सिफारिश करते हैं क्योंकि यह पुनरुत्पादन का समर्थन करता है और इसके लिए Docker की आवश्यकता नहीं होती है। नीचे चरण-दर-चरण तरीके और वैकल्पिक विधियाँ दी गई हैं।

### WSL2 को इंस्टॉल करना या सक्षम करना

Windows पर Manticore Search इंस्टॉल करने के लिए, सबसे पहले आपको Windows Subsystem for Linux को सक्षम करना होगा। WSL2 आपको Windows पर Linux बाइनरी को स्वाभाविक रूप से चलाने देता है। इस विधि के काम करने के लिए, आपको Windows 10 संस्करण 2004 और उच्चतर या Windows 11 चलाना होगा।

WSL2 इंस्टॉल करने के लिए चरण-दर-चरण निर्देशों के लिए [आधिकारिक Microsoft गाइड](https://docs.microsoft.com/en-us/windows/wsl/install) का पालन करें।

### Manticore इंस्टॉल करना

WSL2 के माध्यम से Windows पर Manticore इंस्टॉल करने के लिए, [Debian और Ubuntu इंस्टॉलेशन](../../Installation/Debian_and_Ubuntu.md) अनुभाग देखें।

> नोट: WSL2 के माध्यम से Manticore को इंस्टॉल करना सुझाए गए तरीके के रूप में बेहतर संगतता प्रदान करता है, जो स्वदेशी Windows पैकेज का उपयोग करने की तुलना में है।

## Manticore को स्वदेशी Windows बाइनरी के रूप में इंस्टॉल करना

वैकल्पिक रूप से, आप Manticore को स्वदेशी Windows बाइनरी के रूप में इंस्टॉल कर सकते हैं, जिसे [Manticore Buddy](../../Installation/Manticore_Buddy.md#Manticore-Buddy) के लिए Docker की आवश्यकता है, इन चरणों का पालन करके:

1. Docker डेस्कटॉप इंस्टॉल करें और इसे शुरू करें।
2. Manticore Search इंस्टॉलर डाउनलोड करें (इंस्टॉलेशन पृष्ठ पर एक लिंक खोजें [installation page](http://manticoresearch.com/install/)) और इसे चलाएं। इंस्टॉलेशन निर्देशों का पालन करें।
3. इंस्टॉल करने के लिए डायरेक्टरी चुनें।
4. उन घटकों का चयन करें जिन्हें आप इंस्टॉल करना चाहते हैं। हम सभी को इंस्टॉल करने की सिफारिश करते हैं।
5. Manticore के साथ एक पूर्व-स्थापित `manticore.conf` फ़ाइल आती है [RT मोड](../Read_this_first.md#Real-time-mode-vs-plain-mode) में। कोई अतिरिक्त सेटअप आवश्यक नहीं है।

> NOTE: स्वदेशी Windows बाइनरी [पुनरुत्पादन](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Setting-up-replication) का समर्थन नहीं करते हैं।

### Windows सेवा के रूप में इंस्टॉल करना

Manticore Search सर्वर को Windows सेवा के रूप में इंस्टॉल करने के लिए, चलाएँ:

```bat
\path\to\searchd.exe --install --config \path\to\config --servicename Manticore
```

सुनिश्चित करें कि आप कॉन्फ़िगरेशन फ़ाइल का पूर्ण पथ उपयोग कर रहे हैं, अन्यथा `searchd.exe` सेवा के रूप में शुरू होने पर इसे खोज नहीं पाएगा।

इंस्टॉलेशन के बाद, सेवा को Microsoft प्रबंधन कंसोल के Services स्नैप-इन से शुरू किया जा सकता है।

एक बार शुरू होने के बाद, आप MySQL कमांड लाइन इंटरफेस का उपयोग करके Manticore तक पहुँच सकते हैं:

```bat
mysql -P9306 -h127.0.0.1
```

ध्यान दें कि इस मैनुअल के अधिकांश उदाहरणों में, हम स्थानीय होस्ट से कनेक्ट करने के लिए `-h0` का उपयोग करते हैं, लेकिन Windows में, आपको स्पष्ट रूप से `localhost` या `127.0.0.1` का उपयोग करना चाहिए।

<!-- proofread -->
