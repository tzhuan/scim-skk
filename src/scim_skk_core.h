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

#ifndef __SCIM_SKK_CORE_H__
#define __SCIM_SKK_CORE_H__

#include <scim.h>
#include "scim_skk_keybind.h"
#include "scim_skk_automaton.h"
#include "scim_skk_dictionary.h"


using namespace scim;

typedef enum {
    SKK_MODE_HIRAGANA,
    SKK_MODE_KATAKANA,
    SKK_MODE_HALF_KATAKANA,
    SKK_MODE_ASCII,
    SKK_MODE_WIDE_ASCII
} SKKMode;

typedef enum {
    INPUT_MODE_DIRECT,
    INPUT_MODE_PREEDIT,
    INPUT_MODE_OKURI,
    INPUT_MODE_CONVERTING,
    INPUT_MODE_LEARNING
} InputMode;

typedef enum {
    ACTION_NONE,
    ACTION_INIT,
    ACTION_UP,
    ACTION_DOWN,
    ACTION_CLEAR
} LookupTableAction;


class SKKCore
{
    KeyBind         *m_keybind;
    SKKDictionaries *m_dict;

    SKKMode        m_skk_mode;
    InputMode      m_input_mode;
    SKKAutomaton  *m_key2kana;

    WideString  m_pendingstr;
    WideString  m_preeditstr;
    WideString  m_okuristr;
    WideString  m_okurihead;
    WideString  m_commitstr;


    SKKCore      *m_learning;

    bool          m_commit_flag;
    bool          m_end_flag;

    int           m_preedit_pos;   /* caret position relative to preedit */
    int           m_commit_pos;    /* caret position relative to commit */

    /* for lookup table */
    bool                m_show_ltable;
    CommonLookupTable  *m_ltable;
    CandList            m_candlist;
    CandList::iterator  m_cindex;

    void commit_string     (WideString str);
    void commit_or_preedit (WideString str);
    void commit_converting (int index = -1);

    bool action_kakutei         (void);
    bool action_cancel          (void);
    bool action_convert         (void);
    bool action_katakana        (bool half = false);
    bool action_start_preedit   (void);
    bool action_prevcand        (void);
    bool action_ascii           (bool wide = false);
    bool action_ascii_convert   (void);
    bool action_backspace       (void);
    bool action_delete          (void);
    bool action_forward         (void);
    bool action_backward        (void);

    bool process_remaining_keybinds   (const KeyEvent &key);
    bool process_ascii                (const KeyEvent &key);
    bool process_wide_ascii           (const KeyEvent &key);
    bool process_romakana             (const KeyEvent &key);

    void clear_pending   (void);
    void clear_preedit   (void);
    void clear_candidate (void);

    void init_key2kana (void);
public:
    SKKCore  (KeyBind *keybind, SKKDictionaries *dict,
              SKKAutomaton *key2kana, CommonLookupTable *ltable);
    ~SKKCore (void);

    void        get_preedit_string (WideString &result);
    WideString &get_commit_string  (void);

    int  caret_pos (void);
    void move_preedit_caret (int pos);

    void      set_skk_mode   (SKKMode newmode);
    void      set_input_mode (InputMode newmode);
    SKKMode   get_skk_mode   (void);
    InputMode get_input_mode (void);

    inline bool has_commit_string (void) { return m_commit_flag; }

    void clear        (void);
    void clear_commit (void);

    bool show_lookup_table (void);

    bool process_key_event (const KeyEvent key);

    bool action_nextpage        (void);
    bool action_prevpage        (void);
    void action_select_index    (int i);
};

#endif
