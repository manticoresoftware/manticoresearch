# Windows में Manticore शुरू करना

Windows पर, यदि आप चाहते हैं कि Manticore बूट पर शुरू हो, तो आप इसे Windows सेवा के रूप में स्थापित कर सकते हैं। आप Manticore को सेवा के रूप में स्थापित करने के लिए [Manticore as Windows Service](../Installation/Windows.md) गाइड में दिए गए निर्देशों का पालन कर सकते हैं।

एक बार जब Manticore को सेवा के रूप में स्थापित कर दिया जाता है, तो आप इसे नियंत्रण कक्ष से या sc.exe कमांड का उपयोग करके कमांड लाइन से शुरू और रोक सकते हैं।

```shell
sc.exe start Manticore
```

```shell
sc.exe stop Manticore
```
वैकल्पिक रूप से, यदि आप Manticore को Windows सेवा के रूप में स्थापित नहीं करते हैं, तो आप इसे कमांड लाइन से निम्नलिखित कमांड चलाकर शुरू कर सकते हैं:

```shell
.\bin\searchd -c manticore.conf
```
यह कमांड मानती है कि आपके पास Manticore का बाइनरी और कॉन्फ़िगरेशन फ़ाइल वर्तमान निर्देशिका में है।
<!-- proofread -->
