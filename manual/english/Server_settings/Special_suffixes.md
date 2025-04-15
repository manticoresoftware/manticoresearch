# Special suffixes

Manticore Search supports the use of special suffixes to simplify numeric values with specific meanings. These suffixes are categorized into size suffixes and time suffixes. The common format for suffixes is an `integer` followed by a `literal`, such as `10k` or `100d`. Literals are case-insensitive, so `10W` and `10w` are considered the same.

* Size suffixes: These suffixes can be used in settings that define the size of something, such as memory buffer, disk file size, or RAM limit. If no suffix is specified, the value is considered in bytes by default. The available size suffixes are:
  - `k` for kilobytes (1k = 1024 bytes)
  - `m` for megabytes (1m = 1024k)
  - `g` for gigabytes (1g = 1024m)
  - `t` for terabytes (1t = 1024g)

* Time suffixes: These suffixes can be used in settings that define time interval values, such as delays or timeouts. Unadorned values for these parameters usually have a documented scale, but instead of guessing, you can use an explicit suffix. The available time suffixes are:
  - `us` for microseconds
  - `ms` for milliseconds
  - `s` for seconds
  - `m` for minutes
  - `h` for hours
  - `d` for days
  - `w` for weeks

<!-- proofread -->
