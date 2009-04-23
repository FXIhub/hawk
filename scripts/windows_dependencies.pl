#!/usr/bin/perl -w

use strict;
use Cwd 'abs_path';
use File::Basename;

my $reldir = $ARGV[0];
my $bindir = $reldir."/bin/";
chdir($bindir);
system("cp /mingw/bin/libgsl-0.dll '$bindir'");
system("cp /mingw/bin/libgslcblas-0.dll '$bindir'");
system("cp /mingw/bin/libjpeg-62.dll '$bindir'");
system("cp /mingw/bin/libpng12.dll '$bindir'");
system("cp /mingw/bin/libspimage.dll '$bindir'");
system("cp /mingw/bin/libtiff-3.dll '$bindir'");
system("cp /mingw/bin/mingwm10.dll '$bindir'");
system("cp /mingw/bin/qwt5.dll '$bindir'");
system("cp /mingw/bin/zlib1.dll '$bindir'");
system("cp /c/Qt/4.5.0/bin/QtCore4.dll '$bindir'");
system("cp /c/Qt/4.5.0/bin/QtGui4.dll '$bindir'");
system("cp /c/Qt/4.5.0/bin/QtNetwork4.dll '$bindir'");
