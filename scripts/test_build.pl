#!/usr/bin/perl

use strict;
use warnings;
use Cwd qw(abs_path);
use File::Path qw(rmtree make_path);
use File::Basename;
use File::Spec::Functions qw(catdir catfile);
use Getopt::Long;

my %compilers = (
    'clang' => {
        bin => 'clang',
        flags => [
            '-std=c23', '-O2', '-Wall', '-Wextra', '-Wpedantic', '-Wconversion',
            '-Wsign-conversion', '-Wshadow', '-Wstrict-prototypes', '-Wmissing-prototypes',
            '-Wmissing-declarations', '-Wformat=2', '-Wundef', '-Wcast-qual',
            '-Wwrite-strings', '-Wdouble-promotion', '-Wnull-dereference', '-Wvla', '-Werror'
        ],
        ubsan_flags => [
            '-O1', '-g',
            '-fsanitize=undefined,signed-integer-overflow,implicit-conversion,float-divide-by-zero',
            '-fno-sanitize-recover=all'
        ],
        out_arg => sub { return ('-o', $_[0]); }
    },
    'gcc' => {
        bin => 'gcc',
        flags => [
            '-std=c23', '-O2', '-Wall', '-Wextra', '-Wpedantic', '-Wconversion',
            '-Wsign-conversion', '-Wshadow', '-Wstrict-prototypes', '-Wmissing-prototypes',
            '-Wmissing-declarations', '-Wformat=2', '-Wundef', '-Wcast-qual',
            '-Wwrite-strings', '-Wdouble-promotion', '-Wnull-dereference', '-Wvla', '-Werror'
        ],
        ubsan_flags => [
            '-O1', '-g',
            '-fsanitize=undefined',
            '-fno-sanitize-recover=all'
        ],
        out_arg => sub { return ('-o', $_[0]); }
    },
    'msvc' => {
        bin => 'cl',
        flags => [
            '/std:clatest', '/O2', '/W4', '/WX', '/nologo'
        ],
        ubsan_flags => [
            '/Od', '/Zi', '/RTC1'
        ],
        out_arg => sub { return ('/Fe' . $_[0]); }
    },
    'clang-cl' => {
        bin => 'clang-cl',
        flags => [
            '/std:clatest', '/O2', '/W4', '/WX', '/nologo',
            '-Wpedantic', '-Wconversion', '-Wshadow'
        ],
        ubsan_flags => [
            '/Od', '/Zi',
            '-fsanitize=undefined',
            '-fno-sanitize-recover=all'
        ],
        out_arg => sub { return ('/Fe' . $_[0]); }
    },
);

my @positive_tests = qw(
    test_core
    test_match
    test_random
    test_ulp
    test_ulp_policy
    test_custom
    test_no_short_names
    test_filter
    test_cases_array
);

my @negative_tests = qw(
    test_failure_body
    test_failure_after
    test_match_fallthrough
);

sub find_root {
    return abs_path(catdir(dirname($0), '..'));
}

sub build {
    my ($source_dir, $build_dir, $cases, $config, $use_ubsan) = @_;
    make_path($build_dir) unless -d $build_dir;

    print("Building from: $source_dir -> $build_dir (Compiler: $config->{bin})\n");

    my @compile_cmd = ($config->{bin});
    push @compile_cmd, @{$config->{ubsan_flags}} if $use_ubsan;
    push @compile_cmd, @{$config->{flags}} unless $use_ubsan;

    foreach my $source (@$cases) {
        my $path = catdir($source_dir, $source . ".c");
        die "Error: No such file $path\n" unless -f $path;

        my $out_file = catfile($build_dir, $source);
        my @out_args = $config->{out_arg}->($out_file);

        my @final_cmd = (@compile_cmd, @out_args, $path);
        my $exit_code = system(@final_cmd);

        if ($exit_code != 0) {
            die "Error: Failed to compile $source.c (Exit code: " . ($exit_code >> 8) . ")\n";
        }
    }
}

sub main {
    my $compiler_choice = 'clang';
    my $enable_ubsan    = 0;

    GetOptions(
        "compiler=s" => \$compiler_choice,
        "ubsan"      => \$enable_ubsan,
    ) or die "Error parsing command line arguments.\n";

    $compiler_choice = lc($compiler_choice);
    die "Unsupported compiler: $compiler_choice. Supported are: clang, gcc, msvc, clang-cl\n"
        unless exists $compilers{$compiler_choice};

    my ($src_arg, $positive_arg, $negative_arg) = @ARGV;

    unless (defined $negative_arg) {
        die "Usage: $0 [--compiler=clang|gcc|msvc|clang-cl] [--ubsan] <source_dir> <pos_build_dir> <neg_build_dir>\n";
    }

    my $root_dir = find_root();
    my $src_dir = catdir($root_dir, $src_arg);
    die "Error: No such directory $src_dir\n" unless -d $src_dir;

    my $config = $compilers{$compiler_choice};

    my $positive_build_dir = catdir($root_dir, $positive_arg);
    build($src_dir, $positive_build_dir, \@positive_tests, $config, $enable_ubsan);

    my $negative_build_dir = catdir($root_dir, $negative_arg);
    build($src_dir, $negative_build_dir, \@negative_tests, $config, $enable_ubsan);

    print "Build completed successfully.\n";
}

main();
