/*
 * simple-brightness-applet - simple statusarea applet that allows the user to select the brightness from a range of 1-5
 *
 * Copyright (c) 2009 Faheem Pervez <trippin1@gmail.com>
 *
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "simple-brightness-applet.h"

#include <string.h>
#include <gtk/gtk.h>
#include <hildon/hildon.h>
#include <gconf/gconf-client.h>
#include <libosso.h>

#ifdef HILDON_DISABLE_DEPRECATED
#undef HILDON_DISABLE_DEPRECATED
#endif

HD_DEFINE_PLUGIN_MODULE (SimpleBrightnessApplet, simple_brightness_applet, HD_TYPE_STATUS_MENU_ITEM)

#define SIMPLE_BRIGHTNESS_GCONF_PATH "/system/osso/dsm/display"
#define BRIGHTNESS_KEY SIMPLE_BRIGHTNESS_GCONF_PATH "/display_brightness"

#define SIMPLE_BRIGHTNESS_APPLET_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
                        SIMPLE_TYPE_BRIGHTNESS_APPLET, SimpleBrightnessAppletPrivate))

typedef struct _SimpleBrightnessAppletPrivate SimpleBrightnessAppletPrivate;

struct _SimpleBrightnessAppletPrivate
{
    GConfClient *gconf_client;
    GtkHBox *applet_contents;
    HildonControlbar *brightness_ctrlbar;
    GtkWidget *settings_dialog;
    GtkWidget *dispchkbtn;
    GtkWidget *applet_button;

    gulong brightness_ctrlbar_valchanged_id;
    gulong dispchkbtn_toggled_id;
    guint gconfnotify_id;
    guint display_keepalive_timeout;

    gboolean dispchkbtn_active;

    osso_context_t *osso_context;
};

static void simple_brightness_applet_finalize (GObject *object);

/* Callbacks: */
static gboolean simple_brightness_applet_keep_backlight_alive (SimpleBrightnessApplet *plugin)
{
   SimpleBrightnessAppletPrivate *priv = SIMPLE_BRIGHTNESS_APPLET_GET_PRIVATE (plugin);

   g_return_val_if_fail(priv->osso_context, FALSE);

   osso_display_state_on(priv->osso_context);
   if (osso_display_blanking_pause(priv->osso_context) != OSSO_OK)
	return FALSE;
   else
	return TRUE;
}

static void simple_brightness_applet_on_gconf_value_changed (GConfClient *gconf_client G_GNUC_UNUSED, guint cnxn_id G_GNUC_UNUSED, GConfEntry *entry G_GNUC_UNUSED, SimpleBrightnessApplet *plugin)
{
   SimpleBrightnessAppletPrivate *priv = SIMPLE_BRIGHTNESS_APPLET_GET_PRIVATE (plugin);

   g_signal_handler_block (priv->brightness_ctrlbar, priv->brightness_ctrlbar_valchanged_id);
   hildon_controlbar_set_value (priv->brightness_ctrlbar, gconf_client_get_int(priv->gconf_client, BRIGHTNESS_KEY, NULL));
   g_signal_handler_unblock (priv->brightness_ctrlbar, priv->brightness_ctrlbar_valchanged_id);
}

static void simple_brightness_applet_on_value_changed (HildonControlbar *brightness_ctrlbar G_GNUC_UNUSED, SimpleBrightnessApplet *plugin)
{
   SimpleBrightnessAppletPrivate *priv = SIMPLE_BRIGHTNESS_APPLET_GET_PRIVATE (plugin);

   gconf_client_set_int (priv->gconf_client, BRIGHTNESS_KEY, hildon_controlbar_get_value (priv->brightness_ctrlbar), NULL);
}

static void simple_brightness_applet_on_settings_button_clicked (GtkWidget *button, SimpleBrightnessApplet *plugin)
{
   SimpleBrightnessAppletPrivate *priv = SIMPLE_BRIGHTNESS_APPLET_GET_PRIVATE (plugin);
   gboolean failed = FALSE;

   gtk_widget_hide(priv->settings_dialog);

   if (priv->osso_context)
   {
   	if (osso_cp_plugin_execute(priv->osso_context, "libcpdisplay.so", NULL, TRUE) != OSSO_OK)
		failed = TRUE;
   }
   else
   {
	failed = TRUE;
   }

   gtk_widget_destroy(priv->settings_dialog);

   if(failed)
	hildon_banner_show_information(NULL, NULL, "Failed to show display settings");
}

