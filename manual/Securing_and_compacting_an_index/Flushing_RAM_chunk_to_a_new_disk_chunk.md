# Flushing RAM chunk to a new disk chunk 

## FLUSH RAMCHUNK

<!-- example flush_ramchunk -->

```sql
FLUSH RAMCHUNK rtindex
```

`FLUSH RAMCHUNK` forcibly creates a new disk chunk in an RT index.

Normally, RT index would flush and convert the contents of the RAM chunk into a new disk chunk automatically, once the RAM chunk reaches the maximum allowed [rt_mem_limit](Creating_an_index/Local_indexes/Plain_and_real-time_index_settings.md#rt_mem_limit) size. However, for debugging and testing it might be useful to forcibly create a new disk chunk, and `FLUSH RAMCHUNK` statement does exactly that.

Note that using `FLUSH RAMCHUNK` increases RT index fragmentation. Most likely, you want to use `FLUSH RTINDEX` instead. We suggest that you  abstain from using just this statement unless you're absolutely sure what you're doing. As the right way is to issue `FLUSH RAMCHUNK` with  following [OPTIMIZE](Securing_and_compacting_an_index/Compacting_an_index.md#OPTIMIZE-INDEX) command. Such combo allows to keep RT index fragmentation on minimum.


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