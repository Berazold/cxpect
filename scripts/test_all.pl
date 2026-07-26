#!/usr/bin/perl
use strict;
use warnings;
use Getopt::Long;
use FindBin;
use File::Spec::Functions qw(catfile);

my ($compiler, $ubsan, $fail_fast, $seed, $junit, $html, $help, $silent);
my $src_dir = "src";
my $pos_dir = "build_pos";
my $neg_dir = "build_neg";

GetOptions(
    "compiler=s"  => \$compiler,
    "ubsan"       => \$ubsan,
    "src-dir=s"   => \$src_dir,
    "pos-dir=s"   => \$pos_dir,
    "neg-dir=s"   => \$neg_dir,
    "silent|sl"   => \$silent,
    "fail-fast"   => \$fail_fast,
    "seed=s"      => \$seed,
    "junit=s"     => \$junit,
    "html=s"      => \$html,
    "help|h"      => \$help,
) or die "Error parsing arguments\n";

if ($help) {
    print "Usage: $0 [options] [filter]\n";
    print "Options pass through to build.pl and run.pl appropriately.\n";
    exit(0);
}

my $filter = shift @ARGV || "";

my $perl = $^X;

my @build_cmd = ($perl, catfile($FindBin::Bin, "test_build.pl"));
push @build_cmd, "--compiler=$compiler" if $compiler;
push @build_cmd, "--ubsan" if $ubsan;
push @build_cmd, $src_dir, $pos_dir, $neg_dir;

print "\e[1;35m>>> RUNNING BUILD STEP\e[0m\n";
my $build_status = system(@build_cmd);

if ($build_status != 0) {
    die "\e[31mFatal: Build failed. Tests will not run.\e[0m\n";
}

print "\n";

my @run_cmd = ($perl, catfile($FindBin::Bin, "test_run.pl"));
push @run_cmd, "--pos-dir=$pos_dir", "--neg-dir=$neg_dir";
push @run_cmd, "-sl" if $silent;
push @run_cmd, "--fail-fast" if $fail_fast;
push @run_cmd, "--seed=$seed" if $seed;
push @run_cmd, "--junit=$junit" if $junit;
push @run_cmd, "--html=$html" if $html;
push @run_cmd, $filter if $filter;

print "\e[1;35m>>> RUNNING TEST STEP\e[0m\n";
my $run_status = system(@run_cmd);

exit($run_status >> 8);
