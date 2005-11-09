/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2004 Jun Mukai
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */


#include <gtk/gtk.h>
#include <vector>


#include <scim.h>

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "scim_skk_intl.h"

using namespace scim;

namespace scim_skk {

extern bool __have_changed;
extern GtkWidget *__widget_sysdicts;

enum DictColumnType {
    DICT_TYPE_COLUMN, /* string: type of dictionary */
    DICT_NAME_COLUMN, /* string: dictionary name */
    DICT_N_COLUMNS
};

struct DictFileConfigWidgets {
    gchar *title;
    GtkWidget *widget;
    GtkWidget *entry;
    GtkWidget *button;
};

struct SKKServConfigWidgets {
    GtkWidget *widget;
    GtkWidget *hostname_entry;
    GtkWidget *port_entry;
};

static String __dict_type_names [] = {
    "DictFile",
    "SKKServ",
    ""
};

static DictFileConfigWidgets __widgets_dict_file;
static SKKServConfigWidgets __widgets_skkserv;
static GtkWidget *__combo_box_dict_types = NULL;


static void
file_selection_clicked_cb (GtkButton *button,
                           gpointer   user_data)
{
    DictFileConfigWidgets *data = static_cast <DictFileConfigWidgets *> (user_data);

    if (data) {
        GtkWidget *dialog = gtk_file_selection_new (_(data->title));
        gint result;

        gtk_file_selection_set_filename(GTK_FILE_SELECTION (dialog), gtk_entry_get_text(GTK_ENTRY(data->entry)));

        result = gtk_dialog_run (GTK_DIALOG (dialog));

        if (result == GTK_RESPONSE_OK) {
            const gchar *fname =
                gtk_file_selection_get_filename (GTK_FILE_SELECTION (dialog));

            if (!fname) fname = "";

            gtk_entry_set_text (GTK_ENTRY (data->entry), fname);
        }

        gtk_widget_destroy (dialog);
    }
}

static void
dict_type_changed_cb (GtkComboBox *combo,
                      gpointer userdata)
{
    gchar *typetext = gtk_combo_box_get_active_text(combo);
    if (String("DictFile") == typetext) {
        gtk_widget_show_all(__widgets_dict_file.widget);
        gtk_widget_hide_all(__widgets_skkserv.widget);
        gtk_entry_set_text(GTK_ENTRY(__widgets_dict_file.entry), "");
        gtk_entry_set_text(GTK_ENTRY(__widgets_skkserv.hostname_entry), "");
        gtk_entry_set_text(GTK_ENTRY(__widgets_skkserv.port_entry), "1178");
    } else if (String("SKKServ") == typetext) {
        gtk_widget_hide_all(__widgets_dict_file.widget);
        gtk_widget_show_all(__widgets_skkserv.widget);
        gtk_entry_set_text(GTK_ENTRY(__widgets_dict_file.entry), "");
        gtk_entry_set_text(GTK_ENTRY(__widgets_skkserv.hostname_entry), "");
        gtk_entry_set_text(GTK_ENTRY(__widgets_skkserv.port_entry), "1178");
    }
}

static void
dict_list_add_clicked_cb (GtkButton *button,
                          gpointer userdata)
{
    String dict_type =
        gtk_combo_box_get_active_text(GTK_COMBO_BOX(__combo_box_dict_types));
    String dict_name;
    GtkTreeIter iter;
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(userdata));
    if (dict_type == "DictFile") {
        dict_name = gtk_entry_get_text(GTK_ENTRY(__widgets_dict_file.entry));
    } else if (dict_type == "SKKServ") {
        dict_name = gtk_entry_get_text(GTK_ENTRY(__widgets_skkserv.hostname_entry));
        dict_name += ":";
        dict_name += gtk_entry_get_text(GTK_ENTRY(__widgets_skkserv.port_entry));
    } else {
        return;
    }
    gtk_list_store_append(GTK_LIST_STORE(model), &iter);
    gtk_list_store_set(GTK_LIST_STORE(model), &iter,
                       DICT_TYPE_COLUMN, dict_type.data(),
                       DICT_NAME_COLUMN, dict_name.data(),
                       -1);
}

static void
dict_list_delete_clicked_cb (GtkButton *button,
                             gpointer userdata)
{
    GtkTreeView *view = GTK_TREE_VIEW(userdata);
    GtkTreeModel *model = gtk_tree_view_get_model(view);
    GtkTreeSelection *selection = gtk_tree_view_get_selection(view);
    GtkTreeIter iter;
    if (gtk_tree_selection_get_selected(selection, NULL, &iter)) {
        gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
    }
}

