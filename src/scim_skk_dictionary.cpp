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

#include "scim_skk_dictionary.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define SKKDICT_CHARCODE      "EUC-JP"

using namespace std;

SKKDictionaryBase::SKKDictionaryBase  (void)
{
}
SKKDictionaryBase::~SKKDictionaryBase  (void)
{
}


SKKDictionary::SKKDictionary  (SKKDictionaries *parent, bool writable)
    : m_dictpath   (""),
      m_parent     (parent),
      m_writeflag  (false),
      m_writable (writable)
{
}

SKKDictionary::~SKKDictionary (void)
{
}

void
SKKDictionary::load_dictdata (void)
{
    char *buf;
    int len = scim_load_file(m_dictpath, &buf);

    if (buf) {
        for (int i = 0; i < len; i++) {
            char c = buf[i];

            if (c == ';') {
                while (i < len && buf[i] != '\n') i++;
                continue;
            } else if (c == '\n') {
                continue;
            } else {
                int j;
                WideString key_w;

                for (j = 0; i+j < len && buf[i+j] != ' '; j++);
                char *key = new char[j+1];
                strncpy(key, buf+i, j);
                key[j] = '\0';
                m_iconv.convert(key_w, String(key));

                i += j+2;
                while (buf[i] != '\n') {
                    if (buf[i] == '[') {
                        while (buf[i] != ']') i++;
                        i+=2;
                        continue;
                    }

                    Candidate cand;
                    int candlen = 1;
                    while (buf[i+candlen] != '/') candlen++;
                    char *candstr = new char[candlen+1];
                    strncpy(candstr, buf+i, candlen);
                    candstr[candlen] = '\0';
                    m_iconv.convert(cand, String(candstr));
                    m_dictdata[key_w].push_back(cand);
                    i += candlen+1;
                    delete[] candstr;
                }
                delete[] key;
            }
        }

        /* delete[] buf; */
    }
}

void
SKKDictionary::dump_dict (void)
{
    Dict::iterator dit;
    ofstream dictfs;

    if (m_writable && m_writeflag) {
        dictfs.open(m_dictpath.c_str());
        for (dit = m_dictdata.begin(); dit != m_dictdata.end(); dit++) {
            String line;
            String tmp;
            m_iconv.convert(tmp, dit->first);
            line += tmp;
            line += ' ';

            for(list<Candidate>::iterator cit = dit->second.begin();
                cit != dit->second.end(); cit++) {
                tmp.clear();
                m_iconv.convert(tmp, *cit);
                line += '/';
                line += tmp;
            }
            line += "/\n";
            dictfs << line;
        }
        dictfs.close();
    }
}

void
SKKDictionary::load_dict (const String &dictpath)
{
    if (m_dictpath != dictpath) {
        m_dictpath.assign(dictpath);
        m_iconv.set_encoding(String(SKKDICT_CHARCODE));
        load_dictdata();
    }
}

void
SKKDictionary::lookup (const WideString &key, CandList &result,
                       CommonLookupTable &table)
{
    list<Candidate> &cl = m_dictdata[key];

    for (list<Candidate>::iterator it = cl.begin(); it != cl.end(); it++) {
        if (!m_parent->get_view_annot())
            m_parent->strip_annot(*it);

        CandList::iterator cit = find(result.begin(), result.end(), *it);
        if (cit == result.end()) {
            if (result.size() < m_parent->get_listsize()) {
                result.push_back(*it);
            } else {
                int i, len = table.number_of_candidates();
                for (i = 0; i < len; i++) {
                    if (*it == table.get_candidate(i))
                        break;
                }
                if (i == len) {
                    table.append_candidate(*it);
                }
            }
        }
    }
}

void
SKKDictionary::write (const WideString &key, const WideString &data)
{
    if (m_writable) { /* if not writable, do nothing */
        list<Candidate> &cl = m_dictdata[key];

        list<Candidate>::iterator it = find(cl.begin(), cl.end(), data);
        if (it != cl.end())
            cl.erase(it);
        cl.push_front(data);
        m_writeflag = true;
    }
}

SKKDictionaries::SKKDictionaries (void)
    : m_sysdict(this),
      m_userdict(this),
      m_view_annot (true),
      m_listsize (4)
{
}

