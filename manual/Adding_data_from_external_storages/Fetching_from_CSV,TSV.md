# Fetching from TSV,CSV 

This is the simplest way to pass data to the indexer. It was created due to xmlpipe2 limitations. Namely, indexer must map each attribute and field tag in XML file to corresponding schema element. This mapping requires some time. And time increases with increasing the number of fields and attributes in schema. There is no such issue in tsvpipe because each field and attribute is a particular column in TSV file. So, in some cases tsvpipe could work slightly faster than xmlpipe2.

## File format
The first column in TSV/CSV file must be a document ID. The rest ones must mirror the declaration of fields and attributes in schema definition. Note that you don't need to declare the document ID in the schema, since it's always considered to be present, should be in the 1st column and needs to be a unique signed positive non-zero 64-bit integer.

The difference between tsvpipe and csvpipe is delimiter and quoting rules. tsvpipe has tab character as hardcoded delimiter and has no quoting rules. csvpipe has option `csvpipe_delimiter` for delimiter with default value `,` and also has quoting rules, such as:

* any field may be quoted
* fields containing a line-break, double-quote or commas should be quoted
* a double quote character in a field must be represented by two double quote characters


## Declaration of TSV stream
`tsvpipe_command` directive is mandatory and contains the shell command invoked to produce the TSV stream which gets indexed. The command can just read a TSV file but it can also be a program that generates on-the-fly the tab delimited content.

##  TSV indexed columns 

The following directives can be used to declare the types of the indexed columns:

* `tsvpipe_field` -  declares a `text` field. 
* `tsvpipe_field_string` - declares a text field/string attribute. The column will be both indexed as a text field but also stored as a string attribute.
* `tsvpipe_attr_uint` - declares an integer attribute 
* `tsvpipe_attr_timestamp` - declares a timestamp attribute
* `tsvpipe_attr_bool` -  declares a boolean attribute 
* `tsvpipe_attr_float` - declares a float attribute
* `tsvpipe_attr_bigint` - declares a big integer attribute
* `tsvpipe_attr_multi` - declares a multi-value attribute with integers
* `tsvpipe_attr_multi_64` - declares a multi-value attribute with 64bit integers
* `tsvpipe_attr_string` - declares a string attribute 
* `tsvpipe_attr_json` - declares a JSON attribute


Example of a source using a TSV file

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
`csvpipe_command` directive is mandatory and contains the shell command invoked to produce the TSV stream which gets indexed. The command can just read a CSV file but it can also be a program that generates on-the-fly the comma delimited content.

##  CSV indexed columns

The following directives can be used to declare the types of the indexed columns:

* `csvpipe_field` -  declares a `text` field. 
* `csvpipe_field_string` - declares a text field/string attribute. The column will be both indexed as a text field but also stored as a string attribute.
* `csvpipe_attr_uint` - declares an integer attribute 
* `csvpipe_attr_timestamp` - declares a timestamp attribute
* `csvpipe_attr_bool` -  declares a boolean attribute 
* `csvpipe_attr_float` - declares a float attribute
* `csvpipe_attr_bigint` - declares a big integer attribute
* `csvpipe_attr_multi` - declares a multi-value attribute with integers
* `csvpipe_attr_multi_64` - declares a multi-value attribute with 64bit integers
* `csvpipe_attr_string` - declares a string attribute 
* `csvpipe_attr_json` - declares a JSON attribute

Example of a source using a CSV file

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
