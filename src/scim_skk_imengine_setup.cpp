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

/*
 * Based on scim_anthy_imengine_setup.
 *  Copyright (C) 2004 Hiroyuki Ikezoe
 *  Copyright (C) 2004 Takuro Ashie
 */

#define Uses_SCIM_CONFIG_BASE

#include <gtk/gtk.h>

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <scim.h>
#include <gtk/scimkeyselection.h>
#include "scim_skk_prefs.h"

#ifdef HAVE_GETTEXT
#  include <libintl.h>
#  define _(String) dgettext(GETTEXT_PACKAGE,String)
#  define N_(String) (String)
#else
#  define _(String) (String)
#  define N_(String) (String)
#  define bindtextdomain(Package,Directory)
#  define textdomain(domain)
#  define bind_textdomain_codeset(domain,codeset)
#endif

using namespace scim;

#define scim_module_init skk_imengine_setup_LTX_scim_module_init
#define scim_module_exit skk_imengine_setup_LTX_scim_module_exit

#define scim_setup_module_create_ui       skk_imengine_setup_LTX_scim_setup_module_create_ui
#define scim_setup_module_get_category    skk_imengine_setup_LTX_scim_setup_module_get_category
#define scim_setup_module_get_name        skk_imengine_setup_LTX_scim_setup_module_get_name
#define scim_setup_module_get_description skk_imengine_setup_LTX_scim_setup_module_get_description
#define scim_setup_module_load_config     skk_imengine_setup_LTX_scim_setup_module_load_config
#define scim_setup_module_save_config     skk_imengine_setup_LTX_scim_setup_module_save_config
#define scim_setup_module_query_changed   skk_imengine_setup_LTX_scim_setup_module_query_changed

#define DATA_POINTER_KEY "scim-skk::ConfigPointer"

static GtkWidget * create_setup_window ();
static void        load_config (const ConfigPointer &config);
static void        save_config (const ConfigPointer &config);
static bool        query_changed ();

// Module Interface.
extern "C" {
    void scim_module_init (void)
    {
        bindtextdomain (GETTEXT_PACKAGE, SCIM_SKK_LOCALEDIR);
        bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
    }

    void scim_module_exit (void)
    {
    }

    GtkWidget * scim_setup_module_create_ui (void)
    {
        return create_setup_window ();
    }

    String scim_setup_module_get_category (void)
    {
        return String ("IMEngine");
    }

    String scim_setup_module_get_name (void)
    {
        return String (_("SKK"));
    }

    String scim_setup_module_get_description (void)
    {
        return String (_("An SKK IMEngine Module."));
    }

    void scim_setup_module_load_config (const ConfigPointer &config)
    {
        load_config (config);
    }

    void scim_setup_module_save_config (const ConfigPointer &config)
    {
        save_config (config);
    }

    bool scim_setup_module_query_changed ()
    {
        return query_changed ();
    }
} // extern "C"


// Internal data structure
struct KeyboardConfigData
{
    const char *key;
    String      data;
    const char *label;
    const char *title;
    const char *tooltip;
    GtkWidget  *entry;
    GtkWidget  *button;
};

struct KeyboardConfigPage
{
    const char         *label;
    KeyboardConfigData *data;
};

struct FileConfigData
{
    const char *title;
    String      data;
    GtkWidget  *entry;
    GtkWidget  *button;
};

struct ComboConfigData
{
    const char *label;
    const char *data;
};

// Internal data declaration.
static String __config_userdict   = SCIM_SKK_CONFIG_USERDICT_DEFAULT;
static int    __config_listsize   = SCIM_SKK_CONFIG_DICT_LISTSIZE_DEFAULT;
static bool   __config_view_annot = SCIM_SKK_CONFIG_DICT_VIEW_ANNOT_DEFAULT;
static String __config_selection_style = SCIM_SKK_CONFIG_SELECTION_STYLE_DEFAULT;

static bool __have_changed    = true;

