#
# check that all existing config directives, SphinxQL statements are documented
#

# load sphinx.xml as a single huge string
$sep = $/;
undef $/;
open FP, "<sphinx.xml" or die("failed to open sphinx.xml");
$doc = <FP>;
close FP;
$/ = $sep;

# load and parse sphinx.conf.in to %conf hash
$section = "";
open FP, "<../sphinx.conf.in" or die("failed to open ../sphinx.conf.in");
while (<FP>)
{
	if (/^(source|index|searchd|indexer)\b/)
	{
		$section = $1;
		next;
	}
	if (/^}/)
	{
		$section = "";
		next;
	}
	next if $section eq "";

	if (/^\s*(?:(?:\#\s+)*)(\w+)\s*=/)
	{
		$conf{$section}{$1} = "conf";
	}
}
close FP;

# check config directives from sphinxutils.cpp vs sphinx.xml and sphinx.conf.in
$in_section = 0;
$num_sections = 0;
$num_keys = 0;
$num_missing = 0;
open FP, "<../src/sphinxutils.cpp" or die("failed to open ../src/sphinxutils.cpp");
while (<FP>)
{
	# out of section? scan lines until a section declaration start
	if ($in_section!=1)
	{
		if (/KeyDesc_t\s+g_dKeys(\w+)\s*\[\]/)
		{
			$section = lc $1;
			$in_section = 1;
			$num_sections++;
		}
		next;
	}

	# in a section, handle stuff
	# opening bracket
	next if (/^{$/);

	# closing bracket
	if (/^\s*};\s$/)
	{
		$in_section = 0;
		next;
	}

	# closing entry
	next if (/^\s*{\s*NULL,/);

	# entry
	if (/^\s*\{\s*\"(\w+)\",\s*(\w.*?),/)
	{
		$key = $1;
		$flags = $2;
		next if !$key;
		next if ($flags =~ /KEY_DEPRECATED/);
		next if ($flags =~ /KEY_HIDDEN/);
		$num_keys++;

		$miss = "";
		if ($conf{$section}{"$key"} ne "conf")
		{
			$miss = "sphinx.conf.in";
		}

		# handle doc-level replacements
		if (/check.pl/)
		{
			die("unknown replacement syntax: $_") if (!(/\/\/\s+check.pl\s+(\w+)$/));
			$key = $1;
		}

		$dockey = $key;
		$dockey =~ s/_/-/g;
		if ($doc !~ /<sect2 id="conf-($section-)*$dockey">/)
		{
			$miss .= " and " if ($miss ne "");
			$miss .= "sphinx.xml";
		}

		if ($miss ne "")
		{
			print "section $section, key $key missing from $miss\n";
			$num_missing++;
		}
		next;
	}

	# whoops, unhandled in-section syntax
	die("unhandled in-section syntax: $_");
}
close FP;
die("failed to find key sections in sphinxutils.cpp") if !$num_sections;
print "total $num_keys active config directives, $num_missing not documented\n";

# check SphinxQL statements from searchd.cpp vs sphinx.xml
$in_list = 0;
$num_statements = 0;
$num_missing = 0;

sub check_stmt
{
	$num_statements++;
	my $stmt = shift;
	my $dockey = lc $stmt;
	$dockey =~ s/_/-/g;
	if ($doc !~ /<sect1 id="sphinxql-$dockey">/)
	{
		print "statement $stmt not documented\n";
		$num_missing++;
	}
}

open FP, "<../src/searchd.cpp" or die("failed to open ../src/searchd.cpp");
while (<FP>)
{
	if (!$in_list)
	{
		$in_list = 1 if (/enum SqlStmt_e/);
		next;
	}
	last if /};/; # closing bracket
	next if !/STMT_(\w+)/; # empty line or something
	$stmt = $1;
	next if ($stmt eq "DUMMY") || ($stmt eq "PARSE_ERROR") || ($stmt eq "TOTAL"); # skip internal codes

	# handle doc-level replacements
	if (/check.pl/)
	{
		die("unknown replacement syntax: $_") if (!(/\/\/\s+check.pl\b(.*?)$/));
		@sub = split(/\s+/, $1);
		for $sub (@sub)
		{
			next if !$sub;
			die("unknown replacement syntax: $sub") if (!($sub =~ /^STMT_(\w+)$/));
			check_stmt($1);
		}
	} else
	{
		# no replacements, just check the line
		check_stmt($stmt);
	}
}
close FP;
die("failed to find SphinxQL statements list in searchd.cpp") if !$in_list;
print "total $num_statements SphinxQL statements, $num_missing not documented\n";
