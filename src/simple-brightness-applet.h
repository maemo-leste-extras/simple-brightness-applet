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

#ifndef SIMPLE_BRIGHTNESS_APPLET_H
#define SIMPLE_BRIGHTNESS_APPLET_H

#include <glib.h>
#include <glib-object.h>
#include <libhildondesktop/libhildondesktop.h>

G_BEGIN_DECLS

#define SIMPLE_TYPE_BRIGHTNESS_APPLET (simple_brightness_applet_get_type ())

#define SIMPLE_BRIGHTNESS_APPLET(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
                        SIMPLE_TYPE_BRIGHTNESS_APPLET, SimpleBrightnessApplet))

#define SIMPLE_BRIGHTNESS_APPLET_CLASS(class) (G_TYPE_CHECK_CLASS_CAST ((class), \
                        SIMPLE_TYPE_BRIGHTNESS_APPLET, SimpleBrightnessAppletClass))

#define SIMPLE_IS_BRIGHTNESS_APPLET(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                        SIMPLE_TYPE_BRIGHTNESS_APPLET))

#define SIMPLE_IS_BRIGHTNESS_APPLET_CLASS(class) (G_TYPE_CHECK_CLASS_TYPE ((class), \
                        SIMPLE_TYPE_BRIGHTNESS_APPLET))

#define SIMPLE_BRIGHTNESS_APPLET_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                        SIMPLE_TYPE_BRIGHTNESS_APPLET, SimpleBrightnessAppletClass))

typedef struct _SimpleBrightnessApplet SimpleBrightnessApplet;
typedef struct _SimpleBrightnessAppletClass SimpleBrightnessAppletClass;

struct _SimpleBrightnessApplet
{
    HDStatusMenuItem hitem;
};

struct _SimpleBrightnessAppletClass
{
    HDStatusMenuItemClass parent_class;
};

GType simple_brightness_applet_get_type (void);

SimpleBrightnessApplet* simple_brightness_applet_new (void);

G_END_DECLS

#endif /* SIMPLE_BRIGHTNESS_APPLET_H */

