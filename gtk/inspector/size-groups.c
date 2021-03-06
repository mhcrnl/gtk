/*
 * Copyright (c) 2014 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#include "size-groups.h"
#include "window.h"

#include "gtkcomboboxtext.h"
#include "gtkframe.h"
#include "gtklabel.h"
#include "gtklistbox.h"
#include "gtksizegroup.h"
#include "gtkswitch.h"
#include "gtkwidgetprivate.h"


typedef struct {
  GtkListBoxRow parent;
  GtkWidget *widget;
} SizeGroupRow;

typedef struct {
  GtkListBoxRowClass parent;
} SizeGroupRowClass;

enum {
  PROP_WIDGET = 1,
  LAST_PROPERTY
};

GParamSpec *properties[LAST_PROPERTY] = { NULL };

GType size_group_row_get_type (void);

G_DEFINE_TYPE (SizeGroupRow, size_group_row, GTK_TYPE_LIST_BOX_ROW)

static void
size_group_row_init (SizeGroupRow *row)
{
}

static void
size_group_row_get_property (GObject    *object,
                             guint       property_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  SizeGroupRow *row = (SizeGroupRow*)object;

  switch (property_id)
    {
    case PROP_WIDGET:
      g_value_set_pointer (value, row->widget);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
size_group_row_widget_destroyed (GtkWidget *widget, SizeGroupRow *row)
{
  GtkWidget *parent;

  parent = gtk_widget_get_parent (GTK_WIDGET (row));
  if (parent)
    gtk_container_remove (GTK_CONTAINER (parent), GTK_WIDGET (row));
}

static void
set_widget (SizeGroupRow *row, GtkWidget *widget)
{
  if (row->widget)
    g_signal_handlers_disconnect_by_func (row->widget,
                                          size_group_row_widget_destroyed, row);

  row->widget = widget;

  if (row->widget)
    g_signal_connect (row->widget, "destroy",
                      G_CALLBACK (size_group_row_widget_destroyed), row);
}

static void
size_group_row_set_property (GObject      *object,
                             guint         property_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  SizeGroupRow *row = (SizeGroupRow*)object;

  switch (property_id)
    {
    case PROP_WIDGET:
      set_widget (row, g_value_get_pointer (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}
static void
size_group_row_finalize (GObject *object)
{
  SizeGroupRow *row = (SizeGroupRow *)object;

  set_widget (row, NULL);

  G_OBJECT_CLASS (size_group_row_parent_class)->finalize (object);
}

static void
size_group_state_flags_changed (GtkWidget     *widget,
                                GtkStateFlags  old_state)
{
  SizeGroupRow *row = (SizeGroupRow*)widget;
  GtkStateFlags state;

  if (!row->widget)
    return;

  state = gtk_widget_get_state_flags (widget);
  if ((state & GTK_STATE_FLAG_PRELIGHT) != (old_state & GTK_STATE_FLAG_PRELIGHT))
    {
      if (state & GTK_STATE_FLAG_PRELIGHT)
        gtk_inspector_start_highlight (row->widget);
      else
        gtk_inspector_stop_highlight (row->widget);
    }
}

static void
size_group_row_class_init (SizeGroupRowClass *class)
{
  GObjectClass *object_class = G_OBJECT_CLASS (class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (class);

  object_class->finalize = size_group_row_finalize;
  object_class->get_property = size_group_row_get_property;
  object_class->set_property = size_group_row_set_property;

  widget_class->state_flags_changed = size_group_state_flags_changed;

  properties[PROP_WIDGET] =
    g_param_spec_pointer ("widget", "Widget", "Widget", G_PARAM_READWRITE);

  g_object_class_install_properties (object_class, LAST_PROPERTY, properties);

}

G_DEFINE_TYPE (GtkInspectorSizeGroups, gtk_inspector_size_groups, GTK_TYPE_BOX)

static void
clear_view (GtkInspectorSizeGroups *sl)
{
  GList *children, *l;
  GtkWidget *child;

  children = gtk_container_get_children (GTK_CONTAINER (sl));
  for (l = children; l; l = l->next)
    {
      child = l->data;
      gtk_container_remove (GTK_CONTAINER (sl), child);
    }
  g_list_free (children);
}

static void
add_widget (GtkInspectorSizeGroups *sl,
            GtkListBox             *listbox,
            GtkWidget              *widget)
{
  GtkWidget *row;
  GtkWidget *label;
  gchar *text;

  row = g_object_new (size_group_row_get_type (), "widget", widget, NULL);
  text = g_strdup_printf ("%p (%s)", widget, g_type_name_from_instance ((GTypeInstance*)widget));
  label = gtk_label_new (text);
  g_free (text);
  g_object_set (label, "margin", 10, NULL);
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_BASELINE);
  gtk_widget_show (label);
  gtk_container_add (GTK_CONTAINER (row), label);
  gtk_container_add (GTK_CONTAINER (listbox), row);
}

static void
add_size_group (GtkInspectorSizeGroups *sl,
                GtkSizeGroup           *group)
{
  GtkWidget *frame, *box, *box2;
  GtkWidget *label, *combo;
  GSList *widgets, *l;
  GtkWidget *listbox;

  frame = gtk_frame_new (NULL);
  gtk_container_add (GTK_CONTAINER (sl), frame);
  box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_style_context_add_class (gtk_widget_get_style_context (box), GTK_STYLE_CLASS_VIEW);
  gtk_container_add (GTK_CONTAINER (frame), box);

  box2 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 10);
  gtk_container_add (GTK_CONTAINER (box), box2);

  label = gtk_label_new (_("Mode"));
  g_object_set (label, "margin", 10, NULL);
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_BASELINE);
  gtk_box_pack_start (GTK_BOX (box2), label, TRUE, TRUE);

  combo = gtk_combo_box_text_new ();
  g_object_set (combo, "margin", 10, NULL);
  gtk_widget_set_halign (combo, GTK_ALIGN_END);
  gtk_widget_set_valign (combo, GTK_ALIGN_BASELINE);
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (combo), C_("sizegroup mode", "None"));
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (combo), C_("sizegroup mode", "Horizontal"));
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (combo), C_("sizegroup mode", "Vertical"));
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (combo), C_("sizegroup mode", "Both"));
  g_object_bind_property (group, "mode",
                          combo, "active",
                          G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
  gtk_box_pack_start (GTK_BOX (box2), combo, FALSE, FALSE);

  listbox = gtk_list_box_new ();
  gtk_container_add (GTK_CONTAINER (box), listbox);
  gtk_list_box_set_selection_mode (GTK_LIST_BOX (listbox), GTK_SELECTION_NONE);

  widgets = gtk_size_group_get_widgets (group);
  for (l = widgets; l; l = l->next)
    add_widget (sl, GTK_LIST_BOX (listbox), GTK_WIDGET (l->data));
}

void
gtk_inspector_size_groups_set_object (GtkInspectorSizeGroups *sl,
                                      GObject                *object)
{
  GSList *groups, *l;

  clear_view (sl);

  if (!GTK_IS_WIDGET (object))
    {
      gtk_widget_hide (GTK_WIDGET (sl));
      return;
    }

  groups = _gtk_widget_get_sizegroups (GTK_WIDGET (object));
  if (groups)
    gtk_widget_show (GTK_WIDGET (sl));
  for (l = groups; l; l = l->next)
    {
      GtkSizeGroup *group = l->data;
      add_size_group (sl, group);
    }
}

static void
gtk_inspector_size_groups_init (GtkInspectorSizeGroups *sl)
{
  g_object_set (sl,
                "orientation", GTK_ORIENTATION_VERTICAL,
                "margin-start", 60,
                "margin-end", 60,
                "margin-bottom", 60,
                "margin-bottom", 30,
                "spacing", 10,
                NULL);
}

static void
gtk_inspector_size_groups_class_init (GtkInspectorSizeGroupsClass *klass)
{
}

// vim: set et sw=2 ts=2:
