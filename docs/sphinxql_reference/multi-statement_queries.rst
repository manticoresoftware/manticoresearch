.. _multi-statement_queries:

Multi-statement queries
-----------------------

SphinxQL supports multi-statement queries, or batches. Possible
inter-statement optimizations described in :ref:`multi_queries` do apply to SphinxQL just as
well. The batched queries should be separated by a semicolon. Your MySQL
client library needs to support MySQL multi-query mechanism and multiple
result set. For instance, mysqli interface in PHP and DBI/DBD libraries
in Perl are known to work.

Here's a PHP sample showing how to utilize mysqli interface with Manticore.

::


    <?php

    $link = mysqli_connect ( "127.0.0.1", "root", "", "", 9306 );
    if ( mysqli_connect_errno() )
        die ( "connect failed: " . mysqli_connect_error() );

    $batch = "SELECT * FROM test1 ORDER BY group_id ASC;";
    $batch .= "SELECT * FROM test1 ORDER BY group_id DESC";

    if ( !mysqli_multi_query ( $link, $batch ) )
        die ( "query failed" );

    do
    {
        // fetch and print result set
        if ( $result = mysqli_store_result($link) )
        {
            while ( $row = mysqli_fetch_row($result) )
                printf ( "id=%s\n", $row[0] );
            mysqli_free_result($result);
        }

        // print divider
        if ( mysqli_more_results($link) )
            printf ( "------\n" );

    } while ( mysqli_next_result($link) );

Its output with the sample ``test1`` index included with Manticore is as
follows.

::


    $ php test_multi.php
    id=1
    id=2
    id=3
    id=4
    ------
    id=3
    id=4
    id=1
    id=2

The following statements can currently be used in a batch: SELECT, SHOW
WARNINGS, SHOW STATUS, and SHOW META. Arbitrary sequence of these
statements are allowed. The results sets returned should match those
that would be returned if the batched queries were sent one by one.
