.. _extending:

Extending
===========================

.. _udfs_user_defined_functions:

UDFs (User Defined Functions)
------------------------------------

Our expression engine can be extended with user defined functions, or
UDFs for short, like this:

.. code-block:: mysql


    SELECT id, attr1, myudf(attr2, attr3+attr4) ...

You can load and unload UDFs dynamically into ``searchd`` without having
to restart the daemon, and used them in expressions when searching,
ranking, etc. Quick summary of the UDF features is as follows.

-  UDFs can take integer (both 32-bit and 64-bit), float, string, MVA,
   or PACKEDFACTORS() arguments.

-  UDFs can return integer, float, or string values.

-  UDFs can check the argument number, types, and names during the query
   setup phase, and raise errors.

-  Aggregation UDFs are not yet supported (but might be in the future).

UDFs have a wide variety of uses, for instance:

-  adding custom mathematical or string functions;

-  accessing the database or files from within Manticore;

-  implementing complex ranking functions.

UDFs reside in the external dynamic libraries (.so files on UNIX and
.dll on Windows systems). Library files need to reside in a trusted
folder specified by
:ref:`plugin_dir <plugin_dir>`
directive, for obvious security reasons: securing a single folder is
easy; letting anyone install arbitrary code into ``searchd`` is a risk.
You can load and unload them dynamically into searchd with :ref:`CREATE
FUNCTION <create_function_syntax>` and :ref:`DROP
FUNCTION <drop_function_syntax>` SphinxQL statements
respectively. Also, you can seamlessly reload UDFs (and other plugins)
with :ref:`RELOAD PLUGINS <reload_plugins_syntax>` statement. Manticore
keeps track of the currently loaded functions, that is, every time you
create or drop an UDF, ``searchd`` writes its state to the
:ref:`sphinxql_state <sphinxql_state>`
file as a plain good old SQL script.

Once you successfully load an UDF, you can use it in your SELECT or
other statements just as well as any of the builtin functions:

.. code-block:: mysql


    SELECT id, MYCUSTOMFUNC(groupid, authorname), ... FROM myindex

Multiple UDFs (and other plugins) may reside in a single library. That
library will only be loaded once. It gets automatically unloaded once
all the UDFs and plugins from it are dropped.

In theory you can write an UDF in any language as long as its compiler
is able to import standard C header, and emit standard dynamic libraries
with properly exported functions. Of course, the path of least
resistance is to write in either C++ or plain C. We provide an example
UDF library written in plain C and implementing several functions
(demonstrating a few different techniques) along with our source code,
see
`src/udfexample.c <https://github.com/manticoresoftware/manticore/blob/master/src/udfexample.c>`__.
That example includes
`src/sphinxudf.h <https://github.com/manticoresoftware/manticore/blob/master/src/sphinxudf.h>`__
header file definitions of a few UDF related structures and types. For
most UDFs and plugins, a mere ``#include "sphinxudf.h"``, like
in the example, should be completely sufficient, too. However, if you're
writing a ranking function and need to access the ranking signals
(factors) data from within the UDF, you will also need to compile and
link with ``src/sphinxudf.c`` (also available in our source code),
because the *implementations* of the fuctions that let you access the
signal data from within the UDF reside in that file.

Both ``sphinxudf.h`` header and ``sphinxudf.c`` are standalone. So you
can copy around those files only; they do not depend on any other bits
of Manticore source code.

Within your UDF, you **must** implement and export only a couple
functions, literally. First, for UDF interface version control, you
**must** define a function ``int LIBRARYNAME_ver()``, where
LIBRARYNAME is the name of your library file, and you must return
``SPH_UDF_VERSION`` (a value defined in ``sphinxudf.h``) from it. Here's
an example.

.. code-block:: c


    #include <sphinxudf.h>

    // our library will be called udfexample.so, thus, so it must define
    // a version function named udfexample_ver()
    int udfexample_ver()
    {
        return SPH_UDF_VERSION;
    }

