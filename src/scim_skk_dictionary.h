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
#include <list>

#define Uses_SCIM_ICONV
#include <scim.h>

#include "scim_skk_lookup_table.h"
#include "scim_skk_history.h"

using namespace scim;

namespace scim_skk {

class DictBase;
class UserDict;

class DictCache;

class SKKDictionary
{
    IConvert *m_converter;

    std::list<DictBase*> m_sysdicts;
    UserDict *m_userdict;

    DictCache *m_cache;
public:
    SKKDictionary  (void);
    ~SKKDictionary (void);

    void add_sysdict  (const String &dicturi);
    void set_userdict (const String &dictname, History &hist);

    void dump_userdict (void);

    void lookup (const WideString &key, const bool okuri,
                 SKKCandList &result);
    void write (const WideString &key, const CandEnt &ent);
    void extract_numbers (const WideString &key,
                          std::list<WideString> &result,
                          WideString &newkey);
    bool number_conversion (const std::list<WideString> &numbers,
                            const WideString &cand,
                            WideString &result);
};

} /* namespace scim_skk */

#endif
