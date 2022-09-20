# UDF 

UDFs reside in the external dynamic libraries (.so files on UNIX and .dll on Windows systems). Library files need to reside in a trusted folder specified by [plugin_dir](../../Server_settings/Common.md#plugin_dir) directive, for obvious security reasons: securing a single folder is easy; letting anyone install arbitrary code into `searchd` is a risk. You can load and unload them dynamically into searchd with [CREATE FUNCTION](../../Extensions/UDFs_and_Plugins/UDF/Creating_a_function.md) and [DROP FUNCTION](../../Extensions/UDFs_and_Plugins/UDF/Deleting_a_function.md) SQL statements respectively. Also, you can seamlessly reload UDFs (and other plugins) with [RELOAD PLUGINS](../../Extensions/UDFs_and_Plugins/Plugins/Reloading_plugins.md) statement. Manticore keeps track of the currently loaded functions, that is, every time you create or drop an UDF, `searchd` writes its state to the [sphinxql_state](../../Server_settings/Searchd.md#sphinxql_state) file as a plain good old SQL script.

UDFs are local. In order to use them on a cluster, you have to put the same library on all its nodes and run CREATEs on all the nodes too. This might change in the future versions.

Once you successfully load an UDF, you can use it in your SELECT or other statements just as well as any of the builtin functions:

```sql
SELECT id, MYCUSTOMFUNC (groupid, authorname), ... FROM myindex
```

Multiple UDFs (and other plugins) may reside in a single library. That library will only be loaded once. It gets automatically unloaded once all the UDFs and plugins from it are dropped.

In theory you can write an UDF in any language as long as its compiler is able to import standard C header, and emit standard dynamic libraries with properly exported functions. Of course, the path of least resistance is to write in either C++ or plain C. We provide an example UDF library written in plain C and implementing several functions (demonstrating a few different techniques) along with our source code, see [src/udfexample.c](https://github.com/manticoresoftware/manticore/blob/master/src/udfexample.c). That example includes [src/sphinxudf.h](https://github.com/manticoresoftware/manticore/blob/master/src/sphinxudf.h) header file definitions of a few UDF related structures and types. For most UDFs and plugins, a mere `#include "sphinxudf.h"`, like in the example, should be completely sufficient, too. However, if you're writing a ranking function and need to access the ranking signals (factors) data from within the UDF, you will also need to compile and link with `src/sphinxudf.c` (also available in our source code), because the *implementations* of the functions that let you access the signal data from within the UDF reside in that file.

Both `sphinxudf.h` header and `sphinxudf.c` are standalone. So you can copy around those files only; they do not depend on any other bits of Manticore source code.

Within your UDF, you **must** implement and export only a couple functions, literally. First, for UDF interface version control, you **must** define a function `int LIBRARYNAME_ver()`, where LIBRARYNAME is the name of your library file, and you must return `SPH_UDF_VERSION` (a value defined in `sphinxudf.h`) from it. Here's an example.

```c
#include <sphinxudf.h>

// our library will be called udfexample.so, thus, so it must define
// a version function named udfexample_ver()
int udfexample_ver()
{
    return SPH_UDF_VERSION;
}
```

That protects you from accidentally loading a library with a mismatching UDF interface version into a newer or older `searchd`. Second, your **must** implement the actual function, too.

```c
sphinx_int64_t testfunc ( SPH_UDF_INIT * init, SPH_UDF_ARGS * args, char * error_flag )
{
    return 123;
}
```

UDF function names in SQL are case insensitive. However, the respective C function names are not, they need to be all *lower-case*, or the UDF will not load. More importantly, it is vital that
 
 1. the calling convention is C (aka \_\_cdecl),
 2. arguments list matches the plugin system expectations exactly, and
 3. the return type matches the one you specify in `CREATE FUNCTION`.
 
Unfortunately, there is no (easy) way for us to check for those mistakes when loading the function, and they could crash the server and/or result in unexpected results. Last but not least, all the C functions you implement need to be thread-safe.

The first argument, a pointer to `SPH_UDF_INIT` structure, is essentially a pointer to our function state. It is optional. In the example just above the function is stateless, it simply returns 123 every time it gets called. So we do not have to define an initialization function, and we can simply ignore that argument.
This argument serves one more purpose. Since a single query can be executed on multiple threads (see [pseudo-sharding](../Server_settings/Searchd.md#pseudo_sharding)), daemon tries to determine whether UDF is stateful or stateless by checking this argument. If the argument is initialized, parallel execution will be disabled. So if your UDF is stateful but you don't use this argument, it will be called from multiple threads and your code needs to be aware of that.

The second argument, a pointer to `SPH_UDF_ARGS`, is the most important one. All the actual call arguments are passed to your UDF via this structure; it contains the call argument count, names, types, etc. So whether your function gets called like `SELECT id, testfunc(1)` or like `SELECT id, testfunc('abc', 1000*id+gid, WEIGHT())` or anyhow else, it will receive the very same `SPH_UDF_ARGS` structure in all of these cases. However, the data passed in the `args` structure will be different. In the first example `args->arg_count` will be set to 1, in the second example it will be set to 3, `args->arg_types` array will contain different type data, and so on.

Finally, the third argument is an error flag. UDF can raise it to indicate that some kinda of an internal error happened, the UDF can not continue, and the query should terminate early. You should **not** use this for argument type checks or for any other error reporting that is likely to happen during normal use. This flag is designed to report sudden critical runtime errors, such as running out of memory.

If we wanted to, say, allocate temporary storage for our function to use, or check upfront whether the arguments are of the supported types, then we would need to add two more functions, with UDF initialization and deinitialization, respectively.

```c
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
```

Note how `testfunc_init()` also receives the call arguments structure. By the time it is called it does not receive any actual values, so the `args->arg_values` will be NULL. But the argument names and types are known and will be passed. You can check them in the initialization function and return an error if they are of an unsupported type.


## SPH_UDF_ARGS types

UDFs can receive arguments of pretty much any valid internal Manticore type. Refer to `sphinx_udf_argtype` enumeration in `sphinxudf.h` for a full list. Most of the types map straightforwardly to the respective C types.

The most notable type is the `SPH_UDF_TYPE_FACTORS` argument type. You get that type by calling your UDF with a [PACKEDFACTOR()](../../searching-and-ranking-functions#PACKEDFACTORS()) argument. It's data is a binary blob in a certain internal format, and to extract individual ranking signals from that blob, you need to use either of the two `sphinx_factors_XXX()` or `sphinx_get_YYY_factor()` families of functions.

### sphinx_factors_XXX() functions

This family consists of 3 functions.

* `sphinx_factors_init()` initializes the unpacked `SPH_UDF_FACTORS` structure 
* `sphinx_factors_unpack()` unpacks a binary blob into `SPH_UDF_FACTORS` structure
* `sphinx_factors_deinit()` cleans up an deallocates the `SPH_UDF_FACTORS`.

First you need to call `init()` and `unpack()`, then you can use the `SPH_UDF_FACTORS` fields, and finally you need to cleanup with `deinit()`.

That is simple, but results in a bunch of memory allocations per each processed document, and might be slow.

### sphinx_get_YYY_factor() functions

The other interface, consisting of a bunch of `sphinx_get_YYY_factor()` functions, is a little more wordy to use, but accesses the blob data directly and guarantees that there will be zero allocations. So for top-notch ranking UDF performance, you want to use that one.

## Return types of UDF

As for the return types, UDFs can currently return a single INTEGER, BIGINT, FLOAT, or STRING value. The C function return type should be `sphinx_int64_t`, `sphinx_int64_t`, `double`, or `char*` respectively. In the last case you **must** use `args->fn_malloc` function to allocate space for returned string values. Internally in your UDF you can use whatever you want, so the `testfunc_init()` example above is correct code even though it uses malloc() directly: you manage that pointer yourself, it gets freed up using a matching free() call, and all is well. However, the returned strings values are managed by Manticore and we have our own allocator, so for the return values specifically, you need to use it too.

Depending on how your UDFs are used in the query, the main function call (`testfunc()` in our example) might be called in a rather different volume and order. Specifically,

* UDFs referenced in WHERE, ORDER BY, or GROUP BY clauses must and will be evaluated for every matched document. They will be called in the natural matching order.
* without subselects, UDFs that can be evaluated at the very last stage over the final result set will be evaluated that way, but  before applying the `LIMIT` clause. They will be called in the result set order.
* with subselects, such UDFs will also be evaluated after applying the inner `LIMIT` clause.

The calling sequence of the other functions is fixed, though. Namely,

* `testfunc_init()` is called once when initializing the query. It can return a non-zero code to indicate a failure; in that case query will be terminated, and the error message from the `error_message` buffer will be returned.
* `testfunc()` is called for every eligible row (see above), whenever Manticore needs to compute the UDF value. It can also indicate an (internal) failure error by writing a non-zero byte value to `error_flag`. In that case, it is guaranteed that will no more be called for subsequent rows, and a default return value of 0 will be substituted. Manticore might or might not choose to terminate such queries early, neither behavior is currently guaranteed.
* `testfunc_deinit()` is called once when the query processing (in a given index shard) ends.