# Miscellaneous tools

## indextool

`indextool` is a helpful utility that extracts various information about a physical table, excluding `template` or `distributed` tables. Here's the general syntax for utilizing `indextool`:

```sql
indextool <command> [options]
```

### Options

These options are applicable to all commands:

* `--config <file>` (`-c <file>` for short) lets you override the default configuration file names.
* `--quiet` (`-q` for short) suppresses the output of banners and such by `indextool`.
* `--help` (`-h` for short) displays all parameters available in your specific build of `indextool`.
* `-v` displays the version information of your specific `indextool` build.

### Commands

Here are the available commands:

* `--checkconfig` loads and verifies the config file, checking its validity and for any syntax errors.
* `--buildidf DICTFILE1 [DICTFILE2 ...] --out IDFILE` constructs an IDF file from one or more dictionary dumps (refer to `--dumpdict`). The additional parameter `--skip-uniq` will omit unique words (df=1).
* `--build-infixes TABLENAME` generates infixes for a pre-existing dict=keywords table (updates .sph, .spi in place). Use this option for legacy table files already employing dict=keywords, but now requiring infix search support; updating the table files with indextool may be simpler or quicker than recreating them from scratch with indexer.
* `--dumpheader FILENAME.sph` promptly dumps the given table header file without disturbing any other table files or even the config file. The report offers a detailed view of all the table settings, especially the complete attribute and field list.
* `--dumpconfig FILENAME.sph` extracts the table definition from the specified table header file in an (almost) manticore.conf file-compliant format.
* `--dumpheader TABLENAME` dumps table header by table name while searching for the header path in the config file.
* `--dumpdict TABLENAME` dumps the dictionary. An extra `-stats` switch will add the total document count to the dictionary dump. This is necessary for dictionary files used in IDF file creation.
* `--dumpdocids TABLENAME` dumps document IDs by table name.
* `--dumphitlist TABLENAME KEYWORD` dumps all instances (occurrences) of a specified keyword in a given table, with the keyword defined as text.
* `--dumphitlist TABLENAME --wordid ID` dumps all instances (occurrences) of a specific keyword in a given table, with the keyword represented as an internal numeric ID.
* `--docextract TBL DOCID` executes a standard table check pass of the entire dictionary/docs/hits, and gathers all the words and hits associated with the requested document. Subsequently, all the words are arranged according to their fields and positions, and the result is printed, grouped by field.
* `--fold TABLENAME OPTFILE` This option helps understand how the tokenizer processes input. You can supply the indextool with text from a file, if specified, or from stdin otherwise. The output will replace separators with spaces (based on your `charset_table` settings) and convert letters in words to lowercase.
* `--htmlstrip TABLENAME` applies the HTML stripper settings for a specified table to filter stdin, and sends the filtering results to stdout. Be aware that the settings will be fetched from manticore.conf, and not from the table header.
* `--mergeidf NODE1.idf [NODE2.idf ...] --out GLOBAL.idf` combines multiple .idf files into a single one. The extra parameter `--skip-uniq` will ignore unique words (df=1).
* `--morph TABLENAME` applies morphology to the given stdin and directs the result to stdout.
* `--check TABLENAME` evaluates the table data files for consistency errors that could be caused by bugs in `indexer` or hardware faults. `--check` is also functional on RT tables, RAM, and disk chunks. Additional options:
    - `--check-id-dups` assesses for duplicate ids
    - `--check-disk-chunk CHUNK_NAME` checks only a specific disk chunk of an RT table. The argument is the numeric extension of the RT table's disk chunk to be checked.
* `--strip-path` removes the path names from all file names referred to from the table (stopwords, wordforms, exceptions, etc). This is helpful when verifying tables built on a different machine with possibly varying path layouts.
* `--rotate` is only compatible with `--check` and determines whether to check the table waiting for rotation, i.e., with a .new extension. This is useful when you wish to validate your table before actually putting it into use.
* `--apply-killlists` loads and applies kill-lists for all tables listed in the config file. Changes are saved in .SPM files. Kill-list files (.SPK) are removed. This can be handy if you want to shift the application of tables from server startup to indexing stage.

