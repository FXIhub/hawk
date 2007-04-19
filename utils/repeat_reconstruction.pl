#!/usr/bin/perl -w


if((scalar @ARGV) < 1){    
    print "Usage: repeat_reconstruction <n reconstructions> [starting n]\n";
    exit(0);
}
print "Total reconstructions - ".$ARGV[0]."\n";

if((scalar @ARGV) == 2){
	$i0 = $ARGV[1];
}else{
	$i0 = 0;
}

print "First reconstruction - ".$i0."\n";

for($i = $i0 ;$i<$ARGV[0]+$i0;$i++){
    system("mkdir $i");
    system("cp uwrapc.conf $i");
    system("cd $i && uwrapc");
}