static void simple_brightness_applet_on_dispchkbtn_toggled (GtkWidget *button G_GNUC_UNUSED, SimpleBrightnessApplet *plugin)
{
   SimpleBrightnessAppletPrivate *priv = SIMPLE_BRIGHTNESS_APPLET_GET_PRIVATE (plugin);
   GdkPixbuf *pixbuf = NULL;

   priv->dispchkbtn_active = hildon_check_button_get_active (HILDON_CHECK_BUTTON(priv->dispchkbtn));

   g_signal_handler_block (priv->dispchkbtn, priv->dispchkbtn_toggled_id);
   
   gtk_widget_hide(priv->settings_dialog);

   if (priv->dispchkbtn_active)
   {
	simple_brightness_applet_keep_backlight_alive(plugin);
	priv->display_keepalive_timeout = g_timeout_add_seconds(50, (GSourceFunc) simple_brightness_applet_keep_backlight_alive, plugin);
	if (priv->display_keepalive_timeout == 0)
	{
		hildon_banner_show_information(NULL, NULL, "Failed to keep backlight on");
		priv->dispchkbtn_active = FALSE;
		hildon_check_button_set_active(HILDON_CHECK_BUTTON(priv->dispchkbtn), priv->dispchkbtn_active);
		g_signal_handler_unblock (priv->dispchkbtn, priv->dispchkbtn_toggled_id);
		return;
	}

        gtk_widget_set_name(priv->applet_button, "hildon-reject-button-finger");
   	pixbuf = gtk_icon_theme_load_icon (gtk_icon_theme_get_default(), "general_brightness", 18, 0, NULL);
   	if (pixbuf)
   	{
		hd_status_plugin_item_set_status_area_icon (HD_STATUS_PLUGIN_ITEM (plugin), pixbuf);
   		g_object_unref (pixbuf);
		pixbuf = NULL;
   	}
   }
   else
   {
	if(g_source_remove(priv->display_keepalive_timeout))
	{
		gtk_widget_set_name(priv->applet_button, "GtkButton-finger");
		hd_status_plugin_item_set_status_area_icon (HD_STATUS_PLUGIN_ITEM (plugin), NULL);
		priv->display_keepalive_timeout = 0;
	}
	else
	{
		priv->dispchkbtn_active = TRUE;
		hildon_check_button_set_active(HILDON_CHECK_BUTTON(priv->dispchkbtn), priv->dispchkbtn_active);
	}		
   }

   g_signal_handler_unblock (priv->dispchkbtn, priv->dispchkbtn_toggled_id);
   gtk_widget_destroy(priv->settings_dialog);
}

