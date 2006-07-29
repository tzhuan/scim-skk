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
#include "cdb.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include <list>
#include <map>
#include <utility>
#include <algorithm>
#include <cstring>
#include <fstream>

#include <scim_iconv.h>

#include <scim_socket.h>

#define SKKDICT_CHARCODE      "EUC-JP"

using namespace std;

typedef std::pair<WideString, WideString> CandPair;


static void append_candpair(const WideString &cand,
                            const WideString &annot,
                            list<CandPair> &result);

static int parse_dictline (const IConvert *converter, const char *line,
                           list<CandPair> &ret);
static void rewrite_to_concatform (String &dst, const String &src);

inline void convert_num1 (WideString key, WideString &result);
inline void convert_num2 (WideString key, WideString &result);
inline void convert_num3 (WideString key, WideString &result);
inline void convert_num5 (WideString key, WideString &result);
inline void convert_num9 (WideString key, WideString &result);

inline WideString lltows(unsigned long long n);
inline unsigned long long wstoll(WideString ws);

/* class declarations */
namespace scim_skk {

class DictBase
{
public:
    IConvert *m_converter;
    const String dicturi;

    DictBase  (IConvert *conv=0, const String &uri = "")
        : m_converter(conv), dicturi(uri) {}
    virtual ~DictBase (void) {}

    virtual void lookup (const WideString &key, const bool okuri,
                         list<CandPair> &result) = 0;
};

class DictCache : public DictBase
{
    map<WideString, list<CandPair> > m_cache;
public:
    DictCache  (void) {}
    ~DictCache (void) { m_cache.clear(); }

    void lookup (const WideString &key, const bool okuri,
                 list<CandPair> &result)
    {
        map<WideString, list<CandPair> >::const_iterator cit
            = m_cache.find(key);
        if (cit != m_cache.end()) {
            for(list<CandPair>::const_iterator it = cit->second.begin();
                it != cit->second.end(); it++) {
                result.push_back(*it);
            }
        }
    }
    void write (const WideString &key, const list<CandPair> &data)
    {
        list<CandPair> &cl = m_cache[key];
        cl.clear();
        cl.assign(data.begin(), data.end());
    }
    void write (const WideString &key, const CandPair &data)
    {
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
    void clear (void) { m_cache.clear(); }
};

class DictFile : public DictBase
{
    char   *m_dictdata;
    int     m_length;

    map<int, String> m_key_cache;

    vector<int> m_okuri_indice;
    vector<int> m_normal_indice;

    const String m_dictpath;

    void get_key_from_index (int index, String &key);
    void get_cands_from_index (int index, list<CandPair> &result);

    void clear (void);

    void load_dict (void);
public:
    DictFile  (IConvert *conv, const String &dictpath = 0);
    ~DictFile (void);

    void lookup    (const WideString &key, const bool okuri,
                    list<CandPair> &result);
};

class SKKServ : public DictBase
{
    SocketClient m_socket;
    SocketAddress m_addr;

    bool close(void);
public:
    SKKServ  (IConvert *conv, const String &addrstr);
    ~SKKServ (void);
    void lookup (const WideString &key, const bool okuri,
                 list<CandPair> &result);
};

class CDBFile : public DictBase
{
    CDB m_db;
public:
    CDBFile (IConvert *conv, const String &dictpath);
    ~CDBFile (void);

    void lookup (const WideString &key, const bool okuri,
                 list<CandPair> &result);
};

class UserDict : public DictBase
{
    String     m_dictpath;
    map<WideString, list<CandPair> >  m_dictdata;

    bool m_writeflag;
public:
    const String dicturi;
    UserDict  (IConvert *conv);
    ~UserDict (void);

