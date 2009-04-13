#!/usr/bin/perl -w

# This script assumes no intervention is necessary on cmake

use strict;
use Cwd 'abs_path';
use File::Basename;

my $basedir =  dirname(abs_path(__FILE__));
(my $sec,my $min,my $hour,my $mday,my $mon,my $year,my $wday,my $yday,my $isdst) =
                                                               localtime(time);

my $version = sprintf("%02d.%02d.%02d",$year%100,$mon,$mday);
my $arch = `uname -sm|tr " /" "_"`;
chomp($arch);
my $reldir =($basedir."/../releases/hawk-$version-$arch");
my $builddir = ($basedir."/../releases/hawk-build-$arch");
`mkdir -p $reldir`;
`mkdir -p $builddir`;
$reldir = abs_path($reldir);
$builddir = abs_path($builddir);
print $builddir;
chdir($builddir) or die($!);
print `pwd`;
system("cmake ../.. -DCMAKE_INSTALL_PREFIX:PATH=$reldir");
system("make -j 4");
system("make install");
chdir($reldir);
mkdir("lib");
chdir("lib");
