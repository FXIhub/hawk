#!/usr/bin/python

import time
import urllib
import os
import sys
import zipfile
import subprocess
import glob
import shutil
import datetime

def _reporthook(numblocks, blocksize, filesize, url=None):
#print "reporthook(%s, %s, %s)" % (numblocks, blocksize, filesize)
    base = os.path.basename(url)
#XXX Should handle possible filesize=-1.
    try:
        percent = min((numblocks*blocksize*100)/filesize, 100)
    except:
        percent = 100
    if numblocks != 0:
        sys.stdout.write("\b"*70)
        sys.stdout.write("%-66s%3d%%" % (base, percent))

def geturl(url, dst):
    print "get url '%s' to '%s'" % (url, dst)
    if sys.stdout.isatty():
        urllib.urlretrieve(url, dst,
                           lambda nb, bs, fs, url=url: _reporthook(nb,bs,fs,url))
        sys.stdout.write('\n')
    else:
        urllib.urlretrieve(url, dst)


cmake_url = "http://www.cmake.org/files/v2.8/cmake-2.8.3-win32-x86.exe";
cmake_exe = "cmake-2.8.3-win32-x86.exe";
if(not os.path.exists(cmake_exe)):
    geturl(cmake_url,cmake_exe);
    os.system(cmake_exe);

git_url = "http://msysgit.googlecode.com/files/msysGit-fullinstall-1.7.4-preview20110204.exe"
git_exe = "msysGit-fullinstall-1.7.4-preview20110204.exe"
if(not os.path.exists(git_exe)):
    geturl(git_url,git_exe);
    os.system(git_exe);

gsl_exe = "gsl-1.8.exe"
gsl_url = "http://sourceforge.net/projects/gnuwin32/files/gsl/1.8/gsl-1.8.exe/download"

if(not os.path.exists(gsl_exe)):
    geturl(gsl_url,gsl_exe);
    os.system(gsl_exe);

jpeg_exe = "jpeg.exe";
jpeg_url = "http://gnuwin32.sourceforge.net/downlinks/jpeg.php"

if(not os.path.exists(jpeg_exe)):
    geturl(jpeg_url,jpeg_exe);
    os.system(jpeg_exe);

png_exe = "png.exe";
png_url = "http://gnuwin32.sourceforge.net/downlinks/libpng.php"

if(not os.path.exists(png_exe)):
    geturl(png_url,png_exe);
    os.system(png_exe);

tiff_exe = "tiff.exe";
tiff_url = "http://gnuwin32.sourceforge.net/downlinks/tiff.php"

if(not os.path.exists(tiff_exe)):
    geturl(tiff_url,tiff_exe);
    os.system(tiff_exe);

fftw_zip = "fftw-3.2.2-dll32.zip";
fftw_url = "ftp://ftp.fftw.org/pub/fftw/fftw-3.2.2.pl1-dll32.zip";

if(not os.path.exists(fftw_zip)):
    geturl(fftw_url,fftw_zip);
    file = zipfile.ZipFile(fftw_zip, "r");
    file.extractall("c:/fftw3");

hdf5_url = "http://www.hdfgroup.org/ftp/HDF5/current/bin/windows/hdf5-1.8.5-patch1-win32.zip"
hdf5_zip = "hdf5-1.8.5-patch1-win32.zip"

if(not os.path.exists(hdf5_zip)):
    geturl(hdf5_url,hdf5_zip);
    file = zipfile.ZipFile(hdf5_zip, "r");
    file.extractall("c:/hdf5");


