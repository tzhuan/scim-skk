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


static inline bool match_key_event       (const KeyEventList &keylist,
        const KeyEvent &key);

KeyBind::KeyBind (void)
    : m_style (SSTYLE_QWERTY)
{
}

KeyBind::~KeyBind (void)
{
    m_kakutei_keys.clear();
    m_katakana_keys.clear();
    m_half_katakana_keys.clear();
    m_latin_keys.clear();
    m_wide_latin_keys.clear();
    m_convert_keys.clear();
    m_start_conv_keys.clear();
    m_cancel_keys.clear();
}

void
KeyBind::set_selection_style (String str)
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
        return 1;
    case SCIM_KEY_o:
    case SCIM_KEY_O:
        return 2;
    case SCIM_KEY_e:
    case SCIM_KEY_E:
        return 3;
    case SCIM_KEY_u:
    case SCIM_KEY_U:
        return 4;
    case SCIM_KEY_h:
    case SCIM_KEY_H:
        return 5;
    case SCIM_KEY_t:
    case SCIM_KEY_T:
        return 6;
    case SCIM_KEY_n:
    case SCIM_KEY_N:
        return 7;
    case SCIM_KEY_s:
    case SCIM_KEY_S:
        return 8;
    default:
        return 0;
    }
}

int
KeyBind::match_selection_qwerty (const KeyEvent &key)
{
    switch (key.code) {
    case SCIM_KEY_a:
    case SCIM_KEY_A:
        return 1;
    case SCIM_KEY_s:
    case SCIM_KEY_S:
        return 2;
    case SCIM_KEY_d:
    case SCIM_KEY_D:
        return 3;
    case SCIM_KEY_f:
    case SCIM_KEY_F:
        return 4;
    case SCIM_KEY_j:
    case SCIM_KEY_J:
        return 5;
    case SCIM_KEY_k:
    case SCIM_KEY_K:
        return 6;
    case SCIM_KEY_l:
    case SCIM_KEY_L:
        return 7;
    default:
        return 0;
    }
}

int
KeyBind::match_selection_number (const KeyEvent &key)
{
    if (isdigit(key.code)) {
        if (key.code == SCIM_KEY_0) {
            return 10;
        } else {
            return (key.code - '0');
        }
    } else {
        return 0;
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

void
KeyBind::set_kakutei_keys       (String str)
{
    scim_string_to_key_list(m_kakutei_keys, str);
}
void
KeyBind::set_katakana_keys      (String str)
{
    scim_string_to_key_list(m_katakana_keys, str);
}
void
KeyBind::set_half_katakana_keys (String str)
{
    scim_string_to_key_list(m_half_katakana_keys, str);
}
void
KeyBind::set_latin_keys         (String str)
{
    scim_string_to_key_list(m_latin_keys, str);
}
void
KeyBind::set_wide_latin_keys    (String str)
{
    scim_string_to_key_list(m_wide_latin_keys, str);
}
void
KeyBind::set_convert_keys       (String str)
{
    scim_string_to_key_list(m_convert_keys, str);
}
void
KeyBind::set_start_conv_keys    (String str)
{
    scim_string_to_key_list(m_start_conv_keys, str);
}
void
KeyBind::set_cancel_keys        (String str)
{
    scim_string_to_key_list(m_cancel_keys, str);
}
void
KeyBind::set_latin_convert_keys        (String str)
{
    scim_string_to_key_list(m_latin_convert_keys, str);
}
void
KeyBind::set_prevcand_keys        (String str)
{
    scim_string_to_key_list(m_prevcand_keys, str);
}
void
KeyBind::set_backspace_keys        (String str)
{
    scim_string_to_key_list(m_backspace_keys, str);
}
void
KeyBind::set_delete_keys           (String str)
{
    scim_string_to_key_list(m_delete_keys, str);
}
void
KeyBind::set_forward_keys          (String str)
{
    scim_string_to_key_list(m_forward_keys, str);
}
void
KeyBind::set_backward_keys         (String str)
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
KeyBind::match_latin_keys         (const KeyEvent &key)
{
    return match_key_event(m_latin_keys, key);
}
bool
KeyBind::match_wide_latin_keys    (const KeyEvent &key)
{
    return match_key_event(m_wide_latin_keys, key);
}
bool
KeyBind::match_convert_keys       (const KeyEvent &key)
{
    return match_key_event(m_convert_keys, key);
}
bool
KeyBind::match_start_conv_keys    (const KeyEvent &key)
{
    return match_key_event(m_start_conv_keys, key);
}
bool
KeyBind::match_cancel_keys        (const KeyEvent &key)
{
    return match_key_event(m_cancel_keys, key);
}
bool
KeyBind::match_latin_convert_keys        (const KeyEvent &key)
{
    return match_key_event(m_latin_convert_keys, key);
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
    KeyEventList::const_iterator it = std::find(keylist.begin(),
                                      keylist.end(),
                                      key);

    return it != keylist.end();
}

