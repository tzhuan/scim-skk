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

#ifndef __SCIM_SKK_DICTIONARY_H__
#define __SCIM_SKK_DICTIONARY_H__

#include <map>
#include <utility>
#include <deque>

#define Uses_SCIM_ICONV
#include <scim.h>

using namespace scim;

typedef std::pair<WideString, WideString> Candidate;
typedef std::deque<Candidate>             CandList;
typedef std::map<WideString, CandList>    Dict;

class SKKDictionaryBase
{
public:
    SKKDictionaryBase  (void) {}
    ~SKKDictionaryBase (void) {}

    virtual void lookup     (const WideString &key, CandList &result) = NULL;
};

class SKKDictionary : SKKDictionaryBase
{
    char      *m_dictpath;
    Dict       m_dictdata;
    IConvert   m_iconv;

    int m_writecount;

    bool m_writeflag;

    void load_dictdata (void);
    void dump_dictdata (void);
public:
    bool m_writable;

    SKKDictionary  (bool writable = false);
    ~SKKDictionary (void);

    void load_dict  (const String &dictpath);
    void dump_dict  (void);
    void lookup     (const WideString &key, CandList &result);
    void write      (const WideString &key, const WideString &data);
};

class SKKNumDict : SKKDictionaryBase
{
public:
    SKKNumDict  (void);
    ~SKKNumDict (void);

    void lookup (const WideString &key, CandList &result);
};


class SKKDictionaries
{
    SKKDictionary  m_sysdict;
    SKKDictionary  m_userdict;
public:
    SKKDictionaries  (void);
    ~SKKDictionaries (void);

    void set_sysdict  (const String &dictname);
    void set_userdict (const String &dictname);

    void lookup (const WideString &hira, CandList &result);
    void write (const WideString &key, const WideString &data);
};
#endif
