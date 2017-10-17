Release notes
===================================

Version 2.4.1 GA, 16 October 2017
----------------------------------

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
----------------------------------

- Manticore branding