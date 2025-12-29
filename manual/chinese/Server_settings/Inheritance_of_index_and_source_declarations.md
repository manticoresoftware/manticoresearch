# index 和 source 声明的继承

`index` 和 `source` 声明中的继承能够更好地组织具有相似设置或结构的表，并减少配置的大小。父表和子表或父 source 和子 source 都可以使用继承。

父表或父 source 不需要特定的配置。

在子表或子 source 声明中，指定表或 source 名称后跟冒号 (:) 和父名称：

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

子表将继承父表的全部配置。子表中声明的任何设置都会覆盖继承的值。请注意，对于多值设置，在子表中定义单个值将清除所有继承的值。例如，如果父表有多个 `sql_query_pre` 声明，而子表中只有一个 `sql_query_pre` 声明，则所有继承的 `sql_query_pre` 声明都会被清除。要覆盖父表继承的某些值，请在子表中显式声明这些值。如果不需要父表的某个值，也可以如此操作。例如，如果不需要父表的 `sql_query_pre` 值，则在子表中将该指令声明为空值，如 `sql_query_pre=`。

请注意，如果子表为多值设置声明了单个值，则不会复制该设置的已有值。

继承行为适用于字段和属性，而不仅仅是表选项。例如，如果父表有两个整数属性，而子表需要一个新的整数属性，则必须将父表的整数属性声明复制到子表配置中。  
<!-- proofread -->

