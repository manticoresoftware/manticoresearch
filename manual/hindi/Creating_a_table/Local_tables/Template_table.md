# टेम्पलेट टेबल

<!-- example template -->
 एक टेम्पलेट टेबल Manticore में एक विशेष प्रकार की टेबल होती है जो कोई डेटा संग्रहीत नहीं करती है और आपके डिस्क पर कोई फ़ाइलें नहीं बनाती है। इसके बावजूद, इसमें समान [NLP सेटिंग्स](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Natural-language-processing-specific-settings) हो सकती हैं जैसे कि एक साधारण या वास्तविक-समय की टेबल। टेम्पलेट टेबल निम्नलिखित उद्देश्यों के लिए उपयोग की जा सकती हैं:

* [साधारण मोड](../../Creating_a_table/Local_tables.md#Defining-table-schema-in-config-%28Plain mode%29) में सेटिंग्स विरासत में लेने के लिए एक टेम्पलेट के रूप में, आपकी Manticore कॉन्फ़िगरेशन फ़ाइल को सरल बनाना।
* [CALL KEYWORDS](../../Searching/Autocomplete.md#CALL-KEYWORDS) कमांड की मदद से कीवर्ड उत्पादन।
* [CALL SNIPPETS](../../Searching/Highlighting.md#CALL-SNIPPETS) कमांड का उपयोग करके एक मनचाहा स्ट्रिंग को उजागर करना।


<!-- intro -->
##### एक कॉन्फ़िगरेशन फ़ाइल के माध्यम से टेम्पलेट टेबल बनाना:

<!-- request CONFIG -->

```ini
table template {
  type = template
  morphology = stem_en
  wordforms = wordforms.txt
  exceptions = exceptions.txt
  stopwords = stopwords.txt
}
```
<!-- end -->
<!-- proofread -->
