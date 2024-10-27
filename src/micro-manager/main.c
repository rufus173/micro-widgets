#include <X11/Xlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
static volatile short continue_running = 1;
int clamp_min(int target, int min);
int main(int argc, char **argv){
	//setup some bits
	Display *display; //assume only one screen
	XWindowAttributes attributes;
	XButtonEvent start;
	XEvent event;

	//connect to xserver
	display = XOpenDisplay(NULL);
	if (display == NULL){
		fprintf(stderr, "Could not open x display\n");
		return EXIT_FAILURE;
	}

	//binding keys
	//grabbing windows                                         f1   + windows
	XGrabKey(display,XKeysymToKeycode(display,XStringToKeysym("m")),Mod4Mask, DefaultRootWindow(display), True, GrabModeAsync, GrabModeAsync);
	//mouse
	XGrabButton(display,
		1, Mod4Mask, //left click + windows
		DefaultRootWindow(display), True, 
		ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
		GrabModeAsync, GrabModeAsync, None, None
	);
	XGrabButton(display,
		3, Mod4Mask, //right click + windows
		DefaultRootWindow(display), True, 
		ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
		GrabModeAsync, GrabModeAsync, None, None
	);

	//mainloop
	start.subwindow = None;
	//the position of the cursor in relation to the window origin on window pickup
	int x_window_cursor_diff = 0;
	int y_window_cursor_diff = 0;
	int start_cursor_x = 0;
	int start_cursor_y = 0;
	int start_window_x = 0;
	int start_window_y = 0;
	int mouse_button = 0;
	for (;continue_running == 1;){
		XNextEvent(display,&event); //load the next event into its struct

		//======================== keyboard events ===========================
		if (event.type == KeyRelease){
			int key = event.xkey.keycode;
			if (key == XKeysymToKeycode(display,XStringToKeysym("m"))){
				continue_running = 0;
				printf("terminating...\n");
				break;
			}
		}
		//======================== mouse events ==============================
		// --------------- start of click --------------
		if (event.type == ButtonPress && event.xbutton.subwindow != None){
			mouse_button = event.xbutton.button;
			XGetWindowAttributes(display,event.xbutton.subwindow, &attributes);//get selected window's attributes
			XRaiseWindow(display, event.xkey.subwindow);
			start = event.xbutton;
			printf("====== start of click event ======\n");
			switch(mouse_button){
				case 1:
					printf("event type: move\n");
				break;
				case 3:
					printf("event type: resize\n");
				break;
			}
			start_cursor_x = start.x_root; start_cursor_y = start.y_root;
			start_cursor_y = start.x_root; start_window_y = start.y_root;
			start_window_x = attributes.x;
			start_window_y = attributes.y;

			//so we drag the window from where we clicked it rather then the corner
			x_window_cursor_diff = start.x_root - attributes.x; 
			y_window_cursor_diff = start.y_root - attributes.y;
			
			printf("start cursor pos: %d,%d\n",start.x_root, start.y_root);
			printf("start window pos: %d,%d\n",attributes.x,attributes.y);
		//--------------- dragging the window ---------------
		}else if (event.type == MotionNotify && start.subwindow != None && mouse_button == 1){
			//printf("window moving\n");
			unsigned int new_x = clamp_min(event.xbutton.x-x_window_cursor_diff,0);
			unsigned int new_y = clamp_min(event.xbutton.y-y_window_cursor_diff,0);
			printf("moving win to (%u,%u)\n",new_x,new_y);
			XMoveWindow(display, start.subwindow, new_x, new_y);
		//-------------- resizing the window -----------------
		}else if(event.type == MotionNotify && start.subwindow != None && mouse_button == 3){
			unsigned int new_width = clamp_min(event.xbutton.x_root - start_window_x,1);
			unsigned int new_height = clamp_min(event.xbutton.y_root - start_window_y,1);
			printf("resize to %dx%d\n",new_width,new_height);
			XResizeWindow(display, start.subwindow, new_width, new_height);
			
		//-------------- button released --------------
		}else if (event.type == ButtonRelease){
			printf("====== end of click event ======\n");
			start.subwindow = None;
			mouse_button = 0;
		}
	}

	//clean
	XCloseDisplay(display);

	return 0;
}
int clamp_min(int target, int min){
	if (target < min){
		return min;
	}
	return target;
}
