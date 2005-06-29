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

#ifndef __SCIM_SKK_CANDIDATES_H__
#define __SCIM_SKK_CANDIDATES_H__

#include <vector>
#include <list>
#include <utility>

#define Uses_SCIM_LOOKUP_TABLE

#include <scim.h>

using namespace scim;

typedef WideString Candidate;
typedef WideString Annotation;
typedef std::pair<Candidate, Annotation> CandPair;

class SKKCandList : public CommonLookupTable {
    struct AnnotBuf;
    AnnotBuf *m_annots;

    std::vector<CandPair> m_candvec;
    int m_candindex;
public:
    SKKCandList  (int page_size = 10);
    SKKCandList  (int page_size, const std::vector<WideString> &labels);
    ~SKKCandList (void);

    WideString get_cand (int index) const;
    WideString get_annot (int index) const;

    /* candvec methods */
    virtual WideString get_cand_from_vector (int index = -1);
    virtual WideString get_annot_from_vector (int index = -1);
    virtual WideString get_candidate_from_vector (int index = -1);
    virtual CandPair   get_candpair_from_vector (int index = -1);
    virtual int get_candvec_size (void);
    virtual bool next_candidate (void);
    virtual bool prev_candidate (void);
    virtual bool vector_empty (void);
    virtual bool visible_table (void);

    /* normal lookup table methods */
    virtual bool has_candidate (const WideString &cand) const;

    virtual void clear (void);
    virtual WideString get_candidate (int index) const;
    virtual AttributeList get_attributes (int index) const;
    bool append_candidate (const WideString &cand,
                           const WideString &annot = WideString(),
                           const AttributeList &attrs = AttributeList());

    virtual bool empty (void);

    /* list methods */
    virtual void copy (std::list<CandPair> &dst);


    void SKKCandList::get_annot_string (WideString &result);
};
#endif
