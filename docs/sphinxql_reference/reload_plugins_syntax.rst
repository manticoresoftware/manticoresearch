.. _reload_plugins_syntax:

RELOAD PLUGINS syntax
---------------------

::


    RELOAD PLUGINS FROM SONAME 'plugin_library'

Reloads all plugins (UDFs, rankers, etc) from a given library. Reload
is, in a sense, transactional: a successful reload guarantees that a)
all the plugins were successfully updated with their new versions; b)
the update was atomic, all the plugins were replaced at once. Atomicity
means that queries using multiple functions from a reloaded library will
never mix the old and new versions. The set of plugins is guaranteed to
always be consistent during the RELOAD, it will be either all old, or
all new.

Reload also is seamless, meaning that some version of a reloaded plugin
will be available to concurrent queries at all times, and there will be
no temporary disruptions. Note how this improves on using a pair of DROP
and CREATE statements for reloading: with those, there is a tiny window
between the DROP and the subsequent CREATE, during which the queries
technically refer to an unknown plugin and will thus fail.

In case of any failure RELOAD PLUGINS does absolutely nothing, keeps the
old plugins, and reports an error.

On Windows, either overwriting or deleting a DLL library currently in
use seems to be an issue. However, you can still rename it, then put a
new version under the old name, and RELOAD will then work. After a
succesful reload you will also be able to delete the renamed old
library, too.

::


    mysql> RELOAD PLUGINS FROM SONAME 'udfexample.dll';
    Query OK, 0 rows affected (0.00 sec)

