#include <cairo.h>
#include <cairo-xlib.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <libintl.h>
#include <locale.h>

#define PROG "pmcputemp-0.30"
#define AUTHOR "(c) Michael Amadio"
#define DATE "2014"
#define LICENCE "GPLv2"
#define _(STRING)    gettext(STRING)

int width = 320;
int height = 180;
	
void paint_win(cairo_surface_t *cs) {
	
	const char *desc = (_("A simple tray cpu temperature monitor")); 
	float r1, g1, b1, r2, g2, b2;
	/* for gradient */
	r1 = 0.2;
	g1 = 0.9;
	b1 = 0.9;
	r2 = 0.4;
	g2 = 0.6;
	b2 = 0.6;
	
	cairo_t *c;
	c = cairo_create(cs);
	cairo_select_font_face(c, "sans", CAIRO_FONT_SLANT_NORMAL,
			CAIRO_FONT_WEIGHT_NORMAL);
	cairo_rectangle(c, 0.0, 0.0, width, height);
	cairo_pattern_t *linear = cairo_pattern_create_linear(0, 0, width, height); 
	cairo_pattern_add_color_stop_rgb(linear, 0, r1, g1, b1); 
	cairo_pattern_add_color_stop_rgb(linear, 1, r2, g2, b2);
	cairo_set_source(c, linear);
	cairo_fill(c);

	cairo_set_font_size(c, 20.0);
	cairo_move_to(c, 72.0, 40.0);
	cairo_set_source_rgb(c, 0.0, 0.0, 0.0);
	cairo_show_text(c, PROG);
	cairo_set_font_size(c, 13.0);
	cairo_move_to(c, 27.0, 81.0);
	cairo_set_source_rgb(c, 0.0, 0.0, 0.0);
	cairo_show_text(c, desc);
	cairo_move_to(c, 92.0, 110.0);
	cairo_set_source_rgb(c, 0.0, 0.0, 0.0);
	cairo_show_text(c, AUTHOR);
	cairo_move_to(c, 138.0, 140.0);
	cairo_set_source_rgb(c, 0.0, 0.0, 0.0);
	cairo_show_text(c, DATE);
	cairo_move_to(c, 132.0, 160.0);
	cairo_set_source_rgb(c, 0.0, 0.0, 0.0);
	cairo_show_text(c, LICENCE);
	cairo_show_page(c);
	cairo_destroy(c);
}

void show_xlib() {
	
	Display *dpy;
	Window rootwin;
	Window win;
	XEvent e;
	int scr;
	cairo_surface_t *cs;

	if (!(dpy = XOpenDisplay(NULL))) {
		fprintf(stderr, "ERROR: Could not open display\n");
		exit(1);
	}
	scr = DefaultScreen(dpy);
	rootwin = RootWindow(dpy, scr);
	win = XCreateSimpleWindow(dpy, rootwin, 1, 1, width, height, 0, 
			BlackPixel(dpy, scr), BlackPixel(dpy, scr));
	XStoreName(dpy, win, PROG);
	/* cleanly close window with [X] */
	Atom wm_delete = XInternAtom(dpy, "WM_DELETE_WINDOW", True);
	XSetWMProtocols(dpy, win, &wm_delete, 1);
	XSelectInput(dpy, win, ExposureMask);
	XMapWindow(dpy, win);
	cs = cairo_xlib_surface_create(dpy, win, 
				DefaultVisual(dpy, 0), width, height);
	int loop = 1;
	while (loop) {
		XNextEvent(dpy, &e);
		if(e.type == Expose && e.xexpose.count < 1) {
		paint_win(cs);
		} 
		else if (e.type == ClientMessage) {
			loop = 0;
			break;
		}
	}
	cairo_surface_destroy(cs);
	XCloseDisplay(dpy);
}

