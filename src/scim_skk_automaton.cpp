/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2004 Hiroyuki Ikezoe
 *  Copyright (C) 2004 Takuro Ashie
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

#include "scim_skk_automaton.h"

using namespace scim_skk;


ConvEntry::ConvEntry (WideString s, WideString r, WideString c)
    : str (s), res (r), cont (c)
{
}

SKKAutomaton::SKKAutomaton (WideString title)
    : m_exact_match (NULL),
      m_title (title)
{
}

SKKAutomaton::~SKKAutomaton ()
{
}

WideString&
SKKAutomaton::get_title (void)
{
    return m_title;
}

void
SKKAutomaton::set_title (const WideString &title)
{
    m_title = title;
}


bool
SKKAutomaton::append (const String & str, WideString & result)
{
    WideString widestr = utf8_mbstowcs (str);
    WideString newstr = m_pending + widestr;
    const ConvEntry *exact_match = NULL;
    bool has_partial_match = false;
    bool retval = false;

    /* FIXME! should be optimized */

    /* find matched table */
    for (std::list<ConvEntry>::const_iterator it = m_rules.begin();
         it != m_rules.end();
         it++) {
        /* matching */
        const WideString &roma = it->str;
        if (roma.find (newstr) == 0) {
            if (roma.length () == newstr.length ()) {
                /* exact match */
                exact_match = &(*it);
            } else {
                /* partial match */
                has_partial_match = true;
            }
        }
    }

    /* return result */
    if (has_partial_match) {
        m_exact_match = exact_match;
        result.clear ();
        m_pending += widestr;
    } else if (exact_match) {
        if (exact_match->cont.empty())
            m_exact_match = NULL;
        else
            m_exact_match = exact_match;
        m_pending         = exact_match->cont;
        result           += exact_match->res;
    } else {
        retval = true; /* commit prev pending */
        if (m_exact_match) {
            WideString tmp_result;

            if (!(m_exact_match->res.empty()) &&
                m_exact_match->cont.empty()) {
                result += m_exact_match->res;
            }
            m_pending.clear();
            m_exact_match = NULL;

            append(str, tmp_result);
            result += tmp_result;
        } else {
            if (m_pending.length () > 0) {
                m_pending.clear();
                append(str, result);
            } else {
                result.clear();
                for (int i = 0; i < str.size(); i++) {
                    if (isalpha(str[i]))
                        m_pending += widestr[i];
                }
            }
        }
    }

    return retval;
}

void
SKKAutomaton::clear (void)
{
    m_pending.clear ();
    m_exact_match = NULL;
}

bool
SKKAutomaton::is_pending (void)
{
    if (m_pending.length () > 0)
        return true;
    else
        return false;
}

WideString&
SKKAutomaton::get_pending (void)
{
    return m_pending;
}

void
SKKAutomaton::set_pending (WideString &pending)
{
    m_pending = pending;
}

WideString
SKKAutomaton::flush_pending (void)
{
    WideString result;
    if (m_exact_match &&
        !(m_exact_match->res.empty()) && m_exact_match->cont.empty()) {
        result = m_exact_match->res;
    }
    clear();
    return result;
}

void
SKKAutomaton::set_rules (ConvRule *rules)
{
    clear_rules();
    append_rules(rules);
}

void
SKKAutomaton::append_rules (ConvRule *rules)
{
    for (unsigned int i = 0; rules[i].string; i++) {
        m_rules.push_front(ConvEntry(utf8_mbstowcs(rules[i].string),
                                     utf8_mbstowcs(rules[i].result),
                                     utf8_mbstowcs(rules[i].cont)));
    }
}

void
SKKAutomaton::replace_rules (ConvRule *rules)
{
    for (std::list<ConvEntry>::iterator it = m_rules.begin();
         it != m_rules.end(); ) {
        bool flag = true;
        for (unsigned int i = 0; rules[i].string; i++) {
            if (it->str == utf8_mbstowcs(rules[i].string)) {
                it = m_rules.erase(it);
                flag = false;
                break;
            }
        }
        if (flag) it++;
    }
    append_rules (rules);
}

void
SKKAutomaton::clear_rules (void)
{
    m_rules.clear();
}

void
SKKAutomaton::append_rule (String &key, std::vector<String> &vals)
{
    if (vals.size() > 1) {
        m_rules.push_back(ConvEntry(utf8_mbstowcs(key),
                                    utf8_mbstowcs(vals[0]),
                                    utf8_mbstowcs(vals[1])));
    } else {
        m_rules.push_back(ConvEntry(utf8_mbstowcs(key),
                                    utf8_mbstowcs(vals[0]),
                                    WideString()));
    }
}
