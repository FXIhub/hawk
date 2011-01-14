#!/usr/bin/perl -w

use strict;
use Cwd 'abs_path';
use File::Basename;

my $reldir = $ARGV[0];
my $bindir = $reldir."/bin/";
chdir($bindir);
mkdir(".bins");
my $f = `ls -1`;
my @files = split("\n",$f);
foreach my $file(@files){
    # Skip non binary files 
    unless(-B $file){
      next;
    }
    # Replace file by a wrapper perl script
    rename($file,".bins/".$file);
    my $wrapper = <<'END';
#!/usr/bin/perl -w
use strict;
use Cwd 'abs_path';
my $basedir =  abs_path(__FILE__);
my $basename = `basename $basedir`;
$basedir = `dirname $basedir`;
chomp($basedir);
chomp($basename);
my $binsdir = $basedir."/.bins";   
my $libdir = $basedir."/../lib";
unless(defined $ENV{LD_LIBRARY_PATH}){
    $ENV{LD_LIBRARY_PATH} = "";
}
$ENV{LD_LIBRARY_PATH} = $libdir.":".$ENV{LD_LIBRARY_PATH};
exec("$binsdir/$basename");
END
    open(OUT,">$file");
    print OUT $wrapper;
    close(OUT);
    system("chmod +x $file");
}
