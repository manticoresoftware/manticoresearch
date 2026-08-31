# UDF

UDF 存储在外部动态库中（UNIX 系统上为 .so 文件，Windows 系统上为 .dll 文件）。出于安全考虑，库文件必须放置在由 [plugin_dir](../../Server_settings/Common.md#plugin_dir) 指令指定的受信任文件夹中：保护单个文件夹比允许任何人向 `searchd` 安装任意代码要容易得多。您可以使用 [CREATE FUNCTION](../../Extensions/UDFs_and_Plugins/UDF/Creating_a_function.md) 和 [DROP FUNCTION](../../Extensions/UDFs_and_Plugins/UDF/Deleting_a_function.md) SQL 语句动态加载和卸载 searchd 中的 UDF。此外，您还可以使用 [RELOAD PLUGINS](../../Extensions/UDFs_and_Plugins/Plugins/Reloading_plugins.md) 语句无缝重新加载 UDF（及其他插件）。Manticore 会跟踪当前加载的函数；每次创建或删除 UDF 时，`searchd` 会将其状态更新到 [sphinxql_state](../../Server_settings/Searchd.md#sphinxql_state) 文件中，作为纯 SQL 脚本。

UDF 是本地的。要在集群中使用它们，必须在所有节点上放置相同的库，并在每个节点上运行 CREATE 语句。此流程在未来版本中可能会有所变化。

一旦成功加载 UDF，您可以像使用任何内置函数一样在 SELECT 或其他语句中使用它：

```sql
SELECT id, MYCUSTOMFUNC (groupid, authorname), ... FROM myindex
```

多个 UDF（及其他插件）可以共存在同一个库中。该库只会被加载一次，并在其内所有 UDF 和插件都被删除后自动卸载。

理论上，您可以使用任何语言编写 UDF，只要其编译器能够导入标准 C 头文件并生成带有正确导出函数的标准动态库。然而，使用 C++ 或纯 C 是最少阻力的路径。我们提供了一个用纯 C 编写的示例 UDF 库，实现在多个函数中展示各种技术，源代码位于 [src/udfexample.c](https://github.com/manticoresoftware/manticore/blob/master/src/udfexample.c)。该示例包含头文件 [src/sphinxudf.h](https://github.com/manticoresoftware/manticore/blob/master/src/sphinxudf.h)，其中包含若干 UDF 相关结构和类型的定义。对于大多数 UDF 和插件，只需像示例中那样使用 `#include "sphinxudf.h"` 即可。然而，如果您编写排名函数且需要在 UDF 内访问排名信号（因素）数据，则还需要编译并链接 `src/sphinxudf.c`（可在我们的源码中获得），因为允许您从 UDF 内访问信号数据的函数实现位于该文件。

`sphinxudf.h` 头文件和 `sphinxudf.c` 是独立的，因此您可以单独复制这些文件；它们不依赖 Manticore 源码的其他部分。

在您的 UDF 中，您**必须**只实现并导出几个函数。首先，为了 UDF 接口版本控制，您**必须**定义一个函数 `int LIBRARYNAME_ver()`，其中 LIBRARYNAME 是您库文件的名字，并且此函数必须返回 `SPH_UDF_VERSION`（定义在 `sphinxudf.h` 中）。示例如下。

```c
#include <sphinxudf.h>

// our library will be called udfexample.so, thus, so it must define
// a version function named udfexample_ver()
int udfexample_ver()
{
    return SPH_UDF_VERSION;
}
```

此预防措施可避免您意外加载与 `searchd` 的 UDF 接口版本不匹配的库。不论是较新还是较旧版本。其次，您**必须**实现实际的函数。

```c
sphinx_int64_t testfunc ( SPH_UDF_INIT * init, SPH_UDF_ARGS * args, char * error_flag )
{
    return 123;
}
```

SQL 中的 UDF 函数名不区分大小写。但对应的 C 函数名区分大小写；它们必须全部为小写，否则 UDF 将无法加载。更重要的是，以下几点至关重要：

1. 调用约定必须是 C（即 __cdecl），
2. 参数列表必须完全符合插件系统的期望，
3. 返回类型必须与您在 `CREATE FUNCTION` 中指定的类型匹配。

不幸的是，我们无法（轻松地）在加载函数时检查这些错误，它们可能会导致服务器崩溃和/或产生意外结果。最后但同样重要的是，您实现的所有 C 函数都必须是线程安全的。

第一个参数，是指向 `SPH_UDF_INIT` 结构的指针，本质上是指向我们的函数状态的指针。它是可选的。就如上面示例中，该函数无状态，每次调用都返回 123。因此我们不需要定义初始化函数，可以忽略该参数。
此参数还有另一个目的。由于单个查询可以在多个线程上执行（参见 [pseudo-sharding](../../Server_settings/Searchd.md#pseudo_sharding)），守护进程会通过检查此参数来判断一个 UDF 是有状态还是无状态。如果该参数被初始化，平行执行将被禁用。因此，如果您的 UDF 是有状态的但未使用此参数，则它将由多个线程调用，您的代码需要对此有所意识。

第二个参数，是指向 `SPH_UDF_ARGS` 结构的指针，是最重要的参数。所有实际调用参数都通过此结构传递；它包含调用参数的数量、名称、类型等信息。所以无论函数是以 `SELECT id, testfunc(1)` 还是 `SELECT id, testfunc('abc', 1000*id+gid, WEIGHT())` 或其他语句调用，它接收到的都是同一个 `SPH_UDF_ARGS` 结构。但 `args` 结构中传递的数据会不同。在第一个例子中，`args->arg_count` 被设置为 1；在第二个例子中被设置为 3，`args->arg_types` 数组会包含不同的类型数据，依此类推。

最后，第三个参数是错误标志。UDF 可设置此标志以表明发生某种内部错误，UDF 无法继续执行，查询应提前终止。您**不应该**用此标志进行参数类型检查或用于任何可能在正常使用中发生的错误报告。该标志设计用于报告突然的严重运行时错误，例如内存耗尽。

如果我们想为函数分配临时存储区或预先检查参数是否为支持的类型，则需要添加两个额外函数，分别用于 UDF 初始化和反初始化。

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

注意 `testfunc_init()` 也接收调用参数结构。在调用时，它不会接收任何实际值，因此 `args->arg_values` 将为 NULL。但是，参数名称和类型是已知的，并会被传递。你可以在初始化函数中检查它们，如果它们是不支持的类型，则返回错误。


## SPH_UDF_ARGS 类型

UDFs 可以接收几乎任何有效的内部 Manticore 类型的参数。参见 `sphinx_udf_argtype` 枚举在 `sphinxudf.h` 中的完整列表。大多数类型都直接映射到相应的 C 类型。

最值得注意的类型是 `SPH_UDF_TYPE_FACTORS` 参数类型。当你用 [PACKEDFACTOR()](../../Functions/Searching_and_ranking_functions#PACKEDFACTORS()) 参数调用 UDF 时，你会得到这种类型。它的数据是以某种内部格式的二进制数据块，要从中提取单独的排名信号，你需要使用 `sphinx_factors_XXX()` 或 `sphinx_get_YYY_factor()` 家族中的任何一个函数。

### sphinx_factors_XXX() 函数

这个家族由 3 个函数组成。

* `sphinx_factors_init()` 初始化未打包的 `SPH_UDF_FACTORS` 结构
* `sphinx_factors_unpack()` 将二进制数据块解包到 `SPH_UDF_FACTORS` 结构
* `sphinx_factors_deinit()` 清理并释放 `SPH_UDF_FACTORS`。

首先，你需要调用 `init()` 和 `unpack()`，然后你可以使用 `SPH_UDF_FACTORS` 字段，最后，你需要调用 `deinit()` 进行清理。

这种方法简单但可能会导致每个处理文档都产生大量内存分配，这可能会很慢。

### sphinx_get_YYY_factor() 函数

另一种接口，由多个 `sphinx_get_YYY_factor()` 函数组成，使用起来稍微繁琐一些，但它直接访问数据块，并保证不会进行分配。为了获得顶级排名 UDF 的最佳性能，你将希望使用这种方法。

## UDF 的返回类型

至于返回类型，UDFs 目前可以返回单个 INTEGER、BIGINT、FLOAT 或 STRING 值。C 函数返回类型应分别为 `sphinx_int64_t`、`sphinx_int64_t`、`double` 或 `char*`。在最后一种情况下，你 **必须** 使用 `args->fn_malloc` 函数为返回的字符串值分配空间。在你的 UDF 内部，你可以使用任何你想要的，因此上面的 `testfunc_init()` 示例是正确的代码，即使它直接使用了 malloc()：你自行管理那个指针，它会在匹配的 free() 调用中被释放，一切都很顺利。但是，返回的字符串值由 Manticore 管理，我们有自己的分配器，因此对于返回值，你需要使用它。

根据你的 UDF 在查询中的使用方式，主要函数调用（例如我们示例中的 `testfunc()`）可能会以不同的数量和顺序被调用。具体来说，

* 在 WHERE、ORDER BY 或 GROUP BY 子句中引用的 UDF 必须并且会被为每个匹配的文档评估。它们将以自然匹配顺序被调用。
* 没有子查询时，可以在最终结果集的最后一个阶段评估 UDF，但在应用 `LIMIT` 子句之前。它们将以结果集顺序被调用。
* 有子查询时，这样的 UDF 也会在应用内部的 `LIMIT` 子句之后进行评估。

其他函数的调用顺序是固定的。具体来说，

* `testfunc_init()` 在初始化查询时被调用一次。它可以返回非零代码以指示失败；在这种情况下，查询将终止，并返回 `error_message` 缓冲区中的错误消息。
* `testfunc()` 为每个符合条件的行（见上文）被调用，每当 Manticore 需要计算 UDF 值时。它也可以通过将非零字节值写入 `error_flag` 来指示（内部）失败错误。在这种情况下，它将不会被调用后续行，并将使用默认返回值 0。Manticore 可能或可能不会选择提前终止此类查询；目前没有保证这种行为。
* `testfunc_deinit()` 在查询处理（在给定表分片中）结束时被调用一次。

<!-- proofread -->

