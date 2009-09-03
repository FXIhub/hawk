#include "AllTests.h"
#include "geometry_constraints.h"

void test_difference_project_point(CuTest* tc){
  sp_vector * p = sp_vector_alloc(2);
  sp_vector_set(p,0,0);
  sp_vector_set(p,1,1);
  sp_vector * proj;
  proj = project_point_on_line_through_origin(p,0);
  CuAssertDblEquals(tc,sp_vector_get(proj,0),0,REAL_EPSILON);
  CuAssertDblEquals(tc,sp_vector_get(proj,1),0,REAL_EPSILON);
  sp_vector_free(proj);
  proj = project_point_on_line_through_origin(p,M_PI/2.0);
  CuAssertDblEquals(tc,sp_vector_get(proj,0),0,REAL_EPSILON);
  CuAssertDblEquals(tc,sp_vector_get(proj,1),1,REAL_EPSILON);
  sp_vector_free(proj);
  proj = project_point_on_line_through_origin(p,M_PI/4.0);
  CuAssertDblEquals(tc,sp_vector_get(proj,0),1.0/2,REAL_EPSILON);
  CuAssertDblEquals(tc,sp_vector_get(proj,1),1.0/2,REAL_EPSILON);
  sp_vector_free(proj);
  proj = project_point_on_line_through_origin(p,3*M_PI/4.0);
  CuAssertDblEquals(tc,sp_vector_get(proj,0),-1.0/2,REAL_EPSILON);
  CuAssertDblEquals(tc,sp_vector_get(proj,1),1.0/2,REAL_EPSILON);
  sp_vector_free(proj);
}

void test_affine_transforms(CuTest* tc){
  affine_transform * t =  affine_transfrom_from_parameters(0,0,1,0);
  sp_vector * p = sp_vector_alloc(2);
  sp_vector_set(p,0,0);
  sp_vector_set(p,1,1);
  sp_vector * trans;
  trans = apply_affine_transform(t,p);
  CuAssertDblEquals(tc,sp_vector_get(trans,0),0,REAL_EPSILON);
  CuAssertDblEquals(tc,sp_vector_get(trans,1),1,REAL_EPSILON);
  sp_vector_free(trans);
  affine_transform_free(t);

  t =  affine_transfrom_from_parameters(0,0,1,M_PI);
  trans = apply_affine_transform(t,p);
  CuAssertDblEquals(tc,sp_vector_get(trans,0),0,REAL_EPSILON);
  CuAssertDblEquals(tc,sp_vector_get(trans,1),-1,REAL_EPSILON);
  sp_vector_free(trans);
  affine_transform_free(t);

  t =  affine_transfrom_from_parameters(0,0,2,M_PI);
  trans = apply_affine_transform(t,p);
  CuAssertDblEquals(tc,sp_vector_get(trans,0),0,sqrt(REAL_EPSILON));
  CuAssertDblEquals(tc,sp_vector_get(trans,1),-2,sqrt(REAL_EPSILON));
  sp_vector_free(trans);
  affine_transform_free(t);

  t =  affine_transfrom_from_parameters(1,0,2,M_PI);
  trans = apply_affine_transform(t,p);
  CuAssertDblEquals(tc,sp_vector_get(trans,0),1,sqrt(REAL_EPSILON));
  CuAssertDblEquals(tc,sp_vector_get(trans,1),-2,sqrt(REAL_EPSILON));
  sp_vector_free(trans);
  affine_transform_free(t);

  t =  affine_transfrom_from_parameters(1,1,2,M_PI);
  trans = apply_affine_transform(t,p);
  CuAssertDblEquals(tc,sp_vector_get(trans,0),1,sqrt(REAL_EPSILON));
  CuAssertDblEquals(tc,sp_vector_get(trans,1),-1,sqrt(REAL_EPSILON));
  sp_vector_free(trans);
  affine_transform_free(t);

}

geometry_constraints * create_constraints(){
  Image * a = sp_image_alloc(4,4,1);
  Image * b = sp_image_alloc(4,4,1);
  geometry_constraints * gc = sp_malloc(sizeof(geometry_constraints));
  gc->n_images = 2;
  gc->images = sp_malloc(sizeof(Image * )*gc->n_images);
  gc->images[0] = a;
  gc->images[1] = b;
  gc->n_control_points = sp_malloc(sizeof(int * )*gc->n_images);
  gc->n_control_points[0] = 1;
  gc->n_control_points[1] = 1;
  gc->control_points = sp_malloc(sizeof(sp_vector ** )*gc->n_images);
  gc->control_points[0] = sp_malloc(sizeof(sp_vector * )*gc->n_control_points[0]);
  gc->control_points[0][0] = sp_vector_alloc(2);
  /* control at 1,1 of image a */
  sp_vector_set(gc->control_points[0][0],0,1);
  sp_vector_set(gc->control_points[0][0],1,1);

  gc->control_points[1] = sp_malloc(sizeof(sp_vector * )*gc->n_control_points[1]);
  gc->control_points[1][0] = sp_vector_alloc(2);
  /* control at 3,3 of image a */
  sp_vector_set(gc->control_points[1][0],0,3);
  sp_vector_set(gc->control_points[1][0],1,3);

  gc->n_variables = sp_malloc(sizeof(int * )*gc->n_images);
  gc->n_variables[0] = 0;
  gc->n_variables[1] = 0;
  gc->variable_type = sp_malloc(sizeof(GeometryVariable * )*gc->n_images);
  gc->variable_type[0] = NULL;
  gc->variable_type[1] = NULL;
  
  /* No transformation on the images */
  gc->dx = sp_malloc(sizeof(real )*gc->n_images);
  gc->dx[0] = 0;
  gc->dx[1] = 0;
  gc->dy = sp_malloc(sizeof(real )*gc->n_images);
  gc->dy[0] = 0;
  gc->dy[1] = 0;
  gc->zoom = sp_malloc(sizeof(real )*gc->n_images);
  gc->zoom[0] = 1;
  gc->zoom[1] = 1;
  gc->theta = sp_malloc(sizeof(real )*gc->n_images);
  gc->theta[0] = 0;
  gc->theta[1] = 0;
  gc->type = RadialLineConstraint;
  return gc;
}

void test_control_points_to_global(CuTest* tc){
  real tol = sqrt(REAL_EPSILON);
  geometry_constraints * gc = create_constraints();
  affine_transform **t = affine_transforms_from_constraints(gc);
  sp_vector *** global = control_points_to_global(gc,t);
  /* the 1,1 control point should now be at -1,-1 */
  CuAssertDblEquals(tc,sp_vector_get(global[0][0],0),-1,tol);
  CuAssertDblEquals(tc,sp_vector_get(global[0][0],1),-1,tol);
  for(int i =0 ;i<gc->n_images;i++){
    affine_transform_free(t[i]);
  }
  sp_free(t);
  geometry_constraints_free(gc);
}

void test_basic_minimization(CuTest* tc){
  real tol = sqrt(REAL_EPSILON);
  geometry_constraints * gc = create_constraints();
  minimize_geometry_contraint_error(gc);
  geometry_constraints_free(gc);
}

CuSuite* geometry_get_suite(void)

{
  CuSuite* suite = CuSuiteNew();
  SUITE_ADD_TEST(suite, test_difference_project_point);
  SUITE_ADD_TEST(suite, test_affine_transforms);
  SUITE_ADD_TEST(suite, test_control_points_to_global);
  SUITE_ADD_TEST(suite, test_basic_minimization);
  return suite;

}