git_bat = """@rem Do not use "echo off" to not affect any child calls.
@setlocal

@rem Get the abolute path to the current directory, which is assumed to be the
@rem Git installation root.
@for /F "delims=" %%I in ("%~dp0") do @set git_install_root=%%~fI
@set PATH=%git_install_root%\\bin;%git_install_root%\\mingw\\bin;%git_install_root%\\cmd;%PATH%

@if not exist "%HOME%" @set HOME=%HOMEDRIVE%%HOMEPATH%
@if not exist "%HOME%" @set HOME=%USERPROFILE%

@set PLINK_PROTOCOL=ssh
@cd %HOME%/Desktop

if exist hawk (
   cd hawk
   git pull
   cd ..
) else (
  git clone https://github.com/FilipeMaia/hawk.git
)
if exist libspimage (
   cd libspimage
   git pull
   cd ..
) else (
  git clone https://github.com/FilipeMaia/libspimage.git
)""";
#if(not os.path.exists('c:\msysgit\msysgit\gitclone.bat')):
file = open('c:\msysgit\msysgit\gitclone.bat', 'w')
print >>file,git_bat
file.close()
os.system("c:\msysgit\msysgit\gitclone");

mingw_url = "http://sourceforge.net/projects/mingw/files/Automated%20MinGW%20Installer/mingw-get-inst/mingw-get-inst-20110211/mingw-get-inst-20110211.exe/download";
mingw_exe = "mingw-get-inst-20110211.exe"

if(not os.path.exists(mingw_exe)):
    geturl(mingw_url,mingw_exe);
    os.system(mingw_exe);

msys_url = "http://downloads.sourceforge.net/mingw/MSYS-1.0.11.exe";
msys_exe = "MSYS-1.0.11.exe";

if(not os.path.exists(msys_exe)):
    geturl(msys_url,msys_exe);
    os.system(msys_exe);


qt_url = "http://get.qt.nokia.com/qt/source/qt-win-opensource-4.7.1-mingw.exe"
qt_exe = "qt-win-opensource-4.7.1-mingw.exe"

if(not os.path.exists(qt_exe)):
    geturl(qt_url,qt_exe);
    os.system(qt_exe);

qwt_url = "http://sourceforge.net/projects/qwt/files/qwt/5.2.1/qwt-5.2.1.zip/download"
qwt_zip = "qwt-5.2.1.zip"
if(not os.path.exists(qwt_zip)):
    geturl(qwt_url,qwt_zip);
    file = zipfile.ZipFile(qwt_zip, "r");
    file.extractall("c:/qwt");
    os.mkdir("C:/Qwt-5.2.1");
    bashrc = """cd /c/qwt/qwt-5.2.1
/c/Qt/4.7.1/bin/qmake.exe
make
make install
"""
    file = 'c:/msys/1.0/home/compile/.profile'
    file = open(file, 'w')
    print >>file,bashrc
    file.close()
    p = subprocess.Popen(['c:/msys/1.0/msys.bat'])
    p.wait()

#sys.path.append("c:/MinGW/bin")
#sys.path.append("c:/MinGW/msys/1.0/bin")
#sys.path.append("c:/Program Files/CMake 2.8/bin/")
if(not os.path.exists("libspimage/build")):
    os.mkdir("libspimage/build");
os.chdir("libspimage/build");
if(os.path.exists("CMakeCache.txt")):
    os.unlink("CMakeCache.txt");
