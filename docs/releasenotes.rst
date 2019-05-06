Release notes
=============


Version 3.0.0, 6 May 2019
-------------------------------

Features and improvements
~~~~~~~~~~~~~~~~~~~~~~~~~

* New index storage. Non-scalar attributes are not limited anymore to 4GB size per index
* attr_update_reserve directive
* String,JSON and MVAs can be updated using UPDATE
* killlists are applied at index load time
* killlist_target directive
* multi AND searches speedup
* better average performance and RAM usage
* convert tool for upgrading indexes made with 2.x
* CONCAT() function
* JOIN CLUSTER cluster AT 'nodeaddress:port'
* ALTER CLUSTER posts UPDATE nodes
* node_address directive
* list of nodes printed in SHOW STATUS

Behaviour changes
~~~~~~~~~~~~~~~~~

* in case of indexes with killists, daemon doesn't rotate indexes in order defined in conf, but follows the chain of killlist targets
* order of indexes in a search no longer defines the order in which killlists are applied
* Document IDs are now signed big integers

Removed directives
~~~~~~~~~~~~~~~~~~

* docinfo (always extern now), inplace_docinfo_gap, mva_updates_pool 


Version 2.8.2 GA, 2 April 2019
------------------------------

Features and improvements
~~~~~~~~~~~~~~~~~~~~~~~~~

* Galera replication for percolate indexes
* OPTION morphology

Compiling notes
~~~~~~~~~~~~~~~
Cmake minimum version is now 3.13. Compiling requires boost and libssl development libraries.

Bugfixes
~~~~~~~~

* `6967fedb <https://github.com/manticoresoftware/manticoresearch/commit/6967fedb2ef818ec1c825d482563edd05e1c9245>`__ fixed crash on many stars at select list for query into many distributed indexes
* `36df1a40 <https://github.com/manticoresoftware/manticoresearch/commit/36df1a407dc08263690e3492518613ace82d69ca>`__ fixed `#177 <https://github.com/manticoresoftware/manticoresearch/issues/177>`__ large packet via SphinxQL interface
* `57932aec <https://github.com/manticoresoftware/manticoresearch/commit/57932aec734583fa93359faaf5034b2e2c9d352b>`__ fixed `#170 <https://github.com/manticoresoftware/manticoresearch/issues/170>`__ crash of daemon on RT optimize with MVA updated
* `edb24b87 <https://github.com/manticoresoftware/manticoresearch/commit/edb24b870423add86eba471d361e0e5aff098b18>`__ fixed daemon crash on binlog removed due to RT index remove after config reload on SIGHUP
* `bd3e66e0 <https://github.com/manticoresoftware/manticoresearch/commit/bd3e66e0085bc7e2e351b817dfc972fd8158fbce>`__ fixed mysql handshake auth plugin payloads
* `6a217f6e <https://github.com/manticoresoftware/manticoresearch/commit/6a217f6ed82fb10f752213b15617b9cedf1e8533>`__ fixed `#172 <https://github.com/manticoresoftware/manticoresearch/issues/172>`__ phrase_boundary settings at RT index
* `3562f652 <https://github.com/manticoresoftware/manticoresearch/commit/3562f652753e4091fd7b5b0f65b58341c8cbcb31>`__ fixed `#168 <https://github.com/manticoresoftware/manticoresearch/issues/168>`__ deadlock at ATTACH index to itself
* `250b3f0e <https://github.com/manticoresoftware/manticoresearch/commit/250b3f0e74ad18dda34ba080aa13ca87798a9ac9>`__ fixed binlog saves empty meta after daemon crash
* `4aa6c69a <https://github.com/manticoresoftware/manticoresearch/commit/4aa6c69ad3a181cd7c5dafb990fb528121f68e59>`__ fixed crash of daemon due to string at sorter from RT index with disk chunks


Version 2.8.1 GA, 6 March 2019
------------------------------

Features and improvements
~~~~~~~~~~~~~~~~~~~~~~~~~

* SUBSTRING_INDEX()
* SENTENCE and PARAGRAPH support for percolate queries
* systemd generator for Debian/Ubuntu; also added LimitCORE to allow core dumping

Bugfixes
~~~~~~~~