SKKDictionaries::~SKKDictionaries (void)
{
}

void
SKKDictionaries::set_sysdict (const String &dictname)
{
    m_sysdict.load_dict(dictname);
}

void
SKKDictionaries::set_userdict (const String &dictname)
{
    struct stat statbuf;
    String userdictpath = scim_get_home_dir() +
                          String(SCIM_PATH_DELIM_STRING) + dictname;
    if (stat(userdictpath.c_str(), &statbuf) < 0) {
        String skkuserdict = scim_get_home_dir() +
                             String(SCIM_PATH_DELIM_STRING) + String(".skk-jisyo");
        m_userdict.load_dict(skkuserdict);
    }
    m_userdict.load_dict(userdictpath);
    m_userdict.m_writable = true;
}

void
SKKDictionaries::set_listsize (const int lsize)
{
    m_listsize = lsize;
}
int
SKKDictionaries::get_listsize (void)
{
    return m_listsize;
}

void
SKKDictionaries::set_view_annot (const bool view)
{
    m_view_annot = view;
}
bool
SKKDictionaries::get_view_annot (void)
{
    return m_view_annot;
}

void
SKKDictionaries::dump_userdict (void)
{
    m_userdict.dump_dict();
}

void
SKKDictionaries::lookup (const WideString &key, CandList &result,
                         CommonLookupTable &table)
{
    m_userdict.lookup(key, result, table);
    m_sysdict.lookup(key, result, table);
}

void
SKKDictionaries::write (const WideString &key, const WideString &data)
{
    m_userdict.write(key, data);
}


void
SKKDictionaries::strip_annot (WideString &str)
{
    ucs4_t c;
    unsigned char s[2] = {';', '\0'};
    utf8_mbtowc(&c, s, 1);
    int pos = str.find(c);
    if (pos != WideString::npos) {
        str.erase(pos);
    }
}


void
SKKNumDict::lookup (const WideString &key, CandList &result,
                    CommonLookupTable &table)
{
    /* int x = atoi(key.c_str()); */

    result.push_back(key);

}

void
convert_int_to_num1 (int src, WideString &dst)
{
    switch (src) {
    case 1:
        dst += utf8_mbstowcs("\xE4\xB8\x80");
        break;
    case 2:
        dst += utf8_mbstowcs("\xE4\xB8\x8C");
        break;
    case 3:
        dst += utf8_mbstowcs("\xE4\xB8\x89");
        break;
    case 4:
        dst += utf8_mbstowcs("\xE5\x9B\x9B");
        break;
    case 5:
        dst += utf8_mbstowcs("\xE4\xBA\x94");
        break;
    case 6:
        dst += utf8_mbstowcs("\xE5\x85\xAD");
        break;
    case 7:
        dst += utf8_mbstowcs("\xE4\xB8\x83");
        break;
    case 8:
        dst += utf8_mbstowcs("\xE5\x85\xAB");
        break;
    case 9:
        dst += utf8_mbstowcs("\xE4\xB9\x9D");
        break;
    }
}

void
convert_int_to_num (int src, WideString &dst)
{
    if (src >= 100000000) {
        convert_int_to_num(src/100000000, dst);
        dst += utf8_mbstowcs("\xE5\x84\x84"); /* oku */
        src = src % 100000000;
    }
    if (src >= 10000) {
        convert_int_to_num(src/10000, dst);
        dst += utf8_mbstowcs("\xE4\xB8\x87"); /* man */
        src = src % 10000;
    }
    if (src >= 1000) {
        if (src / 1000 != 1) {
            convert_int_to_num1(src/1000, dst);
        }
        dst += utf8_mbstowcs("\xE5\x8D\x83"); /* sen */
        src = src % 1000;
    }
    if (src > 100) {
        if (src / 100 != 1) {
            convert_int_to_num1(src/100, dst);
        }
        convert_int_to_num1(src/100, dst);
        dst += utf8_mbstowcs("\xE7\x99\xBE"); /* hyaku */
        src = src % 100;
    }
    if (src > 10) {
        if (src / 10 != 1) {
            convert_int_to_num1(src/10, dst);
        }
        dst += utf8_mbstowcs("\xE5\x8D\x81"); /* juu */
        src = src % 10;
    }
    convert_int_to_num1(src, dst);
}
