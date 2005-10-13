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

#define Uses_SCIM_LOOKUP_TABLE

#include <scim.h>

#include "scim_skk_config.h"

using namespace scim;

namespace scim_skk {

typedef WideString Candidate;
typedef WideString Annotation;

struct CandEnt {
    Candidate  cand;   /* candidate string */
    Annotation annot;  /* annotation string */
    Candidate  cand_orig; /* candidate string with #-notation */
    CandEnt (const Candidate &cand = Candidate(),
             const Annotation &annot = Annotation(),
             const Candidate &cand_orig = Candidate());
};

class SKKCandList : public CommonLookupTable {
    struct CLBuffer;
    CLBuffer *m_annot_buf;
    CLBuffer *m_cand_orig_buf;

    SKKConfig *m_skkconfig;

    std::vector<CandEnt> m_candvec;
    int m_candindex;
public:
    SKKCandList  (SKKConfig *config, int page_size = 10);
    SKKCandList  (int page_size, const std::vector<WideString> &labels);
    ~SKKCandList (void);

    Candidate  get_cand (int index) const;
    Annotation get_annot (int index) const;
    Candidate  get_cand_orig (int index) const;

    /* candvec methods */
    virtual WideString get_cand_from_vector (int index = -1) const;
    virtual WideString get_annot_from_vector (int index = -1) const;
    virtual WideString get_candidate_from_vector (int index = -1) const;
    virtual CandEnt    get_candent_from_vector (int index = -1) const;
    virtual int get_candvec_size (void) const;
    virtual bool next_candidate (void);
    virtual bool prev_candidate (void);
    virtual bool vector_empty (void) const;
    virtual bool visible_table (void) const;

    /* normal lookup table methods */
    virtual bool has_candidate (const WideString &cand) const;

    virtual void clear (void);
    virtual WideString get_candidate (int index) const;
    virtual AttributeList get_attributes (int index) const;
    bool append_candidate (const WideString &cand,
                           const WideString &annot = WideString(),
                           const WideString &cand_orig = WideString(),
                           const AttributeList &attrs = AttributeList());

    virtual bool empty (void);

    /* list methods */
    virtual void copy (std::list<CandEnt> &dst);


    void SKKCandList::get_annot_string (WideString &result);
};

} /* namespace scim_skk */
#endif