#env_map = {"PATH": os.environ["PATH"]+";test"}
env_map = os.environ;
env_map["PATH"] = env_map["PATH"]+";c:/MinGW/msys/1.0/bin";
env_map["PATH"] = env_map["PATH"]+";c:/MinGW/bin";
p = subprocess.Popen(['c:/Program Files/CMake 2.8/bin/cmake',
                  '-G','MSYS Makefiles',
                  '-DTIFF_LIBRARY:FILEPATH=c:/Program Files/GnuWin32/bin/libtiff3.dll',
                  '-DTIFF_INCLUDE_DIR:PATH=c:/Program Files/GnuWin32/include',
                  '-DJPEG_LIBRARY:FILEPATH=c:/Program Files/GnuWin32/bin/jpeg62.dll',
                  '-DJPEG_INCLUDE_DIR:PATH=c:/Program Files/GnuWin32/include',
                  '-DPNG_LIBRARY:FILEPATH=c:/Program Files/GnuWin32/bin/libpng3.dll',
                  '-DPNG_PNG_INCLUDE_DIR:PATH=c:/Program Files/GnuWin32/include',
                  '-DZLIB_LIBRARY:FILEPATH=c:/Program Files/GnuWin32/bin/zlib1.dll',
                  '-DZLIB_INCLUDE_DIR:PATH=c:/Program Files/GnuWin32/include',
                  '-DFFTW3_FFTWF_LIBRARY:FILEPATH=c:/fftw3/libfftw3f-3.dll',
                  '-DFFTW3_FFTW_LIBRARY:FILEPATH=c:/fftw3/libfftw3-3.dll',
                  '-DFFTW3_FFTWL_LIBRARY:FILEPATH=c:/fftw3/libfftw3l-3.dll',
                  '-DFFTW3_INCLUDE_DIR:PATH=c:/fftw3',
                  '-DHDF5_LIBRARY:FILEPATH=c:/hdf5/dll/hdf5dll.dll',
                  '-DHDF5_INCLUDE_DIR:PATH=c:/hdf5/include',
                  '-DMATH_LIBRARY:FILE_PATH=c:/fftw3/libfftw3f-3.dll',
                  '-DMATH_INCLUDE_DIR:PATH=c:/hdf5/include',
                  '-DGSL_GSL_LIBRARY:FILEPATH=c:/Program Files/GnuWin32/bin/libgsl.dll',
                  '-DGSL_GSLCBLAS_LIBRARY:FILEPATH=c:/Program Files/GnuWin32/bin/libgslcblas.dll',
                  '-DGSL_INCLUDE_DIR:PATH=c:/Program Files/GnuWin32/include',
                  '-DCMAKE_VERBOSE_MAKEFILE:BOOL=FALSE',
                  '-DCMAKE_INSTALL_PREFIX=C:/',
                  '-DCMAKE_BUILD_TYPE=release',
                  '..'], env=env_map)

ret = p.wait();
print "Process complete %d\n" % (ret)

bashrc = """cd /c/Documents\\ and\\ Settings/compile/Desktop/libspimage/build
make
make install
"""
file = 'c:/msys/1.0/home/compile/.profile'
file = open(file, 'w')
print >>file,bashrc
file.close()
p = subprocess.Popen(['c:/msys/1.0/msys.bat'])
ret = p.wait()
print "Process complete %d\n" % (ret)
os.chdir("..");
os.chdir("..");
time.sleep(20);


if(not os.path.exists("hawk/build")):
    os.mkdir("hawk/build");
os.chdir("hawk/build");
if(os.path.exists("CMakeCache.txt")):
    os.unlink("CMakeCache.txt");
