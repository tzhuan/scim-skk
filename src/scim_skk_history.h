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

#ifndef __SCIM_SKK_HISTORY_H__
#define __SCIM_SKK_HISTORY_H__

#include <scim.h>
#include <list>

using namespace scim;
namespace scim_skk {
class History {
    struct HistoryImpl;
    HistoryImpl *m_impl;

    void get_current_history (const WideString &str,
                              std::list<WideString> &result);
public:
    History(void);
    ~History(void);

    /* manager class */
    class Manager {
        History       &m_hist;
        std::list<WideString> m_hist_cur;
        std::list<WideString>::iterator m_hist_it;
    public:
        Manager(History &hist);
        ~Manager(void);
        void setup_completion (const WideString &str);
        void clear (void);
        bool is_clear (void);
        bool next_cand (void);
        bool prev_cand (void);
        void get_current_candidate (WideString &result);
    };

    void add_entry (const WideString &str);
    void append_entry_to_tail (const WideString &str);
    /* this method is to append user dictionary entries only */
};
} /* namespace scim_skk */
#endif
