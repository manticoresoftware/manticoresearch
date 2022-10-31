<?php

ini_set ( "display_errors", 1 );
ini_set ( "error_reporting", (E_ALL | E_STRICT) & ~E_USER_DEPRECATED );

require_once ( $g_locals['api'] );

function MyMicrotime ()
{
	$q = @gettimeofday();
	return (float)($q["usec"] / 1000000) + $q["sec"];
}

function sphFormatTime ( $time )
{
	if ( $time < 60 )
		return sprintf ( '%.0fs', $time );

	$time = (int)$time;
	if ( $time < 3600 )
		$u = array ( 'm', 's' );
	else
	{
		$time = $time / 60;
		$u = array ( 'h', 'm' );
	}
	return sprintf ( '%d%s:%d%s', $time / 60, $u[0], $time % 60, $u[1] );
}

function StrBegins ( $str, $substr )
{
	return substr($str, 0, strlen($substr)) === $substr;
}


function mysqli_wr ($q, $conn)
{
//	printf ( "$q\n");
	return @$conn->query ( $q );
}

// return line for text formatting of the table, as +----+---+
function PrintLine($columns)
{
    $str = "+";
    foreach ( $columns as $sz )
        $str .= str_pad("",  $sz+2, '-') . "+";
    $str .= "\n";
    return $str;
}

// return formatted line padded by fixed length, like: | foo | bar baz |
function PrintDataLine($row, $columns)
{
    $str = "|";

    foreach ($row as $key => $value) {
        if (is_null($value))
            $value = 'NULL';
        $str .= " " . str_pad ($value, $columns[$key]+1) . "|";
    }
    $str .= "\n";
    return $str;
}

// format $rows dataset as mysql cli does
function WriteSphinxqlTable ( $rows )
{
    $str = "";
    $columns=[];
    foreach ( $rows[0] as $key=>$s )
        $columns[$key] = strlen($key);

    // collect max lengths
    foreach ($rows as $row) {
        foreach ($row as $key => $value) {
            if (is_null($value))
                $value = 'NULL';
            $columns[$key] = max($columns[$key], strlen($value));
        }
    }

    // top line
    $str .= PrintLine ($columns);

    // column names
    $str .= "|";
    foreach ( $rows[0] as $key=>$s )
        $str .= " " . str_pad ($key, $columns[$key]+1) . "|";
    $str .= "\n";

    // 2-nd line
    $str .= PrintLine ($columns);

    // rowset
    foreach ($rows as $row)
        $str .= PrintDataLine ($row, $columns);

    // tail line
    $str .= PrintLine ($columns);
    return $str;
}

// write single mysql query/result as example
function WriteSphinxqlExample ( $log, $result )
{
    $str = "<!-- request mysql -->\n\n```\n$result[sphinxql];\n```\n\n";

    $str .= "<!-- response mysql -->\n\n```\n";

    if ( array_key_exists ("total_affected", $result) )
    {
        $str .= "Query OK, $result[total_affected] rows affected (0.00 sec)\n\n";

    } else if ( array_key_exists ("error", $result) )
    {
        $str .= "ERROR $result[errno]: $result[error]\n\n";

    } else if (array_key_exists ("rows", $result) )
    {
        $str .= WriteSphinxqlTable ($result["rows"]);
        $str .="$result[total_rows] rows in set (0.00 sec)\n\n";

    } else if ( isset($result["total_rows"]) )
    {
        $str .= "$result[total_rows] rows in set (0.00 sec)\n\n";
    }

    $str .= "```\n";
    fwrite ( $log, $str);
}

// writes (append) whole set of examples to provided file.
function WriteExamples ( $example_file, $examples )
{
    if (empty($examples))
        return;

    $log = fopen ( $example_file, "a" );

    foreach ( $examples as $name=>$results )
    {
        fwrite ( $log, "\n<!-- example $name -->\n");

        foreach ( $results as $result )
        {
            if ( array_key_exists ( "sphinxql", $result ) )
                WriteSphinxqlExample ( $log, $result);
            // else - other types
        }

        fwrite ( $log, "<!-- end -->\n");
    }

    fclose ( $log );
}

$gl_conn = false;

// The wrappers for fresh php without php_mysql.
// We use mysqli instead in the code here,
// but custom tests still know nothing about it,
// so make old-fashion wrappers for them.
if ( !function_exists("mysql_query") ) {

	function mysql_query($query, $conn = NULL)
	{
		global $gl_conn;

		if ($conn == NULL)
			return $gl_conn->query($query);
		else
			return $conn->query($query);
	}

	function mysql_connect($host, $user, $password, $newlink)
	{
		global $gl_conn;
		$conndetails = explode(":",$host);
		if ( count($conndetails)==1 )
			$gl_conn = new mysqli($host,$user,$password);
		else
		{
			global $g_locals;
			$dbname = $g_locals["db-name"];
			$host = $conndetails[0];
			$port = $conndetails[1];
			$gl_conn = new mysqli($host, $user, $password, $dbname, (int)$port);
		}
		if ($gl_conn->connect_error)
			return false;
		return $gl_conn;
	}

	function mysql_select_db($db, $conn=NULL)
	{
		global $gl_conn;

		if ( $conn == NULL )
			return $gl_conn->select_db($db);
		else
			return $conn->select_db($db);
	}

	function mysql_close ( $link=NULL )
	{
		if ($link!=NULL) {
			$link->close();
			return;
		}

		global $gl_conn;
		if ($gl_conn!=NULL)
			$gl_conn->close();
	}

	function mysql_errno ( $link=NULL )
	{
		global $gl_conn;
		if ($link)
			return $link->errno;
		else
			return $gl_conn->errno;
	}

	function mysql_error ( $link=NULL )
	{
		global $gl_conn;
		if ($link)
			return $link->error;
		else
			return $gl_conn->error;
	}

	function mysql_affected_rows($conn = NULL)
	{
		if ($conn != NULL)
			return $conn->affected_rows;

		global $gl_conn;
		return $gl_conn->affected_rows;
	}

	function mysql_num_rows($res)
	{
		return $res->num_rows;
	}

	define('MYSQL_ASSOC', MYSQLI_ASSOC);
	define('MYSQL_NUM', MYSQLI_NUM);
	define('MYSQL_BOTH', MYSQLI_BOTH);

	function mysql_fetch_array($res,$restype=MYSQL_BOTH)
	{
		if ($res===true)
			return false;

		$result = $res->fetch_array($restype);
		if ($result===NULL)
			return false;

		return $result;
	}

	function mysql_fetch_assoc($res)
	{
		return $res->fetch_assoc();
	}

	function mysql_free_result($res)
	{
		if ($res===true)
			return;
		$res->free();
	}
	$mysql_simulated = true;
} // function_exists ("mysql_query")
 else {
	$mysql_simulated = false;
 }

// will use mysql_query from php, or our redefined
function legacy_mysql_wr ($q, $conn)
{
//	printf ( "$q\n");
	return @mysql_query($q, $conn);
}

function ConnectDB ()
{
	global $g_locals;

	if ( !function_exists ( "mysqli_connect" ) )
	{
		print ( "ERROR: missing required mysqli_connect(); add php_mysqli.so (.dll on Windows) to your php.ini!\n" );
		exit ( 1 );
	}

	$conn = new mysqli(
		$g_locals["db-host"],
		$g_locals["db-user"],
		$g_locals["db-password"],
		$g_locals['db-name'],
		$g_locals['db-port']);

	return $conn;
}

// will use either system mysql, either our wrapper in order to be in sync with client used in custom tests.
function LegacyConnectDB ()
{
	global $g_locals;

	$conn = @mysql_connect (
			$g_locals["db-host"] . ":" . $g_locals["db-port"],
			$g_locals["db-user"],
			$g_locals["db-password"],
			true );

	if ( $conn === false ||
			!@mysql_query ( "CREATE DATABASE IF NOT EXISTS " . $g_locals['db-name'], $conn ) ||
			!@mysql_select_db ( $g_locals['db-name'], $conn ) )
		return false;

	return $conn;
}

function LegacyCreateDB ( $db_drop, $db_create, $db_insert, $custom_insert, &$error )
{
	$conn = LegacyConnectDB();

	foreach ( $db_drop as $q )
		if ( !legacy_mysql_wr ( $q, $conn ) )
		{
            $error = mysqli_error($conn);
            return false;
		}

	foreach ( $db_create as $q )
	{
		if ( stripos ( $q, "create table")!==false )
		{
			if ( stripos ( $q, "engine=")===false )
			{
				$q = trim ( $q, " \t\n\r;" );
				$q .= " ENGINE=MEMORY";
			}
		}

		if ( !legacy_mysql_wr ( $q, $conn ) )
        {
            $error = mysqli_error($conn);
            return false;
        }
	}

	$oneok = count($db_insert)==0;
	foreach ( $db_insert as $q )
		if ( legacy_mysql_wr ( $q, $conn ) )
			$oneok = true;

	if ( !$oneok )
		return false;

	foreach ( $custom_insert as $code )
	{
		$func = function() use ($code) { eval ("$code"); };
		$func();
	}

	return $conn;
}

function CreateDB ( $db_drop, $db_create, $db_insert, $custom_insert, $skip, &$error )
{
	if ($skip)
		return ConnectDB();

	// here we split codepaths for old (php5) and new (php7 without php_mysql).
	// if there is no custom inserts, we just use mysqli.
	// but if we have them, we need to support mysql syntax since custom inserts use it.
	// This is done by LegacyConnectDB.After it, it we actually doesn't have mysql (i.e. if we
	// use wrappers) we just continue to use the connection as mysqli.
	// but otherwise we have to close mysql conn and reconnect as mysqli to avoid too many changes
	// of the existing code.
	global $mysql_simulated;
	if ( count($custom_insert)>0 )
	{
		$conn = LegacyCreateDB ( $db_drop, $db_create, $db_insert, $custom_insert, $error );
		if ( !$mysql_simulated )
		{
			@mysql_close($conn);
			$conn = ConnectDB();
		}
		return $conn;
	}
	$conn = ConnectDB();


	foreach ( $db_drop as $q )
		if ( !mysqli_wr ( $q, $conn ) )
		{
            $error = mysqli_error($conn);
            return false;
		}

	foreach ( $db_create as $q )
	{
		if ( stripos ( $q, "create table")!==false )
		{
			if ( stripos ( $q, "engine=")===false )
			{
				$q = trim ( $q, " \t\n\r;" );
				$q .= " ENGINE=MEMORY";
			}
		}
		if ( !mysqli_wr ( $q, $conn ) )
        {
            $error = mysqli_error($conn);
            return false;
        }
	}

	$oneok = count($db_insert)==0;
	foreach ( $db_insert as $q )
		if ( mysqli_wr ( $q, $conn ) )
			$oneok = true;

	if ( !$oneok )
		return false;

	return $conn;
}


function RunIndexerOnce ( &$error, $params )
{
	global $g_locals;

	$path = $g_locals['indexer'];

	if ( !is_executable($path) )
	{
		$error = "$path: indexer not found";
		return 1;
	}

	$retval = 0;
	exec ( "$path --quiet --config ".testdir('config.conf')." $params", $error, $retval );

	$error = join ( "\n", $error );
	return ( $retval==0 && !empty($error) ) ? 2 : $retval;
}

function RunIndexer ( &$error, $params )
{
	$INDEXING_TRIES = 5;
	$INDEXING_TICK = 1000000; // msec

	$tries = 0;

	for ( $i=0; $i<$INDEXING_TRIES; ++$i )
	{
		$retval = RunIndexerOnce ( $error, $params );
		if ( empty($error) || stripos ( $error, "failed to lock")===false )
			break;
		usleep ( $INDEXING_TICK );
		++$tries;
	}
	if ( $tries!=0 )
		$error = "After $tries tries: $error";
	return $retval;
}


function CheckSearchdLog ( $error_file, &$retval )
{
	$rawlog = file ( $error_file );
	$error = "";

	foreach ( $rawlog as $line )
	{
		foreach ( array ( "WARNING:", "ERROR:", "FATAL:" ) as $tag )
		{
			$t = strstr ( $line, $tag );
			if ( $t )
			{
				$error .= $t;
				if ( $tag!="WARNING:" )
					$retval = 1;
			}
		}
	}

	return $error;
}

function UseValgrind ()
{
	global $g_locals, $VLG;
	if ( $VLG )
		return true;

	if ( isset ( $g_locals['valgrindsearchd'] ) )
		return  $g_locals['valgrindsearchd'];
	else
		return false;
}

function ActionRetries ()
{
	global $action_retries, $valgrind_action_retries;
	if (!UseValgrind() )
		return $action_retries;

	return $action_retries + $valgrind_action_retries;
}

function WaitRetries ()
{
	return ActionRetries()*50;
}

function GetSearchd ( &$searchd, &$error, $full_featured )
{
	global $g_locals, $VLG;

	if ( isset ( $g_locals['valgrindoptions'] ) )
		$vlgoptions = $g_locals['valgrindoptions'];
	else
		$vlgoptions = '--leak-check=full';

	if ( $full_featured && UseValgrind() ) {
		if ( $VLG )
		{
            $vlgvars = explode ( ' ', $VLG );
            foreach ( $vlgvars as &$vlgvar )
            {
                if ( substr ( $vlgvar, 0, 10 ) == "--log-file" ) # --log-file=/sphinx/sphinxfrommac/build/Testing/Temporary/MemoryChecker.308.log
                {
                    $lgfile = explode ('=', $vlgvar)[1];
                    $vlgvar="--log-fd=9 9>>$lgfile";
                    break;
                }
            }
            $VLG1 = implode (' ', $vlgvars);
			$searchd = "$VLG1 ";
		}
		else
		{
			$searchd = "valgrind $vlgoptions ";
			if ( file_exists("valgrind.supp" ) )
				$searchd .= "--suppressions=" . getcwd() . "/valgrind.supp ";
		}
		$searchd .= $g_locals['searchd'];
	}
	else
	{
		$searchd = $g_locals['searchd'];
		if ( !is_executable($searchd) )
		{
			$error = "$searchd: searchd not found";
			return false;
		}
	}
	return true;
}

function GetTmDelta()
{
	if ( UseValgrind() )
		return 30;
	return 10;
}

function StartSearchd ( $config_file, $error_file, $pidfile, &$error, $requirements, $addr=false, $port=false )
{
	global $g_locals, $windows, $cygwin, $action_wait_timeout, $sd_address, $sd_port;


	$abs_config_file = testdir($config_file);
	$abs_error_file = scriptdir($error_file);
	$win_error_file = testdir($error_file);
	$abs_pidfile = scriptdir($pidfile);

//	print ($abs_config_file . "\n");
//	print ($abs_error_file . "\n");
//	print ($abs_pidfile . "\n");

	if (!GetSearchd ($path, $error, true))
		return 1;

	// try to wait a bit here
	$action_retries = ActionRetries();
	$i = 0;
	while ( !@touch($abs_error_file) && $i < $action_retries )
	{
		usleep ( $action_wait_timeout );
		$i++;
	}
	if ( !@touch($abs_error_file) )
	{
		$error = "$abs_error_file: unable to create error file";
		return 1;
	}

	$extra_params = '';
	$use_watchdog = isset ($requirements["watchdog"]) && $requirements["watchdog"];
	$no_pseudo_sharding = isset ($requirements["no_pseudo_sharding"]) && $requirements["no_pseudo_sharding"];

	if ( !$use_watchdog )
		$extra_params .= ' --test';

	if ( !$no_pseudo_sharding )
		$extra_params .= ' --force-pseudo-sharding';

	if ( isset ( $g_locals['use_pool'] ) && $g_locals['use_pool'] )
		$extra_params .= ' --test-thd-pool';

	if ( isset ( $g_locals['extra_searchd_options'] ) )
		$extra_params .= ' ' . $g_locals['extra_searchd_options'];

	$testdir = $g_locals['scriptdir'];
	$cd = ( $testdir!='' );
	if ($cd) {
		$backdir = getcwd();
		chdir($testdir);
	}

	$retval = 0;
	if ( $windows && !$cygwin )
	{
		// using start /min to fire it "in background"
		// using cmd /c for redirection to work
		if ( file_exists ( $abs_pidfile ) )
			unlink ( $abs_pidfile );
//		print ("start /min cmd /c \"$path --config $abs_config_file --pidfile --console $extra_params > $abs_error_file\"\n");
		$process = popen ("start /min cmd /c \"$path --config $abs_config_file --pidfile --console $extra_params > $abs_error_file\"", "r" );
		pclose ( $process );
	} else if ($cygwin)
	{
		// using start /min to fire it "in background"
		// using cmd /c for redirection to work
		if ( file_exists ( $abs_pidfile ) )
			unlink ( $abs_pidfile );
		$scmd = "cygstart --hide bash -c \"$path --config $abs_config_file --pidfile --console $extra_params > $abs_error_file\"";
		print ("cygwin: $scmd\n");
		$process = popen ($scmd, "r" );
		pclose ( $process );
		print ("started\n");
	} else
	{
//		print  ( "$path --config $abs_config_file $extra_params > $abs_error_file\n" );
		system ( "$path --config $abs_config_file $extra_params > $abs_error_file", $retval );
//		print ("started\n");
	}

	if ($cd)
		chdir($backdir);

	$action_retries = ActionRetries();

	// wait until pid appears
	for ( $i=0; $i<$action_retries && !file_exists($abs_pidfile); $i++ )
		usleep ( $action_wait_timeout );

	if ( !file_exists($abs_pidfile) )
	{
		$error = "PID file ($abs_pidfile) was not created";
		return 1;
	}

	// check for early crash
	$error = CheckSearchdLog ( $abs_error_file, $retval );

	// on windows, searchd starts *fully* async
	// so lets also wait until pidfile gets real data
	// (meaning that index precaching is actually done)

	$STARTUP_TRIES = WaitRetries();
	if ( $retval!=1 && $windows )
	{
		$STARTUP_TICK = 50000; // msec

		// FIXME! add a better check that searchd is still alive than just file_exists
		for ( $i=0; $i<$STARTUP_TRIES && file_exists($abs_pidfile); $i++ )
		{
			$pid = @file($abs_pidfile);
			if ( count($pid) )
				break;
			usleep ( $STARTUP_TICK );
		}
	}

	// lets wait when daemon is ready to accept connections
	if ( $retval==0 )
	{
		if ( !$addr )
			$addr = $sd_address;
		if ( !$port )
			$port = $sd_port;

		$cl = new SphinxClient;
		$cl->SetServer ( $addr, $port );	
		$cl->SetConnectTimeout ( 10 );
		
		$ok = false;
		$start = MyMicrotime();		
		for ( $i=0; $i<$STARTUP_TRIES; $i++ )
		{
			if ( $cl->Open() )
			{
				$cl->Close(); 
				$ok = true;
				break;
			}
			usleep ( 500 );
		}
		
		if ( !$ok )
		{
			$tm = ( MyMicrotime() - $start );
			printf ( "\nWARNING: can't connect to daemon on startup for %.3f sec\t\t\t\n", $tm );
		}
	}

	if ( $retval==0 && !empty($error) )
		$retval = 2; // no errors, but there were warnings

	return $retval;
}


