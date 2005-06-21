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
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include <list>
#include <map>
#include <utility>
#include <algorithm>

#include <scim_iconv.h>

#define SKKDICT_CHARCODE      "EUC-JP"

using namespace std;

static IConvert converter;

static void append_candpair(const WideString &cand,
                            const WideString &annot,
                            list<CandPair> &result);

class SKKSysDict : public SKKDictBase
{
    String  m_dictpath;
    char   *m_dictdata;
    int     m_fd;
    int     m_length;

    map<int, String> m_key_cache;

    vector<int> m_okuri_indice;
    vector<int> m_normal_indice;

    void get_key_from_index (int index, String &key);
    void get_cands_from_index (int index, list<CandPair> &result);

    void clear (void);
public:
    SKKSysDict  (const String &dictpath = 0);
    ~SKKSysDict (void);

    void load_dict (const String &dictpath);
    void lookup    (const WideString &key, const bool okuri,
                    list<CandPair> &result);
    bool compare (const String &dictname);
    bool compare (const String &host, const int port);
};


class SKKUserDict : public SKKDictBase
{
    String     m_dictpath;
    map<WideString, list<CandPair> >  m_dictdata;

    bool m_writeflag;
public:
    SKKUserDict  (void);
    ~SKKUserDict (void);

    void load_dict (const String &dictpath);
    void dump_dict (void);
    void lookup    (const WideString &key, const bool okuri,
                    list<CandPair> &result);
    void write     (const WideString &key, const CandPair &data);
    bool compare (const String &dictname);
    bool compare (const String &host, const int port);
};

class SKKNumDict : SKKDictBase
{
public:
    SKKNumDict  (void);
    virtual ~SKKNumDict (void);

    void lookup (const WideString &key, const bool okuri,
                 list<CandPair> &result);
    bool compare (const String &dictname);
    bool compare (const String &host, const int port);
};



SKKSysDict::SKKSysDict (const String &dictpath)
    : m_dictdata(0),
      m_dictpath("")
{
    if (!dictpath.empty())
        load_dict(dictpath);
}

SKKSysDict::~SKKSysDict (void)
{
    if (m_dictdata)
        delete[] m_dictdata;
    munmap(m_dictdata, m_length);
    close(m_fd);
}

void
SKKSysDict::load_dict (const String &dictpath)
{
    m_dictpath.assign(dictpath);
    struct stat statbuf;
    if (stat(m_dictpath.c_str(), &statbuf) < 0) return;

    if ((m_fd = open(m_dictpath.c_str(), O_RDONLY)) < 0) return;
    m_length = statbuf.st_size;
    m_dictdata = (char*)mmap(0, m_length, PROT_READ, MAP_SHARED, m_fd, 0);
    if (m_dictdata == MAP_FAILED) {
        close(m_fd);
        return;
    }

    vector<int>*indice = &m_okuri_indice;
    bool okuri_flag = false;
    int pos = 0;
    /* skip the header informations */
    while (pos < m_length && m_dictdata[pos] == ';') {
        for (; m_dictdata[pos] != '\n'; pos++);
        pos++;
    }

    while (pos < m_length) {
        if (m_dictdata[pos] == ';') {
            /* the boundary of okuri-ari/nasi entries */
            if (!okuri_flag) {
                okuri_flag = true;
                indice = &m_normal_indice;
            }
        } else {
            indice->push_back(pos);
        }
        for (; pos < m_length && m_dictdata[pos] != '\n'; pos++);
        pos++;
    }
}

void
SKKSysDict::get_key_from_index (int index, String &key)
{
    key.clear();
    if (index == 0 || m_dictdata[index-1] == '\n') {
        map<int, String>::const_iterator it = m_key_cache.find(index);
        if (it == m_key_cache.end()) {
            int s, e;
            s = index; e = 0;
            for (;m_dictdata[index] != ' '; index++, e++);
            key.assign(m_dictdata+s, e);
            m_key_cache.insert(make_pair(index, key));
        } else {
            key.assign(it->second);
        }
    }
}

