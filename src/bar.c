#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xft/Xft.h>
#include <fontconfig/fontconfig.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>

#include "bar.h"
#include "config.h"



XftFont *xft_font = NULL;
XftDraw *xft_draw = NULL;
XftColor xft_color;


//reder the font 
void 
init_font(Display *dpy, Window win, int screen)
{
    xft_font = XftFontOpenName(dpy, screen, BAR_FONT);

    unsigned r = (TEXT_COLOR >> 16) & 0xFF;
    unsigned g = (TEXT_COLOR >> 8)  & 0xFF;
    unsigned b = (TEXT_COLOR)       & 0xFF;

    XRenderColor xrcolor = {
        .red   = r * 257,
        .green = g * 257,
        .blue  = b * 257,
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
    int screen = DefaultScreen(dpy);

	// free the font color
    XftColorFree(
        dpy,
        DefaultVisual(dpy, screen),
        DefaultColormap(dpy, screen),
        &xft_color
    );

	//close the font
    if (xft_font)
        XftFontClose(dpy, xft_font);

    // destroy xft
    if (xft_draw)
        XftDrawDestroy(xft_draw);

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

void 
update_volume(BarState *s)
{
    FILE *f = popen("pactl get-sink-volume @DEFAULT_SINK@", "r");

    fgets(s->volume, sizeof(s->volume), f);

    pclose(f);
}

void 
update_ram(BarState *s)
{
    FILE *f = fopen("/proc/meminfo", "r");

    long total = 0, free = 0;
    char line[128];

    while (fgets(line, sizeof(line), f))
    {
        if (sscanf(line, "MemTotal: %ld kB", &total) == 1) continue;
        if (sscanf(line, "MemAvailable: %ld kB", &free) == 1) continue;
    }

    fclose(f);

    long used = total - free;

    snprintf(s->ram, sizeof(s->ram),
             "RAM %ldMB", used / 1024);
}


void 
update_ipv4(BarState *s)
{
    struct ifaddrs *ifaddr, *ifa;

    getifaddrs(&ifaddr);

    for (ifa = ifaddr; ifa; ifa = ifa->ifa_next)
    {
        if (!ifa->ifa_addr) continue;

        if (ifa->ifa_addr->sa_family == AF_INET)
        {
            void *addr = &((struct sockaddr_in*)ifa->ifa_addr)->sin_addr;

            inet_ntop(AF_INET, addr, s->ipv4, sizeof(s->ipv4));
            break;
        }
    }

    freeifaddrs(ifaddr);
}


//worksapces


int 
connect_i3_ipc(void) 
{
    char *path = getenv("I3SOCK");
    if (!path) path = "/run/user/1000/i3/ipc-socket"; // Fallback standard

    int sock = socket(AF_LOCAL, SOCK_STREAM, 0);
    if (sock < 0) return -1;

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_LOCAL;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);

    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(sock);
        return -1;
    }
    return sock;
}

int 
read_full(int fd, void *buf, size_t n) 
{
    size_t off = 0;
    while (off < n) {
        ssize_t r = read(fd, (char*)buf + off, n - off);
        if (r <= 0) return -1;
        off += r;
    }
    return 0;
}

void 
send_i3_message(int sock, uint32_t type, const char *payload) 
{
    uint32_t len = payload ? strlen(payload) : 0;

    write(sock, I3_IPC_MAGIC, 6);
    write(sock, &len, 4);
    write(sock, &type, 4);

    if (len > 0)
        write(sock, payload, len);
}

void 
i3_subscribe(int sock) 
{
    const char *subscribe_json = "[\"workspace\"]";
    // Inviamo solo il messaggio. La conferma e gli eventi arriveranno 
    // fluidamente tutti nello stesso posto: la select() nel main.
    send_i3_message(sock, I3_IPC_MESSAGE_TYPE_SUBSCRIBE, subscribe_json);
}

void 
update_workspaces(int query_sock, BarState *s) 
{
    send_i3_message(query_sock, I3_IPC_MESSAGE_TYPE_GET_WORKSPACES, NULL);

    char magic[6];
    uint32_t len, type;

    if (read_full(query_sock, magic, 6) < 0) return;
    if (read_full(query_sock, &len, 4) < 0) return;
    if (read_full(query_sock, &type, 4) < 0) return;

    char *json = malloc(len + 1);
    if (!json) return;

    if (read_full(query_sock, json, len) < 0) {
        free(json);
        return;
    }
    json[len] = '\0';

    s->workspace[0] = '\0';
    char *p = json;

    while ((p = strstr(p, "\"name\":\"")) != NULL) {
        p += 8;
        char *end = strchr(p, '"');
        if (!end) break;

        char name[64] = {0};
        size_t nlen = end - p;
        if (nlen >= sizeof(name)) nlen = sizeof(name) - 1;
        memcpy(name, p, nlen);

        // json check
        char *focused_ptr = strstr(end, "\"focused\":");
        int focused = 0;
        if (focused_ptr && (focused_ptr - end < 150)) {
            if (strncmp(focused_ptr + 10, "true", 4) == 0) {
                focused = 1;
            }
        }

        char tmp[128];
        if (focused)
            snprintf(tmp, sizeof(tmp), "[%s] ", name);
        else
            snprintf(tmp, sizeof(tmp), "%s ", name);

        strncat(s->workspace, tmp, sizeof(s->workspace) - strlen(s->workspace) - 1);
        p = end;
    }

    free(json);

	//free all space left
    size_t len_ws = strlen(s->workspace);
    if (len_ws > 0 && s->workspace[len_ws - 1] == ' ')
        s->workspace[len_ws - 1] = '\0';
}