* `84fe7405 <https://github.com/manticoresoftware/manticoresearch/commit/84fe7405d17b59f4dcde15a6c8e8503923503def>`__ fixed crash of daemon on match mode all and empty full text query
* `daa88b57 <https://github.com/manticoresoftware/manticoresearch/commit/daa88b579fa373d5d2e869e43a2e178363b0fef1>`__ fixed crash on deleting of static string
* `22078537 <https://github.com/manticoresoftware/manticoresearch/commit/22078537dddbd9ce81a182dcc879f235b6e87004>`__ fixed exit code when indextool failed with FATAL
* `0721696d <https://github.com/manticoresoftware/manticoresearch/commit/0721696d6780c200c65d596624a8187983fb7fcb>`__ fixed `#109 <https://github.com/manticoresoftware/manticoresearch/issues/109>`__ no matches for prefixes due to wrong exact form check
* `8af81011 <https://github.com/manticoresoftware/manticoresearch/commit/8af810111b8e2f87bc9f378172eff1ab9725c7e7>`__ fixed `#161 <https://github.com/manticoresoftware/manticoresearch/issues/161>`__ reload of config settings for RT indexes
* `e2d59277 <https://github.com/manticoresoftware/manticoresearch/commit/e2d592773f4fd702ace90070a793a842a0fec6ab>`__ fixed crash of daemon on access of large JSON string
* `75cd1342 <https://github.com/manticoresoftware/manticoresearch/commit/75cd1342d05cdc352e3dd145cdbc6f79394a165b>`__ fixed PQ field at JSON document altered by index stripper causes wrong match from sibling field
* `e2f77543 <https://github.com/manticoresoftware/manticoresearch/commit/e2f775437df922674d772ed4417780492502e65a>`__ fixed crash of daemon at parse JSON on RHEL7 builds
* `3a25a580 <https://github.com/manticoresoftware/manticoresearch/commit/3a25a5808feb3f8b80866991b436d6c1241618c2>`__ fixed crash of json unescaping when slash is on the edge
* `be9f4978 <https://github.com/manticoresoftware/manticoresearch/commit/be9f497872bea62dbdccd64fc28294fd7776c289>`__ fixed option 'skip_empty' to skip empty docs and not warn they're not valid json
* `266e0e7b <https://github.com/manticoresoftware/manticoresearch/commit/266e0e7b088549722a805fc837bf101ff681a5e8>`__ fixed `#140 <https://github.com/manticoresoftware/manticoresearch/issues/161>`__ output 8 digits on floats when 6 is not enough to be precise
* `3f6d2389 <https://github.com/manticoresoftware/manticoresearch/commit/3f6d23891064ee8e94030c4231497cdd7da33a6a>`__ fixed empty jsonobj creation
* `f3c7848a <https://github.com/manticoresoftware/manticoresearch/commit/f3c7848a59b7eedc67c3403d6d4b37ce6fa94dc8>`__ fixed `#160 <https://github.com/manticoresoftware/manticoresearch/issues/161>`__ empty mva outputs NULL instead of an empty string
* `0afa2ed0 <https://github.com/manticoresoftware/manticoresearch/commit/0afa2ed058e5759470b1d7354c722faab34f98bb>`__ fixed fail to build without pthread_getname_np
* `9405fccd <https://github.com/manticoresoftware/manticoresearch/commit/9405fccdeb27f0302a8d9a848981d30080216777>`__ fixed crash on daemon shutdown with thread_pool workers

Version 2.8.0 GA, 28 January 2019
---------------------------------

Improvements
~~~~~~~~~~~~
* Distributed indexes for percolate indexes
* CALL PQ new options and changes:

  - skip_bad_json
  - mode (sparsed/sharded)
  - json documents can be passed as a json array
  - shift
  - Column names ‘UID’, ‘Documents’, ‘Query’, ‘Tags’, ‘Filters’ were renamed to ‘id’, ‘documents’, ‘query’, ‘tags’, ‘filters’

* DESCRIBE pq TABLE
* SELECT FROM pq WHERE UID is not possible any more, use ‘id’ instead
* SELECT over pq indexes is on par with regular indexes (e.g. you can filter rules via REGEX())
* ANY/ALL can be used on PQ tags
* expressions have auto-conversion for JSON fields, not requiring explicit casting
* built-in 'non_cjk' charset_table and 'cjk' ngram_chars
* built-in stopwords collections for 50 languages
* multiple files in a stopwords declaration can also be separated by comma
* CALL PQ can accept JSON array of documents


Bugfixes
~~~~~~~~

