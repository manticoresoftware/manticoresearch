# Extracting Boost headers

Boost is official download from `boost.org`.

Just find the folder with following structure:

```
.
└── include
    └── boost
        ├── accumulators
        ├── algorithm
...
        ├── wave.hpp
        ├── weak_ptr.hpp
        ├── xpressive
        └── yap
```

and pack it into archive
```bash
tar -cf - include | xz -9 -f > boost_include.tar.xz
```

Archive `boost_include.tar.xz` is the target, occupy about 9M.
It stored in `/work/cross/sysroots` folder on our dev server, and also available online to build docker
from `https://repo.manticoresearch.com/repository/sysroots/boost_178/`.
