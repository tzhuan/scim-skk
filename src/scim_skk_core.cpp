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


SKKCore::SKKCore      (KeyBind *keybind, SKKDictionaries *dict,
                       SKKAutomaton *key2kana, CommonLookupTable *ltable)
    : m_keybind(keybind),
      m_dict(dict),
      m_skk_mode(SKK_MODE_HIRAGANA),
      m_input_mode(INPUT_MODE_DIRECT),
      m_learning(0),
      m_end_flag(false),
      m_ltable(ltable),
      m_show_ltable(false),
      m_key2kana(key2kana),
      m_preedit_pos(0),
      m_commit_pos(0),
      m_cindex(m_candlist.end()),
      m_commit_flag(false)
{
    m_ltable->clear();
    clear_preedit();
    clear_commit();
    clear_pending();
}

SKKCore::~SKKCore     (void)
{
    if (m_learning)
        delete m_learning;
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
        if (m_cindex != m_candlist.end())
            result += *m_cindex;
        else
            result += m_preeditstr;
        result += m_okuristr;
        break;
    case INPUT_MODE_LEARNING:
        result += utf8_mbstowcs("\xE2\x96\xBC");
        result += m_preeditstr;
        if (!m_okuristr.empty()) {
            result += utf8_mbstowcs("*");
            result += m_okuristr;
        }
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
            clear_candidate();
            m_dict->lookup(m_preeditstr + m_okurihead, m_candlist, *m_ltable);
            if (m_candlist.empty() && m_ltable->number_of_candidates() == 0) {
                set_input_mode(INPUT_MODE_LEARNING);
                m_learning = new SKKCore(m_keybind, m_dict,
                                         m_key2kana, m_ltable);
            } else {
                set_input_mode(INPUT_MODE_CONVERTING);
                if (m_candlist.empty())
                    m_show_ltable = true;
                else
                    m_cindex = m_candlist.begin();
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
SKKCore::commit_converting (int index)
{
    if (!m_candlist.empty() && m_cindex != m_candlist.end()) {
        WideString str = *m_cindex;
        if (m_dict->get_view_annot())
            m_dict->strip_annot(str);
        commit_string(str);
        commit_string(m_okuristr);
        m_dict->write(m_preeditstr, *m_cindex);
        clear_candidate();
        clear_preedit();
        if (m_skk_mode == SKK_MODE_ASCII)
            set_skk_mode(SKK_MODE_HIRAGANA);
    } else if (index >= 0 && index < m_ltable->number_of_candidates()) {
        index += m_ltable->get_current_page_start();
        WideString str = m_ltable->get_candidate(index);
        WideString cand = str;
        if (m_dict->get_view_annot())
            m_dict->strip_annot(cand);
        commit_string(cand);
        commit_string(m_okuristr);
        m_dict->write(m_preeditstr, str);
        clear_candidate();
        clear_preedit();
        if (m_skk_mode == SKK_MODE_ASCII)
            set_skk_mode(SKK_MODE_HIRAGANA);
    }
}


int
SKKCore::caret_pos (void)
{
    int base_pos = m_commit_pos + m_pendingstr.length();

    switch (m_input_mode) {
    case INPUT_MODE_DIRECT:
        return base_pos;
    case INPUT_MODE_PREEDIT:
        return base_pos + m_preedit_pos + 1;
    case INPUT_MODE_OKURI:
        return base_pos + m_preeditstr.length() + 2;
    case INPUT_MODE_CONVERTING:
        if (m_cindex != m_candlist.end())
            return base_pos + (*m_cindex).length() + m_okuristr.length() + 1;
        else
            return base_pos + m_preedit_pos + m_okuristr.length() + 1;
    case INPUT_MODE_LEARNING:
        if (!m_okuristr.empty())
            base_pos += m_okuristr.length() + 1;
        return base_pos + m_preeditstr.length() + 2 + m_learning->caret_pos();
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
                   m_cindex->length() + m_okuristr.length() + 1 &&
                   pos <= m_commitstr.length() +
                   m_cindex->length() + m_okuristr.length() + 1) {
            m_commit_pos = pos - m_cindex->length() - m_okuristr.length() - 1;
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
    } else if (m_skk_mode != newmode) {
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
    if (m_pendingstr == utf8_mbstowcs("n")) {
        commit_or_preedit(utf8_mbstowcs("\xE3\x82\x93"));
    }
    m_pendingstr.clear();
    m_key2kana->clear();
}


void
SKKCore::clear_preedit (void)
{
    m_preeditstr.clear();
    m_preedit_pos = 0;
    m_okuristr.clear();
    m_okurihead = 0;
}

void
SKKCore::clear_commit (void)
{
    m_commit_flag = false;
    m_commit_pos = 0;
    m_commitstr.clear();
}

void
SKKCore::clear_candidate (void)
{
    m_candlist.clear();
    m_ltable->clear();
    m_show_ltable = false;
}

void
SKKCore::clear (void)
{
    clear_pending();
    clear_preedit();
    m_commit_flag = false;
}

bool
SKKCore::action_kakutei (void)
{
    switch (m_input_mode) {
    case INPUT_MODE_DIRECT:
        if (!(m_skk_mode == SKK_MODE_ASCII ||
              m_skk_mode == SKK_MODE_WIDE_ASCII) &&
            m_pendingstr.empty() && m_preeditstr.empty()) {
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
        if (m_cindex != m_candlist.end())
            commit_converting();
        else
            commit_converting(0);
        set_input_mode(INPUT_MODE_DIRECT);
        break;
    }
    if(m_skk_mode == SKK_MODE_ASCII || m_skk_mode == SKK_MODE_WIDE_ASCII) {
        set_skk_mode(SKK_MODE_HIRAGANA);
    }
    return true;
}

bool
SKKCore::action_cancel (void)
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
        if (m_skk_mode == SKK_MODE_ASCII) {
            set_skk_mode(SKK_MODE_HIRAGANA);
        }
        break;
    case INPUT_MODE_CONVERTING:
        if (!m_okuristr.empty()) {
            m_preeditstr += m_okuristr;
            m_preedit_pos += m_okuristr.length();
            m_okuristr.clear();
            m_okurihead = 0;
        }
        set_input_mode(INPUT_MODE_PREEDIT);
        clear_candidate();
        break;
    }
    return retval;
}

bool
SKKCore::action_convert (void)
{
    bool retval;
    switch (m_input_mode) {
    case INPUT_MODE_CONVERTING:
        retval = action_nextpage();
        if (!retval) {
            clear_candidate();
            set_input_mode(INPUT_MODE_LEARNING);
            m_learning = new SKKCore(m_keybind, m_dict, m_key2kana, m_ltable);
        }
        return true;
    case INPUT_MODE_PREEDIT:
        clear_pending();
        m_dict->lookup(m_preeditstr, m_candlist, *m_ltable);
        if (m_candlist.empty() && m_ltable->number_of_candidates() == 0) {
            set_input_mode(INPUT_MODE_LEARNING);
            m_learning = new SKKCore(m_keybind, m_dict, m_key2kana, m_ltable);
        } else {
            set_input_mode(INPUT_MODE_CONVERTING);
            if (m_candlist.empty())
                m_show_ltable = true;
            else
                m_cindex = m_candlist.begin();
        }
        return true;
    }

    return false;
}

bool
SKKCore::action_katakana (bool half)
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
        if (m_cindex != m_candlist.end())
            commit_converting();
        else
            commit_converting(0);
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
SKKCore::action_start_preedit (void)
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
        if (m_cindex != m_candlist.end())
            commit_converting();
        else
            commit_converting(0);
        set_input_mode(INPUT_MODE_PREEDIT);
        return true;
    default:
        return false;
    }
}

