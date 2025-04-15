# Fetching from TSV,CSV 

TSV/CSV is the simplest way to pass data to the Manticore indexer. This method was created due to the limitations of xmlpipe2. In xmlpipe2, the indexer must map each attribute and field tag in the XML file to a corresponding schema element. This mapping requires time, and it increases with the number of fields and attributes in the schema. TSV/CSV has no such issue, as each field and attribute corresponds to a particular column in the TSV/CSV file. In some cases, TSV/CSV could work slightly faster than xmlpipe2.

## File format
The first column in TSV/CSV file must be a document ID. The rest columns must mirror the declaration of fields and attributes in the schema definition. Note that you don't need to declare the document ID in the schema, since it's always considered to be present, should be in the 1st column and needs to be a unique-signed positive non-zero 64-bit integer.

The difference between tsvpipe and csvpipe is delimiter and quoting rules. tsvpipe has a tab character as hardcoded delimiter and has no quoting rules. csvpipe has the  `csvpipe_delimiter`option for delimiter with a default value of , and also has quoting rules, such as:

* Any field may be quoted
* Fields containing a line-break, double-quote or commas should be quoted
* A double quote character in a field must be represented by two double quote characters

## Declaration of TSV stream
`tsvpipe_command` directive is mandatory and contains the shell command invoked to produce the TSV stream that gets indexed. The command can read a TSV file, but it can also be a program that generates on-the-fly the tab delimited content.

##  TSV indexed columns 

The following directives can be used to declare the types of the indexed columns:

* `tsvpipe_field` -  declares a `text` field. 
* `tsvpipe_field_string` - declares a text field/string attribute. The column will be both indexed as a text field but also stored as a string attribute.
* `tsvpipe_attr_uint` - declares an integer attribute. 
* `tsvpipe_attr_timestamp` - declares a timestamp attribute.
* `tsvpipe_attr_bool` -  declares a boolean attribute.
* `tsvpipe_attr_float` - declares a float attribute.
* `tsvpipe_attr_bigint` - declares a big integer attribute.
* `tsvpipe_attr_multi` - declares a multi-value attribute with integers.
* `tsvpipe_attr_multi_64` - declares a multi-value attribute with 64-bit integers.
* `tsvpipe_attr_string` - declares a string attribute. 
* `tsvpipe_attr_json` - declares a JSON attribute.


Example of a source using a TSV file:

```ini
source tsv_test
{
    type = tsvpipe
    tsvpipe_command = cat /tmp/rock_bands.tsv
    tsvpipe_field = name
    tsvpipe_attr_multi = genre_tags
}
```

```ini
1   Led Zeppelin    35,23,16
2   Deep Purple 35,92
3   Frank Zappa 35,23,16,92,33,24
```

## Declaration of CSV stream
`csvpipe_command` directive is mandatory and contains the shell command invoked to produce the CSV stream which gets indexed. The command can just read a CSV file but it can also be a program that generates on-the-fly the comma delimited content.

##  CSV indexed columns

The following directives can be used to declare the types of the indexed columns:

* `csvpipe_field` -  declares a `text` field. 
* `csvpipe_field_string` - declares a text field/string attribute. The column will be both indexed as a text field but also stored as a string attribute.
* `csvpipe_attr_uint` - declares an integer attribute. 
* `csvpipe_attr_timestamp` - declares a timestamp attribute.
* `csvpipe_attr_bool` -  declares a boolean attribute.
* `csvpipe_attr_float` - declares a float attribute.
* `csvpipe_attr_bigint` - declares a big integer attribute.
* `csvpipe_attr_multi` - declares a multi-value attribute with integers.
* `csvpipe_attr_multi_64` - declares a multi-value attribute with 64-bit integers.
* `csvpipe_attr_string` - declares a string attribute.
* `csvpipe_attr_json` - declares a JSON attribute.

Example of a source using a CSV file:

```ini
source csv_test
{
    type = csvpipe
    csvpipe_command = cat /tmp/rock_bands.csv
    csvpipe_field = name
    csvpipe_attr_multi = genre_tags
}
```

```ini
1,"Led Zeppelin","35,23,16"
2,"Deep Purple","35,92"
3,"Frank Zappa","35,23,16,92,33,24"
```
<!-- proofread -->