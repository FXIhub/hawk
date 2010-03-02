#ifndef _SUPPORT_H_
#define _SUPPORT_H

int descend_real_compare(const void * a,const  void * b);
int descend_complex_compare(const void * a,const  void * b);

/*! Returns the support given a real space image and a certain threshold  
 */
Image * get_updated_support(Image * input, real level , real radius, Options * opts);

/*! Calculate an enlarged or reduced version of the initial support
 */
Image * get_support_from_initial_image(Options *opts);

/*! Returns an initial support by calculating the patterson from an input image.
 *
 * The support is calculated from the patterson by using all the points above a certain
 * threshold. This threshold can be a fixed ratio of the maximum int, or a percentage of the
 * total area.
 */
Image * get_support_from_patterson(Image * input, Options * opts);

/*! Calculates the threshold level used for the support
 *
 *  This function returns the threshold level used for the support in the current iteration.
 *  This depends on the kind of algorithm used and user options.
 *  In the future it's possible that some support update algorithms cannot use a simple threshold
 *  method for defining the support so other methods might have to be used 
*/
real get_support_level(Image * input, real * previous_size , real radius, Log * log, Options * opts);

/*! Calculates the threshold level used for the patterson
 *
 * This function returns the threshold level used for the patterson threshold in the first iteration.
 * This depends on the kind of algorithm used and user options.
 * In the future it's possible that some patterson selection algorithms cannot use a simple threshold
 * method for defining the support so other methods might have to be used 
 */
real get_patterson_level(Image * input, real radius, Options * opts);

/*! Calculates the support and also includes fast varying regions. NOT WELL TESTED! 
 *
 * This will return a support based not only on the level, but also on the variance of the image
 * compared to the region. If it's above 3 sigma + average , it's included 
 *
 */
Image * get_filtered_support(Image * input, real level , real radius, Options * opts);

/*! Filters the intensities using the autocorrelation of the support as a low pass filter
 */
void filter_intensities_with_support(Image * amplitudes, Image * real_space_guess, Image * support, Options * opts);

#endif