static void
dict_list_up_clicked_cb (GtkButton *button,
                         gpointer userdata)
{
    GtkTreeView *view = GTK_TREE_VIEW(userdata);
    GtkTreeModel *model = gtk_tree_view_get_model(view);
    GtkTreeSelection *selection = gtk_tree_view_get_selection(view);
    GtkTreeIter iter;
    if (gtk_tree_selection_get_selected(selection, NULL, &iter)) {
        GtkTreePath *path;
        GtkTreeIter prev_iter;
        path = gtk_tree_model_get_path(model, &iter);
        if (gtk_tree_path_prev(path) &&
            gtk_tree_model_get_iter(model, &prev_iter, path)) {
            gtk_list_store_move_before(GTK_LIST_STORE(model),
                                       &iter, &prev_iter);
        }
        gtk_tree_path_free(path);
    }
}

static void
dict_list_down_clicked_cb (GtkButton *button,
                           gpointer userdata)
{
    GtkTreeView *view = GTK_TREE_VIEW(userdata);
    GtkTreeModel *model = gtk_tree_view_get_model(view);
    GtkTreeSelection *selection = gtk_tree_view_get_selection(view);
    GtkTreeIter iter;
    if (gtk_tree_selection_get_selected(selection, NULL, &iter)) {
        GtkTreePath *path;
        GtkTreeIter next_iter;
        path = gtk_tree_model_get_path(model, &iter);
        gtk_tree_path_next(path);
        if (gtk_tree_model_get_iter(model, &next_iter, path)) {
            gtk_list_store_move_after(GTK_LIST_STORE(model),
                                      &iter, &next_iter);
        }
        gtk_tree_path_free(path);
    }
}

static void
dict_list_selection_changed_cb (GtkTreeSelection *selection,
                                gpointer data)
{
    GtkTreeIter iter;
    GtkTreeModel *model;
    gchar *dict_type;
    gchar *dict_name;

    if (gtk_tree_selection_get_selected (selection, &model, &iter)) {
        gtk_tree_model_get(model, &iter,
                           DICT_TYPE_COLUMN, &dict_type,
                           DICT_NAME_COLUMN, &dict_name,
                           -1);
        if (String("DictFile") == dict_type) {
            gtk_combo_box_set_active(GTK_COMBO_BOX(__combo_box_dict_types), 0);
            dict_type_changed_cb(GTK_COMBO_BOX(__combo_box_dict_types), NULL);
            gtk_entry_set_text(GTK_ENTRY(__widgets_dict_file.entry),
                               dict_name);
        } else if (String("SKKServ") == dict_type) {
            String hostname = dict_name;
            String portnum = "1178";
            gint pos = hostname.find(':');
            gtk_combo_box_set_active(GTK_COMBO_BOX(__combo_box_dict_types), 1);
            dict_type_changed_cb(GTK_COMBO_BOX(__combo_box_dict_types), NULL);
            if (pos != String::npos) {
                portnum = hostname.substr(pos+1, String::npos);
                hostname.erase(pos);
            } 
            gtk_entry_set_text(GTK_ENTRY(__widgets_skkserv.hostname_entry),
                               hostname.data());
            gtk_entry_set_text(GTK_ENTRY(__widgets_skkserv.port_entry),
                               portnum.data());
        }
    }
    fflush(stdout);
}

inline GtkListStore*
dict_list_setup (std::vector<String> &data)
{
    GtkListStore *retval =
        gtk_list_store_new(DICT_N_COLUMNS,
                           G_TYPE_STRING,  /* dict type */
                           G_TYPE_STRING); /* dict name */

    for (int i = 0; i < data.size(); i++) {
        GtkTreeIter treeiter;
        int colon_pos;
        gtk_list_store_append(retval, &treeiter);
        colon_pos = data[i].find(':');
        gtk_list_store_set(retval, &treeiter,
                           DICT_TYPE_COLUMN,
                           (colon_pos == String::npos)? "DictFile" : data[i].substr(0, colon_pos).data(),
                           DICT_NAME_COLUMN,
                           (colon_pos == String::npos)? data[i].data() : data[i].substr(colon_pos+1, String::npos).data(),
                           -1);
    }
    return retval;
}

