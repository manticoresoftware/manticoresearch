.. _debug_syntax:

DEBUG syntax
------------

.. code-block:: none


    DEBUG [ subcommand ]

DEBUG statement is designed to call different internal or vip commands for dev/testing purposes. It is not intended
for production automation, since the syntax of ``subcommand`` part may be freely changed in any build.

Call ``DEBUG`` without params to show list of useful commands (in general) and subcommands (of ``DEBUG`` statement) available
at the moment.

However you can invoke ``DEBUG`` without params to know which subcommands of the statement are available in any particular case:

.. code-block:: mysql


	mysql> debug;
	+----------------+---------------------+
	| command        | meaning             |
	+----------------+---------------------+
	| flush logs     | emulate USR1 signal |
	| reload indexes | emulate HUP signal  |
	+----------------+---------------------+
	2 rows in set (0,00 sec)


(these commands are already documented, but such short help just remind about them).

If you connect via 'VIP' connection (see :ref:`listen` for details) the output might be a bit different:

.. code-block:: mysql


	mysql> debug;
	+---------------------------+------------------------------+
	| command                   | meaning                      |
	+---------------------------+------------------------------+
	| debug shutdown <password> | emulate TERM signal          |
	| debug token <password>    | calculate token for password |
	| flush logs                | emulate USR1 signal          |
	| reload indexes            | emulate HUP signal           |
	+---------------------------+------------------------------+
	4 rows in set (0,00 sec)


Here you can see additional commands available only in the current context (namely, if you connected on a VIP port).
Two additional subcommands available right now are ``token`` and ``shutdown``.
The first one just calculates a hash (SHA1) of the <password> (which, in turn, may be empty, or a word, or num/phrase enclosed in
'-quotes) like:

.. code-block:: mysql


	mysql> debug token hello;
	+-------------+------------------------------------------+
	| command     | result                                   |
	+-------------+------------------------------------------+
	| debug token | aaf4c61ddcc5e8a2dabede0f3b482cd9aea9434d |
	+-------------+------------------------------------------+
	1 row in set (0,00 sec)

Another debug subcommand ``shutdown`` will send a TERM signal to the daemon and so will make it shut down.
Since it is quite dangerous (nobody wants accidentally stop a production service), it 1) needs a VIP connection, and 2) needs
the password. For the chosen password you need to generate a token with ``debug token`` subcommand, and put it into
ref:`shutdown_token` param of searchd section of the config file. If no such section exists, or if a hash of the provided
password does not match with the token stored in the config, the subcommand will do nothing. Otherwise it will cause 'clean' shutdown
of the daemon.