* `a4e19af <https://github.com/manticoresoftware/manticoresearch/commit/a4e19afee54dafdc04b0da53372dafd8c0d332d6>`__ fixed csjon-related leak
* `28d8627 <https://github.com/manticoresoftware/manticoresearch/commit/28d862774874891a03e361da1b0347ebe8066ce0>`__ fixed crash because of missed value in json
* `bf4e9ea <https://github.com/manticoresoftware/manticoresearch/commit/bf4e9ea27c349cdc9ae4e54e960a4a17b90e38fa>`__ fixed save of empty meta for RT index
* `33b4573 <https://github.com/manticoresoftware/manticoresearch/commit/33b4573529e7c7c7bce19bf9d54054866f30d99a>`__ fixed lost form flag (exact) for sequence of lemmatizer
* `6b95d48 <https://github.com/manticoresoftware/manticoresearch/commit/6b95d48240d3b5520afade19c249d79e020a5e88>`__ fixed string attrs >4M use saturate instead of overflow
* `621418b <https://github.com/manticoresoftware/manticoresearch/commit/621418b829e70af36aaa322a2f51ece3f86bc499>`__ fixed crash of daemon on SIGHUP with disabled index
* `3f7e35d <https://github.com/manticoresoftware/manticoresearch/commit/3f7e35d1482966cc45d5abbcb2de5de3508d66b0>`__ fixed daemon crash on simultaneous API session status commands
* `cd9e4f1 <https://github.com/manticoresoftware/manticoresearch/commit/cd9e4f1709a48ddafc128c450c2d882bc11ba50e>`__ fixed crash of daemon at delete query to RT index with field filters
* `9376470 <https://github.com/manticoresoftware/manticoresearch/commit/9376470d455fcd256c6abff79c431a6919dfa7ac>`__ fixed crash of daemon at CALL PQ to distributed index with empty document
* `8868b20 <https://github.com/manticoresoftware/manticoresearch/commit/8868b207644f4f8cc1ab0c270adb35493fed22e1>`__ fixed cut SphinxQL error message larger 512 chars
* `de9deda <https://github.com/manticoresoftware/manticoresearch/commit/de9deda9c142823d6dbf529423417b5c670fae94>`__ fixed crash on save percolate index without binlog
* `2b219e1 <https://github.com/manticoresoftware/manticoresearch/commit/2b219e1a32791e7740bb210b7d408f96abc6374f>`__ fixed http interface is not working in OSX
* `e92c602 <https://github.com/manticoresoftware/manticoresearch/commit/e92c60240f8f9e1756bfa14fba0bbb402586a389>`__ fixed indextool false error message on check of MVA
* `238bdea <https://github.com/manticoresoftware/manticoresearch/commit/238bdea59bad89f097403f1c978658ce45f16c70>`__ fixed write lock at FLUSH RTINDEX to not write lock whole index during save and on regular flush from rt_flush_period
* `c26a236 <https://github.com/manticoresoftware/manticoresearch/commit/c26a236bd42b7082db079a51a84172066d337d64>`__ fixed ALTER percolate index stuck waiting search load
* `9ee5703 <https://github.com/manticoresoftware/manticoresearch/commit/9ee5703f6c28f57cd5bff5e705966d93fe30d267>`__ fixed max_children to use default amount of thread_pool workers for value of 0
* `5138fc0 <https://github.com/manticoresoftware/manticoresearch/commit/5138fc048a1bd146b271ce6a72fe954344281f69>`__ fixed error on indexing of data into index with index_token_filter plugin along with stopwords and stopword_step=0
* `2add3d3 <https://github.com/manticoresoftware/manticoresearch/commit/2add3d319ac62fe450bf60e89033100853dc2ecf>`__ fixed crash with absent lemmatizer_base when still using aot lemmatizers in index definitions



Version 2.7.5 GA, 4 December 2018
---------------------------------

Improvements
~~~~~~~~~~~~

* REGEX function 
* limit/offset for json API search
* profiler points for qcache

Bugfixes
~~~~~~~~

* `eb3c768 <https://github.com/manticoresoftware/manticoresearch/commit/eb3c7683b9740cff4cc4cadd2ab9f990b9f7d173>`__ fixed crash of daemon on FACET with multiple attribute wide types
* `d915cf6 <https://github.com/manticoresoftware/manticoresearch/commit/d915cf6696514f4be365ffb5981bccaa15983e65>`__ fixed implicit group by at main select list of FACET query
* `5c25dc2 <https://github.com/manticoresoftware/manticoresearch/commit/5c25dc271a8c11a1c82ca3c88e518acbc6a8c164>`__ fixed crash on query with GROUP N BY
* `85d30a2 <https://github.com/manticoresoftware/manticoresearch/commit/85d30a232d4ffd89916b673a9b2db946d272a945>`__ fixed deadlock on handling crash at memory operations
* `85166b5 <https://github.com/manticoresoftware/manticoresearch/commit/85166b5ef2be38061d62f2b20f15acaa0311258a>`__ fixed indextool memory consumption during check
* `58fb031 <https://github.com/manticoresoftware/manticoresearch/commit/58fb03166830d8b23e7ab9ce56309db7ddac22f4>`__ fixed gmock include not needed anymore as upstream resolve itself


Version 2.7.4 GA, 1 November 2018
---------------------------------

