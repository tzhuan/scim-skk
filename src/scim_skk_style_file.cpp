/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2006 Jun Mukai
 *  Copyright (C) 2005 Takuro Ashie
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

#include "scim_skk_style_file.h"

using namespace scim_skk;

const int MAX_LINE_LENGTH = 4096;

static String
escape (const String &str)
{
    String dest = str;

    for (unsigned int i = 0; i < dest.size (); i++) {
        if (dest[i] == '#'  ||                   // for comment
            dest[i] == '\\' ||                   // for backslash itself
            dest[i] == '='  ||                   // for separatort
            dest[i] == '['  || dest[i] == ']' || // for section
            dest[i] == ','  ||                   // for array
            dest[i] == ' '  || dest[i] == '\t')  // for space
        {
            dest.insert (i, "\\");
            i++;
        }
    }

    return dest;
}

static String
unescape (const String &str)
{
    String dest = str;

    for (unsigned int i = 0; i < dest.size (); i++) {
        if (dest[i] == '\\') {
            dest.erase (i, 1);
            if (i < dest.size () && dest[i] == '\\')
                i++;
        }
    }

    return dest;
}

StyleLine::StyleLine (StyleFile *style_file, String line)
    : m_style_file (style_file),
      m_line  (line),
      m_type  (SCIM_ANTHY_STYLE_LINE_UNKNOWN)
{
}

StyleLine::StyleLine (StyleFile *style_file, String key, String value)
    : m_style_file (style_file),
      m_line  (escape (key) + String ("=")),
      m_type  (SCIM_ANTHY_STYLE_LINE_KEY)
{
    set_value (value);
}

StyleLine::StyleLine (StyleFile *style_file, String key,
                      std::vector<String> &value)
    : m_style_file (style_file),
      m_line  (escape (key) + String("=")),
      m_type  (SCIM_ANTHY_STYLE_LINE_KEY)
{
    set_value_array (value);
}

StyleLine::~StyleLine ()
{
}

StyleLineType
StyleLine::get_type (void)
{
    if (m_type != SCIM_ANTHY_STYLE_LINE_UNKNOWN)
        return m_type;

    unsigned int spos, epos;
    for (spos = 0;
         spos < m_line.length () && isspace (m_line[spos]);
         spos++);
    if (m_line.length() > 0) {
        for (epos = m_line.length () - 1;
             epos >= 0 && isspace (m_line[epos]);
             epos--);
    } else {
        epos = 0;
    }

    if (m_line.length() == 0 || spos >= m_line.length()) {
        m_type = SCIM_ANTHY_STYLE_LINE_SPACE;
        return m_type;

    } else if (m_line[spos] == '#') {
        m_type = SCIM_ANTHY_STYLE_LINE_COMMENT;
        return m_type;

    } else if (m_line[spos] == '[' && m_line[epos] == ']') {
        m_type = SCIM_ANTHY_STYLE_LINE_SECTION;
        return m_type;
    }

    m_type = SCIM_ANTHY_STYLE_LINE_KEY;
    return m_type;
}

bool
StyleLine::get_section (String &section)
{
    if (get_type () != SCIM_ANTHY_STYLE_LINE_SECTION)
        return false;

    unsigned int spos, epos;
    for (spos = 0;
         spos < m_line.length () && isspace (m_line[spos]);
         spos++);
    for (epos = m_line.length () - 1;
         epos >= 0 && isspace (m_line[epos]);
         epos--);
    spos++;

    if (spos < epos)
        section = m_line.substr (spos, epos - spos);
    else
        section = String ();

    return true;
}

bool
StyleLine::get_key (String &key)
{
    if (get_type () != SCIM_ANTHY_STYLE_LINE_KEY)
        return false;

    unsigned int spos, epos;
    for (spos = 0;
         spos < m_line.length () && isspace (m_line[spos]);
         spos++);
    bool found = false;
    for (epos = spos;
         epos < m_line.length ();
         epos++)
    {
        if (m_line[epos] == '\\') {
            epos++;
            continue;
        }
        if (m_line[epos] == '=') {
            found = true;
            break;
        }
    }
    for (--epos;
         epos >= spos && isspace (m_line[epos]);
         epos--);
    if (!isspace(m_line[epos]))
        epos++;

    if (spos >= 0 && spos < epos && epos <= m_line.length ()) {
        key = unescape (m_line.substr (spos, epos - spos));
    } else
        key = String ();

    return true;
}

