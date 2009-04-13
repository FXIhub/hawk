#!/usr/bin/perl -w

use strict;
use Cwd 'abs_path';

my $basedir =  abs_path(__FILE__);
$basedir = `dirname $basedir`;
chomp($basedir);
my $bindir = $basedir."/bin";
my $libdir = $basedir."/lib";
my $mandir = $basedir."/man";
my $datadir = $basedir."/share";

open(FIN, "< scripts/hawkrc.in") or die("Could not open scripts/hawkrc.in");
open(FOUT, "> bin/hawkrc") or die("Could not open bin/hawkrc");
while($_ = <FIN>){
  s/\@bindir@/$bindir/g;
  s/\@libdir@/$libdir/g;
  s/\@mandir@/$mandir/g;
  s/\@datadir@/$datadir/g;
  print FOUT;
}
close FOUT;
close FIN;

open(FIN, "< scripts/hawkrc.zsh.in") or die("Could not open scripts/hawkrc.zsh.in");
open(FOUT, "> bin/hawkrc.zsh") or die("Could not open bin/hawkrc.zsh");
while($_ = <FIN>){
  s/\@bindir@/$bindir/g;
  s/\@libdir@/$libdir/g;
  s/\@mandir@/$mandir/g;
  s/\@datadir@/$datadir/g;
  print FOUT;
}
close FOUT;
close FIN;


open(FIN, "< scripts/hawkrc.bash.in") or die("Could not open scripts/hawkrc.bash.in");
open(FOUT, "> bin/hawkrc.bash") or die("Could not open bin/hawkrc.bash");
while($_ = <FIN>){
  s/\@bindir@/$bindir/g;
  s/\@libdir@/$libdir/g;
  s/\@mandir@/$mandir/g;
  s/\@datadir@/$datadir/g;
  print FOUT;
}
close FOUT;
close FIN;

open(FIN, "< scripts/hawkrc.csh.in") or die("Could not open scripts/hawkrc.csh.in");
open(FOUT, "> bin/hawkrc.csh") or die("Could not open bin/hawkrc.csh");
while($_ = <FIN>){
  s/\@bindir@/$bindir/g;
  s/\@libdir@/$libdir/g;
  s/\@mandir@/$mandir/g;
  s/\@datadir@/$datadir/g;
  print FOUT;
}
close FOUT;
close FIN;
