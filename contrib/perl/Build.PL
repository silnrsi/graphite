use strict;
use warnings;
use Module::Build;

my $builder = Module::Build->new(
    module_name         => 'Text-Graphite2',
    license             => 'Perl',
    dist_author         => 'Simon Cozens <simon@cpan.org>',
    dist_version_from   => 'lib/Text/Graphite2.pm',
    extra_compiler_flags => '-Wall -Wno-unused-variable',
    extra_linker_flags  => '-lgraphite2',
    include_dirs        => ['.'],
    requires => {
        'Test::More' => 0,
    },
    add_to_cleanup      => [ 'Graphite2-*' ],
);

$builder->create_build_script();
