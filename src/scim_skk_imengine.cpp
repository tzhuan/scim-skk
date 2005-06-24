/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 4  -*- */
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
 * The original code is scim_anthy_imengine.cpp in scim-anthy-0.2.0. 
 * Copyright (C) 2004 Takuro Ashie <ashie@homa.ne.jp>
 */

#define Uses_SCIM_UTILITY
#define Uses_SCIM_IMENGINE
/* #define Uses_SCIM_LOOKUP_TABLE */
#define Uses_SCIM_CONFIG_BASE

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <scim.h>
#include "scim_skk_imengine.h"
#include "scim_skk_prefs.h"
#include "conv_table.h"

#include <string.h>

#ifdef HAVE_GETTEXT
#include <libintl.h>
#define _(String) dgettext(GETTEXT_PACKAGE,String)
#define N_(String) (String)
#else
#define _(String) (String)
#define N_(String) (String)
#define bindtextdomain(Package,Directory)
#define textdomain(domain)
#define bind_textdomain_codeset(domain,codeset)
#endif

#define scim_module_init skk_LTX_scim_module_init
#define scim_module_exit skk_LTX_scim_module_exit
#define scim_imengine_module_init skk_LTX_scim_imengine_module_init
#define scim_imengine_module_create_factory skk_LTX_scim_imengine_module_create_factory

#define SCIM_CONFIG_IMENGINE_SKK_UUID       "/IMEngine/SKK/UUID-"

#define SCIM_PROP_PREFIX                    "/IMEngine/SKK"
#define SCIM_PROP_MODE_PREFIX               SCIM_PROP_PREFIX"/InputMode"
#define SCIM_PROP_INPUT_MODE_HIRAGANA       SCIM_PROP_MODE_PREFIX"/Hiragana"
#define SCIM_PROP_INPUT_MODE_KATAKANA       SCIM_PROP_MODE_PREFIX"/Katakana"
#define SCIM_PROP_INPUT_MODE_HALF_KATAKANA  SCIM_PROP_MODE_PREFIX"/HalfKatakana"
#define SCIM_PROP_INPUT_MODE_ASCII          SCIM_PROP_MODE_PREFIX"/ASCII"
#define SCIM_PROP_INPUT_MODE_WIDE_ASCII     SCIM_PROP_MODE_PREFIX"/WideASCII"

#ifndef SCIM_SKK_ICON_FILE
#define SCIM_SKK_ICON_FILE           (SCIM_ICONDIR"/scim-skk.png")
#endif



static ConfigPointer  _scim_config (0);
SKKDictionary *scim_skkdict = 0;

extern "C" {
    void scim_module_init (void)
    {
        bindtextdomain (GETTEXT_PACKAGE, SCIM_SKK_LOCALEDIR);
        bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    }

    void scim_module_exit (void)
    {
        if (_scim_config)
            _scim_config.reset ();
        if (scim_skkdict) {
            scim_skkdict->dump_userdict();
            delete scim_skkdict;
        }
    }

    uint32 scim_imengine_module_init (const ConfigPointer &config)
    {
        SCIM_DEBUG_IMENGINE(1) << "Initialize SKK Engine.\n";

        _scim_config = config;
        scim_skkdict = new SKKDictionary();

        return 1;
    }

    IMEngineFactoryPointer scim_imengine_module_create_factory (uint32 engine)
    {
        SKKFactory *factory = 0;
        try {
            factory =
               new SKKFactory (String ("ja_JP"),
                               String ("ec43125f-f9d3-4a77-8096-de3a35290ba9"),
                               _scim_config);
        } catch (...) {
            delete factory;
            factory = 0;
        }


        return factory;
    }
}


bool annot_view = SCIM_SKK_CONFIG_ANNOT_VIEW_DEFAULT;
/* view annotation if true */
bool annot_pos =
  (strncmp(SCIM_SKK_CONFIG_ANNOT_POS_DEFAULT, "inline", 6) == 0);
/* inline if true, auxwindow if otherwise */
bool annot_target
  (strncmp(SCIM_SKK_CONFIG_ANNOT_TARGET_DEFAULT, "all", 3) == 0);
