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
#include <list>

#define Uses_SCIM_ICONV
#define Uses_SCIM_LOOKUP_TABLE
#include <scim.h>

using namespace scim;

#if 0
typedef std::pair<WideString, WideString> Candidate;
typedef std::deque<Candidate>             CandList;
typedef std::map<WideString, CandList>    Dict;
#else
typedef WideString             Candidate;
typedef std::deque<WideString> CandList;
typedef std::map<WideString, std::list<WideString> > Dict;
#endif

class SKKDictionaries;

class SKKDictionaryBase
{
public:
    SKKDictionaryBase  (void);
    ~SKKDictionaryBase (void);

    virtual void lookup     (const WideString &key, CandList &result,
                             CommonLookupTable &table) = 0;
};

class SKKDictionary : SKKDictionaryBase
{
    String     m_dictpath;
    Dict       m_dictdata;
    IConvert   m_iconv;

    SKKDictionaries *m_parent;

    bool m_writeflag;
    void load_dictdata (void);
    void dump_dictdata (void);
public:
    bool m_writable;

    SKKDictionary  (SKKDictionaries *parent, bool writable = false);
    ~SKKDictionary (void);

    void load_dict  (const String &dictpath);
    void dump_dict  (void);
    void lookup     (const WideString &key, CandList &result,
                     CommonLookupTable &table);
    void write      (const WideString &key, const WideString &data);
};

class SKKNumDict : SKKDictionaryBase
{
public:
    SKKNumDict  (void);
    ~SKKNumDict (void);

    void lookup (const WideString &key, CandList &result,
                 CommonLookupTable &table);
};


class SKKDictionaries
{
    SKKDictionary  m_sysdict;
    SKKDictionary  m_userdict;

public:
    bool view_annot;
    int  listsize;

    SKKDictionaries  (void);
    ~SKKDictionaries (void);

    void set_sysdict  (const String &dictname);
    void set_userdict (const String &dictname);

    void lookup (const WideString &hira, CandList &result,
                 CommonLookupTable &table);
    void write (const WideString &key, const WideString &data);
    void strip_annot (WideString &str);
};
#endif
