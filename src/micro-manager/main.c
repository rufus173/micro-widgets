#include <X11/Xlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
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
	XGrabKey(display,XKeysymToKeycode(display,XStringToKeysym("F1")),Mod4Mask, DefaultRootWindow(display), True, GrabModeAsync, GrabModeAsync);
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
	int x_diff = 0;
	int y_diff = 0;
	for (;;){
		XNextEvent(display,&event); //load the next event into its struct

		//raise window
		if (event.type == KeyPress && event.xkey.subwindow != None){
			XRaiseWindow(display, event.xkey.subwindow);
		}else if (event.type == ButtonPress && event.xbutton.subwindow != None){
			printf("window picked up\n");
			XGetWindowAttributes(display,event.xbutton.subwindow, &attributes);//get selected window's attributes
			start = event.xbutton;
			printf("start cursor pos: %d,%d\n",start.x_root, start.y_root);
			printf("start window pos: %d, %d\n",attributes.x,attributes.y);
		}else if (event.type == MotionNotify && start.subwindow != None){
			//printf("window moving\n");
			x_diff = start.x_root - attributes.x; //so we drag the window from where we clicked it rather then the corner
			y_diff = start.y_root - attributes.y;
			printf("cursor (%d,%d)\n",event.xbutton.x_root,event.xbutton.y_root);
			XMoveWindow(display, start.subwindow, event.xbutton.x_root-x_diff, event.xbutton.y_root-y_diff);
		}else if (event.type == ButtonRelease){
			printf("window dropped\n");
			start.subwindow = None;
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