/* all if true, caret position otherwise */
int candvec_size = SCIM_SKK_CONFIG_CANDVEC_SIZE_DEFAULT;

bool annot_highlight = SCIM_SKK_CONFIG_ANNOT_HIGHLIGHT_DEFAULT;
int annot_bgcolor = strtol(SCIM_SKK_CONFIG_ANNOT_BGCOLOR_DEFAULT+1,
                           (char**)NULL, 16);

SKKFactory::SKKFactory (const String &lang,
                        const String &uuid,
                        const ConfigPointer &config)
    :  m_uuid(uuid),
       m_sysdictpath(SCIM_SKK_CONFIG_SYSDICT_DEFAULT),
       m_userdictname(SCIM_SKK_CONFIG_USERDICT_DEFAULT),
       m_config(config)
{
    SCIM_DEBUG_IMENGINE(0) << "Create SKK Factory :\n";
    SCIM_DEBUG_IMENGINE(0) << "Lang : " << lang << "\n";
    SCIM_DEBUG_IMENGINE(0) << "UUID : " << uuid << "\n";

    if (lang.length() >= 2)
        set_languages(lang);

    reload_config(m_config);
    m_reload_signal_connection = m_config->signal_connect_reload(slot(this, &SKKFactory::reload_config));
}

SKKFactory::~SKKFactory ()
{
    scim_skkdict->dump_userdict();
    m_reload_signal_connection.disconnect ();
}

WideString
SKKFactory::get_name () const
{
    return utf8_mbstowcs("SKK");
}

String
SKKFactory::get_uuid () const
{
    return m_uuid;
}

String
SKKFactory::get_icon_file () const
{
    return String(SCIM_SKK_ICON_FILE);
}

WideString
SKKFactory::get_authors () const
{
    return utf8_mbstowcs("Jun Mukai");
}

WideString
SKKFactory::get_credits () const
{
    return WideString();
}

WideString
SKKFactory::get_help () const
{
    return WideString();
}

void
SKKFactory::dump_dict (void)
{
    scim_skkdict->dump_userdict();
}

IMEngineInstancePointer
SKKFactory::create_instance (const String &encoding, int id)
{
    return new SKKInstance(this, encoding, id);
}

