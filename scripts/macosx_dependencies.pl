#!/usr/bin/perl -w

# This script assumes no intervention is necessary on cmake


use strict;
use Cwd 'abs_path';
use File::Basename;

sub get_dependencies{
  my $bin = shift;
  unless(-f $bin){
    return;
  }
  my $use_ldd = 0;
  if(`which ldd`){
    $use_ldd = 1;
  }
  my $libs;
  if($use_ldd ){
    $libs = `ldd $bin`;
  }else{
    $libs = `otool -L $bin`;
  }
  my @lines = split("\n",$libs);
  my @deps;
  foreach my $line (@lines){
    my $lib;
    if($use_ldd){
      $line =~ /.*? => (.*?) \(0x[0-9abcdef]+\)/;
      $lib = $1;
    }else{
      $line =~ /\s*(.*dylib)/;
      $lib = $1;
    }
    if(defined $lib && $lib ne ""){
      push(@deps, $lib);
    }
  }
  return @deps;
}

sub get_dependencies_recursively{
  my %deps;
  my $bin = shift;
  my @out = get_dependencies($bin);
  while(scalar(@out)){
    my $dep = pop(@out);
    unless($deps{$dep}){
      $deps{$dep} = 1;
      push(@out,get_dependencies($dep));
    }
  }
  return keys(%deps);
}

sub get_all_dependencies{
  my $bindir = shift;
  my $dir = `pwd`;
  chdir($bindir);
  my $f = `ls -1`;
  my @files = split("\n",$f);
  my %deps;
#  print @files;
  foreach my $file(@files){
    my @deps =  get_dependencies_recursively($file);
    foreach my $dep(@deps){
      $deps{$dep} = 1;
    }
  }
  foreach my $key(keys %deps){
#    print $key."\n";
  }
  chdir($dir);
  return keys %deps;
}

sub change_install_name{
  my $bindir = shift;
  my $dir = `pwd`;
  chdir($bindir);  
  my $f = `ls -1`;
  my @files = split("\n",$f);
  chdir($dir);
  foreach my $file(@files){
    my $libs = `otool -L $file`;
    my @lines = split("\n",$libs);
    foreach my $line (@lines){
      $line =~ /\s*(.*\.dylib)/;
      my $old = $1;
      $line =~ /([^\/]*\.dylib)/;
      my $dylib = $1;
      if(defined $dylib && -f "../lib/$dylib"){
	system("install_name_tool -change $old \@loader_path/../lib/$dylib $file");
      }
    }
  }
}


my $reldir = $ARGV[0];

my $libdir = $reldir."/lib/";
my $scriptdir = $reldir."/scripts/";
my $utilsdir = $reldir."/utils/";
my $bindir = $reldir."/bin/";
`mkdir -p $scriptdir`;

chdir($reldir);
mkdir("lib");
chdir("lib");
my @deps = get_all_dependencies("../bin");
foreach my $dep(@deps){
    # Only package certain dependencies
    unless($dep =~ /libQt/ || $dep =~ /libtiff/ ||  $dep =~ /libpng/ || $dep =~ /libgsl/ || 
	   $dep =~ /libgsl/ || $dep =~ /libaudio\./ || $dep =~ /libhdf5/  ||
	   $dep =~ /libjpeg/ || $dep =~ /libqwt/ || $dep =~ /libspimage/ || $dep =~  /libz\.so/ ||
	   $dep =~ /libfftw/ || $dep =~ /libsz\./){
	next;
    }
    if(-f $dep){
      system("cp $dep $libdir");
    }
}

if(`uname -s` =~ /Darwin/){
  change_install_name($bindir);
  change_install_name($libdir);
}
chdir($reldir);
chdir("..");

my $red_reldir = $reldir;
chomp($red_reldir);
$red_reldir = `basename $red_reldir`;
system("tar -zcvf ".$reldir.".tar.gz $red_reldir");
