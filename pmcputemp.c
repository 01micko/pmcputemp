/* Status icon for cpu temperature */
/* (c) Mick Amadio 2011-2015 GPL 2 , 01micko.com */
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
#define CPU_FILE "/proc/cpuinfo"

char temp_icon[128];
static char configdir[256];
static char conf[256];
char s[4];
/* poor man's degree symbol for cairo's poor font handling */
const char deg[2] = "o"; 
char line[512];
char freq_out[1024];
int proc_out;
gchar *tooltip_out;
gchar *tool_tip;
int x_procs;
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
	conf[256] = snprintf(conf, sizeof(conf), "%s/%s", home, CONF);
	char temp_file_out[512];
	int ret_try;
	int temp_val;
	int tries = 0;
	while (1) {
		if (tries > 10) {
			fprintf(stderr,"Aborting\n");
			show_about(1);
			exit(1);
		}
		fp = fopen(conf, "r");
		if ( fp == NULL ) {
			ret_try = mk_conf();
			if (ret_try != 0) {
				fprintf(stderr, _("Unable to create configuration file.\n"));
				show_about(1);
				exit (1); /* kill it */
			} else {
				fprintf(stdout, _("An attempt has been made to create " 
							"a configuration file\n"));
					tries++;
					continue;
			}
		}
		int success_conf = fscanf(fp, "%s", temp_file_out);
		if (success_conf < 1) {
			fprintf(stderr, _("File is empty, trying again\n"));
			fclose(fp);
			tries++;
			continue;
		}
		fclose(fp);
	
		ft = fopen(temp_file_out, "r");
		if (ft == NULL) {
			fprintf(stderr, _("Can't open configured file, deleting "
												"configuration file\n"));
			unlink(conf);
			tries++;
			continue;
		}
		int success_temp = fscanf(ft, "%d", &temp_val);
		if (success_temp < 1) {
			fprintf(stderr,_("Failed to read temperature, giving up.\n"));
			show_about(1);
			exit(1);
		}
		fclose(ft);
		break;
	}
	if ((temp_val < 10000) || (temp_val > 100000)) {
		fprintf(stderr,_("Temperature out of range or bad value, exiting.\n"));
		exit(1);
	}
	int temperature = temp_val / 1000;
	return temperature;
}

/* makes the tray icon to be displayed */
int paint_icon() {
	home = getenv("HOME");
	int temp = cpu_temp();
	configdir[256] = snprintf(configdir, 
						sizeof(configdir),"%s/%s", home, CONFDIR);
	mkdir(configdir, 0755);
	temp_icon[128] = snprintf(temp_icon,
						sizeof(temp_icon),"%s/%s", home, ICON);
	s[4] = snprintf(s, sizeof(s),"%d", temp);
	float r1, g1, b1, r2, g2, b2;
	if (temp < 46 ) {
		r1 = 0.4; r2 = 0.2;
		g1 = 0.9; g2 = 0.7;
		b1 = 0.9; b2 = 0.7;
	}
	if ((temp >= 46) && (temp <= 70)) {
		r1 = 0.4; r2 = 0.4;
		g1 = 0.9; g2 = 0.7;
		b1 = 0.4; b2 = 0.4;
	}
	if ((temp > 70 ) && (temp <= 80)) {
		r1 = 0.6; r2 = 0.8;
		g1 = 0.3; g2 = 0.5;
		b1 = 0.1; b2 = 0.1;
	}
	if (temp > 80 ) {
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
	cairo_surface_write_to_png (cs, temp_icon);
	cairo_destroy(c);
	return temp;
}

/* tooltip */
char *split_string(char *var) {
	char *buf = var;
	const char s[2] = ":";
	char *token;
	token = strtok(buf, s);
	token = strtok(NULL , s);
	return token;
}

/* get number of processors */
int num_procs() {
	char line[512];
	fp = fopen(CPU_FILE, "r");
	if ( fp != NULL ) {
		while (1) {
			while (fgets(line, sizeof(line), fp)) {
				if (strncmp(line,  "processor", 9) == 0) {
					char *res = split_string(line);
					res = strtok(res, "\n");
					proc_out = atoi(res);
				}
			}
			break;
		}
	}
	fclose(fp);
	return proc_out;
}

gchar *get_tt(int num){
	int res1;
	const char *proc = "CPU";
	const char *temperature = (_("temperature"));
	const char *num_processors = (_("Processors"));
	const char *freq_hz = "MHz";
	int i = 0;
	int pos = 0;
	fp = fopen(CPU_FILE, "r");
	while (fgets(line, sizeof(line), fp)) {
		res1 = strncmp(line,  "cpu MHz", 7);
		if (res1 == 0) {
			char *res = split_string(line);
			res = strtok(res, "\n");
			pos += sprintf(&freq_out[pos], "%s %d: %8s %2s\n", proc, i, res, freq_hz);
			i++;
		}
	}
	fclose(fp);
	tooltip_out = g_strdup_printf("%s %s\n%s = %d\n%s",proc, temperature, num_processors, (x_procs + 1), freq_out);
	return tooltip_out;
}
/* end tooltip */

gboolean Update(gpointer ptr) {
	/* refreshes the icon */
	paint_icon();
	temp_pixbuf = gdk_pixbuf_new_from_file(temp_icon,&gerror);
	gtk_status_icon_set_from_pixbuf(tray_icon,temp_pixbuf);
	x_procs = num_procs();
	tool_tip = get_tt(x_procs);
	gtk_status_icon_set_tooltip(tray_icon, tool_tip);
	g_free(tooltip_out);
	return 1;
}

void  view_popup_menu_about(GtkWidget *w, gpointer dummy) {
	show_about(0); /* "About - menu */
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
	temp_icon[128] = snprintf(temp_icon, sizeof(temp_icon),"%s/%s", home, ICON);
        
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
	if (argc == 2) { /* only accepts 1 - 10 */
		interval = atoi(argv[1]); /* returns 0 if garbage is input */
		if ((interval < 1) || (interval > 10)) {
		fprintf(stderr,_("Polling interval out of range. Use 1 to 10\n"
			"Using default 5 seconds\n"));
			interval = 5;
		}
		interval = interval * 1000; /* millisecs */
	}
	gtk_init(&argc, &argv);
	
	create_tray_icon();
	Update(NULL); /* needed to kick it off */
	g_timeout_add(interval, Update, NULL); /*update after 5sec*/
	
	gtk_main();

	return 0;
}