Improvements
~~~~~~~~~~~~

* SHOW THREADS in case of remote distributed indexes prints the original query instead of API call
* SHOW THREADS new option `format=sphinxql` prints all queries in SphinxQL format
* SHOW PROFILE prints additional `clone_attrs` stage

Bugfixes
~~~~~~~~

* `4f15571 <https://github.com/manticoresoftware/manticoresearch/commit/4f155712a0bccc1bd01cc191647bc8cff814888e>`__ fixed failed to build with libc without malloc_stats, malloc_trim
* `f974f20 <https://github.com/manticoresoftware/manticoresearch/commit/f974f20bda3214a56877c393a192be1a77150958>`__ fixed special symbols inside words for CALL KEYWORDS result set
* `0920832 <https://github.com/manticoresoftware/manticoresearch/commit/092083282ea8ae0b2e72fb6989c57ccec81e74ac>`__ fixed broken CALL KEYWORDS to distributed index via API or to remote agent
* `fd686bf <https://github.com/manticoresoftware/manticoresearch/commit/fd686bfe88b720ffd7642e36897ba45161cbd7d2>`__ fixed distributed index agent_query_timeout propagate to agents as max_query_time
* `4ffa623 <https://github.com/manticoresoftware/manticoresearch/commit/4ffa623e9d357a3b0b441615089e211f92f8de32>`__ fixed total documents counter at disk chunk got affected by OPTIMIZE command and breaks weight calculation
* `dcaf4e0 <https://github.com/manticoresoftware/manticoresearch/commit/dcaf4e0e3c8e9b0fbec47bbf3307feddec0936a6>`__ fixed multiple tail hits at RT index from blended
* `eee3817 <https://github.com/manticoresoftware/manticoresearch/commit/eee381754e902a43a8f499e1c950198a2a3e6ee0>`__ fixed deadlock at rotation


Version 2.7.3 GA, 26 September 2018
-----------------------------------

Improvements
~~~~~~~~~~~~
* sort_mode option for CALL KEYWORDS
* DEBUG on VIP connection can perform 'crash <password>' for intentional SIGEGV action on daemon
* DEBUG can perform 'malloc_stats' for dumping malloc stats in searchd.log 'malloc_trim' to perform a malloc_trim()
* improved backtrace is gdb is present on the system


