#include <gtk/gtk.h>
#include <cairo.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <libintl.h>
#include <locale.h>

#define PROG "pmcputemp-0.60"
#define AUTHOR "(c) Michael Amadio"
#define DATE "2015"
#define LICENCE "GPLv2"
#define _(STRING)    gettext(STRING)

int width = 320;
int height = 180;

static char *lang_font() {
	char *local = getenv("LANG");
	char *cn = "zh";
	char *jn = "ja";
	char *kn = "ko";
	char *font = "sans";
	if ((strncmp(local, cn, 2) == 0) ||
			(strncmp(local, jn, 2) == 0) || (strncmp(local, kn, 2) == 0)) {
		font = "wenquanyi micro hei"; /*"m+1p+ipag"*/
	}
	return font;
}
	
static void paint_win(cairo_surface_t *cs) {
	
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
	char *font_family = lang_font();
	cairo_select_font_face(c, font_family, CAIRO_FONT_SLANT_NORMAL,
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

int show_about(gint argc, gchar *argv[]) {
    GdkPixmap *pixmap;
    GtkWidget *image;
    GtkWidget *window;
    cairo_t *cr;
	cairo_surface_t *cs;
	
    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_signal_connect(G_OBJECT(window), "delete-event", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(window);

    pixmap = gdk_pixmap_new(window->window, width, height, -1);
    cr = gdk_cairo_create(pixmap);
    cs = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    paint_win(cs);
	cairo_set_source_surface (cr, cs, 0.0, 0.0);
	cairo_rectangle (cr, 0.0, 0.0, width, height);
	cairo_fill (cr);
    image = gtk_image_new_from_pixmap(pixmap, NULL);

    gtk_container_add(GTK_CONTAINER(window), image);


    gtk_widget_show(image);

    gtk_main();

    return 0;
}

