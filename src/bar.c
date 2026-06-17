#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xft/Xft.h>
#include <fontconfig/fontconfig.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

#include "bar.h"
#include "config.h"



XftFont *xft_font = NULL;
XftDraw *xft_draw = NULL;
XftColor xft_color;


//reder the font 
void 
init_font(Display *dpy, Window win, int screen)
{
	xft_font = XftFontOpenName(
			dpy,
			screen,
			BAR_FONT
			);

	XRenderColor xrcolor = {
		.red   = (unsigned short)(0xebdb * 257),
		.green = (unsigned short)(0xbda2 * 257),
		.blue  = (unsigned short)(0x82e3 * 257),
		.alpha = 0xffff
	};

	XftColorAllocValue(
			dpy,
			DefaultVisual(dpy, screen),
			DefaultColormap(dpy, screen),
			&xrcolor,
			&xft_color
			);

	xft_draw = XftDrawCreate(
			dpy,
			win,
			DefaultVisual(dpy, screen),
			DefaultColormap(dpy, screen)
			);
}

//free everything
void 
cleanup(Display *dpy, Window win, GC gc)
{
    if (xft_draw)
        XftDrawDestroy(xft_draw);

    if (xft_font)
        XftFontClose(dpy, xft_font);

    XftColorFree(
        dpy,
        DefaultVisual(dpy, DefaultScreen(dpy)),
        DefaultColormap(dpy, DefaultScreen(dpy)),
        &xft_color
    );

	XFreeGC(dpy, gc);

    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);
}


// set all the dock posiotion propieties
void 
set_dock_properties(Display *dpy, Window win, int width)
{
    Atom type = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", False);
    Atom dock = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DOCK", False);

    XChangeProperty(
        dpy, win,
        type,
        XA_ATOM,
        32,
        PropModeReplace,
        (unsigned char *)&dock,
        1
    );

    unsigned long strut[12] = {0};

    strut[2] = BAR_HEIGHT;        // top
    strut[8] = 0;         // start x
    strut[9] = width;     // end x

    Atom strut_atom =
        XInternAtom(dpy, "_NET_WM_STRUT_PARTIAL", False);

    XChangeProperty(
        dpy, win,
        strut_atom,
        XA_CARDINAL,
        32,
        PropModeReplace,
        (unsigned char *)strut,
        12
    );
}



//make the bar  
void 
draw_bar(Display *dpy, Window win, GC gc, BarState *s)
{
    char buf[512];

	XSetWindowBackground(dpy, win, BACKGROUND_COLOR);

	XSetForeground(dpy, gc, TEXT_COLOR);


    XClearWindow(dpy, win);
	

	snprintf(buf, sizeof(buf),
			"WS:%s | VOL:%s | IP:%s | RAM:%s | %s",
			s->workspace,
			s->volume,
			s->ipv4,
			s->ram,
			s->datetime
			);

    XftDrawStringUtf8(
        xft_draw,
        &xft_color,
        xft_font,
        10,
        18,
        (FcChar8 *)buf,
        strlen(buf)
    );

    XFlush(dpy);
}

/*============ bar modules =============*/



void 
update_datetime(BarState *s)
{
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);

    strftime(s->datetime, sizeof(s->datetime),
             "%Y-%m-%d %H:%M", tm_info);
}

