static GtkWidget    * __widget_userdict        = 0;
static GtkWidget    * __widget_listsize        = 0;
static GtkWidget    * __widget_view_annot      = 0;
static GtkWidget    * __widget_selection_style = 0;
static GtkTooltips  * __widget_tooltips        = 0;

static KeyboardConfigData __config_keyboards_common [] =
{
    {
        SCIM_SKK_CONFIG_KAKUTEI_KEY,
        SCIM_SKK_CONFIG_KAKUTEI_KEY_DEFAULT,
        N_("Kakutei Keys:"),
        N_("Select Kakutei Keys"),
        N_("The key events to commit the preedit string. "),
        NULL,
        NULL,
    },
    {
        SCIM_SKK_CONFIG_CANCEL_KEY,
        SCIM_SKK_CONFIG_CANCEL_KEY_DEFAULT,
        N_("Cancel Keys:"),
        N_("Select Cancel Keys"),
        N_("The key events to cancel the some skk states. "),
        NULL,
        NULL,
    },
    {
        SCIM_SKK_CONFIG_CONVERT_KEY,
        SCIM_SKK_CONFIG_CONVERT_KEY_DEFAULT,
        N_("Convert Keys:"),
        N_("Select Convert Keys"),
        N_("The key events to start converting. "),
        NULL,
        NULL,
    },
    {
        SCIM_SKK_CONFIG_START_PREEDIT_KEY,
        SCIM_SKK_CONFIG_START_PREEDIT_KEY_DEFAULT,
        N_("Start-Preedit Keys:"),
        N_("Select Start-Preedit Keys"),
        N_("The key events to start preediting. "),
        NULL,
        NULL,
    },
    {
        SCIM_SKK_CONFIG_ASCII_CONVERT_KEY,
        SCIM_SKK_CONFIG_ASCII_CONVERT_KEY_DEFAULT,
        N_("ASCII-Convert Keys:"),
        N_("Select ASCII-Convert Keys"),
        N_("The key events to start ascii-converting mode. "),
        NULL,
        NULL,
    },
    {
        SCIM_SKK_CONFIG_PREVCAND_KEY,
        SCIM_SKK_CONFIG_PREVCAND_KEY_DEFAULT,
        N_("Previous Candidate Keys:"),
        N_("Select Previous Candidate Keys"),
        N_("The key events to turn back. "),
        NULL,
        NULL,
    },
    {
        NULL,
        "",
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
    },
};

static KeyboardConfigData __config_keyboards_mode [] =
{
    {
        SCIM_SKK_CONFIG_KATAKANA_KEY,
        SCIM_SKK_CONFIG_KATAKANA_KEY_DEFAULT,
        N_("Katakana Keys:"),
        N_("Select Katakana Keys"),
        N_("The key events to toggle the skkmode hiragana to katakana. "),
        NULL,
        NULL,
    },
    {
        SCIM_SKK_CONFIG_HALF_KATAKANA_KEY,
        SCIM_SKK_CONFIG_HALF_KATAKANA_KEY_DEFAULT,
        N_("Half-Katakana Keys:"),
        N_("Select Half-Katakana Keys"),
        N_("The key events to toggle the skkmode hiragana to half-katakana. "),
        NULL,
        NULL,
    },
    {
        SCIM_SKK_CONFIG_ASCII_KEY,
        SCIM_SKK_CONFIG_ASCII_KEY_DEFAULT,
        N_("ASCII Keys:"),
        N_("Select ASCII Keys"),
        N_("The key events to enter ASII mode. "),
        NULL,
        NULL,
    },
    {
        SCIM_SKK_CONFIG_WIDE_ASCII_KEY,
        SCIM_SKK_CONFIG_WIDE_ASCII_KEY_DEFAULT,
        N_("Wide-ASCII Keys:"),
        N_("Select Wide-ASCII Keys"),
        N_("The key events to enter Wide-ASCII mode. "),
        NULL,
        NULL,
    },
    {
        NULL,
        "",
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
    },
};