function StopSearchd ( $config, $pidfile )
{
	global $action_wait_timeout;
	$ret = 0;
	$abs_config = testdir($config);
	$abs_pidfile = testdir($pidfile);
	$action_retries = ActionRetries();
	if ( file_exists($abs_pidfile) && count(file($abs_pidfile)) )
	{
		GetSearchd ($path, $error, false);
		$dummy = array();
		exec ( "$path --config $abs_config --stopwait", $dummy, $ret );

		$i = 0;
		while ( file_exists ( $abs_pidfile ) && $i < $action_retries )
		{
			usleep ( $action_wait_timeout );
			$i++;
		}
		// finally kill, since otherwise stacked daemon will ruine following tests
		if ( file_exists ( $abs_pidfile ) )
			KillSearchd($config, $pidfile, 'KILL', true);
	}
	return $ret;
}

function StopWaitSearchd ( $config, $pidfile )
{
	return StopSearchd ( $config, $pidfile );
}

function RestartDaemon ($no_warnings=false)
{
	$stop = StopWaitSearchd ('config.conf', 'searchd.pid');
	$status = 'stop=' . ( $stop==0 ? 'ok' : 'error' ) . ', return code=' . $stop;

	$start = StartSearchd ( 'config.conf', 'error.txt', 'searchd.pid', $error, array() );
	if ( $start == 0 || $start == 2 )
	{
	    if ($no_warnings)
	        $start = 0;
		$status .= "; start=ok" . ', return code=' . $start;
        if ( $start==2 )
            $status .= ", error=" . $error;
	} else
	{
		$status .=  "; start=failed, return code=" . $start . ", error=" . $error;
	}
	return $status;
}

function KillSearchd ( $config, $pidfile, $signal, $unlinkpid=True )
{
	global $action_wait_timeout, $windows;

	$abs_pidfile = testdir($pidfile);
	
	if ( file_exists($abs_pidfile) && count(file($abs_pidfile)) )
	{
		$fp = fopen($abs_pidfile,"r");
		$pid = fread ( $fp, filesize ( $abs_pidfile ) );
		fclose ($fp);

		if ( $windows ) {
		    if ( $signal=='KILL')
                exec ("taskkill /F /PID $pid");
		    else
                exec("kill -f -s $signal $pid");
        }
		else
			exec ("kill -s $signal $pid");

		if ( $unlinkpid && file_exists ( $abs_pidfile ) )
		{
			usleep ( $action_wait_timeout );
			unlink ( $abs_pidfile );
		}
	}
}

function IsModelGenMode ()
{
	global $g_model;
	return $g_model;
}

function RandomWords ( $n, $seed )
{
	srand ( $seed );
	$words = [];
	for ( $k=0; $k<$n; $k++ )
		$words[] = substr ( str_shuffle ( 'abcdefghijklmnopqrstuvwxyz' ), 0, 5 );
	return join ( " ", $words );
}

function RoundFloatValues ( &$values, $roundoff )
{
	if ( !is_array ( $values ) )
		return;

	foreach ( $values as &$value )
	{
		if ( is_float ( $value ) )
			$value = round ( $value, $roundoff );

		if ( is_array ( $value ) )
			RoundFloatValues ( $value, $roundoff );
	}
}


function IsColumnarProperty ( $property )
{
	return $property=="columnar" || $property=="columnar fast_fetch" || $property=="columnar stored";
}


function CompareColumns ( $a, $b )
{
	return strcmp ( array_keys($a)[0], array_keys($b)[0] );
}


function JsonRsetFixup ( &$set, $columnar, $keep_json_ctrls )
{
	if ( isset ( $set["rows"] ) )
	{
		if ( $keep_json_ctrls )
		{
			$json_handler = new SafeJsonHandler();
			$rows = $json_handler->Decode ( $set["rows"], 1 );	
		}
		else
			$rows = json_decode ( $set["rows"], 1 );
		if ( isset ( $rows["hits"] ) && isset ( $rows["hits"]["hits"] ) )
		{
			foreach ( $rows["hits"]["hits"] as &$hit )
			{
				RoundFloatValues ( $hit, 6 );
				if ( isset ( $hit["_source"] ) )
					ksort ( $hit["_source"] );
			}
		}

		$set["rows"] = $keep_json_ctrls ? $json_handler->Encode ( $rows ) : json_encode ( $rows );
	}

	if ( isset ( $set["http_endpoint"] ) && ( $set["http_endpoint"]=="sql" || $set["http_endpoint"]=="cli" ) && isset ( $set["rows"] ) )
	{
		if ( $keep_json_ctrls )
		{
			$json_handler = new SafeJsonHandler();
			$rows = $json_handler->Decode ( $set["rows"], 1 );	
		}
		else
			$rows = json_decode ( $set["rows"], 1 );
		foreach	( $rows as &$row )
		{
			if ( isset ( $row["data"] ) )
			{
				foreach ( $row["data"] as &$entry )
				{
					if ( is_array($entry) && isset ( $entry["Properties"] ) && $columnar && IsColumnarProperty ( $entry["Properties"] ) )
						$entry["Properties"] = "";
				}
			}

			if ( isset ( $row["data"] ) && is_array($row["data"]) )
			{
				foreach ( $row["data"] as &$data_row )
					ksort($data_row);
			}

			if ( isset ( $row["columns"] ) && is_array($row["columns"]) )
				usort ( $row["columns"], "CompareColumns" );
		}

		$set["rows"] = $keep_json_ctrls ? $json_handler->Encode ( $rows ) : json_encode ( $rows );
	}
}


function SqlRsetFixup ( &$set, $columnar )
{
	if ( !isset ( $set["rows"] ) )
		return;

	foreach	( $set["rows"] as &$row )
	{
		if ( count($row)!=3 )
			continue;

		$keys = array_keys($row);
		if ( count($keys)!=3 )
			return;

		if ( $keys[0]!='Field' || $keys[1]!='Type' || $keys[2]!='Properties' )
			return;

		if ( $columnar && IsColumnarProperty ( $row['Properties'] ) )
			$row['Properties'] = '';
	}
}


function CompareResultSetFixup ( &$set, $roundoff, $variants_match, $columnar, $keep_json_ctrls=false )
{
	global $g_ignore_weights;

	if ( !is_array($set) )
		return;

	if ( $roundoff && !@$set["resarray"] ) // FIXME! support resarray too
		foreach ( $set["attrs"] as $name=>$type )
			if ( $type==SPH_ATTR_FLOAT )
			{
				foreach ( $set["matches"] as $id=>$match )
					$set["matches"][$id]["attrs"][$name] = sprintf ( "%.{$roundoff}f", $set["matches"][$id]["attrs"][$name] );
			}

	// fixup floats in json via http
	if ( isset ( $set["http"] ) && $set["http"]==1 )
		JsonRsetFixup ( $set, $columnar, $keep_json_ctrls );
	else
		SqlRsetFixup ( $set, $columnar );

    // fixup sphinxql trim
    if ( isset ( $set['sphinxql'] ) )
        $set['sphinxql'] = trim ($set['sphinxql']);

    // fixup http_request trim
    if ( isset ( $set['http_request'] ) )
        $set['http_request'] = trim ($set['http_request']);

	if ( $g_ignore_weights )
	{
		if ( isset($set["matches"]) )
		{
			if ( @$set["resarray"] )
			{
				for ( $i=0; $i<count($set); $i++ )
					unset ( $set["matches"][$i]["weight"] );
			} else
			{
				foreach ( $set["matches"] as $id=>$match )
					unset ( $set["matches"][$id]["weight"] );
			}
		}
		if ( @$set["words"] )
			foreach ( $set["words"] as $word=>$info )
				$set["words"][$word] = array ( "hits"=>-1, "docs"=>-1 );
		if ( isset($set["sphinxql"]) && isset($set["rows"]) )
		{
			for ( $i=0; $i<count($set["rows"]); $i++ )
				unset($set["rows"][$i]["weight()"]);
		}
	}

	//foreach ( preg_split ( "/\\W+/", "time warning status fields resarray roundoff words" ) as $key )
	foreach ( preg_split ( "/\\W+/", "time warning status fields resarray roundoff" ) as $key )
		unset ( $set[$key] );

	// variants are be used to check mva32/mva64, int/bigint, etc
	// thus in variants check we fixup the compatible attribute types and only check the data
	if ( $variants_match && isset ( $set["attrs"] ) )
	{
		foreach ( $set["attrs"] as $k=>$v )
		{
			if ( $v==SPH_ATTR_MULTI64 )
				$set["attrs"][$k] = SPH_ATTR_MULTI; 
			if ( $v==SPH_ATTR_BIGINT && $k[0]=="@" )
				$set["attrs"][$k] = SPH_ATTR_INTEGER;
		}
	}
}

	
function ChildrenArray ( $node, $name="" )
{
	$res = array ();
	if ( !empty($node) && $node->hasChildNodes() )
		for ( $i=0; $i<$node->childNodes->length; $i++ )
	{
		$child = $node->childNodes->item ( $i );
		if ( $name=="" || strtolower($child->nodeName)==$name )
			$res[] = $child;
	}
	return $res;
}

function NextState ( &$iter, &$limits, $ps )
{
	if ( $ps>count($limits) )
		return;
	++$iter[$ps];
	if ( $iter[$ps]>=$limits[$ps])
	{
		$iter[$ps]=0;
		NextState ($iter,$limits,$ps+1);
	}
}

function AttrArray ( $node, $name="" )
{
	$res = array ();
	if ( !empty($node) && $node->hasAttributes() )
		for ( $i=0; $i<$node->attributes->length; $i++ )
		{
			$child = $node->attributes->item ( $i );
			if ( $name=="" || strtolower($child->nodeName)==$name )
				$res[] = $child;
		}
	return $res;
}

function GetfirstAttr ( $node )
{
	if ( !empty($node) && $node->hasAttributes() )
		return $node->attributes->item (0);
	return NULL;
}


function GetFirstChild ( $node, $name )
{
	$children = ChildrenArray ( $node, $name );
	return empty($children) ? NULL : $children[0];
}


function GetFirstChildValue ( $node, $name, $default="" )
{
	$child = GetFirstChild ( $node, $name );
	return is_null($child) ? $default : $child->nodeValue;
}

function ArrVal ( $var, $name, $default="" )
{
    if ( array_key_exists ( $name, $var ) )
        return $var[$name];
    return $default;
}

function ArrVals ( $var, $name, $delimiter=" " )
{
    if ( array_key_exists( $name, $var) )
        return explode($delimiter, $var[$name]);
    else
        return array();
}

function TouchVal ( &$var, $name, $default="" )
{
    if ( !array_key_exists ( $name, $var ) )
        $var[$name] = $default;
}

function ConnectSpecificQL($agent, $vip=false)
{
	global $agents;

	$address = $agents[$agent]["address"];
	if ( $vip )
		$port = $agents[$agent]["sqlport_vip"];
	else
		$port = $agents[$agent]["sqlport"];

//	echo "Connecting to agent $address:$port\n";
	if ($address == "localhost")
		$address = "127.0.0.1";
	return @mysqli_connect ( $address, '', '', '', $port );
}

function ConnectQL()
{
	return ConnectSpecificQL(0);
}


class QLClient
{
	private $_conn = false;

	function Disconnect()
	{
		if ($this->_conn!==false)
			@mysqli_close($this->_conn);
	}

	function Connect()
	{
		$this->_conn = ConnectQL();
		return $this->_conn!==false;
	}

	function Reconnect()
	{
		$this->Disconnect();
		return $this->Connect();
	}

	function Query($q)
	{
		if ($this->_conn===false)
			return "NOT CONNECTED";

		$r = @mysqli_query($this->_conn, $q);
		if (!$r)
			return "ERROR: ".mysqli_error($this->_conn);

		if ( $r===true )
			return "OK";
			
		$n = 0;
		$res = "";
		while ($row = mysqli_fetch_row($r))
		{
			$res .= join(" | ", $row) . "\n";
			$n++;
		}
		$res .= "$n rows";
		return $res;
	}
	
	function SetConnection($conn)
	{
		$this->_conn = $conn;
	}
}

class APIClient extends SphinxClient
{
	function XQuery ( $query, $index="*", $comment="" )
	{
		$res = $this->Query($query, $index, $comment);
		if ($res===false)
			return $this->GetLastError();
		unset($res["time"]);
		return $res;
	}

	function XUpdateAttributes ( $index, $attrs, $values, $type=SPH_UPDATE_INT, $ignorenonexistent=false )
	{
		$res = $this->UpdateAttributes ( $index, $attrs, $values, $type, $ignorenonexistent );
		if ($res===-1)
			return $this->GetLastError();
		return $res;
	}
}

function FormatJsonRow ( $j )
{
	if ( !is_array ( $j ) ||  !count($j) )
	{
		return "[]\n";
	}
	$str = "[\n";

	$row_sep = "";
	foreach ( $j as $row )
	{
		$str .= "$row_sep\t";
		$str .= json_encode ( $row, JSON_UNESCAPED_SLASHES | JSON_UNESCAPED_UNICODE );
		$row_sep = ",\n";
	}
	$str .= "]";
	return $str;
}

// avoiding json errors in case json being processed contains control characters
class SafeJsonHandler 
{
	private $replaces = [];

	function Decode ( $json, $assoc=false, $depth=512, $flags=0 )
    {
    	$this->replaces = [ "from" => '%', "to" => [] ];
    	$matches = [];

    	preg_match_all( '/[[:cntrl:]]/', $json, $matches);
		if ( !empty( $matches ) )
		{
			$this->replaces["to"] = $matches[0];
			// finding a safe replace token
			$from0 = $this->replaces["from"];	
			while ( strpos( $json, $this->replaces["from"] ) !== false ) {
			    $this->replaces["from"] .= $from0;
			}
			$json = preg_replace( '/[[:cntrl:]]/', $this->replaces["from"], $json );
		}

	    return json_decode ( $json, $assoc, $depth, $flags );
    }

	function Encode ( $json_obj, $flags=0, $depth=512 )
	{
		$json = json_encode ( $json_obj, $flags, $depth );
		if ( !empty( $this->replaces ) && !empty( $this->replaces["to"] ) ) 
		{
			$from = $this->replaces["from"];
			foreach( $this->replaces["to"] as $to )
			{
				$to_encoded = substr( json_encode($to, $flags, $depth), 1, -1 );
	    		$json = preg_replace( "/$from/", $to_encoded, $json, 1 );
			}
		}

	    return $json;
	}

}

function HttpFormatResultSet ( $result, $nquery, $keep_json_ctrls=false )
{
	$str = '';

	$http_endpoint = '';
	$http_request = '';
	$http_method = '';
	$http_code = '';
	$rows = array();
	$attrs = array();
	$matches = array();
	$meta = array();
	$agent = '';

	if ( array_key_exists ( 'http_endpoint', $result ) )
		$http_endpoint = $result['http_endpoint'];

	if ( array_key_exists ( 'http_request', $result ) )
		$http_request = $result['http_request'];

	if ( array_key_exists ( 'http_method', $result ) )
		$http_method = $result['http_method'];

	if ( array_key_exists ( 'http_code', $result ) )	
		$http_code = $result['http_code'];
    
	if ( array_key_exists ( 'rows', $result ) )	
	{
		if ( $keep_json_ctrls )
		{
			$json_handler = new SafeJsonHandler();
			$rows = $json_handler->Decode ( $result['rows'], 1 );	
		}
		else
        	$rows = json_decode ( $result['rows'], 1 );
	}

	if ( array_key_exists ("agent", $result ) )
		$agent =" (agent-" . $result["agent"] . ")" ;

	$request = json_encode ( $http_request, JSON_UNESCAPED_SLASHES | JSON_UNESCAPED_UNICODE );
	$request = stripcslashes ( $request );
	
	$str .= "http-$nquery$agent> /$http_endpoint\n";
	$str .= "Status: $http_code\n";
	$str .= "$http_method ". $request . "\n";
				
	if ( count ( $rows ) )
		$str .= ( $keep_json_ctrls ? $json_handler->Encode ( $rows, JSON_PRETTY_PRINT | JSON_UNESCAPED_SLASHES | JSON_UNESCAPED_UNICODE ) : json_encode ( $rows, JSON_PRETTY_PRINT | JSON_UNESCAPED_SLASHES | JSON_UNESCAPED_UNICODE ) )."\n";

	$str .= "\n";

	return $str;
}


function RunCurl ( $curl_desc, &$rows )
{
	$con = curl_init();
	
	curl_setopt_array ( $con, $curl_desc );
	$rows = curl_exec ( $con );
	$rows = preg_replace('/"time":\d+(\.\d+)*,/', '"time":0.000,', $rows);
	$http_code = curl_getinfo ( $con, CURLINFO_HTTP_CODE );
	
	curl_close($con);
	
	return $http_code;
}

function HttpQueryCurl ( $http_method, $http_endpoint, $http_content, $http_query, $use_agent, $use_ssl )
{
	global $agents;
	
	$ssl = $use_ssl ? "https://" : "";
	
	$http_port = $agents[$use_agent]["http_port"];
	$curl_desc = array ( CURLOPT_RETURNTRANSFER => 1, CURLOPT_CONNECTTIMEOUT=>1, CURLOPT_URL => $ssl . "127.0.0.1:$http_port/" . $http_endpoint );
	if ( $http_method=="POST" )
    {
        $curl_desc[CURLOPT_POST] = 1;
        $curl_desc[CURLOPT_POSTFIELDS] = $http_query;
    } else
        $curl_desc[CURLOPT_URL] = $curl_desc[CURLOPT_URL] . '?' . $http_query;

	if ( $use_ssl )
	{
		$curl_desc[CURLOPT_SSL_VERIFYPEER] = false;
		$curl_desc[CURLOPT_SSL_VERIFYHOST] = false;
		$curl_desc[CURLOPT_SSLVERSION] = CURL_SSLVERSION_DEFAULT;
		//$curl_desc[CURLOPT_VERBOSE] = true; // !COMMIT
	}

	if ( $http_content!==NULL )
		$curl_desc[CURLOPT_HTTPHEADER] = array("Content-Type: ".$http_content);

	$rows = "";
	$http_code = RunCurl( $curl_desc, $rows );
	return array ( 'http_endpoint'=>$http_endpoint, 'http_method'=>$http_method, 'http_request'=>$http_query, 'rows'=>$rows, 'http_code'=>$http_code, 'http'=>1 );
}

