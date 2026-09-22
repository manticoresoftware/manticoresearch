# Mathematical functions

Some common functions, such as `ASIN()`, `ACOS()`, `ATAN()`, `TAN()`, `RADIANS()`, `DEGREES()` and `PI()`, are not implemented in Manticore, but they can easily be replaced with these equivalents:

| Function | Equivalent |
|---|---|
| `ASIN(x)` | `ATAN2(x, SQRT(1 - x*x))` |
| `ACOS(x)` | `ATAN2(SQRT(1 - x*x), x)` |
| `ATAN(x)` | `ATAN2(x, 1)` |
| `TAN(x)` | `SIN(x) / COS(x)` |
| `RADIANS(d)` | `d * 0.017453292519943295` |
| `DEGREES(r)` | `r * 57.29577951308232` |
| `PI()` | `3.141592653589793` |

Calling a function that is not implemented returns a parse error such as `P09: syntax error, unexpected '('`.

Floating-point math is done in single precision (32-bit); see [Float](../Creating_a_table/Data_types.md#Float).

### ABS()
Returns the absolute value of the argument.

### ATAN2()
Returns the arctangent function of two arguments, expressed in **radians**.

### BITDOT()
`BITDOT(mask, w0, w1, ...)` returns the sum of products of each bit of a mask multiplied by its weight. `bit0*w0 + bit1*w1 + ...`

### CEIL()
Returns the smallest integer value greater than or equal to the argument.

### COS()
Returns the cosine of the argument.

### CRC32()
Returns the CRC32 value of a string argument.

### EXP()
Returns the exponent of the argument (e=2.718... to the power of the argument).

### FIBONACCI()
Returns the N-th Fibonacci number, where N is the integer argument. That is, arguments of 0 and up will generate the values 0, 1, 1, 2, 3, 5, 8, 13 and so on. Note that the computations are done using 32-bit integer math and thus numbers 48th and up will be returned modulo 2^32.

### FLOOR()
Returns the largest integer value lesser than or equal to the argument.

### GREATEST()
`GREATEST(attr_json.some_array)` function takes a JSON array as the argument, and returns the greatest value in that array. Also works for MVA.

### IDIV()
Returns the result of an integer division of the first argument by the second argument. Both arguments must be of an integer type.

### LEAST()
`LEAST(attr_json.some_array)` function takes a JSON array as the argument, and returns the least value in that array. Also works for MVA.

### LN()
Returns the natural logarithm of the argument (with the base of e=2.718...).

### LOG10()
Returns the common logarithm of the argument (with the base of 10).

### LOG2()
Returns the binary logarithm of the argument (with the base of 2).

### MAX()
Returns the larger of two arguments.

### MIN()
Returns the smaller of two arguments.

## POW()
Returns the first argument raised to the power of the second argument.
### RAND()
Returns a random float between 0 and 1. It can optionally accept a `seed`, which can be a constant integer or an integer attribute's name.

If you use a `seed`, keep in mind that it resets `rand()`'s starting point separately for each plain table, RT disk, RAM chunk, or pseudo shard. Therefore, queries to a distributed table in any form can return multiple identical random values.

### SIN()
Returns the sine of the argument.

### SQRT()
Returns the square root of the argument.

<!-- proofread -->

