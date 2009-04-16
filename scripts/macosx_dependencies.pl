#!/usr/bin/perl -w

# This script assumes no intervention is necessary on cmake


use strict;
use Cwd 'abs_path';
use File::Basename;


sub change_install_name{
  my $bindir = shift;
  my $dir = `pwd`;
  chdir($bindir);  
  my $f = `ls -1`;
  my @files = split("\n",$f);
  chdir($dir);
  foreach my $file(@files){
    unless(-f $file){
      next;
    }
    my $libs = `otool -L $file`;
    my @lines = split("\n",$libs);
    foreach my $line (@lines){
      my $old;
      if($line =~ /\s*(.*\.dylib)/){
	$old = $1;
      }
      if($line =~ /\s*(Qt.*?)\s/){
	$old = $1;
      }
      my $dylib;
      if($line =~ /([^\/]*\.dylib)/){
	$dylib = $1;
      }
      if($line =~ /(Qt[^\/ ]*)\s/){
	$dylib = $1;
      }

      if(defined $dylib && -f "../lib/$dylib" || -f "../../lib/$dylib"){
	system("install_name_tool -change $old \@executable_path/../lib/$dylib $file");
	if($dylib =~ /$file/){
	  system("install_name_tool -id \@executable_path/../lib/$dylib $file");
	}
      }
    }
  }
}


my $reldir = $ARGV[0];

my $libdir = $reldir."/lib/";
my $bindir = $reldir."/bin/";


if(`uname -s` =~ /Darwin/){
  # Copy some plugins
  mkdir($libdir."/imageformats");
  system("cp /Developer/Applications/Qt/plugins/imageformats/* ".$libdir."/imageformats");
  system("rm ".$libdir."/imageformats/libqsvg*");

  change_install_name($bindir);
  change_install_name($libdir);
  change_install_name($libdir."/imageformats");
}
