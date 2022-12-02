# Mathematical functions

### ABS()
Returns the absolute value of the argument.

### ATAN2()
Returns the arctangent function of two arguments, expressed in **radians**.

### BITDOT()
`BITDOT(mask, w0, w1, ...)` returns the sum of products of an each bit of a mask multiplied with its weight. `bit0*w0 + bit1*w1 + ...`

### CEIL()
Returns the smallest integer value greater or equal to the argument.

### COS()
Returns the cosine of the argument.

### CRC32()
Returns the CRC32 value of a string argument.

### EXP()
Returns the exponent of the argument (e=2.718... to the power of the argument).

### FIBONACCI()
Returns the N-th Fibonacci number, where N is the integer argument. That is, arguments of 0 and up will generate the values 0, 1, 1, 2, 3, 5, 8, 13 and so on. Note that the computations are done using 32-bit integer math and thus numbers 48th and up will be returned modulo 2\^32.

### FLOOR()
Returns the largest integer value lesser or equal to the argument.

### GREATEST()
`GREATEST(attr_json.some_array)` function takes JSON array as the argument, and returns the greatest value in that array. Also works for MVA.

### IDIV()
Returns the result of an integer division of the first argument by the second argument. Both arguments must be of an integer type.

### LEAST()
`LEAST(attr_json.some_array)` function takes JSON array as the argument, and returns the least value in that array. Also works for  MVA.

### LN()
Returns the natural logarithm of the argument (with the base of e=2.718...).

### LOG10()
Returns the common logarithm of the argument (with the base of 10).

### LOG2()
Returns the binary logarithm of the argument (with the base of 2).

### MAX()
Returns the bigger of two arguments.

### MIN()
Returns the smaller of two arguments.

### POW()
Returns the first argument raised to the power of the second argument.

### RAND()
RAND(seed) function returns a random float between 0..1. Optionally can accept `seed` which can be:
* constant integer
* or integer attribute's name

If you use the `seed` take into account that it resets `rand()`'s starting point separately for each plain table / RT disk / RAM chunk / pseudo shard, so queries to a distributed table in any form can return multiple identical random values.

### SIN()
Returns the sine of the argument.

### SQRT()
Returns the square root of the argument.
