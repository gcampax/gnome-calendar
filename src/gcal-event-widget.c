/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 2 -*- */
/*
 * gcal-event-widget.c
 * Copyright (C) 2012 Erick Pérez Castellanos <erickpc@gnome.org>
 *
 * gnome-calendar is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * gnome-calendar is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "gcal-event-widget.h"

struct _GcalEventWidgetPrivate
{
  GdkWindow *event_window;

  gchar     *summary;
  GdkRGBA   *color;
};

enum
{
  PROP_0,
  PROP_SUMMARY,
  PROP_COLOR
};

static void     gcal_event_widget_constructed          (GObject        *object);

static void     gcal_event_widget_set_property         (GObject        *object,
                                                        guint           property_id,
                                                        const GValue   *value,
                                                        GParamSpec     *pspec);

static void     gcal_event_widget_get_property         (GObject        *object,
                                                        guint           property_id,
                                                        GValue         *value,
                                                        GParamSpec     *pspec);

static void     gcal_event_widget_get_preferred_width  (GtkWidget      *widget,
                                                        gint           *minimum,
                                                        gint           *natural);

static void     gcal_event_widget_get_preferred_height (GtkWidget      *widget,
                                                        gint           *minimum,
                                                        gint           *natural);

static void     gcal_event_widget_realize              (GtkWidget      *widget);

static void     gcal_event_widget_unrealize            (GtkWidget      *widget);

static void     gcal_event_widget_map                  (GtkWidget      *widget);

static void     gcal_event_widget_unmap                (GtkWidget      *widget);

static void     gcal_event_widget_size_allocate        (GtkWidget      *widget,
                                                        GtkAllocation  *allocation);

static gboolean gcal_event_widget_draw                 (GtkWidget      *widget,
                                                        cairo_t        *cr);

static gboolean gcal_event_widget_button_press         (GtkWidget      *widget,
                                                        GdkEventButton *event);

G_DEFINE_TYPE(GcalEventWidget, gcal_event_widget, GTK_TYPE_WIDGET)

static void
gcal_event_widget_class_init(GcalEventWidgetClass *klass)
{
  GObjectClass *object_class;
  GtkWidgetClass *widget_class;

  object_class = G_OBJECT_CLASS (klass);
  object_class->constructed = gcal_event_widget_constructed;
  object_class->set_property = gcal_event_widget_set_property;
  object_class->get_property = gcal_event_widget_get_property;

  widget_class = GTK_WIDGET_CLASS (klass);
  widget_class->get_preferred_width = gcal_event_widget_get_preferred_width;
  widget_class->get_preferred_height = gcal_event_widget_get_preferred_height;
  widget_class->realize = gcal_event_widget_realize;
  widget_class->unrealize = gcal_event_widget_unrealize;
  widget_class->map = gcal_event_widget_map;
  widget_class->unmap = gcal_event_widget_unmap;
  widget_class->size_allocate = gcal_event_widget_size_allocate;
  widget_class->draw = gcal_event_widget_draw;
  widget_class->button_press_event = gcal_event_widget_button_press;

  g_object_class_install_property (object_class,
                                   PROP_SUMMARY,
                                   g_param_spec_string ("summary",
                                                        "Summary",
                                                        "The event summary",
                                                        NULL,
                                                        G_PARAM_CONSTRUCT |
                                                        G_PARAM_READWRITE));

  g_object_class_install_property (object_class,
                                   PROP_COLOR,
                                   g_param_spec_boxed ("color",
                                                       "Color",
                                                       "The color to render",
                                                       GDK_TYPE_RGBA,
                                                       G_PARAM_CONSTRUCT |
                                                       G_PARAM_READWRITE));

  g_type_class_add_private((gpointer)klass, sizeof(GcalEventWidgetPrivate));
}

static void gcal_event_widget_init(GcalEventWidget *self)
{
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE(self,
                                           GCAL_TYPE_EVENT_WIDGET,
                                           GcalEventWidgetPrivate);

  gtk_widget_set_has_window (GTK_WIDGET (self), FALSE);
  gtk_widget_set_can_focus (GTK_WIDGET (self), TRUE);
}

static void
gcal_event_widget_constructed (GObject *object)
{
  GcalEventWidgetPrivate *priv;

  priv = GCAL_EVENT_WIDGET (object)->priv;
  if (G_OBJECT_CLASS (gcal_event_widget_parent_class)->constructed != NULL)
    G_OBJECT_CLASS (gcal_event_widget_parent_class)->constructed (object);

  gtk_widget_override_background_color (GTK_WIDGET (object), 0, priv->color);
}

static void
gcal_event_widget_set_property (GObject      *object,
                                guint         property_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  GcalEventWidgetPrivate *priv;

  priv = GCAL_EVENT_WIDGET (object)->priv;

  switch (property_id)
    {
    case PROP_SUMMARY:
      if (priv->summary != NULL)
        g_free (priv->summary);

      priv->summary = g_value_dup_string (value);
      return;
    case PROP_COLOR:
      if (priv->color != NULL)
        gdk_rgba_free (priv->color);

      priv->color = g_value_dup_boxed (value);
      return;
    }

  G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
}

static void
gcal_event_widget_get_property (GObject      *object,
                                guint         property_id,
                                GValue       *value,
                                GParamSpec   *pspec)
{
  GcalEventWidgetPrivate *priv;

  priv = GCAL_EVENT_WIDGET (object)->priv;

  switch (property_id)
    {
    case PROP_SUMMARY:
      g_value_set_string (value, priv->summary);
      return;
    case PROP_COLOR:
      g_value_set_boxed (value, priv->color);
      return;
    }

  G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
}

static void
gcal_event_widget_get_preferred_width (GtkWidget *widget,
                                       gint      *minimum,
                                       gint      *natural)
{
  GtkBorder padding;
  PangoLayout *layout;
  PangoRectangle logical_rect;

  layout = gtk_widget_create_pango_layout (widget, "00:00:00 00:00");


  pango_layout_get_extents (layout, NULL, &logical_rect);
  pango_extents_to_pixels (&logical_rect, NULL);

  gtk_style_context_get_padding (gtk_widget_get_style_context (widget),
                                 gtk_widget_get_state_flags (widget),
                                 &padding);

  *minimum = *natural = logical_rect.width + padding.left + padding.right;

  g_object_unref (layout);
}

static void
gcal_event_widget_get_preferred_height (GtkWidget *widget,
                                        gint      *minimum,
                                        gint      *natural)
{
  GtkBorder padding;
  PangoLayout *layout;
  PangoRectangle logical_rect;

  layout = gtk_widget_create_pango_layout (widget, "00:00:00 00:00");


  pango_layout_get_extents (layout, NULL, &logical_rect);
  pango_extents_to_pixels (&logical_rect, NULL);

  gtk_style_context_get_padding (gtk_widget_get_style_context (widget),
                                 gtk_widget_get_state_flags (widget),
                                 &padding);

  *minimum = *natural = logical_rect.height + padding.top + padding.bottom;

  g_object_unref (layout);
}

static void
gcal_event_widget_realize (GtkWidget *widget)
{
  GcalEventWidgetPrivate *priv;
  GdkWindow *parent_window;
  GdkWindowAttr attributes;
  gint attributes_mask;
  GtkAllocation allocation;

  priv = GCAL_EVENT_WIDGET (widget)->priv;
  gtk_widget_set_realized (widget, TRUE);

  parent_window = gtk_widget_get_parent_window (widget);
  gtk_widget_set_window (widget, parent_window);
  g_object_ref (parent_window);

  gtk_widget_get_allocation (widget, &allocation);

  attributes.window_type = GDK_WINDOW_CHILD;
  attributes.wclass = GDK_INPUT_ONLY;
  attributes.x = allocation.x;
  attributes.y = allocation.y;
  attributes.width = allocation.width;
  attributes.height = allocation.height;
  attributes.event_mask = gtk_widget_get_events (widget);
  attributes.event_mask |= (GDK_BUTTON_PRESS_MASK |
                            GDK_BUTTON_RELEASE_MASK |
                            GDK_BUTTON1_MOTION_MASK |
                            GDK_POINTER_MOTION_HINT_MASK |
                            GDK_POINTER_MOTION_MASK |
                            GDK_ENTER_NOTIFY_MASK |
                            GDK_LEAVE_NOTIFY_MASK);
  attributes_mask = GDK_WA_X | GDK_WA_Y;

  priv->event_window = gdk_window_new (parent_window,
                                       &attributes,
                                       attributes_mask);
  gdk_window_set_user_data (priv->event_window, widget);
}

static void
gcal_event_widget_unrealize (GtkWidget *widget)
{
  GcalEventWidgetPrivate *priv;

  priv = GCAL_EVENT_WIDGET (widget)->priv;
  if (priv->event_window != NULL)
    {
      gdk_window_set_user_data (priv->event_window, NULL);
      gdk_window_destroy (priv->event_window);
      priv->event_window = NULL;
    }

  GTK_WIDGET_CLASS (gcal_event_widget_parent_class)->unrealize (widget);
}

static void
gcal_event_widget_map (GtkWidget *widget)
{
  GcalEventWidgetPrivate *priv;

  priv = GCAL_EVENT_WIDGET (widget)->priv;
  GTK_WIDGET_CLASS (gcal_event_widget_parent_class)->map (widget);

  if (priv->event_window)
    gdk_window_show (priv->event_window);
}

static void
gcal_event_widget_unmap (GtkWidget *widget)
{
  GcalEventWidgetPrivate *priv;

  priv = GCAL_EVENT_WIDGET (widget)->priv;
  if (priv->event_window)
    gdk_window_hide (priv->event_window);

  GTK_WIDGET_CLASS (gcal_event_widget_parent_class)->unmap (widget);
}

static void
gcal_event_widget_size_allocate (GtkWidget     *widget,
                                 GtkAllocation *allocation)
{
  GcalEventWidgetPrivate *priv;

  priv = GCAL_EVENT_WIDGET (widget)->priv;
  gtk_widget_set_allocation (widget, allocation);

  if (gtk_widget_get_realized (widget))
    gdk_window_move_resize (priv->event_window,
                            allocation->x,
                            allocation->y,
                            allocation->width,
                            allocation->height);
}

static gboolean
gcal_event_widget_draw (GtkWidget *widget,
                        cairo_t   *cr)
{
  GcalEventWidgetPrivate *priv;
  GtkStyleContext *context;
  GtkStateFlags state;
  GtkBorder padding;

  gint x,y;
  gint width, height;
  gdouble radius;
  gdouble degrees;

  PangoLayout *layout;
  GdkRGBA fg_color;

  priv = GCAL_EVENT_WIDGET (widget)->priv;
  context = gtk_widget_get_style_context (widget);
  state = gtk_widget_get_state_flags (widget);

  gtk_style_context_get_padding (context, state, &padding);
  gtk_style_context_get_color (context, state, &fg_color);

  x = padding.left;
  y = padding.top;
  width = gtk_widget_get_allocated_width (widget)
   - padding.left - padding.right;
  height = gtk_widget_get_allocated_height (widget)
   - padding.top - padding.bottom;
  radius = height / 10;
  degrees = G_PI / 180.0;

  layout = pango_cairo_create_layout (cr);
  pango_layout_set_font_description (layout,
                                     gtk_style_context_get_font (context,
                                                                 state));
  pango_layout_set_ellipsize (layout, PANGO_ELLIPSIZE_END);
  pango_layout_set_width (layout, (width - 2 * radius ) * PANGO_SCALE);

  cairo_save (cr);

  cairo_arc (cr, x + width - radius, y + radius, radius, -90 * degrees, 0 * degrees);
  cairo_arc (cr, x + width - radius, y + height - radius, radius, 0 * degrees, 90 * degrees);
  cairo_arc (cr, x + radius, y + height - radius, radius, 90 * degrees, 180 * degrees);
  cairo_arc (cr, x + radius, y + radius, radius, 180 * degrees, 270 * degrees);
  cairo_set_source_rgba (cr,
                         priv->color->red,
                         priv->color->green,
                         priv->color->blue,
                         priv->color->alpha);
  cairo_fill (cr);

  cairo_set_source_rgba (cr,
                         fg_color.red,
                         fg_color.green,
                         fg_color.blue,
                         fg_color.alpha);
  pango_layout_set_text (layout, priv->summary, -1);
  pango_cairo_update_layout (cr, layout);
  cairo_move_to (cr, x + radius, y + radius);
  pango_cairo_show_layout (cr, layout);
  cairo_stroke (cr);

  cairo_restore (cr);

  g_object_unref (layout);

  if (GTK_WIDGET_CLASS (gcal_event_widget_parent_class)->draw != NULL)
    GTK_WIDGET_CLASS (gcal_event_widget_parent_class)->draw (widget, cr);

  return FALSE;
}

static gboolean
gcal_event_widget_button_press (GtkWidget      *widget,
                                GdkEventButton *event)
{
  g_debug ("[ITW]: Called button press virtual function\n");
  return FALSE;
}

GtkWidget*
gcal_event_widget_new ()
{
  return g_object_new (GCAL_TYPE_EVENT_WIDGET, NULL);
}

GtkWidget*
gcal_event_widget_new_with_summary_and_color (const gchar   *summary,
                                              const GdkRGBA *color)
{
  return g_object_new (GCAL_TYPE_EVENT_WIDGET,
                       "summary",
                       summary,
                       "color",
                       color,
                       NULL);
}
