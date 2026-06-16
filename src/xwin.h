#ifndef XWIN_H
#define XWIN_H

#include <X11/Xlib.h>

void draw_bar(Display *dpy, Window win, GC gc);
void set_dock_properties(Display *dpy, Window win, int width);

#endif
