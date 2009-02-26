typedef struct {
  int cross_correlate;
  int verbose;
  char output[1024];
  float rescale;
  int average;
  int std_dev;
  int rotate;
  char subtract[1024];
  char input[1024];
}Options;

Options parse_options(int argc, char ** argv);
void set_defaults(Options * opt);