static KeyboardConfigData __config_keyboards_caret [] =
{
    {
        SCIM_SKK_CONFIG_BACKSPACE_KEY,
        SCIM_SKK_CONFIG_BACKSPACE_KEY_DEFAULT,
        N_("Backspace Keys:"),
        N_("Select Backspace Keys"),
        N_("The key events to backspace. "),
        NULL,
        NULL,
    },
    {
        SCIM_SKK_CONFIG_DELETE_KEY,
        SCIM_SKK_CONFIG_DELETE_KEY_DEFAULT,
        N_("Delete Keys:"),
        N_("Select Delete Keys"),
        N_("The key events to delete. "),
        NULL,
        NULL,
    },
    {
        SCIM_SKK_CONFIG_FORWARD_KEY,
        SCIM_SKK_CONFIG_FORWARD_KEY_DEFAULT,
        N_("Forward Keys:"),
        N_("Select Forward Keys"),
        N_("The key events to move the caret forward. "),
        NULL,
        NULL,
    },
    {
        SCIM_SKK_CONFIG_BACKWARD_KEY,
        SCIM_SKK_CONFIG_BACKWARD_KEY_DEFAULT,
        N_("Backward Keys:"),
        N_("Select Backward Keys"),
        N_("The key events to move the caret backward. "),
        NULL,
        NULL,
    },
    {
        NULL,
        "",
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
    },
};

static FileConfigData __config_sysdict = {
    N_("System Dictionary Path"),
    SCIM_SKK_CONFIG_SYSDICT_DEFAULT,
    NULL,
    NULL,
};

static ComboConfigData selection_style[] =
{
    {N_("qwerty arrangement"), "Qwerty"},
    {N_("Dvorak arrangement"), "Dvorak"},
    {N_("number selection"),   "Number"},
    {NULL, NULL},
};



static struct KeyboardConfigPage __key_conf_pages[] =
{
    {N_("Common keys"),     __config_keyboards_common},
    {N_("Mode keys"),       __config_keyboards_mode},
    {N_("Caret keys"),      __config_keyboards_caret},
};
static unsigned int __key_conf_pages_num = sizeof (__key_conf_pages) / sizeof (KeyboardConfigPage);

static void on_default_editable_changed       (GtkEditable     *editable,
                                               gpointer         user_data);
static void on_default_spin_button_changed    (GtkSpinButton   *spin_button,
                                               gpointer         user_data);
static void on_default_toggle_button_toggled  (GtkToggleButton *togglebutton,
                                               gpointer         user_data);
static void on_default_file_selection_clicked (GtkButton       *button,
                                               gpointer         user_data);
static void on_default_key_selection_clicked  (GtkButton       *button,
                                               gpointer         user_data);
static void on_default_combo_changed          (GtkEditable     *editable,
                                               gpointer         user_data);
static void setup_widget_value ();


static GtkWidget *
create_combo_widget (const char *label_text, GtkWidget **widget,
                     gpointer data_p, gpointer candidates_p)
{
    GtkWidget *hbox, *label;

    hbox = gtk_hbox_new (FALSE, 0);
    gtk_widget_show (hbox);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 4);

    label = gtk_label_new (label_text);
    gtk_widget_show (label);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 4);

    *widget = gtk_combo_new ();
    gtk_combo_set_value_in_list (GTK_COMBO (*widget), TRUE, FALSE);
    gtk_combo_set_case_sensitive (GTK_COMBO (*widget), TRUE);
    gtk_entry_set_editable (GTK_ENTRY (GTK_COMBO (*widget)->entry), FALSE);
    gtk_widget_show (*widget);
    gtk_box_pack_start (GTK_BOX (hbox), *widget, FALSE, FALSE, 4);
    g_object_set_data (G_OBJECT (GTK_COMBO (*widget)->entry), DATA_POINTER_KEY,
                       (gpointer) candidates_p);

    g_signal_connect ((gpointer) GTK_COMBO (*widget)->entry, "changed",
                      G_CALLBACK (on_default_combo_changed),
                      data_p);

    return hbox;
}


