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

#define Uses_SCIM_EVENT
#include "scim_skk_keybind.h"

char *qwerty_vec[7]  = {"a", "s", "d", "f", "j", "k", "l"};
char *dvorak_vec[8]  = {"a", "o", "e", "u", "h", "t", "n", "s"};
char *number_vec[9]  = {"1", "2", "3", "4", "5", "6", "7", "8", "9"};

static inline bool match_key_event       (const KeyEventList &keylist,
                                          const KeyEvent &key);

KeyBind::KeyBind (void)
    : m_style (SSTYLE_QWERTY)
{
}

KeyBind::~KeyBind (void)
{
}

void
KeyBind::set_selection_style (const String &str)
{
    if (str == "Qwerty") {
        m_style = SSTYLE_QWERTY;
    } else if (str == "Dvorak") {
        m_style = SSTYLE_DVORAK;
    } else if (str == "Number") {
        m_style = SSTYLE_NUMBER;
    }
}

int
KeyBind::match_selection_dvorak (const KeyEvent &key)
{
    switch (key.code) {
    case SCIM_KEY_a:
    case SCIM_KEY_A:
        return 0;
    case SCIM_KEY_o:
    case SCIM_KEY_O:
        return 1;
    case SCIM_KEY_e:
    case SCIM_KEY_E:
        return 2;
    case SCIM_KEY_u:
    case SCIM_KEY_U:
        return 3;
    case SCIM_KEY_h:
    case SCIM_KEY_H:
        return 4;
    case SCIM_KEY_t:
    case SCIM_KEY_T:
        return 5;
    case SCIM_KEY_n:
    case SCIM_KEY_N:
        return 6;
    case SCIM_KEY_s:
    case SCIM_KEY_S:
        return 7;
    default:
        return -1;
    }
}

int
KeyBind::match_selection_qwerty (const KeyEvent &key)
{
    switch (key.code) {
    case SCIM_KEY_a:
    case SCIM_KEY_A:
        return 0;
    case SCIM_KEY_s:
    case SCIM_KEY_S:
        return 1;
    case SCIM_KEY_d:
    case SCIM_KEY_D:
        return 2;
    case SCIM_KEY_f:
    case SCIM_KEY_F:
        return 3;
    case SCIM_KEY_j:
    case SCIM_KEY_J:
        return 4;
    case SCIM_KEY_k:
    case SCIM_KEY_K:
        return 5;
    case SCIM_KEY_l:
    case SCIM_KEY_L:
        return 6;
    default:
        return -1;
    }
}

int
KeyBind::match_selection_number (const KeyEvent &key)
{
    char c;
    if (isdigit(c = key.get_ascii_code()) && c != '0') {
        return (c - '1');
    } else {
        return -1;
    }
}

int
KeyBind::match_selection_keys (const KeyEvent &key)
{
    if (key.mask & SCIM_KEY_ControlMask || key.mask & SCIM_KEY_Mod1Mask ||
        key.mask & SCIM_KEY_Mod2Mask    || key.mask & SCIM_KEY_Mod3Mask ||
        key.mask & SCIM_KEY_Mod4Mask    || key.mask & SCIM_KEY_Mod5Mask )
        return 0;
    if (!isprint(key.code))
        return 0;

    switch (m_style) {
    case SSTYLE_QWERTY:
        return match_selection_qwerty(key);
    case SSTYLE_DVORAK:
        return match_selection_dvorak(key);
    case SSTYLE_NUMBER:
        return match_selection_number(key);
    }
}

int
KeyBind::selection_key_length (void)
{
    switch (m_style) {
    case SSTYLE_QWERTY:
        return 7;
    case SSTYLE_DVORAK:
        return 8;
    case SSTYLE_NUMBER:
        return 9;
    }
}

void
KeyBind::selection_labels (std::vector<WideString> &result)
{
    switch (m_style) {
    case SSTYLE_QWERTY:
        result.resize(7);
        for (int i = 0; i < 7; i++)
            result[i] = utf8_mbstowcs(qwerty_vec[i]);
        break;
    case SSTYLE_DVORAK:
        result.resize(8);
        for (int i = 0; i < 8; i++)
            result[i] = utf8_mbstowcs(dvorak_vec[i]);
        break;
    case SSTYLE_NUMBER:
        result.resize(9);
        for (int i = 0; i < 9; i++)
            result[i] = utf8_mbstowcs(number_vec[i]);
        break;
    }
}

