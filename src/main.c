#include <X11/X.h>
#include <stdio.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <signal.h>


//my files
#include "bar.h"
#include "config.h"



volatile sig_atomic_t running = 1;

void handle_sigint(int sig);


int main(void)
{
	
	//var that stores all xevents
	XEvent ev;
	
	//initialize the bar state struct
	BarState s = {0};

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

	//PER VALGRIND
	signal(SIGINT, handle_sigint); 

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
	
	//inizialize the font 
	init_font(dpy, win, screen);

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
	while (running)
	{
		while (XPending(dpy))
		{
			XNextEvent(dpy, &ev);

			if (ev.type == Expose)
				draw_bar(dpy, win,gc , &s);
		}

		draw_bar(dpy, win, gc, &s);
		usleep(1000000);
	}


	cleanup(dpy, win, gc);
	return 0;
}


void 
handle_sigint(int sig)
{
    (void)sig;
    running = 0;
}

