``searchd`` command reference
-----------------------------

``searchd`` is the second of the two principle tools as part of Manticore.
``searchd`` is the part of the system which actually handles searches;
it functions as a server and is responsible for receiving queries,
processing them and returning a dataset back to the different APIs for
client applications.

Unlike ``indexer``, ``searchd`` is not designed to be run either from a
regular script or command-line calling, but instead either as a daemon
to be called from init.d (on Unix/Linux type systems) or to be called as
a service (on Windows-type systems), so not all of the command line
options will always apply, and so will be build-dependent.

Calling ``searchd`` is simply a case of:

.. code-block:: bash


    $ searchd [OPTIONS]

The options available to ``searchd`` on all builds are:

-  ``--help`` (``-h`` for short) lists all of the parameters that can be
   called in your particular build of ``searchd``.

-  ``--config <file>`` (``-c <file>`` for short) tells
   ``searchd`` to use the given file as its configuration, just as with
   ``indexer`` above.

-  ``--stop`` is used to asynchronously stop ``searchd``, using the
   details of the PID file as specified in the ``sphinx.conf`` file, so
   you may also need to confirm to ``searchd`` which configuration file
   to use with the ``--config`` option. NB, calling ``--stop`` will also
   make sure any changes applied to the indexes with
   :ref:```UpdateAttributes()`` <update_attributes>`
   will be applied to the index files themselves. Example:

   .. code-block:: bash


       $ searchd --config /home/myuser/sphinx.conf --stop

-  ``--stopwait`` is used to synchronously stop ``searchd``. ``--stop``
   essentially tells the running instance to exit (by sending it a
   SIGTERM) and then immediately returns. ``--stopwait`` will also
   attempt to wait until the running ``searchd`` instance actually
   finishes the shutdown (eg. saves all the pending attribute changes)
   and exits. Example:

   .. code-block:: bash


       $ searchd --config /home/myuser/sphinx.conf --stopwait

   Possible exit codes are as follows:

   -  0 on success;

   -  1 if connection to running searchd daemon failed;

   -  2 if daemon reported an error during shutdown;

   -  3 if daemon crashed during shutdown.

-  ``--status`` command is used to query running ``searchd`` instance
   status, using the connection details from the (optionally) provided
   configuration file. It will try to connect to the running instance
   using the first configured UNIX socket or TCP port. On success, it
   will query for a number of status and performance counter values and
   print them. You can use
   :ref:`Status() <status>` API call to
   access the very same counters from your application. Examples:

   .. code-block:: bash


       $ searchd --status
       $ searchd --config /home/myuser/sphinx.conf --status

-  ``--pidfile`` is used to explicitly force using a PID file (where the
   ``searchd`` process number is stored) despite any other debugging
   options that say otherwise (for instance, ``--console``). This is a
   debugging option.

   .. code-block:: bash


       $ searchd --console --pidfile

-  ``--console`` is used to force ``searchd`` into console mode;
   typically it will be running as a conventional server application,
   and will aim to dump information into the log files (as specified in
   ``sphinx.conf``). Sometimes though, when debugging issues in the
   configuration or the daemon itself, or trying to diagnose
   hard-to-track-down problems, it may be easier to force it to dump
   information directly to the console/command line from which it is
   being called. Running in console mode also means that the process
   will not be forked (so searches are done in sequence) and logs will
   not be written to. (It should be noted that console mode is not the
   intended method for running ``searchd``.) You can invoke it as such:

   .. code-block:: bash


       $ searchd --config /home/myuser/sphinx.conf --console

-  ``--logdebug``, ``--logdebugv``, and ``--logdebugvv`` options enable
   additional debug output in the daemon log. They differ by the logging
   verboseness level. These are debugging options, they pollute the log
   a lot, and thus they should *not* be normally enabled. (The normal
   use case for these is to enable them temporarily on request, to
   assist with some particularly complicated debugging session.)

-  ``--iostats`` is used in conjunction with the logging options (the
   ``query_log`` will need to have been activated in ``sphinx.conf``) to
   provide more detailed information on a per-query basis as to the
   input/output operations carried out in the course of that query, with
   a slight performance hit and of course bigger logs. Further details
   are available under the `query log
   format <README>` section. You might
   start ``searchd`` thus:

   .. code-block:: bash


       $ searchd --config /home/myuser/sphinx.conf --iostats

-  ``--cpustats`` is used to provide actual CPU time report (in addition
   to wall time) in both query log file (for every given query) and
   status report (aggregated). It depends on clock_gettime() system
   call and might therefore be unavailable on certain systems. You might
   start ``searchd`` thus:

   .. code-block:: bash


       $ searchd --config /home/myuser/sphinx.conf --cpustats

-  ``--port portnumber`` (``-p`` for short) is used to specify the port
   that ``searchd`` should listen on, usually for debugging purposes.
   This will usually default to 9312, but sometimes you need to run it
   on a different port. Specifying it on the command line will override
   anything specified in the configuration file. The valid range is 0 to
   65535, but ports numbered 1024 and below usually require a privileged
   account in order to run. An example of usage:

   .. code-block:: bash


       $ searchd --port 9313

