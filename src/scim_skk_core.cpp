/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4  -*- */
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

#define Uses_SCIM_UTILITY
#define Uses_SCIM_LOOKUP_TABLE

#include <scim.h>

#include "scim_skk_core.h"
#include "conv_table.h"

static void convert_char_to_wide         (const int c,
        WideString &result,
        bool space = true);
static void convert_hiragana_to_katakana (const WideString &hira,
        WideString &kata,
        bool half = false);


SKKCore::SKKCore      (KeyBind *keybind, SKKDictionaries *dict)
    : m_keybind(keybind),
      m_dict(dict),
      m_skk_mode(SKK_MODE_HIRAGANA),
      m_input_mode(INPUT_MODE_DIRECT),
      m_learning(NULL),
      m_end_flag(false),
      m_lt_action(ACTION_NONE),
      m_show_lookup_table(false),
      m_commit_flag(false)
{
    init_key2kana();
    clear_preedit();
    clear_commit();
    clear_pending();
}

SKKCore::~SKKCore     (void)
{
    if(m_learning)
        delete m_learning;
}

void
SKKCore::init_key2kana (void)
{
    m_key2kana.set_table(skk_romakana_table);
    m_key2kana.append_table(romakana_ja_period_rule);
}

WideString &
SKKCore::get_commit_string (void)
{
    return m_commitstr;
}

void
SKKCore::get_preedit_string (WideString &result)
{
    if (!m_commitstr.empty()) {
        result += m_commitstr.substr(0, m_commit_pos);
    }

    switch (m_input_mode) {
    case INPUT_MODE_OKURI:
        result += utf8_mbstowcs("\xE2\x96\xBD");
        result += m_preeditstr;
        result += utf8_mbstowcs("*");
        result += m_okuristr;
        result += m_pendingstr;
        break;
    case INPUT_MODE_PREEDIT:
        result += utf8_mbstowcs("\xE2\x96\xBD");
        if (m_skk_mode == SKK_MODE_HIRAGANA) {
            result += m_preeditstr.substr(0, m_preedit_pos);
        } else {
            convert_hiragana_to_katakana(m_preeditstr.substr(0, m_preedit_pos),
                                         result,
                                         m_skk_mode == SKK_MODE_HALF_KATAKANA);
        }
        result += m_pendingstr;
        if (m_skk_mode == SKK_MODE_HIRAGANA) {
            result += m_preeditstr.substr(m_preedit_pos,
                                          m_preeditstr.length());
        } else {
            convert_hiragana_to_katakana(m_preeditstr.substr(m_preedit_pos,
                                         m_preeditstr.length()),
                                         result,
                                         m_skk_mode == SKK_MODE_HALF_KATAKANA);
        }
        break;
    case INPUT_MODE_CONVERTING:
        result += utf8_mbstowcs("\xE2\x96\xBC");
        result += m_cit->first;
        result += m_okuristr;
        break;
    case INPUT_MODE_LEARNING:
        result += utf8_mbstowcs("\xE2\x96\xBC");
        result += m_preeditstr;
        result += utf8_mbstowcs("\xE3\x80\x90");
        m_learning->get_preedit_string(result);
        result += utf8_mbstowcs("\xE3\x80\x91");
        break;
    case INPUT_MODE_DIRECT:
        result += m_pendingstr;
    }

    if (!m_commitstr.empty()) {
        result += m_commitstr.substr(m_commit_pos,
                                     m_commitstr.length() - m_commit_pos);
    }
}

void
SKKCore::commit_string (WideString str)
{
    m_commitstr.insert(m_commit_pos, str);
    m_commit_pos += str.length();
    m_commit_flag = true;
}

