package Text::Graphite2;
use Text::Graphite2::Face;
use Text::Graphite2::Font;
use Text::Graphite2::FeatureRef;
use Text::Graphite2::FeatureVal;
use Text::Graphite2::CharInfo;
use Text::Graphite2::Segment;
use Text::Graphite2::Slot;

use warnings;
use strict;

=head1 NAME

Text::Graphite2 - Interface to SIL's Graphite2 rendering engine

=head1 VERSION

Version 0.01

=cut

our $VERSION = '0.01';

require XSLoader;
XSLoader::load('Text::Graphite2', $VERSION);

sub _decode { my $c = pack("N*",$_[0]); $c=~/^[[:print:]]+/ ? $c : $_[0] }

=head1 SYNOPSIS

    use Text::Graphite2;
    my $face = Text::Graphite2::Face->open($fontfile, cache_size => 1000);
    my $sized_font = $face->make_font($dpi*$pointsize/72);
    my $seg = $sized_font->segment($face, $text);
    my @chars = $seg->cinfos;
    for ($seg->slots) {
        layout_text($_->origin_X, $_->origin_Y, $chars[$c++]->unicode_char);
    }

=head1 WHAT IS GRAPHITE?

Graphite is a I<back-end> rendering engine for complex, non-Roman
scripts. It will not lay out text for you. It I<will> help you to know
where characters in a string should be positioned - how wide accents
should be and where they go, what alternate character forms to use, and
so on.

B<This requires Graphite-enabled fonts>. Most fonts aren't; right now,
only a few fonts from the Graphite project itself are. See
http://graphite.sil.org for more details.

=head1 FUNCTIONS

For most purposes, the entry point to this API is through
C<< Text::Graphite2::Face->open >>. The C<Text::Graphite2> module itself
only provides two functions, for logging:

=head2 start_logging

    Text::Graphite2->start_logging($fh, $mask);

Writes an XML log to the filehandle given. Mask is a bitmask made up of
the following flags:

    GRLOG_NONE = 0x0,
    GRLOG_FACE = 0x01,
    GRLOG_SEGMENT = 0x02,
    GRLOG_PASS = 0x04,
    GRLOG_CACHE = 0x08,
    GRLOG_OPCODE = 0x80,
    GRLOG_ALL = 0xFF

=head2 stop_logging

    END { Text::Graphite2->stop_logging }

Finishes writing the log.

=head1 INTERNAL METHODS

=head2 str_to_tag

=head2 tag_to_str

Convert between language/feature tags and ID numbers. This is
effectively done with a C<pack("N*")>.

=head1 SEE ALSO

L<Text::Graphite2::Face>, L<Text::Graphite2::FeatureRef>,
L<Text::Graphite2::FeatureVal>, L<Text::Graphite2::Font>,
L<Text::Graphite2::Segment>

http://graphite.sil.org/

=cut

1; # End of Text::Graphite2
