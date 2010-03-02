#!/usr/bin/python
import spimage
import pylab
# Read input image
img = spimage.sp_image_read('../ring/raw_ring.h5',0)
# Convolute with a 2 pixel
# standard deviation gaussian
img_blur = spimage.sp_gaussian_blur(img,2.0)
rel_diff = abs((pylab.real(img_blur.image)-
                pylab.real(img.image))
               /pylab.real(img_blur.image))
# Plot relative difference
pylab.imshow(rel_diff,vmin = 0,vmax = 0.5)
pylab.colorbar()
pylab.show()

