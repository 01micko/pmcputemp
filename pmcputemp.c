/* Status icon for cpu temperature */
/* (c) Mick Amadio 2011-2025 GPL 2 , 01micko@gmx.com */
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
#ifdef HAVE_MENU
	#include "about.h"
#endif /* HAVE_MENU */
#ifndef HAVE_MENU
	#include "pmcputemp.h"
#endif
#ifdef HAVE_SENSORS
	#include "lm.h"
#endif /* HAVE_SENSORS */

#define _GNU_SOURCE
#define ICON "temp.png"
#define CONFDIR "pmcputemp"
#define CONF "pmcputemprc"
#define _(STRING)    gettext(STRING)
#define CPU_FILE "/proc/cpuinfo"

//char temp_icon[128];
static gchar *configdir;
static gchar *conf;
char s[5];
char *module = NULL;
char command[42];
/* poor man's degree symbol for cairo's poor font handling */
const char deg[2] = "o"; 
char line[512];
char freq_out[512];
gchar *tooltip_out;
gchar *tool_tip;
gchar *temp_icon;
unsigned short x_procs;
GdkPixbuf *temp_pixbuf;
GtkStatusIcon *tray_icon;
char style;
int first_run = 1;
unsigned int interval = 5000; /*update interval in milliseconds*/ 

FILE *ft;
GError *gerror = NULL;

int mk_conf(char *mod) {
	/* script to setup system temperature location */
	first_run = 0;
	if (mod != NULL) {
		printf("load module : %s\n", mod);
		command[40] = sprintf(command, "bash -c pmcputemp-sh %s", module);
	} else {
		command[40] = sprintf(command, "bash -c pmcputemp-sh");
	}
	int ret_val = system(command);
	return ret_val;
}

