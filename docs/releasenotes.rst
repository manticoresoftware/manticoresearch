Release notes
=============

Version 2.6.3 GA, 28 March  2018
--------------------------------

Improvements
~~~~~~~~~~~~

* jemalloc at compilation. If jemalloc is present on system, it can be enabled with cmake flag ``-DUSE_JEMALLOC=1``

Bugfixes
~~~~~~~~
* `85a6d7e <https://github.com/manticoresoftware/manticoresearch/commit/85a6d7e3db75f28c491042409bb677ed39eba07b>`__ fixed log expand_keywords option into sphinxql query log
* `caaa384 <https://github.com/manticoresoftware/manticoresearch/commit/caaa3842f4dbd8507fd9e20bf3371a0c8ec0c087>`__ fixed HTTP interface to correctly process query with large size
* `e386d84 <https://github.com/manticoresoftware/manticoresearch/commit/e386d842b36195719e459b2db1fb20e402f9a51f>`__ fixed crash of daemon on DELETE to RT index with index_field_lengths enable
* `cd538f3 <https://github.com/manticoresoftware/manticoresearch/commit/ce528f37eb5d8284661928f6f3b340a4a1985253>`__ fixed cpustats searchd cli option to work with unsupported systems
* `8740fd6 <https://github.com/manticoresoftware/manticoresearch/commit/8740fd63ae842c2ab0e51a8fcd6180d787ed57ec>`__ fixed utf8 substring matching with min lengths defined


Version 2.6.2 GA, 23 February  2018
-----------------------------------

Improvements
~~~~~~~~~~~~

* improved :ref:`Percolate Queries <percolate_query>` performance in case of using NOT operator and for batched documents. 
* :ref:`percolate_query_call` can use multiple threads depending on :ref:`dist_threads`
* new full-text matching operator NOTNEAR/N
* LIMIT for SELECT on percolate indexes
* :ref:`expand_keywords` can accept 'start','exact' (where 'star,exact' has same effect as '1')
* ranged-main-query for :ref:`joined fields <sql_joined_field>` which  uses the ranged query defined by  sql_query_range

Bugfixes
~~~~~~~~

* `72dcf66 <https://github.com/manticoresoftware/manticoresearch/commit/72dcf669744e9b7d636dfc213d24df85ab301f6b>`__ fixed crash on searching ram segments; deadlock on save disk chunk with double buffer; deadlock on save disk chunk during optimize
* `3613714 <https://github.com/manticoresoftware/manticoresearch/commit/36137149a1c3c0893bdda5a28fc7e8244bf2d4ae>`__ fixed indexer crash on xml embeded schema with empty attribute name
* `48d7e80 <https://github.com/manticoresoftware/manticoresearch/commit/48d7e8001d2a66466ca64577f27ddc5421a67251>`__ fixed erroneous unlinking of not-owned pid-file
* `a5563a4 <https://github.com/manticoresoftware/manticoresearch/commit/a5563a465ddc59ef71e65f17b68bc33f9700e838>`__ fixed orphaned fifos sometimes left in temp folder
* `2376e8f <https://github.com/manticoresoftware/manticoresearch/commit/2376e8fc4508944b96959bd10686c6d51f5145e8>`__ fixed empty FACET result set with wrong NULL row
* `4842b67 <https://github.com/manticoresoftware/manticoresearch/commit/4842b67acdd5da75672db6c08ac563b48577ddd7>`__ fixed broken index lock when running daemon as windows service
* `be35fee <https://github.com/manticoresoftware/manticoresearch/commit/be35feef54ded59125750916cd211e56108eddae>`__ fixed wrong iconv libs on mac os
* `83744a9 <https://github.com/manticoresoftware/manticoresearch/commit/83744a977d7837d1d87cd506b88b0e7eb199efe6>`__ fixed wrong count(*)

Version 2.6.1 GA, 26 January  2018
----------------------------------

Improvements
~~~~~~~~~~~~
* :ref:`agent_retry_count` in case of agents with mirrors gives the value of retries per mirror instead of per agent, the total retries per agent being agent_retry_count*mirrors.
* :ref:`agent_retry_count  <index_agent_retry_count>` can now be specified per index, overriding global value. An alias :ref:`mirror_retry_count` is added.
* a retry_count can be specified in agent definition and the value represents retries per agent
* Percolate Queries are now in HTTP JSON API at :ref:`/json/pq <http_json_pq>`.
* Added  -h and -v options (help and version) to executables
* :ref:`morphology_skip_fields`   support for  Real-Time indexes

