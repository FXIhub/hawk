#ifndef _RECOVER_SHANON2_H_
#define _RECOVER_SHANON2_H_


typedef struct{
  unsigned int rows;
  unsigned int cols;
  sp_list * indexes;
  sp_list * data;
}sp_sparse_matrix;


sp_sparse_matrix * matrix_to_sparse_matrix(sp_matrix * m);
sp_sparse_matrix * image_to_sparse_matrix(Image * a);
sp_sparse_matrix * sp_sparse_matrix_alloc(int rows, int cols);
void sp_sparse_matrix_free(sp_sparse_matrix * m);
void sp_sparse_matrix_set(sp_sparse_matrix * m, int row, int col, real v);
int sp_sparse_matrix_size(sp_sparse_matrix * m);
real sp_sparse_matrix_integrate(sp_sparse_matrix * m);
int sp_sparse_matrix_rows(sp_sparse_matrix * m);
int sp_sparse_matrix_cols(sp_sparse_matrix * m);
sp_sparse_matrix * sp_sparse_matrix_rotate(sp_sparse_matrix * in,SpAngle angleDef, int in_place);
sp_sparse_matrix * sp_sparse_matrix_duplicate(sp_sparse_matrix * m);
int sp_sparse_matrix_non_zero_entries(sp_sparse_matrix * m);
#endif