void
SKKCore::commit_or_preedit (WideString str)
{
    switch (m_input_mode) {
    case INPUT_MODE_PREEDIT:
        if (m_preedit_pos < m_preeditstr.length()) {
            m_preeditstr.insert(m_preedit_pos, str);
        } else {
            m_preeditstr += str;
        }
        m_preedit_pos += str.length();
        break;
    case INPUT_MODE_OKURI:
        m_okuristr += str;
        if (m_pendingstr.empty()) {
            m_preeditstr += m_okurihead.substr(0, 1);
            m_dict->lookup(m_preeditstr, m_cl);
            if (m_cl.empty()) {
                set_input_mode(INPUT_MODE_LEARNING);
                m_learning = new SKKCore(m_keybind, m_dict);
            } else {
                m_cit = m_cl.begin();
                set_input_mode(INPUT_MODE_CONVERTING);
            }
        }
        break;
    default:
        if (m_skk_mode == SKK_MODE_KATAKANA ||
                m_skk_mode == SKK_MODE_HALF_KATAKANA) {
            WideString katakana;
            convert_hiragana_to_katakana(str, katakana,
                                         m_skk_mode == SKK_MODE_HALF_KATAKANA);
            commit_string(katakana);
        } else {
            commit_string(str);
        }
        break;
    }
}

void
SKKCore::commit_converting (void)
{
    if (!m_cl.empty() && m_cit != m_cl.end()) {
        commit_string(m_cit->first);
        commit_string(m_okuristr);
        m_dict->write(m_preeditstr, m_cit->first);
        m_cl.clear();
        clear_preedit();
        if (m_skk_mode == SKK_MODE_LATIN) {
            set_skk_mode(SKK_MODE_HIRAGANA);
        }
    }
}


int
SKKCore::caret_pos (void)
{
    switch (m_input_mode) {
    case INPUT_MODE_DIRECT:
        return m_commit_pos;
    case INPUT_MODE_PREEDIT:
        return m_commit_pos + m_preedit_pos + 1;
    case INPUT_MODE_OKURI:
        return m_commit_pos + m_preeditstr.length() + 2 + m_pendingstr.length();
    case INPUT_MODE_CONVERTING:
        return m_commit_pos + m_cit->first.length() + m_okuristr.length() + 1;
    case INPUT_MODE_LEARNING:
        return m_commit_pos + m_preeditstr.length() + 2 + m_learning->caret_pos();
    }
}

void
SKKCore::move_preedit_caret (int pos)
{
    if (pos < 0)
        return;

    switch (m_input_mode) {
    case INPUT_MODE_DIRECT:
        if (pos >= 0 && pos <= m_commitstr.length()) {
            m_commit_pos = pos;
        }
        break;
    case INPUT_MODE_PREEDIT:
        if (pos < m_commit_pos) {
            m_commit_pos = pos;
        } else if (pos > m_commit_pos &&
                   pos <= m_commit_pos + 1 + m_preeditstr.length()) {
            m_preedit_pos = pos - m_commit_pos - 1;
            clear_pending();
        } else if (pos > m_commit_pos + m_preeditstr.length() + 1 &&
                   pos <= m_commitstr.length() + m_preeditstr.length() + 1) {
            m_commit_pos = pos - m_preeditstr.length() - 1;
        }
        break;
    case INPUT_MODE_OKURI:
        if (pos < m_commit_pos) {
            m_commit_pos = pos;
        } else if (pos > m_commit_pos +
                   m_preeditstr.length() + m_pendingstr.length() + 2 &&
                   pos <= m_commitstr.length() +
                   m_preeditstr.length() + m_pendingstr.length() + 2) {
            m_commit_pos = pos - m_preeditstr.length() - m_pendingstr.length() - 2;
        }
        break;
    case INPUT_MODE_CONVERTING:
        if (pos < m_commit_pos) {
            m_commit_pos = pos;
        } else if (pos > m_commit_pos +
                   m_cit->first.length() + m_okuristr.length() + 1 &&
                   pos <= m_commitstr.length() +
                   m_cit->first.length() + m_okuristr.length() + 1) {
            m_commit_pos = pos - m_cit->first.length() - m_okuristr.length() - 1;
        }
        break;
    case INPUT_MODE_LEARNING:
        pos -= m_commitstr.length();
        pos -= m_preeditstr.length() + 2;
        m_learning->move_preedit_caret(pos);
        break;
    }
}

void
SKKCore::set_skk_mode (SKKMode newmode)
{
    if (m_learning) {
        m_learning->set_skk_mode(newmode);
    } else {
        clear_pending();
        m_skk_mode = newmode;
    }
}

