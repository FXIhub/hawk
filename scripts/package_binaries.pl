#!/usr/bin/perl -w

use strict;

my $basedir =  dirname(abs_path(__FILE__));
(my $sec,my $min,my $hour,my $mday,my $mon,my $year,my $wday,my $yday,my $isdst) =
                                                               localtime(time);

my $version = sprintf("%02d.%02d.%02d",$year%100,$mon,$mday);
my $arch = `uname -sm|tr " /" "_"`;
my $reldir = $basedir."../releases/hawk-$version-$arch";
`mkdir -p $reldir`

