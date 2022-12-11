# Miscellaneous tools

## indextool

`indextool` is a helper tool used to dump miscellaneous information about a physical table (not `template` or `distributed`). The general usage is:

```sql
indextool <command> [options]
```

Options effective for all commands:

*   `--config <file>` (`-c <file>` for short) overrides the built-in config file names.
*   `--quiet` (`-q` for short) keep indextool quiet - it will not output banner, etc.
*   `--help` (`-h` for short) lists all of the parameters that can be called in your particular build of `indextool`.
*   `-v` show version information of your particular build of `indextool`.

The commands are as follows:

*   `--checkconfig` just loads and verifies the config file to check if it's valid, without syntax errors.
*   `--buildidf DICTFILE1 [DICTFILE2 ...] --out IDFILE` build IDF file from one or several dictionary dumps. Additional parameter `--skip-uniq` will skip unique (df=1) words.
*   `--build-infixes INDEXNAME` build infixes for an existing dict=keywords table (upgrades .sph, .spi in place). You can use this option for legacy table files that already use dict=keywords, but now need to support infix searching too; updating the table files with indextool may prove easier or faster than regenerating them from scratch with indexer.
*   `--dumpheader FILENAME.sph` quickly dumps the provided table header file without touching any other table files or even the configuration file. The report provides a breakdown of all the table settings, in particular the entire attribute and field list.
*   `--dumpconfig FILENAME.sph` dumps the table definition from the given table header file in (almost) compliant `sphinx.conf` file format.
*   `--dumpheader INDEXNAME` dumps table header by table name with looking up the header path in the configuration file.
*   `--dumpdict INDEXNAME` dumps dictionary. Additional `-stats` switch will dump to dictionary the total number of documents. It is required for dictionary files that are used for creation of IDF files.
*   `--dumpdocids INDEXNAME` dumps document IDs by table name.
*   `--dumphitlist INDEXNAME KEYWORD` dumps all the hits (occurrences) of a given keyword in a given table, with keyword specified as text.
*   `--dumphitlist INDEXNAME --wordid ID` dumps all the hits (occurrences) of a given keyword in a given table, with keyword specified as internal numeric ID.
*   `--docextract TBL DOCID` runs usual table check pass of whole dictionary/docs/hits, and collects all the words and hits belonging to requested document. Then all of the words are placed in the order according to their fields and positions, and result is printed, grouping by field.
*   `--fold INDEXNAME OPTFILE` This options is useful too see how actually tokenizer proceeds input. You can feed indextool with text from file if specified or from stdin otherwise. The output will contain spaces instead of separators (accordingly to your `charset_table` settings) and lowercased letters in words.
*   `--htmlstrip INDEXNAME` filters stdin using HTML stripper settings for a given table, and prints the filtering results to stdout. Note that the settings will be taken from sphinx.conf, and not the table header.
*   `--mergeidf NODE1.idf [NODE2.idf ...] --out GLOBAL.idf` merge several .idf files into a single one. Additional parameter `--skip-uniq` will skip unique (df=1) words.
*   `--morph INDEXNAME` applies morphology to the given stdin and prints the result to stdout.
*   `--check INDEXNAME` checks the table data files for consistency errors that might be introduced either by bugs in `indexer` and/or hardware faults. `--check` also works on RT tables, RAM and disk chunks. Additional options:
    - `--check-id-dups` checks if there are duplicate ids
    - `--check-disk-chunk CHUNK_NAME` checks only specific disk chunk of an RT table. The argument is a disk chunk numeric extension of the RT table to check.
*   `--strip-path` strips the path names from all the file names referenced from the table (stopwords, wordforms, exceptions, etc). This is useful for checking tables built on another machine with possibly different path layouts.
*   `--rotate` works only with `--check` and defines whether to check table waiting for rotation, i.e. with .new extension. This is useful when you want to check your table before actually using it.
*   `--apply-killlists` loads and applies kill-lists for all tables listed in the config file. Changes are saved in .SPM files. Kill-list files (.SPK) are deleted. This can be useful if you want to move applying tables from server startup to indexing stage.

## spelldump

`spelldump` is used to extract contents of a dictionary file that uses `ispell` or `MySpell` format, which can help build word lists for *wordforms* - all of the possible forms are pre-built for you.

The general usage is:

```bash
spelldump [options] <dictionary> <affix> [result] [locale-name]
```

The two main parameters are the dictionary's main file and its affix file; usually these are named as `[language-prefix].dict` and `[language-prefix].aff` and will be available with most common Linux distributions, as well as various places online.

`[result]` specifies where the dictionary data should be output to, and `[locale-name]` additionally specifies the locale details you wish to use.

There is an additional option, `-c [file]`, which specifies a file for case conversion details.

Examples of its usage are:

```bash
spelldump en.dict en.aff
spelldump ru.dict ru.aff ru.txt ru_RU.CP1251
spelldump ru.dict ru.aff ru.txt .1251
```

The results file will contain a list of all the words in the dictionary in alphabetical order, output in the format of a wordforms file, which you can use to customize for your specific circumstances. An example of the result file:

```bash
zone > zone
zoned > zoned
zoning > zoning
```

## wordbreaker

`wordbreaker` is used to split compound words, as usual in URLs, into its component words. For example, this tool can split "lordoftherings" into its four component words, or `http://manofsteel.warnerbros.com` into "man of steel warner bros". This helps searching, without requiring prefixes or infixes: searching for "sphinx" wouldn't match "sphinxsearch" but if you break the compound word and index the separate components, you'll get a match without the costs of prefix and infix larger full-text index files.

Examples of its usage are:

```bash
echo manofsteel | bin/wordbreaker -dict dict.txt split
man of steel
```

The input stream will be separated in words using the `-dict` dictionary file. In no dictionary specified, wordbreaker looks in the working folder for a wordbreaker-dict.txt file. (The dictionary should match the language of the compound word.) The `split` command breaks words from the standard input, and outputs the result in the standard output. There are also `test` and `bench` commands that let you test the splitting quality and benchmark the splitting functionality.

Wordbreaker needs a dictionary to recognize individual substrings within a string. To differentiate between different guesses, it uses the relative frequency of each word in the dictionary: higher frequency means higher split probability. You can generate such a file using the `indexer` tool:

```bash
indexer --buildstops dict.txt 100000 --buildfreqs myindex -c /path/to/sphinx.conf
```

which will write the 100,000 most frequent words, along with their counts, from myindex into dict.txt. The output file is a text file, so you can edit it by hand, if need be, to add or remove words.
