#ifndef _GEOMETRY_CONSTRAINTS_H_
#define _GEOMETRY_CONSTRAINTS_H_ 1


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#include <spimage.h>
  
  typedef struct{
    /* transforms point x -> Ax + b */
    /* A shold be symmetric */
    sp_matrix * A;
    sp_vector * b;
  }affine_transform;
  

  /* Throughout this file the image transformation is done by a rotation through the geometrical center of the image
     followed by a scaling followed by a translation 

     Control points are still given in coordinates which have origin in the top left corner of the image
  */
  
  typedef enum{RadialLineConstraint, CircleConstraint}GeometryConstraintType;
  typedef enum{DeltaX,DeltaY,Zoom,Theta}GeometryVariable;
  typedef struct{
    /* number of images in the system */
    int n_images;
    /* pointer to the images in the system */
    Image ** images;
    /* number of control points in each image */
    int * n_control_points;
    /* position of the control points in each image */
    sp_vector *** control_points;
    /* number of degrees of freedom for each image */
    int * n_variables;
    /* degrees of freedom for each image */
    GeometryVariable ** variable_type;
    /* current value for each variable including the ones that are not free */
    real * dx;
    real * dy;
    real * zoom;
    real * theta;
    GeometryConstraintType type;
  }geometry_constraints;
  
    
  void geometry_constraints_free(geometry_constraints * gc);
  /* create an affine transform from the given parameters */
  affine_transform * affine_transfrom_from_parameters(real dx,real dy,real zoom, real theta);
  /* free given affine transform */
  void affine_transform_free(affine_transform * t);
  /* apply affine transform to the input vector */
  sp_vector * apply_affine_transform(affine_transform * t, sp_vector * p);
  /* calculates the affine transform list for the given constraints */
  affine_transform ** affine_transforms_from_constraints(geometry_constraints * gc);
  /* transform control points from image coordinates to global coordinates */
  sp_vector *** control_points_to_global(geometry_constraints * gc, affine_transform ** t);

  real minimize_geometry_contraint_error(geometry_constraints * gc, real starting_angle);

  real geometry_contraint_minimizer(geometry_constraints * gc);

  sp_vector * project_point_on_line_through_origin(sp_vector * p,real theta);
#ifdef __cplusplus
}  /* extern "C" */
#endif /* __cplusplus */


#endif
