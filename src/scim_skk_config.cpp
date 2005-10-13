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

#include <cstdlib>
#include <cstring>

SKKConfig::SKKConfig (void)
  : annot_view(SCIM_SKK_CONFIG_ANNOT_VIEW_DEFAULT),
    annot_pos(strncmp(SCIM_SKK_CONFIG_ANNOT_POS_DEFAULT, "inline", 6) == 0),
    annot_target(strncmp(SCIM_SKK_CONFIG_ANNOT_TARGET_DEFAULT, "all", 3) == 0),
    candvec_size(SCIM_SKK_CONFIG_CANDVEC_SIZE_DEFAULT),
    annot_highlight(SCIM_SKK_CONFIG_ANNOT_HIGHLIGHT_DEFAULT),
    annot_bgcolor(strtol(SCIM_SKK_CONFIG_ANNOT_BGCOLOR_DEFAULT+1,
                         (char**)NULL, 16)),
    ignore_return(SCIM_SKK_CONFIG_IGNORE_RETURN_DEFAULT)
{}
