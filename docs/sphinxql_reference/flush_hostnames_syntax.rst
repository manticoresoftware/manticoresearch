.. _flush_hostnames_syntax:

FLUSH HOSTNAMES syntax
----------------------

::


    FLUSH HOSTNAMES

Renew IPs associates to agent host names. To always query the DNS for
getting the host name IP, see
:ref:`hostname_lookup <hostname_lookup>`
directive.

::


    mysql> FLUSH HOSTNAMES;
    Query OK, 5 rows affected (0.01 sec)