### Important Consideration for RT Table Checking

`indextool` cannot fully check an RT table that is currently served by the daemon. When attempting to check an active RT table, you may encounter the following warning:

```
WARNING: failed to load RAM chunks, checking only N disk chunks
```

To avoid these warning and ensure a proper check of an RT table, consider the following approaches:

- stop the daemon before running `indextool --check`.  
- ensure that the RT table is not actively served by the daemon.  
- check a separate copy of the RT table instead of the live one.  

If stopping the daemon is not an option, you can prevent unintended modifications to the RT table by executing the following MySQL statement before running `indextool --check`:

```sql
SET GLOBAL AUTO_OPTIMIZE=0;
```

This command prevents the daemon from performing auto-optimization, ensuring that RT table files remain unchanged. After executing this statement, wait until the optimization thread has completely stopped before proceeding with `indextool --check`. This ensures that no disk chunks are unintentionally modified or removed during the check process.
If auto-optimize was previously enabled, you should manually re-enable it after the check is complete by running:

```sql
SET GLOBAL AUTO_OPTIMIZE=1;
```

## spelldump

The `spelldump` command is designed to retrieve the contents from a dictionary file that employs the `ispell` or `MySpell` format. This can be handy when you need to compile word lists for wordforms, as it generates all possible forms for you.

Here's the general syntax:

```bash
spelldump [options] <dictionary> <affix> [result] [locale-name]
```

The primary parameters are the main file and the affix file of the dictionary. Typically, these are named as `[language-prefix].dict` and `[language-prefix].aff`, respectively. You can find these files in most standard Linux distributions or from numerous online sources.

The `[result]` parameter is where the extracted dictionary data will be stored, and `[locale-name]` is the parameter used to specify the locale details of your choice.

There's an optional `-c [file]` option as well. This option allows you to specify a file for case conversion details.

Here are some usage examples:

```bash
spelldump en.dict en.aff
spelldump ru.dict ru.aff ru.txt ru_RU.CP1251
spelldump ru.dict ru.aff ru.txt .1251
```

The resulting file will list all the words from the dictionary, arranged alphabetically and formatted like a wordforms file. You can then modify this file as per your specific requirements. Here's a sample of what the output file might look like:

```bash
zone > zone
zoned > zoned
zoning > zoning
```

## wordbreaker

The `wordbreaker` tool is designed to deconstruct compound words, a common feature in URLs, into their individual components. For instance, it can dissect "lordoftherings" into four separate words or break down `http://manofsteel.warnerbros.com` into "man of steel warner bros". This ability enhances search functionality by eliminating the need for prefixes or infixes. To illustrate, a search for "sphinx" wouldn't yield "sphinxsearch" in the results. However, if you apply `wordbreaker` to disassemble the compound word and index the detached elements, a search will be successful without the file size expansion associated with prefix or infix usage in full-text indexing.

Here are some examples of how to use `wordbreaker`:

```bash
echo manofsteel | bin/wordbreaker -dict dict.txt split
man of steel
```

The `-dict` dictionary file is used to separate the input stream into individual words. If no dictionary file is specified, Wordbreaker will look for a file named `wordbreaker-dict.txt` in the current working directory. (Ensure that the dictionary file matches the language of the compound word you're working with.) The `split` command breaks words from the standard input and sends the results to the standard output. The `test` and `bench` commands are also available to assess the splitting quality and measure the performance of the splitting function, respectively.

Wordbreaker uses a dictionary to identify individual substrings within a given string. To distinguish between multiple potential splits, it considers the relative frequency of each word in the dictionary. A higher frequency indicates a higher likelihood for a word split. To generate a file of this nature, you can use the `indexer` tool:


```bash
indexer --buildstops dict.txt 100000 --buildfreqs myindex -c /path/to/manticore.conf
```

which will produce a text file named `dict.txt` that contains the 100,000 most frequently occurring words from `myindex`, along with their respective counts. Since this output file is a simple text document, you have the flexibility to manually edit it whenever needed. Feel free to add or remove words as required.

<!-- proofread -->
