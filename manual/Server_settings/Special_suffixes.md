# Special suffixes

Manticore search recognizes and parses special suffixes which makes easier to use numeric values with special meaning. Common form for them is `integer number`+`literal`, like `10k` or `100d`, but not `40.3s`(since 40.3 is not integer), or not `2d 4h` (since there are two, not one value). Literals are case-insensitive, so `10W` is the same as `10w`. There are 2 types of such suffixes currently supported:

*   Size suffixes - can be used in parameters that define size of something (memory buffer, disk file, limit of RAM, etc. ) in bytes. "Naked" numbers in that places mean literally size in bytes (octets). Size values take suffix `k` for kilobytes (1k=1024), `m` for megabytes (1m=1024k), `g` for gigabytes (1g=1024m) and `t` for terabytes (1t=1024g).
*   Time suffixes - can be used in parameters defining some time interval values like delays, timeouts, etc. "Naked" values for those parameters usually have documented scale, and you must know if their numbers, say, 100, means '100 seconds' or '100 milliseconds'. However instead of guessing you just can write suffixed value and it will be fully determined by its suffix. Time values take suffix `us` for useconds (microseconds), `ms` for milliseconds, `s` for seconds, `m` for minutes, `h` for hours, `d` for days and `w` for weeks.


## Warning

`Giga-`, and especially `tera-` size suffixes are not very usable right now, since most of the sizes inside are limited by 2Gb (or, being precise, 2Gb - 1 byte), and for the moment only [rt_mem_limit](Creating_an_index/Local_indexes/Plain_and_real-time_index_settings.md#rt_mem_limit), [attr_update_reserve](Updating_documents/UPDATE.md#attr_update_reserve) from index config, and [qcache_max_bytes](Server_settings/Searchd.md#qcache_max_bytes) from searchd config accepts 64-bit values which may exceed 2Gb.