#ifndef XWIN_H
#define XWIN_H

#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>
#include <fontconfig/fontconfig.h>

void draw_bar(Display *dpy, Window win, GC gc);

void init_font(Display *dpy, Window win, int screen);

void cleanup(Display *dpy, Window win, GC gc);

void set_dock_properties(Display *dpy, Window win, int width);



extern XftDraw *xft_draw;
extern XftFont *xft_font;
extern XftColor xft_color;

#endif
