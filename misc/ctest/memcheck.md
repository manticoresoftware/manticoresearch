## Memcheck with valgrind

## Quick start on a linux

```bash
cd /manticore/source/dir
bash memdocker.sh
# continue in started docker shell
bash memcheck.sh
```

Notice, on dev environment that is also ok to run memcheck directly on the host (without docker). Usually it is about 10% faster then dockerized check.

```bash
cd /manticore/source/dir
bash memcheck.sh
```

Notice, it is recommended to clean /manticore/source/dir/build folder. That is may be not actual in CI (as it usually starts with clean folders), but very desirable in case of dev machine.

After run, you may notice the text near end of output:
```
MemCheck log files can be found here: (<#> corresponds to test number)
/manticore/source/dir/build/Testing/Temporary/MemoryChecker.<#>.log
```

## Details

### Toolset

For dockerized run, Dockerfile and other staff located in `misc/ctest/valgrind` folder. Build instructions located at the bottom of dockerfile.

For native run, you can use dockerfile as the reference. Assume, you already have all necessary tools to build and test manticore on your machine. To perform memchecking, you need, in addition, valgrind (and it's headers). Also you need (on ubuntu) package `libc6-dbg`, but it most probably already present.

For valgrind we need build with debuginfo. That is perfectly achieved by using 'RelWithDebInfo' config.
Clang-16 compiler produces debug info in dwarf v5 format. Unfortunately, it is too modern for valgrind provided in ubuntu jammy's repo. To deal with it we can either ask compiler to produce more ancient version of debuginfo, or use more actual version of valgrind which can understand dwarf v5. We prefer modern valgrind. v3.24 is enough. In the docker we achieve it from linuxbrew (linux flavour of homebrew). However, you may try the one from alternative sources, for example, snap.

### Execution

Usual way to run a `program` under valgrind is run it as `valgrind program` (omit options for simplicity). The problem is that we run ubertests as `php ubertest ...`, and so, running `valgrind php ubertest...` will actually memcheck php, but we need to memcheck daemon (searchd). In low mode, when we run `php ubertest...` as our tool, we have special options in `~/.sphinx` configuration file. You can find one together with dockerfile in misc/ctest/valgrind folder. In order to invoke valgrind, you should add something like this to the file:

```
'valgrindoptions'=>'--tool=memcheck --leak-check=full --track-origins=yes --time-stamp=yes --show-leak-kinds=all -s',
'valgrindsearchd'=>true
```
That is quite flexible way, especially if you make new suppressions. In this case you need just to add `
--gen-suppressions=all` to the end of 'valgrindoptions' line (so, it will look then like `--tool=memcheck
--leak-check=full --track-origins=yes --time-stamp=yes --show-leak-kinds=all -s --gen-suppressions=all`).

So, if you prefer to run `php ubertest...` manually, to run memcheck you need to add mentioned params to your `~/.sphinx` file, and just run `php ubertest...` as usual. Test suite will see, you want to memcheck the daemon, and will always run daemon under valgrind then.

Another, more automatic way to perform tests is using ctest - tool from cmake framework. When configuring project, we investigate all our ubertests (i.e. tests which is run by executing `php ubertest.php t <N>`), and wrap each of such ubertest into ctest. Then we run `ctest` tool, and it performs testing by run all these `php ubertest.php t <N>` commands one-by-one. Ctest can also perform memcheck pass. But it knows nothing about content of the test, and will also try to run `valgrind php ubertest.php ...` instead of memchecking of searchd. In this case we can't easily override it, because even if we add some lines to `~/.sphinx` config, ctest will still run each test as `valgrind php ubertest.php ...`.

To deal with ctest, we have special wrapper script called 'valgrind' in the root of the sources. We make this script executable, and then run ctest with memcheck pass. Ctest will search for memcheck tool, and will find our `valgrind` script good enough. Then it will run `valgrind php ubertest.php ...` for memchecking, but `valgrind` here will be our script instead of actual system valgrind binary. And using it, we make the magic: we grab all the params passed to our `valgrind` script, and separate test run from memcheck params. Then we provide passed params to our test suite as environment, and it will see, in turn, that daemon should be run under valgrind. It achieves our goal - we run 'memcheck' pass of ctest, and it actually tests daemon (not php or something else).

### Stack probes

During runtime daemon parses queries, recognize expressions, build evaluation tree, and then execute it. Evaluation tree may cause quite deep CPU stack usage. For example, if we have full-text expression like '(a b "a b") | (a -b)', it will need one stack, but '( a a a a a .... a a a a b "a b" | (a -b)' will need much bigger one. Same true for expressions and filters. The size of expression is matter of runtime, we can't guess or deal with it when building the daemon. Also, we can't guess, how '(a a b)' is bigger than just '(a b)', because size of stack frame depends on compiler, compiler flags, CPU architecture, etc. Knowing this values allow us to safely determine, if requested expression can't fit in the current stack, and so, either allocate bigger stack and evaluate it, or reject with graceful error message. Without such knowledge we could either allocate much bigger than necessary, or crash, if our guess is wrong and an expression actually caused current stack overflow.

During startup, daemon performs experiments: it try to match '(a b)', then '(a a b)', then '(a a a b)', etc. For each experiment it prepares special arena of memory, filled by known pattern; then it runs calculation in a coroutine which uses this arena as CPU stack. After calculation, it scans the arena for values different from the pattern, and so, guess the N of bytes which actually was used on the stack to calculate the match. Similar way, it calculates the stack need to apply filters, calculate expressions, and even destroy evaluation tree (yes, we can allocate huge chain of execution, so that even destroying such chain may cause stack overflow because of recursion).

Such probes works seamless in usual runtime, but it causes valgrind to report tons of defects. That is because in usual runtime CPU stack is used to keep return addresses for function calls, and also to store local variables and arrays. When we perform another actions, like fill arena with template, and scan it after experiment, it looks too suspicious for valgrind - as if we read functions local variables and addresses after exit. Together with number of such 'stack probe' experiments (which can be about 200..500), it also make start daemon under valgrind significantly slower.

To deal with valgrind, we extracted daemon's stack probes into separate run. After build, we just run `searchd --mockstack`. It causes daemon to perform all stack probe experiment, then print calculated values to console and exit (it works even without configuration file).

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

We need only 'export...' lines. In the script it is done this way:
```bash
...
src/searchd --mockstack | tail -n +6 > stack.txt
source stack.txt
...
```
That is - we take this lines, store them into stack.txt, and then inject it directly in the current context. On the start, daemon will see env `NO_STACK_CALCULATION=1`. That means, it should NOT perform any experiments. Then it adopts all the values from `MANTICORE_KNOWN.../MANTICORE_START...` environments, and learn necessary stack frame values from them. Finally, it causes daemon to know all correct values of stack, and also not perform any experiments. That is perfect for memcheck, as now it has no suspicious operations to analyze underneath. 

### Results

In case of manual running of `php ubertests.php ...` with enabled valgrind (in .sphinx config), result will be printed in console. You can redirect it into a file and process some way, but such manual running intended mostly to investigate one concrete test for memleaks, or to generate a suppression. That is not intended to be use in automation, so you don't need to waste the time on it.

In case of memcheck with ctest, results are stored as a bunch of files in the build folder - 'build/Testing/Temporary/MemoryChecker.<#>.log'. Each test produced one file, numbered according the test. Notice, that files are not cleared before run memcheck, we make it ourselves by run `rm -f Testing/Temporary/MemoryChecker.*` before executing memcheck pass.

Ideally, when absolutely no defects detected, memcheck produces zero-sized (i.e. empty) report. It is expected all the reports be empty. However, that is quite complex task to achieve such goal, so we ignore some 'well-known' or really minor defects.

#### 'possibly lost' result

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
This is quite seldom minor report. Such reports came when on the end of the program some memory is still allocated, but valgrind is not sure about it. That happens when we store a pointer some way. For example, for threads we store pointer to 'top of my stack'. That is not exactly pointer to the stack, but to the first local variable. If we finish with such pointer, valgrind will see that it points somewhere near the start of the stack, but not exactly to the start. So, it will report it as 'possibly lost'.

#### 'still reachable' result

That is also minor (for us). That may happen, say, if we allocate a string, and then invokes system 'terminate' call. In usual case, string would be deallocated when came out of scope, or (if it is static) - on program's exit callback. But 'terminate' gives no chance to cleanup to happen, so valgrind see the string, and also see, it is used.

#### 'indirectly lost' result

```valgrind
==1534868== 48 bytes in 1 blocks are indirectly lost in loss record 10 of 23
==1534868==    at 0x4849FD3: operator new(unsigned long) (in /home/linuxbrew/.linuxbrew/Cellar/valgrind/3.24.0/libexec/valgrind/vgpreload_memcheck-amd64-linux.so)
==1534868==    by 0xAC55C2: _M_create<(lambda at /home/alexey/sphinxfrommac/src/searchdtask.cpp:109:35)> (std_function.h:161)
==1534868==    by 0xAC55C2: _M_init_functor<(lambda at /home/alexey/sphinxfrommac/src/searchdtask.cpp:109:35)> (std_function.h:215)
==1534868==    by 0xAC55C2: function<(lambda at /home/alexey/sphinxfrommac/src/searchdtask.cpp:109:35), void> (std_function.h:449)
==1534868==    by 0xAC55C2: TaskManager::ScheduleJob(int, long, std::function<void ()>) (src/searchdtask.cpp:109)
==1534868==    by 0xBB3A2D: ScheduleRtFlushDisk() (src/taskflushdisk.cpp:47)
```

That is also minor report by itself, But it always followed by major 'definitely lost' result.

#### 'definitely lost' result

```valgrind
==1534868== 104 (56 direct, 48 indirect) bytes in 1 blocks are definitely lost in loss record 15 of 23
==1534868==    at 0x4849FD3: operator new(unsigned long) (in /home/linuxbrew/.linuxbrew/Cellar/valgrind/3.24.0/libexec/valgrind/vgpreload_memcheck-amd64-linux.so)
==1534868==    by 0xAC552F: TaskManager::ScheduleJob(int, long, std::function<void ()>) (src/searchdtask.cpp:108)
==1534868==    by 0xBB3A2D: ScheduleRtFlushDisk() (src/taskflushdisk.cpp:47)
```

That is true memleak. That is, memory was allocated, and then pointer to it was lost. So, program can't access this memory anymore. It is lost forever. 

When we deal with 'definitely lost' condition, that is good to look into it and decide, how critical is it. Some even true memory leaks may be not critical in general. 

For example, oneshot leaks happen on startup/shutdown/whatever. If we allocate 100 bytes (or even megabytes) and forget about them - well, it will be memleak of 100 bytes of 100 megabytes. It will persist during runtime, it will be detected on shutdown by valgrind, but if it is stable, it will not cause daemon's emergency termination because of OOM.

Another example, if this leaks happens periodically, but shallows small pieces of memory. Say, if for each query we lose exactly 10 bytes. Then with quite big load of 1000qps we will lose 10kb each second. Or, 36mb each hour, or about 800Mb per day. If you have 64G RAM on the host, it will take about 80 days before OOM will kill the daemon. That is not good, but also not emergency critical also.
As the example, leak of 104 bytes happens in ScheduleRtFlushDisk(). If it runs once a second, it will be 8985600 bytes a day, or about 3.3Gb per year. So, it is quite minor also

And most actual - is big leaks. For example, if you receive a query, parse, execute, and... forget to finally delete. This way size of leak is equal to your incoming network traffic, and it will cause OOM quite often. That is serious defect to fix.

#### 'Invalid read' / 'Invalid write'

That is not a memleak, but it is worse. That means, that program tries to read or write to an address it doesn't own, i.e. garbage. Sometimes that will just produce unpredictable result, in other cases it may cause crash. For example, if we calculate a hash of a structure in the memory - we can take address of the beginning, and size, and then just call hash function to process such 'blob'. Most probably structure has padding bytes between members, and value of them is undefined. If it is real garbage, hashing will produce different results each time, and that is most possibly not expected behavior.

All the kinds of errors and their meanings are available in official documentation

https://valgrind.org/docs/manual/mc-manual.html#mc-manual.errormsgs

### Suppressions

Ideally, reports from valgrind are empty. That is ok to have ~30..50 non-empty reports with minor well-known defects. But sometimes it happens, that new code always produce a report.

Last example - usage of cctz library, which by some (unknown) reason invoked early in runtime, and initializes it's internal structures before daemon fork to watchdog and worker. When one of the child finishes by invoking system exit() function, these initialized structures became 'still reachable' and reported by valgrind.

When _each_ run of a test produces non-zero minor report, that is reasonable to produce suppression rule for valgrind, to ignore such defect. You can refer to 'execution' section above to achieve it. Usual workflow is - add `--gen-suppressions=all` directive to `~/.sphinx` file, then run manually `php ubertest.php ...` with the test, then grab suggested suppression rule from valgrind's output, and simplify it (delete non-important frames, m.b. add wildcards). Final rule need to be added to `test/valgrind.supp` file. Our test suite knows about this file, and will provide it to valgrind during memchecks.

## Automation ideas

Provided `memdocker.sh` script is just a way to run shell in the docker. That could be transferred to any CI (script itself is just a reference)

Provided `memcheck.sh` also can be used in CI. It has some preparations, but nutshell is here:

```bash
export NO_TESTS=1
ctest -S ../misc/ctest/memcheck.cmake
src/searchd --mockstack | tail -n +6 > stack.txt
source stack.txt
export NO_TESTS=0
ctest -S ../misc/ctest/memcheck.cmake
```

That is, first run of ctest with NO_TESTS=1 will just build the daemon in desired config.
Then, call daemon with --mockstack will prepare environment for further run under valgrind without noising reports from stack mocking.
Finally, memcheck itselt performed by the same call of ctest, but now with NO_TESTS=0, and with prepared stack measures applied in the environment.

Generally we have no resources to manually process valgrind reports, because any non-zero report implies using brains to understand whether the report is important or not. So, if automate memcheck to pass it nightly, it will produce hundreds of non-zero reports every week which is useless without human-eye analyze.

However, some reports may be automatically detected and produce warnings
* invalid
* uninitialized
* definitely lost

If any of these detected, it most probably means we faced with serious defect.
Words should be scanned in the lines as below:

```valgrind
==24721== 104 (56 direct, 48 indirect) bytes in 1 blocks are definitely lost in loss record 10 of 14
==24721==    at 0x4849FD3: operator new(unsigned long) (in /home/linuxbrew/.linuxbrew/Cellar/valgrind/3.24.0/libexec/valgrind/vgpreload_memcheck-amd64-linux.so)
==24721==    by 0xAA7EAF: TaskManager::ScheduleJob(int, long, std::function<void ()>) (src/searchdtask.cpp:108)
==24721==    by 0xB93D8D: ScheduleRtFlushDisk() (src/taskflushdisk.cpp:47)
```
Here we see 'definitely lost' - so, that is a problem, and any automation should report and warn about it.
All the other defects for now we should omit.

Number in the name of a report is number of test. So, if we see non-empty report 'MemoryChecker.640.log' - it corresponds to test 640, which was 
```
640/720 MemCheck #640: test_411 -- query vs stack overflow Windows ................................................................   Passed   11.25 sec
```
Since N of test depends on run, it would be good also in the case keep and provide this line "test_411 -- query vs stack overflow Windows". With this info, developer can then immediately investigate ubertest 411, without any extra guesses/questions.