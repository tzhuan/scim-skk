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

#define Uses_SCIM_UTILITY
#define Uses_SCIM_LOOKUP_TABLE

#include <scim.h>

#include "scim_skk_core.h"
#include "conv_table.h"

using namespace scim_skk;

static void convert_char_to_wide         (const char c, WideString &result);
static void convert_hiragana_to_katakana (const WideString &hira,
                                          WideString &kata,
                                          bool half = false);

static int skk_key_mask = SCIM_KEY_ControlMask | SCIM_KEY_AltMask;



SKKCore::SKKCore      (KeyBind *keybind, SKKAutomaton *key2kana,
                       SKKDictionary *dict, History &hist)
    : m_keybind(keybind),
      m_hist(hist),
      m_histmgr(hist),
      m_dict(dict),
      m_skk_mode(SKK_MODE_HIRAGANA),
      m_input_mode(INPUT_MODE_DIRECT),
      m_key2kana(key2kana),
      m_learning(0),
      m_commit_flag(false),
      m_end_flag(false),
      m_preedit_pos(0),
      m_commit_pos(0),
      m_ltable()
{
    std::vector<WideString> result;
    m_keybind->selection_labels(result);
    m_ltable.set_page_size(m_keybind->selection_key_length());
    m_ltable.set_candidate_labels(result);
    m_ltable.show_cursor(true);
    clear_preedit();
    clear_commit();
    clear_pending(false);
}

SKKCore::~SKKCore     (void)
{
    clear();
    if (m_learning)
        delete m_learning;
}

WideString &
SKKCore::get_commit_string (void)
{
    return m_commitstr;
}

