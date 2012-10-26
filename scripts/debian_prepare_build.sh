#!/bin/sh

apt-get install  build-essential libtiff4-dev libpng12-dev libfftw3-dev libhdf5-serial-dev libgsl0-dev git git-core wget swig python-dev  python-scipy  subversion
arch=`uname -m`
if [ "$arch" = "x86_64" ];then
if test ! -f cudatoolkit_3.1_linux_64_ubuntu9.10.run; then
	wget developer.download.nvidia.com/compute/cuda/3_1/toolkit/cudatoolkit_3.1_linux_64_ubuntu9.10.run
	sh cudato*_64_*
	echo 'export PATH=${PATH}:/usr/local/cuda/bin' >> ~/.bashrc
	echo 'export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:/usr/local/cuda/lib64' >> ~/.bashrc
	source ~/.bashrc
fi
else
if test ! -f cudatoolkit_3.1_linux_32_ubuntu9.10.run; then
	wget developer.download.nvidia.com/compute/cuda/3_1/toolkit/cudatoolkit_3.1_linux_32_ubuntu9.10.run
	sh cudato*_32_*
	echo 'export PATH=${PATH}:/usr/local/cuda/bin' >> ~/.bashrc
	echo 'export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:/usr/local/cuda/lib' >> ~/.bashrc
	source ~/.bashrc
fi
fi
if test ! -d libspimage; then
	git clone git://github.com/FilipeMaia/libspimage.git
fi
if test ! -d hawk; then
	git clone git://github.com/FilipeMaia/hawk.git
fi
if test ! -f cmake-2.8.2-Linux-i386.tar.gz; then
	wget www.cmake.org/files/v2.8/cmake-2.8.2-Linux-i386.tar.gz
	tar -zxvf cmake-2*	
	echo 'export PATH=${PATH}:${HOME}/cmake-2.8.2-Linux-i386/bin' >> ~/.bashrc
fi


if test ! -f qt-sdk*; then
if [ "$arch" = "x86_64" ];then
	wget get.qt.nokia.com/qtsdk/qt-sdk-linux-x86_64-opensource-2010.04.bin
	chmod +x qt-sdk-linux-x86_64-opensource-2010.04.bin
	./qt-sdk-linux-x86_64-opensource-2010.04.bin
	echo 'export PATH=/opt/qtsdk-2010.04/qt/bin:${PATH}' >> ~/.bashrc

else
	wget get.qt.nokia.com/qtsdk/qt-sdk-linux-x86-opensource-2010.04.bin
	chmod +x qt-sdk-linux-x86-opensource-2010.04.bin
	./qt-sdk-linux-x86-opensource-2010.04.bin
	echo 'export PATH=/opt/qtsdk-2010.04/qt/bin:${PATH}' >> ~/.bashrc
fi
source ~/.bashrc
fi

if [ ! -d qwt-5.2 ]; then
	svn co https://qwt.svn.sourceforge.net/svnroot/qwt/branches/qwt-5.2
	cd qwt-5.2
	qmake
	make
	make install
	cp -r /usr/local/qwt-5.2-svn/* /usr
	cd
fi
cd libspimage && mkdir -p build && cd build && cmake -DCMAKE_INSTALL_PREFIX=/usr .. 
cd
cd hawk && git pull
cd
if test -f /usr/lib/libspimage.so; then
	cd hawk && mkdir -p build && cd build && cmake .. 
fi
