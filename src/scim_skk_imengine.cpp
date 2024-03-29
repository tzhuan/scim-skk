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
#include "scim_skk_intl.h"
#include "scim_skk_config.h"
#include "scim_skk_history.h"
#include "scim_skk_style_file.h"

using namespace scim_skk;

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

#define SCIM_PROP_PERIOD_STYLE                    SCIM_PROP_PREFIX"/PeriodMode"
#define SCIM_PROP_PERIOD_STYLE_TEN_MARU           SCIM_PROP_PERIOD_STYLE"/TenMaru"
#define SCIM_PROP_PERIOD_STYLE_COMMA_PERIOD       SCIM_PROP_PERIOD_STYLE"/CommaPeriod"
#define SCIM_PROP_PERIOD_STYLE_HALF_COMMA_PERIOD  SCIM_PROP_PERIOD_STYLE"/HalfCommaPeriod"
#define SCIM_PROP_PERIOD_STYLE_COMMA_MARU         SCIM_PROP_PERIOD_STYLE"/CommaMaru"


#ifndef SCIM_SKK_ICON_FILE
#define SCIM_SKK_ICON_FILE           (SCIM_ICONDIR"/scim-skk.png")
#endif


static ConfigPointer  _scim_config (0);
static SKKDictionary *scim_skkdict = 0;
static History        scim_skkhistory;


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


SKKFactory::SKKFactory (const String &lang,
                        const String &uuid,
                        const ConfigPointer &config)
    :  m_uuid(uuid),
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
    return utf8_mbstowcs("(C) 2004-2005 Jun Mukai <mukai@jmuk.org>");
}

WideString
SKKFactory::get_credits () const
{
    return WideString();
}