Bugfixes
~~~~~~~~
* `0f3cc33 <https://github.com/manticoresoftware/manticoresearch/commit/0f3cc333f6129451ad1ae2c97905b6c04ba5d454>`__ fixed crash or hfailure of rename on Windows
* `1455ba2 <https://github.com/manticoresoftware/manticoresearch/commit/1455ba2bf8079f03ce7f861a8d9662a360d705de>`__ fixed crashes of daemon on 32-bit systems
* `ad3710d <https://github.com/manticoresoftware/manticoresearch/commit/ad3710d53b5cd6a28b8c60352d0f9e285b03d9fa>`__ fixed crash or hung of daemon on empty SNIPPET expression
* `b36d792 <https://github.com/manticoresoftware/manticoresearch/commit/b36d79214364500252665860f000ae85343528b6>`__ fixed broken non progressive optimize and fixed progressive optimize to not create kill-list for oldest disk chunk
* `34b0324 <https://github.com/manticoresoftware/manticoresearch/commit/34b032499afd42ce47a4c7247814b4031094388a>`__ fixed queue_max_length bad reply for SphinxQL and API at thread pool worker mode
* `ae4b320 <https://github.com/manticoresoftware/manticoresearch/commit/ae4b3202cbdb8014cabe2b90e269d5cf74f49871>`__ fixed crash on adding full-scan query to PQ index with regexp or rlp options set
* `f80f8d5 <https://github.com/manticoresoftware/manticoresearch/commit/f80f8d5d7560187078868aed9a9575f4549e98aa>`__ fixed crash when call one PQ after another
* `9742f5f <https://github.com/manticoresoftware/manticoresearch/commit/9742f5f0866af73f8cd8483ecd18a507ea80dd65>`__ refactor AquireAccum 
* `39e5bc3 <https://github.com/manticoresoftware/manticoresearch/commit/39e5bc3751b7295222eb76407c5d72ce1dad545b>`__ fixed leak of memory after call pq 
* `21bcc6d <https://github.com/manticoresoftware/manticoresearch/commit/21bcc6d17395f0f57dde79f6716ef303b7ea527d>`__ cosmetic refactor (c++11 style c-trs, defaults, nullptrs)
* `2d69039 <https://github.com/manticoresoftware/manticoresearch/commit/2d690398f14c736956cfdd66feb4d3091d6b3a4d>`__ fixed memory leak on trying to insert duplicate into PQ index
* `5ed92c4 <https://github.com/manticoresoftware/manticoresearch/commit/5ed92c4b66da6423fa28f11b1fd59103ed1e1fb9>`__ fixed crash on JSON field IN with large values
* `4a5262e <https://github.com/manticoresoftware/manticoresearch/commit/4a5262e2776aa261e34ba5c914ea60804f59de3f>`__ fixed crash of daemon on CALL KEYWORDS statement to RT index with expansion limit set
* `552646b <https://github.com/manticoresoftware/manticoresearch/commit/552646bb6fefde4e2525298e43e628839b421d3d>`__ fixed invalid filter at PQ matches query;
* `204f521 <https://github.com/manticoresoftware/manticoresearch/commit/204f521408b730198a5bab2c20407a3d7df94873>`__ introduce small obj allocator for ptr attrs
* `25453e5 <https://github.com/manticoresoftware/manticoresearch/commit/25453e5387152c0575d1eda1b2beb89a49c7493a>`__ refactor ISphFieldFilter to refcounted flavour
* `1366ee0 <https://github.com/manticoresoftware/manticoresearch/commit/1366ee06e828397074b69bec9265d7147170c60e>`__ fixed ub/sigsegv when using strtod on non-terminated strings
* `94bc6fc <https://github.com/manticoresoftware/manticoresearch/commit/94bc6fcbdae4d08bcf6da2966e3ec2966091289f>`__ fixed memory leak in json resultset processing
* `e78e9c9 <https://github.com/manticoresoftware/manticoresearch/commit/e78e9c948963416caff9e4e46296a58080107835>`__ fixed read over the end of mem block applying attribute add
* `fad572f <https://github.com/manticoresoftware/manticoresearch/commit/fad572fb543606b33e688eb82a485b86011545fd>`__ fixed refactor CSphDict for refcount flavour
* `fd841a4 <https://github.com/manticoresoftware/manticoresearch/commit/fd841a472b472280c93f4af3506851d19f6adc45>`__ fixed leak of AOT internal type outside
* `5ee7f20 <https://github.com/manticoresoftware/manticoresearch/commit/5ee7f208c7685f2ebb9e5623a8802b721e2a7f0a>`__ fixed memory leak tokenizer management 
* `116c5f1 <https://github.com/manticoresoftware/manticoresearch/commit/116c5f1abebee9a0d99afe93546a1d8e4c6c6958>`__ fixed memory leak in grouper
* `56fdbc9 <https://github.com/manticoresoftware/manticoresearch/commit/56fdbc9ab38973a9a0ba8ccee45f71cf33c16423>`__ special free/copy for dynamic ptrs in matches (memory leak grouper)
* `b1fc161 <https://github.com/manticoresoftware/manticoresearch/commit/31db660f378541aa0d1e7cf2e7209bb0a1f47fd8>`__ fixed memory leak of dynamic strings for RT
* `517b9e8 <https://github.com/manticoresoftware/manticoresearch/commit/517b9e8749f092e923cd2884b674b5bb84e20755>`__ refactor grouper
* `b1fc161 <https://github.com/manticoresoftware/manticoresearch/commit/b1fc16140e5dc44290686330b476a254e0092107>`__ minor refactor (c++11 c-trs, some reformats)
* `7034e07 <https://github.com/manticoresoftware/manticoresearch/commit/7034e073f4cf2844762e0a464b7c3de05d3d122b>`__ refactor ISphMatchComparator to refcounted flavour
* `b1fc161 <https://github.com/manticoresoftware/manticoresearch/commit/b1fc16140e5dc44290686330b476a254e0092107>`__ privatize cloner
* `efbc051 <https://github.com/manticoresoftware/manticoresearch/commit/efbc0511d6809c4a57453c7283f9bf53f3fb8d97>`__ simplify native little-endian for MVA_UPSIZE, DOCINFO2ID_T, DOCINFOSETID
* `6da0df4 <https://github.com/manticoresoftware/manticoresearch/commit/6da0df4ac9946fc59f98d229b90e12c9221dd4c0>`__ add valgrind support to to ubertests
* `1d17669 <https://github.com/manticoresoftware/manticoresearch/commit/1d17669cb7ec54ac80b392e101f37a688eb98080>`__ fixed crash because race of 'success' flag on connection
* `5a09c32 <https://github.com/manticoresoftware/manticoresearch/commit/5a09c32d7ded56b2c0bd3e2ad7968cb8d6362919>`__ switch epoll to edge-triggered flavour
* `5d52868 <https://github.com/manticoresoftware/manticoresearch/commit/5d528682737fca03a4352a3093b1319ec27d598c>`__ fixed IN statement in expression with formatting like at filter
* `bd8b3c9 <https://github.com/manticoresoftware/manticoresearch/commit/bd8b3c976ff8b4667f55dfdd101b20a920137ac5>`__ fixed crash at RT index on commit of document with large docid
* `ce656b8 <https://github.com/manticoresoftware/manticoresearch/commit/ce656b83b747ce7c55795b53915770c13fb99cfe>`__ fixed argless options in indextool
* `08c9507 <https://github.com/manticoresoftware/manticoresearch/commit/08c9507177820f441f534bf06a11dac5e54bebb4>`__ fixed memory leak of expanded keyword
* `30c75a2 <https://github.com/manticoresoftware/manticoresearch/commit/30c75a2f525ec9bda625d903acdc9d4d2e2413dc>`__ fixed memory leak of json grouper
* `6023f26 <https://github.com/manticoresoftware/manticoresearch/commit/6023f269b6f2080e4d380b0957605ef8107c8c9f>`__ fixed leak of global user vars 
* `7c138f1 <https://github.com/manticoresoftware/manticoresearch/commit/7c138f15ca23c0c8717fa12041e3db7f988fcf15>`__ fixed leakage of dynamic strings on early rejected matches
* `9154b18 <https://github.com/manticoresoftware/manticoresearch/commit/9154b18eaed5302e8330cb3a95bd968959ccb312>`__ fixed leakage on length(<expression>) 
* `43fca3a <https://github.com/manticoresoftware/manticoresearch/commit/43fca3a4e26139765d0fac8de054200dc4875fc6>`__ fixed memory leak because strdup() in parser 
* `71ff777 <https://github.com/manticoresoftware/manticoresearch/commit/71ff77737e63d1019b7c9804dca7fa2351025dba>`__ fixed refactor expression parser to accurate follow refcounts


