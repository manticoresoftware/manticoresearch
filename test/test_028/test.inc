printf ( "testing $test_path, spelldump... " );

$windows = isset($_SERVER["WINDIR"]) || isset($_SERVER["windir"]) || isset($_SERVER["HOMEDRIVE"]);
global $locals;

$spelldump_path = $locals['spelldump'];
$res_path = $locals['testdir'].$test_path;

global $g_model;
if ( $g_model )
{
	exec ( "$spelldump_path $test_path/model.dict $test_path/model.aff $test_path/model.spell", $error, $retval );

	if ( !file_exists ( "$test_path/model.spell" ) )
		return false;

	printf ( "done; 1/1 subtests OK\n" );
	return true;
}

exec ( "$spelldump_path $test_path/model.dict $test_path/model.aff $res_path/current.spell", $error, $retval );

if ( !file_exists ( "$res_path/current.spell" ) )
	return false;

$model = file_get_contents ( "$test_path/model.spell" );
$result = file_get_contents ( "$res_path/current.spell" );
$model = str_replace ( "\r\n", "\n", $model );
$result = str_replace ( "\r\n", "\n", $result );
if ( $model != $result )
{
	if ( $windows )
		system ( "diff -u3 $test_path/model.spell $res_path/current.spell > $res_path/report.txt" );
	else
		system ( "diff $test_path/model.spell $res_path/current.spell > $res_path/report.txt" );

	printf ( "FAILED\n" );
	return false;
}

unlink ( "$res_path/current.spell" );
printf ( "done; 1/1 subtests OK\n" );

return true;
