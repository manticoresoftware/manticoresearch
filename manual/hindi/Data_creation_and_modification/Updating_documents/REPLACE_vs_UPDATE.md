# REPLACE बनाम UPDATE

आप RT या PQ तालिका में मौजूदा डेटा को अपडेट या रिप्लेस करके संशोधित कर सकते हैं।

[UPDATE](../../Data_creation_and_modification/Updating_documents/UPDATE.md) मौजूदा दस्तावेजों के [पंक्ति-वार](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) विशेषताओं के मानों को नए मानों के साथ बदलता है। पूर्ण-ข้อความ फ़ील्ड और कॉलम गुणों को अपडेट नहीं किया जा सकता है। यदि आपको पूर्ण-क्रम फ़ील्ड या कॉलम गुणों की सामग्री बदलने की आवश्यकता है, तो [REPLACE](../../Data_creation_and_modification/Updating_documents/REPLACE.md) का उपयोग करें।

[REPLACE](../../Data_creation_and_modification/Updating_documents/REPLACE.md) [INSERT](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md) के समान काम करता है सिवाय इसके कि यदि एक पुराना दस्तावेज़ नए दस्तावेज़ के समान ID रखता है, तो पुराने दस्तावेज़ को नए दस्तावेज़ के सम्मिलित होने से पहले हटाए जाने के रूप में चिह्नित किया जाता है। ध्यान दें कि पुराना दस्तावेज़ तालिका से भौतिक रूप से हटा नहीं जाता है। हटाना केवल तब हो सकता है जब तालिका में चंक्स को मर्ज किया जाता है, जैसे कि [OPTIMIZE](../../Securing_and_compacting_a_table/Compacting_a_table.md) के परिणामस्वरूप।

### UPDATE बनाम आंशिक REPLACE

`UPDATE` और [आंशिक REPLACE](../../Data_creation_and_modification/Updating_documents/REPLACE.md?client=REPLACE+SET) दोनों एक फ़ील्ड का मान बदल सकते हैं, लेकिन वे अलग तरीके से काम करते हैं:
- `UPDATE` केवल उन फ़ील्ड को बदल सकता है जो न तो कॉलमार हैं और न ही पूर्ण-शब्द। यह प्रक्रिया इन-स्थान की जाती है, जो आमतौर पर `REPLACE` की तुलना में तेज है।
- एक आंशिक `REPLACE` दस्तावेज़ में किसी भी फ़ील्ड को बदल सकता है, लेकिन इसके लिए तालिका में सभी फ़ील्ड को "संग्रहीत" पर सेट करना आवश्यक है (हालांकि यह डिफ़ॉल्ट सेटिंग है)। `UPDATE` के साथ यह आवश्यक नहीं है।

<!-- proofread -->