Bugfixes
~~~~~~~~

* `a40b079 <https://github.com/manticoresoftware/manticore/commit/a40b0793feff65e40d10062568d9847c08d10f57>`__ fixed ranged-main-query to correctly work with sql_range_step when used at MVA field
* `f2f5375 <https://github.com/manticoresoftware/manticore/commit/f2f53757db45bcfb1544263ce0817e856656a621>`__ fixed issue with blackhole system loop hung and blackhole agents seems disconnected
* `84e1f54 <https://github.com/manticoresoftware/manticore/commit/84e1f54aef25e0fce98870ad2dd784db5116f1d6>`__  fixed query id to be consistent, fixed duplicated id for stored queries
* `1948423 <https://github.com/manticoresoftware/manticore/commit/19484231814fcb82b21763a3a4a9f45adc6b2d40>`__  fixed daemon crash on shutdown from various states
* `9a706b <https://github.com/manticoresoftware/manticore/commit/9a706b499a1d61a90076065a1a703029d49db958>`__ `3495fd7 <https://github.com/manticoresoftware/manticore/commit/3495fd70cba8846b1a50d55d6679b039414c3d2a>`__ timeouts on long queries
* `3359bcd8 <https://github.com/manticoresoftware/manticore/commit/3359bcd89b4f79a645fe84b8cf8616ce0addff02>`__ refactored master-agent network polling on kqueue-based systems (Mac OS X, BSD).


Version 2.6.0, 29 December 2017
-------------------------------

Features and improvements
~~~~~~~~~~~~~~~~~~~~~~~~~

* :ref:`HTTP JSON <httpapi_reference>`: JSON queries can now do equality on attributes, MVA and JSON attributes can be used in inserts and updates, updates and deletes via JSON API can be performed on distributed indexes
* :ref:`Percolate Queries <percolate_query>`
* Removed support for 32-bit docids from the code. Also removed all the code that converts/loads legacy indexes with 32-bit docids.
* `Morphology only for certain fields <https://github.com/manticoresoftware/manticore/issues/7>`__ . A new index directive morphology_skip_fields allows defining a list of fields for which morphology does not apply.
* `expand_keywords can now be a query runtime directive set using the OPTION statement <https://github.com/manticoresoftware/manticore/issues/8>`__

Bugfixes
~~~~~~~~

* `0cfae4c <https://github.com/manticoresoftware/manticore/commit/0cfae4c>`__ fixed crash on debug build of daemon (and m.b. UB on release) when built with rlp
* `324291e <https://github.com/manticoresoftware/manticore/commit/324291e>`__ fixed RT index optimize with progressive option enabled that merges kill-lists with wrong order
* `ac0efee <https://github.com/manticoresoftware/manticore/commit/ac0efee>`__ minor crash on mac
* lots of minor fixes after thorough static code analysis
* other minor bugfixes

Upgrade
~~~~~~~~

In this release we've changed internal protocol used by masters and agents to speak with each other. In case you run Manticoresearch in a distributed environment with multiple instances make sure your first upgrade agents, then the masters.

Version 2.5.1, 23 November 2017
-------------------------------

Features and improvements
~~~~~~~~~~~~~~~~~~~~~~~~~

* JSON queries on :ref:`HTTP API protocol <httpapi_reference>`. Supported search, insert, update, delete, replace operations. Data manipulation commands can be also bulked, also there are some limitations currently as MVA and JSON attributes can't be used for inserts, replaces or updates.
* :ref:`RELOAD INDEXES <reload_indexes_syntax>` command
* :ref:`FLUSH LOGS <flush_logs_syntax>` command
* :ref:`SHOW THREADS <show_threads_syntax>`  can show progress of optimize, rotation or flushes.
* GROUP N BY  work correctly with MVA attributes
* blackhole agents are run on separate thread to not affect master query anymore
* implemented reference count on indexes, to avoid stalls caused by rotations and high load
* SHA1 hashing implemented, not exposed yet externally
* fixes for compiling on FreeBSD, macOS and Alpine

Bugfixes
~~~~~~~~

