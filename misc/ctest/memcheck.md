## Memcheck with Valgrind

## Quick Start on Linux

```bash
cd /manticore/source/dir
bash memdocker.sh
# continue in the started docker shell
bash memcheck.sh
```

Note that in the development environment, it is also acceptable to run memcheck directly on the host (without Docker). Usually, it is about 10% faster than the dockerized check.

```bash
cd /manticore/source/dir
bash memcheck.sh
```

It is recommended to clean the `/manticore/source/dir/build` folder. This may not be necessary in CI (as it usually starts with clean folders), but it is very desirable in the case of a development machine.

After running, you may notice the text near the end of the output:
```
MemCheck log files can be found here: (<#> corresponds to test number)
/manticore/source/dir/build/Testing/Temporary/MemoryChecker.<#>.log
```

## Details

### Toolset

For a dockerized run, the Dockerfile and other files are located in the `misc/ctest/valgrind` folder. Build instructions are located at the bottom of the Dockerfile.

For a native run, you can use the Dockerfile as a reference. Assume you already have all the necessary tools to build and test Manticore on your machine. To perform memchecking, you need, in addition, Valgrind (and its headers). You also need (on Ubuntu) the package `libc6-dbg`, which is most likely already present.

For Valgrind, we need to build with debug information. This is perfectly achieved by using the 'RelWithDebInfo' configuration. The Clang-16 compiler produces debug info in DWARF v5 format. Unfortunately, this is too modern for the Valgrind provided in the Ubuntu Jammy repository. To deal with it, we can either ask the compiler to produce a more ancient version of debug information or use a more recent version of Valgrind that can understand DWARF v5. We prefer modern Valgrind; v3.24 is sufficient. In the Docker environment, we achieve this from Linuxbrew (the Linux flavor of Homebrew). However, you may also try the one from alternative sources, such as Snap.

### Execution

The usual way to run a `program` under Valgrind is to run it as `valgrind program` (omitting options for simplicity). The problem is that we run ubertests as `php ubertest ...`, and thus, running `valgrind php ubertest...` will actually memcheck PHP, but we need to memcheck the daemon (searchd). In low mode, when we run `php ubertest...` as our tool, we have special options in the `~/.sphinx` configuration file. You can find one together with the Dockerfile in the `misc/ctest/valgrind` folder. To invoke Valgrind, you should add something like this to the file:

```
'valgrindoptions'=>'--tool=memcheck --leak-check=full --track-origins=yes --time-stamp=yes --show-leak-kinds=all -s',
'valgrindsearchd'=>true
```

This is quite a flexible way, especially if you create new suppressions. In this case, you need to just add `--gen-suppressions=all` to the end of the 'valgrindoptions' line (so it will look like `--tool=memcheck --leak-check=full --track-origins=yes --time-stamp=yes --show-leak-kinds=all -s --gen-suppressions=all`).

If you prefer to run `php ubertest...` manually, to run memcheck, you need to add the mentioned parameters to your `~/.sphinx` file and just run `php ubertest...` as usual. The test suite will see that you want to memcheck the daemon and will always run the daemon under Valgrind.

Another, more automatic way to perform tests is using `ctest` - a tool from the CMake framework. When configuring the project, we investigate all our ubertests (i.e., tests that are run by executing `php ubertest.php t <N>`) and wrap each of these ubertests into `ctest`. Then we run the `ctest` tool, which performs testing by running all these `php ubertest.php t <N>` commands one by one. `Ctest` can also perform a memcheck pass. However, it knows nothing about the content of the test and will also try to run `valgrind php ubertest.php ...` instead of memchecking searchd. In this case, we can't easily override it because even if we add some lines to the `~/.sphinx` config, `ctest` will still run each test as `valgrind php ubertest.php ...`.

To deal with `ctest`, we have a special wrapper script called 'valgrind' in the root of the sources. We make this script executable and then run `ctest` with the memcheck pass. `Ctest` will search for the memcheck tool and will find our `valgrind` script good enough. Then it will run `valgrind php ubertest.php ...` for memchecking, but `valgrind` here will be our script instead of the actual system Valgrind binary. Using it, we separate the test run from the memcheck parameters. We then provide the passed parameters to our test suite as environment variables, and it will see that the daemon should be run under Valgrind. This achieves our goal - we run the 'memcheck' pass of `ctest`, and it actually tests the daemon (not PHP or something else).

### Stack Probes

