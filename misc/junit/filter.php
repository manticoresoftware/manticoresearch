<?php

/*
	Process output of ctest dashboard - decompress all compressed outputs
    (that is necessary to prepare then file for junit)
*/

$args = $_SERVER["argv"];
$ifile=$args[1];
if ( $ifile=="" )
	exit (1);

$doc = new DOMDocument ( "1.0");
if ( !$doc->load ( $ifile ) )
	exit (1);

$wr = array (
//"<", ">", "\r", "&",
	"\x0","\x1","\x2","\x3","\x4","\x5","\x6","\x7","\x8","\xb",
	"\xc","\xe","\xf","\x10","\x11","\x12","\x13","\x14","\x15",
	"\x16","\x17","\x18","\x19","\x1a","\x1b","\x1c","\x1d","\x1e","\x1f");

$nw = array (
//"&lt;", "&gt;", "", "&amp;",
	'[NON-XML-CHAR-0x0]','[NON-XML-CHAR-0x1]','[NON-XML-CHAR-0x2]','[NON-XML-CHAR-0x3]',
	'[NON-XML-CHAR-0x4]','[NON-XML-CHAR-0x5]','[NON-XML-CHAR-0x6]','[NON-XML-CHAR-0x7]',
	'[NON-XML-CHAR-0x8]','[NON-XML-CHAR-0xb]','[NON-XML-CHAR-0xc]','[NON-XML-CHAR-0xe]',
	'[NON-XML-CHAR-0xf]','[NON-XML-CHAR-0x10]','[NON-XML-CHAR-0x11]','[NON-XML-CHAR-0x12]',
	'[NON-XML-CHAR-0x13]','[NON-XML-CHAR-0x14]','[NON-XML-CHAR-0x15]','[NON-XML-CHAR-0x16]',
	'[NON-XML-CHAR-0x17]','[NON-XML-CHAR-0x18]','[NON-XML-CHAR-0x19]','[NON-XML-CHAR-0x1A]',
	'[NON-XML-CHAR-0x1B]','[NON-XML-CHAR-0x1C]','[NON-XML-CHAR-0x1D]','[NON-XML-CHAR-0x1E]',
	'[NON-XML-CHAR-0x1F]');

$values = $doc->getElementsByTagName('Value');
foreach ($values as $value)
{
	if ($value->hasAttributes() && $value->getAttribute("encoding")=="base64" && $value->getAttribute("compression")=="gzip")
	{
		$foo = base64_decode ($value->textContent);
		$value->removeAttribute("encoding");
		$foo = gzuncompress ( $foo );
		$value->removeAttribute("compression");
		$value->textContent = str_replace ($wr,$nw,$foo);
	}
}

print $doc->saveXML();
?>