SKKMode
SKKCore::get_skk_mode (void)
{
    if (m_learning) {
        return m_learning->get_skk_mode();
    } else {
        return m_skk_mode;
    }
}

void
SKKCore::set_input_mode (InputMode newmode)
{
    if (m_learning) {
        m_learning->set_input_mode(newmode);
    } else {
        m_input_mode = newmode;
    }
}

InputMode
SKKCore::get_input_mode (void)
{
    if (m_learning) {
        return m_learning->m_input_mode;
    } else {
        return m_input_mode;
    }
}


void
SKKCore::clear_pending (void)
{
    m_pendingstr.clear();
    m_key2kana.clear();
}


void
SKKCore::clear_preedit (void)
{
    m_preeditstr.clear();
    m_preedit_pos = 0;
    m_okuristr.clear();
    m_okurihead.clear();
}

void
SKKCore::clear_commit (void)
{
    m_commit_flag = false;
    m_commit_pos = 0;
    m_commitstr.clear();
}

void
SKKCore::clear (void)
{
    clear_pending();
    clear_preedit();
    m_commit_flag = false;
}

bool
SKKCore::action_kakutei_keys (void)
{
    switch (m_input_mode) {
    case INPUT_MODE_DIRECT:
        if (m_pendingstr.empty() && m_preeditstr.empty()) {
            m_end_flag = true;
        } else {
            clear_pending();
        }
        break;
    case INPUT_MODE_PREEDIT:
    case INPUT_MODE_OKURI:
        set_input_mode(INPUT_MODE_DIRECT);
        if (m_preeditstr.length() > 0) {
            if (m_skk_mode == SKK_MODE_KATAKANA ||
                    m_skk_mode == SKK_MODE_HALF_KATAKANA) {
                WideString katakana;
                convert_hiragana_to_katakana(m_preeditstr, katakana,
                                             m_skk_mode == SKK_MODE_HALF_KATAKANA);
                commit_string(katakana);
            } else {
                commit_string(m_preeditstr);
            }
            clear_preedit();
        }
        clear_pending();
        break;
    case INPUT_MODE_CONVERTING:
        commit_converting();
        set_input_mode(INPUT_MODE_DIRECT);
        break;
    }
    if(m_skk_mode == SKK_MODE_LATIN || m_skk_mode == SKK_MODE_WIDE_LATIN) {
        set_skk_mode(SKK_MODE_HIRAGANA);
    }
    return true;
}

bool
SKKCore::action_cancel_keys (void)
{
    bool retval = true;
    switch (m_input_mode) {
    case INPUT_MODE_DIRECT:
        if (!m_pendingstr.empty()) {
            clear_pending();
        } else {
            clear_commit();
            m_end_flag = true;
            retval = false;
        }
        break;
    case INPUT_MODE_PREEDIT:
    case INPUT_MODE_OKURI:
        clear_preedit();
        clear_pending();
        set_input_mode(INPUT_MODE_DIRECT);
        if (m_skk_mode == SKK_MODE_LATIN) {
            set_skk_mode(SKK_MODE_HIRAGANA);
        }
        break;
    case INPUT_MODE_CONVERTING:
        set_input_mode(INPUT_MODE_PREEDIT);
        m_cl.clear();
        break;
    }
    return retval;
}

bool
SKKCore::action_convert_keys (void)
{
    switch (m_input_mode) {
    case INPUT_MODE_CONVERTING:
        m_cit++;
        if (m_cit == m_cl.end()) {
            set_input_mode(INPUT_MODE_LEARNING);
            m_learning = new SKKCore(m_keybind, m_dict);
        }
        return true;
    case INPUT_MODE_PREEDIT:
        if (m_pendingstr == utf8_mbstowcs("n")) {
            m_preeditstr += utf8_mbstowcs("\xE3\x82\x93");
        }
        clear_pending();
        m_dict->lookup(m_preeditstr, m_cl);
        if (m_cl.empty()) {
            set_input_mode(INPUT_MODE_LEARNING);
            m_learning = new SKKCore(m_keybind, m_dict);
        } else {
            m_cit = m_cl.begin();
            set_input_mode(INPUT_MODE_CONVERTING);
        }
        return true;
    }

    return false;
}

