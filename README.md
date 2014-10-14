Hawk - Image Reconstruction
===========================

Hawk is a collection of computer programs which aim at reconstructing objects from oversampled diffraction data.

<br>
Principle Authors:<br>
Filipe Maia     ||  <filipe.c.maia@gmail.com><br>
Tomas Ekeberg   ||  <ekeberg@xray.bmc.uu.se><br>
Max Hantke      ||  <max.hantke@icm.uu.se><br>
<br>
Hawk is currently v1.00 and is undergoing testing.

-------------------------------------------------------------------------------


Dependencies
------------

Hawk depends on the following packages:

* cmake
* libm
* libpng
* libjpeg
* libtiff
* zlib
* szip
* HDF5 (v1.8)
* FFTW (v3.x)
* GSL

The packages above are necessary for the compilation to complete. Additionally, HawkGUI also uses the following packages for the graphical interface:

* QT
* QTW

If you want to use the Python features in `python/plot_autocorrelation.py`, you also want to install the following Python packages:

* python (v2.7)
* numpy
* scipy
* pylab
* PyQt4

We recommend you to install all these prerequisites before continuing any further. If you're unfamiliar with installing packages, we recommend you to use Homebrew, which can be found at http://brew.sh/

You also need to compile the LibSPImage library. It can be downloaded through:

`git clone https://github.com/filipemaia/hawk.git`

If you don't have git installed, you can follow the link and click `Download ZIP`, but we recommend that you install git (available through Homebrew). Follow the instructions in `libspimage/README.md` to compile the library:

`cd libspimage`

`mkdir build`

`cd build`

`ccmake ..`

`make`

`make install`

If you want to be able to run Hawk on your graphics card, you need CUDA. You may also run Hawk using MPI, but this is not a necessity.


Installation
------------

Once all the dependencies are installed, building and installing Hawk should be completed in a few simple steps:

- Clone the repository to your local computer:

    `git clone https://github.com/filipemaia/hawk.git`

If you don't have git installed, you can follow the link and click `Download ZIP`, but we recommend that you install git (available through Homebrew).

- Create and go into a build directory:

    `cd hawk`

    `mkdir build`

    `cd build`

- Run ccmake and point it to the base directory:

    `ccmake ..`

You will see something like:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
[EMPTY CACHE]

EMPTY CACHE:

Press [enter] to edit option                              CMake Version 3.0.2
Press [c] to configure
Press [h] for help               Press [q] to quit without generating
Press [t] to toggle advanced mode (Currently Off)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

- Press `c` to configure

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
A2X                             *A2X-NOTFOUND                                                                                                       
BUILD_TESTING                   *ON                                                                                                                 
CMAKE_BUILD_TYPE                *                                                                                                                   
CMAKE_INSTALL_PREFIX            */usr/local                                                                                                         
CMAKE_OSX_ARCHITECTURES         *                                                                                                                   
CMAKE_OSX_DEPLOYMENT_TARGET     *                                                                                                                   
CMAKE_OSX_SYSROOT               */Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.9.sdk                 
DEBUG_MEM                       *OFF                                                                                                                
DMALLOC_USE                     *OFF                                                                                                                
DOUBLE_PRECISION                *OFF                                                                                                                
GSL_CONFIG                      */usr/local/bin/gsl-config                                                                                          
GSL_CONFIG_PREFER_PATH          */bin;;/bin;                                                                                                        
GSL_EXE_LINKER_FLAGS            *-Wl,-rpath,/usr/local/Cellar/gsl/1.16/lib                                                                          
HDF5_INCLUDE_DIR                */usr/local/include                                                                                                 
HDF5_LIBRARY                    */usr/local/lib/libhdf5.dylib                                                                                       
MATH_INCLUDE_DIR                */Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.9.sdk/usr/include     
MATH_LIBRARY                    */usr/lib/libm.dylib                                                                                                
QT_QMAKE_EXECUTABLE             *NOTFOUND                                                                                                           
QWT_INCLUDE_DIR                 *QWT_INCLUDE_DIR-NOTFOUND                                                                                           
QWT_LIBRARY                     *QWT_LIBRARY-NOTFOUND                                                                                               
SPIMAGE_INCLUDE_DIR             */usr/local/include                                                                                                 
SPIMAGE_LIBRARY                 */usr/local/lib64/libspimage.dylib                                                                                  
USE_MPI                         *OFF                                                                                                                

Press [enter] to edit option                              CMake Version 3.0.2
Press [c] to configure
Press [h] for help               Press [q] to quit without generating
Press [t] to toggle advanced mode (Currently Off)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Make sure that all the dependencies have been found if they're installed in non-standard paths. You can also specify `CMAKE_INSTALL_PREFIX` to set the install directory. This is done by highlighting `CMAKE_INSTALL_PREFIX`, pressing enter to modify the path, then press enter again to set it. You may press `t` to view all available options.

- If you made any changes, press `c` again to re-configure.

- If everything went well you should see a screen just like the one above and be able to press `g` to generate the Makefiles and exit.

- Now run:

    `make`

This will build things and place the result in the build directory. If the build is slow, you can specify `make -j 2` to use 2 threads.

- If you want to install, just run:

    `make install`

This will automatically install the scripts in your path.

Please check `doc/UserManual.pdf` for further installation instructions and documentation.


Help
----

There are four sources of documentation for Hawk:

* This README
* Additional documentation can be found in `doc/UserManual.pdf`
* Hawk scripts are documented in-place in the code
* You can also pass no arguments to some of the scripts to get some help


Contribute
----------

If you would like to add content to Hawk or suggest an enhancement, please do! The usual GitHub features (issues & pull requests) should suffice for these purposes. Coding standards for Hawk are documented in `doc/CodingStandards.tex`