Version 2.7.2 GA, 27 August 2018
--------------------------------

Improvements
~~~~~~~~~~~~
* compatibility with MySQL 8 clients
* :ref:`TRUNCATE <truncate_rtindex_syntax>` WITH RECONFIGURE
* retired memory counter on SHOW STATUS for RT indexes
* global cache of multi agents
* improved IOCP on Windows
* VIP connections for HTTP protocol
* SphinxQL  :ref:`DEBUG <debug_syntax>` command which can run various subcommands
* :ref:`shutdown_token` - SHA1 hash of password needed to invoke `shutdown` using DEBUG command
* new stats to SHOW AGENT STATUS (_ping, _has_perspool, _need_resolve)
* --verbose option of indexer now accept [debug|debugv|debugvv] for printing debug messages

Bugfixes
~~~~~~~~
* `390082 <https://github.com/manticoresoftware/manticoresearch/commit/390082a7be0a1f9539b30361d11d54de35c62a44>`__ removed wlock at optimize
* `4c3376 <https://github.com/manticoresoftware/manticoresearch/commit/4c337648329be22373be33333cbc9c5664d18b80>`__ fixed wlock at reload index settings
* `b5ea8d <https://github.com/manticoresoftware/manticoresearch/commit/b5ea8dc0b18a23dbd739e92c66dd93b0094df14b>`__ fixed memory leak on query with JSON filter
* `930e83 <https://github.com/manticoresoftware/manticoresearch/commit/930e831a43fd3d1ce996185be968d1df49a088d0>`__ fixed empty documents at PQ result set
* `53deec <https://github.com/manticoresoftware/manticoresearch/commit/53deecfecb374431f84f8592a1095a77407b8aea>`__ fixed confusion of tasks due to removed one
* `cad9b9 <https://github.com/manticoresoftware/manticoresearch/commit/cad9b9c617719aad97e9f5a2ae93491153d8e504>`__ fixed wrong remote host counting
* `90008c <https://github.com/manticoresoftware/manticoresearch/commit/90008cc15c4684d445d37a391bb4a667bbc2e365>`__ fixed memory leak of parsed agent descriptors
* `978d83 <https://github.com/manticoresoftware/manticoresearch/commit/978d839641ad6040c22c9fc3b703b1a02685f266>`__ fixed leak in search
* `019394 <https://github.com/manticoresoftware/manticoresearch/commit/0193946105ac08913362b7d7acf0c90f9b38dc85>`__ cosmetic changes on explicit/inline c-trs, override/final usage
* `943e29 <https://github.com/manticoresoftware/manticoresearch/commit/943e2997b33f9b2eda23d94a66068910f9476ebd>`__ fixed leak of json in local/remote schema 
* `02dbdd <https://github.com/manticoresoftware/manticoresearch/commit/02dbdd6f1d0b72e7e458ebebdd2c67f989577e6b>`__ fixed leak of json sorting col expr in local/remote schema
* `c74d0b <https://github.com/manticoresoftware/manticoresearch/commit/c74d0b4035e4214510376845b22cb676a8da24a3>`__ fixed leak of const alias 
* `6e5b57 <https://github.com/manticoresoftware/manticoresearch/commit/6e5b57e1d3952c9695376728a143bf5434208f53>`__ fixed leak of preread thread
* `39c740 <https://github.com/manticoresoftware/manticoresearch/commit/39c740fef5bf71da0bee60a35f5dd9f471af850f>`__ fixed stuck on exit because of stucked wait in netloop
* `adaf97 <https://github.com/manticoresoftware/manticoresearch/commit/adaf9772f95772ad6d5297acace6c5cb92b56ad5>`__ fixed stuck of 'ping' behaviour on change HA agent to usual host
* `32c40e <https://github.com/manticoresoftware/manticoresearch/commit/32c40eac9cdd40b15d79e58b2f15416f164f9701>`__ separate gc for dashboard storage
* `511a3c <https://github.com/manticoresoftware/manticoresearch/commit/511a3cf2c1f1858f6e94fe9f8175b7216db3cbd6>`__ fixed ref-counted ptr fix
* `32c40e <https://github.com/manticoresoftware/manticoresearch/commit/af2326cd2927effbad7bec88e8dc238a566cf27e>`__ fixed indextool crash on unexistent index
* `156edc <https://github.com/manticoresoftware/manticoresearch/commit/156edc717dc63d3ea120f42466eba6e022da88be>`__ fixed output name of exceeding attr/field in xmlpipe indexing
* `cdac6d <https://github.com/manticoresoftware/manticoresearch/commit/cdac6d1f17d384bfc8bae49e91241b2f346c3830>`__ fixed default indexer's value if no indexer section in config
* `e61ec0 <https://github.com/manticoresoftware/manticoresearch/commit/e61ec00b6b27d1d5878247e2ee817f3b1e7fde16>`__ fixed wrong embedded stopwords in disk chunk by RT index after daemon restart
* `5fba49 <https://github.com/manticoresoftware/manticoresearch/commit/5fba49d5e28c17de4c0acbd984466127f42de6e8>`__ fixed skip phantom (already closed, but not finally deleted from the poller) connections
* `f22ae3 <https://github.com/manticoresoftware/manticoresearch/commit/f22ae34b623906b3c5a05a06198dad2e548dc541>`__ fixed blended (orphaned) network tasks
* `46890e <https://github.com/manticoresoftware/manticoresearch/commit/46890e70eb8dcd00c1bf1b030538b8f131c601c2>`__ fixed crash on read action after write
* `03f9df <https://github.com/manticoresoftware/manticoresearch/commit/03f9df904f611c438d5ebcaaeef911b0dc8d6c39>`__ fixed searchd crashes when running tests on windows
* `e9255e <https://github.com/manticoresoftware/manticoresearch/commit/e9255ed2704790a3d7f5273d38b85433463cb3ff>`__ fixed handle EINPROGRESS code on usual connect() 
* `248b72 <https://github.com/manticoresoftware/manticoresearch/commit/248b72f1544b27e9e82773284050d44eeb1714e1>`__ fixed connection timeouts when working with TFO



