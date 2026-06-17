#ifndef XWIN_H
#define XWIN_H

#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>
#include <fontconfig/fontconfig.h>

//struct that keeps the bar state
typedef struct {
    char workspace[128];
    char volume[64];
    char ipv4[64];
    char ram[64];
    char datetime[128];
} BarState;


//bar setup

void init_font(Display *dpy, Window win, int screen);

void cleanup(Display *dpy, Window win, GC gc);

void set_dock_properties(Display *dpy, Window win, int width);

void draw_bar(Display *dpy, Window win, GC gc, BarState *s);

//bar modules
void update_workspace(BarState *s);
void update_volume(BarState *s);
void update_ipv4(BarState *s);
void update_ram(BarState *s);
void update_datetime(BarState *s);


//font rendering
extern XftDraw *xft_draw;
extern XftFont *xft_font;
extern XftColor xft_color;





#endif