That protects you from accidentally loading a library with a mismatching
UDF interface version into a newer or older ``searchd``. Second, yout
**must** implement the actual function, too.
``sphinx_int64_t testfunc ( SPH_UDF_INIT * init, SPH_UDF_ARGS * args, char * error_flag ) { return 123; }``

UDF function names in SphinxQL are case insensitive. However, the
respective C function names are not, they need to be all
*lower-case*, or the UDF will not load. More importantly, it is
vital that a) the calling convention is C (aka __cdecl), b) arguments
list matches the plugin system expectations exactly, and c) the return
type matches the one you specify in ``CREATE FUNCTION``. Unfortunately,
there is no (easy) way for us to check for those mistakes when loading
the function, and they could crash the server and/or result in
unexpected results. Last but not least, all the C functions you
implement need to be thread-safe.

The first argument, a pointer to SPH_UDF_INIT structure, is
essentially a pointer to our function state. It is option. In the
example just above the function is stateless, it simply returns 123
every time it gets called. So we do not have to define an initialization
function, and we can simply ignore that argument.

The second argument, a pointer to SPH_UDF_ARGS, is the most important
one. All the actual call arguments are passed to your UDF via this
structure; it contians the call argument count, names, types, etc. So
whether your function gets called like ``SELECT id, testfunc(1)`` or
like ``SELECT id, testfunc(&#039;abc&#039;, 1000*id+gid, WEIGHT())`` or
anyhow else, it will receive the very same SPH_UDF_ARGS structure in
all of these cases. However, the data passed in the ``args`` structure
will be different. In the first example ``args->arg_count`` will be
set to 1, in the second example it will be set to 3,
``args->arg_types`` array will contain different type data, and so
on.

Finally, the third argument is an error flag. UDF can raise it to
indicate that some kinda of an internal error happened, the UDF can not
continue, and the query should terminate early. You should **not**
use this for argument type checks or for any other error reporting that
is likely to happen during normal use. This flag is designed to report
sudden critical runtime errors, such as running out of memory.

If we wanted to, say, allocate temporary storage for our function to
use, or check upfront whether the arguments are of the supported types,
then we would need to add two more functions, with UDF initialization
and deinitialization, respectively.

.. code-block:: c


    int testfunc_init ( SPH_UDF_INIT * init, SPH_UDF_ARGS * args,
        char * error_message )
    {
        // allocate and initialize a little bit of temporary storage
        init->func_data = malloc ( sizeof(int) );
        *(int*)init->func_data = 123;

        // return a success code
        return 0;
    }

    void testfunc_deinit ( SPH_UDF_INIT * init )
    {
        // free up our temporary storage
        free ( init->func_data );
    }

Note how ``testfunc_init()`` also receives the call arguments structure.
By the time it is called it does not receive any actual values, so the
``args->arg_values`` will be NULL. But the argument names and types
are known and will be passed. You can check them in the initialization
function and return an error if they are of an unsupported type.

UDFs can receive arguments of pretty much any valid internal Manticore
type. Refer to ``sphinx_udf_argtype`` enumeration in ``sphinxudf.h`` for
a full list. Most of the types map straightforwardly to the respective C
types. The most notable exception is the SPH_UDF_TYPE_FACTORS
argument type. You get that type by calling your UDF with a
:ref:`PACKEDFACTOR() <expr-func-packedfactors>`
argument. It's data is a binary blob in a certain internal format, and
to extract individual ranking signals from that blob, you need to use
either of the two ``sphinx_factors_XXX()`` or
``sphinx_get_YYY_factor()`` families of functions. The first family
consists of just 3 functions, ``sphinx_factors_init()`` that initializes
the unpacked SPH_UDF_FACTORS structure, ``sphinx_factors_unpack()``
that unpacks a binary blob into it, and ``sphinx_factors_deinit()`` that
cleans up an deallocates the SPH_UDF_FACTORS. So you need to call
init() and unpack(), then you can use the SPH_UDF_FACTORS fields, and
then you need to cleanup with deinit(). That is simple, but results in a
bunch of memory allocations per each processed document, and might be
slow. The other interface, consisting of a bunch of
``sphinx_get_YYY_factor()`` functions, is a little more wordy to use,
but accesses the blob data directly and guarantees that there will be
zero allocations. So for top-notch ranking UDF performance, you want to
use that one.

As for the return types, UDFs can currently return a signle INTEGER,
BIGINT, FLOAT, or STRING value. The C function return type should be
sphinx_int64_t, sphinx_int64_t, double, or char\* respectively. In
the last case you **must** use ``args->fn_malloc`` function to
allocate the returned string values. Internally in your UDF you can use
whatever you want, so the ``testfunc_init()`` example above is correct
code even though it uses malloc() directly: you manage that pointer
yourself, it gets freed up using a matching free() call, and all is
well. However, the returned strings values are managed by Manticore and we
have our own allocator, so for the return values specifically, you need
to use it too.

Depending on how your UDFs are used in the query, the main function call
(``testfunc()`` in our example) might be called in a rather different
volume and order. Specifically,

-  UDFs referenced in WHERE, ORDER BY, or GROUP BY clauses must and will
   be evaluated for every matched document. They will be called in the
   natural matching order.

-  without subselects, UDFs that can be evaluated at the very last stage
   over the final result set will be evaluated that way, but before
   applying the LIMIT clause. They will be called in the result set
   order.

-  with subselects, such UDFs will also be evaluated after applying the
   inner LIMIT clause.

The calling sequence of the other functions is fixed, though. Namely,

-  ``testfunc_init()`` is called once when initializing the query. It
   can return a non-zero code to indicate a failure; in that case query
   will be terminated, and the error message from the ``error_message``
   buffer will be returned.

-  ``testfunc()`` is called for every eligible row (see above), whenever
   Manticore needs to compute the UDF value. It can also indicate an
   (internal) failure error by writing a non-zero byte value to
   ``error_flag``. In that case, it is guaranteed that will no more be
   called for subsequent rows, and a default return value of 0 will be
   substituted. Manticore might or might not choose to terminate such
   queries early, neither behavior is currently guaranteed.

-  ``testfunc_deinit()`` is called once when the query processing (in a
   given index shard) ends.

We do not yet support aggregation functions. In other words, your UDFs
will be called for just a single document at a time and are expected to
return some value for that document. Writing a function that can compute
an aggregate value like AVG() over the entire group of documents that
share the same GROUP BY key is not yet possible. However, you can use
UDFs within the builtin aggregate functions: that is, even though
MYCUSTOMAVG() is not supported yet, AVG(MYCUSTOMFUNC()) should work
alright!

UDFs are local. In order to use them on a cluster, you have to put the
same library on all its nodes and run CREATEs on all the nodes too. This
might change in the future versions.

.. _plugins:

Plugins
--------------

Here's the complete plugin type list.

-  UDF plugins;

-  ranker plugins;

-  indexing-time token filter plugins;

-  query-time token filter plugins.

This section discusses writing and managing plugins in general; things
specific to writing this or that type of a plugin are then discussed in
their respective subsections.

So, how do you write and use a plugin? Four-line crash course goes as
follows:

-  create a dynamic library (either .so or.dll), most likely in C or
   C++;

-  load that plugin into searchd using :ref:`CREATE
   PLUGIN <create_plugin_syntax>`;

-  invoke it using the plugin specific calls (typically using this or
   that OPTION).

-  to unload or reload a plugin use :ref:`DROP
   PLUGIN <drop_plugin_syntax>` and :ref:`RELOAD
   PLUGINS <reload_plugins_syntax>` respectively.

Note that while UDFs are first-class plugins they are nevertheless
installed using a separate :ref:`CREATE
FUNCTION <create_function_syntax>` statement. It lets you
specify the return type neatly so there was especially little reason to
ruin backwards compatibility *and* change the syntax.

Dynamic plugins are supported in `workers =
threads <workers>` and
`workers =
thread_pool <workers>`
mode only. Multiple plugins (and/or UDFs) may reside in a single library
file. So you might choose to either put all your project-specific
plugins in a single common uber-library; or you might choose to have a
separate library for every UDF and plugin; that is up to you.

Just as with UDFs, you want to include ``src/sphinxudf.h`` header file.
At the very least, you will need the SPH_UDF_VERSION constant to
implement a proper version function. Depending on the specific plugin
type, you might or might not need to link your plugin with
``src/sphinxudf.c``. However, all the functions implemented in
``sphinxudf.c`` are about unpacking the PACKEDFACTORS() blob, and no
plugin types are exposed to that kind of data. So currently, you would
never need to link with the C-file, just the header would be sufficient.
(In fact, if you copy over the UDF version number, then for some of the
plugin types you would not even need the header file.)

Formally, plugins are just sets of C functions that follow a certain
naming parttern. You are typically required to define just one key
function that does the most important work, but you may define a bunch
of other functions, too. For example, to implement a ranker called
“myrank”, you must define ``myrank_finalize()`` function that actually
returns the rank value, however, you might also define
``myrank_init()``, ``myrank_update()``, and ``myrank_deinit()``
functions. Specific sets of well-known suffixes and the call arguments
do differ based on the plugin type, but _init() and _deinit() are
generic, every plugin has those. Protip: for a quick reference on the
known suffixes and their argument types, refer to ``sphinxplugin.h``, we
define the call prototoypes in the very beginning of that file.

Despite having the public interface defined in ye good olde good pure C,
our plugins essentially follow the *object-oriented model*. Indeed,
every ``_init()`` function receives a ``void ** userdata``
out-parameter. And the pointer value that you store at ``(*userdata)``
location is then be passed as a 1st argument to all the other plugin
functions. So you can think of a plugin as *class* that gets
instantiated every time an object of that class is needed to handle a
request: the ``userdata`` pointer would be its ``this`` pointer; the
functions would be its methods, and the ``_init()`` and ``_deinit()``
functions would be the constructor and destructor respectively.

Why this (minor) OOP-in-C complication? Well, plugins run in a
multi-threaded environment, and some of them have to be stateful. You
can't keep that state in a global variable in your plugin. So we have to
pass around a userdata parameter anyway to let you keep that state. And
that naturally brings us to the OOP model. And if you've got a simple,
stateless plugin, the interface lets you omit the ``_init()`` and
``_deinit()`` and whatever other functions just as well.

To summarize, here goes the simplest complete ranker plugin, in just 3
lines of C code.

.. code-block:: c


    // gcc -fPIC -shared -o myrank.so myrank.c
    #include "sphinxudf.h"
    int myrank_ver() { return SPH_UDF_VERSION; }
    int myrank_finalize(void *u, int w) { return 123; }

And this is how you use it:

.. code-block:: mysql


    mysql> CREATE PLUGIN myrank TYPE 'ranker' SONAME 'myrank.dll';
    Query OK, 0 rows affected (0.00 sec)

    mysql> SELECT id, weight() FROM test1 WHERE MATCH('test')
        -> OPTION ranker=myrank('');
    +------+----------+
    | id   | weight() |
    +------+----------+
    |    1 |      123 |
    |    2 |      123 |
    +------+----------+
    2 rows in set (0.01 sec)


.. _ranker_plugins:

Ranker plugins
--------------

Ranker plugins let you implement a custom ranker that receives all the
occurrences of the keywords matched in the document, and computes a
WEIGHT() value. They can be called as follows:

.. code-block:: mysql


    SELECT id, attr1 FROM test WHERE match('hello')
    OPTION ranker=myranker('option1=1');

The call workflow is as follows:

1. ``XXX_init()`` gets called once per query per index, in the very
   beginning. A few query-wide options are passed to it through a
   ``SPH_RANKER_INIT`` structure, including the user options strings (in
   the example just above, “option1=1” is that string).
2. ``XXX_update()`` gets called multiple times per matched document,
   with every matched keyword occurrence passed as its parameter, a
   ``SPH_RANKER_HIT`` structure. The occurrences within each document
   are guaranteed to be passed in the order of ascending
   ``hit->hit_pos`` values.
3. ``XXX_finalize()`` gets called once per matched document, once there
   are no more keyword occurrences. It must return the WEIGHT() value.
   This is the only mandatory function.
4. ``XXX_deinit()`` gets called once per query, in the very end.