void
SKKSysDict::get_cands_from_index(int index, list<CandPair> &result)
{
    if (index != 0 && m_dictdata[index-1] != '\n') return;

    /* skip key string */
    for(; m_dictdata[index] != ' '; index++);
    index+=2; /* skip first slash */

    WideString cand;
    WideString annot;
    int s;
    while (index < m_length && m_dictdata[index] != '\n') {
        if (m_dictdata[index] == '[') {
            /* skip [...] */
            for (; m_dictdata[index] != ']'; index++);
            index++;
            continue;
        }

        cand.clear(); annot.clear();
        /* clip a candidate */
        s = index;
        for (; m_dictdata[index] != ';' && m_dictdata[index] != '/'; index++);
        converter.convert(cand, m_dictdata+s, index - s);
        if (m_dictdata[index] == ';') {
            /* clip an annotation */
            index++;
            s = index;
            for (; m_dictdata[index] != '/'; index++);
            converter.convert(annot, m_dictdata+s, index-s);
        }
        index++;

        append_candpair(cand, annot, result);
    }
}

void
SKKSysDict::lookup (const WideString &key, const bool okuri,
                    list<CandPair> &result)
{
    String cmp_target;
    String key_s;
    vector<int> &indice = (okuri)? m_okuri_indice : m_normal_indice;
    int ub, lb, pos;

    converter.convert(key_s, key);

    ub = indice.size();
    lb = 0;

    while (true) {
        pos = (ub+lb)/2;
        get_key_from_index(indice[pos], cmp_target);
        if ((okuri && key_s < cmp_target) ||
            ((!okuri) && cmp_target < key_s)) {
            if (ub - lb <= 1)
                break;
            else 
                lb = pos;
        } else if ((okuri && cmp_target < key_s) ||
                   ((!okuri) && key_s < cmp_target)) {
            if (ub == lb)
                break;
            else
                ub = pos;
        } else {
            get_cands_from_index(indice[pos], result);
            break;
        }
    }
}

bool
SKKSysDict::compare (const String &dictname)
{
    return (!m_dictpath.empty()) && dictname == m_dictpath;
}

bool
SKKSysDict::compare (const String &host, const int port)
{
    return false;
}

SKKUserDict::SKKUserDict  (void)
    : m_writeflag  (false)
{
}

SKKUserDict::~SKKUserDict (void)
{
}

void
SKKUserDict::load_dict (const String &dictpath)
{
    //if (m_dictpath == dictpath) return;

    struct stat statbuf;
    int fd;
    int length;

    m_dictpath.assign(dictpath);

    if (stat(m_dictpath.c_str(), &statbuf) < 0) return;

    if ((fd = open(m_dictpath.c_str(), O_RDONLY)) == -1) return;

    length = statbuf.st_size;
    char *buf = (char*)mmap(0, length, PROT_READ, MAP_PRIVATE, fd, 0);

    if (buf != MAP_FAILED) {
        int keylen;
        WideString key;
        Candidate cand;
        Annotation annot;
        list<CandPair> cl;
        int candlen;
        for (int i = 0; i < length; i++) {
            switch(buf[i]) {
            case ';':
                for (;i < length && buf[i] != '\n'; i++);
            case '\n':
                break;
            default:
                key.clear();
                cl.clear();
                for (keylen = 0; i < length && buf[i+keylen] != ' '; keylen++);
                converter.convert(key, buf+i, keylen);

                i += keylen+2;
                while (buf[i] != '\n') {
                    if (buf[i] == '[') {
                        for (; buf[i] != ']'; i++);
                        i+=2;
                        continue;
                    }

                    cand.clear();
                    annot.clear();
                    candlen = 1;
                    while (buf[i+candlen] != '/' && buf[i+candlen] != ';')
                        candlen++;
                    converter.convert(cand, buf+i, candlen);
                    i += candlen+1;
                    if (buf[i-1] == ';') {
                        candlen = 0;
                        while(buf[i+candlen] != '/') candlen++;
                        converter.convert(annot, buf+i, candlen);
                        i += candlen + 1;
                    }
                    cl.push_back(make_pair(cand, annot));
                }
                m_dictdata.insert(make_pair(key, cl));
                break;
            }
        }
        munmap(buf, length);
    }
    close(fd);
}

void
SKKUserDict::dump_dict (void)
{
    DictCache::const_iterator dit;
    ofstream dictfs;

    if (m_writeflag) {
        dictfs.open(m_dictpath.c_str());
        for (dit = m_dictdata.begin(); dit != m_dictdata.end(); dit++) {
            String line;
            String tmp;
            converter.convert(tmp, dit->first);
            line += tmp;
            line += ' ';

            for(list<CandPair>::const_iterator cit = dit->second.begin();
                cit != dit->second.end(); cit++) {
                tmp.clear();
                converter.convert(tmp, cit->first);
                line += '/';
                line += tmp;
                if (!cit->second.empty()) {
                    tmp.clear();
                    converter.convert(tmp, cit->second);
                    line += ';';
                    line += tmp;
                }
            }
            dictfs << line << '/' << endl;
        }
        dictfs.close();
    }
}

