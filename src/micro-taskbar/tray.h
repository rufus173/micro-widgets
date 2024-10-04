#ifndef TRAY_H
#define TRAY_H
int main_tray();
int start_program(const char *executable_path);
struct tray_command {
	int opcode;
	int index;
	char executable_path[1024];
};
struct tray_response {
	int status;
	int count;
	char exectuable[1024];
};
#endif
