#!/usr/bin/perl -w

use strict;

my $prog = shift;

my $libs = `ldd $prog`;
my @lines = split("\n",$libs);

my $machine = `uname -sm|tr " /" "_"`;
chomp($machine);
my $output_dir = $machine."-dependencies";
system("mkdir -p $output_dir");
foreach my $line (@lines){
    $line =~ /.*? => (.*?) \(0x[0-9abcdef]+\)/;
    my $lib = $1;
    if($lib ne ""){
	# Do not copy all libs. Some basic ones should not be copied 
	if($lib =~ /\blibc\./){
	    next;
	}
	if($lib =~ /^\/lib/){
	    next;
	}
	print $lib."\n";
	system("cp $lib $output_dir");
    }
}