static GtkWidget *
create_options_page ()
{
    GtkWidget *vbox, *widget, *table, *label;
    GtkWidget *sysdict_button;
    GtkWidget *userdict_button;
    GtkWidget *listsize_entry;

    vbox = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox);

    table = gtk_table_new (1, 3, FALSE);
    gtk_widget_show (table);

    /* system dictionary */
    widget = gtk_hbox_new(FALSE, 0);
    label  = gtk_label_new(_(__config_sysdict.title));
    __config_sysdict.entry = gtk_entry_new();
    __config_sysdict.button = gtk_button_new_with_label ("...");
    gtk_widget_show (label);
    gtk_widget_show (__config_sysdict.entry);
    gtk_widget_show (__config_sysdict.button);
    gtk_box_pack_start (GTK_BOX (widget), label, FALSE, FALSE, 4);
    gtk_box_pack_start (GTK_BOX (widget), __config_sysdict.entry,
                        TRUE, TRUE, 4);
    gtk_box_pack_start (GTK_BOX (widget), __config_sysdict.button,
                        FALSE, FALSE, 4);
    gtk_widget_show (widget);
    gtk_box_pack_start (GTK_BOX (vbox), widget, FALSE, FALSE, 4);
    gtk_label_set_mnemonic_widget (GTK_LABEL (label), __config_sysdict.entry);
    gtk_label_set_mnemonic_widget (GTK_LABEL (label), __config_sysdict.button);

    /* user dictionary */
    widget            = gtk_hbox_new(FALSE, 0);
    label             = gtk_label_new(_("User Dictionary Name"));
    __widget_userdict = gtk_entry_new();
    gtk_widget_show(label);
    gtk_widget_show(__widget_userdict);
    gtk_box_pack_start (GTK_BOX (widget), label, FALSE, FALSE, 4);
    gtk_box_pack_start (GTK_BOX (widget), __widget_userdict, TRUE, TRUE, 4);
    gtk_label_set_mnemonic_widget (GTK_LABEL (label), __widget_userdict);
    gtk_widget_show(widget);
    gtk_box_pack_start (GTK_BOX (vbox), widget, FALSE, FALSE, 4);

    /* selection style */
    widget = create_combo_widget (_("Selection Style:"),
                                  &__widget_selection_style,
                                  (gpointer) &__config_selection_style,
                                  (gpointer) &selection_style);
    gtk_box_pack_start (GTK_BOX (vbox), widget, FALSE, FALSE, 4);


    /* list size */
    widget         = gtk_hbox_new(FALSE, 0);
    label          = gtk_label_new (_("List Size:"));
    listsize_entry = gtk_spin_button_new_with_range(0, 100, 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON (listsize_entry),
                              __config_listsize);
    gtk_spin_button_set_digits(GTK_SPIN_BUTTON (listsize_entry), 0);
    gtk_spin_button_set_update_policy(GTK_SPIN_BUTTON (listsize_entry),
                                      GTK_UPDATE_IF_VALID);
    gtk_widget_show(label);
    gtk_widget_show(listsize_entry);
    gtk_box_pack_start (GTK_BOX (widget), label, FALSE, FALSE, 4);
    gtk_box_pack_start (GTK_BOX (widget), listsize_entry, FALSE, FALSE, 4);
    gtk_widget_show(widget);
    gtk_box_pack_start (GTK_BOX (vbox), widget, FALSE, FALSE, 4);

    /* view annot */
    __widget_view_annot = gtk_check_button_new_with_mnemonic (_("View Annotation."));
    gtk_widget_show (__widget_view_annot);
    gtk_box_pack_start (GTK_BOX (vbox), __widget_view_annot, FALSE, FALSE, 4);
    gtk_container_set_border_width (GTK_CONTAINER (__widget_view_annot), 4);

    // Connect all signals.
    g_signal_connect ((gpointer) __config_sysdict.button, "clicked",
                      G_CALLBACK (on_default_file_selection_clicked),
                      &__config_sysdict);
    g_signal_connect ((gpointer) __config_sysdict.entry, "changed",
                      G_CALLBACK (on_default_editable_changed),
                      &(__config_sysdict.data));

    g_signal_connect ((gpointer) __widget_userdict, "changed",
                      G_CALLBACK (on_default_editable_changed),
                      &__config_userdict);

    g_signal_connect ((gpointer) listsize_entry, "value-changed",
                      G_CALLBACK (on_default_spin_button_changed),
                      &__config_listsize);

    g_signal_connect ((gpointer) __widget_view_annot, "toggled",
                      G_CALLBACK (on_default_toggle_button_toggled),
                      &__config_view_annot);
    return vbox;
}


