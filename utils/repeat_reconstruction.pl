#!/usr/bin/perl -w


if((scalar @ARGV) < 1){    
    print "Usage: repeat_reconstruction <n reconstructions>\n";
    exit(0);
}
print "Total reconstructions - ".$ARGV[0]."\n";

for($i = 0;$i<$ARGV[0];$i++){
    system("mkdir $i");
    system("cp uwrapc.conf $i");
    system("cd $i && uwrapc");
}


