#!/usr/bin/perl -w

use File::Spec;

if((scalar @ARGV) < 4){
    print "Usage: select_by_error.pl <rs error max> <fs error max> <reconstruction directories> <output directory>\n";
    exit(0);
}

if(-e $ARGV[scalar @ARGV-1]){
    print "The output directory must not exist!\n";
    exit(0);
}
my $rs_max = $ARGV[0];
my $fs_max = $ARGV[1];
my $out_dir = $ARGV[scalar @ARGV-1];
system("mkdir -p $out_dir");
for(my $i = 2;$i<(scalar @ARGV)-1;$i++){
    my $img_dir = File::Spec->rel2abs($ARGV[$i]);
    my $log_file = $ARGV[$i]."/uwrapc.log";
    if(-e $log_file){
	my $rs_error = `tail -n 1 $log_file | awk '{print \$3}'`;
	my $fs_error = `tail -n 1 $log_file | awk '{print \$4}'`;
	if($rs_error < $rs_max && 
	   $fs_error < $fs_max){
	    # create symlink on the output directory 
	    system("ln -s $img_dir $out_dir");
	}
    }
}