Version 2.7.1 GA, 4 July 2018
-----------------------------

Improvements
~~~~~~~~~~~~
* improved wildcards performance on matching multiple documents at PQ
* support for fullscan queries at PQ
* support for MVA attributes at PQ
* regexp and RLP support for percolate indexes

Bugfixes
~~~~~~~~
* `688562 <https://github.com/manticoresoftware/manticoresearch/commit/68856261b41f6950666f9e5122726839fb3c71d0>`__ fixed loose of query string
* `0f1770 <https://github.com/manticoresoftware/manticoresearch/commit/0f1770943cba53bfd2e0edfdf0b1495ff0dd0e89>`__ fixed empty info at SHOW THREADS statement
* `53faa3 <https://github.com/manticoresoftware/manticoresearch/commit/53faa36a916bb87868e83aa79c1a1f972fb20ca0>`__ fixed crash on matching with NOTNEAR operator
* `26029a <https://github.com/manticoresoftware/manticoresearch/commit/26029a7cd77b518fb3a27588d7db6c8bffc73b0f>`__ fixed error message on bad filter to PQ delete

Version 2.7.0 GA, 11 June 2018
------------------------------

Improvements
~~~~~~~~~~~~
* reduced number of syscalls to avoid Meltdown and Spectre patches impact
* internal rewrite of local index management
* remote snippets refactor
* full configuration reload
* all node connections are now independent
* proto improvements
* Windows communication switched from wsapoll to IO completion ports
* TFO can be used for communication between master and nodes
* :ref:`SHOW STATUS <show_status_syntax>` now outputs to daemon version and mysql_version_string
* added `docs_id` option for documents called in CALL PQ. 
* percolate queries filter can now contain expressions
* distributed indexes can work with FEDERATED
* dummy SHOW NAMES COLLATE and :ref:`SET <set_syntax>` wait_timeout (for better ProxySQL compatibility)

