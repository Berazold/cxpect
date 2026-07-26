#!/usr/bin/perl
use strict;
use warnings;
use Getopt::Long;
use POSIX qw(strftime);
use Time::HiRes qw(time);

my $C_RST   = "\e[0m";
my $C_CYAN  = "\e[36m";
my $C_BOLD  = "\e[1m";
my $C_GREEN = "\e[32m";
my $C_RED   = "\e[31m";
my $C_YELLOW = "\e[33m";

my ($silent, $fail_fast, $seed, $help, $junit_file, $html_file);
my $pos_dir = "build_pos";
my $neg_dir = "build_neg";

GetOptions(
    "silent|sl"   => \$silent,
    "fail-fast"   => \$fail_fast,
    "seed=s"      => \$seed,
    "pos-dir=s"   => \$pos_dir,
    "neg-dir=s"   => \$neg_dir,
    "junit=s"     => \$junit_file,
    "html=s"      => \$html_file,
    "help|h"      => \$help,
) or die "Bad options\n";

if ($help) {
    print "Usage: $0 [options] [filter]\n";
    print "Options: -sl (silent), --fail-fast, --seed=<val>, --pos-dir=<path>, --neg-dir=<path>\n";
    print "Reports: --junit=<file.xml>, --html=<file.html>\n";
    exit(0);
}

my $filter = shift @ARGV || "";

die "${C_RED}${C_BOLD}Error:${C_RST} Positive directory '$pos_dir' does not exist\n" unless -d $pos_dir;
die "${C_RED}${C_BOLD}Error:${C_RST} Negative directory '$neg_dir' does not exist\n" unless -d $neg_dir;

$ENV{VUK_TEST_SEED} = $seed if $seed;

my @tests;
push @tests, map { { path => $_, type => "POS" } } sort grep { -f $_ && -x $_ && /$filter/i } glob("$pos_dir/*");
push @tests, map { { path => $_, type => "NEG" } } sort grep { -f $_ && -x $_ && /$filter/i } glob("$neg_dir/*");

die "\e[31mNo executables found in $pos_dir or $neg_dir\e[0m\n" unless @tests;

my @results;
my ($passed, $failed, $total_time) = (0, 0, 0);

foreach my $t (@tests) {
    my ($name) = $t->{path} =~ m{([^/\\]+)$};
    print "[RUN $t->{type}] $name\n";

    my $start = time();
    my $output = "";

    if ($silent) {
        system($t->{path});
    } else {
        $output = `$t->{path} 2>&1`;
    }

    my $status = $?;
    my $duration = time() - $start;
    $total_time += $duration;

    my $is_ok = ($t->{type} eq "POS") ? ($status == 0) : ($status != 0);

    push @results, { name => "[$t->{type}] $name", time => $duration, output => $output, status => $status, passed => $is_ok };

    if ($is_ok) {
        printf "${C_GREEN}${C_BOLD}[OK]${C_RST} %s (${C_GREEN}%.3fs${C_RST})\n\n", $name, $duration;
        $passed++;
    } else {
        my $exit_info = ($status & 127) ? "signal " . ($status & 127) : "exit " . ($status >> 8);
        my $reason = ($t->{type} eq "POS") ? $exit_info : "expected failure, but exited 0";

        print "${C_RED}${C_BOLD}[FAIL]${C_RST} $name ($reason)\n";

        if (!$silent && $output) {
            print "${C_YELLOW}────── Output ──────${C_RST}\n";
            print $output;
            print "\n" unless $output =~ /\n$/;
            print "${C_YELLOW}────────────────────${C_RST}\n\n";
        }
        $failed++;
        last if $fail_fast;
    }
}

my $line = "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━";
print "\n${C_CYAN}${line}${C_RST}\n";
print " ${C_BOLD}TEST SUMMARY${C_RST}\n";
print "${C_CYAN}${line}${C_RST}\n";

my $total = $passed + $failed;
printf " Total:   ${C_BOLD}%-5d${C_RST}\n", $total;
printf " Passed:  ${C_GREEN}%-5d${C_RST} %s\n", $passed, ($passed > 0 ? "✓" : "");
printf " Failed:  ${C_RED}%-5d${C_RST} %s\n", $failed, ($failed > 0 ? "✗" : "");
printf " Time:    %.3fs\n", $total_time;

