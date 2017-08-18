Result set filtering settings
-----------------------------

.. _set_filter:

SetFilter
~~~~~~~~~

**Prototype:** function SetFilter ( $attribute, $values,
$exclude=false )

Adds new integer values set filter.

On this call, additional new filter is added to the existing list of
filters. ``$attribute`` must be a string with attribute name.
``$values`` must be a plain array containing integer values.
``$exclude`` must be a boolean value; it controls whether to accept the
matching documents (default mode, when ``$exclude`` is false) or reject
them.

Only those documents where ``$attribute`` column value stored in the
index matches any of the values from ``$values`` array will be matched
(or rejected, if ``$exclude`` is true).


.. _set_filter_range:

SetFilterRange
~~~~~~~~~~~~~~

**Prototype:** function SetFilterRange ( $attribute, $min, $max,
$exclude=false )

Adds new integer range filter.

On this call, additional new filter is added to the existing list of
filters. ``$attribute`` must be a string with attribute name. ``$min``
and ``$max`` must be integers that define the acceptable attribute
values range (including the boundaries). ``$exclude`` must be a boolean
value; it controls whether to accept the matching documents (default
mode, when ``$exclude`` is false) or reject them.

Only those documents where ``$attribute`` column value stored in the
index is between ``$min`` and ``$max`` (including values that are
exactly equal to ``$min`` or ``$max``) will be matched (or rejected, if
``$exclude`` is true).

.. _set_filter_float_range:

SetFilterFloatRange
~~~~~~~~~~~~~~~~~~~

**Prototype:** function SetFilterFloatRange ( $attribute, $min, $max,
$exclude=false )

Adds new float range filter.

On this call, additional new filter is added to the existing list of
filters. ``$attribute`` must be a string with attribute name. ``$min``
and ``$max`` must be floats that define the acceptable attribute values
range (including the boundaries). ``$exclude`` must be a boolean value;
it controls whether to accept the matching documents (default mode, when
``$exclude`` is false) or reject them.

Only those documents where ``$attribute`` column value stored in the
index is between ``$min`` and ``$max`` (including values that are
exactly equal to ``$min`` or ``$max``) will be matched (or rejected, if
``$exclude`` is true).


.. _set_filter_string:

SetFilterString
~~~~~~~~~~~~~~~

**Prototype:** function SetFilterString ( $attribute, $value,
$exclude=false )

Adds new string value filter.

On this call, additional new filter is added to the existing list of
filters. ``$attribute`` must be a string with attribute name. ``$value``
must be a string. ``$exclude`` must be a boolean value; it controls
whether to accept the matching documents (default mode, when
``$exclude`` is false) or reject them.

Only those documents where ``$attribute`` column value stored in the
index matches string value from ``$value`` will be matched (or rejected,
if ``$exclude`` is true).

.. _set_id_range:

SetIDRange
~~~~~~~~~~

**Prototype:** function SetIDRange ( $min, $max )

Sets an accepted range of document IDs. Parameters must be integers.
Defaults are 0 and 0; that combination means to not limit by range.

After this call, only those records that have document ID between
``$min`` and ``$max`` (including IDs exactly equal to ``$min`` or
``$max``) will be matched.


.. _set_geo_anchor:

SetGeoAnchor
~~~~~~~~~~~~

**Prototype:** function SetGeoAnchor ( $attrlat, $attrlong, $lat,
$long )

Sets anchor point for and geosphere distance (geodistance) calculations,
and enable them.

``$attrlat`` and ``$attrlong`` must be strings that contain the names of
latitude and longitude attributes, respectively. ``$lat`` and ``$long``
are floats that specify anchor point latitude and longitude, in radians.

Once an anchor point is set, you can use magic ``@geodist``
attribute name in your filters and/or sorting expressions. Manticore will
compute geosphere distance between the given anchor point and a point
specified by latitude and longitude attributes from each full-text
match, and attach this value to the resulting match. The latitude and
longitude values both in ``SetGeoAnchor`` and the index attribute data
are expected to be in radians. The result will be returned in meters, so
geodistance value of 1000.0 means 1 km. 1 mile is approximately 1609.344
meters.

