/* Status icon for cpu temperature*/
/* (c) Mick Amadio 2011-2015 GPL 2 , 01micko.com*/
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 */

#include <cairo.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <glib/gstdio.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <libintl.h>
#include <locale.h>
#include "about.h"

#define _GNU_SOURCE
#define ICON ".config/pmcputemp/temp.png"
#define CONFDIR ".config/pmcputemp"
#define CONF ".config/pmcputemp/pmcputemprc"
#define _(STRING)    gettext(STRING)

char temp_icon[128];
static char configdir[128];
static char conf[128];
/* poor man's degree symbol for cairo's poor font handling */
const char deg[2] = "o"; 

GdkPixbuf *temp_pixbuf;
GtkStatusIcon *tray_icon;

unsigned int interval = 5000; /*update interval in milliseconds*/ 

FILE *fp;
FILE *ft;
const char *home;
GError *gerror = NULL;

int mk_conf() {
	/* script to setup system temperature location */
	int ret_val = system("sh pmcputemp.sh");
	return ret_val;
}

int cpu_temp() {
	home = getenv("HOME");
	conf[128] = sprintf(conf, "%s/%s", home, CONF);
	char temp_file[128];
	char temp_file_out[128];
	int ret_try;
	int temp_val;
	while (1) {
		fp = fopen(conf, "r");
		if ( fp == NULL ) {
			ret_try = mk_conf();
			if (ret_try != 0) {
				fprintf(stderr, _("Unable to create configuration file.\n"));
				exit (1); /* kill it */
			} else {
				fprintf(stdout, _("An attempt has been made to create " 
							"a configuration file\n"));
					continue;
			}
		}
		if (fgets(temp_file, sizeof(temp_file), fp) != NULL) {
			temp_file_out[128] = sprintf(temp_file_out, "%s", temp_file);
		} else {
			fprintf(stderr, _("File is empty, trying again\n"));
			continue;
		}
		fclose(fp);
	
		ft = fopen(temp_file_out, "r");
		if (ft == NULL) {
			fprintf(stderr, _("Can't open configured file, deleting "
												"configuration file\n"));
			unlink(conf);
			continue;
		}
		//while(!feof(ft)) {
		int success = fscanf(ft, "%d", &temp_val);
		if (success < 1) {
			fprintf(stderr,_("Failed to read temperature, giving up."));
			exit (1);
		}
		fclose(ft);
		break;
	}
	int temperature = temp_val / 1000;
	return temperature;
}

/* makes the tray icon to be displayed */
int paint_icon() {
	home = getenv("HOME");
	int temp = cpu_temp();
	configdir[128] = sprintf(configdir,"%s/%s", home, CONFDIR);
	mkdir(configdir, 0755);
	temp_icon[128] = sprintf(temp_icon,"%s/%s", home, ICON);
	char s[4];
	s[4] = sprintf(s, "%d", temp);

	float r1, g1, b1, r2, g2, b2;
	if (temp < 40 ) {
		r1 = 0.4; r2 = 0.2;
		g1 = 0.9; g2 = 0.7;
		b1 = 0.9; b2 = 0.7;
	}
	if ((temp >= 40) && (temp <= 56)) {
		r1 = 0.4; r2 = 0.4;
		g1 = 0.9; g2 = 0.7;
		b1 = 0.4; b2 = 0.4;
	}
	if (temp > 56 ) {
		r1 = 0.9; r2 = 0.7;
		g1 = 0.7; g2 = 0.4;
		b1 = 0.7; b2 = 0.4;		
	}
	if (temp > 99) {
		exit (1);
	}
	double width = 24;
	double height = 24;
	cairo_surface_t *cs;
	cs = cairo_image_surface_create 
							(CAIRO_FORMAT_ARGB32, width, height);
	cairo_t *c;
	c = cairo_create(cs);
	cairo_select_font_face(c, "sans", CAIRO_FONT_SLANT_NORMAL,
			CAIRO_FONT_WEIGHT_NORMAL);
	double x = 0;
	double y = 0;
	double aspect = 0.6;
	double corner_radius = height / 10.0;
	double radius = corner_radius / aspect;
	double degrees = M_PI / 180.0;
	cairo_new_sub_path (c);
	cairo_arc (c, x + width - radius, y + radius, radius, -90 * degrees, 0 * degrees);
	cairo_arc (c, x + width - radius, y + height - radius, radius, 0 * degrees, 90 * degrees);
	cairo_arc (c, x + radius, y + height - radius, radius, 90 * degrees, 180 * degrees);
	cairo_arc (c, x + radius, y + radius, radius, 180 * degrees, 270 * degrees);
	cairo_close_path (c);
	cairo_pattern_t *linear = cairo_pattern_create_linear(0, 0, width, height); 
	cairo_pattern_add_color_stop_rgb(linear, 0, r1, g1, b1); 
	cairo_pattern_add_color_stop_rgb(linear, 1, r2, g2, b2);
	cairo_set_source(c, linear);
	cairo_fill_preserve (c);

	cairo_set_font_size(c, 14.0);
	cairo_move_to(c, 1.0, 17.0);
	cairo_set_source_rgb(c, 0.0, 0.0, 0.0);
	cairo_show_text(c, s);
	cairo_set_font_size(c, 8.0);
	cairo_move_to(c, 18.0, 11.0);
	cairo_set_source_rgb(c, 0.0, 0.0, 0.0);
	cairo_show_text(c, deg);
	cairo_destroy(c);
	cairo_surface_write_to_png (cs, temp_icon);
	return temp;
}