WideString
SKKFactory::get_help () const
{
    return utf8_mbstowcs(
#include "scim_skk_help_message.txt"
);
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
        std::vector<String> sysdicts_default;

        scim_split_string_list (sysdicts_default,
                                String (SCIM_SKK_CONFIG_SYSDICT_DEFAULT));
        m_sysdicts = config->read(String(SCIM_SKK_CONFIG_SYSDICT),
                                  sysdicts_default);
        if (m_sysdicts.size() > 0) {
            for (std::vector<String>::const_iterator it = m_sysdicts.begin();
                 it != m_sysdicts.end(); it++) {
                scim_skkdict->add_sysdict(*it);
            }
        } else {
            scim_skkdict->add_sysdict(String(SCIM_SKK_CONFIG_SYSDICT_DEFAULT));
        }
        m_userdictname = config->read(String(SCIM_SKK_CONFIG_USERDICT),
                                      String(SCIM_SKK_CONFIG_USERDICT_DEFAULT));
        scim_skkdict->set_userdict(m_userdictname, scim_skkhistory);
        candvec_size =
            config->read(String(SCIM_SKK_CONFIG_CANDVEC_SIZE),
                         SCIM_SKK_CONFIG_CANDVEC_SIZE_DEFAULT);
        annot_view =
            config->read(String(SCIM_SKK_CONFIG_ANNOT_VIEW),
                         SCIM_SKK_CONFIG_ANNOT_VIEW_DEFAULT);
        str = config->read(String(SCIM_SKK_CONFIG_ANNOT_POS),
                           String(SCIM_SKK_CONFIG_ANNOT_POS_DEFAULT));
        if (islower(str[0])) str[0] = toupper(str[0]);
        annot_pos = (str == String("Inline"));
        str = config->read(String(SCIM_SKK_CONFIG_ANNOT_TARGET),
                           String(SCIM_SKK_CONFIG_ANNOT_TARGET_DEFAULT));
        annot_target = (str == String("all"));

        annot_highlight =
            config->read(String(SCIM_SKK_CONFIG_ANNOT_HIGHLIGHT),
                         SCIM_SKK_CONFIG_ANNOT_HIGHLIGHT_DEFAULT);
        str = config->read(String(SCIM_SKK_CONFIG_ANNOT_BGCOLOR),
                           String(SCIM_SKK_CONFIG_ANNOT_BGCOLOR_DEFAULT));
        annot_bgcolor = strtol(str.c_str() + 1, (char**)NULL, 16);

        ignore_return =
            config->read(String(SCIM_SKK_CONFIG_IGNORE_RETURN),
                         SCIM_SKK_CONFIG_IGNORE_RETURN_DEFAULT);

        str = config->read(String(SCIM_SKK_CONFIG_STYLE_FILENAME), String());
        key2kana.clear_rules();
        {
            /* read style files */
            StyleFile sfile;
            static const String romaji = "RomajiTable/FundamentalTable";
            static const String kana   = "KanaTable/FundamentalTable";
            if (sfile.load(str.c_str())) {
                if (!sfile.get_key2kana_table(key2kana, romaji)) {
                    sfile.get_key2kana_table(key2kana, kana);
                }
            } else {
                /* default settings */
                key2kana.set_rules(romakana_table);
            }
        }

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
        str = config->read(String(SCIM_SKK_CONFIG_COMPLETION_KEY),
                           String(SCIM_SKK_CONFIG_COMPLETION_KEY_DEFAULT));
        m_keybind.set_completion_keys(str);
        str = config->read(String(SCIM_SKK_CONFIG_COMPLETION_BACK_KEY),
                           String(SCIM_SKK_CONFIG_COMPLETION_BACK_KEY_DEFAULT));
        m_keybind.set_completion_back_keys(str);
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
      m_skkcore (&(factory->m_keybind), &(key2kana),
                 scim_skkdict, scim_skkhistory)
{
    SCIM_DEBUG_IMENGINE(1) << "Create SKK Instance : ";
}

SKKInstance::~SKKInstance ()
{
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
SKKInstance::set_period_style (PeriodStyle newstyle)
{
    SCIM_DEBUG_IMENGINE(2) << "set period style to " << newstyle << ".\n";
    if (key2kana.get_period_style() == newstyle) {
        return;
    }

    const char *label = "";

    switch (newstyle) {
    case PERIOD_STYLE_TEN_MARU:
        label = "\xE3\x80\x81\xE3\x80\x82";
        break;
    case PERIOD_STYLE_COMMA_PERIOD:
        label = "\xEF\xBC\x8C\xEF\xBC\x8E";
        break;
    case PERIOD_STYLE_HALF_COMMA_PERIOD:
        label = ",.";
        break;
    case PERIOD_STYLE_COMMA_MARU:
        label = "\xEF\xBC\x8C\xE3\x80\x82";
        break;
    default:
        break;
    }

    if (label && *label) {
        PropertyList::iterator it = std::find (m_properties.begin(),
                                               m_properties.end(),
                                               SCIM_PROP_PERIOD_STYLE);
        if (it != m_properties.end()) {
            it->set_label(label);
            update_property(*it);
        }
    }

    key2kana.set_period_style(newstyle);
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

        prop = Property (SCIM_PROP_PERIOD_STYLE,
                         "\xE3\x80\x81\xE3\x80\x82", String (""), _("Period Style"));
        m_properties.push_back (prop);

        prop = Property (SCIM_PROP_PERIOD_STYLE_TEN_MARU,
                         "\xE3\x80\x81\xE3\x80\x82", String (""), "\xE3\x80\x81\xE3\x80\x82");
        m_properties.push_back (prop);

        prop = Property (SCIM_PROP_PERIOD_STYLE_COMMA_PERIOD,
                         "\xEF\xBC\x8C\xEF\xBC\x8E", String (""), "\xEF\xBC\x8C\xEF\xBC\x8E");
        m_properties.push_back (prop);

        prop = Property (SCIM_PROP_PERIOD_STYLE_HALF_COMMA_PERIOD,
                         ",.", String (""), ",.");
        m_properties.push_back (prop);

        prop = Property (SCIM_PROP_PERIOD_STYLE_COMMA_MARU,
                         "\xEF\xBC\x8C\xE3\x80\x82", String (""), "\xEF\xBC\x8C\xE3\x80\x82");
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
    } else if (property == SCIM_PROP_PERIOD_STYLE_TEN_MARU) {
        set_period_style(PERIOD_STYLE_TEN_MARU);
    } else if (property == SCIM_PROP_PERIOD_STYLE_COMMA_PERIOD) {
        set_period_style(PERIOD_STYLE_COMMA_PERIOD);
    } else if (property == SCIM_PROP_PERIOD_STYLE_HALF_COMMA_PERIOD) {
        set_period_style(PERIOD_STYLE_HALF_COMMA_PERIOD);
    } else if (property == SCIM_PROP_PERIOD_STYLE_COMMA_MARU) {
        set_period_style(PERIOD_STYLE_COMMA_MARU);
    }
}
