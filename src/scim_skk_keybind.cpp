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

static char *qwerty_vec[7]  = {"a", "s", "d", "f", "j", "k", "l"};
static char *dvorak_vec[8]  = {"a", "o", "e", "u", "h", "t", "n", "s"};
static char *number_vec[9]  = {"1", "2", "3", "4", "5", "6", "7", "8", "9"};

static int qwerty_vec_len = sizeof(qwerty_vec);
static int dvorak_vec_len = sizeof(dvorak_vec);
static int number_vec_len = sizeof(number_vec);

static void keybind_string_to_key_list(KeyEventList &keys, const String &str);
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

    // error
    return 0;
}

int
KeyBind::selection_key_length (void)
{
    switch (m_style) {
    case SSTYLE_QWERTY:
        return qwerty_vec_len;
    case SSTYLE_DVORAK:
        return dvorak_vec_len;
    case SSTYLE_NUMBER:
        return number_vec_len;
    }

    // error
    return 0;
}

void
KeyBind::selection_labels (std::vector<WideString> &result)
{
    switch (m_style) {
    case SSTYLE_QWERTY:
        result.resize(qwerty_vec_len);
        for (int i = 0; i < qwerty_vec_len; i++)
            result[i] = utf8_mbstowcs(qwerty_vec[i]);
        break;
    case SSTYLE_DVORAK:
        result.resize(dvorak_vec_len);
        for (int i = 0; i < dvorak_vec_len; i++)
            result[i] = utf8_mbstowcs(dvorak_vec[i]);
        break;
    case SSTYLE_NUMBER:
        result.resize(number_vec_len);
        for (int i = 0; i < number_vec_len; i++)
            result[i] = utf8_mbstowcs(number_vec[i]);
        break;
    }

    return;
}

void
KeyBind::set_kakutei_keys       (const String &str)
{
    keybind_string_to_key_list(m_kakutei_keys, str);
}
void
KeyBind::set_katakana_keys      (const String &str)
{
    keybind_string_to_key_list(m_katakana_keys, str);
}
void
KeyBind::set_half_katakana_keys (const String &str)
{
    keybind_string_to_key_list(m_half_katakana_keys, str);
}
void
KeyBind::set_ascii_keys         (const String &str)
{
    keybind_string_to_key_list(m_ascii_keys, str);
}
void
KeyBind::set_wide_ascii_keys    (const String &str)
{
    keybind_string_to_key_list(m_wide_ascii_keys, str);
}
void
KeyBind::set_convert_keys       (const String &str)
{
    keybind_string_to_key_list(m_convert_keys, str);
}
void
KeyBind::set_start_preedit_keys    (const String &str)
{
    keybind_string_to_key_list(m_start_preedit_keys, str);
}
void
KeyBind::set_cancel_keys        (const String &str)
{
    keybind_string_to_key_list(m_cancel_keys, str);
}
void
KeyBind::set_ascii_convert_keys        (const String &str)
{
    keybind_string_to_key_list(m_ascii_convert_keys, str);
}
void
KeyBind::set_prevcand_keys        (const String &str)
{
    keybind_string_to_key_list(m_prevcand_keys, str);
}
void
KeyBind::set_backspace_keys        (const String &str)
{
    keybind_string_to_key_list(m_backspace_keys, str);
}
void
KeyBind::set_delete_keys           (const String &str)
{
    keybind_string_to_key_list(m_delete_keys, str);
}
void
KeyBind::set_forward_keys          (const String &str)
{
    keybind_string_to_key_list(m_forward_keys, str);
}
void
KeyBind::set_backward_keys         (const String &str)
{
    keybind_string_to_key_list(m_backward_keys, str);
}
void
KeyBind::set_home_keys         (const String &str)
{
    keybind_string_to_key_list(m_home_keys, str);
}
void
KeyBind::set_end_keys         (const String &str)
{
    keybind_string_to_key_list(m_end_keys, str);
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
bool
KeyBind::match_home_keys         (const KeyEvent &key)
{
    return match_key_event(m_home_keys, key);
}
bool
KeyBind::match_end_keys         (const KeyEvent &key)
{
    return match_key_event(m_end_keys, key);
}

static void
keybind_string_to_key_list(KeyEventList &keys, const String &str)
{
    KeyEventList kl;
    KeyEventList::iterator it;
    scim_string_to_key_list(kl, str);
    for (it = kl.begin(); it != kl.end(); it++) {
        char code = it->get_ascii_code();
        if (islower(code) && it->is_shift_down()) {
            it->code = toupper(it->get_ascii_code());
        } else if (isupper(code) && !it->is_shift_down()) {
            it->mask |= SCIM_KEY_ShiftMask;
        }
        keys.push_back(*it);
    }
}

static inline bool
match_key_event (const KeyEventList &keylist, const KeyEvent &key)
{
    KeyEvent k(key.code, key.mask);
    char code = k.get_ascii_code();

    if (islower(code) && k.is_shift_down()) {
        k.code = toupper(k.get_ascii_code());
    } else if (isupper(code) && !k.is_shift_down()) {
        k.code = tolower(k.get_ascii_code());
    }

    KeyEventList::const_iterator it = std::find(keylist.begin(),
                                                keylist.end(), k);
    return it != keylist.end();
}

