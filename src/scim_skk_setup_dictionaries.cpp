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
#include <algorithm>

#include <scim.h>

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "scim_skk_intl.h"

using namespace scim;
using namespace std;

namespace scim_skk {

static void file_selection_clicked_cb (GtkButton *button,
                                       gpointer user_data);
static void dict_type_changed_cb (GtkComboBox *combo,
                                  gpointer user_data);
static void dict_list_add_clicked_cb (GtkButton *button,
                                      gpointer user_data);
static void dict_list_delete_clicked_cb (GtkButton *button,
                                         gpointer user_data);
static void dict_list_up_clicked_cb (GtkButton *button,
                                     gpointer user_data);
static void dict_list_down_clicked_cb (GtkButton *button,
                                       gpointer user_data);

extern bool __have_changed;
extern GtkWidget *__widget_sysdicts;
extern vector<String> __config_sysdicts;

enum DictColumnType {
    DICT_TYPE_COLUMN,  /* string: type of dictionary */
    DICT_NAME_COLUMN,  /* string: dictionary name */
    DICT_N_COLUMNS
};

struct DictionaryConfigWidgets {
    gchar *title;
    GtkWidget *widget;
    GtkWidget *entry;
    GtkWidget *button;
    GtkWidget *entry2;
};

static String __dict_type_names [] = {
    "DictFile",
    "SKKServ",
    "CDBFile",
    ""
};

static DictionaryConfigWidgets __widgets_dicts [] = {
    {
        _("System Dictionary Path:"),
        NULL,
        NULL,
        NULL,
        NULL
    },
    {
        "",
        NULL,
        NULL,
        NULL,
        NULL
    },
    {
        _("CDB Dictionary Path:"),
        NULL,
        NULL,
        NULL,
        NULL
    },
    {
        "",
        NULL,
        NULL,
        NULL,
        NULL
    }
};

static GtkWidget *__combo_box_dict_types = NULL;

static GtkListStore *__dict_list_store = NULL;


GtkListStore*
dict_list_setup (vector<String> &data)
{
    if (__dict_list_store) {
        gtk_list_store_clear(__dict_list_store);
    } else {
        __dict_list_store = 
            gtk_list_store_new(DICT_N_COLUMNS,
                               G_TYPE_STRING,  /* dict type */
                               G_TYPE_STRING); /* dict name */
    }

    for (int i = 0; i < data.size(); i++) {
        GtkTreeIter treeiter;
        int colon_pos;
        gtk_list_store_append(__dict_list_store, &treeiter);
        colon_pos = data[i].find(':');
        gtk_list_store_set(__dict_list_store, &treeiter,
                           DICT_TYPE_COLUMN,
                           (colon_pos == String::npos)? "DictFile" : data[i].substr(0, colon_pos).data(),
                           DICT_NAME_COLUMN,
                           (colon_pos == String::npos)? data[i].data() : data[i].substr(colon_pos+1, String::npos).data(),
                           -1);
    }
    return __dict_list_store;
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
        gtk_tree_view_column_new_with_attributes(_("type"),
                                                 renderer,
                                                 "text", DICT_TYPE_COLUMN,
                                                 NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(retval), column);
    column =
        gtk_tree_view_column_new_with_attributes(_("dict name"),
                                                 renderer,
                                                 "text", DICT_NAME_COLUMN,
                                                 NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(retval), column);

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (retval));
    gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);

    return retval;
}


inline void
dict_entry_widgets_dictfile_setup (DictionaryConfigWidgets *widgets)
{
    GtkWidget *label;
    widgets->widget = gtk_hbox_new(FALSE, 0);
    label  = gtk_label_new(widgets->title);
    widgets->entry = gtk_entry_new();
    widgets->button = gtk_button_new_with_label ("...");
    gtk_box_pack_start (GTK_BOX (widgets->widget),
                        label, FALSE, FALSE, 4);
    gtk_box_pack_start (GTK_BOX (widgets->widget),
                        widgets->entry, TRUE, TRUE, 4);
    gtk_box_pack_start (GTK_BOX (widgets->widget),
                        widgets->button, FALSE, FALSE, 4);
    gtk_widget_show_all (widgets->widget);
    gtk_label_set_mnemonic_widget (GTK_LABEL (label),
                                   widgets->entry);
    gtk_label_set_mnemonic_widget (GTK_LABEL (label),
                                   widgets->button);
    g_signal_connect ((gpointer) widgets->button, "clicked",
                      G_CALLBACK (file_selection_clicked_cb),
                      widgets);
}

