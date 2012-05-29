#!/usr/bin/perl -w

# This script assumes no intervention is necessary on cmake


use strict;
use Cwd 'abs_path';
use File::Basename;


sub is_packaged_lib{
  my $lib = shift;
  if($lib =~ /libcuda.dylib/){
    return 1;
  }
  if($lib =~ /libcudart.dylib/){
    return 1;
  }
  if($lib =~ /libcufft.dylib/){
    return 1;
  }
  if($lib =~ /libgsl.dylib/){
    return 1;
  }
  if($lib =~ /libgslcblas.dylib/){
    return 1;
  }
  if($lib =~ /libqwt.dylib/){
    return 1;
  }
  if($lib =~ /libspimage/){
    return 1;
  }
  if($lib =~ /libtlshook/){
    return 1;
  }
  if($lib =~ /QtGui/){
    return 1;
  }
  if($lib =~ /QtCore/){
    return 1;
  }
  if($lib =~ /QtNetwork/){
    return 1;
  }
  if($lib =~ /libtiff/){
    return 1;
  }
  if($lib =~ /libfftw/){
    return 1;
  }
  if($lib =~ /libpng/){
    return 1;
  }
  if($lib =~ /libhdf5/){
    return 1;
  }
  if($lib =~ /libsz/){
    return 1;
  }
  if($lib =~ /libjpeg/){
    return 1;
  }  return 0;
    
}


sub change_install_name{
  my $bindir = shift;
  my $dir = `pwd`;
  chdir($bindir);  
  my $f = `ls -1`;
  my @files = split("\n",$f);
  chdir($dir);
  foreach my $file(@files){
    unless(-f $file){
      next;
    }
    system("chmod +w $file");
    print "File = $file\n";
    my $libs = `otool -L $file`;
    my @lines = split("\n",$libs);
    foreach my $line (@lines){
#      print "line - $line\n";
      my $old;
#      $line =~ s/\@rpath/\/usr\/local\/cuda\/lib/;
      my $dylib;
      if($line =~ /\s*(Qt.*?)\s/){
	$old = $1;
      }
      if($line =~ /\s*(.*\.dylib)/){
	$old = $1;
      }
      if($line =~ /([^\s\/]*\.dylib)/){
	$dylib = $1;
      }
      if($line =~ /(Qt.*?)\s/){
	$dylib = $1;
      }

      print "$file\n";
      print "old - $old dylib - $dylib\n";
      if(is_packaged_lib($old)){
	system("install_name_tool -change $old \@executable_path/../Frameworks/$dylib $file");
	if($dylib =~ /$file/){
	  system("install_name_tool -id \@executable_path/../FrameWorks/$dylib $file");
	}
      }
    }
  }
}

sub bundle_HawkGUI{
  print `pwd`;
  system("rm -rf HawkGUI.app");
  mkdir("HawkGUI.app");
  mkdir("HawkGUI.app/Contents");
  mkdir("HawkGUI.app/Contents/Resources");
  mkdir("HawkGUI.app/Contents/MacOS");
  system("mv HawkGUI HawkGUI.app/Contents/MacOS");
  # Copy icon
  system("cp ../../../src/HawkGUI/images/Hawk.icns HawkGUI.app/Contents/Resources");
  system("macdeployqt HawkGUI.app");
  my $plist = <<HERE;
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple Computer//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
        <key>CFBundleDevelopmentRegion</key>
        <string>English</string>
        <key>CFBundleExecutable</key>
        <string>HawkGUI</string>
        <key>CFBundleIconFile</key>
        <string>Hawk.icns</string>
        <key>CFBundleIdentifier</key>
        <string>org.cxidb.hawk</string>
        <key>CFBundleName</key>
        <string>Hawk</string>
        <key>CFBundlePackageType</key>
        <string>APPL</string>
        <key>CFBundleSignature</key>
        <string>????</string>
</dict>
</plist>
HERE
  open (MYFILE, '>HawkGUI.app/Contents/info.plist');
  print MYFILE $plist;
}

my $reldir = $ARGV[0];

my $libdir = $reldir."/lib/";
my $bindir = $reldir."/bin/";


if(`uname -s` =~ /Darwin/){
  change_install_name($bindir);
  change_install_name($libdir);
  change_install_name($libdir."/imageformats");
  chdir($bindir);
  bundle_HawkGUI();
  system("cp -a  ../lib/* HawkGUI.app/Contents/Frameworks");
  chdir("..");
  system("mv  bin/HawkGUI.app .");
  system("rm -rf bin");
  system("rm -rf lib");
  system("ln -s /Applications");
}
