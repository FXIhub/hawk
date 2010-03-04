#!/usr/bin/perl -w

# This script assumes no intervention is necessary on cmake


use strict;
use File::Basename;
use File::Spec;    

my $basedir =  dirname( File::Spec->rel2abs(__FILE__));
(my $sec,my $min,my $hour,my $mday,my $mon,my $year,my $wday,my $yday,my $isdst) =
                                                               localtime(time);
my $version = sprintf("%02d.%02d.%02d",$year%100,$mon+1,$mday);
my $arch = `gcc -dumpmachine`;
chomp($arch);
my $reldir =($basedir."/../releases/hawk-$version-src");
`rm -rf $reldir`;
$reldir = File::Spec->rel2abs($reldir);
chdir($basedir."/../releases");
#system("svn export svn://hirst/hawk/trunk $reldir");
system("cd $basedir/.. && git archive --format=tar --prefix=hawk-$version-src/ master  |gzip  > $basedir/releases/hawk-$version-src.tar.gz");
#system("tar -zcvf ".$reldir.".tar.gz hawk-$version-src");