During runtime, the daemon parses queries, recognizes expressions, builds an evaluation tree, and then executes it. The evaluation tree may cause quite deep CPU stack usage. For example, if we have a full-text expression like '(a b "a b") | (a -b)', it will need one stack, but '( a a a a a .... a a a a b "a b" | (a -b)' will need a much bigger one. The same is true for expressions and filters. The size of the expression is a matter of runtime; we can't guess or deal with it when building the daemon. Also, we can't guess how '(a a b)' is bigger than just '(a b)', because the size of the stack frame depends on the compiler, compiler flags, CPU architecture, etc. Knowing these values allows us to safely determine if the requested expression can't fit in the current stack and, therefore, either allocate a bigger stack and evaluate it or reject it with a graceful error message. Without such knowledge, we could either allocate much more than necessary or crash if our guess is wrong and an expression actually caused a stack overflow.

During startup, the daemon performs experiments: it tries to match '(a b)', then '(a a b)', then '(a a a b)', etc. For each experiment, it prepares a special arena of memory filled with a known pattern; then it runs the calculation in a coroutine that uses this arena as the CPU stack. After the calculation, it scans the arena for values different from the pattern and thus guesses the number of bytes that were actually used on the stack to calculate the match. In a similar way, it calculates the stack needed to apply filters, calculate expressions, and even destroy the evaluation tree (yes, we can allocate a huge chain of execution, so that even destroying such a chain may cause a stack overflow because of recursion).

Such probes work seamlessly in the usual runtime, but they cause Valgrind to report tons of defects. This is because, in the usual runtime, the CPU stack is used to keep return addresses for function calls and also to store local variables and arrays. When we perform other actions, like filling the arena with a template and scanning it after the experiment, it looks too suspicious for Valgrind - as if we read function's local variables and addresses after exit. Together with the number of such 'stack probe' experiments (which can be about 200 to 500), it also makes starting the daemon under Valgrind significantly slower.

To deal with Valgrind, we extracted the daemon's stack probes into a separate run. After the build, we just run `searchd --mockstack`. This causes the daemon to perform all stack probe experiments, then print the calculated values to the console and exit (it works even without a configuration file).

```bash
$ src/searchd --mockstack
Manticore 7.0.1 DEADBEEF@220512 devmode
Copyright (c) 2001-2016, Andrew Aksyonoff
Copyright (c) 2008-2016, Sphinx Technologies Inc (http://sphinxsearch.com)
Copyright (c) 2017-2024, Manticore Software LTD (https://manticoresearch.com)

export MANTICORE_KNOWN_CREATE_SIZE=200
export MANTICORE_START_KNOWN_CREATE_SIZE=4568
export MANTICORE_KNOWN_EXPR_SIZE=16
export MANTICORE_START_KNOWN_EXPR_SIZE=200
export MANTICORE_NONE=32
export MANTICORE_START_NONE=104
export MANTICORE_KNOWN_FILTER_SIZE=224
export MANTICORE_START_KNOWN_FILTER_SIZE=11168
export MANTICORE_KNOWN_MATCH_SIZE=320
export MANTICORE_START_KNOWN_MATCH_SIZE=14568
export NO_STACK_CALCULATION=1
```

We need only the 'export...' lines. In the script, it is done this way:
```bash
...
src/searchd --mockstack | tail -n +6 > stack.txt
source stack.txt
...
```
That is, we take these lines, store them into `stack.txt`, and then inject them directly into the current context. At the start, the daemon will see the environment variable `NO_STACK_CALCULATION=1`. This means it should NOT perform any experiments. Then it adopts all the values from the `MANTICORE_KNOWN.../MANTICORE_START...` environments and learns the necessary stack frame values from them. Finally, it causes the daemon to know all the correct values of the stack and also not perform any experiments. This is perfect for memcheck, as now it has no suspicious operations to analyze underneath.

### Results

In the case of manually running `php ubertests.php ...` with enabled Valgrind (in the `.sphinx` config), the result will be printed in the console. You can redirect it into a file and process it in some way, but such manual running is intended mostly to investigate one concrete test for memory leaks or to generate a suppression. This is not intended to be used in automation, so you don't need to waste time on it.

In the case of memcheck with `ctest`, results are stored as a bunch of files in the build folder - 'build/Testing/Temporary/MemoryChecker.<#>.log'. Each test produces one file, numbered according to the test. Note that the files are not cleared before running the memcheck; we handle this ourselves by running `rm -f Testing/Temporary/MemoryChecker.*` before executing the memcheck pass.

Ideally, when absolutely no defects are detected, memcheck produces a zero-sized (i.e., empty) report. It is expected that all the reports will be empty. However, achieving such a goal is quite complex, so we ignore some 'well-known' or really minor defects.

