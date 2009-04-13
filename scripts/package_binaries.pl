#!/usr/bin/perl -w

# This script assumes no intervention is necessary on cmake


use strict;
use Cwd 'abs_path';
use File::Basename;

sub get_dependencies{
  my $bin = shift;
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
    if($lib ne ""){
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
    print $key."\n";
  }
  chdir($dir);
  return keys %deps;
}

my $basedir =  dirname(abs_path(__FILE__));
(my $sec,my $min,my $hour,my $mday,my $mon,my $year,my $wday,my $yday,my $isdst) =
                                                               localtime(time);

my $version = sprintf("%02d.%02d.%02d",$year%100,$mon,$mday);
my $arch = `gcc -dumpmachine`;
chomp($arch);
my $reldir =($basedir."/../releases/hawk-$version-$arch");
my $builddir = ($basedir."/../releases/hawk-build-$arch");
`mkdir -p $reldir`;
`mkdir -p $builddir`;
$reldir = abs_path($reldir);
my $libdir = $reldir."/lib/";
my $scriptdir = $reldir."/scripts/";
`mkdir -p $scriptdir`;
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
my @deps = get_all_dependencies("../bin");
foreach my $dep(@deps){
  system("cp $dep $libdir");
}
system("cp $basedir/hawkrc* $scriptdir");
system("cp $basedir/setup.pl $reldir");
chdir($reldir);
chdir("..");
system("tar -zcvf ".$reldir.".tar.gz  $reldir")
