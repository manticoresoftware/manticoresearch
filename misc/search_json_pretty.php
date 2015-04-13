<?php

$l = '';
while($f = fgets(STDIN))
{
    $l = $l . $f;
}

//print ( $l );
$jin = json_decode ( $l, true );
if ( $jin===null && json_last_error()!==JSON_ERROR_NONE )
{
	$err = json_last_error_msg();
	print ("json\n$l");
	exit ( "\ndecode error: '$err'" );
}

$attrs = $jin['attrs'];
$matches = $jin['matches'];
$meta = $jin['meta'];

$res_match = array();
foreach ( $matches as $row  )
{
	$named_val = array();
	for( $i = 0; $i<count($attrs); $i++ )
	{
		$name = $attrs[$i];
		$val = $row[$i];
		$named_val[$name] = $val;
	}
	$res_match[] = $named_val;
}

function print_json_row ( $j, $name )
{
	if ( !count($j) )
	{
		print ( "$name:[]\n" );
		return;
	}
	print ( "$name:\n[" );

	$row_sep = "";
	foreach ( $j as $row )
	{
		print ( "$row_sep\t" );
		$val_sep = "";
		print ( json_encode ( $row ) );
		$row_sep = ",\n";
	}
	print ( "\n]" );
}

print ( "{\n" );
print_json_row ( $res_match, "\"matches\"" );

print ( ",\n\"meta\":\n" );
print ( json_encode ( $meta, JSON_PRETTY_PRINT ) );
print ( "\n" );
if ( isset ( $jin['warning'] ) )
	print ( "\"warning\": " . $jin['warning'] );
if ( isset ( $jin['error'] ) )
	print ( "\"error\": " . $jin['error'] );

print ( "}\n" );	

?>