# Testing

This is a quick reference to the automated test suites included in Manticore Search for developers to make sure that new changes are stable, free of bugs and don't bring new regressions.
Manticore Search code comes with a test suite which must be run prior to submitting pull request. You will need a MySQL instance to perform all tests.

## Requirements

* Make sure you follow all the requirements needed for compiling Manticore Search, see https://manual.manticoresearch.com/Installation/Compiling_from_sources

* PHP (with mysql,curl,xml,dompdf) and Python

* MySQL (or derivate). By default, the tests will try use in your local MySQL a database `test` with  a no-password user `test`

To overwrite the default settings, create a `.sphinx` file in the home user directory:

```
(
    "db-host"=>"127.0.0.1",
    "db-user"=>"myuser",
    "db-name"=>"mydb",
    "db-password"=>"mypass"
    "lemmatizer_base"=>"path_to_aot_packs"
)
```

## Running Tests

The typical flow is to checkout branch which will be used as pull request and issue these commands:

```
$ mkdir build && cd build
$ cmake ..
$ make -j8 
$ ctest -C Debug
```

To run a specific test there is either regexp option

```
$ cd build
$ ctest -C Debug -R test_101
```

or tests range option

```
$ cd build
$ ctest -C Debug -I 1,10,1
```

If you are fixing a regression you must add or update a functional test which failed prior to the pull request got merged and passed fine after the pull request got merged.