#define APPEND_ENTRY(text, widget, i) \
{ \
    label = gtk_label_new (NULL); \
    gtk_label_set_text_with_mnemonic (GTK_LABEL (label), text); \
    gtk_widget_show (label); \
    gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5); \
    gtk_misc_set_padding (GTK_MISC (label), 4, 0); \
    gtk_table_attach (GTK_TABLE (table), label, 0, 1, i, i+1, \
                      (GtkAttachOptions) (GTK_FILL), \
                      (GtkAttachOptions) (GTK_FILL), 4, 4); \
    widget = gtk_entry_new (); \
    gtk_widget_show (widget); \
    gtk_table_attach (GTK_TABLE (table), widget, 1, 2, i, i+1, \
                      (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), \
                      (GtkAttachOptions) (GTK_FILL), 4, 4); \
}


static GtkWidget *
create_keyboard_page (unsigned int page)
{
    GtkWidget *table;
    GtkWidget *label;

    if (page >= __key_conf_pages_num)
        return NULL;

    KeyboardConfigData *data = __key_conf_pages[page].data;

    table = gtk_table_new (3, 3, FALSE);
    gtk_widget_show (table);

    // Create keyboard setting.
    for (unsigned int i = 0; data[i].key; ++ i) {
        APPEND_ENTRY(_(data[i].label), data[i].entry, i);
        gtk_entry_set_editable (GTK_ENTRY (data[i].entry), FALSE);

        data[i].button = gtk_button_new_with_label ("...");
        gtk_widget_show (data[i].button);
        gtk_table_attach (GTK_TABLE (table), data[i].button, 2, 3, i, i+1,
                          (GtkAttachOptions) (GTK_FILL),
                          (GtkAttachOptions) (GTK_FILL), 4, 4);
        gtk_label_set_mnemonic_widget (GTK_LABEL (label), data[i].button);
    }

    for (unsigned int i = 0; data[i].key; ++ i) {
        g_signal_connect ((gpointer) data[i].button, "clicked",
                          G_CALLBACK (on_default_key_selection_clicked),
                          &(data[i]));
        g_signal_connect ((gpointer) data[i].entry, "changed",
                          G_CALLBACK (on_default_editable_changed),
                          &(data[i].data));
    }

    if (!__widget_tooltips)
        __widget_tooltips = gtk_tooltips_new();
    for (unsigned int i = 0; data[i].key; ++ i) {
        gtk_tooltips_set_tip (__widget_tooltips, data[i].entry,
                              _(data[i].tooltip), NULL);
    }

    return table;
}

static GtkWidget *
create_setup_window ()
{
    static GtkWidget *window = NULL;

    if (!window) {
        GtkWidget *notebook = gtk_notebook_new();
        GtkWidget *page;
        GtkWidget *label;
        gtk_widget_show (notebook);
        window = notebook;
        gtk_notebook_set_scrollable (GTK_NOTEBOOK (notebook), TRUE);

        // Create the first page.
        page = create_options_page ();
        label = gtk_label_new (_("Options"));
        gtk_widget_show (label);
        gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page, label);

        // Create the key bind pages.
        for (unsigned int i = 0; i < __key_conf_pages_num; i++) {
            page = create_keyboard_page (i);
            label = gtk_label_new (_(__key_conf_pages[i].label));
            gtk_widget_show (label);
            gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page, label);
        }

        // for preventing enabling left arrow.
        gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook), 1);
        gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook), 0);

        setup_widget_value ();
    }

    return window;
}