inline void
dict_entry_widgets_skkserv_setup (DictionaryConfigWidgets *widgets)
{
    GtkWidget *hbox, *label;

    widgets->widget = gtk_vbox_new(FALSE, 0);
    hbox = gtk_hbox_new(FALSE, 0);
    gtk_widget_hide(widgets->widget);
    label = gtk_label_new(_("Server Name:"));
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 4);
    widgets->entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(hbox), widgets->entry,
                       TRUE, TRUE, 4);
    gtk_box_pack_start(GTK_BOX(widgets->widget), hbox,
                       FALSE, FALSE, 4);
    hbox = gtk_hbox_new(FALSE, 0);
    label = gtk_label_new(_("Port Number:"));
    widgets->entry2 = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 4);
    gtk_box_pack_start(GTK_BOX(hbox), widgets->entry2,
                       TRUE, TRUE, 4);
    gtk_box_pack_start(GTK_BOX(widgets->widget), hbox,
                       FALSE, FALSE, 4);
}


inline GtkWidget*
dict_entry_widgets_setup (GtkBox *container,
                          GtkTreeView *view)
{
    GtkWidget *hbox;
    GtkWidget *button;

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

GtkWidget*
dict_selection_widget_setup (void)
{
    GtkWidget *tree;

    __widget_sysdicts = gtk_vbox_new(FALSE, 0);
    dict_list_setup(__config_sysdicts);

    /* setup container for dictionaries selection */
    gtk_container_set_border_width(GTK_CONTAINER(__widget_sysdicts), 4);

    tree = dict_list_model_setup(GTK_TREE_MODEL(__dict_list_store));

    gtk_widget_show(tree);

    {
        GtkWidget *hbox, *vbox, *button;
        hbox = gtk_hbox_new(FALSE, 0);
        gtk_widget_show(hbox);
        gtk_box_pack_start(GTK_BOX(hbox), tree, TRUE, TRUE, 4);
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
        button = gtk_button_new_with_label(_("Delete"));
        gtk_box_pack_start (GTK_BOX (vbox), button, TRUE, FALSE, 4);
        g_signal_connect((gpointer) button, "clicked",
                         G_CALLBACK(dict_list_delete_clicked_cb),
                         (gpointer) GTK_TREE_VIEW(tree));
        button = gtk_button_new_with_label(_("Add"));
        gtk_box_pack_start (GTK_BOX (vbox), button, TRUE, FALSE, 12);
        g_signal_connect((gpointer) button, "clicked",
                         G_CALLBACK(dict_list_add_clicked_cb),
                         (gpointer) GTK_TREE_VIEW(tree));
        gtk_widget_show_all(vbox);
        gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 4);
        gtk_box_pack_start(GTK_BOX(__widget_sysdicts), hbox, FALSE, FALSE, 4);
    }

    gtk_widget_show(__widget_sysdicts);

    return __widget_sysdicts;
}


/* callback functions */

static void
file_selection_clicked_cb (GtkButton *button,
                           gpointer   user_data)
{
    DictionaryConfigWidgets *data = static_cast <DictionaryConfigWidgets *> (user_data);

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
    for (int i = 0; __dict_type_names[i] != ""; i++) {
        if (__dict_type_names[i] == typetext) {
            gtk_widget_show_all(__widgets_dicts[i].widget);
        } else {
            gtk_widget_hide_all(__widgets_dicts[i].widget);
        }
    }
    gtk_entry_set_text(GTK_ENTRY(__widgets_dicts[0].entry), "");
    gtk_entry_set_text(GTK_ENTRY(__widgets_dicts[1].entry), "");
    gtk_entry_set_text(GTK_ENTRY(__widgets_dicts[1].entry2), "1178");
    gtk_entry_set_text(GTK_ENTRY(__widgets_dicts[2].entry), "");
}

