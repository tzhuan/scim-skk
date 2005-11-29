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

#ifndef __CDB_H__
#define __CDB_H__

#include <string>

class CDB {
private:
    std::string    m_filename;
    unsigned char *m_dbdata;
    int            m_fd;
    int            m_size;
    bool           m_open_flag;

    unsigned int calc_hash (const std::string &c);
    unsigned int get_value (int ptr);
public:
    CDB (const std::string &filename);
    ~CDB (void);

    void dbclose (void);
    bool is_opened (void);

    bool get (const std::string &key, std::string &val);
};

#endif /* __CDB_H__ */
