Hawk - Image Reconstruction
===========================

Hawk is a collection of computer programs which aim at reconstructing objects from oversampled diffraction data.

<br>
Principle Authors:<br>
Filipe Maia     ||  <filipe.c.maia@gmail.com><br>
Tomas Ekeberg   ||  <ekeberg@xray.bmc.uu.se><br>
Max Hantke      ||  <max.hantke@icm.uu.se><br>
Jonas Sellberg  ||  <sellberg@xray.bmc.uu.se><br>
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
* QWT

If you want to use the Python features in `python/plot_autocorrelation.py`, you also want to install the following Python packages:

* python (v2.7)
* numpy
* scipy
* pylab
* PyQt4

We recommend you to install all these prerequisites before continuing any further. If you're unfamiliar with installing packages, we recommend you to use Homebrew, which can be found at http://brew.sh/

You also need to compile the `libspimage` library. It can be downloaded through:

`git clone https://github.com/FXIhub/libspimage.git`

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
QT_QMAKE_EXECUTABLE             */usr/local/bin/qmake                                                                                               
QWT_INCLUDE_DIR                 */usr/local/lib/qwt.framework/Headers                                                                               
QWT_LIBRARY                     */usr/local/lib/qwt.framework                                                                                       
SPIMAGE_INCLUDE_DIR             */usr/local/include                                                                                                 
SPIMAGE_LIBRARY                 */usr/local/lib64/libspimage.dylib                                                                                  
USE_MPI                         *OFF                                                                                                                

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

This will automatically install the scripts in your path. You may have to obtain administrative permissions by typing `sudo make install` for the install to successfully complete.

Please check `doc/UserManual.pdf` for further installation instructions and documentation.


Help
----

There are five sources of documentation for Hawk:

* This README
* Additional documentation can be found in `doc/UserManual.pdf`
* Hawk script are documented in-place in the source code
* HawkGUI has automatic help pop-ups when hovering over options
* You can also pass no arguments to some of the scripts to get some help


Tutorial: HawkGUI
-------------------------------

Below, we'll go through two examples of how to use HawkGUI to reconstruct and view the real space image of an object from the diffracted intensities.

###(1) Ring###

As a first example we'll use the diffraction image of a ring. This file is provided in `examples/ring/ring.h5` in the HDF5 format. In the same folder, there is a configuration file for the image reconstruction called `uwrapc.conf` that we need for the image reconstruction and a gzipped file of the raw ring that we don't have to care about. Go to the folder `examples/ring/` or copy it to a location where you want to run the phasing algorithm, then start HawkGUI by simply typing `HawkGUI`, which opens the program and should look something like this:

![HawkGUI screen](https://raw.github.com/filipemaia/hawk/master/doc/Images/README/HawkGUI-init.png)

You see an expandable tree of various phasing options to the upper left, two (empty) image slots to the upper right, an empty graph in the lower right, and a few buttons for basic operations in the lower left. Start with clicking `Load` and open the configuration file `uwrapc.conf` inside the copied folder. This will set all the parameters needed to start the image reconstruction. You may expand all the options to the left to see what parameters you are using, which should look identical to:

![HawkGUI ring initial](https://raw.github.com/filipemaia/hawk/master/doc/Images/README/HawkGUI-ring-init.png)

If you hover over the parameter keywords and sometimes their set values, you will see a pop-up help that explains what they do. Now click `Run` to start the iterative phasing algorithm. The image reconustrction should run for 10000 iterations (specified by the configuration file) and look similar to this after finishing:

![HawkGUI ring running](https://raw.github.com/filipemaia/hawk/master/doc/Images/README/HawkGUI-ring-run.png)

The left image shows the real space image and the right image shows the support in red. The support is the area in which the particle is contained and is related to the fact that the diffraction image is oversampled and is a necessary constraint to uniquely determine the phases of the scattering amplitudes (hence, the real space image). You may zoom in on the images by scrolling and translate them by moving them with the mouse while you left-click. As you see, the support image looks different from the real space image, and does not seem to overlap with the ring at all. This is because the FFT algorithms use shifted images where the center of the image is located at the corners. This can be fixed by hovering over the lower part of the image slot, which reveals a hidden image menu, and clicking `Shift Image`, which is the middle button in the lower row. The screen should now look like this:

![HawkGUI ring final](https://raw.github.com/filipemaia/hawk/master/doc/Images/README/HawkGUI-ring-final.png)

You can now click on one of the images to mark them and use the up-arrow and down-arrow keys to scroll through the various iteration steps saved to file. You may also switch the real space and support images in the drop-down menu above the image slots to the Fourier space image, the real space autocorrelation, as well as the initial image and support. If you want to see how the initial diffraction intensities look like, you can press `Load Image`, which is the left-most button in the upper row of the hidden image menu and select `ring.h5`. You may have to press `Shift Image` when you change between the various images to make sure that the center is located at the center of the image (and not at the corners).

Last, you can take a look at the graph in the lower left part of the screen, which shows how various parameters change as a function of the number of iterations. As default, the support size (measured in percent of the image size) and the real space and Fourier space errors are shown. The real space error is defined as the summed amplitudes of the real space image outside of the support. The Fourier space error is defined as the summed difference in magnitude between the (square root of the) measured diffraction intensities and the reconstructed Fourier amplitudes. You can zoom by marking a certain area of the graph and unzoom by right-clicking in the graph window.

Further information about algorithms, parameters, as well as a more detailed example of the ring reconstruction can be found in `doc/UserManual.pdf`

###(2) Mimivirus###

The next example is closer to the reality of single-particle imaging, but also harder to reconstruct. It is a single snapshot of a Mimivirus, measured by Marvin Seibert, Tomas Ekeberg, Filipe Maia, et al. at the AMO endstation at LCLS and published in Nature in 2011, available at: http://dx.doi.org/10.1038/nature09748

The data was published in the Coherent X-ray Imaging Data Bank (CXIDB) and is available to download from: http://www.cxidb.org/id-1.html

Download `cxidb-1.cxi` that includes the diffraction image saved in the CXIDB file format, which is a hierarchical structure using the HDF5 format. For more information about the file format, see: https://github.com/FilipeMaia/CXI/raw/master/cxi_file_format.pdf

Download `mimi_a.conf` that contains the configuration file for the image reconstruction. Copy `mimi_a.conf` to `uwrapc.conf` and change the `intensities_file` parameter to the path of the `cxidb-1.cxi` file (either absolute path or relative path with respect to `work_directory`). `Load` the configuration file `uwrapc.conf` and press `Run`. After the program has finished and you've zoomed and clicked `Shift Image` for the support it should look similar to this:

![HawkGUI ring final](https://raw.github.com/filipemaia/hawk/master/doc/Images/README/HawkGUI-mimi1-final.png)

You see that the real space image has converged towards the shape and size of a Mimivirus with icosahedral facets (that looks hexagonal in a 2D projection). A good indication that the image reconstruction has converged is given by the Fourier space error, which drops slightly after approximately 3000 iterations without the real space error increasing although the support size is lowered. This means that the real space image is localized and still reproduces the measured diffraction intensities in Fourier space. If the image reconstruction succeeds, the image should be comparable to the "Unconstrained" reconstruction in Fig. 2f in the published article. The hollow center of the virus is due to missing diffraction data at small scattering angles and can be corrected for by fitting low-resolution modes to a spherical or icosahedral profile.

You may also run the image reconstruction without the graphical interface. This is done by running `uwrapc` in the folder where the `uwrapc.conf` configuration file is present.


Contribute
----------

If you would like to add content to Hawk or suggest an enhancement, please do! The usual GitHub features (issues & pull requests) should suffice for these purposes. Coding standards for Hawk are documented in `doc/CodingStandards.tex`
