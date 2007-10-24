use strict;
use Sphinx;

my $spx = Sphinx->new;

# SetServer(host, port)
$spx->SetServer('localhost' , 3312);

# SetLimits(offset, limit, [max])
# Limit to 10 results, with offset=20
# $spx->SetLimits(20,10);

# SetMatchMode(mode)
# mode can be SPH_MATCH_ALL, SPH_MATCH_ANY, SPH_MATCH_PHRASE, or SPH_MATCH_BOOLEAN. 
# $spx->SetMatchMode(SPH_MATCH_ANY);

# SetSortMode(mode, [sortby])
# sort can be SPH_SORT_RELEVANCE, SPH_SORT_ATTR_DESC, SPH_SORT_ATTR_ASC or SPH_SORT_TIME_SEGMENTS
# sortby is the attribute to sort by.
# $spx->SetSortMode(SPH_SORT_ATTR_DESC, "date_added");

# SetWeights(values)
# $spx->SetWeights([4,1,3]);

# SetIDRange(min_id, max_id)
# $spx->SetIDRange(0,1000);

# SetFilter(attr, values)
# $spx->SetFilter('group_id', [1961,4228]);

# SetFilterRange(attr, min, max)
# $spx->SetFilterRange('group_id', 10, 1000);

# SetGroupBy(attr, func)
# func can be SPH_GROUPBY_DAY SPH_GROUPBY_WEEK SPH_GROUPBY_MONTH SPH_GROUPBY_YEAR SPH_GROUPBY_ATTR
# $spx->SetGroupBy('group_id', SPH_GROUPBY_ATTR);

my $results = $spx->Query('test', 'lj1');
print "Total documents found: " . $results->{total_found} . "\n";
print "Search took: " . $results->{time} . "s\n";

foreach my $match (@{$results->{matches}}) {
	# Print doc id and weight
        print "Doc: $match->{doc}\tWeight: $match->{weight}";

	# Print attributes for this result
	foreach my $attr (keys %{$results->{attrs}}) {
		print "\t" . $attr . ": " . $match->{$attr};
	}
	print "\n";
}
