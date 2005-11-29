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

#define Uses_SCIM_ICONV
#include "scim_skk_history.h"
#include <map>
#include <list>

using namespace std;
using namespace scim;

namespace scim_skk {

struct History::HistoryImpl {
    map<ucs4_t, list<WideString> > m_hist;
};

History::History (void)
    : m_impl(new HistoryImpl())
{}

History::~History (void)
{
    delete m_impl;
}

void
History::add_entry (const WideString &str)
{
    if (str.empty()) return;

    ucs4_t first_char = str[0];
    list<WideString> *lst = &(m_impl->m_hist[first_char]);
    for (list<WideString>::iterator it = lst->begin();
         it != lst->end(); it++) {
        if (str == *it) {
            lst->erase(it);
            break;
        }
    }
    lst->push_front(str);
}

void
History::append_entry_to_tail (const WideString &str)
{
    if (str.empty()) return;

    ucs4_t first_char = str[0];
    list<WideString> *lst = &(m_impl->m_hist[first_char]);
    lst->push_back(str);
}

void
History::get_current_history (const WideString &str, list<WideString> &result)
{
    if (str.empty()) return;

    ucs4_t first_char = str[0];
    list<WideString> *lst = &(m_impl->m_hist[first_char]);

    for (list<WideString>::const_iterator it = lst->begin();
         it != lst->end(); it++)
        if (str.size() < it->size() && str == it->substr(0, str.size()))
            result.push_back(*it);
}

/*
 * class History:;Manager
 *   an iterator like classes
 *
 */
History::Manager::Manager(History &hist)
    : m_hist(hist) {}
History::Manager::~Manager(void) {}


void
History::Manager::setup_completion (const WideString &str) {
    if (m_hist_cur.empty()) {
        m_hist.get_current_history(str, m_hist_cur);
        m_hist_it = m_hist_cur.begin();
    }
}
void
History::Manager::clear (void) {
    m_hist_cur.clear();
}

bool
History::Manager::is_clear (void) {
    return m_hist_cur.empty();
}

bool
History::Manager::next_cand (void) {
    if (m_hist_cur.empty()) return false;
    m_hist_it++;
    if (m_hist_it == m_hist_cur.end()) m_hist_it = m_hist_cur.begin();
    return true;
}

bool
History::Manager::prev_cand (void) {
    if (m_hist_cur.empty()) return false;
    if (m_hist_it == m_hist_cur.begin())
        m_hist_it = m_hist_cur.end();
    m_hist_it--;
    return true;
}

void
History::Manager::get_current_candidate (WideString &result) {
    if (m_hist_it != m_hist_cur.end()) {
        result.assign(*m_hist_it);
    } 
}

} /* namespace scim_skk */
