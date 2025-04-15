# होस्ट नामों को फ्लश करें

```sql
FLUSH HOSTNAMES
```

FLUSH HOSTNAMES कमांड का उपयोग एजेंट होस्ट नामों के साथ जुड़े IP पत्तों को नवीनीकरण करने के लिए किया जाता है। यदि आप हमेशा होस्ट नाम के IP को प्राप्त करने के लिए DNS को क्वेरी करना चाहते हैं, तो आप [hostname_lookup](../Server_settings/Searchd.md#hostname_lookup) निर्देशिका का उपयोग कर सकते हैं।

```sql
mysql> FLUSH HOSTNAMES;
Query OK, 5 rows affected (0.01 sec)
```

<!-- proofread -->