// these two used from custom tests
function HttpQueryGet ( $http_endpoint, $http_content, $http_get, $use_agent, $use_ssl )
{
    $res = HttpQueryCurl ( "GET", $http_endpoint, $http_content, $http_get, $use_agent, $use_ssl );
    unset ( $res['http'] );
    return $res;
}

function HttpQueryPost ( $http_endpoint, $http_content, $http_get, $use_agent, $use_ssl )
{
    $res = HttpQueryCurl ( "POST", $http_endpoint, $http_content, $http_get, $use_agent, $use_ssl );
    unset ( $res['http'] );
    return $res;
}



function GetJsonError()
{
    switch (json_last_error())
	{
		case JSON_ERROR_NONE:			return 'no errors';
		case JSON_ERROR_DEPTH:			return 'maximum stack depth exceeded';
		case JSON_ERROR_STATE_MISMATCH: return 'underflow or the modes mismatch';
		case JSON_ERROR_CTRL_CHAR:		return 'unexpected control character found';
		case JSON_ERROR_SYNTAX:			return 'syntax error, malformed JSON';
		case JSON_ERROR_UTF8:			return 'malformed UTF-8 characters, possibly incorrectly encoded';
    }

	return 'unknown error';
}

function GetAttrs ($node)
{
    $vals = array ();
    if (!$node->attributes)
        return $vals;
    $len = $node->attributes->length;
    if ( $len == 0 )
        return $vals;
    foreach( AttrArray( $node ) as $attr )
        $vals[$attr->nodeName]=$attr->nodeValue;
    return $vals;
}

 function ParseRange ( $range )
{
    if ( !$range )
        return false;

    $values = explode ( ' ', $range );
    if ( count($values) != 2 )
    {
        printf ( "ERROR: malformed range attribute: '%s'\n", $range );
        return false;
    }

    return array ( 'min' => $values[0], 'max' => $values[1] );
}

function ParseIndexWeights ( $weights )
{
    if ( !$weights )
        return false;

    $result = array();
    preg_match_all ( '/([^\s]+):(\d+)/', $weights, $matches, PREG_SET_ORDER );
    foreach ( $matches as $match )
        $result [ $match[1] ] = (int)$match[2];

    return $result;
}

function close_and_null_conn (&$connection)
{
    if ($connection)
        mysqli_close($connection);
    $connection = NULL;
}

function result_must_be_skipped ( $result )
{
    return  ( array_key_exists('skip', $result) && $result['skip']>0 );
}

function result_must_not_be_displayed ($result)
{
    if ( array_key_exists ("comment", $result) )
        return @$result['hide']==1;
    return @$result['skip']==1;
}


class SphinxConfig
{
	private $_name;
	private $_db_create;
	private $_db_drop;
	private $_db_insert;
	private $_custom_insert;
	private $_counters;
	private $_dynamic_entries;
	private $_queries;
	private $_vars;
	private $_sphqueries;
	private $_query_settings;
	private $_query_attributes;
	private $_indexer_runs;
	private $_custom_test;
	private	$_sd_address;
	private $_sd_log;
	private	$_sd_port;
	private $_sd_sphinxql_port;
	private	$_sd_http_port;
	private $_sd_sphinxql_port_vip;
	private $_sd_replication_port;
	private	$_sd_pid_file;
	private $_num_agents;
	private $_subtest;
	private $_subtestcount;
	private $_results;
	private $_results_model;
	private $_prereqs;
	private $_config;				///< config DOM node
	private $_indexdata;			///< data for use "insert into" instead of run indexer
	private $_connection;			///< mysql connection (since we cound use mysql ans sqphinxql together)
	private $_testdir;				///< the path to the directory with current test (namely for accessing data without knowing the test name)

	function SetConnection ( $connection )
	{
		$this->_connection = $connection;
	}

	function __construct()
	{
		global $sd_address, $sd_port, $sd_sphinxql_port, $sd_pid_file, $sd_http_port, $sd_sphinxql_port_vip, $sd_replication_port;

		$this->_counters 		= array ();
		$this->_dynamic_entries = array ();
		$this->_queries 		= array ();
		$this->_vars            = array ();
		$this->_sphqueries		= array ();
		$this->_results			= array ();
		$this->_results_model	= array ();
		$this->_query_attributes = array ();
		$this->_indexer_runs	= array ();
		$this->_db_create		= array ();
		$this->_db_drop			= array ();
		$this->_db_insert		= array ();
		$this->_custom_insert	= array ();
		$this->_num_agents		= 1;
		$this->_subtest 		= 0;
		$this->_subtestcount	= 0;
		$this->_sd_address		= $sd_address;
		$this->_sd_port			= $sd_port;
		$this->_sd_sphinxql_port	= $sd_sphinxql_port;
		$this->_sd_http_port	= $sd_http_port;
		$this->_sd_sphinxql_port_vip= $sd_sphinxql_port_vip;
		$this->_sd_replication_port = $sd_replication_port;
		$this->_sd_pid_file		= $sd_pid_file;
		$this->_custom_test		= "";
		$this->_compat098		= false;
		$this->_skip_indexer	= false;
		$this->_indexdata		= array ();
		$this->_connection		= false;
		$this->_testdir			= "";
	}

	function EnableCompat098 ()		{ $this->_compat098 = true; }	
	function SubtestNo ()			{ return $this->_subtest; }
	function SubtestCount ()		{ return $this->_subtestcount; }
	function Name ()				{ return $this->_name; }
	function DB_Drop ()				{ return $this->_db_drop; }
	function DB_Create ()			{ return $this->_db_create; }
	function DB_Insert ()			{ return $this->_db_insert; }
	function DB_CustomInsert ()		{ return $this->_custom_insert; }
	function NumAgents ()			{ return $this->_num_agents; }
	function AddressAPI ()				{ return $this->_sd_address; }
	function Port ()						{ return $this->_sd_port; }
	function Requires ( $name )		{ return isset($this->_prereqs[$name]); }
	function IsQueryTest ()			{ return strlen ( $this->_custom_test ) == 0;	}
	function IsNeedDB()				{ return ! ( empty ( $this->_db_drop )
										&& empty ( $this->_db_create )
										&& empty ( $this->_db_insert ) ); }
	function IsRt()
	{
		global $g_locals;
		if ( !array_key_exists ('rt_mode', $g_locals) )
			return false;
		return $g_locals['rt_mode'];
	}

	function IsColumnar()
	{
		global $g_locals;
		if ( !array_key_exists ('columnar_mode', $g_locals) )
			return false;
		return $g_locals['columnar_mode'];
	}

	function NeedIndexerEx ()
	{
		return count ( $this->_indexer_runs ) > 0;
	}
	function Results ()				{ return $this->_results; }
	function GetQuery ( $i )		{ return $this->_queries[$i]; }
	function IsSkipIndexer ()		{ return $this->_skip_indexer; }
	function ResetResults ()				{ $this->_results = array (); }

	function SetTestDir ( $dir )
	{
		$this->_testdir = str_replace ( "\\", "/", $dir );
	}

	function GetLocal ( $key )
	{
		global $g_locals;
		
		if ( !array_key_exists ( $key, $g_locals ) )
		{
			printf ( "FATAL: unbound local variable '%s' (go add it at ~/.sphinx).\n", $key );
			exit ( 1 );
		}
		return $g_locals[$key];
	}

	function CreateNextConfig ()
	{
		return $this->GenNextCfg ( 0 );
	}


	function SubtestFinished ()
	{
		$this->_subtest++;
	}


	function SubtestFailed ()
	{
		$this->_subtest++;

		$failed = array ();
		array_push ( $failed, "failed" );

		if ( IsModelGenMode () )
			array_push ( $this->_results_model, $failed );
	}

	
	function ModelSubtestFailed ()
	{
		$failed = array ();
		array_push ( $failed, "failed" );

		return $this->_results_model [$this->SubtestNo ()] == $failed;
	}


	function SetAgent ( $agent, $i )
	{
		if ( !is_array ( $agent ) )
			return;

		if ( $i>0 )
		    $this->_sd_log = "searchd$i.log";
		else
            $this->_sd_log = "searchd.log";
		$this->_sd_address = $agent ["address"];
		$this->_sd_port = $agent ["port"];
		$this->_sd_sphinxql_port = $agent ["sqlport"];
		$this->_sd_sphinxql_port_vip = $agent ["sqlport_vip"];
		$this->_sd_replication_port = $agent ["replication_port"];
		$this->_sd_http_port = $agent["http_port"];
	}

	
	function SetPIDFile ( $pidfile )
	{
		$this->_sd_pid_file = $pidfile;
	}


	function GenNextCfg ( $i )
	{
		if ( count ( $this->_dynamic_entries ) == 0 )
			return FALSE;

		$num_variants = count ( ChildrenArray ( $this->_dynamic_entries[$i], "variant" ) );
	
		if ( $this->_counters [$i] == $num_variants - 1 )
		{
			if ( $i == count ( $this->_dynamic_entries ) - 1 )
				return FALSE;
			else
			{
				$this->_counters [$i] = 0;
				return $this->GenNextCfg ( $i + 1 );
			}
		}
		else
			$this->_counters [$i]++;

		return TRUE;
	}
	
	
	function WriteCustomTestResults ( $fp )
	{
		$res_fmt = $this->FormatResultSet ( 0, $this->_results );
		fwrite ( $fp, $res_fmt );
	}

	function GatherEntities ( $node, &$array )
	{
		foreach ( ChildrenArray($node) as $child )
			if ( $child->nodeType == XML_ELEMENT_NODE )
				array_push ( $array, $child->nodeValue );
	}


	function GatherNodes ( $node )
	{
		if ( $node->nodeType != XML_TEXT_NODE && $node->nodeType != XML_DOCUMENT_NODE
			&& strtolower ( $node->nodeName ) == "dynamic" )
		{
			$node->id = count ( $this->_dynamic_entries );
			array_push ( $this->_dynamic_entries, $node );
			array_push ( $this->_counters, 0 );
		}

		for ( $i = 0; !is_null ( $node->childNodes ) && $i < $node->childNodes->length; $i++ )
			$this->GatherNodes ( $node->childNodes->item ( $i ) );
	}

	function FormatVar ($nodename, $attrs)
    {
        if ( array_key_exists ( 'format', $attrs ) )
            return sprintf($attrs['format'],  $this->_vars[$nodename]);
        return  $this->_vars[$nodename];
    }

	function GenerateQueryText ( $node )
	{
		global $agents;
		$result = "";
        $attrs = GetAttrs($node);

		$nodename = strtolower ( $node->nodeName );
		switch ( $nodename )
		{
			case "#text":			return $node->nodeValue;
			case "#cdata-section":	return $node->nodeValue;
			case "static":			return $node->nodeValue;
			case "this_test":		return $this->_testdir;
			case "agent0_address":	return $agents[0]["address"].":".$agents[0]["port"];
			case "agent1_address":	return $agents[1]["address"].":".$agents[1]["port"];
			case "agent2_address":	return $agents[2]["address"].":".$agents[2]["port"];
		}
		if ( array_key_exists ($nodename, $this->_vars) )
		    return $this->FormatVar($nodename, $attrs);

		foreach ( ChildrenArray($node) as $child )
			$result.= $this->GenerateQueryText ( $child );

		return $result;
	}

    function GenerateQueryTextWithComments ( $node )
    {
        $results = array();
        $result = '';

        foreach ( ChildrenArray($node) as $child )
            if ( $child->nodeName === '#comment' )
            {
                if ( trim($result)!='' )
                    $results[]= array ( true, $result );
                $results[]= array ( false, $child->nodeValue ) ;
                $result = '';
            } else
                $result .= $this->GenerateQueryText ( $child );

        if ( trim($result)!='' )
            $results[]= array ( true, $result );
        return $results;
    }

    function AddAPIUpdate ($mode, $res)
    {
        $res['mode'] = $mode;
        $json = $res['query'][0];
        $upd = json_decode ($json, true);
        $res['query'] = $upd;
        $res['type'] = 'apiupdate';
        $this->_queries[] = $res;
        return true;
    }

	function AddAPIQuery ($q, $encoding)
    {
        $res = GetAttrs ($q);

        // add query
        if ( array_key_exists ( "source", $res) )
        {
            $source = $res["source"];
            if ( substr ( $source, 0, 6 ) == "local:" )
                $source = $this->GetLocal ( substr ( $source, 6 ) );
            if ( !is_readable($source) )
            {
                printf ( "FATAL: query source file '%s' not found.\n", $source );
                exit ( 1 );
            }
            $queries = file ( $source, FILE_IGNORE_NEW_LINES );
            $limit = $this->GetLocal('qlimit');
            $res["query"] = $limit ? array_slice( $queries, 0, $limit ) : $queries;
        } else
            $res["query"] = array ( iconv ( 'utf-8', $encoding, $q->nodeValue ) );

        // parse query mode
        $mode = 0;
        $mode_s = ArrVal ($res,"mode");
        switch ( $mode_s )
        {
            case "":			$mode_s = "(default)"; break;
            case "all":			$mode = SPH_MATCH_ALL; break;
            case "any":			$mode = SPH_MATCH_ANY; break;
            case "phrase":		$mode = SPH_MATCH_PHRASE; break;
            case "extended":	$mode = SPH_MATCH_EXTENDED; break;
            case "extended2":	$mode = SPH_MATCH_EXTENDED2; break;
            case "fullscan":	$mode = SPH_MATCH_FULLSCAN; break;
            case 'update_int':      return $this->AddAPIUpdate (SPH_UPDATE_INT, $res );
            case 'update_mva':      return $this->AddAPIUpdate (SPH_UPDATE_MVA, $res );
            case 'update_string':   return $this->AddAPIUpdate (SPH_UPDATE_STRING, $res );
            case 'update_json':     return $this->AddAPIUpdate (SPH_UPDATE_JSON, $res );
            default:
                return "unknown matching mode '" . $mode_s . "'";
        }
        $res["mode"] = $mode;
        $res["mode_s"] = $mode_s;

        // parse ranker
        $ranker = 0;
        $ranker_s = ArrVal ($res, "ranker");

        if ( empty($ranker_s) )
        {
            $ranker_s = "(default)";
        } else
        {
            $ranker = @constant("SPH_RANK_" . strtoupper($ranker_s));
            if ( $ranker===NULL )
                return "unknown ranker '" . $ranker_s . "'";
        }

        $res["ranker"] = $ranker;
        $res["ranker_s"] = $ranker_s;

        // parse filter
//        TouchVal ( $res, "filter");
//        TouchVal ( $res, "filter_value");
        if ( @$res['filter_range'])
            $res["filter_range"] = ParseRange ($res["filter_range"]);
//        TouchVal ( $res, "filter_str");
//        TouchVal ( $res, "filter_exclude");

        // parse sort mode and get clause
        $sortmode = 0;
        $sortmode_s = ArrVal ($res,"sortmode");
        switch ( $sortmode_s )
        {
            case "":			$sortmode_s = "(default)"; break;
            case "extended":	$sortmode = SPH_SORT_EXTENDED; break;
            case "expr":		$sortmode = SPH_SORT_EXPR; break;
            case "attr_asc":	$sortmode = SPH_SORT_ATTR_ASC; break;
            case "attr_desc":	$sortmode = SPH_SORT_ATTR_DESC; break;
            default:
                return "unknown sorting mode '" . $sortmode_s . "'";
        }
        $res["sortmode"] = $sortmode;
        $res["sortmode_s" ] = $sortmode_s;
        TouchVal ( $res, "sortby");

        // groupby
        $groupfunc = 0;
        $groupfunc_s = ArrVal ($res,"groupfunc");
        switch ( $groupfunc_s )
        {
            case "":			$groupfunc = SPH_GROUPBY_ATTR; $groupfunc_s = "attr"; break;
            case "day":			$groupfunc = SPH_GROUPBY_DAY; break;
            case "week":		$groupfunc = SPH_GROUPBY_WEEK; break;
            case "month":		$groupfunc = SPH_GROUPBY_MONTH; break;
            case "year":		$groupfunc = SPH_GROUPBY_YEAR; break;
            case "attr":		$groupfunc = SPH_GROUPBY_ATTR; break;
            case "attrpair":	$groupfunc = SPH_GROUPBY_ATTRPAIR; break;
            default:
                return "unknown groupby func '" . $groupfunc_s . "'";
        }

        $res["groupfunc"] = $groupfunc;
        $res["groupfunc_s"] = $groupfunc_s;
//        TouchVal ( $res, "groupattr");
        TouchVal ($res,"groupsort", "@group desc");
//      TouchVal ($res,"groupdistinct");

//        TouchVal ($res,"resarray");
        TouchVal ($res,"index", "*");
//        TouchVal ($res,"select");
        if (@$res["index_weights"] )
            @$res["index_weights"] = ParseIndexWeights ( $res["index_weights"]);
        TouchVal ($res,"roundoff");
//        TouchVal ($res,"tag");
//        TouchVal ($res,"cutoff");
//        TouchVal ($res,"limits");

        $res['type'] = 'api';
        $this->_queries[] = $res;
        return true;
    }

    function AddRepeatComment ($attrs)
    {
        $skip = (int)ArrVal ($attrs, "skip", 0);
        $comment = 'repeat ';
        foreach ($attrs as $name => $value)
            $comment .= "$name=\"$value\" ";

        $this->_queries[] = array(
            'query' => trim($comment),
            'attrs' => $attrs,
            'type' => 'comment',
            'hide' => $skip
        );
    }

    function RepeatedlyAddQuery ($repeat, $addcomment, $cb)
    {
        $attrs = GetAttrs($repeat);
        if ($addcomment)
            $this->AddRepeatComment($attrs);

        $delimiter = ArrVal ($attrs, "delimiter", " ");
        $variants = ArrVals($attrs, "variants", $delimiter);

        if (!empty($variants)) { // varianted query: <repeat var="foo" variants="a b c> - 3 times repeat with foo=a, foo=b, foo=c
            $var = ArrVal($attrs, "var", "_");
            foreach ($variants as $variant)
            {
                $this->_vars[$var] = $variant;
                foreach ( ChildrenArray ( $repeat ) as $q )
                    $cb($q);
            }
            return;
        }

        $count = (int)ArrVal ($attrs, "count", 1);

        $vars = ArrVals($attrs, "vars", $delimiter);
        $inits = ArrVals($attrs, "init", $delimiter);
        $increments = ArrVals($attrs, "inc", $delimiter);

        // set initial values of vars
        foreach ($inits as $idx=>$init)
            $this->_vars[$vars[$idx]] = $init;

        for ($i=0; $i<$count; ++$i)
        {
            foreach ( ChildrenArray ( $repeat ) as $q )
                $cb($q);

            // set next values
            foreach ($increments as $idx=>$inc)
                $this->_vars[$vars[$idx]] += $inc;
        }
    }

