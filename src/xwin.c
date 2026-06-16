#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

#include "xwin.h"

void 
draw_bar(Display *dpy, Window win, GC gc)
{
    XDrawString(
        dpy,
        win,
        gc,
        10,
        18,
        "ciao finestra per test",
        13
    );

    XFlush(dpy);
}


void set_dock_properties(Display *dpy, Window win, int width)
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

    strut[2] = 24;        // top
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