void
KeyBind::set_kakutei_keys       (const String &str)
{
    scim_string_to_key_list(m_kakutei_keys, str);
}
void
KeyBind::set_katakana_keys      (const String &str)
{
    scim_string_to_key_list(m_katakana_keys, str);
}
void
KeyBind::set_half_katakana_keys (const String &str)
{
    scim_string_to_key_list(m_half_katakana_keys, str);
}
void
KeyBind::set_ascii_keys         (const String &str)
{
    scim_string_to_key_list(m_ascii_keys, str);
}
void
KeyBind::set_wide_ascii_keys    (const String &str)
{
    scim_string_to_key_list(m_wide_ascii_keys, str);
}
void
KeyBind::set_convert_keys       (const String &str)
{
    scim_string_to_key_list(m_convert_keys, str);
}
void
KeyBind::set_start_preedit_keys    (const String &str)
{
    scim_string_to_key_list(m_start_preedit_keys, str);
}
void
KeyBind::set_cancel_keys        (const String &str)
{
    scim_string_to_key_list(m_cancel_keys, str);
}
void
KeyBind::set_ascii_convert_keys        (const String &str)
{
    scim_string_to_key_list(m_ascii_convert_keys, str);
}
void
KeyBind::set_prevcand_keys        (const String &str)
{
    scim_string_to_key_list(m_prevcand_keys, str);
}
void
KeyBind::set_backspace_keys        (const String &str)
{
    scim_string_to_key_list(m_backspace_keys, str);
}
void
KeyBind::set_delete_keys           (const String &str)
{
    scim_string_to_key_list(m_delete_keys, str);
}
void
KeyBind::set_forward_keys          (const String &str)
{
    scim_string_to_key_list(m_forward_keys, str);
}
void
KeyBind::set_backward_keys         (const String &str)
{
    scim_string_to_key_list(m_backward_keys, str);
}

bool
KeyBind::match_kakutei_keys       (const KeyEvent &key)
{
    return match_key_event(m_kakutei_keys, key);
}
bool
KeyBind::match_katakana_keys      (const KeyEvent &key)
{
    return match_key_event(m_katakana_keys, key);
}
bool
KeyBind::match_half_katakana_keys (const KeyEvent &key)
{
    return match_key_event(m_half_katakana_keys, key);
}
bool
KeyBind::match_ascii_keys         (const KeyEvent &key)
{
    return match_key_event(m_ascii_keys, key);
}
bool
KeyBind::match_wide_ascii_keys    (const KeyEvent &key)
{
    return match_key_event(m_wide_ascii_keys, key);
}
bool
KeyBind::match_convert_keys       (const KeyEvent &key)
{
    return match_key_event(m_convert_keys, key);
}
bool
KeyBind::match_start_preedit_keys    (const KeyEvent &key)
{
    return match_key_event(m_start_preedit_keys, key);
}
bool
KeyBind::match_cancel_keys        (const KeyEvent &key)
{
    return match_key_event(m_cancel_keys, key);
}
bool
KeyBind::match_ascii_convert_keys        (const KeyEvent &key)
{
    return match_key_event(m_ascii_convert_keys, key);
}
bool
KeyBind::match_prevcand_keys        (const KeyEvent &key)
{
    return match_key_event(m_prevcand_keys, key);
}
bool
KeyBind::match_backspace_keys        (const KeyEvent &key)
{
    return match_key_event(m_backspace_keys, key);
}
bool
KeyBind::match_delete_keys        (const KeyEvent &key)
{
    return match_key_event(m_delete_keys, key);
}
bool
KeyBind::match_forward_keys          (const KeyEvent &key)
{
    return match_key_event(m_forward_keys, key);
}
bool
KeyBind::match_backward_keys         (const KeyEvent &key)
{
    return match_key_event(m_backward_keys, key);
}


static inline bool
match_key_event (const KeyEventList &keylist, const KeyEvent &key)
{
    bool flag = false;
    if (key.mask & SCIM_KEY_ShiftMask &&
        key.get_ascii_code() &&
        !(key.mask & SCIM_KEY_ControlMask || key.mask & SCIM_KEY_Mod1Mask ||
          key.mask & SCIM_KEY_Mod2Mask    || key.mask & SCIM_KEY_Mod3Mask ||
          key.mask & SCIM_KEY_Mod4Mask    || key.mask & SCIM_KEY_Mod5Mask )
        ) {
        flag = true;
    }

    KeyEventList::const_iterator it;

    if (flag) {
        char code = tolower(key.get_ascii_code());
        for (it = keylist.begin(); it != keylist.end(); it++) {
            if (*it == key) break;

            if (it->is_shift_down()) {
                if (it->get_ascii_code() == code)
                    break;
            } else {
                if (it->code == key.code)
                    break;
            }
        }
    } else {
        it = std::find(keylist.begin(), keylist.end(), key);
    }

    return it != keylist.end();
}