void
dict_list_add_clicked_cb (GtkButton *button,
                          gpointer userdata)
{
    GtkWidget *dialog = GTK_WIDGET(gtk_dialog_new());
    GtkWidget *ok_button, *cancel_button;
    GtkWidget *hbox, *label;
    gint result;

    gtk_window_set_title(GTK_WINDOW(dialog),
                         _("Add new dictionary"));

    hbox = gtk_hbox_new(FALSE, 0);
    gtk_widget_show(hbox);
    label = gtk_label_new(_("Dictionary Type: "));
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
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), hbox, FALSE, FALSE, 4);

    /* dictionary file widgets */
    dict_entry_widgets_dictfile_setup(&(__widgets_dicts[0]));
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), __widgets_dicts[0].widget,
                       TRUE, FALSE, 4);

    /* skkserv widgets */
    dict_entry_widgets_skkserv_setup(&__widgets_dicts[1]);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), __widgets_dicts[1].widget,
                       TRUE, FALSE, 4);

    /* dictionary file widgets */
    dict_entry_widgets_dictfile_setup(&__widgets_dicts[2]);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), __widgets_dicts[2].widget,
                       TRUE, FALSE, 4);

    gtk_combo_box_set_active(GTK_COMBO_BOX(__combo_box_dict_types), 0);

    cancel_button = gtk_dialog_add_button(GTK_DIALOG(dialog),
                                          GTK_STOCK_CANCEL,
                                          GTK_RESPONSE_CANCEL);
    ok_button = gtk_dialog_add_button(GTK_DIALOG(dialog),
                                      _("Add"),
                                      GTK_RESPONSE_OK);
    gtk_widget_grab_default(ok_button);
    gtk_dialog_set_has_separator(GTK_DIALOG(dialog), TRUE);
    gtk_widget_show(dialog);

    result = gtk_dialog_run(GTK_DIALOG(dialog));
    if (result == GTK_RESPONSE_OK) {
        String dict_type = gtk_combo_box_get_active_text(GTK_COMBO_BOX(__combo_box_dict_types));
        String dict_name;
        GtkTreeIter iter;
        GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(userdata));
        for (int i = 0; __dict_type_names[i] != ""; i++) {
            if (__dict_type_names[i] == dict_type) {
                dict_name =
                    gtk_entry_get_text(GTK_ENTRY(__widgets_dicts[i].entry));
            }
            if (dict_type == "SKKServ" && !dict_name.empty()) {
                dict_name += ":";
                dict_name +=
                    gtk_entry_get_text(GTK_ENTRY(__widgets_dicts[i].entry2));
            }
            if (!dict_name.empty()) {
                __config_sysdicts.push_back(dict_type+String(":")+dict_name);

                gtk_list_store_append(GTK_LIST_STORE(model), &iter);
                gtk_list_store_set(GTK_LIST_STORE(model), &iter,
                                   DICT_TYPE_COLUMN, dict_type.data(),
                                   DICT_NAME_COLUMN, dict_name.data(),
                                   -1);

                __have_changed = true;
                break;
            }
        }
    }
    gtk_widget_destroy(dialog);
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
        gint i;
        GtkTreePath *path = gtk_tree_model_get_path(model, &iter);
        i = gtk_tree_path_get_indices(path)[0];
        gtk_list_store_remove(GTK_LIST_STORE(model), &iter);

        __config_sysdicts.erase(__config_sysdicts.begin()+i);
        __have_changed = true;
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
        int i;
        path = gtk_tree_model_get_path(model, &iter);
        i = gtk_tree_path_get_indices(path)[0];
        if (i != 0) {
            vector<String>::iterator it  = __config_sysdicts.begin() + i;
            vector<String>::iterator it2 = it - 1;
            iter_swap(it, it2);
            __have_changed = true;
        }
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
        int i;
        path = gtk_tree_model_get_path(model, &iter);
        i = gtk_tree_path_get_indices(path)[0];
        if (i < __config_sysdicts.size() - 1) {
            vector<String>::iterator it  = __config_sysdicts.begin() + i;
            vector<String>::iterator it2 = it + 1;
            iter_swap(it, it2);
            __have_changed = true;
        }

        gtk_tree_path_next(path);
        if (gtk_tree_model_get_iter(model, &next_iter, path)) {
            gtk_list_store_move_after(GTK_LIST_STORE(model),
                                      &iter, &next_iter);
        }
        gtk_tree_path_free(path);
    }
}

}