/* Setup applet and dialogs: */
static void simple_brightness_applet_on_button_clicked (GtkWidget *button, SimpleBrightnessApplet *plugin)
{
   SimpleBrightnessAppletPrivate *priv = SIMPLE_BRIGHTNESS_APPLET_GET_PRIVATE (plugin);
   GtkWidget *toplevel, *settings_button;
   GdkPixbuf *pixbuf = NULL;

   toplevel = gtk_widget_get_toplevel(button);
   gtk_widget_hide (toplevel);

   priv->settings_dialog = gtk_dialog_new ();
   gtk_window_set_title (GTK_WINDOW (priv->settings_dialog), "Simple Brightness Applet:");
   gtk_window_set_transient_for(GTK_WINDOW(priv->settings_dialog), GTK_WINDOW(toplevel));
   gtk_window_set_destroy_with_parent (GTK_WINDOW (priv->settings_dialog), TRUE);

   settings_button = hildon_button_new(HILDON_SIZE_FINGER_HEIGHT | HILDON_SIZE_AUTO_WIDTH, HILDON_BUTTON_ARRANGEMENT_HORIZONTAL);
   hildon_button_set_title(HILDON_BUTTON (settings_button), "Open Display Settings...");
   pixbuf = gtk_icon_theme_load_icon (gtk_icon_theme_get_default(), "general_brightness", 64, 0, NULL);
   if (pixbuf)
   {
	GtkWidget *image;
   	image = gtk_image_new_from_pixbuf (pixbuf);
   	g_object_unref (pixbuf);
	pixbuf = NULL;
   	hildon_button_set_image(HILDON_BUTTON(settings_button), image);
   }
   gtk_box_pack_start(GTK_BOX(GTK_DIALOG(priv->settings_dialog)->vbox), settings_button, TRUE, TRUE, 0);
   g_signal_connect(settings_button, "clicked", G_CALLBACK(simple_brightness_applet_on_settings_button_clicked), plugin);

   priv->dispchkbtn = hildon_check_button_new(HILDON_SIZE_FINGER_HEIGHT | HILDON_SIZE_AUTO_WIDTH);
   gtk_button_set_label(GTK_BUTTON (priv->dispchkbtn), "Keep display on");
   gtk_button_set_alignment(GTK_BUTTON(priv->dispchkbtn), 0.5f, 0.5f);
   hildon_check_button_set_active(HILDON_CHECK_BUTTON(priv->dispchkbtn), priv->dispchkbtn_active);
   gtk_box_pack_start(GTK_BOX(GTK_DIALOG(priv->settings_dialog)->vbox), priv->dispchkbtn, TRUE, TRUE, 0);
   priv->dispchkbtn_toggled_id = g_signal_connect(priv->dispchkbtn, "toggled", G_CALLBACK(simple_brightness_applet_on_dispchkbtn_toggled), plugin); 

   gtk_widget_show_all(priv->settings_dialog);
}

static void simple_brightness_applet_setup (SimpleBrightnessApplet *plugin)
{
   GdkPixbuf *pixbuf = NULL;
   SimpleBrightnessAppletPrivate *priv = SIMPLE_BRIGHTNESS_APPLET_GET_PRIVATE (plugin);

   priv->applet_contents = GTK_HBOX (gtk_hbox_new (FALSE, HILDON_MARGIN_DEFAULT));

   priv->applet_button = hildon_gtk_button_new(HILDON_SIZE_FINGER_HEIGHT | HILDON_SIZE_AUTO_WIDTH);
   gtk_box_pack_start (GTK_BOX (priv->applet_contents), priv->applet_button, FALSE, FALSE, 0);
   g_signal_connect (priv->applet_button, "clicked", G_CALLBACK(simple_brightness_applet_on_button_clicked), plugin);
   pixbuf = gtk_icon_theme_load_icon (gtk_icon_theme_get_default(), "general_brightness", 64, 0, NULL);
   if (pixbuf)
   {
	GtkWidget *image;
   	image = gtk_image_new_from_pixbuf (pixbuf);
   	g_object_unref (pixbuf);
	pixbuf = NULL;
   	gtk_button_set_image(GTK_BUTTON(priv->applet_button), image);
   }

   priv->brightness_ctrlbar = HILDON_CONTROLBAR (hildon_controlbar_new ()); /* Yes, I know: I'm very naughty for using a depreciated widget. But let me know, Nokia, when you actually stop using it too. */
   hildon_gtk_widget_set_theme_size(GTK_WIDGET(priv->brightness_ctrlbar), HILDON_SIZE_FINGER_HEIGHT | HILDON_SIZE_AUTO_WIDTH);
   hildon_controlbar_set_range (priv->brightness_ctrlbar, gconf_client_get_int(priv->gconf_client, "/system/osso/dsm/display/display_brightness_level_step", NULL), gconf_client_get_int(priv->gconf_client, "/system/osso/dsm/display/max_display_brightness_levels", NULL));
   hildon_controlbar_set_value (priv->brightness_ctrlbar, gconf_client_get_int(priv->gconf_client, BRIGHTNESS_KEY, NULL));
   priv->brightness_ctrlbar_valchanged_id = g_signal_connect (priv->brightness_ctrlbar, "value-changed", G_CALLBACK(simple_brightness_applet_on_value_changed), plugin);
   gtk_box_pack_start (GTK_BOX (priv->applet_contents), GTK_WIDGET (priv->brightness_ctrlbar), TRUE, TRUE, 0);

   gconf_client_add_dir (priv->gconf_client, SIMPLE_BRIGHTNESS_GCONF_PATH, GCONF_CLIENT_PRELOAD_NONE, NULL);
   priv->gconfnotify_id = gconf_client_notify_add (priv->gconf_client, BRIGHTNESS_KEY, (GConfClientNotifyFunc)simple_brightness_applet_on_gconf_value_changed, plugin, NULL, NULL);
}

