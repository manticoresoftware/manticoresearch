.. _tsvcsv_data_source:

TSV/CSV data source
----------------------------------------------------------------------------------

This is the simplest way to pass data to the indexer. It was created due
to xmlpipe2 limitations. Namely, indexer must map each attribute and
field tag in XML file to corresponding schema element. This mapping
requires some time. And time increases with increasing the number of
fields and attributes in schema. There is no such issue in tsvpipe
because each field and attribute is a particular column in TSV file. So,
in some cases tsvpipe could work slightly faster than xmlpipe2.

The first column in TSV\CSV file must be a document ID. The
rest ones must mirror the declaration of fields and attributes in schema
definition.

The difference between tsvpipe and csvpipe is delimiter and quoting
rules. tsvpipe has tab character as hardcoded delimiter and has no
quoting rules. csvpipe has option
:ref:`csvpipe_delimiter <csvpipe_delimiter>`
for delimiter with default value ‘,’ and also has quoting rules, such
as:

-  any field may be quoted

-  fields containing a line-break, double-quote or commas should be
   quoted

-  a double quote character in a field must be represented by two double
   quote characters

tsvpipe and csvpipe have same field and attrribute declaration
derectives as xmlpipe.

tsvpipe declarations:

:ref:`tsvpipe_command <xmlpipe_command>`,
:ref:`tsvpipe_field <xmlpipe_field>`,
:ref:`tsvpipe_field_string <xmlpipe_field_string>`,
:ref:`tsvpipe_attr_uint <xmlpipe_attr_uint>`,
:ref:`tsvpipe_attr_timestamp <xmlpipe_attr_timestamp>`,
:ref:`tsvpipe_attr_bool <xmlpipe_attr_bool>`,
:ref:`tsvpipe_attr_float <xmlpipe_attr_float>`,
:ref:`tsvpipe_attr_bigint <xmlpipe_attr_bigint>`,
:ref:`tsvpipe_attr_multi <xmlpipe_attr_multi>`,
:ref:`tsvpipe_attr_multi_64 <xmlpipe_attr_multi_64>`,
:ref:`tsvpipe_attr_string <xmlpipe_attr_string>`,
:ref:`tsvpipe_attr_json <xmlpipe_attr_json>`

csvpipe declarations:

:ref:`csvpipe_command <xmlpipe_command>`,
:ref:`csvpipe_field <xmlpipe_field>`,
:ref:`csvpipe_field_string <xmlpipe_field_string>`,
:ref:`csvpipe_attr_uint <xmlpipe_attr_uint>`,
:ref:`csvpipe_attr_timestamp <xmlpipe_attr_timestamp>`,
:ref:`csvpipe_attr_bool <xmlpipe_attr_bool>`,
:ref:`csvpipe_attr_float <xmlpipe_attr_float>`,
:ref:`csvpipe_attr_bigint <xmlpipe_attr_bigint>`,
:ref:`csvpipe_attr_multi <xmlpipe_attr_multi>`,
:ref:`csvpipe_attr_multi_64 <xmlpipe_attr_multi_64>`,
:ref:`csvpipe_attr_string <xmlpipe_attr_string>`,
:ref:`csvpipe_attr_json <xmlpipe_attr_json>`

::


    source tsv_test
    {
        type = tsvpipe
        tsvpipe_command = cat /tmp/rock_bands.tsv
        tsvpipe_field = name
        tsvpipe_attr_multi = genre_tags
    }

::


    1   Led Zeppelin    35,23,16
    2   Deep Purple 35,92
    3   Frank Zappa 35,23,16,92,33,24

