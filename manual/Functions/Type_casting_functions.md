# Type casting functions

### BIGINT()
Forcibly promotes the integer argument to 64-bit type, and does nothing on floating point argument. It's intended to help enforce evaluation of certain expressions (such as `a*b`) in 64-bit mode even though all the arguments are 32-bit.

### DOUBLE()
Forcibly promotes given argument to floating point type. Intended to help enforce evaluation of numeric JSON fields.

### INTEGER()
Forcibly promotes given argument to 64-bit signed type. Intended to help enforce evaluation of numeric JSON fields.

### TO_STRING()
Forcibly promotes the argument to string type

### UINT()
Converts the given argument to 32-bit unsigned integer type.

### UINT64()
Converts the given argument to 64-bit unsigned integer type.

### SINT()
Forcibly reinterprets its 32-bit unsigned integer argument as signed, and also expands it to 64-bit type (because 32-bit type is unsigned). It's easily illustrated by the following example: 1-2 normally evaluates to 4294967295, but `SINT(1-2)` evaluates to -1.
