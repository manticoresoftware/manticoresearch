# Type casting functions

### BIGINT()
Forcefully promotes the integer argument to a 64-bit type and does nothing to floating-point arguments. It is intended to help enforce the evaluation of certain expressions (such as `a*b`) in 64-bit mode, even when all arguments are 32-bit.

### DOUBLE()
Forcefully promotes the given argument to a floating-point type. This is intended to help enforce the evaluation of numeric JSON fields.

### INTEGER()
Forcefully promotes the given argument to a 64-bit signed type. This is intended to help enforce the evaluation of numeric JSON fields.

### TO_STRING()
Forcefully promotes the argument to a string type.

### UINT()
Converts the given argument to a 32-bit unsigned integer type.

### UINT64()
Converts the given argument to a 64-bit unsigned integer type.

### SINT()
Forcefully reinterprets its 32-bit unsigned integer argument as signed and expands it to a 64-bit type (because the 32-bit type is unsigned). This is easily illustrated by the following example: 1-2 normally evaluates to 4294967295, but `SINT(1-2)` evaluates to -1.
<!-- proofread -->