#### 'Possibly Lost' Result

```valgrind
==17529== 304 bytes in 1 blocks are possibly lost in loss record 12 of 13
==17529==    at 0x4850D23: calloc (in /home/linuxbrew/.linuxbrew/Cellar/valgrind/3.24.0/libexec/valgrind/vgpreload_memcheck-amd64-linux.so)
==17529==    by 0x40147D9: calloc (rtld-malloc.h:44)
==17529==    by 0x40147D9: allocate_dtv (dl-tls.c:375)
==17529==    by 0x40147D9: _dl_allocate_tls (dl-tls.c:634)
==17529==    by 0x4C4C7B4: allocate_stack (allocatestack.c:430)
==17529==    by 0x4C4C7B4: pthread_create@@GLIBC_2.34 (pthread_create.c:647)
==17529==    by 0xF18EFF: Threads::Create(unsigned long*, std::function<void ()>, bool, char const*, int) (src/threadutils.cpp:1687)
==17529==    by 0xF48E4E: TinyTimer_c::TinyTimer_c() (src/mini_timer.cpp:192)
...
```
This is a quite rare minor report. Such reports occur when, at the end of the program, some memory is still allocated, but Valgrind is not sure about it. This happens when we store a pointer in some way. For example, for threads, we store a pointer to the 'top of my stack'. That is not exactly a pointer to the stack, but to the first local variable. If we finish with such a pointer, Valgrind will see that it points somewhere near the start of the stack, but not exactly to the start. So, it will report it as 'possibly lost'.

#### 'Still Reachable' Result

This is also minor (for us). This may happen, for example, if we allocate a string and then invoke the system 'terminate' call. In the usual case, the string would be deallocated when it goes out of scope, or (if it is static) - on the program's exit callback. However, 'terminate' gives no chance for cleanup to happen, so Valgrind sees the string and also sees that it is used.

#### 'Indirectly Lost' Result

```valgrind
==1534868== 48 bytes in 1 blocks are indirectly lost in loss record 10 of 23
==1534868==    at 0x4849FD3: operator new(unsigned long) (in /home/linuxbrew/.linuxbrew/Cellar/valgrind/3.24.0/libexec/valgrind/vgpreload_memcheck-amd64-linux.so)
==1534868==    by 0xAC55C2: _M_create<(lambda at /home/alexey/sphinxfrommac/src/searchdtask.cpp:109:35)> (std_function.h:161)
==1534868==    by 0xAC55C2: _M_init_functor<(lambda at /home/alexey/sphinxfrommac/src/searchdtask.cpp:109:35)> (std_function.h:215)
==1534868==    by 0xAC55C2: function<(lambda at /home/alexey/sphinxfrommac/src/searchdtask.cpp:109:35), void> (std_function.h:449)
==1534868==    by 0xAC55C2: TaskManager::ScheduleJob(int, long, std::function<void ()>) (src/searchdtask.cpp:109)
==1534868==    by 0xBB3A2D: ScheduleRtFlushDisk() (src/taskflushdisk.cpp:47)
```

This is also a minor report by itself, but it is always followed by a major 'definitely lost' result.

#### 'Definitely Lost' Result

```valgrind
==1534868== 104 (56 direct, 48 indirect) bytes in 1 blocks are definitely lost in loss record 15 of 23
==1534868==    at 0x4849FD3: operator new(unsigned long) (in /home/linuxbrew/.linuxbrew/Cellar/valgrind/3.24.0/libexec/valgrind/vgpreload_memcheck-amd64-linux.so)
==1534868==    by 0xAC552F: TaskManager::ScheduleJob(int, long, std::function<void ()>) (src/searchdtask.cpp:108)
==1534868==    by 0xBB3A2D: ScheduleRtFlushDisk() (src/taskflushdisk.cpp:47)
```

This indicates a true memory leak. That is, memory was allocated, and then the pointer to it was lost. Therefore, the program can't access this memory anymore. It is lost forever.

When we deal with the 'definitely lost' condition, it is good to look into it and decide how critical it is. Some true memory leaks may not be critical in general.

For example, one-shot leaks happen on startup/shutdown/whatever. If we allocate 100 bytes (or even megabytes) and forget about them - well, it will be a memleak of 100 bytes or 100 megabytes. It will persist during runtime, and it will be detected on shutdown by Valgrind, but if it is stable, it will not cause the daemon's emergency termination because of OOM.

Another example is if these leaks happen periodically but involve small pieces of memory. For instance, if we lose exactly 10 bytes for each query, then with a significant load of 1000 QPS, we will lose 10 KB each second, or 36 MB each hour, or about 800 MB per day. If you have 64 GB of RAM on the host, it will take about 80 days before OOM will kill the daemon. That is not good, but also not critically urgent.

