printf ( "testing $test_path, spelldump... " );

$windows = isset($_SERVER["WINDIR"]) || isset($_SERVER["windir"]) || isset($_SERVER["HOMEDRIVE"]);
global $locals;

if ( $windows )
	$spelldump_path = "..\\bin\\debug\\spelldump";
else
	$spelldump_path = $locals['srcdir']."/spelldump";

global $g_model;
if ( $g_model )
{
	exec ( "$spelldump_path $test_path/model.dict $test_path/model.aff $test_path/model.spell", $error, $retval );

	if ( !file_exists ( "$test_path/model.spell" ) )
		return false;

	printf ( "done; 1/1 subtests OK\n" );
	return true;
}

exec ( "$spelldump_path $test_path/model.dict $test_path/model.aff $test_path/current.spell", $error, $retval );

if ( !file_exists ( "$test_path/current.spell" ) )
	return false;

$model = file_get_contents ( "$test_path/model.spell" );
$result = file_get_contents ( "$test_path/current.spell" );
$model = str_replace ( $model, "\r\n", "\n" );
$result = str_replace ( $result, "\r\n", "\n" );
if ( $model != $result )
{
	if ( $windows )
		system ( "diff -u3 $test_path/model.spell $test_path/current.spell > $test_path/report.txt" );
	else
		system ( "diff $test_path/model.spell $test_path/current.spell > $test_path/report.txt" );

	printf ( "FAILED\n" );
	return false;
}

unlink ( "$test_path/current.spell" );
printf ( "done; 1/1 subtests OK\n" );

return true;