env_map = os.environ;
env_map["PATH"] = env_map["PATH"]+";c:/MinGW/msys/1.0/bin";
env_map["PATH"] = env_map["PATH"]+";c:/MinGW/bin";
p = subprocess.Popen(['c:/Program Files/CMake 2.8/bin/cmake',
                  '-G','MSYS Makefiles',
                  '-DTIFF_LIBRARY:FILEPATH=c:/Program Files/GnuWin32/bin/libtiff3.dll',
                  '-DTIFF_INCLUDE_DIR:PATH=c:/Program Files/GnuWin32/include',
                  '-DJPEG_LIBRARY:FILEPATH=c:/Program Files/GnuWin32/bin/jpeg62.dll',
                  '-DJPEG_INCLUDE_DIR:PATH=c:/Program Files/GnuWin32/include',
                  '-DPNG_LIBRARY:FILEPATH=c:/Program Files/GnuWin32/bin/libpng3.dll',
                  '-DPNG_PNG_INCLUDE_DIR:PATH=c:/Program Files/GnuWin32/include',
                  '-DZLIB_LIBRARY:FILEPATH=c:/Program Files/GnuWin32/bin/zlib1.dll',
                  '-DZLIB_INCLUDE_DIR:PATH=c:/Program Files/GnuWin32/include',
                  '-DFFTW3_FFTWF_LIBRARY:FILEPATH=c:/fftw3/libfftw3f-3.dll',
                  '-DFFTW3_FFTW_LIBRARY:FILEPATH=c:/fftw3/libfftw3-3.dll',
                  '-DFFTW3_FFTWL_LIBRARY:FILEPATH=c:/fftw3/libfftw3l-3.dll',
                  '-DFFTW3_INCLUDE_DIR:PATH=c:/fftw3',
                  '-DHDF5_LIBRARY:FILEPATH=c:/hdf5/dll/hdf5dll.dll',
                  '-DHDF5_INCLUDE_DIR:PATH=c:/hdf5/include',
                  '-DMATH_LIBRARY:FILE_PATH=c:/fftw3/libfftw3f-3.dll',
                  '-DMATH_INCLUDE_DIR:PATH=c:/hdf5/include',
                  '-DGSL_GSL_LIBRARY:FILEPATH=c:/Program Files/GnuWin32/bin/libgsl.dll',
                  '-DGSL_GSLCBLAS_LIBRARY:FILEPATH=c:/Program Files/GnuWin32/bin/libgslcblas.dll',
                  '-DGSL_INCLUDE_DIR:PATH=c:/Program Files/GnuWin32/include',
                  '-DCMAKE_VERBOSE_MAKEFILE:BOOL=FALSE',
                  '-DQT_QMAKE_EXECUTABLE:FILEPATH=C:/Qt/4.8.2/bin/qmake',
                  '-DQWT_INCLUDE_DIR:PATH=C:/qwt-5.2.2/include',
                  '-DQWT_LIBRARY:FILEPATH=C:/qwt-5.2.2/lib/qwt5.dll',
                  '-DSPIMAGE_INCLUDE_DIR:PATH=C:/include',
                  '-DSPIMAGE_LIBRARY:FILEPATH=C:/bin/libspimage.dll',
                  '-DCMAKE_INSTALL_PREFIX=C:/',
                  '-DCMAKE_BUILD_TYPE=release',
                  '..'], env=env_map)

p.wait();


bashrc = """cd /c/Documents\\ and\\ Settings/compile/Desktop/hawk/build
make
make install
"""
file = 'c:/msys/1.0/home/compile/.profile'
file = open(file, 'w')
print >>file,bashrc
file.close()
p = subprocess.Popen(['c:/msys/1.0/msys.bat'])




if(not os.path.exists("c:/bin")):
    os.mkdir("c:/bin")
dlls =  glob.glob("c:/hdf5/dll/*.dll")
for dll in dlls:
    shutil.copy(dll,"c:/bin");
dlls =  glob.glob("c:/fftw3/*.dll")
print dlls;
for dll in dlls:
    shutil.copy(dll,"c:/bin");
dlls =  glob.glob("c:/Program Files/GnuWin32/bin/*.dll")
print dlls;
for dll in dlls:
    shutil.copy(dll,"c:/bin");

shutil.copy("c:/Qt/4.8.2/bin/QtCore4.dll","c:/bin");
shutil.copy("c:/Qt/4.8.2/bin/QtGui4.dll","c:/bin");
shutil.copy("c:/Qt/4.8.2/bin/QtNetwork4.dll","c:/bin");
shutil.copy("c:/MinGW/bin/mingwm10.dll","c:/bin");
shutil.copy("c:/MinGW/bin/libgcc_s_dw2-1.dll","c:/bin");
shutil.copy("c:/MinGW/bin/libstdc++-6.dll","c:/bin");
dlls =  glob.glob("c:/qwt-5.2.2/lib/*.dll")
print dlls;
for dll in dlls:
    shutil.copy(dll,"c:/bin");

date = datetime.date.today().strftime("%y.%m.%d")
rel = "c:/hawk-"+date+"-mingw32"
if(os.path.exists(rel)):
    shutil.rmtree(rel)
os.mkdir(rel)
shutil.copytree("c:/bin",rel+"/bin");
shutil.copytree("c:/doc",rel+"/doc");
shutil.copytree("c:/examples",rel+"/examples");