static void
setup_combo_value (GtkCombo *combo,
                   ComboConfigData *data, const String & str)
{
    GList *list = NULL;
    const char *defval = NULL;

    for (unsigned int i = 0; data[i].label; i++) {
        list = g_list_append (list, (gpointer) _(data[i].label));
        if (!strcmp (data[i].data, str.c_str ()))
            defval = _(data[i].label);
    }

    gtk_combo_set_popdown_strings (combo, list);
    g_list_free (list);

    if (defval)
        gtk_entry_set_text (GTK_ENTRY (combo->entry), defval);
}

static void
setup_widget_value ()
{
    if (__widget_selection_style) {
        setup_combo_value (GTK_COMBO (__widget_selection_style),
                           selection_style, __config_selection_style);
    }

    if (__widget_view_annot) {
        gtk_toggle_button_set_active (
            GTK_TOGGLE_BUTTON (__widget_view_annot),
            __config_view_annot);
    }

    if (__widget_listsize) {
        gtk_spin_button_set_value (
            GTK_SPIN_BUTTON (__widget_listsize),
            __config_listsize);
    }

    if (__config_sysdict.entry) {
        gtk_entry_set_text (GTK_ENTRY (__config_sysdict.entry),
                            __config_sysdict.data.c_str());
    }

    if (__widget_userdict) {
        gtk_entry_set_text (GTK_ENTRY (__widget_userdict),
                            __config_userdict.c_str());
    }

    for (unsigned int j = 0; j < __key_conf_pages_num; ++j) {
        for (unsigned int i = 0; __key_conf_pages[j].data[i].key; ++ i) {
            if (__key_conf_pages[j].data[i].entry) {
                gtk_entry_set_text (
                    GTK_ENTRY (__key_conf_pages[j].data[i].entry),
                    __key_conf_pages[j].data[i].data.c_str ());
            }
        }
    }
}

static void
load_config (const ConfigPointer &config)
{
    if (!config.null ()) {
        __config_sysdict.data =
            config->read (String (SCIM_SKK_CONFIG_SYSDICT),
                          __config_sysdict.data);
        __config_userdict =
            config->read (String (SCIM_SKK_CONFIG_USERDICT),
                          __config_userdict);
        __config_listsize =
            config->read (String (SCIM_SKK_CONFIG_DICT_LISTSIZE),
                          __config_listsize);
        __config_view_annot =
            config->read (String (SCIM_SKK_CONFIG_DICT_VIEW_ANNOT),
                          __config_view_annot);
        __config_selection_style =
            config->read (String (SCIM_SKK_CONFIG_SELECTION_STYLE),
                          __config_selection_style);

        for (unsigned int j = 0; j < __key_conf_pages_num; ++ j) {
            for (unsigned int i = 0; __key_conf_pages[j].data[i].key; ++ i) {
                __key_conf_pages[j].data[i].data =
                    config->read (String (__key_conf_pages[j].data[i].key),
                                  __key_conf_pages[j].data[i].data);
            }
        }

        setup_widget_value ();

        __have_changed = false;
    }
}

static void
save_config (const ConfigPointer &config)
{
    if (!config.null ()) {
        config->write (String (SCIM_SKK_CONFIG_SYSDICT),
                       __config_sysdict.data);
        config->write (String (SCIM_SKK_CONFIG_USERDICT),
                       __config_userdict);
        config->write (String (SCIM_SKK_CONFIG_DICT_LISTSIZE),
                        __config_listsize);
        config->write (String (SCIM_SKK_CONFIG_DICT_VIEW_ANNOT),
                        __config_view_annot);
        config->write (String (SCIM_SKK_CONFIG_SELECTION_STYLE),
                        __config_selection_style);

        for (unsigned int j = 0; j < __key_conf_pages_num; j++) {
            for (unsigned int i = 0; __key_conf_pages[j].data[i].key; ++ i) {
                config->write (String (__key_conf_pages[j].data[i].key),
                               __key_conf_pages[j].data[i].data);
            }
        }

        __have_changed = false;
    }
}