bool
SKKCore::action_katakana_keys (bool half)
{
    switch (m_input_mode) {
    case INPUT_MODE_DIRECT:
        switch (m_skk_mode) {
        case SKK_MODE_KATAKANA:
        case SKK_MODE_HALF_KATAKANA:
            set_skk_mode(SKK_MODE_HIRAGANA);
            break;
        default:
            if (half) {
                set_skk_mode(SKK_MODE_HALF_KATAKANA);
            } else {
                set_skk_mode(SKK_MODE_KATAKANA);
            }
            break;
        }
        clear_pending();
        return true;
    case INPUT_MODE_PREEDIT:
    case INPUT_MODE_OKURI:
        if (m_preeditstr.length() > 0) {
            if (m_skk_mode == SKK_MODE_HIRAGANA) {
                WideString katakana;
                convert_hiragana_to_katakana(m_preeditstr, katakana, false);
                commit_string(katakana);
            } else {
                commit_string(m_preeditstr);
            }
            clear_preedit();
            clear_pending();
            set_input_mode(INPUT_MODE_DIRECT);
        }
        return true;
    case INPUT_MODE_CONVERTING:
        commit_converting();
        set_input_mode(INPUT_MODE_DIRECT);
        if (m_skk_mode == SKK_MODE_KATAKANA ||
                m_skk_mode == SKK_MODE_HALF_KATAKANA) {
            set_skk_mode(SKK_MODE_HIRAGANA);
        } else {
            set_skk_mode(SKK_MODE_KATAKANA);
        }
        return true;
    }

    return false;
}

bool
SKKCore::action_start_conv_keys (void)
{
    switch (m_input_mode) {
    case INPUT_MODE_DIRECT:
        set_input_mode(INPUT_MODE_PREEDIT);
        m_preedit_pos = 0;
        clear_pending();
        return true;
    case INPUT_MODE_PREEDIT:
    case INPUT_MODE_OKURI:
        if (m_preeditstr.length() > 0) {
            commit_string(m_preeditstr);
            clear_preedit();
        }
        clear_pending();
        return true;
    case INPUT_MODE_CONVERTING:
        commit_converting();
        set_input_mode(INPUT_MODE_PREEDIT);
        return true;
    default:
        return false;
    }
}

bool
SKKCore::action_prevcand_keys (void)
{
    if (m_input_mode == INPUT_MODE_CONVERTING) {
        if (m_cit != m_cl.begin()) {
            m_cit--;
        }
        return true;
    } else {
        return false;
    }
}

bool
SKKCore::action_latin_keys (bool wide)
{
    switch (m_input_mode) {
    case INPUT_MODE_PREEDIT:
    case INPUT_MODE_OKURI:
        commit_string(m_preeditstr);
        clear_preedit();
        set_input_mode(INPUT_MODE_DIRECT);
        break;
    case INPUT_MODE_CONVERTING:
        commit_converting();
        set_input_mode(INPUT_MODE_DIRECT);
    }
    clear_pending();
    if (wide) {
        set_skk_mode(SKK_MODE_WIDE_LATIN);
    } else {
        set_skk_mode(SKK_MODE_LATIN);
    }
    return true;
}

bool
SKKCore::action_latin_convert_keys (void)
{
    switch (m_input_mode) {
    case INPUT_MODE_CONVERTING:
        commit_converting();
    case INPUT_MODE_DIRECT:
        set_skk_mode(SKK_MODE_LATIN);
        set_input_mode(INPUT_MODE_PREEDIT);
        clear_preedit();
        clear_pending();
        return true;
    default:
        return false;
    }
}

