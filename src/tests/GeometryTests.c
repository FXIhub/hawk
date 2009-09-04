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


void test_control_points_to_global(CuTest* tc){
  real tol = sqrt(REAL_EPSILON);

  Image * a = sp_image_alloc(4,4,1);
  Image * b = sp_image_alloc(4,4,1);
  positioned_image * p_a =  create_positioned_image(a);
  positioned_image * p_b =  create_positioned_image(b);
  geometrically_constrained_system * gc = geometrically_constrained_system_alloc();
  geometric_constraint c = geometric_constraint_init(RadialLineConstraint,0);
  /* control at 1,1 of image a */  
  geometric_constraint_add_point(&c,create_control_point(p_a,1,1)); 
  /* control at 3,3 of image b */
  geometric_constraint_add_point(&c,create_control_point(p_b,3,3)); 

  geometrically_constrained_system_add_constraint(gc,c);


  sp_vector ** global = control_point_list_to_global(gc->constraints[0].points,gc->constraints[0].n_points);
  /* the 1,1 control point should now be at -1,-1 */
  CuAssertDblEquals(tc,sp_vector_get(global[0],0),-1,tol);
  CuAssertDblEquals(tc,sp_vector_get(global[0],1),-1,tol);
  for(int i = 0;i<gc->constraints[0].n_points;i++){
    sp_vector_free(global[i]);
  }
}

void test_basic_minimization(CuTest* tc){
  real tol = sqrt(REAL_EPSILON);

  Image * a = sp_image_alloc(4,4,1);
  Image * b = sp_image_alloc(4,4,1);
  positioned_image * p_a =  create_positioned_image(a);
  positioned_image * p_b =  create_positioned_image(b);
  geometrically_constrained_system * gc = geometrically_constrained_system_alloc();
  geometric_constraint c = geometric_constraint_init(RadialLineConstraint,0);
  /* control at 1,1 of image a */  
  geometric_constraint_add_point(&c,create_control_point(p_a,1,1)); 
  /* control at 3,3 of image b */
  geometric_constraint_add_point(&c,create_control_point(p_b,3,3)); 

  geometrically_constrained_system_add_constraint(gc,c);
  geometry_contraint_minimizer(gc);
  real angle = gc->constraints[0].best_fit;
  CuAssertDblEquals(tc,tan(angle),tan(M_PI/4.0),tol);
}

void test_medium1_minimization(CuTest* tc){
  real tol = sqrt(REAL_EPSILON);

  Image * a = sp_image_alloc(4,4,1);
  Image * b = sp_image_alloc(4,4,1);
  positioned_image * p_a =  create_positioned_image(a);
  positioned_image * p_b =  create_positioned_image(b);
  geometrically_constrained_system * gc = geometrically_constrained_system_alloc();
  geometric_constraint c = geometric_constraint_init(RadialLineConstraint,0);
  /* control at 0,2 of image a */  
  geometric_constraint_add_point(&c,create_control_point(p_a,0,2)); 
  geometric_constraint_add_point(&c,create_control_point(p_a,1,2)); 
  geometric_constraint_add_point(&c,create_control_point(p_b,3,3)); 

  geometrically_constrained_system_add_variable(gc,create_geometry_variable(p_a,Theta));
  geometrically_constrained_system_add_constraint(gc,c);

  
  geometry_contraint_minimizer(gc);
  real angle = gc->constraints[0].best_fit;
  CuAssertDblEquals(tc,tan(angle),tan(M_PI/4.0),tol);
}


void test_medium2_minimization(CuTest* tc){
  real tol = sqrt(REAL_EPSILON);

  Image * a = sp_image_alloc(4,4,1);
  Image * b = sp_image_alloc(4,4,1);
  positioned_image * p_a =  create_positioned_image(a);
  positioned_image * p_b =  create_positioned_image(b);
  geometrically_constrained_system * gc = geometrically_constrained_system_alloc();
  geometric_constraint c = geometric_constraint_init(RadialLineConstraint,0);

  geometric_constraint_add_point(&c,create_control_point(p_a,0,1)); 
  geometric_constraint_add_point(&c,create_control_point(p_a,1,1)); 
  geometric_constraint_add_point(&c,create_control_point(p_b,3,3)); 

  geometrically_constrained_system_add_variable(gc,create_geometry_variable(p_a,Theta));
  geometrically_constrained_system_add_variable(gc,create_geometry_variable(p_a,DeltaX));
  geometrically_constrained_system_add_constraint(gc,c);

  
  geometry_contraint_minimizer(gc);
  real angle = gc->constraints[0].best_fit;
  CuAssertDblEquals(tc,tan(angle),tan(M_PI/4.0),tol);
}


void test_hard1_minimization(CuTest* tc){
  real tol = sqrt(REAL_EPSILON);

  Image * a = sp_image_alloc(4,4,1);
  Image * b = sp_image_alloc(4,4,1);
  positioned_image * p_a =  create_positioned_image(a);
  positioned_image * p_b =  create_positioned_image(b);
  geometrically_constrained_system * gc = geometrically_constrained_system_alloc();
  geometric_constraint c = geometric_constraint_init(RadialLineConstraint,0);

  geometric_constraint_add_point(&c,create_control_point(p_a,1,0)); 
  geometric_constraint_add_point(&c,create_control_point(p_a,2,1)); 
  geometric_constraint_add_point(&c,create_control_point(p_b,3,3)); 
  geometrically_constrained_system_add_constraint(gc,c);


  c = geometric_constraint_init(RadialLineConstraint,0);

  geometric_constraint_add_point(&c,create_control_point(p_a,1,4)); 
  geometric_constraint_add_point(&c,create_control_point(p_a,2,3)); 
  geometric_constraint_add_point(&c,create_control_point(p_b,3,1)); 
  geometrically_constrained_system_add_constraint(gc,c);

  //  geometrically_constrained_system_add_variable(gc,create_geometry_variable(p_a,Theta));
  geometrically_constrained_system_add_variable(gc,create_geometry_variable(p_a,DeltaX));

  
  geometry_contraint_minimizer(gc);
  real angle = gc->constraints[0].best_fit;
  CuAssertDblEquals(tc,tan(angle),tan(M_PI/4.0),tol);
}

CuSuite* geometry_get_suite(void)

{
  CuSuite* suite = CuSuiteNew();
  SUITE_ADD_TEST(suite, test_difference_project_point);
  SUITE_ADD_TEST(suite, test_affine_transforms);
  SUITE_ADD_TEST(suite, test_control_points_to_global);
  SUITE_ADD_TEST(suite, test_basic_minimization);
  SUITE_ADD_TEST(suite, test_medium1_minimization);
  SUITE_ADD_TEST(suite, test_medium2_minimization);
  SUITE_ADD_TEST(suite, test_hard1_minimization);
  return suite;

}

