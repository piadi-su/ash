#include <X11/X.h>
#include <stdio.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

//my files
#include "bar.h"
#include "config.h"



int main(void)
{
	
	//var that stores all xevents
	XEvent ev;
	
	//connect to x11 
	Display *dpy = XOpenDisplay(NULL);
	
	if(dpy == NULL)
	{
		fprintf(stderr, "can not open the disply!\n");
		return 1;
	}
	

	// get the display like DP-2 ecc
	int screen = DefaultScreen(dpy);
	
	//create root window/desktop
	Window root = RootWindow(dpy, screen);

	// per non fare coprire da altre finestre
	XSetWindowAttributes attrs = {0};
	

	//get the width of the screen
	int width = DisplayWidth(dpy, screen);

	//create the window
	Window win = XCreateWindow(
			dpy,
			root,
			0,
			0,
			width,
			BAR_HEIGHT,
			0,
			CopyFromParent,
			InputOutput,
			CopyFromParent,
			CWOverrideRedirect,
			&attrs
			);
	
	//make it not in tyle mode reserv the pixel size 
	set_dock_properties(dpy, win, width);
	
	// say what kind of input the bar can recive
	XSelectInput(dpy, win, ExposureMask | KeyPressMask);
	
	//set in wait the window and make it visible 
	XMapWindow(dpy, win);
	
	//create the thing to draw stuff on bar
	GC gc = XCreateGC(
			dpy,
			win,
			0,
			NULL
			);


	//bar cycle
	while (1)
	{
		while (XPending(dpy))
		{
			XNextEvent(dpy, &ev);

			if (ev.type == Expose)
				draw_bar(dpy, win, gc, screen);
		}

		draw_bar(dpy, win, gc, screen);
		usleep(1000000);
	}

	return 0;
}