    function PopulateVar ($var,$cb)
    {
        $attrs = GetAttrs($var);
        $name = ArrVal($attrs, "name" );
        $delimiter = ArrVal ($attrs, "delimiter", null);

        $exists = array_key_exists ( $name, $this->_vars ) && $this->_vars[$name]!=='';
        $val = trim($cb($var));

        if ( !$exists || $delimiter===null )
            $this->_vars[$name] = $val;
        else
            $this->_vars[$name] .= $delimiter . $val;
    }

    function ExposeMetaIndexes ($q, $attrs, $lowername, $metaindexes)
    {
        $was_replaced = false;
        if ( !empty($metaindexes) )
            foreach ( $metaindexes as $name=>$indexes )
                foreach ($indexes as $index)
                {
                    $foo = 0;
                    $res = str_replace ( $name, $index, $q, $foo );
                    if ($foo>0)
                    {
                        $was_replaced = true;
                        $this->_queries[]=array (
                            "query" => $res,
                            "attrs" => $attrs,
                            "type" => $lowername);
                    }
                }

        if (!$was_replaced) // no metaindexes; emit 'as is'
        {
            $this->_queries[] = array (
                "query" => $q,
                "attrs" => $attrs,
                "type" => $lowername);
        }
    }

	function AddQuery ( $q, $encoding, $metaindexes )
	{
		$lowername = strtolower($q->nodeName);
        $attrs = GetAttrs($q);
        switch ( $lowername )
        {
            case 'query':
                if (empty(@$attrs['endpoint'])) // that is API query
                    return $this->AddAPIQuery($q, $encoding);
                break;
            case 'sphinxql':
                break;
            case 'comment':
                $this->_queries[] = array (
                    "query" => iconv('utf-8', $encoding, trim($this->GenerateQueryText($q))),
                    "attrs" => $attrs,
                    "type" => 'comment');
                return;
            case '#comment':
                $this->_queries[] = array (
                    "query" => iconv('utf-8', $encoding, trim($q->nodeValue)),
                    "attrs" => $attrs,
                    "type" => 'comment');
                return;
            case 'repeat':
                $this->RepeatedlyAddQuery($q, true, function($q) use( $encoding, $metaindexes )
                {
                    $this->AddQuery ( $q, $encoding, $metaindexes );
                } );
                return;
            case 'var':
                $this->PopulateVar($q,function($foo) { return $this->GenerateQueryText ($foo); });
                return;
            default:
                return;
        }

        $query_chunks = $this->GenerateQueryTextWithComments ( $q );
        if ( empty ($query_chunks))
            $this->_queries[] = array (
                "query" => "",
                "attrs" => $attrs,
                "type" => $lowername);
        foreach ($query_chunks as $query_chunk)
        {
            if ( !$query_chunk[0] ) // 0-th is bool 'statement/comment'
            {
                $this->_queries[] = array (
                    "query" => iconv('utf-8', $encoding, $query_chunk[1]),
                    "attrs" => $attrs,
                    "type" => 'comment');
                continue;
            }
            $query_texts = preg_split ( "/;\n\s*/", $query_chunk[1], -1, PREG_SPLIT_NO_EMPTY );
            foreach ($query_texts as $query_text)
                $this->ExposeMetaIndexes (iconv('utf-8', $encoding, trim($query_text)), $attrs, $lowername, $metaindexes);
        }
	}

	function AddQueries ( $queries, $encoding, $metaindexes )
	{
		foreach ( ChildrenArray ( $queries ) as $q )
            $this->AddQuery ( $q, $encoding, $metaindexes );
	}


	function ExtractQueries ( $node, $encoding, $metaindexes )
	{
		if ( !empty($node) && $node->hasChildNodes() )
			for ( $i=0; $i<$node->childNodes->length; $i++ )
			{
				$child = $node->childNodes->item ( $i );
				
				$lowername = strtolower($child->nodeName);
				if ( $lowername=="sphqueries" || $lowername=="httpqueries")
					$this->AddQueries ( $child, $encoding, $metaindexes );
			}
	}

	function AddDbInsertClause ($q, $encoding)
    {
        $q = trim ($q);
        if ($q!='')
            $this->_db_insert [] = iconv ( 'utf-8', $encoding, $q);
    }

	function AddDbInsertNode( $q, $encoding, &$accum )
    {
        $lowername = strtolower($q->nodeName);
        switch ( $lowername )
        {
            case 'repeat':
                $this->RepeatedlyAddQuery($q, false, function($foo) use( $encoding, &$accum )
                {
                    $this->AddDbInsertNode ( $foo, $encoding, $accum );
                } );
                return;
            case 'db_insert':
                $this->AddDbInsertClause($accum,$encoding);
                $accum='';
                return;
            case 'var':
                $this->PopulateVar($q,function($foo) { return $this->GenerateQueryText ($foo); });
                return;
            default:
                $accum .= $this->GenerateQueryText ($q);
        }
    }

    function AddDbInsert( $queries, $encoding )
    {
        $accum="";
        foreach ( ChildrenArray ( $queries ) as $q )
            $this->AddDbInsertNode ( $q, $encoding, $accum );

        $this->AddDbInsertClause($accum,$encoding);
    }

	function Load ( $config_file )
	{
		// load the file
		$doc = new DOMDocument ( "1.0" );
		if ( !$doc->load ( $config_file ) )
			return false;

		// check for proper root node
		if ( !$doc->hasChildNodes() )
			return false;

		$xml = $doc->childNodes->item(0);
		if ( strtolower($xml->nodeName)!="test" )
			return false;

		$custom = GetFirstChild ( $xml, "custom_test" );
		if ( $custom )
		{
			$this->_custom_test = $custom->nodeValue;
			if ( $doc->encoding != 'utf-8' )
				$this->_custom_test = iconv ( 'utf-8', $doc->encoding, $this->_custom_test );
		}

		// extract indexer run params
		$indexer_run = GetFirstChild ( $xml, "indexer" );
		if ( $indexer_run )
		{
			foreach ( ChildrenArray ( $indexer_run, "run" ) as $run )
				$this->_indexer_runs [] = $run->nodeValue;
		}

		// extract meta-indexes
		$metaindexes = array();
		foreach ( ChildrenArray ( $xml, "metaindex" ) as $meta )
		{
			$tmp = array();
			foreach ( ChildrenArray ( $meta, "index") as $idx )
				$tmp[] = iconv ( 'utf-8', $doc->encoding, $idx->nodeValue );

			$name = GetFirstAttr ( $meta );
			$metaindexes[$name->nodeValue] = $tmp;
		}

		// extract queries
		$qs = GetFirstChild ( $xml, "queries" );
		if ( $qs )
		{
			// new and cool - everything in <queries>.
            // <query endpoint=...> - as http, <sphinxql...> - as sphinxql,
            // rest <query...> - as api.
            // also <comment...> is suitable here (copied directly into report, doesn't affect result matching)
			$this->AddQueries ( $qs, $doc->encoding, $metaindexes );
		}
		else
		{
			// legacy
			$qs = array ();
			$this->GatherEntities ( GetFirstChild ( $xml, "query" ), $qs );
			foreach ( $qs as $q )
			{
				$this->_queries[] = array (
					"query" => array ( $q ),
					"mode" => 0,
					"mode_s" => '(default)',
					"ranker" => 0,
					"ranker_s" => '(default)',
                    "index" => '*',
                    "type" => 'api' );
			}
		}

		// old fashion: sphinxql are in <sphqueries>, http are in <httpqueries>
		$this->ExtractQueries ( $xml, $doc->encoding, $metaindexes );

		// extract my settings
		$this->_config = GetFirstChild ( $xml, "config" );
		$this->GatherNodes ( $this->_config );
		$this->GatherEntities ( GetFirstChild ( $xml, "query_attributes" ), $this->_query_attributes );

		foreach ( ChildrenArray ( $xml, "db_create" ) as $node )
			$this->_db_create []=$node->nodeValue;

		foreach ( ChildrenArray ( $xml, "db_drop" ) as $node )
			$this->_db_drop []=$node->nodeValue;

		foreach ( ChildrenArray ( $xml, "db_insert" ) as $node )
            $this->AddDbInsert ( $node, $doc->encoding );

		foreach ( ChildrenArray ( $xml, "custom_insert" ) as $node )
			$this->_custom_insert []=$node->nodeValue;

		$this->_name			= GetFirstChildValue ( $xml, "name" );
		$this->_query_settings	= GetFirstChildValue ( $xml, "query_settings" );
		$this->_num_agents		= GetFirstChildValue ( $xml, "num_agents", 1 );
		$this->_skip_indexer	= GetFirstChildValue ( $xml, "skip_indexer", GetFirstChildValue ( $xml, "skip-indexer", false ) );

		$this->_prereqs = array();
		$prereqs = GetFirstChild ( $xml, "requires", false );
		if ( $prereqs )
			foreach ( ChildrenArray ( $prereqs ) as $node )
				$this->_prereqs [ $node->nodeName ] = 1;

		// precalc subtests count
		$this->_subtestcount = 1;
		foreach ( $this->_dynamic_entries as $entry )
		{
			$variants = count ( ChildrenArray ( $entry, "variant" ) );
			$this->_subtestcount *= max ( $variants, 1 );
		}

		return true;
	}


	function RunIndexerEx ( &$error )
	{
		foreach ( $this->_indexer_runs as $param )
		{
			$retval = RunIndexer ( $error, $param );
			if ( $retval != 0 )
				return $retval;
		}

		return 0;
	}

	function FixupAgentAddress ( $s )
	{
		global $agents;
		$tr = array(
			$agents[0]["address"].":".$agents[0]["port"] => "<AGENT0_ADDRESS/>",
			$agents[1]["address"].":".$agents[1]["port"] => "<AGENT1_ADDRESS/>",
			$agents[2]["address"].":".$agents[2]["port"] => "<AGENT2_ADDRESS/>" );
		return strtr ( $s, $tr );
	}

	function FixupErrorMessage ( $err, $s )
	{
		$s = $this->FixupAgentAddress ( $s );
		if ($err!=1064)
            $s = preg_replace ( '/or \d+ other tokens/', 'or N other tokens', $s );
		else
		    $s = preg_replace (
                [
                    '/error when sending data: WSA error 10057/', // windows
                    '/connect timed out/', // also windows
                    '/receiving failure .*$/' // linux (errno 111) and mac (errno 64)
                ],
                [
                    "receiving failure (refused or timedout, substituted)",
                    "receiving failure (refused or timedout, substituted)",
                    "receiving failure (refused or timedout, substituted)"
                ], $s );
		return $s;
	}


	function FixupWarningMessage ( $s )
	{
		if ( strpos ( strtolower ( $s ), 'warning' ) !== false )
			$s = $this->FixupAgentAddress ( $s );

		return $s;
	}

	function PickResult ( $query_result )
    {
        array_push($this->_results, $query_result);
        if (IsModelGenMode() && !result_must_be_skipped($query_result))
            $this->_results_model[$this->SubtestNo()][] = $query_result;
    }

	function PickResults( $query_results )
    {
        $this->_results = $query_results;
        if ( IsModelGenMode () )
            array_push ( $this->_results_model, $query_results );
    }

	function FixupJsonTimeAndFloats ( &$query_result, &$error, $keep_json_ctrls=false )
    {
        if ( !array_key_exists ( 'rows', $query_result ) )
            return true;

        if ( $keep_json_ctrls )
        {
			$json_handler = new SafeJsonHandler();
		   	$decoded_rows = $json_handler->Decode ( $query_result['rows'] );
		}
		else
			$decoded_rows = json_decode ( $query_result['rows'] );

        if ( !$decoded_rows )
        {
            $error = "Unable to decode repsonse json'" . $query_result['rows'] . "': ".GetJsonError();
            return false;
        }

        if ( is_object($decoded_rows) && property_exists ( $decoded_rows, 'took' ) )
            unset ( $decoded_rows->took );

        // fixup floats in json
        if ( isset ( $decoded_rows->hits ) && isset ( $decoded_rows->hits->hits ) )
        {
            foreach ( $decoded_rows->hits->hits as &$hit )
                RoundFloatValues ( $hit, 6 );
        }

        $query_result['rows'] = $keep_json_ctrls ? $json_handler->Encode ( $decoded_rows, JSON_UNESCAPED_SLASHES | JSON_UNESCAPED_UNICODE ) : json_encode ( $decoded_rows, JSON_UNESCAPED_SLASHES | JSON_UNESCAPED_UNICODE );

        return true;
    }

	function FixupJsonTotal ( &$query_result, &$error, $keep_json_ctrls=false )
    {
        if ( !array_key_exists ( 'http_endpoint', $query_result ) )
            return true;

		if ( strtolower ($query_result['http_endpoint'] )!='sql' )
			return true;

        if ( !array_key_exists ( 'rows', $query_result ) )
            return true;

        if ( $keep_json_ctrls )
        {
			$json_handler = new SafeJsonHandler();
		   	$decoded_rows = $json_handler->Decode ( $query_result['rows'] );
		}
		else
			$decoded_rows = json_decode ( $query_result['rows'] );

        if ( !$decoded_rows )
        {
            $error = "Unable to decode repsonse json'" . $query_result['rows'] . "': ".GetJsonError();
            return false;
        }

        if ( !is_object($decoded_rows) || !isset ( $decoded_rows->hits ) )
			return true;

		// fixme! maybe add more intelligent checks instead of just ignoring the 'total'?
        if ( isset ( $decoded_rows->hits->total_relation ) && $decoded_rows->hits->total_relation=='gte' )
			$decoded_rows->hits->total = count ( $decoded_rows->hits->hits );

		$query_result['rows'] = $keep_json_ctrls ? $json_handler->Encode ( $decoded_rows, JSON_UNESCAPED_SLASHES | JSON_UNESCAPED_UNICODE ) : json_encode ( $decoded_rows, JSON_UNESCAPED_SLASHES | JSON_UNESCAPED_UNICODE );

        return true;
    }

	function RunQueryHttp ( $query, &$error, &$examples )
	{
		$attrs = $query['attrs'];

        $http_endpoint=ArrVal ($attrs, "endpoint", NULL);
        $http_method=ArrVal ($attrs, "method", "POST");
        $http_content=ArrVal ($attrs, "content", NULL);
        $use_agent = ArrVal ($attrs, "d", 0);


		if ( !is_array($query) || !$attrs )
		{
			$error = "HTTP endpoint not specified";
			return false;
		}

		$query = trim($query['query']);

		$http_method = strtoupper($http_method);
		if ( !($http_method=="POST" || $http_method=="GET") )
        {
            $error = "Unknown HTTP method: ".$http_method;
            return false;
        }

		$query_result = HttpQueryCurl ( $http_method, $http_endpoint, $http_content, $query, $use_agent, $this->Requires("https") );

		if ( $use_agent>0 )
			$query_result['agent'] = $use_agent;


		$keep_json_ctrls = $this->Requires("keep_json_ctrls");
		if ( !$this->FixupJsonTimeAndFloats( $query_result, $error, $keep_json_ctrls ) )
		    return false;

		if ( !$this->FixupJsonTotal ( $query_result, $error, $keep_json_ctrls ) )
		    return false;

		$this->PickResult ( $query_result );
		return true;
	}

    function RunQueryApiUpdate ( $query, &$error, &$examples )
    {
        global $sd_address, $sd_port;

        $cl = new APIClient();
        $cl->SetServer ( $sd_address, $sd_port );

        $res = $cl->XUpdateAttributes ($query['index'], $query['query']['attrs'], $query['query']['values'], $query['mode'] );
        $query_result = array ('updated'=> $res);
        $this->PickResult ( $query_result );
        return true;
    }


	function RemovePathsFromExtFiles ( &$query )
	{
		$lower_query = strtolower ( trim ( $query ) );

		$create_table = "create table";
		$alter_table = "alter table";
		if ( StrBegins ( $lower_query, $create_table ) || StrBegins ( $lower_query, $alter_table ) )
		{
			$matches = array();
			$preg = preg_match_all ( "/(?:(?:wordforms|stopwords|exceptions|hitless_words)[ \t]*=[ \t]*)'([^']+)'/", $query, $matches, PREG_OFFSET_CAPTURE );

			$result_str = "";
			$prev_offset = 0;
			foreach ( $matches[1] as $entry )
			{
				$paths = explode (' ', $entry[0]);

				$replacement = '';
				foreach ( $paths as $single_path )
				{
					if ( strlen($replacement) )
						$replacement .=' ';
					$replacement .= basename($single_path);
				}

				$result_str.= substr ( $query, $prev_offset, $entry[1]-$prev_offset );
				$result_str.= $replacement;
				$prev_offset = $entry[1] + strlen($entry[0]);
			}

			$result_str.= substr ( $query, $prev_offset );
			$query = $this->FixupAgentAddress($result_str);

			return true;
		}

		return false;
	}


	function RemovePathsFromImportTable ( &$query )
	{
		$lower_query = strtolower ( trim ( $query ) );

		$import_table = "import table";
		if ( StrBegins ( $lower_query, $import_table ) )
		{
			$matches = array();
			$preg = preg_match_all ( "/(?:.*)'([^']+)'/", $query, $matches, PREG_OFFSET_CAPTURE );

			$result_str = "";
			$prev_offset = 0;
			foreach ( $matches[1] as $entry )
			{
				$result_str.= substr ( $query, $prev_offset, $entry[1]-$prev_offset );
				$replacement = basename($entry[0]);
				$result_str.= $replacement;
				$prev_offset = $entry[1] + strlen($entry[0]);
			}

			$result_str.= substr ( $query, $prev_offset );
			$query = $this->FixupAgentAddress($result_str);

			return true;
		}

		return false;
	}


	function PostprocessDDL ( &$query )
	{
		if ( $this->RemovePathsFromExtFiles($query) )
			return true;

		if ( $this->RemovePathsFromImportTable($query) )
			return true;

		return false;
	}

