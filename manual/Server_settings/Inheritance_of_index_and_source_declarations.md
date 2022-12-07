#Inheritance of index and source declarations

Both `index` and `source` declarations support inheritance. It allows a better organization of tables having similar settings or structure and reduces the size of the configuration.

For a parent table/source nothing needs to be specified.

For the child table/source the declaration will contain the table/source name followed by `:` and the parent name.

```ini
table parent {
path = /var/lib/manticore/parent
...
}

table child:parent {
path = /var/lib/manticore/child
...
}
```

The child will inherit the entire configuration of the parent. In the child declaration any setting declared will overwrite the inherited values. Please note that in case of multi-value settings, defining a single value in child will clear out all inherited values. For example in the parent there are several `sql_query_pre` declaration and the child has a single `sql_query_pre` declaration, all the `sql_query_pre` inherited declarations are cleared. If you need to override some of the inherited values from parent, they need to be explicitly declared in the child. This is also available if you don't need a value from parent. For example if the value of `sql_query_pre` from parent is not needed, then in the child you can declare the directive with an empty value like `sql_query_pre=`. This also means that existing values of a multi-value setting will not be copied if the child declares one value for that setting. The inheritance behavior applies to fields and attributes and not just table options. If, for example, the parent has 2 integer attributes and the child needs a new integer attribute, the integer attributes declaration from parent must be copied in the child configuration.