.. _expressions,_functions,_and_operators:

Expressions, functions, and operators
-------------------------------------

Manticore lets you use arbitrary arithmetic expressions both via SphinxQL
and SphinxAPI, involving attribute values, internal attributes (document
ID and relevance weight), arithmetic operations, a number of built-in
functions, and user-defined functions. This section documents the
supported operators and functions. Here's the complete reference list
for quick access.

-  :ref:`Arithmetic operators  <expr-ari-ops>`: +, -, \*, /, %, DIV,
   MOD

-  :ref:`Comparison operators <expr-comp-ops>`: <, > <=, >=, =,
   <> 

-  :ref:`Boolean operators: AND, OR, NOT <expr-bool-ops>`

-  :ref:`Bitwise operators: &, \| <expr-bitwise-ops>`

-  :ref:`ABS() <expr-func-abs>`

-  :ref:`ALL() <expr-func-all>`

-  :ref:`ANY() <expr-func-any>`

-  :ref:`ATAN2() <expr-func-atan2>`

-  :ref:`BIGINT() <expr-func-bigint>`

-  :ref:`BITDOT() <expr-func-bitdot>`

-  :ref:`CEIL() <expr-func-ceil>`

-  :ref:`CONTAINS() <expr-func-contains>`

-  :ref:`COS() <expr-func-cos>`

-  :ref:`CRC32() <expr-func-crc32>`

-  :ref:`DAY() <expr-func-day>`

-  :ref:`DOUBLE() <expr-func-double>`

-  :ref:`EXP() <expr-func-exp>`

-  :ref:`FIBONACCI() <expr-func-fibonacci>`

-  :ref:`FLOOR() <expr-func-floor>`

-  :ref:`GEODIST() <expr-func-geodist>`

-  :ref:`GEOPOLY2D() <expr-func-geopoly2d>`

-  :ref:`GREATEST() <expr-func-greatest>`

-  :ref:`HOUR() <expr-func-hour>`

-  :ref:`IDIV() <expr-func-idiv>`

-  :ref:`IF() <expr-func-if>`

-  :ref:`IN() <expr-func-in>`

-  :ref:`INDEXOF() <expr-func-indexof>`

-  :ref:`INTEGER() <expr-func-integer>`

-  :ref:`INTERVAL() <expr-func-interval>`

-  :ref:`LEAST() <expr-func-least>`

-  :ref:`LENGTH() <expr-func-length>`

-  :ref:`LN() <expr-func-ln>`

-  :ref:`LOG10() <expr-func-log10>`

-  :ref:`LOG2() <expr-func-log2>`

-  :ref:`MAX() <expr-func-max>`

-  :ref:`MIN() <expr-func-min>`

-  :ref:`MINUTE() <expr-func-minute>`

-  :ref:`MIN_TOP_SORTVAL() <expr-func-min-top-sortval>`

-  :ref:`MIN_TOP_WEIGHT() <expr-func-min-top-weight>`

-  :ref:`MONTH() <expr-func-month>`

-  :ref:`NOW() <expr-func-now>`

-  :ref:`POLY2D() <expr-func-poly2d>`

-  :ref:`POW() <expr-func-pow>`

-  :ref:`RAND() <expr-func-rand>`

-  :ref:`REMAP() <expr-func-remap>`

-  :ref:`SECOND() <expr-func-second>`

-  :ref:`SIN() <expr-func-sin>`

-  :ref:`SINT() <expr-func-sint>`

-  :ref:`SQRT() <expr-func-sqrt>`

-  :ref:`UINT() <expr-func-uint>`

-  :ref:`YEAR() <expr-func-year>`

-  :ref:`YEARMONTH() <expr-func-yearmonth>`

-  :ref:`YEARMONTHDAY() <expr-func-yearmonthday>`


.. _Operators:

Operators
~~~~~~~~~

.. _expr-ari-ops:

-  Arithmetic operators: +, -, \*, /, %, DIV, MOD
-  The standard arithmetic operators. Arithmetic calculations involving
   those can be performed in three different modes: (a) using
   single-precision, 32-bit IEEE 754 floating point values (the
   default), (**) using signed 32-bit integers, (c) using 64-bit signed
   integers. The expression parser will automatically switch to integer
   mode if there are no operations the result in a floating point value.
   Otherwise, it will use the default floating point mode. For instance,
   ``a+b`` will be computed using 32-bit integers if both arguments are
   32-bit integers; or using 64-bit integers if both arguments are
   integers but one of them is 64-bit; or in floats otherwise. However,
   ``a/**`` or ``sqrt(a)`` will always be computed in floats, because
   these operations return a result of non-integer type. To avoid the
   first, you can either use ``IDIV(a,**)`` or ``a DIV b`` form. Also,
   ``a*b`` will not be automatically promoted to 64-bit when the
   arguments are 32-bit. To enforce 64-bit results, you can use
   BIGINT(). (But note that if there are non-integer operations,
   BIGINT() will simply be ignored.)
   
.. _expr-comp-ops:

-  Comparison operators: <, > <=, >=, =, <>
-  Comparison operators (eg. = or <=) return 1.0 when the condition is
   true and 0.0 otherwise. For instance, ``(a=b)+3`` will evaluate to 4
   when attribute ‘a’ is equal to attribute ‘b’, and to 3 when ‘a’ is
   not. Unlike MySQL, the equality comparisons (ie. = and <> operators)
   introduce a small equality threshold (1e-6 by default). If the
   difference between compared values is within the threshold, they will
   be considered equal.

.. _expr-bool-ops:
   
-  Boolean operators: AND, OR, NOT
-  Boolean operators (AND, OR, NOT) behave as usual. They are
   left-associative and have the least priority compared to other
   operators. NOT has more priority than AND and OR but nevertheless
   less than any other operator. AND and OR have the same priority so
   brackets use is recommended to avoid confusion in complex
   expressions.

.. _expr-bitwise-ops:

-  Bitwise operators: &, \|
-  These operators perform bitwise AND and OR respectively. The operands
   must be of an integer types.

   
.. _Numeric functions:

Numeric functions
~~~~~~~~~~~~~~~~~

.. _expr-func-abs:

-  ABS()
-  Returns the absolute value of the argument.

.. _expr-func-bitdot:

-  BITDOT()
-  BITDOT(mask, w0, w1, …) returns the sum of products of an each bit of
   a mask multiplied with its weight. ``bit0*w0 + bit1*w1 + ...``

 .. _expr-func-ceil:

-  CEIL()
-  Returns the smallest integer value greater or equal to the argument.

.. _expr-func-contains:

-  CONTAINS()
-  CONTAINS(polygon, x, y) checks whether the (x,y) point is within the
   given polygon, and returns 1 if true, or 0 if false. The polygon has
   to be specified using either the :ref:`POLY2D() <expr-func-poly2d>`
   function or the :ref:`GEOPOLY2D() <expr-func-poly2d>` function. The
   former function is intended for “small” polygons, meaning less than
   500 km (300 miles) a side, and it doesn't take into account the
   Earth's curvature for speed. For larger distances, you should use
   GEOPOLY2D, which tessellates the given polygon in smaller parts,
   accounting for the Earth's curvature.

.. _expr-func-cos:

-  COS()
-  Returns the cosine of the argument.

.. _expr-func-double:

-  DOUBLE()
-  Forcibly promotes given argument to floating point type. Intended to
   help enforce evaluation of numeric JSON fields.

.. _expr-func-exp:
    
-  EXP()
-  Returns the exponent of the argument (e=2.718… to the power of the
   argument).

.. _expr-func-fibonacci:
   
-  FIBONACCI()
-  Returns the N-th Fibonacci number, where N is the integer argument.
   That is, arguments of 0 and up will generate the values 0, 1, 1, 2,
   3, 5, 8, 13 and so on. Note that the computations are done using
   32-bit integer math and thus numbers 48th and up will be returned
   modulo 2^32.

.. _expr-func-floor:

-  FLOOR()
-  Returns the largest integer value lesser or equal to the argument.

.. _expr-func-geopoly2d:

-  GEOPOLY2D()
-  GEOPOLY2D(x1,y1,x2,y2,x3,y3…) produces a polygon to be used with the
   :ref:`CONTAINS() <expr-func-contains>` function. This function takes
   into account the Earth's curvature by tessellating the polygon into
   smaller ones, and should be used for larger areas; see the
   :ref:`POLY2D() <expr-func-poly2d>` function. The function expects
   coordinates to be in degrees, if radians are used it will give same
   result as POLY2D().

.. _expr-func-idiv:

-  IDIV()
-  Returns the result of an integer division of the first argument by
   the second argument. Both arguments must be of an integer type.

.. _expr-func-ln:

-  LN()
-  Returns the natural logarithm of the argument (with the base of
   e=2.718…).

.. _expr-func-log10:
   
-  LOG10()
-  Returns the common logarithm of the argument (with the base of 10).

.. _expr-func-log2:

-  LOG2()
-  Returns the binary logarithm of the argument (with the base of 2).

.. _expr-func-max:

-  MAX()
-  Returns the bigger of two arguments.

.. _expr-func-min:

-  MIN()
-  Returns the smaller of two arguments.

.. _expr-func-poly2d:

-  POLY2D()
-  POLY2D(x1,y1,x2,y2,x3,y3…) produces a polygon to be used with the
   :ref:`CONTAINS() <expr-func-contains>` function. This polygon assumes a
   flat Earth, so it should not be too large; see the
   :ref:`POLY2D() <expr-func-poly2d>` function.

.. _expr-func-pow:

-  POW()
-  Returns the first argument raised to the power of the second
   argument.
   
.. _expr-func-sin:

-  SIN()
-  Returns the sine of the argument.

.. _expr-func-sqrt:

-  SQRT()
-  Returns the square root of the argument.

.. _expr-func-uint:

-  UINT()
-  Forcibly reinterprets given argument to 64-bit unsigned type.


.. _Date and time functions:

Date and time functions
~~~~~~~~~~~~~~~~~~~~~~~

.. _expr-func-day:

-  DAY()
-  Returns the integer day of month (in 1..31 range) from a timestamp
   argument, according to the current timezone.

.. _expr-func-month:

-  MONTH()
-  Returns the integer month (in 1..12 range) from a timestamp argument,
   according to the current timezone.

.. _expr-func-now:

-  NOW()
-  Returns the current timestamp as an INTEGER.

.. _expr-func-year:

-  YEAR()
-  Returns the integer year (in 1969..2038 range) from a timestamp
   argument, according to the current timezone.

.. _expr-func-yearmonth:

-  YEARMONTH()
-  Returns the integer year and month code (in 196912..203801 range)
   from a timestamp argument, according to the current timezone.

.. _expr-func-yearmonthday:

-  YEARMONTHDAY()
-  Returns the integer year, month, and date code (in 19691231..20380119
   range) from a timestamp argument, according to the current timezone.

.. _expr-func-second:

-  SECOND()
-  Returns the integer second (in 0..59 range) from a timestamp
   argument, according to the current timezone.

.. _expr-func-minute:

-  MINUTE()
-  Returns the integer minute (in 0..59 range) from a timestamp
   argument, according to the current timezone.

.. _expr-func-hour:

-  HOUR()
-  Returns the integer hour (in 0..23 range) from a timestamp argument,
   according to the current timezone.

   
.. _Type conversion functions:

Type conversion functions
~~~~~~~~~~~~~~~~~~~~~~~~~

.. _expr-func-bigint:

-  BIGINT()
-  Forcibly promotes the integer argument to 64-bit type, and does
   nothing on floating point argument. It's intended to help enforce
   evaluation of certain expressions (such as ``a*b``) in 64-bit mode
   even though all the arguments are 32-bit.

.. _expr-func-integer:

-  INTEGER()
-  Forcibly promotes given argument to 64-bit signed type. Intended to
   help enforce evaluation of numeric JSON fields.

.. _expr-func-sint:

-  SINT()
-  Forcibly reinterprets its 32-bit unsigned integer argument as signed,
   and also expands it to 64-bit type (because 32-bit type is unsigned).
   It's easily illustrated by the following example: 1-2 normally
   evaluates to 4294967295, but SINT(1-2) evaluates to -1.

   
.. _comparison functions:

Comparison functions
~~~~~~~~~~~~~~~~~~~~

.. _expr-func-if:

-  IF()
-  ``IF()`` behavior is slightly different that that of its MySQL
   counterpart. It takes 3 arguments, check whether the 1st argument is
   equal to 0.0, returns the 2nd argument if it is not zero, or the 3rd
   one when it is. Note that unlike comparison operators, ``IF()`` does
   **not** use a threshold! Therefore, it's safe to use comparison
   results as its 1st argument, but arithmetic operators might produce
   unexpected results. For instance, the following two calls will
   produce *different* results even though they are logically
   equivalent:

   ::


       IF ( sqrt(3)*sqrt(3)-3<>0, a, b )
       IF ( sqrt(3)*sqrt(3)-3, a, b )

   In the first case, the comparison operator <> will return 0.0 (false)
   because of a threshold, and ``IF()`` will always return ‘**’ as a
   result. In the second one, the same ``sqrt(3)*sqrt(3)-3`` expression
   will be compared with zero *without* threshold by the ``IF()``
   function itself. But its value will be slightly different from zero
   because of limited floating point calculations precision. Because of
   that, the comparison with 0.0 done by ``IF()`` will not pass, and the
   second variant will return ‘a’ as a result.

.. _expr-func-in:

-  IN()
-  IN(expr,val1,val2,…) takes 2 or more arguments, and returns 1 if 1st
   argument (expr) is equal to any of the other arguments (val1..valN),
   or 0 otherwise. Currently, all the checked values (but not the
   expression itself!) are required to be constant. (Its technically
   possible to implement arbitrary expressions too, and that might be
   implemented in the future.) Constants are pre-sorted and then binary
   search is used, so IN() even against a big arbitrary list of
   constants will be very quick. First argument can also be a MVA
   attribute. In that case, IN() will return 1 if any of the MVA values
   is equal to any of the other arguments. IN() also supports
   ``IN(expr,@uservar)`` syntax to check whether the value belongs to
   the list in the given global user variable. First argument can be
   JSON attribute.

.. _expr-func-interval:

-  INTERVAL()
-  INTERVAL(expr,point1,point2,point3,…), takes 2 or more arguments, and
   returns the index of the argument that is less than the first
   argument: it returns 0 if expr<point1, 1 if point1<=expr<point2, and
   so on. It is required that point1<point2<…<pointN for this function
   to work correctly.

   
.. _Miscellaneous functions:

Miscellaneous functions
~~~~~~~~~~~~~~~~~~~~~~~

.. _expr-func-all:

-  ALL()
-  ALL(cond FOR var IN json.array) applies to JSON arrays and returns 1
   if condition is true for all elements in array and 0 otherwise.
   ‘cond’ is a general expression which additionally can use ‘var’ as
   current value of an array element within itself.

   ::


       SELECT ALL(x>3 AND x<7 FOR x IN j.intarray) FROM test;

.. _expr-func-any:

-  ANY()
-  ANY(cond FOR var IN json.array) works similar to
   :ref:`ALL() <expr-func-all>` except for it returns 1 if condition is
   true for any element in array.

.. _expr-func-atan2:

-  ATAN2()
-  Returns the arctangent function of two arguments, expressed in
   **radians**.

.. _expr-func-crc32:

-  CRC32()
-  Returns the CRC32 value of a string argument.

.. _expr-func-geodist:

-  GEODIST()
-  GEODIST(lat1, lon1, lat2, lon2, […]) function computes geosphere
   distance between two given points specified by their coordinates.
   Note that by default both latitudes and longitudes must be in
   **radians** and the result will be in **meters**. You can use
   arbitrary expression as any of the four coordinates. An optimized
   path will be selected when one pair of the arguments refers directly
   to a pair attributes and the other one is constant.

   GEODIST() also takes an optional 5th argument that lets you easily
   convert between input and output units, and pick the specific
   geodistance formula to use. The complete syntax and a few examples
   are as follows:

   ::


       GEODIST(lat1, lon1, lat2, lon2, { option=value, ... })

       GEODIST(40.7643929, -73.9997683, 40.7642578, -73.9994565, {in=degrees, out=feet})
       GEODIST(51.50, -0.12, 29.98, 31.13, {in=deg, out=mi}}

   The known options and their values are:

   -  ``in = {deg | degrees | rad | radians}``, specifies the input
      units;
   -  ``out = {m | meters | km | kilometers | ft | feet | mi | miles}``,
      specifies the output units;
   -  ``method = {adaptive | haversine}``, specifies the geodistance
      calculation method.

   The default method is “adaptive”. It is well optimized implementation
   that is both more precise *and* much faster at all times than
   “haversine”.

.. _expr-func-greatest:

-  GREATEST()
-  GREATEST(attr_json.some_array) function takes JSON array as the
   argument, and returns the greatest value in that array. Also works
   for MVA.

.. _expr-func-indexof:

-  INDEXOF()
-  INDEXOF(cond FOR var IN json.array) function iterates through all
   elements in array and returns index of first element for which ‘cond’
   is true and -1 if ‘cond’ is false for every element in array.

   ::


       SELECT INDEXOF(name='John' FOR name IN j.peoples) FROM test;

.. _expr-func-least:

-  LEAST()
-  LEAST(attr_json.some_array) function takes JSON array as the
   argument, and returns the least value in that array. Also works for
   MVA.

.. _expr-func-length:

-  LENGTH()
-  LENGTH(attr_mva) function returns amount of elements in MVA set. It
   works with both 32-bit and 64-bit MVA attributes. LENGTH(attr_json)
   returns length of a field in JSON. Return value depends on type of a
   field. For example LENGTH(json_attr.some_int) always returns 1 and
   LENGTH(json_attr.some_array) returns number of elements in array.

.. _expr-func-min-top-sortval:

-  MIN_TOP_SORTVAL()
-  Returns sort key value of the worst found element in the current
   top-N matches if sort key is float and 0 otherwise.

.. _expr-func-min-top-weight:

-  MIN_TOP_WEIGHT()
-  Returns weight of the worst found element in the current top-N
   matches.

.. _expr-func-packedfactors:

-  PACKEDFACTORS()
-  PACKEDFACTORS() can be used in queries, either to just see all the
   weighting factors calculated when doing the matching, or to provide a
   binary attribute that can be used to write a custom ranking UDF. This
   function works only if expression ranker is specified and the query
   is not a full scan, otherwise it will return an error.
   PACKEDFACTORS() can take an optional argument that disables ATC
   ranking factor calculation:

   ::


       PACKEDFACTORS({no_atc=1})

   Calculating ATC slows down query processing considerably, so this
   option can be useful if you need to see the ranking factors, but do
   not need ATC. PACKEDFACTORS() can also be told to format its output
   as JSON:

   ::


       PACKEDFACTORS({json=1})

   The respective outputs in either key-value pair or JSON format would
   look as follows below. (Note that the examples below are wrapped for
   readability; actual returned values would be single-line.)

   ::


       mysql> SELECT id, PACKEDFACTORS() FROM test1
           -> WHERE MATCH('test one') OPTION ranker=expr('1') \G
       *************************** 1\. row ***************************
                    id: 1
       packedfactors(): bm25=569, bm25a=0.617197, field_mask=2, doc_word_count=2,
           field1=(lcs=1, hit_count=2, word_count=2, tf_idf=0.152356,
               min_idf=-0.062982, max_idf=0.215338, sum_idf=0.152356, min_hit_pos=4,
               min_best_span_pos=4, exact_hit=0, max_window_hits=1, min_gaps=2,
               exact_order=1, lccs=1, wlccs=0.215338, atc=-0.003974),
           word0=(tf=1, idf=-0.062982),
           word1=(tf=1, idf=0.215338)
       1 row in set (0.00 sec)

       mysql> SELECT id, PACKEDFACTORS({json=1}) FROM test1
           -> WHERE MATCH('test one') OPTION ranker=expr('1') \G
       *************************** 1\. row ***************************
                            id: 1
       packedfactors({json=1}):
       {

           "bm25": 569,
           "bm25a": 0.617197,
           "field_mask": 2,
           "doc_word_count": 2,
           "fields": [
               {
                   "lcs": 1,
                   "hit_count": 2,
                   "word_count": 2,
                   "tf_idf": 0.152356,
                   "min_idf": -0.062982,
                   "max_idf": 0.215338,
                   "sum_idf": 0.152356,
                   "min_hit_pos": 4,
                   "min_best_span_pos": 4,
                   "exact_hit": 0,
                   "max_window_hits": 1,
                   "min_gaps": 2,
                   "exact_order": 1,
                   "lccs": 1,
                   "wlccs": 0.215338,
                   "atc": -0.003974
               }
           ],
           "words": [
               {
                   "tf": 1,
                   "idf": -0.062982
               },
               {
                   "tf": 1,
                   "idf": 0.215338
               }
           ]

       }
       1 row in set (0.01 sec)

   This function can be used to implement custom ranking functions in
   UDFs, as in

   ::


       SELECT *, CUSTOM_RANK(PACKEDFACTORS()) AS r
       FROM my_index
       WHERE match('hello')
       ORDER BY r DESC
       OPTION ranker=expr('1');

   Where CUSTOM_RANK() is a function implemented in an UDF. It should
   declare a SPH_UDF_FACTORS structure (defined in ``sphinxudf.h``),
   initialize this structure, unpack the factors into it before usage,
   and deinitialize it afterwards, as follows:

   ::


       SPH_UDF_FACTORS factors;
       sphinx_factors_init(&factors);
       sphinx_factors_unpack((DWORD*)args->arg_values[0], &factors);
       // ... can use the contents of factors variable here ...
       sphinx_factors_deinit(&factors);

   PACKEDFACTORS() data is available at all query stages, not just when
   doing the initial matching and ranking pass. That enables another
   particularly interesting application of PACKEDFACTORS(), namely
   **re-ranking**.

   In the example just above, we used an expression-based ranker with a
   dummy expression, and sorted the result set by the value computed by
   our UDF. In other words, we used the UDF to *rank* all our results.
   Assume now, for the sake of an example, that our UDF is extremely
   expensive to compute and has a throughput of just 10,000 calls per
   second. Assume that our query matches 1,000,000 documents. To
   maintain reasonable performance, we would then want to use a (much)
   simpler expression to do most of our ranking, and then apply the
   expensive UDF to only a few top results, say, top-100 results. Or, in
   other words, build top-100 results using a simpler ranking function
   and then *re-rank* those with a complex one. We can do that just as
   well with subselects:

   ::


       SELECT * FROM (
           SELECT *, CUSTOM_RANK(PACKEDFACTORS()) AS r
           FROM my_index WHERE match('hello')
           OPTION ranker=expr('sum(lcs)*1000+bm25')
           ORDER BY WEIGHT() DESC
           LIMIT 100
       ) ORDER BY r DESC LIMIT 10

   In this example, expression-based ranker will be called for every
   matched document to compute WEIGHT(). So it will get called 1,000,000
   times. But the UDF computation can be postponed until the outer sort.
   And it also will be done for just the top-100 matches by WEIGHT(),
   according to the inner limit. So the UDF will only get called 100
   times. And then the final top-10 matches by UDF value will be
   selected and returned to the application.

   For reference, in the distributed case PACKEDFACTORS() data gets sent
   from the agents to master in a binary format, too. This makes it
   technically feasible to implement additional re-ranking pass (or
   passes) on the master node, if needed.

   If used with SphinxQL but not called from any UDFs, the result of
   PACKEDFACTORS() is simply formatted as plain text, which can be used
   to manually assess the ranking factors. Note that this feature is not
   currently supported by the Manticore API.
   
.. _expr-func-remap:

-  REMAP()
-  REMAP(condition, expression, (cond1, cond2, …), (expr1, expr2, …))
   function allows you to make some exceptions of an expression values
   depending on condition values. Condition expression should always
   result integer, expression can result in integer or float.

   ::


       SELECT REMAP(userid, karmapoints, (1, 67), (999, 0)) FROM users;
       SELECT REMAP(id%10, salary, (0), (0.0)) FROM employes;

.. _expr-func-rand:

-  rand()
-  RAND(seed) function returns a random float between 0..1. Optional, an
   integer seed value can be specified.