void
SKKFactory::reload_config (const ConfigPointer &config)
{
    if (config) {
        String str;

        m_sysdictpath = config->read(String(SCIM_SKK_CONFIG_SYSDICT),
                                     String(SCIM_SKK_CONFIG_SYSDICT_DEFAULT));
        scim_skkdict->add_sysdict(m_sysdictpath);
        m_userdictname = config->read(String(SCIM_SKK_CONFIG_USERDICT),
                                      String(SCIM_SKK_CONFIG_USERDICT_DEFAULT));
        scim_skkdict->set_userdict(m_userdictname);
        candvec_size = config->read(String(SCIM_SKK_CONFIG_CANDVEC_SIZE),
                                    SCIM_SKK_CONFIG_CANDVEC_SIZE_DEFAULT);
        annot_view = config->read(String(SCIM_SKK_CONFIG_ANNOT_VIEW),
                                  SCIM_SKK_CONFIG_ANNOT_VIEW_DEFAULT);
        str = config->read(String(SCIM_SKK_CONFIG_ANNOT_POS),
                           String(SCIM_SKK_CONFIG_ANNOT_POS_DEFAULT));
        annot_pos = (str == String("inline"));
        str = config->read(String(SCIM_SKK_CONFIG_ANNOT_TARGET),
                           String(SCIM_SKK_CONFIG_ANNOT_TARGET_DEFAULT));
        annot_target = (str == String("all"));

        annot_highlight =
            config->read(String(SCIM_SKK_CONFIG_ANNOT_HIGHLIGHT),
                         SCIM_SKK_CONFIG_ANNOT_HIGHLIGHT_DEFAULT);
        str = config->read(String(SCIM_SKK_CONFIG_ANNOT_BGCOLOR),
                           String(SCIM_SKK_CONFIG_ANNOT_BGCOLOR_DEFAULT));
        annot_bgcolor = strtol(str.c_str() + 1, (char**)NULL, 16);

        str = config->read(String(SCIM_SKK_CONFIG_KAKUTEI_KEY),
                           String(SCIM_SKK_CONFIG_KAKUTEI_KEY_DEFAULT));
        m_keybind.set_kakutei_keys(str);
        str = config->read(String(SCIM_SKK_CONFIG_KATAKANA_KEY),
                           String(SCIM_SKK_CONFIG_KATAKANA_KEY_DEFAULT));
        m_keybind.set_katakana_keys(str);
        str = config->read(String(SCIM_SKK_CONFIG_HALF_KATAKANA_KEY),
                           String(SCIM_SKK_CONFIG_HALF_KATAKANA_KEY_DEFAULT));
        m_keybind.set_half_katakana_keys(str);
        str = config->read(String(SCIM_SKK_CONFIG_ASCII_KEY),
                           String(SCIM_SKK_CONFIG_ASCII_KEY_DEFAULT));
        m_keybind.set_ascii_keys(str);
        str = config->read(String(SCIM_SKK_CONFIG_WIDE_ASCII_KEY),
                           String(SCIM_SKK_CONFIG_WIDE_ASCII_KEY_DEFAULT));
        m_keybind.set_wide_ascii_keys(str);
        str = config->read(String(SCIM_SKK_CONFIG_CONVERT_KEY),
                           String(SCIM_SKK_CONFIG_CONVERT_KEY_DEFAULT));
        m_keybind.set_convert_keys(str);
        str = config->read(String(SCIM_SKK_CONFIG_START_PREEDIT_KEY),
                           String(SCIM_SKK_CONFIG_START_PREEDIT_KEY_DEFAULT));
        m_keybind.set_start_preedit_keys(str);
        str = config->read(String(SCIM_SKK_CONFIG_CANCEL_KEY),
                           String(SCIM_SKK_CONFIG_CANCEL_KEY_DEFAULT));
        m_keybind.set_cancel_keys(str);
        str = config->read(String(SCIM_SKK_CONFIG_ASCII_CONVERT_KEY),
                           String(SCIM_SKK_CONFIG_ASCII_CONVERT_KEY_DEFAULT));
        m_keybind.set_ascii_convert_keys(str);
        str = config->read(String(SCIM_SKK_CONFIG_PREVCAND_KEY),
                           String(SCIM_SKK_CONFIG_PREVCAND_KEY_DEFAULT));
        m_keybind.set_prevcand_keys(str);
        str = config->read(String(SCIM_SKK_CONFIG_BACKSPACE_KEY),
                           String(SCIM_SKK_CONFIG_BACKSPACE_KEY_DEFAULT));
        m_keybind.set_backspace_keys(str);
        str = config->read(String(SCIM_SKK_CONFIG_DELETE_KEY),
                           String(SCIM_SKK_CONFIG_DELETE_KEY_DEFAULT));
        m_keybind.set_delete_keys(str);
        str = config->read(String(SCIM_SKK_CONFIG_FORWARD_KEY),
                           String(SCIM_SKK_CONFIG_FORWARD_KEY_DEFAULT));
        m_keybind.set_forward_keys(str);
        str = config->read(String(SCIM_SKK_CONFIG_BACKWARD_KEY),
                           String(SCIM_SKK_CONFIG_BACKWARD_KEY_DEFAULT));
        m_keybind.set_backward_keys(str);
        str = config->read(String(SCIM_SKK_CONFIG_HOME_KEY),
                           String(SCIM_SKK_CONFIG_HOME_KEY_DEFAULT));
        m_keybind.set_home_keys(str);
        str = config->read(String(SCIM_SKK_CONFIG_END_KEY),
                           String(SCIM_SKK_CONFIG_END_KEY_DEFAULT));
        m_keybind.set_end_keys(str);
        str = config->read(String(SCIM_SKK_CONFIG_UPCASE_KEY),
                           String(SCIM_SKK_CONFIG_UPCASE_KEY_DEFAULT));
        m_keybind.set_upcase_keys(str);
        str = config->read(String(SCIM_SKK_CONFIG_SELECTION_STYLE),
                           String(SCIM_SKK_CONFIG_SELECTION_STYLE_DEFAULT));
        m_keybind.set_selection_style(str);
    }
}



