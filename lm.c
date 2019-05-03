#include <gtk/gtk.h>
#include <stdio.h>
#include <libintl.h>
#include <locale.h>
#define _(STRING)    gettext(STRING)

FILE *fp;
char buf[1024] = "No data";

void read_sensors() {
	fp = popen("sensors 2>/dev/null", "r");
	if (!fp)
		return;
	int size = fread(buf, 1, sizeof(buf), fp);
	pclose(fp);
	if (size >= 1024)
		fprintf(stderr, "Too big to read\n");
		return;
}

int sensor_gui(gint argc, gchar *argv[]) {
	
	read_sensors();
	gchar *message = buf;
	GtkWidget *window;
	GtkWidget *label_head;
	GtkWidget *label;
    GtkWidget *box;
    
	gtk_init(&argc, &argv);
	
	gchar *title = (_("Sensors"));
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect( G_OBJECT( window ), "delete-event",
							G_CALLBACK( gtk_widget_destroy ), NULL );
	gtk_container_set_border_width(GTK_CONTAINER(window),10);
	gtk_window_set_title (GTK_WINDOW(window), title);
	gtk_widget_show  (window);
	
	/* Create a vbox and attach it to the window */ 
#ifdef HAVE_GTK3	
	box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	gtk_widget_set_size_request(box, 450, 150); 
#else
	box = gtk_vbox_new (FALSE, 0);
#endif /* HAVE_GTK3 */ 	
	gtk_container_add(GTK_CONTAINER(window), box); 
	gtk_widget_show(box);
    
	/* label widget */
	gchar *heading = (_("Output from lm_sensors"));
	
	label_head = gtk_label_new (heading);
	gtk_widget_modify_font (label_head,
				pango_font_description_from_string ("Sans 12"));
	label = gtk_label_new (message);
	gtk_widget_modify_font (label,
				pango_font_description_from_string ("Monospace 10"));
	gtk_label_set_line_wrap(GTK_LABEL(label_head), TRUE);
	gtk_box_pack_start (GTK_BOX (box), label_head, TRUE, TRUE, 5);
	gtk_widget_show(label_head);
	gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
	gtk_box_pack_start (GTK_BOX (box), label, TRUE, TRUE, 5);
	gtk_widget_show(label);
	
	gtk_main();
	
	return 0;
}