void
SKKUserDict::lookup (const WideString &key, const bool okuri,
                     list<CandPair> &result)
{
    list<CandPair> &cl = m_dictdata[key];

    for (list<CandPair>::iterator it = cl.begin(); it != cl.end(); it++) {
        append_candpair(it->first, it->second, result);
    }
}

void
SKKUserDict::write (const WideString &key, const CandPair &data)
{
    list<CandPair> &cl = m_dictdata[key];
    for (list<CandPair>::iterator it = cl.begin(); it != cl.end(); it++) {
        if (it->first == data.first) {
            cl.erase(it);
            break;
        }
    }
    cl.push_front(data);
    m_writeflag = true;
}

bool
SKKUserDict::compare (const String &dictname)
{
    return (!m_dictpath.empty()) && dictname == m_dictpath;
}

bool
SKKUserDict::compare (const String &host, const int portn)
{
    return false;
}


SKKDictionary::SKKDictionary (void)
    : m_userdict(new SKKUserDict())
{
    converter.set_encoding(String(SKKDICT_CHARCODE));
}

SKKDictionary::~SKKDictionary (void)
{
    for (list<SKKDictBase*>::iterator i = m_sysdicts.begin();
         i != m_sysdicts.end(); i++)
        delete *i;
    if (m_userdict) delete m_userdict;
}

void
SKKDictionary::add_sysdict (const String &dictname)
{
    list<SKKDictBase*>::const_iterator it = m_sysdicts.begin();
    for(; it != m_sysdicts.end(); it++)
        if ((*it)->compare(dictname)) break;
    if (it == m_sysdicts.end()) {
        m_sysdicts.push_back((SKKDictBase*)new SKKSysDict(dictname));
    }
    m_cache.clear();
}

void
SKKDictionary::add_skkserv (const String &host, const int port)
{
}

void
SKKDictionary::set_userdict (const String &dictname)
{
    struct stat statbuf;
    String userdictpath = scim_get_home_dir() +
                          String(SCIM_PATH_DELIM_STRING) + dictname;
    if (stat(userdictpath.c_str(), &statbuf) < 0) {
        String skkuserdict = scim_get_home_dir() +
                             String(SCIM_PATH_DELIM_STRING) + String(".skk-jisyo");
        m_userdict->load_dict(skkuserdict);
    }
    m_userdict->load_dict(userdictpath);
}


void
SKKDictionary::dump_userdict (void)
{
    m_userdict->dump_dict();
}

void
SKKDictionary::lookup (const WideString &key, const bool okuri,
                         SKKCandList &result)
{
    DictCache::const_iterator cit = m_cache.find(key);
    result.clear();
    if (cit == m_cache.end()) {
        list<CandPair> cl;
        m_userdict->lookup(key, okuri, cl);
        for (list<SKKDictBase*>::const_iterator it = m_sysdicts.begin();
             it != m_sysdicts.end(); it++) {
            (*it)->lookup(key, okuri, cl);
        }
        result.copy(m_cache[key]);
        for(list<CandPair>::const_iterator it = cl.begin();
            it != cl.end(); it++) {
            result.append_candidate(it->first, it->second);
        }
        m_cache.insert(make_pair(key, cl));
    } else {
        for(list<CandPair>::const_iterator it = cit->second.begin();
            it != cit->second.end(); it++) {
            result.append_candidate(it->first, it->second);
        }
    }
}

void
SKKDictionary::write (const WideString &key, const CandPair &data)
{
    if (data.first.empty()) return;
    m_userdict->write(key, data);
    list<CandPair> &cl = m_cache[key];
    for (list<CandPair>::iterator cit = cl.begin();
         cit != cl.end(); cit++) {
        if (cit->first == data.first) {
            cl.erase(cit);
            break;
        }
    }
    cl.push_front(data);
}

void
SKKNumDict::lookup (const WideString &key, const bool okuri,
                    list<CandPair> &result)
{
    /* int x = atoi(key.c_str()); */

    /* result.push_back(key); */
}

bool SKKNumDict::compare (const String &dictname) { return false; }
bool SKKNumDict::compare (const String &host, const int port) { return false; }


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

void
append_candpair (const WideString &cand, const WideString &annot,
                 list<CandPair> &result)
{
    list<CandPair>::const_iterator it;
    for (it = result.begin(); it != result.end(); it++) {
        if (it->first == cand)
            break;
    }
    if (it == result.end()) {
        /* new candidate */
        result.push_back(make_pair(cand, annot));
    }
}