static bool
query_changed ()
{
    return __have_changed;
}


static void
on_default_spin_button_changed (GtkSpinButton *spin_button,
                                gpointer       user_data)
{
    gint *n = static_cast <gint*> (user_data);

    if (n) {
        *n = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (spin_button));
        __have_changed = true;
    }
}

static void
on_default_toggle_button_toggled (GtkToggleButton *togglebutton,
                                  gpointer         user_data)
{
    bool *toggle = static_cast<bool*> (user_data);

    if (toggle) {
        *toggle = gtk_toggle_button_get_active (togglebutton);
        __have_changed = true;
    }
}

static void
on_default_editable_changed (GtkEditable *editable,
                             gpointer     user_data)
{
    String *str = static_cast <String *> (user_data);

    if (str) {
        *str = String (gtk_entry_get_text (GTK_ENTRY (editable)));
        __have_changed = true;
    }
}

static void
on_default_file_selection_clicked (GtkButton *button,
                                   gpointer   user_data)
{
    FileConfigData *data = static_cast <FileConfigData *> (user_data);

    if (data) {
        GtkWidget *dialog = gtk_file_selection_new (_(data->title));
        gint result;

        gtk_file_selection_set_filename(GTK_FILE_SELECTION (dialog),
            gtk_entry_get_text(GTK_ENTRY(data->entry)));

        result = gtk_dialog_run (GTK_DIALOG (dialog));

        if (result == GTK_RESPONSE_OK) {
            const gchar *fname =
                gtk_file_selection_get_filename (GTK_FILE_SELECTION (dialog));

            if (!fname) fname = "";

            if (strcmp (fname, gtk_entry_get_text (GTK_ENTRY (data->entry))) != 0)
                gtk_entry_set_text (GTK_ENTRY (data->entry), fname);
        }

        gtk_widget_destroy (dialog);
    }
}

static void
on_default_key_selection_clicked (GtkButton *button,
                                  gpointer   user_data)
{
    KeyboardConfigData *data = static_cast <KeyboardConfigData *> (user_data);

    if (data) {
        GtkWidget *dialog = scim_key_selection_dialog_new (_(data->title));
        gint result;

        scim_key_selection_dialog_set_keys (
            SCIM_KEY_SELECTION_DIALOG (dialog),
            gtk_entry_get_text (GTK_ENTRY (data->entry)));

        result = gtk_dialog_run (GTK_DIALOG (dialog));

        if (result == GTK_RESPONSE_OK) {
            const gchar *keys = scim_key_selection_dialog_get_keys (
                            SCIM_KEY_SELECTION_DIALOG (dialog));

            if (!keys) keys = "";

            if (strcmp (keys, gtk_entry_get_text (GTK_ENTRY (data->entry))) != 0)
                gtk_entry_set_text (GTK_ENTRY (data->entry), keys);
        }

        gtk_widget_destroy (dialog);
    }
}

static void
on_default_combo_changed (GtkEditable *editable,
                          gpointer user_data)
{
    String *str = static_cast<String *> (user_data);
    ComboConfigData *data
        = static_cast<ComboConfigData *> (g_object_get_data (G_OBJECT (editable),
                                                             DATA_POINTER_KEY));

    if (!str) return;
    if (!data) return;

    const char *label =  gtk_entry_get_text (GTK_ENTRY (editable));

    for (unsigned int i = 0; data[i].label; i++) {
        if (label && !strcmp (_(data[i].label), label)) {
            *str = data[i].data;
            __have_changed = true;
            break;
        }
    }
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