* `989752b <https://github.com/manticoresoftware/manticore/commit/989752b>`__ filter regression with block index
* `b1c3864 <https://github.com/manticoresoftware/manticore/commit/b1c3864>`__ rename PAGE_SIZE -> ARENA_PAGE_SIZE for compatibility with musl
* `f2133cc <https://github.com/manticoresoftware/manticore/commit/f2133cc>`__ disable googletests for cmake < 3.1.0
* `f30ec53 <https://github.com/manticoresoftware/manticore/commit/0839de7>`__ failed to bind socket on daemon restart
* `0807240 <https://github.com/manticoresoftware/manticore/commit/0807240>`__ fixed  crash of daemon on shutdown
* `3e3acc3 <https://github.com/manticoresoftware/manticore/commit/3e3acc3>`__ fixed show threads for system blackhole thread
* `262c3fe <https://github.com/manticoresoftware/manticore/commit/262c3fe>`__ Refactored config check of iconv, fixes building on FreeBSD and Darwin

Version 2.4.1 GA, 16 October 2017
---------------------------------

Features and improvements
~~~~~~~~~~~~~~~~~~~~~~~~~

* OR operator in WHERE clause between attribute filters

* Maintenance mode ( SET MAINTENANCE=1)

* :ref:`CALL KEYWORDS <call_keywords_syntax>` available on distributed indexes

* :ref:`Grouping in UTC <grouping_in_utc>`

* :ref:`query_log_mode` for custom log files permissions

* Field weights can be zero or negative

* :ref:`max_query_time <select_option>` can now affect full-scans

* added :ref:`net_wait_tm`, :ref:`net_throttle_accept` and :ref:`net_throttle_action` for network thread fine tuning (in case of workers=thread_pool)

* COUNT DISTINCT works with facet searches

* IN can be used with JSON float arrays

* multi-query optimization is not broken anymore by integer/float expressions

* :ref:`SHOW META <show_meta_syntax>` shows a ``multiplier`` row when multi-query optimization is used

Compiling
~~~~~~~~~

Manticore Search is built using cmake and the minimum gcc version required for compiling is 4.7.2.

Folders and service
~~~~~~~~~~~~~~~~~~~

Manticore Search runs under ``manticore`` user.

Default data folder is now ``/var/lib/manticore/``.

Default log folder is now ``/var/log/manticore/``.

Default pid folder is now ``/var/run/manticore/``.



Bugfixes
~~~~~~~~

* `a58c619 <https://github.com/manticoresoftware/manticore/commit/a58c619>`__ fixed  SHOW COLLATION statement that breaks java connector
* `631cf4e <https://github.com/manticoresoftware/manticore/commit/631cf4e>`__ fixed crashes on processing distributed indexes; added locks to distributed index hash; removed move and copy operators from agent
* `942bec0 <https://github.com/manticoresoftware/manticore/commit/942bec0>`__  fixed crashes on processing distributed indexes due to parallel reconnects
* `e5c1ed2 <https://github.com/manticoresoftware/manticore/commit/e5c1ed2>`__  fixed crash at crash handler on store query to daemon log 
* `4a4bda5 <https://github.com/manticoresoftware/manticore/commit/4a4bda5>`__  fixed a crash with pooled attributes in multiqueries
* `3873bfb <https://github.com/manticoresoftware/manticore/commit/3873bfb>`__  fixed reduced core size by prevent index pages got included into core file
* `11e6254 <https://github.com/manticoresoftware/manticore/commit/11e6254>`__  fixed searchd crashes on startup when invalid agents are specified
* `4ca6350 <https://github.com/manticoresoftware/manticore/commit/4ca6350>`__  fixed indexer reports error in sql_query_killlist query
* `123a9f0 <https://github.com/manticoresoftware/manticore/commit/123a9f0>`__  fixed fold_lemmas=1 vs hit count
* `cb99164 <https://github.com/manticoresoftware/manticore/commit/cb99164>`__  fixed inconsistent behavior of html_strip
* `e406761 <https://github.com/manticoresoftware/manticore/commit/e406761>`__  fixed optimize rt index loose new settings; fixed optimize with sync option lock leaks; 
* `86aeb82 <https://github.com/manticoresoftware/manticore/commit/86aeb82>`__  Fixed processing erroneous multiqueries
* `2645230 <https://github.com/manticoresoftware/manticore/commit/2645230>`__  fixed result set depends on multi-query order
* `72395d9 <https://github.com/manticoresoftware/manticore/commit/72395d9>`__  fixed daemon crash on multi-query with bad query
* `f353326 <https://github.com/manticoresoftware/manticore/commit/f353326>`__  fixed shared to exclusive lock
* `3754785 <https://github.com/manticoresoftware/manticore/commit/3754785>`__  fixed daemon crash for query without indexes
* `29f360e <https://github.com/manticoresoftware/manticore/commit/29f360e>`__  fixed dead lock of daemon



.. _release233:

Version 2.3.3, 06 July 2017
---------------------------

- Manticore branding
