# Special suffixes

Manticore Search supports special suffixes which makes it easier to use numeric values with a special meaning. The common form for them is `integer number`+`literal`, like `10k` or `100d`, but not `40.3s`(since 40.3 is not integer), and not `2d 4h` (since there are two, not one value). Literals are case-insensitive, so `10W` is the same as `10w`. There are 2 types of such suffixes currently supported:

* Size suffixes: can be used in settings that define size of something: memory buffer, disk file size, limit of RAM etc. If you don't specify any suffix the value is considered in bytes by default. The suffixes can be:
  - `k` for kilobytes (1k=1024)
  - `m` for megabytes (1m=1024k)
  - `g` for gigabytes (1g=1024m)
  - `t` for terabytes (1t=1024g)
* Time suffixes: can be used in settings defining some time interval values: delays, timeouts etc. "Naked" values for those parameters usually have documented scale, e.g. for some settings `100` means 100 seconds, for others - 100 milliseconds. However, instead of guessing you can just use an explicit suffix. Those can be:
  - `us` for useconds (microseconds)
  - `ms` for milliseconds
  - `s` for seconds
  - `m` for minutes
  - `h` for hours
  - `d` for days
  - `w` for weeks.
