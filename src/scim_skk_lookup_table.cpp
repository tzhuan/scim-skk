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

#include "scim_skk_lookup_table.h"

extern bool annot_view;
extern bool annot_pos;
extern bool annot_target;
extern int candvec_size;

struct SKKCandList::AnnotBuf
{
public:
    std::vector <ucs4_t> m_buffer;
    std::vector <uint32> m_index;
};

SKKCandList::SKKCandList (int page_size)
    : CommonLookupTable (page_size),
      m_annots (new AnnotBuf),
      m_candindex(0)
{
}

SKKCandList::SKKCandList (int page_size,
                          const std::vector<WideString> &labels)
    : CommonLookupTable (page_size, labels),
      m_annots (new AnnotBuf),
      m_candindex(0)
{
}

SKKCandList::~SKKCandList (void)
{
    delete m_annots;
}

bool
SKKCandList::has_candidate (const WideString &cand) const
{
    bool retval = true;
    for (int i = 0; i < number_of_candidates(); i++) {
        if (CommonLookupTable::get_candidate(i) == cand) {
            retval = false;
            break;
        }
    }
    return retval;
}

WideString
SKKCandList::get_cand (int index) const
{
    return CommonLookupTable::get_candidate(index);
}

WideString
SKKCandList::get_annot (int index) const
{
    if (index < 0 || index >= number_of_candidates())
        return WideString ();
    std::vector<ucs4_t>::const_iterator start, end;
    start = m_annots->m_buffer.begin() + m_annots->m_index[index];
    if (index < number_of_candidates() - 1)
        end = m_annots->m_buffer.begin() + m_annots->m_index[index+1];
    else
        end = m_annots->m_buffer.end();
    return WideString (start, end);
}

WideString
SKKCandList::get_candidate (int index) const
{
    WideString cand = CommonLookupTable::get_candidate(index);
    if (annot_view && annot_pos &&
        (annot_target || get_cursor_pos() == index)) {
        WideString annot = get_annot(index);
        if (!annot.empty()) {
            //cand += utf8_mbstowcs(";");
            cand += get_annot(index);
        }
    }
    return cand;
}

AttributeList
SKKCandList::get_attributes (int index) const
{
    AttributeList al = CommonLookupTable::get_attributes(index);
    if (annot_view && annot_pos &&
        (annot_target || get_cursor_pos() == index)) {
        WideString annot = get_annot(index);
        WideString cand = get_cand(index);
        if (!annot.empty()) {
            al.push_back(Attribute(cand.length(), annot.length(),
                                   SCIM_ATTR_BACKGROUND,
                                   0xa0ff80));
        }
    }
    return al;
}

bool
SKKCandList::append_candidate (const WideString &cand,
                               const WideString &annot,
                               const AttributeList &attrs)
{
    if (cand.length() == 0)
        return false;

    if (m_candvec.size() < candvec_size) {
        m_candvec.push_back(make_pair(cand, annot));
        return true;
    } else {
        m_annots->m_index.push_back(m_annots->m_buffer.size());
        if (!annot.empty())
            m_annots->m_buffer.insert(m_annots->m_buffer.end(),
                                      annot.begin(), annot.end());
        return CommonLookupTable::append_candidate(cand, attrs);
    }
}

void
SKKCandList::clear (void)
{
    m_candvec.clear();
    m_candindex = 0;
    m_annots->m_buffer.clear();
    m_annots->m_index.clear();
    CommonLookupTable::clear();
}


/* candvec methods */
WideString
SKKCandList::get_cand_from_vector (int index)
{
    return get_candpair_from_vector(index).first;
}

WideString
SKKCandList::get_annot_from_vector (int index)
{
    return get_candpair_from_vector(index).second;
}

CandPair
SKKCandList::get_candpair_from_vector (int index)
{
    try {
        return m_candvec.at(index);
    } catch(...) {
        try {
            return m_candvec.at(m_candindex);
        } catch(...) {
            return make_pair(WideString(), WideString());
        }
    }
}

WideString 
SKKCandList::get_candidate_from_vector (int index)
{
    CandPair p = get_candpair_from_vector(index);
    if (annot_view && annot_pos && !p.second.empty())
        return  p.first + utf8_mbstowcs(";") + p.second;
    else
        return p.first;
}

int
SKKCandList::get_candvec_size (void)
{
    return m_candvec.size();
}

bool
SKKCandList::visible_table (void)
{
    return m_candindex >= m_candvec.size() && number_of_candidates() > 0;
}

bool
SKKCandList::vector_empty (void)
{
    return m_candvec.size() == 0;
}

bool
SKKCandList::next_candidate (void)
{
    m_candindex++;
    return m_candindex < m_candvec.size();
}

bool
SKKCandList::prev_candidate (void)
{
    if (m_candindex == 0) return false;
    m_candindex--;
    return true;
}

void
SKKCandList::copy (std::list<CandPair> &dst)
{
    for(std::vector<CandPair>::const_iterator it  = m_candvec.begin();
        it != m_candvec.end(); it++) {
        dst.push_back(*it);
    }
    int n = number_of_candidates();
    for (int i = 0; i < n; i++) {
        WideString cand = get_cand(i);
        WideString annot = get_annot(i);
        dst.push_back(make_pair(cand, annot));
    }
}

bool
SKKCandList::empty (void)
{
    return vector_empty() && number_of_candidates() == 0;
}

void
SKKCandList::get_annot_string (WideString &result)
{
    if (!visible_table()) {
        result += get_annot_from_vector();
    } else {
        int i = get_current_page_start();
        int s = get_current_page_size();
        bool is_first = true;
        int cpos = get_cursor_pos_in_current_page();
        for (int j = 0; j < s; i++, j++) {
            std::vector<ucs4_t>::const_iterator start, end;
            start = m_annots->m_buffer.begin() + m_annots->m_index[i];
            if (i < number_of_candidates() - 1)
                end = m_annots->m_buffer.begin() + m_annots->m_index[i+1];
            else
                end = m_annots->m_buffer.end();
            if (start != end && (annot_target || j == cpos)) {
                if (is_first)
                    is_first = false;
                else
                    result += utf8_mbstowcs("; ");
                if (annot_target) {
                    result += get_candidate_label(j);
                    result += utf8_mbstowcs(".");
                }
                result.append(start, end);
            }
        }
    }
}