	function PostprocessQlResult ( $query, &$query_result, &$raw_result, $connection, $no_time, $ignore_rows )
	{
	    $raw_result["sphinxql"] = $query;
		$result = mysqli_wr ($query,$connection);
		if ($result===true)
		{
			if ( $this->PostprocessDDL($query) )
				$query_result["sphinxql"] = $query;

			$query_result["total_affected"] = mysqli_affected_rows($connection);
            $raw_result["total_affected"] = mysqli_affected_rows($connection);
		}
		else if ($result===false)
		{
			if ( $this->PostprocessDDL($query) )
				$query_result["sphinxql"] = $query;

			$error = mysqli_errno( $connection );
			$query_result["errno"] = $error;
            $raw_result["errno"] = $error;
            $raw_result["error"] = mysqli_error( $connection );
			$query_result["error"] = $this->FixupErrorMessage ( $error, $raw_result["error"] );

		}
		else
		{
            $raw_result["total_rows"] = mysqli_num_rows($result);
			$query_result["total_rows"] = $raw_result["total_rows"];
			while ($row = mysqli_fetch_array($result, MYSQLI_ASSOC))
			{
                $raw_result["rows"][] = $row;
				if ( $no_time===true && array_key_exists ( 'Variable_name', $row ) && $row['Variable_name']=='time' )
					continue;
				$cont = false;
                foreach ( $ignore_rows as $ignore_row )
                {
                    if ( array_key_exists ( $ignore_row, $row ) ) {
                        $cont = true;
                        break;
                    }
                }
                if ($cont)
                    continue;
				$query_result["rows"][] = $row;
			}
			mysqli_free_result($result);

			if ( isset ( $query_result["rows"] ) )
				foreach ( $query_result["rows"] as &$row )
				{
					if ( isset ( $row["Create Table"] ) )
					{
						$create_res = $row["Create Table"];
						if ( $this->PostprocessDDL($create_res) )
							$row["Create Table"] = $create_res;
					}
				}
		}
	}

	function RunQueryQL ( $query, &$error, &$examples, $bench, &$connection, &$old_agent, &$old_vip )
	{
		global $agents, $index_data_path;

		$attrs = ArrVal ( $query, 'attrs', [] );
        $query = trim($query['query']);
		$query_ref = $query;

        $sleep_agent=ArrVal ($attrs, "sleep", 0);
        $use_agent=ArrVal ($attrs, "d", 0);
        $vip_connection=ArrVal ($attrs, "vip", 0);
        $system_cmd = ArrVal ($attrs, "system");
        $wait_value = ArrVal ($attrs, "wait_value", 0);
        $cluster_connect = ArrVal ($attrs, "cluster_connect", -1);
        $cluster_path = ArrVal ($attrs, "cluster_path");
        $cluster_name = ArrVal ($attrs, "cluster");
        $cluster_status = ArrVal ($attrs, "status");
        $example = ArrVal ($attrs, "example");
        $skip_result = ArrVal ($attrs, 'skip',false);
        $ignore_rows = ArrVals ($attrs, 'ignore');
        $hide = ArrVals ($attrs, 'hide',false);
        $params = ArrVal ( $attrs, "params");

        // mysql_host - we direct query to host mysql server, NOT to searchd! (that is sometimes need for rotation)
        $mysql_host = ($use_agent==="mysql");
        if ($mysql_host)
            $use_agent = 0;

		// $query - array, 0 is data, 1 is attributes, 2 is 'is_http' bool.

        $agent = $agents[$use_agent];
        $daemon = ArrVal ($agent, "daemon", null);

		if ($cluster_connect>=0) {
            $cluster = $agents[$cluster_connect];
            $address = $cluster["address"] . ":" . $cluster["port"];
            $query = str_replace("%addr_connect%", $address, $query);
        }

		if ( !empty($cluster_path) ) {
            $data_path = $agent["data_path"];
            $cluster_dir = "$index_data_path/$data_path/$cluster_path";
            if ( !file_exists ( $cluster_dir ) )
                mkdir ( $cluster_dir );

            $query = str_replace ( "%cluster_path%", $cluster_path, $query );
		}

        $query_result = array ();
        if ( $vip_connection )
            $query_result["vip"]=1;

        if ( $use_agent>0 )
            $query_result["agent"]=$use_agent;

        if ($skip_result)
            $query_result['skip']=1;

        if ($hide)
            $query_result['hide']=1;


        if ($old_agent!=$use_agent && $system_cmd!='start-agent')
		{
			if ($old_agent!=-1)
                close_and_null_conn($connection);
			if ($sleep_agent>0)
				usleep($sleep_agent);
            if (!($connection = ConnectSpecificQL($use_agent)))
                return false;
            $old_agent = $use_agent;
		}

		if ($this->Requires('vip') && ( $old_vip!=$vip_connection || $connection===NULL ))
		{
			if ( $connection )
                close_and_null_conn($connection);

			// this is needed to test non-vip connections in maintenance mode
			if (!($connection = ConnectSpecificQL($use_agent,$vip_connection)))
			{
				$query_result["sphinxql"]=$query;
				$query_result["error"] = mysqli_connect_error();
				$query_result["errno"] = mysqli_connect_errno();
				$this->PickResult($query_result);
				$connection = NULL;
				return true;
			}

			$old_vip = $vip_connection;
		}
			
		if ( !empty($system_cmd) )
		{
			$status = '';
            switch ( $system_cmd ) {
                case 'restart-daemon':
                case 'restart-daemon-no-warnings':
                    close_and_null_conn($connection);
                    $status = RestartDaemon($system_cmd=='restart-daemon-no-warnings');
                    $connection = ConnectSpecificQL($use_agent);
                    break;
                case 'kill-daemon':
                    close_and_null_conn($connection);
                    KillSearchd($daemon["config"], $daemon["pid"], 'KILL', false);
                    break;
                case 'start-agent-no-warnings':
                case 'start-agent':
                    $start = StartSearchd($daemon["config"], $daemon["error"], $daemon["pid"], $error,
                        $daemon["requirements"], $daemon["address"], $daemon["port"]);
                    if ($start == 0 || $start == 2)
                    {
                        if ( $system_cmd==='start-agent-no-warnings' )
                            $start = 0;
                        $status .= "; start=ok" . ', return code=' . $start;
                    } else
                    {
                        $status .= "; start=failed, return code=" . $start . ", error=" . $error;
                    }

                    close_and_null_conn($connection);
                    $old_agent = -1;
                    break;
                case 'stop-agent':
                    $stop = StopWaitSearchd($daemon["config"], $daemon["pid"]);
                    $status = 'stop=' . ($stop == 0 ? 'ok' : 'error') . ', return code=' . $stop;
                    close_and_null_conn($connection);
                    $old_agent = -1;
                    break;
                case 'run-indexer':
                    RunIndexer ( $error, $params );
                    if ($sleep_agent>0)
                        Sleep ( $sleep_agent);
                    $status = $params . ' ok.';
                    break;
                case 'wait-ready':
                    $ql = new QLClient();
                    $ql->SetConnection($connection);
                    $timeout=GetTmDelta();
                    $status = $ql->Query("debug wait $cluster_name like 'state' option 'timeout'=$timeout");
                    break;
                case 'wait-commit':
                    $ql = new QLClient();
                    $ql->SetConnection($connection);
                    $timeout=GetTmDelta();
                    $status = $ql->Query("debug wait $cluster_name status $wait_value like 'state' option 'timeout'=$timeout");
                    break;
                default:
                    $status = "unsupported command " . $system_cmd;
            }
            $query_result['sphinxql'] = "/* " . $system_cmd . " => " . $status . " */";
	        $this->PickResult ( $query_result );
			return true;
		}

        if ($mysql_host)
        {
            global $gl_conn;
            if ( !LegacyConnectDB())
                return FALSE;
            @mysql_query ( $query );
            @mysql_close();
            $gl_conn = NULL;

            if ($sleep_agent>0)
                Sleep ( $sleep_agent);

            $r = array("sphinxql"=>"/* " . $query . " => ok. */");
            $this->PickResult ( $r );
            return true;
        }

		if ( $query=="RECONNECT" )
		{
			mysqli_close($connection);
			if (!($connection = ConnectSpecificQL($use_agent)))
				return false;

			$r = array("sphinxql"=>$query, "error"=>"reconnected ok!", "errno"=>0);
            $this->PickResult ( $r );
			return true;
		}

		$no_time = false;
		if ( $bench===false && ( stripos ( $query, 'show' )!==false ) && ( stripos ( $query, 'meta' )!==false ) )
			$no_time = true;

		if ( strpos ($query, ";")===FALSE ) // process a single-query line
		// FIXME! If a query contains ';' it would be false positive for such codepath.
		{
			$query_result["sphinxql"]=trim ( $query_ref );
			unset ( $query_ref );

            $raw_result = [];
			$this->PostprocessQlResult ( $query, $query_result, $raw_result, $connection, $no_time, $ignore_rows );
			$this->PickResult($query_result);
			if ($example)
            {
                $examples[$example][] = $raw_result;
            }
		} else
		{
			$parts = explode (';',$query);
			$haserror = true;
			$erroneousquery = $query;
//            echo $query;
			if ( mysqli_multi_query ($connection,$query) )
			{
				$resultset_num = 0;
				do
				{
					if ($result = mysqli_store_result($connection))
					{
						$query_result["total_rows"] = mysqli_num_rows ($result);
						if (array_key_exists ("rows", $query_result) )
							unset ($query_result["rows"]);
						while ($row = mysqli_fetch_array($result, MYSQLI_ASSOC))
						{
							if ( $no_time===true && array_key_exists ( 'Variable_name', $row ) && $row['Variable_name']=='time' )
								continue;
							if ( array_key_exists ( 'Value', $row ) )
								$row['Value'] = $this->FixupWarningMessage ( $row['Value'] );
							$query_result["rows"][] = $row;
						}
						mysqli_free_result($result);
					} else
					{
						if (mysqli_field_count($connection)) // there were some fields. An error occured.
						{
							$query_result["error"] = mysqli_error( $connection );
							$query_result["errno"] = mysqli_errno( $connection );
						} else
							$query_result["total_affected"] = mysqli_affected_rows($connection);
					}
					if ($resultset_num==0)
						$query_result["sphinxql"]="$query";
					else
						$query_result["sphinxql"]=($resultset_num<count($parts)?"$parts[$resultset_num] ":"")."/* result ".($resultset_num+1)." of previous multistatement */";

					++$resultset_num;
					$this->PickResult($query_result);
					$erroneousquery = ($resultset_num<count($parts)?"$parts[$resultset_num] ":"")."/* result ".($resultset_num+1)." of previous multistatement */";
					if (!mysqli_more_results($connection))
					{
						$haserror = false;
						break;
					}
					$query_result = array ();
				} while (mysqli_next_result($connection));
			};
			if ($haserror)
			{
				$query_result["sphinxql"]=$erroneousquery;
				$query_result["error"] = mysqli_error( $connection );
				$query_result["errno"] = mysqli_errno( $connection );
				$this->PickResult($query_result);
			}
		}

		return true;
	}

	function ToHide($qinfo)
    {
        if ( array_key_exists ( 'hide', $qinfo ) )
            return !!$qinfo['hide'];
        return false;
    }

	function GetSearchdRequirements()
	{
		$requirements = array();
		if ( $this->Requires("no_pseudo_sharding") )
			$requirements["no_pseudo_sharding"] = true;

		if ( $this->Requires("watchdog") )
			$requirements["watchdog"] = true;

		return $requirements;
	}

	function RunQuery ( &$error, &$examples, $benchmark = false )
	{
		global $sd_address, $sd_port, $action_retries, $action_wait_timeout, $g_pick_query;
		$total = $done = 0;

		if ( $benchmark )
		{
			foreach ( $this->_queries as $qinfo )
				$total += count($qinfo['query']);
			$prefix = $benchmark;
			$tm = 0;
			$start = MyMicrotime();
		}
        $compact = $benchmark;

		$cl = new APIClient;
		$pconn = $benchmark && method_exists ( $cl, 'Open' );
		if ( $pconn )
		{
			$cl = new APIClient;
			$cl->SetServer ( $sd_address, $sd_port );
			$cl->Open ();
		}

        $ql_connection=false;
        $old_agent = -1;
        $old_vip = -1;

		$retries = 1;
		if ( !$benchmark )
			$retries = $action_retries;

		// tricky bit
		// sometimes, we run some API queries and then some QL queries!
		// so when picker points to an API query, choose it
		// but if it points past, adjust the picker
		$qmin = 0;
		$qmax = count($this->_queries) - 1;
		if ( $g_pick_query>0 )
		{
			if ( $g_pick_query<=$qmax )
			{
				$qmin = $g_pick_query-1;
				$qmax = $g_pick_query-1;
			} else
			{
				$g_pick_query -= count($this->_queries);
				return true;
			}
		}

        $bOk = true;

		for ( $n=$qmin; $n<=$qmax; $n++ )
		{
			$qinfo = $this->_queries[$n];
			if ($qinfo['type']=='sphinxql')
                $bOk = $this->RunQueryQL ( $qinfo, $error, $examples, $benchmark, $ql_connection, $old_agent, $old_vip );
			elseif ($qinfo['type']=='query')
                $bOk = $this->RunQueryHttp ( $qinfo, $error, $examples );
			elseif ( $qinfo['type']=='comment') {
                $this->PickResult(array("comment" => trim($qinfo["query"]), 'skip' => 1, 'hide' => $this->ToHide($qinfo) ));
                $bOk = true;
            } elseif ($qinfo['type']=='apiupdate')
                $bOk = $this->RunQueryApiUpdate ( $qinfo, $error, $examples );

            if (!$bOk)
			    break;

            if ($qinfo['type']=='api')
			foreach ( $qinfo['query'] as $query ) // in bench query might be loaded as big set of strings, so it's array.
			{
				if ( $benchmark && MyMicrotime() > $tm )
				{
					$tm = MyMicrotime();
					$est = $done ? ( ( $tm - $start ) / $done ) * ( $total - $done ) : 0 ;
					$qps = $done / ( $tm - $start );
					printf ( "\r$prefix %d/%d (est. %s, qps %.1f)", $done, $total, sphFormatTime($est), $qps );
					$tm += 1;
				}
				$bOk = FALSE;
				for ( $i=0; $i<$retries && !$bOk; $i++ )
				{
					if ( !$pconn )
					{
						$cl = new APIClient();
						$cl->SetServer ( $sd_address, $sd_port );
					} else
					{
						$cl->ResetFilters ();
						$cl->ResetGroupBy ();
					}

					$results = 0;
					if ( empty($this->_query_settings) )
					{
						if ( @$qinfo["mode"] )		$cl->SetMatchMode ( $qinfo["mode"] );
						if ( @$qinfo["ranker"] )	$cl->SetRankingMode ( $qinfo["ranker"] );
						if ( @$qinfo["sortmode"] )	$cl->SetSortMode ( $qinfo["sortmode"], $qinfo["sortby"] );
						if ( @$qinfo["groupattr"] )	$cl->SetGroupBy ( $qinfo["groupattr"], $qinfo["groupfunc"], $qinfo["groupsort"] );
						if ( @$qinfo["groupdistinct"] )	$cl->SetGroupDistinct ( $qinfo["groupdistinct"] );
						if ( @$qinfo["resarray"] )	$cl->SetArrayResult ( true );
						if ( @$qinfo["select"] )	$cl->SetSelect ( $qinfo["select"] );
						if ( @$qinfo["index"] )		$my_index = $qinfo["index"];
						if ( @$qinfo["index_weights"] ) $cl->SetIndexWeights ( $qinfo["index_weights"] );
						if ( @$qinfo["cutoff"] )		$cl->SetLimits ( 0, 20, 0, $qinfo["cutoff"] );
						if ( @$qinfo["limits"] )		$cl->SetLimits ( 0, (int)$qinfo["limits"] );
						if ( @$qinfo["filter"] )
						{
							$name = $qinfo["filter"];
							$exclude = false;
							if ( isset ( $qinfo["filter_exclude"] ) && $qinfo["filter_exclude"]=="1" )
								$exclude = true;
							if ( @$qinfo["filter_value"] )
								$cl->SetFilter ( $name, array ( $qinfo["filter_value"] ), $exclude );
							elseif ( @$qinfo["filter_range"] )
							{
								$range = $qinfo["filter_range"];
								$cl->SetFilterRange ( $name, $range['min'], $range['max'], $exclude );
							} elseif ( @$qinfo["filter_str"] )
								$cl->SetFilterString ( $name, $qinfo["filter_str"], $exclude );
						}

						$results = $cl->Query ( $query, $my_index, "run".(1+$this->SubtestNo()) );
						if ( is_array($results) )
						{
							$results["resarray"] = (int)@$qinfo["resarray"];
							$results["roundoff"] = (int)@$qinfo["roundoff"];
						}
					}
					else
					{
						$run_func = function( $client, $query, $index, &$results ) { eval( "$this->_query_settings" ); };
						$run_func ( $cl, $query, "*", $results );
					}

					if ( $results )
					{
						// let also work with "array of arrays" result
						if ( array_key_exists ( "error",$results ) )
						{
							$bOk = TRUE;
							if ( $compact )
								$results = array ( $n, $results['total'], $results['total_found'], $results['time'] );
							else
								$results ["query"] = $query;

                            $this->PickResult ( $results );
						} else
						foreach ( $results as $result )
						{
							$bOk = TRUE;
							if ( $compact )
								$result = array ( $n, $result['total'], $result['total_found'], $result['time'] );
							else
								$result ["query"] = $query;

                            $this->PickResult ( $result );
						}
					}
					else if ( !$cl->IsConnectError() )
					{
						$bOk = true;
                        $this->PickResult ( array (
							"query" => $query,
							"error" => $cl->GetLastError(),
							"warning" => "",
							"total" => 0,
							"total_found" => 0,
							"time" => 0 ) );
					}
					else
					{
						if ( method_exists ( $cl, 'IsConnectError' ) && $cl->IsConnectError() )
							usleep ( $action_wait_timeout );
						else if ( $benchmark && $done )
						{
                            $this->PickResult ( array ( $n, -1, -1, 0 ) );
							$bOk = true;
						}
						else
							break;
					}
				}
				$done++;

				if ( !$bOk )
				{
					$error = sprintf ( "query %d/%d: %s", $n+1, count($this->_queries), $cl->GetLastError() );
					return FALSE;
				}
			}
		}

        if ( $ql_connection )
            mysqli_close ( $ql_connection );

		if ( $benchmark )
			printf ( " - done in %s\n", sphFormatTime ( MyMicrotime() - $start ) );

		if ( $pconn )
			$cl->Close ();

		return $bOk;
	}

	function RunCustomTest ( & $error )
	{
		global $sd_address, $sd_port, $action_retries, $action_wait_timeout, $g_locals;

		$bOk = false;
		$results = false;

		for ( $i = 0; $i < $action_retries && !$bOk; $i++ )
		{
			$cl = new APIClient;
			$cl->SetServer ( $sd_address, $sd_port );

			$results = false;
			$run_func = function( $client, $ql, &$results ) { eval( $this->_custom_test ); };

			if ( !LegacyConnectDB())
				return FALSE;

			$GLOBALS["this_test"] = $this->_testdir;
			$ql = new QLClient();
			$run_func ( $cl, $ql, $results );


			@mysql_close();
			$gl_conn = NULL;

			if ( $results )
				$bOk = TRUE;
			else
				usleep ( $action_wait_timeout );
		}

		if ( !$bOk )
		{
			$error = $cl->GetLastError ();
			return FALSE;
		}

		$my_results = array ();
		$my_results [] = $results;

        $this->PickResults ( $my_results );
		return TRUE;
	}


