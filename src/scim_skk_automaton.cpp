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

SKKAutomaton::SKKAutomaton ()
    : m_table (NULL),
      m_table_len (0),
      m_exact_match (NULL)
{
}

SKKAutomaton::~SKKAutomaton ()
{
}

bool
SKKAutomaton::append (const String & str,
                   WideString & result, WideString & pending)
{
    WideString widestr = utf8_mbstowcs (str);
    WideString newstr = m_pending + widestr;
    ConvRule *exact_match = NULL;
    bool has_partial_match = false;
    bool retval = false;

    /* FIXME! should be optimized */

    /* find matched table */
    for (unsigned int j = 0; j < m_tables.size (); j++) {
        for (unsigned int i = 0; m_tables[j][i].string; i++) {
            /* matching */
            WideString roma = utf8_mbstowcs(m_tables[j][i].string);
            if (roma.find (newstr) == 0) {
                if (roma.length () == newstr.length ()) {
                    /* exact match */
                    exact_match = &m_tables[j][i];
                } else {
                    /* partial match */
                    has_partial_match = true;
                }
            }
        }
    }

    /* return result */
    if (has_partial_match) {
        m_exact_match = exact_match;
        result.clear ();
        m_pending += widestr;
        pending   =  m_pending;
    } else if (exact_match) {
        if (exact_match->cont && *exact_match->cont)
            m_exact_match = exact_match;
        else
            m_exact_match = NULL;
        m_pending         = utf8_mbstowcs (exact_match->cont);
        result            = utf8_mbstowcs (exact_match->result);
        pending           = m_pending;
    } else {
        if (m_exact_match) {
            if (m_exact_match->result && *m_exact_match->result &&
                (!m_exact_match->cont || !*m_exact_match->cont))
            {
                result = utf8_mbstowcs (m_exact_match->result);
            } else {
                retval = true; /* commit prev pending */
            }
            m_pending.clear ();
            m_exact_match = NULL;

            WideString tmp_result;
            append(str, tmp_result, pending);
            result += tmp_result;
        } else {
            if (m_pending.length () > 0) {
                retval     = true; /* commit prev pending */
                m_pending  = widestr;
                pending    = m_pending;
                result.clear();
            } else {
                result.clear();
                pending.clear();
                m_pending.clear ();
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

WideString
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
    if (m_exact_match) {
        if (m_exact_match->result && *m_exact_match->result &&
                (!m_exact_match->cont || !*m_exact_match->cont))
        {
            result = utf8_mbstowcs (m_exact_match->result);
        } else if (m_exact_match->cont && *m_exact_match->cont) {
            result += utf8_mbstowcs (m_exact_match->cont);
        } else if (m_pending.length () > 0) {
            result += m_pending;
        }
    }
    clear ();
    return result;
}

void
SKKAutomaton::set_table (ConvRule *table)
{
    m_tables.clear ();
    m_tables.push_back (table);
}

void
SKKAutomaton::append_table (ConvRule *table)
{
    if (table)
        m_tables.push_back(table);
}

void
SKKAutomaton::remove_table (ConvRule *table)
{
    for (unsigned int i = 0; i < m_tables.size (); i++) {
        if (m_tables[i] == table)
            m_tables.erase(m_tables.begin() + i);
    }
}