bool
SKKCore::action_backspace_keys (void)
{
    if (m_pendingstr.empty()) {
        switch (m_input_mode) {
        case INPUT_MODE_CONVERTING:
            set_input_mode(INPUT_MODE_PREEDIT);
            m_cl.clear();
            break;
        case INPUT_MODE_PREEDIT:
            if (m_preedit_pos == 0) {
                commit_string(m_preeditstr);
                clear_preedit();
                set_input_mode(INPUT_MODE_DIRECT);
            } else {
                m_preeditstr.erase(m_preedit_pos-1, 1);
                m_preedit_pos--;
            }
            break;
        case INPUT_MODE_DIRECT:
            if (m_commit_pos == 0) {
                clear_commit();
                m_end_flag = true;
                return false;
            } else {
                m_commitstr.erase(m_commit_pos-1, 1);
                m_commit_pos--;
            }
        }
    } else {
        if (m_input_mode == INPUT_MODE_OKURI &&
            m_pendingstr.length() == 1) {
            clear_pending();
            set_input_mode(INPUT_MODE_PREEDIT);
            m_preedit_pos = m_preeditstr.length();
        } else {
            m_pendingstr.erase(m_pendingstr.length()-1);
            m_key2kana.set_pending(m_pendingstr);
        }
    }

    return true;
}

bool
SKKCore::action_delete_keys (void)
{
    if (m_pendingstr.empty()) {
        switch (m_input_mode) {
        case INPUT_MODE_CONVERTING:
            set_input_mode(INPUT_MODE_PREEDIT);
            m_cl.clear();
            break;
        case INPUT_MODE_PREEDIT:
            if (m_preedit_pos < m_preeditstr.length())
                m_preeditstr.erase(m_preedit_pos, 1);
            break;
        case INPUT_MODE_DIRECT:
            if (m_commitstr.empty()) {
                clear_commit();
                m_end_flag = true;
                return false;
            } else if (m_commit_pos < m_commitstr.length()) {
                m_commitstr.erase(m_commit_pos, 1);
            }
        }
    } else {
        clear_pending();
    }

    return true;
}

bool
SKKCore::action_forward_keys (void)
{
    switch (m_input_mode) {
    case INPUT_MODE_CONVERTING:
        return false;
    case INPUT_MODE_DIRECT:
        clear_pending();
        if (m_commit_pos < m_commitstr.length()) {
            m_commit_pos++;
            return true;
        } else {
            return false;
        }
    case INPUT_MODE_PREEDIT:
        clear_pending();
        if (m_preedit_pos < m_preeditstr.length()) {
            m_preedit_pos++;
        } else if (m_commit_pos < m_commitstr.length()) {
            m_commit_pos++;
        } else {
            return false;
        }
        return true;
    }
    return false;
}

bool
SKKCore::action_backward_keys (void)
{
    switch (m_input_mode) {
    case INPUT_MODE_CONVERTING:
        return false;
    case INPUT_MODE_DIRECT:
        clear_pending();
        if (m_commit_pos > 0) {
            m_commit_pos--;
            return true;
        } else {
            return false;
        }
    case INPUT_MODE_PREEDIT:
        clear_pending();
        if (m_preedit_pos > 0) {
            m_preedit_pos--;
        } else if (m_commit_pos > 0) {
            m_commit_pos--;
        } else {
            return false;
        }
        return true;
    }
    return false;
}

bool
SKKCore::process_remaining_keybinds (const KeyEvent &key)
{
    if (m_keybind->match_katakana_keys(key))
        return action_katakana_keys(false);

    if (m_keybind->match_half_katakana_keys(key))
        return action_katakana_keys(true);

    if (m_keybind->match_start_conv_keys(key))
        return action_start_conv_keys();

    if (m_keybind->match_prevcand_keys(key))
        return action_prevcand_keys();

    if(m_keybind->match_latin_keys(key))
        return action_latin_keys(false);

    if(m_keybind->match_wide_latin_keys(key))
        return action_latin_keys(true);

    if (m_keybind->match_latin_convert_keys(key))
        return action_latin_convert_keys();

    if (m_keybind->match_backspace_keys(key))
        return action_backspace_keys();

    if (m_keybind->match_delete_keys(key))
        return action_delete_keys();

    if (m_keybind->match_forward_keys(key))
        return action_forward_keys();

    if (m_keybind->match_backward_keys(key))
        return action_backward_keys();

    return false;
}

