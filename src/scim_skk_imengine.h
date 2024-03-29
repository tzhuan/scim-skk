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

/*
 * The original code is scim_uim_imengine.cpp in scim-uim-0.1.3. 
 * Copyright (C) 2004 James Su <suzhe@tsinghua.org.cn>
 */

#ifndef __SCIM_SKK_IMENGINE_H__
#define __SCIM_SKK_IMENGINE_H__

#define Uses_SCIM_ICONV
#include <scim.h>

#include "scim_skk_keybind.h"
#include "scim_skk_core.h"
#include "scim_skk_dictionary.h"
#include "scim_skk_lookup_table.h"
#include "scim_skk_config.h"

using namespace scim;

namespace scim_skk {

class SKKFactory : public IMEngineFactoryBase
{
    friend class SKKInstance;

    String m_uuid;

    /* dictionary */
    std::vector<String> m_sysdicts;
    String              m_userdictname;

    /* config */
    ConfigPointer m_config;
    Connection    m_reload_signal_connection;

    KeyBind       m_keybind;
    /* for key bindings */

public:
    SKKFactory (const String &lang,
                const String &uuid,
                const ConfigPointer &config);
    virtual ~SKKFactory();

    virtual WideString  get_name () const;
    virtual String      get_uuid () const;
    virtual String      get_icon_file () const;
    virtual WideString  get_authors () const;
    virtual WideString  get_credits () const;
    virtual WideString  get_help () const;

    void dump_dict (void);
    virtual IMEngineInstancePointer create_instance (const String &encoding, int id = -1);

private:
    void reload_config (const ConfigPointer &config);
};

class SKKInstance : public IMEngineInstanceBase
{
    PropertyList   m_properties;

    /* for displaying SKKMode */
    SKKMode        m_skk_mode;

    /* core of SKK */
    SKKCore        m_skkcore;

    bool process_kakutei_keys         (const KeyEvent &key);
    bool process_remaining_keybinds   (const KeyEvent &key);

    void install_properties (void);
    void set_skk_mode       (SKKMode newmode);
    void set_period_style   (PeriodStyle newstyle);

    void update_candidates (void);
public:
    SKKInstance (SKKFactory   *factory,
                 const String &encoding,
                 int           id = -1);
    virtual ~SKKInstance ();

    virtual bool process_key_event (const KeyEvent& key);
    virtual void move_preedit_caret (unsigned int pos);
    virtual void select_candidate (unsigned int index);
    virtual void update_lookup_table_page_size (unsigned int page_size);
    virtual void lookup_table_page_up (void);
    virtual void lookup_table_page_down (void);
    virtual void reset (void);
    virtual void focus_in (void);
    virtual void focus_out (void);
    virtual void trigger_property (const String& property);
};

} /* namespace scim_skk */
#endif /* __SCIM_SKK_IMENGINE_H__ */