	function FixKeys ( $v )
	{
		if ( is_array($v) )
		{
			$result = array();
			foreach ( $v as $key=>$value )
			{
				if ( $key==PHP_INT_MAX || $key==-PHP_INT_MAX-1 )
					$key = (int)$key;
				$result[$key] = $this->FixKeys ( $value );
			}
			return $result;
		}
		else
			return $v;
	}


	function IsBigNum ( $v )
	{
		return is_int($v) && ( $v>2147483647 || $v<-2147483648 );
	}


	function FixSerialize64 ( $v, & $fixarr )
	{
		if ( is_array($v) )
		{
			foreach ( $v as $key=>$value )
			{
				if ($this->IsBigNum($key))
					$fixarr[] = $key;
				$this->FixSerialize64 ( $value, $fixarr );
			}
		} else if ($this->IsBigNum($v))
			$fixarr[] = $v;
	}


	function LoadModel ( $filename )
	{
		if ( ! IsModelGenMode () )
		{
			if ( ! file_exists ( $filename ) )
				return -1;

			$contents = file_get_contents ( $filename );
			if ( ! $contents )
				return 0;

			// linux and windows line ending breaks model at different boxes
			$fixed_data = preg_replace_callback('!s:\d+:"(.*?)";!s', function($m) { return "s:" . strlen($m[1]) . ':"'.$m[1].'";'; }, $contents);
			
			$this->_results_model = $this->FixKeys ( unserialize ( $fixed_data ) );
		}

		return 1;
	}

	function ImportantResults()
    {
        $_results			= array ();
        foreach ( $this->_results as $result )
            if ( !result_must_be_skipped ($result) )
                $_results[] = $result;
        return $_results;
    }

	function CompareToModel ()
	{
		return $this->CompareResults ( $this->FixKeys ( $this->ImportantResults () ), $this->_results_model [$this->SubtestNo ()] );
	}


	function CompareResultSets ( $set1, $set2 )
	{

		$roundoff = 0;
		if ( isset($set1["roundoff"]) ) $roundoff = $set1["roundoff"];
		if ( isset($set2["roundoff"]) ) $roundoff = $set2["roundoff"];
		
		$variants_match = $this->Requires("variant_match");
		$keep_json_ctrls = $this->Requires("keep_json_ctrls");

		CompareResultSetFixup ( $set1, $roundoff, $variants_match, $this->IsColumnar(), $keep_json_ctrls );
		CompareResultSetFixup ( $set2, $roundoff, $variants_match, $this->IsColumnar(), $keep_json_ctrls );

		return $set1==$set2;
	}

	function CompareResults ( $query1, $query2 )
	{
		if ( count($query1)!=count($query2) )
			return false;

		for ( $i=0; $i<count($query1); $i++ )
			if ( !$this->CompareResultSets ( $query1[$i], $query2[$i] ) )
				return false;

		return true;
	}


	/// returns false if everything is okay
	/// returns error messages if something failed
	function CheckVariants ( $output_path )
	{
		if ( !$this->Requires("variant_match") )
			return false;
		
		$total = count ( $this->_results_model );
		if ( $total==1 )
			return "variant match required, but there are no variants";
		else if ( !$this->IsQueryTest() )
			return "variant match is not supported with custom tests";

		$failed = false;
		$output = '';
		for ( $i=1; $i<$total; $i++ )
		{
			$nqueries = count ( $this->_results_model[0] );
			for ( $k=0; $k<$nqueries; $k++ )
			if ( !$this->CompareResultSets ( $this->_results_model[0][$k], $this->_results_model[$i][$k] ) )
			{
				$first = $this->FormatResultSet ( $k+1, $this->_results_model[0][$k], array("format_attrs"=>1) );
				$current = $this->FormatResultSet ( $k+1, $this->_results_model[$i][$k], array("format_attrs"=>1) );

				file_put_contents ( "first", $first );
				file_put_contents ( "current", $current );
				system ( "diff --unified=3 first current > diff.txt" );
						
				$diff = file_get_contents ( "diff.txt" );
				unlink ( "current" );
				unlink ( "first" );
				unlink ( "diff.txt" );

				$output .= $diff . "\n";
				$failed = true;
			}
		}

		if ( $failed )
		{
			file_put_contents ( $output_path, $output );
			return "variants mismatch; see $output_path for details";
		}

		// all ok, indicated by false ("no error")
		return false;
	}


	function WriteReportHeader ( $fp )
	{
		fprintf ( $fp, "==== Run %d ====\n", $this->SubtestNo () + 1 );
		fwrite ( $fp, "Settings:\n" );
		$this->WriteDiff ( $fp );
		fwrite ( $fp, "\n" );

		if ( !empty ( $this->_query_settings ) )
			fprintf ( $fp, "Query settings:\n%s\n", $this->_query_settings );
	}

    // At model generation all comments and statements displayed
    // At testing comments with 'hide', and statements with 'skip' are not displayed.
	function FormatResultSet ( $nquery, $result, $opts=array() )
	{
		global $sd_skip_indexer;
		if (@$opts['hide'] && result_must_not_be_displayed ($result))
		    return '';

        if ( array_key_exists ("comment", $result) )
            return "/* " . $result["comment"] . " */\n\n";

		if ( array_key_exists ("http", $result) )
			return HttpFormatResultSet ( $result, $nquery, $this->Requires("keep_json_ctrls") );

        if ( array_key_exists ('updated', $result) )
            return "/* Updated via SphinxAPI: " . $result['updated'] . " */\n\n";

		if ( !$this->IsQueryTest () || !is_array($result) )
			return var_export ( $result, true )."\n";

		$skipped_result = result_must_be_skipped($result);

		if ( array_key_exists ("sphinxql", $result) )
		{
			$str = $skipped_result?"sphinxql":"sphinxql-$nquery";
			if ( array_key_exists ("vip", $result ) )
				$str.=" (vip)";
			if ( array_key_exists ("agent", $result ) )
				$str.=" (agent-" . $result["agent"] . ")" ;
			$str.= "> $result[sphinxql];\n";
			if ( array_key_exists ("total_affected", $result) )
			{
				$str .= "Query OK, $result[total_affected] rows affected\n";

			} else if ( array_key_exists ("error", $result) )
			{
				$str .= "ERROR $result[errno]: $result[error]\n";

			} else if (array_key_exists ("rows", $result) )
			{
				foreach ( $result["rows"][0] as $key=>$s )
					$str .= "\t$key";
				$str .= "\n";
				foreach ($result["rows"] as $row)
				{
					foreach ($row as $value)
					{
						if ( $this->Requires("sphinxql_keep_null") && is_null ( $value ) )
							$value = 'NULL';
						$str .= "\t$value";
					}
					$str .="\n";
				}
				$str .="$result[total_rows] rows in set\n";

			} else if ( isset($result["total_rows"]) )
			{
				$str .= "$result[total_rows] rows in set\n";
			}
			return $str."\n";
		}

		// format header
		$qinfo = @$this->_queries[$nquery-1];
		while ( $nquery>0 && (!@array_key_exists ( "type", $qinfo) || $qinfo['type'] != 'api' ))
        {
            --$nquery;
            $qinfo = @$this->_queries[$nquery-1];
        }
        $str = "";
		if ( $qinfo )
		{
			if ( @array_key_exists ( "index", $qinfo ) && $qinfo ["index"] != '*' )
				$str .= "--- Query $nquery (mode=$qinfo[mode_s],ranker=$qinfo[ranker_s],index=$qinfo[index]) ---\n";
			else
				$str .= "--- Query $nquery (mode=$qinfo[mode_s],ranker=$qinfo[ranker_s]) ---\n";

            if (@$qinfo["groupattr"])
                $str .= "GroupBy: attr: '" . $qinfo["groupattr"] . "' func: '" . $qinfo["groupfunc_s"] . "' sort: '" . $qinfo["groupsort"] . "'\n";

            if (@$qinfo["sortmode"] == SPH_SORT_EXPR)
                $str .= "Sort: expr: " . $qinfo["sortby"] . "\n";
        }

		$str .= @"Query '$result[query]': retrieved $result[total_found] of $result[total] matches in $result[time] sec.\n";
		if ( @array_key_exists ( "error", $result ) && $result["error"] )
			$str .= "Error: $result[error]\n";
		if ( @array_key_exists ( "warning", $result ) && $result["warning"] )
			$str .= "Warning: $result[warning]\n";

		$array_result = @$result["resarray"];

		// format keywords
		if ( isset($result["words"]) && is_array($result["words"]) )
		{
			$str .= "Word stats:\n";
			foreach ( $result ["words"] as $word => $word_result )
			{
				$hits = $word_result ["hits"];
				$docs = $word_result ["docs"];
				$str .= "\t'$word' found $hits times in $docs documents\n";
			}
		}

		// format attribute types
		if ( @$opts["format_attrs"] )
		{
			$typenames = array (
				SPH_ATTR_INTEGER => "int",
				SPH_ATTR_TIMESTAMP=> "timestamp",
				SPH_ATTR_ORDINAL => "ordinal",
				SPH_ATTR_BOOL => "bool",
				SPH_ATTR_FLOAT => "float",
				SPH_ATTR_DOUBLE => "double",
				SPH_ATTR_BIGINT => "bigint",
				SPH_ATTR_STRING => "string",
				SPH_ATTR_MULTI => "mva",
				SPH_ATTR_MULTI64 => "mva" ); // !COMMIT

			$n = 1;
			$str .= "Result set attributes:\n";
			foreach ( $result["attrs"] as $name=>$type )
			{

				$typename = "type-$type";
				if ( $typenames[$type] )
					$typename = $typenames[$type];

				$str .= "\tattr $n: $typename $name\n";
				$n++;
			}
		}

		// check our table for well-known id column names
		$idcol = "";

		if ( $this->IsNeedDB() )
			$r = $this->_connection->query ( "DESC test_table" );
		else
			$r = false;
		if ( $r )
		{
			while ( $row = $r->fetch_assoc() )
			{
				$idcand = strtolower ( $row["Field"] );
				if ( in_array ( $idcand, array ( "id", "document_id" ) ) )
				{
					$idcol = $idcand;
					break;
				}
			}
		}

		// format matches
		$str .= "\n";
		if ( isset($result["matches"]) && is_array($result["matches"]) )
		{
			$n = 1;
			$str .= "Matches:";
			foreach ( $result ["matches"] as $doc => $docinfo )
			{
				$doc_id = $array_result ? $docinfo["id"] : $doc;
				$weight = $docinfo["weight"];

				$str .= "\n$n. doc_id=$doc_id, weight=$weight";
				$n++;

				// only format specified attrs if requested
				if ( !empty ( $this->_query_attributes ) )
				{
					foreach ( $this->_query_attributes as $attr )
						if ( isset($docinfo ["attrs"][$attr]) )
					{
						$val = $docinfo["attrs"][$attr];
						if ( is_array ( $val ) )
							$val = join ( " ", $val );
						$str .= " $attr=$val";
					}
					continue;
				}

				// format attrs
				foreach ( $docinfo["attrs"] as $attr=>$val )
				{
					if ( is_array($val) )
						$val = join ( ",", $val );
					$str .= " $attr=\"$val\"";
				}
			}
			$str .= "\n\n";
		}

		return $str . "\n";
	}

	/// format and write a single result set into log file
	function WriteQuery ( $fp, $nquery, $result, $hide=false )
	{
		$res_fmt = $this->FormatResultSet ( $nquery, $result,array('hide'=>$hide) );
		fwrite ( $fp, $res_fmt );
	}

	/// write all the result sets
	function WriteResults ( $fp, $hide )
	{
		if ( $this->IsQueryTest () || $this->Requires("http") || $this->Requires("https") )
		{
			$nquery = 1;
			foreach ( $this->_results as $result ) {
                $this->WriteQuery($fp, $nquery, $result, $hide);
                if (!result_must_be_skipped($result))
                    ++$nquery;
            }
		}
		else
			$this->WriteCustomTestResults ( $fp );
	}

	/// write difference from the reference result sets
	function WriteReferenceResultsDiff ( $fp )
	{
		$nquery = 0;
		if ( !is_array ( $this->_results_model [ $this->SubtestNo() ] ) )
			return;

		fwrite ( $fp, "Run settings:\n" );
		$this->WriteDiff ( $fp );
		fwrite ( $fp, "\n" );

        $_results = $this->ImportantResults ();

		foreach ( $this->_results_model [ $this->SubtestNo() ] as $ref )
		{
			if (!array_key_exists ($nquery,$_results))
			{
				printf ( "FAILED, model has more results than current test.\n" );
				break;
			}

			$cur = $_results[$nquery];
			if ( $this->CompareResultSets ( $ref, $cur ) )
			{
				$nquery++;
				continue;
			}

			$opts = array();
			if ( isset($cur["attrs"]) || isset($ref["attrs"]) )
				if ( @$cur["attrs"]!=@$ref["attrs"] )
					$opts["format_attrs"] = 1;

			$result_f_cur = $this->FormatResultSet ( $nquery+1, $_results[$nquery], $opts );
			$result_f_ref = $this->FormatResultSet ( $nquery+1, $ref, $opts );
			file_put_contents ( "current", $result_f_cur );
			file_put_contents ( "reference", $result_f_ref );
			system ( "diff --unified=3 reference current > diffed.txt" );

			$diffed = file_get_contents ( "diffed.txt" );
			unlink ( "current" );
			unlink ( "reference" );
			unlink ( "diffed.txt" );

			$nquery++;
			fwrite ( $fp, "=== query $nquery diff start ===\n" );
			fwrite ( $fp, $diffed );
			fwrite ( $fp, "=== query $nquery diff end ===\n" );
		}

		$nref = count ( array_keys ( $this->_results_model [ $this->SubtestNo() ] ) );
		$nres = count ( array_keys ( $_results ) );
		if ( $nres > $nref )
		{
			$delta = $nres - $nref;
			fwrite ( $fp, "$delta result set(s) missing from model!\n" );
		}
	}

	function EraseIndexFiles ( $path )
	{
		$dh = glob ( "$path.*" );
		foreach ( $dh as $entry )
		{
			if ( is_file ($entry) )
				unlink ($entry);
		}
	}

