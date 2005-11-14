/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef __SCIM_SKK_KEYBINDS_H__
#define __SCIM_SKK_KEYBINDS_H__

#include <scim.h>

using namespace scim;

namespace scim_skk {

typedef enum {
    SSTYLE_QWERTY,
    SSTYLE_DVORAK,
    SSTYLE_NUMBER
} SelectionStyle;

class KeyBind {
    KeyEventList m_kakutei_keys;
    KeyEventList m_katakana_keys;
    KeyEventList m_half_katakana_keys;
    KeyEventList m_ascii_keys;
    KeyEventList m_wide_ascii_keys;
    KeyEventList m_convert_keys;
    KeyEventList m_start_preedit_keys;
    KeyEventList m_cancel_keys;
    KeyEventList m_ascii_convert_keys;
    KeyEventList m_prevcand_keys;
    KeyEventList m_backspace_keys;
    KeyEventList m_delete_keys;
    KeyEventList m_forward_keys;
    KeyEventList m_backward_keys;
    KeyEventList m_home_keys;
    KeyEventList m_end_keys;
    KeyEventList m_upcase_keys;
    KeyEventList m_completion_keys;

    SelectionStyle m_style;

    int match_selection_qwerty (const KeyEvent &key);
    int match_selection_dvorak (const KeyEvent &key);
    int match_selection_number (const KeyEvent &key);

public:
    KeyBind  (void);
    ~KeyBind (void);

    void set_kakutei_keys       (const String &str);
    void set_katakana_keys      (const String &str);
    void set_half_katakana_keys (const String &str);
    void set_ascii_keys         (const String &str);
    void set_wide_ascii_keys    (const String &str);
    void set_convert_keys       (const String &str);
    void set_start_preedit_keys (const String &str);
    void set_cancel_keys        (const String &str);
    void set_ascii_convert_keys (const String &str);
    void set_prevcand_keys      (const String &str);
    void set_backspace_keys     (const String &str);
    void set_delete_keys        (const String &str);
    void set_forward_keys       (const String &str);
    void set_backward_keys      (const String &str);
    void set_home_keys          (const String &str);
    void set_end_keys           (const String &str);
    void set_upcase_keys        (const String &str);
    void set_completion_keys    (const String &str);

    void set_selection_style    (const String &str);

    bool match_kakutei_keys       (const KeyEvent &key);
    bool match_katakana_keys      (const KeyEvent &key);
    bool match_half_katakana_keys (const KeyEvent &key);
    bool match_ascii_keys         (const KeyEvent &key);
    bool match_wide_ascii_keys    (const KeyEvent &key);
    bool match_convert_keys       (const KeyEvent &key);
    bool match_start_preedit_keys (const KeyEvent &key);
    bool match_cancel_keys        (const KeyEvent &key);
    bool match_ascii_convert_keys (const KeyEvent &key);
    bool match_prevcand_keys      (const KeyEvent &key);
    bool match_backspace_keys     (const KeyEvent &key);
    bool match_delete_keys        (const KeyEvent &key);
    bool match_forward_keys       (const KeyEvent &key);
    bool match_backward_keys      (const KeyEvent &key);
    bool match_home_keys          (const KeyEvent &key);
    bool match_end_keys           (const KeyEvent &key);
    bool match_upcase_keys        (const KeyEvent &key);
    bool match_completion_keys    (const KeyEvent &key);

    /* returns -1 if no match. return 0-origin number otherwise. */
    int  match_selection_keys     (const KeyEvent &key);
    int  selection_key_length     (void);

    void selection_labels (std::vector<WideString> &result);
};

} /* namespace scim_skk */

#endif /* __SCIM_SKK_KEYBINDS_H__ */