bool
SKKCore::process_latin (const KeyEvent &key)
{
    if (m_keybind->match_kakutei_keys(key))
        return action_kakutei_keys();

    if (m_keybind->match_cancel_keys(key))
        return action_cancel_keys();

    if (m_input_mode == INPUT_MODE_PREEDIT && m_keybind->match_convert_keys(key))
        return action_convert_keys();

    if (!(key.mask & SCIM_KEY_ControlMask || key.mask & SCIM_KEY_Mod1Mask ||
            key.mask & SCIM_KEY_Mod2Mask    || key.mask & SCIM_KEY_Mod3Mask ||
            key.mask & SCIM_KEY_Mod4Mask    || key.mask & SCIM_KEY_Mod5Mask ) &&
            isprint(key.code)) {
        char str[2];
        str[0] = key.code;
        str[1] = '\0';
        commit_or_preedit(utf8_mbstowcs(str));
        return true;
    }

    return process_remaining_keybinds(key);
}

bool
SKKCore::process_wide_latin (const KeyEvent &key)
{
    if (m_keybind->match_kakutei_keys(key))
        return action_kakutei_keys();

    if (m_keybind->match_cancel_keys(key))
        return action_cancel_keys();

    if (!(key.mask & SCIM_KEY_ControlMask || key.mask & SCIM_KEY_Mod1Mask ||
            key.mask & SCIM_KEY_Mod2Mask    || key.mask & SCIM_KEY_Mod3Mask ||
            key.mask & SCIM_KEY_Mod4Mask    || key.mask & SCIM_KEY_Mod5Mask ) &&
            isprint(key.code)) {
        WideString result;

        convert_char_to_wide(key.code, result);
        commit_string(result);
        return true;
    }

    return process_remaining_keybinds(key);
}

bool
SKKCore::process_romakana (const KeyEvent &key)
{
    if (m_keybind->match_kakutei_keys(key))
        return action_kakutei_keys();
    if (m_keybind->match_cancel_keys(key))
        return action_cancel_keys();

    if (m_input_mode == INPUT_MODE_PREEDIT ||
        m_input_mode == INPUT_MODE_OKURI) {
        if (m_keybind->match_convert_keys(key))
            return action_convert_keys();
    }

    if (!(key.mask & SCIM_KEY_ControlMask || key.mask & SCIM_KEY_Mod1Mask ||
          key.mask & SCIM_KEY_Mod2Mask    || key.mask & SCIM_KEY_Mod3Mask ||
          key.mask & SCIM_KEY_Mod4Mask    || key.mask & SCIM_KEY_Mod5Mask ) &&
        isprint(key.code)) {
        if (isalpha(key.code)) {
            bool f = false;
            if (key.mask & SCIM_KEY_ShiftMask) {
                if (m_input_mode == INPUT_MODE_PREEDIT &&
                    !m_preeditstr.empty()) {
                    f = true;
                } else if (m_input_mode != INPUT_MODE_OKURI) {
                    set_input_mode(INPUT_MODE_PREEDIT);
                }
            }
            char str[2];
            WideString result;
            str[0] = (char)tolower(key.code);
            str[1] = '\0';
            m_key2kana.append(String(str), result, m_pendingstr);

            if (m_input_mode == INPUT_MODE_OKURI && !m_pendingstr.empty() &&
                result.empty()) {
                m_okurihead = m_pendingstr;
            }

            if (f) {
                m_okurihead = utf8_mbstowcs(str);
                m_preeditstr.erase(m_preedit_pos);
                set_input_mode(INPUT_MODE_OKURI);
                if (f && m_pendingstr.empty() && !result.empty()) {
                    commit_or_preedit(result);
                }
                return true;
            } else if (result.length() > 0) {
                commit_or_preedit(result);
            } else {
                process_remaining_keybinds(key);
            }
            return true;
        } else {
            char str[2];
            WideString result;
            str[0] = (char)key.code;
            str[1] = '\0';

            if (m_pendingstr == utf8_mbstowcs("z")) {
                m_key2kana.append(String(str), result, m_pendingstr);
                if (result.length() > 0) {
                    commit_or_preedit(result);
                    return true;
                }
            }

            if (process_remaining_keybinds(key)) {
                return true;
            }

            clear_pending();
            m_key2kana.append(String(str), result, m_pendingstr);
            if (result.length() > 0) {
                commit_or_preedit(result);
                return true;
            } else {
                clear_pending();
                return false;
            }
        }
    }

    return process_remaining_keybinds(key);
}

