/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4  -*- */
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

#define SKKDICT_MAXLEN        4096
#define SKKDICT_CHARCODE      "EUC-JP"

SKKDictionary::SKKDictionary  (bool writable)
    : m_writable (writable),
      m_writecount (0)
{
}

SKKDictionary::~SKKDictionary (void)
{
    delete[] m_dictpath;
    dump_dictdata();
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
                        while (i < len && buf[i] != ']') i++;
                        i+=2;
                        continue;
                    }

                    Candidate cand;
                    int candlen = 1;
                    for (; i+candlen < len && buf[i+candlen] != '/'; candlen++);
                    char *candstr = new char[candlen+1];
                    strncpy(candstr, buf+i, candlen);
                    candstr[candlen] = '\0';
                    char *x = strchr(candstr, ';');
                    if (x) {
                        int len = x - candstr;
                        char *tmp1 = new char[len+1];
                        strncpy(tmp1, candstr, len);
                        tmp1[len] = '\0';
                        m_iconv.convert(cand.first, String(tmp1));
                        m_iconv.convert(cand.second, String(x+1));
                        delete[] tmp1;
                    } else {
                        m_iconv.convert(cand.first, String(candstr));
                    }
                    m_dictdata[key_w].push_back(cand);
                    i += candlen+1;
                    delete[] candstr;
                }
            }
        }

        /* delete[] buf; */
    }
}

void
SKKDictionary::dump_dictdata (void)
{
    Dict::iterator dit;
    std::ofstream dictfs;

    if (m_writable && m_writeflag) {
        dictfs.open(m_dictpath);
        for (dit = m_dictdata.begin(); dit != m_dictdata.end(); dit++) {
            String line;
            WideString tmp = dit->first;
            String tmp2;
            m_iconv.convert(tmp2, tmp);
            line += tmp2;
            tmp.clear();
            line += ' ';

            for(CandList::iterator cit = dit->second.begin();
                    cit != dit->second.end(); cit++) {
                tmp2.clear();
                m_iconv.convert(tmp2, cit->first);
                line += '/';
                line += tmp2;
                if(cit->second.length() > 0) {
                    line += ';';
                    tmp2.clear();
                    m_iconv.convert(tmp2, cit->second);
                    line += tmp2;
                }
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
    m_dictpath = new char[dictpath.length()+1];
    dictpath.copy(m_dictpath, dictpath.size(), 0);
    m_dictpath[dictpath.length()] = '\0';
    m_iconv.set_encoding(String(SKKDICT_CHARCODE));
    load_dictdata();
}

void
SKKDictionary::dump_dict (void)
{
    dump_dictdata();
}

void
SKKDictionary::lookup (const WideString &key, CandList &result)
{
    CandList &cl = m_dictdata[key];
    for (CandList::iterator it = cl.begin(); it != cl.end(); it++) {
        result.push_back(*it);
    }
}

void
SKKDictionary::write (const WideString &key, const WideString &data)
{
    if (m_writable) { /* if not writable, do nothing */
        CandList &cl = m_dictdata[key];
        Candidate cand;

        for (CandList::iterator it = cl.begin(); it != cl.end(); it++) {
            if (it->first == data) {
                cand.second = it->second;
                cl.erase(it);
                break;
            }
        }
        cand.first = data;
        cl.push_front(cand);
        m_writeflag = true;
        m_writecount++;
        if (m_writecount > 10) {
            dump_dict();
        }
    }
}

SKKDictionaries::SKKDictionaries (void)
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
SKKDictionaries::lookup (const WideString &key, CandList &result)
{
    CandList scl;
    m_userdict.lookup(key, result);
    m_sysdict.lookup(key, scl);
    for (CandList::iterator cit = scl.begin(); cit != scl.end(); cit++) {
        CandList::iterator i = std::find(result.begin(), result.end(), *cit);
        if (i == result.end()) {
            result.push_back(*cit);
        }
    }
}

void
SKKDictionaries::write (const WideString &key, const WideString &data)
{
    m_userdict.write(key, data);
}


void
SKKNumDict::lookup (const WideString &key, CandList &result)
{
    /* int x = atoi(key.c_str()); */

    result.push_back(make_pair(key, WideString()));

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