int cpu_temp() {
	FILE *fp;
	char temp_file_out[512];
	int ret_try;
	int temp_val;
	int tries = 0;
	while (1) {
		if (tries > 10) {
			fprintf(stderr,"Aborting\n");
		#ifdef HAVE_MENU
			show_about(1, NULL);
		#else
			printf("It seems your processor is unsupported. Exiting\n");
		#endif /* HAVE_MENU */
			exit(1);
		}
		fp = fopen(conf, "r");
		if ( fp == NULL ) {
			ret_try = mk_conf(module);
			if (ret_try != 0) {
				fprintf(stderr, _("Unable to create configuration file.\n"));
		#ifdef HAVE_MENU
				show_about(1, NULL);
		#else
			printf("It seems your processor is unsupported. Exiting\n");
		#endif /* HAVE_MENU */
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
			fclose(ft);
		#ifdef HAVE_MENU
				show_about(1, NULL);
		#else
			printf("It seems your processor is unsupported. Exiting\n");
		#endif /* HAVE_MENU */
			exit(1);
		}
		fclose(ft);
		break;
	}
	if ((temp_val < 10000) || (temp_val > 125000)) {
		fprintf(stderr,_("Temperature out of range or bad value, exiting.\n"));
		exit(1);
	}
	int temperature = temp_val / 1000;
	return temperature;
}

/* makes the tray icon image to be displayed */
int paint_icon(char style) {
	int temp = cpu_temp();
	float font_size = 14.0;
	float deg_size = 8.0;
	s[4] = snprintf(s, sizeof(s),"%d", temp);
	float r1, g1, b1, r2, g2, b2, fr, fg, fb;
	switch (style) {
		case 'd': /* dark */
			r1 = 0.1; r2 = 0.4; fr = 0.9;
			g1 = 0.1; g2 = 0.4; fg = 0.9;G_GNUC_END_IGNORE_DEPRECATIONS
			b1 = 0.1; b2 = 0.4; fb = 0.9;
			break;
		case 'l': /* light */
			r1 = 1.0; r2 = 0.7; fr = 0.1;
			g1 = 1.0; g2 = 0.7; fg = 0.1;
			b1 = 1.0; b2 = 0.7; fb = 0.1;
			break;
		default:
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
		
		fr = 0.1;
		fg = 0.1;
		fb = 0.1;
		break;
	}
	if (temp > 80 ) {
		r1 = 0.9; r2 = 0.7;
		g1 = 0.4; g2 = 0.4;
		b1 = 0.4; b2 = 0.4;		
	}
	if (temp > 99) {
		font_size = 10.0;
		deg_size = 6.0;
	}
	
	double width = 24;
	double height = 24;
	cairo_surface_t *cs;
	cs = cairo_image_surface_create 
							(CAIRO_FORMAT_ARGB32, width, height);
	cairo_t *c;
	c = cairo_create(cs);
	cairo_select_font_face(c, "Sans", CAIRO_FONT_SLANT_NORMAL,
			CAIRO_FONT_WEIGHT_NORMAL);
	cairo_font_face_t *f = cairo_get_font_face(c);
	cairo_font_face_t *fnt = cairo_font_face_reference(f);
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
	cairo_set_font_size(c, font_size);
	cairo_move_to(c, 1.0, 17.0);
	cairo_set_source_rgb(c, fr, fg, fb);
	cairo_show_text(c, s);
	cairo_set_font_size(c, deg_size);
	cairo_move_to(c, 18.0, 11.0);
	cairo_set_source_rgb(c, fr, fg, fb);
	cairo_show_text(c, deg);
	cairo_surface_write_to_png (cs, temp_icon);
	/* hanlde destruction of font_face */
	unsigned int ref_cnt1 = cairo_font_face_get_reference_count(fnt);
	while (ref_cnt1 > 1) {
		cairo_font_face_destroy(fnt);
		ref_cnt1--;
	}
	cairo_pattern_destroy(linear);
	cairo_surface_destroy(cs);
	cairo_destroy(c);
	return temp;
}

/* tooltip */
char *split_string(char *var) {
	char *buf = var;
	const char x[2] = ":";
	char *token;
	token = strtok(buf, x);
	token = strtok(NULL , x);
	return token;
}

/* get number of processors */
unsigned short num_procs() {
	FILE *fp;
	char line[512];
	unsigned short proc_out = 0;
	fp = fopen(CPU_FILE, "r");
	if ( fp != NULL ) {
		while (fgets(line, sizeof(line), fp)) {
			if (strncmp(line, "processor", 9) == 0) {
				proc_out++;
			}
		}
	}
	fclose(fp);
	return proc_out;
}

gchar *get_tt(unsigned short num){
	FILE *fp;
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
	#ifdef HAVE_MENU
		tooltip_out = g_strdup_printf("%s %s\n%s = %d\n%s",proc, temperature, num_processors, x_procs, freq_out);
	#else
		tooltip_out = g_strdup_printf("%s %s\n%s %s\n%s = %d\n%s",PRG, VER, proc, temperature, num_processors, x_procs, freq_out);
	#endif
	return tooltip_out;
}
/* end tooltip */

gboolean Update(gpointer ptr) {
	/* refreshes the icon */
	paint_icon(style);
	if (first_run == 0) {
		sleep(1);
		first_run = 1;
	}
	x_procs = num_procs();
	tool_tip = get_tt(x_procs);
#ifdef HAVE_GTK3
G_GNUC_BEGIN_IGNORE_DEPRECATIONS
	gtk_status_icon_set_tooltip_text(tray_icon, tool_tip);
G_GNUC_END_IGNORE_DEPRECATIONS
#else
	gtk_status_icon_set_tooltip(tray_icon, tool_tip);
#endif /* HAVE_GTK3 */
G_GNUC_BEGIN_IGNORE_DEPRECATIONS
	temp_pixbuf = gdk_pixbuf_new_from_file(temp_icon,&gerror);
	gtk_status_icon_set_from_pixbuf(tray_icon,temp_pixbuf);
G_GNUC_END_IGNORE_DEPRECATIONS
	g_object_unref(temp_pixbuf);
	g_free(tooltip_out);
	return 1;
}

#ifdef HAVE_MENU
void  view_popup_menu_about() {
	show_about(0, NULL); /* "About - menu */
}
#endif /* HAVE_MENU */

#ifdef HAVE_SENSORS

void view_sensors() {
	sensor_gui(0, NULL);
}
#endif /* HAVE_SENSORS */

#ifdef HAVE_HELP

void view_help() {
	FILE *fp;
	fp = popen("mdview /usr/share/pmcputemp '' '' 'pmcputemp help' &", "r");
	if (!fp) {
		fprintf(stderr, "Failed to open help file\n");
		return;
	}
	pclose(fp);
}
#endif /* HAVE_HELP */

#ifdef HAVE_MENU
void  quit() {
    gtk_main_quit();
    exit (EXIT_SUCCESS);
}
#endif /* HAVE_MENU */

#ifdef HAVE_MENU
void tray_icon_on_menu(GtkStatusIcon *status_icon, guint button,guint activate_time, gpointer user_data) {
	/* right click menu */
	GtkWidget *menu, *menuitem, *iconw;
	menu = gtk_menu_new();
	
	#ifdef HAVE_HELP
		menuitem = gtk_image_menu_item_new_with_label(_("Help"));
		#ifdef HAVE_GTK3
		iconw = gtk_image_new_from_icon_name("help-contents", GTK_ICON_SIZE_MENU);
		#else
G_GNUC_BEGIN_IGNORE_DEPRECATIONS
		iconw = gtk_image_new_from_stock(GTK_STOCK_HELP, GTK_ICON_SIZE_MENU);
G_GNUC_END_IGNORE_DEPRECATIONS
	    #endif /* HAVE_GTK3 */
G_GNUC_BEGIN_IGNORE_DEPRECATIONS
	    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuitem), iconw);