bool
SKKCore::action_prevcand (void)
{
    if (m_input_mode == INPUT_MODE_CONVERTING) {
        bool retval = action_prevpage();
        if (!retval)
            action_cancel();
        return true;
    } else {
        return false;
    }
}

bool
SKKCore::action_ascii (bool wide)
{
    switch (m_input_mode) {
    case INPUT_MODE_PREEDIT:
    case INPUT_MODE_OKURI:
        commit_string(m_preeditstr);
        clear_preedit();
        set_input_mode(INPUT_MODE_DIRECT);
        break;
    case INPUT_MODE_CONVERTING:
        if (m_cindex != m_candlist.end())
            commit_converting();
        else
            commit_converting(0);
        set_input_mode(INPUT_MODE_DIRECT);
    }
    clear_pending();
    if (wide) {
        set_skk_mode(SKK_MODE_WIDE_ASCII);
    } else {
        set_skk_mode(SKK_MODE_ASCII);
    }
    return true;
}

bool
SKKCore::action_ascii_convert (void)
{
    switch (m_input_mode) {
    case INPUT_MODE_CONVERTING:
        if (m_cindex != m_candlist.end())
            commit_converting();
        else
            commit_converting(0);
    case INPUT_MODE_DIRECT:
        set_skk_mode(SKK_MODE_ASCII);
        set_input_mode(INPUT_MODE_PREEDIT);
        clear_preedit();
        clear_pending();
        return true;
    default:
        return false;
    }
}

