# RAM खंड को डिस्क पर खाली करना

## FLUSH TABLE

<!-- example flush_rtindex -->

```sql
FLUSH TABLE rt_table
```

`FLUSH TABLE` मजबूरन RT टेबल RAM खंड की सामग्री को डिस्क पर लिखता है।

वास्तविक समय तालिका [RAM खंड](../../Creating_a_table/Local_tables/Real-time_table.md#Real-time-table-files-structure) को स्वचालित रूप से एक स्वच्छ शटडाउन के दौरान, या हर [rt_flush_period](../Server_settings/Searchd.md#rt_flush_period) सेकंड में समय-समय पर डिस्क पर लिखा जाता है।

`FLUSH TABLE` आदेश जारी करने से न केवल RAM खंड की सामग्री को डिस्क पर लिखने के लिए मजबूर किया जाता है बल्कि यह बाइनरी लॉग फ़ाइलों की सफाई को भी ट्रिगर करता है।

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
FLUSH TABLE rt;
```
<!-- response mysql -->
```sql
Query OK, 0 rows affected (0.05 sec)
```
<!-- end -->

<!-- proofread -->
