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

#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include "cdb.h"

using namespace std;

CDB::CDB (const string &filename)
    : m_filename (filename),
      m_open_flag (false)
{
    struct stat statbuf;
    if( stat(m_filename.data(), &statbuf) == 0 &&
        (m_fd = open(m_filename.data(), O_RDONLY)) > 0) {
        m_size = statbuf.st_size;
        m_dbdata = static_cast<unsigned char*>(mmap(0, m_size, PROT_READ, MAP_SHARED, m_fd, 0));
        if (m_dbdata == MAP_FAILED) {
            close(m_fd);
        } else {
            m_open_flag = true;
        }
    }
}

CDB::~CDB (void)
{
    dbclose();
}

void
CDB::dbclose (void)
{
    if (m_open_flag) {
        munmap(m_dbdata, m_size);
        close(m_fd);
    }
}

bool
CDB::is_opened (void)
{
    return m_open_flag;
}

bool
CDB::get (const string &key, string &val)
{
    if (!m_open_flag) return false;

    bool retval = false;
    unsigned int hash_val = calc_hash(key);
    unsigned int hashtbl_pos = get_value((hash_val % 256) * 2 * 4);
    unsigned int hashtbl_len = get_value((hash_val % 256) * 2 * 4 + 4);
    unsigned int entry_point =
        hashtbl_pos + ((hash_val / 256) % hashtbl_len) * 2 * 4;
    unsigned int entry_hashval = get_value(entry_point);
    unsigned int entry_pos = get_value(entry_point + 4);
    while (entry_pos != 0) {
        if (entry_hashval == hash_val) {
            int entry_keylen = get_value(entry_pos);
            int entry_vallen = get_value(entry_pos + 4);
            string entry_key((char *)&(m_dbdata[entry_pos+8]), entry_keylen);
            if (key == entry_key) {
                val.assign((char *)&(m_dbdata[entry_pos+8+entry_keylen]), entry_vallen);
                return true;
            }
        }
        entry_point += 8;
        entry_hashval = get_value(entry_point);
        entry_pos = get_value(entry_point+4);
    }
    return false;
}

unsigned int
CDB::calc_hash (const string &us)
{
    unsigned int retval = 5381;
    for (string::const_iterator it = us.begin(); it != us.end(); it++) {
        retval = ((retval << 5) + retval) ^ ((unsigned char) *it);
    }
    return retval;
}

unsigned int
CDB::get_value (int ptr)
{
    if (is_opened()) {
        unsigned char c1 = m_dbdata[ptr], c2 = m_dbdata[ptr+1],
            c3 = m_dbdata[ptr+2], c4 = m_dbdata[ptr+3];
        return ((c4 << 24) + (c3 << 16) + (c2 << 8) + c1);
    } else {
        return 0;
    }
}
