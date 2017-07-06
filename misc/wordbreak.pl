#
# prepare titles for wordbreaker frequency dictionary builder
# extract and cleanup data
#

#
# usage example:
#
# perl wordbreak.pl < raw.xml > titles.xml
# indexer ub --buildstops titles-freq.txt 10000000 --buildfreqs
#
# sphinx.conf:
#
# source ub
# {
#     type = xmlpipe2
#     xmlpipe_field = title
#     xmlpipe_fixup_utf8 = 1
#     xmlpipe_command = cat titles.xml
# }
#
# index ub
# {
#     dict = keywords
#     type = plain
#     source = ub
#     path = ub
#     charset_type = utf-8
#     html_strip = 0
#     charset_table = A..Z->a..z, a..z
# }
#

$n = 1;
print "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
print "<sphinx:docset>\n";
while (<>)
{
	# extract title
	next if (!/^\s*<title>/);
	chomp;

	# cleanup ABC's as in World's
	s/[a-z]\'s\b//ig;

	# cleanup A.B.C. as in S.r.l. and other abbreviations
	s/\b([a-z]\.){2,}\b//ig;

	# cleanup A&B as in H&M
	s/\b[a-z]\&[a-z]\b//ig;

	# cleanup ABC.com as in google.com, brisbanetimes.com.au, etc
	s/\b\w+(\.(com|org|net))*\.(com|org|net|it|de|pl|co\.uk|nl|edu|eu|info|fr|ch|br|ru|at|ca|si|tv|es|gov|br|au|jp|biz|dk|il|se|cz|no)\b//ig;

	# print out cleaned up document
	print "<sphinx:document id=\"$n\">";
	print;
	print "</sphinx:document>\n";
	$n++;
}
print "</sphinx:docset>\n";