bool
SKKCore::process_key_event (const KeyEvent key)
{
    // ignore key release.
    if (key.is_key_release())
        return false;

    // ignore modifier keys
    if (key.code == SCIM_KEY_Shift_L || key.code == SCIM_KEY_Shift_R ||
            key.code == SCIM_KEY_Control_L || key.code == SCIM_KEY_Control_R ||
            key.code == SCIM_KEY_Alt_L || key.code == SCIM_KEY_Alt_R)
        return false;


    if (m_input_mode == INPUT_MODE_CONVERTING) {
        if (m_keybind->match_kakutei_keys(key))
            return action_kakutei_keys();
        if (m_keybind->match_cancel_keys(key))
            return action_cancel_keys();
        if (m_keybind->match_convert_keys(key))
            return action_convert_keys();
        if (m_keybind->match_prevcand_keys(key))
            return action_prevcand_keys();

        commit_converting();
        set_input_mode(INPUT_MODE_DIRECT);
        if (m_skk_mode == SKK_MODE_LATIN) {
            set_skk_mode(SKK_MODE_HIRAGANA);
        }
    }

    if (m_input_mode == INPUT_MODE_LEARNING) {
        bool retval = m_learning->process_key_event(key);
        if (m_learning->m_end_flag) {
            if (m_learning->m_commitstr.empty()) {
                delete m_learning;
                m_learning = NULL;
                if (m_cl.empty()) {
                    set_input_mode(INPUT_MODE_PREEDIT);
                } else {
                    set_input_mode(INPUT_MODE_CONVERTING);
                    m_cit--;
                }
            } else {
                commit_string(m_learning->m_commitstr);
                commit_string(m_okuristr);
                m_cl.clear();
                m_dict->write(m_preeditstr, m_learning->m_commitstr);
                clear_preedit();
                delete m_learning;
                m_learning = NULL;
                set_input_mode(INPUT_MODE_DIRECT);
            }
        }
        return retval;
    }

    switch (m_skk_mode) {
    case SKK_MODE_LATIN:
        return process_latin(key);
    case SKK_MODE_WIDE_LATIN:
        return process_wide_latin(key);
    default:
        return process_romakana(key);
    }
}

bool
SKKCore::show_lookup_table (void)
{
    return false;
}

void
SKKCore::update_lookup_table (CommonLookupTable &tbl)
{
}

static void
convert_char_to_wide (const int c, WideString & wide, bool space)
{
    if (!isprint(c))
        return;

    char cc[2]; cc[0] = c; cc[1] = '\0';
    bool found = false;

    for (unsigned int i = 0; ja_wide_table[i].code; i++) {
        if (ja_wide_table[i].code && *ja_wide_table[i].code == c) {
            wide += utf8_mbstowcs (ja_wide_table[i].wide);
            found = true;
            break;
        }
    }

    if (!found && space && c == ' ') {
        wide += utf8_mbstowcs ("\xE3\x80\x80");
        found = true;
    }

    if (!found)
        wide += utf8_mbstowcs (cc);
}

static void
convert_hiragana_to_katakana (const WideString & hira, WideString & kata,
                              bool half)
{
    if (hira.length () < 0)
        return;

    for (unsigned int i = 0; i < hira.length (); i++) {
        WideString tmpwide;
        bool found = false;

        for (unsigned int j = 0; ja_hiragana_katakana_table[j].hiragana; j++) {
            tmpwide = utf8_mbstowcs (ja_hiragana_katakana_table[j].hiragana);
            if (hira.substr(i, 1) == tmpwide) {
                if (half)
                    kata += utf8_mbstowcs (ja_hiragana_katakana_table[j].half_katakana);
                else
                    kata += utf8_mbstowcs (ja_hiragana_katakana_table[j].katakana);
                found = true;
                break;
            }
        }

        if (!found)
            kata += hira.substr(i, 1);
    }
}
