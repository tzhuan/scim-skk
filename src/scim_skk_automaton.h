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

#ifndef __SCIM_ANTHY_AUTOMATON_H__
#define __SCIM_ANTHY_AUTOMATON_H__

#define Uses_SCIM_ICONV
#include <scim.h>
using namespace scim;

typedef struct _ConvRule
{
    const char *string;
    const char *result;
    const char *cont;
} ConvRule;

typedef struct _HiraganaKatakanaRule
{
    const char *hiragana;
    const char *katakana;
    const char *half_katakana;
} HiraganaKatakanaRule;

typedef struct _WideRule
{
    const char *code;
    const char *wide;
} WideRule;

#if 0
class ConvRule
{
    WideString *m_string;
    WideString *m_result;
    WideString *m_continue;

public:
    ConvRule ();
    virtual ~ConvRule();
};
#endif

class SKKAutomaton
{
    WideString              m_pending;
    ConvRule               *m_table;
    unsigned int            m_table_len;
    ConvRule               *m_exact_match;
    std::vector<ConvRule*>  m_tables;

public:
    enum {
        HAS_PARSHAL_MATCH,
        HAS_EXACT_MATCH,
        COMMIT_PREV_PENDING,
    };

    SKKAutomaton ();
    virtual ~SKKAutomaton ();

    virtual bool       append             (const String & str,
                                           WideString   & result,
                                           WideString   & pending);
    virtual void       clear              (void);

    virtual bool       is_pending         (void);
    virtual WideString get_pending        (void);
    virtual WideString flush_pending      (void);
    virtual void       set_pending        (WideString &pending);

    virtual void       set_table          (ConvRule *table);
    virtual void       append_table       (ConvRule *table);
    virtual void       remove_table       (ConvRule *table);
};
#endif /* __SCIM_ANTHY_AUTOMATON_H__ */
/*
vi:ts=4:nowrap:ai:expandtab
*/