-  ``--listen ( address ":" port | port | path ) [ ":" protocol ]``
   (or ``-l`` for short) Works as ``--port``, but allow you to specify
   not only the port, but full path, as IP address and port, or
   Unix-domain socket path, that ``searchd`` will listen on. Otherwords,
   you can specify either an IP address (or hostname) and port number,
   or just a port number, or Unix socket path. If you specify port
   number but not the address, searchd will listen on all network
   interfaces. Unix path is identified by a leading slash. As the last
   param you can also specify a protocol handler (listener) to be used
   for connections on this socket. Supported protocol values are
   ‘sphinx’ and ‘mysql41’ (MySQL protocol used since 4.1 upto at least
   5.1).

-  ``--index <index>`` (or ``-i <index>`` for short) forces
   this instance of ``searchd`` only to serve the specified index. Like
   ``--port``, above, this is usually for debugging purposes; more
   long-term changes would generally be applied to the configuration
   file itself. Example usage:

   .. code-block:: bash


       $ searchd --index myindex

-  ``--strip-path`` strips the path names from all the file names
   referenced from the index (stopwords, wordforms, exceptions, etc).
   This is useful for picking up indexes built on another machine with
   possibly different path layouts.

-  ``--replay-flags=<OPTIONS>`` switch can be used to specify a
   list of extra binary log replay options. The supported options are:

   -  ``accept-desc-timestamp``, ignore descending transaction
      timestamps and replay such transactions anyway (the default
      behavior is to exit with an error).

   Example:

   .. code-block:: bash


       $ searchd --replay-flags=accept-desc-timestamp

There are some options for ``searchd`` that are specific to Windows
platforms, concerning handling as a service, are only be available on
Windows binaries.

Note that on Windows searchd will default to ``--console`` mode, unless
you install it as a service.

-  ``--install`` installs ``searchd`` as a service into the Microsoft
   Management Console (Control Panel / Administrative Tools / Services).
   Any other parameters specified on the command line, where
   ``--install`` is specified will also become part of the command line
   on future starts of the service. For example, as part of calling
   ``searchd``, you will likely also need to specify the configuration
   file with ``--config``, and you would do that as well as specifying
   ``--install``. Once called, the usual start/stop facilities will
   become available via the management console, so any methods you could
   use for starting, stopping and restarting services would also apply
   to ``searchd``. Example:

   .. code-block:: bat


       C:\WINDOWS\system32> C:\Manticore\bin\searchd.exe --install
          --config C:\Manticore\sphinx.conf

   If you wanted to have the I/O stats every time you started
   ``searchd``, you would specify its option on the same line as the
   ``--install`` command thus:

   .. code-block:: bat


       C:\WINDOWS\system32> C:\Manticore\bin\searchd.exe --install
          --config C:\Manticore\sphinx.conf --iostats

-  ``--delete`` removes the service from the Microsoft Management
   Console and other places where services are registered, after
   previously installed with ``--install``. Note, this does not
   uninstall the software or delete the indexes. It means the service
   will not be called from the services systems, and will not be started
   on the machine's next start. If currently running as a service, the
   current instance will not be terminated (until the next reboot, or
   ``searchd`` is called with ``--stop``). If the service was installed
   with a custom name (with ``--servicename``), the same name will need
   to be specified with ``--servicename`` when calling to uninstall.
   Example:

   .. code-block:: bat


       C:\WINDOWS\system32> C:\Manticore\bin\searchd.exe --delete

-  ``--servicename <name>`` applies the given name to ``searchd``
   when installing or deleting the service, as would appear in the
   Management Console; this will default to searchd, but if being
   deployed on servers where multiple administrators may log into the
   system, or a system with multiple ``searchd`` instances, a more
   descriptive name may be applicable. Note that unless combined with
   ``--install`` or ``--delete``, this option does not do anything.
   Example:

   .. code-block:: bat


       C:\WINDOWS\system32> C:\Manticore\bin\searchd.exe --install
          --config C:\Manticore\sphinx.conf --servicename ManticoreSearch

-  ``--ntservice`` is the option that is passed by the Management
   Console to ``searchd`` to invoke it as a service on Windows
   platforms. It would not normally be necessary to call this directly;
   this would normally be called by Windows when the service would be
   started, although if you wanted to call this as a regular service
   from the command-line (as the complement to ``--console``) you could
   do so in theory.

-  ``--safetrace`` forces ``searchd`` to only use system backtrace()
   call in crash reports. In certain (rare) scenarios, this might be a
   “safer” way to get that report. This is a debugging option.

-  ``--nodetach`` switch (Linux only) tells ``searchd`` not to detach
   into background. This will also cause log entry to be printed out to
   console. Query processing operates as usual. This is a debugging
   option.

Last but not least, as every other daemon, ``searchd`` supports a number
of signals.

-  SIGTERM
-  Initiates a clean shutdown. New queries will not be handled; but
   queries that are already started will not be forcibly interrupted.

-  SIGHUP
-  Initiates index rotation. Depending on the value of
   :ref:`seamless_rotate <seamless_rotate>`
   setting, new queries might be shortly stalled; clients will receive
   temporary errors.

-  SIGUSR1
-  Forces reopen of searchd log and query log files, letting you
   implement log file rotation.