/* GObject stuff: */
static void simple_brightness_applet_class_init (SimpleBrightnessAppletClass *class)
{
   GObjectClass *object_class = G_OBJECT_CLASS (class);

   object_class->finalize = simple_brightness_applet_finalize;

   g_type_class_add_private (class, sizeof (SimpleBrightnessAppletPrivate));
}

static void simple_brightness_applet_class_finalize (SimpleBrightnessAppletClass *class G_GNUC_UNUSED)
{
}

static void simple_brightness_applet_init (SimpleBrightnessApplet *plugin)
{
   SimpleBrightnessAppletPrivate *priv = SIMPLE_BRIGHTNESS_APPLET_GET_PRIVATE (plugin);
   memset (priv, 0, sizeof (SimpleBrightnessAppletPrivate));

   priv->dispchkbtn_active = FALSE;
   priv->osso_context = osso_initialize(PACKAGE, PACKAGE_VERSION, TRUE, NULL);
   priv->brightness_ctrlbar_valchanged_id = priv->dispchkbtn_toggled_id = priv->gconfnotify_id = priv->display_keepalive_timeout = 0;
   priv->gconf_client = gconf_client_get_default ();

   simple_brightness_applet_setup (plugin);
   gtk_container_add (GTK_CONTAINER (plugin), GTK_WIDGET(priv->applet_contents));
   gtk_widget_show_all (GTK_WIDGET(plugin));
}

static void simple_brightness_applet_finalize (GObject *object)
{
   SimpleBrightnessApplet *plugin = SIMPLE_BRIGHTNESS_APPLET (object);
   SimpleBrightnessAppletPrivate *priv = SIMPLE_BRIGHTNESS_APPLET_GET_PRIVATE (plugin);

   if (priv->brightness_ctrlbar_valchanged_id != 0)
   {
	g_signal_handler_disconnect (priv->brightness_ctrlbar, priv->brightness_ctrlbar_valchanged_id);
        priv->brightness_ctrlbar_valchanged_id = 0;
   }

   if (priv->dispchkbtn_toggled_id != 0)
   {
	g_signal_handler_disconnect (priv->dispchkbtn, priv->dispchkbtn_toggled_id);
        priv->dispchkbtn_toggled_id = 0;
   }

   if (priv->display_keepalive_timeout != 0)
   {
	g_source_remove(priv->display_keepalive_timeout);
	priv->display_keepalive_timeout = 0;
   }

   if (priv->gconfnotify_id != 0)
   {
   	gconf_client_notify_remove (priv->gconf_client, priv->gconfnotify_id);
   	priv->gconfnotify_id = 0;
	gconf_client_remove_dir (priv->gconf_client, SIMPLE_BRIGHTNESS_GCONF_PATH, NULL);
   }

   if (priv->gconf_client)
   {
   	gconf_client_clear_cache (priv->gconf_client);
   	g_object_unref (priv->gconf_client);
   	priv->gconf_client = NULL;
   }

   if (priv->osso_context)
   {
	osso_deinitialize(priv->osso_context);
        priv->osso_context = NULL;
   }

   G_OBJECT_CLASS (simple_brightness_applet_parent_class)->finalize (object);
}

SimpleBrightnessApplet* simple_brightness_applet_new (void)
{
  return g_object_new (SIMPLE_TYPE_BRIGHTNESS_APPLET, NULL);
}