inline GtkWidget*
dict_list_model_setup (GtkTreeModel *model)
{
    GtkWidget *retval =
        gtk_tree_view_new_with_model(model);
    GtkTreeViewColumn *column;
    GtkCellRenderer *renderer;
    GtkTreeSelection *selection;

    renderer = gtk_cell_renderer_text_new();
    column =
        gtk_tree_view_column_new_with_attributes("type",
                                                 renderer,
                                                 "text", DICT_TYPE_COLUMN,
                                                 NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(retval), column);
    column =
        gtk_tree_view_column_new_with_attributes("name",
                                                 renderer,
                                                 "text", DICT_NAME_COLUMN,
                                                 NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(retval), column);

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (retval));
    gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);
    g_signal_connect (G_OBJECT (selection), "changed",
                      G_CALLBACK (dict_list_selection_changed_cb),
                      NULL);

    return retval;
}


inline GtkWidget*
dict_entry_widgets_setup (GtkBox *container,
                          GtkTreeView *view)
{
    GtkWidget *vbox, *hbox;
    GtkWidget *label;
    GtkWidget *button;

    hbox = gtk_hbox_new(FALSE, 0);
    gtk_widget_show(hbox);
    label = gtk_label_new("Dictionary Type: ");
    gtk_widget_show(label);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 4);
    __combo_box_dict_types = gtk_combo_box_new_text();
    for (int i = 0; !__dict_type_names[i].empty(); i++) {
        gtk_combo_box_append_text(GTK_COMBO_BOX(__combo_box_dict_types),
                                  __dict_type_names[i].data());
    }
    g_signal_connect((gpointer) __combo_box_dict_types, "changed",
                     G_CALLBACK(dict_type_changed_cb),
                     NULL);
    gtk_widget_show(__combo_box_dict_types);
    gtk_box_pack_start(GTK_BOX(hbox), __combo_box_dict_types, FALSE, TRUE, 4);
    gtk_box_pack_start(container, hbox, FALSE, FALSE, 4);

    __widgets_dict_file.widget = gtk_hbox_new(FALSE, 0);

    /* dictionary file widgets */
    __widgets_dict_file.widget = gtk_hbox_new(FALSE, 0);
    label  = gtk_label_new(_("System Dictionary Path:"));
    __widgets_dict_file.title = N_("System Dictionary Path:");
    __widgets_dict_file.entry = gtk_entry_new();
    __widgets_dict_file.button = gtk_button_new_with_label ("...");
    gtk_box_pack_start (GTK_BOX (__widgets_dict_file.widget),
                        label, FALSE, FALSE, 4);
    gtk_box_pack_start (GTK_BOX (__widgets_dict_file.widget),
                        __widgets_dict_file.entry, TRUE, TRUE, 4);
    gtk_box_pack_start (GTK_BOX (__widgets_dict_file.widget),
                        __widgets_dict_file.button, FALSE, FALSE, 4);
    gtk_widget_show_all (__widgets_dict_file.widget);
    gtk_box_pack_start(container, __widgets_dict_file.widget,
                       TRUE, FALSE, 4);
    gtk_label_set_mnemonic_widget (GTK_LABEL (label),
                                   __widgets_dict_file.entry);
    gtk_label_set_mnemonic_widget (GTK_LABEL (label),
                                   __widgets_dict_file.button);
    g_signal_connect ((gpointer) __widgets_dict_file.button, "clicked",
                      G_CALLBACK (file_selection_clicked_cb),
                      &__widgets_dict_file);


    /* skkserv widgets */
    __widgets_skkserv.widget = gtk_vbox_new(FALSE, 0);
    hbox = gtk_hbox_new(FALSE, 0);
    gtk_widget_hide(__widgets_skkserv.widget);
    label = gtk_label_new(_("Server Name:"));
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 4);
    __widgets_skkserv.hostname_entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(hbox), __widgets_skkserv.hostname_entry,
                       TRUE, TRUE, 4);
    gtk_box_pack_start(GTK_BOX(__widgets_skkserv.widget), hbox,
                       FALSE, FALSE, 4);
    hbox = gtk_hbox_new(FALSE, 0);
    label = gtk_label_new(_("Port Number:"));
    __widgets_skkserv.port_entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 4);
    gtk_box_pack_start(GTK_BOX(hbox), __widgets_skkserv.port_entry,
                       TRUE, TRUE, 4);
    gtk_box_pack_start(GTK_BOX(__widgets_skkserv.widget), hbox,
                       FALSE, FALSE, 4);

    gtk_box_pack_start(container, __widgets_skkserv.widget,
                       TRUE, FALSE, 4);

    /* edit buttons */
    hbox = gtk_hbox_new(FALSE, 0);
    button = gtk_button_new_with_label(_("Add"));
    gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, FALSE, 4);
    g_signal_connect((gpointer) button, "clicked",
                     G_CALLBACK(dict_list_add_clicked_cb),
                     (gpointer) view);
    button = gtk_button_new_with_label(_("Delete"));
    gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, FALSE, 4);
    g_signal_connect((gpointer) button, "clicked",
                     G_CALLBACK(dict_list_delete_clicked_cb),
                     (gpointer) view);

    gtk_widget_show_all(hbox);
    gtk_box_pack_start(container, hbox, FALSE, FALSE, 4);
}

