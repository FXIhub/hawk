#!/usr/bin/perl -w

# This script assumes no intervention is necessary on cmake


use strict;
use Cwd 'abs_path';
use File::Basename;


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
    my $libs = `otool -L $file`;
    my @lines = split("\n",$libs);
    foreach my $line (@lines){
      my $old;
#      $line =~ s/\@rpath/\/usr\/local\/cuda\/lib/;
      my $dylib;

      if($line =~ /\s*(.*\.dylib)/){
	$old = $1;
      }
      if($line =~ /\s*(Qt.*?)\s/){
	$old = $1;
      }
      if($line =~ /([^\s\/]*\.dylib)/){
	$dylib = $1;
      }
      if($line =~ /(Qt.*?)\s/){
	$dylib = $1;
      }
      print "$file\n";
      print "$old $dylib\n";
      if(defined $dylib && -f "../lib/$dylib" || -f "../../lib/$dylib"){
	system("install_name_tool -change $old \@executable_path/../lib/$dylib $file");
	if($dylib =~ /$file/){
	  system("install_name_tool -id \@executable_path/../lib/$dylib $file");
	}
      }else{
	print "Condition not met\n";
      }
    }
  }
}

sub bundle_HawkGUI{
  print `pwd`;
  mkdir("HawkGUI.app");
  mkdir("HawkGUI.app/Contents");
  mkdir("HawkGUI.app/Contents/Resources");
  mkdir("HawkGUI.app/Contents/MacOS");
  my $heredoc = <<END;
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
	<string>com.novell.monodoc</string>
	<key>CFBundleInfoDictionaryVersion</key>
	<string>6.0</string>
	<key>CFBundleName</key>
	<string>HawkGUI</string>
	<key>CFBundlePackageType</key>
	<string>APPL</string>
	<key>CFBundleShortVersionString</key>
	<string>2.2</string>
	<key>CFBundleSignature</key>
	<string>xmmd</string>
	<key>CFBundleVersion</key>
	<string>2.2</string>
	<key>NSAppleScriptEnabled</key>
	<string>NO</string>
</dict>
</plist>
END
  system("echo '$heredoc' > HawkGUI.app/Contents/Info.plist");
  system("mv HawkGUI HawkGUI.app/Contents/MacOS");
  system("cd HawkGUI.app/Contents && rm lib && ln -s ../../../lib lib");
# This is necessary to prevent the program from loading system Qt is it's installed
  my $heredoc = <<END;
[Paths]
Plugins = bollocks
END
  system("echo '$heredoc' > HawkGUI.app/Contents/Resources/qt.conf");
  # Copy icon
  system("cp ../../../src/HawkGUI/images/Hawk.icns HawkGUI.app/Contents/Resources");
}

my $reldir = $ARGV[0];

my $libdir = $reldir."/lib/";
my $bindir = $reldir."/bin/";


if(`uname -s` =~ /Darwin/){
  # Copy some plugins
  mkdir($libdir."/imageformats");
  system("cp /Developer/Applications/Qt/plugins/imageformats/* ".$libdir."/imageformats");
  system("rm ".$libdir."/imageformats/libqsvg*");

  change_install_name($bindir);
  change_install_name($libdir);
  change_install_name($libdir."/imageformats");
  chdir($bindir);
  bundle_HawkGUI();
}