G_GNUC_END_IGNORE_DEPRECATIONS
	    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
		g_signal_connect(menuitem, "activate", (GCallback) view_help, status_icon);
	#endif /* HAVE_HELP */
	
	#ifdef HAVE_SENSORS
G_GNUC_BEGIN_IGNORE_DEPRECATIONS
		menuitem = gtk_image_menu_item_new_with_label(_("Info"));
G_GNUC_END_IGNORE_DEPRECATIONS
		#ifdef HAVE_GTK3
		iconw = gtk_image_new_from_icon_name("dialog-information", GTK_ICON_SIZE_MENU);
		#else
		iconw = gtk_image_new_from_stock(GTK_STOCK_INFO, GTK_ICON_SIZE_MENU);
	    #endif /* HAVE_GTK3 */
G_GNUC_BEGIN_IGNORE_DEPRECATIONS
	    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuitem), iconw);
G_GNUC_END_IGNORE_DEPRECATIONS
	    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
		g_signal_connect(menuitem, "activate", (GCallback) view_sensors, status_icon);
	#endif /* HAVE_SENSORS */
	
G_GNUC_BEGIN_IGNORE_DEPRECATIONS
	menuitem = gtk_image_menu_item_new_with_label(_("About"));
G_GNUC_END_IGNORE_DEPRECATIONS
	#ifdef HAVE_GTK3
G_GNUC_BEGIN_IGNORE_DEPRECATIONS
		iconw = gtk_image_new_from_icon_name("help-about", GTK_ICON_SIZE_MENU);
G_GNUC_END_IGNORE_DEPRECATIONS
	#else
		iconw = gtk_image_new_from_stock(GTK_STOCK_ABOUT, GTK_ICON_SIZE_MENU);
	#endif /* HAVE_GTK3 */
G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuitem), iconw);
G_GNUC_END_IGNORE_DEPRECATIONS
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	g_signal_connect(menuitem, "activate", (GCallback) view_popup_menu_about, status_icon);
	
G_GNUC_BEGIN_IGNORE_DEPRECATIONS
	menuitem = gtk_image_menu_item_new_with_label(_("Quit"));