void
on_default_dict_selection_clicked (GtkButton *button,
                                   gpointer   user_data)
{
    std::vector<String> *data = static_cast<std::vector<String>*>(user_data);
    if (data) {
        GtkWidget *dialog = GTK_WIDGET(gtk_dialog_new());
        GtkWidget *ok_button, *cancel_button;
        gint result;
        GtkWidget *tree;
        GtkListStore *__dict_list_store;

        gtk_window_set_title(GTK_WINDOW(dialog),
                             _("Configure Dictionaries"));
        __dict_list_store = dict_list_setup(*data);

        /* setup dialog for dictionaries selection */
        gtk_container_set_border_width(GTK_CONTAINER(dialog), 4);
        gtk_window_set_resizable(GTK_WINDOW(dialog), TRUE);

        tree = dict_list_model_setup(GTK_TREE_MODEL(__dict_list_store));

        gtk_widget_show(tree);

        {
            GtkWidget *hbox, *vbox, *button;
            hbox = gtk_hbox_new(FALSE, 0);
            gtk_widget_show(hbox);
            gtk_box_pack_start(GTK_BOX(hbox), tree, FALSE, FALSE, 4);
            vbox = gtk_vbox_new(FALSE, 0);
            button = gtk_button_new_from_stock(GTK_STOCK_GO_UP);
            gtk_box_pack_start(GTK_BOX(vbox), button, TRUE, FALSE, 4);
            g_signal_connect((gpointer) button, "clicked",
                             G_CALLBACK(dict_list_up_clicked_cb),
                             (gpointer) tree);
            button = gtk_button_new_from_stock(GTK_STOCK_GO_DOWN);
            gtk_box_pack_start(GTK_BOX(vbox), button, TRUE, FALSE, 4);
            g_signal_connect((gpointer) button, "clicked",
                             G_CALLBACK(dict_list_down_clicked_cb),
                             (gpointer) tree);
            gtk_widget_show_all(vbox);
            gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 4);
            gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), hbox,
                               FALSE, FALSE, 4);
        }

        dict_entry_widgets_setup(GTK_BOX(GTK_DIALOG(dialog)->vbox), GTK_TREE_VIEW(tree));

        cancel_button = gtk_dialog_add_button(GTK_DIALOG(dialog),
                                              GTK_STOCK_CANCEL,
                                              GTK_RESPONSE_CANCEL);
        ok_button = gtk_dialog_add_button(GTK_DIALOG(dialog),
                                          GTK_STOCK_OK,
                                          GTK_RESPONSE_OK);
        gtk_widget_grab_default(ok_button);
        gtk_dialog_set_has_separator(GTK_DIALOG(dialog), TRUE);
        gtk_widget_show(dialog);

        result = gtk_dialog_run(GTK_DIALOG(dialog));
        if (result == GTK_RESPONSE_OK) {
            GtkTreeIter iter;
            gtk_tree_model_get_iter_first(GTK_TREE_MODEL(__dict_list_store),
                                          &iter);
            data->clear();
            do {
                String s;
                gchar *dict_type, *dict_name;
                gtk_tree_model_get(GTK_TREE_MODEL(__dict_list_store), &iter,
                                   DICT_TYPE_COLUMN, &dict_type,
                                   DICT_NAME_COLUMN, &dict_name,
                                   -1);
                s.assign(String(dict_type));
                s.append(1, ':');
                s.append(String(dict_name));
                data->push_back(s);
            } while (gtk_tree_model_iter_next(GTK_TREE_MODEL(__dict_list_store), &iter));

            if (data->size()>0) {
                std::vector<String>::iterator it = data->begin();
                String s = *it;
                for (; it != data->end(); it++) {
                    s.append(1, ',');
                    s.append(*it);
                }
                gtk_entry_set_text(GTK_ENTRY(__widget_sysdicts), s.data());
            }
            __have_changed = true;
        }

        gtk_widget_destroy (dialog);
    }
}


}