static int
get_value_position (String &str)
{
    unsigned int spos;
    for (spos = 0;
         spos < str.length ();
         spos++)
    {
        if (str[spos] == '\\') {
            spos++;
            continue;
        }
        if (str[spos] == '=') {
            break;
        }
    }
    if (spos >= str.length ())
        return true;
    else
        spos++;
    for (;
         spos < str.length () && isspace(str[spos]);
         spos++);

    return spos;
}

bool
StyleLine::get_value (String &value)
{
    if (get_type () != SCIM_ANTHY_STYLE_LINE_KEY)
        return false;

    unsigned int spos = get_value_position (m_line);
    unsigned int epos = m_line.length ();

    value = unescape (m_line.substr (spos, epos - spos));

    return true;
}

void
StyleLine::set_value (String value)
{
    String key;
    get_key (key);
    m_line = escape (key) + String ("=") + escape (value);
}

bool
StyleLine::get_value_array (std::vector<String> &value)
{
    if (get_type () != SCIM_ANTHY_STYLE_LINE_KEY)
        return false;

    unsigned int spos = get_value_position (m_line);
    unsigned int epos = m_line.length ();

    unsigned int head_of_element = spos;
    for (unsigned int i = spos; i <= epos; i++) {
        if (i < epos && m_line[i] == '\\') {
            i++;
            continue;
        }

        if (i == epos || m_line[i] == ',') {
            String str;
            if (head_of_element == epos)
                str = String ();
            else
                str = unescape (m_line.substr (head_of_element,
                                               i - head_of_element));
            value.push_back (str);
            head_of_element = i + 1;
        }
    }

    return true;
}

void
StyleLine::set_value_array (std::vector<String> &value)
{
    String key;
    get_key (key);

    m_line = escape (key) + String ("=");
    for (unsigned int i = 0; i < value.size (); i++) {
        if (i != 0)
            m_line += ",";
        m_line += escape (value[i]);
    }
}


StyleFile::StyleFile ()
{
    setup_default_entries ();
}

StyleFile::~StyleFile ()
{
}

bool
StyleFile::load (const char *filename)
{
    clear ();
    setup_default_entries ();
    m_filename = filename;

    std::ifstream in_file (filename);
    if (!in_file)
        return false;

    clear ();

    m_sections.push_back (StyleLines ());
    StyleLines *section = &m_sections[0];
    unsigned int section_id = 0;

    char buf[MAX_LINE_LENGTH];
    do {
        in_file.getline (buf, MAX_LINE_LENGTH);  
        if (in_file.eof ())
            break;

        WideString dest;
        m_iconv.convert (dest, buf);
        StyleLine line (this, utf8_wcstombs (dest));
        StyleLineType type = line.get_type ();

        if (type == SCIM_ANTHY_STYLE_LINE_SECTION) {
            m_sections.push_back (StyleLines ());
            section = &m_sections.back();
            section_id++;
        }

        section->push_back (line);

        if (section_id == 0) {
            String key;
            line.get_key (key);
            if (key == "FormatVersion") {
                line.get_value (m_format_version);

            } else if (key == "Encoding") {
                line.get_value (m_encoding);
                bool success = m_iconv.set_encoding (m_encoding);
                if (!success)
                    m_iconv.set_encoding ("UTF-8");

            } else if (key == "Title") {
                line.get_value (m_title);

            } else if (key == "Version") {
                line.get_value (m_version);
            }
        }
    } while (!in_file.eof ());

    in_file.close ();

    m_filename = filename;

    return true;
}

bool
StyleFile::save (const char *filename)
{
    std::ofstream out_file (filename);
    if (!out_file)
        return false;

    StyleSections::iterator it;
    for (it = m_sections.begin (); it != m_sections.end (); it++) {
        StyleLines::iterator lit;
        for (lit = it->begin (); lit != it->end (); lit++) {
            String line, dest;
            lit->get_line (line);
            m_iconv.convert (dest, utf8_mbstowcs (line));
            out_file << dest.c_str () << std::endl;
        }
    }

    out_file.close ();

    m_filename = filename;

    return true;
}

void
StyleFile::clear (void)
{
    m_filename       = String ();
    m_format_version = String ();
    m_encoding       = String ();
    m_title          = String ();
    m_version        = String ();
    m_sections.clear ();
}

String
StyleFile::get_encoding (void)
{
    return m_encoding;
}

String
StyleFile::get_title (void)
{
    return m_title;
}

String
StyleFile::get_file_name (void)
{
    return m_filename;
}

bool
StyleFile::get_string (String &value, String section, String key)
{
    StyleSections::iterator it;
    for (it = m_sections.begin (); it != m_sections.end (); it++) {
        if (it->size () <= 0)
            continue;

        String s, k;
        (*it)[0].get_section (s);

        if (s != section)
            continue;

        StyleLines::iterator lit;
        for (lit = it->begin (); lit != it->end (); lit++) {
            lit->get_key (k);
            if (k == key) {
                lit->get_value (value);
                return true;
            }
        }
    }

    return false;
}

bool
StyleFile::get_string (WideString &value, String section, String key)
{
    String str;
    bool success = get_string (str, section, key);
    if (!success)
        return false;
    value = utf8_mbstowcs (str);
    return true;
}

bool
StyleFile::get_string_array (std::vector<String> &value,
                             String section, String key)
{
    StyleLines *lines = find_section (section);
    if (!lines)
        return false;

    // find entry
    StyleLines::iterator lit;
    for (lit = lines->begin (); lit != lines->end (); lit++) {
        String k;
        lit->get_key (k);
        if (k == key) {
            lit->get_value_array (value);
            return true;
        }
    }

    return false;
}

bool
StyleFile::get_string_array (std::vector<WideString> &value,
                             String section, String key)
{
    std::vector<String> array;
    bool success = get_string_array (array, section, key);
    if (!success)
        return false;

    std::vector<String>::iterator it;
    for (it = array.begin (); it != array.end (); it++)
        value.push_back (utf8_mbstowcs (*it));
    return true;
}

void
StyleFile::set_string (String section, String key, String value)
{
    StyleLines *lines = find_section (section);
    if (lines) {
        // find entry
        StyleLines::iterator lit, last = lines->begin () + 1;
        for (lit = last; lit != lines->end (); lit++) {
            StyleLineType type = lit->get_type ();
            if (type != SCIM_ANTHY_STYLE_LINE_SPACE)
                last = lit + 1;

            String k;
            lit->get_key (k);

            // replace existing entry
            if (k.length () > 0 && k == key) {
                lit->set_value (value);
                return;
            }
        }

        // append new entry if no mathced entry exists.
        lines->insert (last, StyleLine (this, key, value));

    } else {
        StyleLines &newsec = append_new_section (section);

        // append new entry
        newsec.push_back (StyleLine (this, key, value));
    }
}

void
StyleFile::set_string (String section, String key, WideString value)
{
    set_string (section, key, utf8_wcstombs (value));
}

void
StyleFile::set_string_array (String section, String key,
                             std::vector<String> &value)
{
    StyleLines *lines = find_section (section);
    if (lines) {
        // find entry
        StyleLines::iterator lit, last = lines->begin () + 1;
        for (lit = last; lit != lines->end (); lit++) {
            StyleLineType type = lit->get_type ();
            if (type != SCIM_ANTHY_STYLE_LINE_SPACE)
                last = lit;

            String k;
            lit->get_key (k);

            // replace existing entry
            if (k.length () > 0 && k == key) {
                lit->set_value_array (value);
                return;
            }
        }

        // append new entry if no mathced entry exists.
        lines->insert (last + 1, StyleLine (this, key, value));

    } else {
        StyleLines &newsec = append_new_section (section);

        // append new entry
        newsec.push_back (StyleLine (this, key, value));
    }
}

