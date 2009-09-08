#ifndef _GEOMETRY_CONSTRAINTS_H_
#define _GEOMETRY_CONSTRAINTS_H_ 1


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#include <spimage.h>
  
  /* Throughout this file the image transformation is done by a rotation through the geometrical center of the image
     followed by a scaling followed by a translation 

     Control points are still given in coordinates which have origin in the top left corner of the image
  */

  typedef struct{
    /* A behaves like a QTransform */
    sp_matrix * A;
  }affine_transform;
  
  typedef enum{RadialLineConstraint, CircleConstraint}GeometryConstraintType;
  /* NumberOfGeometryVariablesTypes should always come last in the enum! */ 
  typedef enum{DeltaX=0,DeltaY,Zoom,Alpha,Theta,NumberOfGeometryVariablesTypes}GeometryVariableType;

  typedef struct{
    const Image * image;
    /* 
       this is an array that contains the physical position of the detector
       pos[0] corresponds to DeltaX, pos[1] to DeltaY and so on
     */
    real pos[NumberOfGeometryVariablesTypes];
  }positioned_image;

  typedef struct{
    positioned_image * parent;
    real pos[2];
  }control_point;

  typedef struct{
    int n_points;
    control_point * points;
    GeometryConstraintType type;
    /* best_fit represents the angle of the line that best fits the points
     for the RadialLineConstraint
    
     best_fit the radius of the circle line that best fits the points
     for the CircleConstraint
    */
    real best_fit;
    /* error for each control point */
    real * error;
  }geometric_constraint;

  typedef struct{
    GeometryVariableType type;
    positioned_image * parent;
  }geometry_variable;
  
  typedef struct{
    int n_constraints;
    geometric_constraint * constraints;
    int n_variables; 
    geometry_variable * variables;
  }geometrically_constrained_system;


  geometrically_constrained_system * geometrically_constrained_system_alloc();
  void geometrically_constrained_system_add_constraint(geometrically_constrained_system * gc, geometric_constraint c);
  void geometrically_constrained_system_add_variable(geometrically_constrained_system * gc, geometry_variable v);
  control_point create_control_point(positioned_image * parent, real x, real y);
  geometry_variable create_geometry_variable(positioned_image * parent, GeometryVariableType type);
  positioned_image * create_positioned_image(const Image * a);
  void set_image_position(positioned_image *  image ,GeometryVariableType t, real v);
  affine_transform * affine_transfrom_from_positioned_image(positioned_image * image);
  sp_vector ** control_point_list_to_global(control_point * points, int n);
  geometric_constraint geometric_constraint_init(GeometryConstraintType type, real best_guess);
  void geometric_constraint_add_point(geometric_constraint * c,control_point p);

  /* create an affine transform from the given parameters */
  affine_transform * affine_transfrom_from_parameters(real dx,real dy,real zoom, real alpha, real theta);
  /* free given affine transform */
  void affine_transform_free(affine_transform * t);
  /* apply affine transform to the input vector */
  sp_vector * apply_affine_transform(affine_transform * t, sp_vector * p);

  int minimize_geometry_contraint_error(geometrically_constrained_system * gc);

  int geometry_contraint_minimizer(geometrically_constrained_system * gc);

  sp_vector * project_point_on_line_through_origin(sp_vector * p,real theta);
#ifdef __cplusplus
}  /* extern "C" */
#endif /* __cplusplus */


#endif
