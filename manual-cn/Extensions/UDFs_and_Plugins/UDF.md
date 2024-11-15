# UDF

DF 存储在外部动态库文件中（在 UNIX 系统上是 `.so` 文件，在 Windows 系统上是 `.dll` 文件）。为了安全起见，库文件必须放在通过 [plugin_dir](../../Server_settings/Common.md#plugin_dir) 指令指定的受信文件夹中：这样更容易确保安全，因为只需要保护一个文件夹，而不是允许任意人将任意代码安装到 `searchd` 中。可以使用 [CREATE FUNCTION](../../Extensions/UDFs_and_Plugins/UDF/Creating_a_function.md) 和 [DROP FUNCTION](../../Extensions/UDFs_and_Plugins/UDF/Deleting_a_function.md) SQL 语句分别动态加载和卸载 UDF 到 searchd 中。此外，还可以使用 [RELOAD PLUGINS](../../Extensions/UDFs_and_Plugins/Plugins/Reloading_plugins.md) 语句无缝地重新加载 UDF（以及其他插件）。每次创建或删除 UDF 时，`searchd` 都会更新其在 [sphinxql_state](../../Server_settings/Searchd.md#sphinxql_state) 文件中的状态，该文件是一个普通的 SQL 脚本。

UDF 是本地化的。要在集群中使用 UDF，必须将相同的库放在所有节点上，并在每个节点上运行 CREATE 语句。这一过程可能会在未来版本中有所变化。

成功加载 UDF 后，就可以像使用内置函数一样在 SELECT 或其他语句中使用它：

```sql
SELECT id, MYCUSTOMFUNC (groupid, authorname), ... FROM myindex
```

多个 UDF（以及其他插件）可以共存于一个库中。该库仅会被加载一次，并在其中的所有 UDF 和插件都被删除后自动卸载。

理论上，你可以使用任何语言编写 UDF，只要其编译器可以导入标准 C 头文件并输出具有正确导出函数的标准动态库。然而，使用 C++ 或纯 C 是最简单的途径。我们提供了一个用纯 C 编写的 UDF 库示例，它实现了几种函数（展示了各种技术），该示例可以在我们的源代码中找到，路径为 [src/udfexample.c](https://github.com/manticoresoftware/manticore/blob/master/src/udfexample.c)。该示例包括 [src/sphinxudf.h](https://github.com/manticoresoftware/manticore/blob/master/src/sphinxudf.h) 头文件，其中包含几个与 UDF 相关的结构和类型的定义。对于大多数 UDF 和插件，简单地使用 `#include "sphinxudf.h"` 就足够了。然而，如果你编写的是一个排名函数，并需要从 UDF 内部访问排名信号（因子）数据，还需要使用 `src/sphinxudf.c` 进行编译和链接（在我们的源代码中可以找到），因为用于从 UDF 内部访问信号数据的函数实现位于该文件中。

`sphinxudf.h` 头文件和 `sphinxudf.c` 文件是独立的，因此可以单独复制这些文件；它们不依赖于 Manticore 源代码的其他部分。

在你的 UDF 中，必须实现并导出以下几个函数。首先，为了控制 UDF 接口的版本，必须定义一个函数 `int LIBRARYNAME_ver()`，其中 LIBRARYNAME 是你的库文件名称，并且该函数必须返回 `SPH_UDF_VERSION`（在 `sphinxudf.h` 中定义的值）。以下是一个示例：

```c
#include <sphinxudf.h>

// our library will be called udfexample.so, thus, so it must define
// a version function named udfexample_ver()
int udfexample_ver()
{
    return SPH_UDF_VERSION;
}
```

此预防措施可防止意外将不匹配的 UDF 接口版本的库加载到更新或旧版的 `searchd` 中。其次，必须实现实际的函数。

```c
sphinx_int64_t testfunc ( SPH_UDF_INIT * init, SPH_UDF_ARGS * args, char * error_flag )
{
    return 123;
}
```

SQL 中的 UDF 函数名是大小写不敏感的。然而，C 函数名则不是；它们需要全部为 *小写*，否则 UDF 将无法加载。更重要的是：

1. 调用约定是 C（即 __cdecl），
2. 参数列表必须与插件系统的预期完全匹配，
3. 返回类型必须与 `CREATE FUNCTION` 中指定的类型匹配。

我们无法在加载函数时检查这些错误，且可能导致服务器崩溃或产生意外结果。最后，所有你实现的 C 函数都必须是线程安全的。

第一个参数是指向 `SPH_UDF_INIT` 结构的指针，实际上是指向我们函数状态的指针。它是可选的。在上面的示例中，该函数是无状态的，因为它每次调用时都会返回 123。因此，我们不需要定义初始化函数，并且可以忽略这个参数。

这个参数还有另一个用途。由于单个查询可以在多个线程上执行（参见 [pseudo-sharding](../../Server_settings/Searchd.md#pseudo_sharding)），守护进程会通过检查这个参数来确定 UDF 是有状态还是无状态的。如果这个参数被初始化，查询的并行执行将被禁用。如果你的 UDF 是有状态的，但你没有使用这个参数，它将在多个线程上被调用，因此你的代码需要注意线程安全问题。

第二个参数是指向 `SPH_UDF_ARGS` 结构的指针，这是最重要的参数。所有实际的调用参数都通过这个结构传递给 UDF；它包含了调用的参数数量、名称、类型等信息。因此，无论函数是像 `SELECT id, testfunc(1)` 这样被调用，还是像 `SELECT id, testfunc('abc', 1000*id+gid, WEIGHT())` 这样被调用，它都会接收到相同的 `SPH_UDF_ARGS` 结构。在第一种情况下，`args->arg_count` 会设置为 1，在第二种情况下，它会设置为 3，`args->arg_types` 数组会包含不同的类型数据，依此类推。

最后一个参数是一个错误标志。UDF 可以通过它表示某种内部错误，表明 UDF 无法继续运行，查询应提前终止。你**不应**将其用于参数类型检查或其他可能在正常使用期间发生的错误报告。这个标志是用来报告突发的严重运行时错误，例如内存不足。

如果我们想要为函数分配临时存储空间，或提前检查参数类型是否被支持，则需要再添加两个函数，分别用于 UDF 的初始化和反初始化：

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

注意 `testfunc_init()` 也接收到调用参数结构。在调用时，它不会接收任何实际值，因此 `args->arg_values` 将为 NULL。但参数名称和类型已经确定，并且会被传递。你可以在初始化函数中检查它们，并在类型不支持时返回错误。


## SPH_UDF_ARGS 类型

UDF 可以接收几乎所有有效的 Manticore 内部类型的参数。有关完整的类型列表，请参考 `sphinx_udf_argtype` 枚举。大多数类型可以直接映射到相应的 C 类型。

最显著的类型是 `SPH_UDF_TYPE_FACTORS` 参数类型。通过使用 [PACKEDFACTOR()](../../searching-and-ranking-functions#PACKEDFACTORS()) 参数可以获得这种类型。其数据是一个二进制 blob，使用特定的内部格式存储。要从这个 blob 中提取单独的排名信号，可以使用 `sphinx_factors_XXX()` 或 `sphinx_get_YYY_factor()` 函数族中的任意一个。

### sphinx_factors_XXX() 函数

此函数族包括三个函数：

- `sphinx_factors_init()` 初始化解包的 `SPH_UDF_FACTORS` 结构
- `sphinx_factors_unpack()` 将二进制 blob 解包为 `SPH_UDF_FACTORS` 结构
- `sphinx_factors_deinit()` 清理并释放 `SPH_UDF_FACTORS` 的内存。

首先需要调用 `init()` 和 `unpack()`，然后可以使用 `SPH_UDF_FACTORS` 字段，最后需要使用 `deinit()` 进行清理。

这种方法很简单，但对于每个处理的文档可能会导致大量的内存分配，这可能会影响性能。

### sphinx_get_YYY_factor() 函数

另一种接口包括一组 `sphinx_get_YYY_factor()` 函数，使用这种接口时虽然代码会更冗长，但它直接访问 blob 数据，并且不需要分配内存。如果需要实现高效的排名 UDF，建议使用这种方法。

## UDF 的返回类型

UDF 目前可以返回单个 INTEGER、BIGINT、FLOAT 或 STRING 值。C 函数的返回类型应分别是 `sphinx_int64_t`、`sphinx_int64_t`、`double` 或 `char*`。在最后一种情况下，必须使用 `args->fn_malloc` 函数为返回的字符串值分配空间。虽然在 UDF 内部你可以随意使用任何内存管理方法，例如上面的 `testfunc_init()` 示例代码是正确的，即使它直接使用了 malloc()：你自己管理这个指针，并且通过匹配的 free() 调用来释放它，程序运行正常。然而，返回的字符串值是由 Manticore 管理的，我们有自己的内存分配器，因此你也需要使用它来分配返回值。

根据 UDF 在查询中的使用位置，主函数调用（例如我们的 `testfunc()`）可能会被以不同的频率和顺序调用。

* UDF 如果出现在 WHERE、ORDER BY 或 GROUP BY 子句中，必须且会对每个匹配的文档进行求值，并按照自然匹配顺序调用。
* 如果没有子查询，可以在最后阶段对最终结果集进行求值，但要在应用 `LIMIT` 子句之前完成。这时的调用顺序会按照结果集的顺序。
* 如果有子查询，UDF 会在内部 `LIMIT` 子句应用后进行求值。

其他函数的调用顺序是固定的。具体来说：

- `testfunc_init()` 在查询初始化时调用一次。如果它返回非零代码，则查询会被终止，并返回 `error_message` 中的错误消息。
- `testfunc()` 会在每个符合条件的行（参见上文）上调用。当 Manticore 需要计算 UDF 值时，会调用此函数。如果函数通过 `error_flag` 写入了非零值，则表示出现内部错误。在这种情况下，确保后续行不再调用该函数，并且会替换默认返回值 0。Manticore 可能会选择提前终止查询，但目前这种行为不受保证。
- `testfunc_deinit()` 在查询处理结束时（在给定表分片中）调用一次。

<!-- proofread -->