	function WriteConfig ( $filename, $agentid, &$msg, $collectdata = true )
	{
		global $g_locals;
		$fp = fopen ( $filename, 'w' );
		if ( !$fp )
		{
			$msg = "Can't open file $filename for writing";
			return FALSE;
		}

		$this->Dump ( $this->_config, $fp, false, $agentid );
		fclose ( $fp );

		$fp = fopen ( $filename, 'r' );
		if ( !$fp )
		{
			$msg = "Can't open file $filename for reading";
			return FALSE;
		}

		$config = fread ( $fp, filesize ( $filename ) );
		fclose ( $fp );

		// for rt case - extract the schema from the config
		// and make the new config, making the index as rt instead
		if ( $this->IsRt() )
		{
			$body = 1;
			$srcname = 2;
			$parent = 4;
			$content = 5;
			$epilog = 6;
			$pattern = "/.*?(source\s+(\S*?)(\s*\:\s*(\S*?))?\s*\{(.*?)\n\s*\})(.*?)/s";
			preg_match_all ( $pattern, $config, $matches, PREG_SET_ORDER | PREG_OFFSET_CAPTURE );
			$schemas = array();
			$shift = 0;
			$newconfig = "";

			// parse sources
			foreach ( $matches as $match )
			{
				// split to lines, taking into account multiline variables
				$lines = explode("\n", str_replace("\\\n", "", $match[$content][0]));

				$insert_schema = array();
				$insert_types = array();
				$insert_values = array();
				$schema_bits = array();
				$sql_attr_multi = array();
				$sql_query_pre = array();
				$sql_query = "";
				$sql_query_range = "";
				$sql_file_fields = array();

				if ( $match[$parent][0] != "" )
				{
					$insert_types = $schemas[$match[$parent][0]]['types'];
					$insert_schema = $schemas[$match[$parent][0]]['orders'];
					$sql_attr_multi = $schemas[$match[$parent][0]]['multi'];
					$sql_query_pre = $schemas[$match[$parent][0]]['pre'];
					$sql_query = $schemas[$match[$parent][0]]['query'];
					$sql_query_range = $schemas[$match[$parent][0]]['range'];
				}

				foreach ( $lines as $line )
				{
					// skip comment lines (if any)
					if ( preg_match ( "/\s*#/" , $line ) > 0 )
						continue;

					// extract config key/value pairs
					$eq = strpos ( $line,"=" );
					if ($eq == 0)
						continue;
					$key = strtolower ( trim ( substr($line,0,$eq), " \t" ) );
					$value = trim ( substr($line,$eq+1), " \t" );
					$lcvalue = strtolower($value);

					// handle known keys
					switch ( $key )
					{
						case "type":
							if ( $value != "mysql" )
							{
								$msg = "non-mysql source (type=$value), skipping...";
								return FALSE;
							}
							break;

						case "sql_attr_uint":
						case "sql_attr_bigint":
							if (strstr($lcvalue, ":")===false)
							{
								$attr = $lcvalue;
								$bits = "";
							} else
							{
								$attr = strstr($lcvalue, ":", true); // true == extract the part before needle, ie. attr name
								$bits = strstr($lcvalue, ":");
							}
							$insert_types[$attr] = "rt_" . substr($key, 4);
							$schema_bits[$attr] = $bits;
							break;
						case "sql_attr_float":		$insert_types[$lcvalue] = "rt_attr_float"; break;
						case "sql_attr_timestamp":	$insert_types[$lcvalue] = "rt_attr_timestamp"; break;
						case "sql_attr_bool":		$insert_types[$lcvalue] = "rt_attr_bool"; break;
						case "sql_attr_json":		$insert_types[$lcvalue] = "rt_attr_json"; break;
						case "sql_attr_string":		$insert_types[$lcvalue] = "rt_attr_string"; break;
						case "sql_field_string":	$insert_types[$lcvalue] = "FIELD"; break;
						case "sql_attr_multi":		$sql_attr_multi[] = $value; break;
						case "sql_query_pre":		$sql_query_pre[] = $value; break;
						case "sql_query":			$sql_query = $value; break;
						case "sql_query_range":		$sql_query_range = $value; break;
						case "sql_file_field":		$sql_file_fields[] = $value; break;
					}
				}

				// sql query is not mandatory (e.g. in parent sections)
				if ( $sql_query )
				{

					// now let's connect to MySQL, run the query, and fetch the values
					$conn = ConnectDB();
					if (mysqli_connect_error()) {
						$msg = "can't connect or select the database";
						return false;
					}

					// gotta run pre-queries first!
					foreach ( $sql_query_pre as $q )
					{
						if ( mysqli_wr ( $q, $conn ) )
							continue;

						$msg = sprintf ( "sql_query_pre failed (query=%s, error=%s)", $q, $conn->error );
						$conn->close ();
						return false;
					}

					// copy original query
					$sql = $sql_query;

					// apply query range
					if ( $sql_query_range )
					{
						$res = mysqli_wr ( $sql_query_range, $conn );
						if ( !$res )
						{
							$msg = sprintf ( "sql_query failed (query=%s, error=%s)", $sql_query, $conn->error );
							$conn->close();
							return false;
						}

						$range_values = array();
						while ( $row = $res->fetch_row() )
							$range_values[] = array_values ( $row );

						if ( count($range_values)>=1 )
						{
							$sql = str_replace ( '$start', $range_values[0][0], $sql );
							$sql = str_replace ( '$end', $range_values[0][1], $sql );
						}
					}

					// run main query
					$res = mysqli_wr ( $sql_query, $conn );
					if ( !$res )
					{
						$msg = sprintf ( "sql_query failed (query=%s, error=%s)", $sql_query, $conn->error );
						$msg = "sql_query can't fetch test data: " . $conn->error;
						$conn->close ();
						return false;
					}

					// fetch fields
					$insert_schema = array ( "id" => 0 );
					for ( $i=1; $i < $conn->field_count; $i++ )
						$insert_schema [ $res->fetch_field_direct ( $i )->name ] = $i;

					// fetch data
					while ( $row = $res->fetch_row() )
						$insert_values[] = array_values ( $row );

					// cleanup
					$res->close();

					// parse mva statements
					foreach ( $sql_attr_multi as $q )
					{
						$stmt = preg_split ( "/\s*;\s*/", $q, 2 );
						$sql_query = count($stmt)>1 ? $stmt[1] : "";

						if ( !preg_match ( "/^\s*([^\s]+)\s+([^\s]+)\s+from\s+([^\s]+)/", $stmt[0], $regs) )
						{
							$msg = sprintf ( "invalid mva statement %s\n", $value );
							return false;
						}

						list ( $dummy, $attr_type, $attr_name, $source_type ) = $regs;
						$attr_name = strtolower ( $attr_name );
						$multi_type = ( $attr_type=="uint" ? "rt_attr_multi" : ( $attr_type=="bigint" ? "rt_attr_multi_64" : "rt_attr_timestamp" ) );
						$insert_types[$attr_name] = $multi_type;

						if ( $source_type=="query" )
						{
							// query mva_pairs
							$res = mysqli_wr ( $sql_query, $conn );
							if ( $res == false )
							{
								$msg = sprintf ( "sql_query failed (query=%s, error=%s)", $sql_query, $conn->error );
								$conn->close();
								return false;
							}

							// fetch mva pairs (id, value), group by id
							$mva =  array();
							while ( $row = $res->fetch_row() )
							{
								list ( $id, $value ) = array_values ( $row );
								$mva[$id][] = $value;
							}

							// cleanup mysqli
							$res->close();

							// add mva attribute to the schema
							$attr_index = count ( $insert_schema );
							$insert_schema[$attr_name] = $attr_index;

							// insert mva values separated with commas
							for ( $i=0; $i<count($insert_values); $i++ )
							{
								$id = $insert_values[$i][0];
								$insert_values[$i][] = array_key_exists ( $id, $mva ) ? implode ( ',', $mva[$id] ) : "";
							}
						}
						else if ( $source_type=="field" )
						{
							// get field offset
							$attr_index = $insert_schema[$attr_name];

							// insert mva values
							for ( $i=0; $i<count($insert_values); $i++ )
							{
								$mva = preg_replace ( "/[a-z\(\)]/i", " ", $insert_values[$i][$attr_index] );
                                $mva = preg_split ( "/[\s,]+/", $mva, -1, PREG_SPLIT_NO_EMPTY );
                                $insert_values[$i][$attr_index] = implode ( ',', $mva );
							}
						}
					}

					// cleanup
					$conn->close();

					// load files
					foreach ($sql_file_fields as $ff)
					{
						$index = $insert_schema[$ff]; // FIXME? what if it does not exist?
						for ($i=0; $i<count($insert_values); $i++)
							$insert_values[$i][$index] = @file_get_contents($insert_values[$i][$index]); // FIXME? handle errors
					}


				}

				// store
				$schema = array();
				$schema['types'] = $insert_types;
				$schema['bits'] = $schema_bits;
				$schema['orders'] = $insert_schema;
				$schema['multi'] = $sql_attr_multi;
				$schema['pre'] = $sql_query_pre;
				$schema['query'] = $sql_query;
				$schema['range'] = $sql_query_range;
				$schema['values'] = $insert_values;
				$schema['sqlport'] = $this->_sd_sphinxql_port;
				$schema['http_port'] = $this->_sd_http_port;

				$schemas[$match[$srcname][0]] = $schema;
				$srclen = $match[$epilog][1] - $match[$body][1];
				$config = substr_replace ( $config, "", $match[$body][1]-$shift,$srclen );
				$shift += $srclen;
			}

			$body = 1;
			$idxname = 2;
			$parent = 4;
			$content = 5;
			$epilog = 6;
			$pattern = "/.*?(index\s+(\S*?)(\s*\:\s*(\S*?))?\s*\{(.*?)\n\s*\})(.*?)/s";
			preg_match_all ( $pattern, $config, $matches, PREG_SET_ORDER | PREG_OFFSET_CAPTURE );
			$shift = 0;
			// parse indexes
			$indexes = array();
			$sources = array();
			foreach ( $matches as $match )
			{
				$idx = "index ".$match[$idxname][0];
				if ( $match[$parent][0] != "" )
					$idx .= " : ".$match[$parent][0];
				$idx .= "\n{\n\tdict = keywords\n";

				//source could be inherited
				if ( !strpos ( $match[$content][0],"source" ) )
					if ( array_key_exists ( $match[$parent][0], $sources ) )
						$match[$content][0] .= "\nsource = ".$sources[$match[$parent][0]];

				$lines = explode("\n", str_replace("\\\n", "", $match[$content][0]));
				$justcopy = false;
				$rtcopy = false;
				$attr_names = array();
				$idxbody = "";
				foreach ($lines as $line)
				{
					$eq = strpos ( $line,"=" );
					if ($eq == 0)
						continue;
					$key = strtolower ( trim ( substr($line,0,$eq), " \t" ) );
					$value = trim ( substr($line,$eq+1), " \t" );

					switch ( $key )
					{
						case "type":
							if ($value=="rt")
								$rtcopy = true;
							else
								$justcopy = true;
							break;
						case "source";
							{
								$idxbody .= "\ttype\t= rt\n";
								if ( $collectdata )
									$indexes[$match[$idxname][0]] = $schemas[$value];
								foreach ( array_keys( $schemas[$value]['orders'] ) as $key )
									if ( $key != "id" && $key != "document_id" )
									{
										if ( array_key_exists ( $key, $schemas[$value]['types'] ) )
										{
											$rt_attr_type = $schemas[$value]['types'][$key];
											if ( $rt_attr_type!="rt_attr_json" )
												$attr_names[] = $key;

											if ( $rt_attr_type == "FIELD" )
											{
												$idxbody .= "\trt_field\t= $key\n";
												$idxbody .= "\trt_attr_string\t= $key\n";
											} else
											{
												$idxbody .= "\t".$rt_attr_type."\t= $key";
												if (isset($schemas[$value]['bits'][$key]))
													$idxbody .= $schemas[$value]['bits'][$key];
												$idxbody .= "\n";
											}
										} else
											$idxbody .= "\trt_field\t= $key\n";
									}
								$sources[$match[$idxname][0]] = $value;
								break;
							}
						case "path":
							$this->EraseIndexFiles($value);
							// need different index paths at every agent
							if ( $agentid!=0 )
								$value .= "_$agentid";
							if ($rtcopy)
								$justcopy = true;
							// no break!
						default:
							$idxbody .= "\t$key\t= $value\n";
					}
					if ( $justcopy ) // explicitly defined type, don't transform to rt.
					{
						$idxbody = $match[$content][0];
						break;
					}
				}

				if ( $this->IsColumnar() )
				{
					$attr_names[] = "id";
					$columnar_attrs = "\n\tcolumnar_attrs=".implode ( ",", array_unique($attr_names) );
					$idxbody.= "$columnar_attrs";
				}

				$idx .= "$idxbody\n}\n";
				$srclen = $match[$epilog][1] - $match[$body][1];
				$config = substr_replace ($config, $idx, $match[$body][1]-$shift,$srclen );
				$shift += $srclen-strlen($idx);
			}
			if ( $collectdata )
				foreach ($indexes as $key => $value)
					$this->_indexdata[$key] = $value;
			$fp = fopen ( $filename, 'w' );
			if ( !$fp )
			{
				$msg = "Can't open $filename for writing";
				return FALSE;
			}
			fwrite ( $fp, $config );
			fclose ( $fp );
		}
		else // for rt indexes we need to clean up all index files before the run.
		{
			// remove binlog files if any
			$this->EraseIndexFiles ( 'data/binlog' );
			
			$pattern = "/.*?index\s+\S*?(\s*\:\s*\S*?)?\s*\{(.*?)\}.*?/s";
			preg_match_all ( $pattern, $config, $matches, PREG_SET_ORDER | PREG_OFFSET_CAPTURE );
			// parse indexes
			$indexes = array();
			foreach ( $matches as $match )
			{
				$lines = explode("\n", $match[2][0]);
				$path = "";
				$isrt = false;
				foreach ($lines as $line)
				{
					// skip comment lines (if any)
					if ( preg_match ( "/\s*#/" , $line ) > 0 )
						continue;
					$eq = strpos ( $line,"=" );
					if ($eq == 0)
						continue;
					$key = strtolower ( trim ( substr($line,0,$eq), " \t" ) );
					$value = trim ( substr($line,$eq+1), " \t" );
					switch ( $key )
					{
						case "type":
							if ($value=="rt")
								$isrt = true;
							break;
						case "path":
							$path = $value;
					}
					if ( $isrt && $path!="" )
					{
						$this->EraseIndexFiles($path);
						break;
					}
				}
			}
		}
		return TRUE;
	}

	function InsertIntoIndexer ( &$error )
	{
		global $sd_address, $sd_sphinxql_port, $action_retries, $action_wait_timeout;
		$address = $sd_address;
		if ($address == "localhost")
			$address = "127.0.0.1";

		$cn = false;
		$port = 0;
		foreach ( $this->_indexdata as $name => $data )
		{
			if ( $port != $data["sqlport"] )
			{
				$port = $data["sqlport"];
				$connect_string = "$address:$port";
				if ( $cn !== false )
					$cn->close();
				$cn = new mysqli( $address, "", "", "", $port );
			}
			if ( $cn === false )
				return false;

			$corrected_cols = array();
			foreach ( array_keys($data["orders"]) as $key )
				$corrected_cols[] = '`'.$key.'`';

			$cols = join ( ", ", $corrected_cols );
			$prefix = "INSERT INTO $name ($cols) VALUES ";

			$accum = "";

			// mva shouldn't be quoted, e.g. "insert into rt (id,gid,mva) values ('1','2',(1,2))"
			$is_mva = array();
			foreach ( array_keys($data["orders"]) as $key )
				$is_mva[] = array_key_exists ( $key, $data["types"] ) && ( $data["types"][$key]=="rt_attr_multi" || $data["types"][$key]=="rt_attr_multi_64" );

			foreach ($data['values'] as $row)
			{
				$query = "";
				$i = 0;
				foreach ( $row as $column )
				{
					if ( $query!="" )
						$query .=",";
					$s = $cn->real_escape_string($column);
					$query .= ( array_key_exists ( $i, $is_mva ) && $is_mva[$i] ) ? '('.$s.')' : "'".$s."'";
					$i++;
				}
				
				if ( ( strlen ($accum) + strlen ($query) ) > 8192000 ) ///<checkit!
				{
					$result = mysqli_wr ( $prefix.$accum, $cn );
					if ( $result === false )
					{
						$error = $cn->error;
						return false;
					}
					$accum="";
				}

				if ( $accum != "" )
					$accum .=",";
				$accum .= "($query)";
			}
			// final chunk;
			if ( $accum !="" )
			{
				$result = mysqli_wr ( $prefix.$accum, $cn );
				if ( $result === false )
				{
					$error = $cn->error;
					return false;
				}
			}
		}
		if ( $cn )
			$cn->close();
		return true;
	}

	function WriteDiff ( $fp )
	{
		$this->Dump ( $this->_config, $fp, true, "all" );
	}


	function WriteModel ( $filename )
	{
		if ( IsModelGenMode () )
				{
					$result = serialize ( $this->_results_model );
					if (PHP_INT_SIZE>4)
					{
						$keys = array();
						$this->FixSerialize64 ( $this->_results_model, $keys );
						if ( sizeof($keys)>0 )
						{
							$findes=array();
							$replaces=array();
							foreach ($keys as $key)
							{
								$findes[]="i:$key";
								$replaces[]="s:".strlen($key).":\"$key\"";
							}
							$result = str_replace ( $findes, $replaces, $result );
						}
					}
					file_put_contents ( $filename, $result );
				}
	}


	function WriteSearchdSettings ( $fp )
	{
		global $sd_log, $sd_split_logs, $sd_query_log, $sd_network_timeout, $sd_max_children, $sd_pid_file;

		if ( $this->_compat098 )
		{
			fwrite ( $fp, "\taddress	= {$this->_sd_address}\n" );
			fwrite ( $fp, "\tport		= {$this->_sd_port}\n" );
		}
		else
		{
			fwrite ( $fp, "\tlisten		= {$this->_sd_address}:{$this->_sd_port}\n" );
			fwrite ( $fp, "\tlisten		= {$this->_sd_address}:{$this->_sd_sphinxql_port}:mysql41\n" );
			if ( $this->Requires("http") )
				fwrite ( $fp, "\tlisten		= {$this->_sd_address}:{$this->_sd_http_port}:http\n" );
			if ( $this->Requires("https") )
			{
				$test_root = dirname(__FILE__);
				fwrite ( $fp, "\tlisten		= {$this->_sd_address}:{$this->_sd_http_port}:https\n" );
				fwrite ( $fp, "\tssl_key = {$test_root}/ssl_keys/server-key.pem\n" );
				fwrite ( $fp, "\tssl_cert = {$test_root}/ssl_keys/server-cert.pem\n" );
			}

			if ( $this->Requires("vip") )
				fwrite ( $fp, "\tlisten		= {$this->_sd_address}:{$this->_sd_sphinxql_port_vip}:mysql41_vip\n" );
			if ( $this->Requires("replication") )
			{
				$replication_port0 = $this->_sd_replication_port;
				$replication_port1 = $this->_sd_replication_port + 18;
				fwrite ( $fp, "\tlisten		= {$this->_sd_address}:{$replication_port0}-{$replication_port1}:replication\n" );
			}
		}

		if ( $sd_split_logs )
		    fwrite ( $fp, "\tlog			= {$this->_sd_log}\n" );
		else
            fwrite ( $fp, "\tlog			= $sd_log\n" );
		fwrite ( $fp, "\tquery_log		= $sd_query_log\n" );
		fwrite ( $fp, "\tnetwork_timeout= $sd_network_timeout\n" );
		fwrite ( $fp, "\tmax_connections	= $sd_max_children\n" );
		fwrite ( $fp, "\tpid_file		= ".$this->_sd_pid_file."\n" );
		fwrite ( $fp, "#\tbinlog_path		=\n" );
	}

	function WriteSqlSettings ( $fp, $attributes )
	{
		global $g_locals;

		fwrite ( $fp, "\tsql_host		= " . $g_locals['db-host'] . "\n" );
		fwrite ( $fp, "\tsql_user		= " . $g_locals['db-user'] . "\n" );
		fwrite ( $fp, "\tsql_pass		= " . $g_locals['db-password'] . "\n" );
		fwrite ( $fp, "\tsql_port		= " . $g_locals['db-port'] . "\n" );

		if (is_null($attributes))
			return;
		$node = $attributes->getNamedItem('sql_db');
		fprintf ( $fp, "\tsql_db		= %s\n", $node ? $node->nodeValue : $g_locals['db-name'] );
	}

    function WriteOdbcSettings ( $fp, $attributes )
    {
        global $g_locals;

        // params works for official mysql connector
		// available at https://dev.mysql.com/downloads/connector/odbc/
		// after installing, provide full path to libmyodbc*.so as 'odbc_driver' local.

        $dsn=array();
        $dsn[] = "Driver=" . $g_locals['odbc_driver'];
        $dsn[] = "Server=" . $g_locals['db-host'];
        $dsn[] = "Port=" . $g_locals['db-port'];
        $dsn[] = "UID=" . $g_locals['db-user'];
        $dsn[] = "PWD=" . $g_locals['db-password'];

        if (!is_null($attributes)) {
			$node = $attributes->getNamedItem('sql_db');
			if ($node)
				$dsn[] = "Database=" . $node->nodeValue;
			else
				$dsn[] = "Database=" . $g_locals['db-name'];
		}

        $dsn = join ( ";", $dsn );
        fwrite ( $fp, "odbc_dsn		= $dsn" );
    }

    function show_settings ( $nodename )
	{
		global $index_data_path, $agents, $g_locals;
		switch ($nodename)
		{
			case "searchd_settings":	$this->WriteSearchdSettings ( STDOUT ); return;
			case "sql_settings":		$this->WriteSqlSettings ( STDOUT, null ); return;
			case "odbc_settings":       $this->WriteOdbcSettings ( STDOUT, null ); return;
			case "my_address":
			case "agent0_address":		fwrite ( STDOUT, $agents[0]["address"].":".$agents[0]["port"] ); return;
			case "my_port":				fwrite ( STDOUT, $agents[0]["port"]); return;
			case "agent_address":
			case "agent1_address":		fwrite ( STDOUT, $agents[1]["address"].":".$agents[1]["port"] ); return;
			case "agent2_address":		fwrite ( STDOUT, $agents[2]["address"].":".$agents[2]["port"] ); return;

			case "test_root":			fwrite ( STDOUT, dirname(__FILE__) ); return;
			case "testdir":	{
				if ( $g_locals['testdir'] == '' ) {
					fwrite(STDOUT, dirname(__FILE__)."/");
					return;
				}
				fwrite ( STDOUT, $g_locals['testdir'] ); return;
			}
			case "data_path": {
					fwrite(STDOUT, $index_data_path);
					return;
				}
		}
	}

