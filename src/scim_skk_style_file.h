/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
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

#ifndef __SCIM_SKK_STYLE_FILE_H__
#define __SCIM_SKK_STYLE_FILE_H__

#define Uses_SCIM_ICONV
#include <scim.h>

#include "scim_skk_automaton.h"

using namespace scim;

namespace scim_skk {

class StyleLine;
class StyleSection;
class StyleFile;

typedef std::vector<StyleLine>  StyleLines;
typedef std::vector<StyleLines> StyleSections;
typedef std::vector<StyleFile>  StyleFiles;

typedef enum {
    SCIM_ANTHY_STYLE_LINE_UNKNOWN,
    SCIM_ANTHY_STYLE_LINE_SPACE,
    SCIM_ANTHY_STYLE_LINE_COMMENT,
    SCIM_ANTHY_STYLE_LINE_SECTION,
    SCIM_ANTHY_STYLE_LINE_KEY,
} StyleLineType;

class StyleLine
{
public:
    StyleLine (StyleFile  *style_file,
               String      line);
    StyleLine (StyleFile  *style_file,
               String      key,
               String      value);
    StyleLine (StyleFile  *style_file,
               String      key,
               std::vector<String> &value);
    ~StyleLine ();

public:
    StyleLineType get_type        (void);
    void          get_line        (String     &line) { line = m_line; }
    bool          get_section     (String     &section);
    bool          get_key         (String     &key);
    bool          get_value       (String     &value);
    void          set_value       (String      value);
    bool          get_value_array (std::vector<String> &value);
    void          set_value_array (std::vector<String> &value);

private:
    StyleFile     *m_style_file;
    String         m_line;
    StyleLineType  m_type;
};

class StyleFile
{
public:
    StyleFile ();
    ~StyleFile ();

public:
    bool   load                  (const char *filename);
    bool   save                  (const char *filename);

    String get_encoding          (void);
    String get_title             (void);
    String get_file_name         (void);

    bool   get_section_list      (StyleSections &sections);
    bool   get_entry_list        (StyleLines    &lines,
                                  String         section);
    bool   get_key_list          (std::vector<String> &keys,
                                  String         section);
    bool   get_string            (String        &value,
                                  String         section,
                                  String         key);
    bool   get_string            (WideString    &value,
                                  String         section,
                                  String         key);
    bool   get_string_array      (std::vector<String> &value,
                                  String         section,
                                  String         key);
    bool   get_string_array      (std::vector<WideString> &value,
                                  String         section,
                                  String         key);

    void   set_string            (String         section,
                                  String         key,
                                  String         value);
    void   set_string            (String         section,
                                  String         key,
                                  WideString     value);
    void   set_string_array      (String         section,
                                  String         key,
                                  std::vector<String> &value);
    void   set_string_array      (String         section,
                                  String         key,
                                  std::vector<WideString> &value);

    void   delete_key            (String         section,
                                  String         key);
    void   delete_section        (String         section);

public: // for getting specific data
    bool   get_key2kana_table    (SKKAutomaton  &key2kana,
                                  const String  &section);

private:
    void   clear                 (void);
    void   setup_default_entries (void);
    StyleLines *
           find_section          (const String  &section);
    StyleLines &
           append_new_section    (const String  &section);

private:
    IConvert      m_iconv;

    String        m_filename;
    String        m_format_version;
    String        m_encoding;
    String        m_title;
    String        m_version;

    StyleSections m_sections;
};

}

#endif /* __SCIM_SKK_STYLE_FILE_H__ */
