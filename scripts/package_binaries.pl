#!/usr/bin/perl -w

# This script assumes no intervention is necessary on cmake

use strict;
use Cwd 'abs_path';
use File::Basename;
use File::Spec;    

sub get_dependencies{
  my $bin = shift;
  my $use_ldd = 1;
  if(`uname -s` =~ /Darwin/ || !`which ldd`){
    $use_ldd = 0;
  }
  my $libs;
  if($use_ldd ){
    $libs = `ldd $bin`;
  }else{
    $libs = `otool -L $bin`;
  }
  my @lines = split("\n",$libs);
  my @deps;
  foreach my $line (@lines){
    my $lib;
    if($use_ldd){
      $line =~ /.*? => (.*?) \(0x[0-9abcdef]+\)/;
      $lib = $1;
    }else{
      if($line =~ /\@rpath\/(libcu.*dylib)/ || $line =~ /\@rpath\/(libtlshook.dylib)/){
	# Assumes that CUDA is installed in /usr/local/cuda and that
	# cuda libraries set rpath and no one else does, which at the moment is true
	$lib = "/usr/local/cuda/lib/".$1;
      }elsif($line =~ /\s*(libqwt.*?)\s/){
	$lib = "/usr/local/lib/".$1;
      }elsif($line =~ /\s*(.*dylib)/){
	$lib = $1;
      }elsif($line =~ /\s*(Qt.*?)\s/){
	$lib = "/Library/Frameworks/".$1;
      }
    }
    if(defined $lib && $lib ne ""){
      push(@deps, $lib);
    }
  }
  return @deps;
}

sub get_dependencies_recursively{
  my %deps;
  my $bin = shift;
  my @out = get_dependencies($bin);
  while(scalar(@out)){
    my $dep = pop(@out);
    unless($deps{$dep}){
      $deps{$dep} = 1;
      push(@out,get_dependencies($dep));
    }
  }
  return keys(%deps);
}

sub get_all_dependencies{
  my $bindir = shift;
  my $dir = `pwd`;
  chdir($bindir);
  my $f = `ls -1`;
  my @files = split("\n",$f);
  my %deps;
#  print @files;
  foreach my $file(@files){
    my @deps =  get_dependencies_recursively($file);
    foreach my $dep(@deps){
      $deps{$dep} = 1;
    }
  }
  foreach my $key(keys %deps){
    print $key."\n";
  }
  chdir($dir);
  return keys %deps;
}

my $basedir =  dirname( File::Spec->rel2abs(__FILE__));
(my $sec,my $min,my $hour,my $mday,my $mon,my $year,my $wday,my $yday,my $isdst) =
                                                               localtime(time);

my $version = sprintf("%02d.%02d.%02d",$year%100,$mon+1,$mday);
my $arch = `gcc -dumpmachine`;
chomp($arch);
my $reldir =($basedir."/../releases/hawk-$version-$arch");
my $builddir = ($basedir."/../releases/hawk-build-$arch");
`rm -rf $reldir`;
`mkdir -p $reldir`;
`mkdir -p $builddir`;
$reldir = abs_path($reldir);
my $libdir = $reldir."/lib/";
my $utilsdir = $reldir."/utils/";
my $bindir = $reldir."/bin/";
$builddir = abs_path($builddir);
print $builddir;
chdir($builddir) or die($!);
print `pwd`;
if(`uname -s` =~ /MINGW32/){
    system("cmake -G \"MSYS Makefiles\" ../.. -DCMAKE_BUILD_TYPE=release -DCMAKE_INSTALL_PREFIX:PATH=$reldir");
}elsif(`uname -s` =~ /Darwin/){
#    system("cmake ../.. -DCMAKE_BUILD_TYPE=release -DCMAKE_INSTALL_PREFIX:PATH=$reldir -DCMAKE_OSX_DEPLOYMENT_TARGET=10.5");
    system("cmake ../.. -DCMAKE_BUILD_TYPE=release -DCMAKE_INSTALL_PREFIX:PATH=$reldir -DCMAKE_OSX_DEPLOYMENT_TARGET=10.6");
}else{
    system("cmake ../.. -DCMAKE_BUILD_TYPE=release -DCMAKE_INSTALL_PREFIX:PATH=$reldir");
}
system("make -j 4");
system("make install");
chdir($reldir);
mkdir("lib");
chdir("lib");
unless(`uname -s` =~ /MINGW32/){
    my @deps = get_all_dependencies("../bin");
    foreach my $dep(@deps){
	# Only package certain dependencies
	unless($dep =~ /Qt/ || $dep =~ /libtiff/ ||  $dep =~ /libpng/ || $dep =~ /libgsl/ || 
	       $dep =~ /libgsl/ || $dep =~ /libaudio\./ || $dep =~ /libhdf5/  ||
	       $dep =~ /libjpeg/ || $dep =~ /libqwt/ || $dep =~ /libspimage/ || $dep =~  /libz\.so/ ||
	       $dep =~ /libfftw/ || $dep =~ /libsz\./ || $dep =~ /libssl\./ || $dep =~ /libcrypto\./ ||
	       $dep =~ /libcufft/ || $dep =~ /libcuda/ || $dep =~ /libcudart/ || $dep =~ /libtlshook/){
	    next;
	}
	if(-f $dep){
	  if($dep =~ /(.*\.framework)/){
#	    system("cp -R $1 $libdir");
#	    $dep =~ /([^\/]*\.framework)/;
#	    my $fw = $1;
#	    system("find $libdir/$fw -name \'\*_debug*\' -exec rm -rf {} \\;");
	  }elsif($dep =~ s/libQt.*/libQt\*/){
	    #copy all Qt libs
	    system("cp -d $dep $libdir");
	  }else{
	    system("cp $dep $libdir");
	  }
	}
      }
# remove debug libs and strip the rest of the libs
    system("rm $libdir/*.debug");
    system("chmod +w $libdir/*");
    if(`uname -s` =~ /Darwin/){
      system("strip -x $libdir/*");
    }
    if(`uname -s` =~ /Linux/){
      system("strip -s $libdir/*");
    }
}


if(`uname -s` =~ /Darwin/){
  chdir($builddir);
  system("make macosx_bundle");
}

if(`uname -s` =~ /Linux/){
  chdir($builddir);
  system("make linux_bundle");
}

if(`uname -s` =~ /MINGW32/){
  chdir($builddir);
  system("make windows_bundle");
}

chdir($reldir);
chdir("..");
if(`uname -s` =~ /Darwin/){
  system("hdiutil create -srcfolder hawk-$version-$arch -volname Hawk-$version Hawk-$version.dmg");
}else{
  system("tar -zcvf ".$reldir.".tar.gz hawk-$version-$arch");
}
