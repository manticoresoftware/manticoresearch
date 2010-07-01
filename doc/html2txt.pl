#
# $Id$
#

use Text::Wrap;
use Data::Dumper;
$Text::Wrap::columns = 76;
$Text::Wrap::unexpand = 0;

$MAGIC_NBSP = "\xff";

sub unentity
{
	my $html = shift;
	die ( "INTERNAL ERROR: magic nbsp code found in html" ) if ( $html =~ /$MAGIC_NBSP/ );
	$html =~ s/&nbsp;/$MAGIC_NBSP/g;
	$html =~ s/&lt;/</g;
	$html =~ s/&gt;/>/g;
	$html =~ s/&#8220;/<</g;
	$html =~ s/&#8221;/>>/g;
	$html =~ s/&copy;/(c)/g;
	$html =~ s/&amp;/&/g;
	return $html;
};

sub linify
{
	my $text = shift;
	$text =~ s/\n/ /g;
	$text =~ s/\t/ /g;
	$text =~ s/\s{2,}/ /g;
	$text =~ s/^\s+//ms;
	$text =~ s/\s+$//ms;
	$text =~ s/\s+(\w)\s+/ \1$MAGIC_NBSP/g; # glue 1-letter words with nbsp's
	return $text;
};

sub in
{
	my ($a,$v) = @_;
	foreach my $e ( @$a )
	{
		return 1 if $e eq $v;
	}
	return 0;
};

###############
## entry point
###############

undef $/;
$html = <>;

$html =~ s/.*?<body.*?>//ms;
$html =~ s/<\/body.*/\n--eof--\n/ms;

$res = "";
$acc = "";
$hdr = "";

$left = 0;
$red = "";
@lists = "";
$li = 0;
$pre = 0;

while ( $html =~ s/^(.*?)<([\/]*\w+)(\s+.*?)*>//ms )
{
	$text = $1;
	$tag = lc $2;
	$attrs = lc $3;

	$acc .= $text;

	next if ( in ( [ "a","/a","i","/i","p","code","/code","b","/b","span","/span",
		"strong","/strong","br","em","/em" ], $tag ) );

	if ( in ( [ "div","/div","/h1","/h2","/h3","/h4","/h5","/p","hr","h5",
		"dl","ol","ul","dt","dd","/dl","/ol","/ul","/dt","/dd",
		"li","/li","pre","/pre" ], $tag ) )
	{
# fixme! handle userinput instead of literallayout
		if ( !$pre )
		{
			$acc = linify(unentity($acc));
			$acc = wrap ( (" " x $left).$red, " "x($left+length($red)), $acc );
			die if ( $acc =~ /\t/ );
		} else
		{
			my @lines = split ( /\n/, unentity($acc) );
			$acc = "";
			foreach $line ( @lines )
			{
				next if ( !$acc && !$line );
				$acc .= ( " " x $left ) . "| $line\n";
			}
			$acc = "\n$acc\n";
		}
		$red = " "x length($red);

		if ( $acc )
		{
			if ( length($hdr) )
			{
				$res .= "$acc\n" . $hdr x length($acc) . "\n\n";
				$hdr = "";
			} else
			{
				$res .= "$acc\n";
			}
		}
		$acc = "";

		$res .= "\n" if ( in ( [ "div","/div","/h1","/h2","/h3","/p","hr" ], $tag ) );
		$res .= "\n" if ( $#lists<=2 && in ( ["/dl","/ol","/ul" ], $tag ) );

		if ( $tag eq "hr" )
		{
			$res .= "-" x $Text::Wrap::columns . "\n\n";
		}

		if ( in ( [ "dd","ol","ul" ], $tag ) )
		{
			$left += 3;
			$li = 1 if ( $tag eq "ol" ); # fixme! allow nested ol
			push ( @lists, $tag );
			next;
		}

		if ( in ( [ "/dd","/ol","/ul" ], $tag ) )
		{
			$left -= 3;
			pop @lists;
			$red = ""; # just in case
			$res .= "\n";
			next;
		}

		if ( $tag eq "li" )
		{
			my $list = $lists[$#lists];

			if ( $list eq "ol" ) { $red = "$li. "; $li++; }
			elsif ( $list eq "ul" ) { $red = "* "; }
			else { print Dumper(@lists);
			die ( "INTERNAL ERROR: 'li' in unknown list '$list'" ); }
			next;
		}

		if ( $tag eq "pre" || ( $tag eq "div" && $attrs =~ /literallayout/ ) )
		{
			$pre = 1;
			$left += 3;
			next;
		}

		if ( $pre && ( $tag eq "/pre" || $tag eq "/div" ) )
		{
			$pre = 0;
			$left -= 3;
			next;
		}

		next;
	}

	if ( $tag eq "h1" ) { $hdr = "="; next; }
	if ( $tag eq "h2" ) { $hdr = "="; next; }
	if ( $tag eq "h3" ) { $hdr = "-"; next; }
	if ( $tag eq "h4" ) { $hdr = ""; next; }

	die ( "unknown tag='$tag' attrs='$attrs'\n" );
}

$res .= "\n\n--eof--\n";
$res =~ s/^\n+//;
$res =~ s/\n{3,}/\n\n/gms;
$res =~ s/$MAGIC_NBSP/ /g;
print $res;

#
# $Id$
#
