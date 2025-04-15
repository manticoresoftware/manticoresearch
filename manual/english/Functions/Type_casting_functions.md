# Type Casting Functions

Type casting comprises three principal actions: conversion, reinterpretation, and promotion.
* **Conversion**. This refers to the process of changing the data type of a value to another data type. This involves additional computations and is exclusively performed by the `TO_STRING()` function.
* **Reinterpretation**. This involves treating the binary data representing a value as if it were of a different data type, without actually changing the underlying data. This is handled by `SINT()`, doesn't involve extra computations; instead, it merely reinterprets existing data.
* **Promotion**. This refers to the process of converting a value to a "larger" or more precise data type. It doesn't require extra computation either; it merely requests the argument to deliver a value of a different type. Only JSON fields and a few other functions can promote their values to integers. If an argument cannot yield a value of a different type, the promotion will fail. For instance, the `TIMEDIFF()` function usually returns a string, but can also return a number. So, `BIGINT(TIMEDIFF(1,2))` will execute successfully, compelling `TIMEDIFF()` to supply an integer value. Conversely, `DATE_FORMAT()` solely returns strings and can't yield a number, meaning that `BIGINT(DATE_FORMAT(...))` will fail.

### BIGINT()

This function promotes an integer argument to a 64-bit type, leaving floating-point arguments untouched. It's designed to ensure the evaluation of specific expressions (such as `a*b`) in 64-bit mode, even if all arguments are 32-bit.

### DOUBLE()

The `DOUBLE()` function promotes its argument to a floating-point type. This is designed to help enforce the evaluation of numeric JSON fields.

### INTEGER()

The `INTEGER()` function promotes its argument to a 64-bit signed type. This is designed to enforce the evaluation of numeric JSON fields.

### TO_STRING()

This function forcefully converts its argument to a string type.

### UINT()

The `UINT()` function promotes its argument to a 32-bit unsigned integer type.

### UINT64()

The `UINT64()` function promotes its argument to a 64-bit unsigned integer type.

### SINT()

The `SINT()` function forcefully reinterprets its 32-bit unsigned integer argument as signed and extends it to a 64-bit type (since the 32-bit type is unsigned). For instance, 1-2 ordinarily evaluates to 4294967295, but `SINT(1-2)` evaluates to -1.
<!-- proofread -->
