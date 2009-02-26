#!/usr/bin/perl -w

opendir(DIR, ".");
@files = grep(/\.CRW$/,readdir(DIR));
closedir(DIR);

foreach $file (@files) {
    $file =~ m/(.*)\.crw/i;
    $base = $1;
    unless(-e "./".$base){
	system("EOS300D_CRW2H5.pl $file");
    }
}
