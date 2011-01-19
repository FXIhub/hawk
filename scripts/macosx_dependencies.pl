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
      print "line - $line\n";
      my $old;
#      $line =~ s/\@rpath/\/usr\/local\/cuda\/lib/;
      my $dylib;

      if($line =~ /\s*(.*\.dylib)/){
	$old = $1;
      }
# No need for Qt dependencies anymore
#      if($line =~ /\s*(Qt.*?)\s/){
#	$old = $1;
#      }
      if($line =~ /([^\s\/]*\.dylib)/){
	$dylib = $1;
      }
#      if($line =~ /(Qt.*?)\s/){
#	$dylib = $1;
#      }
      print "$file\n";
      print "old - $old dylib - $dylib\n";
      if(defined $dylib && -f "../lib/$dylib" || -f "../../lib/$dylib"){
	system("install_name_tool -change $old \@executable_path/../lib/$dylib $file");
	if($dylib =~ /$file/){
	  system("install_name_tool -id \@executable_path/../lib/$dylib $file");
	}
      }else{
	print "Condition not met\n";
      }
    }
  }
}

sub bundle_HawkGUI{
  print `pwd`;
  system("rm -rf HawkGUI.app");
  mkdir("HawkGUI.app");
  mkdir("HawkGUI.app/Contents");
  mkdir("HawkGUI.app/Contents/Resources");
  mkdir("HawkGUI.app/Contents/MacOS");
  system("mv HawkGUI HawkGUI.app/Contents/MacOS");
  # Copy icon
  system("cp ../../../src/HawkGUI/images/Hawk.icns HawkGUI.app/Contents/Resources");
  system("macdeployqt HawkGUI.app");
}

my $reldir = $ARGV[0];

my $libdir = $reldir."/lib/";
my $bindir = $reldir."/bin/";


if(`uname -s` =~ /Darwin/){
  chdir($bindir);
  bundle_HawkGUI();
  chdir("..");
  change_install_name($bindir);
  change_install_name($libdir);
  change_install_name($libdir."/imageformats");
}