print "${C_CYAN}${line}${C_RST}\n\n";

if ($junit_file) {
    write_junit_xml($junit_file, \@results, $total_time, $passed, $failed);
    print "JUnit report written to: $junit_file\n";
}

if ($html_file) {
    write_html_report($html_file, \@results, $total_time, $passed, $failed);
    print "HTML report written to: $html_file\n";
}

print "\n" if ($junit_file || $html_file);
exit($failed > 0 ? 1 : 0);

sub write_junit_xml {
    my ($file, $res, $time, $p, $f) = @_;
    open(my $fh, '>', $file) or die "Cannot write JUnit file: $!";
    my $ts = strftime("%Y-%m-%dT%H:%M:%S", localtime);

    print $fh qq{<?xml version="1.0" encoding="UTF-8"?>\n};
    print $fh qq{<testsuite name="VukEngine" tests="@{[$p+$f]}" failures="$f" time="$time" timestamp="$ts">\n};

    for my $r (@$res) {
        my ($name, $out) = (clean_text($r->{name}), clean_text($r->{output}));
        print $fh qq{  <testcase name="$name" classname="unit" time="$r->{time}">\n};

        if (!$r->{passed}) {
            my $msg = ($r->{status} & 127) ? "Killed by signal " . ($r->{status} & 127) : "Exited with code " . ($r->{status} >> 8);
            $msg = "Unexpected success (exited with code 0)" if $r->{status} == 0;
            print $fh qq{    <failure message="$msg">\n$out    </failure>\n};
        } elsif ($out) {
            print $fh qq{    <system-out>\n$out    </system-out>\n};
        }
        print $fh qq{  </testcase>\n};
    }
    print $fh qq{</testsuite>\n};
    close($fh);
}

sub write_html_report {
    my ($file, $res, $time, $p, $f) = @_;
    open(my $fh, '>', $file) or die "Cannot write HTML file: $!";

    my $suite_icon = $f == 0 ? '✅' : '❌';
    my $suite_color = $f == 0 ? 'green' : 'red';
    my $total = $p + $f;

    print $fh <<"HTML";
<details id="suite.0">
        <summary>
          <span style="color: $suite_color">$suite_icon</span>
          <span class="testsuite-name" title="VukEngine">VukEngine</span>
          Tests: <b>$total</b>,
          Failures: <b>$f</b>,
          <em>Time: $time</em>
        </summary>
        <div>
HTML

    my $i = 0;
    for my $r (@$res) {
        my ($name, $out) = (clean_text($r->{name}), clean_text($r->{output}));
        my $case_icon = $r->{passed} ? '✅' : '❌';
        my $color = $r->{passed} ? 'green' : 'red';
        my $status_title = $r->{passed} ? 'passed' : 'failed';

        print $fh <<"HTML";
        <details style="margin-left: 1em" id="case.0.$i">
        <summary>
          <span title="$status_title" style="color: $color">$case_icon</span>
          <span class="testcase-name" title="$name unit">
            $name
            unit
          </span>
          <em>$r->{time}</em>
        </summary>
        <div style="margin-left: 1em">
HTML

        if (!$r->{passed}) {
            my $msg = ($r->{status} & 127) ? "Killed by signal " . ($r->{status} & 127) : "Exited with code " . ($r->{status} >> 8);
            $msg = "Unexpected success (exited with code 0)" if $r->{status} == 0;
            print $fh "          <div><b>Failure:</b> $msg</div>\n";
        }

        if ($out) {
            print $fh <<"HTML";
          <div>
              <div><b>System-Out:</b></div>
              <div><pre>$out</pre></div>
          </div>
HTML
        }

        print $fh <<"HTML";
        </div>
      </details>
HTML
        $i++;
    }

    print $fh <<"HTML";
        </div>
</details>
HTML
    close($fh);
}

sub clean_text {
    my $str = shift // return "";

    $str =~ s/\x1B\[[0-9;]*[a-zA-Z]//g;

    $str =~ s/&/&amp;/g;
    $str =~ s/</&lt;/g;

    $str =~ s/>/&gt;/g;
    $str =~ s/"/&quot;/g;
    return $str;
}