bool
SKKCore::action_backspace (void)
{
    if (m_pendingstr.empty()) {
        switch (m_input_mode) {
        case INPUT_MODE_CONVERTING:
            set_input_mode(INPUT_MODE_PREEDIT);
            clear_candidate();
            break;
        case INPUT_MODE_PREEDIT:
            if (m_preedit_pos == 0) {
                commit_string(m_preeditstr);
                action_cancel();
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
            m_key2kana->set_pending(m_pendingstr);
        }
    }

    return true;
}

bool
SKKCore::action_delete (void)
{
    if (m_pendingstr.empty()) {
        switch (m_input_mode) {
        case INPUT_MODE_CONVERTING:
            set_input_mode(INPUT_MODE_PREEDIT);
            clear_candidate();
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
SKKCore::action_forward (void)
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
SKKCore::action_backward (void)
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
SKKCore::action_nextpage (void)
{
    if (m_input_mode != INPUT_MODE_CONVERTING) return false;
    if (!m_candlist.empty() && m_cindex != m_candlist.end()) {
        m_cindex++;
        if (m_cindex == m_candlist.end()) {
            if (m_ltable->number_of_candidates() > 0)
                m_show_ltable = true;
            else
                return false;
        }
        return true;
    } else if (m_ltable->number_of_candidates() > 0) {
        return m_ltable->page_down();
    }
    return false;
}

bool
SKKCore::action_prevpage (void)
{
    if (m_input_mode != INPUT_MODE_CONVERTING) return false;
    if (!m_candlist.empty() && m_cindex != m_candlist.end()) {
        if (m_cindex != m_candlist.begin()) {
            m_cindex--;
            return true;
        } else
            return false;
    } else {
        bool retval = m_ltable->page_up();
        if (!retval) {
            if (m_candlist.empty()) {
                return false;
            } else {
                m_cindex--;
                m_show_ltable = false;
                return true;
            }
        } else {
            return retval;
        }
    }
    return false;
}

void
SKKCore::action_select_index (int i)
{
    commit_converting(i);
    set_input_mode(INPUT_MODE_DIRECT);
}

bool
SKKCore::process_remaining_keybinds (const KeyEvent &key)
{
    if (m_keybind->match_katakana_keys(key))
        return action_katakana(false);

    if (m_keybind->match_half_katakana_keys(key))
        return action_katakana(true);

    if (m_keybind->match_start_preedit_keys(key))
        return action_start_preedit();

    if (m_keybind->match_prevcand_keys(key))
        return action_prevcand();

    if(m_keybind->match_ascii_keys(key))
        return action_ascii(false);

    if(m_keybind->match_wide_ascii_keys(key))
        return action_ascii(true);

    if (m_keybind->match_ascii_convert_keys(key))
        return action_ascii_convert();

    if (m_keybind->match_backspace_keys(key))
        return action_backspace();

    if (m_keybind->match_delete_keys(key))
        return action_delete();

    if (m_keybind->match_forward_keys(key))
        return action_forward();

    if (m_keybind->match_backward_keys(key))
        return action_backward();

    return false;
}

bool
SKKCore::process_ascii (const KeyEvent &key)
{
    if (m_keybind->match_kakutei_keys(key))
        return action_kakutei();

    if (m_keybind->match_cancel_keys(key))
        return action_cancel();

    if (m_input_mode == INPUT_MODE_PREEDIT &&
        m_keybind->match_convert_keys(key))
        return action_convert();

    char code = key.get_ascii_code();

    if (!(key.mask & SCIM_KEY_ControlMask || key.mask & SCIM_KEY_Mod1Mask ||
          key.mask & SCIM_KEY_Mod2Mask    || key.mask & SCIM_KEY_Mod3Mask ||
          key.mask & SCIM_KEY_Mod4Mask    || key.mask & SCIM_KEY_Mod5Mask )) {
        if (m_input_mode == INPUT_MODE_DIRECT) {
            return false;
        } else {
            if (isprint(code)) {
                char str[2] = {code, '\0'};
                commit_or_preedit(utf8_mbstowcs(str));
                return true;
            } else {
                return process_remaining_keybinds(key);
            }
        }
    }

    return process_remaining_keybinds(key);
}

bool
SKKCore::process_wide_ascii (const KeyEvent &key)
{
    if (m_keybind->match_kakutei_keys(key))
        return action_kakutei();

    if (m_keybind->match_cancel_keys(key))
        return action_cancel();

    char code = key.get_ascii_code();

    if (!(key.mask & SCIM_KEY_ControlMask || key.mask & SCIM_KEY_Mod1Mask ||
          key.mask & SCIM_KEY_Mod2Mask    || key.mask & SCIM_KEY_Mod3Mask ||
          key.mask & SCIM_KEY_Mod4Mask    || key.mask & SCIM_KEY_Mod5Mask ) &&
        isprint(code)) {
        WideString result;

        convert_char_to_wide(code, result);
        commit_string(result);
        return true;
    }

    return process_remaining_keybinds(key);
}

bool
SKKCore::process_romakana (const KeyEvent &key)
{
    if (m_keybind->match_kakutei_keys(key))
        return action_kakutei();
    if (m_keybind->match_cancel_keys(key))
        return action_cancel();

    if (m_input_mode == INPUT_MODE_PREEDIT ||
        m_input_mode == INPUT_MODE_OKURI)
        if (m_keybind->match_convert_keys(key))
            return action_convert();

    char code = key.get_ascii_code();

    if (!(key.mask & SCIM_KEY_ControlMask || key.mask & SCIM_KEY_Mod1Mask ||
          key.mask & SCIM_KEY_Mod2Mask    || key.mask & SCIM_KEY_Mod3Mask ||
          key.mask & SCIM_KEY_Mod4Mask    || key.mask & SCIM_KEY_Mod5Mask ) &&
        isprint(code)) {
        if (isalpha(code)) {
            bool f = false;
            char str[2];
            WideString result;
            str[0] = tolower(code);
            str[1] = '\0';

            if (key.mask & SCIM_KEY_ShiftMask &&
                ((m_input_mode == INPUT_MODE_PREEDIT &&
                  !m_preeditstr.empty()) ||
                 m_input_mode == INPUT_MODE_DIRECT))
                f = true;

            m_key2kana->append(String(str), result, m_pendingstr);

            if (m_input_mode == INPUT_MODE_OKURI && !m_pendingstr.empty() &&
                result.empty()) {
                m_okurihead = m_pendingstr[0];
            }

            if (f) {
                if (m_input_mode == INPUT_MODE_PREEDIT) {
                    utf8_mbtowc(&m_okurihead, (unsigned char*)str, 1);
                    m_preeditstr.erase(m_preedit_pos);
                    if (m_pendingstr.empty()) {
                        set_input_mode(INPUT_MODE_OKURI);
                        commit_or_preedit(result);
                    } else{
                        commit_or_preedit(result);
                        set_input_mode(INPUT_MODE_OKURI);
                    }
                    return true;
                } else {
                    if (m_pendingstr.empty()) {
                        set_input_mode(INPUT_MODE_PREEDIT);
                        commit_or_preedit(result);
                    } else {
                        commit_or_preedit(result);
                        set_input_mode(INPUT_MODE_PREEDIT);
                    }
                }
            } else if (result.length() > 0) {
                commit_or_preedit(result);
            }

            if (!m_pendingstr.empty()) {
                if (process_remaining_keybinds(key)) {
                    clear_pending();
                }
            }
            return true;
        } else {
            char str[2];
            WideString result;
            str[0] = code;
            str[1] = '\0';

            if (m_pendingstr == utf8_mbstowcs("z")) {
                m_key2kana->append(String(str), result, m_pendingstr);
                if (result.length() > 0) {
                    commit_or_preedit(result);
                    return true;
                }
            }

            if (process_remaining_keybinds(key)) {
                return true;
            }

            clear_pending();
            m_key2kana->append(String(str), result, m_pendingstr);
            if (result.length() > 0) {
                commit_or_preedit(result);
            } else {
                commit_or_preedit(utf8_mbstowcs(str));
                clear_pending();
            }
            return true;
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
        int index;
        if (m_keybind->match_kakutei_keys(key))
            return action_kakutei();
        if (m_keybind->match_cancel_keys(key))
            return action_cancel();
        if (m_keybind->match_convert_keys(key))
            return action_convert();
        if (m_keybind->match_prevcand_keys(key))
            return action_prevcand();
        if (m_candlist.empty() || m_cindex == m_candlist.end())
            if ((index = m_keybind->match_selection_keys(key)) > -1) {
                action_select_index(index);
                return true;
            }

        commit_converting();
        set_input_mode(INPUT_MODE_DIRECT);
    }

    if (m_input_mode == INPUT_MODE_LEARNING) {
        bool retval = m_learning->process_key_event(key);
        char code = key.get_ascii_code();
        if (key.code == SCIM_KEY_Return || m_learning->m_end_flag) {
            if (m_learning->m_commitstr.empty()) {
                /* learning is canceled */
                delete m_learning;
                m_learning = 0;
                clear_candidate();
                if (m_okurihead == 0)
                    m_dict->lookup(m_preeditstr, m_candlist, *m_ltable);
                else
                    m_dict->lookup(m_preeditstr + m_okurihead,
                                   m_candlist, *m_ltable);
                if (m_candlist.empty() &&
                    m_ltable->number_of_candidates() == 0) {
                    set_input_mode(INPUT_MODE_PREEDIT);
                    clear_candidate();
                    if (!m_okuristr.empty()) {
                        m_preeditstr += m_okuristr;
                        m_preedit_pos += m_okuristr.length();
                        m_okuristr.clear();
                        m_okurihead = 0;
                    }
                } else {
                    set_input_mode(INPUT_MODE_CONVERTING);
                    if (m_ltable->number_of_candidates() == 0) {
                        m_cindex = m_candlist.end();
                        m_cindex--;
                        m_show_ltable = false;
                    } else {
                        m_cindex = m_candlist.end();
                        while(m_ltable->page_down());
                        m_show_ltable = true;
                    }
                }
                retval = true;
            } else {
                /* learning is committed */
                commit_string(m_learning->m_commitstr);
                commit_string(m_okuristr);
                m_dict->write(m_preeditstr, m_learning->m_commitstr);
                clear_preedit();
                clear_candidate();
                delete m_learning;
                m_learning = NULL;
                set_input_mode(INPUT_MODE_DIRECT);
            }
        } else if (retval == false &&
                   m_learning->m_skk_mode == SKK_MODE_ASCII &&
                   m_learning->m_input_mode == INPUT_MODE_DIRECT) {
            retval = true;
            if (isprint(code)) {
                char str[2] = { code, '\0' };
                m_learning->commit_string(utf8_mbstowcs(str));
            }
        } 
        return retval;
    }

    switch (m_skk_mode) {
    case SKK_MODE_ASCII:
        return process_ascii(key);
    case SKK_MODE_WIDE_ASCII:
        return process_wide_ascii(key);
    default:
        return process_romakana(key);
    }
}

bool
SKKCore::show_lookup_table (void)
{
    if (m_learning)
        return m_learning->show_lookup_table();
    else
        return m_show_ltable;
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
