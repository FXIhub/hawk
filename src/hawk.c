#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

int convert_main(int argc, char ** argv);

char * version(void){
	static char buffer[9];
	int y = HAWK_VERSION/10000;
	int m = (HAWK_VERSION/100)%100;
	int d = HAWK_VERSION%100;
	sprintf(buffer,"%d.%d.%d",y,m,d);
	return buffer;
}



int main(int argc, char ** argv){
	static char help[] = 
	"usage: hawk [--version|--help] <command> [<args>]\n\
\n\
The most commonly used hawk commands are:\n\
   convert    Convert an image from a format to another\n\
   phase      Phase a diffraction pattern\n\
\n\
See 'hawk help <command>' for more information on a specific command.\n\
";

	if(argc == 1 || (argc == 2 && (strcmp(argv[1],"--help") == 0 || strcmp(argv[1],"help") == 0))){
		printf("%s\n",help);
		exit(0);
	}
	if(strcmp(argv[1],"--version") == 0|| strcmp(argv[1],"version") == 0){
		printf("hawk version %s\n",version());
		exit(0);
	}
	if(strcmp(argv[1],"help") == 0){		
		char buffer[1024];
		snprintf(buffer,1024,"hawk-%s",argv[2]);
		execlp("man", "man", buffer, (char *)NULL);
		/* If everything goes well execlp will not return */
		printf("failed to exec man: %s\n", strerror(errno));
		printf("Could not start \"man\"\n");
		exit(-1);
	}else if(strcmp(argv[1],"convert") == 0){
		convert_main(argc-1,&argv[1]);
	}else{
		printf("Unknown option: %s\n",argv[1]);
		printf("%s\n",help);
		exit(0);
	}
	return 0;
}