void
SKKCore::get_preedit_attributes (AttributeList &alist)
{
    alist.clear();
    switch (m_input_mode) {
    case INPUT_MODE_CONVERTING:
        if (m_ltable.visible_table()) {
            int cpos = m_ltable.get_cursor_pos();
            int len1 = m_ltable.get_cand(cpos).length();
            int len2 = m_ltable.get_annot(cpos).length();
            alist.push_back(Attribute(1, len1, SCIM_ATTR_DECORATE,
                                      SCIM_ATTR_DECORATE_HIGHLIGHT));
            if (annot_highlight && len2 > 0)
                alist.push_back(Attribute(len1+m_okuristr.length()+2, len2,
                                          SCIM_ATTR_BACKGROUND,
                                          annot_bgcolor));
        } else {
            int len1 = m_ltable.get_cand_from_vector().length();
            int len2 = m_ltable.get_annot_from_vector().length();
            alist.push_back(Attribute(1, len1, SCIM_ATTR_DECORATE,
                                      SCIM_ATTR_DECORATE_HIGHLIGHT));
            if (annot_highlight && len2 > 0)
                alist.push_back(Attribute(len1+m_okuristr.length()+2, len2,
                                          SCIM_ATTR_BACKGROUND,
                                          annot_bgcolor)); 
        }
        break;
    default:
        break;
    }
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
        if (m_ltable.visible_table()) {
            int cpos = m_ltable.get_cursor_pos();
            result += m_ltable.get_cand(cpos);
        } else {
            result += m_ltable.get_cand_from_vector();
        }
        if (!m_okuristr.empty()) {
            result += m_okuristr;
        }
        if (annot_view && annot_pos &&
            !m_ltable.visible_table()) {
            WideString annot = m_ltable.get_annot_from_vector();
            if (annot.length() > 0) {
                result += utf8_mbstowcs(";");
                result += m_ltable.get_annot_from_vector();
            }
        }
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
SKKCore::commit_string (const WideString &str)
{
    m_commitstr.insert(m_commit_pos, str);
    m_commit_pos += str.length();
    m_commit_flag = true;
}

void
SKKCore::commit_or_preedit (const WideString &str)
{
    switch (m_input_mode) {
    case INPUT_MODE_PREEDIT:
        if (m_preedit_pos < m_preeditstr.length()) {
            m_preeditstr.insert(m_preedit_pos, str);
        } else {
            m_preeditstr += str;
        }
        m_preedit_pos += str.length();
        m_histmgr.clear();
        break;
    case INPUT_MODE_OKURI:
        m_okuristr += str;
        if (m_pendingstr.empty()) {
            m_ltable.clear();
            m_dict->lookup(m_preeditstr+m_okurihead, true, m_ltable);
            if (m_ltable.empty()) {
                set_input_mode(INPUT_MODE_LEARNING);
                m_learning = new SKKCore(m_keybind, m_key2kana,
                                         m_dict, m_hist);
            } else {
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
SKKCore::commit_converting (int index)
{
    if (!m_ltable.vector_empty() && !m_ltable.visible_table()) {
        CandEnt cent = m_ltable.get_candent_from_vector();
        commit_string(cent.cand);
        commit_string(m_okuristr);
        if (m_okurihead != 0)
            m_preeditstr += m_okurihead;
        m_dict->write(m_preeditstr, cent);
        m_ltable.clear();
        clear_preedit();
        if (m_skk_mode == SKK_MODE_ASCII)
            set_skk_mode(SKK_MODE_HIRAGANA);
    } else {
        if (index < 0) {
            index = m_ltable.get_cursor_pos();
        } else {
            index += m_ltable.get_current_page_start();
        }
        WideString cand  = m_ltable.get_cand(index);
        WideString annot = m_ltable.get_annot(index);
        WideString cand_orig = m_ltable.get_cand_orig(index);
        commit_string(cand);
        commit_string(m_okuristr);
        if (m_okurihead != 0)
            m_preeditstr += m_okurihead;
        m_dict->write(m_preeditstr, cand);
        m_ltable.clear();
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
        if (m_ltable.visible_table()) {
            int cpos = m_ltable.get_cursor_pos();
            base_pos += m_ltable.get_candidate(cpos).length() + 1;
        } else {
            base_pos += m_ltable.get_candidate_from_vector().length() + 1;
        }
        if (!m_okuristr.empty())
            base_pos += m_okuristr.length() + 1;
        return base_pos;
    case INPUT_MODE_LEARNING:
        if (!m_okuristr.empty())
            base_pos += m_okuristr.length() + 1;
        return base_pos + m_preeditstr.length() + 2 + m_learning->caret_pos();
    }

    return base_pos;
}

void
SKKCore::move_preedit_caret (int pos)
{
    if (pos < 0)
        return;

    switch (m_input_mode) {
    case INPUT_MODE_DIRECT:
        if (pos <= m_commitstr.length()) {
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
                   m_ltable.get_candidate_from_vector().length() +
                   m_okuristr.length() + 1 &&
                   pos <= m_commitstr.length() +
                   m_ltable.get_candidate_from_vector().length() +
                   m_okuristr.length() + 1) {
            m_commit_pos = pos - m_ltable.get_candidate_from_vector().length() - m_okuristr.length() - 1;
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
        return m_learning->get_input_mode();
    } else {
        return m_input_mode;
    }
}


void
SKKCore::clear_pending (bool flag)
{
    if (flag && m_pendingstr == utf8_mbstowcs("n")) {
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
    m_histmgr.clear();
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
    m_ltable.clear();
    m_commit_flag = false;
    if (m_learning)
        m_learning->clear();
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
            return false;
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
            if (m_input_mode == INPUT_MODE_PREEDIT) {
                m_hist.add_entry(m_preeditstr);
            }
            clear_preedit();
        }
        clear_pending();
        break;
    case INPUT_MODE_CONVERTING:
        commit_converting();
        set_input_mode(INPUT_MODE_DIRECT);
        break;
    default:
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
    switch (m_input_mode) {
    case INPUT_MODE_DIRECT:
        if (!m_pendingstr.empty()) {
            clear_pending(false);
        } else {
            clear_commit();
            m_end_flag = true;
            return false;
        }
        break;
    case INPUT_MODE_PREEDIT:
    case INPUT_MODE_OKURI:
        clear_preedit();
        clear_pending(false);
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
        m_ltable.clear();
        set_input_mode(INPUT_MODE_PREEDIT);
        break;
    default:
        break;
    }
    return true;
}

bool
SKKCore::action_convert (void)
{
    bool retval;
    switch (m_input_mode) {
    case INPUT_MODE_CONVERTING:
        retval = action_nextpage();
        if (!retval) {
            set_input_mode(INPUT_MODE_LEARNING);
            m_learning = new SKKCore(m_keybind, m_key2kana, m_dict, m_hist);
        }
        return true;
    case INPUT_MODE_PREEDIT:
        clear_pending();
        m_hist.add_entry(m_preeditstr);
        m_dict->lookup(m_preeditstr, false, m_ltable);
        if (m_ltable.empty()) {
            set_input_mode(INPUT_MODE_LEARNING);
            m_learning = new SKKCore(m_keybind, m_key2kana, m_dict, m_hist);
        } else {
            set_input_mode(INPUT_MODE_CONVERTING);
        }
        return true;
    default:
        break;
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
            if (!m_preeditstr.empty() && m_input_mode == INPUT_MODE_PREEDIT) {
                m_hist.add_entry(m_preeditstr);
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
    default:
        break;
    }

    return false;
}

bool
SKKCore::action_toggle_case (void)
{
    if (m_input_mode == INPUT_MODE_PREEDIT && m_skk_mode == SKK_MODE_ASCII) {
        for (WideString::iterator i = m_preeditstr.begin();
             i != m_preeditstr.end(); i++) {
            int code = *i;
            if (islower(code)) {
                *i = toupper(code);
            } else if (isupper(code)) {
                *i = tolower(code);
            }
        }
        if (!m_preeditstr.empty()) {
            m_hist.add_entry(m_preeditstr);
        }
        commit_string(m_preeditstr);
        clear_preedit();
        clear_pending();
        set_input_mode(INPUT_MODE_DIRECT);
        set_skk_mode(SKK_MODE_HIRAGANA);
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
        commit_converting();
        set_input_mode(INPUT_MODE_PREEDIT);
        return true;
    default:
        return false;
    }
}

bool
SKKCore::action_completion (void)
{
    if (m_input_mode == INPUT_MODE_PREEDIT) {
        if (m_histmgr.is_clear()) {
            m_histmgr.setup_completion(m_preeditstr);
        } else {
            m_histmgr.next_cand();
        }
        m_histmgr.get_current_candidate(m_preeditstr);
        m_preedit_pos = m_preeditstr.size();
        return true;
    } else {
        return false;
    }
}

bool
SKKCore::action_completion_back (void)
{
    if (m_input_mode == INPUT_MODE_PREEDIT) {
        if (m_histmgr.prev_cand()) {
            m_histmgr.get_current_candidate(m_preeditstr);
            m_preedit_pos = m_preeditstr.size();
            return true;
        }
    } 
    return false;
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
        commit_converting();
        set_input_mode(INPUT_MODE_DIRECT);
    default:
        break;
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
        commit_converting();
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
            m_ltable.clear();
            break;
        case INPUT_MODE_PREEDIT:
            if (m_preedit_pos == 0) {
                commit_string(m_preeditstr);
                action_cancel();
            } else {
                m_preeditstr.erase(m_preedit_pos-1, 1);
                m_histmgr.clear();
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
        default:
            break;
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
            m_ltable.clear();
            break;
        case INPUT_MODE_PREEDIT:
            if (m_preedit_pos < m_preeditstr.length()) {
                m_preeditstr.erase(m_preedit_pos, 1);
                m_histmgr.clear();
            }
            break;
        case INPUT_MODE_DIRECT:
            if (m_commitstr.empty()) {
                clear_commit();
                m_end_flag = true;
                return false;
            } else if (m_commit_pos < m_commitstr.length()) {
                m_commitstr.erase(m_commit_pos, 1);
            }
        default:
            break;
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
        if (m_ltable.visible_table()) {
            if (!m_ltable.cursor_down()) {
                set_input_mode(INPUT_MODE_LEARNING);
                m_learning = new SKKCore(m_keybind, m_key2kana,
                                         m_dict, m_hist);
            }
            return true;
        } else {
            return action_convert();
        }
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
        m_histmgr.clear();
        if (m_preedit_pos < m_preeditstr.length()) {
            m_preedit_pos++;
        } else if (m_commit_pos < m_commitstr.length()) {
            m_commit_pos++;
        } else {
            return false;
        }
        return true;
    default:
        break;
    }
    return false;
}

bool
SKKCore::action_backward (void)
{
    switch (m_input_mode) {
    case INPUT_MODE_CONVERTING:
        if (m_ltable.visible_table()) {
            if (!m_ltable.cursor_up()) {
                return m_ltable.prev_candidate();
            } else {
                return true;
            }
        } else {
            return action_prevcand();
        }
    case INPUT_MODE_DIRECT:
        clear_pending();
        m_histmgr.clear();
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
    default:
        break;
    }
    return false;
}

bool
SKKCore::action_home (void)
{
    switch (m_input_mode) {
    case INPUT_MODE_CONVERTING:
        return false;
    case INPUT_MODE_DIRECT:
        clear_pending();
        if (m_commit_pos > 0) {
            m_commit_pos = 0;
            return true;
        } else {
            return false;
        }
    case INPUT_MODE_PREEDIT:
        clear_pending();
        m_histmgr.clear();
        if (m_preedit_pos > 0) {
            m_preedit_pos = 0;
        } else if (m_commit_pos > 0) {
            m_commit_pos = 0;
        } else {
            return false;
        }
        return true;
    default:
        return false;
    }
}

bool
SKKCore::action_end (void)
{
    switch (m_input_mode) {
    case INPUT_MODE_CONVERTING:
        return false;
    case INPUT_MODE_DIRECT:
        clear_pending();
        if (m_commit_pos < m_commitstr.length()) {
            m_commit_pos = m_commitstr.length();
            return true;
        } else {
            return false;
        }
    case INPUT_MODE_PREEDIT:
        clear_pending();
        m_histmgr.clear();
        if (m_preedit_pos < m_preeditstr.length()) {
            m_preedit_pos = m_preeditstr.length();
        } else if (m_commit_pos < m_commitstr.length()) {
            m_commit_pos = m_commitstr.length();
        } else {
            return false;
        }
        return true;
    default:
        return false;
    }
}

bool
SKKCore::action_nextpage (void)
{
    if (m_input_mode != INPUT_MODE_CONVERTING) return false;
    if (!m_ltable.visible_table()) {
        if (!m_ltable.next_candidate()) {
            return m_ltable.number_of_candidates() != 0;
        }
        return true;
    } else if (m_ltable.number_of_candidates() > 0) {
        bool retval = m_ltable.page_down();
        m_ltable.set_page_size(m_keybind->selection_key_length());
        return retval;
    }
    return false;
}

bool
SKKCore::action_prevpage (void)
{
    if (m_input_mode != INPUT_MODE_CONVERTING) return false;
    if (!m_ltable.visible_table()) {
        return m_ltable.prev_candidate();
    } else {
        bool retval = m_ltable.page_up();
        m_ltable.set_page_size(m_keybind->selection_key_length());
        if (!retval)
            return m_ltable.prev_candidate();
        else
            return true;
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

    if (m_keybind->match_home_keys(key))
        return action_home();

    if (m_keybind->match_end_keys(key))
        return action_end();

    if (m_keybind->match_completion_keys(key))
        return action_completion();

    if (m_keybind->match_completion_back_keys(key))
        return action_completion_back();

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

    if (m_input_mode == INPUT_MODE_PREEDIT &&
        m_keybind->match_upcase_keys(key))
        return action_toggle_case();

    char code = key.get_ascii_code();

    if (!(key.mask & skk_key_mask)) {
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

    if (!(key.mask & skk_key_mask) && isprint(code)) {
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

    if (m_pendingstr.empty() && process_remaining_keybinds(key)) {
        return true;
    }

    unsigned char code = key.get_ascii_code();

    if (!(key.mask & skk_key_mask) && isprint(code)) {
        bool d2p = false;  /* direct to preedit flag */
        bool p2o = false;  /* preedit to okuri flag */
        bool is_pending_clear;
        bool retval = false;
        WideString result;

        if (isalpha(code) && key.is_shift_down()) {
            if (m_input_mode == INPUT_MODE_PREEDIT &&
                !m_preeditstr.empty()) {
                p2o = true;
            } else if (m_input_mode == INPUT_MODE_DIRECT) {
                d2p = true;
            }
        }

        is_pending_clear =
            m_key2kana->append(String(1, tolower(code)), result, m_pendingstr);

        if (m_input_mode == INPUT_MODE_OKURI && !m_pendingstr.empty() &&
            result.empty()) {
            m_okurihead = m_pendingstr[0];
        }

        if (d2p) {
            /* shift to preedit from direct */
            if (m_pendingstr.empty()) {
                set_input_mode(INPUT_MODE_PREEDIT);
                commit_or_preedit(result);
            } else {
                commit_or_preedit(result);
                set_input_mode(INPUT_MODE_PREEDIT);
            }
            retval = true;
        } else if (p2o) {
            /* shift to okuri from preedit */
            m_okurihead = (ucs4_t) tolower(code);
            m_preeditstr.erase(m_preedit_pos);
            if (m_pendingstr.empty()) {
                set_input_mode(INPUT_MODE_OKURI);
                commit_or_preedit(result);
            } else{
                commit_or_preedit(result);
                set_input_mode(INPUT_MODE_OKURI);
            }
            retval = true;
        } else if (result.length() > 0) {
            /* otherwise simply commit or add to preedit */
            commit_or_preedit(result);
            retval = true;
        } else if (!m_pendingstr.empty()) {
            retval = true ;
        }

        if ((is_pending_clear) && process_remaining_keybinds(key)) {
            clear_pending();
            retval = true;
        }

        return retval;
    } else {
        return process_remaining_keybinds(key);
    }

    return false;
}

bool
SKKCore::process_key_event (const KeyEvent key)
{
    if (m_input_mode == INPUT_MODE_CONVERTING) {
        if (m_keybind->match_kakutei_keys(key))
            return action_kakutei();
        if (m_keybind->match_cancel_keys(key))
            return action_cancel();
        if (m_keybind->match_convert_keys(key))
            return action_convert();
        if (m_keybind->match_prevcand_keys(key))
            return action_prevcand();
        if (m_keybind->match_forward_keys(key))
            return action_forward();
        if (m_keybind->match_backward_keys(key))
            return action_backward();
        if (m_ltable.visible_table() && m_ltable.number_of_candidates() > 0) {
            int index;
            if ((index = m_keybind->match_selection_keys(key)) > -1) {
                action_select_index(index);
                return true;
            }
        }
        commit_converting();
        set_input_mode(INPUT_MODE_DIRECT);
        if (key.code == SCIM_KEY_Return &&
            (ignore_return ^ (key.mask & SCIM_KEY_ShiftMask != 0))) {
            return true;
        }
    }

    if (m_input_mode == INPUT_MODE_LEARNING) {
        bool retval = m_learning->process_key_event(key);
        char code = key.get_ascii_code();
        if (key.code == SCIM_KEY_Return || m_learning->m_end_flag) {
            if (key.code == SCIM_KEY_Return &&
                (ignore_return ^ (key.mask & SCIM_KEY_ShiftMask != 0)))
                retval = true;
            if (m_learning->m_commitstr.empty()) {
                /* learning is canceled */
                delete m_learning;
                m_learning = 0;
                if (m_ltable.empty()) {
                    set_input_mode(INPUT_MODE_PREEDIT);
                    m_ltable.clear();
                    if (!m_okuristr.empty()) {
                        m_preeditstr += m_okuristr;
                        m_preedit_pos += m_okuristr.length();
                        m_okuristr.clear();
                        m_okurihead = 0;
                    }
                } else {
                    if (m_ltable.number_of_candidates() == 0)
                        m_ltable.prev_candidate();
                    set_input_mode(INPUT_MODE_CONVERTING);
                }
                retval = true;
            } else {
                /* learning is committed */
                static const ucs4_t sharp = 0x23; /* code for number sign */
                if (m_learning->m_commitstr.find(sharp) != WideString::npos) {
                    WideString result, keystr;
                    std::list<WideString> numbers;
                    m_dict->extract_numbers(m_preeditstr,
                                            numbers, keystr);
                    m_dict->number_conversion(numbers,
                                              m_learning->m_commitstr,
                                              result);
                    m_preeditstr.assign(keystr);
                    commit_string(result);
                } else {
                    commit_string(m_learning->m_commitstr);
                }
                commit_string(m_okuristr);
                if (m_okurihead != 0)
                    m_preeditstr += m_okurihead;
                m_dict->write(m_preeditstr,
                              CandEnt(m_learning->m_commitstr));
                clear_preedit();
                m_ltable.clear();
                m_learning->clear();
                delete m_learning;
                m_learning = 0;
                set_input_mode(INPUT_MODE_DIRECT);
            }
        } else if (retval == false &&
                   m_learning->get_skk_mode() == SKK_MODE_ASCII &&
                   m_learning->get_input_mode() == INPUT_MODE_DIRECT) {
            retval = true;
            if (isprint(code)) {
                char str[2] = { code, '\0' };
                m_learning->commit_string(utf8_mbstowcs(str));
            }
        } 
        return retval;
    }

    if (m_input_mode == INPUT_MODE_PREEDIT && key.code == SCIM_KEY_Return &&
        !(key.mask & skk_key_mask)) {
        action_kakutei();
        if (ignore_return ^ (key.mask & SCIM_KEY_ShiftMask != 0)) {
            return true;
        } else {
            return false;
        }
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

SKKCandList&
SKKCore::get_lookup_table (void)
{
    if (m_learning)
        return m_learning->get_lookup_table();
    else
        return m_ltable;
}

bool
SKKCore::lookup_table_visible (void)
{
    if (m_learning)
        return m_learning->lookup_table_visible();
    else
        return m_ltable.visible_table();
}

static void
convert_char_to_wide (const char c, WideString & wide)
{
    bool found = false;

    for (unsigned int i = 0; wide_table[i].code; i++) {
        if (wide_table[i].code[0] == c) {
            wide += utf8_mbstowcs (wide_table[i].wide);
            found = true;
            break;
        }
    }

#if 0
    if (!found && space && c == ' ') {
        wide += utf8_mbstowcs ("\xE3\x80\x80");
        found = true;
    }
#endif

    if (!found)
        wide += utf8_mbstowcs (&c, 1);
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

        for (unsigned int j = 0; hiragana_katakana_table[j].hiragana; j++) {
            tmpwide = utf8_mbstowcs (hiragana_katakana_table[j].hiragana);
            if (hira.substr(i, 1) == tmpwide) {
                if (half)
                    kata += utf8_mbstowcs (hiragana_katakana_table[j].half_katakana);
                else
                    kata += utf8_mbstowcs (hiragana_katakana_table[j].katakana);
                found = true;
                break;
            }
        }

        if (!found)
            kata += hira.substr(i, 1);
    }
}
