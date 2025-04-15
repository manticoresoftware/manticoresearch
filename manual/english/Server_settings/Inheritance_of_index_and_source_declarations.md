#Inheritance of index and source declarations

Inheritance in `index` and `source` declarations enables better organization of tables with similar settings or structures and reduces the configuration size. Both parent and child tables or sources can utilize inheritance.

No specific configurations are needed for a parent table or source.

In the child table or source declaration, specify the table or source name followed by a colon (:) and the parent name:

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

The child will inherit the entire configuration of the parent. Any settings declared in the child will overwrite the inherited values. Be aware that for multi-value settings, defining a single value in the child will clear all inherited values. For example, if the parent has several `sql_query_pre` declarations and the child has a single `sql_query_pre` declaration, all inherited `sql_query_pre` declarations are cleared. To override some of the inherited values from the parent, explicitly declare them in the child. This is also applicable if you don't need a value from the parent. For example, if the `sql_query_pre` value from the parent is not needed, declare the directive with an empty value in the child like `sql_query_pre=`. 

Note that existing values of a multi-value setting will not be copied if the child declares one value for that setting.

The inheritance behavior applies to fields and attributes, not just table options. For example, if the parent has two integer attributes and the child needs a new integer attribute, the integer attribute declarations from the parent must be copied into the child configuration.
<!-- proofread -->