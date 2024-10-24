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
		perror("XOpenDisplay");
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
	for (;;){
		XNextEvent(display,&event); //load the next event into its struct

		//raise window
		if (ev.type == KeyPress && event.xkey.subwindow != None){
			XRaiseWindow(display, event.xkey.subwindow);
		}else if (event.type == ButtonPress && event.xbutton.subwindow != None){
			XGetWindowAttributes(display,event.xbutton.subwindow, &attributes);//get selected window's attributes
			start = event.xbutton;
		}else if (event.type == MotionNotify && start.subwindow != None){
			int x_diff = event.xbutton.x_root - start.x_root;
			int y_diff = event.xbutton.y_root - start.y_root;
			XMoveResizeWindow();
		}else if (event.type == ButtonRelease){
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
