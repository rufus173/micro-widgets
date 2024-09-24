#ifndef DEBUG_H
#define DEBUG_H
#include <stdio.h>
class debug_class {
	private:
	char file[1024];
	bool debug_enabled = true;
	public:
	debug_class(const char *filename){
		snprintf(file,1024,"%s",filename);
	}
	void operator<<(const char *buffer){
		if (debug_enabled) printf("DEBUG in file [%s]: %s\n",file,buffer);
	}
	void operator<(const char *buffer){
		if (debug_enabled) fprintf(stderr,"ERROR in file [%s]: %s\n",file,buffer);
	}
};
#endif
