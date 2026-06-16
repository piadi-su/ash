#include <X11/X.h>
#include <stdio.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

//my files
#include "xwin.h"




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

	int width = DisplayWidth(dpy, screen);

	Window win = XCreateWindow(
			dpy,
			root,
			0,
			0,
			width,
			24,
			0,
			CopyFromParent,
			InputOutput,
			CopyFromParent,
			CWOverrideRedirect,
			&attrs
			);
	
	set_dock_properties(dpy, win, width);

	XSelectInput(dpy, win, ExposureMask | KeyPressMask);

	XMapWindow(dpy, win);
	
	
	GC gc = XCreateGC(
			dpy,
			win,
			0,
			NULL
			);



	while (1)
	{
		while (XPending(dpy))
		{
			XNextEvent(dpy, &ev);

			if (ev.type == Expose)
			{
				XClearWindow(dpy, win);
				draw_bar(dpy, win, gc);
			}
		}

		sleep(1);
	}

	return 0;
}
