use strict;
use Sphinx;

my $spx = Sphinx->new;
$spx->SetServer('localhost' , 3312);

my @docs = (
        "this is my test text to be highlighted",
        "this is another test text to be highlighted",
);
my $words = "test text";
my $index = "test1";
my %opts = (
        "before_match"          => "<b>",
        "after_match"           => "</b>",
        "chunk_separator"       => " ... ",
        "limit"                         => 400,
        "around"                        => 15
);
my $res = $spx->BuildExcerpts(\@docs,$index,$words,\%opts );
if ( !$res ) {
        die ( "ERROR: " . $spx->GetLastError() . ".\n" );
} else {
        my $n;
        foreach my $entry (@$res) {
                $n++;
                print "n=$n, res=$entry\n";
        }
}
