#!perl -T
# SPDX-License-Identifier: Artistic-1.0-Perl
# Copyright (C) 2011 Simon Cozens

use Test::More tests => 1;

BEGIN {
    use_ok( 'Text::Gr2' );
}

diag( "Testing Text::Gr2 $Text::Gr2::VERSION, Perl $], $^X" );
