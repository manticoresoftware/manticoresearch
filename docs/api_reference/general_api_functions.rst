General API functions
---------------------

.. _get_last_error:

GetLastError
~~~~~~~~~~~~

Prototype: function GetLastError()

Returns last error message, as a string, in human readable format. If
there were no errors during the previous API call, empty string is
returned.

You should call it when any other function (such as
:ref:`Query() <query>`) fails (typically, the failing
function returns false). The returned string will contain the error
description.

The error message is *not* reset by this call; so you can safely call it
several times if needed.


.. _get_last_warning:

GetLastWarning
~~~~~~~~~~~~~~

**Prototype:** function GetLastWarning ()

Returns last warning message, as a string, in human readable format. If
there were no warnings during the previous API call, empty string is
returned.

You should call it to verify whether your request (such as
:ref:`Query() <query>`) was completed but with warnings.
For instance, search query against a distributed index might complete
successfully even if several remote agents timed out. In that case, a
warning message would be produced.

The warning message is *not* reset by this call; so you can safely call
it several times if needed.


.. _set_server:

SetServer
~~~~~~~~~

**Prototype:** function SetServer ( $host, $port )

Sets ``searchd`` host name and TCP port. All subsequent requests will
use the new host and port settings. Default host and port are
‘localhost’ and 9312, respectively.


.. _set_retries:

SetRetries

~~~~~~~~~~

**Prototype:** function SetRetries ( $count, $delay=0 )

Sets distributed retry count and delay.

On temporary failures ``searchd`` will attempt up to ``$count`` retries
per agent. ``$delay`` is the delay between the retries, in milliseconds.
Retries are disabled by default. Note that this call will **not**
make the API itself retry on temporary failure; it only tells
``searchd`` to do so. Currently, the list of temporary failures includes
all kinds of connect() failures and maxed out (too busy) remote agents.


.. _set_connect_timeout:

SetConnectTimeout
~~~~~~~~~~~~~~~~~

**Prototype:** function SetConnectTimeout ( $timeout )

Sets the time allowed to spend connecting to the server before giving
up.

Under some circumstances, the server can be delayed in responding,
either due to network delays, or a query backlog. In either instance,
this allows the client application programmer some degree of control
over how their program interacts with ``searchd`` when not available,
and can ensure that the client application does not fail due to
exceeding the script execution limits (especially in PHP).

In the event of a failure to connect, an appropriate error code should
be returned back to the application in order for application-level error
handling to advise the user.


.. _set_array_result:

SetArrayResult
~~~~~~~~~~~~~~

**Prototype:** function SetArrayResult ( $arrayresult )

PHP specific. Controls matches format in the search results set (whether
matches should be returned as an array or a hash).

``$arrayresult`` argument must be boolean. If ``$arrayresult`` is
``false`` (the default mode), matches will returned in PHP hash format
with document IDs as keys, and other information (weight, attributes) as
values. If ``$arrayresult`` is true, matches will be returned as a plain
array with complete per-match information including document ID.

Introduced along with GROUP BY support on MVA attributes. Group-by-MVA
result sets may contain duplicate document IDs. Thus they need to be
returned as plain arrays, because hashes will only keep one entry per
document ID.


.. _is_connect_error:

IsConnectError
~~~~~~~~~~~~~~

**Prototype:** function IsConnectError ()

Checks whether the last error was a network error on API side, or a
remote error reported by searchd. Returns true if the last connection
attempt to searchd failed on API side, false otherwise (if the error was
remote, or there were no connection attempts at all).
 