void
StyleFile::set_string_array (String section, String key,
                             std::vector<WideString> &value)
{
    std::vector<String> array;
    std::vector<WideString>::iterator it;
    for (it = value.begin (); it != value.end (); it++)
        array.push_back (utf8_wcstombs (*it));
    set_string_array (section, key, array);
}

bool
StyleFile::get_section_list (StyleSections &sections)
{
    sections = m_sections;
    return true;
}

bool
StyleFile::get_entry_list (StyleLines &lines, String section)
{
    StyleSections::iterator it;
    for (it = m_sections.begin (); it != m_sections.end (); it++) {
        if (it->size () <= 0)
            continue;

        String s;
        (*it)[0].get_section (s);
        if (s == section) {
            lines = (*it);
            return true;
        }
    }

    return false;
}

bool
StyleFile::get_key_list (std::vector<String> &keys, String section)
{
    StyleLines *lines = find_section (section);
    if (!lines)
        return false;

    StyleLines::iterator lit;
    for (lit = lines->begin (); lit != lines->end (); lit++) {
        if (lit->get_type () != SCIM_ANTHY_STYLE_LINE_KEY)
            continue;

        String key;
        lit->get_key (key);
        keys.push_back (key);
    }
    return true;
}

void
StyleFile::delete_key (String section, String key)
{
    StyleLines *lines = find_section (section);
    if (!lines)
        return;

    // find entry
    StyleLines::iterator lit;
    for (lit = lines->begin (); lit != lines->end (); lit++) {
        String k;
        lit->get_key (k);
        if (k == key) {
            lines->erase (lit);
            return;
        }
    }
}

void
StyleFile::delete_section (String section)
{
    StyleSections::iterator it;
    for (it = m_sections.begin (); it != m_sections.end (); it++) {
        if (it->size () <= 0)
            continue;

        StyleLines::iterator lit;
        String s;
        (*it)[0].get_section (s);
        if (s == section) {
            m_sections.erase (it);
            return;
        }
    }
}

bool
StyleFile::get_key2kana_table (SKKAutomaton &key2kana,
                               const String &section)
{
    std::vector<String> keys;
    bool success = get_key_list(keys, section);
    if (success) {
        key2kana.set_title(utf8_mbstowcs(get_title()));
        std::vector<String>::iterator it;
        for (it = keys.begin (); it != keys.end (); it++) {
            std::vector<String> array;
            get_string_array (array, section, *it);
            key2kana.append_rule (*it, array);
        }
    }
    return success;
}

void
StyleFile::setup_default_entries (void)
{
    m_encoding = "UTF-8";
    m_title    = "User defined";
    m_iconv.set_encoding (m_encoding);
    m_sections.push_back (StyleLines ());

    m_sections.push_back (StyleLines ());
    StyleLines &newsec = m_sections.back ();
    String str = String ("Encoding") + String ("=") + escape (m_encoding);
    newsec.push_back (StyleLine (this, str.c_str ()));
    str = String ("Title") + String ("=") + escape (m_title);
    newsec.push_back (StyleLine (this, str.c_str ()));
}

StyleLines *
StyleFile::find_section (const String  &section)
{
    // find section
    StyleSections::iterator it;
    for (it = m_sections.begin (); it != m_sections.end (); it++) {
        if (it->size () <= 0)
            continue;

        String s;
        (*it)[0].get_section (s);

        if (s == section)
            return &(*it);
    }

    return NULL;
}

StyleLines &
StyleFile::append_new_section (const String &section)
{
    // append space before new section
    if (!m_sections.empty()) {
        StyleLines &sec = m_sections.back ();
        if (sec.empty() ||
            sec.back().get_type() != SCIM_ANTHY_STYLE_LINE_SPACE)
        {
            sec.push_back (StyleLine (this, ""));
        }
    }

    //
    // append new section
    //
    m_sections.push_back (StyleLines ());
    StyleLines &newsec = m_sections.back ();

    // new section entry
    String str = String ("[") + String (section) + String ("]");
    newsec.push_back (StyleLine (this, str.c_str ()));

    return newsec;
}
