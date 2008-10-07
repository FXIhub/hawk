#!/usr/bin/perl -w

use strict;

if((scalar @ARGV) < 1){    
    print "Usage: repeat_reconstruction <n reconstructions per thread> [starting n] [nthreads]\n";
    exit(0);
}
print "Total reconstructions - ".$ARGV[0]."\n";
my $nrec = $ARGV[0];
my $i0 = 0;
my $nthreads = 1;

if((scalar @ARGV) >= 2){
	$i0 = $ARGV[1];
}

if((scalar @ARGV) >= 3){
	$nthreads = $ARGV[2];
}

print "First reconstruction - ".$i0."\n";

if($nthreads > 1){
    for(my $i = 0;$i<$nthreads;$i++){
	my $child_pid;
	if (!defined($child_pid = fork())) {
	    die "cannot fork: $!";
	} elsif ($child_pid) {
	    # I'm the parent
	    # Increment starting point
	    $i0 += $nrec;
	} else {
	    # I'm the child
	    for($i = $i0 ;$i<$nrec+$i0;$i++){
		my $dir = printf("%06d",$i);
		system("mkdir -p $dir");
		system("cp uwrapc.conf $dir");
		system("cd $dir && uwrapc");
	    }
	    #don't loop!
	    last;
	} 
    }
}else{
    for(my $i = $i0 ;$i<$nrec+$i0;$i++){
	my $dir = printf("%06d",$i);
	system("mkdir -p $dir");
	system("cp uwrapc.conf $dir");
	system("cd $dir && uwrapc");
    }
}