Bugfixes
~~~~~~~~
* `5bcff0 <https://github.com/manticoresoftware/manticoresearch/commit/5bcff05d19e495f1a44f190885b25cbb6f217c4a>`__ fixed  added not equal to tags of PQ
* `9ebc58 <https://github.com/manticoresoftware/manticoresearch/commit/9ebc58916cd515eaa88da66d0895aebf0d1f2b5f>`__ fixed added document id field to JSON document CALL PQ statement
* `8ae0e5 <https://github.com/manticoresoftware/manticoresearch/commit/8ae0e593a286110d8505d88baa3ac9e433cb38c9>`__ fixed flush statement handlers to PQ index
* `c24b15 <https://github.com/manticoresoftware/manticoresearch/commit/c24b152344ecd77661566cddb803487f51d3c1aa>`__ fixed PQ filtering on JSON and string attributes
* `1b8bdd <https://github.com/manticoresoftware/manticoresearch/commit/1b8bdde19aaa362785ea4c33bdc019154f8fbac4>`__ fixed parsing of empty JSON string
* `1ad8a0 <https://github.com/manticoresoftware/manticoresearch/commit/1ad8a07dbfc1131913a8d92c261fbb48f934e8b7>`__ fixed crash at multi-query with OR filters
* `69b898 <https://github.com/manticoresoftware/manticoresearch/commit/69b89806380a229e36287ad9daf6f0ea2b5453eb>`__ fixed indextool to use config common section (lemmatizer_base option) for commands (dumpheader)
* `6dbeaf <https://github.com/manticoresoftware/manticoresearch/commit/6dbeaf2389d2af6a9dfccb56bfc986685b9f203e>`__ fixed empty string at result set and filter
* `39c4eb <https://github.com/manticoresoftware/manticoresearch/commit/39c4eb55ecc85f6dd54f6c8c6d6dfcf87fd8748e>`__ fixed negative document id values
* `266b70 <https://github.com/manticoresoftware/manticoresearch/commit/266b7054f98e7d30fc2829b5c2467dc9f90def92>`__ fixed word clip length for very long words indexed
* `47823b <https://github.com/manticoresoftware/manticoresearch/commit/47823b0bd06cbd016a24345c99bb8693790b43dd>`__ fixed matching multiple documents of wildcard queries at PQ


Version 2.6.4 GA, 3 May 2018
----------------------------

Features and improvements
~~~~~~~~~~~~~~~~~~~~~~~~~

* MySQL FEDERATED engine :ref:`support <federated_storage_engine>`
* MySQL packets return now SERVER_STATUS_AUTOCOMMIT flag, adds compatibility with ProxySQL
* :ref:`listen_tfo` - enable TCP Fast Open connections for all listeners
* indexer --dumpheader can dump also RT header from .meta file
* cmake build script for Ubuntu Bionic

Bugfixes
~~~~~~~~
* `355b116 <https://github.com/manticoresoftware/manticoresearch/commit/355b11629174813abd9cd5bf2233be0783f77745>`__ fixed invalid query cache entries for RT index;
* `546e229 <https://github.com/manticoresoftware/manticoresearch/commit/546e2297a01cc2913bc0d33052d5bcefae8737eb>`__ fixed index settings got lost next after seamless rotation
* `0c45098 <https://github.com/manticoresoftware/manticoresearch/commit/0c4509898393993bc87194d2aca2070395ff7f83>`__ fixed fixed infix vs prefix length set; added warning on unsupported infix length
* `80542fa <https://github.com/manticoresoftware/manticoresearch/commit/80542fa54d727c781635918e8d4d65a20c2fbd1e>`__ fixed RT indexes auto-flush order
* `705d8c5 <https://github.com/manticoresoftware/manticoresearch/commit/705d8c5f407726ed1c6f9745ecffa375682c8969>`__ fixed result set schema issues for index with multiple attributes and queries to multiple indexes
* `b0ba932 <https://github.com/manticoresoftware/manticoresearch/commit/b0ba932fff4a082f642cb737381c8a70215d4cc9>`__ fixed some hits got lost at batch insert with document duplicates
* `4510fa4 <https://github.com/manticoresoftware/manticoresearch/commit/4510fa44ad54b5bbac53c591144501565130fcae>`__ fixed optimize failed to merge disk chunks of RT index with large documents count

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
