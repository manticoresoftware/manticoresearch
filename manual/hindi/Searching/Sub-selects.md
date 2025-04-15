# सब-चयन

Manticore निम्नलिखित प्रारूप में SQL के माध्यम से SELECT उप-प्रश्नों का समर्थन करता है:

```sql
SELECT * FROM (SELECT ... ORDER BY cond1 LIMIT X) ORDER BY cond2 LIMIT Y
```

बाहरी SELECT केवल `ORDER BY` और `LIMIT` क्लॉज़ की अनुमति देता है। वर्तमान में उप-SELECT प्रश्नों के दो उपयोग मामले हैं:

1. जब आपके पास दो रैंकिंग UDFs वाला प्रश्न हो, एक बहुत तेज़ और दूसरा धीमा, और आप एक बड़े मैच परिणाम सेट के साथ पूर्ण-पाठ खोज करते हैं। उप-चयन के बिना, प्रश्न इस प्रकार दिखेगा:

    ```sql
    SELECT id,slow_rank() as slow,fast_rank() as fast FROM index
        WHERE MATCH(‘some common query terms’) ORDER BY fast DESC, slow DESC LIMIT 20
        OPTION max_matches=1000;
    ```

    उप-चयन के साथ, प्रश्न को पुनःलिखित किया जा सकता है:

    ```sql
    SELECT * FROM
        (SELECT id,slow_rank() as slow,fast_rank() as fast FROM index WHERE
            MATCH(‘some common query terms’)
            ORDER BY fast DESC LIMIT 100 OPTION max_matches=1000)
    ORDER BY slow DESC LIMIT 20;
    ```

    प्रारंभिक प्रश्न में, पूरे मेल परिणाम सेट के लिए `slow_rank()` UDF की गणना की जाती है। SELECT उप-प्रश्नों के साथ, पूरे मेल परिणाम सेट के लिए केवल `fast_rank()` की गणना की जाती है, जबकि `slow_rank()` की गणना एक सीमित सेट के लिए की जाती है।

2. दूसरा मामला वितरित टेबल से आने वाले बड़े परिणाम सेट के लिए उपयोगी है।

    इस क्वेरी के लिए:

    ```sql
    SELECT * FROM my_dist_index WHERE some_conditions LIMIT 50000;
    ```

    यदि आपके पास 20 नोड्स हैं, तो प्रत्येक नोड मास्टर को अधिकतम 50K रिकॉर्ड भेज सकता है, जिससे 20 x 50K = 1M रिकॉर्ड बनते हैं। हालांकि, चूंकि मास्टर केवल 50K (1M में से) वापस भेजता है, इसलिए नोड्स के लिए केवल शीर्ष 10K रिकॉर्ड भेजना पर्याप्त हो सकता है। उप-चयन के साथ, आप प्रश्न को इस प्रकार पुनःलिखित कर सकते हैं:

    ```sql
    SELECT * FROM
         (SELECT * FROM my_dist_index WHERE some_conditions LIMIT 10000)
     ORDER by some_attr LIMIT 50000;
    ```

    इस मामले में, नोड्स केवल आंतरिक प्रश्न प्राप्त करते हैं और उसे निष्पादित करते हैं। इसका मतलब है कि मास्टर केवल 20x10K=200K रिकॉर्ड प्राप्त करेगा। मास्टर प्राप्त सभी रिकॉर्ड्स लेगा, उन्हें OUTER क्लॉज़ द्वारा पुनर्व्यवस्थित करेगा, और सर्वश्रेष्ठ 50K रिकॉर्ड्स लौटाएगा। उप-चयन मास्टर और नोड्स के बीच ट्रैफ़िक को कम करने में, साथ ही मास्टर के कंप्यूटेशन समय को कम करने में भी सहायक है (क्योंकि यह केवल 200K रिकॉर्ड्स को संसाधित करता है बजाय 1M रिकॉर्ड्स के)।
<!-- proofread -->