G_GNUC_END_IGNORE_DEPRECATIONS
	#ifdef HAVE_GTK3    
G_GNUC_BEGIN_IGNORE_DEPRECATIONS
	    iconw = gtk_image_new_from_icon_name("application-exit", GTK_ICON_SIZE_MENU);
G_GNUC_END_IGNORE_DEPRECATIONS
	#else
	    iconw = gtk_image_new_from_stock(GTK_STOCK_QUIT, GTK_ICON_SIZE_MENU);
	#endif /* HAVE_GTK3 */
G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuitem), iconw);
G_GNUC_END_IGNORE_DEPRECATIONS
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
    g_signal_connect(menuitem, "activate", (GCallback) quit, status_icon);
	
	gtk_widget_show_all(menu);
G_GNUC_BEGIN_IGNORE_DEPRECATIONS
	gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, button, gtk_get_current_event_time());
G_GNUC_END_IGNORE_DEPRECATIONS
}
#endif /* HAVE_MENU */

static GtkStatusIcon *create_tray_icon() {
	if (first_run == 0) {
		sleep(1);
		first_run = 1;
	}
G_GNUC_BEGIN_IGNORE_DEPRECATIONS
	tray_icon = gtk_status_icon_new();
G_GNUC_END_IGNORE_DEPRECATIONS
	#ifdef HAVE_MENU
		g_signal_connect(G_OBJECT(tray_icon), "popup-menu", G_CALLBACK(tray_icon_on_menu), NULL);
	#endif /* HAVE_MENU */
        
	temp_pixbuf = gdk_pixbuf_new_from_file(temp_icon,&gerror);
G_GNUC_BEGIN_IGNORE_DEPRECATIONS
	gtk_status_icon_set_from_pixbuf(tray_icon,temp_pixbuf);
G_GNUC_END_IGNORE_DEPRECATIONS
	g_object_unref(temp_pixbuf);						   
G_GNUC_BEGIN_IGNORE_DEPRECATIONS
	gtk_status_icon_set_visible(tray_icon, TRUE);
G_GNUC_END_IGNORE_DEPRECATIONS

	return tray_icon;
}

int main(int argc, char **argv) {
	setlocale( LC_ALL, "" ); 
	bindtextdomain( "pmcputemp", "/usr/share/locale" ); 
	textdomain( "pmcputemp" );
	int i;
	if (argc < 5) { /* only accepts 1 - 10 and/or 'l' or 'd' */
		for (i = 1; i < argc; i++) {
			switch (argv[i][0]) {
				case 'd':
				style='d';
				break;
				case 'l':
				style='l';
				break;
				default:
				if (argv[i][1]) { /* test if module specified on cli */
					if (module != NULL) {
						fprintf(stderr, 
							"kernel module %s is already specified\n", module);
						break;
					} else {
						module = argv[i];
					}
				} else {
					interval = atoi(argv[i]);
					if ((interval < 1) || (interval > 10)) {
					fprintf(stderr,_("Polling interval out of range. Use 1 to 10\n"
						"Using default 5 seconds\n"));
						interval = 5;
					}
					interval = interval * 1000; /* millisecs */
					break;
				}
			}
		}
	} else {
		fprintf(stderr,"Too many arguments, loading defaults\n");
	}
	
	temp_icon = g_build_filename(g_get_user_runtime_dir(), ICON, NULL);
	configdir = g_build_filename(g_get_user_config_dir(), CONFDIR, NULL);
	mkdir(configdir, 0755);
	conf = g_build_filename(g_get_user_config_dir(), CONFDIR, CONF, NULL);
	
	paint_icon(style); /* needed to kick it off */
	gtk_init(&argc, &argv);
	create_tray_icon();
	Update(NULL); /* needed to kick it off */
	g_timeout_add(interval, Update, NULL); /*update after 'interval' secs (default 5)*/
	gtk_main();
	
	g_free(temp_icon);
	g_free(configdir);
	g_free(conf);
	
	return 0;
}

