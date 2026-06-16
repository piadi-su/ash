#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xft/Xft.h> // <- Nuova libreria per font moderni
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>
#include <stdint.h>

#include "config.h"

#define I3_IPC_MAGIC "i3-ipc"
#define I3_IPC_MESSAGE_TYPE_GET_WORKSPACES 1
#define I3_IPC_MESSAGE_TYPE_SUBSCRIBE 2

int connect_i3_ipc() {
    char *path = getenv("I3SOCK");
    if (!path) path = "/run/user/1000/i3/ipc-socket";

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

void send_i3_message(int sock, uint32_t type, const char *payload) {
    uint32_t len = payload ? strlen(payload) : 0;
    write(sock, I3_IPC_MAGIC, 6);
    write(sock, &len, 4);
    write(sock, &type, 4);
    if (len > 0) write(sock, payload, len);
}

void subscribe_to_workspaces(int sock) {
    const char *subscribe_json = "[\"workspace\"]";
    send_i3_message(sock, I3_IPC_MESSAGE_TYPE_SUBSCRIBE, subscribe_json);
    char magic[6]; uint32_t r_len, r_type;
    read(sock, magic, 6); read(sock, &r_len, 4); read(sock, &r_type, 4);
    if (r_len > 0) { char *t = malloc(r_len); read(sock, t, r_len); free(t); }
}

void update_workspaces_string(int query_sock, char *buffer, size_t max_len) {
    send_i3_message(query_sock, I3_IPC_MESSAGE_TYPE_GET_WORKSPACES, NULL);
    char magic[6]; uint32_t reply_len, reply_type;
    if (read(query_sock, magic, 6) <= 0) return;
    if (read(query_sock, &reply_len, 4) <= 0) return;
    if (read(query_sock, &reply_type, 4) <= 0) return;

    char *json = malloc(reply_len + 1);
    uint32_t bytes_read = 0;
    while (bytes_read < reply_len) {
        int r = read(query_sock, json + bytes_read, reply_len - bytes_read);
        if (r <= 0) break;
        bytes_read += r;
    }
    json[reply_len] = '\0';

    buffer[0] = '\0';
    char *ptr = json;
    while ((ptr = strstr(ptr, "\"name\":\"")) != NULL) {
        ptr += 8; 
        char *end_name = strchr(ptr, '"');
        if (!end_name) break;

        char name[32] = {0};
        strncpy(name, ptr, end_name - ptr);

        char *focused_ptr = strstr(end_name, "\"focused\":");
        int is_focused = 0;
        if (focused_ptr && (focused_ptr - end_name < 150)) {
            if (strncmp(focused_ptr + 10, "true", 4) == 0) is_focused = 1;
        }

        char tmp[64];
        if (is_focused) snprintf(tmp, sizeof(tmp), "[%s] ", name);
        else snprintf(tmp, sizeof(tmp), "%s ", name);

        if (strlen(buffer) + strlen(tmp) < max_len) strcat(buffer, tmp);
        ptr = end_name;
    }
    free(json);
    size_t blen = strlen(buffer);
    if (blen > 0 && buffer[blen - 1] == ' ') buffer[blen - 1] = '\0';
}

int main() {
    int event_sock = connect_i3_ipc();
    int query_sock = connect_i3_ipc();
    if (event_sock < 0 || query_sock < 0) return 1;

    subscribe_to_workspaces(event_sock);

    Display *dpy = XOpenDisplay(NULL);
    if (!dpy) return 1;

    int screen = DefaultScreen(dpy);
    int screen_width = DisplayWidth(dpy, screen);
    unsigned long bg_pixel = BlackPixel(dpy, screen);

    Window win = XCreateSimpleWindow(dpy, RootWindow(dpy, screen), 0, 0, screen_width, BAR_HEIGHT, 0, bg_pixel, bg_pixel);

    // i3 Dock Hints
    Atom wm_type = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", False);
    Atom type_dock = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DOCK", False);
    XChangeProperty(dpy, win, wm_type, XA_ATOM, 32, PropModeReplace, (unsigned char *)&type_dock, 1);

    Atom wm_strut = XInternAtom(dpy, "_NET_WM_STRUT_PARTIAL", False);
    unsigned long struts[12] = {0};
    struts[2] = BAR_HEIGHT; struts[8] = 0; struts[9] = screen_width - 1;
    XChangeProperty(dpy, win, wm_strut, XA_CARDINAL, 32, PropModeReplace, (unsigned char *)struts, 12);

    XSelectInput(dpy, win, ExposureMask);

    // Inizializzazione Xft per font antialiased moderni
    Visual *visual = DefaultVisual(dpy, screen);
    Colormap cmap = DefaultColormap(dpy, screen);
    XftDraw *xft_draw = XftDrawCreate(dpy, win, visual, cmap);

    XftFont *font = XftFontOpenName(dpy, screen, BAR_FONT);
    if (!font) {
        fprintf(stderr, "Impossibile caricare Iosevka, uso 'sans'\n");
        font = XftFontOpenName(dpy, screen, "sans-10");
    }

    XftColor xft_color;
    XRenderColor render_color = {0xffff, 0xffff, 0xffff, 0xffff}; // Bianco puro (RGBA)
    XftColorAllocValue(dpy, visual, cmap, &render_color, &xft_color);

    XMapWindow(dpy, win);
    XFlush(dpy);

    char ws_text[256] = {0};
    update_workspaces_string(query_sock, ws_text, sizeof(ws_text));

    int x11_fd = ConnectionNumber(dpy);
    int max_fd = (x11_fd > event_sock) ? x11_fd : event_sock;
    fd_set in_fds;

    while (1) {
        FD_ZERO(&in_fds);
        FD_SET(x11_fd, &in_fds);
        FD_SET(event_sock, &in_fds);

        select(max_fd + 1, &in_fds, NULL, NULL, NULL);
        int redraw = 0;

        while (XPending(dpy)) {
            XEvent ev; XNextEvent(dpy, &ev);
            if (ev.type == Expose && ev.xexpose.count == 0) redraw = 1;
        }

        if (FD_ISSET(event_sock, &in_fds)) {
            char magic[6]; uint32_t r_len, r_type;
            if (read(event_sock, magic, 6) > 0) {
                read(event_sock, &r_len, 4); read(event_sock, &r_type, 4);
                if (r_len > 0) { char *ev_j = malloc(r_len); read(event_sock, ev_j, r_len); free(ev_j); }
                update_workspaces_string(query_sock, ws_text, sizeof(ws_text));
                redraw = 1;
            }
        }

        if (redraw) {
            XClearWindow(dpy, win);
            
            // Calcolo corretto dell'altezza del testo con Xft
            int text_y = (BAR_HEIGHT / 2) + (font->ascent / 2) - (font->descent / 2);
            
            // Disegna la stringa usando Xft (supporta UTF-8 e font moderni)
            XftDrawStringUtf8(xft_draw, &xft_color, font, 10, text_y, (XftChar8 *)ws_text, strlen(ws_text));
            XFlush(dpy);
        }
    }

    // Pulizia
    XftColorFree(dpy, visual, cmap, &xft_color);
    XftFontClose(dpy, font);
    XftDrawDestroy(xft_draw);
    close(event_sock); close(query_sock);
    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);
    return 0;
}