SKKInstance::SKKInstance (SKKFactory   *factory,
                          const String &encoding,
                          int           id)
    : IMEngineInstanceBase (factory, encoding, id),
      m_skk_mode (SKK_MODE_HIRAGANA),
      m_skkcore (&(factory->m_keybind), &(m_key2kana))
{
    SCIM_DEBUG_IMENGINE(1) << "Create SKK Instance : ";
    init_key2kana();
}

SKKInstance::~SKKInstance ()
{
}

void
SKKInstance::init_key2kana (void)
{
    m_key2kana.set_table(skk_romakana_table);
    m_key2kana.append_table(romakana_ja_period_rule);
}

void
SKKInstance::update_candidates (void)
{
    if (m_skkcore.has_commit_string()) {
        commit_string(m_skkcore.get_commit_string());
        m_skkcore.clear_commit();
    }

    WideString preedit;
    AttributeList alist;
    m_skkcore.get_preedit_string(preedit);
    m_skkcore.get_preedit_attributes(alist);
    update_preedit_string(preedit, alist);
    if (!preedit.empty()) {
        update_preedit_caret(m_skkcore.caret_pos());
        show_preedit_string();
    } else {
        hide_preedit_string();
    }

    if (annot_view && !annot_pos &&
        m_skkcore.get_input_mode() == INPUT_MODE_CONVERTING) {
        WideString auxstr;
        m_skkcore.get_lookup_table().get_annot_string(auxstr);
        update_aux_string(auxstr);
        if (!auxstr.empty())
            show_aux_string();
        else
            hide_aux_string();
    } else {
        update_aux_string(WideString());
        hide_aux_string();
    }

    if (m_skkcore.get_input_mode() == INPUT_MODE_CONVERTING &&
        m_skkcore.lookup_table_visible()) {
        update_lookup_table(m_skkcore.get_lookup_table());
        show_lookup_table();
    } else {
        hide_lookup_table();
    }
}

bool
SKKInstance::process_key_event (const KeyEvent &key)
{
    SCIM_DEBUG_IMENGINE(2) << "process_key_event.\n";
    // ignore key release.
    if (key.is_key_release())
        return false;

    // ignore modifier keys
    if (key.code == SCIM_KEY_Shift_L || key.code == SCIM_KEY_Shift_R ||
        key.code == SCIM_KEY_Control_L || key.code == SCIM_KEY_Control_R ||
        key.code == SCIM_KEY_Alt_L || key.code == SCIM_KEY_Alt_R ||
        key.code == SCIM_KEY_Meta_L || key.code == SCIM_KEY_Meta_R ||
        key.code == SCIM_KEY_Caps_Lock || key.code == SCIM_KEY_Shift_Lock
        )
        return false;

    KeyEvent k(key.code, key.mask);

    // ignore some masks
    k.mask &= ~SCIM_KEY_CapsLockMask;

    bool retval = m_skkcore.process_key_event(k);

    update_candidates();

    set_skk_mode(m_skkcore.get_skk_mode());
    return retval;
}


void
SKKInstance::set_skk_mode (SKKMode newmode)
{
    SCIM_DEBUG_IMENGINE(2) << "set input mode to " << newmode << ".\n";
    if (m_skk_mode == newmode) {
        return;
    }

    const char *label = "";

    switch (newmode) {
    case SKK_MODE_HIRAGANA:
        label = "\xE3\x81\x82";
        break;
    case SKK_MODE_KATAKANA:
        label = "\xE3\x82\xA2";
        break;
    case SKK_MODE_HALF_KATAKANA:
        label = "\xEF\xBD\xB1";
        break;
    case SKK_MODE_ASCII:
        label = "a";
        break;
    case SKK_MODE_WIDE_ASCII:
        label = "\xEF\xBD\x81";
        break;
    default:
        break;
    }

    if (label && *label) {
        PropertyList::iterator it = std::find (m_properties.begin(),
                                               m_properties.end(),
                                               SCIM_PROP_MODE_PREFIX);
        if (it != m_properties.end()) {
            it->set_label(label);
            update_property(*it);
        }
    }

    m_skk_mode = newmode;
    m_skkcore.set_skk_mode(newmode);
}