    void load_dict (const String &dictpath, History &hist);
    void dump_dict (void);
    void lookup    (const WideString &key, const bool okuri,
                    list<CandPair> &result);
    void write     (const WideString &key, const CandPair &data);
};

}

using namespace scim_skk;


/*
 * scim_skk::DictFile
 * a system dictionary object which access to the dictionary file directly.
 */

DictFile::DictFile (IConvert *conv, const String &dictpath)
    : DictBase(conv, String("DictFile:")+dictpath),
      m_dictdata(0),
      m_dictpath(dictpath)
{
    if (!dictpath.empty())
        load_dict();
}

DictFile::~DictFile (void)
{
    //munmap(m_dictdata, m_length);
}

void
DictFile::load_dict (void)
{
    struct stat statbuf;
    int fd;
    if (stat(m_dictpath.c_str(), &statbuf) < 0) return;

    if ((fd = open(m_dictpath.c_str(), O_RDONLY)) < 0) return;
    m_length = statbuf.st_size;
    m_dictdata = (char*)mmap(0, m_length, PROT_READ, MAP_SHARED, fd, 0);
    close(fd);
    if (m_dictdata == MAP_FAILED) {
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
    if (!okuri_flag) {
        for (vector<int>::const_iterator it = m_okuri_indice.begin();
             it != m_okuri_indice.end(); it++) {
            m_normal_indice.push_back(*it);
        }
        m_okuri_indice.clear();
    }
}

void
DictFile::get_key_from_index (int index, String &key)
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
DictFile::get_cands_from_index(int index, list<CandPair> &result)
{
    int len;
    for (len = 0; m_dictdata[index+len] != '\n'; len++);
    parse_dictline(m_converter, m_dictdata+index, result);
}

void
DictFile::lookup (const WideString &key, const bool okuri,
                    list<CandPair> &result)
{
    String cmp_target;
    String key_s;
    vector<int> &indice = (okuri)? m_okuri_indice : m_normal_indice;
    int ub, lb, pos;

    m_converter->convert(key_s, key);

    if(indice.size() == 0) return;
	
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

/*
 * scim_skk::SKKServ
 * a client which connect to skkserv.
 */

#include <ostream>
#include <fstream>
SKKServ::SKKServ  (IConvert *conv,
                   const String &addressstr)
    : DictBase(conv, String("SKKServ:") + addressstr),
      m_addr(String("inet:") + addressstr)
{
}

SKKServ::~SKKServ (void)
{
    if (m_socket.is_connected()) close();
}

bool
SKKServ::close (void)
{
    if (m_socket.is_connected()) {
        if (m_socket.write("0\n", 2) > 0) {
            m_socket.close();
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

void
SKKServ::lookup (const WideString &key, const bool okuri,
                 list<CandPair> &result)
{
    const int buflen = 4096;
    /* reconnect if the connection is closed */
    if (!m_socket.is_connected() && !m_socket.connect(m_addr)) return;

    String skey;
    m_converter->convert(skey, key);
    char writebuf[skey.size() + 3];
    char readbuf[buflen];
    writebuf[0] = '1';
    
    skey.copy(writebuf+1, skey.size());
    writebuf[skey.size()+1] = ' ';
    writebuf[skey.size()+2] = '\n';

    if (m_socket.write(writebuf, skey.size()+3) != skey.size() + 3) {
        close();
        return;
    }
    if (m_socket.wait_for_data(60 * 1000) > 0) {
        int len = m_socket.read(readbuf, buflen);
        String readstr(readbuf, len);
        while (readbuf[len-1] != '\n') {
            len = m_socket.read(readbuf, buflen);
            readstr.append(readbuf, len);
        }
        if (readstr[0] == '1') {
            readstr.append(1, '\n');
            parse_dictline(m_converter, readstr.data(), result);
        }
    }
}


/*
 * scim_skk::CDBFile
 * connection to CDB(constant db) dictionary.
 */

CDBFile::CDBFile  (IConvert *conv,
                   const String &dictpath)
    : DictBase(conv, String("CDBFile:") + dictpath),
      m_db(dictpath)
{
}

CDBFile::~CDBFile (void)
{
    m_db.dbclose();
}

void
CDBFile::lookup (const WideString &key, const bool okuri,
                 list<CandPair> &result)
{
    static const int buflen = 4096;

    /* reconnect if the connection is closed */
    if (!m_db.is_opened()) return;

    String skey, sval;
    m_converter->convert(skey, key);
    if (m_db.get(skey, sval)) {
        sval.append(1, '\n');
        parse_dictline(m_converter, sval.data(), result);
    }
}



/*
 * scim_skk::UserDict
 * a dictionary object which access to user dictionary file directly.
 */

UserDict::UserDict  (IConvert *conv)
    : DictBase(conv),
      m_writeflag  (false)
{
}

UserDict::~UserDict (void)
{
}

void
UserDict::load_dict (const String &dictpath, History &hist)
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
        WideString key;
        list<CandPair> cl;
        int len;
        WideString alph = utf8_mbstowcs("abcdefghijklmnopqrstuvwxyz");
        for (int i = 0; i < length; i++) {
            switch(buf[i]) {
            case ';':
                for (; i < length && buf[i] != '\n'; i++);
            case '\n':
                break;
            default:
                key.clear();
                cl.clear();
                for (len = 0; buf[i+len] != ' '; len++);
                m_converter->convert(key, buf+i, len);
                i += len;
                i += parse_dictline(m_converter, buf+i, cl);
                m_dictdata.insert(make_pair(key, cl));
                if (alph.find(key.at(key.size()-1)) == WideString::npos)
                    hist.append_entry_to_tail(key);
                break;
            }
        }
        munmap(buf, length);
    }
    close(fd);
}

void
UserDict::dump_dict (void)
{
    map<WideString, list<CandPair> >::const_iterator dit;
    ofstream dictfs;

    if (m_writeflag) {
        dictfs.open(m_dictpath.c_str());
        for (dit = m_dictdata.begin(); dit != m_dictdata.end(); dit++) {
            if (!dit->second.empty()) {
                String line;
                String tmp;
                m_converter->convert(tmp, dit->first);
                line += tmp;
                line += ' ';

                for(list<CandPair>::const_iterator cit = dit->second.begin();
                    cit != dit->second.end(); cit++) {
                    String tmp2;
                    m_converter->convert(tmp2, cit->first);
                    tmp.clear();
                    rewrite_to_concatform(tmp, tmp2);
                    line += '/';
                    line += tmp;
                    if (!cit->second.empty()) {
                        tmp2.clear();
                        tmp.clear();
                        m_converter->convert(tmp2, cit->second);
                        rewrite_to_concatform(tmp, tmp2);
                        line += ';';
                        line += tmp;
                    }
                }
                dictfs << line << '/' << endl;
            }
        }
        dictfs.close();
    }
}

void
UserDict::lookup (const WideString &key, const bool okuri,
                     list<CandPair> &result)
{
    list<CandPair> &cl = m_dictdata[key];

    for (list<CandPair>::iterator it = cl.begin(); it != cl.end(); it++) {
        append_candpair(it->first, it->second, result);
    }
}

void
UserDict::write (const WideString &key, const CandPair &data)
{
    list<CandPair> &cl = m_dictdata[key];
    for (list<CandPair>::iterator it = cl.begin(); it != cl.end();) {
        if (it->first == data.first) {
            it = cl.erase(it);
        } else {
            ++it;
        }
    }
    cl.push_front(data);
    m_writeflag = true;
}


/*
 * SKKDictionary
 * SKKDictionary manages all of the dictionary classes and does
 * special conversions such like number conversions.
 */

SKKDictionary::SKKDictionary (void)
    : m_converter(new IConvert),
      m_userdict(new UserDict(m_converter)),
      m_cache(new DictCache())
{
    m_converter->set_encoding(String(SKKDICT_CHARCODE));
}

SKKDictionary::~SKKDictionary (void)
{
    for (list<DictBase*>::iterator i = m_sysdicts.begin();
         i != m_sysdicts.end(); i++)
        delete *i;
    if (m_converter) delete m_converter;
    if (m_cache) delete m_cache;
    if (m_userdict) delete m_userdict;
}

void
SKKDictionary::add_sysdict (const String &dicturi)
{
    list<DictBase*>::const_iterator it = m_sysdicts.begin();
    int pos = dicturi.find(':');
    String dicttype = (pos == String::npos)?
        "DictFile" : dicturi.substr(0, pos);
    String dictname = (pos == String::npos)?
        dicturi : dicturi.substr(pos+1, String::npos);
    for(; it != m_sysdicts.end(); it++)
        if ((*it)->dicturi == dicturi) break;
    if (it == m_sysdicts.end()) {
        if (dicttype == "DictFile") {
            m_sysdicts.push_back((DictBase*)new DictFile(m_converter,
                                                         dictname));
        } else if (dicttype == "SKKServ") {
            m_sysdicts.push_back((DictBase*)new SKKServ(m_converter,
                                                        dictname));
        } else if (dicttype == "CDBFile") {
            m_sysdicts.push_back((DictBase*)new CDBFile(m_converter,
                                                        dictname));
        }
    }
    m_cache->clear();
}

void
SKKDictionary::set_userdict (const String &dictname, History &hist)
{
    struct stat statbuf;
    String userdictpath = scim_get_home_dir() +
                          String(SCIM_PATH_DELIM_STRING) + dictname;
    if (stat(userdictpath.c_str(), &statbuf) < 0) {
        String skkuserdict = scim_get_home_dir() +
                             String(SCIM_PATH_DELIM_STRING) + String(".skk-jisyo");
        m_userdict->load_dict(skkuserdict, hist);
    }
    m_userdict->load_dict(userdictpath, hist);
}


void
SKKDictionary::dump_userdict (void)
{
    m_userdict->dump_dict();
}

static const ucs4_t zero = 0x30;
static const ucs4_t nine = 0x39;
static const ucs4_t sharp = 0x23;

inline void 
lookup_main (const WideString &key, const bool okuri,
             DictCache *cache, UserDict *userdict,
             const list<DictBase*> &sysdicts,
             list<CandPair> &result)
{
    list<CandPair> cl;
    cache->lookup(key, okuri, cl);
    if (cl.empty()) {
        userdict->lookup(key, okuri, cl);
        for (list<DictBase*>::const_iterator it = sysdicts.begin();
             it != sysdicts.end(); it++) {
            (*it)->lookup(key, okuri, cl);
        }
        cache->write(key, cl);
    }
    result.insert(result.end(), cl.begin(), cl.end());
}

void
SKKDictionary::lookup (const WideString &key_const, const bool okuri,
                       SKKCandList &result)
{
    WideString key;
    list<WideString> numbers;
    list<CandPair> cl;

    lookup_main(key_const, okuri, m_cache, m_userdict, m_sysdicts, cl);
    for (list<CandPair>::const_iterator it = cl.begin();
         it != cl.end(); it++) {
        result.append_candidate(it->first, it->second);
    }
    cl.clear();

    extract_numbers(key_const, numbers, key);

    lookup_main(key, okuri, m_cache, m_userdict, m_sysdicts, cl);
    for (list<CandPair>::const_iterator it = cl.begin();
         it != cl.end(); it++) {
        WideString cand;
        bool flag = number_conversion(numbers, it->first, cand);
        if (flag && !result.has_candidate(cand)) {
            result.append_candidate(cand, it->second, it->first);
        }
    }
}

void
SKKDictionary::write (const WideString &key,  const CandEnt &ent)
{
    if (ent.cand.empty()) return;
    if (ent.cand != ent.cand_orig) {
        WideString key2;
        for (int i = 0; i < key.size(); i++) {
            int start = i;
            while (i < key.size() && key[i] >= zero && key[i] <= nine) i++;
            if (start < i) {
                key2 += sharp;
                if (i < key.size()) key2 += key[i];
            } else {
                key2 += key[i];
            }
        }
        m_userdict->write(key2, make_pair(ent.cand_orig, ent.annot));
        m_cache->write(key2, make_pair(ent.cand_orig, ent.annot));
    } else {
        m_userdict->write(key, make_pair(ent.cand, ent.annot));
        m_cache->write(key, make_pair(ent.cand, ent.annot));
    }
}


void
SKKDictionary::extract_numbers (const WideString &key,
                                std::list<WideString> &result /* return value */,
                                WideString &newkey /* return value */)
{
    for (int i = 0; i < key.size(); i++) {
        int start = i;
        while (i < key.size() &&
               key[i] >= zero && key[i] <= nine) i++;
        if (start < i) {
            WideString num = key.substr(start, i-start);
            result.push_back(num);
            newkey += sharp;
            if (i < key.size())
                newkey += key[i];
        } else {
            newkey += key[i];
        }
    }
}

bool
SKKDictionary::number_conversion (const std::list<WideString> &numbers,
                                  const WideString &cand,
                                  WideString &result /* return value */)
{
    bool conversion_success = true;
    if (!numbers.empty()) {
        list<WideString>::const_iterator nit = numbers.begin();
        int start = 0;
        int sharp_pos;
        while(nit != numbers.end() &&
              (sharp_pos = cand.find(sharp, start)) != WideString::npos) {
            if (sharp_pos < cand.size()-1 &&
                cand[sharp_pos+1] >= zero &&
                cand[sharp_pos+1] <= nine) {
                if (sharp_pos > start)
                    result.append(cand, start, sharp_pos - start);
                switch (cand[sharp_pos+1] - zero) {
                case 0:
                    result.append(*nit);
                    break;
                case 1:
                    convert_num1(*nit, result);
                    break;
                case 2:
                    convert_num2(*nit, result);
                    break;
                case 3:
                    convert_num3(*nit, result);
                    break;
                case 4:
                    {
                        list<CandPair> cl;
                        lookup_main(*nit, false,
                                    m_cache, m_userdict, m_sysdicts, cl);
                        if (!cl.empty()) {
                            result += cl.begin()->first;
                        } else {
                            conversion_success = false;
                        }
                    }
                    break;
                case 5:
                    convert_num5(*nit, result);
                    break;
                case 9:
                    convert_num9(*nit, result);
                    break;
                default:
                    result += cand.substr(sharp_pos, 2);
                }
                start = sharp_pos + 2;
                nit++;
                if (!conversion_success) nit = numbers.end();
            } else { /* not conversion */
                result.append(1, sharp);
                start = sharp_pos + 1;
            }
        }
        if (start < cand.size())
            result.append(cand, start, cand.size() - start);
        return conversion_success;
    } else {
        result.append(cand);
        return true;
    }
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

/*
 * functions and constants for number conversions.
 */

static WideString digits_wide = utf8_mbstowcs("\xEF\xBC\x90" // 0
                                              "\xEF\xBC\x91" // 1
                                              "\xEF\xBC\x92" // 2
                                              "\xEF\xBC\x93" // 3
                                              "\xEF\xBC\x94" // 4
                                              "\xEF\xBC\x95" // 5
                                              "\xEF\xBC\x96" // 6
                                              "\xEF\xBC\x97" // 7
                                              "\xEF\xBC\x98" // 8
                                              "\xEF\xBC\x99" ); // 9

static WideString digits_kanji = utf8_mbstowcs("\xE3\x80\x87" // 0
                                               "\xE4\xB8\x80" // 1
                                               "\xE4\xBA\x8C" // 2
                                               "\xE4\xB8\x89" // 3
                                               "\xE5\x9B\x9B" // 4
                                               "\xE4\xBA\x94" // 5
                                               "\xE5\x85\xAD" // 6
                                               "\xE4\xB8\x83" // 7
                                               "\xE5\x85\xAB" // 8
                                               "\xE4\xB9\x9D" ); // 9


static WideString kei_kanji   = utf8_mbstowcs("\xE4\xBA\xAC");
static WideString chou_kanji  = utf8_mbstowcs("\xE5\x85\x86");
static WideString oku_kanji   = utf8_mbstowcs("\xE5\x84\x84");
static WideString man_kanji   = utf8_mbstowcs("\xE4\xB8\x87");
static WideString sen_kanji   = utf8_mbstowcs("\xE5\x8D\x83");
static WideString hyaku_kanji = utf8_mbstowcs("\xE7\x99\xBE");
static WideString juu_kanji   = utf8_mbstowcs("\xE5\x8D\x81");


static WideString digits_kanji_old = utf8_mbstowcs("\xE3\x80\x87" // 0
                                                   "\xE5\xA3\xB1" // 1
                                                   "\xE5\xBC\x90" // 2
                                                   "\xE5\x8F\x82" // 3
                                                   "\xE5\x9B\x9B" // 4
                                                   "\xE4\xBC\x8D" // 5
                                                   "\xE5\x85\xAD" // 6
                                                   "\xE4\xB8\x83" // 7
                                                   "\xE5\x85\xAB" // 8
                                                   "\xE4\xB9\x9D" ); // 9
static WideString man_kanji_old = utf8_mbstowcs("\xE8\x90\xAC");
static WideString sen_kanji_old = utf8_mbstowcs("\xE9\x98\xA1");
static WideString juu_kanji_old = utf8_mbstowcs("\xE6\x8B\xBE");

/* convert function for #1 and #2 */
inline void
convert_num_each_char (WideString key, WideString &result,
                       const WideString &digits)
{
    for (WideString::const_iterator it = key.begin(); it != key.end(); it++) {
        result.append(1, digits[*it - zero]);
    }
}

inline void
convert_num1 (WideString key, WideString &result)
{
    convert_num_each_char(key, result, digits_wide);
}

inline void
convert_num2 (WideString key, WideString &result)
{
    convert_num_each_char(key, result, digits_kanji);
}

/* conversion for #3 and #5 */
inline void
convert_num_with_ranks (WideString key, WideString &result,
                        const WideString &digits,
                        const WideString &kei,
                        const WideString &chou,
                        const WideString &oku,
                        const WideString &man,
                        const WideString &sen,
                        const WideString &hyaku,
                        const WideString &juu,
                        const bool ichi_flag)
{
    unsigned long long ikey = wstoll(key);
    if (ikey >= 10000000000000000ull) { 
        convert_num_with_ranks(lltows(ikey/10000000000000000ull),
                               result,
                               digits, kei, chou, oku, man, sen, hyaku, juu,
                               ichi_flag);
        result += kei;
        ikey = ikey % 10000000000000000ull;
    }
    if (ikey >= 1000000000000ull) { 
        convert_num_with_ranks(lltows(ikey/1000000000000ull),
                               result,
                               digits, kei, chou, oku, man, sen, hyaku, juu,
                               ichi_flag);
        result += chou;
        ikey = ikey % 1000000000000ull;
    }
    if (ikey >= 100000000) { 
        convert_num_with_ranks(lltows(ikey/100000000),
                               result,
                               digits, kei, chou, oku, man, sen, hyaku, juu,
                               ichi_flag);
        result += oku;
        ikey = ikey % 100000000;
    }
    if (ikey >= 10000) {
        if (ikey / 10000000 == 1)
            result += digits[1];
        convert_num_with_ranks(lltows(ikey/10000),
                               result,
                               digits, kei, chou, oku, man, sen, hyaku, juu,
                               ichi_flag);
        result += man;
        ikey = ikey % 10000;
    }
    if (ikey >= 1000) {
        if (ichi_flag || ikey / 1000 != 1)
            result += digits[ikey/1000];
        result += sen;
        ikey = ikey % 1000;
    }
    if (ikey >= 100) {
        if (ichi_flag || ikey / 100 != 1)
            result += digits[ikey/100];
        result += hyaku;
        ikey = ikey % 100;
    }
    if (ikey >= 10) {
        if (ichi_flag || ikey / 10 > 1)
            result += digits[ikey/10];
        result += juu;
        ikey = ikey % 10;
    }
    if (ikey > 0)
        result += digits[ikey];
}

inline void
convert_num3 (WideString key, WideString &result)
{
    convert_num_with_ranks(key, result, digits_kanji,
                           kei_kanji, chou_kanji, oku_kanji, man_kanji,
                           sen_kanji, hyaku_kanji, juu_kanji, false);
}

inline void
convert_num5 (WideString key, WideString &result)
{
    convert_num_with_ranks(key, result, digits_kanji_old,
                           kei_kanji, chou_kanji, oku_kanji, man_kanji_old,
                           sen_kanji_old, hyaku_kanji, juu_kanji_old, true);
}

inline void
convert_num9 (WideString key, WideString &result)
{
    if (key.size() == 2) {
        int a = key[0]-zero, b = key[1]-zero;
        result += digits_wide[a];
        result += digits_kanji[b];
    }
}


/*
 * parser for dictionary entry line
 */

inline int
parse_skip_paren (const char *line, int i)
{
    bool loopflag = true;

    while (loopflag && line[i] != '\n') {
        switch(line[i]) {
        case '(':
            i = parse_skip_paren(line, i+1);
            break;
        case ')':
            loopflag = false;
            i++;
            break;
        default:
            i++;
            break;
        }
    }
    return i;
}

inline int
parse_eval_string (const char *line, int start,
                   String &ret)
{
    int i = start;
    bool loopflag = true;
    while (loopflag && line[i] != '\n') {
        switch(line[i]) {
        case '"':
            i++;
            loopflag = false;
            break;
        case '\\':
            {
                char c1 = line[i+1], c2 = line[i+2], c3 = line[i+3];
                char code =
                    (c1 - '0') * 64 + (c2 - '0') * 8 + (c3 - '0');
                ret.append(1, code);
                i += 4;
            }
            break;
        default:
            ret.append(1, line[i]);
            i++;
            break;
        }
    }
    return i;
}

inline int
parse_paren (const char *line, int start,
             String &ret)
{
    if (strncmp(line+start, "concat", 6) == 0) {
        int i = start+6;
        bool loopflag = true;
        while (loopflag && line[i] != '\n') {
            switch(line[i]) {
            case '(':
                i = parse_skip_paren(line, i+1);
                break;
            case ')':
                loopflag = false;
                i++;
                break;
            case '"':
                i = parse_eval_string(line, i+1, ret);
                break;
            default:
                i++;
            }
        }
        return i;
    } else {
        ret.append(1, '(');
        return start;
    }
}

inline int
parse_skip_bracket (const char *line, int i)
{
    while (line[i] != '\n' && line[i] != ']') i++;
    if (line[i] == ']') i++;
    return i;
}

static int
parse_dictline (const IConvert *converter, const char *line,
                list<CandPair> &ret)
{
    WideString candbuf;
    WideString annotbuf;
    WideString tmpstr;
    WideString *target = &candbuf;
    int i, start;
    for (i = 0; line[i] != '/'; i++);
    i++;
    start = i;

    while (line[i] != '\n') {
        switch (line[i]) {
        case '/':
            tmpstr.clear();
            converter->convert(tmpstr, line+start, i-start);
            target->append(tmpstr);
            i++;
            start = i;
            append_candpair(candbuf, annotbuf, ret);
            candbuf.clear(); annotbuf.clear();
            target = &candbuf;
            break;
        case ';':
            tmpstr.clear();
            converter->convert(tmpstr, line+start, i-start);
            target->append(tmpstr);
            i++;
            start = i;
            target = &annotbuf;
            break;
        case '[':
            i = parse_skip_bracket(line, i+1);
            start = i;
            break;
        case '(':
            {
                tmpstr.clear();
                converter->convert(tmpstr, line+start, i-start);
                target->append(tmpstr);
                String buf;
                i = parse_paren(line, i+1, buf);
                start = i;
                tmpstr.clear();
                converter->convert(tmpstr, buf);
                target->append(tmpstr);
            }
            break;
        default:
            i++;
            break;
        }
    }
    return i;
}

static void
rewrite_to_concatform (String &dst, const String &src)
{
    int slash_pos = src.find('/');
    int semicolon_pos = src.find(';');
    if (slash_pos == String::npos && semicolon_pos == String::npos) {
        dst.assign(src);
    } else {
        dst.append("(concat \"");
        for (int i = 0; i < src.size(); i++) {
            switch (src[i]) {
            case '/':
                dst.append("\\057");
                break;
            case ';':
                dst.append("\\073");
                break;
            case '"':
                dst.append("\\042");
                break;
            default:
                dst.append(1, src[i]);
                break;
            }
        }
        dst.append("\")");
    }
}

inline WideString lltows(unsigned long long n)
{
    WideString ret;
    list<ucs4_t> l;
    if (n == 0) {
        ret.append(1, zero);
        return ret;
    }
    while (n > 0) {
        l.push_front(n%10+zero);
        n/=10;
    }
    for (list<ucs4_t>::const_iterator it = l.begin(); it != l.end(); it++) {
        ret.append(1, *it);
    }
    return ret;
}

inline unsigned long long wstoll(WideString ws)
{
    unsigned long long int ret = 0;
    for (int i = 0; i < ws.size(); i++) {
        if (ws[i] >= zero && ws[i] <= nine) {
            ret *= 10;
            ret += ws[i] - zero;
        } else {
            break;
        }
    }
    return ret;
}

