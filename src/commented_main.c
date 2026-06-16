#include <X11/X.h>
#include <X11/Xlib.h>
#include <stdio.h>

int main(void)
{
    /*
     * Struttura generica che Xlib usa per consegnarci eventi.
     *
     * Dentro può esserci:
     * - Expose
     * - KeyPress
     * - ButtonPress
     * - ConfigureNotify
     * - ecc.
     */
    XEvent ev;


    /*
     * Apriamo una connessione verso l'X Server.
     *
     * X11 funziona client/server:
     *
     * nostro programma ---> X Server ---> schermo
     *
     * dpy rappresenta la connessione.
     */
    Display *dpy = XOpenDisplay(NULL);

    if (dpy == NULL)
    {
        fprintf(stderr, "cannot open display\n");
        return 1;
    }


    /*
     * Otteniamo lo screen di default.
     *
     * Nella maggior parte dei casi è semplicemente:
     *
     * screen = 0
     */
    int screen = DefaultScreen(dpy);


    /*
     * Otteniamo la root window.
     *
     * La root window è la finestra gigante che copre
     * l'intero desktop.
     *
     * Tutte le altre finestre sono figlie della root.
     */
    Window root = RootWindow(dpy, screen);


    /*
     * Creiamo una nuova finestra.
     *
     * ATTENZIONE:
     *
     * La finestra esiste sul server X,
     * ma NON è ancora visibile.
     */
    Window win = XCreateSimpleWindow(
        dpy,        // connessione X
        root,       // parent window

        0,          // posizione x
        0,          // posizione y

        1920,       // larghezza
        24,         // altezza

        0,          // spessore bordo

        0,          // colore bordo
        0x222222    // colore sfondo
    );


    /*
     * Ci iscriviamo agli eventi.
     *
     * ExposureMask:
     *     voglio sapere quando la finestra
     *     deve essere ridisegnata.
     *
     * KeyPressMask:
     *     voglio sapere quando vengono
     *     premuti tasti.
     */
    XSelectInput(
        dpy,
        win,
        ExposureMask | KeyPressMask
    );


    /*
     * Ora chiediamo al server X
     * di mostrare la finestra.
     *
     * Prima esisteva ma era invisibile.
     */
    XMapWindow(dpy, win);


    /*
     * GC = Graphics Context
     *
     * Puoi pensarlo come una "penna"
     * usata per disegnare.
     *
     * Quasi tutte le operazioni grafiche
     * richiedono un GC.
     */
    GC gc = XCreateGC(
        dpy,
        win,
        0,
        NULL
    );


    /*
     * Event loop.
     *
     * Da qui in poi il programma
     * vive aspettando eventi.
     */
    while (1)
    {
        /*
         * Blocca il programma finché
         * non arriva un evento.
         *
         * Quando arriva, viene copiato
         * dentro "ev".
         */
        XNextEvent(dpy, &ev);


        /*
         * Controlliamo il tipo di evento.
         */
        if (ev.type == Expose)
        {
            /*
             * Expose significa:
             *
             * "questa finestra necessita
             *  di essere ridisegnata"
             *
             * Tipicamente:
             *
             * - appena compare
             * - dopo essere stata coperta
             * - dopo essere stata scoperta
             * - dopo resize
             */
            printf("e' da ridisegnare!\n");


            /*
             * Disegna una stringa.
             *
             * dpy  -> connessione X
             * win  -> finestra destinazione
             * gc   -> penna
             *
             * 10   -> coordinata x
             * 18   -> coordinata y
             *
             * "test bar"
             * 5 -> numero di caratteri
             *
             * ATTENZIONE:
             *
             * Qui hai un piccolo bug.
             *
             * "test bar" ha 8 caratteri.
             *
             * Con 5 verrà disegnato:
             *
             * "test "
             */
            XDrawString(
                dpy,
                win,
                gc,
                10,
                18,
                "test bar",
                8
            );


            /*
             * Forza l'invio delle richieste
             * al server X.
             *
             * Molte operazioni Xlib
             * vengono bufferizzate.
             */
            XFlush(dpy);
        }
    }

    return 0;
}