The most critical case is large leaks. For example, if you receive a query, parse it, execute it, and... forget to finally delete it. In this case, the size of the leak is equal to your incoming network traffic, and it will cause OOM quite often. That is a serious defect to fix.

#### 'Invalid Read' / 'Invalid Write'

This is not a memory leak, but it is worse. This means that the program tries to read or write to an address it doesn't own, i.e., garbage. Sometimes that will just produce unpredictable results; in other cases, it may cause a crash. For example, if we calculate a hash of a structure in memory, we can take the address of the beginning and the size, and then just call the hash function to process such a 'blob'. Most probably, the structure has padding bytes between members, and the value of those bytes is undefined. If it is real garbage, hashing will produce different results each time, which is most likely not the expected behavior.

All the kinds of errors and their meanings are available in the official documentation:

https://valgrind.org/docs/manual/mc-manual.html#mc-manual.errormsgs

### Suppressions

Ideally, reports from Valgrind should be empty. It is acceptable to have around 30 to 50 non-empty reports with minor well-known defects. However, sometimes it happens that new code always produces a report.

The last example is the usage of the `cctz` library, which, for some (unknown) reason, is invoked early in runtime and initializes its internal structures before the daemon forks to the watchdog and worker. When one of the children finishes by invoking the system exit() function, these initialized structures become 'still reachable' and are reported by Valgrind.

When _each_ run of a test produces a non-zero minor report, it is reasonable to produce a suppression rule for Valgrind to ignore such defects. You can refer to the 'execution' section above to achieve this. The usual workflow is to add `--gen-suppressions=all` directive to the `~/.sphinx` file, then run `php ubertest.php ...` manually with the test, grab the suggested suppression rule from Valgrind's output, and simplify it (delete non-important frames, and possibly add wildcards). The final rule needs to be added to the `test/valgrind.supp` file. Our test suite knows about this file and will provide it to Valgrind during memchecks.

## Automation Ideas

The provided `memdocker.sh` script is just a way to run a shell in the Docker. That could be transferred to any CI (the script itself is just a reference).

The provided `memcheck.sh` can also be used in CI. It has some preparations, but the nutshell is here:

```bash
export NO_TESTS=1
ctest -S ../misc/ctest/memcheck.cmake
src/searchd --mockstack | tail -n +6 > stack.txt
source stack.txt
export NO_TESTS=0
ctest -S ../misc/ctest/memcheck.cmake
```

That is, the first run of `ctest` with `NO_TESTS=1` will just build the daemon in the desired configuration. Then, calling the daemon with `--mockstack` will prepare the environment for further runs under Valgrind without noisy reports from stack mocking. Finally, the memcheck itself is performed by the same call of `ctest`, but now with `NO_TESTS=0` and with the prepared stack measures applied in the environment.

Generally, we have no resources to manually process Valgrind reports because any non-zero report implies using our brains to understand whether the report is important or not. So, if we automate memcheck to pass it nightly, it will produce hundreds of non-zero reports every week, which are useless without human-eye analysis.

However, some reports may be automatically detected and produce warnings:
* invalid
* uninitialized
* definitely lost

If any of these are detected, it most probably means we have encountered a serious defect. The words should be scanned in the lines as below:

```valgrind
==24721== 104 (56 direct, 48 indirect) bytes in 1 blocks are definitely lost in loss record 10 of 14
==24721==    at 0x4849FD3: operator new(unsigned long) (in /home/linuxbrew/.linuxbrew/Cellar/valgrind/3.24.0/libexec/valgrind/vgpreload_memcheck-amd64-linux.so)
==24721==    by 0xAA7EAF: TaskManager::ScheduleJob(int, long, std::function<void ()>) (src/searchdtask.cpp:108)
==24721==    by 0xB93D8D: ScheduleRtFlushDisk() (src/taskflushdisk.cpp:47)
```

Here we see 'definitely lost' - so that is a problem, and any automation should report and warn about it. All the other defects for now should be omitted.

The number in the name of a report is the number of the test. So, if we see a non-empty report 'MemoryChecker.640.log', it corresponds to test 640, which was 
```
640/720 MemCheck #640: test_411 -- query vs stack overflow Windows ................................................................   Passed   11.25 sec
```
Since the number of the test depends on the run, it would be good to keep and provide this line "test_411 -- query vs stack overflow Windows". With this info, the developer can then immediately investigate ubertest 411 without any extra guesses or questions.