	function Dump ( $node, $fp, $dynamic_only, $agentid )
	{
		global $index_data_path, $agents, $g_locals, $python;

		$nodename = strtolower ( $node->nodeName );

		if ( !$dynamic_only )
			switch ( $nodename )
		{
			case "#text":				fwrite ( $fp, $node->nodeValue ); return;
			case "static":				fwrite ( $fp, $node->nodeValue ); return;
			case "searchd_settings":	$this->WriteSearchdSettings ( $fp ); return;
			case "python":				fwrite ( $fp, $python ); return;
			case "sql_settings":		$this->WriteSqlSettings ( $fp, $node->attributes ); return;
            case "odbc_settings":       $this->WriteOdbcSettings ( $fp, $node->attributes); return;
			case "my_address":
			case "agent0_address":		fwrite ( $fp, $agents[0]["address"].":".$agents[0]["port"] ); return;
			case "my_port":				fwrite ( $fp, $agents[0]["port"]); return;
			case "agent_address":
			case "agent1_address":		fwrite ( $fp, $agents[1]["address"].":".$agents[1]["port"] ); return;
			case "agent2_address":		fwrite ( $fp, $agents[2]["address"].":".$agents[2]["port"] ); return;

			case "local":				fwrite ( $fp, $this->GetLocal ( $node->nodeValue ) ); return;
			case "test_root":			fwrite ( $fp, dirname(__FILE__) ); return;
			case "this_test":			fwrite ( $fp, $this->_testdir ); return;
			case "testdir":	{
				if ( $g_locals['testdir'] == '' ) {
					fwrite($fp, dirname(__FILE__)."/");
					return;
				}
				fwrite ( $fp, $g_locals['testdir'] ); return;
			}
			case "data_path": {
				$subdir = GetfirstAttr($node);
				if ($subdir==NULL) {
					fwrite($fp, $index_data_path);
					return;
				}
				$path = "$index_data_path/$subdir->nodeValue";
				if ( !file_exists ( $path ) )
					mkdir ( $path );
				EraseDirContents($path);
				fwrite($fp, $path);
				$agents[$agentid]["data_path"] = $subdir->nodeValue;
				return;
			}
			case "agent_id": fwrite($fp, $agentid); return;
			
		}

		if ( $nodename=="dynamic" )
		{
			if ( !is_null($node->id) )
			{
				$variants = ChildrenArray ( $node,"variant" );
				$this->Dump ( $variants[$this->_counters[$node->id]], $fp, $dynamic_only, $agentid );
			}
		} else if ( strpos ( $nodename, "agent" )===0 )
		{
				if ( $agentid==="all" || $nodename=="agent$agentid" )
					foreach ( ChildrenArray($node) as $child )
						$this->Dump ( $child, $fp, $dynamic_only, $agentid );
		} else
		{
				foreach ( ChildrenArray($node) as $child )
					$this->Dump ( $child, $fp, $dynamic_only, $agentid );
		}
	}
}

//////////////////////////////////////////////////////////////////////////

function HandleFailure ( $config, $report, $error, &$nfailed )
{
	$ret = true;
	if ( !IsModelGenMode() && !$config->ModelSubtestFailed () )
	{
		$nfailed++;
		$ret = false;

		fwrite ( $report, "SUBTEST FAILED, UNEXPECTED ERROR:\n" );
	}

	fwrite ( $report, "$error\n" );
	$config->SubtestFailed ();

	return $ret;
}


function EraseDirContents ( $path, $dir=false )
{
	$fp = opendir ( $path );

	if ( $fp )
	{
		while ( ( $file = readdir ( $fp ) ) !== false )
		{ 
		if ( $file!="." && $file!=".." && $file!="stub.txt" )
			if (!is_dir("$path/$file")) {
				unlink ( "$path/$file" );
			} else if ( $dir ) {
				EraseDirContents ( "$path/$file", $dir );
				rmdir ( "$path/$file" );
			}
		} 

		closedir ( $fp );
	}
}

function CopyDirContents ( $from, $to )
{
	$ffrom = opendir ( $from );
	
	if ( $ffrom && is_dir ( $to ) )
	{
		while ( ( $file = readdir ( $ffrom ) ) !== false )
		{
			if ( $file != "." && $file != ".." && !is_dir ( $file ) )
				copy ( "$from/$file", "$to/$file" );
		} 

		closedir ( $ffrom );
	}
}

function CheckConfig ( $config, $path )
{
	global $g_re2, $g_icu, $g_odbc, $windows, $g_locals, $mysql_simulated, $g_repli, $g_ssl, $g_columnar_loaded, $g_secondary_loaded;
	
	if ( $config->Requires("non-windows") && $windows )
	{
		printf ( "SKIPPING %s, %s - use non-Windows system to run this test\n", $path, $config->Name () );
		return false;
	}

	if ( $config->Requires("windows") && !$windows )
	{
		printf ( "SKIPPING %s, %s - use Windows system to run this test\n", $path, $config->Name () );
		return false;
	}

	if ( $config->Requires("non-rt") && $config->IsRt() )
	{
		printf ( "SKIPPING %s, %s - explicitly non-RT test skipped in RT mode\n", $path, $config->Name () );
		return false;
	}

	if ( $config->Requires("non-columnar") && $config->IsColumnar() )
	{
		printf ( "SKIPPING %s, %s - explicitly non-columnar test skipped in columnar mode\n", $path, $config->Name () );
		return false;
	}

	if ( $config->Requires("re2") && !$g_re2 )
	{
		printf ( "SKIPPING %s, %s - compile with regexp support to run this test\n", $path, $config->Name () );
		return false;
	}

	if ( $config->Requires("icu") && !$g_icu )
	{
		printf ( "SKIPPING %s, %s - compile with ICU support to run this test\n", $path, $config->Name () );
		return false;
	}

    if ( $config->Requires("odbc") )
    {
        if (!$g_odbc)
        {
            printf ( "SKIPPING %s, %s - compile with ODBC support to run this test\n", $path, $config->Name () );
            return false;
        }
        if ( !isset($g_locals["odbc_driver"]) )
        {
            printf ( "SKIPPING %s, %s - odbc_driver required, add it to your ~/.sphinx\n", $path, $config->Name () );
            return false;
        }
    }

	if ( $config->Requires("columnar") )
	{
		if ( !$g_columnar_loaded )
		{
			printf ( "SKIPPING %s, %s - columnar library not loaded\n", $path, $config->Name () );
			return false;
		}
	}

	if ( $config->Requires("php_mysql") && $mysql_simulated )
	{
		printf ( "SKIPPING %s, %s - need php_mysql (not simulated by mysqli) to run this test\n", $path, $config->Name () );
		return false;
	}

	if ( $config->NeedIndexerEx() && $config->IsRt() )
	{
		printf ( "SKIPPING %s, %s - non-RT test that uses indexer skipped in RT mode\n", $path, $config->Name () );
		return false;
	}

	if ( $config->Requires("lemmatizer_base") && !isset($g_locals["lemmatizer_base"]) )
	{
		printf ( "SKIPPING %s, %s - lemmatizer_base required, add it to your ~/.sphinx\n", $path, $config->Name () );
		return false;
	}
	
	if ( $config->Requires("replication") && !$g_repli )
	{
		printf ( "SKIPPING %s, %s - compile with replication support to run this test\n", $path, $config->Name () );
		return false;
	}

	if ( $config->Requires("https") && !$g_ssl )
	{
		printf ( "SKIPPING %s, %s - compile with SSL support or add SSL extension to php\n", $path, $config->Name () );
		return false;
	}
    
	if ( $config->Requires("secondary") )
	{
		if ( !$g_secondary_loaded )
		{
			printf ( "SKIPPING %s, %s - secondary library not loaded\n", $path, $config->Name () );
			return false;
		}
	}    

	return true;
}

function MarkTest ( $logfile, $test_dir )
{
	$log = fopen ( $logfile, "a" );
	fwrite ( $log, "*** in test $test_dir ***\n");
	fclose ( $log );
}


function RunTest ( $test_dir, $skipdemo, $usemarks )
{
	global	$index_data_path, $agents, $ss_pid_file, $sd_managed_searchd,
			$sd_skip_indexer, $windows, $g_locals, $ss_log, $ss_query_log,
			$g_pick_query;

	$res_path = scriptdir($test_dir);

	$model_file = $test_dir."/model.bin";
	$conf_dir 	= $res_path."/Conf";


	$config = new SphinxConfig;
	$lmodel = $config->LoadModel ( $model_file );
	$isdemo = false;
	$error = "";

	if ( $lmodel==-1 )
	{
		if ( $skipdemo )
		{
			printf ( "Skipping %s, - this is demo or bugreport (no model.bin file)\n", $test_dir );
			return array ( "tests_total"=>0, "tests_failed"=>0, "tests_skipped"=>1 );
		}
		$isdemo = true;
	}

	$config->SetTestDir ( getcwd()."/".$test_dir );

	if ( !$config->Load ( $test_dir."/test.xml" ) )
		return;

	$prefix = sprintf ( "testing %s, %s...", $test_dir, $config->Name () );

	if ( !CheckConfig ( $config, $test_dir ) )
		return array ( "tests_total"=>0, "tests_failed"=>0, "tests_skipped"=>1 );

	if ( $lmodel==0 )
	{
		printf ( "$prefix FAILED, error loading model\n" );
		return;
	}

	if ( $config->IsNeedDB() )
	{
		$connection = CreateDB ( $config->DB_Drop(), $config->DB_Create(), $config->DB_Insert(), $config->DB_CustomInsert(), $sd_skip_indexer, $error );
		if ( $connection === false )
		{
			printf ( "$prefix FAILED, error creating test DB: " . $error );
			return;
		}
		$config->SetConnection($connection);
	}

	if ( !file_exists ( $conf_dir ) )
		mkdir ( $conf_dir );
		
	if ( $config->Requires ( "replication" ) )
		EraseDirContents ( testdir ( "data/" ), true );
	
	$report_path = "$res_path/report";
	$report_file = "$report_path.txt";
	$report = fopen ( $report_file, "w" );

	$example_file = scriptdir("examples.txt");
	$examples = [];

	$nfailed = 0;
	$error = "";
	$log = ""; // subtest failures log
	$nsubtests = $config->SubtestCount();

	// config to pid hash, instances to stop
	// static is only to workaround PHP braindamage, otherwise $stop gets reset (at least on 5.2.2 under win32)
	static $stop = array();
	$oldlog = '';
	$oldquerylog = '';
	if ( $isdemo )
	{
		$oldlog = $ss_log;
		$oldquerylog = $ss_query_log;
		$ss_log				= "$res_path/searchd.log";
		$ss_query_log		= "$res_path/query.log";
		if (file_exists($ss_log))
			unlink ($ss_log);
		if (file_exists($ss_query_log))
			unlink ($ss_query_log);
	}

	if ( $usemarks )
	{
		MarkTest($ss_log,$test_dir);
		MarkTest($ss_query_log,$test_dir);
	}

	do
	{
		// stop them all
		if ( !$sd_managed_searchd )
			foreach ( $stop as $conf=>$pid )
				StopSearchd ( $conf, $pid );
		$stop = array();

		// do the dew
		$subtest = $config->SubtestNo()+1;
		print ( "$prefix $subtest/$nsubtests\r" );
		$config->WriteReportHeader ( $report );

		$config->SetAgent ( $agents [0], 0 );
		$msg = '';
		if (!$config->WriteConfig ( $conf_dir."/"."config_".$config->SubtestNo ().".conf", "all", $msg, false))
		{
			print ("Interrupted, $msg\n");
			continue;
		}
		$config->WriteConfig ( scriptdir("config.conf"), "all", $msg, $config->NumAgents () < 2 );

		if ( !$sd_skip_indexer )
			EraseDirContents ( $index_data_path );

		if ( $config->Requires( "pre_copy_ref" ) )
			CopyDirContents ( $test_dir . "/refdata/", testdir("data/") );

		if ( $config->IsSkipIndexer()===false && $sd_managed_searchd===false && $sd_skip_indexer===false )
		{
			// standard run
			if ( !$config->IsRt() )
			{
			$indexer_ret = RunIndexer ( $error, "--all" );
				if ( $indexer_ret==2 )
				{
					fwrite ( $report, "$error\n" );
				} else if ( $indexer_ret!=0 )
				{
					if ( !HandleFailure ( $config, $report, $error, $nfailed ) )
						$log .= "\tsubtest $subtest: error running indexer with code $indexer_ret; see $report_file\n";
					continue;

				}
			}

			// additional optional runs (eg for merge tests)
			$indexer_ret = $config->RunIndexerEx ( $error );
			if ( $indexer_ret==2 )
			{
				fwrite ( $report, "$error\n" );
			} else if ( $indexer_ret!=0 )
			{
				if ( !HandleFailure ( $config, $report, $error, $nfailed ) )
					$log .= "\tsubtest $subtest: error running indexer with code $indexer_ret; see $report_file\n";
				continue;

			}
		}

		$searchd_error = FALSE;

		if ( $config->NumAgents () == 1 )
		{

            $agents[0]["daemon"] = array ( "config"=>"config.conf", "error"=>"error.txt", "pid"=>$ss_pid_file,
                "requirements"=>$config->GetSearchdRequirements(), "address"=>false, "port"=>false );

			if ( $sd_managed_searchd )
				$searchd_ret = 0;
			else
				$searchd_ret = StartSearchd ( "config.conf", "error.txt", $ss_pid_file, $error, $config->GetSearchdRequirements() );
			
			$stop[scriptdir("config.conf")] = $ss_pid_file;

			if ( $searchd_ret == 1 )
			{
				if ( !HandleFailure ( $config, $report, $error, $nfailed ) )
					$log .= "\tsubtest $subtest: error starting searchd; see $report_file\n";
				
				$searchd_error = TRUE;
			}
			else if ( $searchd_ret==2 )
			{
				fwrite ( $report, "$error\n" );
			}
		}
		else
			for ( $i = $config->NumAgents () - 1; $i >= 0 && !$searchd_error; $i-- )
			{
				static $agent_id = 0;
				$agent_id++;

				$config_file = "config_".$agent_id.".conf";
				$pid_file = "searchd_".$agent_id.".pid";
				$abs_config_file = scriptdir($config_file);
				$abs_pid_file = scriptdir($pid_file);
				$stop[$abs_config_file] = $abs_pid_file;
				$msg = '';
				$error_agent = "error_".$agent_id.".txt";
				$config->SetAgent ( $agents [$i], 0 );
				$config->SetPIDFile ( $abs_pid_file );
				if ( !$config->WriteConfig ( $abs_config_file, $i, $msg ) )
					continue;

				$agents[$i]["daemon"] = array ( "config"=>$config_file, "error"=>$error_agent, "pid"=>$pid_file,
					"requirements"=>$config->GetSearchdRequirements(), "address"=>$config->AddressAPI(), "port"=>$config->Port() );

                if ( $sd_managed_searchd )
                    $searchd_ret = 0;
                else
                    $searchd_ret = StartSearchd ( $config_file, $error_agent, $pid_file, $error, $config->GetSearchdRequirements(), $config->AddressAPI(), $config->Port() );

				if ( $searchd_ret == 1 )
				{
					if ( !HandleFailure ( $config, $report, $error, $nfailed ) )
						$log .= "\tsubtest $subtest: error starting searchd; see $report_file\n";
				
					$searchd_error = TRUE;
		
				}
				else if ( $searchd_ret==2 )
				{
					fwrite ( $report, "$error\n" );
				}

			}

		if ( $searchd_error )
			continue;

		// in case of RT index - run "insert into" instead of indexer
		if ( $config->IsRt () )
			$config->InsertIntoIndexer ( $error );

        $config->ResetResults();

		if ( !$config->IsQueryTest () )
		{
            if (!$config->RunCustomTest($error))
			{
                if (!HandleFailure($config, $report, "$error\n", $nfailed))
                    $log .= "\tsubtest $subtest: query error: $error\n";
                continue;
            }
        }

		$error = "";
		if ( !$config->RunQuery ( $error, $examples ) )
		{
			if ( !HandleFailure ( $config, $report, "$error\n", $nfailed ) )
				$log .= "\tsubtest $subtest: query error: $error\n";
			continue;
		}

		WriteExamples ( $example_file, $examples );

        $showall = $isdemo || IsModelGenMode();

		$allmatch = $showall || $config->CompareToModel();
		if ( !$allmatch )
		{
			$log .= "\tsubtest $subtest: query results mismatch; see $report_file\n";
			$nfailed++;
		}

		if ( $isdemo )
			$log .= "\tdemo/bugreport $subtest done; see $report_file\n";



		$config->WriteResults ( $report, !$showall );

		if ( !$allmatch )
		{
			fwrite ( $report, "SUBTEST FAILED, RESULTS ARE DIFFERENT FROM THE REFERENCE:\n\n" );
			$config->WriteReferenceResultsDiff ( $report );
		}

		$config->SubtestFinished ();
	}
	while ( $config->CreateNextConfig () );

	if ( $isdemo )
	{
		$ss_log				= $oldlog;
		$ss_query_log		= $oldquerylog;
	}

	if ( !$sd_managed_searchd )
		foreach ( $stop as $conf=>$pid )
			StopSearchd ( $conf, $pid );

	$total = $config->SubtestNo()+1;
	$dump_failed = false;
	if ( IsModelGenMode () )
	{
		$variant = $config->CheckVariants ( $report_path."_variant.txt" );
		if ($variant===false)
		{
			$config->WriteModel ( $model_file );
			printf ( "$prefix done; %d/%d subtests run\n", $config->SubtestNo(), $nsubtests );
		} else
		{
			printf ( "$prefix done; %d/%d subtests: VARIANT CHECK FAILED: %s\n", $config->SubtestNo(), $nsubtests, $variant );
			$nfailed = $total;
		}
	}
	else if ( $nfailed==0 )
		printf ( "$prefix done; %d/%d subtests OK\n", $config->SubtestNo(), $nsubtests );
	else {
		printf("$prefix done; %d/%d subtests FAILED:\n%s", $nfailed, $nsubtests, $log);
		$dump_failed = true;
	}

	fclose ( $report );

	if ($dump_failed && $g_locals['ctest'])
	{
		$textreport = file_get_contents ($report_file);
		printf ( "\n--------------Test report:------------\n%s\n--------------Done report.------------\n", $textreport );
	}

	// cleanup DB after ourselves
	if ( !array_key_exists ('no_drop_db', $g_locals) && isset($connection) )
		foreach ( $config->DB_Drop() as $q )
			mysqli_wr ( $q, $connection );

	return array ( "tests_total"=>$total, "tests_failed"=>$nfailed, "tests_skipped"=>0 );
}

