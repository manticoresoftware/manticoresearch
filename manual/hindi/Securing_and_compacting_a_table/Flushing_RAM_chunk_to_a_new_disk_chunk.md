# RAM चंक को एक नए डिस्क चंक में फ्लश करना

## FLUSH RAMCHUNK

<!-- example flush_ramchunk -->

```sql
FLUSH RAMCHUNK rt_table
```

`FLUSH RAMCHUNK` कमांड RT टेबल में एक नया डिस्क चंक बनाती है।

सामान्यतः, एक RT टेबल स्वचालित रूप से फ्लश करती है और RAM चंक की सामग्री को नए डिस्क चंक में परिवर्तित कर देती है जब RAM चंक अधिकतम [rt_mem_limit](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_mem_limit) आकार तक पहुँच जाती है। हालाँकि, डिबगिंग और परीक्षण उद्देश्यों के लिए, एक नए डिस्क चंक को मजबूर करके बनाना उपयोगी हो सकता है, और `FLUSH RAMCHUNK` स्टेटमेंट यही करता है।

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
FLUSH RAMCHUNK rt;
```
<!-- response mysql -->
```sql
Query OK, 0 rows affected (0.05 sec)
```
<!-- end -->
<!-- proofread -->