gboolean Update(gpointer ptr) {
	/* refreshes the icon */
	paint_icon();
	temp_pixbuf = gdk_pixbuf_new_from_file(temp_icon,&gerror);
	gtk_status_icon_set_from_pixbuf(tray_icon,temp_pixbuf);
	return 1;
}

void  view_popup_menu_about(GtkWidget *w, gpointer dummy) {
	show_about(); /* "About - menu */
}

void  quit(GtkWidget *w, gpointer dummy) {
    gtk_main_quit();
}

void tray_icon_on_menu(GtkStatusIcon *status_icon, guint button,guint activate_time, gpointer user_data) {
	/* right click menu */
	GtkWidget *menu, *menuitem, *iconw;
	menu = gtk_menu_new();
	
	menuitem = gtk_image_menu_item_new_with_label(_("About"));
	iconw = gtk_image_new_from_stock(GTK_STOCK_ABOUT, GTK_ICON_SIZE_MENU);
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuitem), iconw);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	g_signal_connect(menuitem, "activate", (GCallback) view_popup_menu_about, status_icon);
	
	menuitem = gtk_image_menu_item_new_with_label(_("Quit"));
    iconw = gtk_image_new_from_stock(GTK_STOCK_QUIT, GTK_ICON_SIZE_MENU);
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuitem), iconw);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
    g_signal_connect(menuitem, "activate", (GCallback) quit, status_icon);
	
	gtk_widget_show_all(menu);
	gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, button, gdk_event_get_time(NULL));
}

static GtkStatusIcon *create_tray_icon() {
	tray_icon = gtk_status_icon_new();
	g_signal_connect(G_OBJECT(tray_icon), "popup-menu", G_CALLBACK(tray_icon_on_menu), NULL);
	
	home = getenv("HOME");
	temp_icon[128] = sprintf(temp_icon,"%s/%s", home, ICON);
        
	temp_pixbuf = gdk_pixbuf_new_from_file(temp_icon,&gerror);
	gtk_status_icon_set_from_pixbuf(tray_icon,temp_pixbuf);							   
	gtk_status_icon_set_tooltip_text(tray_icon, _("CPU temperature"));
							   
	gtk_status_icon_set_visible(tray_icon, TRUE);

	return tray_icon;
}

int main(int argc, char **argv) {
	setlocale( LC_ALL, "" ); 
	bindtextdomain( "pmcputemp", "/usr/share/locale" ); 
	textdomain( "pmcputemp" );
	
	gtk_init(&argc, &argv);
	paint_icon(); /* needed to kick it off */
	create_tray_icon();
	
	g_timeout_add(interval, Update, NULL); /*update after 5sec*/
	
	gtk_main();

	return 0;
}

