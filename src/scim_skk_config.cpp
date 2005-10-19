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

#include "scim_skk_prefs.h"
#include "scim_skk_config.h"

#include <scim.h>
#include <cstdlib>

using namespace scim;

namespace scim_skk {
    bool annot_view = SCIM_SKK_CONFIG_ANNOT_VIEW_DEFAULT;
    bool annot_pos =
    (String(SCIM_SKK_CONFIG_ANNOT_POS_DEFAULT) == String("inline"));
    bool annot_target =
    (String(SCIM_SKK_CONFIG_ANNOT_TARGET_DEFAULT) == String("all"));
    int candvec_size = SCIM_SKK_CONFIG_CANDVEC_SIZE_DEFAULT;
    bool annot_highlight = SCIM_SKK_CONFIG_ANNOT_HIGHLIGHT_DEFAULT;
    int annot_bgcolor = strtol(SCIM_SKK_CONFIG_ANNOT_BGCOLOR_DEFAULT+1,
                               (char**)NULL, 16);
    bool ignore_return = SCIM_SKK_CONFIG_IGNORE_RETURN_DEFAULT;
}