void
SKKInstance::move_preedit_caret (unsigned int pos)
{
    m_skkcore.move_preedit_caret(pos);
    update_preedit_caret(m_skkcore.caret_pos());
}

void
SKKInstance::select_candidate (unsigned int index)
{
    m_skkcore.action_select_index(index);
    if (m_skkcore.has_commit_string()) {
        commit_string(m_skkcore.get_commit_string());
        m_skkcore.clear_commit();
    }
    update_preedit_string(WideString());
    update_aux_string(WideString());
    hide_lookup_table();
    hide_preedit_string();
    hide_aux_string();
}

void
SKKInstance::update_lookup_table_page_size (unsigned int page_size)
{
    if (page_size > 0 && m_skkcore.lookup_table_visible())
        m_skkcore.get_lookup_table().set_page_size (page_size);
}

void
SKKInstance::lookup_table_page_up ()
{
    m_skkcore.action_prevpage();
    update_candidates();
}

void
SKKInstance::lookup_table_page_down ()
{
    m_skkcore.action_nextpage();
    update_candidates();
}

void
SKKInstance::reset ()
{
    m_skkcore.clear();
}

void
SKKInstance::focus_in ()
{
    WideString preedit;

    SCIM_DEBUG_IMENGINE(2) << "focus_in.\n";
    install_properties();

    update_candidates();
    set_skk_mode(m_skkcore.get_skk_mode());
}

void
SKKInstance::focus_out ()
{
    SCIM_DEBUG_IMENGINE(2) << "focus_out.\n";
}

void
SKKInstance::install_properties (void)
{
    if (m_properties.size() <= 0) {
        Property prop;

        prop = Property (SCIM_PROP_MODE_PREFIX,
                         "\xE3\x81\x82", String (""), _("Input mode"));
        m_properties.push_back (prop);

        prop = Property (SCIM_PROP_INPUT_MODE_HIRAGANA,
                         _("Hiragana"), String (""), _("Hiragana"));
        m_properties.push_back (prop);

        prop = Property (SCIM_PROP_INPUT_MODE_KATAKANA,
                         _("Katakana"), String (""), _("Katakana"));
        m_properties.push_back (prop);

        prop = Property (SCIM_PROP_INPUT_MODE_HALF_KATAKANA,
                         _("Half width katakana"), String (""),
                         _("Half width katakana"));
        m_properties.push_back (prop);

        prop = Property (SCIM_PROP_INPUT_MODE_ASCII,
                         _("ASCII"), String (""), _("Direct input"));
        m_properties.push_back (prop);

        prop = Property (SCIM_PROP_INPUT_MODE_WIDE_ASCII,
                         _("Wide ASCII"), String (""), _("Wide ASCII"));
        m_properties.push_back (prop);
    }

    register_properties(m_properties);
}

void
SKKInstance::trigger_property (const String& property)
{
    SCIM_DEBUG_IMENGINE(2) << "trigger_property : " << property << "\n";

    if (property == SCIM_PROP_INPUT_MODE_HIRAGANA) {
        set_skk_mode(SKK_MODE_HIRAGANA);
    } else if (property == SCIM_PROP_INPUT_MODE_KATAKANA) {
        set_skk_mode(SKK_MODE_KATAKANA);
    } else if (property == SCIM_PROP_INPUT_MODE_HALF_KATAKANA) {
        set_skk_mode(SKK_MODE_HALF_KATAKANA);
    } else if (property == SCIM_PROP_INPUT_MODE_ASCII) {
        set_skk_mode(SKK_MODE_ASCII);
    } else if (property == SCIM_PROP_INPUT_MODE_WIDE_ASCII) {
        set_skk_mode(SKK_MODE_WIDE_ASCII);
    }
}
