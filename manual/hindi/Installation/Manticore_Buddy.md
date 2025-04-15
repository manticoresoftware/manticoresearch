# Manticore Buddy

Manticore Buddy एक साइडकार है Manticore Search के लिए, जो PHP में लिखा गया है, जो विभिन्न कार्यों में मदद करता है। सामान्य कार्यप्रवाह यह है कि उपयोगकर्ता को त्रुटि लौटाने से पहले, Manticore Search Buddy से पूछता है कि क्या यह डेमोन के लिए समस्या को संभाल सकता है। Buddy का PHP कोड उच्च-स्तरीय सुविधाओं को लागू करना आसान बनाता है जिन्हें अत्यधिक उच्च प्रदर्शन की आवश्यकता नहीं होती।

Buddy की गहरी समझ के लिए, इन लेखों को देखें:
- [Introducing Buddy: the PHP sidecar for Manticore Search](https://manticoresearch.com/blog/manticoresearch-buddy-intro/)
- [Manticore Buddy: challenges and solutions](https://manticoresearch.com/blog/manticoresearch-buddy-challenges-and-solutions/)
- [Manticore Buddy: pluggable design](https://manticoresearch.com/blog/manticoresearch-buddy-pluggable-design/)
- [Manticore Buddy GitHub repository](https://github.com/manticoresoftware/manticoresearch-buddy)

## Manticore Buddy Installation

अगर आप ऊपर दिए गए स्थापना निर्देशों या [वेबसाइट पर](https://manticoresearch.com/install) का पालन करते हैं, तो आपको Manticore Buddy को स्थापित करने या शुरू करने के बारे में चिंता करने की आवश्यकता नहीं है: यह स्वचालित रूप से तब स्थापित होता है जब आप पैकेज `manticore-extra` को स्थापित करते हैं, और Manticore Search इसे स्वचालित रूप से लॉन्च पर शुरू करता है।

## Disabling Manticore Buddy

Manticore Buddy को अक्षम करने के लिए, कृपया [buddy_path](../Server_settings/Searchd.md#buddy_path) सेटिंग का उपयोग करें।
