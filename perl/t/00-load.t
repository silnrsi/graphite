#!perl -T

use Test::More tests => 1;

BEGIN {
    use_ok( 'Text::Graphite2' );
}

diag( "Testing Text::Graphite2 $Text::Graphite2::VERSION, Perl $], $^X" );
