#include <ctype.h>
#include <spimage.h>
#include <tiffio.h>
#include <png.h>

typedef enum{TypeNone=0,TypeTIFF=1,TypePNG=2}FileType;

FileType get_file_type(char * filename){
  int len = strlen(filename);
  char * extensions[] = {".tif",".tiff",".png",0};
  FileType type[] = {TypeTIFF,TypeTIFF,TypePNG};
  char buffer[1024];
  for(int i = 0;i<len;i++){
    buffer[i] = tolower(filename[i]);
  }
  buffer[len] = 0;
  for(int i = 0;extensions[i];i++){
    if(strstr(buffer,extensions[i]) == (char *)(&buffer[len-strlen(extensions[i])])){
      return type[i];
    }
  }
  return TypeNone;
}

void read_tiff_metadata(char * filename,FILE * out){
  TIFF * tif; 
  tif = TIFFOpen(filename, "r");
  if (tif) {
    int dircount = 0;
    do {
      dircount++;
      TIFFPrintDirectory(tif, out,TIFFPRINT_NONE);
    } while (TIFFReadDirectory(tif));
    TIFFClose(tif);
  }
}


void read_png_metadata(char * filename,FILE * out){
  FILE *fp = fopen(filename, "rb");
  png_structp png_ptr = png_create_read_struct
    (PNG_LIBPNG_VER_STRING, (png_voidp)NULL,
     NULL,NULL);
  png_infop info_ptr = png_create_info_struct(png_ptr);
  png_init_io(png_ptr, fp);
  png_read_info(png_ptr, info_ptr);
  /*  png_get_IHDR(png_ptr, info_ptr, &width, &height,
	       &bit_depth, &color_type, &interlace_type,
	       &compression_type, &filter_method);*/
  png_textp text;
  int num_text;
  if( png_get_text( png_ptr, info_ptr, &text, &num_text ) ){
    for(int i=0 ; i<num_text ; i++ ){
      fprintf(out, "%s: %s\n", text[i].key, text[i].text);
    }
  }  
}

int main(int argc, char ** argv){
  if(argc < 2){
    printf("Usage: %s <file> [output file]\n",argv[0]);
    exit(-1);
  }
  char * filename = argv[1];
  FileType type = get_file_type(filename);
  if(type == TypeNone){
    fprintf(stderr,"File type not recongnized!\n");
    exit(-1);
  }
  FILE * output = stdout;
  if(argc == 3){
    output = fopen(argv[2],"w");
  }
  if(type == TypeTIFF){
    read_tiff_metadata(filename,output);
  }else if(type == TypePNG){
    read_png_metadata(filename,output);
  }
  return 0;
}
