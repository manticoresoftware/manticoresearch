# UDFs and Plugins

Manticore can be extended with user-defined functions, or UDFs for short, like this:

```sql
SELECT id, attr1, myudf (attr2, attr3+attr4) ...
```

You can dynamically load and unload UDFs into `searchd` without having to restart the server, and use them in expressions when searching, ranking, etc. A quick summary of the UDF features is as follows:

* UDFs can take integer (both 32-bit and 64-bit), float, string, MVA, or `PACKEDFACTORS()` arguments.
* UDFs can return integer, float, or string values.
* UDFs can check the argument number, types, and names during the query setup phase, and raise errors.

We do not yet support aggregation functions. In other words, your UDFs will be called for just a single document at a time and are expected to return some value for that document. Writing a function that can compute an aggregate value like AVG() over the entire group of documents that share the same GROUP BY key is not yet possible. However, you can use UDFs within the built-in aggregate functions: that is, even though MYCUSTOMAVG() is not supported yet, AVG(MYCUSTOMFUNC()) should work just fine!

UDFs offer a wide range of applications, such as:

* incorporating custom mathematical or string functions;
* accessing databases or files from within Manticore;
* creating complex ranking functions.

## Plugins

Plugins offer additional opportunities to expand search functionality. They can currently be used to compute custom rankings and tokenize documents and queries.

Here's the complete list of plugin types:

* UDF plugins (essentially UDFs, but since they're plugged in, they're also referred to as 'UDF plugins')
* ranker plugins
* indexing-time token filter plugins
* query-time token filter plugins

This section covers the general process of writing and managing plugins; specifics related to creating different types of plugins are discussed in their respective subsections.

So, how do you write and use a plugin? Here's a quick four-step guide:

* create a dynamic library (either .so or .dll), most likely using C or C++;
* load the plugin into searchd with [CREATE PLUGIN](../../Extensions/UDFs_and_Plugins/Plugins/Creating_a_plugin.md);
* use the plugin with plugin-specific calls (usually through specific OPTIONS).
* unload or reload a plugin with [DROP PLUGIN](../../Extensions/UDFs_and_Plugins/Plugins/Deleting_a_plugin.md) and [RELOAD PLUGINS](../../Extensions/UDFs_and_Plugins/Plugins/Reloading_plugins.md), respectively.

Note that while UDFs are first-class plugins, they are installed using a separate [CREATE FUNCTION](../../Extensions/UDFs_and_Plugins/UDF/Creating_a_function.md) statement. This allows for a neat specification of the return type, without sacrificing backward compatibility or changing the syntax.

Dynamic plugins are supported in threads and thread_pool workers. Multiple plugins (and/or UDFs) can be contained in a single library file. You may choose to either group all project-specific plugins in one large library or create a separate library for each UDF and plugin; it's up to you.

As with UDFs, you should include the `src/sphinxudf.h` header file. At the very least, you'll need the `SPH_UDF_VERSION` constant to implement an appropriate version function. Depending on the specific plugin type, you may or may not need to link your plugin with `src/sphinxudf.c`. However, all functions implemented in `sphinxudf.c` are related to unpacking the `PACKEDFACTORS()` blob, and no plugin types have access to that data. So currently, linking with just the header should suffice. (In fact, if you copy over the UDF version number, you won't even need the header file for some plugin types.)

Formally, plugins are simply sets of C functions that adhere to a specific naming pattern. You're typically required to define one key function for the primary task, but you can also define additional functions. For instance, to implement a ranker called "myrank", you must define a `myrank_finalize()` function that returns the rank value. However, you can also define `myrank_init()`, `myrank_update()`, and `myrank_deinit()` functions. Specific sets of well-known suffixes and call arguments differ based on the plugin type, but _init() and _deinit() are generic, and every plugin has them. Hint: for a quick reference on known suffixes and their argument types, refer to `sphinxplugin.h`, where the call prototypes are defined at the beginning of the file.

Even though the public interface is defined in pure C, our plugins essentially follow an *object-oriented model*. Indeed, every `_init()` function receives a `void ** userdata` out-parameter, and the pointer value stored at `(*userdata)` is then passed as the first argument to all other plugin functions. So you can think of a plugin as a *class* that gets instantiated every time an object of that class is needed to handle a request: the `userdata` pointer serves as the `this` pointer; the functions act as methods, and the `_init()` and `_deinit()` functions work as constructor and destructor, respectively.

This minor OOP-in-C complication arises because plugins run in a multi-threaded environment, and some need to maintain state. You can't store that state in a global variable in your plugin, so we pass around a userdata parameter, which naturally leads to the OOP model. If your plugin is simple and stateless, the interface allows you to omit `_init()`, `_deinit()`, and any other functions.

To summarize, here's the simplest complete ranker plugin in just three lines of C code:

```c
// gcc -fPIC -shared -o myrank.so myrank.c
#include "sphinxudf.h"
int myrank_ver() { return SPH_UDF_VERSION; }
int myrank_finalize(void *u, int w) { return 123; }
```

Here's how to use the simple ranker plugin:

```sql
mysql> CREATE PLUGIN myrank TYPE 'ranker' SONAME 'myrank.dll';
Query OK, 0 rows affected (0.00 sec)

mysql> SELECT id, weight() FROM test1 WHERE MATCH('test') OPTION ranker=myrank('');
+------+----------+
| id   | weight() |
+------+----------+
|    1 |      123 |
|    2 |      123 |
+------+----------+
2 rows in set (0.01 sec)
```
<!-- proofread -->

